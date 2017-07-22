#ifndef CAMX_SONGOBJECTS_H
#define CAMX_SONGOBJECTS_H 1

#include "audiosystem.h"
#include "textandmarker.h"
#include "seq_realtime.h"
#include "metronome.h"
#include "MIDIclock.h"
#include "undo.h"
#include "seqtime.h"
#include "drummap.h"
#include "seqpos.h"

//class Drummap;
class Seq_Signature;
class EventEditor;
class Seq_Group_GroupPointer;
class Seq_Group;
class VSTPlugin;
class NewEventData;
class Editor;
class guiScreen;
class PatternLink;
class EditAudioMix;
class Seq_MetroTrack;

#define CREATESTREAMINIT 1
#define CREATESTREAMMASTER (1<<1)
#define CREATESTREAMDOCYCLE_1 (1<<2)

// Stop Flags
#define NO_SONGPRepair 1
#define SETSONGPOSITION_NOGUI (1<<1)
#define SETSONGPOSITION_FORCE (1<<2)
#define SETSONGPOSITION_NOMIDIOUTPUT (1<<3)

#define RECORD_INDEX_DEVICE 0
#define RECORD_INDEX_TRACKTRACK 1

class Song_Playbacksettings
{
	friend class Seq_Song;

public:
	Song_Playbacksettings();
	void FreeMemory();
	void ConvertTicksToTempoSampleRate();
	char *CreateFromToString(int index,int flag);
	void SetCycleMeasure();
	void Load(camxFile *);
	void Save(camxFile *);
	OSTART GetCycleRange(){return cycleend-cyclestart;}

#ifdef WIN32
	virtual void Lock(){lock_semaphore.Lock();}
	virtual void Unlock(){lock_semaphore.Unlock();}
	CCriticalSection lock_semaphore;
#endif

	char name[8][STANDARDSTRINGLEN+2],*fromtostring;
	Seq_Song *song;
	OSTART cyclestart,cycleend,cyclestart_buffer[8],cycleend_buffer[8],cyclestart_measure,cycleend_measure,cyclestart_measure_buffer[8],cycleend_measure_buffer[8];
	LONGLONG cycle_samplestart,cycle_sampleend;
	int activecycle,cyclecounter_MIDI,solo;
	bool cycleplayback,automationplayback,automationrecording;
};

static OSTART steptimes[]={
	TICK1nd,
	TICK2nd,
	TICK4nd,
	TICK8nd,
	TICK16nd,
	TICK32nd,
	TICK64nd
};

#define RECORD_SKIPPRECOUNTER 1
#define MAXROUTEVENTS 7

class Seq_MIDIRouting_Router_Track:public Object{
public:
	Seq_MIDIRouting_Router_Track(){track=0;}
	Seq_MIDIRouting_Router_Track *NextTrack(){return (Seq_MIDIRouting_Router_Track *)next;}
	Seq_Track *track;
};

class Seq_MIDIRouting_Router{
public:
	Seq_MIDIRouting_Router_Track *FirstTrack(){return (Seq_MIDIRouting_Router_Track *)tracks.GetRoot();}
	Seq_MIDIRouting_Router_Track *AddTrack (Seq_Track *);
	Seq_MIDIRouting_Router_Track *RemoveTrack(Seq_MIDIRouting_Router_Track *);
	Seq_MIDIRouting_Router_Track *GetTrackIndex(int index){return (Seq_MIDIRouting_Router_Track *)tracks.GetO(index);}

	OList tracks;
};

class Seq_MIDIRouting_InputDevice:public ObjectLock{
public:
	enum{
		NOTES,
		PCHANGE,
		CPRESS,
		PPRESS,
		PITCH,
		CCHANGE,
		SYS
	};

	Seq_MIDIRouting_InputDevice *NextDevice(){return (Seq_MIDIRouting_InputDevice *)next;}
	bool CheckEvent(Seq_Track *,UBYTE status,UBYTE b1,UBYTE b2,bool checkfocustrack);
	bool CheckEvent(Seq_Track *track,Seq_Event *e,bool checkfocustrack){return CheckEvent(track,e->status,e->GetByte1(),e->GetByte2(),checkfocustrack);}
	void FreeMemory();

