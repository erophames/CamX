#ifndef CAMX_TRACKHEADOBJECTS_H
#define CAMX_TRACKHEADOBJECTS_H 1

#include "object.h"
#include "audioiofx.h"
#include "audiohardwarebuffer.h"
#include "MIDIeffects.h"

#define MAXRECPATTERNPERTRACK 2

class AutomationTrack;
class AutomationObject;
class AudioObject;
class Seq_Track;
class AudioChannel;
class MasteringCall;

enum{
	SOLO_OFF,
	SOLO_THIS,
	SOLO_OTHER,
};

class MIDIIOFX
{
public:
	void Load(camxFile *);
	void Save(camxFile *);

	int GetVelocity(){return velocity.velocity;}
	void SetVelocity(int v,OSTART automationtime);

	AT_MIDISYS_Velocity velocity;
	AT_MIDISYS_MVolume mvolume;

	TrackHead *trackhead;
};

class TrackHead
{
public:
	enum{
		TRACKTYPE_DATA,
		TRACKTYPE_METRONOM,
		TRACKTYPE_BUS,
		TRACKTYPE_MASTERCHANNEL
	};

	TrackHead();

	AutomationTrack *GetActiveSubTrack(){return activeautomationtrack;}
	AutomationTrack *FirstAutomationTrack(){return (AutomationTrack *)automationtracks.GetRoot();}
	AutomationTrack *LastAutomationTrack(){return (AutomationTrack *)automationtracks.Getc_end();}
	int GetCountAutomationTracks(){return automationtracks.GetCount();}
	void ResetAutomationTracksCycle(OSTART cycleposition,int index);
	bool CheckAutomate(AutomationObject *,OSTART atime,int pindex,double value,int iflag);
	bool AutomateBeginEdit(AutomationObject *,OSTART atime,int pindex);
	void AutomateEndEdit(AutomationObject *,OSTART atime,int pindex);
	int GetCountSelectedAutomationParameters();
	bool CanAutomationObjectBeChanged(AutomationObject *,int iflag,int pindex);

	void AddAutomationTrack(AutomationObject *bobject,int bindex,AutomationTrack *,AutomationTrack *prev,int flag,Seq_Track *,AudioChannel *);
	AutomationTrack *FindMIDIAutomationTrack(int status,int value);
	AutomationTrack *FindPluginAutomationTrack(AudioObject *,int index);
	AutomationTrack *FindSysAutomationTrack(int type);
	AutomationTrack *FindPluginControlAutomationTrack(AudioObject *,int type);
	AutomationTrack *FindMIDISysAutomationTrack(int type);

	bool SetAutomationTracksTouchLatch(AutomationObject *ao,int pindex,OSTART at,bool reset=false);

	AutomationTrack *DeleteAutomationTrack(AutomationTrack *,bool full);
	void DeleteAllAutomationTracks();
	void SetActiveAutomationTrack(AutomationTrack *);
	void ShowAutomationTracks(bool onoff);
	void InitDefaultAutomationTracks(Seq_Track *,AudioChannel *);
	void InitAutomationTracks(InitPlay *);
	AutomationTrack *FirstAudioAutomationTrack();
	void ShowHideAutomationTracks(bool show);
	void ResetAutomationTracks();
	void RepairAutomationTracks(); // After Loading
	void DeleteDeletedAutomationParameter();
	bool HasAutomation();

	void SendAudioAutomation(OSTART position,OSTART endposition=-1); // Endposition send by Audio
	void SendMIDIAutomation(OSTART position);

	void DeselectAllAutomationParameters(AutomationTrack *not);
	bool IsAutomationParameterSelected();
	AutomationTrack *FirstAutomationTrackWithSelectedParameters();

	int GetCountOfAutomationTracksUsing(AutomationObject *);
	AutomationTrack **FillAutomationListUsing(AutomationTrack **list,AutomationObject *);
	void UserEditAutomation(AutomationObject *,int parindex);

	void CheckPlugins();
	guiMenu *CreateChannelTypeMenu(guiMenu *);
	virtual bool SetVType(AudioDevice *,int type,bool guirefresh,bool allselected)=0;

	void LoadAutomationTracks(camxFile *,Seq_Track *,AudioChannel *);
	void SaveAutomationTracks(camxFile *);

	Peak *GetPeak(){return &mix.peak;}
	void CloseMastering(bool freememory,bool canceled);

	void RefreshDo();
	void PreRefreshDo();

	void MIDIVolumeDown();
	void MIDIVolumeUp();

	void VolumeDown();
	void VolumeUp();

	AudioIOFX io;
	MIDIIOFX MIDIfx;
	AudioHardwareBuffer mix;// mixed RAW Song Data + Bridge
	OList automationtracks;

	AutomationTrack *activeautomationtrack;
	Seq_Song *song;
	Seq_Track *track;
	AudioChannel *channel;
	MasteringCall *mastering,*freezing;

	double sizefactor;
	int tracktype;
	bool showautomationstracks,automationon,volumeclicked,m_volumeclicked;
};

#endif
