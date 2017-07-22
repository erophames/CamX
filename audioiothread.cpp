#include "defines.h"

#include "songmain.h"
#include "audiorecord.h"
#include "audiofile.h" // dummy
#include "audiorealtime.h"
#include "semapores.h"
#include "audiohardware.h"
#include "audiobus.h"
#include "MIDIhardware.h"
#include "audioproc.h"
#include "MIDIPattern.h"
#include "object_song.h"
#include "object_track.h"
#include "MIDIoutproc.h"
#include "MIDItimer.h"
#include "audiodevice.h"
#include "audiohdfile.h"
#include "initplayback.h"
#include "object_project.h"
#include "gui.h"
#include "audiohardwarechannel.h"

#ifdef WIN32
#include "asio/asio.h"
#endif

int AudioDevice::ClearOffSet(double offset)
{
	if(offset>=GetSetSize())
		return GetSetSize()-1;

	return (int)offset;
}

int AudioDevice::ClearOffSet(int offset)
{
	if(offset>=GetSetSize())
		return GetSetSize()-1;

	return offset;
}

void Seq_Song::SendAudioWaitEvents(AudioDevice *device,LONGLONG samplestart,LONGLONG sampleend,bool force)
{
	realtimeevents[REALTIMELIST_AUDIO].Lock();

	RealtimeEvent *rte=realtimeevents[REALTIMELIST_AUDIO].FirstREvent();

	while(rte)
	{
		if(force==true || (rte->GetRealEventSampleStart()>=samplestart && rte->GetRealEventSampleStart()<sampleend)) // inside start<>end
		{
			int offset=playback_sampleoffset;

			if(force==true && playback_samplesize && rte->GetRealEventSampleStart()>=sampleend)
				offset+=playback_samplesize-1;
			else
				offset+=(int)(rte->GetRealEventSampleStart()-samplestart);

			if(offset<0)
			{
				// Its OKAY with - Humanize etc...
#ifdef DEBUG
	//			maingui->MessageBoxError(0,"SendAudioWaitEvents sampleoffset<0"); 
#endif
				offset=0;
			}
			else
				if(offset>=device->GetSetSize())
				{
#ifdef DEBUG
					maingui->MessageBoxError(0,"SendAudioWaitEvents sampleoffset>0");
#endif
					offset=device->GetSetSize()-1;
				}

				rte->SendToAudio(rte->audioobject,offset);

				rte=realtimeevents[REALTIMELIST_AUDIO].DeleteREvent(rte);
		}
		else
			rte=rte->NextEvent();
	}

	realtimeevents[REALTIMELIST_AUDIO].UnLock();
}

void Seq_Song::DropDownPeaks(AudioDevice *device,bool withtracks) // Called by Refill
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	if(withtracks==true)
	{
		// Tracks
		Seq_Track *t=FirstTrack();

		while(t)
		{
			// Out
			if(t->GetPeak()->peakused==true)
				t->GetPeak()->Drop(device->devicedroprate);

			t=t->NextTrack_NoUILock();
		}
	}

	{
		// Metro Tracks
		Seq_MetroTrack *mt=FirstMetroTrack();

		while(mt)
		{
			// Out
			if(mt->GetPeak()->peakused==true)
				mt->GetPeak()->Drop(device->devicedroprate);

			mt=mt->NextMetroTrack();
		}
	}

	{
		AudioChannel *c=audiosystem.FirstBusChannel();
		while(c)
		{
			// In
			if(c->io.inputpeaks.peakused==true)
				c->io.inputpeaks.Drop(device->devicedroprate);

			//if(t->io.tempinputmonitoring==true)
			//	t->io.CheckInputPeak(t->GetVIn());

			// Out
			if(c->mix.peak.peakused==true)
				c->mix.peak.Drop(device->devicedroprate);

			c=c->NextChannel();
		}
	}

	if(device)
	{
		AudioHardwareChannel *c=device->FirstOutputChannel();
		while(c)
		{
			c->DropPeak(device->devicedroprate);
			c=c->NextChannel();
		}
	}

	if(audiosystem.masterchannel.io.inputpeaks.peakused==true)
		audiosystem.masterchannel.io.inputpeaks.Drop(device->devicedroprate);

	if(audiosystem.masterchannel.mix.peak.peakused==true)
		audiosystem.masterchannel.mix.peak.Drop(device->devicedroprate);
}

