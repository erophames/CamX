#ifndef CAMX_EVENTOBJECTS_H
#define CAMX_EVENTOBJECTS_H 1

#include "defines.h"
#include "MIDIfilter.h"
#include "objectids.h"

class Seq_Track;
class Seq_Pattern;
class AudioChannel;
class AudioEffects;

class AudioObject;
class AudioPattern;
class MIDIPattern;
class MIDIProcessor;
class Seq_Song;
class MIDIOutputDevice;
class Note;
class NoteOff_Realtime;
class QuantizeEffect;

#ifdef MEMPOOLS
class mempool_Note;
class mempool_Control;
class mempool_Pitchbend;
#endif

class camxFile;

enum{
	EVENTFLAG_PROCALARM=(1<<OFLAG_SPECIALFLAGS),
	EVENTFLAG_THRUEVENT=(1<<(OFLAG_SPECIALFLAGS+1)),
	EVENTFLAG_ADDEDBYTHISMODULE=(1<<(OFLAG_SPECIALFLAGS+2)),
	EVENTFLAG_MUTED=(1<<(OFLAG_SPECIALFLAGS+3)),
	EVENTFLAG_UNDEREDIT=(1<<(OFLAG_SPECIALFLAGS+4)),
	EVENTFLAG_RECORDED=(1<<(OFLAG_SPECIALFLAGS+5)),
	EVENTFLAG_PRESTART=(1<<(OFLAG_SPECIALFLAGS+6)),
	EVENTFLAG_MOVEEVENT=(1<<(OFLAG_SPECIALFLAGS+7))
};

enum{
	SENDAUDIO_DONTCREATEREALEVENTS=1,
	SENDAUDIO_NOCOUNTDOWN=2
};

class Seq_Event:public OStart
{
public:
	enum{
		STD_CREATEREALEVENT=1,
		STD_PRESTARTEVENT=(1<<1)
	};

	Seq_Event(){Init();}
	void Init(){flag=0;}

	virtual void MoveIndex(int index){}
	virtual int GetICD(){return -1;};
	virtual bool QuantizeEvent(QuantizeEffect *);
	virtual OSTART GetPlaybackStart(MIDIPattern *,Seq_Track *);

	virtual Object *Clone(Seq_Song *)=0;
	virtual void CloneData(Seq_Song *,Seq_Event *)=0;

	virtual bool Compare(Seq_Event *){return false;}
	virtual Seq_Event *CloneNewAndSortToPattern(Seq_Song *,MIDIPattern *,int iflag);
	virtual Seq_Event *CloneNewAndSortToPattern(Seq_Song *,MIDIPattern *,OSTART diff,int flag);
	virtual void AddSortToPattern(MIDIPattern *,OSTART start);
	virtual void AddSortToPattern(MIDIPattern *);
	virtual void SendPreProcDelete(Seq_Track *,MIDIPattern *,Seq_Event *){}
	virtual void EraseCloneData(){}
	virtual bool ClonePossible(Seq_Song *){return true;}

	// MIDI Output
	virtual void SendToDevice(MIDIOutputDevice *){}

