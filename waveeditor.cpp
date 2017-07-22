#include "waveeditor.h"
#include "songmain.h"
#include "gui.h"
#include "undo.h"
#include "camxgadgets.h"
#include "editfunctions.h"
#include "undofunctions.h"
#include "object_song.h"
#include "objectpattern.h"
#include "object_track.h"
#include "mainhelpthread.h"
#include "editbuffer.h"
#include "waveeditordefines.h"
#include "audiohardware.h"
#include "menu_editwave.h"
#include "settings.h"
#include "MIDIPattern.h"

void Edit_Wave::MouseWheel(int delta,guiGadget *db)
{
	if(Edit_MouseWheel(delta)==true)
	{
		if(firsttrack && wavedefinition)
		{
			WaveTrack *wt=firsttrack;

			if(delta<0)
			{
				delta=-delta;

				while(delta-- && wt->PrevTrack())
					wt=wt->PrevTrack();
			}
			else
			{
				int ix=firsttrack->GetIndex();

				while(delta-- && wt->NextTrack())
					wt=wt->NextTrack();
			}

			if(wt && wt!=firsttrack)
			{
				firsttrack=wt;
				ShowWavesHoriz(SHOWEVENTS_TRACKS|SHOWEVENTS_EVENTS);
				ShowSlider();
			}
		}
	}
}

void Edit_Wave::MouseMove(bool inside)
{
	bool used=EditorMouseMove(inside);

	if(used==false && inside==true)
	{
		bool functionfound=false;

		switch(right_mousekey)
		{
		case MOUSEKEY_DOWN:
			{
				switch(mousemode)
				{
				case EM_EDIT:
				case EM_CREATE:
				case EM_DELETE:
					{
						if(DeleteEvent()==true)
							functionfound=true;
					}
					break;
				}
			}
			break;
		}

		if(functionfound==false)
			switch(mousemode)
		{
			case EM_SELECTOBJECTS:
				{
#ifdef OLDIE
					RemoveAllSprites(guiSprite::SPRITEDISPLAY_SELECTION);

					guiSprite *s=new guiSprite(guiSprite::SPRITETYPE_RECT,guiSprite::SPRITEDISPLAY_SELECTION);

					if(s)
					{
						WaveTrack *starttrack,*endtrack;

						starttrack=modestarttrack;
						Edit_Wave_Track *f=FindWaveAtYPosition(GetMouseY());

						if(f)
							endtrack=f->track;
						else
							endtrack=wavedefinition->LastTrack();

						if(starttrack && endtrack)
						{
							if(wavedefinition->GetOfWaveTrack(endtrack)<wavedefinition->GetOfWaveTrack(starttrack))
							{
								WaveTrack *h=endtrack;

								endtrack=starttrack;
								starttrack=h;
							}

							// X
							if(modestartposition<startposition)
							{
								s->x=frame_events.x;
								s->subtype|=SPRITETYPE_RECT_NOLEFT;
							}
							else
								if(modestartposition>endposition)
								{
									s->x=frame_events.x2;
									s->subtype|=SPRITETYPE_RECT_NORIGHT;
								}
								else
									s->x=timeline->ConvertTimeToX(modestartposition); // Set Startposition

							if(GetMouseX()>frame_events.x2)
								s->x2=frame_events.x2;
							else
								if(GetMouseX()<frame_events.x)
									s->x2=frame_events.x;
								else
									s->x2=GetMouseX();

							if(!FindTrack(modestarttrack) && f)
							{
								if(wavedefinition->GetOfWaveTrack(modestarttrack)<wavedefinition->GetOfWaveTrack(f->track))
								{
									s->y=frame_events.y;
									s->subtype|=SPRITETYPE_RECT_NOTOP;
								}
								else
								{
									s->y=frame_events.y2;
									s->subtype|=SPRITETYPE_RECT_NOBOTTOM;
								}
							}
							else
								s->y=modestarty;

							OSTART startposition=modestartposition;
							OSTART endposition=timeline->ConvertXPosToTime(GetMouseX());

							if(endposition<startposition)
							{
								OSTART h=endposition;

								endposition=startposition;
								startposition=h;
							}

							maingui->OpenPRepairSelection(&patternselection);

							while(starttrack)
							{
								Seq_SelectionEvent *e=patternselection.FirstMixEvent(startposition);

								while(e && e->event->GetEventStart()<=endposition)
								{	
									if(starttrack->CheckIfEventInside(e->event)==true)
									{
										maingui->PRepairSelection(e->event);
									}

									e=e->NextEvent();
								}

								starttrack=starttrack->NextTrack();
							}

							/*
							while(sel && sel->event->GetEventStart()<=endposition)
							{
							if(sel->event->GetStatus()==NOTEON)
							{
							Note *note=(Note *)sel->event;

							if(
							(note->GetEventStart()>=startposition || note->GetNoteEnd()>startposition) &&
							note->key>=startkey &&
							note->key<=endkey)
							{
							maingui->PRepairSelection(sel->event);
							}
							}

							sel=sel->NextEvent();
							}
							*/

							maingui->ClosePRepairSelection(&patternselection);

							s->colour=COLOUR_BLUE;

							s->x=frame_events.SetToX(s->x);
							s->y=frame_events.SetToY(s->y);
							s->x2=frame_events.SetToX(GetMouseX());
							s->y2=frame_events.SetToY(GetMouseY());

							AddSpriteShow(s);
						}
					}
#endif

				}
				break;

			case EM_CREATE:
				{
					if(left_mousekey==MOUSEKEY_DOWN)
						CreateEvent(false);
				}
				break;

			case EM_EDIT:
				if(left_mousekey==MOUSEKEY_DOWN)
					EditEvent();
				break;

			case EM_DELETE:		
				if(left_mousekey==MOUSEKEY_DOWN)
					DeleteEvent();

				break;
		}
	}
}

void Edit_Wave::SelectMap(WaveMap *map)
{
	if(map!=wavedefinition)
	{
		wavedefinition=map;

		if(map)
			focustrack=firsttrack=map->FirstTrack();
		else
			focustrack=firsttrack=0;

		RefreshMenu();

		RefreshObjects(1,false);

		trackfx.ShowActiveWaveTrack();
	}
}

