#ifdef OLDIE

#include "defines.h"
#include "audioauto_volume.h"
#include "audiofile.h"
#include "MIDIoutproc.h"
#include "object_track.h"
#include "object_song.h"
#include "gui.h"
#include "semapores.h"
#include "editortypes.h"
#include "audiopattern.h"
#include "songmain.h"

/*
ARES AudioPatternVolumeCurve::GetVolumeAt(int pos)
{
	if(AutomationObject *ao=GetVolumeObjectAt(pos))
	{
		if(AutomationObject *ao2=ao->NextAutomationObject())
		{
			ARES vdiff=ao2->GetValue()-ao->GetValue();

			if(vdiff)
			{
				int tdiff=ao2->GetParameterStart()-ao->GetParameterStart();

				if(tdiff) // Pos Between |---...----|
				{
					ARES h=(ARES)(pos-ao->GetParameterStart());

					h/=(ARES)tdiff;

					h*=vdiff;
					h+=ao->GetValue();

					return h;	
				}
			}
		}

		return ao->GetValue();
	}

	return 1;
}
*/

// Audio Volume Curve
ARES AudioPatternVolumeCurve::AddVolumeToBuffer(AudioHardwareBuffer *buffer,OSTART position,int channels)
{
	return GetVolumeAt(position);

	/*
	if(vol!=1)
	{
		ARES *s=buffer->outputbufferARES;
		int i=channels*buffer->samplesinbuffer;

		while(i--)
			*s++ *=vol;
	}
	*/
}

ARES AudioPatternVolumeCurve::GetVolumeAt(OSTART pos)
	{
#ifdef OLDIE
		if(AutomationParameter *ao=GetVolumeObjectAt(pos))
		{
			if(AutomationParameter *ao2=ao->NextAutomationParameter())
			{
				ARES vdiff=ao2->GetValue()-ao->GetValue();

				if(vdiff)
				{
					OSTART tdiff=ao2->GetParameterStart()-ao->GetParameterStart();

					if(tdiff) // Pos Between |---...----|
					{
						ARES h=(ARES)(pos-ao->GetParameterStart());

						h/=(ARES)tdiff;
						h*=vdiff;
						h+=ao->GetValue();

						return h;	
					}
				}
			}

			return ao->GetValue();
		}
#endif

		return 1;
	}

AutomationParameter *AudioPatternVolumeCurve::GetVolumeObjectAt(OSTART pos)
{
#ifdef OLDIE
	if(automationtrack && automationtrack->FirstAutomationParameter())
	{
		if(AutomationParameter *o=automationtrack->FindAutomationParameterBefore(pos))return o;
		return automationtrack->FirstAutomationParameter();
	}
#endif

	return 0;
}

void AudioPatternVolumeCurve::DeleteVolumeCurve()
{
#ifdef OLDIE
	if(automationtrack)
	{
		Seq_Song *song=pattern->GetTrack()->song;

		if(song==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		DeleteAllVolumeObjects();

		automationtrack->track->DeleteAutomationTrack(automationtrack,true);
		automationtrack=0;

		song->MixAllAutomationObjects(INITPLAY_MIDITRIGGER,true,song->GetSongPosition()); // ++ reset

		if(song==mainvar->GetActiveSong())
		{
			mainMIDIalarmthread->SetSignal(); // Send Refresh Alarm
			mainthreadcontrol->UnlockActiveSong();
		}

		// Refresh GUI
		maingui->RefreshAllEditors(pattern->GetTrack()->song,EDITORTYPE_ARRANGE,0);
		maingui->ClearRefresh();
	}
	// maingui->RefreshAllEditorsWithPattern(song,pattern);
#endif

}

void AudioPatternVolumeCurve::MoveCurve(OSTART diff)
{
	//if(automationtrack)
	//	automationtrack->MoveAll(diff);
}

#endif
