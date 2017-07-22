/*
#include <stdio.h>
#include <math.h>
#include "gui.h"


double GUI::GetRButtonValue(int posx,int posy,int x,int y,int x2,int y2)
{
	// return value 0-1

	double v;
	double dx=x+((x2-x)/2);
	double dy=y+((y2-y)/2);

	double fak = 1 / 3.14159; 

	dx=posx-dx;
	dy=posy-dy;

	v = atan2 ((double) dx, (double) -dy);  // 1/(2 * pi / 64)

	v*=fak;

	if(v<0) // Left
	{
		v=1+v;
		v/=2;
	}
	else  // Right
		v=0.5+v/2;
	
	return v;
}

void GUI::DrawRButton (guiWindow *win,int x, int y, double ra, double ri)
{
  double r1, r2, r3, r4, xx, yy;
  double w;

  r1 = ra * 0.75;
  r4 = ra * 0.2;
  r2 = r1 - r4;
  r3 = r1 + r4;

  // w = ri * drehfak;
  w=ri*2*3.14159;

  xx = x - r1 * sin (w);
  yy = y + r1 * cos (w);

  win->bitmap.guiDrawCircle((int)(xx-4),(int)(yy-4),(int)(xx+8),(int)(yy+8),COLOUR_BLACK);
}
*/

/*
#include "include.h"
#include <hardware/custom.h>
#include <hardware/cia.h>
#include <hardware/intbits.h>


#include <proto/cia.h>

#include "blockdefines.h"

#include <math.h>

static USHORT drehpos;
static UBYTE drehwhat, redit;
static double drehfak;



void
ShowRegler ()
{
  struct RastPort *r = DrehWnd->RPort;
  ULONG v = timer->songtakte / 4;

  drehfak = 2 * (3.14159 / timer->songtakte);
  EraseRect (r, 7, 36, 110, 120);
  EraseRect (r, 141, 49, 163, 120);
  DrawImage (r, &reglerim, 24, 45);
  SetFont (r, GuruPtr);
  SetAPen (r, 1);

  Text_NPos (r,55, 118, 0);
  Text_NPos (r,7, 80, v);
  Text_NPos (r,52, 43, v * 2);
  Text_NPos (r,95, 80, v * 3);

  Text_NPos (r,141, 60, 0);
  Text_NPos (r,141, 74, v);
  Text_NPos (r,141, 88, v * 2);
  Text_NPos (r,141, 102, v * 3);
  Text_NPos (r,141, 116, timer->songtakte);

  DrawRKnopf (50, 70, 22, drehpos);
}

void
ShowERegler ()
{
  DrawImage (DrehWnd->RPort, &reglerim, 24, 45);
  DrawRKnopf (50, 70, 22, drehpos);
}

void
CheckRMouse ()
{
  if (edit->mouseposx >= 25 && edit->mouseposy >= 45 && edit->mouseposx <= 90 && edit->mouseposy <= 110)
    {
      WORD dx, dy, r;
      ULONG v = timer->songtakte / 2, od = drehpos;
      double fak = 1 / drehfak;

      dx = edit->mouseposx - 62;
      dy = edit->mouseposy - 75;

      if (dy == 0)
        {
          if (dx > 0)
            drehpos = v + timer->songtakte / 4;
          else
            drehpos = timer->songtakte / 4;
        }
      else
        {
          r = atan2 ((double) dx, (double) -dy) * fak;  // 1/(2 * pi / 64)
          r -= v;
          if (r < 0)
            r += timer->songtakte;

          drehpos = r;
        }
      if (od != drehpos)
        {
          ShowERegler ();
          NewRPos ();
        }
    }
}

*/
