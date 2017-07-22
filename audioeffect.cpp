#include "defines.h"
#include "audioobjects.h"
#include "audioobject_volume.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "audioeffects.h"
#include "object_song.h"

void AudioObject::AO_InitSampleRateAndSize(int rate,int buffersize)
{
	setSize=buffersize;
	setSampleRate=rate;

	InitSampleRateAndSize(rate,buffersize);
}

void AudioObject::AO_InitIOChannels(int channels)
{
	iosetcorrect=InitIOChannels(channels);

	if(iosetcorrect==true)
	{	
		iochannels=channels;

		if(ins) // Use Stream Input (FX)
		{
			if(ins==outs)
				setins=setouts=channels;
			else
			{
				// 1/2
				if(ins>=channels)
					setins=channels;
				else
					setins=ins;

				if(outs<=channels)
					setouts=channels;
				else
					setouts=outs;
			}
		}
		else
		{
			// Just Create Output (Instrument)
			setins=0;
			setouts=outs;
		}
	}
	else
	{
		setins=ins;
		setouts=outs;
	}

}

void AudioObject::TogglePlugInOnOff()
{
	OnOff(plugin_on==true?false:true);
}

void AudioObject::User_TogglePluginBypass()
{
	if(song)
		song->Automate(this,song->GetSongPosition(),ID_AUDIOPLUGINBYPASS,plugin_bypass==true?0:1,AEF_PLUGINCONTROL|AEF_USEREDIT);

	plugin_bypass=plugin_bypass==true?false:true;
}

void AudioObject::CreateAudioObjectBuffer()
{
	Reset();
	DeleteBuffer();

	if(setSize==0)return;

	int outchls=GetOutputPins()<MAXCHANNELSPERCHANNEL?MAXCHANNELSPERCHANNEL:GetOutputPins(); // Minimum MAXCHANNELSPERCHANNEL !

	if(aobuffer=new AudioHardwareBuffer)
	{
		aobuffer->InitARESOut(setSize,outchls);

		if(aobuffer->outputbufferARES)
		{
			aobuffer->SetBuffer(outchls,setSize);

			aobuffer->bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),setSize);
			if(aobuffer->bufferms<=0)aobuffer->bufferms=0.1; // mini !
			aobuffer->delayvalue=(int)floor((1000/aobuffer->bufferms)+0.5);
		}
		else
		{
			delete aobuffer;
			aobuffer=0;
		}
	}
}

void AudioObject::DeleteBuffer()
{
	if(aobuffer)
	{
		aobuffer->Delete32BitBuffer();
		delete aobuffer;
		aobuffer=0;
	}
}

void AudioObject::FreeChunks()
{
	if(chunkdata_buffer)
		delete chunkdata_buffer;

	chunkdata_buffer=0;
}

void AudioObject::SetInVolume(ARES v)
{
	involume=v;
}

void AudioObject::SetOutVolume(ARES v)
{
	outvolume=v;
}

bool AudioObject::IsActive()
{
	return plugin_active;
}



