#include "audiomaster.h"
#include "object_song.h"
#include "audiohardware.h"
#include "audiohdfile.h"
#include "semapores.h"
#include "gui.h"
#include "settings.h"
#include "languagefiles.h"
#include "songmain.h"
#include "audiofile.h"
#include "audiothread.h"
#include "audiorealtime.h"
#include "object_project.h"
#include "editdata.h"
#include "initplayback.h"
#include "mastering.h"
#include "MIDItimer.h"


extern char *channelchannelsinfo[],*channelchannelsinfo_short[];
extern int channelschannelsnumber[];

#define AUDIOMASTERGADGETID_START GADGET_ID_START+50

enum GIDs
{
	GADGET_SONGNAME=(AUDIOMASTERGADGETID_START+1),
	GADGET_RESETMASTERFILE,
	GADGET_MASTERFILE,
	GADGET_MASTERFILENAME,
	GADGET_MASTERFILEFORMAT,

	GADGET_MASTERCHANNELINFO,
	GADGET_MASTERCHANNEL,

	GADGET_STARTMASTER,

	GADGET_STARTPOSITION_I,
	GADGET_STARTPOSITION,

	GADGET_ENDPOSITION_I,
	GADGET_ENDPOSITION,
	GADGET_MASTERPOSITION,
	GADGET_MASTERLENGTH,
	GADGET_NORMALIZE,
	GADGET_CHECKFIRSTSAMPLE,
	GADGET_SAMPLERATE,
	GADGET_MASTERTYPE,
	GADGET_SELECTFROM,
	GADGETID_STOP,
	GADGETID_PLAY,
	GADGET_STATUS,
	GADGET_PAUSE,
	GADGET_PAUSE_I,
	GADGETID_PAUSEVALUE,
	GADGET_UNFREEZE
};

#define EDIT_STARTPOSITION 1
#define EDIT_ENDPOSITION 2

int AudioHardwareBuffer::ConvertARESTo(void *to,int sampleformat,int channels,bool mixchannels,bool *iscleared)
{
	if(!to)return 0;

	ARES *from=outputbufferARES;

	switch(sampleformat)
	{
	case MASTERFORMAT_16BIT:
		{
			if(channelsused==0)
			{
				if(*iscleared==false)
				{
					*iscleared=true;
					memset(to,0,channels*samplesinbuffer*sizeof(short));
				}
			}
			else
			{
				*iscleared=false;

				if(mixchannels==false)
				{
					// [LL][RR]
					int samples=channels*samplesinbuffer;
					short *to16=(short *)to;

					while(samples--)
					{
						// Clip float to signed 16bit
						ARES h=*from++;

												if(h>0){
							if(h>=1)
								*to16++=SHRT_MAX;
							else
#ifdef ARES64
								*to16++=(short)floor((h*SHRT_MAX)+0.5);
#else
								*to16++=(short)floor((h*SHRT_MAX)+0.5f);
#endif
						}
						else{	
							if(h<=-1)
								*to16++=SHRT_MIN;
							else
#ifdef ARES64
								*to16++=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
								*to16++=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
						}

			
					}
				}
				else
					// Mix L/R etc
					for(int c=0;c<channels;c++)
					{
						short *to16=(short *)to;

						to16+=c;

						for(int i=0;i<samplesinbuffer;i++)
						{
							// Clip float to signed 16bit
							ARES h=*from++;

							if(h>0){
								if(h>=1)
									*to16=SHRT_MAX;
								else
#ifdef ARES64
									*to16=(short)floor((h*SHRT_MAX)+0.5);
#else
									*to16=(short)floor((h*SHRT_MAX)+0.5f);
#endif
							}
							else{	
								if(h<=-1)
									*to16=SHRT_MIN;
								else
#ifdef ARES64
									*to16=(short)floor((h*(SHRT_MAX+1))+0.5);
#else
									*to16=(short)floor((h*(SHRT_MAX+1))+0.5f);
#endif
							}

						/*
							if(h>=1)
								*to16=SHRT_MAX;
							else
								if(h<=-1)
									*to16=SHRT_MIN;
								else
								{
#ifdef ARES64
									*to16=(short)floor(h*SHRT_MAX+0.5);
#else
									*to16=(short)floor(h*SHRT_MAX+0.5f);
#endif
								}
*/

								to16+=channels;
						}
					}

			}

			return channels*samplesinbuffer*sizeof(short);
		}
		break;

	case MASTERFORMAT_32BITFLOAT:
		{
			if(channelsused==0)
			{
				if(*iscleared==false)
				{
					*iscleared=true;
					memset(to,0,channels*samplesinbuffer*sizeof(float));

				}
			}
			else
			{
				*iscleared=false;

				if(mixchannels==false)
				{
					// [LL][RR]
					int samples=channels*samplesinbuffer;
					float *to32=(float *)to;

					while(samples--)
					{
						// Clip float to signed 16bit
						ARES h=*from++;

						if(h>=1)
							*to32++=1;
						else
							if(h<=-1)
								*to32++=-1;
							else
								*to32++=(float)h;
					}
				}
				else
					// Mix L/R etc
					for(int c=0;c<channels;c++)
					{
						float *to32=(float *)to;

						to32+=c;

						for(int i=0;i<samplesinbuffer;i++)
						{
							// Clip float to signed 16bit
							ARES h=*from++;

							if(h>=1)
								*to32=1;
							else
								if(h<=-1)
									*to32=-1;
								else
									*to32=(float)h;

							to32+=channels;
						}
					}
			}
			return channels*samplesinbuffer*sizeof(float);
		}
		break;

	case MASTERFORMAT_64BITFLOAT:
		{
			if(channelsused==0)
			{
				if(*iscleared==false)
				{
					*iscleared=true;
					memset(to,0,channels*samplesinbuffer*sizeof(double));
				}
			}
			else
			{
				*iscleared=false;

				if(mixchannels==false)
				{
					// [LL][RR]
					int samples=channels*samplesinbuffer;
					double *to64=(double *)to;

					while(samples--)
					{
						// Clip float to signed 16bit
						ARES h=*from++;

						if(h>=1)
							*to64++=1;
						else
							if(h<=-1)
								*to64++=-1;
							else
								*to64++=(double)h;
					}
				}
				else
					// Mix L/R etc
					for(int c=0;c<channels;c++)
					{
						double *to64=(double *)to;

						to64+=c;

						for(int i=0;i<samplesinbuffer;i++)
						{
							// Clip float to signed 16bit
							ARES h=*from++;

							if(h>=1)
								*to64=1;
							else
								if(h<=-1)
									*to64=-1;
								else
									*to64=(double)h;

							to64+=channels;
						}
					}
			}
			return channels*samplesinbuffer*sizeof(double);
		}
		break;

	case MASTERFORMAT_24BIT:
		{
#define RANGE24_BIT 16777216
#define H24_BIT (RANGE24_BIT/2)

			union
			{
				char cValue[4];
				long lValue;
			} u;

			if(channelsused==0)
			{
				if(*iscleared==false)
				{
					*iscleared=true;
					memset(to,0,channels*samplesinbuffer*3);
				}
			}
			else
			{
				*iscleared=false;

				if(mixchannels==false)
				{
					// [LL][RR]
					int samples=channels*samplesinbuffer;
					char *toc=(char *)to;

					while(samples--)
					{
						// Clip float to signed 16bit
						ARES h=*from++;

						if(h>=1)
							u.lValue=8388607;
						else
							if(h<=-1)
								u.lValue=-8388608;
							else
							{
#ifdef ARES64
								u.lValue=(long)floor(h*8388607+0.5);
#else
								u.lValue=(long)floor(h*8388607+0.5f);
#endif
							}

							*toc++=u.cValue[0];
							*toc++=u.cValue[1];
							*toc++=u.cValue[2];
					}
				}
				else
					// Mix L/R etc
					for(int c=0;c<channels;c++)
					{
						char *toc=(char *)to;

						toc+=3*c;

						for(int i=0;i<samplesinbuffer;i++)
						{
							// Clip float to signed 16bit
							ARES h=*from++;

							if(h>=1)
								u.lValue=8388607;
							else
								if(h<=-1)
									u.lValue=-8388608;
								else
								{
#ifdef ARES64
									u.lValue=(long)floor(h*8388607+0.5);
#else
									u.lValue=(long)floor(h*8388607+0.5f);
#endif
								}

								*toc++=u.cValue[0];
								*toc++=u.cValue[1];
								*toc++=u.cValue[2];

								toc+=3*(channels-1);
						}
					}
			}

			return channels*samplesinbuffer*3;
		}
		break;
	}

	return 0;
}

guiMenu *Edit_AudioMaster::CreateMenu()
{
	if(menu)menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		menu->AddMenu("Audio Mastering",0);
		menu->AddMenu("Audio Bouncing",0);
	}

	return menu;
}

