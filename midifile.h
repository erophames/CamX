#ifndef CAMX_MIDIFILE_H
#define CAMX_MIDIFILE_H 1

#include "defines.h"
#include "camxfile.h"
#include <string.h>

class MIDIPattern;
class Seq_Track;
class Seq_Event;
class Note;
class SysEx;
class Seq_Song;
class Seq_Tempo;
class Seq_Signature;
class Seq_Text;
class Seq_Marker;

class MIDIFile_Note:public Object
{
public:
	Note *note;
};

class MIDIFile
{
public:	
	enum Flags{
		FLAG_AUTOLOAD=1,
		FLAG_FINDTRACK=2,
		FLAG_CREATEAUTOSPLITRACK=4
	};

	enum RFlags{
		SYNC_EVENT=1
	};

	MIDIFile();

	bool CheckFile(camxFile *, char *filename);

	bool ReadMIDIFileToSong(Seq_Song *,char* filename,int flag=0);
	void ReadMIDIFileToPattern(Seq_Song *,Seq_Track *,MIDIPattern *,char* filename);
	bool ReadBankToPattern(MIDIPattern *);
	int ReadEventsToTrackOrPattern(Seq_Song *,Seq_Track *,MIDIPattern *,int flag=0);

	// Save
	void WriteRawEvent(Seq_Event *);
	void SavePatternToFile(MIDIPattern *,char *filename);
	void SavePatternToSysFile(MIDIPattern *,char *filename);

	void SaveSongToFile(Seq_Song *,char *filename,bool format1);

	camxFile file;

private:
	int ReadMetaEvent(Seq_Song *,Seq_Track *);
	void AddSysExData(SysEx *,int syslen);
	UBYTE getbyte();
	UWORD getword();
	int getuword();
	void setchunklen();
	bool checkHeader(char *name);
	bool checkHeaderSysEx();
	void ResetDelta(OSTART time){deltatime=time;runningstatus=0;}
	void ReadDeltaTime();
	void WriteDelta(OSTART);
	void WriteDeltaTime(OSTART);
	void WriteMeta(OSTART time,UBYTE b1,UBYTE b2);

	void DeleteNotFoundNotes();
	OSTART ReadDelta();
	void MixGlobalData(Seq_Song *,OSTART ctime);

	OList noteonevents; // Note

	Seq_Track *splittotrack[16],*sysextrack; 
	bool splittotrack_created[16];

	char songname[STANDARDSTRINGLEN+2],*ourfile,*lasttrackname;
	Seq_Marker *marker;
	Seq_Tempo *tempo;
	Seq_Signature *sig;
	Seq_Track *lastwritentrack;
	Seq_Text *text;
	MIDIPattern *topattern;
	double deltatimefactor;
	LONGLONG filelength;
	OSTART deltaoffset,deltatime,ourdeltatime;
	int chunklen,headformat,headtracknumber,solution;	// PPQ

	UBYTE trackend,runningstatus;
	bool gmgsfound,error,patternmode,sysexfile;
};
#endif