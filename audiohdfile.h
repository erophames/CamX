#ifndef CAMX_AUDIOHDFILE
#define CAMX_AUDIOHDFILE 1

#include "object.h"
#include "camxfile.h"
#include "audiohardwarebuffer.h"

class AudioPattern;
class AudioRegion;
class AudioGFX;
class AudioHardwareChannel;
class AudioChannel;
class guiWindow;
class AudioRealtime;
class Seq_Song;
class Seq_Track;
class AudioPeakBuffer;
class Progress;
class Directory;

#define SAVE_NOREGIONS 1

#define OPENREADFILE_ERROR 1
#define OPENSAVEFILE_ERROR 2
#define AUDIOFILECHECK_ERROR 4
#define AUDIOFILENAME_ERROR 8
#define CHUNK_ERROR 16

#define OPENAUDIOHD_NOPEAK 1
#define OPENAUDIOHD_JUSTINIT 4

class AudioHDFile:public ObjectLock
{
	enum
	{
		OPENSAVE_OPENREADSAVE=1,
	};

	friend class mainAudio;
	friend class AudioPattern;

public:
	AudioHDFile();
	~AudioHDFile(){FreeMemory();}

	virtual void DeleteData(){}

	void Decoder(AudioHardwareBuffer *);
	void Decoder(char *buffer,int samples);

	char *GetDragDropInfoString();
	char *ErrorString();
	void CloneHeader(AudioHDFile *);
	void ReplaceFileName(char *);
	LONGLONG FindZeroSamples(LONGLONG samplespos);
	LONGLONG GetStartOfSample(LONGLONG sample);
	
	void CreateFileName();
	bool LoadHDFile(camxFile *,bool open);
	void Load(camxFile *,CPOINTER *old,Seq_Song *song=0); //v
	void Save(camxFile *); // v
	void CreatePeak();
	void Open(int flag=0);
	char *GetName(){return name?name:"?";}
	char *GetFileName(){return filename?filename:"#?";}
	char *SetName(char *);
	void Open(char *); // r:erroflag
	void InitHDFile(); //r:errorflag
	void FreeMemory();
	void ConvertReadBufferToSamples(void *io_buffer,AudioHardwareBuffer *,int buffersize,int channels,int offset=0);
	void ConvertSampleBufferToRAW(AudioHardwareBuffer *,int buffersize,int channels);
	void AudioFileDeleted();
	void CreatePeakFile(bool withnewcreate);
	AudioPeakBuffer *OpenPeakFile(char *fname,char *peakfilename);
	bool DeleteFileOnHD();
	bool Normalize(ARES maxv,Progress *prog=0);
	bool ShowAudioFile(AudioGFX *);
	bool ShowAudioFile(Seq_Song *,AudioGFX *);

	void StopRecording(Seq_Song *,int *added,int *deleted,bool writeerrorfiles);
	void EndAndAddAudioRecording(Seq_Song *);
	void ConvertARESToRecordingFormat();
	void WriteRecordingFile(AudioDevice *,Seq_Song *);
	void WriteAudioRecording(AudioDevice *,Seq_Song *);
	
	AudioHDFile *NextHDFile();
	AudioHDFile *NextHDFileNoLock(){return (AudioHDFile *)next;}

	AudioRealtime *PlayRealtime(guiWindow *,Seq_Song *,AudioRegion *,AudioChannel *,bool*endstatus,Seq_Track *usetrack=0,bool autoclose=false);
	AudioHDFile *CreateClone();

	// Regions
	AudioRegion *AddRegion(AudioRegion *,bool force=true);
	//void AddVRegion(AudioRegion *);
	AudioRegion *DeleteRegion(AudioRegion *);
	//AudioRegion *DeleteVRegion(AudioRegion *);
	void FreeRegions();
	inline AudioRegion *FirstRegion() {return (AudioRegion *)regions.GetRoot();}
//	inline AudioRegion *FirstVRegion() {return (AudioRegion *)vregions.GetRoot();}
	int GetCountRegions(){return regions.GetCount();}
	AudioRegion *FindRegion(LONGLONG start,LONGLONG end);
	AudioRegion *FindRegion(AudioRegion *);
	AudioRegion *FindRegionInside(LONGLONG start,LONGLONG end);
	
