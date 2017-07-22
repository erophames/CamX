#ifndef CAMX_METRONOME_H
#define CAMX_METRONOME_H 1

#include "defines.h"
#include "object.h"

class Seq_Song;

#define AUDIOMETROINDEX 0
#define MIDIMETROINDEX 1
#define METROINDEXS 2 

class camxFile;
class AudioHDFile;

class metroClick
{
public:
	metroClick()
	{		
		record=true;
		playback=false;
		on=true;
	};

	void Load(camxFile *);
	void Save(camxFile *);

	void InitMetroClick(OSTART,int index);
	void InitMetroClickForce(int index); // | ... next click
	bool CheckIfHi(OSTART click);
	void SendClick(bool hi);
	int GetPreCounterDone();

	void RefreshTempoBuffer();

	Seq_Song *song;
	LONGLONG nextclick_sample[METROINDEXS],startpremetrosystime_samples[METROINDEXS],waitpreMIDIticks;
	OSTART nextclick[METROINDEXS];

	double waitMIDIoffsetticks;

	int beat[METROINDEXS],
		precountertodo[METROINDEXS];

	bool 
		sendpresignaltoMIDI,
		on,playback,record,sendsynctoMIDI;
};
#endif
