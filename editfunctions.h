#ifndef CAMX_EDITFUNCTIONS_H
#define CAMX_EDITFUNCTIONS_H 1

//#include "object.h"

#define CNP_NOGUIREFRESH 1
#define CNP_NOCHECKPLAYBACK 2
#define CNP_ADDTOLASTUNDO 4
#define CNP_NOEDITOKCHECK 8

class TrackHead;
class guiWindow;
class Seq_Track;
class AutomationTrack;
class Seq_Song;
class Seq_SelectionList;
class Seq_Event;
class Seq_Pattern;
class MIDIPatten;
class MIDIEffects;
class mempool_ELEpool;
class MIDIPattern;
class AudioObject;
class AudioChannel;
class AudioPattern;
class UndoFunction;
class EF_CreateEvent;
class AudioRegion;
class AudioHDFile;
class QuantizeEffect;
class Drummap;
class Note;
class AutomationObject;
class MIDIOutputDevice;
class MIDIInputDevice;
class Undo_CreatePattern;
class Edit_Piano;
class Undo_DeletePattern;
class UndoEdit;
class AudioFileWork;
class Undo_CreateTracks;
class Undo_AddTracksAsChild;
class AutomationParameter;
class Undo_ChangeAutomationParameter;
class InsertAudioEffect;
class AudioEffects;

class MoveO
{
public:
	MoveO()
	{
		diff=0;
		index=flag=filter=filter2=0;
		sellist=0;
		dindex=0;
		allpattern=selectedpattern=selectedtracks=cyclecheck=special=resetdbvalues=false;
		quantize=true;
	}

	Seq_Song *song;
	Seq_Track *track;
	Seq_SelectionList *sellist;
	OSTART diff;
	double dindex;
	int index,flag,filter,filter2,icdtype;
	bool allpattern,cyclecheck,selectedpattern,selectedtracks,special,resetdbvalues,quantize;
};

class CreateNewTrackAudio
{
public:
	CreateNewTrackAudio()
	{
		ok=false;
		createtracklist=false;
		count=0;
		tracks=0;
	}

	~CreateNewTrackAudio()
	{
		if(tracks)
		{
			TRACE ("Delete CreateNewTrackAudio Tracks\n");
			delete tracks;
		}
	}
	Seq_Track **tracks;
	OList *list;
	OSTART position;

	int count;
	bool ok,createtracklist;
};

class EditFunctions
{
public:
	EditFunctions()
	{
		locked=0;
		unlock=0;
	}

	void LockEdit(){locked++;}
	void UnlockEdit(){locked--;}

	bool CheckIfEditOK(Seq_Song *);
	void Lock(Seq_Song *);
	void OpenLockAndDo(Seq_Song *,UndoFunction *,bool undocheck);
	void OpenLockAndDo(Seq_Song *,UndoFunction *,bool undocheck,bool addtoundo);
	void LockAndDoFunction(Seq_Song *,UndoFunction *,bool undocheck);
	void CheckPlaybackAndUnLock();
	void UnLock(); // for manual Playback Refresh

	// Arrange
	void SetSongTracksToAudioIO(Seq_Track *,bool selected);

	void SetSongTracksToMIDIInput(Seq_Track *,bool selected);

	void SetMIDIInputAllDevices(Seq_Track *,bool onoff);
	void AddMIDIInput(Seq_Track *,int portindex);
	void RemoveMIDIInput(Seq_Track *,MIDIInputDevice *);
	void ReplaceMIDIInput(Seq_Track *,int oldindex,int newindex);

	void SetSongTracksToMIDIOutput(Seq_Track *,bool selected);
	void AddMIDIOutput(Seq_Track *,int portindex);
	void RemoveMIDIOutput(Seq_Track *,int portindex);
	void ReplaceMIDIOutput(Seq_Track *,int oldindex,int newindex);

	bool SplitTrackToChannels(Seq_Track *);
	bool SplitPatternToChannels(MIDIPattern *);
	bool SplitPatternToEvents(MIDIPattern *);
	bool FlipPattern(MIDIPattern *);
	bool StretchPattern(MIDIPattern *,int to);

	void EditEvents(Seq_Song *,UndoEdit *);

	// Event Editors
	void QuantizeEventsMenu(guiWindow *,Seq_SelectionList *);
	void QuantizeEvents_Execute(Seq_Song *,Seq_SelectionList *,int length_index); // called by menu

