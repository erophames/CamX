#include "songmain.h"
#include "editor.h"
#include "editsettings.h"
#include "camxgadgets.h"
#include "audiomixeditor.h"
#include "guiwindow.h"
#include "audiohardware.h"
#include "languagefiles.h"

#ifdef WIN32
#include<winuser.h>
#endif

void guiBitmap::guiStretchBlit(guiBitmap *to,int sourcew,int sourceh)
{
#ifdef WIN32
	StretchBlt(
		to->hDC,      // handle to destination DC
		0, // x-coord of destination upper-left corner
		0, // y-coord of destination upper-left corner
		to->width,   // width of destination rectangle
		to->height,  // height of destination rectangle
		hDC,       // handle to source DC
		0,  // x-coord of source upper-left corner
		0,  // y-coord of source upper-left corner
		sourcew,    // width of source rectangle
		sourceh,   // height of source rectangle
		SRCCOPY     // raster operation code
		);
#endif
}
void guiBitmap::guiStretchBlit(guiBitmap *to)
{
#ifdef WIN32
	StretchBlt(
		to->hDC,      // handle to destination DC
		0, // x-coord of destination upper-left corner
		0, // y-coord of destination upper-left corner
		to->width,   // width of destination rectangle
		to->height,  // height of destination rectangle
		hDC,       // handle to source DC
		0,  // x-coord of source upper-left corner
		0,  // y-coord of source upper-left corner
		width,    // width of source rectangle
		height,   // height of source rectangle
		SRCCOPY     // raster operation code
		);
#endif
}

void guiBitmap::Draw3DUp(int x,int y,int x2,int y2)
{
	guiDrawLineY(y,x,x2,COLOUR_WHITE);
	guiDrawLine(x,y,x,y2);
	guiDrawLine(x2,y,x2,y2,COLOUR_BLACK);
#ifdef WIN32
	guiDrawLineY(y2,x,x2+1);
#endif
}

void guiBitmap::guiFillRect3D(int x,int y,int x2,int y2,int colour)
{	
	// 3D Stuff
#ifdef WIN32
	//	SelectObject(hDC,colour_hPen[COLOUR_GREY]);

	if(x2>x+2 && y2>y+2)
	{
		guiFillRect(x,y,x2,y2,colour);
		Draw3DUp(x,y,x2,y2);
	}
	else
		guiFillRect(x,y,x2,y2,colour);
#endif
}

void guiBitmap::guiDrawImage(int x,int y,int x2,int y2,guiBitmap *f)
{	
	if(f && x2>x && y2>y)
	{
		int w=f->width;
		int h=f->height;

		if(x+w>x2)w=x2-x;
		if(y+h>y2)h=y2-y;

#ifdef WIN32
		HDC bmphdc = CreateCompatibleDC(hDC);
		SelectObject(bmphdc, f->hBitmap); // Add Bitmap

		BitBlt(
			hDC,
			x,
			y,
			w,
			h,
			bmphdc,
			0,
			0,
			SRCCOPY
			);

		DeleteDC(bmphdc);
	}
#endif	
}

void guiBitmap::guiDrawImage(int x,int y,int x2,int y2,int imageid)
{
	guiBitmap *f=maingui->gfx.FindBitMap(imageid);

	if(f)
	{
#ifdef WIN32
		HDC bmphdc = CreateCompatibleDC(hDC);
		SelectObject(bmphdc, f->hBitmap); // Add Bitmap

		BitBlt(
			hDC,
			x,
			y,
			f->width,
			f->height,
			bmphdc,
			0,
			0,
			SRCCOPY
			);

		DeleteDC(bmphdc);
	}
#endif	

}

