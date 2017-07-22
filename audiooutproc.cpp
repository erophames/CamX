#include "defines.h"

#include "songmain.h"
#include "audiorecord.h"
#include "audiofile.h" // dummy
#include "audiorealtime.h"
#include "semapores.h"
#include "object_song.h"
#include "object_track.h"
#include "audiohardware.h"
#include "audiobus.h"
#include "runningaudiofile.h"
#include "MIDItimer.h"
#include "audioproc.h"
#include "gui.h"

#ifdef WIN32
#include "asio/asio.h"
#endif

PTHREAD_START_ROUTINE AudioRAFFunc(LPVOID pParam) // Core Thread low prio
{
	int threadid=(int)pParam;

	while(mainaudiostreamproc->rafcoreexit[threadid]==false) // Signal Loop
	{	
		mainaudiostreamproc->WaitRAFSignal(threadid);

		if(mainaudiostreamproc->rafcoreexit[threadid]==true)break;

		if(mainaudiostreamproc->rafsong)
		{
			RunningAudioFile *r;

			do{
				mainaudiostreamproc->rafsong->LockRAF();
				if(r=mainaudiostreamproc->raf)mainaudiostreamproc->raf=r->NextRunningFile();
				mainaudiostreamproc->rafsong->UnlockRAF();
				if(r)r->ReadRAF(mainaudiostreamproc->rafdevice);

			}while(r);
		}

		mainaudiostreamproc->DoneRAF(threadid);
	}

	mainaudiostreamproc->DoneRAF(threadid);

	return 0;
}

PTHREAD_START_ROUTINE AudioInputCoreFunc(LPVOID pParam) // Audio Input Core Thread high prio
{
	int threadid=(int)pParam;

	while(mainaudioinproc->inputcoreexit[threadid]==false) // Signal Loop
	{	
		mainaudioinproc->WaitCoreSignal(threadid);

		if(mainaudioinproc->inputcoreexit[threadid]==true)
			break;

		// Audio Core Work Loop
		if(Seq_Song *song=mainvar->GetActiveSong()){

			switch(mainaudioinproc->docorefunction)
			{
			case DOCORETRACKSINPUT:
				while(Seq_Track *uat=song->GetUsedAudioInputTrack())
					uat->DoAudioInput(song->audiosystem.device);
				break;

			case DOCORETRACKSINPUTEFFECTS:
				while(Seq_Track *uat=song->GetUsedAudioInputTrack())
					uat->DoAudioInputEffects(song->audiosystem.device);
				break;

			case DOCORETRACKSINPUTPEAK:
				while(Seq_Track *uat=song->GetUsedAudioInputTrack())
					uat->DoAudioInputPeak();
				break;
			}
		}

		mainaudioinproc->DoneCore(threadid);

	}// while exitthreads

	mainaudioinproc->DoneCore(threadid);

	return 0;
}


PTHREAD_START_ROUTINE AudioCoreFunc(LPVOID pParam) // Audio Output Core Thread high prio
{
	int threadid=(int)pParam;

	while(mainaudiostreamproc->coreexit[threadid]==false) // Signal Loop
	{	
		mainaudiostreamproc->WaitCoreSignal(threadid);

		if(mainaudiostreamproc->coreexit[threadid]==true)
			break;

		// Audio Core Work Loop
		if(Seq_Song *song=mainvar->GetActiveSong()){

			switch(mainaudiostreamproc->docorefunction)
			{
			case DOCORETRACKS:
				{
					while(Seq_Track *uat=song->GetUsedAudioTrack())
						uat->AddTrackToBusOrParent(song->audiosystem.device);
				}
				break;

			case DOCORECHANNEL:
				{
					while(AudioChannel *uac=song->GetUsedAudioChannel())
						uac->AddEffectToChannel(song->audiosystem.device);

				}break;

			case DOCOREBUS: // AudioSystem Bus
				{
					while(AudioChannel *uac=song->GetUsedAudioBus())
						uac->AddEffectToChannel(song->audiosystem.device);
				}
				break;

			case DOCORETRACKSTHRU:
				{
					while(Seq_Track *uat=song->GetUsedAudioThruTrack())
						uat->DoThru(song->audiosystem.device);
				}
				break;

			case DOCOREAUDIOTRACKTRACKIO:
				{
					while(Seq_Track *uat=song->GetUsedAudioTrackTrackIO())
						uat->DoTrackInputFromTrack();
				}
				break;

			}
		}

		mainaudiostreamproc->DoneCore(threadid);

	}// while exitthreads

	mainaudiostreamproc->DoneCore(threadid);

	return 0;
}

