#ifndef CAMX_PATTERNOBJECTS_H
#define CAMX_PATTERNOBJECTS_H 1

#include "object.h"
#include "quantizeeffect.h"
#include "colourrequester.h"
#include "patternvolume.h"

class Seq_Song;
class Seq_Event;
class Seq_Pattern;
class Seq_CrossFade;
class RunningAudioFile;
class MIDIFilter;
class InitPlay;

#define AFTERLOAD_LOOP 1 // Flag for Audio Pattern Loops

class Seq_ClonePattern:public Object
{
public:
	Seq_ClonePattern(){insideundo=false;}

	Seq_ClonePattern *PrevClone() {return (Seq_ClonePattern *)prev;}
	Seq_ClonePattern *NextClone() {return (Seq_ClonePattern *)next;}

	Seq_Pattern *pattern;
	bool insideundo;
};

class Seq_LoopPattern:public Object
{
public:
	Seq_LoopPattern *PrevLoop() {return (Seq_LoopPattern *)prev;}
	Seq_LoopPattern *NextLoop() {return (Seq_LoopPattern *)next;}

	Seq_Pattern *pattern;
	OSTART loopstart;
	int loopindex;
};

class guiWindow;

class PatternLink_Pattern:public Object
{
public:
	PatternLink_Pattern *PrevLink() {return (PatternLink_Pattern *)prev;}
	PatternLink_Pattern *NextLink() {return (PatternLink_Pattern *)next;}

	Seq_Pattern *pattern;
};

class PatternLink:public Object // <- Pattern 1 <-> Pattern 2 etc...
{
public:
	PatternLink(Seq_Song *s){song=s;}

	PatternLink_Pattern *FirstLinkedPattern(){return (PatternLink_Pattern *)patternlinklist.GetRoot();}

	PatternLink_Pattern *AddPattern(Seq_Pattern *);
	bool FindPattern(Seq_Pattern *);
	bool RemovePattern(Seq_Pattern *); // true==closed

	PatternLink *PrevPatternLink() {return (PatternLink *)prev;}
	PatternLink *NextPatternLink() {return (PatternLink *)next;}

	void Load(camxFile *);
	void Save(camxFile *);

	int GetCountOfPattern(){return patternlinklist.GetCount();}
private:
	Seq_Song *song;
	OList patternlinklist;
};


class Seq_Pattern:public OStart
{
	friend class Seq_Track;
	friend class Seq_Song;
	friend class Seq_Main;
	friend class EditFunctions;
	friend class Undo;

public:
	enum{
		PATTERNLOOP_MEASURE,
		PATTERNLOOP_BEAT,
		PATTERNLOOP_NOOFFSET
	};

	Seq_Pattern();

	virtual void InitOffSetEdit(int mode){}
	virtual bool SetOffSetStart(OSTART pos,LONGLONG offset,bool test){return false;}
	virtual bool SetOffSetEnd(LONGLONG offset,bool test){return false;}
	virtual LONGLONG GetOffSetStart(){return 0;}
	virtual LONGLONG GetOffSetEnd(){return 0;}
	virtual bool SetStart(OSTART pos)=0;
	virtual bool SetNewLoopStart(OSTART pos)=0;
	virtual void StopAllofPattern()=0;
	virtual bool SizeAble(){return true;}
	virtual Seq_Event *FirstEditEvent()=0;
	virtual bool AddGMSysEx(bool gs,bool refreshgui){return false;}

	virtual OSTART GetPatternStart(){return ostart;} //object start
	virtual OSTART GetPatternEnd(){return ostart;}
	virtual int GetAccessCounter(){return 0;}
	virtual void CloneFX(Seq_Pattern *){}
	virtual void Clone(Seq_Song *,Seq_Pattern *,OSTART startdiff,int flag){}
	virtual Seq_Pattern *CreateClone(OSTART startdiff,int flag){return 0;}
	virtual Seq_Pattern *CreateLoopPattern(int loop,OSTART pos,int flag){return 0;}
	virtual int QuantizePattern(QuantizeEffect *){return 0;}
	virtual int GetCountOfEvents(){return 0;}
	virtual int GetCountOfEvents(MIDIFilter *){return 0;}

	virtual bool InitPlayback(InitPlay *,int mode){return false;}
	virtual void RefreshAfterPaste(){}
	virtual Seq_Event *FindEventAtPosition(OSTART position,int filter,int icdtype){return 0;}
	virtual void Load_Ex(camxFile *,Seq_Song *){}
	virtual void Save_Ex(camxFile *){}
	virtual void SetName(char *);
	virtual OList *GetEventList(){return 0;}
	virtual bool CanBeLooped(){return false;}
	virtual void MovePatternData(OSTART diff,int flag){}
	virtual void RefreshIndexs(){}
	virtual void CloseEvents(){} // Note-Note Chain MIDI Olny

