#include "defines.h"
#include <stdio.h>
#include <string.h>

#ifdef WIN32

#include "asio/asiosys.h"
#include "asio/asio.h" 
#endif
#include "audiohardware.h"
#include "audiodevice.h"
#include "audiochannel.h"
#include "audiobus.h"
#include "audiopeakfile.h"
#include "audiorealtime.h"
#include "audiosystem.h"

#include "object_song.h"
#include "semapores.h"
#include "audiohardware.h"
#include "audiosend.h"
#include "songmain.h"
#include "audioports.h"
#include "audioproc.h"
#include "colours.h"
#include "gui.h"
#include "editortypes.h"
#include "edit_audiointern.h"
#include "vstguiwindow.h"

extern char *channelchannelsinfo[],*channelchannelsinfo_short[];
extern int channelschannelsnumber[];

/*
Ein Regler des es in sich hat. Mit ihm kann jetzt jedes Effektgerät sinnvoll angeseuert werden.
Mit PRE wird vor dem Fader das Signal abgegriffen. Nur so kann ein reines Effektsignal dem Master hinzugemischt werden.
Im POST Modus wird nach dem Fader das Signal zum Effekt Gerät geführt. So kann ein Effekt zum Original hinzugemischt werden.
*/

int AudioChannel::GetBgColour()
{
	switch(audiochannelsystemtype)
	{
	case CHANNELTYPE_MASTER:
		return COLOUR_MASTERCHANNEL;

	case CHANNELTYPE_BUSCHANNEL:
		return IsSelected()==true?COLOUR_BUSCHANNELSELECTED:COLOUR_BUSCHANNEL;

	case CHANNELTYPE_DEVICEOUT:
		return COLOUR_DEVICEOUT;
		break;

	case CHANNELTYPE_DEVICEIN:
		return COLOUR_DEVICEIN;
		break;
	} 

	return COLOUR_BACKGROUND;
}

void AudioChannel::ShowChannelName(guiWindow *nostringrefresh)
{
	guiWindow *win=maingui->FirstWindow();

	while(win)
	{	
		if(win!=nostringrefresh)
			switch(win->GetEditorID())
		{
			case EDITORTYPE_PLUGIN_INTERN:
				{
					Edit_Plugin_Intern *edintern=(Edit_Plugin_Intern *)win;

					if(edintern->insertaudioeffect && edintern->insertaudioeffect->effectlist->channel==this)
						edintern->SetName(true);
				}
				break;

			case EDITORTYPE_PLUGIN_VSTEDITOR:
				{
					Edit_Plugin_VST *edvst=(Edit_Plugin_VST *)win;

					if(edvst->insertaudioeffect && edvst->insertaudioeffect->effectlist->channel==this)
					{
						edvst->SetName(true);
					}
				}
				break;
		}

		win=win->NextWindow();
	}
}

char *AudioChannel::CreateFullName()
{
	if(fullname)
	{
		delete fullname;
		fullname=0;
	}

	char h2[NUMBERSTRINGLEN];
	char *h=0;
	char typeinfo[8];
	
	typeinfo[0]=0;

	
	int index=1;

	switch(audiochannelsystemtype)
	{
	case CHANNELTYPE_BUSCHANNEL:
		strcpy(typeinfo,"B");
		index=GetIndex();
		break;

	case CHANNELTYPE_DEVICEIN:
		strcpy(typeinfo,"MI");
		index=-1;
		break;

	case CHANNELTYPE_DEVICEOUT:
		strcpy(typeinfo,"MO");
		index=-1;
		break;
	}

	// AI 1 ST:Willi
	if(index>=0)
	{
		index++;

		if(char *indexstr=mainvar->ConvertIntToChar(index,h2))
			h=mainvar->GenerateString(typeinfo,indexstr);
	}
	else
		h=mainvar->GenerateString(typeinfo);

	if(h)
	{
		fullname=mainvar->GenerateString(h," ",channelchannelsinfo_short[io.channel_type],":",name);
		delete h;
	}

	return fullname;
}

void AudioChannel::SetSolo(bool solostatus,bool lock)
{	
	if(lock==true)
		mainthreadcontrol->Lock(CS_audioplayback);

	audiosystem->channel_solomode=solostatus;

	for(int i=0;i<LASTSYNTHCHANNEL;i++)
	{
		// Set All Channels,bus and Instruments
		AudioChannel *ac=audiosystem->FirstChannelType(i);
		while(ac){
			ac->io.audioeffects.SetSolo(false);
			ac=ac->NextChannel();
		}
	}

	io.audioeffects.SetSolo(solostatus); // Set Solo

	if(lock==true)
		mainthreadcontrol->Unlock(CS_audioplayback);
}

void AudioChannel::SetMIDIVelocity(int mv,OSTART automationtime)
{
	MIDIfx.SetVelocity(mv,automationtime);
}

void AudioChannel::ResetAudioChannelBuffers()
{
	mix.channelsused=0;
}

void AudioChannel::ToggleMute()
{
//			OSTART automationtime=GetAutomationTime();
	
	io.audioeffects.Mute(io.audioeffects.GetMute()==true?false:true,audiosystem->song==mainvar->GetActiveSong()?true:false);
}

void AudioChannel::DeleteChannelBuffersMix()
{
	mix.DeleteARESOut();
}

void AudioChannel::CreateChannelBuffers(AudioDevice *device)
{
	DeleteChannelBuffersMix();

	if(device && device->GetSetSize())
	{
		mix.InitARESOut(device->GetSetSize(),MAXCHANNELSPERCHANNEL);

		//mix.channelsinbuffer=MAXCHANNELSPERCHANNEL;
		mix.channelsused=0;

		mix.SetBuffer(MAXCHANNELSPERCHANNEL,device->GetSetSize());

		//mix.samplesinbuffer=device->GetSetSize();
		//mix.samplesinbuffer_size=device->GetSetSize()*sizeof(ARES);

		mix.bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),device->GetSetSize());
		if(mix.bufferms<=0)
			mix.bufferms=0.1; // mini !

		mix.delayvalue=(int)floor((1000/mix.bufferms)+0.5);
	}
}

void AudioChannel::SetName(char *nn)
{
	if(nn)
	{
		size_t sl=strlen(nn);

		if(sl>SMALLSTRINGLEN-1)
		{
			sl=SMALLSTRINGLEN-1;
			strncpy(name,nn,sl);
			name[SMALLSTRINGLEN-1]=0;
		}
		else
			strcpy(name,nn);
	}
	else
		strcpy(name,".");

	CreateFullName();
}
