#include "defines.h"
#include <stdio.h>
#include <string.h>

#include "object_song.h"
#include "object_project.h"

#include "win32_device.h"
#include "gui.h"
#include "semapores.h"
#include "audiohardware.h"
#include "audiorealtime.h"
#include "audiomaster.h"
#include "audiohdfile.h"
#include "MIDIPattern.h"
#include "songmain.h"
#include "audiorecord.h"
#include "MIDItimer.h"
#include "MIDIprocessor.h"
#include "audiothreads.h"
#include "chunks.h"
#include "audioproc.h"
#include "audiohardwarechannel.h"

#include <math.h>

void AudioDevice_WIN32::SkipDeviceOutputBuffer(Seq_Song *song)
{
	ClearOutputBuffer(song);
}

void AudioDeviceDeviceOutThread::WaitLoop() 
{
	Seq_Song *song;
	AudioDevice_WIN32 *device;

	while(IsExit()==false)
	{
		WaitSignal();

		if(IsExit()==true)
			break;

		mainthreadcontrol->Lock(CS_audioplayback);

		if(device=(AudioDevice_WIN32 *)mainaudio->GetActiveDevice())
		{
			//	TRACE ("Inc %d Outc %d\n",device->incounter,device->outcounter);

			if(device->stopdevice==false && device->devicestarted==true)
			{	
				do
				{
					//device->outcounter++;

					//if(device->outcounter>2)
					//{
					if(device->skipoutbuffer)
					{
						device->skipoutbuffer--;
					}
					else
					{
						if(song=mainvar->GetActiveSong())
							song->RefillAudioDeviceBuffer(device);
						else
							device->SkipDeviceOutputBuffer(0);

						//}

						//if(device->outcounter==2) // [b1
						// device->doublebufferfilled.SetEvent();

						// Swap Buffer ------------------------------------
						//device->Callback_AudioBufferFilled(); // Call BEFORE device->play_bufferindex^=1 !!!

						waveOutWrite(device->hWaveOut,&device->header[device->play_bufferindex], sizeof(WAVEHDR));

						device->play_bufferindex^=1;
					}
				}
				while(IsExit()==false && device->CheckOutRefill()==true);
			}
		}

		mainthreadcontrol->Unlock(CS_audioplayback);
	}

	ThreadGone();
}

PTHREAD_START_ROUTINE AudioDeviceDeviceOutThread::Func(LPVOID pParam)// main audio refill thread
{
	AudioDeviceDeviceOutThread *proc=(AudioDeviceDeviceOutThread *)pParam;
	proc->WaitLoop();

	return 0;
}

int AudioDeviceDeviceOutThread::StartThread()
{
	int error=0;

	hWait=CreateEvent(NULL,FALSE,FALSE,"CamX WaveOut Sig"); // Create Thread Signal

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)Func,(LPVOID)this, 0,0);

	if(ThreadHandle)
		SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL); // Same as MIDI Out Proc
	else
		error++;

#endif
	return error;
}

void AudioDeviceDeviceOutThread::StopThread()
{
	exithread=true;

	if(ThreadHandle){

		//if(threadgone==false)
		{
			SetEvent(hWait);

			//	if(threadgone==false)
			WaitForSingleObject(gone, INFINITE);
			//Sleep(10);
		}

		ThreadHandle=0;
		CloseHandle(hWait);
	}
}

void AudioDevice::ConvertInputDataToSongInputARES(Seq_Song *song)
{
#ifdef DEBUG
	LONGLONG t1=maintimer->GetSystemTime();
#endif
	{
		// Dont use Multi Core
		AudioHardwareChannel *hw=FirstInputChannel();

		while(hw)
		{
			hw->DropPeak(devicedroprate);

			if(hw->hwinputbuffer.outputbufferARES)
			{
				hw->CopyFromHardware();
				hw->hwinputbuffer.channelsused=1; // Used by Audio Thru too
				//	hw->hwinputbuffer.CreatePeak(hw);
			}
			else
				hw->hwinputbuffer.channelsused=0;

			hw=hw->NextChannel();
		}
	}

	if(song)
		song->SongAudioDataInput(this,true); // HW Channel Buffer- >Track Input Buffer

	FillInputBufferEnd();

#ifdef DEBUG
	LONGLONG t2=maintimer->GetSystemTime();

	if(t2-t1>timetoconvertinputdata)
		timetoconvertinputdata=t2-t1;
#endif

}

void AudioDeviceDeviceInThread::WaitLoop()
{
	while(IsExit()==false)
	{
		WaitSignal();

		if(IsExit()==true)
			break;

		mainthreadcontrol->Lock(CS_audioinput);

		if(AudioDevice *device=mainaudio->GetActiveDevice()){

			if(device->stopdevice==false && device->devicestarted==true)
			{
				//device->incounter++;
				device->ConvertInputDataToSongInputARES(mainvar->GetActiveSong());
				if(mainvar->GetActiveSong())
					mainvar->GetActiveSong()->SyncAudioRecording(device);

				device->record_bufferindex^=1;

				//TRACE ("IO %d %d \n",device->incounter,device->outcounter);
			}
		}

		mainthreadcontrol->Unlock(CS_audioinput);

	}//while

	ThreadGone();
}

PTHREAD_START_ROUTINE AudioDeviceDeviceInThread::Func(LPVOID pParam)
{
	AudioDeviceDeviceInThread *proc=(AudioDeviceDeviceInThread *)pParam;
	proc->WaitLoop();
	return 0;
}

int AudioDeviceDeviceInThread::StartThread()
{
	int error=0;

	hWait=CreateEvent(NULL,FALSE,FALSE,"CamX WaveIn Sig"); // Create Thread Signal

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)Func,(LPVOID)this, 0,0);

	if(ThreadHandle)
		SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL); // Win32 Audio Input Highest
	else error++;

#endif
	return error;
}

void AudioDevice::InitLatencies()
{
	// Default IO Latency - Buffersize
	SetInputLatency(GetSetSize());
	SetOutputLatency(GetSetSize());
}

void AudioDevice::CleanMemory()
{
	TRACE ("<<<<< CLEAN Memory %s >>>>>\n",devname);

	if(inbuffer_raw)delete inbuffer_raw;
	inbuffer_raw=0;
	buffer_in_channels=0;

	if(buffer_out)delete buffer_out;
	buffer_out=0;
	buffer_out_channels=0;

	if(mixbuffer_out)delete mixbuffer_out;
	mixbuffer_out=0;
	devicepRepared=false;

	AudioHardwareChannel *in=FirstInputChannel();

	while(in)
	{
		// Input Buffer
		//if(in->hwinputbuffers)
		{
			//	for(int i=0;i<in->inputbufferscounter;i++)
			in->hwinputbuffer.DeleteARESOut();

			//delete in->hwinputbuffers;
			//in->hwinputbuffers=0;
			//in->inputbufferscounter=0;
		}

		// Input Record Buffer

		/*
		if(in->hwrecordbuffers)
		{
		for(int i=0;i<in->recordbufferscounter;i++)
		if(in->hwrecordbuffers[i].outputbufferARES)
		delete in->hwrecordbuffers[i].outputbufferARES;

		delete in->hwrecordbuffers;
		in->hwrecordbuffers=0;
		in->recordbufferscounter=0;
		}
		*/

		in=in->NextChannel();
	}
}

