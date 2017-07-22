#include "object_track.h"
#include "objectpattern.h"
#include "objectevent.h"
#include "gui.h"
#include "MIDIhardware.h"
#include "MIDIPattern.h"
#include "songmain.h"
#include "MIDIprocessor.h"
#include "object_song.h"
#include "camxfile.h"

OSTART Seq_Event::GetPlaybackStart(MIDIPattern *p,Seq_Track *t)
{
	OSTART h=GetEventStart(p);
	h+=t->GetFX()->GetDelay();
	return h;
}

bool Seq_Event::CheckIfPlaybackIsAble()
{
	if(mainMIDI->sendcontrolsolomute==true)
		return false;

	return true;
}

bool Seq_Event::QuantizeEvent(QuantizeEffect *fx)
{
	return fx->QuantizeEvent(this);
}

void Seq_MIDIEvent::DeleteFromPattern()
{
	GetMIDIPattern()->events.CutQObject(this); // Remove from Event list
}

Seq_Event *Seq_Event::CloneNewAndSortToPattern(Seq_Song *song,MIDIPattern *p,int flag)
{
	if(Seq_Event *e=(Seq_Event *)Clone(song))
	{
		p->AddSortEvent(e);
		return e;
	}

	return 0;
}

Seq_Event *Seq_Event::CloneNewAndSortToPattern(Seq_Song *song,MIDIPattern *p,OSTART diff,int flag)
{
	if(Seq_Event *e=(Seq_Event *)Clone(song))
	{
		e->staticostart+=diff;
		e->ostart+=diff;
		p->AddSortEvent(e);

		if(flag&CLONEFLAG_ERASECLONEDATA)
			e->EraseCloneData(); // ICD Drum etc.

		return e;
	}

	return 0;	
}

void Seq_Event::AddSortToPattern(MIDIPattern *p,OSTART start)
{
	staticostart=start;
	p->AddSortEvent(this,start);
}

bool Seq_MIDIEvent::CheckFilter(int fflag,int icdtype)
{
	return true;
}

Seq_Track *Seq_Event::GetTrack()
{
	return pattern->track;
}

void Seq_Event::AddSortToPattern(MIDIPattern *p)
{
	p->AddSortEvent(this,ostart);
}

void Seq_Event::SetStartStatic(OSTART newpos)
{
	ostart=staticostart=newpos;
}

bool Seq_Event::SelectEvent(bool select,bool guirefresh)
{
	if((select==true && (!(flag&OFLAG_SELECTED))) ||
		(select==false && (flag&OFLAG_SELECTED))
		)
	{
		if(select==true)
			Select();
		else
			UnSelect();

		return true;
		// Refresh GUI Events
		//if(guirefresh==true)
		//	maingui->RefreshAllEditorsWithEvent(GetTrack()->song,this);
	}

	return false;
}

OSTART Seq_Event::GetEventStart(Seq_Pattern *p){return p?ostart+p->offset:ostart;} // Loop

LONGLONG Seq_Event::GetSampleStart(Seq_Song *song,Seq_Pattern *p)
{
	return song->timetrack.ConvertTicksToTempoSamples(GetEventStart(p));
}

LONGLONG Seq_Event::GetSampleEnd(Seq_Song *song,Seq_Pattern *p)
{
	return song->timetrack.ConvertTicksToTempoSamples(GetEventStart(p));
}

QuantizeEffect *Seq_Event::GetQuantizer()
{
	if((!pattern) || (!pattern->track))
		return 0;

	return &pattern->track->GetFX()->quantizeeffect; // dummy -> track
}

void Seq_Event::SetStatus(UBYTE s)
{
	if(s<0xF0){
		status&=0x0F; // keep channel
		status|=s;
	}
	else
		status=s; // SysEx...
}

void Note::DeleteFromPattern()
{
	GetMIDIPattern()->events.CutQObject(this); // Remove from Event list
	GetMIDIPattern()->DeleteVirtual(&off);
	//GetMIDIPattern()->notes.CutQObject(&list_note);
}

Object *Note::Clone(Seq_Song *song)
{
	#ifdef MEMPOOLS
	if(pool){
		if(Note *n=pool->mainpool->mempGetNote()){
			CloneData(song,n);
			return n;
		}
	}
	else
#endif

	{
		if(Note *n=new Note){
			CloneData(song,n);
			return n;
		}
	}

	return 0;
}

void Note::Delete(bool full)
{
	#ifdef MEMPOOLS
	if(pool)
		pool->mainpool->mempDeleteNote(this);
	else
#endif

		delete this;
}

LONGLONG Note::GetSampleSize(Seq_Song *song,Seq_Pattern *p)
{
	return GetSampleEnd(song,p)-GetSampleStart(song,p);
}

