#ifndef CAMX_AUDIOINTERNEDITOR_H
#define CAMX_AUDIOINTERNEDITOR_H 1

#include "intern_audiofx.h"
#include "pluginwindow.h"

class Edit_Plugin_Intern:public PluginWindow
{
	friend GUI;

public:
	Edit_Plugin_Intern(audioobject_Intern *fx,InsertAudioEffect *ieffect);
	bool CheckIfObjectInside(Object *); //v
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	
	EditData *EditDataMessage(EditData *);
	void Init();

	guiMenu *CreateMenu();

	void FreeMemory()
	{
		effect->CloseGUI();	
		guigadgets.DeleteAllO();
		delete this;
	}

	void Gadget(guiGadget *);

	audioobject_Intern *effect;

private:
	OList guigadgets; //Edit_Plugin_VST_GUIObjects
};

#endif