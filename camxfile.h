#ifndef CAMX_CAMXFILE
#define CAMX_CAMXFILE 1

#include "object.h"

#ifdef WIN32
#include <AFXMT.h> // file
#endif

#include "checksum.h"

#define CAMXFILE_STATUS_NOTOPEN 0
#define CAMXFILE_STATUS_READ 1
#define CAMXFILE_STATUS_SAVE 2
#define CAMXFILE_STATUS_READSAVE (CAMXFILE_STATUS_READ|CAMXFILE_STATUS_SAVE)

#define CHUNKBUFFERSIZE 16384

class guiWindow;
class guiScreen;

class RChunk:public Object
{
public:
	RChunk()
	{
		unknown=true;
		overrun=false;
	}

	LONGLONG chunklen,staticchunklen;
	int chunkheader; //32 bit
	bool unknown,overrun;
};

class Chunk:public Object
{
public:
	Chunk(long cnr)
	{
		chunknumber=cnr;
		chunkbuffersize=0;
		chunklen=0;
		writedata=0;
	}

	void Write(camxFile *);

	LONGLONG chunklen;
	size_t chunkbuffersize;
	char *chunkpointer,*writedata;
	long chunknumber; //32 bit
};

class Pointer:public Object
{
public:
	Pointer()
	{
		autorefresh=true;
	}

	CPOINTER oldpointer,newpointer;
	bool autorefresh;
	Pointer *NextPointer(){return (Pointer *)next;}
};

class ClassPointer:public Object
{
public:
	CPOINTER oldclass,newclass;
};

#define CAMXBUFFERSIZE 4096

class camxScan:public Object
{
public:
	camxScan()
	{
		flag=0;
	}

	camxScan *NextScan(){return (camxScan *)next;}
	LONGLONG nFileSizeLow;
	char *name,*filename;
};

class camxFile
{
public:
	camxFile(){Init();};
	camxFile(char *);
	~camxFile();

	enum FileTypes // see filestypes
	{
		FT_WAVES,
		FT_TEMPOMAP,
		FT_MIDIFILE,
		FT_QUANTIZE,
		FT_PROJECT,
		FT_SONGS,
		FT_ENCODED,
		FT_WAVES_EX,
		FT_MIDIFILTER,
		FT_VSTDUMP,
		FT_SYSEX
	};

	char *AllFiles(int type);

	void BuildDirectoryList(char *dir,char *msk,char *fileext,bool *stop=0);
	void ListDirectoryContents1(char *dir,char *msk,char *fileext,bool *stop=0);

	bool CheckVersion();
	int GetVersion();

	void SaveVersion();
	void Init();

	bool CheckReadChunk()
	{	
		if((!readchunk) || readchunk->overrun==true)return false;
		return true;	
	}

	void RenewPointer();

	ClassPointer *AddClass(CPOINTER newclass,CPOINTER oldclass);
	void DeleteAddedClass(CPOINTER pointer);

	// Pointer
	Pointer *FirstPointer(){return (Pointer *)pointer.GetRoot();}
	Pointer *AddPointer(CPOINTER newpointer);

	CPOINTER FindClass(CPOINTER oldclass);
	ClassPointer *ChangeClass(CPOINTER from,CPOINTER to);
	void ReadAndAddClass(CPOINTER thispointer);

	void SetFileSize(LONGLONG size);
	void JumpOverChunk();
	void AddToFileName(char *);
	int CompareDate (camxFile *);

	bool OpenRead(char *);
	bool OpenSave(char *);
	bool OpenSave_CheckVersion(char *);
	bool OpenReadSave(char *);

	void Close(bool full);
	void Flush(); // Write Buffer

	bool SelectDirectory(guiWindow *,guiScreen *,char *title);
	bool OpenFileRequester(guiScreen *,guiWindow *,char *title,char *filter,bool openreq,char *defaultname=0);

	bool CopyFileFromTo(char *from,char *to);

	int ReadChunk(int *);
	int ReadChunk(long *);
	int ReadChunk(float *);
	int ReadChunk(double *);
	int ReadChunk(ULONGLONG *);
	int ReadChunk(LONGLONG *);
	int ReadChunk(char *);
	int ReadChunk(UBYTE *);
	int ReadChunk(bool *);
	int ReadChunk(void *,int length);

	int Read(long *);
	int Read(float *);
	int Read(long double);
	int Read(double *);
	int Read(unsigned long *);
	int Read(char *);
	int Read(ULONGLONG *);
	int Read(LONGLONG *);
	int Read(bool *);
	int Read(int *);
	int Read(void *,int length);

	void Save_Chunk(void *from,size_t length);
	void Save_Chunk(ULONGLONG);
	void Save_Chunk(LONGLONG);

	void Save_Chunk(unsigned long);
	void Save_Chunk(long);

	void Save_Chunk(unsigned char);
	void Save_Chunk(char);

	void Save_Chunk(unsigned short);	
	void Save_Chunk(short);

	void Save_Chunk(int v);
	void Save_Chunk(bool bv);
	void Save_Chunk(float v);
	void Save_Chunk(double v);

	void Read_ChunkString(char *); // static char array
	void Read_ChunkString(char **); // dynamic
	void Save_ChunkString(char *);
	void Save(void *,size_t length);
	void Save(char *string);

	// Save
	void OpenChunk();
	void OpenChunk(int);
	void CloseChunk(); // Close And Write

	// Standard Write
	void writebyte(UBYTE);
	void writeword(unsigned short);

	// Chunk Write
	LONGLONG GetLength();
	void GetDate();

	void SeekBegin(LONGLONG offset);
	void SeekEnd(LONGLONG offset);
	LONGLONG SeekCurrent(LONGLONG offset);

	void ChunkFound();
	void LoadChunk();

	int GetChunkHeader(){return readchunk?readchunk->chunkheader:0;}

	void CloseReadChunk();
	void DeleteFile();

	// Scan
	camxScan *FirstScan(){return (camxScan*)scandirs.GetRoot();}
	void ClearScan();

	OList scandirs;

#ifdef WIN32
	CFileException exec;
	CFile file;
#endif

	LONGLONG filelength;
	char *filereqname,*fname,*fpath,*helpstring1;
	int audiofilesadded,date_year,date_month,date_day,date_hour,date_min,date_second,
		status,flag,specialflags,errorflag,helpflag;
	bool camxfile,MIDIfile,usesum,eof,errorwriting;

private:
	OList pointer,classpointer;
	checksum csum;
	Chunk *writechunk;
	RChunk *readchunk;
	char *wildname;
	bool writechunkerror;
};
#endif