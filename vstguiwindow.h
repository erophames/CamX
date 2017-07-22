#ifndef CAMX_VSTEDITOR_H
#define CAMX_VSTEDITOR_H 1

#include "pluginwindow.h"

class Seq_Song;
class VSTPlugin;

class Edit_Plugin_VST:public PluginWindow
{
	friend GUI;
	
public:
	Edit_Plugin_VST(VSTPlugin *,InsertAudioEffect *);

	void InitPluginParameterEditor();
	void RefreshRealtime();
	void RefreshRealtime_Slow();

	void RefreshWindowSize();

    bool CheckIfObjectInside(Object *); //v
	void Init();
	void RemoveChildWindows();

	guiMenu *CreateMenu();
	
	void Gadget(guiGadget *);
	
	VSTPlugin *vstplugin;

	#ifdef WIN32
	HWND vstguiwindow; // Window in Window
#endif

private:
	void LoadChunk();
	void SaveChunk();
};

#endif