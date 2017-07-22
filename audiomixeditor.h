#ifndef CAMX_AUDIOMIXEDITOR
#define CAMX_AUDIOMIXEDITOR 1

#include "audiodefines.h"
#include "audiochannel.h"
#include "object.h"
#include "editor.h"
#include "object_track.h"
#include "cshowpeak.h"

#define EQHEIGHT (maingui->GetFontSizeY()*2)

class InsertAudioEffect;
class guiGadget;

enum{

	// Effects
	OBJECTID_EFFECTS=OI_LAST,

	OBJECTID_FX,
	OBJECTID_FXEDIT,
	OBJECTID_EMPTYFX,
	OBJECTID_EMPTYINPUTFX,
	OBJECTID_ABFX,
	OBJECTID_ABFXINPUT,

	OBJECTID_SEND,
	OBJECTID_SENDEDIT,
	OBJECTID_EMPTYSEND,

	LASTEFFECTID,

	// Slider
	OBJECTID_SLIDER,

	OBJECTID_CHILDOPEN,
	OBJECTID_MUTE,
	OBJECTID_RECORD,
	OBJECTID_SOLO,
	OBJECTID_NAME,
	OBJECTID_IOINPUT,
	OBJECTID_IOMIDIINPUT,
	OBJECTID_IOOUTPUT,
	OBJECTID_IOMIDIOUTPUT,
	OBJECTID_INPUTTYPE,
	OBJECTID_THRU,
	OBJECTID_INPUTMONITORING,
	OBJECTID_MIDITHRU,
	OBJECTID_PAN,
	OBJECTID_MIDIPAN,
	OBJECTID_TYPE,

	LASTSLIDERID,

	OBJECTID_MASTEREFFECTS,
	OBJECTID_MASTERABFX,
	OBJECTID_ABFXMASTERINPUT,
	OBJECTID_MASTERFX,
	OBJECTID_MASTERFXEDIT,
	OBJECTID_MASTEREMPTYFX,

	LASTMASTEREFFECTID,

	OBJECTID_MASTERSLIDER,

	OBJECTID_MASTERAB,
	OBJECTID_MASTERMUTE,
	//OBJECTID_MASTERTYPE,
	OBJECTID_MASTERNAME,
	OBJECTID_MASTERIOOUTPUT,

	LASTERMASTERID,
};

class Edit_AudioMix;

class Edit_AudioMixFilterChannel:public Object
{
public:
	Edit_AudioMixFilterChannel *PrevChannel(){return (Edit_AudioMixFilterChannel *)prev;}
	Edit_AudioMixFilterChannel *NextChannel(){return (Edit_AudioMixFilterChannel *)next;}
	AudioIOFX *GetIO(){return channel?&channel->io:&track->io;}
	AudioEffects *GetFX(){return channel?&channel->io.audioeffects:&track->io.audioeffects;}

	void ShowIOPeak(Edit_AudioMix *,bool force=false);

	CShowPeak peak;
	AudioChannel *channel;
	Seq_Track *track;

	int ovx,ovx2,ovy,ovy2;
	int peakx,peakx2;
	bool isMIDI,isaudio;
};

class Edit_AudioMixEffects;
class Edit_AudioMixSlider;
class Edit_AudioMixFilterChannel;

class Edit_AudioMix_FX:public guiObject
{
public:
	Edit_AudioMix_FX(Edit_AudioMixEffects *,InsertAudioEffect *,guiGadget *);
	bool ShowEffectName();
	void FreeMemory()
	{
		if(programname)delete programname;
	}
	Edit_AudioMixEffects *effects;
	InsertAudioEffect *iae;
	char *programname;
	bool onoff,bypass;
};

class Edit_AudioMix_FXEDIT:public guiObject
{
public:
	Edit_AudioMix_FXEDIT(Edit_AudioMixEffects *,InsertAudioEffect *,guiGadget *);
	Edit_AudioMixEffects *effects;
	InsertAudioEffect *iae;
};

class Edit_AudioMix_EmptyFX:public guiObject
{
public:
	Edit_AudioMix_EmptyFX(Edit_AudioMixEffects *,guiGadget *,bool is_MIDI);
	Edit_AudioMixEffects *effects;

	bool isMIDI;
};

class Edit_AudioMix_EmptyInputFX:public guiObject
{
public:
	Edit_AudioMix_EmptyInputFX(Edit_AudioMixEffects *fx,guiGadget *g)
	{
		id=OBJECTID_EMPTYINPUTFX;
		effects=fx;
		gadget=g;
	}

