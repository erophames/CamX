#ifndef CAMX_SEQREALTIME_H
#define CAMX_SEQREALTIME_H 1

#include "defines.h"
#include "objectevent.h"

class Seq_Song;
class AudioChannel; // Audio Output
class AudioObject;
class AudioEffects;
class MIDIOutputDevice; // MIDI Output
class MIDIPlugin;
class mempool_NoteOffRealtime;

#ifdef WIN32
#include <AFXMT.h> // Semaphores
#endif

#define REALTIMELIST_MIDI 0 // MIDI Device
#define REALTIMELIST_AUDIO 1 // AudioInstrument Trigger
#define REALTIME_LISTS 2 // last+1

enum RealtimeIDs
{
	REALTIMEID_UNKNOWN,
	REALTIMEID_NOTEOFFREALTIME,
	REALTIMEID_SUSTAIN_ON
};

class RealtimeEvent:public Object
{
	friend class Seq_Realtime;

public:

	enum RTEFlags
	{
		RTE_MIDIPatternSTART_EVENT=1,
	};

	RealtimeEvent();

	void Init(){
		rteflag=0;
		fromtrack=0;
		audiochanneloutput=0;
		audioobject=0; // audioinstrument
		outputdevice=0;
		MIDIplugin=0;
	}

	virtual Seq_Event *CreateRAWEvent(){return 0;}
	virtual int Callback(Seq_Song *){return 0;} // MIDI Proc Init etc..
	virtual void SendToMIDI(){} // -> MIDI
	virtual bool SendToAudio(AudioObject *,int offset){return false;} // -> AUDIO

#ifdef MEMPOOLS
	virtual void DeleteRealtimeEvent(){delete this;}
#endif

	void SendQuick();
	void SendToProcessor();
	RealtimeEvent *NextEvent(){return (RealtimeEvent*)next;}
	LONGLONG GetRealEventSampleStart(){return rte_callsampleposition;}

	AudioChannel *audiochanneloutput;
	AudioObject *audioobject;
	MIDIOutputDevice *outputdevice;
	Seq_Event *fromevent;
	MIDIPattern *rte_frompattern;
	Seq_Track *fromtrack;
	MIDIPlugin *MIDIplugin;
	LONGLONG rte_inittime,rte_calltime,audiodevicestartcounter,rte_callsampleposition,rte_callsystime;
	double rte_buffercallposition;
	OSTART rte_initposition; // Ticks
	int realtimeid,rteflag;
	UBYTE status;
	bool noteoff;
};

class NoteOff_Realtime:public RealtimeEvent
{
public:
	NoteOff_Realtime()
	{
		realtimeid=REALTIMEID_NOTEOFFREALTIME;
		noteoff=true;
	}

	Seq_Event *CreateRAWEvent();
	void SendToMIDI();
	bool SendToAudio(AudioObject *,int offset);
	void DeleteRealtimeEvent();

#ifdef MEMPOOLS
	mempool_NoteOffRealtime *pool;
#endif

	int iflag;
	char key,velocityoff;
};

class Control_SustainOn:public RealtimeEvent
{
public:
	Control_SustainOn(){realtimeid=REALTIMEID_SUSTAIN_ON;}
	void SendToMIDI();
	bool SendToAudio(AudioObject *,int offset);
	char sustain;
};

class Control_InternStartMIDIProc:public RealtimeEvent
{
public:	
	int Callback(Seq_Song *); // v
};

class Control_Realtime:public RealtimeEvent
{
public:	
	void SendToMIDI();
	bool SendToAudio(AudioObject *,int offset);
	char controller,value;
};

class Program_Realtime:public RealtimeEvent
{
public:	
	void SendToMIDI();
	bool SendToAudio(AudioObject *,int offset);
	char program;
};

class Seq_Realtime
{
	friend class Seq_Song;

public:	
	void BufferBeforeTempoChanges();
	void RefreshTempoBuffer();
	void RemoveRObject(Object *);

	RealtimeEvent* FirstREvent(){return(RealtimeEvent *)realtimeevents.GetRoot(); }
	void AddRealtimeAlarmEvent(RealtimeEvent *,LONGLONG callsampleposition);
	inline RealtimeEvent* DeleteREvent(RealtimeEvent *e)
	{
#ifdef MEMPOOLS
		RealtimeEvent *n=(RealtimeEvent *)realtimeevents.CutObject(e);
		e->DeleteRealtimeEvent();
#else
		RealtimeEvent *n=(RealtimeEvent *)realtimeevents.RemoveO(e);
#endif
		
		return n;
	}

	RealtimeEvent* FirstCDEvent(){return(RealtimeEvent *)realtimecdevents.GetRoot(); }
	void AddCDAlarmEvent(RealtimeEvent *,LONGLONG cdsamples);
	inline RealtimeEvent* DeleteCDEvent(RealtimeEvent *e)
	{
		#ifdef MEMPOOLS
		RealtimeEvent *n=(RealtimeEvent *)realtimecdevents.CutObject(e);
		e->DeleteRealtimeEvent();
#else
		RealtimeEvent *n=(RealtimeEvent *)realtimecdevents.RemoveO(e);
#endif

		return n;
	}

	void DeleteAllREvents(Seq_Song *,int flag);

	void AddWaitEvent(RealtimeEvent *);
	RealtimeEvent* FirstWEvent(){return(RealtimeEvent *)waitrealtimeevents.GetRoot(); }
	inline RealtimeEvent* DeleteWEvent(RealtimeEvent *e)
	{
#ifdef MEMPOOLS
		RealtimeEvent *n=(RealtimeEvent *)waitrealtimeevents.CutObject(e);
		e->DeleteRealtimeEvent();
#else
RealtimeEvent *n=(RealtimeEvent *)waitrealtimeevents.RemoveO(e);
#endif

		return n;
	}

#ifdef WIN32
	inline void Lock(){semaphore.Lock();}
	inline void UnLock(){semaphore.Unlock();}
	CCriticalSection semaphore;
#endif

private:
	OListStart realtimeevents,realtimecdevents,waitrealtimeevents; // waitrealtimeevents f.e. MIDI Control Sustain, AddRealtimeAlarmEvent pos=-1
	Seq_Song *song;
	int type;
};
#endif