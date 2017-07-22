#ifndef CAMX_AUDIOSYSTEM
#define CAMX_AUDIOSYSTEM 1

#define MAXPEAKDROPCOUNT 4

#include "defines.h"
#include "audiochannel.h"
#include "audiochanneltypes.h"

class AudioDevice; // IO
class InitPlay;

class AudioSystem
{
public:
	AudioSystem();

	enum{
		AUDIOCAMX_HASTHRU=1,
		AUDIOCAMX_HASTHRUTRACKS=4,
		AUDIOCAMX_HASMIDIINPUTTRACKS=8,
		AUDIOCAMX_HASINPUTEFFECTS=16,
		AUDIOCAMX_HASTRACKSWITHTRACKINPUT=32
	};

#ifdef _DEBUG
	void ShowInfo();
#endif

	void InitDefaultBusses();
	void CreateChannelsFullName();
	void ResetMaxPeaks();
	
	void CopyAudioChannelsAndMetroTracksToDevice(AudioDevice *);
	void CopyAudioChannels_Master(AudioDevice *);

	int GetCountOfObjectsUsingChannel(AudioChannel *);

	void SetAudioChannelFx(AudioChannel *,bool onoff);
	void SetSystemByPass(bool onoff);
	void SetTrackSystemByPass(bool onoff);
	void SetAudioSystemHasFlag();
	void PRepairForPB(AudioIOFX *);
	void ConnectToDevice(bool connectio);
	void Load(camxFile *);
	void Save(camxFile *);
	void Clone(AudioSystem *);

	// Channels

	void SelectBusFromTo(AudioChannel *from,AudioChannel *to,bool unselect,bool toggle);
	void SelectSingleBus(AudioChannel *);

	int GetCountOfBusChannels(){return audiochannels[CHANNELTYPE_BUSCHANNEL].GetCount();}
	AudioChannel *GetBusIndex(int nr){return (AudioChannel *)audiochannels[CHANNELTYPE_BUSCHANNEL].GetO(nr);}

	void SetFocusBus(AudioChannel *fb,bool refreshgui);
	AudioChannel *GetFocusBus(){return (AudioChannel *)audiochannels[CHANNELTYPE_BUSCHANNEL].activeobject;}

	// Playback
	void FindBestVChannels();

	void ChangeAllToAudioDevice(AudioDevice *,bool connectIO);
	void CloseAudioSystem(bool full);

	AudioChannel *FindPlaybackBusForAudioHDFile(AudioHDFile *);
	AudioChannel *FirstChannelType(int i){return (AudioChannel *)audiochannels[i].GetRoot();}

	inline AudioChannel *FirstBusChannel(){return (AudioChannel *)audiochannels[CHANNELTYPE_BUSCHANNEL].GetRoot();}
	inline AudioChannel *GetLastAudioBusChannel(){return (AudioChannel *)audiochannels[CHANNELTYPE_BUSCHANNEL].Getc_end();}
	inline AudioChannel *FirstDeviceInChannel(){return (AudioChannel *)audiochannels[CHANNELTYPE_DEVICEIN].GetRoot();}
	inline AudioChannel *FirstDeviceOutChannel(){return (AudioChannel *)audiochannels[CHANNELTYPE_DEVICEOUT].GetRoot();}

	AudioChannel *DeleteAudioChannel(AudioChannel *,bool fulldelete,bool locksystem=false);

	void DeleteAllChannels();

	// AudioBus
	AudioChannel *CreateBus(AudioChannel *prev,AudioPort *out);
	void AddBus(AudioChannel *prev,AudioChannel *bus);
	void AddBus(AudioChannel *bus,int index);

	void ResetChannels();
	void StopSystem();
	void AddMasterEffects(); // MasterMix

	void MixHWChannelsToMasterMix();

	AudioChannel masterchannel;

	Seq_Song *song;
	AudioDevice *device;
	int systemhasflag;
	bool channel_solomode,systembypassfx,songtracksbypassfx;

private:
	AudioEffectParameter metropar,masterpar;
	OList audiochannels[7];
};
#endif
