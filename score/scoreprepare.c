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
InitPausenImage (struct PAUSE *p, ULONG len)
{
  UBYTE nr, punkt = 0;

  switch (len)
    {
    case 384:                   /* 1/8 */
      nr = 108;
      break;

    case 768:                   /* 1/4 */
      nr = 119;
      break;

    case 192:                   /* 1/16 */
      nr = 76;
      break;

    case 96:                    /* 1/32 */
      nr = 144;
      break;

    case 1536:                  /* 1/2 */
      nr = 117;                 /* Muss später in 116 umgewandelt werden ! */
      break;

    case 3072:                  /* 1 */
      nr = 116;
      break;

    case 6144:                  /* Doppelganze */
      nr = 67;
      break;

    case 48:                    /* 1/64 */
      nr = 99;
      break;

    case 24:                    /* 1/128 */
      nr = 109;
      break;

      /* Punktierte Pausen */

    case 4608:                  /* Ganze */
      nr = 116;
      punkt = 1;
      break;

    case 2304:
      nr = 117;                 /* Muss später in 116 umgewandelt werden ! */
      punkt = 1;
      break;

    case 576:                   /* 1/8 */
      nr = 108;
      punkt = 1;
      break;

    case 1152:                  /* 1/4 */
      nr = 119;
      punkt = 1;
      break;

    case 288:                   /* 1/16 */
      nr = 76;
      punkt = 1;
      break;

    case 144:                   /* 1/32 */
      nr = 144;
      punkt = 1;
      break;

    case 72:                    /* 1/64 */
      nr = 99;
      punkt = 1;
      break;

    case 36:
      nr = 109;
      punkt = 1;
      break;

    default:
#ifdef SSCORE
      printf ("Unbekannte Pause ! %d\n", len);
#endif
      nr = 0;
      break;
    }

  p->imagenr = nr;
  p->punkt = punkt;
#ifdef SSCORE
  printf ("LenPausenImage %d Punkt %d\n", nr, punkt);
#endif
}

struct BALKENGRUPPE *firstbalken, *lastbalken;

