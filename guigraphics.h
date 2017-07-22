#ifndef CAMX_GUIGRAPHICS_H
#define CAMX_GUIGRAPHICS_H 1

#include "object.h"
#include "defines.h"
#include "guifont.h"
#include "imagesdefines.h"

class GUI;
class guiWindow;
class NumberObject;
class guiObject;
class Seq_Track;
class TrackHead;
class AutomationTrack;

enum{
	NO_SHOWNUMBER=1,
	NO_SELECTED=2,
	NO_MUTED=4
};

class guiBitmap:public Object
{
	friend class guiGFX;

public:
	guiBitmap(){
#ifdef WIN32
		hBitmap=0;
		hDC=0;
		pntArrayX0[0].x=0;
		fillrectX0.left=0;
#endif
		font=0;
	}

	guiBitmap *NextBitMap() {return(guiBitmap *)next;}

	int GetX2(){return width-1;}
	int GetY2(){return height-1;}

	void guiStretchBlit(guiBitmap *to);
	void guiStretchBlit(guiBitmap *to,int sourcew,int sourceh);
	void BltFromBitMap(guiBitmap *to,int fromx,int fromy,int tox,int toy);
	void BltToBitMap(guiBitmap *to,int tox,int toy);
	void Blt(int fx,int fy,int w,int h,int tox,int toy);
	int SetAudioColour(double value);

	inline void SetAPen(int colour){

#ifdef WIN32
		SelectObject(hDC,colour_hPen[colour]);
#endif
	}

	void SetTextColour(int colour){
#ifdef WIN32
		SetTextColor(hDC,colour_ref[colour]);
#endif
	}

	inline void SetTextColour(UBYTE r,UBYTE g,UBYTE b){
#ifdef WIN32
		SetTextColor(hDC,RGB(r,g,b));
#endif
	}

	inline void SetBackgroundColour(UBYTE r,UBYTE g,UBYTE b){
#ifdef WIN32
		SetBkColor(hDC,RGB(r,g,b));
#endif
	}

	guiFont *SetFont(guiFont *);
	void SetFont(Object *);
	void InitTextWidth();
	int GetTextWidth(char *);
	int GetTextWidthCxs(int from,int to);
	void guiDrawImage(int x,int y,int x2,int y2,int imageid);
	void guiDrawImage(int x,int y,int x2,int y2,guiBitmap *);
	void guiDrawImageCenter(int x,int y,int x2,int y2,int imageid);
	void guiDrawText(int x,int y,int x2,char *,int flag=0);
	void guiDrawTextFontY(int x,int y,int x2,char *);
	void guiDrawTextCenter(int x,int y,int x2,int y2,char *);
	void guiDrawNumber(int x,int y,int x2,int number);
	void guiDrawNumberObject(int x,int y,int x2,int number,int flag=0);
	void guiDrawNumberObject(int x,int y,int x2,char *string,int flag=0);
	void guiInvert(int x,int y,int x2,int y2);

	void guiDrawCircle(int x,int y,int x2,int y2){
#ifdef WIN32
		Ellipse(hDC,x,y,x2,y2);	
#endif
	}

	void guiDrawCircle(int x,int y,int x2,int y2,int colour){
		SetAPen(colour);
		SelectObject(hDC,colour_hBrush[colour]);
		guiDrawCircle(x,y,x2,y2);
	}

	// Lines
	inline void guiDrawLine(int x,int y,int x2,int y2){
		pntArray[0].x=x;
		pntArray[0].y=y;
		pntArray[1].x=x2;
		pntArray[1].y=y2;

		Polyline(hDC, pntArray, 2);
	}

	inline void guiDrawLineX(int x,int y,int y2){
		pntArray[0].x=pntArray[1].x=x;
		pntArray[0].y=y;
		pntArray[1].y=y2+1;

		Polyline(hDC, pntArray, 2);
	}

	inline void guiDrawLineX(int x,int y,int y2,int colour){
		SetAPen(colour);
		pntArray[0].x=pntArray[1].x=x;
		pntArray[0].y=y;
		pntArray[1].y=y2+1;

		Polyline(hDC, pntArray, 2);
	}

	inline void guiDrawLineX(int x,int colour){
		SetAPen(colour);

		pntArray[0].x=pntArray[1].x=x;
		pntArray[0].y=0;
		pntArray[1].y=GetY2();

		Polyline(hDC, pntArray, 2);
	}

	inline void guiDrawLineY(int y,int x,int x2){
		pntArray[0].x=x;
		pntArray[0].y=pntArray[1].y=y;
		pntArray[1].x=x2+1;

		Polyline(hDC, pntArray, 2);
	}

	inline void guiDrawLineYX0(int y,int x2){
		//	pntArrayX0[0].x=0;
		pntArrayX0[0].y=pntArrayX0[1].y=y;
		pntArrayX0[1].x=x2+1;

		Polyline(hDC, pntArrayX0, 2);
	}

	inline void guiDrawLineY(int y,int x,int x2,int colour){
		SetAPen(colour);

		pntArray[0].x=x;
		pntArray[0].y=pntArray[1].y=y;
		pntArray[1].x=x2+1;

		Polyline(hDC, pntArray, 2);
	}

	inline void guiDrawLineYX0(int y,int x2,int colour){
		SetAPen(colour);

		//pntArrayX0[0].x=0;
		pntArrayX0[0].y=pntArrayX0[1].y=y;
		pntArrayX0[1].x=x2+1;

		Polyline(hDC, pntArrayX0, 2);
	}

