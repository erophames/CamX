#ifndef CAMX_DRUMEDITORLIST_H
#define CAMX_DRUMEDITORLIST_H 1

#include "editor.h"

class Edit_Drum;
class Edit_DrumList;

class Edit_DrumList_Track:public guiObject
{
	friend class Edit_DrumList;

public:
	Edit_DrumList_Track();

	void DeInit()
	{
		if(namestring)delete namestring;
		namestring=0;
	}

	void ShowTrack(bool refreshbgcolour);
	void ShowName();
	void ShowColour();
	
	Edit_DrumList *editor;
	Drumtrack *track;
	int startnamex;
	char *namestring;
	bool trackselected,focus;
};

class Edit_DrumList:public EventEditor
{
public:
	Edit_DrumList(Edit_Drum *);

	void InitGadgets();
	void Init();
	void DeInitWindow();
	void RefreshObjects(LONGLONG type,bool editcall);
	void RefreshRealtime();

	void Gadget(guiGadget *);
	void MouseClickInTracks(bool leftmouse);

	void BuildTrackList();
	void ShowList();
	void ShowVSlider();
	void FreeEditorMemory();

	OListCoosY trackobjects;
	guiGadget_Tab *list;
	Edit_Drum *editor;
};

#endif