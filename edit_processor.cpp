#include "edit_processor.h"
#include "songmain.h"
#include "editor.h"
#include "gui.h"
#include "camxgadgets.h"
#include "imagesdefines.h"
#include "languagefiles.h"
#include "editdata.h"

#define PROCGADGETID_START GADGET_ID_START+50

#define GADGET_PROCNAME PROCGADGETID_START+3
#define GADGET_PROCS PROCGADGETID_START+4
#define GADGET_PROCMODS PROCGADGETID_START+5
#define GADGET_ADDPROC  PROCGADGETID_START+6
#define GADGET_DELPROC  PROCGADGETID_START+7
#define GADGET_ADDMOD  PROCGADGETID_START+8
#define GADGET_DELMOD  PROCGADGETID_START+9
#define GADGET_EDITMOD PROCGADGETID_START+10

EditData *Edit_Processor::EditDataMessage(EditData *data)
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

void Edit_Processor::DeleteProcessor()
{
}

void Edit_Processor::UserNewProcessor()
{
}

guiMenu *Edit_Processor::CreateMenu()
{
	guiMenu *n;
	
	if(menu)
		menu->RemoveMenu();
	
	if(menu=new guiMenu)
	{
		// Piano Editor Menu
		n=menu->AddMenu(Cxs[CXS_FILE],0);
		
		n=menu->AddMenu("Processor",0);
		if(n)
		{	
			class menu_NewProc:public guiMenu
			{
			public:
				menu_NewProc(Edit_Processor *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->UserNewProcessor();
				} //

				Edit_Processor *editor;
			};
			
			n->AddFMenu(Cxs[CXS_ADDNEWPROCESSOR],new menu_NewProc(this));
			
			class menu_delProc:public guiMenu
			{
			public:
				menu_delProc(Edit_Processor *ed)
				{
					editor=ed;
				}
				
				void MenuFunction()
				{
					editor->DeleteProcessor();
				} //

				Edit_Processor *editor;
			};

			n->AddFMenu(Cxs[CXS_DELETEPROCESSOR],new menu_delProc(this));
		}
	}

	return menu;
}

void Edit_Processor::AddModule(Processor *proc,MIDIPlugin *module,MIDIPlugin *previous)
{
	if(proc && module)
	{
		MIDIPlugin *newmod=module->CreateClone();

		if(newmod){
			proc->AddProcessorModule(newmod,previous);

			if(proc==activeprocessor)
			{
				activemodule=newmod;
				ShowModules();
			}
		}

	}
}

void Edit_Processor::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_EDITMOD:
		if(activemodule)
		{
			activemodule->OpenGUI(0);	
		}
		break;

	case GADGET_PROCS:
		{
			Processor *p=mainprocessor->GetProcessorAtIndex(g->index);
			if(p && p!=activeprocessor)
			{
				activeprocessor=p;
				activemodule=p->FirstProcessorModule();
				ShowModules();
				ShowProcessorName();
			}
		}
		break;

	case GADGET_PROCNAME:
		if(activeprocessor)
		{
			activeprocessor->SetName(g->string);
			ShowProcessors();
		}
		break;

	case GADGET_ADDPROC:
		if(Processor *p=new Processor)
		{
			p->SetName(Cxs[CXS_ADDNEWPROCESSOR]);
			mainprocessor->AddProcessor(p,activeprocessor);
			activeprocessor=p;
			ShowProcessors();
			ShowModules();
			ShowProcessorName();
		}
		break;

	case GADGET_DELPROC:
		if(activeprocessor)
		{
			Processor *nextprev=(Processor *)activeprocessor->NextOrPrev();

			maingui->RemoveProcessorFromGUI(activeprocessor,0);
			mainprocessor->DeleteProcessor(activeprocessor);
			activeprocessor=nextprev;
			ShowProcessors();
			ShowModules();
			ShowProcessorName();
		}
		break;

	case GADGET_PROCMODS:
		if(activeprocessor && activeprocessor->GetModuleAtIndex(g->index))
		{
			activemodule=activeprocessor->GetModuleAtIndex(g->index);
		}
		break;

	case GADGET_ADDMOD:
		if(activeprocessor && mainprocessor->FirstProcessorModule())
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				if(char *h2=mainvar->GenerateString(Cxs[CXS_ADDTO]," ",activeprocessor->name))
				{
					popmenu->AddMenu(h2,0);
					popmenu->AddLine();
					delete h2;
				}

				class menu_addmod:public guiMenu
				{
				public:
					menu_addmod(Edit_Processor *ed,Processor *p,MIDIPlugin *pm,MIDIPlugin *prev)
					{
						editor=ed;
						proc=p;
						module=pm;
						previous=prev;
					}

					void MenuFunction()
					{
						editor->AddModule(proc,module,previous);
					} //

					Edit_Processor *editor;
					Processor *proc;
					MIDIPlugin *module;
					MIDIPlugin *previous;
				};

				ProcessorModulePointer *pmp=mainprocessor->FirstProcessorModule();

				while(pmp)
				{
					if(char *n=mainvar->GenerateString(Cxs[CXS_ADD],":",pmp->module->staticname))
					{
						popmenu->AddFMenu(pmp->module->staticname,new menu_addmod(this,activeprocessor,pmp->module,activemodule));
						delete n;
					}

					pmp=pmp->NextProcessorPointer();
				}

				ShowPopMenu();
			}
		}
		break;

	case GADGET_DELMOD:
		if(activeprocessor && activemodule)
		{
			MIDIPlugin *np=(MIDIPlugin *)activemodule->NextOrPrev();

			maingui->RemoveProcessorFromGUI(0,activemodule);
			activeprocessor->DeleteProcessorModule(activemodule);
			activemodule=np;
			ShowModules();
		}
		break;
	}
}

