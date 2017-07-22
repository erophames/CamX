#include "defines.h"
#include "decoder.h"
#include "camxfile.h"
#include "mp3player.h"
#include "songmain.h"

Decoder::Decoder()
{
	file=0;
	fileread=0;
	decodeddata_bytes=0;
	decodeddata_samples=0;

	sum_decodeddata_bytes=0;
	stop=false;
}

void Decoder::Close()
{
	if(file)
		delete file;
	file=0;
}

void Decoder::ConvertToARES(ARES *to,int samples)
{
	int i=channels*samples;

	switch(samplebits)
	{
	case 16:
		{
			short *from=(short *)decodeddata;
			register ARES mul=1,h;
			mul/=32768;

			while(i--){
				h=*from++;
				*to++=h*mul;
			}
		}
		break;

	case 32:
		{
#ifndef ARES64
		memcpy(to,decodeddata,sizeof(float)*i);
#else
		float *from=(float *)decodeddata;

		while(i--)
			*to++=*from++;
#endif
		}
		break;

			case 64:
				{
#ifdef ARES64
		memcpy(to,decodeddata,sizeof(double)*i);
#else
		double *from=(double *)decodeddata;

		while(i--)
			*to++=(ARES)*from++;
#endif
				}
		break;
	}
}

bool Decoder::CheckFile(char *name)
{
	if(!name)
		return false;

	camxFile test;

	if(test.OpenRead(name)==true)
	{
		test.Close(true);

		char *end=0;
		size_t i=strlen(name);

		if(i>2)
		{
			i--;

			char *t=&name[i];

			while(i--)
			{
				if(*t=='.')
				{
					end=t+1;
					break;
				}

				t--;
			}
		}

		if(end)
		{
			char *h=mainvar->GenerateString(end);

			if(h)
			{
				size_t i=strlen(end);

				while(i--)
				{
					h[i]=toupper(h[i]);
				}

				if(strcmp(h,"MP3")==0)
				{
					delete h;
					return true;
				}

				delete h;	
			}	
		}
	}

	return false;
}