void
ConnectBalkenNotenimTakt (struct SCORETAKT *takt)
{
  struct SCOREBUFF *c = takt->firstnote;
  struct SCOREBUFF *f_buff[32];
  ULONG i, found, tfound;

  for (i = 1; i < (takt->taktA + 1); i++)       /* Die einzelnen Qrts im Takt */
    {
      found = 0;

      while (c && (c->qrt == i))/* Gleicher Schlag */
        {
          if (c->len <= 576)    /* 1/8· Kann verbunden werden ! */
            {
              f_buff[found++] = c;
            }

          c = c->next;
        }

      /* Check ob Pausen dazwischen liegt */
      if (found > 1)            /* Min 2 Gefunden */
        {
          if (tfound = found)
            {
              struct PAUSE *p = takt->firstpause;
              ULONG a;

              while (p)
                {
                  for (a = 1; a < found; a++)
                    {
                      if (f_buff[a] && f_buff[a - 1] &&
                          (p->start < f_buff[a]->quanttime) &&
                          (p->start > f_buff[a - 1]->quanttime)
                        )
                        {
                          f_buff[a] = 0;        /* Diese Noten können nicht verbunden werden */
                          f_buff[a - 1] = 0;

                          tfound -= 2;
                        }
                    }
                  p = p->next;
                }
            }

          if (tfound)           /* In diesem Qrt koennen Noten verbunden werden */
            {
              struct BALKENGRUPPE *nb = 0;
              struct SCOREBUFF *fb;
              ULONG a, lastkey = 255;
              UBYTE down = 0, up = 0;
              UBYTE upmoeglich = 0, downmoeglich = 0;

              for (a = 0; a < found; a++)       /* Waagerechter Balken ? */
                {
                  if (fb = f_buff[a])
                    {
                      if (fb->typ & UP)
                        upmoeglich = 1;
                      else
                        downmoeglich = 1;

                      if (lastkey != 255)
                        {
                          if (lastkey < fb->on->key)
                            up++;
                          else if (lastkey > fb->on->key)
                            down++;
                        }

                      lastkey = fb->on->key;
                    }
                }

              for (a = 0; a < found; a++)
                {
                  if (fb = f_buff[a])   /* Zugelassen ? */
                    {
                      if (!nb)
                        {
                          nb = (struct BALKENGRUPPE *) GetMem (sizeof (struct BALKENGRUPPE));

                          if (!firstbalken)     /* Für Balkenreset */
                            firstbalken = nb;
                          else
                            lastbalken->next = nb;

                          lastbalken = nb;

                          /* Balkengruppe in den Takt einfügen */
                          if (!takt->firstbalken)
                            takt->firstbalken = nb;
                          else
                            takt->lastbalken->next = nb;

                          takt->lastbalken = nb;

                          nb->qrt = i;
                          nb->next = 0;
                          nb->firstnote = fb;   /* Für Balkenzusätze */

                          nb->notecount = tfound;
                          nb->notenpos = (struct BALKENNOTENPOS *) GetMem (tfound * (sizeof (struct BALKENNOTENPOS)));
                        }

                      if (up >= down)
                        {
                          if (upmoeglich)
                            {
                              fb->typ |= UP;
                              fb->imagenr = 69; /* Viertel Oben */
                            }
                          else
                            {
                              if (fb->typ & UP)
                                fb->typ ^= UP;

                              fb->imagenr = 103;        /* Viertel Unten */
                            }
                        }
                      else
                        {
                          if (downmoeglich)
                            {
                              if (fb->typ & UP)
                                fb->typ ^= UP;

                              fb->imagenr = 103;        /* Viertel Unten */
                            }
                          else
                            {
                              fb->typ |= UP;
                              fb->imagenr = 69; /* Viertel Oben */

                            }
                        }

                      fb->balken = nb;  /* Der Noten den Balken zuweisen */
                      nb->typ = FORCEACHTELBALKEN;

                      if ((down && up) || ((!down) && (!up)))
                        nb->typ |= WAAGERECHT;
                    }
                }
            }
        }
    }
}

/* Einer Note Hals, Fahne usw. zuweisen */
void
InitNotenImage (struct SCORESYSTEM *sys, struct SCOREBUFF *to)
{
  UBYTE imagenr;
  UBYTE key = to->on->key;
  UBYTE punkt = 0;
  BOOL fahneoben;

  if (key < sys->clipfahne)
    fahneoben = TRUE;           /* Oberhalb der ClipNote */
  else
    fahneoben = FALSE;

#ifdef DBG
  printf ("** GetNotenImage Ticks: %d **\n", ticks);
#endif

  switch (to->len)
    {
    case 4608:                  /* Ganze · */
      imagenr = 92;
      fahneoben = 0;
      punkt = 1;
      break;

    case 3072:                  /* Ganze , keine Fahne, kein Hals*/
      imagenr = 92;
      fahneoben = 0;
      break;

    case 2304:                  /* Halbe · */
      if (fahneoben)
        imagenr = 138;          /* Hals oben */
      else
        imagenr = 45;           /* Hals unten */
      punkt = 1;
      break;

    case 1536:                  /* Halbe */
      if (fahneoben)
        imagenr = 138;          /* Hals oben */
      else
        imagenr = 45;           /* Hals unten */
      break;

    case 1152:                  /* 1/4 · */
      if (fahneoben)
        imagenr = 69;
      else
        imagenr = 103;
      punkt = 1;
      break;

    case 768:                   /* 1/4 */
      if (fahneoben)
        imagenr = 69;
      else
        imagenr = 103;
      break;

    case 576:                   /* 1/8 · */
      if (fahneoben)
        imagenr = 135;
      else
        imagenr = 42;
      punkt = 1;
      break;

    case 384:                   /* 1/8 */
      if (fahneoben)
        imagenr = 135;
      else
        imagenr = 42;
      break;

    case 288:                   /* 1/16 · */
      if (fahneoben)
        imagenr = 89;
      else
        imagenr = 123;
      punkt = 1;
      break;

    case 192:                   /* 1/16 */
      if (fahneoben)
        imagenr = 89;
      else
        imagenr = 123;
      break;

    case 144:                   /* 1/32 · */
      if (fahneoben)
        imagenr = 70;
      else
        imagenr = 110;
      punkt = 1;
      break;

    case 96:                    /* 1/32 */
      if (fahneoben)
        imagenr = 70;
      else
        imagenr = 110;
      break;

    case 72:                    /* 1/64 · */
      if (fahneoben)
        imagenr = 20;
      else
        imagenr = 117;
      punkt = 1;
      break;

    case 48:                    /* 1/64 */
      if (fahneoben)
        imagenr = 20;
      else
        imagenr = 117;
      break;

    case 36:                    /* 1/96 */
      if (fahneoben)
        imagenr = 58;
      else
        imagenr = 66;
      punkt = 1;
      break;

    case 24:                    /* 1/128 */
      if (fahneoben)
        imagenr = 58;
      else
        imagenr = 66;
      break;
    }
#ifdef SSCORE
  printf ("** GetNotenImage ImageNr %d **\n", imagenr);
#endif
  to->imagenr = imagenr;

  if (fahneoben)
    to->typ |= UP;
  if (punkt)
    to->typ |= PUNKT;
}

