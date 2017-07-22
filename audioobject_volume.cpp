#include "audioobject_volume.h"
#include "audioauto_volume.h"
#include "audiohardware.h"
#include "guigraphics.h"
#include "songmain.h"
#include "object_track.h"
#include "audiochannel.h"
#include "audiopattern.h"
#include "chunks.h"
#include "colours.h"
#include "object_song.h"
#include "gui.h"

AT_AUDIO_Volume::AT_AUDIO_Volume()
{
	id=AID_AUDIOVOLUME;
	sysid=SYS_VOLUME;

	ins=outs=MAXCHANNELSPERCHANNEL;
	numberofparameter=1;
	curvetype=CT_LINEAR;
	
	ResetAudioObject();
};

char *AT_AUDIO_Volume::GetParmValueStringPar(int index,double par)
{
	char *h=mainaudio->ScaleAndGenerateDBString(par,false);

	if(h)
	{
		if(strlen(h)<MAXPLUGINVALUESTRINGLEN)
			strcpy(valuestring,h);
		else
		{
			strncpy(valuestring,h,MAXPLUGINVALUESTRINGLEN);
			valuestring[MAXPLUGINVALUESTRINGLEN]=0;
		}

		delete h;

		return valuestring;
	}

	return "?";
}

char *AT_AUDIO_Volume::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

char *AT_AUDIO_Volume::GetParmTypeValueString(int index)
{
	return "dB";
}

void AT_AUDIO_Volume::DoEffectMaster(AudioEffectParameter *par)
{
	if(value==0.5)return;

	register ARES mul=(ARES)value;

	mul*=LOGVOLUME_SIZE;

#ifdef ARES64
	mul=logvolume[(int)mul];
#else
	mul=logvolume_f[(int)mul];
#endif

	// Volume
	ARES *s=par->startin->outputbufferARES;
	int i=par->startin->channelsused*par->startin->samplesinbuffer;
	while(i--)*s++ *=mul;
}

bool AT_AUDIO_Volume::DoEffect(AudioEffectParameter *par) // virtual
{
	/*
	if(par->realtime==true)
	{
		Seq_Song *song=par->song;

		if(CanPlayAutomation()==true)
		{
			switch(par->in->channelsused)
			{
			case 0:
				{
					// Skip Automation
					SetValue(GetValueAtPosition(song->audioplayback_endposition));
				}
				break;

			case 1: // Mono
				{
					ARES *s1=par->in->outputbufferARES;
					int samples=par->in->samplesinbuffer;
					LONGLONG sampleposition=song->playback_sampleposition;

					while(samples--)
					{
						ARES multi=GetValueAtSamplePosition(song,sampleposition++);
						value=multi;
						multi*=LOGVOLUME_SIZE;
						multi=logvolume[(int)multi];

						*s1++ *=multi;
					}
				}
				break;

			case 2: // Stereo
				{
					ARES *s1,*s2;
					int samples=par->in->samplesinbuffer;

					s1=s2=par->in->outputbufferARES;
					s2+=par->in->samplesinbuffer;

					LONGLONG sampleposition=song->playback_sampleposition;

					while(samples--)
					{
						ARES multi=GetValueAtSamplePosition(song,sampleposition++);
						value=multi;
						multi*=LOGVOLUME_SIZE;
						multi=logvolume[(int)multi];

						*s1++ *=multi;
						*s2++ *=multi;
					}
				}
				break;

			default:
				{
					ARES *s[MAXCHANNELSPERCHANNEL];
					int channels=par->in->channelsused,samples=par->in->samplesinbuffer;

					for(int i=0;i<channels;i++)
						s[i]=par->in->outputbufferARES+par->in->samplesinbuffer*i;

					LONGLONG sampleposition=song->playback_sampleposition;

					while(samples--)
					{
						ARES multi=GetValueAtSamplePosition(song,sampleposition++);
						value=multi;
						multi*=LOGVOLUME_SIZE;
						multi=logvolume[(int)multi];

						for(int i=0;i<channels;i++)
							*s[i]++ *=multi;
					}
				}
				break;
			}

			return true;
		}
	}
*/

	if(par->in->channelsused==0)
		return false;

	ARES multi=(ARES)GetParm(0);

	multi*=LOGVOLUME_SIZE;

	#ifdef ARES64
	multi=logvolume[(int)multi];
#else
	multi=logvolume_f[(int)multi];
#endif

	if(multi==1)
		return false;

	ARES *s=par->in->outputbufferARES;
	int i=par->in->channelsused*par->in->samplesinbuffer;

	if(int loop=i/8)
	{
		i-=8*loop;

		do{
			*s++ *=multi;
			*s++ *=multi;
			*s++ *=multi;
			*s++ *=multi;

			*s++ *=multi;
			*s++ *=multi;
			*s++ *=multi;
			*s++ *=multi;

		}while(--loop);
	}
	
	while(i--)*s++ *=multi; // rest

	return true;
}

void AT_AUDIO_Volume::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter0005();
}

void AT_AUDIO_Volume::Delete(bool FULL)
{
	//if(automationtrack)automationtrack->objects.CutQObject(this);
	delete this;
}

ARES AT_AUDIO_Volume::GetDB()
{
	ARES h=value;
	h*=LOGVOLUME_SIZE;
	
	#ifdef ARES64
	h=logvolume[(int)h];
#else
	h=logvolume_f[(int)h];
#endif

	return (ARES)mainaudio->ConvertFactorToDb(h);
}