#include "songmain.h"
#include "arrangeeditor.h"
#include "object_track.h"
#include "MIDIhardware.h"
#include "editfunctions.h"
#include "semapores.h"
#include "audiofile.h"
#include "gui.h"
#include "undo.h"
#include "peakbuffer.h"
#include "audiohardware.h"
#include "drumevent.h"
#include "MIDIautomation.h"
#include "audioauto_volume.h"
#include "audioauto_vst.h"
#include "MIDIoutproc.h"
#include "editMIDIfilter.h"
#include "audiothread.h"
#include "editsettings.h"
#include "quantizeeditor.h"
#include "audiohdfile.h"
#include "audiomixeditor.h"
#include "MIDIprocessor.h"
#include "initplayback.h"
#include "crossfade.h"
#include "audiopattern.h"
#include "object_project.h"
#include "vstguiwindow.h"
#include "edit_audiointern.h"
#include "languagefiles.h"
#include "chunks.h"
#include "audioauto_volume.h"
#include "object_song.h"
#include "audiodevice.h"

Seq_Track *Seq_Track::NextTrack()
{
	mainthreadcontrol->Lock(CS_UI); // Avoid OO List Conflicts
	Seq_Track *r=(Seq_Track *)NextQObject();
	mainthreadcontrol->Unlock(CS_UI);

	return r;
}

Seq_Track *Seq_Track::PrevTrack()
{
	mainthreadcontrol->Lock(CS_UI); // Avoid OO List Conflicts
	Seq_Track *r=(Seq_Track *)PrevQObject();
	mainthreadcontrol->Unlock(CS_UI);
	return r;
}

Seq_Track *Seq_Track::PrevParentTrack()
{
	Seq_Track *p=PrevTrack();
	while(p && p->parent)p=p->PrevTrack();
	return p;
}

Seq_Track *Seq_Track::NextParentTrack()
{
	Seq_Track *p=NextTrack();
	while(p && p->parent)p=p->NextTrack();
	return p;
}

Seq_Pattern *Seq_Track::FirstPattern(int mediatype)
{
	Seq_Pattern *p=frozen==true?frozenpattern:(Seq_Pattern *)pattern.GetRoot();

	while(p){
		if(p->mediatype&mediatype)return p;
		p=p->NextPattern(mediatype);
	}

	return 0; 
}

Seq_Pattern* Seq_Track::FirstPattern_NotFrozen(int mediatype)
{
	Seq_Pattern *p=(Seq_Pattern *)pattern.GetRoot();

	while(p){
		if(p->mediatype&mediatype)return p;
		p=p->NextPattern(mediatype);
	}

	return 0; 
}

Seq_Pattern* Seq_Track::LastPattern(int mediatype)
{
	Seq_Pattern *p=frozen==true?frozenpattern:(Seq_Pattern *)pattern.Getc_end();

	while(p){
		if(p->mediatype&mediatype)return p;
		p=p->PrevPattern(mediatype);
	}

	return 0; 
}

OSTART Seq_Track::GetEnd()
{
	OSTART end=0;
	Seq_Pattern *p=FirstPattern(MEDIATYPE_ALL);

	while(p){
		if(p->GetPatternEnd()>end)end=p->GetPatternEnd();
		p=p->NextPattern(MEDIATYPE_ALL);
	}

	return end;
}

/*
LONGLONG Seq_Track::AddDelay(LONGLONG sampleposition)
{
OSTART dl=GetFX()->GetDelay();

#ifdef DEBUG
if(dl<0)
maingui->MessageBoxError(0,"Delay <0");
#endif

if(dl)
{
double ddelay=mainaudio->ConvertInternToExternSampleRate(dl);
return sampleposition+ddelay;
}

return sampleposition;
}
*/

LONGLONG Seq_Track::AddSampleDelay(LONGLONG h)
{
	if(double d=(double)GetFX()->GetDelay()){
		d*=song->timetrack.ppqsampleratemul; // Intern to Samples
		LONGLONG rt=h+(LONGLONG)d;
		return rt>0?rt:0;
	}

	return h;
}

LONGLONG Seq_Track::AddFullSampleDelay(LONGLONG h)
{
	if(double d=(double)GetFX()->GetDelay()){
		d*=song->timetrack.ppqsampleratemul; // Intern to Samples
		return h+(LONGLONG)d;
	}

	return h;
}

void Seq_Track::AddDelay(OSTART *h1,OSTART *h2)
{
	OSTART d=GetFX()->GetDelay();

	*h1+=d;
	*h2+=d;
}

OSTART Seq_Track::AddDelay(OSTART h)
{
	return h+GetFX()->GetDelay();
}

void Seq_Track::StopAll()
{
	for(int i=0;i<REALTIME_LISTS;i++){

		song->realtimeevents[i].Lock();

		RealtimeEvent *re=song->realtimeevents[i].FirstREvent();

		while(re){

			if(re->fromtrack==this){
				re->SendQuick();
				re=song->realtimeevents[i].DeleteREvent(re);
			}
			else
				re=re->NextEvent();
		}

		song->realtimeevents[i].UnLock();
	}

	LockOpenOutputNotes();
	SendAllOpenOutputNotes();
	UnlockOpenOutputNotes();
}

int Seq_Track::GetMediaTypes()
{
	int types=0;
	Seq_Pattern *p=FirstPattern(MEDIATYPE_ALL);

	while(p){
		types|=p->mediatype;
		p=p->NextPattern(MEDIATYPE_ALL);
	}

	return types;
}

int Seq_Track::GetCountOfPattern(int mediatype)
{
	Seq_Pattern *p=FirstPattern(mediatype);
	int c=0;

	while(p){
		if(p->itsaloop==false)c++;
		p=p->NextPattern(mediatype);
	}

	return c;
}

int Seq_Track::GetCountChildTracksPattern()
{
	int c=0;

	Seq_Track *ch=FirstChildTrack();
	while(ch && ch->IsTrackChildOfTrack(this))
	{
		c+=ch->GetCountOfPattern(MEDIATYPE_ALL);
		ch=ch->NextTrack();
	}

	return c;
}

int Seq_Track::GetCountSelectedPattern()
{
	int c=0;

	Seq_Pattern *p=FirstPattern();

	while(p)
	{
		if(p->itsaloop==false && p->IsSelected()==true)
			c++;

		p=p->NextPattern();
	}

	return c;
}

int Seq_Track::GetCountOfPattern_NotFrozen(int mediatype)
{
	Seq_Pattern *p=FirstPattern_NotFrozen(mediatype);
	int c=0;

	while(p){

		if(p->itsaloop==false)
		{
			bool ok=true;

			switch(p->mediatype)
			{
			case MEDIATYPE_AUDIO:
				{
					// Check If Audio Pattern is Audio Work File...

					AudioPattern *ap=(AudioPattern *)p;

					if(ap->waitforresample==true)
						ok=false;
				}
				break;

			}

			if(ok==true)
				c++;
		}

		p=p->NextPattern(mediatype);
	}

	return c;
}

Seq_Pattern *Seq_Track::GetPatternIndex(int i,int mediatype)
{
	Seq_Pattern *p=FirstPattern(mediatype);

	while(p){

		if(p->itsaloop==false) // no loops
		{
			if(i==0)return p;
			i--;
		}

		p=p->NextPattern(mediatype);
	}

	return 0;
}

void Seq_Track::DeletePatternMemoryFromTrack(Seq_Pattern *patt,bool full,bool deletefromclones)
{
	if(song->GetFocusPattern()==patt)
		song->SetFocusPattern(0);

	patt->StopAllofPattern();

	// jump over loops
	if((!patt->mainpattern) || patt->mainclonepattern)
	{	
		patt->DeleteLoops();
	}

	if(!patt->mainclonepattern)
	{
		if(full==true)patt->DeleteClones();
		else
			if(deletefromclones==true)patt->CutClones();
	}
	else
	{
		// Clone Remove From Main Pattern
		if(deletefromclones==true)
			patt->mainclonepattern->RemovePatternFromClones(patt);
	}

	// Record Pattern ?
	if(record_MIDIPattern==patt)record_MIDIPattern=0;
	if(patt->link)patt->link->RemovePattern(patt);
}

Seq_Pattern* Seq_Track::CutPattern(Seq_Pattern *patt,bool deletefromclones)
{
	DeletePatternMemoryFromTrack(patt,false,deletefromclones);
	return(Seq_Pattern *)pattern.CutObject(patt);
}

Seq_Pattern* Seq_Track::DeletePattern(Seq_Pattern *patt,bool full,bool deletefromclones)
{	
	DeletePatternMemoryFromTrack(patt,full,deletefromclones);
	Seq_Pattern *n=(Seq_Pattern *)pattern.CutObject(patt);
	patt->Delete(full);
	return n;
}

int Seq_Track::GetOfPattern(Seq_Pattern *p,int mediatype)
{
	Seq_Pattern *f=FirstPattern(mediatype);
	int c=0;

	while(f){
		if(f->mainpattern==0 || f->mainclonepattern) // no loops
		{
			if(f==p)return c;
			c++;
		}

		f=f->NextPattern(mediatype);
	}

	return -1;
}

void Seq_Track::SetFrozenPattern(AudioPattern *ap,OSTART startposition)
{
	if(ap)
	{
		ap->track=this;
		ap->mediatype=MEDIATYPE_AUDIO;
		ap->audioevent.staticostart=ap->audioevent.ostart=ap->ostart=startposition;
		ap->prev=ap->next=0;

		frozenpattern=ap;
		frozen=true;
	}
}

void Seq_Track::AddSortPattern(Seq_Pattern *p,OSTART pstart)
{
	p->track=this;
	p->InitDefaultVolumeCurve();
	pattern.AddOSort(p,pstart);
}

void Seq_Track::MovePatternToTrack(Seq_Pattern *pp,Seq_Track *totrack,OSTART qposition)
{	
	OSTART diff=qposition-pp->GetPatternStart();

	DeletePattern(pp,false,false); // dont delete from clones

	pp->DeleteAllCrossFades(true,true);

	totrack->AddSortPattern(pp,qposition); // add pattern to new track
	pp->MovePatternData(diff,0);
	pp->RefreshAfterPaste();

	// Move Loops ?
	Seq_LoopPattern *lp=pp->FirstLoopPattern();
	while(lp)
	{
		DeletePattern(lp->pattern,false);

		lp->loopstart+=diff;
		totrack->AddSortPattern(lp->pattern,lp->loopstart);
		lp->pattern->RefreshAfterPaste();

		lp=lp->NextLoop();
	}

	if(diff) // <-> ?
	{
		if(pp->FirstClone())
			pp->SetClonesOffset();
		else
			if(pp->mainclonepattern)
				pp->mainclonepattern->SetClonesOffset();
	}
}

void Seq_Track::InitTrack(Seq_Song *i_song)
{
	id=OBJ_TRACK;

	track=this;

	tracktype=TRACKTYPE_DATA;
	song=i_song;
	childs.listtype=OLISTTYPE_FOLDERPARENT;
	childs.parent=this;

	MIDItype=OUTPUTTYPE_MIDI;
	MIDItypesetauto=true;
	skiptrackfromCreateUsedAudioTrackList=false;

	index=-1; // Not set
	recordcycleindex=0;
	flag=0;
	trackname=0;

	for(int i=0;i<INITPLAY_MAX;i++)
	{
		playback_MIDIPattern[i][0]=playback_MIDIPattern[i][1]=
			playback_chainnoteMIDIPattern[i][0]=playback_chainnoteMIDIPattern[i][1]=0;

		playback_audiopattern[i]=0;
	}

	for(int i=0;i<MAXRECPATTERNPERTRACK;i++)audiorecord_audiopattern[i]=0;

	rawpointer=0;
	recordindex=1;
	record_MIDIPattern=0;
	indevice=0;
	MIDInputimpulse=MIDIoutputimpulse=MIDInputimpulse_data=0;

	audioinpeak=0;

	//false
	record_cyclecreated=
		cyclerecordmute=
		checkcrossfade=
		recordbeforestart=
		record=
		outputisinput=
		solobuffer=
		frozen=
		tmp_frozen=
		underfreeze=false;

	frozenhdfile=0;
	frozenfilestart=0;
	frozenpattern=0;

	activeautomationtrack=0;

	t_trackeffects.track=this;
	t_trackeffects.quantizeeffect.track=this;

	recordcounter=-1;
	t_processor=0;
	recordtracktype=mainsettings->defaultrecordtracktype;
	t_audiofx.track=this;

	io.audioeffects.track=io.audioinputeffects.track=this;
	io.audioinputeffects.inputeffects=true;

	t_audiochannelouts.track=this;

	input_par.track=this;
	input_par.io=&io;

	output_par.track=this;
	output_par.io=&io;

	trackimage=0;
	useaudio=false;
	icon=0;

	frozenfile=0;
	usedirecttodevice=true;
	t_muteflag=false;
	
	createdatcycleloop=0;
	ismetrotrack=false;

	freezetype=FREEZE_OFF;
	songstartposition=0;

	for(int i=0;i<5;i++)
		recordsettings_record[i]=false;

	SetDefaultMetroSettings();
}

