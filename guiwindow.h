#ifndef CAMX_GUIWINDOW_H
#define CAMX_GUIWINDOW_H 1

#define WINDOWMODE_NORMAL 0
#define WINDOWMODE_INIT 1
#define WINDOWMODE_RESIZE 2
#define WINDOWMODE_DESTROY 4

#define AUTOSCROLL_UP 1
#define AUTOSCROLL_UPFAST 256
#define AUTOSCROLL_UPTURBO 512

#define AUTOSCROLL_DOWN 2
#define AUTOSCROLL_DOWNFAST 1024
#define AUTOSCROLL_DOWNTURBO 2048

#define AUTOSCROLL_LEFT 4
#define AUTOSCROLL_LEFTFAST 8
#define AUTOSCROLL_LEFTTURBO 16

#define AUTOSCROLL_RIGHT 32
#define AUTOSCROLL_RIGHTFAST 64
#define AUTOSCROLL_RIGHTTURBO 128

#include "guiobjects.h"
#include "guigadgets.h"
#include "winzoom.h"
#include "guimenu.h"
#include "guiform.h"
#include "toolbox.h"

class guiWindowSetting;
class guiTimeLine;
class guiObject;
class guiMenu;
class Seq_Song;
class EditData;
class guiScreen;
class Seq_Track;
class Seq_Pattern;
class GUIMessage;
class Seq_Tempo;
class Seq_SelectionList;
class LMIDIEvents;
class InsertAudioEffect;
class AudioObject;
class AudioPattern;

enum{
	REFRESHEVENT_LIST=1,
	REFRESHSIGNATURE_DISPLAY=8
};

class NumberObject:public Object
{
public:
	NumberObject();
	NumberObject *NextNumberObject(){return (NumberObject *)next;}
	bool CheckIfInside(int mx,int my);
	int GetX2();

#ifdef DEBUG
	char n[4];
#endif

	Object *editobject;
	int x,y,x2,y2,editid,from,to,step;
	bool invert,added,init,ondisplay;
};

enum{
	OL_START
};

class NumberOList:public guiObject
{
public:
	NumberObject *FirstNumberObject(){return (NumberObject *)numbers.GetRoot();}
	int GetIndexColour();

	OList numbers;
	int type,index;
};

class NumberOListStartPosition:public NumberOList
{
public:
	NumberOListStartPosition()
	{
		type=OL_START;
		for(int i=0;i<8;i++)mouseover[i]=false;
	}

	void Init(Seq_Song *s,guiBitmap *b){song=s;bitmap=b;}
	void DrawPosition(OSTART position,int x,int x2,Object *);

	TimeString timestring;
	NumberObject time[8];

	Seq_Song *song;
	Object *posobj;
	int timemode;
	bool mouseover[8];
};

class NumberOListRef:public Object
{
public:
	NumberOListRef(NumberOList *ol,guiGadget *g){number=ol;gadget=g;}
	NumberOListRef *NextOListRef(){return (NumberOListRef *)next;}
	NumberObject *FirstNumberObject(){return number?number->FirstNumberObject():0;}
	NumberObject *FindNumberObject(int mx,int my);
	NumberOList *number;
	guiGadget *gadget;
};

enum{
	OV_HORZ,
	OV_VERT,
	OV_BOTH
};

class guiWindow:public Object,public guiForm
{
	friend class GUI;

public:
	guiWindow();
	~guiWindow();

#ifdef WIN32
	virtual Object *GetDragDrop(HWND wnd,int index){return 0;}
	virtual void DragDrop(guiGadget *){}
#endif

	virtual void CheckIfClose(){}
	virtual void StartSong(){}
	virtual void StopSong(){}
	virtual void UserEdit(){}
	virtual bool RefreshCheck(){return false;}
	virtual void DragDropFile(char *file,guiGadget_CW *){}
	virtual void InitPaste(EditBuffer *){}
	virtual bool SaveAble(){return false;}
	virtual void RefreshTimeLine(){}

	virtual void RefreshSMPTE(){}
	virtual void RefreshMeasure(){}

