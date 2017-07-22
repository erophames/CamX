#include "defines.h"
#include "tempomapeditor.h"
#include "object_song.h"
#include "camxgadgets.h"
#include "guiheader.h"
#include "mainhelpthread.h"
#include "gui.h"
#include "editfunctions.h"
#include "settings.h"
#include "languagefiles.h"
#include "semapores.h"
#include "songmain.h"
#include "undofunctions_tempo.h"
#include "editdata.h"
#include "chunks.h"
#include "tempomaplist.h"
#include "camxfile.h"
#include "MIDItimer.h"

#define TEMPOMENU_START MENU_ID_START+50

enum{
	MESSAGE_TEMPO
};

enum{
	GADGETID_LIST=GADGETID_EDITORBASE,
	GADGETID_RANGE,
	GADGETID_RESETVALUES,
	GADGETID_TAP,
	GADGETID_TAPVALUE,
	GADGETID_SETTAPVALUE
};

int temporange[]=
{
	240,
	480,
	960
};

enum
{
	EDIT_TEMPO=EventEditor::EDITOREDITDATA_ID
};

void Edit_Tempo_Tempo::Draw()
{
	char tempostring[16];
	int sizeh=maingui->GetButtonSizeY();

	x=initx;
	y=inity-sizeh;

	double tv=tempo->tempo;

	if(tempo->IsSelected()==true)
	{
		if(editor->tempolisteditor && editor->showlist==true && editor->tempolisteditor->list)
						tv+=editor->tempolisteditor->editsum;
	}

	char *text=mainvar->ConvertDoubleToChar(tv,tempostring,3);
	x2=x+bitmap->GetTextWidth(text)+4;
	y2=y+maingui->GetButtonSizeY();

	eflag=tempo->flag;

	if(tempo->IsSelected()==true)
	{
		bitmap->guiFillRect(x,y,x2,y2,COLOUR_BACKGROUNDEDITOR_GFX_HIGHLITE,COLOUR_WHITE);
		bitmap->SetTextColour(COLOUR_BLACK);
	}
	else
	{
		bitmap->guiFillRect(x,y,x2,y2,COLOUR_BACKGROUND_TEXT,COLOUR_BLACK);
		bitmap->SetTextColour(COLOUR_BLACK);
	}

	bitmap->guiDrawText(x+2,y2,x2-2,text);

	if(eflag&OFLAG_UNDERSELECTION)
		bitmap->guiInvert(x,y,x2,y2);
}

Edit_Tempo_Tempo *Edit_Tempo::FindTempoUnderMouse(int flag)
{
	if(!tempogfx)
		return 0;

	int x=tempogfx->GetMouseX(),y=tempogfx->GetMouseY();

	Edit_Tempo_Tempo *found=0,*tm=FirstTempo();

	while(tm){

		if(tm->x<=x && tm->x2>=x && tm->y<y && tm->y2>=y)
		{
			if((tm->tempo->flag&flag)==0)
				found=tm;		
		}
		else
			tm->tempo->flag CLEARBIT flag;

		tm=(Edit_Tempo_Tempo *)tm->next;
	}

	if(found)
	{
		found->tempo->flag|=flag;
		return found;
	}

	return 0;
}

class menu_gototmap:public guiMenu
{
public:
	menu_gototmap(Edit_Tempo *ar,int t)
	{
		editor=ar;
		gototype=t;
	}

	void MenuFunction()
	{		
		editor->Goto(gototype);
	} //

	Edit_Tempo *editor;
	int gototype;
};

void Edit_Tempo::CreateGotoMenu()
{
	DeletePopUpMenu(true);
	AddStandardGotoMenu();

	if(popmenu)
	{
		popmenu->AddFMenu(Cxs[CXS_FIRSTTEMPOVALUE],new menu_gototmap(this,GOTO_FIRST));
		popmenu->AddFMenu(Cxs[CXS_LASTTEMPOVALUE],new menu_gototmap(this,GOTO_LAST));

		popmenu->AddLine();
		popmenu->AddFMenu(Cxs[CXS_FIRSTSELECTEDTEMPOVALUE],new menu_gototmap(this,GOTO_FIRSTSELECTED));
		popmenu->AddFMenu(Cxs[CXS_LASTSELECTEDTEMPOVALUE],new menu_gototmap(this,GOTO_LASTSELECTED));
	}
}

void Edit_Tempo::Goto(int to)
{
	UserEdit();

	if(CheckStandardGoto(to)==true)
		return;

	switch(to)
	{
	case GOTO_FIRST:
		if(NewStartPosition(0,true)==true)
		{
			SyncWithOtherEditors();
		}

		break;

	case GOTO_LAST:
		if(NewStartPosition(WindowSong()->timetrack.LastTempo()->GetTempoStart(),true)==true)
		{
			SyncWithOtherEditors();
		}
		break;

	case GOTO_FIRSTSELECTED:
		{
			Seq_Tempo *t=WindowSong()->timetrack.FirstTempo();

			while(t)
			{
				if(t->IsSelected()==true)
				{
					if(NewStartPosition(t->GetTempoStart(),true)==true)
					{
						SyncWithOtherEditors();
					}
					return;
				}

				t=t->NextTempo();
			}
		}
		break;

	case GOTO_LASTSELECTED:
		{
			Seq_Tempo *t=WindowSong()->timetrack.LastTempo();

			while(t)
			{
				if(t->IsSelected()==true)
				{
					if(NewStartPosition(t->GetTempoStart(),true)==true)
					{
						SyncWithOtherEditors();
					}
					return;
				}

				t=t->PrevTempo();
			}
		}
		break;
	}
}

