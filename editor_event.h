#ifndef CAMX_EDITOR_EVENT_H
#define CAMX_EDITOR_EVENT_H 1

#include "editor.h"
#include "gmdrums.h"
#include "MIDIfilter.h"

class Edit_Event_Event:public NumberOListStartPosition
{
	friend class Edit_Event;

public:
	Edit_Event_Event();

	char *GetTool1String();
	Edit_Event_Event *NextEvent(){return (Edit_Event_Event *)next;}

	void RefreshRealtime(OSTART spp,bool active);
	void Draw(bool single=false);
	void DrawGFX();
	void DrawLength(OSTART start,OSTART end,int x,int x2);
	bool CanEditCurve();

	Seq_Event *seqevent;
	Edit_Event *editor;
	double realtime_h2;
	int show_y2,gfx_x,gfx_x2,eflag,fillcolour,
		lenx[8],lenx2[8],lenindex;

	bool set,grey,realtimeactive,activeevent;

private:
	void ShowChannel();
	void ShowStatus(char *);
	void ShowByte1(char *,int ib1);
	void ShowByte2(char *,int ib2);
	void ShowByte1(int);
	void ShowByte2(int);
	void ShowByte3(int);
	void ShowLength(char *l);
	void ShowInfo(char *);
};

class Seq_Pos;

class Edit_Event:public EventEditor_Selection
{
	friend Edit_Event_Event;

public:
	Edit_Event();

	void InitTabs(guiGadget_TabStartPosition *);
	void ScrollTo(Seq_Event *);

	void EditEditorPositions(guiGadget *);
	void EndOfPositionEdit(guiGadget *,OSTART diff);

	char *GetWindowName();
	bool FindAudioEventOnDisplay();
	bool EditCancel();
	char *GetToolTipString1();

	void Goto(int to);
	Seq_SelectionEvent *Goto_Special(int to);
	void GotoEnd();
	void AddSpecialGoto(guiMenu *);
	bool ZoomGFX(int zoomy,bool horiz=false);
	
	void ShowOptionsMenu();
	void ShowEventsHoriz(int flag);
	void SetStartPosition(OSTART pos);
	void ResetGadgets()
	{
		createeventgadget=filtergadget=infotext=0;
	}

	Edit_Event_Event *FindEventInsideEditor(Seq_Event *); // v

	Edit_Event_Event *FirstEvent();
	Edit_Event_Event *LastEvent();
	Edit_Event_Event *FindEvent(int x,int y);

	void NewZoom(); // v

	void Gadget(guiGadget *); //v
	void KeyDown();
	void KeyDownRepeat();
	void MouseWheel(int delta,guiGadget *);

	void MouseClickInEvents(bool leftmouse);
	void MouseReleaseInEvents(bool leftmouse);

	void StartOfNumberEdit(guiGadget *);

	void SetMouseMode(int newmode);
	void SoloPattern(); // v
	void AfterSolo();
	void ShowAllEvents(int flag=0);
	void InitGadgets();

	void CreateWindowMenu();
	void CreateMenuList(guiMenu *);
	guiMenu *CreateMenu();
	void CreateGotoMenu();

	void ShowSlider();
	void Init();
	void DeInitWindow();
	void ShowHeader();
	OObject *BuildEventList();
	void ShowEvents();
	void ShowEventFilter();

	void InitShowEvents();
	void OnInit();

	void ShowEventsGFX();

	void ShowMenu();
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	void RefreshStartPosition();
	void RefreshObjects(LONGLONG type,bool editcall);
	void RedrawDeletedPattern();
	
	void ShowCreateButton();
	void DeltaY(guiGadget_TabStartPosition *);

	guiGadget_TabStartPosition *eventsheader,*events;
	guiGadget_CW *eventsgfx;
	OListCoosY eventobjects;

	int createeventtype,createselection,firstevent_index,getcountselectedevents,getcountevents;
	bool showgfx;

private:
	void ShowEventsRealtime();
	void ClearEventsRealtime();
	void CreateEvent();
	void SendFirstSelectedEvent();
	bool KeepEventinMid(Seq_SelectionEvent *,bool scroll=true);
	void SelectAndEdit(bool rightmousebutton);

	Seq_SelectionEvent *modestartevent;
	EditData * EditDataMessage(EditData *);

	void ShowCursorEvent();
	void ShowFilter();

	Seq_SelectionEvent *cursorevent;

	int cursor,editdata; // User edit

	OSTART solostartposition;

	int cursorevent_index,cursor_index,editstart_x,editstart_y,
		notetype,
		createchannel; // 1-17

	bool moveevents,lengthchanged,selecttype,showonlyselectedevents,setsolostart,dontshowtrackslider;

	guiGadget *filtergadget,*createeventgadget,*infotext;
	guiGadget *notes,*control,*cpress,*ppress,*prog,*pitch,*sysex;

	MIDIFilter displayfilter,checkfilter,cmpfilter;
	Edit_Event_Event *curveevent;

	// GM
	guiMenu *gmmenu,*gfxonoffmenu,*onlyselected;
	GMMap gmmap;
};
#endif