	Seq_MIDIRouting_Router router_channels[16]; // MIDI Channel
	Seq_MIDIRouting_Router router_events[7];
	MIDIInputDevice *device;
};

class Seq_MIDIRouting
{
public:
	void InitDevices();
	void FreeMemory();
	Seq_MIDIRouting_InputDevice *FirstInputDevice(){return (Seq_MIDIRouting_InputDevice *)indevices.GetRoot();}
	Seq_MIDIRouting_InputDevice *FindInputDevice(MIDIInputDevice *);
	void RemoveTrack(Seq_Track *);
	void Load(camxFile *);
	void Save(camxFile *);
	void SaveTrackList(Seq_MIDIRouting_Router *,camxFile *);

	OList indevices;
};

class OpenNoteFilter
{
public:
	OpenNoteFilter(){for(int i=0;i<16;i++)channel|=1<<i;}
	bool CheckNote(UBYTE status,char key);
	int channel;
};

enum{
	RAD_CYCLERESET=1,
	RAD_CYCLERESET2=2
};

class Seq_SelectedPattern;

class SoloPattern:public Object{
public:
	guiWindow *window;
	Seq_SelectedPattern *spattern;
};

enum{
	FX_SHOWMIDI=(1<<1),
	FX_SHOWMIDIPattern=(1<<2),
	FX_SHOWAUDIOPATTERN=(1<<3),
	FX_SHOWAUDIO=(1<<4),
	FX_SHOWQUANT=(1<<5),
	FX_SHOWPATTERN=(1<<6),
	FX_SHOWMIDIFILTER=(1<<7),
	FX_SHOWMETRO=(1<<8),
	FX_SHOWMIXER=(1<<9)
};

class cMIDIPlaybackEvent
{
public:
	cMIDIPlaybackEvent(int i){index=i;playbackevent=0;}

	Seq_Event *playbackevent;
	MIDIPattern *playbackpattern;
	LONGLONG nextMIDIplaybacksampleposition;

	int index;
	bool ischainnote;
};

class Seq_Song:public Object
{	
	friend class Seq_Project;
	friend class Seq_Main; 
	friend class AudioSystem;
	friend class Undo_AddTracksAsChild;
	friend class Undo_RemoveTracksFromChild;
	friend class Undo_SortAllTracks;
	friend class Undo_MoveTrack;

public:

	enum{
		CYCLE_RESET_MIDI,
		CYCLE_RESET_AUDIO,
		CYCLE_RESET_AUDIORECORD,
		CYCLE_RESET_STREAM,
		CYCLE_RESET_SYNCEND
	};

	enum{
		TRACKIO_FLAG_ISCHILD=1
	};

	enum{
		FLAG_AUTOSAVE=1
	};

	enum{
		SONGMM_MASTERMIX,
		SONGMM_MASTERSELTRACKS,
		SONGMM_MASTERSELCHANNELS,
		SONGMM_MASTERSELTRACKSCHANNELS,
	};

	enum{
		STATUS_STOP=0,
		STATUS_SONGPLAYBACK_MIDI=1,
		STATUS_PLAY=(1<<1),
		STATUS_RECORD=(1<<2),
		STATUS_PAUSE=(1<<3),
		STATUS_INIT=(1<<4),
		STATUS_SONGPLAYBACK_AUDIO=(1<<5),
		STATUS_MIDIWAIT=(1<<6),// wait for MIDI etc...
		STATUS_WAITPREMETRO=(1<<7),// Metronome Pre Beats
		STATUS_STEPRECORD=(1<<8), 
		STATUS_WAITAUDIOPREMETROCANCELED=(1<<9), // Audio Metronome PreBeats cancel
		STATUS_MIDICYCLERESET=(1<<10)
	};

	enum{
		ADDTRACK_NOACTIVATE=1
	};

	enum{
		FOCUSTYPE_TRACK,
		FOCUSTYPE_BUS,
		FOCUSTYPE_METRO,
		FOCUSTYPE_MASTER
	};

	enum{
		RLF_NOLOCK=1,
		RLF_REFRESHGUI=2,
		RLF_CHECKREFRESH=4
	};

	enum{
		SENDPRE_MIDI=1,
		SENDPRE_AUDIO=2
	};

	enum{
		SANOFF_ONLYAUDIO=1,
		SANOFF_ONLYMIDI=2
	};

