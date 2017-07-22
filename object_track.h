#ifndef CAMX_TRACKOBJECTS_H
#define CAMX_TRACKOBJECTS_H 1

#include "defines.h"
#include "trackeffect.h"
#include "object_group.h"
#include "openoutputnote.h"
#include "audioiofx.h"
#include "audiogroups.h"
#include "objectids.h"
#include "audioeffectparameter.h"
#include "object_trackhead.h"
#include "audiorealtime.h"

#define SIZETRACKMIN 0.5
#define SIZETRACKMAX 2

enum{
	TRACKTYPE_ALL,
	TRACKTYPE_MIDI,
	TRACKTYPE_AUDIO,
	TRACKTYPE_VIDEO
};

class Seq_Song;
class guiWindow;
class AudioPattern;
class MIDIPattern;
class Seq_ChildTrack;
class TrackIcon;
class Processor;
class guiMenu;
class AudioRegion;
class NewEventData;
class AudioFileInfo;

class Seq_TrackRecord:public Object
{
public:
	Seq_TrackRecord *NextTrackRecord(){return (Seq_TrackRecord *)next;}
	Seq_Track *track;
};

class Seq_TrackFX
{
public:
	Seq_TrackFX();

	void CreateTrackMix(AudioDevice *);
	void DeleteTrackMix();
	bool CheckAudioOutputString(char *,char *added);

	bool CheckAudioInputString(char *);
	char *GetAudioInputString();

	void Repair();
	void DeleteTrackRecord();

	Seq_TrackRecord *AddTrackRecord(Seq_Track *);
	void RemoveTrackRecord(Seq_Track *,bool all);
	Seq_TrackRecord *FindTrackInRecord(Seq_Track *);
	Seq_TrackRecord *FirstTrackRecord(){return (Seq_TrackRecord *)tracksrecord.GetRoot();}

	OList tracksrecord; // Seq_TrackRecord

	AudioHardwareBuffer *t_inputbuffers,*stream; // Thru+Input Audio Effects
	Seq_Track *track,*recordtrack;
	
	int nrstream,inputbufferscounter,t_audioinputwriteindex,t_audioinputreadindex,t_lastfilledbufferindex;
};

#define FREEZE_LOADOLDFILE 1

#define ADDSUBTRACK_LOCK 1
#define ADDSUBTRACK_CREATESTARTOBJECTS 2
#define ADDSUBTRACK_TOP 4
#define ADDSUBTRACK_UNDO 8

class TAudioOut // Audio Output->Gadget
{
public:
	TAudioOut(){returnstring=0;simple=false;}

	~TAudioOut(){
		if(returnstring)delete returnstring;
	}

	Seq_AudioIO *audiochannelouts;
	AudioChannel *status_audiochannel;
	char *returnstring;
	bool simple,usedirecttodevice;
};

class Seq_Track:public FolderObject,public TrackHead
{
	friend class Seq_Song;
	friend class Seq_Main;
	friend class EditFunctions;
	friend class Undo_CreateSubTrack;
	friend class Undo_DeleteAutomationTrack;
	friend class AudioObjectTrack;
	friend class Undo_CreatePattern;

public:

	enum{
		OUTPUTTYPE_MIDI,
		OUTPUTTYPE_AUDIOINSTRUMENT,
	};

	enum{
		TRACKDELETEFLAG_DONTDELETEFROZENFILE=1
	};

	enum{
		FREEZE_OFF,
		FREEZE_SOURCE,
		FREEZE_PREFADER
	};

	Seq_Track(){InitTrack(0);}

	Seq_Track(Seq_Song *s)
	{
		InitTrack(s);
	}

	virtual Seq_Track *Clone(){return new Seq_Track;}

	void SetAudioIOType(int type); // mono,stereo etc...
	void SetAudioIOType(AudioHDFile *);

	bool SetVType(AudioDevice *,int type,bool guirefresh,bool allselected);

	bool ConnectTrackOutputToPort(AudioPort *,bool deleteother); // -> Virtual Channel
	void SetMuteFlag();
	bool CanNewChildTrack();
	bool CanTrackBeChild(Seq_Track *);
	bool CheckTracking(int flag);
	bool CanChangeType();

	void SelectTrackColour(guiWindow *);
	void SetTrackColourOnOff(bool onoff);

	void StopAllofTrack();

	bool IsTrackChildOfTrack(Seq_Track *);
	bool CheckIfParentIsSelected();
	bool CheckIfPlaybackIsAble(){return t_muteflag==true?false:true;}

	bool IsTrackAudioMasterTrack();
	bool IsTrackAudioFreezeTrack();