	virtual void DeleteMouseTooltip(){}
	virtual void SongNameRefresh();
	virtual bool CheckIfObjectInside(Object *){return false;};
	virtual void MouseButtonLeft(int flag){}
	virtual void MouseButtonRight(int flag){}
	virtual void MouseButton(int flag){}
	virtual void ReleaseMouse(){}
	virtual void Gadget(guiGadget *){}
	virtual bool GadgetListView(guiGadget_ListView *,int x,int y){return false;}
	virtual void DoubleClickGadget(guiGadget *){}
	virtual void MenuSelect(int id){}
	virtual void KeyUp(){}
	virtual void KeyDown(){}
	virtual void KeyDownRepeat(){}
	virtual void AutoScroll(){}
	virtual void RedrawGfx(){} // Paint Refresh
	virtual void ShowTime(){}
	virtual char *GetWindowName(){return windowname;}
	virtual void RefreshMenu();
	virtual void KillFocus(){};
	virtual void Activated(){}
	virtual void DeActivated(){}
	virtual void ClearMouseLine(){}
	virtual Seq_SelectionList *GetWindowPatternSelection(){return 0;}

	void EditDataValue(EditData *);
	virtual EditData *EditDataMessage(EditData *data){return data;} // return 0 if data was used

	virtual void UserMessage(int msg,void *par){}
	virtual void RefreshRealtime(){}
	virtual void RefreshRealtime_Slow(){}
	virtual void Init(){}
	void InitOpenWindow(guiWindowSetting *set=0);
	virtual void InitSpecialDisplay(){};
	virtual void CreateMenuList(guiMenu *){} // PopMenu or Title Menu
	virtual guiMenu *CreateMenu(){popmenu=0;return menu=0;}
	virtual guiMenu *CreateMenu2(){popmenu=0;return menu=0;}
	virtual bool RemoveTrackFromWindow(Seq_Track *){return false;}
	virtual void RemovePatternFromWindow(Seq_Pattern *){}
	virtual void DeInitWindow(){} // Free Memory + Window Close	
	virtual void MouseWheel(int delta,guiGadget *){}
	virtual void RefreshObjects(LONGLONG objecttypes,bool editcall){}

	virtual void LearnFromMIDIEvent(LMIDIEvents *){}
	virtual void LearnFromPluginChange(InsertAudioEffect *,AudioObject *,OSTART time,int index,double value){}
	virtual void RemoveAudioEffect(InsertAudioEffect *){}

	void NewTimeFormat();
	virtual void InitNewTimeType(){}

	// Number Objects
	virtual void EditNumberObject(NumberObject *,int flag){}
	virtual void EditNumberObjectReleased(NumberObject *,int flag){}

	virtual void ScrollHoriz(){}
	virtual char *GetMenuName(){return "Menu";}

	bool CanbeClosed();
	virtual void RemoveSong(Seq_Song *){}

	guiMenu *NewWindowMenu();
	virtual void CreateWindowMenu(){}

	void InitAutoVScroll();
	void ScrollVert(int deltay);

	virtual void RemoveChildWindows(){}
	virtual int GetInitHeight(){return 0;}
	virtual void SetWindowName();
	virtual void RefreshTimeSlider(){}

	bool IsFocusWindow();
	bool IsActiveWindow();

	void GenerateOSMenu(bool drawmenu);
	void CreateButton(int id,HWND);
	void Init(int flag)
	{
		winmode=flag;
		Init();
		winmode CLEARBIT flag;
	}

	Seq_Song *WindowSong(){return song;}

	void OnCreate();
	void PopMenuTrack(Seq_Track *);
	void CreateTimeTypePopup(guiGadget_Time *gt=0);
	void Form_CreateFormObjects();
	void Form_RepaintChild(guiForm_Child *);
	void Form_FreeChildObjects(guiForm_Child *);
	void OnNewSize(int x,int y);
	int GetScreenMouseX();
	int GetScreenMouseY();

	int GetWindowMouseX();
	int GetWindowMouseY();
	void SetMouse(int x,int y);

