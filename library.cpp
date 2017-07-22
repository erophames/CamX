#include "library.h"
#include "languagefiles.h"


Edit_Library::Edit_Library()
	{
		editorid=EDITORTYPE_LIBRARY;
		editorname="Libary";
	
		/*
		inbound=false;

		focustrack=0;
		starttrack=0;
		
		zoom_y=20;
		createdoublebuffer=true;
		
		frame_tracks.bpen=COLOUR_GREY_LIGHT;
		
		// FX
		showeffects=true;
		
		headerflag=HEADERFLAG_SHOWZOOM;
		
		// Init Track Fx
		trackfx.drumeditor=this;
		trackfx.frame=&frame_fx;
		
		patternselection.buildfilter=SEL_INTERN;
		
		cursor.ostart=0;
		cursor.drumtrack=0;
		cursorsprite.staticsprite=true;
		
		drummap=0;

		frame_tracks.settingsvar=&mainsettings->drum_trackswidth;

		SetStartFrames();
		*/
	}

guiMenu *Edit_Library::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		guiMenu *n=menu->AddMenu(Cxs[CXS_FILE],0);
	}

	return menu;
}

void Edit_Library::ShowObjects()
{
	if(tree)
	{
		tree->AddItemToTree("Entry1");
		tree->AddItemToTree("Entry2");
		tree->AddItemToTree("Entry3");
	}
}

void Edit_Library::InitGadgets()
{
	#ifdef OLDIE
	guiGadgetList *gl=gadgetlists.AddGadgetList(this);

	ResetGadgets();

	if(gl)
	{
		if(frame_objects.ondisplay==true)
		{
			tree=gl->AddTree(frame_objects.x,frame_objects.y,frame_objects.x2,frame_objects.y2,0);
		}
	}
#endif
}

void Edit_Library::Init()
{
	FreeMemory();

	/*
	if(width && height)
	{
		if(winmode&WINDOWMODE_INIT && 
			(!(winmode&WINDOWMODE_RESIZE)) &&
			(!(winmode&WINDOWMODE_FRAMES)) 
			)
		{
			//InitList();
			//ShowMenu();
		}

		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
		{
			InitFrames();
		}

		ShowObjects();
	}
	*/
}

void Edit_Library::MouseMove(bool inside)
{
}

void Edit_Library::KeyDown()
{
}

void Edit_Library::KeyUp(char nVirtKey)
{
}

void Edit_Library::Gadget(guiGadget *gadget)
{
}

void Edit_Library::FreeMemory()
{
	if(winmode&WINDOWMODE_INIT)
	{
		TRACE ("Remove GL Edit_Library\n");

		//gadgetlists.RemoveAllGadgetLists();

	}
}