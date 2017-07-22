#ifndef CAMX_MONITOREDITOR_H
#define CAMX_MONITOREDITOR_H 1

#include "editor.h"

#define SHOWALL_OUTDEVICES 1
#define SHOWALL_INDEVICES 2
#define SHOWSINGLE_OUTDEVICE 4
#define SHOWSINGLE_INDEVICE 8
#define SHOWALL_PLUGINS 16

class Edit_MonitorList_MonitorEvent:public guiObject
{
	friend class Edit_Monitor;

public:
	Edit_MonitorList_MonitorEvent();

	void ShowMonitorEvent();
	
	Edit_Monitor *editor;
	LMIDIEvents *seqevent;
};

class Edit_Monitor:public EventEditor
{
	friend Edit_MonitorList_MonitorEvent;

public:
	Edit_Monitor();

	bool CreateFilterEvents();
	void Gadget(guiGadget *);
	void EditFilter();
	void FreeEditorMemory();
	void DeInitWindow();
	guiMenu *CreateMenu();
	void Init();
	void InitGadgets();
	void ShowVSlider();
	void BuildMonitorList();
	void ShowList();
	void FreeMemory();
	void RefreshRealtime();

	char help[255];
	guiGadget_Tab *list;
	int showflag;

private:
	void ClearMonitor();
	void ClearMonitorData();
	void ShowFlags();
	void InitDevices();

	OListCoosY monitorobjects;
	
	//OList outdevices,indevices;
	LMIDIList filterevents; // LMIDIEvents Out
	LMIDIEvents *FirstEvent(){return filterevents.FirstEvent();}

	MIDIFilter filter,cmpfilter;

	guiMenu *menu_showmonitordevicename;
	guiGadget *selectgadget,*cleargadget,*g_realtimescroll;
	MIDIInputDevice *singleindevice;
	MIDIOutputDevice *singleoutdevice;
	int outevents,inevents,notetype;
	bool displaynoteoff_monitor,/*initdevices,*/showmonitordevicename,realtimescroll;

};
#endif