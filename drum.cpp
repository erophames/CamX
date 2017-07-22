#include "songmain.h"
#include "editor.h"
#include "drumeditor.h"
#include "gui.h"
#include "undo.h"
#include "camxgadgets.h"
#include "imagesdefines.h"
#include "object_project.h"
#include "object_song.h"
#include "object_track.h"
#include "objectpattern.h"
#include "mainhelpthread.h"
#include "MIDIhardware.h"
#include "drumevent.h"
#include "editbuffer.h"
#include "editfunctions.h"
#include "undofunctions.h"
#include "settings.h"
#include "editdata.h"
#include "chunks.h"
#include "audiohardware.h"
#include "drumeditorlist.h"
#include "drumeditorfx.h"
#include "editortabs.h"

enum Messages{
	MESSAGE_DRUM
};

#define LASTDRUMTRACKID OBJECTID_DRUMTRACKNAME


enum{
	GADGETID_TRACK=GADGETID_EDITORBASE,
	GADGETID_LIST,
	GADGETID_EDITNOTE_TIME_I,
	GADGETID_EDITNOTE_TIME,
	GADGETID_DRUMMAP,
};

void Edit_Drum_Drum::Draw()
{
	eflag=drum->flag;

	int colour,bordercolour;

	if(drum==editor->WindowSong()->GetFocusEvent())
	{
		infodrum=true;
		colour=drum->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT;
	}
	else
		if(eflag&EVENTFLAG_MUTED)
			colour=COLOUR_RED;
		else
		{
			if(drum->IsSelected()==true)
				colour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
			else
				colour=COLOUR_BACKGROUND;
		}

		if(eflag&EVENTFLAG_MOVEEVENT)
		{
			bordercolour=COLOUR_BORDER_OBJECTMOVING;
		}
		else
		{
			if(drum->IsSelected()==true)
				bordercolour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
			else
			{
				bordercolour=drum->pattern==editor->insertpattern?COLOUR_BLACK:COLOUR_GREY_DARK;
			}
		}

		bitmap->guiFillRect(x,y,x2,y2,colour,bordercolour);
		double h=(y2-1)-(y+1);

		double h2=drum->velocity;
		h2/=127;

		h*=h2;

		bitmap->guiFillRect(x+1,(y2-1)-(int)h,x2-1,y2-1,COLOUR_BLACK);

		if(eflag&OFLAG_UNDERSELECTION)
			bitmap->guiInvert(x,y,x2,y2);
}

char *Edit_Drum::GetWindowName()
{
	if(windowname)
	{
		size_t sl=strlen(windowname);

		sl+=strlen(WindowSong()->drummap.GetName());


		/*
		if(focustrack)
		{
		sl+=strlen(focustrack->name);
		}
		*/

		sl+=128;
		char *h=new char[sl];

		if(h)
		{
			strcpy(h,windowname);

			//if(drummap)
			{
				mainvar->AddString(h," <Drum Map:");
				mainvar->AddString(h,WindowSong()->drummap.GetName());
				mainvar->AddString(h,">");
			}
			/*
			else
			{
			mainvar->AddString(h," No Drum Map");
			}
			*/

			delete windowname;
			windowname=h;

		}

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
		}

		return windowname;
	}

	return windowname;
}

void Edit_Drum::ShowMenu()
{
	ShowEditorMenu();
}

void Edit_Drum::CreateMenuList(guiMenu *menu)
{
	if(!menu)
		return;

	AddEventEditorMenu(menu);

	/*
	if(menu->functionsmenu)
	{
	class menu_setnotelength:public guiMenu
	{
	public:
	menu_setnotelength(Edit_Piano *piano){editor=piano;}

	void MenuFunction()
	{
	mainedit->SetNoteLength(editor,&editor->patternselection);	
	}

	Edit_Piano *editor;
	};

	menu->functionsmenu->AddFMenu(Cxs[CXS_SETLENGTHOFNOTES],new menu_setnotelength(this));
	}
	*/
}

void Edit_Drum::CreateWindowMenu()
{
	if(NewWindowMenu())
	{
		CreateMenuList(menu);
		//AddEditMenu(menu);
	}
}


void Edit_Drum::CreateGotoMenu()
{
	DeletePopUpMenu(true);
	AddStandardGotoMenu();

	if(popmenu && WindowSong()->GetFocusEvent())
	{
		popmenu->AddLine();
		popmenu->AddFMenu("Focus Event",new menu_gotoeventeditor(this,GOTO_FOCUS),"F");
	}
}

guiMenu *Edit_Drum::CreateMenu()
{
	//	ResetUndoMenu();
	if(DeletePopUpMenu(true))
	{
		CreateMenuList(popmenu);	
	}

	//maingui->AddCascadeMenu(this,menu);
	return 0;

#ifdef OLDIE
	//	ResetUndoMenu();

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		if(patternselection.FirstSelectedPattern())
			AddEventEditorMenu(menu);		
		else
			functionsmenu=menu->AddMenu("Drummap",0);

		if(functionsmenu)
		{
			class menu_createdrummap:public guiMenu
			{
			public:
				void MenuFunction()
				{
					maindrummap->AddDrummap(true);
				} //
			};

			functionsmenu->AddFMenu("Add Drummap",new menu_createdrummap());

			functionsmenu->AddMenu("Load Drummap",DRUMMENU_LOADMAP);
			functionsmenu->AddMenu("Save Drummap",DRUMMENU_SAVEMAP);

			functionsmenu->AddLine();

			class menu_createdrumtrack:public guiMenu
			{
			public:
				menu_createdrumtrack(Edit_Drum *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->CreateNewDrumTrack(editor->focustrack);
				} //

				Edit_Drum *editor;
			};

			functionsmenu->AddFMenu("Create new Drum Track",new menu_createdrumtrack(this));

			class menu_deletedrumtrack:public guiMenu
			{
			public:
				menu_deletedrumtrack(Edit_Drum *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->DeleteDrumTrack(editor->focustrack);
				} //

				Edit_Drum *editor;
			};
			functionsmenu->AddFMenu("Delete Drum Track",new menu_deletedrumtrack(this));

			if(patternselection.FirstSelectedPattern())
			{
				functionsmenu->AddLine();

				class menu_MIDItodrum:public guiMenu
				{
				public:
					menu_MIDItodrum(Edit_Drum *e)
					{
						drumeditor=e;
					}

					void MenuFunction()
					{	
						drumeditor->ConvertNotesToDrums();
					} //

					Edit_Drum *drumeditor;
				};

				functionsmenu->AddFMenu("Convert MIDI Notes to Drums",new menu_MIDItodrum(this));

				class menu_drumstonotes:public guiMenu
				{
				public:
					menu_drumstonotes(Edit_Drum *e)
					{
						drumeditor=e;
					}

					void MenuFunction()
					{	
						drumeditor->ConvertDrumsToNotes();
					} //

					Edit_Drum *drumeditor;
				};

				functionsmenu->AddFMenu("Convert Drums to MIDI Notes",new menu_drumstonotes(this));
			}		
		}		
	}

	maingui->AddCascadeMenu(this,menu);
#endif

	return menu;
}

void Edit_Drum::ConvertDrumsToNotes()
{
	mainedit->ConvertDrumsToNotes(&patternselection);
}

void Edit_Drum::ConvertNotesToDrums()
{
	mainedit->ConvertNotesToDrums(&patternselection,&WindowSong()->drummap);
}

void Edit_Drum_Track::ShowName()
{
	if(name.ondisplay==false)
		return;

	int bgcolour;

	if(track->IsSelected()==true)
		bgcolour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
	else
		bgcolour=COLOUR_BACKGROUND;

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->SetTextColour(COLOUR_BLACK);
	bitmap->guiFillRect(name.x,name.y,name.x2,name.y2,bgcolour);
	bitmap->guiDrawText(name.x+1,name.y2,name.x2-1,track->GetName());

}

void Edit_Drum_Track::ShowMute(bool blit)
{
#ifdef OLDIE
	if(muteobject)
	{
		muteobject->mutestatus=track->mute;

		switch(track->mute)
		{
		case false:
			{
				guiBitmap *on=maingui->gfx.FindBitMap(IMAGE_TRACKMUTEOFF);

				if(on 
					// && (h+on->height)<=editor->frame_tracks.y2
					)
				{
					bitmap->guiDrawImage(muteobject->x,muteobject->y,muteobject->x2,muteobject->y2,on);
				}
			}
			break;

		case true:
			{
				guiBitmap *off=maingui->gfx.FindBitMap(IMAGE_TRACKMUTEON);

				if(off 
					// && (h+off->height)<=editor->frame_tracks.y2
					)
				{
					bitmap->guiDrawImage(muteobject->x,muteobject->y,muteobject->x2,muteobject->y2,off);
				}
			}
			break;
		}

		if(blit==true)
			editor->BltGUIBuffer_Frame(&editor->frame_tracks);
	}
#endif

}

void Edit_Drum_Track::ShowVolume()
{
#ifdef OLDIE
	if(volumeobject && editor->guibuffer)
	{
		char th[NUMBERSTRINGLEN];

		editor->guibuffer->guiFillRect(volumeobject->x,volumeobject->y,volumeobject->x2,volumeobject->y2,editor->frame_tracks.bpen);
		editor->guibuffer->guiDrawText(volumeobject->x,volumeobject->y2-1,volumeobject->x2,mainvar->ConvertIntToChar(volumeobject->volume=track->volume,th));
	}
#endif
}

