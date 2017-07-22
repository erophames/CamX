#include "defines.h"
#include "songmain.h"
#include "MIDIoutproc.h"
#include "MIDIhardware.h"
#include "semapores.h"
#include "settings.h"
#include "audiohardware.h"
#include "object_song.h"
#include "MIDIPattern.h"
#include "object_track.h"
#include "MIDItimer.h"
#include "audiodevice.h"
#include "MIDIprocessor.h"
#include "object_project.h"
#include <math.h>
#include "gui.h"
#include "camxfile.h"

void mainMIDIBase::SendMIDIStop(Seq_Song *song,int flag)
{
	if(!song)
		return;

	if(song->MIDIsync.sync==SYNC_INTERN)
	{
		//if((flag&NO_SYSTEMLOCK)==0)
		mainMIDIalarmthread->Lock();

		MIDImtcproc->startmtc=false;

		//if((flag&NO_SYSTEMLOCK)==0)
		mainMIDIalarmthread->Unlock();

		{
			//	if((flag&NO_SYSTEMLOCK)==0)
			mainMIDIalarmthread->Lock();

			if(song->MIDIsync.sendmc==true)
			{
				for(int i=0;i<MAXMIDIPORTS;i++)
				{
					if(MIDIoutports[i].visible==true && 
						MIDIoutports[i].outputdevice && 
						MIDIoutports[i].outputdevice->lockMIDIstartstop==false && 
						MIDIoutports[i].sendsync==true)
					{
						MIDIoutports[i].outputdevice->sendSmallDataRealtime(MIDIREALTIME_STOP);
						MIDIoutports[i].outputdevice->lockMIDIstartstop=true;
					}
				}

				// Unlock MIDI Control
				MIDIOutputDevice *mo=FirstMIDIOutputDevice();

				while(mo)
				{
					mo->lockMIDIstartstop=false;
					mo=mo->NextOutputDevice();
				}
			}

			// Send NAK

			if(song->MIDIsync.sendmtc==true)
			{
				for(int i=0;i<MAXMIDIPORTS;i++)
				{
					if(MIDIoutports[i].visible==true && MIDIoutports[i].outputdevice && MIDIoutports[i].outputdevice->lockmtc==false && MIDIoutports[i].sendmtc==true)
					{
						UBYTE nak[6];

						nak[0]=0xF0;

						switch(song->project->standardsmpte)
						{
						case Seq_Pos::POSMODE_SMPTE_24:
							nak[1]=0x00;
							break;

						case Seq_Pos::POSMODE_SMPTE_25:
							nak[1]=0x01;
							break;

						case Seq_Pos::POSMODE_SMPTE_2997:
							nak[1]=0x02;
							break;

						case Seq_Pos::POSMODE_SMPTE_30:
							nak[1]=0x03;
							break;

						default:
							goto nosend;
							break;
						}

						nak[2]=0x7e;
						nak[3]=0x7e;
						nak[4]=0xF7;

						MIDIoutports[i].outputdevice->sendBigData(nak,5);
						MIDIoutports[i].outputdevice->lockmtc=true;
					}

nosend:
					// Unlock MTC
					MIDIOutputDevice *mo=FirstMIDIOutputDevice();

					while(mo)
					{
						mo->lockmtc=false;
						mo=mo->NextOutputDevice();
					}
				}
			}

			//if((flag&NO_SYSTEMLOCK)==0)
			mainMIDIalarmthread->Unlock();
		}
	}
}

void mainMIDIBase::SendSongPosition(Seq_Song *song,OSTART position,bool force)
{
	if(song && song->MIDIsync.sync==SYNC_INTERN )
	{
		if(song->MIDIsync.sendmc==true)
		{
			for(int i=0;i<MAXMIDIPORTS;i++)
			{
				if(MIDIoutports[i].visible==true && MIDIoutports[i].outputdevice && MIDIoutports[i].outputdevice->lockMIDIsongposition==false && MIDIoutports[i].sendsync==true)
				{
					MIDIoutports[i].outputdevice->sendSongPosition(position);
					MIDIoutports[i].outputdevice->lockMIDIsongposition=true;
				}
			}
			// Unlock MIDI Control
			MIDIOutputDevice *mo=FirstMIDIOutputDevice();

			while(mo)
			{
				mo->lockMIDIsongposition=false;
				mo=mo->NextOutputDevice();
			}
		}

		for(int i=0;i<MAXMIDIPORTS;i++)
		{
			if(MIDIoutports[i].visible==true && MIDIoutports[i].outputdevice && MIDIoutports[i].outputdevice->lockmtc==false && MIDIoutports[i].sendmtc==true)
			{
				MIDIOutputDevice *dev=MIDIoutports[i].outputdevice;

				dev->lockmtc=true;

				// MTC Full Message
				UBYTE fm[15];

				fm[0]=0xF0;
				fm[1]=0x7F;
				fm[2]=0x7F;
				fm[3]=0x01;
				fm[4]=0x01;

				bool ok=true;
				LONGLONG h=0,m=0,s=0,f=0,qf=0;

				dev->mtcqf.ticks=position;

				// Type+Hour
				Seq_Pos possmpte(song->project->standardsmpte);
				possmpte.song=song;
				possmpte.offset=0;

				song->timetrack.ConvertTicksToPos(dev->mtcqf.ticks,&possmpte);

				switch(song->project->standardsmpte)
				{
				case Seq_Pos::POSMODE_SMPTE_24:
					{
						fm[5]=0;

						h=possmpte.pos[0];
						m=possmpte.pos[1];
						s=possmpte.pos[2];
						f=possmpte.pos[3];
						qf=possmpte.pos[4];
					}
					break;

				case Seq_Pos::POSMODE_SMPTE_25:
					{
						fm[5]=0x20;

						h=possmpte.pos[0];
						m=possmpte.pos[1];
						s=possmpte.pos[2];
						f=possmpte.pos[3];
						qf=possmpte.pos[4];
					}
					break;

				case Seq_Pos::POSMODE_SMPTE_30df:
				case Seq_Pos::POSMODE_SMPTE_2997:
					{
						fm[5]=0x40;

						h=possmpte.pos[0];
						m=possmpte.pos[1];
						s=possmpte.pos[2];
						f=possmpte.pos[3];
						qf=possmpte.pos[4];
					}
					break;

				case Seq_Pos::POSMODE_SMPTE_30:
					{
						fm[5]=0x60;

						h=possmpte.pos[0];
						m=possmpte.pos[1];
						s=possmpte.pos[2];
						f=possmpte.pos[3];
						qf=possmpte.pos[4];
					}
					break;

				default:
					goto nosend;
					break;
				}

				if(h>23)
					h=23;

				fm[5]|=(UBYTE)h; // Type+Hour
				fm[6]=(UBYTE)m;
				fm[7]=(UBYTE)s;
				fm[8]=(UBYTE)f;
				fm[9]=0xF7;

				dev->mtcqf.nak=false;
				dev->mtcqf.set=true;

				dev->mtcqf.hour=h;
				dev->mtcqf.min=m;
				dev->mtcqf.sek=s;
				dev->mtcqf.frame=f;
				dev->mtcqf.qframe=qf; // MTC Full Message QF:0

				dev->sendBigData(fm,10);

			}
			else 
				if(MIDIoutports[i].outputdevice)
					MIDIoutports[i].outputdevice->mtcqf.set=false;
		}

nosend:
		// Unlock MTC
		MIDIOutputDevice *mo=FirstMIDIOutputDevice();

		while(mo)
		{
			mo->lockmtc=false;
			mo=mo->NextOutputDevice();
		}
	}
}

