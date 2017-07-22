ULONG
ScalePoint (ULONG y)
{
  ULONG yy = ((zooms[zoomfaktor] * y) * 1000) / 37000;
  return (yy);
}

struct BitScaleArgs s;

void                            /* Nur für Entwickler */
EditOldFont ()
{
  struct Screen *f = IntuitionBase->FirstScreen;
  struct Window *dpaint = 0;
  struct SCOREFONT *s = &sfont[zoomwhat];

  while (f)
    {
      if (!f->Title)
        {
          dpaint = f->FirstWindow;
          break;
        }

      f = f->NextScreen;
    }

  if (dpaint)
    {
      SetAPen (dpaint->RPort, 3);
      RectFill (dpaint->RPort, 10, 10, 400, 300);

      SetAPen (dpaint->RPort, 0);
      RectFill (dpaint->RPort, 20, 20, 400, 200);
      BltBitMapRastPort (&s->bitmap, 0, 0, dpaint->RPort, 20, 20, s->breite, s->hoehe, 0xC0);
    }
  else
    printf ("Dpaint nicht gefunden\n");
}

void                            /* Nur für Entwickler */
EditFont ()
{
  struct Screen *f = IntuitionBase->FirstScreen;
  struct Window *dpaint = 0;
  ULONG a = zoomwhat;

  while (f)
    {
      if (!f->Title)
        {
          dpaint = f->FirstWindow;
          break;
        }

      f = f->NextScreen;
    }

  s.bsa_SrcX =
    s.bsa_SrcY = 0;
  s.bsa_DestX = 0;
  s.bsa_DestY = 0;
  s.bsa_SrcWidth = scoreimages[a].br;
  s.bsa_SrcHeight = scoreimages[a].hh;

  s.bsa_XSrcFactor =
    s.bsa_YSrcFactor = 37;
  s.bsa_XDestFactor =
    s.bsa_YDestFactor = zooms[zoomfaktor];
  s.bsa_SrcBitMap = &sbits[a];
  s.bsa_DestBitMap = &tbits[a];
  s.bsa_Flags = 0;

  BitMapScale (&s);

  ClearWindow (ScoreWnd);
  ScrinAt (10, 25, zooms[zoomfaktor]);
  ScrinAt (10, 35, zoomwhat);

  BltBitMapRastPort (&tbits[a], 0, 0, srp, 10, 40, s.bsa_DestWidth, s.bsa_DestHeight, 0xC0);

  if (dpaint)
    {
      SetAPen (dpaint->RPort, 3);
      SetBPen (dpaint->RPort, 3);

      RectFill (dpaint->RPort, 10, 10, 400, 300);

      SetAPen (dpaint->RPort, 0);
      SetBPen (dpaint->RPort, 0);

      RectFill (dpaint->RPort, 20, 20, 20 + s.bsa_DestWidth, 20 + s.bsa_DestHeight);
      BltBitMapRastPort (&tbits[a], 0, 0, dpaint->RPort, 20, 20, s.bsa_DestWidth, s.bsa_DestHeight, 0xC0);
    }
  else
    printf ("Dpaint nicht gefunden\n");
}

void                            /* Nur für Entwickler */
LoadFont ()
{
  ULONG i;
  UBYTE stri[50];

  CopyMem ("prog:camfonts/", stri, 14);
  stci_d (&stri[14], zooms[zoomfaktor]);
  i = strlen (stri);
  CopyMem ("/", &stri[i], 2);
  stci_d (&stri[strlen (stri)], zoomwhat);
  CopyMem (stri, fname, strlen (stri) + 1);

  SOpen (MODE_OLDFILE);
  if (frr)
    {
      ULONG a;
      USHORT br, x, y;
      UBYTE *m = tbits[zoomwhat].Planes[0];

      InOut2 (&br);
      InOut2 (&x);
      InOut2 (&y);

      for (a = 0; a < y; a++)   /* Linie absichern */
        {
          InOut (m, br);
          m += tbits[zoomwhat].BytesPerRow;
        }

      ClearWindow (ScoreWnd);
      BltBitMapRastPort (&tbits[zoomwhat], 0, 0, srp, 10, 40, x, y, 0xC0);

      SClose ();
    }
}

