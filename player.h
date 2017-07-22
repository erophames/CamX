#ifndef CAMX_PLAYER_H
#define CAMX_PLAYER_H 1

#include "editor.h"
#include "guimenu.h"
#include "guigadgets.h"
#include "camxfile.h"

class MIDIPattern;

class Edit_Player:public Editor
{
public:
	Edit_Player()
	{
		editorid=EDITORTYPE_PLAYER;
		playerplayback=false;
		activeproject=0;
		activesong=0;
	}

	void Start(Seq_Song *);
	void Stop();
	void RefreshRealtime();
	void MouseMove(bool inside);
	void MouseButton(int flag);

	EditData *EditDataMessage(EditData *);
	
	guiMenu *CreateMenu();
	
	void ResetGadgets()
	{
		projects=songs=0;
		start=stop=0;
	}
		
	void InitGadgets();
	void Init();
	void FreeMemory();
	void Gadget(guiGadget *);
	void RedrawGfx();
	void ShowProjects();
	void ShowSongs();
	void ShowMenu();

	Seq_Project *activeproject;
	Seq_Song *activesong;

private:
	guiMenu *options;
	guiGadgetList *glist;
	guiGadget_ListBox *projects,*songs;
	guiGadget *start,*stop;
	bool playerplayback;

	void SetActiveProject(Seq_Project *);
	void SetActiveSong(Seq_Song *,bool showsongs);
};


#endif