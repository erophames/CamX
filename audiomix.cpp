#include "defines.h"

#ifdef WIN32
#include "asio/asio.h"
#endif

#include "audiodevice.h"
#include "audiofile.h"
#include "audiohardware.h"
#include "audiohardwarechannel.h"
#include "songmain.h"
#include "audiosend.h"
#include "semapores.h"
#include "runningaudiofile.h"
#include "gui.h"
#include "object_project.h"
#include "object_song.h"
#include "object_track.h"
#include "audiohdfile.h"
#include "peaks.h"
#include "panlaw.h"
#include "crossfade.h"
#include "audiopattern.h"
#include "peakbuffer.h"

extern char *channelchannelsinfo[],*channelchannelsinfo_short[];
extern int channelschannelsnumber[];

void AudioHDFile::CheckAndMixAudioRecordingBuffer(Seq_Song *song)
{
	seekbeforewrite=-1;

	recordingoffset=song->record_sampleoffset;
	recordingsize=song->record_samplesize;

	if(song->record_cyclecounter>reccycleloopcounter) // New Cycle Loop ?
	{
		if(reccycleloopcounter==0)
			ClosePeakBuffer(); // End Peak

		// Audio CycleRecording/Init Mix Start

		// First Cycle Buffer
		LONGLONG cs=song->playbacksettings.cycle_samplestart,
			ps=recordpattern->GetSampleStart(song);

		if(ps>=cs)
			cyclestartoffset_samples=cyclereadsample=0; // Read/Write 1. Sample
		else
			cyclestartoffset_samples=cyclereadsample=cs-ps; // Seek Offset

		TRACE ("New Loop %d\n",song->record_cyclecounter);
		TRACE ("CS Offset Samples %d\n",cyclestartoffset_samples);

		reccycleloopcounter=song->record_cyclecounter;
	}

	if(recordingsize>0)
	{
		recordingadded=true;

		//recmix.channelsused=channels;

		if(reccycleloopcounter>0) // In Buffer Cycle
		{
			// Read -> Buffer ->Convert Write later
			LONGLONG seekto=cyclereadsample;
			cyclereadsample+=recordingsize; // Add Buffer Size

			seekto*=samplesize_all_channels;
			seekto+=datastart;

			seekbeforewrite=seekto;

			LockIOSync(); // Avoid Cycle Read problems

			writefile.SeekBegin(seekto);

			writefile.Read(recmix.inputbuffer32bit,recordingsize*samplesize_all_channels); // Read RAW L/R
			
			UnlockIOSync();

			// L/R RAW to [][]
			if(recmix.channelsused)
				ConvertReadBufferToSamples(recmix.inputbuffer32bit,&recmix,recordingsize,channels,recordingoffset); // + Mix	
		}
	}
}

AudioHardwareChannel::AudioHardwareChannel()
{
	channelcleared[0]=channelcleared[1]=false;
	hardwarechannelused=false;
	notsupported=false;

	sizeofsample=0;

	//	hwrecordbuffers=0;
	currentpeak=peakmax=0;
	p_maxdelaycounter=0;

	audiochannel=0;
	hwname[0]=0;

	recordbufferscounter=0;
	zerocleared=false;

	Init();
}

AudioHardwareBuffer::AudioHardwareBuffer()
{
	inputbuffersize=0;
	inputbuffer32bit=0;
	outputbufferARES=static_outputbufferARES=0;
	channelsinbuffer=0;
	samplesinbuffer=0;
	channelsused=0;
	bufferms=0;
	delayvalue=0;

	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
	{
		hw_peak[i]=0;
		max_peaks[i]=0;
	}

	hw_peaksum=0;
	max_peaksum=0;

	hwbflag=0;

#ifdef _DEBUG
	n[0]='A';
	n[1]='H';
	n[2]='W';
	n[3]='B';	
#endif
}

int AudioHardwareBuffer::ExpandMixAudioBufferVolume(AudioHardwareBuffer *tobuffer,int tochannels,double volume,Peak *peakcreated,PanLaw *plaw,int flag)
{
	volume*=LOGVOLUME_SIZE;

#ifdef ARES64
	volume=logvolume[(int)volume];
#else
	volume=logvolume_f[(int)volume];
#endif

	if(volume==1)
		return ExpandMixAudioBuffer(tobuffer,tochannels,peakcreated,plaw,flag); // no volume

	if(channelsused && tochannels)
	{
		if(tochannels<channelsused)
		{
			if(tochannels==1 && channelsused==2)  // Stereo->Mono
			{
				ARES *to=tobuffer->outputbufferARES,*froml=outputbufferARES,*fromr=froml+samplesinbuffer;
				ARES mul=(ARES)volume;
				int i=samplesinbuffer;

				if(tobuffer->channelsused){

					// Mix
					if(int loop=i/8)
					{
						i-=loop*8;

						do
						{
							*to++ +=(*froml++ + *fromr++) * mul;
							*to++ +=(*froml++ + *fromr++) * mul;
							*to++ +=(*froml++ + *fromr++) * mul;
							*to++ +=(*froml++ + *fromr++) * mul;

							*to++ +=(*froml++ + *fromr++) * mul;
							*to++ +=(*froml++ + *fromr++) * mul;
							*to++ +=(*froml++ + *fromr++) * mul;
							*to++ +=(*froml++ + *fromr++) * mul;

						}while(--loop);
					}

					while(i--) // Rest
						*to++ +=(*froml++ + *fromr++) * mul;

				}
				else{

					// Copy

					if(int loop=i/8)
					{
						i-=loop*8;

						do
						{
							*to++ = (*froml++ + *fromr++) * mul;
							*to++ = (*froml++ + *fromr++) * mul;
							*to++ = (*froml++ + *fromr++) * mul;
							*to++ = (*froml++ + *fromr++) * mul;

							*to++ = (*froml++ + *fromr++) * mul;
							*to++ = (*froml++ + *fromr++) * mul;
							*to++ = (*froml++ + *fromr++) * mul;
							*to++ = (*froml++ + *fromr++) * mul;

						}while(--loop);
					}

					while(i--) // Rest
						*to++ = (*froml++ + *fromr++) * mul;
				}
			}
		}
		else
			if(tochannels>channelsused) // Expand Mono -> Stereo etc..
			{
				int fromc=0;
				ARES *to=tobuffer->outputbufferARES,mul=(ARES)volume;

				for(int c=0;c<tochannels;c++){	
					ARES *from=outputbufferARES;
					from +=samplesinbuffer*fromc;

					if(c+1>tobuffer->channelsused){

						int i=samplesinbuffer;

						if(int loop=i/8)
						{
							i-=loop*8;

							do
							{
								*to++ = *from++ * mul;
								*to++ = *from++ * mul;
								*to++ = *from++ * mul;
								*to++ = *from++ * mul;

								*to++ = *from++ * mul;
								*to++ = *from++ * mul;
								*to++ = *from++ * mul;
								*to++ = *from++ * mul;

							}while(--loop);
						}

						while(i--) // Rest Copy
							*to++ = *from++ * mul;
					}
					else{ 
						// Mix
						int i=samplesinbuffer;

						if(int loop=i/8)
						{
							i-=loop*8;

							do
							{
								*to++ += *from++ * mul;
								*to++ += *from++ * mul;
								*to++ += *from++ * mul;
								*to++ += *from++ * mul;

								*to++ += *from++ * mul;
								*to++ += *from++ * mul;
								*to++ += *from++ * mul;
								*to++ += *from++ * mul;

							}while(--loop);
						}

						while(i--)// Rest
							*to++ += *from++ * mul;
					}

					if(channelsused>fromc+1)
						fromc++;
				}	
			}
			else
				MixAudioBuffer(tobuffer,volume,VOLUMEISCORRECT);

		if(tochannels>tobuffer->channelsused)
			tobuffer->channelsused=tochannels;
	}

	return flag;
}

