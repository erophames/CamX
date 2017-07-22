#ifndef CAMX_EVENTICD_H
#define CAMX_EVENTICD_H 1

#include "icdobject.h"
#include "quantizeeffect.h"
#include "drumtrack.h"
#include "MIDIPattern.h"
#include "seq_realtime.h"
#include "icdobject.h"

class ICD_Drum:public ICD_Object_Seq_MIDIChainEvent{

public:
	ICD_Drum(){
		type=ICD_TYPE_DRUM;
		drumtrack=0;
		datalength=sizeof(drumtrack)+sizeof(velocity)+sizeof(velocityoff);
	}

	void MoveIndex(int index);
	bool CheckMute();
	void LoadICDData(camxFile *);
	void SaveICDData(camxFile *);
	int GetVelocity();
	int GetMSB(){return velocity;} //

	char *GetTypeName(){return "Drum";}
	char *GetInfo(){return drumtrack?drumtrack->name:0;}

	char GetByte1(){return velocity;}
	char GetByte2(){return velocityoff;}

	void SetByte1(char b){velocity=b;}
	void SetByte2(char b){velocityoff=b;}

	void AddSortToPattern(MIDIPattern *p,OSTART start);
	bool QuantizeEvent(QuantizeEffect *fx); // v

	bool Compare(Seq_Event *);

	void SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int flag,int offset,AudioObject *dontsendtoaudioobject);
	void SendToAudioPlayback(AudioObject *,LONGLONG samplestart,Seq_Track *,MIDIPattern *,Seq_Event *,int flag,int offset,bool createrealtime);
	void SendToDevicePlayback(MIDIOutputDevice *,Seq_Song *,Seq_Track *,MIDIPattern *,Seq_Event *,int createflag);
	void SendToDevicePlaybackUser(MIDIOutputDevice *,Seq_Song *,int createflag);

	LONGLONG GetSampleSize(Seq_Song *,Seq_Pattern *);
	LONGLONG GetSampleEnd(Seq_Song *,Seq_Pattern *);

	bool CheckIfPlaybackIsAble(){return true;} // Notes
	bool CheckFilter(MIDIFilter *filter){return filter->CheckFilter(MIDIOUTFILTER_INTERN,drumtrack->GetMIDIChannel());}
	Object *Clone(Seq_Song *);
	void CloneData(Seq_Song *,Seq_Event *);
	
	void EraseCloneData();
	bool ClonePossible(Seq_Song *);
	void Delete(bool full){delete this;}
	void SetMIDIImpulse(Seq_Track *);
	void DoEventProcessor(MIDIProcessor *);

	Drumtrack *drumtrack;
	UBYTE velocity,velocityoff;

	// Copy/Paste
	int dt_volume, // -127 <-> +127
		dt_index;

	UBYTE dtr_channel;
	char dtr_key,dtr_velocityoff;
};

#endif