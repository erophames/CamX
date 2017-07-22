#include "runningaudiofile.h"
#include "audiosystem.h"
#include "audiopattern.h"
#include "object_track.h"
#include "audiohdfile.h"
#include "object_song.h"
#include "audiodevice.h"
#include "songmain.h"
#include "audioproc.h"
#include "semapores.h"
#include "gui.h"

void Seq_Song::RemovePatternFromAudioLists(AudioPattern *pattern)
{
	// Remove From Playback
	DeleteRunningAudioFile(&playingaudiofiles,pattern->runningfile);

	// Loops
	Seq_LoopPattern *lp=pattern->FirstLoopPattern();
	while(lp)
	{
		AudioPattern *lc=(AudioPattern *)lp->pattern;
		DeleteRunningAudioFile(&playingaudiofiles,lc->runningfile);
		lp=lp->NextLoop();
	}

	// Clones
	Seq_ClonePattern *cp=pattern->FirstClone();
	while(cp)
	{
		//	AudioPattern *ac=(AudioPattern *)cp->pattern;
		RemovePatternFromAudioLists((AudioPattern *)cp->pattern);

		/*
		DeleteRunningAudioFile(&playingaudiofiles,ac->runningfile);

		// Clone Loop
		Seq_LoopPattern *lp=ac->FirstLoopPattern();
		while(lp)
		{
		AudioPattern *lc=(AudioPattern *)lp->pattern;

		if(
		DeleteRunningAudioFile(&playingaudiofiles,lc->runningfile);
		lp=lp->NextLoop();
		}
		*/

		cp=cp->NextClone();
	}
}

void Seq_Song::DeleteAllRunningAudioFiles()
{
	//TRACE ("Start DeleteAllRunningAudioFiles\n");

	mainaudiostreamproc->Lock();

	RunningAudioFile *raf=FirstRunningAudioFile();

	while(raf)
		raf=DeleteRunningAudioFile(&playingaudiofiles,raf);

	mainaudiostreamproc->Unlock();

	//TRACE ("End DeleteAllRunningAudioFiles\n");
}

RunningAudioFile *Seq_Song::DeleteRunningAudioFile(OList *rlist,RunningAudioFile *raf)
{
	if(rlist && raf)
	{
		raf->audiopattern->audioevent.iofile.Close(true);
		raf->audiopattern->runningfile=0;
		raf->audiobuffer.Delete32BitBuffer();

		return (RunningAudioFile *)rlist->RemoveO(raf);
	}

	return 0;
}

RunningAudioFile *Seq_Song::AddAudioPatternToPlayback_Realtime(AudioDevice *device,Seq_Track *track,LONGLONG samplestart,LONGLONG sampleend,AudioPattern *audiopattern,int offset,int flag)
{
	if(!audiopattern)return 0;
	if(audiopattern->runningfile)return 0;

	if(RunningAudioFile *raf=new RunningAudioFile){

		raf->track=track;
		raf->song=this;
		raf->streamstartsampleoffset=offset;
		raf->samplestart=samplestart;
		raf->sampleend=sampleend;
		raf->audiopattern=audiopattern;
		audiopattern->runningfile=raf;

		//raf->targetdevice=device;

		// Init Region
		if(raf->audioregion=audiopattern->audioevent.audioregion){
			raf->regionstart=raf->audioregion->regionstart;
			raf->regionend=raf->audioregion->regionend;
		}

		if(raf->InitBuffer(device,flag)==true)
		{
			// Sort Channels Top->Down
			RunningAudioFile *sort=FirstRunningAudioFile();
			while(sort)
			{
				if(sort->audiopattern->audioevent.audioefile->channels<audiopattern->audioevent.audioefile->channels)
				{
					playingaudiofiles.AddNextO(raf,sort);
					return raf;
				}

				sort=sort->NextRunningFile();
			}

			playingaudiofiles.AddEndO(raf);
			return raf;
		}

		audiopattern->runningfile=0;
		delete raf;

#ifdef _DEBUG
		MessageBox(NULL,"Error Init Buffer !!!","Error",MB_OK);
#endif
	}

	return 0;
}

void Seq_Song::StreamInit()
{
	timetrack.flag CLEARBIT Seq_Time::AUDIOSTREAMREFRESH;

	streamoutofsync=false;
	stream_bufferindex=0; // Init Copy BufferIndex	

	ResetSongTracksAudioBuffer();
}

