#include "ex_wave.h"

// SYSTEM Read,Write,Seek
#ifdef WIN32
bool WaveFile::FILE_OpenRead(char *filename)
{
	if(file.Open(filename,CFile::shareDenyWrite,&exec))
		return true;

	return false;
}

bool WaveFile::FILE_OpenSave(char *filename)
{
	if(wfile.Open(filename,CFile::modeCreate|CFile::modeReadWrite))
	{
		writeopen=true;

		return true;
	}

	return false;
}

LONGLONG WaveFile::FILE_GetSize()
{
	return file.GetLength();
}

bool WaveFile::FILE_Read(void *to,int size)
{
	file.Read(to,size);

	return true;
}

bool WaveFile::FILE_Write(void *from,int size)
{
	wfile.Write(from,size);

	return true;
}

ULONGLONG WaveFile::FILE_GetCurrentPos()
{
	return file.Seek(0,CFile::current);
}

void WaveFile::FILE_SeekCurrent(LONGLONG seek)
{
	file.Seek(seek,CFile::current);	
}

void WaveFile::FILE_SeekBegin(LONGLONG seek)
{
	file.Seek(seek, CFile::begin );
}

void WaveFile::FILE_SeekSaveBegin(LONGLONG seek)
{
	wfile.Seek(seek, CFile::begin );
}

void WaveFile::FILE_ReadFromSave(void *to,int size)
{
	wfile.Read(to,size);
}

void WaveFile::FILE_Close()
{
	file.Close();
}

void WaveFile::FILE_CloseSave()
{
	wfile.Close();
}

void WaveFile::FILE_DeleteFile(char *filename)
{
	DeleteFile(filename);
}

// end Win 32
#endif

#ifdef QT

// QT File I/O


// end QT
#endif