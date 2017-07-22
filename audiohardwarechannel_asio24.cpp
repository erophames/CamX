#include "audiohardwarechannel.h"
#include "asio_device.h"

#define RANGE24_BIT 16777216
#define H24_BIT (RANGE24_BIT/2)

#define H16_DIVMUL (1.0f/32768)
#define H16_DIVMUL_P (1.0f/32767)

// V
ARES *AudioHardwareChannel_ASIOSTInt32LSB::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
#ifdef WIN32
	long *to=(long *)bufferinfo->buffers[index];
	ARES max_m=0,max_p=0;

	// Clip float to signed 32bit
	while(samples--)
	{
		register ARES h=*from++;

		if(h>=1)
		{
			if(h>max_p)max_p=h;
			*to++=LONG_MAX;
		}
		else
			if(h<=-1)
			{
				if(h<max_m)max_m=h;
				*to++=LONG_MIN;
			}
			else
			{
				if(h>max_p)max_p=h;
				else
					if(h<max_m)max_m=h;

#ifdef ARES64
				*to++=(long)floor(h*LONG_MAX+0.5);
#else
				*to++=(long)floor(h*LONG_MAX+0.5f);
#endif

			}
	}
#endif	

	ARES max=(-max_m>max_p)?-max_m:max_p;
	currentpeak=max;
	if(max>peakmax)peakmax=max;

	return from;
}

void AudioHardwareChannel_ASIOSTInt32LSB::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;

	long *from=(long *)bufferinfo->buffers[device->record_bufferindex];
	register ARES mul=1;
	mul/=2147483648;

	int i=device->GetSetSize();

	if(int loop=i/8)
	{
		i-=8*loop;

		do
		{
			ARES h=*to++=(ARES)*from++*mul;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++*mul;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++*mul;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++*mul;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

			h=*to++=(ARES)*from++*mul;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++*mul;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++*mul;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++*mul;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

		}while(--loop);
	}

	while(i--)
	{
		ARES h=*to++=(ARES)*from++*mul;
		if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;
}

ARES *AudioHardwareChannel_ASIOSTInt24LSB::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
	union
	{
		char cValue[4];
		long lValue;
	} u;

	char *to=(char *)bufferinfo->buffers[index];
	register ARES max_m=0,max_p=0;

	// Clip float to signed 24bit
	while(samples--)
	{
		register ARES h=*from++;

		if(h>=1)
		{
			if(h>max_p)max_p=h;
			u.lValue=8388607;
		}
		else
			if(h<=-1)
			{
				if(h<max_m)max_m=h;
				u.lValue=-8388608;
			}
			else
			{
				if(h>max_p)max_p=h;
				else
					if(h<max_m)max_m=h;
#ifdef ARES64
				u.lValue=(long)floor(h*8388607+0.5);
#else
				u.lValue=(long)floor(h*8388607+0.5f);
#endif

			}

			*to++=u.cValue[0];
			*to++=u.cValue[1];
			*to++=u.cValue[2];
	}

	ARES max=(-max_m>max_p)?-max_m:max_p;

	currentpeak=max;
	if(max>peakmax)
		peakmax=max;

	return from;	
}

void  AudioHardwareChannel_ASIOSTInt24LSB::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;

	register ARES mul=1;
	mul/=2147483648;
	unsigned char b24[3];
	char *fc=(char *)bufferinfo->buffers[device->record_bufferindex];

	int i=device->GetSetSize();
	while(i--){

		b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
		ARES h=*to++ =(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
		if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;
}

// V
ARES *AudioHardwareChannel_ASIOSTInt16MSB::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
	/*
	#ifdef WIN32
	char *to=(short *)bufferinfo->buffers[index];
	register ARES h;
	short value;
	union{
	char b[2];
	short value;
	}flip;

	// Clip float to signed 16bit
	while(samples--)
	{
	// Left
	h=*from++;

	if(h>=1)
	flip.value=SHRT_MAX;
	else
	if(h<=-1)
	flip.value=SHRT_MIN;
	else
	flip.value=(short)(h*SHRT_MAX);

	value=flip.b[0]>>8|flip.b[0];
	*to++=value;
	}
	#endif
	*/

	return from;
}


