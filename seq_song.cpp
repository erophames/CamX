#include "songmain.h"
#include "semapores.h"
#include "object_song.h"
#include "object_track.h"
#include "editfunctions.h"
#include "audiohardware.h"
#include "MIDIhardware.h"
#include "MIDIoutproc.h"
#include "gui.h"
#include "arrangeeditor.h"
#include "audiorealtime.h"
#include "audiohdfile.h"
#include "runningaudiofile.h"
#include "audiodevice.h"
#include "MIDIPattern.h"
#include "drummap.h"
#include "audiorecord.h"
#include "MIDItimer.h"
#include "audioproc.h"
#include "MIDIprocessor.h"
#include "transporteditor.h"
#include "editdata.h"
#include "initplayback.h"
#include "chunks.h"
#include "object_project.h"
#include "languagefiles.h"
#include "stepeditor.h"
#include "audiohardwarechannel.h"

#ifdef WIN32
#include <cmath>
#endif

Seq_Song::Seq_Song(Seq_Project *p)
{
	id=OBJ_SONG;

	project=p;

	MIDIsync.song=
		timetrack.song=
		playbacksettings.song=
		audiosystem.song=
		undo.song=
		metronome.song=
		textandmarker.song=
		timetrack.song=this;

	InitInternSampleRate();

	refreshcounter=0;

	stream_samplestartposition=stream_sampleendposition=0;
	stream_bufferindex=0;
	MIDI_samplestartposition=MIDI_sampleendposition=0;

	ResetCycleSync();

	audiosystem.masterchannel.Init(&audiosystem,CHANNELTYPE_MASTER);

	MIDIaudiosynccounter=0;
	loaded=false;
	autoloadMIDI=false;

	songposition=songstartposition=0;
	status=STATUS_STOP;

	saveonlyarrangement=false;
	startaudioinit=false;
	startMIDIinit=false; // Thread Sync Flag

	for(int i=0;i<INITPLAY_MAX;i++){

		nextMIDIplaybacksampleposition[i]=nextMIDIchaineventsampleposition[i]=0; 
		buffer_nextMIDIplaybacksampleposition[i]=buffer_nextMIDIchaineventsampleposition[i]=0;

		nextMIDIplaybackpattern[i]=nextMIDIchainlistplaybackpattern[i]=0;
		nextMIDIPlaybackEvent[i]=0;
		nextMIDIchainlistevent[i]=0;
	}

	activepattern=0;
	activeevent=0;

	solocount=0;
	songlength_measure=mainsettings->defaultsonglength_measure;

	//	CalcSongLength();
	//	textandmarker.InitSongStopMarker(songlength_ticks);

	underdeconstruction=false;
	songname=0;
	directoryname=0;

	// TimeTrack Init
	timetrack.AddNewTempo(TEMPOEVENT_REAL,0,120);
	timetrack.Close();

	// Signature Map Init
	AddNewSignature(1,4,TICK4nd); // default

	steprecordingonoff=false;
	stepstep=3;
	steplength=3; // 1/4

	recordcounter=0;

	mastering=masteringpRepared=false;
	masteringendposition=0;

	// Note Display
	notetype=NOTETYPE_B;

	// General MIDI
	generalMIDI=mainsettings->eventeditorgmmode;
	generalMIDI_drumchannel=9;

	metronome.playback=mainsettings->sendmetroplayback;

	autosavecounter=1; // 1-5

	groupsoloed=false;
	punchrecording=PUNCHOFF;

	playbacksettings.SetCycleMeasure();

	newexterntempochange=false;
	newsongpositionset=false;


	firstfreecorechannel=0;
	firstfreecoretrack=0;

	autoloadsong=false;
	loadwhenactivated=false;
	mastertrack=0;

	inputrouting.InitDevices();

	startaudiorecording=false;

	defaultmasterstart=0;
	defaultmasterend=32*4*SAMPLESPERBEAT;

	masterfile=0;
	masterdirectoryselectedtracks=0;

	errorsongwriting=false;
	audioinputneed=false;

	masteringmode=Seq_Song::SONGMM_MASTERMIX;
	default_masterchannels=CHANNELTYPE_STEREO;

	for(int i=0;i<REALTIME_LISTS;i++)
	{
		realtimeevents[i].type=i;
		realtimeevents[i].song=this;
	}

	trackchildflag=0;
	MIDIrecording=false;
	masteringstopflag=false;

	smpteoffset.song=this;

	pRepareMIDIstartdelay=false;

	record_cyclecounter=0;

	pRepareMIDIstartprecounter=false;
	startMIDIprecounter=false;

	default_masterformat=mainsettings->default_masterformat;
	default_masternormalize=mainsettings->default_masternormalize;
	default_mastersavefirst=mainsettings->default_mastersavefirst;
	default_masterpausesamples=mainsettings->default_masterpausesamples;
	default_masterpausems=mainsettings->default_masterpausems;

	waitforspaceup=false;
	freeze=false;
	masteringlock=false;

	fxshowflag=0;

	freezeindexcounter=0; // freezing Track naming

	focustype=FOCUSTYPE_TRACK;
	mastefilename_autoset=false;

	rinfo_audiopatterncounter=0;
	pref_tracktype=CHANNELTYPE_STEREO;
}

void Seq_Song::CheckPluginTimer()
{
	if(mainaudio->GetActiveDevice() && mastering==false)
	{
		Seq_Track *t=FirstTrack();

		while(t)
		{
			InsertAudioEffect *iae=t->io.audioeffects.FirstInsertAudioEffect();

			while(iae)
			{
				if(iae->audioeffect->timeusage) // micro
				{	
					double h=mainaudio->GetActiveDevice()->samplebufferms;
					h*=1000; // micro

					double h2=iae->audioeffect->timeusage;
					h2/=h;

					//TRACE ("Plugin %s Max Micros %d Max Perc: %f\n",iae->audioeffect->GetEffectName(),iae->audioeffect->timeusage,h2);
				}

				iae=iae->NextEffect();
			}

			t=t->NextTrack();
		}
	}
}

bool Seq_Song::CheckPunch(OSTART s)
{
	if(punchrecording&PUNCHIN)
	{
		if(s>=playbacksettings.cyclestart && s<=playbacksettings.cycleend)return true;
		return false;
	}

	if(punchrecording&PUNCHOUT)
	{
		if(s<playbacksettings.cyclestart || s>playbacksettings.cycleend)return true;
		return false;
	}

	return true;
}

// TimeTrack
Seq_Signature *Seq_Song::AddNewSignature(int measure,int new_nn,int new_dn)
{
	if(Seq_Signature *sig=timetrack.AddNewSignature(measure,new_nn,new_dn))
	{
		CalcSongLength();
		return sig;
	}

	return 0;
}

void Seq_Song::CopyTrackStreamBuffer_Mastering(AudioDevice *device)
{
	// Mix Track Stream to Track Buffer
	Seq_Track *t=FirstTrack();

	while(t)
	{
		// Copy/Mix Audio Data File Buffer -> Track I/O Buffer
		if(t->t_audiofx.stream && t->t_audiofx.stream[0].channelsused)
		{
			if(t->GetMute()==false)
			{
				//	t->t_audiofx.LockInput();
				t->t_audiofx.stream[0].MixAudioBuffer(&t->mix,AudioHardwareBuffer::USETEMPPOSSIBLE /*(!t->FirstChildTrack())?AudioHardwareBuffer::USETEMPPOSSIBLE:0*/);
				//	t->t_audiofx.UnlockInput();

			}

			t->t_audiofx.stream[0].channelsused=0; // reset
		}

		t=t->NextTrack_NoUILock();
	}
}

void Seq_Song::CopyTrackStreamBuffer(AudioDevice *device) // called by Refill Audio Device Buffer - Playback Mode
{
	// Mix Track Stream to Track Buffer
	Seq_Track *t=FirstTrack();

	while(t)
	{
#ifdef DEBUG
		if(t->mix.samplesinbuffer!=device->GetSetSize())
			maingui->MessageBoxError(0,"Track Mix Buffer");

		if(t->t_audiofx.stream==0)
			maingui->MessageBoxError(0,"Track Stream Buffer");
		else
		{
			if( t->t_audiofx.stream[playback_bufferindex].samplesinbuffer!=device->GetSetSize())
				maingui->MessageBoxError(0,"Track Stream Buffer");
		}
#endif

		if(t->GetPeak()->peakused==true)
			t->GetPeak()->Drop(device->devicedroprate);

		// Copy/Mix Audio Data File Buffer -> Track I/O Buffer
		if(t->t_audiofx.stream && t->t_audiofx.stream[playback_bufferindex].channelsused)
		{
			t->t_audiofx.stream[playback_bufferindex].MixAudioBuffer(&t->mix,AudioHardwareBuffer::USETEMPPOSSIBLE /*(!t->FirstChildTrack())?AudioHardwareBuffer::USETEMPPOSSIBLE:0*/);
			t->t_audiofx.stream[playback_bufferindex].channelsused=0; // reset
		}

		t=t->NextTrack_NoUILock();
	}
}

void Seq_Song::StartAudioRecording()
{
	record_cyclecounter=0;

	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->createdatcycleloop=0; // Reset Cycle Counter
		t->t_audiofx.t_audioinputreadindex=t->t_audiofx.t_lastfilledbufferindex; // Reset In/Out

		TRACE ("StartAudioRecording %d\n",t->t_audiofx.t_audioinputreadindex);

		t=t->NextTrack_NoUILock();
	}

	startaudiorecording=true;
}

void Seq_Song::SetTrackIndexs()
{
	int cflag=0,index=0;
	Seq_Track *t=FirstTrack();

	while(t){

		cflag|=1<<t->childdepth;
		t->trackindex=index++;

		t=t->NextTrack_NoUILock();
	}

	trackchildflag=cflag;
}

void Seq_Song::AddChildTrack(Seq_Track *parent,Seq_Track *ct,int flag,int index)
{
	if((!parent) || (!ct))return;

	ct->childdepth=parent->childdepth+1;
	ct->song=this;
	ct->UnSelect();
	ct->parent=parent;

	parent->AddChildTrack(ct,index);

	/*
	// Add Last
	if(index==-1)
	{
	Seq_Track *lct=parent->LastChildTrack();
	tracks.AddPrevO(ct,lct?lct:parent);
	}
	else
	tracks.AddOToIndex(ct,index);
	*/

	if((flag&ADDTRACK_NOACTIVATE)==0)
	{
		if(!GetFocusTrack())
			tracks.activeobject=ct;
	}
}

void Seq_Song::AddTrack(Seq_Track *track,int index,int flag)
{
	if(!track)return;

	track->song=this;
	track->io.audiosystem=&audiosystem;

	track->io.Repair();
	track->t_audiofx.CreateTrackMix(audiosystem.device);

	track->UnSelect();

	if(track->parent)
		((Seq_Track *)track->parent)->AddChildTrack(track,index);
	else
		tracks.AddOToIndex(track,index);

	track->GetPeak()->ClearPeaks();

	if((flag&ADDTRACK_NOACTIVATE)==0)
	{
		if(!GetFocusTrack())
		{
			tracks.activeobject=track;
			track->Select();
		}
	}
}

Seq_Pattern *Seq_Song::GetPatternIndex(int trackindex,int patternindex,int mediatype)
{
	if(Seq_Track *t=GetTrackIndex(trackindex))
		return t->GetPatternIndex(patternindex,mediatype);

	return 0;
}

AutomationTrack *Seq_Song::LastAutomationTrack()
{
	Seq_Track *t=LastTrack();

	while(t){
		if(t->LastAutomationTrack())return t->LastAutomationTrack();
		t=t->PrevTrack();
	}

	return 0;
}

Seq_Track *Seq_Song::FirstParentTrack(){

	Seq_Track *t=FirstTrack();
	while(t){
		if(!t->parent)return t;
		t=t->NextTrack();
	}

	return 0;
}

Seq_Track *Seq_Song::LastParentTrack(){

	Seq_Track *t=LastTrack();
	while(t){
		if(!t->parent)return t;
		t=t->PrevTrack();
	}

	return 0;
}

bool Seq_Song::FindTrack(Seq_Track *track)
{
	Seq_Track *t=FirstTrack();

	while(t){
		if(t==track)return true;
		t=t->NextTrack();
	}

	return false;
}

bool Seq_Song::FindPattern(Seq_Pattern *pt)
{
	Seq_Track *t=FirstTrack();

	while(t){
		Seq_Pattern *p=t->FirstPattern();

		while(p){
			if(p==pt)return true;
			p=p->NextPattern();
		}

		t=t->NextTrack();
	}

	return false;
}


void Seq_Track::SetTrackColourOnOff(bool onoff)
{
	Seq_Track *ft=song->FirstTrack();

	while(ft)
	{
		//if(!ft->parent)
		{
			if(ft==this || ft->IsSelected()==true)
			{
				if(ft->t_colour.showcolour!=onoff)
				{
					ft->t_colour.showcolour=onoff;
					maingui->RefreshColour(ft);
				}
			}
		}

		ft=ft->NextTrack();
	}

	maingui->ClearRefresh();
}

void Seq_Track::SelectTrackColour(guiWindow *win)
{
	//if(t->parent)t=t->parent;
	colourReq req;

	req.OpenRequester(win,&t_colour);

	if(t_colour.changed==true)
	{
		t_colour.showcolour=true;

		Seq_Track *ft=song->FirstTrack();

		while(ft)
		{
			//	if(!ft->parent)
			{
				if(ft==this || ft->IsSelected()==true)
				{
					bool show=true;

					if(ft!=this)
					{
						if(t_colour.Compare(ft->GetColour())==false)
						{
							t_colour.Clone(ft->GetColour());
							show=true;
						}
					}
					else
						show=true;

					if(show==true)
						maingui->RefreshColour(ft);

					ft->GetColour()->changed=false;
				}
			}

			ft=ft->NextTrack();
		}
	}

	maingui->ClearRefresh();
}


void Seq_Track::StopAllofTrack()
{
	//	Seq_Track *init=t;

	//	while(t)
	//	{
	Seq_Pattern *p=FirstPattern();

	while(p)
	{
		p->StopAllofPattern();
		p=p->NextPattern();
	}

	// Delete Recoring Pattern
	for(int i=0;i<MAXRECPATTERNPERTRACK;i++){

		if(audiorecord_audiopattern[i]){

			if(audiorecord_audiopattern[i]->audioevent.audioefile)
			{
				audiorecord_audiopattern[i]->audioevent.audioefile->recordingactive=false; // Stop Buffer Writing
				audiorecord_audiopattern[i]->audioevent.audioefile->deleterecording=true; // Stop Buffer Writing
			}

			DeletePattern(audiorecord_audiopattern[i],true);
			audiorecord_audiopattern[i]=0;
		}
	}

	//		t=t->NextChildTrack(init);
	//childtrack=childtrack?track->NextChildTrack(childtrack):track->FirstChildTrack();

	//	}while(t);
}

void Seq_Track::DeleteTrackData(bool full)
{
	if(song->underdeconstruction==false)
	{
		StopAllofTrack();

		if(Seq_TrackRecord *tr=t_audiofx.FirstTrackRecord())
		{
			while(tr)
			{
				tr->track->t_audiofx.recordtrack=0;
				tr=tr->NextTrackRecord();
			}
		}

		if(t_audiofx.recordtrack)
			t_audiofx.recordtrack->t_audiofx.RemoveTrackRecord(this,full);
	}

	// Remove Childs
	Seq_Track *child=FirstChildTrack();
	while(child)
	{
		Seq_Track *nchild=child->NextChildTrack();
		child->DeleteTrackData(full);
		child=nchild;
	}

	/// ++++ DeleteTrackData
	solobuffer=false;
	SetSolo(false);

	UnSelect();

	DeletePRepairEvents(DELETE_PRepair_NOTEOFFS|DELETE_PRepair_CONTROL|DELETE_PRepair_SONGSTART|DELETE_PRepair_CYCLE);
	mainaudioreal->RemoveTrackFromAudioRealtime(this);
	song->inputrouting.RemoveTrack(this);

	if(full==true)
	{
		Delete(true);
	}
	else{

		if(GetProcessor())
			GetProcessor()->RemoveProcessorFromAlarms();

		// Cut Patterns Clones
		Seq_Pattern *p=FirstPattern();

		while(p){
			p->CutClones();
			p=p->NextPattern();
		}

		//DeleteFrozenData();
	}
}

Seq_Track *Seq_Song::FirstSelectedTrack()
{
	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t->IsSelected()==true)
			return t;
		t=t->NextTrack();
	}

	return 0;
}

Seq_Track *Seq_Song::LastSelectedTrack()
{
	Seq_Track *t=LastTrack();

	while(t)
	{
		if(t->IsSelected()==true)
			return t;

		t=t->PrevTrack();
	}

	return 0;
}

Seq_Pattern *Seq_Song::FirstSelectedPattern()
{
	Seq_Track *t=FirstTrack();
	while(t)
	{
		Seq_Pattern *p=t->FirstPattern();

		while(p)
		{
			if(p->IsSelected()==true)
				return p;

			p=p->NextPattern();
		}

		t=t->NextTrack();
	}


	return 0;
}

void Seq_Track::RemoveAllOtherTracksRecording()
{
	if(Seq_TrackRecord *tr=t_audiofx.FirstTrackRecord())
	{
		while(tr)
		{
			tr->track->t_audiofx.recordtrack=0;
			tr=tr->NextTrackRecord();
		}
	}

	t_audiofx.DeleteTrackRecord();
}

void Seq_Song::DeleteTrack(Seq_Track *track,bool full)
{
	int getactive=-1;

	// Active Track ?
	if(underdeconstruction==false)
	{
		if(GetFocusTrack()==track)
			getactive=GetOfTrack(track);
	}

	track->SetRecordMode(false,track->songstartposition);

	// Remove From Track List
	track->olist->CutObject(track); // Tracks or FolderList

	if(!track->parent)
	{
		if(track->GetGroups()->FirstGroup())
			RefreshGroupSolo();
	}

	track->DeleteTrackData(full);

	// Track counter dead now !

	if(underdeconstruction==false)
	{
		if(getactive>=0)
		{
			if(!(tracks.activeobject=GetTrackIndex(getactive))) // 1. Next
				tracks.activeobject=GetTrackIndex(getactive-1); // 2. Prev

			if(GetFocusTrack())
				GetFocusTrack()->Select();
		}
	}
}

void Seq_Song::InitMetroTrack_Clicks()
{
	Seq_MetroTrack *mt=FirstMetroTrack();

	while(mt)
	{
		// Measure
		{
			AudioRealtime *ar=&mt->metroclick_a;

			ar->DeInit(false,true);
			ar->InitClass(this,mt,mainaudio->metro_a,0,true,0);

			ar->staticrealtime=true;
			ar->Init(audiosystem.device,mainaudio->metro_a);
		}

		// Beat

		for(int i=0;i<4;i++)
		{
			AudioRealtime *ar=&mt->metroclick_b[i];

			ar->DeInit(false,true);
			ar->InitClass(this,mt,mainaudio->metro_b,0,true,0);

			ar->staticrealtime=true;
			ar->Init(audiosystem.device,mainaudio->metro_b);
		}

		mt=mt->NextMetroTrack();
	}
}

