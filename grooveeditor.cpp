#include "songmain.h"
#include "editor.h"
#include "gui.h"
#include "camxgadgets.h"
#include "imagesdefines.h"
#include "MIDIhardware.h"
#include "groove.h"
#include "languagefiles.h"
#include "editdata.h"
#include "chunks.h"
#include <math.h>
#define GROOVE_START MENU_ID_START+50	

// Menus
#define GROOVE_ADDGROOVE GROOVE_START+10

// Gadgets ---
#define GROOVEGADGETID_START GADGET_ID_START+50

#define GADGET_GROOVEID GROOVEGADGETID_START+1
#define GADGET_GROOVEDIFFID GROOVEGADGETID_START+2
#define GADGET_GROOVENAME GROOVEGADGETID_START+3
#define GADGET_GROOVEINFO GROOVEGADGETID_START+4

void Edit_Groove::ShowGrooveInfo()
{
	if(glist)
	{
		if(groovename)
		{
			if(activegroove)
			{
				groovename->SetString(activegroove->GetGrooveInfo());
			}
			else
			{
				groovename->SetString("-");
				groovename->Disable();
			}
		}

		if(grooveinfo)
		{
			if(activegroove)
			{
				grooveinfo->SetString(quantstr[activegroove->qrasterid]);
			}
			else
				grooveinfo->SetString("-");

			grooveinfo->Disable();
		}
	}
}

void Edit_Groove::ShowActiveGroove()
{
#ifdef OLDIE
	frame_display.Fill(this,COLOUR_GREY_LIGHT);

	groovepixel.DeleteAllO();
	grooves.DeleteAllO();

	// Raster
	if(activegroove)
	{
		if(groovegadget)
			groovegadget->Enable();

		if(groovediffgadget)
			groovediffgadget->Enable();

		OSTART i=activegroove->raster;
		double x=frame_display.x;
		double stepx=frame_display.x2-frame_display.x;
		double hx;

		stepx/=i+1;

		displaystep_pixel=stepx;
		displaystep_pixeltick=activegroove->tickraster/stepx;

		x+=stepx;

		bitmap.SetAPen(COLOUR_BLACK);

		int c=1;

		char res[NUMBERSTRINGLEN];

		while(i--)
		{
			GroovePixel *npx=new GroovePixel;

			if(npx)
			{
				npx->x=(int)x;
				npx->step=c-1;

				groovepixel.AddEndO(npx);

				bitmap.guiDrawLine((int)x,frame_display.y,(int)x,frame_display.y2);
				//	bitmap.SetFont(&maingui->smallfont);
				bitmap.guiDrawText((int)x+1,frame_display.y+15,frame_display.x2,mainvar->ConvertIntToChar(c,res));
			}

			c++;
			x+=stepx;
		}

		GrooveElement *e=activegroove->FirstGrooveElement();
		GroovePixel *p=FirstGroovePixel();

		while(e && p)
		{
			hx=e->quantdiff;

			if(e==MoveO)
			{
				if(movediff!=0)
					hx+=movediff;
			}

			hx/=displaystep_pixeltick;

			x=(int)hx+p->x;

			GrooveObject *go=new GrooveObject();

			if(go)
			{
				go->element=e;

				int x1,x2;

				x1=x2=(int)x;

				x1-=(int)(stepx/3);
				x2+=(int)(stepx/3);

				go->x1=x1;
				go->x2=x2;
				go->y1=frame_display.y;
				go->y2=frame_display.y2;

				int colour=COLOUR_GREY;

				if(e==MoveO)
					colour=COLOUR_GREEN;
				else
					if(e->quantdiff<0)
						colour=COLOUR_RED;
					else
						if(e->quantdiff>0)
							colour=COLOUR_BLUE;

				grooves.AddEndO(go);

				bitmap.guiDrawLine(x1,frame_display.y2,(int)x,frame_display.y,colour);
				bitmap.guiDrawLine((int)x,frame_display.y,x2,frame_display.y2,colour);
				bitmap.guiDrawLine(x1,frame_display.y2,x2,frame_display.y2,colour);
			}

			e=e->NextGrooveElement();
			p=p->NextGroovePixel();
		}
	}
	else
	{
		if(groovegadget)
			groovegadget->Disable();

		if(groovediffgadget)
			groovediffgadget->Disable();
	}

#endif

}

