#ifndef CAMX_MIDIINDEVICE
#define CAMX_MIDIINDEVICE 1

#include "defines.h"

#ifdef WIN32
#include <mmsystem.h>
#endif

#include "MIDIfilter.h"
#include "MIDIquarterframe.h"
#include "lastMIDIevent.h"
#include "MIDIhardware.h"

class MIDIInBufferEvent // Buffer for short msgs
{
public:
	MIDIInBufferEvent(){data=0;length=0;}

	unsigned char *data;
	LONGLONG systime;
	OSTART songposition;
	int length;
	UBYTE status,bytes[3];
};

class MIDIInputDevice:public Object
{
	friend class mainMIDIBase;

public:
	enum{
		OS_MIDIINTERFACE,
		CAMX_INTERN
	};

	MIDIInputDevice();

	void AddMonitor(UBYTE status,UBYTE byte1,UBYTE byte2);

	void CreateFullName();
	char *FullName()
	{
		if(!fullname)CreateFullName();
		return fullname;
	}

	MIDIInputDevice *NextInputDevice() {return (MIDIInputDevice *)next;}
	MIDIInputDevice *PrevInputDevice() {return (MIDIInputDevice *)prev;}

	void OpenInputDevice();
	void CloseInputDevice();

	void StartInputTime();
	void StopMIDIDevice();

	void NewMIDIClock(Seq_Song *,NewEventData *);
	void NewMTCQF(Seq_Song *,NewEventData *);

	void ShortMIDIInputMessage(LONGLONG systime,DWORD data);
	void LongMIDIInputMessage(LONGLONG systime,unsigned char *data,int length,bool newsysex);

	void LockMonitor(){
#ifdef WIN32
		//	monitor_semaphore.Lock();
#endif
	}

	void UnlockMonitor(){
#ifdef WIN32
		//	monitor_semaphore.Unlock();
#endif
	}

#ifdef WIN32
	//CCriticalSection monitor_semaphore;
#endif

	void LockInputBufferCounter()
	{
#ifdef WIN32
		lockcounter.Lock();
#endif
	}

	void UnLockInputBufferCounter()
	{
#ifdef WIN32
		lockcounter.Unlock();
#endif
	}

#ifdef WIN32
	HMIDIIN hMIDIIn;
	MIDIHDR sysexin_MIDIHdr1,sysexin_MIDIHdr2;
	CCriticalSection lockcounter;
#endif

	LMIDIEvents monitor_events[MAXMONITOREVENTS];
	MTC_Quarterframe mtcinput;
	MIDIFilter inputfilter;
	MIDIInBufferEvent buffer[MIDIINPUTBUFFER];

	char *fullname,userinfo[33];
	char *name,*initname;

	UBYTE *incomingstream;
	unsigned char MIDIinbuf1[MIDIINBUFFERLEN],MIDIinbuf2[MIDIINBUFFERLEN];

	LONGLONG lastMIDIclock_systime;
	OSTART lastMIDIclock_songposition;

	double tempobuffer[8],MIDIclocktempo,minMIDIclockdifference;

	OSTART indesongposition,incomingstream_starttime;
	int type,inputwritebuffercounter,inputreadbuffercounter,MIDIclockraster,
		tempobuffercounter,inputMIDIclockcounter,synccounter,
		monitor_syscounter,monitor_eventcounter,
		incomingstreamlength;

	// MIDI Sync
	bool checksysex,newsongpositionset,
		displaynoteoff_monitor,deviceinit,foundrealtime;

private:
	int id;
	UBYTE runningstatus;
};

class Seq_Group_MIDIInPointer:public Object
{
public:
	Seq_Group_MIDIInPointer *NextGroup(){return (Seq_Group_MIDIInPointer *)next;}
	Seq_Group_MIDIInPointer(){portindex=0;}

	MIDIInputDevice *GetDevice(){return mainMIDI->MIDIinports[portindex].inputdevice;}
	int portindex;
};

class Seq_Group_MIDIInputDevice
{
public:
	Seq_Group_MIDIInPointer *FirstDevice(){return (Seq_Group_MIDIInPointer *)groups.GetRoot();}
	Seq_Group_MIDIInPointer *LastDevice(){return (Seq_Group_MIDIInPointer *)groups.Getc_end();}
	int GetCountGroups(){return groups.GetCount();}
	Seq_Group_MIDIInPointer *AddToGroup(int portindex);
	void RemoveBusFromGroup(MIDIInputDevice *);
	void Replace(int oldindex,int newindex);
	void Delete();
	bool FindPort(int index);
	void CloneToGroup(Seq_Group_MIDIInputDevice *);
	bool CompareWithGroup(Seq_Group_MIDIInputDevice *);

	OList groups; // Seq_Group_MIDIInPointer
};

#endif