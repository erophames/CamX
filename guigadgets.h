#ifndef CAMX_GUIGADGETS_H
#define CAMX_GUIGADGETS_H 1

#include "object.h"
#include "defines.h"
#include "guigraphics.h"
#include "camxgadgets.h"
#include "guiform.h"
#include "timestring.h"

#define CAMX_BUTTONNAME "BUTTON"
#define CAMX_CHILDWINDOWNAME "CAMXCW"

class GUI;
class guiWindow;
class EventEditor;
class AudioSystem;
class guiGadget;
class Groove;
class Editor;
class Seq_Song;
class AudioEffects;
class Colour;
class guiForm;
class guiObject;
class guiGadget_Win;
class NumberOListStartPosition;

#define MODE_NORMAL 0
#define MODE_SELECTED 1
#define MODE_TOGGLE (1<<1)
#define MODE_TOGGLED (1<<2)
#define MODE_BLACK (1<<3)
#define MODE_ALERT (1<<4)
#define MODE_ADDDPOINT (1<<5)
#define MODE_SUBRIGHT (1<<6)
#define MODE_TEXTCENTER (1<<7)
#define MODE_TOP (1<<8)
#define MODE_RIGHT (1<<9)
#define MODE_BOTTOM (1<<10)
#define MODE_LEFT (1<<11)
#define MODE_STATICWIDTH (1<<12)
#define MODE_STATICHEIGHT (1<<13)
#define MODE_SPRITE (1<<14)
#define MODE_PARENT (1<<15)
#define MODE_DUMMY (1<<16)
#define MODE_BOLD (1<<17)
#define MODE_BKCOLOUR (1<<18)
#define MODE_TEXTWIDTH (1<<19)
#define MODE_LEFTTOMID (1<<20)
#define MODE_MIDTORIGHT (1<<21)
#define MODE_NOMOUSEOVER (1<<22)
#define MODE_OS (1<<23)
#define MODE_PUSH (1<24)
#define MODE_AUTOTOGGLE (1<<25)
#define MODE_INFO (1<<26)
#define MODE_GROUP (1<<27)
#define MODE_MENU (1<<28)
#define MODE_ACTIVATE (1<<29)
#define MODE_STATICTIME (1<<30)
#define MODE_LENGTHTIME (1<<31)

#define VTABLE_1 -5
#define VTABLE_2 -6
#define VTABLE_MIDCENTER -7
#define FORMTYPE_VERTICAL 1
#define FORMTYPE_HORZ 2
#define MAXGADGETSPERWINDOW 1024
#define MAXCHILDWINDOWSPERWINDOW 64

#define MAXGUITABS 16

class SliderCo
{
public:
	SliderCo()
	{
		from=to=pos=0;
		page=1;
		x=y=x2=y2=0;
		offsetx=offsety=0;
		subw=subh=staticwidth=staticheight=0;
		nozoom=false;
	}

	int x,y,x2,y2,from,to,pos,page,offsetx,offsety,formx,formy,subw,subh,staticwidth,staticheight;
	bool horz,nozoom;
};

class guiListBoxButton:public Object
{
public:
	guiListBoxButton(int lid,char *s);
	char *string;
	int id;
};

class guiGadgetKey
{
public:
	guiGadgetKey()
	{
		use=false;
		flag=0;
	}

	void SetKey(int k,int flag=0){key=k;use=true;}

	int key,flag;
	bool use;
};

class guiGadget:public Object
{
	friend class GUI;
	friend class guiGadgetList;

public:
	guiGadget();
	~guiGadget()
	{
		DeInit();
	}

	enum{
		CGT_INSIDE=1,
		CGT_LEFTMOUSEDOWN=2,
		CGT_RIGHTMOUSEDOWN=4
	};

	virtual void Call(int status){};

	guiGadget *CheckGadgetTimer(int mx,int my,guiObject *,int iflag);
	void SetToolTip();
	void DeInit();

	void SetStartMouseY();
	void SetEditSteps(double ey){editstepsy=ey;}
	int InitDeltaY();

	bool CheckIf(guiGadget *c)
	{
#ifdef WIN32
		if(c && c->hWnd==hWnd)
			return true;

		return false;
#endif
	}

