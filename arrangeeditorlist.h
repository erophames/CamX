#ifndef CAMX_ARRANGEEDITORLIST_H
#define CAMX_ARRANGEEDITORLIST_H 1

#include "editor.h"

class Edit_Arrange;
class Edit_ArrangeList;

class Edit_ArrangeList_Track:public guiObject
{
	friend class Edit_ArrangeList;

public:
	Edit_ArrangeList_Track();

	void DeInit()
	{
		if(namestring)delete namestring;
		namestring=0;
	}

	void ShowTrack(bool refreshbgcolour);
	
	void ShowNumber();
	void ShowName(bool force);
	void ShowTrackInfo(bool force);
	void ShowColour();
	void ShowChildOpenClose();
	void ShowAudioDisplay(bool force);
	void ShowMIDIDisplay(bool force);
	void ShowRecordStatus(bool force);
	
	void ShowMute(bool force);
	void ShowSolo(bool force);

	// List
	void ShowAutomation(bool force);
	void ShowAutomationMode();

	// Record Settings
	void ShowRecordSettings();
	
	CShowPeak outpeak,m_outpeak;

	Edit_ArrangeList *editor;
	Seq_Track *track;
	AudioChannel *channel;

	char *namestring;
	int startnamex,bgcolour,solostatus,record_tracktype;	
	bool trackselected,focus,record_status,automationonoff,automationused,mute,frozen,hasinstruments,hasfx,recs[5];
};

class Edit_ArrangeList_AutomationTrack:public guiObject
{
public:
	Edit_ArrangeList_AutomationTrack();

	void ShowTrack(bool refreshbgcolour);
	void ShowName();

	Edit_ArrangeList *editor;
	Seq_Track *track;
	AudioChannel *channel;

	char *namestring;
	AutomationTrack *automationtrack;
	int startnamex;
};

class Edit_ArrangeList:public EventEditor
{
public:
	Edit_ArrangeList(Edit_Arrange *);

	void InitTabs();
	void InitGadgets();
	void Init();
	void DeInitWindow();
	void RefreshObjects(LONGLONG type,bool editcall);
	void RefreshRealtime();

	void Gadget(guiGadget *);

	void ScrollTo(Seq_Track *);
	void KeyDown();
	void KeyDownRepeat();
	void MouseClickInTracks(bool leftmouse);
	void MouseDoubleClickInTracks(bool leftmouse);

	void BuildTrackList();
	void ShowList();
	void ShowVSlider();
	void FreeEditorMemory();
	void ShowMode();

	OListCoosY trackobjects;
	guiGadget_Tab *list;
	guiGadget *g_list,*g_recording,*g_recordingget,*g_recordingset;
	Edit_Arrange *editor;

	bool mode; // true==recording
	bool getmode;
};

#endif