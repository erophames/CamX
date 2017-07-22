#include "seq:include.h"
#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>         /* Data types.             */
#include <exec/errors.h>        /* Exec error messages.    */
#include <devices/printer.h>    /* Printer Device.         */
#include <exec/io.h>            /* Standard request block. */

#ifdef SCORE
/*** Routinen zur Verwaltung des Backrastport ***/
struct BitMap backmap;

USHORT leftedge = 1023;
UBYTE scorepage = 1;

void
InitBackRastPort ()
{
  struct RastPort *r = &backrp;

  InitBitMap (&backmap, 1, 1024, 768);
  backmap.Planes[0] = AllocMem (98304, MEMF_CHIP);

  InitRastPort (r);
  r->BitMap = &backmap;

  SetAPen (r, 1);
  SetBPen (r, 0);
  SetWrMsk (r, 1);
}

void
FreeBackRastPort ()
{
  if (backmap.Planes[0])
    {
      FreeMem (backmap.Planes[0], 98304);
      backmap.Planes[0] = 0;
    }
}

ULONG
ScoreBlit (ULONG x, ULONG y, ULONG what)
{
  struct SCOREFONT *s = &sfont[what];
  ULONG br = s->breite, h = s->hoehe;

  if ((x + br) >= 1023)         /* Rechts raus */
    br = 1023 - x;

  if ((y + h) >= 767)           /* Unten raus */
    h = 767 - h;

  BltBitMap (&s->bitmap, 0, 0, &backmap, x, y, br, h, 224, 1, 0);
  return (br);
}

ULONG
ScoreBlitInverse (ULONG x, ULONG y, ULONG what)
{
  struct SCOREFONT *s = &sfont[what];
  ULONG br = s->breite, h = s->hoehe;

  if ((x + br) >= 1023)         /* Rechts raus */
    br = 1023 - x;

  if ((y + h) >= 767)           /* Unten raus */
    h = 767 - h;

  BltBitMap (&s->bitmap, 0, 0, &backmap, x, y, br, h, 177, 1, 0);
  return (br);
}

void
DrawPause (struct SCORESYSTEM *st, struct PAUSE *p, ULONG takt) /* y = 1 Systemlinie */
{
  ULONG br, y = st->starty;
  ULONG x = taktpos[takt][p->einheit];
  ULONG inr = p->imagenr;
  ULONG zf = zooms[zoomfaktor];

  switch (inr)
    {
    case 67:
    case 116:
    case 76:
    case 108:                   /* Doppelganze,Ganze,Achtel,16tel */
      y += zf;
      break;

    case 117:                   /* Halbe */
      inr = 116;                /* Wichtig ! */
      y += zf + (zf / 2);
      break;

    case 144:
    case 99:                    /* Viertel,32tel,64tel */
      y += zf / 2;
      break;
    }

  {
    struct SCOREFONT *s = &sfont[inr];
    ULONG h = s->hoehe;

    br = s->breite;
    if ((x + br) >= 1023)       /* Rechts raus */
      br = 1023 - x;

    if ((y + h) >= 767)         /* Unten raus */
      h = 767 - h;

    BltBitMap (&s->bitmap, 0, 0, &backmap, x, y, br, h, 224, 1, 0);

    if (p->punkt & 1)           /* Punktierte Pause ? */
      {
        UBYTE im;

        y = st->starty;

        switch (p->imagenr)     /* NICHT durch inr ersetzen !!! */
          {
          case 67:              /* Doppelganze */
            im = 116;
            y += zf;
            break;

          case 116:             /* Ganze */
            im = 116;
            y += zf + (zf / 2);
            break;

          case 117:             /* Halbe */
            im = 119;
            break;

          case 119:             /* Viertel */
            im = 108;
            y += zf;
            break;

          case 108:             /* Achtel */
            im = 76;
            y += zf;
            break;

          case 76:              /* Sechstel */
            im = 144;
            y += zf / 2;
            break;

          case 144:             /* 32tel */
            im = 99;
            y += zf / 2;
            break;
          }

        if (im)
          {
            s = &sfont[im];
            h = s->hoehe;

            if ((x + br + s->breite + zf) < 1023)       /* Rechts raus */
              {
                ULONG nx = x + br + zf;

                BltBitMap (&s->bitmap, 0, 0, &backmap, nx, y + zooms[zoomfaktor] / 2, s->breite, s->hoehe, 224, 1, 0);
              }
          }
      }
  }
}

