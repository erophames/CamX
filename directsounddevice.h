#ifndef CAMX_DIRECTSDEVICE
#define CAMX_DIRECTSDEVICE 1

#include "audiodevice.h"

class DSOUND_AudioDevice:public AudioDevice
{
public:
	DSOUND_AudioDevice()
	{
		audiosystemtype=AUDIOCAMX_DIRECTS;
		
	}

	bool InitAudioDevice();
};

#endif