#ifndef CAMX_OPENOUTPUTSNOTE_H
#define CAMX_OPENOUTPUTSNOTE_H 1

#include "object.h"

class AudioChannel;

class OpenOutputNote:public Object // Track -> Note -> Output
{
public:
	OpenOutputNote *NextOpenOutputNote(){return (OpenOutputNote *)next;}
	MIDIOutputDevice *outdevice;
	AudioChannel *channel;
	UBYTE status;
	char key;
};
#endif