#include "undo.h"
#include "patternselection.h"
#include "semapores.h"
#include "object_song.h"
#include "objectpattern.h"
#include "songmain.h"
#include "mempools.h"
#include "MIDIhardware.h"
#include "audiofile.h"
#include "editfunctions.h"
#include "undofunctions.h"
#include "undo_events.h"
#include "folder.h"
#include "editbuffer.h"
#include "drumevent.h"
#include "undo_automation.h"
#include "editsettings.h"
#include "editor_event.h"
#include "languagefiles.h"
#include "audiohdfile.h"
#include "drummap.h"
#include "audioevent.h"
#include "audiopattern.h"
#include "object_track.h"

// Main UndoEdit
bool UndoEdit::Init(int c)
{
	if(c>0 &&
		(oldevents=new Seq_Event*[c]) && 
		(event_p=new Seq_Event*[c]) &&
		(oldindex=new int[c])
		)
	{
		counter=c;
		return true;
	}

	counter=0;
	return false;
}

void UndoEdit::Delete()
{
	for(int i=0;i<counter;i++)
	{
		if(oldevents[i])oldevents[i]->Delete(true);
		if(newevents)newevents[i]->Delete(true);
	}

	if(newevents)delete newevents;
	if(oldevents)delete oldevents;
	if(event_p)delete event_p;
	if(oldindex)delete oldindex;
	if(newindex)delete newindex;

	delete this;
}

bool UndoEdit::CheckChanges()
{
	for(int i=0;i<counter;i++)
	{
		if(oldevents[i]&&

			(
			oldevents[i]->ostart!=event_p[i]->ostart ||
			oldevents[i]->staticostart!=event_p[i]->staticostart ||
			oldevents[i]->Compare(event_p[i])==false
			)

			)
			return true;
	}

	return false;
}

void Undo_EditEvents::FreeData()
{
	editevents->Delete();
}

void Undo_EditEvents::Do()
{
	if(editevents->newevents)
	{
		// Move ?
		for(int i=0;i<editevents->counter;i++)
		{
			if(OSTART diff=editevents->newevents[i]->ostart-editevents->oldevents[i]->ostart)
			{
				editevents->event_p[i]->MoveEvent(diff);
			}
		}

		// Index
		for(int i=0;i<editevents->counter;i++)
		{
			if(editevents->newindex)
			{
				if(editevents->event_p[i]->GetMIDIPattern()) // MIDI only !
					editevents->event_p[i]->GetPattern()->GetEventList()->MoveOToIndex(editevents->event_p[i],editevents->newindex[i]);
			}
		}

		// Data
		for(int i=0;i<editevents->counter;i++)
			editevents->newevents[i]->CloneData(song,editevents->event_p[i]);
	}
}

void Undo_EditEvents::DoUndo()
{
	if(!editevents->newevents)
	{
		if(editevents->newevents=new Seq_Event*[editevents->counter])
			for(int i=0;i<editevents->counter;i++)
				editevents->newevents[i]=(Seq_Event *)editevents->event_p[i]->Clone(song);

		if(editevents->newindex=new int[editevents->counter])
			for(int i=0;i<editevents->counter;i++)
				editevents->newindex[i]=editevents->event_p[i]->GetIndex();
	}

	// Move ?
	if(editevents->newevents)
	{
		for(int i=0;i<editevents->counter;i++)
		{
			if(OSTART diff=editevents->oldevents[i]->ostart-editevents->newevents[i]->ostart)
			{
				editevents->event_p[i]->MoveEvent(diff);
			}
		}

		// Index
		for(int i=0;i<editevents->counter;i++)
		{
			if(editevents->event_p[i]->GetMIDIPattern()) // MIDI only
				editevents->event_p[i]->GetPattern()->GetEventList()->MoveOToIndex(editevents->event_p[i],editevents->oldindex[i]);
		}
	}

	// Data
	for(int i=0;i<editevents->counter;i++)
		editevents->oldevents[i]->CloneData(song,editevents->event_p[i]);
}

void EditFunctions::EditEvents(Seq_Song *song,UndoEdit *uee)
{
	if(uee)
	{
		if(UndoFunction *uf=uee->CreateUndoFunction())
		{
			song->undo.OpenUndoFunction(uf);
			CheckEditElementsForGUI(song,uf,true);
		}
		else
			uee->Delete();
	}
}

// All Editor Edit functions....
void Undo_DeleteEvent::DoUndo()
{
	if(dead==false)
	{
		int i=numberofevents;	
		UndODeInitEvent *p=events;

		//	refreshmode=MEDIATYPE_MIDI;
		while(i--)
		{
			if(p->seqevent)
			{
				p->seqevent->flag&=~OFLAG_SELECTED;

				if(p->topattern)
				{
					p->seqevent->AddSortToPattern(p->topattern);
					p->topattern->checknotechain=true;
				}
				else // AudioPattern
				{
					if(p->seqevent->id==OBJ_AUDIOEVENT)
					{
						AudioEvent *ae=(AudioEvent *)p->seqevent;

						p->totrack->AddSortPattern(ae->GetAudioPattern(),ae->GetAudioPattern()->ostart);
						ae->GetAudioPattern()->RefreshAfterPaste();

						// Clones
						Seq_ClonePattern *cl=ae->GetAudioPattern()->FirstClone();
						while(cl)
						{
							cl->pattern->track->AddSortPattern(cl->pattern,cl->pattern->ostart);
							cl->insideundo=false;
							cl->pattern->RefreshAfterPaste();

							cl=cl->NextClone();
						}
					}
				}
			}

			p++;
		}

		i=numberofevents;	
		p=events;

		while(i--){

			// Index
			if(p->seqevent->GetMIDIPattern()) // MIDI only
				p->seqevent->GetPattern()->GetEventList()->MoveOToIndex(p->seqevent,p->index);

			if(p->topattern && p->topattern->checknotechain==true)
			{
				p->topattern->checknotechain=false;
				p->topattern->CloseEvents();
			}

			p++;
		}
	}

}

