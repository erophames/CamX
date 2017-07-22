#ifndef CAMX_AUDIOOBJECT
#define CAMX_AUDIOOBJECT 1

#include "automation.h"
#include "triggerevent.h"
#include "lastMIDIevent.h"

#define MAXPLUGINVALUESTRINGLEN 32

#define audioobject_TYPE_EFFECT 0
#define audioobject_TYPE_INSTRUMENT 1

class InsertAudioEffect;
class AudioEffects;
class RunningAudioFile;
class AudioHardwareBuffer;
class AudioEffectParameter;
class guiWindow;
class Seq_Event;
class AutomationTrack;
class Seq_Song;
class guiWindowSetting;
class AudioDevice;
class AudioChannel;
class Seq_Track;
class AudioIOFX;
class EditData;
class guiMenu;
class Directory;

#ifdef _DEBUG
extern int defcounter;
#endif

enum{
	TRIGGER_FORCE=1,
	TRIGGER_THRUEVENT=(1<<1),
	TRIGGER_MASTEREVENT=(1<<2)
};

class AudioObjectIO
{
public:
};

class CrashedPlugin:public Object
{
public:
	char *fulldllname;
};


class SeparateOuts // Plugin->Outs
{
public:

	SeparateOuts()
	{
		buspointer=0;
		use=false;
	}

	void SO_Init();
	void SO_DeInit();

	void SO_Load(camxFile *);
	void SO_Save(camxFile *);

	AudioChannel  *buspointer;
	bool use;
};

class AudioObject:public AudioAutomationObject // Audio Effect
{
	friend class AutomationTrack;
public:

	enum{
		FT_32BIT,
		FT_64BIT
	};

	enum{
		MIXLAW_6db,
		MIXLAW_45db,
		MIXLAW_3db,
		MIXLAW_0db
	};

	AudioObject();
	~AudioObject()
	{
		Close(true);
	}

	virtual void Edit_Data(EditData *){}
	virtual int GetProgram(){return 0;}
	virtual char *GetProgramName(){return 0;}
	virtual char *GetProgramNameIndex(int i){return 0;}
	virtual bool CanGetProgramNameIndex(){return false;}
	virtual bool SetProgram(int index){return false;}
	virtual AudioObject *CloneEffect(int flag,Seq_Song *)=0;
	virtual AudioObject *InitOpenEffect()=0;
	virtual void SaveSettings(camxFile *){}

	virtual void PlugInOpen(){}
	virtual void PlugInClose(){}
	virtual void PlugInOn(){};
	virtual void PlugInOff(){};

	virtual void OnOff(bool on);
	virtual void Bypass(bool on);

	virtual void ClearBuffer(){}
	virtual void InitStart(){ClearBuffer();}
	virtual void ResetAudioObject(){}
	virtual void Close(bool full){FreeMemory();}
	virtual void FreeMemory(){}
	virtual void CloneData(AudioObject *to){to->value=value;}
	virtual bool CanChunk(){return false;}
	virtual bool CanMIDIInput(){return false;}
	virtual void LoadChunkData(camxFile *){}
	virtual void SaveChunkData(camxFile *){}

	// FX
	virtual bool DoEffect(AudioEffectParameter *){return false;}

	// Instrument
	virtual bool Do_TriggerEvent(int offset,UBYTE status,char b1,char b2,char velocityoff,int flag=0,LONGLONG noteLength_samples=0,LONGLONG noteOffset_samples=0){return false;}
	virtual bool Do_TriggerSysEx(int offset,UBYTE *data,int datalength){return false;}

	virtual void Execute_TriggerEvents(){triggerevents=0;}
	virtual guiWindow *CheckIfWindowIsEditor(guiWindow *){return 0;}
	virtual guiWindow *OpenGUI(Seq_Song *,InsertAudioEffect *,guiWindowSetting *set=0){return 0;}
	virtual bool CloseGUI(){return false;}
	virtual void Reset(){}
	virtual void Init(){}
	virtual bool GetSpeakerArrangement(){return false;}
	virtual void GetIOConfig(){}
	virtual char *GetEffectName(){return 0;}
	virtual char *GetCompany(){return 0;}