	inline void guiDrawLine(int x,int y,int x2,int y2,int colour){
		SetAPen(colour);

#ifdef WIN32
		pntArray[0].x=x;
		pntArray[0].y=y;
		pntArray[1].x=x2;
		pntArray[1].y=y2;

		Polyline(hDC, pntArray, 2);
#endif
	}

	inline void guiDrawLine_RGB_Y(int y,int x,int x2,int rgb){
#ifdef WIN32
		if(HPEN npen=CreatePen(PS_SOLID, 1, rgb))
		{
			SelectObject(hDC,npen);

			pntArray[0].x=x;
			pntArray[0].y=pntArray[1].y=y;
			pntArray[1].x=x2+1;

			Polyline(hDC, pntArray, 2);

			DeleteObject(npen);
		}
#endif
	}

	inline void guiDrawLine_RGB(int x,int y,int x2,int y2,int rgb){
#ifdef WIN32
		if(HPEN npen=CreatePen(PS_SOLID, 1, rgb)){
			SelectObject(hDC,npen);

			pntArray[0].x=x;
			pntArray[0].y=y;
			pntArray[1].x=x2;
			pntArray[1].y=y2;

			Polyline(hDC, pntArray, 2);

			DeleteObject(npen);
		}
#endif
	}

	int guiGetPixelColor(int x,int y);

	inline void guiDrawPixel(int x,int y,int col){
#ifdef WIN32
		SetPixel(hDC,x,y,colour_ref[col]);
#endif
	}

	inline void guiDrawPixel(int x,int y,int r,int g,int b){
#ifdef WIN32
		SetPixel(hDC,x,y,RGB(r,g,b));
#endif
	}

	void guiFillRect_RGB(int x,int y,int x2,int y2,int rgb,int bordercolour);
	void guiFillRect_RGB(int x,int y,int x2,int y2,int rgb);
	void guiFillRect_OSRGB(int x,int y,int x2,int y2,int rgb);

	void guiFillRect(int x,int y,int x2,int y2,int colour);
	void guiFillRect(int x,int y,int x2,int y2,int colour,int bordercolour);

	void guiFillRectX0(int y,int x2,int y2,int colour);
	void guiFillRectX0(int y,int x2,int y2,int colour,int bordercolour);

	void guiFillRect(int colour);
	void guiFill(int x,int y,int colour);

	void guiDrawRect(int x,int y,int x2,int y2,int colour);
	void guiDrawRect_RGB(int x,int y,int x2,int y2,int rgb);

	void Draw3DUp(int x,int y,int x2,int y2);
	void guiFillRect3D(int x,int y,int x2,int y2,int colour);

	void ShowMute(guiObject *,bool status,int bgcolour);
	void ShowMute(int x,int y,int x2,int y2,bool status,int bgcolour);

	void ShowSolo(guiObject *,int status,int bgcolour);
	void ShowSolo(int x,int y,int x2,int y2,int status,int bgcolour);

	void ShowRecord(guiObject *,Seq_Track *,bool status,int bgcolour);
	void ShowInputMonitoring(guiObject *,Seq_Track *,bool status,int bgcolour);
	void ShowThru(guiObject *,bool status,bool MIDI,int bgcolour);
	void ShowAutomationMode(int x,int y,int x2,int y2,TrackHead *);
	void ShowAutomationSettings(int x,int y,int x2,int y2,TrackHead *);

	void ShowAutomationTrackVisible(int x,int y,int x2,int y2,AutomationTrack *);

	void ShowChildTrackMode(int x,int y,int x2,int y2,Seq_Track *,bool withnumber=false);
	void ShowChildTrackMode(guiObject *,Seq_Track *,bool withnumber=false);

#ifdef WIN32
	RECT fillrect,fillrectX0; // fill
	POINT pntArray[5],pntArrayX0[5]; // drawrect
	HBITMAP hBitmap;
	HDC hDC; // drawfunctions
#endif

	guiFont *font;
	int width,height,prefertimebuttonsize,pref_minuswidth,pref_time[4],pref_smpte[5],pref_space,
		fontsize[256];
};

class TrackIcon:public Object{

public:
	TrackIcon(){
		filename=0;
		bitmap=0;
	}

	char *GetIconName(){return filename;}
	TrackIcon *NextIcon(){return (TrackIcon *)next;}

	char *filename;
	guiBitmap *bitmap;
};

class guiGFX
{
public:
	guiGFX(){for(int i=0;i<LAST_IMAGE;i++)images[i]=0;}

	guiBitmap* AddNewBitMap(int width,int height,int depth);
	guiBitmap* DeleteBitMap(guiBitmap *);
	guiBitmap* FindBitMap(int id);
	guiBitmap* FindBitMapList(int id);
	guiBitmap *LoadImageToBitMap(char *,int w,int h,int id,bool addtolist=true,bool addimagesdir=true);

	void DeleteAllBitMaps();
	bool InitAllBitMaps();
	guiBitmap* FirstBitMap(){return (guiBitmap *)bitmaps.GetRoot(); }

	bool InitTrackIcons();
	void DeleteAllTrackIcons();
	TrackIcon *DeleteTrackIcon(TrackIcon *);
	TrackIcon *FirstTrackIcon(){return (TrackIcon *)trackicons.GetRoot(); }
	TrackIcon *FindIcon(char *);

	guiBitmap *images[LAST_IMAGE];
	GUI *gui;

private:
	OList bitmaps,trackicons;
};
#endif