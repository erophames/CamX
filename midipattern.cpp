#include "defines.h"
#include "MIDIPattern.h"
#include "object_track.h"
#include "object_song.h"
#include "songmain.h"
#include "semapores.h"
#include "camxfile.h"
#include "gui.h"
#include "drumevent.h"
#include "MIDIhardware.h"
#include "initplayback.h"
#include "chunks.h"
#include "languagefiles.h"
#include "MIDIfile.h"

#include <math.h>

bool MIDIPattern::CheckEventsWithChannel(UBYTE channel)
{
	bool found=false;
	Seq_Event *e=FirstEvent();

	while(e && found==false)
	{
		if(e->CheckIfChannelMessage()==true &&
			e->GetChannel()==channel
			)
			found=true;

		e=e->NextEvent();
	}

	return found;
}

void MIDIPattern::CreatePatternInfo(MIDIPatternInfo *info)
{
	Seq_Event *e=FirstEvent();

	while(e)
	{
		switch(e->GetStatus())
		{
		case NOTEON:
			info->notes++;
			break;

		case CONTROLCHANGE:
			info->control++;
			break;

		case SYSEX:
			info->sysex++;
			break;

		case PITCHBEND:
			info->pitch++;
			break;

		case POLYPRESSURE:
			info->polypress++;
			break;

		case INTERN:
		case INTERNCHAIN:
			info->intern++;
			break;

		case CHANNELPRESSURE:
			info->cpress++;
			break;

		case PROGRAMCHANGE:
			info->prog++;
			break;
		}

		e=e->NextEvent();
	}
}

UBYTE MIDIPattern::CheckPatternChannel() // 0==thru, 1-16
{
	UBYTE channel=0;
	bool set=false;
	bool founddifferentchannels=false;

	Seq_Event *e=FirstEvent();

	while(e && founddifferentchannels==false)
	{
		if(e->CheckIfChannelMessage()==true)
		{
			if(set==false)
			{
				channel=e->GetChannel();
				set=true;
			}
			else
			{
				if(e->GetChannel()!=channel)
					founddifferentchannels=true;
			}
		}

		e=e->NextEvent();
	}

	if(set==false || founddifferentchannels==true)
		return 0;
	else
		return channel+1;
}

int MIDIPattern::GetDominantMIDIChannel()
{
	int chl=-1;
	int check=0;
	int chlc[16];

	Seq_Event *e=FirstEvent();

	for(int i=0;i<16;i++)
		chlc[i]=0;

	while(e)
	{
		if(e->CheckIfChannelMessage()==true)
			chlc[e->GetChannel()]++;

		e=e->NextEvent();
	}

	for(int i=0;i<16;i++)
	{
		if(chlc[i]>check)
		{
			chl=i;
			check=chlc[i];
		}
	}

	return chl;
}

void MIDIPattern::ConvertOpenRecordingNotesToNotes(OSTART endpos)
{
	LockOpenNotes();

	NoteOpen *onote=FirstOpenNote();

	while(onote)
	{
		//	OSTART qtime=t->GetFX()->quantizeeffect.Quantize();

		if(Note *note=NewNote(onote->ostart))
		{
			note->status=onote->status;
			note->key=onote->key;
			note->velocity=onote->velocity;
			note->velocityoff=0;

			OSTART iendpos=onote->ostart>=endpos?onote->ostart+1:endpos;

			note->off.staticostart=iendpos;

			AddSortVirtual(&note->off,iendpos); // NoteOff										
		}

		onote=(NoteOpen *)openrecnotes.RemoveO(onote);
	}

	UnlockOpenNotes();
}

void MIDIPattern::DeleteOpenNotes()
{
	LockOpenNotes();

	NoteOpen *note=FirstOpenNote();
	while(note)
		note=(NoteOpen *)openrecnotes.RemoveO(note);

	UnlockOpenNotes();
}

void MIDIPattern::DeleteEvents()
{
	Seq_Event *f=FirstEvent();

	while(f)
		f=DeleteEvent(f,true);

	// notes.Clear(); // Reset and Empty
}

void MIDIPattern::CloneEvents(Seq_Song *song,MIDIPattern *to)
{
	Seq_Event *e=FirstEvent();

	while(e)
	{
		e->CloneNewAndSortToPattern(song,to,0);
		e=e->NextEvent();
	}
}

void MIDIPattern::Delete(bool full)
{
	DeleteAllCrossFades(full);
	DeleteOpenNotes();

	if(full==true)
	{
		if(!mainpattern) // Loops dont have MIDI events !
		{
			DeleteEvents();
		}

		DeleteClones();
		if(patternname)delete patternname;
		delete this;
	}
}

void MIDIPattern::CloneEventsWithChannel(Seq_Song *song,int channel,MIDIPattern *newpattern) // split function
{
	// Clone all Events
	Seq_Event *e=FirstEvent();

	while(e)
	{
		if(e->CheckIfChannelMessage()==true && e->GetChannel()==channel)
			e->CloneNewAndSortToPattern(song,newpattern,0);

		e=e->NextEvent();
	}
}

void MIDIPattern::MixEventsToPattern(Seq_Song *song,MIDIPattern *newpattern) // mix to pattern
{
	// Mix all Events Add
	Seq_Event *e=FirstEvent();

	while(e){

		e->CloneNewAndSortToPattern(song,newpattern,0);
		e=e->NextEvent();
	}
}

