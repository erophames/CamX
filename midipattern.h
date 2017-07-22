#ifndef CAMX_MIDIPattern
#define CAMX_MIDIPattern 1

#include "object.h"
#include "objectpattern.h"
#include "objectevent.h"
#include "MIDIoutdevice.h"
#include "mempools.h"
#include "MIDIeffects.h"

class MIDIPatternInfo
{
public:
	MIDIPatternInfo(){notes=control=sysex=pitch=prog=polypress=cpress=intern=0;};

	int DifferentEventTypes(){
		int c=0;

		if(notes)c++;
		if(control)c++;
		if(sysex)c++;
		if(pitch)c++;
		if(prog)c++;
		if(polypress)c++;
		if(cpress)c++;
		if(intern)c++;

		return c;
	}

	int notes,control,sysex,pitch,prog,polypress,cpress,intern;
};

class MIDIPattern:public Seq_Pattern
{	
	friend Seq_Song; // cycle reset
	friend mainMIDIBase;
	friend Seq_Event; // for Note access
	friend Note; // for Note access
	friend SysEx;

public:
	MIDIPattern();

	Seq_Event *FirstEditEvent(){return FirstEvent();}

	bool SetStart(OSTART pos){return false;}
	bool SetNewLoopStart(OSTART pos);
	void StopAllofPattern();

	void AddBankProgRealtime(OSTART init_position,bool calledbycycle);
	void Load(camxFile *); // v
	void Save(camxFile *); // v
	void RefreshIndexs();
	void SaveToSysExFile(guiWindow *);

	bool AddGMSysEx(bool gs,bool refreshgui);
	bool CheckObjectID(int cid){return (cid==OBJ_MIDIPattern) || (cid==OBJ_PATTERN)?true:false;}

	int GetAccessCounter(){return events.accesscounter;}

	LONGLONG GetSampleStart(Seq_Song *); // + Tempo

	OSTART GetPatternStart(); // v
	OSTART GetPatternEnd(); // v

	void AddSortEvent(Seq_Event *,OSTART start);
	void AddSortEvent(Seq_Event *);

	//  Virtual
	void AddSortVirtual(Seq_Event *,OSTART start);
	void AddSortVirtual(Seq_Event *);
	void DeleteVirtual(Seq_Event *); // NoteOffs, SysExEnd etc...

	void Delete(bool full);

	void ConvertOpenRecordingNotesToNotes(OSTART endpos);
	void DeleteOpenNotes();

	int QuantizePattern(QuantizeEffect *);
	bool CheckEventsWithChannel(UBYTE channel);

	void InitSwing();
	bool InitPlayback(InitPlay *,int mode);
	void SkipPlaybackMIDIEvents(int index,int mode);

	UBYTE CheckPatternChannel(); // 0=thru, 1-16
	void CreatePatternInfo(MIDIPatternInfo *);
	int GetDominantMIDIChannel();
	bool CanBeLooped(){return FirstEvent()?true:false;}

	void CloneFX(Seq_Pattern *to); // v
	void Clone(Seq_Song *,Seq_Pattern *,OSTART startdiff,int flag); // virtual
	Seq_Pattern *CreateLoopPattern(int loop,OSTART pos,int flag); // virtual
	Seq_Pattern *CreateClone(OSTART startdiff,int flag); // 

	Seq_Event *FindEventAtPosition(OSTART pos);
	Seq_Event *FindEventAtPosition(OSTART pos,int filter,int icdtype);
	Seq_Event *FindEventInRange(OSTART startposition,OSTART endposition,int flag);
	Seq_Event *DeleteEvent(Seq_Event *,bool full);

	void DeleteEvents();
	void CloneEventsWithChannel(Seq_Song *,int i,MIDIPattern *); // split function
	void CloneEvents(Seq_Song *,MIDIPattern *);
	void CloneMixEventsToPattern(Seq_Song *,MIDIPattern *); // mix to pattern
	void MixEventsToPattern(Seq_Song *,MIDIPattern *); // mix to pattern

	void MoveEventsWithChannel(int i,MIDIPattern *); // split
	void MoveEventsWithType(UBYTE status,MIDIPattern *); // split
	void MoveAllEvents(MIDIPattern *,Seq_Event *e=0);

	OList *GetEventList(){return &events;}
	bool IsPatternSysExPattern();
	bool SendStartupSysEx();

	Note *NewNote(OSTART,OSTART staticstart);
	Note *NewNote(OSTART);
	PolyPressure *NewPolyPressure(OSTART);
	ControlChange *NewControlChange(OSTART);
	ProgramChange *NewProgramChange(OSTART);
	ChannelPressure *NewChannelPressure(OSTART);
	Pitchbend *NewPitchbend(OSTART);
	SysEx *NewSysEx(OSTART);

	int GetCountOfSysEx();
	int GetCountOfEvents(){return events.GetCount();}
	int GetCountOfEvents(MIDIFilter *);

	Seq_Event *FirstEvent();
	Seq_Event* FirstVirtualEvent();
	Seq_Event *LastEvent();
	Seq_Event* LastVirtualEvent();

	void SetFirstEvent(Seq_Event *e){events.SetRoot(e);}
	void SetLastEvent(Seq_Event *e){events.Setc_end(e);}

	void SetFirstVirtualEvent(Seq_Event *e){virtualevents.SetRoot(e);}
	void SetLastVirtualEvent(Seq_Event *e){virtualevents.Setc_end(e);}

	void MovePatternData(OSTART diff,int flag);

	MIDIEffects *GetMIDIFX(){return mainpattern?&((MIDIPattern *)mainpattern)->t_MIDIeffects:&t_MIDIeffects;}

	// Record Open Notes
	inline NoteOpen *FirstOpenNote(){return (NoteOpen *)openrecnotes.GetRoot();}

#ifdef WIN32
	inline void LockOpenNotes(){openrecnotes_semaphore.Lock();}
	inline void UnlockOpenNotes(){openrecnotes_semaphore.Unlock();}
	CCriticalSection openrecnotes_semaphore;
#endif

	void BuildChainEventList();
	void CloseEvents(); // Index+Build NoteList

	MIDIEffects t_MIDIeffects;
	MIDIOutputProgram MIDIprogram;

	OListStart events, // *Seq_Event
		virtualevents, // Noteoffs, SysExEnd
		openrecnotes; // open MIDI record note on's

	Seq_Event *playback_MIDIevent[INITPLAY_MAX][2]; // next Playback Event, [2] 0=Playback,1=Cycle

	Seq_MIDIChainEvent *playback_nextchainevent[INITPLAY_MAX][2], // Swing Notes etc...
		*firstchainevent,*lastchainevent; // Swing Notes etc...

	bool sendonlyatstartup;
};	

#endif