#ifndef CAMX_AUDIOPEAKS_H
#define CAMX_AUDIOPEAKS_H 1

#include "defines.h"

class Peak
{
public:
	Peak()
	{
		channels=0;
		Reset();
	}

	void Reset()
	{
		for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
		{
			p_current[i]=0;
			p_max[i]=0;
			p_maxdelaycounter[i]=0;
		}

		peakused=false;

		absolut_max=0;
		current_sum=0;
		current_max=0;
	}

	void InitPeak(int c);

	void FreeMemory()
	{
		channels=0;
		peakused=false;
	}

	ARES GetCurrent();
	ARES GetCurrentMaximum();
	void ResetMax();
	ARES GetAbsMaximum();
	void ClearPeaks();
	void Drop(ARES droprate);
	void SetChannelPeak(int channel,ARES p);
	
	ARES p_current[MAXCHANNELSPERCHANNEL],p_max[MAXCHANNELSPERCHANNEL],current_sum,current_max,absolut_max;
	int p_maxdelaycounter[MAXCHANNELSPERCHANNEL],channels;
	bool peakused/*,copyied*/;
};
#endif