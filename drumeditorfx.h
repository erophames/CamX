#ifndef CAMX_DRUMEDITORFX_H
#define CAMX_DRUMEDITORFX_H 1

#include "editor.h"
#include "drumeditor.h"

class Edit_DrumFX:public guiWindow
{
public:
	Edit_DrumFX(Edit_Drum *);

	void RefreshRealtime_Slow();
	void InitGadgets();
	void Init();
	void RefreshMIDI(Seq_Track *);
	void Gadget(guiGadget *);
	void ShowTrackNumber(bool force);
	void ShowTrackName(bool force);
	void ShowTrackChannel(bool force);
	void ShowTrackVelocity(bool force);
	void ShowTrackKey(bool force);
	void ShowTrackLength(bool force);

	Edit_Drum *editor;
	guiGadget *g_track,*g_trackstring,*trackMIDIoutput,*tracklength;
	guiGadget_Number *trackchannel,*trackvelocity,*trackkey;
	LONGLONG track_length;
	int track_volume,tracknr,recordtracktype,MIDItype,notetype;
	UBYTE track_channel;
	char track_key;
	bool showMIDI,showMIDIfilter,showaudio,showquant,showpattern,showMIDI_pattern,showaudio_pattern,
		MIDItypesetauto;
};
#endif
