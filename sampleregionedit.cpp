#include "gui.h"
#include "sampleeditor.h"
#include "audiofile.h"
#include "audiorealtime.h"
#include "objectpattern.h"
#include "object_track.h"
#include "seqtime.h"
#include "audiohardware.h"
#include "camxgadgets.h"
#include "songmain.h"
#include "editbuffer.h"
#include "editfunctions.h"
#include "gui.h"
#include "languagefiles.h"
#include "object_song.h"
#include "peakbuffer.h"
#include "semapores.h"
#include "object_project.h"
#include "audiohdfile.h"
#include "audiofilework.h"
#include "languagefiles.h"

void menu_createregion::MenuFunction()
{
	if(editor->audiohdfile && editor->clipstart!=1 && editor->clipend!=-1)
	{
		if(AudioRegion *ar=new AudioRegion(editor->audiohdfile))
		{
			newregion=ar;

		//	editor->activeregion=ar;

			ar->regionstart=editor->clipstart; 
			ar->regionend=editor->clipend;
			ar->InitRegion();
			editor->audiohdfile->AddRegion(ar);

			if(editor_regionlist)
			{
				editor_regionlist->selectedregion=ar;
			}

			maingui->RefreshRegionGUI(ar->r_audiohdfile);
		}
	}
}

void menu_deleteregion::MenuFunction()
{
	editor->DeleteRegion(deleteregion);
}

enum{
	GADGETID_CREATE=GADGETID_EDITORBASE,
	GADGETID_COPY,
	GADGETID_DELETE,
	GADGETID_REGIONSTRING,
	GADGETID_REGIONSLV
	
};

Edit_RegionList::Edit_RegionList(Edit_Sample *ed)
{
	editorid=EDITORTYPE_SAMPLEREGIONLIST;
	InitForms(FORM_PLAIN1x1);

	//autovscroll=true;
	isstatic=true;
	song=ed->WindowSong();
	editor=ed;

	usemenuofwindow=ed;
	selectedregion=0;
	dontshowregionname=false;
}

void Edit_RegionList::RefreshRealtime()
{
}

void Edit_RegionList::RefreshRealtime_Slow()
{
}

void Edit_RegionList::ShowRegionName()
{
	if(dontshowregionname==true)
		return;

	if(selectedregion)
	{
		if(g_regionstring)
		{
			g_regionstring->Enable();
			g_regionstring->SetString(selectedregion->GetName());
		}
	}
	else
	{
		if(g_regionstring)
		{
			g_regionstring->SetString("-");
			g_regionstring->Disable();
		}
	}

}

void Edit_RegionList::ShowRegions()
{
	if(!regionslistview)return;

	if(!selectedregion)
	{
		if(editor->audiohdfile)
			selectedregion=editor->audiohdfile->FirstRegion();

	}

	regionslistview->ClearListView();

	char h2[NUMBERSTRINGLEN];

	AudioRegion *r=editor->audiohdfile->FirstRegion();

	while(r)
	{
		regionslistview->AddItem(0,r->GetName());
		char *time=mainvar->ConvertSamplesToTime(r->regionend-r->regionstart,0,h2);
		regionslistview->AddItem(1,time);

		regionslistview->AddItem(2,">");

		r=r->NextRegion();
	}

	if(selectedregion)
	{
		regionslistview->SetSelection(selectedregion->GetIndex());
	}
}

bool Edit_RegionList::GadgetListView(guiGadget_ListView *lv,int x,int y)
{
	switch(lv->gadgetID)
	{
		case GADGETID_REGIONSLV:
		{
			if(editor->audiohdfile)
			{
				AudioRegion *r=editor->audiohdfile->GetRegionIndex(lv->index);
				selectedregion=r;
				ShowRegionName();

				switch(x)
				{
				case 2:
					if(selectedregion)
					{
						// Playback
						menu_testregion play(editor,Edit_Sample::PLAYREGION_MODE_START,selectedregion);
						play.MenuFunction();

					}
					break;

				}

				return true;
			}
		}
		break;
	}

	return false;
}

void Edit_RegionList::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_REGIONSTRING:
		{
			if(selectedregion)
			{
				dontshowregionname=true;
				selectedregion->SetName(g->string,true,0);
				dontshowregionname=false;
			}

		}
		break;

	case GADGETID_COPY:
		{
			mainbuffer->CreateAudioBuffer(editor->audiohdfile,selectedregion);
		}
		break;

	case GADGETID_CREATE:
		{
			menu_createregion func(editor,this);
			func.MenuFunction();
		}break;

	case GADGETID_DELETE:
		if(selectedregion)
		{
			if(maingui->MessageBoxYesNo(0,Cxs[CXS_DELETEREGION_Q])==true)
			{
				menu_deleteregion func(editor,selectedregion);
				func.MenuFunction();
			}
		}
		break;
	}
}

void Edit_RegionList::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,-1,Cxs[CXS_CREATEREGION],GADGETID_CREATE,MODE_TEXTCENTER|MODE_RIGHT);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,Cxs[CXS_COPYREGION],GADGETID_COPY,MODE_LEFTTOMID|MODE_TEXTCENTER,Cxs[CXS_COPYREGION_CLIPBOARD]);
	glist.AddButton(-1,-1,-1,-1,Cxs[CXS_DELETEREGION],GADGETID_DELETE,MODE_MIDTORIGHT|MODE_TEXTCENTER);
	glist.Return();
	
	g_regionstring=glist.AddString(-1,-1,-1,-1,GADGETID_REGIONSTRING,MODE_RIGHT,0,0);
	glist.Return();

	regionslistview=glist.AddListView(-1,-1,-1,-1,GADGETID_REGIONSLV,MODE_RIGHT|MODE_BOTTOM);

	if(regionslistview)
	{
		regionslistview->AddColume("Regions",10);
		regionslistview->AddColume(Cxs[CXS_LENGTH],6);
		regionslistview->AddColume("Play",4);
	}

	ShowRegions();
	ShowRegionName();
}

void Edit_RegionList::DeInitWindow()
{
}