void Edit_Drum_Track::ShowSolo()
{
#ifdef OLDIE
	if(soloobject)
	{
		soloobject->solostatus=track->solo;

		switch(track->solo)
		{
		case false:
			{
				guiBitmap *on=maingui->gfx.FindBitMap(IMAGE_TRACKSOLOOFF);

				if(on 
					)
				{
					bitmap->guiDrawImage(soloobject->x,soloobject->y,soloobject->x2,soloobject->y2,on);
				}
			}
			break;

		case true:
			{
				guiBitmap *off=maingui->gfx.FindBitMap(IMAGE_TRACKSOLOON);

				if(off 
					)
				{
					bitmap->guiDrawImage(soloobject->x,soloobject->y,soloobject->x2,soloobject->y2,off);
				}
			}
			break;
		}		
	}
#endif

}

void Edit_Drum::MouseWheel(int delta,guiGadget *db)
{
	if(Edit_MouseWheel(delta)==false)
	{
		if((!db) || db==tracks || db==eventsgfx)
		{
			if(vertgadget)
				vertgadget->DeltaY(delta);
		}
	}
}


EditData * Edit_Drum::EditDataMessage(EditData *data)
{
#ifdef OLDIE
	data=trackfx.EditDataMessage(data);

	if(data)
	{
		switch(data->id)
		{
		case EDIT_DRUMNAME:
			if(drummap)
			{
				drummap->SetName(data->newstring,false);

				// Refresh GUI
				guiWindow *f=maingui->FirstWindow();

				while(f)
				{
					switch(f->GetEditorID())
					{
					case EDITORTYPE_DRUM:
						{
							Edit_Drum *ed=(Edit_Drum *)f;

							if(ed->drummap==drummap)
							{
								ed->SongNameRefresh();
								ed->trackfx.ShowActiveTrack();
							}
						}
						break;
					}

					f=f->NextWindow();
				}

			}
			break;
		}
	}

#endif

	return 0;
}

void Edit_Drum::MoveTrack(Drumtrack *t,int diff)
{
	if(t && diff)
	{
		if(t->GetList()->MoveOIndex(t,diff)==true)
		{
			maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_DRUM,1); // 1== ShowTracks
		}
	}
}

void Edit_Drum::Goto(int to)
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
			{
				el=patternselection.FirstMixEvent();
			}
			break;

		case GOTO_LAST:
			{
				el=patternselection.LastMixEvent();
			}
			break;

		case GOTO_FIRSTSELECTED:
			{
				el=patternselection.FirstMixEvent();

				while(el && 
					(
					(!(el->seqevent->flag&OFLAG_SELECTED)) || (focustrack->CheckEvent(el->seqevent)==false)
					)
					) // Drum ICD
					el=el->NextEvent();
			}
			break;

		case GOTO_LASTSELECTED:
			{
				el=patternselection.LastMixEvent();

				while(
					el &&
					(
					(!(el->seqevent->flag&OFLAG_SELECTED)) || (focustrack->CheckEvent(el->seqevent)==false)
					)
					) // Drum ICD
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
					focustrack->CheckEvent(el->seqevent)==false
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
					focustrack->CheckEvent(el->seqevent)==false
					)
					)
					el=el->PrevEvent();
			}
			break;
		}

		if(el)
		{
			if((NewStartPosition(el->seqevent->GetEventStart(),true)==true))
			{
				SyncWithOtherEditors();
			}

			if(focustrack->CheckEvent(el->seqevent)==true) // Scroll Up<->
			{
				/*
				Note *n=(Note *)el->event;

				int nsk=n->key+numberofkeys;

				if(nsk>127)
				nsk=127-numberofkeys;

				if(startkey!=nsk)
				{
				startkey=nsk;
				Init();
				}
				*/
			}
		}//if el
	}//if focustrack
}


void Edit_Drum::KeyDown()
{
	Editor_KeyDown();

	/*
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
	PlayFocusTrack();
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
	PlayFocusTrack();
	}
	}
	}
	break;

	// Cursor ?
	case 'W':
	if(cursor.drumtrack && cursor.drumtrack->PrevTrack())
	{
	cursor.drumtrack=cursor.drumtrack->PrevTrack();
	ShowCursor();
	PlayCursor();
	}
	break;

	case 'S':
	if(cursor.drumtrack && cursor.drumtrack->NextTrack())
	{
	cursor.drumtrack=cursor.drumtrack->NextTrack();
	ShowCursor();
	PlayCursor();
	}
	break;

	case 'Q':
	{
	SetCursorToMousePosition();
	PlayCursor();
	}
	break;

	case 'R': // Delete Note under Cursor
	if(cursor.drumtrack)
	{
	ICD_Drum *f=FindDrumUnderCursor();

	if(f)
	mainedit->DeleteEvent(f,false);
	}
	break;

	case 'E': // create Note under Cursor
	if(
	insertpattern && 
	(insertpattern->mediatype==MEDIATYPE_MIDI) &&
	cursor.drumtrack
	)
	{
	ICD_Drum *drum=new ICD_Drum;

	if(drum)
	{
	drum->staticostart=cursor.ostart;

	drum->drumtrack=cursor.drumtrack;
	drum->velocity=127;
	drum->velocityoff=0;

	editevent.song=WindowSong();
	editevent.pattern=(MIDIPattern *)insertpattern;
	editevent.event=drum;

	editevent.position=cursor.ostart;

	editevent.doundo=true;
	editevent.addtolastundo=false;
	editevent.playit=true;

	mainedit->CreateNewMIDIEvent(&editevent);
	}
	}
	break;

	case 'A':
	case 'D':
	if(cursor.drumtrack)
	{
	OSTART add;

	switch(mousequantize)
	{
	case MOUSEQUANTIZE_MEASURE: // Measure
	{
	OSTART h=WindowSong()->timetrack.ConvertTicksToMeasureTicks(cursor.ostart,false);

	Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(h);

	add=(h+sig->measurelength)-cursor.ostart;
	}
	break;

	case MOUSEQUANTIZE_BEAT: // Measure
	{
	OSTART h=WindowSong()->timetrack.ConvertTicksToBeatTicks(cursor.ostart,false);

	Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(h);

	add=(h+sig->dn_ticks)-cursor.ostart;
	}
	break;

	case MOUSEQUANTIZE_1:
	add=TICK1nd;
	break;

	case MOUSEQUANTIZE_12:
	add=TICK2nd;
	break;

	case MOUSEQUANTIZE_14:
	add=TICK4nd;
	break;

	case MOUSEQUANTIZE_18:
	add=TICK8nd;
	break;

	case MOUSEQUANTIZE_16:
	add=TICK16nd;
	break;

	case MOUSEQUANTIZE_ZOOM:
	add=WindowSong()->timetrack.zoomticks;
	break;

	default:
	add=1; // 1 tick
	break;
	}

	if(nVirtKey=='A')
	add*=-1;

	OSTART newstart=cursor.ostart+add;

	if(newstart<0)
	newstart=0;

	if(cursor.ostart!=newstart)
	{
	cursor.ostart=newstart;
	ShowCursor();
	PlayCursor();
	}
	}
	break;
	}
	*/
}

void Edit_Drum::KeyDownRepeat()
{
	Editor_KeyDown();

	/*
	switch(nVirtKey)
	{
	case KEY_UP10:
	case KEY_CURSORUP:
	{
	// Active Track --
	if(OObject *no=trackobjects.FindPrevSameObject(trackobjects.FindObject(WindowSong()->GetFocusTrack())))
	{
	WindowSong()->SetFocusTrackLock((Seq_Track *)no->object,true,this);
	//BuildTrackList(&trackobjects);
	ScrollTo(WindowSong()->GetFocusTrack());
	}
	}
	break;

	case KEY_DOWN10:
	case KEY_CURSORDOWN:
	{
	// Active Track ++
	if(OObject *no=trackobjects.FindNextSameObject(trackobjects.FindObject(WindowSong()->GetFocusTrack())))
	{
	WindowSong()->SetFocusTrackLock((Seq_Track *)no->object,true,this);
	//BuildTrackList(&trackobjects);
	ScrollTo(WindowSong()->GetFocusTrack());
	}	
	}
	break;
	}
	*/

}

void Edit_Drum::KeyUp()
{

}


bool Edit_Drum::FindDrumEventsInsideMap(Seq_Song *s,Drumtrack *dt)
{
	Seq_Project *p=mainvar->FirstProject();

	while(p)
	{
		Seq_Song *song=p->FirstSong();

		while(song)
		{
			Seq_Track *t=song->FirstTrack();

			while(t)
			{
				Seq_Pattern *p=t->FirstPattern(MEDIATYPE_MIDI);

				while(p)
				{
					MIDIPattern *mp=(MIDIPattern *)p;
					Seq_Event *e=mp->FirstEvent();

					while(e)
					{
						if(e->GetICD()==ICD_TYPE_DRUM)
						{
							ICD_Drum *drum=(ICD_Drum *)e;

							if(drum->drumtrack==dt)
								return true;
						}

						e=e->NextEvent();
					}

					p=p->NextPattern(MEDIATYPE_MIDI);
				}

				t=t->NextTrack();
			}

			song=song->NextSong();
		}

		p=p->NextProject();
	}

	return false;
}

void Edit_Drum::Gadget(guiGadget *gadget)
{	
	gadget=Editor_Gadget(gadget);

	if(gadget)
		switch(gadget->gadgetID)
	{
		case GADGETID_TOOLBOX_QUANTIZE:
			mainedit->QuantizeEventsMenu(this,&patternselection);
			break;

		case GADGETID_LIST:
			{
				showlist=showlist==true?false:true;
				FormEnable(0,1,showlist);
				mainsettings->showdrumlist=showlist;
			}
			break;

		case GADGETID_TRACK:
			{
				showeffects=showeffects==true?false:true;
				FormEnable(1,1,showeffects);
				mainsettings->showdrumeffects=showeffects;
			}
			break;

		case GADGETID_EDITORSLIDER_VERT: // Track Scroll
			trackobjects.InitWithSlider(vertgadget,true);
			DrawDBBlit(tracks,eventsgfx);
			break;
			break;

		case GADGETID_EDITORSLIDER_VERTZOOM:
			ZoomGFX(gadget->GetPos());
			break;

		default:
			//	trackfx.Gadget(gadget);
			break;
	}
}

