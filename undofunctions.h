#ifndef CAMX_UNDOFUNCTIONS_H
#define CAMX_UNDOFUNCTIONS_H 1

#include "defines.h"
#include "undo.h"
#include "object_song.h"
#include "MIDIoutdevice.h"
#include "languagefiles.h"

class ICD_Drum;
class Drumtrack;

class Undo_PastePatternBuffer:public UndoFunction
{
public:
	Undo_PastePatternBuffer(int mtype)
	{
		id=Undo::UID_PASTEPATTERNBUFFER;
		mediatype=mtype;
	}

	void AddedToUndo(){CreateUndoString(Cxs[CXS_PASTEPATTERNBUFFER]);}

private:
	int mediatype;
};

class Undo_PasteTrack:public UndoFunction
{
public:
	Undo_PasteTrack(Seq_Track *tt)
	{
		id=Undo::UID_PASTETRACK;
		totrack=tt;
	}

	void AddedToUndo(){CreateUndoString(Cxs[CXS_PASTETRACK]);}

	Seq_Track *totrack;
};

class Undo_PastePattern:public UndoFunction
{
public:
	Undo_PastePattern(int mt)
	{
		id=Undo::UID_PASTEPATTERN;
		mediatype=mt;
	}

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_PASTEPATTERN]);}
	void UndoGUI();

	Seq_Pattern *newpattern;

private:
	int mediatype;
};

class UndoSetMIDIOutBuffer
{
public:
	Seq_Track *track;
	Seq_Group_MIDIOutputDevice olddevices;
	MIDIFilter oldfilter;
};

class Undo_SetMIDIOut:public UndoFunction
{
public:
	Undo_SetMIDIOut(Seq_Track *,bool sel);
	void Do();
	void DoUndo();
	void FreeData();

	void AddedToUndo();

	UndoSetMIDIOutBuffer *oldbuffer;

	Seq_Track *track;
	bool selected;
	int nrtracks;
	Seq_Group_MIDIOutputDevice devicegroup;
	MIDIFilter filter;
};

class Undo_SetAudioIO:public UndoFunction
{
public:
	Undo_SetAudioIO(Seq_Track *,bool sel);
	void Do();
	void DoUndo();
	void FreeData();
	void AddedToUndo();

	Seq_AudioIO channelgroup,*oldchannels;
	int nrtracks;
	bool selected,usedirecttodevice;
};

class UndoSetMIDIInBuffer
{
public:
	Seq_Track *track;
	Seq_Group_MIDIInputDevice olddevices;
	MIDIFilter oldfilter;

	// Buffer Effects
	bool oldnoMIDIinput,olduseallinputdevices,oldusealwaysthru,olduserouting;
};

class Undo_SetMIDIIn:public UndoFunction
{
public:
	Undo_SetMIDIIn(Seq_Track *,bool sel);

	void Do();
	void DoUndo();
	void FreeData();
	void AddedToUndo();

	UndoSetMIDIInBuffer *oldbuffer;

	MIDIFilter inputfilter;
	int nrtracks;
	bool selected,noMIDIinput,useallinputdevices,usealwaysthru,userouting;
	Seq_Group_MIDIInputDevice devicegroup;
	Seq_Track *track;
};

/*
class Undo_CreateFolder:public UndoFunction
{
public:
	Undo_CreateFolder(Folder *f,OSTART s)
	{
		id=Undo::UID_CREATEFOLDER;
		folder=f;
		startposition=s;

#ifdef _DEBUG
		n[0]='U';
		n[1]='C';
		n[2]='F';
		n[3]='O';
#endif
	}

#ifdef _DEBUG
	char n[4];
#endif

	void Do();
	void DoUndo();
	void DoRedo();

	void AddedToUndo(){CreateUndoString("Create Folder");}

	Folder *folder;
	OSTART startposition;
};
*/

#endif