static ULONG akkordkeyposy[16];

void
DrawAkkordNote (struct SCORESYSTEM *sys, struct SCOREBUFF *akkord, UBYTE takt)
{
  struct SCOREFONT *s;
  struct SCOREBUFF *note = akkord;
  ULONG zf = zooms[zoomfaktor], br, h;
  ULONG x, y, htopy = 0, hbtny;
  UBYTE nr = 0;

  printf ("Zeichne einen AKKORD mit %d Noten\n", akkord->akkordanzahl);

  do
    {
      printf ("*AKKORD*\n");

      if (note->len >= 1536)
        s = &sfont[1];          /* Halbe Kopf */
      else
        s = &sfont[50];         /* Viertel Kopf */

      x = taktpos[takt][note->einheit];
      y = sys->key[note->on->key] & KPOS;

      br = s->breite;
      h = s->hoehe;

      akkordkeyposy[nr] = y;

      if (nr)
        {                       /* Links-Rechts Check */
          ULONG kpy;
          UBYTE i;

          for (i = 0; i < nr; i++)
            {
              kpy = akkordkeyposy[i];

              if ((y <= kpy && y + br > kpy) ||
                  (y > kpy && y < kpy + br)
                )
                {
                  printf ("Akkordnote gefliipt \n");
                  akkordkeyposy[nr] = 0;
                  x -= br;
                  break;
                }
            }
        }

      nr++;
      note->x = x;
      note->y = y;

      if (!htopy)
        htopy = hbtny = y;
      else
        {
          if (htopy > y)
            htopy = y;
          if (hbtny < y)
            hbtny = y;
        }

      if (note->overbuff)
        {
          ULONG fx = note->overbuff->x + zf / 2;
          ULONG tx = x + zf / 2;
          ULONG ty = y + zf + zf / 4;
          printf ("Ziehe Verbindungsbogen X1 %d X2 %d Y%d\n", note->overbuff->x, x, y);

          DrawBogen (&backrp, fx, tx, ty, ty + zf);
        }


      if (note->typ & KREUZ)    /* # oder b */
        {
          struct SCOREFONT *k = &sfont[23];
          ULONG ky = y;

          if (note->typ & UP)
            {
              ky = y + h;
              ky -= zf;
            }

          ky -= zf;
          BltBitMap (&k->bitmap, 0, 0, &backmap, x - (k->breite + zf / 2), ky, k->breite, k->hoehe, 224, 1, 0);
        }
      else if (note->typ & AUFLOESUNG)  /* Auflösungszeichen */
        {
          struct SCOREFONT *k = &sfont[143];
          ULONG ky = y;
          printf ("Zeige Auflösung \n");

          if (note->typ & UP)
            {
              ky = y + h;
              ky -= zf;
            }

          ky -= zf;
          BltBitMap (&k->bitmap, 0, 0, &backmap, x - (k->breite + zf / 2), ky, k->breite, k->hoehe, 224, 1, 0);
        }

      {
        ULONG yh = sys->key[note->on->key] & KPOS;

        if (yh < (sys->starty - zf))    /* Hilfslinien oben */
          {
            ULONG x1, x2, dy = sys->starty;
            x2 = x1 = x - zf / 2;
            x2 += 2 * zf;

            do
              {
                dy -= zf;
                Move (&backrp, x1, dy);
                Draw (&backrp, x2, dy);
              }
            while ((dy - zf) > yh);
          }
        else if (yh > sys->bty) /* Hilfslinien unten */
          {
            ULONG x1, x2, dy = sys->bty;

            x2 = x1 = x - zf / 2;
            x2 += 2 * zf;

            do
              {
                dy += zf;
                Move (&backrp, x1, dy);
                Draw (&backrp, x2, dy);
              }
            while (dy < yh);
          }
      }

      if ((x + br) >= 1023)     /* Rechts raus */
        br = 1023 - x;

      if ((y + h) >= 767)       /* Unten raus */
        h = 767 - h;

      BltBitMap (&s->bitmap, 0, 0, &backmap, x, y, br, h, 224, 1, 0);

      if (note->typ & PUNKT)    /* Note mit Punkt ? */
        {
          ULONG yp = sys->key[note->on->key] & KPOS;
          s = &sfont[31];
          BltBitMap (&s->bitmap, 0, 0, &backmap, x + br, yp, s->breite, s->hoehe, 224, 1, 0);

          br += s->breite;
        }

      /*
      if (note->balken)             /* Diese Note befindet sich in einer Balkengruppe */
      {
        struct BALKENGRUPPE *bg = note->balken;
        struct BALKENNOTENPOS *bpos = bg->notenpos;

        if (note == bg->firstnote)      /* Balkenpositionen erfassen */
          bg->noteposact = 0;

        if (bg->noteposact)
          bpos += bg->noteposact;
        bg->noteposact++;

        bpos->note = note;

        if (note->typ & UP)     /* Note zeigt nach oben */
          {
            ULONG tox = (x + br) - 1;

            bpos->x = tox;      /* Halsposition */
            bpos->y = y;

            if (!bg->fromx)
              bg->fromx = tox;
            else if ((!bg->tox) || (tox > bg->tox))
              bg->tox = tox;

            if ((!bg->fromy) ||
                ((bg->typ & WAAGERECHT) && bg->fromy > y)
              )
              {
                bg->fromy = y;
              }
            else
              {
                if ((!bg->toy) || (y > bg->toy))
                  bg->toy = y;
              }
          }
        else
          /* Note zeigt nach unten */
          {
            ULONG toy = y + h;

            bpos->x = x;        /* Halsposition */
            bpos->y = y;

            if (!bg->fromx)
              bg->fromx = x;
            else if ((!bg->tox) || (x > bg->tox))
              bg->tox = x;

            if ((!bg->fromy) ||
                ((bg->typ & WAAGERECHT) && bg->fromy < toy)
              )
              {
                bg->fromy = toy;
              }
            else if ((!bg->toy) || (toy > bg->toy))
              bg->toy = toy;
          }
      }
      */

        if (note == akkord)
        note = note->firstunderakkord;
      else
        note = note->next;
    }
  while (note);

  /* Hals zeichnen */
  {
    s = 0;

    printf ("Akkordhals Ytop %d YBtn %d\n", htopy, hbtny);
    if (akkord->typ & UP)       /* Hals nach oben */
      {
        x += sfont[50].breite;

        Move (&backrp, x, hbtny);
        Draw (&backrp, x, htopy = htopy - (zf * 3));

        switch (akkord->len)
          {
          case 384:
          case 576:             /* 1/8,· */
            s = &sfont[0];
            break;

          case 192:
          case 288:
            s = &sfont[140];
            break;
          }

        if (s)
          {
            BltBitMap (&s->bitmap, 0, 0, &backmap, x, htopy, s->breite, s->hoehe, 224, 1, 0);
          }
      }
    else
      /* Hals nach unten */
      {
        Move (&backrp, x, htopy + (zf / 2));
        Draw (&backrp, x, hbtny + (zf * 3));

        switch (akkord->len)
          {
          case 384:
          case 576:             /* 1/8,· */
            s = &sfont[47];
            break;

          case 192:
          case 288:
            s = &sfont[48];
            break;
          }

        if (s)
          {
            hbtny += zf;
            BltBitMap (&s->bitmap, 0, 0, &backmap, x, hbtny, s->breite, s->hoehe, 224, 1, 0);
          }
      }
  }
}

