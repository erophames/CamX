#include "seq:include.h"

#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>         /* Data types.             */
#include <exec/errors.h>        /* Exec error messages.    */
#include <devices/printer.h>    /* Printer Device.         */
#include <exec/io.h>            /* Standard request block. */

#ifdef SCORE
struct OVERLONG *firstover, *lastover;

void
InitKeyPositions (struct SCORESYSTEM *sys)
{
  ULONG y, i;
  ULONG addy = zooms[zoomfaktor];

  y = sys->starty;
  y -= 5 * addy;                /* 5 Hilfslinien von oben */
  addy /= 2;
  y -= addy;

  for (i = sys->maxkey; i > sys->minkey; i--)
    {
      sys->key[i] = y;
      /* Bit 15=VORZ */

      if (!black[i])
        y += addy;
    }
}

void
InitFirstScoreTakts ()
{
  ULONG ed = editstart;
  ULONG taktnr;
  ULONG cc;

  ConvertTimeToBar (ed, &globbar[8], 0);
  taktnr = globbar[8].bar;

  if (taktnr != scoretaktstart)
    {
      struct SCORESYSTEM *sys;
      struct SCORETAKT *stf, *stl;

      scoretaktstart = taktnr;

      for (cc = 0; cc < onscorecount; cc++)
        {
          sys = scoresystems[cc];
          stf = sys->firsttakt;
          stl = sys->lasttakt;
          sys->edittakt = 0;

          if (stl && stl->taktnr < taktnr)
            break;

          while (stf && stl)
            {
              if (stf->taktnr >= taktnr)        /* Von vorn */
                {
                  sys->edittakt = stf;
                  break;
                }

              if (stl->taktnr < taktnr) /* Von hinten */
                {
                  sys->edittakt = stl->next;
                  break;
                }

              stf = stf->next;
              stl = stl->last;
            }
        }
    }
}

void
FreeOverLong ()
{
  struct OVERLONG *n = firstoverlong, *nb;

  while (n)
    {
      nb = n;
      n = n->next;

      Free_Mem (nb, sizeof (struct OVERLONG));
    }

  firstoverlong = lastoverlong = 0;
}

void
InitStartOverLong ()            /* Noten suchen, die aus Takten VOR den editstartpunkt überlappen */
{
  struct EVENT *check;

  FreeOverLong ();

  if (firstevent && (check = firstevent->lastedit))
    {
      ULONG s = editstart + quantlist120[scorezoom];
      ULONG low = editstart - longestnote;

      while (check && check->start >= low)
        {
          if (((check->status & 240) == NOTEON) &&
              (((struct NON *) check)->end >= s)
            )
            {
              struct OVERLONG *new = (struct OVERLONG *) GetMem (sizeof (struct OVERLONG));

              new->on = (struct NON *) check;
              new->end = ((struct NON *) check)->end;

              if (!firstoverlong)
                firstoverlong = new;
              else
                lastoverlong->next = new;

              lastoverlong = new;

              printf ("Überlange Note gefunden Start %d Ende %d\n", check->start, new->end);
            }

          check = check->lastedit;
        }
    }
}

void
AllocScoreTrack ()
{
  struct TRACK *t = MTR->firsttrack;
  struct BLOCK *block;
  struct SCORESYSTEM *sys;
  ULONG addy = zooms[zoomfaktor];

  onscorecount = 0;
  while (t)
    {
      if ((!t->maintrack) && (block = t->firstblock))   /* Noch kein System */
        {
          while (block)
            {
              if (block->selected && (!block->head) && (!block->twinblock))     /* Ein Block ist selektiert */
                {
                  struct SCORETOP *top;

                  if (t->score)
                    {
                      scoresystems[onscorecount++] = t->score->score;

                      if (t->score->scorebass)
                        scoresystems[onscorecount++] = t->score->scorebass;
                    }
                  else
                    {
                      if (top = t->score = (struct SCORETOP *) GetMem (sizeof (struct SCORETOP)))
                        {
                          sys = top->score = (struct SCORESYSTEM *) GetMem (sizeof (struct SCORESYSTEM));

                          if (!actscore)
                            actscore = sys;

                          scoresystems[onscorecount++] = sys;

                          sys->track = t;
                          /* Default 1-Liniensystem,Violin */
                          sys->vorzeichen = 7;  /* C-Dur */

                          sys->minkey = 52;
                          sys->maxkey = 95;
                          sys->clipfahne = 72;  /* Fahne oben/unten */
                          sys->startx = stopx + 45;     /* Ab hier können Noten geblittet werden */
                          sys->starty = starty; /* Ab hier können Noten geblittet werden */
                          sys->bty = sys->starty + 4 * addy;
                          sys->notenschluessel = VIOLINKEY;
                          sys->onscore = 0;
                          InitKeyPositions (sys);

                          top->clip = 1;        /* Doppelsystem !!! */
                          top->clipdoppelkey = 60;

                          /* Unteres System */
                          sys = top->scorebass = (struct SCORESYSTEM *) GetMem (sizeof (struct SCORESYSTEM));

                          scoresystems[onscorecount++] = sys;
                          sys->track = t;

                          sys->minkey = 32;
                          sys->maxkey = 75;
                          sys->startx = stopx + 45;     /* Ab hier können Noten geblittet werden */
                          sys->starty = starty + 8 * addy;
                          sys->bty = sys->starty + 4 * addy;
                          sys->clipfahne = 52;  /* C5 */
                          sys->notenschluessel = BASSKEY;
                          InitKeyPositions (sys);

                          starty += 100;
                        }
                    }
                  break;
                }

              block = block->next;
            }
        }
      t = t->next;
    }

  printf ("Anzahl Onscore %d\n", onscorecount);
}