void Edit_Groove::ListActiveGroove()
{
	activegroove_shown=activegroove;

	if(glist && groovediffgadget)
	{	
		groovediffgadget->ClearListBox();

		if(activegroove)
		{
			GrooveElement *ge=activegroove->FirstGrooveElement();
			int i=1;

			while(ge)
			{
				char h[256];
				char res[NUMBERSTRINGLEN];

				strcpy(h,mainvar->ConvertIntToChar(i,res));
				mainvar->AddString(h,":");
				mainvar->AddString(h,mainvar->ConvertLongLongToChar(ge->quantdiff,res));

				groovediffgadget->AddStringToListBox(h);

				i++;
				ge=ge->NextGrooveElement();
			}
		}
	}
}

EditData * Edit_Groove::EditDataMessage(EditData *data)
{	
	if(data)
	{
		switch(data->id)
		{
		default:

			break;
		}
	}

	return 0;
}

void Edit_Groove::DeleteGroove()
{
	if(activegroove)
	{
		Groove *n=(Groove *)activegroove->NextOrPrev();

		mainMIDI->DeleteGroove(activegroove);
		activegroove=n;
		RefreshGrooves();
		maingui->RefreshAllEditors(0,EDITORTYPE_QUANTIZEEDITOR,0);
	}
}

void Edit_Groove::ResetGroove()
{
	if(activegroove)
	{
		ChangeGroove(0,0); // reset
	}
}

void Edit_Groove::UserNewGroove(int steps)
{
	if(DeletePopUpMenu(true))
	{
		int a;

		class menu_qt:public guiMenu
		{
		public:
			menu_qt(Edit_Groove *ed,int t,int s)
			{
				editor=ed;
				tx=t;
				steps=s;
			}

			void MenuFunction()
			{
				Groove *g=mainMIDI->AddGroove(steps,tx);

				if(g)
				{
					g->InitGroove(tx);

					editor->activegroove=g;

					/*
					editor->RefreshGrooves();

					maingui->RefreshAllEditors(0,EDITORTYPE_QUANTIZEEDITOR,0);
					*/
				}
			} //

			Edit_Groove *editor;
			int tx,steps;
		};

		popmenu->AddMenu(Cxs[CXS_SELECTGROOVERASTER],0);
		popmenu->AddLine();

		// Quantize
		for(a=0;a<QUANTNUMBER;a++)
			popmenu->AddFMenu(quantstr[a],new menu_qt(this,a,steps));

		ShowPopMenu();
	}

}

guiMenu *Edit_Groove::CreateMenu()
{
	guiMenu *n;

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		n=menu->AddMenu(Cxs[CXS_FILE],0);

		if(n)
		{
			class menu_LoadGrooveList:public guiMenu
			{
			public:
				menu_LoadGrooveList(Edit_Groove *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->LoadList();
				} //

				Edit_Groove *editor;
			};

			guiMenu *s=n->AddFMenu("Add (Load) Groove List",new menu_LoadGrooveList(this));

			class menu_SaveGrooveList:public guiMenu
			{
			public:
				menu_SaveGrooveList(Edit_Groove *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->SaveList();
				} //

				Edit_Groove *editor;
			};

			s=n->AddFMenu("Save Groove List",new menu_SaveGrooveList(this));

			n->AddLine();

			class menu_LoadGroove:public guiMenu
			{
			public:
				menu_LoadGroove(Edit_Groove *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->Load();
				} //

				Edit_Groove *editor;
			};

			s=n->AddFMenu("Load Groove",new menu_LoadGroove(this));

			class menu_SaveGroove:public guiMenu
			{
			public:
				menu_SaveGroove(Edit_Groove *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->Save();
				} //

				Edit_Groove *editor;
			};

			s=n->AddFMenu("Save Groove",new menu_SaveGroove(this));
		}

		n=menu->AddMenu("Groove",0);

		if(n)
		{	
			class menu_NewGroove:public guiMenu
			{
			public:
				menu_NewGroove(Edit_Groove *ed,int s)
				{
					editor=ed;
					steps=s;
				}

				void MenuFunction()
				{
					editor->UserNewGroove(steps);
				} //

				Edit_Groove *editor;
				int steps;
			};

			guiMenu *s=n->AddMenu("Create Groove",0);

			if(s)
			{
				s->AddFMenu(("8 Groove Points"),new menu_NewGroove(this,8));
				s->AddFMenu(("16 Groove Points"),new menu_NewGroove(this,16));
				s->AddFMenu(("24 Groove Points"),new menu_NewGroove(this,24));
				s->AddFMenu(("32 Groove Points"),new menu_NewGroove(this,32));
			}

			class menu_delGroove:public guiMenu
			{
			public:
				menu_delGroove(Edit_Groove *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->DeleteGroove();
				} //

				Edit_Groove *editor;
			};

			n->AddFMenu(("Delete Groove"),new menu_delGroove(this));

			class menu_resetGroove:public guiMenu
			{
			public:
				menu_resetGroove(Edit_Groove *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->ResetGroove();
				} //

				Edit_Groove *editor;
			};

			n->AddFMenu(("Reset Groove"),new menu_resetGroove(this));
		}
	}

		maingui->AddCascadeMenu(this,menu);
	return menu;
}