void
DrawNote (struct SCORESYSTEM *sys, struct SCOREBUFF *note, UBYTE takt)
{
  struct SCOREFONT *s = &sfont[note->imagenr];
  ULONG zf = zooms[zoomfaktor], br, h;
  ULONG y = sys->key[note->on->key] & KPOS;
  ULONG x = taktpos[takt][note->einheit];

  note->x = x;
  note->y = y;

  if (note->overbuff)
    {
      ULONG fx = note->overbuff->x + zf / 2;
      ULONG tx = x + zf / 2;
      ULONG ty = y + zf + zf / 4;
      printf ("Ziehe Verbindungsbogen X1 %d X2 %d Y%d\n", note->overbuff->x, x, y);

      DrawBogen (&backrp, fx, tx, ty, ty + zf);
    }

  br = s->breite;
  h = s->hoehe;

  if (note->typ & UP)           /* Fahne nach oben */
    {
      y -= h;
      y += zf;
#ifdef SSCORE
      printf ("Fahnenlängen %d\n", h);
#endif
    }

  if (note->typ & KREUZ)        /* # oder b */
    {
      struct SCOREFONT *k = &sfont[23];
      ULONG ky = y;

      if (note->typ & UP)
        {
          ky = y + h;
          ky -= zf;
        }

      ky -= zf;
      BltBitMap (&k->bitmap, 0, 0, &backmap, x - (k->breite + zf / 2), ky, k->breite, k->hoehe, 224, 1, 0);
    }
  else if (note->typ & AUFLOESUNG)      /* Auflösungszeichen */
    {
      struct SCOREFONT *k = &sfont[143];
      ULONG ky = y;
      printf ("Zeige Auflösung \n");

      if (note->typ & UP)
        {
          ky = y + h;
          ky -= zf;
        }

      ky -= zf;
      BltBitMap (&k->bitmap, 0, 0, &backmap, x - (k->breite + zf / 2), ky, k->breite, k->hoehe, 224, 1, 0);
    }

  {
    ULONG yh = sys->key[note->on->key] & KPOS;

    if (yh < (sys->starty - zf))/* Hilfslinien oben */
      {
        ULONG x1, x2, dy = sys->starty;
        x2 = x1 = x - zf / 2;
        x2 += 2 * zf;

        do
          {
            dy -= zf;
            Move (&backrp, x1, dy);
            Draw (&backrp, x2, dy);
          }
        while ((dy - zf) > yh);
      }
    else if (yh > sys->bty)     /* Hilfslinien unten */
      {
        ULONG x1, x2, dy = sys->bty;

        x2 = x1 = x - zf / 2;
        x2 += 2 * zf;

        do
          {
            dy += zf;
            Move (&backrp, x1, dy);
            Draw (&backrp, x2, dy);
          }
        while (dy < yh);
      }
  }

  if ((x + br) >= 1023)         /* Rechts raus */
    br = 1023 - x;

  if ((y + h) >= 767)           /* Unten raus */
    h = 767 - h;

  BltBitMap (&s->bitmap, 0, 0, &backmap, x, y, br, h, 224, 1, 0);

  if (note->typ & PUNKT)        /* Note mit Punkt ? */
    {
      ULONG yp = sys->key[note->on->key] & KPOS;
      s = &sfont[31];
      BltBitMap (&s->bitmap, 0, 0, &backmap, x + br, yp, s->breite, s->hoehe, 224, 1, 0);

      br += s->breite;
    }

  if (note->balken)             /* Diese Note befindet sich in einer Balkengruppe */
    {
      struct BALKENGRUPPE *bg = note->balken;
      struct BALKENNOTENPOS *bpos = bg->notenpos;

      if (note == bg->firstnote)/* Balkenpositionen erfassen */
        bg->noteposact = 0;

      if (bg->noteposact)
        bpos += bg->noteposact;
      bg->noteposact++;

      bpos->note = note;

      if (note->typ & UP)       /* Note zeigt nach oben */
        {
          ULONG tox = (x + br) - 1;

          bpos->x = tox;        /* Halsposition */
          bpos->y = y;

          if (!bg->fromx)
            bg->fromx = tox;
          else if ((!bg->tox) || (tox > bg->tox))
            bg->tox = tox;

          if ((!bg->fromy) ||
              ((bg->typ & WAAGERECHT) && bg->fromy > y)
            )
            {
              bg->fromy = y;
            }
          else
            {
              if ((!bg->toy) || (y > bg->toy))
                bg->toy = y;
            }
        }
      else
        /* Note zeigt nach unten */
        {
          ULONG toy = y + h;

          bpos->x = x;          /* Halsposition */
          bpos->y = y;

          if (!bg->fromx)
            bg->fromx = x;
          else if ((!bg->tox) || (x > bg->tox))
            bg->tox = x;

          if ((!bg->fromy) ||
              ((bg->typ & WAAGERECHT) && bg->fromy < toy)
            )
            {
              bg->fromy = toy;
            }
          else if ((!bg->toy) || (toy > bg->toy))
            bg->toy = toy;
        }
    }
}

