#ifndef CAMX_mainMIDIthruthread_H
#define CAMX_mainMIDIthruthread_H 1

#include "threads.h"
#include "object.h"
#include "objectevent.h"

class NewEventData;
class Seq_Track;
class MIDIOutputDevice;
class MIDIInputDevice;
class InsertAudioEffect;
class OpenNoteFilter;

class MIDIThruOff:public Object
{
	friend class MIDIThruThread;

public:
	MIDIThruOff()
	{
		outdev=0;
		outaudio=0;
		velocityoff=0;
	}

	MIDIThruOff *NextThruOff() {return (MIDIThruOff *)next;}
	Seq_Event *FirstThruEvent(){return (Seq_Event *)events.GetRoot();}

	OList events; // Added Events

	MIDIOutputDevice *outdev;
	InsertAudioEffect *outaudio;
	MIDIInputDevice *indev;
	Seq_Track *track;
	
	OSTART start;

	// Input
	UBYTE status,key,
	// Output
	outstatus,outkey,velocityoff;
};

#define THRUINDEX_MIDI 0
#define THRUINDEX_PLUGIN 1
#define THRUINDEX_MAX 2

class MIDIThruThread:public Thread
{
public:
	MIDIThruThread()
	{
		for(int i=0;i<THRUINDEX_MAX;i++)
		{
			processornote[i].ostart=
				processorpitch[i].ostart=
				processorcontrol[i].ostart=
				processorprogram[i].ostart=
				processorcpressure[i].ostart=
				processorppressure[i].ostart=0; // Thru Events, Start always 0

			processornote[i].pattern=
				processorpitch[i].pattern=
				processorcontrol[i].pattern=
				processorprogram[i].pattern=
				processorcpressure[i].pattern=
				processorppressure[i].pattern=0; // Thru Events, pattern always 0
		}
	}

	int StartThread();
	void WaitLoop();

#ifdef WIN32
	static PTHREAD_START_ROUTINE MIDIThruThreadFunc (LPVOID pParam);
#endif

	void CheckNewEvent(Seq_Song *,NewEventData *,int index);
	void ExecuteThruEvent(Seq_Song *,NewEventData *,Seq_Track *intrack,Seq_Event *);
	void DeleteThruOffs(Seq_Track *,MIDIInputDevice *,UBYTE status,UBYTE key,UBYTE veloff);

	void LockThruOffs(){sema_thruoff.Lock();}
	void UnLockThruOffs(){sema_thruoff.Unlock();}

	bool FindOpenKey(int key,OpenNoteFilter *filter=0);
	MIDIThruOff *AddThruOff(MIDIOutputDevice *,MIDIInputDevice *,Seq_Track *,OSTART start,UBYTE status,UBYTE key,UBYTE outstatus,UBYTE outkey);
	MIDIThruOff *AddThruOffToPlugin(InsertAudioEffect *,MIDIInputDevice *,Seq_Track *,OSTART start,UBYTE status,UBYTE key,UBYTE outstatus,UBYTE outkey);
	int RemoveOffsFromEffect(InsertAudioEffect *);
	MIDIThruOff* FirstThruOff(){return (MIDIThruOff *)offs.GetRoot(); }
	MIDIThruOff* DeleteThruOff(MIDIThruOff *,bool send);

	void DoThruOff(MIDIThruOff *); // Send Note Off MIDI/and or AUDIO
	void DeleteAllThruOff();
	void DeleteAllThruOffsTrack(Seq_Track *);

	OList thruevents,offs;

#ifdef WIN32
	void LockInput(){sema_input.Lock();}
	void UnlockInput(){sema_input.Unlock();}
	CCriticalSection sema_input,sema_thruoff;
#endif

	Note processornote[THRUINDEX_MAX];
	Pitchbend processorpitch[THRUINDEX_MAX];
	ControlChange processorcontrol[THRUINDEX_MAX];
	ProgramChange processorprogram[THRUINDEX_MAX];
	ChannelPressure processorcpressure[THRUINDEX_MAX];
	PolyPressure processorppressure[THRUINDEX_MAX];
};

#endif