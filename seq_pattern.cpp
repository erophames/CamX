#include "songmain.h"
#include "audiofile.h"
#include "MIDIhardware.h"
#include "object_song.h"
#include "object_track.h"
#include "objectpattern.h"
#include "arrangeeditor.h"
#include "MIDIfile.h"
#include "quantizeeditor.h"
#include "languagefiles.h"
#include "gui.h"
#include "MIDIPattern.h"
#include "semapores.h"
#include "crossfade.h"
#include "audiopattern.h"
#include "chunks.h"
#include "MIDIprocessor.h"
#include "audiodevice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef WIN32
CCriticalSection sema_patternid;
#endif
static int patternid=0;

static int GetPatternID()
{
	int r;

	sema_patternid.Lock();
	r=patternid++;
	sema_patternid.Unlock();

	return r;
}

Seq_Pattern::Seq_Pattern() 
{
	loopflag=PATTERNLOOP_BEAT;

	flag=0;
	patternname=0;
	track=0;

	ResetLoops();

	offset=0;

	mainpattern= // for loops
	mainclonepattern=0; // for clones

	//	folder=0;
	track=0;
	mediatype=0; // no media

	quantizeeffect.pattern=this;
	recordcounter=-1;
	link=0; // Pattern Link

	// false
	recordeventsadded=
		itsaloop=
		p_muteflag=
		useoffsetregion=
		itsaclone=
		recordpattern=
		mute=false;

	//true
	visible=
		realpattern=true;

	offsetstartoffset=0;
	loopindex=-1;
	patternID=GetPatternID();
	loadoffset=-1;
}

Seq_Pattern *Seq_Pattern::NextRealPattern()
{
	Seq_Pattern *c=NextPattern();

	while(c){

		if(c->itsaloop==false)
			return c;
		//if(!c->mainpattern)return c;
		c=c->NextPattern();
	}

	return 0;
}

Seq_Pattern *Seq_Pattern::NextPattern(int findtype)
{
	bool checkloops;

	if(findtype&MEDIATYPE_NOLOOPS)
	{
		checkloops=false;
		findtype CLEARBIT MEDIATYPE_NOLOOPS;
	}
	else
		checkloops=true;

	Seq_Pattern *n=(Seq_Pattern *)next;

	while(n)
	{	
		if((findtype&n->mediatype) &&
			((checkloops==true || n->mainpattern==0) || n->mainclonepattern) // no loops
			)
			return n;

		n=(Seq_Pattern *)n->next;
	}

	return 0;
}

Seq_Pattern *Seq_Pattern::PrevPattern(int findtype)
{
	bool checkloops;

	if(findtype&MEDIATYPE_NOLOOPS)
	{
		checkloops=false;
		findtype CLEARBIT MEDIATYPE_NOLOOPS;
	}
	else
		checkloops=true;

	Seq_Pattern *p=(Seq_Pattern *)prev;

	while(p)
	{	
		if((findtype&p->mediatype) &&
			((checkloops==true || p->mainpattern==0) || p->mainclonepattern) // no loops
			)
			return p;

		p=(Seq_Pattern *)p->prev;
	}

	return 0;
}

void Seq_Pattern::MovePattern(OSTART diff,int flag)
{
	if(diff)
	{	
		GetList()->MoveO(this,GetPatternStart()+diff); // Move Header

		MovePatternData(diff,flag);

		if(mainpattern)
		{
			if(mainclonepattern)SetOffset();
		}
		else
			SetClonesOffset();

		LoopPattern();
	}
}

void Seq_Pattern::SetName(char *newname)
{
	if(patternname)
	{
		delete patternname;
		patternname=0;
	}

	if(mainclonepattern)
		return;

	if(newname)
	{
		if(!(patternname=mainvar->ClearString(newname)))
			patternname=mainvar->GenerateString(newname);
	}
	else
		patternname=mainvar->GenerateString("");
}

char *Seq_Pattern::GetName()
{
	if(mainclonepattern)return mainclonepattern->GetName();
	if(!patternname)return "_";

	return patternname;
}

bool Seq_Pattern::CheckIfInRange(OSTART cstart,OSTART cend,bool quantmeasure)
{
	OSTART patternstart=GetPatternStart(),patternend=GetPatternEnd();

	if(quantmeasure==true)
	{
		patternstart=GetTrack()->song->timetrack.ConvertTicksToMeasureTicks(patternstart,false);	
		patternend=GetTrack()->song->timetrack.ConvertTicksToMeasureTicks(patternend,true);
	}

	if((patternstart<=cstart && patternend>=cstart) || (patternstart>=cstart && patternstart<cend))
		return true;

	return false;
}

void Seq_Pattern::LoadStandardEffects(camxFile *file) // Quantize, Loops etc..
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_PATTERNFX)
	{
		file->ChunkFound();

		file->ReadChunk(&ostart);
		file->ReadChunk(&offset);

		file->Read_ChunkString(&patternname);

		file->ReadChunk(&mediatype);
		file->ReadChunk(&loopendless);
		file->ReadChunk(&loopwithloops);
		file->ReadChunk(&loops);
		file->ReadChunk(&mute);

		file->AddPointer((CPOINTER)&link); // Pointer to Pattern Link List
		file->AddPointer((CPOINTER)&mainclonepattern); // Clone ?

#ifdef _DEBUG
		if(mainclonepattern)
		{
			int i;

			i=1;
		}
#endif

		t_colour.LoadChunk(file);

		file->ReadChunk(&loopflag);

		file->CloseReadChunk();

		quantizeeffect.Load(file);
		volumecurve.Load(file);
	}
}

void Seq_Pattern::SaveStandardEffects(camxFile *file) // Quantize, Loops etc..
{
	file->OpenChunk(CHUNK_PATTERNFX);

	OSTART pstart=GetPatternStart();

	file->Save_Chunk(pstart);
	file->Save_Chunk(offset);

	file->Save_ChunkString(patternname);

	file->Save_Chunk(mediatype);
	file->Save_Chunk(loopendless);
	file->Save_Chunk(loopwithloops);
	file->Save_Chunk(loops);
	file->Save_Chunk(mute);

	file->Save_Chunk((CPOINTER)link); // Pointer to Pattern Link List

	// Clone ?
	file->Save_Chunk((CPOINTER)mainclonepattern);

	t_colour.SaveChunk(file);
	file->Save_Chunk(loopflag);

	file->CloseChunk();

	quantizeeffect.Save(file);
	volumecurve.Save(file);
}

void Seq_Pattern::ResetLoops()
{
	// Loops
	loopendless=loopwithloops=false;
	loops=1;
}