void Edit_AudioMaster::ShowLength()
{
	if(masterlength)
	{
		if(mastersong)
		{
			// Ticks
			mastersamples=mastersong->timetrack.ConvertTicksToTempoSamplesStart(startticks,endticks-startticks);

			Seq_Pos spos(mastersong->project->standardsmpte);
			char slen[NUMBERSTRINGLEN];

			mainaudio->ConvertSamplesToTime(mastersamples,&spos);
			spos.ConvertToString(mastersong,slen,NUMBERSTRINGLEN);

			if(char *h=mainvar->GenerateString(Cxs[CXS_LENGTH]," ",slen))
			{
				masterlength->ChangeButtonText(h);
				delete h;
			}
		}
	}
}

void Edit_AudioMaster::ShowActiveHDFilePositions()
{
	if(!showsound)
		return;

	guiBitmap *bitmap=&showsound->spritebitmap;
	bitmap->guiFillRect(COLOUR_BLACK);

	if(mastersong->mastering==true || mastersong->masteringpRepared==true || (!masterhdfile))
		return;

	int sx=ConvertSamplePositionX(filestartposition);

	//	bitmap->guiFillRectX0(0,x,showsound->GetHeight(),COLOUR_GREEN);
	bitmap->guiFillRect(sx,0,sx+3,showsound->GetHeight(),COLOUR_WHITE);

	if(frealtimepos!=-1)
	{
		frealtimeposx=ConvertSamplePositionX(frealtimepos);

		if(frealtimeposx>sx)
		{
			bitmap->guiFillRect(sx,0,frealtimeposx,showsound->GetHeight(),COLOUR_GREEN);
			bitmap->guiDrawLine(frealtimeposx,0,frealtimeposx,showsound->GetHeight(),COLOUR_WHITE);
		}
	}
}

void ShowMasterFile_Callback(guiGadget_CW *g,int status)
{
	Edit_AudioMaster *ma=(Edit_AudioMaster *)g->from;

	switch(status)
	{
	case DB_CREATE:
		ma->showsound=g;
		break;

	case DB_PAINT:
		{
			ma->ShowActiveHDFile();
			ma->ShowActiveHDFilePositions();
		}
		break;

	case DB_DOUBLECLICKLEFT:
		ma->Play();
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		{
			ma->MouseClickInGFX();
		}break;

	}
}

void Edit_AudioMaster::ShowStartPosition()
{
	if(statusgadget)
	{
		Seq_Pos spos(Seq_Pos::POSMODE_TIME);
		char slen[NUMBERSTRINGLEN];

		mainaudio->ConvertSamplesToTime(filestartposition,&spos);
		spos.ConvertToString(WindowSong(),slen,NUMBERSTRINGLEN);

		statusgadget->ChangeButtonText(slen);
	}
}

void Edit_AudioMaster::InitGadgets()
{
	glist.SelectForm(0,0);

	int w=2*bitmap.GetTextWidth("DIRECTORY");

	if(freeze==false)
	{
		masterfileclick=glist.AddButton(-1,-1,w,-1,GADGET_MASTERFILE,MODE_ADDDPOINT,Cxs[CXS_SELECTMASTERFILE_I]);
		glist.AddLX();
	
		glist.AddButton(-1,-1,2*bitmap.GetTextWidth("R"),-1,"R",GADGET_RESETMASTERFILE,0,"Reset Master File Name");
		glist.AddLX();

		masterfile=glist.AddString(-1,-1,-1,-1,GADGET_MASTERFILENAME,MODE_RIGHT,0,mastersong->masterfile);
		ShowMasterFile();

		glist.Return();

		mastertype=glist.AddCycle(-1,-1,-1,-1,GADGET_MASTERTYPE,MODE_RIGHT,0,Cxs[CXS_MASTERTYPES]);
		if(mastertype)
		{
			mastertype->AddStringToCycle(Cxs[CXS_MASTERMIX]);
			mastertype->AddStringToCycle(Cxs[CXS_BOUNCESELTRACKS]);
			mastertype->AddStringToCycle(Cxs[CXS_BOUNCESELCHANNELS]);
			mastertype->AddStringToCycle(Cxs[CXS_BOUNCESELTRACKCHANNELS]);

			mastertype->SetCycleSelection(WindowSong()->masteringmode);
		}
		glist.Return();

		sampleformat=glist.AddCycle(-1,-1,-1,-1,GADGET_MASTERFILEFORMAT,MODE_RIGHT,0,Cxs[CXS_MASTERFILEFORMAT_I]);

		if(sampleformat)
		{
			sampleformat->AddStringToCycle("Sample Format 16 Bit");
			sampleformat->AddStringToCycle("Sample Format 24 Bit");
			sampleformat->AddStringToCycle("Sample Format 32 Bit Float");
			sampleformat->AddStringToCycle("Sample Format 64 Bit Float");

			sampleformat->SetCycleSelection(mastersampleformat);
		}

		glist.Return();

		glist.AddButton(-1,-1,w,-1,"Output",GADGET_MASTERCHANNELINFO,MODE_ADDDPOINT);
		glist.AddLX();
		channels=glist.AddButton(-1,-1,w,-1,GADGET_MASTERCHANNEL,MODE_TEXTCENTER|MODE_MENU);

		glist.Return();

	}
	else
	{
		masterfileclick=0;
		masterfile=0;
		mastertype=0;
		sampleformat=0;
		channels=0;
	}

	if(mastersong || mastertrack)
	{
		if(freeze==false)
		{
			glist.AddButton(-1,-1,2*bitmap.GetTextWidth(Cxs[CXS_FROM]),-1,Cxs[CXS_FROM],GADGET_STARTPOSITION_I,MODE_ADDDPOINT);
			glist.AddLX();

			startposition=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGET_STARTPOSITION,WINDOWDISPLAY_MEASURE,0);
			glist.AddLX();
		}
		else
		{
			glist.AddButton(-1,-1,2*bitmap.GetTextWidth(Cxs[CXS_FROM]),-1,"Freeze",GADGET_STARTPOSITION_I,MODE_NOMOUSEOVER);
			glist.AddLX();
			startposition=0;
		}


		glist.AddButton(-1,-1,2*bitmap.GetTextWidth(Cxs[CXS_FROM]),-1,Cxs[CXS_TO],GADGET_ENDPOSITION_I,MODE_ADDDPOINT);
		glist.AddLX();
		endposition=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGET_ENDPOSITION,WINDOWDISPLAY_MEASURE,0);

		glist.AddLX();
		if(endposition)
			endposition->BuildPair(startposition);

		glist.AddButton(-1,-1,-1,-1,Cxs[CXS_SELECTMASTERREGION],GADGET_SELECTFROM,MODE_RIGHT|MODE_MENU);
		glist.Return();

		ShowFromTo();
	}
	else
		startposition=endposition=0;

	if(freeze==false)
	{
		samplerate=glist.AddText(-1,-1,-1,-1,0,GADGET_SAMPLERATE,MODE_RIGHT|MODE_NOMOUSEOVER|MODE_INFO);
		ShowMasterSampleRate();	
		glist.Return();
	}
	else
		samplerate=0;

	masterlength=glist.AddText(-1,-1,-1,-1,"-",GADGET_MASTERLENGTH,MODE_RIGHT|MODE_NOMOUSEOVER|MODE_INFO);
	glist.Return();

	//ShowMasterPosition(true);

	//glist.Return();
	if(freeze==false)
	{
		normalize=glist.AddCheckBox(-1,-1,-1,-1,GADGET_NORMALIZE,MODE_RIGHT,Cxs[CXS_NORMALIZE],Cxs[CXS_NORMALIZE_MI]);
		glist.Return();
		if(normalize)normalize->SetCheckBox(flag_normalize);

		pause=glist.AddCheckBox(-1,-1,-1,-1,GADGET_PAUSE,MODE_RIGHT,Cxs[CXS_MASTERPAUSE]);
		glist.Return();
		if(pause)pause->SetCheckBox(flag_pausesamples);

		glist.AddButton(-1,-1,w,-1,"0 Samples/Pause (ms)",GADGET_PAUSE_I,MODE_ADDDPOINT);
		glist.AddLX();

		pausevalue=glist.AddNumberButton(-1,-1,w,-1,GADGETID_PAUSEVALUE,1,8000,WindowSong()->default_masterpausems,NUMBER_000,0);

		glist.Return();
	}

	if(freeze==false)
	{
		savefirst=glist.AddCheckBox(-1,-1,-1,-1,GADGET_CHECKFIRSTSAMPLE,MODE_RIGHT,Cxs[CXS_SAVEMASTERFILEFS],Cxs[CXS_SAVEMASTERFILEFS_I]);
		if(savefirst)savefirst->SetCheckBox(flag_savefirst);
		glist.Return();
	}
	else
		savefirst=0;

	masterposition=glist.AddButton(-1,-1,-1,-1,Cxs[freeze==true?CXS_NOFREEZEDONE:CXS_NOMASTERINGDONE],GADGET_MASTERPOSITION,MODE_RIGHT,0);
	glist.Return();

	if(freeze==true)
	start=glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),Cxs[CXS_STARTFREEZING],GADGET_STARTMASTER,MODE_RIGHT|MODE_TEXTCENTER,0);
	else
	start=glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),Cxs[CXS_STARTMASTERING],GADGET_STARTMASTER,MODE_RIGHT|MODE_TEXTCENTER,0);

	glist.Return();

	if(freeze==false)
	{
		startsound=glist.AddImageButton(-1,-1,CTRL_XY,CTRL_XY,IMAGE_PLAYBUTTON_SMALL_OFF,GADGETID_PLAY,0,Cxs[CXS_PLAYAUDIOFILECURSOR]);
		glist.AddLX();

		stopsound=glist.AddImageButton(-1,-1,CTRL_XY,CTRL_XY,IMAGE_STOPBUTTON_SMALL_ON,GADGETID_STOP,0,Cxs[CXS_STOPAUDIOFILEPLAYBACK]);
		glist.AddLX();

		frealtimepos=-1;
		statusgadget=glist.AddButton(-1,-1,150,-1,"",GADGET_STATUS,0,"Position");

		ShowStartPosition();

		glist.Return();
	}
	else
	{
		unfreeze=glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),Cxs[CXS_UNFREEZE],GADGET_UNFREEZE,MODE_RIGHT|MODE_TEXTCENTER,0);
		glist.Return();

		startsound=0;
		stopsound=0;
		statusgadget=0;
	}

	ShowLength();
	ShowGadgetStatus();

	//	glist.SelectForm(0,1);

	if(freeze==false)
		showsound=glist.AddChildWindow(-1,-1,-1,-1,MODE_RIGHT|MODE_BOTTOM|MODE_SPRITE,0,&ShowMasterFile_Callback,this);
	else
		showsound=0;

}