void Edit_Drum::BuildTrackList()
{
	if(!tracks)return;

	trackobjects.DeleteAllO(tracks);

	//if(drummap)
	{
		Drumtrack *t=WindowSong()->drummap.FirstTrack();

		while(t)
		{
			double h;

			if(t==focustrack)
				h=zoomybuffermul*zoomy;
			else
				h=zoomy;

			trackobjects.AddCooObject(t,h,0);

			t=t->NextTrack();
		}
	}

	trackobjects.EndBuild();
}

void Edit_Drum::ShowVSlider()
{
	if(vertgadget)
		vertgadget->ChangeSlider(&trackobjects,zoomy);
}


void Edit_Drum::ShowTracks()
{
	guiobjects.RemoveOsFromTo(OBJECTID_DRUMTRACK,LASTDRUMTRACKID);	

	if(!tracks)return;

	if(zoomvert==true)
		trackobjects.BufferYPos();

	BuildTrackList();

	if(zoomvert==true)
		trackobjects.RecalcYPos();

	ShowVSlider();
	trackobjects.InitYStartO();

	tracks->ClearTab();

	int sizewchildopen=tracks->gbitmap.GetTextWidth("MM");

	TRACE ("Tracks Init Y %d\n",trackobjects.GetInitY());

	if(trackobjects.GetShowObject()) // first track ?
	{
		//guiBitmap *automationtrackbitmap=maingui->gfx.FindBitMap(IMAGE_SUBTRACK_CLOSE);

		int sizemininame=tracks->gbitmap.GetTextWidth("ABCDEF");
		int startnumberw=tracks->gbitmap.GetTextWidth("1234.");
		int w=tracks->gbitmap.GetTextWidth(" mR ");
		int wrt=tracks->gbitmap.GetTextWidth("AA Rec:Audio");

		// Create Track List
		while(trackobjects.GetShowObject() && trackobjects.GetInitY()<tracks->GetHeight())
		{
			Drumtrack *t=(Drumtrack *)trackobjects.GetShowObject()->object;

			if(Edit_Drum_Track *et=new Edit_Drum_Track)
			{
				int startx=startnumberw;

				et->trackselected=t->IsSelected();
				et->startnamex=startx;
				et->editor=this;
				et->bitmap=&tracks->gbitmap;

				et->track=t;

				// Zoom Y2
				et->dataoutputy2=trackobjects.GetInitY2();

				guiobjects.AddTABGUIObject(0,trackobjects.GetInitY(),tracks->GetWidth(),trackobjects.GetInitY2(),tracks,et);

				int xx=startx+1;
				int hxstart=xx;

				int sy1=trackobjects.GetInitY()+1;
				int sy2=sy1+maingui->GetButtonSizeY();

				// Mute ------------------------------------
				guiobjects.AddGUIObject(
					xx,
					sy1,
					xx+w,
					sy2,
					tracks,
					&et->mute
					);

				xx+=w+1;

				// Solo
				guiobjects.AddGUIObject(
					xx,
					sy1,
					xx+w,
					sy2,
					tracks,
					&et->solo
					);

				xx+=w+1;



				/*
				tracks->formchild->SetGXY(startx+1,trackobjects.GetInitY()+1+17);

				if(guiGadget_Volume *g=glist.AddVolumeButton(-1,-1,w,-1,0,et->track->io.audioeffects.volume.value,MODE_PARENT))
				{
				//guiobjects.AddGUIObject(g,tracks,new Edit_AudioMix_SendEdit(this,as,g,as->sendvolume));
				//sendvolumegadgets.AddEndO(g);
				}
				*/

				int namexs=xx+w+2;
				int datax=tracks->GetX2();

				if(namexs+sizemininame<datax)
				{
					et->dataoutputx=datax-21;
					et->dataoutputx2=datax-1;
					et->datadisplay=true;
				}
				else
				{
					et->dataoutputx=datax;
					et->datadisplay=false;
				}

				int endx2=et->datadisplay==true?et->dataoutputx-3:datax-2;

				int vy=sy2+ADDYSPACE+1;
				int vy2=vy+3+maingui->GetButtonSizeY();


#ifdef OLDIE
				{
					// MIDI ?
					if(vy2<et->y2 && endx2>hxstart+ADDXSPACE)
					{
						// Audio Input
						int vx=hxstart;

						if(GetShowFlag()&SHOW_IO)
						{
							int midx=(endx2-hxstart)-ADDXSPACE;

							midx/=2;

							// Volume Input
							guiobjects.AddGUIObject(
								vx,
								vy,
								vx+midx-1,
								vy2,
								tracks,
								&et->MIDIinput
								);

							vx+=midx+ADDXSPACE+1;
						}

						// MIDI Volume
						guiobjects.AddGUIObject(
							vx,
							vy,
							endx2,
							vy2,
							tracks,
							&et->MIDIvolume
							);

						vy2+=ADDYSPACE+1;
						vy=vy2;
						vy2=vy+3+maingui->GetButtonSizeY();

					}
				}
#endif

				// Name
				guiobjects.AddGUIObject(
					namexs,
					sy1,
					endx2,
					sy2,
					tracks,
					&et->name
					);

				// 

				// Type

				//int ty2=trackobjects.GetInitY()+2*maingui->GetFontSizeY()+2;


				//et->ShowTrackRaster();
				//	if(showtracknumber==true)et->ShowTrackNumber();

			} // if et

			trackobjects.NextYO();

		}// while list

		guiObject_Pref *o=tracks->FirstGUIObjectPref();
		while(o)
		{
			Edit_Drum_Track *et=(Edit_Drum_Track *)o->gobject;
			et->ShowTrack();

			o=o->NextGUIObjectPref();
		}


	}// if t

	trackobjects.DrawUnUsed(tracks);
#ifdef OLDIE
	if(y!=-1)
	{
		frame_tracks.y=y;
	}

	y=frame_tracks.y;

	frame_tracks.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,0);

	if(frame_tracks.ondisplay==true)
	{
		int numberoftracks=0;
		int y2;

		guiobjects.RemoveOs(0); // tracks

		Drumtrack *t=starttrack;

		if(!drummap)
			t=0;
		else
		{
			if(!t)
				starttrack=t=drummap->FirstTrack();
		}

		lasttrack=starttrack;

		frame_tracks.Fill3D(guibuffer,frame_tracks.bpen);

		while(t && y<frame_tracks.y2)
		{
			if(Edit_Drum_Track *et=new class Edit_Drum_Track)
			{
				et->namex2=frame_tracks.x2-12;
				et->dataoutputx=et->namex2+1;
				et->dataoutputx2=frame_tracks.x2-1;

				et->editor=this;
				et->bitmap=guibuffer;

				et->track=t;

				// Active Track

				if(t==focustrack)
				{
					y2=y+(3*zoomy);
					guibuffer->guiFillRect3D(frame_tracks.x,y,frame_tracks.x2,y2+1,COLOUR_GREY);
				}
				else
				{
					y2=y+zoomy;
					guibuffer->guiDrawLine(frame_tracks.x,y2,frame_tracks.x2,y2,COLOUR_BLACK);
				}

				et->virtualy=y;
				et->virtualy2=y2;

				lasttrack=t;

				if(y2<=frame_tracks.y2)
					numberoftracks++; // full display

				guiobjects.AddGUIObject(frame_tracks.x,y,frame_tracks.x2,y2,&frame_tracks,et);

				int hy=y+19;

				if(hy<y2)
				{
					// Mute
					{
						Edit_Drum_Mute *mute=new Edit_Drum_Mute;

						if(et->muteobject=mute)
						{
							mute->track=et;

							guiobjects.AddGUIObject(
								frame_tracks.x+1,
								y+1,
								frame_tracks.x+1+16,
								y+1+16,
								&frame_tracks,
								mute
								);
						}
					}		

					// Solo
					{
						Edit_Drum_Solo *solo=new Edit_Drum_Solo;

						if(et->soloobject=solo)
						{
							solo->track=et;

							guiobjects.AddGUIObject(
								frame_tracks.x+1+16,
								y+1,
								frame_tracks.x+1+16+16,
								y+1+16,
								&frame_tracks,
								solo
								);
						}
					}
				}

				hy+=13;

				if(hy<y2)

					// Volume
				{
					Edit_Drum_Volume *vol=new Edit_Drum_Volume;

					if(et->volumeobject=vol)
					{
						vol->track=et;

						guiobjects.AddGUIObject(
							frame_tracks.x+1,
							hy-13,
							frame_tracks.x2,
							hy,
							&frame_tracks,
							vol
							);
					}
				}

				y=y2+1;

			}// if et

			t=t->NextTrack();
		}// while t

		// Show Track Data
		guiObject *o=guiobjects.FirstObject();
		Edit_Drum_Track *et;

		while(o)
		{
			if(o->id==OBJECTID_DRUMTRACK)
			{
				et=(Edit_Drum_Track *)o;

				// Mute / Record
				et->ShowMute();
				et->ShowSolo();

				et->ShowName();
				et->ShowVolume();
			}

			o=o->NextObject();
		}

		// Show Track Slider
		if(vertgadget && starttrack)
		{
			vertgadget->ChangeSlider(0,drummap->GetCountOfTracks()-numberoftracks,drummap->GetIndexOfTrack(starttrack));
		}

		if(!(winmode&WINDOWMODE_FRAMES))
			BltGUIBuffer_Frame(&frame_tracks);
	}

