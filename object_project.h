#ifndef CAMX_PROJECTOBJECTS_H
#define CAMX_PROJECTOBJECTS_H 1

#include "object.h"
#include "panlaw.h"

class Seq_Song;
class guiScreen;

#define DIRECTORY_FREEZETRACKS "Freeze Tracks"

enum Pro_MeasureFormat{
	PM_1111,
	PM_1p1p1p1,
	PM_1110,
	PM_1p1p1p0,
	PM_11_1,
	PM_1p1p_1,
	PM_11_0,
	PM_1p1p_0
};

class Seq_Project:public Object
{
	friend class Seq_Main;

public:
	enum{
		CREATESONG_ACTIVATE=1,
		CREATESONG_CREATEAUDIOMASTER=2,
		CREATESONG_LOAD=4,
		CREATESONG_NOACTIVATE=8,
		CREATESONG_NONEWDIRECTORY=16,
		CREATESONG_IMPORTFROMFILE=32,
		CREATESONG_NOGUIREFRESH=64
	};

	enum{
		DELETESONG_FULL=1,
		DELETESONG_SONGWASACTIVE=2,
		DELETESONG_NOLOCK=4,
		DELETESONG_ONLYCLOSE=8
	};

	Seq_Project();

	void InitPPQ();
	void RefreshRealtime_Slow();
	void FreeMemory();
	int GetCountOfAudioPattern();
	
	bool OpenSong(guiScreen *screen=0);
	Seq_Song *ImportMIDIFile(guiScreen *screen=0);

	bool CheckNewSampleRateIsOk(int nsrate);
	void SetSampleRate(int srate);
	void InitSongsTimeTrackPPQ();

	void Load(camxFile *); // v
	void Save(camxFile *); // v
	void SaveAllSongs();
	bool CheckIfFileIsSong(camxFile *);
	
	Seq_Song *FirstSong(){return (Seq_Song *)songs.GetRoot();}
	Seq_Song *FirstOpenSong();
	Seq_Song *LastSong(){return (Seq_Song *)songs.Getc_end();}
	Seq_Song *GetSongIndex(int index){return (Seq_Song *)songs.GetO(index);}
	
	int GetCountOfSongs(){return songs.GetCount();}
	int GetCountOfOpenSongs();
	Seq_Song *CreateNewSong(char *name,int flag,Seq_Song *prevsong,char *filename); 
	void AddSong(Seq_Song *);
	Seq_Song *DeleteSong(Seq_Song *,int flag);
	
	void CloseAllButSong(guiScreen *,Seq_Song *);
	void CloseAllSongs();
	void SetProjectName(char *,bool refreshgui);
	Seq_Project *PrevProject() {return (Seq_Project *)prev;}	
	Seq_Project *NextProject() {return (Seq_Project *)next;}

	// Audio
	PanLaw panlaw;

	char *projectdirectory,name[STANDARDSTRINGLEN+2];
	Seq_Song *loadactivesong;
	double checkaudiothreshold,checkaudioendthreshold,ppqsampleratemul;
	int projectsamplerate,projectmeasureformat,standardsmpte,realtimerecordtempoevents;
	bool loadok,checkaudiostartpeak,autocutzerosamples,underdestruction,editprojectname,errorprojectwriting;

private:
	OList songs;
	Seq_Song *activesong; // working songs
	int songcounter;
};
#endif