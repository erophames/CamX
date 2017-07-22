#ifndef CAMX_VSTDIRECTORY_H
#define CAMX_VSTDIRECTORY_H 1

#include "object.h"

class Directory:public Object
{
public:
	Directory()
	{
		dir=0;
		vstcount=0;
		audiofilecount=0;
		dontdelete=false;
	}
	
	Directory *NextDirectory() {return (Directory *)next;}
	
	char *dir;
	int vstcount,audiofilecount;
	bool dontdelete;
};

#endif