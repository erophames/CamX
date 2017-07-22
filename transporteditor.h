#ifndef CAMX_TRANSPORTEDITOR_H
#define CAMX_TRANSPORTEDITOR_H 1

#include "editor.h"
#include "settings.h"
#include "lastMIDIevent.h"
#include "cshowpeak.h"

class Seq_Project;
class Seq_Tempo;
class AudioDevice;

class Edit_Transport:public guiWindow
{
public:
	Edit_Transport();

	void ShowMasterAB(bool realtime);
	bool SaveAble(){return true;}
	void ShowProgress(char *string);

	void Init();
	void DeInitWindow();
	void RefreshRealtime();
	void RefreshRealtime_Slow();

	void RefreshSMPTE();
	void RefreshMeasure();
	
	void Gadget(guiGadget *);

	void ShowTempo(bool realtime);
	void ShowSignature(bool realtime);
	void ShowTime(bool realtime);
	void ShowCycle(bool realtime);

	void ShowTransportMenu();
	void ShowSongZoom();

	void ShowLatency(bool force);
	void ShowSampleRate(bool force);

	void ShowCPU(bool force=false);
	void ShowMasterPeak(bool force=false);
	void ShowMasterVolume(bool force=false);

	void MouseClickInVolume(bool leftmouse);
	void MouseMoveInVolume();
	void MouseReleaseInVolume(bool leftmouse);

	void RefreshSong(Seq_Song *);

	int mmx;

	CShowPeak peak;
	guiGadget_CW *cpudb,*masterpeak,*mastervolume;

private:
	void InitDevices();

	void ShowSongLength();
	void ShowLastMIDIIO(Seq_Song *);
	void ShowStatus();
	void InitGadgets();
	void OpenMixer();
	guiWindow *OpenEditor(int id=-1);

	LMIDIEvents lastout,lastin;

	guiGadget_Numerator *signgadget;
	guiGadget_Number *songlengthgadget,*tempogadget;
	guiGadget_Time *timegadget_measure,*timegadget_smpte,*cycle_left,*cycle_right;
	guiGadget_Volume *mastervolume_db;

	guiGadget *gl_start,*gl_stop,*gl_record,
		*songzoomgadget,*MIDIininfo,*MIDIoutinfo,*progress,
		*gl_dock,*gl_autoplayback,*gl_autorecording,
		*gl_solo,*gl_cycle,*gl_metro,*audiousage,/**syncgadget,*/
		*g_samplerate,*g_latency,
		*g_mix,*g_edit,
		*masterfx;

	int lastmonitor_inputevent,lastmonitor_outevent,smpteflag,song_status,song_punchrecording,status_sync;
	
	// Display Number, Text etc...
	double showtempo,latency;
	int sign_nn,sign_dn;
	ARES maxpeaksum,dbvalue;
	OSTART sl_measure;

	int showwhat,sl_solo,cycleindex,samplerate,setSize,switchusagecounter,trans_editorcount,trans_editindex,trans_mixercount,trans_mixerindex;
	double lastfilems;
	LONGLONG lastdevicesmicrosec;
	bool sl_metro,impulseset,initdevices,showaudiomaster,showaudioclear,sl_cycle,sl_automationplayback,sl_automationrecording,masterabstatus;
	char lastMIDIoutstring[255],lastMIDIinstring[255],*progressstring;
};

#endif