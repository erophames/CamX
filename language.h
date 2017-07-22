#ifndef CAMX_LANGUAGE_H
#define CAMX_LANGUAGE_H 1

//#include "defines.h"
//#include "text.h"
//#include "object.h"


/*
#ifdef WIN32
#include <AFXMT.h> // Semaphores
#endif

#define LANGADD 1

class LString;

class LanguageString
{
	friend class Language;

	LanguageString()
	{
		defstring=0;
		string=0;
	}

public:
	char *defstring;
	char *string;
};

class Language
{
public:
	Language()
	{
		strings=0;
		stringcounter=0;
	};

#ifdef _DEBUG
	void CreateLanguageFiles();
	void AddString(char *string); // read
	void WriteString(LString *s);

	OList create;
#endif

	bool OpenLanguage(char *langfile,int flag);
	void CloseLanguage();
	char *GetString(char *s);

	#ifdef WIN32
	CFile file;
#endif

	LanguageString *strings;
	int stringcounter;
};
*/

#endif