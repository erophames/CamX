#ifndef CAMX_AUDIOFILEWORK_H
#define CAMX_AUDIOFILEWORK_H 1

#include "audiothread.h"
#include "audiohdfile.h"
#include "progress.h"
#include "objectpattern.h"
#include "resample/config.h"
#include "resample/samplerate.h"
#include "crossfade.h"

class Decoder;

#define MAXSAMPLEZOOM 1000 // max samples per pixel 1- maxsamplezoom

#define AFW_BLOCKSIZE 4096 // Samples<->Block Size
#define AFW_READWRITEBUFFERSIZE 4096 // Samples
#define AFW_MAXVCUTS 16 // Number max VCuts,Undo/Redos

class AudioVCut
{
public:
	LONGLONG from, // samplestart
		to; // sampleend
};

class AudioVirtualWork
{
public:
	AudioVirtualWork()
	{
		cuts=0; // max AFW_MAXVCUTS
		blockpointer=0; // Seek Pointer

		blockmem=0;
		blockmem_blocks=0;
	}

	~AudioVirtualWork()
	{
		if(blockmem)
			delete blockmem;
	}

	void RecalcVSamples();
	bool AddCutSamples(LONGLONG from,LONGLONG to);
	bool Undo();
	bool Redo();

	// V Seek
	bool Seek(LONGLONG samples);
	LONGLONG GetReadBlock();

	LONGLONG FirstBlock();
	LONGLONG NextBlock(LONGLONG block);

	LONGLONG orgblocksize,orgsamplesize,vsamplesize;

	int cuts;
	AudioVCut avcuts[AFW_MAXVCUTS];

private:
	void MuteSamples(LONGLONG from,LONGLONG to);

	LONGLONG *blockmem,blockmem_blocks,blockpointer;
};

class AudioFileWork:public AudioWork
{
public:
	AudioFileWork()
	{
		filename=0; 
		creatednewfile=0;
		skipped=false;
		camximport=false;
		directory=0;
		nogui=false;
	}

	~AudioFileWork()
	{
		DeleteWork();
	}

	bool CreateTMP(char *add=0);
	void CreateFileNameTMP();

	bool CheckHDFile(AudioHDFile *,int type); //v

	void Init(char *fname);
	void AddGUIMessage(int type,LONGLONG from,LONGLONG tosample);
	void InitFile();
	bool VolumeUpDown(LONGLONG samplepos,LONGLONG samples);

	void DeleteWork(); //v
	void Stop()// v
	{
		stopped=true;
	}

	AudioHDFile hd;
	Progress progress;

	Directory *directory;
	char *filename, // file to work with
		*creatednewfile; // new file or NULL
	bool audiofilecheck,skipped,camximport,nogui;

private:
#ifdef CAMX
	bool ConvertRawToFloat_Ex(void *from,ARES *to,int samples);
	void ConvertFloatToRaw_Ex(ARES *from,void *to,int samples);
#else
	bool ConvertRawToFloat(void *from,float *to,long samples);
	void ConvertFloatToRaw(float *from,void *to,long samples);
#endif

	AudioVirtualWork vwork;
};

class AudioFileWork_CutRange:public AudioFileWork
{
public:
	void Start();

	LONGLONG samplestart,sampleend;
	int crossover; // samples >|<
};

class AudioFileWork_FillZero:public AudioFileWork
{
public:
	void Start();

	LONGLONG samplestart,sampleend;
};

class AudioFileWork_CreateNewFile:public AudioFileWork
{
public:
	AudioFileWork_CreateNewFile()
	{
		copyfile=false;
	}

	void Start();

	LONGLONG samplestart,sampleend;
	bool copyfile;
};

class AudioFileWork_SplitFileInChannels:public AudioFileWork
{
public:

	AudioFileWork_SplitFileInChannels()
	{
		tracks=0;
		newfiles=0;
		count=0;
		dontdelete=true; // use later
	}

	void EndWork();
	void Start();

	OSTART position;

	Seq_Song *song;
	Seq_Track **tracks;
	char **newfiles;
	int count;
	int samplebits;
	int samplerate;
};

class AudioFileWork_CopyFile:public AudioFileWork
{
public:
	void Start();
};

class Work_CrossFade:public Object
{
public:
	Work_CrossFade()
	{
		connectfile=0;
		readcfsamples=0;
		endreached=false;
	}

	Work_CrossFade *Next(){return (Work_CrossFade *)next;}

	Seq_CrossFade fade1,fade2;
	char *connectfile;
	LONGLONG from2,to2,readcfsamples,crossfadesize; // region file2
	bool endreached;
};

class AudioFileWork_ExportPatternFile:public AudioFileWork
{
public:
	AudioFileWork_ExportPatternFile()
	{
		split=false;
	}

	void Start();
	char *orgfile,*tofile;
	LONGLONG from,to; // region file1
	OList wfades; // Work_CrossFade
	bool split;
};

class AudioFileWork_Normalize:public AudioFileWork
{
public:
	AudioFileWork_Normalize()
	{
		userange=false;
	}

	void Start();

	double maxpeak;
	bool userange;
	LONGLONG samplestart,sampleend;
};

class AudioFileWork_Resample:public AudioFileWork
{
public:
	void Start();
	int newsamplerate;
};

class AudioFileWork_Finder:public AudioFileWork
{
public:
	void Start();
	int counter;
};

class AudioFileWork_FinderList:public AudioFileWork
{
public:

	~AudioFileWork_FinderList(){DeInit();}
	void DeInit();
	void Start();

	OList list; // Audio HD File
};

class AudioFileWork_Converter:public AudioFileWork
{
public:
	AudioFileWork_Converter()
	{
		inputfile=outputfile=0;
		src_state=0;
		resampler=false;
		input=output=0;
		output_count=0;
		decodedfile=0;
		decoder=0;
	}

	void Start();
	void Stop();

	void WriteResampler(Decoder *decoder);

	static bool DecodeFirstSamples(Decoder *);
	static bool WriteDecodedData(Decoder *);
	static bool WriteDecodedDataEnd(Decoder *);

	AudioHDFile savedecoder;
	AudioHardwareBuffer buff;

	SRC_DATA	src_data ;
	SRC_STATE	*src_state ;

	char *inputfile,*outputfile,*decodedfile;

	// Resampler
	
	ARES *input,*output;
	int bsize,newsamplerate;
	LONGLONG output_count;

	Decoder *decoder;
	bool resampler;

};
#endif