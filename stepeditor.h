#ifndef CAMX_STEPEDITOR_H
#define CAMX_STEPEDITOR_H 1

#include "editor.h"

#define RECORDEDITORLINES 21
#define SYNCEDITORLINES 20

class Edit_RecordingSettings:public guiWindow
{
public:
	Edit_RecordingSettings();

	void Gadget(guiGadget *);
	void KeyDown();
	void ShowGadgets();
	void RefreshRealtime();

	guiMenu *CreateMenu();
	void Init();

private:

	bool status_metrorecord,status_metroplayback,status_onoff,sl_metro;
	int status_punch,status_stepstep,status_steplength;

	guiGadget_Number *premetroint;
	guiGadget *tempochange,*preccountertype,*MIDIrecording,*audiorecording,*recordtofirstpattern,
	 *premetro,*premetronote,*metro,*metroplayback,*cyclecreatenewtrack,
	 *cyclecreatenewchild,*steponoff,*stepstep,*steplength,*stepleft,
	 *stepright,*songpunchin,*songpunchout,*triggerrecordevent,*triggerrecordnote,*metroonoff,*setrecord;
};

class Edit_SyncEditor:public guiWindow
{
public:
	enum
	{
		GADGET_RECEIVEMIDISTARTSTOP,
		GADGET_RECEIVEMIDISTARTPLAYBACK,
		GADGET_RECEIVEMIDISTARTRECORD,
		GADGET_RECEIVEMIDISTARTRECORDNOPRE,
		GADGET_RECEIVEMIDISTOP,
		GADGET_WAITFORMIDIPLAYBACK
	};

	Edit_SyncEditor();
	void Gadget(guiGadget *);
	void RefreshRealtime_Slow();
	guiMenu *CreateMenu();
	void Init();

private:
	void ShowReceiveMIDIStartStop(bool force);
	void ShowReceiveMIDIStartPlayback(bool force);
	void ShowReceiveMIDIStartRecord(bool force);
	void ShowReceiveMIDIStartRecordNoPre(bool force);
	void ShowReceiveMIDIStop(bool force);
	void ShowWaitForMIDIPlayback(bool force);
	
	int status_receiveMIDIstartstop,status_receiveMIDIstartplayback,status_receiveMIDIstartrecord,status_receiveMIDIstartrecordnopre;
	bool status_receiveMIDIstop,status_waitforMIDIplayback;
	guiGadget *receiveMIDIstartstop,
		*receiveMIDIstartplayback,
		*receiveMIDIstartrecord,
		*receiveMIDIstartrecordnopre,
		*receiveMIDIstop,
		*waitforMIDIplayback;
};
#endif