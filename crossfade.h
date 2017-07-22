#ifndef CAMX_CROSSFADES_H
#define CAMX_CROSSFADES_H 1

#include "object.h"

class camxFile;
class Seq_Pattern;

class Seq_CrossFade:public Object
{
public:
	Seq_CrossFade(){
		connectwith=0;
		used=false;
		type=CFTYPE_SIN1;
		deletethis=false;
		dontdeletethis=false;
		pattern=0;
	}

	enum CF_Type{
		CFTYPE_SIN1,
		CFTYPE_SIN2,
		CFTYPE_SIN3,
		CFTYPE_LINEAR,
		CFTYPE_CURVE,
		CFTYPE_COS1,
		CFTYPE_MAX, // 1
		CFTYPE_OFF, // 0
	};

	void Load(camxFile *);
	void Save(camxFile *);
	void Toggle();
	void RefreshCrossFadeGUI();
	void SetCrossFadeSamplePositions();
	bool ChangeType(int);
	bool CheckIfInRange(LONGLONG start,LONGLONG end);
	bool CheckIfInRange_File(LONGLONG start,LONGLONG end);

	Object *Clone();
	void CopyData(Seq_CrossFade *);

	ARES ConvertToVolume(double,bool in,int checktype=-1); // 0-1
	void DeInit(){}
	Seq_CrossFade *NextCrossFade(){return (Seq_CrossFade *)next;}

	Seq_Pattern *pattern;
	Seq_CrossFade *connectwith;
	OSTART from,to;
	LONGLONG from_sample,to_sample ,from_sample_file,to_sample_file; // samples
	int type; // ticks
	bool infade,used,deletethis,dontdeletethis,patterninsong;
};

#endif