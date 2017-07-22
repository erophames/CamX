#ifdef OLDIE

#include "gui.h"

#define CHECKRANGE 2

bool Edit_Frame::CheckIfDisplay(guiWindow *win,int subx,int suby)
{
	if(on==true && win)
	{
		ondisplay=true;

		if( win->width>-subx &&
			win->height>-suby &&
			x2>x &&
			y2>y &&
			x>=0 && 
			y>=0 && 
			x<win->width+subx && 
			y<win->height+suby
			)
		{	
			if(x2>win->width+subx)
			{
				x2=(win->width-1)+subx;

				if(x2<=x)
					ondisplay=false;
			}

			if(y2>win->height+suby)
			{
				y2=(win->height-1)+suby;

				if(y2<=y)
					ondisplay=false;
			}
		}
		else
			ondisplay=false;
	}
	else
		ondisplay=false;

	return ondisplay;
}

bool Edit_Frame::CheckIfDisplay(guiWindow *win)
{
	if(on==true && win)
	{
		ondisplay=true;

		if(x2>x &&
			y2>y &&
			x>=0 && 
			y>=0 && 
			x<=win->width && 
			y<=win->height
			)
		{	
			if(x2>=win->width)
			{
				x2=win->width-1;
				if(x2<=x)
					ondisplay=false;
			}

			if(y2>=win->height)
			{
				y2=win->height-1;
				if(y2<=y)
					ondisplay=false;
			}
		}
		else
			ondisplay=false;
	}
	else
		ondisplay=false;

	return ondisplay;
}

int Edit_Frame::SetToX(int cx)
{
	if(cx<x)
		return x;

	if(cx>x2)
		return x2;

	return cx;
}

Edit_Frame::Edit_Frame()
{
	guiframe=0;
	win=0;

	apen=COLOUR_BLACK;
	bpen=COLOUR_BACKGROUND;
	bpen2=COLOUR_BACKGROUND2;

	settingsvar=0;
	id=0;

	min_x=min_y=min_x2=min_y2=
		max_x=max_y=max_x2=max_y2=-1; // no limits

	infostring=0;
	infostringflag=0;
	infowindow=false;
	on=true;
	flag=0;

#ifdef WIN32
	hWnd=0;
#endif

	createOSwindow=false;
};

void Edit_Frame::ShowInfoWindow(guiBitmap *bitmap)
{	
	infostringflag CLEARBIT INFOSTRINGFLAG_INFOWINDOW;

	if(infowindow==true && bitmap)
	{
		int x2=infowindow_x2;

		if(x2>infowindow_x+4)
		{
			if(infostringflag&INFOSTRINGFLAG_TOP)
			{
				bitmap->guiFillRect_RGB(infowindow_x,infowindow_y,x2,infowindow_y2,0xEEDDDD);

				bitmap->SetAPen(COLOUR_BLACK);
				bitmap->guiDrawLine(infowindow_x,infowindow_y+1,infowindow_x,infowindow_y2);
				bitmap->guiDrawLine(infowindow_x+1,infowindow_y,x2-1,infowindow_y);
				bitmap->guiDrawLine(x2-1,infowindow_y+1,x2-1,infowindow_y2);
				bitmap->SetBackgroundColour(0xDD,0xDD,0xDD);

				UBYTE r,g,b;
				maingui->colourtable.GetRGB(COLOUR_BLACK,&r,&g,&b);
				bitmap->SetTextColour(r,g,b);
				
				bitmap->guiDrawText(infowindow_x+2,infowindow_y2,x2-3,infostring);

				infostringflag |= INFOSTRINGFLAG_INFOWINDOW;
			}
			else
				if(infostringflag&INFOSTRINGFLAG_LEFT)
				{
					bitmap->guiFillRect_RGB(infowindow_x,infowindow_y,x2,infowindow_y2,0xCCFFEE);

					bitmap->SetAPen(COLOUR_BLACK);
					bitmap->guiDrawLine(infowindow_x+1,infowindow_y,infowindow_x2,infowindow_y);
					bitmap->guiDrawLine(infowindow_x,infowindow_y+1,infowindow_x,infowindow_y2-1);
					bitmap->guiDrawLine(infowindow_x+1,infowindow_y2-1,infowindow_x2,infowindow_y2-1);
					
					bitmap->SetBackgroundColour(0xDD,0xDD,0xDD);
					
					UBYTE r,g,b;
					maingui->colourtable.GetRGB(COLOUR_BLACK,&r,&g,&b);

					bitmap->SetTextColour(r,g,b);

					int sx=infowindow_x2-2-bitmap->GetTextWidth(infostring);

					if(sx<infowindow_x+1)
						sx=infowindow_x+1;

					bitmap->guiDrawText(sx,infowindow_y2-2,infowindow_x2,infostring);

					infostringflag |= INFOSTRINGFLAG_INFOWINDOW;
				}
		}
	}
}