	void InitStartPositions(OSTART *ps,OSTART *pe);

	enum{
		MOVENO_STATIC=1
	};
	
	bool CheckIfPlaybackIsAble(){return p_muteflag==true?false:true;}
	bool GetUseOffSetRegion(){return itsaclone==true?mainpattern->useoffsetregion:useoffsetregion;}

	bool CheckIfPatternOrFromClones(Seq_Pattern *);
	bool EditAble();
	void ShowPatternName(guiWindow *);
	void MovePattern(OSTART diff,int flag);

	QuantizeEffect *GetQuantizer();

	// Loop 
	void LoopPattern(int flag=0); // mediatype return
	bool CheckLoopPattern(int flag=0); // r:true difference
	void DeleteLoops();	
	void CloneLoops(Seq_Pattern *);

	Seq_LoopPattern *GetLoop(int index);
	int GetLoopIndex(Seq_Pattern *);

	// Clones
	Seq_ClonePattern *FirstClone(){return (Seq_ClonePattern *)clonepattern.GetRoot();}

	void AddClone(Seq_Pattern *);
	void RemovePatternFromClones(Seq_Pattern *);
	void CutClones();
	Seq_ClonePattern *DeleteClone(Seq_ClonePattern *,bool full);
	void DeleteClones();
	void SetClonesOffset();
	void SetOffset();

	Seq_LoopPattern *FirstLoopPattern(){return (Seq_LoopPattern *)looppattern.GetRoot();}
	Seq_Track *GetTrack(){return track;}
	Seq_Pattern *NextRealPattern();
	Seq_Pattern *NextPattern(){return (Seq_Pattern *)next;}
	Seq_Pattern *PrevPattern(){return (Seq_Pattern *)prev;}
	Seq_Pattern *NextPattern(int selmediatype);
	Seq_Pattern *PrevPattern(int selmediatype);

	bool CheckIfInRange(OSTART start,OSTART end,bool quantmeasure);
	char *GetName();
	void LoadStandardEffects(camxFile *);
	void SaveStandardEffects(camxFile *);
	void LoadFromFile(char *);
	void SaveToMIDIFile(guiWindow *); // +req
	void SaveToFile(guiWindow *); //+req

	Seq_CrossFade *FirstCrossFade(){return (Seq_CrossFade *)crossfades.GetRoot();}
	inline void Lock_CrossFades(){sema_crossfades.Lock();}
	inline void Unlock_CrossFades(){sema_crossfades.Unlock();}
	void RemovePatternFromOtherCrossFades();
	void DeleteAllCrossFades(bool full,bool removefrompattern=true);
	void MarkAllCrossFades();
	Seq_CrossFade *FindCrossFade(Seq_Pattern *);
	void AddCrossFade(Seq_CrossFade *);
	bool CheckIfCrossFadeUsed();
	void DeleteCrossFade(Seq_CrossFade *);
	void DeleteMarkedCrossFades();
	virtual bool IfOffSetStart(){return false;}
	virtual bool IfOffSetEnd(){return false;}
	void ResetLoops();
	Colour *GetColour();

	OList looppattern, // Seq_LoopPattern
	clonepattern,// Seq_ClonePattern
	crossfades; // >> CrossFade

	Colour t_colour;
	QuantizeEffect quantizeeffect;

	virtual void InitDefaultVolumeCurve(){}
	Seq_Pattern_VolumeCurve *GetVolumeCurve();
	Seq_Pattern_VolumeCurve volumecurve;

	char *patternname;
	Seq_Track *track;
	Seq_Pattern *mainpattern, // main loop
		*mainclonepattern; // main clone pattern

	PatternLink *link;
	LONGLONG offset_samples;
	OSTART offset,offsetstartoffset,loadoffset;
	
	int  // diff to main pattern >0 (loops+clones)
		recordcounter, // -1=not recorded
		loops,loopindex,
		mainclonepatternID,
		patternID,
		mediatype,
		loopflag;

	bool itsaloop,itsaclone,loopendless,recordeventsadded,
		checknotechain, // Note -- Note Chain -MIDI ONLY
		loopwithloops, // use loops
		p_muteflag,
		useoffsetregion,eventclose,
		mute,recordpattern,
		visible,realpattern; // Frozen Pattern=false

private:
#ifdef WIN32
	CCriticalSection sema_crossfades;
#endif
};

#endif