bool Seq_Pattern::CheckIfPatternOrFromClones(Seq_Pattern *p)
{
	if(p==this)return true;

	Seq_ClonePattern *cp=FirstClone();

	while(cp){
		if(cp->pattern==p)return true;
		cp=cp->NextClone();
	}

	return false;
}

bool Seq_Pattern::EditAble()
{
	/*
	if(mediatype==MEDIATYPE_AUDIO)
	{
	AudioPattern *ap=(AudioPattern *)this;

	if(!ap->audioevent.audioefile)return false;
	}
	*/

	if(mediatype==MEDIATYPE_AUDIO_RECORD)return false;

	if(itsaclone==false && mainpattern) // Loop
		return false;

	return true;
}

void Seq_Pattern::ShowPatternName(guiWindow *ex)
{
	guiWindow *win=maingui->FirstWindow();

	while(win)
	{
		if(win->WindowSong()==track->song)
		{
			switch(win->GetEditorID())
			{
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
						if(sp->pattern==this)
						{
							e->ShowSelectionPattern();
							break;
						}

						sp=sp->NextSelectedPattern();
					}
				}
				break;

			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)win;

					bool refresh=false;

					if(Edit_Arrange_Pattern *fp=ar->FindPattern(this))
						refresh=true;

					// Loops
					Seq_LoopPattern *lp=FirstLoopPattern();
					while(refresh==false && lp)
					{
						if(Edit_Arrange_Pattern *eap=ar->FindPattern(lp->pattern))
							refresh=true;

						lp=lp->NextLoop();
					}

					// Clones
					Seq_ClonePattern *scp=FirstClone();
					while(refresh==false && scp)
					{
						if(Edit_Arrange_Pattern *eap=ar->FindPattern(scp->pattern))
							refresh=true;

						scp=scp->NextClone();
					}

					if(refresh==true)
						ar->DrawDBBlit(ar->pattern);
				}
				break;

			case EDITORTYPE_QUANTIZEEDITOR:
				{
					Edit_QuantizeEditor *ed=(Edit_QuantizeEditor *)win;

					if(ed->effect->pattern==this)
						ed->ShowTitle();
				}

				break;

			}// switch

		}

		win=win->NextWindow();
	}
}

void Seq_Pattern::LoadFromFile(char *filename)
{
	camxFile read;

	char header[5];

	header[0]=header[4]=0;

	if(read.OpenRead(filename)==true)
	{
		read.Read(header,4);

		if(strcmp("CAMP",header)==0)
		{
			char type[5];

			type[0]=type[4]=0;

			read.Read(type,4);

			read.LoadChunk();
			read.ChunkFound();

			read.flag=LOAD_DONTSETSTARTPOSITION;

			loadoffset=GetPatternStart();

			Load(&read);
			Load_Ex(&read,0);
		}
	}

	read.Close(true);
}

void Seq_Pattern::SaveToMIDIFile(guiWindow *win)
{
	if(mediatype==MEDIATYPE_MIDI)
	{
		MIDIPattern *mp=(MIDIPattern *)this;

		if(mp->FirstEvent())
		{
			camxFile save;

			if (save.OpenFileRequester(0,win,Cxs[CXS_SAVEPATTERNASSMF],save.AllFiles(camxFile::FT_MIDIFILE),false,mp->GetName())==true)
			{					
				save.AddToFileName(".mid");
				MIDIFile MIDIfile;
				MIDIfile.SavePatternToFile((MIDIPattern *)this,save.filereqname);
				save.Close(true);
			}
		}
		else
			maingui->MessageBoxOk(0,Cxs[CXS_PATTERNEMPTY]);
	}
}

void Seq_Pattern::SaveToFile(guiWindow *win)
{
	camxFile save;

	if (save.OpenFileRequester(0,win,Cxs[CXS_SAVEPATTERN],"Pattern (*.pcmx)|*.pcmx;|All Files (*.*)|*.*||",false,GetName())==true)
	{					
		save.AddToFileName(".pcmx");

		if(save.OpenSave(save.filereqname)==true)
		{
			save.Save("CAMP",4);

			switch(mediatype)
			{
			case MEDIATYPE_MIDI:
				save.Save("MIDI",4);
				break;

			case MEDIATYPE_AUDIO:
				save.Save("AUDI",4);
				break;

			default:
				save.Save("0000",4);
			}

			Save(&save);
			Save_Ex(&save);
		}

		save.Close(true);
	}
}

Seq_LoopPattern *Seq_Pattern::GetLoop(int index)
{
	if(FirstLoopPattern())
		return (Seq_LoopPattern *)looppattern.GetO(index);

	return 0;
}

int Seq_Pattern::GetLoopIndex(Seq_Pattern *p)
{
	if(Seq_LoopPattern *f=FirstLoopPattern())
	{
		int i=0;

		while(f)
		{
			if(f->pattern==p)
				return i;

			i++;

			f=f->NextLoop();
		}	
	}

	return -1;
}

void Seq_Pattern::CloneLoops(Seq_Pattern *top)
{
	top->loops=loops;
	top->loopendless=loopendless;
	top->loopwithloops=loopwithloops;
	top->loopflag=loopflag; // Measure,Beat etc.
}

void Seq_Pattern::DeleteLoops()
{
	if(mainpattern==0 || mainclonepattern) // loops cant have loops
	{
		Seq_LoopPattern *l=(Seq_LoopPattern *)looppattern.GetRoot();

		while(l)
		{
			GetTrack()->DeletePattern(l->pattern,true);
			l=(Seq_LoopPattern *)looppattern.RemoveO(l);
		}
	}
}

void Seq_Pattern::SetOffset()
{
	if(mainclonepattern)
		offset=ostart-mainclonepattern->ostart;
	else
		if(mainpattern && mainpattern->mainclonepattern)
			offset=ostart-mainpattern->mainclonepattern->ostart;
		else
			offset=0;
}

void Seq_Pattern::SetClonesOffset()
{
	Seq_ClonePattern *l=FirstClone();

	while(l)
	{	
		l->pattern->offset=l->pattern->ostart-ostart;

		// Set Clones Loops Offset
		Seq_LoopPattern *sl=l->pattern->FirstLoopPattern();
		while(sl)
		{
			sl->pattern->offset=sl->pattern->ostart-ostart;
			sl=sl->NextLoop();
		}

		l=l->NextClone();
	}
}

