#ifndef CAMX_AUDIOTHREADS 
#define CAMX_AUDIOTHREADS 1

#include "threads.h"

class AudioDeviceDeviceOutThread:public Thread
{
public:
	AudioDeviceDeviceOutThread(){hWait=0;}

	int StartThread();
	void WaitLoop();

#ifdef WIN32
	static PTHREAD_START_ROUTINE Func (LPVOID pParam);
#endif

	inline void SetSignal(){SetEvent(hWait);}
	inline void WaitSignal(DWORD ms=0){WaitForSingleObject(hWait, INFINITE);}

	void StopThread();
	HANDLE hWait; 
};

class AudioDeviceDeviceInThread:public AudioDeviceDeviceOutThread
{
public:
	int StartThread();
	void WaitLoop();

#ifdef WIN32
	static PTHREAD_START_ROUTINE Func (LPVOID pParam);
#endif
};

#endif