	// Piano
	void SetNoteLength(guiWindow *,Seq_SelectionList *);
	void SetNoteLength_Execute(Seq_Song *,Seq_SelectionList *,int length_index); // called by menu

	// Drum
	void ConvertNotesToDrums(Seq_SelectionList *,Drummap *);
	void ConvertDrumsToNotes(Seq_SelectionList *);

	// Cut
	void CutPattern(Seq_SelectionList *,OSTART cutposition);
	void CutNote(Note *,OSTART cutposition);

	// Delete Functions
	int DeleteEvent(Seq_Event *,bool addtolastundo);
	int DeleteEvents(Seq_SelectionList *,bool addtolastundo);
	
	Undo_DeletePattern *DeletePattern(Seq_Song *,Seq_SelectionList *,Seq_Pattern *,bool addtolastundo);
	void MoveTracks(Seq_Song *,int diff);
	void DeleteAllEmptyTracks(Seq_Song *);
	void DeleteSelectedTracks(Seq_Song *,Seq_Track *singletrack=0);
	void SortTracks(Seq_Song *);

	enum{
		CREATETRACK_ACTIVATE=(1<<0),
		CREATETRACK_NODOLOCK=(1<<1),
	};

	enum{
		DELETESONG_FLAG_DELETEFILES=1,
		DELETESONG_FLAG_SETNEXTSONGAUTO=2,
		DELETESONG_FLAG_NOGUIREFRESH=4
	};

	void DeleteSong(guiScreen *screen,Seq_Song *,int flag);

	// Create new Functions
	Seq_Track *CreateNewTrack(UndoFunction *,Seq_Song *,Seq_Track *clonetrack,int index,bool doundo,bool activate,bool createclonename=false);
	Undo_AddTracksAsChild *AddSelectedTrackToTrackAsChild(Seq_Song *,Seq_Track *,bool doit=true);

	void CreateNewParentAndAddSelectedTracks(Seq_Song *,Seq_Track *next,Seq_Track *clone);
	int CreateNewTracks(Seq_Song *,Seq_Track *next,int number,int flag,Seq_Track *clone,CreateNewTrackAudio *newaudiotracks=0);
	Seq_Track **CreateNewChildTracks(Seq_Song *,Seq_Track *next,int number,int flag,Seq_Track *parent,Seq_Track *clone=0);
	void RemoveSelectedTrackFromParent(Seq_Song *);

	int CreateNewBusChannels(Seq_Song *,AudioChannel *next,int number,Seq_Track *clone);
	void DeleteBusChannels(Seq_Song *);

	Seq_Pattern *CreateNewPattern(UndoFunction *,Seq_Track *,int type,OSTART startpos,bool doundo,int flag=0,OList *list=0,Seq_Pattern *mainclonepattern=0);

	void CreateNewMIDIEvents(MIDIPattern *to,OList *,OSTART position);
	void CreateNewMIDIEvent(EF_CreateEvent *);

	// Automation
	void CreateAutomationParameter(Seq_Song *,AutomationTrack *,AutomationParameter *);
	void DeleteAutomationParameter(Seq_Song *,AutomationTrack *,AutomationParameter *);
	void ResetAutomationParameter(Seq_Song *,AutomationTrack *,AutomationParameter *);
	void DeleteSelectedAutomationObjects(Seq_Song *);
	Undo_ChangeAutomationParameter *InitChangeAutomation(AutomationTrack *);
	void CreateAutomationTrack(AutomationTrack *track);

	void DeleteAutomationTrack(AutomationTrack *);
	void CreateMixBuffer(Seq_SelectionList *);

	void QuantizePatternMenu(guiWindow *,Seq_SelectionList *,bool force=false);
	void QuantizePattern_Execute(Seq_Song *,Seq_SelectionList *,int notelength); // called by menu

	// Edit
	void EditEventData(Seq_Event *,EF_CreateEvent *);

	// Clone
	bool ClonePatternList(MoveO *);
	bool MovePatternList(MoveO *);
	void SizePattern(Seq_Pattern *,OSTART pos,LONGLONG sampleoffset,bool right);
	bool CopyPatternList(MoveO *);

	bool CopySelectedEventsInPatternList(MoveO *);
	bool MoveSelectedEventsInPatternList(MoveO *);

