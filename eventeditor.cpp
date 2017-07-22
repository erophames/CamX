#include "songmain.h"
#include "editor_event.h"

#include "gui.h"
#include "camxgadgets.h"
#include "guigadgets.h"
#include "seqtime.h"
#include "objectevent.h"
#include "object_song.h"

#include "audiofile.h"
#include "icdobject.h"
#include "MIDIhardware.h"
#include "MIDIPattern.h"
#include "editfunctions.h"
#include "settings.h"
#include "audiohardware.h"
#include "eventeditordefines.h"
#include "languagefiles.h"
#include "songmain.h"
#include "audiohdfile.h"
#include "object_track.h"
#include "undofunctions.h"
#include "semapores.h"
#include "editdata.h"
#include "audioevent.h"
#include "audiopattern.h"
#include "editortabs.h"
#include "drumevent.h"


enum
{
	EVENTEDIT_NOTELENGTH_1000=30,
	EVENTEDIT_NOTELENGTH_0100,
	EVENTEDIT_NOTELENGTH_0010,
	EVENTEDIT_NOTELENGTH_0001,

	EVENTEDIT_NOTELENGTH_HOUR=40,
	EVENTEDIT_NOTELENGTH_MIN,
	EVENTEDIT_NOTELENGTH_SEC,
	EVENTEDIT_NOTELENGTH_FRAME,
	EVENTEDIT_NOTELENGTH_QFRAME
};

enum
{
	CURSOR_POS_1000=0, // Measre/hour
	CURSOR_POS_0100, // /min
	CURSOR_POS_0010,// sec
	CURSOR_POS_0001, // frame
	CURSOR_POS_00001,// qframe

	CURSOR_STATUS,
	CURSOR_CHANNEL,
	CURSOR_BYTE1,
	CURSOR_BYTE2,
	CURSOR_VELOOFF,

	CURSOR_LEN_1000, // Measre/hour
	CURSOR_LEN_0100,// /min
	CURSOR_LEN_0010,// sec
	CURSOR_LEN_0001, // frame
	CURSOR_LEN_00001,// qframe
};

char *headerstrings[]=
{
	"Position",
	"Status     ",
	"Chl   ",
	"Byte1    ",
	"Byte2    ",
	"Byte3    ",
	"Data     ",
	"", // Length
	"Info    "
};

enum{
	GADGETID_DATA=GADGETID_EDITORBASE,
	GADGETID_FILTERID,
	GADGETID_FILTERNOTES,
	GADGETID_FILTERCONTROL,
	GADGETID_FILTERPITCH,
	GADGETID_FILTERCPRESS,
	GADGETID_FILTERPPRESS,
	GADGETID_FILTERPROGRAM,
	GADGETID_FILTERSYSEX,
	GADGETID_FILTERRESET
};

enum
{
	EDITID_PROGTEXT=EventEditor::EDITOREDITDATA_ID
};

void Edit_Event::ShowCreateButton()
{
	if(createeventgadget)
	{
		char h[200];

		strcpy(h,Cxs[CXS_CREATE]);

		if(createeventtype!=CREATE_SYSEX)
		{
			mainvar->AddString(h,Cxs[CXS_MIDICHANNEL]);
			mainvar->AddString(h," ");
			mainvar->AddString(h,MIDIchannels[createchannel]);
			mainvar->AddString(h," ");
		}

		switch(createeventtype)
		{
		case CREATE_NOTEON:
			mainvar->AddString(h,"Note");
			break;

		case CREATE_POLYPRESSURE:
			mainvar->AddString(h,"PolyPressure");
			break;

		case CREATE_CONTROLCHANGE:
			mainvar->AddString(h,"ControlChange");
			break;

		case CREATE_PROGRAMCHANGE:
			mainvar->AddString(h,"ProgramChange");
			break;

		case CREATE_CHANNELPRESSURE:
			mainvar->AddString(h,"ChannelPressure");
			break;

		case CREATE_PITCHBEND:
			mainvar->AddString(h,"Pitchbend");
			break;

		case CREATE_SYSEX:
			mainvar->AddString(h,"SysEx");
			break;
		}

		createeventgadget->ChangeButtonText(h);
	}
}

