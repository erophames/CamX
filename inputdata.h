#ifndef CAMX_INPUTDATA
#define CAMX_INPUTDATA 1

#include "object.h"

class MIDIInputDevice;
class AudioObject;
class Seq_Song;

enum{
	NED_NOMIDI=1,
	NED_NOAUDIO=2
};

class NewEventData:public Object
{
public:
	NewEventData()
	{
		fromdev=0;
		fromplugin=0;
		data=0;
		deltaframes=0;
		flag=0;
	}

	~NewEventData(){FreeData();}
	void FreeData()
	{
		if(data)delete data;
		data=0;
	}
	
	NewEventData *NextEvent(){return (NewEventData *)next;}
	
	MIDIInputDevice *fromdev;
	Seq_Song *song;
	AudioObject *fromplugin;
	UBYTE *data;
	LONGLONG initsystime,alarmsystime,nesystime;
	OSTART netime,nedeltatime;
	int deltaframes,datalength,songstatusatinput;
	UBYTE status,byte1,byte2;
};
#endif
