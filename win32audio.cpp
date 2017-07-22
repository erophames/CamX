#include "defines.h"
#include <stdio.h>
#include <string.h>

#include "object_song.h"
#include "win32_device.h"
#include "gui.h"
#include "audiorecord.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "semapores.h"
#include "songmain.h"
#include "audiohdfile.h"
#include "audiothreads.h"
#include "audiohardwarechannel.h"

#define H16_DIVMUL (1.0f/32768)
#define H16_DIVMUL_P (1.0f/32767)

AudioDevice_WIN32::AudioDevice_WIN32()
{
	audiosystemtype=AUDIOCAMX_WIN32;

	userawdata=true;

	for(int i=ADSR_44;i<ADSR_LAST;i++)
	{
		buffersettings[i].minSize=1024;
		buffersettings[i].prefSize=1024;
		buffersettings[i].maxSize=8*1024;
	}

	waveinopen=false;
	sync=false;
	allcleared[0]=allcleared[1]=false;

	hWaveOut=0;
	hWaveIn=0;
}

bool AudioDevice_WIN32::CheckAudioDevice(char *name)
{
	if(OpenAudioDevice(0)==true)
	{
		return true;
	}
	return false;
}

bool AudioDevice_WIN32::InitAudioDeviceChannels()
{
	InitLatencies();

	return init;
}

bool AudioDevice_WIN32::OpenAudioDevice(char *name)
{
	if(init==false) // avoid double open
	{
#ifdef WIN32
		bitresolution=16;		
		//	byteresolution=bitresolution/8;

		out_channels=2;
		in_channels=2;

		//latency_input_samples=100000;

		if(!FirstInputChannel())
		{
			// Input -----------------
			if(AudioHardwareChannel_WIN32Int16LSB *in=new AudioHardwareChannel_WIN32Int16LSB)
			{
				in->device=this;
				in->iotype=AUDIOCHANNEL_INPUT;
				//in->numberofsamples=prefsize; // default
				in->audiochannelgroup=0;
				in->channelindex=0;
				in->sizeofsample=sizeof(short);

				in->SetName("In_L_Mapper");

				AddInputHWChannel(in,0);
			}			

			if(AudioHardwareChannel_WIN32Int16LSB *in=new AudioHardwareChannel_WIN32Int16LSB)
			{
				in->device=this;
				in->iotype=AUDIOCHANNEL_INPUT;
				//in->numberofsamples=prefsize; // default
				in->audiochannelgroup=0;
				in->sizeofsample=sizeof(short);

				in->SetName("In_R_Mapper");

				in->channelindex=1;
				AddInputHWChannel(in,1);
			}	

			// Output -----------------
			if(AudioHardwareChannel_WIN32Int16LSB *out=new AudioHardwareChannel_WIN32Int16LSB)
			{
				out->device=this;
				out->iotype=AUDIOCHANNEL_OUTPUT;
				//out->numberofsamples=prefsize; // default
				out->audiochannelgroup=0;
				out->sizeofsample=sizeof(short);

				out->SetName("L_Mapper");

				AddOutputHWChannel(out,0);
			}

			if(AudioHardwareChannel_WIN32Int16LSB *out=new AudioHardwareChannel_WIN32Int16LSB)
			{
				out->device=this;
				out->iotype=AUDIOCHANNEL_OUTPUT;
				//out->numberofsamples=prefsize; // default
				out->audiochannelgroup=0;
				out->sizeofsample=sizeof(short);

				out->SetName("R_Mapper");

				AddOutputHWChannel(out,1);
			}	
		}

		init=true;
#endif
	}

	return init;
}

int AudioDevice_WIN32::GetSamplePosition()
{
	return 0;
}

