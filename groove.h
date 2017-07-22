#ifndef CAMX_GROOVE_H
#define CAMX_GROOVE_H 1

#include "editor.h"

#include "guimenu.h"
#include "guigadgets.h"
#include "camxfile.h"

class MIDIPattern;

class GrooveElement:public OStart
{	
	friend class Groove;
public:
	GrooveElement()
	{
		quantdiff=0;
		initcounter=0;
		initdiff=0;
	}

	void Load(camxFile *);
	void Save(camxFile *);

	OSTART GetGrooveStart(){return ostart;}
	GrooveElement *NextGrooveElement(){return (GrooveElement *)next;}
	Groove *groove;
	OSTART quantdiff,initdiff;
	int initcounter;
};

class Groove:public Object
{
	friend class mainMIDIBase;
	friend class GrooveElement;
	friend class Edit_Groove;
public:

	Groove();
	Groove(OSTART grooveraster,int qid);
	void ChangeInSongs();

	void InitGrooveName();
	void SetName(char *);

	void Load(camxFile *);
	void Save(camxFile *);

	OSTART Quantize(OSTART pos);

	GrooveElement * FirstGrooveElement() { return (GrooveElement *)grooves.GetRoot(); }
	GrooveElement * LastGrooveElement() { return (GrooveElement *)grooves.Getc_end(); }
	GrooveElement *GetGrooveElementAtIndex(int ix){return (GrooveElement *)grooves.GetO(ix);}

	GrooveElement *RemoveGrooveElement(GrooveElement *);
	Groove *NextGroove(){return (Groove *)next;}

	void InitGroove(OSTART tickraster);
	void ConvertMIDIPatternToGroove(MIDIPattern *);

	char *GetGrooveInfo(){return groovename;}
	char *GetGrooveName(){return fullgroovename;}

	char *groovename,*fullgroovename;

	OSTART raster, // number of raster pointes
		tickraster,
		grooveend;

	int qrasterid;
	OListStart grooves; 
};

class GroovePixel:public Object
{
public:
	int x,y,step;
	GroovePixel *NextGroovePixel(){return (GroovePixel *)next;}
};

class GrooveObject:public Object
{
public:
	GrooveObject *NextGrooveObject(){return (GrooveObject *)next;}
	GrooveElement *element;
	int x1,x2,y1,y2;
};

class Edit_Groove:public Editor
{
public:
	Edit_Groove();

	void Load();
	void Save();
	void LoadList();
	void SaveList();

	void RefreshRealtime();
	void MouseMove(bool inside);
	void MouseButton(int flag);

	EditData *EditDataMessage(EditData *);

	guiMenu *CreateMenu();

	void ResetGadgets()
	{
		groovegadget=groovediffgadget=0;
		groovename=grooveraster=grooveinfo=0;
	}

	void UserNewGroove(int steps);
	void DeleteGroove();
	void ResetGroove();

	void ListActiveGroove();

	void ShowActiveGroove();
	void ShowGrooveInfo();
	void InitGadgets();
	void Init();
	void FreeMemory();
	void Gadget(guiGadget *);
	void RedrawGfx();
	void ShowGrooves();
	void RefreshGrooves();

	Groove *activegroove;

private:
	void ClearMoveO();

	GroovePixel *FirstGroovePixel(){return (GroovePixel *)groovepixel.GetRoot();}
	void ChangeGroove(GrooveElement *,OSTART qdiff);

	GrooveObject *FirstGrooveObject(){return (GrooveObject *)grooves.GetRoot();}

	GrooveObject *FindGrooveObject(int x,int y)
	{
		GrooveObject *f=FirstGrooveObject();

		while(f)
		{
			if(f->x1<=x && f->x2>=x && f->y1<=y && f->y2>=y)return f;
			f=f->NextGrooveObject();
		}

		return 0;
	}

	guiGadgetList *glist;
	guiGadget_ListBox *groovegadget,*groovediffgadget;
	guiGadget *groovename,*grooveraster,*grooveinfo;
	//Edit_Frame frame_grooves,frame_display;
	OList grooves,groovepixel;

	// Move
	GrooveElement *MoveO;

	OSTART movediff;
	int startmouse_x,numbergrooves;
	double displaystep_pixel,displaystep_pixeltick;
	Groove *activegroove_shown;
};

#endif