void Edit_AudioMaster::ShowMasterFile()
{
	char *h="?";
	char *h2=0;

	switch(mastersong->masteringmode)
	{
	case Seq_Song::SONGMM_MASTERMIX:
		{
			h=Cxs[CXS_MASTERFILE];
			h2=mastersong->masterfile;
		}
		break;

	case Seq_Song::SONGMM_MASTERSELCHANNELS:

		break;

	case Seq_Song::SONGMM_MASTERSELTRACKS:
		{
			h=Cxs[CXS_MASTERDIRECTORYSELTRACKS];
			h2=mastersong->masterdirectoryselectedtracks;
		}
		break;

	case Seq_Song::SONGMM_MASTERSELTRACKSCHANNELS:

		break;
	}

	if(masterfileclick)
		masterfileclick->ChangeButtonText(h);


	if(masterfile)
	{
		if(h2)
			masterfile->SetString(h2);
		else
		{
			masterfile->SetString("-");
			masterfile->Disable();
		}
	}
}

void Edit_AudioMaster::ShowMasterSampleRate()
{
	if(samplerate)
	{
		char help[256],h2[NUMBERSTRINGLEN];

		strcpy(help,Cxs[CXS_SAMPLINGRATE]);

		mainvar->AddString(help,mainvar->ConvertIntToChar(mainaudio->GetGlobalSampleRate(),h2));
		mainvar->AddString(help," Hz");

		samplerate->ChangeButtonText(help);
	}
}

void Edit_AudioMaster::ShowMasterPosition(bool force)
{
	if(masterposition)
	{
		if(mastersong->mastering==true && masterend==false)
		{
			TimeString timestring;

			mastersong->timetrack.CreateTimeString(&timestring,masterticks,Seq_Pos::POSMODE_NORMAL);

			if(char *s=mainvar->GenerateString("Position:",timestring.string))
			{
				masterposition->ChangeButtonText(s);
				delete s;
			}

			masterposition->Enable();
		}
		else
		{

			masterposition->ChangeButtonText(Cxs[CXS_MASTERINGEND]);
		}
	}

	lastmasterticks=masterticks;
}

void Edit_AudioMaster::ShowFromTo()
{
	if(startposition)
	{
		startposition->SetTime(startticks);
		startposition->SetMaxTime(endticks-1);
	}

	if(endposition)
	{
		endposition->SetTime(endticks);
		endposition->SetMinTime(startticks+1);
	}
}

void Edit_AudioMaster::RefreshRealtime_Slow()
{
	if(masterlength)
	{
		if(mastersong)
		{
			// Ticks
			LONGLONG samples=mastersong->timetrack.ConvertTicksToTempoSamplesStart(startticks,endticks-startticks);

			if(samples!=mastersamples)
				ShowLength();
		}
	}

	ShowGadgetStatus();
}

void Edit_AudioMaster::MouseClickInGFX()
{
	LONGLONG pos=ConvertXToPositionX(showsound->SetToXX2(showsound->GetMouseX()));

	if(pos!=filestartposition)
	{
		filestartposition=pos;
		if(audiorealtime)
			Play();

		ShowActiveHDFilePositions();
		showsound->DrawSpriteBlt();
		ShowStartPosition();
	}
}

void Edit_AudioMaster::RefreshRealtime()
{
		if(status==AR_STARTED && endstatus_realtime==true)
	{
		status=AR_STOPPED;
		audiorealtime=0;
	}

	if(mastersong->mastering==true)
	{
		ShowMasterPosition(false);
	}
	else
		if(masterend==true)
		{
			int h=duration_ms;
			int min,sec,ms;

			min=h/(60*1000);
			h-=min*60*1000;

			sec=h/1000;
			h-=sec*1000;

			ms=h;

			char h1[NUMBERSTRINGLEN],h2[NUMBERSTRINGLEN],h3[NUMBERSTRINGLEN];

			if(char *hs=mainvar->GenerateString(Cxs[CXS_MASTERINGEND],"->",mainvar->ConvertIntToChar(min,h1),":",mainvar->ConvertIntToChar(sec,h2),".",mainvar->ConvertIntToChar(ms,h3)) )
			{
				masterposition->ChangeButtonText(hs);
				delete hs;
			}

			DrawDBBlit(showsound);

			if(freeze==true)
				maingui->RefreshAllEditors(mastersong,0);

			masterend=false;
		}

		if(audiorealtime && status==AR_STARTED)
		{
			// Check If Realtime Audio Stopped
			mainthreadcontrol->Lock(CS_audiorealtime);
			if(mainaudioreal->FindAudioRealtime(audiorealtime)==false)
			{
				audiorealtime=0;
				status=AR_STOPPED;
			}
			mainthreadcontrol->Unlock(CS_audiorealtime);

			if(audiorealtime)
			{
				//LONGLONG fpos=(audiorealtime->audiopattern.audioevent.fileposition-audiorealtime->audiopattern.audioevent.audioefile->datastart)/audiorealtime->audiopattern.audioevent.audioefile->samplesize_all_channels;

				if(frealtimepos!=audiorealtime->audiopattern.audioevent.sampleposition)
				{
					frealtimepos=audiorealtime->audiopattern.audioevent.sampleposition;

					if(statusgadget)
					{
						Seq_Pos spos(Seq_Pos::POSMODE_TIME);
						char slen[NUMBERSTRINGLEN];

						mainaudio->ConvertSamplesToTime(frealtimepos,&spos);
						spos.ConvertToString(WindowSong(),slen,NUMBERSTRINGLEN);

						statusgadget->ChangeButtonText(slen);
					}

					int px=ConvertSamplePositionX(frealtimepos);

					if(px!=frealtimeposx)
					{
						ShowActiveHDFilePositions();

						if(showsound)
							showsound->DrawSpriteBlt();
					}
				}
			}
		}
		else
		{
			if(frealtimepos!=-1)
			{
				frealtimepos=-1;

				//ShowStartPosition();

				if(mastersong->mastering==false && mastersong->masteringpRepared==false)
				{
					ShowActiveHDFilePositions();
					DrawDBSpriteBlit(showsound);
				}
				//	statusgadget->ChangeButtonText("- - -");
			}
		}

		if(r_status!=status)
		{
			r_status=status;

			switch(status)
			{
			case AR_STARTED:
				if(startsound)
					startsound->ChangeButtonImage(IMAGE_PLAYBUTTON_ON);

				if(stopsound)
					stopsound->ChangeButtonImage(IMAGE_STOPBUTTON_OFF);
				break;

			case AR_STOPPED:
				if(startsound)
					startsound->ChangeButtonImage(IMAGE_PLAYBUTTON_OFF);

				if(stopsound)
					stopsound->ChangeButtonImage(IMAGE_STOPBUTTON_ON);
				break;
			}
		}
}

void Edit_AudioMaster::InitWindowName()
{
	if(char *h=mainvar->GenerateString(freeze==true?"Freeze/UnFreeze":"Mastering/Bounce",":",mastersong->GetName()))
	{
		guiSetWindowText(h);
		delete h;
	}
}

void Edit_AudioMaster::SongNameRefresh()
{
	InitWindowName();
}

LONGLONG Edit_AudioMaster::ConvertXToPositionX(int x)
{
	if(masterhdfile){

		double w=showsound->GetWidth();
		double h=x;

		h/=w;
		h*=masterhdfile->samplesperchannel;

		return (LONGLONG)floor(h+0.5);
	}

	return 0;
}

