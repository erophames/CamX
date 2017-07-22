#include "defines.h"

#include "songmain.h"
#include "audiofile.h"
#include "initplayback.h"
#include "object_song.h"
#include "object_track.h"
#include "MIDIPattern.h"
#include "gui.h"

void Seq_Track::RefreshTrackAfterRecording()
{
	// MIDI
	if(record_MIDIPattern)
	{
		record_MIDIPattern->ConvertOpenRecordingNotesToNotes(song->GetSongPosition());
		record_MIDIPattern->DeleteOpenNotes();

		AddEffectsToPattern(record_MIDIPattern); // Track FX
	}
}