Seq_MetroTrack *Seq_Song::DeleteMetroTrack(Seq_MetroTrack *t)
{
	Seq_MetroTrack *ntrack=t->NextMetroTrack();

	t->metroclick_a.DeInit(false,true);

	for(int i=0;i<4;i++)
	t->metroclick_b[i].DeInit(false,true);

	DeleteTrack(t,true);

	return ntrack;
}

Seq_Track *Seq_Song::DeleteTrack_R(Seq_Track *track,bool full)
{	
	Seq_Track *ntrack=track->NextParentTrack();
	DeleteTrack(track,full);
	return ntrack;
}

bool Seq_Song::MoveTrackIndex(Seq_Track *t,int idiff)
{
	if(idiff)
	{
		// Move Childs
		int childs;
		Seq_Track **cl=0;

		if(childs=t->GetCountChildTracks())
		{
			cl=new Seq_Track*[childs];

			if(cl)
			{
				int i=0;
				Seq_Track *c=t->FirstChildTrack();

				while(c && c->IsTrackChildOfTrack(t)==true)
				{
					cl[i++]=c;
					c=c->NextTrack();
				}

				for(int i=0;i<childs;i++)
					tracks.CutQObject(cl[i]);
			}
		}

		bool ok=tracks.MoveOIndex(t,idiff);

		if(cl)
		{
			for(int i=childs-1;i>=0;i--)
				tracks.AddPrevO(cl[i],t);

			tracks.Close();

			delete cl;
		}

		return ok;
	}

	return false;
}

#ifdef OLDIE
Seq_Song *Seq_Song::CreateClone()
{

	Seq_Song *n=new Seq_Song(project);

	if(n){
		n->focustrack=focustrack;

		// Clone AudioSystem
		audiosystem.Clone(&n->audiosystem);

		/*
		// Clone Tracks
		Seq_Track *nt,*t=FirstTrack();

		while(t)
		{
		nt=t->Clone();

		if(nt)
		{
		n->AddTrack(nt,-1);
		}

		t=t->NextTrack();
		}
		*/
	}

	return n;
}
#endif


void Seq_Song::InitNextPreMetro(AudioDevice *device,int index,bool next)
{
	Seq_Signature *sig=timetrack.FindSignatureBefore(songposition);
	Seq_Tempo *tempo=timetrack.GetTempo(songposition);

	if(next==true)
	{
		double h=sig->dn_ticks;

		h*=timetrack.ppqsampleratemul;// ticks->samples
		h*=tempo->tempofactor; // ticks to wait

		metronome.startpremetrosystime_samples[index]+=(LONGLONG)h;

		//	TRACE ("Next PS %d\n",metronome.startpremetrosystime_samples[index]);

		metronome.beat[index]++;
		metronome.precountertodo[index]--;
	}
	else
	{
		metronome.sendpresignaltoMIDI=true;

		// Init
		if(device && device->GetSetSize()>0)
		{
			double h=sig->dn_ticks;

			h*=timetrack.ppqsampleratemul;// ticks->samples
			h*=tempo->tempofactor; // ticks to wait

			h*=metronome.precountertodo[index]+1;

			LONGLONG samples=h;

			double buffer=h;
			double s=device->GetSetSize();
			buffer/=s;

			double h2=ceil(buffer);
			h2*=device->GetSetSize();

			LONGLONG samples2=h2;
			metronome.startpremetrosystime_samples[index]=samples2-samples; // Start Offset
		}
		else
		{
			metronome.startpremetrosystime_samples[index]=maintimer->GetSystemTime(); // Sys Init Time
		}

		metronome.beat[index]=0;
	}

	// TRACE ("InitNextAudioPreMetro ToDo %d\n",metronome.audioprecountertodo);
	// TRACE ("InitNextAudioPreMetro Offset %d\n",metronome.startpreaudiosampleposition);
}

bool Seq_Song::InitMetroAndStream(AudioDevice *device)
{
	bool startaudionew=false;

	// Init Tempo
	Seq_Signature *sig=timetrack.FindSignatureBefore(songposition);

	for(int i=0;i<METROINDEXS;i++)
		metronome.precountertodo[i]=mainsettings->numberofpremetronomes*sig->nn;

	// InitNextPreMetro(0,MIDIMETROINDEX,false); // MIDI Timer init later

	if(device){

		// Audio Pre Metro
		metronome.sendsynctoMIDI=true;

		/*
		//metronome.startpreaudiooffset=1-(metronome.startpreaudiowaitbuffer-h);

		// Init Audio Buffer PerMetroList

		double samples=0,add;

		add=sig->dn_ticks;
		add*=tempo->tempofactor;

		h=(metronome.startprecounterdodo-1)*add;
		//h=metronome.preaudiowaitsamples;
		h/=device->ticksperbuffer;

		TRACE ("Wait BufferToWait %f Samples %f Add %f\n",h,add);

		for(int i=0;i<metronome.startprecounterdodo;i++)
		{
		if(metropreAudio *pA=new metropreAudio)
		{
		double buffer=samples/device->ticksperbuffer;
		pA->buffer=ceil(buffer);

		double offset=pA->buffer*device->ticksperbuffer;
		offset-=samples;
		pA->samplesOffset=(int)offset;

		pA->buffer=metronome.startpreaudiowaitbuffer-pA->buffer;

		int measure=i/sig->nn,
		beats=i-measure*sig->nn;

		if(beats==0)
		pA->click=mainaudio->metro_a;
		else
		pA->click=mainaudio->metro_b;

		metronome.preaudiometrolist.AddStartO(pA);
		}			

		samples+=add;
		}

		#ifdef DEBUG
		metropreAudio *m=(metropreAudio *)metronome.preaudiometrolist.GetRoot();
		while(m)
		{
		TRACE ("PreAudio Buffer %d BOffset %d\n",m->buffer,m->samplesOffset);
		m=(metropreAudio *)m->next;
		}
		#endif
		*/

		mainaudio->SetAllDevicesStartPosition(this,songposition,AudioDevice::SETSTART_INIT);

		//device->devicerecordstartposition=songposition;

		// Create first x Audio Playbackbuffer
		mainaudiostreamproc->Lock();
		CreateAudioStream(device,CREATESTREAMINIT);
		mainaudiostreamproc->Unlock();
	}
	else
	{
		// No Audio Just MIDI
		metronome.sendsynctoMIDI=false;
	}

	return startaudionew;
}

void Seq_Song::StartSongPlayback(int mode,int playstatus)
{	
	if(this!=mainvar->GetActiveSong())return;

	SetMIDIRecordingFlag();

	if(AudioDevice *device=mainaudio->GetActiveDevice()) // Reset Audio Device Sync
	{
		device->deviceoutofsync=device->deviceoutofMIDIsync=false;

		device->LockTimerCheck_Input();
		device->LockTimerCheck_Output();

		device->timeforrefill_maxsystime=device->timeforaudioinputrefill_maxsystime=0;

		device->UnlockTimerCheck_Input();
		device->UnlockTimerCheck_Output();

	}

	// Reset Stream Timing Check
	mainaudiostreamproc->LockTimerCheck();
	mainaudiostreamproc->timeforrefill_maxms=0;
	mainaudiostreamproc->UnlockTimerCheck();

	songstartposition=songposition;

	{
		Seq_Track *t=FirstTrack();
		while(t)
		{
			t->songstartposition=songposition;
			t=t->NextTrack();
		}
	}

	stream_samplestartposition=MIDI_samplestartposition=timetrack.ConvertTicksToTempoSamples(songposition);

	if(playstatus!=STATUS_STEPRECORD)
	{
		if( (mode&MEDIATYPE_AUDIO) && audiosystem.device && playstatus!=STATUS_STEPRECORD)
		{
			TRACE ("StartSongPlayback InitMetroAndStream\n");
			InitMetroAndStream(audiosystem.device);
		}
		else
			InitMetroAndStream(0);
	}

	TRACE ("StartSongPlayback ResetMaxPeaks\n");
	audiosystem.ResetMaxPeaks();

	TRACE ("StartSongPlayback PRepairPreStartAndCycleEvents\n");
	PRepairPreStartAndCycleEvents(songposition); // Crtl Events before SongPosition (Ctrl,Notes etc..)

	if(mode&MEDIATYPE_MIDI) // before set status !!!
	{
		TRACE ("StartSongPlayback SendPRepairControl\n");
		//SendSysExAtSongPosition();

		SendPRepairControl(false,SENDPRE_AUDIO|SENDPRE_MIDI);

		if((!(playstatus&STATUS_MIDIWAIT)) && (!(playstatus&STATUS_WAITPREMETRO)) && (!(playstatus&STATUS_STEPRECORD)))
		{
			TRACE ("StartSongPlayback SendPRepairNotes\n");
			SendPRepairNotes(false,SENDPRE_AUDIO|SENDPRE_MIDI);	
		}
		else
			DeleteAllPreEvents(DELETE_PRepair_SONGSTART|DELETE_PRepair_NOTEOFFS); // Delete Notes
	}

	TRACE ("StartSongPlayback SendPreStartPrograms\n");
	SendPreStartPrograms();

	TRACE ("Windows Start Song\n");

	{
		guiWindow *w=maingui->FirstWindow();
		while(w)
		{
			if(w->WindowSong()==this)
				w->StartSong();

			w=w->NextWindow();
		}
	}

	//playback_sampleposition=stream_samplestartposition=timetrack.ConvertTicksToTempoSamples(0,songstartposition);

	ResetCycleSync();

	if(playstatus!=STATUS_STEPRECORD)
	{
		//		int h=playstatus;

		//		h CLEARBIT STATUS_WAITPREMETRO;
		//		h CLEARBIT STATUS_MIDIWAIT;

		// h |=STATUS_SONGPLAYBACK_MIDI;

		//	startstatus=h; // set by Audio or MIDI thread 

		TRACE ("StartSongPlayback StartThreadPlayback\n");
		StartThreadPlayback(playstatus);
	}
	else
		status=playstatus;

	//TRACE ("stream_samplestartposition= %d\n",stream_samplestartposition);
	//TRACE ("playback_sampleposition= %d\n",playback_sampleposition);
	//TRACE ("MIDI_samplestartposition= %d\n",MIDI_samplestartposition);

	TRACE ("StartSongPlayback End\n");
}

bool Seq_Song::CheckForPattern(Seq_Pattern *pattern)
{
	Seq_Track *t=FirstTrack();

	while(t){
		Seq_Pattern *p=t->FirstPattern();

		while(p){
			if(p==pattern)return true;
			p=p->NextPattern();
		}

		t=t->NextTrack();
	}

	return false;
}

void Seq_Song::DeletePatternFromLink(Seq_Pattern *p)
{
	if(p && p->link){
		p->link->RemovePattern(p);

		if(!p->link->FirstLinkedPattern()) // Empty Link ?
			patternlinks.RemoveO(p->link);

		p->link=0;
	}
}

PatternLink *Seq_Song::CreatePatternLink()
{
	if(PatternLink *pl=new PatternLink(this)){
		patternlinks.AddEndO(pl);
		return pl;
	}

	return 0;
}

PatternLink *Seq_Song::FirstPatternLink(){return (PatternLink *)patternlinks.GetRoot();}

PatternLink *Seq_Song::DeletePatternLink(PatternLink *p)
{
	return (PatternLink *)patternlinks.RemoveO(p);
}

bool Seq_Track::ParentShowChilds()
{
	Seq_Track *p=(Seq_Track *)parent;

	while(p)
	{
		if(p->showchilds==false)
			return false;

		p=(Seq_Track *)p->parent;
	}

	return true;
}

bool Seq_Track::IsTrackAudioFreezeTrack()
{
	if(frozen==true)
		return true;

	// Find Audio Channel Instruments
	if(FirstPattern(MEDIATYPE_MIDI)) // Trigger Events ?
	{
		if(CheckIfTrackIsAudioInstrument()==true)
			return true;
	}

	if(FirstPattern(MEDIATYPE_AUDIO)) // Trigger Events ?
	{
		if(io.bypassallfx==false && io.audioeffects.FirstActiveAndNonBypassEffectWithInput())
			return true;
	}

	return false;
}

bool Seq_Track::IsTrackAudioMasterTrack()
{
	if(GetMute()==true)
		return false;

	if(FirstPattern(MEDIATYPE_AUDIO))
		return true;

	// Find Audio Channel Instruments
	if(FirstPattern(MEDIATYPE_MIDI)) // Trigger Events ?
	{
		if(CheckIfTrackIsAudioInstrument()==true)
			return true;
	}

	return false;
}

bool Seq_Song::CheckPreMetronome()
{
	int index=MIDIMETROINDEX;

	// MIDI 
	//if(status&STATUS_WAITAUDIOPREMETROCANCELED)
	//	return true;

	Seq_Tempo *tempo=timetrack.GetTempo(songposition);

	// MIDI
	LONGLONG systime=maintimer->GetSystemTime();
	double h=maintimer->ConvertSysTimeToInternTicks(systime-metronome.startpremetrosystime_samples[index]); // Ticks

	if(h>=metronome.waitMIDIoffsetticks)
	{
		metronome.waitMIDIoffsetticks=0;

		Seq_Signature *sig=timetrack.FindSignatureBefore(songposition);
		h/=tempo->tempofactor; // Add PreCounter Tempo

		if(metronome.beat[index]==0 || h>=sig->dn_ticks)
		{
			metronome.startpremetrosystime_samples[index]=systime;
			int measure=metronome.beat[index]/sig->nn,beats=metronome.beat[index]-measure*sig->nn;

			metronome.SendClick(beats==0?true:false);

			double w=sig->dn_ticks;
			w*=tempo->tempofactor;

			w*=timetrack.ppqsampleratemul; // -> Samples

			metronome.waitpreMIDIticks=w;
			metronome.precountertodo[index]--;

			if(metronome.precountertodo[index]<0)
				return true;

			metronome.beat[index]++;
		}
		else
		{
			double w=sig->dn_ticks-h;
			w*=tempo->tempofactor;
			w*=timetrack.ppqsampleratemul; // -> Samples

			//w=timetrack.ConvertInternRateToSamples(w);

			metronome.waitpreMIDIticks=w;
		}
	}
	else
	{
		double w=metronome.waitMIDIoffsetticks-h;
		w*=tempo->tempofactor;
		w*=timetrack.ppqsampleratemul; // -> Samples

		//w=timetrack.ConvertInternRateToSamples(w);

		metronome.waitpreMIDIticks=w;
	}

	return false;
}

bool Seq_Song::CheckPreMetronome(AudioDevice *device)
{
	int index=AUDIOMETROINDEX;

	// Audio 
	if(status&STATUS_WAITAUDIOPREMETROCANCELED)
	{
		status CLEARBIT STATUS_WAITAUDIOPREMETROCANCELED;
		return true;	
	}

	// Audio
	if(metronome.startpremetrosystime_samples[index]<device->GetSetSize())
	{
		while(metronome.startpremetrosystime_samples[index]<device->GetSetSize())
		{
			if(metronome.precountertodo[index]<1)
				return true;

			int sampleoffset=(int)metronome.startpremetrosystime_samples[index];

			if(sampleoffset>=0 && sampleoffset<device->GetSetSize())
			{
				Seq_Signature *sig=timetrack.FindSignatureBefore(songposition);
				int measure=metronome.beat[index]/sig->nn,beats=metronome.beat[index]-measure*sig->nn;

				//TRACE ("PM %d PB %d Off %d\n",measure,beats,sampleoffset);

				Seq_MetroTrack *mt=FirstMetroTrack();

				while(mt)
				{
					if(mt->GetMute()==false && mt->metrosendtoaudio==true)
					{
						AudioRealtime *ar=0;
						AudioHDFile *ahd=0;

						if(beats==0)
						{
							// Measure
							ar=&mt->metroclick_a;
							ahd=mainaudio->metro_a;
						}
						else
						{
							//Beat
							ar=&mt->metroclick_b[mt->metroclickcount];

							if(mt->metroclickcount==3)
								mt->metroclickcount=0;
							else
								mt->metroclickcount++;

							ahd=mainaudio->metro_b;
						}

						mainaudioreal->AddAudioRealtime_AR(this,device,mt,0,ar,ahd,0,0,sampleoffset,true);

						//mainaudioreal->AddAudioRealtime(this,device,mt,0,beats==0?mainaudio->metro_a:mainaudio->metro_b,0,0,sampleoffset,true);
					}

					mt=mt->NextMetroTrack();
				}
			}
#ifdef DEBUG
			else
				maingui->MessageBoxError(0,"Audio Premeto Offset");///
#endif


			if(metronome.sendpresignaltoMIDI==true) // Signal to MIDI PreCounter Start?
			{
				metronome.sendpresignaltoMIDI=false; // Reset

				pRepareMIDIstartprecounter=true;
				pRepareMIDIstartprecounterms=mainaudio->ConvertSamplesToMs(device->GetOutputLatencySamples());

				MIDIstartthread->SetSignal(); // Add Output Latency - Start/Sync MIDI
			}

			InitNextPreMetro(device,index,true);

			metronome.startpremetrosystime_samples[index]-=device->GetSetSize();
		}
	}
	else
		metronome.startpremetrosystime_samples[index]-=device->GetSetSize();

	return false;
}

OSTART Seq_Song::ConvertBeatsToTicks(OSTART beats)
{
	OSTART ticks=0;
	Seq_Signature *sig=timetrack.FirstSignature();

	while(sig && beats)
	{
		if(sig->NextSignature())
		{
			OSTART diff=sig->NextSignature()->GetSignatureStart()-sig->GetSignatureStart(),
				b=diff/sig->dn_ticks;

			if(beats>=b){
				ticks+=b*sig->dn_ticks;
				beats-=b;
			}
			else{
				ticks+=beats*sig->dn_ticks;
				beats=0;
			}
		}
		else
			ticks+=beats*sig->dn_ticks;

		sig=sig->NextSignature();
	}

	return ticks;
}

OSTART Seq_Song::ConvertTicksToBeats(OSTART ticks)
{
	OSTART beats=0;

	Seq_Signature *sig=timetrack.FirstSignature();
	while(sig)
	{
		if(sig->NextSignature() && sig->NextSignature()->GetSignatureStart()<ticks)
		{
			OSTART diff=sig->NextSignature()->GetSignatureStart()-sig->GetSignatureStart();
			diff/=sig->dn_ticks;
			beats+=diff;
		}
		else
		{
			OSTART diff=ticks-sig->GetSignatureStart();
			diff/=sig->dn_ticks;
			beats+=diff;
		}

		sig=sig->NextSignature();
	}

	return beats;
}

bool Seq_Song::ChangeTempoAtPosition(OSTART position,double newtempo,int iflag)
{
	if(position==-1)
		position=GetSongPosition();

	if((status&Seq_Song::STATUS_PLAY) || (status&Seq_Song::STATUS_RECORD))
	{
		return AddVirtualTempoAtPosition(position,newtempo,iflag);
	}

	Seq_Tempo *t=timetrack.GetTempo(position);

	if(newtempo!=t->tempo)
	{
		//	if((!editor) || editor->InitTempoEdit(this,t)==true)
		{
			BufferBeforeTempoChanges();
			return t->ChangeTempo(this,t->GetTempoStart(),newtempo,iflag);
		}
	}

	return false;
}