void MIDIPattern::CloneMixEventsToPattern(Seq_Song *song,MIDIPattern *newpattern)
{
	OSTART diff=newpattern->GetPatternStart()-GetPatternStart();

	// Clone all Events 1-1
	Seq_Event *e=FirstEvent();

	while(e){

		e->CloneNewAndSortToPattern(song,newpattern,diff,0);
		e=e->NextEvent();
	}
}

bool MIDIPattern::IsPatternSysExPattern()
{
	if(GetCountOfEvents()==1 && FirstEvent()->GetStatus()==SYSEX)
	{
		return true;
	}

	return false;
}

void MIDIPattern::MoveAllEvents(MIDIPattern *topattern,Seq_Event *e)
{
	// Clone all Events
	if(!e)e=FirstEvent();

	while(e)
	{
		Seq_Event *n=e->NextEvent();

		events.MoveOToOList(&topattern->events,e); // Sort

		switch(e->id)
		{
		case OBJ_NOTE:
			{
				Note *on=(Note *)e;
				virtualevents.MoveOToOList(&topattern->virtualevents,&on->off);
				on->off.pattern=topattern;
			}
			break;

			/*
			case OBJ_SYSEX:
			{
			SysEx *sys=(SysEx *)e;

			if(sys->sysexend.pattern)
			{
			virtualevents.MoveOToOList(&topattern->virtualevents,&sys->sysexend);
			sys->sysexend.pattern=topattern;
			}
			}
			break;
			*/
		}

		e->pattern=topattern;
		e=n;
	}
}

#ifdef OLDIE
void MIDIPattern::MoveAllEventsToPattern(Seq_Event *e,MIDIPattern *topattern)
{
	if(!e) // e==0 All Events
		e=FirstEvent();

	while(e)
	{
		Seq_Event *n=e->NextEvent();

		switch(e->id)
		{
		case OBJ_NOTE:
			{
				Note *on=(Note *)e;
				virtualevents.MoveOToOList(&topattern->virtualevents,&on->off);
				on->off.pattern=topattern;
			}
			break;

			/*
			case OBJ_SYSEX:
			{
			SysEx *sys=(SysEx *)e;

			if(sys->sysexend.pattern)
			{
			virtualevents.MoveOToOList(&topattern->virtualevents,&sys->sysexend);
			sys->sysexend.pattern=topattern;
			}
			}
			break;
			*/
		}

		events.MoveOToEndOList(&topattern->events,e);
		e->pattern=topattern;

		e=n;
	}
}
#endif

void MIDIPattern::MoveEventsWithType(UBYTE status,MIDIPattern *topattern)
{
	// Clone all Events
	Seq_Event *e=FirstEvent();

	while(e)
	{
		Seq_Event *n=e->NextEvent();

		if(e->GetStatus()==status)
		{
			events.MoveOToEndOList(&topattern->events,e);

			switch(e->id)
			{
			case OBJ_NOTE:
				{
					Note *on=(Note *)e;
					virtualevents.MoveOToOList(&topattern->virtualevents,&on->off);
					on->off.pattern=topattern;
				}
				break;

				/*
				case OBJ_SYSEX:
				{
				SysEx *sys=(SysEx *)e;

				if(sys->sysexend.pattern)
				{
				virtualevents.MoveOToOList(&topattern->virtualevents,&sys->sysexend);
				sys->sysexend.pattern=topattern;
				}
				}
				break;
				*/
			}

			e->pattern=topattern;
		}

		e=n;
	}
}

void MIDIPattern::MoveEventsWithChannel(int channel,MIDIPattern *topattern)
{
	// Clone all Events
	Seq_Event *e=FirstEvent();

	while(e)
	{
		Seq_Event *n=e->NextEvent();

		if(e->CheckIfChannelMessage()==true && e->GetChannel()==channel)
		{
			events.MoveOToEndOList(&topattern->events,e);

			switch(e->id)
			{
			case OBJ_NOTE:
				{
					Note *on=(Note *)e;
					virtualevents.MoveOToOList(&topattern->virtualevents,&on->off);
					on->off.pattern=topattern;
				}
				break;

				/*
				case OBJ_SYSEX:
				{
				SysEx *sys=(SysEx *)e;

				if(sys->sysexend.pattern)
				{
				virtualevents.MoveOToOList(&topattern->virtualevents,&sys->sysexend);
				sys->sysexend.pattern=topattern;
				}
				}
				break;
				*/
			}

			e->pattern=topattern;
		}

		e=n;
	}
}

Seq_Pattern *MIDIPattern::CreateLoopPattern(int loop,OSTART pos,int flag)
{
	return new MIDIPattern;
}

LONGLONG MIDIPattern::GetSampleStart(Seq_Song *song)
{
	LONGLONG rv=-1;

	LockOpenNotes();
	if(FirstOpenNote())
	{
		rv=FirstOpenNote()->GetSampleStart(song);
	}
	UnlockOpenNotes();

	Seq_Event *e=FirstEvent(),*ve=FirstVirtualEvent();

	if(e || ve)
	{
		LONGLONG ev=e->GetSampleStart(song,this),vev=ve->GetSampleStart(song,this);

		if(e){
			if(!ve){
				if(rv==-1)
					return ev;

				return rv<ev?rv:ev;
			}

			if(ev<vev)
			{
				if(rv==-1)
					return ev;

				return rv<ev?rv:ev;
			}
		}

		if(rv==-1)
			return vev;

		return rv<vev?rv:vev;
	}

	if(rv==-1)
		return OStart::GetSampleStart(song);

	return rv;
}

