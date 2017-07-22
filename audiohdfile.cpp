#include "audiohardware.h"
#include "audiohdfile.h"
#include "audiomanager.h"
#include "audiothread.h"
#include "gui.h"
#include "object_song.h"
#include "languagefiles.h"
#include "object_track.h"
#include "semapores.h"
#include "songmain.h"
#include "peakbuffer.h"
#include "audiofile.h"
#include "crossfade.h"
#include "audiopattern.h"
#include "chunks.h"
#include "audiodevice.h"
#include "audioports.h"
#include "audiomaster.h"

#ifdef WIN32
#include <cmath>
#endif}

LONGLONG AudioHDFile::GetStartOfSample(LONGLONG sample)
{
	if(sample<=samplesperchannel)
	{
		return datastart+(sample*samplesize_all_channels); 
	}

	return datastart;
}

AudioRealtime *AudioHDFile::PlayRealtime(guiWindow *from,Seq_Song *song,AudioRegion *region,AudioChannel *audiochannel,bool*endstatus,Seq_Track *usetrack,bool autoclose)
{
	if(errorflag)return 0;

	if(song)
	{
		if(!audiochannel)// Use Default Song Channel
			audiochannel=song->audiosystem.FindPlaybackBusForAudioHDFile(this);

		if(audiochannel)
		{
			/*
			AudioHardwareChannel *hwc;

			if(audiochannel->bus)
			hwc=audiochannel->bus->playback_channel;
			else
			hwc=audiochannel->playback_channel;

			if(hwc)
			{
			*/

			if(song->audiosystem.device)
			{
				AudioRealtime *art=mainaudioreal->AddAudioRealtime(song,song->audiosystem.device,audiochannel,usetrack,this,region,endstatus,0,autoclose,from);
				return art;
			}
			//}
		}
#ifdef _DEBUG
		else
			MessageBox(NULL,"No Audio Channel found for Realtime Audiofile","Error",MB_OK_STYLE);
#endif
	}

	return 0;
}

AudioGFX::AudioGFX(){

	showregionsinside=showmix=true;
	showscale=usebitmap=subpattern=mouseselection=drawborder=dontclearbackground=showline=showregion=undermove=nonreal=false;

	audiopattern=0;
	timeline=0;
	drawcolour=COLOUR_SAMPLESPAINT;
	crossfade=0;
	ostartx=0;
	startposition=0;

	patternvolumecurve=0;
	patternvolumepositions=0;
	showvolumecurve=true;
	samplezoom=1;
}

bool AudioHDFile::ShowAudioFile(AudioGFX *af) // simple, without tempo
{
	guiBitmap *bm=af->usebitmap==true?af->bitmap:&af->win->bitmap;
	SHORT *peak[MAXCHANNELSPERCHANNEL],*peakend[MAXCHANNELSPERCHANNEL];
	int posy[MAXCHANNELSPERCHANNEL],posy2[MAXCHANNELSPERCHANNEL],midy[MAXCHANNELSPERCHANNEL],maxchannels=0;
	LONGLONG peaksize;

	if(!bm)return false;
	if(af->x>=af->x2 || af->y>=af->y2)return false;

	if(af->dontclearbackground==false)
		bm->guiFillRect(af->x,af->y,af->x2,af->y2,COLOUR_SAMPLEBACKGROUND);

	if(!peakbuffer)return false;

	peakbuffer->LockO();

	if(af->showregion==true)
	{
		LONGLONG start=af->regionstart/PEAKBUFFERBLOCKSIZE,end=af->regionend/PEAKBUFFERBLOCKSIZE;
		peaksize=end-start;
	}
	else
		peaksize=peakbuffer->peaksamples;

	if(af->showmix==true){

		if(!peakbuffer->peakmixbuffer)
		{
			peakbuffer->CreatePeakMix();

			if(!peakbuffer->peakmixbuffer)
			{
				peakbuffer->UnlockO();
				return false;
			}
		}

		maxchannels=1;

		if(af->showregion==true)
		{
			LONGLONG start=af->regionstart/PEAKBUFFERBLOCKSIZE,end=af->regionend/PEAKBUFFERBLOCKSIZE;
			peak[0]=&peakbuffer->peakmixbuffer[2*start]; // 2* phase
			peakend[0]=&peakbuffer->peakmixbuffer[2*end]; // 2* phase
		}
		else
		{
			peak[0]=peakbuffer->peakmixbuffer;
			peakend[0]=peak[0]+2*peakbuffer->peaksamples; // 2* phase
		}

		midy[0]=af->y+(af->y2-af->y)/2;
		posy[0]=af->y;
		posy2[0]=af->y2;
	}
	else // multi channel
	{
		maxchannels=channels>MAXCHANNELSPERCHANNEL?MAXCHANNELSPERCHANNEL:channels;

		int ystep=af->y2-af->y;

		ystep-=maxchannels; // Channel Line
		ystep/=maxchannels;

		int y=af->y;

		for(int i=0;i<maxchannels;i++){

			int y2=y+ystep;

			posy[i]=y;
			posy2[i]=y2;

			midy[i]=y+((y2-y)/2);
			y+=ystep;

			if(i+1<maxchannels)bm->guiDrawLineY(y,af->x,af->x2,COLOUR_GREY);

			y++;

			if(!peakbuffer->channelbuffer[i])
			{
				peakbuffer->UnlockO();
				return false;
			}

			if(af->showregion==true)
			{
				LONGLONG start=af->regionstart/PEAKBUFFERBLOCKSIZE,end=af->regionend/PEAKBUFFERBLOCKSIZE;

				peak[i]=&peakbuffer->channelbuffer[i][2*start]; // 2* phase
				peakend[i]=&peakbuffer->channelbuffer[i][2*end]; // 2* phase
			}
			else
			{
				peak[i]=peakbuffer->channelbuffer[i];
				peakend[i]=peak[i]+2*peakbuffer->peaksamples; // 2* phase
			}
		}
	}// multichannel

	bm->SetAPen(af->drawcolour);

	//int y1,y2,lastx,lasty1,lasty2;
	//	SHORT max;
	double ypixel_double=af->y2-af->y;

	ypixel_double/=maxchannels;
	ypixel_double/=32767;
	ypixel_double/=2; // Top/Bottom

	// Jump >>> Samples per Channel ! --->
	/*
	if(af->perstart>0){	
	double h=samplesperchannel;

	h*=af->perstart; // 0.001-1
	h/=PEAKBUFFERSIZE;

	int addj=(int)floor(h+0.5);
	for(int i=0;i<maxchannels;i++)peak[i]+=addj;
	}
	*/

	//double pixelcounter,pixelfactor=PEAKBUFFERSIZE;
	//pixelfactor/=af->samplesperpixel;// spixel pro sec

	LONGLONG sampleposition=af->startposition;
	int xs=af->x;
	bool endreached=false;

	while(xs<=af->samplex2 && endreached==false){

		LONGLONG peakpos=sampleposition/PEAKBUFFERBLOCKSIZE,nsamplepos=sampleposition+af->samplesperpixel;
		LONGLONG npeakpos=(nsamplepos-sampleposition)/PEAKBUFFERBLOCKSIZE;

		sampleposition=nsamplepos;

		if(!npeakpos)
			npeakpos=1; // min 1

		if(peakpos+npeakpos>peakbuffer->peaksamples){
			npeakpos=peakbuffer->peaksamples-peakpos; // Rest
			endreached=true;
		}

		if(!npeakpos)
			break;

		for(int channel=0;channel<maxchannels;channel++)
		{
			SHORT max_m=0,max_p=0,
				*peakcheck=peak[channel]+2*peakpos;

			for(int i=0;i<npeakpos;i++){

				if(*peakcheck>max_p)
					max_p=*peakcheck++;
				else
					peakcheck++;

				if(*peakcheck<max_m)
					max_m=*peakcheck++;
				else
					peakcheck++;
			}

			// Draw
			if(max_p!=0 || max_m!=0) // !=0
				bm->guiDrawLineX(xs,midy[channel]-(int)(ypixel_double*max_p),midy[channel]+(int)(ypixel_double*(-max_m)));
		}

		xs++;

	} // while

	peakbuffer->UnlockO();

	if(samplerate!=mainaudio->GetGlobalSampleRate())
	{
		char h2[NUMBERSTRINGLEN];
		bm->SetTextColour(255,0,0);
		bm->guiDrawText(af->x+1,af->y2-2,af->x2-1,mainvar->ConvertIntToChar(samplerate,h2));
	}

	if(af->showscale==true)
	{
		for(int i=0;i<maxchannels;i++)
		{
			double h=posy2[i]-posy[i];
		}
	}
	else
	{
		// Mid
		for(int i=0;i<maxchannels;i++)bm->guiDrawLineY(midy[i],af->x,af->x2);
	}

	return true;
}

