#ifndef CAMX_GUIOBJECTS_H
#define CAMX_GUIOBJECTS_H 1

#define GUIOBJ_LIST 333333

#include "object.h"

class guiGadget;
class guiGadget_CW;
class guiGadget_Tab;
class guiBitmap;

class guiObject:public Object
{
	friend class guiWindow;
	friend class guiOList;
	
public:
	guiObject()
	{
#ifdef _DEBUG
		n[0]='G';
		n[1]='O';
		n[2]='B';
		n[3]='J';
#endif	
		
		ondisplay=false;
		deleteable=true;
		object=0;
		parent=0;
		gadget=0;
		gadget_DB=0;
		idprio=0;
	}
	
	virtual void DeInit(){}

	bool CheckObjectinRange(int x,int y,int x2,int y2);

	guiObject *NextObject(){return (guiObject *)next;}
	guiObject *PrevObject(){return (guiObject *)prev;}
	
	guiObject *RemoveGuiObject(int rid);
	void Blt();

#ifdef _DEBUG
	char n[4];
#endif
	
	Object *object;
	guiGadget_CW *parent,*gadget_DB;
	guiGadget *gadget;

	guiBitmap *bitmap;
	int x,y,x2,y2,idprio;

	bool deleteable, // default true, not part of frame
	 ondisplay;
};

class guiObject_Pref:public Object
{
public:
	guiObject_Pref(guiObject *o){gobject=o;}
	guiObject_Pref *NextGUIObjectPref(){return (guiObject_Pref *)next;}
	guiObject_Pref *PrevGUIObjectPref(){return (guiObject_Pref *)prev;}
	
	guiObject *gobject;
};

class guiObjectWithGadgets:public guiObject
{
public:
	void RemoveGadgets();
	OList gadgets;
};

class guiMouseObject:public guiObject
{
public:
	int m_id,from, to,value_now;
};

class guiOList
{
public:
	guiObject *FirstObject(){return (guiObject *)objects.GetRoot();}
	guiObject *LastObject(){return (guiObject *)objects.Getc_end();}

	guiMouseObject *FirstMouseObject(){return (guiMouseObject *)mouseobjects.GetRoot();}
	guiMouseObject *LastMouseObject(){return (guiMouseObject *)mouseobjects.Getc_end();}

	void RemoveOsFromTo(int from,int to);
	void RemoveOs(int rid);

	guiObject *CheckObjectClicked(int x,int y);
	void AddGUIObject(guiObject *,guiGadget_CW *);
	void AddGUIObject(int x,int y,int x2,int y2,guiGadget_CW *,guiObject *);
	void AddGUIObject(guiGadget *,guiGadget_CW *,guiObject *);
	void AddTABGUIObject(int x,int y,int x2,int y2,guiGadget_Tab *,guiObject *);

	OList objects,mouseobjects;
};
#endif