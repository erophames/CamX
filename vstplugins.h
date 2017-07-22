#ifndef CAMX_VSTPLUGINS_H
#define CAMX_VSTPLUGINS_H 1

#undef strncpy

#include "vst24/aeffectx.h"
#include "object.h"
#include <windows.h>

#define MAXVSTRIGGEREVENTS 256

#include "audioobjects.h"

class VSTOverFlowBuffer:public Object
{
public:
	LONGLONG noteLength_samples,noteOffset_samples;
	int offset,flag;
	UBYTE status;
	char b1,b2,velocityoff;
};

enum{
	VSTCRASH_effProcessEvents=1,
	VSTCRASH_effGetParamName=(1<<1),
	VSTCRASH_effGetProgram=(1<<2),
	VSTCRASH_effSetProgram=(1<<3),
	VSTCRASH_getParameter=(1<<4),
	VSTCRASH_setParameter=(1<<5),
	VSTCRASH_effClose=(1<<6),
	VSTCRASH_processReplacing=(1<<7),
	VSTCRASH_Reset=(1<<8),
	VSTCRASH_SetSampleSize=(1<<9),
	VSTCRASH_effEditGetRect=(1<<10),
	VSTCRASH_processreplacing=(1<<11),
	VSTCRASH_Bypass=(1<<12),
	VSTCRASH_GetInputProperties=(1<<13),
	VSTCRASH_GetOutputProperties=(1<<14),
	VSTCRASH_GetSpeakerArrangement=(1<<15),
	VSTCRASH_effGetCompany=(1<<16),
	VSTCRASH_SetSpeakerArrangement=(1<<17),
	VSTCRASH_SetProcessPrecision=(1<<18),
	VSTCRASH_EditIdle=(1<<19),
	VSTCRASH_Open=(1<<20),
	VSTCRASH_Close=(1<<21),
	VSTCRASH_GetChunk=(1<<22),
	VSTCRASH_SetChunk=(1<<23)
};

enum{
	VST_SENDVSTEVENTS=(1<<1),
	VST_SENDVstMidiEventS=(1<<2),
	VST_RECEIVEVSTEVENTS=(1<<3),
	VST_RECEIVEVstMidiEventS=(1<<4),
	VST_SUPPORTBYPASS=(1<<5),
	VST_PROGRAMCHUNKS=(1<<6),
	VST_HASEDITOR=(1<<7)
};

class VSTDataDump
{
public:
	VSTDataDump()
	{
		values=0;
		valuesize=0;
		dumpdata=0;
		dumpsize=0;
	}

	ARES *values;
	int valuesize;

	// Dump
	char *dumpdata;
	int dumpsize;
};

class VSTPlugin:public AudioObject
{
	friend class mainAudio;

public:
	VSTPlugin();

	virtual double GetParm(int index);
	virtual bool SetParm(int index,double par);
	
	bool GetOwnEditor();
	void InitOwnEditor(int *width,int *height);
	char *GenerateInfoString();

	void GetIOConfig();
	bool GetSpeakerArrangement();
	bool CanMIDIInput();

	// Chunks
	bool CanChunk();
	void LoadChunkData(camxFile *);
	void SaveChunkData(camxFile *);

	// V Wrapper
	int GetProgram();
	char *GetProgramName();
	char *GetProgramNameIndex(int i);
	bool CanGetProgramNameIndex();
	void SaveSettings(camxFile *);
	bool CheckSettings();

	bool IsInstrument();
	
	void Close(bool full);
	// GUI
	guiWindow *CheckIfWindowIsEditor(guiWindow *);
	guiWindow *OpenGUI(Seq_Song *,InsertAudioEffect *,guiWindowSetting *set=0); //v
	bool CloseGUI();

	AudioObject *CloneEffect(int flag,Seq_Song *);
	AudioObject *InitOpenEffect();

	void Reset();
	void InitSampleRateAndSize(int rate,int buffersize);
	bool InitIOChannels(int channels);

	void PlugInOpen();
	void PlugInClose();
	void PlugInOn(); // Bypass
	void PlugInOff(); // Bypass
	
	void ClearBuffer();
	bool DoEffect(AudioEffectParameter *); // virtual
	bool Do_TriggerEvent(int offset,UBYTE status,char b1,char b2,char velocityoff,int flag,LONGLONG noteLength_samples,LONGLONG noteOffset_samples);
	bool Do_TriggerSysEx(int offset,UBYTE *data,int datalength);

	void Execute_TriggerEvents();
	char *GetParmName(int index);
	char *GetParmValueString(int index);
	char *GetParmTypeValueString(int index);
	bool SetProgram(int program);

	char *GetEffectName(){return effectname;} //v
	char *GetCompany();

	void Load(camxFile *);
	void Save(camxFile *);
	void CopySettings(VSTPlugin *);

	void CopyChunkData(AudioObject *);
	void BufferChunkData();
	void RestoreChunkData();

	VSTPlugin *DeleteVSTPlugin();	
	VSTPlugin *NextVSTPlugin() {return (VSTPlugin *)next;}

	#ifdef DEBUG
	char n[4];
#endif

	VstPinProperties inproperties;
	VstTimeInfo timeinfo;
	VSTDataDump vstdatadump;
	OList vstoverflowbuffer,*vstlist;

#ifdef WIN32
	HINSTANCE libhandle;
	AEffect* ptrPlug;

	void LockRefreshSize(){sema_size.Lock();}
	void UnlockRefreshSize(){sema_size.Unlock();}
	CCriticalSection sema_size;
#endif

	char vstring[64+1]; //64 = avoid Bugs in SDK ?

	char effectname[kVstMaxEffectNameLen+1], //max 32
		company[kVstMaxVendorStrLen+1],
		*fulldllname, // <type connected with VSTPlugin_Settings !
		*dllname;// <type connected with VSTPlugin_Settings !
	
	int type;
	//	version;// <type connected with VSTPlugin_Settings !
	
	int vst_version; // 1...2 // <type connected with VSTPlugin_Settings !
	int version;
	int vstguistartx,guiwidth,guiheight,newwidth,newheight;
	bool synth, // <type connected with VSTPlugin_Settings !
		canDoubleReplacing,NoSoundInStop,
		refreshgui,
		refreshwindowsize;

#ifdef DEBUG
	int noteoncounter,noteoffcounter;
#endif

private:
	void SetVSTEventData(VstMidiEvent *,int offset,UBYTE status,char b1,char b2,char velocityoff,LONGLONG noteLength_samples,LONGLONG noteOffset_samples,int flag);
	void SetVSTSysExEventData(VstMidiSysexEvent *,int offset,UBYTE *data,int datalength);
	
	VstMidiEvent VstMidiEvents[MAXVSTRIGGEREVENTS];
	VstMidiSysexEvent vstsysexMIDIevents[MAXVSTRIGGEREVENTS];

	void **ptrInputBuffers,**ptrOutputBuffers; // IO Buffer
	VstEvents *vsteventarray,*vstsysexeventarray; // MIDI + SysEx Event Trigger
};
#endif