void Edit_Event::SelectAndEdit(bool right)
{
	bool dontselect=false;

#ifdef OLDIE
	//editdata=NOEVENTEDIT;

	Edit_Event_Event *e=FindEvent(GetMouseX(),GetMouseY());	

	if((!e) && frame.status.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
	{
		patternselection.SelectAllEvents(false,SEL_ALLEVENTS);
		return;
	}

	if(maingui->GetCtrlKey()==false)
	{
		// Note Length
		if(frame.info.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
		{
			if(e)
			{
				switch(e->event->GetStatus())
				{
				case PROGRAMCHANGE:
					{
						ProgramChange *prog=(ProgramChange *)e->event;

						switch(generalMIDIdisplay)
						{
						case GM_ON:
							{
								DeletePopUpMenu(true);

								if(popmenu)
								{
									class menu_MIDIprog:public guiMenu
									{
									public:
										menu_MIDIprog(Edit_Event *e,ProgramChange *c,int co)
										{
											ed=e;
											prognr=co;
											cc=c;
										}

										void MenuFunction()
										{
											if(prognr!=cc->GetByte1())
											{
												EF_CreateEvent editevent;

												editevent.event=(Seq_Event *)cc->Clone();

												if(editevent.event)
												{
													editevent.event->SetByte1((char)prognr);

													editevent.song=ed->WindowSong();
													editevent.doundo=true;
													editevent.addtolastundo=false;

													mainedit->EditEventData(cc,&editevent);
												}
											}
										} //

										Edit_Event *ed;
										ProgramChange *cc;

										int prognr;
									};

									char help[255];

									int p=0;
									for(int a=0;a<16;a++)
									{
										if(prog->program>=p && prog->program<p+8)
										{
											strcpy(help,">>>");
											mainvar->AddString(help,gmproggroups[a]);
										}
										else
											strcpy(help,gmproggroups[a]);

										if(guiMenu *s=popmenu->AddMenu(help,0))
										{
											for(int i=0;i<8;i++)
											{
												s->AddFMenu(gmprognames[p],new menu_MIDIprog(this,prog,p),prog->program==p?true:false);
												p++;
											}
										}
									}

									ShowPopMenu();	

									dontselect=true;

								}// if popmenu
							}
							break;
						}// switch gmmode
					}
					break;

				case CONTROLCHANGE:
					{
						ControlChange *cc=(ControlChange *)e->event;

						DeletePopUpMenu(true);

						if(popmenu)
						{
							guiMenu *s;

							class menu_MIDIctrl:public guiMenu
							{
							public:
								menu_MIDIctrl(Edit_Event *e,ControlChange *c,int co)
								{
									ed=e;
									ctrlnumber=co;
									cc=c;
								}

								void MenuFunction()
								{
									if(ctrlnumber!=cc->GetByte1())
									{
										EF_CreateEvent editevent;

										editevent.event=(Seq_Event *)cc->Clone();

										if(editevent.event)
										{
											editevent.event->SetByte1((char)ctrlnumber);

											editevent.song=ed->WindowSong();
											editevent.doundo=true;
											editevent.addtolastundo=false;

											mainedit->EditEventData(cc,&editevent);
										}
									}
								} //

								Edit_Event *ed;
								ControlChange *cc;

								int ctrlnumber;
							};

							for(int a=0;a<4;a++)
							{
								int f,t;
								char *c;

								switch(a)
								{
								case 0:
									f=0;
									t=31;
									c=cc->GetByte1()>=f && cc->GetByte1()<=t?">>> MIDI Control 0-31":"MIDI Control 0-31";
									break;

								case 1:
									f=32;
									t=63;
									c=cc->GetByte1()>=f && cc->GetByte1()<=t?">>> MIDI Control 32-63":"MIDI Control 32-63";
									break;

								case 2:
									f=64;
									t=95;
									c=cc->GetByte1()>=f && cc->GetByte1()<=t?">>> MIDI Control 64-95":"MIDI Control 64-95";
									break;

								case 3:
									f=96;
									t=127;
									c=cc->GetByte1()>=f && cc->GetByte1()<=t?">>> MIDI Control 96-127":"MIDI Control 96-127";
									break;
								}

								if(s=popmenu->AddMenu(c,0))
								{
									for(int i=f;i<t;i++)
										s->AddFMenu(maingui->ByteToControlInfo(i,-1),new menu_MIDIctrl(this,cc,i),cc->GetByte1()==i?true:false);
								}
							}

							ShowPopMenu();	

							dontselect=true;

						}//if popmenu

					}
					break;
				}//switch
			}
		}
		else
		{
			if(e)
			{
				if(frame.curve.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
				{
					if(e->CanEditCurve()==true)
					{
						dontselect=true;
						editdata=EVENTEDIT_CURVE;
					}
				}
				else
					// program change info ?
					if(frame.byte2.CheckIfInFrame(GetMouseX(),GetMouseY())==true && e->event->GetStatus()==PROGRAMCHANGE)
					{
						ProgramChange *pro=(ProgramChange *)e->event;

						if(EditData *edit=new EditData)
						{
							// long position;

							edit->song=WindowSong();
							edit->win=this;
							edit->x=GetMouseX();
							edit->y=GetMouseY();

							edit->name=Cxs[CXS_EDIT_PCINFO];
							edit->deletename=false;

							edit->id=EDITID_PROGTEXT;

							edit->type=EditData::EDITDATA_TYPE_STRING;

							edit->helpobject=pro; // EditEvent
							edit->string=pro->info;

							maingui->EditDataValue(edit);
						}
					}
			}
		}
	}

	if(e && dontselect==false)
	{
		if(maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false && editdata==NOEVENTEDIT)
		{
			patternselection.SelectAllEvents(false,SEL_ALLEVENTS);
			e=FindEvent(GetMouseX(),GetMouseY()); // Refresh e
		}

		if(e)
		{
			selectmode=true;

			if(editdata==NOEVENTEDIT && (e->event->flag&OFLAG_SELECTED))
				selecttype=false;
			else
				selecttype=true;

			if(maingui->GetShiftKey()==true)
			{
				//if(selecttype==true)
				//		patternselection.SelectFromLastEventTo(e->event,true);
			}
			else
			{
				patternselection.lastselectedevent=e->event;

				if(selecttype==true || maingui->GetCtrlKey()==true)
				{
					/*
					cursorevent_index=patternselection.GetOfMixEvent(e->event);

					Seq_Event *event=e->event;

					if(maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false)
					patternselection.SelectAllEvents(false,SEL_ALLEVENTS,false); // no gui refresh

					patternselection.SelectEvent(event,true);

					if(CheckSelectPlayback(event)==true)
					{
					event->pattern->track->SendOutEvent_User(event->GetMIDIPattern(),event,true);
					}

					KeepEventinMid(patternselection.FindEvent(event),false); // no scroll
					*/
				}
			}
		}
	}

	if(editdata==EVENTEDIT_CURVE)
	{
		EditCurveEvent(e);
	}
	else
	{
		if(editdata!=NOEVENTEDIT)
		{
			if(left_mousekey==MOUSEKEY_DOWN || right_mousekey==MOUSEKEY_DOWN)
			{
				editstart_x=GetMouseX();
				editstart_y=GetMouseY();

				refreshmousebuttondown=true;
				refreshmousebuttonright=right;
			}
		}

		//if(selecttype==true && patternselection.GetCountofSelectedEvents()==1 &&editdata==NOEVENTEDIT)
		//	SendFirstSelectedEvent();

	}

#endif
}


bool Edit_Event::KeepEventinMid(Seq_SelectionEvent *e,bool scroll)
{
#ifdef OLDIE
	if(e)
	{
		int c=numberofdisplayevents;
		int ce=cursor_index=patternselection.GetOfMixEvent(e);

		cursorevent=e;

		if(ce>=0)
		{
			c/=2;
			int h=ce-c;

			if(h>=0 && scroll==true)
				firstevent_index=h;

			cursorevent=e;

			ShowEventsHoriz(SHOWEVENTS_HEADER|SHOWEVENTS_EVENTS);
			ShowSlider();
			RefreshTimeSlider();

			return true;
		}
#ifdef _DEBUG
		else
		{
			if(ce<0)
				MessageBox(NULL,"Illegal Keep in Mid","Error",MB_OK_STYLE);
		}
#endif
	}
#endif
	return false;
}

void Edit_Event::KeyDownRepeat()
{
	Editor_KeyDown();

	switch(nVirtKey)
	{
	case KEY_UP10:
	case KEY_CURSORUP:
		{
			/*
			if(maingui->GetCtrlKey()==true)
			{
			mainedit->MoveTracks(WindowSong(),-1);
			}
			else
			*/
			{
				// Active Track --

				/*
				if(GetCursor())
				{
				OObject *oo=eventobjects.FindObject(patternselection.GetCursor());

				if(OObject *po=eventobjects.FindPrevSameObject(oo))
				{
				patternselection.SetCursor(po->object);
				ScrollTo(GetCursor());
				}
				}
				*/
			}
		}
		break;

	case KEY_DOWN10:
	case KEY_CURSORDOWN:
		{
			/*
			if(GetCursor())
			{
			OObject *oo=eventobjects.FindObject(GetCursor());

			if(OObject *no=eventobjects.FindNextSameObject(oo))
			{
			eventobjects.SetCursor(no->object);
			ScrollTo(GetCursor());
			}
			}
			*/
		}
		break;
	}
}

void Edit_Event::KeyDown()
{
	Editor_KeyDown();

	Seq_Event *fe=patternselection.FirstSelectedEvent()?patternselection.FirstSelectedEvent()->seqevent:0;

	switch(nVirtKey)
	{
	case KEY_CURSORLEFT:
		if(cursorevent)
		{
			Edit_Event_Event *ed=FindEventInsideEditor(cursorevent->seqevent);

			if(ed)
			{
				if(cursor==0)
					cursor=CURSOR_LEN_00001;
				else
					cursor--;

				if(cursor==CURSOR_LEN_00001 && showquarterframe==false)
					cursor=CURSOR_LEN_0001;
				else
					if(cursor==CURSOR_POS_00001 && showquarterframe==false)
						cursor=CURSOR_POS_0001;

				ed->Draw(true);
			}
		}
		break;

	case KEY_CURSORRIGHT:
		if(cursorevent)
		{
			Edit_Event_Event *ed=FindEventInsideEditor(cursorevent->seqevent);

			if(ed)
			{
				if(cursor==CURSOR_LEN_00001)
					cursor=CURSOR_POS_1000;
				else
					cursor++;

				if(cursor==CURSOR_LEN_00001 && showquarterframe==false)
					cursor=CURSOR_POS_1000;
				else
					if(cursor==CURSOR_POS_00001 && showquarterframe==false)
						cursor=CURSOR_STATUS;

				ed->Draw(true);
			}
		}
		break;

		/*
		case KEY_CURSORUP:
		case KEY_CURSORDOWN:
		{
		if(cursorevent)
		{
		Seq_SelectionEvent *p=nVirtKey==KEY_CURSORUP?cursorevent->PrevEvent():cursorevent->NextEvent();

		if(p)
		{
		cursorevent_index=patternselection.GetOfMixEvent(p);

		Seq_Event *ev=p->event;

		if(maingui->GetShiftKey()==false)
		patternselection.SelectAllEvents(false,SEL_ALLEVENTS,false); // no gui refresh

		patternselection.SelectEvent(ev,true);

		// Scroll

		KeepEventinMid(patternselection.FindEvent(ev));
		}
		}
		}
		break;
		*/

	default:
		KeyDownRepeat();
		break;
	}

	/*
	Seq_SelectionEvent *ne=patternselection.FirstSelectedEvent();

	if(ne && ne->event!=fe)
	SendFirstSelectedEvent();
	*/
}

void Edit_Event::ShowCursorEvent()
{
	if(infotext)
	{
		if(cursorevent)
		{
			Seq_Event *sevent=cursorevent->seqevent;
			Seq_Pattern *p=sevent->GetPattern();
			Seq_Track *t=sevent->GetTrack();

			if(p && t)
			{
				if(char *h=mainvar->GenerateString(t->GetName()," : ",p->GetName()))
				{
					infotext->SetString(h);
					delete h;
				}
			}
			else
				infotext->SetString("Event Error");
		}
		else
			infotext->SetString("-");
	}
}

void Edit_Event::SendFirstSelectedEvent()
{
	Seq_SelectionEvent *ne=patternselection.FirstSelectedEvent();

	if(ne && ne->seqevent->GetStatus()==NOTEON)
	{
		Note *n=(Note *)ne->seqevent;

		// Play Note
		OSTART length=n->GetNoteLength();

		Note note;
		note.status=n->status;
		note.key=n->key;
		note.velocity=n->velocity;
		note.velocityoff=0;
		note.ostart=0;

		if(length>TICK4nd)
			length=TICK4nd;

		note.off.ostart=length;

		ne->seqevent->GetTrack()->SendOutEvent_User(ne->seqevent->GetMIDIPattern(),&note,true);
	}
}

void Edit_Event::SetMouseMode(int newmode)
{
#ifdef OLDIE
	modestartposition=GetMousePosition();

	if(modestartposition>=0)
	{
		SetEditorMode(newmode);

		modestartx=GetMouseX();
		modestarty=GetMouseY();

		Edit_Event_Event *e=FindEvent(GetMouseX(),GetMouseY());	

		//	modestartevent=e?e->event:0;

		SetAutoScroll(
			newmode,
			frame.gfx.x,
			frame.gfx.y,
			frame.gfx.x2,
			frame.gfx.y2
			);

		TRACE ("Mode Start Event %d Pos %d\n",modestartevent,modestartposition);
	}
#endif
}

void Edit_Event::StartOfNumberEdit(guiGadget *g)
{
	TRACE ("StartOfNumberEdit \n");

	Seq_Event *se=(Seq_Event *)editobject;

	if(se->IsSelected()==false)
	{
		if(maingui->GetCtrlKey()==false)
			patternselection.SelectAllEventsNot(se,false,0,false);

		se->Select();
	}
}

void Edit_Event::DeltaY(guiGadget_TabStartPosition *g)
{
	switch(mousemode)
	{
	case EM_EDITTIME:
		{
			EditPositions((int)editsum);
		}
		break;

	case EM_EDIT:
		{
			EditSelectedEventsDelta_Tab(events->edittabx);
		}break;
	}
}

void Edit_Event::MouseClickInEvents(bool leftmouse)
{
	if(IsPositionObjectClicked(events,leftmouse)==true) // Edit Time ?
	{
		events->SetEditSteps(1);
		SetEditorMode(EM_EDITTIME);
		return;
	}

	guiObject *o=events->CheckObjectClicked(); // Object Under Mouse ?

	if(o)
	{
		events->InitEdit(0,0); // X

		bool select=false;

		switch(events->edittabx)
		{
		case TAB_STATUS:
			{
				Edit_Event_Event *ee=(Edit_Event_Event *)o;
				SelectEvent(ee->seqevent);
			}
			break;

		case TAB_CHANNEL:
		case TAB_BYTE1:
		case TAB_BYTE2:
		case TAB_BYTE3:
		case TAB_DATA:
			{
				Edit_Event_Event *ee=(Edit_Event_Event *)o;

				if(ee->seqevent->IsSelected()==false)
				{
					SelectEvent(ee->seqevent);
					return;
				}
				else
					SelectEvent(ee->seqevent,true);

				if(events->edittabx!=TAB_STATUS)
					OpenEditSelectedEvents_Tab(events,ee->seqevent);
			}
			break;

		case TAB_LEN:
			{
				Edit_Event_Event *ee=(Edit_Event_Event *)o;
				Seq_Event *e=ee->seqevent;

				switch(e->GetStatus())
				{
				case NOTEON:
					{
						if(e->IsSelected()==false)
						{
							SelectEvent(e);
							return;
						}
						else
							SelectEvent(e,true);

						int mx=events->GetMouseX();

						for(int i=0;i<ee->lenindex;i++)
						{
							if(mx>=ee->lenx[i] && mx<=ee->lenx2[i])
							{
								edit_lenidenx=i;

								OpenEditSelectedEvents_Tab(events,e);
								break;
							}
						}
					}
					break;
				}
			}
			break;

		}
	}

	TRACE ("FUM %d\n",o);
}

void Edit_Event::MouseReleaseInEvents(bool leftmouse)
{
	if(IsEndOfPositionEditing(events)==true)
		return;

	switch(mousemode)
	{
	case EM_EDIT:
		{
			ReleaseEdit();
		}
		break;
	}

	events->EndEdit();
	ResetMouseMode();
}


EditData *Edit_Event::EditDataMessage(EditData *data)
{
	if(CheckStandardDataMessage(data)==true)
		return 0;

	if(data)
	{
		data=Editor_DataMessage(data);

		if(data)
			switch(data->id)
		{
			case EDITID_PROGTEXT:
				{
					ProgramChange *pro=(ProgramChange *)data->helpobject;

					if(pro->ChangeInfo(data->newstring)==true)
						maingui->RefreshAllEditorsWithEvent(WindowSong(),pro);
				}
				break;
		}
	}

	return 0;
}

void Edit_Event::SoloPattern() // v
{
	solostartposition=0;

	//if(editsolopattern==true)
	{
		Edit_Event_Event *firstevent=FirstEvent();

		if(firstevent)
		{
			solostartposition=firstevent->seqevent->GetEventStart();
		}

		/*
		//	if(cursorevent && ((Seq_Pattern *)cursorevent->event->pattern)!=patternselection.solopattern)
		{
		cursorevent_index=0;
		cursorevent=0;
		}
		*/
	}

	setsolostart=true;
}

void Edit_Event::AfterSolo()
{
	ShowSlider();
	setsolostart=false;
}

void Edit_Event::ShowAllEvents(int flag) //
{
	if((flag&NOBUILD_REFRESH)==0)
	{
		patternselection.BuildEventList(SEL_ALLEVENTS,&displayfilter); // Mix new List, events maybe moved/deleted

		OnInit();

		cursorevent=patternselection.GetMixEventAtIndex(cursorevent_index);

		if(!cursorevent)
		{
			cursorevent_index=0; // first event
			cursorevent=patternselection.FirstMixEvent();
		}

		if(setsolostart==true)
		{
			Seq_SelectionEvent *sel=patternselection.FirstMixEvent(solostartposition);
			firstevent_index=sel?sel->GetIndex():0;
		}
	}

	ShowEvents();

	if((flag&NOBUILD_REFRESH)==0)
		ShowCursorEvent();
}

#ifdef OLDIE
void Edit_Event::MouseMove(bool inside)
{
	bool used=EditorMouseMove(inside);

	if(used==false && editdata!=NOEVENTEDIT) // Edit MIDI Event Data
	{
		int diffx=GetMouseX()-editstart_x,diffy=GetMouseY()-editstart_y,datadiff;

		datadiff=diffy;

		if(editdata==EVENTEDIT_CURVE)
		{
			//Edit_Event_Event *e=FindEvent(GetMouseX(),GetMouseY());

			if(curveevent)
				EditCurveEvent(curveevent);
		}
	}
	else
	{
		if(inside==true)
		{
			if(selectmode==true)
			{
				Edit_Event_Event *e=FindEvent(GetMouseX(),GetMouseY());

				//if(e)
				//	patternselection.SelectFromLastEventTo(e->event,selecttype);
			}
			else
			{
				switch(mousemode)
				{
				case EM_SELECTOBJECTS:
					{
						/*
						if(patternselection.FirstMixEvent() && frame.gfx.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
						{
						OSTART selectionstart=modestartposition,selectionend;
						Seq_SelectionEvent *startevent=modestartevent;
						guiTimeLinePos *pos=timeline->FindPositionX(GetMouseX());

						if(pos && pos->NextPosition())
						selectionend=pos->NextPosition()->GetStart();
						else
						selectionend=GetMouseX()>timeline->x2?endposition:startposition;

						if(selectionstart>selectionend)
						{
						OSTART h=selectionend;
						selectionend=selectionstart;
						selectionstart=h;
						}

						Edit_Event_Event *fe=FindEvent(GetMouseX(),GetMouseY());

						if(fe && (!startevent))
						startevent=patternselection.LastMixEvent();

						Seq_SelectionEvent *endevent=fe?fe->event:patternselection.LastMixEvent(),*eventundermouse=endevent;

						if(!endevent)
						endevent=patternselection.LastMixEvent();

						if(startevent && endevent && patternselection.GetOfMixEvent(startevent)>patternselection.GetOfMixEvent(endevent))
						{
						Seq_SelectionEvent *h=startevent;
						startevent=endevent;
						endevent=h;
						}

						RemoveAllSprites(guiSprite::SPRITEDISPLAY_SELECTION);

						guiSprite *s=new guiSprite(guiSprite::SPRITETYPE_RECT,guiSprite::SPRITEDISPLAY_SELECTION);

						if(s)
						{
						s->colour=COLOUR_BLUE;

						// X
						if(modestartposition<startposition){
						s->x=frame.gfx.x;
						s->subtype|=SPRITETYPE_RECT_NOLEFT;
						}
						else
						if(modestartposition>endposition){
						s->x=frame.gfx.x2;
						s->subtype|=SPRITETYPE_RECT_NORIGHT;
						}
						else
						s->x=timeline->ConvertTimeToX(modestartposition); // Set Startposition

						if(GetMouseX()>frame.gfx.x2)
						s->x2=frame.gfx.x2;
						else
						s->x2=GetMouseX()<frame.gfx.x?frame.gfx.x:frame.gfx.x2;

						if(modestartevent && (!FindEventInsideEditor(modestartevent->event)))
						{
						if(patternselection.GetOfMixEvent(modestartevent)<patternselection.GetOfMixEvent(eventundermouse))
						{
						s->y=frame.gfx.y;
						s->subtype|=SPRITETYPE_RECT_NOTOP;
						}
						else
						{
						s->y=frame.gfx.y2;
						s->subtype|=SPRITETYPE_RECT_NOBOTTOM;
						}
						}
						else
						s->y=modestarty;

						maingui->OpenPRepairSelection(&patternselection);

						Seq_SelectionEvent *sel=startevent;

						while(sel)
						{
						maingui->PRepairSelection(sel->event);

						if(sel==endevent)
						break;

						sel=sel->NextEvent();
						}

						maingui->ClosePRepairSelection(&patternselection);

						s->x=frame.gfx.SetToX(s->x);
						s->y=frame.gfx.SetToY(s->y);
						s->x2=frame.gfx.SetToX(GetMouseX());
						s->y2=frame.gfx.SetToY(GetMouseY());

						AddSpriteShow(s);
						}
						}
						*/
					}
					break;

				}
			}
		}
	}
}
#endif

char *Edit_Event::GetToolTipString1()
{
	/*
	if(Edit_Event_Event *e=FindEvent(GetMouseX(),GetMouseY()))
	{
	if(e->gfx_x!=-1 && e->gfx_x<=GetMouseX() && e->gfx_x2>=GetMouseX())
	return e->GetTool1String();
	}
	*/

	return 0;
}

void Edit_Event::EditEditorPositions(guiGadget *g)
{
	OSTART maxminus=1;

	Edit_Event_Event *ee=FirstEvent();

	while(ee)
	{
		if(ee->seqevent->IsSelected()==true)
		{
			OSTART pos=ee->seqevent->GetEventStart();
			pos+=numbereditposition_sum;

			if(pos<0)
			{
				if(maxminus==1 || -ee->seqevent->GetEventStart()>maxminus)
					maxminus=-ee->seqevent->GetEventStart();
			}
		}

		if(maxminus!=1)
			numbereditposition_sum=maxminus;

		ee=ee->NextEvent();
	}

	editsum=0;

	RefreshObjects(0,true);
	ResetEditSum();
}

void Edit_Event::EndOfPositionEdit(guiGadget *,OSTART diff)
{
	MoveO mo;

	mo.song=WindowSong();
	mo.sellist=&patternselection;
	mo.diff=diff;

	mainedit->MoveSelectedEventsInPatternList(&mo);
}

Edit_Event::Edit_Event()
{
	editorname="Event";
	editorid=EDITORTYPE_EVENT;

	InitForms(FORM_HORZ2x1BAR_EVENT_SLIDERHV);

	resizeable=true;

	minwidth=minheight=maingui->GetButtonSizeY(30);
	minheight=maingui->GetButtonSizeY(12);

	modestartevent=0;
	firstevent_index=cursorevent_index=0;

	zoomy=20;
	editdata=NOEVENTEDIT;
	filtergadget=0;

	moveevents=false;
	lengthchanged=false;
	cursor=CURSOR_STATUS;
	cursorevent=0;
	createeventtype=CREATE_NOTEON;
	createselection=0;
	displayusepos=0;
	solostartposition=0;
	setsolostart=false;
	createeventgadget=0;
	events=0;
	eventsgfx=0;

	SetMinZoomY(maingui->GetButtonSizeY());

	createchannel=1;
	showgfx=mainsettings->event_showgfx;
	showonlyselectedevents=patternselection.showonlyselectedevents;
	followsongposition=mainsettings->followeditor;
	curveevent=0;
	dontshowtrackslider=false;

	ResetGadgets();
}

void Edit_Event::InitTabs(guiGadget_TabStartPosition *list)
{
	list->InitTabs(TAB_TABS);

	list->InitTabWidth(TAB_TIME,list->GetDefaultTimeWidth()); // Position
	list->InitTabWidth(TAB_STATUS,"  STATUS  ");
	list->InitTabWidth(TAB_CHANNEL," CHL ");
	list->InitTabWidth(TAB_BYTE1," Byte # ");
	list->InitTabWidth(TAB_BYTE2," Byte # ");
	list->InitTabWidth(TAB_BYTE3," Byte # ");
	list->InitTabWidth(TAB_DATA," #Data# ");
	list->InitTabWidth(TAB_LEN,list->GetDefaultTimeWidth());

	list->InitStartPosition(TAB_TIME);
	list->InitXX2(); // X<>X2
}

void Edit_Event::Goto(int to)
{
	UserEdit();

	Seq_SelectionEvent *el=0;

	if(CheckStandardGoto(to)==true)
		return;

	switch(to)
	{
	case GOTO_FOCUS:
		//		el=patternselection.FindEvent(infonote);
		break;

	case GOTO_FIRST:
		{
			el=patternselection.FirstMixEvent();

			while(el && el->seqevent->GetStatus()!=NOTEON) // Note
				el=el->NextEvent();
		}
		break;

	case GOTO_LAST:
		{
			el=patternselection.LastMixEvent();

			while(el && el->seqevent->GetStatus()!=NOTEON) // Note
				el=el->PrevEvent();
		}
		break;

	case GOTO_FIRSTSELECTED:
		{
			el=patternselection.FirstMixEvent();

			while(el && 
				(el->seqevent->GetStatus()!=NOTEON || ((el->seqevent->flag&OFLAG_SELECTED)==0))) // Note
				el=el->NextEvent();
		}
		break;

	case GOTO_LASTSELECTED:
		{
			el=patternselection.LastMixEvent();

			while(el && 
				(el->seqevent->GetStatus()!=NOTEON || ((el->seqevent->flag&OFLAG_SELECTED)==0))) // Note
				el=el->PrevEvent();
		}
		break;

	case GOTO_FIRSTEVENTTRACK:
		if(WindowSong()->GetFocusTrack())
		{
			el=patternselection.FirstMixEvent();
			Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

			while(el && 
				(el->seqevent->GetPattern()->GetTrack()!=WindowSong()->GetFocusTrack() ||
				el->seqevent->GetStatus()!=NOTEON
				)
				)
				el=el->NextEvent();
		}
		break;

	case GOTO_LASTEVENTTRACK:
		if(WindowSong()->GetFocusTrack())
		{
			el=patternselection.LastMixEvent();
			Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

			while(el && 
				(el->seqevent->GetPattern()->GetTrack()!=WindowSong()->GetFocusTrack() ||
				el->seqevent->GetStatus()!=NOTEON
				)
				)
				el=el->PrevEvent();
		}
		break;
	}

	if(el)
	{
		if(NewStartPosition(el->seqevent->GetEventStart(),true)==true)
			SyncWithOtherEditors();
	}
}

Seq_SelectionEvent *Edit_Event::Goto_Special(int to)
{
	Seq_SelectionEvent *f=patternselection.FirstMixEvent(),*l=patternselection.LastMixEvent();

	switch(to)
	{
	case GOTO_FIRST_NOTE:
		{
			while(f){
				if(f->seqevent->GetStatus()==NOTEON)return f;
				f=f->NextEvent();
			}
		}
		break;

	case GOTO_FIRST_PROGRAM:
		{
			while(f){
				if(f->seqevent->GetStatus()==PROGRAMCHANGE)return f;
				f=f->NextEvent();
			}
		}
		break;

	case GOTO_FIRST_PITCH:
		{
			while(f)
			{
				if(f->seqevent->GetStatus()==PITCHBEND)
					return f;

				f=f->NextEvent();
			}
		}
		break;

	case GOTO_FIRST_CONTROL:
		{
			while(f)
			{
				if(f->seqevent->GetStatus()==CONTROLCHANGE)
					return f;
				f=f->NextEvent();
			}
		}
		break;

	case GOTO_FIRST_SYSEX:
		{
			while(f)
			{
				if(f->seqevent->GetStatus()==SYSEX)
					return f;
				f=f->NextEvent();
			}
		}
		break;
	case GOTO_FIRST_CHANNELPRESSURE:
		{
			while(f)
			{
				if(f->seqevent->GetStatus()==CHANNELPRESSURE)
					return f;
				f=f->NextEvent();
			}
		}
		break;

	case GOTO_FIRST_POLYPRESSURE:
		{
			while(f)
			{
				if(f->seqevent->GetStatus()==POLYPRESSURE)
					return f;
				f=f->NextEvent();
			}
		}
		break;

	case GOTO_LAST_NOTE:
		{
			while(l)
			{
				if(l->seqevent->GetStatus()==NOTEON)
					return l;
				l=l->PrevEvent();
			}
		}
		break;

	case GOTO_LAST_PROGRAM:
		{
			while(l)
			{
				if(l->seqevent->GetStatus()==PROGRAMCHANGE)
					return l;
				l=l->PrevEvent();
			}
		}
		break;

	case GOTO_LAST_PITCH:
		{
			while(l)
			{
				if(l->seqevent->GetStatus()==PITCHBEND)
					return l;
				l=l->PrevEvent();
			}
		}
		break;
	case GOTO_LAST_CONTROL:
		{
			while(l)
			{
				if(l->seqevent->GetStatus()==CONTROLCHANGE)
					return l;
				l=l->PrevEvent();
			}
		}
		break;
	case GOTO_LAST_SYSEX:
		{
			while(l)
			{
				if(l->seqevent->GetStatus()==SYSEX)
					return l;
				l=l->PrevEvent();
			}
		}
		break;

	case GOTO_LAST_CHANNELPRESSURE:
		{
			while(l)
			{
				if(l->seqevent->GetStatus()==CHANNELPRESSURE)
					return l;
				l=l->PrevEvent();
			}
		}
		break;

	case GOTO_LAST_POLYPRESSURE:
		{
			while(l)
			{
				if(l->seqevent->GetStatus()==POLYPRESSURE)
					return l;
				l=l->PrevEvent();
			}
		}
		break;
	}

	return false;
}

void Edit_Event::GotoEnd()
{
	ShowSlider();
}

bool Edit_Event::FindAudioEventOnDisplay()
{
	Edit_Event_Event *f=FirstEvent();

	while(f){
		if(f->seqevent->IsAudio()==true)return true;
		f=f->NextEvent();
	}

	return false;
}

void Edit_Event::ScrollTo(Seq_Event *se)
{
	if(OObject *f=eventobjects.FindObject(se))
	{
		eventobjects.ScrollY(f);
		DrawDBBlit(events,eventsgfx);
	}
}

bool Edit_Event::EditCancel()
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

void Edit_Event::AddSpecialGoto(guiMenu *menu)
{
	menu->AddLine();

	menu->AddFMenu(Cxs[CXS_FIRSTNOTEEVENT],new menu_gotoeventeditor(this,GOTO_FIRST_NOTE));
	menu->AddFMenu(Cxs[CXS_FIRSTPROGRAMEVENT],new menu_gotoeventeditor(this,GOTO_FIRST_PROGRAM));
	menu->AddFMenu(Cxs[CXS_FIRSTPITCHEVENT],new menu_gotoeventeditor(this,GOTO_FIRST_PITCH));
	menu->AddFMenu(Cxs[CXS_FIRSTCTRLEVENT],new menu_gotoeventeditor(this,GOTO_FIRST_CONTROL));
	menu->AddFMenu(Cxs[CXS_FIRSTSYSEVENT],new menu_gotoeventeditor(this,GOTO_FIRST_SYSEX));
	menu->AddFMenu(Cxs[CXS_FIRSTCPREVENT],new menu_gotoeventeditor(this,GOTO_FIRST_CHANNELPRESSURE));
	menu->AddFMenu(Cxs[CXS_FIRSTPPREVENT],new menu_gotoeventeditor(this,GOTO_FIRST_POLYPRESSURE));

	menu->AddLine();
	menu->AddFMenu(Cxs[CXS_LASTNOTEEVENT],new menu_gotoeventeditor(this,GOTO_LAST_NOTE));
	menu->AddFMenu(Cxs[CXS_LASTPROGRAMEVENT],new menu_gotoeventeditor(this,GOTO_LAST_PROGRAM));
	menu->AddFMenu(Cxs[CXS_LASTPITCHEVENT],new menu_gotoeventeditor(this,GOTO_LAST_PITCH));
	menu->AddFMenu(Cxs[CXS_LASTCTRLEVENT],new menu_gotoeventeditor(this,GOTO_LAST_CONTROL));
	menu->AddFMenu(Cxs[CXS_LASTSYSEVENT],new menu_gotoeventeditor(this,GOTO_LAST_SYSEX));
	menu->AddFMenu(Cxs[CXS_LASTCPREVENT],new menu_gotoeventeditor(this,GOTO_LAST_CHANNELPRESSURE));
	menu->AddFMenu(Cxs[CXS_LASTPPREVENT],new menu_gotoeventeditor(this,GOTO_LAST_POLYPRESSURE));
	menu->AddLine();

	menu->AddFMenu(Cxs[CXS_FIRSTMUTEEVENT],new menu_gotoeventeditor(this,GOTO_FIRST_MUTE));
	menu->AddFMenu(Cxs[CXS_LASTMUTEEVENT],new menu_gotoeventeditor(this,GOTO_LAST_MUTE));
}

bool Edit_Event::ZoomGFX(int z,bool horiz)
{
	if(SetZoomGFX(z,horiz)==true)
	{
		if(horiz==false)
		{
			OnInit();
		}

		DrawDBBlit(events,showgfx==true?eventsgfx:0);
		return true;
	}

	return false;
}


void Edit_Event::ShowOptionsMenu()
{
	if(onlyselected)
		onlyselected->menu->Select(onlyselected->index,patternselection.showonlyselectedevents);

	if(gmmenu)
	{
		switch(generalMIDIdisplay)
		{
		case GM_OFF:
			gmmenu->menu->Select(gmmenu->index,false);
			break;

		case GM_ON:
			gmmenu->menu->Select(gmmenu->index,true);
			break;
		}
	}

	if(gfxonoffmenu)
		gfxonoffmenu->menu->Select(gfxonoffmenu->index,showgfx);
}

// Buttons,Slider ...

void Edit_Event::NewZoom()
{
	RefreshStartPosition();
	ShowEventsGFX();
}

void Edit_Event::Gadget(guiGadget *gadget)
{	
	if(!Editor_Gadget(gadget))return;

	if(gadget)
	{
		switch(gadget->gadgetID)
		{
		case GADGETID_FILTERRESET:
			displayfilter.Reset();
			break;

		case GADGETID_FILTERNOTES:
			displayfilter.Toggle(MIDIOUTFILTER_NOTEON);
			break;

		case GADGETID_FILTERCONTROL:
			displayfilter.Toggle(MIDIOUTFILTER_CONTROLCHANGE);
			break;

		case GADGETID_FILTERPITCH:
			displayfilter.Toggle(MIDIOUTFILTER_PITCHBEND);
			break;

		case GADGETID_FILTERCPRESS:
			displayfilter.Toggle(MIDIOUTFILTER_CHANNELPRESSURE);
			break;

		case GADGETID_FILTERPPRESS:
			displayfilter.Toggle(MIDIOUTFILTER_POLYPRESSURE);
			break;

		case GADGETID_FILTERPROGRAM:
			displayfilter.Toggle(MIDIOUTFILTER_PROGRAMCHANGE);
			break;


		case GADGETID_FILTERSYSEX:
			displayfilter.Toggle(MIDIOUTFILTER_SYSEX);
			break;

		case GADGETID_TOOLBOX_QUANTIZE:
			mainedit->QuantizeEventsMenu(this,&patternselection);
			break;

		case GADGETID_DATA:
			{
				showgfx=showgfx==true?false:true;
				//FormXEnable(2,showgfx);
				FormEnable(1,1,showgfx);
			}
			break;

		case EVENT_CREATE_ID:
			{
				if(DeletePopUpMenu(true))
				{
					class menu_selcevent:public guiMenu
					{
					public:
						menu_selcevent(Edit_Event *ed,int t)
						{
							editor=ed;
							type=t;
						}

						void MenuFunction()
						{
							editor->createeventtype=type;
							editor->ShowCreateButton();	
						} //

						Edit_Event *editor;
						int type;
					};

					popmenu->AddMenu(Cxs[CXS_SELECTNEWEVENTTYPE],0);
					popmenu->AddLine();
					popmenu->AddFMenu("Note",new menu_selcevent(this,CREATE_NOTEON),createeventtype==CREATE_NOTEON?true:false);
					popmenu->AddFMenu("PolyPressure",new menu_selcevent(this,CREATE_POLYPRESSURE),createeventtype==CREATE_POLYPRESSURE?true:false);
					popmenu->AddFMenu("ControlChange",new menu_selcevent(this,CREATE_CONTROLCHANGE),createeventtype==CREATE_CONTROLCHANGE?true:false);
					popmenu->AddFMenu("ProgramChange",new menu_selcevent(this,CREATE_PROGRAMCHANGE),createeventtype==CREATE_PROGRAMCHANGE?true:false);
					popmenu->AddFMenu("ChannelPressure",new menu_selcevent(this,CREATE_CHANNELPRESSURE),createeventtype==CREATE_CHANNELPRESSURE?true:false);
					popmenu->AddFMenu("Pitchbend",new menu_selcevent(this,CREATE_PITCHBEND),createeventtype==CREATE_PITCHBEND?true:false);
					popmenu->AddFMenu("SysEx",new menu_selcevent(this,CREATE_SYSEX),createeventtype==CREATE_SYSEX?true:false);

					// Channel
					{
						popmenu->AddLine();

						char h2[NUMBERSTRINGLEN],
							*help=mainvar->GenerateString(Cxs[CXS_MIDICHANNEL],":",mainvar->ConvertIntToChar(createchannel,h2));

						if(help)
						{
							if(guiMenu *s=popmenu->AddMenu(help,0))
							{
								class menu_MIDIchl:public guiMenu
								{
								public:
									menu_MIDIchl(Edit_Event *e,int c)
									{
										editor=e;
										channel=c;
									}

									void MenuFunction()
									{
										if(channel!=editor->createchannel)
										{
											editor->createchannel=channel;
											editor->ShowCreateButton();
										}
									} //

									Edit_Event *editor;
									int channel;
								};

								for(int m=1;m<17;m++)
									s->AddFMenu(MIDIchannels[m],new menu_MIDIchl(this,m),createchannel==m?true:false);
							}

							delete help;
						}
					}

					ShowPopMenu();
				}
			}
			break;

		case GADGETID_FILTERID:
			{
				guiWindow *win=maingui->FindWindow(EDITORTYPE_MIDIFILTER,&displayfilter,0);

				if(!win)
				{	
					mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].x=GetWinPosX()+gadget->x2;
					mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].y=GetWinPosY()+gadget->y;

					guiWindow *w=maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,WindowSong(),0,0,0,&displayfilter,0);
					if(w)
					{
						w->calledfrom=this;
						w->guiSetWindowText(Cxs[CXS_EVENTDISPLAYFILTER]);
					}
				}
				else
					win->WindowToFront(true);	
			}
			break;

		case GADGETID_EDITORSLIDER_VERT: // Track Scroll
			{
				TRACE ("VSlider Pos %d H %d GH %d\n",vertgadget->pos,eventobjects.height,events->GetHeight());

				eventobjects.InitWithSlider(vertgadget,true);
				eventobjects.InitYStartO();

				Seq_Event *so=(Seq_Event *)eventobjects.showobject->object;

				if(startposition!=so->GetEventStart())
				{
					SetStartPosition(so->GetEventStart());
					DrawHeader();
				}

				DrawDBBlit(events,showgfx==true?eventsgfx:0);
				RefreshTimeSlider();

				if(syncsongposition==true)

					SyncWithOtherEditors();

				UserEdit();
			}
			break;

			/*
			case GADGETID_EDITORSLIDER_VERTZOOM:
			ZoomGFX(gadget->GetPos());
			break;
			*/
		}
	}
}