int AudioHardwareBuffer::ExpandMixAudioBuffer(AudioHardwareBuffer *tobuffer,int tochannels,Peak *peak,PanLaw *plaw,int flag,ARES mulpeak)
{
	if(channelsused==0 || tochannels==0)
		return 0;

	if(tochannels<channelsused)
	{
		if(tochannels==1 && channelsused==2)  // Stereo->Mono
		{
			ARES *to=tobuffer->outputbufferARES,*from_l=outputbufferARES,*from_r=from_l+samplesinbuffer,p1=0,p2=0,mul;

			int i=samplesinbuffer;

			if(plaw && (mul=mainaudio->panorama_db[plaw->panorama_db]!=1))
			{
				if(peak)
				{
					ARES pc1_m=0,pc1_p=0,pc2_m=0,pc2_p=0;

					if(tobuffer->channelsused)
						while(i--)
						{
							ARES h1=*from_l++;

							if(h1<pc1_m)pc1_m=h1;
							else
								if(h1>pc1_p)pc1_p=h1;

							ARES h2=*from_r++;
							if(h2<pc2_m)pc2_m=h1;
							else
								if(h2>pc2_p)pc2_p=h1;

							*to++ += (h1+h2)*mul;
						}
					else
						while(i--)
						{
							ARES h1=*from_l++;

							if(h1<pc1_m)pc1_m=h1;
							else
								if(h1>pc1_p)pc1_p=h1;

							ARES h2=*from_r++;
							if(h2<pc2_m)pc2_m=h1;
							else
								if(h2>pc2_p)pc2_p=h1;

							*to++ =(h1+h2)*mul;
						}

						p1=(-pc1_m>pc1_p)?-pc1_m:pc1_p;
						p2=(-pc2_m>pc2_p)?-pc2_m:pc2_p;
				}
				else
				{
					if(tobuffer->channelsused)
						while(i--)*to++ += (*from_l++ + *from_r++)*mul;
					else
						while(i--)*to++ = (*from_l++ + *from_r++)*mul;
				}
			}
			else //no plaw
			{
				if(peak)
				{
					ARES pc1_m=0,pc1_p=0,pc2_m=0,pc2_p=0;

					if(tobuffer->channelsused)
						while(i--)
						{
							ARES h1=*from_l++;

							if(h1<pc1_m)pc1_m=h1;
							else
								if(h1>pc1_p)pc1_p=h1;

							ARES h2=*from_r++;
							if(h2<pc2_m)pc2_m=h1;
							else
								if(h2>pc2_p)pc2_p=h1;

							*to++ += (h1+h2)*mMIX;
						}
					else
						while(i--)
						{
							ARES h1=*from_l++;

							if(h1<pc1_m)pc1_m=h1;
							else
								if(h1>pc1_p)pc1_p=h1;

							ARES h2=*from_r++;
							if(h2<pc2_m)pc2_m=h1;
							else
								if(h2>pc2_p)pc2_p=h1;

							*to++ = (h1+h2)*mMIX;
						}

						p1=(-pc1_m>pc1_p)?-pc1_m:pc1_p;
						p2=(-pc2_m>pc2_p)?-pc2_m:pc2_p;
				}
				else
				{
					if(tobuffer->channelsused)
						while(i--)*to++ += (*from_l++ + *from_r++)*mMIX;
					else
						while(i--)*to++ = (*from_l++ + *from_r++)*mMIX;
				}
			}

			if(peak)
			{
				p1*=mulpeak;
				p2*=mulpeak;

				peak->p_current[0]=p1;
				peak->p_current[1]=p2;

				if(peak->p_max[0]<p1){
					peak->p_max[0]=p1;
					if(bufferms>=1)peak->p_maxdelaycounter[0]=delayvalue; // 1000 ms max peak drop down
				}

				if(peak->p_max[1]<p2){
					peak->p_max[1]=p2;
					if(bufferms>=1)peak->p_maxdelaycounter[1]=delayvalue; // 1000 ms max peak drop down
				}

				if(p2>p1)p1=p2;

				if(p1>peak->current_sum)peak->current_sum=p1;
				if(p1>peak->current_max)peak->current_max=p1;

				if(p1>0 || peak->p_max[0]>0 || peak->p_max[1]>0)
					peak->peakused=true;

				flag|=PEAKCREATED;
			}
		}
	}
	else
		if(tochannels>channelsused) // Expand Mono -> Stereo etc..
		{
			switch(channelsused)
			{
			case 1:
				{
					switch(tochannels)
					{
					case 2: // Mono -> Stereo
						{
							ARES *l,*r,*from=outputbufferARES;
							register ARES h;

							l=r=tobuffer->outputbufferARES;
							r+=samplesinbuffer;

							int i=samplesinbuffer;

							if(peak)
							{
								ARES p_m=0,p_p=0;

								if(tobuffer->channelsused==1)
								{
									memcpy(r,from,samplesinbuffer_size);

									while(i--) // mix mono left,add right
									{
										h=*from++;
										*l++ +=h; // *lp
										//*r++ =h; //*rp;
										if(h<p_m)p_m=h;
										else
											if(h>p_p)p_p=h;
									}
								}
								else
									if(tobuffer->channelsused>=2)
										while(i--) // mix stereo
										{
											h=*from++;
											*l++ +=h; //*lp;
											*r++ +=h; //*rp;
											if(h<p_m)p_m=h;
											else
												if(h>p_p)p_p=h;
										}
									else
									{
										// copy stereo
										memcpy(l,from,samplesinbuffer_size);
										memcpy(r,from,samplesinbuffer_size);

										while(i--) 
										{
											h=*from++;
											if(h<p_m)p_m=h;
											else
												if(h>p_p)p_p=h;
										}
									}

									p_m*=mulpeak;
									p_p*=mulpeak;

									ARES p=(-p_m>p_p)?-p_m:p_p;

									for(int cl=0;cl<2;cl++)
									{
										peak->p_current[cl]=p;

										if(peak->p_max[cl]<p){
											peak->p_max[cl]=p;
											if(bufferms>=1)peak->p_maxdelaycounter[cl]=delayvalue; // 1000 ms max peak drop down
										}

										if(p>0 || peak->p_max[cl]>0)peak->peakused=true;
									}

									if(p>peak->current_sum)peak->current_sum=p;
									if(p>peak->current_max)peak->current_max=p;

									flag|=PEAKCREATED;
							}//if peak
							else
							{
								if(tobuffer->channelsused==1)
								{
									// mix mono left,copy right
									memcpy(r,from,samplesinbuffer_size);
									while(i--)*l++ +=*from++; // *lp
								}
								else
									if(tobuffer->channelsused>=2)
									{
										while(i--) // mix stereo
										{
											h=*from++;
											*l++ +=h; //*lp;
											*r++ +=h; //*rp;
										}
									}
									else
									{
										// Copy Mono -> L/R
										memcpy(l,from,samplesinbuffer_size);
										memcpy(r,from,samplesinbuffer_size);
									}
							}
						}
						break;
					}
				}
				break;
			}

			//kLinearPanLaw = 0,	// L = pan * M; R = (1 - pan) * M;
			//kEqualPowerPanLaw	// L = pow (pan, 0.5) * M; R = pow ((1 - pan), 0.5) * M;
		}
		else // tochannels==usechannels
		{
			if(peak)
			{
				MixAudioBuffer(tobuffer,peak,flag,mulpeak);
				flag|=PEAKCREATED;
			}
			else
				MixAudioBuffer(tobuffer,flag);
		}

		if(tochannels>tobuffer->channelsused)
			tobuffer->channelsused=tochannels;

		return flag;
}

