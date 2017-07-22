#include "include.h"
#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>         /* Data types.             */
#include <exec/errors.h>        /* Exec error messages.    */
#include <devices/printer.h>    /* Printer Device.         */
#include <exec/io.h>            /* Standard request block. */


#ifdef SCORE
#include "scoredef.h"

#define stopx 10
#define stopy 88

#define SCOREBR 12
#define startkey 65

ULONG scoretaktstart = 65535;

USHORT taktstriche[80];
USHORT taktpos[80][32];

struct SCOREBUFF *firstscorelist, *lastscorelist;
static USHORT lscorebuff;
UBYTE *scorebuffmem;

static ULONG starty;
static struct SCORESYSTEM *scoresystems[255];
UBYTE onscorecount;

struct SCORESYSTEM *actscore;

struct RastPort *srp;

USHORT singley[128];            /* Bei der Editierung nur eines Tracks */

/* ADD Festlegungen */
USHORT addtonart = 6;
USHORT addpunkt = 11;

USHORT scorelinien, maxscore;
UBYTE scoremodus;               /* 0 = Singlemodus, 1=Parallelmodus */

/* Pixelbreitendefinition */
USHORT punktbr = 10;
USHORT fahnebr = 10;
USHORT akttaktx;

struct SCOREGFX *fscore, *lscore;

/************ globale Scoreeinstellungen ********************/
struct SCOREGF far scoregf[80];
struct SCOREGF *startscoregf, *lastscoregf;
struct Gadget *ScoreSList, *scoreslider;

struct OVERLONG *firstoverlong,*lastoverlong;

ULONG scoreeditend, lastscoreused;
ULONG scoreh, sbtx, sbty, startscorebar, endscorebar, startscoreqrt, scorefactor, scorezoom = 14;
ULONG scorefaktor=23040;
ULONG startpixel = 8, vtopy, linienh = 88;

UBYTE scorebreite = 7, realscore, notequant = 14;
UBYTE toplinien = 2, downlinien = 2;

/*      Include-Dateien */
struct RastPort backrp;

void
ScrsAt (ULONG x, ULONG y, UBYTE * s)
{
  Move (&backrp, x, y);
  Text (&backrp, s, strlen (s));
}

void
ScrinAt (ULONG x, ULONG y, long s)
{
  Move (&backrp, x, y);
  Text (&backrp, nrstr, (stci_d (nrstr, s)));
}

#include "scoremem.c"
#include "scorefont.c"
#include "scoreformat.c"
#include "scorefilter.c"
#include "scoreprepare.c"
#include "scoretoolbox.c"
#include "scorevi.c"
#include "bindebogen.c"
#include "scoreshow.c"
#include "scoreimages.c"

#define UW UWORD
#define RD __chip n

ULONG sqt;

void
InitScoreGadgets ()
{
  struct Window *h = ScoreWnd;
  struct NewGadget *ngp = &ng;
  struct Gadget *g;

  if(ScoreSList)
  {
  EraseRect(srp,385,15,sbtx+20,stopy);
  }

  g = PrepareGadgetList (h, &ScoreSList, 0);

  ngp->ng_LeftEdge = 385;
  ngp->ng_TopEdge = 15;
  ngp->ng_Width = sbtx-385;
  ngp->ng_Height = 32;
  ngp->ng_Flags = PLACETEXT_IN;
  ngp->ng_GadgetText = "‰_Modus";
  ngp->ng_GadgetID = 5000;

  g = CreateGadget (BUTTON_KIND, g, &ng,
                    (GT_Underscore), '_', (TAG_DONE),
                    TAG_END);

  ngp->ng_LeftEdge = sbtx +1;
  ngp->ng_Width = 18;
  ngp->ng_Height = 16;
  ngp->ng_GadgetText = "—";
  ngp->ng_GadgetID = 56;

  g = CreateGadget (BUTTON_KIND, g, &ng,
                    (TAG_DONE),
                    TAG_END);

  ngp->ng_TopEdge = 31;
  ngp->ng_GadgetText = "’";
  ngp->ng_GadgetID = 55;

  g = CreateGadget (BUTTON_KIND, g, &ng,
                    (TAG_DONE),
                    TAG_END);

  ngp->ng_TopEdge = 48;
  ngp->ng_Height = h->Height - (h->BorderBottom + 48);
  ngp->ng_GadgetText = 0;
  ngp->ng_GadgetID = 5100;
  ngp->ng_Flags = 0;

  scoreslider = CreateGadget
    (SLIDER_KIND, g, &ng,
     GTSL_Min, 0,
     GTSL_Max, 15,
     GTSL_Level, 5,
     GA_Immediate, TRUE,
     GA_RelVerify, TRUE,
     PGA_Freedom, LORIENT_VERT, TAG_DONE);

  RenderGadget (h, scoreslider);

  AddGList (h, ScoreSList, -1, -1, NULL);
  if(MessageClass!=NEWSIZE)RefreshGList (ScoreSList, h, NULL, -1);
}