void Undo_DeleteEvent::Do()
{
	int i=numberofevents;	
	UndODeInitEvent *p=events;

	while(i--)
	{	
		if(p->seqevent)
		{
			mainMIDI->StopAllofEvent(song,p->seqevent);

			if(p->topattern)
			{
				p->topattern->DeleteEvent(p->seqevent,false); // + set editsort
				p->seqevent->SetPattern(0);

				p->topattern->checknotechain=true;
			}
			else // Audio Event
			{
				if(p->seqevent->id==OBJ_AUDIOEVENT)
				{
					AudioEvent *ae=(AudioEvent *)p->seqevent;

					p->totrack=ae->GetTrack();
					p->totrack->DeletePattern(ae->GetAudioPattern(),false);

					maingui->RemovePatternFromGUI(song,ae->GetAudioPattern());
				}
			}

			// Delete selection flag
			p->seqevent->flag &= ~(1<<OFLAG_SELECTED);
		}

		p++;
	}

	// Note Chain
	i=numberofevents;
	p=events;

	while(i--)
	{	
		if(p->seqevent)
		{
			if(p->topattern && p->topattern->checknotechain==true)
			{
				p->topattern->CloseEvents();
				p->topattern->checknotechain=false;
			}
		}

		p++;
	}
}

void Undo_DeleteEvent::RefreshGUI(bool undorefresh)
{
	for(int i=0;i<numberofevents;i++)
		maingui->RemoveEventFromGUI(song,events[i].seqevent);

	mainedit->CheckEditElementsForGUI(song,this,undorefresh);
}

void Undo_DeleteEvent::FreeData()
{
	if(events)
	{
		if(inundo==true)
		{
			UndODeInitEvent *de=events;
			int c=numberofevents;

			while(c--){		
				DeleteEvent(de);
				de++;
			}
		}

		delete events;
	}
}

void Undo_DeleteEvent::DeleteEvent(UndODeInitEvent *e)
{
	if(e && e->seqevent && inundo==true)
	{
		if(e->topattern)
		{
			e->seqevent->Delete(true);
			e->topattern->CloseEvents();
		}
		else
		{
			if(e->seqevent->id==OBJ_AUDIOEVENT)
			{
				AudioEvent *ae=(AudioEvent *)e->seqevent;
				ae->GetAudioPattern()->Delete(true);
			}
		}
	}
}

int EditFunctions::DeleteEvent(Seq_Event *seqevent,bool addtolastundo)
{
	int c=0;

	if(seqevent && CheckIfEditOK(seqevent->pattern->track->song)==true)
	{
		Seq_Song *song=seqevent->pattern->track->song;

		if(UndODeInitEvent *eventpointer=new UndODeInitEvent)
		{
			eventpointer->seqevent=seqevent;
			eventpointer->topattern=seqevent->GetMIDIPattern();
			eventpointer->index=seqevent->GetIndex();

			if(Undo_DeleteEvent *ude=new Undo_DeleteEvent(song,eventpointer,1))
			{
				Lock(song);
				ude->Do();
				CheckPlaybackAndUnLock();

				song->undo.OpenUndoFunction(ude,addtolastundo);
				ude->RefreshGUI(true);
				c=1;
			}
		}
	}

	return c;
}


int EditFunctions::DeleteEvents(Seq_SelectionList *list,bool addtolastundo)
{
	int c;

	if(list && 
		CheckIfEditOK(list->song)==true && 
		(c=list->GetCountofSelectedEvents_Filter(list->buildfilter,list->buildicdtype))
		) // list or event ?
	{
		Seq_Song *song=list->song;

		if(UndODeInitEvent *eventpointer=new UndODeInitEvent[c])
		{
			int i=0;

			Seq_SelectionEvent *sele=list->FirstMixEvent();

			while(sele)
			{
				if(sele->seqevent->IsSelected()==true && 
					sele->seqevent->CheckSelectFlag(list->buildfilter|OFLAG_SELECTED,list->buildicdtype)==true
					)
				{
					eventpointer[i].seqevent=sele->seqevent;
					eventpointer[i].topattern=sele->seqevent->GetMIDIPattern();
					eventpointer[i].index=sele->seqevent->GetIndex();
					
					i++;
				}

				sele=sele->NextEvent();
			}

#ifdef _DEBUG
			if(i!=c)
				MessageBox(NULL,"Illegal Delete Event Operations","Error",MB_OK_STYLE);
#endif

			if(Undo_DeleteEvent *ude=new Undo_DeleteEvent(song,eventpointer,c))
			{
				Lock(song);

				ude->Do();
				CheckPlaybackAndUnLock();
				song->undo.OpenUndoFunction(ude,addtolastundo);
				
				ude->RefreshGUI(true);

				return c;
			}
			else
				delete eventpointer;
		}
	}

	return 0;
}

void Undo_CutNote::DoUndo()
{
	if(newnote){
		note->SetEnd(newnote->GetNoteEnd());
		note->GetPattern()->DeleteEvent(newnote,true);
		newnote=0;

		note->GetPattern()->CloseEvents();
	}
}

void Undo_CutNote::Do()
{
	if(newnote=(Note *)note->Clone(0)){
		newnote->ostart=newnote->staticostart=cutposition; // New Note Position
		newnote->AddSortToPattern(note->GetPattern(),cutposition);
		note->SetEnd(cutposition);

		note->GetPattern()->CloseEvents();
	}
}

