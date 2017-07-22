#ifndef CAMX_MIDICONTROLMAP
#define CAMX_MIDICONTROLMAP 1

#include "defines.h"

class MIDIOutControl
{
public:
	MIDIOutControl(){value=255;}
	UBYTE value;
};

class MIDIOutPitchbend
{
public:
	MIDIOutPitchbend(){lsb=msb=255;}
	UBYTE lsb,msb;
};

class MIDIOutProgram
{
public:
	MIDIOutProgram(){program=255;}
	UBYTE program;
};
class MIDIOutChannelPressure
{
public:
	MIDIOutChannelPressure(){pressure=255;}
	UBYTE pressure;
};
class MIDIOutPolyPressure
{
public:
	MIDIOutPolyPressure(){key=pressure=255;}
	UBYTE key,pressure;
};
#endif