void AudioHardwareBuffer::CreatePeak(AudioHardwareChannel *hwc)
{
	if(channelsused==0){hwc->currentpeak=0;return;}

	ARES *sample=outputbufferARES,m_peak=0,p_peak=0;
	int i=samplesinbuffer;

	if(int loop=i/8)
	{
		i-=loop*8;

		do
		{
			ARES h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

			h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

		}while(--loop);
	}

	while(i--){
		ARES h=*sample++;
		if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
	}	

	ARES peak=(-m_peak>p_peak)?-m_peak:p_peak;

	hwc->currentpeak=peak;

	if(peak>hwc->peakmax)
		hwc->peakmax=peak;

	//TRACE ("IC CPeak %f MPeak %f Max %f\n",hwc->currentpeak,hwc->peakmax,peak);
}

void AudioHardwareBuffer::CreatePeakTo(Peak *to)
{
	if(!channelsused)return;

	int chls=to->channels<channelsused?to->channels:channelsused;
	ARES *sample=outputbufferARES;

	for(int cl=0;cl<chls;cl++)
	{
		ARES m_peak=0,p_peak=0;

		int i=samplesinbuffer;

		if(int loop=i/8)
		{
			i-=8*loop;

			do
			{
				ARES h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

			}while(--loop);
		}

		while(i--){
			ARES h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
		}	

		ARES maxpeak=(-m_peak>p_peak)?-m_peak:p_peak;

		to->p_current[cl]=maxpeak;
		if(maxpeak>to->current_sum)to->current_sum=maxpeak;

		if(to->p_max[cl]<maxpeak){
			to->p_max[cl]=maxpeak;
			if(bufferms>=1)to->p_maxdelaycounter[cl]=delayvalue; // 1000 ms max peak drop down
		}

		if(maxpeak>to->current_max)to->current_max=maxpeak;
	}

	for(int i=0;i<chls;i++)
		if(to->p_current[i]!=0 || to->p_max[i]!=0){
			to->peakused=true;
			return;
		}

		to->peakused=false;
}

