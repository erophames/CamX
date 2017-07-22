#ifndef CAMX_CHECKSUM
#define CAMX_CHECKSUM 1

#include "defines.h"

class checksum {
public:
	checksum() { clear(); }
	void clear() { input_checksum=sum = 0; r = 55665; c1 = 52845; c2 = 22719;}
	void add(BYTE b);
	DWORD get() { return sum; }

	DWORD input_checksum;

protected:
	WORD r,c1,c2;
	DWORD sum;

};
#endif