void Seq_Song::CheckAndCloseRAFFiles(AudioDevice *device,int flag)
{
	RunningAudioFile *raf=FirstRunningAudioFile();

	while(raf)
		raf=raf->closeit==true?DeleteRunningAudioFile(&playingaudiofiles,raf):raf->NextRunningFile();
}

void Seq_Song::CheckStreamAudioTempoRefresh(int mediatype)
{
	timetrack.flag CLEARBIT Seq_Time::AUDIOSTREAMREFRESH;

	Seq_Track *track=FirstTrack();

	while(track){

		AudioPattern *ap=(AudioPattern *)track->FirstPattern(mediatype);

		while(ap)
		{
			if(ap->waitforresample==false && ap->audioevent.audioefile)
			{
				LONGLONG startsample=ap->audioevent.GetSampleStart(this,ap);

				// Add Track Delay
				startsample=track->AddFullSampleDelay(startsample); // startsample can be < 0 !

				LONGLONG endsample=startsample+ap->audioevent.audioefile->samplesperchannel;

				if(!ap->runningfile)
				{
					if(endsample>stream_samplestartposition)
					{
						if((!track->playback_audiopattern[INITPLAY_MIDITRIGGER]) || track->playback_audiopattern[INITPLAY_MIDITRIGGER]->GetPatternStart()>ap->GetPatternStart())
							track->playback_audiopattern[INITPLAY_MIDITRIGGER]=ap;
					}
				}
				else
				{
					if(endsample<=stream_samplestartposition)
						DeleteRunningAudioFile(&playingaudiofiles,ap->runningfile);
					else
						if(endsample!=ap->runningfile->sampleend)
						{
							DeleteRunningAudioFile(&playingaudiofiles,ap->runningfile);

							if((!track->playback_audiopattern[INITPLAY_MIDITRIGGER]) || track->playback_audiopattern[INITPLAY_MIDITRIGGER]->GetPatternStart()>ap->GetPatternStart())
								track->playback_audiopattern[INITPLAY_MIDITRIGGER]=ap;
						}

				}
			}
			ap=(AudioPattern *)ap->NextPattern(mediatype);
		}

		track=track->NextTrack_NoUILock();
	}
}