AudioHDFile *AudioHDFile::NextHDFile()
{
	AudioHDFile *ahf;

	mainaudio->LockAudioFiles();
	ahf=(AudioHDFile *)next;
	mainaudio->UnlockAudioFiles();

	return ahf;
}

bool AudioHDFile::ShowAudioFile(Seq_Song *song,AudioGFX *af) // with song tempo
{
	guiBitmap *bm=af->usebitmap==true?af->bitmap:&af->win->bitmap;
	SHORT *peak[MAXCHANNELSPERCHANNEL],*peakend[MAXCHANNELSPERCHANNEL];
	double ypixel_double[MAXCHANNELSPERCHANNEL];
	int chly[MAXCHANNELSPERCHANNEL],chly2[MAXCHANNELSPERCHANNEL],midy[MAXCHANNELSPERCHANNEL],lxp[MAXCHANNELSPERCHANNEL],maxchannels=0;
	bool audiocrossfades=af->audiopattern->CheckIfCrossFadeUsed();

	if(af->x>=af->x2 || af->y>=af->y2)
		return false;

	if(af->drawborder==true)
		bm->guiDrawRect(af->x,af->y,af->x2,af->y2,COLOUR_BLACK);

	if(errorflag)
	{
		bm->guiFillRect(af->x+1,af->y+1,af->x2-1,af->y2-1,COLOUR_YELLOW);
		bm->guiDrawText(af->x+1,af->y2-1,af->x2,ErrorString());

		return false;
	}

	if((!peakbuffer) || peakbuffer->initok==false)return false;

	peakbuffer->LockO();

	if(af->showmix==true){

		if(!peakbuffer->peakmixbuffer)
		{
			peakbuffer->CreatePeakMix();

			if(!peakbuffer->peakmixbuffer){
				peakbuffer->UnlockO();
				return false;
			}
		}

		maxchannels=1;
		peak[0]=peakbuffer->peakmixbuffer;
		peakend[0]=peak[0]+2*peakbuffer->peaksamples; //2* phase

		lxp[0]=-1;
		chly[0]=af->y;
		chly2[0]=af->y2;
		midy[0]=af->y+((af->y2-af->y)/2);
	}
	else // multi channel
	{
		int ystep=af->y2-af->y;

		ystep-=channels; // Channel Line
		ystep/=channels;

		maxchannels=channels>MAXCHANNELSPERCHANNEL?MAXCHANNELSPERCHANNEL:channels;

		int y=af->y;

		for(int i=0;i<maxchannels;i++){

			int y2=y+ystep;

			lxp[i]=-1;

			chly[i]=y;
			chly2[i]=y2;
			midy[i]=y+((y2-y)/2);
			y+=ystep;

			y++;

			if(!(peak[i]=peakbuffer->channelbuffer[i])){
				peakbuffer->UnlockO();
				return false;
			}

			peakend[i]=peak[i]+2*peakbuffer->peaksamples; //2* phase
		}
	}// multichannel

	bm->SetAPen(af->drawcolour);

	for(int i=0;i<maxchannels;i++)
	{
		bm->guiDrawLineY(midy[i],af->x,af->x2);

		ypixel_double[i]=chly2[i]-chly[i];
		ypixel_double[i]/=2*32767;
	}

	LONGLONG sampleposition;

	if(af->audiopattern->GetUseOffSetRegion()==true)
		sampleposition=af->audiopattern->GetOffSetStart(); // Offset <>
	else
		if(af->audiopattern->audioevent.audioregion)
			sampleposition=af->audiopattern->audioevent.audioregion->regionstart; // Region
		else
			sampleposition=0;

	// Jump >>>
	if(af->eventstart<af->start)	
		sampleposition+=song->timetrack.ConvertTicksToTempoSamplesStart(af->eventstart,af->start-af->eventstart);

	{
		LONGLONG sbpos=sampleposition,*samplepositions=0;
		OSTART tickposition=af->start;
		int xs=af->x;
		bool endreached=false;

		if(af->ostartx)
			samplepositions=af->ostartx;
		else{
			if(af->win && af->win->timeline)
				samplepositions=af->win->timeline->sampleposition;
		}

		LONGLONG subfromposition;

		if(af->audiopattern->audioevent.audioregion)
			subfromposition=af->audiopattern->audioevent.audioregion->regionstart;
		else
			subfromposition=0;

		bool withcurve;

		if(!af->patternvolumecurve)
			withcurve=false;
		else
		{
			if(af->patternvolumecurve->fadeinoutactive==true || af->patternvolumecurve->volumeactive==true)
				withcurve=true;
			else
				withcurve=false;
		}

		if(!af->audiopattern->audioevent.audioefile)
			return false;

		LONGLONG maxsamples=af->audiopattern->audioevent.audioefile->samplesperchannel;

		// 1. Sample
		while(xs<=af->samplex2 && endreached==false){

			//	LONGLONG spos=sampleposition;
			LONGLONG peakpos=sampleposition/PEAKBUFFERBLOCKSIZE,npeakpos;

			if(samplepositions)
			{
				LONGLONG tx1=samplepositions[xs],tx2=samplepositions[xs+1],nsamplepos=sampleposition+(tx2-tx1);
				npeakpos=(nsamplepos-sampleposition)/PEAKBUFFERBLOCKSIZE;
				sampleposition=nsamplepos;

#ifdef DEBUG
				if(sampleposition<0)
					maingui->MessageBoxError(0,"<0 1");
#endif
			}
			else
			{
				LONGLONG nsamplepos=sampleposition+song->timetrack.ConvertTicksToTempoSamplesStart(tickposition,af->win->zoom->ticksperpixel); // +Add Tempo 1 pixel

				npeakpos=(nsamplepos-sampleposition)/PEAKBUFFERBLOCKSIZE;
				tickposition+=af->win->zoom->ticksperpixel;
				sampleposition=nsamplepos;

#ifdef DEBUG
				if(sampleposition<0)
					maingui->MessageBoxError(0,"<0 2");
#endif
			}

			if(sampleposition>=maxsamples)
				break;

			if(!npeakpos)
				npeakpos=1; // min 1

			if(peakpos+npeakpos>peakbuffer->peaksamples){
				npeakpos=peakbuffer->peaksamples-peakpos; // Rest
				endreached=true;
			}

			if(npeakpos<0)
				break;

			if(withcurve==false)
			{
				//maingui->MessageBoxOk(0,"No Curve");

				for(int channel=0;channel<maxchannels;channel++)
				{
					SHORT max_m=0,max_p=0,*peakcheck=peak[channel];

					peakcheck+=2*peakpos; // 2*phase

					LONGLONG i=npeakpos;

					if(peakcheck+i>=peakend[channel]) // >= !
						goto exit; // Recording etc...

					if(int loop=i/8)
					{
						i-=loop*8;

						do{

							SHORT pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;
						
						}while(--loop);
					}

					while(i--){

						SHORT pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;
					}

					max_m*=af->samplezoom;
					max_p*=af->samplezoom;

					// Draw
					if(max_m!=0 || max_p!=0) // !=0
						bm->guiDrawLineX(xs,midy[channel]-(int)(ypixel_double[channel]*(double)max_p),midy[channel]+(int)(ypixel_double[channel]*(double)(-max_m)));
				}

			} // no curve
			else // + Fade + Volume Curve + Peak
			{
				
				for(int channel=0;channel<maxchannels;channel++)
				{
					SHORT max_m=0,max_p=0,*peakcheck=peak[channel];

					peakcheck+=2*peakpos; // 2*phase

					LONGLONG i=npeakpos;

					if(peakcheck+i>=peakend[channel]) // >= !
						goto exit; // Recording etc...

					if(int loop=i/8)
					{
						i-=loop*8;

						do{

							SHORT pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

							pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;

						}while(--loop);
					}

					while(i--){

						SHORT pc=*peakcheck++;if(pc>max_p)max_p=pc;
							pc=*peakcheck++;if(pc<max_m)max_m=pc;
					}

					// Add Fades/Curves/Volume
					LONGLONG spos=sampleposition-subfromposition;
					double multi=af->patternvolumecurve->GetFactor(spos); // 0-1

					double dmaxp=max_p;
					dmaxp*=multi;

					dmaxp*=af->samplezoom;
					

					if(dmaxp>=32767)
						dmaxp=32767;

					double dmaxm=max_m;
					dmaxm*=multi;
					dmaxm*=af->samplezoom;

					if(dmaxm<=-32767)
						dmaxm=-32767;

					

					// Draw
					if(dmaxp!=0 || dmaxm!=0) // !=0
						bm->guiDrawLineX(xs,midy[channel]-(int)(ypixel_double[channel]*(double)dmaxp),midy[channel]+(int)(ypixel_double[channel]*(double)(-dmaxm)));
				}

			} // fade in

			xs++;
		}

		// 2. Volume Curve
		if(af->showvolumecurve==true && withcurve==true && af->nonreal==false && af->audiopattern->mediatype!=MEDIATYPE_AUDIO_RECORD)
		{		
			//Volume/Fade In/Fade Out Curve
			double hg=af->y2-af->y;
			LONGLONG midsample=af->audiopattern->GetSamples();
			midsample/=2;

			sampleposition=sbpos;
			tickposition=af->start;
			xs=af->x;
			int lxy=-1;
			endreached=false;
			samplepositions=0;

			if(af->ostartx)
			{
				samplepositions=af->ostartx;
			}
			else
			{
				if(af->win && af->win->timeline)
					samplepositions=af->win->timeline->sampleposition;
			}

			bool checkmidsample=true;

			while(xs<=af->samplex2 && endreached==false){

				LONGLONG spos=sampleposition,peakpos=sampleposition/PEAKBUFFERBLOCKSIZE,npeakpos;

				if(samplepositions)
				{
					LONGLONG tx1=samplepositions[xs],tx2=samplepositions[xs+1],nsamplepos=sampleposition+(tx2-tx1);
					npeakpos=(nsamplepos-sampleposition)/PEAKBUFFERBLOCKSIZE;
					sampleposition=nsamplepos;
				}
				else
				{
					LONGLONG nsamplepos=sampleposition+song->timetrack.ConvertTicksToTempoSamplesStart(tickposition,af->win->zoom->ticksperpixel); // +Add Tempo 1 pixel

					npeakpos=(nsamplepos-sampleposition)/PEAKBUFFERBLOCKSIZE;
					tickposition+=af->win->zoom->ticksperpixel;
					sampleposition=nsamplepos;
				}

				if(!npeakpos)
					npeakpos=1; // min 1

				if(peakpos+npeakpos>peakbuffer->peaksamples){
					npeakpos=peakbuffer->peaksamples-peakpos; // Rest
					endreached=true;
				}

				if(npeakpos<=0)
					break;

				{
					spos-=subfromposition;

					int spostype;

					double h=af->patternvolumecurve->GetFactor(spos,&spostype);

					if(h>1)
						h=1;

					double h2=hg;

					h2*=1-h;

					int midy=af->y+(int)h2;

					switch(spostype)
					{
					case 0: // Fade In
					case 2: // Fade Out
						{
							if(lxy==-1)
								bm->guiDrawPixel(xs,midy,COLOUR_GREEN);
							else
								bm->guiDrawLine(xs-1,lxy,xs,midy,COLOUR_GREEN);
						}
						break;

					case 1: // Volume
						{
							bm->guiDrawPixel(xs,midy,COLOUR_GREEN);
						}
						break;
					}

					
					lxy=midy;

					/*
					// Mid
					if(checkmidsample==true && spos>=midsample && af->patternvolumepositions)
					{
						checkmidsample=false;

						int addw=4;

						int mdx=xs-addw;
						int mdx2=xs+addw;
						int mdy=af->patternvolumepositions->volumey;
						int mdy2=af->patternvolumepositions->volumey2;

						if(mdx<af->x)
							mdx=af->x;

						if(mdx2>af->x2)
							mdx2=af->x2;

						if(mdy<af->y)
							mdy=af->y;

						if(mdy2>af->y2)
							mdy2=af->y2;

						if((!af->patternvolumepositions) || af->patternvolumecurve->editmode==false)
							bm->guiFillRect(mdx,mdy,mdx2,mdy2,COLOUR_BLUE,COLOUR_GREEN);

						if(af->patternvolumepositions)
						{
						
						}
					}
					*/
				}

				xs++;

			} // while Sample


			if(af->patternvolumepositions)
			{

				// Volume
				if(af->patternvolumecurve->volumeactive==true)
				{
					OSTART fs;

					if(af->patternvolumecurve->fadeinoutactive==true)
						fs=song->timetrack.ConvertSamplesToTempoTicks(af->audiopattern->GetPatternStart(),af->patternvolumecurve->fadeinsamples);
					else
						fs=af->audiopattern->GetPatternStart();

					int fsx=af->timeline->ConvertTimeToX(fs);

					OSTART fo;
					
					if(af->patternvolumecurve->fadeinoutactive==true)
					fo=song->timetrack.ConvertSamplesToTempoTicks(af->audiopattern->GetPatternStart(),af->patternvolumecurve->allsamples-af->patternvolumecurve->fadeoutsamples);
					else
						fo=af->audiopattern->GetPatternEnd();

					int fox=af->timeline->ConvertTimeToX(fo);

					if(fsx<0)
						fsx=0;

					if(fox<0)
						fox=af->samplex2;

					double h=af->patternvolumecurve->GetVolume();

					if(h>1)
						h=1;

					double h2=hg;

					h2*=1-h;

					int midy=af->y+(int)h2;

					af->patternvolumepositions->volumeondisplay=true;
					af->patternvolumepositions->volumex=fsx;
					af->patternvolumepositions->volumex2=fox; // end
					af->patternvolumepositions->volumey=midy-maingui->GetFontSizeY()/2;

					if(af->patternvolumepositions->volumey<af->y)
					{
						af->patternvolumepositions->volumey=af->y;
					}

					af->patternvolumepositions->volumey2=af->patternvolumepositions->volumey+maingui->GetFontSizeY();

					int midx=fsx+(fox-fsx)/2;

					if(af->patternvolumecurve->editmode==true)
					{
						char n[NUMBERSTRINGLEN];
						char *h=mainvar->ConvertDoubleToChar(af->patternvolumecurve->dbvolume,n,2);
						int w=bm->GetTextWidth(h);
						
						midx-=w/2;

						int tx=midx+2;

						if(tx>af->x2)
							tx=af->x2-w;

						int y2=midy+maingui->GetFontSizeY();

						if(y2>af->y2)
							y2=af->y2;

						bm->SetTextColour(COLOUR_WHITE);
						bm->guiDrawText(tx,y2,af->x2,h);
					}
					else
					{
						midx-=maingui->GetFontSizeY()/2;
						bm->guiDrawRect(midx,midy,midx+maingui->GetFontSizeY(),midy+maingui->GetFontSizeY(),COLOUR_WHITE);
					}

				}

				// Fade In
				if(af->patternvolumecurve->fadeinoutactive==true)
				{
					OSTART fs=song->timetrack.ConvertSamplesToTempoTicks(af->audiopattern->GetPatternStart(),af->patternvolumecurve->fadeinsamples);
					int fsx=af->timeline->ConvertTimeToX(fs);

					if(fsx>=0)
					{
						double h=af->patternvolumecurve->GetVolume();

						if(h>1)
							h=1;

						double h2=hg;

						h2*=1-h;

						int midy=af->y+(int)h2;

						int my2=midy+maingui->GetFontSizeY();

						if(my2>af->y2)
							my2=af->y2;

						af->patternvolumepositions->fadeinondisplay=true;
						af->patternvolumepositions->fadeinx=fsx;
						af->patternvolumepositions->fadeiny=midy;
						af->patternvolumepositions->fadeinx2=fsx+maingui->GetFontSizeY();
						af->patternvolumepositions->fadeiny2=my2;

						if(af->patternvolumecurve->editmode==true)
						{
							char n[NUMBERSTRINGLEN];

							char *h=mainvar->ConvertDoubleToChar(af->patternvolumecurve->fadeinms/1000,n,2);
							int w=bm->GetTextWidth(h);

							int tx=fsx+2;

							if(tx+w>af->x2)
								tx=af->x2-w;

							bm->SetTextColour(COLOUR_WHITE);
							bm->guiDrawText(tx,my2,af->x2,h);
						}
						else
						{
							bm->guiDrawLineX(fsx,midy,my2,COLOUR_WHITE);
							bm->guiDrawLine(fsx,my2,fsx+maingui->GetFontSizeY(),midy);
							bm->guiDrawLineY(midy+1,fsx,fsx+maingui->GetFontSizeY());
						}
					}
				} // Fade In

				// Fade Out
				if(af->patternvolumecurve->fadeinoutactive==true)
				{
					OSTART fo=song->timetrack.ConvertSamplesToTempoTicks(af->audiopattern->GetPatternStart(),af->patternvolumecurve->allsamples-af->patternvolumecurve->fadeoutsamples);
					int fox=af->timeline->ConvertTimeToX(fo);

					if(fox>=0)
					{
						double h=af->patternvolumecurve->GetVolume();

						if(h>1)
							h=1;

						double h2=hg;
						h2*=1-h;
						int midy=af->y+(int)h2;

						int my2=midy+maingui->GetFontSizeY();

						if(my2>af->y2)
							my2=af->y2;

						af->patternvolumepositions->fadeoutondisplay=true;
						af->patternvolumepositions->fadeoutx=fox-maingui->GetFontSizeY();
						af->patternvolumepositions->fadeouty=midy;
						af->patternvolumepositions->fadeoutx2=fox;
						af->patternvolumepositions->fadeouty2=my2;

						if(af->patternvolumecurve->editmode==true)
						{
							char n[NUMBERSTRINGLEN];

							char *h=mainvar->ConvertDoubleToChar(af->patternvolumecurve->fadeoutms/1000,n,2);
							int w=bm->GetTextWidth(h);

							int tx=fox+2;

							if(tx+w>af->x2)
								tx=af->x2-w;

							bm->SetTextColour(COLOUR_WHITE);
							bm->guiDrawText(tx,my2,af->x2,h);
						}
						else
						{
							bm->guiDrawLineX(fox,midy,my2,COLOUR_WHITE);
							bm->guiDrawLine(fox-maingui->GetFontSizeY(),midy,fox,my2);
							bm->guiDrawLineY(midy+1,fox-maingui->GetFontSizeY(),fox);
						}
					}
				}// Fadeout
			}

		}// if withcurve

	}

exit:

	if(af->subpattern==false && af->showline==true)
	{
		for(int i=0;i<maxchannels;i++)bm->guiDrawLineY(midy[i],af->x,af->x2,af->linecolour);
	}

	// Patterns Volume Curve
#ifdef OLDIE
	if(af->subpattern==false && af->audiopattern->volumecurve.FirstVolumeObject())
	{
		AutomationParameter *ao=af->audiopattern->volumecurve.GetVolumeObjectAt(af->start);

		while(ao){

			if(ao->GetParameterStart()>af->win->endposition)break;

			if(ao->GetParameterStart()>=af->win->startposition){

				int x=af->win->timeline->ConvertTimeToX(ao->GetParameterStart()),lastx=-1;

				if(x>af->x2)break;

				bm->guiDrawLineX(x,af->y,af->y2,COLOUR_RED);

				if(x+5<=af->x2)
				{
					for(int i=0;i<maxchannels;i++)bm->guiDrawRect(x,midy[i]-2,x+4,midy[i]+2,COLOUR_RED);
				}
			}

			ao=ao->NextAutomationParameter();
		}
	}
#endif

	peakbuffer->UnlockO();

	// Regions inside Audio HD File?
	if(af->audiopattern->audioevent.audioefile && af->showregionsinside==true && af->audiopattern->audioevent.audioregion==0 && af->timeline){

		AudioRegion *r=af->audiopattern->audioevent.audioefile->FirstRegion();

		while(r){

			OSTART start=song->timetrack.ConvertSamplesToTempoTicks(af->eventstart,r->regionstart);
			if(start>=af->win->endposition)break;
			OSTART end=song->timetrack.ConvertSamplesToTempoTicks(af->eventstart,r->regionend);

			if(start<af->win->endposition && end>=af->win->startposition) // Inside Header ?
			{
				int sx=start>af->win->startposition?af->timeline->ConvertTimeToX(start):af->timeline->x,
					ex=end<af->win->endposition?af->timeline->ConvertTimeToX(end):af->timeline->x2;

				bm->Draw3DUp(sx,af->y,ex,af->y2);
				bm->guiDrawText(sx,af->y2-1,af->x2,r->regionname);

				if(AudioGFX_Region *afr=new AudioGFX_Region){
					afr->region=r;
					afr->x=sx;
					afr->x2;
					af->regions.AddEndO(afr);
				}
			}

			r=r->NextRegion();
		}
	}

	return true;
}

