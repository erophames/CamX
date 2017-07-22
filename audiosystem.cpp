#include "defines.h"
#include <stdio.h>
#include <string.h>

#ifdef WIN32

#include "asio/asiosys.h"
#include "asio/asio.h" 
#endif

#include "songmain.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "audiochannel.h"
#include "audiobus.h"
#include "audiofile.h"
#include "audiopeakfile.h"
#include "audiorealtime.h"
#include "audiomaster.h"
#include "audiosystem.h"

#include "object_song.h"
#include "semapores.h"
#include "audiohardware.h"
#include "audiosend.h"

#include "audiomixeditor.h"
#include "gui.h"

#include "audioproc.h" // Cores
#include "audiohdfile.h"

#include "peakbuffer.h"
#include "audiothread.h"

#include <math.h>
#include "languagefiles.h"
#include "object_project.h"
#include "chunks.h"

#include "mastering.h"
#include "MIDIinproc.h"
#include "inputdata.h"
#include "audiorecord.h"
#include "audiohardwarechannel.h"

void AudioSystem::AddBus(AudioChannel *prev,AudioChannel *bus)
{
	audiochannels[CHANNELTYPE_BUSCHANNEL].AddPrevO(bus,prev);
	audiochannels[CHANNELTYPE_BUSCHANNEL].Close(); // Index

	CreateChannelsFullName();
}

void AudioSystem::AddBus(AudioChannel *bus,int index)
{
#ifdef DEBUG
	if(index<0 || index>50)
		maingui->MessageBoxOk(0,"Add Bus Index Error");
#endif

	audiochannels[CHANNELTYPE_BUSCHANNEL].AddOToIndex(bus,index);
	audiochannels[CHANNELTYPE_BUSCHANNEL].Close(); // Index

	CreateChannelsFullName();
}

AudioChannel *AudioSystem::CreateBus(AudioChannel *prev,AudioPort *out/*AudioPort *record*/)
{
	if((!prev) || prev->audiochannelsystemtype!=CHANNELTYPE_BUSCHANNEL)
	{
		prev=GetLastAudioBusChannel();
	}

	if(AudioChannel *bus=new AudioChannel(this,CHANNELTYPE_BUSCHANNEL))
	{
		//bus->io.inputpeaks.InitPeak(MAXCHANNELSPERCHANNEL);
		//bus->mix.peak.InitPeak(MAXCHANNELSPERCHANNEL);

		// Channel Number
		//	char nrs[NUMBERSTRINGLEN];
		//	int i=GetCountOfBusChannels();

		//	i++;

		bus->InitDefaultAutomationTracks(0,bus);

		strcpy(bus->name,"Bus");
		//	strcpy(&bus->name[3],mainvar->ConvertIntToChar(i,nrs));

		bus->io.Repair();
		bus->CreateChannelBuffers(device);
		bus->io.channel_type=out?out->channels-1:mainaudio->defaultchannel_type;

		if(device)
		{
			if(!out)
				out=device->FindVOutChannel(bus->io.GetChannels(),mainaudio->defaultchannelindex_out);

			//if(!record)
			//	record=device->FindVInChannel(bus->io.GetChannels(),mainaudio->defaultchannelindex_in);
		}

		bus->io.SetPlaybackChannel(out,true,false);
		//bus->SetRecordChannel(recchannel);

		if(song==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		if(audiochannels[CHANNELTYPE_BUSCHANNEL].activeobject==0)
			audiochannels[CHANNELTYPE_BUSCHANNEL].activeobject=bus;

		audiochannels[CHANNELTYPE_BUSCHANNEL].AddPrevO(bus,prev);

		audiochannels[CHANNELTYPE_BUSCHANNEL].Close(); // Index

		if(song==mainvar->GetActiveSong())
			mainthreadcontrol->UnlockActiveSong();

		//	if(!activechannel)activechannel=bus;

		CreateChannelsFullName();

		return bus;
	}

	return 0;
}

// AudioChannels -----------------------------
void AudioSystem::StopSystem()
{
	ResetChannels();
}

void AudioSystem::ResetChannels()
{
	// Clear Channels
	for(int i=0;i<LASTSYNTHCHANNEL;i++)
	{
		AudioChannel *chl=FirstChannelType(i);

		while(chl){
			//chl->ClearPeak();
			//	chl->ClearMaxPeak();
			chl->ResetAudioChannelBuffers();

			chl=chl->NextChannel();
		}
	}

	//masterchannel.ClearPeak();
}

AudioChannel *AudioSystem::FindPlaybackBusForAudioHDFile(AudioHDFile *playfile)
{
	//if(activechannel)
	//	return activechannel;

	if(playfile)
	{
		AudioChannel *b=FirstBusChannel();

		while(b)
		{
			if(channelschannelsnumber[b->io.channel_type]==playfile->channels)
				return b;

			b=b->NextChannel();
		}
	}


	return GetLastAudioBusChannel();
}

AudioChannel *AudioSystem::DeleteAudioChannel(AudioChannel *chl,bool deleteall,bool locksystem)
{
	if(locksystem==true)
		mainthreadcontrol->LockActiveSong();

	if(chl->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL)
	{
		if(GetFocusBus()==chl)
			SetFocusBus(chl->NextChannel(),false);

		// Remove Bus from Tracks/Sends

		Seq_Track *t=song->FirstTrack();

		while(t)
		{
			// Remove Sends
			AudioSend *s=t->io.FirstSend();

			while(s){

				if(s->sendchannel==chl){
					AudioSend *ns=s->NextSend();
					t->io.DeleteSend(s,deleteall);
					s=ns;
				}
				else
					s=s->NextSend();
			}

			// Remove Track Connection Bus
			t->GetAudioOut()->RemoveBusFromGroup(chl);

			if(t->usedirecttodevice==false && t->GetAudioOut()->FirstChannel()==0)
				t->usedirecttodevice=true;

			t=t->NextTrack();
		}
	}

	if(chl->io.audioeffects.GetSolo()==true)
		chl->SetSolo(false,false);

	AudioChannel *n=(AudioChannel *)audiochannels[chl->audiochannelsystemtype].CutObject(chl);
	audiochannels[chl->audiochannelsystemtype].Close();

	if(locksystem==true)
		mainthreadcontrol->UnlockActiveSong();

	if(deleteall==true)
	{
		chl->FreeAudioChannelMemory();
		delete chl;
	}

	CreateChannelsFullName();

	return n;
}

void AudioSystem::DeleteAllChannels()
{
	for(int i=0;i<LASTSYNTHCHANNEL;i++)
	{
		AudioChannel *chl=FirstChannelType(i);

		while(chl)
			chl=DeleteAudioChannel(chl,true);
	}

	masterchannel.FreeAudioChannelMemory();
}

void mainAudio::SetAllDevicesStartPosition(Seq_Song *song,OSTART pos,int flag)
{
	if(selectedaudiohardware)
	{
		AudioDevice *dev=selectedaudiohardware->FirstDevice();

		while(dev)
		{
			dev->SetStart(song,pos,flag);
			dev=dev->NextDevice();
		}
	}

	/*
	if(mainvar->GetActiveSong())
	mainvar->GetActiveSong()->audiosystem.InitStart();
	*/
}

AudioDevice *AudioHardware::RemoveAudioDevice(AudioDevice *dev)
{
	//if(!dev->NextDevice())
	//	dev->audiodriver.RemoveDriver(); // Last Device clears drivers

	dev->CloseAudioDevice(true);

	if(dev->initname)delete dev->initname;
	if(dev->devname)delete dev->devname;

	dev->DeInitDefaultPorts();

	return (AudioDevice *)devices.RemoveO(dev);
}

AudioHardware::AudioHardware(char *id_name,int id)
{
	strcpy(name,id_name);
	systemtype=id;
	activedevice=0;
}

void AudioHardware::AddAudioDevice(AudioDevice *dev,char *dname,char *dtypename)
{
	if(dev && dname)
	{
		// Load AudioDevice Settings
		//if(dev->FirstVOutChannel()==0 && dev->FirstVInputChannel()==0)
		//	dev->CreateVHardwareChannels();

		dev->devicetypname=mainvar->GenerateString(dtypename);
		dev->initname=mainvar->GenerateString(dname);

		if(!dev->initname)
			return;

		// Find Device with same Name
		int idck=0;
		AudioDevice *ckdev=FirstDevice();

		while(ckdev)
		{
			if(ckdev->initname && strcmp(ckdev->initname,dname)==0)
				idck++;

			ckdev=ckdev->NextDevice();
		}

		if(idck>0)
		{
			char h2[NUMBERSTRINGLEN];

			dev->devname=mainvar->GenerateString(dev->initname,"_",mainvar->ConvertIntToChar(idck,h2));
		}
		else
			dev->devname=mainvar->GenerateString(dev->initname);

		devices.AddEndO(dev);
	}
}

/*
typedef struct ASIOCallbacks
{
void (*bufferSwitch) (long doubleBufferIndex, ASIOBool directProcess);
// bufferSwitch indicates that both input and output are to be processed.
// the current buffer half index (0 for A, 1 for B) determines
// - the output buffer that the host should start to fill. the other buffer
//   will be passed to output hardware regardless of whether it got filled
//   in time or not.
// - the input buffer that is now filled with incoming data. Note that
//   because of the synchronicity of i/o, the input always has at
//   least one buffer latency in relation to the output.
// directProcess suggests to the host whether it should immedeately
// start processing (directProcess == ASIOtrue), or whether its process
// should be deferred because the call comes from a very low level
// (for instance, a high level priority interrupt), and direct processing
// would cause timing instabilities for the rest of the system. If in doubt,
// directProcess should be set to ASIOfalse.
// Note: bufferSwitch may be called at interrupt time for highest efficiency.

void (*sampleRateDidChange) (ASIOSampleRate sRate);
// gets called when the AudioStreamIO detects a sample rate change
// If sample rate is unknown, 0 is passed (for instance, clock loss
// when externally synchronized).

long (*asioMessage) (long selector, long value, void* message, double* opt);
// generic callback for various purposes, see selectors below.
// note this is only present if the asio version is 2 or higher

ASIOTime* (*bufferSwitchTimeInfo) (ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
// new callback with time info. makes ASIOGetSamplePosition() and various
// calls to ASIOGetSampleRate obsolete,
// and allows for timecode sync etc. to be preferred; will be used if
// the driver calls asioMessage with selector kAsioSupportsTimeInfo.
} ASIOCallbacks;

*/

void Seq_Song::ResetSongTracksAudioBuffer()
{
	// Reset Playback Buffer
	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(/*(!t->parent) && */t->t_audiofx.stream)
		{
			for(int i=0;i<t->t_audiofx.nrstream;i++)
				t->t_audiofx.stream[i].channelsused=0;
		}
		t=t->NextTrack();
	}
}