OSTART MIDIPattern::GetPatternStart() // v
{
	OSTART rv=-1;

	LockOpenNotes();

	if(FirstOpenNote())
		rv=FirstOpenNote()->ostart;
	UnlockOpenNotes();

	Seq_Event *e=FirstEvent(),*ve=FirstVirtualEvent();

	if(e || ve)
	{
		if(e)
		{
			if(!ve)
			{
				if(rv==-1)
					return e->GetEventStart(this);

				return rv<e->GetEventStart(this)?rv:e->GetEventStart(this);
			}

			if(e->GetEventStart(this)<ve->GetEventStart(this))
			{
				if(rv==-1)
					return e->GetEventStart(this);

				return rv<e->GetEventStart(this)?rv:e->GetEventStart(this);
			}
		}

		if(rv==-1)
			return ve->GetEventStart(this);

		return rv<ve->GetEventStart(this)?rv:ve->GetEventStart(this);
	}

	if(rv==-1)
		return ostart; //object start

	return rv<ostart?rv:ostart;
}

OSTART MIDIPattern::GetPatternEnd() // v
{
	if(IsPatternSysExPattern()==true)
	{
		return GetPatternStart()+16*SAMPLESPERBEAT;
	}

	OSTART rv=-1;

	LockOpenNotes();
	if(FirstOpenNote())
		rv=track->song->GetSongPosition();
	UnlockOpenNotes();

	Seq_Event *e=LastEvent(),*ve=LastVirtualEvent();

	if(e || ve)
	{
		if(e)
		{
			if(!ve)
			{
				if(rv==-1)
					return e->GetEventStart(this);

				return rv>e->GetEventStart(this)?rv:e->GetEventStart(this);
			}

			if(e->GetEventStart(this)>ve->GetEventStart(this))
			{
				if(rv==-1)
					return e->GetEventStart(this);

				return rv>e->GetEventStart(this)?rv:e->GetEventStart(this);
			}
		}

		if(rv==-1)
			return ve->GetEventStart(this);

		return rv>ve->GetEventStart(this)?rv:ve->GetEventStart(this);
	}

	if(rv==-1)
		return ostart;

	return rv>ostart?rv:ostart;
}

/*
void MIDIPattern::AddSortNote(OStart *nl,OSTART start)
{
notes.AddOSort(nl,start);
}
*/

void MIDIPattern::AddSortEvent(Seq_Event *e,OSTART start)
{
	e->SetList(&events);
	e->pattern=this;
	events.AddOSort(e,start);
}

void MIDIPattern::AddSortEvent(Seq_Event *e)
{
	e->SetList(&events);
	e->pattern=this;
	events.AddOSort(e);
}

//  Virtual
void MIDIPattern::AddSortVirtual(Seq_Event *ve,OSTART start)
{
	ve->SetList(&virtualevents);
	ve->pattern=this;
	virtualevents.AddOSort(ve,start);
}

void MIDIPattern::AddSortVirtual(Seq_Event *ve)
{
	ve->SetList(&virtualevents);
	ve->pattern=this;
	virtualevents.AddOSort(ve,ve->ostart);
}

void MIDIPattern::DeleteVirtual(Seq_Event *e) // NoteOffs, SysExEnd etc...
{
	virtualevents.CutQObject(e);
}

bool MIDIPattern::AddGMSysEx(bool gs,bool refreshgui)
{
	if(SysEx *sys=new SysEx)
	{
		//		 GS reset     	  F0 41 10 42 12 40 00 7F 00 41 F7
		//		 GM reset   	  F0 7E 7F 09 01 F7

		sys->ostart=sys->staticostart=GetPatternStart();

		if(gs==true)
			sys->data=new UBYTE[11];
		else
			sys->data=new UBYTE[6];

		if(sys->data)
		{
			if(gs==true) // GS
			{
				sys->length=11;

				sys->data[0]=0xF0;
				sys->data[1]=0x41;
				sys->data[2]=0x10;
				sys->data[3]=0x42;
				sys->data[4]=0x12;
				sys->data[5]=0x40;
				sys->data[6]=0x00;
				sys->data[7]=0x7F;
				sys->data[8]=0x00;
				sys->data[9]=0x41;
				sys->data[10]=0xF7;

				SetName("GS ON SysEx");

			}
			else // GM
			{
				sys->length=6;

				sys->data[0]=0xF0;
				sys->data[1]=0x7E;
				sys->data[2]=0x7F;
				sys->data[3]=0x09;
				sys->data[4]=0x01;
				sys->data[5]=0xF7;

				SetName("GM ON SysEx");
			}

			mainthreadcontrol->LockActiveSong();
			AddSortEvent(sys,GetPatternStart());
			mainthreadcontrol->UnlockActiveSong();

			if(refreshgui==true)
			{
				maingui->RefreshAllEditorsWithPattern(track->song,this);
				maingui->ClearRefresh();
			}
		}
		else
		{
			delete sys;
			return false;
		}

		return true;
	}

	return false;
}

