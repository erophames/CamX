#ifndef CAMX_AUDIOMASTER_H
#define CAMX_AUDIOMASTER_H 1

#include "audiodefines.h"
#include "audiochannel.h"
#include "object.h"
#include "editor.h"
#include "object_song.h"
#include "object_track.h"
#include "audiodevice.h"
#include "threads.h"

enum{
	MASTERFORMAT_16BIT,
	MASTERFORMAT_24BIT,
	MASTERFORMAT_32BITFLOAT,
	MASTERFORMAT_64BITFLOAT
};

class Mastering
{
public:
	Mastering();
	void Do();

	// Called by Refill
	void SaveBuffer(MasteringCall *,AudioHardwareBuffer *,bool masterchannel);

	LONGLONG firstsampleposition,endsamples;
	OSTART startticks,endticks,oldsongposition,*masterticks;

	ARES normalizemax;
	char *masterfilename;
	Seq_Song *song;
	Seq_Track *track;
	double *progresspercent,addpausesamples_ms;
	int sampleformat,sampleoffset;
	bool flag_normalize,done,checkforusedsample,addpausesamples;
	
};

class Edit_AudioMaster:public Editor
{
public:
	Edit_AudioMaster(Seq_Song *,Seq_Track *,Seq_Pattern *); // Mastering/Bounce
	Edit_AudioMaster(Seq_Song *); // Freeze

	guiMenu *CreateMenu(); // v

	void MouseButton(int flag);
	void Init();
	void InitClass();
	void FreeAudioHDFile();
	void DeInitWindow();
	void Gadget(guiGadget *);
	
	int ConvertSamplePositionX(LONGLONG);
	LONGLONG ConvertXToPositionX(int x);

	void ShowActiveHDFilePositions();
	void ShowActiveHDFile();
	void SongNameRefresh();
	void InitWindowName();
	void RefreshRealtime(); // v
	void RefreshRealtime_Slow();

	void Play();
	void MouseClickInGFX();
	void ShowMasterChannels();

	Thread masterthread;

	guiGadget_Time *startposition,*endposition;

	guiGadget_Number *pausevalue;
	guiGadget *samplerate,*mastertype,*start,*masterposition,*masterlength,*masterfileclick,*masterfile,
		*sampleformat,*normalize,*pause,*savefirst,*stopsound,*startsound,*statusgadget,*unfreeze,*channels;
	guiGadget_CW *showsound;

	AudioHDFile *masterhdfile;

protected:
	bool CheckMastering();
	
	void StopPlayback();
	void DeleteTestRegion();
	void ShowPeakProgress(double per);
	void ShowLength();
	void StartMastering();
	void StopMastering();
	void ShowStartPosition();
	void InitGadgets();
	void ShowFromTo();
	void ShowMasterPosition(bool force);
	void ShowMasterFile();
	void ShowGadgetStatus();
	void ShowMasterSampleRate();
	void UnFreezeTracks();
	void UnFreezeFrozenTracks();

	// Thread
#ifdef WIN32
	static PTHREAD_START_ROUTINE MasterThread(LPVOID pParam);
#endif

	bool StartThread();
	void StopThread();
	bool CanMastering();

	LONGLONG mastersamples;

	AudioRealtime *audiorealtime;
	AudioRegion *testregion;
	Seq_Song *mastersong;
	Seq_Track *mastertrack;
	Seq_Pattern *masterpattern;

	double masteringpercent,duration_ms;
	LONGLONG filestartposition,frealtimepos;
	OSTART startticks,endticks,masterticks,lastmasterticks;
	int status,r_status,mastersampleformat,frealtimeposx;
	bool masterend,flag_normalize,flag_savefirst,endstatus_realtime,startcanbedone,freeze,flag_pausesamples;
};
#endif