void Edit_Wave::ReplaceTrackChannel(WaveTrack *track,int chl)
{
	if(track && chl>=0 && chl<=16)
	{
		guiWindow *w=maingui->FirstWindow();

		track->chl=chl;

		while(w)
		{
			if(w->WindowSong()==WindowSong() && w->GetEditorID()==EDITORTYPE_WAVE)
			{
				Edit_Wave *ck=(Edit_Wave *)w;

				if(ck->wavedefinition==wavedefinition)
				{
					if(ck->focustrack==track)
					{
						ck->trackfx.ShowActiveWaveTrack();
					}

					//	ck->ShowWaveTracks();

					ck->ShowEvents();
				}
			}

			w=w->NextWindow();
		}
	}
}

void Edit_Wave::ReplaceTrackWithTrack(WaveTrack *old,WaveTrack *newt)
{
	if(old && newt)
	{
	//	wavedefinition->tracks.ReplaceO(old,newt);

		delete old;

		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			if(w->WindowSong()==WindowSong() && w->GetEditorID()==EDITORTYPE_WAVE)
			{
				Edit_Wave *ck=(Edit_Wave *)w;

				if(ck->wavedefinition==wavedefinition)
				{
					if(ck->focustrack==old)
					{
						ck->focustrack=newt;

						ck->trackfx.ShowActiveWaveTrack();
					}

					if(ck->firsttrack==old)
						ck->firsttrack=newt;

					ck->ShowWaveTracks(-1);

					ck->ShowEvents();
				}
			}

			w=w->NextWindow();
		}
	}
}

void Edit_Wave::DeleteTrack(WaveTrack *track)
{
	if(wavedefinition)
	{
		WaveTrack *deletetrack=track;

		if(!deletetrack)
			deletetrack=focustrack;

		if(deletetrack)
		{
			bool ok=false;

			guiWindow *w=maingui->FirstWindow();

			while(w)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_WAVE:
					{
						Edit_Wave *wave=(Edit_Wave *)w;

						if(wave->firsttrack==deletetrack)
						{
							if(deletetrack->NextTrack())
								wave->firsttrack=deletetrack->NextTrack();
							else
								wave->firsttrack=deletetrack->PrevTrack();
						}

						if(wave->focustrack==deletetrack)
						{
							if(deletetrack->NextTrack())
								wave->NewActiveTrack(deletetrack->NextTrack());
							else
								wave->NewActiveTrack(deletetrack->PrevTrack());
						}
					}
					break;
				}

				w=w->NextWindow();
			}

			wavedefinition->DeleteTrack(deletetrack);
			ok=true;

			maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_WAVE,1); // 1=Full refresh
		}
	}	
}

void Edit_Wave::CreateNewTrack(WaveTrack *track)
{
	if(wavedefinition)
	{
		if(!track)
			wavedefinition->AddNewWaveTrack(focustrack,focustrack); // Clone active Track
		else
			wavedefinition->AddNewWaveTrack(track,track);

		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_WAVE:
				{
					Edit_Wave *wave=(Edit_Wave *)w;

					if(!wave->firsttrack)
					{
						wave->firsttrack=wave->wavedefinition->FirstTrack();
					}

					if(!wave->focustrack)
					{
						wave->NewActiveTrack(wave->wavedefinition->FirstTrack());
					}
				}
				break;
			}

			w=w->NextWindow();
		}

		maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_WAVE,1); // 1=Full refresh
	}	
}

void Edit_Wave::NewActiveTrack(WaveTrack *track)
{
	if(focustrack!=track)
	{
		focustrack=track;

		ShowWaveTracks(-1);
		ShowEvents();

		trackfx.ShowActiveWaveTrack();
	}
}

void Edit_Wave::KeyDown()
{
	Editor_KeyDown();

	switch(nVirtKey)
	{
	case KEY_CURSORUP:
		if(focustrack)
		{
			if(maingui->GetCtrlKey()==true)
			{
				MoveTrack(focustrack,-1);
			}
			else
			{
				if(focustrack->PrevTrack())
				{
					NewActiveTrack(focustrack->PrevTrack());
				}
			}
		}
		break;

	case KEY_CURSORDOWN:
		if(focustrack)
		{
			if(maingui->GetCtrlKey()==true)
			{
				MoveTrack(focustrack,1);
			}
			else
			{
				if(focustrack->NextTrack())
				{
					NewActiveTrack(focustrack->NextTrack());
				}
			}
		}
		break;
	}
}

void Edit_Wave::MoveTrack(WaveTrack *t,int diff)
{
	if(t && diff)
	{
		if(t->GetList()->MoveOIndex(t,diff)==true)
		{
			maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_WAVE,1); // 1== ShowTracks
		}
	}
}