void Seq_Pattern::AddClone(Seq_Pattern *p)
{
	if(Seq_ClonePattern *cp=new Seq_ClonePattern)
	{
		p->mainpattern=p->mainclonepattern=this;
		p->mainclonepatternID=patternID;

		p->itsaclone=true; // Set Clone Flag
		cp->pattern=p;

		clonepattern.AddEndO(cp);
	}
}

void Seq_Pattern::RemovePatternFromClones(Seq_Pattern *p)
{
	Seq_ClonePattern *l=FirstClone();

	while(l)
	{
		if(l->pattern==p)
			l=(Seq_ClonePattern *)clonepattern.RemoveO(l);
		else
			l=l->NextClone();
	}
}

void Seq_Pattern::CutClones()
{
	// Cut Clones
	Seq_ClonePattern *cl=FirstClone();

	while(cl)
	{
		cl->pattern->DeleteLoops();
		cl->pattern->track->CutPattern(cl->pattern,false);
		cl->insideundo=true;

		cl=cl->NextClone();
	}
}

Seq_ClonePattern *Seq_Pattern::DeleteClone(Seq_ClonePattern *clone,bool full)
{
	clone->pattern->DeleteLoops();

	if(full==true)
	{
		if(clone->insideundo==true)
			clone->pattern->Delete(true);
		else
			clone->pattern->track->DeletePattern(clone->pattern,true,false);
	}

	return(Seq_ClonePattern *)clonepattern.RemoveO(clone);
}

void Seq_Pattern::DeleteClones()
{
	if(!mainclonepattern) // clones/loops cant have clones
	{
		Seq_ClonePattern *l=FirstClone();

		while(l)
			l=DeleteClone(l,true);
	}
}

QuantizeEffect *Seq_Pattern::GetQuantizer()
{
	if(quantizeeffect.usegroove==true || quantizeeffect.usequantize==true) // Pattern use Quantize ?
		return &quantizeeffect;

	if(track->GetFX()->quantizeeffect.usegroove==true || track->GetFX()->quantizeeffect.usequantize==true)
		return &track->GetFX()->quantizeeffect;

	return 0;
}

int AudioPattern::RefreshLoopPositions()
{
	Seq_Time *timetrack=&GetTrack()->song->timetrack;
	int changedcounter=0;

	OSTART patternstart=GetPatternStart(),startpos=GetPatternEnd();

	Seq_LoopPattern *lp=FirstLoopPattern();

	while(lp)
	{
		startpos=timetrack->ConvertTicksToMeasureTicks(startpos,true); // Quantize to Measure >>

		if(startpos!=lp->loopstart)
		{
			AudioPattern *looppattern=(AudioPattern *)lp->pattern;

			lp->loopstart=startpos;
			looppattern->offset=startpos-patternstart;

			if(mainclonepattern)
				looppattern->offset+=offset;

			looppattern->audioevent.ostart=looppattern->ostart=startpos;

			changedcounter++;
		}

		lp=lp->NextLoop();
	}

	return changedcounter;
}

bool Seq_Pattern::CheckLoopPattern(int flag)
{

	if((loopendless==false && loopwithloops==false) && FirstLoopPattern())
		return true;

	if(loopendless==true || loopwithloops==true)
	{
		if(CanBeLooped()==false)
			return true;

		Seq_Time *timetrack=&GetTrack()->song->timetrack;
		OSTART startpos=GetPatternEnd(),end, patternstart=GetPatternStart(),startoffset,MIDIticklength;

		switch(loopflag)
		{
		case PATTERNLOOP_MEASURE:
			startoffset=patternstart-timetrack->ConvertTicksToMeasureTicks(patternstart,false);
			break;

		case PATTERNLOOP_BEAT:
			startoffset=patternstart-timetrack->ConvertTicksToBeatTicks(patternstart,false);
			break;

		case PATTERNLOOP_NOOFFSET:
			startoffset=0;
			break;
		}

		switch(mediatype)
		{
		case MEDIATYPE_MIDI:
			MIDIticklength=GetPatternEnd()-GetPatternStart();
			break;
		}

		if(Seq_Pattern *nextpattern=NextRealPattern())
			end=timetrack->ConvertTicksToMeasureTicks(nextpattern->GetPatternStart(),false); // Quantize to Measure <<
		else
			end=track->song->GetSongLength_Ticks();

		Seq_LoopPattern *lp=FirstLoopPattern();
		int i=0;

		while(loopendless==true || i<loops)
		{
			if(!lp)
				return true;

			switch(loopflag)
			{
			case PATTERNLOOP_MEASURE:
				startpos=timetrack->ConvertTicksToNextMeasureTicks(startpos); // Quantize to Measure >>
				break;

			case PATTERNLOOP_BEAT:
				startpos=timetrack->ConvertTicksToNextBeatTicks(startpos); // Quantize to Beat >>	
				break;

			case PATTERNLOOP_NOOFFSET:
				break;
			}

			if(startpos+startoffset!=lp->loopstart)
				return true;

			if(startpos<end)
			{
				switch(mediatype)
				{
				case MEDIATYPE_MIDI:
					startpos+=MIDIticklength;
					break;

				case MEDIATYPE_AUDIO:
					{
						AudioPattern *ap=(AudioPattern *)this;
						startpos=ap->GetTickEnd(startpos);
					}
					break;
				}

				i++;
				lp=lp->NextLoop();
			}
			else
				return true;
		}

		if(lp)
			return true;
	}

	return false;
}

