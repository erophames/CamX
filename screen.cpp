#include "gui.h"
#include "transporteditor.h"
#include "songmain.h"
#include "object_song.h"
#include "object_project.h"
#include "editfunctions.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "languagefiles.h"

guiScreen::guiScreen()
{
	id=OID_SCREEN;

	minwidth=maingui->GetButtonSizeY(24);
	minheight=maingui->GetButtonSizeY(20);

	for(int ix=0;ix<MAXFORMCHILDS;ix++)
		for(int iy=0;iy<MAXFORMCHILDS;iy++)
			forms[ix][iy].screen=this;

	InitForms(FORM_SCREEN);

	//x=0;
	//y=0;
	width=maingui->GetButtonSizeY(75);
	height=maingui->GetButtonSizeY(70);

	open=false;
	menu=0;
	project=0;
	song=0;
	screenname=0;
	resizeable=true;
	maximized=true;
}

void guiScreen::ODeInit()
{
	if(screenname)
		delete screenname;
}

void guiScreen::OnCreate()
{
	Form_OnCreate();

#ifdef WIN32
	HWND desktopwin=GetDesktopWindow();

	RECT rect;

	GetClientRect(desktopwin,&rect);

	maximumwidth=rect.right;
	maximumheight=rect.bottom;
#endif
}

void guiScreen::WindowClosed(guiWindow *win)
{
	if(win && win->screen==this && win->parentformchild && (!win->bindtoform))
	{
		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			if(w->screen==this && w->parentformchild==win->parentformchild)
			{
				w->parentformchild->fhWnd=w->hWnd;
				w->parentformchild->forceredraw=true;

				//if(maingui->dontrecalcscreens==false)
				ReDraw(false);
				//else
				//	recalcscreen=true;

				return;
			}

			w=w->NextWindow();
		}

		// closeit:
		// Disable Form
		win->parentformchild->fhWnd=0;

		//if(maingui->dontrecalcscreens==false)
		win->parentformchild->Enable(false);

		/*
		else
		{
		win->parentformchild->disablewithrecalc=true;
		recalcscreen=true;
		}
		*/

		//ReDraw(true);
	}
}

Edit_Arrange *guiScreen::GetArrange()
{
	guiWindow *w=maingui->FirstWindow();
	while(w)
	{
		if(w->screen==this && w->GetEditorID()==EDITORTYPE_ARRANGE)
			return (Edit_Arrange *)w;

		w=w->NextWindow();
	}

	return 0;
}

Edit_Transport *guiScreen::GetTransport()
{
	guiWindow *w=maingui->FirstWindow();
	while(w)
	{
		if(w->screen==this && w->GetEditorID()==EDITORTYPE_TRANSPORT)
			return (Edit_Transport *)w;

		w=w->NextWindow();
	}

	return 0;
}

char *guiScreen::CreateScreenName()
{
	char *f1=0;

#ifdef WIN64

#ifdef ARES64
	f1="CamX Alpha UI2 X64-64Bit Audio";
#endif

#ifndef ARES64
	f1="CamX Alpha UI2 X64-32Bit Audio"; 
#endif

#else

#ifdef ARES64

#ifdef NOSSE2
	f1="CamX Alpha UI2/DT x86 64Bit/SSE1 Audio"; 
#else
	f1="CamX Alpha UI2/DT x86 64Bit/SSE2 Audio";
#endif

#endif

#ifndef ARES64

#ifdef NOSSE2
	f1="CamX Alpha UI2/DT x86 32Bit/SSE1 Audio";
#else
	f1="CamX Alpha UI2/DT x86 32Bit/SSE2 Audio";
#endif
#endif

#endif

	if(f1)
	{
		char hs[NUMBERSTRINGLEN];
		char *f2=mainvar->GenerateString(f1," | CPU Cores:",mainvar->ConvertIntToChar(mainvar->cpucores,hs)," | ");

		if(f2)
			f1=f2;

		char *devicename=0;

		if(mainaudio->GetActiveDevice())
		{
			// 1. Device Type ASIO etc..

			devicename=mainvar->GenerateString("*",mainaudio->GetActiveDevice()->initname,"(",mainaudio->GetActiveDevice()->devicetypname,")");
		}
		else
			devicename=mainvar->GenerateString("*",Cxs[CXS_NOAUDIODEVICE]);

		if(project && song)
		{
			char *h=mainvar->GenerateString(mainvar->GetActiveSong()==song?"*Song:":"Song:",song->GetName(),mainvar->GetActiveProject()==project?" *Project:":" Project:",project->name," ",f1,devicename);

			delete devicename;

			if(h)
			{
				if(screenname)
					delete screenname;

				screenname=h;

				return h;
			}
		}
		else
			if(project)
			{
				char *h=mainvar->GenerateString(mainvar->GetActiveProject()==project?" *Project:":" Project:",project->name," ",f1,devicename);

				delete devicename;

				if(h)
				{
					if(screenname)
						delete screenname;

					screenname=h;

					return h;
				}
			}
			else
			{

				if(screenname)
					delete screenname;

				screenname=mainvar->GenerateString(f1,Cxs[CXS_NOPROJECTSONG],devicename);

				delete devicename;

				return screenname;
			}

			if(f2)
				delete f2;
	}

	return 0;
}