void Edit_Event::MouseWheel(int delta,guiGadget *db)
{
	if(Edit_MouseWheel(delta)==false)
	{
		if((!db) || db==events || db==eventsgfx)
		{
			if(vertgadget)
				vertgadget->DeltaY(delta);
		}
	}
}

Edit_Event_Event *Edit_Event::FirstEvent()
{
	return (Edit_Event_Event *)guiobjects.FirstObject();
}

Edit_Event_Event *Edit_Event::LastEvent()
{
	return (Edit_Event_Event *)guiobjects.LastObject();
}

Edit_Event_Event *Edit_Event::FindEvent(int x,int y)
{
	Edit_Event_Event *e=FirstEvent();

	while(e){
		if(e->y<=y && e->y2>y)return e;
		e=e->NextEvent();
	}

	return 0;
}

OObject *Edit_Event::BuildEventList()
{
	eventobjects.DeleteAllO(events);

	OObject *startoo=0;
	Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

	while(selevent)
	{
		OObject *oo=eventobjects.AddCooObjectRStart(selevent->seqevent,selevent->seqevent->GetEventStart(),zoomy,0);

		if((!startoo) && selevent->seqevent->GetEventStart()>=startposition)
		{
			startoo=oo;
		}

		selevent=selevent->NextEvent();
	}

	eventobjects.EndBuild();

	return startoo;
}