	enum{
		PUNCHOFF=0,
		PUNCHIN=1,
		PUNCHOUT=2
	};

	Seq_Song(Seq_Project *);

	void CheckKeyTimer(); 
	void TempSoloOnOff(guiWindow *,Seq_SelectionList *);
	SoloPattern *FindSoloPattern(Seq_SelectedPattern *);
	SoloPattern *AddSoloPattern(guiWindow *,Seq_SelectedPattern *);
	int CheckSoloPattern(Seq_Pattern *);
	void RemoveSoloPattern(SoloPattern *);
	void RemoveFromSoloOnOff(guiWindow *);

	bool IsPlayback(){return (status&STATUS_PLAY)?true:false;}
	bool IsRecording(){return (status&STATUS_RECORD)?true:false;}

	bool IsAutomationParameterSelected();
	AutomationTrack *FirstAutomationTrackWithSelectedParameters();

	void RefreshRealtime_Slow();
	void AudioInputNeeded();
	bool FindOpenNote(int key,OpenNoteFilter *filter=0);
	void SysexEnd(NewEventData *);
	bool CheckStatus(UBYTE status);

	void NewEventInput(NewEventData *); // main Input
	void NewEventInput(NewEventData *,Seq_Event *); // event ->NewEventData
	void NewMIDIInput(NewEventData *);
	void InitNewSongPositionToStreams();

	void OpenIt(guiScreen *);
	void OpenRecordEditor();
	void OpenSyncEditor();

	void ToggleCycle();
	void CheckForCycleReset();
	void InitDefaultMetroTracks();
	void SetSync(int type,bool bufferold=false);

	void CheckCrossFades();
	bool RepairCrossFades();
	int GetCountSelectedTracks();
	int GetCountTrackWithAudioOut(Seq_Track *not);

	void SelectTracksExcept(Seq_Track *,bool onoff,bool onoffextrack);
	void SelectTracksFromTo(Seq_Track *from,Seq_Track *to,bool unselect=true,bool toggle=false);
	void SelectSingleTrack(Seq_Track *);
	void UnSelectTracksFromTo(Seq_Track *from,Seq_Track *to);
	void Activated();
	void DeActivated();
	void ResetSoloMute();
	void ResetPlugins();
	int ReplacedFile(AudioHDFile *);
	void RefreshGroupSolo();

	void SetMIDIRecordingFlag();
	void RefreshRealtime();
	void SetMuteFlags();
	void SetPatternMuteFlag(Seq_Track *,Seq_Pattern *);

	void CheckEffectPipeline(AudioEffects *);
	void DoAutoSave();
	void SendAllOpenNotes();
	void ResetRecordingFlags();

	void CheckStreamAudioTempoRefresh(int mediatype);
	void CreateAudioStream(AudioDevice *,int flag); // force=create ring buffer
	void CheckAndCloseRAFFiles(AudioDevice *,int flag);
	void StreamInit();

	void AddAudioInstrumentsToStream(AudioDevice *);
	void SendAudioMIDIAutomation();

	void SendAudioWaitEvents(AudioDevice *,LONGLONG samplestart,LONGLONG sampleend,bool force); //  samplestart=-1, force all
	LONGLONG CheckRTEAlarmEvents();

	bool CanStatusBeChanged(){return mastering==true?false:true;}
	bool CanSongPositionBeChanged(OSTART pos);

	void DoSongAudioEffects(AudioDevice *);
	void DoAudioEffects_Mastering(AudioDevice *);
	void DoAudioEffectsAndCopyToDevice(AudioDevice *);

	bool GetFXFlag(int flag); // Edit_Arrange_FX
	void Load(camxFile *); // v
	bool LoadSongInfo(camxFile *); // only info, songname etc..
	void Save(camxFile *); // v
	void SaveArrangement(camxFile *);

	void LoadDrumMaps(camxFile *);
	void SaveDrumMaps(camxFile *);

	void RepairSongAfterLoading();
	bool CheckPunch(OSTART);
	void CheckPluginTimer();

	void SetRecordTrackTypes(Seq_Track *,int type);

	Seq_Signature *AddNewSignature(int measure,int new_nn,int new_dn);