void Edit_AudioMaster::MouseButton(int flag)
{
#ifdef OLDIE
	switch(left_mousekey)
	{
	case MOUSEKEY_DOWN:
		{
			if(frame_wave.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
			{
				LONGLONG pos=ConvertXToPositionX(GetMouseX());

				if(pos!=filestartposition)
				{
					filestartposition=pos;
					ShowStartSprite();

					if(audiorealtime)
						Play();
				}
			}
		}
		break;
	}
#endif
}

void Edit_AudioMaster::ShowActiveHDFile()
{
	if(!showsound)
		return;

	guiBitmap *bitmap=&showsound->gbitmap;

	if(masterhdfile)
	{
		if(
			(!masterhdfile->GetName()) || 
			(!mastersong->masterfile) ||
			strcmp(masterhdfile->GetName(),mastersong->masterfile)!=0
			)
			FreeAudioHDFile();
	}

	if((!masterhdfile) && mastersong->masterfile && mastersong->mastering==false && mastersong->masteringpRepared==false)
	{
		if(mainaudio->CheckIfAudioFile(mastersong->masterfile)==true)
		{
			masterhdfile=new AudioHDFile;

			if(masterhdfile)
			{
				masterhdfile->Open(mastersong->masterfile);
				masterhdfile->nopeakfileasfile=true;				
				masterhdfile->CreatePeak();
			}

			//mainaudio->AddAudioFile
		}
	}

	if(masterhdfile)
	{
		if(!masterhdfile->peakbuffer)
		{
			bitmap->guiFillRect(COLOUR_UNUSED);
			bitmap->SetTextColour(COLOUR_YELLOW);
			bitmap->guiDrawText(0,maingui->GetFontSizeY(),bitmap->GetX2(),"Peak File...");

			return;
		}

		if(masterhdfile->errorflag)
		{
			bitmap->guiFillRect(COLOUR_YELLOW);

			if(char *es=mainvar->GenerateString(masterhdfile->ErrorString(),":",masterhdfile->GetName()))
			{			
				bitmap->guiDrawText(0,maingui->GetFontSizeY(),bitmap->GetX2(),es);
				delete es;
			}
		}
		else{
			AudioGFX g;
			double sppixel;

			sppixel=masterhdfile->samplesperchannel;
			sppixel/=showsound->GetWidth();

			g.showmix=false;
			//g.perstart=0; // sample position 0
			g.samplesperpixel=(int)floor(sppixel+0.5); 
			g.win=this;
			g.bitmap=bitmap;
			g.usebitmap=true;

			g.x=0;
			g.samplex2=g.x2=bitmap->GetX2();

			g.y=0;
			g.y2=bitmap->GetY2();

			if(g.y+8<g.y2)
			{
				masterhdfile->ShowAudioFile(&g);
			}
			else
				bitmap->guiFillRect(COLOUR_UNUSED);
		}
	}
	else
	{
		bitmap->guiFillRect(COLOUR_UNUSED);

		if(mastersong->mastering==true || mastersong->masteringpRepared==true)
		{
			bitmap->SetTextColour(COLOUR_YELLOW);
			bitmap->guiDrawText(0,maingui->GetFontSizeY(),bitmap->GetX2(),"Mastering...");
		}
		else
		{
			bitmap->SetTextColour(COLOUR_RED);
			bitmap->guiDrawText(0,maingui->GetFontSizeY(),bitmap->GetX2(),Cxs[CXS_NOMASTERFILE]);
		}
	}

	//	BltGUIBuffer_Frame(&frame_wave);

	//	ShowStartSprite();

}

void Edit_AudioMaster::ShowMasterChannels()
{
	if(channels)
	{
		channels->ChangeButtonText(channelchannelsinfo[WindowSong()->default_masterchannels]);
	}

}

void Edit_AudioMaster::Init()
{
	if(WindowSong()->masterfile==0)
	WindowSong()->InitDefaultMasterName();

	InitWindowName();
	InitGadgets();

	ShowMasterChannels();
}

void Edit_AudioMaster::ShowPeakProgress(double per)
{
#ifdef OLDIE
	if(guibuffer && frame_wave.ondisplay==true){
		char h2[16];

		if(char *h=mainvar->GenerateString(Cxs[CXS_PEAKPROGRESS],":",mainvar->ConvertDoubleToChar(per,h2,2),"%"))
		{
			guibuffer->guiFillRect(frame_wave.x,frame_wave.y,frame_wave.x2,frame_wave.y2,COLOUR_YELLOW);

			if(frame_wave.GetHeight()>(maingui->GetFontSizeY()+4))
				guibuffer->guiDrawText(frame_wave.x,frame_wave.y2-1,frame_wave.x2,h);

			BltGUIBuffer_Frame(&frame_wave);
			delete h;
		}
	}
#endif
}

void Edit_AudioMaster::FreeAudioHDFile()
{
	if(masterhdfile)
	{
		audiopeakthread->StopPeakFile(masterhdfile);

		delete masterhdfile;
		masterhdfile=0;
	}
}

void Edit_AudioMaster::DeInitWindow()
{
	StopPlayback();
	FreeAudioHDFile();

	StopThread();
}

void Edit_AudioMaster::InitClass()
{
	InitForms(FORM_PLAIN1x1);
	resizeable=true;
	ondesktop=true;
	dialogstyle=true;

	minwidth=maingui->GetButtonSizeY(32);

	if(freeze==true)
	{
		maxheight=minheight=maingui->GetButtonSizeY(7);
	}
	else
	minheight=maingui->GetButtonSizeY(18);

	masterend=false;

	masterticks=startticks=mastersong->defaultmasterstart;
	endticks=mastersong->defaultmasterend;

	mastersampleformat=mastersong->default_masterformat;
	flag_normalize=mastersong->default_masternormalize;
	flag_savefirst=mastersong->default_mastersavefirst;
	flag_pausesamples=mastersong->default_masterpausesamples;

	masterhdfile=0;

	filestartposition=0;
	audiorealtime=0;
	status=AR_STOPPED;
	testregion=0;
	mastersamples=0;
}

Edit_AudioMaster::Edit_AudioMaster(Seq_Song *song)
{
	editorid=EDITORTYPE_AUDIOFREEZE;

	mastersong=song;
	freeze=true;

	mastertrack=0;
	masterpattern=0;

	InitClass();
}

Edit_AudioMaster::Edit_AudioMaster(Seq_Song *song,Seq_Track *track,Seq_Pattern *pattern)
{
	editorid=EDITORTYPE_AUDIOMASTER;

	freeze=false;
	mastersong=song;
	mastertrack=track;
	masterpattern=pattern;

	InitClass();
}

void Edit_AudioMaster::DeleteTestRegion()
{
	if(testregion)
	{
		testregion->FreeMemory();
		delete testregion;
		testregion=0;
	}
}

bool Edit_AudioMaster::CheckMastering()
{
	if(freeze==true)
	{
		if(mastersong)
		{
			int sel=0;
			Seq_Track *t=mastersong->FirstTrack();

			while(t)
			{
				if(t->IsSelected()==true)
					sel++;

				t=t->NextTrack();
			}

			return sel>0?true:false;
		}

		return false;
	}
	else
	{
		if(mastersong && mastersong->masterfile && strlen(mastersong->masterfile)>1)
		{
			if(mainaudio->CheckAudioFileUsage(mastersong->masterfile)==false)
				return true;

			maingui->MessageBoxOk(0,Cxs[CXS_FILEUSED]);
		}
	}

	return false;
}

void  Edit_AudioMaster::StopPlayback()
{
	if(status==AR_STARTED)
	{
	status=AR_STOPPED;

	if(audiorealtime){
		mainaudioreal->StopAudioRealtime(audiorealtime,&endstatus_realtime);
		audiorealtime=0;
	}

	DeleteTestRegion();
	}
}

void Edit_AudioMaster::Play()
{
	if(masterhdfile)
	{	
		StopPlayback();

		if(filestartposition!=0 && filestartposition<masterhdfile->samplesperchannel)
		{
			testregion=new AudioRegion(masterhdfile);

			if(testregion)
			{
				// Start Position/End
				testregion->regionstart=filestartposition; 
				testregion->regionend=masterhdfile->samplesperchannel;
				testregion->InitRegion();

				audiorealtime=masterhdfile->PlayRealtime(this,WindowSong(),testregion,0,&endstatus_realtime,0,true);
			}
		}
		else

			audiorealtime=masterhdfile->PlayRealtime(this,WindowSong(),0,0,&endstatus_realtime,0,true);

		if(audiorealtime)
			status=AR_STARTED;
	}
}

int Edit_AudioMaster::ConvertSamplePositionX(LONGLONG pos)
{
	if(masterhdfile && showsound && pos<=masterhdfile->samplesperchannel){

		double w=showsound->GetWidth(),per=pos;

		per/=masterhdfile->samplesperchannel;
		w*=per;

		return (int)floor(w+0.5f);
	}

	return -1;
}

void Edit_AudioMaster::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_RESETMASTERFILE:
		WindowSong()->InitDefaultMasterName();
		ShowMasterFile();
		break;

	case GADGETID_PAUSEVALUE:
		{
			TRACE ("Pause %f \n,",g->GetDoublePos());

			WindowSong()->default_masterpausems=g->GetDoublePos();
		}
		break;

	case GADGET_STARTPOSITION:
		{
			guiGadget_Time *gt=(guiGadget_Time *)g;

			WindowSong()->defaultmasterstart=startticks=gt->t_time;
			if(endposition)
				endposition->SetMinTime(startticks+1);
		}
		break;

	case GADGET_ENDPOSITION:
		{
			guiGadget_Time *gt=(guiGadget_Time *)g;

			WindowSong()->defaultmasterend=endticks=gt->t_time;
			if(startposition)
				startposition->SetMaxTime(endticks-1);
		}
		break;

	case GADGET_MASTERTYPE:
		{
			WindowSong()->masteringmode=g->index;

			StopPlayback();
			ShowMasterFile();
			ShowGadgetStatus();
		}
		break;

	case GADGETID_PLAY:
		Play();
		break;

	case GADGETID_STOP:
		{
			if(audiorealtime)
				StopPlayback();
			else
				if(filestartposition!=0)
				{
					filestartposition=0;

					ShowActiveHDFilePositions();
					DrawDBSpriteBlit(showsound);
				}
		}
		break;

	case GADGET_SELECTFROM:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_selectpos:public guiMenu
				{
				public:
					menu_selectpos(Edit_AudioMaster *m,OSTART p){master=m;position=p;}

					void MenuFunction()
					{
						master->WindowSong()->defaultmasterend=master->endticks=position;
						master->ShowFromTo();
					} //

					Edit_AudioMaster *master;
					OSTART position;
				};

				class menu_selectcycle:public guiMenu
				{
				public:
					menu_selectcycle(Edit_AudioMaster *m){master=m;}

					void MenuFunction()
					{
						if(master->freeze==false)
						master->WindowSong()->defaultmasterstart=master->startticks=master->WindowSong()->playbacksettings.cyclestart;

						master->WindowSong()->defaultmasterend=master->endticks=master->WindowSong()->playbacksettings.cycleend;
						master->ShowFromTo();
					} //

					Edit_AudioMaster *master;
				};

				// SongPosition
				TimeString timestring_c1,timestring_c2;
				char *h;

				OSTART sp=WindowSong()->GetSongPosition();

				song->timetrack.CreateTimeString(&timestring_c1,sp,Seq_Pos::POSMODE_NORMAL);

				h=mainvar->GenerateString("Song Position"," [",timestring_c1.string,"]");

				if(h)
				{
					popmenu->AddFMenu(h,new menu_selectpos(this,sp));
					delete h;
				}

				if(Seq_Marker *mk=WindowSong()->textandmarker.FindMarkerID(Seq_Marker::MARKERFUNC_STOPPLAYBACK))
				{
					song->timetrack.CreateTimeString(&timestring_c1,mk->GetMarkerStart(),Seq_Pos::POSMODE_NORMAL);

					h=mainvar->GenerateString("Song Stop Position"," [",timestring_c1.string,"]");

					if(h)
					{
						popmenu->AddFMenu(h,new menu_selectpos(this,mk->GetMarkerStart()));
						delete h;
					}
				}

				song->timetrack.CreateTimeString(&timestring_c1,WindowSong()->playbacksettings.cyclestart,Seq_Pos::POSMODE_NORMAL);
				song->timetrack.CreateTimeString(&timestring_c2,WindowSong()->playbacksettings.cycleend,Seq_Pos::POSMODE_NORMAL);

				if(freeze==true)
					h=mainvar->GenerateString(Cxs[CXS_CYCLEEND]," [",timestring_c2.string,"]");
				else
					h=mainvar->GenerateString(Cxs[CXS_SELECTCYCLE]," [",timestring_c1.string,"< - >",timestring_c2.string,"]");

				if(h)
				{
					popmenu->AddFMenu(h,new menu_selectcycle(this));
					delete h;
				}

				if(WindowSong()->textandmarker.FirstMarker(Seq_Marker::MARKERTYPE_DOUBLE))
				{
					popmenu->AddLine();

					class menu_selectmarker:public guiMenu
					{
					public:
						menu_selectmarker(Edit_AudioMaster *m,Seq_Marker *mk)
						{
							master=m;
							marker=mk;
						}

						void MenuFunction()
						{
							master->WindowSong()->defaultmasterstart=master->startticks=marker->GetMarkerStart();
							master->WindowSong()->defaultmasterend=master->endticks=marker->GetMarkerEnd();
							master->ShowFromTo();
						} //

						Edit_AudioMaster *master;
						Seq_Marker *marker;
					};

					if(guiMenu *s=popmenu->AddMenu(Cxs[CXS_SELECTMARKER],0))
					{
						Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();

						while(m){

							if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
							{
								TimeString timestring_c1,timestring_c2;

								song->timetrack.CreateTimeString(&timestring_c1,m->GetMarkerStart(),Seq_Pos::POSMODE_NORMAL);
								song->timetrack.CreateTimeString(&timestring_c2,m->GetMarkerEnd(),Seq_Pos::POSMODE_NORMAL);

								char *h=mainvar->GenerateString(m->GetString(),"[",timestring_c1.string,"< - >",timestring_c2.string,"]");
								if(h)
								{
									s->AddFMenu(h,new menu_selectmarker(this,m));
									delete h;
								}
							}

							m=m->NextMarker();
						}
					}
				}

				if(WindowSong()->textandmarker.FirstMarker(Seq_Marker::MARKERTYPE_SINGLE))
				{
					popmenu->AddLine();

					class menu_selectmarkersingle:public guiMenu
					{
					public:
						menu_selectmarkersingle(Edit_AudioMaster *m,Seq_Marker *mk)
						{
							master=m;
							marker=mk;
						}

						void MenuFunction()
						{
							master->WindowSong()->defaultmasterstart=0;
							master->WindowSong()->defaultmasterend=master->endticks=marker->GetMarkerStart();
							master->ShowFromTo();
						} //

						Edit_AudioMaster *master;
						Seq_Marker *marker;
					};

					if(guiMenu *s=popmenu->AddMenu(Cxs[CXS_SELECTMARKER],0))
					{
						Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();

						while(m){

							if(m->markertype==Seq_Marker::MARKERTYPE_SINGLE)
							{
								TimeString timestring_c1,timestring_c2;

								song->timetrack.CreateTimeString(&timestring_c1,0,Seq_Pos::POSMODE_NORMAL);
								song->timetrack.CreateTimeString(&timestring_c2,m->GetMarkerStart(),Seq_Pos::POSMODE_NORMAL);

								char *h=mainvar->GenerateString(m->GetString(),"[",timestring_c1.string,"< - >",timestring_c2.string,"]");
								if(h)
								{
									s->AddFMenu(h,new menu_selectmarkersingle(this,m));
									delete h;
								}
							}

							m=m->NextMarker();
						}
					}
				}

				ShowPopMenu();
			}
		}
		break;

	case GADGET_CHECKFIRSTSAMPLE:
		{
			if(flag_savefirst==true)
				flag_savefirst=false;
			else
				flag_savefirst=true;

			mainsettings->default_mastersavefirst=WindowSong()->default_mastersavefirst=flag_savefirst;
			mainsettings->Save(0);
		}
		break;

	case GADGET_NORMALIZE:
		{
			if(flag_normalize==true)
				flag_normalize=false;
			else
				flag_normalize=true;

			mainsettings->default_masternormalize=WindowSong()->default_masternormalize=flag_normalize;
			mainsettings->Save(0);
		}
		break;

	case GADGET_PAUSE:
		{
			if(flag_pausesamples==true)
				flag_pausesamples=false;
			else
				flag_pausesamples=true;

			mainsettings->default_masterpausesamples=WindowSong()->default_masterpausesamples=flag_pausesamples;
			mainsettings->Save(0);
		}
		break;

	case GADGET_STARTMASTER:
		{
			if(mastersong->mastering==true)
			{
				StopMastering();
			}
			else
			{
				if(mastersong->mastefilename_autoset==true)
				{
					mastersong->InitDefaultMasterName();
					ShowMasterFile();
				}

				if(CheckMastering()==true)
				{
					StopPlayback();
					StartMastering();
				}
			}
		}
		break;

	case GADGET_UNFREEZE:
		UnFreezeTracks();
		break;

	case GADGET_MASTERFILEFORMAT:
		{
			if(mastersong->mastering==false)
			{
				mastersampleformat=g->index;

				mainsettings->default_masterformat=WindowSong()->default_masterformat=mastersampleformat;
				mainsettings->Save(0);
			}
			else
				sampleformat->SetCycleSelection(mastersampleformat); // dont change during mastering
		}
		break;

	case GADGET_MASTERFILENAME:
		{
			switch(mastersong->masteringmode)
			{
			case Seq_Song::SONGMM_MASTERMIX:
				{
					StopPlayback();

					mastersong->SetMasterFileName(g->string);
					mastersong->mastefilename_autoset=false;

					ShowGadgetStatus();

					//ShowActiveHDFile();
					DrawDBBlit(showsound);
				}
				break;

			case Seq_Song::SONGMM_MASTERSELCHANNELS:
				{

				}
				break;

			case Seq_Song::SONGMM_MASTERSELTRACKS:
				{
					if(mastersong->masterdirectoryselectedtracks)
						delete mastersong->masterdirectoryselectedtracks;

					mastersong->masterdirectoryselectedtracks=mainvar->GenerateString(g->string);
				}
				break;

			case Seq_Song::SONGMM_MASTERSELTRACKSCHANNELS:

				break;
			}
		}
		break;

	case GADGET_MASTERFILE:
		if(mastersong->mastering==false)
		{
			if(char *h=mainvar->GenerateString("Master_",mastersong->GetName()))
			{
				switch(mastersong->masteringmode)
				{
				case Seq_Song::SONGMM_MASTERMIX:
					{
						camxFile write;

						if(write.OpenFileRequester(0,this,Cxs[CXS_SELECTMASTERFILE_I],write.AllFiles(camxFile::FT_WAVES),false,h)==true)
						{
							write.AddToFileName(".wav");

							StopPlayback();

							mastersong->SetMasterFileName(write.filereqname);
							mastersong->mastefilename_autoset=false;

							ShowMasterFile();
							ShowGadgetStatus();
							ShowActiveHDFile();
						}

						write.Close(true);

					}
					break;

				case Seq_Song::SONGMM_MASTERSELCHANNELS:

					break;

				case Seq_Song::SONGMM_MASTERSELTRACKS:
					{
						camxFile write;

						if(write.SelectDirectory(this,0,Cxs[CXS_SELECTMASTERFILE_I])==true)
						{
							if(mastersong->masterdirectoryselectedtracks)delete mastersong->masterdirectoryselectedtracks;
							mastersong->masterdirectoryselectedtracks=mainvar->GenerateString(write.filereqname);

							ShowMasterFile();
						}

						write.Close(true);
					}
					break;

				case Seq_Song::SONGMM_MASTERSELTRACKSCHANNELS:

					break;
				}

				delete h;
			}
		}
		break;
	}
}