void Seq_Pattern::LoopPattern(int flag)
{
	if(itsaloop==false && (loopendless==true || (loopwithloops==true && loops>0)))
	{
		if(CanBeLooped()==false)
		{
			DeleteLoops();
			return;
		}

		Seq_Time *timetrack=&GetTrack()->song->timetrack;
		OSTART patternstart=GetPatternStart(),startoffset,MIDIticklength;
		bool forceend;

		switch(mediatype)
		{
		case MEDIATYPE_MIDI:
			MIDIticklength=GetPatternEnd()-GetPatternStart();
			break;
		}

		switch(loopflag)
		{
		case PATTERNLOOP_MEASURE:
			startoffset=patternstart-timetrack->ConvertTicksToMeasureTicks(patternstart,false);
			break;

		case PATTERNLOOP_BEAT:
			startoffset=patternstart-timetrack->ConvertTicksToBeatTicks(patternstart,false);
			break;

		case PATTERNLOOP_NOOFFSET:
			{
				startoffset=0;
				if(GetPatternEnd()-GetPatternStart()<MINLOOPSIZE)
				{
					DeleteLoops();
					return;
				}

			}
			break;
		}

		// Check Existing Loops...
		if(Seq_LoopPattern *slp=FirstLoopPattern())
		{
			OSTART startpos=GetPatternEnd(),end;
			int i=0;
			bool nochanges=true;

			if(Seq_Pattern *nextpattern=NextPattern(MEDIATYPE_ALL|MEDIATYPE_NOLOOPS))
			{
				end=timetrack->ConvertTicksToMeasureTicks(nextpattern->GetPatternStart(),false); // Quantize to Measure <<
				forceend=true;
			}
			else
			{
				end=track->song->GetSongLength_Ticks();
				forceend=false;
			}

			while(loopendless==true || i<loops)
			{
				switch(loopflag)
				{
				case PATTERNLOOP_MEASURE:
					startpos=timetrack->ConvertTicksToNextMeasureTicks(startpos); // Quantize to Measure >>
					break;

				case PATTERNLOOP_BEAT:
					startpos=timetrack->ConvertTicksToNextBeatTicks(startpos); // Quantize to Beat >>	
					break;

				case PATTERNLOOP_NOOFFSET:
					break;
				}

				if((forceend==false && loopendless==false) || 
					startpos<end)
				{
					if(slp){

						bool changed=slp->pattern->SetNewLoopStart(slp->loopstart=startpos+startoffset);

						slp->pattern->offset=slp->loopstart-patternstart;
						if(mainclonepattern)slp->pattern->offset+=offset;

						if(changed==true)
							slp->pattern->StopAllofPattern();
					}
					else{
						nochanges=false;
						break;
					}

					switch(mediatype)
					{
					case MEDIATYPE_MIDI:
						startpos+=MIDIticklength;
						break;

					case MEDIATYPE_AUDIO:
						{
							AudioPattern *ap=(AudioPattern *)this;
							startpos=ap->GetTickEnd(startpos);
						}
						break;
					}

					i++;
				}
				else
				{
					// End
					if(slp)nochanges=false;
					break;
				}

				if(slp)slp=slp->NextLoop();
			}//while

			if((!slp) && nochanges==true)return;

			// Create new
			DeleteLoops();
		}

		OSTART startpos=GetPatternEnd(),end;
		int i=0;

		if(Seq_Pattern *nextpattern=NextPattern(MEDIATYPE_ALL))
		{
			end=timetrack->ConvertTicksToMeasureTicks(nextpattern->GetPatternStart(),false); // Quantize to Measure <<
			forceend=true;
		}
		else
		{
			if(Seq_Marker *mk=track->song->textandmarker.FindMarkerID(Seq_Marker::MARKERFUNC_STOPPLAYBACK))
			{
				end=mk->GetMarkerStart();
			}
			else
				end=track->song->GetSongLength_Ticks();

			forceend=false;
		}

		// Create new Loops
		char *loopname=new char[strlen(GetName())+32],nrs[NUMBERSTRINGLEN];
		if(!loopname)return;

		while(loopendless==true || i<loops)
		{
			switch(loopflag)
			{
			case PATTERNLOOP_MEASURE:
				startpos=timetrack->ConvertTicksToNextMeasureTicks(startpos); // Quantize to Measure >>
				break;

			case PATTERNLOOP_BEAT:
				startpos=timetrack->ConvertTicksToNextBeatTicks(startpos); // Quantize to Beat >>	
				break;

			case PATTERNLOOP_NOOFFSET:
				break;
			}

			OSTART endpos=startpos+startoffset;

			switch(mediatype)
			{
			case MEDIATYPE_MIDI:
				endpos+=MIDIticklength;
				break;

			case MEDIATYPE_AUDIO:
				{
					AudioPattern *ap=(AudioPattern *)this;
					endpos=ap->GetTickEnd(startpos);
				}
				break;
			}

			if(endpos>end)
				break;

			if((forceend==false && loopendless==false) || startpos+startoffset<end)
			{
				if(Seq_LoopPattern *lp=new Seq_LoopPattern)
				{
					if(Seq_Pattern *newpattern=CreateLoopPattern(i,startpos+startoffset,flag))
					{
						// Create Loopname
						if(loopname)
						{
							strcpy(loopname,"Loop:");
							mainvar->AddString(loopname,mainvar->ConvertIntToChar(i+1,nrs));
							mainvar->AddString(loopname," ");
							mainvar->AddString(loopname,GetName());
							newpattern->SetName(loopname);
						}

						newpattern->mainpattern=this; // Connect to mainpattern
						newpattern->itsaloop=true;
						newpattern->loopindex=lp->loopindex=i++;

						lp->pattern=newpattern;

						// Loop Start (Ticks)
						lp->loopstart=startpos+startoffset;

						newpattern->offset=lp->loopstart-patternstart;
						if(mainclonepattern)newpattern->offset+=offset;

						track->AddSortPattern(newpattern,lp->loopstart);

						switch(mediatype)
						{
						case MEDIATYPE_MIDI:
							startpos+=MIDIticklength;
							break;

						case MEDIATYPE_AUDIO:
							{
								AudioPattern *ap=(AudioPattern *)this;
								startpos=ap->GetTickEnd(startpos);
							}
							break;
						}

						looppattern.AddEndO(lp);
					}
					else
					{
						delete lp;
						break;
					}
				}
				else
					break;
			}
			else
				break;

		}//while

		delete loopname;
	}
	else
		DeleteLoops(); // No Loops
}

void Seq_Track::SendAllOpenOutputNotes()
{	
	NoteOff_Raw noteoff;

	while(OpenOutputNote *onote=FirstOpenOutputNote())
	{
		noteoff.status=NOTEOFF|(onote->status&0x0F);
		noteoff.key=onote->key;
		noteoff.velocityoff=0;
		noteoff.pattern=0;

		SendOutEvent_User(0,&noteoff,false);

		onote=DeleteOpenOutputNote(onote);
	}
}

void Seq_Track::DeleteAllOpenNotes()
{
	LockOpenOutputNotes();

	OpenOutputNote *onote=FirstOpenOutputNote();
	while(onote)onote=DeleteOpenOutputNote(onote);
	UnlockOpenOutputNotes();
}

