#include "gui.h"
#include "audioeffects.h"
#include "peaks.h"
#include "cshowpeak.h"
#include "audiochannel.h"
#include "object_track.h"
#include "object_song.h"
#include "audiosystem.h"
#include "audioports.h"
#include "songmain.h"
#include "audiohardware.h"
#include "audiohardwarechannel.h"

ARES Peak::GetCurrent()
{
	ARES max=0;

	for(int i=0;i<channels;i++)
	{
		if(p_current[i]>max)
			max=p_current[i];
	}

	return max;
}

ARES Peak::GetCurrentMaximum()
{
	ARES max=GetCurrent();

	if(max>current_max)current_max=max;
	return current_max;
}

void Peak::ResetMax()
{
	for(int i=0;i<channels;i++)
		p_max[i]=0;

	absolut_max=0;
}

ARES Peak::GetAbsMaximum()
{
	ARES max=GetCurrent();

	if(max>absolut_max)
		absolut_max=max;

	return absolut_max;
}

void Peak::Drop(ARES droprate)
{
	if(!channels){
		peakused=false;
		return;
	}

	ARES drop_max=droprate/2;

	for(int i=0;i<channels;i++){

		p_current[i]>droprate?p_current[i]-=droprate:p_current[i]=0;

		if(!p_maxdelaycounter[i]){
			//	if(dropmax==true)
			p_max[i]>drop_max?p_max[i]-=drop_max:p_max[i]=0;
		}
		else
			p_maxdelaycounter[i]--;
	}

	// Max
	ARES mc=0,mm=0;
	bool delayused=false;
	for(int i=0;i<channels;i++){
		if(p_maxdelaycounter[i])delayused=true;
		if(p_current[i]>mc)mc=p_current[i];
		if(p_max[i]>mm)mm=p_max[i];
	}

	current_sum=mc;
	current_max=mm;

	if(delayused==true)
		peakused=true;
	else
		peakused=(mc==0 && mm==0)?false:true;
}

void Peak::ClearPeaks()
{
	for(int i=0;i<channels;i++)
	{
		p_current[i]=0;
		p_max[i]=0;
		p_maxdelaycounter[i]=0;
	}

	current_sum=current_max=absolut_max=0;
	peakused=false;
}

void Peak::SetChannelPeak(int channel,ARES p)
{
	p_current[channel]=p;
	if(p>p_max[channel])p_max[channel]=p;
	peakused=true;
}

void Peak::InitPeak(int c)
{
	if(c>MAXCHANNELSPERCHANNEL)
	{
#ifdef DEBUG
		MessageBox(NULL,"Illegal Peak Channels Init","Error",MB_OK_STYLE);
#endif

		channels=MAXCHANNELSPERCHANNEL;
	}
	else
		channels=c;

	ClearPeaks();
}

