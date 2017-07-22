#ifndef CAMX_CREATETRACKS_H
#define CAMX_CREATETRACKS_H 1

#include "guiwindow.h"
#include "object_track.h"

class AudioPort;

class Edit_CreateTracks:public guiWindow
{
public:
	Edit_CreateTracks();

	void FreeMemory();
	void Gadget(guiGadget *);
	void Init();
	void RefreshRealtime_Slow();
	void ShowMIDI();
	void ShowAudio();
	void ShowRecordType();

private:
	Seq_Track clonetrack;

	guiGadget_Number *g_int;
	guiGadget *g_trackrecord,*focustrack,*g_create,*g_child,*g_clone,*audioio,*audioin,*audioout,*MIDIin,*MIDIout;
	int trackstocreate;
};

class Edit_CreateBus:public Edit_CreateTracks
{
public:
	Edit_CreateBus();
};

class Edit_CreateAutomationTrack:public guiWindow
{
public:
	Edit_CreateAutomationTrack(Seq_Track *,AudioChannel *);

	void FreeMemory();
	void Gadget(guiGadget *);
	void Init();
	void ShowInfo();
	void ShowPlugInInfo();
	void ShowPluginControlInfo();
	void ShowSysControlInfo();
	void ShowMIDISysInfo();

	void LearnFromMIDIEvent(LMIDIEvents *);
	void LearnFromPluginChange(InsertAudioEffect *,AudioObject *,OSTART time,int index,double value);
	void RemoveAudioEffect(InsertAudioEffect *);

	void UserSelect(int type,int ctrl);
	void UserSelectSysControl(int type);
	void UserSelectPluginControl(InsertAudioEffect *iae,int type);
	void UserSelectMIDIControl(int type);

	bool CheckIfObjectInside(Object *o);

	void CreatePluginCtrl();
	void CreateSysAutomation();
	void CreatePluginAutomation();
	void CreateMIDIAutomation();
	void CreateMIDISysAutomation();
	
	guiGadget_Number *MIDIchannel,*MIDInote;
	guiGadget *info,*MIDInote_i,*MIDIcreateautomationtrack,*plugininfo,*plugincreateautomationtrack,*syscontrolinfo,*plugincontrolinfo,
		*syscreateautomationtrack,*plugincontrolcreateautomationtrack,*glearn,*glearnandcreate,*MIDIsyscreateautomationtrack,
		*MIDIcontrolinfo;

	Seq_Track *track;
	AudioChannel *audiochannel;
	AutomationTrack *prevautomationtrack;

	// MIDI
	UBYTE data[4];
	UBYTE channel;
	UBYTE key;

	// PLUGIN
	InsertAudioEffect *learnpluginaudioeffect,*plugincontroleffect;
	AudioObject *learnpluginaudioobject;
	OSTART learnplugintime;
	int learnpluginindex,systype,plugincontroltype,MIDIcontroltype;
	double learnpluginvalue;

	bool MIDIdataset,plugindataset,syscontrolset,MIDIsyscontrolset,pluginctrlset,learnandcreate;
};
#endif