	virtual void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag){}
	virtual void SendToDevicePlaybackUser(MIDIOutputDevice *d,Seq_Song *,int createflag){SendToDevice(d);}

	// AUDIO Output
	virtual void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject)=0;
	virtual void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime){}

	virtual void DeleteFromPattern(){}
	virtual void MoveEventNoStatic(OSTART pos){GetList()->MoveO(this,pos);}
	virtual void MoveEvent(OSTART diff){staticostart+=diff;GetList()->MoveO(this,ostart+diff);}
	virtual void MoveEventQuick(OSTART diff){staticostart+=diff;ostart+=diff;}

	virtual bool CheckSelectFlag(int flag,int icdtype){return false;}
	virtual bool CheckIfPlaybackIsAble();
	virtual bool CheckFilter(MIDIFilter *){return true;}
	virtual bool CheckFilter(int fflag,int icdtype){return false;}
	virtual void DoEventProcessor(MIDIProcessor *){}
	virtual UBYTE GetStatus(){return 0;} 
	virtual bool EventHasByte1(){return true;}
	virtual bool EventHasByte2(){return false;}
	virtual bool EventHasByte3(){return false;}
	virtual bool EventHasChannel(){return true;}
	virtual bool EventHasLength(){return false;}
	virtual char GetByte1(){return -1;}
	virtual char GetByte2(){return -1;}
	virtual char GetByte3(){return -1;}
	virtual void SetByte1(char){}
	virtual void SetByte2(char){}
	virtual void SetByte3(char){}
	virtual bool IsMIDI(){return true;}
	virtual bool IsAudio(){return false;}
	virtual bool IsVideo(){return false;}
	virtual bool IsSwingAble(){return false;}

	virtual Seq_Track *GetTrack();
	virtual void SetStartStatic(OSTART pos);	// start+static
	virtual AudioPattern *GetAudioPattern(){return 0;}
	virtual MIDIPattern *GetMIDIPattern(){return 0;}
	virtual void SetMIDIImpulse(Seq_Track *){}

	inline OSTART GetEventStart(){return ostart;}
	inline OSTART GetEventStart(Seq_Pattern *);

	LONGLONG GetSampleStart(Seq_Song *,Seq_Pattern *); // Loop
	LONGLONG GetSampleEnd(Seq_Song *,Seq_Pattern *); // Loop

	inline OSTART GetStaticStart(){return staticostart;}

	void MoveEventAbs(OSTART pos){MoveEvent(pos-ostart);}

	QuantizeEffect *GetQuantizer();
	Seq_Event *NextEvent(){return (Seq_Event *)next;}	
	Seq_Event *PrevEvent(){return (Seq_Event *)prev;}

	bool CheckIfChannelMessage(){return status<0xF0?true:false;}

	void SetStatus(UBYTE);
	inline UBYTE GetChannel() {return status&0x0F;} // first 4 bits are statusbits
	void SetChannel(UBYTE c){if(c<16){status&=0xF0;status|=c;} }
	void WriteEventToMIDIFile(camxFile *){}
	bool SelectEvent(bool select,bool guirefresh=true);
	Seq_Pattern *GetPattern(){return pattern;}
	void SetPattern(Seq_Pattern *p){pattern=p;}

	Seq_Pattern *pattern; // Pattern of this Event
	OSTART staticostart; // originalposition without quantize
	UBYTE status;	
};	

class Seq_AudioEvent:public Seq_Event
{
public:
	AudioPattern *GetAudioPattern(){return (AudioPattern *)pattern;}
	AudioPattern *GetPattern(){return (AudioPattern *)pattern;}

	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject){}
};

class Seq_MIDIEvent:public Seq_Event
{
public:
	void DeleteFromPattern();
	bool CheckFilter(int fflag,int icdtype);

	MIDIPattern *GetMIDIPattern() {return (MIDIPattern *)pattern;}
	MIDIPattern *GetPattern(){return (MIDIPattern *)pattern;}
};

class Seq_MIDIChainEvent:public Seq_MIDIEvent
{
public:
	bool IsSwingAble(){return true;}

	Seq_MIDIChainEvent *prev_chainevent,*next_chainevent; // --- Playback Note Chain ---
	OSTART realtimeswing;
};

class NoteOff:public Seq_MIDIEvent // Virtual object of note
{
	friend class Note;

public:
	NoteOff(){id=OBJ_NOTEOFF;}
	UBYTE GetStatus(){return NOTEOFF;}

	Object *Clone(Seq_Song *){return 0;}
	void CloneData(Seq_Song *,Seq_Event *){}
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject){}

	Note *noteon; // Pointer to note
};

class NoteOff_Raw:public Seq_MIDIEvent
{
public:
	NoteOff_Raw(UBYTE s,UBYTE k,UBYTE velooff){note=0;status=s;key=k;velocityoff=velooff;}
	NoteOff_Raw(){}
	void DoEventProcessor(MIDIProcessor *);

