#include "songmain.h"
#include "object_song.h"
#include "object_track.h"
#include "MIDIhardware.h"
#include "MIDIPattern.h"
#include "settings.h"
#include "semapores.h"


MIDIPattern *Seq_Track::GetMIDIRecordPattern(OSTART time) // Called by MIDI In Proc
{	
	if(record_MIDIPattern)return record_MIDIPattern;

	// Create new Pattern

	if(mainsettings->recordtofirstselectedMIDIPattern==true)
	{
		Seq_Pattern *c=FirstPattern(MEDIATYPE_MIDI);

		while(c)
		{
			if(c->flag&OFLAG_SELECTED)
			{
				c->recordeventsadded=true;

				LockRecord();
				record_MIDIPattern=(MIDIPattern *)c;
				record_MIDIPattern->recordcounter=song->recordcounter;
				record_MIDIPattern->recordpattern=true;
				UnlockRecord();

				return record_MIDIPattern;
			}

			c=c->NextPattern(MEDIATYPE_MIDI);
		}
	}

	if(record_MIDIPattern=new MIDIPattern)
	{	
		record_MIDIPattern->recordcounter=song->recordcounter;
		record_MIDIPattern->recordpattern=true; // This is a record pattern, no select etc..

		LockRecord();
		AddSortPattern(record_MIDIPattern,time);
		UnlockRecord();

		record_MIDIPattern->SetName("record");
	}
	
	return record_MIDIPattern;
}

void mainMIDIRecord::AddMIDIRecordEvent(NewEventData *data)
{
	if(!data)return;

	Lock();
	newevents.AddEndO(data);
	UnLock();
}