void AudioHDFile::Decoder(char *buffer,int samples)
{
#ifdef WIN32

	// Big Endian -> Little Endian 

	if(type==TYPE_AIFF || type==TYPE_AIFFC) // Big Endian -> Little
	{
		switch(samplebits)
		{
		case 16:
			{
				unsigned short *c=(unsigned short *)buffer;
				int convert=samples*channels;

				while(convert--)
					*c++=_byteswap_ushort(*c);
			}
			break;

		case 24:
			{
				unsigned char t[3];
				unsigned char *c=(unsigned char *)buffer;
				unsigned char *to=(unsigned char *)buffer;

				int convert=samples*channels;

				while(convert--)
				{
					t[0]=*c++;
					t[1]=*c++;
					t[2]=*c++;

					*to++=t[2];
					*to++=t[1];
					*to++=t[0];
				}
			}
			break;

		case 32:
			{
				unsigned long *c=(unsigned long *)buffer;
				int convert=samples*channels;

				while(convert--)
					*c++=_byteswap_ulong(*c);
			}
			break;

		case 64:
			{
				unsigned __int64 *c=(unsigned __int64 *)buffer;
				int convert=samples*channels;

				while(convert--)
					*c++=_byteswap_uint64(*c);
			}
			break;
		}
	}
#endif

}

