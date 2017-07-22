#define stopx 10
#define stopy 88

#define SCOREBR 12
#define startkey 65

#define SCOREIMAGES 145

/* Für SCOREBUFF->typ */
#define PUNKT 1
#define KREUZ 2
#define PSEUDO    4    /* überlange Note */
#define UP    8
#define ST_TYP 16      /* STATISCH */
#define AKKORD 32
#define DOPPEL 64
#define AUFLOESUNG 128

#define FORCEACHTELBALKEN 128
#define WAAGERECHT 64

#define SINGLESYSTEM 0
#define DOPPELSYSTEM 1
#define PERCSYSTEM 2

/* Notenschluessel */
#define VIOLINKEY 0
#define BASSKEY 1

/* Format - Draw */
#define DRAW 0
#define FORMAT 1

#define KPOS (USHORT)0x7FFF
#define VORZ (USHORT)0x8000      /* Bit 15 */

struct SCOREFONT
{
  struct BitMap bitmap;
  USHORT breite, hoehe, size;
};

struct SCOREIMAGE
{
UWORD *mem;
ULONG br,hh;
};

struct SCORETOP                 /* Track -> Score */
{
  struct SCORESYSTEM *score;    /* Oberes System */
  struct SCORESYSTEM *scorebass;/* Oberes und unteres System */

  UBYTE clip, clipdoppelkey;    /* clip=Doppelsystem, clipkey=ClipNote */
};

struct SCOREELEMENT
{
  struct SCOREELEMENT *next, *last;
  ULONG x, x2, y, y2;
  UBYTE typ;
};

struct SCOREBUFF
{
  struct SCOREBUFF *next, *last;
  struct SCOREBUFF *overbuff;
  struct BALKENGRUPPE *balken;
  struct NON *on;
  ULONG quanttime;
  USHORT takt;                  /* Kann weg */
  USHORT len,x,y;
  UBYTE einheit;
  UBYTE typ;
  UBYTE qrt;
  UBYTE qrtend;
  UBYTE imagenr;
  UBYTE akkordanzahl;
  struct SCOREBUFF *firstunderakkord,*lastunderakkord;
};

struct PAUSE
{
  struct PAUSE *next, *last;
  ULONG start;
  USHORT len;
  UBYTE imagenr, punkt;
  UBYTE qrt;
  UBYTE einheit;
};

struct SCORESYSTEM
{
  struct TRACK *track;
  struct SCORETAKT *firsttakt, *lasttakt;       /* Im ganzen System */
  struct SCORETAKT *edittakt;   /* ab Editstartpunkt */
  struct TRACK *source;

  struct SCORETAKT *taktformat;
  struct SCOREBUFF *noteformat;
  struct PAUSE *pauseformat;

  USHORT key[128];                          /* Bit 15 = Vorzeichen */
  USHORT startx, starty, bty,pixused;

  UBYTE notenschluessel;
  UBYTE vorzeichen;
  UBYTE minkey, maxkey,clipfahne;
  UBYTE onscore;
};

struct BALKENNOTENPOS
{
struct SCOREBUFF *note;
USHORT x,y;
};

struct BALKENGRUPPE
{
  struct BALKENGRUPPE *next;
  struct SCOREBUFF *firstnote;
  struct BALKENNOTENPOS *notenpos;
  UBYTE notecount,noteposact;
  USHORT fromx, fromy, tox, toy;
  UBYTE qrt, typ;               /* 0=1/8 */
};

struct AKKORDGRUPPE
{
struct SCOREBUFF *firstakkordnote,*lastakkordnote;
UBYTE anzahl;
UBYTE qrt,einheit;
};

struct SCORETAKT
{
  struct BALKENGRUPPE *firstbalken, *lastbalken;
  struct SCORETAKT *next, *last;
  struct SCOREBUFF *firstnote, *lastnote;
  struct PAUSE *firstpause, *lastpause;
  struct AKKORDGRUPPE *firstakkord,*lastakkord;
  struct HALTEBOGEN *firstbogen,*lastbogen;

  ULONG taktstart, taktende;
  USHORT taktnr, pixbr;
  UBYTE taktA, taktB;
};

struct SCOREGF                  /* 1 Note am Bildschirm */
{
  struct NON *on;
  struct SCOREGF *next, *last, *connect;
  ULONG quanttime;
  USHORT x, y, takt, len;
  UBYTE fahne, vorzeichen, qrt, sixtl, used;
};

struct OVERLONG                /* Noten die über den Takt raushängen */
{
struct OVERLONG *next,*last;
struct NON *on;
struct SCOREBUFF *buff;
ULONG start,end;
USHORT starttakt,endtakt;
};