void mainAudio::SetAudioSystem(char *defaultaudio)
{
	if(defaultaudio)
	{
		int c=0;

		AudioHardware *ah=FirstAudioHardware();

		while(ah)
		{
			if(strcmp(defaultaudio,ah->name)==0)
			{
				SetAudioSystem(c);
				break;
			}

			c++;
			ah=ah->NextHardware();
		}
	}
}

void mainAudio::SetSoundCard(char *defaultsoundcard)
{
	if(defaultsoundcard && selectedaudiohardware)
	{
		AudioDevice *dev=selectedaudiohardware->FirstDevice();

		while(dev)
		{
			if(dev->devname && strcmp(defaultsoundcard,dev->devname)==0)
			{
				selectedaudiohardware->SetActiveDevice(dev);
				break;
			}

			dev=dev->NextDevice();
		}
	}
}

void mainAudio::ResetAudioDevices()
{
	if(GetActiveDevice())
	{
		GetActiveDevice()->Reset(0);
		GetActiveDevice()->InitAudioDevice();
	}
}

bool mainAudio::StartDevices()
{
	TRACE ("Start Devices \n");

	if(GetActiveDevice())
	{
		TRACE ("Start Device %s \n",mainaudio->GetActiveDevice()->devname);

		if(GetActiveDevice()->InitAudioDeviceChannels()==false)
			return false;

		GetActiveDevice()->devicepRepared=true;
		return GetActiveDevice()->StartDevice();
	}

	TRACE ("Start Devices FALSE \n");
	return false;
}

bool mainAudio::StopDevices()
{
	if(mainaudio->selectedaudiohardware)
	{
		bool stopped=false;

		//if(!(flag&(NO_SYSTEMLOCK|STOPSELECT_SYNC)))
		{
			mainthreadcontrol->Lock(CS_audioplayback);
			//audiorecordbufferthread->Lock();

			AudioDevice *dev=mainaudio->selectedaudiohardware->FirstDevice();

			while(dev){
				dev->stopdevice=true;
				dev=dev->NextDevice();
			}

			//audiorecordbufferthread->Unlock();
			mainthreadcontrol->Unlock(CS_audioplayback);
		}

		AudioDevice *dev=mainaudio->selectedaudiohardware->FirstDevice();

		while(dev){
			if(dev->devicestarted==true){
				dev->StopDevice();

				stopped=true;
			}

			dev->stopdevice=false;
			dev=dev->NextDevice();
		}

		return stopped;
	}

	return false;
}

void mainAudio::ConvertSampleToPeak(ARES peak,char *to,int length)
{
	char *p,hs[16];

	if(length>=16)
	{
		ARES h;

		if(peak==0)
		{
			strcpy(to,"0.00");
		}
		else
			if(peak>1)
			{
				strcpy(to,"+1.");
				to+=3;

				peak-=1;
				h=peak*10;
				p=mainvar->ConvertIntToChar((int)h,hs);
				strcpy(to,p);
			}
			else
			{
				strcpy(to,"0.");
				to+=2;
				h=peak*10;
				p=mainvar->ConvertIntToChar((int)h,hs);
				strcpy(to,p);
			}
	}
}

double mainAudio::ConvertSamplesToMs(LONGLONG samples)
{
	LONGLONG sec=samples/GetGlobalSampleRate(); // seconds

	if(LONGLONG rest=samples-sec*GetGlobalSampleRate())
	{
		double hms=(double)rest;
		hms/=samplesperms;
		return (double)sec*1000+hms;
	}

	return (double)sec*1000;
}

double mainAudio::ConvertInternToExternSampleRate(double v)
{
	return v/internexternfactor;
}

double mainAudio::ConvertExternToInternSampleRate(double v)
{
	return v*internexternfactor;
}

void mainAudio::ConvertSamplesToTime(LONGLONG samples,Seq_Pos *pos)
{
	LONGLONG h=samples/GetGlobalSampleRate(); // seconds

	//	pos->samples=samples;

	// Hour
	pos->pos[0]=(int)h/3600;
	h-=pos->pos[0]*3600;

	// Min
	pos->pos[1]=(int)h/60;
	h-=pos->pos[1]*60;

	// Sec
	pos->pos[2]=(int)h;

	h=GetGlobalSampleRate()*(pos->pos[2]+60*pos->pos[1]+3600*pos->pos[0]);
	samples-=h;

	if(samples>0) // Rest Samples 0-999.999 ms
	{
		double dh=GetGlobalSampleRate(),ms=(double)samples;

		dh/=1000;
		ms/=dh;

		// Ms
		double framefactor;

		if(pos->IsSmpte()==true)
		{
			framefactor=1000;
			framefactor/=SMPTE_FPS[pos->mode];
		}
		else
			framefactor=0;

		if(framefactor)
		{
			dh=ms;
			dh/=framefactor;

			// frames
			pos->pos[3]=(int)dh;
			ms-=pos->pos[3]*framefactor;

			if(ms>0)
			{
				// sub frames
				framefactor/=4;
				ms/=(int)framefactor; // round down

				pos->pos[4]=(int)ms;
				pos->pos[4]++;
			}
			else
				pos->pos[4]=0;
		}
		else
		{
			pos->pos[3]=0;
			pos->pos[4]=0;
		}

	}
	else
	{
		pos->pos[3]=0;
		pos->pos[4]=0;
	}
}

void AudioSystem::MixHWChannelsToMasterMix()
{
	int channels=channelschannelsnumber[song->default_masterchannels];

	// 1. Clear
	memset(device->mastermix.outputbufferARES,0,device->mastermix.samplesinbuffer_size*channels);
	device->mastermix.channelsused=channels;


	// Channels -> Stereo etc. ABABAB

	int index=0;

	AudioHardwareChannel *ahw=device->FirstOutputChannel();
	while(ahw)
	{
		if(ahw->hardwarechannelused==true)
		{
			int i=device->mastermix.samplesinbuffer;

			ARES *from=device->devmix.outputbufferARES;
			from+=i*ahw->channelindex;

			ARES *to=device->mastermix.outputbufferARES;
			to+=i*index;	

			if(int loop=i/8)
			{
				i-=loop*8;

				do
				{
					*to++ +=*from++;
					*to++ +=*from++;
					*to++ +=*from++;
					*to++ +=*from++;

					*to++ +=*from++;
					*to++ +=*from++;
					*to++ +=*from++;
					*to++ +=*from++;

				}while(--loop);
			}

			while(i--)
				*to++ +=*from++;

		}

		index++;
		if(index==channels)
			index=0;

		ahw=ahw->NextChannel();
	}
}

void AudioSystem::AddMasterEffects() 
{
	// Clear not used Channels
	AudioHardwareChannel *ahw=device->FirstOutputChannel();
	while(ahw)
	{
		if(ahw->hardwarechannelused==true)break;
		ahw=ahw->NextChannel();
	}

	ARES peakmax=0;

	if(!ahw)
	{
		// 0 Hardware Channels used
		if(device->outbuffercleared==false)
		{
			device->outbuffercleared=true;
			memset(device->devmix.outputbufferARES,0,device->devmix.samplesinbuffer_size*device->GetCountOfOutputChannels());

			AudioHardwareChannel *ah=device->FirstOutputChannel();

			while(ah)
			{
				ah->zerocleared=true;
				ah=ah->NextChannel();
			}
		}
	}
	else
	{
		device->outbuffercleared=false;

		ahw=device->FirstOutputChannel();
		while(ahw)
		{
			if(ahw->hardwarechannelused==false)
			{
				if(ahw->zerocleared==false)
				{
					ARES *clear=device->devmix.outputbufferARES;
					clear+=device->devmix.samplesinbuffer*ahw->channelindex;
					memset(clear,0,device->devmix.samplesinbuffer_size);

					ahw->zerocleared=true;
				}
			}
			else
			{
				ahw->zerocleared=false;

				// Add Master Volume + Peak

				ARES *to=device->devmix.outputbufferARES;
				to+=device->devmix.samplesinbuffer*ahw->channelindex;

				ARES m_peak=0,p_peak=0;
				ARES mul=masterchannel.io.audioeffects.volume.GetValue();

				mul*=LOGVOLUME_SIZE;
#ifdef ARES64
				mul=logvolume[(int)mul];
#else
				mul=logvolume_f[(int)mul];
#endif

				if(mul!=1)
				{
					int i=device->GetSetSize();

					if(int loop=i/8)
					{
						i-=8*loop;

						do
						{
							ARES h=*to++ *=mul;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
							h=*to++ *=mul;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
							h=*to++ *=mul;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
							h=*to++ *=mul;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

							h=*to++ *=mul;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
							h=*to++ *=mul;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
							h=*to++ *=mul;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
							h=*to++ *=mul;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

						}while(--loop);
					}

					while(i--)
					{
						ARES h=*to++ *=mul;
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
					}	

					ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
					if(peak>peakmax)
						peakmax=peak;

				} // mul !=1
				else 
					if(!masterchannel.mastering)
					{
						// Get Peak no Volume mul

						int i=device->GetSetSize();

						if(int loop=i/8)
						{
							i-=8*loop;

							do
							{
								ARES h=*to++;
								if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
								h=*to++;
								if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
								h=*to++;
								if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
								h=*to++;
								if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

								h=*to++;
								if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
								h=*to++;
								if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
								h=*to++;
								if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
								h=*to++;
								if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

							}while(--loop);
						}

						while(i--)
						{
							ARES h=*to++;
							if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						}	

						ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
						if(peak>peakmax)
							peakmax=peak;
					}
			}

			ahw=ahw->NextChannel();
		}
	} 

	if(masterchannel.mastering)
	{
		// Master Mastering

		// Mix HW Channels -> Master Channel Format
		// DevMix->MasterMix

		MixHWChannelsToMasterMix();
		masterchannel.mastering->master->SaveBuffer(masterchannel.mastering,&device->mastermix,true);
	}
	else
	{
		masterchannel.mix.peak.SetChannelPeak(0,peakmax);
	}
}

