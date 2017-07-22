#include "songmain.h"
#include "editor.h"
#include "editor_event.h"
#include "editsettings.h"
#include "object_song.h"
#include "camxgadgets.h"
#include "audiomixeditor.h"
#include "sampleeditor.h"
#include "audiomanager.h"
#include "arrangeeditor.h"
#include "transporteditor.h"
#include "waveeditor.h"
#include "pianoeditor.h"
#include "groove.h"
#include "editdata.h"
#include "MIDIhardware.h"
#include "quantizeeditor.h"
#include "audiomaster.h"
#include "editMIDIfilter.h"
#include "vstguiwindow.h"
#include "tempomapeditor.h"
#include "editor_help.h"
#include "vstplugins.h"
#include "stepeditor.h"
#include "keyboard.h"
#include "edit_audiointern.h"
#include "editor_text.h"
#include "editor_marker.h"
#include "wavemap.h"
#include "groupeditor.h"
#include "edit_processor.h"
#include "player.h"
#include "bigtime.h"
#include "score.h"
#include "rmg.h"
#include "editwin32audio.h"
#include "library.h"
#include "audiohardware.h"
#include "object_project.h"
#include "audiopattern.h"

#ifdef WIN32
#include<winuser.h>
#endif

#include <stdio.h>
#include <stdlib.h>

OSTART guiWindow::GetMousePosition()
{
	return WindowSong()?WindowSong()->mouseposition:-1;
}

LONGLONG guiWindow::GetMouseSamplePosition(AudioPattern *p)
{
	LONGLONG msp=WindowSong()?WindowSong()->timetrack.ConvertTicksToTempoSamples(GetMousePosition()):-1;

	if(songmode==true && p)
	{
		LONGLONG sps=WindowSong()->timetrack.ConvertTicksToTempoSamples(p->GetPatternStart());

		if(sps>msp)
			return 0;

		LONGLONG spe=WindowSong()->timetrack.ConvertTicksToTempoSamples(p->GetPatternEnd());

		if(msp>spe)
			msp=spe;

		return msp-sps;
	}

	return msp;
}

