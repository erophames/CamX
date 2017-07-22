#ifndef CAMX_AUDIOHARDWAREBUFFER
#define CAMX_AUDIOHARDWAREBUFFER 1

#include "defines.h"
#include "audiodefines.h"
#include "peaks.h"

class AudioDevice;
class AudioPort;
class AudioHardwareChannel;
class Peak;
class PanLaw;
class AudioHDFile;

class AudioHardwareBuffer
{
public:
	enum
	{
		PEAKCREATED=1,
		USETEMPPOSSIBLE=2,
		VOLUMEISCORRECT=4
	};

	AudioHardwareBuffer();

#ifdef _DEBUG
	char n[4];
#endif

	void SetBuffer(int i_channels,int i_size)
	{
		samplesinbuffer=i_size;
		samplesinbuffer_size=i_size*sizeof(ARES);
		channelsinbuffer=i_channels;

#ifdef DEBUG
		CheckBuffer();
#endif
	}

	void Mul(int samples,ARES mul);
	void MulCon(int samples,ARES mul);
	void MulCon32(float *o32,int samples,float mul);

//	void MulFloat(int samples,float mul);
	void CopyToFrames(ARES *to,int frames,int tchannels);
	void CopyFromFrames(ARES *from,int frames,int tchannels);
	int ConvertARESTo(void *to,int sampleformat,int channels,bool mixchannels,bool *iscleared); // Master->File

	void CreatePeak(AudioHardwareBuffer *b=0,PanLaw *plaw=0);
	void CreatePeakTo(Peak *to);
	void CreatePeak(AudioHardwareChannel *);
	void CopyAudioBuffer(AudioHardwareBuffer *);
	int GetFirstSampleOffset();

	void MixAudioBuffer(AudioHardwareBuffer *,AudioPort *);
	void MixAudioBuffer(AudioHardwareBuffer *,int mflag=0);
	void MixAudioBuffer(AudioHardwareBuffer *,Peak *,int mflag=0,ARES mulpeak=1);
	void MixAudioBuffer(AudioHardwareBuffer *,double volume,int mflag=0);

	int ExpandMixAudioBuffer(AudioHardwareBuffer *,int tochannels,Peak *,PanLaw *,int exflag=0,ARES mulpeak=1);
	int ExpandMixAudioBufferVolume(AudioHardwareBuffer *,int tochannels,double volume,Peak *,PanLaw *,int exflag=0);

	void ClearInput()
	{
		if(inputbuffer32bit && inputbuffersize)
			memset(inputbuffer32bit,0,inputbuffersize);
	}

	void ClearInputTo(char *to)
	{
		if(inputbuffer32bit)
			memset(inputbuffer32bit,0,to-(char *)inputbuffer32bit);
	}

	void ClearOutput(int clchls)
	{
		if(clchls>channelsinbuffer)clchls=channelsinbuffer;

		if(outputbufferARES)
			memset(outputbufferARES,0,clchls*samplesinbuffer_size);

		channelsused=0;

#ifdef _DEBUG
		CheckBuffer();
#endif
	}

	bool Create32BitBuffer_Input(int channels,int samples); // + Input Buffer
	void Delete32BitBuffer();
	void DeleteARESOut();
	void InitARESOut(int setSize,int channels);


#ifdef _DEBUG
	void CheckBuffer();
#endif

#ifdef ARES64
	void BridgeIn(float *,int channels,double multi);
	void BridgeOut32To64(float *from,int channels,double multi);
	void BridgeOut32To64(float *from,int channels);
	void BridgeOut32To64Mix(float *from,int channels);
	void BridgeOut32To64Mix(float *from,int channels,ARES multi);
#endif

	Peak peak;

	void *inputbuffer32bit; // Read from File/AudioHardware
	int inputbuffersize;

	ARES *outputbufferARES,*static_outputbufferARES, // -> Converted Samples/Mix
		hw_peaksum,max_peaksum,hw_peak[MAXCHANNELSPERCHANNEL],max_peaks[MAXCHANNELSPERCHANNEL];



#ifdef DEBUG
	LONGLONG streamstart,streamend;
#endif

	double bufferms,
		addpausesamples_ms; // mastering 0 Silence 

	int delayvalue,
		samplesinbuffer, // samples per buffer, LR Samples = 1 Samples (2x 32 Bit)
		samplesinbuffer_size,
		channelsinbuffer,
		channelsused,
		hwbflag,
		masteroffset; // mastering offset
	
	bool endreached;
};

#ifdef OLDIE

#define MAXHARDWAREBUFFERINFO 16

class AudioHardwareBufferInfo
{
public:
	AudioHardwareBufferInfo()
	{
		i_counter=0;
	}

	void CopyInfo(AudioHardwareBufferInfo *to)
	{
		memcpy(to,this,sizeof(AudioHardwareBufferInfo));

		/*
		to->i_counter=i_counter;

		for(int i=0;i<i_counter;i++)
		{
		to->sampleposition[i]=sampleposition[i];
		to->samplesize[i]=samplesize[i];
		to->sampleoffset[i]=sampleoffset[i];
		to->loop[i]=loop[i];
		}
		*/
	}

	LONGLONG sampleposition[MAXHARDWAREBUFFERINFO]; // you when Audio IN Disabled
	int samplesize[MAXHARDWAREBUFFERINFO],sampleoffset[MAXHARDWAREBUFFERINFO],loop[MAXHARDWAREBUFFERINFO]; //flag[MAXHARDWAREBUFFERINFO];
	int i_counter;
};


class AudioHardwareBufferEx:public AudioHardwareBuffer
{
public:
	AudioHardwareBufferInfo bufferinfo;
};

#endif

#endif