void Seq_Track::SetDefaultMetroSettings()
{
	metrochl_b=mainsettings->defaultmetrochl_b;
	metroport_b=mainsettings->defaultmetroport_b;
	metrokey_b=mainsettings->defaultmetrokey_b;
	metrovelo_b=mainsettings->defaultmetrovelo_b;
	metrovelooff_b=mainsettings->defaultmetrovelooff_b;

	metrochl_m=mainsettings->defaultmetrochl_m;
	metroport_m=mainsettings->defaultmetroport_m;
	metrokey_m=mainsettings->defaultmetrokey_m;
	metrovelo_m=mainsettings->defaultmetrovelo_m;
	metrovelooff_m=mainsettings->defaultmetrovelooff_m;

	metrosendtoMIDI=mainsettings->defaultmetrosendMIDI;
	metrosendtoaudio=mainsettings->defaultmetrosendaudio;
}

void Seq_Track::SetOrGetRecordingSettings(int index,bool getmode)
{
	if(getmode==false)
		recordsettings_record[index]=record;
	else
		SetRecordMode(recordsettings_record[index],0);

	if(IsSelected()==true && maingui->GetCtrlKey()==false)
	{
		Seq_Track *t=song->FirstTrack();
		while(t)
		{
			if(t->IsSelected()==true)
			{
				if(getmode==false)
					t->recordsettings_record[index]=t->record;
				else
					t->SetRecordMode(t->recordsettings_record[index],0);
			}

			t=t->NextTrack();
		}
	}
}

void Seq_Track::SetSolo(bool s)
{
	io.audioeffects.SetSolo(s);
}

bool Seq_Track::GetSolo()
{
	return io.audioeffects.GetSolo();
}

int Seq_Track::GetSoloStatus()
{
	if(GetSolo()==false)
	{
		if(parent && ((Seq_Track *)parent)->GetSolo())
			return 0;

		return song->solocount?2:0;
	}

	return 1;
}

bool Seq_Track::CheckIfAudioRecording()
{
	for(int i=0;i<MAXRECPATTERNPERTRACK;i++)
		if(audiorecord_audiopattern[i])
			return true;

	return false;
}

bool Seq_Track::CheckSoloTrack()
{
	if(GetSolo()==true)
		return true;

	if(parent && ((Seq_Track *)parent)->CheckSoloTrack()==true)
		return true;

	/*
	Seq_Track *ct=FirstChildTrack();
	while(ct)
	{
	if(ct->GetSolo()==true)
	return true;

	ct=ct->NextChildTrack();
	}
	*/
	return false;
}

bool Seq_Track::GetMute()
{
	if(cyclerecordmute==true)return true;
	if(io.audioeffects.GetMute()==true)return true;
	if(parent && ((Seq_Track *)parent)->GetMute()==true)return true;
	return false;
}

void Seq_Track::GenerateCrossFades()
{
	return;

	Seq_Pattern *dp=FirstPattern();

	while(dp){
		dp->MarkAllCrossFades();
		dp=dp->NextPattern();
	}

	// Generate Tracks CrossFades
	AudioPattern *ap=(AudioPattern *)FirstPattern(MEDIATYPE_AUDIO);
	while(ap)
	{
		AudioPattern *nap=(AudioPattern *)ap->NextPattern(MEDIATYPE_AUDIO);

		while(nap && nap->itsaloop==false && nap->GetPatternStart()<ap->GetPatternEnd())
		{
			Seq_CrossFade *find=nap->FindCrossFade(ap);

			if((!find) || find->dontdeletethis==false)
			{
				if(find && find->connectwith)
				{
					while(find && find->pattern==nap)
					{
						if(find->connectwith)
						{
							find->connectwith->deletethis=find->deletethis=false;

							if(find->infade==true){
								find->connectwith->from=find->from=nap->GetPatternStart();
								find->connectwith->to=find->to=ap->GetPatternEnd();
							}

							find->SetCrossFadeSamplePositions();
							find->connectwith->SetCrossFadeSamplePositions();
						}

						find=find->NextCrossFade();
					}
				}
				else
				{
					bool patternallinside=nap->GetPatternEnd()<=ap->GetPatternEnd()?true:false;
					OSTART to;

					if(patternallinside==false)
					{
						to=ap->GetPatternEnd();
					}
					else
					{
						// 50%
						OSTART h=nap->GetPatternEnd()-nap->GetPatternStart();
						to=nap->GetPatternStart()+h/2;
					}

					// Inside ?
					if(Seq_CrossFade *cf=new Seq_CrossFade)
					{
						cf->from=nap->GetPatternStart();
						cf->to=to;

						cf->infade=false;

						// <<
						if(Seq_CrossFade *pf=new Seq_CrossFade)
						{
							pf->from=cf->from;
							pf->to=cf->to;
							pf->infade=true;

							pf->connectwith=cf;
							cf->connectwith=pf;

							ap->Lock_CrossFades();
							nap->Lock_CrossFades();

							ap->AddCrossFade(cf);
							nap->AddCrossFade(pf);

							pf->SetCrossFadeSamplePositions();
							cf->SetCrossFadeSamplePositions();

							nap->Unlock_CrossFades();
							ap->Unlock_CrossFades();

						}
						else
							cf->connectwith=0;
					}

					// Patter [In Pattern]
					if(patternallinside==true)
					{
						if(Seq_CrossFade *cf=new Seq_CrossFade)
						{
							cf->from=to;
							cf->to=nap->GetPatternEnd();

							cf->infade=true;

							// <<
							if(Seq_CrossFade *pf=new Seq_CrossFade)
							{
								pf->from=cf->from;
								pf->to=cf->to;
								pf->infade=false;

								pf->connectwith=cf;
								cf->connectwith=pf;

								ap->Lock_CrossFades();
								nap->Lock_CrossFades();

								ap->AddCrossFade(cf);
								nap->AddCrossFade(pf);

								pf->SetCrossFadeSamplePositions();
								cf->SetCrossFadeSamplePositions();

								nap->Unlock_CrossFades();
								ap->Unlock_CrossFades();

							}
							else
								cf->connectwith=0;
						}

					}

				}
			}

			nap=(AudioPattern *)nap->NextPattern(MEDIATYPE_AUDIO);
		}

		ap=(AudioPattern *)ap->NextPattern(MEDIATYPE_AUDIO);
	}

	dp=FirstPattern();

	while(dp)
	{
		dp->DeleteMarkedCrossFades();
		dp=dp->NextPattern();
	}
}

OSTART Seq_Track::GetTrackLength()
{
	if(Seq_Pattern *p=FirstPattern())
	{
		OSTART e=0;

		while(p){

			if(p->itsaloop==false)  // no loops !
			{
				OSTART h=p->GetPatternEnd();
				if(h>e)
					e=h;
			}

			p=p->NextPattern();
		}

		return e;
	}

	return 0;
}

void Seq_Track::Load(camxFile *file)
{
	// Chunk Track
	file->ReadAndAddClass((CPOINTER)this);

	file->ReadChunk(&parentindex);
	file->ReadChunk(&childdepth);

	file->Read_ChunkString(&trackname);
	file->ReadChunk(&recordtracktype);

	if(recordtracktype==TRACKTYPE_ALL)
		recordtracktype=TRACKTYPE_MIDI;

	file->ReadChunk(&frozen);
	file->ReadChunk(&frozenfilestart);
	file->Read_ChunkString(&frozenfile);
	file->ReadChunk(&recordindex);

	t_colour.LoadChunk(file);

	file->Read_ChunkString(&trackimage);

	if(trackimage)
		icon=maingui->gfx.FindIcon(trackimage);

	file->ReadChunk(&MIDItype);
	file->ReadChunk(&usedirecttodevice);
	file->ReadChunk(&MIDItypesetauto);
	file->ReadChunk(&showchilds);
	file->ReadChunk(&sizefactor);
	file->ReadChunk(&record);
	file->ReadChunk(&outputisinput);
	file->ReadChunk(&vutype);

	file->AddPointer((CPOINTER)&t_audiofx.recordtrack);

	file->ReadChunk(&tracktype);
	file->ReadChunk(&ismetrotrack);

	file->ReadChunk(&metrochl_b);
	file->ReadChunk(&metroport_b);
	file->ReadChunk(&metrokey_b);
	file->ReadChunk(&metrovelo_b);
	file->ReadChunk(&metrovelooff_b);

	file->ReadChunk(&metrochl_m);
	file->ReadChunk(&metroport_m);
	file->ReadChunk(&metrokey_m);
	file->ReadChunk(&metrovelo_m);
	file->ReadChunk(&metrovelooff_m);
	file->ReadChunk(&metrosendtoaudio);
	file->ReadChunk(&metrosendtoMIDI);
	file->ReadChunk(&freezetype);

	tmp_frozen=frozen;
	frozen=false;

	file->ReadChunk(&trackfreezeendposition);

	file->CloseReadChunk();

	io.Load(file);
	MIDIfx.Load(file);

	// Group Pointer ?
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_TRACKGROUPS)
	{
		file->ChunkFound();

		int nrg=0;
		file->ReadChunk(&nrg);

		file->AddPointer((CPOINTER)&t_groups.activegroup);

		while(nrg--)
		{
			if(Seq_Group_GroupPointer *sgp=new Seq_Group_GroupPointer)
			{
				file->AddPointer((CPOINTER)&sgp->group);
				t_groups.groups.AddEndO(sgp);
			}
		}

		file->CloseReadChunk();
	}

	// MIDI Output Groups
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_TRACKMIDIOutputDeviceS)
	{
		file->ChunkFound();
		int nrg=0;
		file->ReadChunk(&nrg);

		while(nrg--)
		{
			if(Seq_Group_MIDIOutPointer *sgp=new Seq_Group_MIDIOutPointer)
			{
				file->ReadChunk(&sgp->portindex);		
				GetMIDIOut()->groups.AddEndO(sgp);
			}
		}

		file->CloseReadChunk();
	}

	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_TRACKMIDIINPUTDEVICES)
	{
		file->ChunkFound();

		int nrg=0;
		file->ReadChunk(&nrg);

		while(nrg--)
		{
			if(Seq_Group_MIDIInPointer *sgp=new Seq_Group_MIDIInPointer)
			{
				file->ReadChunk(&sgp->portindex);		
				GetMIDIIn()->groups.AddEndO(sgp);
			}
		}

		file->CloseReadChunk();
	}

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_TRACKAUDIOOUTPUTCHANNELS) // Track -> Bus
	{
		file->ChunkFound();

		int nrg=0;
		file->ReadChunk(&nrg);

		file->AddPointer((CPOINTER)&t_audiochannelouts.defaultchannel);
		file->AddPointer((CPOINTER)&t_audiochannelouts.defaultrecordchannel);

		int h=nrg;

		if(h)
		{
			while(h--)
			{
				if(Seq_AudioIOPointer *sgp=new Seq_AudioIOPointer)
				{
					file->AddPointer((CPOINTER)&sgp->channel);
					t_audiochannelouts.busgroups.AddEndO(sgp);
				}
			}

			t_audiochannelouts.busgroups.Close(); // Index
		}

		h=nrg;
		Seq_AudioIOPointer *c=t_audiochannelouts.FirstChannel();
		while(c && h--){
			file->ReadChunk(&c->bypass);
			c=c->NextChannel();
		}
		file->CloseReadChunk();
	}

	// FX
	t_trackeffects.Load(file);

	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_TRACKPOINTER)
	{
		file->ChunkFound();

		// Audio
		file->AddPointer((CPOINTER)&indevice);

		for(int i=0;i<5;i++)
		file->ReadChunk(&recordsettings_record[i]);

		file->CloseReadChunk();
	}