void                            /* Nur für Entwickler */
SaveFont ()
{
  struct Screen *f = IntuitionBase->FirstScreen;
  struct Window *dpaint = 0;
  UBYTE i;
  UBYTE stri[50];

  while (f)
    {
      if (!f->Title)
        {
          dpaint = f->FirstWindow;
          break;
        }

      f = f->NextScreen;
    }

  CopyMem ("prog:camfonts/", stri, 14);
  stci_d (&stri[14], zooms[zoomfaktor]);
  i = strlen (stri);
  CopyMem ("/", &stri[i], 2);
  stci_d (&stri[strlen (stri)], zoomwhat);
  printf ("File %s\n", stri);

  if (dpaint)
    {
      CopyMem (stri, fname, strlen (stri) + 1);
      SOpen (MODE_NEWFILE);
      if (frr)
        {
          ULONG a,b,y;
          USHORT x,br=0,ho=0;
          UBYTE *m = tbits[zoomwhat].Planes[0];

          ClearWindow (ScoreWnd);
          ClipBlit (dpaint->RPort, 20, 20, srp, 20, 20, s.bsa_DestWidth + 1, s.bsa_DestHeight + 1, 0xC0);

          y = 20;
          for (a = 0; a < s.bsa_DestHeight + 1; a++)
            {
              x = 20;
              for (b = 0; b < s.bsa_DestWidth + 1; b++)
                {
                  if (ReadPixel (srp, x, y))
                    {
                      if ((!br) || (x > br))
                        br = x;
                      if ((!ho) || (y > ho))
                        ho = y;
                    }
                  x++;
                }
              y++;
            }

          if (br && ho)
            {
              br -= 19;
              ho -= 19;

              printf ("Effektive Grösse %d X %d\n", br, ho);

              x = br / 8;
              if (x * 8 < br)
                x++;

              BltBitMap (srp->BitMap, 20, 20, &tbits[zoomwhat], 0, 0, br, ho, 0xC0, 1, 0);
              BltBitMapRastPort (&tbits[zoomwhat], 0, 0, srp, 200, 20, br, ho, 0xC0);

              InOut2 (&x);
              InOut2 (&br);
              InOut2 (&ho);

              for (a = 0; a < ho; a++)  /* Linie absichern */
                {
                  InOut (m, x);
                  m += tbits[zoomwhat].BytesPerRow;
                }
            }
          SClose ();
        }
    }
}

void
FreeScoreFontMem ()
{
  ULONG i;

  for (i = 0; i < SCOREIMAGES; i++)
    {
      if (sfont[i].size)
        {
          FreeMem (sfont[i].bitmap.Planes[0], sfont[i].size);
          sfont[i].size = 0;
        }
    }
}

void
OpenScoreFont ()
{
  UBYTE stri[50];
  UBYTE *plane;
  ULONG i;
  USHORT br, x, y;
  ULONG fontsize = 0;

  FreeScoreFontMem ();

  for (i = 0; i < SCOREIMAGES; i++)
    {
      CopyMem ("prog:camfonts/", stri, 14);
      stci_d (&stri[14], 6);    /* 8 = zooms[zoomfaktor] */
      CopyMem ("/", &stri[strlen (stri)], 2);
      stci_d (&stri[strlen (stri)], i);
      CopyMem (stri, fname, strlen (stri) + 1);
      byebye = 1;
      SOpen (MODE_OLDFILE);
      if (frr)
        {
          ULONG oy;

          InOut2 (&br);
          InOut2 (&x);
          InOut2 (&y);

          sfont[i].breite = x;
          sfont[i].hoehe = y;
          InitBitMap (&sfont[i].bitmap, 1, x, y);
          plane = sfont[i].bitmap.Planes[0] = AllocMem (sfont[i].size = sfont[i].bitmap.BytesPerRow * y, MEMF_CHIP | MEMF_CLEAR);
          fontsize += sfont[i].size;
          oy = y;
          while (y)
            {
              InOut (plane, br);
              plane += sfont[i].bitmap.BytesPerRow;
              y--;
            }
          SClose ();
        }
      else
        {
          printf ("Font nicht vorhanden : ");
          printf ("%s\n", stri);
        }
    }

  printf ("Speicherbedarf :%d\n", fontsize);
  byebye = 0;
}