void EditFunctions::CutNote(Note *note,OSTART cutposition)
{
	if(note && CheckIfEditOK(note->pattern->track->song)==true &&
		note->GetEventStart()<cutposition &&
		note->GetNoteEnd()>cutposition)
	{
		Seq_Song *song=note->pattern->track->song;

		if(Undo_CutNote *ucn=new Undo_CutNote(note,cutposition))
			OpenLockAndDo(song,ucn,true);
	}
}

void Undo_ConvertNotesToDrums::DoUndo()
{
	if(drumbuffer)
	{
		for(int i=0;i<counter;i++)
		{
			if(events[i] && drumbuffer[i])
			{
				mainMIDI->StopAllofEvent(song,drumbuffer[i]);
				drumbuffer[i]->GetMIDIPattern()->DeleteEvent(drumbuffer[i],true);
				events[i]->AddSortToPattern(events[i]->GetMIDIPattern(),events[i]->GetEventStart());
			}
		}

		delete drumbuffer;
		drumbuffer=0;
	}
}

void Undo_ConvertNotesToDrums::Do()
{
	if(!drumbuffer)
		drumbuffer=new ICD_Drum*[counter];

	if(drumbuffer)
	{
		for(int i=0;i<counter;i++)
		{
			if(events[i])
			{
				// Note to Drum
				ICD_Drum *drum=new ICD_Drum;

				if(drumbuffer[i]=drum)
				{
					mainMIDI->StopAllofEvent(song,events[i]);

					Note *note=(Note *)events[i];
					//	drumbuffer[i]=drum;

					drum->staticostart=note->staticostart;
					drum->velocity=note->velocity;
					drum->velocityoff=note->velocityoff;
					drum->drumtrack=drumtracks[i]; // <-> drumtrack
					// Delete old Notes
					drum->AddSortToPattern(note->GetMIDIPattern(),note->GetEventStart());
					note->GetMIDIPattern()->DeleteEvent(note,false);
				}
			}
		}
	}
}

void Undo_ConvertNotesToDrums::FreeData()
{
	if(drumbuffer)
		delete drumbuffer;

	if(events)
	{
		if(inundo==true)
			for(int i=0;i<counter;i++)
				events[i]->Delete(true);

		delete events;
	}

	if(drumtracks)
		delete drumtracks;
}

void EditFunctions::ConvertNotesToDrums(Seq_SelectionList *list ,Drummap *map)
{
	if(map && list && CheckIfEditOK(list->song)==true)
	{
		int converted=0;

		Seq_Event *d=list->FirstSelectionEvent(0,SEL_NOTEON);

		while(d)
		{
			Drumtrack *dt=map->FirstTrack();

			while(dt && d)
			{
				Note *note=(Note *)d;

				if(dt->GetMIDIChannel()==d->GetChannel() && note->key==dt->key)
					converted++;

				dt=dt->NextTrack();
			}

			d=list->NextSelectionEvent(SEL_NOTEON);
		}

		if(converted)
		{
			d=list->FirstSelectionEvent(0,SEL_NOTEON);
			Seq_Event **eventbuffer=new Seq_Event*[converted];
			Drumtrack **drumtrackbuffer=new Drumtrack*[converted];

			if(eventbuffer && drumtrackbuffer)
			{
				int c=0;

				while(d)
				{
					Drumtrack *dt=map->FirstTrack();

					while(dt && d)
					{
						Note *note=(Note *)d;

						if(dt->GetMIDIChannel()==d->GetChannel() && note->key==dt->key)
						{
							eventbuffer[c]=d;
							drumtrackbuffer[c++]=dt;
						}

						dt=dt->NextTrack();
					}

					d=list->NextSelectionEvent(SEL_NOTEON);
				}

				if(Undo_ConvertNotesToDrums *ucd=new Undo_ConvertNotesToDrums(eventbuffer,drumtrackbuffer,converted))
				{
					OpenLockAndDo(list->song,ucd,true);
				}
				else
				{
					delete eventbuffer;
					delete drumtrackbuffer;
				}
			}//if buffer
			else
			{
				if(eventbuffer)
					delete eventbuffer;

				if(drumtrackbuffer)
					delete drumtrackbuffer;
			}
		}
	}
}

void Undo_ConvertDrumsToNotes::DoUndo()
{
	if(notebuffer)
	{
		for(int i=0;i<counter;i++)
		{
			if(notebuffer[i])
			{
				mainMIDI->StopAllofEvent(song,notebuffer[i]);
				notebuffer[i]->GetMIDIPattern()->DeleteEvent(notebuffer[i],true);
				events[i]->AddSortToPattern(events[i]->GetMIDIPattern(),events[i]->GetEventStart());
			}
		}

		delete notebuffer;
		notebuffer=0;
	}
}

void Undo_ConvertDrumsToNotes::Do()
{
	if(!notebuffer)
		notebuffer = new Note*[counter];

	if(events)
	{
		for(int i=0;i<counter;i++)
		{
			ICD_Drum *drum=(ICD_Drum *)events[i];

			if(notebuffer)
			{
#ifdef MEMPOOLS
				Note *note=mainpools->mempGetNote();
#else
				Note *note=new Note;
#endif

				if(notebuffer[i]=note)
				{
					note->staticostart=drum->staticostart;
					note->status=NOTEON|drum->drumtrack->GetMIDIChannel();
					note->key=drum->drumtrack->key;
					note->velocity=drum->velocity;
					note->velocityoff=drum->velocityoff;
					note->ostart=drum->GetEventStart();
					note->off.ostart=note->off.staticostart=drum->GetEventStart()+drum->drumtrack->ticklength;

					// Delete old Notes		
					mainMIDI->StopAllofEvent(song,events[i]);

					events[i]->GetMIDIPattern()->DeleteEvent(events[i],false);

					note->AddSortToPattern(events[i]->GetMIDIPattern(),drum->GetEventStart());
				}
			}
		}//for 
	}
}

