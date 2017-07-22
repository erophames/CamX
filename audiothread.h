#ifndef CAMX_AUDIOTHREAD_H
#define CAMX_AUDIOTHREAD_H 1

#include "defines.h"
#include "threads.h"
#include "object.h"

#define PEAKFILEVERSION 21 // 1.2

class AudioPattern;
class AudioHDFile;
class guiWindow;
class AudioCreatePeakFile;
class AudioPeakBuffer;

class AudioPeakFileThread:public Thread
{
public:
	AudioPeakFileThread()
	{
		createprogress=0;
		runningcpf=0;
	}

	int StartThread();

	// Background Create Peak Files
	bool CreatePeakFile(guiWindow *,AudioHDFile *,char *name,AudioCreatePeakFile *);
	bool StopPeakFile(char *);

	bool StopPeakFile(AudioHDFile *);

	AudioCreatePeakFile *FirstCreatePeakFile(){return (AudioCreatePeakFile *)createpeakfiles.GetRoot();}
	AudioCreatePeakFile *AddCreatePeakFile(AudioHDFile *,AudioPeakBuffer *);
	AudioCreatePeakFile *DeleteCreatePeakFile(AudioCreatePeakFile *);

	AudioHDFile *GetRunningFile();

#ifdef WIN32
	static PTHREAD_START_ROUTINE AudioPeakFileCreator(LPVOID pParam); 
#endif

	AudioCreatePeakFile *runningcpf;
	double createprogress; // 0-100%
	
private:
	OList createpeakfiles;
};

class AudioWork:public Object
{
public:
	AudioWork()
	{
		ok=false;
		error=false;
		stopped=false;
		dontdelete=false;
	}

	enum WorkFlags{
		// Function Classes
		AWORK_CUT,
		AWORK_CREATENEW ,
		AWORK_NORMALIZE,
		AWORK_FILLZERO
	};

	virtual bool CheckHDFile(AudioHDFile *,int type){return true;}
	virtual void DeleteWork(){}
	virtual void Start(){}
	virtual void Stop(){}

	AudioWork *NextWork(){return (AudioWork *)next;}

	OList regions;
	bool ok,error,stopped,dontdelete;
};

class AudioFileWork;

class AudioWorkedFile:public Object
{
public:
	AudioWorkedFile(int t,LONGLONG from,LONGLONG to)
	{
		type=t;
		filename=0;
		file=0;
		createnewfile=0;
		fromsample=from;
		tosample=to;

		camximport=false;
	}

	enum types{
		AUDIOWORKED_TYPE_CUT,
		AUDIOWORKED_TYPE_NORMALIZE,
		AUDIOWORKED_TYPE_CREATEFILE,
		AUDIOWORKED_TYPE_DELETE,
		AUDIOWORKED_TYPE_FILLZERO,
		AUDIOWORKED_TYPE_RESAMPLING,
		AUDIOWORKED_TYPE_CONVERTED,
		AUDIOWORKED_TYPE_FINDER,
		AUDIOWORKED_TYPE_COPYIED,
		AUDIOWORKED_TYPE_SPLITTED,

	};

	AudioFileWork *fromfilework;
	LONGLONG fromsample,tosample;
	int type;
	char *filename,*createnewfile;
	AudioHDFile *file;
	bool camximport;
	OList regions;
};

class AudioWorkFileThread:public Thread
{
public:
	int StartThread();

	bool CheckIfWorkPossible(AudioHDFile *,int type);
	void AddWork(AudioWork *);
	AudioWork *DeleteWork(AudioWork *);
	void AddWorkedFile(AudioWorkedFile *);
	AudioWorkedFile *DeleteWorkedFile(AudioWorkedFile *);

#ifdef WIN32
	static PTHREAD_START_ROUTINE AudioWorkFileCreator(LPVOID pParam); 
#endif

	AudioWork *FirstWork(){return (AudioWork *)audioworklist.GetRoot();}
	AudioWorkedFile *FirstWorkedFile(){return (AudioWorkedFile *)workedfiles.GetRoot();}

	int StopAllProcessing();

private:
	OList audioworklist, // class AudioWork
	 workedfiles; // AudioWorkedFile
};
#endif