#endif

}

int Edit_Drum_Track::GetTrackValue(int iy)
{
	double per,h;

	per=y2-y;

	h=y2-iy;
	h/=per;
	h*=127;

	return (int)h;
}

void Edit_Drum_Track::ShowGFXRaster(guiBitmap *bitmap)
{
	int bgcolour;

	if(track->IsSelected()==true || track==editor->focustrack)
		bgcolour=COLOUR_BACKGROUNDEDITOR_GFX_HIGHLITE;
	else
		bgcolour=COLOUR_BACKGROUNDEDITOR_GFX;

	bitmap->guiFillRectX0(y,bitmap->GetX2(),y2,bgcolour);

	// Track Lines

	if(track==editor->focustrack)
	{
		bitmap->guiDrawLineYX0(y,bitmap->GetX2(),track->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT);
		bitmap->guiDrawLineYX0(y2,bitmap->GetX2());
	}
	else
	{
		bitmap->guiDrawLineYX0(y,bitmap->GetX2(),COLOUR_RASTER1);
		bitmap->guiDrawLineYX0(y2,bitmap->GetX2(),COLOUR_RASTER2);
	}
}

void Edit_Drum_Track::ShowEvents(guiBitmap *bitmap)
{
	Seq_SelectionEvent *selevent=editor->patternselection.FirstMixEvent(editor->startposition);

	while(selevent && selevent->seqevent->GetEventStart()<editor->endposition)
	{	
		if(selevent->seqevent->GetICD()==ICD_TYPE_DRUM)
		{
			ICD_Drum *drum=(ICD_Drum *)selevent->seqevent;

			if(drum->drumtrack==track)
			{
				if(Edit_Drum_Drum *ndrum=new Edit_Drum_Drum(editor,selevent,drum,bitmap))
				{
					editor->drums.AddEndO(ndrum);

					selevent->flag|=SEQSEL_ONDISPLAY;

					ndrum->x=editor->timeline->ConvertTimeToX(drum->GetEventStart());
					ndrum->x2=editor->timeline->ConvertTimeToX(drum->GetEventStart()+editor->WindowSong()->timetrack.zoomticks);

					if(ndrum->x2==-1)
						ndrum->x2=bitmap->GetX2();

					ndrum->y=y;
					ndrum->y2=y2;

					ndrum->Draw();
				}
			}
		}

		selevent=selevent->NextEvent();
	} // while event
}

void Edit_Drum_Track::ShowTrackRaster()
{
	trackselected=track->IsSelected();

	int bgcolour;

	if(track->IsSelected()==true)
		bgcolour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
	else
		bgcolour=COLOUR_BACKGROUND;

	// Colour
	int col,col2;

	if(track==editor->focustrack)
	{
		col=col2=track->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT;
	}
	else
	{
		col=COLOUR_WHITE;
		col2=COLOUR_GREY_DARK;
	}


	bitmap->guiFillRectX0(y,startnamex,y2,bgcolour);

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->SetTextColour(COLOUR_BLACK);

	int number=track->map->GetIndexOfTrack(track)+1;

	bitmap->guiDrawNumber(1,y+maingui->GetButtonSizeY(),startnamex-1,number);

	bitmap->guiFillRect(startnamex,y,x2,y2,bgcolour);

	bitmap->guiDrawLineX(0,y,y2,col);
	bitmap->guiDrawLineX(startnamex-1,y,y2,col);

	bitmap->guiDrawLineYX0(y,x2,col);
	bitmap->guiDrawLineYX0(y2,x2,col2);
}

void Edit_Drum_Track::ShowTrack()
{
	ShowTrackRaster();
	ShowName();
}

void Edit_Drum::ShowEvents()
{
	patternselection.ClearFlags();
	drums.DeleteAllO();

	if(!tracks)
		return;

	if(!eventsgfx)
		return;

	if(!timeline)
		return;

	eventsgfx->gbitmap.guiFillRect(COLOUR_UNUSED);

	patternselection.CopyStatus();

	guiObject_Pref *o=tracks->FirstGUIObjectPref();
	while(o)
	{
		Edit_Drum_Track *et=(Edit_Drum_Track *)o->gobject;

		et->ShowGFXRaster(&eventsgfx->gbitmap);
		et->ShowEvents(&eventsgfx->gbitmap);

		o=o->NextGUIObjectPref();
	}

	timeline->DrawPositionRaster(&eventsgfx->gbitmap);

	if(mousemode==EM_MOVEOS)
	{
		TRACE ("Drum MOVEOS ... \n");

		if(ICD_Drum *n=(ICD_Drum *)patternselection.FirstSelectionEvent(0,SEL_SELECTED))
		{
			if(patternselection.movediff<-n->GetEventStart()) // FirstPosition
				patternselection.movediff=-n->GetEventStart();

			// Find Lowest/Highest Key		
			{
				int lowest,highest;

				highest=lowest=n->drumtrack->index;

				while(n)
				{
					if(n->drumtrack->index<lowest)
						lowest=n->drumtrack->index;

					if(n->drumtrack->index>highest)
						highest=n->drumtrack->index;

					n=(ICD_Drum *)patternselection.NextSelectionEvent(SEL_SELECTED);
				}

				if(patternselection.moveobjects_vert<0)
				{
					if(lowest+patternselection.moveobjects_vert<0)
						patternselection.moveobjects_vert=-lowest;
				}
				else
				{
					if(highest+patternselection.moveobjects_vert>127)
						patternselection.moveobjects_vert=127-highest;
				}
			}

			ICD_Drum movedrum;
			Edit_Drum_Drum edrum(this,0,&movedrum,&eventsgfx->gbitmap);

			n=(ICD_Drum *)patternselection.FirstSelectionEvent(0,SEL_SELECTED);

			while(n)
			{
				TRACE ("ICD m \n");

				OSTART start=n->GetEventStart()+patternselection.movediff;
				Edit_Drum_Track *edt_to=FindTrack(WindowSong()->drummap.GetTrackWithIndex(n->drumtrack->index+patternselection.moveobjects_vert));

				if(edt_to)
				{
					// Insert Quantize ?
					{
						QuantizeEffect *qeff=n->pattern->GetQuantizer();

						if(qeff)
							start=qeff->Quantize(start);
					}

					if(start>=startposition && start<=endposition)
					{
						movedrum.flag=EVENTFLAG_MOVEEVENT;
						movedrum.pattern=n->pattern;
						movedrum.ostart=start;
						movedrum.drumtrack=edt_to->track;
						movedrum.velocity=n->velocity;
						movedrum.velocityoff=n->velocityoff;

						edrum.x=timeline->ConvertTimeToX(start);
						edrum.x2=timeline->ConvertTimeToX(start+WindowSong()->timetrack.zoomticks);
						edrum.y=edt_to->y;
						edrum.y2=edt_to->y2;

						edrum.Draw();

						TRACE ("Move Drum %d %d\n",patternselection.movediff,patternselection.moveobjects_vert);
					}
				}

				n=(ICD_Drum *)patternselection.NextSelectionEvent(SEL_SELECTED);
			}// while n
		}
	}
}

void Edit_Drum::ShowDrumsHoriz(int flag)
{
#ifdef OLDIE
	int y=-1;

	if(guibuffer)
	{
		if((flag&SHOWEVENTS_HEADER) && WindowSong())
			ShowEditorHeader(0);

		if(guictrlbox.stopbutton && (!timeline))
			y=guictrlbox.stopbutton->y2+4+maingui->GetFontSizeY(); // + Header

		if(flag&SHOWEVENTS_TRACKS)
			ShowDrumTracks(y);

		if((flag&SHOWEVENTS_EVENTS) && WindowSong())
			ShowEvents(y);
	}
#endif
}

void Edit_Drum::DeInitWindow()
{	
	drums.DeleteAllO();
	EventEditor_Selection::DeInitWindow();
}

bool Edit_Drum::EditCancel()
{
	switch(mousemode)
	{
	case EM_MOVEOS:
		SetEditorMode(EM_RESET);
		DrawDBBlit(eventsgfx);
		return true;
		break;

	case EM_SELECTOBJECTS:
		maingui->ResetPRepairSelection(&patternselection);
		ResetMouseMode();
		return true;
		break;
	}

	return false;
}

Edit_Drum_Track *Edit_Drum::FindDrumTrackAtYPosition(int y)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_DRUMTRACK)
		{	
			if(o->y<=y && o->y2>=y)
				return (Edit_Drum_Track *)o;
		}

		o=o->NextObject();
	}

	return 0;
}


bool Edit_Drum::CreateDrum()
{
	Edit_Drum_Track *t=FindDrumTrackAtYPosition(eventsgfx->GetMouseY());

	if(t && insertpattern && insertpattern->mediatype==MEDIATYPE_MIDI)
	{
		OSTART pos=GetMousePosition();
		bool create=true;

		// Find Drum Event in insertpattern
		Seq_Event *check=insertpattern->FindEventAtPosition(pos,SEL_INTERN,ICD_TYPE_DRUM);

		while(check && check->GetEventStart()<=pos && create==true)
		{
			if(check->GetEventStart()==pos &&
				(((ICD_Object_Seq_MIDIChainEvent *)check)->type==ICD_TYPE_DRUM)
				)
			{
				ICD_Drum *cdrum=(ICD_Drum *)check;

				if(cdrum->drumtrack==t->track) // same drumtrack == dont create
				{
					return false;
				}
			}

			check=check->NextEvent();
		}

		if(create==true)
		{
			if(ICD_Drum *drum=new ICD_Drum)
			{
				drum->ostart=drum->staticostart=pos;
				drum->drumtrack=t->track;

				UBYTE velocity=t->GetTrackValue(eventsgfx->GetMouseY());

				if(!velocity)velocity=1;

				drum->velocity=velocity;
				drum->velocityoff=0;

				editevent.song=WindowSong();
				editevent.pattern=(MIDIPattern *)insertpattern;
				editevent.seqevent=drum;
				editevent.position=pos;

				editevent.doundo=true;
				editevent.addtolastundo=addtolastundo;
				editevent.playit=true;

				addtolastundo=true;

				mainedit->CreateNewMIDIEvent(&editevent);

				return true;
			}
		}
	}

	addtolastundo=true;
	return false;
}

