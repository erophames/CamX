#ifndef CAMX_ARRANGEEDITOR_H
#define CAMX_ARRANGEEDITOR_H 1

#include "editor.h"
#include "trackfx.h"
#include "audiomixeditor.h"
#include "audiofile.h"

class Seq_Marker;
class Edit_Arrange_Channel;

enum{
	// Track
	OBJECTID_ARRANGETRACK=OI_LAST,
	OBJECTID_ARRANGEMUTE,
	OBJECTID_ARRANGEINPUT,
	OBJECTID_ARRANGERECORD,
	OBJECTID_ARRANGESHOWAUTOMATIONTRACKS,
	OBJECTID_ARRANGEAUTOMATIONSETTINGS,
	OBJECTID_ARRANGECHILDOPEN,
	OBJECTID_ARRANGEREGION,
	OBJECTID_ARRANGESOLO,
	OBJECTID_ARRANGEAVOLUME,
	OBJECTID_ARRANGEMVOLUME,
	OBJECTID_ARRANGEAINPUT,
	OBJECTID_ARRANGEMINPUT,
	OBJECTID_ARRANGETRACKINFO,

	OBJECTID_ARRANGECHANNEL,
	OBJECTID_ARRANGECHANNELSOLO,
	OBJECTID_ARRANGECHANNELCHANNELTYPE,
	OBJECTID_ARRANGECHANNELMUTE,
	OBJECTID_ARRANGECHANNELNAME,
	OBJECTID_ARRANGECHANNELVOLUME,
	OBJECTID_ARRANGEAUTOMATIONCHANNELSETTINGS,
	OBJECTID_ARRANGEAUTOMATIONCHANNELVISIBLE,
	OBJECTID_ARRANGECHANNELMVOLUME,
	OBJECTID_ARRANGECHANNELINFO,

	OBJECTID_ARRANGEAUTOMATIONTRACK,
	OBJECTID_ARRANGEAUTOMATIONTRACKNAME,
	OBJECTID_ARRANGEAUTOMATIONTRACKMODE,
	OBJECTID_ARRANGEAUTOMATIONTRACKVU,
	OBJECTID_ARRANGESINGLEAUTOMATIONTRACKVISIBLE,

	OBJECTID_ARRANGESUBINFO,
	OBJECTID_ARRANGESUBVALUEINFO,
	OBJECTID_ARRANGESUBMOUSEVALUEINFO,
	OBJECTID_ARRANGESUBCREATESTEP,
	OBJECTID_ARRANGEAUTOMATIONTRACKMUTE,

	OBJECTID_ARRANGETRACKNAME,
	OBJECTID_ARRANGETRACKTYPE,
	OBJECTID_ARRANGETRACKCHANNELTYPE, // Stereo,Mono...
	OBJECTID_ARRANGEAUTOMATIONTRACKVISIBLE, // see LASTARRANGETRACKID !
	// End Track

	// Pattern
	OBJECTID_ARRANGEPATTERN,

	OBJECTID_ARRANGEPATTERNLISTPATTERN,
	OBJECTID_ARRANGEPATTERNLISTPATTERNMUTE,

	OBJECTID_ARRANGEMARKER
};

#define LASTARRANGETRACKID OBJECTID_ARRANGEAUTOMATIONTRACKVISIBLE

#define EM_MOVEMARKER (EM_SPECIAL+1)
#define EM_MOVEMARKER_LEFT (EM_SPECIAL+2)
#define EM_MOVEMARKER_RIGHT (EM_SPECIAL+3)

enum{
	EDIT_SIGNATURE=EventEditor::EDITOREDITDATA_ID,
	EDIT_TRACKNAME,
	EDIT_PATTERNNAME,
	EDIT_SMOVEPATTERN,
	EDIT_SCOPYPATTERN,
	EDIT_MARKERNAME
};

class Seq_Track;
class Seq_Pattern;
class Edit_Arrange;
class guiObject;
class AutomationTrack;
class Edit_Arrange_Track;
class guiBitmap;
class AudioPattern;
class AudioHDFile;
class Seq_Signature;
class Edit_Arrange_AutomationTrack;
class InsertAudioEffect;
class Edit_Arrange_PatternList_Pattern;
class AudioRegion;
class Seq_CrossFade;
class Undo_ChangeAutomationParameter;

