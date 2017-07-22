#ifndef CAMX_SCOREEDITOR_H
#define CAMX_SCOREEDITOR_H 1

#include "editor.h"

#include "objectevent.h"
#include "guiheader.h"

class Edit_Score: public EventEditor_Selection
{
public:
	Edit_Score()
	{
		editorid=EDITORTYPE_SCORE;
		editorname="Score";
	
		/*
		inbound=false;

		focustrack=0;
		starttrack=0;
		
		zoom_y=20;

		frame_tracks.bpen=COLOUR_GREY_LIGHT;
		
		// FX
		showeffects=true;
		
		headerflag=HEADERFLAG_SHOWZOOM;
		
		// Init Track Fx
		trackfx.drumeditor=this;
		trackfx.frame=&frame_fx;
		
		patternselection.buildfilter=SEL_INTERN;
		
		cursor.ostart=0;
		cursor.drumtrack=0;
		cursorsprite.staticsprite=true;
		
		drummap=0;

		frame_tracks.settingsvar=&mainsettings->drum_trackswidth;

		SetStartFrames();
		*/
	}

	/*
	long MouseOverEditorFrame(int mousex,int mousey)
	{
		if(timeline && frame_drums.ondisplay==true && frame_drums.CheckIfInFrame(mousex,mousey)==true)
			return timeline->ConvertXPosToTime(mousex);

		return -1;
	}

	void SetStartFrames()
	{

	}

	void Goto(int to); //v
	void SelectAll(bool on); //v
	void ShowEvents(int y=-1); // v
	void ShowEvent(Seq_SelectionEvent *e,bool direct); // virtual

	void ClearMemory()
	{

	}

	void NewActiveTrack(Drumtrack *track);
	void PlayFocusTrack();

	void DisableFrame()
	{
		frame_drums.ondisplay=false;
		frame_tracks.ondisplay=false;
	}

	//GUI

	void NewZoom(int zoompos) // v
	{
		// ShowDrumsHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_HEADER);
	}

	//	void RefreshEvent(Seq_Event *e);

	void RefreshStartPosition(); // v
	void MouseButton(int flag);

	void MouseMove(bool inside);
	void KeyDown();
	void KeyUp(char nVirtKey);
	void Gadget(guiGadget *gadget);

	EditData *EditDataMessage(EditData *data);

	void CheckPopMenu();

	guiMenu *CreateMenu();

	void ShowMenu();
	void SetMouseMode(int m);
	void ShowDrumsHoriz(int flag);

	bool EditEvent();

	void FreeMemory();

	void ShowMouse(int time); // v

	void InitFrames();
	void Init();
	void RefreshRealtime();
	void RefreshObjects(LONGLONG type,bool editcall);

	void ShowAllEvents(int flag=0) // v
	{
	if((flag&NOBUILD_REFRESH)==0)
	patternselection.BuildEventList(SEL_INTERN,0); // Mix new List, events maybe moved/deleted
	
	  ShowDrums();
	  }

	  */

	//Edit_Frame frame_fx;
	//Edit_Frame frame_tracks;
	//Edit_Frame frame_drums;
	
private:
	void ShowMoveDrumsSprites();
	void ShowRaster();

	int zoom_y;

	EF_CreateEvent editevent;
};
#endif