#ifdef _DEBUG
	else
		MessageBox(NULL,"Tracks IO not found","Error",MB_OK);
#endif

	file->LoadChunk();

	// MIDI Pattern
	if(file->GetChunkHeader()==CHUNK_TRACKMIDIPattern)
	{
		file->ChunkFound();
		int pattern=0;
		file->ReadChunk(&pattern);

		TRACE ("Load Pattern %d\n",pattern);

		file->CloseReadChunk();

		while(pattern--){
			file->LoadChunk();

			if(file->CheckReadChunk()==false)break;

			if(file->GetChunkHeader()==CHUNK_MIDIPattern)
			{
				file->ChunkFound();

				// Save MIDI Pattern
				if(MIDIPattern *mp=new MIDIPattern)
				{
					mp->track=this;
					mp->Load(file);

					TRACE ("Load Pattern %s Start %d\n",mp->GetName(),mp->ostart);
					AddSortPattern(mp,mp->ostart);
				}
				else
					file->CloseReadChunk();
			}
			else
				file->JumpOverChunk();
		}
	}

	// Audio Pattern
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_TRACKAUDIOPATTERN)
	{
		file->ChunkFound();

		int apattern=0;
		file->ReadChunk(&apattern);
		file->CloseReadChunk();

		while(apattern--){

			file->LoadChunk();

			if(file->CheckReadChunk()==false)
				break;

			if(file->GetChunkHeader()==CHUNK_AUDIOPATTERN)
			{
				file->ChunkFound();

				if(AudioPattern *ap=new AudioPattern)
				{
					ap->track=this;

					ap->Load(file);
					ap->Load_Ex(file,song);
					AddSortPattern(ap,ap->ostart);
				}
				else
					file->CloseReadChunk();
			}
			else
				file->JumpOverChunk();
		}
	}

	/*
	//refresh Clones
	file->RenewPointer(); // Clone <-> MainPattern

	// Add Clones to mainpattern
	Seq_Pattern *p=FirstPattern();

	while(p){

	if(p->mainclonepattern)
	p->mainclonepattern->AddClone(p);

	p=p->NextPattern();
	}
	*/

	LoadAutomationTracks(file,this,0);
	LoadProcessor(file);
	file->RenewPointer(); 

	//GenerateCrossFades();
}


void Seq_Track::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_TRACK);

	file->Save_Chunk((CPOINTER)this);

	parentindex=parent?song->GetOfTrack((Seq_Track *)parent):0;

	TRACE ("ChildDepth %d PIndex %d\n",childdepth,parentindex);
	file->Save_Chunk(parentindex);
	file->Save_Chunk(childdepth);

	// Name
	file->Save_ChunkString(trackname);
	file->Save_Chunk(recordtracktype);

	// Save Froozen Track Data
	file->Save_Chunk(frozen);
	file->Save_Chunk(frozenfilestart);

	char *frostyfile=0;
	if(frozenhdfile)
		frostyfile=frozenhdfile->GetName();

	file->Save_ChunkString(frostyfile);
	file->Save_Chunk(recordindex);

	t_colour.SaveChunk(file);

	file->Save_ChunkString(trackimage);
	file->Save_Chunk(MIDItype);
	file->Save_Chunk(usedirecttodevice);
	file->Save_Chunk(MIDItypesetauto);
	file->Save_Chunk(showchilds);
	file->Save_Chunk(sizefactor);

	file->Save_Chunk(record);
	file->Save_Chunk(outputisinput);
	file->Save_Chunk(vutype);

	file->Save_Chunk((CPOINTER)t_audiofx.recordtrack);

	file->Save_Chunk(tracktype);
	file->Save_Chunk(ismetrotrack);

	file->Save_Chunk(metrochl_b);
	file->Save_Chunk(metroport_b);
	file->Save_Chunk(metrokey_b);
	file->Save_Chunk(metrovelo_b);
	file->Save_Chunk(metrovelooff_b);

	file->Save_Chunk(metrochl_m);
	file->Save_Chunk(metroport_m);
	file->Save_Chunk(metrokey_m);
	file->Save_Chunk(metrovelo_m);
	file->Save_Chunk(metrovelooff_m);
	file->Save_Chunk(metrosendtoaudio);
	file->Save_Chunk(metrosendtoMIDI);
	file->Save_Chunk(freezetype);

	tmp_frozen=frozen;
	frozen=false;

	file->Save_Chunk(trackfreezeendposition);

	file->CloseChunk(); // Track

	io.Save(file);
	MIDIfx.Save(file);

	// Save Track Group Pointer
	if(t_groups.GetCountGroups()>0)
	{
		file->OpenChunk(CHUNK_TRACKGROUPS);
		file->Save_Chunk(t_groups.GetCountGroups());

		file->Save_Chunk((CPOINTER)t_groups.activegroup);

		// Save Groups Pointer
		Seq_Group_GroupPointer *sgp=t_groups.FirstGroup();
		while(sgp){
			file->Save_Chunk((CPOINTER)sgp->group);
			sgp=sgp->NextGroup();
		}

		file->CloseChunk();
	}

	// Save Track MIDI Output Pointer
	if(GetMIDIOut()->GetCountGroups()>0)
	{
		file->OpenChunk(CHUNK_TRACKMIDIOutputDeviceS);
		file->Save_Chunk(GetMIDIOut()->GetCountGroups());

		// Save Groups Pointer
		Seq_Group_MIDIOutPointer *sgp=GetMIDIOut()->FirstDevice();
		while(sgp){
			file->Save_Chunk(sgp->portindex);
			sgp=sgp->NextGroup();
		}

		file->CloseChunk();
	}

	// Save Track MIDI Input Pointer
	if(GetMIDIIn()->GetCountGroups()>0)
	{
		file->OpenChunk(CHUNK_TRACKMIDIINPUTDEVICES);
		file->Save_Chunk(GetMIDIIn()->GetCountGroups());

		// Save Groups Pointer
		Seq_Group_MIDIInPointer *sgp=GetMIDIIn()->FirstDevice();
		while(sgp){
			file->Save_Chunk(sgp->portindex);
			sgp=sgp->NextGroup();
		}

		file->CloseChunk();
	}

	if(t_audiochannelouts.FirstChannel()) // Track->Bus
	{
		file->OpenChunk(CHUNK_TRACKAUDIOOUTPUTCHANNELS);
		file->Save_Chunk(t_audiochannelouts.GetCountGroups());

		file->Save_Chunk((CPOINTER)t_audiochannelouts.defaultchannel);
		file->Save_Chunk((CPOINTER)t_audiochannelouts.defaultrecordchannel);

		// 1. Save Groups Pointer
		Seq_AudioIOPointer *sgp=t_audiochannelouts.FirstChannel();
		while(sgp){
			file->Save_Chunk((CPOINTER)sgp->channel);
			sgp=sgp->NextChannel();
		}

		// 2. Save Groups Bypass
		sgp=t_audiochannelouts.FirstChannel();
		while(sgp){
			file->Save_Chunk(sgp->bypass);
			sgp=sgp->NextChannel();
		}

		file->CloseChunk();
	}

	// Track Effects
	t_trackeffects.Save(file);

	// Track I/O	
	file->OpenChunk(CHUNK_TRACKPOINTER);

	// MIDI
	file->Save_Chunk((CPOINTER)indevice);

	for(int i=0;i<5;i++)
		file->Save_Chunk(recordsettings_record[i]);

	file->CloseChunk();

	if(song->saveonlyarrangement==false)
	{
		// MIDI Pattern
		if(int mpattern=GetCountOfPattern_NotFrozen(MEDIATYPE_MIDI|MEDIATYPE_NOLOOPS))
		{
			file->OpenChunk(CHUNK_TRACKMIDIPattern);
			file->Save_Chunk(mpattern);
			file->CloseChunk();

			// Pattern
			Seq_Pattern *p=FirstPattern_NotFrozen(MEDIATYPE_MIDI|MEDIATYPE_NOLOOPS);

			while(p){
				p->Save(file);
				p=p->NextPattern(MEDIATYPE_MIDI|MEDIATYPE_NOLOOPS);
			}
		}

		// Audio Pattern
		if(int apattern=GetCountOfPattern_NotFrozen(MEDIATYPE_AUDIO|MEDIATYPE_NOLOOPS))
		{
			file->OpenChunk(CHUNK_TRACKAUDIOPATTERN);
			file->Save_Chunk(apattern);
			file->CloseChunk();

			// Pattern
			Seq_Pattern *p=FirstPattern_NotFrozen(MEDIATYPE_AUDIO|MEDIATYPE_NOLOOPS);

			while(p){

				bool save=true;

				switch(p->mediatype)
				{
				case MEDIATYPE_AUDIO:
					{
						// Check If Audio Pattern is Audio Work File...
						AudioPattern *ap=(AudioPattern *)p;

						if(ap->waitforresample==true)
							save=false;
					}
					break;

				}

				if(save==true)
				{
					p->Save(file);
					p->Save_Ex(file);
				}

				p=p->NextPattern(MEDIATYPE_AUDIO|MEDIATYPE_NOLOOPS);
			}
		}
	}// Arrangement only ?

	SaveAutomationTracks(file);
	SaveProcessor(file);

	frozen=tmp_frozen;
}

void Seq_Track::LoadProcessor(camxFile *file)
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_TRACKPROCESSOR)
	{
		file->ChunkFound();
		file->CloseReadChunk();

		file->LoadChunk();
		if(file->GetChunkHeader()==CHUNK_PROCESSOR)
		{
			file->ChunkFound();

			if(t_processor=new Processor)
			{
				t_processor->track=this;
				t_processor->Load(file);
			}
			else
				file->CloseReadChunk();
		}
	}
}

void Seq_Track::SaveProcessor(camxFile *file)
{
	if(t_processor)
	{
		file->OpenChunk(CHUNK_TRACKPROCESSOR);
		file->CloseChunk();

		t_processor->Save(file);
	}
}


void Seq_Track::DeleteFrozenData()
{
	DeleteFrozenPattern();

	if(frozenhdfile)
	{
		frozenhdfile->FreeMemory();
		delete frozenhdfile;
		frozenhdfile=0;
	}
}

void Seq_Track::Delete(bool full)
{
	//TRACE ("Delete Song %s Track %s Full %d Lock %d\n",song->songname,GetName(),full,lock);

	if(song)
	{
	//	audiofreezethread->StopFreezing(this,true);
		DeleteFrozenData();

		if(frozenfile)
		{
			delete frozenfile;
			frozenfile=0;
		}

		frozen=false;
	}

	// Delete Pattern
	Seq_Pattern *pattern=FirstPattern(MEDIATYPE_ALL);

	while(pattern){
		//TRACE ("Delete Pattern %s \n",pattern->GetName());
		pattern=DeletePattern(pattern,true);
	}

	// Childs have empty List...
	t_groups.Delete();

	t_MIDIinputdevices.Delete();
	t_MIDIoutdevices.Delete();
	t_audiochannelouts.Delete();
	//	t_audiochannelins.Delete();

	// Audio FX
	t_audiofx.DeleteTrackRecord();
	t_audiofx.DeleteTrackMix();

	io.FreeMemory();

	// Free SubTracks
	DeleteAllAutomationTracks();

	// Free Processor
	if(t_processor)
	{
		t_processor->Delete(true);
		t_processor=0;
	}

	if(trackimage)
		delete trackimage;

	if(trackname)
		delete trackname;

	GetPeak()->FreeMemory();

	delete this;
}

void Seq_Track::CloneFx(Seq_Track *to)
{
	to->sizefactor=sizefactor;
	to->MIDItype=MIDItype;
	to->MIDItypesetauto=MIDItypesetauto;
	to->recordtracktype=recordtracktype;
	to->vutype=vutype;
//	to->outputisinput=outputisinput;

	GetColour()->Clone(&to->t_colour);

	GetFX()->Clone(&to->t_trackeffects);

	to->usedirecttodevice=usedirecttodevice;

	// Audio Inpu
	io.Clone(&to->io);
	GetAudioOut()->CloneToGroup(&to->t_audiochannelouts);

	//	if(t_audiofx.recordtrack) // Input Track ?
	//	to->AddTrackToRecord(t_audiofx.recordtrack);

	GetMIDIIn()->CloneToGroup(&to->t_MIDIinputdevices);
	GetMIDIOut()->CloneToGroup(&to->t_MIDIoutdevices);

	if(trackimage)
	{
		if(to->trackimage)
			delete to->trackimage;

		to->trackimage=mainvar->GenerateString(trackimage);
		to->icon=icon;
	}
}

