#ifndef EX_WAVE
#define EX_WAVE 1

#define TEST_EXWAVE 1

// Ex_Wave File Version 1.0 20.04.2009
// Copyright Martin Endres

// #define WIN32=Windows
// #define QT=QT

#include "systemdefines.h" // #define int here

// Error Flags : wavefile.GetErrorFlag()
#define ERROR_UNABLETOOPEN_READ 1
#define ERROR_UNABLETOOPEN_SAVE 2
#define ERROR_CHUNKERROR 4
#define ERROR_UNKNOWNBITRATE 8
#define ERROR_UNKNOWNFORMAT 16
#define ERROR_MEMORYALLOC 32
#define ERROR_FILENOTINIT 64
#define ERROR_FILEISNORMALIZED 128

#define EX_BLOCKSIZE 4096 // Peak Range [><], min 1024. 2048,4096,8192,16384 etc...
#define MAXCHANNELS 32 // Maximum of Channels per File
#define MAXCUTS 32 // Maximum Virtual Cut

class WaveFile;

class AudioVWorkCut
{
public:
	LONGLONG from; // samplestart
	LONGLONG to; // sampleend
};

class AudioVWork
{
	friend WaveFile;

public:
	AudioVWork()
	{
		cuts=0; // max MAXVCUTS
		blockpointer=0; // Seek Pointer

		blockmem=0;
		blockmem_blocks=0;

		allcut=false;
	}

	~AudioVWork()
	{
		if(blockmem)
			delete blockmem;
	}

	void RecalcVSamples();
	bool AddCutSamples(ULONGLONG from,ULONGLONG to);
	bool Undo();
	bool Redo();

	// V Seek
	bool Seek(LONGLONG samples);
	LONGLONG GetReadBlock();
	LONGLONG FirstBlock();
	ULONGLONG NextBlock(ULONGLONG block);

	ULONGLONG GetFileSize()
	{
		if(cuts==0)
			return orgsamplesize;

		return vsamplesize;
	}

	ULONGLONG orgblocksize,
	 orgsamplesize,
	 vsamplesize;

	bool allcut; // all Samples cut, empty virtual file!
	int cuts;
	AudioVWorkCut avcuts[MAXCUTS];

private:
	void Init();
	void MuteSamples(ULONGLONG from,ULONGLONG to);

	ULONGLONG *blockmem; // Main Block Buffer
	ULONGLONG blockmem_blocks;
	ULONGLONG blockpointer;

	WaveFile *wavefile; // <-> WaveFile
};

// Wave File Class
// 1. Init and Check Wave File
//		Init(filename);
// 2. CreatePeak if Peak needed, return: bool=true if ok

class WaveFile
{
	friend AudioVWork;

public:
	WaveFile()
	{
		rbuffer=0;
		floatbuffer=0;
		readopen=false;

		filename=0;
		errorflag=0;

		init=false;
		initpeak=false;

		for(int i=0;i<MAXCHANNELS;i++)
		{
			peak[i]=0;
			peakaftercut[i]=0;
		}

		stop=false;
		progress=0; // 0%

		channels=0;
		samplerate=0;
		samplebits=0;
		maxpeakvalue=0;
		smooth=EX_BLOCKSIZE; // default smooth Sample Size
		vcut.wavefile=this; // Connect with VCut Class
		readblock=0;
		writeopen=false;
	}

	~WaveFile()
	{
		if(rbuffer)delete rbuffer;
		if(floatbuffer)delete floatbuffer;
		if(filename)delete filename;

		// Release Peak Buffer
		for(int i=0;i<MAXCHANNELS;i++)
		{
			if(peak[i])delete peak[i];
			if(peakaftercut[i])delete peakaftercut[i];
		}
	}

	int GetErrorFlag() // 0=no Errors
	{
		return errorflag;
	}

	void ResetErrorFlag() // Set Errorflag=0
	{
		errorflag=0;
	}

	bool Init(char *name); //return true/false
	bool CreatePeak();// return true/false

	ULONGLONG GetFileSize()
	{
		if(init==true)
		{
			return vcut.GetFileSize();
		}

		return 0;
	}

	bool Normalize(); // Normalize File, return true/false, Peaks needed, Normalize only if maxpeak<0.99
	bool SaveBlock(char *newfile,LONGLONG start,LONGLONG end); // Save to new File, Sample Start, Sample End

