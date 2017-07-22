#ifdef OLDIE

#ifndef CAMX_EDITFRAME_H
#define CAMX_EDITFRAME_H 1

#include "guigraphics.h"

class guiWindow;
class guiFrame;

#define INFOSTRINGFLAG_TOP 1
#define INFOSTRINGFLAG_LEFT 2
#define INFOSTRINGFLAG_RIGHT 3
#define INFOSTRINGFLAG_BOTTOM 4
#define INFOSTRINGFLAG_INFOWINDOW 8

class Edit_Frame
{
public:
	Edit_Frame();

	virtual void Changed(){};

	void ShowInfoWindow(guiBitmap *);

	int GetWidth(){return x2-x;}
	int GetHeight(){return y2-y;}

	bool CheckIfInFrame(int x,int y);
	bool CheckIfInHeader(int px,int py);

	bool CheckIfDisplay(guiWindow *);
	bool CheckIfDisplay(guiWindow *,int subx,int suby);

	void Fill(guiWindow *,int colour);
	void Fill(guiBitmap *,int colour);
	void Fill3D(guiWindow *,int colour);
	void Fill3D(guiBitmap *,int colour);

	void CreateBufferWindow();
	void DeleteBufferWindow();
	void PasteBufferWindow();

	int SetToX(int x);
	int SetToY(int y);

	guiBitmap bitmap;
	guiFrame *guiframe;
	guiWindow *win;
	char *infostring;

#ifdef WIN32
	HWND hWnd;
#endif

	int x,y,x2,y2,
		min_x,min_y,min_x2,min_y2,
		max_x,max_y,max_x2,max_y2,
		infowindow_x,infowindow_y,infowindow_x2,infowindow_y2,
		*settingsvar,value,addy,
		apen, // foreground colour
		bpen, // background colour
		bpen2, //
		id,infostringflag,flag;

	bool on,ondisplay,infowindow,createOSwindow;
};

#endif

#endif
