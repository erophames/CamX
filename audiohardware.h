#ifndef CAMX_AUDIOHARDWARE
#define CAMX_AUDIOHARDWARE 1

#include "audiodefines.h"
#include "object.h"
#include "defines.h"
#include <cmath>

#define DEFAULTAUDIOBLOCKSIZE 1024

class Seq_Song;
class Seq_Track;
class TrackHead;
class AudioHardware;
class AudioHardwareBuffer;
class AudioPeakBuffer;
class AudioDevice;
class AudioBus;
class AudioPattern;
class RunningAudioFile;
class RecordingAudioFile;
class AudioHardwareChannel;
class Seq_Pos;
class AudioHDFile;
class AudioRAMFile;
class audioobject_Intern;
class VSTPlugin;
class AudioRealtime;
class AudioObject;
class InitPlay;
class Directory;
class AudioChannel;
class AudioRegion;
class guiWindow;
class guiMenu;
class AudioIOFX;
class guiGadget;
class AudioSend;
class InsertAudioEffect;
class AudioPort;
class CrashedPlugin;

extern double logvolume[];
extern float logvolume_f[];

enum ChannelTypes
{
	CT_MONO,
	CT_STEREO,
	CT_21,
	CT_QUADRO,
	CT_51,
	CT_61,
	CT_71
};

class mainAudioRealtime
{
public:
	bool FindAudioRealtime(AudioRealtime *);
	AudioRealtime *FirstAudioRealtime(){return(AudioRealtime *)realtimeaudiofiles.GetRoot();}
	AudioRealtime *RemoveAudioRealtime(AudioRealtime *,bool signal);
	void RemoveAllAudioRealtime(AudioChannel *);
	void RemoveTrackFromAudioRealtime(Seq_Track *,bool forcedelete=false); // Remove Track

	bool PlayRealtimeFile(AudioRealtime *);
	AudioRealtime *AddAudioRealtime(Seq_Song *,AudioDevice *,TrackHead *,Seq_Track *,AudioHDFile *,AudioRegion *,bool *endstatus,int offset,bool autoclose,guiWindow *from=0);
	void AddAudioRealtime_AR(Seq_Song *,AudioDevice *,TrackHead *,Seq_Track *,AudioRealtime *,AudioHDFile *,AudioRegion *,bool *endstatus,int offset,bool autoclose,guiWindow *from=0);

	bool StopRealtimePlayback(AudioRealtime *);
	void StopAudioRealtime(AudioRealtime *,bool*endstatus);
	void StopAllRealtimeEvents();
	void RefreshSignal();
	
	void LockRTObjects(){lock_semaphore.Lock();}
	void UnlockRTObjects(){lock_semaphore.Unlock();}

#ifdef WIN32
	CCriticalSection lock_semaphore;
#endif

private:
	OList realtimeaudiofiles;
};

class AudioFileInfo
{
public:

	AudioFileInfo()
	{
		type=TYPE_UNKNOWN;
		m_ok=false;
		samplesarefloat=false;
		errorflag=0;
	}

	LONGLONG samples;
	LONGLONG filelength;
	LONGLONG datastart,dataend,headerlen,datalen;

	int errorflag;
	int bits;
	int type;
	int channels;
	int samplerate;
	bool samplesarefloat,m_ok;
};

class mainAudio
{
	friend AudioPeakBuffer;

public:
	mainAudio();

	void ConnectAudioPortWithHardwareChannel(AudioPort *,AudioHardwareChannel *, int channel);

	char *GenerateSampleRateTMP();
	char *GenerateDBString(double,bool adddbstring=true);
	char *ScaleAndGenerateDBString(double f,bool adddbstring=true);

	bool CheckAudioFileUsage(char *name);
	bool CheckAudioFileUsage(AudioHDFile *);
	void DeleteUnusedPeaks();
	AudioDevice *GetActiveDevice();