class Edit_Arrange_Pattern_CrossFade:public Object
{
public:
	Edit_Arrange_Pattern_CrossFade *NextCF(){return (Edit_Arrange_Pattern_CrossFade *)next;}

	Seq_CrossFade *crossfade;
	int x,x2,y,y2,headery2;
	bool startinside,endinside;
};

class Edit_Arrange_Pattern:public guiObject
{
	friend class Edit_Arrange;

public:
	Edit_Arrange_Pattern();

	void DeInit();

	bool CheckIfRegionIsInside(AudioRegion *);
	void ShowPattern();
	void ShowPatternRaster();
	void ShowPattern_Blt();
	void ShowInsidePattern();
	void CheckMouseXY(int x,int y);
	bool RefreshPositions();

	OList regions;
	Colour patterncolour;
	PatternVolumePositions patternvolumepositions; // Fade In-Volume-Fade Out
	Seq_Pattern_VolumeCurve *patterncurve;

	Edit_Arrange_Track *track;
	Edit_Arrange *editor;
	Seq_Pattern *pattern;
	char *patternname;

	OSTART start,end; //offset; // init pattern start end

	double showpeakprogress,fadeinms,fadeoutms,volume; // Peak Creation progres..

	int accesscounter,numberofevents,frame_x,frame_x2,fadeintype,fadeouttype,
		sx,sx2,sy,sy2,
		mediatype;

	bool mouseselection,
		fadeactive,fadeeditmode,
		volumeactive,
		subpattern,
		undermove,
		blink,
		focus,
		nonreal,
		muteflag,
		selectflag,
		recordstatus,
		notext,
		folder,
		automationpattern,
		sysstartup;
};

class Edit_Arrange_Record:public guiObject
{
public:
	Edit_Arrange_Record(){id=OBJECTID_ARRANGERECORD;}
	Edit_Arrange_Track *track;

	int recordmode;
	bool status;
};

class Edit_Arrange_Automation:public guiObject // Open/Close Sub
{
public:
	Edit_Arrange_Automation(){id=OBJECTID_ARRANGESHOWAUTOMATIONTRACKS;}
	Edit_Arrange_Track *track;
};

class Edit_Arrange_AutomationSettings:public guiObject // Open/Close Sub
{
public:
	Edit_Arrange_AutomationSettings(){id=OBJECTID_ARRANGEAUTOMATIONSETTINGS;}
	Edit_Arrange_Track *track;
	bool onoff;
	bool used;
};

class Edit_Arrange_Child:public guiObject // Open/Close Sub
{
public:
	Edit_Arrange_Child(){id=OBJECTID_ARRANGECHILDOPEN;}
	Edit_Arrange_Track *track;
};

class Edit_Arrange_AutomationName:public guiObject
{
public:
	Edit_Arrange_AutomationName(){id=OBJECTID_ARRANGEAUTOMATIONTRACKNAME;}
	Edit_Arrange_AutomationTrack *track;
};

class Edit_Arrange_AutomationTrackVisible:public guiObject
{
public:
	Edit_Arrange_AutomationTrackVisible(){id=OBJECTID_ARRANGESINGLEAUTOMATIONTRACKVISIBLE;}
	Edit_Arrange_AutomationTrack *track;
};

class Edit_Arrange_AutomationMode:public guiObject
{
public:
	Edit_Arrange_AutomationMode(){id=OBJECTID_ARRANGEAUTOMATIONTRACKMODE;}
	Edit_Arrange_AutomationTrack *track;
	int mode;
};

class Edit_Arrange_AutomationVU:public guiObject
{
public:
	Edit_Arrange_AutomationVU(){id=OBJECTID_ARRANGEAUTOMATIONTRACKVU;}
	Edit_Arrange_AutomationTrack *track;
};

class Edit_Arrange_AutomationInfo:public guiObject
{
public:
	Edit_Arrange_AutomationInfo(){id=OBJECTID_ARRANGESUBINFO;}
	Edit_Arrange_AutomationTrack *track;
};

class Edit_Arrange_AutomationValue:public guiObject
{
public:
	Edit_Arrange_AutomationValue(){id=OBJECTID_ARRANGESUBVALUEINFO;}
	double value;
	Edit_Arrange_AutomationTrack *track;
};