void AudioDevice_WIN32::StartAudioHardware()
{
	if(devicepRepared==false)
	{
	}

	if(devicepRepared==true && devicestarted==false && numberhardwarebuffer>0)
	{
		WAVEFORMATEX wfx;

		wfx.nSamplesPerSec = mainaudio->GetGlobalSampleRate(); /* sample rate */
		wfx.wBitsPerSample = 16; /* sample size */
		wfx.nChannels= 2; /* channels*/
		wfx.cbSize = 0; /* size of _extra_ info */
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
		wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

		ZeroMemory(&header[0], sizeof(WAVEHDR));
		ZeroMemory(&header[1], sizeof(WAVEHDR));
		ZeroMemory(&headerin[0], sizeof(WAVEHDR));
		ZeroMemory(&headerin[1], sizeof(WAVEHDR));

		stopdevice=true;

		MMRESULT mres=MMSYSERR_NOERROR;

		// ! Win7 Input may be disabled

		if(inbuffer_raw &&

			(mres=waveInOpen(
			&hWaveIn, 
			WAVE_MAPPER, 
			&wfx, 
			(DWORD_PTR)maindeviceinputthread->hWait, 
			(DWORD_PTR)0,
			CALLBACK_EVENT|WAVE_FORMAT_DIRECT
			)) == MMSYSERR_NOERROR)
		{
			// DB1
			headerin[0].dwBufferLength = GetSetSize()*2*sizeof(short);
			headerin[0].lpData = (LPSTR)inbuffer_raw;
			headerin[0].dwFlags=WHDR_BEGINLOOP;

			MMRESULT res1=waveInPrepareHeader(hWaveIn, &headerin[0], sizeof(WAVEHDR));

			maingui->MessageMMError(GetDeviceName(),"waveInPrepareHeader1",res1);

			// DB 2
			headerin[1].dwBufferLength = headerin[0].dwBufferLength;
			headerin[1].lpData =(LPSTR)inbuffer_raw+headerin[0].dwBufferLength;
			headerin[1].dwFlags=WHDR_ENDLOOP;
			MMRESULT res2=waveInPrepareHeader(hWaveIn, &headerin[1], sizeof(WAVEHDR));

			maingui->MessageMMError(GetDeviceName(),"waveInPrepareHeader2",res1);
			waveinopen=true;
			status|=AudioDevice::STATUS_INPUTOK;
		}
		else
		{
			//	maingui->MessageMMError("waveInOpen",mres); Sometimes no Audio In under Win7
			waveinopen=false;
			status CLEARBIT AudioDevice::STATUS_INPUTOK;
		}

		//maingui->MessageMMError("waveInOpen\nAudio Device WAVE_MAPPER INPUT\n Windows Driver Problem ?\nReboot PC ?...",mres);
		//if(!hWaveIn)return;

		skipoutbuffer=1;

		mres=MMSYSERR_NOERROR;

		if(
			mixbuffer_out &&
			(

#ifdef WIN32_CALLBACK
			waveOutOpen(
			&hWaveOut, 
			WAVE_MAPPER, 
			&wfx, 
			(DWORD_PTR)waveOutProc_Callback, 
			(DWORD_PTR)this,
			CALLBACK_FUNCTION
			) == MMSYSERR_NOERROR
#else
			(mres=waveOutOpen(
			&hWaveOut, 
			WAVE_MAPPER, 
			&wfx, 
			(DWORD_PTR)maindevicerefillthread->hWait, 
			(DWORD_PTR)0,
			CALLBACK_EVENT|WAVE_FORMAT_DIRECT
			))==MMSYSERR_NOERROR
#endif
			)
			)
		{
			// Init Wave Header
			//waitingoutbuffer=2;

			// DB1
			header[0].dwBufferLength = header[1].dwBufferLength = GetSetSize()*2*sizeof(short); // 2=Stereo
			header[0].lpData = header[1].lpData = (LPSTR)mixbuffer_out;
			header[0].dwFlags=WHDR_BEGINLOOP;

			MMRESULT res1=waveOutPrepareHeader(hWaveOut, &header[0], sizeof(WAVEHDR));
			maingui->MessageMMError(GetDeviceName(),"waveOutPrepareHeader1",res1);

			// DB 2
			// header[0].dwBufferLength;
			header[1].lpData +=header[0].dwBufferLength;
			header[1].dwFlags=WHDR_ENDLOOP;
			MMRESULT res2=waveOutPrepareHeader(hWaveOut, &header[1], sizeof(WAVEHDR));

			maingui->MessageMMError(GetDeviceName(),"waveOutPrepareHeader2",res2);

			if(res1==MMSYSERR_NOERROR && res2==MMSYSERR_NOERROR)
			{
				status|=AudioDevice::STATUS_OUTPUTOK;

				// inbufferreadc=/*inbufferwritec=*/0;

				stopdevice=false;

				if(waveinopen==true)
				{
					res1=waveInAddBuffer(hWaveIn, &headerin[0], sizeof(WAVEHDR)); // PRepair Input 1
					maingui->MessageMMError(GetDeviceName(),"waveInAddBuffer1",res1);
					res2=waveInAddBuffer(hWaveIn, &headerin[1], sizeof(WAVEHDR)); // PRepair Input 2
					maingui->MessageMMError(GetDeviceName(),"waveInAddBuffer2",res2);
				}

				devicestarted=true;

				//mainthreadcontrol->Lock(CS_audioplayback);

				res1=waveOutWrite(hWaveOut, &header[0], sizeof(WAVEHDR)); // Start Out 1
				maingui->MessageMMError(GetDeviceName(),"hWaveOut1",res1);
				res2=waveOutWrite(hWaveOut, &header[1], sizeof(WAVEHDR)); // Start Out 2
				maingui->MessageMMError(GetDeviceName(),"hWaveOut2",res2);

				//mainthreadcontrol->Unlock(CS_audioplayback);;

				if(waveinopen==true)
				{
					MMRESULT res=waveInStart(hWaveIn);
					maingui->MessageMMError(GetDeviceName(),"waveInStart",res);
				}
			}
			else
				status CLEARBIT AudioDevice::STATUS_OUTPUTOK;
		}
		else
		{
			maingui->MessageMMError(GetDeviceName(),"waveOutOpen\nWindows Driver Problem",mres);
			status CLEARBIT AudioDevice::STATUS_OUTPUTOK;
		}
	}
}