	virtual bool IsDoubleBuffered(){return false;}
	virtual void DrawOnInit(){}
	virtual void DrawOnNewSize(){}
	virtual void DrawGadgetEx(){}
	virtual double GetDoublePos(){return 0;}
	virtual LONGLONG GetTime(){return 0;}
	virtual int GetPos(){return 0;}

	virtual void FreeMemory(){}
	virtual void OnClick(){}
	virtual void ResetValue(){} // R Mouse
	virtual void MouseWheel(int delta){}

	virtual void RightMouseUp(){}
	virtual void RightMouseDown(){}
	virtual void DoubleClicked(){}
	virtual void ChangeEditData(OSTART newvalue,bool refreshgui){}
	virtual bool CheckClicked(){return true;}

	virtual void SetMinWidthAndHeight(int *newwidth,int *newheight){}
	virtual void Timer(){}
	bool IsSystemGadget();

	void Toggle(bool on);
	void InitGetMouseMove();
	void InitDelta();
	void ExitDelta();
	
	void SetMouse(int x,int y);
	guiObject *CheckObjectClicked();

	int GetMouseX();
	int GetMouseY();
	
	void On();
	void Off();
	
	int GetDefaultTimeWidth(); 

	void SetColourNoDraw(int fg,int bg)
	{
		useextracolour=fg>=0?true:false;
		fgcolour=fg;
		bgcolour=bg;
	}

	void SetColour(int fg,int bg)
	{
		useextracolour=fg>=0?true:false;
		fgcolour=fg;
		bgcolour=bg;
		DrawGadget();
	}

	void Blt(); // Double to Front
	void Blt(guiObject *); // Double to Fron
	void Blt(int x,int y,int x2,int y2); // Double to Front

	void SetButtonQuickTest();

	void SetKeyFocus();

	void SetButtonQuickText(char *text,bool on);
	void SetButtonQuickOn(bool on);

	void InitNewDoubleBuffer();
	void AddTooltip(char *);

#ifdef _DEBUG
	char n[4];
#endif

	guiGadget *NextGadget(){return (guiGadget *)next;}
	guiGadget *PrevGadget(){return (guiGadget *)prev;}

	void MouseOver(int mx,int my);

	int GetX2(){return GetWidth()-1;}
	int GetY2(){return GetHeight()-1;}

	int GetX(){return mode&MODE_PARENT?x:0;}
	int GetY(){return mode&MODE_PARENT?y:0;}

	int GetWidth(){return x2>=x?(x2-x)+1:1;}
	int GetHeight(){return y2>=y?(y2-y)+1:1;}

	virtual void Enable();
	virtual void Disable();
	virtual void Disabled(){}

	//void Selected();
	void MouseMove();
	virtual bool DeltaY(){return false;}
	void EndMouseMove();

	virtual bool CheckMouseOver(int mx,int my);
	virtual void CheckMouseOverStatus(int mx,int my){}
	void DrawGadget();
	void DrawGadgetEnable();

	int GetBackGroundColour();

	void ChangeButtonImage(int imageID,int flag=-1);
	bool ChangeButtonText(char *,bool force=false);
	void ChangeButtonText(char *,int rgb_fg,int rgb_bkg);

	bool GetBoxStatus(){
		if(index==1)return true;
		return false;
	}

	void ClearCycle();

	// Cycle
	void AddStringToCycle(char *);
	void AddNumberToCycle(int);
	void SetCycleSelection(int);

	void AddMIDIChannelsStrings();
	void AddMIDIKeysStrings(Seq_Song *);
	void AddMIDIRange(int from,int to);

	// Tree
	void AddItemToTree(char *);

	// CheckBox
	void SetCheckBox(bool);

	// String 
	void SetString(char *);
	void CheckString(char *,bool force);

	char *GetString(int length);
	void Delete();
	void DeleteDoubleBuffers();
	int SetVisibleFlag();

	guiGadgetKey key;
	guiGadgetList *guilist;
	Object *object;

	guiBitmap gbitmap,spritebitmap,mixbitmap; // Double/Triple Buffer

	guiGadget_Win *group;
	guiGadget_CW *parent;
	guiGadget *childof,*bottomdock,*linkgadget;

#ifdef WIN32
	HWND hWnd,ttWnd,oldfocushWnd;
	HDC ghDC;
	RECT eraserect;
#endif