void CShowPeak::ShowPeakSum()
{
	changed=false;

	if(maxpeak==true && maxpeakx<maxpeakx2 && maxpeaky<maxpeaky2 && (force==true || p_current_max!=current_max || p_absolutmax!=absolut_max))
	{
		ShowPeakText(current_max,absolut_max);

		if(force==false)
			changed=true;
	}

	if(force==true || changed==true || p_current_sum!=current_sum || p_current_max!=current_max)
	{
		if(force==false)
			changed=true;

		p_current_sum=current_sum;
		p_current_max=current_max;
		p_absolutmax=absolut_max;

		if(x2>x+1 && y2>y && peak)
		{
			if(inputmonitoring==true)
			{
				ARES c=p_current_sum,p=p_current_max;

				if(c>1)c=1; // Input Clipping ?
				if(p>1)p=1;

				double h2=c*(double)(y2-y);
				int maxy=y2-(int)h2;

				h2=p*(double)(y2-y);
				int maxliney=y2-(int)h2;

				if(maxy>y)
					bitmap->guiFillRect(x,y,x2,maxy,COLOUR_INPUTMONITOR_BACKGROUND);

				int x=this->x+1;
				int x2=this->x2-1;

				if(maxy<y2-3)
				{
					for(int i=y2;i>maxy;i-=3)
					{
						bitmap->guiDrawLineY(i,x,x2,COLOUR_INPUT1);
						bitmap->guiDrawLineY(i-1,x,x2,COLOUR_INPUT2);
						if(i-1==maxy)break;
						bitmap->guiDrawLineY(i-2,x,x2,COLOUR_INPUT3);
						if(i-2==maxy)break;
					}
				}
				else
					bitmap->guiFillRect(x,maxy,x2,y2,COLOUR_INPUT1);

				// Max Peak
				if(maxliney>=0 && maxliney<y2)
					bitmap->guiDrawLineY(maxliney,x,x2,COLOUR_INPUTMONITOR_LINEMAX);

				return;
			}

			if(active==true)
			{
				disabled=false;

				// Software Channel
				ARES c=current_sum,p=current_max;

				int redy=-1,greeny=-1;

				if(c>MAXPEAK)c=MAXPEAK;
				if(p>MAXPEAK)p=MAXPEAK;

				if(horiz==true)
				{
					double hmid=(x2-x)*0.25;
					int maxx,midx=x2-(int)floor(hmid+0.5);

					if(p>1){

						double h=p-1,h2=h*(double)(x2-midx);

						maxx=midx+(int)h2;

						if(maxx==midx)
							maxx=midx+1;
					}
					else{
						double h2=p*(double)midx;
						maxx=(int)h2;
					}

					if(c>1){

						double h=c-1,h2=h*(double)(x2-midx);

						redy=midx+(int)h2;

						if(redy==midx)
							redy=midx+1;

						greeny=midx;
					}
					else{
						double h2=c*(double)midx;
						greeny=(int)h2;
					}

					if(greeny>=0 && greeny>x2)greeny=x2;
					if(redy>=0 && redy>x2)redy=x2;
					if(maxx>=0 && maxx>x2)maxx=x2;

					int redrgb=(showactivated==true)?RGB(255,215,215-5):RGB(255,231,231-5),
						//	greenrgb=(showactivated==true)?RGB(44,255,44-5):RGB(155,255,155-5),
						redmax=(showactivated==true)?RGB(255,195,195-5):RGB(255,211,211-5),
						redabs=(showactivated==true)?RGB(255,20,20-5):RGB(255,100,100-5);

					bitmap->guiFillRect_RGB(redy>0?redy:midx,y,x2,y2,output==true?redrgb:RGB(255,253,253));

					if(colour && colour->showcolour==true)
						bitmap->guiFillRect_RGB(x,y,midx-1,y2,colour->rgb);
					else
						bitmap->guiFillRect(x,y,midx-1,y2,COLOUR_AUDIOOUTPEAK_BACKGROUND);


					if(redy>0)
						bitmap->guiFillRect_RGB(midx,y,redy,y2,redabs);

					if(greeny>0 && greeny<x2)
					{
						if(output==true)
							bitmap->guiFillRect(x,y,greeny,y2,showactivated==true?COLOUR_AUDIOOUTPEAK_SELECTED:COLOUR_AUDIOOUTPEAK);
						else
							bitmap->guiFillRect(x,greeny,x2,y2,COLOUR_RED_LIGHT);
					}

					// Max Peak
					if(maxx>0)
					{
						if(redy==-1 && midx<maxx)
							bitmap->guiFillRect_RGB(midx,y,maxx,y2,redmax);

						if(maxx<x2)
							bitmap->guiDrawLineX(maxx,y,y2,COLOUR_BLACK);
					}

				}
				else
				{
					double hmid=(y2-y)*0.25;
					int  maxy=-1,midy=y+(int)floor(hmid+0.5);

					if(p>1){

						double h=p-1,h2=h*(midy-y);

						maxy=midy-(int)h2;
						if(maxy==midy)
							maxy=midy-1;
					}
					else{
						double h2=p*(double)(y2-midy);
						maxy=y2-(int)h2;
					}

					if(c>1){

						double h=c-1,h2=h*(double)(midy-y);
						redy=midy-(int)h2;
						if(redy==midy)
							redy=midy-1;

						greeny=midy;
					}
					else{
						double h2=c*(double)(y2-midy);
						greeny=y2-(int)h2;
					}

					if(greeny>=0 && greeny<y)greeny=y;
					if(redy>=0 && redy<y)redy=y;
					if(maxy>=0 && maxy<y)maxy=y;

					int redrgb=(showactivated==true)?RGB(255,215,215-5):RGB(255,231,231-5),
						//greenrgb=(showactivated==true)?RGB(44,255,44-5):RGB(155,255,155-5),
						redmax=(showactivated==true)?RGB(255,195,195-5):RGB(255,211,211-5),
						redabs=(showactivated==true)?RGB(255,20,20-5):RGB(255,100,100-5);

					bitmap->guiFillRect_RGB(x,y,x2,redy>=0?redy:midy,(output==true)?redrgb:RGB(255,253,253));

					if(colour && colour->showcolour==true)
						bitmap->guiFillRect_RGB(x,midy+1,x2,y2,colour->rgb);
					else
						bitmap->guiFillRect(x,midy+1,x2,y2,COLOUR_AUDIOOUTPEAK_BACKGROUND);

					//bitmap->guiFillRect_RGB(x,midy+1,x2,y2,(colour && colour->showcolour==true)?colour->rgb:RGB(111,111,111));

					int x=this->x+1;
					int x2=this->x2-1;

					if(redy>0)
						bitmap->guiFillRect_RGB(x,redy,x2,midy,redabs);

					if(greeny>0 && greeny<y2)
					{
						if(output==true)
							bitmap->guiFillRect(x,greeny,x2,y2,showactivated==true?COLOUR_AUDIOOUTPEAK_SELECTED:COLOUR_AUDIOOUTPEAK);
						else
							bitmap->guiFillRect(x,greeny,x2,y2,COLOUR_RED_LIGHT);
					}

					// Max Peak
					if(maxy>=0)
					{
						if(redy==-1 && midy>maxy)
							bitmap->guiFillRect_RGB(x,maxy,x2,midy,redmax);

						if(maxy<y2)
							bitmap->guiDrawLineY(maxy,x,x2,COLOUR_BLACK);
					}

					//bitmap->guiDrawRect(x,y,x2,y2,showactivated==true?COLOUR_GREY:COLOUR_BLACK_LIGHT);
				}
			}
			else
			{
				if(disabled==false || force==true)
				{
					disabled=true;

					/*
					int cleartox=x+step*channels;

					for(int i=0;i<channels;i++)
					p_outputmaxpeak[i]=p_outputcurrentpeak[i]=-1;
					*/

					bitmap->guiFillRect(x,y,x2,y2,COLOUR_BLACK);

					//	bitmap->guiFillRect(x,y,x2,y2,COLOUR_GREY);
					//	bitmap->guiDrawLine(x,y,x2,y2,(output==true)?COLOUR_GREEN:COLOUR_RED_LIGHT);
					//	bitmap->guiDrawRect(x,y,x2,y2,COLOUR_BLACK);
				}
			}
		}
	}		
}

