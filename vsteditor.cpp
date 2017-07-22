#include "defines.h"

#include "audioobjects.h"
#include "vstguiwindow.h"
#include "vstplugins.h"
#include "audioeffects.h"
#include "audiohardware.h"
#include "camxgadgets.h"
#include "songmain.h"
#include "editdata.h"
#include "languagefiles.h"
#include "audiochannel.h"
#include "object_track.h"
#include "camxfile.h"
#include "gui.h"

void Edit_Plugin_VST::Gadget(guiGadget *g)
{
	g=PluginGadget(g);

	if(!g)
		return;
}

Edit_Plugin_VST::Edit_Plugin_VST(VSTPlugin *vst,InsertAudioEffect *ieffect)
{
	editorid=EDITORTYPE_PLUGIN_VSTEDITOR;
	InitForms(FORM_PLUGIN);
	insertaudioeffect=ieffect;
	vstplugin=vst;

	if(vstplugin)
	{
		resizeable=vstplugin->GetOwnEditor()==true?false:true;
	}
	else
	{
		resizeable=false;
	}

	vstguiwindow=0;
}

void Edit_Plugin_VST::RefreshRealtime_Slow()
{
	PluginRefreshRealtime_Slow();
}

void Edit_Plugin_VST::RefreshRealtime()
{
	PluginRefreshRealtime();

	if(vstguiwindow && vstplugin->crashed==0)
	{
		if(vstplugin->refreshwindowsize==true)
		{
			vstplugin->LockRefreshSize();
			RefreshWindowSize();
			vstplugin->refreshwindowsize=false;
			vstplugin->UnlockRefreshSize();
		}

		// Refresh Plugin GUI
		
		#ifdef TRYCATCH
		try
#endif
		{
			vstplugin->ptrPlug->dispatcher(vstplugin->ptrPlug,effEditIdle,0,0,0,0.0f);
		}

		#ifdef TRYCATCH
		catch(...)
		{
			vstplugin->crashed=VSTCRASH_EditIdle;
		}
#endif
	}

	return;

	if(vstplugin->crashed)return;

	/*
	// Plugin idle call
	void tVSTIdle()
	{
	MSG msg;
	HWND hwnd = GetWindow(t_hwnd, GW_CHILD); // Get child (plugin) window
	while ((PeekMessage(&msg, hwnd, WM_TIMER, WM_TIMER, PM_NOREMOVE)) ||
	(PeekMessage(&msg, hwnd, WM_PAINT, WM_PAINT, PM_NOREMOVE)) ||
	(PeekMessage(&msg, hwnd, WM_NCPAINT, WM_NCPAINT, PM_NOREMOVE)))
	{
	if (GetMessage(&msg, NULL, 0, 0))
	{
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	}
	} 
	*/




	//TRACE ("effEditIdle %d\n",rt);
}


void Edit_Plugin_VST::RemoveChildWindows()
{
	if(vstguiwindow)
	{
		vstplugin->CloseGUI();

		DestroyWindow(vstguiwindow);
		vstguiwindow=0;
	}

	CloseListEditor();
	
}

void Edit_Plugin_VST::RefreshWindowSize()
{
	RECT winRect;
	int newwidth=vstplugin->newwidth;
	int newheight=vstplugin->newheight+PLUGINTOPBARHEIGHT;

	winRect.left=0;
	winRect.top=0;
	winRect.right=newwidth;
	winRect.bottom=newheight;

	AdjustWindowRectEx(&winRect,style,FALSE,flagex);
	
	SetWindowPos (hWnd,0,winRect.left,winRect.top,winRect.right-winRect.left,winRect.bottom-winRect.top,SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOOWNERZORDER);

	if(vstguiwindow)
		SetWindowPos (vstguiwindow,0,0,0,vstplugin->newwidth,vstplugin->newheight,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOOWNERZORDER);
}


void Edit_Plugin_VST::InitPluginParameterEditor()
{
	RemoveChildWindows();

	// Open VST win inside window
	if(vstplugin->GetOwnEditor()==true && mode==PIN_EDITOR)
	{
#ifdef DEBUG
		if(vstguiwindow)
			maingui->MessageBoxError(0,"2x InitPluginEditor");
#endif

		guiForm_Child *c=glist.SelectForm(0,1);

		vstplugin->guiwidth=0;
		vstplugin->guiheight=0;

		vstplugin->InitOwnEditor(&vstplugin->guiwidth,&vstplugin->guiheight);

		int sx=c->x;
		int flag=WS_CHILD|WS_CLIPSIBLINGS;
		int flagex=WS_EX_NOPARENTNOTIFY|WS_EX_ACCEPTFILES;

		// WS_EX_NOPARENTNOTIFY NEEDED ! ELSE Gadget/DB Problems

		RECT wRect;
		SetRect (&wRect, 0, 0, vstplugin->guiwidth-1, vstplugin->guiheight-1);
		AdjustWindowRectEx (&wRect,flag , FALSE,flagex);

		vstplugin->guiwidth=(wRect.right-wRect.left)+1;
		vstplugin->guiheight=(wRect.bottom-wRect.top)+1;

		vstguiwindow=CreateWindowEx
			(
			flagex,
			"VST Frame", 
			0,
			flag,
			vstplugin->vstguistartx=sx,
			c->y,
			vstplugin->guiwidth,
			vstplugin->guiheight,
			hWnd,
			(HMENU)0,
			hInst,
			this
			);

		if(vstguiwindow)
		{	
			vstplugin->ptrPlug->dispatcher(vstplugin->ptrPlug,effEditOpen,0,0,(void *)vstguiwindow,0.0f);
			//SetTimer(vstguiwindow, NULL,USER_TIMER_MINIMUM ,NULL);
			ShowWindow(vstguiwindow,SW_SHOWNA);
		}

	}
	else
	{
		InitPlugInList();
	}
	}

