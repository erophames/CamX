#ifndef CAMX_PLUGINWINDOW_H
#define CAMX_PLUGINWINDOW_H 1

#include "editortypes.h"
#include "editor.h"
#include "MIDIfilter.h"

class InsertAudioEffect;

enum{
	ID_BYPASS,
	ID_ONOFF,
	ID_INVOLUME,
	ID_OUTVOLUME,
	ID_EDITOR,
	ID_TAB,
	ID_PROGRAMSELECT,
	ID_PROGRAMUSER,
	ID_PROGRAMTEXT,
	ID_PROGRAMNR,
	ID_OUTS,
	ID_MFILTER,
	ID_AUTOMATE,

	ID_INTERNSTART
};

enum{
	PIN_EDITOR,
	PIN_TAB
};

enum{
	OBJECTID_PARAMETERLIST=OI_LAST,
};

class Edit_PluginList_Parameter:public guiObject
{
	friend class Edit_PluginList;
public:

	Edit_PluginList_Parameter()
	{
		id=OBJECTID_PARAMETERLIST;
		error=mouseclicked=false;
	}

	void ShowParameter();

	Edit_PluginList *editor;

	double mmxpar;
	int index,mmx;
	bool mouseclicked,error;
};

class PluginWindow;

class Edit_PluginList:public EventEditor
{
public:
	Edit_PluginList(PluginWindow *);

	void Gadget(guiGadget *);
	void ShowVSlider();
	void ShowList();
	void Init();
	void DeInitWindow();
	void FreeEditorMemory();

	void MouseClickInList(bool leftmouse);
	void MouseReleaseInList(bool leftmouse);
	void MouseMoveInList(bool leftmouse);

	OListCoosY parameterobjects;
	PluginWindow *editor;
	guiGadget_Tab *pllist;
};

class PluginWindow:public guiWindow
{
public:

	PluginWindow();

	virtual void InitPluginParameterEditor(){}
	virtual void RemoveChildWindows(){}

	void ShowMode();
	int GetCountParameter();
	void InitPluginEditor();
	void InitFormSize();
	void ShowOnOff();
	void ShowBypass();
	guiGadget *PluginGadget(guiGadget *);
	void PluginRefreshRealtime();
	void PluginRefreshRealtime_Slow();
	void InitPlugInList();
	void CloseListEditor();
	void SetName(bool refreshmenu);
	void ShowPlugInProgram();
	void ShowMIDIFilter();
	
	MIDIFilter comparefilter;
	Edit_PluginList *listeditor;
	InsertAudioEffect *insertaudioeffect;
	guiGadget_Number *programuser;
	guiGadget *onoff,*bypass,*g_mode_tab,*g_mode_editor,*outputs,*programselect,*programtext,*automate,*g_MIDIfilter;
	guiGadget_Volume *involume,*outvolume;

	ARES status_involume,status_outvolume;
	int mode,plugineditorwidth,plugineditorheight,pluginbypasswidth;
	bool onoff_status,bypass_status;
};
#endif