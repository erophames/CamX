#ifndef CAMX_SEMAPHORES_H
#define CAMX_SEMAPHORES_H 1

enum{
	CS_audioinput,
	CS_audiorealtime,
	CS_mainMIDIalarmthreadessor,
	CS_audiowork,
	CS_audiopeakfile,
	CS_createpeakfile,
	CS_audioplayback,	
	CS_peakbuffer,
	CS_audiopeakfilecheck,
	CS_UI,
	CS_SONGCONTROL,

	CS_CRITICALSECTIONSEND
};

class ThreadControl
{
public:
	ThreadControl();
	~ThreadControl();

	void LockActiveSong();
	void UnlockActiveSong();

	inline void Lock(int cs_id){EnterCriticalSection(&cs[cs_id]);}
	inline void Unlock(int cs_id){LeaveCriticalSection(&cs[cs_id]);}

	inline void Lock(int cs_id,int cs_id2){EnterCriticalSection(&cs[cs_id]);EnterCriticalSection(&cs[cs_id2]);}
	inline void Unlock(int cs_id,int cs_id2){LeaveCriticalSection(&cs[cs_id]);LeaveCriticalSection(&cs[cs_id2]);}
	inline void Lock(int cs_id,int cs_id2,int cs_id3){EnterCriticalSection(&cs[cs_id]);EnterCriticalSection(&cs[cs_id2]);EnterCriticalSection(&cs[cs_id3]);}
	inline void Unlock(int cs_id,int cs_id2,int cs_id3){LeaveCriticalSection(&cs[cs_id]);LeaveCriticalSection(&cs[cs_id2]);LeaveCriticalSection(&cs[cs_id3]);}

private:
#ifdef WIN32
	CRITICAL_SECTION cs[CS_CRITICALSECTIONSEND];
#endif

};
#endif