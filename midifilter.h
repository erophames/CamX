#ifndef CAMX_MIDIFILTER
#define CAMX_MIDIFILTER 1

#include "object.h"

enum{
	MIDIOUTFILTER_NOTEON=1,
	MIDIOUTFILTER_POLYPRESSURE=2,
	MIDIOUTFILTER_CONTROLCHANGE=4,
	MIDIOUTFILTER_PROGRAMCHANGE=8,
	MIDIOUTFILTER_CHANNELPRESSURE=16,
	MIDIOUTFILTER_PITCHBEND=32,
	MIDIOUTFILTER_SYSEX=64,
	MIDIOUTFILTER_INTERN=128
};

#define MIDIOUTFILTER_NOCHANNELCHECK 0xF0

enum{
	OCTAVE_0=1,
	OCTAVE_1=(1<<1),
	OCTAVE_2=(1<<2),
	OCTAVE_3=(1<<3),

	OCTAVE_4=(1<<4),
	OCTAVE_5=(1<<5),
	OCTAVE_6=(1<<6),
	OCTAVE_7=(1<<7),

	OCTAVE_8=(1<<8),
	OCTAVE_9=(1<<9),
	OCTAVE_10=(1<<10),
	OCTAVE_11=(1<<11),
};

#define OCTAVE_RESET (OCTAVE_0|OCTAVE_1|OCTAVE_2|OCTAVE_3|OCTAVE_4|OCTAVE_5|OCTAVE_6|OCTAVE_7|OCTAVE_8|OCTAVE_9|OCTAVE_10|OCTAVE_11)

class Edit_MIDIFilter;
class camxFile;
class Seq_Event;

class MIDIFilter:public Object
{
	friend Edit_MIDIFilter;

public:
	MIDIFilter(){
		Reset();	
	}
	
	bool CheckFilterActive();
	void Toggle(int flag);	
	void Reset();	

	bool Compare(MIDIFilter *f)
	{
		if(channelfilter!=f->channelfilter || statusfilter!=f->statusfilter || bypass!=f->bypass || octavefilter!=f->octavefilter)
			return false;

		return true;
	}

	bool IsFilterActive();
	void Clone(MIDIFilter *f)
	{
		f->bypass=bypass;
		f->channelfilter=channelfilter;
		f->statusfilter=statusfilter;
		f->octavefilter=octavefilter;
	}

	bool CheckEvent(Seq_Event *);
	bool CheckBytes(UBYTE status,UBYTE byte1,UBYTE byte2);

	bool CheckFilter(int flag)
	{
		if(bypass==true)return true;
		if(statusfilter&flag)return true;
		return false;
	}

	bool CheckFilter(int flag,UBYTE channel)
	{
		if(bypass==true)return true;

		if((statusfilter&flag) &&// Status ok ?
			(channel==MIDIOUTFILTER_NOCHANNELCHECK || ((1<<(channel&0x0F))&channelfilter)) // Channel ok ?
			)
			return true;

		return false;
	}

	bool CheckFilterNote(UBYTE status,UBYTE key);

	void LoadData(camxFile *);
	void SaveData(camxFile *);

	void Load(camxFile *);
	void Save(camxFile *);

	bool bypass;

protected:
	int channelfilter,statusfilter,octavefilter;
};

#endif
