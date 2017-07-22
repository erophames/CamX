#ifdef WIN32

/*
#include "directs.h"
#include "directsounddevice.h"
#include "audiohardware.h"

DirectS directsound;

bool CALLBACK DSoundEnu(LPGUID,
						LPCSTR pcName,
						LPCSTR pcDriver,
						LPVOID pContext
						)
{
	AudioHardware *adhw=(AudioHardware *)mainaudio->audiohardware.Getc_end();
	
	if(adhw)
	{
		AudioDevice *dev=adhw->FirstDevice();
		
		if(dev)
		{
			AudioHardwareChannel *inchl=new AudioHardwareChannel;
			AudioHardwareChannel *outchl=new AudioHardwareChannel;
			
			if(inchl && outchl)
			{
				// Input
				inchl->iotype=AUDIOCHANNEL_INPUT;
				strcpy(inchl->name,pcName);
				inchl->channels=2;
				inchl->sizeofsample=sizeof(short);
				inchl->numberofsamples=1024;
				dev->inchannels.AddEndO(inchl);
				
				
				// Output
				outchl->iotype=AUDIOCHANNEL_OUTPUT;
				strcpy(outchl->name,pcName);
				outchl->channels=2;
				outchl->sizeofsample=sizeof(short);
				outchl->numberofsamples=1024;
				
				dev->outchannels.AddEndO(outchl);
			}
		}
	}
	
	return true;
}

void DirectS::CollectDevices()
{
	AudioHardware *dhardware=new AudioHardware("Direct Sound",AUDIOCAMX_DIRECTS);
	
	if(dhardware)
	{
		DSOUND_AudioDevice *ad=new DSOUND_AudioDevice;
		
		if(ad)
		{
			ad->setSize=1024;

			mainaudio->audiohardware.AddEndO(dhardware);

			strcpy(ad->name,"Direct Sound System");
			dhardware->AddAudioDevice(ad);
			
			DirectSoundEnumerate(DSoundEnu,NULL); // Collect Channels

			// 
			ad->in_channels=ad->inchannels.GetCount();
			ad->out_channels=ad->outchannels.GetCount();
		}
		else
			delete dhardware;
	}
}

// Device
bool DSOUND_AudioDevice::InitAudioDevice()
{	
	init=true;
				
	samplerateset=frequency=(long)48000; // Default Rate
				
	bitresolution=16;
				
	byteresolution=bitresolution/8;
				
	if(bitresolution)
	{
		// 
		dbdynamic=6.0206;
		dbdynamic*=bitresolution;
		
		GetAudioDeviceBufferSize();
	
		
		// ASIOControlPanel();
	} // bitresolution >=16bit
		
	return true;
}


*/
#endif