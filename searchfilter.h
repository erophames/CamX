#ifndef SYS_SEARCHFILTER
#define SYS_SEARCHFILTER

#include "defines.h"
#include "text.h"
#include "object.h"

class SearchFilter
{
public:
	SearchFilter()
	{
		filterstring=0;
	}

	~SearchFilter()
	{
		if(filterstring)
			delete filterstring;
	}

	bool CheckString(char *s,int index);
	bool SetString(char *n);

	char *filterstring;
};

#endif