	bool ParentShowChilds();
	void EditMIDIEventOutput(guiWindow *);

	void SetMIDIType(int type,bool force=false); // -1== Check For Audio Instrument
	void SetRecordTrackType(int type);

	void AddTrackToRecord(Seq_Track *);
	
	bool CompareMIDIOut(Seq_Track *);
	bool CompareMIDIIn(Seq_Track *);
	void AddTrackToBusOrParent(AudioDevice *);
	void ShowAudioOutput(TAudioOut *);
	void GenerateCrossFades();
	OSTART GetTrackLength();
	bool CheckIfTrackIsAudioInstrument();
	bool CheckMIDIInputEvent(MIDIInputDevice *,Seq_Event *);
	bool CheckMIDIInputEvent(MIDIInputDevice *,UBYTE status,UBYTE byte1,UBYTE byte2);
	bool CheckMIDIInputEvent(NewEventData *);
	void Load(camxFile *);
	void Save(camxFile *);
	//void CheckMIDIOutput(Seq_Event *);

	void LoadProcessor(camxFile *);
	void SaveProcessor(camxFile *);
	void Delete(bool full); // v
	void DeleteFrozenData();
	void CloneFx(Seq_Track *);
	Seq_Track *Clone(Seq_Song *);
	Seq_Track *Clone(Seq_Track *);
	void CloneToTrack(Seq_Track *,bool withpattern=true);
	AudioPattern *InitAudioRecordingPattern(AudioDevice *,OSTART patternstart,int cycleloop,LONGLONG writeoffset,bool setpunch2flag);
	void ToggleRecord();
	void ToggleMute();
	bool IsEditAble();

	void DoTrackInputFromTrack();
	void ResetAudioRecordPattern();
	void ResetMIDIRecordPattern();

	void ShowAllChildTracks();
	void ToggleShowChild(bool leftmouse);
	void ToggleShowAutomationTracks();

	void RemoveAllOtherTracksRecording();

	int GetTrackIndex(){return trackindex;}

	Seq_Track *PrevParentTrack();
	Seq_Track *NextParentTrack();

	Seq_Track *NextTrack_NoUILock(){return (Seq_Track *)NextQObject();}
	Seq_Track *NextTrack();
	Seq_Track *PrevTrack();
	

	void AddEffectsToPattern(Seq_Pattern *);
	void AddSortPattern(Seq_Pattern *,OSTART pstart);
	void SetFrozenPattern(AudioPattern *,OSTART pstart); // Audio -> Frozen

	int GetCountOfPattern(int mediatype);
	int GetCountOfPattern_NotFrozen(int mediatype);
	int GetCountChildTracksPattern();
	int GetCountSelectedPattern();

	Seq_Pattern *GetPatternIndex(int i,int mediatype=MEDIATYPE_ALL);
	int GetOfPattern(Seq_Pattern *,int mediatype=MEDIATYPE_ALL);

	void DeletePatternMemoryFromTrack(Seq_Pattern *,bool full,bool deletefromclones=true);
	Seq_Pattern* CutPattern(Seq_Pattern *,bool deletefromclones);
	Seq_Pattern* DeletePattern(Seq_Pattern *,bool full,bool deletefromclones=true);
	void MovePatternToTrack(Seq_Pattern *,Seq_Track *,OSTART pos);

	Seq_Pattern* FirstFrozenPattern(){return frozenpattern;}
	Seq_Pattern* FirstPattern_NotFrozen(int mediatype=MEDIATYPE_ALL);
	Seq_Pattern* FirstPattern(int mediatype=MEDIATYPE_ALL);
	Seq_Pattern* LastPattern(int mediatype);

	Seq_Pattern* LastPattern(){return frozen==true?frozenpattern:(Seq_Pattern *)pattern.Getc_end();}

	MIDIPattern *GetMIDIRecordPattern(OSTART time);

	Seq_Event *FirstPRepairNoteOff(int index){
		return (Seq_Event *)pRepareeventsnotes[index].GetRoot();
	}

	Seq_Event *FirstPRepairCtrl(int index){
		return (Seq_Event *)pRepareeventsctrl[index].GetRoot();
	}

	void DeletePRepairEvents(int mode);
	void RefreshTrackAfterRecording();
	
	char *GetName();
	void SetName(char *);
	void ShowTrackName(guiWindow *nostringrefresh);

	OSTART GetEnd();

	OSTART AddDelay(OSTART);
	void AddDelay(OSTART *h1,OSTART *h2);

	LONGLONG AddSampleDelay(LONGLONG);
	LONGLONG AddFullSampleDelay(LONGLONG); // no <0 check
	int GetMediaTypes();

	void DeleteTrackData(bool full);
	void StopAll();