void Seq_Track::DeletePRepairEvents(int mode)
{
	// NoteOffs
	if(mode&DELETE_PRepair_NOTEOFFS)
	{
		if(mode&DELETE_PRepair_SONGSTART)
		{
			Seq_Event *e=FirstPRepairNoteOff(0);

			while(e){	
				Seq_Event *b=e;
				e=(Seq_Event *)pRepareeventsnotes[0].CutObject(e);

				b->pattern=0;
				b->Delete(true);
			}
		}

		if(mode&DELETE_PRepair_CYCLE)
		{
			// Cycle
			Seq_Event *e=FirstPRepairNoteOff(1);

			while(e){
				Seq_Event *b=e;
				e=(Seq_Event *)pRepareeventsnotes[1].CutObject(e);

				b->pattern=0;
				b->Delete(true);
			}
		}
	}

	// Ctrls
	if(mode&DELETE_PRepair_CONTROL)
	{
		if(mode&DELETE_PRepair_SONGSTART)
		{
			Seq_Event *e=FirstPRepairCtrl(0);

			while(e)
			{
				Seq_Event *b=e;
				e=(Seq_Event *)pRepareeventsctrl[0].CutObject(e);

				b->pattern=0;
				b->Delete(true);
			}
		}

		if(mode&DELETE_PRepair_CYCLE)
		{
			// Cycle
			Seq_Event *e=FirstPRepairCtrl(true);

			while(e)
			{				
				Seq_Event *b=e;
				e=(Seq_Event *)pRepareeventsctrl[1].CutObject(e);

				b->pattern=0;
				b->Delete(true);
			}
		}
	}
}

void Seq_Song::SendAllOpenNotes()
{
	Seq_Track *track=FirstTrack();

	while(track)
	{
		track->LockOpenOutputNotes();
		track->SendAllOpenOutputNotes();
		track->UnlockOpenOutputNotes();

		track=track->NextTrack();
	}

	SendAllNoteOffs(true,0);

	// Send WEvents
	for(int i=0;i<REALTIME_LISTS;i++)
	{
		realtimeevents[i].Lock();
		RealtimeEvent *rte=realtimeevents[i].FirstWEvent();
		while(rte)
		{
			rte->SendQuick();
			rte=realtimeevents[i].DeleteWEvent(rte);
		}
		realtimeevents[i].UnLock();
	}	
}

void Seq_Song::SendPRepairNotes(bool cycle,int iflag)
{
	OSTART spos;
	LONGLONG samplestartpos;

	if(cycle==false)
	{
		if(mastering==true)
			spos=masteringposition;
		else
			spos=GetSongPosition();

		samplestartpos=timetrack.ConvertTicksToTempoSamples(spos);
	}
	else
	{
		spos=playbacksettings.cyclestart;
		samplestartpos=playbacksettings.cycle_samplestart;
	}

	int oflag=mastering==true?TRIGGER_MASTEREVENT:0;

	Seq_Track *t=FirstTrack();

	while(t)
	{
		bool sendaudio=t->CheckIfTrackIsAudioInstrument();

		if(mainMIDI->sendnoteprevcylce==true)
		{
			Seq_Event *e=t->FirstPRepairNoteOff(cycle==true?1:0);

			while(e)
			{
				// MIDI Output Processor

				MIDIProcessor processor(this,t);

				processor.EventInput((MIDIPattern *)e->pattern,e,spos); // ->> fill+create Processor Events

				if(Seq_Event *oevent=processor.FirstProcessorEvent())
				{
					bool cansendpattern=e->pattern->CheckIfPlaybackIsAble();
					bool cansendtrack=t->CheckIfPlaybackIsAble();

					while(oevent){

						if(oevent->CheckIfPlaybackIsAble()==false || // Check only Note == MUTE ?
							(cansendpattern==true && cansendtrack==true)
							)
						{
							oevent->SetMIDIImpulse(t);
							
							if(sendaudio==true)
							{
								if(iflag&SENDPRE_AUDIO)
								{
									Seq_Track *child=t;

									do
									{
										// Tracks
										InsertAudioEffect *ci=child->io.audioeffects.FirstActiveAudioEffect();

										while(ci) // Tracks AudioInstrument Loop ?
										{
											if(ci->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ci->audioeffect->plugin_on==true){

												// ---> Audio Trigger List
												oevent->SendToAudioPlayback(
													ci->audioeffect,
													samplestartpos,
													child,
													(MIDIPattern *)e->pattern,
													e,
													oflag,
													playback_sampleoffset,
													true
													);
											}

											ci=ci->NextActiveEffect();
										}

										child=(Seq_Track *)child->parent;

									}while(child);
								}

							}
							else
							{
								if(iflag&SENDPRE_MIDI)
								{
									if(mastering==false)
									{
										/*
										if(e->GetStatus()==NOTEON)
										{
										Note *note=(Note *)e;

										//note->off.ostart=note->off.staticostart=end;
										note->ostart=note->staticostart=spos;
										}
										*/
										if(e->GetMIDIPattern())
											SendMIDIOutPlaybackEvent(t,e->pattern?(MIDIPattern *)e->pattern:0,e,samplestartpos,Seq_Event::STD_CREATEREALEVENT|Seq_Event::STD_PRESTARTEVENT);
										else
											SendMIDIOutPlaybackEventNoEffects(t,0,e,Seq_Event::STD_CREATEREALEVENT);
									}
								}
							}
						}

						oevent=processor.DeleteEvent(oevent);	
					}// while proc events
				}

				e=e->NextEvent();
			}
		}

		if(cycle==false)
			t->DeletePRepairEvents(DELETE_PRepair_SONGSTART|DELETE_PRepair_NOTEOFFS);

		// dont delete cycle pRepare events
		t=t->NextTrack();
	}
}

void Seq_Song::SetSysExMIDIPattern(MIDIPattern *pattern,bool on)
{
	Seq_Track *t=FirstTrack();

	while(t)
	{
		MIDIPattern *mp=(MIDIPattern *)t->FirstPattern(MEDIATYPE_MIDI);

		while(mp)
		{
			if(mp->IsPatternSysExPattern()==true && 
				(mp==pattern  || (mp->IsSelected()==true && pattern->IsSelected()==true))
				)
			{
				mp->sendonlyatstartup=on;
			}

			mp=(MIDIPattern *)mp->NextPattern(MEDIATYPE_MIDI);
		}

		t=t->NextTrack();
	}
}

void Seq_Song::SendSysExStartupMIDIPattern()
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t)
	{
		MIDIPattern *mp=(MIDIPattern *)t->FirstPattern(MEDIATYPE_MIDI);

		while(mp)
		{
			if(mp->SendStartupSysEx()==true)
				c++;

			mp=(MIDIPattern *)mp->NextPattern(MEDIATYPE_MIDI);
		}

		t=t->NextTrack();
	}

	if(c)
	{
		char nr[NUMBERSTRINGLEN];
		char *h=mainvar->GenerateString(mainvar->ConvertIntToChar(c,nr)," ",Cxs[CXS_COUNTSYSEXSTARTPATTERN]);

		if(h)
		{
			maingui->MessageBoxOk(0,h);
			delete h;
		}
	}

}

