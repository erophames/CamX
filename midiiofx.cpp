#include "object_trackhead.h"
#include "camxfile.h"
#include "chunks.h"

void MIDIIOFX::Load(camxFile *file)
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_MIDIIOFX)
	{
		file->ChunkFound();
		file->CloseReadChunk();

		velocity.Load(file);
		mvolume.Load(file);
	}
}

void MIDIIOFX::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_MIDIIOFX);
	file->CloseChunk();

	velocity.Save(file);
	mvolume.Save(file);
}

void MIDIIOFX::SetVelocity(int nv,OSTART automationtime)
{
	if(nv<-127)
		nv=-127;
	else
		if(nv>127)
			nv=127;

	if(GetVelocity()!=nv)
	{
		double h2=nv;// 0-254

		h2+=127;
		h2/=254;

		velocity.AutomationEdit(trackhead->song,automationtime,0,h2,AEF_USEREDIT);
	}
}