void Edit_Tempo::AddEditMenu(guiMenu *menu)
{
	class menu_copy:public guiMenu
	{
	public:
		menu_copy(Edit_Tempo *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			//	editor->CopySelectedEvents();
		}

		Edit_Tempo *editor;
	};

	menu->AddFMenu(Cxs[CXS_COPY],new menu_copy(this),SK_COPY);

	class menu_cut:public guiMenu
	{
	public:
		menu_cut(Edit_Tempo *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			//editor->CopySelectedEvents();
			//mainedit->DeleteEvents(&editor->patternselection,false);
		}

		Edit_Tempo *editor;
	};

	menu->AddFMenu(Cxs[CXS_CUT],new menu_cut(this),SK_CUT);

	class menu_paste:public guiMenu
	{
	public:
		menu_paste(Edit_Tempo *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			//editor->Paste();
		}

		Edit_Tempo *editor;
	};

	//if(mainbuffer->CheckBuffer(this,WindowSong(),insertpattern)==true)
	menu->AddFMenu(Cxs[CXS_PASTE],new menu_paste(this),SK_PASTE);

	class menu_delete:public guiMenu
	{
	public:
		menu_delete(Edit_Tempo *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			editor->DeleteTempos();
		}

		Edit_Tempo *editor;
	};

	menu->AddFMenu(Cxs[CXS_DELETE],new menu_delete(this),SK_DEL);
}

void Edit_Tempo::CreateMenuList(guiMenu *menu)
{
	if(!menu)
		return;

	menu->editmenu=menu->AddMenu(Cxs[CXS_EDIT],0);

	if(menu->editmenu)
	{
		maingui->AddUndoMenu(menu->editmenu);
		AddEditMenu(menu->editmenu);
	}

		menu->AddLine();

	if(guiMenu *n=menu->AddMenu(Cxs[CXS_SELECT],0) )
	{
	class menu_selAll:public guiMenu
	{
	public:
		menu_selAll(Edit_Tempo *r){editor=r;}

		void MenuFunction()
		{
			editor->SelectAllTempos(true);
		} //

		Edit_Tempo *editor;
	};
	n->AddFMenu(Cxs[CXS_SELECTALLTEMPOS],new menu_selAll(this),SK_SELECTALL);

	class menu_selAllOff:public guiMenu
	{
	public:
		menu_selAllOff(Edit_Tempo *r){editor=r;}

		void MenuFunction()
		{
			editor->SelectAllTempos(false);
		} //

		Edit_Tempo *editor;
	};
	n->AddFMenu(Cxs[CXS_DESELECTALLTEMPOS],new menu_selAllOff(this),SK_DESELECTALL);
	}

#ifdef OLDIE
	guiMenu *n;

	//ResetUndoMenu();

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		n=menu->AddMenu(Cxs[CXS_FILE],0);
		if(n)
		{
			class menu_loadtmap:public guiMenu
			{
			public:
				menu_loadtmap(Edit_Tempo *e){editor=e;}

				void MenuFunction()
				{
					editor->LoadTempoMap();
				}

				Edit_Tempo *editor;
			};

			n->AddFMenu(Cxs[CXS_LOADTEMPOMAP],new menu_loadtmap(this));

			class menu_savetmap:public guiMenu
			{
			public:
				menu_savetmap(Edit_Tempo *e){editor=e;}
				void MenuFunction(){editor->SaveTempoMap();}

				Edit_Tempo *editor;
			};

			n->AddFMenu(Cxs[CXS_SAVETEMPOMAP],new menu_savetmap(this));
		}

		// Piano Editor Menu
		editmenu=menu->AddMenu(Cxs[CXS_EDIT],0);
		if(editmenu)
		{
			maingui->AddUndoMenu(editmenu);

			class menu_deletetempos:public guiMenu
			{
			public:
				menu_deletetempos(Edit_Tempo *e){editor=e;}
				void MenuFunction(){editor->DeleteTempos();}
				Edit_Tempo *editor;
			};
			editmenu->AddFMenu(Cxs[CXS_DELETE],new menu_deletetempos(this),SK_DEL);

			editmenu->AddLine();

			class menu_selAll:public guiMenu
			{
			public:
				menu_selAll(Edit_Tempo *r){editor=r;}

				void MenuFunction()
				{
					editor->SelectAllTempos(true);
				} //

				Edit_Tempo *editor;
			};
			editmenu->AddFMenu(Cxs[CXS_SELECTALLTEMPOS],new menu_selAll(this));

			class menu_selAllOff:public guiMenu
			{
			public:
				menu_selAllOff(Edit_Tempo *r){editor=r;}

				void MenuFunction()
				{
					editor->SelectAllTempos(false);
				} //

				Edit_Tempo *editor;
			};
			editmenu->AddFMenu(Cxs[CXS_DESELECTALLTEMPOS],new menu_selAllOff(this));
			AddStandardGotoMenu();
		}

		// Standard Editor Menu
		n=menu->AddMenu("Editor",0);
		if(n)
		{			
			CreateEditorMenu(n);
		}
	}

	maingui->AddCascadeMenu(this,menu);
#endif

}