void MIDIPattern::Load(camxFile *file) // v
{
	file->ReadAndAddClass((CPOINTER)this);

	file->ReadChunk(&sendonlyatstartup);

	file->CloseReadChunk();

	LoadStandardEffects(file);
	MIDIprogram.Load(file);

	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_MIDIEVENTS)
	{
		int mc=0;
		OSTART offset=0;
		bool offsetset=false;
		
		file->ChunkFound();
		file->ReadChunk(&mc);

		while(mc--)
		{
			OSTART ostart=0,sstart=0;
			UBYTE status=0;

			file->ReadChunk(&ostart);
			file->ReadChunk(&sstart);
			file->ReadChunk(&status);

			if(offsetset==false)
			{
				offsetset=true;

				if(loadoffset!=-1)
				{
					offset=loadoffset-ostart; // Offset 1. Event
				}
			}

			ostart+=offset;
			sstart+=offset;

			if(status==INTERN) // ICD
			{
				ICD_Object h;
				h.ReadAndAddToPattern(this,file,ostart,sstart);
			}
			else
			if(status==INTERNCHAIN)
			{
				ICD_Object_Seq_MIDIChainEvent h;
				h.ReadAndAddToPattern(this,file,ostart,sstart);
			}
			else
				switch(status&0xF0)
			{
				case NOTEON:
					{
#ifdef MEMPOOLS
						Note *note=mainpools->mempGetNote();
#else
						Note *note=new Note;
#endif

						if(note)
						{
							note->ostart=ostart;
							note->staticostart=sstart;
							note->status=status;

							note->Load(file);

							note->off.ostart+=offset;
							note->off.staticostart+=offset;

							note->AddSortToPattern(this);
						}
					}
					break;

				case POLYPRESSURE:
					{
						if(PolyPressure *p=new PolyPressure)
						{
							p->ostart=ostart;
							p->staticostart=sstart;
							p->status=status;
							p->Load(file);
							p->AddSortToPattern(this);
						}
					}
					break;

				case CONTROLCHANGE:
					{
#ifdef MEMPOOLS
						ControlChange *c=mainpools->mempGetControl();
#else
						ControlChange *c=new ControlChange;
#endif

						if(c)
						{	
							c->ostart=ostart;
							c->staticostart=sstart;
							c->status=status;
							c->Load(file);
							c->AddSortToPattern(this);
						}
					}
					break;

				case PROGRAMCHANGE:
					{
						if(ProgramChange *p=new ProgramChange)
						{
							p->ostart=ostart;
							p->staticostart=sstart;
							p->status=status;

							p->Load(file);
							p->AddSortToPattern(this);
						}
					}
					break;

				case CHANNELPRESSURE:
					{
						if(ChannelPressure *cp=new ChannelPressure)
						{
							cp->ostart=ostart;
							cp->staticostart=sstart;
							cp->status=status;

							cp->Load(file);
							cp->AddSortToPattern(this);
						}
					}
					break;

				case PITCHBEND:
					{
#ifdef MEMPOOLS
						Pitchbend *p=mainpools->mempGetPitchbend();
#else
						Pitchbend *p=new Pitchbend;
#endif

						if(p)
						{
							p->ostart=ostart;
							p->staticostart=sstart;
							p->status=status;

							p->Load(file);
							p->AddSortToPattern(this);
						}
					}
					break;

				case SYSEX:
					{
						if(SysEx *s=new SysEx)
						{
							s->ostart=ostart;
							s->staticostart=sstart;

							s->Load(file);
							s->AddSortToPattern(this);
						}
					}
					break;
			}
		}

		CloseEvents();

		file->CloseReadChunk();
	}

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_MIDIEVENTSFLAGS)
	{
		int mc=0;

		file->ChunkFound();
		file->ReadChunk(&mc);

		Seq_Event *e=FirstEvent();
		while(e && mc)
		{
			file->ReadChunk(&e->flag);
			e->flag = e->flag&EVENTFLAG_MUTED; // Clear Rest
			mc--;
			e=e->NextEvent();
		}

		file->CloseReadChunk();
	}

	// Effects
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_MIDIEFFECTS)
	{
		file->ChunkFound();
		t_MIDIeffects.Load(file);
	}
}

void MIDIPattern::Save(camxFile *file) // v
{
	file->OpenChunk(CHUNK_MIDIPattern);

	file->Save_Chunk((CPOINTER)this);
	file->Save_Chunk(sendonlyatstartup);

	file->CloseChunk();

	SaveStandardEffects(file); // Pattern Effects
	MIDIprogram.Save(file);

	if(!mainclonepattern)
	{
		// Save Events
		if(GetCountOfEvents())
		{
			file->OpenChunk(CHUNK_MIDIEVENTS);
			file->Save_Chunk(GetCountOfEvents());

			// Pattern
			Seq_Event *e=FirstEvent();
			while(e)
			{
				file->Save_Chunk(e->ostart);
				file->Save_Chunk(e->staticostart);
				file->Save_Chunk(e->status);

				e->Save(file);

				e=e->NextEvent();
			}

			file->CloseChunk();

			file->OpenChunk(CHUNK_MIDIEVENTSFLAGS);
			file->Save_Chunk(GetCountOfEvents());

			// Pattern
			e=FirstEvent();
			while(e)
			{
				file->Save_Chunk(e->flag);
				e=e->NextEvent();
			}

			file->CloseChunk();
		}
	}
	else
	{
		// Clone Pattern NO MIDIEvents
		file->OpenChunk(CHUNK_MIDIEVENTS);
		int clone=0;
		file->Save_Chunk(clone);
		file->CloseChunk();
	}

	// MIDIPattern  Effects
	t_MIDIeffects.Save(file);
}