void AudioHDFile::Decoder(AudioHardwareBuffer *buffer)
{
	if(type==TYPE_AIFF || type==TYPE_AIFFC) // Big Endian -> Little
	{
		Decoder((char *)buffer->inputbuffer32bit,buffer->samplesinbuffer);
	}
}

void AudioHDFile::CreateFileName()
{
	if((!filename) && name){

		size_t sl=strlen(name);

		if(sl>1){

			sl--;
			char *p=&name[sl],*from=0,*to=p;

			while(sl--){

				if(*p==0x2F || // /
					*p==0x5c || // backslash
					*p==':'
					)
					break;

				from=p--;
			}

			filename=mainvar->GenerateString(from);
		}
	}
}

bool AudioHDFile::LoadHDFile(camxFile *file,bool open)
{
	bool createnew=true;
	AudioHDFile *tohd=this;
	CPOINTER old=0;

	Load(file,&old);

	file->CloseReadChunk();

	if(channels==0)open=true;

	if(createnew==true && open==true)
		Open(OPENAUDIOHD_NOPEAK|OPENAUDIOHD_JUSTINIT); // Init/Open HD File !!! Before Regions !

	// Regions ?
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUDIOHDREGIONS){

		file->ChunkFound();
		int nr=0;
		file->ReadChunk(&nr);

		file->CloseReadChunk();

		while(nr--){

			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_AUDIOHDREGION)
			{
				file->ChunkFound();

				if(AudioRegion *ar=new AudioRegion(this))
				{
					ar->Load(file);

					// Check Existing Region
					AudioRegion *r=tohd->FirstRegion();

					while(r)
					{
						if(ar->regionname && r->regionname &&
							strcmp(ar->regionname,r->regionname)==0 && 
							ar->regionstart==r->regionstart && 
							ar->regionend==r->regionend)
							break;

						r=r->NextRegion();
					}

					if(!r)
						tohd->AddRegion(ar,true); // force=false, ar may be deleted
					else
					{
						// Region exists
						file->ChangeClass((CPOINTER)ar,(CPOINTER)r);
						ar->FreeMemory();
						delete ar;
					}
				}
			}
		}
	}

	camxFile test;

	if(test.OpenRead(GetName())==true)
	{
		file->AddClass((CPOINTER)tohd,old);
		test.Close(true);

		return createnew;
	}

	test.Close(true);
	return false;
}

