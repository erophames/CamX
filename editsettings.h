#ifndef CAMX_EDITORSETTINGS_H
#define CAMX_EDITORSETTINGS_H 1

#include "defines.h"
#include "gui.h"
#include "editortypes.h"
#include "audiohardware.h"

#define MINSETTINGSEDITORHEIGHT 32// Y-Font Size

class AudioDevice;
class MIDIInputDevice;
class MIDIOutputDevice;
class Seq_MIDIRouting_Router;
class VSTPlugin;
class Edit_Settings;
class Directory;
class SynthDevice;
class DeviceProgram;

class Edit_Settings_Audio:public guiWindow
{ 
public:
	enum{
		GID_AUDIO,
		GID_AUDIOSYSTEM,

		GID_AUDIODEVICES_I,
		GID_AUDIODEVICES,

		GID_USEAUDIODEVICE,

		GID_AUDIODEVICEINFO_I,
		GID_AUDIOINFOID,

		GID_AUDIODEVICECHANNELS_I,

		GID_USEAUDIODEVICEID,
		GDI_DEVICEPREFS,

		GID_BSI,
		GID_BUFFERSIZE,
		GID_BUFFERSIZEINT,
		GID_BUFFERSIZEUSE,

		GID_LI_I,
		GID_LI,
		GID_LO_I,
		GID_LO,

		GID_AUDIOAUTOINPUT,
		GID_ASIOQUESTION
	};

	Edit_Settings_Audio(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_AUDIO;}

	void Init();
	void Gadget(guiGadget *);
	void ShowSetAudioDevices();
	void ShowAudioDeviceChannels();
	void ShowAudioDeviceInfo();
	void AddInfoString(AudioDevice *,char *name,int index);
	void ShowBufferSize();
	void RefreshRealtime();

private:
	Edit_Settings *editor;
	AudioDevice *audiodevice;

	guiGadget *buffersize,*usedevice;
	guiGadget_Integer *buffersizeint;
	guiGadget *set_audiohardware,*devicesettings,*set_autoaudioinput;
	guiGadget_ListBox *set_audiodevices,*audioinfo;
	guiGadget_ListView *audiochannels;
	guiGadget_Number *lat_in,*lat_out;

	int setSize,setSampleRate,userSetSize;
};

class Edit_Settings_Plugins:public guiWindow
{
public:
	enum{
		GID_PLUGIN,
		GID_PINFO,
		GID_PLIST,
		GID_ADD,
		GID_DELETE,

		#ifdef TRYCATCH
		GID_CHECKPLUGINS,
#endif

	};

	void Gadget(guiGadget *);
	void Init(char *,int index);
	void ShowVSTDirectories();
	void ShowDeleteButton();

	Directory *activevstdirectory;
	guiGadget *add,*del
		#ifdef TRYCATCH
		,*checkplugins
#endif
		;
	guiGadget_ListBox *vstdirectory;
	Edit_Settings *editor;

	int vstindex,selection;
};

class Edit_Settings_VST2:public Edit_Settings_Plugins
{ 
public:
	Edit_Settings_VST2(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_PLUGINGS;}
	void Init();
};

class Edit_Settings_VST3:public Edit_Settings_Plugins
{ 
public:
	Edit_Settings_VST3(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_PLUGINGS;}
	void Init();
};

class Edit_Settings_VSTFX:public guiWindow
{ 
public:
	enum{
		GID_FXINFO,
		GID_FXVST2,GID_FXVST3,GID_FXFX,GID_FXINSTRUMENTS,
		GID_PINFO,
		GID_PLIST,
		GID_PTINFO,GID_PTINFOTEXT,
	};

	enum{
		SHOW_VST2=1,
		SHOW_VST3=2,
		SHOW_VSTFX=4,
		SHOW_VSTINSTRUMENTS=8
	};

	Edit_Settings_VSTFX(Edit_Settings *ed)
	{
		editor=ed;editorid=EDITORTYPE_SETTINGS_VSTFX;activevsteffect=0;
		flag=SHOW_VST2|SHOW_VST3|SHOW_VSTFX|SHOW_VSTINSTRUMENTS;
	}
	void Init();
	void ShowVSTEffects();
	void ShowInfo();

	void Gadget(guiGadget *);
	bool GadgetListView(guiGadget_ListView *,int x,int y);

	Edit_Settings *editor;
	guiGadget_ListView *vstview;
	guiGadget *info;
	VSTPlugin *activevsteffect;
	int flag;
};

class Edit_Settings_CrashedPlugins:public guiWindow
{ 
public:
	enum{
		GID_CPINFO,
		GID_DELETE,
		GID_PLIST
	};