void Seq_Song::DoRealtimeAndCDEvents(AudioDevice *device)
{
	realtimeevents[REALTIMELIST_AUDIO].Lock();

	// CD Events
	RealtimeEvent *rte=realtimeevents[REALTIMELIST_AUDIO].FirstCDEvent();
	while(rte)
	{
		if(rte->rte_callsampleposition<device->GetSetSize())
		{
			rte->SendToAudio(rte->audioobject,(int)rte->rte_callsampleposition); // Rest=Offset
			rte=realtimeevents[REALTIMELIST_AUDIO].DeleteCDEvent(rte);
		}
		else
		{
			// CD use setSize
			rte->rte_callsampleposition-=device->GetSetSize();
			rte=rte->NextEvent();
		}
	}

	realtimeevents[REALTIMELIST_AUDIO].UnLock();


	mainaudioreal->LockRTObjects();
	AudioRealtime *ar=mainaudioreal->FirstAudioRealtime();

	while(ar)
	{
		AudioRealtime *n=ar->NextAudioRealtime();

		//if(ar->canbeclose==false)
		//{
			if(ar->stream && ar->audiochannel && ar->audiochannel->io.out_vchannel)
			{
				//if(ar->audiochannel->Muted()==true)
				//	ar->stream[ar->streamreadc].channelsused=0;

				ar->stream[ar->streamreadc].ExpandMixAudioBuffer(&ar->audiochannel->mix,ar->audiochannel->io.out_vchannel->channels,0,0);
				ar->stream[ar->streamreadc].channelsused=0; // reset
			}

			if(ar->streamreadc==ar->nrstream-1)
				ar->streamreadc=0;
			else
				ar->streamreadc++;
		//}
		//else
		//	ar->canbeclose=true;

		if(ar->endreached==true)
		{
			ar->closeit=true; // Set CLOSE Flag

			/*
			if(ar->autoclose==true)
				mainaudioreal->RemoveAudioRealtime(ar,false);
				*/

		}

		ar=n;
	}

	mainaudioreal->UnlockRTObjects();
}

void Seq_Song::RefillAudioDeviceMaster(AudioDevice *device)
{
	// Mastering...
	playback_sampleoffset=0;
	playback_samplesize=device->GetSetSize();
	playback_sampleendposition=playback_sampleposition+playback_samplesize;

	// Samples -> OSTART
	audioplayback_position=timetrack.ConvertSamplesToOSTART(playback_sampleposition);
	audioplayback_endposition=timetrack.ConvertSamplesToOSTART(playback_sampleendposition);

	AddAudioInstrumentsToStream(device);
	CopyTrackStreamBuffer_Mastering(device);
	DoAudioEffects_Mastering(device);

	CreateAudioStream(device,CREATESTREAMMASTER); // Refill Audio

	playback_sampleposition=playback_sampleendposition;

	//TRACE ("RK device IRQ Counter %d\n",device->irqplaybackcounter);
}

// Main Audio mix Function
// called by ASIO: direct Callback
// called by WIN32 PCM waveout: thread


