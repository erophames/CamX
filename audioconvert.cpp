#include "defines.h"
#include "audiofile.h"
#include "audiohdfile.h"
#include "audiohardwarechannel.h"
#include "semapores.h"
#include "object_song.h"
#include "guiheader.h"
#include "audiohardware.h"
#include "audiothread.h"
#include "audiorealtime.h"
#include "audioregion.h"
#include "gui.h"

#define RANGE24_BIT 16777216
#define H24_BIT (RANGE24_BIT/2) // 8388608
const ARES H24_DIVMUL=(1.0f/H24_BIT);

#define RANGE20_BIT 16777216

#define H16_DIVMUL (1.0f/32768)
#define H16_DIVMUL_P (1.0f/32767)

#define H20_BIT (RANGE20_BIT/2)
const ARES H20_DIVMUL=1.0f/H20_BIT;
const ARES H32_DIVMUL=1.0f/2147483648;

void AudioHDFile::CloneHeader(AudioHDFile *to)
{
	// ID Header
	to->type=type; // AIFF, wave

	to->channels=channels; // mono, stereo

	to->samplebits=samplebits; // 16,24,32...
	to->samplerate=samplerate;
	to->samplesize_one_channel=samplesize_one_channel; // 16 bit=2,24=3
	to->samplesize_all_channels=samplesize_all_channels; // 16 bit stereo=2*2=4
	to->headerlen=headerlen;
}

void AudioHDFile::ReplaceFileName(char *filename)
{
	//	Close(); // close old file
	Open(filename);
	CreatePeakFile(true);
}

void AudioHDFile::ConvertSampleBufferToRAW(AudioHardwareBuffer *buffer,int buffersize,int channels)
{
	register ARES h;

	if(channels==1)
	{
		ARES *from=buffer->outputbufferARES;
		int samples=buffersize;

		switch(samplebits)
		{
		case 16:
			{
				short *to=(short *)buffer->inputbuffer32bit;

				while(samples--)
				{
					h=*from++;

					if(h>0){
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
						if(h<=-1)
							*to++=SHRT_MIN;
						else
#ifdef ARES64
							*to++=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
							*to++=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
					}

				}
			}
			break;

		case 24:
			{
				char *charto=(char *)buffer->inputbuffer32bit;

				union 
				{
					unsigned char help[4];
					long value;
				}u;

				while(samples--)
				{
					h=*from++;

					if(h==0)u.value=0;
					else if(h>=1)u.value=8388607;
					else if(h<=-1)u.value=-8388608;
					else u.value=(long)(*from*8388608);

					//if(u.value<0)
					//	u.value+=RANGE24_BIT; // -> linear
					*charto++=u.help[0];
					*charto++=u.help[1];
					*charto++=u.help[2];
				}
			}
			break;

		case 32:
			{
				float *to=(float *)buffer->inputbuffer32bit;

				while(samples--){
					h=*from++;

					if(h>=1)*to++=1;
					else if(h<=-1)*to++=-1;
					else *to++=(float)h;
				}
			}
			break;

		case 64:
			{
				double *to=(double *)buffer->inputbuffer32bit;

				while(samples--){
					h=*from++;

					if(h>=1)*to++=1;
					else if(h<=-1)*to++=-1;
					else *to++=(double)h;
				}
			}
			break;
		}
	}
	else // Multichannel
	{
		// [][]-> L/R
		for(int c=0;c<channels;c++)
		{	
			ARES *from=buffer->outputbufferARES;
			from+=buffer->samplesinbuffer*c;

			int samples=buffersize;

			register ARES h;

			switch(samplebits)
			{
			case 16:
				{
					short *to=(short *)buffer->inputbuffer32bit;
					to+=c;
#ifdef _DEBUG
					ARES sum=0;
#endif
					while(samples--){

						h=*from++;

						if(h>0){
							if(h>=1)
								*to=SHRT_MAX;
							else
#ifdef ARES64
								*to=(short)floor((h*SHRT_MAX)+0.5);
#else
								*to=(short)floor((h*SHRT_MAX)+0.5f);
#endif
						}
						else{	
							if(h<=-1)
								*to=SHRT_MIN;
							else
#ifdef ARES64
								*to=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
								*to=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
						}
#ifdef _DEBUG
						sum+=h;
#endif
						to+=channels;
					}
#ifdef _DEBUG
					//TRACE ("CSB Raw Sum %f\n",sum);
#endif
				}
				break;

			case 24:
				{
					char *charto=(char *)buffer->inputbuffer32bit;

					union 
					{
						char help[4];
						long value;
					}u;

					charto+=3*c;

					while(samples--)
					{
						// charto+=c*3;

						if(*from==0)u.value=0;
						else if(*from>=1)u.value=8388607;
						else if(*from<=-1)u.value=-8388608;
						else u.value=(long)(*from*8388608);

						from++;

						//	if(u.value<0)
						//		u.value+=RANGE24_BIT; // -> linear

						*charto++=u.help[0];
						*charto++=u.help[1];
						*charto=u.help[2];

						charto+=3+channels;
					}
				}
				break;

			case 32:
				{
					float *to=(float *)buffer->inputbuffer32bit;
					to+=c;

					while(samples--)
					{
						h=*from++;

						if(h==0)*to=0;
						else if(h>=1)*to=1;
						else if(h<=-1)*to=-1;
						else *to=(float)h;

						to+=channels;
					}
				}
				break;

			case 64:
				{
					double *to=(double *)buffer->inputbuffer32bit;
					to+=c;

					while(samples--)
					{
						h=*from++;

						if(h==0)*to=0;
						else if(h>=1)*to=1;
						else if(h<=-1)*to=-1;
						else *to=h;

						to+=channels;
					}
				}
				break;
			}
		}
	}
}