void Edit_Processor::FreeMemory()
{
#ifdef OLDIE
	if(winmode&WINDOWMODE_INIT)
	{
		TRACE ("Remove GL Edit_Processor\n");

		gadgetlists.RemoveAllGadgetLists();

		if(winmode&WINDOWMODE_DESTROY)
		{
			mainprocessor->Save(0);
			delete this;
		}
	}
#endif
}

void Edit_Processor::ShowProcessorName()
{
	if(procname)
	{
		if(activeprocessor)
		{
			procname->Enable();
			procname->SetString(activeprocessor->name);
		}
		else
		{
			procname->SetString("-");
			procname->Disable();
		}
	}
}

void Edit_Processor::ShowModules()
{
	if(modulegadget)
	{
		modulegadget->ClearListBox();

		if(activeprocessor && activeprocessor->FirstProcessorModule())
		{
			modulegadget->Enable();
			MIDIPlugin *pm=activeprocessor->FirstProcessorModule();

			if(!activemodule)
				activemodule=pm;

			while(pm)
			{
				modulegadget->AddStringToListBox(pm->staticname);
				pm=pm->NextModule();
			}

			modulegadget->SetListBoxSelection(activemodule->GetIndex());
		}
		else
		{
			activemodule=0;
			modulegadget->Disable();
		}
	}
}

void Edit_Processor::ShowProcessors()
{
	if(procgadget)
	{
		procgadget->ClearListBox();

		Processor *p=mainprocessor->FirstProcessor();

		if(p)
		{
			if(!activeprocessor)
			{
				activeprocessor=p;
				if(activeprocessor)
					activemodule=p->FirstProcessorModule();
			}

			procgadget->Enable();

			while(p)
			{
				procgadget->AddStringToListBox(p->name);
				p=p->NextProcessor();
			}

			procgadget->SetListBoxSelection(activeprocessor->GetIndex());
		}
		else
		{
			activeprocessor=0;
			procgadget->Disable();
		}
	}
}