class Edit_Arrange_AutomationMouseValue:public guiObject
{
public:
	Edit_Arrange_AutomationMouseValue(){id=OBJECTID_ARRANGESUBMOUSEVALUEINFO;}
	double value;
	Edit_Arrange_AutomationTrack *track;
	bool active;
};

class Edit_Arrange_AutomationCreateTicks:public guiObject
{
public:
	Edit_Arrange_AutomationCreateTicks(){id=OBJECTID_ARRANGESUBCREATESTEP;}
	Edit_Arrange_AutomationTrack *track;
};

class Edit_Arrange_AutomationVisible:public guiObject // Open/Close Sub
{
public:
	Edit_Arrange_AutomationVisible(){id=OBJECTID_ARRANGEAUTOMATIONTRACKVISIBLE;}
	Edit_Arrange_AutomationTrack *track;
};

class Edit_Arrange_AudioInputMonitoring:public guiObject
{
public:
	Edit_Arrange_AudioInputMonitoring(){id=OBJECTID_ARRANGEINPUT;}
	Edit_Arrange_Track *track;
	bool status;
};

class Edit_Arrange_Volume:public guiObject
{
public:
	Edit_Arrange_Volume(){
		id=OBJECTID_ARRANGEAVOLUME;
		volume=0;
		insert=false;
	}

	Edit_Arrange_Track *track;
	double volume;

	bool insert;
};

class Edit_Arrange_MIDIVolume:public guiObject
{
public:
	Edit_Arrange_MIDIVolume(){
		id=OBJECTID_ARRANGEMVOLUME;
		insert=false;
	}

	Edit_Arrange_Track *track;
	int MIDIvelocity;
	bool insert,editable;
};

class Edit_Arrange_Input:public guiObject
{
public:
	Edit_Arrange_Input(){id=OBJECTID_ARRANGEAINPUT;}
	Edit_Arrange_Track *track;
};

class Edit_Arrange_MIDIInput:public guiObject
{
public:
	Edit_Arrange_MIDIInput(){id=OBJECTID_ARRANGEMINPUT;}
	Edit_Arrange_Track *track;
};

class Edit_Arrange_TrackInfo:public guiObject
{
public:
	Edit_Arrange_TrackInfo(){id=OBJECTID_ARRANGETRACKINFO;}
	Edit_Arrange_Track *track;
};

class Edit_Arrange_Mute:public guiObject
{
public:
	Edit_Arrange_Mute(){id=OBJECTID_ARRANGEMUTE;}
	Edit_Arrange_Track *track;
	bool status;
};

class Edit_Arrange_Solo:public guiObject
{
public:
	Edit_Arrange_Solo(){
		id=OBJECTID_ARRANGESOLO;
	}

	Edit_Arrange_Track *track;
};

#define ETRACKTYPE_MIDI 1
#define ETRACKTYPE_AUDIOFILE 2
#define ETRACKTYPE_AUDIOINSTRUMENT 4

class Edit_Arrange_TrackChannelType:public guiObject
{
public:
	Edit_Arrange_TrackChannelType(){id=OBJECTID_ARRANGETRACKCHANNELTYPE;}
	Edit_Arrange_Track *track;
	int channeltype;
};

class Edit_Arrange_Type:public guiObject
{
public:
	Edit_Arrange_Type(){id=OBJECTID_ARRANGETRACKTYPE;}
	Edit_Arrange_Track *track;
	int status; // Tracktype
};

class Edit_Arrange_Name:public guiObject
{
public:
	Edit_Arrange_Name(){id=OBJECTID_ARRANGETRACKNAME;}
	Edit_Arrange_Track *track;
};

#define REFRESH_EAT_PATTERN 0
#define REFRESH_EAT_PATTERNANDTRACK 1
#define REFRESH_EAT_COLOUR 2

#define SHOWFREEZE_CREATING 1
#define SHOWFREEZE_WAITING 2

class Edit_Arrange_Track:public guiObject
{
	friend class Edit_Arrange;

public:
	Edit_Arrange_Track();

	void DeInit()
	{
		if(namestring)delete namestring;
		namestring=0;
	}

	void ShowTrack(bool refreshbgcolour);
	void ShowTrackNumber();