	void ValueChanged(OSTART time,int index,float value);

	int GetSubID(){return audioeffecttype;}
	void AddIOMenu(guiMenu *);
	void CopySettings(AudioObject *);

	enum{
		CREATENEW_PLUGIN=1
	};

	Object *Clone(){return CloneEffect(0,0);}

	void AO_InitSampleRateAndSize(int rate,int buffersize);
	void AO_InitIOChannels(int channels);
	virtual void InitSampleRateAndSize(int rate,int buffersize)=0;
	virtual bool InitIOChannels(int channels)=0;
	virtual bool GetOwnEditor(){return false;}
	virtual void InitOwnEditor(int *width,int *height){}
	virtual bool IsInstrument(){return false;}

	void TogglePlugInOnOff();
	void User_TogglePluginBypass();

	int GetSetInputPins(){return setins;}
	int GetSetOutputPins(){return setouts;}

	int GetInputPins(){return ins;}
	int GetOutputPins(){return outs;}

	int GetCountOfParameter(){return numberofparameter;}

	void CreateAudioObjectBuffer();
	void DeleteBuffer();

	void SetInVolume(ARES);
	void SetOutVolume(ARES);
	bool IsActive();

		// Chunks
	virtual void CopyChunkData(AudioObject *){} // Plugin->Plugin direct
	virtual void BufferChunkData(){} // Plugin->chunkdata
	virtual void RestoreChunkData(){} // chunkdata->Plugin

	void FreeChunks();

#ifdef WIN32
	CCriticalSection sema_trigger,sema_lock,autosema;
#endif

	AudioObjectIO iosettings;
	LMIDIEvents monitor_events[MAXMONITOREVENTS];
	SeparateOuts sep_outs;

	LONGLONG filesize,timeusage; // <type connected with VSTPlugin_Settings !
	InsertAudioEffect *inserteffect;
	Seq_Song *song;
	AudioObject *fromobject;
	AudioEffects *audioeffects;
	AudioHardwareBuffer *aobuffer;
	Directory *directory;
	char *chunkdata_buffer;
	ARES involume,outvolume;

	int chunksize_buffer,
		monitor_syscounter,monitor_eventcounter,
		numberofprograms,
		numberofparameter, // <type connected with VSTPlugin_Settings !
		internid, // for intern effects
		triggerevents,sysextriggerevents,
		floattype,crashed,mixlaw,
		ins,outs,setins,setouts,
		iochannels, // Set Channels
		intern_flag,audioeffecttype,
		setSampleRate,setSize;

	bool crashmessage,init,plugin_on,plugin_bypass,plugin_active,iovolume,iosetcorrect,settrackname,updatedisplay;

#ifdef WIN32
	void LockAudioObject(){sema_lock.Lock();}
	void UnlockAudioObject(){sema_lock.Unlock();}

	void LockTrigger(){sema_trigger.Lock();}
	void UnlockTrigger(){sema_trigger.Unlock();}

	void LockAutomation(){autosema.Lock();}
	void UnlockAutomation(){autosema.Unlock();}
#endif
};

class InsertAudioEffect:public Object
{
public:
	InsertAudioEffect(AudioEffects *fx)
	{
		effectlist=fx;
		audioeffect=0;
		loadparameters=0;
		bufferobject=0;
	}

	InsertAudioEffect *PrevEffect(){return (InsertAudioEffect *)prev;}
	InsertAudioEffect *NextEffect(){return (InsertAudioEffect *)next;}

	InsertAudioEffect *NextActiveEffect();
	void Load(Seq_Song *,camxFile *);
	void Save(camxFile *);

	MIDIFilter MIDIfilter;
	AudioEffects *effectlist; // List
	AudioObject *audioeffect,*bufferobject;
	ARES *loadparameters;
	char *read_dllname,*read_fullname;
};
#endif