#ifndef CAMX_AUDIOHARDWARECHANNEL
#define CAMX_AUDIOHARDWARECHANNEL 1

#include "object.h"
#include "audiodefines.h"
#include "audiohardwarebuffer.h"

#include <math.h>

#ifdef WIN32
#include "asio/asio.h"

struct ASIOBufferInfo;
#endif

class AudioChannel;
class AudioHardwareChannel;
class RunningAudioFile;
class Seq_Song;
class AudioSystem;
class AudioHardwareBuffer;
class AudioHDFile;

class AudioHardwareChannel:public Object
{
	friend class AudioDevice;

public:
	AudioHardwareChannel();

	virtual void ClearChannel(int samples,int index){};
	virtual void Init(){}
	virtual ARES *WriteToHardwareBufferData(ARES *from,int samples,int index){return from;}
	virtual void CopyFromHardware(){}
	virtual char *GetHardwareInfo(){return "";}

	void CopyToHardware(Seq_Song *); // I/O Out

	AudioHardwareChannel *NextChannel() {return (AudioHardwareChannel *)next;}

	inline void DropPeak(ARES droprate)
	{
		ARES drop_max=droprate/2;

		if(!p_maxdelaycounter)
		{
			//	if(dropmax==true)
			peakmax>drop_max?peakmax-=drop_max:peakmax=0;
		}
		else
			p_maxdelaycounter--;

		// Drop Input Channel Max
	}

	void SetName(char *);

	AudioHardwareBuffer hwinputbuffer; // input -> ARES -> output channel
	char hwname[128];

	AudioChannel *audiochannel; // 0 default
	AudioHardwareChannel *nextcorechannel,*nextcoreinputchannel;
	AudioDevice *device;

	ARES currentpeak,peakmax;

	int p_maxdelaycounter,
		sizeofsample,
		channelindex, // channel number
		audiochannelgroup,
		recordbufferscounter,
		iotype; // input/output

	bool hardwarechannelused,recordpeakbuffererror,notsupported,
		zerocleared,
		inputused,canbedisabled, // Input Flags
		channelcleared[2];
};

class AudioDevice_WIN32;
class AudioDevice_ASIO;

class AudioHardwareChannel_WIN32Int16LSB:public AudioHardwareChannel
{
public:
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void ClearChannel(int samples,int index);
	void CopyFromHardware();

	char *GetHardwareInfo(){return "16Bit";}
};

// ASIO Channels ...
// ASIO Container Class
class AudioHardwareChannel_ASIO:public AudioHardwareChannel
{
public:
#ifdef WIN32
	ASIOBufferInfo *AddBufferToASIOBuffer(ASIOBufferInfo *);
	ASIOBufferInfo *bufferinfo; // ASIO Doublebuffer 0/1 2 DB Buffer
	//ASIOSampleType asiotype; // sample type
#endif
};

class AudioHardwareChannel_ASIOSTInt16LSB:public AudioHardwareChannel_ASIO
{
public:
	// V
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();

	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(short));}
	char *GetHardwareInfo(){return "Int16LSB";}
};

class AudioHardwareChannel_ASIOSTInt16MSB:public AudioHardwareChannel_ASIO
{
public:
	// V
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(short));}
	char *GetHardwareInfo(){return "Int16MSB";}
};

class AudioHardwareChannel_ASIOSTInt32LSB16:public AudioHardwareChannel_ASIO // xx00 container
{
public:
	// V
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();
	char *GetHardwareInfo(){return "Int32LSB16";}

	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(long));}
};

class AudioHardwareChannel_ASIOSTInt32LSB18:public AudioHardwareChannel_ASIO // xx00 container
{
public:	
	// V
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();
	char *GetHardwareInfo(){return "Int32LSB18";}

	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(long));}
};

class AudioHardwareChannel_ASIOSTInt32LSB20:public AudioHardwareChannel_ASIO // xx00 container
{
public:	
	// V
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();
	char *GetHardwareInfo(){return "Int32LSB20";}

	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(long));}
};

class AudioHardwareChannel_ASIOSTInt32LSB24:public AudioHardwareChannel_ASIO // xx00 container
{
public:
	// V
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();
	char *GetHardwareInfo(){return "Int32LSB24";}

	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(long));}
};

class AudioHardwareChannel_ASIOSTInt24LSB:public AudioHardwareChannel_ASIO
{
public:
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();
	char *GetHardwareInfo(){return "Int24LSB";}

	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*3);}

#ifdef _DEBUG
	void TestConvert();
#endif
};

class AudioHardwareChannel_ASIOSTInt32LSB:public AudioHardwareChannel_ASIO
{
public:
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();
	char *GetHardwareInfo(){return "Int32LSB";}
	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(long));}
};

class AudioHardwareChannel_ASIOSTFloat32LSB:public AudioHardwareChannel_ASIO
{
public:	
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();
	char *GetHardwareInfo(){return "Float32LSB";}
	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(float));}
};

class AudioHardwareChannel_ASIOSTFloat64LSB:public AudioHardwareChannel_ASIO
{
public:	
	ARES *WriteToHardwareBufferData(ARES *from,int samples,int index);
	void CopyFromHardware();
	char *GetHardwareInfo(){return "Float64LSB";}
	void ClearChannel(int samples,int index){memset(bufferinfo->buffers[index],0,samples*sizeof(double));}
};
#endif

/*
// **************************************************************************
// to_float24 : converts 24-bit signed ints to float, simple method          
// **************************************************************************

template<>
struct converter<void,float,sample_bit_type<24> > {
static inline void convert
(void *source, float *target,long frames,sample_bit_type<24>& value) throw()
{
// not extremely fast, but exact and portable
union
{
long lValue;
char cValue[4];
} u;
char *src = (char *)source;
char *dst;

u.lValue = 0;
while (--frames >= 0)
{
#if ASIO_LITTLE_ENDIAN
dst = &u.cValue[1];
#else
dst = &u.cValue[0];
#endif
*dst++ = *src++;
*dst++ = *src++;
*dst++ = *src++;
*target++ = ((u.lValue >> 8) + .5f) * (1.0f / 8388607.75f);
}
}
};

// ****************************************************************************
// from_float24 : converts float to 24-bit signed ints, simple method         
// ****************************************************************************

template<>
struct converter<float,void,sample_bit_type<24> > {
static inline void convert(float *source, void *target, long frames,sample_bit_type<24>) throw()
{
// not extremely fast, but exact and portable
union
{
long lValue;
char cValue[4];
} u;
char *src;
char *dst = (char *)target;
float finter;

while (--frames >= 0)
{
finter = saturate(*source++, 1.0f);
u.lValue = ((long)((finter * 8388607.75f) - .5f)) << 8;

#if ASIO_LITTLE_ENDIAN
src = &u.cValue[1];
#else
src = &u.cValue[0];
#endif
*dst++ = *src++;
*dst++ = *src++;
*dst++ = *src++;
}
}
};
*/