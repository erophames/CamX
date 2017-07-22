#include "edit_audiointern.h"
#include "audioeffects.h"
#include "audiohardware.h"
#include "camxgadgets.h"
#include "songmain.h"
#include "editdata.h"
#include "audiochannel.h"
#include "object_track.h"

void Edit_Plugin_Intern::RefreshRealtime_Slow()
{
	PluginRefreshRealtime_Slow();
}

void Edit_Plugin_Intern::RefreshRealtime()
{
	PluginRefreshRealtime();

	/*
	if(status_involume!=effect->involume)
	ShowVolumeIn();

	if(status_outvolume!=effect->outvolume)
	ShowVolumeOut();
	*/
}

EditData *Edit_Plugin_Intern::EditDataMessage(EditData *data)
{
	switch(data->id)
	{
	case EDIT_INVOLUME:
		{
			effect->involume=data->volume;
		}
		break;

	case EDIT_OUTVOLUME:
		{
			effect->outvolume=data->volume;
		}
		break;

	case EDIT_FXINTERN:
		{
			effect->Edit_Data(data);
		}
		break;

	}

	return 0;
}

void Edit_Plugin_Intern::Gadget(guiGadget *g)
{
	g=PluginGadget(g);

	if(!g)
		return;


	/*
	if(g==involume)
	{
	if(EditData *edit=new EditData)
	{
	edit->song=WindowSong();
	edit->win=this;
	edit->x=g->x2;
	edit->y=g->y;

	edit->name="Input";

	edit->id=EDIT_INVOLUME;

	edit->type=EditData::EDITDATA_TYPE_VOLUMEDB;
	edit->volume=effect->involume;
	edit->audiosend=0;

	maingui->EditDataValue(edit);
	}
	return;
	}

	if(g==outvolume)
	{
	if(EditData *edit=new EditData)
	{
	edit->song=WindowSong();
	edit->win=this;
	edit->x=g->x2;
	edit->y=g->y;

	edit->name="Output";

	edit->id=EDIT_OUTVOLUME;

	edit->type=EditData::EDITDATA_TYPE_VOLUMEDB;
	edit->volume=effect->outvolume;
	edit->audiosend=0;

	maingui->EditDataValue(edit);
	}

	return;
	}
	*/

	effect->Gadget(g);
}

guiMenu *Edit_Plugin_Intern::CreateMenu()
{
	if(menu)menu->RemoveMenu();
	menu=new guiMenu;

	if(menu)
	{
		effect->inserteffect->audioeffect->AddIOMenu(menu);
	}

	return menu;
}

void Edit_Plugin_Intern::Init()
{
	InitPluginEditor();

	glist.SelectForm(0,1);

	if(effect->GetOwnEditor()==true)
	{
	//	effect->InitEffectGUI(this);
	}
	else
		InitPlugInList();
}

Edit_Plugin_Intern::Edit_Plugin_Intern(audioobject_Intern *fx,InsertAudioEffect *ieffect)
{
	editorid=EDITORTYPE_PLUGIN_INTERN;

	InitForms(FORM_PLUGIN);
	resizeable=true;
	effect=fx;
	insertaudioeffect=ieffect;
}


bool Edit_Plugin_Intern::CheckIfObjectInside(Object *o)
{
	if(o==insertaudioeffect ||
		o==effect ||
		o==(Object *)insertaudioeffect->effectlist->channel ||
		o==(Object *)insertaudioeffect->effectlist->track
		)
	{
		return true;
	}

	return false;
}