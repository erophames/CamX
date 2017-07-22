#ifndef CAMX_SETTINGS_H
#define CAMX_SETTINGS_H 1

#include "defines.h"
#include "vstdirectory.h"
#include "eventeditordefines.h"
#include "programsettings.h"
#include "editortypes.h"

#define SETTINGSFILE_MIDIDEVICES_DIR "\\Preferences\\MIDI Devices\\"
#define SETTINGSFILE_AUDIODEVICES_DIR "\\Preferences\\Audio Devices\\"
#define SETTINGSFILE_DEVICEPROGRAMS_DIR "\\Preferences\\DevicePrograms\\"

#define SETTINGSFILE_PROCESSORS_FILE "\\Preferences\\camx_procsettings.cpx"
#define SETTINGSFILE_GUI_FILE "\\Preferences\\camx_guisettings.cpx"
#define SETTINGSFILE_SETTINGS_FILE "\\Preferences\\camx_globalsettings.cpx"

#define SETTINGSFILE_PLUGINTEST_FILE "\\Preferences\\camx_plugintest.cpx"
#define SETTINGSFILE_CRASHEDPLUGINS_FILE "\\Preferences\\camx_crashedplugins.cpx"

#ifdef WIN32

#ifdef WIN64
#define SETTINGSFILE_SETTINGS_PLUGINS "\\Preferences\\camx_plugins_64.cpx"
#define SETTINGSFILE_SETTINGS_PROJECTS "\\Preferences\\camx_projects_64.cpx"
#else
#define SETTINGSFILE_SETTINGS_PLUGINS "\\Preferences\\camx_plugins_32.cpx"
#define SETTINGSFILE_SETTINGS_PROJECTS "\\Preferences\\camx_projects_32.cpx"
#endif

#endif


#define ADDNOTHING_TOMIDIFILE 0
#define ADDGM_TOMIDIFILE 1
#define ADDGS_TOMIDIFILE 2

#define CREATE_EVENTS 8

// PEAK FILES
#define PEAKFILES_TOFILE 0
#define PEAKFILES_NOPEAKFILE 1

class Seq_Project;

#define UNDOSTEPS_25 0
#define UNDOSTEPS_50 1
#define UNDOSTEPS_75 2
#define UNDOSTEPS_100 3
#define UNDOSTEPS_UNLIMITED 4

#define ARRANGEEDITOR_SHOWNOTES_OFF 0
#define ARRANGEEDITOR_SHOWNOTES_LINES 1
#define ARRANGEEDITOR_SHOWNOTES_NOTES 2

class Directory;

class VSTPlugin_Settings:public Object
{
public:
	VSTPlugin_Settings()
	{
		fulldllname=dllname=effectname=company=0;
		ins=outs=0;
		type=0;
		internflag=0;
		active=true;
		audioeffecttype=0;
		synth=canreceiveEvents=canDoubleReplacing=NoSoundInStop=false;
	}

	void FreeMemory();
	void ReadSettings(camxFile *);

	char *fulldllname,*dllname,*effectname,*company;
	LONGLONG filesize;
	int numberofparameter,numberofprograms,ins,outs,type,vstversion,audioeffecttype,internflag,version;
	bool synth,canreceiveEvents,canDoubleReplacing,NoSoundInStop,active;
};

class guiWindowPosition
{
public:
	guiWindowPosition(){set=false;maximized=false;dontchange=false;}

	void Save(camxFile *);
	void Load(camxFile *);

	int x,y,width,height;
	bool set,maximized,dontchange;
};

#define VST2 0
#define VST3 1
#define MAXVSTTYPES 2

// Audio Mixer
#define SHOW_AUDIOINPUT 1
#define SHOW_AUDIOOUTPUT (1<<1)
#define SHOW_MIDI (1<<2)
#define SHOW_AUDIO (1<<3)
#define SHOW_TRACKS (1<<4)
#define SHOW_CHANNELS (1<<5)
#define SHOW_INSTRUMENTS (1<<6)
#define SHOW_BUS (1<<7)
#define EX_SHOW_MASTER (1<<8)
#define SHOW_IO (1<<9)
#define SHOW_PAN (1<<10)
#define SHOW_FX (1<<11)
#define SHOW_EVENTTYPE (1<<12)
#define SHOW_INPUTFX (1<<13)
#define SHOW_AUTOMATION (1<<14)
#define SHOW_VOLUMECURVES (1<<15)
#define SHOW_AUTO (1<<16)

enum
{
	AUTO_ALL,
	AUTO_SELECTED,
	AUTO_FOCUS,
	AUTO_WITHAUDIOORMIDI,
	AUTO_WITHAUDIO,
	AUTO_WITHMIDI,
	AUTO_INSTRUMENT,
	AUTO_RECORDINGTRACKS,

