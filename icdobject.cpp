#include "icdobject.h"
#include "drumevent.h"
#include "camxfile.h"
#include "object_track.h"

bool ICD_Object::CheckSelectFlag(int checkflag,int icdtype)
{
	if( (checkflag&SEL_INTERN) &&
		(icdtype==type || icdtype==0) && 
		( 
		(!(checkflag&SEL_SELECTED)) || (flag&OFLAG_SELECTED))
		)
		return true;

	return false;
}

void ICD_Object::Save(camxFile *file)
{
	file->Save_Chunk(type);
	file->Save_Chunk(datalength); // Data Length

	SaveICDData(file);
}

void ICD_Object::ReadAndAddToPattern(MIDIPattern *p,camxFile *file,OSTART ostart,OSTART sstart)
{
	ICD_Object *newobject=0;

	file->ReadChunk(&type);
	file->ReadChunk(&datalength);

	switch(type)
	{
	case ICD_TYPE_DRUM:
		{
			newobject=(ICD_Object *)new ICD_Drum;
		}
		break;
	}

	if(newobject)
	{
		newobject->ostart=ostart;
		newobject->staticostart=sstart;

		newobject->LoadICDData(file);
		newobject->AddSortToPattern(p);
	}
}

// ICD_Object_Seq_MIDIChainEvent ##############################################################################
bool ICD_Object_Seq_MIDIChainEvent::CheckSelectFlag(int checkflag,int icdtype)
{
	if( (checkflag&SEL_INTERN) &&
		(icdtype==type || icdtype==0) && 
		( 
		(!(checkflag&SEL_SELECTED)) || (flag&OFLAG_SELECTED))
		)
		return true;

	return false;
}

void ICD_Object_Seq_MIDIChainEvent::Save(camxFile *file)
{
	file->Save_Chunk(type);
	file->Save_Chunk(datalength); // Data Length

	SaveICDData(file);
}

void ICD_Object_Seq_MIDIChainEvent::ReadAndAddToPattern(MIDIPattern *p,camxFile *file,OSTART ostart,OSTART sstart)
{
	ICD_Object_Seq_MIDIChainEvent *newobject=0;

	file->ReadChunk(&type);
	file->ReadChunk(&datalength);

	switch(type)
	{
	case ICD_TYPE_DRUM:
		{
			newobject=(ICD_Object_Seq_MIDIChainEvent *)new ICD_Drum;
		}
		break;
	}

	if(newobject)
	{
		newobject->ostart=ostart;
		newobject->staticostart=sstart;

		newobject->LoadICDData(file);
		newobject->AddSortToPattern(p);
	}
}

OSTART ICD_Object_Seq_MIDIChainEvent::GetPlaybackStart(MIDIPattern *p,Seq_Track *t) //v
{
	OSTART h=GetEventStart(p);
	h+=t->GetFX()->GetDelay();

	h+=realtimeswing; // Add Swing

	return h;
}