void AudioHDFile::CreatePeak()
{
	if(errorflag==0 && m_ok==true){

		if(!peakbuffer)
			CreatePeakFile(true);
	}
}

char *AudioHDFile::SetName(char *newname)
{
	if(!newname)return 0;
	if(name)delete name;

	name=mainvar->GenerateString(newname);

	if(filename)delete filename;
	filename=0;

	CreateFileName();

	return name;
}

void AudioHDFile::Open(int flag)
{
	//if(!(flag&OPENAUDIOHD_JUSTINIT))
	//	opencounter++;

	//if(opencounter==1 || (flag&OPENAUDIOHD_JUSTINIT))
	InitHDFile();

	if(!(flag&OPENAUDIOHD_NOPEAK))
		CreatePeak();
}

void AudioHDFile::Load(camxFile *file,CPOINTER *old,Seq_Song *song)
{
	file->ReadChunk(old);

	file->Read_ChunkString(&name);
	file->Read_ChunkString(&info);
	file->Read_ChunkString(&filename);

	//Settings
	file->ReadChunk(&showregionsineditors);
	file->ReadChunk(&camxrecorded);

	// File Type
	file->ReadChunk(&channels);
	file->ReadChunk(&samplebits);
	file->ReadChunk(&samplerate);
	file->ReadChunk(&samplesize_one_channel);
	file->ReadChunk(&samplesize_all_channels);
	file->ReadChunk(&headerlen);
	file->ReadChunk(&flag);
	file->ReadChunk(&errorflag);
	file->ReadChunk(&type);
	file->ReadChunk(&mode);
	file->ReadChunk(&filelength);
	file->ReadChunk(&datalen);
	file->ReadChunk(&datastart);
	file->ReadChunk(&dataend);
	file->ReadChunk(&samplesperchannel);
	//file->ReadChunk(&channelsampleswritten);
	file->ReadChunk(&camximport);

	if(song && filename && camximport==true)
	{
		// Set To Audio Import Directory

		char *h=mainvar->GenerateString(song->directoryname,"\\","Audio Imports","\\",filename);

		if(h)
		{
			if(name)delete name;
			name=h;
		}
	}
	else
		if(song && filename && camxrecorded==true) // Song Dir intern File
		{
			// Set To Audio Recorded Directory

			char *h=mainvar->GenerateString(song->directoryname,"\\",filename);

			if(h)
			{
				if(name)delete name;
				name=h;
			}
		}
}

void AudioHDFile::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOHDFILE);

	file->Save_Chunk((CPOINTER)this);

	file->Save_ChunkString(name);
	file->Save_ChunkString(info);
	file->Save_ChunkString(filename);

	//Settings
	file->Save_Chunk(showregionsineditors);
	file->Save_Chunk(camxrecorded);

	// File Type
	file->Save_Chunk(channels);
	file->Save_Chunk(samplebits);
	file->Save_Chunk(samplerate);
	file->Save_Chunk(samplesize_one_channel);
	file->Save_Chunk(samplesize_all_channels);
	file->Save_Chunk(headerlen);
	file->Save_Chunk(flag);
	file->Save_Chunk(errorflag);
	file->Save_Chunk(type);
	file->Save_Chunk(mode);
	file->Save_Chunk(filelength);
	file->Save_Chunk(datalen);
	file->Save_Chunk(datastart);
	file->Save_Chunk(dataend);
	file->Save_Chunk(samplesperchannel);
	//file->Save_Chunk(channelsampleswritten);

	file->Save_Chunk(camximport);

	file->CloseChunk();

	if((FirstRegion() /*|| FirstVRegion()*/) && (file->flag&SAVE_NOREGIONS)==0) // Save Regions
	{
		file->OpenChunk(CHUNK_AUDIOHDREGIONS);
		file->Save_Chunk(regions.GetCount());
		file->CloseChunk();

		AudioRegion *reg=FirstRegion();

		while(reg){
			reg->Save(file);
			reg=reg->NextRegion();
		}
	}
}

void AudioHDFile::WriteWaveHeader()
{
	m_ok=true;

	mode=FILEMODE_WRITE;

	// Write Header
	unsigned char buff[256];

	buff[0]='R';
	buff[1]='I';
	buff[2]='F';
	buff[3]='F';

	// clen
	buff[4]=buff[5]=buff[6]=buff[7]=0;

	buff[8]='W';
	buff[9]='A';
	buff[10]='V';
	buff[11]='E';

	writefile.Save(buff,12);

	headerlen=12;

	// fmt
	{
		char w_cid[4];

		int w_clen;
		USHORT w_compcode; // default 0
		USHORT w_channels;
		int w_samplerate;
		int w_avgrate;
		USHORT w_blockallign;
		USHORT w_bitspersample;
		//USHORT extraformabytes;

		headerlen+=
			sizeof(w_cid)+
			sizeof(w_clen)+
			sizeof(w_compcode)+
			sizeof(w_channels)+
			sizeof(w_samplerate)+
			sizeof(w_avgrate)+
			sizeof(w_blockallign)+
			sizeof(w_bitspersample);
		// +sizeof(extraformabytes);

		w_cid[0]='f';
		w_cid[1]='m';
		w_cid[2]='t';
		w_cid[3]=' ';

		writefile.Save(w_cid,sizeof(w_cid));

		switch(samplebits)
		{
		case 16:
		case 20:
		case 24:
			w_clen=16;
			break;

		case 32:
		case 64:
			w_clen=18; // + cbsize
			break;
		}

		writefile.Save(&w_clen,sizeof(w_clen));

		switch(samplebits)
		{
		case 16:
		case 20:
		case 24:
			w_compcode=1;
			break;

		case 32:
		case 64:
			{
			samplesarefloat=true;
			w_compcode=3;
			}
			break;
		}

		writefile.Save(&w_compcode,sizeof(w_compcode));

		w_channels=(USHORT)channels;
		writefile.Save(&w_channels,sizeof(w_channels));

		if(externsamplerate==true)
			w_samplerate=samplerate;
		else
			w_samplerate=mainaudio->GetGlobalSampleRate();

		writefile.Save(&w_samplerate,sizeof(w_samplerate));

		w_avgrate=w_samplerate*channels*(samplebits/8);
		writefile.Save(&w_avgrate,sizeof(w_avgrate));

		w_blockallign=(USHORT)(channels*(samplebits/8));
		writefile.Save(&w_blockallign,sizeof(w_blockallign));

		w_bitspersample=(USHORT)samplebits;
		writefile.Save(&w_bitspersample,sizeof(w_bitspersample));

		if(samplebits==32 || samplebits==64) // ext
		{
			USHORT cb_size=0; // 0 ext size

			writefile.Save(&cb_size,sizeof(cb_size));
			headerlen+=sizeof(cb_size);
		}
		// extraformabytes=0;
		// writefile.Save(&extraformabytes,sizeof(extraformabytes));
	}

	// PAD ?

	// data -------
	{
		char cid[4];

		cid[0]='d';
		cid[1]='a';
		cid[2]='t';
		cid[3]='a';

		writefile.Save(cid,4);
		headerlen+=4;

		int clen=0;
		writefile.Save(&clen,4);
		headerlen+=4;
	}

	datastart=headerlen;
	datalen=0;
	dataend=0;
}