void
InitAkkord (struct SCORESYSTEM *sys, struct SCOREBUFF *fakkord)
{
  struct SCOREBUFF *check = fakkord;
  UBYTE up = 0, down = 0;
  printf ("Initalisiere einen %d Akkord !\n", fakkord->akkordanzahl);

  printf("ClipFahne %d\n",sys->clipfahne);

  do
    {
      if (check->on->key < sys->clipfahne)
        up++;                   /* Oberhalb der ClipNote */
      else
        down++;

      printf("Akkordkey %d\n",check->on->key);

      if (check == fakkord)
        check = check->firstunderakkord;
      else
        check = check->next;
    }
  while (check);

  if (up > down)
    fakkord->typ |= UP;

  printf ("Up %d Down %d\n", up, down);
}

void
PrepareScore ()
{
  struct TRACK *t = MTR->firsttrack;
  struct SCORESYSTEM *sys;
  struct SCORETAKT *st;
  struct SCOREBUFF *note;
  ULONG lnote;
  ULONG plen;

#ifdef SSCORE
  printf ("********* Analyse ******** \n");
#endif

  while (t)
    {
      if (t->score)
        {
          sys = t->score->score;

          /* Analyse der einzelnen Takte */
          printf ("Analyse Track %s\n", t->name);

          for (;;)
            {
              st = sys->firsttakt;

              while (st)
                {
#ifdef SSCORE
                  printf ("TaktNummer %d Taktbeginn %d\n", st->taktnr, st->taktstart);
#endif
                  st->pixbr = 0;
                  /***** musikalische Notenanalyse innerhalb des Taktes ****/

                  /* Pausen im Takt feststellen */
                  note = st->firstnote;
                  lnote = st->taktstart;

                  firstbalken = 0;

                  while (note)
                    {
                      if (note->akkordanzahl)
                        InitAkkord (sys, note); /* Akkord initalisieren */
                      else
                        InitNotenImage (sys, note);     /* Image/Punkt/Fahne usw. für die Länge ->imagenr erzeugen */

                      /* Pause zur letzten Note errechen */
                      if (note->quanttime >= (lnote + quantlist120[notequant])) /* min. 1/128pause */
                        {
                          plen = (note->quanttime - lnote) / 120;

                          /********* Pausenlänge quantisieren **********/
                          {
                            ULONG time, mi, quant = quantlist[notequant];
                            LONG quantdiff = 0;

                            time = plen;
                            /* Len ist jetzt ppq */

                            /* Quantizieren */
                            mi = quant / 2;
                            time -= (time / quant) * quant;     /* Runter auf Q-Raster */

                            /*    Normales Quantizieren */
                            if (time < mi)
                              quantdiff = -time;
                            else
                              quantdiff = quant - time;

                            plen += quantdiff;

                            if (plen)   /* Pause in die Pausenliste aufnehmen */
                              {
                                struct PAUSE *np = GetPausenMem (st);
                                struct globb *g = &globbar[2];

                                np->einheit = ConvertScoreTimeToBar ((np->start = lnote));
                                np->qrt = g->qrt;
                                printf ("Pausen Einheit I Takt %d EH %d\n", st->taktnr, np->einheit);

                                if (plen <= quantlist[st->taktB])
                                  InitPausenImage (np, (np->len = plen));
#ifdef SSCORE
                                printf ("<> Pause zur vorherigen Note in Ticks %d Start %d\n", plen, lnote);
#endif
                              }
                          }
                        }

                      lnote = note->quanttime + 120 * note->len;        /* Soviel Taktzeit verbraucht diese Note */
                      if ((!note->next) && (lnote + quantlist120[notequant]) <= st->taktende)   /* Der Takt ist zu Ende - Pause bis zum Ende des Taktes min. 1/128 ?*/
                        {
                          struct PAUSE *np = 0;
                          ULONG add = quantlist120[st->taktB];
                          ULONG right;
                          ULONG quant = quantlist[notequant];

                          if (note->qrtend & 128)
                            right = st->taktende;
                          else
                            right = st->taktstart + note->qrtend * add;

                          for (;;)
                            {
                              if (lnote + quantlist120[notequant] <= right)
                                {
                                  plen = (right - lnote) / 120; /* Der Rest im Qrt */

                                  /* Quantisieren */
                                  {
                                    ULONG time, mi;
                                    LONG quantdiff = 0;

                                    time = plen;
                                    /* Len ist jetzt ppq */

                                    /* Quantizieren */
                                    mi = quant / 2;
                                    time -= (time / quant) * quant;     /* Runter auf Q-Raster */

                                    /*    Normales Quantizieren */
                                    if (time < mi)
                                      quantdiff = -time;
                                    else
                                      quantdiff = quant - time;

                                    plen += quantdiff;
                                  }

                                  {
                                    struct globb *g = &globbar[2];

                                    np = GetPausenMem (st);
                                    np->einheit = ConvertScoreTimeToBar ((np->start = lnote));
                                    np->qrt = g->qrt;

                                    lnote = right;

                                    printf ("Pausen Einheit II Takt %d EH %d\n", st->taktnr, np->einheit);

                                    InitPausenImage (np, (np->len = plen));
                                  }

                                  right = st->taktende;
                                }
                              else
                                break;
                            }
                        }

                      note = note->next;
                    }

                  /*** Balkenverbindung im Takt ***/
                  if (st->firstnote && st->firstnote->next)
                    ConnectBalkenNotenimTakt (st);

                  if (st = st->next)
                    {
                      ULONG tdiff = st->taktnr - st->last->taktnr;

                      /* Leertakte */
                      if (tdiff > 1)
                        {
                          struct PAUSE *np = GetPausenMem (st);
                          struct globb *g = &globbar[2];

                          np->len = tdiff - 1;
                          np->einheit = ConvertScoreTimeToBar ((np->start = st->last->taktstart));
                          np->qrt = g->qrt;

                          printf ("Pausen Einheit III Takt %d EH %d\n", st->taktnr, np->einheit);

                          if ((st->taktA != 4) || (st->taktB != 8))     /* 4/2 */
                            np->imagenr = 67;
                          else
                            np->imagenr = 116;  /* Leertakte werden mit Ganzen gefüllt */

                          np->punkt |= 2;       /* Zeichen für Leertakt !!! */
                        }
                    }
                }

              if ((!t->score->scorebass) || (sys == t->score->scorebass))       /* Ins Basssystem -> */
                break;
              else
                sys = t->score->scorebass;
            }
        }

      t = t->next;
    }

  /*
  PrepareBalken ();             /* ggf. Z.b 1/16 Balken erzeugen */
  */
}

#endif
