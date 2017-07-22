#include "objectpattern.h"
#include "object_song.h"
#include "chunks.h"
#include "camxfile.h"

void PatternLink::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);
	int nr=0;
	file->ReadChunk(&nr);

	while(nr--)
	{
		if(PatternLink_Pattern *plp=new PatternLink_Pattern)
		{
			file->AddPointer((CPOINTER)&plp->pattern);
			patternlinklist.AddEndO(plp);
		}
	}

	file->CloseReadChunk();
}

void PatternLink::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_SONGPATTERNLINK);
	file->Save_Chunk((CPOINTER)this);

	file->Save_Chunk(patternlinklist.GetCount());

	PatternLink_Pattern *plp=FirstLinkedPattern();

	while(plp){
		file->Save_Chunk((CPOINTER)plp->pattern);
		plp=plp->NextLink();
	}

	file->CloseChunk();
}

PatternLink_Pattern *PatternLink::AddPattern(Seq_Pattern *p)
{
	if(!FindPattern(p))
	{
		if(PatternLink_Pattern *plp=new PatternLink_Pattern)
		{
			p->link=this;
			plp->pattern=p;
			patternlinklist.AddEndO(plp);

			return plp;
		}
	}

	return 0;
}

bool PatternLink::FindPattern(Seq_Pattern *p)
{
	PatternLink_Pattern *f=FirstLinkedPattern();

	while(f)
	{
		if(f->pattern==p)
			return true;

		f=f->NextLink();
	}

	return false;
}

bool PatternLink::RemovePattern(Seq_Pattern *p)
{
	bool closed=false;

	PatternLink_Pattern *f=FirstLinkedPattern();

	while(f)
	{
		if(f->pattern==p)
		{
			p->link=0;
			patternlinklist.RemoveO(f);
			break;
		}

		f=f->NextLink();
	}

	if(patternlinklist.GetCount()<2) // Links Empty or 1 Left ?
	{
		if(PatternLink_Pattern *f=FirstLinkedPattern())
		{	
			f->pattern->link=0;
			patternlinklist.RemoveO(f);
		}

		song->DeletePatternLink(this);
		closed=true;
	}

	return closed;
}

void Seq_Song::LoadLinks(camxFile *file)
{
	int nrlinks=0;
	file->ReadChunk(&nrlinks);
	file->CloseReadChunk();

	while(nrlinks--)
	{
		file->LoadChunk();

		if(file->CheckReadChunk()==false)
			break;

		if(file->GetChunkHeader()==CHUNK_SONGPATTERNLINK)
		{
			file->ChunkFound();

			if(PatternLink *pl=CreatePatternLink())
				pl->Load(file);
			else
				file->CloseReadChunk();
		}
	}
}

void Seq_Song::SaveLinks(camxFile *file)
{
	if(FirstPatternLink())
	{
		file->OpenChunk(CHUNK_SONGPATTERNLINKLIST);
		file->Save_Chunk(patternlinks.GetCount());
		file->CloseChunk();

		PatternLink *pl=FirstPatternLink();

		while(pl){
			pl->Save(file);
			pl=pl->NextPatternLink();
		}
	}
}