void MIDIPattern::SaveToSysExFile(guiWindow *win)
{
	if(this->IsPatternSysExPattern()==true)
	{
		camxFile save;

		if (save.OpenFileRequester(0,win,Cxs[CXS_SAVEPATTERNASSYSEX],save.AllFiles(camxFile::FT_SYSEX),false,GetName())==true)
		{					
			save.AddToFileName(".syx");

			MIDIFile MIDIfile;

			MIDIfile.SavePatternToSysFile(this,save.filereqname);
			save.Close(true);
		}
	}
}

void MIDIPattern::RefreshIndexs()
{
	if(eventclose==true) // avoid 2x Close...
	{
		eventclose=false;
		events.Close(); // Index
	}
}

void MIDIPattern::CloneFX(Seq_Pattern *to)
{
	MIDIPattern *newp=(MIDIPattern *)to;

	t_colour.Clone(&newp->t_colour);
	GetMIDIFX()->Clone(&newp->t_MIDIeffects);
	quantizeeffect.Clone(&newp->quantizeeffect);
	newp->loops=loops;
	newp->mute=mute;

	MIDIprogram.Clone(&newp->MIDIprogram);
}

void MIDIPattern::Clone(Seq_Song *song,Seq_Pattern *pp,OSTART diff,int iflag) // VF
{
	MIDIPattern *newp=(MIDIPattern *)pp;

	newp->ostart=ostart;

	if(!(iflag&CLONEFLAG_NOFX))
	{
		// Clone Data
		newp->SetName(GetName()); // copy name

		t_colour.Clone(&newp->t_colour);
		GetMIDIFX()->Clone(&newp->t_MIDIeffects);
		quantizeeffect.Clone(&newp->quantizeeffect);

		CloneLoops(newp);
		newp->mute=mute;
	}

	MIDIprogram.Clone(&newp->MIDIprogram);

	if(!(iflag&CLONEFLAG_NODATA))
	{
		// Clone all Events
		Seq_Event *e=FirstEvent();

		while(e)
		{
			if(e->ClonePossible(song)==true)
			{
				e->CloneNewAndSortToPattern(song,newp,diff,iflag);	
			}

			e=e->NextEvent();
		}

		newp->CloseEvents();
	}
}

Seq_Pattern *MIDIPattern::CreateClone(OSTART startdiff,int flag) // v
{
	if(MIDIPattern *newclone=new MIDIPattern)
	{
		Clone(track->song,newclone,startdiff,flag);
		return newclone;
	}

	return 0;
}

Seq_Event* MIDIPattern::DeleteEvent(Seq_Event *e,bool full)
{	
	Seq_Event *n=e->NextEvent();
	e->DeleteFromPattern();
	if(full==true)e->Delete(full);
	return n;
}

/*
Note *MIDIPattern::FindNoteAtPosition(OSTART pos)
{
NoteConnect *c;

if(mainpattern)
{
if(pos>=offset)
{
if(mainpattern->mainclonepattern)
{
c=(NoteConnect *)((MIDIPattern *)mainpattern->mainclonepattern)->notes.FindObject(pos-offset);
goto exit;
}

c=(NoteConnect *)((MIDIPattern *)mainpattern)->notes.FindObject(pos-offset);
goto exit;
}
else
{
if(mainpattern->mainclonepattern)
{
c=(NoteConnect *)((MIDIPattern *)mainpattern->mainclonepattern)->notes.GetRoot();
goto exit;
}

c=(NoteConnect *)((MIDIPattern *)mainpattern)->notes.GetRoot();
goto exit;
}
}

c=(NoteConnect *)notes.FindObject(pos);

exit:
return c?c->note:0;
}
*/

Seq_Event *MIDIPattern::FindEventAtPosition(OSTART pos)
{
	if(mainpattern)
	{
		if(pos>=offset)
		{
			if(mainpattern->mainclonepattern)
				return(Seq_Event *)((MIDIPattern *)mainpattern->mainclonepattern)->events.FindObject(pos-offset);

			return(Seq_Event *)((MIDIPattern *)mainpattern)->events.FindObject(pos-offset);
		}
		else
		{
			if(mainpattern->mainclonepattern)
				return(Seq_Event *)((MIDIPattern *)mainpattern->mainclonepattern)->events.GetRoot();

			return(Seq_Event *)((MIDIPattern *)mainpattern)->events.GetRoot();
		}
	}

	return (Seq_Event *)events.FindObject(pos);
}

Seq_Event *MIDIPattern::FindEventAtPosition(OSTART position,int filter,int icdtype)
{
	Seq_Event *fe=FindEventAtPosition(position);

	if(filter!=SEL_ALLEVENTS)
		while(fe && fe->CheckSelectFlag(filter,icdtype)==false)
			fe=fe->NextEvent();		

	return fe;
}