void AudioHDFile::ConvertARESToRecordingFormat()
{
	// ARES -> IO Format

	// outputbufferARES [ARES] -> inputbuffer32bit [RAW]

	if(writeoffset)
	{
		// CUT PUNCH etc..
		recordingoffset+=writeoffset;
		recordingsize-=writeoffset;

		writeoffset=0; // Reset
	}

#ifdef DEBUG
	if(recordingsize<0)
		maingui->MessageBoxError(0,"recfile->recordingsize <0");
#endif

	if(recmix.outputbufferARES && recmix.inputbuffer32bit && recordingsize>0)
	{
		if(recmix.channelsused==0)
		{
			size_t samples=recordingsize*channels;

			switch(samplebits)
			{
			case 16:
				{
					memset(recmix.inputbuffer32bit,0,sizeof(short)*samples);	
				}
				break;

			case 32:
				{
					memset(recmix.inputbuffer32bit,0,sizeof(float)*samples);	
				}
				break;

			case 64:
				{
					memset(recmix.inputbuffer32bit,0,sizeof(double)*samples);	
				}
				break;
			}

		}
		else
		{
			int xchannels=channels;

			for(int i=0;i<xchannels;i++)
			{
				int samples=recordingsize;

				ARES *from=recmix.outputbufferARES;

				from+=recordingoffset;
				from+=recmix.samplesinbuffer*i; // Block

				switch(samplebits)
				{
				case 16:
					{
						register short *to=(short *)recmix.inputbuffer32bit;

						to+=i;

						// Clip float to signed 16bit
						while(samples--)
						{
							// Left
							ARES h=*from++;

							if(h>0){
								if(h>=1)
									*to=SHRT_MAX;
								else
#ifdef ARES64
									*to=(short)floor((h*SHRT_MAX)+0.5);
#else
									*to=(short)floor((h*SHRT_MAX)+0.5f);
#endif
							}
							else{	
								if(h<=-1)
									*to=SHRT_MIN;
								else
#ifdef ARES64
									*to=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
									*to=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
							}

							/*
							if(h<0)
							*to=h<=-1?SHRT_MIN:(short)(h*SHRT_MIN);
							else
							*to=h>=1?SHRT_MAX:*to=(short)(h*SHRT_MAX);
							*/

							to+=xchannels;
						}
					}
					break;

				case 32:
					{
						register float *to=(float *)recmix.inputbuffer32bit;
						to+=i;

						// ARES to float
						while(samples--)
						{
							// Left
							ARES h=*from++;

							if(h>=1)*to=1;
							else
								if(h<=-1)*to=-1;
								else
									*to=(float)h;

							to+=xchannels;
						}

					}
					break;

				case 64:
					{
						register double *to=(double *)recmix.inputbuffer32bit;
						to+=i;

						// ARES to double
						while(samples--)
						{
							// Left
							ARES h=*from++;

							if(h>=1)*to=1;
							else
								if(h<=-1)*to=-1;
								else
									*to=(double)h;

							to+=xchannels;
						}
					}
					break;
				}
			}
		}
	}
}

AudioDevice_BufferSettings::AudioDevice_BufferSettings()
{
	setSize=minSize=maxSize=prefSize=latency_input_samples=latency_output_samples=granularity=0;

	addtoinputlatency_samples16=addtooutputlatency_samples16=
		addtoinputlatency_samples32=addtooutputlatency_samples32=
		addtoinputlatency_samples64=addtooutputlatency_samples64=
		addtoinputlatency_samples96=addtooutputlatency_samples96=
		addtoinputlatency_samples128=addtooutputlatency_samples128=
		addtoinputlatency_samples160=addtooutputlatency_samples160=
		addtoinputlatency_samples192=addtooutputlatency_samples192=
		addtoinputlatency_samples224=addtooutputlatency_samples224=
		addtoinputlatency_samples256=addtooutputlatency_samples256=
		addtoinputlatency_samples384=addtooutputlatency_samples384=
		addtoinputlatency_samples512=addtooutputlatency_samples512=
		addtoinputlatency_samples1024=addtooutputlatency_samples1024=
		addtoinputlatency_samples2048=addtooutputlatency_samples2048=
		addtoinputlatency_samples3072=addtooutputlatency_samples3072=
		addtoinputlatency_samples4096=addtooutputlatency_samples4096=
		addtoinputlatency_samples5120=addtooutputlatency_samples5120=
		addtoinputlatency_samples6144=addtooutputlatency_samples6144=
		addtoinputlatency_samples8192=addtooutputlatency_samples8192=
		addtoinputlatency_samplesmisc=addtooutputlatency_samplesmisc=0;

	used=false;
}

AudioDevice::AudioDevice()
{
	//	devicerefillsamplecounter=0;

	resetrequest=false;
	newsamplerate=0;

	ResetTimer();

	sync=false;
	userawdata=false;

	stopdevice=false;
	//mixbuffer_out_bytespersamples=0;
	mixbuffer_out=0;
	inbuffer_raw=0;

	buffer_out=0;
	buffer_out_channels=0;
	buffer_in_channels=0;

	skipoutbuffer=0;

	//dbdynamic=0;
	init=false;
	devicepRepared=false;
	devicestarted=false;

	// Stream/Audio Sync Loop Counter

	numberhardwarebuffer=0;

	//virtualdevice=false;
	info1[0]=0;

	devname=initname=devicetypname=0;

	inportsfound=outportsfound=false;

	for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
	{
		for(int c=0;c<CHANNELSPERPORT;c++)
		{
			if(c<6)
			{
				outputaudioports[i][c].visible=true;
				inputaudioports[i][c].visible=true;
			}

			inputaudioports[i][c].iotype=AudioPort::AP_INPUT;
			outputaudioports[i][c].iotype=AudioPort::AP_OUTPUT;

			inputaudioports[i][c].portindex=outputaudioports[i][c].portindex=c;
			outputaudioports[i][c].channels=inputaudioports[i][c].channels=channelschannelsnumber[i]; // Mono =1 etc..
		}
	}

	//devicestartcounter=0;
	outbuffercleared=false;

	recorderror=0;
	//ctrecordbuffer=0;
	recordbuffer_readwritecounter=0;

	play_bufferindex=record_bufferindex=recordbuffer_writeindex=recordbuffer_readindex=0;

	status=0;

#ifdef DEBUG
	timetoconvertinputdata=0;
#endif

	usebuffersettings=ADSR_48;

}

int AudioDevice::GetBestSampleRate()
{
	if(IsSampleRate(ADSR_44)==true)return 44100;
	if(IsSampleRate(ADSR_48)==true)return 48000;
	if(IsSampleRate(ADSR_88)==true)return 88200;
	if(IsSampleRate(ADSR_96)==true)return 96000;

	if(IsSampleRate(ADSR_176)==true)return 176400;
	if(IsSampleRate(ADSR_192)==true)return 192000;
	if(IsSampleRate(ADSR_352)==true)return 352800;
	if(IsSampleRate(ADSR_384)==true)return 384000;

	return 48000;
}

void AudioDevice::SetStart(Seq_Song *song,OSTART pos,int flag)
{
	// Record
	if(flag==SETSTART_INIT)
	{
		song->playback_sampleposition=song->record_sampleposition[0]=song->record_sampleposition[1]=song->timetrack.ConvertTicksToTempoSamples(pos);
		recorderror=0;
		song->playback_bufferindex=0;
		//	devicerecordstartposition=pos;
	}
}