bool Edit_AudioMaster::CanMastering()
{
	if(mastersong->mastering==true)
		return false;

	if(freeze==true)
	{
		Seq_Track *t=mastersong->FirstTrack();
		while(t)
		{
			if(t->IsSelected()==true && (t->frozen==false || t->trackfreezeendposition!=endticks) )
			{
				if(t->IsTrackAudioFreezeTrack()==true)
					return true;
			}

			t=t->NextTrack();
		}

		return false;
	}
	else
		switch(mastersong->masteringmode)
	{
		case Seq_Song::SONGMM_MASTERMIX:
			{
				if(mastersong->masterfile && strlen(mastersong->masterfile))
				{
					Seq_Track *t=mastersong->FirstTrack();
					while(t)
					{
						if(t->IsTrackAudioMasterTrack()==true)
							return true;

						t=t->NextTrack();
					}
				}
			}
			break;

		case Seq_Song::SONGMM_MASTERSELCHANNELS:

			break;

		case Seq_Song::SONGMM_MASTERSELTRACKS:
			{
				if(mastersong->masterdirectoryselectedtracks && strlen(mastersong->masterdirectoryselectedtracks))
				{
					Seq_Track *t=mastersong->FirstTrack();
					while(t)
					{
						if(t->IsSelected()==true)
						{
							if(t->IsTrackAudioMasterTrack()==true)
								return true;
						}

						t=t->NextTrack();
					}
				}
			}
			break;

		case Seq_Song::SONGMM_MASTERSELTRACKSCHANNELS:

			break;
	}

	return false;
}

