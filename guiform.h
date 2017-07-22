#ifndef CAMX_UIFORM_H
#define CAMX_UIFORM_H 1

#ifdef WIN32
#include <afxwin.h>
#endif

#define DOCK_LEFT 1 // Window or Pre Form/Window
#define DOCK_TOP 2
#define DOCK_RIGHT 4
#define DOCK_BOTTOM 8
#define DOCK_STATICWIDTH 16
#define DOCK_STATICHEIGHT 32
#define DOCK_TOOLBARTOP 64
#define DOCK_SCROLLABLE 128

#define MAXFORMCHILDS 8

#define SPLITTERVERTICAL_H maingui->borderheight
#define SPLITTERHORIZON_W maingui->borderwidth

#define TOPBARHEIGHT (5*maingui->GetButtonSizeY()+(5+3)*ADDYSPACE)
#define TOPBARHEIGHT_SMALL (3*maingui->GetButtonSizeY()+(3+3)*ADDYSPACE)
#define PLUGINTOPBARHEIGHT maingui->GetButtonSizeY(3)

#define EDITOR_SLIDER_SIZE_VERT maingui->scrollwidth
#define EDITOR_SLIDER_SIZE_HORZ maingui->scrollheight

enum{
	FORM_SCREEN,

	FORM_PLUGIN,
	FORM_PLAIN1x1,

	FORM_VERT1x2,
	FORM_VERT1x2_2small,
	FORM_VERT1x3,

	FORM_HORZ2x1SLIDERV, // List
	FORM_HORZ1x2SLIDERVTOOLBAR, // MIDI Monitor

	FORM_HORZ2x1,
	FORM_HORZ2x1_SLIDERHV,

	FORM_HORZBAR_SLIDERHV,
	FORM_HORZ2x1BAR_SLIDERHV,
	FORM_HORZ2x1BAR_EVENT_SLIDERHV,
	FORM_HORZ3x1BAR_SLIDERHV,
	FORM_HORZ3x2BAR_SLIDERHV,

	FORM_HORZ4x1BAR_SLIDERHV, // Arrange
	FORM_HORZ4x1BAR_SLIDERHV3, // Piano

	FORM_HORZ2x2BAR_SLIDERHV,

	FORM_HORZ1x2BAR_SLIDERV, // Mixer
	FORM_HORZ1x2BAR_MIXER1TRACK, // Mixer 1 Track
};

#define STARTOFGADGETSX ADDXSPACE
#define STARTOFGADGETSY ADDYSPACE

class guiScreen;
class guiForm;
class guiWindow;
class camxFile;

enum //CH_FLAGS
{
	CHILD_HASWINDOW=1
};

class guiForm_Child
{
public:
	guiForm_Child();

	void BindWindow(guiWindow *);
	void ChangeChild();

	bool InUse();
	bool Disabled();

	guiForm_Child *NextHChildInUse();

	guiForm_Child *PrevVChild();
	guiForm_Child *PrevVChildInUse();
	guiForm_Child *NextVChildInUse();

	guiForm_Child *NextHChild();
	guiForm_Child *NextVChild();

	void SetGXY(int x,int y){gx=x;gy=y;lastheight=0;}
	void SetGX(int x){gx=x;}
	void SetGY(int y){gy=y;}

	int GetYSub(int subindex,int y2);
	void SetGYSub(int subindex,int y2){gy=GetYSub(subindex,y2);}

	void Enable(bool on);

	int GetWidth(int addw=0)
	{
		if(deactivated==true)return 0;
		if(width+addw<0)return 0;
		if(width+addw>width)return width;
		return width+addw;
	}

	int GetHeight(int addh=0)
	{
		if(deactivated==true)return 0;
		if(height+addh<0)return 0;
		if(height+addh>height)return height;
		return height+addh;
	}

	int GetX2(int addw=0){return GetWidth(addw-1);}
	int GetY2(int addh=0){return GetHeight(addh-1);}

	guiScreen *screen;
	guiForm *form;

#ifdef WIN32
	HWND fhWnd,sizervert_hWnd,sizerhorz_hWnd,oldcapturehWnd;
#endif

	guiWindow *bindwindow;

	double percentofparentwindow_h,percentofparentwindow_v,percentofparentwindow_v_toggle,
		percentofparentwindow_buffer_h[MAXFORMCHILDS][MAXFORMCHILDS],percentofparentwindow_buffer_v[MAXFORMCHILDS][MAXFORMCHILDS];

	int x,y,width,height,ox,oy,owidth,oheight,fx,fy,flag,
		subwidth,subheight,
		sizerstartx,sizerstarty,sizediffx,sizediffy,sizerhorzsubh,sizerversubw,
		type,dock,
		gx,gy,lastheight,maxx2,maxy2; // Window Gadget Positions

	bool enable,deactivated,forceredraw,userselect,sizechanged,hsizer,vsizer,bindtohorzslider,bindtovertslider,horzsliderhide,vertsliderhide,disablewithrecalc,fromdesktoptoscreen;
};

class guiForm
{
public:
	guiForm();

	virtual void Form_Create(){InitForms(FORM_PLAIN1x1);}
	virtual void Form_CreateFormObjects(){};
	virtual void Form_RepaintChild(guiForm_Child *){}
	virtual void Form_FreeChildObjects(guiForm_Child *){}
	virtual void InitAutoVScroll(){}
	virtual void InitFormSize(){}

	void Form_OnCreate();
	void Form_NewSize(int w,int h,bool force);
	void InitForms(int t){type=t;}
	void InitFormEnable(int x,int y,bool on)
	{
		forms[x][y].enable=on;
	}

	void EditForm(int x,int y,int flag)
	{
		forms[x][y].flag=flag;
	}

	void RecalcForm();
	void ReDraw(bool force);

	int InitY(int starty,int maxheight,int bottomheight,guiForm_Child *);
	
	void FormEnable(int x,int y,bool enable);
	void FormYEnable(int y,bool enable);

	void MoveForm();
	void ToggleForm(guiForm_Child *,bool leftmouse);
	void SetChildHeight(guiForm_Child *,int height);

	void RecalcChildPercent(guiForm_Child *);
	int GetMaxWidth(bool subsizer);
	void GetMaxHeight(int *max,int *bottomheight);

	void SetFormXY(int x,int y);
	void SetFormSizeFlags(WPARAM sflag,int nW,int nH);

	void ReadForm(camxFile *);
	void SaveForm(camxFile *);
	void InitMaxInfo(LPARAM);

	guiForm_Child *GetForm(int x,int y){return &forms[x][y];}

	guiForm_Child forms[MAXFORMCHILDS][MAXFORMCHILDS]; // X/Y

#ifdef WIN32
	HWND hWnd; // Frame
	HINSTANCE hInst;
	DWORD flagex,style;
#endif

	int forms_horz,forms_vert,
		fx,fy,
		width,height,
		save_width,save_height,
		minwidth,minheight,maxwidth,maxheight,
		type, //FORM_PLAIN1x1 etc...
		formcounter,
		topbaroffsety,
		subrightwidth,subbottomheight;

	bool form_screen,autovscroll,closeit,maximized,minimized,ondesktop,resizeable;

private:
	
	void BufferOldPositions();
	void RefreshUseBuffer();

};
#endif