void AudioDevice::CalcSampleBufferMs(int rate,int size)
{
	if(rate && size)
	{
		double ms=(double)size;
		ms/=(double)rate;
		ms*=1000;
		samplebufferms=ms;
		samplebufferms_long=(int)floor(samplebufferms+0.5);

		double h=samplebufferms/10,h2=0.01;
		h2*=h;
		devicedroprate=(ARES)h2;
	}
	else
	{
		samplebufferms=0;
		samplebufferms_long=0;
		devicedroprate=0.008;
	}

	ResetTimer();
}

void AudioDevice::CalcTicksPerBuffer()
{
	//if(virtualdevice==false) // virtual device always use 1 numberhardwarebuffer !
	if(GetSetSize())
	{
		int t=GetSetSize();

		if(t<=0)t=1; 

		double rafh=rafbuffersize[mainaudio->rafbuffersize_index];
		double ms=samplebufferms;

		if(ms<=0.1)
			ms=0.1;

		numberhardwarebuffer=(int)(rafh/ms)+1;

		if(numberhardwarebuffer<MINNUMBERHARDWAREBUFFER)
			numberhardwarebuffer=MINNUMBERHARDWAREBUFFER; // min 4 Buffer !

		TRACE ("DEvice %s NumberHardbuffer %d\n",devname,numberhardwarebuffer);
	}
#ifdef DEBUG
	else
		maingui->MessageBoxError(0,"setSize AudioDevice");
#endif

	//else
	//	numberhardwarebuffer=1; // always 1 !!!
}

AudioPort *AudioDevice::FindVOutChannel(int channelchannels,int index)
{
	if(channelchannels==0 || index>=CHANNELSPERPORT)return 0;

	// AudioVHardwareChannel *firstfound=FirstVOutChannel();

	for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
	{
		if(channelschannelsnumber[i]==channelchannels)
			return &outputaudioports[i][index];
	}

	return 0;
}

AudioPort *AudioDevice::FindVInChannel(int channelchannels,int index)
{
	if(channelchannels==0 || index>=CHANNELSPERPORT)return 0;

	for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
	{
		if(channelschannelsnumber[i]==channelchannels)
			return &inputaudioports[i][index];
	}
	return 0;
}

bool AudioDevice::CheckIfAllHardwareChannelsAreUsed()
{
	AudioHardwareChannel *c=FirstOutputChannel();
	while(c)
	{
		if(c->hardwarechannelused==false)
			return false;

		c=c->NextChannel();
	}
	return true;
}

AudioHardwareChannel *AudioDevice::FirstOutputChannelWithGroup(int group)
{
	AudioHardwareChannel *o=FirstOutputChannel();

	while(o)
	{
		if(o->audiochannelgroup==group)
			return o;

		o=o->NextChannel();
	}

	return 0;
}

AudioHardwareChannel *AudioDevice::FirstInputChannelWithGroup(int group)
{
	AudioHardwareChannel *i=FirstInputChannel();

	while(i)
	{
		if(i->audiochannelgroup==group)
			return i;

		i=i->NextChannel();
	}

	return 0;
}

bool AudioDevice::StartDevice()
{
	if(devicestarted==false && devicepRepared==true){

		bool error=false;

		//devicerefillsamplecounter=0;
		deviceoutofsync=deviceoutofMIDIsync=deviceoutofsyncmsg=false;

		if(inbuffer_raw)
			memset(inbuffer_raw,0,inbuffer_rawsize*sizeof(int));

		if(mixbuffer_out)
			memset(mixbuffer_out,0,sizeof(int)*buffer_out_channels*GetSetSize()*2);

		AudioHardwareChannel *out=FirstOutputChannel();

		while(out){
			out->hardwarechannelused=false;
			out=out->NextChannel();
		}

		AudioHardwareChannel *in=FirstInputChannel();

		while(in){
			in->hardwarechannelused=false;
			in=in->NextChannel();
		}

		outbuffercleared=false;
		devmix.channelsused=0;
		//	incounter=outcounter=0;
		//	/*inbufferwritec=*/inbufferreadc=/*inbufferrecc=*/0;
		play_bufferindex=record_bufferindex=0;

		ResetTimer();

		//inputbufferskipped=0;

		if(error==false)
			StartAudioHardware();
	}

	return devicestarted;
}


void AudioDevice::Load(camxFile *file)
{
	int exsetsize=0;
	file->ReadChunk(&exsetsize);

	for(int i=0;i<ADSR_LAST;i++)
	{
		file->ReadChunk(&buffersettings[i].setSize);
	}

	int exin=0,exout=0;
	for(int i=0;i<ADSR_LAST;i++)
	{
		file->ReadChunk(&exin);
		file->ReadChunk(&exout);
	}

	for(int i=0;i<ADSR_LAST;i++)
	{
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples16);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples32);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples64);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples96);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples128);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples160);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples192);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples224);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples256);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples384);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples512);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples1024);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples2048);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples3072);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples4096);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples5120);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples6144);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samples8192);
		file->ReadChunk(&buffersettings[i].addtoinputlatency_samplesmisc);

		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples16);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples32);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples64);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples96);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples128);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples160);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples192);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples224);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples256);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples384);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples512);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples1024);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples2048);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples3072);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples4096);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples5120);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples6144);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samples8192);
		file->ReadChunk(&buffersettings[i].addtooutputlatency_samplesmisc);
	}
}

void AudioDevice::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIODEVICE);
	int exsetsize=0;
	file->Save_Chunk(exsetsize);

	for(int i=0;i<ADSR_LAST;i++)
	{
		file->Save_Chunk(buffersettings[i].setSize);
	}

	int exin=0,exout=0;
	for(int i=0;i<ADSR_LAST;i++)
	{
		file->Save_Chunk(exin);
		file->Save_Chunk(exout);
	}

	for(int i=0;i<ADSR_LAST;i++)
	{
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples16);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples32);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples64);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples96);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples128);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples160);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples192);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples224);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples256);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples384);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples512);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples1024);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples2048);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples3072);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples4096);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples5120);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples6144);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samples8192);
		file->Save_Chunk(buffersettings[i].addtoinputlatency_samplesmisc);

		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples16);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples32);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples64);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples96);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples128);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples160);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples192);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples224);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples256);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples384);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples512);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples1024);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples2048);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples3072);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples4096);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples5120);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples6144);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samples8192);
		file->Save_Chunk(buffersettings[i].addtooutputlatency_samplesmisc);
	}

	file->CloseChunk();

	file->OpenChunk(CHUNK_AUDIODEVICEINPUTPORTS);

	for(int c=0;c<AUDIOCHANNELSDEFINED;c++)
	{
		for(int p=0;p<CHANNELSPERPORT;p++)
		{
			file->Save_Chunk(inputaudioports[c][p].visible);

			for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
			{
				int index=inputaudioports[c][p].hwchannel[i]?inputaudioports[c][p].hwchannel[i]->GetIndex():-1;
				file->Save_Chunk(index);
			}
		}
	}

	file->CloseChunk();

	file->OpenChunk(CHUNK_AUDIODEVICEOUTPUTPORTS);

	for(int c=0;c<AUDIOCHANNELSDEFINED;c++)
	{
		for(int p=0;p<CHANNELSPERPORT;p++)
		{
			file->Save_Chunk(outputaudioports[c][p].visible);

			for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
			{
				int index=outputaudioports[c][p].hwchannel[i]?outputaudioports[c][p].hwchannel[i]->GetIndex():-1;
				file->Save_Chunk(index);
			}
		}
	}

	file->CloseChunk();

}

