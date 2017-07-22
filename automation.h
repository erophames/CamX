#ifndef CAMX_AUTOMATION_H
#define CAMX_AUTOMATION_H 1

#include "defines.h"
#include "object.h"
#include "objectids.h"
#include "objectevent.h"

enum // Plugin IDS
{
	PSYS_BYPASS
};

enum {

	SYS_MUTE,// -- Audio
	SYS_SOLO,// -- Audio
	SYS_VOLUME,// -- Audio
	SYS_PAN,// -- Audio

	SYS_MIDIVOLUME,// -- MIDI
	SYS_MIDIMAINVOLUME
};

enum {
	CT_ABSOLUT,
	CT_LINEAR
};

enum{
	ATMODE_OFF,
	ATMODE_READ,
	ATMODE_TOUCH,
	ATMODE_LATCH,
	ATMODE_WRITE
};

enum AUTOIDS
{
	// AUDIO
	AID_FIRSTAUDIO,

	AID_AUDIOVOLUME,
	AID_AUDIOPAN,

	AID_LASTAUDIO,

	// MIDI
	AID_FIRSTMIDI=500,

	AID_MIDIVELOCITY,
	AID_MIDITRANSPOSE,
	AID_MIDICHANNEL,
	AID_MIDIMVOLUME,

	AID_LASTMIDI,
};

#define MAXPLUGINAUTOMATIONPARAMETER 128

class MIDIOutputDevice;
class Seq_Event;
class InitPlay;
class MIDIOutputDevice;
class Seq_Pattern;
class Seq_Track;
class AudioChannel;
class AudioObject;
class guiBitmap;
class AutomationObject;
class AutomationParameter;
class InsertAudioEffect;

extern char *automationtrack_mode_names[];

enum{
	AEF_FORCE=1,
	AEF_USEREDIT=2,
	AEF_UNDO=4,
	AEF_FROMOBJECT=8,
	AEF_PLUGINCONTROL=16
};

#define APINIT_SETCURVE 1

class AutomationTrack:public Object
{
	friend class Seq_Song;
	friend class Seq_Main;
	friend class EditFunctions;
	friend class TrackHead;
	friend class Undo;

public:
	enum{
		SBTYPE_ABS=0,
		SBTYPE_LINEAR=1
	};

	enum{
		SB_FLAG_MUTE=1,
		SB_FLAG_SELECTED=2
	};

	AutomationTrack();
	~AutomationTrack()
	{
		FreeMemory();
	}

	void FreeMemory();

	virtual ARES DoScale(ARES value){return value;}
	virtual bool ObjectCheck(Object *){return true;}
	virtual int DrawScale(guiBitmap *,int x,int y,int x2,int y2){return y+((y2-y)/2);}
	virtual bool CompareWithSolo(){return false;}
	virtual bool CompareWithMute(){return false;}

	// MIDI
	virtual Seq_Event *CreateRealMIDIEvent(AutomationObject *){return 0;}

	char *GetAutomationTrackName();
	void SetLinear(LONGLONG start);

	void Automate(OSTART atime,double value,int iflag);

	void MoveOrCopySelectedAutomationParameters(OSTART diff,bool copy);
	void ResetAutomationParameter(AutomationParameter *);

	AutomationTrack *PrevAutomationTrack(){return (AutomationTrack *)prev;}
	AutomationTrack *NextAutomationTrack(){return (AutomationTrack *)next;}
	AutomationTrack *NextAudioAutomationTrack();

	void InitAutomationTrackPlayback(InitPlay *,bool send);
	void Refresh();

	void Load(camxFile *);
	void Save(camxFile *);
	Seq_Song *GetSong();

	// Parameter
	AutomationParameter *FirstAutomationParameter();
	AutomationParameter *FindAutomationParameterBefore(OSTART pos);
	AutomationParameter *FindAutomationParameter(OSTART pos);

	void CreateAndAddParameter(OSTART,double value,Seq_Song *,int iflag);
	void CloneParameter(OListStart *);

	int GetAccessCounter(){return parameter.accesscounter;}
	int GetCountOfAutomationParameter(){return parameter.GetCount();}
	int GetCountOfSelectedAutomationParameter(){return parameter.GetCountSelectedObjects();}

	void MoveAll(OSTART ticks){parameter.MoveAll(ticks);}
	void AddParameter(AutomationParameter *,int iflag);
	void AddParameter(AutomationParameter *,OSTART,int iflag);
	void SortParameter(AutomationParameter *);
	void DeleteParameter(AutomationParameter *,bool full);
	void DeleteTouchLatchParameter(OSTART endtime,bool deletetrue=false);
	void SetAutomationMode(int m);

	void CreateStartParameter000(); // 0
	void CreateStartParameter001();// 1.0
	void CreateStartParameter0005();// 0.5