void Seq_Song::SendPreStartPrograms()
{
	ProgramChange programchange;
	ControlChange bankselect_lsb,bankselect_msb;

	// 1. Bank Sel MSB
	// 2. Bank Sel LSB
	// 3. Program Change

	int oflag=mastering==true?TRIGGER_MASTEREVENT:0;

	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t->GetFX()->MIDIprogram.usebank_msb==true ||
			t->GetFX()->MIDIprogram.usebank_lsb==true ||
			t->GetFX()->MIDIprogram.useprogramchange==true)
		{

			// Bank Sel MSB
			if(t->GetFX()->MIDIprogram.usebank_msb==true)
			{
				bankselect_msb.status=CONTROLCHANGE;
				bankselect_msb.controller=0;
				bankselect_msb.value=t->GetFX()->MIDIprogram.MIDIBank_msb;
			}

			// Bank Sel LSB
			if(t->GetFX()->MIDIprogram.usebank_lsb==true)
			{
				bankselect_lsb.status=CONTROLCHANGE;
				bankselect_lsb.controller=0;
				bankselect_lsb.value=t->GetFX()->MIDIprogram.MIDIBank_lsb;
			}

			// Program Change
			if(t->GetFX()->MIDIprogram.useprogramchange==true)
			{
				programchange.status=PROGRAMCHANGE;
				programchange.program=t->GetFX()->MIDIprogram.MIDIProgram;
			}

			UBYTE channel;

			if(t->GetFX()->GetChannel()>0 && t->GetFX()->GetChannel()<=16) // Track Channel
				channel=t->GetFX()->GetChannel()-1;
			else
				channel=t->GetFX()->MIDIprogram.channel;

			programchange.status|=channel;
			bankselect_lsb.status|=channel;
			bankselect_msb.status|=channel;

			if(t->CheckIfTrackIsAudioInstrument())
			{
				if(t->GetFX()->MIDIprogram.usebank_msb==true)
					bankselect_msb.SendToAudio(0,&t->io.audioeffects,oflag,0,0);

				if(t->GetFX()->MIDIprogram.usebank_lsb==true)
					bankselect_lsb.SendToAudio(0,&t->io.audioeffects,oflag,0,0);

				if(t->GetFX()->MIDIprogram.useprogramchange==true)
					programchange.SendToAudio(0,&t->io.audioeffects,oflag,0,0);
			}
			else
			{
				if(t->GetFX()->MIDIprogram.usebank_msb==true)
					SendMIDIOutPlaybackEventNoEffects(t,0,&bankselect_msb,0);

				if(t->GetFX()->MIDIprogram.usebank_lsb==true)
					SendMIDIOutPlaybackEventNoEffects(t,0,&bankselect_lsb,0);

				if(t->GetFX()->MIDIprogram.useprogramchange==true)
					SendMIDIOutPlaybackEventNoEffects(t,0,&programchange,0);
			}

		}

		t=t->NextTrack();
	}
}

void Seq_Song::SendPRepairControl(bool cycle,int flag)
{
	OSTART spos;

	if(cycle==false)
	{
		if(mastering==true)
			spos=masteringposition;
		else
			spos=GetSongPosition();
	}
	else
		spos=playbacksettings.cyclestart;

	int oflag=mastering==true?TRIGGER_MASTEREVENT:0;

	Seq_Track *t=FirstTrack();
	while(t)
	{
		bool sendaudio=t->CheckIfTrackIsAudioInstrument();



		Seq_Event *e=t->FirstPRepairCtrl(cycle);
		while(e)
		{
			if(t->GetFX()->filter.CheckEvent(e)==true)
			{
				if(sendaudio==true)
				{
					if(flag&SENDPRE_AUDIO)
					{
#ifdef OLDIE
						Seq_AudioIOPointer *sgp=t->GetAudioOut()->FirstChannel();

						while(sgp)
						{
							if(sgp->channel->audiochannelsystemtype==CHANNELTYPE_AUDIOINSTRUMENT 
								//&& sgp->channel->io.audioeffects.FirstActiveAudioInstrument()
								)
								sgp->channel->SendMIDIToAudio(e);

							sgp=sgp->NextChannel();
						}
#endif

						//	if(t->io.bypassallfx==false && t->io.audioeffects.FirstActiveAudioInstrument())
						//			{
						e->SendToAudio(0,&t->io.audioeffects,oflag,0,0);
						//
					}
				}
				else
				{
					if(flag&SENDPRE_MIDI)
					{
						if(mastering==false)
						{
							if(e->GetMIDIPattern())
								SendMIDIOutPlaybackEvent(t,e->GetMIDIPattern(),e,0,0);
							else
								SendMIDIOutPlaybackEventNoEffects(t,0,e,0);
						}
					}
				}
			}

			e=e->NextEvent();
		}

		if(cycle==false)
			t->DeletePRepairEvents(DELETE_PRepair_CONTROL|DELETE_PRepair_SONGSTART);

		// dont delete cycle pRepare events
		t=t->NextTrack();
	}

	//	mainMIDI->DisableMIDIOutFilter();
}


