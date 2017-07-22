#include "songmain.h"
#include "MIDIhardware.h"
#include "MIDIindevice.h"
#include "MIDIthruproc.h"
#include "MIDIinproc.h"
#include "languagefiles.h"
#include "gui.h"
#include "object_song.h"
#include "semapores.h"
#include "MIDItimer.h"

void MIDIInputDevice::AddMonitor(UBYTE status,UBYTE byte1,UBYTE byte2)
{
	switch(status&0xF0)
	{
	case NOTEON:
		if(byte2!=0 || displaynoteoff_monitor==true)
		{
			LockMonitor();
			monitor_events[monitor_eventcounter].Init(status,byte1,byte2,3,0);
			monitor_eventcounter==MAXMONITOREVENTS-1?monitor_eventcounter=0:monitor_eventcounter++;
			UnlockMonitor();
		}
		break;

	case NOTEOFF:
		if(displaynoteoff_monitor==true)
		{
			LockMonitor();
			monitor_events[monitor_eventcounter].Init(status,byte1,byte2,3,0);
			monitor_eventcounter==MAXMONITOREVENTS-1?monitor_eventcounter=0:monitor_eventcounter++;
			UnlockMonitor();
		}
		break;

	case PROGRAMCHANGE:
	case CONTROLCHANGE:
	case CHANNELPRESSURE:
	case POLYPRESSURE:
	case PITCHBEND:
		{
			LockMonitor();
			monitor_events[monitor_eventcounter].Init(status,byte1,byte2,3,0);
			monitor_eventcounter==MAXMONITOREVENTS-1?monitor_eventcounter=0:monitor_eventcounter++;
			UnlockMonitor();
		}
		break;
	}
}

void MIDIInputDevice::ShortMIDIInputMessage(LONGLONG systime,DWORD data) // millisec since Start
{
	union{             
		DWORD dwData; 
		BYTE bData[4]; 
	} u;

	UBYTE status,byte1,byte2;

	u.dwData=data;

	if(u.bData[0]>=128) // Status
	{
		status=u.bData[0];
		byte1=u.bData[1];
		byte2=u.bData[2];

		// RealTime Category messages (ie, Status of 0xF8 to 0xFF) do not effect running status in any way.

		if(status<0xF8) // no RT 
			runningstatus=status;
	}
	else
	{
		status=runningstatus;

		byte1=u.bData[0];
		byte2=u.bData[1];
	}

	// status=u.bData[0];

	// Filter Realtime Messages ?
	switch(status)
	{
	case 0xFE: // Active Sensing
		return;
		break;


	// Realtime Messages
	case MIDIREALTIME_START:
	case MIDIREALTIME_STOP:
	case MIDIREALTIME_CONTINUE:
	case MIDIREALTIME_CLOCK:
		{
			
		}
		break;

	default:
		{
			if(inputfilter.CheckBytes(status,byte1,byte2)==false)
				return;

			AddMonitor(status,byte1,byte2);
		}
		break;
	}

	LockInputBufferCounter();

	{
		MIDIInBufferEvent *b=&buffer[inputwritebuffercounter];

		b->systime=systime;
		b->status=status;
		b->bytes[0]=byte1;
		b->bytes[1]=byte2;
	}

	if(inputwritebuffercounter==MIDIINPUTBUFFER-1)
		inputwritebuffercounter=0;
	else 
		inputwritebuffercounter++;

	UnLockInputBufferCounter();

	// Signal to MIDI Record/Thru Thread

	//mainMIDIthruthread->SetSignal();
	MIDIinproc->SetSignal();
	/*
	mainthreadcontrol->SendMIDIThruSignal();
	mainthreadcontrol->SendMIDIInSignal(); // Signal to  Record Proc
	*/
}