// HW Channels

ARES *AudioHardwareChannel_ASIOSTInt16LSB::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
#ifdef WIN32
	short *to=(short *)bufferinfo->buffers[index];
	ARES max_m=0,max_p=0;

	// Clip float to signed 16bit
	while(samples--)
	{
		// Left
		register ARES h=*from++;

		if(h>0){
			if(h>max_p)max_p=h;

			if(h>=1)
				*to++=SHRT_MAX;
			else
#ifdef ARES64
				*to++=(short)floor((h*SHRT_MAX)+0.5);
#else
				*to++=(short)floor((h*SHRT_MAX)+0.5f);
#endif
		}
		else{
			if(h<max_m)max_m=h;

			if(h<=-1)
				*to++=SHRT_MIN;
			else
#ifdef ARES64
				*to++=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
				*to++=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
		}

		/*
		if(h>0)
		{
		if(h>max_p)max_p=h;

		#ifdef ARES64
		*to++=h>=1?SHRT_MAX:(short)floor(h*SHRT_MAX+0.5);
		#else
		*to++=h>=1?SHRT_MAX:(short)floor(h*SHRT_MAX+0.5f);
		#endif
		}
		else
		{
		if(h<max_m)max_m=h;

		#ifdef ARES64
		*to++=h<=1?SHRT_MIN:(short)floor(h*SHRT_MIN-0.5);
		#else
		*to++=h>=1?SHRT_MIN:(short)floor(h*SHRT_MIN-0.5f);
		#endif
		}
		*/

	}
#endif	

	ARES max=(-max_m>max_p)?-max_m:max_p;

	currentpeak=max;
	if(max>peakmax)peakmax=max;
	return from;
}

void AudioHardwareChannel_ASIOSTInt16LSB::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;

	// 16 bit -> float
	short *from=(short *)bufferinfo->buffers[device->record_bufferindex];
	
	int i=device->GetSetSize();
	
	if(int loop=i/8)
	{
		i-=8*loop;

		do
		{
			ARES h;
			
			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}
			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}
			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}
			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}
			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}
			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}
			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

		}while(--loop);
	}

	while(i--) //rest
	{
		ARES h;
		
		if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
			else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;

}

ARES *AudioHardwareChannel_ASIOSTInt32LSB16::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
	union
	{
		short v1;
		short v2;
		long value;
	}u;

	u.v2=0;

#ifdef WIN32
	long *to=(long *)bufferinfo->buffers[index];
	ARES max_m=0,max_p=0;

	// Clip float to signed 16bit
	while(samples--)
	{
		// Left
		register ARES h=*from++;

		if(h>0){
			if(h>max_p)max_p=h;

			if(h>=1)
				u.v1=SHRT_MAX;
			else
#ifdef ARES64
				u.v1=(short)floor((h*SHRT_MAX)+0.5);
#else
				u.v1=(short)floor((h*SHRT_MAX)+0.5f);
#endif
		}
		else{
			if(h<max_m)max_m=h;

			if(h<=-1)
				u.v1=SHRT_MIN;
			else
#ifdef ARES64
				u.v1=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
				u.v1=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
		}

		/*
		if(h>=1)
		{
			if(h>max_p)max_p=h;
			u.v1=SHRT_MAX;
		}
		else
			if(h<=-1)
			{
				if(h<max_m)max_m=h;
				u.v1=SHRT_MIN;
			}
			else
			{
				if(h>max_p)max_p=h;
				else
					if(h<max_m)max_m=h;

#ifdef ARES64
				u.v1=(short)floor(h*SHRT_MAX+0.5);
#else
				u.v1=(short)floor(h*SHRT_MAX+0.5f);
#endif
			}
*/

			*to++=u.value; // write 16 bit->32 bit container
	}