void Edit_Tempo::MouseWheel(int delta,guiGadget *db)
{
	if(Edit_MouseWheel(delta)==false)
	{
		if((!db) || db==tempogfx)
		{
			if(vertgadget)
				vertgadget->DeltaY(delta);
		}
	}
}

void Edit_Tempo::ShowOverview()
{
	if(!overview)return;

	guiBitmap *bitmap=&overview->gbitmap;

	bitmap->guiFillRect(COLOUR_OVERVIEW_BACKGROUND);
	//bitmap->guiDrawRect(0,0,overview->GetX2(),overview->GetY2(),COLOUR_GREY);

	overviewlenght=WindowSong()->GetSongLength_Ticks();

	double maxtempo=temporange[tempogfxrange];
	double h2=overview->GetY2();
	int y2=overview->GetY2();

	Seq_Tempo *lasttempo=0;

	for(int x=0;x<overview->GetX2();x++)
	{
		Seq_Tempo *tempo=WindowSong()->timetrack.GetTempo(GetOverviewTime(x));

		if(tempo->tempo>maxtempo)
		{
			bitmap->guiDrawLineX(x,COLOUR_OVERVIEWOBJECT);
		}
		else
		{
			double h=tempo->tempo;
			h/=maxtempo;
			h*=h2;

			bitmap->guiDrawLineX(x,y2-(int)h,y2,lasttempo!=tempo?COLOUR_GREEN:COLOUR_OVERVIEWOBJECT);

			if(lasttempo==tempo)
				bitmap->guiDrawPixel(x,y2-(int)h,COLOUR_GREEN);
		}

		lasttempo=tempo;
	}
}

