#include "audioobjects.h"
#include "chunks.h"
#include "camxfile.h"

void SeparateOuts::SO_Init()
{
	int i=0;
}

void SeparateOuts::SO_DeInit()
{
	int i=0;
}

void SeparateOuts::SO_Load(camxFile *file)
{
	if(file->GetChunkHeader()==CHUNK_AUDIOEFFECT_SEPERATEOUTS)
	{
		file->ChunkFound();

		file->CloseReadChunk();
	}
}

void SeparateOuts::SO_Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOEFFECT_SEPERATEOUTS);

	file->CloseChunk();
}

