#ifndef CAMX_PATTERNSELECTION_H
#define CAMX_PATTERNSELECTION_H 1

#include "object.h"
#include "defines.h"

class Seq_Pattern;
class Seq_Track;
class Seq_Event;
class Seq_MIDIEvent;
class EventEditor;
class MIDIFilter;
class PatternLink;
class UndoEdit;
class mempool_SelectionEvent;
class MoveO;

class Seq_SelectedPattern:public Object
{
	friend class Seq_SelectionList;
	friend class Seq_Song;

public:
	Seq_SelectedPattern()
	{
		pattern=0;
		prevevent=nextevent=0;
		tracknumberinsong=0;
		status_nrpatternevents=nrevents=0;
		patternname=trackname=0;
	}

	Seq_SelectedPattern *NextSelectedPattern() {return(Seq_SelectedPattern *)next;}
	Seq_SelectedPattern *PrevSelectedPattern() {return(Seq_SelectedPattern *)prev;}	
	Seq_SelectedPattern *NextOrPrevSelectedPattern() {return(Seq_SelectedPattern *)NextOrPrev();}

	void SetPrevEvent(int filter,int icdtype);
	void SetNextEvent(int filter,int icdtype);

	Seq_Pattern *pattern;
	Seq_Event *prevevent,*nextevent;
	
	char *patternname,*trackname;

	int tracknumberinsong, //0=firstrack
		status_nrpatternevents, // realtime refresh
		nrevents,
		trackindex;
};

enum{
	SEQSEL_ONDISPLAY=1,
	SEQSEL_REALTIMEACTIVATED=2,
	SEQSEL_FRESHRESELECTED=4,
	SEQSEL_MOUSEOVER=8
};

class Seq_SelectionEvent:public OStart
{
public:
	Seq_SelectionEvent()
	{
		Init();
	}

	void Init()
	{
		changedlength=0;
		flag=0;
	}

	void AddMoreEvent(Seq_SelectionEvent *more){moreevents.AddEndO(more);}

	Seq_SelectionEvent *NextEventOnDisplay();
	Seq_SelectionEvent *NextSelectedEvent();
	Seq_SelectionEvent *NextSelectedEvent(Seq_Event *);
	Seq_SelectionEvent *NextEvent() {return(Seq_SelectionEvent *)next; }
	Seq_SelectionEvent *PrevEvent() {return(Seq_SelectionEvent *)prev; }
	void ClearMoreEvents(){moreevents.DeleteAllO();}
	Seq_SelectionEvent *FirstMoreEvent(){return (Seq_SelectionEvent *)moreevents.GetRoot();}

	Seq_Event *seqevent;
	OSTART changedlength;
	int x,y,x2,y2,helpvalue,flag;

#ifdef MEMPOOLS
	mempool_SelectionEvent *pool;
#endif

private:
	OList moreevents; // multi display
};

class Seq_SelectionList:public Object
{
	friend Seq_Song;
	friend class EventEditor_Selection;

public:
	Seq_SelectionList();
	void ClearEventList();
	void DeleteSelectionList();

	bool Compare(Seq_SelectionList *);

	void ClearFlags();
	void ClearFlag(int flag);
	bool CheckForMediaType(int);
	Seq_SelectionEvent *FindEvent(Seq_Event *);
	
	Seq_SelectionEvent *FirstMixEvent(){return (Seq_SelectionEvent *)events.GetRoot();}
	Seq_SelectionEvent *FirstMixEvent(OSTART start);
	Seq_SelectionEvent *LastMixEvent(){return (Seq_SelectionEvent *)events.Getc_end();}
	Seq_SelectionEvent *GetMixEventAtIndex(int index){return (Seq_SelectionEvent *)events.GetO(index);}
	
	bool CheckEventList(Seq_Event *,MIDIFilter *);
	void BuildEventList(int filter,MIDIFilter *,int icdtype=0);
	int GetIndexOfEvent(Seq_Event *);

	bool CheckEventList(); // return true=refresh
	void DeleteAllPattern();
	void Delete(bool full);
	void Clone(Seq_Song *,Seq_SelectionList *,OSTART diff,int flag);
	void CopyStatus();
	void ResetChanges();
	void ResetSelection();
	int GetCountOfEvents(){return events.GetCount();}
	Seq_SelectionEvent *FirstSelectedEvent();
	Seq_SelectionEvent *FirstSelectedEvent(Seq_Event *);
	UndoEdit *CreateEditEvents(Seq_Event *,int tab);
	int GetCountofSelectedEvents();
	int GetCountofSelectedEvents(Seq_Event *);
	int GetCountofSelectedEvents_Filter(int filter,int icdtype=0);
	int GetCountOfRealSelectedPattern();
	int GetOfRealPattern(Seq_Pattern *);
	int GetCountOfSelectedPattern(){return pattern.GetCount();}
	int GetCountOfLinkPatternSelectedPattern();
	int GetCountOfLinkPatternSelectedPattern(PatternLink *);
	Seq_SelectedPattern *FirstSelectedPattern() {return (Seq_SelectedPattern *)pattern.GetRoot(); }
	Seq_SelectedPattern *FindPattern(Seq_Pattern *);
	Seq_SelectedPattern *FindSelectedPattern(Seq_Pattern *);
	OSTART FirstPatternPosition();
	int FirstTrackNumber();
	int LastTrackNumber();
	Seq_SelectedPattern* LastSelectedPattern() {return (Seq_SelectedPattern *)pattern.Getc_end(); }
	Seq_Track *FindTrack(Seq_Track *);
	int GetOfPattern(Seq_Pattern *);
	Seq_SelectedPattern *DeletePattern(Seq_SelectedPattern *p){return (Seq_SelectedPattern *)pattern.RemoveO(p);}
	Seq_SelectedPattern *AddPattern(Seq_Pattern *);
	bool CheckIfPatternInList(Seq_Pattern *);
	bool SelectPattern(Seq_Pattern *);
	bool UnselectPattern(Seq_Pattern *);
	bool UnselectTrack(Seq_Track *);

	bool CheckMove(MoveO *);
	Seq_Event *FirstSelectionEvent(OSTART start,int addfilter);
	Seq_Event *NextSelectionEvent(int addfilter);
	
	void SelectAllEvents(bool select,bool showgui=true);
	void SelectAllEvents(bool select,UBYTE status,char byte1,bool showgui=true);
	void SelectAllEventsNot(Seq_Event *not,bool select,int filter,bool showgui=true);

	bool SelectEvent(Seq_Event *,bool select);
	void SelectFromTo(Seq_Event *from,Seq_Event *to);
	void ToggleSelection(Seq_Event *);

	Seq_Event *GetCursor(){return cursorevent;}
	void SetCursor(Seq_Event *e){cursorevent=e;}

	Seq_SelectionList *NextSelection(){return (Seq_SelectionList *)next;}
	Seq_SelectedPattern *MixNextSelection();

	Seq_Song *song;
	EventEditor *editor;
	Seq_Pattern *solopattern;
	Seq_Event *cursorevent;
	Seq_SelectionEvent *seqlist;

	OSTART openstartposition,movediff;
	int moveobjects_vert,firsttracknumber,buildfilter,buildicdtype,edittab;
	bool refresh,patternremoved,showonlyselectedevents,deletethis,playsolo;

private:
	OList pattern; // Mixed Event List
	OListStart events;
};
#endif