	bool CanAutomation();
	bool CanPlayAutomation();

	void InitPlayback(OSTART);
	double GetValueAtPosition(OSTART);
	double GetValueAtSamplePosition(Seq_Song *,LONGLONG);

	void Cut();
	void InsertOldIndex();

	void SelectAllParameter(){parameter.SelectAll();}
	void DeSelectAllParameter(){parameter.DeSelectAll();}

	OListStartIndex parameter; // AutomationParameter

	AutomationObject *bindtoautomationobject; // Connection To Automation Object
	int bindtoautomationobject_parindex;
	int curvetype;

	Seq_Track *track;
	AudioChannel *audiochannel;
	OSTART automationstarttime,automationendtime;

	LONGLONG activeobject_samplestart,playbackobject_samplestart;
	int tracktype,automode,old_index;

	// AUDIO
	UBYTE status,byte1,byte2,byte3;
	bool visible,touchlatch;
};

/*
Off
Off disables the current track automation data without deleting it. No automation data
is written, read, or played back. If the current automation mode is Off, any edits to track
automation data in the Arrange area automatically switch the automation mode to Read.
This ensures that the data, as currently edited, will be played.
Given that track automation can be recorded during playback mode, Off is the default
setting, as any mix automation recording may prove disconcerting while arranging.

Read
Read mode automates the current track, using the existing automation data.
The data cannot be changed by moving the channel strip controls, or using an external
automation controller, when in Read mode.

Touch
Touch mode plays back automation data in the same fashion as Read mode.
If a channel strip or an external (touch-sensitive) automation controller is touched, the
existing track automation data of the active parameter is replaced by any controller
movements—for as long as the fader or knob is touched. When you release the controller,
the automation parameter returns to its original (recorded) value. The time required by
a parameter to return to its previously recorded setting is set via Logic Pro > Preferences
> Automation > Ramp Time.
Touch is the most useful mode for creating a mix, and is directly comparable to “riding
the faders” on a hardware mixing console. It allows you to correct and improve the mix
at any time, when automation is active.

Latch
Latch mode basically works like Touch mode, but the current value replaces any existing
automation data after releasing the fader or knob, when Logic Pro is in playback (or
record) mode.
To finish, or to end parameter editing, stop playback (or recording).

Write
In Write mode, existing track automation data is erased as the playhead passes it.
If you move any of the Mixer’s (or an external unit’s) controls, this movement is recorded;
if you don’t, existing data is simply deleted as the playhead passes it.
*/

/*
Aus

Der Modus "Aus" deaktiviert die aktuellen Spurautomationsdaten, ohne diese zu löschen. Es werden dann keine Automationsdaten geschrieben, gelesen oder wiedergegeben. Ist der Automationsmodus "Aus" eingestellt, wird für alle Änderungen an Spurautomationsdaten im Arrangierbereich automatisch in den Automationsmodus "Read" gewechselt. So wird sichergestellt, dass die Daten mit den aktuellen Änderungen wiedergegeben werden.

Da die Spurautomation auch im Wiedergabe-Modus aufgezeichnet werden kann, ist standardmäßig "Aus" eingestellt, da die Automationsaufzeichnung beim Arrangieren im Mix verwirrend sein kann.
Read

Der Read-Modus automatisiert die ausgewählte Spur mithilfe der bestehenden Automationsdaten.

Wenn "Read" eingestellt ist, können die Daten nicht durch Bewegen der Bedienelemente im Channel-Strip oder mithilfe eines externen Automation-Controllers verändert werden.
Touch

Der Touch-Modus gibt Automationsdaten in derselben Weise wieder wie der Read-Modus.

Wenn ein Bedienelement eines Kanalzugs oder ein externer (touch-sensitiver) Automation-Controller bewegt wird, werden die bestehenden Spurautomationsdaten des aktivierten Parameters durch die entsprechenden Controller-Bewegungen ersetzt, und zwar so lange wie der Fader oder Regler bewegt wird. Wenn Sie den Controller loslassen, geht der Automationsparameter wieder auf seinen bestehenden (aufgezeichneten) Wert zurück. Die Zeit, mit der sich der Wert wieder den zuvor aufgezeichneten Daten für den Parameter anpasst, stellen Sie ein unter "Logic Pro" > "Einstellungen" > "Automation" > "Rampenzeit".

Touch ist der beste Modus für das Erstellen einer Mischung und ist vergleichbar mit der Fader-Automation auf einem Hardware-Mischpult. Sie können so die Mischung bei aktivierter Automation jederzeit korrigieren und optimieren.
Latch

Der Latch-Modus funktioniert im Grunde wie der Touch-Modus. Im Wiedergabe- und Aufnahme-Modus ist es jedoch so, dass der letzte Wert nach Loslassen des Faders oder Reglers alle bestehenden Automationsdaten ersetzt, bis die Wiedergabe in Logic Pro gestoppt wird.

Um die Aufzeichnung oder Bearbeitung der Parameter zu beenden, muss die Wiedergabe (oder Aufnahme) gestoppt werden.
Write

Im Write-Modus werden die bestehenden Spurautomationsdaten mit dem Fortschreiten der Positionslinie entsprechend überschrieben.

Wenn Sie ein Bedienelement im Mixer (oder an einem externen Gerät) bewegen, wird diese Bewegung aufgezeichnet. Werden keine Bedienelemente bewegt, werden die bestehenden Daten einfach mit dem Fortschreiten der Positionslinie gelöscht. 
*/

