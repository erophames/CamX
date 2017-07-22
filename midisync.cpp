#include "MIDIclock.h"
#include "object_song.h"

void MIDISync::BufferBeforeTempoChanges()
{
	buffer_nextMIDIclockposition=song->timetrack.ConvertSamplesToTicks(out_nextclocksample);
}

void MIDISync::RefreshTempoBuffer()
{
	out_nextclocksample=song->timetrack.ConvertSamplesToTicks(buffer_nextMIDIclockposition);
}