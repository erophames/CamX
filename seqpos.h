#ifndef CAMX_SEQPOS_H
#define CAMX_SEQPOS_H 1

#include "defines.h"

class Seq_Signature;
class Seq_Project;

/*
case kVstSmpte24fps:

rate = AudioPlayHead::fps24;       fps = 24.0;  break;
case kVstSmpte25fps:        rate = AudioPlayHead::fps25;       fps = 25.0;  break;
case kVstSmpte2997fps:      rate = AudioPlayHead::fps2997;     fps = 29.97; break;
case kVstSmpte30fps:        rate = AudioPlayHead::fps30;       fps = 30.0;  break;
case kVstSmpte2997dfps:     rate = AudioPlayHead::fps2997drop; fps = 29.97; break;
case kVstSmpte30dfps:       rate = AudioPlayHead::fps30drop;   fps = 30.0;  break;

case kVstSmpteFilm16mm:
case kVstSmpteFilm35mm:     fps = 24.0; break;

case kVstSmpte239fps:       fps = 23.976; break;
case kVstSmpte249fps:       fps = 24.976; break;
case kVstSmpte599fps:       fps = 59.94; break;
case kVstSmpte60fps:        fps = 60; break;
*/

extern double SMPTE_FPS[];
extern char *smptestring[];
extern char *smpte_modestring[];
extern char *measure_str[];
extern char *measure_str_empty[];
extern char *smpte_str[];
extern char *sec_str[];
extern int smptemode[];

class Seq_Pos_Offset
{
public:
	Seq_Pos_Offset()
	{
		h=m=sec=frame=qf=0;
		changed=minus=false;
	}

	double GetOffSetMs();

	LONGLONG h,m,sec,frame,qf;
	Seq_Song *song;
	bool minus,changed;
};

class Seq_Pos
{
	friend class Seq_Time;

public:
	enum{
		POSMODE_NORMAL, // 1-1-1-1
		POSMODE_COMPRESS, // 1-1-1
		POSMODE_TIME, // std-min-sec

		POSMODE_SAMPLES,
		POSMODE_ONLYMEASURE,

		POSMODE_SMPTE_24,
		POSMODE_SMPTE_25,
		POSMODE_SMPTE_48,
		POSMODE_SMPTE_50,
		POSMODE_SMPTE_2997,
		POSMODE_SMPTE_30,
		POSMODE_SMPTE_2997df,
		POSMODE_SMPTE_30df,
		POSMODE_SMPTE_239,
		POSMODE_SMPTE_249,
		POSMODE_SMPTE_599,
		POSMODE_SMPTE_60
	};

	Seq_Pos();
	Seq_Pos(int m);

	bool IsSmpte()
	{
		return mode>=POSMODE_SMPTE_24 && mode<=POSMODE_SMPTE_60?true:false;
	}

	double GetFPS(); // SMPTE only
	bool Compare(Seq_Pos *);
	void Clone(Seq_Pos *);
	int GetPos3(Seq_Song *);
	void InitWithWindowDisplay(Seq_Project *,int mode);
	void ConvertToString(Seq_Song *,char *,size_t slen,char **singlestrings=0,int flag=0);
	void ConvertToLengthString(Seq_Song *,char *,size_t slen,char **singlestrings=0,int flag=0);

	bool PositionChanged();

	double ConvertToMicro();
	void ConvertMicroToPos(double);

	// Add Time

	// - Measure -
	bool AddMeasure(int);
	bool AddBeat(int);
	bool AddZoomTicks(int);
	bool AddTicks(int);

	// Length
	bool AddStartPositionEditing(int);
	bool AddBeatLength(int);
	bool AddZoomTicksLength(int);
	bool AddTicksLength(int);

	// - SMPTE -
	bool AddHour(LONGLONG);
	bool AddMin(LONGLONG);
	bool AddSec(LONGLONG);
	bool AddFrame(LONGLONG);
	bool AddQuarterFrame(LONGLONG);

	bool AddSec100(double); // .100

	// SMPTE -length
	bool AddPosition(Seq_Pos *);
	bool SubPosition(Seq_Pos *);

	LONGLONG pos[5];

	Seq_Signature *sig;
	Seq_Song *song;
	char **space;
	Seq_Pos_Offset *offset;

	OSTART zoomticks;// 1-1-1-1 0:0:0:0;3 // 04=sub
	int measureformat,mode,index;
	bool minus,nozoom,showquarterframe,usesmpte,length;
};
#endif