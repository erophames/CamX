#include "defines.h"
#include "songmain.h"
#include "audiorecord.h"
#include "audiofile.h" // dummy
#include "audiorealtime.h"
#include "semapores.h"
#include "object_project.h"
#include "object_song.h"
#include "audiohardware.h"
#include "audiohdfile.h"
#include "object_track.h"
#include "settings.h"
#include "initplayback.h"
#include "gui.h"
#include "audiodevice.h"

#ifdef WIN32
#include "asio/asio.h"
#endif

PTHREAD_START_ROUTINE AudioRecordThread::AudioRecordingBufferThread(LPVOID pParam)
{
	AudioRecordThread *thread=(AudioRecordThread *)pParam;

	while(thread->IsExit()==false) // Signal Loop
	{
		thread->WaitSignal();
		if(thread->IsExit()==true)break;	// Exit ?
		thread->mainart->WriteRecordingFiles(false);
		thread->ThreadGone();
	}

	thread->ThreadGone();

	return 0;
}

int AudioRecordThread::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioRecordingBufferThread,(LPVOID)this, 0,0);

	if(!ThreadHandle)
		error++;

	//if(ThreadHandle)
	//	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_NORMAL);
	//else
	//	error=1;
#endif

	return error;
}

void AudioHDFile::WriteAudioRecording(AudioDevice *device,Seq_Song *song)
{
	if(recordingactive==true && recended==false && writefile.errorwriting==false)
	{
		if( (song->record_audioindex==RECORD_INDEX_DEVICE && istrackrecordingfile==false) ||
			(song->record_audioindex==RECORD_INDEX_TRACKTRACK && istrackrecordingfile==true)
			)
		{
			recmix.channelsused=0; // reset
			recordingadded=false;

			if(recordingtrack){

				recordingtrack->t_audiofx.t_inputbuffers[recordingtrack->t_audiofx.t_audioinputreadindex].CopyAudioBuffer(&recmix);	

				if(recordingactive==true)
				{
					CheckAndMixAudioRecordingBuffer(song);
					WriteRecordingFile(device,song); // Write Buffer to File + Offsets
				}
			}
		}
	}
}

void AudioHDFile::WriteRecordingFile(AudioDevice *device,Seq_Song *song)
{
	if(recordingadded==true) // Create RAW L/R etc.. ->rec->inputbuffer
	{
		ConvertARESToRecordingFormat(); // [][]->L/R + Punch + Offset

		/*
		[+++++] ++++=rec->recordingsize, NO Offset always 0....
		*/

		bool cycle=song->playbacksettings.cycleplayback;

		if(reccycleloopcounter==0) // PUNCH OUT Check
		{
			recordedsamples+=recordingsize;

			if(song->punchrecording&Seq_Song::PUNCHIN)
			{
				if(cycle==false) // cycle == no cut
				{
					LONGLONG se=recordpattern->GetSampleStart(song)+recordedsamples;

					if(se>song->playbacksettings.cycle_sampleend)
					{
						LONGLONG cut=se-song->playbacksettings.cycle_sampleend;

						recordingsize-=(int)cut;

#ifdef DEBUG
						if(recordingsize<0)
							maingui->MessageBoxError(0,"rec->recordingsize <0 PUNCH");
#endif

						recended=true; // stop audio recording next buffer
					}
				}
			}
			else
				if(song->punchrecording&Seq_Song::PUNCHOUT)
				{
					if(recordpattern->punch1==true)
					{
						LONGLONG se=recordpattern->GetSampleStart(song)+recordedsamples;

						if(se>song->playbacksettings.cycle_samplestart)
						{
							LONGLONG cut=se-song->playbacksettings.cycle_samplestart;
							recordingsize-=(int)cut;

#ifdef DEBUG
							if(recordingsize<0)
								maingui->MessageBoxError(0,"rec->recordingsize <0 PUNCH OUT");
#endif
							recended=true; // stop audio recording next buffer
						}
					}
				}
		}

		if(cycle==true){

			LockIOSync(); // Write

			if(seekbeforewrite>=0){
				// Mix

				//TRACE ("seekbeforewrite %d\n",rec->seekbeforewrite);

				writefile.SeekBegin(seekbeforewrite);
				seekbeforewrite=-1; // Reset
			}
			else
				writefile.SeekEnd(0);
		}

		// No Offset, Offset done by ConvertARESToRecordingFormat
		//TRACE ("WBS %d\n",writebuffersize);

#ifdef DEBUG
		if(recmix.samplesinbuffer!=device->GetSetSize())
			maingui->MessageBoxError(0,"REC SetSize");
#endif

		if(recordingsize>0)
		{
			Save(recmix.inputbuffer32bit,recordingsize*samplesize_all_channels); // Save L/R Mix to File

			//rec->writefile.Flush();

			if(writefile.errorwriting==false) // Write OK ? or Disk full etc. ?
			{
				if(reccycleloopcounter==0){

					LONGLONG newsize=samplesperchannel+recordingsize;

					samplesperchannel=newsize;
					datalen=newsize*samplesize_all_channels;
					dataend=datastart+datalen;

					if(cycle==true)
						UnlockIOSync();

					CreateRecordPeak(song->record_sampleoffset,recordingsize,4);

					//	if(rec->recordingflag&RAD_CYCLERESET)
					//		rec->CloseRecordPeak();
				}
				else{

					// Cycle Recording Write...

					if(cycle==true)
						UnlockIOSync();

					MixRecordPeak(song->record_sampleoffset,recordingsize);
				}
			}
			else{
				if(cycle==true)
					UnlockIOSync();
			}
		}
	}
}