OSTART Seq_Song::GetSongEnd_Pattern()
{
	//OSTART sl=GetSongLength_Ticks();
	OSTART slp=0;
	Seq_Track *dtl=FirstTrack();

	while(dtl)
	{
		OSTART tl=dtl->GetTrackLength();

		if(tl>slp)
			slp=tl;

		dtl=dtl->NextTrack();
	}

	return slp;
}

void Seq_Song::SetSongLength(OSTART sl,bool guirefresh)
{
	if(sl!=songlength_measure){

		songlength_measure=sl;
		songlength_ticks=timetrack.ConvertMeasureToTicks(songlength_measure);

		if(guirefresh==true)
		{
			RepairLoops(RLF_REFRESHGUI|RLF_CHECKREFRESH);
			maingui->RefreshTimeSlider(this);
		}
	}
}

void Seq_Song::EditSongPosition(guiWindow *win,int x,int y,int smpteflag,int editid)
{
	if(!(status&STATUS_RECORD)){

		if(EditData *edit=new EditData){

			edit->checksongrealtimeposition=true;
			edit->song=this;
			edit->win=win;
			edit->x=x;
			edit->y=y;
			edit->title=maingui->GetFPSName("Song Position",smpteflag);
			edit->deletename=true;

			edit->id=editid;

			edit->type=EditData::EDITDATA_TYPE_TIME;
			edit->smpteflag=smpteflag;
			edit->time=GetSongPosition();

			edit->from=0;
			edit->to=GetSongLength_Ticks();

			maingui->EditDataValue(edit);
		}
	}
}

bool Seq_Song::CycleResetSync(int index)
{
	LockCycleSync();
	cycleresetcounter[index]++;

	if(index==CYCLE_RESET_MIDI && mainaudio->GetActiveDevice())
	{
		int audiodiff=cycleresetcounter[CYCLE_RESET_MIDI]-cycleresetcounter[CYCLE_RESET_AUDIO];

		if(audiodiff<-1 || audiodiff>1)
			cycleoutofsync=true;
	}

	UnlockCycleSync();

#ifdef DEBUG
	//TRACE (" ### CYCLERESET SYNC ### %d\n",index); 
	//for(int i=0;i<CYCLE_RESET_SYNCEND;i++)
	//	TRACE("i=%d -> %d\n",i,cycleresetcounter[i]);	
#endif

	return true;
}

void Seq_Song::InitNewSongPositionToStreams()
{
	playback_sampleposition=stream_samplestartposition=record_sampleposition[0]=timetrack.ConvertTicksToTempoSamples(songposition);
}

void Seq_Song::CycleReset_All(int index)
{
	{
		LockCopyRecordData(); // MIDI+Audio Cycle Reset
		CopyRecordDataToRecordPattern();
		UnlockCopyRecordData(); // Main+MIDI+Audio Cycle Reset
	}

	{
		InitPlay init(index,playbacksettings.cyclestart);
		init.mode=MEDIATYPE_MIDI;

		Seq_Track *t=FirstTrack();

		while(t)
		{
			if(t->record_MIDIPattern){

				t->record_MIDIPattern->ConvertOpenRecordingNotesToNotes(playbacksettings.cycleend);
				t->record_MIDIPattern->CloseEvents(); // Index+Note Chain List
				
				// New MIDI Events on this Track
				t->record_MIDIPattern->InitPlayback(&init,1);

				t->record_MIDIPattern->InitSwing();

				if(!t->playback_MIDIPattern[index][1])
				{
					t->playback_MIDIPattern[index][1]=t->playback_chainnoteMIDIPattern[index][1]=t->record_MIDIPattern; // No Cycle MIDI PAttern, set rec Pattern
				}
				else
				{
					// Track Has MIDI Cycle Playback Pattern
					if(t->playback_MIDIPattern[index][1]!=t->record_MIDIPattern)
					{
						if(t->GetOfPattern(t->record_MIDIPattern,MEDIATYPE_MIDI)<t->GetOfPattern(t->playback_MIDIPattern[index][1],MEDIATYPE_MIDI))
							t->playback_MIDIPattern[index][1]=t->playback_chainnoteMIDIPattern[index][1]=t->record_MIDIPattern; // set rec Pattern <<< old
					}
				}

				SetPatternMuteFlag(t,t->record_MIDIPattern);
				AddSinglePatternToPRepairList(t->record_MIDIPattern,playbacksettings.cyclestart,1);
			}

			// Set MIDI Start Events
			{
				Seq_Pattern *p=t->playback_MIDIPattern[index][0]=t->playback_MIDIPattern[index][1];

				while(p){
					((MIDIPattern *)p)->playback_MIDIevent[index][0]=((MIDIPattern *)p)->playback_MIDIevent[index][1];
					p=p->NextPattern(MEDIATYPE_MIDI);
				}	
			}

			// Set Note Chain Start Events
			{
				Seq_Pattern *p=t->playback_chainnoteMIDIPattern[index][0]=t->playback_chainnoteMIDIPattern[index][1];

				while(p){
					((MIDIPattern *)p)->playback_nextchainevent[index][0]=((MIDIPattern *)p)->playback_nextchainevent[index][1];
					p=p->NextPattern(MEDIATYPE_MIDI);
				}	
			}

			t->ResetAutomationTracksCycle(playbacksettings.cyclestart,index);

			t=t->NextTrack_NoUILock();
		}

		audiosystem.masterchannel.ResetAutomationTracksCycle(playbacksettings.cyclestart,index); // Master
		AudioChannel *ac=audiosystem.FirstBusChannel();
		while(ac){
			ac->ResetAutomationTracksCycle(playbacksettings.cyclestart,index);
			ac=ac->NextChannel();
		}

	}

	// Mix MIDI/ Init Next MIDI Output Event
	cMIDIPlaybackEvent nmpbe(index);
	MixNextMIDIPlaybackEvent(&nmpbe);

	SendPRepairNotes(true,index==INITPLAY_AUDIOTRIGGER?SENDPRE_AUDIO:SENDPRE_MIDI);
	SendPRepairControl(true,index==INITPLAY_AUDIOTRIGGER?SENDPRE_AUDIO:SENDPRE_MIDI);
}

void Seq_Song::CycleReset_Audio(AudioDevice *device)
{
	SendAudioWaitEvents(device,playback_sampleposition,playback_sampleendposition,true); // Send All Waiting Events + Offset
	
	playback_samplesize=playback_setSize;
	playback_sampleoffset=device->GetSetSize()-playback_samplesize;
	playback_sampleposition=playbacksettings.cycle_samplestart;

	CycleReset_All(INITPLAY_AUDIOTRIGGER);
	metronome.InitMetroClick(playbacksettings.cyclestart,AUDIOMETROINDEX);
	CycleResetSync(CYCLE_RESET_AUDIO);
}

void Seq_Song::SyncAudioRecording(AudioDevice *device)
{
	if(mainsettings->autoaudioinput==false || audioinputneed==true)
	{
		if(startaudiorecording==true)
		{
			device->LockRecordBuffer();
			device->recordbuffer_readwritecounter++;
			device->UnlockRecordBuffer();

			audiorecordthread->SetSignal();
		}
	}
}

void Seq_Song::SongAudioDataInput(AudioDevice *device,bool withhardware)
{

	if(mainsettings->autoaudioinput==false || audioinputneed==true)
	{
		LONGLONG t1=maintimer->GetSystemTime();

		//RefillAudioInput(device,withhardware); // Copy HardwareChannel ARES Input -> Track Input

		// Drop Tracks Input Peak
		if(withhardware==true)
		{
			int c=0;
			firstfreeaudioinputtrack=0;

			Seq_Track *lasttrack=0,*t=FirstTrack();

			while(t){

				if(t->io.inputpeaks.peakused==true)
					t->io.inputpeaks.Drop(device->devicedroprate);

				Seq_TrackFX *fx=&t->t_audiofx;

				t->t_audiofx.t_inputbuffers[t->t_audiofx.t_audioinputwriteindex].channelsused=0;
				fx->t_lastfilledbufferindex=fx->t_audioinputwriteindex;

				if(fx->t_audioinputwriteindex==fx->inputbufferscounter-1)
					fx->t_audioinputwriteindex=0;
				else
					fx->t_audioinputwriteindex++;

				if((!fx->recordtrack) && 
					t->io.in_vchannel &&
					t->io.audioinputeffects.CheckIfEffectHasOnInstruments()==false && // Skip Tracks with MIDI Instruments
					(t->io.inputmonitoring==true || t->io.thru==true || (t->record==true && t->recordtracktype==TRACKTYPE_AUDIO))
					)
				{
					if(!firstfreeaudioinputtrack)
						firstfreeaudioinputtrack=t;
					else
						lasttrack->nextcoreaudioinputtrack=t;

					lasttrack=t;
					c++;
				}

				t=t->NextTrack_NoUILock();
			}

			if(lasttrack)
				lasttrack->nextcoreaudioinputtrack=0;

			if(c)
			{
				if(mainvar->cpucores==1)
				{
					while(firstfreeaudioinputtrack){
						firstfreeaudioinputtrack->DoAudioInput(device);
						firstfreeaudioinputtrack=firstfreeaudioinputtrack->nextcoreaudioinputtrack;
					}
				}
				else
				{
					if(c==1)
						firstfreeaudioinputtrack->DoAudioInput(device);
					else
						mainaudioinproc->DoFunction(this,DOCORETRACKSINPUT,c);
				}

				//sendrecordsignal=true;
			}

		}
		else
		{
			Seq_Track *t=FirstTrack();

			while(t){

				// In
				if(t->io.inputpeaks.peakused==true)
					t->io.inputpeaks.Drop(device->devicedroprate);

				t->t_audiofx.t_inputbuffers[t->t_audiofx.t_audioinputwriteindex].channelsused=0; // Reset
				//else
				//	hwb->hwbflag=0;

				t->t_audiofx.t_lastfilledbufferindex=t->t_audiofx.t_audioinputwriteindex;

				if(t->t_audiofx.t_audioinputwriteindex==t->t_audiofx.inputbufferscounter-1)
					t->t_audiofx.t_audioinputwriteindex=0;
				else
					t->t_audiofx.t_audioinputwriteindex++;

				t=t->NextTrack_NoUILock();
			}
		}

		if(int c=CreateTrackListWithAudioInputEffects())
		{
			if(mainvar->cpucores==1)
			{
				while(firstfreeaudioinputtrack){
					firstfreeaudioinputtrack->DoAudioInputEffects(device);
					firstfreeaudioinputtrack=firstfreeaudioinputtrack->nextcoreaudioinputtrack;
				}
			}
			else
			{
				// Track Audio Input Effects
				if(c==1)
					firstfreeaudioinputtrack->DoAudioInputEffects(device);
				else
					mainaudioinproc->DoFunction(this,DOCORETRACKSINPUTEFFECTS,c);
			}
			//sendrecordsignal=true;
		}

		// Input Peak
		firstfreeaudioinputtrack=0;

		int c=0;

		Seq_Track *lasttrack=0;

		// Tracks
		Seq_Track *t=FirstTrack();

		while(t)
		{
			if(t->io.tempinputmonitoring==true && t->io.audioinputeffects.FirstActiveAudioEffect())
			{
				if(!firstfreeaudioinputtrack)firstfreeaudioinputtrack=t;
				if(lasttrack)lasttrack->nextcoreaudioinputtrack=t;
				lasttrack=t;
				c++;
			}

			//t->io.CheckInputPeak(t->GetVIn());
			t=t->NextTrack_NoUILock();
		}

		if(lasttrack)
			lasttrack->nextcoreaudioinputtrack=0;

		if(c)
		{
			if(mainvar->cpucores==1)
			{
				while(firstfreeaudioinputtrack)
				{
					firstfreeaudioinputtrack->DoAudioInputPeak();
					firstfreeaudioinputtrack=firstfreeaudioinputtrack->nextcoreaudioinputtrack;
				}
			}
			else
			{
				// Track Audio Input Effects
				if(c==1)
					firstfreeaudioinputtrack->DoAudioInputPeak();
				else
					mainaudioinproc->DoFunction(this,DOCORETRACKSINPUTPEAK,c);
			}
			//sendrecordsignal=true;


			LONGLONG t2=maintimer->GetSystemTime();

			//	double ms=maintimer->ConvertSysTimeToMs(t2-t1);

			device->LockTimerCheck_Input();

			if(t2-t1>device->timeforaudioinputrefill_maxsystime)
				device->timeforaudioinputrefill_maxsystime=t2-t1;

			//if(t2-t1>timeforrefill_systime)
			device->timeforaudioinpurefill_systime=t2-t1;

			device->UnlockTimerCheck_Input();

		}
	}
	else
	{
		// Reset Thru ChannelsUsed, use empty buffers
		Seq_Track *t=FirstTrack();

		while(t){

			if(t->t_audiofx.t_inputbuffers)
				t->t_audiofx.t_inputbuffers[t->t_audiofx.t_audioinputwriteindex].channelsused=0; // Reset

			t->t_audiofx.t_lastfilledbufferindex=t->t_audiofx.t_audioinputwriteindex;

			t=t->NextTrack_NoUILock();
		}
	}
}

void Seq_Song::CycleReset_Stream(AudioDevice *device)
{
	if(device)
	{
		//	playback_samplesize=playback_setSize;
		//device->playback_sampleoffset=device->setSize-playback_samplesize;

		stream_buffersize=stream_setSize;
		stream_sampleoffset=device->GetSetSize()-stream_setSize;	
	}
	else{
		stream_buffersize=0;
		stream_sampleoffset=0;
	}

	stream_samplestartposition=playbacksettings.cycle_samplestart;
	stream_sampleendposition=playbacksettings.cycle_samplestart+stream_buffersize;

	DeleteAllRunningAudioFiles();

	InitPlay init(INITPLAY_MIDITRIGGER,playbacksettings.cyclestart);
	init.mode=MEDIATYPE_AUDIO|MEDIATYPE_AUDIO_RECORD;
	init.initstream=true;
	InitSongPlayback(&init); // Audio+MIDI -----------------> Playback Main function

	CycleResetSync(CYCLE_RESET_STREAM);
}

void Seq_Song::CreateRecordingCloneTrack(Seq_Track *t,Seq_Track *ct,int cycleloop,bool copyaudio)
{
	if(mainsettings->automutechildsparent==true && (t->parent || ct->parent))
	{
		t->cyclerecordmute=true;
	}

	t->SetRecordMode(false,0); // Record Off

	ct->record_cyclecreated=true;
	ct->createdatcycleloop=cycleloop;

	ct->recordbeforestart=false;
	ct->SetRecordMode(true,GetSongPosition()); // Record On

	if(!ct->parent)
		ct->parent=t->parent;

	ct->InitDefaultAutomationTracks(ct,0);

	//if(ct->parent)
	AddTrack(ct,t->GetTrackIndex()+1);
	//else
	//	AddTrack(ct,t->olist->GetIx(t)+1);

	// Copy Input Buffers + Buffer Info

	//t_lastfilledbufferindex,t_lastfilledbufferthruindex

	if(copyaudio==true)
	{
		// Copy 
		ct->t_audiofx.t_lastfilledbufferindex=t->t_audiofx.t_lastfilledbufferindex;
		//ct->t_audiofx.t_lastfilledbufferthruindex=t->t_audiofx.t_lastfilledbufferthruindex;
		ct->t_audiofx.t_audioinputreadindex=t->t_audiofx.t_audioinputreadindex;
		ct->t_audiofx.t_audioinputwriteindex=t->t_audiofx.t_audioinputwriteindex;

		for(int i=0;i<ct->t_audiofx.inputbufferscounter;i++)
		{
			ct->t_audiofx.t_inputbuffers[i].channelsused=0; // Reset
			t->t_audiofx.t_inputbuffers[i].CopyAudioBuffer(&ct->t_audiofx.t_inputbuffers[i]);
		}

		TRACE ("New Audio Track %d Loop %d \n",t->t_audiofx.t_audioinputreadindex,cycleloop);
	}

	mainsettings->createnewtrack_trackadded=true;
}

void Seq_Song::CheckMIDICycleRecordingForNewTracks(int cycleloop)
{
	// Auto Add Track and Cylce End ?
	if( (status&STATUS_RECORD) && (mainsettings->createnewtrackaftercycle==true /* || mainsettings->createnewchildtrackwhenrecordingstarts==true*/ ))
	{
		Seq_Track **tracks=0;
		Seq_Track **newcreatedtracks=0;

		mainthreadcontrol->LockActiveSong();

		int MIDIrecordtracks=0,newtracks=0;

		{
			Seq_Track *t=FirstTrack();

			while(t){

				if(t->record_cyclecreated==true && mainsettings->automutechildsparent==true && t->parent)
					t->cyclerecordmute=true;

				if(t->record==true){
					if(t->record_MIDIPattern)
						MIDIrecordtracks++;
				}

				t=t->NextTrack_NoUILock();
			}
		}

		if(MIDIrecordtracks){

			char hs[NUMBERSTRINGLEN];

			if(tracks=new Seq_Track*[MIDIrecordtracks]){

				// Init List
				{
					int i=0;
					Seq_Track *t=FirstTrack();

					while(t){

						if(t->record==true)
						{
							if(t->record_MIDIPattern)
								tracks[i++]=t;
						}

						t=t->NextTrack_NoUILock();
					}
				}

				for(int i=0;i<MIDIrecordtracks;i++)
				{
					Seq_Track *t=tracks[i];

					if(Seq_Track *ct=new Seq_Track)
					{
						t->CloneToTrack(ct,false);

						ct->recordcycleindex=t->recordcycleindex+1;

						// New Track Name
						if(char *tname=mainvar->GenerateString(t->GetName()))
						{
							if(t->record_cyclecreated==true){
								size_t i=strlen(tname);
								if(i>0){
									char *h=&tname[i-1];
									while(i--){
										if(*h=='*'){*h=0;break;}
										h--;
									}
								}
							}

							if(char *help=mainvar->GenerateString(tname,"*",mainvar->ConvertIntToChar(ct->recordcycleindex,hs)) )
							{
								ct->SetName(help);
								delete help;
							}

							delete tname;
						}

						CreateRecordingCloneTrack(t,ct,cycleloop,true);
						newtracks++;

					}//if nt
				}// for

				if(newtracks){
					mainthreadcontrol->Lock(CS_UI); // MIDI Avoid OO List Conflicts+crash with UI !
					CreateQTrackList();
					mainthreadcontrol->Unlock(CS_UI);
				}

			}//if list

		}// if new track rec

		mainthreadcontrol->UnlockActiveSong();

		if(tracks)
			delete tracks;
	}
}