	Edit_Settings_CrashedPlugins(Edit_Settings *ed)
	{
		editor=ed;
		editorid=EDITORTYPE_SETTINGS_CRASHEDPLUGIN;
		selected=0;
	}

	void Init();
	void ShowCrashedPlugins();
	void Gadget(guiGadget *);

	Edit_Settings *editor;
	guiGadget *del;
	guiGadget_ListBox *crashed;
	int selected;
};

class Edit_Settings_MIDI:public guiWindow
{ 
public:
	enum{
		GID_MIDI,
		GID_SENDCYCLENOTES
	};

	Edit_Settings_MIDI(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_MIDI;}
	void Init();
	void Gadget(guiGadget *);

private:
	Edit_Settings *editor;

	guiGadget *mport,*mhport,*maudioport,*maudiotype;
	guiGadget *sendcyclenotes;
};

class Edit_Settings_Metronom:public guiWindow
{ 
public:
	enum{

		GID_METRO,

		GID_SENDMETROMIDI,
		GID_SENDMETROAUDIO,

		GID_MHEAD,
		GID_MCHL_I,
		GID_MCHL,
		GID_MPORT_I,
		GID_MPORT,
		GID_MKEY_I,
		GID_MKEY,
		GID_MVELO_I,
		GID_MVELO,
		GID_MVELOOFF_I,
		GID_MVELOOFF,

		GID_MHHEAD,
		GID_MHCHL_I,
		GID_MHCHL,
		GID_MHPORT_I,
		GID_MHPORT,
		GID_MHKEY_I,
		GID_MHKEY,
		GID_MHVELO_I,
		GID_MHVELO,
		GID_MHVELOOFF_I,
		GID_MHVELOOFF,
		GID_MAPORT_I,
		GID_MAPORT,
		GID_MATYPE_I,
		GID_MATYPE
	};

	Edit_Settings_Metronom(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_METRONOM;}
	void Init();
	void SendMetro(int i);
	void Gadget(guiGadget *);
	void ShowMetroPorts();
	void ShowMetroAudio();

private:
	Edit_Settings *editor;

	guiGadget *mport,*mhport,*maudioport,*maudiotype;
	guiGadget *sendcyclenotes,*sendMIDIcontrol,*sendMIDImtc;
};

class Edit_Settings_MIDIInput:public guiWindow
{ 
public:
	enum{
		GID_MIDIINPUT,

		GID_DEVICEINFOSTRING_I,
		GID_DEVICEINFOSTRING,

		GID_MIDIDEVICEINFO_I,
		GID_PORTS
	};

	Edit_Settings_MIDIInput(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_MIDIINPUT;selectedport=0;}
	void Init();
	void Gadget(guiGadget *);
	bool GadgetListView(guiGadget_ListView *,int x,int y);
	void ShowPorts();
	void ShowPortInfo();

private:
	Edit_Settings *editor;

	guiGadget *infostring;
	guiGadget_ListView *ports;
	int selectedport;
};

class Edit_Settings_MIDIOutput:public guiWindow
{ 
public:
	enum{
		GID_MIDIOUTPUT,

		GID_DEVICEINFO,

		GID_DEVICEINFOSTRING_I,
		GID_DEVICEINFOSTRING,

		GID_MIDIDEVICEINFO_I,
		GID_PORTS
	};

	Edit_Settings_MIDIOutput(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_MIDIINPUT;selectedport=0;}
	void Init();
	void ShowPorts();
	void ShowPortInfo();
	void Gadget(guiGadget *);
	bool GadgetListView(guiGadget_ListView *,int x,int y);

private:
	Edit_Settings *editor;
	guiGadget *infostring;
	guiGadget_ListView *ports;

	int selectedport;
};

class Edit_Settings_Automation:public guiWindow
{
public:
	enum{
		GID_AUTOMATION,
		GID_RECWHILEPLAYBACK,
		GID_RECWHILERECORD,
		GID_APLUGINS,
		GID_ACAUDIOVOLUME,
		GID_ACAUDIOPAN,
		GID_MIDIVELOCITY,
		GID_MIDIPITCHBEND,
		GID_MIDIVOLUME,
		GID_MIDIMODULATION,
		GID_MUTE,
		GID_SOLO
	};

	Edit_Settings_Automation(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_AUTOMATION;}
	void Init();
	void Gadget(guiGadget *);

	Edit_Settings *editor;
};

class Edit_Settings_Files:public guiWindow
{ 
public:
	enum{
		GID_FILES,
		GID_FILE_RAFI,
		GID_FILE_RAF,