void Edit_Drum::DeleteDrum()
{
	if(ICD_Drum *d=FindDrumUnderMouse())
		mainedit->DeleteEvent(d,addtolastundo);
}

void Edit_Drum::CreateNewDrumTrack(Drumtrack *clone)
{
		Drumtrack *newt=0;

		if(clone)
		{
			newt=WindowSong()->drummap.CreateDrumTrack(WindowSong()->drummap.GetIndexOfTrack(clone));

			if(newt)
				clone->CloneData(newt);
		}
		else
			newt=WindowSong()->drummap.CreateDrumTrack(0);

		if(newt)
			WindowSong()->drummap.RefreshAllEditors();
}

void Edit_Drum::DeleteDrumTrack(Drumtrack *track)
{
	if(track)
	{
		bool deleteok=true;

		if(FindDrumEventsInsideMap(WindowSong(),track)==true)
			deleteok=maingui->MessageBoxYesNo(this,"Drumtrack has Drum-Events, Delete Drumtrack (and Events) ?");

		if(deleteok==true)
		{
			track->map->RemoveDrumtrackFromSongs(track);
		}
	}
}

void Edit_Drum::PlayFocusTrack()
{
	Seq_Track *t=0;

	if(!insertpattern)
	{
		t=WindowSong()->GetFocusTrack();
	}
	else
		t=insertpattern->track;

	// Play Track Sound
	if(focustrack && t && ((!insertpattern) || insertpattern->mediatype==MEDIATYPE_MIDI))
	{
		ICD_Drum adrum;

		adrum.ostart=WindowSong()->GetSongPosition();
		adrum.drumtrack=focustrack;
		adrum.velocity=127;
		adrum.pattern=insertpattern;

		t->SendOutEvent_User((MIDIPattern *)insertpattern,&adrum,true);
	}
}

void Edit_Drum::NewActiveTrack(Drumtrack *track)
{
	if(focustrack!=track)
	{
		focustrack=track;
		ShowDrumsHoriz(SHOWEVENTS_TRACKS|SHOWEVENTS_EVENTS);
		//trackfx.ShowActiveTrack();
		SongNameRefresh();
	}
}

ICD_Drum *Edit_Drum::FindDrumUnderCursor()
{
	if(cursor.drumtrack)
	{
		Seq_SelectionEvent *selevent=patternselection.FirstMixEvent(cursor.ostart);
		OSTART end=WindowSong()->timetrack.ConvertTicksToBeatTicks(cursor.ostart,true);

		while(selevent && selevent->seqevent->GetEventStart()<=end)
		{
			Seq_Event *check=selevent->seqevent;

			if(
				(((ICD_Object_Seq_MIDIChainEvent *)check)->type==ICD_TYPE_DRUM)
				)
			{
				ICD_Drum *cdrum=(ICD_Drum *)check;

				if(cdrum->drumtrack==cursor.drumtrack) // same drumtrack == dont create
					return cdrum;
			}

			selevent=selevent->NextEvent();
		}
	}

	return 0;
}

ICD_Drum *Edit_Drum::FindDrumUnderMouse(int flag)
{
	if(!eventsgfx)
		return 0;

	int x=eventsgfx->GetMouseX(),y=eventsgfx->GetMouseY();

	Edit_Drum_Drum *found=0,*pn=(Edit_Drum_Drum *)drums.GetRoot();

	while(pn){

		if(pn->x<=x && pn->x2>=x && pn->y<y && pn->y2>=y)
		{
			if((pn->drum->flag&flag)==0)
				found=pn;		
		}
		else
			pn->drum->flag CLEARBIT flag;

		pn=(Edit_Drum_Drum *)pn->next;
	}

	if(found)
	{
		found->drum->flag|=flag;
		return found->drum;
	}

	return 0;
}

void Edit_Drum::ClearCursor()
{
	//	ClearSprite(&cursorsprite);
	//	RemoveSprite(&cursorsprite);
}

void Edit_Drum::SetCursorToMousePosition()
{
#ifdef OLDIE
	if(frame_drums.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
	{
		Edit_Drum_Track *etrack=FindDrumTrackAtYPosition(GetMouseY());

		if(etrack)
		{
			if(cursor.drumtrack!=etrack->track || cursor.ostart!=GetMousePosition())
			{
				cursor.ostart=GetMousePosition();
				cursor.drumtrack=etrack->track;

				ShowCursor();
			}
		}
	}
#endif

}

void Edit_Drum::PlayCursor()
{
	if(cursor.drumtrack && insertpattern && insertpattern->mediatype==MEDIATYPE_MIDI)
	{
		ICD_Drum adrum;

		adrum.drumtrack=cursor.drumtrack;
		adrum.velocity=127;
		adrum.pattern=insertpattern;

		insertpattern->track->SendOutEvent_User((MIDIPattern *)insertpattern,&adrum,true);
	}
}

void Edit_Drum::ShowCursor()
{
#ifdef OLDIE
	ClearCursor();

	if(timeline && frame_drums.ondisplay==true)
	{
		if(!cursor.drumtrack)
		{
			Edit_Drum_Track *t=FirstTrack();

			if(t)
				cursor.drumtrack=t->track;
		}

		Edit_Drum_Track *ft;

		if(cursor.ostart>=startposition && cursor.ostart<endposition &&
			(ft=FindTrack(cursor.drumtrack))
			)
		{
			//	MessageBeep(-1);
			OSTART end=WindowSong()->timetrack.ConvertTicksToBeatTicks(cursor.ostart,true);

			cursorsprite.x=timeline->ConvertTimeToX(cursor.ostart,frame_drums.x2);

			if(timeline->CheckIfInHeader(end)==true)
				cursorsprite.x2=timeline->ConvertTimeToX(end,frame_drums.x2);
			else
				cursorsprite.x2=frame_drums.x2;

			cursorsprite.y=ft->y;
			cursorsprite.y2=ft->y2;
			cursorsprite.colour=COLOUR_RED;
			cursorsprite.type=guiSprite::SPRITETYPE_RECT;

			AddSprite(&cursorsprite);
			ShowAllSprites();
		}
	}
#endif

}

void Edit_Drum::ChangeToDrummap(Drummap *d)
{
#ifdef OLDIE
	if(drummap!=d) 
	{
		drummap=d;

		if(d)
			starttrack=focustrack=d->FirstTrack();
		else
			starttrack=focustrack=0;

		ShowDrumsHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_TRACKS);

		//trackfx.ShowActiveTrack();
	}
#endif

}


void Edit_Drum::SetName(Drumtrack *track,char *newname)
{
	track->SetName(newname);

	guiWindow *win=maingui->FirstWindow();

	while(win)
	{
		if(win->WindowSong()==WindowSong())
			switch(win->GetEditorID())
		{
			/*
			case EDITORTYPE_DRUMMAP:
			{
			Edit_Drummap *map=(Edit_Drummap *)win;

			if(map->activedrummap==track->map)
			map->ShowDrumTracks();
			}
			break;
			*/

			case EDITORTYPE_DRUM:
				{
					Edit_Drum *drum=(Edit_Drum *)win;

					//if(drum!=this)
					//	drum->trackfx.ShowActiveTrack();

					//drum->ShowDrumTracks();
				}
				break;

		}

		win=win->NextWindow();
	}
}

void Edit_Drum::Paste()
{
	PasteMouse(eventsgfx);
}

void Edit_Drum::ShowDrumMapName()
{
	if(drummap)
	{
		char *h=mainvar->GenerateString("Drum Map:",WindowSong()->drummap.GetName());

		if(h)
		{
			drummap->ChangeButtonText(h);
			delete h;
		}
	}
}

void Edit_Drum::Init()
{	
	focustrack=WindowSong()->drummap.FirstTrack();

	patternselection.BuildEventList(SEL_INTERN,0,ICD_TYPE_DRUM); // Mix new List, events maybe moved/deleted
	InitGadgets();
}

void Edit_Drum::SetFocusTrack(Drumtrack *track)
{
	focustrack=track;
	DrawDBBlit(tracks,eventsgfx);
}

void Edit_Drum::MouseClickInTracks(bool leftmouse)
{
	guiObject *o=tracks->CheckObjectClicked(); // Object Under Mouse ?

	/*
	if(o && maingui->GetShiftKey()==true)
	{
	switch(o->id)
	{
	case OBJECTID_ARRANGESUBPATTERN:
	case OBJECTID_ARRANGEPATTERN:
	o=0;
	break;
	}
	}
	*/

	if(!o)
	{
		//editmode=ED_TRACKS;
		//SelectAll(false);
		return;
	}

	if(leftmouse==true)
	{
		//Left Mouse
		switch(o->id)
		{
		case OBJECTID_DRUMTRACKNAME:
			{
				Edit_Drum_Name *edt=(Edit_Drum_Name *)o;

				if(focustrack==edt->track->track)
				{
					PlayFocusTrack();
				}
				else
				{
					SetFocusTrack(edt->track->track);
					PlayFocusTrack();
				}
			}
			break;

		case OBJECTID_DRUMTRACK:
			{
				Edit_Drum_Track *edt=(Edit_Drum_Track *)o;
				SetFocusTrack(edt->track);
				PlayFocusTrack();
			}
			break;
		}
		return;
	}

	// Right Mouse
}