void Edit_Wave::MouseButton(int flag)
{
#ifdef OLDIE
	bool functionfound=false;

	addtolastundo=false; // new undo

	if(CheckEditorMouseButton()==false)return;

	switch(right_mousekey)
	{
	case MOUSEKEY_DOWN:
		CheckPopMenu();
		break;
	}

	if(right_mousekey!=MOUSEKEY_UP)
	{
		switch(right_mousekey)
		{
		case MOUSEKEY_DOWN:
			{
				switch(mousemode)
				{
				case EM_EDIT:
				case EM_CREATE:
				case EM_DELETE:
					{
						DeleteEvent();
					}
					break;
				}

				ReleaseMouse();
			}
			break;
		}
	}
	else
		switch(left_mousekey)
	{
		case MOUSEKEY_DOWN:
			{
				guiObject *o=0;

				if(maingui->GetShiftKey()==false)
					o=guiobjects.CheckObjectClicked(GetMouseX(),GetMouseY());

				if(o) // Track selected ?
				{
					switch(o->id)
					{
					case OBJECTID_WAVETRACK:
						NewActiveTrack(((Edit_Wave_Track *)o)->track);
						functionfound=true;
						break;
					}
				}

				if(functionfound==false)
				{
					// Clicked in GFX Area
					if (frame_events.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
					{
						bool checkmode=true;

						switch(mousemode)
						{
						case EM_SELECT:
							if(timeline)
							{
								// unselect all Pattern
								if(maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false)
									patternselection.SelectAllEvents(false,SEL_ALLEVENTS);

								if(patternselection.FirstSelectionEvent(0,SEL_ALLEVENTS))
								{
									SetMouseMode(EM_SELECTOBJECTS);
								}

								guiTimeLinePos *pos=timeline->FindPositionX(GetMouseX());

								modestartposition=(!pos)?startposition:pos->GetStart();
							}
							break;

						case EM_CREATE:
							CreateEvent(true);
							break;

						case EM_EDIT:
							EditEvent();
							break;

						case EM_DELETE:
							DeleteEvent();
							break;
						}// switch mousemode

					}//if frame
				}//if functionfound
			}
			// MouseButton Down
			break;	

		case MOUSEKEY_UP:
			{
				switch(mousemode)
				{
				case EM_SELECTOBJECTS:
					{	
						SelectAllEvents();
					}
					break;
				}
			}
			break;

		case MOUSEKEY_DBCLICK_LEFT:
			{
				guiObject *o=guiobjects.CheckObjectClicked(GetMouseX(),GetMouseY());

				if(o) // Track selected ?
				{
					switch(o->id)
					{
					case OBJECTID_WAVETRACK:
						{
							Edit_Wave_Track *ewt=(Edit_Wave_Track *)o;
							CreateNewTrack(ewt->track);
						}
						break;
					}
				}
			}
			break;

	}// switch left MB

#endif

}

void Edit_Wave::SelectAll(bool on)
{
	if(focustrack)
	{
		bool found=false;
		Seq_SelectionEvent *e=patternselection.FirstMixEvent();

		while(e)
		{
			if(	
				((on==true && ((e->seqevent->flag&OFLAG_SELECTED)==0)) || (on==false && ((e->seqevent->flag&OFLAG_SELECTED)))) &&
				focustrack->CheckIfEventInside(e->seqevent)
				)
			{
				if(on==true)
					e->seqevent->flag|=OFLAG_SELECTED;
				else
					e->seqevent->flag CLEARBIT OFLAG_SELECTED;

				found=true;
			}

			e=e->NextEvent();
		}

		if(found==true)
			maingui->RefreshAllEditorsWithEvent(WindowSong(),0);
	}
}

bool Edit_Wave::ZoomGFX(int z,bool horiz)
{
#ifdef OLDIE
	if(SetZoomGFX(z,horiz)==true)
	{
		
		if(CreateGUIBuffer()==true)
			horiz=true;

		if(horiz==true)
			ShowWavesHoriz(SHOWEVENTS_HEADER|SHOWEVENTS_TRACKS|SHOWEVENTS_EVENTS);
		else
			ShowWavesHoriz(SHOWEVENTS_TRACKS|SHOWEVENTS_EVENTS);

		ShowSlider();
		BlitGUIInfos();
		return true;
	}
#endif

	return false;
}

Edit_Wave::Edit_Wave()
{
	editorid=EDITORTYPE_WAVE;
	editorname="Wave";

	inbound=false;
	wavedefinition=0;
	focustrack=0;

	default_notelength=SAMPLESPERBEAT/4;
	
	firsttrack=0;

	// FX
	showeffects=true;

	headerflag=0;

	// Init Track Fx
	trackfx.waveeditor=this;

	followsongposition=mainsettings->followeditor;

	trackcount=0;
}

void Edit_Wave::Goto(int to)
{
	UserEdit();

	if(focustrack)
	{
		Seq_SelectionEvent *el=0;

		if(CheckStandardGoto(to)==true)
			return;

		switch(to)
		{
		case GOTO_FIRST:
		case GOTO_FIRSTSELECTED:
			{
				el=patternselection.FirstMixEvent();

				while(el && 
					focustrack->CheckIfEventInside(el->seqevent)==false
					&&
					(to!=GOTO_FIRSTSELECTED || (el->seqevent->flag&OFLAG_SELECTED))
					)
					el=el->NextEvent();

				if(el && NewStartPosition(el->seqevent->GetEventStart(),true)==true)
				{
					SyncWithOtherEditors();
				}
			}
			break;

		case GOTO_LAST:
		case GOTO_LASTSELECTED:
			{
				el=patternselection.LastMixEvent();

				while(el && 
					focustrack->CheckIfEventInside(el->seqevent)==false &&
					(to!=GOTO_LASTSELECTED || (el->seqevent->flag&OFLAG_SELECTED))
					)
					el=el->PrevEvent();

				if(el && (NewStartPosition(el->seqevent->GetEventStart(),true)==true))
				{
					SyncWithOtherEditors();
				}
			}
			break;

		case GOTO_FIRSTEVENTTRACK:
			if(WindowSong()->GetFocusTrack())
			{
				el=patternselection.FirstMixEvent();
				Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

				while(el && 
					(el->seqevent->GetPattern()->GetTrack()!=WindowSong()->GetFocusTrack() ||
					focustrack->CheckIfEventInside(el->seqevent)==false
					)
					)
					el=el->NextEvent();

				if(el && NewStartPosition(el->seqevent->GetEventStart(),true)==true)
				{
					SyncWithOtherEditors();
				}
			}
			break;

		case GOTO_LASTEVENTTRACK:
			if(WindowSong()->GetFocusTrack())
			{
				el=patternselection.LastMixEvent();
				Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

				while(el && 
					(el->seqevent->GetPattern()->GetTrack()!=WindowSong()->GetFocusTrack() || focustrack->CheckIfEventInside(el->seqevent)==false)
					)
					el=el->PrevEvent();

				if(el && NewStartPosition(el->seqevent->GetEventStart(),true)==true)
				{
					SyncWithOtherEditors();
				}
			}
			break;
		}

		if(el && NewStartPosition(el->seqevent->GetEventStart(),true,el)==true)
		{
			SyncWithOtherEditors();	
		}
	}
}

// Buttons,Slider ...
void Edit_Wave::Gadget(guiGadget *gadget)
{	
	gadget=Editor_Gadget(gadget);

	if(gadget)
	{
		gadget=trackfx.Gadget(gadget);

		if(gadget)
		{
			switch(gadget->gadgetID)
			{
			case GADGETID_EDITORSLIDER_VERT: // Track Scroll
				{
					WaveTrack *t=wavedefinition->GetWaveTrackIndex(gadget->GetPos());

					if(t!=firsttrack)
					{
						firsttrack=t;
						ShowWavesHoriz(SHOWEVENTS_TRACKS|SHOWEVENTS_EVENTS);
					}
				}
				break;

			case GADGETID_EDITORSLIDER_VERTZOOM:
				ZoomGFX(gadget->GetPos());
				break;

			default:

				break;
			}
		}
	}
}

Edit_Wave_Track *Edit_Wave::FindWaveAtYPosition(int y)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_WAVETRACK)
		{	
			if(o->y<=y && o->y2>=y)
				return (Edit_Wave_Track *)o;
		}

		o=o->NextObject();
	}

	return 0;
}