	void CloneData(Seq_Song *song,Seq_Event *e)
	{
		NoteOff_Raw *nor=(NoteOff_Raw *)e;
		nor->status=status;
		nor->key=key;
		nor->velocityoff=velocityoff;
		nor->ostart=ostart;
		nor->staticostart=staticostart;
	}

	Object *Clone(Seq_Song *song)
	{
		if(NoteOff_Raw *n=new NoteOff_Raw(status,key,velocityoff)){
			n->ostart=ostart;
			n->staticostart=staticostart;
			return n;
		}

		return 0;
	}

	UBYTE GetStatus() {return NOTEOFF;}

	void Delete(bool full){delete this;}

	void SendToDevice(MIDIOutputDevice *);
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);

	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);

	Note *note;
	OSTART notestartpos;
	UBYTE key,velocityoff;
};

class Note:public Seq_MIDIChainEvent
{
public:
	Note(){id=OBJ_NOTE;off.noteon=this;
#ifdef MEMPOOLS

	pool=0;
#endif
	}

	bool Compare(Seq_Event *);
	void MoveIndex(int index);
	void Load(camxFile *);
	void Save(camxFile *);

	LONGLONG GetSampleEnd(Seq_Song *,Seq_Pattern *);

	UBYTE GetStatus() {return NOTEON;}

	bool EventHasByte2(){return true;}
	bool EventHasByte3(){return true;}
	bool EventHasLength(){return true;}

	char GetByte1(){return key;}
	char GetByte2(){return velocity;}
	char GetByte3(){return velocityoff;}

	void SetByte1(char b){key=b;}
	void SetByte2(char b){velocity=b;}
	void SetByte3(char b){velocityoff=b;}
	void SendPreProcDelete(Seq_Track *,MIDIPattern *,Seq_Event *);

	// MIDI
	void SendToDevice(MIDIOutputDevice *);
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);
	void SendToDevicePlaybackUser(MIDIOutputDevice *,Seq_Song *,int createflag);

	//AUDIO
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);

	bool QuantizeEvent(QuantizeEffect *); //v
	OSTART GetPlaybackStart(MIDIPattern *,Seq_Track *); //v

	void MoveEvent(OSTART diff);
	void MoveEventQuick(OSTART diff);
	void MoveNote(OSTART startposition,OSTART endposition);
	void DoEventProcessor(MIDIProcessor *);
	bool CheckSelectFlag(int checkflag,int icdtype);
	bool CheckFilter(MIDIFilter *filter){return filter->CheckFilterNote(status,key);}
	bool CheckIfPlaybackIsAble(){return true;}

	Object *Clone(Seq_Song *);
	void CloneData(Seq_Song *,Seq_Event *);
	Seq_Event *CloneNewAndSortToPattern(Seq_Song *,MIDIPattern *,int flag);
	Seq_Event *CloneNewAndSortToPattern(Seq_Song *,MIDIPattern *,OSTART diff,int flag);

	void AddSortToPattern(MIDIPattern *);
	void AddSortToPattern(MIDIPattern *,OSTART start);
	void DeleteFromPattern();
	void Delete(bool full);
	void SetStartStatic(OSTART);

	OSTART GetNoteLength(){return off.ostart-ostart;}

	void SetStart(OSTART newpos){if(newpos<=off.ostart && newpos!=ostart)GetList()->MoveO(this,newpos);}
	void SetEnd(OSTART newpos);
	OSTART GetNoteEnd(){return off.ostart;}
	OSTART GetNoteEnd(Seq_Pattern *p){return off.GetEventStart(p);}
	void SetLength(OSTART ticks){SetEnd(ostart+ticks);}
	LONGLONG GetSampleSize(Seq_Song *,Seq_Pattern *);
	void SetMIDIImpulse(Seq_Track *);

	NoteOff off;
#ifdef MEMPOOLS
	mempool_Note *pool;
#endif
	UBYTE key,velocity,velocityoff;