void Seq_Song::SendMIDIOutPlaybackEventNoEffects(Seq_Track *track,MIDIPattern *pattern,Seq_Event *e,int createflag)
{
	if(mainMIDI->MIDIoutactive==true)
	{	
		CheckMIDIAlarms(MIDI_samplestartposition);

		Seq_Group_MIDIOutPointer *dev=track->GetMIDIOut()->FirstDevice();

		while(dev){

			e->SendToDevicePlayback(dev->GetDevice(),this,track,0,e,createflag);
			//e->SendTriggerImpulse();
			dev=dev->NextGroup();
		}

		//MIDIPlaybackCall_SendRealtimeEvent(songposition);
	} 
}

void Seq_Song::SendMIDIOutPlaybackEvent(Seq_Track *track,MIDIPattern *pattern,Seq_Event *e,LONGLONG playbacksampleposition,int createflag)
{		
	if(mainMIDI->MIDIoutactive==true)
	{	
		CheckMIDIAlarms(playbacksampleposition);

		if(!pattern) // No Effects- PRepair Events etc...
		{
			Seq_Group_MIDIOutPointer *dev=track->GetMIDIOut()->FirstDevice();

			while(dev){
				e->SendToDevicePlayback(dev->GetDevice(),this,track,0,e,createflag);
				dev=dev->NextGroup();
			}
		}
		else
		{
			MIDIProcessor processor(this,track);

			processor.EventInput(pattern,e,e->GetEventStart(pattern)); // ->> fill+create Processor Events

			if(Seq_Event *oevent=processor.FirstProcessorEvent())
			{
				bool cansendpattern=pattern->CheckIfPlaybackIsAble(),
					cansendtrack=pattern->track->CheckIfPlaybackIsAble();

				while(oevent)
				{
					if(oevent->CheckIfPlaybackIsAble()==false || // Check only Note == MUTE ?
						(cansendpattern==true && cansendtrack==true)
						)
					{
						if(cansendpattern==true && cansendtrack==true)
						{
							if(mainMIDI->MIDI_outputimpulse<=50)
								mainMIDI->MIDI_outputimpulse=100;

							oevent->SetMIDIImpulse(track);
							//track->CheckMIDIOutput(oevent);
						}

						Seq_Group_MIDIOutPointer *dev=track->GetMIDIOut()->FirstDevice();

						while(dev){
							oevent->SendToDevicePlayback(dev->GetDevice(),this,track,pattern,e,createflag);
							dev=dev->NextGroup();
						}
					}

					oevent=processor.DeleteEvent(oevent);
				}
			}
		}
	} 
}

void mainMIDIBase::StopAllofEvent(Seq_Song *song,Seq_Event *e)
{
	for(int i=0;i<REALTIME_LISTS;i++){

		song->realtimeevents[i].Lock();
		RealtimeEvent *re=song->realtimeevents[i].FirstREvent();

		while(re){

			if(re->fromevent==e){
				re->SendQuick();
				re=song->realtimeevents[i].DeleteREvent(re);
			}
			else
				re=re->NextEvent();
		}
		song->realtimeevents[i].UnLock();
	}
}

void mainMIDIBase::StopAllofPattern(Seq_Song *song,Seq_Pattern *p)
{
	if(p->mediatype==MEDIATYPE_MIDI)
	{
		for(int i=0;i<REALTIME_LISTS;i++){

			song->realtimeevents[i].Lock();
			RealtimeEvent *re=song->realtimeevents[i].FirstREvent();

			while(re){

				if(p->CheckIfPatternOrFromClones(re->rte_frompattern)==true)
				{
					re->SendQuick();
					re=song->realtimeevents[i].DeleteREvent(re);
				}
				else
					re=re->NextEvent();
			}

			song->realtimeevents[i].UnLock();
		}
	}
}

void mainMIDIBase::StopAllofTrack(Seq_Track *t)
{	
	t->StopAll();

	// Childs
	Seq_Track *c=t->FirstChildTrack(),*lc=t->LastChildTrack();
	while(c!=lc)
	{
		c->StopAll();
		c=c->NextTrack();
	}
}

#ifdef OLDIE
void Seq_Song::StopAllExceptTrack(Seq_Track *t) // After Solo
{
	if(!t)return;

	for(int i=0;i<REALTIME_LISTS;i++){

		realtimeevents[i].Lock();

		RealtimeEvent *re=realtimeevents[i].FirstREvent();

		while(re){
			if(re->fromtrack!=t){
				re->SendQuick();
				re=realtimeevents[i].DeleteREvent(re);
			}
			else
				re=re->NextEvent();
		}

		realtimeevents[i].UnLock();
	}
}
#endif

