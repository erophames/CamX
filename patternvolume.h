
#ifndef CAMX_PATTERNVOLUME_H
#define CAMX_PATTERNVOLUME_H 1

#include "audioobject_volume.h"

class camxFile;
class AudioHardwareBuffer;

class Seq_Pattern_VolumeCurve
{
public:

	enum{
		VC_LOG8,
		VC_LOG7,
		VC_LOG6,
		VC_LOG5,
		VC_LOG4,
		VC_LOG3,
		VC_LOG2,
		VC_LOG1,

		VC_EXP1,
		VC_EXP2,

		VC_LINEAR,
	};

	Seq_Pattern_VolumeCurve();

	void Clone(Seq_Pattern_VolumeCurve *);
	void InitFadeInOut(double msfadein,double msfadeout,LONGLONG patternsamples=-1);
	double GetFactor(LONGLONG,int *type);
	double GetFactor(LONGLONG);
	double GetVolume();

	void DoEffect(AudioHardwareBuffer *,LONGLONG sampleposition);
	void Load(camxFile *);
	void Save(camxFile *);
	void StartEdit();
	void SetFadeIn(double ms);
	void SetFadeOut(double ms);
	void SetVolume(double);

	AudioPattern *audiopattern;

	LONGLONG fadeinsamples,fadeoutsamples,allsamples,fadeoutstart,
		b_fadeinsamples,b_fadeoutsamples;

	double d_fadeinsamples,d_fadeoutsamples,
		fadeinms,fadeoutms,dbvolume,b_fadeinms,b_fadeoutms,b_dbvolume;

	int fadeintype,fadeouttype,
		b_fadeintype,b_fadeouttype;

	bool init,fadeinoutactive,volumeactive,editmode;

private:
	void Lock();
	void UnLock();
};

#endif