void Undo_ConvertDrumsToNotes::FreeData()
{
	if(notebuffer)
		delete notebuffer;

	if(events)
	{
		if(inundo==true)
			for(int i=0;i<counter;i++)
				events[i]->Delete(true);

		delete events;
	}
}

void EditFunctions::ConvertDrumsToNotes(Seq_SelectionList *list)
{
	if(CheckIfEditOK(list->song)==true)
	{	
		int converted=0;

		Seq_Event *d=list->FirstSelectionEvent(0,SEL_INTERN);

		while(d){
			ICD_Object *io=(ICD_Object *)d;

			if(io->type==ICD_TYPE_DRUM)
				converted++;

			d=list->NextSelectionEvent(SEL_INTERN);
		}

		if(converted) // build buffer
		{
			Seq_Event **eventbuffer=new Seq_Event*[converted];
			int c=0;

			if(eventbuffer){

				Seq_Event *d=list->FirstSelectionEvent(0,SEL_INTERN);

				while(d){

					ICD_Object *io=(ICD_Object *)d;

					if(io->type==ICD_TYPE_DRUM)
						eventbuffer[c++]=d;

					d=list->NextSelectionEvent(SEL_INTERN);
				}

				if(Undo_ConvertDrumsToNotes *ucd=new Undo_ConvertDrumsToNotes(eventbuffer,converted)){

					OpenLockAndDo(list->song,ucd,true);
				}
				else
					delete eventbuffer;
			}
		}
	}
}

// Start Quantize Event
void Undo_QuantizeEvent::RefreshIndexs()
{
	for(int i=0;i<qcounter;i++)
		qnl[i].seqevent->pattern->RefreshIndexs();
}

void Undo_QuantizeEvent::Do()
{
	for(int i=0;i<qcounter;i++)
	{
		qnl[i].seqevent->pattern->checknotechain=true;
		qnl[i].seqevent->pattern->eventclose=true;

		qnl[i].seqevent->MoveEvent(qnl[i].newstart-qnl[i].oldstart);
	}

	RefreshIndexs();

	for(int i=0;i<qcounter;i++)
	{
		if(qnl[i].seqevent->pattern->checknotechain==true)
		{
			qnl[i].seqevent->pattern->checknotechain=false;
			qnl[i].seqevent->pattern->CloseEvents();
		}
	}
}

void Undo_QuantizeEvent::DoUndo()
{
	for(int i=0;i<qcounter;i++)
	{
		qnl[i].seqevent->pattern->checknotechain=true;
		qnl[i].seqevent->pattern->eventclose=true;
		qnl[i].seqevent->MoveEvent(qnl[i].oldstart-qnl[i].newstart);
	}

	RefreshIndexs();

	for(int i=0;i<qcounter;i++)
	{
		if(qnl[i].seqevent->pattern->checknotechain==true)
		{
			qnl[i].seqevent->pattern->checknotechain=false;
			qnl[i].seqevent->pattern->CloseEvents();
		}	
	}

}

void EditFunctions::QuantizeEvents_Execute(Seq_Song *song,Seq_SelectionList *list,int qindex) // called by menu
{
	if(CheckIfEditOK(song)==true)
	{
		// Count selected events
		Seq_SelectionEvent *d=list->FirstMixEvent();
		int c=0;
		OSTART ticks=quantlist[qindex];

		while(d)
		{
			if(d->seqevent->IsSelected()==true)
			{
				OSTART ns=mainvar->SimpleQuantize(d->seqevent->GetEventStart(),ticks);

				if(ns!=d->seqevent->GetEventStart())
					c++;
			}

			d=d->NextEvent();
		}

		if(c)
		{
			if(UndoQuantizeEvent *ql=new UndoQuantizeEvent[c])
			{
				Seq_SelectionEvent *d=list->FirstMixEvent();
				int i=0;

				while(d && i<c)
				{
					if(d->seqevent->IsSelected()==true)
					{
						OSTART ns=mainvar->SimpleQuantize(d->seqevent->GetEventStart(),ticks);

						if(ns!=d->seqevent->GetEventStart())
						{
							ql[i].seqevent=d->seqevent;
							ql[i].newstart=ns;
							ql[i].oldstart=d->seqevent->GetEventStart();

							i++;
						}
					}

					d=d->NextEvent();
				}

				if(Undo_QuantizeEvent *udt=new Undo_QuantizeEvent(list,c,ticks,ql))
					OpenLockAndDo(song,udt,true);
				else
					delete ql;
			}
		}
	}
}

void EditFunctions::QuantizeEventsMenu(guiWindow *win,Seq_SelectionList *list)
{
	if(list->GetCountofSelectedEvents())
	{
		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_qevent:public guiMenu
			{
			public:
				menu_qevent(Seq_Song *s,Seq_SelectionList *l,int qt)
				{
					song=s;
					list=l;
					qtx=qt;
				}

				void MenuFunction()
				{
					mainedit->QuantizeEvents_Execute(song,list,qtx);
				} //

				Seq_Song *song;
				Seq_SelectionList *list;
				int qtx;
			};

			win->skipdeletepopmenu=true;
			win->popmenu->AddMenu(Cxs[CXS_QUANTIZEEVENTS],0);
			win->popmenu->AddLine();

			for(int m=0;m<QUANTNUMBER;m++)
				win->popmenu->AddFMenu(quantstr[m],new menu_qevent(win->WindowSong(),list,m));

			win->ShowPopMenu();
		}
	}
}
// -End Quantize Events

