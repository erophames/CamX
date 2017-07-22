#ifndef CAMX_AUDIOCHANNEL
#define CAMX_AUDIOCHANNEL 1

#include "object.h"
#include "object_trackhead.h"
#include "audiodefines.h"
#include "audioeffects.h"
#include "colourrequester.h"
#include "audioiofx.h"
#include "audiogroups.h"
#include "audiochanneltypes.h"
#include "audioeffectparameter.h"

class AudioRealtime;
class AddRealtimeLatency;
class AudioPattern;
class AudioChannel;
class AudioChannelMix;
class AudioHDFile;
class AudioRegion;
class MIDIProcessor;
class AudioEffectParameter;
class AudioHardwareBuffer;
class AudioHardwareChannel;
class guiMenu;

enum ChannelChannels
{
	CHANNELTYPE_MONO, //1
	CHANNELTYPE_STEREO, //2
	CHANNELTYPE_SURROUND, //3
	CHANNELTYPE_QUADRO, //4
	CHANNELTYPE_PROLOGIC, //5
	CHANNELTYPE_PROLOGIC2, //6
	CHANNELTYPE_DOLBYDIGITAL, //7
	CHANNELTYPE_DOLBYDIGITALTRUEHD, //8
};

class AudioChannel:public Object,public TrackHead
{
	friend class AudioSystem;

public:
	enum ChannelFlags{
		CHANNEL_NORECORD=1,
		CHANNEL_NOSOLO=2,
		CHANNEL_NOHARDWARE=4
	};

	AudioChannel(){id=OBJ_AUDIOCHANNEL;channel=this;}
	AudioChannel(AudioSystem *,int stype);
	void Init(AudioSystem *,int stype);

	bool SetVType(AudioDevice *,int type,bool guirefresh,bool allselected); // Virtual Type
	bool ConnectTOVChannel(AudioPort *,bool out); // -> Virtual Channel

	guiMenu *CreateChannelOutputMenu(guiMenu *);
	
	// Output Connect
	void SetRecordChannel(AudioPort *,bool lock=true);
	void SetAudioInBypass(bool by);
	int GetSoloStatus();
	void AddEffectToChannel(AudioDevice *); // Core Function
	void SetName(char *);
	bool Muted(bool solocheck=true);
	void CloneToChannel(AudioChannel *);
	void CloneFxToChannel(AudioChannel *);
	void Load(camxFile *);
	void Save(camxFile *);	
	void CreateChannelBuffers(AudioDevice *);
	void DeleteChannelBuffersMix();
	void ResetAudioChannelBuffers();
	void ToggleMute();

	AudioChannel *NextOrPrevChannel()
	{
		if(NextChannel())return NextChannel();
		return PrevChannel();
	}

	inline AudioChannel *NextChannel() {return (AudioChannel *)next;}
	inline AudioChannel *PrevChannel() {return (AudioChannel *)prev;}

	virtual void FreeAudioChannelMemory();

	AudioPort *GetVOut();
	AudioPort *GetVIn(){return io.in_vchannel;}

	void SetSolo(bool solostatus,bool lock);
	
	char *CreateFullName();
	char *GetFullName()
	{
		if(!fullname){
			if(CreateFullName())return fullname;
			return name;
		}

		return fullname;
	}
	char *GetName(){return name;}
	void ShowChannelName(guiWindow *nostringrefresh);

	int GetBgColour();
	int GetChannelIndex(){return index;}

#ifdef WIN32
	inline void FuncMixLock(){func_mixlock.Lock();}
	inline void FuncMixUnLock(){func_mixlock.Unlock();}
	CCriticalSection func_mixlock;
#endif

	Colour colour;

	char name[SMALLSTRINGLEN+1],*fullname;
	
	AudioSystem *audiosystem;
	AudioChannel *nextcorechannel,*nextcorebus;
	AudioHardwareChannel *hardwarechannel; // Device I/O only

	void SetMIDIVelocity(int mv,OSTART automationtime);

	int audiochannelsystemtype,MIDIoutputimpulse;

private:
	AudioEffectParameter outpar;
	
};
#endif