Seq_Track *Seq_Track::Clone(Seq_Song *tosong)
{
	if(Seq_Track *newtrack=new Seq_Track)
	{
		tosong->AddTrack(newtrack,-1); // add to end
		CloneToTrack(newtrack);
		return newtrack;
	}

	return 0;
}

Seq_Track *Seq_Track::Clone(Seq_Track *totrack)
{
	if(totrack)
	{
		CloneToTrack(totrack);
	}

	return totrack;
}

bool Seq_Track::SetFrozenHDFile(char *filename,int startposition)
{
	DeleteFrozenData(); // Delete old

	if(AudioHDFile *hdfile=new AudioHDFile)
	{
		hdfile->SetName(filename);

		hdfile->InitHDFile();

		if(hdfile->m_ok==true)
		{
			if(AudioPattern *ap=new AudioPattern)
			{
				// Create Frozen Pattern <Track>

				ap->realpattern=false;

				ap->SetName("Frozen Track");

				ap->audioevent.audioefile=hdfile;
				
				SetFrozenPattern(ap,startposition);

				frozenhdfile=hdfile;
				frozenfilestart=startposition;
				frozen=true;

				hdfile->CreatePeakFile(true);

				return true;
			}
			else
			{
				hdfile->FreeMemory();
				delete hdfile;
			}
		}
		else
		{
			hdfile->FreeMemory();
			delete hdfile;
		}
	}

	return false;
}

void Seq_Track::FreezeTrack(int flag)
{
	if(frozenfile)
	{
		bool ok=SetFrozenHDFile(frozenfile,0);

		if(ok==false) 
		{
			// File not found or error

			frozen=false;
			delete frozenfile;
			frozenfile=0;

			frozenpattern=0;
		}
	}
	else
	{
		frozen=false;
		frozenpattern=0;
	}
}

void Seq_Track::DeleteFrozenPattern()
{
	frozenpattern=0;
}

bool Seq_Track::UnFreezeTrack()
{
	//mainthreadcontrol->Lock(CS_audiofreeze);

	if(frozen==true || underfreeze==true)
	{
		mainthreadcontrol->LockActiveSong();

		if(frozenfile)
		{
			mainvar->DeletePeakFile(frozenfile);
			mainvar->DeleteAFile(frozenfile);
			delete frozenfile;
			frozenfile=0;
		}

		DeleteFrozenData();

		frozen=false;
		underfreeze=false;

		mainthreadcontrol->UnlockActiveSong();

		Seq_Pattern *p=FirstPattern(); // Mark Pattern as real

		while(p)
		{
			p->visible=true;

			if(p->itsaloop==false)
				switch(p->mediatype)
			{
				case MEDIATYPE_AUDIO:
					{
						AudioPattern *ap=(AudioPattern *)p;

						// Reload/Load Peak File
						if(ap->audioevent.audioefile)
						ap->audioevent.audioefile->CreatePeakFile(true);

					}
					break;
			}

			p=p->NextPattern();
		}

		// maingui->CheckUserMessage(0,MESSAGE_REFRESHFREEZETRACK,this,0); // Refresh Arrange etc..

		return true;
	}

//	mainthreadcontrol->Unlock(CS_audiofreeze);;

	return false;
}

void Seq_Track::DeleteAllRawEvents()
{
	rawpointer=0;

	Seq_Event *e=FirstRawEvent();

	while(e)
		e=DeleteRawEvent(e);
}

void Seq_Track::BufferRawEvent(MIDIPattern *p,Seq_Event *e)
{
	switch(e->GetStatus())
	{
	case INTERN:
		break;

	case INTERNCHAIN:
		{
			// Drums ?
			ICD_Object *icd=(ICD_Object *)e;

			switch(icd->type)
			{
			case ICD_TYPE_DRUM:
				{
					ICD_Drum *icddrum=(ICD_Drum *)icd;

					if(icddrum->drumtrack->mute==false)
					{
#ifdef MEMPOOLS
						if(Note *note=mainpools->mempGetNote())
#else
						if(Note *note=new Note)
#endif
						{
							note->pattern=p;
							note->status=NOTEON|icddrum->drumtrack->GetMIDIChannel();
							note->key=icddrum->drumtrack->key;

							int velo=icddrum->velocity+icddrum->drumtrack->volume;

							if(velo>127)
								velo=127;
							else
								if(velo<1)
									velo=1;

							note->velocity=(UBYTE)velo;

							if(NoteOff_Raw *raw=new NoteOff_Raw(NOTEOFF|icddrum->drumtrack->GetMIDIChannel(),icddrum->drumtrack->key,icddrum->drumtrack->velocityoff))
							{
								raw->pattern=p;
								events_raw.AddOSort(note,e->GetEventStart(p));
								events_raw.AddOSort(raw,e->GetEventStart(p)+icddrum->drumtrack->ticklength);
							}
							else
								delete note;
						}
					}
				}
				break;
			}// switch ICD Type
		}
		break;

	case NOTEON:
		{
			if(Seq_Event *notec=(Seq_Event *)e->Clone(song))
			{
				notec->pattern=p;

				Note *n=(Note *)notec;

				if(NoteOff_Raw *raw=new NoteOff_Raw(NOTEOFF|e->GetChannel(),n->key,n->velocityoff))
				{
					raw->pattern=p;
					raw->note=n;

					raw->notestartpos=n->GetEventStart(p);
					raw->staticostart=n->GetNoteEnd(p);

					events_raw.AddOSort(notec,raw->notestartpos); // Note On
					events_raw.AddOSort(raw,raw->staticostart); // Note Off

#ifdef _DEBUG
					if(raw->ostart-n->GetEventStart(p) != n->GetNoteLength())
						MessageBox(NULL,"Illegal Note Length","Error",MB_OK);
#endif
				}
				else
					delete notec;
			}
		}
		break;

	default:
		{
			Seq_Event *clone=(Seq_Event *)e->Clone(song);

			if(clone)
			{
				clone->pattern=p;
				events_raw.AddOSort(clone,e->GetEventStart(p));
			}
		}
		break;
	}// switch
}

Seq_Event *Seq_Track::DeleteRawEvent(Seq_Event *e)
{
	Seq_Event *n=e->NextEvent();
	events_raw.CutQObject(e);
	e->Delete(true);
	return n;
}

void Seq_Track::CreateRawEvents(Seq_Pattern *singlepattern)
{
	DeleteAllRawEvents();

	Seq_Pattern *p;

	if(!singlepattern)
	{
		// Add Track Bank Select/Program
		char channel=-1;

		if(GetFX()->GetChannel())
			channel=(UBYTE)GetFX()->GetChannel()-1;

		if(channel!=-1)
		{
#ifdef OLDIE
			//if(GetFX()->MIDIprogram.on==true)
			{

				// Write BankSelect/ProgramChange
				if(GetFX()->MIDIprogram.usebank==true)
				{
#ifdef MEMPOOLS
					ControlChange *cc=mainpools->mempGetControl();
#else
					ControlChange *cc=new ControlChange;
#endif

					if(cc)
					{
						cc->status=CONTROLCHANGE|channel;
						cc->controller=0;
						cc->value=(UBYTE)GetFX()->MIDIprogram.MIDIBank;
						events_raw.AddOSort(cc,0);
					}
				}

				// Program Change
				{
					ProgramChange *pc=new ProgramChange;

					if(pc)
					{
						pc->status=PROGRAMCHANGE|channel;
						pc->program=GetFX()->MIDIprogram.MIDIProgram;
						events_raw.AddOSort(pc,0);
					}
				}
			}
#endif

		}

		// Track Start Program
		p=FirstPattern(MEDIATYPE_MIDI);
	}
	else
		p=singlepattern;

	while(p && p->mediatype==MEDIATYPE_MIDI)
	{
		if(p->mute==false)
		{
			MIDIPattern *mp=(MIDIPattern *)p;

			// Add Bank Select/Program
			char channel=-1;

			if(mp->GetMIDIFX()->GetChannel())
			{
				channel=(UBYTE)mp->GetMIDIFX()->GetChannel()-1;
			}
			else
			{
				if(GetFX()->GetChannel())
					channel=(UBYTE)GetFX()->GetChannel()-1;
			}

			if(channel!=-1)
			{
				//if(mp->MIDIprogram.on==true)
				{
#ifdef OLDIE
					// Write BankSelect/ProgramChange
					if(mp->MIDIprogram.usebank==true)
					{
#ifdef MEMPOOLS
						ControlChange *cc=mainpools->mempGetControl();
#else
						ControlChange *cc=new ControlChange;
#endif

						if(cc)
						{
							cc->status=CONTROLCHANGE|channel;
							cc->controller=0;
							cc->value=(UBYTE)mp->MIDIprogram.MIDIBank;
							cc->pattern=0;  // Set No Module Change

							events_raw.AddOSort(cc,mp->GetPatternStart());
						}
					}
#endif

					// Program Change
					{
						ProgramChange *pc=new ProgramChange;

						if(pc)
						{
							pc->status=PROGRAMCHANGE|channel;
							pc->program=mp->MIDIprogram.MIDIProgram;
							pc->pattern=0; // Set No Module Change

							events_raw.AddOSort(pc,mp->GetPatternStart());
						}
					}
				}
			}

			// MIDI Output Processor
			MIDIProcessor processor(song,this);
			processor.createraw=true;

			Seq_Event *e=mp->FirstEvent();

			while(e)
			{
				processor.EventInput(mp,e,e->GetEventStart()); // ->> fill+create Processor Events

				// 1. Check pattern events
				Seq_Event *oevent=processor.FirstProcessorEvent();

				while(oevent)
				{
					if(oevent->CheckIfPlaybackIsAble()==false || // Check only Note == MUTE ?
						p->CheckIfPlaybackIsAble()==true
						)	
						BufferRawEvent(mp,oevent);

					oevent=processor.DeleteEvent(oevent);	
				}// while proc events

				e=e->NextEvent();
			}
		}//mute ?

		if(!singlepattern)
			p=p->NextPattern(MEDIATYPE_MIDI);
		else
			p=0;

	}// while pattern

	// Check processor/module rawevents
	// 2. Raw Events -> Module+Static FX

	{
		OListStart bufferlist;

		MIDIProcessor processor(song,this);
		processor.createraw=true;

		Seq_Event *re=FirstRawEvent();

		while(re)
		{
			processor.EventInput_RAW(re->GetMIDIPattern(),re); // Add Processor+Static FX

			Seq_Event *oevent=processor.FirstProcessorEvent();

			while(oevent)
			{
				if(Seq_Event *clone=(Seq_Event *)oevent->Clone(song))
				{
					clone->pattern=oevent->pattern;
					bufferlist.AddEndS(clone,clone->ostart);
				}

				oevent=processor.DeleteEvent(oevent);	
			}// while proc events

			re=re->NextEvent();

			// Alarm Check
			Proc_Alarm *rawalarm=MIDIalarmprocessorproc->FirstRAWAlarm();

			while(rawalarm && ((!re) || rawalarm->songposition<re->ostart))
			{
				rawalarm->module->Alarm(rawalarm);
				rawalarm=MIDIalarmprocessorproc->DeleteRAWAlarm(rawalarm);
			}
		}

		DeleteAllRawEvents();

		// Rest Alarm
		{
			Proc_Alarm *restalarm=MIDIalarmprocessorproc->FirstRAWAlarm();

			while(restalarm)
			{
				restalarm->module->Alarm(restalarm);
				restalarm=MIDIalarmprocessorproc->DeleteRAWAlarm(restalarm);
			}
		}

		//1. Copy List 
		bufferlist.MoveListToList(&events_raw);
		// 2. Mix Created RawList
		Seq_Event *me=mainprocessor->FirstRAWEvent();

		while(me)
			me=mainprocessor->DeleteRAWEvent(me,&events_raw);
	}

	rawpointer=FirstRawEvent();
}