void TempoEditor_Overview_Callback(guiGadget_CW *g,int status)
{
	Edit_Tempo *p=(Edit_Tempo *)g->from;

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

void Edit_Tempo::ShowSlider()
{
	if(vertgadget)
		vertgadget->ChangeSlider(&tempogfxobjects,zoomy);
}

double Edit_Tempo::ConvertYToTempo(int my)
{
	double h=starttempo;

	double sh=tempogfxobjects.height;
	double sh2=my;

	sh2/=sh;

	sh=temporange[tempogfxrange];
	sh2*=sh;

	h-=sh2;

	if(h<0)
		h=0;

	TRACE ("CYT %f\n",h);

	return h;
}

int Edit_Tempo::ConvertTempoToY(double tempo)
{
	return tempogfxobjects.GetYPosHiLo(tempo,temporange[tempogfxrange]);
}

void Edit_Tempo::ShowGFX()
{
	if(!tempogfx)
		return;

	guiBitmap *bitmap=&tempogfx->gbitmap;
	bitmap->guiFillRect(COLOUR_BACKGROUNDEDITOR_GFX);

	int numberoftempos=tempogfx->GetHeight()/zoomy;

	if(zoomvert==true)
		tempogfxobjects.BufferYPos();

	int tempostep=temporange[tempogfxrange]/10;

	tempogfxobjects.DeleteAllO(tempogfx);
	tempogfxobjects.SetHeight(tempostep*zoomy);
	tempogfxobjects.CalcStartYOff(zoomy,numberoftempos,0,tempostep-1);
	tempogfxobjects.EndBuild();

	if(initstarty==true)
	{
		// Find Start Tempo
		Seq_Tempo *t=WindowSong()->timetrack.GetTempo(startposition);

		if(t->tempo<temporange[tempogfxrange])
		{
			double max=temporange[tempogfxrange];
			double h=max-t->tempo;
			h/=10;
			h*=(double)zoomy;

			tempogfxobjects.SetStartY_Mid((int)h);
		}

		initstarty=false;
	}

	if(zoomvert==true)
		tempogfxobjects.RecalcYPos();

	// Show V Tempo Lines

	tempogfxobjects.InitYStart(zoomy);

	Seq_Tempo *ltempo=0;

	for(int x=0;x<tempogfx->GetX2();x++)
	{
		Seq_Tempo *t=WindowSong()->timetrack.GetTempo(timeline->ConvertXPosToTime(x));

		int y=ConvertTempoToY(t->tempo);

		switch(y)
		{
		case -1: // top
			bitmap->guiDrawLineX(x,ltempo!=t?COLOUR_WHITE:COLOUR_BLUE_LIGHT);
			break;

		case -2: // bottom
			break;

		default:
			bitmap->guiDrawLineX(x,y,bitmap->GetY2(),ltempo!=t?COLOUR_WHITE:COLOUR_BLUE_LIGHT);

			if(ltempo==t)
				bitmap->guiDrawPixel(x,y,COLOUR_YELLOW);
			break;
		}

		ltempo=t;
	}

	// Scale 10 Steps
	char tns[NUMBERSTRINGLEN];
	int tempo=tempostep-tempogfxobjects.startobjectint,
		subw=bitmap->GetTextWidth(" 9999 ");

	// Start/End Tempo

	double sh=tempogfxobjects.height;
	double sh2=tempogfxobjects.starty;

	sh2/=sh;

	sh=temporange[tempogfxrange];
	sh2*=sh;

	starttempo=sh-sh2;

	TRACE ("STempo %f\n",starttempo);

	bitmap->SetFont(&maingui->standard_bold);
	bitmap->SetTextColour(COLOUR_GREY_LIGHT);

	while(tempogfxobjects.GetInitY()<tempogfx->GetY2() && tempo>=0)
	{
		int ty=tempogfxobjects.GetInitY(); // y
		int ty2=tempogfxobjects.AddInitY(zoomy)-1;

		bitmap->guiDrawLineYX0(ty,tempogfx->GetX2(),COLOUR_GREY_LIGHT);

		char *text=mainvar->ConvertIntToChar(tempo*10,tns);

		bitmap->guiDrawText(1,ty,tempogfx->GetX2(),text);

		if(tempogfx->GetWidth()>400)
		{
			bitmap->guiDrawText(tempogfx->GetX2()-subw,ty+maingui->GetFontSizeY()+1,tempogfx->GetX2(),text);
		}

		tempo--;
	}

	bitmap->SetFont(&maingui->standardfont);

	//if(tempogfxobjects.guiheight>tempogfx->g
	tempogfxobjects.DrawUnUsed(tempogfx);

	timeline->DrawPositionRaster(bitmap);

	ShowSlider();
}

void ShowTempoGFX_Callback(guiGadget_CW *g,int status)
{
	Edit_Tempo *p=(Edit_Tempo *)g->from;

	switch(status)
	{
	case DB_CREATE:
		g->menuindex=0;
		p->tempogfx=g;
		break;

	case DB_PAINT:
		{
			p->ShowGFX();
			p->ShowTempos();
			p->ShowCycleAndPositions(g);
		}
		break;

	case DB_PAINTSPRITE:
		p->ShowCycleAndPositions(p->tempogfx);
		break;

	case DB_LEFTMOUSEDOWN:
		p->MouseClickInTempos(true);
		break;

	case DB_DOUBLECLICKLEFT:
		p->MouseDoubleClickInTempos(true);
		break;

	case DB_LEFTMOUSEUP:
		p->MouseReleaseInTempos(true);
		break;

	case DB_RIGHTMOUSEDOWN:
		p->MouseClickInTempos(false);
		break;

	case DB_MOUSEMOVE:
	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		p->MouseMoveInTempos(true);	
		break;

		//case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
		//case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInOverview(false);	
		break;
	}
}

void Edit_Tempo::NewZoom()
{
	RefreshStartPosition();
}

void Edit_Tempo::RefreshStartPosition()
{
	DrawHeader();
	ShowCycleAndPositions(tempogfx);
	DrawDBBlit(tempogfx); ///*showlist==true?waveraster:*/0);
	ShowOverviewCycleAndPositions();
	DrawDBSpriteBlit(overview);
}

void Edit_Tempo::ShowMenu()
{
	ShowEditorMenu();
}

void Edit_Tempo::SelectAllTempos(bool on)
{
	Seq_Tempo *t=WindowSong()->timetrack.FirstTempo();

	while(t)
	{
		if(on==true)
			t->Select();
		else
			t->UnSelect();

		t=t->NextTempo();
	}
}

bool Edit_Tempo::ZoomGFX(int z,bool horiz)
{
	if(SetZoomGFX(z,horiz)==true)
	{
		DrawDBBlit(tempogfx);
		return true;
	}

	return false;
}

void Edit_Tempo::InitMouseEditRange()
{
	int mx=tempogfx->GetMouseX();
	int my=tempogfx->GetMouseY();

	mstempo=modestarttempo;
	metempo=ConvertYToTempo(my);

	msposition=modestartposition;
	msendposition=mx<tempogfx->GetX2()?timeline->ConvertXPosToTime(mx):endposition;

	if(msposition>msendposition){
		OSTART h=msendposition;
		msendposition=msposition;
		msposition=h;
	}

	if(mstempo<metempo){
		double h=metempo;
		metempo=mstempo;
		mstempo=h;
	}

	int sx=timeline->ConvertTimeToX(msposition);
	int sx2=timeline->ConvertTimeToX(msendposition);
	int sy=ConvertTempoToY(mstempo);
	int sy2=ConvertTempoToY(metempo);

	if(sy==-1)sy=0;
	else
		if(sy==-2)sy=tempogfx->GetY2();

	if(sy2==-1)sy2=0;
	else
		if(sy2==-2)sy2=tempogfx->GetY2();

	mouseeditx=sx==-1?0:sx;
	mouseeditx2=sx2==-1?tempogfx->GetX2():sx2;
	mouseedity=sy;
	mouseedity2=sy2;
}

Edit_Tempo::Edit_Tempo()
{
	editorid=EDITORTYPE_TEMPO;
	editorname="Tempo";
	InitForms(FORM_HORZ2x1BAR_SLIDERHV);
	EditForm(0,1,CHILD_HASWINDOW);

	minwidth=minheight=maingui->GetButtonSizeY(8);
	resizeable=true;

	starttempo=140;

	showlist=true;
	displayusepos=0;
	showquarterframe=false;
	followsongposition=mainsettings->followeditor;
	tempogfxrange=mainsettings->defaulttemporange;
	selectmode=false;

	SetMinZoomY(maingui->GetButtonSizeY()+4);

	initstarty=true;
	tapinit=false;
}

void Edit_Tempo::LoadTempoMap()
{
	camxFile sfile;

	if(sfile.OpenFileRequester(0,this,Cxs[CXS_LOADTEMPOMAP],sfile.AllFiles(camxFile::FT_TEMPOMAP),true)==true)
	{
		if(sfile.OpenRead(sfile.filereqname)==true)
		{
			char check[4];

			sfile.Read(check,4);

			if(mainvar->CompareStringWithOutZero(check,"CAMX")==true)
			{
				sfile.Read(check,4);

				if(mainvar->CompareStringWithOutZero(check,"TMAP")==true)
				{
					int version=0;

					sfile.Read(&version);

					// Chunk
					sfile.LoadChunk();

					if(sfile.GetChunkHeader()==CHUNK_TEMPOMAP)
					{
						mainthreadcontrol->LockActiveSong();

						WindowSong()->timetrack.LoadTempoMap(&sfile);

						mainthreadcontrol->UnlockActiveSong();

						maingui->OpenEditEditors(WindowSong(),EDITORTYPE_TEMPO);
						maingui->RefreshAllEditorsWithTempo(WindowSong(),0);
						maingui->CloseEditEditors(WindowSong(),EDITORTYPE_TEMPO);
					}
				}
			}
		}

		sfile.Close(true);
	}
}

void Edit_Tempo::SaveTempoMap()
{
	camxFile sfile;

	if(sfile.OpenFileRequester(0,this,Cxs[CXS_SAVETEMPOMAP],sfile.AllFiles(camxFile::FT_TEMPOMAP),false)==true)
	{
		sfile.AddToFileName(".cxtm");

		if(sfile.OpenSave(sfile.filereqname)==true)
		{
			int version=maingui->GetVersion();

			// Header
			sfile.Save("CAMX",4);
			sfile.Save("TMAP",4);
			sfile.Save(&version,sizeof(int)); // Version

			WindowSong()->timetrack.SaveTempoMap(&sfile);
		}

		sfile.Close(true);
	}
}

void Edit_Tempo::RefreshObjects(LONGLONG type,bool editcall) // v
{
	DrawHeader(); // Tempo Changes etc..

	DrawDBBlit(tempogfx,overview);

	if(tempolisteditor && showlist==true)
		DrawDBBlit(tempolisteditor->list);
}

void Edit_Tempo::CreateWindowMenu()
{
	if(NewWindowMenu())
	{
		CreateMenuList(menu);
		//AddEditMenu(menu);
	}
}

guiMenu *Edit_Tempo::CreateMenu()
{
	//	ResetUndoMenu();
	if(DeletePopUpMenu(true))
	{
		CreateMenuList(popmenu);	
	}

	//maingui->AddCascadeMenu(this,menu);
	return 0;
}

void Edit_Tempo::ShowTapValue()
{
	if(g_tapvalue)
	{
		char nr[NUMBERSTRINGLEN];

		if(tapinit==true)
		{
			if(tapvalue>0)
			{
			char *h=mainvar->ConvertDoubleToChar(tapvalue,nr,3);
			g_tapvalue->ChangeButtonText(h);

			glist.Enable(settapvalue,true);
			}
			else
			{
				g_tapvalue->ChangeButtonText("...");
				glist.Enable(settapvalue,false);
			}
		}
		else
		{
			g_tapvalue->ChangeButtonText("-Init-");
			glist.Enable(settapvalue,false);
		}
	}

}

void Edit_Tempo::Init()
{
	glist.SelectForm(0,0);
	guitoolbox.CreateToolBox(TOOLBOXTYPE_EDITOR);
	glist.Return();

	int addw=INFOSIZE;

	glist.AddButton(-1,-1,addw,-1,"List",GADGETID_LIST,showlist==true?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED|MODE_AUTOTOGGLE:MODE_GROUP|MODE_TOGGLE|MODE_AUTOTOGGLE);
	glist.AddLX();

	range=glist.AddCycle(-1,-1,2*addw,-1,GADGETID_RANGE,0,0,Cxs[CXS_RANGETEMPO]);

	if(range)
	{
		range->AddStringToCycle("Tempo: 0-240 BPM");
		range->AddStringToCycle("Tempo: 0-480 BPM");
		range->AddStringToCycle("Tempo: 0-960 BPM");

		range->SetCycleSelection(tempogfxrange);
	}

	glist.AddLX();
	glist.AddButton(-1,-1,2*addw,-1,"Reset xxx.000",GADGETID_RESETVALUES,MODE_TEXTCENTER,Cxs[CXS_TEMPOEVENTRESET]);

	glist.AddLX();
	glist.AddButton(-1,-1,2*addw,-1,"TAP",GADGETID_TAP,MODE_TEXTCENTER,"Mouse Click->Tempo");

	glist.AddLX();
	g_tapvalue=glist.AddButton(-1,-1,addw,-1,"",GADGETID_TAPVALUE,MODE_TEXTCENTER,Cxs[CXS_TAPTEMPO]);
	
	glist.AddLX();
	settapvalue=glist.AddButton(-1,-1,addw,-1,"Tap->Song",GADGETID_SETTAPVALUE,MODE_TEXTCENTER,Cxs[CXS_SETTAPVALUE]);

	ShowTapValue();

	int offsettracksy=SIZEV_OVERVIEW+SIZEV_HEADER+2*(ADDYSPACE+1);

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

	// List
	glist.SelectForm(0,1);
	glist.form->BindWindow(tempolisteditor=new Edit_TempoList(this));

	// Fx
	glist.SelectForm(1,1);
	glist.AddChildWindow(-1,-1,-1,SIZEV_OVERVIEW,MODE_RIGHT|MODE_SPRITE,0,&TempoEditor_Overview_Callback,this);
	glist.Return();
	glist.AddChildWindow(-1,-1,-1,SIZEV_HEADER,MODE_RIGHT|MODE_SPRITE,0,&Editor_Header_Callback,this);

	editarea=glist.AddChildWindow(-1,offsettracksy,-1,-2,MODE_BOTTOM|MODE_RIGHT|MODE_SPRITE,0,&ShowTempoGFX_Callback,this);
}

void Edit_Tempo::SetMouseMode(int newmode)
{
	InitMousePosition();
	modestartposition=GetMousePosition(); // X

	if(modestartposition>=0)
	{
		modestarttempo=ConvertYToTempo(tempogfx->GetMouseY());

		if(modestarttempo!=-1)
		{
			SetEditorMode(newmode);
			SetAutoScroll(newmode,tempogfx);
		}
	}
}

void Edit_Tempo::DeleteTempos()
{
	mainedit->DeleteSelectedTempos(WindowSong(),addtolastundo);
	addtolastundo=true;
}

void Edit_Tempo::CreateTempo(OSTART position,double ntempo)
{
	Seq_Tempo *t=WindowSong()->timetrack.FirstTempo();

	while(t && t->GetTempoStart()<=position)
	{
		if(t->GetTempoStart()==position)
			return;

		t=t->NextTempo();
	}

	mainedit->CreateNewTempo(WindowSong(),position,ntempo,false);
}

EditData *Edit_Tempo::EditDataMessage(EditData *data)
{
	if(CheckStandardDataMessage(data)==true)
		return 0;

	switch(data->id)
	{
	case EDIT_TEMPO:
		{
			if(data->helpobject)
			{
				Edit_Tempo_Tempo *et=(Edit_Tempo_Tempo *)data->helpobject;
				et->tempo->ChangeTempo(WindowSong(),et->tempo->GetTempoStart(),data->dnewvalue);
			}
		}
		break;

	default:
		return data;
		break;
	}

	return 0;
}


void Edit_Tempo::MouseMoveInTempos(bool leftmouse)
{
	if(CheckMouseMovePosition(tempogfx)==true)
		return;


	switch(mousemode)
	{
	case EM_SELECTOBJECTS:
		{
			ShowCycleAndPositions(tempogfx);
			tempogfx->DrawSpriteBlt();

			int mx=tempogfx->GetMouseX();
			int my=tempogfx->GetMouseY();

			WindowSong()->timetrack.OpenPRepairTempoSelection();

			// 1. 
			Seq_Tempo *t=WindowSong()->timetrack.FirstTempo();

			while(t)
			{
				if(mainvar->CheckIfInPosition(msposition,msendposition,t->GetTempoStart())==true && mainvar->CheckIfInIndex(mstempo,metempo,t->tempo)==true)
					t->PRepairSelection();

				t=t->NextTempo();
			}

			// 2. Lasso
			Edit_Tempo_Tempo *ett=FirstTempo();

			while(ett){

				if(CheckInLasso(ett->x,ett->y,ett->x2,ett->y2)==true)
					ett->tempo->PRepairSelection();

				ett=(Edit_Tempo_Tempo *)ett->next;
			}

			//	maingui->ClosePRepairSelection(&patternselection);
		}
		break;

	case EM_MOVEOS:
		{
			ShowMoveTempoSprites();
		}
		break;

	}
}

void Edit_Tempo::MouseReleaseInTempos(bool leftmouse)
{
	switch(mousemode)
	{
	case EM_SELECTOBJECTS:
		{
			Seq_Tempo *t=WindowSong()->timetrack.FirstTempo();

			while(t)
			{
				if(t->flag&OFLAG_UNDERSELECTION)
				{
					t->flag CLEARBIT OFLAG_UNDERSELECTION;
					t->Select();
				}

				t=t->NextTempo();
			}
		}
		break;

	case EM_MOVEOS:
		{
			EditCancel();

			MoveO mo;

			mo.song=WindowSong();
			mo.diff=movediff;
			mo.dindex=moveobjects_vert;

			//if(maingui->GetCtrlKey()==true)
			//	mainedit->CopySelectedEventsInPatternList(&mo);
			//else

			mainedit->EditSelectedTempos(&mo);
		}
		break;
	}

	ResetMouseMode();
}

void Edit_Tempo::MouseDoubleClickInTempos(bool leftmouse)
{
	Edit_Tempo_Tempo *te=FindTempoUnderMouse();
	Seq_Tempo *tempo=te?(Seq_Tempo *)te->tempo:0;

	if( (!te) || maingui->GetShiftKey()==true)
	{
		DoubleClickInEditArea();
		return;
	}

}

void Edit_Tempo::FindAndDeleteTempos(Seq_Tempo *t)
{
	if(t)
		t->Select();

	DeleteTempos();
}

void Edit_Tempo::MouseClickInTempos(bool leftmouse)
{
	if(leftmouse==false)
	{
		if(EditCancel()==true)
			return;
	}
	else
	{
		if(CheckMouseClickInEditArea(tempogfx)==true) // Left Mouse
		{
			return;
		}
	}

	Edit_Tempo_Tempo *e=FindTempoUnderMouse();
	Seq_Tempo *tempo=e?(Seq_Tempo *)e->tempo:0;

	InitMousePosition();

	switch(mousemode)
	{
	case EM_SELECT:
	case EM_CUT:
	case EM_DELETE:
		{
			if(leftmouse==true)
			{
				if((!e) && maingui->GetCtrlKey()==false)
					WindowSong()->timetrack.SelectAllTempos(false);

				if((!e) || maingui->GetShiftKey()==true)
				{
					SetMouseMode(EM_SELECTOBJECTS);
					return;
				}
			}

			if((!e) && mousemode!=EM_DELETE)
				return;
		}
		break;
	}	

	switch(mousemode)
	{
	case EM_CREATE:
		{
			double ntempo=ConvertYToTempo(tempogfx->GetMouseY());

			CreateTempo(this->GetMousePosition(),ntempo);
		}
		break;

	case EM_CUT:

		break;

	case EM_DELETE:
		if(leftmouse==true)
		{
			addtolastundo=false;
			FindAndDeleteTempos(tempo);
		}
		break;

	default:
		if(leftmouse==true && tempo)
		{
			tempo->Select();
			//RefreshEvents();

			//SetEditorMode(EM_SELECTOBJECTSSTART);
			mainhelpthread->AddMessage(MOUSEBUTTONDOWNMS,this,MESSAGE_CHECKMOUSEBUTTON,MESSAGE_TEMPO,tempo);
		}
		break;
	}
}

bool Edit_Tempo::EditCancel()
{
	// Reset Selection
	Seq_Tempo *e=WindowSong()->timetrack.FirstTempo();

	while(e){
		e->flag CLEARBIT OFLAG_UNDERSELECTION;
		e=e->NextTempo();
	}

	switch(mousemode)
	{
	case EM_SELECTOBJECTS:
		ResetMouseMode();
		return true;
		break;

	case EM_MOVEOS:
		SetEditorMode(EM_RESET);
		DrawDBBlit(tempogfx);
		break;
	}

	return false;
}

void Edit_Tempo::KeyDown()
{
	Editor_KeyDown();
}

void Edit_Tempo::KeyDownRepeat()
{
	Editor_KeyDown();
}

void Edit_Tempo::Gadget(guiGadget *gadget)
{
	if(!Editor_Gadget(gadget))return;

	if(gadget)
	{
		switch(gadget->gadgetID)
		{
		case GADGETID_SETTAPVALUE:
			{
				if(tapvalue>0)
					WindowSong()->ChangeTempoAtPosition(-1,tapvalue,0);
			}
			break;

		case GADGETID_TAPVALUE:
			{
				tapinit=false;
				ShowTapValue();
			}
			break;

		case GADGETID_TAP:
			{
				if(tapinit==false)
				{
					lasttempotapsystime=maintimer->GetSystemTime();
					tapinit=true;

					tapvalue=-1;

					ShowTapValue();
				}
				else
				{
					LONGLONG t=maintimer->GetSystemTime();
					double ms=maintimer->ConvertSysTimeToMs(t-lasttempotapsystime);
					lasttempotapsystime=t;

					// 120 BPM 500ms beat
					ms/=2;

					double h=500/ms;
					h*=120;
					
					if(h<10)
					{
						return;
					}
					else
					tapvalue=h;
					
					TRACE ("ms %f\n",ms);

					ShowTapValue();	
				}
			}
			break;

		case GADGETID_RESETVALUES:
			{
				MoveO mo;

				mo.song=WindowSong();
				mo.diff=0;
				mo.dindex=0;
				mo.resetdbvalues=true;

				mainedit->EditSelectedTempos(&mo);
			}
			break;

		case GADGETID_LIST:
			{
				showlist=showlist==true?false:true;
				FormEnable(0,1,showlist);
			}
			break;

		case GADGETID_RANGE:
			{
				tempogfxrange=mainsettings->defaulttemporange=gadget->index;
				DrawDBBlit(tempogfx,overview);
			}
			break;

		case GADGETID_EDITORSLIDER_VERT: // Scroll V
			{
				tempogfxobjects.InitWithSlider(vertgadget);
				DrawDBBlit(tempogfx);
			}
			break;
		}
	}
}

void Edit_Tempo::ShowMoveTempoSprites()
{
	int my=tempogfx->GetMouseY();
	double temponow=ConvertYToTempo(my);
	InitMousePosition();

	if(GetMousePosition()>=0 && temponow>0)
	{
		moveobjects_vert=CanMoveY()==true?temponow-modestarttempo:0;

		TRACE ("Tempo Move V %f\n",moveobjects_vert);

		movediff=CanMoveX()==true?GetMousePosition()-modestartposition:0;
		tempogfx->DrawGadgetBlt();
	}
}

void Edit_Tempo::UserMessage(int msg,void *par)
{
	switch(msg)
	{
	case MESSAGE_CHECKMOUSEBUTTON:
		{
			if(maingui->GetShiftKey()==false && par)
			{
				Seq_Tempo *tempo=(Seq_Tempo *)par;

				if(maingui->GetLeftMouseButton()==true)
				{	
					Edit_Tempo_Tempo *ett=(Edit_Tempo_Tempo *)tempos.GetRoot();

					while(ett)
					{
						if(ett->tempo==tempo)
						{
							SetMouseMode(EM_MOVEOS);
							ShowMoveTempoSprites();
							return;
						}

						ett=(Edit_Tempo_Tempo *)ett->next;
					}

					return;
				}

				//	patternselection.SelectAllEventsNot(note,false,0);
			}
		}
		break;
	}
}

void Edit_Tempo::ReleaseEdit()
{
	if(GetUndoEditEvents())
	{
		if(GetUndoEditEvents()->CheckChanges()==false)
		{
			GetUndoEditEvents()->Delete();
		}
		else
		{
			// -> Undo
			mainedit->EditEvents(WindowSong(),GetUndoEditEvents());
		}

		SetUndoEditEvents(0);
	}
}



void Edit_Tempo::AddStartY(int addy)
{
	if(tempogfxobjects.AddStartY(addy)==true)
	{
		DrawDBBlit(tempogfx);
	}
}

void Edit_Tempo::AutoScroll()
{
	DoAutoScroll();
	DoStandardYScroll(tempogfx);
}

void Edit_Tempo::RefreshRealtime()
{		
	if(RefreshEventEditorRealtime()==true)	
	{
		if(tempogfx)
		{
			ShowCycleAndPositions(tempogfx);
			tempogfx->DrawSpriteBlt();
		}
	}

	bool blit=false;

	Edit_Tempo_Tempo *t=FirstTempo();

	while(t)
	{
		if(t->eflag!=t->tempo->flag)
		{
			t->Draw();
			blit=true;
		}

		t=(Edit_Tempo_Tempo *)t->next;
	}

	if(blit==true)
	DrawDBSpriteBlit(tempogfx);


	RefreshToolTip();
}


void Edit_Tempo::RefreshRealtime_Slow()
{
	RefreshEventEditorRealtime_Slow();

	/*
	int h_getcountselectedtempos=WindowSong()->timetrack.GetSelectedTempos(),
	h_getcounttempos=WindowSong()->timetrack.GetCountOfTempos();

	if(h_getcountselectedtempos!=getcountselectedtempos ||
	h_getcounttempos!=getcounttempos)
	SongNameRefresh();
	*/
}

void Edit_Tempo::DeInitWindow()
{	
	tempos.DeleteAllO();
}

void Edit_Tempo::ShowTempos()
{
	tempos.DeleteAllO();

	if(!tempogfx)
		return;

	guiBitmap *bitmap=&tempogfx->gbitmap;

	{
		Seq_Tempo *t=WindowSong()->timetrack.FirstTempo();

		while(t)
		{
			OSTART pos=t->GetTempoStart();

			if(t->PrevTempo() && t->IsSelected()==true)
			{
				pos+=numbereditposition_sum;
				if(tempolisteditor && showlist==true)
					pos+=tempolisteditor->numbereditposition_sum;
			}

			if(pos>=startposition && pos<endposition)
			{
				double tv=t->tempo;

				if(t->IsSelected()==true)
				{
					if(tempolisteditor && showlist==true && tempolisteditor->list)
						tv+=tempolisteditor->editsum;
				}

				int y=ConvertTempoToY(tv);

				if(y>=0)
				{
					int x=timeline->ConvertTimeToX(pos);

					if(Edit_Tempo_Tempo *ett=new Edit_Tempo_Tempo(this,t,bitmap,x,y))
					{
						tempos.AddEndO(ett);
						ett->Draw();
					}
				}
			}

			t=t->NextTempo();
		}
	}

	if(mousemode==EM_MOVEOS)
	{
		Seq_Tempo *t=WindowSong()->timetrack.FirstSelectedTempo();

		if(t)
		{
			double lowest,highest;

			lowest=highest=t->tempo;

			if(movediff<-t->GetTempoStart()) // FirstPosition
				movediff=-t->GetTempoStart();

			// High/Low
			{
				Seq_Tempo *t2=t;
				while(t2)
				{
					if(t2->IsSelected()==true)
					{
						if(t2->tempo<lowest)
							lowest=t2->tempo;

						if(t2->tempo>highest)
							highest=t2->tempo;
					}

					t2=t2->NextTempo();
				}
			}

			double max=temporange[tempogfxrange];

			if(moveobjects_vert<0)
			{
				if(lowest+moveobjects_vert<0)
					moveobjects_vert=-lowest;
			}
			else
			{
				if(highest+moveobjects_vert>max)
					moveobjects_vert=max-highest;
			}

			Seq_Tempo movetempo;
			Edit_Tempo_Tempo etempo(this,&movetempo,bitmap,0,0);

			// Draw
			while(t)
			{
				if(t->IsSelected()==true)
				{
					OSTART start=t->GetTempoStart();

					if(t!=WindowSong()->timetrack.FirstTempo())
						start+=movediff;

					movetempo.ostart=start;
					movetempo.tempo=t->tempo+moveobjects_vert;

					if(movetempo.tempo<MINIMUM_TEMPO)
						movetempo.tempo=MINIMUM_TEMPO;

					etempo.inity=ConvertTempoToY(movetempo.tempo);

					if(etempo.inity>=0 && start>=startposition && start<=endposition)
					{
						etempo.initx=timeline->ConvertTimeToX(start);
						etempo.Draw();
					}
				}

				t=t->NextTempo();
			}
		}
	}
}

char *Edit_Tempo::GetToolTipString1() //v
{
#ifdef OLDIE
	char *string=0;
	Edit_Tempo_Tempo *et=FindTempoAtPosition(GetMouseX(),GetMouseY());

	if(et) 
	{
		char h2[NUMBERSTRINGLEN],*tv=mainvar->ConvertDoubleToChar(et->tempo->tempo,h2,3);
		return mainvar->GenerateString("T:",tv);
	}

	if(frame_tempos.ondisplay==true && frame_tempos.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
	{
		// Y Tempo
		double h=FindTempoAtPosition(GetMouseY());

		if(h!=-1)
		{
			char h2[NUMBERSTRINGLEN];
			return mainvar->GenerateString(mainvar->ConvertDoubleToChar(h,h2,3));
		}
	}
#endif

	return 0;
}