void guiBitmap::guiDrawImageCenter(int x,int y,int x2,int y2,int imageid)
{
	guiBitmap *f=maingui->gfx.FindBitMap(imageid);

	if(f)
	{
		int gw=(x2-x)/2,gh=(y2-y)/2;

		x+=gw;
		y+=gh;

		x-=f->width/2;
		y-=f->height/2;

#ifdef WIN32
		HDC bmphdc = CreateCompatibleDC(hDC);

		SelectObject(bmphdc, f->hBitmap); // Add Bitmap

		BitBlt(
			hDC,
			x,
			y,
			f->width,
			f->height,
			bmphdc,
			0,
			0,
			SRCAND
			);

		DeleteDC(bmphdc);
	}
#endif		
}

void guiBitmap::BltFromBitMap(guiBitmap *to,int fromx,int fromy,int tox,int toy)
{
	//if(to && to->width>=tox-fromx && to->height>=toy-fromy)
	{
#ifdef WIN32
		BitBlt(
			to->hDC, // handle to destination device context
			0,  // x-coordinate of destination rectangle's upper-left 
			0,  // y-coordinate of destination rectangle's upper-left 
			(tox-fromx)+1,  // width of destination rectangle
			(toy-fromy)+1, // height of destination rectangle
			hDC,  // handle to source device context
			fromx,   // x-coordinate of source rectangle's upper-left 
			fromy,   // y-coordinate of source rectangle's upper-left 
			SRCCOPY  // raster operation code
			);
#endif
	}
}

void guiBitmap::Blt(int fx,int fy,int w,int h,int tox,int toy)
{
	#ifdef WIN32
		BitBlt(
			hDC, // handle to destination device context
			tox,  // x-coordinate of destination rectangle's upper-left 
			toy,  // y-coordinate of destination rectangle's upper-left 
			w,  // width of destination rectangle
			h, // height of destination rectangle
			hDC,  // handle to source device context
			fx,   // x-coordinate of source rectangle's upper-left 
			fy,   // y-coordinate of source rectangle's upper-left 
			// SRCCOPY  // raster operation code
			SRCCOPY
			);
#endif
}

void guiBitmap::BltToBitMap(guiBitmap *to,int tox,int toy)
{
#ifdef WIN32
		BitBlt(
			to->hDC, // handle to destination device context
			tox,  // x-coordinate of destination rectangle's upper-left 
			toy,  // y-coordinate of destination rectangle's upper-left 
			width,  // width of destination rectangle
			height, // height of destination rectangle
			hDC,  // handle to source device context
			0,   // x-coordinate of source rectangle's upper-left 
			0,   // y-coordinate of source rectangle's upper-left 
			// SRCCOPY  // raster operation code
			PATINVERT
			);
#endif
}

