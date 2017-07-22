/*
#include "audiochannel.h"

class SubTrack_Mute:public AutomationTrack
{	
public:	
	AutomationTrack_Volume()
	{
		
		
		trackvaluestep=0.01;
		
		max=logvolume[LOGVOLUME_SIZE-1];
		min=logvolume[AUDIOMIXER_SILENCE];
	}
	
	char *GetAutomationTrackName(){return "Volume";} //v
	char *GetSubTrackInfo(){return "Audio";} //v
	
	void CreateAutomationStartParameters()
	{
		AutomationObject *s=AddAutomationObject(0,AUTOMATIONOBJECTTYPE_CONNECT);
		
		if(s)
			s->SetValue(1); // +/-
	}
	
	virtual void SendAutoAudioOut(AudioChannel *chl,AutomationObject *ao)
	{
		chl->audioeffects.volume.AutomationEdit(0,ao->GetValue());
	}
};
*/