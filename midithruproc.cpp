#include "defines.h"
#include "songmain.h"

#ifdef WIN32
#include <mmsystem.h>
#endif

#include "MIDIthruproc.h"
#include "MIDIoutdevice.h"
#include "object_track.h"
#include "semapores.h"
#include "object_song.h"
#include "MIDIhardware.h"
#include "MIDIinproc.h"
#include "audiochannel.h"
#include "MIDIprocessor.h"


MIDIThruOff *MIDIThruThread::AddThruOff(MIDIOutputDevice *odev,MIDIInputDevice *idev,Seq_Track *track,OSTART start,UBYTE status,UBYTE key,UBYTE outstatus,UBYTE outkey){

	if(MIDIThruOff *thru=new MIDIThruOff){

		thru->key=key;
		thru->start=start;
		thru->status=status;
		thru->track=track;
		thru->outstatus=NOTEOFF|(outstatus&0x0F);
		thru->outkey=outkey;
		thru->velocityoff=0;
		thru->indev=idev;
		thru->outdev=odev; // <MIDI Output Device>

		LockThruOffs();
		offs.AddEndO(thru);
		UnLockThruOffs();

		return thru;
	}

	return 0;
}


MIDIThruOff *MIDIThruThread::AddThruOffToPlugin(InsertAudioEffect *oaudioeffect,MIDIInputDevice *idev,Seq_Track *track,OSTART start,UBYTE status,UBYTE key,UBYTE outstatus,UBYTE outkey){
	if(MIDIThruOff *thru=new MIDIThruOff){

		thru->key=key;
		thru->start=start;
		thru->status=status;
		thru->track=track;
		thru->outstatus=NOTEOFF|(outstatus&0x0F);
		thru->outkey=outkey;
		thru->velocityoff=0;
		thru->indev=idev;
		thru->outaudio=oaudioeffect; // < Audio Effect Instrument >

		offs.AddEndO(thru);
		return thru;
	}

	return 0;
}

bool MIDIThruThread::FindOpenKey(int key,OpenNoteFilter *filter)
{
	LockThruOffs();
	MIDIThruOff *off=FirstThruOff();
	while(off)
	{
		if((off->key==key || key==-1) && ((!filter) || filter->CheckNote(off->status,key)==true))
		{
			UnLockThruOffs();
			return true;
		}

		off=off->NextThruOff();
	}
	UnLockThruOffs();

	return false;
}


void MIDIThruThread::DoThruOff(MIDIThruOff *off)
{
	// MIDI =
	if(off->outdev)
	{
		off->outdev->sendNoteOff(NOTEOFF|(off->outstatus&0x0F),off->outkey,off->velocityoff);
	}
	else
	{
		// Audio ?
		if(off->outaudio)
		{
			off->outaudio->audioeffect->Do_TriggerEvent(0,NOTEOFF|(off->outstatus&0x0F),off->outkey,off->velocityoff,TRIGGER_FORCE);
		}
	}

	//mainMIDI->monitor.CloseThru(off->outkey);
}

MIDIThruOff* MIDIThruThread::DeleteThruOff(MIDIThruOff *e,bool send)
{
	if(send==true)DoThruOff(e);
	return (MIDIThruOff *)offs.RemoveO(e); // delete noteoff
}

int MIDIThruThread::RemoveOffsFromEffect(InsertAudioEffect *oaudioeffect)
{
	int c=0;

	LockThruOffs();

	MIDIThruOff *re=FirstThruOff();

	while(re)
	{
		if(re->outaudio==oaudioeffect){
			c++;
			re=DeleteThruOff(re,true);
		}
		else
			re=re->NextThruOff();
	}

	UnLockThruOffs();

	return c;
}

void MIDIThruThread::DeleteAllThruOffsTrack(Seq_Track *track)
{
	if(track){	
		LockThruOffs();

		MIDIThruOff *re=FirstThruOff();

		while(re){
			if(re->track==track)
				re=DeleteThruOff(re,true);
			else
				re=re->NextThruOff();
		}

		UnLockThruOffs();
	}
}

void MIDIThruThread::DeleteAllThruOff()
{
	LockThruOffs();
	MIDIThruOff *re=FirstThruOff();
	while(re)re=DeleteThruOff(re,true);
	UnLockThruOffs();
}

