
/*** Die Tracks, von denen Blocks selektiert sind, erhalten ein Standardsystem ***/
UBYTE
ConvertScoreTimeToBar (ULONG time)
{
  struct globb *z = &globbar[2];
  struct CMD *c;
  ULONG factor, qrt, sixtl;
  UBYTE normal = 1;
  UBYTE einheit;

  z->nextcmd = 0;

  if (MTR->muteblock && (c = (struct CMD *) MTR->muteblock->instr))
    {
      /****************** Mit Taktwechsel ***********************/
      struct CMD *lt = 0;
      ULONG taktmin = 0;

      while (c)
        {
          if (c->start > time)  /* Nächster Takt ? */
            {
              z->nextcmd = c;
              break;
            }

          if (!lt)
            taktmin = c->start;
          else
            taktmin += c->start - lt->start;

          lt = c;
          normal = 0;           /* Ein anderer Takt als der Starttakt ist für diese Zeit */

          z->taktA = c->value;
          z->taktB = c->valueb;
          z->bar = c->dummy;

          c = (struct CMD *) c->nextedit;
        }

      if (!normal)
        {
          time -= taktmin;
          einheit = time / scorefaktor;

          /******** Umrechnen ************/
          if ((factor = quantlist120[z->taktB]) <= time)
            {
              /* QRT */
              qrt = time / factor;

              switch (qrt)
                {
                case 1:
                  time -= factor;
                  break;

                case 2:
                  time -= 2 * factor;
                  break;

                case 3:
                  time -= 3 * factor;
                  break;

                default:
                  time -= qrt * factor;
                }

              if (qrt >= z->taktA)
                {
                  ULONG abar;

                  if (abar = qrt / z->taktA)
                    {
                      qrt -= abar * z->taktA;
                      z->bar += abar;
                    }
                }

              z->qrt = qrt + 1;
            }
          else
            z->qrt = 1;

          /* SIXTL */
          if ((factor = quantlist120[scorezoom]) <= time)
            {
              sixtl = time / factor;

              switch (sixtl)
                {
                case 1:
                  time -= factor;
                  z->sixtl = 2;
                  break;

                case 2:
                  time -= 2 * factor;
                  z->sixtl = 3;
                  break;

                case 3:
                  time -= 3 * factor;
                  z->sixtl = 4;
                  break;

                default:
                  time -= sixtl * factor;
                  z->sixtl = sixtl + 1;
                }
            }
          else
            z->sixtl = 1;

          if (time >= 120)
            z->t768 = (time / 120) + 1;
          else
            z->t768 = 1;
        }
    }

  /* Ohne Taktänderung */

  if (normal)
    {
      ULONG bar;

      z->taktA = g_taktA;       /* StartTakt */
      z->taktB = g_taktB;

      /* BAR */
      if ((factor = g_taktlen) <= time)
        {
          bar = time / factor;
          time -= bar * factor;

          z->bar = bar + 1;
        }
      else
        z->bar = 1;

      einheit = time / scorefaktor;

      /* QRT */
      if (time)
        {
          factor /= g_taktA;

          if (factor <= time)
            {
              qrt = time / factor;

              switch (qrt)
                {
                case 1:
                  time -= factor;
                  z->qrt = 2;
                  break;

                case 2:
                  time -= 2 * factor;
                  z->qrt = 3;
                  break;

                case 3:
                  time -= 3 * factor;
                  z->qrt = 4;
                  break;

                default:
                  time -= qrt * factor;
                  z->qrt = qrt + 1;
                }
            }
          else
            z->qrt = 1;
        }
      else
        z->qrt = 1;

      /* SIXTL */
      if (time)
        {
          if ((factor = quantlist120[scorezoom]) <= time)
            {
              sixtl = time / factor;

              switch (sixtl)
                {
                case 1:
                  time -= factor;
                  z->sixtl = 2;
                  break;

                case 2:
                  time -= 2 * factor;
                  z->sixtl = 3;
                  break;

                case 3:
                  time -= 3 * factor;
                  z->sixtl = 4;
                  break;

                default:
                  time -= sixtl * factor;
                  z->sixtl = sixtl + 1;
                }
            }
          else
            z->sixtl = 1;

          if (time >= 120)
            {
              z->t768 = (time / 120) + 1;
            }
          else
            z->t768 = 1;
        }
      else
        {
          z->sixtl = 1;
          z->t768 = 1;
        }
    }

  printf ("Einheit %d\n", einheit);

  return (einheit);
}
