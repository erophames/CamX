#include "seq:include.h"
#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>         /* Data types.             */
#include <exec/errors.h>        /* Exec error messages.    */
#include <devices/printer.h>    /* Printer Device.         */
#include <exec/io.h>            /* Standard request block. */

#ifdef SCORE
#define fline 50
#define vioy 44
#define bassy 51
#define soprany 63

static UBYTE vikreuz[]=
{
  1, 4, 0, 3, 6, 2, 5
};

static UBYTE vibass[]=
{
  5, 2, 6, 3, 7, 4, 8
};

static UBYTE basskreuz[]=
{
  3, 6, 2, 5, 8, 4, 7
};

static UBYTE bassbass[]=
{
  7, 4, 8, 5, 9, 6, 10
};

static UBYTE altkreuz[]=
{
  2, 5, 1, 4, 7, 3, 6
};

static UBYTE altbass[]=
{
  6, 3, 7, 4, 8, 5, 9
};

static UBYTE tenorkreuz[]=
{
  7, 3, 6, 2, 5, 1, 4
};

static UBYTE tenorbass[]=
{
  4, 1, 5, 2, 6, 3, 7
};

UBYTE far *schluesselname[]=
{
  "C", "A",
  "G", "E",
  "D", "B",
  "A", "F#",
  "E", "C#",
  "B", "G#",
  "F#", "D#",
  "C#", "A#",

  "Cb", "Ab",
  "Gb", "Eb",
  "Db", "Bb",
  "Ab", "F",
  "Eb", "C",
  "Bb", "G",
  "F", "D",
  "C", "A"
};

void
ViBlit (USHORT x, USHORT y, UBYTE what)
{
  struct SCOREFONT *s = &sfont[what];
  BltBitMapRastPort (&s->bitmap, 0, 0, ViWnd->RPort, x, y, s->breite, s->hoehe, 224);
}

void
ShowVi ()
{
  struct RastPort *r = ViWnd->RPort;
  USHORT y;
  UBYTE *kr, *be, image, i;

  EraseRect (r, 10, 28, 140, 107);

  SetAPen (r, 1);

  Move (r, 10, y = fline);
  Draw (r, 10, fline+24);
  Move (r, 12, fline);
  Draw (r, 12, fline+24);

  for (i = 0; i < 5; i++)
    {
      Move (r, 11, y);
      Draw (r, 140, y);

      y += 6;
    }

  switch (actscore->notenschluessel)
    {
    case 0:                     /* Violin */
      y = vioy;
      image = 26;
      kr = vikreuz;
      be = vibass;
      break;

    case 1:                     /* Violin 8va */
      y = vioy;
      image = 26;
      ViBlit (28, 36, 15);
      kr = vikreuz;
      be = vibass;
      break;

    case 2:                     /* Violin 15va */
      y = vioy;
      image = 26;
      ViBlit (28, 36, 8);
      ViBlit (35, 36, 12);
      kr = vikreuz;
      be = vibass;
      break;

    case 3:                     /* Tenor */
      y = vioy;
      image = 26;
      ViBlit (28, 80, 15);
      kr = vikreuz;
      be = vibass;
      break;

    case 4:                     /* Bass */
      y = bassy;
      image = 36;

      kr = basskreuz;
      be = bassbass;
      break;

    case 5:                     /* Bass 8vb */
      y = bassy;
      image = 36;
      ViBlit (28, 80, 15);
      kr = basskreuz;
      be = bassbass;
      break;

    case 6:                     /* Bass 15vb */
      y = bassy;
      image = 36;
      ViBlit (28, 80, 8);
      ViBlit (35, 80, 12);
      kr = basskreuz;
      be = bassbass;
      break;

    case 7:                     /* Sopran */
      y = soprany;
      image = 39;
      break;

    case 8:                     /* Alt */
      y = bassy;
      image = 39;
      kr = altkreuz;
      be = altbass;
      break;

    case 9:                     /* Tenor/alt */
      y = vioy;
      image = 39;
      kr = tenorkreuz;
      be = tenorbass;
      break;

    case 10:
      break;
    }

  /* Schluessel */
  ViBlit (18, y, image);

  /*
  6  #,
  5  ##,
  4  ###,
  3  ####,
  2  #####,
  1  ######,
  0  #######,
  7  -
  8  b
  9  bb
  10 bbb
  11 bbbb
  12 bbbbb
  13 bbbbbb
  14 bbbbbbb
  */

  {
    USHORT x = 40, y;

    UBYTE *rf;
    switch (actscore->vorzeichen)
      {
      case 0:
        i = 7;
        rf = kr;
        break;

      case 1:
        i = 6;
        rf = kr;
        break;

      case 2:
        i = 5;
        rf = kr;
        break;

      case 3:
        i = 4;
        rf = kr;
        break;

      case 4:
        i = 3;
        rf = kr;
        break;

      case 5:
        i = 2;
        rf = kr;
        break;

      case 6:
        i = 1;
        rf = kr;
        break;

      case 7:
        i = 0;
        break;

      case 8:
        i = 1;
        rf = be;
        break;

      case 9:
        i = 2;
        rf = be;
        break;

      case 10:
        i = 3;
        rf = be;
        break;

      case 11:
        i = 4;
        rf = be;
        break;

      case 12:
        i = 5;
        rf = be;
        break;

      case 13:
        i = 6;
        rf = be;
        break;

      case 14:
        i = 7;
        rf = be;
        break;
      }

    while (i)
      {
        if (actscore->vorzeichen > 7)         /* Bass */
          {
            y = 35 + *rf * 3;

            ViBlit (x, y, 131);
          }
        else
          {
            y = 38 + *rf * 3;

            ViBlit (x, y, 23);  /* Kreuz */
          }

        rf++;
        x += 8;
        i--;
      }
  }
}

void
InsertVICode ()
{
  if (actscore->vorzeichen != code && code < 255)
    {
      actscore->vorzeichen = code;
      ShowVi ();
    }
}

void ShowSystemWhat()
{
          DrawGroup (ViWnd, 5, 16, 166, 133, "System");
}

void
ViKey ()
{
  if (!ViWnd)
    {
      if (!OpenViWindow ())
        {
          SetIDCMP (ViWnd, 0, 0);
          ShowSystemWhat();
          ShowVi ();

          srl[0].ti_Data = 16;
          srl[1].ti_Data = 2;
          srl[2].ti_Data = 14 - actscore->vorzeichen;

          GT_SetGadgetAttrsA (ViGadgets[GD_visld], ViWnd, 0, (struct TagItem *) & srl);

          box0 = actscore->track->score->clip;
          GT_SetGadgetAttrsA (ViGadgets[GD_splitscore], ViWnd, 0, (struct TagItem *) & box);
        }
      return;
    }

  switch (MessageClass)
    {
    case IDCMP_MOUSEMOVE:
      if (slideractive)
        InsertVICode ();
      break;

    case IDCMP_GADGETDOWN:
      if (GadgetID == GD_visld)
        {
          InsertVICode ();
          slideractive = 1;
        }
      break;

    case IDCMP_GADGETUP:
      if (slideractive)
        slideractive = 0;
      else
        switch (GadgetID)
          {
          case GD_splitscore:
          actscore->track->score->clip^=1;
          ShowSystemWhat();
          ShowVi();
          break;

          case GD_vischluessel:
            actscore->notenschluessel=code;
            ShowVi ();
            break;
          }
      break;

    case IDCMP_CLOSEWINDOW:
      CloseViWindow ();
      break;
    }
}

#endif

