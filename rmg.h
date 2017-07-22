#ifndef CAMX_RMG_H
#define CAMX_RMG_H 1

#include "editor.h"
#include "objectevent.h"
#include "guiheader.h"

#define RMGTYPE_MIDI 0 // MIDI Event
#define RMGTYPE_VST 1 // VST Plugin Index
#define RMGTYPE_INTERN 2 // Track Volume etc..

#define MAXGADGETS 4

#define GADGETTYPE_BUTTON 0
#define GADGETTYPE_SLIDER_VERT 1
#define GADGETTYPE_SLIDER_HORZ 2
#define GADGETTYPE_GMBUTTON 3
#define GADGETTYPE_TOGGLEBUTTON 4 // Value 1<>2

class Edit_RMG;

class RMGObject:public Object
{
public:
	RMGObject()
	{
		fromx=fromy=0;
		tox=1;
		toy=1;

		for(int i=0;i<MAXGADGETS;i++)
			gadget[i]=0;

		type=RMGTYPE_MIDI;

		status=CONTROLCHANGE;
		bytes[0]=7;

		value=0;

		value1=1; // Toggle Buttons
		value2=2;

		editor=0;

		// Default
		from=0;
		to=127;
		offset=0;
		multi=1;
	}

	void NewStatus(UBYTE nstatus);

	int type; // RMGTYPE
	int gadgettype;

	// MIDI
	UBYTE status;
	UBYTE bytes[3];

	int value,
	 value1,
	 value2,

	// Range
	 from,to,
	 offset,
	 multi;

	// RMG Raster
	int fromx,fromy,tox,toy;

	virtual void FreeRMGMemory(){}
	virtual int Draw(Edit_RMG *rmg,int id){return id;}
	virtual void RefreshGUI(int flag){}
	virtual void Clicked(Edit_RMG *rmg,guiGadget *g){Execute();}
	virtual void Execute(){}
	virtual void Output();

	void Send(Seq_Event *);

	RMGObject *PrevRMGObject(){return (RMGObject *)prev;}
	RMGObject *NextRMGObject(){return (RMGObject *)next;}
	RMGObject *NextOrPrevObject(){return (RMGObject *)NextOrPrev();}

	Edit_RMG *editor;
	guiGadget *gadget[4];
};

class RMGObjectOnDisplay:public Object
{
public:
	bool Inside(int x,int y)
	{
		if(x>=co_x && x<=co_x2 &&
			y>=co_y && y<=co_y2)
			return true;

		return false;
	}

	RMGObject *object;

	int co_x,co_y,co_x2,co_y2;

	RMGObjectOnDisplay *NextRMGObjectOnDisplay(){return (RMGObjectOnDisplay *)next;}
};

class RMGOBJ_Button:public RMGObject
{
public:
	RMGOBJ_Button()
	{
		gadgettype=GADGETTYPE_BUTTON;
	}

	int Draw(Edit_RMG *rmg,int id);
	void Execute();
	// void Clicked();
};

class RMGOBJ_ToggleButton:public RMGObject
{
public:
	RMGOBJ_ToggleButton()
	{
		gadgettype=GADGETTYPE_TOGGLEBUTTON;
		valueindex=0;
	}

	int Draw(Edit_RMG *rmg,int id);
	void Execute();
	
	int valueindex; // 0<>1
};

#define RMG_RGUI_SLIDER_BUTTON 1
#define RMG_RGUI_SLIDER_SLIDER 2

class RMGOBJ_Slider_Vert:public RMGObject
{
public:
	RMGOBJ_Slider_Vert()
	{
		gadgettype=GADGETTYPE_SLIDER_VERT;
	}

	int Draw(Edit_RMG *rmg,int id);
	void RefreshGUI(int flag);
	void Execute();
	void Clicked(Edit_RMG *rmg,guiGadget *g);
};

