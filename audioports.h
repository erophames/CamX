#ifndef CAMX_AUDIOPORTS_H
#define CAMX_AUDIOPORTS_H 1

#define AUDIOCHANNELSDEFINED 8
#define CHANNELSPERPORT 32

#include "defines.h"

class AudioHardwareChannel;

class AudioPort
{
public:
	enum{
		AP_UNDEF,
		AP_INPUT,
		AP_OUTPUT
	};

	AudioPort();
	void GenerateName();
	void MixVInputToBuffer(AudioHardwareBuffer *,int tochannels,Peak *);
	int GetRecordBits();

	AudioHardwareChannel *hwchannel[MAXCHANNELSPERCHANNEL];
	char *name;
	int channels,portindex,iotype;
	bool hwchannelchanged,visible;
};
#endif