void
ShowScoreSystemLinien (struct SCORESYSTEM *sys)
{
  struct RastPort *r = &backrp;
  ULONG y = sys->starty;
  UBYTE i, addy = zooms[zoomfaktor];

  for (i = 0; i < 5; i++)
    {
      if (y >= 767)
        break;
      Move (r, 0, y);
      Draw (r, 1023, y);

      y += addy;
    }
}

void
ShowScoreGrid (struct SCORESYSTEM *f)
{
  ULONG y = f->starty;
  UBYTE addy = zooms[zoomfaktor];

  ShowScoreSystemLinien (f);

  ScoreBlit (22, y + 1, 11);    /* Zähler */
  ScoreBlit (22, y + 1 + 2 * addy, 11); /* Nenner */
  {
    UBYTE schluessel;

    switch (f->notenschluessel)
      {
      case VIOLINKEY:
        ScrsAt (0, y - 9, f->track->name);
        schluessel = 26;
        y -= addy;
        break;

      case BASSKEY:
        schluessel = 36;
        break;
      }

    if (f != actscore)
      ScoreBlit (2, y, schluessel);     /* Violinenschluessel */
    else
      ScoreBlitInverse (2, y, schluessel);
  }
}

void
ShowBalken (struct BALKENGRUPPE *balken)
{
  struct RastPort *r = &backrp;
  ULONG x1, x2, y1, y2;
  ULONG balkenbreite, balkenhoehe, distanz = 0;
  UBYTE i, a, obennachunten, kopfoben;
  UBYTE zf2 = zooms[zoomfaktor] / 2;

  while (balken)
    {
      x1 = balken->fromx;
      x2 = balken->tox;
      balkenbreite = x2 - x1;
      y1 = balken->fromy;

      if (balken->firstnote->typ & UP)
        kopfoben = 0;
      else
        kopfoben = 1;

      if (balken->typ & WAAGERECHT)
        y2 = y1;
      else
        y2 = balken->toy;       /* Schräger Balken */

      distanz = 0;

      if (y1 < y2)
        {
          obennachunten = 1;    /* Der Balken von oben nach unten */
          balkenhoehe = y2 - y1;
        }
      else
        {
          obennachunten = 0;
          balkenhoehe = y1 - y2;
        }

      /* Achtelbalken zeichnen */
      if (!kopfoben)
        {
          for (i = 0; i < zf2; i++)     /* Achtel Balken zeichnen */
            {
              Move (r, x1, y1++);
              Draw (r, x2, y2++);
            }

          y1 += zf2;
          y2 += zf2;
        }
      else
        {
          for (i = 0; i < zf2; i++)     /* Achtel Balken zeichnen */
            {
              Move (r, x1, y1--);
              Draw (r, x2, y2--);
            }

          y1 -= zf2;
          y2 -= zf2;
        }

      distanz += zf2 * 2;

      {
        struct BALKENNOTENPOS *bpos;
        struct SCOREBUFF *note;
        ULONG len;
        UBYTE schraeg;
        BOOL flipx1 = FALSE;

        x1 = 0;
        for (i = 1; i < 4; i++) /* Check ob in diesem Balken noch 16,32,64,128 */
          {
            bpos = balken->notenpos;
            schraeg = 0;

            for (a = 0; a < balken->notecount; a++)
              {
                note = bpos->note;
                len = note->len;/* Notenlänge */

                printf ("Len of Note %d\n", len);

                if ((len == 384 || len == 576)) /* 1/8 oder 1/8· unterbrechen den Zusatzbalken*/
                  {
                    /************* Unterbrecher *************/
                    if (x1)
                      {
                        if (schraeg)    /* Schraege Kleinbalkenberechunung */
                          {
                            ULONG h;

                            if (schraeg & 1)    /* Y1 berechnen */
                              {
                                h = ((((x1 - balken->fromx) * 16384) / balkenbreite) * balkenhoehe) / 16384;

                                if (obennachunten)
                                  {
                                    if (!kopfoben)
                                      y1 = balken->fromy + h + distanz;
                                    else
                                      y1 = balken->fromy + h - distanz;
                                  }
                                else
                                  {
                                    if (!kopfoben)
                                      y1 = balken->fromy - h + distanz;
                                    else
                                      y1 = balken->fromy - h - distanz;
                                  }
                              }

                            if (schraeg & 2)    /* Y2 berechnen */
                              {
                                h = ((((x2 - balken->fromx) * 16384) / balkenbreite) * balkenhoehe) / 16384;

                                if (obennachunten)
                                  {
                                    if (!kopfoben)
                                      y2 = balken->fromy + h + distanz;
                                    else
                                      y2 = balken->fromy + h - distanz;
                                  }
                                else
                                  {
                                    if (!kopfoben)
                                      y2 = balken->fromy - h + distanz;
                                    else
                                      y2 = balken->fromy - h - distanz;
                                  }
                              }
                          }

                        {
                          UBYTE a;
                          for (a = 0; a < zf2; a++)     /* U-Zusatzbalken zeichnen */
                            {
                              if (!kopfoben)
                                {
                                  Move (r, x1, y1 + a);
                                  Draw (r, x2, y2 + a);
                                }
                              else
                                {
                                  Move (r, x1, y1 - a);
                                  Draw (r, x2, y2 - a);
                                }
                            }
                        }

                        printf ("Zeichne Unterbrechungs ZB von X1 %d nach X2 %d\n", x1, x2);
                        x1 = 0; /* Zurücksetzen */
                      }
                  }
                else
                  {
                    switch (i)
                      {
                      case 1:   /* 1/16 + 1/16· */
                        if (len == 192 || len == 288 ||
                            len == 96 || len == 144 ||
                            len == 48 || len == 72 ||
                            len == 24 || len == 36
                          )
                          {
                            /* Schräger Balken */
                            if (!(balken->typ & WAAGERECHT))
                              {
                                printf ("Schraege ZU berechnet\n");

                                if (!a) /* Erste Note im Balken */
                                  {
                                    x1 = bpos->x;
                                    x2 = x1 + 6;
                                    schraeg = 2;        /* NUR y2 berechen */
                                  }
                                else
                                  {
                                    x2 = bpos->x;

                                    if (a == balken->notecount) /* Letzte Note im Balken */
                                      {
                                        if (schraeg == 2)       /* Durchgehende Linie ziehen - KEINE Berechnenung */
                                          schraeg = 0;
                                        else
                                          schraeg = 1;  /* NUR y1 berechnen */
                                      }
                                    else
                                      {
                                        x2 = bpos->x;
                                        x1 = x2 - 6;    /* Kleinbalken nach links */
                                        schraeg = 3;    /* Beide Position berechnen */
                                      }
                                  }
                              }
                            else
                              /* Waagerechter Balken */
                              {
                                printf ("Waagerechten Balken berechnen \n");

                                if (!x1)
                                  {
                                    if (!a)     /* Erste Note im Balken -> Kleinbalken nach rechts*/
                                      {
                                        x1 = bpos->x;
                                        printf ("*** Balken *** X1 %d\n", x1);
                                        x2 = x1 + 6;
                                      }
                                    else
                                      {
                                        x2 = bpos->x;
                                        x1 = x2 - 6;    /* Kleinbalken nach links */
                                        flipx1 = TRUE;
                                      }
                                  }
                                else
                                  {
                                    if (flipx1 == TRUE)
                                      {
                                        x1 = x2;
                                        flipx1 = FALSE;
                                      }

                                    x2 = bpos->x;
                                  }
                              }
                          }
                        printf ("1/16 im Balken gefunden X1 %d -> X2 %d\n", x1, x2);
                        break;

                      case 2:   /* 1/32 + 1/32· */
                        if (len == 96 || len == 144 ||
                            len == 48 || len == 72 ||
                            len == 24 || len == 36
                          )
                          {
                            x1 = bpos->x;
                          }

                        printf ("1/32 im Balken gefunden\n");
                        break;

                      case 3:   /* 1/64 + 1/64· */
                        if (len == 48 || len == 72 || len == 24 || len == 36)
                          {
                            x1 = bpos->x;
                          }
                        printf ("1/64 im Balken gefunden\n");
                        break;

                      case 4:   /* 1/96 oder 1/128 */
                        if (len == 24 || len == 36)
                          {
                            x1 = bpos->x;
                          }
                        printf ("1/128 im Balken gefunden\n");
                        break;
                      }
                  }

                bpos++;
              }

            /* ZusatzBalken zeichnen */
            if (x1)
              {
                if (schraeg)    /* Schraege Kleinbalkenberechunung */
                  {
                    ULONG h;

                    if (schraeg & 1)    /* Y1 berechnen */
                      {
                        h = ((((x1 - balken->fromx) * 16384) / balkenbreite) * balkenhoehe) / 16384;

                        if (obennachunten)
                          {
                            if (!kopfoben)
                              y1 = balken->fromy + h + distanz;
                            else
                              y1 = balken->fromy + h - distanz;
                          }
                        else
                          {
                            if (!kopfoben)
                              y1 = balken->fromy - h + distanz;
                            else
                              y1 = balken->fromy - h - distanz;
                          }
                      }

                    if (schraeg & 2)    /* Y2 berechnen */
                      {
                        h = ((((x2 - balken->fromx) * 16384) / balkenbreite) * balkenhoehe) / 16384;

                        if (obennachunten)
                          {
                            if (!kopfoben)
                              y2 = balken->fromy + h + distanz;
                            else
                              y2 = balken->fromy + h - distanz;
                          }
                        else
                          {
                            if (!kopfoben)
                              y2 = balken->fromy - h + distanz;
                            else
                              y2 = balken->fromy - h - distanz;
                          }
                      }

                    printf ("Schraegbalken DISTANZ %d\n", distanz);
                  }

                if (!kopfoben)  /* Restbalken zeichnen */
                  {
                    for (i = 0; i < zf2; i++)   /* Achtel Balken zeichnen */
                      {
                        Move (r, x1, y1++);
                        Draw (r, x2, y2++);
                      }

                    y1 += zf2;
                    y2 += zf2;
                  }
                else
                  {
                    for (i = 0; i < zf2; i++)   /* Achtel Balken zeichnen */
                      {
                        Move (r, x1, y1--);
                        Draw (r, x2, y2--);
                      }

                    y1 -= zf2;
                    y2 -= zf2;
                  }

                distanz += zf2 * 2;

                printf ("Zeichne ZB von X1 %d nach X2 %d\n", x1, x2);

                x1 = 0;         /* Zurücksetzen */
              }
            else
              break;            /* Keine Balken mehr */
          }
      }

      /* Notenbalken verlängern ? */
      {
        struct BALKENNOTENPOS *bpos = balken->notenpos;

        for (i = 0; i < balken->notecount; i++)
          {
            if (balken->firstnote->typ & UP)    /* Die Noten in dieser Gruppe zeigen nach OBEN */
              {
                if (bpos->y > (balken->fromy + zf2))
                  {
                    Move (r, bpos->x, bpos->y);
                    Draw (r, bpos->x, (balken->fromy + zf2));

                    printf ("Balken OBEN nachzeichnen \n");
                  }
              }
            else
              {
                if (bpos->y < balken->toy)
                  {
                    printf ("Balken UNTEN nachzeichnen \n");
                  }
              }

            printf ("NP X: %d Y: %d\n", bpos->x, bpos->y);
            bpos++;
          }
      }

      balken = balken->next;
    }
}