void AudioHDFile::SaveBuffer(AudioHardwareBuffer *buffer,int buffersize,void *to,int format,bool *iscleared,bool masterchannel)
{
	// Convert ARES in 16bit etc...
	int conbytes=buffer->ConvertARESTo(to,format,channels,true,iscleared);

#ifdef DEBUG
	if(conbytes!=buffersize)
		maingui->MessageBoxError(0,"Error ConvertARES!");
#endif

	if(conbytes)
	{
		if(buffer->masteroffset!=0)
		{
			// Master Offset

			int offset=channels*buffer->masteroffset; // l/r = channel

			switch(format)
			{
			case MASTERFORMAT_16BIT:
				{
					offset*=sizeof(short);
				}
				break;

			case MASTERFORMAT_32BITFLOAT:
				{
					offset*=sizeof(float);
				}
				break;

			case MASTERFORMAT_64BITFLOAT:
				{
					offset*=sizeof(double);
				}
				break;

			case MASTERFORMAT_24BIT:
				{
					offset*=3;
				}
				break;
			}

			char *offsetto=(char *)to;
			offsetto+=offset;

			to=offsetto;
			conbytes-=offset;
		}

		if(buffer->addpausesamples_ms>0)
		{
			double h=buffer->addpausesamples_ms;
			double sr=samplerate;
			
			h/=1000;
			sr*=h;

			switch(format)
			{
			case MASTERFORMAT_16BIT:
				{
					sr*=sizeof(short);
				}
				break;

			case MASTERFORMAT_32BITFLOAT:
				{
					sr*=sizeof(float);
				}
				break;

			case MASTERFORMAT_64BITFLOAT:
				{
					sr*=sizeof(double);
				}
				break;

			case MASTERFORMAT_24BIT:
				{
					sr*=3;
				}
				break;
			}

			//sr*=channels;

			size_t emptysamples=(size_t)sr;
			size_t buffersize=32768;

			if(char *emptybytes=new char[buffersize])
			{
				memset(emptybytes,0,buffersize);

				size_t loop=emptysamples/buffersize;
				size_t rest=emptysamples-loop*buffersize;

				while(loop--)
				{
					for(int i=0;i<channels;i++)
					{
						writefile.Save(emptybytes,buffersize);
						datalen+=buffersize;
					}
				}

				if(rest)
				{
					for(int i=0;i<channels;i++)
					{
						writefile.Save(emptybytes,rest);
						datalen+=rest;
					}
				}

				delete emptybytes;
			}

		}

		writefile.Save(to,conbytes);
		datalen+=conbytes;
	}
}

void AudioHDFile::WriteHeader()
{
	union 
	{
		unsigned char buff[4];
		long value; //32bit
	}u;

	dataend=datastart+datalen;

	// RIFF CHUNKLEN
	writefile.SeekBegin(4);
	u.value=headerlen+(int)datalen;
	writefile.Save(u.buff,4);

	// data CHUNKLEN
	writefile.SeekBegin(headerlen-4);
	u.value=(long)datalen;
	writefile.Save(u.buff,4);
}

void AudioHDFile::AddRecordPeakBlock(AudioPeakBuffer *hdpeak,SHORT **newchannelbuffer,int add)
{
	if(add<=0 || (!hdpeak))return;

	LONGLONG newpeaksamples=hdpeak->peaksamples+add;
	bool deletenewbuffer=true;

	// Add new Blocks
	for(int i=0;i<channels;i++)
	{
		if(newchannelbuffer[i])
		{
			if(!hdpeak->channelbuffer[i])
			{
				deletenewbuffer=false;

				hdpeak->LockO();

				hdpeak->channelbuffer[i]=newchannelbuffer[i];

				if(channels==1){ // Init Mix Buffer Mono
					hdpeak->peakmixbuffer=newchannelbuffer[i];
					hdpeak->dontdeletepeakmixbuffer=true;
				}

				hdpeak->UnlockO();
			}
			else
			{
				// Add
				if(SHORT *cb=new SHORT[2*newpeaksamples]){

					memcpy(cb,hdpeak->channelbuffer[i],2*sizeof(SHORT)*hdpeak->peaksamples);
					memcpy(&cb[2*hdpeak->peaksamples],newchannelbuffer[i],2*sizeof(SHORT)*add);

					SHORT *old=hdpeak->channelbuffer[i];

					hdpeak->LockO();
					hdpeak->channelbuffer[i]=cb;

					if(channels==1){ // Init Mix Buffer Mono
						hdpeak->peakmixbuffer=cb;
						hdpeak->dontdeletepeakmixbuffer=true;
					}
					hdpeak->UnlockO();

					if(old)delete old;
				}
			}
		}
	}

	// Mix -> Mixbuffer
	if(channels>1) // Stereo or more, Mono has NO Mix
	{
		SHORT *oldmixbuff=hdpeak->peakmixbuffer;

		if(SHORT *newmixbuffer=new SHORT[2*newpeaksamples]){

			if(oldmixbuff)
				memcpy(newmixbuffer,oldmixbuff,2*sizeof(SHORT)*hdpeak->peaksamples);

			SHORT *to=&newmixbuffer[2*hdpeak->peaksamples];

			for(int i=0;i<add;i++){

				SHORT max_p=0,max_m=0;

				for(int ch=0;ch<channels;ch++){
					SHORT *from=&newchannelbuffer[ch][2*i];

					if(*from>max_p)max_p=*from++; // phase +
					else from++;

					if(*from<max_m)max_m=*from; // phase -
				}

				*to++=max_p;
				*to++=max_m;
			}

			hdpeak->LockO();
			hdpeak->peakmixbuffer=newmixbuffer;
			hdpeak->UnlockO();
		}
		else goto exit;

		if(oldmixbuff)delete oldmixbuff;
	}

	hdpeak->LockO();
	hdpeak->peaksamples=newpeaksamples; // set new size, Lock !
	hdpeak->UnlockO();

exit:
	if(deletenewbuffer==false) 
		return;

	// Delete new Buffer
	for(int i=0;i<channels;i++)
		if(newchannelbuffer[i])delete newchannelbuffer[i];
}