	void CopyAudioFile(guiWindow *,AudioHDFile *,AudioPattern *ap=0);
	void CreateAudioRegionFile(guiWindow *,AudioHDFile *,AudioRegion *);
	void CheckWorkedFiles();
	void ReplaceAudioFiles();
	void CreateSendPopMenu(guiWindow *,AudioIOFX *,AudioSend *oldsend);
	void AddTrackMenu(Seq_Track *,guiMenu *,bool fix);
	AudioHDFile *FindAudioHDFile(char *);
	bool FindAudioHDFile(AudioHDFile *);
	double GetMaxDBFactor(){return logvolume[LOGVOLUME_SIZE];}
	void Load(camxFile *);
	void Save(camxFile *);
	void LoadHDFiles(camxFile *);
	void SaveHDFiles(camxFile *);
	int GetCountOfAudioFiles();
	void InitDefaultPorts();
	unsigned __int64 GetFreeRecordingMemory(Seq_Song *song,bool *ok); // -1 no Recording files
	void MoveRecFileToHDFiles(AudioHDFile *recfile);
	void RemoveAudioRecordingFile(AudioHDFile *recfile);

	// AudioHardware
	AudioHardware *FirstAudioHardware(){return (AudioHardware *)audiohardware.GetRoot();}
	AudioHardware *LastAudioHardware(){return (AudioHardware *)audiohardware.Getc_end();}
	AudioHardware *GetAudioAudioHardware(int ix){return (AudioHardware *)audiohardware.GetO(ix);}

	// AudioHDFiles+Regions
	void SaveDataBase();
	void LoadDataBase();

	AudioHDFile *AddAudioFile(AudioHDFile *);
	void AddAudioFileNoCheck(AudioHDFile *,Directory *);
	AudioHDFile *FirstAudioHDFile();
	AudioHDFile *DeleteHDFile(AudioHDFile *);
	bool DeleteAudioFile(char *); // Remove File, Stop Peak and Delete Peak File

	void CloseAllHDFiles();
	void FindNotFoundFiles(guiWindow *,bool allfoundmsg=false);
	void RemoveFileNotFound();
	int FindAudioHDFileInsideSong(AudioHDFile *);

	AudioHDFile *AddAudioFileQ(char *,bool camximport=false);
	AudioHDFile *AddAudioFile(char *,bool refreshgui,bool camximport=false);

	void AddAudioFile(AudioHDFile *,bool refreshgui);
	AudioRegion *FindAudioRegion(AudioHDFile *,AudioRegion *);
	
	AudioHDFile *GetHDFileIndex(int index){return (AudioHDFile *)audiohdfiles.GetO(index);}
	void RefreshAudioFileGUI(AudioHDFile *);

	// Audio Recording Files
	AudioHDFile *FirstAudioRecordingFile(){return (AudioHDFile *)audiorecordingfiles.GetRoot();}
	AudioHDFile *AddAudioRecordingFile(Seq_Track *,char *);

	// Audio Effect
	AudioObject *GetAudioInstrument(AudioChannel *,int index);
	AudioObject *GetAudioEffect(AudioChannel *,int index);

	// Intern Effects
	void CollectInternEffects();
	void CloseAllInternEffects();
	void AddInternEffect(audioobject_Intern *);

	audioobject_Intern *FirstInternEffect(){return (audioobject_Intern *)interneffects.GetRoot();}

	// VST Plugins
	VSTPlugin *FirstVSTInstrument(){return (VSTPlugin *)vstinstruments.GetRoot();} // instruments only
	int GetCountOfVSTInstruments(){return vstinstruments.GetCount();}
	
	VSTPlugin *FirstVSTEffect(){return (VSTPlugin *)vsteffects.GetRoot();}
	int GetCountOfVSTEffects(){return vsteffects.GetCount();}
	
	VSTPlugin *FindVSTEffect(char *);
	bool TestVST(char *dllname,char *filename,LONGLONG dlllength,Directory *dir);
	int CollectVSTPlugins(int index,Directory *,bool checkcheckplugins); // or NULL
	void SortVSTPlugin(OList *list,VSTPlugin *);
	void CloseAllVSTPlugins();

	// AudioFiles
	void AddCrashedPlugin(char *,bool save=true);
	bool IsPluginCrashed(char *);
	void DeleteCrashedPlugin(CrashedPlugin *);

	OList crashplugins;

	void CollectAudioFiles(char *dir,Directory *pdirectory);
	int GetCountOfAudioFilesInDirectoy(Directory *);