void Seq_Song::CheckAudioCycleRecordingForNewTracks(int cycleloop)
{
	// Auto Add Track and Cylce End ?
	if((status&STATUS_RECORD) && (mainsettings->createnewtrackaftercycle==true /* || mainsettings->createnewchildtrackwhenrecordingstarts==true*/ ))
	{
		Seq_Track **tracks=0;

		mainthreadcontrol->LockActiveSong();

		int audiorecordtracks=0,newtracks=0;

		{
			Seq_Track *t=FirstTrack();

			while(t){

				// Check for new recorded Audio Pattern..
				if(t->record==true)
				{
					for(int i=0;i<MAXRECPATTERNPERTRACK;i++)
						if(t->audiorecord_audiopattern[i] && 
							t->audiorecord_audiopattern[i]->audioevent.audioefile && 
							t->audiorecord_audiopattern[i]->audioevent.audioefile->samplesperchannel)
							audiorecordtracks++;
				}

				t=t->NextTrack_NoUILock();
			}
		}

		if(audiorecordtracks)
		{
			char hs[NUMBERSTRINGLEN];

			if(tracks=new Seq_Track*[audiorecordtracks]){

				// Init List
				{
					int i=0;
					Seq_Track *t=FirstTrack();

					while(t){

						if(t->record==true)
						{
							for(int i2=0;i2<MAXRECPATTERNPERTRACK;i2++)
								if(t->audiorecord_audiopattern[i2] && t->audiorecord_audiopattern[i2]->audioevent.audioefile && t->audiorecord_audiopattern[i2]->audioevent.audioefile->samplesperchannel)
									tracks[i++]=t;
						}

						t=t->NextTrack_NoUILock();
					}
				}

				for(int i=0;i<audiorecordtracks;i++)
				{
					Seq_Track *t=tracks[i];

					if(Seq_Track *ct=new Seq_Track)
					{
						// Get Parent Track
						Seq_Track *parent=(Seq_Track *)t->parent;

						t->CloneToTrack(ct,false);

						ct->recordcycleindex=t->recordcycleindex+1;

						// New Track Name
						if(char *tname=mainvar->GenerateString(t->GetName()))
						{
							if(t->record_cyclecreated==true)
							{
								size_t i=strlen(tname);
								if(i>0)
								{
									char *h=&tname[i-1];
									while(i--)
									{
										if(*h=='*'){*h=0;break;}
										h--;
									}
								}
							}

							if(char *help=mainvar->GenerateString(tname,"*",mainvar->ConvertIntToChar(ct->recordcycleindex,hs)) )
							{
								ct->SetName(help);
								delete help;
							}

							delete tname;
						}

						CreateRecordingCloneTrack(t,ct,cycleloop,true);
						newtracks++;

					}//if nt

				} // for

				if(newtracks)
				{
					mainthreadcontrol->Lock(CS_UI); // Audio Avoid OO List Conflicts+crash with UI !
					CreateQTrackList();
					mainthreadcontrol->Unlock(CS_UI);
				}


			}//if list

		}//if

		mainthreadcontrol->UnlockActiveSong();

		if(tracks)
			delete tracks;
	}
}

void Seq_Song::CycleReset_MIDI() // called by MIDI Thread !!!!
{
	LockSongPosition();
	status|=STATUS_MIDICYCLERESET; // GetSongPosition=cycleplaybackstart
	cyclestartposition=playbacksettings.cyclestart;
	UnlockSongPosition();

	SendAllNoteOffs(true,SANOFF_ONLYMIDI);

	MIDI_samplestartposition=MIDI_sampleendposition=playbacksettings.cycle_samplestart;
	playbacksettings.cyclecounter_MIDI++;

	//audiorecordthread->Unlock();
	//mainMIDIrecord->UnLock();

	// Send MIDI STOP ----------------------------------------------------------------

	if(MIDIsync.sync==SYNC_INTERN)
	{
		if(MIDIsync.sendmc==true)
		{
			for(int i=0;i<MAXMIDIPORTS;i++)
			{
				if(mainMIDI->MIDIoutports[i].visible==true &&
					mainMIDI->MIDIoutports[i].outputdevice && 
					mainMIDI->MIDIoutports[i].outputdevice->lockMIDIstartstop==false &&
					mainMIDI->MIDIoutports[i].sendsync==true)
				{
					mainMIDI->MIDIoutports[i].outputdevice->sendSmallDataRealtime(MIDIREALTIME_STOP);
				}
			}

			// Unlock MIDI Control
			MIDIOutputDevice *mo=mainMIDI->FirstMIDIOutputDevice();

			while(mo)
			{
				mo->lockMIDIstartstop=false;
				mo=mo->NextOutputDevice();
			}
		}

		mainMIDI->SendSongPosition(this,playbacksettings.cyclestart,true);
	}

	CycleReset_All(INITPLAY_MIDITRIGGER);	

	// Init Metro
	metronome.InitMetroClick(playbacksettings.cyclestart,MIDIMETROINDEX);

	// Reset Song MIDI Clock
	MIDIsync.out_nextclockposition=playbacksettings.cyclestart+(SAMPLESPERBEAT/24);
	MIDIsync.out_nextclocksample=timetrack.ConvertTicksToTempoSamples(MIDIsync.out_nextclockposition);

	// Send MIDI Start/Continue ----------------------------------------------------------------
	if(MIDIsync.sync==SYNC_INTERN && MIDIsync.sendmc==true)
	{
		for(int i=0;i<MAXMIDIPORTS;i++)
		{
			if(mainMIDI->MIDIoutports[i].visible==true && mainMIDI->MIDIoutports[i].outputdevice && 
				mainMIDI->MIDIoutports[i].outputdevice->lockMIDIstartstop==false && mainMIDI->MIDIoutports[i].sendsync==true)
			{
				mainMIDI->MIDIoutports[i].outputdevice->sendSmallDataRealtime(playbacksettings.cyclestart==0?MIDIREALTIME_START:MIDIREALTIME_CONTINUE);
				mainMIDI->MIDIoutports[i].outputdevice->lockMIDIstartstop=true;
			}
		}

		// Unlock MIDI Control
		MIDIOutputDevice *mo=mainMIDI->FirstMIDIOutputDevice();

		while(mo)
		{
			mo->lockMIDIstartstop=false;
			mo=mo->NextOutputDevice();
		}
	}

	SetRunningFlag(playbacksettings.cyclestart,true); // + status CLEARBIT STATUS_MIDICYCLERESET;

	CycleResetSync(CYCLE_RESET_MIDI);
	CheckMIDICycleRecordingForNewTracks(cycleresetcounter[CYCLE_RESET_MIDI]); // MIDI + Audio Instruments
	//MixAllAutomationObjects(INITPLAY_MIDITRIGGER,true,playbacksettings.cyclestart); // Reset Automation
}

void Seq_Song::CheckCycleStart()
{
	//playbacksettings.cyclecounter_MIDI=playbacksettings.cyclecounter_audio=0;

	if(playbacksettings.cycleplayback==true)
	{
		if(songposition>=playbacksettings.cycleend)
		{
			SetSongPosition(playbacksettings.cyclestart,0);
			/*
			PRepairPlayback(songposition=playbacksettings.cyclestart,MEDIATYPE_ALL);
			mainMIDI->SendSongPosition(this,true);
			*/
		}
	}
}

void Seq_Song::SongInit()
{
	CreateQTrackList();
	audiosystem.ChangeAllToAudioDevice(mainaudio->GetActiveDevice(),true);
	ConvertTicksToTempoSampleRate();

	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->SetTempInputMonitoring();
		t=t->NextTrack();
	}

	InitMetroTrack_Clicks();

	PRepairPlayback(songposition,MEDIATYPE_ALL);
	SetAutomationTracksPosition(GetSongPosition()); // 0= all tracks, Send Ctrl Events etc.
}

Seq_MIDIRouting_InputDevice *Seq_MIDIRouting::FindInputDevice(MIDIInputDevice *mid)
{
	Seq_MIDIRouting_InputDevice *id=FirstInputDevice();

	while(id)
	{
		if(id->device==mid)
			return id;

		id=id->NextDevice();
	}

	return 0;
}

void Seq_MIDIRouting::Load(camxFile *file)
{
	int inr=0;
	file->ReadChunk(&inr);
	file->CloseReadChunk();

	while(inr--)
	{
		file->LoadChunk();

		if(file->GetChunkHeader()==CHUNK_SONGROUTING_DEVICE)
		{
			char *devicename=0;

			file->ChunkFound();
			file->Read_ChunkString(&devicename);
			file->CloseReadChunk();

			if(devicename)
			{
				if(Seq_MIDIRouting_InputDevice *ip=new Seq_MIDIRouting_InputDevice)
				{
					// Channels
					file->LoadChunk();
					if(file->GetChunkHeader()==CHUNK_SONGROUTING_DEVICE_CHANNELS)
					{
						int channels=0;
						int channelc=0;

						file->ChunkFound();
						file->ReadChunk(&channels);
						file->CloseReadChunk();

						// Track List
						while(channels--)
						{
							file->LoadChunk();
							if(file->GetChunkHeader()==CHUNK_SONGROUTING_DEVICETRACKS)
							{
								file->ChunkFound();
								int tracks=0;
								file->ReadChunk(&tracks);
								file->CloseReadChunk();

								while(tracks--)
								{
									file->LoadChunk();
									if(file->GetChunkHeader()==CHUNK_SONGROUTING_DEVICETRACK)
									{
										file->ChunkFound();

										if(channelc<16)
										{
											if(Seq_MIDIRouting_Router_Track *rt=new Seq_MIDIRouting_Router_Track)
											{
												file->AddPointer((CPOINTER)&rt->track);
												ip->router_channels[channelc].tracks.AddEndO(rt);
											}
										}

										file->CloseReadChunk();
									}
								}
							}

							channelc++;
						}// while channels
					}// CHANNELS

					file->LoadChunk();
					if(file->GetChunkHeader()==CHUNK_SONGROUTING_DEVICE_EVENTS)
					{
						int events=0,eventc=0;

						file->ChunkFound();
						file->ReadChunk(&events);
						file->CloseReadChunk();

						// Track List
						while(events--)
						{
							file->LoadChunk();
							if(file->GetChunkHeader()==CHUNK_SONGROUTING_DEVICETRACKS)
							{
								file->ChunkFound();
								int tracks=0;
								file->ReadChunk(&tracks);
								file->CloseReadChunk();

								while(tracks--)
								{
									file->LoadChunk();
									if(file->GetChunkHeader()==CHUNK_SONGROUTING_DEVICETRACK)
									{
										file->ChunkFound();

										if(eventc<MAXROUTEVENTS)
										{
											if(Seq_MIDIRouting_Router_Track *rt=new Seq_MIDIRouting_Router_Track)
											{
												file->AddPointer((CPOINTER)&rt->track);
												ip->router_events[eventc].tracks.AddEndO(rt);
											}
										}

										file->CloseReadChunk();
									}
								}
							}

							eventc++;
						}//while events
					}// EVENTS

					if(ip->device=mainMIDI->FindMIDIInputDevice(devicename))
						indevices.AddEndO(ip);
					else
					{
						ip->FreeMemory();
						delete ip;
					}

				}// if ip

				delete devicename;
			}
		}
	}
}

void Seq_MIDIRouting::SaveTrackList(Seq_MIDIRouting_Router *router,camxFile *file)
{
	file->OpenChunk(CHUNK_SONGROUTING_DEVICETRACKS);
	file->Save_Chunk(router->tracks.GetCount());
	file->CloseChunk();

	if(router->tracks.GetCount()>0)
	{
		Seq_MIDIRouting_Router_Track *ft=router->FirstTrack();

		while(ft)
		{
			file->OpenChunk(CHUNK_SONGROUTING_DEVICETRACK);
			file->Save_Chunk((CPOINTER)ft->track); //0==Active Track
			file->CloseChunk();

			ft=ft->NextTrack();
		}
	}
}

void Seq_MIDIRouting::Save(camxFile *file)
{
	int nr=indevices.GetCount();

	if(nr>0)
	{
		file->OpenChunk(CHUNK_SONGROUTING);
		file->Save_Chunk(nr);
		file->CloseChunk();

		Seq_MIDIRouting_InputDevice *ip=FirstInputDevice();

		while(ip)
		{
			file->OpenChunk(CHUNK_SONGROUTING_DEVICE);
			file->Save_ChunkString(ip->device->FullName());
			file->CloseChunk();

			file->OpenChunk(CHUNK_SONGROUTING_DEVICE_CHANNELS);
			int channels=16;
			file->Save_Chunk(channels);
			file->CloseChunk();

			// 1. Channels
			for(int i=0;i<16;i++)
				SaveTrackList(&ip->router_channels[i],file);

			file->OpenChunk(CHUNK_SONGROUTING_DEVICE_EVENTS);
			int events=MAXROUTEVENTS;
			file->Save_Chunk(events);
			file->CloseChunk();

			// 2. Events
			for(int i=0;i<MAXROUTEVENTS;i++)
				SaveTrackList(&ip->router_events[i],file);

			// 3. Logic

			ip=ip->NextDevice();
		}
	}
}

void Seq_MIDIRouting::RemoveTrack(Seq_Track *track)
{
	Seq_MIDIRouting_InputDevice *id=FirstInputDevice();
	while(id)
	{
		// Remove from Channels
		for(int c=0;c<16;c++)
		{
			Seq_MIDIRouting_Router_Track *t=id->router_channels[c].FirstTrack();

			while(t)
			{
				if(t->track==track)
				{
					id->LockO();
					id->router_channels[c].tracks.RemoveO(t);
					id->UnlockO();
					break;
				}

				t=t->NextTrack();
			}
		}

		// Remove from Events
		for(int e=0;e<MAXROUTEVENTS;e++)
		{
			Seq_MIDIRouting_Router_Track *t=id->router_events[e].FirstTrack();

			while(t)
			{
				if(t->track==track)
				{
					id->LockO();
					id->router_events[e].tracks.RemoveO(t);
					id->UnlockO();
					break;
				}

				t=t->NextTrack();
			}
		}

		id=id->NextDevice();
	}
}

Seq_MIDIRouting_Router_Track *Seq_MIDIRouting_Router::AddTrack(Seq_Track *t)
{
	Seq_MIDIRouting_Router_Track *c=FirstTrack();

	while(c)
	{
		if(c->track==t)
			return 0;

		c=c->NextTrack();
	}

	if(Seq_MIDIRouting_Router_Track *rt=new Seq_MIDIRouting_Router_Track)
	{
		rt->track=t;

		if(!t)
			tracks.AddStartO(rt);
		else
			tracks.AddEndO(rt);

		return rt;
	}

	return 0;
}

Seq_MIDIRouting_Router_Track *Seq_MIDIRouting_Router::RemoveTrack(Seq_MIDIRouting_Router_Track *t)
{
	return (Seq_MIDIRouting_Router_Track *)tracks.RemoveO(t);
}

bool Seq_MIDIRouting_InputDevice::CheckEvent(Seq_Track *track,UBYTE status,UBYTE b1,UBYTE b2,bool checkfocustrack)
{
	LockO();

	switch(status&0xF0)
	{
	case NOTEON:
	case NOTEOFF:
	case POLYPRESSURE:
	case CONTROLCHANGE:
	case PROGRAMCHANGE:
	case CHANNELPRESSURE:
	case PITCHBEND:
		{
			// 1.Check Channel
			int channel=status&0x0F;

			if(Seq_MIDIRouting_Router_Track *t=router_channels[channel].FirstTrack()) 
			{
				if(t->track==0 && (checkfocustrack==false || track==track->song->GetFocusTrack()))
				{
					UnlockO();
					return true;
				}

				while(t)
				{
					if(t->track==track)
					{
						UnlockO();
						return true;
					}

					t=t->NextTrack();
				}
			}

			// 2. Check Event Type
			{
				int type=Seq_MIDIRouting_InputDevice::NOTES; // Default Notes
				switch(status&0xF0)
				{
				case PITCHBEND:
					type=Seq_MIDIRouting_InputDevice::PITCH;
					break;

				case CONTROLCHANGE:
					type=Seq_MIDIRouting_InputDevice::CCHANGE;
					break;

				case POLYPRESSURE:
					type=Seq_MIDIRouting_InputDevice::PPRESS;
					break;

				case PROGRAMCHANGE:
					type=Seq_MIDIRouting_InputDevice::PCHANGE;
					break;

				case CHANNELPRESSURE:
					type=Seq_MIDIRouting_InputDevice::CPRESS;
					break;
				}

				if(Seq_MIDIRouting_Router_Track *t=router_events[type].FirstTrack()) 
				{
					if(t->track==0 && (checkfocustrack==false || track==track->song->GetFocusTrack()))
					{
						UnlockO();
						return true;
					}

					while(t)
					{
						if(t->track==track)
						{
							UnlockO();
							return true;
						}

						t=t->NextTrack();
					}
				}
			}
		}
		break;

	case INTERN:
	case INTERNCHAIN:
	case SYSEX:
		{
			if(checkfocustrack==false || track==track->song->GetFocusTrack()) // SysEx only  to active Track
			{
				UnlockO();
				return true;
			}

			// 2. Check Event Type
			if(Seq_MIDIRouting_Router_Track *t=router_events[Seq_MIDIRouting_InputDevice::SYS].FirstTrack()) 
			{
				if(t->track==0 && (checkfocustrack==false || track==track->song->GetFocusTrack()))
				{
					UnlockO();
					return true;
				}

				while(t)
				{
					if(t->track==track)
					{
						UnlockO();
						return true;
					}

					t=t->NextTrack();
				}
			}
		}
		break;
	}

	UnlockO();

	return false;
}

void Seq_MIDIRouting::InitDevices()
{
	MIDIInputDevice *mid=mainMIDI->FirstMIDIInputDevice();

	while(mid)
	{
		if(!FindInputDevice(mid))
		{
			if(Seq_MIDIRouting_InputDevice *ip=new Seq_MIDIRouting_InputDevice)
			{
				ip->device=mid;
				indevices.AddEndO(ip);

				ip->LockO();

				// Channels
				for(int c=0;c<16;c++)
					ip->router_channels[c].AddTrack(0);

				// Events
				for(int e=0;e<MAXROUTEVENTS;e++)
					ip->router_events[e].AddTrack(0);

				ip->UnlockO();
			}
		}

		mid=mid->NextInputDevice();
	}
}

void Seq_MIDIRouting_InputDevice::FreeMemory()
{
	// Channels
	LockO();

	for(int c=0;c<16;c++)
		router_channels[c].tracks.DeleteAllO();

	for(int e=0;e<MAXROUTEVENTS;e++)
		router_events[e].tracks.DeleteAllO();

	UnlockO();
}

void Seq_MIDIRouting::FreeMemory()
{
	Seq_MIDIRouting_InputDevice *ip=FirstInputDevice();

	while(ip){

		ip->FreeMemory();
		ip=ip->NextDevice();
	}

	indevices.DeleteAllO();
}

char *Seq_Song::GetName()
{
	if(!songname)
		songname=mainvar->GenerateString("nSong");

	return songname;
}

void Seq_Song::InitInternSampleRate()
{
	timetrack.ppqsampleratemul=project->ppqsampleratemul;
}


