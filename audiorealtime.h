#ifndef CAMX_AUDIOREALTIME
#define CAMX_AUDIOREALTIME 1

#include "object.h"
#include "audiopattern.h"
#include "audiohardwarebuffer.h"
#include "threads.h"

#define ART_FILE 0
#define ART_SAMPLES 1

class AudioChannel;
class AudioHardwareChannel;

#define AR_STOPPED 0
#define AR_STARTED 1

class AudioRealtimeThread:public Thread
{
public:
	AudioRealtimeThread()
	{
		threadid=0;
	};
#ifdef WIN32
	static PTHREAD_START_ROUTINE AudioRealtimeThreadFunc(LPVOID pParam);
#endif

	int StartThread();
	void StopThread();
	void WaitLoop();

	int threadid;
};

class AudioRealtime:public Object
{
	friend class AudioChannel;
	
public:
	AudioRealtime(){stream=0;addedtolist=closeit=false;}
	AudioRealtime(Seq_Song *,TrackHead *,AudioHDFile *,AudioRegion *,bool aclose,int offset=0);
	
	void InitClass(Seq_Song *,TrackHead *,AudioHDFile *,AudioRegion *,bool aclose,int off);

	void Init(AudioDevice *,AudioHDFile *);
	void DeInit(bool signal,bool freememory); 

	void SeekRealtime();
	void ReadRealtime();

	AudioRealtime *NextAudioRealtime() {return (AudioRealtime *)next;}

	// File -----
	AudioPattern audiopattern;
	AudioHardwareBuffer audiobuffer,*stream;
	TrackHead *audiochannel;
	Seq_Song *song;
	Seq_Track *usetrack;
	guiWindow *startwindow;

	// Samples -----
	char *ramposition; // AUDIO RAM FILE
	int offset,rbytes,type,nrstream,streamreadc,streamwritec; // Sync Counter Refill
	bool endreached,*endstatus,autoclose,staticrealtime,addedtolist,closeit;	
};
#endif