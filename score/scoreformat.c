#include "seq:include.h"
#include <string.h>
#include <graphics/scale.h>
#include <hardware/blit.h>

#include <exec/types.h>		/* Data types.             */
#include <exec/errors.h>	/* Exec error messages.    */
#include <devices/printer.h>	/* Printer Device.         */
#include <exec/io.h>		/* Standard request block. */

#ifdef SCORE
#include "scoreconvert.c"

ULONG actscorex;

void
ResetVorzeichen ()
{
  struct SCORESYSTEM *sys;
  ULONG i, cc;

  for (cc = 0; cc < onscorecount; cc++)	/* Starttakte, Buffs */
    {
      sys = scoresystems[cc];

      for (i = 0; i < 128; i++)
	sys->key[i] &= KPOS;
    }
}

void
FormatScore ()
{
  struct SCORESYSTEM *sys;
  struct SCOREFONT *s;
  USHORT *einheit = 0;
  ULONG taktnr, totakt, x;
  ULONG cc, br, found, checkeinheit = 0, ff, lf;

  printf ("ONSCORE %d\n", onscorecount);
  for (cc = 0; cc < onscorecount; cc++)	/* Starttakte, Buffs */
    {
      sys = scoresystems[cc];

      if (sys->taktformat = sys->edittakt)
	{
	  sys->noteformat = sys->taktformat->firstnote;
	  sys->pauseformat = sys->taktformat->firstpause;
	}
    }

  actscorex = 50;		/* StartX */

  for (taktnr = 1; taktnr < 4; taktnr++)	/* Wieviel Takte sollen dargestellt werden ? */
    {
      totakt = taktnr - scoretaktstart;
      taktpos[totakt][0] = actscorex;	/* Die erste Einheit ist stets bekannt */

      ResetVorzeichen ();

      /***** Suche nach der KLEINSTEN Einheit in den Systemen ********/
      do
	{
	  found = 0;

	  for (cc = 0; cc < onscorecount; cc++)
	    {
	      sys = scoresystems[cc];

	      if (sys->taktformat && sys->taktformat->taktnr == taktnr)	/* Passender Takt ? */
		{
		  if (sys->noteformat)	/* Note */
		    {
		      if ((!found) || (sys->noteformat->einheit < checkeinheit))
			{
			  if (!found)
			    ff = cc;

			  lf = cc;

			  checkeinheit = sys->noteformat->einheit;
			  found = 1;
			}
		      else if (sys->noteformat->einheit == checkeinheit)
			lf = cc;
		    }

		  if (sys->pauseformat)	/* Note */
		    {
		      if ((!found) || (sys->pauseformat->einheit < checkeinheit))
			{
			  if (!found)
			    ff = cc;

			  lf = cc;

			  checkeinheit = sys->pauseformat->einheit;
			  found = 1;
			}
		      else if (sys->pauseformat->einheit == checkeinheit)
			lf = cc;
		    }
		}
	    }

	  if (found)
	    {
	      einheit = &taktpos[totakt][checkeinheit];
	      *einheit = actscorex;

	      printf ("Einheit gesetzt Takt %d EH %d X %d\n", totakt, checkeinheit, actscorex);

	      for (cc = ff; cc <= lf; cc++)
		{
		  sys = scoresystems[cc];

		  if (sys->taktformat && (sys->taktformat->taktnr == taktnr))
		    {
		      if (sys->noteformat && (sys->noteformat->einheit == checkeinheit))	/* Note */
			{
			  ULONG zf = zooms[zoomfaktor];
			  ULONG key = sys->noteformat->on->key;

			  x = actscorex;

			  if (sys->noteformat->typ & KREUZ)	/* # oder b */
			    {
			      key--;

			      if (!(sys->key[key] & VORZ))	/* Kein # auf dieser Note */
				{
				  sys->key[key] |= VORZ;	/* Die nächste gleiche Note erhält ein Auflösungszeichen */
				  s = &sfont[23];
				  x += s->breite;
				  x += zf / 2;
				}
			      else
				sys->noteformat->typ ^= KREUZ;
			    }
			  else
			    /* Auflösungzeichen ? */
			    {
			      if (sys->key[key] & VORZ)	/* Kein # auf dieser Note */
				{
				  printf ("Auflösungszeichen\n");
				  sys->noteformat->typ |= AUFLOESUNG;
				  sys->key[key] ^= VORZ;
				  s = &sfont[143];
				  x += s->breite;
				  x += zf / 2;
				}
			    }

			  if (x > *einheit)
			    *einheit = x;

			  sys->noteformat = sys->noteformat->next;
			}

		      if (sys->pauseformat && sys->pauseformat->einheit == checkeinheit)	/* Pause */
			{
			  s = &sfont[sys->pauseformat->imagenr];
			  x = actscorex;
			  br = s->breite;

			  if (sys->pauseformat->punkt & 1)	/* Punktiert ? */
			    {
			      ULONG im = 0;

			      switch (sys->pauseformat->imagenr)	/* NICHT durch inr ersetzen !!! */
				{
				case 67:	/* Doppelganze */
				  im = 116;
				  break;

				case 116:	/* Ganze */
				  im = 116;
				  break;

				case 117:	/* Halbe */
				  im = 119;
				  break;

				case 119:	/* Viertel */
				  im = 108;
				  break;

				case 108:	/* Achtel */
				  im = 76;
				  break;

				case 76:	/* Sechstel */
				  im = 144;
				  break;

				case 144:	/* 32tel */
				  im = 99;
				  break;
				}

			      if (im)
				{
				  s = &sfont[im];
				  br += s->breite;
				}
			    }

			  x += br;

			  if (x > *einheit)
			    *einheit = x;

			  sys->pauseformat = sys->pauseformat->next;
			}
		    }
		}
	    }

	  if (einheit)
	    actscorex = *einheit + 14;	/* Notenbreite */
	}
      while (found);		/* Takt ist ausgelesen */

      taktstriche[taktnr] = actscorex;
      actscorex += 14;		/* Notenbreite - Neuer Takt */

      for (cc = 0; cc < onscorecount; cc++)
	{
	  sys = scoresystems[cc];

	  if (sys->taktformat = sys->taktformat->next)
	    {
	      sys->noteformat = sys->taktformat->firstnote;
	      sys->pauseformat = sys->taktformat->firstpause;

	      found = 1;
	    }
	}

      if (!found)
	break;
    }

  /*
  {
    UBYTE i, a;

    for (i = 0; i < 4; i++)
      for (a = 0; a < 32; a++)
        printf ("Takt %d EH %d Pos %d\n", i, a, taktpos[i][a]);
  }
  */

}

#endif