	char *text,*string,*tooltext,*quicktext;

	guiGadget *linkclickgadget;
	guiForm_Child *formchild;

	OSTART zoom;

	void EndEdit();

	double editstepsy;

	int dock,x,x2,y,y2,gadgetindex,
		mx,my,
		menuindex,
		defaultwidth,defaultheight,
		fgcolour,bgcolour,
		mouseoverindex,editdataindex,edittabx,
		index,deltay,editmousey,
		gadgetID,camxID,type,buttontype,mode,subvalue[6],subx[6],subx2[6],startValue,editIndex,startmousex,startmousey,
		mouseoverx,mouseovery,format,
		subw,subh,
		staticwidth,staticheight,
		bgcolour_rgb,
		bordercolour_rgb;

	bool on,gadgetinit,useextracolour,staticbutton,sysgadget,selected,disabled,getMouseMove,mouseover,
		deltareturn,
		init,textinit,
		justselected,doubleclick,ownergadget,pushed,
		quicktext_onoff,
		skippaint,
		bordercolour_use,
		horzslider,
		leftmousedown,rightmousedown;
};

class guiGadget_Denumerator;
class Seq_Signature;

class guiGadget_Numerator:public guiGadget
{
public:
	void ShowSignature(Seq_Signature *);
	void Enable();
	void Disable();

	guiGadget_Denumerator *dn;
};

class guiGadget_Denumerator:public guiGadget
{
public:
	guiGadget_Numerator *nn;
};

#define DB_PAINT 0
#define DB_LEFTMOUSEDOWN 1
#define DB_LEFTMOUSEUP (1<<1)
#define DB_RIGHTMOUSEDOWN (1<<2)
#define DB_RIGHTMOUSEUP (1<<3)
#define DB_MOUSEMOVE (1<<4)
#define DB_DOUBLECLICKLEFT (1<<5)
#define DB_PAINTSPRITE (1<<6)
#define DB_KILLFOCUS (1<<7)
#define DB_KEYDOWN (1<<8)
#define DB_KEYUP (1<<9)
#define DB_FREEOBJECTS (1<<10)
#define DB_DELTA (1<<11)
#define DB_CREATE (1<<12)
#define DB_NEWSIZE (1<<13)

class guiWindow;

class guiGadget_Win:public guiGadget
{
public:
	guiGadget_Win(guiWindow *w)
	{
		type=GADGETTYPE_WINDOW;
		win=w;
		toggler=0;
	}

	void SetMinWidthAndHeight(int *newwidth,int *newheight);
	guiWindow *win;
	guiGadget *toggler;
};

class guiGadget_CW:public guiGadget
{
public:
	guiGadget_CW()
	{
		Init();
	}

	guiGadget_CW(int m)
	{
		Init();	
		mode=m;
	}

	bool IsDoubleBuffered(){return true;}
	void Init();
	void DrawOnInit();
	void DrawOnNewSize();
	void DrawGadgetBlt();
	void MixSprite();
	bool DeltaY()
	{
		callback(this,DB_DELTA);
		return deltareturn;
	}

	void DrawGadgetEx()
	{
		DrawGadgetBlt();
	}

	void DrawSpriteBlt();

	void Call(int status)
	{
		callback(this,status);
	}

	void (*callback) (guiGadget_CW *,int status);
	void *from;

	int GetCursorX();

	int SetToXX2(int xp)
	{
		if(xp<0)return 0;
		if(xp>=GetWidth())return GetWidth();
		return xp;
	}

	int SetToYY2(int yp)
	{
		if(yp<0)return 0;
		if(yp>=GetHeight())return GetHeight();
		return yp;
	}

	guiGadget_Slider *scroller_h;
	int vkey;
};

class guiObject_Pref;

class guiGadget_Tab:public guiGadget_CW
{
public:
	guiGadget_Tab(){tabs=0;}

	void FreeMemory()
	{
		ClearTab();
	}

	void InitTabs(int);
	void InitTabWidth(int index,int width);
	void InitTabWidth(int index,char *);

	void InitXX2();
	int GetTabX(int index){return tabx[index];}
	int GetTabX2(int index);
	int GetMouseClickTabIndex();

	void ClearTab();

