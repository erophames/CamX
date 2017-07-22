#ifndef CAMX_TIMESTRING_H
#define CAMX_TIMESTRING_H 1

#include "seqpos.h"

class TimeString
{
public:
	TimeString()
	{
		div[1]=0;

		for(int i=0;i<5;i++)
		{
			strings[i][0]=0;
			stringspointer[i]=strings[i];
		}

		index=0;
	}

	Seq_Pos pos;
	
	
	char *stringspointer[5];
	char strings[5][32],
		div[2], // - or :
		string[70]; // 4*16+5

	OSTART time;
	int index;
};

#endif