void AudioCoreAndStreamProc::WaitLoop()
{
	while(IsExit()==false){ // Signal Loop

		WaitSignal();

		if(IsExit()==true)
			break;

		Lock();

		if(AudioDevice *device=mainaudio->GetActiveDevice())
			if(Seq_Song *song=mainvar->GetActiveSong())
			{
				LONGLONG t1=maintimer->GetSystemTime();
				song->CreateAudioStream(device,0);
				LONGLONG t2=maintimer->GetSystemTime();

				double ms=maintimer->ConvertSysTimeToMs(t2-t1);

				LockTimerCheck();

				if(ms>timeforrefill_maxms)
					timeforrefill_maxms=ms;

				//	if(ms>mainaudiostreamproc->timeforrefill_ms)
				timeforrefill_ms=ms;

				UnlockTimerCheck();

				/*
				else
				{
				if(mainaudiostreamproc->timeforrefill_ms>0.05)
				mainaudiostreamproc->timeforrefill_ms-=0.05;
				else
				mainaudiostreamproc->timeforrefill_ms=0;
				}
				*/

			}

			Unlock();
	}

	ThreadGone();
}

PTHREAD_START_ROUTINE AudioStreamFunc(LPVOID pParam) // STream Creator
{	
	mainaudiostreamproc->WaitLoop();
	return 0;
}

void AudioCoreAndStreamProc::DoRAFS(Seq_Song *song,AudioDevice *device) // RAF refill
{
	raf=song->FirstRunningAudioFile();
	rafsong=song;
	rafdevice=device;

	if(raf && rafsong && rafdevice){
		SetRAFSignals(song->GetCountRunningAudioFiles()); // Start Signal Core Threads
		WaitAllRAFsFinished();
	}
}

void AudioCoreAndStreamProc::DoFunction(Seq_Song *song,int func,int usethreads)
{
	docorefunction=func;
	SetCoreSignals(usethreads); // Start Signal Core Threads
	WaitAllCoresFinished();
}

void AudioCoreAudioInputProc::DoFunction(Seq_Song *song,int func,int usethreads)
{
	docorefunction=func;
	SetCoreSignals(usethreads); // Start Signal Core Threads
	WaitAllCoresFinished();
}

