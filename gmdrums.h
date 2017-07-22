#ifndef CAMX_GMDRUMMAP_H
#define CAMX_GMDRUMMAP_H 1

#include "defines.h"

class GMMap
{
public:
	GMMap()
	{
		int i;
		
		for(i=0;i<128;i++)
		{
			keys[i]=0;
		}

		for(i=0;i<47;i++)
		{
			keys[35+i]=gmkeynames[i];
		}

	//	UBYTE k=keys[46].key;
	};
	
	char *keys[128];// the map
};

#endif