void AudioDevice_WIN32::CloseDeviceDriver()
{
	if(devicepRepared==true)
	{
		devicepRepared=false;
		TRACE ("AudioDevice_WIN32 CloseDriver\n");
		if(hWaveOut)waveOutClose(hWaveOut);
		hWaveOut=0;

		if(hWaveIn && waveinopen==true)waveInClose(hWaveIn);
		hWaveIn=0;
	}
}

void AudioDevice_WIN32::InitMinMaxPrefBufferSizes()
{
	SetMinBufferSize(256);
	SetMaxBufferSize(8192);
	SetPrefBufferSize(1024);
}

void AudioDevice_WIN32::StopAudioHardware()
{
	if(devicepRepared==true && devicestarted==true){

		//devicebuffersend=deviceinbuffersend=false;

		devicestarted=false; // signal to fill last audiobuffer with 0

		TRACE ("Stop WaveOut \n");
		if(hWaveOut){

			MMRESULT res=waveOutReset(hWaveOut);
			maingui->MessageMMError(GetDeviceName(),"waveOutReset",res);

			for(int i=0;i<2;i++)
			{
				do{
					res=waveOutUnprepareHeader(hWaveOut,&header[i],sizeof(WAVEHDR));

					if(res==WAVERR_STILLPLAYING)
						Sleep(10);


				}while(res==WAVERR_STILLPLAYING);
			}

#ifdef WIN32_CALLBACK
			while(devicebuffersend==false)
			{
				TRACE ("Stop WaveIn Loop %d\n",this->waitinginbuffer);
				Sleep(10);
			}
#endif

			//
			res=waveOutClose(hWaveOut);
			maingui->MessageMMError(GetDeviceName(),"waveOutClose",res);
			hWaveOut=0;
		}
		TRACE ("WaveOut Close \n");

		TRACE ("Stop WaveIn \n");
		if(hWaveIn && waveinopen==true){

			//	TRACE ("Stop WaveIn Reset\n");
			MMRESULT res=waveInReset(hWaveIn);
			maingui->MessageMMError(GetDeviceName(),"waveInReset",res);

			res=waveInStop(hWaveIn);
			maingui->MessageMMError(GetDeviceName(),"waveInStop",res);

			for(int i=0;i<2;i++)
			{
				MMRESULT res;

				do{
					res=waveInUnprepareHeader(hWaveIn,&headerin[i],sizeof(WAVEHDR));

					if(res==WAVERR_STILLPLAYING)
						Sleep(10);

				}while(res==WAVERR_STILLPLAYING);
			}

			TRACE ("waveInClose \n");
			res=waveInClose(hWaveIn);
			maingui->MessageMMError(GetDeviceName(),"waveInClose",res);
			hWaveIn=0;
		}
		//devicepRepared=false;
	}
}