	void Blit();
	void ShowPatternBGRaster();
	void ShowMIDIDisplay(bool force);
	void ShowAudioDisplay(bool force);
	void ShowTrackRaster();
	void Refresh(int flag);
	int GetType();

	Edit_Arrange *editor;
	Seq_Track *track;
	char *namestring;

	Edit_Arrange_AutomationSettings automationsettings;
	Edit_Arrange_Automation automation;
	Edit_Arrange_Child child;
	Edit_Arrange_Mute mute;
	Edit_Arrange_Solo solo;

	Edit_Arrange_Record record;
	Edit_Arrange_AudioInputMonitoring audioinputmonitoring;
	Edit_Arrange_Name name;
	Edit_Arrange_Type type;
	Edit_Arrange_TrackChannelType channels;
	Edit_Arrange_Volume volume;
	Edit_Arrange_MIDIVolume MIDIvolume;
	Edit_Arrange_Input input;
	Edit_Arrange_MIDIInput MIDIinput;
	Edit_Arrange_TrackInfo info;

	Colour trackcolour;
	CShowPeak outpeak,m_outpeak;

	double freezeprogress; // under frezze ?

	int nulline,showfreeze_flag,startnamex,
		dataoutputx,dataoutputx2,bgcolour,accesscounter,MIDIdisplay,audiooutdisplay,
		solostatus;

	bool blinkrecord,trackselected,datadisplay,volumeeditable,m_volumeclicked,volumeclicked,playable,
		frozen,hasinstruments,hasfx;

private:
	void ShowMute();
	void ShowAudioInputMonitoring();
	void ShowSolo();
	void ShowVolume();
	void ShowMIDIVolume();
	void ShowRecordMode();
	void ShowAutomationMode();
	void ShowAutomationSettings();

	void ShowChildTrackMode();
	void ShowName();
	void ShowType();
	void ShowChannels();
	void ShowInput();
	void ShowMIDIInput();
	void ShowTrackInfo();
};

class Edit_Arrange_PatternMute:public guiObject
{
public:
	Edit_Arrange_PatternMute(){id=OBJECTID_ARRANGEPATTERNLISTPATTERNMUTE;}
	Edit_Arrange_PatternList_Pattern *pattern;
	bool status;
};

class Edit_Arrange_Marker:public guiObject
{
	friend class Edit_Arrange;

public:
	Edit_Arrange_Marker(Seq_Marker *m){
		id=OBJECTID_ARRANGEMARKER;
		marker=m;
	};

	Seq_Marker *marker;
};

class Edit_Arrange_PatternList_Pattern:public guiObject
{
	friend class Edit_Arrange;

public:
	Edit_Arrange_PatternList_Pattern(){
		id=OBJECTID_ARRANGEPATTERNLISTPATTERN;
		mute.pattern=this;
		mute.deleteable=false; // part of
	};

	void ShowPattern();

	Seq_Pattern *pattern;
	Edit_Arrange_PatternMute mute;
	Edit_Arrange *editor;
	int posx,posx2,namex,namex2;	

private:
	void ShowMute();
	void ShowName(bool blit);
	void ShowPatternPos();
};

class Edit_Arrange_Channel;


class Edit_Arrange_ChannelMute:public guiObject
{
public:
	Edit_Arrange_ChannelMute(){id=OBJECTID_ARRANGECHANNELMUTE;}
	Edit_Arrange_Channel *channel;
	bool status;
};

class Edit_Arrange_ChannelAutomationSettings:public guiObject
{
public:
	Edit_Arrange_ChannelAutomationSettings(){id=OBJECTID_ARRANGEAUTOMATIONCHANNELSETTINGS;}
	Edit_Arrange_Channel *channel;
	bool onoff;
	bool used;
};

class Edit_Arrange_ChannelAutomation:public guiObject
{
public:
	Edit_Arrange_ChannelAutomation(){id=OBJECTID_ARRANGEAUTOMATIONCHANNELVISIBLE;}
	Edit_Arrange_Channel *channel;
};

class Edit_Arrange_ChannelVolume:public guiObject
{
public:
	Edit_Arrange_ChannelVolume(){id=OBJECTID_ARRANGECHANNELVOLUME;}
	Edit_Arrange_Channel *channel;
	double volume;
};

class Edit_Arrange_ChannelInfo:public guiObject
{
public:
	Edit_Arrange_ChannelInfo(){id=OBJECTID_ARRANGECHANNELINFO;}
	Edit_Arrange_Channel *channel;
};