	Seq_Song *CreateClone();
	Seq_Song *PrevSong() {return (Seq_Song *)prev;}
	Seq_Song *NextSong() {return (Seq_Song *)next;}

	void CopyTrackStreamBuffer_Mastering(AudioDevice *);
	void CopyTrackStreamBuffer(AudioDevice *);

	int CreateUsedAudioTrackList(int flag,int childdepth);
	int CreateUsedAudioBusList();
	int CreateUsedTrackThruList();
	int CreateUsedTrackWithTrackInput();
	int CreateUsedAudioChannelThruList();
	int CreateTrackListWithAudioInputEffects();
	int CreateAudioDeviceChannelList(AudioDevice *);

	void SetTrackIndexs();
	void AddChildTrack(Seq_Track *parent,Seq_Track *child,int flag=0,int index=-1);
	void AddTrack(Seq_Track *,int index =-1,int flag=0); // index -1 == add to end

	bool MoveTrackIndex(Seq_Track *,int index);

	void DeleteTrack(Seq_Track *,bool full);
	Seq_Track *DeleteTrack_R(Seq_Track *,bool full);

	inline Seq_Track *FirstTrack(){return (Seq_Track *)tracks.GetRoot();}
	Seq_Track *FirstSelectedTrack();
	Seq_Track *LastSelectedTrack();

	Seq_Track *FirstParentTrack();
	Seq_Track *LastParentTrack();
	Seq_Track* LastTrack() {return (Seq_Track *)tracks.Getc_end(); } 
	AutomationTrack *LastAutomationTrack();

	bool FindTrack(Seq_Track *);
	bool FindPattern(Seq_Pattern *);
	Seq_Pattern *FirstSelectedPattern();
	void SendMIDI2MIDIAutomation(OSTART position);

	// Index
	Seq_Pattern *GetPatternIndex(int trackindex,int patternindex,int mediatype);

	// Core Called
	AudioChannel *GetUsedAudioChannel();
	AudioChannel *GetUsedAudioBus();
	Seq_Track *GetUsedAudioTrack();
	Seq_Track *GetUsedAudioThruTrack();
	Seq_Track *GetUsedAudioInputTrack();
	Seq_Track *GetUsedAudioTrackTrackIO(); // Track->Output -> Track Input

	void InitDefaultMasterName();
	void SetMasterFileName(char *);

	void RefreshAudioFocusWindows(Seq_Track *,AudioChannel *,guiWindow *dontrefresh);

	AudioHardwareChannel *GetUsedAudioDeviceChannel();
	void ResetSongTracksAudioBuffer();

	void FreeMemory(int flag);
	void DeleteUnUsedAudioFiles();
	void Delete();
	char *CreateWindowTitle();

	void ClearAllUnderSelectionPattern();
	void UnMuteAllTracks();
	void RefreshMuteBuffer();

	void UnSoloAllTracks();
	void RefreshSoloBuffer();
	bool AddVirtualTempoAtPosition(OSTART position,double tempo,int flag);
	void DeleteAllVirtualTempos();

	void BufferBeforeTempoChanges();
	void RefreshTempoBuffer();
	void StartAudioRecording();
	void WriteAudioRecording(AudioDevice *);

	OSTART QuantizeWithFlag(OSTART,int qflag);

	bool RepairLoops(int flag=0);
	bool CanAudioRecordingFileGotoCycleMode();

	void SetFocusType(int t,bool guirefresh=true);
	int GetFocusType(){return focustype;}

	Seq_Track *GetTrackIndex(int index){return (Seq_Track *)tracks.GetO(index);}
	Seq_Track *GetFocusTrack(){return (Seq_Track *)tracks.activeobject;}
	Seq_Track *GetFocusMetroTrack(){return (Seq_Track *)metrotracks.activeobject;}

	Seq_Pattern* GetFocusPattern(){return activepattern;}
	Seq_Event *GetFocusEvent(){return activeevent;}

	Seq_Track *GetMuteTrack(Seq_Track *track=0);
	Seq_Track *GetSoloTrack(Seq_Track *track=0);