	guiObject_Pref *FirstGUIObjectPref(){return (guiObject_Pref *)preflist.GetRoot();}
	guiObject_Pref *LastGUIObjectPref(){return (guiObject_Pref *)preflist.Getc_end();}

	OList preflist;
	int tabx[MAXGUITABS],tabx2[MAXGUITABS],tabwidth[MAXGUITABS],tabs;
};

class guiGadget_TabStartPosition:public guiGadget_Tab
{
public:
	guiGadget_TabStartPosition()
	{
		tabstart=0; // default
		startinit=false;
	}

	void InitStartPosition(int tabindex);
	Object *InitEdit(OListCoosY *,double ydeltasteps);

	int timewidth_smpte[5], // SMPTE
		timewidth_time[5],
		tabstart;

	bool startinit;
};

class guiGadget_ListBox:public guiGadget
{
public:
	guiGadget_ListBox(int m)
	{
		type=GADGETTYPE_LISTBOX;
		mode=m;
	}

	void CalcScrollWidth();
	void AddStringToListBox(char *);
	void AddStringToListBox(char *,int id);
	void FreeMemory()
	{
		ClearAllListBoxButtons();
	}
	void ClearAllListBoxButtons();
	int GetListBoxID(int index)
	{
		if(guiListBoxButton *lb=(guiListBoxButton *)listboxstrings.GetO(index))return lb->id;
		return -1;
	}
	void ClearListBox();
	void AddNumberToListBox(int number);
	void SetListBoxSelection(int index);

	OList listboxstrings;
};

class guiListViewText:public Object
{
public:
	guiListViewText(char *);
	char *string;
};

class guiListViewColum:public Object
{
public:
	void FreeMemory();
	OList objects;
};

class guiGadget_ListView:public guiGadget
{
public:
	guiGadget_ListView(int m);
	void AddColume(char *string,int widthmul,bool last=false);
	void AddItem(int col,char *string);
	void ClearListView();
	void FreeMemory();
	void SetSelection(int ix);
	void ChangeText(int x,int y,char *newtext);
	void Refresh(int y);

	OList column;
	int columc;
};

class guiGadget_Slider:public guiGadget
{
public:
	guiGadget_Slider(){type=GADGETTYPE_SLIDER;}

	int GetPos(){return pos;}
	void ChangeSlider(OListCoos *,int page=1);
	void ChangeSlider(int pos);
	void ChangeSlider(int from,int to,int pos,int newpage=1);
	void ChangeSliderPage(int page);
	void DeltaY(int deltay);

	int from,to,pos,page;
	bool horz;
};

class guiGadget_Integer:public guiGadget
{
public:
	guiGadget_Integer(){
		type=GADGETTYPE_INTEGER;
		gtext=0;
	}

	void SetInteger(int i);
	guiGadget *gtext;
	int integer;
	char numberstring[16];
};

class guiGadget_Time:public guiGadget_CW
{
public:
	guiGadget_Time();

	bool IsDoubleBuffered(){return true;}

	void LeftMouseDown();
	void LeftMouseUp();
	void MouseMoveLeftDown();
	void DoubleClickedLeftMouse();

	LONGLONG GetTime();
	void DrawGadgetEx();
	void SetTime(LONGLONG t);
	void SetLength(OSTART spp,OSTART len);
	void Timer();

	void SetMinTime(LONGLONG);
	void SetMaxTime(LONGLONG);

	void CheckTime(LONGLONG t,bool realtime);
	void CheckPreCounter();

	bool DeltaY();
	void MouseWheel(int delta);
	void CheckMouseOverStatus(int mx,int my);
	void BuildPair(guiGadget_Time *gt)
	{
		pairedwith=gt;
		if(gt)
			gt->pairedwith=this;
	}

	void RightMouseUp();

	void ChangeType(int type);
	void ChangeEditData(OSTART nv,bool refreshgui);
	void Disabled();

	TimeString timestring;

	guiGadget_Time *pairedwith;
	LONGLONG samples,mintime,maxtime;
	OSTART t_time,t_length;
	int ttype,precounter,minusx,minusx2,precountertodo;
	bool precountermode,mintimeset,maxtimeset,showsamples,islength,statictimeformat,showprecounter,minusclicked;
};

class guiGadget_Volume:public guiGadget_CW
{
public:
	guiGadget_Volume()
	{
		type=GADGETTYPE_BUTTON_VOLUME;
		addtext=0;
	}

