#ifndef CAMX_AUDIODEVICE
#define CAMX_AUDIODEVICE 1

#include "object.h"
#include "audiohardwarebuffer.h"
#include "audioports.h"

#define MAXRECORDCHANNELS 32
#define MAXRECORDBUFFER 128
#define AUDIORECORDBUFFERSIZE 32768 // 32 KB buffer/p Audio Record Channel
#define DEFAULT_MASTERSIZEFACTOR 100 // 48000 -> 480 Sample
#define MINNUMBERHARDWAREBUFFER 4


#define RESETAUDIO_NEWSAMPLERATE 1
#define RESETAUDIO_NEWRAFBUFFERSIZE 2
#define RESETAUDIO_NEWBUFFERSIZE 4

class Seq_Song;
class AudioDevice;
class AudioHDFile;
class CEvent;
class AudioHardwareChannel;
class AudioChannel;

class AudioEffectParameter;
class AudioIOFX;
class Seq_AudioIO;

enum
{
	ADSR_44,
	ADSR_48,
	ADSR_88,
	ADSR_96,

	ADSR_176,
	ADSR_192,
	ADSR_352,
	ADSR_384,

	ADSR_LAST
};

class AudioDevice_BufferSettings
{
public:
	AudioDevice_BufferSettings();

	int setSize,latency_input_samples,latency_output_samples,minSize,maxSize,prefSize,granularity,

		addtoinputlatency_samples16,addtooutputlatency_samples16,
		addtoinputlatency_samples32,addtooutputlatency_samples32,
		addtoinputlatency_samples64,addtooutputlatency_samples64,
		addtoinputlatency_samples96,addtooutputlatency_samples96,
		addtoinputlatency_samples128,addtooutputlatency_samples128,
		addtoinputlatency_samples160,addtooutputlatency_samples160,
		addtoinputlatency_samples192,addtooutputlatency_samples192,
		addtoinputlatency_samples224,addtooutputlatency_samples224,
		addtoinputlatency_samples256,addtooutputlatency_samples256,
		addtoinputlatency_samples384,addtooutputlatency_samples384,
		addtoinputlatency_samples512,addtooutputlatency_samples512,
		addtoinputlatency_samples1024,addtooutputlatency_samples1024,
		addtoinputlatency_samples2048,addtooutputlatency_samples2048,
		addtoinputlatency_samples3072,addtooutputlatency_samples3072,
		addtoinputlatency_samples4096,addtooutputlatency_samples4096,
		addtoinputlatency_samples5120,addtooutputlatency_samples5120,
		addtoinputlatency_samples6144,addtooutputlatency_samples6144,
		addtoinputlatency_samples8192,addtooutputlatency_samples8192,
		addtoinputlatency_samplesmisc,addtooutputlatency_samplesmisc;

	bool used;
};

class AudioDevice:public Object
{
	friend class mainAudio;

public:
	AudioDevice();

	enum
	{
		SETSTART_INIT,
		SETSTART_CYCLERESET
	};

	enum{
		RECORDERROR_BUFFEROVERFLOW=1,
		RECORDERROR_MESSAGE=2
	};

	enum{
		STATUS_OUTPUTOK=1,
		STATUS_INPUTOK=2
	};

	virtual void FillInputBufferEnd(){}
	virtual bool CheckOutRefill(){return false;}
	virtual void InitLatencies();
	virtual void CreateDeviceStreamMix(Seq_Song *){}
	virtual bool CheckAudioDevice(char *name){return false;}
	virtual bool OpenAudioDevice(char *name){return false;}
	virtual void CleanMemory();
	virtual bool InitAudioDeviceChannels(){return false;}
	virtual void StartAudioHardware(){}
	virtual void StopAudioHardware(){}
	virtual int GetSamplePosition(){return 0;}
	virtual void CloseDeviceDriver(){};
	virtual void SkipDeviceOutputBuffer(Seq_Song *){}

	enum{
		RESETAUDIODEVICE_FORCE=1
	};

	virtual void Reset(int flag){}

