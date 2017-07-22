#include "audiochannel.h"
#include "guimenu.h"
#include "audiosystem.h"
#include "audiodevice.h"
#include "audiohardware.h"
#include "semapores.h"
#include "gui.h"
#include "audiomixeditor.h"
#include "object_song.h"
#include "songmain.h"
#include "languagefiles.h"
#include "audioports.h"
#include "audiohardwarechannel.h"

// see enum ChannelTypes !!

char *channelchannelsinfo_short[MAXCHANNELSPERCHANNEL]=
{
	"MO",
	"ST",
	"2.1",
	"QD",
	"PC",
	"5.1",
	"6.1",
	"7.1"
};

char *channelchannelsinfo[MAXCHANNELSPERCHANNEL]=
{
	"Mono",
	"Stereo",
	"Surround 2+1",
	"Quadro (4)",
	"Prologic (5)",
	"Prologic II (5+1)",
	"Digital EX (6+1)",
	"Digital True HD (7+1)"
};

int channelschannelsnumber[MAXCHANNELSPERCHANNEL]=
{
	1,2,3,4,5,6,7,8
};

bool AudioChannel::SetVType(AudioDevice *device,int type,bool guirefresh,bool allselected)
{
	bool changed=false;
	if(type>1)return false;

	mainaudio->defaultchannel_type=type;

	Seq_Song *song=audiosystem->song;

	if(song==mainvar->GetActiveSong())
		mainthreadcontrol->LockActiveSong();

	if(type!=io.channel_type)
	{
		changed=true;

		mix.peak.Reset();
		io.SetChannelType(type);

		/*
		if(device)
		{
			// Input
			io.in_vchannel=&device->inputaudioports[type][io.inputportindexs[type]];


			// Output
			io.out_vchannel=&device->outputaudioports[type][io.outputportindexs[type]];
		}
		*/
	}

	if(song==mainvar->GetActiveSong())
		mainthreadcontrol->UnlockActiveSong();

	if(changed==true)
	{
		CreateFullName();

		if(guirefresh==true)
		{
			maingui->RefreshAllEditors(song,EDITORTYPE_AUDIOMIXER,Edit_AudioMix::REFRESH_OUTCHANNELS);
			maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
			maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGELIST,0);
		}
	}

	return true;
}

bool AudioChannel::ConnectTOVChannel(AudioPort *vchl,bool out)
{
	if(vchl->channels>2)
		return false;

	if(out==true)
	{
		mainthreadcontrol->LockActiveSong();

		io.SetPlaybackChannel(vchl,true,false);
		mix.peak.InitPeak(io.out_vchannel?io.out_vchannel->channels:0);

		mainthreadcontrol->UnlockActiveSong();
	}
	else
	{
		mainthreadcontrol->LockActiveSong();
		io.SetInput(vchl);
		mainthreadcontrol->UnlockActiveSong();
	}

	maingui->RefreshAllEditors(audiosystem->song,EDITORTYPE_AUDIOMIXER,Edit_AudioMix::REFRESH_OUTCHANNELS);

	return true;
}

guiMenu *AudioChannel::CreateChannelOutputMenu(guiMenu *to)
{
	class menu_con:public guiMenu
	{
	public:
		menu_con(AudioChannel *c,AudioPort *avh)
		{
			channel=c;
			avchannel=avh;
		}

		void MenuFunction()
		{
			channel->ConnectTOVChannel(avchannel,true);
		}

		AudioChannel *channel;
		AudioPort *avchannel;
	};

	if(to && mainaudio->GetActiveDevice())
	{
		Seq_Song *song=audiosystem->song;

		for(int chls=0;chls<MAXCHANNELSPERCHANNEL /*song->audiosystem.masterchannel.io.channel_type*/;chls++)
		{
			char *h=0;

			if(io.out_vchannel /*&& GetAudioOut() && GetAudioOut()->GetCountGroups()==0*/)
			{
				for(int i=0;i<CHANNELSPERPORT;i++)
				{
					AudioPort *avh=&mainaudio->GetActiveDevice()->outputaudioports[chls][i];

					if(avh==io.out_vchannel)
					{
						h=mainvar->GenerateString(channelchannelsinfo[chls]," = ",avh->name);
						break;
					}
				}
			}

			if(!h)
				h=mainvar->GenerateString(channelchannelsinfo[chls]);

			if(h)
			{
				if(guiMenu *sub=to->AddMenu(h,0))
					for(int i=0;i<CHANNELSPERPORT;i++)
					{
						AudioPort *avh=&mainaudio->GetActiveDevice()->outputaudioports[chls][i];
						
						bool sel=false;

						if(io.out_vchannel==avh)
							sel=true;

						if(avh->visible==true)
							sub->AddFMenu(avh->name,new menu_con(this,avh),sel);
					}

					delete h;
			}
		}
	}

	return 0;
}