Seq_Event *MIDIPattern::FindEventInRange(OSTART startposition,OSTART endposition,int cflag)
{
	Seq_Event *fe=FindEventAtPosition(startposition);

	if(cflag!=SEL_ALLEVENTS)
		while(fe && fe->GetEventStart(this)<=endposition && fe->CheckSelectFlag(cflag,0)==false) // Right range end
			fe=fe->NextEvent();	

	return fe;
}

void MIDIPattern::MovePatternData(OSTART diff,int cflag)
{
	if(diff && mainpattern==0)
	{
		Seq_Event *e=FirstEvent();

		while(e){	
			e->MoveEventQuick(diff);
			e=e->NextEvent();
		}
	}
}

void MIDIPattern::BuildChainEventList()
{
	// Create Note ---- Note --- Note Chain for Realtime Swing etc...
	firstchainevent=lastchainevent=0;

	Seq_Event *e=FirstEvent();

	while(e){	

		if(e->IsSwingAble()==true)
		{
			Seq_MIDIChainEvent *s=(Seq_MIDIChainEvent *)e;

			if(!firstchainevent)
			{
				firstchainevent=s;
				s->prev_chainevent=0;
			}
			else
			{
				lastchainevent->next_chainevent=s;
				s->prev_chainevent=lastchainevent;
			}

			lastchainevent=s;
			s->next_chainevent=0;
		}

		e=e->NextEvent();
	}

#ifdef DEBUG
	int i=0;
#endif

}

void MIDIPattern::CloseEvents()
{
	events.Close(); // Index
	BuildChainEventList(); // Create Note Chain
}

int MIDIPattern::QuantizePattern(QuantizeEffect *fx)
{
	if(mainpattern==0)
	{
		if(Seq_Event *e=FirstEvent())
		{
			int xc=GetCountOfEvents();

			if(xc==0)
				return 0;

			Seq_Event **pp=new Seq_Event*[xc]; 

			if(pp)
			{
				Seq_Event **pc=pp;

				while(e) // Copy To Buffer
				{
					*pc++=e;
					e=e->NextEvent();
				}

				int c=0;

				pc=pp;

				while(xc--){

					switch((*pc)->GetStatus())
					{
					case NOTEON:
						{
							if(fx->flag&QUANTIZE_NOTES)
								goto doquant;
						}
						break;

					case CONTROLCHANGE:
						{
							if(fx->flag&QUANTIZE_CONTROL)
								goto doquant;
						}
						break;

					case PITCHBEND:
						{
							if(fx->flag&QUANTIZE_PITCHBEND)
								goto doquant;
						}
						break;

					case POLYPRESSURE:
						{
							if(fx->flag&QUANTIZE_POLYPRESSURE)
								goto doquant;
						}
						break;

					case CHANNELPRESSURE:
						{
							if(fx->flag&QUANTIZE_CHANNELPRESSURE)
								goto doquant;
						}
						break;

					case PROGRAMCHANGE:
						{
							if(fx->flag&QUANTIZE_PROGRAMCHANGE)
								goto doquant;
						}
						break;

					case SYSEX:
						{
							if(fx->flag&QUANTIZE_SYSEX)
								goto doquant;
						}
						break;

					case INTERN:
						break;

					case INTERNCHAIN:
						{
							if((*pc)->GetICD()==ICD_TYPE_DRUM) // Drum Event
								goto doquant;
							else
								if(fx->flag&QUANTIZE_INTERN)
									goto doquant;
						}
						break;
					}

					goto donotquant;

doquant:
					// ################# Quantize Event #################

					if((*pc)->QuantizeEvent(fx)==true){
						c++;
					}

donotquant:
					pc++;

				}//while

				delete pp; // free buffer mem

				if(c)
					CloseEvents();

				return c;
			}
		}
	}

	return 0;
}

bool MIDIPattern::SendStartupSysEx()
{
	if(IsPatternSysExPattern()==true && sendonlyatstartup==true)
	{
		GetTrack()->SendOutEvent_User(this,FirstEvent(),true);
		return true;
	}

	return false;
}

MIDIPattern::MIDIPattern()
{
	id=OBJ_MIDIPattern;
	mediatype=MEDIATYPE_MIDI;

	for(int i=0;i<INITPLAY_MAX;i++)
	{
		playback_MIDIevent[i][0]=playback_MIDIevent[i][1]=
			playback_nextchainevent[i][0]=playback_nextchainevent[i][1]=0;
	}

	t_MIDIeffects.pattern=this;
	sendonlyatstartup=false;

	firstchainevent=lastchainevent=0;
}

