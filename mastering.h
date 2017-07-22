#ifndef CAMX_MASTERING
#define CAMX_MASTERING 1

#include "audiohdfile.h"

class Mastering;
class Seq_Track;
class AudioChannel;

class MasteringCall
{
public:
	MasteringCall()
	{
		master=0;
		track=0;
		channel=0;
	//	device=false;
		rawbuffer=0;
		rawsize=0;
		iscleared=false;
		canceled=0;

#ifdef DEBUG
		counter=1;
#endif
	}

	void InitRawBuffer(int channels,int samples,int format);
	void Close(bool freememory);

	AudioHDFile savefile;

	void *rawbuffer;
	int rawsize;
	bool iscleared,canceled;

#ifdef DEBUG
	int counter; 
#endif;

	Mastering *master;
	Seq_Track *track;
	AudioChannel *channel;
	int format;
	//bool device;
};

#endif