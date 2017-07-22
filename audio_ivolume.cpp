#include "audioobject_insertvolume.h"
#include "audiohardwarebuffer.h"
#include "gui.h"
#include "songmain.h"
#include "audioeffects.h"
#define PI 3.1415926535
#include "edit_audiointern.h"

guiWindow *audioobject_Intern_IVolume::OpenGUI(Seq_Song *s,InsertAudioEffect *ieffect,guiWindowSetting *settings) //v
{
	guiWindow *win;
	
	win=maingui->OpenEditor(EDITORTYPE_PLUGIN_INTERN,s,0,0,settings,this,ieffect);
	
	return win;
}

bool audioobject_Intern_IVolume::CloseGUI()
{
	return false;
}

void audioobject_Intern_IVolume::ShowVolume()
{
	if(button)
	{
		if(char *h=mainaudio->GenerateDBString(volume))
		{
			char *h2=mainvar->GenerateString("Vol:",h);
			if(h2)
			{
				button->ChangeButtonText(h2);
				delete h2;
			}
			delete h;
		}
	}
}

#ifdef OLDIE
void audioobject_Intern_IVolume::InitEffectGUI(guiWindow *ed,int x,int y,int x2,int y2) // v
{
	guiGadgetList *gl=ed->gadgetlists.AddGadgetList(ed);
	if(gl)
	{
		button=gl->AddButton(x,y,x2,y2,0,0,0,0);
		ShowVolume();
	}

	editor=ed;
}

void audioobject_Intern_IVolume::Gadget(guiGadget *g)
{
	if(EditData *edit=new EditData)
	{
			edit->win=editor;
			edit->x=g->x2;
			edit->y=g->y;

			edit->name="Volume";

			edit->id=EDIT_FXINTERN;

			edit->type=EditData::EDITDATA_TYPE_VOLUMEDB;
			edit->volume=volume;
			edit->audiosend=0;

			maingui->EditDataValue(edit);
		}
}
#endif