bool Seq_Track::IsEditAble()
{
	if(frozen==true)
		return false;

	return true;
}

void Seq_Track::ToggleMute()
{
	bool muteflag=GetMute()==true?false:true;
	OSTART automationtime=song->GetSongPosition();

	if(ismetrotrack==true)
	{
		song->MuteTrack(this,muteflag);
	}
	else
	{
		Seq_Track *t=song->FirstTrack();

		while(t){

			if(t->IsPartOfEditing(this)==true)
				song->MuteTrack(t,muteflag,automationtime);

			t=t->NextTrack();
		}
	}
}

void Seq_Track::ToggleRecord()
{
	if(ismetrotrack==true || frozen==true)
		return;

	//if(song->IsRecording()==true)
	//	return;

	bool togglerecord=record==true?false:true;

	OSTART position=song->GetSongPosition();
	Seq_Track *t=song->FirstTrack();

	while(t){

		if(t->IsPartOfEditing(this,true)==true)
			t->SetRecordMode(togglerecord,position);

		t=t->NextTrack();
	}

	song->SetMIDIRecordingFlag();
}

AudioPattern *Seq_Track::InitAudioRecordingPattern(AudioDevice *device,OSTART position,int cycleloop,LONGLONG writeoffset,bool setpunch2flag)
{
	AudioPattern *newaudiopattern=0;
	int newpattern=0;

	if(record==true && recordtracktype==TRACKTYPE_AUDIO && song->directoryname)
	{
		//AudioRecordBuffer *rb=0;
		//LONGLONG samplestart;

		if(device->status&AudioDevice::STATUS_INPUTOK)
		{
			AudioPort *port=GetVIn();
			if(!port)return 0;

			AudioHardwareChannel *hw=port->hwchannel[0];
			if(!hw)return 0;

			//	samplestart=hw->hwrecordbuffers[device->recordbuffer_readindex].bufferinfo.sampleposition[0];
		}
		else
		{
			//	samplestart=t_audiofx.inputbuffers[song->inputbuffersrecc].bufferinfo.sampleposition[0];

			//	TRACE ("inputbuffersrecc %d\n",song->inputbuffersrecc);

			//	TRACE ("InitAudioRecordingPattern %d\n",samplestart);
		}

		//OSTART position;

		// Check Punch
	//	bool setpunch2flag=false;

		TRACE ("######################### Init Record Pattern ######################### Track: %s\n",GetName());

		// Create Record File Name
		size_t i=strlen(song->directoryname); // Directoryname

		i+=2; // /+ 0
		char *s=GetName();
		i+=strlen(s);

#ifdef WIN32
		SYSTEMTIME systime;

		// CHAR sh[255];

		GetSystemTime(&systime);

		/*
		GetTimeFormat(
		LOCALE_CAMX_DEFAULT,				
		LOCALE_USE_CP_ACP,
		&systime,
		NULL,
		&sh[0],
		254);

		int systemtime=GetTickCount(); // ms
		*/

		char nrsstd[NUMBERSTRINGLEN],nrsmin[NUMBERSTRINGLEN],nrssec[NUMBERSTRINGLEN],nrindex[NUMBERSTRINGLEN],nrrecindex[NUMBERSTRINGLEN];

		//char index[32];

		char *timestrstd=mainvar->ConvertIntToChar(systime.wHour,nrsstd),
			*timestrmin=mainvar->ConvertIntToChar(systime.wMinute,nrsmin),
			*timestrsec=mainvar->ConvertIntToChar(systime.wSecond,nrssec),
			*timestrrecindex=mainvar->GenerateString("t",mainvar->ConvertIntToChar(index+1,nrindex),"i",mainvar->ConvertIntToChar(recordindex,nrrecindex));

		// char *indexstr=mainvar->ConvertIntToChar(mainaudio->GetCountOfRecordingFiles()+1,index);

		i+=strlen(timestrstd);
		i+=strlen(timestrmin);
		i+=strlen(timestrsec);
		i+=strlen(timestrrecindex);
#endif
		i+=strlen(".wav");

		char *newfilename=new char[i+16];

		if(newfilename)
		{
			// Dir name
			i=strlen(song->directoryname);

			strcpy(newfilename,song->directoryname);
			newfilename[i]='\\';
			i++;

			// Track Name ( Filter ASCII)
			char *to=&newfilename[i],*from=s;
			size_t sl=strlen(s);

			for(size_t i2=0;i2<sl;i2++)
			{
				if( (*from>=48 && *from<=57) ||
					(*from>=65 && *from<=90) ||
					(*from>=97 && *from<=122)
					)
				{
					*to++=*from;
					i++;
				}

				from++;
			}

			// strcpy(&newfilename[i],s);
			// i+=strlen(s);

			// Hour
			newfilename[i++]='H';
			strcpy(&newfilename[i],timestrstd);
			i+=strlen(timestrstd);

			// Min
			newfilename[i++]='M';
			strcpy(&newfilename[i],timestrmin);
			i+=strlen(timestrmin);

			// Sek
			newfilename[i++]='S';
			strcpy(&newfilename[i],timestrsec);
			i+=strlen(timestrsec);

			// Track Record Index
			strcpy(&newfilename[i],timestrrecindex);
			i+=strlen(timestrrecindex);

			size_t c=i;

			if(setpunch2flag==true)
				strcpy(&newfilename[c],"_p2.wav");
			else
				strcpy(&newfilename[c],".wav");

			if(AudioHDFile *recordfile=mainaudio->AddAudioRecordingFile(this,newfilename)) // Init Write File ?
			{
				recordfile->writeoffset=writeoffset;

				//AudioHDFile *recordfile2=0;
				if(AudioPattern *rec=(AudioPattern *)mainedit->CreateNewPattern(0,this,MEDIATYPE_AUDIO_RECORD,position,false,CNP_NOGUIREFRESH|CNP_NOCHECKPLAYBACK|CNP_NOEDITOKCHECK)) // Norm, Punch In and Out1
				{
					newaudiopattern=rec;

					rec->recordpattern=true;
					//recordfile->recordstart_sampleposition=samplestart;

					rec->audioevent.audioefile=recordfile; // Connect RecFile <> Pattern

					recordfile->reccycleloopcounter=cycleloop; // Init Cycle Loop Counter
					recordfile->recordpattern=rec;
					//recordfile->flag=setpunch2flag==true?HDFLAG_PUNCH2:HDFLAG_PUNCH1; // or norm
					recordfile->istrackrecordingfile=t_audiofx.recordtrack?true:false;

					// OpenFlag
					int writeflag=0;
					recordfile->InitAudioFileSave(this,writeflag);

					recordfile->recstarted=true;
					newpattern++;

					if(rec->audioevent.audioefile->m_ok==true){

						if(setpunch2flag==true)
							audiorecord_audiopattern[1]=rec; // Punch 2
						else
						{
							audiorecord_audiopattern[0]=rec; // Punch 1
						}

						recordindex++;
						//mainaudio->AddAudioPatternToRecord(position,rec1);
					}
				}
			}

			delete newfilename;

			if(timestrrecindex)
				delete timestrrecindex;
		}
	}

	return newaudiopattern;
}

void Seq_Track::ShowAllChildTracks()
{
	bool changed=true;

	if(showchilds==false && FirstChildTrack())
	{
		changed=showchilds=true;
	}

	Seq_Track *nt=NextTrack();

	while(nt && nt->IsTrackChildOfTrack(this)==true)
	{
		if(nt->showchilds==false && nt->FirstChildTrack())
		{
			changed=nt->showchilds=true;
		}

		nt=nt->NextTrack();
	}

	maingui->ShowToggleChildTracks(song);
}

void Seq_Track::ToggleShowChild(bool leftmouse)
{
	if(ismetrotrack==true)
		return;

	showchilds=showchilds==true?false:true;

	if(leftmouse==false) // All
	{
		Seq_Track *t=song->FirstTrack();
		while(t)
		{
			t->showchilds=showchilds;
			t=t->NextTrack();
		}
	}

	maingui->ShowToggleChildTracks(song);
}

void Seq_Track::CloneToTrack(Seq_Track *t,bool withpattern)
{
	// Clone Object ID
	t->MIDItype=MIDItype;
	t->recordtracktype=recordtracktype;
	t->id=id;
	t->vutype=vutype;

	t->SetName(GetName());

	//Clone Data
	CloneFx(t);

	t->activeautomationtrack=activeautomationtrack;
	t->indevice=indevice;

	t->SetRecordMode(record,songstartposition);
	t->SetInputMonitoring(io.inputmonitoring);

	t->recordbeforestart=recordbeforestart;
	t->SetSolo(GetSolo());
	t->showautomationstracks=showautomationstracks;
	t->showchilds=showchilds;

	t_groups.CloneToGroup(&t->t_groups);

	if(withpattern==true)
	{
		//Clone Pattern
		Seq_Pattern *p=FirstPattern(MEDIATYPE_ALL);

		while(p)
		{
			if(Seq_Pattern *clone=p->CreateClone(0,0))
			{
				t->AddSortPattern(clone,p->GetPatternStart());
				clone->mute=false;
			}

			p=p->NextPattern(MEDIATYPE_ALL);
		}
	}
}

char *Seq_Track::GetName()
{
	if(trackname)return trackname;
	return "_";
}

void Seq_Track::SetName(char *newstring)
{
	if(trackname)delete trackname;
	trackname=0;

	if(newstring)
		trackname=mainvar->ClearString(newstring);
}

void Seq_Track::InitAudioIO(Seq_Song *song,int channels,int inputchannels)
{
	switch(channels)
	{
	case 1:
		SetAudioIOType(CHANNELTYPE_MONO);
		break;

	case 2:
		SetAudioIOType(CHANNELTYPE_STEREO);
		break;
	}

	SetRecordTrackType(TRACKTYPE_AUDIO);

	// Init Audio In

	if(song->audiosystem.device)
		io.SetInput(song->audiosystem.device->FindVInChannel(inputchannels!=-1?inputchannels:channels,0));

	io.inputpeaks.InitPeak(inputchannels!=-1?inputchannels:channels);

	// Init Audio Out
	if(song->audiosystem.device)
		io.SetOutput(song->audiosystem.device->FindVOutChannel(channels,0));

	GetPeak()->InitPeak(channels);
}

void Seq_Track::InitAudioIO(Seq_Song *song,AudioFileInfo *ainfo)
{
	if(!ainfo)return;

	InitAudioIO(song,ainfo->channels);
}

void Seq_Track::InitAudioIO(Seq_Song *,AudioHDFile *hdfile)
{
	if(!hdfile)return;

	InitAudioIO(song,hdfile->channels);
}

void Seq_Track::InitAudioIOWithNewAudioFile(Seq_Song *song,AudioHDFile *hdfile)
{
	InitAudioIO(song,channelschannelsnumber[song->pref_tracktype],hdfile->channels);
}

void Seq_Track::ShowTrackName(guiWindow *nostringrefresh)
{
	guiWindow *win=maingui->FirstWindow();

	while(win)
	{	
		if(win!=nostringrefresh)
			switch(win->GetEditorID())
		{
			case EDITORTYPE_PLUGIN_INTERN:
				{
					Edit_Plugin_Intern *edintern=(Edit_Plugin_Intern *)win;

					if(edintern->insertaudioeffect && edintern->insertaudioeffect->effectlist->track==this)
						edintern->SetName(true);
				}
				break;

			case EDITORTYPE_PLUGIN_VSTEDITOR:
				{
					Edit_Plugin_VST *edvst=(Edit_Plugin_VST *)win;

					if(edvst->insertaudioeffect && edvst->insertaudioeffect->effectlist->track==this)
					{
						edvst->SetName(true);
					}
				}
				break;

			case EDITORTYPE_MIDIFILTER:
				{
					Edit_MIDIFilter *em=(Edit_MIDIFilter *)win;

					if(em->filter==&GetFX()->inputfilter || em->filter==&GetFX()->filter)
						em->SetInfo(GetName());
				}
				break;

			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					// Refresh Pattern List
					EventEditor_Selection *e=(EventEditor_Selection *)win;

					Seq_SelectedPattern *sp=e->patternselection.FirstSelectedPattern();

					while(sp)
					{
						if(sp->pattern->track==this)
						{
							e->ShowSelectionPattern();
							break;
						}

						sp=sp->NextSelectedPattern();
					}
				}
				break;

			case EDITORTYPE_SETTINGS:
				{
					Edit_Settings *set=(Edit_Settings *)win;

					if(set->WindowSong()==this->song && set->settingsselection==Edit_Settings::SONG_ROUTING)
					{
						set->ShowSettingsData(0);
					}
				}
				break;

			case EDITORTYPE_QUANTIZEEDITOR:
				{
					Edit_QuantizeEditor *ed=(Edit_QuantizeEditor *)win;

					if(ed->effect->track==this)
						ed->ShowTitle();
				}
				break;

		}

		win=win->NextWindow();
	}
}

