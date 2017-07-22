#ifndef CAMX_UNDOFUNCTIONS_TRACK_H
#define CAMX_UNDOFUNCTIONS_TRACK_H 1

#include "defines.h"
#include "undo.h"
#include "languagefiles.h"

class Undo_SplitMIDITrack:public UndoFunction
{
public:
	Undo_SplitMIDITrack(Seq_Track *t,MIDIPattern *mp)
	{
		id=mp?Undo::UID_SPLITPATTERN:Undo::UID_SPLITTRACK;
		track=t;
		mpattern=mp; // or NULL=Full Track
		split=false;
		for(int i=0;i<16;i++)newcreatedtrack[i]=0;
	}

	void Do();
	void UndoGUI();
	void DoUndo();
	void DoRedo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_SPLITTRACKCHANNELS]);}

	MIDIPattern *mpattern;
	Seq_Track *newcreatedtrack[16], // MIDIChannel 1-16
		*track;
	bool split;
};

class TrackTypeList
{
public:
	Seq_Track *track;
	int type; //1==Audio,2==MIDI,3==Empty
};

class Undo_SortAllTracks:public UndoFunction
{
public:
	Undo_SortAllTracks(Seq_Song *s,TrackTypeList *l,int c)
	{
		id=Undo::UID_SORTTRACKS;
		song=s;
		oldsorttracks=0;
		list=l;
		count=c;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_SORTTRACKSBYTYPE]);}
	void FreeData();

	Seq_Track **oldsorttracks;
	TrackTypeList *list;
	int count;
};

class Undo_DeleteAllEmptyTracks:public UndoFunction
{
public:
	Undo_DeleteAllEmptyTracks(Seq_Song *s);
	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_DELETEALLEMPTYTRACKS]);}
	void FreeData();
	void RefreshGUI(bool undorefresh);
	void RefreshDo(); // Delete Plugins Memory etc
	void RefreshPreUndo(); // Delete Plugins Memory etc

	OOList list;
	Seq_Track **tracks;
	int trackscounter;
};

class Undo_DeleteTracks:public UndoFunction
{
public:
	Undo_DeleteTracks(Seq_Song *,Seq_Track *);
	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[singletrack?CXS_DELETETRACK:CXS_DELETEALLSELTRACKS]);}
	void RefreshGUI(bool undorefresh);
	void FreeData();
	void RefreshDo(); // Delete Plugins Memory etc
	void RefreshPreUndo(); // Delete Plugins Memory etc

	OOList list;
	Seq_Track **tracks,*singletrack;
	int trackscounter;
};

class Undo_MoveTrack:public UndoFunction
{
public:
	Undo_MoveTrack(Seq_Song *s,int d)
	{
		id=Undo::UID_MOVETRACK;
		song=s;
		diff=d;
		song->tracks.CreateList(&list); // Buffer old
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_MOVETRACKS]);}
	void FreeData()
	{
		list.FreeMemory();
	}

	OOList list;
	int diff;
};

class Undo_AddTracksAsChild:public UndoFunction
{
public:
	Undo_AddTracksAsChild(Seq_Song *s,Seq_Track *t)
	{
		id=Undo::UID_ADDASCHILDTRACK;
		totrack=t;
		song=s;
		s->tracks.CreateList(&list); // Buffer old
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_ADDOTHERTRACKASCHILD]);}
	void FreeData()
	{
		list.FreeMemory();
	}

	OOList list;
	Seq_Track *totrack;
};

class Undo_RemoveTracksFromChild:public UndoFunction
{
public:
	Undo_RemoveTracksFromChild(Seq_Song *song)
	{
		id=Undo::UID_REMOVETRACKSFROMPARENT;
		song->tracks.CreateList(&list); // Buffer old
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_RELEASECHILDTRACKS]);}
	void FreeData()
	{
		list.FreeMemory();
	}

	OOList list;
};

class undocreatebussends
{
public:
	Seq_Track *track;
	AudioSend *send;
	int index;
};

class undotrackdirectbus
{
public:
	Seq_Track *track;
	AudioChannel *bus;
	int index;
	bool usedirecttodevice;
};

class Undo_CreateBus:public UndoFunction
{
public:
	Undo_CreateBus(Seq_Song *,AudioChannel **cl,int cnr,AudioPort *op,AudioChannel *pc)
	{
		id=Undo::UID_CREATEBUS;
		channels=cl;
		outport=op;
		prevchannel=pc;
		number=cnr;
		created=false;
		ucb=0;
		pdc=0;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_CREATEBUS]);}
	void FreeData();
	void RefreshPostUndo(); //  Plugins Memory etc
	void RefreshPreRedo(); // Plugins Memory etc

	undocreatebussends *ucb;
	undotrackdirectbus *pdc;
	AudioPort *outport;
	AudioChannel **channels,*prevchannel;
	int number,ucbnr,pdcnr;
	bool created;
};

class Undo_DeleteBus:public UndoFunction
{
public:
	Undo_DeleteBus(Seq_Song *,AudioChannel **cl,int cnr,int *i)
	{
		id=Undo::UID_DELETEBUS;
		channels=cl;
		number=cnr;
		ucb=0;
		pdc=0;
		index=i;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_DELETEBUS]);}
	void FreeData();
	void RefreshPostUndo(); //  Plugins Memory etc
	void RefreshPreRedo(); // Plugins Memory etc
	void RefreshGUI(bool undorefresh);

	undocreatebussends *ucb;
	undotrackdirectbus *pdc;
	AudioChannel **channels;
	int number,ucbnr,pdcnr,*index;
};

class Undo_CreateTracks:public UndoFunction
{
public:
	Undo_CreateTracks(Seq_Song *,Seq_Track **tl,int tracknumber,int ix,Seq_Track *parent,Seq_Track *cl,int flag);
	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[number==1?CXS_CREATETRACK:CXS_CREATETRACKS]);}
	void FreeData();
	void RefreshPostUndo(); //  Plugins Memory etc
	void RefreshPreRedo(); // Plugins Memory etc

	Seq_Track *parent,*clonetrack,**tracks;
	int number,index;
	bool activate;
};

class Undo_CreateParent:public UndoFunction
{
public:
	Undo_CreateParent(Seq_Song *s,Seq_Track *nt,Seq_Track *clone);
	void Do();
	void DoUndo();
	void FreeData();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_CREATEPARENTFORSELECTEDTRACKS]);}

	OOList sellist,list;
	Seq_Track *nexttrack,*newtrack,*clonetrack;
	bool done;
};

class Undo_QuantizeTrack:public UndoFunction
{
public:
	Undo_QuantizeTrack(Seq_Track *t,QuantizeEffect *backup,QuantizeEffect *newq);
	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_QUANTIZETRACK]);}

	QuantizeEffect backupeffects, // oldbuffer
		neweffects; // oldbuffer

	Seq_Track *track;
	OSTART changedpositions;
	int mediatypes;
};
#endif
