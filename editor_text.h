#ifndef CAMX_EDITOR_TEXT_H
#define CAMX_EDITOR_TEXT_H 1

#include "editor.h"
#include "textandmarker.h"

class Edit_Text_Frame
{
public:
	Edit_Text_Frame(){zoom_y=20;}
	//Edit_Frame position,textname,textstring;
	int zoom_y;
};

class Edit_Text_Text:public NumberOList
{
	friend class Edit_Text;

public:
	Edit_Text_Text()
	{
		set=false;
		grey=false;
		init=false;
	}

	Edit_Text_Text *NextText(){return (Edit_Text_Text *)next;}
	void Draw(bool single=false);

	Edit_Text *editor;
	Seq_Text *text;
	int y,y2;
	bool set,grey,init;
};

#define NOEVENTEDIT 0

class Seq_Pos;

class Edit_Text: public EventEditor
{
	friend Edit_Text_Text;

public:
	enum en_Cursor{
		CURSORPOS1000, // Measre/hour
		CURSORPOS0100, // /min
		CURSORPOS0010,// sec
		CURSORPOS0001, // frame
		CURSORPOS00001// qframe
	};

	Edit_Text();

	bool InitEdit(Seq_Event *);
	void ClickOnNumberObject(NumberObject *);
	void EditNumberObject(NumberObject *,int flag);
	void EditNumberObjectReleased(NumberObject *,int flag);
	void ShowAllTextTexts();

	bool ZoomGFX(int zoomy,bool horiz=false);
	void EditTextPosition();
	void ResetGadgets()
	{
		infotext=0;
	}

	void MouseWheel(int delta,guiGadget *);
	void DisableFrame()
	{
		//	frame.frame_events.ondisplay=false;
	}

	Edit_Text_Text *FirstText(){return (Edit_Text_Text *)textsineditor.GetRoot();}
	Edit_Text_Text *LastText(){return (Edit_Text_Text *)textsineditor.Getc_end();}
	Edit_Text_Text *FindText(int x,int y);

	void NewZoom() // v
	{
	}

	EditData *EditDataMessage(EditData *);//v
	void Gadget(guiGadget *); //v
	void KeyDown();
	void KeyUp();
	void CheckMouseButtonDown(); //v
	void MouseButton(int flag); //v
	void MouseMove(bool inside);

	Edit_Text_Text *FindTextText(int x,int y);
	void SelectAllTexts(bool on);
	void ShowAllText(int flag=0) //
	{
		ShowTexts();
		if((flag&NOBUILD_REFRESH)==0)ShowCursorText();
	}

	void ShowMouse(OSTART time) // v
	{

	}
	char *GetWindowName();
	void InitGadgets();
	void Goto(int to);
	guiMenu *CreateMenu();
	void DeleteTexts();
	
	void Init();
	
	void FreeMemory();
	void ClearTextDisplay();
	void ShowTexts();
	void ShowMenu();
	//void SetMouseMode(int newmode,Edit_Frame *);
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	void RefreshObjects(LONGLONG type,bool editcall);
	void RefreshText(Seq_Text *);
	bool Scroll(int diff,bool refreshdisplay);

private:
	bool EditNumberObject_Text(Edit_Text_Text *,NumberObject *,Seq_Text *);
	void EditText(Seq_Text *t);
	bool KeepTextinMid(Seq_Text *,bool scroll=true);
	void EditTexts(int what,int diff);
	void ShowCursorText();
	int GetY2(int yh){return yh;}

	Seq_Text *edittext,*cursortext;
	int cursor,editdata;
	bool movetexts,lengthchanged,selectmode,selecttype;
	int cursor_index,firsttext_index,cursortext_index,numberofdisplaytexts,getcounttexts,getcountselectedtexts,
		editstart_x,editstart_y,position_1000_x,position_0100_x,position_0010_x,position_0001_x,position_0001QF_x;
	Edit_Text_Frame frame;
	OList textsineditor;
	guiGadget *filtergadget,*infotext;
};
#endif