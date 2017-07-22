#include "drumeditor.h"
#include "drumeditorlist.h"
#include "object_song.h"
#include "gui.h"
#include "songmain.h"

enum{
	TAB_COLOUR,
	TAB_MIDI,
	TAB_VU,
	TAB_CHILD,
	TAB_NAME
};

enum TlOB_ID{

	// Track
	OBJECTID_TRACKLIST=OI_LAST,
};

Edit_DrumList::Edit_DrumList(Edit_Drum *ar)
{
	editorid=EDITORTYPE_DRUMLIST;
	InitForms(FORM_HORZ2x1SLIDERV);

	//autovscroll=true;
	isstatic=true;

	song=ar->WindowSong();
	editor=ar;
}

void DrumEditor_List_Callback(guiGadget_CW *g,int status)
{
	Edit_DrumList *al=(Edit_DrumList *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			g->menuindex=0;


			al->list=(guiGadget_Tab *)g;
			al->list->InitTabs(5);

			al->editor->drumlist=al;

			al->list->InitTabWidth(TAB_COLOUR,g->gbitmap.GetTextWidth(">")); // Colour
			al->list->InitTabWidth(TAB_MIDI,g->gbitmap.GetTextWidth("#")); // MIDI
			al->list->InitTabWidth(TAB_VU,g->gbitmap.GetTextWidth(">>")); // VU
			al->list->InitTabWidth(TAB_CHILD,g->gbitmap.GetTextWidth(">>")); // Child
		}
		break;

	case DB_PAINT:
		{
			al->ShowList();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		//	ar->MouseMoveInTracks(true);
		break;

	case DB_LEFTMOUSEDOWN:
		al->MouseClickInTracks(true);	
		break;

	case DB_LEFTMOUSEUP:
		//ar->MouseReleaseInTracks(true);	
		break;

	case DB_RIGHTMOUSEDOWN:
		al->MouseClickInTracks(false);	
		break;

	case DB_DOUBLECLICKLEFT:
		//ar->MouseDoubleClickInTracks(true);
		break;

	case DB_DELTA:
		//ar->DeltaInTracks();
		break;
	}
}

void Edit_DrumList::Init()
{
	InitGadgets();
}

void Edit_DrumList::InitGadgets()
{
	SliderCo vert;

	vert.formx=1;
	vert.formy=0;
	vert.nozoom=true;

	AddEditorSlider(0,&vert);

	glist.SelectForm(0,0);

	glist.AddTab(-1,-1,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,&DrumEditor_List_Callback,this);
}

void Edit_DrumList::MouseClickInTracks(bool leftmouse)
{
	guiObject *o=list->CheckObjectClicked(); // Object Under Mouse ?

	if(o)
	{
		switch(o->id)
		{
		case OBJECTID_TRACKLIST:
			{
				Edit_DrumList_Track *eat=(Edit_DrumList_Track *)o;

				if(leftmouse==true)
				{
					int index=list->GetMouseClickTabIndex();

					switch(index)
					{
					case TAB_CHILD:
						{
							//eat->track->ToggleShowChild(leftmouse);
						}
						break;

					default:

						break;
					}
				}
				else
				{
					int index=list->GetMouseClickTabIndex();

					switch(index)
					{
					case TAB_CHILD:
						{
							//eat->track->ToggleShowChild(leftmouse);
						}
						break;

					default:
						//PopMenuTrack(eat->track);
						break;
					}


				}

			}
			break;
		}
	}
}

void Edit_DrumList::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_EDITORSLIDER_VERT: // Track Scroll
		trackobjects.InitWithSlider(vertgadget,true);
		DrawDBBlit(list);
		break;
	}
}