class Edit_Arrange_ChannelChannelType:public guiObject
{
public:
	Edit_Arrange_ChannelChannelType(){id=OBJECTID_ARRANGECHANNELCHANNELTYPE;}
	Edit_Arrange_Channel *channel;
	int channeltype;
};

class Edit_Arrange_ChannelSolo:public guiObject
{
public:
	Edit_Arrange_ChannelSolo(){
		id=OBJECTID_ARRANGECHANNELSOLO;
		status=0; // Bitmap
	}

	Edit_Arrange_Channel *channel;
	int status;
};

class Edit_Arrange_ChannelName:public guiObject
{
public:
	Edit_Arrange_ChannelName(){id=OBJECTID_ARRANGECHANNELNAME;}
	Edit_Arrange_Channel *channel;
};

class Edit_Arrange_ChannelMIDIVolume:public guiObject
{
public:
	Edit_Arrange_ChannelMIDIVolume(){
		id=OBJECTID_ARRANGECHANNELMVOLUME;
		insert=false;
	}

	Edit_Arrange_Channel *channel;
	int MIDIvelocity;
	bool insert;
};

class Edit_Arrange_Channel:public guiObject
{
public:
	Edit_Arrange_Channel();

	void DeInit()
	{
		if(namestring)delete namestring;
		namestring=0;
	}

	void ShowChannelRaster();
	void ShowChannelChannelType();

	void ShowMute();
	void ShowSolo();
	void ShowChannel(bool refreshbgcolour);
	void ShowMIDIDisplay(bool force);
	void ShowAudioDisplay(bool force);
	void ShowVolume(bool force);
	void ShowMIDIVolume();
	void ShowAutomationSettings();
	void ShowAutomationMode();
	void ShowChannelNumber();
	void ShowChannelInfo();

	CShowPeak outpeak,m_outpeak;
	Edit_Arrange_ChannelMute mute;
	Edit_Arrange_ChannelSolo solo;
	Edit_Arrange_ChannelName name;
	Edit_Arrange_ChannelVolume volume;
	Edit_Arrange_ChannelInfo info;

	Edit_Arrange_ChannelAutomationSettings automationsettings;
	Edit_Arrange_ChannelAutomation automation;

	Edit_Arrange_ChannelMIDIVolume MIDIvolume;
	Edit_Arrange_ChannelChannelType channeltype;

	Edit_Arrange *editor;
	AudioChannel *channel;
	char *namestring;
	int startnamex,dataoutputx,dataoutputx2,bgcolour;
	bool datadisplay,volumeeditable,volumeclicked,m_volumeclicked,hasinstruments,hasfx,channelselected;
};

class AutomationObject;

class Edit_Arrange_AutomationTrackParameter:public Object
{
public:
	Edit_Arrange_AutomationTrackParameter *NextAutomationTrackObject(){return (Edit_Arrange_AutomationTrackParameter *)next;}
	bool CheckObjectinRange(int cx,int cy,int cx2,int cy2);
	bool IsUnderMouse(int x,int y);
	AutomationObject *object;
	AutomationParameter *parameter;
	Edit_Arrange_AutomationTrack *eat;
	int x,y,x2,y2;
};

class Edit_Arrange_AutomationPattern:public Object
{
public:
	Seq_Pattern_VolumeCurve *patterncurve;
	Seq_Pattern *pattern;

	double fadeinms,fadeoutms,volume;
	int fadeintype,fadeouttype;
	bool fadeactive,volumeactive;
};

class Edit_Arrange_AutomationTrack:public guiObject
{
public:
	friend Edit_Arrange;

	Edit_Arrange_AutomationTrack();

	void ShowTrack(bool refreshbgcolour);
	void ShowTrackRaster();
	void ShowValue();
	void ShowVU();
	void ShowChannelRaster();
	void ShowVisible();
	void ShowMode();

	void DrawPattern(bool withbackgroundclear,bool withmarker,bool withscaleandparameters);
	void ShowPatternBGRaster();
	void DrawPatternScale(bool withtimeline);
	void DrawPatternParameters();
	void ShowPattern();
	void RefreshRealtime();

	void DeleteObjects()
	{
		parameter.DeleteAllO();pattern.DeleteAllO();
	}