void Seq_Song::StartMIDI()
{
	// Send MIDI Stop+SPP ----------------------------------------------------------------

	if(MIDIsync.sync==SYNC_INTERN && MIDIsync.sendmc==true)
	{
		/*
		// Send MIDI Song Position
		for(int i=0;i<MAXMIDIPORTS;i++)
		{
		if(mainMIDI->MIDIoutports[i].visible==true && mainMIDI->MIDIoutports[i].outputdevice && mainMIDI->MIDIoutports[i].outputdevice->lockMIDIsongposition==false && mainMIDI->MIDIoutports[i].sendsync==true)
		{
		mainMIDI->MIDIoutports[i].outputdevice->sendSongPosition(songstartposition);
		mainMIDI->MIDIoutports[i].outputdevice->lockMIDIsongposition=true;
		}
		}
		*/

		// Send MIDI Start/Continue ----------------------------------------------------------------
		for(int i=0;i<MAXMIDIPORTS;i++)
		{
			if(mainMIDI->MIDIoutports[i].visible==true && 
				mainMIDI->MIDIoutports[i].outputdevice && 
				mainMIDI->MIDIoutports[i].outputdevice->lockMIDIstartstop==false && 
				mainMIDI->MIDIoutports[i].sendsync==true)
			{
				mainMIDI->MIDIoutports[i].outputdevice->sendSmallDataRealtime(MIDIREALTIME_START /*songstartposition==0?MIDIREALTIME_START:MIDIREALTIME_CONTINUE*/);
				mainMIDI->MIDIoutports[i].outputdevice->lockMIDIstartstop=true;
			}
		}

		// Unlock MIDI Control
		MIDIOutputDevice *mo=mainMIDI->FirstMIDIOutputDevice();

		while(mo)
		{
			mo->lockMIDIstartstop=mo->lockMIDIsongposition=false;
			mo=mo->NextOutputDevice();
		}
	}

	status CLEARBIT STATUS_INIT;
	SetRunningFlag(songstartposition,false); // not started by Audio ?

	//mainthreadcontrol->Lock(CS_audioplayback);
	//status=Seq_Song::STATUS_SONGPLAYBACK_MIDI|flag; // Main Song Start
	//mainthreadcontrol->Unlock(CS_audioplayback);
	//if(status&STATUS_RECORD)
	//	startrecording=true;

	if(MIDIsync.sync==SYNC_INTERN)
	{
		for(int i=0;i<MAXMIDIPORTS;i++)
		{
			if(mainMIDI->MIDIoutports[i].sendmtc==true)
			{
				// Start MTC Thread
				MIDImtcproc->startmtc=true;
				MIDImtcproc->SetSignal();
				break;
			}
		}
	}

	// Reset Song MIDI Clock
	MIDIsync.out_nextclockposition=songstartposition+(SAMPLESPERBEAT/24);
	MIDIsync.out_nextclocksample=timetrack.ConvertTicksToTempoSamples(MIDIsync.out_nextclockposition);
}

int Control_InternStartMIDIProc::Callback(Seq_Song *song)
{
	// Start MIDI Playback
	//if(song->status&Seq_Song::STATUS_WAITPREMETRO){

	//TRACE ("Control_InternStartMIDIProc Call\n");
	//song->status |=Seq_Song::STATUS_RECORD;
	//song->status CLEARBIT (Seq_Song::STATUS_WAITPREMETRO|Seq_Song::STATUS_WAITMIDIPREMETROCANCELED);


	//mainMIDIalarmthread->SetSignal();
	//mainMIDIalarmthread->StartMIDI(song,Seq_Song::STATUS_RECORD);
	//}

	//song->metronome.waitforpreMIDIcallback=false;
	return -1; // force loop
}


void Seq_Song::InitMIDIIOTimer(OSTART spos)
{
	if(spos>=0)
		songstartposition=spos;

	mainMIDI->ResetMIDIInTimer(true);

#ifdef WIN32
	startsystemcounter64=maintimer->GetSystemTime();
#endif
}

void Seq_Song::SetRunningFlag(OSTART sposition,bool force)
{
	// Set by Audio and/or MIDI
	LockSongPosition();

	status CLEARBIT STATUS_MIDICYCLERESET;

	if((!(status&STATUS_SONGPLAYBACK_MIDI)) || force==true)
	{
		InitMIDIIOTimer(sposition);
		status|=STATUS_SONGPLAYBACK_MIDI;
	}

	UnlockSongPosition();
}

OSTART Seq_Song::ConvertSysTimeToSongPosition(LONGLONG systime)
{
	LockSongPosition();

	OSTART r=songposition;

	if(status&STATUS_SONGPLAYBACK_MIDI)
	{
		if(status&STATUS_MIDICYCLERESET)
		{
			r=cyclestartposition;
		}
		else
		{
			r=timetrack.ConvertTempoTicksToTicks(songstartposition,maintimer->ConvertSysTimeToInternTicks(systime-startsystemcounter64));
		}
	}

	UnlockSongPosition();

	return r;
}

OSTART Seq_Song::GetSongPosition()
{
	LockSongPosition();

	if(status&STATUS_SONGPLAYBACK_MIDI)
	{
		if(status&STATUS_MIDICYCLERESET)
		{
			songposition=cyclestartposition;
		}
		else
		{
			songposition=timetrack.ConvertTempoTicksToTicks(songstartposition,maintimer->ConvertSysTimeToInternTicks(maintimer->GetSystemTime()-startsystemcounter64));
		}
	}

	UnlockSongPosition();

	return songposition;
}

OSTART Seq_Song::GetSongPosition(LONGLONG *systime)
{
	*systime=maintimer->GetSystemTime();

	LockSongPosition();

	if(status&STATUS_SONGPLAYBACK_MIDI)
	{
		if(status&STATUS_MIDICYCLERESET)
		{
			songposition=cyclestartposition;
		}
		else
		{
			songposition=timetrack.ConvertTempoTicksToTicks(songstartposition,maintimer->ConvertSysTimeToInternTicks(*systime-startsystemcounter64));
		}
	}

	UnlockSongPosition();

	return songposition;
}