void Edit_DrumList::BuildTrackList()
{
	if(!list)return;

	trackobjects.DeleteAllO(list);

	//if(editor->WindowSong()-drummap)
	{
		Drumtrack *t=editor->WindowSong()->drummap.FirstTrack();
		while(t)
		{
			trackobjects.AddCooObject(t,maingui->GetFontSizeY(),0);

			t=t->NextTrack();
		}
	}

	trackobjects.EndBuild();

	//TRACE ("Build Track List Objects= %d W=%d H=%d\n",trackobjects.GetCount(),trackobjects.width,trackobjects.height);

	//	TRACE ("Track List %d\n",trackobjects.GetCount());
}

void Edit_DrumList::ShowVSlider()
{
	// Show Slider
	if(vertgadget)
		vertgadget->ChangeSlider(&trackobjects,maingui->GetFontSizeY());
	//vertgadget->ChangeSliderPage(numberoftracks);
}


Edit_DrumList_Track::Edit_DrumList_Track()
{
	id=OBJECTID_TRACKLIST;
	namestring=0;
	focus=false;
}

void Edit_DrumList_Track::ShowName()
{
	if(namestring)delete namestring;
	namestring=0;

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->SetTextColour(COLOUR_TEXTCONTROL);

	int bgcolour=COLOUR_GADGETBACKGROUND;

	bitmap->guiFillRect(x,y,x2,y2,bgcolour);

	namestring=mainvar->GenerateString(track->GetName());

	if(namestring)
		bitmap->guiDrawText(editor->list->GetTabX(TAB_NAME),y2,editor->list->GetTabX2(TAB_NAME),namestring);
}

void Edit_DrumList_Track::ShowTrack(bool refreshbgcolour)
{
	ShowName();
}

void Edit_DrumList::ShowList()
{
	guiobjects.RemoveOs(OBJECTID_TRACKLIST);	
	if(!list)return;

	list->InitXX2();
	BuildTrackList();
	ShowVSlider();
	trackobjects.InitYStartO();
	list->ClearTab();

	if(trackobjects.GetShowObject()) // first track ?
	{
		// Create Track List
		while(trackobjects.GetShowObject() && trackobjects.GetInitY()<list->GetHeight())
		{
			Drumtrack *t=(Drumtrack *)trackobjects.GetShowObject()->object;

			if(Edit_DrumList_Track *et=new Edit_DrumList_Track)
			{
				et->trackselected=t->IsSelected();
				et->startnamex=0;
				et->editor=this;
				et->bitmap=&list->gbitmap;
				et->track=t;

				guiobjects.AddTABGUIObject(0,trackobjects.GetInitY(),list->GetWidth(),trackobjects.GetInitY2(),list,et);

			} // if et

			trackobjects.NextYO();

		}// while list

		guiObject_Pref *o=list->FirstGUIObjectPref();
		while(o)
		{
			Edit_DrumList_Track *et=(Edit_DrumList_Track *)o->gobject;
			et->ShowTrack(false);
			o=o->NextGUIObjectPref();
		}

	}// if t

	trackobjects.DrawUnUsed(list);
}

void Edit_DrumList::FreeEditorMemory()
{
	guiobjects.RemoveOs(0);
	trackobjects.DeleteAllO(0);
}

void Edit_DrumList::DeInitWindow()
{	
	FreeEditorMemory();
}

void Edit_DrumList::RefreshObjects(LONGLONG type,bool editcall)
{
	DrawDBBlit(list);
}

void Edit_DrumList::RefreshRealtime()
{
	guiObject_Pref *o=list->FirstGUIObjectPref();
	while(o)
	{
		Edit_DrumList_Track *et=(Edit_DrumList_Track *)o->gobject;

		if(et->trackselected!=et->track->IsSelected())
		{
			DrawDBBlit(list);
			return;
		}

		/*
		bool isfocus=et->track==WindowSong()->GetFocusTrack()?true:false;

		if(et->focus!=isfocus)
		{
		DrawDBBlit(list);
		return;
		}

		et->ShowAudioDisplay(false);
		et->ShowMIDIDisplay(false);
		*/

		o=o->NextGUIObjectPref();
	}
}