void Edit_Event::ShowHeader()
{
	if(!eventsheader)return;

	guiBitmap *bitmap=&eventsheader->gbitmap;

	bitmap->guiFillRect(COLOUR_BACKGROUND);

	bitmap->SetTextColour(COLOUR_BLACK);
	bitmap->SetFont(&maingui->standard_bold);

	for(int i=0;i<TAB_TABS;i++)
	{
		int x=eventsheader->GetTabX(i);
		int x2=eventsheader->GetTabX2(i);

		char *h=headerstrings[i];

		switch(i)
		{
		case TAB_TIME:
		case TAB_LEN:
			{
				if(i==TAB_LEN)
					h=Cxs[CXS_LENGTH];
			}
			break;
		}

		bitmap->guiDrawText(x+2,bitmap->GetY2()-maingui->GetFontSizeY(),x2,h);

		if(i<TAB_INFO)
		{
			bitmap->guiDrawLineX(x2,COLOUR_BLACK_LIGHT);
		}
	}

	bitmap->guiDrawLineYX0(bitmap->GetY2(),bitmap->GetX2(),COLOUR_BLACK_LIGHT);
}

void Edit_Event::ShowSlider()
{
	if(vertgadget)
		vertgadget->ChangeSlider(&eventobjects,zoomy);
}

void Edit_Event::ShowEventsGFX()
{
	if(!eventsgfx)
		return;

	eventsgfx->gbitmap.guiFillRect(COLOUR_UNUSED);

	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		Edit_Event_Event *ee=(Edit_Event_Event *)o;
		ee->DrawGFX();
		o=o->NextObject();
	}
}

void Edit_Event::OnInit()
{
	if(zoomvert==true)
		eventobjects.BufferYPos();

	OObject *startobject=BuildEventList();

	if(zoomvert==true)
		eventobjects.RecalcYPos();
	else
		eventobjects.ScrollY(startobject);
}

void Edit_Event::InitShowEvents()
{
	eventobjects.SetGUIHeight(events);
	eventobjects.SetSlider(vertgadget);

	if(eventobjects.startsetbyslider==true)
	{	
		// V Scroll
		eventobjects.InitYStartO();

		/*
		if(eventobjects.GetShowObject())
		{
		Seq_Event *e=(Seq_Event *)eventobjects.GetShowObject()->object;

		if(e && e->GetEventStart()!=startposition)
		{
		TRACE ("OSTART %d",startposition);
		TRACE ("NSTART %d \n",e->GetEventStart());

		if(NewStartPosition(e->GetEventStart(),true)==true)
		{
		SyncWithOtherEditors();
		UserEdit();
		}	
		}
		}
		*/
	}
}

void Edit_Event::ShowEventFilter()
{
	if(notes)
		notes->Toggle(displayfilter.CheckFilter(MIDIOUTFILTER_NOTEON));

	if(control)
		control->Toggle(displayfilter.CheckFilter(MIDIOUTFILTER_CONTROLCHANGE));

	if(cpress)
		cpress->Toggle(displayfilter.CheckFilter(MIDIOUTFILTER_CHANNELPRESSURE));

	if(ppress)
		ppress->Toggle(displayfilter.CheckFilter(MIDIOUTFILTER_POLYPRESSURE));

	if(prog)
		prog->Toggle(displayfilter.CheckFilter(MIDIOUTFILTER_PROGRAMCHANGE));

	if(pitch)
		pitch->Toggle(displayfilter.CheckFilter(MIDIOUTFILTER_PITCHBEND));

	if(sysex)
		sysex->Toggle(displayfilter.CheckFilter(MIDIOUTFILTER_SYSEX));
}

void Edit_Event::ShowEvents()
{
	if(zoomvert==true)
		InitShowEvents();

	eventobjects.EndBuild();
	eventobjects.InitYStartO();

	patternselection.ClearFlags();
	patternselection.CopyStatus();

	guiobjects.RemoveOs(0);

	if(!events)
		return;

	guiBitmap *bitmap=&events->gbitmap;

	bitmap->guiFillRect(COLOUR_BACKGROUND);

	/*
	for(int i=0;i<6;i++)
	{
	bitmap->guiDrawLineX(drawposx2[i],COLOUR_BLACK_LIGHT);
	}
	*/

	ShowSlider();

	events->ClearTab();

	glist.SelectForm(events);

	if(eventobjects.GetShowObject())
	{
		int index;
		Seq_Event *firstevent=(Seq_Event *)eventobjects.GetShowObject()->object;

		index=patternselection.GetIndexOfEvent(firstevent);

		// Create Track List
		while(eventobjects.GetShowObject() && eventobjects.GetInitY()<events->GetHeight())
		{
			if(Edit_Event_Event *et=new Edit_Event_Event)
			{
				et->editor=this;
				et->song=WindowSong();
				et->bitmap=&events->gbitmap;
				et->timemode=ConvertWindowDisplayToTimeMode();
				et->seqevent=(Seq_Event *)eventobjects.GetShowObject()->object;
				et->index=index++;

				guiobjects.AddTABGUIObject(0,eventobjects.GetInitY(),events->GetWidth(),eventobjects.GetInitY2()-1,events,et);
			}

			eventobjects.NextYO();

		}// while list

		guiObject_Pref *o=events->FirstGUIObjectPref();
		while(o)
		{
			Edit_Event_Event *et=(Edit_Event_Event *)o->gobject;

			et->Draw();
			AddNumberOList(et,events); // Init Time

			o=o->NextGUIObjectPref();
		}
	}

	eventobjects.DrawUnUsed(events);
}

void Edit_Event::RefreshRealtime()
{
	if(RefreshEventEditorRealtime()==true)
	{
		if(eventsgfx && showgfx==true)
		{
			ShowCycleAndPositions(eventsgfx);
			eventsgfx->DrawSpriteBlt();
		}
	}

	OSTART spp=WindowSong()->GetSongPosition();

	Edit_Event_Event *ee=FirstEvent();

	while(ee)
	{
		if(WindowSong()->mastering==false && WindowSong()->IsPlayback()==true && ee->seqevent->GetStatus()==NOTEON)
		{
			Note *note=(Note *)ee->seqevent;

			if(note->GetEventStart()<=spp && note->GetNoteEnd()>spp)
			{
				ee->RefreshRealtime(spp,true);
			}
			else
				if(ee->realtimeactive==true)
				{
					ee->RefreshRealtime(spp,false);
				}
		}

		if(ee->activeevent==true && ee->seqevent!=WindowSong()->GetFocusEvent())
			goto refresh;

		if(ee->seqevent==WindowSong()->GetFocusEvent() && ee->activeevent==false)
			goto refresh;

		if(ee->eflag!=ee->seqevent->flag)
			goto refresh;

		ee=(Edit_Event_Event *)ee->NextObject();
	}

	// Filter ?
	if(displayfilter.Compare(&checkfilter)==false)
	{
		patternselection.BuildEventList(SEL_ALLEVENTS,&displayfilter); // Mix new List, events maybe moved/deleted
		ShowFilter();

		BuildEventList();
		RefreshObjects(0,false);
	}

	return;

refresh:
	//ShowEvents();
	DrawDBBlit(events,showgfx==true?eventsgfx:0);
	return;

	// Clock
	//	if(frame.gfx.ondisplay==true)
	//		ShowClockLine(WindowSong()->GetSongPosition(),frame.gfx.y,frame.gfx.y2);

	bool shownew=false;

	if(displayfilter.Compare(&cmpfilter)==false || patternselection.showonlyselectedevents!=showonlyselectedevents) // new filter
	{
		showonlyselectedevents=patternselection.showonlyselectedevents;

		Seq_Event *bcursorevent=0;

		if(cursorevent)
			bcursorevent=cursorevent->seqevent;

		patternselection.BuildEventList(SEL_ALLEVENTS,&displayfilter); // Mix new List

		// Find Cursor Event Buffer again
		cursorevent=bcursorevent?patternselection.FindEvent(bcursorevent):0; // or NULL

		if(!cursorevent)
			cursorevent_index=0;

		shownew=true;
	}

	// New Events added - Recording ?
	Seq_SelectedPattern *s=patternselection.FirstSelectedPattern();
	while(s && shownew==false)
	{
		if(s->status_nrpatternevents!=s->pattern->GetCountOfEvents())
			shownew=true;

		s=s->NextSelectedPattern();
	}

	if(shownew==true)
	{
		ShowEvents();
		ShowCursorEvent();
	}
	else
	{
		Edit_Event_Event *fe=FirstEvent();
		while(fe)
		{
			if(fe->eflag!=fe->seqevent->flag)
				fe->Draw(true);

			fe=fe->NextEvent();
		}
	}

	if(WindowSong()->status&Seq_Song::STATUS_SONGPLAYBACK_MIDI)
		ShowEventsRealtime();
	else
		ClearEventsRealtime();

	RefreshToolTip();
}

void Edit_Event::ShowFilter()
{
	ShowEventFilter();

	if(filtergadget)
	{
		if(displayfilter.CheckFilterActive()==true)
		{
			char *h=mainvar->GenerateString("Event Filter",">",Cxs[CXS_ON],"<");
			if(h)
			{
				filtergadget->ChangeButtonText(h);
				delete h;
			}
		}
		else
			filtergadget->ChangeButtonText("- Event Filter -");
	}

	displayfilter.Clone(&checkfilter);
}