void Seq_Song::AddEventToPRepairList(Seq_Track *track,Seq_Event *seqevent,MIDIPattern *pattern,OSTART pos,OSTART delay,int index)
{
	Seq_Event *addevent=0;
	//bool addnew=false;

	// Find NoteOffs <->
	switch(seqevent->id)
	{
	case OBJ_NOTE:
		{
			Note *enote=(Note *)seqevent;
			OSTART end=enote->GetNoteEnd(pattern)+delay;

			if(end>pos)
			{
				if(addevent=(Seq_Event *)seqevent->Clone(this))
				{
					Note *note=(Note *)addevent;

					note->off.pattern=pattern;
					//note->off.noteon=note;
				}

				//addnew=true;
			}
		}
		break;

	case OBJ_PROGRAM:
		{
			ProgramChange *pc=(ProgramChange *)seqevent;
			Seq_Event *check=track->FirstPRepairCtrl(0);
			bool found=false;

			while(check && (!addevent))	
			{
				if(check->status==seqevent->status)
				{
					if(check->ostart<=seqevent->ostart)
						seqevent->CloneData(this,check); // Event->Clone

					found=true;
				}

				check=check->NextEvent();
			}
			if(found==false)
				addevent=(Seq_Event *)seqevent->Clone(this);
		}

		break;

	case OBJ_CHANNELPRESSURE:
		{
			ChannelPressure *cp=(ChannelPressure *)seqevent;
			Seq_Event *check=track->FirstPRepairCtrl(0);
			bool found=false;

			while(check && found==false)	
			{
				if(check->status==seqevent->status)
				{
					if(check->ostart<=seqevent->ostart)
						seqevent->CloneData(this,check); // Event->Clone

					found=true;
				}

				check=check->NextEvent();
			}

			if(found==false)
				addevent=(Seq_Event *)seqevent->Clone(this);
		}
		break;

	case OBJ_POLYPRESSURE:
		{
			PolyPressure *pp=(PolyPressure *)seqevent;
			Seq_Event *check=track->FirstPRepairCtrl(0);
			bool found=false;

			while(check && found==false)	
			{
				if(check->status==seqevent->status)
				{
					PolyPressure *cpp=(PolyPressure *)check;

					if(cpp->key==pp->key)
					{
						if(check->ostart<=seqevent->ostart)
							seqevent->CloneData(this,check); // Event->Clone

						found=true;
					}
				}

				check=check->NextEvent();
			}

			if(found==false)
				addevent=(Seq_Event *)seqevent->Clone(this);
		}
		break;

	case OBJ_CONTROL:
		{
			ControlChange *cc=(ControlChange *)seqevent;

			if(cc->controller<96) // No Non/Registered etc...
			{
				Seq_Event *check=track->FirstPRepairCtrl(0);
				bool found=false;

				while(check && found==false)	
				{
					if(check->status==seqevent->status)
					{
						ControlChange *ccp=(ControlChange *)check;

						if(ccp->controller==cc->controller)
						{
							if(check->ostart<=seqevent->ostart)
								seqevent->CloneData(this,check); // Event->Clone

							found=true;
						}
					}

					check=check->NextEvent();
				}

				if(found==false)
					addevent=(Seq_Event *)seqevent->Clone(this);
			}
		}
		break;
	}

	if(addevent)
	{
		//	TRACE ("Add Event Status %d B1 %d\n",addevent->GetStatus(),addevent->GetByte1());

		//if(index==1)
		//	event->flag |= EVENTFLAG_ADDEDTOCYCLE;

		// Init Pattern Pointer
		addevent->pattern=pattern;
		addevent->flag=EVENTFLAG_PRESTART;
		addevent->staticostart=pos;

		if(delay)
			addevent->MoveEventQuick(delay);

		//if(addnew==true) // Add To List
		//{

		if(addevent->id==OBJ_NOTE) // NOTES
		{
			track->pRepareeventsnotes[index].AddEndO(addevent);
		}
		else // rest MIDI Events
			track->pRepareeventsctrl[index].AddEndO(addevent);
		//	}
	}
}

void Seq_Song::AddSinglePatternToPRepairList(MIDIPattern *pattern,OSTART pos,int index)
{
	if(pattern->GetPatternStart()<=pos)
	{
		pattern->GetMIDIFX()->MIDIbank_progselectsend=true;

		// Add Pattern Bank Select/Program
		int channel=-1;

		if(pattern->GetMIDIFX()->GetChannel())
			channel=pattern->GetMIDIFX()->GetChannel()-1;
		else
		{
			if(pattern->track->GetFX()->GetChannel())
				channel=pattern->track->GetFX()->GetChannel()-1;
		}

		// Bank/ProgramSelect --- Pattern

		/*
		if(channel!=-1)
		{
		if(pattern->MIDIprogram.on==true)
		{
		if(pattern->MIDIprogram.usebank==true)
		{
		if(ControlChange *cc=mainpools->mempGetControl())
		{
		cc->status=(UBYTE)(CONTROLCHANGE|channel);

		cc->ostart=startsample;
		cc->controller=0;
		cc->value=pattern->MIDIprogram.MIDIBank;
		cc->pattern=pattern;

		if(cycle==false)
		pattern->GetTrack()->pRepareeventsctrl.AddEndO(cc);
		else
		pattern->GetTrack()->pRepareeventsctrl_cycle.AddEndO(cc);
		}
		}

		// Program Change
		{
		if(ProgramChange *pc=new ProgramChange)
		{
		pc->sampleposition=startsample;
		pc->status=(UBYTE)(PROGRAMCHANGE|channel);
		pc->program=pattern->MIDIprogram.MIDIProgram;
		pc->pattern=pattern;

		if(cycle==false)
		pattern->GetTrack()->pRepareeventsctrl.AddEndO(pc);
		else
		pattern->GetTrack()->pRepareeventsctrl_cycle.AddEndO(pc);
		}
		}
		}

		}
		*/
	}

	OSTART tdelay=pattern->track->GetFX()->GetDelay(),
		delaypos=pos+tdelay;

	if(pattern->GetPatternStart()<delaypos)
	{
		Seq_Event *e=pattern->FindEventAtPosition(delaypos);

		if(!e)
			e=pattern->LastEvent();
		else
			while(e && e->GetEventStart(pattern)>=delaypos) // <--- Last Event before startposition
				e=e->PrevEvent(); 

		while(e)
		{
			if( /* (index==0 || (!(e->flag&EVENTFLAG_ADDEDTOCYCLE))) && */ e->GetEventStart(pattern)<delaypos)
				AddEventToPRepairList(pattern->GetTrack(),e,pattern,delaypos,tdelay,index);

			e=e->PrevEvent();
		}
	}
}


void Seq_Song::DeleteAllPreEvents(int flag)
{
	Seq_Track *t=FirstTrack();

	while(t)
	{	
		t->DeletePRepairEvents(flag);
		t=t->NextTrack();
	}
}

void Seq_Song::PRepairPreStartAndCycleEvents(OSTART pos,bool onlycycle)
{
	TRACE ("PRepairPreStartAndCycleEvents %d \n",pos);

	DeleteAllPreEvents(DELETE_PRepair_SONGSTART|DELETE_PRepair_NOTEOFFS|DELETE_PRepair_CONTROL|DELETE_PRepair_CYCLE); // Delete All

	//PRepairPreStartCycleEvents(); // Cycle Events

	for(int i=0;i<2;i++)
	{
		if(onlycycle==true)
			i=INITPLAY_NEWCYCLE;

		// Song Start Pre Events
		Seq_Track *t=FirstTrack();

		while(t)
		{	
			if(onlycycle==false)
				t->recordcycleindex=0; // Reset to Zero

			MIDIPattern *mp=(MIDIPattern *)t->FirstPattern(MEDIATYPE_MIDI);

			while(mp && mp->GetPatternStart()<pos)
			{
				AddSinglePatternToPRepairList(mp,pos,onlycycle==true?1:i);

				mp=(MIDIPattern *)mp->NextPattern(MEDIATYPE_MIDI);
			}

			//AddTracksSubTracksToPRepair(t,pos,false);

			t=t->NextTrack();
		}// while t

		pos=playbacksettings.cyclestart; // i==1, cycle
	}

#ifdef DEBUG
	int c=0;
	int c1=0;

	{
		Seq_Track *t=FirstTrack();

		while(t)
		{	
			c+=t->pRepareeventsnotes[0].GetCount();
			c1+=t->pRepareeventsnotes[1].GetCount();

			t=t->NextTrack();
		}
	}

	TRACE ("################## Song Start Notes %d Cycle %d \n",c,c1);

#endif
}

