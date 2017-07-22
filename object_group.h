#ifndef CAMX_GROUP_H
#define CAMX_GROUP_H 1

//#include "defines.h"
#include "object.h"
#include "colourrequester.h"

class Seq_Group:public Object
{
public:
	Seq_Group()
	{
		mute=false;
		solo=false;
		rec=false;
		strcpy(name,"Group");
	}

	void SetName(char *);
	void Load(camxFile *);
	void Save(camxFile *);

	Seq_Group *NextGroup(){return (Seq_Group *)next;}
	Seq_Group *PrevGroup(){return (Seq_Group *)prev;}

	Seq_Song *song;
	bool mute,solo,rec;
	Colour colour;
	char name[STANDARDSTRINGLEN+1];	
};

class Seq_Group_GroupPointer:public Object
{
public:
	Seq_Group_GroupPointer()
	{
		underdeconstruction=0;
	}

	Seq_Group *group;
	bool underdeconstruction;

	Seq_Group_GroupPointer *NextGroup(){return (Seq_Group_GroupPointer *)next;}
};

class Seq_Group_Group
{
public:
	Seq_Group_Group()
	{
		activegroup=0;
	}

	Seq_Group_GroupPointer *FirstGroup(){return (Seq_Group_GroupPointer *)groups.GetRoot();}
	Seq_Group_GroupPointer *LastGroup(){return (Seq_Group_GroupPointer *)groups.Getc_end();}
	int GetCountGroups(){return groups.GetCount();}

	Seq_Group_GroupPointer *AddToGroup(Seq_Group *);
	void RemoveBusFromGroup(Seq_Group *);
	void Delete();

	bool FindGroup(Seq_Group *g)
	{
		Seq_Group_GroupPointer *sgp=FirstGroup();

		while(sgp){
			if(sgp->group==g)return true;
			sgp=sgp->NextGroup();
		}

		return false;
	}

	inline bool CheckIfPlaybackIsAbled()
	{
		Seq_Group_GroupPointer *sgp=FirstGroup();

		while(sgp){
			if(sgp->group->mute==true)return true;
			sgp=sgp->NextGroup();
		}

		return false;
	}

	inline bool CheckIfSolo()
	{
		Seq_Group_GroupPointer *sgp=FirstGroup();

		while(sgp){
			if(sgp->group->solo==true)return true;
			sgp=sgp->NextGroup();
		}

		return false;
	}

	bool CheckIfRecord()
	{
		Seq_Group_GroupPointer *sgp=FirstGroup();

		while(sgp){
			if(sgp->group->rec==true)return true;
			sgp=sgp->NextGroup();
		}

		return false;
	}

	void CloneToGroup(Seq_Group_Group *sg)
	{
		sg->Delete();

		Seq_Group_GroupPointer *fd=FirstGroup();

		while(fd)
		{
			sg->AddToGroup(fd->group);
			fd=fd->NextGroup();
		}
	}

	Seq_Group *activegroup;
	OList groups;
};

#endif