LONGLONG Note::GetSampleEnd(Seq_Song *song,Seq_Pattern *p)
{
	return song->timetrack.ConvertTicksToTempoSamples(off.GetEventStart(p));
}

void Note::CloneData(Seq_Song *song,Seq_Event *e)
{
	Note *to=(Note *)e;

	to->status=status;

	to->key=key;
	to->velocity=velocity;
	to->velocityoff=velocityoff;

	to->ostart=ostart;
	to->staticostart=staticostart;
	//to->sampleposition=sampleposition;

	to->off.staticostart=off.staticostart;
	to->off.ostart=off.ostart;
	//to->off.sampleposition=off.sampleposition;
}

void Note::AddSortToPattern(MIDIPattern *p,OSTART start)
{
	off.pattern=pattern=p;
	OSTART length=GetNoteLength();

	ostart=staticostart=start;
	off.ostart=off.staticostart=start+length;

	if(QuantizeEffect *fx=GetQuantizer())
	{
		olist=0; // Reset Quantize - skip move <>
		fx->QuantizeNote(this); // No Move
	}

	AddSortToPattern(p);
}

void Note::AddSortToPattern(MIDIPattern *p)
{
	p->AddSortVirtual(&off,off.ostart);
	p->AddSortEvent(this);
	//p->AddSortNote(&list_note,ostart);
}

void Note::SetStartStatic(OSTART newpos)
{
	staticostart=ostart=newpos;
	off.ostart=off.staticostart=newpos+GetNoteLength();
}

void Note::SetEnd(OSTART newpos)
{
	if(newpos>=ostart && newpos!=off.ostart)
	{
		if(off.GetList())
			off.GetList()->MoveO(&off,newpos);
		else
			off.ostart=newpos;

		off.staticostart=newpos;
	}
}

void Note::MoveEvent(OSTART diff)
{	
	off.staticostart+=diff;
	staticostart+=diff;

	off.GetList()->MoveO(&off,off.ostart+diff); // Move Off
	GetList()->MoveO(this,ostart+diff); // Move Note

//	list_note.GetList()->MoveO(&list_note,ostart+diff);
}

void Note::MoveEventQuick(OSTART diff)
{
	off.staticostart+=diff;
	staticostart+=diff;

	ostart+=diff;
	off.ostart+=diff;

//	list_note.ostart=ostart;
}

void Note::MoveNote(OSTART startposition,OSTART endposition)
{
	if(ostart!=startposition)
	{
		GetList()->MoveO(this,startposition);
		//list_note.GetList()->MoveO(&list_note,startposition);
	}

	if(off.ostart!=endposition)
		off.GetList()->MoveO(&off,endposition);
}

void Note::DoEventProcessor(MIDIProcessor *proc)
{
	int h=key;

	if(proc->trackfx)
		h+=proc->trackfx->GetTranspose();

	if(proc->patternfx)
	{
		h+=proc->patternfx->GetTranspose();
	
		if(pattern && pattern->itsaloop==true)
		{
			MIDIPattern *mp=(MIDIPattern *)pattern;

			h+=mp->t_MIDIeffects.GetTranspose();
		}

	}

	if(h<0)key=0;
	else
		if(h>127)key=127;
		else
			key=(char)h;

	// Velocity
	h=velocity;

	if(proc->trackfx)
		h+=proc->trackfx->GetVelocity();

	if(proc->patternfx)
	{
		h+=proc->patternfx->GetVelocity();

		if(pattern && pattern->itsaloop==true)
		{
			MIDIPattern *mp=(MIDIPattern *)pattern;

			h+=mp->t_MIDIeffects.GetVelocity();
		}
	}

	// Add Master Velocity
	h+=proc->song->audiosystem.masterchannel.MIDIfx.velocity.GetVelocity();

	if(h<1)
		velocity=1; // 0== 0ff
	else
		if(h>127)
			velocity=127;
		else
			velocity=(char)h;		
}

bool Note::CheckSelectFlag(int checkflag,int icdtype)
{
	return ( (checkflag&SEL_NOTEON) && ((!(checkflag&SEL_SELECTED)) || (flag&OFLAG_SELECTED)) )?true:false;
}

OSTART Note::GetPlaybackStart(MIDIPattern *p,Seq_Track *t)
{
	OSTART h=GetEventStart(p);
	h+=t->GetFX()->GetDelay();
	
	h+=realtimeswing; // Add Swing

	return h;
}

bool Note::QuantizeEvent(QuantizeEffect *fx)
{
	return fx->QuantizeNote(this);
}