void Edit_Groove::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_GROOVENAME:
		if(activegroove)
		{
			activegroove->SetName(g->string);
			activegroove->InitGrooveName();
			ShowGrooves();
		}
		break;

	case GADGET_GROOVEID:
		{	
			Groove *a=mainMIDI->GetGrooveIndex(g->index);

			if(a!=activegroove)
			{
				activegroove=a;

				ShowGrooveInfo();
				ShowActiveGroove();
				ListActiveGroove();
			}	
		}
		break;
	}
}

void Edit_Groove::FreeMemory()
{
	if(winmode&WINDOWMODE_INIT)
	{
		groovepixel.DeleteAllO();
		grooves.DeleteAllO();

		//gadgetlists.RemoveAllGadgetLists();
	}
}

void Edit_Groove::ShowGrooves()
{
	numbergrooves=mainMIDI->GetNrGrooves();

	if(!activegroove)
	{
		activegroove=mainMIDI->FirstGroove();
	}

	if(activegroove && activegroove!=activegroove_shown)
	{
		ShowGrooveInfo();
		ShowActiveGroove();
		ListActiveGroove();
	}

	if(glist && groovegadget)
	{
		Groove *a=mainMIDI->FirstGroove();

		groovegadget->ClearListBox();

		while(a)
		{
			if(!activegroove)
			{
				activegroove=a;
				ShowActiveGroove();
			}

			groovegadget->AddStringToListBox(a->GetGrooveName());

			a=a->NextGroove();
		}

		if(activegroove)
		{
			groovegadget->SetListBoxSelection(activegroove->GetIndex());
		}
	}	
}

void Edit_Groove::InitGadgets()
{
	ResetGadgets();

	//	groovename=glist->AddString(frame_grooves.x,0,frame_grooves.x2/2,frame_grooves.y-1,GADGET_GROOVENAME,0,0);
	//	grooveinfo=glist->AddString((frame_grooves.x2/2)+1,0,frame_grooves.x2,frame_grooves.y-1,GADGET_GROOVEINFO,0,0);

		//groovegadget=glist->AddListView(frame_grooves.x,frame_grooves.y,frame_grooves.x2/2,frame_grooves.y2,GADGET_GROOVEID);
		//groovediffgadget=glist->AddListView(frame_grooves.x2/2+1,frame_grooves.y,frame_grooves.x2,frame_grooves.y2,GADGET_GROOVEDIFFID);

	ShowGrooves();
}

void Edit_Groove::RedrawGfx()
{
	ShowActiveGroove();
}

void Edit_Groove::RefreshGrooves()
{				
	ShowGrooveInfo();
	ShowGrooves();

	ShowActiveGroove();
	ListActiveGroove();
}

void Edit_Groove::ClearMoveO()
{
	if(MoveO)
	{
		MoveO=0;
		movediff=0;

		ShowActiveGroove();
	}
}

void Edit_Groove::MouseMove(bool inside)
{
#ifdef OLDIE
	if(MoveO)
	{
		double diffx=GetMouseX()-startmouse_x;

		diffx*=displaystep_pixeltick;

		if((int)floor(diffx+0.5)!=movediff)
		{
			movediff=(int)floor(diffx+0.5);

			if(movediff<-(activegroove->tickraster/2))
			{
				movediff=-(activegroove->tickraster/2);
			}
			else
			{
				if(movediff>(activegroove->tickraster/2))
				{
					movediff=activegroove->tickraster/2;
				}
			}

			ShowActiveGroove();
		}
	}
#endif

}

void Edit_Groove::MouseButton(int flag)
{
	if(flag&(MOUSEKEY_LEFT_UP|MOUSEKEY_RIGHT_UP)) // Mouse Released
	{
		if(MoveO && (movediff!=0))
		{
			ChangeGroove(MoveO,MoveO->quantdiff+movediff);
		}

		ClearMoveO();
	}
	else
	{
		switch(right_mousekey)
		{
		case MOUSEKEY_DOWN:
			{
				/*
				switch(mousemode)
				{
				case EM_SELECTOBJECTS:
				{
				ClearAllSprites();

				SetEditorMode(EM_RESET);
				ShowAllSprites();
				}
				break;

				case EM_SELECT:
				{
				SelectAndEdit(true);
				}
				break;
				}
				*/
			}
			break;
		}

		switch(left_mousekey)
		{
		case MOUSEKEY_DOWN:
			{
				/*
				GrooveObject *f=FindGrooveObject(GetMouseX(),GetMouseY());

				if(f)
				{
					//	MessageBeep(-1);
					MoveO=f->element;
					startmouse_x=GetMouseX();

					ShowActiveGroove();
				}
*/
			}
			break; // down
		}
	}
}