int Edit_Wave_Track::GetTrackValue(int y)
{
	double per;
	double h;

	per=virtualy2-virtualy;

	h=virtualy2-y;
	h/=per;
	h*=track->maxvalue;

	return (int)h;
}

void Edit_Wave::SetMouseMode(int newmode)
{
#ifdef OLDIE
	modestartposition=GetMousePosition();

	if(modestartposition>=0)
	{
		SetEditorMode(newmode);

		Edit_Wave_Track *ewt=FindWaveAtYPosition(GetMouseY());

		if(ewt)
			modestarttrack=ewt->track;
		else
			modestarttrack=0;

		modestartx=GetMouseX();
		modestarty=GetMouseY();

		SetAutoScroll(
			newmode,
			frame_events.x,
			frame_events.y,
			frame_events.x2,
			frame_events.y2
			);
	}
#endif
}

void Edit_Wave::AutoScroll()
{
	// MessageBeep(-1);

	switch(mousemode)
	{
	case EM_SELECTOBJECTS:
		{
			if(autoscrollstatus&AUTOSCROLL_LEFT)
			{
				if(ScrollMeasures(-1)==true)
					TimeChange();
			}

			if(autoscrollstatus&AUTOSCROLL_RIGHT)
			{
				if(ScrollMeasures(1)==true)
					TimeChange();
			}

			if(autoscrollstatus&AUTOSCROLL_UP)
			{
				/*
				if(lastkey<127)
				{
				startkey++;
				Init();
				}
				*/
			}

			if(autoscrollstatus&AUTOSCROLL_DOWN)
			{
				/*
				if(startkey)
				{
				startkey--;
				Init();
				}
				*/

			}
		}
		break;
	}
}

void Edit_Wave::CheckPopMenu()
{
#ifdef OLDIE
	if(guiObject *o=guiobjects.CheckObjectClicked(GetMouseX(),GetMouseY()))
	{
		switch(o->id)
		{
		case OBJECTID_WAVETRACK:
			{
				if(DeletePopUpMenu(true))
				{	
					popmenu->AddFMenu(Cxs[CXS_CREATENEWTRACK],new menu_waveeditor_createnewtrack(this,((Edit_Wave_Track *)o)->track));
					popmenu->AddFMenu(Cxs[CXS_DELETE],new menu_waveeditor_deletetrack(this,((Edit_Wave_Track *)o)->track));

					ShowPopMenu();
				}
			}
			break;
		}
	}
#endif

}