void guiBitmap::SetFont(Object *o)
{
	SetFont(o->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
}

guiFont *guiBitmap::SetFont(guiFont *f){

	if(f)
	{
		guiFont *oldfont=font;

		font=f;
		SelectObject(hDC,f->hfont);
		GetCharWidth32(hDC, 0, 255, fontsize);

		return oldfont;
	}

	return 0;
}

void guiBitmap::InitTextWidth()
{
	int s[10];

	s[0]=GetTextWidth("0");
	s[1]=GetTextWidth("1");
	s[2]=GetTextWidth("2");
	s[3]=GetTextWidth("3");
	s[4]=GetTextWidth("4");
	s[5]=GetTextWidth("5");
	s[6]=GetTextWidth("6");
	s[7]=GetTextWidth("7");
	s[8]=GetTextWidth("8");
	s[9]=GetTextWidth("9");

	int max=0;
	for( int i=0;i<10;i++)
		if(max<s[i])max=s[i];

	int space=GetTextWidth("W");

	pref_time[0]=5*max+space;
	pref_time[1]=2*max+space;
	pref_time[2]=2*max+space;
	pref_time[3]=4*max;

	pref_minuswidth=GetTextWidth("- ");

	int ss1=pref_minuswidth;

	for(int i=0;i<4;i++)
		ss1+=pref_time[i];

	pref_smpte[0]=3*max+space; // h
	pref_smpte[1]=2*max+space; // min
	pref_smpte[2]=2*max+space; // sec
	pref_smpte[3]=2*max+space; // frame
	pref_smpte[4]=1*max; // qf/ms

	int ss2=0;
	for(int i=0;i<5;i++)
		ss2+=pref_smpte[i];

	int add1=GetTextWidth(";");
	int add2=GetTextWidth(":");
	int add3=GetTextWidth(",");
	int add4=GetTextWidth(".");

	int size=add1;
	if(add2>size)
		size=add2;

	if(add3>size)
		size=add3;

	if(add3>size)
		size=add3;

	pref_space=size+1;

	prefertimebuttonsize=ss1<ss2?ss2:ss1;	
	//prefertimebuttonsize+=16;
}

int guiBitmap::GetTextWidthCxs(int from,int to)
{
	int l=0;

	for(int i=from;i<=to;i++)
	{
		int cl=GetTextWidth(Cxs[i]);

		if(cl>l)
			l=cl;
	}

	return l;
}
int guiBitmap::GetTextWidth(char *text)
{
	int lensum=0;
	size_t sl=strlen(text);

	while(sl--)
		lensum+=fontsize[*text++];

	return lensum;
}

int guiBitmap::SetAudioColour(double dbvalue)
{
	if(dbvalue>0.5) // 1 >
	{
		SetTextColour(255,150,150);
		return 1;
	}
	else if(dbvalue==0.5) // 1
	{
		SetTextColour(100,230,255);
		return 0;
	}

	// 1<
	SetTextColour(244,244,255);
	return -1;
}

void guiBitmap::guiDrawTextFontY(int x,int y,int x2,char *string)
{
	if(string /* && x>=0 && y>=0 && x2>x*/)
	{
#ifdef WIN32
		fillrect.left=x;
		fillrect.right=x2;

		fillrect.top=y-(maingui->GetFontSizeY()+maingui->buttonsizeaddy/2);
		fillrect.bottom=y;
		size_t slen=strlen(string);

		DrawText(hDC, string, slen, &fillrect, DT_SINGLELINE | DT_LEFT|DT_VCENTER);
#endif
	}
}

void guiBitmap::guiDrawText(int x,int y,int x2,char *string,int flag)
{
	if(string /* && x>=0 && y>=0 && x2>x*/)
	{
#ifdef WIN32
		fillrect.left=x;
		fillrect.right=x2;

		fillrect.top=y-maingui->GetButtonSizeY();
		fillrect.bottom=y;
		size_t slen=strlen(string);

		DrawText(hDC, string, slen, &fillrect, DT_SINGLELINE | DT_LEFT);

		if(flag&NO_MUTED)
		{
			guiDrawLineY(y-maingui->GetFontSizeY()/2,x,x2,COLOUR_RED);
		}

#endif
	}
}

void guiBitmap::guiDrawTextCenter(int x,int y,int x2,int y2,char *string)
{

	fillrect.left=x;
	fillrect.right=x2;
	fillrect.top=y;
	fillrect.bottom=y2;

	size_t slen=strlen(string);

	DrawText(hDC, string, slen, &fillrect, DT_SINGLELINE | DT_LEFT|DT_VCENTER|DT_CENTER);
}

void guiBitmap::guiDrawNumber(int x,int y,int x2,int number)
{
	if(x<x2)
	{
		char nrs[NUMBERSTRINGLEN];
		guiDrawText(x,y,x2,mainvar->ConvertIntToChar(number,nrs));
	}
}

int guiBitmap::guiGetPixelColor(int x,int y)
{
#ifdef WIN32
	COLORREF ref=GetPixel(
		hDC,   // handle to device context
		x,  // x-coordinate of pixel
		y  // y-coordinate of pixel
		);

	return ref;
#endif	
}

void guiBitmap::guiInvert(int x,int y,int x2,int y2)
{	
	//if(x>=0 && y>=0 && x2>=x && y2>=y)
	{
#ifdef WIN32
		BitBlt(
			hDC,
			x,
			y,
			(x2-x)+1,
			(y2-y)+1,
			hDC,
			x,
			y,
			DSTINVERT
			);
#endif
	}
}

void guiBitmap::guiFillRect_OSRGB(int x,int y,int x2,int y2,int rgb)
{
	fillrect.left=x;
	fillrect.top=y;
	fillrect.right=x2+1; 
	fillrect.bottom=y2+1; // +1

	//100,230,255

	if(HBRUSH hb=CreateSolidBrush(rgb))
	{
		//	SelectObject(hDC,hb);
		FillRect(hDC,&fillrect,hb);
		DeleteObject(hb);
	}
}

void guiBitmap::guiFillRect_RGB(int x,int y,int x2,int y2,int rgb)
{
#ifdef WIN32

	/*
	SelectObject(hDC,maingui->hPen[colour]);
	SelectObject(hDC,maingui->hBrush[colour]);
	Rectangle(hDC, x, y, x2, y2);
	*/
	//RECT rt;

	fillrect.left=x;
	fillrect.top=y;
	fillrect.right=x2+1; 
	fillrect.bottom=y2+1; // +1

	int r=GetRValue(rgb);
	int g=GetGValue(rgb);
	int b=GetBValue(rgb);

	//100,230,255

	if(HBRUSH hb=CreateSolidBrush(RGB(r,g,b)))
	{
		//	SelectObject(hDC,hb);
		FillRect(hDC,&fillrect,hb);
		DeleteObject(hb);
	}

	/*
	Rectangle(
	hDC,         // handle to device context
	x,   // x-coord of bounding rectangle's upper-left corner
	y,    // y-coord of bounding rectangle's upper-left corner
	x2,  // x-coord of bounding rectangle's lower-right corner
	y2  // y-coord of bounding rectangle's lower-right corner
	);
	*/
#endif
}


void guiBitmap::guiFillRect_RGB(int x,int y,int x2,int y2,int rgb,int bordercolour)
{
					#ifdef DEBUG
	if(bordercolour<0 || bordercolour>=LASTCOLOUR)
		maingui->MessageBoxError(0,"guiFillRect_RGB");
#endif

#ifdef WIN32

	/*
	SelectObject(hDC,maingui->hPen[colour]);
	SelectObject(hDC,maingui->hBrush[colour]);
	Rectangle(hDC, x, y, x2, y2);
	*/
	//RECT rt;

	guiDrawRect(x,y,x2,y2,bordercolour);

	fillrect.left=x;
	fillrect.top=y;
	fillrect.right=x2+1; 
	fillrect.bottom=y2+1; // +1

	int r=GetRValue(rgb);
	int g=GetGValue(rgb);
	int b=GetBValue(rgb);

	//100,230,255

	if(HBRUSH hb=CreateSolidBrush(RGB(r,g,b)))
	{
		//	SelectObject(hDC,hb);
		FillRect(hDC,&fillrect,hb);
		DeleteObject(hb);
	}

	/*
	Rectangle(
	hDC,         // handle to device context
	x,   // x-coord of bounding rectangle's upper-left corner
	y,    // y-coord of bounding rectangle's upper-left corner
	x2,  // x-coord of bounding rectangle's lower-right corner
	y2  // y-coord of bounding rectangle's lower-right corner
	);
	*/
#endif
}

#ifdef OLDIE
void guiBitmap::guiFillRect_Brush(int x,int y,int x2,int y2,HBRUSH brush)
{
#ifdef WIN32
	/*
	SelectObject(hDC,maingui->hPen[colour]);
	SelectObject(hDC,maingui->hBrush[colour]);
	Rectangle(hDC, x, y, x2, y2);
	*/

	//RECT rt;

	fillrect.left=x;
	fillrect.top=y;
	fillrect.right=x2+1; 
	fillrect.bottom=y2+1;

	FillRect(hDC,&fillrect,brush);
#endif
}
#endif

void guiBitmap::guiFillRect(int x,int y,int x2,int y2,int colour)
{
#ifdef DEBUG
	if(colour<0 || colour>=LASTCOLOUR)
		maingui->MessageBoxError(0,"guiFillRect");
#endif

#ifdef WIN32
	//
	// The FillRect function fills a rectangle by using the specified brush. 
	// This function includes the left and top borders, but excludes the right and bottom borders of the rectangle.
	fillrect.left=x;
	fillrect.top=y;
	fillrect.right=x2+1; 
	fillrect.bottom=y2+1;

	FillRect(hDC,&fillrect,colour_hBrush[colour]);
#endif
}

void guiBitmap::guiFillRectX0(int y,int x2,int y2,int colour)
{
#ifdef DEBUG
	if(colour<0 || colour>=LASTCOLOUR)
		maingui->MessageBoxError(0,"guiFillRect");
#endif

#ifdef WIN32
	//
	// The FillRect function fills a rectangle by using the specified brush. 
	// This function includes the left and top borders, but excludes the right and bottom borders of the rectangle.
	//fillrect.left=x;
	fillrectX0.top=y;
	fillrectX0.right=x2+1; 
	fillrectX0.bottom=y2+1;

	FillRect(hDC,&fillrectX0,colour_hBrush[colour]);
#endif
}

void guiBitmap::guiFillRect(int colour)
{
	#ifdef DEBUG
	if(colour<0 || colour>=LASTCOLOUR)
		maingui->MessageBoxError(0,"guiFillRect");
#endif

#ifdef WIN32
	//
	// The FillRect function fills a rectangle by using the specified brush. 
	// This function includes the left and top borders, but excludes the right and bottom borders of the rectangle.
	//fillrectX0.left=0;
	fillrectX0.top=0;
	fillrectX0.right=width; 
	fillrectX0.bottom=height;

	FillRect(hDC,&fillrectX0,colour_hBrush[colour]);
#endif
}

void guiBitmap::guiFillRect(int x,int y,int x2,int y2,int colour,int bordercolour)
{
#ifdef WIN32
	/*
	SelectObject(hDC,maingui->hPen[colour]);
	SelectObject(hDC,maingui->hBrush[colour]);
	Rectangle(hDC, x, y, x2, y2);
	*/

	//RECT fillrect;

	guiDrawRect(x,y,x2,y2,bordercolour);

	fillrect.left=x+1;
	fillrect.top=y+1;
	fillrect.right=x2; 
	fillrect.bottom=y2;

	FillRect(hDC,&fillrect,colour_hBrush[colour]);
#endif
}

void guiBitmap::guiFillRectX0(int y,int x2,int y2,int colour,int bordercolour)
{
#ifdef WIN32
	guiDrawRect(0,y,x2,y2,bordercolour);

	fillrect.left=1;
	fillrect.top=y+1;
	fillrect.right=x2; 
	fillrect.bottom=y2;

	FillRect(hDC,&fillrect,colour_hBrush[colour]);
#endif
}

void guiBitmap::guiFill(int x,int y,int colour)
{
		#ifdef DEBUG
	if(colour<0 || colour>=LASTCOLOUR)
		maingui->MessageBoxError(0,"guiFill");
#endif

#ifdef WIN32
	SelectObject(hDC,colour_hBrush[colour]);
	ExtFloodFill(hDC,x,y,guiGetPixelColor(x,y),FLOODFILLSURFACE);
#endif
}

void guiBitmap::guiDrawRect_RGB(int x,int y,int x2,int y2,int rgb)
{
	/*
	int r=GetRValue(rgb);
	int g=GetGValue(rgb);
	int b=GetBValue(rgb);

	//100,230,255

	COLORREF ref=RGB(r, g, b);
*/

	HPEN pen=CreatePen(PS_SOLID, 1, rgb);

#ifdef WIN32
		SelectObject(hDC,pen);
#endif

	pntArray[0].x=x;
	pntArray[0].y=y;

	pntArray[1].x=x2;
	pntArray[1].y=y;

	pntArray[2].x=x2;
	pntArray[2].y=y2;

	pntArray[3].x=x;
	pntArray[3].y=y2;

	pntArray[4].x=x;
	pntArray[4].y=y;

	Polyline(hDC, pntArray, 5);

#ifdef WIN32
	DeleteObject(pen);
#endif

}

void guiBitmap::guiDrawRect(int x,int y,int x2,int y2,int colour)
{
			#ifdef DEBUG
	if(colour<0 || colour>=LASTCOLOUR)
		maingui->MessageBoxError(0,"guiDrawRect");
#endif

	SetAPen(colour);

	pntArray[0].x=x;
	pntArray[0].y=y;

	pntArray[1].x=x2;
	pntArray[1].y=y;

	pntArray[2].x=x2;
	pntArray[2].y=y2;

	pntArray[3].x=x;
	pntArray[3].y=y2;

	pntArray[4].x=x;
	pntArray[4].y=y;

	Polyline(hDC, pntArray, 5);

#ifdef OLDIE
	return;

#ifdef WIN32
	MoveToEx(hDC, x, y, NULL);
	LineTo(hDC, x2, y);	
	LineTo(hDC, x2, y2);	
	LineTo(hDC, x, y2);	
	LineTo(hDC, x, y);
#endif

#endif
}

#ifdef OLDIE
void guiBitmap::guiDrawRoundRect(int x,int y,int x2,int y2,int colour)
{
				#ifdef DEBUG
	if(colour<0 || colour>=LASTCOLOUR)
		maingui->MessageBoxError(0,"guiDrawRoundRect");
#endif

	SetAPen(colour);

	pntArray[0].x=x;
	pntArray[0].y=y;

	pntArray[1].x=x2;
	pntArray[1].y=y;

	pntArray[2].x=x2;
	pntArray[2].y=y2;

	pntArray[3].x=x;
	pntArray[3].y=y2;

	pntArray[4].x=x;
	pntArray[4].y=y;

	Polyline(hDC, pntArray, 5);

#ifdef OLDIE
	return;

#ifdef WIN32
	MoveToEx(hDC, x, y, NULL);
	LineTo(hDC, x2, y);	
	LineTo(hDC, x2, y2);	
	LineTo(hDC, x, y2);	
	LineTo(hDC, x, y);
#endif

#endif
}
#endif

void guiBitmap::guiDrawNumberObject(int x,int y,int x2,int number,/*NumberObject *obj,*/int flag)
{
	//if(x2>width)x2=width-1;

	//if(x<x2 && x<width)
	{

		/*
		if(obj){
			obj->x=x;
			obj->x2=x2;
			obj->y=y-maingui->GetFontSizeY();

			if(obj->y<0)obj->y=0;
			obj->y2=y;
			obj->ondisplay=true;
		}
*/

		if(flag&NO_SHOWNUMBER)
		{
			guiDrawNumber(x,y,x2,number);
			if(flag&NO_SELECTED)
				guiDrawNumber(x+1,y,x2,number);
		}
		else 
		{
			guiDrawText(x,y,x2,"_");
			if(flag&NO_SELECTED)
				guiDrawText(x+1,y,x2,"_");
		}

		if(flag&NO_MUTED)
		{
			guiDrawLineY(y-maingui->GetFontSizeY()/2,x,x2,COLOUR_RED);
		}

	}
	/*
	else
	{
	if(obj)obj->ondisplay=false;
	}
	*/
}

void guiBitmap::guiDrawNumberObject(int x,int y,int x2,char *string,/*NumberObject *obj,*/int flag)
{
	//if(x2>width)x2=width-1;

	//	if(x<x2 && x<width){

	/*
	if(obj){

		obj->x=x;
		obj->x2=x2;
		obj->y=y-maingui->GetFontSizeY();
		if(obj->y<0)
			obj->y=0;

		obj->y2=y;
		obj->ondisplay=true;

		//TRACE ("Draw String Object %s\n",string);
	}
*/

	guiDrawText(x,y,x2,string);
	if(flag&NO_SELECTED)
		guiDrawText(x+1,y,x2,string);

	if(flag&NO_MUTED)
	{
		guiDrawLineY(y-maingui->GetFontSizeY()/2,x,x2,COLOUR_RED);
	}

	/*
	}
	else{
	if(obj)obj->ondisplay=false;
	}
	*/
}