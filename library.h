#ifndef CAMX_LIBRARY_H
#define CAMX_LIBRARY_H 1

#include "editor.h"

#include "guimenu.h"
#include "guigadgets.h"
#include "audiofile.h"
#include "audiohdfile.h"

class Edit_Library: public Editor
{
public:
	Edit_Library();

	void ResetGadgets()
	{
		tree=0;
	}

	guiMenu *CreateMenu();
	void ShowObjects();
	
	void InitGadgets();
	void Init();
	void MouseMove(bool inside);
	void KeyDown();
	void KeyUp(char nVirtKey);
	void Gadget(guiGadget *);
	void FreeMemory();


	//Edit_Frame frame_objects;

private:

	guiGadget *tree;
};
#endif