	int GetWinPosX();
	int GetWinPosY();
	int GetWinWidth();
	int GetWinHeight();
	int GetSizeOfString(char *);
	int GetEditorID(){return editorid;}
	bool IsWinEventEditor();
	virtual OSTART QuantizeEditorMouse(OSTART ostart){return ostart;}
	OSTART GetAutomationTime();

	void DrawDBBlit(guiGadget_CW *,guiObject *);
	inline void DrawDBBlit(guiGadget_CW *db){if(db)db->DrawGadgetBlt();}
	inline void DrawDBBlit(guiGadget_CW *db,guiGadget_CW *db2)
	{
		if(db)db->DrawGadgetBlt();
		if(db2)db2->DrawGadgetBlt();
	}
	inline void DrawDBBlit(guiGadget_CW *db,guiGadget_CW *db2,guiGadget_CW *db3)
	{
		if(db)db->DrawGadgetBlt();
		if(db2)db2->DrawGadgetBlt();
		if(db3)db3->DrawGadgetBlt();
	}

	inline void DrawDBSpriteBlit(guiGadget_CW *db)
	{
		if(db)db->DrawSpriteBlt();
	}
	inline void DrawDBSpriteBlit(guiGadget_CW *db,guiGadget_CW *db2)
	{
		if(db)db->DrawSpriteBlt();
		if(db2)db2->DrawSpriteBlt();
	}
	inline void DrawDBSpriteBlit(guiGadget_CW *db,guiGadget_CW *db2,guiGadget_CW *db3)
	{
		if(db)db->DrawSpriteBlt();
		if(db2)db2->DrawSpriteBlt();
		if(db3)db3->DrawSpriteBlt();
	}

	inline void DrawDB(guiGadget_CW *db)
	{
		if(db)db->DrawGadgetEx();
	}

	void AddUndoMenu();
	void ResetMenu(){menu_multiedit=0;}
	void SetKey(int v){nVirtKey=v;}
	void SetAutoScroll(int mode,int x,int y,int x2,int y2);
	void SetAutoScroll(int mode,guiGadget_CW *);
	void ResetAutoScrolling();
	void CheckAutoScroll();
	void CheckXButtons();
	virtual void DoAutoScroll(){}
	virtual void InitMouseEditRange(){}
	void DoStandardYScroll(guiGadget_CW *);
	virtual void AddStartY(int addy){}
	guiMenu *DeletePopUpMenu(bool createnew=false);
	void RemoveMenu();
	void ShowPopMenu();
	void OwnerDrawGadget(int,LPDRAWITEMSTRUCT);
	void OwnerDrawGadgetSelect(int,LPDRAWITEMSTRUCT);
	void CheckGadget(guiGadget *);

	void CheckGadget(
#ifdef WIN32
		HWND child,
#endif
		int code,
		int id);

	guiWindow *PrevWindow() {return (guiWindow *)prev;}
	guiWindow *NextWindow() {return (guiWindow *)next;}

	void ResetRefresh();
	void SetMouseCursor(int type);
	void Clear();
	void guiSetWindowText(char *);
	void WindowToFront(bool activate);

	void SetWindowSize(int w,int h);
	void CloseHeader();
	void SetWindowDefaultSize();
	void SetSong(Seq_Song *newsong){song=newsong;}

	OSTART GetMousePosition();
	LONGLONG GetMouseSamplePosition(AudioPattern *);

	//Objects
	guiObject *AddObject();
	guiObject *FirstObject(){return (guiObject *)objectslist.GetRoot(); }
	void AddNumberOList(NumberOList *,guiGadget *g=0);
	void DeleteAllNumberObjects(guiGadget *);
	void DeleteAllNumberObjects();
	void CheckNumberObjects(guiGadget *);
	bool IsPositionObjectClicked(guiGadget *,bool leftmouse);
	bool IsEndOfPositionEditing(guiGadget *);
	void StartPositionEditing(NumberOListStartPosition *,int index,guiGadget *);
	bool EditPositions(int deltay); 

	virtual void StartOfNumberEdit(guiGadget *){}
	virtual void EditEditorPositions(guiGadget *){}
	virtual void EndOfPositionEdit(guiGadget *,OSTART diff){}

