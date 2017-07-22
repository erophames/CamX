#include "MIDIprocessor.h"
#include "editortypes.h"
#include "gui.h"
#include "camxfile.h"

void MIDIPlugin::OpenGUI(guiWindowSetting *settings){
	if(!win)
		win=maingui->OpenEditor(EDITORTYPE_PROCESSORMODULE,0,0,0,settings,this,0);
	else
		win->WindowToFront(true);
}

void MIDIPlugin::CloseGUI()
{
	if(win)
	{
		guiWindow *owin=win;
		win=0;
		maingui->CloseWindow(owin);
	}
}

void MIDIPlugin::LoadStandards(camxFile *file)
{
	file->ReadChunk(&bypass);
}

void MIDIPlugin::SaveStandards(camxFile *file)
{
	file->Save_Chunk(bypass);
}
