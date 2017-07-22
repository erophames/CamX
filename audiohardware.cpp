#include "defines.h"

#include <stdio.h>
#include <string.h>
#include "songmain.h"

#include "object_song.h"
#include "object_track.h"
#include "audiofile.h"
#include "audiohardware.h"
#include "gui.h"
#include "semapores.h"
#include "audiothread.h"
#include "editortypes.h"
#include "audiodevice.h"

#ifdef WIN32
#include "asio/asio.h"
#endif

#include "audiopeakbuffer.h"

#include "vstplugins.h"

void AudioDevice::CheckSetSize()
{
	if(GetSetSize()==0)
	{
		SetBufferSize(GetPrefBufferSize());

		if(GetSetSize()<=0 || GetSetSize()>8192)
			SetBufferSize(512);
	}

}

void AudioDevice::InitAudioDevice()
{
	InitMinMaxPrefBufferSizes();
	CheckSetSize();

	CalcSampleBufferMs(mainaudio->GetGlobalSampleRate(),GetSetSize());
	CalcTicksPerBuffer();
	CreateAudioBuffers();
}

void AudioDevice::WaitForRecordingEnd()
{
	// Wait for Audio Recoring Last Buffer
	LockRecordBuffer();

	while(recordbuffer_readwritecounter)
	{
		UnlockRecordBuffer();
		Sleep(10);
		LockRecordBuffer();
	}

	UnlockRecordBuffer();
}

void AudioDevice::ConvertSampleRateToIndex()
{
switch(mainaudio->GetGlobalSampleRate())
	{
	case 44100:
		usebuffersettings=ADSR_44;
		break;

	case 48000:
		usebuffersettings=ADSR_48;
		break;

	case 88200:
		usebuffersettings=ADSR_88;
		break;

	case 96000:
		usebuffersettings=ADSR_96;
		break;


	case 176400:
		usebuffersettings=ADSR_176;
		break;

	case 192000:
		usebuffersettings=ADSR_192;
		break;

	case 352800:
		usebuffersettings=ADSR_352;
		break;

	case 384000:
		usebuffersettings=ADSR_384;
		break;
	}
}

void AudioDevice::InitNewAudioDevice()
{
	switch(mainaudio->GetGlobalSampleRate())
	{
	case 44100:
		if(IsSampleRate(ADSR_44)==false)
		{
			mainaudio->SetGlobalSampleRate(GetBestSampleRate());
		}
		break;

	case 48000:
		if(IsSampleRate(ADSR_48)==false)
		{
			mainaudio->SetGlobalSampleRate(GetBestSampleRate());
		}
		break;

	case 88200:
		if(IsSampleRate(ADSR_88)==false)
		{
			mainaudio->SetGlobalSampleRate(GetBestSampleRate());
		}
		break;

	case 96000:
		if(IsSampleRate(ADSR_96)==false)
		{
			mainaudio->SetGlobalSampleRate(GetBestSampleRate());
		}
		break;

	case 176400:
		if(IsSampleRate(ADSR_176)==false)
		{
			mainaudio->SetGlobalSampleRate(GetBestSampleRate());
		}
		break;

	case 192000:
		if(IsSampleRate(ADSR_192)==false)
		{
			mainaudio->SetGlobalSampleRate(GetBestSampleRate());
		}
		break;

	case 256000:
		if(IsSampleRate(ADSR_352)==false)
		{
			mainaudio->SetGlobalSampleRate(GetBestSampleRate());
		}
		break;

	case 384000:
		if(IsSampleRate(ADSR_384)==false)
		{
			mainaudio->SetGlobalSampleRate(GetBestSampleRate());
		}
		break;
	}

	ConvertSampleRateToIndex();

	if(devicepRepared==false)
		InitAudioDeviceChannels();

	InitAudioDevice();
	StartAudioHardware();

	devicepRepared=true;
}

void AudioIOFX::RefreshEffects(AudioDevice *device)
{
	audioeffects.RefreshBuffer(device);
	audioinputeffects.RefreshBuffer(device);
}

void AudioIOFX::DeleteAllEffectBuffers()
{
	audioeffects.DeleteAllEffectBuffers();
	audioinputeffects.DeleteAllEffectBuffers();
}

void AudioIOFX::ResetPlugins()
{
	audioeffects.ResetPlugins();
	audioinputeffects.ResetPlugins();
}

void AudioIOFX::RefreshDo()
{
	// Close Plugins/Save Data->Buffer
	audioeffects.RefreshDo(); 
	audioinputeffects.RefreshDo();
}

