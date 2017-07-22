#ifndef CAMX_AUDIOPATTERN
#define CAMX_AUDIOPATTERN 1

#define REPLACEAUDIOFILE_CHECKFORREGION 1

#include "defines.h"
#include "object.h"
#include "audioregion.h"
#include "audioevent.h"
#include "objectpattern.h"

class Edit_Arrange;
class AudioDevice;

class AudioPattern:public Seq_Pattern
{	
public:
	AudioPattern();

	void InitOffSetEdit(int mode);
	bool SetOffSetStart(OSTART pos,LONGLONG,bool test);
	bool SetOffSetEnd(LONGLONG,bool test);
	bool IfOffSetStart();
	bool IfOffSetEnd();
	LONGLONG GetOffSetStart();
	LONGLONG GetOffSetEnd();
	Seq_Event *FirstEditEvent(){return &audioevent;}
	LONGLONG GetFilePosition();
	bool SetStart(OSTART);
	bool SetNewLoopStart(OSTART);
	void StopAllofPattern();
	bool SizeAble();
	void InitDefaultVolumeCurve(bool force=false);
	bool CanBeLooped(){return true;}
	void SetAudioMediaTypeAfterRecording();

	// Cross Fades
	ARES GetCrossFadeVolume(LONGLONG pos);
	void RefreshAfterPaste();
	void SelectAudioFile(guiWindow *,char *);
	void ReplaceAudioHDFileWith(AudioHDFile *,AudioRegion *,int flag=0);
	
	void SetName(char *); //v
	void Load(camxFile *);
	void Save(camxFile *);
	void Load_Ex(camxFile *,Seq_Song *); //v
	void Save_Ex(camxFile *); //v

	bool CheckObjectID(int cid)
	{
		if(cid==OBJ_AUDIOPATTERN || cid==OBJ_PATTERN)return true;
		return false;
	}

	int GetAccessCounter(){return accesscounter;}

	Seq_Pattern *CreateLoopPattern(int loop,OSTART pos,int flag); // VF

	void CloneFX(Seq_Pattern *); //VF
	void Clone(Seq_Song *,Seq_Pattern *,OSTART startdiff,int flag); // VF
	Seq_Pattern *CreateClone(OSTART startdiff,int flag); // v

	int GetCountOfEvents(){return 1;} // 1 audio sample

	bool InitPlayback(InitPlay *,int mode); // v
	void Delete(bool full); // v
	void MovePatternData(OSTART diff,int flag); // v

	Seq_Event *FindEventAtPosition(OSTART position,int filter,int icdtype)
	{
		if(GetPatternStart()>=position) // PatternStart=AudioEvent Start
			return &audioevent;

		return 0;
	}

	// Functions
	bool OpenAudioPattern();
	void CloseAudioFile(bool full);
	int QuantizePattern(QuantizeEffect *);
	bool SeekSamplesCurrent(int seeksamples);

	OSTART GetPatternStart(){return audioevent.ostart;}
	OSTART GetPatternEnd(){return GetTickEnd(GetPatternStart());}
	//OSTART GetTickLength();

	int FillAudioBuffer(Seq_Song *,/*AudioDevice *,*/AudioHardwareBuffer *,RunningAudioFile * /*,bool seek*/); // from track
	int FillAudioBuffer(Seq_Song *,AudioHardwareBuffer *); // realtime
	
	int RefreshLoopPositions();

	LONGLONG GetAudioSampleStart();
	LONGLONG GetAudioSampleEnd();
	LONGLONG GetSamples();
	
	OSTART GetTickEnd(OSTART start);

	bool CanBeEdited();

	AudioEvent audioevent;
	AudioRegion *waitforregion,offsetregion;
	char *waitforresamplefile,*waitforresampleendfile;
	RunningAudioFile *runningfile;
	int accesscounter;
	bool waitforresample,internrecorded,punch1,punch2;
};
#endif