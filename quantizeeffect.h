#ifndef CAMX_QUANTIZEEFFECTS_H
#define CAMX_QUANTIZEEFFECTS_H 1

#include "object.h"

class Groove;
class Seq_Pattern;
class Seq_Track;
class Seq_Event;
class Note;

#define QUANTIZE_NOTES 1
#define QUANTIZE_CONTROL 2
#define QUANTIZE_PITCHBEND 4
#define QUANTIZE_POLYPRESSURE 8
#define QUANTIZE_CHANNELPRESSURE 16
#define QUANTIZE_SYSEX 32
#define QUANTIZE_PROGRAMCHANGE 64
#define QUANTIZE_INTERN 128

class QuantizeEffect:public Object
{
public:
	QuantizeEffect()
	{
		track=0;
		pattern=0;
		qinfo=0;
		flag=QUANTIZE_NOTES;
		humanq=0; // Track Humanize Quant

		Reset();
	}

	~QuantizeEffect()
	{
		if(qinfo)delete qinfo;
	}

	bool IsInUse();
	void Reset();
	bool Compare(QuantizeEffect *); // true==not the same
	void Clone(QuantizeEffect *);

	OSTART Quantize(OSTART p);
	OSTART QuantizeNoteOff(OSTART p);
	bool QuantizeNote(Note *);
	bool QuantizeEvent(Seq_Event *);

	void Load(camxFile *);
	void Save(camxFile *);
	char *GetQuantizeInfo();
	void Humanize(bool useh);
	void SetHumanQ(int q);
	void SetHumanRange(int r);

	Groove *groove;
	Seq_Track *track;
	Seq_Pattern *pattern;

	int quantize,
		capturestrength, // 1-100
		capturerange; // 1-100

	int humanrange,
		humanq, // 0=Track Quantize
		notelength,
		flag;

	bool usequantize,
		usegroove,
		usehuman,
		setnotelength,
		capturequant,
		noteonquant,
		noteoffquant,
		groovequant,
		strengthquant;

private:
	char *qinfo; // Info Text
};

#endif