void Edit_Event::RedrawDeletedPattern()
{
	patternselection.BuildEventList(SEL_ALLEVENTS,&displayfilter); // Mix new List
	RefreshObjects(0,false);
}

void Edit_Event::RefreshObjects(LONGLONG type,bool editcall)
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

	if(refreshflag&REFRESHEVENT_LIST)
	{
		patternselection.BuildEventList(SEL_ALLEVENTS,&displayfilter); // Mix new List, events maybe moved/deleted
		BuildEventList();
	}

	DrawHeader(); // Tempo Changes etc..
	DrawDBBlit(events,eventsgfx);

	patternselection.patternremoved=false;
}

void Edit_Event::SetStartPosition(OSTART pos)
{
	startposition=pos;

	if(Seq_SelectionEvent *e=patternselection.FirstMixEvent(pos))
		firstevent_index=e->GetIndex();
	else
		firstevent_index=-1;
}

Edit_Event_Event *Edit_Event::FindEventInsideEditor(Seq_Event *e)
{
	Edit_Event_Event *ce=FirstEvent();

	while(ce){
		if(ce->seqevent==e)return ce;
		ce=ce->NextEvent();
	}

	return 0;
}

void Edit_Event::ShowMenu()
{
	ShowEditorMenu();
}

void Edit_Event::ClearEventsRealtime()
{
	Edit_Event_Event *e=FirstEvent();

	while(e){

		if(e->realtimeactive==true){
			e->realtimeactive=false;
			e->Draw(true);
		}

		e=e->NextEvent();
	}
}

void Edit_Event::ShowEventsRealtime()
{
	Edit_Event_Event *e=FirstEvent();

	while(e)
	{
		bool draw=false;

		switch(e->seqevent->GetStatus())
		{
		case NOTEON:
			{
				Note *note=(Note *)e->seqevent;
				OSTART end=note->GetNoteEnd();

				if(e->seqevent->GetEventStart()<=WindowSong()->GetSongPosition() && end>WindowSong()->GetSongPosition())
				{
					if(e->realtimeactive==false)
					{
						draw=true;
						e->realtimeactive=true;
					}
				}
				else
				{
					if(e->realtimeactive==true)
					{
						draw=true;
						e->realtimeactive=false;
					}
				}
			}
			break;

		default:
			{
				OSTART end=e->seqevent->GetEventStart()+2*SAMPLESPERBEAT;

				if(e->seqevent->GetEventStart()<=WindowSong()->GetSongPosition() && end>WindowSong()->GetSongPosition())
				{
					if(e->realtimeactive==false)
					{
						draw=true;
						e->realtimeactive=true;
					}
				}
				else
				{
					if(e->realtimeactive==true)
					{
						draw=true;
						e->realtimeactive=false;
					}
				}
			}
			break;
		}

		if(draw==true)
			e->Draw(true);

		e=e->NextEvent();
	}
}

void Edit_Event::CreateEvent()
{
	/*
	Edit_Event_Event *e=FindEvent(GetMouseX(),GetMouseY());

	EF_CreateEvent ec;

	if(mainsettings->eventcreation.lastcreatedevent && insertpattern && insertpattern->mediatype==MEDIATYPE_MIDI)
	{
	ec.event=mainsettings->eventcreation.GetEvent(createeventtype,createselection);

	if(ec.event)
	{
	ec.event->SetChannel(createchannel-1);
	ec.song=WindowSong();
	ec.pattern=(MIDIPattern *)insertpattern;
	ec.position=WindowSong()->GetSongPosition();
	ec.doundo=true;
	ec.addtolastundo=false;
	ec.playit=true;

	mainedit->CreateNewMIDIEvent(&ec);

	AddSteptoSongPosition();
	}
	}
	*/
}


void Edit_Event::CreateGotoMenu()
{
	DeletePopUpMenu(true);
	AddStandardGotoMenu();

	/*
	if(popmenu && infonote)
	{
	popmenu->AddLine();
	popmenu->AddFMenu("Focus Event",new menu_gotoeventeditor(this,GOTO_FOCUS),"F");
	}
	*/
}

void Edit_Event::CreateMenuList(guiMenu *menu)
{
	AddEventEditorMenu(menu);
//	o->AddLine(); 
}

void Edit_Event::CreateWindowMenu()
{
	if(NewWindowMenu())
	{
		CreateMenuList(menu);
		//AddEditMenu(menu);
	}
}

guiMenu *Edit_Event::CreateMenu()
{
	//	ResetUndoMenu();

	//	ResetUndoMenu();
	if(DeletePopUpMenu(true))
	{
		CreateMenuList(popmenu);	
	}

	//maingui->AddCascadeMenu(this,menu);
	return 0;

#ifdef OLDIE
	if(menu)
		menu->RemoveMenu();

	onlyselected=gfxonoffmenu=gmmenu=0;

	if(menu=new guiMenu)
	{
		AddEventEditorMenu(menu);

		guiMenu *o=menu->AddMenu("Event Editor",0);

		if(o)
		{
			

			// Options
			class menu_onlyselevents:public guiMenu
			{
			public:
				menu_onlyselevents(Edit_Event *ee){eventeditor=ee;}

				void MenuFunction()
				{	
					if(eventeditor->patternselection.showonlyselectedevents==true)
						eventeditor->patternselection.showonlyselectedevents=false;
					else
						eventeditor->patternselection.showonlyselectedevents=true;

					eventeditor->ShowOptionsMenu();
				} //

				Edit_Event *eventeditor;
			};

			o->AddFMenu(Cxs[CXS_SHOWONLYSELEVENTS],onlyselected=new menu_onlyselevents(this),patternselection.showonlyselectedevents);

			class menu_gm:public guiMenu
			{
			public:
				menu_gm(Edit_Event *ee){eventeditor=ee;}

				void MenuFunction()
				{	
					switch(eventeditor->generalMIDIdisplay)
					{
					case GM_OFF:
						eventeditor->generalMIDIdisplay=GM_ON;
						break;

					case GM_ON:
						eventeditor->generalMIDIdisplay=GM_OFF;
						break;
					}

					eventeditor->ShowOptionsMenu();
					eventeditor->ShowEvents();
				} //

				Edit_Event *eventeditor;
			};

			o->AddFMenu(Cxs[CXS_USEGM],gmmenu=new menu_gm(this));

			class menu_timeline:public guiMenu
			{
			public:
				menu_timeline(Edit_Event *ee){eventeditor=ee;}

				void MenuFunction()
				{	
					eventeditor->TimeLineOnOff();

				} //

				Edit_Event *eventeditor;
			};

			gfxonoffmenu=o->AddFMenu(Cxs[CXS_SHOWEVENTSTIMELINE],gmmenu=new menu_timeline(this),showgfx);
		}
	}

	maingui->AddCascadeMenu(this,menu);
	return menu;

#endif

}