	void DoAudioInput(AudioDevice *);
	void DoAudioInputEffects(AudioDevice *);
	void DoAudioInputPeak();

	void DoThru(AudioDevice *);
	bool InitPlayback(InitPlay *);

	void SendOutEvent_Automation(Seq_Event *,int createflag);
	void SendOutEvent_User(MIDIPattern *,Seq_Event *,int createflag);

	enum{
		SOE_CREATEREALEVENT=1,
		SOE_THRUEVENT=2,
		SOE_INPUT=4
	};

	void SendOutEvent_AudioEvent(Seq_Event *,int flag,int offset,AudioObject *dontsendtoaudioobject=0); // Send Single Event To Tracks Audio Instruments

	OpenOutputNote *FirstOpenOutputNote(){return (OpenOutputNote *)opennotes.GetRoot();}
	void AddOpenOutputNote(OpenOutputNote *note){opennotes.AddEndO(note);}
	OpenOutputNote *DeleteOpenOutputNote(OpenOutputNote *note){return (OpenOutputNote *)opennotes.RemoveO(note);}
	void DeleteAllOpenNotes();
	void SendAllOpenOutputNotes();

	void LockOpenOutputNotes(){sema_opennotes.Lock();}
	void UnlockOpenOutputNotes(){sema_opennotes.Unlock();}

	bool CheckIfTrackIsAudio();
	bool CheckIfTrackIsMIDI();

	bool IsPartOfEditing(Seq_Track *,bool allchilds=false); // clicked Track
	guiMenu *CreateTrackOutputMenu(guiMenu *,Seq_Song *);

	bool FindAudioRegion(AudioRegion *);

	void SetProcessor(Processor *);
	void SetProcessorBypass(bool onoff);

	bool AddAudioChannel(AudioChannel *);
	bool RemoveAudioChannel(AudioChannel *);
	bool ReplaceAudioChannel(AudioChannel *oldchannel,AudioChannel *newchannel);

	void InitAudioIO(Seq_Song *,int channels,int inputchannels=-1);
	void InitAudioIO(Seq_Song *,AudioFileInfo *);
	void InitAudioIO(Seq_Song *,AudioHDFile *);
	void InitAudioIOWithNewAudioFile(Seq_Song *,AudioHDFile *); // OUTPUT=pref_tracktype,INPUT hdfile

	int GetSoloStatus();
	bool GetSolo();
	void SetSolo(bool);

	bool CheckIfAudioRecording();

	bool GetMute();
	bool CheckSoloTrack();
	void SetDefaultAudio(bool lock,Seq_Track *clone);

	bool SetRecordChannel(AudioPort *,bool lock=true);
	void SetAudioInputEnable(bool);

	AudioPort *GetVIn(){return io.in_vchannel;}
	virtual Processor *GetProcessor(){return t_processor;}

	void SetTempInputMonitoring();
	void SetRecordMode(bool on,OSTART position);

	void SetInputMonitoring(bool on)
	{
		io.inputmonitoring=on;
		SetTempInputMonitoring();
	}

#ifdef WIN32
	inline void LockRecord(){trackrecord.Lock();}
	inline void UnlockRecord(){trackrecord.Unlock();}

	inline void LockStream(){stream.Lock();}
	inline void UnlockStream(){stream.Unlock();}

	inline void FuncMixLock(){func_mixlock.Lock();}
	inline void FuncMixUnLock(){func_mixlock.Unlock();}

	CCriticalSection trackrecord,stream,func_mixlock;
#endif

	Seq_TrackFX *GetAudioFX(){return &t_audiofx;}
	TrackEffects *GetFX(){return &t_trackeffects;}

	// Child Tracks
	int GetCountChildTracks();
	void AddChildTrack(Seq_Track *,int index);
	Seq_Track *FirstChildTrack(){return (Seq_Track *)FirstChild();}
	Seq_Track *LastChildTrack(){return (Seq_Track *)LastChild();}
	Seq_Track *NextChildTrack(){return (Seq_Track *)next;}
	Seq_Track *NextChildTrack(Seq_Track *init){return (Seq_Track *)NextChildNot(init);}

	char *GetMIDIInputString(bool add=true);
	char *GetMIDIOutputString(bool add=true);

	Seq_Event *FirstRawEvent(){return (Seq_Event *)events_raw.GetRoot();}
	void DeleteAllRawEvents();
	void CreateRawEvents(Seq_Pattern *singlepattern=0);
	void BufferRawEvent(MIDIPattern *,Seq_Event *);

