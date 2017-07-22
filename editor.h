#ifndef CAMX_EDITOR_H
#define CAMX_EDITOR_H 1

#include "defines.h"
#include "guiwindow.h"
#include "toolbox.h"
#include "patternselection.h"
#include "editframe.h"
#include "settings.h"
#include "seqpos.h"
#include "editortypes.h"

#define HEAD_POSITION_1xxx 0
#define HEAD_POSITION_x1xx 1
#define HEAD_POSITION_xx1x 2

#define SIZEV_OVERVIEW (3*maingui->GetButtonSizeY())
#define SIZEV_HEADER (2*maingui->GetButtonSizeY())

#define AUTOSCROLL_TIME_EDITOR 555

enum {
	MOUSEQUANTIZE_MEASURE,
	MOUSEQUANTIZE_BEAT,
	MOUSEQUANTIZE_1,
	MOUSEQUANTIZE_12,
	MOUSEQUANTIZE_14,
	MOUSEQUANTIZE_18,
	MOUSEQUANTIZE_16,
	MOUSEQUANTIZE_FREE,
	MOUSEQUANTIZE_ZOOM,
	MOUSEQUANTIZE_FRAME,
	MOUSEQUANTIZE_QFRAME
};

enum {
	ADDSTEP_OFF,
	ADDSTEP_1,
	ADDSTEP_12,
	ADDSTEP_14,
	ADDSTEP_18,
	ADDSTEP_16,
	ADDSTEP_DYNAMIC,
	ADDSTEP_MEASURE
};

enum{
	EM_RESET,

	EM_SELECT,
	EM_CREATE,
	EM_CUT,
	EM_DELETE,
	EM_EDITTIME,
	EM_EDIT,
	EM_SIZEY, // Tracks etc...
	EM_LASTSTANDARD, // 0-> EM_LASTSTANDARD

	EM_MOVEPATTERN,
	EM_SIZEPATTERN_LEFT,
	EM_SIZEPATTERN_RIGHT,
	EM_MOVETRACK,
	EM_SELECTOBJECTSSTART,
	EM_SELECTOBJECTS,
	EM_MOVEOS,
	EM_SIZENOTES_LEFT,
	EM_SIZENOTES_RIGHT,
	EM_SIZENEWNOTE,

	EM_SELECTRANGE,
	EM_SELECTRANGE_LEFT,
	EM_SELECTRANGE_RIGHT,
	EM_SELECTRANGE_MOVE,
	EM_MOVEOVERVIEW,
	EM_MOVEAUTOMATION,

	EM_MOVECYCLE, // <->
	EM_SETCYCLE_START, // <-
	EM_SETCYCLE_END, // ->
	EM_SETCYCLE_SETCYCLE, // |----x

	EM_MOVESONGPOSITION,
	EM_MOVESONGSTOPPOSITION,

	EM_EDIT_FADEIN, // Fades
	EM_EDIT_FADEOUT,
	EM_EDIT_VOLUME,

	EM_SPECIAL
};

class Seq_Song;
class Seq_Event;
class Seq_Track;
class Seq_Pattern;
class AudioFile;
class UndoFunction;
class Edit_BoundEditor;
class Editor;
class MIDIEffects;
class MIDIOutDevice;
class AudioPeakBuffer;
class Edit_Arrange_Track;
class Edit_Wave_DefinitionTrack;
class AutomationTrack;
class guiGadget;
class guiMenu;
class SliderCo;
class guiBitmap;
class guiObject;
class AudioRealtime;
class MIDIPattern;
class UndoEdit;
class Seq_Tempo;

class Editor:public guiWindow
{
public:
	Editor ()
	{			
		editorname=0;
		undo_editevents=0;
	}

	enum{ // GotoMenu
		GOTO_FIRST,
		GOTO_LAST,
		GOTO_FIRSTSONG,
		GOTO_LASTSONG,
		GOTO_FIRSTEVENTTRACK,
		GOTO_LASTEVENTTRACK,
		GOTO_FIRSTSELECTED,
		GOTO_LASTSELECTED,
		GOTO_SONG,
		GOTO_MEASURE,
		GOTO_CYCLESTART,
		GOTO_CYCLEEND,
		GOTO_SOLOTRACK,
		GOTO_MUTETRACK,
		GOTO_FOCUSTRACK,
		GOTO_FOCUSPATTERN,