Seq_Song *guiScreen::GetSong()
{
	return song;
}

guiWindow *guiScreen::FindWindow(int wid)
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(w->screen==this && w->GetEditorID()==wid)
			return w;

		w=w->NextWindow();
	}

	return 0;
}

bool guiScreen::Mixer()
{
	guiWindow *ontop=maingui->FirstWindow();

	guiWindow *w=maingui->FirstWindow();
	while(w)
	{
		if(w->screen==this && w->parentformchild && w->hide==false)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_AUDIOMIXER:
				{
					if(w->NextWindow())
						ontop=w->NextWindow();

					goto check;
				}
				break;
			}
		}

		w=w->NextWindow();
	}

check:

	for(int i=0;i<2;i++)
	{
		w=i==0?ontop:maingui->FirstWindow();

		while(w)
		{
			if(w->screen==this && w->parentformchild && w->hide==true)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_AUDIOMIXER:
					{
						w->WindowToFront(true);
						return true;
					}
					break;
				}
			}

			w=w->NextWindow();
		}

		if(ontop==maingui->FirstWindow())
			break;
	}

	return false;
}

guiWindow *guiScreen::Editor(int id)
{
	guiWindow *ontop=maingui->FirstWindow(),*w=maingui->FirstWindow();

	while(w)
	{
		if(w->screen==this && w->parentformchild && w->hide==false)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_TEMPO:
			case EDITORTYPE_SAMPLE:
				{
					if(w->NextWindow())
						ontop=w->NextWindow();

					goto check;
				}
				break;
			}
		}

		w=w->NextWindow();
	}

check:

	for(int i=0;i<2;i++)
	{
		w=i==0?ontop:maingui->FirstWindow();

		while(w)
		{
			if(w->screen==this && w->parentformchild && w->hide==true)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_EVENT:
				case EDITORTYPE_PIANO:
				case EDITORTYPE_DRUM:
				case EDITORTYPE_WAVE:
				case EDITORTYPE_TEMPO:
				case EDITORTYPE_SAMPLE:
					{
						w->WindowToFront(true);
						return w;
					}
					break;
				}
			}

			w=w->NextWindow();
		}

		if(ontop==maingui->FirstWindow())
			break;
	}

	return 0;
}

bool guiScreen::OnNewSize(int w,int h)
{
	/*
	if(flag&SCREEN_MINIMIZED)
	{
		widthbeforeminimized=width;
		heightbeforeminimized=height;
	}
*/

	Form_NewSize(w,h,false);

#ifdef WIN32
	//UpdateWindow(hWnd);
#endif

	//header.NewSize(w,h);
	return true;
}

void guiScreen::SetNewSong(Seq_Song *so)
{
	if(so)
	{
		project=so->project;
		InitNewSong(so,true);
	}
	else
	{
		song=0;

		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			if(w->screen==this)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_TRANSPORT:
					{
						Edit_Transport *t=(Edit_Transport *)w;
						t->RefreshSong(0);

					}break;
				}
			}
			w=w->NextWindow();
		}
	}
}

