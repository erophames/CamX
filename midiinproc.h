#ifndef CAMX_MIDIINPROC_H
#define CAMX_MIDIINPROC_H 1

#include "defines.h"
#include "threads.h"

class Seq_Song;
class NewEventData;
class AudioObject;

// Incoming MIDI Events
class MIDIInProc:public Thread
{
public:	
	int StartThread();
	void WaitLoop();

#ifdef WIN32
	static PTHREAD_START_ROUTINE MIDIInThreadFunc (LPVOID pParam);
#endif
};

// Incoming VST Events
class PluginMIDIInputProc:public Thread
{
public:	
	int StartThread();
	void WaitLoop();

#ifdef WIN32
	static PTHREAD_START_ROUTINE PluginInputFunc (LPVOID pParam);
#endif

	// VST Input
	void LockPluginInput(){sema_vstinputtrigger.Lock();}
	void UnLockPluginInput(){sema_vstinputtrigger.Unlock();}
	void AddNewInputData(Seq_Song *,NewEventData *);
	void SendForce(Seq_Song *,AudioObject *);

	NewEventData *FirstEvent(){return (NewEventData *)vstinputevents.GetRoot();}
	NewEventData *FirstSleepEvent(){return (NewEventData *)sleepevents.GetRoot();}

	CCriticalSection sema_vstinputtrigger;

private:
	void CheckSleepAlarms_SysTime(Seq_Song *);
	OList sleepevents,vstinputevents;
};

#endif