#ifndef CAMX_GLOBALMENUS_H
#define CAMX_GLOBALMENUS_H 1

#include "guimenu.h"

class Seq_Song;
class EventEditor;
class Seq_Track;

class globmenu_Event:public guiMenu
{
public:
	globmenu_Event(EventEditor *ed,OSTART s){editor=ed;startposition=s;}
	void MenuFunction();
	EventEditor *editor;
	OSTART startposition;
};

class globmenu_Piano:public guiMenu
{
public:
	globmenu_Piano(EventEditor *ed,OSTART s){editor=ed;startposition=s;}
	void MenuFunction();
	EventEditor *editor;
	OSTART startposition;
};

class globmenu_Drum:public guiMenu
{
public:
	globmenu_Drum(EventEditor *ed,OSTART s){editor=ed;startposition=s;}
	void MenuFunction();
	EventEditor *editor;
	OSTART startposition;
};

class globmenu_Score:public guiMenu
{
public:
	globmenu_Score(EventEditor *ed){editor=ed;}
	void MenuFunction();
	EventEditor *editor;
};

class globmenu_Wave:public guiMenu
{
public:
	globmenu_Wave(EventEditor *ed){editor=ed;}
	void MenuFunction();
	EventEditor *editor;
};

class globmenu_Arrange:public guiMenu
{
public:
	globmenu_Arrange(Seq_Song *s){song=s;}
	void MenuFunction();
	Seq_Song *song;
};

class globmenu_Sample:public guiMenu
{
public:
	globmenu_Sample(EventEditor *ed,OSTART s){editor=ed;startposition=s;}
	void MenuFunction();
	EventEditor *editor;
	OSTART startposition;
};

class globmenu_Toolbox:public guiMenu
{
public:
	globmenu_Toolbox(guiScreen *s){screen=s;}
	void MenuFunction();
	guiScreen *screen;
};

class globmenu_cpu:public guiMenu
{
public:
	void MenuFunction();
};

class globmenu_TMap:public guiMenu
{
public:
	globmenu_TMap(guiScreen *s){screen=s;}

	void MenuFunction();
	guiScreen *screen;
};

class globmenu_TEd:public guiMenu
{
public:
	globmenu_TEd(Seq_Song *s){song=s;}

	void MenuFunction();
	Seq_Song *song;
};
class globmenu_Marker:public guiMenu
{
public:
	globmenu_Marker(Seq_Song *s){song=s;}
	void MenuFunction();
	Seq_Song *song;
};

class globmenu_AudioMixer:public guiMenu
{
public:
	globmenu_AudioMixer(Seq_Song *s,Seq_Track *st,guiScreen *scr)
	{
		song=s;
		starttrack=st;
		screen=scr;
	}

	void MenuFunction();
	guiScreen *screen;
	Seq_Song *song;
	Seq_Track *starttrack;
};

class globmenu_Editor:public guiMenu
{
public:
	globmenu_Editor(guiScreen *scr)
	{
		screen=scr;

	}

	void MenuFunction();
	guiScreen *screen;
};

class globmenu_AManager:public guiMenu
{
	void MenuFunction();
};

class globmenu_Keyboard:public guiMenu
{
	void MenuFunction();
};

class globmenu_Bigtime:public guiMenu
{
	void MenuFunction();
};

class globmenu_cnewtrack:public guiMenu
{
public:
	globmenu_cnewtrack(Seq_Song *s,Seq_Track *t){song=s;track=t;}

	void MenuFunction();
	char *GetMenuName();

	Seq_Song *song;
	Seq_Track *track;
};

class globmenu_AMaster:public guiMenu
{
public:
	globmenu_AMaster()
	{
		freeze=false;
	}
	globmenu_AMaster(bool f)
	{
		freeze=f;
	}

	void MenuFunction();
	bool freeze;
};

class globmenu_RMG:public guiMenu
{
	void MenuFunction();
};

#endif