void Edit_Drum::SetMouseMode(int newmode,OSTART mp,Drumtrack *track)
{
	if(mp==-1)
	{
		InitMousePosition();
		modestartposition=GetMousePosition(); // X
	}
	else
		modestartposition=mp;

	if(modestartposition>=0)
	{
		if(track==0)
		{
			Edit_Drum_Track *edt=FindDrumTrackAtYPosition(eventsgfx->GetMouseY()); // Y
			modestarttrack=edt?edt->track:0;
		}
		else
			modestarttrack=track;

		if(modestarttrack)
		{
			SetEditorMode(newmode);
			SetAutoScroll(newmode,eventsgfx);
		}
	}
}

void Edit_Drum::DeleteDrums()
{
	TRACE ("DeleteDrums Undo = %d\n",addtolastundo);

	mainedit->DeleteEvents(&patternselection,addtolastundo);
	addtolastundo=true;
}

void Edit_Drum::FindAndDeleteDrums(ICD_Drum *drum)
{
	if(eventsgfx)
	{
		if(drum)
			drum->Select();

		DeleteDrums();
	}
}

void Edit_Drum::MouseClickInDrums(bool leftmouse)
{
	if(leftmouse==false)
	{
		if(EditCancel()==true)
			return;
	}
	else
	{
		if(CheckMouseClickInEditArea(eventsgfx)==true) // Left Mouse
		{
			return;
		}
	}

	ICD_Drum *e=FindDrumUnderMouse();
	InitMousePosition();

	switch(mousemode)
	{
	case EM_EDIT:
	case EM_SELECT:
	case EM_CUT:
	case EM_DELETE:
		{
			if(leftmouse==true)
			{
				if((!e) && maingui->GetCtrlKey()==false)
					patternselection.SelectAllEvents(false);

				if((!e) || maingui->GetShiftKey()==true)
				{
					SetMouseMode(EM_SELECTOBJECTS,-1);
					return;
				}
			}

			if((!e) && mousemode!=EM_DELETE)
				return;
		}
		break;
	}

	// Drum clicked
	switch(mousemode)
	{
	case EM_EDIT:
		{
			if(leftmouse==true && e)
			{
				WindowSong()->SetFocusEvent(e);
				patternselection.SelectEvent(e,true);
				RefreshEvents();

				eventsgfx->SetEditSteps(1);
				eventsgfx->SetStartMouseY();

				OpenEditSelectedEvents(editvelocity==true?TAB_BYTE1:TAB_BYTE2,e);
			}
		}
		break;

	case EM_CREATE:
		{
			if(leftmouse==true)
			{
				CreateDrum();
			}
			else
			{
				DeleteDrum();
			}
		}
		break;

	case EM_DELETE:
		if(leftmouse==true)
		{
			addtolastundo=false;
			FindAndDeleteDrums(e);
		}
		break;

	default:
		if(leftmouse==true && e)
		{
			WindowSong()->SetFocusEvent(e);
			patternselection.SelectEvent(e,true);
			RefreshEvents();

			//SetEditorMode(EM_SELECTOBJECTSSTART);
			mainhelpthread->AddMessage(MOUSEBUTTONDOWNMS,this,MESSAGE_CHECKMOUSEBUTTON,MESSAGE_DRUM,e);
		}
		break;
	}
}

void Edit_Drum::MouseDoubleClickInDrums(bool leftmouse)
{
	int *te=0;

	if( (!te) || maingui->GetShiftKey()==true)
	{
		DoubleClickInEditArea();
		return;
	}
}


void Edit_Drum::InitMouseEditRange()
{
	int mx=eventsgfx->GetMouseX();
	int my=eventsgfx->GetMouseY();

	mstrack=modestarttrack;

	Edit_Drum_Track *edt=FindDrumTrackAtYPosition(my);
	metrack=edt?edt->track:0;

	if(metrack==0)
	{
		if(my<0)metrack=WindowSong()->drummap.FirstTrack();
		else metrack=WindowSong()->drummap.LastTrack();
	}

	if(!metrack)
		return;

	msposition=modestartposition;
	msendposition=mx<eventsgfx->GetX2()?timeline->ConvertXPosToTime(mx):endposition;

	if(msposition>msendposition){
		OSTART h=msendposition;
		msendposition=msposition;
		msposition=h;
	}

	if(mstrack->map->GetIndexOfTrack(mstrack)>metrack->map->GetIndexOfTrack(metrack)){
		Drumtrack *h=metrack;
		metrack=mstrack;
		mstrack=h;
	}

	int sx=timeline->ConvertTimeToX(msposition);
	int sx2=timeline->ConvertTimeToX(msendposition);

	Edit_Drum_Track *eds=FindTrack(mstrack);

	int sy=0;

	if(eds && eds->y>0)
		sy=eds->y;

	Edit_Drum_Track *ede=FindTrack(metrack);

	int sy2=eventsgfx->GetY2();

	if(ede && ede->y2<sy2)
		sy2=ede->y2;

	mouseeditx=sx==-1?0:sx;
	mouseeditx2=sx2==-1?eventsgfx->GetX2():sx2;
	mouseedity=sy;
	mouseedity2=sy2;
}

void Edit_Drum::MouseMoveInDrums(bool leftmouse)
{
	if(CheckMouseMovePosition(eventsgfx)==true)
		return;

	switch(mousemode)
	{
	case EM_EDIT: // Velocity
		if(leftmouse==true)
		{
			if(eventsgfx->InitDeltaY())
			{
				EditSelectedEventsDelta_Tab(editvelocity==true?TAB_BYTE1:TAB_BYTE2);
			}
		}
		break;

	case EM_CREATE:
		{
			if(leftmouse==true)
			{
				CreateDrum();
			}
			else
				if(maingui->GetRightMouseButton()==true)
				{
					DeleteDrum();
				}
		}
		break;

	case EM_DELETE:
		{
			if(maingui->GetRightMouseButton()==true || leftmouse==true)
				DeleteDrum();
		}
		break;

	case EM_SELECTOBJECTS:
		{
			ShowCycleAndPositions(editarea);
			DrawDBSpriteBlit(editarea);

			maingui->OpenPRepairSelection(&patternselection);

			Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

			while(selevent )
			{	
				if(selevent->seqevent->GetICD()==ICD_TYPE_DRUM)
				{
					ICD_Drum *drum=(ICD_Drum *)selevent->seqevent;

					if(mainvar->CheckIfInPosition(msposition,msendposition,drum->GetEventStart())==true && WindowSong()->drummap.CheckIfDrumEvent(mstrack,metrack,drum)==true)
						drum->PRepairSelection();
				}

				selevent=selevent->NextEvent();
			}

			maingui->ClosePRepairSelection(&patternselection);
		}
		break;

	case EM_MOVEOS:
		{
			ShowMoveDrumsSprites();
		}
		break;
	}
}

void Edit_Drum::MouseReleaseInDrums(bool leftmouse)
{
	switch(mousemode)
	{
	case EM_EDIT:
		{
			ReleaseEdit();
		}
		break;

	case EM_CREATE:
		{
			ResetMouseMode(EM_RESET); // default mode
			return;
		}
		break;

	case EM_SELECTOBJECTS:
		{	
			SelectAllEvents();
		}
		break;

	case EM_MOVEOS:
		{
			EditCancel();

			MoveO mo;

			mo.song=WindowSong();
			mo.sellist=&patternselection;
			mo.diff=patternselection.movediff;
			mo.index=patternselection.moveobjects_vert;
			mo.flag=GetMouseQuantizeFlag();
			mo.filter=SEL_INTERN;
			mo.icdtype=ICD_TYPE_DRUM;

			if(maingui->GetCtrlKey()==true)
				mainedit->CopySelectedEventsInPatternList(&mo);
			else
				mainedit->MoveSelectedEventsInPatternList(&mo);
		}
		break;
	}

	eventsgfx->EndEdit();
	ResetMouseMode();
}

void DrumEditor_EventGFX_Callback(guiGadget_CW *g,int status)
{
	Edit_Drum *e=(Edit_Drum *)g->from;

	switch(status)
	{
	case DB_CREATE:
		e->eventsgfx=g;
		//Init Raster
		break;

	case DB_PAINT:
		{
			e->ShowEvents();
			e->ShowCycleAndPositions(g);
		}
		break;

	case DB_PAINTSPRITE:
		e->ShowCycleAndPositions(e->eventsgfx);
		break;

	case DB_LEFTMOUSEDOWN:
		e->MouseClickInDrums(true);	
		break;

	case DB_RIGHTMOUSEDOWN:
		e->MouseClickInDrums(false);
		break;

	case DB_LEFTMOUSEUP:
		e->MouseReleaseInDrums(true);
		break;

	case DB_DOUBLECLICKLEFT:
		e->MouseDoubleClickInDrums(true);
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_MOUSEMOVE:
		e->MouseMoveInDrums(false);
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		e->MouseMoveInDrums(true);
		break;
	}
}

void Edit_Drum::NewYPosition(double y,bool draw)
{
	if(trackobjects.InitWithPercent(y)==true)
		DrawDBBlit(tracks,draw==true?eventsgfx:0);
}

