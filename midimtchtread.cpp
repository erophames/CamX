#include "defines.h"
#include "songmain.h"
#include "MIDIoutproc.h"
#include "MIDIhardware.h"
#include "semapores.h"
#include "settings.h"
#include "object_song.h"
#include "object_project.h"
#include "MIDIoutdevice.h"
#include "audiohardware.h"
#include "audiodevice.h"

void MIDIStartThread::WaitLoop()
{
	double nextpulse_ms=-1;
	bool nextpulse_MIDIstart=false;
	bool nextpulse_MIDIprestart=false;

	while(IsExit()==false) // Signal Loop
	{
		WaitSignal((DWORD)nextpulse_ms);
		nextpulse_ms=-1;

		if(IsExit()==true) // Exit Signal ?
			break;

		mainMIDIalarmthread->Lock();
		if(Seq_Song *song=mainvar->GetActiveSong())
		{
			// MIDI Precounter ?
			if(nextpulse_MIDIprestart==true)
			{
				nextpulse_MIDIprestart=false;

				song->metronome.waitMIDIoffsetticks=0;
				song->startMIDIprecounter=true;
				mainMIDIalarmthread->SetSignal();
			}
			else
				if(song->pRepareMIDIstartprecounter==true) // MIDI Precounter Start
				{ 
					song->pRepareMIDIstartprecounter=false;
					song->startMIDIprecounter=true;
					nextpulse_ms=song->pRepareMIDIstartprecounterms;


					/*
					metronome.startpremetrosystime_samples[MIDIMETROINDEX]=maintimer->GetSystemTime(); // Sys Init Time

					double offsetsamples=device->GetOutputLatencySamples();
					offsetsamples/=timetrack.ppqsampleratemul; // Project Samplingrate -> Offset Intern
					metronome.waitMIDIoffsetticks=offsetsamples;

					mainMIDIalarmthread->SetSignal(); // Start MIDI

					*/

					nextpulse_MIDIprestart=true;

				}
				else // MIDI Start
				{
					if(nextpulse_MIDIstart==true)
					{
						nextpulse_MIDIstart=false;

						// Start MIDI+Signal MIDI
						song->SetRunningFlag(-1,false);
						song->startMIDIinit=true;	
						mainMIDIalarmthread->SetSignal();
					}
					else
					{
						if(song->pRepareMIDIstartdelay==true)
						{
							song->pRepareMIDIstartdelay=false;

							if(AudioDevice *device=mainaudio->GetActiveDevice())
							{
								double ms_delay=mainaudio->ConvertSamplesToMs(device->GetOutputLatencySamples());

								if(ms_delay<1)
								{
									song->SetRunningFlag(-1,false);
									song->startMIDIinit=true;	
									mainMIDIalarmthread->SetSignal();
								}
								else
								{
									nextpulse_ms=ms_delay;
									nextpulse_MIDIstart=true;
								}
							}

						}
					}
				}
		}

		mainMIDIalarmthread->Unlock();
	}

	ThreadGone();
}

PTHREAD_START_ROUTINE MIDIStartThread::MIDIStartFunc(LPVOID pParam)
{
	MIDIStartThread *mstartthread=(MIDIStartThread *)pParam;
	mstartthread->WaitLoop();
	return 0;
}

int MIDIStartThread::StartThread()
{
	int error=0;

	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MIDIStartFunc,(LPVOID)this, 0,0);

	if(!ThreadHandle)
		error++;

	//if(ThreadHandle)
	//	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);
	//	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_TIME_CRITICAL); // Best Priority
	//	else
	//		error=1;

	return error;
}

void MIDIMTCThread::WaitLoop()
{
	double nextpulse_ms=-1;	

	while(IsExit()==false) // Signal Loop
	{
		WaitSignal((DWORD)nextpulse_ms);
		nextpulse_ms=-1;

		if(IsExit()==true) // Exit Signal ?
			break;		

		mainMIDIalarmthread->Lock();

		if(Seq_Song *asong=mainvar->GetActiveSong())
		{
			if(
				(asong->status&(Seq_Song::STATUS_PLAY|Seq_Song::STATUS_RECORD)) && 
				asong->MIDIsync.sync==SYNC_INTERN &&
				asong->MIDIsync.sendmtc==true &&
				startmtc==true
				)
			{
				for(int i=0;i<MAXMIDIPORTS;i++)
				{
					if(mainMIDI->MIDIoutports[i].visible==true && 
						mainMIDI->MIDIoutports[i].outputdevice && 
						mainMIDI->MIDIoutports[i].outputdevice->lockmtc==false && 
						mainMIDI->MIDIoutports[i].sendmtc==true)
					{
						MIDIOutputDevice *outdev=mainMIDI->MIDIoutports[i].outputdevice;

						switch(asong->project->standardsmpte)
						{
						case Seq_Pos::POSMODE_SMPTE_24:
							nextpulse_ms=1000/(24*4);
							break;

						case Seq_Pos::POSMODE_SMPTE_25:
							nextpulse_ms=1000/(25*4);
							break;

						case Seq_Pos::POSMODE_SMPTE_2997:
							nextpulse_ms=1000/(29.97*4);
							break;

						case Seq_Pos::POSMODE_SMPTE_30:
							nextpulse_ms=1000/(30*4);
							break;

						default:
							goto nosend;
							break;
						}

						outdev->sendSongPosition_MTCRealtime(asong);
						outdev->mtcqf.MTC_AddQuarterFrame(asong);
						outdev->lockmtc=true;
					}
				}

nosend:
				// Unlock MTC
				MIDIOutputDevice *mo=mainMIDI->FirstMIDIOutputDevice();

				while(mo)
				{
					mo->lockmtc=false;
					mo=mo->NextOutputDevice();
				}

			}
		}

		mainMIDIalarmthread->Unlock();
	}

	ThreadGone();
}

PTHREAD_START_ROUTINE MIDIMTCThread::MIDIOutMTCFunc(LPVOID pParam)
{
	MIDIMTCThread *mtcproc=(MIDIMTCThread *)pParam;
	mtcproc->WaitLoop();
	return 0;
}

int MIDIMTCThread::StartThread()
{
	int error=0;

	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MIDIOutMTCFunc,(LPVOID)this, 0,0);

	if(!ThreadHandle)
		error++;

	//if(ThreadHandle)
	//	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);
	//	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_TIME_CRITICAL); // Best Priority
	//	else
	//		error=1;

	return error;
}