void AudioSystem::InitDefaultBusses()
{
	//AudioPort *pmono_in=mainaudio->GetActiveDevice()?mainaudio->GetActiveDevice()->FindVInChannel(1,0):0;
	AudioPort *pmono_out=mainaudio->GetActiveDevice()?mainaudio->GetActiveDevice()->FindVOutChannel(1,0):0;

	song->audiosystem.CreateBus(0,pmono_out);
	song->audiosystem.CreateBus(0,pmono_out);
	song->audiosystem.CreateBus(0,pmono_out);
	song->audiosystem.CreateBus(0,pmono_out);

	//AudioPort *pstereo_in=mainaudio->GetActiveDevice()?mainaudio->GetActiveDevice()->FindVInChannel(2,0):0;
	AudioPort *pstereo_out=mainaudio->GetActiveDevice()?mainaudio->GetActiveDevice()->FindVOutChannel(2,0):0;

	song->audiosystem.CreateBus(0,pstereo_out);
	song->audiosystem.CreateBus(0,pstereo_out);
	song->audiosystem.CreateBus(0,pstereo_out);
	song->audiosystem.CreateBus(0,pstereo_out);

	//FindBestBusForPlayback();
}

void AudioSystem::CreateChannelsFullName()
{
	for(int i=0;i<LASTSYNTHCHANNEL;i++) // Channel, Bus, Instr
	{
		AudioChannel *c=FirstChannelType(i);

		while(c){
			c->CreateFullName();
			c=c->NextChannel();
		}
	}
}

void AudioSystem::ResetMaxPeaks()
{
	//Tracks
	Seq_Track *t=song->FirstTrack();

	while(t){
		t->GetPeak()->ResetMax();
		t=t->NextTrack();
	}

	for(int i=0;i<LASTSYNTHCHANNEL;i++) // Channel, Bus, Instr
	{
		AudioChannel *c=FirstChannelType(i);
		while(c){
			c->mix.peak.ResetMax();
			c=c->NextChannel();
		}
	}

	// Reset Device Out
	AudioChannel *c=FirstChannelType(CHANNELTYPE_DEVICEOUT);
	while(c){
		c->mix.peak.ResetMax();
		c=c->NextChannel();
	}

	masterchannel.mix.peak.ResetMax();
}

void AudioSystem::CopyAudioChannelsAndMetroTracksToDevice(AudioDevice *device)
{
	if(!device)
	{
		// No Audio Device
		for(int i=0;i<LASTSYNTHCHANNEL;i++) // Channel, Instruments, Bus
		{
			// Just clear
			AudioChannel *c=FirstChannelType(i);

			while(c)
			{
				if(c->mix.channelsused)
					c->mix.ClearOutput(c->mix.channelsused);

				c->mix.outputbufferARES=c->mix.static_outputbufferARES; // Reset
				c=c->NextChannel();
			}
		}

		return;
	}

#ifdef DEBUG
	device->devmix.CheckBuffer();
#endif

	{
		AudioChannel *bus=FirstBusChannel();

		while(bus)
		{
			//if(bus->mastering)
			//	bus->mastering->master->SaveBuffer(bus->mastering,&bus->mix,false);

			if(bus->mix.channelsused)
			{
				// Copy/Mix to Audio Hardware ?
				if( //bus->io.out_vchannel && 
					bus->io.audioeffects.mute.mute==false &&
					bus->GetSoloStatus()!=SOLO_OTHER
					//	bus->io.audioeffects.solo
					)
				{
					device->LockMix();
					bus->mix.MixAudioBuffer(&device->devmix,bus->io.out_vchannel);
					device->UnlockMix();
				}

				bus->mix.outputbufferARES=bus->mix.static_outputbufferARES; // Reset Buffer Pointer
				bus->mix.channelsused=0;
			}

			bus=bus->NextChannel();
		}
	}

#ifdef DEBUG
	device->devmix.CheckBuffer();
#endif

	// MasterMix
	AddMasterEffects(); // MasterMix Device

	//if(song->mastering==false)
	{
		// After Master Mix

		// Metro Tracks
		Seq_MetroTrack *mt=song->FirstMetroTrack();
		while(mt)
		{
			if(mt->mix.channelsused)
			{
				// Copy/Mix to Audio Hardware ?
				if(mt->io.out_vchannel)
				{
					metropar.song=song;
					metropar.startin=metropar.in=&mt->mix; // use devmix
					metropar.io=&mt->io;
					metropar.streamchannels=mt->io.GetChannels();
					metropar.bypassfx=false;

					mt->io.audioeffects.AddEffects(&metropar); // Add Effects

					device->LockMix();
					mt->mix.MixAudioBuffer(&device->devmix,mt->io.out_vchannel);
					device->UnlockMix();

					mt->mix.CreatePeakTo(mt->GetPeak());
				}

				mt->mix.outputbufferARES=mt->mix.static_outputbufferARES; // Reset Buffer Pointer
				mt->mix.channelsused=0;
			}

			mt=mt->NextMetroTrack();
		}
	}
}

void AudioSystem::CopyAudioChannels_Master(AudioDevice *device)
{
#ifdef DEBUG
	device->devmix.CheckBuffer();
#endif

	{
		AudioChannel *bus=FirstBusChannel();

		while(bus)
		{
			if(bus->mastering)
				bus->mastering->master->SaveBuffer(bus->mastering,&bus->mix,false);

			if(bus->mix.channelsused)
			{
				// Copy/Mix to Audio Hardware ?
				if( //bus->io.out_vchannel && 
					bus->io.audioeffects.mute.mute==false &&
					bus->GetSoloStatus()!=SOLO_OTHER
					//	bus->io.audioeffects.solo
					)
				{
					device->LockMix();
					bus->mix.MixAudioBuffer(&device->devmix,bus->io.out_vchannel);
					device->UnlockMix();
				}

				bus->mix.outputbufferARES=bus->mix.static_outputbufferARES; // Reset Buffer Pointer
				bus->mix.channelsused=0;
			}

			bus=bus->NextChannel();
		}
	}

#ifdef DEBUG
	device->devmix.CheckBuffer();
#endif

	AddMasterEffects(); // MasterMix Master
}

void AudioSystem::ConnectToDevice(bool connectio)
{
	// Delete old Masters
	AudioChannel *ic=FirstDeviceInChannel();
	while(ic)
		ic=DeleteAudioChannel(ic,true);

	AudioChannel *oc=FirstDeviceOutChannel();
	while(oc)
		oc=DeleteAudioChannel(oc,true);

	if(device=mainaudio->GetActiveDevice())
	{	
		// Inputs
		{
			int c=1;

			AudioHardwareChannel *in=device->FirstInputChannel();
			while(in)
			{
				if(AudioChannel *inc=new AudioChannel(this,CHANNELTYPE_DEVICEIN))
				{
					inc->flag=AudioChannel::CHANNEL_NORECORD|AudioChannel::CHANNEL_NOSOLO;
					inc->io.inputpeaks.InitPeak(1);
					inc->hardwarechannel=in;
					if(connectio==true)in->audiochannel=inc;

					audiochannels[CHANNELTYPE_DEVICEIN].AddEndO(inc);

					char help[NUMBERSTRINGLEN];

					if(char *l=mainvar->GenerateString("In ",mainvar->ConvertIntToChar(c,help)," :",in->hwname))
					{
						if(strlen(l)<SMALLSTRINGLEN-1)
							strcpy(inc->name,l);
						else
						{
							strncpy(inc->name,l,SMALLSTRINGLEN-1);
							inc->name[SMALLSTRINGLEN]=0;
						}

						delete l;
					}
					else
						strcpy(inc->name,"IN ?");
				}

				c++;
				in=in->NextChannel();
			}
		}

		{
			// Outs
			int c=1;
			AudioHardwareChannel *out=device->FirstOutputChannel();
			while(out)
			{
				if(AudioChannel *outc=new AudioChannel(this,CHANNELTYPE_DEVICEOUT))
				{
					outc->flag=AudioChannel::CHANNEL_NORECORD|AudioChannel::CHANNEL_NOSOLO;
					//outc->io.outpeaks.dropmax=false;
					outc->mix.peak.InitPeak(1);
					outc->hardwarechannel=out;
					if(connectio==true)out->audiochannel=outc;

					audiochannels[CHANNELTYPE_DEVICEOUT].AddEndO(outc);

					char help[NUMBERSTRINGLEN];

					if(char *l=mainvar->GenerateString("Out ",mainvar->ConvertIntToChar(c,help),": ",out->hwname))
					{
						if(strlen(l)<SMALLSTRINGLEN-1)
							strcpy(outc->name,l);
						else
						{
							strncpy(outc->name,l,SMALLSTRINGLEN-1);
							outc->name[SMALLSTRINGLEN]=0;
						}

						delete l;
					}
					else
						strcpy(outc->name,"OUT ?");
				}

				c++;
				out=out->NextChannel();
			}
		}
	}

	CreateChannelsFullName();
	song->InitNewSongPositionToStreams();
}

void AudioSystem::Load(camxFile *file)
{
	int nrchls=0;

	file->ReadChunk(&nrchls);
	file->ReadChunk(&channel_solomode);
	file->ReadChunk(&systembypassfx);
	file->ReadChunk(&songtracksbypassfx);

	file->CloseReadChunk();

	while(nrchls--)
	{
		file->LoadChunk();

		if(file->CheckReadChunk()==false)
			break;

		if(file->GetChunkHeader()==CHUNK_AUDIOCHANNEL)
		{	
			file->ChunkFound();

			if(AudioChannel *chl=new AudioChannel(this,0))
			{
				chl->Load(file);

				if(chl->audiochannelsystemtype<3) // Add Channel, Instr, Bus only !
				{
					//if(!activechannel)activechannel=chl;

					if(audiochannels[chl->audiochannelsystemtype].activeobject==0)
						audiochannels[chl->audiochannelsystemtype].activeobject=chl;

					audiochannels[chl->audiochannelsystemtype].AddEndO(chl);
				}
				else
				{
					chl->FreeAudioChannelMemory();
					delete chl;
				}
			}
			else
				file->JumpOverChunk();

			//FindBestBusForPlayback();
		}
	}

	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUDIOCHANNEL)
	{
		file->ChunkFound();
		masterchannel.Load(file);
		strcpy(masterchannel.name,"Master");
	}

	SetAudioSystemHasFlag(); // Refresh Flags
}

void AudioSystem::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOSYSTEM);

	int count=0;
	for(int i=0;i<LASTSYNTHCHANNEL;i++) // Channel,Bus,Aux
		count+=audiochannels[i].GetCount();

	file->Save_Chunk(count);
	file->Save_Chunk(channel_solomode);
	file->Save_Chunk(systembypassfx);
	file->Save_Chunk(songtracksbypassfx);

	file->CloseChunk();

	// Audio Channels Channels, Bus + Master
	for(int i=0;i<LASTSYNTHCHANNEL;i++)
	{
		AudioChannel *c=FirstChannelType(i);

		while(c){

			c->Save(file);
			c=c->NextChannel();
		}
	}

	// Write Master
	masterchannel.Save(file);
}