	// AudioPeak
	AudioPeakBuffer *FirstPeakBuffer(){return (AudioPeakBuffer *)audiopeakbuffer.GetRoot();}

	AudioPeakBuffer *FindPeakBuffer(char *);
	void ClosePeakFile(AudioHDFile *,AudioPeakBuffer *,bool forceclose,bool *deleteflag);

	void CheckPeakFiles();

	// Recording
	int GetCountOfRecordingFiles(int index);

	bool CheckIfAudioFileInfo(char *,AudioFileInfo *);
	bool CheckIfAudioFile(char *,int *to_samplerate=0,bool decodercheck=false,AudioFileInfo *info=0);

	void StopRecordingFiles(Seq_Song * /*,AudioHDFile */);
	void StopRecordingFiles(Seq_Track *track);

	void SetAllDevicesStartPosition(Seq_Song *,OSTART pos,int flag);

	bool SetAudioSystem(int index); // ASIO, DirectS....
	void SelectOtherDevice(AudioDevice *);

	void SetAudioSystem(char *defaultaudio);
	void SetSoundCard(char *defaultsoundcard);

	bool ResetAudio(int flag,int value);
	bool StartDevices();
	void ResetAudioDevices();
	bool StopDevices();

	void OpenAudio();
	void CloseAudio();

	double GetSamplesPerMs(){return samplesperms;}
	int GetGlobalSampleRate(){return samplerate;}
	int SetGlobalSampleRate(int newsamplerate);

	void SetGlobalRafSize(int);
	void SetSamplesSize(AudioDevice *,int size);

	void ConvertSampleToPeak(ARES peak,char *to,int length);
	void ConvertSamplesToTime(LONGLONG samples,Seq_Pos *);
	double ConvertSizeToMs(int rate,int size);
	double ConvertFactorToDb(double f){return 20*log10(f);} // 20er log}
	double ConvertDbToFactor(double db){return pow (10,db/20);}
	double ConvertPPQToInternRate(double ppq);
	double ConvertInternRateToPPQ(double ppq);
	int ConvertLogArrayVolumeToInt(double vol);
	double ConvertDBToLogArrayFactor(double vol);
	int FindLogArrayVolume(double vol);
	double ConvertToLogScale(double ); // 0 -1

	int ConvertLogArrayVolumeToIntNoAdd(double vol);
	double ConvertSamplesToMs(LONGLONG);
	double ConvertInternToExternSampleRate(double);
	double ConvertExternToInternSampleRate(double);
	LONGLONG ConvertMsToSamples(double ms);

	// Metro
	void OpenMetroClicks();
	void DeleteAllMetroClicks();
	void SelectPluginProgram(guiWindow *,InsertAudioEffect *);

	void TestForWaveFormat(camxFile *,AudioFileInfo *);
	void TestForAIFFFormat(camxFile *,AudioFileInfo *);

	OList audiohardware; // Asio, DirectS ....

	double silencefactor;
	ARES panorama_db[4];
	AudioHDFile *activehdfile;
	AudioRAMFile *metro_a,*metro_b;
	AudioHardware *selectedaudiohardware;
	char *old_hardwarename,*old_hardwaredevice;
	double samplesperms,samplespermsmul,ppqrateinternmul,internexternfactor;
	int oldmetrosamplerate,defaultchannel_type,defaultchannelindex_out,defaultchannelins,defaultchannelindex_in,rafbuffersize_index,newvstpluginsadded;
	
	bool audiooutactive,collectmode,ignorecorrectaudiofiles;
#ifdef WIN32
	bool foundasio;
#endif

	// Audio File Work
	void ChangeAudioFileToFile(char *oldfilename,char *newfilename);

	// Peak
	void AddPeakBuffer(AudioPeakBuffer *);
	bool DeleteAudioFileAndPeakBuffer(char *);

	void LockAudioFiles(){afsemaphore.Lock();}
	void UnlockAudioFiles(){afsemaphore.Unlock();}
	CCriticalSection afsemaphore;

private:
	void CollectAudioHardware();
	void CloseAllAudioDevices();
	void InitSampleRate();

	int samplerate; // default Samplerate
	OList audiopeakbuffer,interneffects,vstinstruments,vsteffects,audiohdfiles,audiorecordingfiles;
};
#endif