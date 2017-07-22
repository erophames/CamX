#ifndef CAMX_AUDIOFILE
#define CAMX_AUDIOFILE 1

#include "object.h"

class guiWindow;
class Seq_Song;
class Seq_Track;
class Seq_Pattern;
class AudioChannel;
class AudioHardwareChannel;
class AudioHardwareBuffer;
class AudioPattern;
class RunningAudioFile;
class AudioRealtime;
class guiBitmap;
class guiTimeLine;
class InitPlay;
class RecordingAudioFile;
class AudioHDFile;
class Edit_Arrange;
class AutomationTrack_Volume_Pattern;
class AudioRegion;
class Seq_CrossFade;
class Seq_Pattern_VolumeCurve;

class AudioGFX_Region:public Object
{
public:
	AudioRegion *region;
	int x,x2;
	AudioGFX_Region *Next(){return (AudioGFX_Region*)next;}
};

class PatternVolumePositions{

public:

	enum{
		IN_FADEIN=1,
		IN_VOLUME,
		IN_FADEOUT
	};

	PatternVolumePositions()
	{
		fadeoutondisplay=fadeinondisplay=volumeondisplay=false;
	}

	int CheckXY(int x,int y);
	bool SetMouse(guiWindow *win,int type);
	void InitEdit();

	int fadeinx,fadeiny,fadeinx2,fadeiny2,
		fadeoutx,fadeouty,fadeoutx2,fadeouty2,
		volumex,volumey,volumex2,volumey2;

	bool fadeinondisplay,fadeoutondisplay,volumeondisplay;
};

class AudioGFX
{
public:
	AudioGFX();
	~AudioGFX(){
		regions.DeleteAllO();
	}

	OList regions; // AudioGFX_Region
	guiWindow *win;
	guiBitmap *bitmap;
	guiTimeLine *timeline;
	AudioPattern *audiopattern;
	Seq_CrossFade *crossfade;
	Seq_Pattern_VolumeCurve *patternvolumecurve;
	PatternVolumePositions *patternvolumepositions;

	LONGLONG startposition,regionstart,regionend,samplesperpixel,*ostartx;

	double samplezoom;
	OSTART start,eventstart,eventend;
	int x,y,x2,y2,samplex2,drawcolour,linecolour;
	bool showscale,undermove,showmix,showregion,usebitmap,subpattern,mouseselection,
		drawborder,showregionsinside,dontclearbackground,showline,nonreal,showvolumecurve;
};

#endif