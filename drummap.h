#ifndef CAMX_DRUMMAP_H
#define CAMX_DRUMMAP_H 1

#include "object.h"

class Seq_Song;
class ICD_Drum;

class Drummap:public Object
{
	friend class mainMIDIBase;
	friend class Drumtrack;

public:
	Drummap();
	char *GetName();
	bool SetName(char *string); // return false=change
	int GetIndexOfTrack(Drumtrack *);
	Drumtrack *GetTrackIndex(int index){return (Drumtrack *)tracks.GetO(index);}
	void RemoveDrumtrackFromSongs(Drumtrack *);
	void Load(camxFile *);
	void Save(camxFile *);

	void RefreshAllEditors(bool refreshmixlist=false);
	void InitGMDrumMap();
	void FreeMemory();
	void SetTrackIndexs();
	int GetCountOfTracks();
	bool CheckIfDrumEvent(Drumtrack *start,Drumtrack *end,ICD_Drum *);

	Drumtrack *GetTrackWithIndex(int n);

	Drumtrack* CreateDrumTrack(int position);
	Drumtrack* DeleteDrumTrack(Drumtrack *);

	void DeleteAllDrumTracks();

	Drumtrack* FirstTrack() {return (Drumtrack *)tracks.GetRoot();}
	Drumtrack* LastTrack() {return (Drumtrack *)tracks.Getc_end();}

	Drummap *NextMap() {return (Drummap *)next;}

	void ResetSolo();

	char *dmfilename,name[STANDARDSTRINGLEN+2];
	Seq_Song *song;
	bool usegm,solomode,readcheck;

private:
	OList tracks;
};
#endif