	AUTO_ENDOFFLAGS
};

enum
{
	PATTERING_OFF,
	PATTERING_FROMTO,
	PATTERING_UNDER
};

class PianoSettings
{
public:
	int status;
	UBYTE channel;
	UBYTE controlnr;
};

class Settings
{
public:
	Settings();
	~Settings();

	enum UnusedAudioFiles{
		SETTINGS_AUDIODEINITASK,
		SETTINGS_NODeInit,
		SETTINGS_AUTODeInit
	};

	enum SettingsType{
		SETTINGSFILE_GUI,
		SETTINGSFILE_SETTINGS,
		SETTINGSFILE_PLUGINS,
		SETTINGSFILE_PROJECTS,
		SETTINGSFILE_DEVICEPROGRAMS,
		SETTINGSFILE_PROCESSORS,
		SETTINGSFILE_AUDIODEVICES,
		SETTINGSFILE_MIDIDEVICES,
		SETTINGSFILE_PLUGINTEST,
		SETTINGSFILE_CRASHEDPLUGINS,

		LASTSETTINGS
	};

	enum{
	PRECOUNTER_ATMEASUREONE,
	PRECOUNTER_ALWAYS
	};

	enum{
		OPENDBCLK_PIANO,
		OPENDBCLK_EVENT,
		OPENDBCLK_DRUM,
		OPENDBCLK_WAVE,
	};

	enum{
		PLUGINCHECK_NOCHECKS,
		PLUGINCHECK_TRYCATCH,
		PLUGINCHECK_TRYCATCHANDTIMER
	};

	enum{
		SPLITMIDIFILE0_OFF,
		SPLITMIDIFILE0_ON,
		SPLITMIDIFILE0_ASKMESSAGE
	};

	void InitDefaultWindowPositions();
	void SetMultiEditing(bool);
	void SetLastAudioManagerFile(char *);
	
	char *lastaudiomanagerfile;

	int mixerzoom,defaultzoomy,defaultzoomx,outportsselection,inportsselection,audioinporttypeselection,audiooutporttypeselection,
		flag_unusedaudiofiles;

	SynthDeviceList synthdevices;
	guiWindowPosition windowpositions[EDITORTYPE_LASTEDITOR];

	// Player
	bool player_loopsongs,player_loopprojects;

	// Autosave
	int autosavemin,defaultsonglength_measure;

	bool createnewtrack_trackadded,createnewtrackaftercycle,createnewchildtrackwhenrecordingstarts;

	char *GetSettingsFileName(int type);

	char *settingsfilename[LASTSETTINGS],
	 *prevprojects_dirname[6], // Directory, 0=last active project
	 *prevprojects_proname[6]; // Name

	bool recording_MIDI,recording_audio,importfilequestion,mouseonlyvertical,setfocustrackautotorecord;

	int defaultmousequantize;

	// Editor
	int editordefaulttimeformat,opendoubleclickeditor;
	bool showbothsformatsintimeline;

	// MIDI Files
	int addgsgmtoMIDIfiles;

	// MIDI
	bool sendprectrl;

	// Arrange Editor
	int shownotesinarrageeditor;

	bool showarrangecontrols,showarrangetracknumber,showarrangepatternlist,
	 showarrangeshowallpattern,showarrangetextmap,showarrangemarkermap;

	int arr_textmapheight,arr_markermapheight,arr_signaturemapheight,
	 arr_tempomapheight,arr_foldermapheight,arr_overviewheight,
	 arr_trackswidth,arr_patternlistwidth;

	// Drum Editor
	int drum_trackswidth;

	// Piano Editor
	OSTART lastsetnotelength; // -1 not set
	int piano_keyswidth,piano_overviewheight;
	int defaultpianosettings;
	double piano_keyheight; // %
	bool piano_showwave,piano_default_notelengthuseprev,playmouseover;

	// Wave Editor
	int wave_trackswidth;

	// Big Time
	bool bigtime_showbw;

	// Tempo
	int defaulttemporange;
	int tempo_timewidth,tempo_tempostringwidth,tempo_mapwidth,tempo_statuswidth;

	// Manager
	bool manager_showinfo,manager_showregions,manager_showfullpath,
	 manager_showmb,manager_showtime,manager_shownotfound,
	 manager_showrecorded,manager_showonlysamplerate,manager_showintern;

	// Event Editor
	int event_timewidth,event_statuswidth,event_byte1width,event_byte2width,
	 event_channelwidth,event_infowidth,event_byte3width,event_curvewidth;
	bool event_showgfx;

	// Marker
	int marker_positionwidth,marker_textwidth,marker_textnamewidth,marker_endwidth,marker_colourwidth;

	// Text
	int text_positionwidth,text_textwidth,text_textstringwidth,text_textnamewidth;