void AudioHDFile::ClosePeakBuffer()
{
	if(!peakbuffer)
		return;

	if(peakbuffer->closed==true)
		return;

	SHORT *newchannelbuffer[MAXCHANNELSPERCHANNEL];
	int newchannelbuffercounter[MAXCHANNELSPERCHANNEL];

	for(int i=0;i<recmix.channelsused;i++){
		newchannelbuffer[i]=0;
		newchannelbuffercounter[i]=0;
	}

	peakbuffer->closed=true;

	for(int chl=0;chl<channels;chl++)
	{
		if(recmixcounter[chl])
		{
			recmixpeakbuffercounter[chl]++;

			// Fill Block
			recmixpeakbuffer_p[chl][recmixpeakbuffercounter[chl]]=recmixmaxpeak_p[chl]; // phase
			recmixpeakbuffer_m[chl][recmixpeakbuffercounter[chl]]=recmixmaxpeak_m[chl];
		}

		if(recmixpeakbuffercounter[chl])
		{
			// Init
			if(newchannelbuffer[chl]=new SHORT[2*recmixpeakbuffercounter[chl]])
			{
				SHORT *to=newchannelbuffer[chl];

				// Copy ARES->USHORT
				for(int i2=0;i2<recmixpeakbuffercounter[chl];i2++){
					*to++=(SHORT)(32767*recmixpeakbuffer_p[chl][i2]);
					*to++=(SHORT)(32767*recmixpeakbuffer_m[chl][i2]);
				}

				newchannelbuffercounter[chl]=recmixpeakbuffercounter[chl];
			}
		}
	}

	if(newchannelbuffer[0])
		AddRecordPeakBlock(peakbuffer,&newchannelbuffer[0],newchannelbuffercounter[0]);

#ifdef DEBUG

	LONGLONG clen=mainvar->GetActiveSong()->playbacksettings.cycle_sampleend-mainvar->GetActiveSong()->playbacksettings.cycle_samplestart;
	LONGLONG soll=samplesperchannel;
	soll/=PEAKBUFFERBLOCKSIZE;

	if(soll*PEAKBUFFERBLOCKSIZE!=samplesperchannel)
		soll++;

	LONGLONG ist=peakbuffer->peaksamples;

	if(soll!=ist)
		maingui->MessageBoxOk(0,"Close Rec Peak !=");

#endif

}

void AudioHDFile::CreateRecordPeak(int offset,int writesamples,int crpsize)
{
#ifdef DEBUG
	if(offset>recmix.samplesinbuffer)
		maingui->MessageBoxError(0,"CreateRecordPeak");
#endif

	AudioPeakBuffer *hdpeak=peakbuffer;

	if(!hdpeak){
		hdpeak=new AudioPeakBuffer;
		//	hdpeak->peakopencount=1;
		hdpeak->channels=channels;
		hdpeak->initok=true;
	}

	if(!hdpeak)return;

	SHORT *newchannelbuffer[MAXCHANNELSPERCHANNEL];
	int newchannelbuffercounter[MAXCHANNELSPERCHANNEL];

	for(int i=0;i<channels;i++){
		newchannelbuffer[i]=0;
		newchannelbuffercounter[i]=0;
	}

	short *newblock=0;

	for(int chl=0;chl<channels;chl++)
	{
		ARES *from=recmix.outputbufferARES;

		from+=chl*recmix.samplesinbuffer;
		from+=offset; // [Offset

		int i=writesamples;

		while(i--){

			ARES max=recmix.channelsused==0?0:*from++;

			{
				ARES mp=max;
				if(mp<0)mp=-max;
				if(mp>hdpeak->maxpeakfound)hdpeak->maxpeakfound=mp;
			}

			if(max>recmixmaxpeak_p[chl])
				recmixmaxpeak_p[chl]=max;
			else
				if(max<recmixmaxpeak_m[chl])
					recmixmaxpeak_m[chl]=max;	

			if(++recmixcounter[chl]==PEAKBUFFERBLOCKSIZE)
			{
				recmixcounter[chl]=0; // Reset

				// Fill Block
				recmixpeakbuffer_p[chl][recmixpeakbuffercounter[chl]]=recmixmaxpeak_p[chl]; // phase
				recmixpeakbuffer_m[chl][recmixpeakbuffercounter[chl]]=recmixmaxpeak_m[chl];

				if(++recmixpeakbuffercounter[chl]==crpsize){

					// Add Block To Peak Stream
					recmixpeakbuffercounter[chl]=0; // Reset

					if(!newchannelbuffer[chl])
					{
						// Init
						if(newchannelbuffer[chl]=new SHORT[2*crpsize])
						{
							SHORT *to=newchannelbuffer[chl];

							// Copy ARES->USHORT
							for(int i2=0;i2<crpsize;i2++){
								*to++=(SHORT)(32767*recmixpeakbuffer_p[chl][i2]);
								*to++=(SHORT)(32767*recmixpeakbuffer_m[chl][i2]);
							}

							newchannelbuffercounter[chl]=crpsize;
						}
					}
					else // Add
						if(SHORT *b=new SHORT[2*(newchannelbuffercounter[chl]+crpsize)])
						{
							memcpy(b,newchannelbuffer[chl],sizeof(SHORT)*2*newchannelbuffercounter[chl]); // Copy Old

							delete newchannelbuffer[chl]; // Delete Old

							SHORT *to=newchannelbuffer[chl]=b;
							to+=2*newchannelbuffercounter[chl];

							// Copy ARES->USHORT
							for(int i2=0;i2<crpsize;i2++){
								*to++=(SHORT)(32767*recmixpeakbuffer_p[chl][i2]);
								*to++=(SHORT)(32767*recmixpeakbuffer_m[chl][i2]);
							}

							newchannelbuffercounter[chl]+=crpsize;
						}
				}

				recmixmaxpeak_p[chl]=recmixmaxpeak_m[chl]=0; // reset
			}
		}

	}// for chl

	if(newchannelbuffer[0])
		AddRecordPeakBlock(hdpeak,&newchannelbuffer[0],newchannelbuffercounter[0]);	

	//	TRACE ("Peaks %d \n",hdpeak->peaksamples);

	if(!peakbuffer)peakbuffer=hdpeak;
}

void AudioHDFile::MixRecordPeak(int offset,int size) // Mix New Buffer to existing Peak Buffer, Cycle Mode
{
	if(size<=0)
		return;

	if(!peakbuffer)
		return;

	for(int chl=0;chl<recmix.channelsused;chl++)
	{
		LONGLONG start=cyclestartoffset_samples;
		start/=PEAKBUFFERBLOCKSIZE;

		ARES *from=recmix.outputbufferARES+offset;
		from+=chl*recmix.samplesinbuffer;

		int loop=size/PEAKBUFFERBLOCKSIZE;

		if(loop*PEAKBUFFERBLOCKSIZE!=size)
			loop++; // min 1

		int csize=size;

		while(loop--)
		{
			if(start>=peakbuffer->peaksamples)
			{
				TRACE ("START > peaksamples !!! \n");
				LONGLONG h=samplesperchannel;
				h/=PEAKBUFFERBLOCKSIZE;

				if(h*PEAKBUFFERBLOCKSIZE!=samplesperchannel)
					h++;

				TRACE ("Soll %d\n",h);
				TRACE ("Ist %d\n",peakbuffer->peaksamples);
				TRACE ("Start %d\n",start);

				return;
			}

			ARES max_p=0,max_m=0;

			int i=csize<PEAKBUFFERBLOCKSIZE?csize:PEAKBUFFERBLOCKSIZE;
			csize-=PEAKBUFFERBLOCKSIZE;

			while(i--)
			{
				ARES h=*from++;

				if(h>max_p)
					max_p=h;
				else
					if(h<max_m)
						max_m=h;
			}

			//	TRACE ("MaxP %f MaxM %f \n",max_p,max_m);

			if(max_p>recmixmaxpeak_p[chl])
				recmixmaxpeak_p[chl]=max_p;
			else
				if(max_m<recmixmaxpeak_m[chl])
					recmixmaxpeak_m[chl]=max_m;

			{
				ARES h=-max_m>max_p?-max_m:max_p;

				//SHORT umax=(SHORT)(32767*h);
				if(h>peakbuffer->maxpeakfound)
					peakbuffer->maxpeakfound=h;
			}

			SHORT *check=&peakbuffer->channelbuffer[chl][2*start]; // 2* up/down

			{
				SHORT mp=(SHORT)(max_p*32767);
				if(mp>*check)
					*check=mp;
			}

			{
				SHORT mm=(SHORT)(max_m*32767);
				if(mm<*(check+1))
					*(check+1)=mm;
			}

			start++;

		} // while loop

	}// for channels

	cyclestartoffset_samples+=size;
}