	Edit_Arrange_AutomationTrackParameter *FirstTrackParameter(){return (Edit_Arrange_AutomationTrackParameter *)parameter.GetRoot();}
	void AddTrackParameter(Edit_Arrange_AutomationTrackParameter *ob){parameter.AddEndO(ob);}
	void MouseClick(int px,int py,bool leftmouse);
	AutomationParameter *FindParameterMouse(int px,int py);
	void CheckParameterMouse(AutomationParameter *);
	void InitAutomationMouseMove();

	Edit_Arrange_AutomationName name;
	Edit_Arrange_AutomationMode mode; // Off,Read,Touch,Latch,Write
	Edit_Arrange_AutomationValue value;
	Edit_Arrange_AutomationVU vu;
	Edit_Arrange_AutomationTrackVisible visible;

	Edit_Arrange_AutomationInfo info;
	Edit_Arrange_AutomationMouseValue mousevalue;
	Edit_Arrange_AutomationCreateTicks createticks;

	Edit_Arrange *editor;
	AutomationTrack *automationtrack;
	double parmvalue;
	int startnamex,dataoutputx,dataoutputx2,accesscount,fgcolour;
	bool datadisplay,selected;

private:
	OList pattern, // Edit_Arrange_AutomationPattern
		parameter; // Edit_Arrange_AutomationTrackParameters
};

class Edit_Arrange:public EventEditor
{
	friend class Edit_ArrangeFX;
	friend class Edit_Arrange_Track;
	friend class Edit_Arrange_AutomationTrack;
	friend class Edit_Arrange_Pattern;
	friend class EditFunctions;
	friend class Edit_Arrange_PatternList;
	friend class Edit_Arrange_PatternList_Pattern;
	friend class Edit_ArrangeList;

public:

	enum{
		ED_PATTERN,
		ED_TRACKS,
	};

	enum{
		REFRESH_OVERVIEW=99
	};

	Edit_Arrange();

	void InitNewTimeType();
	void SelectAll(bool on); //v
	void MuteSelected(bool mute);
	void Delete(); //v

	void RefreshMIDI(Seq_Track *);

	void DragDrop(guiGadget *);
	void DragDropFile(char *file,guiGadget *);

	bool ZoomGFX(int zoomy,bool horiz=false);
	void ChangeTrackIcon(Seq_Track *,TrackIcon *);
	bool EditCancel();
	void ResetMoreEventFlags();

	char *GetToolTipString1(); //v
	char *GetToolTipString2(); //v
	void ShowFocusPattern();

	void ShowShowStatus();

	Seq_SelectionList *GetPatternSelection();

	// Freeze
	void FreezeTrack(Seq_Track *,bool freeze);

	void Goto(int to);
	void ScrollTo(Seq_Track *);

	void ShowMarker(int y,int y2);

	void RefreshAutomationObject(AutomationObject *);
	Edit_Arrange_AutomationTrack *FindAutomationTrack(AutomationTrack *);
	Edit_Arrange_AutomationTrack *FindAutomationTrackAtY(int y);
	void ShowAutomationTracks();
	void CheckAutomationParameterUnderMouse();

	Editor *OpenDoubleClickEditor(Seq_Pattern *,OSTART time);
	Edit_Arrange_Track *FindTrack(Seq_Track *);

	Edit_Arrange_Pattern *FindPattern(Seq_Pattern *);
	Edit_Arrange_Pattern *FindAudioHDFile(AudioHDFile *);
	Edit_Arrange_Pattern *FindAudioRegion(AudioRegion *);

	void CutPattern_Arrange(Seq_Pattern *);

	void NewZoom(); // v
	void NewDataZoom();
	void RefreshStartPosition(); // v
	void NewYPosition(double y,bool draw);
	void KeyUp(); // v
	void KeyDown(); // v
	void KeyDownRepeat();

	bool SizePattern(bool test);
	void MoveAutomationParameters();
	void ShowPatternAndAutomation();

	void RefreshTrack(Seq_Track *);
	void AddEditorMenu(guiMenu *);
	guiMenu *CreateMenu(); // Tracks
	guiMenu *CreateMenu2(); // Pattern
	void CreateGotoMenu();