void EventEditor_EventHeader_Callback(guiGadget_CW *g,int status)
{
	Edit_Event *e=(Edit_Event *)g->from;

	switch(status)
	{
	case DB_CREATE:
		e->eventsheader=(guiGadget_TabStartPosition *)g;
		e->InitTabs(e->eventsheader);
		break;

	case DB_PAINT:
		{
			e->ShowHeader();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		//	p->MouseClickInKeys(true);	
		break;

	case DB_LEFTMOUSEUP:
		//	p->MouseUpInKeys(true);
		break;

	case DB_DOUBLECLICKLEFT:
		//	p->MouseDoubleClickInKeys(true);
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInOverview(false);	
		break;
	}
}

void EventEditor_EventGFX_Callback(guiGadget_CW *g,int status)
{
	Edit_Event *e=(Edit_Event *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			e->eventsgfx=g;
		}
		break;

	case DB_PAINT:
		{
			e->ShowEventsGFX();
			e->ShowCycleAndPositions(g);
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		//	p->MouseClickInKeys(true);	
		break;

	case DB_LEFTMOUSEUP:
		//	p->MouseUpInKeys(true);
		break;

	case DB_DOUBLECLICKLEFT:
		//	p->MouseDoubleClickInKeys(true);
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInOverview(false);	
		break;
	}
}

void EventEditor_Event_Callback(guiGadget_CW *g,int status)
{
	Edit_Event *e=(Edit_Event *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			e->events=(guiGadget_TabStartPosition *)g;

			e->InitTabs(e->events);
			e->OnInit();
		}break;

	case DB_NEWSIZE:
		e->InitShowEvents();
		break;

	case DB_PAINT:
		{
			e->ShowEvents();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		if(e->events->InitDeltaY())
		{
			e->DeltaY(e->events);
		}
		break;

	case DB_LEFTMOUSEDOWN:
		e->MouseClickInEvents(true);
		break;

	case DB_LEFTMOUSEUP:
		e->MouseReleaseInEvents(true);
		break;

	case DB_DOUBLECLICKLEFT:
		//	p->MouseDoubleClickInKeys(true);
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInOverview(false);	
		break;
	}
}

void Edit_Event::InitGadgets()
{
	glist.SelectForm(0,0);
	guitoolbox.CreateToolBox(TOOLBOXTYPE_EDITOR);
	glist.Return();

	InitSelectionPatternGadget();

	int addw=INFOSIZE;

	glist.AddButton(-1,-1,addw,-1,"Data",GADGETID_DATA,showgfx==true?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED|MODE_AUTOTOGGLE:MODE_GROUP|MODE_TOGGLE|MODE_AUTOTOGGLE);
	glist.AddLX();

	filtergadget=glist.AddButton(-1,-1,3*addw,-1,GADGETID_FILTERID,MODE_TEXTCENTER,Cxs[CXS_EVENTDISPLAYFILTER]);

	glist.AddLX();

	notes=glist.AddButton(-1,-1,addw,-1,"Note",GADGETID_FILTERNOTES,MODE_TEXTCENTER|MODE_TOGGLE);
	glist.AddLX();

	control=glist.AddButton(-1,-1,addw,-1,"Control",GADGETID_FILTERCONTROL,MODE_TEXTCENTER|MODE_TOGGLE);
	glist.AddLX();

	pitch=glist.AddButton(-1,-1,addw,-1,"Pitchbend",GADGETID_FILTERPITCH,MODE_TEXTCENTER|MODE_TOGGLE);
	glist.AddLX();

	cpress=glist.AddButton(-1,-1,addw,-1,"ChlPress",GADGETID_FILTERCPRESS,MODE_TEXTCENTER|MODE_TOGGLE);
	glist.AddLX();

	ppress=glist.AddButton(-1,-1,addw,-1,"PolyPress",GADGETID_FILTERPPRESS,MODE_TEXTCENTER|MODE_TOGGLE);
	glist.AddLX();

	prog=glist.AddButton(-1,-1,addw,-1,"Program",GADGETID_FILTERPROGRAM,MODE_TEXTCENTER|MODE_TOGGLE);
	glist.AddLX();

	sysex=glist.AddButton(-1,-1,addw,-1,"SysEx",GADGETID_FILTERSYSEX,MODE_TEXTCENTER|MODE_TOGGLE);
	glist.AddLX();

	glist.AddButton(-1,-1,addw/4,-1,"R",GADGETID_FILTERRESET,MODE_TEXTCENTER|MODE_TOGGLE,"Reset Filter");
	glist.AddLX();

	ShowFilter();

	glist.SelectForm(1,1);
	glist.AddChildWindow(-1,-1,-1,SIZEV_HEADER,MODE_RIGHT|MODE_SPRITE,0,&Editor_Header_Callback,this);
	glist.Return();

	int offsettracksy=glist.form->gy;

	SliderCo horz,vert;

	horz.formx=0;
	horz.formy=2;

	vert.formx=2;
	vert.formy=1;
	vert.offsety=offsettracksy;
	vert.from=0;
	vert.to=0; // trackobjects.GetCount()-numberoftracks;
	vert.pos=0; //firstshowtracknr;

	AddEditorSlider(&horz,&vert);

	glist.SelectForm(0,1);

	glist.AddTabStartPosition(-1,-1,-1,offsettracksy-1,MODE_RIGHT,0,EventEditor_EventHeader_Callback,this); // Header
	glist.AddTabStartPosition(-1,offsettracksy,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,EventEditor_Event_Callback,this); // Event List

	glist.SelectForm(1,1);
	editarea=glist.AddChildWindow(-1,-1,-1,offsettracksy-1,MODE_RIGHT|MODE_BOTTOM|MODE_SPRITE,0,EventEditor_EventGFX_Callback,this); // GFX
}

void Edit_Event_Event::ShowChannel()
{
	int flag=NO_SHOWNUMBER;

	if(seqevent->IsSelected()==true)
		flag|=NO_SELECTED;

	bitmap->guiDrawNumberObject(editor->events->GetTabX(TAB_CHANNEL),y2,editor->events->GetTabX2(TAB_CHANNEL),seqevent->GetChannel()+1,flag);
}

void Edit_Event_Event::ShowStatus(char *status)
{	
	int flag=0;

	if(seqevent->IsSelected()==true)
		flag|=NO_SELECTED;

	bitmap->guiDrawText(editor->events->GetTabX(TAB_STATUS),y2,editor->events->GetTabX2(TAB_STATUS),status,flag);
}

void Edit_Event_Event::ShowByte1(char *byte1,int ib1)
{	
	if(byte1)
	{
		int flag=NO_SHOWNUMBER;

		if(seqevent->IsSelected()==true)
			flag|=NO_SELECTED;

		bitmap->guiDrawNumberObject(editor->events->GetTabX(TAB_BYTE1),y2,editor->events->GetTabX2(TAB_BYTE1),byte1,flag);
	}
}

void Edit_Event_Event::ShowByte2(char *byte2,int ib2)
{	
	if(byte2)
	{
		int flag=NO_SHOWNUMBER;

		if(seqevent->IsSelected()==true)
			flag|=NO_SELECTED;

		bitmap->guiDrawNumberObject(editor->events->GetTabX(TAB_BYTE2),y2,editor->events->GetTabX2(TAB_BYTE2),byte2,flag);
	}
}

void Edit_Event_Event::ShowByte1(int byte1)
{	
	int flag=NO_SHOWNUMBER;

	if(seqevent->IsSelected()==true)
		flag|=NO_SELECTED;

	bitmap->guiDrawNumberObject(editor->events->GetTabX(TAB_BYTE1),y2,editor->events->GetTabX2(TAB_BYTE1),byte1,flag);
}

void Edit_Event_Event::ShowByte2(int byte2)
{	
	int flag=NO_SHOWNUMBER;

	if(seqevent->IsSelected()==true)
		flag|=NO_SELECTED;

	bitmap->guiDrawNumberObject(editor->events->GetTabX(TAB_BYTE2),y2,editor->events->GetTabX2(TAB_BYTE2),byte2,flag);
}

void Edit_Event_Event::ShowByte3(int byte3)
{	
	//status_byte3=byte3;

	int flag=NO_SHOWNUMBER;

	if(seqevent->IsSelected()==true)
		flag|=NO_SELECTED;

	bitmap->guiDrawNumberObject(editor->events->GetTabX(TAB_BYTE3),y2,editor->events->GetTabX2(TAB_BYTE3),byte3,flag);

}

void Edit_Event_Event::ShowLength(char *l)
{	
	if(l)
	{
		int flag=0;

		if(seqevent->IsSelected()==true)
			flag|=NO_SELECTED;

		bitmap->guiDrawText(editor->events->GetTabX(TAB_LEN),y2,editor->events->GetTabX2(TAB_LEN),l,flag);
	}
}

void Edit_Event_Event::ShowInfo(char *info)
{	
	if(info)
	{
		int flag=0;

		if(seqevent->IsSelected()==true)
			flag|=NO_SELECTED;

		bitmap->guiDrawText(editor->events->GetTabX(TAB_INFO),y2,editor->events->GetTabX2(TAB_INFO),info,flag);
	}
}

bool Edit_Event_Event::CanEditCurve()
{
	return true;
}

Edit_Event_Event::Edit_Event_Event()
{
	set=false;
	grey=false;
	realtimeactive=false;
	gfx_x=-1;
	gfx_x2=-1;
}

char *Edit_Event_Event::GetTool1String()
{
	char *string=0;

	switch(seqevent->GetStatus())
	{
	case AUDIO:
		{
			string=mainvar->GenerateString("Audio");
		}
		break;

	default: // MIDI
		{
			switch(seqevent->GetStatus())
			{
			case NOTEON:
				{
					class Note *note=(Note *)seqevent;
					string=mainvar->GenerateString("Note ",maingui->ByteToKeyString(editor->WindowSong(),note->key));
				}
				break;

			case PITCHBEND:
				{
					Pitchbend *pitch=(Pitchbend *)seqevent;
					string=mainvar->GenerateString("Pitchbend");
				}
				break;

			case POLYPRESSURE:
				{
					PolyPressure *poly=(PolyPressure *)seqevent;
					string=mainvar->GenerateString("Poly Pressure");
				}
				break;

			case CONTROLCHANGE:
				{
					ControlChange *cc=(ControlChange *)seqevent;
					string=mainvar->GenerateString("Control Change");
				}
				break;

			case PROGRAMCHANGE:
				{
					ProgramChange *pc=(ProgramChange *)seqevent;
					string=mainvar->GenerateString("Program Change");
				}
				break;

			case CHANNELPRESSURE:
				{
					ChannelPressure *cp=(ChannelPressure *)seqevent;
					string=mainvar->GenerateString("Channel Pressure");

				}
				break;

			case SYSEX:
				{
					SysEx *sysex=(SysEx *)seqevent;
					string=mainvar->GenerateString("SysEx");
				}
				break;

			case INTERN:
				{
					ICD_Object *icd=(ICD_Object *)seqevent;
					string=mainvar->GenerateString("Intern");
				}
				break;

				case INTERNCHAIN:
				{
					ICD_Object *icd=(ICD_Object *)seqevent;
					string=mainvar->GenerateString("Intern");
				}
				break;
			}// switch MIDI event
		}

		break;
	}

	return string;
}



#ifdef REFREAL
void Edit_Event_Event::RefreshRealtime()
{
	switch(event->event->GetStatus())
	{
	case AUDIO:
		break;

	default:
		{			
			// MIDI Event ###############################################################

			if(event->event)
			{
				if(status_start!=event->event->GetEventStart())
				{
					Draw(true);
					return;
				}

				if(status_channel!=event->event->GetChannel())
				{
					Draw(true);
					return;
				}

				if(event->event->GetByte1()>=0 && status_byte1!=event->event->GetByte1())
				{
					Draw(true);
					return;
				}

				if(event->event->GetByte2()>=0 && status_byte2!=event->event->GetByte2())
				{
					Draw(true);
					return;
				}

				if(event->event->GetByte3()>=0 && status_byte3!=event->event->GetByte3())
				{
					Draw(true);
					return;
				}
			}

#ifdef TESTA
			switch(event->event->GetStatus())
			{
			case NOTEON:
				{
					class Note *note=(Note *)event->event;

					if(status_channel!=note->GetChannel())
						ShowChannel();
					ShowStatus("Note");

					ShowByte1(maingui->ByteToKeyString(editor->WindowSong(),note->key));
					ShowByte2(note->velocity);
					ShowByte3(note->velocityoff);

					// Note Length
					if(editor->frame.info.ondisplay==true)
					{
						editor->WindowSong()->timetrack.ConvertTicksToLength(note->GetNoteLength()+event->changedlength,editor->displayusepos);

						if(editor->displayusepos->pos[0])
						{
							editor->guibuffer->guiDrawNumberObject(editor->notelength_1000_x,y2,editor->frame.info.x2,editor->displayusepos->pos[0],&this->note_1000);
							editor->guibuffer->guiDrawText(editor->notelength_0100_x-editor->spacesize,y2,editor->frame.info.x2,editor->space);
						}
						else
							editor->guibuffer->guiDrawNumberObject(editor->notelength_1000_x,y2,editor->frame.info.x2,"_",&this->note_1000);

						editor->guibuffer->guiDrawNumberObject(editor->notelength_0100_x,y2,editor->frame.info.x2,editor->displayusepos->pos[1],&this->note_0100);

						editor->guibuffer->guiDrawText(editor->notelength_0010_x-editor->spacesize,y2,editor->frame.info.x2,editor->space);
						editor->guibuffer->guiDrawNumberObject(editor->notelength_0010_x,y2,editor->frame.info.x2,editor->displayusepos->pos[2],&this->note_0010);

						editor->guibuffer->guiDrawText(editor->notelength_0001_x-editor->spacesize,y2,editor->frame.info.x2,editor->space);

						if(editor->showquarterframe==true && editor->displayusepos->pos[4]!=0)
						{
							char htext[NUMBERSTRINGLEN];
							char htext2[NUMBERSTRINGLEN];

							htext2[0]=0;

							strcpy(htext2,mainvar->ConvertIntToChar(editor->displayusepos->pos[3],htext));
							mainvar->AddString(htext2,";");
							mainvar->AddString(htext2,mainvar->ConvertIntToChar(editor->displayusepos->pos[4],htext));

							editor->guibuffer->guiDrawNumberObject(editor->notelength_0001_x,y2,editor->frame.info.x2,htext2,&note_0001);
						}
						else	
							editor->guibuffer->guiDrawNumberObject(editor->notelength_0001_x,y2,editor->frame.info.x2,editor->displayusepos->pos[3],&this->note_0001);	
					}

					if(editor->timeline && editor->frame.gfx.ondisplay==true && note->GetEventStart()+event->changedposition<=editor->endposition)
					{
						int x=editor->timeline->ConvertTimeToX(note->GetEventStart()+event->changedposition);
						int x2=editor->timeline->ConvertTimeToX(note->GetNoteEnd()+event->changedlength);

						if(x2==x)
							x2=x+4; // 4 pix minimun

						if(x2==0)
							x2=editor->frame.gfx.x2;

						if(x>0 && x2>0)
						{
							editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_BLUE_LIGHT);

							gfx_x=x;
							gfx_x2=x2;
						}

						bool showkey=true;

						// GM Drum ?
						if(note->GetChannel()==editor->WindowSong()->generalMIDI_drumchannel)
						{
							switch(editor->generalMIDIdisplay)
							{
							case GM_OFF:
								{
								}
								break;

							case GM_ON:
								{
									if(editor->gmmap.keys[note->key])
									{
										editor->guibuffer->guiDrawText(editor->frame.gfx.x+1,y2-1,editor->frame.gfx.x2,editor->gmmap.keys[note->key]);
										showkey=false;								
									}

								}
								break;
							}
						}

						if(showkey==true)
						{
							editor->guibuffer->guiDrawText(x,y2-1,editor->frame.gfx.x2-1,maingui->ByteToKeyString(editor->WindowSong(),note->key));
						}
					}

					// Curve
					if(editor->frame.curve.ondisplay==true)
					{
						double h=note->velocity;
						double h2=(editor->frame.curve.x2-1)-(editor->frame.curve.x+1);

						h/=127;
						h*=h2;

						editor->guibuffer->guiFillRect3D(editor->frame.curve.x+1,y+1,(int)(editor->frame.curve.x+1+h),y2-1,COLOUR_BLACK);
					}
				}
				break;

			case PITCHBEND:
				{
					Pitchbend *pitch=(Pitchbend *)event->event;

					ShowChannel();
					ShowStatus("Pitch");

					ShowByte1(pitch->lsb);
					ShowByte2(pitch->msb);

					int i=128*pitch->msb;
					i+=pitch->lsb;

					i-=8192;

					ShowByte3(i);

					if(editor->timeline)
					{
						int x=editor->timeline->ConvertTimeToX(pitch->GetEventStart()+event->changedposition);

						if(x>0)
						{
							int x2=x+32;

							if(x2>editor->frame.gfx.x2)
								x2=editor->frame.gfx.x2;

							gfx_x=x;
							gfx_x2=x2;

							editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_RED);
							editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"Pitch");
						}
					}

					// Curve
					if(editor->frame.curve.ondisplay==true)
					{
						int mx=(editor->frame.curve.x2-1)-(editor->frame.curve.x+1);

						mx/=2;
						mx+=editor->frame.curve.x+1;

						if(i>0)
						{
							double h=i;
							double h2=(editor->frame.curve.x2-1)-mx;

							h/=8192;
							h*=h2;

							editor->guibuffer->guiFillRect(mx,y+1,(int)(mx+h)+1,y2-1,COLOUR_RED);
						}
						else
							if(i<0)
							{
								double h=-i;
								double h2=(editor->frame.curve.x2-1)-mx;

								h/=8192;
								h*=h2;

								editor->guibuffer->guiFillRect((int)(mx-h)-1,y+1,mx,y2-1,COLOUR_RED_LIGHT);
							}
							else
							{
								int my=y+((y2-y)/2);

								editor->guibuffer->guiDrawLine(editor->frame.curve.x+1,my,(int)(editor->frame.curve.x2-1),my,COLOUR_BLACK);
								editor->guibuffer->guiDrawLine(mx,y,mx,y2,COLOUR_BLACK);
							}
					}
				}
				break;

			case POLYPRESSURE:
				{
					PolyPressure *poly=(PolyPressure *)event->event;

					ShowChannel();
					ShowStatus("PolyPress");
					ShowByte1(maingui->ByteToKeyString(editor->WindowSong(),poly->key));
					ShowByte2(poly->pressure);

					if(editor->timeline)
					{
						int x=editor->timeline->ConvertTimeToX(poly->GetEventStart()+event->changedposition);

						if(x>0)
						{
							int x2=x+32;

							if(x2>editor->frame.gfx.x2)
								x2=editor->frame.gfx.x2;

							gfx_x=x;
							gfx_x2=x2;

							editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_BLUE);
							editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"Poly");
						}
					}

					// Curve
					if(editor->frame.curve.ondisplay==true)
					{
						double h=poly->pressure;
						double h2=(editor->frame.curve.x2-1)-(editor->frame.curve.x+1);

						h/=127;
						h*=h2;

						editor->guibuffer->guiFillRect3D(editor->frame.curve.x+1,y+1,(int)(editor->frame.curve.x+1+h),y2-1,COLOUR_BLUE);
					}
				}
				break;

			case CONTROLCHANGE:
				{
					ControlChange *cc=(ControlChange *)event->event;

					ShowChannel();
					ShowStatus("Control");
					ShowByte1(cc->controller);
					ShowByte2(cc->value);
					ShowInfo(maingui->ByteToControlInfo(cc->controller));

					if(editor->timeline)
					{
						int x=editor->timeline->ConvertTimeToX(cc->GetEventStart()+event->changedposition);

						if(x>0)
						{
							int x2=x+32;

							if(x2>editor->frame.gfx.x2)
								x2=editor->frame.gfx.x2;

							gfx_x=x;
							gfx_x2=x2;

							editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_GREEN);
							editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"Ctrl");
						}	
					}

					// Curve
					if(editor->frame.curve.ondisplay==true)
					{
						double h=cc->value;
						double h2=(editor->frame.curve.x2-1)-(editor->frame.curve.x+1);

						h/=127;
						h*=h2;

						editor->guibuffer->guiFillRect3D(editor->frame.curve.x+1,y+1,(int)(editor->frame.curve.x+1+h),y2-1,COLOUR_GREEN);
					}
				}
				break;

			case PROGRAMCHANGE:
				{
					ProgramChange *pc=(ProgramChange *)event->event;

					ShowChannel();
					ShowStatus("Program");
					ShowByte1(pc->program+1);

					if(editor->frame.byte2.ondisplay==true)
						editor->guibuffer->guiFillRect_RGB(editor->frame.byte2.x+1,this->y+1,editor->frame.byte2.x2-1,this->y2-1,0xDDEEFF);

					if(pc->info)
						ShowByte2(pc->info);

					switch(editor->generalMIDIdisplay)
					{
						/*
						case GMMODE_OFF:
						break;
						*/

					case GM_ON:
						ShowInfo(gmprognames[pc->program]);
						break;
					}

					if(editor->timeline)
					{
						int x=editor->timeline->ConvertTimeToX(pc->GetEventStart()+event->changedposition);

						if(x>0)
						{
							int x2=x+32;

							if(x2>editor->frame.gfx.x2)
								x2=editor->frame.gfx.x2;

							gfx_x=x;
							gfx_x2=x2;

							editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_YELLOW);
							editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"Prog");
						}
					}

					// Curve
					if(editor->frame.curve.ondisplay==true)
					{
						double h=pc->program;
						double h2=(editor->frame.curve.x2-1)-(editor->frame.curve.x+1);

						h/=127;
						h*=h2;

						editor->guibuffer->guiFillRect3D(editor->frame.curve.x+1,y+1,(int)(editor->frame.curve.x+1+h),y2-1,COLOUR_BLUE_LIGHT);
					}
				}
				break;

			case CHANNELPRESSURE:
				{
					ChannelPressure *cp=(ChannelPressure *)event->event;

					ShowChannel();
					ShowStatus("ChlPress");
					ShowByte1(cp->pressure);

					if(editor->timeline)
					{
						int x=editor->timeline->ConvertTimeToX(cp->GetEventStart()+event->changedposition);

						if(x>0)
						{
							int x2=x+32;

							if(x2>editor->frame.gfx.x2)
								x2=editor->frame.gfx.x2;

							gfx_x=x;
							gfx_x2=x2;

							editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_BLUE_LIGHT);
							editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"CPress");
						}
					}

					// Curve
					if(editor->frame.curve.ondisplay==true)
					{
						double h=cp->pressure;
						double h2=(editor->frame.curve.x2-1)-(editor->frame.curve.x+1);

						h/=127;
						h*=h2;

						editor->guibuffer->guiFillRect3D(editor->frame.curve.x+1,y+1,(int)(editor->frame.curve.x+1+h),y2-1,COLOUR_BLUE_LIGHT);
					}
				}
				break;

			case SYSEX:
				{
					SysEx *sysex=(SysEx *)event->event;

					ShowStatus("SysEx");
					ShowInfo(sysex->GetSysExString());

					editor->guibuffer->SetTextColour(255,0,0);
					ShowByte1(sysex->length);
					editor->guibuffer->SetTextColour(0,0,0);

					if(editor->timeline)
					{
						int x=editor->timeline->ConvertTimeToX(sysex->GetEventStart()+event->changedposition);

						if(x>0)
						{
							int x2=x+32;

							if(x2>editor->frame.gfx.x2)
								x2=editor->frame.gfx.x2;

							gfx_x=x;
							gfx_x2=x2;

							editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_GREY);
							editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"SysEx");
						}
					}
				}
				break;

			case INTERN:
				{
					ICD_Object *icd=(ICD_Object *)event->event;

					//guibuffer->guiDrawText(frame.status.x,y,frame.status.x2,"Intern");

					ShowStatus(icd->GetTypeName());
					ShowByte1(icd->GetMSB());
					ShowInfo(icd->GetInfo());

					if(editor->timeline)
					{
						int x=editor->timeline->ConvertTimeToX(icd->GetEventStart()+event->changedposition);

						if(x>0)
						{
							int x2=x+32;

							if(x2>editor->frame.info.x2)
								x2=editor->frame.info.x2;

							gfx_x=x;
							gfx_x2=x2;

							editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_GREY);
						}				
					}

					// Curve
					if(editor->frame.curve.ondisplay==true)
					{
						double h=icd->GetMSB();
						double h2=(editor->frame.curve.x2-1)-(editor->frame.curve.x+1);

						h/=127;
						h*=h2;

						editor->guibuffer->guiFillRect3D(editor->frame.curve.x+1,y+1,(int)(editor->frame.curve.x+1+h),y2-1,COLOUR_BLUE_LIGHT);
					}
				}
				break;

			}// switch MIDI event