void MIDIPattern::AddBankProgRealtime(OSTART init_position,bool calledbycycle)
{
	if(init_position<GetPatternStart() || (calledbycycle==true && init_position==GetPatternStart()))
	{
		/*
		if(Control_Realtime *cr=new Control_Realtime)
		{
		cr->rteflag=RealtimeEvent::RTE_MIDIPatternSTART_EVENT|RealtimeEvent::MIDIPatternSTART_EVENT;

		if(mainpattern)
		cr->rte_frompattern=(MIDIPattern *)mainpattern; // Clone use mainpattern BankSelect/ProgrChange
		else
		cr->rte_frompattern=this;

		cr->fromtrack=track;

		track->song->realtimeevents[REALTIMELIST_MIDI].AddRealtimeAlarmEvent(cr,GetPatternStart(),GetPatternStart());
		}

		if(Program_Realtime *pr=new Program_Realtime)
		{		
		pr->flag=RealtimeEvent::RTE_MIDIPatternSTART_EVENT|RealtimeEvent::MIDIPatternSTART_EVENT;

		if(mainpattern)
		pr->rte_frompattern=(MIDIPattern *)mainpattern; // Clone use mainpattern BankSelect/ProgrChange
		else
		pr->rte_frompattern=this;

		pr->fromtrack=track;
		track->song->realtimeevents[REALTIMELIST_MIDI].AddRealtimeAlarmEvent(pr,GetPatternStart(),GetPatternStart());
		}
		*/
	}
}

bool MIDIPattern::SetNewLoopStart(OSTART pos)
{
	ostart=pos;
	return false;
}

void MIDIPattern::StopAllofPattern()
{
	if(track && track->song)
		mainMIDI->StopAllofPattern(track->song,this);
}

// MemPools

Note *MIDIPattern::NewNote(OSTART start,OSTART staticstart)
{

#ifdef MEMPOOLS
	Note *e=mainpools->mempGetNote();
#else
	Note *e=new Note;
#endif

	if(e){
		e->staticostart=staticstart;
		e->pattern=this;

		events.AddOSort(e,start); // sort
		//notes.AddOSort(&e->list_note,start);

		return e;
	}

	return 0;
}
Note *MIDIPattern::NewNote(OSTART start)
{
#ifdef MEMPOOLS
	Note *e=mainpools->mempGetNote();
#else
	Note *e=new Note;
#endif

	if(e){
		e->staticostart=start;
		e->pattern=this;
		events.AddOSort(e,start); // sort
		return e;
	}

	return 0;
}

ControlChange *MIDIPattern::NewControlChange(OSTART start)
{
#ifdef MEMPOOLS
	ControlChange *e=mainpools->mempGetControl();
#else
	ControlChange *e=new ControlChange;
#endif

	if(e){
		e->staticostart=start;
		e->pattern=this;	
		events.AddOSort(e,start); // sort
	}

	return e;
}

Pitchbend *MIDIPattern::NewPitchbend(OSTART start)
{
#ifdef MEMPOOLS
	Pitchbend *e=mainpools->mempGetPitchbend();
#else
	Pitchbend *e=new Pitchbend;
#endif

	if(e){
		e->staticostart=start;
		e->pattern=this;	
		events.AddOSort(e,start); // sort
	}

	return e;
}

PolyPressure *MIDIPattern::NewPolyPressure(OSTART start)
{
	if(PolyPressure *e=new PolyPressure){
		e->staticostart=start;
		e->pattern=this;	
		events.AddOSort(e,start); // sort
		return e;
	}

	return 0;
}

ProgramChange *MIDIPattern::NewProgramChange(OSTART start)
{
	if(ProgramChange *e=new ProgramChange){
		e->staticostart=start;
		e->pattern=this;	
		events.AddOSort(e,start); // sort
		return e;
	}

	return 0;
}

ChannelPressure *MIDIPattern::NewChannelPressure(OSTART start)
{
	if(ChannelPressure *e=new ChannelPressure){
		e->staticostart=start;
		e->pattern=this;	
		events.AddOSort(e,start); // sort
		return e;
	}

	return 0;
}

SysEx *MIDIPattern::NewSysEx(OSTART start)
{
	if(SysEx *e=new SysEx){
		e->staticostart=start;
		e->pattern=this;	
		events.AddOSort(e,start); // sort
		return e;
	}

	return 0;
}

int MIDIPattern::GetCountOfSysEx()
{
	int c=0;

	Seq_Event *e=FirstEvent();

	while(e){
		if(e->GetStatus()==SYSEX)c++;
		e=e->NextEvent();
	}

	return c;
}

int MIDIPattern::GetCountOfEvents(MIDIFilter *f)
{
	int c=0;
	Seq_Event *e=FirstEvent();

	while(e){
		if(e->CheckFilter(f))c++;
		e=e->NextEvent();
	}

	return c;
}

Seq_Event *MIDIPattern::FirstEvent()
{
	if(mainpattern) // Loop ?
	{
		if(mainpattern->mainclonepattern)
			return (Seq_Event *)((MIDIPattern *)mainpattern->mainclonepattern)->events.GetRoot();

		return (Seq_Event *)((MIDIPattern *)mainpattern)->events.GetRoot();
	}

	return (Seq_Event *)events.GetRoot();
}

Seq_Event *MIDIPattern::FirstVirtualEvent()
{
	if(mainpattern) // Loop ?
	{
		if(mainpattern->mainclonepattern)
			return (Seq_Event *)((MIDIPattern *)mainpattern->mainclonepattern)->virtualevents.GetRoot();

		return (Seq_Event *)((MIDIPattern *)mainpattern)->virtualevents.GetRoot();
	}

	return (Seq_Event *)virtualevents.GetRoot();
}