// Start Set Note Length
void Undo_SetNoteLength::Do()
{
	for(int i=0;i<notecounter;i++)
	{
		usnl[i].note->SetLength(ticks);
		//usnl[i].note->v_SetChildSampleFactor(song);
	}
}

void Undo_SetNoteLength::DoUndo()
{
	for(int i=0;i<notecounter;i++)
	{
		usnl[i].note->SetLength(usnl->oldlength);
		//usnl[i].note->v_SetChildSampleFactor(song);
	}
}

void EditFunctions::SetNoteLength_Execute(Seq_Song *song,Seq_SelectionList *list,int nlindex) // called by menu
{
	if(CheckIfEditOK(song)==true)
	{
		// Count selected notes
		Seq_Event *d=list->FirstSelectionEvent(0,SEL_NOTEON);
		int c=0;
		OSTART ticks=quantlist[nlindex];

		while(d)
		{
			if(d->IsSelected()==true)
			{
				Note *note=(Note *)d;

				if(note->GetNoteLength()!=ticks)
					c++;
			}

			d=list->NextSelectionEvent(SEL_NOTEON);
		}

		if(c)
		{
			if(UndoSetNoteLength *snl=new UndoSetNoteLength[c])
			{
				Seq_Event *d=list->FirstSelectionEvent(0,SEL_NOTEON);
				int i=0;

				while(d && i<c)
				{
					if(d->IsSelected()==true)
					{
						Note *note=(Note *)d;

						if(note->GetNoteLength()!=ticks)
						{
							snl[i].note=note;
							snl[i].oldlength=note->GetNoteLength();
							i++;
						}
					}

					d=list->NextSelectionEvent(SEL_NOTEON);
				}

				if(Undo_SetNoteLength *udt=new Undo_SetNoteLength(list,c,ticks,snl))
					OpenLockAndDo(song,udt,true);
				else
					delete snl;
			}
		}
	}
}

void EditFunctions::SetNoteLength(guiWindow *win,Seq_SelectionList *list)
{
	// Count selected notes
	Seq_Event *d=list->FirstSelectionEvent(0,SEL_NOTEON);
	int c=0;

	while(d)
	{
		if(d->flag&OFLAG_SELECTED)
		{
			c++;
			break;
		}

		d=list->NextSelectionEvent(SEL_NOTEON);
	}

	if(c)
	{
		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_setnote:public guiMenu
			{
			public:
				menu_setnote(Seq_Song *s,Seq_SelectionList *l,int nl)
				{
					song=s;
					list=l;
					notelength=nl;
				}

				void MenuFunction()
				{
					mainedit->SetNoteLength_Execute(song,list,notelength);
				} //

				Seq_Song *song;
				Seq_SelectionList *list;
				int notelength;
			};

			win->popmenu->AddMenu(Cxs[CXS_SETLENGTHOFNOTES],0);

			for(int m=0;m<QUANTNUMBER;m++)
				win->popmenu->AddFMenu(quantstr[m],new menu_setnote(win->WindowSong(),list,m));

			win->ShowPopMenu();
		}
		win->skipdeletepopmenu=true;
	}
}
// ---- end Set Note Length 


Undo_EditEvent::Undo_EditEvent(Seq_Event *e,EF_CreateEvent *cr)
{
	id=Undo::UID_EDITEVENTS;
	seqevent=e;
	oldevent=(Seq_Event *)e->Clone(song);
	newevent=(Seq_Event *)cr->seqevent->Clone(song);
}


void Undo_EditEvent::Do()
{
	if(newevent)
		newevent->CloneData(song,seqevent);
}

void Undo_EditEvent::DoRedo()
{
	Do();

	/*
	UndoFunction *uf=FirstFunction();

	while(uf){
	uf->Do();
	uf=uf->NextFunction();
	}
	*/
}

void Undo_EditEvent::DoUndo()
{
	if(newevent)
		seqevent->CloneData(song,newevent);

	if(oldevent)
		oldevent->CloneData(song,seqevent);

	//AddSubUndoFunctions();
}

void Undo_EditEvent::FreeData()
{
	if(oldevent)
		oldevent->Delete(true);

	if(newevent)
		newevent->Delete(true);
}

void EditFunctions::EditEventData(Seq_Event *seqevent,EF_CreateEvent *create)
{
	return;

	if(CheckIfEditOK(create->song)==true)
	{
		UndoFunction *last=0;

		create->insidelastundo=0;

		if(create->doundo==true)
		{
			if(create->addtolastundo==true && 
				(last=create->song->undo.LastUndo()) && 
				last->GetOpenStatus()==true &&
				last->id==Undo::UID_EDITEVENTS
				)
			{
				// Check Head
				if(((Undo_EditEvent *)last)->seqevent==seqevent)
					create->insidelastundo=(Undo_EditEvent *)last;

				// Check If Event is in Last Undo
				UndoFunction *ue=last->FirstFunction();

				while(ue && create->insidelastundo==0)
				{
					if(ue->id==Undo::UID_EDITEVENTS)
					{
						if(((Undo_EditEvent *)ue)->seqevent==seqevent)
							create->insidelastundo=(Undo_EditEvent *)ue;
					}

					ue=ue->NextFunction();
				}
			}
			else
				create->addtolastundo=false; // no last undo function
		}

		Undo_EditEvent *uee;

		if(!(uee=create->insidelastundo))
			uee=new Undo_EditEvent(seqevent,create);
		else
		{
			if(uee->newevent)
				uee->newevent->Delete(true);

			uee->newevent=(Seq_Event *)create->seqevent->Clone(create->song);
		}

		if(uee)
		{	
			uee->Do();

			if(create->doundo==false || create->insidelastundo)
			{
				if(create->checkgui==true)
					CheckEditElementsForGUI(create->song,uee,false);

				if(!create->insidelastundo)
					uee->DeleteUndoFunction(true,true);
			}
			else
			{
				if(create->addtolastundo==true)
				{
					if(!create->insidelastundo)
						last->AddFunction(uee);
				}
				else
					create->song->undo.OpenUndoFunction(uee); // new undo ?

				if(create->checkgui==true)
					CheckEditElementsForGUI(create->song,uee,true);
			}

			if(create->playit==true)
				create->pattern->track->SendOutEvent_User(create->pattern,create->seqevent,true);

		}
	}
}

