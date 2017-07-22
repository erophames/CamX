#include "editor_cpu.h"
#include "gui.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "songmain.h"
#include "languagefiles.h"
#include "audioproc.h"
#include "MIDItimer.h"
#include "object_song.h"
#include "object_track.h"
#include "audiohardwarechannel.h"

enum RecEditorGID
{
	GADGETID_CPUINFO,
	GADGETID_CPU,
	GADGETID_CPUMAXINFO,
	GADGETID_CPUMAX,
	GADGETID_HDINFO,
	GADGETID_HD,
	GADGETID_HDMAXINFO,
	GADGETID_HDMAX,
	GADGETID_SYNCINFO,
	GADGETID_SYNC,
	GADGETID_HDMEMINFO,
	GADGETID_HDMEM,
#ifdef DEBUG
	GADGETID_AUDIOINPUT
#endif

};

void Edit_CPU::ShowGFX()
{
	if(!mainaudio->GetActiveDevice())
		return;

	AudioDevice *device=mainaudio->GetActiveDevice();

	{
		// Current

		device->LockTimerCheck_Output();
		LONGLONG timeforrefill_systime=device->timeforrefill_systime;
		LONGLONG timeforrefill_maxsystime=device->timeforrefill_maxsystime;
		device->UnlockTimerCheck_Output();

		device->LockTimerCheck_Input();
		LONGLONG timeforaudioinpurefill_systime=device->timeforaudioinpurefill_systime;
		LONGLONG timeforaudioinputrefill_maxsystime=device->timeforaudioinputrefill_maxsystime;
		device->UnlockTimerCheck_Input();

		double c1=device->samplebufferms,c2=maintimer->ConvertSysTimeToMs(timeforrefill_systime+timeforaudioinpurefill_systime);

		c2/=c1;
		double dh=c2*100;

		char h2[NUMBERSTRINGLEN],*x=mainvar->ConvertDoubleToChar(dh,h2,3);

		// Max
		c1=device->samplebufferms; // ms
		c2=maintimer->ConvertSysTimeToMs(timeforrefill_maxsystime+timeforaudioinputrefill_maxsystime);

		c2/=c1;
		dh=c2*100;

		if(cpuusage)
		{
			char *ph=mainvar->GenerateString(x," %");
			if(ph)
			{
				cpuusage->ChangeButtonText(ph);
				delete ph;		
			}
		}

		x=mainvar->ConvertDoubleToChar(dh,h2,3);

		if(cpuusagemax)
		{
			char *ph=mainvar->GenerateString(x," %");
			if(ph)
			{
				cpuusagemax->ChangeButtonText(ph);
				delete ph;		
			}
		}
	}

	{
		mainaudiostreamproc->LockTimerCheck();
		double timeforrefill_maxms=mainaudiostreamproc->timeforrefill_maxms;
		double timeforrefill_ms=mainaudiostreamproc->timeforrefill_ms;
		mainaudiostreamproc->UnlockTimerCheck();

		double c2=timeforrefill_ms,c1=rafbuffersize[mainaudio->rafbuffersize_index]; // ms
		c2/=c1;
		double dh=c2*100;
		char h2[NUMBERSTRINGLEN],*x=mainvar->ConvertDoubleToChar(dh,h2,3);

		if(hdusage)
		{
			char *ph=mainvar->GenerateString(x," %");
			if(ph)
			{
				hdusage->ChangeButtonText(ph);
				delete ph;		
			}
		}

		// Max
		c2=timeforrefill_maxms;

		c2/=c1;

		dh=c2*100;

		x=mainvar->ConvertDoubleToChar(dh,h2,3);

		if(hdusagemax)
		{
			char *ph=mainvar->GenerateString(x," %");
			if(ph)
			{
				hdusagemax->ChangeButtonText(ph);
				delete ph;		
			}
		}
	}

	// Audio HD Mem

	if(mainaudio->GetActiveDevice() && sync)
	{
		outofsync=mainaudio->GetActiveDevice()->deviceoutofsync;

		if(outofsync==true)
		{
			sync->SetColourNoDraw(COLOUR_WHITE,COLOUR_ERROR);
			sync->ChangeButtonText("Error:Audio Sync");
		}
		else
		{
			sync->SetColourNoDraw(COLOUR_GADGETTEXT,COLOUR_GADGETBACKGROUNDSYSTEM);
			sync->ChangeButtonText("Okay");
		}
	}

	if(hdmem)
	{
		if(mainvar->GetActiveSong())
		{
			//guibuffer->guiDrawRect(2,y,width-2,y+maingui->GetFontSizeY_Sub(),COLOUR_WHITE);

			unsigned __int64 audiotrackschannels=0;
			Seq_Track *t=mainvar->GetActiveSong()->FirstTrack();
			while (t)
			{
				if(t->recordtracktype==TRACKTYPE_AUDIO && t->record==true)
				{
					if(t->io.in_vchannel)
						audiotrackschannels+=t->io.in_vchannel->channels;
				}

				t=t->NextTrack();
			}

			bool recok;
			unsigned __int64 freerecmemoryondisk=mainaudio->GetFreeRecordingMemory(mainvar->GetActiveSong(),&recok); // Bytes if recok==true

			if(recok==true)
			{
				if(audiotrackschannels==0)
				{
					hdmem->ChangeButtonText(Cxs[CXS_NOSONGAUDIORECORDING]);
				}
				else
				{
					//freerecmemoryondisk/=1024; // ->Kbytes
					unsigned __int64 h=mainaudio->GetGlobalSampleRate();
					unsigned __int64 h2=mainaudio->GetActiveDevice()->FirstInputChannel()->sizeofsample;

					h*=h2; // Bytes pro Channel/sek
					h*=audiotrackschannels; // Bytes all Channels
					freerecmemoryondisk/=h; // seconds

					int hour=(int)(freerecmemoryondisk/3600);
					freerecmemoryondisk-=hour*3600;

					int min=(int)(freerecmemoryondisk/60);
					char hourstr[NUMBERSTRINGLEN],minstr[NUMBERSTRINGLEN];

					if(char *hs=mainvar->GenerateString(mainvar->ConvertIntToChar(hour,hourstr),"h:",mainvar->ConvertIntToChar(min,minstr),"min"))
					{
						hdmem->ChangeButtonText(hs);
						delete hs;
					}
				}
			}
			else
				hdmem->ChangeButtonText(Cxs[CXS_NOSONGAUDIORECORDING]);
		}
		else
		{
			hdmem->ChangeButtonText(Cxs[CXS_NOSONG]);

		}

	}

#ifdef DEBUG
	if(audioinput)
	{
		LONGLONG tc=mainaudio->GetActiveDevice()->timetoconvertinputdata;
		double c1=maintimer->ConvertSysTimeToMs(mainaudio->GetActiveDevice()->timetoconvertinputdata);

		char help[32];
		char *hs=mainvar->GenerateString(mainvar->ConvertDoubleToChar(c1,help,3));
		if(hs)
		{
			audioinput->ChangeButtonText(hs);
			delete hs;
		}
	}
#endif

}