	void InitDefaultPorts();
	void DeInitDefaultPorts();
	void ClearOutputBuffer(Seq_Song *);
	void InitAudioDevice();
	void CheckSetSize();
	void InitNewAudioDevice();
	void ConvertSampleRateToIndex();

	void WaitForRecordingEnd();
	void ConvertInputDataToSongInputARES(Seq_Song *);
	
	void RecordInputAudioBufferToSong(Seq_Song *,int buffer);
	ARES CopyDeviceToHardware(Seq_Song *); // return device max peak
	void SetStart(Seq_Song *,OSTART pos,int flag);
	int GetBestSampleRate();
	void Load(camxFile *);
	void Save(camxFile *);
	void ResetDeviceIndex();
	
	AudioDevice *NextDevice() {return (AudioDevice *)next;}
	void CalcTicksPerBuffer();
	int ClearOffSet(double);
	int ClearOffSet(int);

	void CreateAudioBuffers(); // Audio Hardware Buffers
	void DeleteAudioBuffers();
	void CalcSampleBufferMs(int rate,int size);
	void StopDevice();

	void AddInputHWChannel(AudioHardwareChannel *,int index);
	void AddOutputHWChannel(AudioHardwareChannel *,int index);

	bool CloseAudioDevice(bool full);
	bool CheckSampleRate(int samplerate);
	bool StartDevice();
	bool CheckIfAllHardwareChannelsAreUsed();

	AudioHardwareChannel *FirstOutputChannel(){return (AudioHardwareChannel *)outchannels.GetRoot();}
	AudioHardwareChannel *FirstOutputChannelWithGroup(int group);
	int GetCountOutputChannel(){return outchannels.GetCount();}
	AudioPort *FindVOutChannel(int channelchannels,int index);
	AudioPort *FindVInChannel(int channelchannels,int index);

	AudioHardwareChannel *FirstInputChannel(){return (AudioHardwareChannel *)inchannels.GetRoot();}
	AudioHardwareChannel *FirstInputChannelWithGroup(int group);

	AudioHardwareChannel *GetOutputChannelIndex(int index){return (AudioHardwareChannel *)outchannels.GetO(index);}
	AudioHardwareChannel *GetInputChannelIndex(int index){return (AudioHardwareChannel *)inchannels.GetO(index);}

	int GetCountOfOutputChannels(){return outchannels.GetCount();}
	int GetCountOfInputChannels(){return inchannels.GetCount();}

	inline void LockMix(){
#ifdef WIN32
		lock_mixsemaphore.Lock();
#endif
	}

	inline void UnlockMix(){
#ifdef WIN32
		lock_mixsemaphore.Unlock();
#endif
	}

	inline void LockRecordBuffer(){sema_recordbuffer.Lock();}
	inline void UnlockRecordBuffer(){sema_recordbuffer.Unlock();}

#ifdef WIN32
	CCriticalSection lock_mixsemaphore,sema_recordbuffer;
#endif

	char *GetDeviceName()
	{
		if(!devname)return "Dev?";
		return devname;
	}

	AudioHardwareBuffer devmix,mastermix; // mixed RAW Song Data Out
	AudioPort inputaudioports[AUDIOCHANNELSDEFINED][CHANNELSPERPORT],outputaudioports[AUDIOCHANNELSDEFINED][CHANNELSPERPORT];
	OList inchannels,outchannels;

	char info1[64],*devname,*initname,*devicetypname;

	void *buffer_out, // [L][R]1+2 Channels Blocks
		*mixbuffer_out, // [LR][LR] etc...
		*inbuffer_raw; // Record Buffer [LR][LR]

	inline void LockCore(){core_sync.Lock();}
	inline void UnlockCore(){core_sync.Unlock();}
	CCriticalSection core_sync;

	void ResetTimer(){timeforrefill_maxsystime=timeforrefill_systime=timeforaudioinpurefill_systime=timeforaudioinputrefill_maxsystime=0;}

	inline void LockTimerCheck_Output(){core_time_output.Lock();}
	inline void UnlockTimerCheck_Output(){core_time_output.Unlock();}
	CCriticalSection core_time_output;

	inline void LockTimerCheck_Input(){core_time_input.Lock();}
	inline void UnlockTimerCheck_Input(){core_time_input.Unlock();}
	CCriticalSection core_time_input;