void Seq_Song::DeActivated()
{
	if(underdeconstruction==true)
		return;
	/*
	if(mainaudio->GetActiveDevice())
	{
	AudioHardwareChannel *i=mainaudio->GetActiveDevice()->FirstInputChannel();

	while(i)
	{
	for(int i2=0;i2<2;i2++)
	i->inbuffers[i2].channelsused=0;

	i=i->NextChannel();
	}

	if(mainaudio->GetActiveDevice())
	mainaudio->GetActiveDevice()->mix.channelsused=0;
	}
	*/

	// Save Memory (Audio BUffers)

	events_in.DeleteAllO();
	events_out.DeleteAllO();

	TRACE ("Song DeActivated %s",songname);

	for(int i=0;i<LASTSYNTHCHANNEL;i++)
	{
		AudioChannel *c=audiosystem.FirstChannelType(i);

		while(c)
		{
			c->io.inputpeaks.Reset();
			c->mix.peak.Reset();

			c->DeleteChannelBuffersMix();
			c->io.DeleteAllEffectBuffers();

			c=c->NextChannel();
		}
	}

	audiosystem.masterchannel.io.DeleteAllEffectBuffers();

	Seq_Track *t=FirstTrack(); 
	while(t)
	{
		t->io.inputpeaks.Reset();
		t->GetPeak()->Reset();

		t->GetAudioFX()->DeleteTrackMix();
		t->io.DeleteAllEffectBuffers();

		t=t->NextTrack();
	}
}

void Seq_Song::OpenSyncEditor()
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(w->GetEditorID()==EDITORTYPE_SYNCEDITOR && w->WindowSong()==this)
			break;

		w=w->NextWindow();
	}

	if(!w)
	{
		guiWindowSetting set;

		set.startposition_x=0;
		set.startposition_y=0;

		set.startwidth=260;
		set.startheight=maingui->GetFontSizeY()*SYNCEDITORLINES;

		w=maingui->OpenEditorStart(EDITORTYPE_SYNCEDITOR,this,0,0,&set,0,0);
	}
	else
		w->WindowToFront(true);
}

void Seq_Song::OpenRecordEditor()
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(w->GetEditorID()==EDITORTYPE_RECORDEDITOR && w->WindowSong()==this)
			break;

		w=w->NextWindow();
	}

	if(!w)
	{
		guiWindowSetting set;

		set.startposition_x=0;
		set.startposition_y=0;

		set.startwidth=260;
		set.startheight=maingui->GetFontSizeY()*RECORDEDITORLINES;

		w=maingui->OpenEditorStart(EDITORTYPE_RECORDEDITOR,this,0,0,&set,0,0);
	}
	else
		w->WindowToFront(true);
}

void Seq_Song::OpenIt(guiScreen *screen)
{
	mainvar->SetActiveSong(this);

	if(!screen)
	{
		if(maingui->GetActiveScreen() && 
			(maingui->GetActiveScreen()->project==0 || (maingui->GetActiveScreen()->project==project && maingui->GetActiveScreen()->song==0))
			)
			screen=maingui->GetActiveScreen();
	}

	if(!screen)
	{
		maingui->OpenNewScreen(project,this);
	}
	else
	{
		screen->InitNewSong(this);
	}

	maingui->RefreshScreenNames();
}

void Seq_Song::AddMetroTrack(Seq_MetroTrack *track)
{
	if(track)
	{
		track->ismetrotrack=true;

		if(!metrotracks.activeobject)
			metrotracks.activeobject=track;

		metrotracks.AddEndO(track);

#ifdef DEBUG
		Seq_Track *f=FirstMetroTrack();
#endif

		track->song=this;
		track->io.audiosystem=&audiosystem;
		track->t_audiofx.CreateTrackMix(audiosystem.device);

		track->UnSelect();

		track->GetPeak()->ClearPeaks();

		if(!track->GetMIDIOut()->FirstDevice())
			track->GetMIDIOut()->AddToGroup(mainMIDI->GetDefaultDevice());
	}
}

void Seq_Song::InitDefaultMetroTracks()
{
	Seq_MetroTrack *t=new Seq_MetroTrack;

	if(t)
	{
		t->SetVType(mainaudio->GetActiveDevice(),mainsettings->defaultmetroaudiochanneltype,false,false);

		if(mainaudio->GetActiveDevice())
			t->io.SetOutput(&mainaudio->GetActiveDevice()->outputaudioports[mainsettings->defaultmetroaudiochanneltype][mainsettings->defaultmetroaudiochannelportindex]);

		t->SetName(Cxs[CXS_METRONOME]);

		AddMetroTrack(t);
	}
}

void Seq_Song::CheckForCycleReset()
{
	if(status&STATUS_PLAY)
	{
		if(cycleresetcounter[CYCLE_RESET_MIDI]!=cycleresetcounter[CYCLE_RESET_AUDIO] ||
			cycleresetcounter[CYCLE_RESET_MIDI]!=cycleresetcounter[CYCLE_RESET_STREAM] ||
			cycleresetcounter[CYCLE_RESET_AUDIO]!=cycleresetcounter[CYCLE_RESET_STREAM]){
				SetSongPosition(playbacksettings.cyclestart,true); // Reset To Cycle Start
				return;		
		}

		if(playbacksettings.cycleplayback==true){

			if(GetSongPosition()>=playbacksettings.cycleend ||
				stream_samplestartposition>=playbacksettings.cycle_sampleend ||
				(playback_sampleposition>=playbacksettings.cycle_sampleend)
				)
				SetSongPosition(playbacksettings.cyclestart,true); // Reset To Cycle Start
		}
	}
}

void Seq_Song::SetSync(int type,bool bufferold)
{
	if(type!=MIDIsync.sync && IsPlayback()==false && IsRecording()==false)
	{
		if(bufferold==true)
			MIDIsync.syncbeforestart=MIDIsync.sync;
		else
			MIDIsync.syncbeforestart=type;

		MIDIsync.sync=type;
	}
}

void Seq_Song::ToggleCycle()
{
	if(status&STATUS_RECORD)
		return;

	if(this==mainvar->GetActiveSong())
	{
		mainthreadcontrol->LockActiveSong();
		playbacksettings.cycleplayback=playbacksettings.cycleplayback==true?false:true; // Toggle
		CheckForCycleReset();
		mainthreadcontrol->UnlockActiveSong();
	}

	maingui->RefreshAllHeaders(this);
}

void Seq_Song::Activated()
{
	SongInit();
}

void Seq_Song::ResetPlugins()
{
	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t->frozen==false)
			t->io.ResetPlugins();

		t=t->NextTrack();
	}

	AudioChannel *c=audiosystem.FirstBusChannel();
	while(c)
	{
		c->io.ResetPlugins();
		c=c->NextChannel();
	}

	audiosystem.masterchannel.io.ResetPlugins();
}

void Seq_Song::ResetSoloMute()
{
	Seq_Track *t=FirstTrack();

	while(t)
	{
		// Track
		MuteTrack(t,false);
		SoloTrack(t,false);

		if(!t->parent)
		{
			// Groups
			Seq_Group_GroupPointer *g=t->GetGroups()->FirstGroup();
			while(g)
			{
				SetGroupSolo(g->group,g->group->solo==true?false:true);
				g->group->mute=false;

				g=g->NextGroup();
			}
		}

		t=t->NextTrack();
	}
}

int Seq_Song::ReplacedFile(AudioHDFile *file)
{
	if(file)
	{
		int found=0;
		Seq_Track *t=FirstTrack();

		while(t)
		{
			Seq_Pattern *p=t->FirstPattern(MEDIATYPE_AUDIO);

			while(p)
			{
				AudioPattern *ap=(AudioPattern *)p;

				if(ap->audioevent.audioefile==file)
					found++;

				p=p->NextPattern(MEDIATYPE_AUDIO);
			}

			t=t->NextTrack();
		}

		return found;
	}

	return 0;
}

void Seq_Song::RefreshGroupSolo()
{
	if(groupsoloed==true){
		Seq_Group *f=FirstGroup();

		while(f)
		{
			if(f->solo==true)
			{
				Seq_Track *t=FirstTrack();

				while(t)
				{
					if(!t->parent)
					{
						if(t->GetGroups()->FindGroup(f)==true)
							break;
					}

					t=t->NextTrack();
				}

				if(!t)
					groupsoloed=f->solo=false;

				break;
			}

			f=f->NextGroup();
		}
	}
}

void Seq_Song::CheckEffectPipeline(AudioEffects *fx)
{
	InsertAudioEffect *iae=fx->FirstInsertAudioEffect();

	while(iae)
	{
		if(iae->audioeffect->crashed && iae->audioeffect->crashmessage==false)
		{
			iae->audioeffect->crashmessage=true;

			char h2[NUMBERSTRINGLEN],*en=mainvar->ConvertIntToChar(iae->audioeffect->crashed,h2);

			if(char *h=mainvar->GenerateString("Plugin Crash !\n","PlugIn:",iae->audioeffect->GetEffectName(),"\nError:",en))
			{
				maingui->MessageBoxError(0,h);
				delete h;
			}
		}

		iae=iae->NextEffect();
	}
}

void Seq_Song::SetPatternMuteFlag(Seq_Track *track,Seq_Pattern *p)
{
	/*
	bool used;

	// Audio Pattern Check
	if(p->mediatype==MEDIATYPE_AUDIO)
	used=((AudioPattern *)p)->CheckIfUsed();
	else
	used=true;
	*/

	// Set Pattern Mute Flag
	if(p->mute==true ||
		(mastering==false && CheckSoloPattern(p)==0) || // Disable Editor Solo etc..// Edfitor Solo
		(p->mainpattern && p->mainclonepattern==0 && p->mainpattern->mute==true) //|| //loops
		//track->GetGroups()->CheckIfPlaybackIsAbled()==true || // Group Mute
		//(track->song->groupsoloed==true && track->GetGroups()->CheckIfSolo()==false) // Group Solo
		)
	{
		p->p_muteflag=true;
	}
	else
	{
		p->p_muteflag=false;
	}
}

void Seq_Song::SetMuteFlags()
{
	solocount=0;

	Seq_Track *track=FirstTrack();
	while(track)
	{
		if(track->GetSolo()==true)
			solocount++;

		track=track->NextTrack();
	}

	track=FirstTrack();
	while(track)
	{
		track->SetMuteFlag();

		// Set Pattern Mute Flag
		Seq_Pattern *p=track->FirstPattern();

		while(p)
		{
			SetPatternMuteFlag(track,p);

			/*
			if(track->CheckIfPlaybackIsAble()==false)
			return false;

			return true;
			*/

			p=p->NextPattern();
		}

		track=track->NextTrack();
	}
}

void Seq_Song::ResetCycleSync()
{
	cycleoutofsync=false;
	for(int i=0;i<CYCLE_RESET_SYNCEND;i++)
		cycleresetcounter[i]=0;
}

void Seq_Song::CheckAudioMIDISync()
{
	if(AudioDevice *device=mainaudio->GetActiveDevice())
	{
		device->LockTimerCheck_Output();
		LONGLONG timeforrefill_systime=device->timeforrefill_systime;
	//	LONGLONG timeforrefill_maxsystime=device->timeforrefill_maxsystime;
		device->UnlockTimerCheck_Output();

		device->LockTimerCheck_Input();
		LONGLONG timeforaudioinpurefill_systime=device->timeforaudioinpurefill_systime;
	//	LONGLONG timeforaudioinputrefill_maxsystime=device->timeforaudioinputrefill_maxsystime;
		device->UnlockTimerCheck_Input();


		{ // Audio Device In
			double ms=maintimer->ConvertSysTimeToMs(timeforaudioinpurefill_systime);

			if(ms>device->samplebufferms)
			{
				device->deviceoutofsync=true;
			}
		}

		{ // Audio Device Out
			double ms=maintimer->ConvertSysTimeToMs(timeforrefill_systime);

			if(ms>device->samplebufferms)
			{
				device->deviceoutofsync=true;
			}
		}

		if(status&(STATUS_SONGPLAYBACK_MIDI|Seq_Song::STATUS_SONGPLAYBACK_AUDIO)) // Out of Sync Check
		{	
			MIDIaudiosynccounter++;

			if(MIDIaudiosynccounter>=60)
			{
				MIDIaudiosynccounter=0;

				// IRQ Sync
				{ // Intern Tempo Sync Map
					LockCycleSync();
					int cm=cycleresetcounter[CYCLE_RESET_MIDI];
					int ca=cycleresetcounter[CYCLE_RESET_AUDIO];
					UnlockCycleSync();

					if(cm==ca)
					{
						mainMIDIalarmthread->Lock();
						mainthreadcontrol->Lock(CS_audioplayback);

						LONGLONG samples_MIDItime=MIDI_samplestartposition,samples_audiotime=playback_sampleendposition;

						mainMIDIalarmthread->Unlock();
						mainthreadcontrol->Unlock(CS_audioplayback);

						if(samples_MIDItime>samples_audiotime)
						{
							// TRACE ("Sync Out of+++ MIDI %d > Audio +++\n");


							device->deviceoutofMIDIsync=true;
						}

						// TRACE ("MIDI %d\n",MIDItime);
						// TRACE ("AUDIO %d\n",audiotime);

					}
				}
			}
		}
	}
}

void Seq_Song::SetMIDIRecordingFlag()
{
	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->record==true && t->recordtracktype==TRACKTYPE_MIDI)
		{
			MIDIrecording=true;
			return;
		}

		t=t->NextTrack();
	}

	MIDIrecording=false;
}

void Seq_Song::CheckKeyTimer()
{
	if(mainsettings->checkspacenonfocus==false)
		return;

	if(maingui->CheckIfKeyDown(KEYSPACE)==false)
	{
		waitforspaceup=false;
		return;
	}

	if(waitforspaceup==false)
	{
		if(IsPlayback()==false && IsRecording()==false)
		{
			if(maingui->GetShiftKey()==true)
				RecordSong();
			else
				PlaySong();

			waitforspaceup=true;

		}
		else // Playback or Recording
			if(IsPlayback()==true || IsRecording()==true)
			{
				StopSelected();
				waitforspaceup=true;
			}
	}
}

void Seq_Song::RefreshRealtime()
{
	CheckKeyTimer();

	SetMuteFlags();
	audiosystem.SetAudioSystemHasFlag();
	AudioInputNeeded();
	CheckPluginTimer();
	DeleteDeletedAutomationParameter();

	if(mainaudio->GetActiveDevice())
	{
		CheckAudioMIDISync();

		CheckEffectPipeline(&audiosystem.masterchannel.io.audioeffects);

		for(int i=0;i<LASTSYNTHCHANNEL;i++)
		{
			AudioChannel *channel=audiosystem.FirstChannelType(i);

			while(channel)
			{
				CheckEffectPipeline(&channel->io.audioeffects);
				channel=channel->NextChannel();
			}
		}
	}

	if(audiosystem.masterchannel.MIDIoutputimpulse){

		if(audiosystem.masterchannel.MIDIoutputimpulse>2)
			audiosystem.masterchannel.MIDIoutputimpulse-=2;
		else
			audiosystem.masterchannel.MIDIoutputimpulse=0;
	}

	SetMIDIRecordingFlag();

	Seq_Track *t=FirstTrack();

	while(t){

		// Check for corrupt Plugins
		if(mainaudio->GetActiveDevice())
		{
			CheckEffectPipeline(&t->io.audioeffects);
		}

		// Song Plugin Event Monitoring
		InsertAudioEffect *iae=t->io.audioeffects.FirstInsertAudioEffect();

		while(iae)
		{
			while(iae->audioeffect->monitor_syscounter!=iae->audioeffect->monitor_eventcounter)
			{
				if(LMIDIEvents *ne=new LMIDIEvents)
				{
					iae->audioeffect->monitor_events[iae->audioeffect->monitor_syscounter].Clone(ne);

					ne->track=t;
					ne->plugin=mainvar->GenerateString(iae->audioeffect->GetEffectName());
					ne->outdevice=0;
					ne->indevice=0;

					events_out.AddEvent(ne);
				}

				iae->audioeffect->monitor_syscounter==MAXMONITOREVENTS-1?iae->audioeffect->monitor_syscounter=0:iae->audioeffect->monitor_syscounter++;
			}

			iae=iae->NextEffect();
		}

		if(mainsettings->autoinstrument==true && t->MIDItypesetauto==true)
		{
			if(t->MIDItype==Seq_Track::OUTPUTTYPE_MIDI)
			{
				//	if(t->CheckMIDIType()==true)
				//		t->SetMIDIType(Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT);
				//	else
				{
					{
						InsertAudioEffect *iae=t->io.audioeffects.FirstInsertAudioEffect();
						while(iae)
						{
							if(iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)
							{
								t->SetMIDIType(Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT);
								break;
							}

							iae=iae->NextEffect();
						}
					}

					{
						InsertAudioEffect *iae=t->io.audioinputeffects.FirstInsertAudioEffect();
						while(iae)
						{
							if(iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)
							{
								t->SetMIDIType(Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT);
								break;
							}

							iae=iae->NextEffect();
						}
					}
				}
			}
			else
				if(t->MIDItype==Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT)
				{
					{
						InsertAudioEffect *iae=t->io.audioeffects.FirstInsertAudioEffect();
						while(iae)
						{
							if(iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)
								goto nextcheck;

							iae=iae->NextEffect();
						}
					}

					{
						InsertAudioEffect *iae=t->io.audioinputeffects.FirstInsertAudioEffect();
						while(iae)
						{
							if(iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)
								goto nextcheck;

							iae=iae->NextEffect();
						}
					}

					t->SetMIDIType(Seq_Track::OUTPUTTYPE_MIDI); // back to MIDI
				}
		}

nextcheck:
		if(t->GetFX()->setalwaysthruautomatic==true)
		{
			if(t->GetFX()->usealwaysthru==false)
				t->GetFX()->setalwaysthruautomatic=false;
			else
			{
				// Check For Instruments
				{
					InsertAudioEffect *iae=t->io.audioeffects.FirstInsertAudioEffect();
					while(iae)
					{
						if(iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)
							goto pulsecheck;

						iae=iae->NextEffect();
					}
				}

				{
					InsertAudioEffect *iae=t->io.audioinputeffects.FirstInsertAudioEffect();
					while(iae)
					{
						if(iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)
							goto pulsecheck;

						iae=iae->NextEffect();
					}
				}

				t->GetFX()->usealwaysthru=t->GetFX()->setalwaysthruautomatic=false;

			}
		}

		// Audio Record Pattern Threshold
		/*
		for(int i=0;i<MAXRECPATTERNPERTRACK;i++) // AUDIO
		if(t->audiorecord_audiopattern[i])
		{
		if(t->audiorecord_audiopattern[i]->audioevent.audioefile->startpeakoffset>=0)
		{
		mainthreadcontrol->LockActiveSong();

		t->MovePatternToTrack(t->audiorecord_audiopattern[i],t,t->audiorecord_audiopattern[i]->audioevent.audioefile->startpeakoffset);
		t->audiorecord_audiopattern[i]->audioevent.audioefile->startpeakoffset=-1; // reset
		mainthreadcontrol->UnlockActiveSong();
		}
		}
		*/

		// Display MIDI Output

pulsecheck:
		if(t->MIDInputimpulse){

			if(t->MIDInputimpulse>2)
				t->MIDInputimpulse-=2;
			else
				t->MIDInputimpulse=0;
		}

		if(t->MIDInputimpulse_data){

			if(t->MIDInputimpulse_data>2)
				t->MIDInputimpulse_data-=2;
			else
				t->MIDInputimpulse_data=0;
		}

		if(t->MIDIoutputimpulse){

			if(t->MIDIoutputimpulse>audiosystem.masterchannel.MIDIoutputimpulse)
				audiosystem.masterchannel.MIDIoutputimpulse=t->MIDIoutputimpulse;

			if(t->MIDIoutputimpulse>2)
				t->MIDIoutputimpulse-=2;
			else
				t->MIDIoutputimpulse=0;
		}

		// Track Plugin Events/Automation
		t->CheckPlugins();

		t=t->NextTrack();
	}

	{
		AudioChannel *c=audiosystem.FirstBusChannel();
		while(c)
		{
			c->CheckPlugins();
			c=c->NextChannel();
		}

		audiosystem.masterchannel.CheckPlugins();
	}

	{
		Seq_MetroTrack *mt=FirstMetroTrack();

		while(mt)
		{
			if(mt->MIDIoutputimpulse){

				if(mt->MIDIoutputimpulse>2)
					mt->MIDIoutputimpulse-=2;
				else
					mt->MIDIoutputimpulse=0;
			}

			mt=mt->NextMetroTrack();
		}

	}

	// MIDI I/O Event Monitoring
	// Device Output >>>>
	{
		MIDIOutputDevice *od=mainMIDI->FirstMIDIOutputDevice();

		while(od)
		{
			while(od->monitor_eventcounter!=od->monitor_syscounter)
			{
				if(LMIDIEvents *ne=new LMIDIEvents)
				{
					od->monitor_events[od->monitor_syscounter].Clone(ne);

					ne->outdevice=od;
					ne->indevice=0;

					events_out.AddEvent(ne);
				}

				od->monitor_syscounter==MAXMONITOREVENTS-1?od->monitor_syscounter=0:od->monitor_syscounter++;

			}// while

			od=od->NextOutputDevice();
		}
	}

	// Device Input <<<<<<<<<<<
	{
		MIDIInputDevice *id=mainMIDI->FirstMIDIInputDevice();

		while(id)
		{
			while(id->monitor_eventcounter!=id->monitor_syscounter)
			{
				if(LMIDIEvents *ne=new LMIDIEvents())
				{
					id->monitor_events[id->monitor_syscounter].Clone(ne);

					ne->indevice=id;
					ne->outdevice=0;

					events_in.AddEvent(ne);

					maingui->LearnFromMIDIEvent(ne);
				}

				id->monitor_syscounter==MAXMONITOREVENTS-1?id->monitor_syscounter=0:id->monitor_syscounter++;

			}// while

			id=id->NextInputDevice();
		}
	}
}

