#include "include.h"
#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>         /* Data types.             */
#include <exec/errors.h>        /* Exec error messages.    */
#include <devices/printer.h>    /* Printer Device.         */
#include <exec/io.h>            /* Standard request block. */

#include "scoregui.c"
#include "scoredef.h"

struct Library *UtilityBase, *GadToolsBase, *DiskfontBase, *AslBase, *TimerLib;

struct SCOREFONT
{
  struct BitMap bitmap;
  USHORT breite, hoehe, size;
};

struct SCOREFONT far sfont[150];
USHORT editfont;
USHORT addx = 6;
USHORT addy = 5;

UBYTE editmodus;

/*
USHORT oldix,oldiy,actix,actiy;

void SActFont()
{
if(oldix)ClipBlit(SoWnd->RPort,oldix,oldiy,SoWnd->RPort,oldix,oldiy,27,32,0x50);

ClipBlit(SoWnd->RPort,oldix,oldiy,SoWnd->RPort,oldix,oldiy,27,32,0x50);
}
*/

void
CloseLibs ()
{
  if (DiskfontBase)
    CloseLibrary ((struct Library *) DiskfontBase);

  if (UtilityBase)
    CloseLibrary ((struct Library *) UtilityBase);

  if (AslBase)
    CloseLibrary (AslBase);

  if (GadToolsBase)
    CloseLibrary ((struct Library *) GadToolsBase);

  if (GfxBase)
    CloseLibrary ((struct Library *) GfxBase);
}

UBYTE fontnr = 0;
UBYTE zoom = 7;

UBYTE zooms[]=
{20, 18, 16, 14, 12, 10, 8, 6, 4, 3};

void
OpenSFont (USHORT zoom, USHORT i)
{
  USHORT br, x, y, oy;
  BPTR fr;
  UBYTE *plane;
  UBYTE stri[50];
  ULONG fontsize = 0;

  CopyMem ("prog:camfonts/", stri, 14);
  stci_d (&stri[14], zooms[zoom]);      /* 8 = zooms[zoomfaktor] */
  CopyMem ("/", &stri[strlen (stri)], 2);
  stci_d (&stri[strlen (stri)], i);

  fr = Open (stri, MODE_OLDFILE);
  if (fr)
    {
      Read (fr, &br, 2);
      Read (fr, &x, 2);
      Read (fr, &y, 2);

      sfont[i].breite = x;
      sfont[i].hoehe = y;
      InitBitMap (&sfont[i].bitmap, 1, x, y);
      plane = sfont[i].bitmap.Planes[0] = AllocMem (sfont[i].size = sfont[i].bitmap.BytesPerRow * y, MEMF_CHIP | MEMF_CLEAR);

      oy = y;
      while (y)
        {
          Read (fr, plane, br);
          plane += sfont[i].bitmap.BytesPerRow;
          y--;
        }

      Close (fr);
    }
  else
    {
      printf ("Font nicht gefunden Zoom: %d Nr %d\n", zooms[zoom], i);
      SetAPen (SoWnd->RPort, 3);
      RectFill (SoWnd->RPort, 10, 20, 479, 545);
    }
}

void
FreeSFonts ()
{
  USHORT i;

  for (i = 0; i < SCOREIMAGES; i++)
    {
      if (sfont[i].size)
        {
          FreeMem (sfont[i].bitmap.Planes[0], sfont[i].size);
          sfont[i].size = 0;
        }
    }
}

struct BitScaleArgs sc;

void
ScaleSFont (UBYTE zoom, UBYTE a)
{
  sc.bsa_SrcX =
  sc.bsa_SrcY = 0;
  sc.bsa_DestX = 0;
  sc.bsa_DestY = 0;
  sc.bsa_SrcWidth = scores[a].br;
  sc.bsa_SrcHeight = scores[a].hh;

  sc.bsa_XSrcFactor =
    sc.bsa_YSrcFactor = 37;
  sc.bsa_XDestFactor =
    sc.bsa_YDestFactor = zooms[zoom];

  sc.bsa_SrcBitMap = &sbits[a];
  sc.bsa_DestBitMap = &tbits[a];
  sc.bsa_Flags = 0;

  BitMapScale (&sc);
  printf("Scale Zoom %d\n >Br %d >Hh %d \n",zoom,sc.bsa_DestWidth, sc.bsa_DestHeight);
  /*
  BltBitMapRastPort (&tbits[a], 0, 0, srp, 10, 40, sc.bsa_DestWidth, sc.bsa_DestHeight, 0xC0);
  */
}

void
EditFont ()
{
      struct SCOREFONT *s = &sfont[editfont];
      USHORT br = s->breite, h = s->hoehe;
      UBYTE nrstr[10];

      EraseRect (SoWnd->RPort, 10, 20, 479, 545);
      EraseRect (SoWnd->RPort, 120, 545, 265, 566);

      if(editmodus)
      {
      ScaleSFont(zoom,editfont);
      BltBitMapRastPort (&tbits[editfont], 0, 0, SoWnd->RPort, 10, 20, br=sc.bsa_DestWidth, h=sc.bsa_DestHeight, 0xC0);
      }
      else
      BltBitMapRastPort (&s->bitmap, 0, 0, SoWnd->RPort, 10, 20, br, h, 0xC0);

      /* Gitter */
      {
        USHORT xt = 10 + br + 5, yt = 20, i, a, x = 10, y = 20;

        SetOPen (SoWnd->RPort, 1);

        for (a = 0; a < h; a++)
          {
            for (i = 0; i < br; i++)
              {
                if (ReadPixel (SoWnd->RPort, x++, y))
                  SetAPen (SoWnd->RPort, 3);
                else
                  SetAPen (SoWnd->RPort, 0);

                RectFill (SoWnd->RPort, xt, yt, xt + addx, yt + addy);

                xt += addx;
              }

            xt = 10 + br + 5;
            yt += addy;
            x = 10;
            y++;
          }

      }

      SetAPen (SoWnd->RPort, 3);

      Move (SoWnd->RPort, 120, 560);
      Text (SoWnd->RPort, nrstr, (stci_d (nrstr, editfont)));

      SetAPen (SoWnd->RPort, 1);

      Move (SoWnd->RPort, 140, 560);
      Text (SoWnd->RPort, nrstr, (stci_d (nrstr, br)));
      Move (SoWnd->RPort, 160, 560);
      Text (SoWnd->RPort, nrstr, (stci_d (nrstr, h)));
}

