#include "audiosend.h"
#include "audioiofx.h"
#include "audiochannel.h"
#include "audiohardware.h"
#include "audiosystem.h"
#include "object_song.h"
#include "audioports.h"
#include "chunks.h"
#include "camxfile.h"
#include "audiodevice.h"

void AudioIOFX::DoAudioChannelSends(AudioEffectParameter *par,bool post) // Post
{
	AudioSend *s=FirstSend();

	while(s){

		if(s->sendbypass==false && s->sendpost==post) // Pre:: Vor Fader, Post Nach Fader
		{
			// Add Buffer To Send Bus
			if(AudioChannel *sendchannel=s->sendchannel)
			{
				//if(sendchannel->GetVOut() && sendchannel->Muted()==false)
				//{

				// Add Send Volume
				if(s->sendvolume>0)
				{
					sendchannel->FuncMixLock();
					par->in->ExpandMixAudioBufferVolume(&sendchannel->mix,sendchannel->GetVOut()->channels,s->sendvolume,0,0);
					sendchannel->FuncMixUnLock();
				}

				//}
			}
		}

		s=s->NextSend();
	}
}

bool AudioIOFX::CheckSendPre()
{
	AudioSend *send=FirstSend();

	while(send){

		if(//send->sendpost==false &&
			send->sendchannel && 
			send->sendchannel->GetVOut() && 
			send->sendchannel->Muted()==false
			)
			return true;

		send=send->NextSend();
	}

	return false;
}

bool AudioIOFX::CheckSend()
{
	AudioSend *send=FirstSend();

	while(send){

		if(send->sendchannel && 
			send->sendchannel->GetVOut() && 
			send->sendchannel->Muted()==false
			)
			return true;

		send=send->NextSend();
	}

	return false;
}

void AudioIOFX::SaveSend(camxFile *file)
{
	if(sends.GetCount()>0)
	{
		file->OpenChunk(CHUNK_AUDIOCHANNEL_SEND);
		file->Save_Chunk(sends.GetCount());

		AudioSend *send=FirstSend();

		while(send){
			file->Save_Chunk((CPOINTER)send->sendchannel);
			file->Save_Chunk(send->sendpost);
			file->Save_Chunk(send->sendvolume);
			file->Save_Chunk(send->sendbypass);
			send=send->NextSend();
		}

		file->CloseChunk();
	}
}

void AudioIOFX::LoadSend(camxFile *file)
{
	file->ChunkFound();
	int nr=0;
	file->ReadChunk(&nr);

	while(nr--)
	{
		AudioSend *newsend=new AudioSend(0);

		if(newsend){
			file->AddPointer((CPOINTER)&newsend->sendchannel);// <-> Bus
			file->ReadChunk(&newsend->sendpost);
			file->ReadChunk(&newsend->sendvolume);
			file->ReadChunk(&newsend->sendbypass);
			sends.AddEndO(newsend);
			sends.Close();
		}
	}

	file->CloseReadChunk();
}

AudioSend *AudioIOFX::FirstSend()
{
	if(audiosystem && audiosystem->song->mastering==true)
		return 0; // No Send while mastering

	return (AudioSend *)sends.GetRoot();
}

AudioSend *AudioIOFX::FindSend(AudioChannel *bus)
{
	AudioSend *s=(AudioSend *)sends.GetRoot();

	while(s){
		if(s->sendchannel==bus)
			return s;

		s=s->NextSend();
	}

	return 0;
}

void AudioIOFX::SetInput(AudioPort *port)
{
	in_vchannel=port;

	// In 
	invchannel_index=in_vchannel?in_vchannel->portindex:0;
	invchannel_channels=in_vchannel?in_vchannel->channels:-1;

	if(port)
		inchannelindex[port->channels-1]=port->portindex;
}

void AudioIOFX::SetOutput(AudioPort *port)
{
	out_vchannel=port;
	// Out
	outvchannel_index=out_vchannel?out_vchannel->portindex:0;
	outvchannel_channels=out_vchannel?out_vchannel->channels:-1;

	if(port)
		outchannelindex[port->channels-1]=port->portindex;
}

void AudioIOFX::SetChannelType(int type)
{
	channel_type=type;

	// Refresh Ports
	if(mainaudio->GetActiveDevice())
	{
		in_vchannel=&mainaudio->GetActiveDevice()->inputaudioports[channel_type][inchannelindex[GetChannels()-1]];
		out_vchannel=&mainaudio->GetActiveDevice()->outputaudioports[channel_type][outchannelindex[GetChannels()-1]];
	}

	audioeffects.RefreshBuffer(mainaudio->GetActiveDevice());
	audioinputeffects.RefreshBuffer(mainaudio->GetActiveDevice());

	//audioeffects.AO_InitIOChannels(GetChannels());
	//audioinputeffects.AO_InitIOChannels(GetChannels());
}

bool AudioIOFX::AddSend(AudioChannel *bus)
{
	if(bus)
	{
		if(AudioSend *newsend=new AudioSend(bus))
		{
			sends.AddEndO(newsend);
			sends.Close();

			return true;
		}
	}

	return false;
}

void AudioIOFX::AddSend(AudioSend *send,int index)
{
	sends.AddOToIndex(send,index);
	sends.Close();
}

void AudioIOFX::DeleteSend(AudioSend *s,bool deleteall)
{
	if(deleteall==true)
		sends.RemoveO(s);
	else
		sends.CutObject(s);

	sends.Close();
}