#endif

	ARES max=(-max_m>max_p)?-max_m:max_p;

	currentpeak=max;
	if(max>peakmax)peakmax=max;

	return from;
}

void AudioHardwareChannel_ASIOSTInt32LSB16::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;

	union
	{
		short v1;
		short v2;
		long value;
	}u;

	u.v2=0;

	long *from=(long *)bufferinfo->buffers[device->record_bufferindex];

	int i=device->GetSetSize();
	while(i--){

		ARES h;
		u.value=*from++;
		if(u.v1>0)h=*to++ =(ARES)u.v1 * H16_DIVMUL_P; else h=*to++ =(ARES)u.v1 * H16_DIVMUL;
		
		if(h<m_peak)m_peak=h;					
		else
			if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;
}

ARES *AudioHardwareChannel_ASIOSTInt32LSB18::WriteToHardwareBufferData(ARES *from,int samples,int index)
{	
#ifdef WIN32
	long *to=(long *)bufferinfo->buffers[index];
	ARES max_m=0,max_p=0;

	// Clip float to signed 18bit
	while(samples--)
	{
		// Left
		register ARES h=*from++;

		if(h>=1)
		{
			if(h>max_p)max_p=h;
			*to++=131071;
		}
		else
			if(h<=-1)
			{
				if(h<max_m)max_m=h;
				*to++=-131072;
			}
			else
			{
				if(h>max_p)max_p=h;
				else
					if(h<max_m)max_m=h;

#ifdef ARES64
				*to++=(long)floor(h*131071+0.5);
#else
				*to++=(long)floor(h*131071+0.5f);
#endif

			}
	}
#endif	

	ARES max=(-max_m>max_p)?-max_m:max_p;
	currentpeak=max;
	if(max>peakmax)peakmax=max;

	return from;
}


void AudioHardwareChannel_ASIOSTInt32LSB18::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;

	union
	{
		short v1;
		short v2;
		long value;
	}u;

	u.v2=0;

	long *from=(long *)bufferinfo->buffers[device->record_bufferindex];

	register ARES mul=1;
	mul/=131072;

	register int i=device->GetSetSize();
	while(i--){
		u.value=*from++;
		ARES h=*to++=(ARES)u.value*mul;

		if(h<m_peak)m_peak=h;					
		else
			if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;
}

ARES *AudioHardwareChannel_ASIOSTInt32LSB20::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
#ifdef WIN32
	long *to=(long *)bufferinfo->buffers[index];
	ARES max_m=0,max_p=0;

	// Clip float to signed 20bit
	while(samples--)
	{
		// Left
		register ARES h=*from++;

		if(h>=1)
		{
			if(h>max_p)max_p=h;
			*to++=524287;
		}
		else
			if(h<=-1)
			{
				if(h<max_m)max_m=h;
				*to++=-524288;
			}
			else
			{
				if(h>max_p)max_p=h;
				else
					if(h<max_m)max_m=h;

#ifdef ARES64
				*to++=(long)floor(h*524287+0.5);
#else
				*to++=(long)floor(h*524287+0.5f);
#endif

			}
	}
#endif	

	ARES max=(-max_m>max_p)?-max_m:max_p;
	currentpeak=max;
	if(max>peakmax)peakmax=max;

	return from;
}

