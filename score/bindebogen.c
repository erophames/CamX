#include "seq:include.h"
#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>         /* Data types.             */
#include <exec/errors.h>        /* Exec error messages.    */
#include <devices/printer.h>    /* Printer Device.         */
#include <exec/io.h>            /* Standard request block. */

#ifdef SCORE
void
DrawBogen(struct RastPort *r, WORD px0, WORD px2, WORD y, WORD midy)
{
  double a1, a2, c0;
  LONG px1 = px0 + (px2 - px0) / 2;

  a1 = (double) (y - midy) / (px0 - px1);
  c0 = (double) (midy - y) / (px1 - px2);
  a2 = (double) (a1 - c0) / (px0 - px2);

  {
    ULONG ym, x, x2 = px2;

    for (x = px0; x <= px1; x++)
      {
        ym = y
          + a1 * (x - px0)
          + a2 * (x - px0) * (x - px1);

        WritePixel (r, x, ym);
        WritePixel (r, x2--, ym);
      }
  }

}

#endif