void Seq_Song::RefillAudioDeviceBuffer(AudioDevice *device) // Main Audio Kernel Function
{
	if(masteringlock==true)
	{
		device->SkipDeviceOutputBuffer(this);
		return;
	}

	LONGLONG t1=maintimer->GetSystemTime();

	playback_sampleoffset=0; // Cycle Offset Buffer Cut 
	playback_samplesize=playback_setSize=device->GetSetSize();

	if(!(status&STATUS_SONGPLAYBACK_AUDIO)){

		if(status&STATUS_MIDIWAIT){

			if((status&STATUS_WAITPREMETRO) && CheckPreMetronome(device)==true){
				status CLEARBIT STATUS_WAITPREMETRO;
			}
		}
		else
		{
			if( (startaudioinit==true && (!(status&STATUS_WAITPREMETRO))) ||
				((status&STATUS_WAITPREMETRO) && CheckPreMetronome(device)==true)
				){
					// Start Audio
					status CLEARBIT STATUS_WAITPREMETRO;
					status CLEARBIT STATUS_MIDIWAIT;
					status |=STATUS_SONGPLAYBACK_AUDIO;

					if(status&STATUS_RECORD)
						StartAudioRecording();

					pRepareMIDIstartdelay=true;
					MIDIstartthread->SetSignal(); // Add Output Latency - Start/Sync MIDI
			}
		}
	}

	if(!(device->status&AudioDevice::STATUS_INPUTOK))
	{
		SongAudioDataInput(device,false); // Replace HW Input + Fill Track Buffer/Record
	}

	MIDIalarmprocessorproc->AddAudioAlarms(device);

	if(status&STATUS_SONGPLAYBACK_AUDIO){

		startaudioinit=false;  // Reset

		DropDownPeaks(device,false); // No Tracks
		CopyTrackStreamBuffer(device); // Copy/Mix Audio Files->Track Buffer + Track Thru + Drop Down Tracks

		int refillflag=0;
	
cycleloop:

		playback_sampleendposition=playback_sampleposition+playback_setSize;

		if(playbacksettings.cycleplayback==true && playback_sampleendposition>=playbacksettings.cycle_sampleend)
		{
			playback_sampleendposition=playbacksettings.cycle_sampleend;

			playback_samplesize=playback_sampleendposition-playback_sampleposition;
			refillflag = RAD_CYCLERESET;
		}

		playback_setSize-=playback_samplesize;

		// Samples -> OSTART
		timetrack.ConvertSamplesToOSTART(playback_sampleposition,&audioplayback_position,playback_sampleendposition,&audioplayback_endposition);

		// Execute ...
		AddAudioMetronome(device); // +metro Realtime
		AddAudioInstrumentsToStream(device);
		// End Execute

		if(refillflag&RAD_CYCLERESET){

			refillflag = RAD_CYCLERESET2;
			CycleReset_Audio(device);

			if(playback_setSize)
				goto cycleloop;
		}

		// End Cycle Loop
		DoAudioEffectsAndCopyToDevice(device);

		playback_sampleposition=playback_sampleendposition;

		if(playback_bufferindex==device->numberhardwarebuffer-1)playback_bufferindex=0; else playback_bufferindex++;

		LockRefreshCounter();
		refreshcounter++;
		UnlockRefreshCounter();

		mainaudiostreamproc->SetSignal(); // Send Signal to Stream Refill Thread

		// end Song Started
	}
	else{
		// Song Stopped
		DropDownPeaks(device,true); // + Tracks
		DoAudioEffectsAndCopyToDevice(device);
	}

	mainaudiorealtimethread->SetSignal(); // Realtime Playback

	LONGLONG t2=maintimer->GetSystemTime();

	device->LockTimerCheck_Output();

	if(t2-t1>device->timeforrefill_maxsystime)
		device->timeforrefill_systime=device->timeforrefill_maxsystime=t2-t1;
	else
		device->timeforrefill_systime=t2-t1;

	device->UnlockTimerCheck_Output();
}

// ********************** Event Output Audio
void PolyPressure::SendToDevice(MIDIOutputDevice *device)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	device->sendPolyPressure(status,key,pressure);
}

void PolyPressure::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *pattern,Seq_Event *seqevent,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	device->sendPolyPressure(status,key,pressure);
}

void PolyPressure::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{	
	InsertAudioEffect *ai=fx->FirstActiveAudioEffect();

	while(ai){

		if(ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ai->audioeffect!=dontsendtoaudioobject && ai->MIDIfilter.CheckEvent(this)==true)
			ai->audioeffect->Do_TriggerEvent(offset,status,key,pressure,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);

		ai=ai->NextActiveEffect();
	}
}

bool PolyPressure::Compare(Seq_Event *e)
{
	if(e->status==status && ((PolyPressure *)e)->key==key && ((PolyPressure *)e)->pressure==pressure)
		return true;

	return false;
}

void PolyPressure::Load(camxFile *file)
{
	file->ReadChunk(&key);
	file->ReadChunk(&pressure);
}

void PolyPressure::Save(camxFile *file)
{
	file->Save_Chunk(key);
	file->Save_Chunk(pressure);
}

Object *PolyPressure::Clone(Seq_Song *song)
{
	if(PolyPressure *p=new PolyPressure){
		CloneData(song,p);
		return p;
	}

	return 0;
}

bool PolyPressure::CheckSelectFlag(int checkflag,int icdtype)
{
	if( (checkflag&SEL_POLYPRESSURE) &&
		( 
		((checkflag&SEL_SELECTED) && (flag&OFLAG_SELECTED)) ||
		((!(checkflag&SEL_SELECTED)) && (!(flag&OFLAG_SELECTED)) )
		)
		)
		return true;

	return false;
}