		// Special
		GOTO_FIRSTTRACK,
		GOTO_LASTTRACK,
		GOTO_FIRSTSELECTEDTRACK,
		GOTO_LASTSELECTEDTRACK,

		GOTO_FIRST_NOTE,
		GOTO_FIRST_PROGRAM,
		GOTO_FIRST_PITCH,
		GOTO_FIRST_CONTROL,
		GOTO_FIRST_SYSEX,
		GOTO_FIRST_CHANNELPRESSURE,
		GOTO_FIRST_POLYPRESSURE,

		GOTO_LAST_NOTE,
		GOTO_LAST_PROGRAM,
		GOTO_LAST_PITCH,
		GOTO_LAST_CONTROL,
		GOTO_LAST_SYSEX,
		GOTO_LAST_CHANNELPRESSURE,
		GOTO_LAST_POLYPRESSURE,

		GOTO_FIRST_MUTE,
		GOTO_LAST_MUTE,
		GOTO_FOCUS,
	};

	virtual void DeInitWindow(){}
	virtual UndoEdit *GetUndoEditEvents(){return undo_editevents;} // Default Seq_Event
	virtual void SetUndoEditEvents(UndoEdit *ee){undo_editevents=ee;}

	UndoEdit *undo_editevents;
};

// Flag ShowAllEvents
#define NOBUILD_REFRESH 1
#define BLIT_REFRESH 2

// Flag ShowHort
#define SHOWEVENTS_HEADER 1
#define SHOWEVENTS_EVENTS 2
#define SHOWEVENTS_TRACKS 4
#define SHOWEVENTS_INIT 8
#define SHOWEVENTS_PIANOWAVETRACK 16
#define SHOWEVENTS_OVERVIEW 32

void Editor_Header_Callback(guiGadget_CW *g,int status);

class EventEditor:public Editor
{
public:
	enum{
		GOTOMEASURE_ID,
		EDIT_TIME, // Edit Data
		EDIT_MOVEMEASURE,
		EDIT_COPYMEASURE,

		// Now Editor IDS...
		EDITOREDITDATA_ID
	};

	EventEditor();

	virtual void OtherRealtime(){}
	virtual void ReleaseEdit(){}
	virtual bool ZoomGFX(int zy,bool horz){return false;}
	virtual bool EditCancel(){return false;}
	virtual void ResetMoreEventFlags(){}
	virtual void Goto(int to){}
	virtual Seq_SelectionEvent *Goto_Special(int to){return 0;}
	virtual void GotoEnd(){}
	virtual void AddSpecialGoto(guiMenu *){}
	virtual void RemoveEvent(Seq_Event *){};
	virtual void SelectAll(bool on){}
	virtual EditData *EditDataMessage(EditData *data){return Editor_DataMessage(data);}
	virtual void RemovePattern(Seq_Pattern *,bool redraw=false){}
	virtual void ShowEvent(Seq_SelectionEvent *,bool direct=false){}
	virtual void SoloPattern(){}
	virtual void AfterSolo(){}
	virtual void NewZoom(){}
	virtual void ShowMouse(OSTART time){}
	virtual char *GetToolTipString1(){return 0;}
	virtual char *GetToolTipString2(){return 0;}
	virtual void RefreshStartPosition(){}
	virtual void NewYPosition(double h,bool draw=true){}
	virtual void ShowTooltip(OSTART time){DisplayTooltip(time);}
	virtual void ResetGadgets_SelectPattern(){}
	virtual void AddFunctionsMenu(guiMenu *){}
	virtual void AddEditMenu(guiMenu *){}
	virtual guiGadget *Editor_Gadget(guiGadget *);
	virtual EditData *SpecialEdit(EditData *data){return data;}
	virtual Seq_SelectionList *GetPatternSelection(){return 0;}
	virtual bool IfEvents(){return false;}
	virtual bool IfSelectedEvents(){return false;}
	virtual void Paste(){}
	virtual OSTART GetTimeLineGrid();

	guiGadget *SpecialGadgets(guiGadget *);

