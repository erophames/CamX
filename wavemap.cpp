#include "wavemap.h"
#include "chunks.h"
#include "camxfile.h"

void WaveTrack::FreeMemory()
{
	if(info)
	{
		delete info;
		info=0;
	}
}

void WaveTrack::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);
	
	file->Read_ChunkString(name);
	file->Read_ChunkString(&info);

	// data
	file->ReadChunk(&type);
	file->ReadChunk(&colour);
	file->ReadChunk(&maxvalue);
	
	file->ReadChunk(&status);
	file->ReadChunk(&chl);
	file->ReadChunk(&byte1);
	file->ReadChunk(&byte2);
	
	file->CloseReadChunk();
}

void WaveTrack::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_WAVETRACK);
	file->Save_Chunk((CPOINTER)this);
	
	file->Save_ChunkString(name);
	file->Save_ChunkString(info);

	// data
	file->Save_Chunk(type);
	file->Save_Chunk(colour);
	file->Save_Chunk(maxvalue);
	
	file->Save_Chunk(status);
	file->Save_Chunk(chl);
	file->Save_Chunk(byte1);
	file->Save_Chunk(byte2);
	
	file->CloseChunk();
}

WaveTrack* WaveMap::DeleteTrack(WaveTrack *t)
{
	t->FreeMemory();

	return (WaveTrack *)tracks.RemoveO(t);
}

void WaveMap::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);
	file->Read_ChunkString(name);

	int tr=0;

	file->ReadChunk(&tr);
	file->CloseReadChunk();

	while(tr--)
	{
		file->LoadChunk();
		
		if(file->CheckReadChunk()==false)
			break;
		
		if(file->GetChunkHeader()==CHUNK_WAVETRACK)
		{
			file->ChunkFound();
			
			WaveTrack *t=AddNewWaveTrack();
			
			if(t)
			{
				t->Load(file);
			}
			else
				file->CloseReadChunk();
		}
	}
}


WaveTrack *WaveMap::AddNewWaveTrack(WaveTrack *prev,WaveTrack *clonetrack)
{
	WaveTrack *newtrack=clonetrack?clonetrack->CloneTrack():new WaveTrack;
	
	if(newtrack)
	{
		if(prev)
			tracks.AddPrevO(newtrack,prev);
		else
			tracks.AddEndO(newtrack);
	}

	return newtrack;
}

void WaveMap::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_WAVEMAP);
	file->Save_Chunk((CPOINTER)this);

	file->Save_ChunkString(name);

	file->Save_Chunk(tracks.GetCount());
	file->CloseChunk();
	
	// Tracks ...
	WaveTrack *t=FirstTrack();
	while(t){
		t->Save(file);	
		t=t->NextTrack();
	}
}

WaveMap *mainWaveMap::DeleteWaveMap(WaveMap *def)
{
	WaveTrack *t=def->FirstTrack();
	
	while(t)
		t=def->DeleteTrack(t);
	
	return (WaveMap *)maps.RemoveO(def);
}

void mainWaveMap::DeleteAllWaveMaps()
{
	WaveMap *d=FirstWaveMap();
	
	while(d)
		d=DeleteWaveMap(d);
}

void mainWaveMap::InitDefaultWaveDefinitions()
{
	WaveMap *w=AddWaveMap();
	
	if(w)
	{
		WaveTrack *ewt=w->FirstTrackType();
		
		while(ewt){
			w->AddNewWaveTrack(0,ewt);
			ewt=ewt->NextTrack();
		}
	}
}

WaveMap* mainWaveMap::AddWaveMap()
{
	int id=0;
	WaveMap *d=LastWaveMap();
	
	if(d)id=d->id_number+1;
	
	if(d=new class WaveMap){
		maps.AddEndO(d);
		d->id_number=id;	
	}
	
	return d;
}

void mainWaveMap::Load(camxFile *file)
{
	int nrmaps=0;
	file->ReadChunk(&nrmaps);
	file->CloseReadChunk();
	
	while(nrmaps--)
	{
		file->LoadChunk();
		
		if(file->CheckReadChunk()==false)break;
		
		if(file->GetChunkHeader()==CHUNK_WAVEMAP)
		{
			file->ChunkFound();
			
			WaveMap *nwd=AddWaveMap();
			
			if(nwd)
				nwd->Load(file);
			else
				file->CheckReadChunk();
		}
	}
}

void mainWaveMap::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_WAVEMAPHEADER);
	file->Save_Chunk(maps.GetCount());
	file->CloseChunk();
	
	WaveMap *t=FirstWaveMap();
	while(t){
		t->Save(file);
		t=t->NextMap();
	}	
}