void AudioHardwareBuffer::CreatePeak(AudioHardwareBuffer *buffer,PanLaw *plaw)
{
	int chls=buffer?buffer->channelsused:channelsused;

	if(!chls)return;
	ARES *sample=buffer?buffer->outputbufferARES:outputbufferARES;

	for(int cl=0;cl<chls;cl++)
	{
		ARES m_peak=0,p_peak=0;

		int i=samplesinbuffer;

		if(int loop=i/8)
		{
			i-=8*loop;

			do
			{
				ARES h=*sample++;

				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				h=*sample++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

			}while(--loop);
		}

		while(i--){
			ARES h=*sample++;
			if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
		}	

		if(plaw)
		{
			m_peak*=plaw->GetPanValue();
			p_peak*=plaw->GetPanValue();
		}

		ARES maxpeak=peak.p_current[cl]=-m_peak>p_peak?-m_peak:p_peak;

		if(maxpeak>peak.current_sum)peak.current_sum=maxpeak;
		if(maxpeak>peak.current_max)peak.current_max=maxpeak;

		if(peak.p_max[cl]<maxpeak){
			peak.p_max[cl]=maxpeak;
			if(bufferms>=1)peak.p_maxdelaycounter[cl]=delayvalue; // 1000 ms max peak drop down
		}

	}

	for(int i=0;i<chls;i++)
		if(peak.p_current[i]!=0 || peak.p_max[i]!=0){
			peak.peakused=true;
			return;
		}

		peak.peakused=false;
}

void AudioHardwareBuffer::CopyFromFrames(ARES *from,int frames,int tchannels)
{
	switch(tchannels)
	{
	case 1:
		{
			memcpy(outputbufferARES,from,sizeof(ARES)*frames);
		}
		break;

	case 2:
		{
			register ARES *to_l,*to_r;

			to_l=to_r=outputbufferARES;
			to_r+=samplesinbuffer;

			// L/R > [L][R]

			register int i=frames;
			while(i--){
				*to_l++=*from++;
				*to_r++=*from++;
			}
		}
		break;

	default:
		for(int c=0;c<tchannels;c++)
		{
			register ARES *fc=from,*to=outputbufferARES;

			fc+=c;
			to+=samplesinbuffer*c;

			register int i=frames;
			while(i--){
				*to++=*fc;
				fc+=tchannels;
			}
		}
		break;
	}
}

/*
void AudioHardwareBuffer::MulFloat(int samples,float mul)
{
		if(mul!=1)
	{
		float *s=(float *)outputbufferARES;

		if(int loop=samples/8)
		{
			samples-=8*loop;

			do
			{
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;

				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;

			}while(--loop);
		}

		while(samples--)
		{
			*s++ *=mul;
		}
	}
}
*/

void AudioHardwareBuffer::MulCon32(float *s,int samples,float mul)
{
	mul*=LOGVOLUME_SIZE;
	mul=logvolume_f[(int)mul];

	if(mul!=1)
	{
		if(int loop=samples/8)
		{
			samples-=8*loop;

			do
			{
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;

				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;

			}while(--loop);
		}

		while(samples--)*s++ *=mul;
	}
}

void AudioHardwareBuffer::MulCon(int samples,ARES mul)
{
	mul*=LOGVOLUME_SIZE;

#ifdef ARES64
	mul=logvolume[(int)mul];
#else
	mul=logvolume_f[(int)mul];
#endif

	if(mul==1)return;

	ARES *s=outputbufferARES;

	if(int loop=samples/8)
	{
		samples-=8*loop;

		do
		{
			*s++ *=mul;
			*s++ *=mul;
			*s++ *=mul;
			*s++ *=mul;

			*s++ *=mul;
			*s++ *=mul;
			*s++ *=mul;
			*s++ *=mul;

		}while(--loop);
	}

	while(samples--)*s++ *=mul;
}


void AudioHardwareBuffer::Mul(int samples,ARES mul)
{
	if(mul!=1)
	{
		ARES *s=(ARES *)outputbufferARES;

		if(int loop=samples/8)
		{
			samples-=8*loop;

			do
			{
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;

				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;

			}while(--loop);
		}

		while(samples--)*s++ *=mul;
	}
}

void AudioHardwareBuffer::CopyToFrames(ARES *to,int frames,int tchannels) 
{
	if(tchannels==1)
	{
		memcpy(to,outputbufferARES,frames*sizeof(ARES));
		return;
	}

	if(tchannels==2)
	{
		for(int c=0;c<2;c++)
		{
			register ARES *from=outputbufferARES,*toc=to;

			from+=samplesinbuffer*c;
			toc+=c;

			register int i=frames;

			while(i--){
				*toc=*from++;
				toc+=2;
			}
		}

		return;
	}

	for(int c=0;c<tchannels;c++)
	{
		register ARES *from=outputbufferARES,*toc=to;

		from+=samplesinbuffer*c;
		toc+=c;

		register int i=frames;

		while(i--){
			*toc=*from++;
			toc+=tchannels;
		}
	}
}

int AudioHardwareBuffer::GetFirstSampleOffset()
{
	int foundsample=samplesinbuffer;

	for(int chl=0;chl<channelsused;chl++)
	{
		ARES *c=outputbufferARES;
		c+=samplesinbuffer*chl;

		for(int i=0;i<samplesinbuffer;i++){
			if(*c++!=0){
				if(i<foundsample)foundsample=i;
				break;
			}
		}
	}

	return foundsample;
}

void AudioHardwareBuffer::CopyAudioBuffer(AudioHardwareBuffer *tobuffer)
{
	if(channelsused)
	{
		if(tobuffer->channelsinbuffer<=channelsused) // 2->1, 2->2
		{
			tobuffer->channelsused=tobuffer->channelsinbuffer;
			memcpy(tobuffer->outputbufferARES,outputbufferARES,tobuffer->channelsinbuffer*samplesinbuffer_size);
		}
		else
		{
			if(tobuffer->channelsused<channelsused)
				tobuffer->channelsused=channelsused;

			memcpy(tobuffer->outputbufferARES,outputbufferARES,channelsused*samplesinbuffer_size);		
		}
	}
	else
	tobuffer->channelsused=0;
}