Undo_CreateEvent::Undo_CreateEvent(Seq_Song *s,Seq_Event *e,EF_CreateEvent *cr)
{
	id=Undo::UID_CREATENEWEVENTS;

	seqevent=e;
	eventlist=0;

	song=s;
	topattern=cr->pattern;
	startposition=cr->position;
	endposition=cr->endposition;
	playit=cr->playit;
	eventlist=0;
	neweventlist=0;
}

Undo_CreateEvent::Undo_CreateEvent(Seq_Song *s,MIDIPattern *to,OList *l,OSTART sp)
{
	id=Undo::UID_CREATENEWEVENTS;

	song=s;
	topattern=to;
	startposition=sp;
	endposition=0;
	eventlist=l;
	seqevent=0;
	neweventlist=0;
}

void Undo_CreateEvent::Do()
{
	if(dead==false)
	{
		if(eventlist)
		{
			// List
			if(neweventlist)
			{
				for(int i=0;i<neweventcount;i++)
				{
					if(neweventlist[i])
						neweventlist[i]->AddSortToPattern(topattern); // Redo
				}
			}
			else
			{
				if(Seq_Event *e=(Seq_Event *)eventlist->GetRoot())
				{
					if(neweventlist=new Seq_Event*[neweventcount=eventlist->GetCount()])
					{
						int c=0;

						OSTART fs=e->GetEventStart(); // sorted left--->right

						while(e)
						{
							Seq_Event *clone=(Seq_Event *)e->Clone(song);

							neweventlist[c++]=clone;

							if(clone)
								clone->AddSortToPattern(topattern,startposition+(e->GetEventStart()-fs));

							e=(Seq_Event *)e->Next();
						}
					}
				}
			}
		}
		else
		{
			// Single Event
			seqevent->AddSortToPattern(topattern,startposition);	
		}

		topattern->CloseEvents();
	}
}

void Undo_CreateEvent::DoUndo()
{
	if(dead==false)
	{
		if(eventlist)
		{
			for(int i=0;i<neweventcount;i++)
			{
				mainMIDI->StopAllofEvent(song,neweventlist[i]);
				topattern->DeleteEvent(neweventlist[i],false);
			}
		}
		else
		{
			if(seqevent)
			{
				mainMIDI->StopAllofEvent(song,seqevent);
				topattern->DeleteEvent(seqevent,false);
			}
		}

		topattern->CloseEvents();
	}
}

void Undo_CreateEvent::FreeData()
{
	if(eventlist)
	{
		// Delete Init List
		Seq_Event *e=(Seq_Event *)eventlist->GetRoot();

		while(e)
		{
			Seq_Event *ne=(Seq_Event *)e->Next();
			e->Delete(true);
			e=ne;
		}

		if(inundo==false)
		{
			for(int i=0;i<neweventcount;i++)
			{
				if(neweventlist[i])
					neweventlist[i]->Delete(true);
			}
		}
	}
	else
		if(seqevent)
		{
			if(inundo==false)	
				seqevent->Delete(true);
		}
}

void EditFunctions::CreateNewMIDIEvents(MIDIPattern *topattern,OList *list,OSTART position)
{
	Seq_Song *song=topattern->track->song;

	if(Undo_CreateEvent *uee=new Undo_CreateEvent(song,topattern,list,position))
	{	
		Lock(song);
		uee->Do();
		CheckPlaybackAndUnLock();

		song->undo.OpenUndoFunction(uee);
		CheckEditElementsForGUI(song,uee,true);
	}
}

void EditFunctions::CreateNewMIDIEvent(EF_CreateEvent *create)
{
	if(CheckIfEditOK(create->song)==true)
	{
		if(create->seqevent)
		{
			TRACE ("CreateNewMIDIEvent %d\n",create->addtolastundo);

			if(Undo_CreateEvent *uee=new Undo_CreateEvent(create->song,create->seqevent,create))
			{	
				Lock(create->song);
				uee->Do();
				CheckPlaybackAndUnLock();

				if(create->doundo==false)
				{
					if(create->checkgui==true)
						CheckEditElementsForGUI(create->song,uee,false);

					delete uee;
				}
				else
				{
					create->song->undo.OpenUndoFunction(uee,create->addtolastundo);

					if(create->checkgui==true)
						CheckEditElementsForGUI(create->song,uee,true);
				}

				if(create->playit==true)
					create->pattern->track->SendOutEvent_User(create->pattern,create->seqevent,true);
			}
		}	
	}
}


void Undo_SizeNotes::Do()
{
	for(int i=0;i<numberofevents;i++)
	{
		events[i].note->pattern->checknotechain=true;

		OSTART nstart=events[i].note->ostart,nend=events[i].note->GetNoteEnd();

		if(startorend==true)
		{
			nstart+=events[i].changelength;

			if(nstart>nend)nstart=nend;
			if(nstart<0)nstart=0;
			events[i].note->SetStart(nstart);
		}
		else
		{
			nend+=events[i].changelength;
			if(nend<nstart)nend=nstart+1;
			events[i].note->SetEnd(nend);
		}
	}

	for(int i=0;i<numberofevents;i++) // Note Chain
	{
		if(events[i].note->pattern->checknotechain==true)
		{
			events[i].note->pattern->checknotechain=false;
			events[i].note->pattern->CloseEvents();
		}
	}
}