void Seq_Song::InitMIDIPosition()
{
	double h=maintimer->ConvertSysTimeToInternTicks(maintimer->GetSystemTime()-startsystemcounter64);
	h*=timetrack.ppqsampleratemul; // Intern -> Project Samplingrate

#ifdef DEBUG
	if(h<0)
		maingui->MessageBoxError(0,"InitMIDIPosition");
#endif

	MIDI_sampleendposition=MIDI_samplestartposition+(LONGLONG)h;

	CheckMIDIAlarms(MIDI_sampleendposition);
}

void Seq_Song::SendMIDI2MIDIAutomation(OSTART position)
{
	Seq_Track *t=FirstTrack();

	while(t){
		if(t->MIDItype==Seq_Track::OUTPUTTYPE_MIDI)
			t->SendMIDIAutomation(position);

		t=t->NextTrack_NoUILock();
	}
}

inline LONGLONG Seq_Song::MIDIPlaybackCall()
{
	if(startMIDIinit==true){
		startMIDIprecounter=startMIDIinit=false;
		StartMIDI();
	}

	nextpulse=-1; // Reset NextPulse

	if(startMIDIprecounter==true && (status&Seq_Song::STATUS_WAITPREMETRO)){

		if(CheckPreMetronome()==false){
			nextpulse=metronome.waitpreMIDIticks;
		}
		else{
			if(metronome.sendsynctoMIDI==false){
				status CLEARBIT STATUS_WAITPREMETRO;
				startMIDIinit=false;
				StartMIDI();
			}
		}
	}

	if(status&STATUS_SONGPLAYBACK_MIDI) // Sequencer START/RECORD ----------------------------
	{
		InitMIDIPosition();

loop:
		if(playbacksettings.cycleplayback==true && MIDI_sampleendposition>=playbacksettings.cycle_sampleend){
			CycleReset_MIDI(); // Init new songposition + new Tracks + recorded MIDI Patt
		}

		if(startMIDIinit==false){

			// Start With Metro Pulse
			if(metronome.nextclick_sample[MIDIMETROINDEX]<=MIDI_sampleendposition){

				if(metronome.on==true &&
					(
					((status&STATUS_PLAY) && metronome.playback==true) ||		
					((status&STATUS_RECORD) && metronome.record==true)
					)
					)
					metronome.SendClick(metronome.CheckIfHi(metronome.nextclick[MIDIMETROINDEX]));

				// Init next metro click
				metronome.InitMetroClickForce(MIDIMETROINDEX);
			}

			nextpulse=metronome.nextclick_sample[MIDIMETROINDEX]; // 1. Pulse

			// MIDI Clock/No Audio Engine Automation
			if(MIDIsync.out_nextclocksample<=MIDI_sampleendposition){

				if(MIDIsync.sync==SYNC_INTERN && MIDIsync.sendmc==true){

					// Send MIDI Clock
					for(int i=0;i<MAXMIDIPORTS;i++){

						MIDIOutputPort *m=&mainMIDI->MIDIoutports[i];

						if(m->visible==true && 
							m->outputdevice && 
							m->outputdevice->lockMIDIclock==false && 
							m->sendsync==true){
								m->outputdevice->sendRealtimeMessage(MIDIREALTIME_CLOCK,1);
								m->outputdevice->lockMIDIclock=true; // Avoid multiply Sending of MIDI CLock
						}
					}

					// Unlock MIDI Control
					MIDIOutputDevice *mo=mainMIDI->FirstMIDIOutputDevice();

					while(mo){
						mo->lockMIDIclock=false;
						mo=mo->NextOutputDevice();
					}
				}

				MIDIsync.out_nextclockposition+=SAMPLESPERBEAT/24; // Calc next MIDI Clock Alarm
				MIDIsync.out_nextclocksample=timetrack.ConvertTicksToTempoSamples(MIDIsync.out_nextclockposition);

			}//if h>>

			if(MIDIsync.out_nextclocksample<nextpulse)
				nextpulse=MIDIsync.out_nextclocksample; // Check MIDI Clock Impulse

			SendMIDI2MIDIAutomation(GetSongPosition());

			// MAIN MIDI Event Output Loop
			{
				cMIDIPlaybackEvent nmpbe(INITPLAY_MIDITRIGGER);

				GetNextMIDIPlaybackEvent(&nmpbe); // Init

				while(nmpbe.playbackevent && nmpbe.nextMIDIplaybacksampleposition<=MIDI_sampleendposition){

					if(nmpbe.playbackpattern->track->CheckIfTrackIsAudioInstrument()==false && // Send To Audio Instruments 
						(!(nmpbe.playbackevent->flag&EVENTFLAG_MUTED))) // MIDI Trigger ?
					{
						SendMIDIOutPlaybackEvent(nmpbe.playbackpattern->track,nmpbe.playbackpattern,nmpbe.playbackevent,nmpbe.nextMIDIplaybacksampleposition,Seq_Event::STD_CREATEREALEVENT); // +create real events
					}

					MixNextMIDIPlaybackEvent(&nmpbe);
					GetNextMIDIPlaybackEvent(&nmpbe);
				}

				if(nmpbe.playbackevent && nmpbe.nextMIDIplaybacksampleposition<nextpulse)
					nextpulse=nmpbe.nextMIDIplaybacksampleposition;
			}

		}// startinit ?			

		CheckMIDIAlarmsCheck(MIDI_sampleendposition,&nextpulse); // Send Rest + Check Nextpulse

		if(playbacksettings.cycleplayback==true && (nextpulse==-1 || playbacksettings.cycle_sampleend<nextpulse))
			nextpulse=playbacksettings.cycle_sampleend;

		// Automation
		{
			LONGLONG nextautomation=mainaudio->GetGlobalSampleRate();

			nextautomation/=100; // 10 ms
			nextautomation+=MIDI_sampleendposition;

			if(nextpulse==-1 || nextautomation<nextpulse)
				nextpulse=nextautomation;
		}

		if(nextpulse==-1)
			return -1;

		InitMIDIPosition();

		if(nextpulse<=MIDI_sampleendposition){
			nextpulse=-1;
			goto loop;
		}

		nextpulse-=MIDI_sampleendposition;
		return nextpulse; // Return Wait Samples

	}//song playback

	return nextpulse;
}