void Edit_Groove::ChangeGroove(GrooveElement *e,OSTART qdiff)
{
	if(activegroove)
	{
		bool changed=false;

		if(e && 
			e->quantdiff!=qdiff && 
			(qdiff>=-(activegroove->tickraster/2)) &&
			(qdiff<=(activegroove->tickraster/2))
			)
		{
			e->quantdiff=qdiff;

			changed=true;
		}
		else
		{
			if(!e) // Reset All
			{	
				GrooveElement *e=activegroove->FirstGrooveElement();

				while(e)
				{
					if(e->quantdiff)
					{
						e->quantdiff=0;
						changed=true;
					}

					e=e->NextGrooveElement();
				}
			}
		}

		if(changed==true)
		{
			activegroove->ChangeInSongs();

			ShowActiveGroove();
			ListActiveGroove();
		}
	}
}

Edit_Groove::Edit_Groove()
	{
		editorid=EDITORTYPE_GROOVE;
		activegroove=0;
		MoveO=0;
		movediff=0;
		activegroove_shown=0;
	}

void Edit_Groove::Load()
{
	camxFile load;

	if (load.OpenFileRequester(0,this,"Load/Add Groove"," (*.grvx)|*.grvx;|All Files (*.*)|*.*||",true)==true)
	{
		if(load.OpenRead(load.filereqname)==true)
		{
			load.LoadChunk();

			if(load.GetChunkHeader()==CHUNK_GROOVE)
			{
				load.ChunkFound();

				if(Groove *g=new Groove)
				{
					g->Load(&load);

					mainMIDI->AddGroove(g);
					ShowGrooves();
				}
			}

			load.Close(true);
		}
	}
}

void Edit_Groove::Save()
{
	if(activegroove)
	{
		camxFile save;

		if (save.OpenFileRequester(0,this,"Save Groove"," (*.grvx)|*.grvx;|All Files (*.*)|*.*||",false)==true)
		{					
			save.AddToFileName(".grvx");

			if(save.OpenSave(save.filereqname)==true)
			{
				activegroove->Save(&save);

				save.Close(true);
			}
		}
	}
}

void Edit_Groove::LoadList()
{
	camxFile load;

	if (load.OpenFileRequester(0,this,"Load/Add Grooves"," (*.grlx)|*.grlx;|All Files (*.*)|*.*||",true)==true)
	{
		if(load.OpenRead(load.filereqname)==true)
		{
			load.LoadChunk();

			if(load.GetChunkHeader()==CHUNK_MIDIGROOVES)
			{
				load.ChunkFound();
				if(mainMIDI->LoadGrooves(&load)>0)
					ShowGrooves();
			}

			load.Close(true);
		}
	}
}

void Edit_Groove::SaveList()
{
	if(mainMIDI->FirstGroove())
	{
		camxFile save;

		if (save.OpenFileRequester(0,this,"Save Grooves"," (*.grlx)|*.grlx;|All Files (*.*)|*.*||",false)==true)
		{					
			save.AddToFileName(".grlx");

			if(save.OpenSave(save.filereqname)==true)
			{
				mainMIDI->SaveGrooves(&save);

				save.Close(true);
			}
		}
	}
}

void Edit_Groove::RefreshRealtime()
{
	if(numbergrooves!=mainMIDI->GetNrGrooves())
	{
		ShowGrooves();
	}
}

void Edit_Groove::Init()
{
	InitGadgets();

#ifdef OLDIE
	// bool ok=true;

	FreeMemory();

	if(width && height)
	{			
		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
		{
			double h=height;

			frame_grooves.on=true;
			frame_grooves.x=0;
			frame_grooves.x2=width;
			frame_grooves.y=22;
			frame_grooves.y2=(int)(h*0.60);

			frame_display.on=true;
			frame_display.x=frame_grooves.x;
			frame_display.y=frame_grooves.y2;
			frame_display.x2=frame_grooves.x2;
			frame_display.y2=height-2;

			frame_display.CheckIfDisplay(this,0,0);

			InitGadgets();	
		}

		ShowGrooveInfo();
		ListActiveGroove();
		RedrawGfx();
	}
#endif

}
