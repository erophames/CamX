#include "songmain.h"
#include "editor_marker.h"
#include "gui.h"
#include "camxgadgets.h"
#include "guigadgets.h"
#include "seqtime.h"
#include "object_song.h"
#include "editfunctions.h"
#include "settings.h"
#include "languagefiles.h"
#include "undofunctions_marker.h"
#include "editdata.h"
#include "audiohardware.h"

enum{
	TAB_TIME,
	TAB_END, // 120.xxx
	TAB_MARKER, // xxx.000

	TAB_TABS
};

#define TEXT_POSITIONX_STD 1
#define TEXT_POSITIONX_MIN TEXT_POSITIONX_STD+35
#define TEXT_POSITIONX_SEC TEXT_POSITIONX_STD+60
#define TEXT_POSITIONX_TK TEXT_POSITIONX_STD+90
#define TEXT_POSITIONX_QF TEXT_POSITIONX_STD+120

#define NOTEXTEDIT 0

// Measure Edit
#define TEXTEDIT_1000 10
#define TEXTEDIT_0100 11
#define TEXTEDIT_0010 12
#define TEXTEDIT_0001 13

// Time Edit H-min-sec-frames-s-frames
#define TEXTEDIT_HOUR 20
#define TEXTEDIT_MIN 21
#define TEXTEDIT_SEC 22
#define TEXTEDIT_FRAME 23
#define TEXTEDIT_QFRAME 24

// Messages
enum MEditorIDs
{
	EDITID_TEXT=EventEditor::EDITOREDITDATA_ID,
	EDIT_MARKERNAME
};

bool Edit_Marker::Scroll(int diff,bool refreshdisplay)
{
	/*
	if(diff && FirstText())
	{
	bool scroll=false;
	long nrTEXTs=patternselection();

	if(diff<0)
	{
	Edit_Marker_Text *fe=FirstText();
	long c=firsttext_index+diff;

	if(c>=0)
	{
	firsttext_index=c;
	scroll=true;
	}
	}
	else
	{
	long nrTEXTs=patternselection.GetCountOfTEXTs();
	long ondisplay=firsttext_index+numberofdisplaytexts;

	if(ondisplay+diff<=nrTEXTs)
	{
	firsttext_index+=diff;
	scroll=true;
	}
	}

	if(scroll==true && refreshdisplay==true)
	{
	ShowTEXTs();
	}

	return scroll;
	}
	*/

	return false;
}

