#include "audiochannel.h"
#include "audiosend.h"
#include "camxfile.h"
#include "audioports.h"
#include "chunks.h"
#include "audiosystem.h"
#include "audiodevice.h"

AudioIOFX::AudioIOFX()
{
	channel_type=CHANNELTYPE_STEREO;

	out_vchannel=in_vchannel=0;
	outvchannel_index=invchannel_index=0;
	
	outvchannel_channels=invchannel_channels=-1;

	tempinputmonitoring=inputmonitoring=false;

	bypass_input=false;
	bypassallfx=bypassallinputfx=false;
	thru=skipthru=false;

	audiosystem=0;

	audioeffects.io=this;
	audioeffects.instrumentspossible=true;

	audioinputeffects.io=this;
	audioinputeffects.instrumentspossible=true;

	audioinputenable=false;

	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
	{
		inchannelindex[i]=0;
		outchannelindex[i]=0;
	}
}

void AudioIOFX::FreeMemory()
{
	inputpeaks.FreeMemory();
	//outpeaks.FreeMemory();

	// Remove Trigger
	//audioeffects.ClearAllTrigger();

	// Remove Effects
	audioeffects.FreeMemory();
	audioinputeffects.FreeMemory();

	// Delete Sends
	AudioSend *send=FirstSend();
	while(send)send=(AudioSend *)sends.RemoveO(send);
}

void AudioIOFX::Load(camxFile *file)
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUDIOIOFX)
	{
		file->ChunkFound();

		file->ReadChunk(&thru);
		file->ReadChunk(&bypassallfx);
		file->ReadChunk(&bypass_input);

		file->ReadChunk(&invchannel_index);
		file->ReadChunk(&invchannel_channels);

		// Out
		file->ReadChunk(&outvchannel_index);
		file->ReadChunk(&outvchannel_channels);

		file->ReadChunk(&channel_type);

		file->ReadChunk(&inputmonitoring);
		file->ReadChunk(&bypassallinputfx);
		file->ReadChunk(&audioinputenable);

		for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
		{
			file->ReadChunk(&inchannelindex[i]);
			file->ReadChunk(&outchannelindex[i]);
		}

		file->CloseReadChunk();

		audioeffects.Load(file);

		file->LoadChunk();

		if(file->GetChunkHeader()==CHUNK_AUDIOCHANNEL_SEND)
			LoadSend(file);

		audioinputeffects.Load(file);
	}
}

void AudioIOFX::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOIOFX);

	file->Save_Chunk(thru);
	file->Save_Chunk(bypassallfx);
	file->Save_Chunk(bypass_input);

	// In 
	InitIOOldChannel();

	file->Save_Chunk(invchannel_index);
	file->Save_Chunk(invchannel_channels);

	// Out
	file->Save_Chunk(outvchannel_index);
	file->Save_Chunk(outvchannel_channels);

	file->Save_Chunk(channel_type);
	file->Save_Chunk(inputmonitoring);
	file->Save_Chunk(bypassallinputfx);
	file->Save_Chunk(audioinputenable);

	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
	{
		file->Save_Chunk(inchannelindex[i]);
		file->Save_Chunk(outchannelindex[i]);
	}

	file->CloseChunk();

	audioeffects.Save(file);// Output VST,Intern etc...
	SaveSend(file);
	audioinputeffects.Save(file);
}

void AudioIOFX::Repair()
{
	// In 
	if(invchannel_channels>=MAXCHANNELSPERCHANNEL)
		invchannel_channels=-1; 

	//Out
	if(channel_type>=MAXCHANNELSPERCHANNEL)
		channel_type=CHANNELTYPE_STEREO;

	if(audiosystem->device)
	{
		in_vchannel=audiosystem->device->FindVInChannel(invchannel_channels,invchannel_index);
		out_vchannel=audiosystem->device->FindVOutChannel(outvchannel_channels,outvchannel_index);
	}
	else
	{
		in_vchannel=0;
		out_vchannel=0;
	}

	audioeffects.RefreshBuffer(audiosystem->device);
	audioinputeffects.RefreshBuffer(audiosystem->device);
}

bool AudioIOFX::Compare(AudioIOFX *c)
{
	/*
	if(!c)return false;

	if(c->in_vchannel!=in_vchannel)return false;
	if(c-
		// In 
	invchannel_index=in_vchannel?in_vchannel->index:0; // default Mono
	invchannel_channels=in_vchannel?in_vchannel->channels:-1; // -1 not set

	file->Save_Chunk(invchannel_index);
	file->Save_Chunk(invchannel_channels);

	// Out
	outvchannel_index=out_vchannel?out_vchannel->index:0;
	*/

	return true;
}