void PolyPressure::CloneData(Seq_Song *song,Seq_Event *e)
{
	PolyPressure *to=(PolyPressure *)e;
	to->status=status;
	to->key=key;
	to->pressure=pressure;
	to->ostart=ostart;
	to->staticostart=staticostart;
	//to->sampleposition=sampleposition;
}

void PolyPressure::SendToAudioPlayback(AudioObject *o,
									   LONGLONG samplestart,
									   Seq_Track *track,
									   MIDIPattern *fpattern,
									   Seq_Event *fevent,
									   int iflag,
									   int sampleoffset,
									   bool createreal
									   )
{
	o->Do_TriggerEvent(sampleoffset,status,key,pressure,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);
}

bool ControlChange::Compare(Seq_Event *e)
{
	if(e->status==status && ((ControlChange *)e)->controller==controller && ((ControlChange *)e)->value==value)
		return true;

	return false;
}

void ControlChange::Load(camxFile *file)
{
	file->ReadChunk(&controller);
	file->ReadChunk(&value);
}

void ControlChange::Save(camxFile *file)
{
	file->Save_Chunk(controller);
	file->Save_Chunk(value);
}

void ControlChange::CloneData(Seq_Song *song,Seq_Event *e)
{
	ControlChange *to=(ControlChange *)e;

	to->status=status;
	to->controller=controller;
	to->value=value;

	to->ostart=ostart;
	to->staticostart=staticostart;
	//to->sampleposition=sampleposition;
}

void ControlChange::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *pattern,Seq_Event *seqevent,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	device->sendControlChange(status,controller,value);

	if((createflag&Seq_Event::STD_CREATEREALEVENT)==0) // no off
		return;

	switch(controller)
	{
	case 64: // Sustain
		{
			if(value>=64) // Sustain 0-63==off, 64-127 ON
			{
				if(Control_SustainOn *sustainoff=new Control_SustainOn)
				{
					// Init NoteOff
					sustainoff->outputdevice=device; // Connect

					sustainoff->fromevent=0;
					sustainoff->rte_frompattern=pattern;
					sustainoff->fromtrack=0;
					sustainoff->status=CONTROLCHANGE|GetChannel();
					sustainoff->sustain=value;

					song->realtimeevents[REALTIMELIST_MIDI].AddWaitEvent(sustainoff);
				}
			}
			else // Sustain OFF - Remove Sustain RTE
			{
				song->realtimeevents[REALTIMELIST_MIDI].Lock();

				RealtimeEvent *rte=song->realtimeevents[REALTIMELIST_MIDI].FirstWEvent();
				while(rte)
				{
					if(rte->realtimeid==REALTIMEID_SUSTAIN_ON && rte->status==status && rte->outputdevice==device)
					{
						rte=song->realtimeevents[REALTIMELIST_MIDI].DeleteWEvent(rte);
						break;
					}
					else
						rte=rte->NextEvent();
				}

				song->realtimeevents[REALTIMELIST_MIDI].UnLock();
			}
		}

		break;
	}
}

void ControlChange::SendToDevice(MIDIOutputDevice *device)
{
	device->sendControlChange(status,controller,value);

	if(controller==64 && value<64) // Remove Sustain RTE
	{
		mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_MIDI].Lock();
		RealtimeEvent *rte=mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_MIDI].FirstWEvent();
		while(rte)
		{
			if(rte->realtimeid==REALTIMEID_SUSTAIN_ON && rte->status==status && rte->outputdevice==device)
			{
				mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_MIDI].DeleteWEvent(rte);
				break;
			}
			else
				rte=rte->NextEvent();
		}
		mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_MIDI].UnLock();
	}
}