void Edit_Plugin_VST::Init()
{
	InitPluginEditor();
	InitPluginParameterEditor();
	ShowMode();
}

bool Edit_Plugin_VST::CheckIfObjectInside(Object *o)
{
	if(o==insertaudioeffect ||
		o==vstplugin ||
		o==(Object *)insertaudioeffect->effectlist->channel ||
		o==(Object *)insertaudioeffect->effectlist->track
		)
		return true;

	return false;
}


void Edit_Plugin_VST::LoadChunk()
{
	camxFile read;

	if(read.OpenFileRequester(0,this,Cxs[CXS_LOADDATADUMPPLUGIN],read.AllFiles(camxFile::FT_VSTDUMP),true)==true)
	{
		if(read.OpenRead(read.filereqname)==true)
		{
			char vstring[5];

			vstring[0]=vstring[4]=0;
			read.Read(vstring,4);

			if(strcmp(vstring,"VSTD")==0)
			{
				LONGLONG size=0;
				read.Read(&size,sizeof(LONGLONG));

				if(size>0)
				{
					if(char *dump=new char[size])
					{
						read.Read(dump,size);

						if(vstplugin->CanChunk()==true)
						{

							#ifdef TRYCATCH
						try
#endif
						{
							vstplugin->ptrPlug->dispatcher(vstplugin->ptrPlug,effSetChunk,0,size,dump, 0);
						}

						#ifdef TRYCATCH
						catch(...)
						{
							vstplugin->crashed=VSTCRASH_SetChunk;
						}
#endif
						}

						delete dump;
					}
				}
			}

			read.Close(true);
		}
	}

}

void Edit_Plugin_VST::SaveChunk()
{
	VstIntPtr datasize=0;
	void *chunkdata=0;

	if(vstplugin->CanChunk()==true)
	{
		if(vstplugin->vst_version>=2300)
		{
			/*
			VstPatchChunkInfo cinfo;

			VstIntPtr r=ptrPlug->dispatcher(ptrPlug,effBeginLoadBank,0,0,&cinfo, 0);

			//[return value]: -1: bank can't be loaded, 1: bank can be loaded, 0: unsupported

			TRACE ("R %d effBeginLoadBank \n",r);
			if(r)
			{
			}
			*/
		}

		#ifdef TRYCATCH
		try
#endif
		{
			datasize=vstplugin->ptrPlug->dispatcher(vstplugin->ptrPlug,effGetChunk,0,0,&chunkdata, 0);
		}

		#ifdef TRYCATCH
		catch(...)
		{
			vstplugin->crashed=VSTCRASH_GetChunk;
			return;
		}
#endif

	}

	LONGLONG savesize=chunkdata?datasize:0;

	if(savesize)
	{
		camxFile write;

		char *h=mainvar->GenerateString(vstplugin->GetEffectName(),"_VST_dump");

		if(write.OpenFileRequester(0,this,Cxs[CXS_SAVEDATADUMPFROMPLUGIN],write.AllFiles(camxFile::FT_VSTDUMP),false,h)==true)
		{
			write.AddToFileName(".vst");
			if(write.OpenSave(write.filereqname)==true)
			{
				// Header
				write.Save("VSTD",4);
				write.Save(&savesize,sizeof(LONGLONG));
				write.Save(chunkdata,savesize);
			}
		}

		if(h)
			delete h;

		write.Close(true);
	}
}

guiMenu *Edit_Plugin_VST::CreateMenu()
{
	guiMenu *n;

	if(menu)menu->RemoveMenu();
	menu=new guiMenu;

	if(menu)
	{
		if(vstplugin->CanChunk()==true)
		{
			n=menu->AddMenu(Cxs[CXS_FILE],0);
			if(n)
			{
				class menu_loaddump:public guiMenu
				{
				public:
					menu_loaddump(Edit_Plugin_VST *v){editor=v;}

					void MenuFunction()
					{
						editor->LoadChunk();
					} 

					Edit_Plugin_VST *editor;
				};

				n->AddFMenu(Cxs[CXS_LOADDATADUMPPLUGIN],new menu_loaddump(this));

				class menu_savedump:public guiMenu
				{
				public:
					menu_savedump(Edit_Plugin_VST *v){editor=v;}

					void MenuFunction()
					{
						editor->SaveChunk();
					} 

					Edit_Plugin_VST *editor;
				};

				n->AddFMenu(Cxs[CXS_SAVEDATADUMPFROMPLUGIN],new menu_savedump(this)); 
			}
		}

		//n=menu->AddMenu(vstplugin->GetEffectName(),0);
		vstplugin->AddIOMenu(menu);
	}

	return menu;
}