	int GetOfTrack(Seq_Track *t){return tracks.GetIx((FolderObject *)t);}
	int GetCountOfTrackswithMIDIData();
	int GetCountOfTracks(){return qtracklist.number;}
	int GetCountOfTracksWithOutChilds();
	int GetCountOfPattern();
	int GetCountOfAudioPattern();
	int GetCountOfPattern(OSTART position,Seq_SelectionList *sellist=0,bool onlyselected=false,bool selectedtracks=false); // >>> position
	int GetCountOfPattern(OSTART position,bool onlyselected=false,bool onlyselectedtracks=false);
	int GetCountOfAutomationTracksUsing(AutomationObject *);
	int GetCountSelectedPattern(); 

	void FillAutomationListUsing(AutomationTrack **,int c,AutomationObject *);
	void UserEditAutomation(AutomationObject *,int parindex);

	bool SetFocusTrack(Seq_Track *);
	void SetFocusTrackLock(Seq_Track *,bool unselectold,guiWindow *dontrefresh=0,bool refreshaudiomixgui=true); // Locks MIDI Thru
	void SetFocusTrackNoGUIRefresh(Seq_Track *);

	void SetFocusMetroTrack(Seq_Track *);

	void SetFocusPattern(Seq_Pattern *);
	void SetFocusEvent(Seq_Event *);
	void ResetTrackZoom();

	bool CopyRecordDataToRecordPattern();
	void RefreshLoops();
	void SetRunningFlag(OSTART sposition,bool force);
	void InitMIDIIOTimer(OSTART startposition);

	OSTART GetSongPosition();
	OSTART GetSongPosition(LONGLONG *systime);
	OSTART ConvertSysTimeToSongPosition(LONGLONG systime);

	LONGLONG MIDIPlaybackCall();
	void InitMIDIPosition();

	void CheckMIDIAlarms(LONGLONG calarmsampleposition);
	void CheckMIDIAlarmsCheck(LONGLONG calarmsampleposition,LONGLONG *alarmcheck);

	OSTART GetStepTime(){return steptimes[stepstep];}
	OSTART GetStepLength(){return steptimes[steplength];}
	int GetSubTrackPosition(AutomationTrack *);

	bool CheckPlaybackRefresh(int flag=0);

	bool InitMetroAndStream(AudioDevice *);
	void InitNextPreMetro(AudioDevice *,int index,bool next);
	bool CheckPreMetronome(AudioDevice *); // Audio
	bool CheckPreMetronome(); // MIDI

	void EditSongPosition(guiWindow *,int x,int y,int smpteflag,int editid);
	void RestartSong(OSTART pos,int playsongflag,int flag);
	void SetSongPosition(OSTART position,bool restart,int flag=0);
	void SetSongPositionPrevOrNextMeasure(bool prev);

	void SetSongPositionWithMarker(OSTART pos,bool nextmarker);
	void FF();
	void REW();

	void SetCycleStart(OSTART position,bool changerange);
	void SetCycleEnd(OSTART position);
	void SetCycle(OSTART start,OSTART end);
	bool CheckCyclePositions(OSTART start,OSTART end);
	void SetCycleIndex(int index);

	void InitSongPlayback(InitPlay *);/*,bool onlycycle*/
	void CopyInitPlay(int to,int from);
	void InitAudioRecording();
	void RefreshAudioBuffer();
	void StartSongPlayback(int mode,int playstatus);
	void CheckCycleStart();
	void StartMIDI();

	void CycleReset_All(int index);
	void CycleReset_MIDI();
	void CycleReset_Stream(AudioDevice *);
	void CycleReset_Audio(AudioDevice *);

	void CreateRecordingCloneTrack(Seq_Track *,Seq_Track *clone,int cycleloop,bool copyaudio);
	void CheckAudioCycleRecordingForNewTracks(int cycleloop); // init new Cycle Tracks etc...
	void CheckMIDICycleRecordingForNewTracks(int cycleloop); // init new Cycle Tracks etc...

	void SongAudioDataInput(AudioDevice *,bool withhardware);
	void SyncAudioRecording(AudioDevice *);

	void DoPlayback(int mode,int playstatus);

	void SendPRepairNotes(bool cycle,int flag);
	void SendPRepairControl(bool cycle,int flag);

	void SetSysExMIDIPattern(MIDIPattern *,bool on);
	void SendSysExStartupMIDIPattern();
	void SendPreStartPrograms();

	void PRepairPreStartAndCycleEvents(OSTART pos,bool onlycycle=false); // + cycle

