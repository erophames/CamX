#ifndef CAMX_PROCESSOREDITOR_H
#define CAMX_PROCESSOREDITOR_H 1

#include "editor.h"
#include "guimenu.h"
#include "guigadgets.h"
#include "camxfile.h"
#include "MIDIprocessor.h"
#include "gui.h"

class MIDIPattern;

class menu_editmodule:public guiMenu
{
public:
	menu_editmodule(MIDIPlugin *m,guiGadget *g)
	{
		module=m;
		gadget=g;
	}

	void MenuFunction()
	{
		guiWindow *w=maingui->FirstWindow();
		guiWindow *open=0;

		while(w && open==0)
		{
		//	open=effect->audioeffect->CheckIfWindowIsEditor(w);
			w=w->NextWindow();
		}

		if(open)
		{
			open->WindowToFront(true);
		}	
		else
		{
			if(gadget)
			{
				guiWindowSetting setting;

				setting.startposition_x=gadget->x2+gadget->guilist->win->win_screenposx;
				setting.startposition_y=gadget->y+gadget->guilist->win->win_screenposy;

				module->OpenGUI(&setting);
			}
			else
				module->OpenGUI(0);
		}
	} //

	MIDIPlugin *module;
	guiGadget *gadget;
};

class Edit_ProcMod:public Editor
{
	friend GUI;
	
public:
	Edit_ProcMod(MIDIPlugin *m)
	{
		editorid=EDITORTYPE_PROCESSORMODULE;
		editorname="Processor Module";
		module=m;
	}
	
	void ResetModule();
	void Init();
	void Gadget(guiGadget *);
	EditData * EditDataMessage(EditData *data);

	guiMenu *CreateMenu();

	void FreeMemory()
	{	
		module->win=0;
		guigadgets.DeleteAllO();
		delete this;
	}
	
	int GetStartHeight();
	int GetStartWidth();

	MIDIPlugin *module;
private:
	OList guigadgets; //Edit_Plugin_VST_GUIObjects
};


class Edit_Processor:public Editor
{
public:
	Edit_Processor()
	{
		editorid=EDITORTYPE_PROCESSOR;
		activeprocessor=0;
		activemodule=0;
	}
	
	void AddModule(Processor *,MIDIPlugin *,MIDIPlugin *previous);
	EditData *EditDataMessage(EditData *);
	
	guiMenu *CreateMenu();
	
	void ResetGadgets()
	{
		procgadget=0;

		procname=
			modulegadget=0;

		add_proc=del_proc=0;
		add_mod=del_mod=0;
		edit_mod=0;
	}
	
	void UserNewProcessor();
	void DeleteProcessor();

	void ShowProcessorName();
	void ShowModules();
	void ShowProcessors();

	void ShowGrooveInfo();
	
	void InitGadgets();
	void Init();
	
	void FreeMemory();
	void Gadget(guiGadget *);
	
	void RedrawGfx();

	Processor *activeprocessor;
	MIDIPlugin *activemodule;
private:

	guiGadgetList *glist;

	guiGadget *procname;
	guiGadget_ListBox *procgadget;
	guiGadget_ListBox *modulegadget;

	guiGadget *add_proc;
	guiGadget *del_proc;
	guiGadget *add_mod;
	guiGadget *del_mod;
	guiGadget *edit_mod;

	//Edit_Frame frame_processor;
};

#endif