#ifndef CAMX_RUNNINGAUDIOFILE
#define CAMX_RUNNINGAUDIOFILE 1

#include "object.h"
#include "audiohardwarebuffer.h"
#include "audioeffectparameter.h"

class AudioDevice;
class AudioPattern;
class AudioChannel;
class AudioObject;
class RunningAudioFile;
class Seq_Song;
class Seq_Track;
class AudioRegion;

class RunningAudioFile:public Object
{
	friend class mainAudio;
	
public:
	RunningAudioFile()
	{
		streamstartsampleoffset=0;
		closeit=false;
		effectparameter.out=&audiobuffer;
	}
	
	void ReadRAF(AudioDevice *);
	bool InitBuffer(AudioDevice *device,int flag);
	
	RunningAudioFile *NextRunningFile(){return ( RunningAudioFile *)next;}

	AudioHardwareBuffer audiobuffer;
	
	// Effect Parameter, Automation etc...
	AudioEffectParameter effectparameter;

	Seq_Song *song;
	Seq_Track *track;
	AudioPattern *audiopattern;

	// Region
	AudioRegion *audioregion;
	LONGLONG regionstart,regionend,xinitstart,xinitend; // position or NULL
	LONGLONG samplestart,sampleend;
	int bytesread,streamstartsampleoffset; // start offset
	bool closeit;

private:
	void AddCrossFadesTobuffer(Seq_Song *,AudioPattern *);
};
#endif