AudioIOFX *CShowPeak::GetAudioIO(){return channel?&channel->io:&track->io;}
AudioEffects *CShowPeak::GetFX(){return &GetAudioIO()->audioeffects;}
Peak *CShowPeak::GetOutPeak()
{
	return channel?&channel->mix.peak:track->GetPeak();
}

void CShowPeak::ShowPeakText(ARES cp,ARES mp)
{
	int midx2=maxpeakx2-maxpeakx;
	midx2/=2;

	midx2=maxpeakx+midx2;

	// Current

	if(cp>1)
		bitmap->guiFillRect(maxpeakx,maxpeaky,midx2,maxpeaky2,COLOUR_RED,COLOUR_GREY);
	else
		bitmap->guiFillRect(maxpeakx,maxpeaky,midx2,maxpeaky2,inputmonitoring==true?COLOUR_INPUTMONITOR_BACKGROUND:COLOUR_BLACK,COLOUR_GREY);

	if(char *h=mainaudio->GenerateDBString(cp,false)) 
	{
		bitmap->SetTextColour(COLOUR_WHITE);
		bitmap->guiDrawText(maxpeakx,maxpeaky2,midx2-1,h);
		delete h;
	}

	// Max
	if(mp>1)
		bitmap->guiFillRect(midx2,maxpeaky,maxpeakx2,maxpeaky2,COLOUR_RED,COLOUR_GREY);
	else
		bitmap->guiFillRect(midx2,maxpeaky,maxpeakx2,maxpeaky2,inputmonitoring==true?COLOUR_INPUTMONITOR_BACKGROUND:COLOUR_BLACK,COLOUR_GREY);

	if(char *h=mainaudio->GenerateDBString(mp,false)) 
	{
		bitmap->SetTextColour(COLOUR_YELLOW);

		bitmap->guiDrawLineX(midx2-1,maxpeaky,maxpeaky2,COLOUR_WHITE);
		bitmap->guiDrawLineX(midx2,maxpeaky,maxpeaky2,COLOUR_YELLOW);

		bitmap->guiDrawText(midx2+2,maxpeaky2,maxpeakx2,h);
		delete h;
	}
}

