#ifndef CAMX_MIDIHARDWARE
#define CAMX_MIDIHARDWARE 1

#ifdef WIN32
#include <AFXMT.h>
#include <mmsystem.h>
#endif

#define MAXMIDIPORTS 32

#include "defines.h"
#include "object.h"
#include "MIDImonitor.h"
#include "inputdata.h"
#include "MIDIfilter.h"

class Seq_Song;
class Seq_Track;
class Seq_Pattern;
class MIDIPattern;
class MIDIOutputDevice;
class MIDIInputDevice;
class InsertAudioEffect;
class Groove;
class Processor;
class Seq_Event;
class VSTPlugin;
class NewEventData;

#define SENDONLYNOTEOFFS 1

class mainMIDIRecord
{
public:
	void AddMIDIRecordEvent(NewEventData *);

	virtual void Lock()
	{
#ifdef WIN32
		lock_semaphore.Lock();
#endif
	}

	virtual void UnLock()
	{
#ifdef WIN32
		lock_semaphore.Unlock();
#endif
	}

	inline NewEventData *FirstMIDIRecordData(){return (NewEventData *)newevents.GetRoot(); }
	inline NewEventData *DeleteMIDIRecordData(NewEventData *data)
	{
		if(data->data)delete data->data; // SysEx ...
		return (NewEventData *)newevents.RemoveO(data);
	}

	OList newevents;

private:
#ifdef WIN32
	CCriticalSection lock_semaphore;
#endif
};

#define RECEIVEMIDISTART_OFF 0
#define RECEIVEMIDISTART_PLAYBACK 1
#define RECEIVEMIDISTART_RECORD 2
#define RECEIVEMIDISTART_RECORD_NOPRECOUNTER 3

class MIDIOutputPort
{
public:
	MIDIOutputPort();	
	~MIDIOutputPort();

	void SetDevice(MIDIOutputDevice *,bool user);
	void SetInfo(char *);
	char *GetName();

	MIDIFilter filter;

	MIDIOutputDevice *outputdevice;
	char *odevicename,*fullname; // User Info etc... DX7...
	char info[32];
	bool autoset,visible,sendsync,sendmtc;
};

class MIDIInPort
{
public:
	MIDIInPort();
	~MIDIInPort()
	{
		if(idevicename)
			delete idevicename;

		if(fullname)
			delete fullname;
	}

	void SetDevice(MIDIInputDevice *,bool user);
	void SetInfo(char *);

	char *GetName();

	MIDIFilter filter;

	MIDIInputDevice *inputdevice;
	char *idevicename,*fullname;
	char info[32]; // User Info etc... DX7...
	bool autoset,visible,receivesync,receivemtc;
};

class mainMIDIBase
{
public:
	mainMIDIBase();

	void LoadPorts(camxFile *);
	void SavePorts(camxFile *);
	void Init();
	void DeInit();

	void SetReceiveMIDIStart(int flag);
	bool FindOpenNote(Seq_Song *,int key);
	bool CheckIfSysexActive();
	void ChangeMIDIInputDeviceUserInfo(MIDIInputDevice *,char *);
	void ChangeMIDIOutputDeviceUserInfo(MIDIOutputDevice *,char *);

	int LoadGrooves(camxFile *);
	void SaveGrooves(camxFile *);

	void LoadDevices(camxFile *);
	void SaveDevices(camxFile *);

	void RefreshMIDIDevices(); // Realtime

	// --- Output
	MIDIOutputDevice *InitOutputDevice(char *name,int id);
	MIDIOutputDevice *DeleteOutputDevice(MIDIOutputDevice *);
	void CollectMIDIOutputDevices();
	void RemoveMIDIOutputDevices();
	int GetNrMIDIOutputDevices(){return outputdevices.GetCount();}

	MIDIOutputDevice *lastsendevent_device;
	MIDIOutputDevice* FirstMIDIOutputDevice(){return (MIDIOutputDevice *)outputdevices.GetRoot(); }
	MIDIOutputDevice *FindMIDIOutputDevice(char *);
	MIDIOutputDevice *GetMIDIOutputDevice(int index){return (MIDIOutputDevice *)outputdevices.GetO(index);}

	int GetDefaultDevice(){return 0;}
	void SetDefaultDevice(MIDIOutputDevice *d){defaultMIDIOutputDevice=d;}
	void InitDefaultPorts();

	void CheckNewEventData(Seq_Song *,NewEventData *,Seq_Event *);

	void StopAllofTrack(Seq_Track *); // after mute
	void StopAllofPattern(Seq_Song *,Seq_Pattern *);
	void StopAllofEvent(Seq_Song *,Seq_Event *);

	void SendResetToAllDevices(int flag);
	void SendPanic();
	void SendSongPosition(Seq_Song *,OSTART position,bool force);
	void SendMIDIStop(Seq_Song *,int flag);

	// --- Input
	MIDIInputDevice *GetMIDIInputDevice(int index){return (MIDIInputDevice *)inputdevices.GetO(index);}
	MIDIInputDevice *AddInputDevice(char *name,int id);
	MIDIInputDevice *DeleteInputDevice(MIDIInputDevice *);

	void CollectMIDIInputDevices();
	void RemoveMIDIInputDevices();
	void StopMIDIInputDevices();

	void ResetMIDIInputTimer();
	void ResetMIDIInTimer(bool startrecordtimer);

	MIDIInputDevice* FirstMIDIInputDevice(){return (MIDIInputDevice *)inputdevices.GetRoot(); }
	MIDIInputDevice* FindMIDIInputDevice(char *);

	char *GetSysExString(UBYTE *data,int length); // SysEx Data->string

	// --- Groove
	void RefreshGrooveGUI(Groove *);
	Groove* FirstGroove(){return (Groove *)grooves.GetRoot(); }
	int GetNrGrooves(){return grooves.GetCount();}

	Groove* AddGroove(OSTART rastersteps,int rasterid);
	Groove* AddGroove(Groove *);
	Groove* RemoveGroove(Groove *);
	void DeleteGroove(Groove *); //+ GUI refresh

	void RemoveAllGrooves();
	Groove *GetGrooveIndex(int index){return (Groove *)grooves.GetO(index);}

	// Help Tools
	bool CheckIfChannelModeMessage(UBYTE status,UBYTE byte1);

	// Processor
	Processor* FirstProcessor() { return (Processor *)processors.GetRoot(); }
	Processor* RemoveProcessor(Processor *);
	void DeleteProcessor(Processor *); //+ GUI refresh

	void RemoveAllProcessor();
	void SetDefaultMIDIDevice(char *name);

	// Drummaps
	void CollectDrumsMaps();

	MIDIOutputPort MIDIoutports[MAXMIDIPORTS];
	MIDIInPort MIDIinports[MAXMIDIPORTS];

	MIDIOutputDevice *defaultMIDIOutputDevice;
	MIDIInputDevice *keyboard_inputdevice,*generator_inputdevice;
	int MIDI_inputimpulse,MIDI_outputimpulse,baudrate,receiveMIDIstart;

	bool init,MIDIthru,MIDIthru_notes,MIDIthru_polypress,MIDIthru_controlchange,
		MIDIthru_programchange,MIDIthru_channelpress,MIDIthru_pitchbend,MIDIthru_sysex,
		MIDIoutactive,MIDIinactive,sendnoteprevcylce,sendcontrolsolomute,receiveMIDIstop,
		quantizesongpositiontoMIDIclock;

private:
	OList grooves,outputdevices,inputdevices,processors,drummaps;
};
#endif

