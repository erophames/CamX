#ifndef CAMX_AUDIOOUTPROC_H
#define CAMX_AUDIOOUTPROC_H 1

#include "defines.h"
#include "threads.h"

#ifdef WIN32
#include <AFXMT.h>
#endif

enum{
	DOCORECHANNEL,
	DOCORETRACKS,
	DOCOREAUDIOTRACKTRACKIO,
	DOCORETRACKSINPUT,
	DOCORETRACKSINPUTEFFECTS,
	DOCORETRACKSINPUTPEAK,
	DOCORETRACKSTHRU,
	DOCOREBUS,
};

class AudioCoreAndStreamProc:public Thread
{
public:
	AudioCoreAndStreamProc(){
		raf=0;
		coresinit=0;

		for(int i=0;i<MAXCORES;i++){
			CoreThreadEventHandle[i]=CoreThreadHandle[i]=RAFThreadEventHandle[i]=RAFThreadHandle[i]=0;
			coreexit[i]=rafcoreexit[i]=false;
		}

		timeforrefill_maxms=0;
		timeforrefill_ms=0;
	}

#ifdef WIN32
	int StartThread();
	void StopThread();
	void WaitLoop();

	// Main Core Do ##################
	void DoFunction(Seq_Song *,int func,int usethreads); // Audio Device Refill

	inline void WaitCoreSignal(int id){WaitForSingleObject(Wait_CoreThreadEventHandle[id], INFINITE);}

	inline void SetCoreSignals(int usethreads){
		usedthreads=usethreads>coresinit?coresinit:usethreads;
		for(int i=0;i<usedthreads;i++)SetEvent(Wait_CoreThreadEventHandle[i]); // corewait[i].SetEvent();
	}

	inline void DoneCore(int id){SetEvent(CoreThreadEventHandle[id]);}

	inline void WaitAllCoresFinished(){
		if(usedthreads){
			WaitForMultipleObjects( 
				usedthreads,           // number of objects in array
				CoreThreadEventHandle,     // array of objects
				TRUE,       // wait for any object
				INFINITE);       // five-second wait
		}
	}

	// RAFS ##################
	void DoRAFS(Seq_Song *,AudioDevice *); // RAF refill

	inline void SetRAFSignals(int usethreads){
		usedrafthreads=usethreads>coresinit?coresinit:usethreads;
		for(int i=0;i<usedrafthreads;i++)SetEvent(Wait_RAFThreadEventHandle[i]); //rafwait[i].SetEvent();
	}

	inline void WaitRAFSignal(int id){WaitForSingleObject(Wait_RAFThreadEventHandle[id], INFINITE);}
	inline void DoneRAF(int id){SetEvent(RAFThreadEventHandle[id]);}

	inline void WaitAllRAFsFinished(){
		if(usedrafthreads){
			DWORD dwEvent = WaitForMultipleObjects( 
				usedrafthreads,           // number of objects in array
				RAFThreadEventHandle,     // array of objects
				TRUE,       // wait for any object
				INFINITE);       // five-second wait
		}
	}

	inline void LockTimerCheck(){core_time.Lock();}
	inline void UnlockTimerCheck(){core_time.Unlock();}
	CCriticalSection core_time;

	Seq_Song *rafsong;
	RunningAudioFile *raf;
	AudioDevice *rafdevice;
	double timeforrefill_maxms,timeforrefill_ms;

	HANDLE Wait_CoreThreadEventHandle[MAXCORES],Wait_RAFThreadEventHandle[MAXCORES],
		CoreThreadEventHandle[MAXCORES],CoreThreadHandle[MAXCORES],
		RAFThreadEventHandle[MAXCORES],RAFThreadHandle[MAXCORES]; // Refill RAF's

	int docorefunction, // init by Flags
		coresinit,usedthreads,usedrafthreads;

	bool coreexit[MAXCORES],rafcoreexit[MAXCORES];
#endif
};

class AudioCoreAudioInputProc
{
public:
	AudioCoreAudioInputProc(){
		coresinit=0;
		for(int i=0;i<MAXCORES;i++){
			CoreThreadEventHandle[i]=CoreThreadHandle[i]=0;
			inputcoreexit[i]=false;
		}
	}

#ifdef WIN32
	void DoFunction(Seq_Song *,int func,int usethreads); // Audio Device Refill
	int StartThread();
	void StopThread();

	inline void WaitCoreSignal(int id){WaitForSingleObject(Wait_CoreThreadEventHandle[id], INFINITE);}
	inline void SetCoreSignals(int usethreads){
		usedthreads=usethreads>coresinit?coresinit:usethreads;
		for(int i=0;i<usedthreads;i++)SetEvent(Wait_CoreThreadEventHandle[i]); // corewait[i].SetEvent();
	}
	inline void DoneCore(int id){SetEvent(CoreThreadEventHandle[id]);}

	inline void WaitAllCoresFinished(){
		if(usedthreads){
			WaitForMultipleObjects( 
				usedthreads,           // number of objects in array
				CoreThreadEventHandle,     // array of objects
				TRUE,       // wait for any object
				INFINITE);       // five-second wait
		}
	}

	HANDLE Wait_CoreThreadEventHandle[MAXCORES],CoreThreadEventHandle[MAXCORES],CoreThreadHandle[MAXCORES]; // Refill Audio Device

	int docorefunction, // init by Flags
		coresinit,usedthreads,usedrafthreads;

	bool inputcoreexit[MAXCORES];
#endif
};

#endif