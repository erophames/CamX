#ifndef CAMX_EDITMIDIFILTER
#define CAMX_EDITMIDIFILTER 1

#include "editor.h"
#include "guiwindow.h"
#include "MIDIfilter.h"

class Edit_MIDIFilter:public guiWindow
{
public:
	Edit_MIDIFilter(MIDIFilter *);

	EditData *EditDataMessage(EditData *);
	void Gadget(guiGadget *);
	void CreateWindowMenu();

	void DeInitWindow()
	{	
		if(infostring)delete infostring;
		//delete this;
	}

	void LoadSettings();
	void SaveSettings();
	void SetInfo(char *info);
	void ShowFilter();
	void Init();
	void RefreshRealtime();

	MIDIFilter c_filter,*filter;
	char *infostring;
	int notetype;

private:
	void ShowOctaves();
	void RefreshEvents(bool force);

	MIDIFilter backup;
	guiGadgetList *gadgetlist;
	guiGadget *boxnote,*boxprogchg,*boxcontrol,*boxpitchbend,*boxsysex,*boxchannelpressure,*filterbybpass,
	 *boxpolypressure,*boxintern,*info,*boxchannel[16],*boxoctave[11];
};

#endif