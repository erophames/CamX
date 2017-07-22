#ifndef CAMX_AUDIOPEAKBUFFER
#define CAMX_AUDIOPEAKBUFFER 1

class AudioPeakBuffer:public ObjectLock
{
	friend class mainAudio;
	
public:
	AudioPeakBuffer();
	~AudioPeakBuffer()
	{
		FreePeakMemory(true);
	}

#ifdef _DEBUG
	char n[4];
#endif
	
	void CopyPeakBuffer(AudioPeakBuffer *);
	
	void CreatePeakMix();
	bool ReadPeakFile(AudioHDFile *);
	void FreePeakMemory(bool full);
	AudioPeakBuffer *NextAudioPeakBuffer() {return (AudioPeakBuffer*)next;}
	
	LONGLONG peaksamples,mixposition;
	char *samplefilename,*peakfilename;
	SHORT *peakmixbuffer,// mix of all channels, arrange editor	
	 *channelbuffer[MAXCHANNELSPERCHANNEL];	

	double maxpeakfound;
	int channels;
	bool initok,lastsampleset,dontdeletepeakmixbuffer,closed;	
};

#endif