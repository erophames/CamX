#ifndef CAMX_AUDIOREGION
#define CAMX_AUDIOREGION 1

#include "defines.h"

class AudioHDFile;
class guiWindow;
class Seq_CrossFade;

class AudioRegion:public Object
{
public:
	AudioRegion(){crossfade=0;regionname=0;r_audiohdfile=0;}

	AudioRegion(AudioHDFile *);
	~AudioRegion(){FreeMemory();}

	void CloneTo(AudioRegion *);

	LONGLONG GetLength()
	{
		return regionend-regionstart;
	}

	bool CheckIfInRegion(LONGLONG s,LONGLONG e)
	{
		if((regionstart>=s && regionstart<=e) || (regionstart<=s && regionend>s))return true;
		return false;
	}

	bool Compare(AudioRegion *reg)
	{
		if(regionstart==reg->regionstart && regionend==reg->regionend)return true;
		return false;
	}

	void FreeMemory();

	char *GetName()
	{
		if(regionname)return regionname;
		return "_";
	}

	void Load(camxFile *);
	void Save(camxFile *);

	AudioRegion *NextOrPrev()
	{
		if(next)return (AudioRegion *)next;
		return (AudioRegion *)prev;
	}

	AudioRegion *NextRegion(){return (AudioRegion *)next;}

	int GetUsedCounter();
	bool ChangeRegionPosition(LONGLONG s,LONGLONG e);
	void InitRegion();
	void SetName(char *name,bool refreshgui,guiWindow *ex);

	AudioHDFile *r_audiohdfile;
	Seq_CrossFade *crossfade;
	char *regionname;
	AudioRegion *clonedfrom; // ptr buffer Audio CUT etc
	LONGLONG regionstart,regionend,seekstart,seekend;
	bool autODeInit,destructive,regionseek;
	
};
#endif