void
AddNoteToTakt (struct EVENT *e, ULONG start, ULONG end, ULONG takt, ULONG taktende, struct SCOREBUFF *over)
{
  struct SCOREBUFF *newnoteintakt;
  struct globb *g = &globbar[2];
  struct TRACK *track;
  struct SCORETAKT *nt;
  struct SCORESYSTEM *sys;

  printf ("AddNoteToTakt Start %d - Ende %d\n", start, end);

  /************** In die ScoreListe aufnehmen ******************/
  if (lscorebuff < 1000)        /* Aus dem Buffer */
    {
      newnoteintakt = (struct SCOREBUFF *) (scorebuffmem + (lscorebuff * sizeof (struct SCOREBUFF)));
      lscorebuff++;
      newnoteintakt->typ = 0;   /* Buffer */
    }
  else
    /* Extra anfordern */
    {
      newnoteintakt = (struct SCOREBUFF *) GetMem (sizeof (struct SCOREBUFF));
      newnoteintakt->typ = ST_TYP;      /* Allokiert */
    }
  /*************************************************************/

  newnoteintakt->overbuff = over;

  newnoteintakt->on = (struct NON *) e; /* Zeiger auf die org. Note */
  newnoteintakt->next = 0;

  /** Quantisieren und Takt zuorden **/

  /**** Zeit Quantisierung ****/
  if (!over)
    newnoteintakt->quanttime = SimpleQuantize (start, notequant);
  else
    newnoteintakt->quanttime = start;

  newnoteintakt->einheit = ConvertScoreTimeToBar (newnoteintakt->quanttime);
  newnoteintakt->takt = g->bar; /* Aus ConvertScoreTimeToBar ! */
  newnoteintakt->qrt = g->qrt;
  /****************************************************************/

  track = e->block->track;

  /*** Hier clippen ****/
  if (track->score->clip && track->score->clipdoppelkey >= ((struct NON *) e)->key)
    {
      sys = track->score->scorebass;
    }
  else
    {
      sys = track->score->score;/* Ins obere System */
    }

  /***  Schwarze Taste ***/
  if (!over && black[((struct NON *) e)->key])
    {
      newnoteintakt->typ |= KREUZ;
      printf ("Schwarze Taste %s\n", cks[((struct NON *) e)->key]);
    }

  /* Taktzuordung und Schaffung einer Systemtaktliste */
  if ((!(nt = sys->lasttakt)) ||
      (nt->taktnr != newnoteintakt->takt)
    )
    {
      nt = (struct SCORETAKT *) GetMem (sizeof (struct SCORETAKT));

      /* Einsortieren der Taktstruktur in das Tracksystem --> */
      if (!sys->firsttakt)
        {
          sys->firsttakt = nt;
          nt->last = 0;
        }
      else
        {
          sys->lasttakt->next = nt;
          nt->last = sys->lasttakt;
        }

      sys->lasttakt = nt;
      nt->next = 0;

      /* 1.Note einhängen */
      nt->firstnote = nt->lastnote = newnoteintakt;
      newnoteintakt->last = 0;

      /* TaktInitialisierung */
      nt->taktA = g->taktA;
      nt->taktB = g->taktB;
      nt->taktnr = g->bar;
      nt->taktende = nt->taktstart = ConvertBarToTime (g->bar, 0, 0, 0, 0);
      nt->taktende += nt->taktA * quantlist120[nt->taktB];
#ifdef SSCORE
      printf ("Takt generiert %d im Track %s\n Taktart %d / %s \n", nt->taktnr, t->name, g->taktA, quantstr[g->taktB]);
#endif
    }
  else
    {
      /************************** Akkordnote ? ******************************/
      if (nt->lastnote->qrt == newnoteintakt->qrt &&
          nt->lastnote->einheit == newnoteintakt->einheit)
        {
          struct SCOREBUFF *ntlast = nt->lastnote;

          ntlast->typ |= AKKORD;

          if (!ntlast->firstunderakkord)
            {
              ntlast->firstunderakkord = newnoteintakt;
              ntlast->akkordanzahl = 2;
              newnoteintakt->last = 0;
            }
          else
            {
              ntlast->lastunderakkord->next = newnoteintakt;
              newnoteintakt->last = ntlast->lastunderakkord;
              ntlast->akkordanzahl++;
            }

          ntlast->lastunderakkord = newnoteintakt;

          printf ("Akkordnote hinzugefügt Anzahl %d\n", ntlast->akkordanzahl);
        }
      else
        /* KEIN Akkord */
        {
          nt->lastnote->next = newnoteintakt;
          newnoteintakt->last = nt->lastnote;

          nt->lastnote = newnoteintakt;
        }

      newnoteintakt->next = 0;
    }

  /****************************************************************************/

  /**** Ende quantisieren ***/
  {
    ULONG qend = SimpleQuantize (end, notequant);
    ULONG eeh;

    if (qend == newnoteintakt->quanttime)
      {
        printf ("Ende auf Anfang quantisiert !!! Note zu klein\n");
        qend += quantlist120[notequant];        /* Minimum Note */
      }

    eeh = ConvertScoreTimeToBar (qend);

    /*** Note ragt über den Takt hinaus ***/
    if (qend >= taktende + quantlist120[notequant])
      {
        struct OVERLONG *newover = (struct OVERLONG *) GetMem (sizeof (struct OVERLONG));

        if (!firstover)
          {
            firstover = newover;
            newover->last = 0;
          }
        else
          {
            lastover->next = newover;
            newover->last = lastover;
          }

        lastover = newover;
        newover->next = 0;

        newover->starttakt = takt + 1;
        newover->endtakt = g->bar;
        newover->start = taktende + 120;
        newover->end = qend;    /* Bereits quantisiert !!! */
        newover->on = (struct NON *) e;
        newover->buff = newnoteintakt;

        printf ("Note ragt über den Takt hinaus %d >>> %d\n", takt, g->bar);
        printf ("Rest %d\n", qend - taktende);
        qend = taktende;
        eeh = ConvertScoreTimeToBar (qend);
      }

    if (newnoteintakt->quanttime > 1)
      newnoteintakt->len = (qend - newnoteintakt->quanttime) / 120;
    else
      newnoteintakt->len = qend / 120;

    newnoteintakt->qrtend = g->qrt;

    if (g->sixtl == 1 && g->t768 == 1)
      newnoteintakt->qrtend |= 128;     /* Liegt genau am Qrt-Punkt */

    /******** PseudoNoten im Takt erzeugen ? *********/
    if (g->qrt > newnoteintakt->qrt)
      {
        printf ("Split Note SQ %d SE %d EQ %d EE %d\n", newnoteintakt->qrt, newnoteintakt->einheit, g->qrt, eeh);

        /* Note zerschneiden im Takt */
        if (newnoteintakt->takt == g->bar)
          {
            ULONG schnitt1, schnitt2;
            ULONG schnittstart[16];
            ULONG schnittlen[16];
            ULONG end = 0, q, schnittcount = 0, geschnitten = 0;

            schnitt1 = newnoteintakt->quanttime;
            schnitt2 = nt->taktstart + (quantlist120[g->taktB] * newnoteintakt->qrt);

            for (q = newnoteintakt->qrt; q <= g->qrt; q++)
              {
                printf ("Schnitt von %d bis %d\n", schnitt1, schnitt2);

                schnittstart[schnittcount] = schnitt1;
                schnittlen[schnittcount] = schnitt2 - schnitt1;

                if (end || (schnitt2 == schnitt1))
                  break;

                schnittcount++;
                schnitt1 = schnitt2;
                schnitt2 += quantlist120[g->taktB];

                if (schnitt2 >= qend)   /* Rest */
                  {
                    schnitt2 = qend;
                    end = 1;
                  }
              }

            printf ("Die Note wurde in %d Einheiten zerschnitten \n", schnittcount);

            /* Optimierung */
            if (schnittcount > 1)
              {
                UBYTE opt[16], optimiert = 0;

                for (q = 0; q < 16; q++)
                  opt[q] = 0;

                for (q = 0; q < (schnittcount - 1); q++)
                  {
                    if (schnittlen[q] == schnittlen[q + 1])
                      {
                        optimiert = opt[q + 1] = 1;
                        printf ("Schnitt %d weg optimiert \n", q + 1);
                      }
                  }

                if (optimiert)
                  {
                    UBYTE lastnichtopt = 0;

                    for (q = 0; q < (schnittcount - 1); q++)
                      {
                        if (opt[q + 1])
                          {
                            schnittlen[lastnichtopt] += schnittlen[q + 1];      /* Optimierungsaddierung */
                            schnittlen[q + 1] = 0;
                          }
                        else
                          lastnichtopt++;
                      }
                  }
              }

            printf (" *** Endschnitt ***\n");

            for (q = 0; q < schnittcount; q++)
              {
                if (schnittlen[q])
                  {
                    struct SCOREBUFF *ps;

                    if (!geschnitten)
                      {
                        newnoteintakt->len = schnittlen[q] / 120;
                        newnoteintakt->qrtend = newnoteintakt->qrt;

                        printf ("Ausgangsnotelänge %d\n", newnoteintakt->len);
                        geschnitten = 1;
                      }
                    else
                      {
                        printf ("Speicher für eine PS-Note angefordert\n");

                        if (lscorebuff < 1000)  /* Aus dem Buffer */
                          {
                            ps = (struct SCOREBUFF *) (scorebuffmem + (lscorebuff * sizeof (struct SCOREBUFF)));
                            lscorebuff++;
                            ps->typ = 0;        /* Buffer */
                            ps->last = 0;
                          }
                        else
                          /* Extra anfordern */
                          {
                            ps = (struct SCOREBUFF *) GetMem (sizeof (struct SCOREBUFF));
                            ps->typ = ST_TYP;   /* Allokiert */
                          }

                        ps->typ |= PSEUDO;

                        ps->last = nt->lastnote;
                        nt->lastnote->next = ps;
                        ps->next = 0;
                        nt->lastnote = ps;

                        ps->on = (struct NON *) e;      /* Zeiger auf die org. Note */
                        ps->quanttime = schnittstart[q];

                        ps->einheit = ConvertScoreTimeToBar (ps->quanttime);
                        ps->takt = g->bar;      /* Aus ConvertScoreTimeToBar ! */
                        ps->qrt = g->qrt;

                        ps->len = schnittlen[q] / 120;
                        ps->qrtend = g->qrt;

                        if (g->sixtl == 1 && g->t768 == 1)
                          ps->qrtend |= 128;    /* Liegt genau am Qrt-Punkt */

                        printf ("PS-Start %d PS-EH %d PS-Len %d\n", ps->quanttime, ps->einheit, ps->len);
                      }
                  }
              }
          }
      }
  }
}


