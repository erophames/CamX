//#include "defines.h"
#include "object.h"
#include "audiodevice.h"
#include "audiofile.h"
#include "audiorealtime.h"
#include "audiochannel.h"
#include "semapores.h"
#include "songmain.h"

#include "gui.h"
#include "object_song.h"
#include "audiohardware.h"
#include "object_track.h"
#include "sampleeditor.h"

#include "audiohdfile.h"
#include "crossfade.h"


void mainAudioRealtime::StopAllRealtimeEvents() // lock audio devices
{	
	LockRTObjects();

	AudioRealtime *c=FirstAudioRealtime();
	while(c)c=RemoveAudioRealtime(c,true);

	UnlockRTObjects();
}

bool mainAudioRealtime::FindAudioRealtime(AudioRealtime *ar)
{
	LockRTObjects();

	AudioRealtime *c=FirstAudioRealtime();

	while(c){

		if(c==ar)
		{
			UnlockRTObjects();
			return true;
		}

		c=c->NextAudioRealtime();
	}

	UnlockRTObjects();
	return false;
}

AudioRealtime *mainAudioRealtime::RemoveAudioRealtime(AudioRealtime *ar,bool signal)
{
	if(!ar)
		return 0;

	LockRTObjects();

	AudioRealtime *n=ar->NextAudioRealtime();

	bool remove;

	if(ar->staticrealtime==false)
	{
		// Files, Close etc..
		remove=true;
	}
	else
	{
		// Static  - Metro etc..
		remove=false;
	}

	if(ar->addedtolist==true)
		realtimeaudiofiles.CutObject(ar);

	UnlockRTObjects();

	ar->DeInit(signal,remove);

	if(remove==true)
		delete ar;

	return n;
}

void mainAudioRealtime::RemoveAllAudioRealtime(AudioChannel *chl) // lock audio realtime
{
	LockRTObjects();

	AudioRealtime *c=FirstAudioRealtime();

	while(c)
	{
		if(c->audiochannel==chl || (!chl))
			c=RemoveAudioRealtime(c,true);
		else
			c=c->NextAudioRealtime();
	}

	UnlockRTObjects();
}

void AudioRealtime::InitClass(Seq_Song *s,TrackHead *achl,AudioHDFile *hdfile,AudioRegion *r,bool aclose,int off)
{
	audiochannel=achl;
	audiopattern.audioevent.audioefile=hdfile;

	if(r)
	{
		if(audiopattern.audioevent.audioregion=new AudioRegion)
		{
			r->CloneTo(audiopattern.audioevent.audioregion);
			audiopattern.audioevent.audioregion->InitRegion();
		}
	}

	offset=off;

	song=s;
	
	startwindow=0;
	endstatus=0;

	autoclose=aclose;

	usetrack=0;
	stream=0;

	streamreadc=streamwritec=0;

	endreached=closeit=staticrealtime=addedtolist=false;
}

AudioRealtime::AudioRealtime(Seq_Song *s,TrackHead *achl,AudioHDFile *hdfile,AudioRegion *r,bool aclose,int off)
{
	InitClass(s,achl,hdfile,r,aclose,off);
}

void AudioRealtime::Init(AudioDevice *device,AudioHDFile *afile)
{
	if(!device)
		return;

	stream=new AudioHardwareBuffer[nrstream=device->numberhardwarebuffer]; // min 1 buffer

	if(!stream)
		return;

	double bufferms=mainaudio->ConvertSizeToMs(mainaudio->GetGlobalSampleRate(),device->GetSetSize());
	if(bufferms<=0)
		bufferms=0.1;

	for(int i=0;i<nrstream;i++)
	{
		stream[i].InitARESOut(device->GetSetSize(),afile->channels);
		stream[i].SetBuffer(afile->channels,device->GetSetSize());
		/*
		art->stream[i].channelsinbuffer=afile->channels;
		art->stream[i].samplesinbuffer=device->setSize;
		art->stream[i].samplesinbuffer_size=art->stream[i].samplesinbuffer*sizeof(ARES);
		*/
		stream[i].bufferms=bufferms;

		stream[i].delayvalue=(int)floor((1000/bufferms)+0.5);

		if(!stream[i].outputbufferARES)
		{
			delete stream;
			stream=0;
			return;
		}

	}// for

	audiobuffer.Create32BitBuffer_Input(afile->channels,device->GetSetSize());

#ifdef DEBUG
	if(!audiopattern.audioevent.audioefile)
		maingui->MessageBoxError(0,"AR AUDIOEFILE");
#endif
}

