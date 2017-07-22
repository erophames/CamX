#ifdef OLDIE

#include "drumeditor.h"
#include "songmain.h"
#include "editor.h"
#include "gui.h"
#include "camxgadgets.h"
#include "imagesdefines.h"
#include "languagefiles.h"
#include "chunks.h"

// Gadgets ---
#define DRUMMAPGADGETID_START GADGET_ID_START+50

#define GADGET_DRUMMAPID DRUMMAPGADGETID_START+1
#define GADGET_DRUMMAPDIFFID DRUMMAPGADGETID_START+2
#define GADGET_DRUMMAPNAME DRUMMAPGADGETID_START+3
#define GADGET_DRUMMAPINFO DRUMMAPGADGETID_START+4

#define GADGET_NEWDRUMMAP DRUMMAPGADGETID_START+5
#define GADGET_DELETEDRUMMAP DRUMMAPGADGETID_START+6
#define GADGET_EDITDRUMMAP DRUMMAPGADGETID_START+7

guiMenu *Edit_Drummap::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		guiMenu *s=menu->AddMenu(Cxs[CXS_FILE],0);

		if(s)
		{
			class menu_loaddrummap:public guiMenu
			{
			public:
				menu_loaddrummap(Edit_Drummap *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					editor->LoadDrummap();
				}

				Edit_Drummap *editor;
			};

			s->AddFMenu("Load Drummap",new menu_loaddrummap(this));

			class menu_savedrummap:public guiMenu
			{
			public:
				menu_savedrummap(Edit_Drummap *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					editor->SaveDrummap();	
				}

				Edit_Drummap *editor;
			};

			s->AddFMenu("Save Drummap",new menu_savedrummap(this));
		}
	}

	return menu;
}

void Edit_Drummap::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_DELETEDRUMMAP:
		if(activedrummap)
		{
			Drummap *np=(Drummap *)activedrummap->NextOrPrev();

			//maindrummap->RemoveDrumMapFromGUI(activedrummap);

			if(activedrummap->filename)
				mainvar->DeleteAFile(activedrummap->filename);

			//maindrummap->RemoveDrummap(activedrummap);

			activedrummap=np;

			ShowDrumMapName();
			ShowDrumMaps();
			ShowDrumTracks();
		}
		break;

	case GADGET_DRUMMAPNAME:
		if(activedrummap)
		{
			bool ok=activedrummap->SetName(g->string,true);
			ShowDrumMaps();
			if(ok==false)
				ShowDrumMapName();
		}
		break;

	case GADGET_DRUMMAPID:
		{
			int ix=g->index;
			Drummap *imap=maindrummap->FirstDrummap();

			while(ix--)
			{
				if(imap)
					imap=imap->NextMap();
			}

			if(imap && activedrummap!=imap)
			{
				activedrummap=imap;
				ShowDrumMaps();
				ShowDrumTracks();
				ShowDrumMapName();
			}
		}
		break;

	case GADGET_EDITDRUMMAP:
		if(activedrummap)
		{
			maingui->OpenEditorStart(EDITORTYPE_DRUM,0,0,0,0,activedrummap,0);
		}
		break;

	case GADGET_NEWDRUMMAP:
		{
			if(Drummap *ndmap=new Drummap)
			{
				ndmap->SetName("New Drum Map",true); // +Save

				maindrummap->AddDrummap(ndmap,false);

				activedrummap=ndmap;

				ShowDrumMaps();
				ShowDrumTracks();
				ShowDrumMapName();
			}
		}
		break;
	}

}

void Edit_Drummap::FreeMemory()
{
	if(winmode&WINDOWMODE_INIT)
	{
		gadgetlists.RemoveAllGadgetLists();
	}
}

void Edit_Drummap::ShowDrumTracks()
{
	if(drumtrackgadget)
	{
		drumtrackgadget->ClearListBox();

		if(activedrummap)
		{
			drumtrackgadget->Enable();

			Drumtrack *t=activedrummap->FirstTrack();

			if(!activedrumtrack)
				activedrumtrack=t;

			if(t)
			{
				while(t)
				{
					drumtrackgadget->AddStringToListBox(t->name);

					t=t->NextTrack();
				}

				drumtrackgadget->SetListBoxSelection(activedrummap->GetIndexOfTrack(activedrumtrack));
			}
		}
		else
		{
			drumtrackgadget->Disable();
		}
	}
}

void Edit_Drummap::ShowDrumMapName()
{
	if(drummapname)
	{
		if(activedrummap)
		{
			drummapname->SetString(activedrummap->GetName());
		}
		else
		{
			drummapname->SetString("No Drum Map");
			drummapname->Disable();
		}
	}
}