void AudioDevice::ResetDeviceIndex()
{
	play_bufferindex=0;
	//	record_bufferindex=MIXBUFFER_INDEX1;	
}


void AudioDevice::StopDevice()
{
	stopdevice=true;

	//	StopAudioHardware();
	CloseDeviceDriver();

	AudioHardwareChannel *o=FirstOutputChannel();
	while(o){
		o->hardwarechannelused=false;
		o=o->NextChannel();
	}

	AudioHardwareChannel *i=FirstInputChannel();
	while(i){
		i->hardwarechannelused=false;
		i=i->NextChannel();
	}

	ResetDeviceIndex();

	devmix.channelsused=0;
	outbuffercleared=false;

	//playback_sampleposition=
	//	playback_sampleendposition=
	//record_songposition=
	//record_songendposition=0;

	devicestarted=false;
}

void Seq_Song::SendAudioMIDIAutomation()
{
	// 1. Tracks
	{
		Seq_Track *t=FirstTrack();

		while(t)
		{
			t->SendAudioAutomation(audioplayback_position,audioplayback_endposition);

			if(t->MIDItype==Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT)
				t->SendMIDIAutomation(audioplayback_position);

			t=t->NextTrack_NoUILock();
		}
	}

	// 2. Bus
	{
		AudioChannel *bus=audiosystem.FirstBusChannel();

		while(bus)
		{
			bus->SendAudioAutomation(audioplayback_position);
			bus=bus->NextChannel();
		}
	}

	audiosystem.masterchannel.SendAudioAutomation(audioplayback_position);
	audiosystem.masterchannel.SendMIDIAutomation(audioplayback_position);
}

void Seq_Song::AddAudioInstrumentsToStream(AudioDevice *device) // Audio Instruments
{
	SendAudioMIDIAutomation();

	cMIDIPlaybackEvent nmpbe(INITPLAY_AUDIOTRIGGER);
	GetNextMIDIPlaybackEvent(&nmpbe);

	while(nmpbe.playbackevent &&
		nmpbe.nextMIDIplaybacksampleposition<playback_sampleposition) // Security:Skip Events < [],Avoid Events beforce []=crash
	{		
		// Possible after Copy Init MIDI->Audio
		MixNextMIDIPlaybackEvent(&nmpbe);
		GetNextMIDIPlaybackEvent(&nmpbe);
	}

	int oflag=mastering==true?TRIGGER_MASTEREVENT:0;

	// Song Events --------------------------------------
	while(nmpbe.playbackevent && nmpbe.nextMIDIplaybacksampleposition<playback_sampleendposition)
	{
		if((!(nmpbe.playbackevent->flag&EVENTFLAG_MUTED)) && 
			nmpbe.playbackpattern && 
			nmpbe.playbackpattern->track->CheckIfTrackIsAudioInstrument()==true
			)
		{
			int sampleoffset=playback_sampleoffset+(int)(nmpbe.nextMIDIplaybacksampleposition-playback_sampleposition);
			bool mute=true;

			if(sampleoffset<0)
			{
#ifdef DEBUG
				maingui->MessageBoxError(0,"AddAudioInstrumentsToStream sampleoffset<0");
#endif
				sampleoffset=playback_sampleoffset;
			}
			else
				if(sampleoffset>=device->GetSetSize())
				{
#ifdef DEBUG
					maingui->MessageBoxError(0,"AddAudioInstrumentsToStream sampleoffset>0");
#endif
					sampleoffset=device->GetSetSize()-1;
				}

				bool cansend=nmpbe.playbackpattern->CheckIfPlaybackIsAble();

				MIDIProcessor processor(this,nmpbe.playbackpattern->track);

				processor.EventInput(nmpbe.playbackpattern,nmpbe.playbackevent,nmpbe.nextMIDIplaybacksampleposition); // ->> fill+create Processor Events

				Seq_Event *oevent=processor.FirstProcessorEvent();

				while(oevent){

					if(oevent->CheckIfPlaybackIsAble()==false || // Check only Note == MUTE ?
						cansend==true
						)
					{
						if(mute==true && mastering==false)
							oevent->SetMIDIImpulse(nmpbe.playbackpattern->track);
							
						Seq_Track *track=nmpbe.playbackpattern->track;

						do // Child->Parent Loop
						{
							if(oevent->CheckIfPlaybackIsAble()==false || track->CheckIfPlaybackIsAble()==true)
							{
								// Tracks
								InsertAudioEffect *ci=track->io.audioeffects.FirstActiveAudioEffect();

								while(ci){ // Tracks AudioInstrument Loop ?

									if(ci->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT && ci->audioeffect->plugin_on==true && ci->MIDIfilter.CheckEvent(oevent)==true){

										SendAudioWaitEvents(device,playback_sampleposition,nmpbe.nextMIDIplaybacksampleposition+1,false); // +1 force send before nextMIDIplaybacksampleposition

										// ---> Audio Trigger List
										oevent->SendToAudioPlayback(
											ci->audioeffect,
											nmpbe.nextMIDIplaybacksampleposition,
											track,
											nmpbe.playbackpattern,
											nmpbe.playbackevent,
											oflag,
											sampleoffset,
											true
											);
									}

									ci=ci->NextEffect();
								}
							}

							track=(Seq_Track *)track->parent;

						}while(track);
					}

					oevent=processor.DeleteEvent(oevent);	
				}// while proc events

		}// if Instrument

		MixNextMIDIPlaybackEvent(&nmpbe); // Get next Audio Playback Event
		GetNextMIDIPlaybackEvent(&nmpbe);

	}// while playevent

	SendAudioWaitEvents(device,playback_sampleposition,playback_sampleendposition,false); // NoteOffs... Full Buffer Rest
}

int AudioDevice::GetAddToInputLatencySamples()
{
	switch(GetSetSize())
	{
	case 16:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples16; 
		break;

	case 32:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples32; 
		break;

	case 64:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples64; 
		break;

	case 96:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples96; 
		break;

	case 128:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples128; 
		break;

	case 160:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples160; 
		break;
	case 192:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples192; 
		break;
	case 224:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples224; 
		break;
	case 256:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples256; 
		break;
	case 384:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples384; 
		break;
	case 512:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples512; 
		break;
	case 1024:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples1024; 
		break;
	case 2048:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples2048; 
		break;

	case 3072:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples3072; 
		break;

	case 4096:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples4096; 
		break;

	case 5120:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples5120; 
		break;

	case 6144:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples6144; 
		break;

	case 8192:
		return  buffersettings[usebuffersettings].addtoinputlatency_samples8192; 
		break;

	default:
		return buffersettings[usebuffersettings].addtoinputlatency_samplesmisc;
	}
}

int AudioDevice::GetAddToOutputLatencySamples()
{
	switch(GetSetSize())
	{
	case 16:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples16; 
		break;

	case 32:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples32; 
		break;

	case 64:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples64; 
		break;

	case 96:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples96; 
		break;

	case 128:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples128; 
		break;

	case 160:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples160; 
		break;
	case 192:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples192; 
		break;
	case 224:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples224; 
		break;
	case 256:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples256; 
		break;
	case 384:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples384; 
		break;
	case 512:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples512; 
		break;
	case 1024:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples1024; 
		break;
	case 2048:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples2048; 
		break;

	case 3072:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples3072; 
		break;

	case 4096:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples4096; 
		break;

	case 5120:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples5120; 
		break;
	case 6144:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples6144; 
		break;

	case 8192:
		return  buffersettings[usebuffersettings].addtooutputlatency_samples8192; 
		break;

	default:
		return buffersettings[usebuffersettings].addtooutputlatency_samplesmisc;
	}
}