void Edit_Wave::EditEvent()
{
#ifdef OLDIE
	if(frame_events.ondisplay==true && frame_events.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
	{
		Edit_Wave_Track *mousewave=FindWaveAtYPosition(GetMouseY());

		if(mousewave && timeline->FirstPosition())
		{	
			OSTART start=-1;
			OSTART end;
			int c=0;

			int mousevalue=mousewave->GetTrackValue(GetMouseY());

			guiTimeLinePos *pos=timeline->FindPositionX(GetMouseX());

			if(!pos) // |---
			{
				pos=timeline->LastPosition();

				start=pos->GetStart();
				end=endposition;
			}
			else // ---|  
			{
				start=pos->GetStart();

				if(!pos->NextPosition())
					end=endposition;
				else
					end=pos->NextPosition()->GetStart();
			}

			if(start!=-1)
			{
				Seq_Event *event=patternselection.FirstSelectionEvent(start,SEL_ALLEVENTS);

				while(event && event->GetEventStart()<end)
				{	
					editevent.event=0; // reset

					if(mousewave->track->CheckIfEventInside(event)==true)
					{
						if(mousewave->track->GetMSB(event)!=mousevalue)
						{
							editevent.event=(Seq_Event *)event->Clone();

							if(editevent.event)
							{
								mousewave->track->SetMSB(editevent.event,mousevalue);

								c++;

								editevent.song=WindowSong();
								editevent.doundo=true;
								editevent.addtolastundo=true;
								editevent.checkgui=false; // manual refresh
								editevent.playit=false;

								mainedit->EditEventData(event,&editevent);

								editevent.event->Delete(true);
							}
						}
					}

					event=patternselection.NextSelectionEvent(SEL_ALLEVENTS);
				}// while event

				if(c)
				{
					// MessageBeep(-1);
					mainedit->CheckEditElementsForGUI(WindowSong(),WindowSong()->undo.LastUndo(),true);
				}
			}
		}
	}
#endif

}

void Edit_Wave::CreateEvent(bool openundo)
{
#ifdef OLDIE
	if(frame_events.ondisplay==true && frame_events.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
	{
		Edit_Wave_Track *mousewave=FindWaveAtYPosition(GetMouseY());

		if(mousewave && 
			insertpattern &&
			insertpattern->mediatype==MEDIATYPE_MIDI &&
			mousewave->track->chl>0
			) // No Event Create Chl==All Channels
		{
			//	Seq_Event *newevent;
			WaveTrack *track=mousewave->track;

			bool create=true;

			int mousevalue=mousewave->GetTrackValue(GetMouseY());

			OSTART pos=GetMousePosition();

			// Insert Quantize ?
			{
				QuantizeEffect *qeff=insertpattern->GetQuantizer();

				if(qeff)
					pos=qeff->Quantize(pos);
			}

			// Find Event in insertpattern
			Seq_Event *check=((MIDIPattern *)insertpattern)->FindEventAtPosition(pos);

			while(check && check->GetEventStart()<=pos && create==true)
			{
				if(mousewave->track->CheckIfEventExists(check,pos)==true)
					create=false;

				check=check->NextEvent();
			}

			if(create==true)
			{
				editevent.song=WindowSong();
				editevent.pattern=(MIDIPattern *)insertpattern;
				editevent.event=mousewave->track->CreateNewEvent(WindowSong(),pos,0,mousewave->track->chl-1,mousevalue);

				if(editevent.event)
				{
					editevent.event->ostart=editevent.event->staticostart=pos; // Set Position

					editevent.position=pos;

					editevent.doundo=true;
					editevent.addtolastundo=true;
					editevent.playit=true;

					//	mainMIDI->SendMIDIRealtimeEvent(mp->track,mp,note,end-pos); // Play Note

					mainedit->CreateNewMIDIEvent(&editevent);
				}
			}
		}
	}
#endif

}

bool Edit_Wave::DeleteEvent()
{		
#ifdef OLDIE
	int c=0;

	if(frame_events.ondisplay==true && frame_events.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
	{
		Edit_Wave_Track *mousewave=FindWaveAtYPosition(GetMouseY());

		if(mousewave && timeline->FirstPosition())
		{
			OSTART start=-1;
			OSTART end;

			guiTimeLinePos *pos=timeline->FindPositionX(GetMouseX());

			if(!pos) // |---
			{
				pos=timeline->LastPosition();

				start=pos->GetStart();
				end=endposition;
			}
			else // ---|  
			{
				start=pos->GetStart();

				if(!pos->NextPosition())
					end=endposition;
				else
					end=pos->NextPosition()->GetStart();
			}

			if(start!=-1)
			{
				Seq_Event *event=patternselection.FirstSelectionEvent(start,SEL_ALLEVENTS);

				while(event && event->GetEventStart()<end)
				{	
					if(mousewave->track->CheckIfEventInside(event)==true)
					{
						// Delete Event
						mainedit->DeleteEvent(event,addtolastundo);
						addtolastundo=true;

						c++;
					}

					event=patternselection.NextSelectionEvent(0);
				}// while event
			}
		}
	}

	if(c)
		return true;
#endif

	return false;
}

void Edit_Wave::ShowEvent(Seq_SelectionEvent *e,bool direct)
{	
#ifdef OLDIE
	if((e->x2>e->x+5) && (e->y2>e->y+6))
	{
		int colour;

		if(e->event->flag&OFLAG_SELECTED)
			colour=COLOUR_SELECTED;
		else
			colour=COLOUR_GREY_LIGHT;

		// Y Pos
		double h=(double)e->helpvalue;
		int y;

		h/=(double)127;
		h*=(double)((e->y2-1)-(e->y+1));

		y=e->y2-(int)h;

		guibuffer->guiFillRect3D(e->x+1,e->y+1,e->x2-1,e->y2-1,colour);
		guibuffer->guiFillRect(e->x+2,y,e->x2-2,e->y2,COLOUR_GREEN);

		if(e->event->flag&OFLAG_UNDERSELECTION)
			guibuffer->guiInvert(e->x,e->y,e->x2,e->y2);
	}
	else
	{
		int colour;

		if(e->event->flag&OFLAG_UNDERSELECTION)
			colour=COLOUR_BLACK;
		else
			if(e->event->flag&OFLAG_SELECTED)
				colour=COLOUR_SELECTED;
			else
				colour=COLOUR_GREEN;

		guibuffer->guiFillRect(e->x,e->y,e->x2,e->y2,colour);
	}

	if(direct==true)
		BltGUIBuffer(e->x,e->y,e->x2,e->y2);
#endif

}

void Edit_Wave::ShowEvents(int y)
{
#ifdef OLDIE
	patternselection.ClearFlags();

	if(y!=-1)
	{
		frame_events.y=y;
	}

	frame_events.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ);

	if(timeline && 
		frame_events.ondisplay==true && 
		timeline->FirstPosition() &&
		wavedefinition)
	{
		int maxvalue;
		int x,x2;
		OSTART nextsigpoint;
		bool selected;

		ShowWaveRaster();

		patternselection.CopyStatus();

		guiObject *ot=guiobjects.FirstObject();

		while(ot)
		{
			if(ot->id==OBJECTID_WAVETRACK)
			{
				Edit_Wave_Track *t=(Edit_Wave_Track *)ot;

				guiTimeLinePos *pos=timeline->FirstPosition();

				x=timeline->x+1;

				if(startposition<pos->GetStart())
				{
					nextsigpoint=pos->GetStart();

					x2=pos->x;
				}
				else
				{
					pos=pos->NextPosition();

					if(pos)
					{
						nextsigpoint=pos->GetStart();
						x2=pos->x;
					}
					else
					{
						nextsigpoint=endposition;
						x2=timeline->x2;
					}
				}

				Seq_SelectionEvent *selevent=patternselection.FirstMixEvent(startposition);

				while(selevent)
				{	
					// reset
					maxvalue=-1;
					selected=false;

					// Find Maxvalue
					while(
						selevent && 
						(selevent->event->GetEventStart()<nextsigpoint)
						)
					{	
						if(t->track->CheckIfEventInside(selevent->event)==true)
						{
							if(!(selevent->flag&SEQSEL_ONDISPLAY))
							{
								selevent->flag|=SEQSEL_ONDISPLAY;

								selevent->x=x;
								selevent->y=ot->y+1;
								selevent->x2=x2;
								selevent->y2=ot->y2-1;

								selevent->helpvalue=t->track->GetMSB(selevent->event);
								ShowEvent(selevent,false);
							}
							else // MultiDisplay
							{
								if(Seq_SelectionEvent *newsel=new Seq_SelectionEvent)
								{
									newsel->flag|=SEQSEL_ONDISPLAY;

									newsel->x=x;
									newsel->y=ot->y+1;
									newsel->x2=x2;
									newsel->y2=ot->y2-1;

									newsel->helpvalue=t->track->GetMSB(newsel->event=selevent->event);

									selevent->AddMoreEvent(newsel);

									ShowEvent(newsel,false);
								}
							}
						}
						//	}

						selevent=selevent->NextEvent();
					} // while event

					// Reset
					if((!pos) || (!selevent))break;

					pos=pos->NextPosition();

					x=x2;

					if(pos)
					{
						nextsigpoint=pos->GetStart();
						x2=pos->x;
					}
					else
					{
						nextsigpoint=endposition;
						x2=timeline->x2;
					}
				}// while event

				ot=ot->NextObject();
			}
		}

		BltGUIBuffer_Frame(&frame_events);
	}// if header
#endif

}

void Edit_Wave::ShowWaveTracks(int y)
{
	guiobjects.RemoveOs(0); // wave tracks


#ifdef OLDIE
	trackcount=0;

	if(y!=-1)
		frame_waves.y=y;

	frame_waves.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,0);

	if(frame_waves.ondisplay==true)
	{	
		frame_waves.Fill3D(guibuffer,frame_waves.bpen);

		if(wavedefinition)
		{
			Edit_Wave_Track *newtrack;

			int y=frame_waves.y;
			int colour=COLOUR_GREY_LIGHT;

			WaveTrack *t=firsttrack;

			while(t && y<frame_waves.y2)
			{
				char *type=0;

				newtrack=new class Edit_Wave_Track;

				int y2=y;

				if(t==focustrack)
				{
					y2+=zoomy*3;
					guibuffer->guiFillRect3D(frame_waves.x,y-1,frame_waves.x2,y2+1,COLOUR_GREY);
				}
				else
				{
					y2+=zoomy;
					guibuffer->guiDrawLine(frame_waves.x,y2,frame_waves.x2,y2,COLOUR_BLACK);
				}

				if(newtrack)
				{
					// Set Virtual Y Position
					newtrack->virtualy=y;
					newtrack->virtualy2=y2;

					if(y2<=height-EDITOR_SLIDER_SIZE_HORZ)
						trackcount++;
				}

				//	guibuffer->guiFillRect(frame_waves.x+1,y,frame_waves.x2,y2,colour);

				//	guibuffer->guiDrawLine(frame_waves.x,y,frame_waves.x2,y,COLOUR_BLACK);

				// Show Track Names
				guibuffer->guiDrawText(frame_waves.x+1,y2-1,frame_waves.x2,t->name);

				if(newtrack)
				{
					newtrack->track=t;

					guiobjects.AddGUIObject
						(
						frame_waves.x+1,
						y+1,
						frame_waves.x2,
						y2,
						&frame_waves,
						newtrack
						);
				}

				if(colour==COLOUR_GREY_LIGHT)
					colour=COLOUR_WHITE;		
				else
					colour=COLOUR_GREY_LIGHT;

				y=y2;

				t=t->NextTrack();
			}// while t
		}

		BltGUIBuffer_Frame(&frame_waves);
	}
#endif

}

void Edit_Wave::ShowWaveRaster()
{
#ifdef OLDIE
	if(timeline && frame_events.ondisplay==true)
	{
		frame_events.Fill(guibuffer,COLOUR_WHITE);

		// Track Rect
		{
			guiObject *ot=guiobjects.FirstObject();

			while(ot && ot->id==OBJECTID_WAVETRACK)
			{
				Edit_Wave_Track *ewt=(Edit_Wave_Track *)ot;

				guibuffer->guiFillRect(frame_events.x,ot->y,frame_events.x2,ot->y2,ewt->track->colour);

				ot=ot->NextObject();
			}
		}

		// Marker ?
		if(frame_events.y+4<frame_events.y2)
		{
			// Cycle
			if(WindowSong()->playbacksettings.cyclestart<endposition && WindowSong()->playbacksettings.cycleend>startposition)
			{
				int mx=-1;
				int mx2=-1;

				if(WindowSong()->playbacksettings.cyclestart>=startposition)
					mx=timeline->ConvertTimeToX(WindowSong()->playbacksettings.cyclestart);
				else
					mx=timeline->x;

				if(WindowSong()->playbacksettings.cycleend>endposition)
					mx2=timeline->x2;
				else
					mx2=timeline->ConvertTimeToX(WindowSong()->playbacksettings.cycleend);

				if(mx2!=-1)
					guibuffer->guiFillRect_RGB(mx,frame_events.y+1,mx2,frame_events.y2-1,0xDEDEDE);
			}

			Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();

			while(m && m->GetMarkerStart()<=endposition)
			{
				int mx=-1;
				int mx2=-1;

				if(m->GetMarkerStart()>=startposition)
				{
					mx=timeline->ConvertTimeToX(m->GetMarkerStart());

					if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
					{
						if(m->GetMarkerEnd()>=endposition)
							mx2=timeline->x2;
						else
							mx2=timeline->ConvertTimeToX(m->GetMarkerEnd());
					}
				}
				else // Start <<....|
				{
					if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
					{
						if(m->GetMarkerEnd()>startposition)
						{
							mx=timeline->x;

							if(m->GetMarkerEnd()>=endposition)
								mx2=timeline->x2;
							else
								mx2=timeline->ConvertTimeToX(m->GetMarkerEnd());
						}
					}
				}

				if(mx!=-1 && (mx2!=-1 || m->markertype==Seq_Marker::MARKERTYPE_SINGLE))
				{
					if(m->markertype==Seq_Marker::MARKERTYPE_SINGLE)
					{
						guibuffer->guiDrawLine(mx,frame_events.y,mx,frame_events.y2,COLOUR_BLACK);

						if(m->colour.showcolour==true)
						{
							for(int hx=mx+1,i=4;i>0;hx+=2,i--)
							{
								if(hx<timeline->x2)
									guibuffer->guiDrawLine_RGB(hx,frame_events.y+1,hx,frame_events.y2-1,m->colour.rgb);
							}
						}
						else
						{
							for(int hx=mx+1,i=4;i>0;hx+=2,i--)
							{
								if(hx<timeline->x2)
									guibuffer->guiDrawLine(hx,frame_events.y+1,hx,frame_events.y2-1,COLOUR_BLACK);
							}
						}
					}
					else
					{
						if(m->colour.showcolour==true)
						{
							guibuffer->guiFillRect_RGB
								(
								mx,
								frame_events.y,
								mx2,
								frame_events.y2,
								m->colour.rgb);
						}
						else
						{
							guibuffer->guiFillRect_RGB
								(
								mx,
								frame_events.y,
								mx2,
								frame_events.y2,
								COLOUR_BLUE_LIGHT);
						}
					}
				}

				m=m->NextMarker();
			}
		}

		// Track Lines
		{
			guiObject *ot=guiobjects.FirstObject();

			while(ot && ot->id==OBJECTID_WAVETRACK)
			{
				Edit_Wave_Track *ewt=(Edit_Wave_Track *)ot;

				guibuffer->guiDrawLine(frame_events.x,ot->y2,frame_events.x2,ot->y2,COLOUR_BLACK);
				ot=ot->NextObject();
			}
		}

		guiTimeLinePos *pos=timeline->FirstPosition();

		while(pos && pos->x<=frame_events.x2)
		{
			int colour;

			switch(pos->type)
			{
			case HEAD_POSITION_x1xx:
				colour=COLOUR_GREY;
				break;

			case HEAD_POSITION_xx1x:
				colour=COLOUR_GREY_LIGHT;
				break;

			default:
				colour=COLOUR_BLACK;
				break;
			}

			guibuffer->guiDrawLine(pos->x,frame_events.y,pos->x,frame_events.y2,colour);

			pos=pos->NextPosition();
		}
	}
#endif

}

guiMenu *Edit_Wave::CreateMenu()
{	
//	ResetUndoMenu();

#ifdef OLDIE
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		AddEventEditorMenu(menu);

		// Wave Editor Menu
		if(functionsmenu)
		{
			functionsmenu->AddFMenu("Create new Map",new menu_waveeditor_createnewmap(this));
			functionsmenu->AddFMenu("Delete Map",new menu_waveeditor_deletemap(this));

			// List of Maps
			WaveMap *map=mainwavemap->FirstWaveMap();

			if(map)
			{
				guiMenu *s=functionsmenu->AddMenu("Select Map",0);

				if(s)
				{	
					while(map)
					{
						s->AddFMenu(map->name,new menu_waveeditor_selectmap(this,map),map==wavedefinition?true:false);
						map=map->NextMap();
					}
				}
			}
			functionsmenu->AddLine();

			functionsmenu->AddFMenu("Create new Track",new menu_waveeditor_createnewtrack(this,0));
			functionsmenu->AddFMenu("Delete Track",new menu_waveeditor_deletetrack(this,0));
		}
	}

	maingui->AddCascadeMenu(this,menu);
#endif

	return menu;
}

