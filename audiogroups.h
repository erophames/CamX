#ifndef CAMX_AUDIOGROUPS
#define CAMX_AUDIOGROUPS 1

#include "object.h"

class Seq_AudioIOPointer:public Object
{
public:
	Seq_AudioIOPointer()
	{
		channel=0;
		bypass=false;
	}

	AudioChannel *channel;
	bool bypass;

	Seq_AudioIOPointer *NextChannel(){return (Seq_AudioIOPointer *)next;}
	Seq_AudioIOPointer *PrevChannel(){return (Seq_AudioIOPointer *)prev;}
};

class Seq_AudioIO
{
public:
	Seq_AudioIO()
	{
		track=0;
		defaultchannel=0;
		defaultrecordchannel=0;
	}

	bool CompareWithGroup(Seq_AudioIO *);
	Seq_AudioIOPointer *FirstChannel(){return (Seq_AudioIOPointer *)busgroups.GetRoot();}
	Seq_AudioIOPointer *LastChannel(){return (Seq_AudioIOPointer *)busgroups.Getc_end();}
	Seq_AudioIOPointer *FirstOutputBus();
	int GetCountGroups(){return busgroups.GetCount();}

	Seq_AudioIOPointer *AddToGroup(AudioChannel *);
	Seq_AudioIOPointer *AddToGroup(AudioChannel *,int index);

	void RemoveBusFromGroup(AudioChannel *);
	void ReplaceChannel(AudioChannel *,AudioChannel *);
	void Delete();
	bool FindChannel(AudioChannel *);
	void CloneToGroup(Seq_AudioIO *);
	bool Compare(Seq_AudioIO *);

	AudioChannel *DefaultChannel(){return defaultchannel;}
	AudioChannel *DefaultRecordChannel(){return defaultrecordchannel;}

	OList busgroups;
	AudioChannel *defaultchannel,*defaultrecordchannel;
	Seq_Track *track;
};

#endif
