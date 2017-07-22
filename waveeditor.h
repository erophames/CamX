#ifndef CAMX_WAVEEDITOR_H
#define CAMX_WAVEEDITOR_H 1

#include "editor.h"
#include "objectevent.h"
#include "mempools.h"
#include "undo.h"
#include "undofunctions.h"
#include "wavetrackfx.h"
#include "gui.h"
#include "guiheader.h"
#include "wavemap.h"

// GUI OBJECT IDs Waveeditor
#define OBJECTID_WAVETRACK 100

#define WAVESTATUS_ID 64

class Edit_Wave_Track:public guiObject
{
public:
	Edit_Wave_Track(){id=OBJECTID_WAVETRACK;}

	int GetTrackValue(int y);

	WaveTrack *track;
	int virtualy,virtualy2;
};

class Edit_Wave: public EventEditor_Selection
{
	friend class GUI;
	friend class Edit_WaveEditorEffects;

public:
	Edit_Wave();

	bool ZoomGFX(int zoomy,bool horiz=false);
	void Goto(int to); //v
	void SelectAll(bool on); //v
	void ShowEvents(int y=-1); //v
	void ShowEvent(Seq_SelectionEvent *,bool direct);
	void ShowMouse(OSTART time); // v
	void RefreshStartPosition(); // v

	char *GetWindowName();

	guiMenu *CreateMenu();
	void CheckPopMenu();

	void ShowMenu();
	void ShowWavesHoriz(int flag);
	void AutoScroll();

	void NewZoom() // v
	{
		ShowWavesHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_HEADER);
	}

	Edit_Wave_Track *FindTrack(WaveTrack *f)
	{
		guiObject *o=guiobjects.FirstObject();

		while(o)
		{
			if(o->id==OBJECTID_WAVETRACK)
			{
				Edit_Wave_Track *c=(Edit_Wave_Track *)o;
				if(c->track==f)return c;
			}

			o=o->NextObject();
		}

		return 0;
	}

	Edit_Wave_Track *FindWaveAtYPosition(int y);

	void DeInitWindow();

	bool CheckIfEventInsideEditor(Seq_Event *e)
	{
		guiObject *o=guiobjects.FirstObject();

		while(o)
		{
			if(o->id==OBJECTID_WAVETRACK)
			{
				Edit_Wave_Track *t=(Edit_Wave_Track *)o;
				if(t->track->CheckIfEventInside(e)==true)
					return true;
			}

			o=o->NextObject();
		}

		return false;
	}

	void CreateNewTrack(WaveTrack *);
	void DeleteTrack(WaveTrack *);
	void NewActiveTrack(WaveTrack *);
	bool EditCancel();
	void Init();
	void ShowSlider();
	void InitGadgets();
	void ReplaceTrackWithTrack(WaveTrack *old,WaveTrack *newt);
	void ReplaceTrackChannel(WaveTrack *tchl,int chl);
	void RefreshObjects(LONGLONG type,bool editcall);

	//GUI
	void KeyDown();
	void MouseButton(int flag);
	void SetMouseMode(int newmode);
	void MouseMove(bool inside);
	void MouseWheel(int delta,guiGadget *);
	void Gadget(guiGadget *);
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	void ShowWaveTracks(int y);

	void SelectMap(WaveMap *);

	bool inbound, // BOUND to ArrangeEditor
	 showeffects;

	Edit_WaveEditorEffects trackfx;

	WaveMap *wavedefinition; // Wave Track Map
	WaveTrack *firsttrack, // show firsrt track
	 *focustrack;

private:
	void MoveTrack(WaveTrack *t,int diff);

	void EditEvent();
	void CreateEvent(bool newundo);
	bool DeleteEvent();

	void ShowWaveRaster();

	//Edit_Frame frame_waves,frame_events,frame_fx;
	EF_CreateEvent editevent;

	guiGadget *activestatus;
	WaveTrack *modestarttrack;
	int default_notelength,getcountselectedevents,getcountevents,trackcount;
};

#endif