#ifdef OLDIE

#ifndef CAMX_GUIFRAME
#define CAMX_GUIFRAME 1

#include "object.h"
#include "defines.h"

#ifdef WIN32
#include <afxwin.h>
#endif

class guiWindow;

#define FRAME_TYPE_STANDARD 0
#define FRAME_TYPE_CROSS 1

#define GUIFRAME_LEFT 1
#define GUIFRAME_RIGHT 2
#define GUIFRAME_TOP 4
#define GUIFRAME_BOTTOM 8

// gui IDs
#define GUIFRAME_ID_HEADER 555

class Edit_Frame;

class guiFrame:public Object
{
	friend class guiWindow;
	friend class guiTimeLine;

public:

	guiFrame()
	{
		editframe=0;
	}

	void DrawBorder();
	//void ClearBackground();
	bool CheckIfInFrame(int x,int y);
	bool CheckIfInFrame_TopBar(int x,int y);
	bool CheckIfInFrame_BottomBar(int x,int y);
	bool CheckIfInFrame_LeftBar(int x,int y);
	bool CheckIfInFrame_RightBar(int x,int y);
	guiFrame *NextFrame(){return (guiFrame *)next;} 
	
	Edit_Frame *editframe;
	guiWindow *win;
	
#ifdef WIN32
	HWND hWnd;
#endif
	int id,type;
};

#endif


#endif