private:
	void InitNoteOff(MIDIOutputDevice *,Seq_Track *,MIDIPattern *,Seq_Event *,LONGLONG sampleposition,int createflag);
};

class NoteOpen:public OStart // MIDI Record Note On
{
public:
	NoteOpen *NextOpenNote(){return (NoteOpen *)next;}
	UBYTE status,key,velocity;
};

class PolyPressure:public Seq_MIDIEvent
{
public:
	PolyPressure(){id=OBJ_POLYPRESSURE;}

	bool Compare(Seq_Event *);
	void Load(camxFile *);
	void Save(camxFile *);

	Object *Clone(Seq_Song *);
	void CloneData(Seq_Song *,Seq_Event *);

	bool CheckSelectFlag(int checkflag,int icdtype);
	UBYTE GetStatus() {return POLYPRESSURE;}

	bool EventHasByte2(){return true;}

	char GetByte1(){return key;}
	char GetByte2(){return pressure;}
	void SetByte1(char b){key=b;}
	void SetByte2(char b){pressure=b;}

	//MIDI
	void SendToDevice(MIDIOutputDevice *);
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);

	//AUDIO
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);

	void Delete(bool full){delete this;}
	bool CheckFilter(MIDIFilter *filter){return filter->CheckFilter(MIDIOUTFILTER_POLYPRESSURE,status);}

	UBYTE key,pressure;
};

// First send MSB, the the LSB
// 7 Bits needed: only the MSB is sent (not neceesarry to send the LSB)
class ControlChange:public Seq_MIDIEvent
{
public:
	ControlChange()
	{
		id=OBJ_CONTROL;
#ifdef MEMPOOLS
		pool=0;
#endif
	}

	bool Compare(Seq_Event *);
	void Load(camxFile *);
	void Save(camxFile *);

	Object *Clone(Seq_Song *);
	void CloneData(Seq_Song *,Seq_Event *);

	UBYTE GetStatus() {return CONTROLCHANGE;}

	bool EventHasByte2(){return true;}
	char GetByte1(){return controller;}
	char GetByte2(){return value;}

	void SetByte1(char b){controller=b;}
	void SetByte2(char b){value=b;}

	//MIDI
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);
	void SendToDevice(MIDIOutputDevice *);

	//AUDIO
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);

	bool CheckSelectFlag(int checkflag,int icdtype)
	{
		if( (checkflag&SEL_CONTROLCHANGE) && ( (!(checkflag&SEL_SELECTED)) || (flag&OFLAG_SELECTED)))return true;
		return false;
	}

	bool CheckFilter(MIDIFilter *filter){return filter->CheckFilter(MIDIOUTFILTER_CONTROLCHANGE,status);}
	void Delete(bool full);

#ifdef MEMPOOLS
	mempool_Control *pool;
#endif

	UBYTE controller,value;
};

class ProgramChange:public Seq_MIDIEvent
{
public:
	ProgramChange(){id=OBJ_PROGRAM;info=0;}
	Object *Clone(Seq_Song *);
	void CloneData(Seq_Song *,Seq_Event *);

	bool ChangeInfo(char *);
	bool Compare(Seq_Event *);
	void Load(camxFile *);
	void Save(camxFile *);
	UBYTE GetStatus() {return PROGRAMCHANGE;}

	char GetByte1(){return program;}
	void SetByte1(char b){program=b;}

	//MIDI
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);
	void SendToDevice(MIDIOutputDevice *);

	//AUDIO
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);
	bool CheckSelectFlag(int checkflag,int icdtype);
	bool CheckFilter(MIDIFilter *filter){return filter->CheckFilter(MIDIOUTFILTER_PROGRAMCHANGE,status);}
	void Delete(bool full);

	char *info;
	UBYTE program; // Prog Info String default 0
};

class ChannelPressure:public Seq_MIDIEvent
{
public:
	ChannelPressure(){id=OBJ_CHANNELPRESSURE;}