	bool SetZoomGFX(int,bool horiz);
	NumberOListRef *FirstNumberOListRef(){return (NumberOListRef *)numberobjectslistref.GetRoot();}

	Seq_Pos numbereditpos;
	guiGadget *numbereditgadget;
	
	int numbereditindex /*,numberedit_mx,numberedit_my*/;
	LONGLONG numbereditsamples;
	double datazoom;
	OSTART numbereditposition,numbereditposition_diff,numbereditposition_sum;
	bool numberedited;

	virtual void CreateGotoMenu(){}
	guiGadget *CheckToolBox(guiGadget *);

	void DrawHeader();
	int ConvertWindowDisplayToTimeMode();
	bool CheckInLasso(int x,int y,int x2,int y2);
	bool CanMoveX();
	bool CanMoveY();

	guiGadgetList glist;
	guiToolBox guitoolbox;
	guiBitmap bitmap;

	guiOList guiobjects;
	OList numberobjectslistref;
	guiMenu *menu_timedisplay,*menu,*popmenu,*menu_multiedit;
	guiWindow *calledfrom,*parentwindow;
	guiGadget *mousedowngadget;
	guiForm_Child *parentformchild,*bindtoform;
	NumberObject *activenumberobject;
	guiGadget_CW *editarea,*editarea2,*editarea3,*header,*autoscrollwin;
	guiTimeLine *timeline;
	guiScreen *openscreen,*screen;
	guiWindow *usemenuofwindow;
	char *windowname,*editorname;
	LONGLONG samplestartposition,sampleendposition;
	OSTART startposition,endposition;
	guiZoom *zoom; // X zoom

	// Edit
	Object *editobject;

	void ResetEditSum(){editsum=0;}
	double editsum;

#ifdef CAMXGUIHTREADS
	guiWindow *nextcorewindow;
#endif

	void InitVKey(int k){nVirtKey=k;}

	int
		overviewmode,
		exparentformx,exparentformy,
		mx,my, // Mouse/Cursor
		editactivenumberobject_mousestartx,editactivenumberobject_mousestarty,
		editnumber_diffx,editnumber_diffy,activenumberobject_flag,
		moveframeflag,mouseframeflag_selected,refreshflag,
		mousemode,selectedmousemode,
		mouseclickdowncounter,
		windowtimeformat, // Measure, SMPTE...
		generalMIDIdisplay,mousepointer,
		nVirtKey,refreshmousebuttontimer,xbuttoncounter,
		autoscrollstatus,autoscrollmode,autoscrollx,autoscrolly,autoscrollx2,autoscrolly2,
		win_screenposx,win_screenposy,zoomy,minzoomy,zoomybuffermul,winmode,rrt_slowcounter,
		left_mousekey,mid_mousekey,right_mousekey,realtimewaitcounter,blinkcounter,
		mouseeditx,mouseeditx2,mouseedity,mouseedity2,
		editmode // Arrange,Mixer
		;

	bool editactivenumberobject_left,repeatkey,hotkey,closepopmenu,refreshgui,
		dialogstyle,borderless,
		refreshed,
		dontchangesettings,ignoreleftmouse,refresfmenu,hide,
		addtolastundo,zoomvert,autoscroll,horzscroll,vertscroll,skipdeletepopmenu,
		refreshmousebuttondown,// refresh Mouse Button Down
		refreshmousebuttonright,
		mousexbuttonleftdown,mousexbuttonrightdown,
		songmode,
		mainvolumemousemove,
		editrefresh, // something changed ?
		editrefreshdone, // private Window Refresh
		bottominfo, // bar on bottom of window
		followsongposition,followsongposition_stopped,
		isstatic,
		editornameisdeleteable,
		hasownmenu,
		learn,
		xmove,ymove;

	void SetMinZoomY(int mzy)
	{
		minzoomy=mzy;
		if(zoomy<minzoomy)zoomy=minzoomy;
	}

protected:
	Seq_Song *song;
	int editorid;

private:
	OList objectslist; // objects inside Window
};

#endif