CShowPeak::CShowPeak(){
	colour=0;
	active=true;
	output=true;
	showactivated=false;
	peak=0;
	inputmonitoring=false;
	showMIDI=false;
	currentMIDIinput_data=currentMIDIinput=currentMIDIoutput=0;
	noMIDItext=false;
	maxpeak=false;
	horiz=false; // <-> Transport etc..
}

void CShowPeak::ShowInit(bool isaudio)
{
	if(isaudio==true)
	{
		Seq_Song *song=track?track->song:channel->audiosystem->song;

		if(track && track==song->GetFocusTrack())
			showactivated=true;
		else
			//if(channel && channel->audiosystem->activechannel==channel)
			//	peak.showactivated=true;
			//else
			showactivated=false;

		active=true;
		force=force;
		Peak *frompeak=0;

		inputmonitoring=false;

		if(channel && (channel->audiochannelsystemtype==CHANNELTYPE_DEVICEOUT || channel->audiochannelsystemtype==CHANNELTYPE_DEVICEIN) )
		{
			if(channel->audiochannelsystemtype==CHANNELTYPE_DEVICEIN)
			{
				channel->mix.peak.p_current[0]=channel->mix.peak.current_sum=channel->hardwarechannel->currentpeak;
				channel->mix.peak.p_max[0]=channel->hardwarechannel->peakmax;
			}

			if(channel->audiochannelsystemtype==CHANNELTYPE_DEVICEOUT)
			{
				channel->mix.peak.p_current[0]=channel->mix.peak.current_sum=channel->hardwarechannel->currentpeak;
				channel->mix.peak.p_max[0]=channel->hardwarechannel->peakmax;
			}

			channels=1;
			frompeak=&channel->mix.peak;
		}
		else
		{
			bool tinputmonitoring=GetAudioIO()->tempinputmonitoring;

			if(tinputmonitoring==true && song && GetAudioIO()->inputmonitoring==false && (song->status&Seq_Song::STATUS_PLAY))
				tinputmonitoring=false;

			if(tinputmonitoring==true)
			{
				if(track)
				{
					// memcpy(&peak,,sizeof(peak));
					if(track->t_audiofx.recordtrack)
					{
						frompeak=track->t_audiofx.recordtrack->GetPeak();
						channels=channelschannelsnumber[track->t_audiofx.recordtrack->io.channel_type];
						inputmonitoring=true;
					}
					else
					{
						AudioPort *inputchannel;

						frompeak=&track->io.inputpeaks;

						if(!track->io.in_vchannel)
							inputchannel=track->GetVIn();
						else
							inputchannel=track->io.in_vchannel;

						if(inputchannel)
						{
							channels=inputchannel?inputchannel->channels:0;
							inputmonitoring=true;
						}
						else
							channels=0;
					}

					/*
					if(track->record==true)
					colour=COLOUR_RED;
					*/
				}
			}
			else
			{
				channels=track?track->io.GetChannels():channel->io.GetChannels();
				frompeak=GetOutPeak();// track?&track->io.outpeaks:&channel->io.outpeaks;
				inputmonitoring=false;
			}
		}

		if(peak=frompeak)
		{
			absolut_max=peak->GetAbsMaximum();

			ARES h;

			current_max=current_sum=0; // Reset 

			for(int i=0;i<channels;i++)
			{
				h=current[i]=peak->p_current[i];

				if(h>current_sum)
					current_sum=h;

				h=max[i]=peak->p_max[i];

				if(h>current_max)
					current_max=h;
			}
		}

		//colour=track?track->GetColour():&channel->colour;
		//p_outputcurrentpeak=p_outputcurrentpeak;
		//		p_outputmaxpeak=p_outputmaxpeak;
	}
	else
	{
		showMIDI=true;

		if(channel)
		{
			MIDIinput_data=0;
			MIDIinput=0;

			MIDIoutput=channel->MIDIoutputimpulse;
		}
		else
		{
			MIDIinput_data=track->MIDInputimpulse_data;
			MIDIinput=track->MIDInputimpulse;
			MIDIoutput=track->MIDIoutputimpulse;
		}
	}
}

