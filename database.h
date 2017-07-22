#ifndef DATABASE_CAMX
#define DATABASE_CAMX

#include "object.h"
#include "defines.h"

class DataBaseEntry:public Object
{
public:
	int type;

	char *data;
	int size;
};

class DataBase{

public:

	bool AddEntry(DataBaseEntry *);
	DataBaseEntry *AddData(char *,int size);
};
#endif