#ifdef OLDIE
void AudioHDFile::CloseRecordPeak()
{
	// Init
	// Rest Samples

	if(recordpeakclosed==true) // Done by Recoring Thread, Cycle Recording ?
		return;

	recordpeakclosed=true;

	for(int chl=0;chl<channels;chl++)
	{
		if(recmixcounter[chl]){

			recmixpeakbuffercounter[chl]++;
			//recmixpeakbuffer[chl][recmixpeakbuffercounter[chl]]=(-recmixmaxpeak_m[chl])>recmixmaxpeak_p[chl]?recmixmaxpeak_m[chl]:recmixmaxpeak_p[chl];
		}
	}

	if(recmixpeakbuffercounter[0]>0)
	{
		int crp=recmixpeakbuffercounter[0];
		SHORT *newchannelbuffer[MAXCHANNELSPERCHANNEL];

		for(int chl=0;chl<channels;chl++)
		{
			if(newchannelbuffer[chl]=new SHORT[2*crp])
			{
				SHORT *to=newchannelbuffer[chl];

				// Copy ARES->USHORT
				for(int i2=0;i2<crp;i2++)
				{
					*to++=(SHORT)(32767*recmixpeakbuffer_p[chl][i2]);
					*to++=(SHORT)(32767*recmixpeakbuffer_m[chl][i2]);

					//newchannelbuffer[chl][i2]=(SHORT)(recmixmaxpeak_p[chl]*32767);
					//newchannelbuffer[chl][i2+1]=(SHORT)(recmixmaxpeak_m[chl]*32767);
				}
			}
		}

		AddRecordPeakBlock(peakbuffer,&newchannelbuffer[0],crp);
	}
}
#endif

int AudioHDFile::Save(void *from,int len)
{
	if((mode&FILEMODE_WRITE) && from && len)
	{
		writefile.Save(from,len);

		if(reccycleloopcounter==0)
			datalen+=len; // +bytes

		return len;
	}

	return 0;
}

// writefile is in Save Mode !
bool AudioHDFile::InitAudioFileSave(char *name,int flag)
{
	if(name)
	{
		bool ok;

		if(flag&OPENSAVE_OPENREADSAVE)
			ok=writefile.OpenReadSave(name);
		else
			ok=writefile.OpenSave(name);

		if(ok==true)
		{
			m_ok=true;
			mode=FILEMODE_WRITE;
			headerlen=0;
			datastart=0;
			datalen=0;
			WriteWaveHeader();

			return true;
		}

#ifdef DEBUG
		maingui->MessageBoxError(0,"InitAudioFileSave 1");
#endif

		errorflag|=AUDIOFILENAME_ERROR;
	}
	else
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"InitAudioFileSave 2");
#endif

		errorflag|=AUDIOFILENAME_ERROR;
	}

	return false;
}

void AudioHDFile::InitAudioFileSave(Seq_Track *track,int flag)
{
	if(!track)return;
	if(!mainaudio->GetActiveDevice())return;

	channels=0;

	if(track->t_audiofx.recordtrack)
	{
		// Track
		channels=channelschannelsnumber[track->io.channel_type];
		samplebits=mainaudio->GetActiveDevice()->bitresolution;
	}
	else
		if(track->GetVIn())
		{
			// Device
			channels=track->GetVIn()->channels;
			samplebits=track->GetVIn()->GetRecordBits();
		}

		/*
		else
		{
		// Max Channel
		Seq_AudioIOPointer *acp=track->GetAudioOut()->FirstChannel();

		while(acp)
		{
		if(acp->channel->GetVIn() && acp->channel->io.bypass_input==false)
		{
		if(acp->channel->record==true && acp->channel->GetVIn()->channels>channels)
		channels=acp->channel->GetVIn()->channels;
		}

		acp=acp->NextChannel();
		}
		}
		*/

		if(channels>0)
		{
			samplesize_all_channels=channels*(samplebits/8);
			samplesize_one_channel=samplebits/8;

			mainvar->DeletePeakFile(name);
			WriteWaveHeader();
		}
		else
		{
			writefile.Close(true); // No Track Parameter
			mode=FILEMODE_CLOSE;
			m_ok=false;
		}
}

bool AudioHDFile::Normalize(ARES maxv,Progress *prog)// prog can be NULL
{
	bool normalized=false;

	if(maxv>0 && maxv<1)
	{
		ARES mul=1/maxv;
		AudioHardwareBuffer buff;
		camxFile file;
		//	file.nobuffer=true;

#define NORMBUFF 16*1024
		LONGLONG i=samplesperchannel,sampleswritten=0;

		if(buff.Create32BitBuffer_Input(channels,NORMBUFF)==true)
		{
			if(file.OpenReadSave(name)==true)
			{
				file.SeekBegin(datastart); // Offset

				while(i>0){

					LONGLONG c;

					if(i<NORMBUFF)
						c=i;
					else
						c=NORMBUFF;

					i-=c;

					LONGLONG jump;

					file.Read(buff.inputbuffer32bit,jump=c*samplesize_all_channels); // -> RAW
					file.SeekCurrent(-jump); // <--- 

					// Convert RAW Stream -> ARES
					// buff.channelsused=0; // Force Simple Copy

					buff.channelsused=0; // Reset
					ConvertReadBufferToSamples(buff.inputbuffer32bit,&buff,c,channels);

					// Do Normalize
					int a=c*channels;
					ARES *ac=buff.outputbufferARES;

					while(a--)
					{
						ARES h=*ac;

						h*=mul;

						// clip
						if(h>1)
							h=1;
						else
							if(h<-1)
								h=-1;

						/*
						#ifdef _DEBUG
						if(h==1 || h==-1)
						{
						int i;

						i=2;
						}

						#endif
						*/

						*ac++ =h;
					}

					// Back to RAW Stream
					ConvertSampleBufferToRAW(&buff,c,channels);
					file.Save(buff.inputbuffer32bit,(int)jump);

					sampleswritten+=c;

					// Calc Percent
					if(prog)
					{
						double h=sampleswritten,h2=samplesperchannel;
						h/=h2;
						h*=100;
						prog->SetPercent(h);
					}

					normalized=true;
				}

				file.Close(true);

				/*
				// Normalize Peak
				if(normalized==true && peakbuffer)
				peakbuffer->Normalize(maxv);
				*/
			}

			buff.Delete32BitBuffer();
		}
	}

	return normalized;
}

int PatternVolumePositions::CheckXY(int ix,int iy)
{
	if(fadeinondisplay==true)
	{
		if(ix>=fadeinx && ix<=fadeinx2 && iy>=fadeiny && iy<=fadeiny2)
			return IN_FADEIN;
	}

	if(fadeoutondisplay==true)
	{
		if(ix>=fadeoutx && ix<=fadeoutx2 && iy>=fadeouty && iy<=fadeouty2)
			return IN_FADEOUT;
	}

	if(volumeondisplay==true)
	{
		if(ix>=volumex && ix<=volumex2 && iy>=volumey && iy<=volumey2)
			return IN_VOLUME;
	}

	
	return 0;
}

bool PatternVolumePositions::SetMouse(guiWindow *win,int type)
{
	switch(type)
	{
	case IN_FADEIN:
		win->SetMouseCursor(CURSOR_LEFT);
		return true;
		break;

	case IN_FADEOUT:
			win->SetMouseCursor(CURSOR_LEFTRIGHT);
			return true;
		break;

	case IN_VOLUME:
		win->SetMouseCursor(CURSOR_UPDOWN);
		return true;
		break;
	}

	return false;
}