Seq_Event *Note::CloneNewAndSortToPattern(Seq_Song *song,MIDIPattern *p,int flag)
{
	if(Note *newnote=(Note *)Clone(0))
	{
		p->AddSortVirtual(&newnote->off);
		p->AddSortEvent(newnote);

		return newnote;
	}

	return 0;
}

Seq_Event *Note::CloneNewAndSortToPattern(Seq_Song *song,MIDIPattern *p,OSTART diff,int flag)
{
	if(Note *newnote=(Note *)Clone(0))
	{
		newnote->ostart+=diff;
		newnote->off.ostart+=diff;

		newnote->staticostart+=diff;
		newnote->off.staticostart+=diff;

		p->AddSortVirtual(&newnote->off);
		p->AddSortEvent(newnote);
		
		return newnote;
	}

	return 0;
}

Seq_Event *SysEx::CloneNewAndSortToPattern(Seq_Song *song,MIDIPattern *p,int flag)
{
	if(SysEx *newsys=(SysEx *)Clone(0))
	{
		p->AddSortEvent(newsys);

		/*
		if(sysexend.pattern)
		{
			p->AddSortVirtual(&newsys->sysexend);
			newsys->sysexend.pattern=p;
		}
*/

		return newsys;
	}

	return 0;
}

Seq_Event *SysEx::CloneNewAndSortToPattern(Seq_Song *song,MIDIPattern *p,OSTART diff,int flag)
{
	if(SysEx *newsys=(SysEx *)Clone(0))
	{
		newsys->ostart+=diff;
		//newsys->sysexend.ostart+=diff;

		newsys->staticostart+=diff;
		//newsys->sysexend.staticostart+=diff;

		p->AddSortEvent(newsys);

		/*
		if(sysexend.pattern)
		{
			p->AddSortVirtual(&newsys->sysexend);
			newsys->sysexend.pattern=p;
		}
		*/

		return newsys;
	}

	return 0;
}


#ifdef OLDSYS
void SysEx::DeleteFromPattern()
{
	GetMIDIPattern()->events.CutObject(this); // Remove from Event list

	/*
	if(sysexend.pattern)
	{
		GetMIDIPattern()->DeleteVirtual(&sysexend);
		sysexend.pattern=0;
	}
	*/
}

void SysEx::CalcSysTicks()
{
	if(data && length && mainMIDI->baudrate)
	{
		double h=mainMIDI->baudrate/10; // - start/stop
		double h2=length;

		// start/stopbits ->bytes/sec
		h2/=h;

		// h2=ms
		dataticklength=mainvar->ConvertMilliSecToTicks(h2);
	}
	else
		dataticklength=0;
}

void SysEx::CheckSysExEnd()
{
	if(GetMIDIPattern())
		GetMIDIPattern()->AddSortVirtual(&sysexend,ostart+dataticklength);

	/*
	else
	if(sysexend.pattern)
	{
	pattern->DeleteVirtual(&sysexend);
	sysexend.pattern=0;
	}
	*/
}
#endif

Object *ControlChange::Clone(Seq_Song *song)
{
	#ifdef MEMPOOLS
	if(pool)
	{
		if(ControlChange *p=pool->mainpool->mempGetControl())
		{
			CloneData(song,p);
			return p;
		}
	}
	else
#endif

	{
		if(ControlChange *p=new ControlChange)
		{
			CloneData(song,p);
			return p;
		}
	}

	return 0;
}

void ControlChange::Delete(bool full)
{
	#ifdef MEMPOOLS
	if(pool)
		pool->mainpool->mempDeleteControl(this);
	else
#endif

		delete this;
}

bool ProgramChange::ChangeInfo(char *i)
{
	if((!i) && info)
	{
		delete info;
		info=0;

		return true;
	}

	if(i && ((!info) || (info && strcmp(i,info)!=0)))
	{
		if(info)
			delete info;
		info=mainvar->GenerateString(i);

		return true;
	}

	return false;
}

bool ProgramChange::Compare(Seq_Event *e)
{
	return (e->status==status && ((ProgramChange *)e)->program==program)?true:false;
}

void ProgramChange::Load(camxFile *file)
{
	file->ReadChunk(&program);
	file->Read_ChunkString(&info);
}

void ProgramChange::Save(camxFile *file)
{
	file->Save_Chunk(program);
	file->Save_ChunkString(info);
}

Object *ProgramChange::Clone(Seq_Song *song)
{
	if(ProgramChange *p=new ProgramChange){
		CloneData(song,p);
		return p;
	}

	return 0;
}

bool ProgramChange::CheckSelectFlag(int checkflag,int icdtype)
{
	return ( (checkflag&SEL_PROGRAMCHANGE) && ((!(checkflag&SEL_SELECTED)) || (flag&OFLAG_SELECTED)))?true:false;
}

