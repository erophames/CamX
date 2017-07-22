#ifndef CAMX_DECODER_H
#define CAMX_DECODER_H 1

/*
#include "object.h"

class DecodedData:public Object
{
public:
	char *data;
	long length;
};
*/
#ifdef ARES64
#define ARES double
#else
#define ARES float
#endif

class guiWindow;

class Decoder
{
public:
	Decoder();
	void Close();
	void ConvertToARES(ARES *to,int samples);

	bool CheckFile(char *name);

	char *file;
	int channels;
	int samplerate;
	int samplebits;

	int filesize;
	int fileread;

	guiWindow *win;

	ULONGLONG samples;
	
	char *decodeddata;
	int decodeddata_bytes;
	int decodeddata_samples;
	LONGLONG sum_decodeddata_bytes;

	bool (*FirstData) (Decoder *decoder);
	bool (*WriteData) (Decoder *decoder);
	bool (*WriteDataEnd) (Decoder *decoder);

	bool stop;
	// OList data;
};
#endif