	void ResetMouseMode(int nmode=-1);
	void MouseClickInOverview(bool leftmouse);
	virtual void MouseClickInOverview_Ex(bool leftmouse){}

	OSTART GetOverviewTime(int x);

	void DoubleClickInEditArea();
	void InitSpecialDisplay(){InitDisplay();}
	char *MouseQuantizeString();
	void CreatePos(OSTART);
	bool CheckSelectPlayback(Seq_Event *);
	void StartSong(); //v
	void StopSong(); //v
	void UserEdit(); //v
	bool CheckStandardGoto(int to);
	bool CheckStandardDataMessage(EditData *);
	void DeActivated();//v

	void RefreshTimeLine();//v
	void RefreshSMPTE(); // v
	void RefreshMeasure();

	void DeleteMouseTooltip();//v
	void RefreshMarker();
	void CreateMarkerMenu(guiMenu *);
	void AddSetMarkerPositions(guiMenu *);
	void AddSteptoSongPosition();

	void ResetAllGadgets();

	void ShowCycleAndPositions(guiGadget_CW *);
	virtual void ShowSpecialEditorRange(){}

	void MouseClickInTimeLine(int status);
	void MouseDoubleClickInTimeLine();
	void MouseReleaseInTimeLine(int status);
	bool CheckMouseClickInEditArea(guiGadget_CW *);
	int CheckMouseInTimeLine();

	void ResetEditorGadgets();
	EditData *Editor_DataMessage(EditData *);
	bool RefreshEventEditorRealtime(bool force=false);
	void RefreshEventEditorRealtime_Slow();

	void ShowEditorHeader(guiGadget_CW *);
	void DoAutoScroll();

	void ResetSlider(){mouseqgadget=horzgadget=vertgadget=vertzoomgadget=horzzoomgadget=0;}

	void AddStepMenu(guiMenu *);
	void InitMouseMoveSPP(guiGadget_CW *);
	void InitSongStopMarker(guiGadget_CW *);

	bool CheckMouseMovePosition(guiGadget_CW *);

	guiGadget *CheckControlBox(guiGadget *);

	void ShowEditorMenu();

	void SetEditorMode(int nmode);
	void CreateEditorMenu(guiMenu *);
	void AddMouseQuantizeMenu(guiMenu *);
	void AddTimeDisplayMenu(guiMenu *);

	void Editor_KeyDown();
	void Editor_KeyUp();

	guiGadget *Editor_Slider(guiGadget *);
	void ScrollSliderHoriz(OSTART diff); // <->

	void SyncWithOtherEditors();
	void TimeChange();

	bool ScrollMeasures(int measures);

	// Select 
	void SelectAllEvents(int flag);
	bool NewStartPosition(OSTART newposition,bool setwaitrealtime,Seq_SelectionEvent *startevent=0);
	OSTART QuantizeEditorMouse(OSTART start);
	virtual void NewDataZoom(){}

	void InitMousePosition();
	bool EditorMouseMove(bool inside);
	bool Edit_MouseWheel(int delta);
	void AddEditorSlider(SliderCo *hor,SliderCo *vert,bool datazoom=false);

	void DisplayTooltip(OSTART time);
	void ToolTipOff();
	void ClearTooltip();
	void RefreshToolTip();

	void ClearMouseWindow();
	// Slider
	void RefreshTimeSlider();
	virtual void RefreshTimeSlider_Ex(){}

	int GetMouseQuantizeFlag();
	void PasteMouse(guiGadget_CW *);

	guiControlBox guictrlbox;	
	guiGadget_Slider *horzgadget,*horzzoomgadget,*vertgadget,*vertzoomgadget;
	guiGadget *timebutton,*patternedit,*patterneditsolo,*mouseqgadget,*syncgadget;
	guiGadget_Number *datazoomgadget;

	Seq_Pattern *insertpattern; // Create new MIDI Events here
	OSTART modestartposition,msposition,msendposition,modecyclestartposition,modecycleendposition,eventeditortime,eventeditormousetime;

	int /*modestartx,*/modestarty,rt_mousequantize,rt_windowdisplay,mousequantize,addsongpositionstep,headerflag,edit_lenidenx;	
	guiMenu *mousequantizemenu,*displaymenu,*followmenu,*syncmenu;