void Edit_AudioMaster::ShowGadgetStatus()
{
	startcanbedone=CanMastering();

	if(mastersong->mastering==true)
	{
		if(start)
			start->ChangeButtonText(Cxs[CXS_STOPMASTERING]);
	}
	else
	{
		if(start)
		{
			if(freeze==true && startcanbedone==true)
			{
				int c=0;
				Seq_Track *t=mastersong->FirstTrack();
				while(t)
				{
					if( t->IsSelected()==true && 
						(t->frozen==false || t->trackfreezeendposition!=endticks)
						)
					{
						if(t->IsTrackAudioFreezeTrack()==true)
							c++;
					}

					t=t->NextTrack();
				}

				char nr[NUMBERSTRINGLEN];

				if(char *h=mainvar->GenerateString(Cxs[CXS_STARTFREEZING],"[",mainvar->ConvertIntToChar(c,nr),"]"))
				{
					start->ChangeButtonText(h);
					delete h;
				}
			}
			else
				start->ChangeButtonText(Cxs[CXS_STARTMASTERING]);

			startcanbedone==true?start->Enable():start->Disable();
		}
	}

	if(freeze==true)
	{
		if(unfreeze)
		{
			int c=0;

			Seq_Track *t=mastersong->FirstTrack();
			while(t)
			{
				if(t->IsSelected()==true && t->frozen==true)
					c++;

				t=t->NextTrack();
			}

			if(c)
			{
				char nr[NUMBERSTRINGLEN];

				if(char *h=mainvar->GenerateString(Cxs[CXS_UNFREEZE],"[",mainvar->ConvertIntToChar(c,nr),"]"))
				{
				unfreeze->ChangeButtonText(h);
				delete h;
				}

				unfreeze->Enable();
			}
			else
			{
				unfreeze->Disable();
			}
		}
	}
}

void Edit_AudioMaster::UnFreezeFrozenTracks()
{
	if(mastersong->IsRecording()==false)
	{
		Seq_Track *t=mastersong->FirstTrack();
		while(t)
		{
			if(t->IsSelected()==true && t->frozen==true && t->trackfreezeendposition!=endticks)
				goto go;

			t=t->NextTrack();
		}
		return;

go:
		mastersong->StopSelected();

		int c=0;
		t=mastersong->FirstTrack();
		while(t)
		{
			if(t->IsSelected()==true && t->frozen==true && t->trackfreezeendposition!=endticks)
			{
				if(t->UnFreezeTrack()==true)
					c++;
			}

			t=t->NextTrack();
		}

		mastersong->PRepairPlayback(mastersong->GetSongPosition(),MEDIATYPE_ALL);
		maingui->RefreshAllEditors(mastersong,0);
	}
}

void Edit_AudioMaster::UnFreezeTracks()
{
	if(mastersong->IsRecording()==false)
	{
		Seq_Track *t=mastersong->FirstTrack();
		while(t)
		{
			if(t->IsSelected()==true && t->frozen==true)
				goto go;

			t=t->NextTrack();
		}
		return;

go:
		mastersong->StopSelected();

		int c=0;
		t=mastersong->FirstTrack();
		while(t)
		{
			if(t->IsSelected()==true && t->frozen==true)
			{
				if(t->UnFreezeTrack()==true)
					c++;
			}

			t=t->NextTrack();
		}

		mastersong->PRepairPlayback(mastersong->GetSongPosition(),MEDIATYPE_ALL);
		maingui->RefreshAllEditors(mastersong,0);

	}
}

// MAIN AUDIO -> AudioFile Mastering
void Edit_AudioMaster::StartMastering()
{
	if(mastersong->audiosystem.device==0)
	{
		maingui->MessageBoxError(0,Cxs[CXS_NODEVICE]);
		return;
	}

	if(freeze==true || 
		(mastersong->masterfile && strlen(mastersong->masterfile)>0 && mastersong->mastering==false)
		)
	{
		// Create Project Dirs...
		char *mp=mainvar->GenerateString(mastersong->project->projectdirectory,"\\","Master Audio");

		if(mp)
		{
			mainvar->CreateNewDirectory(mp);
			delete mp;
		}

		bool masteringable=false;

		if(mastersong)
		{
			masteringable=CheckMastering();
		}

		if(masteringable==true)
		{
			bool go=true;

			if(mastersong->status!=Seq_Song::STATUS_STOP)
				go=maingui->MessageBoxYesNo(this,Cxs[CXS_STOPSONGFORMASTERING]);

			if(go==true)
			{
				mastersong->freeze=freeze;
				mastersong->masteringpRepared=true;

				StopPlayback();
				FreeAudioHDFile();

				mainvar->SetActiveSong(mastersong);

				if(mainvar->GetActiveSong()==mastersong)
				{
					if(freeze==true)
					{
						UnFreezeFrozenTracks();

						// Close GUI of selected Tracks
						Seq_Track *t=mastersong->FirstTrack();

						while(t)
						{
							if( t->IsSelected()==true && 
								(t->frozen==false || t->trackfreezeendposition!=endticks)
								)
							{

								if(t->IsTrackAudioFreezeTrack()==true)
								{
									if(t->record==true)
										t->SetRecordMode(false,0);

									maingui->RemoveFreezeTrackFromGUI(t);

									t->trackfreezeendposition=endticks;
								}

							}

							/*
							if(t->IsSelected()==true && 
							t->frozen
							t->trackfreezeendposition!=endticks

							t->IsTrackAudioMasterTrack()==true
							)
							{

							}
							*/

							t=t->NextTrack();
						}
					}

					if(StartThread()==true)
					{
						ShowGadgetStatus();
						DrawDBBlit(showsound);
					}
					else
						mastersong->masteringpRepared=false;
				}

			}
		}
		else
			maingui->MessageBoxOk(0,Cxs[CXS_NOAUDIOORINSTRUMENTSFOUND]);
	}
}

