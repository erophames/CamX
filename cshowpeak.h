#ifndef CAMX_CSHOWPEAK_H
#define CAMX_CSHOWPEAK_H 1

#include "defines.h"

class Colour;
class guiBitmap;
class Peak;
class AudioChannel;
class Seq_Track;
class AudioIOFX;
class AudioEffects;

// Pan
class CShowPan
{
public:

	bool CheckPanValues();
	bool ShowPan(bool mouseedit=false);
	bool EditPan(int diff,OSTART time);

	AudioIOFX *GetAudioIO();

	Seq_Song *song;
	AudioChannel *channel;
	Seq_Track *track;
	guiBitmap *bitmap;

	double stereopanv;
	int x,y,x2,y2,colour;
	bool isMIDI;
};

// Peak VU
class CShowPeak
{
public:
	CShowPeak();
	void ShowInit(bool isaudio);
	void ShowPeak();
	void ShowPeakSum();
	void ShowPeakText(ARES current,ARES max);

	AudioIOFX *GetAudioIO();
	AudioEffects *GetFX();
	Peak *GetOutPeak();

	ARES current[MAXCHANNELSPERCHANNEL],max[MAXCHANNELSPERCHANNEL],
		p_outputcurrentpeak[MAXCHANNELSPERCHANNEL],p_outputmaxpeak[MAXCHANNELSPERCHANNEL],
		current_sum,current_max,absolut_max,p_current_sum,p_current_max,p_absolutmax;

	Colour *colour;
	guiBitmap *bitmap;
	Peak *peak;
	AudioChannel *channel;
	Seq_Track *track;

	int channels,x,y,x2,y2,
		xpix[MAXCHANNELSPERCHANNEL],xpix2[MAXCHANNELSPERCHANNEL],

		MIDIinput_data,currentMIDIinput_data,
		MIDIinput,currentMIDIinput,
		MIDIoutput,currentMIDIoutput,
		maxpeakx,maxpeaky,maxpeakx2,maxpeaky2;

	bool active,output,force,changed,disabled,showactivated,inputmonitoring,showMIDI,noMIDItext,maxpeak,horiz;
};

#endif