void AudioDevice::SetAddInputLatency(int v)
{
	switch(GetSetSize())
	{
	case 16:
		buffersettings[usebuffersettings].addtoinputlatency_samples16=v; 
		break;

	case 32:
		buffersettings[usebuffersettings].addtoinputlatency_samples32=v;
		break;

	case 64:
		buffersettings[usebuffersettings].addtoinputlatency_samples64=v; 
		break;

	case 96:
		buffersettings[usebuffersettings].addtoinputlatency_samples96=v;
		break;

	case 128:
		buffersettings[usebuffersettings].addtoinputlatency_samples128=v;
		break;

	case 160:
		buffersettings[usebuffersettings].addtoinputlatency_samples160=v; 
		break;
	case 192:
		buffersettings[usebuffersettings].addtoinputlatency_samples192=v; 
		break;
	case 224:
		buffersettings[usebuffersettings].addtoinputlatency_samples224=v;
		break;
	case 256:
		buffersettings[usebuffersettings].addtoinputlatency_samples256=v;
		break;
	case 384:
		buffersettings[usebuffersettings].addtoinputlatency_samples384=v;
		break;
	case 512:
		buffersettings[usebuffersettings].addtoinputlatency_samples512=v;
		break;
	case 1024:
		buffersettings[usebuffersettings].addtoinputlatency_samples1024=v;
		break;
	case 2048:
		buffersettings[usebuffersettings].addtoinputlatency_samples2048=v;
		break;

	case 3072:
		buffersettings[usebuffersettings].addtoinputlatency_samples3072=v;
		break;

	case 4096:
		buffersettings[usebuffersettings].addtoinputlatency_samples4096=v;
		break;

	case 5120:
		buffersettings[usebuffersettings].addtoinputlatency_samples5120=v;
		break;
	case 6144:
		buffersettings[usebuffersettings].addtoinputlatency_samples6144=v;
		break;

	case 8192:
		buffersettings[usebuffersettings].addtoinputlatency_samples8192=v;
		break;

	default:
		buffersettings[usebuffersettings].addtoinputlatency_samplesmisc=v;
	}
}

void AudioDevice::SetPrefBufferSize(int v)
{
	buffersettings[usebuffersettings].prefSize=v;
	if(GetSetSize()==0)
		SetBufferSize(v);
}

void AudioDevice::SetAddOutputLatency(int v)
{
	switch(GetSetSize())
	{
	case 16:
		buffersettings[usebuffersettings].addtooutputlatency_samples16=v; 
		break;

	case 32:
		buffersettings[usebuffersettings].addtooutputlatency_samples32=v;
		break;

	case 64:
		buffersettings[usebuffersettings].addtooutputlatency_samples64=v; 
		break;

	case 96:
		buffersettings[usebuffersettings].addtooutputlatency_samples96=v;
		break;

	case 128:
		buffersettings[usebuffersettings].addtooutputlatency_samples128=v;
		break;

	case 160:
		buffersettings[usebuffersettings].addtooutputlatency_samples160=v; 
		break;
	case 192:
		buffersettings[usebuffersettings].addtooutputlatency_samples192=v; 
		break;
	case 224:
		buffersettings[usebuffersettings].addtooutputlatency_samples224=v;
		break;
	case 256:
		buffersettings[usebuffersettings].addtooutputlatency_samples256=v;
		break;
	case 384:
		buffersettings[usebuffersettings].addtooutputlatency_samples384=v;
		break;
	case 512:
		buffersettings[usebuffersettings].addtooutputlatency_samples512=v;
		break;
	case 1024:
		buffersettings[usebuffersettings].addtooutputlatency_samples1024=v;
		break;
	case 2048:
		buffersettings[usebuffersettings].addtooutputlatency_samples2048=v;
		break;

	case 3072:
		buffersettings[usebuffersettings].addtooutputlatency_samples3072=v;
		break;

	case 4096:
		buffersettings[usebuffersettings].addtooutputlatency_samples4096=v;
		break;

	case 5120:
		buffersettings[usebuffersettings].addtooutputlatency_samples5120=v;
		break;
	case 6144:
		buffersettings[usebuffersettings].addtooutputlatency_samples6144=v;
		break;

	case 8192:
		buffersettings[usebuffersettings].addtooutputlatency_samples8192=v;
		break;

	default:
		buffersettings[usebuffersettings].addtooutputlatency_samplesmisc=v;
	}
}

void AudioDevice::DeleteAudioBuffers()
{
	TRACE ("Audio Device DeleteAudioBuffers %s\n",devname);

	devmix.DeleteARESOut();
	mastermix.DeleteARESOut();
}

void AudioDevice::CreateAudioBuffers()
{
	TRACE ("Audio Device CreateBuffers %s\n",devname);

	DeleteAudioBuffers();

	CalcSampleBufferMs(mainaudio->GetGlobalSampleRate(),GetSetSize());

	int devchannels=MAXCHANNELSPERCHANNEL;

	if(GetCountOfOutputChannels()>devchannels)
		devchannels=GetCountOfOutputChannels();

	outbuffercleared=false;

	devmix.InitARESOut(GetSetSize(),devchannels);
	mastermix.InitARESOut(GetSetSize(),devchannels);

	//devmix.channelsinbuffer=;
	mastermix.channelsused=devmix.channelsused=0;

	devmix.SetBuffer(devchannels,GetSetSize());
	mastermix.SetBuffer(devchannels,GetSetSize());

	//devmix.samplesinbuffer=setSize;
	mastermix.bufferms=devmix.bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),GetSetSize());

	if(devmix.bufferms<=0)
		devmix.bufferms=0.1; // mini !

	devmix.delayvalue=(int)floor((1000/devmix.bufferms)+0.5);

	if(mastermix.bufferms<=0)
		mastermix.bufferms=0.1; // mini !

	mastermix.delayvalue=(int)floor((1000/mastermix.bufferms)+0.5);

#ifdef _DEBUG
	devmix.CheckBuffer();
