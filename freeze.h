#ifndef CAMX_FREEZE 
#define CAMX_FREEZE 1

/*

#include "defines.h"
#include "threads.h"

#include "audiodevice.h"
#include "audiomaster.h"

class ToFreezePattern:public Object
{
public:
	ToFreezePattern()
	{
		started=false;
		start=true;
		frozen=false;

		masterticks=0;

		freezefilename=0;

		tofreezepattern=0;
		track=0;
		flag=0;
	}

	OSTART masterticks;

	MIDIPattern *tofreezepattern;
	Seq_Track *track;

	char *freezefilename;
	bool started,start,frozen;
	ToFreezePattern *NextPatternToFreeze(){return (ToFreezePattern *)next;}
};


class AudioFreezeThread:public Thread
{
public:
	AudioFreezeThread()
	{
		createfrozentrack=0;
		createfreezeprogress=0;
	}

	void DoFreeze(ToFreezePattern *tf);
	int StartThread();
	void StopFreezing(Seq_Track *track,bool lock);

#ifdef WIN32
	static PTHREAD_START_ROUTINE AudioFreezeCreator(LPVOID pParam);
#endif

	bool AddFreezePattern(ToFreezePattern *tf);

	ToFreezePattern *FirstToFreezePattern(){return (ToFreezePattern *)tofreezetracks.GetRoot();}

	Seq_Track *createfrozentrack;
	double createfreezeprogress; // 0-100%

private:
	OList tofreezetracks;
};
*/

#endif