// MIDI Output -> Devices
MIDIOutputDevice *MIDIOutProc::GetMIDIOutputDevice()
{
	MIDIOutputDevice *r=0;

#ifdef WIN32
	c_outdevices_sema.Lock();
#endif

	if(c_MIDIOutDevice *f=(c_MIDIOutDevice *)c_outdevices.GetRoot()){
		r=f->device;
		c_outdevices.RemoveO(f);
	}

#ifdef WIN32
	c_outdevices_sema.Unlock();
#endif

	return r;
}

void MIDIOutDeviceChildThread::WaitLoop()
{
	while(IsExit()==false) // Signal Loop
	{
		WaitSignal(-1);

		if(IsExit()==true)
			break;

		MIDIOutputDevice *dev=MIDIproc->GetMIDIOutputDevice();

		while(dev){
			dev->SendOutput();
			dev=MIDIproc->GetMIDIOutputDevice(); // Next Device
		}

		SetEvent(MIDIproc->coredone[threadid]);
	}

	ThreadGone();
}

PTHREAD_START_ROUTINE MIDIOutDeviceChildThread::MIDIOutDeviceChildThread_Func(LPVOID pParam)
{
	MIDIOutDeviceChildThread *modt=(MIDIOutDeviceChildThread *)pParam;
	modt->WaitLoop();
	return 0;
}

void MIDIOutProc::WaitLoop()
{
	while(IsExit()==false) // Signal Loop
	{
		WaitSignal(-1);

		if(IsExit()==true)
			break;

		if(devinit==0)
		{
			// 1 MIDI Output Device only
			if(MIDIOutputDevice *dev=mainMIDI->FirstMIDIOutputDevice())
				dev->SendOutput();
		}
		else
		{
			// 2 or more MIDI Output Devices

			// Build List of used Device
			MIDIOutputDevice *dev=mainMIDI->FirstMIDIOutputDevice(),*onlydevice=0;
			while(dev){

				if(dev->datareadbuffercounter!=dev->datawritebuffercounter)
				{
					// Avoid List managment
					if(c_outdevices.GetCount()==0 && (!onlydevice))
						onlydevice=dev;
					else
					{
						if(c_outdevices.GetCount()==0)
						{
							if(c_MIDIOutDevice *nc=new c_MIDIOutDevice(onlydevice))c_outdevices.AddEndO(nc);
						}

						// Add Device
						if(c_MIDIOutDevice *nc=new c_MIDIOutDevice(dev))c_outdevices.AddEndO(nc);
					}
				}

				dev=dev->NextOutputDevice();
			}

			if(int devused=c_outdevices.GetCount())
			{
				if(devused>devinit)
					devused=devinit;

				//	TRACE ("Call Record Cores gone\n");
				// Call Sub Process
				for(int i=0;i<devused;i++)
				{
					devicechildthreads[i]->SetSignal();
				}

				DWORD dwEvent = WaitForMultipleObjects( 
					devused,           // number of objects in array
					MIDIoutproc->coredone,     // array of objects
					TRUE,       // wait for any object
					INFINITE);       // five-second wait

				//WaitForSingleObject(MIDIoutproc->devicethreads[i]->coredone, INFINITE);

				MIDIoutproc->c_outdevices.DeleteAllO();
			}
			else
			{
				// Single Device, send direct

				if(onlydevice)
					onlydevice->SendOutput();
			}
		}

	}// while exitthreads

	ThreadGone();
}

PTHREAD_START_ROUTINE MIDIOutProc::MIDIOutThreadFunc(LPVOID pParam)
{
	MIDIOutProc *MIDIoutproc=(MIDIOutProc *)pParam;
	MIDIoutproc->WaitLoop();
	return 0;
}

void Seq_Song::CheckMIDIAlarmsCheck(LONGLONG calarmsampleposition,LONGLONG *check)
{
	Seq_Realtime *real=&realtimeevents[REALTIMELIST_MIDI];
	real->Lock();

	// Reset
	RealtimeEvent *rte=real->FirstREvent();

	while(rte && rte->GetRealEventSampleStart()<=calarmsampleposition){

		rte->SendToMIDI();
		rte=real->DeleteREvent(rte);
	}

	if(rte=real->FirstREvent())
	{
		if(*check==-1 || *check>rte->GetRealEventSampleStart())
			*check=rte->GetRealEventSampleStart();
	}

	real->UnLock();
}

void Seq_Song::CheckMIDIAlarms(LONGLONG calarmsampleposition)
{
	Seq_Realtime *real=&realtimeevents[REALTIMELIST_MIDI];
	real->Lock();

	// Reset
	RealtimeEvent *rte=real->FirstREvent();

	while(rte && rte->GetRealEventSampleStart()<=calarmsampleposition){

		rte->SendToMIDI();
		rte=real->DeleteREvent(rte);
	}

	/*
	// Track First
	RealtimeEvent *rte=realtimeevents[REALTIMELIST_MIDI].FirstREvent();

	if((!rte) || rte->GetRealEventSampleStart()>calarmsampleposition){
	realtimeevents[REALTIMELIST_MIDI].UnLock();
	return;
	}

	while(rte && rte->GetRealEventSampleStart()<calarmsampleposition)
	{
	rte->SendToMIDI();
	rte=realtimeevents[REALTIMELIST_MIDI].DeleteREvent(rte);
	}

	if((!rte) || rte->GetRealEventSampleStart()>calarmsampleposition){
	realtimeevents[REALTIMELIST_MIDI].UnLock();
	return;
	}

	//	rte=realtimeevents[REALTIMELIST_MIDI].FirstREvent();

	int  ix=track->GetIndex(); // Track Index Check top/down

	while(rte && rte->GetRealEventSampleStart()==calarmsampleposition)
	{
	if(rte->fromtrack && rte->fromtrack->GetIndex()<=ix){
	rte->SendToMIDI();
	rte=realtimeevents[REALTIMELIST_MIDI].DeleteREvent(rte);
	}
	else
	rte=rte->NextEvent();
	}
	*/

	real->UnLock();
}

