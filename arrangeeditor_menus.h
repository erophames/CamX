#include "languagefiles.h"

// Menus
class menu_solooff:public guiMenu
{
public:
	menu_solooff(Seq_Song *s){song=s;}

	void MenuFunction()
	{		
		Seq_Track *t=song->FirstTrack();

		while(t){
			song->SoloTrack(t,false);
			t=t->NextTrack();
		}
	}

	Seq_Song *song;
};

class menu_muteoff:public guiMenu
{
public:
	menu_muteoff(Seq_Song *s,bool sel)
	{
		song=s;
		selected=sel;
	}

	void MenuFunction()
	{		
		Seq_Track *t=song->FirstTrack();

		while(t){

			if(selected==false || (t->flag&OFLAG_SELECTED) )
				song->MuteTrack(t,false);

			t=t->NextTrack();
		}
	} //

	Seq_Song *song;
	bool selected;
};

class menu_solomuteoff:public guiMenu
{
public:
	menu_solomuteoff(Seq_Song *s){song=s;}

	void MenuFunction()
	{	
		Seq_Track *t=song->FirstTrack();

		while(t){
			song->SoloTrack(t,false);
			song->MuteTrack(t,false);

			t=t->NextTrack();
		}
	}

	Seq_Song *song;
};