void Edit_Marker::SelectAndEdit(bool right)
{
	#ifdef OLDIE
	bool dontselect=false;

	editdata=NOTEXTEDIT;

	Edit_Marker_Text *e=FindText(GetMouseX(),GetMouseY());	

	if(maingui->GetCtrlKey()==false)
	{
		// Positions
		if(frame.position.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
		{
			if(e)
			{
				if(position_1000_x<=GetMouseX() && position_0100_x>GetMouseX())
				{
					switch(windowdisplay)
					{
					case WINDOWDISPLAY_MEASURE:
						editdata=TEXTEDIT_1000;
						break;

					case WINDOWDISPLAY_SMPTE:
						editdata=TEXTEDIT_HOUR;
						break;
					}
				}
				else
					if(position_0100_x<=GetMouseX() && position_0010_x>GetMouseX())
					{
						switch(windowdisplay)
						{
						case WINDOWDISPLAY_MEASURE:
							editdata=TEXTEDIT_0100;
							break;

						case WINDOWDISPLAY_SMPTE:
							editdata=TEXTEDIT_MIN;
							break;
						}
					}
					else
						if(position_0010_x<=GetMouseX() && position_0001_x>GetMouseX())
						{
							switch(windowdisplay)
							{
							case WINDOWDISPLAY_MEASURE:
								editdata=TEXTEDIT_0010;
								break;

							case WINDOWDISPLAY_SMPTE:
								editdata=TEXTEDIT_SEC;
								break;
							}
						}
						else
							if(position_0001_x<=GetMouseX() && position_0001QF_x>GetMouseX())
							{
								switch(windowdisplay)
								{
								case WINDOWDISPLAY_MEASURE:
									editdata=TEXTEDIT_0001;
									break;

								case WINDOWDISPLAY_SMPTE:
									editdata=TEXTEDIT_FRAME;
									break;
								}
							}
							else
								if(position_0001_x<=GetMouseX())
								{
									switch(windowdisplay)
									{
									case WINDOWDISPLAY_MEASURE:
										editdata=TEXTEDIT_0001;
										break;

									case WINDOWDISPLAY_SMPTE:
										editdata=TEXTEDIT_QFRAME;
										break;
									}
								}
			}
		}
		else
		{
		}
	}

	if(e && (dontselect==false))
	{
		/*
		if(maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false && editdata==NOTEXTEDIT)
		{
		patternselection.SelectAllTEXTs(false,SEL_ALLTEXTS);
		e=FindTEXT(GetMouseX(),GetMouseY()); // Refresh e
		}

		if(e)
		{
		selectmode=true;

		if(editdata==NOTEXTEDIT && e->text->flag&TEXTFLAG_SELECTED)
		selecttype=false;
		else
		selecttype=true;

		mainsettings->TEXTcreation.SetTEXT(e->TEXt->TEXT);

		if(maingui->GetShiftKey()==true)
		{
		if(selecttype==true)
		patternselection.SelectFromLastTEXTTo(e->TEXT,true);
		}
		else
		{
		patternselection.lastselectedTEXT=e->TEXt->TEXT;

		if( 
		(selecttype==true) ||
		(maingui->GetCtrlKey()==true)
		)
		{
		cursortext_index=patternselection.GetOfMixTEXT(e->TEXT);

		Seq_Text *TEXT=e->TEXt->TEXT;

		if(maingui->GetShiftKey()==false)
		patternselection.SelectAllTEXTs(false,SEL_ALLTEXTS,false); // no gui refresh

		patternselection.SelectTEXT(TEXT,true);

		KeepTEXTinMid(patternselection.FindTEXT(TEXT),false); // no scroll

		if(selecttype==true && patternselection.GetCountofSelectedTEXTs()==1)
		SendFirstSelectedTEXT();
		}
		}
		}
		*/
	}

	if(editdata!=NOTEXTEDIT)
	{
		editstart_x=GetMouseX();
		editstart_y=GetMouseY();

		refreshmousebuttondown=true;
		refreshmousebuttonright=right;

		if(right==true)
			EditTexts(editdata,1);
		else
			EditTexts(editdata,-1);
	}
#endif

}

void Edit_Marker::CheckMouseButtonDown()
{
	/*
	if(editdata!=NOTEXTEDIT)
	{
		addtolastundo=true;

		if(refreshmousebuttonright==true)
			EditTexts(editdata,1);
		else
			EditTexts(editdata,-1);
	}
	*/
}

bool Edit_Marker::KeepTextinMid(Seq_Marker *e,bool scroll)
{
	if(e)
	{
		int c=numberofdisplaytexts;
		int ce=cursor_index=e->GetIndex();

		cursortext=e;

		if(ce>=0)
		{
			c/=2;
			int h=ce-c;

			if(h>=0 && scroll==true)
				firsttext_index=h;

			cursortext=e;

			//ShowTexts();

			ShowCursorText();

			return true;
		}
#ifdef _DEBUG
		else
		{
			if(ce<0)
				MessageBox(NULL,"Illegal Keep in Mid","Error",MB_OK);
		}
#endif

	}

	return false;
}

void Edit_Marker::KeyDown()
{
	Editor_KeyDown();
}

void Edit_Marker::ShowCursorText()
{
	/*
	if(infotext && cursortext)
	{
	Seq_Text *TEXT=cursortext->text;

	Seq_Pattern *p=TEXt->GetPattern();
	Seq_Track *t=TEXt->GetTrack();

	if(p && t)
	{
	long l=strlen(p->GetName())+strlen(t->name);

	l+=16;

	char *h=new char[l];

	if(h)
	{
	strcpy(h,t->name);
	mainvar->AddString(h," / ");
	mainvar->AddString(h,p->GetName());

	infotext->SetString(h);

	delete h;
	}
	}
	else
	infotext->SetString("TEXT Error");
	}
	*/
}

void Edit_Marker::KeyUp()
{

}

void Edit_Marker::EditText(Seq_Marker *t)
{
	if(t)
	{
		EditData *edit=new EditData;

		if(edit)
		{
			// long position;
			edit->song=WindowSong();
			edit->win=this;
			edit->x=GetWindowMouseX();
			edit->y=GetWindowMouseY();

			edit->title="Edit Marker";
			edit->deletename=false;

			edit->id=EDITID_TEXT;

			edit->type=EditData::EDITDATA_TYPE_STRING;

			edit->helpobject=t;
			edit->string=t->string;

			maingui->EditDataValue(edit);
		}
	}
}


void Edit_Marker::MouseWheel(int delta,guiGadget *db)
{
	if(Edit_MouseWheel(delta)==true)
	{
	}
}

void Edit_Marker::SelectAllMarker(bool on)
{
	Seq_Marker *t=WindowSong()->textandmarker.FirstMarker();

	while(t)
	{
		if(on==true)
			t->flag |=OFLAG_SELECTED;
		else
			t->flag CLEARBIT OFLAG_SELECTED;

		t=t->NextMarker();
	}
}

EditData *Edit_Marker::EditDataMessage(EditData *data)
{
	if(CheckStandardDataMessage(data)==true)
		return 0;

	switch(data->id)
	{
	case EDITID_TEXT:
		{
			Seq_Marker *m=(Seq_Marker *)data->helpobject;
			maingui->ChangeMarker(m,data->newstring,0,0);
		}
		break;

	case EDIT_MARKERNAME:
		{
			Seq_Marker *mk=(Seq_Marker *)data->helpobject;
			maingui->ChangeMarker(mk,data->newstring,0,0);
		}
		break;

	default:
		return data;
		break;
	}

	return 0;
}

// Buttons,Slider ...
void Edit_Marker::Gadget(guiGadget *gadget)
{	
	gadget=Editor_Gadget(gadget);

	if(gadget)
		switch(gadget->gadgetID)
	{		
		case GADGETID_EDITORSLIDER_VERT: // Track Scroll
			{
				markerobjects.InitWithSlider(vertgadget,true);
		DrawDBBlit(list);
			}
			break;

		case GADGETID_EDITORSLIDER_VERTZOOM:
			ZoomGFX(gadget->GetPos());
			break;
	}
}


void Edit_Marker::RefreshText(Seq_Marker *t)
{
	/*
	Edit_Marker_Text *f=FirstText();

	while(f)
	{
		if(f->marker==t)
		{
			f->Draw(true);
			break;
		}

		f=f->NextText();
	}
	*/
}

void Edit_Marker::RefreshRealtime()
{
	RefreshEventEditorRealtime();

	guiObject_Pref *o=list->FirstGUIObjectPref();
	while(o)
	{
		Edit_Marker_Text *et=(Edit_Marker_Text *)o->gobject;

		if(et->eflag!=et->marker->flag)
		{
			DrawDBBlit(list);
			return;
		}

		o=o->NextGUIObjectPref();
	}
}

void Edit_Marker::RefreshObjects(LONGLONG type,bool editcall)
{
	DrawDBBlit(list);
}

void Edit_Marker::ShowMenu()
{
	ShowEditorMenu();
}


void Edit_Marker::Goto(int to)
{
	UserEdit();

	if(CheckStandardGoto(to)==true)
		return;

	switch(to)
	{
	case GOTO_FIRST:
		{
			Seq_Marker *txt=WindowSong()->textandmarker.FirstMarker();

			if(txt)
			{
				if(NewStartPosition(txt->GetMarkerStart(),true)==true)
					SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_LAST:
		{
			Seq_Marker *txt=WindowSong()->textandmarker.LastMarker();

			if(txt)
			{
				if(NewStartPosition(txt->GetMarkerStart(),true)==true)
					SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_FIRSTSELECTED:
		{
			Seq_Marker *t=WindowSong()->textandmarker.FirstMarker();

			while(t)
			{
				if(t->flag&OFLAG_SELECTED)
				{
					if(NewStartPosition(t->GetMarkerStart(),true)==true)
						SyncWithOtherEditors();
					return;
				}

				t=t->NextMarker();
			}
		}
		break;

	case GOTO_LASTSELECTED:
		{
			Seq_Marker *t=WindowSong()->textandmarker.LastMarker();

			while(t)
			{
				if(t->flag&OFLAG_SELECTED)
				{
					if(NewStartPosition(t->GetMarkerStart(),true)==true)
						SyncWithOtherEditors();
					return;
				}

				t=t->PrevMarker();
			}
		}
		break;
	}
}

void Edit_Marker::DeleteMarkers()
{
	mainedit->DeleteSelectedMarkers(WindowSong(),addtolastundo);
	addtolastundo=true;
}

guiMenu *Edit_Marker::CreateMenu()
{
#ifdef OLDIE
//	ResetUndoMenu();

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		// Standard Editor Menu
		// Piano Editor Menu
		editmenu=menu->AddMenu(Cxs[CXS_EDIT],0);
		if(editmenu)
		{
			maingui->AddUndoMenu(editmenu);

			class menu_selectmarker:public guiMenu{
			public:
				menu_selectmarker(Edit_Marker *e,bool o){editor=e;on=o;}
				void MenuFunction(){editor->SelectAllMarker(on);}
				Edit_Marker *editor;
				bool on;
			};
			editmenu->AddFMenu(Cxs[CXS_SELECTALLMARKEREVENTS],new menu_selectmarker(this,true));
			editmenu->AddFMenu(Cxs[CXS_UNSELECTALLMARKEREVENTS],new menu_selectmarker(this,false));

			class menu_deletemarker:public guiMenu
			{
			public:
				menu_deletemarker(Edit_Marker *e){editor=e;}
				void MenuFunction(){editor->DeleteMarkers();}
				Edit_Marker *editor;
			};
			editmenu->AddFMenu(Cxs[CXS_DELETE],new menu_deletemarker(this),SK_DEL);

			AddStandardGotoMenu();
		}

		guiMenu *n=menu->AddMenu("Editor",0);
		if(n)
		{	
			CreateEditorMenu(n);
			AddStepMenu(n);
		}
	}

	maingui->AddCascadeMenu(this,menu);
#endif

	return menu;
}

bool Edit_Marker::ZoomGFX(int z,bool horiz)
{
	if(SetZoomGFX(z,horiz)==true)
	{
		DrawDBBlit(list);

		return true;
	}

	return false;
}


char *Edit_Marker::GetWindowName()
{
	char h[64],h2[32];
	strcpy(h,"-M:");
	mainvar->AddString(h,mainvar->ConvertIntToChar(getcountselectedmarker=WindowSong()->textandmarker.GetCountOfSelectedMarker(),h2));
	mainvar->AddString(h,"/");
	mainvar->AddString(h,mainvar->ConvertIntToChar(getcountmarker=WindowSong()->textandmarker.GetCountOfMarker(),h2));

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

void Edit_Marker::RefreshRealtime_Slow()
{

	/*
	int h_getcountselectedmarker=WindowSong()->textandmarker.GetCountOfSelectedMarker(),
		h_getcountmarker=WindowSong()->textandmarker.GetCountOfMarker();

	if(h_getcountselectedmarker!=getcountselectedmarker ||
		h_getcountmarker!=getcountmarker)
		SongNameRefresh();
		*/

	CheckNumberObjects(list);

}

Edit_Marker::Edit_Marker()
{
	editorname="Marker";
	editorid=EDITORTYPE_MARKER;

	InitForms(FORM_HORZ1x2SLIDERVTOOLBAR);

	minwidth=maingui->GetButtonSizeY(24);
	minheight=maingui->GetButtonSizeY(8);
	
	resizeable=true;
	ondesktop=true;

	firsttext_index=cursortext_index=0;
	zoomy=20;

	editdata=NOEVENTEDIT;

	filtergadget=0;
	selectmode=false;

	numberofdisplaytexts=0;

	movetexts=false;
	lengthchanged=false;

	cursor=CURSOR_POS_1000;
	cursortext=0;

	followsongposition=mainsettings->followeditor;
}

void MarkerEditor_List_Callback(guiGadget_CW *g,int status)
{
	Edit_Marker *tl=(Edit_Marker *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			g->menuindex=0;
			tl->list=(guiGadget_TabStartPosition *)g;
			tl->InitTabs();			
		}
		break;

	case DB_PAINT:
		{
			tl->ShowList();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		if(tl->list->InitDeltaY())
		{
		//	tl->DeltaY(tl->list);
		}
		break;

	case DB_LEFTMOUSEDOWN:
		//tl->MouseClickInTempos(true);	
		break;

	case DB_LEFTMOUSEUP:
		//tl->MouseReleaseInTempos(true);
		break;

	case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInTracks(false);	
		break;

	case DB_DOUBLECLICKLEFT:
		//ar->MouseDoubleClickInTracks(true);
		break;

	case DB_DELTA:
		//ar->DeltaInTracks();
		break;
	}
}

void Edit_Marker::BuildMarkerList()
{
	if(!list)return;

	markerobjects.DeleteAllO(list);

	int h=maingui->GetFontSizeY()+2;

	Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();

	while(m)
	{
		markerobjects.AddCooObject(m,h,0);
		m=m->NextMarker();
	}

	markerobjects.EndBuild();
}

void Edit_Marker::ShowVSlider()
{
	if(vertgadget)
		vertgadget->ChangeSlider(&markerobjects,maingui->GetFontSizeY());
}

void Edit_Marker::ShowList()
{
	guiobjects.RemoveOs(0);	
	if(!list)return;

	BuildMarkerList();
	ShowVSlider();
	markerobjects.InitYStartO();
	list->ClearTab();

	if(markerobjects.GetShowObject()) // first track ?
	{
		// Create Track List
		while(markerobjects.GetShowObject() && markerobjects.GetInitY()<list->GetHeight())
		{
			Seq_Marker *sm=(Seq_Marker *)markerobjects.GetShowObject()->object;

			if(Edit_Marker_Text *et=new Edit_Marker_Text)
			{
				et->song=WindowSong();
				et->index=sm->GetIndex();
				editobject=et->marker=sm;
				et->editor=this;
				et->bitmap=&list->gbitmap;
				
				et->timemode=ConvertWindowDisplayToTimeMode();

				guiobjects.AddTABGUIObject(0,markerobjects.GetInitY(),list->GetWidth(),markerobjects.GetInitY2(),list,et);
			}

			markerobjects.NextYO();

		}// while list

		guiObject_Pref *o=list->FirstGUIObjectPref();
		while(o)
		{
			Edit_Marker_Text *et=(Edit_Marker_Text *)o->gobject;
			
			et->Draw();
			AddNumberOList(et,list); // Init Time

			o=o->NextGUIObjectPref();
		}

	}// if t

	markerobjects.DrawUnUsed(list);
}

void Edit_Marker::InitGadgets()
{
	glist.SelectForm(0,0);
	guitoolbox.CreateToolBox(TOOLBOXTYPE_MARKER);

	glist.Return();

	SliderCo vert;

	vert.formx=1;
	vert.formy=1;
	vert.nozoom=true;

	AddEditorSlider(0,&vert);

	glist.SelectForm(0,1);
	glist.AddTabStartPosition(-1,-1,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,&MarkerEditor_List_Callback,this);
}


void Edit_Marker_Text::Draw(bool single)
{
	eflag=marker->flag;
	object=marker;

	OSTART pos=marker->GetMarkerStart();

	if(marker->IsSelected()==true)
	{
		pos+=editor->numbereditposition_sum; // List Editor
		pos+=editor->numbereditposition_sum; // Tempo Editor
	}

	bitmap->guiFillRect(editor->list->GetTabX(TAB_TIME),y,x2,y2,GetIndexColour());
	DrawPosition(pos,editor->list->GetTabX(TAB_TIME),editor->list->GetTabX2(TAB_TIME),marker);

	bitmap->guiDrawText(editor->list->GetTabX(TAB_MARKER),y2,editor->list->GetTabX2(TAB_MARKER),marker->string);
	set=true;
}

void Edit_Marker::InitTabs()
{
	list->InitTabs(TAB_TABS);
	
	list->InitTabWidth(TAB_TIME,list->GetDefaultTimeWidth()); // TIME
	list->InitTabWidth(TAB_END,list->GetDefaultTimeWidth()); // END

	list->InitStartPosition(TAB_TIME);
	list->InitXX2(); // X<>X2
}

void Edit_Marker::Init()
{
	InitGadgets();
}


Edit_Marker_Text::Edit_Marker_Text()
	{
		set=false;
		grey=false;
	}

void Edit_Marker::DeInitWindow()
{
	guiobjects.RemoveOs(0);
	markerobjects.DeleteAllO(0);
}

bool Edit_Marker::InitEdit(Seq_Event *seqevent)
{
	if(!seqevent)return false;

	if(!GetUndoEditEvents())
	{
		if((seqevent->flag&OFLAG_SELECTED)==0)
		{
			//if(maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false)
			//	SelectAllTempos(false);

			seqevent->flag |=OFLAG_SELECTED;
		}

		int c=0;
		Seq_Marker *t=WindowSong()->textandmarker.FirstMarker();

		while(t){
			if(t->flag&OFLAG_SELECTED)c++;
			t=t->NextMarker();
		}

		if(c)
		{
			TRACE ("Selected Txt %d\n",c);

			if(UndoEditMarker *uet=new UndoEditMarker)
			{
				// Buffer Tempo Events+Data
				if(uet->Init(c)==true)
				{
					int i=0;
					t=WindowSong()->textandmarker.FirstMarker();

					while(t){

						if(t->IsSelected()==true)
						{
							uet->event_p[i]=t;
							uet->oldindex[i]=t->GetIndex();
							uet->oldevents[i++]=(Seq_Event *)t->Clone(0);
						}

						t=t->NextMarker();
					}

					SetUndoEditEvents(uet);

					return true;
				}

				delete uet;
			}
		}
	}		

	return false;
}

UndoFunction *UndoEditMarker::CreateUndoFunction()
{
	return new Undo_EditMarker(this);
}



