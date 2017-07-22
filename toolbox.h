#ifndef CAMX_GUITOOLBOX
#define CAMX_GUITOOLBOX 1

class guiGadget;
class guiWindow;
class guiGadgetList;
class EventEditor;

#define DEFAULT_TOOLSIZE 140;

class guiControlBox
{
public:
	guiControlBox();

	void ResetGadgets();
	int GetY2(int hy);
	void ShowStatus(int status);
	void ShowTime();

	guiGadgetList *glist;
	guiGadget *startbutton,*stopbutton,*recordbutton,*timebutton;
	EventEditor *editor;
	int status,x,y,x2,y2;
};

#define TOOLBOXTYPE_SELECT (1<<1)
#define TOOLBOXTYPE_CREATE (1<<2)
#define TOOLBOXTYPE_EDIT (1<<3)
#define TOOLBOXTYPE_CUT (1<<4)
#define TOOLBOXTYPE_DELETE (1<<5)
#define TOOLBOXTYPE_RANGE (1<<6)
#define TOOLBOXTYPE_ARRANGEMENUS (1<<7)
#define TOOLBOXTYPE_FOLLOWSPP (1<<8)
#define TOOLBOXTYPE_NOWINDOWCHANGING (1<<9)
#define TOOLBOXTYPE_OVERVIEW (1<<10)
#define TOOLBOXTYPE_QUANTIZE (1<<11)
#define TOOLBOXTYPE_XY (1<<12)
#define TOOLBOXTYPE_ARRANGE_F (1<<13)
#define TOOLBOXTYPE_EDITOR_F (1<<14)
#define TOOLBOXTYPE_AUDIOMIXER (1<<15)
#define TOOLBOXTYPE_MARKER_F (1<<16)
#define TOOLBOXTYPE_MOVEPATTERN (1<<17)

#define TOOLBOXTYPE_ARRANGE TOOLBOXTYPE_ARRANGE_F|TOOLBOXTYPE_XY|TOOLBOXTYPE_SELECT|TOOLBOXTYPE_CREATE|TOOLBOXTYPE_CUT|TOOLBOXTYPE_DELETE|TOOLBOXTYPE_RANGE|TOOLBOXTYPE_ARRANGEMENUS|TOOLBOXTYPE_FOLLOWSPP|TOOLBOXTYPE_NOWINDOWCHANGING|TOOLBOXTYPE_OVERVIEW|TOOLBOXTYPE_QUANTIZE
#define TOOLBOXTYPE_EDITOR TOOLBOXTYPE_EDITOR_F|TOOLBOXTYPE_XY|TOOLBOXTYPE_SELECT|TOOLBOXTYPE_EDIT|TOOLBOXTYPE_CREATE|TOOLBOXTYPE_CUT|TOOLBOXTYPE_DELETE|TOOLBOXTYPE_RANGE|TOOLBOXTYPE_FOLLOWSPP|TOOLBOXTYPE_OVERVIEW|TOOLBOXTYPE_QUANTIZE
#define TOOLBOXTYPE_MARKER TOOLBOXTYPE_MARKER_F|TOOLBOXTYPE_NOWINDOWCHANGING|TOOLBOXTYPE_SELECT|TOOLBOXTYPE_CREATE|TOOLBOXTYPE_DELETE|TOOLBOXTYPE_FOLLOWSPP|TOOLBOXTYPE_QUANTIZE|TOOLBOXTYPE_MOVEPATTERN

class guiToolBox
{
public:
	guiToolBox();

	enum{
		CTB_NOOVERVIEWVERT=1,
		CTB_NOSPP=2
	};

	void ShowStatus();
	bool CreateToolBox(int types,int iflag=0);
	void RefreshRealtime();
	void ShowTimeType();
	bool Key_Down(int key);

	guiGadget *selectgadget,*cutgadget,*drawgadget,*deletegadget,*editgadget,*rangegadget,*statustext,*sppsync,*overview,*timetype,
		*xmove,*ymove,*quantgadget;

	guiWindow *editor;
	int edittypes,timesync_status,status;
	bool box_ok,sppstatus,sppstopped;
};
#endif