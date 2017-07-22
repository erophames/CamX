#include "searchfilter.h"
#include <string.h>
#include "songmain.h"

bool SearchFilter::SetString(char *n)
{
	if(!n)
	{
		if(filterstring)
		{		
			delete filterstring;
			filterstring=0;
			return true;
		}

		return false;
	}

	if((!filterstring) && strlen(n)==0)
		return false;

	if(filterstring)
		delete filterstring;

	if(filterstring=mainvar->GenerateString(n))
	{
		strcpy(filterstring,n);
		_strlwr(filterstring);

		//TRACE ("Filter :%s\n",filterstring);

		return true;
	}

	return false;
}

bool SearchFilter::CheckString(char *s,int index){

	if(!s)
		return false;

	if(filterstring)
	{
		size_t fsl=strlen(filterstring);

		if(strlen(s)<fsl)
			return false;

		char *s_small=new char[strlen(s)+1];

		if(!s_small)
			return false;

		strcpy(s_small,s);
		_strlwr(s_small);

		if(index==0)
		{
			if(strncmp(s_small,filterstring,fsl)==0)
			{
				delete s_small;
				return true;
			}
		}
		else
		{
			size_t smallsl=strlen(s_small)-fsl;

			for(size_t i=0;i<smallsl;i++)
				if(strncmp(&s_small[i],filterstring,fsl)==0)
				{
					delete s_small;
					return true;
				}
		}

		delete s_small;
		return false;
	}

	return true;
}