void AudioRecordMainThread::WriteRecordingFiles(bool singlethread) // Write AudioBuffers -> Files
{
	AudioDevice *device=mainaudio->GetActiveDevice();

	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"Audio Device"); 
#endif

		return;
	}

	Seq_Song *song=mainvar->GetActiveSong();
	if(!song)return;
	
	AudioHDFile *rec=singlethread==true?mainaudio->FirstAudioRecordingFile():audiorecordthread->GetAudioRecordingFile();

	while(rec){

		rec->WriteAudioRecording(device,song);

		rec=singlethread==true?rec->NextHDFileNoLock():audiorecordthread->GetAudioRecordingFile(); // Single/Multi Core Recording
	}
}

void Seq_Song::WriteAudioRecording(AudioDevice *device)
{
	for(int i=0;i<2;i++) //RECORD_INDEX_DEVICE || RECORD_INDEX_TRACKTRACK
	{
		bool write;

		record_audioindex=i;
		record_sampleoffset=0; // Cycle Offset Buffer Cut 
		record_samplesize=record_setSize=device->GetSetSize();

		if(i==RECORD_INDEX_DEVICE)
		{
			recordedsamples+=device->GetSetSize();

			if(!(device->status&AudioDevice::STATUS_INPUTOK))
			{
				write=true; // No In Latency
			}
			else
			{
				write=false;

				if(checkinlatency==true)
				{
					LONGLONG waitlatency=device->GetInputLatencySamples();

					if(recordedsamples>=waitlatency)
					{
						int offset=recordedsamples-waitlatency;
						int writeoffset=device->GetSetSize()-offset;

						checkinlatency=false;
						write=true;

						record_sampleoffset=writeoffset; // Latency Offset
						record_setSize-=writeoffset;
						record_samplesize-=writeoffset;
					}
				}
				else
				{
					// Device without Latency
					write=true; // No In Latency
				}
			}
		}
		else
			write=true; // Track->Track

		if(record_setSize==0)
			return;

		// Write, if Latencyok or Audio Track Recording from Track...

		int refillflag=0;

cycleloop:
		record_sampleendposition[i]=record_sampleposition[i]+record_setSize; // Init Recording Positions

		if(playbacksettings.cycleplayback==true && record_sampleendposition[i]>=playbacksettings.cycle_sampleend)
		{
			record_samplesize=(int)((record_sampleendposition[i]=playbacksettings.cycle_sampleend)-record_sampleposition[i]);
			refillflag = RAD_CYCLERESET;
		}

		record_setSize-=record_samplesize;

		{ // New recording Audio Pattern
			Seq_Track *t=FirstTrack();

			while(t)
			{

				if(t->record==true &&
					t->recordtracktype==TRACKTYPE_AUDIO &&

					( (i==RECORD_INDEX_TRACKTRACK && t->t_audiofx.recordtrack) || (i==RECORD_INDEX_DEVICE && t->t_audiofx.recordtrack==0)) && // From Device write=true or from Track == Latency Check

					t->FirstChildTrack()==0
					) 
				{
					if(punchrecording&Seq_Song::PUNCHIN)
					{
						// Init Check Punch IN
						if(t->audiorecord_audiopattern[0]==0)
						{
							if(t->songstartposition>playbacksettings.cyclestart)
							{
								if(t->songstartposition<playbacksettings.cycleend)
									t->InitAudioRecordingPattern(device,t->songstartposition,t->createdatcycleloop,0,false);
							}
							else
								if(record_sampleendposition[i]>playbacksettings.cycle_samplestart)
								{
									// PUNCH IN

									LONGLONG offset=playbacksettings.cycle_samplestart-record_sampleposition[i];

									t->InitAudioRecordingPattern(device,playbacksettings.cyclestart,t->createdatcycleloop,offset,false);
								}
						}
					}
					else
						if(punchrecording&Seq_Song::PUNCHOUT)
						{
							// Init Check Punch Out
							if(t->audiorecord_audiopattern[0]==0)
							{
								if(t->songstartposition<playbacksettings.cyclestart)
								{
									AudioPattern *p1=t->InitAudioRecordingPattern(device,t->songstartposition,t->createdatcycleloop,0,false);

									if(p1)
									{
										p1->punch1=true;
									}
								}
							}

							if(t->audiorecord_audiopattern[1]==0)
							{
								if(t->songstartposition>=playbacksettings.cycleend)
								{
									t->InitAudioRecordingPattern(device,t->songstartposition,t->createdatcycleloop,0,true);
								}
								else
									if(record_sampleendposition[i]>=playbacksettings.cycle_sampleend)
									{
										// PUNCH IN
										LONGLONG cut_offset=record_sampleendposition[i]-playbacksettings.cycle_sampleend;
										t->InitAudioRecordingPattern(device,playbacksettings.cycleend,t->createdatcycleloop,cut_offset,true);
									}
							}

						}
						else
						{
							// NORMAL
							if(t->audiorecord_audiopattern[0]==0)
							{
								t->InitAudioRecordingPattern(device,t->songstartposition,t->createdatcycleloop,0,false);
								//TRACE ("New Audio Record Pattern %s\n",t->GetName());
							}
						}

				}

				t=t->NextTrack_NoUILock();
			}
		}

		if(int c=mainaudio->GetCountOfRecordingFiles(i))
		{
			//TRACE ("Audio Rec Files %d\n",c);

			if(mainvar->cpucores==1 || c==1)// Single Core recording
				audiorecordthread->WriteRecordingFiles(true);
			else{// Multi Core recording or more than 1 file

				AudioHDFile *ahd=mainaudio->FirstAudioRecordingFile();
				while(ahd){

					if(ahd->recordingactive==true){

						if( (i==RECORD_INDEX_DEVICE && ahd->istrackrecordingfile==false) ||
							(i==RECORD_INDEX_TRACKTRACK && ahd->istrackrecordingfile==true)
							)
							if(c_AudioHDFile *nc=new c_AudioHDFile(ahd))audiorecordthread->c_recfiles.AddEndO(nc);
					}

					ahd=ahd->NextHDFileNoLock();
				}

				int reccores=audiorecordthread->c_recfiles.GetCount();
				if(reccores>mainvar->cpucores)
					reccores=mainvar->cpucores; // Maximum

				//	TRACE ("Call Record Cores gone\n");
				// Call Sub Process

				for(int i=0;i<reccores;i++)
				{
					if(audiorecordthread->threads[i])
					{
						audiorecordthread->threads[i]->SetSignal();
						WaitForSingleObject(audiorecordthread->threads[i]->gone, INFINITE);// Wait For Sub Threads End
					}
				}
			}
		}

		if(refillflag&RAD_CYCLERESET){

			refillflag = RAD_CYCLERESET2;

			record_samplesize=record_setSize;
			record_sampleoffset=device->GetSetSize()-record_samplesize;
			record_sampleposition[i]=playbacksettings.cycle_samplestart;
			record_cyclecounter++;

			if(record_setSize)
				goto cycleloop;
		}
		else
			record_sampleoffset=0; // reset

		record_sampleposition[i]=record_sampleendposition[i];

	}

	// Add Read Buffer Index Counter
	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t->t_audiofx.t_audioinputreadindex==t->t_audiofx.inputbufferscounter-1)
			t->t_audiofx.t_audioinputreadindex=0;
		else
			t->t_audiofx.t_audioinputreadindex++;

		t=t->NextTrack_NoUILock();
	}
}

