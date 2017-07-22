#include "defines.h"
#include <stdio.h>
#include <string.h>

#include "asio/ginclude.h"
#include "asio/asiosys.h"
#include "asio/asio.h"
#include "asio/asiodrivers.h"
#include "asio/iasiodrv.h"

#include "object_song.h"
#include "asio_device.h"

#include "audiohardware.h"
#include "audiodevice.h"
#include "languagefiles.h"
#include "audiohdfile.h"
#include "audiorecord.h"
#include "songmain.h"
#include "audiohardwarechannel.h"

#include "gui.h"
#include "semapores.h"

bool loadAsioDriver(char *name);

// Asio CallsBacks

// Every Hardware gets its own callbacks
static ASIOCallbacks asioCallbacks_1;

/*
static ASIOCallbacks asioCallbacks_2;
static ASIOCallbacks asioCallbacks_3;
static ASIOCallbacks asioCallbacks_4;
*/


extern IASIO *theAsioDriver;
extern AsioDrivers* asioDrivers;

AudioDevice_ASIO *asioaudiodevice=0;

void AudioDevice_ASIO::SkipDeviceOutputBuffer(Seq_Song *song)
{
	ConvertInputDataToSongInputARES(0);
	ClearOutputBuffer(song);
}

// Callback Functions ----------------------------------------------------------------------------------------------
static void bufferSwitch_1(long doubleBufferIndex, ASIOBool directProcess)
{
	mainthreadcontrol->Lock(CS_audioplayback);

	if(asioaudiodevice && asioaudiodevice->resetrequest==false)
	{
		asioaudiodevice->ASIOCall(doubleBufferIndex);
	}

	mainthreadcontrol->Unlock(CS_audioplayback);
}