int Edit_Frame::SetToY(int cy)
{
	if(cy<y)
		return y;

	if(cy>y2)
		return y2;

	return cy;
}

void Edit_Frame::Fill(guiWindow *win,int colour)
{
#ifdef OLDIE
	if(ondisplay==true && win)
		win->bitmap.guiFillRect(x,y,x2,y2,colour);
#endif
}

void Edit_Frame::Fill(guiBitmap *bm,int colour)
{
#ifdef OLDIE
	if(ondisplay==true && bm)
		bm->guiFillRect(x,y,x2,y2,colour);
#endif
}

void Edit_Frame::Fill3D(guiWindow *win,int colour)
{
	//if(ondisplay==true && win)win->bitmap.guiFillRect3D(x,y,x2,y2,colour);
}

void Edit_Frame::Fill3D(guiBitmap *bm,int colour)
{
	//if(ondisplay==true && bm)bm->guiFillRect3D(x,y,x2,y2,colour);
}

bool Edit_Frame::CheckIfInFrame(int px,int py)
{
	if(ondisplay==true && px>=x && px<=x2 && py>=y && py<=y2)return true;
	return false;
}

bool Edit_Frame::CheckIfInHeader(int px,int py)
{
	/*
	if(ondisplay==true && infowindow==true && 
		infowindow_x<=px && 
		infowindow_x2>=px && 
		infowindow_y<=py && 
		infowindow_y2>=py)
		return true;	
*/
	return false;
}

bool guiFrame::CheckIfInFrame(int px,int py)
{
	if(px>=editframe->x && 
		px<=editframe->x2 && 
		py>=editframe->y && 
		py<=editframe->y2
		)
		return true;

	return false;
}

bool guiFrame::CheckIfInFrame_TopBar(int px,int py)
{
	if((flag&GUIFRAME_TOP) && 
		px>=editframe->x && 
		px<=editframe->x2 && 
		py>=editframe->y-2 && 
		py<=editframe->y+2
		)
		return true;

	return false;
}

bool guiFrame::CheckIfInFrame_BottomBar(int px,int py)
{
	if((flag&GUIFRAME_BOTTOM) && 
		px>=editframe->x && 
		px<=editframe->x2 && 
		py>=editframe->y2-CHECKRANGE && 
		py<=editframe->y2+CHECKRANGE
		)
		return true;

	return false;
}

bool guiFrame::CheckIfInFrame_LeftBar(int px,int py)
{
	if((flag&GUIFRAME_LEFT) && 
		px>=editframe->x-CHECKRANGE && 
		px<=editframe->x+CHECKRANGE && 
		py>=editframe->y && 
		py<=editframe->y2
		)
		return true;

	return false;
}

bool guiFrame::CheckIfInFrame_RightBar(int px,int py)
{
	if((flag&GUIFRAME_RIGHT) && 
		px>=editframe->x2-CHECKRANGE && 
		px<=editframe->x2+CHECKRANGE && 
		py>=editframe->y && 
		py<=editframe->y2
		)
		return true;

	return false;
}

/*
void guiFrame::ClearBackground()
{
	if(type!=FRAME_TYPE_CROSS)
	{
		win->bitmap.guiFillRect(editframe->x,editframe->y,editframe->x2,editframe->y2,COLOUR_BACKGROUND);

		if(editframe->infowindow==true)
		{
			win->bitmap.guiFillRect(
				editframe->infowindow_x,
				editframe->infowindow_y,
				editframe->infowindow_x2,
				editframe->infowindow_y2,
				COLOUR_BACKGROUND);
		}
	}
}
*/

void guiFrame::DrawBorder()
{
	/*
	guiDrawRect(userx,usery,userx2,usery2,COLOUR_BLACK);

	userx++;
	usery++;
	usery2--;
	userx2--;

	if(flag&GUIFRAME_TOP)
	{
	guiDrawLine(x,y,x2,y,COLOUR_WHITE);
	guiDrawLine(x,y+1,x2,y+1,COLOUR_BLACK);
	}

	if(flag&GUIFRAME_BOTTOM)
	{
	guiDrawLine(x,y2,x2,y2,COLOUR_WHITE);
	guiDrawLine(x,y2-1,x2,y2-1,COLOUR_BLACK);
	}

	if(flag&GUIFRAME_LEFT)
	{

	guiDrawLine(x+2,y,x+2,y2,COLOUR_BLACK);

	userx+=2;
	}

	if(flag&GUIFRAME_RIGHT)
	{
	guiDrawLine(x2-2,y,x2-2,y2,COLOUR_BLACK);

	userx2-=2;
	}
	*/
}
#endif