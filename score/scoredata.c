#ifdef SCORE

struct SCOREFONT far sfont[150];

ULONG zoomfaktor = 7, zoomwhat = 27;

ULONG zooms[]=
{20, 18, 16, 14, 12, 10, 8, 6, 4, 3};

ULONG linienbr[]=
{2, 2, 2, 2, 2, 1, 1, 1, 1, 1};

ULONG scoretaktstart = 65535;
USHORT taktstriche[80];
USHORT taktpos[80][32];

struct SCOREBUFF *firstscorelist, *lastscorelist;
USHORT lscorebuff;
UBYTE *scorebuffmem;

ULONG starty;
struct SCORESYSTEM *scoresystems[255];
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
struct Gadget *ScoreSList, *scoreslider;

struct OVERLONG *firstoverlong, *lastoverlong;

ULONG scoreeditend, lastscoreused;
ULONG scoreh, sbtx, sbty, startscorebar, endscorebar, startscoreqrt, scorefactor,
  scorezoom = 14;
ULONG scorefaktor = 23040;
ULONG startpixel = 8, vtopy, linienh = 88;

UBYTE scorebreite = 7, realscore, notequant = 14;
UBYTE toplinien = 2, downlinien = 2;

static UWORD __chip szdata[72] =
{
/* Plane 0 */
  0xFFFF, 0xF000,
  0xFFFF, 0xE000,
  0xFFFF, 0xE000,
  0xFFFF, 0xE000,
  0xFFFD, 0xE000,
  0xFFF9, 0xE000,
  0xFFF1, 0xE000,
  0xFFE1, 0xE000,
  0xFFC1, 0xE000,
  0xFF81, 0xE000,
  0xFF01, 0xE000,
  0xFE01, 0xE000,
  0xFC01, 0xE000,
  0xF801, 0xE000,
  0xFFFF, 0xE000,
  0xFFFF, 0xE000,
  0xFFFF, 0xE000,
  0x8000, 0x0000,
/* Plane 1 */
  0x0000, 0x0000,
  0x7FFF, 0xF000,
  0x7FFE, 0xF000,
  0x7FFC, 0xF000,
  0x7FF8, 0xF000,
  0x7FF0, 0xF000,
  0x7FE0, 0xF000,
  0x7FC0, 0xF000,
  0x7F80, 0xF000,
  0x7F00, 0xF000,
  0x7E00, 0xF000,
  0x7C00, 0xF000,
  0x7800, 0xF000,
  0x7000, 0xF000,
  0x6000, 0xF000,
  0x7FFF, 0xF000,
  0x7FFF, 0xF000,
  0x7FFF, 0xF000
};

struct Image sizeim =
{
  0, 0,
  20, 18,
  2,
  &szdata[0],
  0x03,0x03,
  NULL
};

UBYTE toolboxact;
UBYTE toolboxmax;

#endif