#ifndef CAMX_DRUMEDITOR_H
#define CAMX_DRUMEDITOR_H 1

#include "editor.h"
#include "drumeditordefines.h"
#include "drumtrackfx.h"
#include "drumevent.h"

#define EDIT_DRUMNAME 1
#define EDIT_DRUMTRACKFX_TICKS 0

class ICD_Drum;
class Edit_DrumList;
class Edit_Drum_Track;

enum {
	// Track
	OBJECTID_DRUMTRACK=OI_LAST,
	OBJECTID_DRUMMUTE,
	OBJECTID_DRUMSOLO,
	OBJECTID_DRUMVOLUME,
	OBJECTID_DRUMTRACKNAME
};

class Edit_Drum_Mute:public guiObject
{
public:
	Edit_Drum_Mute(){id=OBJECTID_DRUMMUTE;}

	Edit_Drum_Track *track;
	bool mutestatus;
};

class Edit_Drum_Solo:public guiObject
{
public:
	Edit_Drum_Solo(){id=OBJECTID_DRUMSOLO;}

	Edit_Drum_Track *track;
	bool solostatus;
};

class Edit_Drum_Volume:public guiObject{
public:
	Edit_Drum_Volume(){id=OBJECTID_DRUMVOLUME;}
	Edit_Drum_Track *track;
	int volume;
};

class Edit_Drum_Name:public guiObject{
public:
	Edit_Drum_Name(){id=OBJECTID_DRUMTRACKNAME;}
	Edit_Drum_Track *track;
};

class Edit_Drum_Track:public guiObject{
	friend class Edit_Drum;
	friend class Edit_DrumEditorEffects;

public:
	Edit_Drum_Track()
	{
		id=OBJECTID_DRUMTRACK;
		volumeobject=0;
		mute.track=solo.track=name.track=this;
		mute.deleteable=solo.deleteable=name.deleteable=false;
	}

	void ShowTrackRaster();
	void ShowTrack();
	void ShowGFXRaster(guiBitmap *);
	void ShowEvents(guiBitmap *);

	Drumtrack *track;

	Edit_Drum_Mute mute;
	Edit_Drum_Solo solo;
	Edit_Drum_Name name;

	Edit_Drum_Volume *volumeobject;
	Edit_Drum *editor;

	int GetTrackValue(int y);

	int namex2,dataoutputx,dataoutputx2,startnamex,dataoutputy2;
	bool trackselected,datadisplay;

private:	
	void ShowMute(bool blit=false);
	void ShowSolo();
	void ShowVolume();
	void ShowName();
};

class Edit_Drum_Drum:public Object
{
public:
	enum{
		NOSTART=1,
		NOEND=2,
		MOUSEOVERLEFT=4,
		MOUSEOVERRIGHT=8
	};

	Edit_Drum_Drum(Edit_Drum *ed,Seq_SelectionEvent *se,ICD_Drum *n,guiBitmap *b){editor=ed;sevent=se;drum=n;eflag=flag=0;infodrum=false;bitmap=b;}
	void Draw();

	Colour rgb;
	Edit_Drum *editor;
	Seq_SelectionEvent *sevent;
	guiBitmap *bitmap;
	ICD_Drum *drum;
	int x,y,x2,y2,eflag;
	bool infodrum;
};

class Edit_Drum: public EventEditor_Selection
{
	friend class Drummap;

public:
	Edit_Drum();

	void SetFocusTrack(Drumtrack *);
	void MouseClickInTracks(bool leftmouse);

	void DeleteDrums();
	void FindAndDeleteDrums(ICD_Drum *);

	void InitMouseEditRange();
	void MouseClickInDrums(bool leftmouse);
	void MouseDoubleClickInDrums(bool leftmouse);
	void MouseMoveInDrums(bool leftmouse);
	void MouseReleaseInDrums(bool leftmouse);

	void RenameDrumMap(Drummap *,int px,int py);
	void SelectDrumMap(Drummap *);

	bool ZoomGFX(int zoomy,bool horiz=false);
	char *GetToolTipString1(); //v
	char *GetToolTipString2(); //v

	void ChangeToDrummap(Drummap *);
	void Goto(int to); //v

	void ShowEvents(); // v
	void NewActiveTrack(Drumtrack *);
	void PlayFocusTrack();
	void UserMessage(int msg,void *par);

	//GUI
	void NewZoom() // v
	{
		RefreshStartPosition();
	}

	void RefreshStartPosition(); // v
	void MouseWheel(int delta,guiGadget *);

	void ConvertDrumsToNotes();
	void ConvertNotesToDrums();

	void CreateNewDrumTrack(Drumtrack *clone);
	void DeleteDrumTrack(Drumtrack *);
	bool CreateDrum();
	void DeleteDrum();

	void KeyDown();
	void KeyDownRepeat();

	void KeyUp();
	void Gadget(guiGadget *);

	bool FindDrumEventsInsideMap(Seq_Song *,Drumtrack *);
	EditData *EditDataMessage(EditData *);
	void CreateMenuList(guiMenu *);

	void CreateWindowMenu();
	guiMenu *CreateMenu();
	void CreateGotoMenu();

	void ShowMenu();

	Edit_Drum_Track *FindDrumTrackAtYPosition(int y);
	void ShowDrumsHoriz(int flag);

	void DeInitWindow();

	void Paste();

	bool EditCancel();
	void Init();
	void InitGadgets();
	void ShowTracks();

	void ShowOverview();
	void ShowOverviewVertPosition(int *y,int *y2);
	void AddStartY(int addy);
	void NewYPosition(double y,bool draw);

	void SetMouseMode(int mode,OSTART mp,Drumtrack *track=0);
	void AutoScroll();
	void RefreshEvents();
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	char *GetWindowName();
	void RefreshObjects(LONGLONG type,bool editcall);

	Edit_Drum_Track *FindTrack(Drumtrack *);
	Edit_Drum_Track *FirstTrack();

	void SetName(Drumtrack *,char *newname);

	// Cursor
	ICD_Drum *FindDrumUnderCursor();
	ICD_Drum *FindDrumUnderMouse(int flag=0);

	void ClearCursor();
	void ShowCursor();
	void SetCursorToMousePosition();
	void PlayCursor();

	ICD_Drum cursor;
	void BuildTrackList();
	void ShowVSlider();

	OList drums;
	OListCoosY trackobjects;

	Edit_DrumList *drumlist;
	Drumtrack *focustrack,*modestarttrack,*mstrack,*metrack;
	guiGadget_Time *infonote_time;
	guiGadget_Tab *tracks;
	guiGadget_CW *eventsheader,*eventsgfx;
	guiGadget *drummap;

	int getcountselectedevents,getcountevents;
	bool showlist,showeffects,editvelocity;

private:
	void ShowMoveDrumsSprites();
	void MoveTrack(Drumtrack *,int diff);
	void ShowDrumMapName();

	int zoom_y;
	EF_CreateEvent editevent;
};
#endif