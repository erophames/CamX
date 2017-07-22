#include "audiohardwarechannel.h"
#include "songmain.h"
#include "audioports.h"

extern char *channelchannelsinfo[],*channelchannelsinfo_short[];
extern int channelschannelsnumber[];

AudioPort::AudioPort()
{
	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)hwchannel[i]=0;
	name=0;
	iotype=AP_UNDEF;

	hwchannelchanged=false; // Sync with Settings Editor
	visible=false;
}

void AudioPort::GenerateName()
{
	if(name)delete name;
	name=0;

	if(channels==0)
	{
		name=mainvar->GenerateString("00");
		return;
	}

	char *type="?";

	switch(iotype)
	{
	case AP_INPUT:
		type="i";
		break;

	case AP_OUTPUT:
		type="o";
		break;
	}
	char *h3=0;

	char *sum=0;

	for(int c=0;c<channels;c++)
	{
		char *h2;

		if((!hwchannel[c]) || strlen(hwchannel[c]->hwname)==0)
			h2="-";
		else
		{
			h2=hwchannel[c]->hwname;
		}

		if(!h3)
			h3=mainvar->GenerateString(h2);
		else
		{
			// Compare in h3
	
			char *c=h2;
			char *ct=h3;
			size_t i=strlen(h2);
			size_t i2=strlen(h3);

			while(i-- && i2--)
			{
				if(*ct==' ')
				{
					if(i2>0)
						c++;

					goto add;
				}

				if(*c!=*ct)
					goto add;

				c++;
				ct++;
			}

			if(!i2)
				c=h2; // full

add:
			char *t=mainvar->GenerateString(h3,"+",c);
			delete h3;
			h3=t;
		}
	}

	char *h4=mainvar->GenerateString("(");

	if(h3)
	{

		char nr[NUMBERSTRINGLEN];

		for(int c=0;c<channels;c++)
		{
			if(hwchannel[c])
			{
				char *n=mainvar->ConvertIntToChar(hwchannel[c]->channelindex+1,nr);

				char *hh;

				if(c>0)
					hh=mainvar->GenerateString(h4,"+",n);
				else
					hh=mainvar->GenerateString(h4,n);

				delete h4;

				h4=hh;
			}
		}

		char *hh=mainvar->GenerateString(h4,"):");
		delete h4;

		h4=hh;
	}

		char ht[NUMBERSTRINGLEN];

	name=mainvar->GenerateString(type,"P",mainvar->ConvertIntToChar(portindex+1,ht)," ",channelchannelsinfo_short[channels-1],h4,h3);

	if(h4)delete h4;
}

void AudioPort::MixVInputToBuffer(AudioHardwareBuffer *tobuffer,int tochannels,Peak *peak)
{
	if(tobuffer->channelsinbuffer>=tochannels && hwchannel[0]->hwinputbuffer.channelsused>0)
	{
		// Peak
		for(int i=0;i<channels;i++)
		{
			if(hwchannel[i]->currentpeak>peak->p_current[i])
			{
				peak->p_current[i]=hwchannel[i]->currentpeak;
				peak->peakused=true;
			}

			if(hwchannel[i]->peakmax>peak->p_max[i])
			{
				peak->p_max[i]=hwchannel[i]->peakmax;
				peak->peakused=true;
			}
		}

		ARES *to=tobuffer->outputbufferARES;

		if(tochannels>channels) // Expand ?
		{
			int fromc=0;
			for(int c=0;c<tochannels;c++)
			{
				if(hwchannel[fromc])
				{
					if(c<tobuffer->channelsused){
						// Mix
						ARES *from=hwchannel[fromc]->hwinputbuffer.outputbufferARES;
						
						int i=tobuffer->samplesinbuffer;

						if(int loop=i/8)
						{
							i-=8*loop;

							do
							{
								*to++ += *from++;
								*to++ += *from++;
								*to++ += *from++;
								*to++ += *from++;

								*to++ += *from++;
								*to++ += *from++;
								*to++ += *from++;
								*to++ += *from++;

							}while(--loop);
						}

						while(i--)*to++ += *from++; // rest
					}
					else{
						// Copy
						memcpy(to,hwchannel[fromc]->hwinputbuffer.outputbufferARES,tobuffer->samplesinbuffer_size);
						to+=tobuffer->samplesinbuffer;
					}
				}

				if(fromc+1<channels)fromc++;				
			}
		}
		else
		{
			if(tobuffer->channelsused==0)
			{
				for(int i=0;i<channels;i++)
				{
					if(hwchannel[i])
						memcpy(to,hwchannel[i]->hwinputbuffer.outputbufferARES,tobuffer->samplesinbuffer_size);

					to+=tobuffer->samplesinbuffer;
				}
			}
			else
				if(tobuffer->channelsused>=channels) // mix
				{
					for(int c=0;c<channels;c++)
						if(hwchannel[c]){
							ARES *from=hwchannel[c]->hwinputbuffer.outputbufferARES;
							int i=tobuffer->samplesinbuffer;

							if(int loop=i/8)
							{
								i-=8*loop;

								do
								{
									*to++ += *from++;
									*to++ += *from++;
									*to++ += *from++;
									*to++ += *from++;

									*to++ += *from++;
									*to++ += *from++;
									*to++ += *from++;
									*to++ += *from++;

								}while(--loop);
							}

							while(i--)*to++ += *from++; //rest
						}
				}
				else // 2-1
				{
					for(int c=0;c<tobuffer->channelsused;c++)
						if(hwchannel[c]){
							ARES *from=hwchannel[c]->hwinputbuffer.outputbufferARES;
							int i=tobuffer->samplesinbuffer;

							if(int loop=i/8)
							{
								i-=8*loop;

								do
								{
									*to++ += *from++;
									*to++ += *from++;
									*to++ += *from++;
									*to++ += *from++;

									*to++ += *from++;
									*to++ += *from++;
									*to++ += *from++;
									*to++ += *from++;

								}while(--loop);
							}

							while(i--)*to++ += *from++; //rest
						}

						// rest cpy
						for(int i=tobuffer->channelsused;i<channels;i++)
						{
							if(hwchannel[i])
								memcpy(to,hwchannel[i]->hwinputbuffer.outputbufferARES,tobuffer->samplesinbuffer_size);

							to+=tobuffer->samplesinbuffer;
						}
				}
		}

		if(tobuffer->channelsused<tochannels)
			tobuffer->channelsused=tochannels;
	}
}

int AudioPort::GetRecordBits()
{
	int bits=0;

	for(int i=0;i<channels;i++)
	{
		if(hwchannel[i])
		{
			if(hwchannel[i]->sizeofsample*8>bits)
				bits=hwchannel[i]->sizeofsample*8;
		}

	}

	return bits;
}

