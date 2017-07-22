#include "object_project.h"
#include "songmain.h"
#include "audioobject_pan.h"
#include "camxfile.h"
#include "audiohardware.h"
#include "audioeffectparameter.h"
#include "chunks.h"
#include "audiohardwarebuffer.h"
#include "object_song.h"
#include "gui.h"

AT_AUDIO_Panorama::AT_AUDIO_Panorama()
{
	id=AID_AUDIOPAN;
	sysid=SYS_PAN;
	
	ins=outs=2;
	numberofparameter=1;
	curvetype=CT_LINEAR;
	
	ResetAudioObject();
};

void AT_AUDIO_Panorama::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter0005();
}

char *AT_AUDIO_Panorama::GetParmValueStringPar(int index,double par)
{
	switch(channels)
	{
	case 1:
		return "-";
		break;

	case 2:
		{
			if(par==0.5)
				return "C";
			else
				if(par<0.5)
				{
					// left
					// 0-0.49
					double h=0.5-par;
					double h2=0.5;
					h2=h/h2;
					h2*=100;

					char help[NUMBERSTRINGLEN];
					char *l=mainvar->GenerateString("L ",mainvar->ConvertIntToChar((int)h2,help));

					if(l)
					{
						strcpy(valuestring,l);
						delete l;

						return valuestring;
					}
				}
				else
				{
					// right
					// 0.51-1
					double h=par-0.5;
					double h2=0.5;
					h2=h/h2;
					h2*=100;

					char help[NUMBERSTRINGLEN];
					char *l=mainvar->GenerateString(mainvar->ConvertIntToChar((int)h2,help)," R");

					if(l)
					{
						strcpy(valuestring,l);
						delete l;

						return valuestring;
					}
				}
		}
		break;

	default:

#ifdef DEBUG
		maingui->MessageBoxError(0,"Default PAN Channels");
#endif
		break;
	}

#ifdef DEBUG
	maingui->MessageBoxError(0,"PAN <>");
#endif

	return "?";
}

char *AT_AUDIO_Panorama::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

bool AT_AUDIO_Panorama::DoEffect(AudioEffectParameter *par) // virtual
{
	switch(channels)
	{
	case 1:
		if(par->in->channelsused)
		{
			if(par->channel) // No Bus Mono Pan 
				return true;

			ARES mul=par->song->project->panlaw.GetPanValue();

			if(mul==1)
				return true;

			ARES *s=par->in->outputbufferARES;
			int i=par->in->samplesinbuffer;

			if(int loop=i/8)
			{
				i-=8*loop;

				do
				{
					*s++ *=mul;
					*s++ *=mul;
					*s++ *=mul;
					*s++ *=mul;

					*s++ *=mul;
					*s++ *=mul;
					*s++ *=mul;
					*s++ *=mul;

				}while(--loop);
			}

			while(i--) // rest  
			*s++ *=mul;


		}
		break;

	case 2: // STEREO
		if(par->in->channelsused==2)
		{
			ARES *l,*r;
			l=r=par->in->outputbufferARES;
			r+=par->in->samplesinbuffer;

			ARES mleft,mright;

			par->song->project->panlaw.GetValue(&mleft,&mright,value);

			if(mleft==1 && mright==1)
				return true;

			int i=par->in->samplesinbuffer;

			if(int loop=i/8)
			{
				i-=8*loop;

				do
				{
					*l++ *=mleft;*r++ *=mright;
					*l++ *=mleft;*r++ *=mright;
					*l++ *=mleft;*r++ *=mright;
					*l++ *=mleft;*r++ *=mright;

					*l++ *=mleft;*r++ *=mright;
					*l++ *=mleft;*r++ *=mright;
					*l++ *=mleft;*r++ *=mright;
					*l++ *=mleft;*r++ *=mright;

				}while(--loop);
			}

			while(i--) // rest  
			{
				*l++ *=mleft;*r++ *=mright;
			}

			return true;
		}
		break;
	}

	return false;
}

Object *AT_AUDIO_Panorama::Clone()
{
	if(AT_AUDIO_Panorama *n=new AT_AUDIO_Panorama)
	{
		return n;
	}

	return 0;
}
