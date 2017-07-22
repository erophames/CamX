#ifdef OLDIE

#ifndef CAMX_DRUMTRACKFX_H
#define CAMX_DRUMTRACKFX_H 1

#include "defines.h"

class Edit_Drum;
class MIDIOutputDevice;
class AudioChannel;
class Drumtrack;

// MIDI FX Editor
class Edit_DrumEditorEffects
{
public:	
	Edit_DrumEditorEffects()
	{		
		ResetGadgets();
	};
	
	void FreeMemory()
	{
		ResetGadgets();
	}

	void InitGadgets();
	bool Init();
	
	void ShowActiveTrack();

	void RefreshRealtime();
	
	void ResetGadgets()
	{
		trackvolume=0;

		drummap=
		mute=
		trackvolumebutton=
		trackname=
			MIDIchannel=
			MIDIoutput=
			MIDIkey=
			drumlength=
			audiochannel=
			0;
	}
	
	void CreateNewSongDrumMap();

	void Close();
	void Gadget(guiGadget *);
	EditData *EditDataMessage(EditData *data);

	Edit_Drum *drumeditor;
	guiGadget *trackname;

private:
	void ShowTrackVolumeButton();

	guiGadget *drummap;
	// MIDI
	guiGadget *MIDIchannel; // MIDI-> 0=track channel
	guiGadget *audiochannel; // 0=track channel 

	guiGadget *MIDIoutput;
	guiGadget *MIDIkey;
	guiGadget *drumlength;
	guiGadget_Slider *trackvolume;
	guiGadget *trackvolumebutton;

	guiGadget *mute;

	int status_channel;

	// Realtime
	int rl_volumeslider;
	int rl_volumebutton;
};

#endif
#endif