		GID_MIDIFILE,
		GID_AUTOSAVE,
	};

	Edit_Settings_Files(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_FILES;}
	void Init();
	void ShowRAFSize();
	void Gadget(guiGadget *);

private:
	Edit_Settings *editor;
	guiGadget *rafsize;
};

class Edit_Settings_UI:public guiWindow
{ 
public:
	enum{
		GID_UI,
		GID_MOUSEHV
	};

	Edit_Settings_UI(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_UI;}
	void Init();
	void Gadget(guiGadget *);

private:
	Edit_Settings *editor;
};

class Edit_Settings_Keys:public guiWindow
{ 
public:
	enum{
		GID_KEYS,
		GID_SPACE
	};

	Edit_Settings_Keys(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_KEYS;}
	void Init();
	void Gadget(guiGadget *);

private:
	Edit_Settings *editor;
};

class Edit_Settings_AudioInput:public guiWindow
{ 
public:
	enum{
		GID_AI,

		GID_PORTINFO,
		GID_PDCONNECT,
		GID_DEVICEINFO,

		GID_DEVICEINFOSTRING_I,
		GID_DEVICEINFOSTRING,

		GID_MIDIDEVICEINFO_I,
		GID_TYPE,
		GID_PORTS,
		GID_CHANNELS
	};

	Edit_Settings_AudioInput(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_AUDIOINPUT;selectedport=0;selectedtype=CT_MONO;selectedchannel=0;}
	void Init();
	void Gadget(guiGadget *);
	bool GadgetListView(guiGadget_ListView *gl,int x,int y);
	void RefreshRealtime_Slow();

	void ShowPorts();
	void ShowPortInfo();

	void ShowPortChannels();
	void ShowTypes();
private:
	Edit_Settings *editor;

	guiGadget *porttype,*port,*device,*infostring;
	guiGadget_ListView *type,*ports,*channels;

	int selectedtype,selectedport,selectedchannel;
};

class Edit_Settings_AudioOutput:public guiWindow
{ 
public:
	enum{
		GID_AO,

		GID_PORTINFO,
		GID_PDCONNECT,
		GID_DEVICEINFO,

		GID_DEVICEINFOSTRING_I,
		GID_DEVICEINFOSTRING,

		GID_MIDIDEVICEINFO_I,
		GID_TYPE,
		GID_PORTS,
		GID_CHANNELS
	};

	Edit_Settings_AudioOutput(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_AUDIOOUTPUT;selectedport=0;selectedtype=CT_MONO;selectedchannel=0;}
	void Init();
	void Gadget(guiGadget *);
	bool GadgetListView(guiGadget_ListView *,int x,int y);
	void RefreshRealtime_Slow();

	void ShowPorts();
	void ShowPortInfo();
	void ShowPortChannels();
	void ShowTypes();

private:
	Edit_Settings *editor;

	guiGadget *porttype,*port,*device,*infostring;
	guiGadget_ListView *type,*ports,*channels;

	int selectedtype,selectedport,selectedchannel;
};

class Edit_Settings_AudioFiles:public guiWindow
{ 
public:
	enum{
		GID_AAF,
		GID_AINFO,
		GID_ALIST,
		GID_ADD,
		GID_DELETE
	};

	Edit_Settings_AudioFiles(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_AUDIOFILES;}
	void Init();
	void Gadget(guiGadget *);
	void ShowAudioDirectories();

private:
	Edit_Settings *editor;
	Directory *activedirectory;
	int selection;
	guiGadget *add,*del;
	guiGadget_ListView *directory;
};

class Edit_Settings_Project:public guiWindow
{ 
public:
	enum{
		GID_PROJECT,
		GID_PRONAMETEXT,
		GID_PRO_NAME,
		GID_PRODIRECTORY_I,
		GID_PRODIRECTORY
	};

	Edit_Settings_Project(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_PROJECT;}
	void Init();
	void Gadget(guiGadget *);

private:
	Edit_Settings *editor;
	guiGadget *pname;
};

class Edit_Settings_ProjectAudio:public guiWindow
{ 
public:
	enum{
		GID_PROJECTA,
		GID_SRI,
		GID_SR,
		GID_PANLAWI,
		GID_PANLAW,
		GID_BSI
	};

	Edit_Settings_ProjectAudio(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_PROJECTAUDIO;}
	void Init();
	void Gadget(guiGadget *);
	void ShowProjectSampleRate();
	void ShowProjectPanLaw();
	void RefreshRealtime();

private:
	Edit_Settings *editor;
	int projectsample,p_db;
	guiGadget *pro_samplerate,*pro_panlaw;
	guiGadget_Integer *pro_buffersizeint;
};