	void AddPopFileMenu(Seq_Track *);
	void Paste();
	void FreeEditorMemory();
	void DeInitWindow();
	void CalcPatternPosition(Edit_Arrange_Pattern *);
	void Init();
	
	void MouseWheel(int delta,guiGadget *);

	void MouseMoveInPattern(bool leftmouse);
	void MouseReleaseInPattern(bool leftmouse);

	void InitMouseEditRange();
	void ShowVSlider();

	void RefreshObjects(LONGLONG otype,bool editcall);
	EditData *EditDataMessage(EditData *);

	void SetAutoMode(int);
	void RefreshAllArrangeWithSameSet();
	void ClearFlag(int);
	void AddFlag(int);

	void Gadget(guiGadget *);
	void SetSet(int s);

	bool SelectPattern(Seq_Pattern *,bool select,bool draw,bool toggleselection=false);
	bool SelectPatternCheckKeys(Seq_Pattern *,bool select,bool draw,bool toggleselection=false);

	void SetPatternColour(Seq_Pattern *);
	void ToggleUseColour(Seq_Pattern *);

	bool CheckAuto();
	void RefreshRealtime();
	void RefreshRealtime_Slow();

	void ShowSoloMute(bool force);
	void RefreshAudio(AudioHDFile *);
	void RefreshPattern(Seq_Pattern *);
	void RefreshAudioRegion(AudioRegion *);

	Seq_Track *FindTrackAtY(int ypos);

	Seq_Track *FirstTrack();
	Seq_Track *LastTrack();

	void ShowAllPattern();
	void ShowPattern();
	void ShowPatternCrossFades();

	void PlayAudioPattern(AudioPattern *,LONGLONG startpostion);
	
	void UserMessage(int msg,void *par);
	void SetMouseMode(int nmode,AutomationTrack *atrack=0);

	void AddStartY(int addy);
	void AutoScroll();
	void SetStartTrack(Seq_Track *);
	void SetStartTrack(int index);
	void ExportSelectedPattern(bool split);
	void ExportSelectedPattern(AudioPattern *,char *file,bool split);
	void AddPatternMenu(Seq_Pattern *);
	void AddActiveTrackMenu(Seq_Track *);
	void CreateLinkMenu(guiMenu *,Seq_Pattern *);
	void CreateLinks();
	void RemoveLinks(Seq_Pattern *,bool refreshgui);
	void SelectLinks(Seq_Pattern *);

	void ShowHoriz(bool showtracks,bool showheader,bool showoverview);

	void SelectAllPattern (bool on,Seq_Pattern *not,Seq_Track *track=0);
	void SelectAllAutomationParameters(bool on);
	void DeselectAllAutomationParameters(AutomationTrack *not);

	void ShowOverview();
	void ShowOverviewVertPosition(int *y,int *y2);
	void InitPattern(Edit_Arrange_Track *,Seq_Track *,bool folder);

	void ShowHideAutomationTracks(bool show);

	int ShowTextMap();
	int ShowMarkerMap();
	int ShowTempoMap();
	int ShowSignatureMap();

	Edit_Arrange_Pattern_CrossFade *FirstCrossFade(){return (Edit_Arrange_Pattern_CrossFade *)crossfades.GetRoot();}
	void RemoveCrossFades(Seq_Pattern *);

	int getcountselectedtracks,getcounttracks,getcountselectedpattern,getcountpattern;

	void ShowMenu();
	void MouseClickInTracks(bool leftmouse);
	void MouseReleaseInTracks(bool leftmouse);
	void DeltaInTracks();
	void MouseMoveInTracks();

	void MouseDoubleClickInTracks(bool leftmouse);
	void MouseDoubleClickInPattern(bool leftmouse);

	void UnSelectAllInPattern(AutomationTrack *,AutomationParameter *);
	void MouseClickInPattern(bool leftmouse);
	void MouseClickInSignature(bool leftmouse);
	void MouseClickInText(bool leftmouse);
	void MouseClickInMarker(bool leftmouse);

	void BuildTrackList(OListCoosY *,guiGadget_Tab *,int zoom,bool withautomationtracks,bool autozoom=true); // Tracks+Automation Tracks
	void BuildAddAutomationTracks(OListCoosY *,TrackHead *,double h,bool withautomationtracks);

	int ShowTracks();
	int GetShowFlag();