void ProgramChange::Delete(bool full)
{
	if(info)delete info; // Info String
	delete this;
}

void ProgramChange::CloneData(Seq_Song *song,Seq_Event *e)
{
	ProgramChange *to=(ProgramChange *)e;

	to->status=status;
	to->program=program;

	to->ostart=ostart;
	to->staticostart=staticostart;
	//to->sampleposition=sampleposition;
}

Object* Pitchbend::Clone(Seq_Song *song)
{
	#ifdef MEMPOOLS
	if(pool)
	{
		if(Pitchbend *p=pool->mainpool->mempGetPitchbend())
		{
			CloneData(song,p);
			return p;
		}
	}
	else
#endif

	{
		if(Pitchbend *p=new Pitchbend)
		{
			CloneData(song,p);
			return p;
		}
	}

	return 0;
}

void Pitchbend::Delete(bool full)
{
	#ifdef MEMPOOLS
	if(pool)
		pool->mainpool->mempDeletePitchbend(this);
	else
#endif

		delete this;
}

bool Pitchbend::Compare(Seq_Event *e)
{
	return(e->status==status && ((Pitchbend *)e)->msb==msb && ((Pitchbend *)e)->lsb==lsb)?true:false;
}

void Pitchbend::Load(camxFile *file)
{
	file->ReadChunk(&lsb);
	file->ReadChunk(&msb);
}

void Pitchbend::Save(camxFile *file)
{
	file->Save_Chunk(lsb);
	file->Save_Chunk(msb);
}

int Pitchbend::GetPitch()
{
	int i=128*msb;

	i+=lsb;
	i-=8192;

	return i;
}

void Pitchbend::SetPitchbend(int cpitch)
{
	cpitch+=8192;

	msb=cpitch/128;
	lsb=cpitch-(msb*128);
}

bool Pitchbend::CheckSelectFlag(int checkflag,int icdtype)
{
	return ((checkflag&SEL_PITCHBEND) && ((!(checkflag&SEL_SELECTED)) || (flag&OFLAG_SELECTED)))?true:false;
}

void Pitchbend::CloneData(Seq_Song *song,Seq_Event *e)
{
	Pitchbend *to=(Pitchbend *)e;

	to->status=status;
	to->lsb=lsb;
	to->msb=msb;

	to->ostart=ostart;
	to->staticostart=staticostart;
	//to->sampleposition=sampleposition;
}

SysEx::SysEx()
{
	id=OBJ_SYSEX;
	status=SYSEX;
	data=0;
	length=0;
	//sysexend.pattern=0;
	//dataticklength=0;
}

void SysEx::Load(camxFile *file)
{
	file->ReadChunk(&length);

	if(length>0)
	{
		if(data=new UBYTE[length])
		{
			file->ReadChunk(data,length);
			//CalcSysTicks();
		}
	}
}

void SysEx::Save(camxFile *file)
{
	if(data)
	{
		file->Save_Chunk(length); // 0 or more
		file->Save_Chunk(data,length);
	}
	else
	{
		int savelength=0;
		file->Save_Chunk(savelength); // 0
	}
}

#ifdef OLDSYS
void SysEx::MoveEvent(OSTART diff)
{	
	staticostart+=diff;

	GetList()->MoveO(this,ostart+diff);

//	if(sysexend.pattern)
//		sysexend.GetList()->MoveO(&sysexend,sysexend.ostart+diff);
}

void SysEx::MoveEventQuick(OSTART diff)
{
	staticostart+=diff;
	ostart+=diff;
	//sysexend.ostart+=diff;
}
#endif

bool SysEx::CheckSelectFlag(int checkflag,int icdtype)
{
	return((checkflag&SEL_SYSEX) && ((!(checkflag&SEL_SELECTED)) || (flag&OFLAG_SELECTED)))?true:false;
}

Object *SysEx::Clone(Seq_Song *song)
{
	if(data && length){
		if(SysEx *n=new SysEx){
			CloneData(song,n);
			return n;
		}
	}

	return 0;
}

void SysEx::CloneData(Seq_Song *song,Seq_Event *e)
{
	SysEx *sys=(SysEx *)e;

	e->ostart=ostart;
	e->staticostart=staticostart;
	//e->sampleposition=sampleposition;

	//sys->sysexend.ostart=sysexend.ostart;
	//sys->sysexend.staticostart=sysexend.staticostart;
	//sys->sysexend.sampleposition=sysexend.sampleposition;

	if(data && length>0)
	{
		if(sys->data=new UBYTE[length])
		{
			memcpy(sys->data,data,sys->length=length);
	//		sys->dataticklength=dataticklength;	
		}
	}
}

void SysEx::Delete(bool full)
{
	if(data && length)
		delete data;
	
	delete this;
}