guiMenu *Seq_Track::CreateTrackOutputMenu(guiMenu *to,Seq_Song *song)
{
	if(!song)
		return 0;

	class menu_con:public guiMenu
	{
	public:
		menu_con(Seq_Track *t,AudioPort *avh)
		{
			track=t;
			avchannel=avh;
		}

		void MenuFunction()
		{
			track->ConnectTrackOutputToPort(avchannel,true);
		}

		Seq_Track *track;
		AudioPort *avchannel;
	};

	if(to && mainaudio->GetActiveDevice())
	{
		for(int chls=0;chls<MAXCHANNELSPERCHANNEL /*song->audiosystem.masterchannel.io.channel_type*/;chls++)
		{
			char *h=0;

			if(io.out_vchannel /*&& GetAudioOut() && GetAudioOut()->GetCountGroups()==0*/)
			{
				for(int i=0;i<CHANNELSPERPORT;i++)
				{
					AudioPort *avh=&mainaudio->GetActiveDevice()->outputaudioports[chls][i];

					if(avh==io.out_vchannel && GetAudioOut()->FirstChannel()==0)
					{
						h=mainvar->GenerateString(channelchannelsinfo[chls]," = ",io.out_vchannel->name);
						break;
					}
				}
			}

			if(!h)
				h=mainvar->GenerateString(channelchannelsinfo[chls]);

			if(h)
			{
				if(guiMenu *sub=to->AddMenu(h,0))
					for(int i=0;i<CHANNELSPERPORT;i++)
					{
						AudioPort *avh=&mainaudio->GetActiveDevice()->outputaudioports[chls][i];

						bool sel=false;

						if(GetAudioOut()->FirstChannel()==0)
						{
						if(io.out_vchannel==avh)
							sel=true;

						if(io.in_vchannel==avh)
							sel=true;
						}

						if(avh->visible==true)
							sub->AddFMenu(avh->name,new menu_con(this,avh),sel);
					}

					delete h;
			}

		}
	}

	return 0;
}

void Seq_Track::ShowAudioOutput(TAudioOut *tao)
{
	tao->audiochannelouts->Delete();

	GetAudioOut()->CloneToGroup(tao->audiochannelouts);

	tao->usedirecttodevice=usedirecttodevice;

	if(GetAudioOut()->FirstChannel() && usedirecttodevice==false)
		tao->status_audiochannel=GetAudioOut()->FirstChannel()->channel;
	else
		tao->status_audiochannel=0;

	char *help=0,*c;

	if(io.out_vchannel && usedirecttodevice==true)
	{
		tao->returnstring=mainvar->GenerateString(io.out_vchannel->name);
		return;
	}

	if((!io.out_vchannel) && usedirecttodevice==true)
	{
		tao->returnstring=mainvar->GenerateString("Error:Device");
		return;
	}

	if((!tao->status_audiochannel) && usedirecttodevice==false)
	{
		tao->returnstring=mainvar->GenerateString("Error:Channel");
		return;
	}

	if(tao->status_audiochannel && usedirecttodevice==false)
	{
		c=tao->status_audiochannel->GetFullName(); // 1 Audio Track

		if(GetAudioOut()->FirstChannel()!=GetAudioOut()->LastChannel())
		{
			size_t len=0;

			if(c)len+=strlen(c);

			if(help=new char[len+32])
			{
				if(c)
					strcpy(help,c);
				else
					help[0]=0;

				mainvar->AddString(help,"+[");

				char h2[NUMBERSTRINGLEN];

				mainvar->AddString(help,mainvar->ConvertIntToChar(GetAudioOut()->GetCountGroups()-1,h2));
				mainvar->AddString(help,"]");

				c=help;
			}
		}

		//Bypass ?
		Seq_AudioIOPointer *p=GetAudioOut()->FirstChannel();
		while(p)
		{
			if(p->bypass==true)
			{
				char *h=mainvar->GenerateString("[By]",c?c:".");
				if(help)
					delete help;
				c=h;
				help=h;
				break;
			}

			p=p->NextChannel();
		}
	}
	else
		c=Cxs[CXS_NOAUDIO];

	tao->returnstring=tao->simple==false?mainvar->GenerateString("AOut:",c):mainvar->GenerateString(c);

	if(help)delete help;
}

int AudioChannel::GetSoloStatus()
{
	if(io.audioeffects.GetSolo()==false)
	{
		return audiosystem->channel_solomode==true?SOLO_OTHER:SOLO_OFF;
	}

	return SOLO_THIS;
}

void AudioChannel::SetAudioInBypass(bool by)
{
	mainthreadcontrol->LockActiveSong();
	io.bypass_input=by;
	mainthreadcontrol->UnlockActiveSong();
}

void AudioChannel::SetRecordChannel(AudioPort *vh,bool lock)
{
	if(lock==true)mainthreadcontrol->LockActiveSong();

	if(vh)
		io.inputpeaks.InitPeak(vh->channels);
	else
		io.inputpeaks.InitPeak(0);

	io.SetInput(vh);

	io.bypass_input=false;

	if(lock==true)mainthreadcontrol->UnlockActiveSong();
}

void AudioIOFX::SetFXBypass(bool onoff)
{
	bypassallfx=onoff;
}

void AudioIOFX::SetInputFXBypass(bool onoff)
{
	bypassallinputfx=onoff;
}

void AudioIOFX::ActivateInputs()
{
	if(in_vchannel)
	{
		for(int i=0;i<in_vchannel->channels;i++)
		{
			if(in_vchannel->hwchannel[i])
			{
				in_vchannel->hwchannel[i]->canbedisabled=false;
				in_vchannel->hwchannel[i]->inputused=true;
			}
		}
	}
}

void AudioIOFX::SetPlaybackChannel(AudioPort *vh,bool dontchangechannelchannels,bool lock)
{
	if(lock==true)
		mainthreadcontrol->LockActiveSong();

	SetOutput(vh);
	
	/*
	if(dontchangechannelchannels==false)
	{
		if(vh && vh->channels>0)
			channel_type=vh->channels-1;
		else
			channel_type=0;
	}
*/

	if(lock==true)
		mainthreadcontrol->UnlockActiveSong();
}

void AudioIOFX::Clone(AudioIOFX *to)
{
	to->channel_type=channel_type;

	to->in_vchannel=in_vchannel;
	to->audioinputenable=audioinputenable;

	//Audio Out
	to->out_vchannel=out_vchannel;

	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
	{
		to->inchannelindex[i]=inchannelindex[i];
		to->outchannelindex[i]=outchannelindex[i];
	}

}