#endif

	// Create BufferArray
	CleanMemory();

	buffer_out_channels=GetCountOfOutputChannels();
	buffer_in_channels=GetCountOfInputChannels();

	TRACE ("Create AHW Buffer %d Ins %d Outs %d\n",numberhardwarebuffer,buffer_in_channels,buffer_out_channels);

	if(buffer_in_channels)
	{
		TRACE ("Create INBUFFER RAW Size %d\n",GetSetSize());

		if(userawdata==true)
			inbuffer_raw=new int[inbuffer_rawsize=buffer_in_channels*GetSetSize()*2]; // *2=index 0/1

		if(inbuffer_raw || userawdata==false)
		{
			TRACE ("Create INBUFFER Input Channels %d\n",GetSetSize());

			// Record Buffer 3x numberhardwarebuffer
			//ctrecordbuffer=3*numberhardwarebuffer;

			//memset(inbuffer_raw,0,inbuffer_rawsize*sizeof(int));
			AudioHardwareChannel *in=FirstInputChannel();

			while(in)
			{
				// Input Buffer
				//	if(numberhardwarebuffer && (in->hwinputbuffers=new AudioHardwareBufferEx[in->inputbufferscounter=numberhardwarebuffer]))
				//	{
				//for(int i=0;i<numberhardwarebuffer;i++)
				//	{

				in->hwinputbuffer.InitARESOut(GetSetSize(),1);

				if(in->hwinputbuffer.outputbufferARES==0){
					CleanMemory();
					break;
				}

				in->hwinputbuffer.bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),GetSetSize());
				in->hwinputbuffer.SetBuffer(1,GetSetSize());
				//in->inputbuffers[i].samplesinbuffer=setSize;
				//	}
				//	}

#ifdef OLDIE
				// Record Buffer
				if(ctrecordbuffer && (in->hwrecordbuffers=new AudioHardwareBufferEx[in->recordbufferscounter=ctrecordbuffer]))
				{
					for(int i=0;i<ctrecordbuffer;i++)
					{
#ifdef _DEBUG
						in->hwrecordbuffers[i].outputbufferARES=new ARES[setSize+1];
						in->hwrecordbuffers[i].outputbufferARES[setSize]=1.1f;		
#else
						in->hwrecordbuffers[i].outputbufferARES=new ARES[setSize];
#endif

						//	in->hwrecordbuffers[i].outputbufferARES=new ARES[setSize];

						if(in->hwrecordbuffers[i].outputbufferARES==0){
							CleanMemory();
							break;
						}

						in->hwrecordbuffers[i].bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),setSize);
						in->hwrecordbuffers[i].SetBuffer(1,setSize);
						//in->inputbuffers[i].samplesinbuffer=setSize;
					}
				}
#endif

				in=in->NextChannel();
			}
		}
	}

	if(buffer_out_channels && userawdata==true)
	{
		buffer_out=new int[buffer_out_channels*GetSetSize()*2]; // *2=index 0/1

		if(buffer_out)
		{
			mixbuffer_out=new int[buffer_out_channels*GetSetSize()*2];	// *2=index 0/1

			if(mixbuffer_out)
			{
				//memset(buffer_out,0,sizeof(int)*buffer_out_channels*setSize*2);
				//memset(mixbuffer_out,0,sizeof(int)*buffer_out_channels*setSize*2);
			}
			else
			{
				delete buffer_out;
				buffer_out=0;
			}
		}
	}
}

bool AudioDevice::CheckSampleRate(int samplerate)
{
	switch(samplerate)
	{
	case 384000:
		if(IsSampleRate(ADSR_384)==true)
			return true;
		break;

	case 352800:
		if(IsSampleRate(ADSR_352)==true)
			return true;
		break;

	case 192000:
		if(IsSampleRate(ADSR_192)==true)
			return true;
		break;

	case 176400:
		if(IsSampleRate(ADSR_176)==true)
			return true;
		break;


	case 96000:
		if(IsSampleRate(ADSR_96)==true)
			return true;
		break;

	case 88200:
		if(IsSampleRate(ADSR_88)==true)
			return true;
		break;

	case 48000:
		if(IsSampleRate(ADSR_48)==true)
			return true;
		break;

	case 44100:
		if(IsSampleRate(ADSR_44)==true)
			return true;
		break;
	}

	return false;
}

void AudioDevice::AddInputHWChannel(AudioHardwareChannel *inchl,int index)
{
	inchl->channelindex=index;
	inchl->device=this;
	inchannels.AddEndO(inchl);	
}

void AudioDevice::AddOutputHWChannel(AudioHardwareChannel *outchl,int index)
{
	outchl->channelindex=index;
	outchl->device=this;
	outchannels.AddEndO(outchl);	
}

bool AudioDevice::CloseAudioDevice(bool full)
{
	StopDevice();
	CleanMemory();
	CloseDeviceDriver();
	DeleteAudioBuffers();

	TRACE ("SIZEOF AHC %d\n",sizeof(AudioHardwareChannel));

	// Clear Input
	if(full==true)
	{
		AudioHardwareChannel *chl=FirstInputChannel();
		while(chl)
		{
			chl=(AudioHardwareChannel *)inchannels.RemoveO(chl);
		}

		// Clear Output
		chl=FirstOutputChannel();
		while(chl)
		{
			chl=(AudioHardwareChannel *)outchannels.RemoveO(chl);
		}

		if(initname)
		{
			delete initname;
			initname=0;
		}

		if(devicetypname)
		{
			delete devicetypname;
			devicetypname=0;
		}

		if(devname)
		{
			delete devname;
			devname=0;
		}

	}

	init=false;

	return true;
}

void AudioDevice::InitDefaultPorts()
{
	// Input
	for(int i=0;i<AUDIOCHANNELSDEFINED;i++) // Mono, Stereo etc..
	{
		AudioHardwareChannel *hwc=FirstInputChannel();

		for(int p=0;p<CHANNELSPERPORT;p++)
		{
			int usegroup;

			for(int c=0;c<channelschannelsnumber[i];c++)
			{
				if(inportsfound==false || inputaudioports[i][p].hwchannel[c]==0)
					inputaudioports[i][p].hwchannel[c]=hwc;

				if(inputaudioports[i][p].hwchannel[0])
					usegroup=inputaudioports[i][p].hwchannel[0]->audiochannelgroup;
				else
					usegroup=0;

				if(hwc)
				{
					AudioHardwareChannel *nhwc=hwc->NextChannel();

					if(nhwc && nhwc->audiochannelgroup!=usegroup)
						hwc=FirstInputChannelWithGroup(usegroup); // Use same Group First Channel 
					else
						hwc=nhwc;

					if(!hwc)
					{
						hwc=FirstInputChannelWithGroup(usegroup);

						if(!hwc)
							hwc=FirstInputChannel();
					}
				}

				if(c==channelschannelsnumber[i]-1)
				{
					hwc=inputaudioports[i][p].hwchannel[c];

					if(hwc)
						hwc=hwc->NextChannel();

					if(!hwc)
						hwc=FirstInputChannel();
				}
			}
		}
	}

	for(int i=0;i<AUDIOCHANNELSDEFINED;i++) // Mono, Stereo etc..
	{
		AudioHardwareChannel *hwc=FirstOutputChannel();

		for(int p=0;p<CHANNELSPERPORT;p++)
		{
			int usegroup;

			for(int c=0;c<channelschannelsnumber[i];c++)
			{
				if(outportsfound==false || outputaudioports[i][p].hwchannel[c]==0)
					outputaudioports[i][p].hwchannel[c]=hwc;

				if(outputaudioports[i][p].hwchannel[0])
					usegroup=outputaudioports[i][p].hwchannel[0]->audiochannelgroup;
				else
					usegroup=0;

				if(hwc)
				{
					AudioHardwareChannel *nhwc=hwc->NextChannel();

					if(nhwc && nhwc->audiochannelgroup!=usegroup)
						hwc=FirstOutputChannelWithGroup(usegroup); // Use same Group First Channel 
					else
						hwc=nhwc;

					if(!hwc)
					{
						hwc=FirstOutputChannelWithGroup(usegroup);

						if(!hwc)
							hwc=FirstOutputChannel();
					}
				}

				if(c==channelschannelsnumber[i]-1)
				{
					hwc=outputaudioports[i][p].hwchannel[c];

					if(hwc)
						hwc=hwc->NextChannel();

					if(!hwc)
						hwc=FirstOutputChannel();
				}
			}
		}
	}

	// Port Names
	for(int i=0;i<AUDIOCHANNELSDEFINED;i++) // Mono, Stereo etc..
	{
		for(int p=0;p<CHANNELSPERPORT;p++)
		{
			inputaudioports[i][p].GenerateName();
			outputaudioports[i][p].GenerateName();
			//TRACE ("In Name %s Out Name %s\n",inputaudioports[i][p].name,outputaudioports[i][p].name);
		}
	}
}