void
ShowSystem (struct SCORESYSTEM *sys)
{
  struct SCORETAKT *st = sys->edittakt; /* Ab Editierstart */
  struct SCOREBUFF *noten;
  struct PAUSE *pausen;
  ULONG newx, xp;
  ULONG zf = zooms[zoomfaktor];
  ULONG zf2 = zf / 2;
  ULONG taktto;

#ifdef DBG
  printf ("*** ShowSystem ***\n");
#endif

  newx = xp = sys->startx;

  ShowScoreGrid (sys);

  while (st)
    {
      noten = st->firstnote;
      pausen = st->firstpause;
      taktto = st->taktnr - scoretaktstart;

#ifdef SSCORE
      printf ("** ShowTakt Nr.: %d **\n", st->taktnr);
#endif

      /* Sprung zum passenden Editstart */
      while (noten && noten->quanttime < editstart)
        noten = noten->next;

      while (pausen && pausen->start < editstart)
        pausen = pausen->next;

      if (st->firstbalken)
        {
          struct BALKENGRUPPE *r = st->firstbalken;

          while (r)
            {
              r->fromx = r->fromy = r->tox = r->toy = 0;

              r = r->next;
            }
        }

      while (noten || pausen)
        {
          /* Checken ob Pause oder Note */
          if (pausen && ((!noten) || (pausen->start < noten->quanttime)))       /****** Pause *********/
            {
              if (pausen->punkt & 2)    /* Ab hier Leertakte */
                {
                  ULONG l = pausen->len;        /* Anzahl der Leertakte */

                  if (pausen->imagenr)  /* Leertakt kann mit 1 Image gefüllt werden */
                    {
                      while (l)
                        {
                          xp += zf;

                          DrawPause (sys, pausen, taktto);

                          xp += zf;

                          if (xp < leftedge)
                            {
                              Move (&backrp, xp, sys->starty),  /* Taktstrich */
                                Draw (&backrp, xp, sys->bty);

                              xp += zf;
                            }

                          l--;
                        }
                    }
                }
              else
                /* Richtige Pause */
                {
                  DrawPause (sys, pausen, taktto);
                }

              pausen = pausen->next;
            }
          else if (noten)
            /****** Note *********/
            {
              if (noten->akkordanzahl)
                DrawAkkordNote (sys, noten, taktto);
              else
                DrawNote (sys, noten, taktto);

              noten = noten->next;
            }
          else
            noten = noten->next;

          if (xp >= leftedge)
            break;              /* Rechts raus */
        }

      /* Takt zu Ende - TaktStrich */

      xp = taktstriche[st->taktnr];
      if (xp < leftedge)
        {
          Move (&backrp, xp, sys->starty),      /* Taktstrich */
            Draw (&backrp, xp, sys->bty);
        }

      /* Balken zeichnen */
      if (st->firstbalken)
        ShowBalken (st->firstbalken);

      if (xp >= leftedge)
        break;                  /* Rechts raus */

      st = st->next;
    }
}