void AudioSystem::Clone(AudioSystem *to)
{
	if(to)
	{

	}
}

void AudioChannel::AddEffectToChannel(AudioDevice *device) // Called by Single or Core
{
	mix.CreatePeakTo(&io.inputpeaks); // Channel Input Peak

	outpar.startin=outpar.in=&mix;
	outpar.streamchannels=io.GetChannels();
	outpar.bypassfx=(io.bypassallfx==true || audiosystem->systembypassfx==true)?true:false;
	outpar.song=song;
	outpar.realtime=(song->status&Seq_Song::STATUS_SONGPLAYBACK_AUDIO) || song->mastering==true ?true:false;

	io.audioeffects.AddEffects(&outpar); // Add Effects

	if(outpar.startin->channelsused)
	{	
		if(outpar.startin!=&mix)
		{
			mix.outputbufferARES=outpar.startin->outputbufferARES;
			mix.channelsused=outpar.startin->channelsused;
		}

		if(song->mastering==false)
		{
			//mix.peak.copyied=false; // force
			mix.CreatePeak();
		}

	}
}

void Seq_Track::SetDefaultAudio(bool lock,Seq_Track *clone)
{
	if(clone)
	{
		if(!parent)
			clone->GetAudioOut()->CloneToGroup(GetAudioOut());

		io.SetPlaybackChannel(clone->io.out_vchannel,false,lock);
		io.SetInput(clone->io.in_vchannel);

		if(clone->t_audiofx.recordtrack)
			AddTrackToRecord(clone->t_audiofx.recordtrack);

	}
	else
	{
		/*
		if(!GetAudioOut()->FirstChannel())
		{
		for(int i=0;i<3;i++)
		{
		if(song->audiosystem.FirstChannelType(i))
		{
		GetAudioOut()->AddToGroup(song->audiosystem.FirstChannelType(i));
		break;
		}
		}
		}
		*/

		if(!io.out_vchannel)
		{
			if(mainaudio->GetActiveDevice())
				io.SetPlaybackChannel(mainaudio->GetActiveDevice()->FindVOutChannel(channelschannelsnumber[io.channel_type],0),false,lock);
		}

		if(!io.in_vchannel)
		{
			// Default Audio In
			if(mainaudio->GetActiveDevice())
				SetRecordChannel(mainaudio->GetActiveDevice()->FindVInChannel(channelschannelsnumber[io.channel_type],0),lock);
		}
	}

	io.inputpeaks.InitPeak(io.in_vchannel?io.in_vchannel->channels:channelschannelsnumber[io.channel_type]);
	mix.peak.InitPeak(channelschannelsnumber[io.channel_type]);

	//t_audiofx.mix.peak.InitPeak(channelschannelsnumber[io.channel_type]);
}

bool Seq_Track::CompareMIDIOut(Seq_Track *t)
{
	if(t->GetMIDIOut()->CompareWithGroup(GetMIDIOut())==false)
		return false;

	if(t->GetFX()->CompareOutput(GetFX())==false)
		return false;

	return true;
}

bool Seq_Track::CompareMIDIIn(Seq_Track *t)
{
	if(t->GetMIDIIn()->CompareWithGroup(GetMIDIIn())==false)
		return false;

	if(t->GetFX()->CompareInput(GetFX())==false)
		return false;

	return true;
}

/*
bool Seq_Track::CheckMIDIType()
{
#ifdef OLDIE
Seq_AudioIOPointer *acp=t_audiochannelouts.FirstChannel();

while(acp)
{
if(acp->channel->audiochannelsystemtype==CHANNELTYPE_AUDIOINSTRUMENT)
return true;

acp=acp->NextChannel();
}
#endif

return false;
}
*/

void Seq_Track::EditMIDIEventOutput(guiWindow *win)
{
	win->DeletePopUpMenu(true);

	if(win->popmenu)
	{
		char *h=mainvar->GenerateString(Cxs[CXS_TRACKTYPE]," Track:",GetName());

		if(h)
		{
			win->popmenu->AddMenu(h,0);
			win->popmenu->AddLine();
			delete h;
		}

		class menu_trackauto:public guiMenu
		{
		public:
			menu_trackauto(Seq_Track *tr)
			{
				track=tr;
			}

			void MenuFunction()
			{
				track->MIDItypesetauto=track->MIDItypesetauto==true?false:true;
			} //

			Seq_Track *track;

		};

		win->popmenu->AddFMenu(Cxs[CXS_SENDTRACKALWAYSTOAUTO],new menu_trackauto(this),MIDItypesetauto);
		win->popmenu->AddLine();

		class menu_tracktype:public guiMenu
		{
		public:
			menu_tracktype(Seq_Track *tr,int t)
			{
				track=tr;
				type=t;
			}

			void MenuFunction()
			{
				track->SetMIDIType(type,true);
				track->MIDItypesetauto=false;
			} //

			Seq_Track *track;
			int type;
		};

		win->popmenu->AddFMenu(Cxs[CXS_SENDTRACKALWAYSTOMIDI],new menu_tracktype(this,OUTPUTTYPE_MIDI),MIDItype==OUTPUTTYPE_MIDI?true:false);
		win->popmenu->AddFMenu(Cxs[CXS_SENDTRACKALWAYSTOAUDIO],new menu_tracktype(this,OUTPUTTYPE_AUDIOINSTRUMENT),MIDItype==OUTPUTTYPE_AUDIOINSTRUMENT?true:false);
		//win->popmenu->AddFMenu(Cxs[CXS_SENDTRACKALWAYSTOMIDIANDAUDIO],new menu_tracktype(this,OUTPUTTYPE_AUDIOINSTRUMENTANDMIDI),MIDItype==OUTPUTTYPE_AUDIOINSTRUMENTANDMIDI?true:false);

		// other Tracks ?
		int add=0;
		Seq_Track *c=song->FirstTrack();

		while(c)
		{
			if(c!=this && c->IsSelected()==true)
				add++;

			c=c->NextParentTrack();
		}

		if(add)
		{
			win->popmenu->AddLine();

			class menu_MIDItypeall:public guiMenu
			{
			public:
				menu_MIDItypeall(Seq_Track *t,int tid)
				{
					track=t;
					MIDItype=tid;
				}

				void MenuFunction()
				{
					Seq_Track *ct=track->song->FirstTrack();
					while(ct)
					{
						if(ct==track || (ct->flag&OFLAG_SELECTED))
						{
							ct->SetMIDIType(MIDItype);
						}

						ct=ct->NextTrack();
					}
				} //

				Seq_Track *track;
				int MIDItype;
			};

			char h2[32];
			h=mainvar->GenerateString("Track->MIDI ",Cxs[CXS_ALLSELECTEDTRACKS]," (",mainvar->ConvertIntToChar(add,h2),")");
			if(h)
			{
				win->popmenu->AddFMenu(h,new menu_MIDItypeall(this,OUTPUTTYPE_MIDI));
				delete h;
			}

			h=mainvar->GenerateString("Track->Audio Instruments ",Cxs[CXS_ALLSELECTEDTRACKS]," (",mainvar->ConvertIntToChar(add,h2),")");
			if(h)
			{
				win->popmenu->AddFMenu(h,new menu_MIDItypeall(this,OUTPUTTYPE_AUDIOINSTRUMENT));
				delete h;
			}

			/*
			h=mainvar->GenerateString("Track->Audio Instruments+MIDI ",Cxs[CXS_ALLSELECTEDTRACKS]," (",mainvar->ConvertIntToChar(add,h2),")");
			if(h)
			{
			win->popmenu->AddFMenu(h,new menu_MIDItypeall(this,OUTPUTTYPE_AUDIOINSTRUMENTANDMIDI));
			delete h;
			}
			*/
		}
		win->ShowPopMenu();
	}
}

bool Seq_Track::CanTrackBeChild(Seq_Track *track)
{
	if(!track)return false;
	if(track==this)return false;
	if(childdepth>=MAX_CHILDS)return false;
	if(track->parent==this)
		return false;

	if(parent)
	{
		Seq_Track *p=(Seq_Track *)parent;
		while(p)
		{
			if(p==track)return false;
			p=(Seq_Track *)p->parent;
		}
	}

	Seq_Track *ch=track->FirstChildTrack();
	while(ch && ch->IsTrackChildOfTrack(track)==true)
	{
		if(CanTrackBeChild(ch)==false)
			return false;

		if(childdepth+(ch->childdepth-track->childdepth)>=MAX_CHILDS)
			return false;

		ch=ch->NextTrack();
	}

	return true;
}


void Seq_Track::SetMuteFlag()
{
	bool solo=true;

	if( (song->playbacksettings.solo && song->GetFocusTrack() && this!=song->GetFocusTrack() && IsTrackChildOfTrack(song->GetFocusTrack())==false ) ||
		(song->playbacksettings.solo==0 && song->solocount>0 && CheckSoloTrack()==false)
		)
		solo=false;

	// Check Track
	if(solo==false || // Solo Playack Focus Track
		GetMute()==true ||
		GetGroups()->CheckIfPlaybackIsAbled()==true || // Group Mute
		(song->playbacksettings.solo==0 && song->groupsoloed==true && GetGroups()->CheckIfSolo()==false) // Group Solo
		)
	{
		t_muteflag=true; // Track Muted etc... Skip Audio Out, Sends etc...

		/*
		if(solo==true)
		{
		bool audiostream=false;


		// Check Audio Stream Mute ?
		Seq_Track *p=(Seq_Track *)parent;
		while(p)
		{
		if(p->t_stopaudiostream==false)
		{
		t_stopaudiostream=false; // Stream
		return;
		}

		p=(Seq_Track *)p->parent;
		}

		if(io.audioeffects.FirstEnabledEffectWithInput())
		{
		t_stopaudiostream=false; // Stream
		return;
		}

		//Bus
		if(GetAudioOut()->FirstChannel())
		{
		// Copy/Mix Buffer to Audio Channels/Bus
		Seq_AudioIOPointer *acp=GetAudioOut()->FirstChannel();

		while(acp){

		if(acp->channel->io.audioeffects.FirstEnabledEffectWithInput())
		{
		t_stopaudiostream=false; // Bus
		return;
		}

		acp=acp->NextChannel();
		}	
		}
		}
		*/

		/*
		// Sends
		AudioSend *s=io.FirstSend();

		while(s)
		{
		if(s->sendchannel && s->sendchannel->io.audioeffects.FirstEnabledEffectWithInput())
		{
		t_stopaudiostream=false; // Bus
		return;
		}

		s=s->NextSend();
		}
		*/

		//t_stopaudiostream=true;
	}
	else
	{
		//	t_stopaudiostream=false;
		t_muteflag=false; // Track can be played

		Seq_Track *pt=(Seq_Track *)parent;
		while(pt)
		{
			if(pt->t_muteflag==true)
				pt->t_muteflag=false;

			pt=(Seq_Track *)pt->parent;
		}

	}
}