void Undo_SizeNotes::DoUndo()
{
	for(int i=0;i<numberofevents;i++)
	{
		events[i].note->pattern->checknotechain=true;

		if(startorend==true)
			events[i].note->SetStart(events[i].oldstart);
		else
			events[i].note->SetEnd(events[i].oldend);
	}

	for(int i=0;i<numberofevents;i++)// Note Chain
	{
		if(events[i].note->pattern->checknotechain==true)
		{
			events[i].note->pattern->checknotechain=false;
			events[i].note->pattern->CloseEvents();
		}
	}
}

bool EditFunctions::SizeNote(Seq_Song *song,Note *note,OSTART newend,bool addtoundo)
{
	if(CheckIfEditOK(song)==true)
	{
		if(UndoSizeNote *usn=new UndoSizeNote[1])
		{
			usn->note=note;
			usn->changelength=newend-note->GetNoteLength();
			usn->oldend=note->GetNoteEnd();

			if(Undo_SizeNotes *u_sn=new Undo_SizeNotes(song,usn,1,false))
			{
				if(addtoundo==true)
					song->undo.OpenUndoFunction(u_sn);

				Lock(song);
				u_sn->Do();

				CheckPlaybackAndUnLock();
				CheckEditElementsForGUI(song,u_sn,true);

				if(addtoundo==false)
				{
					//Set Static NoteOff Position
					note->off.staticostart=note->off.ostart;

					delete usn;
					delete u_sn;
				}
			}
			else
				delete usn;
		}
	}

	return true;
}

bool EditFunctions::SizeSelectedNotesInPatternList(Seq_Song *song,Seq_SelectionList *sellist,OSTART diff,int flag,bool startorend)
{
	if(CheckIfEditOK(song)==true)
	{
		bool sized=false;
		int c=0;

		Seq_SelectionEvent *se=sellist->FirstMixEvent();

		while(se)
		{			
			if(se->seqevent->GetStatus()==NOTEON && se->seqevent->IsSelected()==true && (diff || se->changedlength))
				c++;

			se=se->NextEvent();
		}

		if(c && (diff || (flag&SIZENOTES_FLAGEVENTDIFF)) )
		{
			UndoSizeNote *pp,*eventpointer;	

			pp=eventpointer=new UndoSizeNote[c];

			if(pp)
			{
				se=sellist->FirstMixEvent();

				while(se)
				{			
					if(se->seqevent->GetStatus()==NOTEON && se->seqevent->IsSelected()==true && (diff || se->changedlength))
					{
						pp->note=(Note *)se->seqevent;

						if(se->changedlength)
						{
							pp->changelength=se->changedlength;
							se->changedlength=0; // reset
						}
						else
							pp->changelength=diff;

						pp->oldstart=pp->note->GetEventStart();
						pp->oldend=pp->note->GetNoteEnd();

						pp++;
					}

					se=se->NextEvent();
				}

			}//if pp

			if(Undo_SizeNotes *usn=new Undo_SizeNotes(song,eventpointer,c,startorend))
			{
				OpenLockAndDo(song,usn,true);
				sized=true;
			}
			else
				delete eventpointer;
		}
		else
			sellist->ResetChanges();

		return sized;
	}

	return 0;
}

void Undo_CopyEvents::Do()
{
	UndoCopyEvent *ep=events;
	copied=false;

	for(int i=0;i<numberofevents;i++)
	{		
		ep->clone=ep->seqevent->CloneNewAndSortToPattern(song,ep->seqevent->GetMIDIPattern(),movediff,0);

		if(ep->clone)
		{
			ep->seqevent->pattern->eventclose=true;

			if(keydiff)
				ep->clone->MoveIndex(keydiff);

			copied=true;
		}

		ep++;
	}

	ep=events;

	for(int i=0;i<numberofevents;i++)
	{
		ep->seqevent->pattern->RefreshIndexs();
		ep++;
	}
}

void Undo_CopyEvents::DoUndo()
{
	UndoCopyEvent *ep=events;

	for(int i=0;i<numberofevents;i++)
	{	
		if(ep->clone)
		{
			ep->clone->GetMIDIPattern()->DeleteEvent(ep->clone,true);
			ep->clone=0;
		}

		ep++;
	}

	ep=events;

	for(int i=0;i<numberofevents;i++)
	{
		ep->seqevent->pattern->RefreshIndexs();
		ep++;
	}
}

bool EditFunctions::CopySelectedEventsInPatternList(MoveO *mo)
{
	OList selevents;
	int refreshmode=0,c;
	bool moved=false;

	if((mo->diff || mo->index) && (c=mo->sellist->GetCountofSelectedEvents()) )
	{
		UndoCopyEvent *pp,*eventpointer;

		pp=eventpointer=new UndoCopyEvent[c];

		if(pp)
		{
			int c2=0;
			bool error=false;
			Seq_Event *sele=mo->sellist->FirstSelectionEvent(0,mo->filter|SEL_SELECTED);

			while(sele)
			{
				if(c2<c)
				{
					pp->seqevent=sele;
					pp++;
					c2++;
				}
				else
					error=true;

				sele=mo->sellist->NextSelectionEvent(mo->filter|SEL_SELECTED);
			}

			if(c2!=c)
				error=true;

			if(error==false)
			{
				Undo_CopyEvents *ude=new Undo_CopyEvents(mo->song,eventpointer,c,mo->diff,mo->index);

				if(ude)
				{
					mo->song->undo.OpenUndoFunction(ude);
					Lock(mo->song);
					ude->Do();

					if(mo->song==mainvar->GetActiveSong())
					{
						if(ude->copied==true)
							mo->song->CheckPlaybackRefresh();
					}

					UnLock();

					CheckEditElementsForGUI(mo->song,ude,true);
				}
			}
			else
			{
				delete eventpointer;
#ifdef _DEBUG
				MessageBox(NULL,"Illegal Copy Event Function","Error",MB_OK_STYLE);
#endif
				return false;
			}
		}

		return true;
	}

	return false;
}