void Seq_Song::SetFocusType(int t,bool guirefresh)
{
	focustype=t;
}

bool Seq_Song::CanAudioRecordingFileGotoCycleMode()
{
	//if(mainsettings->createnewchildtrackwhenrecordingstarts==true)
	//	return false;

	if(mainsettings->createnewtrackaftercycle==true)
		return false;

	return true;
}

bool Seq_Song::RepairLoops(int flag)
{
	bool repair=RepairCrossFades();

	Seq_Track *firstreftrack=0,*t=FirstTrack();

	// 1. Check
	while(t && firstreftrack==0)
	{
		Seq_Pattern *p=t->FirstPattern();

		while(p && firstreftrack==0)
		{
			if(p->loopendless==true || (p->loops && p->loopwithloops==true))
			{
				Seq_Pattern *nrealpattern=p->NextRealPattern();

				if((!nrealpattern) || p->mediatype==MEDIATYPE_AUDIO)
				{
					if(p->CheckLoopPattern()==true)
						firstreftrack=t;
				}

				p=nrealpattern;

			}
			else
				p=p->NextPattern();
		}

		t=t->NextTrack();
	}

	if(firstreftrack)
	{
		if((!(flag&RLF_NOLOCK)) && this==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		t=firstreftrack;

		// 1. Do
		while(t)
		{
			Seq_Pattern *p=t->FirstPattern();

			while(p)
			{
				if(p->loopendless==true || (p->loops && p->loopwithloops==true))
				{
					Seq_Pattern *nrealpattern=p->NextRealPattern();

					if((!nrealpattern) || p->mediatype==MEDIATYPE_AUDIO)
					{
						if(p->CheckLoopPattern()==true)
							p->LoopPattern();
					}

					p=nrealpattern;
				}
				else
					p=p->NextPattern();

			}

			t=t->NextTrack();
		}

		if((!(flag&RLF_NOLOCK)) && this==mainvar->GetActiveSong())
		{
			CheckPlaybackRefresh();
			mainthreadcontrol->UnlockActiveSong();
		}
		else
		{
			if(flag&RLF_CHECKREFRESH)
				CheckPlaybackRefresh();
		}

		if(flag&RLF_REFRESHGUI)
			maingui->RefreshRepairedLoopsGUI(this);

		repair=true;
	}

	return repair;
}

void Seq_Pattern::RemovePatternFromOtherCrossFades()
{
	Seq_CrossFade *cf=FirstCrossFade();

	while(cf)
	{
		if(cf->connectwith)
		{
			AudioPattern *connectpattern=(AudioPattern *)cf->connectwith->pattern;

			if(connectpattern)
			{
				connectpattern->Lock_CrossFades();
				cf->connectwith->DeInit();
				connectpattern->crossfades.RemoveO(cf->connectwith);
				connectpattern->Unlock_CrossFades();
			}

			cf->connectwith=0;
		}

		cf=cf->NextCrossFade();
	}
}

void Seq_Pattern::DeleteAllCrossFades(bool all,bool removefrompattern)
{
	if(removefrompattern==true) // Avoid Pattern Access (Undo: Create/Delete Pattern
		RemovePatternFromOtherCrossFades();

	if(all==true)
	{
		Seq_CrossFade *cf=FirstCrossFade();

		while(cf)
		{
			cf->DeInit();
			cf=cf->NextCrossFade();
		}

		Lock_CrossFades();
		crossfades.DeleteAllO();
		Unlock_CrossFades();
	}
}

Seq_CrossFade *Seq_Pattern::FindCrossFade(Seq_Pattern *find)
{
	Seq_CrossFade *cf=FirstCrossFade();

	while(cf)
	{
		if(cf->connectwith &&
			(cf->pattern==find || cf->connectwith->pattern==find)
			)return cf;

		cf=cf->NextCrossFade();
	}

	return 0;
}

Colour *Seq_Pattern::GetColour()
{
	return itsaclone==true || itsaloop==true?mainpattern->GetColour():&t_colour;
}

bool Seq_Pattern::CheckIfCrossFadeUsed()
{
	Seq_CrossFade *cf=FirstCrossFade();

	while(cf){
		if(cf->used==true)return true;
		cf=cf->NextCrossFade();
	}

	return false;
}

void Seq_Pattern::AddCrossFade(Seq_CrossFade *newcf)
{
	// Sort Cross Fade Start-Start...

	if(!newcf)return;

	newcf->pattern=this;

	Seq_CrossFade *cf=FirstCrossFade();

	while(cf)
	{
		if(cf->from>newcf->from)
		{
			crossfades.AddNextO(newcf,cf);
			return;
		}

		cf=cf->NextCrossFade();
	}

	crossfades.AddEndO(newcf);

#ifdef DEBUG
	{
		Seq_CrossFade *cf=FirstCrossFade();

		while(cf)
		{

			TRACE ("CF %d <-> %d\n",cf->from_sample,cf->to_sample);
			cf=cf->NextCrossFade();
		}
	}
#endif

}

void Seq_Pattern::DeleteCrossFade(Seq_CrossFade *cf)
{
	crossfades.RemoveO(cf);
}

void Seq_Pattern::MarkAllCrossFades()
{
	Seq_CrossFade *cf=FirstCrossFade();

	while(cf)
	{
		if(cf->dontdeletethis==true)
			cf->deletethis=false;
		else
			cf->deletethis=true;

		cf=cf->NextCrossFade();
	}
}

void Seq_Pattern::DeleteMarkedCrossFades()
{
	Seq_CrossFade *cf=FirstCrossFade();

	while(cf)
	{
		cf->dontdeletethis=false;

		if(cf->deletethis==true)
		{
			Lock_CrossFades();
			Seq_CrossFade *nf=cf->NextCrossFade();
			cf->DeInit();
			crossfades.RemoveO(cf);
			cf=nf;
			Unlock_CrossFades();
		}
		else
		{
			cf->SetCrossFadeSamplePositions();
			cf=cf->NextCrossFade();
		}
	}
}