class Edit_Settings_ProjectGUI:public guiWindow
{ 
public:
	enum{
		GID_PROJECTUI,

		GID_PROSMPTEI,
		GID_PROSMPTE,

		GID_PROMEASUREI,
		GID_PROMEASURE
	};

	Edit_Settings_ProjectGUI(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_PROJECTGUI;}
	void Init();
	void Gadget(guiGadget *);

private:
	Edit_Settings *editor;
	guiGadget *pro_smpteformat,*pro_measureformat;
};

class Edit_Settings_Song:public guiWindow
{ 
public:
	enum{
		GID_SONG,

		GID_SONGNAMETEXT,
		GID_SONG_NAME,
		GID_SONGDIRECTORY_I,
		GID_SONGDIRECTORY,

		GID_PTRACKTYPE_I,
		GID_PTRACKTYPE,

		GADGET_SONG_NOTEDISPLAY_I,
		GADGET_SONG_NOTEDISPLAY,

		GADGET_SONG_SMPTEOFFSET_I,
		GADGET_SONG_SMPTEOFFSET
	};

	Edit_Settings_Song(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_SONG;}
	void Init();
	void Gadget(guiGadget *);
	void ShowPrefTrackType();

private:
	Edit_Settings *editor;
	guiGadget *sname,*set_songnotes,*preftracktype;
	guiGadget_Time *smpteoffset;
};

class Edit_Settings_SongSync:public guiWindow
{ 
public:
	enum{
		GID_SONGSYNC,
		GADGET_SYNCINTERN,
		GADGET_SYNCMTC,
		GADGET_SYNCMC,
		GADGET_SENDMTC,
		GADGET_SENDMC
	};

	Edit_Settings_SongSync(Edit_Settings *ed){editor=ed;editorid=EDITORTYPE_SETTINGS_SONGSYNC;}
	void Init();
	void Gadget(guiGadget *);
	void ShowSync();

private:
	Edit_Settings *editor;

	guiGadget *syncintern,
		*syncmtc,
		*syncmc,*sendmtc,*sendmc;
};

class Edit_Settings:public guiWindow
{
public:
	Edit_Settings();
	~Edit_Settings()
	{
		if(settingname_help)
			delete settingname_help;
	}

	enum{
		PRO_PROJECT,
		PRO_PROJECT_AUDIO,
		PRO_PROJECT_GUI
	};

	enum{
		SONG_SONG,
		SONG_SYNC,
		SONG_NOTEDISPLAY,
		SONG_GMDISPLAY,
		SONG_METRO,
		SONG_ROUTING
	};

	enum{
		EDIT_SMPTEOFFSET,
		EDIT_PROGRAM_PROGRAMSELECT=101,
		EDIT_PROGRAM_BANKSELECT,
		EDIT_DEFAULTSONGLENGTH,
		EDIT_AUDIOTHRESH,
		EDIT_AUDIOENDTHRESH
	};

	enum{
		AUDIOSETTINGS,

		AUDIOSETTINGS_AUDIOOUTCHANNELS,
		AUDIOSETTINGS_AUDIOINCHANNELS,

		AUDIOSETTINGS_AUDIODIRECTORY,
		AUDIOSETTINGS_PLUGINS,
		AUDIOSETTINGS_VSTDIRECTORY,
		AUDIOSETTINGS_VSTDIRECTORY3,
		AUDIOSETTINGS_VSTFX,
		AUDIOSETTINGS_CRASHEDPLUGINS,

		MIDISETTINGS,
		METROSETTINGS,
		MIDISETTINGS_OUTPUT,
		MIDISETTINGS_INPUT,
		AUTOMATIONSETTINGS,
		GUISETTINGS,
		FILESETTINGS,
		PROGRAMSETTINGS,
		LANGUAGESETTINGS,
		KEYSSETTINGS,
		INTERNSETTINGS
	};

	void Save();
	void ResetGadgets();

	void RefreshAudioDirectories();
	void CloseAllAudioDeviceWindows();
	void FreeMemory();

	EditData *EditDataMessage(EditData *data);
	void Gadget(guiGadget *);
	void ShowSettings();
	void ShowSettingsData(int flag=0);
	void ShowTargetTracks();

	void ShowDefaultSongLength();

	void InitSelectBox();
	void InitEditField();
	void InitGadgets();
	guiMenu *CreateMenu();

	void Init();
	void RefreshRealtime();

	guiGadgetList setGadgets1;