bool AudioDevice_WIN32::CheckOutRefill()
{
	return header[play_bufferindex].dwFlags&WHDR_DONE?true:false;
}


void AudioDevice_WIN32::FillInputBufferEnd()
{
	waveInAddBuffer(hWaveIn, &headerin[record_bufferindex], sizeof(WAVEHDR));
}

void AudioDevice_WIN32::CreateDeviceStreamMix(Seq_Song *song)
{
	// Create L/R Mix
	if(song && song->masteringlock==true)
	{
		short *to=(short *)mixbuffer_out;

		to+=play_bufferindex*out_channels*GetSetSize();

		// Set All to 0
		memset(to,0,sizeof(short)*out_channels*GetSetSize());

		for(int i=0;i<out_channels;i++)
		allcleared[i]=false;

		return;
	}
	
	AudioHardwareChannel *hw=FirstOutputChannel();

	bool left=(hw && hw->hardwarechannelused==true)?true:false;
	hw=hw->NextChannel();
	bool right=(hw && hw->hardwarechannelused==true)?true:false;

	short *from_l=(short *)buffer_out;
	from_l+=play_bufferindex*2*GetSetSize();

	short *to=(short *)mixbuffer_out;
	to+=play_bufferindex*2*GetSetSize();

	int i=GetSetSize();

	//TRACE ("CreateDeviceStreamMix %d\n",index);
	//mixbuffer_out_bytespersamples=2; // 16 Bit

	if(left==true && right==true)
	{
		// Mix Left/Right > L/R
		short *from_r=from_l;
		from_r+=GetSetSize();

		while(i--){
			*to++=*from_l++;
			*to++=*from_r++;
		}

		allcleared[play_bufferindex]=false;
		return;
	}

	if(left==true && right==false)
	{
		while(i--){
			*to++=*from_l++; // L0
			*to++=0;
		}

		allcleared[play_bufferindex]=false;
		return;
	}

	if(left==false && right==true)
	{
		short *from_r=from_l;
		from_r+=GetSetSize();

		while(i--){ // 0R
			*to++=0;
			*to++=*from_r++;
		}

		allcleared[play_bufferindex]=false;
		return;
	}

	if(allcleared[play_bufferindex]==false)
	{
		allcleared[play_bufferindex]=true;
		memset(to,0,sizeof(short)*2*GetSetSize());
	}
}

void AudioHardwareChannel_WIN32Int16LSB::ClearChannel(int samples,int index)
{
	/*
	short *to=(short *)device->buffer_out;

	to+=samples*device->buffer_out_channels*index;
	to+=samples*channelindex;

	memset(to,0,samples*sizeof(short));
	*/
}

