#ifndef CAMX_TOOLBOXEDITOR_H
#define CAMX_TOOLBOXEDITOR_H 1

#include "editor.h"

class Edit_Toolbox:public Editor
{
public:
	Edit_Toolbox();
	void Gadget(guiGadget *);
	void Init();
	void RefreshRealtime_Slow();
	guiScreen *fromscreen;
	guiGadget *undo,*redo;
};
#endif