void Edit_AudioMaster::StopMastering()
{
	if(mastersong->mastering==true)
	{
		StopThread();

		mastersong->mastering=mastersong->masteringpRepared=false;
		masterend=false;

		ShowGadgetStatus();
		DrawDBBlit(showsound);
	}

	/*
	Seq_Track *t=WindowSong()->FirstTrack();
	while(t)
	{
		if(t->frozen==true && t->frozenhdfile==0)
		{
			t->UnFreezeTrack();
		}

		t=t->NextTrack();
	}
	*/

}

Mastering::Mastering()
{
	progresspercent=0;
	done=false;
	track=0;
	sampleoffset=0;
	firstsampleposition=0;
}

void Mastering::SaveBuffer(MasteringCall *call,AudioHardwareBuffer *buff,bool masterchannel)
{
	if(call->rawbuffer)
	{
		if(masterchannel==true && checkforusedsample==true && call->savefile.datalen==0)
		{
			int fsample=buff->GetFirstSampleOffset();

			if(fsample==buff->samplesinbuffer)
				return;

			buff->masteroffset=fsample;
		}
		else
		{
			buff->masteroffset=0;
		}

		if(masterchannel==true && addpausesamples==true && call->savefile.datalen==0) // Add Silence ?
		{
			buff->addpausesamples_ms=addpausesamples_ms;
		}
		else 
			buff->addpausesamples_ms=0;

		call->savefile.SaveBuffer(buff,call->rawsize,call->rawbuffer,call->format,&call->iscleared,masterchannel);
	}
}

void Mastering::Do()
{
	AudioDevice *device=song->audiosystem.device;

	song->StopSong(SETSONGPOSITION_NOGUI /*|SETSONGPOSITION_NOQUANTIZE*/,startticks);

	//	mainaudio->StopDevices();

	song->masteringposition=startticks;
	song->masteringendposition=endticks; // Set EndPosition for Notes etc....

	song->mastering=true; // Checked by Plugins, set always AFTER song->masteringposition+song->masteringendposition are set !!!

	song->SetMuteFlags(); // Disable Editor Solo etc..

	song->InitSongPlaybackPattern(startticks,MEDIATYPE_ALL,INITPLAY_MIDITRIGGER);

	song->PRepairPreStartAndCycleEvents(startticks); // Crtl Events before SongPosition (Ctrl,Notes etc..)

	song->SendPRepairControl(false,Seq_Song::SENDPRE_AUDIO|Seq_Song::SENDPRE_MIDI);
	song->SendPRepairNotes(false,Seq_Song::SENDPRE_AUDIO|Seq_Song::SENDPRE_MIDI);	
	song->SendPreStartPrograms();

	device->SetStart(song,startticks,AudioDevice::SETSTART_INIT);
	
	song->CreateAudioStream(device,CREATESTREAMMASTER|CREATESTREAMINIT);

	*masterticks=startticks;
	song->mastertrack=track;

	TRACE ("Master Start ....\n");
	TRACE ("Device Start %d\n",song->stream_samplestartposition);
	TRACE ("Device End %d\n",song->stream_sampleendposition);

	// Save Master Mix Loop
	while(song->masteringstopflag==false)
	{	
		song->RefillAudioDeviceMaster(device); // + Save Calls

		song->masteringposition=*masterticks=song->timetrack.ConvertSamplesToOSTART(song->stream_samplestartposition);

		if(song->stream_sampleendposition>=endsamples)
			break;

		if(progresspercent && endsamples)
		{
			double h=song->stream_sampleendposition;
			double h2=endsamples;

			h/=h2;
			h*=100;

			if(h>100)
				h=100;

			*progresspercent=h;

			TRACE ("Mastering Percent %f\n",*progresspercent);
		}

	}// while Master Save Loop 

	if(song->masteringstopflag==true)
	{
		Seq_Track *t=song->FirstTrack();
		while(t)
		{
			if(t->underfreeze==true)
			{
				t->UnFreezeTrack();
				//t->underfreeze=false;
			}

			t=t->NextTrack();
		}

		*masterticks=endticks; // Set End Time
	}
	else
	{
		Seq_Track *t=song->FirstTrack();
		while(t)
		{
			t->underfreeze=false; 
			t=t->NextTrack();
		}
	}


	song->mastertrack=0;

	song->SendOpenEvents();
	
	song->ResetSongTracksAudioBuffer();

	// Stop Audio
	song->DeleteAllRunningAudioFiles();
	song->audiosystem.StopSystem();
	//song->audiosystem.ChangeAllBackFromVirtualDevice(olddevice); // Set Old Device

	//device->virtualdevice=false;
	song->PRepairPlayback(song->songposition,MEDIATYPE_ALL); // Reset MIDI+Audio Playback
	song->SetAutomationTracksPosition(song->songposition); // 0= all tracks, Send Ctrl Events etc.

	mainthreadcontrol->LockActiveSong();

	song->mastering=song->masteringpRepared=false; // Reset

	song->SetMuteFlags(); // Enable Editor Solo etc..

	//	mainaudio->StartDevices();
	mainthreadcontrol->UnlockActiveSong();

	done=true;
}

void MasteringCall::InitRawBuffer(int channels,int samples,int format)
{
	rawsize=channels*samples;

	switch(format)
	{
	case MASTERFORMAT_16BIT:
		rawsize*=sizeof(short);
		break;

	case MASTERFORMAT_24BIT:
		rawsize*=3;
		break;

	case MASTERFORMAT_32BITFLOAT:
		rawsize*=sizeof(float);
		break;

		//case MASTERFORMAT_64BITFLOAT:
	default:
		rawsize*=sizeof(double);
		break;
	}

	rawbuffer=new char[rawsize];
}

void MasteringCall::Close(bool freememory)
{
	if(rawbuffer)delete rawbuffer;
	rawbuffer=0;

	char *dfile=0;

	if(canceled==false)
	savefile.WriteHeader(); // File is in save mode
	else
		dfile=mainvar->GenerateString(savefile.writefile.fname);
	
	savefile.writefile.Close(true);

	if(canceled==true && dfile)
	{
		mainvar->DeleteAFile(dfile);
		delete dfile;
	}

	if(freememory==true)
		savefile.FreeMemory();
}