void Seq_Song::DoAutoSave()
{
	if(directoryname)
	{
		if(char *h=new char[strlen(directoryname)+64])
		{
			strcpy(h,directoryname);
			mainvar->AddString(h,"\\Autosave");
			mainvar->CreateNewDirectory(h);
			mainvar->AddString(h,"\\Auto");

			char h2[16];

			mainvar->AddString(h,mainvar->ConvertIntToChar(autosavecounter,h2));
			mainvar->AddString(h,".camx");

			camxFile save;

			if(save.OpenSave(h)==true)
			{
				save.helpflag|=FLAG_AUTOSAVE;
				Save(&save);

				if(++autosavecounter==6)
					autosavecounter=1;
			}

			save.Close(true);

			delete h;
		}
	}
}

void Seq_Song::DoPlayback(int mode,int playstatus)
{
	/*
	#ifdef DEBUG
	Seq_Track *t=FirstTrack();
	while(t)
	{
	TRACE ("DoPlayback Playback %d\n",t->playback_MIDIPattern[0]);
	t=t->NextTrack();
	}

	#endif
	*/

	// Sync CamX Song Position<>MIDI Song Position
	if(playbacksettings.cycleplayback==false || (!(playstatus&STATUS_RECORD))) // No SongPosition Change while Cycle Recording
	{
		if(mainMIDI->quantizesongpositiontoMIDIclock==true)
		{
			OSTART qpos=mainvar->SimpleQuantize(songposition,SAMPLESPERBEAT/4);
			SetSongPosition(qpos,0);
		}
	}

	//SetTrackIndexs();

	TRACE ("DoPlayback CheckCycleStart\n");
	CheckCycleStart();

	TRACE ("DoPlayback InitMetroClick\n");
	metronome.InitMetroClick(GetSongPosition(),AUDIOMETROINDEX);
	metronome.InitMetroClick(GetSongPosition(),MIDIMETROINDEX);

	//Audio Recording
	if(playstatus&(STATUS_RECORD|STATUS_WAITPREMETRO))
	{
		TRACE ("DoPlayback InitAudioRecording\n");
		//int newfiles=

		InitAudioRecording();

		/*
		if(newfiles && playbacksettings.cycleplayback==true)
		{
		PRepairPlayback(GetSongPosition(),MEDIATYPE_AUDIO|MEDIATYPE_AUDIO_RECORD); // Add
		}
		*/
	}

	TRACE ("DoPlayback StartSongPlayback\n");
	StartSongPlayback(mode,playstatus);

	TRACE ("DoPlayback DONE +++\n");
}

void Seq_Song::InitSongPlaybackPatternNewCycleStart()
{
	// MIDI Only
	InitPlay init(INITPLAY_NEWCYCLE);

	init.mode=MEDIATYPE_MIDI;
	init.position=playbacksettings.cyclestart;

	InitSongPlayback(&init);

	// Copy Init INITPLAY_NEWCYCLE -> Cycle Audio+MIDI, Cycle Only
	Seq_Track *t=FirstTrack();

	while(t)
	{
		// Track Start Pattern
		//	t->playback_cycleMIDIPattern[to]=t->playback_cycleMIDIPattern[from];

		t->playback_MIDIPattern[INITPLAY_MIDITRIGGER][1]=t->playback_MIDIPattern[INITPLAY_AUDIOTRIGGER][1]=t->playback_MIDIPattern[INITPLAY_NEWCYCLE][1];
		t->playback_chainnoteMIDIPattern[INITPLAY_MIDITRIGGER][1]=t->playback_chainnoteMIDIPattern[INITPLAY_AUDIOTRIGGER][1]=t->playback_chainnoteMIDIPattern[INITPLAY_NEWCYCLE][1];


		// Copy MIDI Pattern CHECK->SONG
		MIDIPattern *copy=(MIDIPattern *)t->FirstPattern(MEDIATYPE_MIDI);

		while(copy)
		{
			copy->playback_MIDIevent[INITPLAY_MIDITRIGGER][1]=copy->playback_MIDIevent[INITPLAY_AUDIOTRIGGER][1]=copy->playback_MIDIevent[INITPLAY_NEWCYCLE][1];
			copy->playback_nextchainevent[INITPLAY_MIDITRIGGER][1]=copy->playback_nextchainevent[INITPLAY_AUDIOTRIGGER][1]=copy->playback_nextchainevent[INITPLAY_NEWCYCLE][1];
			
			copy=(MIDIPattern *)copy->NextPattern(MEDIATYPE_MIDI);
		}

		t=t->NextTrack();
	}			

	PRepairPreStartAndCycleEvents(playbacksettings.cyclestart,true); // Refresh Cycle Pre Event
}

void Seq_Song::RefreshMIDIPatternEventIndexs() // MIDI File etc..
{
	Seq_Track *t=FirstTrack();

	while(t)
	{
		Seq_Pattern *p=t->FirstPattern(MEDIATYPE_MIDI);

		while(p)
		{
			MIDIPattern *mp=(MIDIPattern *)p;

			mp->CloseEvents();

			p=p->NextPattern(MEDIATYPE_MIDI);
		}

		t=t->NextTrack();
	}
}

void Seq_Song::InitSongPlaybackPattern(OSTART position,int mediatype,int index) // + Called by Master
{
	InitPlay init(index);

	init.mode=mediatype;
	init.position=position;

	InitSongPlayback(&init/*,false*/); // Audio+MIDI -----------------> Playback Main function

	// Mix MIDI/ Init Next MIDI Output Event
	cMIDIPlaybackEvent nmpbe(index);
	MixNextMIDIPlaybackEvent(&nmpbe);

	if(index==INITPLAY_MIDITRIGGER)
	{
		for(int i=INITPLAY_MIDITRIGGER+1;i<INITPLAY_MAX;i++)
			CopyInitPlay(i,INITPLAY_MIDITRIGGER);
	}

	// Init Stream+Audio

	// Intern Tempo Sync Map
	if(audiosystem.device)
	{
		stream_samplestartposition=timetrack.ConvertTicksToTempoSamples(position);
	}
}

void Seq_Song::PRepairPlayback(OSTART position,int mediatype)
{
	songstartposition=position;
	InitSongPlaybackPattern(position,mediatype,INITPLAY_MIDITRIGGER);
}

void Seq_Song::PlaySong(int flag)
{	
	//if(!(flag&SETSONGPOSITION_NOLOCK))LockSync();
	/*
	#ifdef DEBUG
	Seq_Track *t=FirstTrack();
	while(t)
	{
	TRACE ("PlaySong Playback %d\n",t->playback_MIDIPattern[0]);
	t=t->NextTrack();
	}
	#endif
	*/

	mainthreadcontrol->Lock(CS_SONGCONTROL);

	if((!(status&STATUS_PLAY)) && (!(status&STATUS_RECORD)) && (!(status&STATUS_STEPRECORD)) && CanStatusBeChanged()==true)
	{
		bool stopposition=false;

		// Check Song Stop Position Marker
		Seq_Marker *marker=textandmarker.FirstMarker();
		while(marker)
		{
			if(marker->functionflag==Seq_Marker::MARKERFUNC_STOPPLAYBACK)
			{
				if(GetSongPosition()>=marker->GetMarkerStart())
				{
					stopposition=true;
					break;
				}
			}

			marker=marker->NextMarker();
		}

		if(stopposition==false)
		{
			//TRACE ("Play Stop Song...\n");
			//StopSong(0,GetSongPosition());
			//TRACE ("Play Stop Song DONE\n");

			TRACE ("Play SetActiveSong...\n");
			mainvar->SetActiveSong(this);
			TRACE ("Play SetActiveSong DONE\n");

			if(mainvar->GetActiveSong()==this)
			{
				if(status==STATUS_STOP || (status&STATUS_WAITPREMETRO) || status==STATUS_STEPRECORD)
				{	
					if(status&STATUS_WAITPREMETRO)
					{
						status = STATUS_INIT|STATUS_PLAY;
						SetOldRecordStatus(); // Reset Track Status
						mainMIDIalarmthread->SetSignal();
					}
					else
					{
						int flag=STATUS_INIT|STATUS_PLAY;

						if(status==STATUS_STOP && mainsettings->waitforMIDIplayback==true)
							flag|=STATUS_MIDIWAIT;

						TRACE ("Play DoPlayback ... %d FLAG=%d\n",status,flag);

						DoPlayback(MEDIATYPE_MIDI|MEDIATYPE_AUDIO,flag);
						TRACE ("Play DoPlayback DONE %d\n",status);
					}
				}
				else
				{
					// Switch Playback To Record

					if(status&STATUS_RECORD){
						SetOldRecordStatus();
						status CLEARBIT STATUS_RECORD;
					}

					status|=STATUS_PLAY;
				}
			}
		}
	}

	mainthreadcontrol->Unlock(CS_SONGCONTROL);

	TRACE ("+++ Play Song %d DONE +++\n",songposition);

	//if(!(flag&SETSONGPOSITION_NOLOCK))UnlockSync();
}

void Seq_Song::InitRecordStatus()
{
	if(mainsettings->setfocustrackautotorecord==true)
	{
		int tracksinrecordmode=0;
		Seq_Track *t=FirstTrack();

		while(t)
		{
			t->recordbeforestart=t->record;

			if(t->record==true)
				tracksinrecordmode++;

			t=t->NextTrack();
		}

		if(tracksinrecordmode==0 && GetFocusTrack())
		{
			GetFocusTrack()->SetRecordMode(true,GetSongPosition());
		}

	}

	recordedsamples=0;
	checkinlatency=true; // Input Latency Check
}

#include "mainhelpthread.h"
extern MainHelpThread *mainhelpthread;

void Seq_Song::StopThreadPlayback()
{
	TRACE ("StopThreadPlayback ... \n");

	mainthreadcontrol->LockActiveSong();

	/*
	mainhelpthread->Lock();

	TRACE ("1 StopThreadPlayback ... \n");

	mainthreadcontrol->Lock(CS_sync);
	TRACE ("2 StopThreadPlayback ... \n");

	mainthreadcontrol->Lock(CS_MIDIthru);
	TRACE ("3 StopThreadPlayback ... \n");
	mainMIDIalarmthread->Lock();
	TRACE ("4 StopThreadPlayback ... \n");
	MIDIinproc->Lock();
	TRACE ("5 StopThreadPlayback ... \n");
	mainthreadcontrol->Lock(CS_vstin);
	TRACE ("6 StopThreadPlayback ... \n");

	mainaudiostreamproc->Lock(); // Before Lock Playback !!!
	TRACE ("7 StopThreadPlayback ... \n");

	mainthreadcontrol->Lock(CS_audioplayback);
	TRACE ("8 StopThreadPlayback ... \n");

	TRACE ("9 StopThreadPlayback ... \n");
	audiorecordthread->Lock();
	TRACE ("10 StopThreadPlayback ... \n");
	mainthreadcontrol->Lock(CS_audiorecordbuffer);
	TRACE ("11 StopThreadPlayback ... \n");
	mainthreadcontrol->Lock(CS_audiofreeze);
	TRACE ("12 StopThreadPlayback ... \n");
	MIDIrtealarmproc->Lock();
	TRACE ("13 StopThreadPlayback ... \n");
	*/

	TRACE ("StopThreadPlayback Lock ... \n");

	status CLEARBIT STATUS_SONGPLAYBACK_MIDI;
	status CLEARBIT STATUS_SONGPLAYBACK_AUDIO;
	status CLEARBIT STATUS_WAITPREMETRO;

	startaudiorecording=startaudioinit=startMIDIinit=false;

	mainthreadcontrol->UnlockActiveSong();

	if(mainaudio->GetActiveDevice())
	{
		mainaudio->GetActiveDevice()->WaitForRecordingEnd();

		/*
		AudioHDFile *rec=mainaudio->FirstAudioRecordingFile();

		while(rec){
		rec->CloseRecordPeak();
		rec=rec->NextHDFile();
		}
		*/

		/*
		Seq_Track *t=FirstTrack();
		while(t){
		if(t->CheckIfAudioRecording()==true)break;
		t=t->NextTrack();
		}
		*/
	}

	TRACE ("StopThreadPlayback Done \n");

	//waitingforrecordend=false;
}

void Seq_Song::StartThreadPlayback(int setstatus)
{
	mainaudiostreamproc->LockTimerCheck();
	mainaudiostreamproc->timeforrefill_maxms=mainaudiostreamproc->timeforrefill_ms=0;
	mainaudiostreamproc->UnlockTimerCheck();

	startMIDIprecounter=false;

	if(setstatus&STATUS_WAITPREMETRO)
	{
		// Init Precounter
		if(mainaudio->GetActiveDevice())
			InitNextPreMetro(mainaudio->GetActiveDevice(),AUDIOMETROINDEX,false);

		InitNextPreMetro(0,MIDIMETROINDEX,false);
	}

	if(mainaudio->GetActiveDevice() && mainaudio->GetActiveDevice()->devicestarted==true)
	{
		TRACE ("StartThreadPlayback devicestarted==TRUE\n");
		TRACE ("StartThreadPlayback Lock_AudioPlayback\n");

		mainMIDIalarmthread->Lock();
		mainthreadcontrol->Lock(CS_audioplayback);
		//	audiorecordbufferthread->Lock();
		audiorecordthread->Lock();

		//TRACE ("Start Record Buffer %d %d \n",inputbuffersinc,inputbuffersrecc);

		//inputbuffersinc=inputbuffersrecc=0;

		startaudioinit=true; // + Sync MIDI
		status=setstatus;

		TRACE ("StartThreadPlayback Unlock\n");

		audiorecordthread->Unlock();
		//audiorecordbufferthread->Unlock();
		mainthreadcontrol->Unlock(CS_audioplayback);
		mainMIDIalarmthread->Unlock();
		TRACE ("StartThreadPlayback Unlock Done\n");
	}
	else
	{
		TRACE ("StartThreadPlayback devicestarted==FALSE\n");
		metronome.sendpresignaltoMIDI=false;
		metronome.waitMIDIoffsetticks=0;

		// No AUdio Device -> only MIDI Proc
		mainMIDIalarmthread->Lock();

		if((setstatus&Seq_Song::STATUS_WAITPREMETRO) || (setstatus&Seq_Song::STATUS_MIDIWAIT))
			startMIDIinit=false;
		else
			startMIDIinit=true;

		status=setstatus;
		mainMIDIalarmthread->Unlock();

		mainMIDIalarmthread->SetSignal(); // Start MIDI
	}
}