LONGLONG Seq_Song::CheckRTEAlarmEvents()
{
	LONGLONG r=-1;
	Seq_Realtime *real=&realtimeevents[REALTIMELIST_MIDI];

	real->Lock();

	RealtimeEvent *rte=real->FirstCDEvent();

	while(rte && rte->rte_callsystime<=maintimer->GetSystemTime()){
		rte->SendToMIDI();
		//MessageBeep(-1);
		rte=real->DeleteCDEvent(rte);
	}

	if(rte)
		r=rte->rte_callsystime;

	real->UnLock();

	return r;
}

void MIDIRTEProc::WaitLoop()
{
	double nextcall=-1;

	while(IsExit()==false) // Signal Loop
	{
		//		TRACE ("NextCall %f\n",nextcall);
		WaitSignal((int)nextcall);
		nextcall=-1;

		if(IsExit()==true)
			break; // Exit Signal ?

		Lock();

		if(Seq_Song *song=mainvar->GetActiveSong())
		{
			nextcall=song->CheckRTEAlarmEvents();

			if(nextcall!=-1) // Sys->MS
			{
				LONGLONG h=nextcall-maintimer->GetSystemTime();

				if(h>0)
				{
					nextcall=maintimer->ConvertSysTimeToMs(h);
					nextcall=ceil(nextcall);

					if(nextcall<1)
						nextcall=1;
				}
				else
					nextcall=1;
			}
		}

		Unlock();
	}

	ThreadGone();
}

PTHREAD_START_ROUTINE MIDIRTEProc::MIDIRTEAlarmThreadFunc(LPVOID pParam)
{
	MIDIrtealarmproc->WaitLoop();
	return 0;
}

void MIDIPlaybackThread::WaitLoop()
{
	LONGLONG nextalarmsamples=-1;	

	while(IsExit()==false){

		if(nextalarmsamples==-1)
			WaitSignal(-1);
		else{
			double h=nextalarmsamples;

			// Wait Samples -> ms
			h*=mainaudio->samplespermsmul;

			int i_np=(int)ceil(h); // round up 0.6ms>1
			WaitSignal(i_np<=1?1:i_np);
		}

		if(IsExit()==true)
			break; // Exit Signal ?	

		Lock();
		nextalarmsamples=mainvar->GetActiveSong()?mainvar->GetActiveSong()->MIDIPlaybackCall():-1;
		Unlock();
	}

	ThreadGone();
}

// MIDIout Realtime Mixer&Player
PTHREAD_START_ROUTINE MIDIPlaybackThread::MIDIPlaybackThreadFunc(LPVOID pParam) 
{
	MIDIPlaybackThread *mainMIDIalarmthread=(MIDIPlaybackThread *)pParam;
	mainMIDIalarmthread->WaitLoop();
	return 0;
}

void MIDIOutProc::Init()
{
	devinit=mainMIDI->GetNrMIDIOutputDevices();

	if(devinit>1) // min 2 Devices
	{
		if(devinit>MAXMIDIOUTDEVICETHREADS)
			devinit=MAXMIDIOUTDEVICETHREADS;

		for(int i=0;i<devinit;i++)
		{
			coredone[i]=CreateEvent( 
				NULL,   // default security attributes
				FALSE,  // auto-reset event object
				FALSE,  // initial state is nonsignaled
				NULL);  // unnamed object

			if(devicechildthreads[i]=new MIDIOutDeviceChildThread(this))
			{
				devicechildthreads[i]->threadid=i;
				devicechildthreads[i]->StartThread();
			}
		}
	}
}

void MIDIOutProc::DeInit()
{
	SendQuit(); // +Send Rest of MIDI Events

	for(int i=0;i<devinit;i++)
	{
		if(devicechildthreads[i]) // Sub Threads
		{
			devicechildthreads[i]->SendQuit();
			delete devicechildthreads[i];
			devicechildthreads[i]=0;
		}

		if(coredone[i]){
			CloseHandle(coredone[i]);
			coredone[i]=0;
		}
	}
}

int MIDIOutDeviceChildThread::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MIDIOutDeviceChildThread_Func,(LPVOID)this, 0,0);

	if(!ThreadHandle)
		error++;

	if(ThreadHandle)
		SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);
#endif

	return error;
}

MIDIOutProc::MIDIOutProc(){
	for(int i=0;i<MAXMIDIOUTDEVICETHREADS;i++)
	{
		devicechildthreads[i]=0;
		coredone[i]=0;
	}

	devinit=0;
}

int MIDIOutProc::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MIDIOutProc::MIDIOutThreadFunc,(LPVOID)this, 0,0);

	if(!ThreadHandle)
		error++;

	if(ThreadHandle)
		SetThreadPriority(ThreadHandle,THREAD_PRIORITY_HIGHEST); // Same as Audio Thread + Core Threads
#endif

	return error;
}

int MIDIPlaybackThread::StartThread()
{
	int error=0;
#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MIDIPlaybackThreadFunc,(LPVOID)this, 0,0);

	if(ThreadHandle)
		SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL); // Same as Audio Thread + Core Threads, ABOVE MIDI Device Output !
	else
		error=1;
#endif

	return error;
}

void MIDIPlaybackThread::StopThread()
{
	//	mainthreadcontrol->SignalDoubleBuffersFilled(); // Unlock from MIDI<->Audio Sync Wait
	SendQuit();
}

int MIDIRTEProc::StartThread()
{
	int error=0;
#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MIDIRTEAlarmThreadFunc,(LPVOID)this, 0,0);

	if(ThreadHandle)
		SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL); // Same as Audio Thread + Core Threads
	else
		error=1;
#endif

	return error;
}

void MIDIRTEProc::StopThread()
{
	//	mainthreadcontrol->SignalDoubleBuffersFilled(); // Unlock from MIDI<->Audio Sync Wait
	SendQuit();
}