void Edit_Wave::ShowMouse(OSTART time)
{
#ifdef OLDIE
	if(timeline && frame_events.ondisplay==true)
	{
		Edit_Wave_Track *t=FindWaveAtYPosition(GetMouseY());

		if(t)
		{
			bool show=false;

			if(mousehorzy1.ondisplay==false || (mousehorzy1.y!=t->y))
			{
				ClearSprite(&mousehorzy1);
				// Horz
				mousehorzy1.x=frame_events.x;
				mousehorzy1.y=t->y;
				mousehorzy1.x2=frame_events.x2;
				mousehorzy1.on=true;

				AddSpriteShow(&mousehorzy1);
				show=true;
			}

			if(mousehorzy2.ondisplay==false || mousehorzy2.y!=t->y2)
			{
				ClearSprite(&mousehorzy2);

				if(t->y2<frame_events.y2)
				{
					mousehorzy2.x=frame_events.x;
					mousehorzy2.y=t->y2;
					mousehorzy2.x2=frame_events.x2;
					mousehorzy2.on=true;

					AddSpriteShow(&mousehorzy2);
					show=true;
				}
			}

			if(show==true)
				ShowAllSprites();
		}
		else
		{
			ClearSprite(&mousehorzy1);
			ClearSprite(&mousehorzy2);

			mousehorzy1.on=false;
			mousehorzy2.on=false;
		}
	}
#endif

}