void Edit_Drum::ShowOverviewVertPosition(int *y,int *y2)
{
	*y=0;
	*y2=overview->GetY2();

	if(trackobjects.height==0)
		return;

	if(!tracks)
		return;

	double h=trackobjects.height;
	double h2=trackobjects.starty;
	double height=overview->GetY2();

	h2/=h;
	h2*=height;

	*y=(int)h2;

	h2=trackobjects.starty+trackobjects.guiheight;
	h2/=h;
	h2*=height;

	*y2=(int)h2;
}

void Edit_Drum::ShowOverview()
{
	if(!overview)return;

	guiBitmap *bitmap=&overview->gbitmap;
	bitmap->guiFillRect(COLOUR_OVERVIEW_BACKGROUND);

	OSTART slen=overviewlenght=WindowSong()->GetSongLength_Ticks();

	if(!slen)return;

	double hx=(double)overview->GetX2(),hy=(double)overview->GetY2();

	//	bitmap->SetAPen(COLOUR_OVERVIEWOBJECT);

	int trackcount=WindowSong()->drummap.GetCountOfTracks();

	if(trackcount && hy>8)
	{
		double h2=trackcount;
		double py=(hy-2);

		py/=h2;

		int ipy=(int)py;
		if(ipy<1)ipy=1;

		//Draw Notes
		Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

		// NoteOns ---------------------------------------------------------
		while(selevent && selevent->GetOStart()<=slen)
		{	
			if(selevent->seqevent->GetICD()==ICD_TYPE_DRUM)
			{
				ICD_Drum *drum=(ICD_Drum *)selevent->seqevent;

				// x+x2
				double x=(double)drum->GetEventStart(),x2;
				x/=slen;
				x*=hx;

				OSTART e=drum->GetEventStart()+drum->drumtrack->ticklength;

				if(e>slen)
					x2=overview->GetX2();
				else
				{
					x2=(double)e;
					x2/=slen;
					x2*=hx;
				}

				double y=drum->drumtrack->map->GetIndexOfTrack(drum->drumtrack);
				y*=py;

				y+=1;

				int ix=(int)x;
				int ix2=(int)x2;
				int iy=(int)y;

				if(ix2<ix+2)
					ix2=ix+2;

				bitmap->guiDrawLineY(iy,ix,ix2,COLOUR_OVERVIEWOBJECT);
			}

			selevent=selevent->NextEvent();

		}//while event
	}
}

void DrumEditor_Overview_Callback(guiGadget_CW *g,int status)
{
	Edit_Drum *p=(Edit_Drum *)g->from;

	switch(status)
	{
	case DB_CREATE:
		p->overview=g;
		break;

	case DB_PAINT:
		{
			p->ShowOverview();
			p->ShowOverviewCycleAndPositions();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		p->MouseClickInOverview(true);	
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		p->MouseClickInOverview(false);	
		break;
	}
}

void DrumEditor_Tracks_Callback(guiGadget_CW *g,int status)
{
	Edit_Drum *e=(Edit_Drum *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			e->tracks=(guiGadget_Tab *)g;
			//e->OnInit();
		}break;

	case DB_NEWSIZE:
		//e->InitShowEvents();
		break;

	case DB_PAINT:
		{
			e->ShowTracks();
		}
		break;

		//case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		e->MouseClickInTracks(true);
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

void Edit_Drum::InitGadgets()
{
	int addw=INFOSIZE;
	int iw=bitmap.GetTextWidth("WWWw");

	glist.SelectForm(0,0);
	guitoolbox.CreateToolBox(TOOLBOXTYPE_EDITOR);
	glist.Return();

	InitSelectionPatternGadget();

	glist.AddButton(-1,-1,addw,-1,"TList",GADGETID_LIST,showlist==true?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED|MODE_AUTOTOGGLE:MODE_GROUP|MODE_TOGGLE|MODE_AUTOTOGGLE,"Track List+Groups");
	glist.AddLX();

	glist.AddButton(-1,-1,addw,-1,"Track",GADGETID_TRACK,showeffects==true?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED|MODE_AUTOTOGGLE:MODE_GROUP|MODE_TOGGLE|MODE_AUTOTOGGLE);
	glist.AddLX();

	drummap=glist.AddButton(-1,-1,5*iw,-1,"",GADGETID_DRUMMAP,MODE_MENU,"Drum Maps");
	glist.AddLX();
	ShowDrumMapName();

	glist.AddButton(-1,-1,iw,-1,"S",GADGETID_EDITNOTE_TIME_I,MODE_TEXTCENTER|MODE_ADDDPOINT);
	glist.AddLX();

	infonote_time=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGETID_EDITNOTE_TIME,windowtimeformat,MODE_INFO,"Position (Focus Note)");
	glist.AddLX();

	FormEnable(0,1,showlist);
	FormEnable(1,1,showeffects);

	int offsettracksy=SIZEV_OVERVIEW+SIZEV_HEADER+2*(ADDYSPACE+1);

	SliderCo horz,vert;

	horz.formx=0;
	horz.formy=2;

	vert.formx=4;
	vert.formy=1;
	vert.offsety=offsettracksy;
	vert.from=0;
	vert.to=0; // trackobjects.GetCount()-numberoftracks;
	vert.pos=0; //firstshowtracknr;

	AddEditorSlider(&horz,&vert);

	glist.SelectForm(0,1);
	glist.form->BindWindow(new Edit_DrumList(this));

	glist.SelectForm(1,1);
	glist.form->BindWindow(new Edit_DrumFX(this));

	glist.SelectForm(2,1);
	glist.AddTab(-1,offsettracksy,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,DrumEditor_Tracks_Callback,this); // Event List

	glist.SelectForm(3,1);
	glist.AddChildWindow(-1,-1,-1,SIZEV_OVERVIEW,MODE_RIGHT|MODE_SPRITE,0,&DrumEditor_Overview_Callback,this);
	glist.Return();

	glist.AddChildWindow(-1,-1,-1,SIZEV_HEADER,MODE_RIGHT|MODE_SPRITE,0,&Editor_Header_Callback,this);
	editarea=glist.AddChildWindow(-1,offsettracksy,-1,-2,MODE_RIGHT|MODE_BOTTOM|MODE_SPRITE,0,DrumEditor_EventGFX_Callback,this); // GFX
}

void Edit_Drum::AddStartY(int addy)
{
	if(trackobjects.AddStartY(addy)==true)
	{
		DrawDBBlit(tracks,eventsgfx);
	}
}

void Edit_Drum::AutoScroll()
{
	DoAutoScroll();
	DoStandardYScroll(eventsgfx);
}

void Edit_Drum::RefreshRealtime_Slow()
{
	RefreshEventEditorRealtime_Slow();
	RefreshSelectionGadget();

	/*
	int h_getcountselectedevents=patternselection.GetCountofSelectedEvents(),
	h_getcountevents=patternselection.GetCountOfEvents();

	if(h_getcountselectedevents!=getcountselectedevents ||
	h_getcountevents!=getcountevents)
	SongNameRefresh();
	*/
}

void Edit_Drum::RefreshEvents()
{
	patternselection.ClearFlags();
	//	ShowRaster();

	//noteraster->DrawGadgetBlt();
	patternselection.CopyStatus();

	Edit_Drum_Drum *drd=(Edit_Drum_Drum *)drums.GetRoot();

	while(drd)
	{
		drd->Draw();
		drd=(Edit_Drum_Drum *)drd->next;
	}

	DrawDBSpriteBlit(eventsgfx);
}

void Edit_Drum::RefreshRealtime()
{
	if(RefreshEventEditorRealtime()==true)	
	{
		if(eventsgfx)
		{
			ShowCycleAndPositions(eventsgfx);
			//	if(refreshnotes==false)
			eventsgfx->DrawSpriteBlt();
		}
	}

	bool refreshdrums=false;

	Edit_Drum_Drum *pn=(Edit_Drum_Drum *)drums.GetRoot();

	while(pn && refreshdrums==false){

		if(pn->eflag!=pn->sevent->seqevent->flag)  // Selection On/Off etc
			refreshdrums=true;

		if(pn->sevent->seqevent==WindowSong()->GetFocusEvent() && pn->infodrum==false)
			refreshdrums=true;

		pn=(Edit_Drum_Drum *)pn->next;
	}

	if(RefreshEventEditorRealtime()==true)
	{
		if(eventsgfx)
		{
			ShowCycleAndPositions(eventsgfx);
			if(refreshdrums==false)
				eventsgfx->DrawSpriteBlt();
		}
	}

	if(refreshdrums==true)
		RefreshEvents();

	return;

#ifdef OLDIE
	trackfx.RefreshRealtime();

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

	// Check Tracks
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_DRUMTRACK)
		{	
			Edit_Drum_Track *et=(Edit_Drum_Track *)o;

			if(et->muteobject)
			{
				if(et->muteobject->mutestatus!=et->track->mute)
				{
					et->muteobject->mutestatus=et->track->mute;
					et->ShowMute();
					BltGUIBuffer(et->muteobject);
				}
			}

			if(et->soloobject)
			{
				if(et->soloobject->solostatus!=et->track->solo)
				{
					et->soloobject->solostatus=et->track->solo;
					et->ShowSolo();
					BltGUIBuffer(et->soloobject);
				}
			}

			if(et->volumeobject)
			{
				if(et->volumeobject->volume!=et->track->volume)
				{
					et->volumeobject->volume=et->track->volume;
					et->ShowVolume();
					BltGUIBuffer(et->volumeobject);
				}
			}

		}	

		o=o->NextObject();
	}

	// Clock
	if(timeline && frame_drums.ondisplay==true)
		ShowClockLine(WindowSong()->GetSongPosition(),timeline->y,frame_drums.y2);

	RefreshToolTip();
#endif

}

