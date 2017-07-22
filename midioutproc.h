#ifndef CAMX_MIDIOUTPROC_H
#define CAMX_MIDIOUTPROC_H 1

#include "defines.h"
#include "threads.h"

class AudioDevice;
class Control_InternStartMIDIProc;
class Proc_Alarm;
class MIDIPlugin;
class Seq_Event;
class MIDIOutputDevice;

class MIDIProcessorProc:public Thread // MIDI Processor Thread
{
public:
	int StartThread();
	void StopThread();

#ifdef WIN32
	static PTHREAD_START_ROUTINE mainMIDIalarmthreadessorThreadFunc (LPVOID pParam);
#endif

	void AddAudioAlarms(AudioDevice *); // Audio Event Alarms

	// Raw Alarms
	Proc_Alarm *FirstRAWAlarm(){return (Proc_Alarm *)rawalarms.GetRoot();}
	Proc_Alarm *DeleteRAWAlarm(Proc_Alarm *alarm);

	// Real Alarms
	Proc_Alarm *FirstAlarm(){return (Proc_Alarm *)alarms.GetRoot();}

	bool AddAlarm(Proc_Alarm *,Seq_Event *,Proc_Alarm *calledbyalarm);
	void InitAlarm(Proc_Alarm *,Seq_Event *,Proc_Alarm *calledbyalarm); 
	Proc_Alarm *RemoveAlarm(Proc_Alarm *);
	void DeleteAllAlarms(MIDIPlugin *,Object *);

private:
	OListStart alarms, //Proc_Alarm
		rawalarms;
};

#define MAXMIDIOUTDEVICETHREADS 8

class c_MIDIOutDevice:public Object
{
public:
	c_MIDIOutDevice(MIDIOutputDevice *d){device=d;}
	MIDIOutputDevice *device;
};

class MIDIOutDeviceChildThread:public Thread
{
	friend class MIDIOutProc;

public:
	MIDIOutDeviceChildThread(MIDIOutProc *mp){MIDIproc=mp;threadid=0;}

#ifdef WIN32
	static PTHREAD_START_ROUTINE MIDIOutDeviceChildThread_Func(LPVOID pParam);
#endif

	int StartThread();
	void WaitLoop();

	MIDIOutProc *MIDIproc;
	int threadid;	
};

class MIDIOutProc:public Thread // MIDI Out Device RingBuffer Thread
{
public:
	MIDIOutProc();
	int StartThread();
	void WaitLoop();

	void Init();
	void DeInit();
	MIDIOutputDevice *GetMIDIOutputDevice();

	OList c_outdevices; // c_MIDIOutDevice
	MIDIOutDeviceChildThread *devicechildthreads[MAXMIDIOUTDEVICETHREADS];
	HANDLE coredone[MAXMIDIOUTDEVICETHREADS];
	
#ifdef WIN32
	static PTHREAD_START_ROUTINE MIDIOutThreadFunc(LPVOID pParam);
	CCriticalSection c_outdevices_sema;
#endif

	int devinit;
};

class MIDIPlaybackThread:public Thread // MIDI Alarm Thread no MIDI Output
{
public:
	int StartThread();
	void StopThread();

#ifdef WIN32
	static PTHREAD_START_ROUTINE MIDIPlaybackThreadFunc (LPVOID pParam); // Playback Main
#endif

	void WaitLoop();
};

class MIDIRTEProc:public Thread // MIDI RTE Alarm Thread no MIDI Output
{
public:
	int StartThread();
	void WaitLoop();
	void StopThread();

#ifdef WIN32
	static PTHREAD_START_ROUTINE MIDIRTEAlarmThreadFunc (LPVOID pParam); // Realtime Events, Note Offs etc...
#endif
};

class MIDIMTCThread:public Thread
{
public:
	MIDIMTCThread()
	{
		startmtc=false;
	}

	int StartThread();
	void WaitLoop();
	bool startmtc;

#ifdef WIN32
	static PTHREAD_START_ROUTINE MIDIOutMTCFunc (LPVOID pParam);
#endif
};

class MIDIStartThread:public Thread
{
public:

	int StartThread();
	void WaitLoop();

#ifdef WIN32
	static PTHREAD_START_ROUTINE MIDIStartFunc (LPVOID pParam);
#endif
};
#endif