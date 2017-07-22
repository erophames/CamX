#include "seq:include.h"
#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>         /* Data types.             */
#include <exec/errors.h>        /* Exec error messages.    */
#include <devices/printer.h>    /* Printer Device.         */
#include <exec/io.h>            /* Standard request block. */

#ifdef SCORE
#define trp Tool1Wnd->RPort

void
ShowSToolEmpty ()
{
  DrawImage (trp, &sizeim, 293, 27 + (toolboxmax * 44));
}

void
ShowSTool (UBYTE nr, UBYTE what)
{
  ULONG x = 10 + nr * 28;
  ULONG y = 17;

  struct SCOREFONT *s = &sfont[what];

  SetAPen (trp, 0);
  SetOPen (trp, 1);
  RectFill (trp, x, y, x + 24, y + 28);
  SetAPen (trp, 2);
  Move (trp, x, y + 28);
  Draw (trp, x, y);
  Draw (trp, x + 24, y);
  SetAPen (trp, 1);
  BltBitMapRastPort (&s->bitmap, 0, 0, trp, x + 2, y + 2, s->breite, s->hoehe, 0xC0);
  if (what == 11)
    BltBitMapRastPort (&s->bitmap, 0, 0, trp, x + 2, y + 14, s->breite, s->hoehe, 0xC0);

  if (what == 123)
    {
      s = &sfont[119];
      BltBitMapRastPort (&s->bitmap, 0, 0, trp, x + 13, y + 10, s->breite, s->hoehe, 0xC0);
    }
}

void
ShowTB (UBYTE nr, UBYTE what)
{
  struct SCOREFONT *s = &sfont[what];
  ULONG y = 47;
  ULONG x = 12 + 28 * nr;

  if (what == 92)
    y = 57;
  BltBitMapRastPort (&s->bitmap, 0, 0, trp, x, y, s->breite, s->hoehe, 0xC0);
}

void
ShowNoteTri (UBYTE nr, UBYTE what)
{
  struct SCOREFONT *s = &sfont[what];
  ULONG x = 12 + 28 * nr, y = 82;

  if (what == 92)
    y = 92;
  BltBitMapRastPort (&s->bitmap, 0, 0, trp, x, y, s->breite, s->hoehe, 0xC0);
  s = &sfont[21];
  BltBitMapRastPort (&s->bitmap, 0, 0, trp, x + 15, 82, s->breite, s->hoehe, 224);
}

void
ShowNotePkt (UBYTE nr, UBYTE what)
{
  struct SCOREFONT *s = &sfont[what];
  ULONG x = 12 + 28 * nr, y = 117;

  if (what == 92)
    y = 127;
  BltBitMapRastPort (&s->bitmap, 0, 0, trp, x, y, s->breite, s->hoehe, 0xC0);
  s = &sfont[31];
  BltBitMapRastPort (&s->bitmap, 0, 0, trp, x + 15, 127, s->breite, s->hoehe, 224);
}

void
ShowSToolDir ()
{
  ULONG x = 10 + toolboxact * 28;

  SetAPen (trp, 0);
  SetOPen (trp, 0);
  RectFill (trp, x, 45, x + 24, 48);
  SetAPen (trp, 1);
  Move (trp, x, 46);
  Draw (trp, x, 17);
  Draw (trp, x + 24, 17);
  SetAPen (trp, 2);
  Draw (trp, x + 24, 46);
  SetAPen (trp, 1);

  switch (toolboxact)
    {
    case 0:
      ShowTB (0, 92);
      ShowTB (1, 138);
      ShowTB (2, 69);
      ShowTB (3, 135);
      ShowTB (4, 74);
      ShowTB (5, 70);
      ShowTB (6, 20);
      ShowTB (7, 58);

      if (toolboxmax > 1)
        {
          ShowNoteTri (0, 92);
          ShowNoteTri (1, 138);
          ShowNoteTri (2, 69);
          ShowNoteTri (3, 135);
          ShowNoteTri (4, 74);
          ShowNoteTri (5, 70);
          ShowNoteTri (6, 20);
          ShowNoteTri (7, 58);

          if (toolboxmax > 2)
            {
              ShowNotePkt (0, 92);
              ShowNotePkt (1, 138);
              ShowNotePkt (2, 69);
              ShowNotePkt (3, 135);
              ShowNotePkt (4, 74);
              ShowNotePkt (5, 70);
              ShowNotePkt (6, 20);
              ShowNotePkt (7, 58);
            }
          else
            ShowSToolEmpty ();

        }
      else
        ShowSToolEmpty ();

      break;

    case 1:
      ShowTB (0, 56);
      ShowTB (1, 27);
      break;

    case 2:
      ShowTB (0, 26);
      ShowTB (1, 36);
      ShowTB (2, 39);
      break;

    }
}

void
InitScoreToolBox (UBYTE all)
{
  ULONG y;

  if (all)
    {
      toolboxmax = (Tool1Wnd->GZZHeight - 33) / 44;

      SetAPen (trp, 1);
      SetAfPt (trp, &patt2[0], 1);
      RectFill (trp, Tool1Wnd->BorderLeft, Tool1Wnd->BorderTop, Tool1Wnd->Width - (Tool1Wnd->BorderRight + 1), Tool1Wnd->Height - (Tool1Wnd->BorderBottom + 1));
      SetAfPt (trp, 0, 0);
    }

  SetAPen (trp, 0);
  SetOPen (trp, 1);
  RectFill (trp, 10, 45, 314, y = 45 + 44 * toolboxmax);
  SetAPen (trp, 2);
  Move (trp, 10, y);
  Draw (trp, 10, 45);
  Draw (trp, 314, 45);

  SetAPen (trp, 1);
  ShowSTool (0, 123);
  ShowSTool (1, 56);
  ShowSTool (2, 36);
  ShowSTool (3, 75);
  ShowSTool (4, 102);
  ShowSTool (5, 121);
  ShowSTool (6, 35);
  ShowSTool (7, 81);
  ShowSTool (8, 11);
  ShowSTool (9, 126);
  ShowSTool (10, 130);

  ShowSToolDir ();
}

void
ScoreToolBox ()
{
  switch (MessageClass)
    {
      case IDCMP_CHANGEWINDOW:
      Tool1Left = Tool1Wnd->LeftEdge;
      Tool1Top = Tool1Wnd->TopEdge;
      Tool1Width = Tool1Wnd->Width;
      Tool1Height = Tool1Wnd->GZZHeight + Tool1Wnd->BorderBottom;
      break;

    case IDCMP_NEWSIZE:
    {
      ULONG max = (Tool1Wnd->GZZHeight - 33) / 44;

      if (max != toolboxmax)
        InitScoreToolBox (1);
     }
      break;

    case IDCMP_MOUSEBUTTONS:
      if (down)
        {
          ULONG x = 10,i;

          for (i = 0; i < 11; i++)
            {
              if (m_x >= x && m_x <= x + 24 && m_y >= 17 && m_y <= 45)
                {

                  if (i != toolboxact)
                    {
                      toolboxact = i;
                      InitScoreToolBox (0);

                    }
                  break;
                }

              x += 28;
            }
        }
      break;
    }
}

#endif