	void DeleteAllPreEvents(int flag);
	void PRepairPlayback(OSTART position,int mediatype);
	void InitSongPlaybackPattern(OSTART position,int mediatype,int index); // + used by Master
	void InitSongPlaybackPatternNewCycleStart(); 

	void SongInit();
	void SetSongName(char *,bool refreshgui);
	void SetSongDirectory(char *);
	bool CheckForOtherMIDI(Seq_Track *,bool output,bool onlyselected);

	// Automation
	void SetAutomationModeOfTracks(AutomationTrack *,int mode);
	void SetVisibleOfAutomationTrack(AutomationTrack *,bool visible);

	void SetAutomationTracksPosition(OSTART pos);
	void Automate(AutomationObject *,OSTART time,int pindex,double value,int iflag);
	bool AutomateBeginEdit(AutomationObject *ao,OSTART atime,int index);
	void AutomateEndEdit(AutomationObject *ao,OSTART atime,int index);

	void AddAudioMetronome(AudioDevice *device);

	void ToggleShowAutomationTracks(Seq_Track *);
	void ToggleShowAutomationChannels(AudioChannel *);
	void ResetAutomationTracks();
	void DeleteDeletedAutomationParameter();

	void MixNextMIDIPlaybackEvent(cMIDIPlaybackEvent *);
	void GetNextMIDIPlaybackEvent(cMIDIPlaybackEvent *);

	bool CheckForPattern(Seq_Pattern *);
	void PlaySong(int flag=0);
	void DoRealtimeAndCDEvents(AudioDevice *);
	void RefillAudioDeviceBuffer(AudioDevice *); // Main Audio mix Function
	void RefillAudioDeviceMaster(AudioDevice *); // Thread and Master

	void DropDownPeaks(AudioDevice *,bool withtracks);
	void MuteTrack(Seq_Track *,bool mute,OSTART automationtime=-1);
	void SoloTrack(Seq_Track *,bool solo,OSTART automationtime=-1);

	void RefreshExternSongTempoChanges();
	void MIDIInputDeviceToSongPosition(int flag);

	void SendMIDIOutPlaybackEvent(Seq_Track *,MIDIPattern *,Seq_Event *,LONGLONG playbackposition,int createflag);
	void SendMIDIOutPlaybackEventNoEffects(Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);

	void SendAllNoteOffs(int index,bool forcedelete,int flag);
	void SendAllNoteOffs(bool forcedelete,int flag);

	OSTART StopSong(int stopflag,OSTART songpos,bool setsongposition=true);
	void StopSelected(int flag=0);
	void InitRecordStatus();
	void SetOldRecordStatus();
	void RecordSong(int flag=0);
	void Pause();
	void SendOpenEvents();
	void StartThreadPlayback(int setstatus);
	void StopThreadPlayback();
	void CalcSongLength(){songlength_ticks=timetrack.ConvertMeasureToTicks(songlength_measure);}
	void InitInternSampleRate();

	OSTART GetSongLength_Measure(){return songlength_measure;}
	OSTART ConvertTicksToBeats(OSTART);
	OSTART ConvertBeatsToTicks(OSTART);

	bool ChangeTempoAtPosition(OSTART position,double newtempo,int flag);
	void SetSongLength(OSTART,bool guirefresh=true);
	OSTART GetSongLength_Ticks(){return songlength_ticks;}
	OSTART GetSongEnd_Pattern(); // use Pattern

	// Pattern Selection
	bool AddNewSelectionList(EventEditor *,Seq_SelectionList *);
	bool AddNewSelectionList(EventEditor *,Seq_SelectionList *,Seq_SelectionList *copyfrom);

	// Groups
	void SetGroupSolo(Seq_Group *,bool solo);
	void SetGroupRec(Seq_Group *,bool rec);

	Seq_Group* FirstGroup(){return (Seq_Group *)groups.GetRoot(); }
	Seq_Group* GetGroupIndex(int ix){return (Seq_Group *)groups.GetO(ix);}
	int GetCountGroups() {return groups.GetCount();}

	Seq_Group* CreateNewGroup();
	void SetTrackGroup(Seq_Track *,Seq_Group *);

	Seq_Group* DeleteGroup(Seq_Group *,bool removefromtracks=true);
	void DeleteAllGroups();

