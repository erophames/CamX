#ifndef CAMX_MIDISYNC_H
#define CAMX_MIDISYNC_H 1

#include "defines.h"

class Seq_Song;

	enum
	{
		SYNC_INTERN,
		SYNC_MTC,
		SYNC_MC
	};

class MIDISync
{
public:

	MIDISync()
	{
		sync=syncbeforestart=SYNC_INTERN;
		out_nextclocksample=0;
		out_nextclockposition=0;
		startedwithmc=startedwithmtc=false;
		sendmtc=false;
		sendmc=false;
	};

	void BufferBeforeTempoChanges();
	void RefreshTempoBuffer();

	Seq_Song *song;
	LONGLONG out_nextclocksample;
	double buffer_nextMIDIclockposition;
	OSTART out_nextclockposition;
	int sync,syncbeforestart,mtccounter,mtccheckstart,mtccheckcounter;
	bool startedwithmc,startedwithmtc,sendmtc,sendmc;
};

#endif
