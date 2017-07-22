#ifndef CAMX_WIN32AUDIOEDITOR_H
#define CAMX_WIN32AUDIOEDITOR_H 1

#ifdef OLDIE
#include "editor.h"

class Edit_Settings;
class AudioDevice;

class Edit_Win32Audio:public guiWindow
{
public:
	Edit_Win32Audio(Edit_Settings *,AudioDevice *);

	void ResetGadgets()
	{
		slider=0;
		ok=string=intg=samples=latency=0;
	}

	void Gadget(guiGadget *);
	void KeyDown();
	void FreeMemory();

	void ShowGadgets();
	void ShowSlider();
	void Init();
	void RefreshRealtime();

	Edit_Settings *editor;
	AudioDevice *audiodevice;

private:
	int setSize,srate;
	guiGadget_Slider *slider;
	guiGadget *ok,*string,*intg,*samples,*latency;
};
#endif

#endif