void AudioHardwareBuffer::MixAudioBuffer(AudioHardwareBuffer *tobuffer,AudioPort *avhc)
{
	if(!avhc)return;

#ifdef DEBUG
	if(samplesinbuffer!=tobuffer->samplesinbuffer)
	{
		MessageBox(NULL,"Sample Size Incorrect 1","Error",MB_OK);
	}
#endif

	if(avhc->channels>channelsused) // Expand Mono->stereo
	{
		if(channelsused==1 && avhc->channels==2)
		{
			register ARES *to=tobuffer->outputbufferARES; // Mono -> Stereo

			for(int c=0;c<avhc->channels;c++)
			{
				if(AudioHardwareChannel *hw=avhc->hwchannel[c])
				{
					register ARES *from=outputbufferARES;

					if(hw->hardwarechannelused==true){
						// Mix
						int i=samplesinbuffer;

						if(int loop=i/8)
						{
							i-=8*loop;
							do
							{
								*to++ +=*from++;
								*to++ +=*from++;
								*to++ +=*from++;
								*to++ +=*from++;

								*to++ +=*from++;
								*to++ +=*from++;
								*to++ +=*from++;
								*to++ +=*from++;

							}while(--loop);
						}

						while(i--)*to++ +=*from++;
					}
					else{
						//Copy
						memcpy(to,from,samplesinbuffer_size);
						to+=samplesinbuffer;
						hw->hardwarechannelused=true; // Set HW Channel Used Flag
					}
				}
			}
		}

		return;
	}
	else
	{
		register ARES *from=outputbufferARES;

		for(int c=0;c<channelsused;c++)
		{
			// else mix to old hw
			if(AudioHardwareChannel *hw=avhc->hwchannel[c])
			{
				register ARES *to=tobuffer->outputbufferARES;
				to+=samplesinbuffer*hw->channelindex; // Add HW Channel Index

				if(hw->hardwarechannelused==true){
					// Mix
					int i=samplesinbuffer;
					
					if(int loop=i/8)
						{
							i-=8*loop;
							do
							{
								*to++ +=*from++;
								*to++ +=*from++;
								*to++ +=*from++;
								*to++ +=*from++;

								*to++ +=*from++;
								*to++ +=*from++;
								*to++ +=*from++;
								*to++ +=*from++;

							}while(--loop);
						}

					while(i--)*to++ +=*from++;
				}
				else{
					// Copy
					memcpy(to,from,samplesinbuffer_size);
					from+=samplesinbuffer;
					hw->hardwarechannelused=true; // Set HW Channel Used Flag
				}
			}
		}
	}

	// avhc->UnlockO();
}

void AudioHardwareBuffer::MixAudioBuffer(AudioHardwareBuffer *tobuffer,Peak *topeak,int flag,ARES mulpeak) // no volume
{
	if(!channelsused)return;

	if(tobuffer->channelsused) // Mix ?
	{
		// Mix I
		if(tobuffer->channelsused>=channelsused){

			ARES *to=tobuffer->outputbufferARES,*from=outputbufferARES;

			for(int cl=0;cl<channelsused;cl++)
			{
				ARES m_peak=0,p_peak=0;
				int i=samplesinbuffer;

				if(int loop=i/8)
				{
					i-=loop*8;

					do
					{
						ARES h=*to++ +=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

					}while(--loop);
				}

				while(i--)
				{
					ARES h=*to++ +=*from++; // Mix
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				}

				m_peak*=mulpeak;
				p_peak*=mulpeak;

				ARES peak=topeak->p_current[cl]=(-m_peak>p_peak)?-m_peak:p_peak;

				if(peak>topeak->current_sum)topeak->current_sum=peak;
				if(peak>topeak->current_max)topeak->current_max=peak;

				if(topeak->p_max[cl]<peak){
					topeak->p_max[cl]=peak;
					if(bufferms>=1)topeak->p_maxdelaycounter[cl]=delayvalue; // 1000 ms max peak drop down
				}
			}
		}
		else{

			// Mix II
			ARES *to=tobuffer->outputbufferARES,*from=outputbufferARES;

			for(int cl=0;cl<tobuffer->channelsused;cl++)
			{
				ARES m_peak=0,p_peak=0;
				int i=samplesinbuffer;

				if(int loop=i/8)
				{
					i-=loop*8;

					do
					{
						ARES h=*to++ +=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++ +=h=*from++; // Mix
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

					}while(--loop);
				}

				while(i--){ //Rest
					ARES h=*to++ +=*from++; // Mix
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				}

				m_peak*=mulpeak;
				p_peak*=mulpeak;

				ARES peak=topeak->p_current[cl]=(-m_peak>p_peak)?-m_peak:p_peak;

				if(peak>topeak->current_sum)topeak->current_sum=peak;
				if(peak>topeak->current_max)topeak->current_max=peak;

				if(topeak->p_max[cl]<peak){
					topeak->p_max[cl]=peak;
					if(bufferms>=1)topeak->p_maxdelaycounter[cl]=delayvalue; // 1000 ms max peak drop down
				}
			}

			// Rest Copy
			for(int cl=tobuffer->channelsused;cl<channelsused;cl++)
			{
				ARES m_peak=0,p_peak=0;
				int i=samplesinbuffer;

				if(int loop=i/8)
				{
					i-=loop*8;

					do
					{
						ARES h=*to++=*from++; // Copy
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++=h=*from++; // Copy
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++=h=*from++; // Copy
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++=h=*from++; // Copy
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

						*to++=h=*from++; // Copy
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++=h=*from++; // Copy
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++=h=*from++; // Copy
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
						*to++=h=*from++; // Copy
						if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;


					}while(--loop);
				}

				while(i--){
					ARES h=*to++=*from++; // Copy
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
				}

				m_peak*=mulpeak;
				p_peak*=mulpeak;

				ARES peak=topeak->p_current[cl]=(-m_peak>p_peak)?-m_peak:p_peak;

				if(peak>topeak->current_sum)topeak->current_sum=peak;
				if(peak>topeak->current_max)topeak->current_max=peak;

				if(topeak->p_max[cl]<peak){
					topeak->p_max[cl]=peak;
					if(bufferms>=1)topeak->p_maxdelaycounter[cl]=delayvalue; // 1000 ms max peak drop down
				}
			}

			tobuffer->channelsused=channelsused;
		}
	}
	else {

		// Copy
		if(flag&USETEMPPOSSIBLE)
		{
			tobuffer->outputbufferARES=outputbufferARES;
#ifdef DEBUG
			tobuffer->streamstart=streamstart;
			tobuffer->streamend=streamend;
#endif
		}
		else
			memcpy(tobuffer->outputbufferARES,outputbufferARES,channelsused*samplesinbuffer_size);

		ARES *from=outputbufferARES;

		for(int cl=0;cl<channelsused;cl++)
		{
			ARES m_peak=0,p_peak=0;
			int i=samplesinbuffer;

			if(int loop=i/8)
			{
				i-=loop*8;

				do
				{
					ARES h=*from++;

					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
					h=*from++;
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
					h=*from++;
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
					h=*from++;
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

					h=*from++;
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
					h=*from++;
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
					h=*from++;
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
					h=*from++;
					if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;

				}while(--loop);
			}

			while(i--){ // Rest
				ARES h=*from++;
				if(h<m_peak)m_peak=h;else if(h>p_peak)p_peak=h;
			}

			m_peak*=mulpeak;
			p_peak*=mulpeak;

			ARES peak=topeak->p_current[cl]=(-m_peak>p_peak)?-m_peak:p_peak;

			if(peak>topeak->current_sum)topeak->current_sum=peak;
			if(peak>topeak->current_max)topeak->current_max=peak;

			if(topeak->p_max[cl]<peak){
				topeak->p_max[cl]=peak;
				if(bufferms>=1)topeak->p_maxdelaycounter[cl]=delayvalue; // 1000 ms max peak drop down
			}
		}

		if(tobuffer->channelsused<=channelsused)
			tobuffer->channelsused=channelsused;

	}//else

	for(int i=0;i<tobuffer->channelsused;i++)
		if(topeak->p_current[i]!=0 || topeak->p_max[i]!=0){
			topeak->peakused=true;

			/*
			if(flag&USETEMPPOSSIBLE)
			tobuffer->peak.peakused=true;
			*/
			break;
		}
}