void Undo_MoveEvents::RefreshIndexs()
{
	for(int i=0;i<numberofevents;i++)
		events[i].seqevent->pattern->RefreshIndexs();
}

void Undo_MoveEvents::Do()
{
	UndoMoveEvent *ep=events;
	moved=false;

	for(int i=0;i<numberofevents;i++)
	{	
		ep->seqevent->pattern->checknotechain=true;
		ep->seqevent->pattern->eventclose=true;

		bool eventmoved=false;

		if(index){
			ep->seqevent->MoveIndex(index);
			eventmoved=true;
		}

		if(ep->diff){	
			eventmoved=true;
			ep->seqevent->MoveEvent(ep->diff);
		}

		if(eventmoved==true){
			mainMIDI->StopAllofEvent(song,ep->seqevent);
			moved=true;
		}

		ep++;
	}

	RefreshIndexs();

	ep=events;
	
	for(int i=0;i<numberofevents;i++)
	{
		if(ep->seqevent->pattern->checknotechain==true)
		{
			ep->seqevent->pattern->checknotechain=false;
			ep->seqevent->pattern->CloseEvents();
		}

		ep++;
	}
}

void Undo_MoveEvents::DoUndo()
{
	UndoMoveEvent *ep=events;

	for(int i=0;i<numberofevents;i++)
	{
		ep->seqevent->pattern->eventclose=true;

		mainMIDI->StopAllofEvent(song,ep->seqevent);

		if(index)
			ep->seqevent->MoveIndex(-index);

		if(ep->diff)
		{
			switch(ep->seqevent->pattern->mediatype)
			{
			case MEDIATYPE_MIDI:
				{
					MIDIPattern *mp=(MIDIPattern *)ep->seqevent->pattern;
					mp->events.CutObject(ep->seqevent);
					ep->seqevent->MoveEventQuick(-ep->diff);

					mp->checknotechain=true;
				}
				break;

			default:
				ep->seqevent->MoveEvent(-ep->diff);
				break;
			}
		}

		ep++;
	}

	// 2. Add MIDI Events at Index
	ep=events;
	for(int i=0;i<numberofevents;i++)
	{
		if(ep->diff)
		{
			switch(ep->seqevent->pattern->mediatype)
			{
			case MEDIATYPE_MIDI:
				{
					MIDIPattern *mp=(MIDIPattern *)ep->seqevent->pattern;

					mp->events.AddOToIndex(ep->seqevent,ep->oldindex);

					if(mp->checknotechain==true)
					{
						mp->checknotechain=false;
						mp->CloseEvents();
					}

				}
				break;
			}
		}

		ep++;
	}

	// 3. Refresh Indexs
	RefreshIndexs();
}

bool EditFunctions::MoveSelectedEventsInPatternList(MoveO *mo)
{
	if(mo && CheckIfEditOK(mo->song)==true)
	{
		OList selevents;
		int refreshmode=0,c=mo->sellist->GetCountofSelectedEvents();
		bool moved=false,error=false;

		if(mo->diff || mo->index)
		{
			UndoMoveEvent *pp,*eventpointer;

			pp=eventpointer=new UndoMoveEvent[c];

			if(pp)
			{
				Seq_SelectionEvent *sele=mo->sellist->FirstMixEvent();
				int c2=0;
				bool moved=false;

				while(sele && error==false)
				{
					if(sele->seqevent->IsSelected()==true)
					{
						if(c2<c)
						{
						//	if(mo->flag&MOVEPATTERN_FLAGEVENTDIFF)
						//		pp->diff=sele->changedposition;
						//	else
								pp->diff=mo->diff;

							OSTART npos=sele->seqevent->GetEventStart()+pp->diff;

							// Insert Quantize ?
							if(QuantizeEffect *qeff=sele->seqevent->pattern->GetQuantizer())
							{
								npos=qeff->Quantize(npos);
								pp->diff=npos-sele->seqevent->GetEventStart();
							}

							if(pp->diff)
								moved=true;

							pp->seqevent=sele->seqevent;
							pp->oldindex=sele->seqevent->GetIndex();
							
							pp++;
							c2++;
						}
						else 
							error=true;
					}

					sele=sele->NextEvent();
				}

				if(c2!=c)
					error=true;

				if(error==false)
				{
					if(moved==true || mo->index)
					{
						if(Undo_MoveEvents *ude=new Undo_MoveEvents(mo->song,eventpointer,c,mo->index))
						{
							Lock(mo->song);
							ude->Do();

							if(mo->song==mainvar->GetActiveSong())
							{
								if(ude->moved==true)
									mo->song->CheckPlaybackRefresh();
							}

							UnLock();

							mo->song->undo.OpenUndoFunction(ude);

							if(ude->moved==true)
								CheckEditElementsForGUI(mo->song,ude,true);
						}
						else
						{
							delete eventpointer;
							return false;
						}

					}
					else
					{
						delete eventpointer;
						return false;
					}

				}
				else
				{
					delete eventpointer;
#ifdef DEBUG
					MessageBox(NULL,"Illegal Move Event Function",Cxs[CXS_ERROR],MB_OK_STYLE);
#endif
					return false;
				}

				return true;
			}
		}

		mo->sellist->ResetChanges();

		return false;
	}

	return false;
}


UndoFunction *UndoEditEvents::CreateUndoFunction()
{
	return new Undo_EditEvents(this);
}