void AudioRealtime::DeInit(bool signal,bool freememory)
{
	if(audiopattern.audioevent.audioefile)
	{
		if(audiopattern.audioevent.audioefile->ramfile==true)
		{
			// RAM FILE
			//	MessageBeep(-1);
		}
		else
		{
			// HD FILE
			audiopattern.audioevent.iofile.Close(true);

			if(endstatus)
				*endstatus=true; // Signal Editor, end of file

			if(signal==true && startwindow)
				maingui->SendGUIMessage(MESSAGE_REFRESHSAMPLEEDITOR,startwindow);
		}

		// Region Auto Delete
		if(audiopattern.audioevent.audioregion && audiopattern.audioevent.audioregion->autODeInit==true){

			audiopattern.audioevent.audioregion->FreeMemory();
			delete audiopattern.audioevent.audioregion;
			audiopattern.audioevent.audioregion=0;
		}
	}

	if(freememory==true)
	{
		if(stream)
		{
			for(int i=0;i<nrstream;i++)
				delete stream[i].outputbufferARES;

			delete stream;
			stream=0;
		}

		audiobuffer.Delete32BitBuffer();
	}

}

void AudioRealtime::SeekRealtime()
{
	if(stream)
		stream[streamwritec].channelsused=0; // reset

	if(audiopattern.audioevent.audioefile->ramfile==true) // RAM FILE
	{
		/*
		// HD FILE 
		AudioRAMFile *arf=(AudioRAMFile *)audiopattern.audioevent.audiohdfile;

		rbytes=arf->S(&audiobuffer,ramposition);
		ramposition+=rbytes;
		*/
	}
	else
	{		// HD FILE 
		bool ok=audiopattern.SeekSamplesCurrent(audiobuffer.samplesinbuffer); // read to input buffer		
		audiobuffer.endreached=ok==true?false:true;
	}

	endreached=audiobuffer.endreached;

	if(streamwritec==nrstream-1)
		streamwritec=0;
	else
		streamwritec++;
}

void AudioRealtime::ReadRealtime()
{
	LONGLONG fs=audiopattern.audioevent.sampleposition;

	//TRACE ("ReadRealtime %d\n",fs);

	Seq_CrossFade *cf=audiopattern.audioevent.audioregion?audiopattern.audioevent.audioregion->crossfade:0;

	rbytes=0;

	if(audiopattern.audioevent.audioefile->ramfile==true) // RAM FILE
	{
		AudioRAMFile *arf=(AudioRAMFile *)audiopattern.audioevent.audioefile;

		rbytes=arf->FillAudioBuffer(&audiobuffer,ramposition,offset);
		ramposition+=rbytes;
	}
	else
		rbytes=audiopattern.FillAudioBuffer(song,&audiobuffer); //  HD FILE  read to input buffer

	offset=0;
	endreached=audiobuffer.endreached;

	if(stream)
		stream[streamwritec].channelsused=0; // reset

	if(rbytes && stream)
	{		
		audiopattern.audioevent.audioefile->ConvertReadBufferToSamples // convert input buffer to ARES
			(
			audiobuffer.inputbuffer32bit,
			&stream[streamwritec],
			audiobuffer.samplesinbuffer,
			audiobuffer.channelsinbuffer
			);

		// Add CrossFade
		if(cf && cf->CheckIfInRange_File(fs,fs+audiobuffer.samplesinbuffer)==true)
		{
			double h=fs-cf->from_sample_file,h2=cf->to_sample_file-cf->from_sample_file;
			ARES v=cf->ConvertToVolume(h/h2,cf->infade);

			if(v<1)
			{
				ARES *s=stream[streamwritec].outputbufferARES;
				int i=stream[streamwritec].samplesinbuffer*stream[streamwritec].channelsinbuffer;

				while(i--)*s++ *=v;
			}
		}
	}

	if(streamwritec==nrstream-1)
		streamwritec=0;
	else
		streamwritec++;
}