int AudioCoreAndStreamProc::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioStreamFunc,(LPVOID)this, 0,0); // Audio File Buffer Refill Thread

	if(ThreadHandle)SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);
	else error++;

	if(mainvar->cpucores==1)
	{
		coresinit=0;
		return error;
	}

	coresinit=mainvar->cpucores<=MAXCORES?mainvar->cpucores:MAXCORES;

	// Create X Cores Threads (2 Cores or more)
	for(int id=0;id<coresinit;id++)
	{
		CoreThreadEventHandle[id]=CreateEvent( 
			NULL,   // default security attributes
			FALSE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object

		if(!CoreThreadEventHandle[id])error++;

		Wait_CoreThreadEventHandle[id]=CreateEvent( 
			NULL,   // default security attributes
			FALSE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object

		if(!Wait_CoreThreadEventHandle[id])error++;

		CoreThreadHandle[id]=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioCoreFunc,(LPVOID)id, 0,0); // Audio File Buffer Refill Thread
		if(CoreThreadHandle[id])SetThreadPriority(CoreThreadHandle[id],THREAD_PRIORITY_ABOVE_NORMAL); // Above Main!
		else
			error++;

		Wait_RAFThreadEventHandle[id]=CreateEvent( 
			NULL,   // default security attributes
			FALSE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object

		if(!Wait_RAFThreadEventHandle[id])error++;

		RAFThreadEventHandle[id]=CreateEvent( 
			NULL,   // default security attributes
			FALSE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object

		if(!RAFThreadEventHandle[id])error++;

		RAFThreadHandle[id]=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioRAFFunc,(LPVOID)id, 0,0);
		if(!RAFThreadHandle[id])error++;

	}
#endif

	return error;
}

void AudioCoreAndStreamProc::StopThread()
{
	// Stop Playback
	mainthreadcontrol->Lock(CS_audioplayback);
	mainaudiostreamproc->Lock();

	maingui->SetInfoWindowText("AudioCoreAndStreamProc SQ");

	SendQuit();

	maingui->SetInfoWindowText("AudioCoreAndStreamProc SC");

	// Bye Bye Core Threads...
	for(int i=0;i<MAXCORES;i++)
		coreexit[i]=true;

	SetCoreSignals(coresinit);

	maingui->SetInfoWindowText("AudioCoreAndStreamProc SR");

	for(int i=0;i<MAXCORES;i++)
		rafcoreexit[i]=true;
	SetRAFSignals(coresinit);

	maingui->SetInfoWindowText("AudioCoreAndStreamProc WC");
	WaitAllCoresFinished();

	maingui->SetInfoWindowText("AudioCoreAndStreamProc WR");
	WaitAllRAFsFinished();

	// Reset Core Handles
	for(int i=0;i<coresinit;i++)
	{
		if(CoreThreadEventHandle[i])CloseHandle(CoreThreadEventHandle[i]); 
		if(RAFThreadEventHandle[i])CloseHandle(RAFThreadEventHandle[i]);
		if(Wait_CoreThreadEventHandle[i])CloseHandle(Wait_CoreThreadEventHandle[i]); 
		if(Wait_RAFThreadEventHandle[i])CloseHandle(Wait_RAFThreadEventHandle[i]);
	}

	mainaudiostreamproc->Unlock();
	mainthreadcontrol->Unlock(CS_audioplayback);
}

int AudioCoreAudioInputProc::StartThread()
{
	int error=0;

#ifdef WIN32

	/*
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioInputFunc,(LPVOID)this, 0,0); // Audio File Buffer Refill Thread

	if(ThreadHandle)SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);
	else error++;
	*/

	if(mainvar->cpucores==1)
	{
		coresinit=0;
		return error;
	}

	coresinit=mainvar->cpucores<=MAXCORES?mainvar->cpucores:MAXCORES;

	// Create X Cores Threads (2 Cores or more)
	for(int id=0;id<coresinit;id++)
	{
		CoreThreadEventHandle[id]=CreateEvent( 
			NULL,   // default security attributes
			FALSE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object

		if(!CoreThreadEventHandle[id])error++;

		Wait_CoreThreadEventHandle[id]=CreateEvent( 
			NULL,   // default security attributes
			FALSE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object

		if(!Wait_CoreThreadEventHandle[id])error++;

		CoreThreadHandle[id]=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioInputCoreFunc,(LPVOID)id, 0,0); // Audio In Thread
		if(CoreThreadHandle[id])SetThreadPriority(CoreThreadHandle[id],THREAD_PRIORITY_ABOVE_NORMAL); // Above Main!
		else
			error++;
	}
#endif

	return error;
}

void AudioCoreAudioInputProc::StopThread()
{
	// Stop Playback
	mainthreadcontrol->Lock(CS_audioinput);
	mainaudiostreamproc->Lock();

	//SendQuit();

	// Bye Bye Core Threads...
	for(int i=0;i<coresinit;i++)
		inputcoreexit[i]=true;
	SetCoreSignals(coresinit);
	WaitAllCoresFinished();

	// Reset Core Handles
	for(int i=0;i<coresinit;i++)
	{
		if(CoreThreadEventHandle[i])CloseHandle(CoreThreadEventHandle[i]); 
		if(Wait_CoreThreadEventHandle[i])CloseHandle(Wait_CoreThreadEventHandle[i]); 
	}

	mainaudiostreamproc->Unlock();
	mainthreadcontrol->Unlock(CS_audioinput);
}