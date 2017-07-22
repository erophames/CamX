#ifndef CAMX_AUDIOEVENT
#define CAMX_AUDIOEVENT 1

#define LOAD_DONTSETSTARTPOSITION 1

#include "objectevent.h"
#include "camxfile.h"

class AudioEvent:public Seq_AudioEvent
{
	friend AudioPattern;

public:
	AudioEvent();
	Object *Clone(Seq_Song *);
	void CloneData(Seq_Song *,Seq_Event *);

	int GetIndex();
	Seq_Track *GetTrack();

	UBYTE GetStatus() {return AUDIO;}
	void MoveEvent(OSTART diff);
	void MoveEventQuick(OSTART diff);
	void Load(camxFile *);
	void Save(camxFile *);
	bool IsMIDI(){return false;} //v
	bool IsAudio(){return true;} //v
	int Read(void* to,int len);
	void SeekCurrentBytes(int seekbytes);
	void SetToRegion(AudioRegion *);

	void Delete(bool full){delete this;}

	camxFile iofile;
	LONGLONG sampleposition,openseek;

	AudioHDFile *audioefile;
	AudioRegion *audioregion;

	int openseek_cycle;	
	bool destructive,io_region;
};

#endif