	bool SaveAll(char *newfile) // Save File to new File, Sample 0-Last Sample
	{
		return SaveBlock(newfile,0,GetFileSize());
	}

	double GetProgess(){return progress;} // return progress status 0-100%

	void Stop(){stop=true;} // Stop Work Function

	bool DoSmooth_Save(); // Volume Up/Down Save File

	void SmoothUp(void *rbuffer,int buffersize);
	void SmoothDown(void *rbuffer,int buffersize);

	// Playback, Read file
	bool Read_Seek(LONGLONG samples); // From Sample 0, return true=seek ok
	bool Read_Open(); // return true=open ok
	void Read_Close();
	bool Read_ReadSamplesToBuffer(); // return true=buffer read ok, false=end of file or error

	ULONGLONG Read_GetSamplePosition()
	{
		return EX_BLOCKSIZE*readbuffer_block;
	}

	float *GetFloatBuffer(){return floatbuffer;} // pointer converted RAW->float samples or NULL

	// SYSTEM Read,Write,Seek
#include "fileio.h"

	// Read
	bool FILE_OpenRead(char *filename);
	LONGLONG FILE_GetSize(); // return - ULONGLONG size in Bytes
	bool FILE_Read(void *to,int size); // Read (size) Bytes From Read File
	ULONGLONG FILE_GetCurrentPos(); // return - ULONGLONG Read File Position

	void FILE_SeekCurrent(LONGLONG seek); // Seek <> Bytes
	void FILE_SeekBegin(LONGLONG seek); // Seek |> Bytes From File Start
	void FILE_Close(); // close Read File

	// Save
	bool FILE_OpenSave(char *filename);
	bool FILE_Write(void *from,int size); // Wrize (size) Bytes to Save File
	void FILE_SeekSaveBegin(LONGLONG seek); // Seek Save File |> Bytes from File Start
	void FILE_ReadFromSave(void *to,int size); // Read (size) Bytes From Save File
	void FILE_CloseSave(); // close Save File

	void FILE_DeleteFile(char *filename);

	// File Info
	int channels, // 1,2...
	 samplerate,// 44100,48000...
	 samplebits, // 16,20,24,32
	 samplesize_one_channel,// 16 bit = 2
	 samplesize_all_channels;  // 16 bit stereo =4
	int headerlen; // File Header in Bytes

	ULONGLONG samplesperchannel, // Samples per Channel in File
	 datalen, // Length of Data Block in Bytes
	 datastart, // First Sample Position, Byte (ULONGLONG)
	 dataend, // Last Byte (ULONGLONG)
	 filelength; // ULONGLONG
	float maxpeakvalue; // Max Peak Sample Found in File

	// Write
	int writeheaderlen;
	ULONGLONG writedatalen,writedataend;
	bool writeopen;

	// Peak
	USHORT **GetPeakBuffer() // USHORT First Peak Buffer or NULL, (USHORT 0-65535)
	{
		if(init==true && initpeak==true)
		{
			if(vcut.cuts==0)return peak;
			return peakaftercut;
		}

		return 0; // error ?
	}

	int GetPeakBufferLength() // Length of Peak Buffer Array or NULL, Array in USHORT (0-65535)
	{
		if(init==true && initpeak==true)
		{
			if(vcut.cuts==0)
			{
				return peakbuffersamples; // (samplesperchannel/PEAKBUFFERSIZE)+1;
			}

			return (int)(vcut.vsamplesize/EX_BLOCKSIZE)+1; // +1
		}

		return 0; // error ?
	}

	// Cut
	bool VirtualCut(ULONGLONG from,ULONGLONG to) // Cut Sample from<->to (virtual), return true if ok
	{
		bool ok=vcut.AddCutSamples(from,to);

		if(ok==true)
			GenerateVPeak();

		return ok;
	}

private:
	AudioVWork vcut;

	// Peak
	void GenerateVPeak();

	USHORT *peak[MAXCHANNELS]; // max 32 channels (peak for full file)
	USHORT *peakaftercut[MAXCHANNELS];

	int peakbuffersamples;
	ULONGLONG readblock;

	char *filename; // name of Wave File on HD

	double progress; //0-100%

	void *rbuffer;
	float *floatbuffer;
	int readbuffer_block; // read file block 0 ... <
	bool readopen,read_smoothdown,read_smoothup;

	int errorflag;
	bool init, // true if wave format
	 initpeak, //true if peak created
	 stop;

	ULONGLONG smooth; // Sample 0->100,100->0 size
};

#endif