bool Seq_Track::CanNewChildTrack()
{
	if(childdepth>=MAX_CHILDS)
		return false;

	return true;
}

bool Seq_Track::IsTrackChildOfTrack(Seq_Track *ct)
{
	if(!ct)return false;

	Seq_Track *p=(Seq_Track *)parent;
	while(p)
	{
		if(p==ct)return true;
		p=(Seq_Track *)p->parent;
	}

	return false;
}

bool Seq_Track::CheckIfParentIsSelected()
{
	if(!parent)return false;
	if(((Seq_Track *)parent)->CheckIfParentIsSelected())return true;
	if(((Seq_Track *)parent)->flag&OFLAG_SELECTED)return true;
	return false;
}

void Seq_Track::SetRecordTrackType(int type)
{
	if(type!=recordtracktype)
	{
		if(record==true && (song->status&Seq_Song::STATUS_RECORD) )
			return;

		recordtracktype=type;

		if(record==true)
			SetRecordMode(false,0);

		SetTempInputMonitoring();
	}
}

void Seq_Track::AddTrackToRecord(Seq_Track *rt)
{
	if(t_audiofx.recordtrack!=rt && rt)
	{
		if(Seq_TrackRecord *tr=new Seq_TrackRecord)
		{
			tr->track=this;

			if(song==mainvar->GetActiveSong())
				mainthreadcontrol->LockActiveSong();

			rt->t_audiofx.tracksrecord.AddEndO(tr);
			t_audiofx.recordtrack=rt;

			if(song==mainvar->GetActiveSong())
				mainthreadcontrol->UnlockActiveSong();
		}
	}
}

void Seq_Track::SetMIDIType(int type,bool force)
{
	// -1 Check for Audio Instrument CHannel

	//	if(force==false && CheckMIDIType()==true)
	//		type=OUTPUTTYPE_AUDIOINSTRUMENT;

	if(type>=0 && type!=MIDItype)
	{
		if(song==mainvar->GetActiveSong())
		{
			mainthreadcontrol->LockActiveSong();
			MIDItype=type;
			mainthreadcontrol->UnlockActiveSong();
		}
		else
			MIDItype=type;
	}
}



void Seq_Track::AddTrackToBusOrParent(AudioDevice *device)	// Copy Mix Tracks->AudioChannel
{
	output_par.startin=output_par.in=&mix;
	output_par.streamchannels=io.GetChannels();
	output_par.bypassfx=io.bypassallfx==true || song->audiosystem.songtracksbypassfx==true || song->audiosystem.systembypassfx==true?true:false;
	output_par.song=song;
	output_par.realtime=(song->status&Seq_Song::STATUS_SONGPLAYBACK_AUDIO) || song->mastering==true?true:false;
	output_par.playable=CheckIfPlaybackIsAble();

	output_par.io->audioeffects.AddEffects(&output_par); // Add Effects

	if(t_audiofx.FirstTrackRecord() && output_par.startin!=&mix) // Force Copy -> Mix
	{
		output_par.startin->CopyAudioBuffer(&mix);
		output_par.startin=&mix;
	}

	if(mastering)
	{
		int cused=output_par.startin->channelsused;

		if(output_par.playable==false)
			output_par.startin->channelsused=0; // force 0 Samples

		mastering->master->SaveBuffer(mastering,output_par.startin,false);

		output_par.startin->channelsused=cused;
	}

	if(output_par.startin->channelsused)
	{
		Peak *createpeak=song->mastering==false && freezing==0?GetPeak():0;

		if(output_par.playable==true)
		{
			if(parent) // Mix Child Track->Parent
			{
				int cflag;

				((Seq_Track *)parent)->FuncMixLock();

				if(((Seq_Track *)parent)->io.channel_type>io.channel_type) // + Mono->Stereo
				{
					PanLaw law(&song->project->panlaw,&io.audioeffects.pan);
					cflag=output_par.startin->ExpandMixAudioBuffer(&((Seq_Track *)parent)->mix,((Seq_Track *)parent)->io.GetChannels(),createpeak,&law,AudioHardwareBuffer::USETEMPPOSSIBLE);
				}				
				else
					cflag=output_par.startin->ExpandMixAudioBuffer(&((Seq_Track *)parent)->mix,((Seq_Track *)parent)->io.GetChannels(),createpeak,0,AudioHardwareBuffer::USETEMPPOSSIBLE);

				((Seq_Track *)parent)->FuncMixUnLock();

				if(cflag&AudioHardwareBuffer::PEAKCREATED)createpeak=0; // Avoid multi Peak Creation
			}
			else
			{
				Seq_AudioIO *aio=GetAudioOut();

				// Copy/Mix to Audio Hardware ?
				if(io.out_vchannel && (!aio->FirstChannel()))
				{
					// Track -> Device

					device->LockMix();
					output_par.startin->MixAudioBuffer(&device->devmix,io.out_vchannel);
					device->UnlockMix();
				}
				else
				{
					// Copy/Mix Buffer to Audio Channels/Bus
					int cflag=AudioHardwareBuffer::USETEMPPOSSIBLE;

					Seq_AudioIOPointer *acp=aio->FirstChannel();

					while(acp){

						if(acp->bypass==false 
							/*
							&&
							//(acp->channel->Muted()==false || acp->channel->io.CheckSend()==true) &&
							(
							acp->channel->audiosystem->channel_solomode==false || // Solo Channel or Bus ?
							(acp->channel->io.audioeffects.GetSolo()==true)
							)
							*/
							)
						{
							//int expflag;

							/*
							if(acp->channel->io.channel_type>io.channel_type) // + Mono->Stereo
							{
							PanLaw law(&song->project->panlaw,&io.audioeffects.pan);

							acp->channel->FuncMixLock(); // Sync Track -> Mix/Copy Channels
							expflag=output_par.startin->ExpandMixAudioBuffer(&acp->channel->mix,acp->channel->io.GetChannels(),createpeak,&law,cflag);
							}				
							else
							{
							*/

							acp->channel->FuncMixLock(); // Sync Track -> Mix/Copy Channels, No Pan

							int expflag=output_par.startin->ExpandMixAudioBuffer(&acp->channel->mix,acp->channel->io.GetChannels(),createpeak,0,cflag,output_par.panningdone==false?song->project->panlaw.GetPanValue():1);

							//}
							acp->channel->FuncMixUnLock();

							cflag=0; // No More USETEMP, Force Copy

							if(expflag&AudioHardwareBuffer::PEAKCREATED)
							{
								createpeak=0; // Avoid multi Peak Creation
							}
						}

						acp=acp->NextChannel();
					}	
				}
			}

			//	device->CopyTrackIOToBusOrDevice(&output_par,&io,GetAudioOut(),createpeak); // To Device or Bus
		}

		// Track Peak
		if(createpeak)
			mix.CreatePeak(output_par.startin,output_par.panningdone==false?&song->project->panlaw:0); // mix or effect buffer
	}// if channelsused
}

#define TRACKSWITHMIDIINPUTONLY 1

int Seq_Song::CreateUsedAudioTrackList(int flag,int childdepth) // PRepair Used Audio Channel List (X Core)
{
	Seq_Track *lasttrack=0,*t=FirstTrack();
	int c=0;

	firstfreecoretrack=0;	// Reset

	while(t){

		if(t->childdepth==childdepth){

			if(t->skiptrackfromCreateUsedAudioTrackList==true)
			{
				t->skiptrackfromCreateUsedAudioTrackList=false;
				goto nexttrack;
			}

			if(flag&TRACKSWITHMIDIINPUTONLY)
			{
				InsertAudioEffect *iae=t->io.audioeffects.FirstInsertAudioEffect();

				while(iae){
					if(iae->audioeffect->CanMIDIInput()==true)
						goto ok;

					iae=iae->NextEffect();
				}

				goto nexttrack;
			}

ok:

			/*
			if(t->mix.channelsused ||
			//	((status&Seq_Song::STATUS_SONGPLAYBACK_AUDIO) && t->NeedAudioAutomation()==true) || 
			t->io.audioeffects.CheckIfEffectHasOnInstrumentsOrEffectsWithInput()==true ||
			(t->t_audiofx.FirstTrackRecord() && audioinputneed==true) //||
			//t->t_stopaudiostream==false
			)
			*/
			{
				if(!firstfreecoretrack)firstfreecoretrack=t;
				else
					lasttrack->nextcoretrack=t;

				if(flag&TRACKSWITHMIDIINPUTONLY)
					t->skiptrackfromCreateUsedAudioTrackList=true; // reset later

				lasttrack=t;
				c++;
			}

		}

nexttrack:
		t=t->NextTrack_NoUILock();
	}

	if(lasttrack)
		lasttrack->nextcoretrack=0;

	return c;
}

int Seq_Song::CreateUsedAudioBusList()
{
	int ct=0;
	firstfreecorebus=0;
	AudioChannel *lastbus=0,*c=audiosystem.FirstBusChannel();

	// Busses
	while(c){

		/*
		if(c->Muted()==true)
		{
		c->mix.channelsused=0; // Reset
		}
		else
		*/

		if(c->mix.channelsused ||
			//	((status&Seq_Song::STATUS_SONGPLAYBACK_AUDIO) && c->NeedAudioAutomation()==true) || 
			c->io.audioeffects.CheckIfEffectHasOnInstrumentsOrEffectsWithInput()==true || 
			(c->io.bypassallfx==false && c->io.audioeffects.CheckIfEffectHasOnFX()==true) 
			)
		{
			if(!firstfreecorebus)firstfreecorebus=c;
			else
				lastbus->nextcorebus=c;

			lastbus=c;
			ct++;
		}

		c=c->NextChannel();
	}

	if(lastbus)lastbus->nextcorebus=0;

	return ct;
}


int Seq_Song::CreateAudioDeviceChannelList(AudioDevice *device)
{
	firstfreecoredevicechannel=0;
	AudioHardwareChannel *lastchannel=0;
	int c=0;

	AudioHardwareChannel *hw=device->FirstOutputChannel();

	while(hw)
	{
		if(!firstfreecoredevicechannel)firstfreecoredevicechannel=hw;
		if(lastchannel)lastchannel->nextcorechannel=hw;
		lastchannel=hw;
		c++;

		hw=hw->NextChannel();
	}

	if(lastchannel)lastchannel->nextcorechannel=0;

	return c;
}



