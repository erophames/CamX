#ifndef CAMX_MIDIQUARTERFRAME

#define CAMX_MIDIQUARTERFRAME 1
class Seq_Song;

class MTC_Quarterframe
{
	// Nibbles
	// ms/ls
public:
	MTC_Quarterframe()
	{
		Reset();
		nak=true;
	}

	enum{
	IN_FRAME_LN=1,
	IN_FRAME=2,

	IN_SEC_LN=4,
	IN_SEC=8,

	IN_MIN_LN=16,
	IN_MIN=32,

	IN_HOUR_LN=64,
	IN_HOUR=128,
	};

	enum{
		SENDNEXTMTCQF_FRAME,
		SENDNEXTMTCQF_FRAME_H,
		SENDNEXTMTCQF_SEC,
		SENDNEXTMTCQF_SEC_H,
		SENDNEXTMTCQF_MIN,
		SENDNEXTMTCQF_MIN_H,
		SENDNEXTMTCQF_HOUR,
		SENDNEXTMTCQF_HOUR_H
	};

	void Reset();
	void MTC_AddQuarterFrame(Seq_Song *);

	LONGLONG hour,min,sek,frame,qframe;

	OSTART ticks;
	int rotation,inputflag;

	// -1=not set
	BYTE frame_ls, // frame 000fffff
	 frame_ms,
	 sec_ls, // sec 00ssssss
	 sec_ms,
	 min_ls, // min 00mmmmmm
	 min_ms,
	 hour_ls, // hour+Type 0tthhhhh
	 hour_ms;

	UBYTE timecodetype;
	bool set,nak;
};
#endif