void CShowPeak::ShowPeak()
{
	changed=false;

	if(x2>x+2 && y2>y+6)
	{
		if(showMIDI==true)
		{
			// MIDI Output
			if(force==true || currentMIDIoutput!=MIDIoutput || currentMIDIinput!=MIDIinput || currentMIDIinput_data!=MIDIinput_data)
			{
				currentMIDIoutput=MIDIoutput;

				if(force==false)
					changed=true;

				double h=y2-y;
				double h2=currentMIDIoutput;

				h2/=100;
				h*=h2;

				int sy=y2-(int)h;

				if(sy>y)
					bitmap->guiFillRect(x,y,x2,sy,COLOUR_MIDIVUBACKGROUND);

				if(currentMIDIoutput)
					bitmap->guiFillRect(x,sy,x2,y2,COLOUR_MIDIVUFOREGROUND);

				if(currentMIDIinput!=MIDIinput || currentMIDIinput_data!=MIDIinput_data)
				{
					currentMIDIinput=MIDIinput;
					currentMIDIinput_data=MIDIinput_data;

					if(force==false)
						changed=true;

					int midx=(x2-x)/2;
					midx+=x;

					if(currentMIDIinput)
					{
						double h=y2-y;
						double h2=currentMIDIinput;

						h2/=100;
						h*=h2;
						int sy=y2-(int)h;
						bitmap->guiFillRect(x,sy,midx,y2,COLOUR_MIDIVUBACKGROUND_INPUT);
					}

					if(currentMIDIinput_data)
					{
						double h=y2-y;
						double h2=currentMIDIinput_data;

						h2/=100;
						h*=h2;
						int sy=y2-(int)h;
						bitmap->guiFillRect(x,sy,midx,y2,COLOUR_MIDIVUFOREGROUND_INPUT);
					}
				}

				/*
				if(noMIDItext==false)
				{
				bitmap->SetTextColour(COLOUR_BLACK);
				bitmap->guiDrawText(x+1,y+maingui->GetFontSizeY(),x2,"MIDI");
				}
				*/

			}

			return;
		}//end MIDI

		// Audio
		if(channels>0)
		{	 
			if(maxpeak==true && maxpeakx<maxpeakx2 && maxpeaky<maxpeaky2 && 
				(force==true || p_current_max!=current_max || p_absolutmax!=absolut_max)
				)
			{
				ShowPeakText(current_max,absolut_max);

				if(force==false)
					changed=true;
			}

			p_current_max=current_max;
			p_absolutmax=absolut_max;

			int bcol=showactivated==true?COLOUR_GREY_DARK:COLOUR_BACKGROUND;

			x++;
			x2--;
			y++;
			y2--;

			int step=((x2-x)+1)-(channels-1); // Line Y
			step/=channels;

			if(step<1)
				step=1;

			/*
			if(force==true)
			{
			bitmap->guiDrawRect(x,y,x+step*channels,y2,bcol);
			}
			*/

			int dx=x,dx2;

			if(inputmonitoring==true)
			{
				// Input
				if(active==true)
				{
					disabled=false;

					for(int i=0;i<channels;i++)
					{
						dx2=dx+(step-1);

						if(dx2>x2)
							break;

						if(force==true)
						{
							xpix[i]=dx;
							xpix2[i]=dx2;

							if(i<channels-1)
								bitmap->guiDrawLineX(dx2+1,y,y2,bcol);
						}

						// Software Channel
						ARES c=current[i],p=max[i];

						if(force==true || p!=p_outputmaxpeak[i] || c!=p_outputcurrentpeak[i])
						{
							if(force==false)
								changed=true;

							p_outputcurrentpeak[i]=c;
							p_outputmaxpeak[i]=p;

							if(c>1)c=1; // Input Clipping ?
							if(p>1)p=1;

							double h2=c*(double)(y2-y);
							int maxy=y2-(int)h2;

							h2=p*(double)(y2-y);
							int maxliney=y2-(int)h2;

							bitmap->guiFillRect(xpix[i],y,xpix2[i],y2,showactivated==true?COLOUR_INPUTMONITOR_BACKGROUND_ACTIVE:COLOUR_INPUTMONITOR_BACKGROUND);

							if(maxy<y2-3)
							{
								for(int hy=y2;hy>maxy;hy-=3)
								{
									bitmap->guiDrawLineY(hy,xpix[i],xpix2[i],COLOUR_INPUT1 /*RGB(150,100,100)*/);

									bitmap->guiDrawLineY(hy-1,xpix[i],xpix2[i],COLOUR_INPUT2 /*RGB(172,122,122)*/);
									if(hy-1==maxy)break;
									bitmap->guiDrawLineY(hy-2,xpix[i],xpix2[i],COLOUR_INPUT3 /*RGB(194,144,144)*/);
									if(hy-2==maxy)break;
								}
							}
							else
							{
								if(maxy<y2)
									bitmap->guiFillRect(xpix[i],maxy,xpix2[i],y2,COLOUR_INPUT1);
							}

							if(maxliney>=0 && maxliney<y2)
								bitmap->guiDrawLineY(maxliney,xpix[i],xpix2[i],COLOUR_INPUTMONITOR_LINEMAX);

							if(i==channels-1)
								break;

							dx=dx2+2;

						}
					}// for


					// End Input Monitoring
				}
				else
				{
					// Output Monitoring

					if(disabled==false || force==true)
					{
						disabled=true;
						//int cleartox=x+step*channels;

						for(int i=0;i<channels;i++)
							p_outputmaxpeak[i]=p_outputcurrentpeak[i]=-1;

						bitmap->guiFillRect(x,y,x2,y2,COLOUR_BLACK);
						//bitmap->guiDrawLine(x,y,cleartox-1,y2,(output==true)?COLOUR_GREEN:COLOUR_RED_LIGHT);
						//bitmap->guiDrawRect(x,y,cleartox-1,y2,COLOUR_BLACK);
					}
				}
			}
			else
			{
				// Output
				double hmid=(y2-y)*0.25;
				int midy=y+(int)floor(hmid+0.5);

				if(active==true)
				{
					disabled=false;

					for(int i=0;i<channels;i++)
					{
						dx2=dx+(step-1);
						if(dx2>x2)
							break;

						if(force==true)
						{
							xpix[i]=dx;
							xpix2[i]=dx2;

							if(i<channels-1)
								bitmap->guiDrawLineX(dx2+1,y,y2,bcol);
						}

						ARES c=current[i],p=max[i];

						if(force==true || p!=p_outputmaxpeak[i] || c!=p_outputcurrentpeak[i])
						{
							p_outputcurrentpeak[i]=c;
							p_outputmaxpeak[i]=p;

							if(force==false)
								changed=true;

							int redy=-1,greeny,maxy;

							if(c>MAXPEAK)c=MAXPEAK;
							if(p>MAXPEAK)p=MAXPEAK;

							if(p>1){

								double h=p-1,h2=h*(double)(midy-y);

								maxy=midy-(int)h2;
								if(maxy==midy)
									maxy=midy-1;
							}
							else{
								double h2=p*(double)(y2-midy);
								maxy=y2-(int)h2;
							}

							if(c>1){

								double h=c-1,h2=h*(double)(midy-y);

								redy=midy-(int)h2;
								if(redy==midy)
									redy=midy-1;

								greeny=midy;
							}
							else{
								double h2=c*(double)(y2-midy);
								greeny=y2-(int)h2;
							}

							// Limits
							if(greeny<y)greeny=y;
							if(redy>=0 && redy<y)redy=y;
							if(maxy<y)maxy=y;

							int redrgb=showactivated==false?RGB(255,202,202-5):RGB(255,231,231-5),
								//	greenrgb=showactivated==true?RGB(44,222,244-5):RGB(155,244,255-5),
								redmax=showactivated==true?RGB(255,195,195-5):RGB(255,211,211-5),
								redabs=showactivated==true?RGB(255,20,20-5):RGB(255,100,100-5);

							bitmap->guiFillRect_RGB(xpix[i],y,xpix2[i],redy>=0?redy:midy,redrgb);
							bitmap->guiFillRect(xpix[i],midy+1,xpix2[i],y2,COLOUR_AUDIOOUTPEAK_BACKGROUND);

							//bitmap->guiFillRect_RGB(xpix[i],midy+1,xpix2[i],/*greeny>=0?greeny:*/y2,showactivated==false?RGB(90,108,108):RGB(110,137,137));

							if(redy>=0)
							{
								bitmap->guiFillRect_RGB(xpix[i],redy,xpix2[i],midy,redabs);
								bitmap->guiFillRect_RGB(xpix[i],midy,xpix2[i],y2,redrgb);
							}
							else
							{
								if(greeny<y2)
								{
									bitmap->guiFillRect(xpix[i],greeny,xpix2[i],y2,showactivated==true?COLOUR_AUDIOOUTPEAK_SELECTED:COLOUR_AUDIOOUTPEAK);
									if(maxy<greeny)
									{
										bitmap->guiFillRect(xpix[i],maxy<midy?midy:maxy,xpix2[i],greeny,showactivated==true?COLOUR_AUDIOOUTPEAK_SELECTED_GREEN:COLOUR_AUDIOOUTPEAK_GREEN);
									}
								}
							}

							// Max Peak Line
							if(maxy<y2)
							{
								//if(redy==-1 && midy>maxy)
								//	bitmap->guiFillRect_RGB(xpix[i],maxy,xpix2[i],midy,redmax);

								bitmap->guiDrawLineY(maxy,xpix[i],xpix2[i],COLOUR_BLACK);
							}

							if(i==channels-1)
								break;

							dx=dx2+2;
						}

					}// for

				}
				else
				{
					if(disabled==false || force==true)
					{
						disabled=true;
						//int cleartox=x+step*channels;

						for(int i=0;i<channels;i++)
							p_outputmaxpeak[i]=p_outputcurrentpeak[i]=-1;

						bitmap->guiFillRect(x,y,x2,y2,COLOUR_BLACK);
						//bitmap->guiDrawLine(x,y,cleartox-1,y2,(output==true)?COLOUR_GREEN:COLOUR_RED_LIGHT);
						//bitmap->guiDrawRect(x,y,cleartox-1,y2,COLOUR_BLACK);
					}
				}
			}
		}
	}
}

