#ifndef CAMX_COLOURREQ 
#define CAMX_COLOURREQ 1

#include "defines.h"

class Seq_Event;
class camxFile;

class Colour
{
public:
	Colour()
	{
		showcolour=false;
		rgb=0x00EEEEEE;
		changed=false;
	}

	bool Compare(Colour *g)
	{
		if(g->showcolour==showcolour && g->rgb==rgb)return true;
		return false;
	}

	void InitColour(Seq_Event *);

	void Clone(Colour *c)
	{
		c->showcolour=showcolour;
		c->rgb=rgb;
	}

	void LoadChunk(camxFile *);
	void SaveChunk(camxFile *);

	int rgb;
	bool showcolour,changed;
};

class guiWindow;
class colourReq
{
public:
	colourReq();
	~colourReq();

	void OpenRequester(guiWindow *,Colour *c=0);
	void CloseRequester();
};
#endif