	bool IsDoubleBuffered(){return true;}
	void LeftMouseDown();
	void LeftMouseUp();
	void MouseMoveLeftDown();
	void LeftMouseDoubleClick();

	bool DeltaY();
	void MouseWheel(int delta);
	void ResetValue();
	void DrawGadgetEx();
	void SetPos(double i);
	void FreeMemory(){if(addtext)delete addtext;}
	void RightMouseDown();

	double volume;
	int nrtype;
	char numberstring[NUMBERSTRINGLEN],*addtext;
};

class guiGadget_Number:public guiGadget_CW
{
public:
	guiGadget_Number(){type=GADGETTYPE_BUTTON_NUMBER;}

	bool IsDoubleBuffered(){return true;}
	void LeftMouseDown();
	void LeftMouseUp();
	void MouseMoveLeftDown();
	void LeftMouseDoubleClick();
	bool DeltaY();
	void MouseWheel(int delta);

	double GetDoublePos(){return vnumber;}
	int GetPos(){return (int)vnumber;}
	void ChangeEditData(OSTART newvalue,bool refreshgui);
	void DrawGadgetEx();
	void SetPos(double i);
	void SetPosType(int);
	void SetFromTo(double f,double t)
	{
		vfrom=f;
		vto=t;
	}

	void ChangeEditData(double nv){vnumber=nv;}

	double vfrom,vto,vnumber;
	int nrtype;
	char numberstring[NUMBERSTRINGLEN];
};

#define ADDXSPACE 0
#define ADDYSPACE 1
#define DEFAULTLISTY maingui->GetFontSizeY()+2*ADDYSPACE

#define MOUSERANGEX 4
#define CTRL_XY maingui->GetSizeCtrlXY()
#define CTRL_SMALLXY  maingui->GetSizeSmallCtrlXY()

#define NUMBER_INTEGER 0
#define NUMBER_0 1
#define NUMBER_00 2
#define NUMBER_000 4
#define NUMBER_MIDICHANNEL 8
#define NUMBER_KEYS 16
#define NUMBER_INTEGER_PERCENT 32

class guiGadgetList:public Object
{
	friend class guiGadget;
	friend class Gadgets;

public:	
	guiGadgetList();
	int GetMaxHeight();
	void AddGadget(guiGadget *,bool ownergadget=false);
	void OpenGroup(guiWindow *,guiGadget *gr_Toggler,int flag,Seq_Song *song=0);
	void GroupToggled(guiGadget *);

	void AddLX()
	{
		if(gc)form->gx+=gadgets[gc-1]->GetWidth()+ADDXSPACE;
	}

	void AddLX(int add)
	{
		form->gx+=add+ADDXSPACE;
	}

	void AddLY()
	{
		form->gy+=form->lastheight+ADDYSPACE;
		form->lastheight=0;
	}

	void AddLY(int add)
	{
		form->gy+=form->lastheight+add+ADDYSPACE;
		form->lastheight=0;
	}

	void ResetLX()
	{
		form->gx=STARTOFGADGETSX;
	}

	void Return()
	{
		AddLY();
		ResetLX();
	}

	guiForm_Child *SelectForm(int x,int y);
	void SelectForm(guiGadget_CW *);
	guiForm_Child *GetActiveForm(){return form;}

	int GetLX(){return form->gx;}
	int GetLY(){return form->gy;}

	void Disable(guiGadget *g){if(g)g->Disable();}
	void NextHForm();
	void NextVForm();

	void Resize(int width,int height,guiForm_Child *);
	void FreeObjects(guiForm_Child *);

	void InitXY(guiGadget *,int *y,int *w,int *h);
	void ConvertModeToDock(guiGadget *);
	void InitForm();

	guiGadget* FirstGadget(){return (guiGadget*)guigadgets.GetRoot(); }
	guiGadget* LastGadget(){return (guiGadget*)guigadgets.Getc_end(); }

	guiGadget_CW *AddChildWindow(int oldx,int y,int w,int h,int mode,char *tool,void (*callback) (guiGadget_CW *,int status),void *from);
	guiGadget_Tab *AddTab(int oldx,int y,int w,int h,int mode,char *tool,void (*callback) (guiGadget_CW *,int status),void *from);
	guiGadget_TabStartPosition *AddTabStartPosition(int oldx,int y,int w,int h,int mode,char *tool,void (*callback) (guiGadget_CW *,int status),void *from);