void ControlChange::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{	
	InsertAudioEffect *ai=fx->FirstActiveAudioEffect();

	while(ai){

		if(ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ai->audioeffect!=dontsendtoaudioobject && ai->MIDIfilter.CheckEvent(this)==true)
		{
			ai->audioeffect->Do_TriggerEvent(offset,status,controller,value,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);

			// Sustain On ?
			if(controller==64 && value>=64) // Sustain 0-63==off, 64-127 ON
			{
				if(!(iflag&SENDAUDIO_DONTCREATEREALEVENTS))
				{
					if(Control_SustainOn *sustainoff=new Control_SustainOn)
					{
						// Init NoteOff
						sustainoff->audioobject=ai->audioeffect; // Connect

						sustainoff->fromevent=0;
						sustainoff->rte_frompattern=frompattern;
						sustainoff->fromtrack=0;
						sustainoff->status=CONTROLCHANGE|GetChannel();
						sustainoff->sustain=value;

						fx->GetSong()->realtimeevents[REALTIMELIST_AUDIO].AddWaitEvent(sustainoff);
					}
				}
			}
			else
				if(controller==64 && value<64) // Remove Sustain RTE
				{
					Seq_Song *song=fx->GetSong();

					song->realtimeevents[REALTIMELIST_AUDIO].Lock();

					RealtimeEvent *rte=song->realtimeevents[REALTIMELIST_AUDIO].FirstWEvent();
					while(rte)
					{
						if(rte->realtimeid==REALTIMEID_SUSTAIN_ON && rte->status==status && rte->audioobject==ai->audioeffect)
						{
							song->realtimeevents[REALTIMELIST_AUDIO].DeleteWEvent(rte);
							break;
						}
						else
							rte=rte->NextEvent();
					}

					song->realtimeevents[REALTIMELIST_AUDIO].UnLock();
				}
		}

		ai=ai->NextActiveEffect();
	}
}

void ControlChange::SendToAudioPlayback(AudioObject *o,
										LONGLONG samplestart,
										Seq_Track *track,
										MIDIPattern *fpattern,
										Seq_Event *fevent,
										int iflag,
										int sampleoffset,
										bool createreal
										)
{
	o->Do_TriggerEvent(sampleoffset,status,controller,value,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);

	if(createreal==true)
	{
		if(controller==64 && value>=64) // Sustain 0-63==off, 64-127 ON
		{
			if(Control_SustainOn *sustainoff=new Control_SustainOn)
			{
				// Init NoteOff
				sustainoff->audioobject=o; // Connect

				sustainoff->fromevent=0;
				sustainoff->rte_frompattern=fpattern;
				sustainoff->fromtrack=0;
				sustainoff->status=CONTROLCHANGE|GetChannel();
				sustainoff->sustain=value;

				mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_AUDIO].AddWaitEvent(sustainoff);
			}
		}
		else
			if(controller==64 && value<64) // Remove Sustain RTE
			{
				mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_AUDIO].Lock();
				RealtimeEvent *rte=mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_AUDIO].FirstWEvent();
				while(rte)
				{
					if(rte->realtimeid==REALTIMEID_SUSTAIN_ON && rte->status==status && rte->audioobject==o)
					{
						mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_AUDIO].DeleteWEvent(rte);
						break;
					}
					else
						rte=rte->NextEvent();
				}

				mainvar->GetActiveSong()->realtimeevents[REALTIMELIST_AUDIO].UnLock();
			}
	}
}

void ProgramChange::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *pattern,Seq_Event *seqevent,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	device->sendProgramChange(status,program);
}

void ProgramChange::SendToDevice(MIDIOutputDevice *device)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	device->sendProgramChange(status,program);
}

void ProgramChange::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{	
	InsertAudioEffect *ai=fx->FirstActiveAudioEffect();

	while(ai){

		if(ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ai->audioeffect!=dontsendtoaudioobject && ai->MIDIfilter.CheckEvent(this)==true)
			ai->audioeffect->Do_TriggerEvent(offset,status,program,0,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);

		ai=ai->NextActiveEffect();
	}
}

void ProgramChange::SendToAudioPlayback(AudioObject *o,LONGLONG samplestart,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,int iflag,int sampleoffset,bool createreal)
{
	o->Do_TriggerEvent(sampleoffset,status,program,0,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);
}

void Pitchbend::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{	
	InsertAudioEffect *ai=fx->FirstActiveAudioEffect();

	while(ai){
		if(ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ai->audioeffect!=dontsendtoaudioobject && ai->MIDIfilter.CheckEvent(this)==true)
			ai->audioeffect->Do_TriggerEvent(offset,status,lsb,msb,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);

		ai=ai->NextActiveEffect();
	}
}

void Pitchbend::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *pattern,Seq_Event *seqevent,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}
	device->sendPitchbend(status,lsb,msb);
}

void Pitchbend::SendToDevice(MIDIOutputDevice *device)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}
	device->sendPitchbend(status,lsb,msb);
}

