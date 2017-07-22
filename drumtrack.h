#ifndef CAMX_DRUMTRACK_H
#define CAMX_DRUMTRACK_H 1

#include "object.h"
#include "MIDIoutdevice.h"
#include "audiochannel.h"
#include "colours.h"

#define DRUMTRACKNAME_LENGTH 32

class Drummap;

class Drumtrack:public Object
{
	friend class Drummap;

public:
	Drumtrack(Drummap *);

	void Load(camxFile *);
	void Save(camxFile *);

	int GetMIDIChannel(){return dtr_channel-1;} // 0-15

	bool CheckEvent(Seq_Event *);
	void SetName(char *newstring);
	char *GetName(){return name;}
	void SetChannel(int nchannel);
	void SetKey(int nkey);
	void SetVelocity(int nvelo);

	Object *Clone()
	{
		if(Drumtrack *dt=new Drumtrack(map))
		{
			CloneData(dt);
			return dt;
		}

		return 0;
	}

	void CloneData(Drumtrack *);

	Drumtrack *PrevTrack() {return (Drumtrack *)prev;}
	Drumtrack *NextTrack() {return (Drumtrack *)next;}

	// DATA ----
	char name[DRUMTRACKNAME_LENGTH+1];
	MIDIOutputDevice *MIDIoutdevice;
	Drummap *map;
	
	LONGLONG ticklength;
	
	int volume, // -127 <-> +127
		colour,index;
	
	UBYTE dtr_channel;
	char key,velocityoff;
	bool solo,mute;
};

#endif