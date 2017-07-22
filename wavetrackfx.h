#ifndef CAMX_WAVETRACKFX_H
#define CAMX_WAVETRACKFX_H 1

#include "defines.h"

class Edit_Wave;
class WaveTrack;

// MIDI FX Editor
class Edit_WaveEditorEffects
{
	friend class Edit_Wave;

public:	
	Edit_WaveEditorEffects()
	{		
		ResetGadgets();
	};
	
	// 	guiMenu *CreateMenu();
	
	void FreeMemory()
	{
		ResetGadgets();
	}

	void InitGadgets();
	bool Init();
	
	void ShowActiveWaveTrack();

	void RefreshRealtime();
	
	void ResetGadgets()
	{
		trackname=
			trackstatus=
			trackchannel=
			trackdata1=
			trackdata2=
			0;
	}
	
	void Close();
	guiGadget *Gadget(guiGadget *);

	Edit_Wave *waveeditor;
	guiGadget *trackname;

private:	
	// MIDI
	guiGadget *trackstatus;
	guiGadget *trackchannel;
	guiGadget *trackdata1;
	guiGadget *trackdata2;
	
	int status_channel;

	void AddKeysToMenu(guiMenu *m,WaveTrack *track);
	void AddCtrlToMenu(guiMenu *m,WaveTrack *track);
};

#endif