ASIOTime* bufferSwitchTimeInfo_1(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
{
	mainthreadcontrol->Lock(CS_audioplayback);

	if(asioaudiodevice && asioaudiodevice->resetrequest==false)
	{
		asioaudiodevice->ASIOCall(doubleBufferIndex);
	}

	mainthreadcontrol->Unlock(CS_audioplayback);
	return 0L;
}

static void sampleRateChanged_1(ASIOSampleRate sRate)
{
	if(AudioDevice *device=asioaudiodevice)
	{

	}
}

static long asioMessages_1(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch(selector)
	{
	case kAsioSelectorSupported:
		if(value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them


			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor

			)
			ret = 1L;
		break;

	case kAsioLatenciesChanged:
		// This will inform the host application that the drivers were latencies changed.
		// Beware, it this does not mean that the buffer sizes have changed!
		// You might need to update internal delay data.

		// New Buffer Size ?
		if(AudioDevice *device=asioaudiodevice)
		{
			device->resetrequest=true;
		}

		ret = 1L;
		break;

	case kAsioResetRequest:
		// defer the task and perform the reset of the driver during the next "safe" situation
		// You cannot reset the driver right now, as this code is called from the driver.
		// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
		// Afterwards you initialize the driver again.
		// asioDriverInfo.stopped;  // In this sample the processing will just stop

		// New Buffer Size ?
		if(AudioDevice *device=asioaudiodevice)
		{
			device->resetrequest=true;
		}

		ret = 1L;
		break;

	case kAsioResyncRequest:
		// This informs the application, that the driver encountered some non fatal data loss.
		// It is used for synchronization purposes of different media.
		// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
		// Windows Multimedia system, which could loose data because the Mutex was hold too long
		// by another thread.
		// However a driver can issue it in other situations, too.
		ret = 1L;
		break;



	case kAsioEngineVersion:
		// return the supported ASIO version of the host application
		// If a host applications does not implement this selector, ASIO 1.0 is assumed
		// by the driver
		ret = 2L;
		break;

	case kAsioSupportsTimeInfo:
		// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
		// is supported.
		// For compatibility with ASIO 1.0 drivers the host application should always support
		// the "old" bufferSwitch method, too.
		ret = 1;
		break;

	case kAsioSupportsTimeCode:
		// informs the driver wether application is interested in time code info.
		// If an application does not need to know about time code, the driver has less work
		// to do.
		ret = 0;
		break;
	}

	return ret;
}

AudioDevice_ASIO::AudioDevice_ASIO()
{
	audiosystemtype=AUDIOCAMX_ASIO;

	asiouseoutready=false;
	bufferarray=0;
	sync=true;
}

void AudioDevice_ASIO::MessageASIOError(char *from,ASIOError error)
{
	if(resetrequest==true)
		return;

	if(mainaudio->collectmode==true)return;

	if(error==ASE_OK || error==ASE_SUCCESS)return;

	char *h=0;
	switch(error)
	{
	case ASE_NotPresent:
#ifdef DEBUG
		h="ASE_NotPresent";
#endif
		break;

	case ASE_HWMalfunction:
		h="ASE_HWMalfunction";
		break;

	case ASE_InvalidParameter:
		h="ASE_InvalidParameter";
		break;

	case ASE_InvalidMode:
		h="ASE_InvalidMode";
		break;

	case ASE_SPNotAdvancing:
		h="ASE_SPNotAdvancing";
		break;

	case ASE_NoClock:
		h="ASE_NoClock";
		break;

	case ASE_NoMemory:
		h="ASE_NoMemory";
		break;
	}

	if(h)
	{
		if(char *t=mainvar->GenerateString("ASIO\n",GetDeviceName(),"\n",from,"\n",h))
		{
			maingui->MessageBoxError(0,t);
			delete t;
		}
	}
}

/*
bool AudioDevice_ASIO::FillInputBuffer()
{
AudioHardwareChannel_ASIO *c=(AudioHardwareChannel_ASIO *)FirstInputChannel();

while(c)
{
if(ARES *to=c->hwinputbuffers[inbufferwritec].outputbufferARES)
{
switch(c->asiotype)
{
// these are used for 32 bit data buffer, with different alignment of the data inside
// 32 bit PCI bus systems can more easily used with these




case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
{

}
break;

case ASIOSTInt32LSB24:	// 32 bit data with 24 bit alignment
{
register ARES mul=1;
mul/=2147483648;

unsigned char b24[3];
char *fc=(char *)c->bufferinfo->buffers[record_bufferindex];

register int i=setSize;
while(i--){

b24[0]=*fc++;b24[1]=*fc++;b24[2]=*fc++;

fc++; // skip byte 4
*to++ =(ARES)((int)(b24[2] << 24 |  b24[1] << 16 |  b24[0] << 8))*mul;
}
}
break;
}

c->hwinputbuffers[inbufferwritec].channelsused=1;
}
else
c->hwinputbuffers[inbufferwritec].channelsused=0;

c=(AudioHardwareChannel_ASIO *)c->NextChannel();
}

return true;
}
*/

bool AudioDevice_ASIO::CheckAudioDevice(char *name)
{
	if(OpenAudioDevice(name)==true)
	{
		CloseDeviceDriver();
		init=false;
		return true;
	}

	CloseDeviceDriver();
	return false;
}


void AudioDevice_ASIO::InitLatencies()
{
	long inl=0,outl=0;

	ASIOError err=ASIOGetLatencies(&inl,&outl);

	//MessageASIOError("ASIOGetLatencies",err);

	if(err==ASE_NotPresent)
	{
		AudioDevice::InitLatencies(); // use buffer size default

		//	TRACE ("No ASIOGetLatencies\n");
		
		TRACE ("No ASIOGetLatencies ............... \n");
	}
	else
	{
		SetInputLatency(inl);
		SetOutputLatency(outl);
	}
}

AudioHardwareChannel_ASIO *AudioDevice_ASIO::GetAsioHWChannel(int index,int asiotype,bool inputchannel)
{
	AudioHardwareChannel_ASIO *newchannel=0;
	bool notsupported=false;
	bool nottested=false;

	switch(asiotype)
	{
	case ASIOSTInt16MSB:
		bitresolution=16;
		//	notsupported=false; // ASIO OK
		//	newchannel=new AudioHardwareChannel_ASIOSTInt16MSB;

		TRACE ("Out: ASIOSTInt16MSB \n");
		break;

	case ASIOSTInt24MSB:
		// *3
		bitresolution=24;
		TRACE ("Out: ASIOSTInt24MSB \n");
		break;

	case ASIOSTInt32MSB:
		//*4
		bitresolution=32;
		TRACE ("Out: ASIOSTInt32MSB \n");
		break;

	case ASIOSTFloat32MSB:
		//*4
		bitresolution=32;
		TRACE ("Out: ASIOSTFloat32MSB \n");
		break;

	case ASIOSTInt32MSB16:
		//*4
		bitresolution=32;
		TRACE ("Out: ASIOSTInt32MSB16 \n");
		break;

	case ASIOSTInt32MSB18:
		//*4
		bitresolution=32;
		TRACE ("Out: ASIOSTInt32MSB18 \n");
		break;

	case ASIOSTInt32MSB20:
		//*4
		bitresolution=32;
		TRACE ("Out: ASIOSTInt32MSB20 \n");
		break;

	case ASIOSTInt32MSB24:
		//*4
		bitresolution=32;
		TRACE ("Out: ASIOSTInt32MSB24 \n");
		break;

	case ASIOSTFloat64MSB:
		bitresolution=64;
		TRACE ("Out: ASIOSTFloat64MSB \n");
		break;

	case ASIOSTInt16LSB:
		// *2
		bitresolution=16;
		notsupported=false; // ASIO OK
		newchannel=new AudioHardwareChannel_ASIOSTInt16LSB;
		TRACE ("Out: ASIOSTInt16LSB Sizeof %d\n",sizeof(AudioHardwareChannel_ASIOSTInt16LSB));
		break;

	case ASIOSTInt24LSB:
		// *3
		bitresolution=24;
		notsupported=false; // ASIO OK
		newchannel=new AudioHardwareChannel_ASIOSTInt24LSB;
		TRACE ("Out: ASIOSTInt24LSB \n");
		break;

	case ASIOSTInt32LSB:
		//*4
		bitresolution=32;
		notsupported=false; // ASIO OK
		newchannel=new AudioHardwareChannel_ASIOSTInt32LSB;
		TRACE ("Out: ASIOSTInt32LSB \n");
		break;

	case ASIOSTFloat32LSB:
		//*4
		bitresolution=32;
		notsupported=false; // ASIO OK
		newchannel=new AudioHardwareChannel_ASIOSTFloat32LSB;
		TRACE ("Out: ASIOSTFloat32LSB \n");
		break;

	case ASIOSTInt32LSB16:
		//*4
		bitresolution=32;
		newchannel=new AudioHardwareChannel_ASIOSTInt32LSB16;
		notsupported=false; // ASIO OK
		nottested=true;
		TRACE ("Out: ASIOSTInt32LSB16 \n");
		break;

	case ASIOSTInt32LSB18:
		//*4
		bitresolution=32;
		newchannel=new AudioHardwareChannel_ASIOSTInt32LSB18;
		notsupported=false; // ASIO OK
		nottested=true;
		TRACE ("Out: ASIOSTInt32LSB18 \n");
		break;

	case ASIOSTInt32LSB20:
		//*4
		bitresolution=32;
		newchannel=new AudioHardwareChannel_ASIOSTInt32LSB20;
		notsupported=false; // ASIO OK
		nottested=true;
		TRACE ("Out: ASIOSTInt32LSB20 \n");
		break;

	case ASIOSTInt32LSB24:
		//*4
		bitresolution=32;
		newchannel=new AudioHardwareChannel_ASIOSTInt32LSB24;
		notsupported=false; // ASIO OK
		nottested=true;
		TRACE ("Out: ASIOSTInt32LSB24 \n");
		break;

	case ASIOSTFloat64LSB:
		//*8
		bitresolution=64;
		notsupported=false; // ASIO OK
		nottested=true;
		newchannel=new AudioHardwareChannel_ASIOSTFloat64LSB;
		TRACE ("Out: ASIOSTFloat64LSB \n");
		break;

	default:
		maingui->MessageBoxError(NULL,"Unknown ASIO Bit Resolution");
		bitresolution=0; // Error
		break;
	}

	if(notsupported==true)
	{
		if(newchannel)
			newchannel->notsupported=true;
		/*
		if(char *h=mainvar->GenerateString(Cxs[CXS_ASIOERROR],":",sampleinfo,"\n",Cxs[CXS_ASIOEMAIL]))
		{
		maingui->MessageBoxOk(NULL,h);
		delete h;
		}
		*/
	}
	else
		if(nottested==true)
		{
			/*
			char *h=mainvar->GenerateString(Cxs[CXS_ASIONOTTESTED],":",sampleinfo,"\n",Cxs[CXS_ASIOEMAIL]);

			if(h)
			{
			maingui->MessageBoxError(NULL,h);
			delete h;
			}
			*/
		}

		if(!newchannel)
			return 0;

		newchannel->device=this;
		newchannel->iotype=inputchannel==true?AUDIOCHANNEL_INPUT:AUDIOCHANNEL_OUTPUT;

		switch(asiotype)
		{
		case ASIOSTInt16LSB:	
			newchannel->sizeofsample=2;
			break;

		case ASIOSTInt32LSB:
			newchannel->sizeofsample=4;
			break;

		case ASIOSTFloat32LSB:
			newchannel->sizeofsample=4;
			break;

		case ASIOSTFloat64LSB:
			newchannel->sizeofsample=8;
			break;

		default:
			newchannel->sizeofsample=sizeof(float);
			maingui->MessageBoxError(NULL,"Unsupported ASIO Data Type found (Input)");
			break;

		}// switch

return newchannel;
}

void AudioDevice_ASIO::InitMinMaxPrefBufferSizes()
{
	ConvertSampleRateToIndex();

	// 1. Set Sampling Rate !
	ASIOError error=ASIOSetSampleRate(mainaudio->GetGlobalSampleRate()); // Set SampleRate before ASIOCreateBuffers
	MessageASIOError("ASIOSetSampleRate",error);

	long minsize_h=0,maxsize_h=0,prefsize_h=0,granularity_h=0;

	error=ASIOGetBufferSize(&minsize_h,&maxsize_h,&prefsize_h,&granularity_h);

	// WIN 8 OS problem ?
	//MessageASIOError("ASIOGetBufferSize",error);

	//if(error==ASE_OK)
	{	
		SetMinBufferSize(minsize_h);
		SetMaxBufferSize(maxsize_h);
		SetPrefBufferSize(prefsize_h);
		SetGranularity(granularity_h);
	}
}

bool AudioDevice_ASIO::OpenAudioDevice(char *name)
{
	if(!name)
		return false;

	if(strlen(name)>31)
		return false; // Max 31+0 in ASIODriverInfo

	if(init==false || (!theAsioDriver)) // avoid double open
	{
		bool ok=false;

#ifdef TRYCATCH
		try
#endif
		{
			ok=loadAsioDriver(name);
		}

#ifdef TRYCATCH
		catch(...)
		{
			return false; // Crash loadAsioDriver !!!
		}
#endif

		//ok=false;

		if(ok==true)
		{
			TRACE ("ASIO Driver %s\n",name);

			ASIODriverInfo info;

			info.asioVersion=2;
			strcpy(info.name,name);
			info.driverVersion=0;

			// (Windows: application main window handle, Mac & SGI: 0)

#ifdef WIN32
			info.sysRef=maingui->FirstScreen()?maingui->FirstScreen()->hWnd:0;
#else
			info.sysRef=0;
#endif

			ASIOError error=ASE_NotPresent;

			#ifdef TRYCATCH
			try
#endif
			{
				error=ASIOInit(&info);
			}

			#ifdef TRYCATCH
			catch(...)
			{
				return false; // Crash ASIO Init !!!
			}
#endif

			MessageASIOError("ASIOInit",error);

			if(error==ASE_NotPresent)
			{
				//maingui->MessageBoxError(0,"Asio Hardware NotPresent");
			}

			if(error==ASE_OK || error==ASE_SUCCESS)
			{
				//long i;
				//	AudioHardwareChannel_ASIO *newchannel=0;

				//	ASIOSampleRate rate;

				//	ASIOControlPanel();
				//	error=ASIOGetSampleRate(&rate);

				//	MessageASIOError("ASIOGetSampleRate",error);

				long inh=0,outh=0;

				error=ASIOGetChannels(&inh,&outh);

				in_channels=inh;
				out_channels=outh;

				MessageASIOError("ASIOGetChannels",error);

				if(error!=ASE_OK && error!=ASE_SUCCESS)
					return false;

				//InitMinMaxPrefBufferSizes();

				// Bit Resolution 1. Output Channel

				if(!FirstInputChannel())
				{
					for(int i=0;i<in_channels;i++)
					{
						ASIOChannelInfo asio_cinfo; // Audio Hardware Info		

						asio_cinfo.channel=i;
						asio_cinfo.isInput=ASIOTrue;

						error=ASIOGetChannelInfo(&asio_cinfo);
						MessageASIOError("ASIOGetChannelInfo In",error);

						TRACE ("ASIO Input Type %s %d\n",asio_cinfo.name,asio_cinfo.type);

						if(AudioHardwareChannel_ASIO *inchl=GetAsioHWChannel(i,asio_cinfo.type,true))
						{
							inchl->audiochannelgroup=asio_cinfo.channelGroup;

							inchl->SetName(asio_cinfo.name);
							AddInputHWChannel(inchl,i);
						}
					}
				}

				if(!FirstOutputChannel())
				{
					for(int i=0;i<out_channels;i++)
					{
						ASIOChannelInfo asio_cinfo; // Audio Hardware Info		

						asio_cinfo.channel=i;
						asio_cinfo.isInput=ASIOFalse;

						error=ASIOGetChannelInfo(&asio_cinfo);
						MessageASIOError("ASIOGetChannelInfo Out",error);

						TRACE ("ASIO Output Type %s %d\n",asio_cinfo.name,asio_cinfo.type);

						if(AudioHardwareChannel_ASIO *outchl=GetAsioHWChannel(i,asio_cinfo.type,false))
						{
							outchl->audiochannelgroup=asio_cinfo.channelGroup;

							outchl->SetName(asio_cinfo.name);
							AddOutputHWChannel(outchl,i);
						}
					}
				}

				//
				if(error!=ASE_OK && error!=ASE_SUCCESS)
				{
					return false;
				}

				init=true; // Device ok


				//	byteresolution=bitresolution/8;

				//if(bitresolution)
				//{

				// 
				//dbdynamic=6.0206;
				//dbdynamic*=bitresolution;

				//GetAudioDeviceBufferSize();

				//}

				asiouseoutready=ASIOOutputReady() == ASE_OK?true:false;

				if(asiouseoutready==true)
					strcpy(info1,"ASIOOutputReady");

				/*
				int samplerates[]=
				{	
					44100,
					48000,
					88200,
					96000,

					176400,
					192000,
					352800,
					384000
				};
*/

				// Sample Rate Check
				ActivateSampleRate(ADSR_44,ASIOCanSampleRate(44100)!=ASE_NoClock?true:false);
				ActivateSampleRate(ADSR_48,ASIOCanSampleRate(48000)!=ASE_NoClock?true:false);
			//	ActivateSampleRate(ADSR_56,ASIOCanSampleRate(56000)!=ASE_NoClock?true:false);
			//	ActivateSampleRate(ADSR_64,ASIOCanSampleRate(64000)!=ASE_NoClock?true:false);
				ActivateSampleRate(ADSR_88,ASIOCanSampleRate(88200)!=ASE_NoClock?true:false);
				ActivateSampleRate(ADSR_96,ASIOCanSampleRate(96000)!=ASE_NoClock?true:false);

				ActivateSampleRate(ADSR_176,ASIOCanSampleRate(176400)!=ASE_NoClock?true:false);
				ActivateSampleRate(ADSR_192,ASIOCanSampleRate(192000)!=ASE_NoClock?true:false);
				ActivateSampleRate(ADSR_352,ASIOCanSampleRate(352800)!=ASE_NoClock?true:false);
				ActivateSampleRate(ADSR_384,ASIOCanSampleRate(384000)!=ASE_NoClock?true:false);

				// ASIOControlPanel();

			} // bitresolution >=16bit

			//ASIOExit();
		}
		else
		{
			if(mainaudio->collectmode==false && (!theAsioDriver))
			{
				if(char *h=mainvar->GenerateString("ASIO driver [ASIOInit]","\n",name))
				{
					maingui->MessageBoxError(NULL,h);
					delete h;
				}

				return false;
			}
		}

	}// load driver
	else
	{
		if(!theAsioDriver)
		{
			maingui->MessageBoxError(NULL,"Load ASIO Driver");
			return false;
		}
	}

	return init;
}

bool AudioDevice_ASIO::InitAudioDeviceChannels()
{
	/*ASIOError error=*/

	ASIODisposeBuffers();

	//MessageASIOError("ASIODisposeBuffers",error);

	if(bufferarray)
	{
		delete bufferarray;
		bufferarray=0;
	}

	bufferarrayplaybackchannels=GetCountOfOutputChannels();
	bufferarrayrecordchannels=GetCountOfInputChannels();
	bufferarraychannels=bufferarrayplaybackchannels+bufferarrayrecordchannels;

	if(bufferarraychannels) // Sum I/O Channels
		bufferarray=new ASIOBufferInfo[bufferarraychannels];

	devicepRepared=false;

	if(ASIOBufferInfo *initarray=bufferarray){

		for(int i=0;i<bufferarraychannels;i++) // Reset Buffer
			memset(&bufferarray[i],0,sizeof(ASIOBufferInfo));

		// Init Input Channels -------------------------------------------------
		AudioHardwareChannel_ASIO *in=(AudioHardwareChannel_ASIO *)FirstInputChannel();
		while(in)
		{	
			initarray=in->AddBufferToASIOBuffer(initarray);
			in=(AudioHardwareChannel_ASIO *)in->NextChannel();
		}

		// Init Output Channels -------------------------------------------------
		AudioHardwareChannel_ASIO *out=(AudioHardwareChannel_ASIO *)FirstOutputChannel();
		while(out)
		{		
			initarray=out->AddBufferToASIOBuffer(initarray);
			out=(AudioHardwareChannel_ASIO *)out->NextChannel();
		}

		if(bufferarray)
		{		
			Reset(RESETAUDIODEVICE_FORCE);

			InitMinMaxPrefBufferSizes(); // +Set Sample Rate

			asioCallbacks_1.bufferSwitch = &bufferSwitch_1;
			asioCallbacks_1.sampleRateDidChange = &sampleRateChanged_1;
			asioCallbacks_1.asioMessage = &asioMessages_1;
			asioCallbacks_1.bufferSwitchTimeInfo = &bufferSwitchTimeInfo_1;	

			ASIOError error=ASIOCreateBuffers(bufferarray,bufferarraychannels,GetSetSize(),&asioCallbacks_1); // + setBufferSize
			MessageASIOError("ASIOCreateBuffers %d",error);

			TRACE ("CreateBuffers with %d SetSize \n",GetSetSize());

#ifdef _DEBUG
			for(int i=0;i<bufferarraychannels;i++)
				TRACE ("ASIO Buffer Input %d Chl Num %d B1 %d B2 %d\n",bufferarray[i].isInput,bufferarray[i].channelNum,bufferarray[i].buffers[0],bufferarray[i].buffers[1]);
			//ASIOBool isInput;			// on input:  ASIOTrue: input, else output
			//long channelNum;			// on input:  channel index
			//void *buffers[2];			// on output: double buffer addresses
#endif

			if(error==ASE_OK || error==ASE_SUCCESS) // Buffer ok
			{
				InitLatencies();

				devicepRepared=true;

				/*
				// perform the processing
				for (int i = 0; i < bufferarraychannels; i++)
				{
				if (bufferarray[i].isInput == false)
				{
				// OK do processing for the outputs only
				switch (asioDriverInfo.channelInfos[i].type)
				{
				case ASIOSTInt16LSB:
				memset (bufferarray.buffers[index], 0, buffSize * 2);
				break;
				case ASIOSTInt24LSB:		// used for 20 bits as well
				memset (bufferarray[i].buffers[index], 0, buffSize * 3);
				break;
				case ASIOSTInt32LSB:
				memset (bufferarray[i].buffers[index], 0, buffSize * 4);
				break;
				case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				memset (bufferarray[i].buffers[index], 0, buffSize * 4);
				break;
				case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				memset (bufferarray[i].buffers[index], 0, buffSize * 8);
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
				case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
				memset (bufferarray[i].buffers[index], 0, buffSize * 4);
				break;

				case ASIOSTInt16MSB:
				memset (bufferarray[i].buffers[index], 0, buffSize * 2);
				break;
				case ASIOSTInt24MSB:		// used for 20 bits as well
				memset (bufferarray[i].buffers[index], 0, buffSize * 3);
				break;
				case ASIOSTInt32MSB:
				memset (bufferarray[i].buffers[index], 0, buffSize * 4);
				break;
				case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				memset (bufferarray[i].buffers[index], 0, buffSize * 4);
				break;
				case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				memset (bufferarray[i].buffers[index], 0, buffSize * 8);
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
				case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
				memset (bufferarray[i].buffers[index], 0, buffSize * 4);
				break;
				}
				}
				}

				*/
			}


			//return ok;
		}

	}

	if(devicepRepared==false)
	{
		CleanMemory();
		return false;
	}

	return true;
}

int AudioDevice_ASIO::GetSamplePosition()
{
	if(devicestarted==true)
	{
#ifdef WIN32
		ASIOSamples samples;
		ASIOTimeStamp tStamp;

		ASIOGetSamplePosition(&samples,&tStamp);

		return (samples.lo);
#endif
	}

	return 0;
}

void AudioDevice_ASIO::StartAudioHardware()
{
	if(theAsioDriver && devicestarted==false && numberhardwarebuffer>0)
	{
		ASIOError error=ASIOStart();
		MessageASIOError("ASIOStart",error);

		if(error==ASE_OK || error==ASE_SUCCESS)
		{
			asioaudiodevice=this;
			devicestarted=true;

			status |= AudioDevice::STATUS_OUTPUTOK;
			status |= AudioDevice::STATUS_INPUTOK;
		}
		else
		{
			status CLEARBIT AudioDevice::STATUS_OUTPUTOK;
			status CLEARBIT AudioDevice::STATUS_INPUTOK;
		}
	}
}

void AudioDevice_ASIO::Reset(int iflag)
{
	OpenAudioDevice(initname);


	if(!(iflag&RESETAUDIODEVICE_FORCE))
	{
		/*
		if(GetMinBufferSize()>0 && GetMinBufferSize()>GetSetSize())
		{
		if(maingui->MessageBoxYesNo(0,Cxs[CXS_ASIOMINSIZEQUESTION])==false)
		SetBufferSize(GetMinBufferSize());
		}

		if(GetMaxBufferSize()>0 && GetMaxBufferSize()<GetSetSize())
		{
		if(maingui->MessageBoxYesNo(0,Cxs[CXS_ASIOMAXSIZEQUESTION])==false)
		SetBufferSize(GetMaxBufferSize());
		}
		*/

		SetBufferSize(GetSetSize());
		CheckSetSize();
	}
}

void AudioDevice_ASIO::ASIOCall(long index)
{
	play_bufferindex=record_bufferindex=index;

	if(Seq_Song *song=mainvar->GetActiveSong())
	{
		ConvertInputDataToSongInputARES(song);
		song->RefillAudioDeviceBuffer(this);
		song->SyncAudioRecording(this);
	}
	else
		SkipDeviceOutputBuffer(0);

#ifdef WIN32
	if(asiouseoutready==true)// send return
		ASIOOutputReady();
#endif
}

void AudioDevice_ASIO::CloseDeviceDriver()
{
	if(theAsioDriver)
	{
		StopAudioHardware();

		if(devicepRepared==true && bufferarray)
		{
			/*ASIOError error=*/ASIODisposeBuffers();
			//MessageASIOError("CDD ASIODisposeBuffers",error);
		}

		ASIOError error=ASIOExit(); //+ remove Driver

		MessageASIOError("ASIOExit",error);
	}

	if(asioDrivers)
	{
		delete asioDrivers;
		asioDrivers=0;
	}

	if(bufferarray)
	{	
		//TRACE ("Delete BufferArray %d\n",sizeof(ASIOBufferInfo));
		delete bufferarray;
		bufferarray=0;
	}

	devicepRepared=false;
}

void AudioDevice_ASIO::StopAudioHardware()
{
	if(devicestarted==true)
	{
		devicestarted=false; 
		ASIOError error=ASIOStop();
		MessageASIOError("ASIOStop",error);
	}

	asioaudiodevice=0;
}

ASIOBufferInfo *AudioHardwareChannel_ASIO::AddBufferToASIOBuffer(ASIOBufferInfo *buffer)
{
	bufferinfo=buffer;
	buffer->channelNum=channelindex;
	// Record Channel
	buffer->isInput=iotype==AUDIOCHANNEL_INPUT?ASIOTrue:ASIOFalse;
	buffer++;
	return buffer;
}