void Seq_Song::RecordSong(int flag)
{
	mainthreadcontrol->Lock(CS_SONGCONTROL);

	if((!(status&STATUS_RECORD)) && CanStatusBeChanged()==true)
	{
		StopSong(0,GetSongPosition());

		mainvar->SetActiveSong(this);

		if(this==mainvar->GetActiveSong())
		{
			if(playbacksettings.cycleplayback==true)
			{
				if(punchrecording&Seq_Song::PUNCHOUT)
				{
					maingui->MessageBoxError(0,Cxs[CXS_PUNCHCYCLEERROR]);
					return;
				}

				if(GetSongPosition()>playbacksettings.cyclestart)
				{
					SetSongPosition(playbacksettings.cyclestart,true);
				}
			}

			InitRecordStatus();

			if(mainsettings->createnewchildtrackwhenrecordingstarts==true)
			{
				// Check New Child Tracks
				int c=0;
				Seq_Track *t=FirstTrack();

				while(t)
				{
					if(t->record==true && t->CanNewChildTrack()==true)
						c++;

					t=t->NextTrack();
				}

				if(c)
				{
					if(Seq_Track **tracks=new Seq_Track*[c])
					{
						{
							int i=0;
							Seq_Track *t=FirstTrack();

							while(t)
							{
								if(t->record==true && t->CanNewChildTrack()==true)
									tracks[i++]=t;

								t=t->NextTrack();
							}
						}

						mainthreadcontrol->LockActiveSong();

						for(int i=0;i<c;i++)
						{
							if(Seq_Track *nt=new Seq_Track)
							{
								tracks[i]->CloneToTrack(nt,false);
								nt->parent=tracks[i];

								CreateRecordingCloneTrack(tracks[i],nt,0,false);
							}
						}

						CreateQTrackList();

						mainthreadcontrol->UnlockActiveSong();

						delete tracks;
					}


					/*
					if(mainsettings->createnewchildtrackwhenrecordingstarts==true)
					{
					if(!parent)
					{
					ct->parent=t;
					ct->childdepth=1;
					}
					else
					{
					if(parent->childdepth<MAX_CHILDS)
					{
					// Add as Child
					ct->parent=parent;
					ct->childdepth=parent->childdepth+1;
					}
					else
					{
					ct->parent=t->parent;
					ct->childdepth=t->childdepth;
					}
					}
					}7
					*/

				}
			}

			if(steprecordingonoff==true)
			{
				DoPlayback(MEDIATYPE_MIDI|MEDIATYPE_AUDIO,STATUS_STEPRECORD);
				status=STATUS_STEPRECORD;
			}
			else
			{
				/*
				bool stopsong;

				if(song->status&STATUS_STEPRECORD)
				stopsong=true;
				else
				stopsong=false;

				mainthreadcontrol->LockActiveSong();

				if(song->steprecordingonoff==true)
				{
				song->status|=STATUS_STEPRECORD;
				song->InitRecordStatus();
				}
				else
				song->status CLEARBIT STATUS_STEPRECORD;

				mainthreadcontrol->UnlockActiveSong();

				if(stopsong==true)
				{
				if(mainedit->RefreshRecording(song)==true)
				maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
				}
				*/

				//	{	
				int rflag=STATUS_INIT|STATUS_RECORD;

				if(status==STATUS_STOP && mainsettings->waitforMIDIrecord==true)
					rflag|=STATUS_MIDIWAIT;

				// Metro PreCounter
				if(status==STATUS_STOP && 
					mainsettings->usepremetronome==true &&
					metronome.on==true &&
					(!(flag&RECORD_SKIPPRECOUNTER)) && // Skip Precounter ?
					( (GetSongPosition()==0 || mainsettings->precountertype==Settings::PRECOUNTER_ALWAYS) )
					)
				{
					rflag|=STATUS_WAITPREMETRO; // set wait flag
				}

				songstartposition=songposition;

				DoPlayback(MEDIATYPE_MIDI|MEDIATYPE_AUDIO,rflag);

				/*
				}
				else
				{
				status&=~STATUS_PLAY;
				status|=STATUS_RECORD;
				}
				*/
			}

			recordcounter++;
		}
	}

	mainthreadcontrol->Unlock(CS_SONGCONTROL);
}

void Seq_Song::SetOldRecordStatus()
{
	mainaudio->StopRecordingFiles(this);

	if(mainvar->exitthreads==false)
	{
		mainedit->AddRecordingToUndo(this);
	}

	// Set old record status
	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->record==true && t->recordbeforestart==false)
			t->SetRecordMode(false,t->songstartposition);
		else
			t->record=t->recordbeforestart;

		t->cyclerecordmute=false;

		t=t->NextTrack();
	}

	//	maingui->RefreshAllEditors(this,EDITORTYPE_ARRANGE,0);
}

void Seq_Song::SendOpenEvents()
{
	SendAllOpenNotes();

	for(int i=0;i<REALTIME_LISTS;i++)
		realtimeevents[i].DeleteAllREvents(this,0); // remove Realtime Events
}

void Seq_Song::SetSongDirectory(char *dir)
{
	if(dir){
		if(directoryname)delete directoryname;
		directoryname=mainvar->GenerateString(dir);
	}
}

bool Seq_Song::CheckForOtherMIDI(Seq_Track *track,bool output,bool onlyselected)
{
	if(!track)return false;

	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t!=track)
		{
			if(onlyselected==false || (t->flag&OFLAG_SELECTED))
			{
				if(output==true)
				{
					if(t->CompareMIDIOut(track)==false)
						return true;
				}
				else
				{
					if(t->CompareMIDIIn(track)==false)
						return true;
				}
			}
		}

		t=t->NextTrack();
	}

	return false;
}

OSTART Seq_Song::StopSong(int stopflag,OSTART songpos,bool setsongposition)
{
	TRACE ("Stop Song\n");

	MIDIsync.startedwithmc=MIDIsync.startedwithmtc=false;
	MIDIsync.sync=MIDIsync.syncbeforestart;

	songposition=GetSongPosition(); // Save Song Position

	StopThreadPlayback(); // Clear SONG RUNNING FLAG

	TRACE ("CopyRecordDataToRecordPattern\n");

	bool newpattern=CopyRecordDataToRecordPattern();

	//mainMIDI->monitor.ResetKeys();

	if(status!=STATUS_STOP || underdeconstruction==true)
	{	
		TRACE ("status!=STATUS_STOP\n");
		bool setnewsongpos=false;

		/*
		if((!(stopflag&SETSONGPOSITION_NOQUANTIZE)) && setsongposition==true)
		{
		// Sync CamX Song Position<>MIDI Song Position 
		if(mainMIDI->quantizesongpositiontoMIDIclock==true && mainMIDI->sendMIDIcontrol==true){

		OSTART h=mainvar->SimpleQuantizeRight(songpos,PPQRATE/4);

		if(h!=songpos){
		setnewsongpos=true;
		songpos=h;
		}
		}
		}
		*/

		int oldstatus=status;

		status=STATUS_STOP;

		if(setsongposition==false) // called by SetSongPosition
			songposition=songpos;

		ResetCycleSync();

		TRACE ("SendOpenEvents\n");
		
		SendOpenEvents();
		DeleteAllVirtualTempos();
		DeleteAllRunningAudioFiles();
		CheckCrossFades();

		ResetAutomationTracks();

		// Stop MIDI		
		//mainMIDI->ResetMIDIInClocks(this);

		if((oldstatus!=STATUS_STOP) && (oldstatus&STATUS_STEPRECORD))
			mainMIDI->SendMIDIStop(this,stopflag);

		//	mainMIDI->SendSongStopEvents(this);

		if(!(oldstatus&(STATUS_RECORD|STATUS_STEPRECORD)) )
		{
			//	timetrack.StopTimeTrack(this);// delete virtual tempo...

		}
		else
			SetOldRecordStatus();

		if(mainvar->exitthreads==false){

			if(underdeconstruction==false){

				if(setnewsongpos==true && setsongposition==true)
					SetSongPosition(songpos,false,0);
				else{
					if(!(stopflag&NO_SONGPRepair))
						PRepairPlayback(songpos,MEDIATYPE_ALL);
				}

				//if(!(stopflag&NO_MIDIRESET))
				mainMIDI->SendResetToAllDevices(stopflag);

				if(!(stopflag&SETSONGPOSITION_NOGUI))
					maingui->RefreshAllEditors(this,EDITORTYPE_TRANSPORT,0); // reset CPU Usage
			}
		}

		//ResetSongTracksAudioBuffer();
	}

	if(underdeconstruction==false)
	{
		if(newpattern==true && mainvar->exitthreads==false && underdeconstruction==false)
			maingui->RefreshAllEditors(this,EDITORTYPE_ARRANGE,Edit_Arrange::REFRESH_OVERVIEW); // Refresh Overview

		guiWindow *w=maingui->FirstWindow();
		while(w)
		{
			if(w->WindowSong()==this)
				w->StopSong();

			w=w->NextWindow();
		}
	}

	TRACE ("Stop Song Done \n");
	return songpos;
}

void Seq_Song::StopSelected(int flag)
{
	if(mainvar->GetActiveSong()!=this)
	{
		mainvar->SetActiveSong(this);
		return;
	}

	mainthreadcontrol->Lock(CS_SONGCONTROL);

	if(CanStatusBeChanged()==true)
	{
		if(status!=STATUS_STOP || (status&STATUS_STEPRECORD))
			StopSong(flag,GetSongPosition());
		else
		{
			mainsyncthread->Lock();

			bool stop=true;

			mainaudioreal->LockRTObjects();

			if(mainaudioreal->FirstAudioRealtime()) // Stop All Realtime Playback
			{
				mainaudioreal->UnlockRTObjects();

				if(this==mainvar->GetActiveSong()){
					mainaudioreal->StopAllRealtimeEvents();
					stop=false;
				}
			}
			else
				mainaudioreal->UnlockRTObjects();

			if(stop==true)
			{
				// cycle <-> 1.1.1.1
				if(GetSongPosition()>playbacksettings.cyclestart)
					SetSongPosition(playbacksettings.cyclestart,false,0);
				else
					SetSongPosition(GetSongPosition()==playbacksettings.cyclestart?0:playbacksettings.cyclestart,false,0);

				/*
				if(mainsettings->doublestopeditrefresh==true)
				{
				guiWindow *w=maingui->FirstWindow();
				while(w)
				{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_ARRANGE:
				case EDITORTYPE_EVENT:
				case EDITORTYPE_SCORE:
				case EDITORTYPE_DRUM:
				case EDITORTYPE_WAVE:
				case EDITORTYPE_TEMPO:
				{
				EventEditor *ee=(EventEditor *)w;
				ee->RefreshEventEditorRealtime(true); // force
				}
				break;
				}

				w=w->NextWindow();
				}
				}
				*/

			}

			mainsyncthread->Unlock();
		}
	}

	mainthreadcontrol->Unlock(CS_SONGCONTROL);
}


void Seq_Song::MuteTrack(Seq_Track *track,bool mute,OSTART automationtime)
{
	if(track->GetMute()!=mute){

		if(automationtime>0)
			Automate(&track->io.audioeffects.mute,automationtime,0,mute==true?1:0,0);

		track->io.audioeffects.SetMute(mute);

		if(track->ismetrotrack==false)
		{
			SetMuteFlags();

			if(track->GetMute()==true && (track->GetMediaTypes()&MEDIATYPE_MIDI)){

				//TRACE ("Mute TRack---\n");
				mainMIDIalarmthread->Lock();
				mainMIDI->StopAllofTrack(track);	
				mainMIDIalarmthread->Unlock();
			}
		}
	}
}

void Seq_Song::SoloTrack(Seq_Track *track,bool solo,OSTART automationtime)
{
	mainthreadcontrol->LockActiveSong();

	bool ctrlkey=maingui->GetCtrlKey();

	Seq_Track *t=FirstTrack();

	while(t){

		if(t==track || (ctrlkey==false && t->IsSelected()==true && track->IsSelected()==true))
		{
			if(automationtime>0)
				Automate(&t->io.audioeffects.solo,automationtime,0,solo==true?1:0,0);

			t->SetSolo(solo);
		}
		else
			if(ctrlkey==false && ( (track->IsSelected()==true && t->IsSelected()==false) || track->IsSelected()==false) )
			{
				if(automationtime>0)
					Automate(&t->io.audioeffects.solo,automationtime,0,0,0);

				t->SetSolo(false);
			}
			t=t->NextTrack();
	}

	mainthreadcontrol->UnlockActiveSong();

	SetMuteFlags();
}

bool Seq_Song::AddNewSelectionList(EventEditor *editor,Seq_SelectionList *list)
{
	if(list){
		list->editor=editor;
		list->song=this;
		return true;
	}

	return false;
}

bool Seq_Song::AddNewSelectionList(EventEditor *editor,Seq_SelectionList *list,Seq_SelectionList *copyfrom)
{
	if(copyfrom && list){

		list->editor=editor;
		list->song=this;

		Seq_SelectedPattern *c=copyfrom->FirstSelectedPattern();

		// Copy from
		while(c){
			list->AddPattern(c->pattern);
			c=c->NextSelectedPattern();
		}

		return true;		
	}

	return false;
}

void Seq_SelectionList::DeleteSelectionList()
{
	ClearEventList(); // Delete mixed Event List

	Seq_SelectedPattern *sp=FirstSelectedPattern();

	while(sp) // Delete Sel Pattern
	{
		if(sp->trackname)
		{
			delete sp->trackname;
			sp->trackname=0;
		}

		if(sp->patternname)
		{
			delete sp->patternname;
			sp->patternname=0;
		}

		sp=DeletePattern(sp);
	}
}

bool Seq_Song::CopyRecordDataToRecordPattern() // MIDI InProc creates+adds *MIDIInputData
{
	bool newdata=false,refreshloops=false;

	mainMIDIrecord->Lock();
	NewEventData *mid=mainMIDIrecord->FirstMIDIRecordData();
	mainMIDIrecord->UnLock();

	while(mid){

		if(mid->song==this){

			if(mid->songstatusatinput&STATUS_STEPRECORD) 	// MIDI Step Recording ?
			{
				//long songposition=GetSongPosition();
				bool noteadded=false;
				UBYTE checkstatus=mid->status&0xF0;

				if(Seq_Track *rectrack=FirstTrack()){

					do{	
						if(rectrack->record==true && /*(rectrack->tracktype==TRACKTYPE_ALL || */rectrack->recordtracktype==TRACKTYPE_MIDI/*)*/ )
						{
							MIDIPattern *recpattern=rectrack->record_MIDIPattern;

							if(recpattern && ((checkstatus==NOTEON && mid->byte2==0) || checkstatus==NOTEOFF)){

								// Note OFF -> Note
								recpattern->LockOpenNotes();

								NoteOpen *onote=recpattern->FirstOpenNote();

								while(onote){

									if(onote->key==mid->byte1 && // Same Key ?
										(onote->status&0x0F)==(mid->status&0x0F) // Same Channel ?
										){
											// noteon startpos
											if(CheckPunch(songposition)==true)
											{
												if(Note *note=recpattern->NewNote(songposition,songposition)){

													OSTART noteofftime=songposition+GetStepLength();

													note->flag|=EVENTFLAG_RECORDED;
													note->status=onote->status;
													note->key=onote->key;
													note->velocity=onote->velocity;
													note->velocityoff=mid->byte2;
													note->off.staticostart=noteofftime;

													recpattern->AddSortVirtual(&note->off,noteofftime); // NoteOff										

													noteadded=true;
													newdata=true;
												}
											}

											recpattern->openrecnotes.RemoveO(onote);
											break;
									}

									onote=onote->NextOpenNote();
								}

								recpattern->UnlockOpenNotes();

							}// status noteoff
							else
							{
								if(checkstatus==NOTEON && 
									rectrack->CheckMIDIInputEvent(mid)==true &&
									(recpattern=rectrack->GetMIDIRecordPattern(songposition))){

										// add new NoteOpen, dont create NoteON !
										if(NoteOpen *noteopen=new NoteOpen)
										{
											noteopen->ostart=mid->netime;
											noteopen->status=mid->status;
											noteopen->key=mid->byte1;
											noteopen->velocity=mid->byte2;

											recpattern->LockOpenNotes();
											recpattern->openrecnotes.AddEndO(noteopen);
											recpattern->events.accesscounter++;
											recpattern->UnlockOpenNotes();

											refreshloops=true;
											newdata=true;
										}	
								}

							}//else


						}//if

						rectrack=rectrack->NextTrack();

					}while(rectrack);

				}//if rectrack

				if(noteadded==true){

					Seq_Track *t=FirstTrack();

					while(t){

						MIDIPattern *recpattern;

						if(t->record==true && (recpattern=t->GetMIDIRecordPattern(songposition)))
						{
							// Check all Notes released ?
							recpattern->LockOpenNotes();
							if(recpattern->FirstOpenNote())
							{
								recpattern->UnlockOpenNotes();
								break;
							}
							recpattern->UnlockOpenNotes();
						}

						t=t->NextTrack();
					}

					if(!t) // songposition+= step
						songposition+=GetStepTime();
				}

			}// end == STEPRECORD
			else
			{
#ifdef DEBUG
				if(mid->netime<0)
					maingui->MessageBoxError(0,"NETIME<0");
#endif

				switch(mid->status)
				{
				case SYSEX:
					// Check For MTC ...
					if(mid->data && mid->datalength)
					{
						Seq_Track *rectrack=FirstTrack();

						while(rectrack)
						{
							if(rectrack->record==true && 
								/*(rectrack->tracktype==TRACKTYPE_ALL || */ rectrack->recordtracktype==TRACKTYPE_MIDI /*)*/ &&
								rectrack->CheckMIDIInputEvent(mid)==true
								){
									if(MIDIPattern *recpattern=rectrack->GetMIDIRecordPattern(mid->netime)) // Create or old record pattern
									{
										if(UBYTE *newbytedata=new UBYTE[mid->datalength]){

											if(SysEx *sysex=recpattern->NewSysEx(mid->netime)){

												sysex->flag|=EVENTFLAG_RECORDED;
												memcpy(newbytedata,mid->data,mid->datalength);
												sysex->data=newbytedata;
												sysex->length=mid->datalength;

												//sysex->CalcSysTicks();
												//sysex->CheckSysExEnd();

												newdata=true;
											}
											else
												delete newbytedata;
										}
									}
							}// if

							rectrack=rectrack->NextTrack();
						}
					}
					break;

				default: // normal MIDI Events
					{
						Seq_MIDIRouting_InputDevice *fid=inputrouting.FindInputDevice(mid->fromdev);
						Seq_Track *rectrack=FirstTrack();

						while(rectrack)
						{		
							if( ((mid->status&0xF0)==NOTEON && mid->byte2==0) || // NoteOn with Velo0=Off
								(mid->status&0xF0)==NOTEOFF
								){
									// STATUS: NoteOff
									// Search for old Note
									MIDIPattern *recpattern;

									if(((!rectrack->indevice) || rectrack->indevice==mid->fromdev) &&
										(recpattern=rectrack->record_MIDIPattern) // Pattern must Exists, no Off without On !
										){

											recpattern->LockOpenNotes();
											// Note OFF -> Note
											NoteOpen *onote=recpattern->FirstOpenNote();

											while(onote){

												if(onote->key==mid->byte1 && // Same Key ?
													(onote->status&0x0F)==(mid->status&0x0F) // Same Channel ?
													){
														// Cut at Cycle End
														if(playbacksettings.cycleplayback==true && mid->netime>=playbacksettings.cycleend)
															mid->netime=playbacksettings.cycleend-1;

														// NoteLength min x-x-x-1
														if(mid->netime<=onote->ostart)
															mid->netime=onote->ostart+1;

														OSTART length=mid->netime-onote->ostart,
															qtime=rectrack->GetFX()->quantizeeffect.Quantize(onote->ostart); // Quantize NoteOn Pos ?

														// noteon startpos
														if(Note *note=recpattern->NewNote(qtime,onote->ostart)){

															OSTART noteofftime=qtime+length;

															note->flag|=EVENTFLAG_RECORDED;
															note->status=onote->status;
															note->key=onote->key;
															note->velocity=onote->velocity;
															note->velocityoff=mid->byte2; // V Off
															note->off.staticostart=mid->netime;

															noteofftime=rectrack->GetFX()->quantizeeffect.Quantize(noteofftime);
															if(noteofftime<=qtime)noteofftime=qtime+1;

															recpattern->AddSortVirtual(&note->off,noteofftime); // NoteOff

															newdata=true;
														}

														recpattern->openrecnotes.RemoveO(onote);
														break;
												}

												onote=onote->NextOpenNote();
											} // while onote

											recpattern->UnlockOpenNotes();
									}//if
							} // NoteOff
							else{
								MIDIPattern *recpattern;

								if((mid->songstatusatinput&(STATUS_RECORD|STATUS_SONGPLAYBACK_MIDI)) &&
									rectrack->record==true &&
									CheckPunch(mid->netime)==true &&
									(rectrack->GetFX()->userouting==false || mid->fromplugin || (!mid->fromdev) || (fid && fid->CheckEvent(rectrack,mid->status,mid->byte1,mid->byte2,false)==true)) &&
									/*(rectrack->tracktype==TRACKTYPE_ALL || */rectrack->recordtracktype==TRACKTYPE_MIDI/*)*/ &&
									rectrack->CheckMIDIInputEvent(mid)==true

									)
								{
									switch(mid->status&0xF0)
									{				
									case NOTEON:
										if(recpattern=rectrack->GetMIDIRecordPattern(mid->netime))
										{	
											// add new NoteOpen, dont create NoteON !
											if(NoteOpen *noteopen=new NoteOpen){

												noteopen->ostart=mid->netime;

												noteopen->status=mid->status;
												noteopen->key=mid->byte1;
												noteopen->velocity=mid->byte2;

												recpattern->LockOpenNotes();
												recpattern->openrecnotes.AddEndO(noteopen);
												recpattern->events.accesscounter++;
												recpattern->UnlockOpenNotes();
											}	
										}
										break;

									case POLYPRESSURE:
										if(recpattern=rectrack->GetMIDIRecordPattern(mid->netime))
										{
											if(PolyPressure *poly=recpattern->NewPolyPressure(mid->netime)){
												poly->flag|=EVENTFLAG_RECORDED;
												poly->status=mid->status;
												poly->key=mid->byte1;
												poly->pressure=mid->byte2;
												newdata=true;
											}
										}
										break;

									case PITCHBEND:
										if(recpattern=rectrack->GetMIDIRecordPattern(mid->netime))
										{
											if(Pitchbend *pitch=recpattern->NewPitchbend(mid->netime)){
												pitch->flag|=EVENTFLAG_RECORDED;
												pitch->status=mid->status;
												pitch->lsb=mid->byte1;
												pitch->msb=mid->byte2;
												newdata=true;
											}
										}
										break;

									case PROGRAMCHANGE:
										if(recpattern=rectrack->GetMIDIRecordPattern(mid->netime))
										{
											if(ProgramChange *p=recpattern->NewProgramChange(mid->netime)){
												p->flag|=EVENTFLAG_RECORDED;
												p->status=mid->status;
												p->program=mid->byte1;
												newdata=true;
											}
										}
										break;

									case CHANNELPRESSURE:
										if(recpattern=rectrack->GetMIDIRecordPattern(mid->netime))
										{
											if(ChannelPressure *cp=recpattern->NewChannelPressure(mid->netime)){
												cp->flag|=EVENTFLAG_RECORDED;
												cp->status=mid->status;
												cp->pressure=mid->byte1;
												newdata=true;
											}
										}
										break;

									case CONTROLCHANGE:	
										{
											if(mid->byte1<128){

												if(mainMIDI->CheckIfChannelModeMessage(mid->status,mid->byte1)==false) // dont record All Notes Off etc...
												{
													if(recpattern=rectrack->GetMIDIRecordPattern(mid->netime))
													{
														if(ControlChange *cc=recpattern->NewControlChange(mid->netime)){
															cc->flag|=EVENTFLAG_RECORDED;
															cc->status=mid->status;
															cc->controller=mid->byte1;
															cc->value=mid->byte2;
															newdata=true;
														}
													}
												}
											}
										}
										break;
									}// switch event
								}//if
							}// else standard event

							rectrack=rectrack->NextTrack();
						}// while rectrack

					}// default event
					break;

				}// switch status
			}
		} // song==aong

		mainMIDIrecord->Lock();
		mid=mainMIDIrecord->DeleteMIDIRecordData(mid);
		mainMIDIrecord->UnLock();
	}// while

	if(newdata==false)
	{
		Seq_Track *rt=FirstTrack();

		while(rt && newdata==false)
		{
			if(rt->record_MIDIPattern)
			{
				newdata=true;
				rt->checkcrossfade=true;
				break;
			}

			for(int i=0;i<MAXRECPATTERNPERTRACK;i++)
			{
				if(rt->audiorecord_audiopattern[i] && rt->audiorecord_audiopattern[i]->audioevent.audioefile->samplesperchannel>0)
				{
					newdata=true;
					rt->checkcrossfade=true;
					break;
				}
			}

			rt=rt->NextTrack();
		}
	}

	return newdata;
}

