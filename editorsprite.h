#ifndef CAMX_GUISPRITES
#define CAMX_GUISPRITES 1

#ifdef OLDIE

#include "colours.h"

// SPRITES ------------------

// SUBTYPES-FLAGS
#define SPRITETYPE_RECT_NOLEFT 1
#define SPRITETYPE_RECT_NOTOP 2
#define SPRITETYPE_RECT_NORIGHT 4
#define SPRITETYPE_RECT_NOBOTTOM 8

class guiSprite:public Object
{
public:
	enum Type{
		SPRITETYPE_LINE_VERT,
		SPRITETYPE_LINE_HORZ,
		SPRITETYPE_RECT
	};

	enum Mode{
		SPRITEDISPLAY_USER,
		SPRITEDISPLAY_MOUSETIME,
		SPRITEDISPLAY_CLOCKTIME, 
		SPRITEDISPLAY_OVERVIEWCLOCKTIME,
		SPRITEDISPLAY_OVERVIEWRANGE,
		SPRITEDISPLAY_SELECTION,
		SPRITEDISPLAY_MOVE,
		SPRITEDISPLAY_SIZE,
		SPRITEDISPLAY_NEWNOTE
	};

	guiSprite()
	{
		colour=COLOUR_RED;
		type=SPRITETYPE_LINE_VERT;
		subtype=0;

		display=SPRITEDISPLAY_USER;

		ondisplay=false;
		staticsprite=true;
		on=true;

	//	list=0;
	}

	guiSprite(int t,int dt)
	{
		colour=COLOUR_RED;

		display=dt;
		type=t;
		subtype=0;
		staticsprite=false;
	}

	guiSprite *NextSprite() {return (guiSprite *)next;}
	guiSprite *PrevSprite() {return (guiSprite *)prev;}

	int type,subtype,display,colour;
	int x,x2,y,y2;
	bool ondisplay,staticsprite,on;
};
#endif

#endif