void MIDIThruThread::DeleteThruOffs(Seq_Track *track,MIDIInputDevice *indev,UBYTE instatus,UBYTE key,UBYTE veloff)
{
	LockThruOffs();

	MIDIThruOff *off=FirstThruOff();

	while(off) // Find Offs in List
	{
		if((off->status&0x0F)==(instatus&0x0F) && // Same Input Channel 0-15?
			off->indev==indev &&  // Same MIDI Input Device
			off->key==key // Same Input Key
			)
		{
			//TRACE ("Delete Thru Off Output\n");

			off->velocityoff=veloff; // Velocity Off
			off=DeleteThruOff(off,true); // Send and Delete
		}
		else
			off=off->NextThruOff();
	}

	UnLockThruOffs();

	if(track && track->GetProcessor())
		track->GetProcessor()->DeleteThruOffs(track,indev,instatus,key,veloff);
}


void MIDIThruThread::ExecuteThruEvent(Seq_Song *song,NewEventData *ned,Seq_Track *intrack,Seq_Event *e)
{
	if((!song) || (!e) || song->mastering==true)return;

	if(Seq_Event *clone=(Seq_Event *)e->Clone(song))
	{
		Seq_MIDIRouting_InputDevice *inputdevice=ned->fromdev?song->inputrouting.FindInputDevice(ned->fromdev):0;

		if((!ned->fromdev) || inputdevice)
		{
			Seq_Track *t=song->FirstTrack();

			while(t){

				if(t->IsEditAble()==true)
				{
					// Track MIDI Input Peak
					if(
						(t->record==true || t->t_trackeffects.MIDIthru==true) && 
						t->recordtracktype==TRACKTYPE_MIDI && 
						t->CheckMIDIInputEvent(ned->fromdev,e)==true
						)
					{
						t->MIDInputimpulse=100;

						switch(e->GetStatus())
						{
						case NOTEON:
							{
								Note *note=(Note *)e;

								double h=127;
								double k=note->velocity;

								k/=h;

								k*=100;

								t->MIDInputimpulse_data=(int)k;
							}
							break;

						default:
							break;
						}
					}

					// in -> Input effects
					if(t->io.audioinputeffects.FirstActiveEffectWithMIDIInput())
					{
						bool checkfocustrack=t->GetFX()->usealwaysthru==true?false:true;

						if(t->GetFX()->userouting==false || (!ned->fromdev) || inputdevice->CheckEvent(t,e,checkfocustrack)==true)
						{
							//  input
							MIDIProcessor processor(song,t);

							processor.indevice=ned->fromdev; // Set Input Device
							processor.inplugin=ned->fromplugin;
							processor.inevent=clone;
							processor.flag=PROCESSORFLAG_DOSENDTOTRACKWITHALWAYSTHRU;

							// Create Output Events
							processor.EventInput(0,clone,0);

							// Mark as Thru event
							Seq_Event *pe=processor.FirstProcessorEvent();
							while(pe)
							{
								pe->SetMIDIImpulse(t);
								pe->flag|=EVENTFLAG_THRUEVENT;
								pe=pe->NextEvent();
							}

							processor.SendProcessorEvents(ned,MIDIProcessor::SPE_THRUCHECK|MIDIProcessor::SPE_INPUT,ned->fromplugin?ned->deltaframes:0,ned->fromplugin?ned->flag:0);
						}
					}

					if(t->t_trackeffects.MIDIthru==true) // In -> thru -> out
					{
						bool checkfocustrack=t->GetFX()->usealwaysthru==true?false:true;

						if(
							(checkfocustrack==false || t==song->GetFocusTrack()) &&
							(t->GetFX()->userouting==false || (!ned->fromdev) || inputdevice->CheckEvent(t,e,checkfocustrack)==true)
							)
						{
							// Thru Output
							MIDIProcessor processor(song,t);

							processor.indevice=ned->fromdev; // Set Input Device
							processor.inplugin=ned->fromplugin;
							processor.inevent=clone;
							processor.flag=PROCESSORFLAG_DOSENDTOTRACKWITHALWAYSTHRU;

							// Create Output Events
							processor.EventInput(0,clone,0);

							// Mark as Thru event
							Seq_Event *pe=processor.FirstProcessorEvent();
							while(pe)
							{
								pe->SetMIDIImpulse(t);
								pe->flag|=EVENTFLAG_THRUEVENT;
								pe=pe->NextEvent();
							}

							processor.SendProcessorEvents(ned,MIDIProcessor::SPE_THRUCHECK,ned->fromplugin?ned->deltaframes:0,ned->fromplugin?ned->flag:0);
						}
					}
				}

				t=t->NextTrack_NoUILock();
			}// 
		}

		clone->Delete(true);
	}
}

