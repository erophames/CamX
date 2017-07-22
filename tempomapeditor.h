#ifndef CAMX_TEMPOEDITOR_H
#define CAMX_TEMPOEDITOR_H 1

#include "editor.h"

class Edit_Tempo;
class Seq_Tempo;

class Edit_Tempo_Tempo:public Object
{
public:
	Edit_Tempo_Tempo(Edit_Tempo *ed,Seq_Tempo *t,guiBitmap *b,int px,int py){editor=ed;tempo=t;eflag=flag=0;bitmap=b;initx=px;inity=py;}
	void Draw();

	Edit_Tempo *editor;
	guiBitmap *bitmap;
	Seq_Tempo *tempo;

	int initx,inity,x,y,x2,y2,eflag;
};

class Edit_Tempo:public EventEditor
{
	friend GUI;
	friend class Edit_TempoList;

public:
	Edit_Tempo();

	void MouseWheel(int delta,guiGadget *);
	void InitMouseEditRange();
	
	bool ZoomGFX(int zoomy,bool horiz=false);
	char *GetToolTipString1();

	void CreateGotoMenu();
	void Goto(int to);

	void LoadTempoMap();
	void SaveTempoMap();
	void SelectAllTempos(bool on);
	
	void ShowMenu();
	
	bool EditCancel();
	void Init();

	void CreateWindowMenu();
	guiMenu *CreateMenu();
	void CreateMenuList(guiMenu *);
	void AddEditMenu(guiMenu *);

	void DeInitWindow();
	void SetMouseMode(int m);
	void KeyDown();
	void Gadget(guiGadget *);
	void MouseClickInTempos(bool leftmouse);
	void MouseDoubleClickInTempos(bool leftmouse);
	void MouseMoveInTempos(bool leftmouse);
	void MouseReleaseInTempos(bool leftmouse);

	void FindAndDeleteTempos(Seq_Tempo *);
	void ShowSlider();

	EditData *EditDataMessage(EditData *);

	void KeyDownRepeat();
	void ReleaseEdit();
	void ShowOverview();
	void ShowTempos();
	void ShowMoveTempoSprites();

	void RefreshObjects(LONGLONG type,bool editcall); // v
	
	void AddStartY(int addy);
	void AutoScroll();
	void NewZoom();
	void RefreshStartPosition();
	
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	
	void UserMessage(int msg,void *par);
	
	int ConvertTempoToY(double tempo);
	double ConvertYToTempo(int);

	void ShowGFX();

	Edit_TempoList *tempolisteditor;
	guiGadget_CW *tempogfx;
	guiGadget *range,*g_tapvalue,*settapvalue;

	bool selectmode,selecttype,initstarty,tapinit;

private:
	void CreateTempo(OSTART position,double ntempo);
	void DeleteTempos();
	void ShowTapValue();
	
	Edit_Tempo_Tempo *FindTempoUnderMouse(int flag=0);
	
	Edit_Tempo_Tempo* FirstTempo() {return (Edit_Tempo_Tempo *)tempos.GetRoot();}
	
	OListCoosY tempogfxobjects;
	OList tempos;

	OSTART movediff;
	double starttempo,moveobjects_vert,tapvalue;
	double modestarttempo,tempoperypixel,mstempo,metempo;
	int tempogfxrange;
	LONGLONG lasttempotapsystime;
};
#endif