	void LoadGroups(camxFile *);
	void SaveGroups(camxFile *);
	void AddGroupToSelectedTracks(Seq_Group *);
	void RemoveBusFromGroupFromSelectedTracks(Seq_Group_GroupPointer *);

	void EditAutomationSettings(guiWindow *,Seq_Track *,AudioChannel *);
	void EditAutomation(guiWindow *,AutomationTrack *);

	// Pattern Links
	PatternLink *FirstPatternLink();
	PatternLink *CreatePatternLink();
	PatternLink *DeletePatternLink(PatternLink *);
	void DeletePatternFromLink(Seq_Pattern *);

	void LoadLinks(camxFile *);
	void SaveLinks(camxFile *);
	void SetGUIZoom(int ticks);
	void ConvertTicksToTempoSampleRate();
	void SetMousePosition(OSTART);

	void InitDefaultDrumMap();

	// GU
	RunningAudioFile* AddAudioPatternToPlayback_Realtime(AudioDevice *,Seq_Track *,LONGLONG startsample,LONGLONG endsample,AudioPattern *,int offset,int flag);
	RunningAudioFile *DeleteRunningAudioFile(OList *,RunningAudioFile *);
	RunningAudioFile *FirstRunningAudioFile(){return (RunningAudioFile*)playingaudiofiles.GetRoot();}
	int GetCountRunningAudioFiles(){return playingaudiofiles.GetCount();}

	void DeleteAllRunningAudioFiles();
	void RemovePatternFromAudioLists(AudioPattern *);

	Seq_MetroTrack *FirstMetroTrack(){return (Seq_MetroTrack *)metrotracks.GetRoot();}
	Seq_MetroTrack *DeleteMetroTrack(Seq_MetroTrack *);
	void AddMetroTrack(Seq_MetroTrack *);
	void InitMetroTrack_Clicks();

	void CreateQTrackList(){tracks.CreateList(&qtracklist);SetTrackIndexs();}
	void RefreshMIDIPatternEventIndexs();

	#ifdef WIN32
	inline void LockExternSync(){extern_sync.Lock();}
	inline void UnlockExternSync(){extern_sync.Unlock();}

	inline void LockCore(){core_sync.Lock();}
	inline void UnlockCore(){core_sync.Unlock();}
	inline void LockRAF(){raf_sync.Lock();}
	inline void UnlockRAF(){raf_sync.Unlock();}

	inline void LockSongPosition(){songposition_cs.Lock();}
	inline void UnlockSongPosition(){songposition_cs.Unlock();}

	CCriticalSection extern_sync,core_sync,raf_sync,songposition_cs,sema_refresh,sema_cyclesync,sema_recdata;
#endif

	FolderList tracks; // Data Tracks
	OOList qtracklist;
	LMIDIList events_in,events_out;
	AudioSystem audiosystem;
	OList metrotracks,playingaudiofiles,unabletoreadplugins,solopattern;
	Song_Playbacksettings playbacksettings;
	Seq_TextandMarker textandmarker;
	Seq_Time timetrack; // Tempo,Signature etc...	
	Seq_Realtime realtimeevents[REALTIME_LISTS]; // Realtime Events NoteOff etc...
	metroClick metronome;
	MIDISync MIDIsync;
	Undo undo;
	Seq_MIDIRouting inputrouting; // MIDI In->Track Routing
	Seq_Pos_Offset smpteoffset;
	Drummap drummap;
	
	char *GetName();

	Seq_Project *project;
	MIDIPattern *nextMIDIplaybackpattern[INITPLAY_MAX],*nextMIDIchainlistplaybackpattern[INITPLAY_MAX];
	Seq_Event *nextMIDIPlaybackEvent[INITPLAY_MAX];
	Seq_MIDIChainEvent *nextMIDIchainlistevent[INITPLAY_MAX];

	Seq_Track *mastertrack;
	Seq_Pattern *activepattern;
	Seq_Event *activeevent;

	// Core Help Pointer
	AudioChannel *firstfreecorechannel,*firstfreecorebus;
	Seq_Track *firstfreecoretrack,*firstfreethrutrack,*firstfreeaudioinputtrack;
	AudioHardwareChannel *firstfreecoredevicechannel;