void Seq_Song::CreateAudioStream(AudioDevice *device,int flag) // Create Song Audio Data
{
	// 1. Mix Song Data
	if(flag&CREATESTREAMINIT) // Song Start
	{
		StreamInit();

		LockRefreshCounter();
		refreshcounter=device->numberhardwarebuffer;
		UnlockRefreshCounter();
	}

	if(streamoutofsync==true)
		return;

	int mediatypeflag=MEDIATYPE_AUDIO;

	if(playbacksettings.cycleplayback==true)
		mediatypeflag|=MEDIATYPE_AUDIO_RECORD; // + add Record Files (Cycle Recording)

	if(!(flag&CREATESTREAMMASTER))LockRefreshCounter();

	while(refreshcounter || (flag&CREATESTREAMMASTER))
	{
		//TRACE ("### CreateAudioStream %d\n ###",stream_bufferindex);
		if(!(flag&CREATESTREAMMASTER))
		{
			if(refreshcounter>device->numberhardwarebuffer){
				streamoutofsync=true;
				break;
			}

			refreshcounter--;
			UnlockRefreshCounter();
		}

		stream_sampleoffset=0;
		streamflag=0;
		stream_buffersize=stream_setSize=device->GetSetSize();

		if((!(flag&CREATESTREAMMASTER)) && (timetrack.flag&Seq_Time::AUDIOSTREAMREFRESH))
			CheckStreamAudioTempoRefresh(mediatypeflag);

		CheckAndCloseRAFFiles(device,flag);

cycleloop:
		stream_sampleendposition=stream_samplestartposition+stream_setSize;

		//flag CLEARBIT CREATESTREAMFORCELOOP;

		if( (status!=Seq_Song::STATUS_STOP) || (flag&(CREATESTREAMMASTER|CREATESTREAMINIT)) )
		{
			//TRACE ("+++ > Create Audio Stream=%d RT=%d\n",stream_sampleendposition,playback_bufferindex);

			// Intern Tempo Sync Master
			if(playbacksettings.cycleplayback==true && (!(flag&CREATESTREAMMASTER)) && stream_sampleendposition>=playbacksettings.cycle_sampleend)
			{
				stream_buffersize=int((stream_sampleendposition=playbacksettings.cycle_sampleend)-stream_samplestartposition);
				//stream_buffersize-=stream_sampleendposition-playbacksettings.cycle_sampleend;
				streamflag=CREATESTREAMDOCYCLE_1;
			}

			stream_setSize-=stream_buffersize;

			if(stream_buffersize>0)
			{
				Seq_Track *track=FirstTrack();

				while(track){

					AudioPattern *p=track->playback_audiopattern[INITPLAY_MIDITRIGGER];

					while(p){

						if(p->runningfile==0 && p->waitforresample==false && p->audioevent.audioefile)
						{
							if(p->mediatype==MEDIATYPE_AUDIO || (p->mediatype==MEDIATYPE_AUDIO_RECORD && playbacksettings.cycleplayback==true) ) // Cycle Recording check
							{
								RunningAudioFile *raf=0;
								LONGLONG startsample=p->GetSampleStart(this);

								// Add Track Delay
								startsample=track->AddFullSampleDelay(startsample);
								if(startsample>stream_sampleendposition)
									break;

								LONGLONG endsample=startsample+p->audioevent.audioefile->samplesperchannel;

								if(startsample==stream_samplestartposition){ // Start Pattern== SongPosition
									p->audioevent.openseek=0;
									raf=AddAudioPatternToPlayback_Realtime(device,track,startsample,endsample,p,stream_sampleoffset,flag);
								}
								else
									if(startsample<stream_samplestartposition){ // Start Pattern < SongPosition

										if(raf=AddAudioPatternToPlayback_Realtime(device,track,startsample,endsample,p,stream_sampleoffset,flag))
											p->audioevent.openseek=stream_samplestartposition-startsample;
									}
									else
										if(startsample<stream_sampleendposition){ // Start Pattern>  SongPosition Start <= SongPositioEnd

											//[ | End ]
											//int offset=timetrack.ConvertTickToAudioBufferOffSet(stream_samplestartposition,stream_sampleendposition,start,stream_buffersize,device);

											int offset=(int)(startsample-stream_samplestartposition);

											if(raf=AddAudioPatternToPlayback_Realtime(device,track,startsample,endsample,p,offset+stream_sampleoffset,flag))
												p->audioevent.openseek=0; // Seek Start
										}

										// Pattern started now ?
										if(raf){

											if((!p->audioevent.audioefile) || p->audioevent.audioefile->m_ok==false)
												maingui->MessageBoxError(0,"OpenAudioPattern");
											else
												p->OpenAudioPattern(); // + seek to offset

											p=track->playback_audiopattern[INITPLAY_MIDITRIGGER]=(AudioPattern *)p->NextPattern(mediatypeflag);	
										}
										else
											break;
							}
							else
								p=(AudioPattern *)p->NextPattern(mediatypeflag);

						}
						else // Audio Pattern still running ?
							p=track->playback_audiopattern[INITPLAY_MIDITRIGGER]=(AudioPattern *)p->NextPattern(mediatypeflag);

					} //while  p

					track=track->NextTrack_NoUILock();
				}

				// X Processor System
				switch(mainvar->cpucores)
				{
				case 1:
					{	
						RunningAudioFile *raf=FirstRunningAudioFile();

						while(raf){
							raf->ReadRAF(device);
							raf=raf->NextRunningFile();
						}
					}
					break;

				default:
					if(RunningAudioFile *raf=FirstRunningAudioFile())
					{
						if(raf->NextRunningFile())
							mainaudiostreamproc->DoRAFS(this,device);
						else
							raf->ReadRAF(device); // 1 RAF
					}
					break;
				}
			}

			if(streamflag&CREATESTREAMDOCYCLE_1)
			{
				// Fill [-][X] 2. Buffer Part
				streamflag CLEARBIT CREATESTREAMDOCYCLE_1; // Avoid Buffer Clear
				CycleReset_Stream(device);

				if(stream_setSize)
					goto cycleloop;
			}
		}

		CheckAndCloseRAFFiles(device,flag);

		stream_samplestartposition=stream_sampleendposition;

		if(flag&CREATESTREAMMASTER) // 1 Buffer only
			return;

		if(stream_bufferindex==device->numberhardwarebuffer-1)
			stream_bufferindex=0;
		else 
			stream_bufferindex++;

		LockRefreshCounter();
		if(!refreshcounter)
			break;
	}

	if(!(flag&CREATESTREAMMASTER))
		UnlockRefreshCounter();
}