void AudioDevice::DeInitDefaultPorts()
{
	// Port Names
	for(int i=0;i<AUDIOCHANNELSDEFINED;i++) // Mono, Stereo etc..
	{
		for(int p=0;p<CHANNELSPERPORT;p++)
		{
			for(int c=0;c<channelschannelsnumber[i];c++)
			{
				//	TRACE ("DeInitDefaultPorts In %s\n",inputaudioports[i][p].name);

				if(inputaudioports[i][p].name)delete inputaudioports[i][p].name;
				inputaudioports[i][p].name=0;

				//	TRACE ("DeInitDefaultPorts Out %s\n",outputaudioports[i][p].name);

				if(outputaudioports[i][p].name)delete outputaudioports[i][p].name;
				outputaudioports[i][p].name=0;
			}
		}
	}
}

// Playback
void AudioDevice::ClearOutputBuffer(Seq_Song *song) // called by Audio Device Refill SkipDeviceOutputBuffer
{
	if(song && song->masteringlock==false)
	{
		AudioHardwareChannel *hw=FirstOutputChannel();

		while(hw){
			if(hw->channelcleared[play_bufferindex]==false)
			{
				hw->channelcleared[play_bufferindex]=true;
				hw->ClearChannel(devmix.samplesinbuffer,play_bufferindex);
			}

			hw=hw->NextChannel();
		}
	}

	//CreateDeviceStreamMix(song); // V L/R Mix
	CopyDeviceToHardware(song); // Main HardwareBuffer Mix
}

void AudioHardwareChannel::CopyToHardware(Seq_Song *song)
{
	// -> Output
	if(channelindex<device->devmix.channelsused) // Set DevMix Channels Used->HW
		hardwarechannelused=true;

	if(hardwarechannelused==false ||
		(audiochannel && audiochannel->io.audioeffects.GetMute()==true) ||
		(song && song->audiosystem.masterchannel.io.audioeffects.GetMute()==true) ||
		(song && song->masteringlock==true)
		)
	{
		// Mute
		currentpeak=0;
		hardwarechannelused=false;

		if(channelcleared[device->play_bufferindex]==false)
		{
			channelcleared[device->play_bufferindex]=true;
			ClearChannel(device->devmix.samplesinbuffer,device->play_bufferindex); // ASIO Set Channel to 0
		}
	}
	else{
		ARES *from=device->devmix.outputbufferARES;
		from+=channelindex*device->devmix.samplesinbuffer; // +Channel

		// Device Out Channel Volume
		if(audiochannel){

			ARES mul=(ARES)audiochannel->io.audioeffects.volume.GetValue();

			mul*=LOGVOLUME_SIZE;

#ifdef ARES64
			mul=logvolume[(int)mul];
#else
			mul=logvolume_f[(int)mul];
#endif
			if(mul!=1){

				ARES *mulfrom=from;

				int i=device->devmix.samplesinbuffer;
				while(i--)*mulfrom++ *=mul;
			}
		}

		channelcleared[device->play_bufferindex]=false;
		WriteToHardwareBufferData(from,device->devmix.samplesinbuffer,device->play_bufferindex);
	}

	//if(currentpeak>max)max=hw->currentpeak;
}

ARES AudioDevice::CopyDeviceToHardware(Seq_Song *song)
{
	if(!devmix.outputbufferARES)
		return 0;

#ifdef DEBUG
	devmix.CheckBuffer();
#endif

	//copytoportindex=play_bufferindex;

	/*
	if(song && mainvar->cpucores>=2)
	{
	mainaudiostreamproc->DoFunction(song,DOCOREDEVICETOHW,song->CreateAudioDeviceChannelList(this));
	}
	else
	*/

	if(song && song->masteringlock==true)
	{
		AudioHardwareChannel *hw=FirstOutputChannel();

		while(hw)
		{
			if(hw->channelcleared[play_bufferindex]==false)
			{
				hw->channelcleared[play_bufferindex]=true;
				hw->ClearChannel(devmix.samplesinbuffer,play_bufferindex); // ASIO Set Channel to 0
			}

			hw=hw->NextChannel();
		}
	}
	else
	{
		// No Song or 1 Cpu Core
		AudioHardwareChannel *hw=FirstOutputChannel();

		while(hw)
		{
			hw->CopyToHardware(song);
			hw=hw->NextChannel();
		}
	}

	CreateDeviceStreamMix(song); // V create L/R Mix Buffer

	ARES max=0;

	// Reset
	if(song && song->masteringlock==false)
	{
		AudioHardwareChannel *hw=FirstOutputChannel();

		while(hw){

			if(hw->currentpeak>max)
				max=hw->currentpeak;

			hw->hardwarechannelused=false;
			hw=hw->NextChannel();
		}
	}

	devmix.channelsused=0;

#ifdef DEBUG
	devmix.CheckBuffer();
#endif

	return max;
}

void AudioHardwareChannel::SetName(char *n)
{
	if(!n)
		return;

	if(strlen(n)>127)
	{
		strncpy(hwname,n,127);
		hwname[127]=0;
	}
	else
		strcpy(hwname,n);
}