void Pitchbend::SendToAudioPlayback(AudioObject *o,LONGLONG samplestart,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,int iflag,int sampleoffset,bool createreal)
{
	o->Do_TriggerEvent(sampleoffset,status,lsb,msb,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);
}

void ChannelPressure::CloneData(Seq_Song *song,Seq_Event *e)
{
	ChannelPressure *to=(ChannelPressure *)e;
	to->status=status;
	to->pressure=pressure;
	to->ostart=ostart;
	to->staticostart=staticostart;
	//	to->sampleposition=sampleposition;
}

void ChannelPressure::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{	
	InsertAudioEffect *ai=fx->FirstActiveAudioEffect();
	while(ai)
	{
		if(ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ai->audioeffect!=dontsendtoaudioobject && ai->MIDIfilter.CheckEvent(this)==true)
			ai->audioeffect->Do_TriggerEvent(offset,status,pressure,0,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);

		ai=ai->NextActiveEffect();
	}
}

void ChannelPressure::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *pattern,Seq_Event *seqevent,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}
	device->sendChannelPressure(status,pressure);
}

void ChannelPressure::SendToDevice(MIDIOutputDevice *device)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}
	device->sendChannelPressure(status,pressure);
}

void ChannelPressure::Load(camxFile *file)
{
	file->ReadChunk(&pressure);
}

void ChannelPressure::Save(camxFile *file)
{
	file->Save_Chunk(pressure);
}

void ChannelPressure::SendToAudioPlayback(AudioObject *o,LONGLONG samplestart,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,int iflag,int sampleoffset,bool createreal)
{
	o->Do_TriggerEvent(sampleoffset,status,pressure,0,0,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);
}

void SysEx::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *pattern,Seq_Event *seqevent,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}
	if(data && length)
		device->sendSysEx(data,length);
}

void SysEx::SendToDevice(MIDIOutputDevice *device)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}
	if(data && length)
		device->sendSysEx(data,length);
}

void SysEx::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{
	if(data && length)
	{
		InsertAudioEffect *ai=fx->FirstActiveAudioEffect();
		while(ai)
		{
			if(ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ai->audioeffect!=dontsendtoaudioobject && ai->MIDIfilter.CheckEvent(this)==true)
				ai->audioeffect->Do_TriggerSysEx(offset,data,length);

			ai=ai->NextActiveEffect();
		}
	}
}

void SysEx::SendToAudioPlayback(AudioObject *o,
								LONGLONG samplestart,
								Seq_Track *track,
								MIDIPattern *fpattern,
								Seq_Event *fevent,
								int flag,
								int sampleoffset,
								bool createreal
								)
{
	if(data && length)
		o->Do_TriggerSysEx(sampleoffset,data,length);
}

void Note::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{	
	bool addedtomon=false;

	if(InsertAudioEffect *ai=fx->FirstActiveAudioEffect())
	{
		LONGLONG noteoffset_samples,notelength_samples; // Thru no Note Length!

		if(iflag&SENDAUDIO_DONTCREATEREALEVENTS){
			// Thru Note etc...
			noteoffset_samples=0;
			notelength_samples=0;
		}
		else{
			// Real Note
			notelength_samples=fx->GetSong()->timetrack.ConvertTicksToTempoSamplesStart(ostart,GetNoteLength());
			noteoffset_samples=0;
		}

		while(ai)
		{
			if(ai->audioeffect->plugin_on==true && 
				ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT &&
				ai->audioeffect!=dontsendtoaudioobject &&
				ai->MIDIfilter.CheckEvent(this)==true &&
				ai->audioeffect->Do_TriggerEvent(offset,status,key,velocity,(flag&EVENTFLAG_THRUEVENT)?0:velocityoff,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag,notelength_samples,noteoffset_samples)==true)
			{
				if(addedtomon==false)
				{
					addedtomon=true;
					//mainMIDI->monitor.OpenKey(status,key,pos+GetNoteLength());
				}

				if(!(iflag&SENDAUDIO_DONTCREATEREALEVENTS))
				{
					
#ifdef MEMPOOLS
	NoteOff_Realtime *noteoff=mainpools->mempGetNoteOff_Realtime();
#else
	NoteOff_Realtime *noteoff=new NoteOff_Realtime;
#endif

					if(noteoff)
					{
						// Init NoteOff
						noteoff->audioobject=ai->audioeffect; // Connect
						noteoff->outputdevice=0;

						noteoff->fromevent=0;
						noteoff->rte_frompattern=frompattern;
						noteoff->fromtrack=0;
						noteoff->status=NOTEOFF|GetChannel();
						noteoff->key=key;
						noteoff->velocityoff=velocityoff;
						noteoff->iflag=iflag;

						fx->GetSong()->realtimeevents[REALTIMELIST_AUDIO].AddCDAlarmEvent(noteoff,notelength_samples);
					}
				}
			}

			ai=ai->NextActiveEffect();
		}
	}
}