enum{
	ID_STATICOBJECT,

	ID_MIDICONTROL,
	ID_MIDIPITCHBEND,
	ID_MIDIPOLYPRESSURE,
	ID_MIDICHANNELPRESSURE,
	ID_MIDIPROGRAM,

	ID_AUDIOPLUGIN=100,
	ID_AUDIOPLUGINBYPASS,

	ID_SYSPLUGIN=200,

	ID_SOLO,
	ID_MUTE,
};

class EditAutomationParameter
{
public:
	EditAutomationParameter(){automationeditparameter_counter=checkcounter=0;}
	OSTART time[MAXPLUGINAUTOMATIONPARAMETER];
	double value[MAXPLUGINAUTOMATIONPARAMETER];
	int index[MAXPLUGINAUTOMATIONPARAMETER],automationeditparameter_counter,checkcounter;
};

class AutomationObject:public Object
{
public:
	AutomationObject();

	virtual double ConvertValueToAutomationSteps(double v){return v;}
	virtual AutomationObject *GetAutoObject(){return this;}
	virtual AutomationObject *GetContainerAutoObject(){return GetAutoObject();}

	virtual double GetParm(int index){return value;} //default index=0, 1 parameter
	virtual bool SetParm(int index,double par){value=par;return true;} //default index=0, 1 parameter
	virtual char *GetParmName(int index){return 0;} // Volume 
	virtual char *GetParmValueString(int index){return "";} // -10
	virtual char *GetParmTypeValueString(int index){return "";} // dB
	virtual char *GetParmValueStringPar(int index,double par){return 0;}

	virtual bool IsAudio(){return false;}
	virtual bool IsMIDI(){return false;}
	virtual bool IsSystem(){return false;}

	virtual bool CompareWithMIDI(int status,int b1,int b2){return false;}
	virtual bool CompareWithPlugin(AudioObject *,int index){return false;}
	virtual bool CompareWithPluginCtrl(AudioObject *,int type){return false;}

	virtual void CreateAutomationStartParameters(AutomationTrack *){}
	virtual void LoadSpecialData(camxFile *){}
	virtual void SaveSpecialData(camxFile *){}
	virtual void ResetAutomationParameter(AutomationParameter *,int index);

	double GetValue(){return value;}
	void SetValue(AutomationTrack *,double v,bool force=false);
	virtual void SendNewValue(AutomationTrack *){}
	virtual void SetNewInsertAudioEffect(InsertAudioEffect *){}
	virtual void ConvertValueToIntern(){}

	bool AutomationEdit(Seq_Song *,OSTART time,int index, double par,int iflag=0);
	void Load(camxFile *);
	void LoadWithOutID(camxFile *);
	void Save(camxFile *);

	bool BeginEdit(Seq_Song *song,int index);
	void EndEdit(Seq_Song *song,int index);

	EditAutomationParameter automationeditparameter;
	AutomationTrack *automationtrack;
	double scalefactor; // default 0.5
	double value; // 1<>-1
	int autotype,curvetype,automationobjectid,sysid;
	bool staticobject,hasspecialdata;
};

class MIDIAutomationObject:public AutomationObject
{
public:
	bool IsMIDI(){return true;}
};

class AudioAutomationObject:public AutomationObject
{
public:
	AudioAutomationObject(){curvetype=CT_LINEAR;}
	bool IsAudio(){return true;}
};

class SysAutomationObject:public AutomationObject
{
public:
	bool IsSystem(){return true;}
};

class AT_MIDI_Control:public MIDIAutomationObject
{
public:
	AT_MIDI_Control();
	AT_MIDI_Control(int c);

	bool CompareWithMIDI(int status,int b1,int b2)
	{
		if((status&0xF0)==CONTROLCHANGE && b1==controller)
			return true;

		return false;
	}

	char *GetParmName(int index);
	void LoadSpecialData(camxFile *);
	void SaveSpecialData(camxFile *);
	void CreateAutomationStartParameters(AutomationTrack *);
	void SendNewValue(AutomationTrack *);
	void ConvertValueToIntern();

	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);

	ControlChange ctrlevent,lastsendevent;
	int controller,MIDIvalue;
	char valuestring[40];
};

