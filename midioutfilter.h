#ifndef CAMX_MIDIOUTFILTER
#define CAMX_MIDIOUTFILTER 1

#include "defines.h"

class Filter_Note
{
public:
	UBYTE status,key,velocity;
	bool send;
};

class Filter_NoteOff
{
public:
	UBYTE status,key,velocityoff;
	bool send;
};

class Filter_Program
{
public:
	UBYTE status,program;
	bool send;
};

class Filter_PolyPressure
{
public:
	UBYTE status,key,pressure;	
	bool send;
};

class Filter_ChannelPressure
{
public:
	UBYTE status,pressure;
	bool send;
};

class Filter_Controller
{
public:
	UBYTE status,controller,value;
	bool send;
};

class Filter_Pitchbend
{
public:
	UBYTE status,lsb,msb;
	bool send;
};

class MIDIOutFilter
{
public:
	MIDIOutFilter()
	{
		ResetFilter();
	};
	
	Filter_Program programs[16];
	Filter_ChannelPressure channelpressure[16];
	Filter_PolyPressure polypressure[16];
	Filter_Pitchbend pitchbend[16];
	
	UBYTE controller[16][128];

	void ResetFilter();
};

#endif