Edit_Drum_Track *Edit_Drum::FindTrack(Drumtrack *f)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_DRUMTRACK){
			Edit_Drum_Track *c=(Edit_Drum_Track *)o;
			if(c->track==f)return c;
		}

		o=o->NextObject();
	}

	return 0;
}

Edit_Drum_Track *Edit_Drum::FirstTrack()
{
	guiObject *o=guiobjects.FirstObject();

	while(o){
		if(o->id==OBJECTID_DRUMTRACK)return (Edit_Drum_Track *)o;
		o=o->NextObject();
	}

	return 0;
}

void Edit_Drum::RefreshObjects(LONGLONG type,bool editcall)
{
	if(editcall==true)
	{
		if(((refreshflag&REFRESHEVENT_LIST)==0) && patternselection.CheckEventList()==true)
		{
			refreshflag|=REFRESHEVENT_LIST;
		}
	}

	if(patternselection.patternremoved==true)
	{
		ShowSelectionPattern();
	}

	DrawHeader(); // Tempo Changes etc..
	ShowAllEvents(refreshflag&REFRESHEVENT_LIST?0:NOBUILD_REFRESH);

	//ShowOverview();
	DrawDBBlit(eventsgfx,overview);
	patternselection.patternremoved=false; // reset
}

#define CAMX_DEFAULTDRUMMAPDIRECTORY "\\DataBase\\DrumMaps"

bool Edit_Drum::ZoomGFX(int z,bool horiz)
{
	if(SetZoomGFX(z,horiz)==true)
	{
		DrawDBBlit(tracks,eventsgfx);
		return true;
	}

	return false;
}

void Edit_Drum::RenameDrumMap(Drummap *map,int px,int py)
{
	if(map)
	{
		if(EditData *edit=new EditData)
		{
			edit->win=this;
			edit->x=px;
			edit->y=py;
			edit->title="Song Drum Map Name";
			edit->id=EDIT_DRUMNAME;
			edit->type=EditData::EDITDATA_TYPE_STRING;
			edit->string=map->GetName();

			maingui->EditDataValue(edit);
		}
	}
}

void Edit_Drum::SelectDrumMap(Drummap *map)
{
	focustrack=WindowSong()->drummap.FirstTrack();

	ShowDrumsHoriz(SHOWEVENTS_TRACKS|SHOWEVENTS_EVENTS);

	SongNameRefresh();
	//trackfx.ShowActiveTrack();
}

Edit_Drum::Edit_Drum()
{
	editorid=EDITORTYPE_DRUM;
	editorname="Drum";

	InitForms(FORM_HORZ4x1BAR_SLIDERHV3);
	EditForm(0,1,CHILD_HASWINDOW);
	EditForm(1,1,CHILD_HASWINDOW);

	resizeable=true;
	minwidth=minheight=maingui->GetButtonSizeY(8);

	SetMinZoomY(maingui->GetButtonSizeY()+4);

	focustrack=0;

	// FX
	headerflag=0;

	patternselection.buildfilter=SEL_INTERN;

	cursor.ostart=0;
	cursor.drumtrack=0;

	followsongposition=mainsettings->followeditor;
	showlist=mainsettings->showdrumlist;
	showeffects=mainsettings->showdrumeffects;

	eventsgfx=0;
	editvelocity=true;

	drumlist=0;
}

char *Edit_Drum::GetToolTipString1() //v
{
	char *string=0;

#ifdef OLDIE
	ICD_Drum *drum=FindDrumUnderMouse();

	if(drum) // Note clicked
	{
		char h2[16];
		string=mainvar->GenerateString("Drum Velo:",mainvar->ConvertIntToChar(drum->velocity,h2));
	}
	else
	{
		Edit_Drum_Track *et=FindDrumTrackAtYPosition(GetMouseY());

		if(et)
		{
			int h=et->GetTrackValue(GetMouseY());
			char h2[16];
			string=mainvar->GenerateString("Velo:",mainvar->ConvertIntToChar(h,h2));
		}
	}
#endif

	return string;
}

char *Edit_Drum::GetToolTipString2() //v
{
	char *string=0;
	ICD_Drum *drum=FindDrumUnderMouse();

	if(drum) // Note clicked
		string=mainvar->GenerateString("P:",drum->pattern->GetName());

	return string;
}

void Edit_Drum::ShowMoveDrumsSprites()
{
	int my=eventsgfx->GetMouseY();
	Edit_Drum_Track *edt=FindDrumTrackAtYPosition(my);

	InitMousePosition();

	if(GetMousePosition()>=0 && edt)
	{
		Drumtrack *tracknow=edt->track;

		patternselection.moveobjects_vert=CanMoveY()==true?tracknow->index-modestarttrack->index:0;
		patternselection.movediff=CanMoveX()==true?GetMousePosition()-modestartposition:0;

		eventsgfx->DrawGadgetBlt();
	}
}

void Edit_Drum::UserMessage(int msg,void *par)
{
	switch(msg)
	{
	case MESSAGE_CHECKMOUSEBUTTON:
		{
			if(maingui->GetShiftKey()==false && par)
			{
				ICD_Drum *drum=(ICD_Drum *)par;

				if(maingui->GetLeftMouseButton()==true)
				{	
					Edit_Drum_Drum *epn=(Edit_Drum_Drum *)drums.GetRoot();

					while(epn)
					{
						if(epn->drum==drum)
						{
							SetMouseMode(EM_MOVEOS,-1);
							ShowMoveDrumsSprites();
							return;
						}

						epn=(Edit_Drum_Drum *)epn->next;
					}

					return;
				}

				patternselection.SelectAllEventsNot(drum,false,0);
			}
		}
		break;
	}
}

void mainMIDIBase::CollectDrumsMaps()
{
	return;

	for(int i=0;i<2;i++)
	{
		camxFile file;

		if(char *h=mainvar->GenerateString(i==0?mainvar->GetCamXDirectory():mainvar->GetUserDirectory(),CAMX_DEFAULTDRUMMAPDIRECTORY))
		{
			file.BuildDirectoryList(h,"*.*",".cdmp");
			delete h;

			camxScan *scan=file.FirstScan();

			while(scan)
			{
				bool ok=false;
				camxFile read;

				if(read.OpenRead(scan->name)==true && read.CheckVersion()==true)
				{
					read.LoadChunk();

					if(read.GetChunkHeader()==CHUNK_SETTINGSDRUMMAP)
					{
						read.ChunkFound();

						char text[4];

						read.ReadChunk(text,4);

						if(text[0]=='C' &&
							text[1]=='A' &&
							text[2]=='D' &&
							text[3]=='M')
						{
							ok=true;
						}

						read.CloseReadChunk();

						if(ok==true)
						{
							read.LoadChunk();

							if(read.GetChunkHeader()==CHUNK_DRUMMAP)
							{
								read.ChunkFound();

								if(Drummap *newmap=new Drummap)
								{
									//newmap->filename=mainvar->GenerateString(scan->name);

									newmap->Load(&read);

									//if(!defaultdrummap)
									//	defaultdrummap=newmap;

									drummaps.AddEndO(newmap);
								}
							}
						}
					}
				}

				read.Close(true);

				scan=scan->NextScan();
			}

			file.ClearScan();
			file.Close(true);
		}

		file.Close(true);
	}
}


/*


void mainDrumMap::SaveDrumMap(Drummap *dm)
{
camxFile file;

if(char *h=mainvar->GenerateString(
mainvar->GetCamXDirectory(),
CAMX_DEFAULTDRUMMAPDIRECTORY,
"\\DM_",
dm->GetName(),
".cdmp")
)
{
if(file.OpenSave_CheckVersion(h)==true)
{
file.SaveVersion();

file.OpenChunk(CHUNK_SETTINGSDRUMMAP);
file.Save_Chunk("CADM",4);
file.CloseChunk();

dm->Save(&file);

if((!dm->filename) || strcmp(dm->filename,h)!=0)
{
if(dm->filename)
delete dm->filename;

dm->filename=mainvar->GenerateString(h);
}
}

file.Close(true);

delete h;
}

file.Close(true);
}

void mainDrumMap::InitDefaultDrumMap()
{
//Drummap *map=AddDrummap();

//	if(map)
//		map->InitGMDrumMap();
}

void mainDrumMap::RemoveAllDrummaps()
{
Drummap *g=FirstDrummap();

while(g)
{
maindrummap->SaveDrumMap(g); // SAve
g=RemoveDrummap(g);
}
}
*/


/*
void mainDrumMap::AddDrummap(Drummap *map,bool save)
{
if(!defaultdrummap)
defaultdrummap=map;

drummaps.AddEndO(map);

if(save==true)
SaveDrumMap(map);
}

Drummap *mainDrumMap::AddDrummap(bool refreshgui)
{
Drummap *ndm=new Drummap;

if(ndm)
{
if(!defaultdrummap)
defaultdrummap=ndm;

drummaps.AddEndO(ndm);
SaveDrumMap(ndm);
}

if(refreshgui==true)
{
}

return ndm;
}

void mainDrumMap::RemoveDrumMapFromGUI(Drummap *d)
{
guiWindow *f=maingui->FirstWindow(),*nf;

while(f)
{	
nf=f->NextWindow();

switch(f->GetEditorID())
{
case EDITORTYPE_DRUM:
{
Edit_Drum *drumed=(Edit_Drum *)f;

if(drumed->drummap==d)
f->CloseWindow();
}
break;
}

f=nf;
}
}

Drummap* mainDrumMap::RemoveDrummap(Drummap *d)
{
if(defaultdrummap==d)
defaultdrummap=(Drummap *)d->NextOrPrev();

d->DeleteAllDrumTracks();

if(d->filename)
delete d->filename;

return (Drummap *)drummaps.RemoveO(d);
}
*/