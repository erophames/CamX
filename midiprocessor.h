#ifndef CAMX_MIDIPROCESSOR_H
#define CAMX_MIDIPROCESSOR_H 1

class MIDIPattern;
class TrackEffects;
class MIDIEffects;

#include "object.h"
#include "defines.h"

class Seq_Event;
class Seq_Track;
class MIDIInputDevice;
class ICD_Processor;
class Seq_Song;
class Proc_Alarm;
class Processor;
class Edit_ProcMod;
class guiWindowSetting;
class guiGadget;
class guiWindow;
class EditData;
class AudioObject;
class NewEventData;
class MIDIProcessor;

enum ProcFlags
{
	PROCESSORFLAG_DOSENDTOTRACKWITHALWAYSTHRU=1
};



class Proc_AddEvent
{
public:
	Proc_AddEvent(Seq_Event *e,Seq_Track *t)
	{
		triggerevent=e;
		track=t;
	}
	Seq_Track *track;
	Seq_Event *triggerevent;
};

class MIDIPlugin:public ObjectLock
{
public:
	void InitModule()
	{
		win=0;
		bypass=false;
	}

	enum ModuleTypes{
		PM_UNKNOWN,
		PM_ARPEGGIATOR,
		PM_DELAY
	};

	virtual void Reset()=0;
	virtual int GetEditorSizeX(){return 200;}
	virtual int GetEditorSizeY(){return 180;}

	virtual void EditDataMessage(EditData *){}
	virtual guiGadget *Gadget(guiGadget *g){return g;}

	virtual void OpenGUI(guiWindowSetting *);
	virtual void CloseGUI();
	virtual void InitModuleGUI(Edit_ProcMod *,int x,int y,int x2,int y2){}
	virtual void ShowGUI()=0;

	MIDIPlugin *NextModule(){return (MIDIPlugin *)next;}

	virtual int NextAlarm(int alarmtime){return alarmtime;}
	virtual void FreeProcessorModuleMemory(){}
	virtual void InsertEvent(Proc_AddEvent *addevent,MIDIProcessor *)// do nothing
	{
	}

	virtual MIDIPlugin *CreateClone(){return 0;}
	virtual void Alarm(Proc_Alarm *){}

	// Events IO
	virtual void Trigger(ICD_Processor *){}
	virtual void Load(camxFile *)=0;
	virtual void Save(camxFile *)=0;

	void LoadStandards(camxFile *);
	void SaveStandards(camxFile *);

	char staticname[STANDARDSTRINGLEN+1];
	Processor *processor;
	guiWindow *win; // Editor
	int moduletype;
	bool bypass;
};

class MIDIProcessor:public Object // Event-> Processor -> Event
{
	friend class MIDIThruThread;

public:	
	MIDIProcessor(Seq_Song *,Seq_Track *);
	~MIDIProcessor();

	Seq_Event *FirstProcessorEvent(){return (Seq_Event *)events.GetRoot();}
	Seq_Event *DeleteEvent(Seq_Event *);
	Seq_Event *DeletedEventByModule(MIDIPlugin *,Seq_Event *,Seq_Track *,MIDIPattern *,Seq_Event *);

	void EventInput(MIDIPattern *,Seq_Event *,OSTART pos);
	void EventInput_RAW(MIDIPattern *,Seq_Event *);
	void EventInput_CalledByProcAlarm(Proc_Alarm *,MIDIPlugin *,Seq_Event *);
	void AddProcEvent(Seq_Event *,MIDIPattern *,Seq_Event *,int flag=0);

	enum SPE_Flags
	{
		SPE_THRUCHECK=1,
		SPE_INPUT=2
	};

	void SendProcessorEvents(NewEventData *,int checkflag,int sampleoffset=0,int flag=0);
	void AddEventsToRAW();

	TrackEffects *trackfx;
	MIDIEffects *patternfx;
	Seq_Song *song;
	Seq_Track *track;
	Seq_Event *inevent;
	MIDIInputDevice *indevice;
	AudioObject *inplugin;
	bool createraw;

private:
	void AddStaticFX(MIDIPattern *);
	void AddModules(MIDIPlugin *,Seq_Event *);

	OListStart events;
};

class Proc_Alarm:public OStart // <Thread Alarm Object>
{
public:
	Proc_Alarm(MIDIProcessor *proc,Seq_Song *s,MIDIPlugin *m,OSTART ticks)
	{
		alarmticks=ticks;
		module=m;

		processor=proc;
		song=s;
		infoflag=0;
		calledbyaudio=calledbyalarm=false;
		deleteobject=0;
		forcealarm=false;
		createraw=false;
		offset_sample=0;
		offset_ms=0;
	}

	Proc_Alarm *NextAlarm(){return (Proc_Alarm *)next;}
	
	MIDIPlugin *module;
	OSTART songposition,alarmticks;
	LONGLONG systemtime_ms,alarmsystemtime_ms;
	MIDIProcessor *processor;
	Seq_Song *song;
	Object *object,*deleteobject;

	int offset_sample, // called by audio
		offset_ms; // called by audio

	int infoflag;
	bool calledbyalarm,calledbyaudio,forcealarm,createraw;
};

class Processor:public Object
{
public:
	Processor()
	{
		strcpy(name,"Processor");
		track=0;
		bypass=false;
	}

	Processor *Clone();
	void AddProcessorModule(MIDIPlugin *,MIDIPlugin *prev=0);
	MIDIPlugin *FirstProcessorModule(){return (MIDIPlugin *)modules.GetRoot();}
	MIDIPlugin *DeleteProcessorModule(MIDIPlugin *);
	
	MIDIPlugin *GetModuleAtIndex(int ix){return (MIDIPlugin *)modules.GetO(ix);}

	void DeleteAllProcessorModule();
	void RemoveProcessorFromAlarms();
	Processor *NextProcessor(){return (Processor *)next;}

	void SetName(char *);
	void Delete(bool full);
	void DeleteThruOffs(Seq_Track *,MIDIInputDevice *,UBYTE instatus,UBYTE key,UBYTE veloff);

	void Save(camxFile *);
	void Load(camxFile *);

	char *ProcessorName(){return name;}
	char name[STANDARDSTRINGLEN+1];

	Seq_Track *track;
	bool bypass;

private:
	OList modules;
};

class ProcessorModulePointer:public Object
{
public:
	ProcessorModulePointer *NextProcessorPointer(){return (ProcessorModulePointer *)next;}	
	MIDIPlugin *module;
};

class mainProcessor
{
public:	
	void AddRAWEvent(Seq_Event *);
	Seq_Event *FirstRAWEvent(){return (Seq_Event *)rawevents.GetRoot();}
	Seq_Event *DeleteRAWEvent(Seq_Event *,OListStart *sortto);

	Processor *FirstProcessor() {return (Processor *)processorlist.GetRoot();}
	Processor *LastProcessor() {return (Processor *)processorlist.Getc_end();}
	Processor *AddProcessor(Processor *,Processor *prev=0);
	Processor *DeleteProcessor(Processor *);

	void DeleteAllProcessor();

	void Load(camxFile *);
	void Save(camxFile *);

	ProcessorModulePointer *InitProcessorModule(MIDIPlugin *);
	void InitDefaultProcessorModule();
	ProcessorModulePointer *FirstProcessorModule() {return (ProcessorModulePointer *)processormodulelist.GetRoot();}
	Processor *GetProcessorAtIndex(int ix){return (Processor *)processorlist.GetO(ix);}
private:
	OList processorlist,processormodulelist; // OBJ
	OListStart rawevents;
};
#endif
