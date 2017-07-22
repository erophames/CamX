#ifndef CAMX_AUDIOIOFX
#define CAMX_AUDIOIOFX 1

#include "object.h"
#include "audioeffects.h"
#include "peaks.h"

class AudioSend;
class AudioPort;
class AudioSystem;

extern int channelschannelsnumber[];

class AudioIOFX
{
public:
	AudioIOFX();

	void FreeMemory();
	void Load(camxFile *);
	void Save(camxFile *);
	void Repair();
	bool Compare(AudioIOFX *);
	void RefreshEffects(AudioDevice *);
	void DeleteAllEffectBuffers();
	void ResetPlugins();
	void RefreshDo();
	void PreRefreshDo();

	void InitIOOldChannel();

	// Sends
	void LoadSend(camxFile *);
	void SaveSend(camxFile *);

	bool CheckSendPre();
	bool CheckSend();
	AudioSend *FirstSend();
	int GetCountSends(){return sends.GetCount();}
	AudioSend *FindSend(AudioChannel *bus);
	bool AddSend(AudioChannel *);
	void AddSend(AudioSend *,int index);

	void DeleteSend(AudioSend *,bool deleteall);
	void SetPrePost();
	void DoAudioChannelSends(AudioEffectParameter *,bool post); // Post

	void SetOutput(AudioPort *);
	void SetInput(AudioPort *);

	void SetChannelType(int type);
	int GetChannels(){return channelschannelsnumber[channel_type];}
	void SetPlaybackChannel(AudioPort *,bool dontchangechannelchannels,bool lock);

	void Clone(AudioIOFX *);
	void SetFXBypass(bool onooff);
	void SetInputFXBypass(bool onoff);

	void ActivateInputs();

	Peak inputpeaks;
	OList sends;
	AudioEffects audioeffects,audioinputeffects;
	AudioSystem *audiosystem;
	AudioPort *in_vchannel,*out_vchannel;
	
	int invchannel_index,invchannel_channels, // Input
		outvchannel_index,outvchannel_channels, // Output
		channel_type; 

	int inchannelindex[MAXCHANNELSPERCHANNEL],outchannelindex[MAXCHANNELSPERCHANNEL];

	bool bypassallfx,bypassallinputfx,bypass_input,thru,skipthru,tempinputmonitoring,inputmonitoring,audioinputenable;
};
#endif