	Seq_SelectionList *CreateSelectionListAll(bool all);
	Seq_SelectionList *CreateSelectionList(Seq_Pattern *addpattern=0);

	void CopySelectedPattern();
	void CutSelectedPattern();
	void MoveSelectedPatternToMeasure(bool copy);
	void MovePatternCycle(bool right,bool selectedpattern,bool selectedtracks);
	void MoveSelectedPatternToPosition(OSTART to);
	void MoveSelectedPatternToMeasureToTick(OSTART tick,bool copy,Seq_Track *starttrack=0);
	void ShowSignature(Seq_Signature *);
	void CreateArrangeAutoMenu(Edit_Arrange_AutomationTrack *);

	Seq_SelectionList selection;
	OListCoosY trackobjects,overviewtrackobjects,realtimecheck;
	guiGadget_Time *g_info_pos,*g_info_end;

	guiGadget *g_mute,*g_solo,*g_info_name,*g_createnewtrack,*g_createnewchildtrack,
		*g_filtertracks,*g_audio,*g_MIDI,*g_type,*g_input,*g_automation,*g_volumecurves,
		*g_set[5],*g_movebutton,
		*eventeditor,*pianoeditor,*drumeditor,*sampleeditor,
		*g_tmaster,*g_tbus,*g_tmetro;

	guiGadget_Tab *tracks;
	guiGadget_CW *pattern,*tempo,*signature;
	guiMenu *menu_showcontrol,*menu_shownotes,*menu_showpatternlist,*menu_showsongspatternlist,*menu_showtextmap,*menu_showmarkermap,*menu_shownotesmenu,
		*menu_shownotesmenu_off,*menu_shownotesmenu_lines,*menu_shownotesmenu_notes;

	Edit_ArrangeList *listeditor;

	Seq_Pattern *fadeeditpattern;
	OSTART fadeeditstartposition;
	int fademousey,fadeeditdelta;

	OSTART overviewrangestart,overviewrangeend;
	int shownotesinarrageeditor,track_soloed,track_muted;

	bool showlist,showeffects,showcontrols,showtracknumber,showpatternlist,
		showtextmap,showmarkermap,trackonoff,patternonoff;

private:
	void SetFadePattern(Seq_Pattern *);
	void EditFades();
	void CancelEditFades();

	void ShowMovePatternSprites(bool force=false);
	void ShowSizePatternSprites();

	void EditMarker(Seq_Marker *,int x,int y);
	void MarkerMenu(Edit_Arrange_Marker *,bool leftmouse);
	void FreeEditorPatternMemory();
	void GotoSoloTrack();
	void FreePatternRegionMemory();
	void RefreshAudioPeak();
	void AddEffectsToMenu(AutomationTrack *,InsertAudioEffect *);
	void AddCreatePatternMenu(Seq_Track *);
	void AddPasteMenu(Seq_Track *);
	void AddCopyPastePatternMenu(Seq_Pattern *);
	void AddOffsetMenu(Seq_Pattern *);
	void AddSetSongPositionMenu(Seq_Pattern *,Seq_Track *);
	void AddMoveCopySelectedPatternMenu(OSTART,Seq_Track *,Seq_Pattern *);

	bool CheckCrossFadeClick();
	bool CheckPopMenu(guiObject *,int flag=0);
	void AddPatternAutomationMenu(Seq_Pattern *,guiMenu *);
	bool CreatePopMenuCrossFade(Seq_CrossFade *);
	void DoubleClickInEmptyTrack(Seq_Track *);

	void ShowFilter();
	void ShowTracking();
	void RefreshTracking();

	OList crossfades;

	Seq_Pattern *addpattern,*sizepattern;
	char  *focustrackwindowname_string,*activepatternwindowname_string,*focustrackwindowname;
	AutomationTrack *moveautomationtrack,*modestartautomationtrack;
	Seq_Marker *movemarker;
	Seq_Track *modestarttrack,*mouseselectionstarttrack,*mouseselectionendtrack;

	Undo_ChangeAutomationParameter *automationtrackeditinit;

	OSTART movemarkerstartposition,movemarkerendposition,moveautomationstartposition; 
	int moveautomationstartx,moveautomationstarty,blinkrecordcounter,set,trackingindex;
	bool blinkrecordcounter_toggle,showmaster,showmetro,showbus;
};

#endif