int Seq_Song::CreateTrackListWithAudioInputEffects()
{
	firstfreeaudioinputtrack=0;

	Seq_Track *lasttrack=0,*t=FirstTrack();
	int c=0;

	while(t){

		if(t->io.audioinputeffects.CheckIfEffectHasOnFX()==true)
		{
			if(!firstfreeaudioinputtrack)firstfreeaudioinputtrack=t;
			else
				lasttrack->nextcoreaudioinputtrack=t;

			lasttrack=t;
			c++;
		}

		t=t->NextTrack_NoUILock();
	}

	if(lasttrack)
		lasttrack->nextcoreaudioinputtrack=0;

	return c;
}

int Seq_Song::CreateUsedTrackWithTrackInput()
{
	firstfreecoretrack=0;
	Seq_Track *lasttrack=0,*t=FirstTrack();
	int c=0;

	while(t){

		if(t->t_audiofx.recordtrack)
		{
			if(!firstfreecoretrack)firstfreecoretrack=t;
			else
				lasttrack->nextcoretrack=t;

			lasttrack=t;
			c++;
		}

		t=t->NextTrack_NoUILock();
	}

	if(lasttrack)
		lasttrack->nextcoretrack=0;

	return c;
}

int Seq_Song::CreateUsedTrackThruList()
{
	firstfreethrutrack=0;
	Seq_Track *lasttrack=0,*t=FirstTrack();
	int c=0;

	while(t){

		AudioIOFX *fx=&t->io;

		if(fx->thru==true && (fx->audioinputenable==true || fx->audioinputeffects.CheckIfEffectHasOnFX()==true))
		{
			if(fx->skipthru==true)
				fx->skipthru=false;
			else
			{
				if(fx->in_vchannel &&
					t->t_audiofx.recordtrack==0 &&
					((fx->out_vchannel && t->usedirecttodevice==true) || (t->t_audiochannelouts.FirstChannel() && t->usedirecttodevice==false))
					)
				{
					if(!firstfreethrutrack)firstfreethrutrack=t;
					else
						lasttrack->nextcorethrutrack=t;

					lasttrack=t;
					c++;
				}
			}
		}

		t=t->NextTrack_NoUILock();
	}

	if(lasttrack)
		lasttrack->nextcorethrutrack=0;

	return c;
}

bool Seq_Song::CanSongPositionBeChanged(OSTART pos)
{
	if(status&STATUS_RECORD)
		return false;

	if(status&STATUS_PLAY)
	{
		if(playbacksettings.cycleplayback==true && pos>=playbacksettings.cycleend)
			return false;
	}

	return true;
}

void Seq_Song::DoSongAudioEffects(AudioDevice *device)
{
	// X Processor System
	switch(mainvar->cpucores)
	{
	case 1: // Single Core CPU
		{
			if(mastering==false)
			{
				if(audiosystem.systemhasflag&AudioSystem::AUDIOCAMX_HASTHRUTRACKS)
				{
					if(int c=CreateUsedTrackThruList())
					{
						if(c==1)
							firstfreethrutrack->DoThru(device);
						else
							do{
								firstfreethrutrack->DoThru(device);
							}while(firstfreethrutrack=firstfreethrutrack->nextcorethrutrack);
					}
				}

				if(audiosystem.systemhasflag&AudioSystem::AUDIOCAMX_HASMIDIINPUTTRACKS)
				{
					for(int i=MAX_CHILDS;i>=0;i--){
						if(trackchildflag&(1<<i))
						{
							if(int c=CreateUsedAudioTrackList(TRACKSWITHMIDIINPUTONLY,i))
							{
								if(c==1)
									firstfreecoretrack->AddTrackToBusOrParent(device);
								else // 2 or more Tracks
									mainaudiostreamproc->DoFunction(this,DOCORETRACKS,c); // Track FX -> AudioChannel
							}
						}
					}
				}
			}// end no mastering

			// Tracks + Reset skiptrackfromCreateUsedAudioTrackList
			for(int i=MAX_CHILDS;i>=0;i--){

				if(trackchildflag&(1<<i))
				{
					if(CreateUsedAudioTrackList(0,i)) // Fx PRepair
					{
						// Tracks->Channel
						Seq_Track *t=firstfreecoretrack;
						do{
							t->AddTrackToBusOrParent(device);
						}while(t=t->nextcoretrack);
					}
				}
			}

			if(audiosystem.systemhasflag&AudioSystem::AUDIOCAMX_HASTRACKSWITHTRACKINPUT)
			{
				if(int c=CreateUsedTrackWithTrackInput())
				{
					if(c==1)
						firstfreecoretrack->DoTrackInputFromTrack();
					else
						do{
							firstfreecoretrack->DoTrackInputFromTrack();
						}while(firstfreecoretrack=firstfreecoretrack->nextcoretrack);

					//	sendrecordsignal=true;
				}
			}

			if(CreateUsedAudioBusList()){
				AudioChannel *bus=firstfreecorebus;

				// 2. AudioBus
				do{
					bus->AddEffectToChannel(device);
				}while(bus=bus->nextcorebus);
			}
		}
		break;

	default: // Multi Core CPU
		{
			if(mastering==false) // Audio Thru
			{
				if(audiosystem.systemhasflag&AudioSystem::AUDIOCAMX_HASTHRUTRACKS)
				{
					if(int c=CreateUsedTrackThruList())
					{
						if(c==1)
							firstfreethrutrack->DoThru(device);
						else
							mainaudiostreamproc->DoFunction(this,DOCORETRACKSTHRU,c); // Track Thru
					}
				}

				if(audiosystem.systemhasflag&AudioSystem::AUDIOCAMX_HASMIDIINPUTTRACKS)
				{
					for(int i=MAX_CHILDS;i>=0;i--){
						if(trackchildflag&(1<<i))
						{
							if(int c=CreateUsedAudioTrackList(TRACKSWITHMIDIINPUTONLY,i))
							{
								if(c==1)
									firstfreecoretrack->AddTrackToBusOrParent(device);
								else // 2 or more Tracks
									mainaudiostreamproc->DoFunction(this,DOCORETRACKS,c); // Track FX -> AudioChannel
							}
						}
					}
				}
			} // end no mastering

			// Tracks + Reset skiptrackfromCreateUsedAudioTrackList
			for(int i=MAX_CHILDS;i>=0;i--){
				if(trackchildflag&(1<<i))
				{
					if(int c=CreateUsedAudioTrackList(0,i))
					{
						if(c==1)
							firstfreecoretrack->AddTrackToBusOrParent(device);
						else // 2 or more Tracks
							mainaudiostreamproc->DoFunction(this,DOCORETRACKS,c); // Track FX -> AudioChannel
					}
				}
			}

			if(audiosystem.systemhasflag&AudioSystem::AUDIOCAMX_HASTRACKSWITHTRACKINPUT)
			{
				if(int c=CreateUsedTrackWithTrackInput())
				{
					if(c==1)
						firstfreecoretrack->DoTrackInputFromTrack();
					else
						mainaudiostreamproc->DoFunction(this,DOCOREAUDIOTRACKTRACKIO,c); // Track Thru

					//	sendrecordsignal=true;
				}
			}

			if(int c=CreateUsedAudioBusList())
			{
				if(c==1)
					firstfreecorebus->AddEffectToChannel(device);
				else // 2 or more Tracks
					mainaudiostreamproc->DoFunction(this,DOCOREBUS,c); // Bus FX
			}
		}
		break;
	}

	// Reset  Pointer
	{
		bool realtime=(status&Seq_Song::STATUS_SONGPLAYBACK_AUDIO) || mastering==true?true:false;

		Seq_Track *t=FirstTrack();
		while(t)
		{
			// Reset
			t->mix.outputbufferARES=t->mix.static_outputbufferARES; // Reset Child->Parent Copy
			t->mix.channelsused=0;

			t=t->NextTrack_NoUILock();
		}
	}
}

void Seq_Song::DoAudioEffects_Mastering(AudioDevice *device)
{
	DoRealtimeAndCDEvents(device);
	DoSongAudioEffects(device); // + post send XCore Track+Channel Effect

#ifdef DEBUG
	device->devmix.CheckBuffer();
#endif

	audiosystem.CopyAudioChannels_Master(device); // + Master FX

	// Reset
	AudioHardwareChannel *hw=device->FirstOutputChannel();

	while(hw)
	{
		hw->hardwarechannelused=false;
		hw=hw->NextChannel();
	}

	device->devmix.channelsused=0; // Reset Master Device Buffer
}

void Seq_Song::DoAudioEffectsAndCopyToDevice(AudioDevice *device) // RT
{
#ifdef DEBUG
	if(masteringlock==true)
		maingui->MessageBoxError(0,"DoAudioEffectsAndCopyToDevice Master");
#endif

	DoRealtimeAndCDEvents(device);

	DoSongAudioEffects(device); // + post send XCore Track+Channel Effect

#ifdef DEBUG
	device->devmix.CheckBuffer();
#endif

	audiosystem.CopyAudioChannelsAndMetroTracksToDevice(device); // + Master FX

	device->devmix.peak.current_max=device->devmix.peak.current_sum=device->CopyDeviceToHardware(this); // Main HardwareBuffer Mix + Device Out Channel Peak
}

void mainAudioRealtime::StopAudioRealtime(AudioRealtime *art,bool*endstatus)
{
	LockRTObjects();

	AudioRealtime *ar=FirstAudioRealtime(); // Check List

	while(ar && ar!=art)ar=ar->NextAudioRealtime();

	if(ar && 
		((!endstatus) || (endstatus && *endstatus==false))
		)
		mainaudioreal->StopRealtimePlayback(ar);

	UnlockRTObjects();
}

void mainAudio::StopRecordingFiles(Seq_Track *t)
{
	audiorecordthread->Lock();

	for(int i=0;i<MAXRECPATTERNPERTRACK;i++)
	{
		if(t->audiorecord_audiopattern[i] && t->audiorecord_audiopattern[i]->audioevent.audioefile)
		{
			int added=0,deleted=0;

			t->audiorecord_audiopattern[i]->audioevent.audioefile->StopRecording(t->song,&added,&deleted,false);
			t->ResetAudioRecordPattern();
		}
	}

	audiorecordthread->Unlock();
}