	bool Compare(Seq_Event *e){return (e->status==status && ((ChannelPressure *)e)->pressure==pressure )?true:false;}
	void Load(camxFile *);
	void Save(camxFile *);
	UBYTE GetStatus() {return CHANNELPRESSURE;}
	char GetByte1(){return pressure;}
	void SetByte1(char b){pressure=b;}

	//MIDI
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);
	void SendToDevice(MIDIOutputDevice *);

	//AUDIO
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);

	bool CheckSelectFlag(int checkflag,int icdtype){return ((checkflag&SEL_CHANNELPRESSURE) && ((!(checkflag&SEL_SELECTED)) || (flag&OFLAG_SELECTED)))?true:false;}
	bool CheckFilter(MIDIFilter *filter){return filter->CheckFilter(MIDIOUTFILTER_CHANNELPRESSURE,status);}

	Object *Clone(Seq_Song *song)
	{
		if(ChannelPressure *p=new ChannelPressure){CloneData(0,p);return p;}
		return 0;
	}

	void Delete(bool full){delete this;}
	void CloneData(Seq_Song *,Seq_Event *);

	UBYTE pressure;
};

class Pitchbend:public Seq_MIDIEvent
{
public:
	Pitchbend()
	{
		id=OBJ_PITCHBEND;
#ifdef MEMPOOLS
		pool=0;
#endif
	}

	bool Compare(Seq_Event *);
	void Load(camxFile *);
	void Save(camxFile *);
	int GetPitch();
	void SetPitchbend(int pitchbend);

	UBYTE GetStatus() {return PITCHBEND;}
	bool EventHasByte2(){return true;}

	char GetByte1(){return lsb;}
	char GetByte2(){return msb;}

	void SetByte1(char b){lsb=b;}
	void SetByte2(char b){msb=b;}

	//MIDI
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);
	void SendToDevice(MIDIOutputDevice *);

	//AUDIO
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);

	bool CheckSelectFlag(int checkflag,int icdtype);
	bool CheckFilter(MIDIFilter *filter){return filter->CheckFilter(MIDIOUTFILTER_PITCHBEND,status);}

	Object *Clone(Seq_Song *song);
	void Delete(bool full);
	void CloneData(Seq_Song *,Seq_Event *);

#ifdef MEMPOOLS
	mempool_Pitchbend *pool;
#endif

	UBYTE lsb,msb;
};

class SysEx:public Seq_MIDIEvent
{
public:
	SysEx();

	void Load(camxFile *);
	void Save(camxFile *);

	UBYTE GetStatus() {return SYSEX;}

	bool EventHasChannel(){return false;}
	bool EventHasByte1(){return false;}
	bool EventHasByte2(){return false;}

	//MIDI
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);
	void SendToDevice(MIDIOutputDevice *);

	//AUDIO
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);
	bool CheckSelectFlag(int checkflag,int icdtype);
	bool CheckFilter(MIDIFilter *filter){return filter->CheckFilter(MIDIOUTFILTER_SYSEX,MIDIOUTFILTER_NOCHANNELCHECK);}

	Object *Clone(Seq_Song *);
	Seq_Event *CloneNewAndSortToPattern(Seq_Song *,MIDIPattern *,int flag);
	Seq_Event *CloneNewAndSortToPattern(Seq_Song *,MIDIPattern *,OSTART diff,int flag);

	void CloneData(Seq_Song *,Seq_Event *);
	void Delete(bool full);

	char *GetSysExString();
	char *Get3SysEx ();
	char *GetRealTime ();
	char *GetNonRealtime ();

	UBYTE *data; // SysEx
	int length; // SysEx Length
};

class Undo_EditEvent;

class EF_CreateEvent // Editor Edit/Create Event
{
public:
	EF_CreateEvent(){
		checkgui=true;
		playit=addtolastundo=false;
		insidelastundo=0;
	};

	Seq_Song *song;
	MIDIPattern *pattern;
	Seq_Event *seqevent;
	Undo_EditEvent *insidelastundo;
	OSTART position,endposition; // note
	bool doundo,checkgui,playit,addtolastundo;
};
#endif
