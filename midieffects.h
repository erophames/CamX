#ifndef CAMX_MIDIEFFECTS_H
#define CAMX_MIDIEFFECTS_H 1

#include "MIDIfilter.h"
#include "automation.h"

class MIDIPattern;
class Seq_Track;
class Seq_Event;
class camxFile;

class AT_MIDISYS_Channel:public MIDIAutomationObject
{
public:
	AT_MIDISYS_Channel()
	{
		id=AID_MIDICHANNEL;
		channel=0;
	}

	char *GetParmName(int index){return "MIDI Channel";}
	int channel;  // 0==thru, 1-16 channel
};

class AT_MIDISYS_Velocity:public MIDIAutomationObject
{
public:
	AT_MIDISYS_Velocity();

	char *GetParmName(int index){return "MIDI Velocity (Note)";}
	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);

	void CreateAutomationStartParameters(AutomationTrack *);
	void SendNewValue(AutomationTrack *);
	void ConvertValueToIntern();
	int GetVelocity(){return velocity;}

	char valuestring[16];
	int velocity;
};

class AT_MIDISYS_MVolume:public MIDIAutomationObject
{
public:
	AT_MIDISYS_MVolume();

	char *GetParmName(int index){return "MIDI Main Volume";}
	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);

	void CreateAutomationStartParameters(AutomationTrack *);
	void SendNewValue(AutomationTrack *);
	void ConvertValueToIntern();
	
	char valuestring[16];
	int velocity;
};

class AT_MIDISYS_Transpose:public MIDIAutomationObject
{
public:
	AT_MIDISYS_Transpose(){
		id=AID_MIDITRANSPOSE;
		transpose=0;
	}

	char *GetParmName(int index){return "MIDI Transpose";}
	int transpose;
};

class MIDIEffects
{
public:
	MIDIEffects();

	void Load(camxFile *);
	void Save(camxFile *);
	bool Compare(MIDIEffects *);
	void Clone(MIDIEffects *);

	void SetMIDIChannel(int);
	void SetVelocity(int);
	void SetTranspose(int);

	int GetChannel(){return MIDI_channel.channel;}
	int GetVelocity(){return MIDI_velocity.velocity;}
	int GetTranspose(){return MIDI_transpose.transpose;}

	MIDIFilter filter;

	AT_MIDISYS_Channel MIDI_channel;
	AT_MIDISYS_Transpose MIDI_transpose;
	AT_MIDISYS_Velocity MIDI_velocity;
	AT_MIDISYS_MVolume MIDI_mvolume;

	Seq_Track *track;
	MIDIPattern *pattern;

	bool MIDIbank_progselectsend,mute;
};
#endif