void Seq_Track::ResetMIDIRecordPattern()
{
	if(record_MIDIPattern)
	{
		record_MIDIPattern->recordpattern=false;
		record_MIDIPattern->recordeventsadded=false;

		Seq_Event *e=record_MIDIPattern->FirstEvent();
		while(e)
		{
			e->flag CLEARBIT EVENTFLAG_RECORDED;
			e=e->NextEvent();
		}

		record_MIDIPattern->CloseEvents();
		record_MIDIPattern=0;
	}	

}

void Seq_Track::ResetAudioRecordPattern()
{
	Seq_Pattern *p=FirstPattern();

	while(p)
	{
		if(p->mediatype==MEDIATYPE_AUDIO_RECORD || p->mediatype==MEDIATYPE_AUDIO)
		{
			AudioPattern *ap=(AudioPattern *)p;

			if(ap->recordpattern==true)
			{
				ap->SetAudioMediaTypeAfterRecording();

			}
		}

		p=p->NextRealPattern();
	}

	for(int i=0;i<MAXRECPATTERNPERTRACK;i++)
		audiorecord_audiopattern[i]=0;
}


void Seq_Track::AddEffectsToPattern(Seq_Pattern *p)
{
	if(p->mainpattern==0)
	{
		// only Real Pattern !
		p->QuantizePattern(&GetFX()->quantizeeffect);
	}
}

void Seq_Track::DoTrackInputFromTrack()
{
	if(!t_audiofx.t_inputbuffers)
		return;

	if(!t_audiofx.recordtrack)
		return;

	t_audiofx.recordtrack->mix.CopyAudioBuffer(&t_audiofx.t_inputbuffers[t_audiofx.t_lastfilledbufferindex]);
//	t_audiofx.t_inputbuffers[t_audiofx.t_lastfilledbufferindex].hwbflag=AudioHardwareBuffer::AHB_DONTCLEAR;
}

void Seq_Track::DoAudioInput(AudioDevice *device)
{
	// Copy Hardware -> Track + Input Peak

	if(io.in_vchannel && io.audioinputenable==true)
	{
		io.in_vchannel->MixVInputToBuffer(&t_audiofx.t_inputbuffers[t_audiofx.t_lastfilledbufferindex],io.GetChannels(),&io.inputpeaks); // 1. Get Audio Data from Device
	}
}

void Seq_Track::DoAudioInputEffects(AudioDevice *device)
{
	if(!t_audiofx.t_inputbuffers)
		return;

	// TRACE ("DoAudioInputAndEffects %s %d\n",this->trackname,t_audiofx.t_useeffectindex);

	input_par.startin=input_par.in=&t_audiofx.t_inputbuffers[t_audiofx.t_lastfilledbufferindex];

	input_par.streamchannels=io.GetChannels();
	input_par.bypassfx=io.bypassallinputfx;
	input_par.song=song;

	io.audioinputeffects.AddEffects(&input_par); // 2. Add Input Effects
}

void  Seq_Track::DoAudioInputPeak()
{
	t_audiofx.t_inputbuffers[t_audiofx.t_lastfilledbufferindex].CreatePeakTo(&io.inputpeaks);

	//hwb->CreatePeakTo(&t->io.inputpeaks);
}

void Seq_Track::DoThru(AudioDevice *device)
{
	AudioHardwareBuffer *hwb=&t_audiofx.t_inputbuffers[t_audiofx.t_lastfilledbufferindex];

	if(hwb->channelsused) // 3. Mix to Thru Buffer
	{
		/*
		if(t_audiofx.t_lastfilledbufferthruindex==t_audiofx.inputbufferscounter-1)
		t_audiofx.t_lastfilledbufferthruindex=0;
		else
		t_audiofx.t_lastfilledbufferthruindex++;
		*/

		//	io.skipthru=true;
		hwb->MixAudioBuffer(&mix); // mix Effect Outbuffer->Track Out
	}

	//	if((device->status&AudioDevice::STATUS_INPUTOK) && t_audiofx.recordtrack==0)
	//	io.in_vchannel->MixVInputToBuffer(&t_audiofx.mix,device->inbufferreadc,io.GetChannels());
}

void TrackHead::InitAutomationTracks(InitPlay *init)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at){
		at->InitAutomationTrackPlayback(init,true);
		at=at->NextAutomationTrack();
	}
}

void Seq_Pattern::InitStartPositions(OSTART *ps,OSTART *pe)
{
	*ps=GetPatternStart();
	*pe=GetPatternEnd();

	//Add Delay
	track->AddDelay(ps,pe);
}

bool Seq_Track::InitPlayback(InitPlay *init)
{
	OSTART initposition=init->position;

	int oldmode=init->mode;

	for(int mode=0;mode<2;mode++)
	{
		if(init->mode&MEDIATYPE_MIDI)
		{
			// MIDI Events + Chaín Notes
			playback_MIDIPattern[init->initindex][mode]=playback_chainnoteMIDIPattern[init->initindex][mode]=0;

			// Init MIDI Pattern
			Seq_Pattern *p=FirstPattern(MEDIATYPE_MIDI);

			while(p)
			{
				// Reset Bank/Prog Change Out
				((MIDIPattern *)p)->GetMIDIFX()->MIDIbank_progselectsend=false;

				OSTART ps,pe;
				p->InitStartPositions(&ps,&pe);

				if( (ps<=init->position && pe>=init->position) || ps>=init->position)// <=
				{
					init->delay=GetFX()->GetDelay();

					// Reset Start Event	
					((MIDIPattern *)p)->playback_MIDIevent[init->initindex][mode]=((MIDIPattern *)p)->playback_nextchainevent[init->initindex][mode]=0;
					
					if(p->InitPlayback(init,mode)==true){

						MIDIPattern *mp=(MIDIPattern *)p;
						if(mp->playback_MIDIevent[init->initindex]) // in use ?
						{
							if(!playback_MIDIPattern[init->initindex][mode])
								playback_MIDIPattern[init->initindex][mode]=playback_chainnoteMIDIPattern[init->initindex][mode]=mp; // MIDI Start Pattern found |---->
						}
					}
				}

				p=p->NextPattern(MEDIATYPE_MIDI);
			}

			// End MIDI
		}

		if(init->mode&MEDIATYPE_AUDIO)
		{
			// Audio
			int audiomode=init->mode&(MEDIATYPE_AUDIO|MEDIATYPE_AUDIO_RECORD);

			playback_audiopattern[init->initindex]=0;

			// Init Audio Pattern
			Seq_Pattern *p=FirstPattern(audiomode);

			while(p)
			{
				OSTART ps,pe;
				p->InitStartPositions(&ps,&pe);

				if( (ps<=init->position && pe>=init->position) || ps>=init->position)// <=
				{
					if(p->InitPlayback(init,mode)==true){

						if(((AudioPattern *)p)->runningfile==0)// Already started
						{
							if(!playback_audiopattern[init->initindex])
							{
								AudioPattern *ap=(AudioPattern *)p;
								//	if(!song->audiosystem.FindRunningAudioFile(ap))
								playback_audiopattern[init->initindex]=ap; // PRepair future audio pattern
							}
						}
					}

				}// if

				p=p->NextPattern(audiomode);
			}

			// End Audio
		}

		// mode== cycle
		init->mode CLEARBIT MEDIATYPE_AUDIO; // No Audio,MIDI (Cycling) only
		init->position=song->playbacksettings.cyclestart;
	}

	init->mode=oldmode;
	init->position=initposition;

	return true; // track init ok
}

bool Seq_TrackFX::CheckAudioOutputString(char *check,char *added)
{
	TAudioOut tao;
	Seq_AudioIO ao;

	tao.audiochannelouts=&ao;
	tao.simple=true;

	track->ShowAudioOutput(&tao);

	if(added)
		check+=strlen(added);

	char *c=tao.returnstring;

	if(c==0 && check==0)
		return true;

	if( ((!check) && c) ||
		((!c) && check) ||
		strcmp(c,check)!=0
		)
	{
		return false;
	}

	return true;
}

bool Seq_TrackFX::CheckAudioInputString(char *check)
{
	char *c=GetAudioInputString();

	if(c==0 && check==0)
		return true;

	if( ((!check) && c) ||
		((!c) && check) ||
		strcmp(c,check)!=0
		)
	{
		delete c;
		return false;
	}

	delete c;
	return true;
}

char *Seq_TrackFX::GetAudioInputString()
{
	if(recordtrack)
	{
		char h[NUMBERSTRINGLEN];
		return mainvar->GenerateString("[T ",mainvar->ConvertIntToChar(recordtrack->GetTrackIndex()+1,h),"]:",recordtrack->GetName());
	}

	if(track->io.audioinputenable==false)
		return mainvar->GenerateString(Cxs[CXS_NOINPUTHARDWARE]);

	if(track->io.in_vchannel)
		return mainvar->GenerateString(track->io.in_vchannel->name);

	return mainvar->GenerateString("?");
}

void Seq_TrackFX::Repair()
{
	if(recordtrack)
	{
		if(Seq_TrackRecord *tr=new Seq_TrackRecord)
		{
			tr->track=track;
			recordtrack->t_audiofx.tracksrecord.AddEndO(tr);
		}
	}
}

void Seq_TrackFX::DeleteTrackRecord()
{
	tracksrecord.DeleteAllO();
}

void Seq_TrackFX::DeleteTrackMix()
{
	if(stream)
	{
		for(int i=0;i<nrstream;i++)
			stream[i].DeleteARESOut();
		
		delete stream;
		stream=0;
		nrstream=0;
	}

	track->mix.DeleteARESOut();

	// Input Buffer
	if(t_inputbuffers)
	{
		for(int i=0;i<inputbufferscounter;i++)
			t_inputbuffers[i].DeleteARESOut();

		delete t_inputbuffers;
		t_inputbuffers=0;
		inputbufferscounter=0;
		t_audioinputwriteindex=t_audioinputreadindex=0;
	}
}