#endif
		}
		break;

	}// switch

}
#endif

void Edit_Event_Event::DrawLength(OSTART start,OSTART end,int x,int x2)
{
	TimeString timestring;
	song->timetrack.CreateLengthString(&timestring,start,end,timemode);

	int tx=x; 

	lenindex=timestring.index;

	for(int i=0;i<timestring.index;i++)
	{
		int tox2;

		if(timestring.index==1)
			tox2=x2;
		else
			tox2=timestring.pos.usesmpte==true?tx+bitmap->pref_smpte[i]:tx+bitmap->pref_time[i];

		if(tox2>x2)
			tox2=x2;

		bitmap->guiDrawTextFontY(tx,y2,tox2-1,timestring.strings[i]);

		if(i<timestring.index-1 && strcmp(timestring.strings[i],"_")!=0)
		{
			int zx=tox2;
			zx-=bitmap->pref_space;
			bitmap->guiDrawTextFontY(zx,y2,tox2-1,timestring.pos.space[i]);
		}

		lenx[i]=tx;
		lenx2[i]=tox2;

		tx=tox2;
	}
}

void Edit_Event_Event::RefreshRealtime(OSTART spp,bool active)
{
	switch(seqevent->GetStatus())
	{
	case NOTEON:
		{
			Note *note=(Note *)seqevent;

			if(double h3=note->GetNoteLength())
			{
				if(active==true)
				{
					double h2;

					if(spp>=note->GetNoteEnd())
						h2=1;
					else
						if(spp<=note->GetEventStart())
							h2=0;
						else
						{
							h2=spp-note->GetEventStart();

							h2/=h3;
						}

						if(realtimeactive==false || h2!=realtime_h2)
						{
							realtime_h2=h2;
							realtimeactive=true;

							editor->events->gbitmap.guiFillRect(editor->events->GetTabX(TAB_LEN),y,editor->events->GetTabX2(TAB_LEN),y2,GetIndexColour());

							if(h2>0)
							{
								double h=editor->events->GetTabX2(TAB_LEN)-editor->events->GetTabX(TAB_LEN);

								h*=h2;
								editor->events->gbitmap.guiFillRect(editor->events->GetTabX(TAB_LEN),y,editor->events->GetTabX(TAB_LEN)+h,y2,COLOUR_GREEN);
							}

							DrawLength(note->GetEventStart(),note->GetNoteLength(),editor->events->GetTabX(TAB_LEN),editor->events->GetTabX2(TAB_LEN));
							editor->DrawDBBlit(editor->events,this);
						}
				}
				else
				{
					realtimeactive=false;

					editor->events->gbitmap.guiFillRect(editor->events->GetTabX(TAB_LEN),y,editor->events->GetTabX2(TAB_LEN),y2,GetIndexColour());
					DrawLength(note->GetEventStart(),note->GetNoteLength(),editor->events->GetTabX(TAB_LEN),editor->events->GetTabX2(TAB_LEN));
					editor->DrawDBBlit(editor->events,this);
				}
			}
		}
		break;

	}

}

void Edit_Event_Event::DrawGFX()
{
	if(!editor->eventsgfx)
		return;

	int fillcolour;

	if(seqevent->IsSelected()==true) // Event Selected
		fillcolour=COLOUR_BACKGROUNDEDITOR_GFX_HIGHLITE;
	else
		fillcolour=COLOUR_BACKGROUNDEDITOR_GFX;

	guiBitmap *gfxbitmap=&editor->eventsgfx->gbitmap;

	gfxbitmap->guiFillRectX0(y,gfxbitmap->GetX2(),y2,fillcolour);

	editor->timeline->DrawPositionRaster(gfxbitmap,y,y2);

	switch(seqevent->GetStatus())
	{
	case AUDIO:
		{
			AudioEvent *audio=(AudioEvent *)seqevent;

			OSTART pos=audio->GetAudioPattern()->GetPatternStart();
			OSTART endpos=audio->GetAudioPattern()->GetPatternEnd();

			if(seqevent->IsSelected()==true)
			{
				pos+=editor->numbereditposition_sum; // List Editor
				endpos+=editor->numbereditposition_sum;
			}

			if(endpos>editor->startposition)
			{
				AudioGFX gfx;

				gfx.audiopattern=audio->GetAudioPattern();
				gfx.win=editor;
				gfx.bitmap=gfxbitmap;
				gfx.usebitmap=true;
				gfx.drawborder=true;
				gfx.x2=gfxbitmap->GetX2();
				gfx.y=y;
				gfx.y2=y2;

				gfx.eventstart=pos;
				gfx.eventend=endpos;

				if(gfx.eventstart>=editor->startposition)
				{
					// int cx;

					gfx.start=gfx.eventstart;

					// cx=timeline->ConvertTimeToX(gfx.eventstart);


					// if(cx!=x)
					// editor->guibuffer->guiDrawLine(cx,y,cx,y2,COLOUR_GREEN);

				}
				else
					gfx.start=editor->startposition;

				gfx.x=editor->timeline->ConvertTimeToX(gfx.start);

				if(gfx.x>=0)
				{
					gfx.samplex2=gfx.eventend>editor->endposition?editor->timeline->x2:editor->timeline->ConvertTimeToX(gfx.eventend);

					if(gfx.samplex2>0)
					{
						if(audio->audioefile)
						{
							audio->audioefile->ShowAudioFile(editor->WindowSong(),&gfx);

							gfx_x=gfx.x;
							gfx_x2=gfx.samplex2;
						}
					}
				}
			}
		}
		break;

	default:
		{			
			// MIDI Event ###############################################################
			OSTART pos=seqevent->GetEventStart();

			if(seqevent->IsSelected()==true)
			{
				pos+=editor->numbereditposition_sum; // List Editor
			}

			switch(seqevent->GetStatus())
			{
			case INTERN:
				break;

			case INTERNCHAIN:
				{
					switch(seqevent->GetICD())
					{
					case ICD_TYPE_DRUM:
						{
							ICD_Drum *icd=(ICD_Drum *)seqevent;
							OSTART endpos=icd->drumtrack->ticklength+pos;

							if(icd->IsSelected()==true)
							{
								endpos+=editor->numbereditposition_sum; // List Editor
							}

							if(pos<editor->endposition && endpos>=editor->startposition)
							{
								int x=pos<=editor->startposition?0:editor->timeline->ConvertTimeToX(pos),
									x2=endpos>=editor->endposition?gfxbitmap->GetX2():editor->timeline->ConvertTimeToX(endpos);

								if(x>=0 && x2>0)
								{
									gfxbitmap->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_BLUE);

									gfx_x=x;
									gfx_x2=x2;
								}
							}

						}
						break;

					}
				}
				break;

			case NOTEON:
				{
					Note *note=(Note *)seqevent;
					OSTART endpos=note->GetNoteEnd();

					if(seqevent->IsSelected()==true)
					{
						endpos+=editor->numbereditposition_sum; // List Editor
					}

					if(pos<editor->endposition && endpos>=editor->startposition)
					{
						int x=pos<=editor->startposition?0:editor->timeline->ConvertTimeToX(pos),
							x2=endpos>=editor->endposition?gfxbitmap->GetX2():editor->timeline->ConvertTimeToX(endpos);

						if(x>=0 && x2>0)
						{
							gfxbitmap->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_BLUE_LIGHT);

							gfx_x=x;
							gfx_x2=x2;
						}

						bool showkey=true;

						// GM Drum ?
						if(note->GetChannel()==editor->WindowSong()->generalMIDI_drumchannel)
						{
							switch(editor->generalMIDIdisplay)
							{
							case GM_OFF:
								{
								}
								break;

							case GM_ON:
								{
									if(editor->gmmap.keys[note->key])
									{
										gfxbitmap->guiDrawText(1,y2-1,gfxbitmap->GetX2(),editor->gmmap.keys[note->key],flag);
										showkey=false;								
									}

								}
								break;
							}
						}

						if(showkey==true)
						{
							//gfxbitmap->guiDrawText(x,y2-1,editor->frame.gfx.x2-1,maingui->ByteToKeyString(editor->WindowSong(),note->key),flag);
						}
					}

				}
				break;

			case PITCHBEND:
				{
					Pitchbend *pitch=(Pitchbend *)seqevent;

					int i=128*pitch->msb;
					i+=pitch->lsb;

					i-=8192;

					ShowByte3(i);

					if(editor->timeline)
					{
						/*
						int x=editor->timeline->ConvertTimeToX(pitch->GetEventStart());

						if(x>0)
						{
						int x2=x+32;

						if(x2>editor->frame.gfx.x2)
						x2=editor->frame.gfx.x2;

						gfx_x=x;
						gfx_x2=x2;

						gfxbitmap->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_RED);
						gfxbitmap->guiDrawText(x,y2,editor->frame.gfx.x2,"Pitch",flag);
						}
						*/
					}

					// Curve
					//if(editor->frame.curve.ondisplay==true)
					{
						int mx=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

						mx/=2;
						mx+=editor->events->GetTabX(TAB_DATA);

						if(i>0)
						{
							double h=i,h2=editor->events->GetTabX2(TAB_DATA)-mx;

							h/=8192;
							h*=h2;

							bitmap->guiFillRect(mx,y+1,(int)(mx+h)+1,y2-1,COLOUR_RED);
						}
						else
							if(i<0)
							{
								double h=-i,h2=editor->events->GetTabX2(TAB_DATA)-mx;

								h/=8192;
								h*=h2;

								bitmap->guiFillRect((int)(mx-h)-1,y+1,mx,y2-1,COLOUR_RED_LIGHT);
							}
							else
							{
								int my=y+((y2-y)/2);

								bitmap->guiDrawLineY(my,editor->events->GetTabX(TAB_DATA),(int)(editor->events->GetTabX2(TAB_DATA)),COLOUR_BLACK);
								bitmap->guiDrawLine(mx,y,mx,y2,COLOUR_BLACK);
							}
					}
				}
				break;

			case POLYPRESSURE:
				{
					PolyPressure *poly=(PolyPressure *)seqevent;

					ShowChannel();
					ShowStatus("PolyPress");
					ShowByte1(maingui->ByteToKeyString(editor->WindowSong(),poly->key),poly->key);
					ShowByte2(poly->pressure);

					if(editor->timeline)
					{
						/*
						int x=editor->timeline->ConvertTimeToX(poly->GetEventStart());

						if(x>0)
						{
						int x2=x+32;

						if(x2>editor->frame.gfx.x2)
						x2=editor->frame.gfx.x2;

						gfx_x=x;
						gfx_x2=x2;

						editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_BLUE);
						editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"Poly",flag);
						}
						*/
					}
				}
				break;

			case CONTROLCHANGE:
				{
					ControlChange *cc=(ControlChange *)seqevent;

					if(editor->timeline)
					{
						/*
						int x=editor->timeline->ConvertTimeToX(cc->GetEventStart());

						if(x>0)
						{
						int x2=x+32;

						if(x2>editor->frame.gfx.x2)x2=editor->frame.gfx.x2;

						gfx_x=x;
						gfx_x2=x2;

						editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_GREEN);
						editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"Ctrl",flag);
						}
						*/
					}


				}
				break;

			case PROGRAMCHANGE:
				{
					ProgramChange *pc=(ProgramChange *)seqevent;

					if(editor->timeline)
					{
						/*
						int x=editor->timeline->ConvertTimeToX(pc->GetEventStart());

						if(x>0)
						{
						int x2=x+32;

						if(x2>editor->frame.gfx.x2)
						x2=editor->frame.gfx.x2;

						gfx_x=x;
						gfx_x2=x2;

						editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_YELLOW);
						editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"Prog",flag);
						}
						*/
					}
				}
				break;

			case CHANNELPRESSURE:
				{
					ChannelPressure *cp=(ChannelPressure *)seqevent;


					if(editor->timeline)
					{
						/*
						int x=editor->timeline->ConvertTimeToX(cp->GetEventStart());

						if(x>0)
						{
						int x2=x+32;

						if(x2>editor->frame.gfx.x2)x2=editor->frame.gfx.x2;

						gfx_x=x;
						gfx_x2=x2;

						editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_BLUE_LIGHT);
						editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"CPress",flag);
						}
						*/
					}
				}
				break;

			case SYSEX:
				{
					SysEx *sysex=(SysEx *)seqevent;

					if(editor->timeline)
					{
						/*
						int x=editor->timeline->ConvertTimeToX(sysex->GetEventStart());

						if(x>0)
						{
						int x2=x+32;

						if(x2>editor->frame.gfx.x2)x2=editor->frame.gfx.x2;

						gfx_x=x;
						gfx_x2=x2;

						editor->guibuffer->guiFillRect3D(x,y+1,x2,y2-1,COLOUR_GREY);
						editor->guibuffer->guiDrawText(x,y2,editor->frame.gfx.x2,"SysEx",flag);
						}
						*/
					}
				}
				break;

			}// switch MIDI event
		}
		break;
	}// switch

	if(seqevent->flag&OFLAG_UNDERSELECTION)
	{
		/*
		if(editor->frame.position.ondisplay==true)
		editor->guibuffer->guiInvert(editor->frame.position.x+1,y,editor->frame.position.x2-1,y2);

		if(editor->frame.gfx.ondisplay==true && gfx_x!=-1 && gfx_x2!=-1)
		editor->guibuffer->guiInvert(gfx_x,y,gfx_x2,y2);
		*/
	}
}

