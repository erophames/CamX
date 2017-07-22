#include "object_song.h"
#include "MIDIhardware.h"
#include "MIDIoutproc.h"
#include "gui.h"
#include "MIDIindevice.h"
#include "semapores.h"

void Seq_Song::MIDIInputDeviceToSongPosition(int flag)
{
	MIDIInputDevice *i=mainMIDI->FirstMIDIInputDevice();

	while(i)
	{
		if(i->newsongpositionset==true){

			i->newsongpositionset=false; // reset
			SetSongPosition(i->indesongposition,true,flag);
			
		}

		i=i->NextInputDevice();
	}
}

void Seq_Song::RefreshExternSongTempoChanges()
{
	bool loopsrepaired=false;

	mainthreadcontrol->LockActiveSong();

	loopsrepaired=RepairLoops(Seq_Song::RLF_NOLOCK|Seq_Song::RLF_CHECKREFRESH);

	mainthreadcontrol->UnlockActiveSong();

	if(loopsrepaired==true)
		maingui->RefreshRepairedLoopsGUI(this);

	maingui->RefreshTempoGUI(this);
}