	Edit_AudioMixEffects *effects;
};

class Edit_AudioMix_ABFX:public guiObject
{
public:
	Edit_AudioMix_ABFX(Edit_AudioMixEffects *,guiGadget *,bool is_MIDI);
	void ShowAB();
	Edit_AudioMixEffects *effects;
	bool ab,ismaster,isMIDI;
};

class Edit_AudioMix_ABINPUTFX:public guiObject
{
public:
	Edit_AudioMix_ABINPUTFX(Edit_AudioMixEffects *,guiGadget *);
	void ShowAB();
	Edit_AudioMixEffects *effects;
	bool ab;
};

class Edit_AudioMix_Send:public guiObject
{
public:
	Edit_AudioMix_Send(Edit_AudioMixEffects *fx,AudioSend *s,guiGadget *g)
	{
		id=OBJECTID_SEND;
		effects=fx;
		send=s;
		gadget=g;
		sendname=0;

		ShowSendName();
	}

	void ShowSendName();
	void FreeMemory()
	{
		if(sendname)
			delete sendname;
	}
	Edit_AudioMixEffects *effects;
	AudioSend *send;
	char *sendname;
	bool sendpost;
};

class Edit_AudioMix_SendEdit:public guiObject
{
public:
	Edit_AudioMix_SendEdit(Edit_AudioMixEffects *fx,AudioSend *s,guiGadget_CW *gDB,double v)
	{
		id=OBJECTID_SENDEDIT;
		effects=fx;
		send=s;
		gadget=gDB;
		gadget_DB=gDB;
		volume=v;
	}

	Edit_AudioMixEffects *effects;
	AudioSend *send;
	double volume;
};

class Edit_AudioMix_EmptySend:public guiObject
{
public:
	Edit_AudioMix_EmptySend(Edit_AudioMixEffects *fx,guiGadget *g)
	{
		id=OBJECTID_EMPTYSEND;
		effects=fx;
		gadget=g;
	}

	Edit_AudioMixEffects *effects;
};

class Edit_AudioMix_Name:public guiObject
{
public:
	Edit_AudioMix_Name(Edit_AudioMixSlider *s,guiGadget *g,bool ismaster)
	{
		id=ismaster==true?OBJECTID_MASTERNAME:OBJECTID_NAME;
		slider=s;
		gadget=g;

		ShowName();
	}

	char *GetName();
	void ShowName();
	void EditName(char *newname);

	Edit_AudioMixSlider *slider;
};

class Edit_AudioMix_MIDIInput:public guiObject
{
public:
	Edit_AudioMix_MIDIInput(Edit_AudioMixSlider *s,guiGadget *g)
	{
		id=OBJECTID_IOMIDIINPUT;
		slider=s;
		gadget=g;
		ShowMIDIInput();
	}

	void ShowMIDIInput();
	Edit_AudioMixSlider *slider;
};

class Edit_AudioMix_MIDIOutput:public guiObject
{
public:
	Edit_AudioMix_MIDIOutput(Edit_AudioMixSlider *s,guiGadget *g)
	{
		id=OBJECTID_IOMIDIOUTPUT;
		slider=s;
		gadget=g;
		ShowMIDIOutput();
	}

	void ShowMIDIOutput();
	Edit_AudioMixSlider *slider;;
};

class Edit_AudioMix_Input:public guiObject
{
public:
	Edit_AudioMix_Input(Edit_AudioMixSlider *s,guiGadget *g)
	{
		id=OBJECTID_IOINPUT;
		slider=s;
		gadget=g;
		ShowInput();
	}

	void ShowInput();
	Edit_AudioMixSlider *slider;
};

class Edit_AudioMix_InputType:public guiObject
{
public:
	Edit_AudioMix_InputType(Edit_AudioMixSlider *s,guiGadget *g)
	{
		id=OBJECTID_INPUTTYPE;
		slider=s;
		gadget=g;
		ShowInputType();
	}

	void ShowInputType();
	Edit_AudioMixSlider *slider;
	int recordtracktype;
};

class Edit_AudioMix_Output:public guiObject
{
public:
	Edit_AudioMix_Output(Edit_AudioMixSlider *,guiGadget *,bool ismaster);
	void ShowOutput();
	Edit_AudioMixSlider *slider;
};

