#include "audiofile.h"
#include "audiohdfile.h"
#include "audioevent.h"
#include "audiopattern.h"
#include "chunks.h"

AudioEvent::AudioEvent()
{
	id=OBJ_AUDIOEVENT;
	status=AUDIO;

	audioefile=0;
	audioregion=0;
	destructive=false;

	sampleposition=
	openseek=0;
	openseek_cycle=0;

	// Single Event
	next=0;
	prev=0;
	//iofile.nobuffer=true;

	// I/O
	io_region=false;
}

void AudioEvent::SetToRegion(AudioRegion *r)
{
	audioregion=r;
	r->r_audiohdfile=audioefile;
}

void AudioEvent::MoveEvent(OSTART diff)
{
	GetAudioPattern()->StopAllofPattern();
	GetAudioPattern()->MovePattern(diff,0);
}

void AudioEvent::MoveEventQuick(OSTART diff)
{
	GetAudioPattern()->StopAllofPattern();
	GetAudioPattern()->MovePattern(diff,0);
}

void AudioEvent::Load(camxFile *file)
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUDIOEVENT)
	{
		file->ChunkFound();

		// Old HD File
		CPOINTER o_hdfile,o_region;

		file->ReadChunk(&o_hdfile);
		file->ReadChunk(&o_region);

		//	audiohdfile=(AudioHDFile *)file->FindClass(o_hdfile);
		//	audioregion=(AudioRegion *)file->FindClass(o_region);

		if((file->flag&LOAD_DONTSETSTARTPOSITION)==0)
		{
			file->ReadChunk(&ostart);
			file->ReadChunk(&staticostart);
		}
		else
		{
			// jump over startposition
			int dummyobstart;
			int dummystaticstart;

			file->ReadChunk(&dummyobstart);
			file->ReadChunk(&dummystaticstart);
		}

		file->CloseReadChunk();
	}

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_AUDIOEVENT_EVENTREGION)
	{
		file->ChunkFound();
		file->CloseReadChunk();

		file->LoadChunk();
		if(file->GetChunkHeader()==CHUNK_AUDIOHDREGION)
		{
			file->ChunkFound();
			
			if(audioregion=new AudioRegion)
				audioregion->Load(file);
		}
	}
}

Object *AudioEvent::Clone(Seq_Song *song)
{
	if(AudioEvent *ae=new AudioEvent)
	{
		ae->ostart=ostart;
		ae->staticostart=staticostart;

		return ae;
	}

	return 0;
}

void AudioEvent::CloneData(Seq_Song *song,Seq_Event *to)
{
	//int diff=to->ostart-ostart;
	AudioEvent *ae=(AudioEvent *)to;

	ae->GetAudioPattern()->StopAllofPattern();
	to->MoveEventAbs(ostart);
}

void  AudioEvent::SeekCurrentBytes(int seekbytes)
{
	if(!seekbytes)
		return;

	if(audioefile)
	{
		if(pattern && pattern->mediatype==MEDIATYPE_AUDIO_RECORD)
		{
			// Read from writefile

			//TRACE ("Cycle Seek Read %d\n",sampleposition);

			LONGLONG seek=sampleposition;

			seek*=audioefile->samplesize_all_channels;
			seek+=audioefile->datastart;
			seek+=seekbytes;

			audioefile->LockIOSync(); // Read
			audioefile->writefile.SeekBegin(seek);
			audioefile->UnlockIOSync(); // Read
		}
		else
		if(iofile.status==CAMXFILE_STATUS_READ)
		{
#ifdef WIN32
			iofile.SeekCurrent(seekbytes);
#endif
		}
	}
}

int AudioEvent::Read(void* to,int len)
{
	if(audioefile)
	{
		if(pattern && pattern->mediatype==MEDIATYPE_AUDIO_RECORD)
		{
			// Read from writefile

			//TRACE ("Cycle Seek Read %d\n",sampleposition);

			LONGLONG seek=sampleposition;
		
			seek*=audioefile->samplesize_all_channels;
			seek+=audioefile->datastart;

			audioefile->LockIOSync(); // Read

			audioefile->writefile.SeekBegin(seek);
			len=audioefile->writefile.Read(to,len);
			
			audioefile->UnlockIOSync(); // Read


			return len;
		}

		if(iofile.status==CAMXFILE_STATUS_READ)
		{
#ifdef WIN32
			return iofile.file.Read(to,len);
#endif
		}

	}

	return 0;
}

void AudioEvent::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOEVENT);

	file->Save_Chunk((CPOINTER)audioefile);
	file->Save_Chunk((CPOINTER)audioregion);

	file->Save_Chunk(ostart);
	file->Save_Chunk(staticostart);

	file->CloseChunk();

	if(audioregion)
	{
		file->OpenChunk(CHUNK_AUDIOEVENT_EVENTREGION);
		file->CloseChunk();
		audioregion->Save(file);
	}
}

int AudioEvent::GetIndex()
{
	return 0;
}

Seq_Track *AudioEvent::GetTrack()
{
	return GetAudioPattern()->track;
}