void mainAudio::StopRecordingFiles(Seq_Song *song /*,AudioHDFile *onefile*/)
{
	int added=0,deleted=0;
	bool refreshundos=false,changedsize=false,writeerrorfiles=true;

	// All Files ok?
	if(AudioHDFile *rec=FirstAudioRecordingFile())

		//if(!onefile)
		//{
		while(rec){

			if(rec->writefile.errorwriting==true)
			{
				writeerrorfiles=maingui->MessageBoxYesNo(0,Cxs[CXS_ERRORRECORDING_Q]);
				break;
			}

			rec=rec->NextHDFileNoLock();
		}
		//}

		AudioHDFile *rec=/*onefile?onefile:*/FirstAudioRecordingFile();

		if(rec){

			while(rec){

				AudioHDFile *nrec=rec->NextHDFileNoLock();

				rec->StopRecording(song,&added,&deleted,writeerrorfiles);

				//if(onefile)
				//	return;

				rec=nrec;
			}

			//if(added)maingui->RefreshAllEditors(0,EDITORTYPE_AUDIOMANAGER,0);
			if((deleted && added==0) || changedsize==true)
			{
				// Refresh Audio Recording GUI
				maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
			}
		}
}

void AudioChannel::FreeAudioChannelMemory()
{
	mainaudioreal->RemoveAllAudioRealtime(this); // remove channel from playback
	io.FreeMemory();
	DeleteChannelBuffersMix();

	if(fullname)delete fullname;
	fullname=0;
}

void AudioChannel::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);
	file->ReadChunk(&flag);
	file->ReadChunk(&audiochannelsystemtype); // Channel, Bus...
	file->Read_ChunkString(name);

	colour.LoadChunk(file);

	file->ReadChunk(&sizefactor);

	file->CloseReadChunk();

	io.Load(file);

	if(audiochannelsystemtype==CHANNELTYPE_MASTER)
		io.channel_type=CHANNELTYPE_MONO;

	MIDIfx.Load(file);

	LoadAutomationTracks(file,0,this);
}

void AudioChannel::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOCHANNEL);

	file->Save_Chunk((CPOINTER)this);
	file->Save_Chunk(flag);
	file->Save_Chunk(audiochannelsystemtype); // Channel, Bus...

	// Name
	file->Save_ChunkString(name);

	colour.SaveChunk(file);

	file->Save_Chunk(sizefactor);

	file->CloseChunk();

	io.Save(file);
	MIDIfx.Save(file);

	SaveAutomationTracks(file);
}

AudioPort *AudioChannel::GetVOut()
{
	return io.out_vchannel;
}

void mainAudio::ConnectAudioPortWithHardwareChannel(AudioPort *p,AudioHardwareChannel *hw,int channel)
{
	if(p->channels>channel)
	{
		mainthreadcontrol->LockActiveSong();

		p->hwchannel[channel]=hw;
		p->hwchannelchanged=true;

		mainthreadcontrol->UnlockActiveSong();

		p->GenerateName();
	}
}

char *mainAudio::GenerateSampleRateTMP()
{
	char *h=0;

	int sr=GetGlobalSampleRate();
	sr/=1000;

	char h2[NUMBERSTRINGLEN];
	h=mainvar->GenerateString(mainvar->ConvertIntToChar(sr,h2),"k");

	return h;
}

char *mainAudio::ScaleAndGenerateDBString(double f,bool adddbstring)
{
	if(f>=0 && f<=1)
	{
		f*=LOGVOLUME_SIZE;
		//	f=floor(f+0.5);

		return GenerateDBString(logvolume[(int)f],adddbstring);
	}

	return 0;
}

char *mainAudio::GenerateDBString(double f,bool adddbstring)
{
	if(f<=silencefactor)return mainvar->GenerateString("-oo");

	//TRACE ("GDB %f \n",f);

	double conv=ConvertFactorToDb(f);

	if(conv==0)return mainvar->GenerateString("0.00");

	//conv*=10;
	//conv=floor(conv);
	//conv/=10;

	char dbstring[20];

	if(adddbstring==true)
	{
		char *dbs=mainvar->ConvertDoubleToChar(conv,dbstring,2);
		mainvar->AddString(dbs,"dB");
		return mainvar->GenerateString(dbs);
	}

	return mainvar->GenerateString(mainvar->ConvertDoubleToChar(conv,dbstring,2));
}

void AudioSystem::CloseAudioSystem(bool full)
{
	//	if((song==mainvar->GetActiveSong()) || songwasactivesong==true){
	//	mainaudioreal->StopAllRealtimeEvents(); // Stop Realtime Audio Events
	//mainaudio->StopDevices(NO_SYSTEMLOCK);
	//}

	if(full==true)
		DeleteAllChannels();
}

void Seq_Song::AddAudioMetronome(AudioDevice *device)
{
	// Metronome
	while(metronome.nextclick_sample[AUDIOMETROINDEX]>=playback_sampleposition && 
		metronome.nextclick_sample[AUDIOMETROINDEX]<playback_sampleendposition
		)
	{
		if(metronome.on==true && device)
		{
			if(((status&Seq_Song::STATUS_RECORD) && metronome.record==true) ||
				((status&Seq_Song::STATUS_PLAY) && metronome.playback==true)
				)
			{
				int sampleoffset=(int)(metronome.nextclick_sample[AUDIOMETROINDEX]-playback_sampleposition);

				if(sampleoffset>=0 && sampleoffset<playback_samplesize)
				{
					bool hi=metronome.CheckIfHi(metronome.nextclick[AUDIOMETROINDEX]);

					Seq_MetroTrack *mt=FirstMetroTrack();

					while(mt)
					{
#ifdef DEBUG
						if(mt->metroclick_a.audiobuffer.samplesinbuffer!=device->GetSetSize())
							maingui->MessageBoxError(0,"Metro Track Buffer Click A");

						for(int i=0;i<4;i++)
							if(mt->metroclick_b[i].audiobuffer.samplesinbuffer!=device->GetSetSize())
								maingui->MessageBoxError(0,"Metro Track Buffer Click B");
#endif

						if(mt->GetMute()==false && mt->metrosendtoaudio==true)
						{
							AudioRealtime *ar=0;
							AudioHDFile *ahd=0;

							switch(hi)
							{
							case true:
								{
									// Measure
									ar=&mt->metroclick_a;
									ahd=mainaudio->metro_a;
								}
								break;

							default:
								{
									//Beat
									ar=&mt->metroclick_b[mt->metroclickcount];

									if(mt->metroclickcount==3)
										mt->metroclickcount=0;
									else
										mt->metroclickcount++;

									ahd=mainaudio->metro_b;
								}
								break;
							}

#ifdef DEBUG
							if((!ar) || (!ahd))
								maingui->MessageBoxError(0,"Metro Click Realtime");
							else
							{
								if(ahd->samplerate!=mainaudio->GetGlobalSampleRate())
									maingui->MessageBoxError(0,"Metro Click Realtime Samplerate");

								if(ar->audiobuffer.samplesinbuffer!=device->GetSetSize())
									maingui->MessageBoxError(0,"Metro Click Realtime SetSize");


							}
#endif

							mainaudioreal->AddAudioRealtime_AR(this,device,mt,0,ar,ahd,0,0,sampleoffset+playback_sampleoffset,true);

							//TRACE ("Metro Clicked \n");

							//mainaudioreal->AddAudioRealtime(this,device,mt,0,metronome.CheckIfHi(metronome.nextclick[AUDIOMETROINDEX])==true?mainaudio->metro_a:mainaudio->metro_b,0,0,sampleoffset+playback_sampleoffset,true);
						}

						mt=mt->NextMetroTrack();
					}
				}
#ifdef DEBUG
				else
					maingui->MessageBoxError(0,"Audio metro Offset");///
#endif
			}
		}

		// Init next metro click
		metronome.InitMetroClickForce(AUDIOMETROINDEX);
	}
}

AudioSystem::AudioSystem()
{
	device=0;
	songtracksbypassfx=systembypassfx=false;
	channel_solomode=false;
	systemhasflag=0;
	//	activechannel=0;
	masterpar.itsmaster=true;
	masterpar.channel=&masterchannel;

	masterchannel.next=masterchannel.prev=0;
}

int AudioSystem::GetCountOfObjectsUsingChannel(AudioChannel *chl)
{
	int c=0;

	// Tracks
	Seq_Track *t=song->FirstTrack();
	while(t)
	{
		if(!t->parent)
		{
			// Track Out
			Seq_AudioIOPointer *out=t->GetAudioOut()->FirstChannel();

			while(out)
			{
				if(out->channel==chl)
					c++;

				out=out->NextChannel();
			}
		}

		t=t->NextTrack();
	}

	if(chl->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL)
	{
		// Find Sends

		// Tracks
		Seq_Track *t=song->FirstTrack();
		while(t)
		{
			//if(!t->parent)
			{
				if(t->io.FindSend(chl))
					c++;
			}

			t=t->NextTrack();
		}

		// Audio Channels
		for(int i=0;i<LASTSYNTHCHANNEL;i++) // Channels+Instrumets
		{
			AudioChannel *chl=FirstChannelType(i);
			while(chl)
			{
				if(chl->io.FindSend(chl))
					c++;

				chl=chl->NextChannel();
			}
		}
	}

	return c;
}

#ifdef OLDIE
void AudioSystem::SetActiveChannel(AudioChannel *ac)
{
	if(ac!=activechannel)
	{
		activechannel=ac;

		guiWindow *w=maingui->FirstWindow();

		while(w)
		{		
			if(w->WindowSong()==song)
				switch(w->GetEditorID())
			{
				case EDITORTYPE_AUDIOMIXER:
					{
						Edit_AudioMix *eam=(Edit_AudioMix *)w;

						//eam->ShowAllChannelPeaks();
						eam->ShowActiveChannel();
					}
					break;
			}

			w=w->NextWindow();
		}		
	}
}
#endif

void AudioSystem::SetAudioChannelFx(AudioChannel *c,bool onoff)
{
	c->io.bypassallfx=onoff;
}

void AudioSystem::SetSystemByPass(bool onoff)
{
	systembypassfx=onoff;
}

void AudioSystem::SetTrackSystemByPass(bool onoff)
{
	songtracksbypassfx=onoff;
}