guiScreen *GUI::OpenNewScreen(Seq_Project *project,Seq_Song *song)
{
	// Open new Song Screen
	if(guiScreen *ns=new guiScreen)
	{
		AddScreen(ns);
		OpenScreen(ns,false);

		ns->project=project;
		ns->SetNewSong(song);

		RefreshScreenNames();
		return ns;
	}

	return 0;
}

void GUI::RefreshProjectScreens(Seq_Project *p)
{
	guiScreen *s=FirstScreen();
	while(s)
	{
		if(s->project==p || p==0)
		{
			s->RefreshMenu();
			s->SetTitle();
		}

		s=s->NextScreen();
	}
}

void guiScreen::SetNewProject(Seq_Project *p,Seq_Song *s)
{
	project=p;
	song=s;

}

void guiScreen::InitNewSong(Seq_Song *newsong,bool calledbytransport)
{
	if(!song)
	{
		guiWindowSetting settings;
		settings.screen=this;

		if(newsong)
			project=newsong->project;

		song=newsong;

		bool foundtrans=false;
		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			if(w->screen==this)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_TRANSPORT:
					{
						Edit_Transport *t=(Edit_Transport *)w;
						t->RefreshSong(newsong);
						foundtrans=true;

					}break;
				}
			}
			w=w->NextWindow();
		}

		if(foundtrans==false)
			maingui->OpenEditorStart(EDITORTYPE_TRANSPORT,newsong,0,0,&settings,0,0);

		maingui->OpenEditorStart(EDITORTYPE_ARRANGE,newsong,0,0,&settings,0,0); // Always 1. !!!
		maingui->OpenEditorStart(EDITORTYPE_AUDIOMIXER,newsong,0,0,&settings,0,0);	
	}
	else
	{
		maingui->OpenNewScreen(project,newsong);
	}

	maingui->RefreshProjectScreens(project);

//	FormEnable(0,2,false);
}

guiWindow *GUI::WindowToFront(Seq_Song *song,int editorid)
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->WindowSong()==song)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_TEMPO:
				{
					w->WindowToFront(0);
					return w;
				}
				break;
			}
		}

		w=w->NextWindow();
	}

	return 0;
}

/*
void GUI::AdjustWindowPositionsToDesktop(RECT *wrect)
{
	if(HWND desk=GetDesktopWindow())
	{
		int height=wrect->bottom-wrect->top;
		int width=wrect->right-wrect->left;

		RECT screenrect;
		GetWindowRect(desk,&screenrect);

		if(wrect->bottom>screenrect.bottom)
			wrect->top=0;
		
		if(wrect->right>screenrect.right)
			wrect->left=0;

		wrect->right=wrect->left+width;
		wrect->bottom=wrect->top+height;
	}
}
*/

void GUI::CloseSong(guiScreen *screen,bool remove)
{
	if(!screen)
		return;

	Seq_Project *project=screen->project;
	Seq_Song *song=screen->song;
	if(!song)
		return;

	if(remove==true)
	{
		if(char *h=mainvar->GenerateString(Cxs[CXS_DELETESONG_Q],"\n","Song:",song->GetName()))
		{
			if(maingui->MessageBoxYesNo(0,h)==true)
			{
				if(maingui->MessageBoxYesNo(0,h)==true)
				{
					mainedit->DeleteSong(screen,song,EditFunctions::DELETESONG_FLAG_DELETEFILES|EditFunctions::DELETESONG_FLAG_SETNEXTSONGAUTO);
					RefreshProjectScreens(project);
				}
			}

			delete h;
		}
	}
	else
		if(char *h=mainvar->GenerateString(Cxs[CXS_CLOSESONG_Q],"\n","Song:",song->GetName()))
		{
			if(maingui->MessageBoxYesNo(0,h)==true)
			{
				mainedit->DeleteSong(screen,song,0);
				RefreshProjectScreens(project);
			}
			delete h;
		}
}

void guiScreen::SetTitle(char *t)
{
#ifdef WIN32
	SetWindowText(hWnd,t?t:CreateScreenName());
#endif
}

void guiScreen::RefreshMenu()
{
	maingui->CreateScreenMenu(this,0);
	if(menu)
		SetMenu(hWnd,menu->OSMenuHandle);

	maingui->RefreshScreenNames();
}