void AudioHardwareBuffer::MixAudioBuffer(AudioHardwareBuffer *tobuffer,int flag)
{
#ifdef DEBUG
	if(!outputbufferARES)
	{
		maingui->MessageBoxError(0,"outputbufferARES");
		return;
	}

	if(!tobuffer)
	{
		maingui->MessageBoxError(0,"tobuffer");
		return;
	}

	if(!tobuffer->outputbufferARES)
	{
		maingui->MessageBoxError(0,"tobuffer->outputbufferARES");
		return;
	}

	if(tobuffer->samplesinbuffer!=samplesinbuffer)
	{
		char h2[32];

		char *h=mainvar->GenerateString("MixAudioBuffer !=",mainvar->ConvertIntToChar(samplesinbuffer,h2),":",mainvar->ConvertIntToChar(tobuffer->samplesinbuffer,h2));
		maingui->MessageBoxError(0,h);
		return;

	}
#endif

	if(channelsused)
	{
		if(tobuffer->channelsused) // Mix ?
		{
			register ARES *from=outputbufferARES,*to=tobuffer->outputbufferARES;

			if(tobuffer->channelsused>=channelsused){

				int i=channelsused*samplesinbuffer;

				if(int loop=i/8)
				{
					i-=8*loop;

					do
					{
						*to++ +=*from++; // Mix (Add Samples)
						*to++ +=*from++; 
						*to++ +=*from++; 
						*to++ +=*from++; 

						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;

					}while(--loop);
				}

				while(i--)*to++ +=*from++; // Rest Mix (Add Samples)
			}
			else{

				int i=tobuffer->channelsused*samplesinbuffer;

				if(int loop=i/8)
				{
					i-=8*loop;

					do
					{
						*to++ +=*from++; // Mix (Add Samples)
						*to++ +=*from++; 
						*to++ +=*from++; 
						*to++ +=*from++; 

						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;

					}while(--loop);
				}

				while(i--)*to++ +=*from++; // Rest Mix (Add Samples)

				//Rest Copy
				memcpy(to,from,(channelsused-tobuffer->channelsused)*samplesinbuffer_size);
				tobuffer->channelsused=channelsused;
			}
		}
		else { // Copy

			if(flag&USETEMPPOSSIBLE)
				tobuffer->outputbufferARES=outputbufferARES;
			else
				memcpy(tobuffer->outputbufferARES,outputbufferARES,channelsused*samplesinbuffer_size);

#ifdef DEBUG
			tobuffer->streamstart=streamstart;
			tobuffer->streamend=streamend;
#endif
			tobuffer->channelsused=channelsused;
		}
	}
}

