#ifndef CAMX_PROGRESS
#define CAMX_PROGRESS 1

#define PROGRESSERROR_ID_ERROR 0 // unknown error
#define PROGRESSERROR_ID_OUTOFMEMORY 1
#define PROGRESSERROR_ID_FILEERROR 2

#include "object.h"

class Progress:public Object
{
public:
	Progress()
	{
		infostring=0;
		ResetInfo();
		counter=0;
		value=false; // default percent
		p_stopped=false;
	}

	void Start(char *idstring);
	void End(bool stopped);
	void SetPercent(double per);

	void ResetInfo()
	{
		working=false;
		percent=0;
		l_value=p_value=0;
		done=false;
		error=false;
	}

	void SetError(int id) // see progress error IDs
	{
		error=true;
		errorid=id;
	}

	// Working on/off - percent
	bool value,working,p_stopped,error,done;
	double lastpercent,percent;
	int l_value,p_value,counter,errorid;
	char *infostring;
};

#endif