	AudioDevice_BufferSettings buffersettings[ADSR_LAST];
	int usebuffersettings;
	inline int GetSetSize(){return buffersettings[usebuffersettings].setSize;}
	void SetBufferSize(int newBufferSize){buffersettings[usebuffersettings].setSize=newBufferSize;}

	int GetInputLatencySamples(){int li=buffersettings[usebuffersettings].latency_input_samples+GetAddToInputLatencySamples();return li>0?li:0;}
	int GetOutputLatencySamples(){int lo=buffersettings[usebuffersettings].latency_output_samples+GetAddToOutputLatencySamples();return lo>0?lo:0;}

	int GetInputLatencySamples_NoAdd(){return buffersettings[usebuffersettings].latency_input_samples;}
	int GetOutputLatencySamples_NoAdd(){return buffersettings[usebuffersettings].latency_output_samples;}

	int GetAddToInputLatencySamples();
	int GetAddToOutputLatencySamples();
	void SetAddInputLatency(int v);
	void SetAddOutputLatency(int v);

	void SetInputLatency(int s){buffersettings[usebuffersettings].latency_input_samples=s;}
	void SetOutputLatency(int s){buffersettings[usebuffersettings].latency_output_samples=s;}
	bool IsSampleRate(int sr){return buffersettings[sr].used;}
	void ActivateSampleRate(int sr,bool onoff){buffersettings[sr].used=onoff;}
	int GetMinBufferSize(){return buffersettings[usebuffersettings].minSize;}
	int GetMaxBufferSize(){return buffersettings[usebuffersettings].maxSize;}
	int GetPrefBufferSize(){return buffersettings[usebuffersettings].prefSize;}
	virtual void InitMinMaxPrefBufferSizes(){}

	void SetMinBufferSize(int v){buffersettings[usebuffersettings].minSize=v;}
	void SetMaxBufferSize(int v){buffersettings[usebuffersettings].maxSize=v;}
	void SetPrefBufferSize(int v);
	void SetGranularity(int v){buffersettings[usebuffersettings].granularity=v;}

	AudioHardwareChannel *firstfreeaudioinputhw;
	
	LONGLONG timeforrefill_systime,timeforrefill_maxsystime,
		timeforaudioinpurefill_systime,timeforaudioinputrefill_maxsystime;

#ifdef DEBUG
	LONGLONG timetoconvertinputdata; // Audio In
#endif

	double ticksperbuffer,samplebufferms,newsamplerate;
	ARES devicedroprate;

	int play_bufferindex,
		record_bufferindex,
		recordbuffer_writeindex,recordbuffer_readindex,recordbuffer_readwritecounter,recorderror,
		buffer_out_channels,inbuffer_rawsize,buffer_in_channels,
		numberhardwarebuffer,status,
		in_channels,out_channels,bitresolution,
		samplebufferms_long,
		skipoutbuffer,
		sampleformat,audiosystemtype,id,
		audiosystem // ASIO,Directs....
		;

	bool userawdata,stopdevice,init,devicepRepared,sync,
		devicestarted,deviceoutofsync,deviceoutofsyncmsg,deviceoutofMIDIsync,
		inportsfound,outportsfound,outbuffercleared,resetrequest;
};

class AudioHardware:public Object
{
public:
	AudioHardware(char *id_name,int id);

	AudioDevice *GetActiveDevice(){return activedevice;}
	void SetActiveDevice(AudioDevice *);
	void SetActiveDeviceInit();

	AudioDevice *FirstDevice(){return (AudioDevice *)devices.GetRoot();}
	AudioDevice *RemoveAudioDevice(AudioDevice *);
	AudioDevice *GetAudioDeviceIndex(int index){return (AudioDevice *)devices.GetO(index);}
	AudioHardware *NextHardware() {return (AudioHardware *)next;}

	// Devices ----------------------------------------------------
	void AddAudioDevice(AudioDevice *,char *name,char *dtypename);

	OList devices;
	AudioDevice *activedevice;
	int systemtype; // ASIO, DirectS ....
	char name[255];
};
#endif