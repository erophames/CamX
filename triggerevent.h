#ifndef CAMX_TRIGGEREVENT_H
#define CAMX_TRIGGEREVENT_H 1

#include "object.h"

class TriggerEvent:public Object
{
public:

	TriggerEvent()
	{
		#ifdef _DEBUG
		strcpy(info,"TGE");
		#endif

		data=0;
	}

	#ifdef _DEBUG
	char info[4];
	int i;
#endif

	char *data;
	int datalength,sampleoffset;
	UBYTE bytes[3]; // status,byte1,byte2

	TriggerEvent *NextEvent(){return (TriggerEvent *)next;}	
	TriggerEvent *PrevEvent(){return (TriggerEvent *)prev;}
};

#endif