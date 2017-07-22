#ifndef CAMX_GUIFONT_H
#define CAMX_GUIFONT_H 1

#ifdef WIN32
#include <afxwin.h>
#endif

class guiFont
{
public:
#ifdef WIN32
	HFONT hfont;
#endif
};

#endif
