#include "rmg.h"
#include "gui.h"

// main RMG
void RMGMap::AddRMGObject(RMGObject *o)
{
	objects.AddEndO(o);
}

RMGObject *RMGMap::DeleteRMGObject(RMGObject *r)
{
	r->FreeRMGMemory();

	return (RMGObject *)objects.RemoveO(r);
}

void RMGMap::DeleteAllO()
{
	RMGObject *o=FirstRMGObject();

	while(o)
	{
		o=DeleteRMGObject(o);
	}
}

void mainRMGMap::AddMap(RMGMap *o)
{
	if(o)
	{
		maps.AddEndO(o);
	}
}

RMGMap *mainRMGMap::DeleteMap(RMGMap *m)
{
	m->DeleteAllO();

	return (RMGMap *)maps.RemoveO(m);
}

void mainRMGMap::DeleteAllMaps()
{
	RMGMap *m=FirstMap();

	while(m)
	{
		m=DeleteMap(m);
	}
}

RMGMap *mainRMGMap::CreateNewMap()
{
	RMGMap *rmg=new RMGMap;

	if(rmg)
		AddMap(rmg);

	return rmg;
}

void mainRMGMap::InitMaps()
{
	// Default GM
	RMGMap *gm=CreateNewMap();

	if(gm)
	{
		RMGOBJ_Button *rmgbutton=new RMGOBJ_Button;

		gm->AddRMGObject(rmgbutton);

		RMGOBJ_GMButton *gmbutton=new RMGOBJ_GMButton;

		gmbutton->fromx=0;
		gmbutton->fromy=4;
		gmbutton->toy=8;

		gm->AddRMGObject(gmbutton);

		RMGOBJ_ToggleButton *tbutton=new RMGOBJ_ToggleButton;

		tbutton->fromx=0;
		tbutton->fromy=9;
		tbutton->toy=12;

		gm->AddRMGObject(tbutton);

		RMGOBJ_Slider_Vert *rmgslider=new RMGOBJ_Slider_Vert;

		rmgslider->fromx=3;
		rmgslider->tox=6;
		rmgslider->fromy=0;
		rmgslider->toy=10;

		gm->AddRMGObject(rmgslider);

		rmgslider=new RMGOBJ_Slider_Vert;

		rmgslider->fromx=7;
		rmgslider->tox=10;
		rmgslider->fromy=0;
		rmgslider->toy=10;

		gm->AddRMGObject(rmgslider);
	}
}