class AT_MIDI_Channelpressure:public MIDIAutomationObject
{
public:
	AT_MIDI_Channelpressure();
	bool CompareWithMIDI(int status,int b1,int b2);
	char *GetParmName(int index);
	void LoadSpecialData(camxFile *);
	void SaveSpecialData(camxFile *);
	void CreateAutomationStartParameters(AutomationTrack *);
	void SendNewValue(AutomationTrack *);
	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);
	void ConvertValueToIntern();

	ChannelPressure channelevent,lastsendevent;
	int pressure;
	char valuestring[40];
};

class AT_MIDI_Polypressure:public MIDIAutomationObject
{
public:
	AT_MIDI_Polypressure(int key);
	bool CompareWithMIDI(int status,int b1,int b2);
	char *GetParmName(int index);
	void LoadSpecialData(camxFile *);
	void SaveSpecialData(camxFile *);
	void CreateAutomationStartParameters(AutomationTrack *);
	void SendNewValue(AutomationTrack *);
	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);
	void ConvertValueToIntern();

	PolyPressure polyevent,lastsendevent;
	int pressure,pkey;
	char valuestring[80];
};

class AT_MIDI_Program:public MIDIAutomationObject
{
public:
	AT_MIDI_Program();
	bool CompareWithMIDI(int status,int b1,int b2);
	char *GetParmName(int index);
	void LoadSpecialData(camxFile *);
	void SaveSpecialData(camxFile *);
	void CreateAutomationStartParameters(AutomationTrack *);
	void SendNewValue(AutomationTrack *);
	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);
	void ConvertValueToIntern();

	ProgramChange programevent,lastsendevent;
	int program;
	char valuestring[40];
};

class AT_MIDI_Pitchbend:public MIDIAutomationObject
{
public:
	AT_MIDI_Pitchbend();
	bool CompareWithMIDI(int status,int b1,int b2);
	char *GetParmName(int index);
	void LoadSpecialData(camxFile *);
	void SaveSpecialData(camxFile *);
	void CreateAutomationStartParameters(AutomationTrack *);
	void SendNewValue(AutomationTrack *);
	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);
	void ConvertValueToIntern();

	Pitchbend pitchevent,lastsendevent;
	int pitchbend;
	char valuestring[40];
};

class AT_AUDIO_Plugin:public AudioAutomationObject
{
public:
	AT_AUDIO_Plugin(InsertAudioEffect *,int i);
	AutomationObject *GetAutoObject();
	char *GetParmName(int index);
	void LoadSpecialData(camxFile *);
	void SaveSpecialData(camxFile *);
	void CreateAutomationStartParameters(AutomationTrack *);
	void SendNewValue(AutomationTrack *);
	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);
	bool CompareWithPlugin(AudioObject *,int ix);
	void SetNewInsertAudioEffect(InsertAudioEffect *);

	InsertAudioEffect *insertaudiofx;
	int index;
	char valuestring[64];
};

class AT_AUDIO_PluginByPass:public AudioAutomationObject
{
public:
	AT_AUDIO_PluginByPass(InsertAudioEffect *iae)
	{
		automationobjectid=ID_AUDIOPLUGINBYPASS;
		staticobject=false;
		hasspecialdata=true;
		curvetype=CT_ABSOLUT;
		insertaudiofx=iae;
		value=0;
	}

	void CreateAutomationStartParameters(AutomationTrack *);
	double ConvertValueToAutomationSteps(double v);
	char *GetParmName(int index);
	void LoadSpecialData(camxFile *);
	void SaveSpecialData(camxFile *);
	void SendNewValue(AutomationTrack *);
	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);
	void SetNewInsertAudioEffect(InsertAudioEffect *);
	bool CompareWithPluginCtrl(AudioObject *,int type);
	AutomationObject *GetContainerAutoObject();

	InsertAudioEffect *insertaudiofx;
	char valuestring[64];
};

class AutomationParameter:public OStart
{
public:
	enum{
		AP_DONTDELETE=1<<OFLAG_SPECIALFLAGS,
		AP_DONTMOVE=1<<(OFLAG_SPECIALFLAGS+1),
		AP_COPYIED=1<<(OFLAG_SPECIALFLAGS+2),
		AP_TOUCHLATCH=1<<(OFLAG_SPECIALFLAGS+3),
		AP_DELETED=1<<(OFLAG_SPECIALFLAGS+4)
	};

	AutomationParameter(){}

	AutomationParameter(double v,int iflag)
	{
		value=v;
		flag=iflag;
	}

	OSTART GetParameterStart(){return ostart;}
	void SetParameterStart(OSTART t){ostart=t;}
	double GetParameterValue(){return value;}
	AutomationParameter *NextAutomationParameter();

	double value;
	int curvetype;
};

#endif