void Edit_CPU::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_CPUMAXINFO:
		{
			if(mainaudio->GetActiveDevice())
			{
				mainaudio->GetActiveDevice()->LockTimerCheck_Output();
				mainaudio->GetActiveDevice()->timeforrefill_maxsystime=0;
				mainaudio->GetActiveDevice()->UnlockTimerCheck_Output();
			}
		}
		break;

	case GADGETID_HDMAXINFO:
		{
			mainaudiostreamproc->LockTimerCheck();
			mainaudiostreamproc->timeforrefill_maxms=0;
			mainaudiostreamproc->UnlockTimerCheck();
		}
		break;
	}
}

 Edit_CPU::Edit_CPU()
	{
		editorid=EDITORTYPE_CPU;
		editorname=Cxs[CXS_CPUUSAGE_EDITOR];
		dialogstyle=true;
		ondesktop=true;
		outofsync=false;
	}

void Edit_CPU::Init()
{
	glist.SelectForm(0,0);

	glist.AddButton(-1,-1,-1,-1,"CPU",GADGETID_CPUINFO,MODE_LEFTTOMID|MODE_TEXTCENTER|MODE_ADDDPOINT);
	glist.AddLX();
	cpuusage=glist.AddButton(-1,-1,-1,-1,GADGETID_CPU,MODE_MIDTORIGHT|MODE_INFO);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"[CPU Max]",GADGETID_CPUMAXINFO,MODE_LEFTTOMID|MODE_TEXTCENTER|MODE_ADDDPOINT,"Reset");
	glist.AddLX();
	cpuusagemax=glist.AddButton(-1,-1,-1,-1,GADGETID_CPUMAX,MODE_MIDTORIGHT|MODE_INFO);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"HD",GADGETID_HDINFO,MODE_LEFTTOMID|MODE_TEXTCENTER|MODE_ADDDPOINT);
	glist.AddLX();
	hdusage=glist.AddButton(-1,-1,-1,-1,GADGETID_HD,MODE_MIDTORIGHT|MODE_INFO);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"[HD Max]",GADGETID_HDMAXINFO,MODE_LEFTTOMID|MODE_TEXTCENTER|MODE_ADDDPOINT,"Reset");
	glist.AddLX();
	hdusagemax=glist.AddButton(-1,-1,-1,-1,GADGETID_HDMAX,MODE_MIDTORIGHT|MODE_INFO);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"Audio Sync",GADGETID_SYNCINFO,MODE_LEFTTOMID|MODE_TEXTCENTER|MODE_ADDDPOINT);
	glist.AddLX();
	sync=glist.AddButton(-1,-1,-1,-1,GADGETID_SYNC,MODE_MIDTORIGHT|MODE_INFO);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,Cxs[CXS_AUDIORECORDINGTIME],GADGETID_HDMEMINFO,MODE_LEFTTOMID|MODE_TEXTCENTER|MODE_ADDDPOINT);
	glist.AddLX();
	hdmem=glist.AddButton(-1,-1,-1,-1,GADGETID_HDMEM,MODE_MIDTORIGHT|MODE_INFO);
	glist.Return();

#ifdef DEBUG
	audioinput=glist.AddButton(-1,-1,-1,-1,GADGETID_AUDIOINPUT,MODE_MIDTORIGHT|MODE_INFO);
	glist.Return();
#endif

}

void Edit_CPU::RefreshRealtime_Slow()
{
	ShowGFX();
}