#ifndef CAMX_CPUUSAGE_H
#define CAMX_CPUUSAGE_H 1

#include "editor.h"

class Edit_CPU:public guiWindow
{
public:
	Edit_CPU();
	void Init();
	void RefreshRealtime_Slow();
	void Gadget(guiGadget *);

private:
	void ShowGFX();
	guiGadget *cpuusage,*cpuusagemax,*hdusage,*hdusagemax,*hdmem,*audioinput,*sync;
	bool outofsync;
};
#endif