void Seq_TrackFX::CreateTrackMix(AudioDevice *device)
{
#ifdef DEBUG
	if(device && device->GetSetSize()<=0)
	{
		maingui->MessageBoxError(0,"CreateTrackMix setSize<0");
	}
#endif

	if(device && device->GetSetSize())
	{
		// Same as Existing Buffer ?
		if(device->GetSetSize()==track->mix.samplesinbuffer && 
			track->mix.outputbufferARES &&
			t_inputbuffers &&
			stream && 
			nrstream==device->numberhardwarebuffer
			)
		{
			track->mix.channelsused=0;
			track->mix.bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),device->GetSetSize());
			if(track->mix.bufferms<=0)
				track->mix.bufferms=0.1; // mini !

			track->mix.delayvalue=(int)floor((1000/track->mix.bufferms)+0.5);

			for(int i=0;i<inputbufferscounter;i++)
			{
				t_inputbuffers[i].bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),device->GetSetSize());

				if(t_inputbuffers[i].bufferms<=0)
					t_inputbuffers[i].bufferms=0.1; // mini !

				t_inputbuffers[i].delayvalue=(int)floor((1000/t_inputbuffers[i].bufferms)+0.5);
			}

			for(int i=0;i<nrstream;i++)
			{
				stream[i].channelsused=0;
				stream[i].bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),device->GetSetSize());
				if(stream[i].bufferms<=0)
					stream[i].bufferms=0.1; // mini !

				stream[i].delayvalue=(int)floor((1000/stream[i].bufferms)+0.5);
			}

			return;
		}
	}

	DeleteTrackMix();

	bool ok=true;

	if(device && device->GetSetSize())
	{
		track->mix.InitARESOut(device->GetSetSize(),MAXCHANNELSPERCHANNEL);

		if(!track->mix.outputbufferARES)
			ok=false;

		// Input
		if(device->numberhardwarebuffer && (t_inputbuffers=new AudioHardwareBuffer[inputbufferscounter=device->numberhardwarebuffer]))
		{
			for(int i=0;i<inputbufferscounter;i++)
			{
				t_inputbuffers[i].InitARESOut(device->GetSetSize(),MAXCHANNELSPERCHANNEL);
				t_inputbuffers[i].bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),device->GetSetSize());
				t_inputbuffers[i].SetBuffer(MAXCHANNELSPERCHANNEL,device->GetSetSize());

				if(t_inputbuffers[i].bufferms<=0)
					t_inputbuffers[i].bufferms=0.1; // mini !

				t_inputbuffers[i].delayvalue=(int)floor((1000/t_inputbuffers[i].bufferms)+0.5);

				//in->inputbuffers[i].samplesinbuffer=setSize;
			}
		}

		if(ok==true)
		{
			// Init Mix Out
			track->mix.channelsused=0;
			track->mix.SetBuffer(MAXCHANNELSPERCHANNEL,device->GetSetSize());
			//mix.channelsinbuffer=MAXCHANNELSPERCHANNEL;
			//mix.samplesinbuffer=device->setSize;

			track->mix.bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),device->GetSetSize());
			if(track->mix.bufferms<=0)
				track->mix.bufferms=0.1; // mini !

			track->mix.delayvalue=(int)floor((1000/track->mix.bufferms)+0.5);

			stream=new AudioHardwareBuffer[nrstream=device->numberhardwarebuffer]; // min 1 buffer

			if(stream)
			{
				for(int i=0;i<nrstream;i++)
				{
					stream[i].InitARESOut(device->GetSetSize(),MAXCHANNELSPERCHANNEL);
					stream[i].channelsused=0;
					stream[i].SetBuffer(MAXCHANNELSPERCHANNEL,device->GetSetSize());

					//stream[i].channelsinbuffer=MAXCHANNELSPERCHANNEL;
					//stream[i].samplesinbuffer=device->setSize;
					stream[i].bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),device->GetSetSize());

					if(stream[i].bufferms<=0)
						stream[i].bufferms=0.1; // mini !

					stream[i].delayvalue=(int)floor((1000/stream[i].bufferms)+0.5);

#ifdef _DEBUG
					stream[i].CheckBuffer();
#endif 
					if(!stream[i].outputbufferARES)
					{
						ok=false;
						break;
					}

				}// for
			}
			else
				ok=false;
		}

		if(ok==false)
		{
			DeleteTrackMix();
			MessageBox(NULL,"Error Creating Audio Track Buffer","Error",MB_OK);
		}
	}
}

Seq_TrackRecord *Seq_TrackFX::AddTrackRecord(Seq_Track *totrack)
{
	if(Seq_TrackRecord *nt=new Seq_TrackRecord)
	{
		nt->track=track;
		totrack->t_audiofx.tracksrecord.AddEndO(nt);
		return nt;
	}

	return 0;
}

void Seq_TrackFX::RemoveTrackRecord(Seq_Track *t,bool all)
{
	Seq_TrackRecord *f=FindTrackInRecord(t);

	if(f)
		tracksrecord.RemoveO(f);

	if(all==true)
		t->t_audiofx.recordtrack=0;
}

Seq_TrackRecord *Seq_TrackFX::FindTrackInRecord(Seq_Track *ctrack)
{
	Seq_TrackRecord *c=FirstTrackRecord();

	while(c)
	{
		if(c->track==ctrack)
			return c;

		c=c->NextTrackRecord();
	}

	return 0;
}

Seq_TrackFX::Seq_TrackFX()
{
	stream=0;
	nrstream=0;
	recordtrack=0;

	t_inputbuffers=0;
	inputbufferscounter=0;

	t_audioinputreadindex=0;
	t_audioinputwriteindex=0;

	t_lastfilledbufferindex=0;
	//t_lastfilledbufferthruindex=0;
}

bool Seq_Track::ConnectTrackOutputToPort(AudioPort *port,bool deleteother) // -> Virtual Channel
{
	if(port->channels>2)
		return false;

	mainthreadcontrol->LockActiveSong();

	io.SetPlaybackChannel(port,true,false);

	if(deleteother==true)
	{
		GetAudioOut()->Delete(); // Delete Connection -> Channels+Bus
	}

	usedirecttodevice=true;

	mainthreadcontrol->UnlockActiveSong();

	maingui->RefreshAudio(this);

	return true;
}

void Seq_Track::SetAudioIOType(int type)
{
	GetPeak()->Reset();
	io.SetChannelType(type);
	GetPeak()->channels=channelschannelsnumber[type];
	io.audioinputenable=true;
}

void Seq_Track::SetAudioIOType(AudioHDFile *hd)
{
	if(!hd)return;

	switch(hd->channels)
	{
	case 1:
		SetAudioIOType(CHANNELTYPE_MONO);
		break;

	case 2:
		SetAudioIOType(CHANNELTYPE_STEREO);
		break;
	}
}

bool Seq_Track::SetVType(AudioDevice *device,int type,bool guirefresh,bool allselected)
{
	if(type>1)return false; // Max Stereo

	if(ismetrotrack==false)
		mainaudio->defaultchannel_type=type;

	if(song==mainvar->GetActiveSong())
		mainthreadcontrol->LockActiveSong();

	if(ismetrotrack==true)
	{
		Seq_MetroTrack *t=song->FirstMetroTrack();

		while(t)
		{
			if(t==this)
			{
				t->SetAudioIOType(type);
			}

			t=t->NextMetroTrack();
		}
	}
	else
	{
		Seq_Track *t=song==0?this:song->FirstTrack();

		while(t)
		{
			if(t==this || (allselected==true && IsPartOfEditing(t,true)==true))
			{
				t->GetPeak()->Reset();
				t->io.SetChannelType(type);
				t->GetPeak()->channels=channelschannelsnumber[type];
			}

			if(allselected==false || song==0)
				break;

			t=t->NextTrack();
		}
	}

	if(song==mainvar->GetActiveSong())
		mainthreadcontrol->UnlockActiveSong();

	if(song && guirefresh==true)
	{
		maingui->RefreshAllEditors(song,EDITORTYPE_AUDIOMIXER,Edit_AudioMix::REFRESH_OUTCHANNELS);
		maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
		maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGELIST,0);
	}

	return true;
}

bool Seq_Track::IsPartOfEditing(Seq_Track *track,bool allchilds)
{
	if(!track)
		return false;

	if(this==track ||
		(maingui->GetCtrlKey()==false && IsSelected()==true && track->IsSelected()==true) ||
		((allchilds==true || maingui->GetShiftKey()==true) && IsTrackChildOfTrack(track)) 
		)
		return true;

	return false;
}

bool Seq_Track::CheckIfTrackIsMIDI()
{
	if(FirstPattern(MEDIATYPE_MIDI))
		return true;

	if(io.audioeffects.FirstInsertAudioInstrument())
		return true;

	return false;
}

bool Seq_Track::CheckIfTrackIsAudio()
{
	if(MIDItype==OUTPUTTYPE_AUDIOINSTRUMENT)
		return true;

	if(io.audioeffects.FirstInsertAudioEffect())
		return true;

	if(FirstPattern(MEDIATYPE_AUDIO))return true;
	if(FirstPattern(MEDIATYPE_AUDIO_RECORD))return true;

	return false;
}


// Audio System
bool Seq_Track::AddAudioChannel(AudioChannel *channel)
{
	if(GetAudioOut()->FindChannel(channel)==false)
	{
		if(song)
		{
			mainthreadcontrol->LockActiveSong();
			mainMIDI->StopAllofTrack(this);
		}

		if(GetAudioOut()->AddToGroup(channel))
			usedirecttodevice=false;

		if(song)
			mainthreadcontrol->UnlockActiveSong();

		return true;
	}

	return false;
}

bool Seq_Track::ReplaceAudioChannel(AudioChannel *oldchannel,AudioChannel *newchannel)
{
	if(oldchannel!=newchannel){

		if(song)
			mainthreadcontrol->LockActiveSong();

		GetAudioOut()->ReplaceChannel(oldchannel,newchannel);
		usedirecttodevice=false;

		if(song)
			mainthreadcontrol->UnlockActiveSong();
		return true;
	}

	return false;
}

bool Seq_Track::RemoveAudioChannel(AudioChannel *channel)
{
	if(GetAudioOut()->FindChannel(channel)==true)
	{
		if(song)
			mainthreadcontrol->LockActiveSong();

		GetAudioOut()->RemoveBusFromGroup(channel);
		if(!GetAudioOut()->FirstChannel())
			usedirecttodevice=true; // Back to device

		if(song)
			mainthreadcontrol->UnlockActiveSong();
		return true;
	}

	return false;
}

bool Seq_Track::FindAudioRegion(AudioRegion *r){

	if(r && r->r_audiohdfile){

		Seq_Pattern *p=FirstPattern(MEDIATYPE_AUDIO);

		while(p){

			AudioPattern *ap=(AudioPattern *)p;

			if(ap->audioevent.audioefile==r->r_audiohdfile)
				return true;

			p=p->NextPattern(MEDIATYPE_AUDIO);
		}
	}

	return false;
}

void Seq_Track::SetProcessor(Processor *p)
{
	if(GetProcessor()) // Close GUI
	{
		MIDIPlugin *pm=GetProcessor()->FirstProcessorModule();

		while(pm)
		{
			pm->CloseGUI();
			pm=pm->NextModule();
		}
	}

	mainthreadcontrol->LockActiveSong();
	mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);

	if(GetProcessor())
		GetProcessor()->Delete(true);

	if(p)
	{
		if(parent)
		{
			((Seq_Track *)parent)->t_processor=p->Clone();
			if( ((Seq_Track *)parent)->t_processor)
				((Seq_Track *)parent)->t_processor->track=(Seq_Track *)parent;
		}
		else
		{
			t_processor=p->Clone();
			if(t_processor)
				t_processor->track=this;
		}
	}
	else
	{
		if(parent)
			((Seq_Track *)parent)->t_processor=0;
		else
			t_processor=0;
	}

	mainthreadcontrol->UnlockActiveSong();
	mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);

	maingui->RefreshProcessorName(this);
}

void Seq_Track::SetProcessorBypass(bool onoff)
{
	mainthreadcontrol->LockActiveSong();
	mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);

	if(GetProcessor())
	{
		GetProcessor()->bypass=onoff;

		if(onoff==false)
		{
			MIDIPlugin *pm=GetProcessor()->FirstProcessorModule();

			while(pm)
			{
				MIDIalarmprocessorproc->DeleteAllAlarms(pm,0);
				pm=pm->NextModule();
			}
		}
	}

	mainthreadcontrol->UnlockActiveSong();
	mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);
}

bool Seq_Track::CheckMIDIInputEvent(NewEventData *data)
{
	if((!data->fromdev) && (!data->fromplugin))
	{
		if(GetFX()->useallinputdevices==true)
			return GetFX()->inputfilter.CheckBytes(data->status,data->byte1,data->byte2);

		return false;
	}

	if(GetFX()->useallinputdevices==true)
		return GetFX()->inputfilter.CheckBytes(data->status,data->byte1,data->byte2);

	if(data->fromdev)
	{
		Seq_Group_MIDIInPointer *sgp=GetMIDIIn()->FirstDevice();
		while(sgp)
		{
			if(sgp->GetDevice()==data->fromdev)
				return GetFX()->inputfilter.CheckBytes(data->status,data->byte1,data->byte2);

			sgp=sgp->NextGroup();
		}
	}
	else
	{
		if(data->fromplugin)
			return true;
	}

	return false;
}

bool Seq_Track::CheckMIDIInputEvent(MIDIInputDevice *indev,UBYTE status,UBYTE byte1,UBYTE byte2)
{
	if(!indev)
	{
		if(GetFX()->useallinputdevices==true)
			return GetFX()->inputfilter.CheckBytes(status,byte1,byte2);

		return false;
	}

	if(GetFX()->useallinputdevices==true)
		return GetFX()->inputfilter.CheckBytes(status,byte1,byte2);

	Seq_Group_MIDIInPointer *sgp=GetMIDIIn()->FirstDevice();
	while(sgp)
	{
		if(sgp->GetDevice()==indev)
			return GetFX()->inputfilter.CheckBytes(status,byte1,byte2);

		sgp=sgp->NextGroup();
	}

	return false;
}

