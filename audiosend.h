#ifndef CAMX_AUDIOSEND_H
#define CAMX_AUDIOSEND_H 1

#include "object.h"

class AudioChannel;

class AudioSend:public Object
{
public:
	AudioSend(AudioChannel *chl)
	{
		sendchannel=chl;
		sendpost=true;
		sendbypass=false;
		sendvolume=0; // default silence
	}

	AudioSend *NextSend(){return (AudioSend *)next;}

	void EditVolume(double nsv) // Auto later
	{
		sendvolume=nsv;
	}

	AudioChannel *sendchannel;
	double sendvolume;
	bool sendpost,sendbypass;
};

#endif