void Edit_Drummap::ShowDrumMaps()
{
	if(!activedrummap)
		activedrummap=maindrummap->FirstDrummap();

	if(drummapgadget)
	{
		drummapgadget->ClearListBox();

		Drummap *d=maindrummap->FirstDrummap();

		if(d)
		{
			drummapgadget->Enable();
			char h2[NUMBERSTRINGLEN];

			while(d)
			{
				char *h=mainvar->GenerateString(d->GetName()," Tracks:",mainvar->ConvertIntToChar(d->GetCountOfTracks(),h2));

				if(h)
				{
					drummapgadget->AddStringToListBox(h);
					delete h;
				}
				else
					drummapgadget->AddStringToListBox("DM?");
				d=d->NextMap();
			}

			drummapgadget->SetListBoxSelection(maindrummap->GetOfDrummap(activedrummap));
		}
		else
		{
			drummapgadget->Disable();
		}
	}

	if(activedrummap)
	{
		if(deletedrummap)
			deletedrummap->Enable();

		if(editdrummap)
			editdrummap->Enable();
	}
	else
	{
		if(deletedrummap)
			deletedrummap->Disable();

		if(editdrummap)
			editdrummap->Disable();
	}
}

void Edit_Drummap::InitGadgets()
{
	#ifdef OLDIE
	ResetGadgets();

	glist=gadgetlists.AddGadgetList(this);

	if(glist)
	{
	//	drummapname=glist->AddString(frame_drummaps.x,0,frame_drummaps.x2/2,frame_drummaps.y-1,GADGET_DRUMMAPNAME,0,0);
	//	drumtrackname=glist->AddString((frame_drummaps.x2/2)+1,0,frame_drummaps.x2,frame_drummaps.y-1,GADGET_DRUMMAPINFO,0,0);

		drummapgadget=glist->AddListBox(frame_drummaps.x,frame_drummaps.y,frame_drummaps.x2/2,frame_drummaps.y2,GADGET_DRUMMAPID,0);
		drumtrackgadget=glist->AddListBox(frame_drummaps.x2/2+1,frame_drummaps.y,frame_drummaps.x2,frame_drummaps.y2,GADGET_DRUMMAPDIFFID,0);

		int hx=0;
		int h=width/3-1;

		newdrummap=glist->AddButton(hx,frame_drummaps.y2+1,hx+h,height,"New Drum Map",GADGET_NEWDRUMMAP);
		hx+=h+1;
		deletedrummap=glist->AddButton(hx,frame_drummaps.y2+1,hx+h,height,"Delete Drum Map",GADGET_DELETEDRUMMAP);
		hx+=h+1;
		editdrummap=glist->AddButton(hx,frame_drummaps.y2+1,hx+h,height,"Edit Drum Tracks",GADGET_EDITDRUMMAP);

		ShowDrumMapName();
		ShowDrumMaps();
		ShowDrumTracks();
	}
#endif
}

void Edit_Drummap::LoadDrummap()
{
	camxFile sfile;

	if(sfile.OpenFileRequester(0,this,"Load Drummap","Drummaps (*.cxdm)|*.cxdm;|All Files (*.*)|*.*||",true)==true)
	{
		if(sfile.OpenRead(sfile.filereqname)==true)
		{
			char check[4];

			sfile.Read(check,4);

			if(mainvar->CompareStringWithOutZero(check,"CAMX")==true)
			{
				sfile.Read(check,4);

				if(mainvar->CompareStringWithOutZero(check,"DMAP")==true)
				{
					int version=0;

					sfile.Read(&version);

					Drummap *ndmap=maindrummap->AddDrummap();

					if(ndmap)
					{
						sfile.LoadChunk();

						if(sfile.GetChunkHeader()==CHUNK_DRUMMAP)
						{
							sfile.ChunkFound();

							ndmap->Load(&sfile);

							activedrummap=ndmap;
						}

						ShowDrumMaps();
					}
				}
			}
		}

		sfile.Close(true);
	}
}

void Edit_Drummap::SaveDrummap()
{
	if(activedrummap)
	{
		camxFile sfile;

		if(sfile.OpenFileRequester(0,this,"Save Drummap","Drummaps (*.cxdm)|*.cxdm;|All Files (*.*)|*.*||",false)==true)
		{
			sfile.AddToFileName(".cxdm");

			if(sfile.OpenSave(sfile.filereqname)==true)
			{
				int version=maingui->GetVersion();

				// Header
				sfile.Save("CAMX",4);
				sfile.Save("DMAP",4);
				sfile.Save(&version,sizeof(int)); // Version

				activedrummap->Save(&sfile);
			}

			sfile.Close(true);
		}
	}
}

void Edit_Drummap::Init()
{
	#ifdef OLDIE
	FreeMemory();

	if(width && height)
	{			
		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
		{
			frame_drummaps.on=true;
			frame_drummaps.x=0;
			frame_drummaps.x2=width;
			frame_drummaps.y=22;
			frame_drummaps.y2=height-maingui->GetFontSizeY();

			frame_display.on=true;
			frame_display.x=frame_drummaps.x;
			frame_display.y=frame_drummaps.y2;
			frame_display.x2=frame_drummaps.x2;
			frame_display.y2=height;

			frame_display.CheckIfDisplay(this,0,0);

			if(!activedrummap)
				activedrummap=maindrummap->FirstDrummap();

			InitGadgets();	
		}
	}
#endif
}

#endif