	// Audio Master Mixer
	bool showmasterio;

	// Audio Track Editor
	bool showonlyaudiotracks;

	int eventeditorgmmode,arrangeeditorfiledisplay;

	void SetPlaybackTrigger(bool on);
	void InitPrevProjects();
	void SetPrevProject(Seq_Project *);

	OList vstdirectories[MAXVSTTYPES],audiodirectores,settingsreadvsteffects,settingsreadvstinstruments;

	// All
	char *audiodefaultdevice;
	int defaultprojectmeasureformat,undosteps,defaultsettingsselection,headerheight,
		numberofpremetronomes,defaultpianoeditorlength,audioresamplingformat,
		smpteoffset_h,smpteoffset_m,smpteoffset_s,smpteoffset_f,smpteoffset_sf;

	double checkaudiothreshhold,checkaudioendthreshold,default_masterpausems;
	
	int default_masterformat,default_masterchannels,
		defaultrecordtracktype,projectstandardsmpte,peakfiles,defaultnotetype,loadsettingfile_version,
		plugincheck,precountertype,
		splitMIDIfiletype0;

	bool showmonitordevicename,autoloadlastusedproject,doublestopeditrefresh,usecheckaudiothreshold,autocutzerosamples,
		autoupdatequestion,importaudiofiles,askimportaudiofiles,autoinstrument,autotrackMIDIinthru,default_masternormalize,default_masterpausesamples,default_mastersavefirst,
		waitforMIDIplayback,waitforMIDIrecord,usepremetronome,recordtofirstselectedMIDIPattern,
		noteendsprecounter,soloeditor,followeditor,
		sendmetroplayback,displaynoteoff_monitor,displayeditortooltip,
		transportdesktop,openmultieditor,autoaudioinput,automutechildsparent,allowtempochangerecording,
		ex_splitMIDIfilestype0,showsamplescale,showsampleregions,showsamplevolume;

	// Vst Plugins
	void DeleteAllVSTDirectories();

	// VST
	Directory *AddVSTDirectory(int i,char *);
	Directory *DeleteVSTDirectory(int i,Directory *);
	Directory *FirstVSTDirectory(int i){return (Directory *)vstdirectories[i].GetRoot();}
	int GetCountOfVSTDirectories(int i){return vstdirectories[i].GetCount();}
	void DeleteAllAudioDirectories();

	// Audio
	Directory *AddAudioDirectory(char *);
	Directory *DeleteAudioDirectory(Directory *);
	Directory *FirstAudioDirectory(){return (Directory *)audiodirectores.GetRoot();}
	int GetCountOfAudioDirectories(){return audiodirectores.GetCount();}

	void Load(char *);
	void LoadPluginSettings();
	void LoadProjects();

	void Save(char *);
	void SavePluginSettings();

	void LoadCrashedPlugins();
	void SaveCrashedPlugins();
	
	void SaveProjects();

	void LoadDevices();
	void SaveDevices();
	void LoadDevicePrograms();
	void SaveDevicePrograms(SynthDevice *);

	int audiomixersettings[5];
	int audiomixersettings_solo[5];
	int autotracking[5];	
	int autopattering[5];
	int arrangetracking[5];
	int arrangesettings[5];

	enum
	{
		REALTIMERECTEMPOEVENTS_DELETE,
		REALTIMERECTEMPOEVENTS_CONVERT,
		REALTIMERECTEMPOEVENTS_CHANGE

	};
	int realtimerecordtempoevents;

	PianoSettings pianosettings[5];

	// Arrange
	int lastselectmixset,lastselectarrangeset,lastselectpianoset;
	bool showarrangelist,showarrangeeffects;
	bool showarrangemaster,showarrangebus,showarrangemetro;

	// Drum
	bool showdrumlist,showdrumeffects;

	// Metro
	int defaultmetrochl_b,defaultmetroport_b,defaultmetrokey_b,defaultmetrovelo_b,defaultmetrovelooff_b;
	int defaultmetrochl_m,defaultmetroport_m,defaultmetrokey_m,defaultmetrovelo_m,defaultmetrovelooff_m;
	int defaultmetroaudiochanneltype,defaultmetroaudiochannelportindex;
	bool defaultmetrosendMIDI,defaultmetrosendaudio;

	// Automation
	bool automation_createpluginautomationtrack,automation_createvolumetrack,automation_createpantrack;
	bool automation_createMIDIvolumetrack,automation_createMIDImodulationtrack,automation_createMIDIpitchbendtrack,automation_createMIDIvelocitytrack,
		automation_createmute,automation_createsolo,automationrecordingrecord,automationrecordingplayback;

	// Big Time
	int defaultbigtimezoom;

	bool askforasio,checkspacenonfocus;
};
#endif