// Pan
AudioIOFX *CShowPan::GetAudioIO(){return channel?&channel->io:&track->io;}

bool CShowPan::EditPan(int diff,OSTART time)
{
	if(!diff)return false;

	AudioIOFX *fx=GetAudioIO();

	switch(fx->GetChannels())
	{
	case 0:
	case 1:

		break;

	case 2: // Stereo
		{
			double panv=fx->audioeffects.pan.GetParm(0);

			if(panv==0.5)
			{
				if(diff>0 && diff<8)
					return false;

				if(diff<0 && diff>-8)
					return false;

				if(diff>1)
					diff=1;
				else
					if(diff<-1)
						diff=-1;
			}

			double add=diff*0.01;

			if(panv<0.5 && panv+add>0.5)
				panv=0.5;
			else
				if(panv>0.5 && panv+add<0.5)
					panv=0.5;
				else
				{
					panv+=add;
					if(panv<0)
						panv=0;
					else
						if(panv>1)
							panv=1;
				}

				fx->audioeffects.pan.AutomationEdit(song,time,0,panv);
				return true;

		}
		break;
	}

	return false;
}

bool CShowPan::CheckPanValues()
{
	if(isMIDI==true)
	{
	}
	else
	{
		AudioIOFX *fx=GetAudioIO();

		switch(fx->GetChannels())
		{
		case 2:
			{
				if(stereopanv!=fx->audioeffects.pan.GetParm(0))
					return true;
			}
			break;
		}
	}

	return false;
}

