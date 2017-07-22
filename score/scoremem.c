#include "seq:include.h"
#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>         /* Data types.             */
#include <exec/errors.h>        /* Exec error messages.    */
#include <devices/printer.h>    /* Printer Device.         */
#include <exec/io.h>            /* Standard request block. */

#ifdef SCORE

#define SCOREBUFFER 150

static struct SCOREGF far scoregfbuffer[SCOREBUFFER];
static struct SCOREGF *startscoregfbuffer, *lastscoregfbuffer;

/************ Bildschirmnotenspeicher freigeben ********************/
void
FreeScoreShowMem ()
{
  struct SCOREGF *bb, *b = startscoregfbuffer;

  while (b)
    {
      bb = b;
      b = b->next;

      if (bb->used == 1)
        bb->used = 0;
      else if (bb->used == 2)
        Free_Mem (bb, sizeof (struct SCOREGF));
    }

  startscoregfbuffer = 0;
  lastscoreused = 0;
}

/************ Eine Note/Pause usw. am Bildschirm anforden ******************/
struct SCOREELEMENT *
GetScoreShowMem ()
{
  struct SCOREGF *ff = 0;
  ULONG i;

  for (i = lastscoreused; i < SCOREBUFFER; i++)
    {
      if (!scoregfbuffer[i].used)
        {
          ff = &scoregfbuffer[i];
          ff->used = 1;         /* Nicht allokiert */
          lastscoreused = i;
          break;
        }
    }

  if (!ff)                      /* Memory anfordern */
    {
      if(ff = (struct SCOREGF *) GetMem (sizeof (struct SCOREGF)))
        ff->used = 2;
    }

  return ((struct SCOREELEMENT *) ff);
}

void                            /* Alle ScoreBuffer freigeben */
FreeScoreList ()
{
  struct SCOREBUFF *e = firstscorelist, *b;

  struct TRACK *t = MTR->firsttrack;

  while (t)
    {
      if (t->score)
        {
          t->score->score->onscore = 0;
        }

      t = t->next;
    }

  while (e)                     /* Buffer ferigeben */
    {
      if (e->typ)
        {
          b = e;
          e = e->next;

          if (b->akkordanzahl)  /* Akkorde freigeben */
            {
              struct SCOREBUFF *a = b->firstunderakkord, *na;

              while (a)
                {
                  na = a;
                  Free_Mem (na, sizeof (struct SCOREBUFF));

                  a = a->next;
                }

            }

          Free_Mem (b, sizeof (struct SCOREBUFF));

        }
      else
        e = e->next;
    }

  firstscorelist = 0;
  lscorebuff = 0;
}

/* Den Takt-Speicher freigeben */
void
FreeTaktMem (struct SCORESYSTEM *sys)
{
  struct SCORETAKT *st = sys->firsttakt, *bt;

  while (st)
    {
      bt = st;
      st = st->next;
      Free_Mem (bt, sizeof (struct SCORETAKT));
    }

  sys->firsttakt = sys->lasttakt = 0;
}

struct PAUSE *
GetPausenMem (struct SCORETAKT *st)
{
  struct PAUSE *np = (struct PAUSE *) GetMem (sizeof (struct PAUSE));

  if (!st->firstpause)
    {
      st->firstpause = np;
      np->last = 0;
    }
  else
    {
      st->lastpause->next = np;
      np->last = st->lastpause;
    }

  st->lastpause = np;
  np->next = 0;
  return (np);
}

void
RemovePausenfromSystem (struct SCORESYSTEM *sys)
{
  struct SCORETAKT *takt = sys->firsttakt;

  while (takt)
    {
      /** Pausen/Balkengruppen entfernen **/
      {
        struct BALKENGRUPPE *g = takt->firstbalken, *gb;
        /* struct BALKENZUSATZ *zu, *bzu; */

        while (g)
          {
            /* Zusatzbalken entfernen */
            /*
            zu = g->firstzusatz;
            while (zu)
              {
                bzu = zu;
                zu = zu->next;

                Free_Mem (bzu, sizeof (struct BALKENZUSATZ));
              }
            */

            gb = g;
            g = g->next;

            if (gb->notecount && gb->notenpos)
              Free_Mem (gb->notenpos, gb->notecount * sizeof (struct BALKENNOTENPOS));

            Free_Mem (gb, sizeof (struct BALKENGRUPPE));
          }
      }
      takt->firstbalken = takt->lastbalken = 0;

      /* Pausen entfernen */
      {
        struct PAUSE *p = takt->firstpause, *pb;

        while (p)
          {
            pb = p;
            p = p->next;
            Free_Mem (pb, sizeof (struct PAUSE));
          }
      }
      takt->firstpause = takt->lastpause = 0;

      takt = takt->next;
    }
}

void
FreeAllPausenMem ()
{
  struct TRACK *t = MTR->firsttrack;

  while (t)
    {
      if (t->score)
        {
          RemovePausenfromSystem (t->score->score);
          if (t->score->scorebass)
            RemovePausenfromSystem (t->score->scorebass);
        }

      t = t->next;
    }
}

void
FreeAllTaktMem ()
{
  struct TRACK *t = MTR->firsttrack;

  while (t)
    {
      if (t->score)
        {
          FreeTaktMem (t->score->score);

          if (t->score->scorebass)
            FreeTaktMem (t->score->scorebass);
        }

      t = t->next;
    }
}

void
FreeAllScoreMem ()
{
  FreeAllPausenMem ();
  FreeAllTaktMem ();
  FreeScoreList ();
  FreeScoreShowMem ();

  FreeZoomBitMaps ();
  scoretaktstart = 65535;
}

#endif