	// Automation
	char *directoryname,*songname,*masterfile,*masterdirectoryselectedtracks;

	double buffer_nextMIDIplaybacksampleposition[INITPLAY_MAX],buffer_nextMIDIchaineventsampleposition[INITPLAY_MAX],
		buffer_record_songposition[2],
		buffer_playback_songposition,buffer_stream_songposition,pRepareMIDIstartprecounterms,default_masterpausems;

	LONGLONG nextMIDIplaybacksampleposition[INITPLAY_MAX],nextMIDIchaineventsampleposition[INITPLAY_MAX],
		nextautoMIDI_sample[INITPLAY_MAX],nextautoaudio_sample[INITPLAY_MAX],
		nextpulse,startsystemcounter64,
		playback_sampleposition,playback_sampleendposition, // Audio
		record_sampleposition[2],record_sampleendposition[2], // Audio
		recordedsamples,
		MIDI_samplestartposition,MIDI_sampleendposition, // MIDI Out
		stream_samplestartposition,stream_sampleendposition; // Audio Strean

	OSTART songposition,songstartposition,
		audioplayback_position,audioplayback_endposition, // Audio Buffer
		audiorecord_position,audiorecord_endposition,
		cyclestartposition,buffer_MIDI_songposition,
		masteringposition,masteringendposition,defaultmasterstart,defaultmasterend,mouseposition;

	int status,playback_samplesize,playback_setSize,playback_sampleoffset,playback_bufferindex, // Audio
		record_samplesize,record_setSize,record_sampleoffset,record_cyclecounter,record_audioindex, //Audio
		streamflag,stream_sampleoffset,stream_setSize,stream_buffersize,stream_bufferindex, // CreateStream
		recordcounter, // Tracks+Pattern
		trackchildflag,
		solocount,
		stepstep,steplength,
		refreshaudiofilescounter,
		punchrecording,
		notetype, // SI, H B ....
		masteringmode,MIDIaudiosynccounter,
		refreshcounter,cycleresetcounter[CYCLE_RESET_SYNCEND],
		generalMIDI,// GM Display
		fxshowflag,
		default_masterformat, // 16bit,24
		default_masterchannels, // MONO,Stereo
		focustype,
		freezeindexcounter,
		rinfo_audiopatterncounter,
		pref_tracktype; // CHANNELTYPE_STEREO etc...

	bool loaded,
		pRepareMIDIstartdelay,pRepareMIDIstartprecounter,
		startMIDIprecounter,
		checkinlatency,
		streamoutofsync,
		cycleoutofsync,
		audioinputneed,
		saveonlyarrangement,
		startaudioinit,startnextaudioinit, // Double Buffer Sync
		startMIDIinit, // Thread Sync Flag
		mastering,masteringpRepared,masteringstopflag,masteringlock,
		freeze,
		newexterntempochange,
		newsongpositionset,
		autoloadsong,autoloadMIDI,
		loadwhenactivated,
		steprecordingonoff,
		groupsoloed,
		MIDIrecording,
		waitforspaceup,
		underdeconstruction,
		startaudiorecording,errorsongwriting,
		default_masternormalize,default_masterpausesamples,default_mastersavefirst,
		mastefilename_autoset; // audiomastering

	UBYTE generalMIDI_drumchannel;

//	void StopAllExceptTrack(Seq_Track *); // After Solo

private:
	void AddSinglePatternToPRepairList(MIDIPattern *,OSTART pos,int index);
	void AddEventToPRepairList(Seq_Track *,Seq_Event *,MIDIPattern *,OSTART pos,OSTART delay,int index);

	// Audio-MIDI Sync
	void CheckAudioMIDISync();
	void ResetCycleSync();
	void LockRefreshCounter(){sema_refresh.Lock();}
	void UnlockRefreshCounter(){sema_refresh.Unlock();}
	bool CycleResetSync(int index); // Sync

	void LockCycleSync(){sema_cyclesync.Lock();}
	void UnlockCycleSync(){sema_cyclesync.Unlock();}

	void LockCopyRecordData(){sema_recdata.Lock();}
	void UnlockCopyRecordData(){sema_recdata.Unlock();}

	OList groups,patternlinks;
	OSTART songlength_measure, // Measures
		songlength_ticks; // Ticks

	int autosavecounter;
};
#endif