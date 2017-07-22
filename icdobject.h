#ifndef CAMX_ICDOBJ_H
#define CAMX_ICDOBJ_H 1

#include "objectevent.h"

// ICD Types
#define ICD_TYPE_UNDEFINED 0
#define ICD_TYPE_DRUM 1
#define ICD_TYPE_PROCESSOR 2

class ICD_Object:public Seq_MIDIEvent
{
public:	
	ICD_Object()
	{
		status=INTERN;
		id=OBJ_ICDEVENT;
		
#ifdef _DEBUG
		n[0]='I';
		n[1]='C';
		n[2]='D';
		n[3]='O';
#endif
	}
	
#ifdef _DEBUG
	char n[4];
#endif
	
	virtual int GetMSB(){return 0;}
	virtual char *GetTypeName(){return 0;}
	virtual char *GetInfo(){return 0;}
	virtual void LoadICDData(camxFile *){}
	virtual void SaveICDData(camxFile *){}

	int GetICD(){return type;}
	Object* Clone(Seq_Song *song){return 0;}
	void CloneData(Seq_Song *,Seq_Event *){}
	void ReadAndAddToPattern(MIDIPattern *,camxFile *,OSTART ostart,OSTART sstart);
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject){}
	void Save(camxFile *);
	UBYTE GetStatus() {return INTERN;}

	bool CheckSelectFlag(int checkflag,int icdtype);
	
	int type,datalength;
};

class ICD_Object_Seq_MIDIChainEvent:public Seq_MIDIChainEvent
{
public:
	ICD_Object_Seq_MIDIChainEvent()
	{
		status=INTERNCHAIN;
		id=OBJ_ICDEVENT;

#ifdef _DEBUG
		n[0]='I';
		n[1]='C';
		n[2]='D';
		n[3]='O';
#endif
	}

#ifdef _DEBUG
	char n[4];
#endif

	virtual void LoadICDData(camxFile *){}
	virtual void SaveICDData(camxFile *){}

	virtual int GetMSB(){return 0;}
	virtual char *GetTypeName(){return 0;}
	virtual char *GetInfo(){return 0;}

	int GetICD(){return type;}
	Object* Clone(Seq_Song *song){return 0;}
	void CloneData(Seq_Song *,Seq_Event *){}

	void ReadAndAddToPattern(MIDIPattern *,camxFile *,OSTART ostart,OSTART sstart);
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject){}
	void Save(camxFile *);
	UBYTE GetStatus() {return INTERNCHAIN;}

	bool CheckSelectFlag(int checkflag,int icdtype);
	OSTART GetPlaybackStart(MIDIPattern *,Seq_Track *); //v

	int type,datalength;
};
#endif