	bool showlist,mouseoversongposition,refreshslider,bound,refresheditorevents,editsolopattern,
		syncsongposition,showquarterframe,gotocalled,
		songmodepossible;

	void ShowOverviewCycleAndPositions();
	virtual void ShowOverviewPositions_Ex(){}
	virtual void ShowOverviewVertPosition(int *y,int *y2){}

	int ConvertTimeToOverviewX(OSTART time);

	guiGadget_CW *overview,*timesprite;
	OSTART overviewtime,overviewlenght;
	int overview_y,overview_y2;

	guiBitmap tooltip;
	int tooltipx,tooltipy,tooltipw,tooltiph,spacesize,overviewsppx;

	Seq_Pos pos_measure,pos_samples,pos_seconds,pos24,pos25,pos48,pos50,pos2997,pos30,
		pos2997df,pos30df,pos239,pos249,pos599,pos60,*displayusepos;

	void InitDisplay();

	enum{
		EO_TIMECHANGE=1,
		EO_DATACHANGE=2
	};

	int editnumberobjectflag;
	char space[4],mousequantstring[64];

protected:
	void CheckRealtimeHeader();
	void AddStandardGotoMenu();
	void EditSongStartPosition(int x,int y);
};

class EventEditor_Selection:public EventEditor
{
public:
	virtual void ShowEvents(){}

	Seq_SelectionList *GetWindowPatternSelection(){return &patternselection;} //v

	bool IfEvents(){return patternselection.GetCountOfEvents()>0?true:false;}
	bool IfSelectedEvents(){return patternselection.GetCountofSelectedEvents()>0?true:false;}

	virtual void RedrawDeletedPattern()
	{
		patternselection.BuildEventList(SEL_ALLEVENTS,0); // Mix new List
		RefreshObjects(0,false);
	}

	virtual void ShowAllEvents(int iflag=0)
	{
		if((iflag&NOBUILD_REFRESH)==0)
			patternselection.BuildEventList(patternselection.buildfilter,0,patternselection.buildicdtype); // Mix new List, events maybe moved/deleted

		//ShowEvents();
	}

	EditData *SpecialEdit(EditData *);
	void Goto(int to);
	void AddEditMenu(guiMenu *);
	void AddFunctionsMenu(guiMenu *);
	void SelectAll(bool on);
	int SelectDoubleEvents(int selflag);

	void MoveSelectedEventsToTick(OSTART tick,bool copy);

	// Pattern Selection
	void ShowSelectionPattern();
	void InitSelectionPatternGadget();
	void RefreshSelectionGadget();

	void ResetGadgets_SelectPattern()
	{
		timebutton=0;
		patternedit=0;
		patterneditsolo=0;
	}

	void ReleaseEdit();

	guiGadget *Editor_Gadget(guiGadget *);
	bool RemoveTrackFromWindow(Seq_Track *); //v

	void ResetMoreEventFlags();
	void SelectAllEvents();
	void CopySelectedEvents(); // selected Events
	void MuteSelectedEvents(bool onoff);
	void UnmuteEvents();
	void RemovePattern(Seq_Pattern *,bool redraw=false);
	void MoveSelectedEventsToMeasure(bool copy);

	void DeInitWindow();
	void OtherRealtime();

	void RemoveEvent(Seq_Event *);
	void SelectEvent(Seq_Event *,bool nodeselected=false);

	bool OpenEditSelectedEvents(int index,Seq_Event *,guiGadget_Tab *g=0);
	bool OpenEditSelectedEvents_Tab(guiGadget_Tab *,Seq_Event *);

	void EditSelectedEventsDelta_Tab(int);

	void AddEventEditorMenu(guiMenu *); // Standard Editor Menu

	Seq_SelectionList *GetPatternSelection(){return &patternselection;}
	Seq_SelectionList patternselection; // Pattern,Event list
	int eventcounter_display;
};

class menu_gotoeventeditor:public guiMenu
{
public:
	menu_gotoeventeditor(EventEditor *ar,int t)
	{
		editor=ar;
		gototype=t;
	}

	void MenuFunction()
	{		
		editor->Goto(gototype);
	} //

	EventEditor *editor;
	int gototype;
};

#endif