	AudioRegion *GetRegionIndex(int index){return (AudioRegion *)regions.GetO(index);}	
	void WriteHeader();
	void WriteWaveHeader();
	void SaveBuffer(AudioHardwareBuffer *,int buffersize,void *to,int format,bool *iscleared,bool masterchannel); // Mastering
	void InitAudioFileSave(Seq_Track *,int flag=0);
	bool InitAudioFileSave(char *,int flag=0);
	
	void CreateRecordPeak(int offset,int writesamples,int crpsize); // recmix->outputARES
	void MixRecordPeak(int offset,int size);
	void CheckAndMixAudioRecordingBuffer(Seq_Song *);
	void AddRecordPeakBlock(AudioPeakBuffer *,SHORT **newchannelbuffer,int add); // + End Peak Sample
	
	void ClosePeakBuffer();

	int Save(void *,int len);

	#ifdef WIN32
	void LockIOSync(){lock_semaphore.Lock();} // IO Access Locks
	void UnlockIOSync(){lock_semaphore.Unlock();}
	CCriticalSection lock_semaphore;
	#endif

	camxFile writefile; // Recording Mode
	AudioHardwareBuffer recmix;
	OList regions;
	ARES recmixmaxpeak_m[MAXCHANNELSPERCHANNEL],recmixmaxpeak_p[MAXCHANNELSPERCHANNEL],recmixpeakbuffer_p[MAXCHANNELSPERCHANNEL][16],recmixpeakbuffer_m[MAXCHANNELSPERCHANNEL][16];

	Directory *directory;
	AudioPattern *recordpattern;
	AudioPeakBuffer *peakbuffer;

	LONGLONG filelength,datalen,datastart,dataend,samplesperchannel,
		recmaxpeakposition_zerosample,cyclereadsample,seekbeforewrite,/*recordstart_sampleposition,*/
		cyclestartoffset_samples,
		recordedsamples;

	Seq_Track *recordingtrack; // for recording
	char *creatednewfile,*replacewithfile;

	// ID Header
	int recmixcounter[MAXCHANNELSPERCHANNEL],recmixpeakbuffercounter[MAXCHANNELSPERCHANNEL],
		recordingoffset,recordingsize,reccycleloopcounter,

		channels, // mono, stereo
		writezerobytes,addzerosamples,
		// AIFF, wave
		samplebits, // 16,24,32...
		samplerate,
		samplesize_one_channel, // 16 bit=2,24=3
		samplesize_all_channels, // 16 bit stereo=2*2=4
		headerlen,
		type, // WAV,AIFF etc..
		mode,errorflag,
		writeoffset;

	bool externsamplerate,recstarted,recended,m_ok,
		waitingforpeakfile,destructive,samplesarefloat,
		ramfile, // Audio Data inside RAM
		dontcreatepeakfile,showregionsineditors,
		deleted,filenotfound,recordingactive,deleterecording,
		recordingadded,camxrecorded,camximport,nopeakfileasfile,recordpeakclosed,istrackrecordingfile;
	

private:

	char *info,*filename,*name;
};

class AudioRAMFile:public AudioHDFile
{
public:
	AudioRAMFile()
	{
		ramdata=0;
		ramfile=true;
	}

	void DeleteData()
	{
		if(ramdata){
			delete ramdata;
			ramdata=0;
		}
	}

	int FillAudioBuffer(AudioHardwareBuffer *,char *ramposition,int offset);
	void LoadSoundToRAM();

	char *ramdata,*ramend; // end of data
};
#endif
