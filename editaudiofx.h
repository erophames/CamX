#ifndef CAMX_EDITFX
#define CAMX_EDITFX 1

#include "semapores.h"

class menu_editeff:public guiMenu
{
public:
	menu_editeff(Seq_Song *s,InsertAudioEffect *e)
	{
		song=s;
		effect=e;
	}

	void MenuFunction()
	{
		guiWindow *w=maingui->FirstWindow();
		guiWindow *open=0;

		while(w && open==0)
		{
			open=effect->audioeffect->CheckIfWindowIsEditor(w);
			w=w->NextWindow();
		}

		if(open)
		{
			open->WindowToFront(true);
		}	
		else
		{
			/*
			if(gadget)
			{
				guiWindowSetting setting;
				setting.startposition_x=gadget->x2+gadget->guilist->win->win_screenposx;
				setting.startposition_y=gadget->y+gadget->guilist->win->win_screenposy;
				effect->audioeffect->OpenGUI(song,effect,&setting);
			}
			else
			*/
				effect->audioeffect->OpenGUI(song,effect);
		}
	} //

	Seq_Song *song;
	InsertAudioEffect *effect;
	guiGadget *gadget;
};

class menu_moveup:public guiMenu
{
public:
	menu_moveup(Seq_Song *s,InsertAudioEffect *fx)
	{
		song=s;
		effect=fx;
	}

	void MenuFunction()
	{	
		mainthreadcontrol->LockActiveSong();
		effect->effectlist->effects.MoveOIndex(effect,-1);
		mainthreadcontrol->UnlockActiveSong();
		maingui->RefreshAudioMixer(song,effect->effectlist);
	} //

	Seq_Song *song;
	InsertAudioEffect *effect;
};


class menu_movedown:public guiMenu
{
public:
	menu_movedown(Seq_Song *s,InsertAudioEffect *fx)
	{
		song=s;
		effect=fx;
	}

	void MenuFunction()
	{	
		mainthreadcontrol->LockActiveSong();
		effect->effectlist->effects.MoveOIndex(effect,1);
		mainthreadcontrol->UnlockActiveSong();

		maingui->RefreshAudioMixer(song,effect->effectlist);
	} //

	Seq_Song *song;
	InsertAudioEffect *effect;
};

/*
class menu_moveinstrup:public guiMenu
{
public:
	menu_moveinstrup(Seq_Song *s,InsertAudioEffect *fx)
	{
		song=s;
		effect=fx;
	}

	void MenuFunction()
	{	
		mainthreadcontrol->LockActiveSong(MEDIATYPE_ALL);
		effect->effectlist->instruments.MoveOIndex(effect,-1);
		mainthreadcontrol->UnlockActiveSong(MEDIATYPE_ALL);

		maingui->RefreshAudioMixer(song,effect->effectlist);
	} //

	Seq_Song *song;
	InsertAudioEffect *effect;
};

class menu_moveinstrdown:public guiMenu
{
public:
	menu_moveinstrdown(Seq_Song *s,InsertAudioEffect *fx)
	{
		song=s;
		effect=fx;
	}

	void MenuFunction()
	{	
		mainthreadcontrol->LockActiveSong(MEDIATYPE_ALL);
		effect->effectlist->instruments.MoveOIndex(effect,1);
		mainthreadcontrol->UnlockActiveSong(MEDIATYPE_ALL);

		maingui->RefreshAudioMixer(song,effect->effectlist);
	} //

	Seq_Song *song;
	InsertAudioEffect *effect;
};
*/

#endif