#ifdef CONCO
void AudioDevice::WriteVirtualDevice(Seq_Song *song,AudioHDFile *file,int sampleformat,bool *checkstartsample,int *sampleoffset)
{
	if(mixbuffer_out)
	{
		int offset=0;
		bool offsetset=false,write=true;

		if(checkstartsample && *checkstartsample==true)
		{
			write=false;

			AudioHardwareChannel *hw=FirstOutputChannel();

			while(hw)
			{
				if(hw->hardwarechannelused==true)
				{
					for(int c=0;c<GetCountOfOutputChannels();c++)
					{
						ARES *from=devmix.outputbufferARES;
						from+=c*devmix.samplesinbuffer; // +Channel

						for(int i=0;i<devmix.samplesinbuffer;i++)
						{
							if(*from!=0)
							{
								write=true;
								*checkstartsample=false;

								if(i>0)i--;

								if(i<offset || offsetset==false)
								{
									offset=i;
									offsetset=true;
								}

								break;
							}

							from++;
						}
					}

					break;
				}

				hw=hw->NextChannel();
			}
		}

		if(write==true)
		{
			CopyDeviceToHardware(song,0);

			char *writefrom=(char *)mixbuffer_out;
			int writesize=setSize;

			if(offset)
			{
				writefrom+=offset*mixbuffer_out_bytespersamples*GetCountOfOutputChannels();
				writesize-=offset;
			}

			file->channelsampleswritten+=setSize;
			file->Save(writefrom,writesize*mixbuffer_out_bytespersamples*GetCountOfOutputChannels());
		}
	}

#ifdef VIRTUA
	if(sampleoffset)
		*sampleoffset=0;

	AudioHardwareChannel *c=FirstOutputChannel();

	while(c)
	{
		bool foundzerobuffer=true;
		long writesamples=c->numberofsamples;;
		long writeoffset=0;
		long samplesize=0;

		if(c->audiochannel && c->audiochannel->channelmix)
		{
			if(c->audiochannel->channelmix[0].channelsused)
			{
				c->audiochannel->channelmix[0].channelsused=0; // reset

				bool skipzero=false;

				ARES *from_l,*from_r;
				int i=c->numberofsamples;

				from_l=from_r=c->audiochannel->channelmix[0].outputbufferARES;
				from_r+=i;

				// Check Buffer for !0 Samples
				if(checkstartsample && *checkstartsample==true)
				{
					int ci=i;
					ARES *c_l=from_l;
					ARES *c_r=from_r;

					skipzero=true;

					while(ci)
					{
						if(*c_l++!=0 || *c_r++!=0)
						{
							foundzerobuffer=false;
							skipzero=false;

							// Check First Used Sample
							*checkstartsample=false;
							writeoffset=i-ci;
							writesamples=ci;

							if(sampleoffset)
								*sampleoffset=writeoffset;

							break;
						}

						ci--;
					}

					if(skipzero==true)
						writesamples=0;
				}
				else
				{
					int ci=i;
					ARES *c_l=from_l;
					ARES *c_r=from_r;

					while(ci--)
					{
						if(*c_l++!=0 || *c_r++!=0)
						{
							foundzerobuffer=false;
							break;
						}
					}
				}

				// switch audio sample type
				if(skipzero==false)
					switch(sampleformat)
				{
					case  MASTERFORMAT_16BIT:
						{
#ifdef _DEBUG
							ARES sum_l=0;
							ARES sum_r=0;
#endif
							short *cto=(short *)c->virtualbuffer;

							// Mix Channels to CHL 1 | CHL 2 etc
							while(i--)
							{
#ifdef _DEBUG
								sum_l+=*from_l;
								sum_r+=*from_r;
#endif
								// Left
								if(*from_l>=1)
									*cto++=32767;
								else
									if(*from_l<=-1)
										*cto++=-32768;
									else
										*cto++=(short)(*from_l*32768);

								// Right
								if(*from_r>=1)
									*cto++=32767;
								else
									if(*from_r<=-1)
										*cto++=-32768;
									else
										*cto++=(short)(*from_r*32768);

								if(normalize) // Normalizer Stereo
								{
									if(*from_l<0 && -*from_l>*normalize)
										*normalize=-*from_l;
									else
										if(*from_l>*normalize)
											*normalize=*from_l;

									if(*from_r<0 && -*from_r>*normalize)
										*normalize=-*from_r;
									else
										if(*from_r>*normalize)
											*normalize=*from_r;
								}

								from_l++;
								from_r++;

							}// while i
#ifdef _DEBUG
							TRACE ("Sum 16 Bit L %f R %f\n",sum_l,sum_r);
#endif
							samplesize=sizeof(short);
						}
						break;

					case  MASTERFORMAT_24BIT:
						{
							char *charto=(char *)c->virtualbuffer;

							union 
							{
								char help[4];
								long value;
							}u;

							// Mix Channels to CHL 1 | CHL 2 etc
							while(i--)
							{
								//	*from_l=-0.7f;
								//	*from_r=0.7f;

								// Left <<<<<<<<<<<<<<<<<
								if(*from_l>=1)
									u.value=8388607;
								else
									if(*from_l<=-1)
										u.value=-8388608;
									else
										u.value=(long)(*from_l*8388608);

								/*
								if(u.value<0)
								u.value+=16777216; // -> linear
								*/
								*charto++=u.help[0];
								*charto++=u.help[1];
								*charto++=u.help[2];

								// Right >>>>>>>>>>>>>
								if(*from_r>=1)
									u.value=8388607;
								else
									if(*from_r<=-1)
										u.value=-8388608;
									else
										u.value=(long)(*from_r*8388608);

								/*
								if(u.value<0)
								u.value+=16777216; // -> linear
								*/

								*charto++=u.help[0];
								*charto++=u.help[1];
								*charto++=u.help[2];

								if(normalize) // Normalizer Stereo
								{
									if(*from_l<0 && -*from_l>*normalize)
										*normalize=-*from_l;
									else
										if(*from_l>*normalize)
											*normalize=*from_l;

									if(*from_r<0 && -*from_r>*normalize)
										*normalize=-*from_r;
									else
										if(*from_r>*normalize)
											*normalize=*from_r;
								}

								from_l++;
								from_r++;

							}// while

							samplesize=3;
						}
						break;

					case MASTERFORMAT_32BITFLOAT:
						{
							ARES *cto=(ARES *)c->virtualbuffer;

							// Mix Channels to CHL 1 | CHL 2 etc
							while(i--)
							{
								// Left
								if(*from_l>=1)
									*cto++=1;
								else
									if(*from_l<=-1)
										*cto++=-1;
									else
										*cto++=*from_l;

								// Right

								if(*from_r>=1)
									*cto++=1;
								else
									if(*from_r<=-1)
										*cto++=-1;
									else
										*cto++=*from_r;

								if(normalize) // Normalizer Stereo
								{
									if(*from_l<0 && -*from_l>*normalize)
										*normalize=-*from_l;
									else
										if(*from_l>*normalize)
											*normalize=*from_l;

									if(*from_r<0 && -*from_r>*normalize)
										*normalize=-*from_r;
									else
										if(*from_r>*normalize)
											*normalize=*from_r;
								}

								from_l++;
								from_r++;
							}// while

							samplesize=sizeof(ARES);
						}
						break;
				}//switch
			}
			else // Write 0 Samples
			{
				if(checkstartsample && *checkstartsample==true)
					writesamples=0;

				int clear=c->numberofsamples;

				switch(sampleformat)
				{
				case  MASTERFORMAT_16BIT:
					{
						if(c->virtualbuffercleared==false)
						{
							memset(c->virtualbuffer,0,clear*sizeof(short));

							/*
							// switch audio sample type
							short *cto=(short *)c->virtualbuffer;

							while(clear--)
							*cto ++=0;
							*/
						}

						samplesize=sizeof(short);
					}
					break;

				case  MASTERFORMAT_24BIT:
					{
						if(c->virtualbuffercleared==false)
						{
							memset(c->virtualbuffer,0,clear*sizeof(long));

							/*
							// switch audio sample type
							long *cto=(long *)c->virtualbuffer;

							while(clear--)
							*cto ++=0;
							*/
						}

						samplesize=3;
					}
					break;

				case  MASTERFORMAT_32BITFLOAT:
					{
						if(c->virtualbuffercleared==false)
						{
							memset(c->virtualbuffer,0,clear*sizeof(float));


							// switch audio sample type
							// float *cto=(float *)c->virtualbuffer;

							// while(clear--)
							//*cto ++=0;

						}

						samplesize=sizeof(float);
					}
					break;
				}// switch

				c->virtualbuffercleared=true;
			}

			if(writesamples)
			{
				if(foundzerobuffer==true)
				{
					file->writezerobytes+=writesamples*samplesize;
					file->addzerosamples+=writesamples;
				}else
				{
					if(file->writezerobytes)
					{
						if(!zerobuffer)
						{
							zerobuffer=new char[32768];
							if(zerobuffer)
								memset(zerobuffer,0,32768);
						}

						if(zerobuffer)
						{
							long loop=file->writezerobytes/32768;
							long rest=file->writezerobytes-loop*32768;

							while(loop--)
								file->Save(zerobuffer,32768);

							if(rest>0)
								file->Save(zerobuffer,rest);
						}

						file->channelsampleswritten+=file->addzerosamples;
						file->writezerobytes=0;
						file->addzerosamples=0;
					}

					file->channelsampleswritten+=writesamples;
					file->Save(c->virtualbuffer+(writeoffset*samplesize),writesamples*samplesize);
				}
			}

			//	c->ResetMasterBuffers(AUDIOSTREAM_SONG);
		}

		c=c->NextChannel();
	}
#endif
}
#endif