void Edit_Event_Event::Draw(bool single)
{
	eflag=seqevent->flag;

	object=seqevent;

	Colour colour;
	colour.InitColour(seqevent);

	gfx_x=gfx_x2=-1;

	if(seqevent->IsSelected()==true || seqevent==editor->patternselection.GetCursor()) // Event Selected
		fillcolour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
	else
		grey==true?fillcolour=COLOUR_GREY_LIGHT:fillcolour=COLOUR_BACKGROUNDFORMTEXT;

	OSTART pos=seqevent->GetEventStart();

	if(seqevent->IsSelected()==true)
	{
		pos+=editor->numbereditposition_sum; // List Editor
	}

	bitmap->guiFillRect(editor->events->GetTabX(TAB_TIME),y,x2,y2,GetIndexColour());
	bitmap->SetFont(seqevent);

	DrawPosition(pos,editor->events->GetTabX(TAB_TIME),editor->events->GetTabX2(TAB_TIME),seqevent);

	int flag=NO_SHOWNUMBER;

	if(seqevent->IsSelected()==true)
		flag|=NO_SELECTED;

	if(seqevent->flag&EVENTFLAG_MUTED)
		flag|=NO_MUTED;

	switch(seqevent->GetStatus())
	{
	case AUDIO:
		{
			AudioEvent *audio=(AudioEvent *)seqevent;

			bitmap->guiDrawText(editor->events->GetTabX(TAB_STATUS),y2,editor->events->GetTabX2(TAB_STATUS),"Audio",flag);

			DrawPosition(audio->GetAudioPattern()->GetPatternEnd(),editor->events->GetTabX(TAB_INFO),editor->events->GetTabX2(TAB_INFO),seqevent);

			bitmap->guiDrawNumber(editor->events->GetTabX(TAB_CHANNEL),y2,editor->events->GetTabX2(TAB_CHANNEL),audio->audioefile->channels);
			bitmap->guiDrawText(editor->events->GetTabX(TAB_BYTE1),y2,editor->events->GetTabX2(TAB_LEN),audio->GetAudioPattern()->GetName(),flag);
		}
		break;

	default:
		{			
			// MIDI Event ###############################################################
			switch(seqevent->GetStatus())
			{
			case NOTEON:
				{
					Note *note=(Note *)seqevent;

					ShowChannel();
					ShowStatus("Note");

					ShowByte1(maingui->ByteToKeyString(editor->WindowSong(),note->key),note->key);

					ShowByte2(note->velocity);
					
					ShowByte3(note->velocityoff);

					flag CLEARBIT NO_SHOWNUMBER;

					// Note Length

					DrawLength(note->GetEventStart(),note->GetNoteLength(),editor->events->GetTabX(TAB_LEN),editor->events->GetTabX2(TAB_LEN));

					// Curve

					double h=note->velocity;
					double h2=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

					h/=127;
					h*=h2;

					bitmap->guiFillRect3D(editor->events->GetTabX(TAB_DATA),y+1,(int)(editor->events->GetTabX(TAB_DATA)+h),y2-1,COLOUR_BLACK);
				}
				break;

			case PITCHBEND:
				{
					Pitchbend *pitch=(Pitchbend *)seqevent;

					ShowChannel();
					ShowStatus("Pitch");

					ShowByte1(pitch->lsb);
					ShowByte2(pitch->msb);

					int i=pitch->GetPitch();

					ShowByte3(i);

					// Curve

					int mx=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

					mx/=2;
					mx+=editor->events->GetTabX(TAB_DATA);

					if(i>0)
					{
						double h=i,h2=editor->events->GetTabX2(TAB_DATA)-mx;

						h/=8192;
						h*=h2;

						bitmap->guiFillRect(mx,y+1,(int)(mx+h)+1,y2-1,COLOUR_RED);
					}
					else
						if(i<0)
						{
							double h=-i,h2=editor->events->GetTabX2(TAB_DATA)-mx;

							h/=8192;
							h*=h2;

							bitmap->guiFillRect((int)(mx-h)-1,y+1,mx,y2-1,COLOUR_RED_LIGHT);
						}
						else
						{
							int my=y+((y2-y)/2);

							bitmap->guiDrawLineY(my,editor->events->GetTabX(TAB_DATA),(int)(editor->events->GetTabX2(TAB_DATA)),COLOUR_BLACK);
							bitmap->guiDrawLine(mx,y,mx,y2,COLOUR_BLACK);
						}
				}
				break;

			case POLYPRESSURE:
				{
					PolyPressure *poly=(PolyPressure *)seqevent;

					ShowChannel();
					ShowStatus("PolyPress");
					ShowByte1(maingui->ByteToKeyString(editor->WindowSong(),poly->key),poly->key);
					ShowByte2(poly->pressure);

					// Curve

					double h=poly->pressure;
					double h2=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

					h/=127;
					h*=h2;

					bitmap->guiFillRect3D(editor->events->GetTabX(TAB_DATA),y+1,(int)(editor->events->GetTabX(TAB_DATA)+h),y2-1,COLOUR_BLUE);
				}
				break;

			case CONTROLCHANGE:
				{
					ControlChange *cc=(ControlChange *)seqevent;

					ShowChannel();
					ShowStatus("Control");
					ShowByte1(cc->controller);
					ShowByte2(cc->value);
					ShowInfo(maingui->ByteToControlInfo(cc->controller,cc->value));

					// Curve

					double h=cc->value;
					double h2=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

					h/=127;
					h*=h2;

					bitmap->guiFillRect3D(editor->events->GetTabX(TAB_DATA),y+1,(int)(editor->events->GetTabX(TAB_DATA)+h),y2-1,COLOUR_GREEN);
				}
				break;

			case PROGRAMCHANGE:
				{
					ProgramChange *pc=(ProgramChange *)seqevent;

					ShowChannel();
					ShowStatus("Program");
					ShowByte1(pc->program+1);

					bitmap->guiFillRect_RGB(editor->events->GetTabX(TAB_BYTE2),y+1,editor->events->GetTabX2(TAB_BYTE2),y2-1,0xDDEEFF);

					if(pc->info)
						ShowByte2(pc->info,0);

					switch(editor->generalMIDIdisplay)
					{
						/*
						case GMMODE_OFF:
						break;
						*/

					case GM_ON:
						ShowInfo(gmprognames[pc->program]);
						break;
					}

					// Curve
					double h=pc->program;
					double h2=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

					h/=127;
					h*=h2;

					bitmap->guiFillRect3D(editor->events->GetTabX(TAB_DATA),y+1,(int)(editor->events->GetTabX(TAB_DATA)+h),y2-1,COLOUR_BLUE_LIGHT);
				}
				break;

			case CHANNELPRESSURE:
				{
					ChannelPressure *cp=(ChannelPressure *)seqevent;

					ShowChannel();
					ShowStatus("ChlPress");
					ShowByte1(cp->pressure);

					// Curve
					double h=cp->pressure;
					double h2=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

					h/=127;
					h*=h2;

					bitmap->guiFillRect3D(editor->events->GetTabX(TAB_DATA),y+1,(int)(editor->events->GetTabX(TAB_DATA)+h),y2-1,COLOUR_BLUE_LIGHT);
				}
				break;

			case SYSEX:
				{
					SysEx *sysex=(SysEx *)seqevent;

					ShowStatus("SysEx");
					ShowInfo(sysex->GetSysExString());
					bitmap->SetTextColour(255,0,0);

					char l[NUMBERSTRINGLEN];
					ShowLength(mainvar->ConvertIntToChar(sysex->length,l));
					bitmap->SetTextColour(0,0,0);
				}
				break;

			case INTERN:
				{
					ICD_Object *icd=(ICD_Object *)seqevent;

					ShowStatus(icd->GetTypeName());
					ShowByte1(icd->GetByte1());

					if(icd->GetByte2()>=0)
						ShowByte2(icd->GetByte2());

					ShowInfo(icd->GetInfo());

					// Curve

					double h=icd->GetByte1();
					double h2=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

					h/=127;
					h*=h2;

					bitmap->guiFillRect3D(editor->events->GetTabX(TAB_DATA),y+1,(int)(editor->events->GetTabX(TAB_DATA)+h),y2-1,COLOUR_BLUE_LIGHT);
				}
				break;

			case INTERNCHAIN:
				{
					ICD_Object_Seq_MIDIChainEvent *icd=(ICD_Object_Seq_MIDIChainEvent *)seqevent;

					ShowStatus(icd->GetTypeName());
					ShowByte1(icd->GetByte1());

					if(icd->GetByte2()>=0)
						ShowByte2(icd->GetByte2());

					ShowInfo(icd->GetInfo());

					// Curve
					double h=icd->GetByte1();
					double h2=editor->events->GetTabX2(TAB_DATA)-editor->events->GetTabX(TAB_DATA);

					h/=127;
					h*=h2;

					bitmap->guiFillRect3D(editor->events->GetTabX(TAB_DATA),y+1,(int)(editor->events->GetTabX(TAB_DATA)+h),y2-1,COLOUR_BLUE_LIGHT);
				}
				break;

			}// switch MIDI event
		}
		break;
	}// switch

	if(seqevent->flag&OFLAG_UNDERSELECTION)
	{
		/*
		if(editor->frame.position.ondisplay==true)
		editor->guibuffer->guiInvert(editor->frame.position.x+1,y,editor->frame.position.x2-1,y2);

		if(editor->frame.gfx.ondisplay==true && gfx_x!=-1 && gfx_x2!=-1)
		editor->guibuffer->guiInvert(gfx_x,y,gfx_x2,y2);
		*/
	}

	if(song->GetFocusEvent()==seqevent)
	{
		activeevent=true;
		bitmap->guiDrawRect(editor->events->GetTabX(TAB_TIME),y,editor->events->GetTabX2(TAB_INFO),y2,COLOUR_FOCUSOBJECT);
	}
	else
		activeevent=false;

	set=true;
}

void Edit_Event::Init()
{
	patternselection.BuildEventList(SEL_ALLEVENTS,&displayfilter); // Mix new List, events maybe moved/deleted

	Seq_SelectionEvent *sel=patternselection.FirstMixEvent(startposition);

	if(!sel)
		sel=patternselection.LastMixEvent();

	if(sel)
		patternselection.SetCursor(sel->seqevent);

	InitGadgets();

	notetype=WindowSong()->notetype;
}

void Edit_Event::ShowEventsHoriz(int flag)
{
	DrawDBBlit(events,showgfx==true?eventsgfx:0);
}

void Edit_Event::DeInitWindow()
{
	EventEditor_Selection::DeInitWindow();
}

char *Edit_Event::GetWindowName()
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

void Edit_Event::RefreshRealtime_Slow()
{
	if(notetype!=WindowSong()->notetype)
	{
		notetype=WindowSong()->notetype;
		DrawDBBlit(events,eventsgfx);
	}

	CheckNumberObjects(events);
}