Seq_Event *MIDIPattern::LastEvent()
{
	if(mainpattern) // Loop ?
	{
		if(mainpattern->mainclonepattern)
			return (Seq_Event *)((MIDIPattern *)mainpattern->mainclonepattern)->events.Getc_end();	

		return (Seq_Event *)((MIDIPattern *)mainpattern)->events.Getc_end();	
	}

	return (Seq_Event *)events.Getc_end();
}

Seq_Event *MIDIPattern::LastVirtualEvent()
{
	if(mainpattern) // Loop ?
	{
		if(mainpattern->mainclonepattern)
			return (Seq_Event *)((MIDIPattern *)mainpattern->mainclonepattern)->virtualevents.Getc_end();

		return (Seq_Event *)((MIDIPattern *)mainpattern)->virtualevents.Getc_end();	
	}

	return (Seq_Event *)virtualevents.Getc_end();
}

void MIDIPattern::InitSwing()
{
	QuantizeEffect *qe=&track->t_trackeffects.quantizeeffect;

	if(qe->usehuman==false)
	{
		// Reset Swing
		Seq_MIDIChainEvent *n=firstchainevent;

		while(n){
			n->realtimeswing=0;
			n=n->next_chainevent;
		}

		return;
	}

	double hrangefac=qe->humanrange;
	hrangefac/=100;

	double humanq;

	if(qe->humanq==0) // 0=Track Human Q
		humanq=(double)quantlist[qe->quantize];
	else
		humanq=qe->humanq;

	srand(time(NULL));              /* Zufallsgenerator initialisieren */

	OSTART prevstart=-1;
	Seq_MIDIChainEvent *n=firstchainevent;

	while(n)
	{
		double h=rand()%100; // 0-100

		h*=0.01; // 0-1
		h*=humanq;

		//	TRACE ("RND %f\n",h);

		h*=hrangefac;

		OSTART swing=(OSTART)floor(h+0.5);

		// << 0,1 or 2,3 >> ?
		int lr=rand()%4;

		//	TRACE ("LR %d\n",lr);

		if(lr<2)
			swing=-swing;

		n->realtimeswing=0;
		OSTART p=n->GetPlaybackStart(this,track);

		if(prevstart!=-1 && p+swing<prevstart){// Check < Prev Event
			swing=prevstart-p;
			TRACE ("PCon %d\n",swing);
		}

		if(p+swing<0) // <0 Correction
			swing=-p;

		n->realtimeswing=swing;

#ifdef DEBUG
		if(prevstart!=-1 && p+swing<prevstart)
			maingui->MessageBoxError(0,"PS <0");
#endif

		prevstart=p+swing;

		n=n->next_chainevent;
	}

#ifdef DEBUG
	{
		OSTART ph=-1;
		Seq_MIDIChainEvent *n=firstchainevent;

		while(n)
		{
			OSTART p=n->GetPlaybackStart(this,track);

			if(p<0)
				maingui->MessageBoxError(0,"Note Chain Swing <0");

			if(ph!=-1)
			{
				if(p<ph)
					maingui->MessageBoxError(0,"Note Chain Swing <Prev");
			}

			ph=p;

			n=n->next_chainevent;
		}
	}
#endif
}

void MIDIPattern::SkipPlaybackMIDIEvents(int index,int mode) // Skip Notes - use Note Chain List < --- --- --- >
{
	//	TRACE ("SkipPlaybackMIDIEvents Index %d Mode %d\n",index,mode);

	while(playback_MIDIevent[index][mode] && playback_MIDIevent[index][mode]->IsSwingAble()==true)
		playback_MIDIevent[index][mode]=playback_MIDIevent[index][mode]->NextEvent();

	//	TRACE ("PlayList after Skip Notes %d\n",playback_MIDIevent[index][mode]);
}

bool MIDIPattern::InitPlayback(InitPlay *init,int mode) // Init Pattern and Send Reset Events - virtual
{		
	OSTART fposition=init->position;

	// Delay ?
	if(init->delay<0)
	{
		// <<< Seek
		fposition+=-init->delay;
	}

	// Realtime Swing Init
	if(init->initindex==0 && mode==0)
	{
		InitSwing();
	}

	playback_MIDIevent[init->initindex][mode]=FindEventAtPosition(fposition); // Seq_Event

	{ // Find and Set Next Playback Note in Chain List

		Seq_Event *nn=playback_MIDIevent[init->initindex][mode];

		while(nn){

			if(nn->IsSwingAble()==true){

				playback_nextchainevent[init->initindex][mode]=(Seq_MIDIChainEvent *)nn; // | --- > Next swingable Event
				//	TRACE (" >>> Found Chain List <<< playback_nextchainevent Index= %d Mode =%d \n",init->initindex,mode);
				goto gogo;
			}

			nn=nn->NextEvent();
		}

		playback_nextchainevent[init->initindex][mode]=0; // No Chain List Notes

		//TRACE (">>> NO Init Chain List playback_nextchainevent Index= %d Mode =%d \n",init->initindex,mode);
	}

gogo:

	SkipPlaybackMIDIEvents(init->initindex,mode);

	// Pattern Bank Select/Program Change
	if(GetMIDIFX()->MIDIbank_progselectsend==false)
	{
		GetMIDIFX()->MIDIbank_progselectsend=true; // Set Send Flag
		AddBankProgRealtime(init->position,false);
	}

	return true;
}