void mainAudioRealtime::RefreshSignal()
{
#ifdef DEBUG
	bool showzero=false;

	if(FirstAudioRealtime())
	{
		TRACE ("Realtime %d\n",realtimeaudiofiles.GetCount());
		showzero=true;
	}
#endif

	// Close...

	LockRTObjects();

	{
		AudioRealtime *art=FirstAudioRealtime();

		while(art)
		{
			if(art->closeit==true && art->autoclose==true)
				art=RemoveAudioRealtime(art,true);
			else
				art=art->NextAudioRealtime();
		}
	}

	UnlockRTObjects();

#ifdef DEBUG
	if(showzero==true)
	{
		if(!FirstAudioRealtime())
		{
			TRACE ("++++ 0 Realtime +++++ \n");
		}
	}
#endif

	// Play
	if(mainaudio->GetActiveDevice())
	{
		LockRTObjects();

		AudioRealtime *art=FirstAudioRealtime();

		while(art){
			AudioRealtime *n=art->NextAudioRealtime();

			if(art->endreached==false)
				PlayRealtimeFile(art);

			art=n;
		}

		UnlockRTObjects();
	}
}

bool mainAudioRealtime::PlayRealtimeFile(AudioRealtime *art)
{
	if(art->endreached==true)
	{
		if(art->autoclose==true)
			RemoveAudioRealtime(art,false);

		return false;
	}

	/*
	// Offset ?
	if(art->offset)
	{
	art->InitOffset(art->offset);				
	art->offset=0;
	}
	*/

	//art->newadded=false; // reset INIT Flag

	// Init StartBuffer
	//	if((!art->audiochannel) ||  (art->audiochannel->Muted()==false))
	art->ReadRealtime();
	//	else
	//		art->SeekRealtime();

	if(art->endstatus && art->endreached==true)
		*art->endstatus=true;

	return true;
}

void mainAudioRealtime::AddAudioRealtime_AR(Seq_Song *song,AudioDevice *device,TrackHead *channel,Seq_Track *usetrack,AudioRealtime *art,AudioHDFile *afile,AudioRegion *region,bool *endstatus,int offset,bool autoclose,guiWindow *from) // lock audio realtime
{
#ifdef DEBUG
	if(offset>=device->GetSetSize())
	{
		maingui->MessageBoxError(0,"AddAudioRealtime Offset");
	}
#endif

	if(song && device && art && afile && channel && offset<device->GetSetSize()){

		// Avoid double Playback
		LockRTObjects();

		AudioRealtime *ar=FirstAudioRealtime();

		while(ar)
		{
			if(ar==art)
			{
				UnlockRTObjects();
				return;
			}

			ar=ar->NextAudioRealtime();
		}

		UnlockRTObjects();

		bool ok=false;

		// Reset
		art->prev=art->next=0;
		art->audiobuffer.endreached=art->endreached=false;
		art->streamreadc=art->streamwritec=0;
		art->addedtolist=false;

		art->startwindow=from;
		art->usetrack=usetrack;

		if(art->endstatus=endstatus)
			*endstatus=false;

		if(afile->ramfile==true) // RAM File
		{
			AudioRAMFile *arf=(AudioRAMFile *)afile;

			art->ramposition=arf->ramdata; // Init to start of data

			if(arf->ramdata)
				ok=true;
		}
		else
		{
			if(art->audiopattern.OpenAudioPattern()==true) // HD File
				ok=true;
#ifdef _DEBUG
			else
				MessageBox(NULL,"Add Audio Realtime Error 1","Error",MB_OK);
#endif
		}

		if(ok==false)
			return;

		if(art->audiobuffer.inputbuffer32bit){

			// Input Buffer ok

			if(PlayRealtimeFile(art)==true) // Init
			{
				LockRTObjects();
				realtimeaudiofiles.AddEndO(art);
				art->addedtolist=true;
				UnlockRTObjects();
			}

		}
	}
#ifdef DEBUG
	else
		maingui->MessageBoxError(0,"Realtime Audio");
#endif

	TRACE("Audio Realtimes %d\n",realtimeaudiofiles.GetCount());
}

