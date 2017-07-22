#ifndef CAMX_AUDIORECORDTHREAD_H
#define CAMX_AUDIORECORDTHREAD_H 1

#include "defines.h"
#include "threads.h"

class AudioHDFile;

class AudioRecordThread:public Thread
{
	friend AudioRecordMainThread;

public:
	AudioRecordThread(AudioRecordMainThread *art)
	{
		threadid=0;
		endit=false;
		mainart=art;
	};

#ifdef WIN32
	static PTHREAD_START_ROUTINE AudioRecordingBufferThread(LPVOID pParam);
#endif

	int StartThread();

	AudioRecordMainThread *mainart;
	int threadid;
	bool endit;
};

class c_AudioHDFile:public Object
{
public:
	c_AudioHDFile(AudioHDFile *h){hdfile=h;}
	AudioHDFile *hdfile;
};

class AudioRecordMainThread:public Thread
{
public:
	AudioRecordMainThread(){for (int i=0;i<MAXCORES;i++)threads[i]=0;}

#ifdef WIN32
	static PTHREAD_START_ROUTINE AudioRecordMainThreadFunc(LPVOID pParam);
#endif

	void WriteRecordingFiles(bool singlethread);
	
	bool Init();
	void DeInit();
	void WaitLoop();

	AudioHDFile *GetAudioRecordingFile();

	OList c_recfiles; // c_AudioHDFile
	AudioRecordThread *threads[MAXCORES];
#ifdef WIN32
	CCriticalSection crecfiles_semaphore;
#endif
};
#endif