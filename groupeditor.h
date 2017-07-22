#ifndef SYS_GROUPEDITOR
#define SYS_GROUPEDITOR

#include "editor.h"

class Seq_Group;

class Edit_Group_Tracks:public Object
{
public:

	Edit_Group_Tracks *NextGroupTrack(){return (Edit_Group_Tracks *)next;}
	Seq_Track *track;
	char *name;
	int index;
};

class Edit_Group:public Editor
{
public:
	Edit_Group()
	{
		editorid=EDITORTYPE_GROUP;
		activegroup=0;
		editorname="Songs Groups";
	}

	EditData *EditDataMessage(EditData *data);

	guiMenu *CreateMenu();

	void ResetGadgets()
	{
		colour_x=-1;

		group_colour=
			group_solo=
			group_mute=
			group_rec=
			groupname=0;

		grouptracks=groupgadget=0;
	}

	void DeleteActiveGroup();

	void RefreshObjects(LONGLONG par,bool editcall);
	void RefreshRealtime();

	void ShowActiveGroup();

	void InitGadgets();
	void Init();

	void DeleteAllTrackObjects();
	void FreeMemory();
	void Gadget(guiGadget *);

	void RedrawGfx();

	void ShowGroups();
	void ShowActiveGroupStatus();
	void ShowActiveGroupColour();

	void AddGroupFromFile();
	void SaveAllGroups(bool all);

	void AddSelectedTracks();
	void AddActiveTrack();
	void RemoveSelectedTracks();
	void RemoveActiveTrack();

	void RefreshGroupGUI();

	Seq_Group *activegroup;

	Edit_Group_Tracks *FirstGroupTrack(){return (Edit_Group_Tracks *)tracks.GetRoot();}
	OList tracks; // Edit_Group_Tracks

private:
	guiGadgetList *glist;

	guiGadget *group_colour;
	guiGadget *group_rec;
	guiGadget *group_solo;
	guiGadget *group_mute;

	guiGadget_ListBox *groupgadget;
	guiGadget *groupname;
	guiGadget_ListBox *grouptracks;

//	Edit_Frame frame_groups;

	bool status_group_mute;
	bool status_group_solo;
	bool status_group_rec;

	int colour_x,colour_x2,colour_y,colour_y2;
};

#endif