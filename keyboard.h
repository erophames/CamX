#ifndef CAMX_KEYBOARD_H
#define CAMX_KEYBOARD_H 1

#include "editor.h"
#include "guimenu.h"
#include "guigadgets.h"
#include "MIDIindevice.h"
#include "object_song.h"

#define KEYBOARD_FRAMEID_KEYS 1
#define MAXKEYBOARDKEYS 128

#define FLAG_MOUSE_KEY 1
#define FLAG_KEYBOARD_KEY 2

class Keyboardkey
{
public:
	Keyboardkey()
	{
		counter=0;
		used=hold=false;
		flag=0;
	}

	int counter,flag,key;
	char playedkey;
	bool used,hold;
};

class Keyboardmap:public Object
{
public:
	Keyboardmap()
	{
		ResetDefault();	
	}

	void ResetDefault();
	void Load(camxFile *);
	void Save(camxFile *);

	Keyboardkey keys[MAXKEYBOARDKEYS];
};

class Edit_Keyboard:public EventEditor
{
public:
	Edit_Keyboard();

	bool CheckKeyDown();
	bool CheckKeyUp();
	void LoadMap();
	void SaveMap();
	void LoadAutoMap(bool nomessage);
	void SaveAutoMap();
	void KillFocus();

	void MouseMove(bool inside);
	void MouseWheel(int delta,guiGadget *);
	guiMenu *CreateMenu(); // v
	void KeyDown();
	void KeyUp();
	void DeActivated();
	void RefreshRealtime();
	void Init();
	void ScrollHoriz();
	void SendOpenKeys(int flag);
	void DeInitWindow();
	void Gadget(guiGadget *);
	void ShowKeys();	
	void ShowFilter();
	int FindKeyUnderMouse();
	void PlayMouseKey(bool mousemove);

	Keyboardmap map;
	guiMenu *filtermenu;
	guiGadget_CW *keys;
	int channel,transpose,velocity;
	bool hold;

private:
	void ClearKeyUnderMouse();
	bool CheckNoteOff();
	int LongToKeylist(int);
	void ShowEditMode();
	void ShowHoldMode();
	void PlayKey(int key,int flag);
	void SendNoteOff(int key,int flag,bool force=false);

	OpenNoteFilter filter;
	
	guiGadget *recgadget,*holdgadget;
	int lastkey,startkey,numberofkeys,clearkey,keyundermouse,zoomc,keyposx[MAXKEYBOARDKEYS],keyposx2[MAXKEYBOARDKEYS],keyposy[MAXKEYBOARDKEYS],keyposy2[MAXKEYBOARDKEYS];
	int notetype;
	bool showkeyundermouse,edit,mousekeyactive[MAXKEYBOARDKEYS],keyactive[128],keyblack[MAXKEYBOARDKEYS];
};
#endif