	guiGadget *AddImageButton(int x,int y,int w,int h,int imageID,int gadgetID,int flag,char *tool=0,int key=-1);
	guiGadget *AddImageButtonColour(int x,int y,int w,int h,int imageID,int gadgetID,int flag,int fgcol,int bgcol,char *tool=0,int key=-1);

	guiGadget *AddText(int x,int y,int x2,int y2,char *text,int gadgetID,int mode,char *tool=0);

	guiGadget *AddButton(int x,int y,int w,int h,char *text,int gadgetID,int mode=0,char *tool=0);
	guiGadget *AddButton(int x,int y,int w,int h,int gadgetID,int mode=0,char *tool=0);
	guiGadget *AddButtonColour(int oldx,int y,int w,int h,char *text,int gadgetID,int mode,int fgcol,int bgcol,char *tool=0);

	guiGadget_Volume *AddVolumeButton(int x,int y,int w,int h,int gadgetID,double vol,int flag=0,char *tool=0);
	guiGadget_Volume *AddVolumeButtonText(int x,int y,int w,int h,char *addtext,int gadgetID,double vol,int flag=0,char *tool=0);

	guiGadget_Number *AddNumberButton(int x,int y,int w,int h,int gadgetID,double from,double to,double nr,int type,int flag=0,char *tool=0);

	guiGadget_Numerator *AddSignatureButton(int oldx,int y,int w,int h,int gadgetID_nn,int gadgetID_dn,int mode);

	guiGadget_Time *AddTimeButton(int x,int y,int w,int h,LONGLONG time,int gadgetID,int type,int flag=0,char *tool=0);
	guiGadget_Time *AddLengthButton(int x,int y,int w,int h,OSTART pos,OSTART length,int gadgetID,int type,int flag=0,char *tool=0);

	guiGadget_Slider *AddSlider(SliderCo *,int gadgetID,Object *,char *tool=0);
	guiGadget_Slider *AddSlider(SliderCo *,int gadgetID,int mode,Object *,char *tool=0);

	guiGadget_ListBox *AddListBox(int x,int y,int w,int h,int gadgetID,int mode,char *tool=0,bool readonly=false);
	guiGadget_ListView *AddListView(int x,int y,int w,int h,int gadgetID,int mode,char *tool=0);
	guiGadget *AddCheckBox(int x,int y,int w,int h,int gadgetID,int mode,char *text,char *tool=0);
	guiGadget *AddCycle(int x,int y,int w,int h,int gadgetID,int mode,char *text,char *tool=0);
	guiGadget *AddString(int x,int y,int w,int h,int gadgetID,int mode,char *name,char *text,char *tool=0);
	guiGadget_Integer *AddInteger(int x,int y,int w,int h,int gadgetID,char *text,int integer,char *tool=0);
	guiGadget *AddTree(int x,int y,int x2,int y2,int gadgetID);

	void RemoveGadget(guiGadget *);
	void RemoveGadget(int from,int to);
	void Enable(guiGadget *,bool onoff);

	void RemoveGadgetList();

	void SetHorzScroll(int pos,int range);
	void SetVertScroll(int pos,int range);

	guiGadgetList *PrevGadgetList() {return (guiGadgetList *)prev;}
	guiGadgetList *NextGadgetList() {return (guiGadgetList *)next;}

#ifdef WIN32
	HWND hWnd; // Default Form HWND
#endif

	guiWindow *win;
	guiGadget *gadgets[MAXGADGETSPERWINDOW],*g_cw[MAXCHILDWINDOWSPERWINDOW];
	guiForm_Child *form;
	guiGadget_CW *formgadget;

	int gc,gcwc,scrollh_range,scrollh_pos,scrollv_range,scrollv_pos,form_h,form_v;
	bool dontdelete;

private:
	int ConvertHTable(guiGadget *,int h,int y);
	int ConvertWTable(guiGadget *,int w);
	int ConvertYTable(guiGadget *,int y,int h);
	void AddChildGadget(guiGadget *);

	OList guigadgets; // Gadgets
};
#endif
