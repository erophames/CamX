#ifndef CAMX_THREAD_H
#define CAMX_THREAD_H 1

#ifdef WIN32
#include <AFXMT.h> // Semaphores
#endif

class Thread
{
public:
	Thread(){
#ifdef WIN32
		ThreadHandle=0;
#endif
		exithread=false;
	}

#ifdef WIN32
	inline void SetSignal(){waitsignal.SetEvent();}
	inline void WaitSignal(int ms=0){WaitForSingleObject(waitsignal, ms<=0?INFINITE:ms);} // Buffer Signal
	inline void Lock(){semaphore.Lock();}
	inline void Unlock(){semaphore.Unlock();}

	bool IsExit(){return exithread;}
	void ThreadGone();
	void SendQuit();
	virtual void StopThread(){SendQuit();}
	
	CCriticalSection semaphore;
	CEvent waitsignal,gone;
	HANDLE ThreadHandle;
	bool exithread;
#endif
};

#endif