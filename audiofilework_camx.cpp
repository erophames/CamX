#include "audiofilework.h"

#ifdef CAMX
#include "songmain.h"
#include "audiohardware.h"
#endif

bool AudioFileWork::ConvertRawToFloat_Ex(void *from,ARES *to,int samples)
{
	switch(hd.samplebits)
	{
	case 16: // 16 bit
		{
			// 16 Bit
			short *bit16=(short *)from;

			while(samples--)
			{
				ARES h=*bit16++;

				h/=32768;

				*to++=h;
			}
		}
		return true;

	case 20:
	case 24:  // 24bit audio file ---------------------------------------
		{
			union 
			{
				char help[4];
				long value;
			}u;

#ifdef WIN32
			u.help[3]=0;
#endif
			char *fc=(char *)from;

			while(samples--)
			{
				// Left
#ifdef WIN32
				u.help[0]=*fc++;
				u.help[1]=*fc++;
				u.help[2]=*fc++;						
#endif														
				// 24 linear to signed float -1 <-> 1
				if(u.value>8388608)
					*to++ +=((ARES)(u.value-16777216))/8388608; //-
				else
					*to++ +=((ARES)u.value)/8388608; //+
			}
		}// case 24
		return true;

		// 32 bit float no changes !
	case 32: // 32 bit float ------------------------------------
		return true;

	case 64:
		return true;
	}

#ifdef _DEBUG
	MessageBox(NULL,"Unknown Sample Format Raw->Float","Error",MB_OK);
#endif

	return false; // no sample format found
}

void AudioFileWork::ConvertFloatToRaw_Ex(ARES *from,void *to,int samples)
{
	switch(hd.samplebits)
	{
	case 16:
		{
			// 16 Bit
			short *bit16=(short *)to;

			while(samples--)
			{
				ARES h=*from++;

				if(h>0){
					if(h>=1)
						*bit16++=SHRT_MAX;
					else
#ifdef ARES64
						*bit16++=(short)floor((h*SHRT_MAX)+0.5);
#else
						*bit16++=(short)floor((h*SHRT_MAX)+0.5f);
#endif
				}
				else{	
					if(h<=-1)
						*bit16++=SHRT_MIN;
					else
#ifdef ARES64
						*bit16++=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
						*bit16++=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
				}

				/*
				if(*from>=1)
				*bit16++ =SHRT_MAX;
				else
				if(*from<=-1)
				*bit16++ =SHRT_MIN;
				else
				*bit16++ =(short)(*from*32768);

				from++;
				*/

			}
		}
		break;

	case 20:
	case 24:  // 24bit audio file ---------------------------------------
		{
			union 
			{
				char help[4];
				long value;
			}u;

			char *bit24=(char *)to;
	
			while(samples--)
			{
				if(*from>=1)
					u.value=8388608; //+1
				else
					if(*from<-1)
						u.value=2*8388608; //-1
					else
						if(*from>=0)
							u.value=(long)(*from*8388608); //+
						else
							u.value=(long)(8388608+*from*8388608); //-

				*bit24++=u.help[0];
				*bit24++=u.help[1];
				*bit24++=u.help[2];

				*from++;
			}
		}
		break;

		//32 bits no changes
	}
}