void MIDIThruThread::CheckNewEvent(Seq_Song *song,NewEventData *ned,int index)
{
	Seq_Track *atrack=song?song->GetFocusTrack():0;

	UBYTE status=ned->status;

	if((status&0xF0)==NOTEON && ned->byte2==0) // Note On Velo=0 -> Note Off
		// Velo 0-> NoteOff
		status=NOTEOFF|(status&0x0F);

	switch(status&0xF0)
	{
	case NOTEON:
		{
			processornote[index].status=status;
			processornote[index].key=ned->byte1;
			processornote[index].velocity=ned->byte2;
			ExecuteThruEvent(song,ned,atrack,&processornote[index]);
		}
		break;

	case NOTEOFF:
		{
			// Find and Send Thru Offs
			DeleteThruOffs(atrack,ned->fromdev,status,ned->byte1,ned->byte2);
		}
		break;

	case POLYPRESSURE:
		{
			processorppressure[index].status=status;
			processorppressure[index].key=ned->byte1;
			processorppressure[index].pressure=ned->byte2;
			ExecuteThruEvent(song,ned,atrack,&processorppressure[index]);
		}
		break;

	case CONTROLCHANGE:
		{
			if(mainMIDI->CheckIfChannelModeMessage(status,ned->byte1)==false)
			{
				processorcontrol[index].status=status;
				processorcontrol[index].controller=ned->byte1;
				processorcontrol[index].value=ned->byte2;

				ExecuteThruEvent(song,ned,atrack,&processorcontrol[index]);
			}
		}
		break;

	case PROGRAMCHANGE:
		{
			processorprogram[index].status=status;
			processorprogram[index].program=ned->byte1;
			ExecuteThruEvent(song,ned,atrack,&processorprogram[index]);
		}
		break;

	case CHANNELPRESSURE:
		{
			processorcpressure[index].status=status;
			processorcpressure[index].pressure=ned->byte1;
			ExecuteThruEvent(song,ned,atrack,&processorcpressure[index]);
		}
		break;

	case PITCHBEND:
		{
			processorpitch[index].status=status;
			processorpitch[index].lsb=ned->byte1;
			processorpitch[index].msb=ned->byte2;
			ExecuteThruEvent(song,ned,atrack,&processorpitch[index]);
		}
		break;

	case SYSEX:
		{
			// Check SysEx Thru
		}
		break;
	}//switch

}

void MIDIThruThread::WaitLoop()
{
	while(IsExit()==false)
	{
		// Wait for incoming Signal		
		WaitSignal();

		if(IsExit()==true)
			break;

		Lock();

		Seq_Song *asong=mainvar->GetActiveSong();

		LockInput();
		NewEventData *input=(NewEventData*)thruevents.GetRoot();
		UnlockInput();

		while(input){

			if(asong)
				CheckNewEvent(asong,input,THRUINDEX_MIDI);

			input->FreeData();

			LockInput();
			input=(NewEventData *)thruevents.RemoveO(input);
			UnlockInput();

		} // while

		Unlock();
	}

	LockInput();
	NewEventData *input=(NewEventData*)thruevents.GetRoot();
	while(input){
		input->FreeData();
		input=(NewEventData *)thruevents.RemoveO(input);
	}
	UnlockInput();

	// Exit
	DeleteAllThruOff();
	ThreadGone();
}

PTHREAD_START_ROUTINE MIDIThruThread::MIDIThruThreadFunc(LPVOID pParam) // MIDI Input Controll Thread, writes MIDI Buffer to Track
{
	MIDIThruThread *thruproc=(MIDIThruThread *)pParam;
	thruproc->WaitLoop();
	return 0;
}

int MIDIThruThread::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MIDIThruThreadFunc,(LPVOID)this, 0,0);

	if(!ThreadHandle)
		error++;

	if(ThreadHandle)
		SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL); // Best Priority
#endif

	return error;
}