void Edit_Wave::ShowMenu()
{
	ShowEditorMenu();
}

void Edit_Wave::ShowWavesHoriz(int flag)
{
#ifdef OLDIE
	int y=-1;

	if(guibuffer)
	{
		if(flag&SHOWEVENTS_HEADER)
			ShowEditorHeader(0);

		if(guictrlbox.stopbutton && (!timeline))
			y=guictrlbox.stopbutton->y2+4+maingui->GetFontSizeY(); // + Header

		if(flag&SHOWEVENTS_TRACKS)
			ShowWaveTracks(y);

		if(flag&SHOWEVENTS_EVENTS)
			ShowEvents(y);
	}
#endif
}

void Edit_Wave::DeInitWindow()
{
	guiobjects.RemoveOs(0);

	if(winmode&WINDOWMODE_INIT)
	{
		
		patternselection.ClearFlags();

		if(bound==false)
			CloseHeader();

		//ResetAllGadgets();

		trackfx.FreeMemory();

		if(winmode&WINDOWMODE_DESTROY)
		{
			patternselection.ClearEventList(); // Delete Event List
		
		}
	}		
}


bool Edit_Wave::EditCancel()
{
	switch(mousemode)
	{
	case EM_SELECTOBJECTS:
		{
			SetEditorMode(EM_RESET);
				return true;
		}
		break;
	}

	return false;
}

char *Edit_Wave::GetWindowName()
{
	char h[64],h2[32];
	strcpy(h,"-E:");
	mainvar->AddString(h,mainvar->ConvertIntToChar(getcountselectedevents=patternselection.GetCountofSelectedEvents(),h2));
	mainvar->AddString(h,"/");
	mainvar->AddString(h,mainvar->ConvertIntToChar(getcountevents=patternselection.GetCountOfEvents(),h2));

	if(windowname)
	{
		if(char *hn=mainvar->GenerateString(windowname,h))
		{
			delete windowname;
			windowname=hn;
		}
	}

	return windowname;
}