void AudioRecordMainThread::WaitLoop()
{
	while(IsExit()==false)
	{
		WaitSignal(); // Wait for incoming Signal

		if(IsExit()==true)
			break;

		Lock();

		if(AudioDevice *device=mainaudio->GetActiveDevice())
		{
			if(Seq_Song *song=mainvar->GetActiveSong())
			{
				device->LockRecordBuffer();

				while(device->recordbuffer_readwritecounter)
				{
					if(device->recordbuffer_readwritecounter>=device->numberhardwarebuffer) // Overflow Check and avoid endless loop
					{
						device->recorderror|=AudioDevice::RECORDERROR_BUFFEROVERFLOW;
						break;
					}

					device->recordbuffer_readwritecounter--; // Sync
					device->UnlockRecordBuffer();

					song->WriteAudioRecording(device);

					device->LockRecordBuffer();

				}//while

				device->UnlockRecordBuffer();

			} // song ?
			else
			{
				device->recordbuffer_readindex=0;
				device->recordbuffer_readwritecounter=0;
			}

		}// device

		Unlock();

		c_recfiles.DeleteAllO();	
	}// while exit

	ThreadGone();
}

PTHREAD_START_ROUTINE AudioRecordMainThread::AudioRecordMainThreadFunc(LPVOID pParam)
{
	audiorecordthread->WaitLoop();
	return 0;	
}

AudioHDFile *AudioRecordMainThread::GetAudioRecordingFile()
{
	AudioHDFile *r=0;

#ifdef WIN32
	crecfiles_semaphore.Lock();
#endif

	if(c_AudioHDFile *f=(c_AudioHDFile *)c_recfiles.GetRoot()){
		r=f->hdfile;
		c_recfiles.RemoveO(f);
	}

#ifdef WIN32
	crecfiles_semaphore.Unlock();
#endif

	return r;
}

bool AudioRecordMainThread::Init()
{
	if(mainvar->cpucores>1)
		for(int i=0;i<mainvar->cpucores;i++)
		{
			if(threads[i]=new AudioRecordThread(this))
				threads[i]->StartThread();
			else 
				return false;
		}

		ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioRecordMainThreadFunc,(LPVOID)this, 0,0);

		return true;
}

void AudioRecordMainThread::DeInit()
{
	SendQuit();

	if(mainvar->cpucores>1)
		for(int i=0;i<mainvar->cpucores;i++)
		{
			if(threads[i])
			{
				threads[i]->SendQuit();
				delete threads[i];
				threads[i]=0;
			}
		}
}