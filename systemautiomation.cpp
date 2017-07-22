#include "systemautomation.h"
#include "chunks.h"
#include "camxfile.h"
#include "object_song.h"

// Mute
void AT_SYS_Mute::Load(camxFile *file)
{
	AutomationObject::Load(file);

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_AUTOMATIONOBJECTSPECIALDATA)
	{
		file->ChunkFound();
		file->ReadChunk(&mute);
		file->CloseReadChunk();
	}
}

void AT_SYS_Mute::Save(camxFile *file)
{
	AutomationObject::Save(file);

	file->OpenChunk(CHUNK_AUTOMATIONOBJECTSPECIALDATA);
	file->Save_Chunk(mute);
	file->CloseChunk();
}

double AT_SYS_Mute::ConvertValueToAutomationSteps(double v)
{
	if(v>=0.5)
		return 1;
	
	return 0;
}

void AT_SYS_Mute::SendNewValue(AutomationTrack *atrack)
{
	if(value==1)
	{
		if(mute==true)
			return;

		mute=true;
	}
	else
	{
		if(mute==false)
			return;

		mute=false;
	}

	automationtrack->GetSong()->SetMuteFlags();
}

char *AT_SYS_Mute::GetParmValueStringPar(int index,double par)
{
	return par==1?"Mute":"";
}

char *AT_SYS_Mute::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}
// Solo
void AT_SYS_Solo::Load(camxFile *file)
{
	AutomationObject::Load(file);

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_AUTOMATIONOBJECTSPECIALDATA)
	{
		file->ChunkFound();
		file->ReadChunk(&solo);
		file->CloseReadChunk();
	}
}

void AT_SYS_Solo::Save(camxFile *file)
{
	AutomationObject::Save(file);

	file->OpenChunk(CHUNK_AUTOMATIONOBJECTSPECIALDATA);
	file->Save_Chunk(solo);
	file->CloseChunk();
}

void AT_SYS_Solo::SendNewValue(AutomationTrack *atrack)
{	
	if(value==1)
	{
		if(solo==true)
			return;

		solo=true;
	}
	else
	{
		if(solo==false)
			return;

		solo=false;
	}

	automationtrack->GetSong()->SetMuteFlags();
}

double AT_SYS_Solo::ConvertValueToAutomationSteps(double v)
{
	if(v>=0.5)
		return 1;
	
	return 0;
}

char *AT_SYS_Solo::GetParmValueStringPar(int index,double par)
{
	return par==1?"Solo":"";
}

char *AT_SYS_Solo::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}