void Edit_Wave::RefreshRealtime_Slow()
{
	int h_getcountselectedevents=patternselection.GetCountofSelectedEvents(),
		h_getcountevents=patternselection.GetCountOfEvents();

	if(h_getcountselectedevents!=getcountselectedevents ||
		h_getcountevents!=getcountevents)
		SongNameRefresh();
}

void Edit_Wave::ShowSlider()
{
	// Show Slider
	if(vertgadget && wavedefinition)
	{
		int ix=firsttrack->GetIndex();
		vertgadget->ChangeSlider(0,wavedefinition->GetCountOfWaveTracks()-trackcount,ix);
	}
}

void Edit_Wave::InitGadgets()
{
#ifdef OLDIE
	RemoveEditorSlider();

	if(frame_events.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,0)==true)
	{
		SliderCo horz,vert;

		if(wavedefinition)
			vert.to=wavedefinition->GetCountOfWaveTracks()-trackcount;
		else
			vert.to=0;

		AddEditorSlider(&horz,&vert,frame_events.x,frame_events.y);
		SetEditRange(&frame_events);
	}
#endif

}



void Edit_Wave::Init()
{		

#ifdef OLDIE
	{
		ToolTipOff();
	//	InitFrames();
		RemoveAllSprites();
	}

	if((!firsttrack) && wavedefinition)
		firsttrack=wavedefinition->FirstTrack();

	if(guibuffer && width && height)
	{
		if((winmode&WINDOWMODE_INIT) && ((winmode&(WINDOWMODE_RESIZE|WINDOWMODE_FRAMES))==0))
		{
			patternselection.BuildEventList(SEL_ALLEVENTS,0); // Mix new List
			ShowMenu();
		}

		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
		{
			ToolTipOff();
			InitFrames();

			// ClearCursor();

			if((!focustrack) && wavedefinition)
				focustrack=wavedefinition->FirstTrack();

			if(bound==true)
			{
				/*
				frame_header.on=false;///

				frame_waves=boundframe.boundleft;
				frame_events=boundframe.boundright;
				*/
			}
			else
			{
				RemoveAllSprites();

				guitoolbox.CreateToolBox(TOOLBOXTYPE_EDITOR);

				/*
				frame_patternedit.x=guitoolbox.x2;
				frame_patternedit.y=0;
				frame_patternedit.x2=width;
				//frame_patternedit.y2=guitoolbox.GetY2(0);

				if(frame_patternedit.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ)==true)
				{
					if(patternselection.FirstSelectedPattern())
						InitSelectionPatternGadget();
				}
*/

				guictrlbox.x=0;
				//guictrlbox.y=guitoolbox.GetY2(0);
				guictrlbox.CreateControlBox(this);

				if(showeffects==true)
				{
					frame_fx.on=true;

					frame_fx.x=0;
					frame_fx.x2=100;

				//	frame_fx.y=guictrlbox.GetY2(guitoolbox.GetY2(0));
				//	frame_fx.y2=height;

					if(frame_fx.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ)==true)
						trackfx.Init();

					frame_waves.x=frame_fx.x2+1;
				}
				else
				{
					frame_fx.on=false;
					frame_waves.x=0;
				}

				/*
				if(bound==false)
				{
				SliderCo horz,vert;

				vert.from=0;

				if(wavedefinition)
				vert.to=wavedefinition->GetCountOfWaveTracks();
				else
				vert.to=0;

				vert.pos=0;

				AddEditorSlider(&horz,&vert,frame_waves.x+frame_waves.value+1+4);
				}
				*/
			}
		}

		frame_waves.on=true;
		frame_waves.x2=frame_waves.x+frame_waves.value;

		frame_header.on=true;
		frame_events.x=frame_header.x=frame_waves.x2+1+4;
		frame_header.y=guictrlbox.y2+1;
		frame_header.y2=frame_header.y+frame_header.value;

		frame_events.on=true;
		frame_events.y=frame_waves.y=frame_header.y2+1;
		frame_events.x2=frame_header.x2=width;

		frame_waves.y2=frame_events.y2=height;

		frame_header.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ);
		ShowWavesHoriz(SHOWEVENTS_TRACKS|SHOWEVENTS_EVENTS|SHOWEVENTS_HEADER);

		frame_waves.min_x2=frame_waves.x+40;
		frame_waves.max_x2=frame_waves.x+300;

		AddFrame(&frame_waves,WAVE_FRAMEID_TRACKS,GUIFRAME_RIGHT);
		AddFrame(&frame_events,WAVE_FRAMEID_EVENTS,0);

		SetEditRange(&frame_events);

		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE|WINDOWMODE_FRAMES))
			InitGadgets();
	}
	else
	{
		DisableFrame();
	}
#endif

}

void Edit_Wave::RefreshObjects(LONGLONG par,bool editcall)
{
	if(editcall==true)
	{
		if(((refreshflag&REFRESHEVENT_LIST)==0) && patternselection.CheckEventList()==true)
		{
			refreshflag|=REFRESHEVENT_LIST;
		}
	}


	if(patternselection.patternremoved==true)
		ShowSelectionPattern();

	if(par==1)
		ShowWaveTracks(-1);

	ShowWaveRaster();

	if(refreshflag&REFRESHEVENT_LIST)
		ShowAllEvents(0);
	else
		ShowAllEvents(NOBUILD_REFRESH);

	patternselection.patternremoved=false;
}

void Edit_Wave::RefreshRealtime()
{
	RefreshEventEditorRealtime();

	// New Events added - Recording ?
	Seq_SelectedPattern *s=patternselection.FirstSelectedPattern();
	while(s)
	{
		if(s->status_nrpatternevents!=s->pattern->GetCountOfEvents())
		{
			ShowEvents();
			break;
		}

		s=s->NextSelectedPattern();
	}

	RefreshToolTip();
}
