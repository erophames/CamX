#ifndef CAMX_audioobject_PAN
#define CAMX_audioobject_PAN 1

#include "audioobjects.h"

#define PANFLAG_MID 0
#define PANFLAG_LEFT 1
#define PANFLAG_RIGHT 2

class AT_AUDIO_Panorama:public AudioObject
{
public:
	AT_AUDIO_Panorama();

	void InitSampleRateAndSize(int rate,int buffersize){}
	bool InitIOChannels(int c)
	{
		channels=c;
		return true;
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		if(AT_AUDIO_Panorama *ap=new AT_AUDIO_Panorama)
		{
			ap->value=value;
			return ap;
		}

		return 0;
	}

	AudioObject *InitOpenEffect(){return this;}

	void ResetAudioObject()
	{
		value=0.5;
	}

	bool SetParm(int index,double par) 
	{
			value=par;
			return true;
	}

	bool DoEffect(AudioEffectParameter *); // v
	void CreateAutomationStartParameters(AutomationTrack *);
	char *GetParmName(int index){return "Audio Pan";}
	char *GetParmValueString(int index);
	char *GetParmValueStringPar(int index,double par);

	void Delete(bool full)
	{
		delete this;
	}

	Object *Clone();

	int channels;

private:
	char valuestring[MAXPLUGINVALUESTRINGLEN+1];
};	

#endif