void Note::SendToAudioPlayback(AudioObject *o,LONGLONG samplestart,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,int iflag,int sampleoffset,bool createreal)
{	
	if(off.ostart==-1) // no off
	{
		o->Do_TriggerEvent(sampleoffset,status,key,velocity,127,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag);
		return;
	}

	/*
	#ifdef DEBUG
	if(!(fevent->flag&EVENTFLAG_PRESTART))
	{
	LONGLONG ss=GetSampleStart(track->song,fpattern);

	if(ss!=samplestart)
	maingui->MessageBoxError(0,"Note SampleStart");

	}
	#endif
	*/

	LONGLONG noteStart_samples=GetSampleStart(track->song,fpattern),
		noteEnd_samples=GetSampleEnd(track->song,fpattern),
		noteLength_samples=noteEnd_samples-noteStart_samples;

	if(!(fevent->flag&EVENTFLAG_PRESTART))
	{
		noteStart_samples=track->AddSampleDelay(noteStart_samples);
		noteEnd_samples=track->AddSampleDelay(noteEnd_samples);
	}

	LONGLONG noteOffset_samples=(fevent->flag&EVENTFLAG_PRESTART)?samplestart-noteStart_samples:0;// +Tempo

	if(noteOffset_samples<0 || noteOffset_samples>noteLength_samples){
		noteLength_samples=0;
#ifdef DEBUG
		maingui->MessageBoxError(0,"Note noteOffset_samples");
#endif
	}

	//	TRACE ("noteLength_samples %d\n",noteLength_samples);

	if(o->Do_TriggerEvent(sampleoffset,status,key,velocity,velocityoff,(flag&EVENTFLAG_THRUEVENT)?TRIGGER_THRUEVENT|iflag:iflag,noteLength_samples,noteOffset_samples)==true)
	{
		if(createreal==true)
		{
			
#ifdef MEMPOOLS
	NoteOff_Realtime *noteoff=mainpools->mempGetNoteOff_Realtime();
#else
	NoteOff_Realtime *noteoff=new NoteOff_Realtime;
#endif

			if(noteoff)
			{
				// Init NoteOff
				noteoff->audioobject=o;
				noteoff->outputdevice=0;

				noteoff->fromevent=fevent;
				noteoff->rte_frompattern=fpattern;
				noteoff->fromtrack=track;
				noteoff->status=NOTEOFF|GetChannel();
				noteoff->key=key;
				noteoff->velocityoff=velocityoff;
				noteoff->iflag=iflag;

				track->song->realtimeevents[REALTIMELIST_AUDIO].AddRealtimeAlarmEvent(noteoff,noteEnd_samples);
			}
		}
	}
}

// Realtime
bool NoteOff_Realtime::SendToAudio(AudioObject *o,int offset)
{
#ifdef DEBUG
	if(offset<0 || offset>=mainaudio->GetActiveDevice()->GetSetSize())
		maingui->MessageBoxError(0,"NoteOff_Realtime SendToAudio Offset Error\n");
#endif

	SendToProcessor();
	return o->Do_TriggerEvent(offset,status,key,velocityoff,0,TRIGGER_FORCE|iflag);
}

#ifdef MEMPOOLS
void NoteOff_Realtime::DeleteRealtimeEvent()
{
	pool->mainpool->mempDeleteNoteOff_Realtime(this);
}
#endif

bool Control_SustainOn::SendToAudio(AudioObject *o,int offset)
{
	SendToProcessor();
	return o->Do_TriggerEvent(offset,status,64,0,TRIGGER_FORCE);
}

bool Control_Realtime::SendToAudio(AudioObject *o,int offset)
{
	SendToProcessor();
	return o->Do_TriggerEvent(offset,status,controller,value,0);
}

bool Program_Realtime::SendToAudio(AudioObject *o,int offset)
{
	SendToProcessor();
	return o->Do_TriggerEvent(offset,status,program,0,0);
}