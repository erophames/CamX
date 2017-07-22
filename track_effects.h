#ifndef CAMX_TRACK_EFFECTS_H
#define CAMX_TRACK_EFFECTS_H 1

class tmenu_deleteplugin:public guiMenu
{
public:
	tmenu_deleteplugin(InsertAudioEffect *e){effect=e;}
	void MenuFunction();
	InsertAudioEffect *effect;
};

class tmenu_effect:public guiMenu
{
public:
	tmenu_effect(AudioEffects *efx,InsertAudioEffect *to,AudioObject *fx,guiWindow *w)
	{
		effects=efx;
		fxplugin=fx;
		oldeffect=to;
		win=w;
	}

	void MenuFunction();

	AudioEffects *effects;
	AudioObject *fxplugin;
	InsertAudioEffect *oldeffect;
	guiWindow *win;
};

class tmenu_moveup:public guiMenu
{
public:
	tmenu_moveup(InsertAudioEffect *fx){effect=fx;}

	void MenuFunction()
	{	
		mainthreadcontrol->LockActiveSong();
		effect->effectlist->effects.MoveOIndex(effect,-1);
		mainthreadcontrol->UnlockActiveSong();

		maingui->RefreshEffects(effect->effectlist);
	} //

	InsertAudioEffect *effect;
};


class tmenu_movedown:public guiMenu
{
public:
	tmenu_movedown(InsertAudioEffect *fx){effect=fx;}

	void MenuFunction()
	{	
		mainthreadcontrol->LockActiveSong();
		effect->effectlist->effects.MoveOIndex(effect,1);
		mainthreadcontrol->UnlockActiveSong();

		maingui->RefreshEffects(effect->effectlist);
	} //

	InsertAudioEffect *effect;
};

/*
class tmenu_moveinstrup:public guiMenu
{
public:
	tmenu_moveinstrup(InsertAudioEffect *fx){effect=fx;}

	void MenuFunction()
	{	
		mainthreadcontrol->LockActiveSong(MEDIATYPE_ALL);
		effect->effectlist->instruments.MoveOIndex(effect,-1);
		mainthreadcontrol->UnlockActiveSong(MEDIATYPE_ALL);

		maingui->RefreshEffects(effect->effectlist);
	} //

	InsertAudioEffect *effect;
};

class tmenu_moveinstrdown:public guiMenu
{
public:
	tmenu_moveinstrdown(InsertAudioEffect *fx){effect=fx;}

	void MenuFunction()
	{	
		mainthreadcontrol->LockActiveSong(MEDIATYPE_ALL);
		effect->effectlist->instruments.MoveOIndex(effect,1);
		mainthreadcontrol->UnlockActiveSong(MEDIATYPE_ALL);

		maingui->RefreshEffects(effect->effectlist);
	} //

	InsertAudioEffect *effect;
};
*/
#endif