bool Seq_Track::CheckMIDIInputEvent(MIDIInputDevice *indev,Seq_Event *e)
{
	if(!e)return false;

	if(GetFX()->useallinputdevices==true || (!indev))
		return GetFX()->inputfilter.CheckEvent(e);

	Seq_Group_MIDIInPointer *sgp=GetMIDIIn()->FirstDevice();
	while(sgp)
	{
		if(sgp->GetDevice()==indev)
			return GetFX()->inputfilter.CheckEvent(e);

		sgp=sgp->NextGroup();
	}

	return false;
}

bool Seq_Track::CheckIfTrackIsAudioInstrument()
{
	// Check Audio Instrument Output
	Seq_Track *p=(Seq_Track *)parent;
	while(p)
	{
		if(p->MIDItype==OUTPUTTYPE_AUDIOINSTRUMENT)
			return true;

		p=(Seq_Track *)p->parent;
	}

	if(MIDItype==OUTPUTTYPE_AUDIOINSTRUMENT)
		return true;

	return false;
}

void Seq_Track::SendOutEvent_AudioEvent(Seq_Event *audioevent,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{
	int sflag=(!(iflag&SOE_CREATEREALEVENT))?SENDAUDIO_DONTCREATEREALEVENTS:0;

	// Track Effects
	Seq_Track *track=this;

	do // Child->Parent Loop
	{

		if(iflag&SOE_INPUT)
		{
			audioevent->SendToAudio(0,&track->io.audioinputeffects,sflag,offset,dontsendtoaudioobject); // INPUT Audio Instruments
		}
		else
		{
			audioevent->SendToAudio(0,&track->io.audioeffects,sflag,offset,dontsendtoaudioobject); // Output Audio Instruments

#ifdef OLDIE
			// Track Channel
			Seq_AudioIOPointer *acp=track->GetAudioOut()->FirstChannel();

			while(acp)
			{
				if(acp->channel->audiochannelsystemtype==CHANNELTYPE_AUDIOINSTRUMENT 
					//&& acp->channel->io.audioeffects.FirstActiveAudioInstrument()
					)
				{
					audioevent->SendToAudio(0,&acp->channel->io.audioeffects,sflag,offset,dontsendtoaudioobject);
				}

				acp=acp->NextChannel();
			}
#endif

		}

		track=(Seq_Track *)track->parent;

	}while(track);
}

void Seq_Track::SendOutEvent_Automation(Seq_Event *e,int createflag)
{
	bool sendtoaudio=CheckIfTrackIsAudioInstrument();

	if(sendtoaudio==false)
	{
		if(!GetMIDIOut()->FirstDevice()) // MIDI Device ?
			return;
	}

	if(mainMIDI->MIDI_outputimpulse<=50)
		mainMIDI->MIDI_outputimpulse=100;

	if(sendtoaudio==true)
		SendOutEvent_AudioEvent(e,(createflag&Seq_Event::STD_CREATEREALEVENT)?SOE_CREATEREALEVENT:0,0);
	else
	{
		e->SetMIDIImpulse(this);
		

		Seq_Group_MIDIOutPointer *sgp=GetMIDIOut()->FirstDevice();

		while(sgp){
			e->SendToDevicePlaybackUser(sgp->GetDevice(),song,createflag);
			sgp=sgp->NextGroup();
		}
	}
}

void Seq_Track::SendOutEvent_User(MIDIPattern *p,Seq_Event *e,int createflag)
{
	if(song!=mainvar->GetActiveSong())return;

	bool sendtoaudio=CheckIfTrackIsAudioInstrument();

	if(sendtoaudio==false)
	{
		if(!GetMIDIOut()->FirstDevice()) // MIDI Device ?
			return;
	}

	if(e->pattern && (!(e->flag&EVENTFLAG_PROCALARM))) // Thru, Pattern=0 or ProcEvent
	{
		MIDIProcessor processor(song,this);

		// MIDI Output Processor
		processor.EventInput(p,e,e->GetEventStart());

		if(Seq_Event *oevent=processor.FirstProcessorEvent())
		{
			if(mainMIDI->MIDI_outputimpulse<=50)
				mainMIDI->MIDI_outputimpulse=100;

			while(oevent)
			{
				// Set Note Length Maximum
				if(oevent->GetStatus()==NOTEON)
				{
					Note *note=(Note *)oevent;

					if(note->GetNoteLength()>MAXUSERTICKS)
						note->SetLength(MAXUSERTICKS);

					TRACE ("Send User Note Lenght %d\n",note->GetNoteLength());
				}

				if(sendtoaudio==true) // ---> Audio
					SendOutEvent_AudioEvent(oevent,(createflag&Seq_Event::STD_CREATEREALEVENT)?SOE_CREATEREALEVENT:0,0);
				else // MIDI --->
				{
					oevent->SetMIDIImpulse(this);
					Seq_Group_MIDIOutPointer *sgp=GetMIDIOut()->FirstDevice();

					while(sgp)
					{
						oevent->SendToDevicePlaybackUser(sgp->GetDevice(),song,createflag);
						sgp=sgp->NextGroup();
					}
				}

				//if(oevent->SendTriggerImpulse()==true)
				//	realtimeeventcreated=true;

				oevent=processor.DeleteEvent(oevent);	
			}// while proc events	
		}
	}
	else // Thru Event
	{
		if(mainMIDI->MIDI_outputimpulse<=50)
			mainMIDI->MIDI_outputimpulse=100;

		if(sendtoaudio==true)
			SendOutEvent_AudioEvent(e,(createflag&Seq_Event::STD_CREATEREALEVENT)?SOE_CREATEREALEVENT:0,0);
		else
		{
			e->SetMIDIImpulse(this);
			Seq_Group_MIDIOutPointer *sgp=GetMIDIOut()->FirstDevice();

			while(sgp){
				e->SendToDevicePlaybackUser(sgp->GetDevice(),song,createflag);
				sgp=sgp->NextGroup();
			}
		}

		//if(e->SendTriggerImpulse()==true)
		//	realtimeeventcreated=true;
	}

	//if(realtimeeventcreated==true)
	//	mainMIDIalarmthread->SetSignal(); // Start MIDI Alarm
}

// Stop All Realtime Events, NoteOffs etc...
void Seq_Song::SendAllNoteOffs(int index,bool forcedelete,int flag){

	realtimeevents[index].Lock();

	RealtimeEvent *re=realtimeevents[index].FirstREvent();

	while(re){

		if(flag&SANOFF_ONLYAUDIO)
		{
			if(re->audioobject)
			{
				re->SendQuick();
				re=realtimeevents[index].DeleteREvent(re);
			}
			else
				re=re->NextEvent();
		}
		else
			if(flag&SANOFF_ONLYMIDI)
			{
				if(re->outputdevice)
				{
					re->SendQuick();
					re=realtimeevents[index].DeleteREvent(re);
				}
				else
					re=re->NextEvent();
			}
			else
			{
				// Send Note Offs
				if(re->noteoff==true)
				{
					re->SendQuick();
					re=realtimeevents[index].DeleteREvent(re);
				}
				else 
					re=(forcedelete==true)?realtimeevents[index].DeleteREvent(re):re->NextEvent();
			}
	}

	realtimeevents[index].UnLock();
}

void Seq_Song::SendAllNoteOffs(bool forcedelete,int flag){

	if(flag&SANOFF_ONLYAUDIO)
	{
		SendAllNoteOffs(REALTIMELIST_AUDIO,forcedelete,flag);
		return;
	}

	if(flag&SANOFF_ONLYMIDI)
	{
		SendAllNoteOffs(REALTIMELIST_MIDI,forcedelete,flag);
		return;
	}

	for(int i=0;i<REALTIME_LISTS;i++)
		SendAllNoteOffs(i,forcedelete,flag);
}

// Sub Track
void Seq_Track::AddChildTrack(Seq_Track *t,int index)
{
	AddChildObject(t,index);
}

char *Seq_Track::GetMIDIInputString(bool add)
{
	char *n=0;
	char *c=0;
	char *h=0;
	char nr[NUMBERSTRINGLEN];

	Seq_Group_MIDIInPointer *sgp=GetMIDIIn()->FirstDevice();

	if(GetFX()->noMIDIinput==true)
		c=Cxs[CXS_NOMIDIIN];
	else
	{
		if(GetFX()->useallinputdevices==true)
			c=Cxs[CXS_USEALLMIDIINDEVICES];
		else{
			if(sgp)
			{

				h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(sgp->portindex+1,nr),":",mainMIDI->MIDIinports[sgp->portindex].GetName());
				c=h;
			}
			else
				c=Cxs[CXS_NOMIDIINDEV];
		}
	}

	char *r=0;

	if(GetFX()->usealwaysthru==true)
		r=add==true?mainvar->GenerateString("mI:",c,"*"):mainvar->GenerateString(c,"*");
	else
		r=add==true?mainvar->GenerateString("mI:",c):mainvar->GenerateString(c);

	if(h)
		delete h;

	if(sgp && sgp->NextGroup())
	{
		int c=GetMIDIIn()->GetCountGroups();
		h=mainvar->GenerateString("[",mainvar->ConvertIntToChar(c,nr),"]",r);

		char *or=r;

		r=h;

		if(or)
			delete or;
	}

	return r;
}

char *Seq_Track::GetMIDIOutputString(bool add)
{
	Seq_Group_MIDIOutPointer *sgp=GetMIDIOut()->FirstDevice();

	char *n=0;
	char *c=0;
	char *h=0;
	char nr[NUMBERSTRINGLEN];

	if(sgp)
	{
		h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(sgp->portindex+1,nr),":",mainMIDI->MIDIoutports[sgp->portindex].GetName());
		c=h;
	}
	else
		c=Cxs[CXS_NOMIDIOUT];

	char *r=add==true?mainvar->GenerateString("mO:",c):mainvar->GenerateString(c);

	if(h)
		delete h;

	if(sgp && sgp->NextGroup())
	{
		int c=GetMIDIOut()->GetCountGroups();
		h=mainvar->GenerateString("[",mainvar->ConvertIntToChar(c,nr),"]",r);

		char *or=r;

		r=h;

		if(or)
			delete or;
	}

	return r;
}

int Seq_Track::GetCountChildTracks()
{
	int c=0;

	Seq_Track *t=FirstChildTrack();
	while(t && t->IsTrackChildOfTrack(this))
	{
		c++;
		t=t->NextTrack();
	}

	return c;
}


void Seq_Track::SetRecordMode(bool on,OSTART position)
{
	if(on==true && frozen==true)
		return;

	if(record==on)
		return;

	if(ismetrotrack==false)
	{
		if((song->status & Seq_Song::STATUS_RECORD) && record==true)
		{
			mainaudio->StopRecordingFiles(this);
		}

		if(on==true)
			songstartposition=song->GetSongPosition();

		record=on;

		SetTempInputMonitoring();
	}
}

void Seq_Track::SetTempInputMonitoring()
{
	if(FirstChildTrack())
	{
		io.tempinputmonitoring=false;
	}
	else
		io.tempinputmonitoring=(recordtracktype==TRACKTYPE_AUDIO && record==true) || /*inputmonitoring==true || */ io.inputmonitoring==true?true:false;
}

void Seq_Track::SetAudioInputEnable(bool on)
{
	io.audioinputenable=on;
	maingui->RefreshAudio(this);
}

bool Seq_Track::SetRecordChannel(AudioPort *vh,bool lock)
{
	if(lock==true)
		mainthreadcontrol->LockActiveSong();

	io.SetInput(vh);
	io.audioinputenable=true;

	if(t_audiofx.recordtrack)
		t_audiofx.recordtrack->t_audiofx.RemoveTrackRecord(this,true);

	if(lock==true)
		mainthreadcontrol->UnlockActiveSong();

	maingui->RefreshAudio(this);

	//maingui->RefreshAllEditors(song,EDITORTYPE_AUDIOMIXER,0);

	return true;
}