void MIDIInputDevice::LongMIDIInputMessage(LONGLONG systime,unsigned char *data,int length,bool newsysex)
{
	if(length>0)
	{
		MIDIInBufferEvent *b=&buffer[inputwritebuffercounter];

		b->bytes[0]=newsysex==true?1:0;
		b->status=SYSEX;
		b->data=0;

		TRACE (" LongMIDIInputMessage,%d\n",length);

		if(b->data=new unsigned char[length]){

			memcpy(b->data,data,length);

			b->systime=systime;
			b->length=length;

			LockInputBufferCounter();
			
			if(inputwritebuffercounter==MIDIINPUTBUFFER-1)
				inputwritebuffercounter=0;
			else 
				inputwritebuffercounter++;
			
			UnLockInputBufferCounter();

			// Signal to MIDI Record/Thru Thread
			//mainMIDIthruthread->SetSignal();
			MIDIinproc->SetSignal();
		}
		else
			b->length=0;
	}
}

#ifdef WIN32
void CALLBACK MIDIInProc(HMIDIIN handle, UINT message, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{	
	switch(message)
	{
	case MIM_OPEN: //is called on midiInOpen()
		TRACE("Callback MIDI In Open \n");
		break;

	case MIM_CLOSE: //is called on midiInClose()
		{
		}
		TRACE("Callback MIDI In Close \n");
		break;

	case MIM_DATA: //is called when a MIDI message is received; it's packed in dwParam1. dwParam2 is Time Stamp
		//	case MIM_MOREDATA: //this should not occur (your computer is too slow for MIDI). Anyway, these messages are handles as MIM_DATA
		//	case MIM_ERROR: //"invalid" MIDI message. This will be handled as a normal one.

		if(mainMIDI->init==true)
		{
			MIDIInputDevice *dev=mainMIDI->FirstMIDIInputDevice();

			while(dev) // Find MIDI Input Device for this message
			{
				if(dev->hMIDIIn==handle)
				{
					dev->ShortMIDIInputMessage(maintimer->GetSystemTime(),dwParam1);
					return;
				}

				dev=dev->NextInputDevice();
			}
		}
		break;

	case MIM_ERROR:
		{
			int i=0;
		}
		break;

	case MIM_MOREDATA:
		{
			int i=0;
		}
		break;

		// case MIM_LONGERROR:
	case MIM_LONGDATA:  //System Exclusive Message received
		if(mainMIDI->init==true)
		{
			MIDIInputDevice *dev=mainMIDI->FirstMIDIInputDevice();

			while(dev) // Find MIDI Input Device for this message
			{
				if(dev->hMIDIIn==handle)
				{
					if(dev->checksysex==true)
					{
						LPMIDIHDR hdr = (LPMIDIHDR) dwParam1;

						//MIDIHDR *hdr=(MIDIHDR *)dwParam1;

						if(unsigned char *check=(unsigned char *)hdr->lpData)
						{
							// New Sysdata
							dev->LongMIDIInputMessage(maintimer->GetSystemTime(),(unsigned char *)hdr->lpData,hdr->dwBytesRecorded,*check==SYSEX?true:false);

							// Switch Buffer
							if(check==dev->MIDIinbuf1)
								midiInAddBuffer(dev->hMIDIIn, &dev->sysexin_MIDIHdr1, sizeof(MIDIHDR));
							else
								if(check==dev->MIDIinbuf2)
									midiInAddBuffer(dev->hMIDIIn, &dev->sysexin_MIDIHdr2, sizeof(MIDIHDR));
						}
					}

					return;
				}

				dev=dev->NextInputDevice();
			}
		}

		break;
	}
}
#endif

void MIDIInputDevice::OpenInputDevice() // open, init MIDI-Device
{
	TRACE ("OpenInput Device %s\n",name);

	if(deviceinit==false)
	{	
		if(type==OS_MIDIINTERFACE)
		{
#ifdef WIN32
			MMRESULT res=MMSYSERR_ERROR;

			try
			{
				res=midiInOpen(&hMIDIIn, id, (DWORD_PTR)MIDIInProc, 0, CALLBACK_FUNCTION);
			}

			catch(...)
			{
			}

			maingui->MessageMMError(FullName(),"MIDI Input Device (Open)",res);

			if(res==MMSYSERR_NOERROR && hMIDIIn)
			{			
				sysexin_MIDIHdr1.lpData = (char *)MIDIinbuf1;
				sysexin_MIDIHdr1.dwBufferLength = sizeof(MIDIinbuf1);
				sysexin_MIDIHdr1.dwFlags = 0;

				res=midiInPrepareHeader(hMIDIIn, &sysexin_MIDIHdr1, sizeof(MIDIHDR));

				maingui->MessageMMError(FullName(),"MIDI Input Device (midiInPrepareHeader 1)",res);

				if(res==MMSYSERR_NOERROR)
				{
					sysexin_MIDIHdr2.lpData = (char *)MIDIinbuf2;
					sysexin_MIDIHdr2.dwBufferLength = sizeof(MIDIinbuf2);
					sysexin_MIDIHdr2.dwFlags = 0;
					res=midiInPrepareHeader(hMIDIIn, &sysexin_MIDIHdr2, sizeof(MIDIHDR));

					maingui->MessageMMError(FullName(),"MIDI Input Device (midiInPrepareHeader 2)",res);

					if(res==MMSYSERR_NOERROR)
					{
						// Queue MIDI input buffer
						res = midiInAddBuffer(hMIDIIn, &sysexin_MIDIHdr1, sizeof(MIDIHDR));
						maingui->MessageMMError(FullName(),"midiInAddBuffer1",res);

						if(res!=MMSYSERR_NOERROR)
						{
							midiInUnprepareHeader(hMIDIIn, &sysexin_MIDIHdr1, sizeof(MIDIHDR));
							midiInUnprepareHeader(hMIDIIn, &sysexin_MIDIHdr2, sizeof(MIDIHDR));

							res=midiInClose(hMIDIIn);
							hMIDIIn=0;
							maingui->MessageMMError(FullName(),"midiInClose PH1+2 Add1",res);
						}
						else
						{
							res = midiInAddBuffer(hMIDIIn, &sysexin_MIDIHdr2, sizeof(MIDIHDR));
							maingui->MessageMMError(FullName(),"midiInAddBuffer2",res);

							if(res!=MMSYSERR_NOERROR)
							{
								midiInUnprepareHeader(hMIDIIn, &sysexin_MIDIHdr1, sizeof(MIDIHDR));
								midiInUnprepareHeader(hMIDIIn, &sysexin_MIDIHdr2, sizeof(MIDIHDR));

								res=midiInClose(hMIDIIn);
								hMIDIIn=0;
								maingui->MessageMMError(FullName(),"midiInClose PH1+2 Add2",res);
							}
							else
							{
								deviceinit=true;

								// Start MIDI Input
								res=MMSYSERR_ERROR;

								try
								{
									res=midiInStart(hMIDIIn);
								}

								catch(...)
								{
								}

								maingui->MessageMMError(FullName(),"SIT midiInStart",res);
							}
						}
					}
					else
					{
						midiInUnprepareHeader(hMIDIIn, &sysexin_MIDIHdr1, sizeof(MIDIHDR));

						MMRESULT res=midiInClose(hMIDIIn);
						hMIDIIn=0;
						maingui->MessageMMError(FullName(),"midiInClose PH2",res);
					}
				}
				else
				{
					MMRESULT res=midiInClose(hMIDIIn);
					hMIDIIn=0;
					maingui->MessageMMError(FullName(),"midiInClose PH1",res);
				}
			}				
#endif
		}

		if(deviceinit==true)
			StartInputTime();
	}
}

MIDIInputDevice::MIDIInputDevice()
{
	deviceinit=false;
	//timerstarted=false;

	runningstatus=0;
	inputwritebuffercounter=inputreadbuffercounter=0;
	MIDIclocktempo=0;
	inputMIDIclockcounter=0;
	MIDIclockraster=-1; // no quant

	minMIDIclockdifference=1;

	checksysex=true;

	userinfo[0]=0; // No User Info
	fullname=0;

	tempobuffercounter=0;
	
	indesongposition=0;
	newsongpositionset=false;

	synccounter=0; // Sync Events Receive

	monitor_syscounter=
		monitor_eventcounter=0; // no output
	displaynoteoff_monitor=false;

	name=initname=0;
	incomingstream=0;
	incomingstreamlength=0;

#ifdef WIN32
	hMIDIIn=0;
#endif
}

void MIDIInputDevice::CreateFullName()
{
	if(fullname)delete fullname;

	if(strlen(userinfo)>0)
		fullname=mainvar->GenerateString(name,"<",userinfo);
	else
		fullname=mainvar->GenerateString(name);
}

void MIDIInputDevice::CloseInputDevice()
{
	if(fullname)delete fullname;
	fullname=0;

	if(name)delete name;
	name=0;

	if(initname)delete initname;
	initname=0;

	deviceinit=false;
	StopMIDIDevice();
}

void MIDIInputDevice::StartInputTime()
{
	if(deviceinit==true)
	{
#ifdef OLDIE
		if(type==OS_MIDIINTERFACE)
		{
#ifdef WIN32
			if(hMIDIIn)
			{
				if(timerstarted==true)
				{
					MMRESULT res=MMSYSERR_ERROR;

					try
					{
						res=midiInStop(hMIDIIn);
					}

					catch(...)
					{
					}

					maingui->MessageMMError(FullName(),"SIT midiInStop",res);
					timerstarted=false;
				}

				MMRESULT res=MMSYSERR_ERROR;

				try
				{
					res=midiInStart(hMIDIIn);
				}

				catch(...)
				{
				}

				maingui->MessageMMError(FullName(),"SIT midiInStart",res);

				if(res==MMSYSERR_NOERROR)
					timerstarted=true;
			}
#endif
		}
#endif

	}

	inputMIDIclockcounter=0; // Reset MIDI Input Clock

	//LockInputBufferCounter();
	//recordbuffercounter=inputbuffercounter; // Cut Old Events
	//UnLockInputBufferCounter();
}

void MIDIInputDevice::StopMIDIDevice()
{
	if(type==OS_MIDIINTERFACE)
	{
#ifdef WIN32
		if(hMIDIIn)
		{
			MMRESULT res=MMSYSERR_ERROR;

			try
			{
				res=midiInStop(hMIDIIn);
			}

			catch(...)
			{
			}

			maingui->MessageMMError(FullName(),"SMD midiInStop",res);

			sysexin_MIDIHdr1.dwFlags=sysexin_MIDIHdr2.dwFlags=MHDR_DONE; // Set Done Flag , else midiInReset endless Loop !
			sysexin_MIDIHdr1.dwFlags=sysexin_MIDIHdr2.dwFlags=MHDR_DONE; // Set Done Flag , else midiInReset endless Loop !

			res=MMSYSERR_ERROR;

			try{
				res=midiInReset(hMIDIIn);
			}

			catch(...)
			{
			}

			maingui->MessageMMError(FullName(),"SMD midiInReset",res);

			res=MMSYSERR_ERROR;
			do
			{
				try
				{
					res=midiInUnprepareHeader(hMIDIIn, &sysexin_MIDIHdr1, sizeof(MIDIHDR));
				}

				catch(...)
				{
					break;
				}

				Sleep(10);
			}while(res==MIDIERR_STILLPLAYING);

			maingui->MessageMMError(FullName(),"SMD midiInUnprepareHeader1",res);

			res=MMSYSERR_ERROR;
			do
			{
				try{
					res=midiInUnprepareHeader(hMIDIIn, &sysexin_MIDIHdr2, sizeof(MIDIHDR));
				}

				catch(...)
				{
					break;
				}

				Sleep(10);
			}while(res==MIDIERR_STILLPLAYING);

			maingui->MessageMMError(FullName(),"SMD midiInUnprepareHeader3",res);

			res=MMSYSERR_ERROR;
			do
			{
				try
				{
					res=midiInClose(hMIDIIn);
				}

				catch(...)
				{
					break;
				}

				Sleep(10);

			}while(res==MIDIERR_STILLPLAYING);

			maingui->MessageMMError(FullName(),"SMD midiInClose",res);

			hMIDIIn=0;

			if(incomingstream)
				delete incomingstream;

			incomingstream=0;

		}
#endif
	}
}