void AudioSystem::SetAudioSystemHasFlag()
{
	int flag=0;

	Seq_Track *t=song->FirstTrack();
	while(t)
	{
		if(!(flag&AUDIOCAMX_HASMIDIINPUTTRACKS))
		{
			InsertAudioEffect *iae=t->io.audioeffects.FirstInsertAudioEffect();
			while(iae)
			{
				if(iae->audioeffect->CanMIDIInput()==true)
				{
					flag|=AUDIOCAMX_HASMIDIINPUTTRACKS;
					break;
				}

				iae=iae->NextEffect();
			}
		}

		if(!(flag&AUDIOCAMX_HASTHRUTRACKS))
		{
			if(t->io.thru==true && t->io.in_vchannel)
			{
				flag|=AUDIOCAMX_HASTHRU;
				flag|=AUDIOCAMX_HASTHRUTRACKS;
			}
		}

		if(t->t_audiofx.recordtrack)
			flag|=AUDIOCAMX_HASTRACKSWITHTRACKINPUT;

		if(!(flag&AUDIOCAMX_HASINPUTEFFECTS))
		{
			if(t->io.audioinputeffects.FirstInsertAudioEffect())
			{
				flag|=AUDIOCAMX_HASINPUTEFFECTS;
			}
		}

		/*
		int tflag=AUDIOCAMX_HASMIDIINPUTTRACKS|AUDIOCAMX_HASTHRUTRACKS;

		int h=flag&tflag;
		*/

		// All Set ?
		if((flag&(AUDIOCAMX_HASMIDIINPUTTRACKS|AUDIOCAMX_HASTHRUTRACKS|AUDIOCAMX_HASMIDIINPUTTRACKS|AUDIOCAMX_HASTRACKSWITHTRACKINPUT))==(AUDIOCAMX_HASMIDIINPUTTRACKS|AUDIOCAMX_HASTHRUTRACKS|AUDIOCAMX_HASMIDIINPUTTRACKS|AUDIOCAMX_HASTRACKSWITHTRACKINPUT) )
			goto exit; // found all

		t=t->NextTrack();
	}

exit:

	systemhasflag=flag;
}

void AudioSystem::SelectBusFromTo(AudioChannel *from,AudioChannel *to,bool unselect,bool toggle)
{
	if(from && to)
	{
		if(from->GetIndex()>to->GetIndex())
		{
			AudioChannel *h=to;
			to=from;
			from=h;
		}

		if(unselect==true)
		{
			AudioChannel *c=from->PrevChannel();
			while(c)
			{
				c->UnSelect();
				c=c->PrevChannel();
			}

			c=to->NextChannel();
			while(c)
			{
				c->UnSelect();
				c=c->NextChannel();
			}
		}

		do
		{
			if(toggle==true)
			{
				if(from->IsSelected()==true)
					from->UnSelect();
				else
					from->Select();

				//changed++;
			}
			else
			{
				if(from->IsSelected()==false)
					from->Select();
			}

			if(from==to)
				return;

			from=from->NextChannel();

		}while(from);
	}

}


void AudioSystem::SelectSingleBus(AudioChannel *chl)
{
	AudioChannel *c=FirstBusChannel();

	while(c)
	{
		if(c!=chl)
			c->UnSelect();
		else
			c->Select();

		c=c->NextChannel();
	}
}

void AudioSystem::SetFocusBus(AudioChannel *fb,bool refreshgui)
{
	if(!fb)
	{
		audiochannels[CHANNELTYPE_BUSCHANNEL].activeobject=0;
		return;
	}

	if(fb->audiochannelsystemtype!=CHANNELTYPE_BUSCHANNEL)
		return;

	if(maingui->GetShiftKey()==true)
		SelectBusFromTo(GetFocusBus(),fb,true,false);
	else
		if(maingui->GetCtrlKey()==false)
			SelectSingleBus(fb);
		else
			fb->Select();

	audiochannels[CHANNELTYPE_BUSCHANNEL].activeobject=fb;

	if(refreshgui==true)
	{
	song->SetFocusType(Seq_Song::FOCUSTYPE_BUS,false);
	song->RefreshAudioFocusWindows(0,fb,0);
	}
}

void AudioSystem::FindBestVChannels()
{
	if(device)
	{

		{// Song Tracks
			Seq_Track *t=song->FirstTrack();

			while(t){

				// Track In
				t->io.in_vchannel=device->FindVInChannel(t->io.invchannel_channels>=1?t->io.invchannel_channels:t->io.GetChannels(),t->io.invchannel_index);
				t->io.inputpeaks.InitPeak(t->io.in_vchannel?t->io.in_vchannel->channels:0);

				// Track Out
				t->io.out_vchannel=device->FindVOutChannel(t->io.outvchannel_channels>=1?t->io.outvchannel_channels:t->io.GetChannels(),t->io.outvchannel_index);
				t->GetPeak()->InitPeak(t->io.out_vchannel?t->io.out_vchannel->channels:0);

				t=t->NextTrack();
			}
		}

		{// Metro

			Seq_MetroTrack *mt=song->FirstMetroTrack();

			while(mt){

				// Track Out
				mt->io.out_vchannel=device->FindVOutChannel(mt->io.outvchannel_channels>=1?mt->io.outvchannel_channels:mt->io.GetChannels(),mt->io.outvchannel_index);
				mt->GetPeak()->InitPeak(mt->io.out_vchannel?mt->io.out_vchannel->channels:0);

				mt=mt->NextMetroTrack();
			}
		}

		for(int i=0;i<LASTSYNTHCHANNEL;i++) // Channels,Bus,Instr
		{
			AudioChannel *c=FirstChannelType(i);

			while(c){

				// Channel In
				c->io.in_vchannel=device->FindVInChannel(c->io.invchannel_channels>=1?c->io.invchannel_channels:c->io.GetChannels(),c->io.invchannel_index);
				c->io.inputpeaks.InitPeak(c->io.in_vchannel?c->io.in_vchannel->channels:0);

				// Channel Out
				c->io.out_vchannel=device->FindVOutChannel(c->io.outvchannel_channels>=1?c->io.outvchannel_channels:c->io.GetChannels(),c->io.outvchannel_index);
				c->mix.peak.InitPeak(c->io.out_vchannel?c->io.out_vchannel->channels:0);

				c=c->NextChannel();
			}
		}

		// Master
		masterchannel.io.out_vchannel=device->FindVOutChannel(masterchannel.io.GetChannels(),masterchannel.io.outvchannel_index);
		masterchannel.mix.peak.InitPeak(masterchannel.io.out_vchannel?masterchannel.io.out_vchannel->channels:0);
	}
}

void AudioSystem::ChangeAllToAudioDevice(AudioDevice *setdevice,bool connectIO)
{
	//bool newdevice=(device==setdevice)?false:true;

	//if(newdevice==true)
	//	CreateOldChannelIndex();

	device=setdevice; // setdevice MAYBE NULL !!

	//if(newdevice==true)
	ConnectToDevice(connectIO);

	FindBestVChannels();

	song->RefreshAudioBuffer();
	maingui->RefreshAllEditors(song,EDITORTYPE_AUDIOMIXER,0); // Refresh Channels, ELSE invalid Channel Pointer !

	if(device)
	{
		if(mainsettings->audiodefaultdevice)
		{
			if(device->devname && strcmp(device->devname,mainsettings->audiodefaultdevice)==0)
				return; // Default Device set

			delete mainsettings->audiodefaultdevice;
		}

		mainsettings->audiodefaultdevice=mainvar->GenerateString(device->devname);
		if(mainsettings->audiodefaultdevice)
			mainsettings->Save(0);
	}
}

void AudioChannel::Init(AudioSystem *sys,int stype)
{
	MIDIoutputimpulse=0;

	fullname=0;
	song=sys->song;

	io.audiosystem=audiosystem=sys;
	audiochannelsystemtype=stype; // Channel, Bus, Instrument

	io.inputpeaks.InitPeak(MAXCHANNELSPERCHANNEL);

	if(stype==CHANNELTYPE_MASTER)
	{
		strcpy(name,"Master");
		io.channel_type=CHANNELTYPE_MONO;
	}
	else
		name[0]=0;

	flag=0;
	io.audioeffects.channel=this;
	outpar.channel=this;
	outpar.io=&io;
	hardwarechannel=0;
}

AudioChannel::AudioChannel(AudioSystem *sys,int stype)
{
	id=OBJ_AUDIOCHANNEL;
	channel=this;
	Init(sys,stype);
}

bool AudioChannel::Muted(bool solocheck)
{
	if(io.audioeffects.GetSolo()==true)return true;

	if(AudioSend *s=io.FirstSend())
	{
		do
		{
			if(s->sendpost==false)
				return false;

			s=s->NextSend();
		}while(s);
	}

	if(solocheck==true && audiosystem){

		if(audiosystem->channel_solomode==true && io.audioeffects.GetSolo()==false)
			return true;
	}

	return false;
}

bool Seq_Track::CanChangeType()
{
	if(record==true && (song->status&Seq_Song::STATUS_RECORD))
		return false;

	return true;
}

bool Seq_Track::CheckTracking(int flag)
{
	bool add=false;

	if((!parent) || ParentShowChilds()==true)
	{
		switch(flag)
		{
		case AUTO_ALL:
			add=true;
			break;

		case AUTO_SELECTED:
			add=IsSelected();
			break;

		case AUTO_FOCUS:
			add=this==song->GetFocusTrack()?true:false;
			break;

		case AUTO_WITHAUDIOORMIDI:
			if(MIDItype==Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT)
				add=true;
			else
				add=FirstPattern(MEDIATYPE_AUDIO|MEDIATYPE_AUDIO_RECORD|MEDIATYPE_MIDI)?true:false;

			if(add==false)
			{
				if(io.audioeffects.CheckIfEffectHasOnInstruments()==true || io.audioinputeffects.CheckIfEffectHasOnInstruments()==true)
					add=true;
			}
			break;

		case AUTO_WITHAUDIO:
			if(MIDItype==Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT)
				add=true;
			else
				add=FirstPattern(MEDIATYPE_AUDIO|MEDIATYPE_AUDIO_RECORD)?true:false;

			if(add==false)
			{
				if(io.audioeffects.CheckIfEffectHasOnInstruments()==true || io.audioinputeffects.CheckIfEffectHasOnInstruments()==true)
					add=true;
			}
			break;

		case AUTO_WITHMIDI:
			add=FirstPattern(MEDIATYPE_MIDI)?true:false;
			break;

		case AUTO_INSTRUMENT:
			add=MIDItype==Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT?true:false;

			if(add==false)
			{
				if(io.audioeffects.CheckIfEffectHasOnInstruments()==true || io.audioinputeffects.CheckIfEffectHasOnInstruments()==true)
					add=true;
			}
			break;

		case AUTO_RECORDINGTRACKS:
			if(song->IsRecording()==false || record==true)
				add=true;
			break;
		}	
	}

	return add;
}

#ifdef _DEBUG
void AudioSystem::ShowInfo()
{
	TRACE ("############## Audio System Device: %d %s ####################\n",song->audiosystem.device,song->audiosystem.device?song->audiosystem.device->devname:"-");
	if(song->audiosystem.device)
	{
		TRACE ("Device Set Size %d\n",song->audiosystem.device->GetSetSize());
		TRACE ("Device Set Smaples Ms %d\n",song->audiosystem.device->samplebufferms_long);
	}
}
#endif


