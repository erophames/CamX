#ifndef CAMX_EDITOR_MARKER_H
#define CAMX_EDITOR_MARKER_H 1

#include "editor.h"
#include "textandmarker.h"

class Edit_Marker_Text:public NumberOListStartPosition
{
	friend class Edit_Marker;

public:
	Edit_Marker_Text();
	Edit_Marker_Text *NextText(){return (Edit_Marker_Text *)next;}

	void Draw(bool single=false);

	Seq_Marker *marker;
	Edit_Marker *editor;
	int eflag;
	bool set,grey;
};

#define NOEVENTEDIT 0

// Cursor
#define CURSOR_POS_1000 0 // Measre/hour
#define CURSOR_POS_0100 1 // /min
#define CURSOR_POS_0010 2// sec
#define CURSOR_POS_0001 3 // frame
#define CURSOR_POS_00001 4// qframe

class Seq_Pos;

class Edit_Marker: public EventEditor
{
	friend Edit_Marker_Text;

public:
	Edit_Marker();

	void DeInitWindow();
	void ShowVSlider();
	void BuildMarkerList();
	void ShowList();
	void InitTabs();
	bool InitEdit(Seq_Event *);

	bool ZoomGFX(int zoomy,bool horiz=false);

	void SelectAllMarker(bool);

	void NewZoom() // v
	{
	}
	EditData *EditDataMessage(EditData *);//v
	void Gadget(guiGadget *); //v

	void KeyDown();
	void KeyUp();

	void CheckMouseButtonDown(); //v
	
	void MouseWheel(int delta,guiGadget *);

	void ShowMouse(OSTART time) // v
	{
	}
	void DeleteMarkers();
	void InitGadgets();
	void Goto(int to);
	guiMenu *CreateMenu();
	
	void Init();
	void ShowMenu();
	char *GetWindowName();
	
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	void RefreshObjects(LONGLONG type,bool editcall);
	void RefreshText(Seq_Marker *);
	bool Scroll(int diff,bool refreshdisplay);

	guiGadget_TabStartPosition *list;

private:
	void EditText(Seq_Marker *);
	bool KeepTextinMid(Seq_Marker *,bool scroll=true);
	void SelectAndEdit(bool rightmousebutton);
	
	void ShowCursorText();

	int GetY2(int yh){return yh;}

	OListCoosY markerobjects;

	guiGadget *filtergadget,*infotext;
	Seq_Marker *cursortext,*editmarker;	
	
	int cursor_index,firsttext_index,cursortext_index,getcountmarker,getcountselectedmarker,editstart_x,editstart_y,numberofdisplaytexts,
		position_1000_x,position_0100_x,position_0010_x,position_0001_x,position_0001QF_x,
		endposition_1000_x,endposition_0100_x,endposition_0010_x,endposition_0001_x,endposition_0001QF_x;
	int cursor,editdata;

	bool movetexts,lengthchanged,selectmode,selecttype;
};
#endif