#ifndef CAMX_ASIODEVICE
#define CAMX_ASIODEVICE 1

#include "audiodevice.h"

#ifdef WIN32
#include "asio/asio.h"
#endif

class AudioHardwareChannel_ASIO;

class AudioDevice_ASIO:public AudioDevice
{
public:
	AudioDevice_ASIO();

	void MessageASIOError(char *from,ASIOError error);

	void SkipDeviceOutputBuffer(Seq_Song *);
	void InitMinMaxPrefBufferSizes();
	bool CheckAudioDevice(char *name);
	bool OpenAudioDevice(char *name);
	AudioHardwareChannel_ASIO *GetAsioHWChannel(int index,int asiotype,bool inputchannel);

	void InitLatencies(); //
	bool InitAudioDeviceChannels();
	void StartAudioHardware();
	void StopAudioHardware();
	int GetSamplePosition();
	void CloseDeviceDriver();
	void Reset(int flag);
	void ASIOCall(long index);

#ifdef WIN32
	
	ASIOBufferInfo *bufferarray;
	ASIOSampleType asiotype; // sample type

	int bufferarraychannels,bufferarrayplaybackchannels,bufferarrayrecordchannels;
	bool asiouseoutready;
#endif
};

#endif