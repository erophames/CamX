/*
#include "subtrack.h"

class MidiObject;

class MidiObjectTrack:public Seq_SubTrack
{
	friend Seq_Track;

public:
	MidiObjectTrack()
	{
		objectid=OBJECT_MIDISUBTRACK;

		max=127;
		min=0;
		startvalue=64;
	}

	UDWORD GetAccessCounter()
	{
		return objects.accesscounter;
	}

	MidiObjectTrack *PrevMidiTrack(){return (MidiObjectTrack *)prev;}
	MidiObjectTrack *NextMidiTrack() {return (MidiObjectTrack *)next;}

	MidiObject *FindAudioObjectBefore(long pos)
	{
		return (AudioObject *)objects.FindObjectBefore(pos);
	}
	
	MidiObject *FindAudioObject(long pos)
	{
		return (MidiObject *)objects.FindObject(pos);
	}
	
	void AddMidiObject(MidiObject *mo,long position)
	{
		mo->track=this;
		
		objects.AddObjectSort(mo,position);
	}
	
	void SaveSubTrackData(camxFile *file)
	{
		file->Save_Chunk(max);
		file->Save_Chunk(min);
		file->Save_Chunk(startvalue);
	}

	ARES max;
	ARES min;
	ARES startvalue;

	UBYTE status;
	UBYTE byte1;
	UBYTE byte2;
	UBYTE byte3;
};

#endif

  */