	Seq_Event *DeleteRawEvent(Seq_Event *);
	virtual Colour *GetColour()
	{
		if(t_colour.showcolour==true)return &t_colour;
		if(parent)return ((Seq_Track *)parent)->GetColour();
		return &t_colour;
	}

	virtual Seq_Group_Group *GetGroups()
	{
		FolderObject *p=(FolderObject *)parent;
		while(p && p->parent)p=p->parent;
		return p?&((Seq_Track *)p)->t_groups:&t_groups;
	}

	virtual Seq_Group_MIDIOutputDevice *GetMIDIOut()
	{
		FolderObject *p=(FolderObject *)parent;
		while(p && p->parent)p=p->parent;
		return p?&((Seq_Track *)p)->t_MIDIoutdevices:&t_MIDIoutdevices;
	}

	virtual Seq_Group_MIDIInputDevice *GetMIDIIn(){return &t_MIDIinputdevices;}
	virtual Seq_AudioIO *GetAudioOut()
	{
		FolderObject *p=(FolderObject *)parent;
		while(p && p->parent)p=p->parent;
		return p?&((Seq_Track *)p)->t_audiochannelouts:&t_audiochannelouts;
	}

	bool GetUseVChannel(){return io.audioinputenable;}

	// Freeze
	bool SetFrozenHDFile(char *,int ticks);
	void FreezeTrack(int flag=0);
	bool UnFreezeTrack();
	void DeleteFrozenPattern();

	void SetDefaultMetroSettings();
	void SetOrGetRecordingSettings(int index,bool getmode);

	Seq_Group_MIDIOutputDevice t_MIDIoutdevices;
	Seq_Group_MIDIInputDevice t_MIDIinputdevices;
	Seq_AudioIO t_audiochannelouts;
	Seq_Group_Group t_groups;
	Seq_TrackFX t_audiofx;
	TrackEffects t_trackeffects;
	Colour t_colour;

	Seq_Track *nextcoretrack,*nextcorethrutrack,*nextcoreaudioinputtrack; // Audio Core Engine
	MIDIInputDevice *indevice; // 0 == all devices

	Seq_Pattern *frozenpattern;

	MIDIPattern *playback_MIDIPattern[INITPLAY_MAX][2], // next Playback Pattern nonNotes
		*playback_chainnoteMIDIPattern[INITPLAY_MAX][2], // Chain Notes
		*record_MIDIPattern;

	AudioPattern *playback_audiopattern[INITPLAY_MAX],*audiorecord_audiopattern[MAXRECPATTERNPERTRACK];

	Seq_Event *rawpointer; // Pointer <-> events_raw
	char *trackname,*frozenfile,*trackimage;
	TrackIcon *icon;
	AudioHDFile *frozenhdfile;
	Processor *t_processor;

	ARES audioinpeak;
	OSTART songstartposition,trackfreezeendposition;

	int trackindex,
		frozenfilestart,MIDIoutputimpulse,MIDInputimpulse,MIDInputimpulse_data,
		freezetype,
		recordindex,recordcounter,
		MIDItype, // MIDI/AudioInstrument
		recordtracktype, // User define MIDI, AUDIO etc...
		recordcycleindex, // Record Cycle Default:0
		vutype,
		createdatcycleloop,
		metrochl_b,metroport_b,metrokey_b,metrovelo_b,metrovelooff_b,
		metrochl_m,metroport_m,metrokey_m,metrovelo_m,metrovelooff_m;

	bool recordsettings_record[5],
		skiptrackfromCreateUsedAudioTrackList,
		checkcrossfade,record_cyclecreated,record,outputisinput,
		cyclerecordmute,
		recordbeforestart,
		t_mutebuffer,solobuffer,
		buffer_solo, // Freeze buffer
		useaudio, // same as Audio Channel
		frozen,underfreeze,tmp_frozen,
		usedirecttodevice,
		MIDItypesetauto,
		t_muteflag,
		ismetrotrack,
		metrosendtoaudio,metrosendtoMIDI;

protected:
	void InitTrack(Seq_Song *);

	AudioEffectParameter input_par,output_par;
	OList pRepareeventsnotes[2],pRepareeventsctrl[2],opennotes;
	OListStart events_raw,pattern;
	CCriticalSection sema_opennotes;
};

class Seq_MetroTrack:public Seq_Track
{
public:

	Seq_MetroTrack()
	{
		metroclickcount=0;
		InitTrack(0);
	}

	Seq_MetroTrack(Seq_Song *s)
	{
		metroclickcount=0;
		InitTrack(s);
	}

	Seq_MetroTrack *NextMetroTrack(){return (Seq_MetroTrack *)next;}

	AudioRealtime metroclick_a,metroclick_b[4];
	int metroclickcount;
};
#endif