void
InitScore (UBYTE how)
{
  scorelinien = 0;              /* Wieviel Tracks sind selektiert ? */
  vtopy = 75 + (toplinien * 8);
  linienh = 8 * (5 + toplinien + downlinien);
  maxscore = (sbty - vtopy) / linienh;

#ifdef DBG
  printf ("Notensysteme: %d\n", scorelinien);
  printf ("Max. Notensysteme: %d\n", maxscore);
#endif

  scorefactor = GetFactor (quantlist[g_taktB], scorezoom);
  /*
  PrepareScore ();
  ShowScoreGrid ();
  */
}

void
InitAllScore ()
{
  ShowScore ();
}

void
InitOpenScore ()
{
  DrawImage(srp,&scoreheadim,6,15);
  InitScoreGadgets ();
}

void
InitScoreSize ()
{
  struct Window *h = ScoreWnd;

  sbty = h->Height - (h->BorderBottom + 1);
  sbtx = (h->Width - h->BorderRight) - 19;
  sbtx -= 2;
}

void InitOpenScoreEditor()
{
          InitScoreSize ();
          InitOpenScore ();

          FilterScoreEvents ();
          PrepareScore ();
          InitFirstScoreTakts ();       /* Vor ShowScore ! */

          OpenScoreFont ();
          InitScoreBitMaps ();
}

void
Score ()
{
  struct Window *h = ScoreWnd;

  if (!scorebuffmem)
    {
      if (!(scorebuffmem = AllocMem (1000 * sizeof (struct SCOREBUFF), MEMF_ANY | MEMF_CLEAR)))
          return;
    }

  if (!ScoreWnd)
    {
      InitBackRastPort();
      if(!backmap.Planes[0])return;

      if (!OpenScoreWindow ())
        {
          CheckLongestNote();

          h = ScoreWnd;
          srp = h->RPort;

          /*
          OpenTool1Window ();
          SetIDCMP (Tool1Wnd, 0, 0);
          Tool1Wnd->MinHeight = 90;
          Tool1Wnd->MinWidth = 350;

          Tool1Wnd->MaxHeight = 208;
          Tool1Wnd->MaxWidth = 350;
          InitScoreToolBox (1);
          */

          MouseSprite(ScoreWnd,MS_WAIT);
          InitOpenScoreEditor();
          FormatScore();
          ShowScore ();
          MouseSprite(ScoreWnd,MS_NORMAL);

      h = ScoreWnd;
      SetIDCMP (h, 2000, 768);

      h->MinHeight = 220;
      h->MinWidth = 480;
      /*
      InitScoreGadgets ();      /* Inuit-Gadgets */
      InitScore (2);
      */

        }
        else Question(252);

        return;
    }

  switch (MessageClass)
    {
    case IDCMP_GADGETUP:
      switch (GadgetID)
        {
        case 55:                /* Zoom ++ */
          if (zoomfaktor < 9)
            {
              zoomfaktor++;
              ShowScore ();
            }
          break;

        case 56:
          if (zoomfaktor)
            {
              zoomfaktor--;
              ShowScore ();
            }
          break;
        }

      break;

    case IDCMP_RAWKEY:
      switch (code)
        {
        case 180:     /* V */
          ViKey ();
          break;

        case 168:     /* L */
          LoadFont ();
          EditOldFont ();
          break;

        case 161:     /* S */
          SaveFont ();
          break;

        case 207:
                  if (zoomfaktor)
            zoomfaktor--;
          EditFont ();
          break;

        case 206:
          if (zoomfaktor < 9)
            zoomfaktor++;
          EditFont ();
          break;

        case 204:
          if (zoomwhat)
            zoomwhat--;
          EditFont ();
          break;

        case 205:
          if (zoomwhat < 157)
            zoomwhat++;
          EditFont ();
          break;

        case 146:        /* E */
          EditFont ();
          break;
        }
      break;

    case IDCMP_CHANGEWINDOW:
      ScoreLeft = h->LeftEdge;
      ScoreTop = h->TopEdge;
      ScoreWidth = h->Width;
      ScoreHeight = h->GZZHeight + h->BorderBottom;
      break;

    case IDCMP_NEWSIZE:
      InitScoreSize ();
      EraseRect(srp,sbtx-4,stopy,sbtx+20,sbty);
      InitScoreGadgets ();
      ClipBack();
      break;

    case IDCMP_CLOSEWINDOW:
      SCloseScoreWnd ();
      break;
    }
}

void
SCloseScoreWnd ()
{
  FreeBackRastPort ();

  if(Tool1Wnd)CloseTool1Window ();
  if (ViWnd)
    CloseViWindow ();

  FreeAllScoreMem();
  CloseScoreWindow ();
  ScoreSList = NULL;

  FreeScoreFontMem();      /*   Font wieder freigeben */

  actscore=0;

  if (scorebuffmem)
  {
    FreeMem (scorebuffmem, 1000 * sizeof (struct SCOREBUFF));
    scorebuffmem=0;
  }

}
#endif
