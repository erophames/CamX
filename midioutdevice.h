#ifndef CAMX_MIDIOUTDEVICE
#define CAMX_MIDIOUTDEVICE 1

#include "object.h"
#include "MIDIfilter.h"
#include "MIDIoutfilter.h"
#include "MIDIhardware.h"

#ifdef WIN32
#include <mmsystem.h> // HMIDIOUT
#endif

#include "MIDIquarterframe.h"
#include "lastMIDIevent.h"

class Seq_Track;
class guiWindow;

#define DATABUFFERSIZE 2048

class MIDIOutputProgram
{
public:
	MIDIOutputProgram()
	{
		channel=0; // 0-15
		MIDIProgram=MIDIBank_lsb=MIDIBank_msb=0;
		useprogramchange=usebank_msb=usebank_lsb=false;
	}

	void Load(camxFile *);
	void Save(camxFile *);

	void Clone(MIDIOutputProgram *);
	bool Compare(MIDIOutputProgram *);

	int channel,MIDIProgram,MIDIBank_lsb,MIDIBank_msb; //0-127
	bool useprogramchange,usebank_lsb,usebank_msb;
};

class MIDIOutputDevice:public Object
{
	friend class mainMIDIBase;

public:
	MIDIOutputDevice();

	void CreateFullName();

	char *FullName()
	{
		if(!fullname)CreateFullName();
		return fullname;
	}

	MIDIOutputDevice *NextOutputDevice() { return (MIDIOutputDevice *)next;}
	MIDIOutputDevice *PrevOutputDevice() { return (MIDIOutputDevice *)prev;}

	void SendDeviceReset(bool force);

	int OpenOutputDevice(int mode);
	void CloseOutputDevice();

	void SendOutput();

	void sendSmallData(UBYTE *data,int length);
	void sendData_NL(UBYTE *data,int length);
	void sendSmallDataRealtime(UBYTE byte);
	void sendBigData(UBYTE *data,int length);

	void sendData(UBYTE *data,int length){sendSmallData(data,length);}
	void sendNote(UBYTE status,UBYTE key,UBYTE velo);
	void sendNoteOff(UBYTE status,UBYTE key,UBYTE velooff);
	void sendPolyPressure(UBYTE status,UBYTE key,UBYTE pressure);
	void sendControlChange(UBYTE status,UBYTE controller,UBYTE value);
	void sendPitchbend(UBYTE status,UBYTE lsb,UBYTE msb);
	void sendProgramChange(UBYTE status,UBYTE program);
	void sendChannelPressure(UBYTE status,UBYTE pressure);
	void sendSysEx(UBYTE *data,int length);

	void sendRealtimeMessage(UBYTE byte,int counter)
	{
		while(counter--)sendSmallDataRealtime(byte);
	}

	void sendSongPosition(OSTART ticks);
	void sendSongPosition_MTCRealtime(Seq_Song *);

	void LockDevice(){
#ifdef WIN32
		lock_semaphore.Lock();
#endif
	}

	void UnlockDevice(){
#ifdef WIN32
		lock_semaphore.Unlock();
#endif
	}

	void UnlockAndSend();

#ifdef WIN32
	CCriticalSection lock_semaphore;
#endif


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

	char userinfo[33];
	MIDIFilter device_eventfilter; // Channel/Event Filter
	MTC_Quarterframe mtcqf;
	LMIDIEvents monitor_events[MAXMONITOREVENTS];

	// SysEx 
	char *datasysex[DATABUFFERSIZE];
	int datasysexlen[DATABUFFERSIZE];

	// Standard Data
#ifdef WIN32
	union { 
		DWORD dwData; 
		BYTE bData[4]; 
	} databuffer[DATABUFFERSIZE]; 

	HMIDIOUT hMIDIOut;
#endif

#ifdef DEBUG
	OSTART datatime[DATABUFFERSIZE];
#endif

	char *fullname,*initname,*name;
	int synccounter,monitor_syscounter,monitor_eventcounter,id,init,datareadbuffercounter,datawritebuffercounter; // Data Ring Buffer
	bool displaynoteoff_monitor,lockMIDIclock,lockMIDIstartstop,lockMIDIsongposition,lockmtc;
};

class Seq_Group_MIDIOutPointer:public Object
{
public:
	Seq_Group_MIDIOutPointer(){portindex=0;}
	MIDIOutputDevice *GetDevice(){return mainMIDI->MIDIoutports[portindex].outputdevice;}
	Seq_Group_MIDIOutPointer *NextGroup(){return (Seq_Group_MIDIOutPointer *)next;}

	int portindex;
};

class Seq_Group_MIDIOutputDevice
{
public:
	Seq_Group_MIDIOutPointer *FirstDevice(){return (Seq_Group_MIDIOutPointer *)groups.GetRoot();}
	Seq_Group_MIDIOutPointer *LastDevice(){return (Seq_Group_MIDIOutPointer *)groups.Getc_end();}
	int GetCountGroups(){return groups.GetCount();}

	Seq_Group_MIDIOutPointer *AddToGroup(int portindex);
	void RemoveBusFromGroup(int portindex);
	void Replace(int oldindex,int newindex);
	void Delete();
	bool FindPort(int index);
	void CloneToGroup(Seq_Group_MIDIOutputDevice *);
	bool CompareWithGroup(Seq_Group_MIDIOutputDevice *);

	OList groups;
	Seq_Track *track; // for undo/redo
};
#endif