struct EVENT *
FilterToTakt (struct EVENT *e, USHORT takt)
{
  ULONG taktende = ConvertBarToTime (takt + 1, 0, 0, 0, g_zoom);
  printf("Filtere Note im Takt %d\n",takt);

  /******** Overlong-Check ***********/
  {
    struct OVERLONG *check = firstover;

    while (check)
      {
        if (check->starttakt == takt)
          {
            printf ("OVERLONG in den Takt %d eingebunden \n", takt);
            AddNoteToTakt ((struct EVENT *) check->on, check->start, check->end, takt, taktende, check->buff);
          }

        check = check->next;
      }
  }

  /* Noten aus der Eventliste rausholen und Notenstruktur anlegen */
  while (e)
    {
      if (e->start >= taktende)
        break;                  /* Takt ist zu Ende */

      if ((e->status & 240) == NOTEON)
        {
          struct EVENT *n = e->nextedit;
          ULONG end = ((struct NON *) e)->end;

          /* Check ob eine Note diese Note schneidet - die 2.Note schneidet die 1. ab */
          while (n && n->start < end && n->start < taktende)
            {
              if (((n->status & 240) == NOTEON) &&
                  (((struct NON *) n)->key == ((struct NON *) e)->key)
                )
                {
                  printf (">>>>>>>>>>>>>>>>>>>>>>>>>> Note abgeschnitten %s \n", cks[((struct NON *) e)->key]);
                  end = n->start;
                  break;
                }

              n = n->nextedit;
            }

          if (end >= e->start)
            {
              AddNoteToTakt (e, e->start, end, takt, taktende, 0);
            }
        }

      e = e->nextedit;
    }

  return (e);
}

void
FilterScoreEvents ()
{
  struct EVENT *e = firstevent;

#ifdef SSCORE
  printf ("********* Filter+Logik1 ******** \n");
#endif

  starty = 70;

  MouseSprite (ScoreWnd, MS_WAIT);
  AllocScoreTrack ();
  InitStartOverLong ();

  FreeScoreList ();

  lscorebuff = 0;               /* Reset der Buffer */

  e = FilterToTakt (e, 1);
  e = FilterToTakt (e, 2);

  MouseSprite (ScoreWnd, MS_NORMAL);
}

#endif

