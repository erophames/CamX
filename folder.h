#ifndef CAMX_FOLDER
#define CAMX_FOLDER 1

/*
#include "object.h"
#include "camxfile.h"
#include "object_song.h"
#include "object_track.h"
#include "objectpattern.h"

class Folder:public OStart
{
public:
	Seq_Song *song;
	OSTART end;
	
	Folder()
	{
		end=-1;
	}

	void AddPattern(Seq_Pattern *p);
	
	Folder *NextFolder() {return (Folder *)next;}

	Folder *DeleteFolder();

	Seq_Pattern* FirstFolderPattern() {return (Seq_Pattern *)pattern.GetRoot(); }
	Seq_Pattern* LastFolderPattern() {return (Seq_Pattern *)pattern.Getc_end(); } 
	
	OListStart pattern;

	OSTART GetFolderStart(){return ostart;}
	OSTART GetFolderEnd(){return end;}
};
*/
#endif