	int settingsselection;



	// Devices
	MIDIInputDevice *MIDIinputdevice;
	MIDIOutputDevice *MIDIoutdevice;
	AudioDevice *audiodevice;

	// Devices->ProgramChange
	void ShowMIDIDevicePrograms();
	void ShowMIDIDeviceProgramInfo();
	void ShowMIDIProgramName();
	void ShowMIDIDeviceName();

	SynthDevice *activeMIDIdevice;
	guiGadget_ListBox *selectbox,*MIDIdevices;
	guiGadget *MIDIdevicename,*addMIDIdevice,*deleteMIDIdevice;
	bool dontshowMIDIdevicename;

	DeviceProgram *activeMIDIprogram;
	guiGadget_ListBox *deviceprograms;
	bool dontshowMIDIprogramname;
	guiGadget *addMIDIprogram,*deleteMIDIprogram,*programname,
		*programchannel,*programprogram,*programbank,*programbankuse;

	// MIDI In
	guiGadget_ListBox *MIDIinputports,*MIDIinputdevices;

	guiGadget
		*MIDIinputfilter,
		*MIDIinputstring,
		*MIDIinputsync,
		*MIDIintempoquant;
	bool MIDIinputfilter_status;
	char *GetMIDIInFilterName();

	// MIDI Out
	guiGadget_ListBox *MIDIOutputDevices,*MIDIoutputports;
	guiGadget 
		*defaultdevice,
		*MIDIclockout,
		*MIDIoutmetronome,
		*mtcout;
	char *GetMIDIOutFilterName();
	bool MIDIoutputfilter_status;

	guiGadget *MIDIoutmetrochannel,
		*MIDIoutmetronote,
		*MIDIoutmetrovelo,
		*MIDIsendsolomutecontrol,
		*MIDIoutmetrochannel_hi,
		*MIDIoutmetronote_hi,
		*MIDIoutmetrovelo_hi,
		*sendcyclenotes,
		*MIDIoutputstring,
		*MIDIdeviceoutputfilter,
		*songlength,
		*defaulttime;

	int defaulttime_status;

	guiGadget_ListBox *set_audiodevices,*vstdirectories,*audiooutportlist,
		*audioinportlist,*audiooutlist,*audioinlist,*audiochannels,*audiooutports,*audioinports;

	guiGadget *set_audiohardware,
		*set_rafbuffer,
		*set_setsize,
		*set_setsizecycle,
		*set_autoaudioinput,
		*useaudiodevice,
		*samplerates,
		*vstinfo,
		*vstadd,
		*vstdelete,	
		*audiodeviceprefs,
		*infotext,
		*audioinporttype,
		*audiooutporttype,
		*g_cpucores,
		// Sync
		*globalsmpte,
		*sendMIDIclocksp,
		*quantsongpositiontoMIDIclockres,
		*receiveMIDIclocksp,
		*sendmtc,
		*receivemtc,
		*smptedisplayoffset,
		*pluginsettings;

	int vstindex;

	// Project
	guiGadget *pro_samplerate,*pro_panlaw,*pro_xboxthreshold,*pro_xboxrecthreshold,*pro_smpteformat,*pro_measureformat;
	int projectsample;

	// Song
	guiGadget *set_songname;
	char *songnamebuff;

	guiGadget *set_songtime,*set_songnotes,*set_songgm;

	// Hardware
	void ShowHWDevices();

	guiGadget *hw_system;
	int selected_hw_system;
	guiGadget *hw_device;
	int selected_hw_device;

	enum SelectInputType{
		SIT_CHANNEL,
		SIT_TYPE
	};

	void ShowActiveTrack();
	char *focustrackname;

	MIDIInputDevice *set_selectinputdevice;
	int set_selecttypeindex,
		set_selectedchannel,
		set_selectedtrack,
		set_selectedtype;

	guiGadget_ListBox *set_selectchannel,*set_selecttrack;

	guiGadget *set_selectrouting, // Song Input Routing
		*set_selectinputtype,
		*set_addsongsfocustrack,
		*set_sendtofocustrack,
		*set_deletetrack;

	Seq_Project *editproject;

	char *settingname_help;

private:

	void AddRoutingInfo(Seq_MIDIRouting_Router *,char *);
	void ShowProjectAudioThresh();
	void NewProjectMeasureFormat();

	void ShowAudioInports();
	void ShowAudioOutports();

	guiWindow *openeditor;

	int audioinporttypeselection,audiooutporttypeselection,
		outportsselection,inportsselection;

	Directory *activevstdirectory,*activeaudiodirectory;
};
#endif
