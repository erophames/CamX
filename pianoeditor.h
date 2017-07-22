#ifndef CAMX_PIANOEDITOR_H
#define CAMX_PIANOEDITOR_H 1

#include "editor.h"
#include "objectevent.h"
#include "colourrequester.h"

enum
{
	OBJECTID_TRACKTYPE=100,
	OBJECTID_TRACKCHANNEL,
	OBJECTID_TRACKBYTE1
};

class Edit_Piano;
class Edit_PianoWave;


class PWaveEvent:public Object
{
public:
	PWaveEvent(Seq_SelectionEvent *e){seqevent=e;}
	PWaveEvent *NextPEvent(){return (PWaveEvent *)next;}
	Seq_SelectionEvent *seqevent;
};

class Edit_PianoWave_Type:public guiObject
{
public:
	Edit_PianoWave_Type(){
		id=OBJECTID_TRACKTYPE;
	}
};

class Edit_PianoWave_Channel:public guiObject
{
public:
	Edit_PianoWave_Channel(){
		id=OBJECTID_TRACKCHANNEL;
	}
};

class Edit_PianoWave_Byte1:public guiObject
{
public:
	Edit_PianoWave_Byte1(){
		id=OBJECTID_TRACKBYTE1;
	}
};

class Edit_Piano_Note:public Object
{
public:
	enum{
		NOSTART=1,
		NOEND=2,
		MOUSEOVERLEFT=4,
		MOUSEOVERRIGHT=8
	};

	Edit_Piano_Note(Edit_Piano *ed,Seq_SelectionEvent *se,Note *n,guiBitmap *b){editor=ed;sevent=se;note=n;eflag=flag=0;infonote=false;bitmap=b;}
	void Draw();
	void Init(OSTART songposition); // -1 no check

	Colour rgb;
	Edit_Piano *editor;
	Seq_SelectionEvent *sevent;
	guiBitmap *bitmap;
	Note *note;
	int x,y,x2,y2,eflag;
	bool infonote;
};

class Edit_PianoWave:public guiWindow
{
public:
	Edit_PianoWave(Edit_Piano *);

	void SetSet(int);
	void ShowWaveChannel();
	void ShowWaveStatus();
	void ShowWaveControl();
	void Init();
	void ShowWaveEvents();

	void Gadget(guiGadget *);
	PianoSettings *GetSet();

	Edit_Piano *editor;
	guiGadget *g_wave,*g_control,*g_set[5];
	guiGadget_Number *g_channel;
	int set;
};

class Edit_Piano:public EventEditor_Selection
{	
	friend Edit_Piano_Note;

public:
	Edit_Piano();
	
	void Paste();
	void ShowVeloMode();

	char *GetWindowName();
	bool ZoomGFX(int zoomy,bool horiz=false);
	char *GetToolTipString1(); //v
	char *GetToolTipString2(); //v
	char *GetMenuName(){return "Event";}

	void ResetKeysOnDisplay()
	{
		startkey=lastkey=127;

		for(int i=0;i<128;i++) // Reset
		{
			keyondisplay[i]=keyisc[i]=false;
		}
	}

	void Goto(int to); //v

	void ShowEvents();
	void ShowWaveEvents();
	void RefreshEvents();

	void NewZoom() // v
	{
		RefreshStartPosition();
	}

	void AfterSolo();
	void SelectAllKeys(int key);
	void RefreshStartPosition(); // v
	void NewYPosition(double y,bool draw);

	void ShowPianoHoriz(int flag);
	void KeyDown();
	void KeyDownRepeat();

	void RefreshObjects(LONGLONG type,bool editcall);
	void RedrawDeletedPattern();
	
	void CreateWindowMenu();
	void CreateMenuList(guiMenu *);
	guiMenu *CreateMenu();
	void CreateGotoMenu();

	void ShowMenu();
	void UserMessage(int msg,void *par);
	void DeInitWindow();
	bool EditCancel(bool dontchangenotelength=true);
	void Init(); // -1 no check
	void InitGadgets();
	void InitNewTimeType();
	void MouseWheel(int delta,guiGadget *);

	void ShowSlider();
	void ShowSizeNotesSprites();
	void ShowMoveNotesSprites();

	//GUI
	void SetMouseMode(int mode,OSTART mp,int key=-1);
	void AutoScroll();

	void AddStartY(int addy);
	void SetStartKey(int key);
	void InitMouseEditRange();

	void Gadget(guiGadget *);
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	void RefreshFocusEvent();
	void RefreshActiveKeys(bool blit);

	int FindKeyAtPosition(int y); // -1 no key
	void ShowOverview();
	void ShowOverviewVertPosition(int *y,int *y2);

	void ShowKeys();
	void ShowRaster();

	// Cursor
	Note *FindNoteUnderCursor();
	void ClearCursor();
	void ShowCursor();
	void SetCursorToMousePosition();
	void PlayCursor();
	int FindKeyAtKeyMousePosition();
	
	void MouseClickInKeys(bool leftmouse);
	void MouseDoubleClickInKeys(bool leftmouse);
	void MouseUpInKeys(bool leftmouse);

	// Notes
	void MouseClickInNotes(bool leftmouse);
	void MouseDoubleClickInNotes(bool leftmouse);
	void MouseMoveInNotes(bool leftmouse);
	void MouseReleaseInNotes(bool leftmouse);

	void DeltaY(guiGadget *);
	void MouseClickInData(bool leftmouse);
	void MouseMoveInData(bool leftmouse);
	void MouseReleaseInData(bool leftmouse);
	void ShowFocusEvent();
	void ShowDefaultNoteLength();

	PWaveEvent *FirstPWaveEvent(){return (PWaveEvent *)PWaveEvents.GetRoot();}

	OListCoosY keyobjects;
	
	guiGadget *velomode;
	guiGadget_CW *keys,*noteraster,*wavetrack,*waveraster;
	OSTART default_notelength,reset_oldnotelength,overviewrangestart,overviewrangeend;
	double piano_keyheight;
	int modestartkey,getcountselectedevents,getcountevents;
	int newchannel,newvelocity,newvelocityoff;

private:
	void CheckDefaultNoteLength();

	void FillMouseKey();
	void PlayMouseMoveKey();
	Edit_Piano_Note *FindNoteUnderMouse(int flag=0);
	int PlayKeyNote(bool check,int key);

	Note *CreateNote();
	void FindAndDeleteNotes(Note *);

	void DeleteNote();
	void DeleteNotes();

	EF_CreateEvent editevent;
	Note cursor;
	OList notes,PWaveEvents; // Seq_SelectionEvent

	Edit_PianoWave *wave;

	guiGadget_Number *infonote_chl,*infonote_key,*infonote_velo,*infonote_velooff;
	guiGadget_Time *infonote_time,*infonote_end,*infonote_length,*newnote_len;
	guiGadget *notechannel,*newnote_chl,*newnote_velo,*newnote_velooff;
	Note *newmousenote;

	OSTART focus_start,focus_end,focus_length;
	UBYTE focus_channel,focus_key,focus_velo,focus_velooff;

	OSTART newmouselength;
	int lastplaykey,lastfillkey,startkey,lastkey,numberofkeys,
		mskey,mekey,
		keyposx[128],keyposx2[128],
		keyposy[128],keyposy2[128],
		keyposwhitey[128],keyposwhitey2[128],
		notetype;

	bool keyblack[128],keyondisplay[128],keyactive[128],keyisc[128],
		showwave,default_notelengthuseprev,mouseoverchangelength_left,
		mouseoverchangelength_right,playkeys,playmouseovernotes,editvelocity,setstarttomid;
};

#endif