void AudioIOFX::PreRefreshDo()
{
	// Reset Plugins
	audioeffects.PreRefreshDo();
	audioinputeffects.PreRefreshDo();
}

void AudioIOFX::InitIOOldChannel()
{
	// In 
	invchannel_index=in_vchannel?in_vchannel->portindex:0;
	invchannel_channels=in_vchannel?in_vchannel->channels:-1;

	// Out
	outvchannel_index=out_vchannel?out_vchannel->portindex:0;
	outvchannel_channels=out_vchannel?out_vchannel->channels:-1;
}

void mainAudio::SelectOtherDevice(AudioDevice *device)
{
	if(device && selectedaudiohardware)
	{
		if(device->GetIndex()>=0)
		{
			selectedaudiohardware->SetActiveDeviceInit(); // Stop old

			if(GetActiveDevice()) // Close old Device
			{
				GetActiveDevice()->CloseAudioDevice(false);
				selectedaudiohardware->activedevice=0; // Reset old
			}

			selectedaudiohardware->SetActiveDevice(device); // Init new Device
			device->StartDevice();
		}
	}
}

bool mainAudio::SetAudioSystem(int index) // ASIO, DirectS...
{
	TRACE ("SetAudioSystem\n");

	AudioHardware *hardware=(AudioHardware *)audiohardware.GetO(index);

	if(hardware && hardware!=selectedaudiohardware)
	{
		AudioDevice *newdevice=hardware->FirstDevice();

		while(newdevice)
		{
			if(mainvar->CheckSampleRateOfNewDevice(newdevice)==true)
			{
				break;
			}
			newdevice=newdevice->NextDevice();
		}

		if(selectedaudiohardware)
			selectedaudiohardware->SetActiveDeviceInit(); // Stop old

		if(GetActiveDevice()) // Close old Device
			GetActiveDevice()->StopDevice();

		mainthreadcontrol->LockActiveSong();

		selectedaudiohardware=hardware;
		hardware->activedevice=0; // Reset new
		mainthreadcontrol->UnlockActiveSong();

		// Init New Hardware
		if(newdevice)
		{
			hardware->SetActiveDevice(newdevice); // Init new Device
		}

		return true;
	}

	return false;
}

void AudioHardware::SetActiveDeviceInit()
{
	if(mainvar->GetActiveSong())
		mainvar->GetActiveSong()->StopSong(0,mainvar->GetActiveSong()->GetSongPosition());

	// Close All Open Realtime File
	mainaudioreal->StopAllRealtimeEvents(); // Lock Realtime !
	//mainaudio->StopAudio(true,NO_AUDIOTHRUREFRESH); // stop active device and no thru/instr refresh!
}

void AudioHardware::SetActiveDevice(AudioDevice *device)
{
	if(device!=activedevice) // NULL==close Device
	{
		bool ok=false;

		SetActiveDeviceInit();

		if(activedevice){
			activedevice->CloseAudioDevice(false);
			activedevice=0;
		}

		if(device)
		{
			if(device->OpenAudioDevice(device->devname)==true){

				activedevice=device; // Device

				device->InitNewAudioDevice();

				mainaudio->OpenMetroClicks();

				if(mainvar->GetActiveSong())
					mainvar->GetActiveSong()->audiosystem.ChangeAllToAudioDevice(device,true);

				mainaudioreal->StopAllRealtimeEvents();

				device->StartDevice();

				ok=true;
			}
			else{
				activedevice=0;
				maingui->MessageBoxOk(0,"Unable to open Audio Device!");
			}
		}

		if(ok==true){

			/*
			// Refresh Audio Effect Buffers
			Seq_Project *p=mainvar->GetActiveProject();

			while(p){
			Seq_Song *s=p->FirstSong();

			while(s){
			s->audiosystem.ChangeAllToAudioDevice(device);
			s=s->NextSong();
			}

			p=p->NextProject();
			}
			*/

			//if(mainvar->GetActiveSong()){

			//	mainvar->GetActiveSong()->audiosystem.ChangeAllToAudioDevice(device);

			//	mainvar->GetActiveSong()->PRepairPlayback(mainvar->GetActiveSong()->GetSongPosition(),MEDIATYPE_AUDIO);
			//}

			//if(lock==true)
			//	mainthreadcontrol->UnlockActiveSong();

			
		}// if device
		//else
		//{
		//	if(lock==true)
		//		mainthreadcontrol->UnlockActiveSong();
		//}
	}
}