void guiWindow::SetWindowSize(int w,int h)
{
	if(w>0 && h>0)
	{
		RECT wsize;

		wsize.left=0;
		wsize.top=0;
		wsize.right=w-1;
		wsize.bottom=h-1;

		AdjustWindowRectEx(&wsize,style,menu?TRUE:FALSE /*win->mdimode==true?false:true*/ ,flagex);
		SetWindowPos(hWnd, HWND_TOPMOST,0,0,w,h,SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
	}
}

void guiWindow::SetWindowDefaultSize()
{
	// Save Coos
	if( dontchangesettings==false &&
		editorid>=0 && 
		minimized==false &&
		width>=0 &&
		height>=0
		)
	{
		mainsettings->windowpositions[editorid].x=win_screenposx;
		mainsettings->windowpositions[editorid].y=win_screenposy;

		mainsettings->windowpositions[editorid].width=width;
		mainsettings->windowpositions[editorid].height=height;

		mainsettings->windowpositions[editorid].set=true;
		mainsettings->windowpositions[editorid].maximized=maximized;
	}	
}


void guiWindow::NewTimeFormat()
{

	if(timeline)
	{
		timeline->format=windowtimeformat;
		DrawHeader();
	}

	guitoolbox.ShowTimeType();

	

	switch(editorid)
	{
	case EDITORTYPE_ARRANGE:
	case EDITORTYPE_DRUM:
	case EDITORTYPE_WAVE:
	case EDITORTYPE_PIANO:
	case EDITORTYPE_SCORE:
	case EDITORTYPE_SAMPLE:
	case EDITORTYPE_TEMPO:
	case EDITORTYPE_EVENT:
	case EDITORTYPE_TEXT:
	case EDITORTYPE_MARKER:
		{
			((EventEditor *)this)->InitDisplay();

		RefreshObjects(0,false);
		}
		break;
	}

	InitNewTimeType();
}

void guiWindow::CloseHeader()
{
	if(timeline){

		timeline->DeInit();
		delete timeline;
		timeline=0;
	}
}

guiMenu *guiWindow::DeletePopUpMenu(bool createnew)
{
	if(popmenu){

		if(skipdeletepopmenu==true)
		{
			skipdeletepopmenu=false;
		}
		else
		{
			TRACE ("DeletePopUpMenu \n");
			popmenu->RemoveMenu();
			popmenu=0;
		}
	}

	if(createnew==true)
	{
		popmenu=new guiMenu;
		popmenu->window=this;
	}

	return popmenu;
}

void guiWindow::CheckGadget(guiGadget *g)
{
	switch(g->type)
	{
	case GADGETTYPE_BUTTON_VOLUME:
		{
			if(EditData *edit=new EditData)
			{
				guiGadget_Volume *vol=(guiGadget_Volume *)g;

				// long position;
				edit->song=0;
				edit->win=this;

				edit->x=g->x2;
				edit->y=GetWindowMouseY();
				edit->width=g->x2-g->x;

				edit->title=Cxs[CXS_EDIT];
				edit->deletename=false;
				edit->id=-2;

				edit->type=EditData::EDITDATA_TYPE_DOUBLE;
				edit->doubledigits=1;
				edit->helpobject=g;

				edit->dfrom=mainaudio->ConvertFactorToDb(mainaudio->silencefactor);
				edit->dto=AUDIO_MAXDB;

				double h=vol->volume;
				h*=LOGVOLUME_SIZE;


#ifdef ARES64
				h=logvolume[(int)h];
#else
				h=logvolume_f[(int)h];
#endif

				edit->dvalue=mainaudio->ConvertFactorToDb(h);

				maingui->EditDataValue(edit);
			}
		}
		break;

	case GADGETTYPE_BUTTON_TEXT:
	case GADGETTYPE_BUTTON_IMAGE:
		{
			DoubleClickGadget(g);
		}
		break;

	case GADGETTYPE_TIME:
		g->DoubleClicked();
		break;
	}
}

void guiWindow::CheckGadget(
#ifdef WIN32
							HWND child,
#endif
							int code,
							int id
							)
{
	if(winmode!=WINDOWMODE_NORMAL)
		return; // Skip Init Messages ...

	guiGadget *g=glist.gadgets[id];

	if(g)
	{
#ifdef DEBUG
		if(child!=g->hWnd)
			maingui->MessageBoxError(0,"CheckGadget");
#endif

		if(g->ownergadget==true)
		{
			if(g->on==true && g->disabled==false)
			{
				switch(code)
				{
					/*
					case BN_PUSHED:
					{
					SetFocus(hWnd); // -> KeyFocus -> Window

					if(g->mode&MODE_PUSH)
					{
					g->pushed=true;

					g->DrawGadget();

					if(g->linkgadget)
					CheckGadget(g->linkgadget);
					else
					Gadget(g);
					}
					}
					break;

					case BN_UNPUSHED:
					g->pushed=false;
					break;
					*/

				case BN_CLICKED:
					{
						SetFocus(hWnd); // -> KeyFocus -> Window

						/*
						if(g->mode&MODE_PUSH)
						{
						g->pushed=true;
						g->DrawGadget();

						//	if(g->parent)
						//		g->parent->Blt(o);

						Gadget(g);
						}
						else
						*/
						switch(g->type)
						{
						case GADGETTYPE_BUTTON_TEXT:
						case GADGETTYPE_BUTTON_IMAGE:
							{
								if(g->linkgadget)
									CheckGadget(g->linkgadget);
								else
								{
									if((g->mode&MODE_GROUP) || (g->mode&MODE_AUTOTOGGLE))
									{
										if(g->mode&MODE_TOGGLED)
											g->mode CLEARBIT MODE_TOGGLED;
										else
											g->mode |= MODE_TOGGLED;

										g->DrawGadget();

										if(g->mode&MODE_GROUP)
											glist.GroupToggled(g);
									}

									Gadget(g);
								}
							}
							break;
						}
					}
					break;

				case BN_DOUBLECLICKED:
					{
						SetFocus(hWnd); // -> KeyFocus -> Window
						CheckGadget(g);
					}
					break;

				case BN_DISABLE:
					{
						int i=0;

					}
					break;


					//case 
				}
			}

			return;
		}

		switch(g->type)
		{
		case GADGETTYPE_INTEGER:
			{
				LRESULT length=SendMessage(g->hWnd,WM_GETTEXTLENGTH,0,0);

				if(char *s=new char[length+2])
				{
					SendMessage(g->hWnd,WM_GETTEXT,length+1,(LPARAM)s);

					// Convert Char To Integer

					char * pEnd;
					int l = strtol (s,&pEnd,0);

					if(l!=g->index){
						g->index=l;
						Gadget(g);
					}

					delete s;
				}
			}
			break;

		case GADGETTYPE_STRING:
			{
				//			TRACE ("Message GADGETTYPE_STRING \n");
				LRESULT length=SendMessage(g->hWnd,WM_GETTEXTLENGTH,0,0);

				if(char *s=new char[length+2])
				{
					SendMessage(g->hWnd,WM_GETTEXT,length+1,(LPARAM)s);

					if((!g->string) || (g->string && strcmp(g->string,s)!=0))
					{
						if(g->string)
							delete g->string;

						g->string=s;	
						Gadget(g);
					}
					else
						delete s;
				}
			}
			break;

		case GADGETTYPE_CYCLE:
			{
				// CB_ Combo
#ifdef WIN32
				LRESULT index= SendMessage(g->hWnd, CB_GETCURSEL, 0, 0);

				if((int)index!=g->index)
				{
					g->index=(int)index;
					Gadget(g);	
				}
#endif

				/*
				#ifdef WIN32
				SetFocus(hWnd);
				#endif
				*/
			}
			break;

		case GADGETTYPE_CHECKBOX:
			{
#ifdef WIN32
				LRESULT index= SendMessage(g->hWnd, (WPARAM)BM_GETCHECK, 0, 0);

				if(index==BST_CHECKED)
				{
					g->index=1;
					Gadget(g);
				}
				else
					if(index==BST_UNCHECKED)
					{
						g->index=0;
						Gadget(g);
					}
#endif
			}
			break;

		case GADGETTYPE_LISTBOX:
			{
				TRACE ("LB Code %d\n",code);

				g->doubleclick=false;

#ifdef WIN32
				switch(code)
				{
				case LBN_DBLCLK:
					{
						g->doubleclick=true;

						LRESULT index= SendMessage(g->hWnd, LB_GETCURSEL, 0, 0);
						g->index=(int)index;

						//etListBoxSelection(index);

						switch(g->index)
						{
						case LB_ERR:
							break;

						default:
							Gadget(g);
							break;
						}

					}
					break;

				case LBN_SELCHANGE:
					{
						// LB_ Listbox
						LRESULT index= SendMessage(g->hWnd, LB_GETCURSEL, 0, 0);
						g->index=(int)index;

						//etListBoxSelection(index);

						switch(g->index)
						{
						case LB_ERR:
							break;

						default:
							Gadget(g);
							break;
						}
					}
				}
#endif
			}	
			break;
		}
	}
}

void guiWindow::Clear()
{
	//bitmap.guiFillRectX0(0,width,height,COLOUR_BACKGROUND);
}

void GUI::MouseWheelCheck(int delta,guiWindow *win,guiGadget *db)
{
	// 1.  Check Gadgets

	guiWindow *w=FirstWindow();

	while(w)
	{
		{
			guiObject *o=w->guiobjects.FirstObject();
			while(o)
			{
				if(guiGadget *g=o->gadget)
				{
					if(g->mouseover==true)
					{
						g->MouseWheel(delta);
						return;
					}
				}

				o=o->NextObject();
			}

			for(int i=0;i<w->glist.gc;i++)
			{
				guiGadget *g=w->glist.gadgets[i];

				if(g->mouseover==true)
				{
					g->MouseWheel(delta);
					return;
				}
			}
		}

		w=w->NextWindow();
	}

	//2. Check Window
	if(win)
		win->MouseWheel(delta,db);
}

void guiWindow::EditDataValue(EditData *data)
{
	if(data->id<0)
	{
		if(guiGadget *g=(guiGadget *)data->helpobject)
		{
			g->DrawGadget();
			Gadget(g);
		}
	}
	else
		EditDataMessage(data);
}

bool guiWindow::CanbeClosed()
{
	//if(bindtoform)
	//	return false;

	if(parentwindow)
		return false;

	return true;
}

guiMenu *guiWindow::NewWindowMenu()
{
	menu=new guiMenu;

	menu->window=this;

	return menu;
}

void guiWindow::InitAutoVScroll()
{
	if(autovscroll==true)
	{
		int oldvpos=glist.scrollv_pos;

		// Gadgets Height
		int maxheight=glist.GetMaxHeight();

		TRACE ("VSCROLL Max %d Height %d\n",maxheight,height);

		if(maxheight<=height)
		{
			maxheight=0;
			glist.scrollv_pos=0;
		}
		else
		{
			if(glist.scrollv_pos+height>maxheight)
				glist.scrollv_pos=maxheight-height;

			maxheight-=height;
		}

		glist.scrollv_range=1;
		// glist.scrollv_pos=0;

		SCROLLINFO info;

		info.cbSize=sizeof(SCROLLINFO);

		info.fMask=SIF_POS|SIF_RANGE|SIF_PAGE;

		info.nMin=0;
		info.nMax=maxheight;
		info.nPage=4;
		info.nPos=glist.scrollv_pos;

		SetScrollInfo(hWnd,SB_VERT,&info,true);

		if(oldvpos!=glist.scrollv_pos)
			ScrollVert(glist.scrollv_pos-oldvpos);
	}
}

void guiWindow::ScrollVert(int deltay)
{
	if(autovscroll==false)
		return;

	for(int i=0;i<glist.gc;i++)
	{
		guiGadget *g=glist.gadgets[i];

		g->y-=deltay;
		g->y2-=deltay;

		SetWindowPos(g->hWnd,0,g->x,g->y,0,0,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE);

		if(g->group && (!(g->mode&MODE_TOGGLED)) )
		{
			// Skip Hidden Window/Group
			i++;
			g=glist.gadgets[i];
			g->y-=deltay;
			g->y2-=deltay;

			SetWindowPos(g->hWnd,0,g->x,g->y,0,0,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE);
		}
	}

	InvalidateRect(hWnd,0,TRUE);
	UpdateWindow(hWnd);

	for(int i=0;i<glist.gc;i++)
	{
		guiGadget *g=glist.gadgets[i];

		InvalidateRect(g->hWnd,0,FALSE);
		UpdateWindow(g->hWnd);

		if(g->group && (!(g->mode&MODE_TOGGLED)) )
		{
			// Skip Hidden Window/Group
			i++;
			g=glist.gadgets[i];

			InvalidateRect(g->hWnd,0,FALSE);
			UpdateWindow(g->hWnd);
		}
	}
}

guiWindow::guiWindow()
{
	id=OID_WINDOW;

	songmode=true;
	mouseclickdowncounter=50;

	overviewmode=OV_BOTH;
	exparentformx=-1;

	guitoolbox.editor=this;
	glist.win=this;
	vertscroll=horzscroll=false;

	resizeable=false;
	skipdeletepopmenu=false;

	editorid=-1;
	SetList(0);

	minzoomy=-1;
	rrt_slowcounter=0;

	refresfmenu=false;
	dontchangesettings=false;
	//close=false;
	closepopmenu=false;
	refreshgui=false;
	startposition=0;
	timeline=0;
	screen=0;
	winmode=WINDOWMODE_INIT;
	bottominfo=false;
	refreshed=false;
	ondesktop=false;
	hotkey=false;

#ifdef WIN32
	hWnd=0;
#endif

	width=0;
	height=0;
	left_mousekey=mid_mousekey=right_mousekey=MOUSEKEY_UP;
	editrefresh=false;
	menu=0;

	refreshmousebuttontimer=0;
	refreshmousebuttondown=false;
	editrefreshdone=false;
	
	mousedowngadget=0;
	mousepointer=CURSOR_NOTSET;

	autoscroll=false;

	zoom=&guizoom[mainsettings->defaultzoomx]; // default zoom X
	zoomy=mainsettings->defaultzoomy; // zoom Y
	zoomybuffermul=1;

	realtimewaitcounter=0;
	popmenu=0;
	song=0;

	refreshflag=0;
	blinkcounter=0;
	windowname=0;

	windowtimeformat=WINDOWDISPLAY_MEASURE; // Measure
	generalMIDIdisplay=GM_OFF;

	activenumberobject=0;
	editactivenumberobject_left=false;
	editorid=-1;

	ResetMenu();
	repeatkey=false;

	activenumberobject_flag=0;
	ignoreleftmouse=false;

	calledfrom=0;
	nVirtKey=-1;
	editorname=0;

	borderless=dialogstyle=false;
	autoscrollwin=0;
	editarea=editarea2=editarea3=0;
	parentformchild=0;
	hide=false;

	bindtoform=0;
	isstatic=false;
	parentwindow=0;

	mousexbuttonleftdown=mousexbuttonrightdown=false;
	xbuttoncounter=0;
	addtolastundo=false;
	openscreen=0;
	zoomvert=false;

	samplestartposition=sampleendposition=0;
	header=0;
	editornameisdeleteable=false;
	hasownmenu=false;
	learn=false;

	xmove=ymove=true;
	numbereditindex=-1;
	numbereditgadget=0;
	numbereditposition_sum=0;

	usemenuofwindow=0;
	mx=my=-1;

	datazoom=1; // 100%

	editobject=0;
	editsum=0;
}

guiWindow::~guiWindow()
{
	if(editornameisdeleteable==true && editorname)
	{
		delete editorname;
		editorname=0;
	}

	if(windowname)
	{
		delete windowname;
		windowname=0;
	}

	DeleteAllNumberObjects();
}


void guiWindow::Form_RepaintChild(guiForm_Child *child)
{
	glist.Resize(child->width,child->height,child);
}

void guiWindow::Form_FreeChildObjects(guiForm_Child *child)
{
	glist.FreeObjects(child);
}

void guiWindow::CreateButton(int id,HWND wnd)
{
	if(id<glist.gc) // Gadgets ?
	{
		//TRACE ("PN %d\n i = %d\n",id,lParam);
		guiGadget *g=glist.gadgets[id];
		g->hWnd=wnd;

		if(g->ownergadget==true || g->IsDoubleBuffered()==true)
		{
			if(g->GetWidth()>=0 && g->GetHeight()>=0)
			{
#ifdef DEBUG
				if(g->gbitmap.hBitmap)
					maingui->MessageBoxError(0,"Double hDC");
#endif
				// Init Doublebuffer
				g->ghDC=GetDC(g->hWnd);

				// Init Double Buffer
				g->gbitmap.hDC = CreateCompatibleDC(g->ghDC);

				if(g->mode&MODE_SPRITE)
				{
					g->spritebitmap.hDC= CreateCompatibleDC(g->ghDC);
					g->mixbitmap.hDC= CreateCompatibleDC(g->ghDC);
				}

				g->InitNewDoubleBuffer();
			}
		}
	}
}

void guiWindow::CreateTimeTypePopup(guiGadget_Time *gt)
{
	DeletePopUpMenu(true);

	if(popmenu)
	{
		if(gt)
		{
			class menu_selecttimemode:public guiMenu
			{
			public:
				menu_selecttimemode(guiGadget_Time *g,int t){gt=g;type=t;}

				void MenuFunction(){
					gt->ChangeType(type);
				} 

				guiGadget_Time *gt;
				int type;
			};

			popmenu->AddFMenu("Samples",new menu_selecttimemode(gt,WINDOWDISPLAY_SAMPLES),gt->ttype==WINDOWDISPLAY_SAMPLES?true:false);

			popmenu->AddFMenu(Cxs[CXS_MEASURE],new menu_selecttimemode(gt,WINDOWDISPLAY_MEASURE),gt->ttype==WINDOWDISPLAY_MEASURE?true:false);
			//	guilist->win->popmenu->AddFMenu("Frames",new menu_selecttimemode(this,WINDOWDISPLAY_SMPTE),ttype==WINDOWDISPLAY_SMPTE?true:false);

			if(WindowSong())
			{
				if(char *h=mainvar->GenerateString("Frames (",smpte_modestring[WindowSong()->project->standardsmpte],")"))
				{
					popmenu->AddFMenu(h,new menu_selecttimemode(gt,WINDOWDISPLAY_SMPTE),gt->ttype==WINDOWDISPLAY_SMPTE?true:false);
					delete h;
				}
			}
		}
		else
		{
			class menu_selecttimemode:public guiMenu
			{
			public:
				menu_selecttimemode(guiWindow *w,int t){win=w;type=t;}

				void MenuFunction(){
					if(win->windowtimeformat!=type)
					{
						win->windowtimeformat=type;
						win->NewTimeFormat();
					}
				} 

				guiWindow *win;
				int type;
			};

			popmenu->AddFMenu(Cxs[CXS_SECONDS],new menu_selecttimemode(this,WINDOWDISPLAY_SECONDS),windowtimeformat==WINDOWDISPLAY_SECONDS?true:false);

			popmenu->AddFMenu("Samples",new menu_selecttimemode(this,WINDOWDISPLAY_SAMPLES),windowtimeformat==WINDOWDISPLAY_SAMPLES?true:false);

			popmenu->AddFMenu(Cxs[CXS_MEASURE],new menu_selecttimemode(this,WINDOWDISPLAY_MEASURE),windowtimeformat==WINDOWDISPLAY_MEASURE?true:false);
			//	guilist->win->popmenu->AddFMenu("Frames",new menu_selecttimemode(this,WINDOWDISPLAY_SMPTE),ttype==WINDOWDISPLAY_SMPTE?true:false);

			if(WindowSong())
			{
				int s=WindowSong()->project->standardsmpte;

				if(char *h=mainvar->GenerateString("Frames (",smpte_modestring[WindowSong()->project->standardsmpte],")"))
				{
					popmenu->AddFMenu(h,new menu_selecttimemode(this,WINDOWDISPLAY_SMPTE),windowtimeformat==WINDOWDISPLAY_SMPTE?true:false);
					delete h;
				}
			}
		}

		TRACE ("GT RMB pop\n");
		ShowPopMenu();
	}
}

void guiWindow::Form_CreateFormObjects()
{
	//if(formcounter>1)
	{
		//int xp=0,yp=0;

		for(int x=0;x<forms_horz;x++)
		{
			for(int y=0;y<forms_vert;y++)
			{
				//	TRACE ("Create guiWindow Form Window \n");

				if(forms[x][y].deactivated==false /*&& (!(forms[x][y].dock&DOCK_NOWINDOW))*/ )
				{
					if(forms[x][y].flag&CHILD_HASWINDOW)
					{
						forms[x][y].fhWnd=0;
					}
					else
					{
						int flag=WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

						if(forms[x][y].dock&DOCK_SCROLLABLE)
						{
							flag|=WS_HSCROLL|WS_VSCROLL;
						}

						forms[x][y].fhWnd= 
							CreateWindowEx
							(
							0,
							forms[x][y].dock&DOCK_TOOLBARTOP?CAMX_TOOLBARTOP: CAMX_WINDOWNAME, 
							0,
							flag,
							forms[x][y].x, //x
							forms[x][y].y, //y
							forms[x][y].GetWidth(),
							forms[x][y].GetHeight(),  
							hWnd,
							(HMENU)9998, // Screen Menu Global
							maingui->hInst,
							NULL
							);
					}
				}
			}
		}
	}

	glist.InitForm();
}

#ifdef OLDIE
void guiWindow::Form_Repaint()
{
	TRACE ("Form_Repaint guiWindow \n");

	// Form -> Child Windows/Buttons
	for(int x=0;x<forms_horz;x++)
	{
		for(int y=0;y<forms_vert;y++)
		{
			Form_RepaintChild(GetForm(x,y));

			//if(fo->hWnd)
			//	MoveWindow(fo->hWnd,fo->x,fo->y,fo->width,fo->height,TRUE);

			//glist.Resize(fo->width,fo->height,fo);
		}
	}

	UpdateWindow(hWnd);
}
#endif

void guiWindow::OnCreate()
{	
	hInst=maingui->hInst;

	bitmap.hDC=GetDC(hWnd);
	bitmap.width=width;
	bitmap.height=height;

	bitmap.SetFont(&maingui->smallfont);
	bitmap.InitTextWidth();

	SetMouseCursor(CURSOR_STANDARD);

	Form_OnCreate();
	Init();

	ReDraw(true);
	//Form_NewSize(-1,-1,true);

	winmode=WINDOWMODE_NORMAL; // Reset
}

void guiWindow::OnNewSize(int w,int h)
{
	Form_NewSize(w,h,false);
	SetWindowDefaultSize();
}

int guiWindow::GetScreenMouseX()
{
	POINT lpPoint;
	GetCursorPos(&lpPoint);
	return lpPoint.x;
}

int guiWindow::GetScreenMouseY()
{
	POINT lpPoint;
	GetCursorPos(&lpPoint);
	return lpPoint.y;
}

int guiWindow::GetWindowMouseX()
{
	POINT lpPoint;

	GetCursorPos(&lpPoint);
	BOOL r=ScreenToClient(hWnd,&lpPoint);

	//TRACE ("MX %d\n",lpPoint.x);

	if(r==1)
		return lpPoint.x;

	return -1;
}

int guiWindow::GetWindowMouseY()
{
	POINT lpPoint;

	GetCursorPos(&lpPoint);
	BOOL r=ScreenToClient(hWnd,&lpPoint);

	return lpPoint.y;
}


void guiWindow::SetMouse(int x,int y)
{
	mx=x;
	my=y;
}

void guiWindow::DrawDBBlit(guiGadget_CW *db,guiObject *o)
{
	if(db && o)
	{
		//db->MixSprite(o);
		db->Blt(o);
	}
}

OSTART guiWindow::GetAutomationTime()
{
	return WindowSong()?WindowSong()->GetSongPosition():0;
}

bool guiWindow::IsWinEventEditor()
{
	switch(GetEditorID())
	{
	case EDITORTYPE_PIANO:
	case EDITORTYPE_EVENT:
	case EDITORTYPE_DRUM:
	case EDITORTYPE_TEXT:
	case EDITORTYPE_MARKER:
	case EDITORTYPE_SCORE:
	case EDITORTYPE_WAVE:
	case EDITORTYPE_ARRANGE:
	case EDITORTYPE_TEMPO:
		return true;
	}

	return false;
}

int guiWindow::GetSizeOfString(char *string)
{
	if(!string)
		return 0;

#ifdef WIN32
	size_t l=strlen(string);
	int length=0;
	int plen;

	for(size_t i=0;i<l;i++)
	{
		GetCharWidth32(
			bitmap.hDC,         // handle to device context
			string[i], // first character in range to query
			string[i],  // last character in range to query
			&plen   // pointer to buffer for widths
			);

		length+=plen;
	}

	return length;

#endif
	return 0;
}

int guiWindow::GetWinWidth()
{
#ifdef WIN32
	RECT r;

	GetClientRect(
		hWnd,      // handle to window
		&r   // address of structure for window coordinates
		);

	return r.right-r.left;
#endif
}

int guiWindow::GetWinHeight()
{
#ifdef WIN32
	RECT r;

	GetClientRect(
		hWnd,      // handle to window
		&r   // address of structure for window coordinates
		);

	return r.bottom-r.top;
#endif
}

int guiWindow::GetWinPosX()
{
	RECT r;

	GetWindowRect(
		hWnd,      // handle to window
		&r   // address of structure for window coordinates
		);

	return r.left;

	/*
	if(ondesktop==true)
	{
	RECT r;

	GetWindowRect(
	hWnd,      // handle to window
	&r   // address of structure for window coordinates
	);

	return r.left;
	}
	else
	if(screen)
	{
	int x=0;

	#ifdef WIN32
	RECT r;

	GetWindowRect(screen->screen.hWnd,&r);

	int sx=r.left;

	GetWindowRect(
	hWnd,      // handle to window
	&r   // address of structure for window coordinates
	);

	x=r.left-sx;
	#endif

	return x;
	}
	return 0;
	*/
}

int guiWindow::GetWinPosY()
{
	RECT r;

	GetWindowRect(
		hWnd,      // handle to window
		&r   // address of structure for window coordinates
		);

	return r.top;

	/*
	if(screen)
	{
	int y=0;

	#ifdef WIN32
	RECT r;
	GetWindowRect(screen->screen.hWnd,&r);

	int sy=r.top;

	GetWindowRect(
	hWnd,      // handle to window
	&r   // address of structure for window coordinates
	);

	y=r.top-sy;
	#endif

	return y;
	}
	return 0;
	*/
}

void guiWindow::InitOpenWindow(guiWindowSetting *set)
{
	if(set && set->calledfromwindow)
	{
		windowtimeformat=set->calledfromwindow->windowtimeformat;
	}
	else
		windowtimeformat=mainsettings->editordefaulttimeformat;

	if(WindowSong())
	{
		generalMIDIdisplay=WindowSong()->generalMIDI;
	}
	else
	{	
		generalMIDIdisplay=mainsettings->eventeditorgmmode;
	}

	switch(GetEditorID())
	{
	case EDITORTYPE_SAMPLE:
		{
			if(!set->calledfromwindow)
			{
				if(windowtimeformat==WINDOWDISPLAY_MEASURE)
					windowtimeformat=WINDOWDISPLAY_SAMPLES;
			}
		}
		break;
	}

	InitSpecialDisplay();
}

void guiWindow::guiSetWindowText(char *ntext)
{
#ifdef WIN32
	SetWindowText(hWnd,ntext);
#endif
}

void guiWindow::WindowToFront(bool activate)
{
	if(screen && parentformchild)
	{
		if(hide==true)
		{
			guiWindow *w=maingui->FirstWindow();
			while(w)
			{
				if(w!=this && w->hide==false && w->screen==screen && w->parentformchild==parentformchild/*w->hWnd==screen->forms[forms_horz][this->forms_vert].fhWnd*/ )
				{
					w->hide=true; // Stop Realtime Refresh etc..
#ifdef WIN32
					ShowWindow(w->hWnd,SW_HIDE);
					//UpdateWindow(win->parentformchild->hWnd);
#endif
					break;
				}
				w=w->NextWindow();
			}

			parentformchild->fhWnd=hWnd;
			hide=false;
			parentformchild->ChangeChild(); // Refresh Size etc...
		}

		return;
	}

#ifdef WIN32
	SetWindowPos(hWnd,
		HWND_TOPMOST,
		0,
		0,
		0,
		0,
		SWP_NOMOVE|SWP_NOSIZE
		);
#endif

}

void guiWindow::OwnerDrawGadgetSelect(int id,LPDRAWITEMSTRUCT pDIS)
{
#ifdef DEBUG
	if(id>=MAXGADGETSPERWINDOW)
		maingui->MessageBoxError(0,"OwnerDrawGadget Select");
#endif

	if(id>=MAXGADGETSPERWINDOW)
		return;

	guiGadget *gad=glist.gadgets[id];

	switch(gad->type)
	{
	case GADGETTYPE_TIME:
		{
			guiGadget_Time *gt=(guiGadget_Time *)gad;

			switch(gt->ttype)
			{
			case WINDOWDISPLAY_SMPTEOFFSET:
				{
					switch(pDIS->itemAction)
					{
					case ODA_SELECT:
						if(pDIS->itemState&ODS_SELECTED)
						{
							if(gad->getMouseMove==false)
							{
								int mx=gad->GetMouseX();

								for(int i=0;i<4;i++)
								{
									if(gad->subx[i]>=0)
									{
										if(mx>=gad->subx[i] && mx<=gad->subx2[i])
										{
											if(gad->CheckClicked()==true)
											{
												gad->startmousey=GetWindowMouseX();
												gad->startValue=gad->subvalue[i];
												gad->editIndex=i;
												gad->InitGetMouseMove();
												gad->DrawGadget();
											}

											return;
										}
									}
								}
							}
						}
						break;
					}
				}
				break;

			case WINDOWDISPLAY_MEASURE:
				{
					switch(pDIS->itemAction)
					{
					case ODA_SELECT:
						if(pDIS->itemState&ODS_SELECTED)
						{
							if(gad->getMouseMove==false)
							{
								int mx=gad->GetMouseX();

								for(int i=0;i<4;i++)
								{
									if(gad->subx[i]>=0)
									{
										if(mx>=gad->subx[i] && mx<=gad->subx2[i])
										{
											if(gad->CheckClicked()==true)
											{
												gad->startmousey=GetWindowMouseX();
												gad->startValue=gad->subvalue[i];
												gad->editIndex=i;
												gad->InitGetMouseMove();
												gad->DrawGadget();
											}

											return;
										}
									}
								}
							}
						}
						break;
					}
				}
				break;

			case WINDOWDISPLAY_SAMPLES:
				{
					switch(pDIS->itemAction)
					{
					case ODA_SELECT:

						if(pDIS->itemState&ODS_SELECTED)
						{
							if(gad->getMouseMove==false)
							{
								int mx=gad->GetMouseX();

								gad->startmousey=GetWindowMouseX();

								gad->editIndex=0;
								gad->InitGetMouseMove();
								gad->DrawGadget();
							}
						}
						break;
					}
				}
				break;

			case WINDOWDISPLAY_SMPTE:
				{
					switch(pDIS->itemAction)
					{
					case ODA_SELECT:

						if(pDIS->itemState&ODS_SELECTED)
						{
							if(gad->getMouseMove==false)
							{
								int mx=gad->GetMouseX();

								// 1.1.1.1
								for(int i=0;i<5;i++)
								{
									if(gad->subx[i]>=0)
									{
										if(mx>=gad->subx[i] && mx<=gad->subx2[i])
										{
											if(gad->CheckClicked()==true)
											{
												gad->startmousey=GetWindowMouseX();
												gad->startValue=gad->subvalue[i];
												gad->editIndex=i;
												gad->InitGetMouseMove();
												gad->DrawGadget();
											}

											return;
										}
									}
								}
							}
						}
						break;
					}

				}
				break;

			}
		}
		break;

	case GADGETTYPE_BUTTON_NUMBER:
	case GADGETTYPE_BUTTON_VOLUME:
		{
			switch(pDIS->itemAction)
			{
			case ODA_SELECT:
				if(pDIS->itemState&ODS_SELECTED)
				{
					if(gad->getMouseMove==false)
					{
						gad->InitGetMouseMove();
						gad->DrawGadget();
					}
				}
				break;
			}
		}
		break;
	}
}

void guiWindow::OwnerDrawGadget(int id,LPDRAWITEMSTRUCT pDIS)
{
#ifdef DEBUG
	if(id>=MAXGADGETSPERWINDOW)
		maingui->MessageBoxError(0,"OwnerDrawGadget");
#endif

	if(id>=MAXGADGETSPERWINDOW)
		return;

	guiGadget *gad=glist.gadgets[id];

	gad->init=false;
	gad->DrawGadget();
	//gad->Blt();
}

/*
void guiWindow::ShowPopMenu(guiGadget *g)
{
maingui->CloseAllWindows(EDITORTYPE_EDITDATA);

if(popmenu && g)
{
maingui->ConvertGUIMenuToOSPopMenu(popmenu);

GetWindowPositions(); // Get X+Y Positions on screen

#ifdef WIN32

int x=g->x2,
y=g->y;

x+=win_screenposx;
y+=win_screenposy;

TrackPopupMenuEx(
popmenu->OSMenuHandle,
0,      
x,y,
hWnd,
0  
);
#endif
}
}
*/

void guiWindow::ShowPopMenu()
{
//	maingui->CloseAllWindows(EDITORTYPE_EDITDATA);

	if(popmenu)
	{
		popmenu->popup=true;

		maingui->ConvertGUIMenuToOSPopMenu(popmenu);

		//	GetWindowPositions(); // Get X+Y Positions on screen

		int x=GetScreenMouseX(),y=GetScreenMouseY();

#ifdef WIN32
		if(popmenu->OSMenuHandle)
		{
			TRACE ("TrackPopupMenuEx \n");

			TrackPopupMenuEx(
				popmenu->OSMenuHandle,
				0,      
				x,y,
				hWnd, // Send To Win Message
				0  
				);
		}
		else
			TRACE ("Error TrackPopupMenuEx \n");
#endif

		TRACE ("Show PopMenu \n");
	}
	else
		TRACE ("No PopMenu !\n");
}


guiObject *guiWindow::AddObject()
{
	if(guiObject *ol=new guiObject)
	{
		objectslist.AddEndO(ol);
		return ol;
	}

	return 0;
}

void guiWindow::AddNumberOList(NumberOList *o,guiGadget *g)
{
	if(NumberOListRef *po=new NumberOListRef(o,g))
		numberobjectslistref.AddEndO(po);
}

void guiWindow::DeleteAllNumberObjects()
{
	numberobjectslistref.DeleteAllO();
	activenumberobject=0;
}

void guiWindow::DeleteAllNumberObjects(guiGadget *g)
{
	NumberOListRef *r=FirstNumberOListRef();

	while(r)
	{
		NumberOListRef *n=(NumberOListRef *)r->next;

		if(r->gadget==g)
			numberobjectslistref.RemoveO(r);

		r=n;
	}
}

void guiWindow::StartPositionEditing(NumberOListStartPosition *sp,int index,guiGadget *g)
{
	if(g && index>=0 && index<sp->timestring.index)
	{
		sp->timestring.pos.Clone(&numbereditpos);

		numbereditindex=index;
		numbereditposition=sp->timestring.time;
		numbereditsamples=sp->timestring.pos.pos[0];

		numbereditposition_diff=0;
		numbereditposition_sum=0;

		numberedited=false;
		numbereditgadget=g;

		editobject=sp->posobj;

		g->editmousey=g->GetMouseY();

		//if(numbereditobject)
		{
			StartOfNumberEdit(g); // virtual
			
		}
	}
}

bool guiWindow::IsEndOfPositionEditing(guiGadget *g)
{
	if(numbereditindex!=-1)
	{
		numbereditindex=-1;

		OSTART diff=numbereditposition_sum;

		numbereditposition_sum=0; // Reset
		numberedited=false;
		numbereditgadget=0;

		EndOfPositionEdit(g,diff); // virtual

		return true;
	}

	return false;
}

bool guiWindow::EditPositions(int deltay)
{
	if(numbereditgadget && numbereditindex!=-1)
	{
		if(deltay)
		{
			if(numbereditpos.IsSmpte()==true || numbereditpos.mode==Seq_Pos::POSMODE_TIME)
			{
				if(WindowSong())
				{
					if(numbereditpos.IsSmpte()==true)
					{
						numbereditpos.song=WindowSong();
						numbereditpos.offset=0;
						WindowSong()->timetrack.ConvertTicksToPos(numbereditposition,&numbereditpos);
					}

					switch(numbereditindex)
					{
					case 0:
						{
							if(deltay>1)deltay=1;
							if(deltay<-1)deltay=-1;
							numbereditpos.AddHour(deltay);
						}
						break;

					case 1:
						{
							if(deltay>1)deltay=1;
							if(deltay<-1)deltay=-1;
							numbereditpos.AddMin(deltay);
						}
						break;

					case 2:
						{
							if(deltay>1)deltay=1;
							if(deltay<-1)deltay=-1;
							numbereditpos.AddSec(deltay);
						}
						break;

					case 3:
						{
							if(deltay>1)deltay=1;
							if(deltay<-1)deltay=-1;

							if(numbereditpos.IsSmpte()==true)
								numbereditpos.AddFrame(deltay);
							else
								numbereditpos.AddSec100(deltay);

						}
						break;

					case 4:
						{
							if(deltay>1)deltay=1;
							if(deltay<-1)deltay=-1;
							numbereditpos.AddQuarterFrame(deltay);
						}
						break;
					}

					OSTART vtime=WindowSong()->timetrack.ConvertPosToTicks(&numbereditpos);

					TRACE ("FPS %d\n",vtime);

					if(vtime!=numbereditposition && vtime>=0)
					{
						numbereditposition_diff=vtime-numbereditposition;
						numbereditposition=vtime;
						numbereditposition_sum+=numbereditposition_diff;

						EditEditorPositions(numbereditgadget);
					}
				}
			}
			else
				switch(numbereditpos.mode)
			{
				case Seq_Pos::POSMODE_SAMPLES:
					if(Seq_Song *song=WindowSong())
					{
						numbereditsamples+=deltay;

						OSTART nt=song->timetrack.ConvertSamplesToTicks(numbereditsamples);

						TRACE ("S %d\n",nt);

						if(nt!=numbereditposition && nt>=0)
						{
							numbereditposition_diff=nt-numbereditposition;
							numbereditposition=nt;
							numbereditposition_sum+=numbereditposition_diff;

							EditEditorPositions(numbereditgadget);
						}
					}
					break;

				case Seq_Pos::POSMODE_NORMAL:
				case Seq_Pos::POSMODE_COMPRESS:
					{
						if(WindowSong())
						{
							WindowSong()->timetrack.ConvertTicksToPos(numbereditposition,&numbereditpos);

							if(numbereditpos.index==3 && numbereditindex==2)
								numbereditindex=3; // 1.1.5, no zoom

							switch(numbereditindex)
							{
							case 0:
								{
									if(deltay>1)deltay=1;
									if(deltay<-1)deltay=-1;
									numbereditpos.AddMeasure(deltay);
								}
								break;

							case 1:
								{
									if(deltay>1)deltay=1;
									if(deltay<-1)deltay=-1;
									numbereditpos.AddBeat(deltay);
								}
								break;

							case 2:
								{
									if(deltay>1)deltay=1;
									if(deltay<-1)deltay=-1;

									numbereditpos.AddZoomTicks(deltay);
								}
								break;

							case 3:
								{
									if(deltay>1)deltay=1;
									if(deltay<-1)deltay=-1;

									numbereditpos.AddTicks(deltay);
								}
								break;
							}

							OSTART vtime=WindowSong()->timetrack.ConvertPosToTicks(&numbereditpos);

							TRACE ("M %d\n",vtime);

							if(vtime!=numbereditposition && vtime>=0)
							{
								numbereditposition_diff=vtime-numbereditposition;
								numbereditposition=vtime;
								numbereditposition_sum+=numbereditposition_diff;

								EditEditorPositions(numbereditgadget);
							}
						}
					}
					break;
			}
		}

		return true;
	}

	return false;
}

bool guiWindow::IsPositionObjectClicked(guiGadget *g,bool leftmouse)
{
	if(!g)
		return false;

	// Check Mouse Over
	if(leftmouse==true && IsFocusWindow()==true)
	{
		int mx=g->GetMouseX(),my=g->GetMouseY();

		NumberOListRef *r=FirstNumberOListRef();

		while(r)
		{
			if(r->gadget==g)
			{
				NumberOList *ol=r->number;

				switch(ol->type)
				{ 
				case OL_START:
					{
						NumberOListStartPosition *sp=(NumberOListStartPosition *)ol;

						for(int i=0;i<sp->timestring.index;i++)
						{
							if(mx>=sp->time[i].x && mx<=sp->time[i].x2 && my>=sp->time[i].y && my<=sp->time[i].y2)
							{
								StartPositionEditing(sp,i,g);
								
								return true;
							}

						}
					}
					break;
				}
			}

			r=(NumberOListRef *)r->next;
		}
	}

	return false;
}

void guiWindow::CheckNumberObjects(guiGadget *g)
{
	if(!g)
		return;

	// Check Mouse Over
	if(IsFocusWindow()==true && maingui->GetLeftMouseButton()==false)
	{
		int mx=g->GetMouseX();
		int my=g->GetMouseY();

		bool blit=false;

		NumberOListRef *r=FirstNumberOListRef();

		while(r)
		{
			if(r->gadget==g)
			{
				NumberOList *ol=r->number;

				switch(ol->type)
				{ 
				case OL_START:
					{
						NumberOListStartPosition *sp=(NumberOListStartPosition *)ol;

						for(int i=0;i<sp->timestring.index;i++)
						{
							if(mx>=sp->time[i].x && mx<=sp->time[i].x2 && my>=sp->time[i].y && my<=sp->time[i].y2)
							{
								if(sp->mouseover[i]==false && maingui->GetLeftMouseButton()==false && maingui->GetRightMouseButton()==false)
								{
									sp->mouseover[i]=true;
									blit=true;
									g->gbitmap.guiInvert(sp->time[i].x,sp->time[i].y,sp->time[i].x2,sp->time[i].y2);
								}
							}
							else
								if(sp->mouseover[i]==true)
								{
									sp->mouseover[i]=false;
									blit=true;
									g->gbitmap.guiInvert(sp->time[i].x,sp->time[i].y,sp->time[i].x2,sp->time[i].y2);
								}
						}
					}
					break;

				}
			}

			r=(NumberOListRef *)r->next;
		}

		if(blit==true)
			g->Blt();
	}
}

bool guiWindow::SetZoomGFX(int z,bool horiz)
{
	if(horiz==true)
	{
		// buffer old mid
		if(songmode==true && timeline)
			timeline->BufferOldMidPosition();

		// X
		if(z>=0 && z<mainvar->numberwinzooms && zoom!=&guizoom[z])
		{
			mainsettings->defaultzoomx=z;
			zoom=&guizoom[z];
			return true;
		}

		return false;
	}

	// Y
	if(z>=ZOOMY_MAX)z=ZOOMY_MAX;
	else
		if(z<=ZOOMY_MIN)z=ZOOMY_MIN;

	if(zoomy!=z)
	{
		mainsettings->defaultzoomy=zoomy=z;
		return true;
	}

	return false;
}

void guiWindow::RemoveMenu()
{
	if(menu)
	{
		menu->RemoveMenu();
		menu=0;
	}
}

void guiWindow::GenerateOSMenu(bool drawmenu)
{
	/*
	HMENU oldhandle=menu?menu->OSMenuHandle:0;

	ResetMenu();
	CreateMenu();

	if(menu)
	{
	maingui->ConvertGUIMenuToOSMenu(this,menu);

	#ifdef WIN32
	if(mdimode==false)
	{
	SetMenu(hWnd,menu->OSMenuHandle);
	}
	else
	{
	// MDI
	if(activemdiwindow==true)
	{
	SendMessage(screen->hWnd, 
	WM_MDISETMENU,
	(WPARAM) menu->OSMenuHandle, 
	(LPARAM) screen->menu->OSMenuHandle);

	DrawMenuBar(screen->hWndClient);
	}
	}
	#endif
	}

	if(oldhandle)
	DestroyMenu(oldhandle);
	*/
}

void guiWindow::RefreshMenu()
{
	GenerateOSMenu(true);
}

bool guiWindow::IsActiveWindow()
{
	HWND hcwnd=GetFocus();

	while(hcwnd)
	{
		if(hcwnd==hWnd)
			return true;

		hcwnd=GetParent(hcwnd);
	}

	return false;
}

bool guiWindow::IsFocusWindow()
{
	HWND hfocus=GetFocus();
	while(hfocus)
	{
		if(HWND pfocus=GetParent(hfocus))
			hfocus=pfocus;
		else
			break;
	}

	HWND hcwnd=hWnd;

	while(hcwnd)
	{
		if(hcwnd==hfocus)
			return true;

		hcwnd=GetParent(hcwnd);
	}

	return false;
}

void guiWindow::SetWindowName()
{
	if(windowname)
	{
		delete windowname;
		windowname=0;
	}

	if(!editorname)return;

	if(WindowSong())
	{
		if(char *h=WindowSong()->CreateWindowTitle())
		{
			windowname=mainvar->GenerateString(editorname," ",h);
			delete h;
		}
	}
	else
		windowname=mainvar->GenerateString(editorname);
}

void guiWindow::SongNameRefresh()
{
	SetWindowName();
	guiSetWindowText(GetWindowName());
} //v


void guiWindow::ResetRefresh()
{
		refreshmousebuttontimer=0;
		refreshmousebuttondown=false;
}

void guiWindow::SetMouseCursor(int type)
{
	mousepointer=type;

#ifdef WIN32
	HCURSOR hc=0;

	switch(type)
	{
	case CURSOR_UP:
		hc=LoadCursor(0,IDC_SIZENS);
		break;

	case CURSOR_DOWN:
		hc=LoadCursor(0,IDC_SIZENS);
		break;

	case CURSOR_LEFT:
		hc=LoadCursor(0,IDC_SIZEWE);
		break;

	case CURSOR_UPDOWN:
		hc=LoadCursor(0,IDC_SIZENS);
		break;

	case CURSOR_LEFTRIGHT:
		hc=LoadCursor(0,IDC_SIZEWE);
		break;

	case CURSOR_RIGHT:
		hc=LoadCursor(0,IDC_SIZEWE);
		break;

	case CURSOR_NOTSET:
	case CURSOR_STANDARD:
		hc=LoadCursor(0,IDC_ARROW);
		break;

	case CURSOR_SIZE4D:
		hc=LoadCursor(0,IDC_SIZEALL);
		break;

	case CURSOR_HAND:
		hc=LoadCursor(0,IDC_HAND);
		break;
	}

	if(hc)
		SetCursor(hc);
#endif
}