void
ClearScore ()
{
  EraseRect (&backrp, 0, 0, leftedge, 767);
}

void
ClipBack ()
{
  BltBitMapRastPort (&backmap, 0, 0, srp, 6, stopy, sbtx - 6, sbty - stopy, 0xC0);      /* Notenkopf */
}

void
ShowScoreHead ()
{
  struct RastPort *r = srp;
  ULONG x, takt = 0, einheit, to;

  to = g_taktA * quantlist[g_taktB] / quantlist[notequant];

  for (einheit = 0; einheit < to; einheit += g_taktA)
    {
      x = taktpos[takt][einheit];
      printf ("Einheit %d XPOS %d\n", einheit, x);
      Move (r, x, stopy);
      Draw (r, x, stopy + 10);
    }

  Move (r, stopx, stopy + 10);
  Draw (r, sbtx, stopy + 10);
  Move (r, stopx, stopy + 11);
  Draw (r, sbtx, stopy + 11);
}

void
ShowScore ()
{
  struct TRACK *t = MTR->firsttrack;

#ifdef SSCORE
  printf ("********* ScoreShow ******** \n");
#endif

  ClearScore ();

  SetAPen (srp, 1);
  SetBPen (srp, 0);
  SetWrMsk (srp, 1);

  while (t)
    {
      if (t->score)
        {
          printf ("Zeige Track %s\n", t->name);
          ShowSystem (t->score->score);
          if (t->score->scorebass)
            ShowSystem (t->score->scorebass);
        }

      t = t->next;
    }
  ClipBack ();

  ShowScoreHead ();
}

#endif