void AudioHardwareChannel_ASIOSTInt32LSB20::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;
	register ARES mul=1;
	mul/=2147483648;

	unsigned char b24[3];
	char *fc=(char *)bufferinfo->buffers[device->record_bufferindex];

	register int i=device->GetSetSize();
	while(i--){

		b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
		fc++; // skip byte 4

		ARES h=*to++ =(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
		if(h<m_peak)m_peak=h;					
		else
			if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;	
}

ARES *AudioHardwareChannel_ASIOSTInt32LSB24::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
#ifdef WIN32
	long *to=(long *)bufferinfo->buffers[index];
	ARES max_m=0,max_p=0;

	// Clip float to signed 24bit
	while(samples--)
	{
		// Left
		register ARES h=*from++;

		if(h>=1)
		{
			if(h>max_p)max_p=h;
			*to++=8388607;
		}
		else
			if(h<=-1)
			{
				if(h<max_m)max_m=h;
				*to++=-8388608;
			}
			else
			{
				if(h>max_p)max_p=h;
				else
					if(h<max_m)max_m=h;

#ifdef ARES64
				*to++=(long)floor(h*8388607+0.5);
#else
				*to++=(long)floor(h*8388607+0.5f);
#endif

			}
	}
#endif	

	ARES max=(-max_m>max_p)?-max_m:max_p;
	currentpeak=max;
	if(max>peakmax)peakmax=max;
	return from;
}

void AudioHardwareChannel_ASIOSTInt32LSB24::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;
	register ARES mul=1;
	mul/=2147483648;

	unsigned char b24[3];
	char *fc=(char *)bufferinfo->buffers[device->record_bufferindex];

	register int i=device->GetSetSize();
	while(i--){

		b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
		fc++; // skip byte 4

		ARES h=*to++ =(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;

		if(h<m_peak)m_peak=h;					
		else
			if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;

}

ARES * AudioHardwareChannel_ASIOSTFloat32LSB::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
	float *to=(float *)bufferinfo->buffers[index];

	ARES max_m=0,max_p=0;

	while(samples--)
	{
		register ARES h=*from++;

		if(h>=1){
			if(h>max_p)max_p=h;
			*to++=1;
		}
		else
			if(h<=-1){
				if(h<max_m)max_m=h;
				*to++=-1;
			}
			else{
				if(h>max_p)max_p=h;
				else 
					if(h<max_m)max_m=h;

				*to++=(float)h;
			}
	}

	ARES max=(-max_m>max_p)?-max_m:max_p;
	currentpeak=max;
	if(max>peakmax)peakmax=max;

	return from;
}

void AudioHardwareChannel_ASIOSTFloat32LSB::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;
	float *from=(float *)bufferinfo->buffers[device->record_bufferindex];
	
	int i=device->GetSetSize();

	if(int loop=i/8)
	{
		i-=8*loop;

		do
		{
			ARES h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

		}while(--loop);
	}

	while(i--)
	{
		ARES h=*to++=(ARES)*from++;
		if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;
}

ARES *AudioHardwareChannel_ASIOSTFloat64LSB::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
	double *to=(double *)bufferinfo->buffers[index];
	ARES max_m=0,max_p=0;

	// Clip float
	while(samples--)
	{
		register ARES h=*from++;

		if(h>=1)
		{
			if(h>max_p)max_p=h;
			*to++=1;
		}
		else
			if(h<=-1)
			{
				if(h<max_m)max_m=h;
				*to++=-1;
			}
			else
			{
				if(h>max_p)max_p=h;
				else
					if(h<max_m)max_m=h;

				*to++=(double)h;
			}
	}	

	ARES max=(-max_m>max_p)?-max_m:max_p;
	currentpeak=max;
	if(max>peakmax)peakmax=max;

	return from;
}

void AudioHardwareChannel_ASIOSTFloat64LSB::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;
	double *from=(double *)bufferinfo->buffers[device->record_bufferindex];

	int i=device->GetSetSize();

	if(int loop=i/8)
	{
		i-=8*loop;

		do
		{
			ARES h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*to++=(ARES)*from++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

		}while(--loop);
	}

	while(i--)
	{
		register ARES h=*to++=(ARES)*from++;
		if(h<m_peak)m_peak=h;					
		else
			if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
	currentpeak=peak;
	if(peak>peakmax)
		peakmax=peak;
}