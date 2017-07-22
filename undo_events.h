#ifndef CAMX_UNDOFUNCTIONS_EVENTS_H
#define CAMX_UNDOFUNCTIONS_EVENTS_H 1

#include "defines.h"
#include "undo.h"
#include "languagefiles.h"

class ICD_Drum;
class Seq_SelectionList;

class UndoSetNoteLength
{
public:
	Note *note;
	OSTART oldlength;
};

class UndoCopyEvent
{
public:
	Seq_Event *seqevent, // org
		*clone; // clone of event
};

class UndoMoveEvent
{
public:
	Seq_Event *seqevent;
	OSTART diff;
	int oldindex;
};

class UndoSizeNote
{
public:
	Note *note;
	OSTART changelength,oldstart,oldend;
};

class Undo_SizeNotes:public UndoFunction
{
public:
	Undo_SizeNotes(Seq_Song *s,UndoSizeNote *e,int nr,bool setstart)
	{
		id=Undo::UID_SIZEOFNOTES;
		song=s;
		events=e;
		numberofevents=nr;
		startorend=setstart;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_CHANGELENGTHOFNOTES]);}

	void FreeData()
	{
		if(events)delete events;
	}

	UndoSizeNote *events;
	int numberofevents;
	bool startorend;
};

class Undo_CopyEvents:public UndoFunction
{
public:
	Undo_CopyEvents(Seq_Song *s,UndoCopyEvent *e,int nr,OSTART diff,int kdiff)
	{
		id=Undo::UID_COPYEVENTS;
		song=s;
		events=e;
		movediff=diff;
		keydiff=kdiff;
		numberofevents=nr;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_COPYEVENTS]);}

	void FreeData()
	{
		if(events)
			delete events;
	}

	Seq_Song *song;
	UndoCopyEvent *events;

	OSTART movediff;
	int numberofevents,keydiff;
	bool copied;
};

class Undo_MoveEvents:public UndoFunction
{
public:
	Undo_MoveEvents(Seq_Song *s,UndoMoveEvent *e,int nr,int ix)
	{
		id=Undo::UID_MOVEEVENTS;
		song=s;
		events=e;
		index=ix;
		numberofevents=nr;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_MOVEEVENTS]);}

	void FreeData(){if(events)delete events;events=0;}

	UndoMoveEvent *events;
	int numberofevents,index;
	bool moved;

private:
	void RefreshIndexs();

};

class UndoEditEvents:public UndoEdit
{
public:
	UndoFunction *CreateUndoFunction();
};

class UndODeInitEvent
{
public:
	Seq_Event *seqevent;
	Seq_Track *totrack;
	MIDIPattern *topattern;
	int index;
};

class Undo_CutNote:public UndoFunction
{
public:
	Undo_CutNote(Note *n,OSTART pos)
	{
		id=Undo::UID_CUTNOTE;
		cutposition=pos;
		note=n;
		newnote=0;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_SPLITNOTE]);}

	Note *note,*newnote;
	OSTART cutposition;
};

class Undo_ConvertDrumsToNotes:public UndoFunction
{
public:
	Undo_ConvertDrumsToNotes(Seq_Event **e,int c)
	{
		id=Undo::UID_CONVERTDRUMSTONOTES;
		events=e;
		counter=c;
		notebuffer=0;
	}

	void FreeData();
	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_CONVERTDRUMSTONOTES]);}

	Seq_Event **events;
	Note **notebuffer;
	int counter;
};

class Undo_ConvertNotesToDrums:public UndoFunction
{
public:
	Undo_ConvertNotesToDrums(Seq_Event **e,Drumtrack **dt,int c)
	{
		id=Undo::UID_CONVERTNOTESTODRUMS;
		events=e;
		drumtracks=dt;
		counter=c;
		drumbuffer=0;
	}

	void FreeData();
	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_CONVERTNOTESTODRUMS]);}

	Seq_Event **events;
	Drumtrack **drumtracks;
	ICD_Drum **drumbuffer;
	int counter;
};

class Undo_EditEvents:public UndoFunction
{
public:
	Undo_EditEvents(UndoEditEvents *uee)
	{
		id=Undo::UID_EDITEVENTS;
		editevents=uee;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_EDITEVENTS]);}
	void FreeData();

	UndoEditEvents *editevents;
};

class Undo_DeleteEvent:public UndoFunction
{
public:
	Undo_DeleteEvent(Seq_Song *s,UndODeInitEvent *e,int c)
	{
		id=Undo::UID_DELETEEVENT;
		song=s;
		events=e;
		numberofevents=c;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_DELETEEVENTS]);}
	void RefreshGUI(bool undorefresh);
	void FreeData();

	UndODeInitEvent *events;
	int numberofevents;

private:
	void DeleteEvent(UndODeInitEvent *e);
};

class UndoQuantizeEvent
{
public:
	Seq_Event *seqevent;
	OSTART oldstart,newstart;
};

class Undo_QuantizeEvent:public UndoFunction
{
public:
	Undo_QuantizeEvent(Seq_SelectionList *l,int c,OSTART tx,UndoQuantizeEvent *ql)
	{
		id=Undo::UID_QUANTIZEEVENT;
		list=l;
		qcounter=c;
		ticks=tx;
		qnl=ql;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_QUANTIZEEVENTS_S]);}

	void FreeData()
	{
		if(qnl)delete qnl;
	}

	UndoQuantizeEvent *qnl;
	Seq_SelectionList *list;
	OSTART ticks;
	int qcounter;

private:
	void RefreshIndexs();

};

class Undo_SetNoteLength:public UndoFunction
{
public:
	Undo_SetNoteLength(Seq_SelectionList *l,int c,OSTART tx,UndoSetNoteLength *nl)
	{
		id=Undo::UID_SETNOTELENGTH;
		list=l;
		notecounter=c;
		ticks=tx;
		usnl=nl;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_SETLENGTHOFNOTES]);}

	void FreeData()
	{
		if(usnl)delete usnl;
	}

	UndoSetNoteLength *usnl;
	Seq_SelectionList *list;
	OSTART ticks;
	int notecounter;
};

class Undo_CreateEvent:public UndoFunction
{
public:
	Undo_CreateEvent(Seq_Song *,Seq_Event *,EF_CreateEvent *);
	Undo_CreateEvent(Seq_Song *,MIDIPattern *,OList *,OSTART);
	void AddedToUndo(){CreateUndoString(Cxs[CXS_CREATEEVENTS]);}
	void Do();
	void DoUndo();
	void FreeData();

	Seq_Event **neweventlist;
	int neweventcount;
	OList *eventlist;
	Seq_Event *seqevent;
	MIDIPattern *topattern;
	OSTART startposition,endposition;
	bool playit;
};

class Undo_EditEvent:public UndoFunction
{
public:
	Undo_EditEvent(Seq_Event *e,EF_CreateEvent *cr);
	void Do();
	void DoUndo();
	void DoRedo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_EDITEVENTS]);}
	void FreeData();

	Seq_Event *seqevent,*oldevent,*newevent;
	UBYTE oldstatus,oldbytes[3],newstatus,newbytes[3];
};
#endif