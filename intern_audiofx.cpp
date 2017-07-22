#include "intern_audiofx.h"
#include "edit_audiointern.h"
#include "audioauto_intern.h"
#include "gui.h"

#ifdef OLDIE
void audioobject_Intern::InitEffectGUI(PluginWindow *w)
{
	int gid=ID_INTERNSTART;

	// Text, Value, Slider
	SliderCo horz;

	for(int i=0;i<numberofparameter;i++)
	{
		w->glist.form->SetGX(0);

		w->glist.AddButton(-1,-1,60,-1,GetParmName(i),gid++);
		w->glist.AddLX();
		w->glist.AddButton(-1,-1,60,-1,GetParmValueString(i),gid++);
		w->glist.AddLX();
		w->glist.AddButton(-1,-1,60,-1,GetParmTypeValueString(i),gid++);

		/*
		w->glist.AddLX();
		horz.x=w->glist.GetLX();
		horz.y=w->glist.GetLY();
		horz.x2=w->w;
		horz.y2=horz.y+maingui->GetFontSizeY();
		horz.horz=true;

		w->glist.AddSlider(&horz,gid++,MODE_RIGHT,this);
*/

		w->glist.AddLY();
	
	}
}
#endif

guiWindow *audioobject_Intern::CheckIfWindowIsEditor(guiWindow *win)
{
	if(win->GetEditorID()==EDITORTYPE_PLUGIN_INTERN)
	{
		Edit_Plugin_Intern *eint=(Edit_Plugin_Intern *)win;
		
		if(eint->effect==this)
			return win;
	}
	
	return 0;
}

int audioobject_Intern::GetEditorSizeY()
{
	return numberofparameter*maingui->GetFontSizeY();
}