class RMGOBJ_Slider_Horz:public RMGObject
{
public:
	RMGOBJ_Slider_Horz()
	{
		gadgettype=GADGETTYPE_SLIDER_HORZ;
	}

	int Draw(Edit_RMG *rmg,int id);
	void RefreshGUI(int flag);
	void Execute();
	void Clicked(Edit_RMG *rmg,guiGadget *g);
};

class RMGOBJ_GMButton:public RMGObject
{
public:
	RMGOBJ_GMButton()
	{
		gadgettype=GADGETTYPE_GMBUTTON;
		status=PROGRAMCHANGE;
	}

	int Draw(Edit_RMG *rmg,int id);
	// void RefreshGUI(int flag);
	void Execute();
	void Clicked(Edit_RMG *rmg,guiGadget *g);
};

class RMGMap:public Object
{
public:
	RMGMap()
	{
		rastersize_x=24;
		rastersize_y=24;

		strcpy(name,"RMG Map");
	}

	void AddRMGObject(RMGObject *o);
	RMGObject *DeleteRMGObject(RMGObject *r);

	void DeleteAllO();

	RMGObject* FirstRMGObject() {return (RMGObject *)objects.GetRoot();}

	RMGMap *NextMap(){return (RMGMap *)next;}

	char name[STANDARDSTRINGLEN+2];

	int rastersize_x; // Pixel X
	int rastersize_y; // Pixel Y

	OList objects;
};

class mainRMGMap
{
public:
	void AddMap(RMGMap *o);
	RMGMap *DeleteMap(RMGMap *m);
	void DeleteAllMaps();
	RMGMap *CreateNewMap();
	RMGMap* FirstMap() {return (RMGMap *)maps.GetRoot();}

	void InitMaps(); // Load Maps

	OList maps;
};

class Edit_RMG_Frame
{
public:
	//Edit_Frame edit;
	//Edit_Frame raster; // Marker Right Position
};

class Edit_RMGFX
{
public:	
	Edit_RMGFX()
	{	
		activermgobject=0;
		glist=0;
		ResetGadgets();
	};

	void FreeMemory()
	{
		ResetGadgets();
	}

	guiGadget *Gadget(guiGadget *);
	bool Init();

	void ResetGadgets()
	{
		type=0;
		eventtype=0;
		byte1=0;
		gadgettype=0;

		value=0;
	}

	void ShowObject();

	void Close();
	
	EditData *EditDataMessage(EditData *data);

	Edit_RMG *rmgeditor;

	RMGObject *activermgobject;
	guiGadgetList *glist;
private:
	guiGadget *type;
	guiGadget *eventtype;
	guiGadget *byte1;
	guiGadget *gadgettype;

	guiGadget *value;
};

class Edit_RMG: public EventEditor
{
public:
	Edit_RMG();

	void MouseButton(int flag);

	void RefreshObject(RMGObject *o);
	guiMenu *CreateMenu();
	void Gadget(guiGadget *gadget);
	bool CheckRMGObjectInside(RMGObject *r);
	void AddObjectToDisplay(RMGObject *r,int x,int y,int x2,int y2);

	void ShowRaster();
	
	EditData *EditDataMessage(EditData *data);

	void Init();
	void ShowMap();
	void FreeMemory();

	int ConvertXToIndex(int cx);
	int ConvertYToIndex(int cy);

	int GetX(int indexx);
	int GetY(int indexy);

	RMGMap *map;

	Edit_RMG_Frame frame;

	int from_x,from_y;
	int to_x,to_y;

	guiGadgetList *rastergadgets;

	RMGObjectOnDisplay *FirstObjectOnDisplay(){return (RMGObjectOnDisplay *)objectsondisplay.GetRoot();}
	OList objectsondisplay; //RMGObjectOnDisplay

	Edit_RMGFX fx;
	bool showeffects;

	guiGadget_Slider *xslider;
	guiGadget_Slider *yslider;
};
#endif