bool Seq_Song::RepairCrossFades()
{
	bool repair=false;

	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->checkcrossfade==false)
		{
			// Check Tracks CrossFades
			Seq_Pattern *p=t->FirstPattern();
			while(p && t->checkcrossfade==false)
			{
				if(p->FirstCrossFade()) // old CF exists ?
				{
					t->checkcrossfade=true;
					repair=true;
				}
				else
				{
					Seq_Pattern *np=p->NextPattern();

					while(t->checkcrossfade==false && np && np->GetPatternStart()<p->GetPatternEnd())
					{
						t->checkcrossfade=true;
						repair=true;
						np=np->NextPattern();
					}
				}

				p=p->NextPattern();
			}
		}

		t=t->NextTrack();
	}

	if(repair==true)
		CheckCrossFades();

	return repair;
}

void Seq_Song::CheckCrossFades()
{
	TRACE ("Start CheckCrossFades\n");

	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->checkcrossfade==true)
		{
			t->checkcrossfade=false; // reset
			t->GenerateCrossFades();
		}

		t=t->NextTrack();
	}

	TRACE ("End CheckCrossFades\n");
}


int Seq_Song::GetCountSelectedTracks()
{
	int c=0;

	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t->IsSelected()==true)c++;
		t=t->NextTrack();
	}

	return c;
}

int Seq_Song::GetCountTrackWithAudioOut(Seq_Track *not)
{
	int c=0;

	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(not!=t && t->outputisinput==true)c++;
		t=t->NextTrack();
	}

	return c;
}

void Seq_Song::SelectTracksExcept(Seq_Track *extrack,bool onoff,bool onoffextrack)
{
	int changed=0;

	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t!=extrack)
		{
			if(onoff==true)
			{
				if(!(t->flag&OFLAG_SELECTED))
				{
					t->flag |=OFLAG_SELECTED;
					changed++;
				}
			}
			else
			{
				if(t->IsSelected()==true)
				{
					t->UnSelect();
					changed++;
				}
			}
		}
		else
		{
			if(onoffextrack==true)
			{
				if(t->IsSelected()==false)
				{
					t->Select();
					changed++;
				}
			}
			else
			{
				if(t->IsSelected()==true)
				{
					t->UnSelect();
					changed++;
				}
			}
		}

		t=t->NextTrack();
	}

	if(changed)
	{
		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			if(w->WindowSong()==this)
				switch(w->GetEditorID())	
			{
				case EDITORTYPE_ARRANGE:
					{
						Edit_Arrange *ar=(Edit_Arrange *)w;
						ar->ShowHoriz(true,false,false);
					}
					break;
			}

			w=w->NextWindow();
		}
	}
}

void Seq_Song::SelectSingleTrack(Seq_Track *track)
{
	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t!=track)
			t->UnSelect();
		else
			t->Select();

		t=t->NextTrack();
	}
}

void Seq_Song::SelectTracksFromTo(Seq_Track *from,Seq_Track *to,bool unselect,bool toggle)
{
	if(from && to)
	{
		if(GetOfTrack(from)>GetOfTrack(to))
		{
			Seq_Track *h=to;
			to=from;
			from=h;
		}

		if(unselect==true)
		{
			Seq_Track *t=from->PrevTrack();
			while(t)
			{
				t->UnSelect();
				t=t->PrevTrack();
			}

			t=to->NextTrack();
			while(t)
			{
				t->UnSelect();
				t=t->NextTrack();
			}
		}

		do
		{
			if(toggle==true)
			{
				if(from->IsSelected()==true)
					from->UnSelect();
				else
				{
					if(from->IsOpen()==true)
						from->Select();
				}

				//changed++;
			}
			else
			{
				if(from->IsSelected()==false && from->IsOpen()==true)
					from->Select();
			}

			if(from==to)
				return;

			from=from->NextTrack();

		}while(from);
	}
}

void  Seq_Song::UnSelectTracksFromTo(Seq_Track *from,Seq_Track *to)
{
	if(from && to)
	{
		int changed=0;

		if(GetOfTrack(from)>GetOfTrack(to))
		{
			Seq_Track *h=to;
			to=from;
			from=h;
		}

		while(from)
		{
			if(from->IsSelected()==true)
			{
				from->UnSelect();
				changed++;
			}

			if(from==to)
				break;

			from=from->NextTrack();
		}

		if(changed)
		{
			guiWindow *w=maingui->FirstWindow();

			while(w)
			{
				if(w->WindowSong()==this)
					switch(w->GetEditorID())	
				{
					case EDITORTYPE_ARRANGE:
						{
							Edit_Arrange *ar=(Edit_Arrange *)w;
							ar->ShowHoriz(true,false,false);
						}
						break;
				}

				w=w->NextWindow();
			}
		}
	}
}

/*

// DrumMaps
Drummap *Seq_Song::AddDrumMap(Drummap *map)
{
	if(!map)
		map=new Drummap;

	if(!defaultdrummap)
		defaultdrummap=map;

	if(map)
	{
		map->song=this;
		drummaps.AddEndO(map);
	}

	return map;
}

Drummap *Seq_Song::DeleteDrumMap(Drummap *map)
{
	if(defaultdrummap==map)
		defaultdrummap=(Drummap *)map->NextOrPrev();

	map->FreeMemory();
	return (Drummap *)drummaps.RemoveO(map);
}

void Seq_Song::DeleteAllDrumMaps()
{
	Drummap *m=FirstDrumMap();

	while(m)
		m=DeleteDrumMap(m);
}
*/

void Seq_Song::InitDefaultDrumMap()
{
	//Drummap *map=AddDrumMap(0);

	if(drummap.FirstTrack()==0)
	{
	drummap.DeleteAllDrumTracks();
	drummap.InitGMDrumMap();
	}
}

void Seq_Song::SetMousePosition(OSTART mp)
{
	mouseposition=mp;
}

void Seq_Song::ConvertTicksToTempoSampleRate()
{
	playbacksettings.ConvertTicksToTempoSampleRate();	
}

void Seq_Song::SetGUIZoom(int ticks)
{
	if(ticks!=timetrack.zoomticks)
	{
		timetrack.zoomticks=ticks;

		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_TRANSPORT:
				if(this==mainvar->GetActiveSong())
				{
					Edit_Transport *ed=(Edit_Transport *)w;

					ed->ShowSongZoom();
				}
				break;
			}

			if(w->WindowSong()==this)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_ARRANGE:
				case EDITORTYPE_SCORE:
				case EDITORTYPE_DRUM:
				case EDITORTYPE_EVENT:
				case EDITORTYPE_WAVE:
				case EDITORTYPE_TEMPO:
				case EDITORTYPE_PIANO:
					{
						EventEditor *ee=(EventEditor *)w;

						ee->DrawHeader();
						ee->RefreshObjects(0,false);
					}
					break;
				}
			}

			w=w->NextWindow();
		}
	}
}

SoloPattern *Seq_Song::FindSoloPattern(Seq_SelectedPattern *sp)
{
	SoloPattern *solo=(SoloPattern *)solopattern.GetRoot();

	while(solo)
	{
		if(solo->spattern==sp)
			return solo;

		solo=(SoloPattern *)solo->next;
	}

	return 0;
}

int Seq_Song::CheckSoloPattern(Seq_Pattern *pattern)
{
	SoloPattern *solo=(SoloPattern *)solopattern.GetRoot();

	if(solo)
	{
		while(solo)
		{
			if(solo->spattern->pattern==pattern ||
				(pattern->itsaloop==true && solo->spattern->pattern==pattern->mainpattern)
				)
				return 1;

			solo=(SoloPattern *)solo->next;
		}

		return 0;
	}

	return -1;
}

void Seq_Song::RemoveFromSoloOnOff(guiWindow *w)
{
	int c=0;
	SoloPattern *solo=(SoloPattern *)solopattern.GetRoot();

	while(solo)
	{
		SoloPattern *nsolo=(SoloPattern *)solo->next;

		if(solo->window==w)
		{
			RemoveSoloPattern(solo);
			c++;
		}

		solo=nsolo;
	}

	if(c)
		SetMuteFlags();
}

SoloPattern *Seq_Song::AddSoloPattern(guiWindow *w,Seq_SelectedPattern *sp)
{
	SoloPattern *newsolo=new SoloPattern;

	if(!newsolo)
		return 0;

	newsolo->spattern=sp;
	newsolo->window=w;

	solopattern.AddEndO(newsolo);
	return newsolo;
}

void Seq_Song::RemoveSoloPattern(SoloPattern *sp)
{
	solopattern.RemoveO(sp);
}

void Seq_Song::TempSoloOnOff(guiWindow *win,Seq_SelectionList *sl)
{
	bool c=false;

	if(sl->playsolo==true)
	{
		// Add
		Seq_SelectedPattern *sp=sl->FirstSelectedPattern();

		while(sp)
		{
			if(!FindSoloPattern(sp))
			{
				AddSoloPattern(win,sp);
				c=true;
			}

			sp=sp->NextSelectedPattern();
		}
	}
	else
	{
		// Remove
		Seq_SelectedPattern *sp=sl->FirstSelectedPattern();

		while(sp)
		{
			if(SoloPattern *solo=FindSoloPattern(sp))
			{
				RemoveSoloPattern(solo);
				c=true;
			}

			sp=sp->NextSelectedPattern();
		}
	}

	if(c==true)
		SetMuteFlags();
}

#ifdef DEBUG
bool checkerror=false;
#endif

void Seq_Song::RefreshRealtime_Slow()
{
#ifdef DEBUG

	// TestClass

	if(checkerror==false)
	{
		if(timetrack.tempomap.CheckIndex()==false)
		{
			maingui->MessageBoxOk(0,"TempoMap Index Error");
			checkerror=true;
			return;
		}

		{
			int tix=0;
			Seq_Track *t=FirstTrack();

			while(t)
			{
				
				if(t->GetTrackIndex()!=tix)
				{
					checkerror=true;
					maingui->MessageBoxOk(0,"Track Index Error");
					return;
				}

				tix++;

				if(t->parent)
				{
					int tpx=0;

					FolderObject *cfo=t->parent->FirstChild();
					while(cfo)
					{
						if(cfo==t)
						{
							if(t->GetIndex()!=tpx)
							{
								checkerror=true;
								maingui->MessageBoxOk(0,"Track Child Index Error");
								return;
							}

							goto checkpattern;
						}

						tpx++;

						cfo=cfo->NextChild();
					}

					checkerror=true;
					maingui->MessageBoxOk(0,"Track Child Not In List Error");
					return;
				}

checkpattern:

				Seq_Pattern *p=t->FirstPattern(MEDIATYPE_MIDI);

				while(p)
				{
					MIDIPattern *mp=(MIDIPattern *)p;

					if(mp->recordpattern==false)
					{
						if(mp->events.CheckIndex()==false)
						{
							checkerror=true;
							maingui->MessageBoxOk(0,"Pattern Index Error");
							return;
						}
					}

					p=p->NextPattern(MEDIATYPE_MIDI);
				}

				t=t->NextTrack();
			}
		}
	}

#endif

	// MTC Check
	if(MIDIsync.startedwithmtc==true)
	{
		if(MIDIsync.mtccheckcounter==2)
		{
			MIDIsync.mtccheckcounter=0;

			if(MIDIsync.mtccheckstart==MIDIsync.mtccounter)
			{
				MIDIsync.startedwithmtc=false;

				if(status!=STATUS_STOP || (status&STATUS_STEPRECORD))
					StopSong(0,GetSongPosition());
			}
			else
				MIDIsync.mtccheckstart=MIDIsync.mtccounter;
		}
		else
			MIDIsync.mtccheckcounter++;
	}

	// Track Names
	Seq_Track *t=FirstTrack();

	while(t)
	{
		InsertAudioEffect *iae=t->io.audioeffects.FirstInsertAudioEffect();
		while(iae)
		{
			if(iae->audioeffect->settrackname==true)
			{
				char *h,*progn=iae->audioeffect->GetProgramName();

				if(progn)
					h=mainvar->GenerateString(iae->audioeffect->GetEffectName(),":",progn);
				else
					h=mainvar->GenerateString(iae->audioeffect->GetEffectName(),"::");

				if(h)
				{
					if(strcmp(t->GetName(),h)!=0)
					{
						t->SetName(h);
						t->ShowTrackName(0);
					}

					delete h;
				}

				if(progn)
					delete progn;

				break;
			}

			iae=iae->NextEffect();
		}

		t=t->NextTrack();
	}
}


void Seq_Song::AudioInputNeeded()
{
	bool used=false;

	if(mainaudio->GetActiveDevice())
	{
		{
			AudioHardwareChannel *c=mainaudio->GetActiveDevice()->FirstInputChannel();

			while(c)
			{
				c->canbedisabled=true;
				c=c->NextChannel();
			}
		}

		if(audiosystem.systemhasflag&AudioSystem::AUDIOCAMX_HASTHRU)
		{
			used=true;
			//return;
		}

		if(audiosystem.systemhasflag&AudioSystem::AUDIOCAMX_HASINPUTEFFECTS)
		{
			used=true;
			//return;
		}

		Seq_Track *t=FirstTrack();

		while(t){

			if(t->io.tempinputmonitoring==true)
			{
				t->io.ActivateInputs();
				used=true;
				//return;
			}

			t=t->NextTrack();
		}

		for(int i=0;i<LASTSYNTHCHANNEL;i++)
		{
			AudioChannel *c=audiosystem.FirstChannelType(i);

			while(c)
			{
				if(c->io.thru==true)
				{
					used=true;
					//return;
				}

				c=c->NextChannel();
			}
		}

		{
			AudioHardwareChannel *c=mainaudio->GetActiveDevice()->FirstInputChannel();

			while(c)
			{
				if(c->canbedisabled==true)
				{
					//if(c->hwinputbuffers)
					c->hwinputbuffer.channelsused=0;
					c->inputused=false;
				}

				c=c->NextChannel();
			}
		}
	}

	audioinputneed=used;
}

bool Seq_Song::CheckStatus(UBYTE istatus)
{
	bool recordstarted=false;

	// Start Playback ?
	// Start/Record Check
	if(istatus==NOTEON || istatus==SYSEX)
	{
		if(mainsettings->noteendsprecounter==true)
		{
			// Start Playback ?
			if(status&STATUS_MIDIWAIT)
			{									
				status CLEARBIT STATUS_MIDIWAIT;

				if(!mainaudio->GetActiveDevice())
				{
					startMIDIinit=true;
					mainMIDIalarmthread->SetSignal(); // Start MIDI Signal
				}
			}
			else
			{
				// Start Record ?
				if(status&STATUS_WAITPREMETRO)
				{									
					recordstarted=true;

					if(!mainaudio->GetActiveDevice())
					{
						status CLEARBIT STATUS_WAITPREMETRO;
						startMIDIinit=true;
						mainMIDIalarmthread->SetSignal(); // Start MIDI Signal
					}
					else
					{
						status |=STATUS_WAITAUDIOPREMETROCANCELED;
					}
				}
			}
		}
	}

	return recordstarted;
}
