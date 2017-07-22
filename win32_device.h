#ifndef CAMX_WIN32DEVICE
#define CAMX_WIN32DEVICE 1

#include "audiodevice.h"

class AudioDevice_WIN32:public AudioDevice
{
public:
	AudioDevice_WIN32();

	bool CheckOutRefill();
	void FillInputBufferEnd();

	void SkipDeviceOutputBuffer(Seq_Song *);
	bool InitAudioDeviceChannels();
	void CreateDeviceStreamMix(Seq_Song *); //
	bool CheckAudioDevice(char *name);
	bool OpenAudioDevice(char *name);
	void StartAudioHardware();
	void StopAudioHardware();
	void InitMinMaxPrefBufferSizes();

	int GetSamplePosition();
	void CloseDeviceDriver();

	WAVEHDR header[2], // out
		headerin[2];

	HWAVEOUT hWaveOut;
	HWAVEIN hWaveIn;
	
	bool waveinopen,allcleared[2];
};

#endif