class Edit_AudioMixRoot:public guiObjectWithGadgets
{
	friend class Edit_AudioMix;

public:
	int GetBGColour();
	int GetBorder();

	Seq_Track *GetTrack(){return filterchannel->track;}
	AudioChannel *GetChannel(){return filterchannel->channel;}

	AudioIOFX *GetAudioIO(){return GetChannel()?&GetChannel()->io:&GetTrack()->io;}
	AudioEffects *GetFX(){return &GetAudioIO()->audioeffects;}
	AudioEffects *GetInputFX(){return &GetAudioIO()->audioinputeffects;}

	Edit_AudioMix *editor;
	Edit_AudioMixFilterChannel *filterchannel;
	int bgcolour;
};

class Edit_AudioMixEffects:public Edit_AudioMixRoot
{
public:
	Edit_AudioMixEffects(guiGadget_CW *,bool ismaster);

	void FreeMemory();
	void GenerateTypeString(char *string);
	void ShowEffects();
	void ShowTrackType();

	Edit_AudioMixEffects *NextChannel() {return (Edit_AudioMixEffects *)next;}

	OList effectgadgets,effecteditgadgets,sendgadgets,sendvolumegadgets;

	guiGadget_CW *effects;
	char *outputhardwarename;
	AudioPort *status_trackvchannel;

	guiGadget *eqgadget,*typegadget,*outdb,*maxdb,
		*pangadget,*emptyeffect,*emptyinputeffect,*emptysend,*emptyMIDIeffect,
		*abgadget,*abgadgetinput,*abMIDIgadget,
		*inputhardware,*outputtype,*outputhardware,*channeltype;

	int status_MIDItype,track_index;

	// Pan
	bool bypassallfx,master,recordmode,incleared,outcleared,record,abid,thrustatus,
		panonwindow,status_usetrackvchannels,isMIDI,isaudio,valueclicked,ismaster;
};

class Edit_AudioMixSlider;

class Edit_AudioMix_ChildOpen:public guiObject
{
public:
	Edit_AudioMix_ChildOpen(){id=OBJECTID_CHILDOPEN;}
	Edit_AudioMixSlider *slider;
	bool status;
};

class Edit_AudioMix_MasterAB:public guiObject
{
public:
	Edit_AudioMix_MasterAB(){id=OBJECTID_MASTERAB;}
	Edit_AudioMixSlider *slider;
	bool status;
};

class Edit_AudioMix_Mute:public guiObject
{
public:
	Edit_AudioMix_Mute(){id=OBJECTID_MUTE;}
	Edit_AudioMixSlider *slider;
	bool status;
};

class Edit_AudioMix_Record:public guiObject
{
public:
	Edit_AudioMix_Record(){id=OBJECTID_RECORD;}
	Edit_AudioMixSlider *slider;
	int recordtracktype;
	bool status;
};

class Edit_AudioMix_Solo:public guiObject
{
public:
	Edit_AudioMix_Solo(){id=OBJECTID_SOLO;}
	Edit_AudioMixSlider *slider;
	int status;
};

class Edit_AudioMix_Thru:public guiObject
{
public:
	Edit_AudioMix_Thru(){id=OBJECTID_THRU;}
	Edit_AudioMixSlider *slider;
	bool status;
};

class Edit_AudioMix_InputMonitor:public guiObject
{
public:
	Edit_AudioMix_InputMonitor(){id=OBJECTID_INPUTMONITORING;}
	Edit_AudioMixSlider *slider;
	bool status;
};

class Edit_AudioMix_MIDIThru:public guiObject
{
public:
	Edit_AudioMix_MIDIThru(){id=OBJECTID_MIDITHRU;}
	Edit_AudioMixSlider *slider;
	bool status;
};

class Edit_AudioMix_ChannelType:public guiObject
{
public:
	Edit_AudioMix_ChannelType(){id=OBJECTID_TYPE;}
	Edit_AudioMixSlider *slider;
	int type;
};

class Edit_AudioMix_MIDIPan:public guiObject
{
public:
	Edit_AudioMix_MIDIPan()
	{
		id=OBJECTID_MIDIPAN;
		clicked=false;
	}

	Edit_AudioMixSlider *slider;
	bool editable,clicked;
};

class Edit_AudioMix_Pan:public guiObject
{
public:
	Edit_AudioMix_Pan()
	{
		id=OBJECTID_PAN;
		clicked=false;
	}