// Master Thread 
PTHREAD_START_ROUTINE Edit_AudioMaster::MasterThread(LPVOID pParam)
{
	Mastering mastering;
	LONGLONG time1,time2=-1;

	Edit_AudioMaster *mct=(Edit_AudioMaster *)pParam;
	int errors=0;
	char *errormsg=0;

	mct->masterend=false;

	if(mct->endticks>mct->startticks)
	{
		mainthreadcontrol->LockActiveSong(); // Wait Audio DB Refill
		mct->song->masteringlock=true;
		mainthreadcontrol->UnlockActiveSong();

		TRACE ("Start Mastering...\n");

		mastering.song=mct->song;
		mastering.startticks=mct->startticks;
		mastering.endticks=mct->endticks;
		mastering.endsamples=mct->song->timetrack.ConvertTicksToTempoSamples(mastering.endticks);

		mct->masteringpercent=0;
		mastering.progresspercent=&mct->masteringpercent;

		mastering.sampleformat=mct->mastersampleformat;
		mastering.masterticks=&mct->masterticks;
		mastering.flag_normalize=mct->flag_normalize;
		mastering.checkforusedsample=mct->mastersong->default_mastersavefirst;

		mastering.addpausesamples=mct->mastersong->default_masterpausesamples;
		mastering.addpausesamples_ms=mct->mastersong->default_masterpausems;

		mastering.normalizemax=0;
		mastering.oldsongposition=mct->mastersong->GetSongPosition(); // Buffer Old SPP if Extern Synth recording needed
		mastering.masterfilename=mct->mastersong->masterfile;

		mct->mastersong->ResetPlugins();

		if(mct->mastersong->freeze==false)
			switch(mct->mastersong->masteringmode)
		{
			case Seq_Song::SONGMM_MASTERMIX:
				{
				}
				break;

			case Seq_Song::SONGMM_MASTERSELTRACKS:
				{
					mastering.masterfilename=mct->mastersong->masterdirectoryselectedtracks;
				}
				break;
		}

		time1=maintimer->GetSystemTime();

		if(mct->mastersong->freeze==true)
		{
			// Freeze Tracks

			int c=0;
			Seq_Track *t=mct->mastersong->FirstTrack();

			while(t)
			{
				if( t->IsSelected()==true && 
					(t->frozen==false || t->trackfreezeendposition!=mct->endticks)
					)
				{
					//	bool trackmastering=false;

					if(t->IsTrackAudioFreezeTrack()==true)
					{
						// Master File
						if(MasteringCall *mc=t->freezing=new MasteringCall)
						{
							t->underfreeze=true;

							// Init
							mc->track=t;
							mc->master=&mastering;
							//	mc->device=true;

#ifdef ARES64
							mct->mastersampleformat=MASTERFORMAT_64BITFLOAT;
#else
							mct->mastersampleformat=MASTERFORMAT_32BITFLOAT;
#endif

							mc->format=mct->mastersampleformat;


							switch(mct->mastersampleformat)
							{
							case MASTERFORMAT_32BITFLOAT:
								mc->savefile.samplebits=32;
								break;

							case MASTERFORMAT_64BITFLOAT:
								mc->savefile.samplebits=64;
								break;
							}

							mc->savefile.samplerate=mainaudio->GetGlobalSampleRate();
							mc->savefile.channels=channelschannelsnumber[t->io.channel_type];

							mc->InitRawBuffer(mc->savefile.channels,mainaudio->GetActiveDevice()?mainaudio->GetActiveDevice()->GetSetSize():DEFAULTAUDIOBLOCKSIZE,mct->mastersampleformat);

							char *dir=mainvar->GenerateString(mct->mastersong->directoryname,"\\",DIRECTORY_FREEZETRACKS,"\\");
							// Add Track Name
							char help[NUMBERSTRINGLEN],
								*h=mainvar->GenerateString(dir,mainvar->ConvertIntToChar(mct->mastersong->freezeindexcounter,help),"_",mainvar->ConvertIntToChar(mct->mastersong->GetOfTrack(t)+1,help),t->GetName(),".wav");

							if(dir)
								delete dir;

							if(h)
							{
								bool ok=mc->savefile.InitAudioFileSave(h);

								if(ok==false)
								{
									errormsg=Cxs[CXS_MASTERSAVEERROR_DIRECTORY];
									errors++;
									delete h;
									goto error;
								}

								// Delete old
								if(t->frozenfile)
								{
									mainvar->DeletePeakFile(t->frozenfile);
									mainvar->DeleteAFile(t->frozenfile);
									delete t->frozenfile;

									t->frozen=false;
								}

								t->frozenfile=h;
							}	
						}

						c++;
					}
				}

				t=t->NextTrack();
			}

			if(c)
				mct->mastersong->freezeindexcounter++;

			// End Freeze
		}
		else
			switch(mct->mastersong->masteringmode)
		{
			case Seq_Song::SONGMM_MASTERMIX:
				{
					camxFile test;
					if(test.OpenRead(mastering.masterfilename)==true)
					{
						if(char *h=mainvar->GenerateString(Cxs[CXS_QOVERWRITEFILE],"\n",mastering.masterfilename))
						{
							if(maingui->MessageBoxYesNo(mct,h)==false)
							{
								delete h;
								test.Close(true);
								goto goodbye;
							}

							delete h;
						}
					}
					test.Close(true);

					// Master File
					if(MasteringCall *mc=mct->mastersong->audiosystem.masterchannel.mastering=new MasteringCall)
					{
						// Init
						mc->channel=&mct->mastersong->audiosystem.masterchannel;
						mc->master=&mastering;
						//	mc->device=true;
						mc->format=mct->mastersampleformat;

						switch(mct->mastersampleformat)
						{
						case MASTERFORMAT_16BIT:
							mc->savefile.samplebits=16;
							break;

						case MASTERFORMAT_24BIT:
							mc->savefile.samplebits=24;
							break;

						case MASTERFORMAT_32BITFLOAT:
							mc->savefile.samplebits=32;
							break;

						case MASTERFORMAT_64BITFLOAT:
							mc->savefile.samplebits=64;
							break;

						default:
							mc->savefile.samplebits=32;
							break;
						}

						mc->savefile.samplerate=mainaudio->GetGlobalSampleRate();
						mc->savefile.channels=channelschannelsnumber[mct->mastersong->default_masterchannels];
						mc->InitRawBuffer(mc->savefile.channels,mainaudio->GetActiveDevice()?mainaudio->GetActiveDevice()->GetSetSize():DEFAULTAUDIOBLOCKSIZE,mct->mastersampleformat);
						bool ok=mc->savefile.InitAudioFileSave(mastering.masterfilename);

						if(ok==false)
						{
							errormsg=Cxs[CXS_MASTERSAVEERROR];
							errors=1;
						}
					}
				}
				break;

			case Seq_Song::SONGMM_MASTERSELTRACKS:
				{
					if(!mastering.masterfilename)
						goto goodbye;

					int c=0;
					bool askoverwrite=true;
					Seq_Track *t=mct->mastersong->FirstTrack();

					while(t)
					{
						if(t->IsSelected()==true)
						{
							bool trackmastering=false;

							if(t->IsTrackAudioMasterTrack()==true)
							{
								// Master File
								if(MasteringCall *mc=t->mastering=new MasteringCall)
								{
									// Init
									mc->track=t;
									mc->master=&mastering;
									//	mc->device=true;
									mc->format=mct->mastersampleformat;

									switch(mct->mastersampleformat)
									{
									case MASTERFORMAT_16BIT:
										mc->savefile.samplebits=16;
										break;

									case MASTERFORMAT_24BIT:
										mc->savefile.samplebits=24;
										break;

									case MASTERFORMAT_32BITFLOAT:
										mc->savefile.samplebits=32;
										break;

									case MASTERFORMAT_64BITFLOAT:
										mc->savefile.samplebits=64;
										break;

									default:
										mc->savefile.samplebits=32;
										break;
									}

									mc->savefile.samplerate=mainaudio->GetGlobalSampleRate();
									mc->savefile.channels=channelschannelsnumber[t->io.channel_type];
									mc->InitRawBuffer(mc->savefile.channels,mainaudio->GetActiveDevice()?mainaudio->GetActiveDevice()->GetSetSize():DEFAULTAUDIOBLOCKSIZE,mct->mastersampleformat);

									// Add Track Name
									char help[NUMBERSTRINGLEN],
										*h=mainvar->GenerateString(mastering.masterfilename,"\\",mainvar->ConvertIntToChar(mct->mastersong->GetOfTrack(t)+1,help),"T_",t->GetName(),".wav");

									if(h)
									{
										if(askoverwrite==true)
										{
											camxFile test;
											if(test.OpenRead(h)==true)
											{
												if(char *a=mainvar->GenerateString(Cxs[CXS_QOVERWRITEFILE],"\n",h))
												{
													if(maingui->MessageBoxYesNo(mct,a)==false)
													{
														delete a;
														delete h;
														test.Close(true);
														goto goodbye;
													}

													askoverwrite=false;
													delete a;
												}
											}
											test.Close(true);
										}

										bool ok=mc->savefile.InitAudioFileSave(h);

										delete h;

										if(ok==false)
										{
											errormsg=Cxs[CXS_MASTERSAVEERROR_DIRECTORY];
											errors++;
											goto error;
										}
									}
								}

								c++;
							}
						}

						t=t->NextTrack();
					}
				}
				break;
		}

error:
		if(errors==0)
		{
			//	mastering.file=&master; // File Closed in Mastering.Do
			mastering.Do();
			TRACE ("End Mastering...\n");
		}
		else
		{
			mastering.done=true;
			maingui->MessageBoxOk(0,errormsg);

			goto goodbye;
		}

		time2=maintimer->GetSystemTime();

		mct->mastersong->ResetPlugins(); // Reset - Avoid Sound Output !

		mainthreadcontrol->LockActiveSong();
		mct->song->masteringlock=false;
		mainthreadcontrol->UnlockActiveSong();
	}

	bool canceled=mct->mastersong->masteringstopflag;

	if(errors==0)
	{
		if(mastering.done==true && canceled==false) // Open Save and Write Header
		{
			if(mastering.flag_normalize==true && mastering.normalizemax>0 && mastering.normalizemax<1) // Do Normalize
			{
				AudioHDFile norm;
				norm.Open(mastering.masterfilename);

				if(norm.errorflag==0)
				{
					norm.Normalize(mastering.normalizemax);
					//norm.Close();
				}
				else
					maingui->MessageBoxOk(0,Cxs[CXS_MASTERNORMERROR]);
			}
		}
	}

goodbye:
	// Close Files/Mastering

	// Master Channel
	mct->mastersong->audiosystem.masterchannel.CloseMastering(true,canceled);

	// Bus
	AudioChannel *ac=mct->mastersong->audiosystem.FirstBusChannel();

	while(ac)
	{
		ac->CloseMastering(true,canceled);
		ac=ac->NextChannel();
	}

	// Tracks
	Seq_Track *t=mct->mastersong->FirstTrack();

	while(t)
	{
		if(t->freezing)
		{
			t->CloseMastering(false,canceled);

			if(t->frozenfile && canceled==false)
			{
				// Create Peak
				t->SetFrozenHDFile(t->frozenfile,0);
			}
		}
		else
			t->CloseMastering(true,canceled);

		t=t->NextTrack();
	}

	mct->mastersong->mastering=mct->mastersong->masteringpRepared=false; // Reset

	if(time2!=-1)
	mct->duration_ms=maintimer->ConvertSysTimeToMs(time2-time1);
	else
	mct->duration_ms=0;

	mct->masterend=true;
	mct->masterthread.ThreadGone();

	mct->masterthread.ThreadHandle=0;

	return 0;
}

bool Edit_AudioMaster::StartThread()
{
	if(!mastersong)
		return false;

#ifdef WIN32
	masterthread.ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MasterThread,(LPVOID)this, 0,0);
	return masterthread.ThreadHandle?true:false;
#endif
}

void Edit_AudioMaster::StopThread()
{
	if(masterthread.ThreadHandle)
	{
		mastersong->masteringstopflag=true; // stop refill loop
		masterthread.SendQuit();
		mastersong->masteringstopflag=false;
		masterthread.ThreadHandle=0;
	}
}