void AudioHardwareBuffer::MixAudioBuffer(AudioHardwareBuffer *tobuffer,double volume,int flag)
{
	if(!(flag&VOLUMEISCORRECT))
	{
	volume*=LOGVOLUME_SIZE;
	volume=logvolume[(int)volume];
	}

	if(channelsused)
	{
		if(tobuffer->channelsused) // Mix ?
		{
			register ARES *from=outputbufferARES,*to=tobuffer->outputbufferARES,mul=(ARES)volume;

			if(tobuffer->channelsused>=channelsused){

				int i=channelsused*samplesinbuffer;

				if(int loop=i/8)
				{
					i-=8*loop;

					do
					{
						*to++ +=*from++ * mul; // Mix (Add Samples)
						*to++ +=*from++ * mul; 
						*to++ +=*from++ * mul;
						*to++ +=*from++ * mul;

						*to++ +=*from++ * mul;
						*to++ +=*from++ * mul;
						*to++ +=*from++ * mul;
						*to++ +=*from++ * mul;

					}while(--loop);
				}

				while(i--)*to++ +=*from++ * mul; // Rest Mix (Add Samples)
			}
			else{

				{
					int i=tobuffer->channelsused*samplesinbuffer;

					if(int loop=i/8)
					{
						i-=8*loop;

						do
						{
							*to++ +=*from++ * mul; // Mix (Add Samples)
							*to++ +=*from++ * mul; 
							*to++ +=*from++ * mul;
							*to++ +=*from++ * mul;

							*to++ +=*from++ * mul;
							*to++ +=*from++ * mul;
							*to++ +=*from++ * mul;
							*to++ +=*from++ * mul;

						}while(--loop);
					}

					while(i--)*to++ +=*from++ *mul; // Rest Mix (Add Samples)
				}

				//Rest Copy
				{
					int i=(channelsused-tobuffer->channelsused)*samplesinbuffer;

					if(int loop=i/8)
					{
						i-=loop*8;

						do
						{
							*to++ =*from++ *mul;
							*to++ =*from++ *mul;
							*to++ =*from++ *mul;
							*to++ =*from++ *mul;

							*to++ =*from++ *mul;
							*to++ =*from++ *mul;
							*to++ =*from++ *mul;
							*to++ =*from++ *mul;

						}while(--loop);
					}

					while(i--)
						*to++ =*from++ *mul; // Copy 
				}

				tobuffer->channelsused=channelsused;
			}
		}
		else {
			// Copy
			register ARES *from=outputbufferARES,*to=tobuffer->outputbufferARES,mul=(ARES)volume;

			int i=channelsused*samplesinbuffer;

			if(int loop=i/8)
			{
				i-=8*loop;

				do
				{
					*to++ =*from++ *mul; // Copy
					*to++ =*from++ *mul;
					*to++ =*from++ *mul;
					*to++ =*from++ *mul;

					*to++ =*from++ *mul;
					*to++ =*from++ *mul;
					*to++ =*from++ *mul;
					*to++ =*from++ *mul;

				}while(--loop);
			}

			while(i--)
				*to++ =*from++ *mul; // Rest Copy 

			if(tobuffer->channelsused<channelsused)tobuffer->channelsused=channelsused;
		}
	}
}

bool AudioHardwareBuffer::Create32BitBuffer_Input(int initchannels,int initsamples)
{
	Delete32BitBuffer();

	if(initchannels>MAXCHANNELSPERCHANNEL)initchannels=MAXCHANNELSPERCHANNEL;

	if(initchannels>0 && initsamples>0){

		//channelsinbuffer=initchannels;
		//samplesinbuffer=initsamples;
		//samplesinbuffer_size=initsamples*sizeof(ARES);

		SetBuffer(initchannels,initsamples);

		// Create 32 Bit Stereo Read Buffer --> LONG <--- 4 Bytes
		inputbuffer32bit=new double[channelsinbuffer*initsamples]; // double=64 Bit INPUT
		inputbuffersize=sizeof(double)*channelsinbuffer*initsamples;

		if(inputbuffer32bit){

			InitARESOut(initsamples,channelsinbuffer);

			if(outputbufferARES){
				//buffersize_bytes=sizeof(ARES)*channelsinbuffer*initsamples;
				//hardwareinit=true;
#ifdef _DEBUG
				CheckBuffer();
#endif
				return true;
			}

			delete inputbuffer32bit;
			inputbuffer32bit=0;
			//buffersize_bytes=0;
		}
	}

	return false;
}

#ifdef _DEBUG
void AudioHardwareBuffer::CheckBuffer()
{
	if(outputbufferARES)
	{
		if(outputbufferARES[channelsinbuffer*samplesinbuffer]!=1.1f)
		{
			double h=outputbufferARES[channelsinbuffer*samplesinbuffer];

			if(outputbufferARES!=static_outputbufferARES)
				MessageBox(NULL,"Hardware Buffer overflow (outputbufferARES!=static_outputbufferARES)","Error",MB_OK);
			else
				MessageBox(NULL,"Hardware Buffer overflow","Error",MB_OK);
		}
	}
	//	else
	//		MessageBox(NULL,"No Hardware Buffer ","Error",MB_OK);
}
#endif

void AudioHardwareBuffer::DeleteARESOut()
{
	if(outputbufferARES){
		delete outputbufferARES;
		outputbufferARES=static_outputbufferARES=0;
	}
}

void AudioHardwareBuffer::InitARESOut(int setSize,int channels)
{
	int buffersize=setSize*channels;

#ifdef DEBUG
	buffersize+=1;
#endif

	if(buffersize>0)
	{
		outputbufferARES=new ARES[buffersize];

#ifdef DEBUG
		outputbufferARES[buffersize-1]=1.1f;
#endif
	}
	else
		outputbufferARES=0;

	static_outputbufferARES=outputbufferARES;
}

void AudioHardwareBuffer::Delete32BitBuffer()
{	
	if(inputbuffer32bit){
		delete inputbuffer32bit; // memory
		inputbuffer32bit=0;
	}

	DeleteARESOut();

	//buffersize_bytes=0;
	bufferms=0;
	channelsused=0;
	channelsinbuffer=0;
	samplesinbuffer=0;
	//hardwareinit=false;
}

void RunningAudioFile::AddCrossFadesTobuffer(Seq_Song *song,AudioPattern *audiopattern)
{
	audiopattern->Lock_CrossFades();

	Seq_CrossFade *scf=audiopattern->FirstCrossFade();

	while(scf)
	{
		if(scf->used==true && scf->CheckIfInRange(effectparameter.stream_samplestartposition,effectparameter.stream_samplestartposition_end)==true)
		{
			ARES *startsample=audiobuffer.outputbufferARES;
			startsample+=streamstartsampleoffset;

			LONGLONG sstart,send=effectparameter.stream_samplestartposition_end<scf->to_sample?effectparameter.stream_samplestartposition_end:scf->to_sample;
			double cfsize=scf->to_sample-scf->from_sample;

			if(scf->from_sample>effectparameter.stream_samplestartposition)
				sstart=scf->from_sample;
			else
				sstart=effectparameter.stream_samplestartposition;

			switch(audiobuffer.channelsused)
			{
			case 1:
				while(sstart<=send)
				{
					double x=sstart++-scf->from_sample;
					x/=cfsize;
					*startsample++ *=scf->ConvertToVolume(x,scf->infade);
				}
				break;

			case 2:
				while(sstart<=send)
				{
					double x=sstart++-scf->from_sample;
					x/=cfsize;

					ARES y=scf->ConvertToVolume(x,scf->infade);

					*startsample *=y; // Left
					ARES *sample2=startsample++ +audiobuffer.samplesinbuffer;
					*sample2 *=y; // Right
				}
				break;

			default:
				while(sstart<=send)
				{
					double x=sstart++-scf->from_sample;
					x/=cfsize;
					ARES y=scf->ConvertToVolume(x,scf->infade),*samplec=startsample++;

					for(int channel=0;channel<audiobuffer.channelsused;channel++)
					{
						samplec+=audiobuffer.samplesinbuffer*channel;
						*samplec *=y;
					}
				}
				break;
			}
		}

		scf=scf->NextCrossFade();
	}

	audiopattern->Unlock_CrossFades();
}