	Edit_AudioMixSlider *slider;
	bool editable,clicked;
};

class Edit_AudioMixSlider:public Edit_AudioMixRoot
{
public:
	Edit_AudioMixSlider(guiGadget_CW *,bool ismaster);

	void ShowMasterAB();
	bool ShowMIDIPan(bool force,bool mouseedit=false);
	bool ShowPan(bool force,bool mouseedit=false);
	void ShowSlider();
	void ShowVolumeSlider();
	void ShowMIDIVolumeSlider();

	void ShowMIDIPeak(bool force=false);
	void ShowIOPeak(bool force=false);

	void ShowMute();
	void ShowSolo();
	void ShowRecord();
	void ShowInputMonitoring();
	void ShowMIDIThru();
	void ShowThru();
	void ShowType();
	void ShowChildOpenClose();

	void FreeMemory();

	bool GetMute();
	int GetSolo();
	bool GetRecord();
	bool GetThru();

	TrackHead *GetTrackHead();

	Peak *GetInPeak();
	Peak *GetOutPeak();

	Edit_AudioMix_ChildOpen child;
	Edit_AudioMix_MasterAB masterab;
	Edit_AudioMix_Mute mute;
	Edit_AudioMix_Record record;
	Edit_AudioMix_Solo solo;

	Edit_AudioMix_MIDIThru mthru;
	Edit_AudioMix_Thru thru;
	Edit_AudioMix_ChannelType type; // Mo,Stereo etc...
	Edit_AudioMix_Pan pan;
	Edit_AudioMix_MIDIPan mpan;
	Edit_AudioMix_InputMonitor inputmonitor;

	CShowPan cspan;
	CShowPeak peak,mpeak;

	MIDIFilter status_inputfilter;
	Edit_AudioMixEffects *effects;

	MIDIInputDevice *status_MIDIindevice;
	MIDIOutputDevice *status_MIDIoutdevice;

	double dbvalue;
	guiGadget_CW *slider;
	guiGadget *masterabgadget,*abgadget,*namegadget,*inputgadget,*inputtypegadget,*outputgadget,*inputMIDIgadget,*outputMIDIgadget;
	char *name,*inputstring,*outputstring;

	int vux,vux2,vuy,vuy2,
		sliderx,sliderx2,slidery,slidery2,
		valuex,valuex2,valuey,valuey2,

		// maxpeak
		maxpeakx,maxpeaky,maxpeakx2,maxpeaky2, // Audio
		m_maxpeakx,m_maxpeaky,m_maxpeakx2,m_maxpeaky2, // MIDI

		// MIDI
		m_panx,m_panx2,m_pany,m_pany2,
		m_vux,m_vux2,m_vuy,m_vuy2,
		m_sliderx,m_sliderx2,m_slidery,m_slidery2,
		m_valuex,m_valuex2,m_valuey,m_valuey2,
		velocity,MIDIoutputimpulse;

	bool bypassallfx,master,recordmode,incleared,outcleared,abid,thrustatus,
		panonwindow,status_usetrackvchannels,isMIDI,isaudio,
		valueondisplay,
		m_valueondisplay,
		status_noMIDIinput,status_useallinputdevices,pan_enable,
		ismaster,
		valueclicked,m_valueclicked,
		volumeeditable,MIDIvolumeeditable;
};

class Edit_AudioMix:public guiWindow
{
	friend AudioSystem;

public:
	enum{
		REFRESH_OUTCHANNELS=1,
		REFRESH_INCHANNELS=2
	};

	enum{
		ED_BUS,
		ED_TRACKS,
	};

	Edit_AudioMix();

	void RefreshSoloTrack();

	void InitIsMIDIAUDIO(Edit_AudioMixFilterChannel *);
	void CreateFilterChannels();
	void CreateTrackFilter(OList *);

	void CopyFX(AudioChannel *from);
	void PasteFX(AudioChannel *to);

	void RefreshObjects(LONGLONG type,bool editcall);

	void RefreshTrack(Seq_Track *);
	void RefreshRealtime();
	void RefreshRealtime_Slow();

	void ShowOverview();
	void ShowOverviewPositions();

	void ShowAll(bool refreshfilter=false);
	void ShowFilter();

	void SetSet(int s);

	void ShowDisplayMenu();
	guiMenu *CreateMenu();
	guiMenu *CreateMenu2();

	void CreateMenuList(guiMenu *);
	void CreateWindowMenu();