void AudioHDFile::ConvertReadBufferToSamples(void *io_buffer,AudioHardwareBuffer *buffer,int buffersize,int tochannels,int offset)
{
	if(!channels)
		return;

	if(buffersize<0)
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"ConvertReadBufferToSamples buffersize<0");
#endif

		return;
	}

	if(buffersize+offset>buffer->samplesinbuffer)
	{
		#ifdef DEBUG
		maingui->MessageBoxError(0,"ConvertReadBufferToSamples buffersize+offset ");
#endif

		return;
	}

	if(buffer->channelsused) // Mix it ***************************************************************** 
	{
		if(tochannels==channels) // Same Channels
		{
			switch(channels)
			{
			case 1:
				{
					switch(samplebits)
					{
					case 16:
						{
							ARES *to=buffer->outputbufferARES+offset;
							short *from16=(short *)io_buffer;
							
							while(buffersize--)
								*to++ +=*from16>0?(ARES)*from16++*H16_DIVMUL_P:(ARES)*from16++*H16_DIVMUL;
						}
						break;

					case 20:
					case 24:  // 24bit audio file ---------------------------------------
						{
							ARES *to=buffer->outputbufferARES+offset;
							char *fc=(char *)io_buffer;
							register ARES mul=1;

							switch(samplebits)
							{	
							case 20:
								break;

							case 24:
								mul/=2147483648;
								break;
							}

							unsigned char b24[3];

							while(buffersize--){

								b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
								*to++ +=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
							}
						}
						break;

					case 32:
						{
							if(samplesarefloat==true)
							{
								// 32 bit float ------------------------------------
								ARES *to=buffer->outputbufferARES+offset;
								float *from32=(float *)io_buffer;

								while(buffersize--)*to++ +=(ARES)*from32++; // Mix
							}
							else
							{
								// 32 bit int ------------------------------------
								ARES *to=buffer->outputbufferARES+offset;
								long *from32=(long *)io_buffer;
								register ARES mul=H32_DIVMUL;

								while(buffersize--)*to++ +=(ARES)*from32++*mul; // Mix
							}
						}
						break;

					case 64: 
						{
								// 64 bit float ------------------------------------
							ARES *to=buffer->outputbufferARES+offset;
							double *from64=(double *)io_buffer;

							while(buffersize--)*to++ +=(ARES)*from64++; // Mix
						}
						break;

					}// switch 16,24
				}
				break;

			case 2:
				{
					switch(samplebits)
					{
					case 16:
						{
							ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;	
							short *from16=(short *)io_buffer;

							while(buffersize--){
								
								// Left
								*to++ +=*from16>0?(ARES)*from16++*H16_DIVMUL_P:(ARES)*from16++*H16_DIVMUL;

								// Right
								*to_r++ +=*from16>0?(ARES)*from16++*H16_DIVMUL_P:(ARES)*from16++*H16_DIVMUL;
							}	
						}
						break;

					case 20:
					case 24:  // 24bit audio file ---------------------------------------
						{
							ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;	
							char *fc=(char *)io_buffer;
							register ARES mul=1;
							mul/=2147483648;

							unsigned char b24[3];

							while(buffersize--){

								b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
								*to++ +=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;

								b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
								*to_r++ +=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
							}
						}// case 24
						break;

					case 32: 
						{
							if(samplesarefloat==true)
							{
								// 32 bit float ------------------------------------
								ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;
								float *from32=(float *)io_buffer;

								while(buffersize--){	
									// Left
									*to++ +=*from32++;
									// Right
									*to_r++ +=*from32++;
								}
							}
							else
							{
								// 32 bit int ------------------------------------
								ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;
								long *from32=(long *)io_buffer;
								register ARES mul=H32_DIVMUL;

								while(buffersize--){	
									// Left
									*to++ +=*from32++*mul;
									// Right
									*to_r++ +=*from32++*mul;
								}
							}

						}
						break;

					case 64: // 64 bit float ------------------------------------
						{
							ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;
							double *from32=(double *)io_buffer;

							while(buffersize--){	
								// Left
								*to++ +=(ARES)*from32++;
								// Right
								*to_r++ +=(ARES)*from32++;
							}
						}
						break;

					}// switch 16,24
				}
				break;

			default: // Else Channels
				{
					for(int c=0;c<channels;c++) // X Channels...
					{
						int i=buffersize;

						switch(samplebits)
						{
						case 16:
							{
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;

								//register ARES mul=H16_DIVMUL;
								short *from16=(short *)io_buffer;
								from16+=c;

								while(i--){
									*to++ +=*from16>0?(ARES)*from16*H16_DIVMUL_P:(ARES)*from16*H16_DIVMUL;
									from16+=channels;
								}	
							}
							break;

						case 20:
						case 24:  // 24bit audio file ---------------------------------------
							{
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;

								char *fc=(char *)io_buffer;
								fc+=3*c;
								int add=3*channels;
								unsigned char b24[3];
								register ARES mul=1;
								mul/=2147483648;

								while(i--){

									b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc;
									fc+=add;
									*to++ +=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;

								}
							}// case 24
							break;

						case 32: 
							{
								if(samplesarefloat==true)
								{
									// 32 bit float ------------------------------------

									ARES *to=buffer->outputbufferARES+offset;
									to+=c*buffer->samplesinbuffer;

									float *from32=(float *)io_buffer;
									from32+=c;

									while(i--){	
										*to++ +=(ARES)*from32;
										from32+=channels;	
									}
								}
								else
								{
									// 32 bit int ------------------------------------

									ARES *to=buffer->outputbufferARES+offset;
									to+=c*buffer->samplesinbuffer;

									long *from32=(long *)io_buffer;
									from32+=c;

									register ARES mul=H32_DIVMUL;

									while(i--){	
										*to++ +=(ARES)*from32*mul;
										from32+=channels;	
									}
								}
							}
							break;

						case 64: // 64 bit float ------------------------------------
							{
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;

								double *from32=(double *)io_buffer;
								from32+=c;

								while(i--){	
									*to++ +=(ARES)*from32;
									from32+=channels;	
								}	
							}
							break;

						}// switch 16,24

						i=buffersize;

					} // for channels copy tochannels==channels
				}
				break;
			}

		}// if tochannels==channels
		else
			if(tochannels==1 && channels==2) // Stereo -> Mono
			{
				switch(samplebits)
				{
				case 16:
					{
						ARES *to=buffer->outputbufferARES+offset;
						short *from16=(short *)io_buffer;
						//register ARES mul=H16_DIVMUL,h;

						while(buffersize--){

							ARES h=*from16++; // L
							h+=*from16++; // +R

							*to++ +=h*H16_DIVMUL;
						}	
					}
					break;

				case 20:
				case 24:  // 24bit audio file ---------------------------------------
					{
						ARES *to=buffer->outputbufferARES+offset;
						char *fc=(char *)io_buffer;
						register ARES mul=1,h;
						mul/=2147483648;

						unsigned char b24[3];

						while(buffersize--){

							b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
							h =(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8));

							b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
							h +=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8));

							*to++ +=(ARES)h*mul;
						}
					}
					break;

				case 32: 
					{
						if(samplesarefloat==true)
						{
							// 32 bit float ------------------------------------
							ARES *to=buffer->outputbufferARES+offset;
							float *from32=(float *)io_buffer,h;

							while(buffersize--){
								h=*from32++;
								h+=*from32++;

								*to++ +=(ARES)h;
							}
						}
						else
						{
							// 32 bit int ------------------------------------
							ARES *to=buffer->outputbufferARES+offset;
							long *from32=(long *)io_buffer,h;
							register ARES mul=H32_DIVMUL;

							while(buffersize--){
								h=*from32++;
								h+=*from32++;

								*to++ +=(ARES)h*mul;
							}
						}
					}
					break;

				case 64: // 64 bit float ------------------------------------
					{
						ARES *to=buffer->outputbufferARES+offset;
						double *from32=(double *)io_buffer,h;

						while(buffersize--){
							h=*from32++;
							h+=*from32++;
							*to++ +=(ARES)h;
						}
					}
					break;

				}// switch 16,24
			}
			else
				if(tochannels==2 && channels==1) // Expand Mix Mono->Stereo (Mix)
				{	
					for(int c=0;c<2;c++)
					{
						int i=buffersize;

						switch(samplebits)
						{
						case 16:
							{
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;

								short *from16=(short *)io_buffer;
								//register ARES mul=H16_DIVMUL;

								while(i--)
									*to++ +=*from16>0?(ARES)*from16++*H16_DIVMUL_P:(ARES)*from16++*H16_DIVMUL;
							}
							break;

						case 20:
						case 24:  // 24bit audio file ---------------------------------------
							{	
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;

								char *fc=(char *)io_buffer;
								register ARES mul=1;
								mul/=2147483648;

								unsigned char b24[3];

								while(i--){
									b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
									*to++ +=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
								}

							}// case 24
							break;

						case 32: 
							{
								if(samplesarefloat==true)
								{
									// 32 bit float ------------------------------------
									ARES *to=buffer->outputbufferARES+offset;
									to+=c*buffer->samplesinbuffer;

									float *from32=(float *)io_buffer;

									while(i--)
										*to++ +=(ARES)*from32++;
								}
								else
								{
									// 32 bit int ------------------------------------
									ARES *to=buffer->outputbufferARES+offset;
									to+=c*buffer->samplesinbuffer;

									long *from32=(long *)io_buffer;
									register ARES mul=H32_DIVMUL;

									while(i--)
										*to++ +=(ARES)*from32++*mul;
								}
							}
							break;

						case 64: // 32 bit float ------------------------------------
							{
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;

								double *from32=(double *)io_buffer;

								while(i--)
									*to++ +=(ARES)*from32++;
							}
							break;

						}// switch 16,24
					} // for -expand mix
				}

	}
	else // if used - Empty Buffer Copy ***************************************************************** 
	{
		buffer->channelsused=tochannels;

		if(tochannels==channels)
		{
			if(channels==1)
			{
				switch(samplebits)
				{
				case 16:
					{
						ARES *to=buffer->outputbufferARES+offset;
						short *from16=(short *)io_buffer;
						
						while(buffersize--)
							*to++ =*from16>0?(ARES)*from16++*H16_DIVMUL_P:(ARES)*from16++*H16_DIVMUL;
					}
					break;

				case 20:
				case 24:  // 24bit audio file ---------------------------------------
					{
						ARES *to=buffer->outputbufferARES+offset;
						char *fc=(char *)io_buffer;
						register ARES mul=1;
						mul/=2147483648;

						unsigned char b24[3];

						while(buffersize--){

							b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
							*to++ =(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
						}
					}
					break;

				case 32: // 32 bit float ------------------------------------
					{
						if(samplesarefloat==true)
						{
							ARES *to=buffer->outputbufferARES+offset;
#ifndef ARES64
							memcpy(to,io_buffer,sizeof(ARES)*buffersize);
#else
							float *from32=(float *)io_buffer;

							while(buffersize--)	
								*to++ =*from32++;
#endif
						}
						else
						{
							ARES *to=buffer->outputbufferARES+offset;
							long *from32=(long *)io_buffer;
							register ARES mul=H32_DIVMUL;

							while(buffersize--)	
								*to++ =*from32++*mul;
						}
					}
					break;

				case 64: // 32 bit float ------------------------------------
					{
						ARES *to=buffer->outputbufferARES+offset;
#ifdef ARES64
						memcpy(to,io_buffer,sizeof(ARES)*buffersize);
#else
						double *from32=(double *)io_buffer;

						while(buffersize--)	
							*to++ =(ARES)*from32++;
#endif
					}
					break;

				}// switch 16,24
			}
			else
				if(channels==2)
				{
					switch(samplebits)
					{
					case 16:
						{
							ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;
							short *from16=(short *)io_buffer;
							
							while(buffersize--){
								// Left
								*to++ =*from16>0?(ARES)*from16++*H16_DIVMUL_P:(ARES)*from16++*H16_DIVMUL;
								// Right
								*to_r++  =*from16>0?(ARES)*from16++*H16_DIVMUL_P:(ARES)*from16++*H16_DIVMUL;
							}
						}
						break;

					case 20:
					case 24:  // 24bit audio file ---------------------------------------
						{
							ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;	
							char *fc=(char *)io_buffer;
							register ARES mul=1;
							mul/=2147483648;

							unsigned char b24[3];

							while(buffersize--){

								b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
								*to++=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;

								b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
								*to_r++=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
							}
						}// case 24
						break;

					case 32: 
						{
							if(samplesarefloat==true)
							{
								// 32 bit float ------------------------------------
								ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;
								float *from32=(float *)io_buffer;

								while(buffersize--){	
									*to++ =(ARES)*from32++;
									*to_r++ =(ARES)*from32++;
								}
							}
							else
							{
								// 32 bit signed ------------------------------------
								ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;
								long *from32=(long *)io_buffer;

								register ARES mul=H32_DIVMUL;

								while(buffersize--){	
									*to++ =(ARES)*from32++*mul;
									*to_r++ =(ARES)*from32++*mul;
								}
							}
						}
						break;

					case 64: // 64 bit float ------------------------------------
						{
							ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;
							double *from32=(double *)io_buffer;

							while(buffersize--){	
								*to++ =(ARES)*from32++;
								*to_r++ =(ARES)*from32++;
							}
						}
						break;

					}// switch 16,24
				}
				else // else X Channels
				{
					for(int c=0;c<channels;c++)
					{
						int i=buffersize;

						switch(samplebits)
						{
						case 16:
							{
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;
								short *from16=(short *)io_buffer;
								from16+=c;

							//	register ARES mul=H16_DIVMUL;

								while(i--) {
									*to++ =*from16>0?(ARES)*from16*H16_DIVMUL_P:(ARES)*from16*H16_DIVMUL;
									from16+=channels;
								}
							}
							break;

						case 20:
						case 24:  // 24bit audio file ---------------------------------------
							{
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;

								char *fc=(char *)io_buffer;
								fc+=3*c;
								int add=3*channels;

								register ARES mul=1;
								mul/=2147483648;

								unsigned char b24[3];

								while(i--){

									b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc;
									fc+=add;
									*to++=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
								}

							}// case 24
							break;

						case 32: // 32 bit float ------------------------------------
							{
								if(samplesarefloat==true)
								{
									ARES *to=buffer->outputbufferARES+offset;
									to+=c*buffer->samplesinbuffer;

									float *from32=(float*)io_buffer;
									from32+=c;

									while(i--){	
										*to++ =(ARES)*from32;
										from32+=channels;	
									}
								}
								else
								{
									ARES *to=buffer->outputbufferARES+offset;
									to+=c*buffer->samplesinbuffer;

									long *from32=(long*)io_buffer;
									from32+=c;

									register ARES mul=H32_DIVMUL;

									while(i--){	
										*to++ =(ARES)*from32*mul;
										from32+=channels;	
									}
								}

							}
							break;

						case 64: // 32 bit float ------------------------------------
							{
								ARES *to=buffer->outputbufferARES+offset;
								to+=c*buffer->samplesinbuffer;

								double *from32=(double*)io_buffer;
								from32+=c;

								while(i--){	
									*to++ =(ARES)*from32;
									from32+=channels;	
								}
							}
							break;

						}// switch 16,24

					} // for channels copy
				}
		}
		else
			if(tochannels==1 && channels==2) // Stereo -> Mono
			{
				switch(samplebits)
				{
				case 16:
					{
						ARES *to=buffer->outputbufferARES+offset;
						short *from16=(short *)io_buffer;
						register ARES /*mul=H16_DIVMUL,*/l,r;

						// Add Law
						//mul*=mMIX;

						while(buffersize--){
							l=*from16++; // L
							r=*from16++; // +R
							*to++ =(l+r)*(H16_DIVMUL*mMIX);
						}	
					}
					break;

				case 20:
				case 24:  // 24bit audio file ---------------------------------------
					{
						ARES *to=buffer->outputbufferARES+offset;
						char *fc=(char *)io_buffer;
						register ARES mul=1,l,r;
						mul/=2147483648;

						unsigned char b24[3];

						// Add Law
						mul*=mMIX;

						while(buffersize--){

							b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
							l =(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8));

							b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
							r =(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8));

							*to++ =(l+r)*mul;
						}
					}
					break;

				case 32: // 32 bit float ------------------------------------
					{
						if(samplesarefloat==true)
						{
							ARES *to=buffer->outputbufferARES+offset;
							float *from32=(float *)io_buffer,l,r;

							// Add Law
							while(buffersize--){
								l=*from32++;
								r=*from32++;
								*to++ =(l+r)*mMIX;
							}
						}
						else
						{
							ARES *to=buffer->outputbufferARES+offset;
							long *from32=(long *)io_buffer,l,r;

							register ARES mul=H32_DIVMUL;

							// Add Law
							mul*=mMIX;

							while(buffersize--){
								l=*from32++;
								r=*from32++;
								*to++ =(l+r)*mul;
							}
						}
					}
					break;

				case 64: // 32 bit float ------------------------------------
					{
						ARES *to=buffer->outputbufferARES+offset;
						double *from32=(double *)io_buffer,h;

						while(buffersize--){
							h=*from32++;
							h+=*from32++;
							*to++ =(ARES)h*mMIX;
						}
					}
					break;
				}// switch 16,24
			}
			else
				if(tochannels==2 && channels==1) // Expand  Mono->Stereo
				{
					ARES *to=buffer->outputbufferARES+offset,*to_r=to+buffer->samplesinbuffer;
					
					//for(int c=0;c<2;c++){

					int i=buffersize;

					switch(samplebits)
					{
					case 16:
						{
							short *from16=(short *)io_buffer;
						//	register ARES mul=H16_DIVMUL;

							while(i--)
								*to++ =*to_r++ =*from16>0?(ARES)*from16++*H16_DIVMUL_P:(ARES)*from16++*H16_DIVMUL;	
						}
						break;

					case 20:
					case 24:  // 24bit audio file ---------------------------------------
						{
							register ARES mul=1;
							mul/=2147483648;

							unsigned char b24[3];
							char *fc=(char *)io_buffer;

							while(i--){

								b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;
								*to++ =*to_r++=(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
							}
						}
						break;

					case 32: // 32 bit float ------------------------------------
						{
							if(samplesarefloat==true)
							{
#ifndef ARES64
								size_t size=sizeof(float)*i;

								memcpy(to,io_buffer,size);
								memcpy(to_r,io_buffer,size);
#else
								float *from32=(float *)io_buffer;
								while(i--)
									*to++ =*to_r++=*from32++;
#endif
							}
							else
							{
								register ARES mul=H32_DIVMUL;
								long *from32=(long *)io_buffer;
								while(i--)
									*to++ =*to_r++=(ARES)*from32++*mul;
							}

						}
						break;

					case 64: // 64 bit float ------------------------------------
						{
#ifdef ARES64
							size_t size=sizeof(double)*i;

							memcpy(to,io_buffer,size);
							memcpy(to_r,io_buffer,size);
#else
							double *from32=(double *)io_buffer;
							while(i--)
								*to++ =*to_r++=(ARES)*from32++;
#endif
						}
						break;

					}// switch 16,24

				} // for channels copy tochannels==channels

	}

#ifdef _DEBUG
	buffer->CheckBuffer();
#endif
}