void Edit_Processor::InitGadgets()
{
#ifdef OLDIE
	ResetGadgets();

	glist=gadgetlists.AddGadgetList(this);

	if(glist)
	{
		int y2=frame_processor.y2-(maingui->GetFontSizeY()+4);

		if(y2>frame_processor.y)
		{
			procname=glist->AddString(frame_processor.x,0,frame_processor.x2/2,frame_processor.y-1,GADGET_PROCNAME,0,0,Cxs[CXS_NAMEOFPROCESSOR]);
			procgadget=glist->AddListBox(frame_processor.x,frame_processor.y,frame_processor.x2/2,y2,GADGET_PROCS,0,Cxs[CXS_LISTOFPROCESSORS]);
			modulegadget=glist->AddListBox(frame_processor.x2/2+1,frame_processor.y,frame_processor.x2,y2,GADGET_PROCMODS,0,Cxs[CXS_PROCESSORMODULES]);

			if(procgadget)
			{
				int h=(frame_processor.x2-frame_processor.x)/5;

				int hx=frame_processor.x;
				add_proc=glist->AddButton(hx,y2+1,hx+h,height,Cxs[CXS_ADDNEWPROCESSOR],GADGET_ADDPROC,0,Cxs[CXS_ADDNEWPROCESSOR]);
				hx+=h+1;
				del_proc=glist->AddButton(hx+1,y2+1,hx+h,height,Cxs[CXS_DELETEPROCESSOR],GADGET_DELPROC,0,Cxs[CXS_DELETEPROCESSOR]);
				hx+=h+1;

				if(modulegadget)
				{
					edit_mod=glist->AddButton(hx,y2+1,hx+h,height,Cxs[CXS_EDITMODULE],GADGET_EDITMOD,0,Cxs[CXS_EDITMODULE]);
					hx+=h+1;
					add_mod=glist->AddButton(hx,y2+1,hx+h,height,Cxs[CXS_ADDMODULE],GADGET_ADDMOD,0,Cxs[CXS_ADDMODULE]);
					hx+=h+1;
					del_mod=glist->AddButton(hx,y2+1,frame_processor.x2,height,Cxs[CXS_DELETEMODULE],GADGET_DELMOD,0,Cxs[CXS_DELETEMODULE]);

				}
			}
		}	
		
		/*
		int hx=frame_groups.x;
		int y=frame_groups.y2+1;
		int y2;
		int w=(frame_groups.x2-frame_groups.x)/4;

		w-=1;

		y2=y+maingui->GetFontSizeY_Sub();

		group_mute=glist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKMUTEOFF,FX_GROUPMUTE_ID,"Mute Group");

		hx+=w+1;
		group_solo=glist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKSOLOOFF,FX_GROUPSOLO_ID,"Solo Playback Group");

		hx+=w+1;
		group_rec=glist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKRECORDOFF,FX_GROUPREC_ID,"Set Tracks using Group in Record Mode");
	
		hx+=w+1;
		group_colour=glist->AddButton(hx,y,hx+w,y2,"Colour",FX_GROUPCOLOUR_ID,0,"Select Group Colour");

		if(group_colour)
		{
			colour_x=group_colour->x;
			colour_y=group_colour->y2+1;
			colour_y2=height-1;
			colour_x2=group_colour->x2;
		}

		ShowGroups();
		*/
	}
#endif
}

void Edit_Processor::RedrawGfx()
{
}


void Edit_Processor::Init()
{
	#ifdef OLDIE
	// bool ok=true;
	
	FreeMemory();
	
	if(width && height)
	{			
		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
		{
			frame_processor.on=true;
			frame_processor.x=0;
			frame_processor.x2=width;
			frame_processor.y=22;
			frame_processor.y2=height;
			
			frame_processor.CheckIfDisplay(this,0,0);
			
			InitGadgets();
			ShowProcessors();
			ShowModules();
			ShowProcessorName();
		}
	}
#endif
}

// Edit Proc Mod
void Edit_ProcMod::Gadget(guiGadget *g)
{
	module->Gadget(g);
}

EditData * Edit_ProcMod::EditDataMessage(EditData *data)
{
	module->EditDataMessage(data);
	return 0;
}

void Edit_ProcMod::ResetModule()
{
	module->Reset();
	module->ShowGUI();
}

guiMenu *Edit_ProcMod::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		guiMenu *gm=menu->AddMenu(Cxs[CXS_MODULES],0);
		if(gm)
		{
			class menu_reset:public guiMenu
			{
			public:
				menu_reset(Edit_ProcMod *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->ResetModule();
				}

				Edit_ProcMod *editor;
			};

			gm->AddFMenu("Reset",new menu_reset(this));
		}
	}

	return menu;
}

void Edit_ProcMod::Init()
{
#ifdef OLDIE
	if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
	{
		gadgetlists.RemoveAllGadgetLists();
		guiGadgetList *gl=gadgetlists.AddGadgetList(this);
		
		if(gl)
		{
			/*
			// Byepass
			bypass=gl->AddCheckBox(0,0,100,20,0,"Bypass");

			if(bypass)
			{
				if(effect->bypass==true)
					bypass->SetCheckBox(true);
			}
*/

			module->InitModuleGUI(this,0,0,width,height);
		}
	}
#endif
}