void RunningAudioFile::ReadRAF(AudioDevice *device)
{
	bytesread=0;

	LONGLONG psampleposition=audiopattern->audioevent.sampleposition;

	if(closeit==false){

		// Buffer Tick Position Start <---> End
		effectparameter.stream_samplestartposition=song->stream_samplestartposition;
		effectparameter.stream_samplestartposition_end=song->stream_sampleendposition;

#ifdef DEBUG
		if(audiobuffer.samplesinbuffer!=device->GetSetSize())
			maingui->MessageBoxError(0,"Read RAF SetSize");
#endif

		// Seek or Read
		bytesread=audiopattern->FillAudioBuffer(song,/*device,*/&audiobuffer,this); // ,audiopattern->CheckIfPlaybackIsAble()==false || audiopattern->track->CheckIfPlaybackIsAble()==false?true:false/);  // Read Samples from File->RAW Buffer

		// Seek=true bytesread=0

		if(audiopattern->itsaclone==true || audiopattern->itsaloop==true)
		{
			AudioPattern *mainaudiopattern=(AudioPattern *)audiopattern->mainpattern;

			if(mainaudiopattern->useoffsetregion==true)
			{
				if(audiopattern->audioevent.sampleposition>=mainaudiopattern->offsetregion.regionend)
					closeit=true; // End of Region ?
			}
			else
			{
				if(mainaudiopattern->audioevent.audioregion){
					// End of Region ?
					if(audiopattern->audioevent.sampleposition>=mainaudiopattern->audioevent.audioregion->regionend)
						closeit=true; // End of Region ?
				}
				else{
					// End of File ?
					if(audiopattern->audioevent.sampleposition>=audiopattern->audioevent.audioefile->samplesperchannel)
						closeit=true;	// Close RunningAudioFile, End Of File reached ?
				}
			}
		}
		else
		{
			if(audiopattern->useoffsetregion==true)
			{
				if(audiopattern->audioevent.sampleposition>=audiopattern->offsetregion.regionend)
					closeit=true; // End of Region ?
			}
			else
			{
				if(audiopattern->audioevent.audioregion){
					// End of Region ?
					if(audiopattern->audioevent.sampleposition>=audiopattern->audioevent.audioregion->regionend)
						closeit=true; // End of Region ?
				}
				else{
					// End of File ?
					if(audiopattern->audioevent.sampleposition>=audiopattern->audioevent.audioefile->samplesperchannel)
						closeit=true;	// Close RunningAudioFile, End Of File reached ?
				}
			}
		}

		if(bytesread>0){// Convert Buffer

			audiobuffer.channelsused=0; // Force copy
			// audiopattern->audioevent.audioefile->ConvertReadBufferToSamples(&audiobuffer);

			audiopattern->audioevent.audioefile->ConvertReadBufferToSamples // convert input buffer to ARES
				(
				audiobuffer.inputbuffer32bit,
				&audiobuffer, // -> Channel Mix Buffer
				audiobuffer.samplesinbuffer,
				channelschannelsnumber[audiopattern->track->io.channel_type]
			//audiobuffer.channelsinbuffer
			);
		}
	}

	if(bytesread>0) {

		if(audiopattern->audioevent.audioregion)
			psampleposition-=audiopattern->audioevent.audioregion->regionstart;

		audiopattern->GetVolumeCurve()->DoEffect(&audiobuffer,psampleposition);

		audiopattern->track->LockStream(); // Avoid MultiCore RAF mix buffer problems

		/*
		if(audiopattern->track->io.channelchannels==0 && audiobuffer.channelsinbuffer==2)
		{
		PanLaw law(&song->project->panlaw_files,0);
		audiobuffer.ExpandMixAudioBuffer(&audiopattern->track->GetAudioFX()->stream[device->useaudiobuffer],audiopattern->track->io.GetChannels(),0,&law);
		}
		else
		*/

#ifdef DEBUG
		//TRACE ("STREAM %d song->stream_bufferindex \n",song->stream_samplestartposition);
		//TRACE ("Index %d\n",song->stream_bufferindex);

		audiobuffer.streamstart=song->stream_samplestartposition;
		audiobuffer.streamend=song->stream_sampleendposition;
#endif

		audiobuffer.ExpandMixAudioBuffer(&audiopattern->track->GetAudioFX()->stream[song->stream_bufferindex],audiopattern->track->io.GetChannels(),0,0); // No TEMP POSSIBLE !

		audiopattern->track->UnlockStream();
	}

	streamstartsampleoffset=0; // reset offset
}

bool RunningAudioFile::InitBuffer(AudioDevice *device,int flag)
{
	/*
	int channels;

	if(audiopattern)channels=audiopattern->audioevent.audioefile->channels;
	else if(audioregion)channels=audioregion->audiohdfile->channels;
	else return 0;
	*/
	if(flag&CREATESTREAMMASTER) // Mastering
		return audiobuffer.Create32BitBuffer_Input(MAXCHANNELSPERCHANNEL,device->GetSetSize());

	if(device)
		return audiobuffer.Create32BitBuffer_Input(MAXCHANNELSPERCHANNEL,device->GetSetSize());

	return false;
}