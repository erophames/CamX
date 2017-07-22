#include "defines.h"

#include "songmain.h"
#include "audiofile.h" // dummy
#include "audiohdfile.h"
#include "object.h"
#include "audiohardware.h"
#include "audiothread.h"
#include "semapores.h"
#include "gui.h"

// Creates Audiopeakfiles in Background
PTHREAD_START_ROUTINE AudioWorkFileThread::AudioWorkFileCreator(LPVOID pParam) 
{
	AudioWorkFileThread *thread=(AudioWorkFileThread *)pParam;

	AudioHDFile refresh;
	// AudioHDFile *af;

	while(thread->IsExit()==false) // Signal Loop
	{	
		thread->WaitSignal();

		if(thread->IsExit()==true)
			break;
	
		mainthreadcontrol->Lock(CS_audiowork);

		AudioWork *w=thread->FirstWork();
		mainthreadcontrol->Unlock(CS_audiowork);

		while(w)
		{
			if(mainvar->exitthreads==false && w->stopped==false)
				w->Start();

			mainthreadcontrol->Lock(CS_audiowork);
			w=thread->DeleteWork(w);
			mainthreadcontrol->Unlock(CS_audiowork);
		}

	}// while

	// Delete Work rest
	mainthreadcontrol->Lock(CS_audiowork);

	AudioWork *w=thread->FirstWork();

	while(w)
		w=thread->DeleteWork(w);

	AudioWorkedFile *wf=thread->FirstWorkedFile();

	while(wf)
		wf=thread->DeleteWorkedFile(wf);

	mainthreadcontrol->Unlock(CS_audiowork);

	thread->ThreadGone();

	return 0;
}

int AudioWorkFileThread::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioWorkFileCreator,(LPVOID)this, 0,0);

	if(!ThreadHandle)
		error++;

	//if(ThreadHandle)SetThreadPriority(ThreadHandle,THREAD_PRIORITY_NORMAL); // Best Priority
	//else
	//	error=1;
#endif

	return error;
}

void AudioWorkFileThread::AddWork(AudioWork *work)
{
	mainthreadcontrol->Lock(CS_audiowork);

	audioworklist.AddEndO(work);

	mainthreadcontrol->Unlock(CS_audiowork);

	audioworkthread->SetSignal();
	//mainthreadcontrol->SendAudioWorkFileSignal(); // send start signal
}

AudioWork *AudioWorkFileThread::DeleteWork(AudioWork *w)
{
	AudioWork *n=(AudioWork *)w->next;

	if(w->dontdelete==false)
	{
		w->DeleteWork();
		audioworklist.RemoveO(w);
	}
	else
		audioworklist.CutObject(w);

	return n;
}

void AudioWorkFileThread::AddWorkedFile(AudioWorkedFile *work)
{
	mainthreadcontrol->Lock(CS_audiowork);

	workedfiles.AddEndO(work);

	mainthreadcontrol->Unlock(CS_audiowork);
}

AudioWorkedFile *AudioWorkFileThread::DeleteWorkedFile(AudioWorkedFile *w)
{
	AudioWorkedFile *n=(AudioWorkedFile *)w->next;

	if(w->filename)
		delete w->filename;

	if(w->createnewfile)
		delete w->createnewfile;

	workedfiles.RemoveO(w);

	return n;
}

int AudioWorkFileThread::StopAllProcessing()
{
	mainthreadcontrol->Lock(CS_audiowork);

	AudioWork *w=FirstWork();

	while(w)
	{
		w->Stop();

		w=w->NextWork();
	}

	mainthreadcontrol->Unlock(CS_audiowork);


	return 0;
}

bool AudioWorkFileThread::CheckIfWorkPossible(AudioHDFile *file,int type)
{
	if(file)
	{
		mainthreadcontrol->Lock(CS_audiowork);
		AudioWork *w=FirstWork();

		while(w)
		{
			if(w->CheckHDFile(file,type)==false) // Call V Function
			{
				bool usage=true;

				mainthreadcontrol->Unlock(CS_audiowork);

				maingui->MessageBoxOk(0,"Audio File in use, Operation not possible");
				return false;
			}

			w=w->NextWork();
		}

		mainthreadcontrol->Unlock(CS_audiowork);
	}

	return true;
}