bool CShowPan::ShowPan(bool mouseedit)
{
	bitmap->guiDrawRect(x,y,x2,y2,mouseedit==true?COLOUR_BLACK:COLOUR_GREY);

	int sx=x+1;
	int sx2=x2-1;
	int sy=y+1;
	int sy2=y2-1;

	bitmap->guiFillRect(sx,sy,sx2,sy2,colour);

	if(isMIDI==true)
	{

	}
	else
	{
		AudioIOFX *fx=GetAudioIO();

		switch(fx->GetChannels())
		{
		case 0:
		case 1:

			break;

		case 2: // Stereo
			{
				double panv=fx->audioeffects.pan.GetParm(0);

				stereopanv=panv;

				bitmap->guiDrawLineY(sy+maingui->GetFontSizeY(),x,sx2,COLOUR_GREY_LIGHT);

				int mx=sx2-sx;
				mx/=2;
				mx+=sx;

				//	panv=0.99;

				bitmap->SetTextColour(mouseedit==true?COLOUR_BLACK:COLOUR_BLACK_LIGHT);

				char *l=fx->audioeffects.pan.GetParmValueString(0);

				if(panv<0.5)
				{
					// left
					// 0-0.49
					double h=0.5-panv;
					double h2=0.5;
					h2=h/h2;
					h2*=100;

					h2/=100;
					int x1=mx-sx;
					h2*=x1;
					x1=mx-(int)h2;

					bitmap->guiFillRect(x1,sy,mx,sy+maingui->GetFontSizeY(),COLOUR_GREEN);
				}
				else
				{
					// right
					// 0.51-1
					double h=panv-0.5;
					double h2=0.5;
					h2=h/h2;
					h2*=100;

					h2/=100;
					int x2=sx2-mx;
					h2*=x2;
					x2=mx+(int)h2;

					bitmap->guiFillRect(mx,sy,x2,sy+maingui->GetFontSizeY(),COLOUR_RED);
				}

				if(l)
				{
					bitmap->guiDrawText(x+1,sy2+1,sx2-1,l);
					if(mouseedit==true)
						bitmap->guiDrawText(x+2,sy2+1,sx2-1,l);
				}

				bitmap->guiDrawLineX(mx,sy,sy+maingui->GetFontSizeY(),COLOUR_GREY_DARK);

				return true;
			}
			break;
		}
	}

	return false;
}