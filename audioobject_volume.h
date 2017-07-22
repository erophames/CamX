#ifndef CAMX_AT_AUDIO_Volume
#define CAMX_AT_AUDIO_Volume 1

#include "audioobjects.h"

class AT_AUDIO_Volume:public AudioObject // Standard Volume Effect
{
public:

	AT_AUDIO_Volume();

	char *GetParmName(int index){return "Audio Volume";}
	char *GetParmValueString(int index);
	char *GetParmTypeValueString(int index);
	char *GetParmValueStringPar(int index,double par);

	void InitSampleRateAndSize(int rate,int buffersize){}
	bool InitIOChannels(int channels){return true;}
	void ResetAudioObject(){value=0.5;} // default 0Db
	
	void DoEffectMaster(AudioEffectParameter *);
	bool DoEffect(AudioEffectParameter *);
	void Delete(bool full);
	void CreateAutomationStartParameters(AutomationTrack *);
	
	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		AT_AUDIO_Volume *n=new AT_AUDIO_Volume;
		
		if(n)
			CloneData(n);
		
		return n;
	}

	AudioObject *InitOpenEffect(){return this;}

	ARES GetDB();

	char valuestring[MAXPLUGINVALUESTRINGLEN+1];
};

#endif