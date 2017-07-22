#ifndef CAMX_AUDIOEFFECTS_H
#define CAMX_AUDIOEFFECTS_H 1

#include "defines.h"
#include "audioobject_volume.h"
#include "audioobject_pan.h"
#include "MIDIindevice.h"
#include "systemautomation.h"

#ifdef ARES64
#include "audiohardwarebuffer.h"
#endif

#define MAXAUDIOPAN 64

enum CopyTo_Flags
{
	COPYTO_EFFECTS=1,
	COPYTO_PASTE=2
};

enum FX_EditData{
	EDIT_OUTVOLUME=100,
	EDIT_INVOLUME,
	EDIT_FXINTERN
};

class AudioChannel;
class AudioHardwareBuffer;
class AudioHardwareChannel;
class AudioChannel;
class AudioObject;
class InsertAudioEffect;
class Seq_Event;

class c_Pluginnotfound:public Object
{
public:
	char *plugin;
};

class AudioEffects:public Object
{
public:

	AudioEffects();

	void Mute(bool mute,bool lock);
	void FreeMemory();
	void AO_InitIOChannels(int channels);
	void InitStart();
	void SetFlags();
	void ClearBuffer();

	inline bool CheckIfEffectHasOnFX(){return hasfxs;}
	inline bool CheckIfEffectHasOnInstruments(){return hasinstruments;}
	inline bool CheckIfEffectHasNonInstrumentFX(){return hasnoninstrumentfx;}
	inline bool CheckIfEffectHasOnInstrumentsOrEffectsWithInput(){return hasfxwithinput;}

	void MixEffectDownToStream(AudioObject *,AudioEffectParameter *);
	bool AddEffects(AudioEffectParameter *);
	void SendEndEffects(AudioEffectParameter *);

	void SetTrackName(AudioObject *);
	bool IsInput(){return inputeffects;}

	InsertAudioEffect *AddInsertAudioEffect(InsertAudioEffect *next,AudioObject *,bool toclip=false,bool clone=true);
	InsertAudioEffect *DeleteInsertAudioEffect(Seq_Song *,InsertAudioEffect *,bool activesong,bool full=true);
	inline InsertAudioEffect *FirstInsertAudioEffect(){return (InsertAudioEffect *)effects.GetRoot();}

	inline InsertAudioEffect *FirstInsertAudioInstrument()
	{
		InsertAudioEffect *iae=FirstInsertAudioEffect();

		while(iae){
			if(iae->audioeffect && iae->audioeffect->IsInstrument()==true)return iae;
			iae=iae->NextActiveEffect();
		}

		return 0;
	}

	inline InsertAudioEffect *FirstActiveAudioInstrument()
	{
		InsertAudioEffect *iae=FirstInsertAudioEffect();

		while(iae){
			if(iae->audioeffect && iae->audioeffect->plugin_on==true && iae->audioeffect->IsInstrument()==true)return iae;
			iae=iae->NextActiveEffect();
		}

		return 0;
	}
	
	inline InsertAudioEffect *FirstActiveAndNonBypassEffectWithInput()
	{
		InsertAudioEffect *iae=FirstInsertAudioEffect();

		while(iae){
			if(iae->audioeffect && iae->audioeffect->plugin_on==true && iae->audioeffect->plugin_bypass==false && iae->audioeffect->GetInputPins()>0)return iae;
			iae=iae->NextActiveEffect();
		}

		return 0;
	}

	inline InsertAudioEffect *FirstActiveEffectWithMIDIInput()
	{
		InsertAudioEffect *iae=FirstInsertAudioEffect();

		while(iae){
			if(iae->audioeffect && iae->audioeffect->plugin_on==true && iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)return iae;
			iae=iae->NextActiveEffect();
		}

		return 0;
	}
	
	inline InsertAudioEffect *FirstActiveAudioEffect()
	{
		InsertAudioEffect *iae=FirstInsertAudioEffect();

		while(iae){
			if(iae->audioeffect && iae->audioeffect->plugin_on==true)return iae;
			iae=iae->NextActiveEffect();
		}

		return 0;
	}

	InsertAudioEffect *FindAudioObject(AudioObject *);
	void AddEffectEnd(InsertAudioEffect *);

	void DeleteAllEffects();
	void DeleteAllEffectBuffers();
	void RefreshBuffer(AudioDevice *);
	void ResetPlugins();
	void RefreshDo();
	void PreRefreshDo();

	void Delete(bool full);
	int GetCountEffects(){return effects.GetCount();}
	
	// I/O
	void Load(camxFile *);
	void Save(camxFile *);
	void CopyTo(AudioEffects *,int flag);
	void SetLoadParms();

	Object *Clone();
	Seq_Song *GetSong();

	// Static Effects
	AT_AUDIO_Volume volume;
	AT_AUDIO_Panorama pan;

	OList effects; // InsertAudioEffect intern audio effects, vsts ...

	AudioIOFX *io;
	Seq_Track *track; // Track FX
	AudioChannel *channel; // Audio Channel FX

	int fxflag;

#ifdef ARES
	float *output32;
#endif

	void SetMute(bool m){mute.mute=m;}
	bool GetMute(){return mute.mute;}
	AT_SYS_Mute mute;
	
	bool GetSolo(){return solo.solo;}
	void SetSolo(bool s){solo.solo=s;}
	AT_SYS_Solo solo;

	bool instrumentspossible,inputeffects,
		hasfxs,hasinstruments,hasfxwithinput,hasnoninstrumentfx;
};
#endif

