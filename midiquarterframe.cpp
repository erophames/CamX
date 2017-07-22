#include "object_song.h"
#include "object_project.h"
#include "MIDIquarterframe.h"
#include "settings.h"

void MTC_Quarterframe::Reset()
	{
		rotation=SENDNEXTMTCQF_FRAME;
		hour=min=sek=frame=0;
		set=false;
		ticks=-1;
		inputflag=0;
	}

void MTC_Quarterframe::MTC_AddQuarterFrame(Seq_Song *song)
{
	/*
	The most important message is the Quarter Frame message (which is not a SysEx message). 
	It has a status of 0xF1, and one subsequent data byte. This message is sent periodically to keep track of the running SMPTE time. 
	It is similar to the MIDI Clock message. The Quarter Frame messages are sent at a rate of 4 per each SMPTE Frame. 
	In other words, by the time that a slave has received 4 Quarter Frame messages, a SMPTE Frame has passed. 
	So, the Quarter Frame messages provide a "sub-frame" clock reference. (With 30 fps SMPTE, this "clock tick" happens every 8.3 milliseconds).
*/

	switch(song->project->standardsmpte)
	{
	case Seq_Pos::POSMODE_SMPTE_24:
		{
			Seq_Pos pos24(Seq_Pos::POSMODE_SMPTE_24);

			pos24.pos[0]=hour;
			pos24.pos[1]=min;
			pos24.pos[2]=sek;
			pos24.pos[3]=frame;
			pos24.pos[4]=qframe;

			pos24.AddQuarterFrame(1);

			hour=pos24.pos[0];
			min=pos24.pos[1];
			sek=pos24.pos[2];
			frame=pos24.pos[3];
			qframe=pos24.pos[4];
		}
		break;

	case Seq_Pos::POSMODE_SMPTE_25:
		{
			Seq_Pos pos25(Seq_Pos::POSMODE_SMPTE_25);

			pos25.pos[0]=hour;
			pos25.pos[1]=min;
			pos25.pos[2]=sek;
			pos25.pos[3]=frame;
			pos25.pos[4]=qframe;

			pos25.AddQuarterFrame(1);

			hour=pos25.pos[0];
			min=pos25.pos[1];
			sek=pos25.pos[2];
			frame=pos25.pos[3];
			qframe=pos25.pos[4];
		}
		break;

	case Seq_Pos::POSMODE_SMPTE_2997:
		{
			Seq_Pos pos297(Seq_Pos::POSMODE_SMPTE_2997);

			pos297.pos[0]=hour;
			pos297.pos[1]=min;
			pos297.pos[2]=sek;
			pos297.pos[3]=frame;
			pos297.pos[4]=qframe;

			pos297.AddQuarterFrame(1);

			hour=pos297.pos[0];
			min=pos297.pos[1];
			sek=pos297.pos[2];
			frame=pos297.pos[3];
			qframe=pos297.pos[4];
		}
		break;

	case Seq_Pos::POSMODE_SMPTE_30:
		{
			Seq_Pos pos30(Seq_Pos::POSMODE_SMPTE_30);

			pos30.pos[0]=hour;
			pos30.pos[1]=min;
			pos30.pos[2]=sek;
			pos30.pos[3]=frame;
			pos30.pos[4]=qframe;

			pos30.AddQuarterFrame(1);

			hour=pos30.pos[0];
			min=pos30.pos[1];
			sek=pos30.pos[2];
			frame=pos30.pos[3];
			qframe=pos30.pos[4];
		}
		break;
	}
}