	void KeyDown();
	void KeyDownRepeat();

	void SetMasterMode();
	void Init();
	void ClearEMCs();
	void ClearEMCSlider();
	void ClearEMCEffects();

	void ClearEMCMasterSlider();
	void ClearEMCMasterEffects();

	void DeInitWindow();

	void SetAutoMode(int tracking,int pattering);
	bool CheckAuto();

	EditData *EditDataMessage(EditData *);
	void MouseWheel(int delta,guiGadget *);
	void Gadget(guiGadget *);
	void DoubleClickGadget(guiGadget *);
	void ShowEffects();
	void BuildMinMaxEffects();
	void FreeEffects();

	void MouseClickInOverview(bool leftmouse);

	void MouseClickInEffects(guiGadget_CW *effects,bool leftmouse);
	void MouseDoubleClickInEffects(guiGadget_CW *effects,bool leftmouse);
	void MouseReleaseInEffects(guiGadget_CW *effects,bool leftmouse);
	void MouseMoveInEffects(guiGadget_CW *effects);
	void DeltaInEffects(guiGadget_CW *effects);

	void MouseDoubleClickInSlider(guiGadget_CW *slider,bool leftmouse);
	void MouseClickInSlider(guiGadget_CW *slider,bool leftmouse);
	void MouseMoveInSlider(guiGadget_CW *slider);
	void DeltaInSlider(guiGadget_CW *slider);
	void MouseReleaseInSlider(guiGadget_CW *slider,bool leftmouse);
	void ShowFullSlider();

	// Master
	void ShowMasterEffects();
	void ShowFullMasterSlider();

	bool IsEffect(AudioEffects *);

	void Delete();

	OListCoosX sliderxobjects,effectxobjects;
	OListCoosY effectobjects;

	Edit_AudioMixFilterChannel masterfilterchannel;

	guiGadget_CW *effects,*mastereffects,*slider,*masterslider,*overview;
	guiGadget_Slider *horzgadget,*horzzoomgadget,*effectvertgadget;

	guiGadget *g_tracks,*g_auto,*g_audio,*g_MIDI,*g_bus,*g_channels,*g_instruments,*g_audioinput,*g_audiooutput,
		*g_io,*g_pan,*g_fx,*g_inputfx,*g_showwhat,*g_pattern,*g_set[5],
		*fx_fxgadget,*fx_fx2gadget;

	AudioChannel *laststartchannel;
	Seq_Track *opentrack,*track; // track=Single Track

	void RefreshAllMixerWithSameSet(int flag);

	int GetShowFlag();
	void ClearFlag(int flag);
	void AddFlag(int flag);

	guiMenu *menu_showmasterio;

	int set,blinkrecordcounter, // start
		maxeffects,maxinputeffects,maxsends,mixerzoom,
		overviewx2,sizeslider,
		trackingindex,patteringindex;

	bool solotrack,simple,audiomasteronly,showmasterio,blinkrecordcounter_toggle,fx_fxid,fx_fx2id,
		nooverview,nopeak,skipoverview,showoverviewonpaint,refresheffectsonly,mastermode;

protected:

	bool CompareIsMIDIAUDIO(Edit_AudioMixSlider *);
	int CreateMixerObjects(guiForm_Child *,OListCoosX *,bool slider);
	void ShowTracking();
	void ShowPattering();

	void InitGadgets();
	void ClickInEmptyArea(bool leftmouse);

	void CreateChannelTypePopMenu(Edit_AudioMixSlider *); // Stereo,Mono etc...
	void CreateOutputPopMenu(Edit_AudioMixSlider *,bool isMIDI);
	void CreateInputPopMenu(Edit_AudioMixSlider *,bool isMIDI);
	
	Edit_AudioMixFilterChannel *GetFilterAtIndex(int index){return (Edit_AudioMixFilterChannel *)filter.GetO(index);}
	Edit_AudioMixFilterChannel *FirstFilterChannel(){return (Edit_AudioMixFilterChannel *)filter.GetRoot();}
	Edit_AudioMixFilterChannel *LastFilterChannel(){return (Edit_AudioMixFilterChannel *)filter.Getc_end();}

	Edit_AudioMixFilterChannel comparefilterchannel;
	OList filter,realtimefilter; // Edit_AudioMixFilterChannel
	guiMenu *bypassallfx,*menu_showdevice;
	bool flag_showdevices;
};

#endif