AudioRealtime *mainAudioRealtime::AddAudioRealtime(Seq_Song *song,AudioDevice *device,TrackHead *channel,Seq_Track *usetrack,AudioHDFile *afile,AudioRegion *region,bool *endstatus,int offset,bool autoclose,guiWindow *from) // lock audio realtime
{
#ifdef DEBUG
	if(offset>=device->GetSetSize())
	{
		maingui->MessageBoxError(0,"AddAudioRealtime Offset");
	}

	if(!afile)
		maingui->MessageBoxError(0,"AddAudioRealtime AFile");
#endif

	if(song && device && afile && channel && offset<device->GetSetSize()){

		if(AudioRealtime *art=new AudioRealtime(song,channel,afile,region,autoclose,offset))
		{
			art->Init(device,afile);

			if(!art->stream)
			{
				delete art;
				return 0;
			}

			bool ok=false;

			art->startwindow=from;
			art->usetrack=usetrack;

			if(art->endstatus=endstatus)
				*endstatus=false;

			if(afile->ramfile==true) // RAM File
			{
				AudioRAMFile *arf=(AudioRAMFile *)afile;

				art->ramposition=arf->ramdata; // Init to start of data

				if(arf->ramdata)
					ok=true;
			}
			else
			{
				if(art->audiopattern.OpenAudioPattern()==true) // HD File
					ok=true;
#ifdef _DEBUG
				else
					MessageBox(NULL,"Add Audio Realtime Error 2","Error",MB_OK);
#endif
			}

			if(ok==false)
			{
				delete art;
				return 0;
			}

			if(art->audiobuffer.inputbuffer32bit){

				// Input Buffer ok
				if(PlayRealtimeFile(art)==true) // Init
				{
					LockRTObjects();
					realtimeaudiofiles.AddEndO(art);
					art->addedtolist=true;
					UnlockRTObjects();
				}

				return art;
			}

			//Error
			art->DeInit(false,true);
			delete art;

			return 0;

		}//if art
	}
#ifdef DEBUG
	else
		maingui->MessageBoxError(0,"Realtime Audio");
#endif

	return 0;
}

bool mainAudioRealtime::StopRealtimePlayback(AudioRealtime *d) // lock audio realtime
{
	bool ok=false;

	//	mainthreadcontrol->Lock(CS_audiorealtime);

	/*
	if(d->audiochannel && 
	d->audiochannel->GetHardwarePlaybackChannel() &&
	d->audiochannel->GetHardwarePlaybackChannel()->channel_running==true
	)
	{
	d->canbeclose=
	d->autoclose=true; // let Realtime Thread close File
	}
	else
	{
	*/
	d->audiobuffer.Delete32BitBuffer();

	d=RemoveAudioRealtime(d,false);

	if(!d)
	{
		// Check if device can be stopped...
	}
	// }

	// mainthreadcontrol->Unlock(CS_audiorealtime);

	return ok;
}

void mainAudioRealtime::RemoveTrackFromAudioRealtime(Seq_Track *t,bool forcedelete)
{
	if(!t)return;

	LockRTObjects();

	AudioRealtime *ar=FirstAudioRealtime();

	while(ar)
	{
		AudioRealtime *arn=ar->NextAudioRealtime();

		if(t->IsTrackChildOfTrack(ar->usetrack)==true)
		{
			if(forcedelete==true)
				StopRealtimePlayback(ar);
			else
				ar->usetrack=0;
		}

		ar=arn;
	}

	UnlockRTObjects();
}

void AudioRealtimeThread::WaitLoop()
{
	while(IsExit()==false)
	{
		WaitSignal(); // Wait for incoming Signal

		if(IsExit()==true)
			break;

		mainthreadcontrol->Lock(CS_audiorealtime);
		mainaudioreal->RefreshSignal();
		mainthreadcontrol->Unlock(CS_audiorealtime);
	}

	ThreadGone();
}

PTHREAD_START_ROUTINE AudioRealtimeThread::AudioRealtimeThreadFunc(LPVOID pParam)
{
	AudioRealtimeThread *art=(AudioRealtimeThread *)pParam;
	art->WaitLoop();
	return 0;	
}

int AudioRealtimeThread::StartThread()
{
	int error=0;
#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioRealtimeThreadFunc,(LPVOID)this, 0,0);

	if(!ThreadHandle)
		error++;

	/*
	if(ThreadHandle)
	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);

	//	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_TIME_CRITICAL); // Best Priority
	else
	error=1;
	*/

#endif

	return error;
}

void AudioRealtimeThread::StopThread()
{
	mainthreadcontrol->Lock(CS_audiorealtime);
	SendQuit();
	mainthreadcontrol->Unlock(CS_audiorealtime);
}