void mainMIDIBase::SendResetToAllDevices(int iflag)
{
	//if(!(flag&(NO_SYSTEMLOCK|STOPSELECT_SYNC)))
	mainMIDIalarmthread->Lock();

	// Reset Ctrl -> MIDI
	for(int i=0;i<16;i++){
		MIDIOutputDevice *dev=mainMIDI->FirstMIDIOutputDevice();

		while(dev){	
			dev->sendControlChange(CONTROLCHANGE|i,123,0); // all notes off
			dev->sendControlChange(CONTROLCHANGE|i,121,0); // ResetCtrl

			// dev->sendPitchbend(i,0,64,true);
			dev=dev->NextOutputDevice();
		}
	}
	mainMIDIalarmthread->Unlock();

	// End MIDI

	// Reset Ctrl -> Track + Channel Plugins
	if(mainvar->GetActiveSong())
	{
		ControlChange cc,cc2;

		for(int i=0;i<16;i++){

			cc.status=CONTROLCHANGE|i; // all notes off
			cc.controller=123;
			cc.value=0;
			cc.pattern=0;

			cc2.status=CONTROLCHANGE|i; // ResetCtrl
			cc2.controller=121;
			cc2.value=0;
			cc2.pattern=0;

			// Tracks
			Seq_Track *t=mainvar->GetActiveSong()->FirstTrack();

			while(t)
			{
				cc.SendToAudio(0,&t->io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
				cc2.SendToAudio(0,&t->io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);

				cc.SendToAudio(0,&t->io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
				cc2.SendToAudio(0,&t->io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);

				t=t->NextTrack();
			}

			for(int i=0;i<LASTSYNTHCHANNEL;i++)
			{
				AudioChannel *c=mainvar->GetActiveSong()->audiosystem.FirstChannelType(i);

				while(c)
				{
					cc.SendToAudio(0,&c->io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
					cc2.SendToAudio(0,&c->io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);

					cc.SendToAudio(0,&c->io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
					cc2.SendToAudio(0,&c->io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);

					c=c->NextChannel();
				}
			}

			// Master
			cc.SendToAudio(0,&mainvar->GetActiveSong()->audiosystem.masterchannel.io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
			cc2.SendToAudio(0,&mainvar->GetActiveSong()->audiosystem.masterchannel.io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);

			cc.SendToAudio(0,&mainvar->GetActiveSong()->audiosystem.masterchannel.io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
			cc2.SendToAudio(0,&mainvar->GetActiveSong()->audiosystem.masterchannel.io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
		}
	}
}

void mainMIDIBase::SendPanic()
{
	// Reset NoteOffs on a channels an all keys

	mainMIDIalarmthread->Lock();
	// MIDI
	for(int i=0;i<16;i++){
		for(int k=0;k<127;k++){
			MIDIOutputDevice *dev=mainMIDI->FirstMIDIOutputDevice();

			while(dev){
				dev->sendNoteOff((UBYTE)(NOTEOFF|i),(UBYTE)k,0);
				dev=dev->NextOutputDevice();
			}
		}
	}
	mainMIDIalarmthread->Unlock();

	// Audio
	if(mainvar->GetActiveSong())
	{
		NoteOff_Raw off;

		for(int i=0;i<16;i++){

			for(int key=0;key<127;key++)
			{
				off.status=NOTEOFF|i; // all notes off
				off.key=key;
				off.velocityoff=0;
				off.pattern=0;

				// Tracks
				Seq_Track *t=mainvar->GetActiveSong()->FirstTrack();

				while(t)
				{
					off.SendToAudio(0,&t->io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
					off.SendToAudio(0,&t->io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);

					t=t->NextTrack();
				}

				for(int i=0;i<LASTSYNTHCHANNEL;i++)
				{
					AudioChannel *c=mainvar->GetActiveSong()->audiosystem.FirstChannelType(i);

					while(c)
					{
						off.SendToAudio(0,&c->io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
						off.SendToAudio(0,&c->io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);

						c=c->NextChannel();
					}
				}

				// Master
				off.SendToAudio(0,&mainvar->GetActiveSong()->audiosystem.masterchannel.io.audioinputeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);

				off.SendToAudio(0,&mainvar->GetActiveSong()->audiosystem.masterchannel.io.audioeffects,SENDAUDIO_DONTCREATEREALEVENTS,0,0);
			}
		}
	}

	SendResetToAllDevices(0/*NO_SYSTEMLOCK*/); // +Reset Filter
}

// Note On
void Note::SendPreProcDelete(Seq_Track *track,MIDIPattern *pattern,Seq_Event *e)
{
	if(track && pattern) // Pattern=0 Thru, No Init
		InitNoteOff(0,track,pattern,e,0,0);
}

void Note::SendToDevice(MIDIOutputDevice *device)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	device->sendNote(status,key,velocity);
}

void Note::SendToDevicePlaybackUser(MIDIOutputDevice *device,Seq_Song *song,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}
	device->sendNote(status,key,velocity);

	if((createflag&Seq_Event::STD_CREATEREALEVENT)==0) // no off
		return;


#ifdef MEMPOOLS
	NoteOff_Realtime *noteoff=mainpools->mempGetNoteOff_Realtime();
#else
	NoteOff_Realtime *noteoff=new NoteOff_Realtime;
#endif

	if(noteoff)
	{
		// Init NoteOff
		noteoff->audioobject=0;
		noteoff->outputdevice=device;

		noteoff->fromevent=0;
		noteoff->rte_frompattern=0;
		noteoff->fromtrack=0;
		noteoff->status=NOTEOFF|GetChannel();
		noteoff->key=key;
		noteoff->velocityoff=velocityoff;
		noteoff->iflag=0;

		song->realtimeevents[REALTIMELIST_MIDI].AddCDAlarmEvent(noteoff,off.ostart-ostart);
	}
}

void Note::InitNoteOff(MIDIOutputDevice *device,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,LONGLONG sampleposition,int createflag)
{
	LONGLONG end=(createflag&Seq_Event::STD_PRESTARTEVENT)?GetSampleEnd(track->song,fpattern):sampleposition+GetSampleSize(track->song,fpattern); // Pre Events No Timing Correction


#ifdef MEMPOOLS
	NoteOff_Realtime *noteoff=mainpools->mempGetNoteOff_Realtime();
#else
	NoteOff_Realtime *noteoff=new NoteOff_Realtime;
#endif

	if(noteoff)
	{
		// Init NoteOff
		noteoff->audioobject=0;
		noteoff->outputdevice=device;

		noteoff->fromevent=fevent;
		noteoff->rte_frompattern=fpattern;
		noteoff->fromtrack=track;
		noteoff->status=NOTEOFF|GetChannel();
		noteoff->key=key;
		noteoff->velocityoff=velocityoff;
		noteoff->iflag=0;

		track->song->realtimeevents[REALTIMELIST_MIDI].AddRealtimeAlarmEvent(noteoff,end);
	}
}

void Note::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}
	// MIDI Timing NoteOn <-> NoteOff Latency Correction
	// Avoid 2x NoteOn

	Seq_Realtime *real=&song->realtimeevents[REALTIMELIST_MIDI];

	real->Lock();	
	RealtimeEvent *rte=real->FirstREvent();

	while(rte){

		if(rte->outputdevice==device && rte->realtimeid==REALTIMEID_NOTEOFFREALTIME)
		{
			NoteOff_Realtime *noff=(NoteOff_Realtime *)rte;

			if(noff->status==(NOTEOFF|GetChannel()) && noff->key==key)
			{
				rte->SendToMIDI();

				//	TRACE ("Send Off before NoteOn...\n");
				rte=real->DeleteREvent(rte);
			}
			else
				rte=rte->NextEvent();
		}
		else
			rte=rte->NextEvent();
	}

	real->UnLock();

	device->sendNote(status,key,velocity);

	if(createflag&Seq_Event::STD_CREATEREALEVENT) // Create Alarm Off ?
		InitNoteOff(device,track,fpattern,fevent,song->MIDI_sampleendposition,createflag);
}

void Note::SetMIDIImpulse(Seq_Track *t)
{
	double h=100;

	h/=127;
	h*=velocity;

	if((int)h>t->MIDIoutputimpulse)
		t->MIDIoutputimpulse=(int)h;
}

bool Note::Compare(Seq_Event *e)
{
	if(e->status==status && 
		((Note *)e)->key==key && 
		((Note *)e)->velocity==velocity &&
		((Note *)e)->velocityoff==velocityoff &&
		((Note *)e)->GetNoteLength()==GetNoteLength()
		)
		return true;

	return false;
}

void Note::MoveIndex(int index)
{
	int newkey=key;

	newkey+=index;

	if(newkey>127)
		key=127;
	else
		if(newkey<0)
			key=0;
		else
			key=(char)newkey;
}

void Note::Load(camxFile *file)
{
	file->ReadChunk(&key);
	file->ReadChunk(&velocity);
	file->ReadChunk(&velocityoff);
	file->ReadChunk(&off.ostart);
	file->ReadChunk(&off.staticostart);
}

void Note::Save(camxFile *file)
{
	file->Save_Chunk(key);
	file->Save_Chunk(velocity);
	file->Save_Chunk(velocityoff);
	file->Save_Chunk(off.ostart);
	file->Save_Chunk(off.staticostart);
}

void NoteOff_Raw::SendToDevice(MIDIOutputDevice *device)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	if(velocityoff==0)
		device->sendNote(NOTEON|GetChannel(),key,0);
	else
		device->sendNoteOff(status,key,velocityoff);
}

void NoteOff_Raw::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,int createflag)
{	
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	SendToDevice(device);
}

void NoteOff_Raw::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{	
	InsertAudioEffect *ai=fx->FirstActiveAudioEffect();

	while(ai)
	{
		if(ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ai->audioeffect!=dontsendtoaudioobject)
			ai->audioeffect->Do_TriggerEvent(offset,velocityoff==0?(NOTEON|GetChannel()):status,key,velocityoff,velocityoff);

		ai=ai->NextActiveEffect();
	}
}

void NoteOff_Raw::SendToAudioPlayback(AudioObject *o,LONGLONG samplestart,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,int flag,int sampleoffset,bool createreal)
{	
	if(velocityoff=0)
		status=NOTEON|GetChannel();

	o->Do_TriggerEvent(sampleoffset,status,key,velocityoff,velocityoff);
}

void NoteOff_Raw::DoEventProcessor(MIDIProcessor *proc)
{
	int h=key;

	if(proc->trackfx)
		h+=proc->trackfx->GetTranspose();

	if(proc->patternfx)
		h+=proc->patternfx->GetTranspose();

	if(h<0)
		key=0;
	else
		if(h>127)key=127;
		else
			key=(char)h;
}


void MIDIProcessorProc::AddAudioAlarms(AudioDevice *device)
{
	/*
	if(!device){#ifdef DEBUG maingui->MessageBoxError(0,"MIDI Device"); #endif }

	mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);

	Proc_Alarm *p=FirstAlarm();

	while(p)
	{
	LONGLONG systime_end=maintimer->GetSystemTime()+device->samplebufferms_long;

	if(p->alarmsystemtime_ms<=systime_end && p->module->processor->track->CheckIfAudioInstrumentTrack()==true)
	{
	//TRACE ("Call Module (Alarm) %d\n",p->alarmsystemtime);
	MIDIPlugin *module=p->module;

	module->LockO();

	p->calledbyaudio=true;

	double offset_ms=p->alarmsystemtime_ms-maintimer->GetTime(); //ms

	if(offset_ms>0)
	{
	p->offset_ms=(int)floor(offset_ms+0.5);

	double h=offset_ms;
	h/=device->samplebufferms;
	h*=device->setSize;

	p->offset_sample=(int)floor(h+0.5);
	}

	TRACE("Audio Offset Buffer %d %d\n",p->offset_sample,device->setSize);

	//	if(p->offset>=device->t)
	//		p->offset=device->ticksperbuffer_long-1;

	p->calledbyalarm=true;
	module->Alarm(p);
	p->forcealarm=false; // Reset Force Flag
	p=RemoveAlarm(p);

	module->UnlockO();
	}
	else
	p=p->NextAlarm();
	}

	mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);
	*/
}