void AudioHardwareChannel_WIN32Int16LSB::CopyFromHardware()
{
	ARES *to=hwinputbuffer.outputbufferARES,m_peak=0,p_peak=0;
	short *from=(short *)device->inbuffer_raw;

	from +=device->in_channels*device->GetSetSize()*device->record_bufferindex;
	from+=channelindex; // Channel L/R

	int i=device->GetSetSize(); // L/R -> [L][R]

	//register ARES mul=H16_DIVMUL;

	if(device->in_channels==1)
	{
		if(int loop=i/8)
		{
			i-=loop*8;

			do{
				ARES h;
				
				if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

				if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

				if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

				if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}


				if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

				if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

				if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

				if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}

			}while(--loop);
		}

		while(i--)
		{
			ARES h;
			
			if(*from>0){h=*to++ =(ARES)*from++ * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
				else {h=*to++ =(ARES)*from++ * H16_DIVMUL;if(h<m_peak)m_peak=h;}
		}	

	}
	else
		if(device->in_channels==2)
		{
			if(int loop=i/8)
			{
				i-=loop*8;

				do{
					ARES h;
				
					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;
					
					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;


					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;

				}while(--loop);
			}

			while(i--){
				ARES h;
				
				if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=2;
			}
		}
		else
		{
			int chls=device->in_channels;

			if(int loop=i/8)
			{
				i-=loop*8;

				do{
					ARES h;
					
					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;


					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;

					if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;

				}while(--loop);
			}

			while(i--){
				ARES h;
				
				if(*from>0){h=*to++ =(ARES)*from * H16_DIVMUL_P;if(h>p_peak)p_peak=h;}
					else{h=*to++ =(ARES)*from * H16_DIVMUL;if(h<m_peak)m_peak=h;}
					from+=chls;
			}
		}

		ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;
		currentpeak=peak;
		if(peak>peakmax)
			peakmax=peak;
}

ARES *AudioHardwareChannel_WIN32Int16LSB::WriteToHardwareBufferData(ARES *from,int samples,int index)
{
	if(short *to=(short *)device->buffer_out)
	{
		to+=samples*device->buffer_out_channels*index;
		to+=samples*channelindex;

		ARES max_p=0,max_m=0;

		while(samples--)
		{
			// Left
			register ARES h=*from++;

			if(h>0)
			{
				if(h>max_p)max_p=h;

				if(h>=1)*to++=SHRT_MAX;
				else
#ifdef ARES64
					*to++=(short)floor((h*SHRT_MAX)+0.5);
#else
					*to++=(short)floor((h*SHRT_MAX)+0.5f);
#endif
			}
			else
			{	
				if(h<max_m)max_m=h;

#ifdef DEBUG
				/*
				ARES h2=floor(-32767.7+0.5);
				ARES h3=floor(-32767.3+0.5);
				short t=-32767.7;
				ARES hh=-32767.7+0.5;
				ARES th=floor(hh);
				*/
#endif

				if(h<=-1)
					*to++=SHRT_MIN;
				else
#ifdef ARES64
					*to++=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
					*to++=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
			}
		}

		ARES max=-max_m>max_p?-max_m:max_p;
		currentpeak=max;
		if(max>peakmax)peakmax=max;
	}

	return from;
}

#ifdef WIN32_CALLBACK
// Win Audio DB Refill
static void CALLBACK waveOutProc_Callback(
	HWAVEOUT hwo,
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
	)
{
	AudioDevice_WIN32 *awdevice=(AudioDevice_WIN32 *)dwInstance;

	switch(uMsg)
	{
	case WOM_OPEN:
		{

		}
		break;

	case WOM_CLOSE:
		{
			//TRACE ("WOM Close\n");
		}
		break;

	case WOM_DONE:
		{
			sema_audioout.Lock();

			if(awdevice->waitingoutbuffer)
			{
				if(awdevice->stopdevice==true){

					awdevice->waitingoutbuffer--;

					if(!awdevice->waitingoutbuffer)
						awdevice->devicebuffersend=true;
				}
				else{
					mainthreadcontrol->Lock(CS_audioplayback);

					if(Seq_Song *song=mainvar->GetActiveSong())
						song->RefillAudioDeviceBuffer(awdevice);

					mainthreadcontrol->Unlock(CS_audioplayback);;
				}
			}
			else
			{
				if(awdevice->stopdevice==true)
					awdevice->devicebuffersend=true;
			}

			sema_audioout.Unlock();
		}
		break;
	}
}