	bool SizeNote(Seq_Song *,Note *,OSTART newend,bool addtoundo);
	bool SizeSelectedNotesInPatternList(Seq_Song *,Seq_SelectionList *,OSTART diff,int flag,bool startorend);

	// Tempo
	void CreateNewTempo(Seq_Song *,OSTART pos,double tempo,bool addtoundo);
	bool CopySelectedTempos(Seq_Song *,OSTART tickdiff,double tempodiff,int flag);
	void DeleteSelectedTempos(Seq_Song *,bool addtoundo);
	void EditSelectedTempos(MoveO *);

	// Marker
	void DeleteSelectedMarkers(Seq_Song *,bool addtoundo);

	// Text
	void DeleteSelectedTexts(Seq_Song *,bool addtoundo);

	// Quantize
	void QuantizeTrack(Seq_Track *,QuantizeEffect *,bool force=false);
	void QuantizePattern(Seq_Pattern *,QuantizeEffect *,bool force=false);

	// Paste
	void PasteSelectionListToSong(Seq_SelectionList *,Seq_Song *,Seq_Track *firsttrack,OSTART position);
	UndoFunction *PasteObjectToTrack(Seq_Track *,Object *pattern,OSTART position,int flag);

	// div
	void MutePattern(Seq_Song *,Seq_SelectionList *,Seq_Pattern *single,bool mute);
	bool LoopPattern(Seq_Pattern *, bool loopendless,bool loopwithloops,int loops);

	// Plugins
	void InsertPlugin(AudioEffects *,AudioObject *,bool openeditor=true);
	void ReplacePlugin(AudioEffects *,InsertAudioEffect *oldeffect,AudioObject *);
	void DeletePlugin(InsertAudioEffect *);
	void InsertPlugins(AudioEffects *from,AudioEffects *to); //Group
	void DeletePlugins(AudioEffects *); // Group

	// AudioPattern
	AudioPattern *CreateAudioPattern(Seq_Track *,AudioHDFile *,AudioRegion *,OSTART position,guiWindow *);
	bool LoadSoundFile(guiWindow *,Seq_Track *,char *,OSTART position);
	Undo_CreatePattern *InitNewAudioPattern(guiWindow *,Seq_Track *,char *,OSTART sfposition,bool lock,AudioPattern *clonefrom=0,AudioPattern *mainclonepattern=0,Undo_CreateTracks *addto=0);
	
	bool LoadSoundFileNewTracks(guiWindow *,Seq_Track *,OSTART position);
	bool LoadSoundFileAndSplitNewTracks(guiWindow *,Seq_Track *,OSTART position);
	bool LoadSoundFileDirectoryNewTracks(guiWindow *,Seq_Track *,OSTART position);

	bool AddRecordingToUndo(Seq_Song *);
	bool ChangeRegion(AudioRegion *,LONGLONG start,LONGLONG end);
	int RemoveAudioRegionFromProjects(AudioRegion *);
	void ReplaceAudioPatternHDFile(AudioPattern *,AudioHDFile *,AudioRegion *region=0);

	// MIDI File ->Pattern
	bool LoadMIDIFile(Seq_Track *,char *,OSTART mfposition,guiWindow *);
	bool LoadPatternFile(Seq_Track *,char *,OSTART mfposition,guiWindow *);

	void ConvertCloneToPattern(Seq_Pattern *);	
	void ConvertLoopToPattern(Seq_Pattern *,bool convert); // convert true=create events inside old pattern
	void ConvertAllLoopsToPattern(MIDIPattern *);

	void MixAllSelectedPatternToPattern(Seq_SelectionList *,MIDIPattern *,bool add,bool connect=false);	

	// Edit<->GUI
	bool CheckIfWindowRefreshObjects(Seq_Song *,guiWindow *,UndoFunction *); // Check Editor type, with edit ID
	void CheckEditElementsForGUI(Seq_Song *,UndoFunction *,bool undocheck); // and close Edit Element list

	void CheckFunctionForGUI(Seq_Song *,UndoFunction *,bool checknext);

private:
	char *InitNewAudioFile(char *newfile,char *directoryname);
	bool InitNewAudioFile(AudioHDFile *,char *directoryname,char **waitfornewfile,bool *doresample,AudioFileWork **afwork,AudioHDFile **usehdfile);

	OList elements;
	Seq_Song *unlock;
	int unlockmediatype,locked;
};

#endif
