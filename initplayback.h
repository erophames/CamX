#ifndef CAMX_INITPLAYBACK
#define CAMX_INITPLAYBACK 1

#include "defines.h"

class InitPlay
{
public:
	InitPlay(int ii)
	{
		onlyrecordingpattern=false;
		initindex=ii;
		delay=position=0;
		initstream=false;
	};

	InitPlay(int ii,OSTART pos)
	{
		onlyrecordingpattern=false;
		initindex=ii;
		position=pos;
		delay=0;
		initstream=false;
	};

	OSTART position,audioautomationstart,delay;
	int mode,initindex;
	bool started,onlyrecordingpattern,initstream;
};
#endif