void CALLBACK waveInProc_Callback(
								  HWAVEIN hwi,
								  UINT uMsg,
								  DWORD_PTR dwInstance,
								  DWORD_PTR dwParam1,
								  DWORD_PTR dwParam2
								  )
{
	AudioDevice_WIN32 *awdevice=(AudioDevice_WIN32 *)dwInstance;

	switch(uMsg)
	{
	case WIM_OPEN:
		//TRACE ("WIM Open\n");
		break;

	case WIM_CLOSE:
		//TRACE ("WIM Close\n");
		break;

	case WIM_DATA:
		{

			// WARNING : Dead Locks with mainthreadcontrol->Locks !!!

			//TRACE ("WIM Data %d %d\n",awdevice->headerin[awdevice->inbufferwritec].dwBytesRecorded,awdevice->inputbuffercounter);

			//sema_audioin.Lock();

			if(awdevice->waitinginbuffer)
			{
				if(awdevice->stopdevice==true){
					awdevice->waitinginbuffer--;

					if(!awdevice->waitinginbuffer)
						awdevice->deviceinbuffersend=true;
				}
				else{
					if(awdevice->headerin[awdevice->inbufferwritec].dwBytesRecorded==awdevice->setSize*awdevice->in_channels*sizeof(short))
					{
						awdevice->headerin[awdevice->inbufferwritec].dwBytesRecorded=0; // Fill again
						awdevice->ConvertInputToARES(awdevice->inbufferwritec);

						audioinputthread.SetSignal();
						/*
						awdevice->Lock_AudioRecord();

						//audiorecordthread->Lock();
						Seq_Song *song=mainvar->GetActiveSong();

						if( song &&
						(!(song->status&Seq_Song::STATUS_WAITPREMETRO)) && 
						mainaudio->FirstAudioRecordingFile())
						{
						awdevice->SetRecordTime(song,SETRECORDFLAG_ADD);
						awdevice->Unlock_AudioRecord();

						awdevice->RecordInputAudioBufferToSong(awdevice->inbufferwritec); // Record ?
						}
						else
						awdevice->Unlock_AudioRecord();
						*/
					}

					waveInAddBuffer(hwi, &awdevice->headerin[awdevice->inbufferwritec], sizeof(WAVEHDR));
					awdevice->inbufferwritec^=1;
				}
			}
			else
			{
				if(awdevice->stopdevice==true)
					awdevice->deviceinbuffersend=true;
			}
		}
		break;
	}
}
#endif

/*
//#define WIN32_CALLBACK 1 // !!! Callback deadlock problems
void AudioDevice_WIN32::ConvertInputToARES(int buffer)
{
if(inbuffer_raw)
{
AudioHardwareChannel *c=FirstInputChannel();

while(c)
{
if(ARES *to=c->inputbuffers[buffer].outputbufferARES)
{
ARES h;

short *from=(short *)inbuffer_raw;
from +=in_channels*setSize*record_bufferindex;
from+=c->channelindex; // Channel L/R

long i=setSize; // L/R -> [L][R]

if(in_channels==1)
{
while(i--){
h=*from++;
*to++ = h*H16_DIVMUL;
}
}
else
if(in_channels==2)
{
while(i--){
h=*from;
from+=2;
*to++ = h*H16_DIVMUL;
}
}
else
while(i--){
h=*from;
from+=in_channels;
*to++ = h*H16_DIVMUL;
}

c->inputbuffers[buffer].channelsused=1;
}

c=c->NextChannel();
}
}
}
*/