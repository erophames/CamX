#ifndef CAMX_MEMPOOLS_H
#define CAMX_MEMPOOLS_H 1

#include "object.h"

#ifdef WIN32
#include <AFXMT.h> // Semaphores
#endif

#include "objectevent.h"
#include "patternselection.h"
#include "seq_realtime.h"

#ifdef MEMPOOLS

class mainPools;

class mempool_Note:public Object
{
public:
	mempool_Note(mainPools *);
	mempool_Note *NextPool(){return (mempool_Note *)next;}

	Note notes[EVENTMEMPOOLSIZE];
	mainPools *mainpool;
	int notesused[EVENTMEMPOOLSIZE/32];
	bool full;
};

class mempool_Control:public Object
{
public:
	mempool_Control(mainPools *);
	mempool_Control *NextPool(){return (mempool_Control *)next;}

	ControlChange controls[EVENTMEMPOOLSIZE];
	mainPools *mainpool;
	int controlused[EVENTMEMPOOLSIZE/32];
	bool full;
};

class mempool_Pitchbend:public Object
{
public:
	mempool_Pitchbend(mainPools *);
	mempool_Pitchbend *NextPool(){return (mempool_Pitchbend *)next;}

	Pitchbend pitch[EVENTMEMPOOLSIZE];
	mainPools *mainpool;
	int pitchused[EVENTMEMPOOLSIZE/32];
	bool full;
};

class mempool_SelectionEvent:public Object
{
public:
	mempool_SelectionEvent(mainPools *);
	mempool_SelectionEvent *NextPool(){return (mempool_SelectionEvent *)next;}

	Seq_SelectionEvent sevents[EVENTMEMPOOLSIZE];
	mainPools *mainpool;
	int used[EVENTMEMPOOLSIZE/32];
	bool full;
};

class mempool_NoteOffRealtime:public Object
{
public:
	mempool_NoteOffRealtime(mainPools *);
	mempool_NoteOffRealtime *NextPool(){return (mempool_NoteOffRealtime *)next;}

	NoteOff_Realtime offs[EVENTREALTIMEMEMPOOLSIZE];

	mainPools *mainpool;
	int used[EVENTREALTIMEMEMPOOLSIZE/32];
	bool full;
};

#endif


#ifndef MEMPOOLS

#define NEWNOTE new Note
#define DELETENOTE delete
//#define mainpools->mempGetNote() new Note
//#define mainpools->mempDeleteNote( delete(

#endif

#ifdef MEMPOOLS
class mainPools
{
	friend class Seq_Song;
	friend class Note;
	friend class NoteOff_Realtime;

public:
	mainPools();

	Note *mempGetNote();
	void mempDeleteNote(Note *);

	mempool_Note* FirstNotePool() { return (mempool_Note *)notepool.GetRoot(); }

	ControlChange *mempGetControl();
	void mempDeleteControl(ControlChange *);

	mempool_Control* FirstControlPool() { return (mempool_Control *)controlpool.GetRoot(); }

	Pitchbend *mempGetPitchbend();
	void mempDeletePitchbend(Pitchbend *);

	mempool_Pitchbend* FirstPitchbendPool() { return (mempool_Pitchbend *)pitchbendpool.GetRoot(); }

	Seq_SelectionEvent *mempGetSelEvent();
	void mempDeleteSelEvent(Seq_SelectionEvent *);

	mempool_SelectionEvent *FirstSelectionPool(){return (mempool_SelectionEvent *)selectionpool.GetRoot(); }

	NoteOff_Realtime *mempGetNoteOff_Realtime();
	void mempDeleteNoteOff_Realtime(NoteOff_Realtime *);

	mempool_NoteOffRealtime *FirstNoteOffRealtimePool(){return (mempool_NoteOffRealtime *)noteoffrealtimepool.GetRoot(); }

	void CloseAllMemoryPools();

private:

	// Locks

#ifdef WIN32
	CCriticalSection lock_notes,lock_control,lock_pitch,lock_selection,lock_noteoffreal;
#endif

	OList notepool,controlpool,pitchbendpool,selectionpool,noteoffrealtimepool;
	mempool_Note *firstfreenotepool;
	mempool_Control *firstfreecontrolpool;
	mempool_Pitchbend *firstfreepitchbendpool;
	mempool_SelectionEvent *firstfreeselectionpool;
	mempool_NoteOffRealtime *firstfreenoteoffrealtimepool;

};
#endif

#endif