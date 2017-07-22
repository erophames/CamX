#ifndef CAMX_AUDIOPEAKFILE
#define CAMX_AUDIOPEAKFILE 1

#include "object.h"


class AudioHDFile;
class AudioPeakBuffer;

class AudioCreatePeakFile:public ObjectLock
{
	friend AudioPeakFileThread;

public:
	AudioCreatePeakFile()
	{
		audiohdfile=0;
		newpeakbuffer=refreshpeakbuffer=0;
		name=0;
		stop=false;
	};
	
	AudioHDFile *audiohdfile; // AudioFile 
	AudioPeakBuffer *refreshpeakbuffer,*newpeakbuffer; // refresh of peakbuffer
	AudioCreatePeakFile *NextPeakFile() {return (AudioCreatePeakFile *)next;}

	bool CheckFile(char *file);

	bool stop;
private:
	char *name;
};

#endif