void
ShowSAll ()
{
  USHORT x = 480, y = 20;
  UBYTE ax, ay, nr = 0;

  SetAPen (SoWnd->RPort, 0);
  SetOPen (SoWnd->RPort, 1);

  for (ay = 0; ay < 16; ay++)
    {
      for (ax = 0; ax < 10; ax++)
        {
          RectFill (SoWnd->RPort, x, y, x + 27, y + 32);

          if (nr <= SCOREIMAGES)
            {
              OpenSFont (zoom, nr);
              if (sfont[nr].bitmap.Planes[0])
                {
                  USHORT br = sfont[nr].breite;
                  USHORT h = sfont[nr].hoehe;

                  if (br > 26)
                    br = 26;
                  if (h > 31)
                    h = 31;

                  BltBitMapRastPort (&sfont[nr].bitmap, 0, 0, SoWnd->RPort, x + 1, y + 1, br, h, 0xC0);
                }
              else
                {
                  SetAPen (SoWnd->RPort, 3);
                  RectFill (SoWnd->RPort, x, y, x + 27, y + 32);
                  SetAPen (SoWnd->RPort, 0);
                }

              nr++;
            }

          x += 27;
        }

      x = 480;
      y += 32;
    }
}

void
main ()
{
  if (!(GfxBase = (struct GfxBase *) OpenLibrary ("graphics.library", 0)))
    {
      return;
    }
  if (!(GadToolsBase = OpenLibrary ("gadtools.library", 0)))
    {
      return;
    }
  if (!(DiskfontBase = OpenLibrary ("diskfont.library", 0)))
    {
      return;
    }
  if (!(UtilityBase = OpenLibrary ("utility.library", 0)))
    {
      return;
    }
  if (!(AslBase = OpenLibrary ("asl.library", 0)))
    {
      return;
    }

  InitScoreBitMaps();
  SetupScreen ();
  OpenSoWindow ();

  if (SoWnd)
    {
      UBYTE ok = 1;
      struct IntuiMessage *Message;
      ULONG MessageClass;

      ShowSAll ();

      OpenSFont (zoom, editfont);
      EditFont ();

      while (ok)
        {
          WaitPort (SoWnd->UserPort);

          if (Message = (struct IntuiMessage *) GT_GetIMsg (SoWnd->UserPort))
            {
              MessageClass = Message->Class;

              GT_ReplyIMsg (Message);
            }

          switch (MessageClass)
            {
            case IDCMP_MOUSEBUTTONS:
              if (Message->Code & IECODE_UP_PREFIX)
                {
                  if (SoWnd->MouseX >= 480 && SoWnd->MouseY >= 20)
                    {
                      USHORT nr = (SoWnd->MouseX - 480) / 27;
                      nr += 10 * ((SoWnd->MouseY - 20) / 32);

                      printf ("FontNr: %d\n", nr);
                      if (nr >= 0 && nr < SCOREIMAGES)
                        {
                          editfont = nr;

                          OpenSFont (zoom, editfont = nr);
                          EditFont ();
                        }
                    }
                }
              break;

            case IDCMP_CLOSEWINDOW:
              ok = 0;
              break;

            case IDCMP_GADGETUP:
              {
                struct Gadget *h;
                h = (struct Gadget *) Message->IAddress;
                switch (h->GadgetID)
                  {
                  case GD_zoomi:
                    switch (Message->Code)
                      {
                      case 0:
                        addx = 4;
                        addy = 3;
                        break;

                      case 1:
                        addx = 6;
                        addy = 5;
                        break;

                      case 2:
                        addx = 8;
                        addy = 6;
                        break;

                      case 3:
                        addx = 10;
                        addy = 8;
                        break;
                      }
                    break;

                  case GD_sub:
                    if (editfont)
                      OpenSFont (zoom, --editfont);
                    break;

                  case GD_add:
                    OpenSFont (zoom, ++editfont);
                    break;

                  case GD_zorro:
                    FreeSFonts ();
                    OpenSFont (zoom=Message->Code, editfont);
                    break;

                  case GD_edscale:
                  editmodus=Message->Code;
                  break;

                  case GD_test:
                    {
                      USHORT i;

                      for (i = 0; i < SCOREIMAGES; i++)
                        {
                          OpenSFont (zoom, i);
                          EditFont ();
                          Delay (20);
                        }

                    }
                    break;
                  }

                EditFont ();
              }
              break;
            }
        }
    }

  FreeZoomBitMaps();
  FreeSFonts ();
  CloseSoWindow ();
  CloseDownScreen ();
  CloseLibs ();
}
