#include "defines.h"

#include <stdio.h>
#include <windows.h>

#ifdef WIN32
#include "asio/asiodrivers.h"
#include "directs.h"
#endif

#include "audiohardware.h"
#include "audiodevice.h"
#include "vstplugins.h"

#include "object_project.h"
#include "object_song.h"
#include "object_track.h"

#include "songmain.h"
#include "audiohdfile.h"
#include "audioregion.h"
#include "audiomanager.h"
#include "audiofilework.h"
#include "languagefiles.h"
#include "editbuffer.h"

// Devices
#include "asio_device.h"
#include "win32_device.h"
#include "gui.h"
#include "sampleeditor.h"
#include "semapores.h"

#include <cmath>

#include "settings.h"
#include "audiofile.h"
#include "audiosend.h"
#include "audiopattern.h"
#include "chunks.h"
#include "MIDItimer.h"
#include "editdata.h"
#include "editsettings.h"
#include "editfunctions.h"

int rafbuffersize[]= // pre Buffer ms
{
	500,
	600,
	750,
	1000,
	1250,
	1500,
	1750,
	2000,
	2500,
	3000,
	5000
};


LONGLONG mainAudio::ConvertMsToSamples(double ms)
{
	double h_rate=(double)mainaudio->GetGlobalSampleRate();

	h_rate/=1000;
	h_rate*=ms;

	return (LONGLONG)floor(h_rate+0.5);
}

void mainAudio::InitSampleRate()
{
	ppqrateinternmul=PPQRATEINTERN;
	ppqrateinternmul/=SAMPLESPERBEAT;

	double h=samplerate;
	double h2=SAMPLESPERBEAT*2;
	internexternfactor=h2/h;

	h=samplerate;
	h/=1000;
	samplesperms=h;

	h=1000;
	h/=samplerate;
	samplespermsmul=h;

	mainvar->InitQuantList(SAMPLESPERBEAT);

	if(GetActiveDevice())
	GetActiveDevice()->ConvertSampleRateToIndex();
}

bool mainAudio::ResetAudio(int flag,int value)
{
	//maingui->MessageBoxOk(0,"Reset Audio Init");

	if(mainvar->GetActiveSong())
		mainvar->GetActiveSong()->StopSong(NO_SONGPRepair,mainvar->GetActiveSong()->GetSongPosition());

	mainaudioreal->StopAllRealtimeEvents();

	if(GetActiveDevice())
		GetActiveDevice()->StopAudioHardware();

	mainthreadcontrol->LockActiveSong();

	if(flag&RESETAUDIO_NEWRAFBUFFERSIZE)
	{
		rafbuffersize_index=value;
	}

	if(flag&RESETAUDIO_NEWBUFFERSIZE)
	{
		if(GetActiveDevice())
		{
			if(value==0)
			{
				GetActiveDevice()->InitMinMaxPrefBufferSizes();
				GetActiveDevice()->SetBufferSize(GetActiveDevice()->GetPrefBufferSize());
			}
			else
				GetActiveDevice()->SetBufferSize(value);

			TRACE ("BufferSettings %d\n",GetActiveDevice()->usebuffersettings);
			TRACE ("RESETAUDIO_NEWBUFFERSIZE %d\n",GetActiveDevice()->GetSetSize());
		}
	}

	if(flag&RESETAUDIO_NEWSAMPLERATE)
	{
		samplerate=value;
		InitSampleRate();
		OpenMetroClicks();
	}

	ResetAudioDevices();
	StartDevices();

	if(mainvar->GetActiveSong())
		mainvar->GetActiveSong()->RefreshAudioBuffer(); // New setSize ?

	if(GetActiveDevice())
	{
		GetActiveDevice()->resetrequest=false;
	}

	mainthreadcontrol->UnlockActiveSong();

	return true;
}

void mainAudio::SetSamplesSize(AudioDevice *device,int size)
{
	if(device && device->GetSetSize()!=size)
		ResetAudio(RESETAUDIO_NEWBUFFERSIZE,size);
}

void mainAudio::SetGlobalRafSize(int newraf)
{
	if(rafbuffersize_index!=newraf)
		ResetAudio(RESETAUDIO_NEWRAFBUFFERSIZE,newraf);
}

int mainAudio::SetGlobalSampleRate(int newsr)
{
	if(newsr==0)
		return samplerate;

	if(newsr!=samplerate)
	{
		if(mainvar->GetActiveProject())
		{
			if(mainvar->GetActiveProject()->CheckNewSampleRateIsOk(newsr)==false)
			{
				maingui->MessageBoxOk(0,Cxs[CXS_PROJECTSAMPLEMSG]);
				return samplerate;
			}

			mainvar->GetActiveProject()->SetSampleRate(newsr);

			if(mainvar->GetActiveProject()->projectsamplerate>0 && mainvar->GetActiveProject()->projectsamplerate!=newsr)
			{
				maingui->MessageBoxOk(0,Cxs[CXS_PROJECTSAMPLEMSG]);
				return samplerate;
			}
		}

		ResetAudio(RESETAUDIO_NEWSAMPLERATE,newsr);
	}

	return samplerate;
}

void mainAudio::CollectAudioHardware()
{
	collectmode=true;

#ifndef NOAUDIO

#ifdef WIN32

	// ASIO
	AsioDrivers adrivers;

	LONG numberdriver=adrivers.asioGetNumDev();

	if(numberdriver>0)
	{
		maingui->SetInfoWindowText(">ASIO...");

		if(AudioHardware *asiohardware=new AudioHardware("ASIO",AUDIOCAMX_ASIO))
		{
			for(int i=0;i<numberdriver;i++)
			{
				if(AudioDevice_ASIO *ad=new AudioDevice_ASIO)
				{
					char name[256];

					adrivers.asioGetDriverName(i,name,255); // Name

					if(char *h=mainvar->GenerateString("...ASIO ",name))
					{
						maingui->SetInfoWindowText(h);
						delete h;
					}

					TRACE ("ASIO Device Nr %d %s Size Device %d \n",i,name,sizeof(AudioDevice_ASIO));

					if(ad->CheckAudioDevice(name)==true) // Check ok ?
					{
						asiohardware->AddAudioDevice(ad,name,"ASIO");
						ad->id=i;
					}
					else
						delete ad; // Error
				}
			}

			if(asiohardware->FirstDevice())
			{
				if(old_hardwarename && strcmp(old_hardwarename,"ASIO")==0)
				{
					selectedaudiohardware=asiohardware; // ASIO -> ASIO
				}
				else
				{
					if(mainsettings->askforasio==true)
					{
						if(!selectedaudiohardware)
						{
							if(maingui->MessageBoxYesNo(0,Cxs[CXS_ASIOWASFOUND])==true)
								selectedaudiohardware=asiohardware; // Set ASIO Default
						}
					}
				}

				audiohardware.AddEndO(asiohardware);

				mainaudio->foundasio=true;
			}
			else
			{
				delete asiohardware; 
			}
		}

	}// end ASIO

	maingui->SetInfoWindowText(">Win PCM...");

	// Win32 PCM
	if(AudioHardware *win32audio=new AudioHardware("Win PCM",AUDIOCAMX_WIN32))
	{
		HWAVEOUT hWaveOut_44=0,hWaveOut_48=0,hWaveOut_88=0,hWaveOut_96=0;
		WAVEFORMATEX wfx;

		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 2;
		wfx.nSamplesPerSec = 44100;
		wfx.nAvgBytesPerSec = (2 * 44100 * 16) / 8;
		wfx.nBlockAlign =	(16 * 2) / 8;
		wfx.wBitsPerSample = 16;
		wfx.cbSize = 0;

		MMRESULT m_mmr44=waveOutOpen (&hWaveOut_44,WAVE_MAPPER,&wfx,0,0,WAVE_FORMAT_QUERY);

		if(m_mmr44==MMSYSERR_NOERROR)
		{
			waveOutClose(hWaveOut_44);
		}

		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 2;
		wfx.nSamplesPerSec = 48000;
		wfx.nAvgBytesPerSec = (2 * 48000 * 16) / 8;
		wfx.nBlockAlign =	(16 * 2) / 8;
		wfx.wBitsPerSample = 16;
		wfx.cbSize = 0;

		MMRESULT m_mmr48=waveOutOpen (&hWaveOut_48,WAVE_MAPPER,&wfx,0,0,WAVE_FORMAT_QUERY);

		if(m_mmr48==MMSYSERR_NOERROR)
		{
			waveOutClose(hWaveOut_48);
		}

		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 2;
		wfx.nSamplesPerSec = 88200;
		wfx.nAvgBytesPerSec = (2 * 88200 * 16) / 8;
		wfx.nBlockAlign =	(16 * 2) / 8;
		wfx.wBitsPerSample = 16;
		wfx.cbSize = 0;

		MMRESULT m_mmr88=waveOutOpen (&hWaveOut_88,WAVE_MAPPER,&wfx,0,0,WAVE_FORMAT_QUERY);

		if(m_mmr88==MMSYSERR_NOERROR)
		{
			waveOutClose(hWaveOut_88);
		}

		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 2;
		wfx.nSamplesPerSec = 96000;
		wfx.nAvgBytesPerSec = (2 * 96000 * 16) / 8;
		wfx.nBlockAlign =	(16 * 2) / 8;
		wfx.wBitsPerSample = 16;
		wfx.cbSize = 0;

		MMRESULT m_mmr96=waveOutOpen (&hWaveOut_96,WAVE_MAPPER,&wfx,0,0,WAVE_FORMAT_QUERY);

		if(m_mmr96==MMSYSERR_NOERROR)
		{
			waveOutClose(hWaveOut_96);
		}

		if(m_mmr44==MMSYSERR_NOERROR || m_mmr48==MMSYSERR_NOERROR || m_mmr88==MMSYSERR_NOERROR || m_mmr96==MMSYSERR_NOERROR)
		{			
			if(old_hardwarename && strcmp(old_hardwarename,"Win PCM")==0)
			{
				if(!selectedaudiohardware)
					selectedaudiohardware=win32audio; // Set WIN32 Default
			}
			else
			{
				if(!selectedaudiohardware)
					selectedaudiohardware=win32audio; // Set WIN32 Default
			}

			audiohardware.AddEndO(win32audio);

			if(AudioDevice_WIN32 *ad=new AudioDevice_WIN32)
			{
				ad->ActivateSampleRate(ADSR_44,m_mmr44==MMSYSERR_NOERROR?true:false);
				ad->ActivateSampleRate(ADSR_48,m_mmr48==MMSYSERR_NOERROR?true:false);
				ad->ActivateSampleRate(ADSR_88,m_mmr88==MMSYSERR_NOERROR?true:false);
				ad->ActivateSampleRate(ADSR_96,m_mmr96==MMSYSERR_NOERROR?true:false);

				ad->id=0;

				if(ad->CheckAudioDevice(0)==true)
					win32audio->AddAudioDevice(ad,"Mapper (Stereo)","WIN PCM");
				else
					delete ad;
			}

		}
		else
			delete win32audio;

	}//end win32 pcm

	// Try to open Direct S
#ifdef _DEBUG
	// directsound.CollectDevices();
#endif

#endif

#endif

	maingui->SetInfoWindowText("Load Audio Devices Prefs");

	// Audio Devices
	AudioHardware *ahw=FirstAudioHardware();
	while(ahw)
	{
		AudioDevice *ad=ahw->FirstDevice();
		while(ad)
		{
			if(ad->devname)
			{
				if(char *filename=mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_AUDIODEVICES)){

					if(char *clearstring=mainvar->CreateSimpleASCIIString(ad->devname))
					{
						if(char *h=mainvar->GenerateString(filename,ahw->name,"_",clearstring))
						{
							camxFile file;

							if(file.OpenRead(h)==true && file.CheckVersion()==true)
							{
								file.LoadChunk();

								if(file.GetChunkHeader()==CHUNK_AUDIODEVICE)
								{
									file.ChunkFound();
									ad->Load(&file);
									file.CloseReadChunk();
								}

								file.LoadChunk();

								if(file.GetChunkHeader()==CHUNK_AUDIODEVICEINPUTPORTS)
								{
									ad->inportsfound=true;
									file.ChunkFound();

									for(int c=0;c<AUDIOCHANNELSDEFINED;c++)
									{
										for(int p=0;p<CHANNELSPERPORT;p++)
										{
											file.ReadChunk(&ad->inputaudioports[c][p].visible);

											for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
											{
												int index=0;
												file.ReadChunk(&index);

												ad->inputaudioports[c][p].hwchannel[i]=ad->GetInputChannelIndex(index);

												if(ad->inputaudioports[c][p].hwchannel[i]<0)ad->inputaudioports[c][p].hwchannel[i]=0;// avoid -1
											}
										}
									}

									file.CloseReadChunk();
								}

								file.LoadChunk();

								if(file.GetChunkHeader()==CHUNK_AUDIODEVICEOUTPUTPORTS)
								{
									ad->outportsfound=true;
									file.ChunkFound();

									for(int c=0;c<AUDIOCHANNELSDEFINED;c++)
									{
										for(int p=0;p<CHANNELSPERPORT;p++)
										{
											file.ReadChunk(&ad->outputaudioports[c][p].visible);

											for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
											{
												int index=0;
												file.ReadChunk(&index);

												ad->outputaudioports[c][p].hwchannel[i]=ad->GetOutputChannelIndex(index);

												if(ad->outputaudioports[c][p].hwchannel[i]<0)ad->outputaudioports[c][p].hwchannel[i]=0; // avoid -1
											}
										}
									}

									file.CloseReadChunk();
								}
							}

							file.Close(true);
							delete h;
						}
						delete clearstring;
					}
				}
			}

			ad=ad->NextDevice();
		}

		ahw=ahw->NextHardware();
	}

	collectmode=false;
}

void mainAudio::CloseAllAudioDevices()
{
	AudioHardware *h=FirstAudioHardware();

	while(h)
	{
		TRACE ("Close Hardware %s\n",h->name);

		AudioDevice *dev=h->FirstDevice();

		while(dev)
		{
			TRACE ("Close Device %s\n",dev->devname);
			dev->StopDevice();
			dev=h->RemoveAudioDevice(dev);
		}

		h=(AudioHardware *)audiohardware.RemoveO(h);
	}

	selectedaudiohardware=0;
}

mainAudio::mainAudio()
{
	double h;

	ignorecorrectaudiofiles=false;

	defaultchannel_type=0; // Default Mono
	defaultchannelindex_out=0; // First Channel

	defaultchannelins=0; // Input Mono Default
	defaultchannelindex_in=0; 

	audiooutactive=true;

#ifdef WIN32
	foundasio=false;
#endif

	old_hardwarename=old_hardwaredevice=0;

	//lockaudiofiles=false;
	selectedaudiohardware=0;

	samplerate=48000; // default Samplerate
	InitSampleRate();

	// +
	int dbc=0;

	h=AUDIO_MAXDB; 
	for(int a=LOGVOLUME_SIZE;a>0;a--)
	{
		logvolume[a]=ConvertDbToFactor(h);
		logvolume_f[a]=(float)logvolume[a];

		TRACE ("logvolume[a] db=%f a=%d %f dbc=%d\n",h,a, logvolume[a],dbc);

		if(h<-48)
			h-=2.5;
		else
			if(h<-24)
				h-=2;
			else
				if(h<-12)
					h-=0.4;
				else
					if(h<-9)
						h-=0.2;
					else
						if(h<-6)
							h-=0.1;
						else
							h-=0.05;

		dbc++;
	}


	logvolume[0]=0; // Silence
	logvolume_f[0]=0;

	double th=LOGVOLUME_SIZE;

	th*=0.5;
	int lt=(int)th;

	logvolume[lt]=1.0; // 0db
	logvolume_f[lt]=1.0;

	silencefactor=ConvertDbToFactor(-82);

	panorama_db[0]=1;
	panorama_db[1]=(ARES)ConvertDbToFactor(-3);
	panorama_db[2]=(ARES)ConvertDbToFactor(-4.5);
	panorama_db[3]=(ARES)ConvertDbToFactor(-6);

	activehdfile=0;

	// Audio Metro
	metro_a=0;
	metro_b=0;
	oldmetrosamplerate=0;

	rafbuffersize_index=4; // Default

	newvstpluginsadded=0;
	collectmode=false;
}

void mainAudio::OpenAudio()
{
	//crashed Plugin ?
	mainsettings->LoadCrashedPlugins();

	char *pfile=mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_PLUGINTEST);
	camxFile plugintest;

	if(plugintest.OpenRead(pfile)==true)
	{
		short l=0;

		plugintest.Read(&l,sizeof(short));

		if(l)
		{
			char *fl=new char[l+1];

			if(fl)
			{
				plugintest.Read(fl,l);
				fl[l]=0;

				AddCrashedPlugin(fl);
				delete fl;
			}
		}
	
		plugintest.Close(true);

		mainvar->DeleteAFile(pfile);
	}

	maingui->SetInfoWindowText("Init Plugins...#3");
	mainsettings->LoadPluginSettings();

	maingui->SetInfoWindowText("Collect Audio Hardware");
	CollectAudioHardware();

	maingui->SetInfoWindowText("Collect Intern Effects");
	CollectInternEffects();

	// Add Default Directory
#ifdef WIN32
	Directory *dir=mainsettings->AddVSTDirectory(VST2,"VST_plugins\\");
	if(dir)dir->dontdelete=true;
#endif

#ifdef WIN64
	dir=mainsettings->AddVSTDirectory(VST2,"VST_plugins64\\");
	if(dir)dir->dontdelete=true;
#endif

	

	CollectVSTPlugins(VST2,0,true);

	if(selectedaudiohardware) // Default
	{
		maingui->SetInfoWindowText("Init Audio Device");

		bool setbydefault=false;

		if(old_hardwaredevice)
		{
			AudioDevice *c=selectedaudiohardware->FirstDevice();
			while(c)
			{
				if(strcmp(c->devname,old_hardwaredevice)==0)
				{
					setbydefault=true;
					selectedaudiohardware->SetActiveDevice(c);
					break;
				}

				c=c->NextDevice();
			}
		}

		if(setbydefault==false)
			selectedaudiohardware->SetActiveDevice(selectedaudiohardware->FirstDevice());
	}

	InitDefaultPorts();
}

void mainAudio::CloseAudio()
{	
	maingui->SetInfoWindowText("Close Audio Devices ...");
	CloseAllAudioDevices();

	maingui->SetInfoWindowText("Close Intern Effects ...");
	CloseAllInternEffects();

	maingui->SetInfoWindowText("Close Plugins ...");
	CloseAllVSTPlugins();

	maingui->SetInfoWindowText("Close Audio Files ...");
	CloseAllHDFiles();
	DeleteAllMetroClicks();

	if(old_hardwaredevice)delete old_hardwaredevice;
	if(old_hardwarename)delete old_hardwarename;

	old_hardwaredevice=old_hardwarename=0;
}

int mainAudio::GetCountOfAudioFilesInDirectoy(Directory *d)
{
	int c=0;

	AudioHDFile *h=FirstAudioHDFile();
	while(h)
	{
		if(h->directory==d)
			c++;

		h=h->NextHDFile();
	}

	return c;
}

void mainAudio::CollectAudioFiles(char *dir,Directory *pdirectory)
{
	if(!dir)return;

	if(AudioFileWork_Finder *work=new AudioFileWork_Finder){

		work->directory=pdirectory;
		work->filename=mainvar->GenerateString(dir);

		audioworkthread->AddWork(work);
	}
}

void mainAudio::OpenMetroClicks()
{
	char *metroa=0,*metrob=0;

	if(oldmetrosamplerate!=GetGlobalSampleRate())
	{
		oldmetrosamplerate=GetGlobalSampleRate();

		DeleteAllMetroClicks();

		switch(GetGlobalSampleRate())
		{
		case 44100:
			metroa="\\sounds\\metro\\metro441_a.wav";
			metrob="\\sounds\\metro\\metro441_b.wav";
			break;

		case 48000:
			metroa="\\sounds\\metro\\metro48_a.wav";
			metrob="\\sounds\\metro\\metro48_b.wav";
			break;

		case 88200:
			metroa="\\sounds\\metro\\metro882_a.wav";
			metrob="\\sounds\\metro\\metro882_b.wav";
			break;

		case 96000:
			metroa="\\sounds\\metro\\metro96_a.wav";
			metrob="\\sounds\\metro\\metro96_b.wav";
			break;

		case 176400:
			metroa="\\sounds\\metro\\metro176_a.wav";
			metrob="\\sounds\\metro\\metro176_b.wav";
			break;

		case 192000:
			metroa="\\sounds\\metro\\metro192_a.wav";
			metrob="\\sounds\\metro\\metro192_b.wav";
			break;

		case 352800:
			metroa="\\sounds\\metro\\metro352_a.wav";
			metrob="\\sounds\\metro\\metro352_b.wav";
			break;

		case 384000:
			metroa="\\sounds\\metro\\metro384_a.wav";
			metrob="\\sounds\\metro\\metro384_b.wav";
			break;
		}

		if(metroa && metrob)
		{
			char *h=new char[strlen(mainvar->GetCamXDirectory())+128];

			if(h){

				if(metro_a=new AudioRAMFile)
				{
					metro_a->ramfile=true;
					metro_a->dontcreatepeakfile=true;

					strcpy(h,mainvar->GetCamXDirectory());
					mainvar->AddString(h,metroa);

					if(metro_a->SetName(h))
					{
						metro_a->InitHDFile();

						if(metro_a->errorflag==0)
						{
							metro_a->LoadSoundToRAM();
						}
						else
						{
							delete metro_a;
							metro_a=0;						
#ifdef _DEBUG
							MessageBox(NULL,"Cant open Metro A","Error",MB_OK);
#endif
						}
					}
					else
					{
						delete metro_a;
						metro_a=0;
					}
				}

				if(metro_b=new AudioRAMFile)
				{
					metro_b->ramfile=true;
					metro_b->dontcreatepeakfile=true;

					strcpy(h,mainvar->GetCamXDirectory());
					mainvar->AddString(h,metrob);

					if(metro_b->SetName(h))
					{	
						metro_b->InitHDFile();

						if(metro_b->errorflag==0)
						{
							metro_b->LoadSoundToRAM();
						}	
						else
						{
							delete metro_b;
							metro_b=0;			
#ifdef _DEBUG

							MessageBox(NULL,"Cant open Metro B","Error",MB_OK);
#endif
						}
					}
					else
					{
						delete metro_b;
						metro_b=0;
					}
				}

				delete h;
			}
		}
	}
}

void mainAudio::DeleteAllMetroClicks()
{
	if(metro_a)
	{
		DeleteHDFile(metro_a);
		metro_a=0;
	}

	if(metro_b)
	{
		DeleteHDFile(metro_b);
		metro_b=0;
	}
}

void mainAudio::SelectPluginProgram(guiWindow *win,InsertAudioEffect *iae)
{
	if(EditData *edit=new EditData)
	{
		edit->insertaudioeffect=iae;
		edit->song=win->WindowSong();
		edit->win=win;
		edit->x=0;
		edit->y=0;
		edit->deletename=true;
		edit->type=EditData::EDITDATA_TYPE_PLUGINPROGRAMS;

		maingui->EditDataValue(edit);
	}
}

AudioObject *mainAudio::GetAudioEffect(AudioChannel *channel,int index)
{
	// VST PlugIns
	VSTPlugin *vst=FirstVSTEffect();

	while(vst){	
		if(index==0)return vst;
		index--;
		vst=vst->NextVSTPlugin();
	}

	return 0;
}

AudioObject *mainAudio::GetAudioInstrument(AudioChannel *channel,int index)
{
	// VST PlugIns
	VSTPlugin *vst=FirstVSTInstrument();

	while(vst){	
		if(index==0)return vst;
		index--;
		vst=vst->NextVSTPlugin();
	}

	return 0;
}

void mainAudio::CreateAudioRegionFile(guiWindow *win,AudioHDFile *hdfile,AudioRegion *region)
{
	if(hdfile && region)
	{
		camxFile write;

		if(char *rname=mainvar->GenerateString(region->GetName(),"_R_",hdfile->GetFileName()))
		{
			if(write.OpenFileRequester(0,win,Cxs[CXS_EXPORTREGIONAS],write.AllFiles(camxFile::FT_WAVES),false,rname)==true)
			{
				write.AddToFileName(".wav");

				if(write.filereqname){

					if(char *newfile=mainvar->GenerateString(write.filereqname)){

						AudioFileWork_CreateNewFile *work=new AudioFileWork_CreateNewFile; // From

						if(work){
							work->Init(hdfile->GetName()); // org file
							work->creatednewfile=newfile;
							work->samplestart=region->regionstart;
							work->sampleend=region->regionend;
							audioworkthread->AddWork(work);
						}
					}
				}
			}

			delete rname;
		}
	}
}

AudioDevice *mainAudio::GetActiveDevice()
{
	return selectedaudiohardware?selectedaudiohardware->GetActiveDevice():0;
}

double mainAudio::ConvertSizeToMs(int rate,int size)
{
	if(rate && size)
	{
		double ms=(double)size;
		ms/=(double)rate;
		ms*=1000;
		return ms;
	}

	return 0;
}

void mainAudio::CheckPeakFiles()
{
	AudioHDFile *hd=FirstAudioHDFile();

	while(hd)
	{
		if(!hd->peakbuffer)
		{
			if(CheckAudioFileUsage(hd)==true)
			{
				hd->CreatePeak();
				// mainaudio->ClosePeakFile(hd->peakbuffer,false,&deleted);
			}
		}

		hd=hd->NextHDFile();
	}
}

bool mainAudio::CheckAudioFileUsage(char  *file)
{
	if(!file)return false;

	// gui
	guiWindow *w=maingui->FirstWindow();
	while(w)
	{
		switch(w->GetEditorID())
		{
		case EDITORTYPE_AUDIOMANAGER:
			{
				Edit_Manager *em=(Edit_Manager *)w;

				if(em->activefile && strcmp(em->activefile->hdfile->GetName(),file)==0)
					return true;
			}
			break;

		case EDITORTYPE_SAMPLE:
			{
				Edit_Sample *es=(Edit_Sample *)w;

				if(strcmp(es->audiohdfile->GetName(),file)==0)
					return true;
			}
			break;
		}

		w=w->NextWindow();
	}

	// Songs
	Seq_Project *p=mainvar->FirstProject();
	while(p)
	{
		Seq_Song *s=p->FirstSong();
		while(s)
		{
			Seq_Track *t=s->FirstTrack();
			while(t)
			{
				AudioPattern *p=(AudioPattern *)t->FirstPattern(MEDIATYPE_AUDIO);
				while(p)
				{
					if(p->audioevent.audioefile && strcmp(p->audioevent.audioefile ->GetName(),file)==0)
						return true;

					p=(AudioPattern *)p->NextPattern(MEDIATYPE_AUDIO);
				}

				t=t->NextTrack();
			}

			s=s->NextSong();
		}

		p=p->NextProject();
	}

	return false;
}

bool mainAudio::CheckAudioFileUsage(AudioHDFile *hd)
{
	if(!hd)return false;
	if(!hd->peakbuffer)return false;

	// gui
	guiWindow *w=maingui->FirstWindow();
	while(w)
	{
		switch(w->GetEditorID())
		{
		case EDITORTYPE_AUDIOMANAGER:
			{
				Edit_Manager *em=(Edit_Manager *)w;

				if(em->activefile && em->activefile->hdfile==hd)
					return true;
			}
			break;

		case EDITORTYPE_SAMPLE:
			{
				Edit_Sample *es=(Edit_Sample *)w;

				if(es->audiohdfile==hd)
					return true;
			}
			break;
		}

		w=w->NextWindow();
	}

	// Songs
	Seq_Project *p=mainvar->FirstProject();
	while(p)
	{
		Seq_Song *s=p->FirstSong();
		while(s)
		{
			if(s->loaded==true)
			{
				Seq_Track *t=s->FirstTrack();
				while(t)
				{
					AudioPattern *p=(AudioPattern *)t->FirstPattern(MEDIATYPE_AUDIO);
					while(p)
					{
						if(p->audioevent.audioefile && p->audioevent.audioefile->peakbuffer==hd->peakbuffer)
							return true;

						p=(AudioPattern *)p->NextPattern(MEDIATYPE_AUDIO);
					}

					t=t->NextTrack();
				}
			}

			s=s->NextSong();
		}

		p=p->NextProject();
	}

	return false;
}

void mainAudio::DeleteUnusedPeaks()
{
#ifdef WIN32
	int timenow=GetTickCount();
#endif	
	int checktime=mainvar->lastpeakcheck;

	checktime+=30000; // 30 sec

	if(checktime<=timenow)
	{
		mainvar->ResetPeakCheck();
		AudioHDFile *hd=FirstAudioHDFile();

		while(hd)
		{
			if(CheckAudioFileUsage(hd)==false)
			{
				bool deleted;

				mainaudio->ClosePeakFile(hd,hd->peakbuffer,false,&deleted);
				if(deleted==true)
					hd->peakbuffer=0;
			}

			hd=hd->NextHDFile();
		}
	}
}

void mainAudio::CopyAudioFile(guiWindow *win,AudioHDFile *audiohdfile,AudioPattern *pattern)
{
	if(audiohdfile && audiohdfile->filenotfound==false && audiohdfile->GetFileName())
	{
		camxFile copy;

		if(char *h=mainvar->GenerateString(Cxs[CXS_COPY],"<",audiohdfile->GetFileName(),">",Cxs[CXS_TOFILE]) )
		{
			if(copy.OpenFileRequester(0,win,h,copy.AllFiles(camxFile::FT_WAVES),false,audiohdfile->GetFileName())==true)
			{
				if(copy.filereqname)
				{
					copy.AddToFileName(".wav");

					if(char *newfile=mainvar->GenerateString(copy.filereqname)){

						AudioFileWork_CreateNewFile *work=new AudioFileWork_CreateNewFile; // From

						if(work){
							work->Init(audiohdfile->GetName()); // org file
							work->creatednewfile=newfile; // new file

							if(pattern)
							{
								if(pattern->GetUseOffSetRegion()==true)
								{
									work->samplestart=pattern->GetOffSetStart();
									work->sampleend=pattern->GetOffSetEnd();
									work->copyfile=false;
								}
								else
									if(pattern->audioevent.audioregion)
									{
										work->samplestart=pattern->audioevent.audioregion->regionstart;
										work->sampleend=pattern->audioevent.audioregion->regionend;
										work->copyfile=false;
									}
									else
										work->copyfile=true;
							}
							else
								work->copyfile=true; // Simple Copy

							audioworkthread->AddWork(work);
						}
						else
							delete newfile;
					}
				}
			}

			delete h;
		}

		copy.Close(true);
	}
}


void mainAudio::RefreshAudioFileGUI(AudioHDFile *file)
{
	guiWindow *f=maingui->FirstWindow();

	while(f)
	{
		switch(f->GetEditorID())
		{
			/*
			case EDITORTYPE_AUDIOMANAGER:
			{
			Edit_Manager *e=(Edit_Manager *)f;

			if(!e->FindAudioHDFile(file))
			e->InitList();

			e->ShowAudioFiles();
			}
			break;
			*/

		}

		f=f->NextWindow();
	}
}

AudioHDFile *mainAudio::AddAudioRecordingFile(Seq_Track *track,char *name)
{	
	if(AudioHDFile *ahd=new AudioHDFile)
	{
		if(ahd->SetName(name))
		{
			bool ok;

			if(track->song->playbacksettings.cycleplayback==true)
				ok=ahd->writefile.OpenReadSave(name); // Cycle Recording<+>Playback
			else
				ok=ahd->writefile.OpenSave(name);

			if(ok==false)
				maingui->MessageBoxOk(0,"AddAudioRecordingFile");

			if(ok==true)
			{
				if(mainaudio->GetActiveDevice())
				{
					int size=mainaudio->GetActiveDevice()->GetSetSize()*MAXCHANNELSPERCHANNEL;

#ifdef DEBUG
					size++;
#endif

					if(ahd->recmix.outputbufferARES=new ARES[size]) // ARES MIX
					{
						ahd->recmix.static_outputbufferARES=ahd->recmix.outputbufferARES;

#ifdef DEBUG
						ahd->recmix.outputbufferARES[size-1]=1.1f;
#endif
						// 64 Bit
						ahd->recmix.inputbuffer32bit=new double[mainaudio->GetActiveDevice()->GetSetSize()*MAXCHANNELSPERCHANNEL];
						ahd->recmix.inputbuffersize=sizeof(double)*mainaudio->GetActiveDevice()->GetSetSize()*MAXCHANNELSPERCHANNEL;

						ahd->recmix.SetBuffer(MAXCHANNELSPERCHANNEL,mainaudio->GetActiveDevice()->GetSetSize());
					}
					else
					{
						ahd->writefile.Close(true);
						delete ahd; // ARES memory problem !
						return 0;
					}
				}

				ahd->samplerate=mainaudio->GetGlobalSampleRate();
				ahd->recordingtrack=track;
				ahd->recordingactive=true; // Set to Record Mode
				ahd->deleterecording=false;

				audiorecordingfiles.AddEndO(ahd);

				return ahd;
			}
		}

		ahd->FreeMemory();
		delete ahd; // Error...
	}

	return 0;
}


void mainAudio::CreateSendPopMenu(guiWindow *win,AudioIOFX *io,AudioSend *oldsend)
{
	if(!win)return;

	Seq_Song *song=(Seq_Song *)win->WindowSong();
	if(!song)return;

	AudioChannel *bus;

	win->DeletePopUpMenu();

	if(bus=song->audiosystem.FirstBusChannel())
	{
		if(win->DeletePopUpMenu(true))
		{
			if(oldsend) // Delete old
			{
				class menu_oldbypass:public guiMenu
				{
				public:
					menu_oldbypass(Seq_Song *s,AudioIOFX *i,AudioSend *send)
					{
						song=s;
						io=i;
						audiosend=send;
					}

					void MenuFunction()
					{
						audiosend->sendbypass=audiosend->sendbypass==true?false:true;
						maingui->RefreshAudioMixer(song,&io->audioeffects);
					}	

					Seq_Song *song;
					AudioIOFX *io;
					AudioSend *audiosend;
				};

				class menu_oldsend:public guiMenu
				{
				public:
					menu_oldsend(Seq_Song *s,AudioIOFX *i,AudioSend *send)
					{
						song=s;
						io=i;
						audiosend=send;
					}

					void MenuFunction()
					{
						// Delete Send
						mainthreadcontrol->Lock(CS_audioplayback);
						mainthreadcontrol->Lock(CS_audiorealtime); // Realtime Effects

						io->DeleteSend(audiosend,true);

						mainthreadcontrol->Unlock(CS_audiorealtime);
						mainthreadcontrol->Unlock(CS_audioplayback);

						maingui->RefreshAudioMixer(song,&io->audioeffects);
					}	

					Seq_Song *song;
					AudioIOFX *io;
					AudioSend *audiosend;
				};

				if(char *h=mainvar->GenerateString("->",oldsend->sendchannel->GetFullName()))
				{
					win->popmenu->AddMenu(h,0);
					delete h;
					win->popmenu->AddLine();
				}

				win->popmenu->AddFMenu("Send Bypass",new menu_oldbypass(song,io,oldsend),oldsend->sendbypass);
				win->popmenu->AddLine();


				win->popmenu->AddFMenu(Cxs[CXS_NOSEND],new menu_oldsend(song,io,oldsend));
				win->popmenu->AddLine();

				class menu_oldpre:public guiMenu
				{
				public:
					menu_oldpre(AudioSend *send,bool sp)
					{
						audiosend=send;
						sendpost=sp;
					}

					void MenuFunction()
					{	
						mainthreadcontrol->LockActiveSong();
						audiosend->sendpost=sendpost;
						mainthreadcontrol->UnlockActiveSong();
					}	

					AudioSend *audiosend;
					bool sendpost;
				};

				win->popmenu->AddFMenu("Pre-Fader",new menu_oldpre(oldsend,false),oldsend->sendpost==false?true:false);
				win->popmenu->AddFMenu("Post-Fader",new menu_oldpre(oldsend,true),oldsend->sendpost);

				win->popmenu->AddLine();
			}

			while(bus && bus->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL)
			{
				class menu_newsend:public guiMenu
				{
				public:
					menu_newsend(Seq_Song *s,AudioIOFX *i,AudioChannel *b)
					{
						song=s;
						io=i;
						audiobus=b;
					}

					void MenuFunction()
					{
						AudioSend *check=io->FirstSend();

						while(check && check->sendchannel!=audiobus)
							check=check->NextSend();

						if(!check)
						{
							// Replace
							mainthreadcontrol->Lock(CS_audioplayback);
							mainthreadcontrol->Lock(CS_audiorealtime); // Realtime Effects

							io->AddSend(audiobus);

							mainthreadcontrol->Unlock(CS_audiorealtime);
							mainthreadcontrol->Unlock(CS_audioplayback);

							maingui->RefreshAudioMixer(song,&io->audioeffects);
						}
					} //

					Seq_Song *song;
					AudioIOFX *io;
					AudioChannel *audiobus;
				};

				AudioSend *check=io->FirstSend();

				while(check && check->sendchannel!=bus)
					check=check->NextSend();

				if(!check) // Send already to bus
					win->popmenu->AddFMenu(bus->GetFullName(),new menu_newsend(song,io,bus),check?true:false);

				bus=bus->NextChannel();
			}

			win->ShowPopMenu();
		}
	}
}

void mainAudio::AddTrackMenu(Seq_Track *track,guiMenu *menu,bool fix)
{
	AudioEffects *effects=&track->io.audioeffects;

	if(fix==true || effects->FirstInsertAudioEffect() /*|| effects->FirstActiveAudioInstrument()*/)
	{
		class menu_copy:public guiMenu
		{
		public:
			menu_copy(AudioEffects *e){effects=e;}

			void MenuFunction()
			{
				mainbuffer->CopyEffectList(effects);
			} //

			AudioEffects *effects;
		};

		menu->AddFMenu(Cxs[CXS_COPYINSTRUMENTSANDEFFECT],new menu_copy(effects),fix==true?SK_COPY:0);
	}

	class menu_paste:public guiMenu
	{
	public:
		menu_paste(AudioEffects *e){effects=e;}

		void MenuFunction()
		{
			mainbuffer->PasteBufferToEffectList(effects);
		} //

		AudioEffects *effects;
	};

	if(fix==true || mainbuffer->CheckHeadBuffer(OBJ_EFFECTLIST)==true)
		menu->AddFMenu(Cxs[CXS_PASTE],new menu_paste(effects),fix==true?SK_PASTE:0);

	menu->AddLine();

	class menu_deletefx:public guiMenu
	{
	public:
		menu_deletefx(AudioEffects *e)
		{
			effects=e;
		}

		void MenuFunction()
		{
			mainedit->DeletePlugins(effects);

			/*
			bool refresh=effects->DeleteEffectsFlag(flag);

			if(refresh==true)
			maingui->RefreshEffects(effects);
			*/
		} //
		AudioEffects *effects;
	};

	if(fix==true || effects->FirstInsertAudioEffect() /*|| effects->FirstActiveAudioInstrument()*/)
		menu->AddFMenu(Cxs[CXS_DELETEALLIE],new menu_deletefx(effects));

	/*
	if(fix==true || effects->FirstActiveAudioInstrument())
	menu->AddFMenu(Cxs[CXS_DELETEINSTRUMENTS],new menu_deletefx(effects,AudioEffects::DELETE_INSTRUMENTS));
	*/

	//if(fix==true || effects->FirstInsertAudioEffect())
	//	menu->AddFMenu(Cxs[CXS_DELETEEFFECTS],new menu_deletefx(effects));
}

void mainAudio::ReplaceAudioFiles()
{
	// Reset Counter
	{
		Seq_Project *p=mainvar->FirstProject();
		while(p){
			Seq_Song *s=p->FirstSong();
			while(s){
				s->refreshaudiofilescounter=0;
				s=s->NextSong();
			}

			p=p->NextProject();
		}
	}

	int refreshcounter=0;
	mainthreadcontrol->LockActiveSong();
	{
		AudioHDFile *f=FirstAudioHDFile();

		while(f){

			if(f->replacewithfile){

				f->Open(f->replacewithfile);

				if(f->errorflag==0)
				{
					refreshcounter++;

					// New File-> Songs
					Seq_Project *p=mainvar->FirstProject();
					while(p)
					{
						Seq_Song *s=p->FirstSong();
						while(s)
						{
							s->refreshaudiofilescounter+=s->ReplacedFile(f);
							s=s->NextSong();
						}

						p=p->NextProject();
					}
				}
			}

			f=f->NextHDFile();
		}

		// Refresh Song Playback
		{
			Seq_Project *p=mainvar->FirstProject();
			while(p){

				Seq_Song *s=p->FirstSong();

				while(s){

					if(s==mainvar->GetActiveSong() && s->refreshaudiofilescounter){
						s->CheckPlaybackRefresh();
						break;
					}

					s=s->NextSong();
				}

				p=p->NextProject();
			}
		}
	}
	mainthreadcontrol->UnlockActiveSong();

	// Refresh GUI
	if(refreshcounter)
	{
		AudioHDFile *f=FirstAudioHDFile();

		while(f){

			if(f->replacewithfile){

				delete f->replacewithfile;
				f->replacewithfile=0;

				if(f->errorflag==0){
					refreshcounter--;
					maingui->RefreshAudioHDFile(f,0,true,refreshcounter);
				}
			}

			f=f->NextHDFile();
		}
	}
}

AudioHDFile *mainAudio::FindAudioHDFile(char *name)
{
	if(!name)return 0;

	// Search Database
	AudioHDFile *f=FirstAudioHDFile();

	while(f){

		if(strcmp(name,f->GetName())==0)return f;

		f=f->NextHDFile();
	}

	return 0;
}

bool mainAudio::FindAudioHDFile(AudioHDFile *hd)
{
	if(!hd)return false;

	AudioHDFile *f=FirstAudioHDFile();

	while(f){
		if(f==hd)return true;
		f=f->NextHDFile();
	}

	return false;
}

AudioHDFile *mainAudio::AddAudioFileQ(char *name,bool camximport)
{
	AudioHDFile *ahd=new AudioHDFile;
	if(!ahd)return 0;

	if(ahd->SetName(name))
	{	
		ahd->camximport=camximport;
		ahd->InitHDFile();

		if(ahd->m_ok==true)
		{
			LockAudioFiles();
			audiohdfiles.AddEndO(ahd);
			UnlockAudioFiles();

			if(!activehdfile)activehdfile=ahd;

		}
		else
		{
			ahd->FreeMemory();
			delete ahd;
			return 0;
		}
	}
	else
	{
		ahd->FreeMemory();
		delete ahd;
		return 0;
	}

	return ahd;
}

AudioHDFile *mainAudio::AddAudioFile(char *name,bool refreshgui,bool camximport)
{	
	if(!name)return 0;

	AudioHDFile *ahd=FindAudioHDFile(name);

	if(!ahd)
	{
		ahd=AddAudioFileQ(name,camximport);

		if(refreshgui==true)RefreshAudioFileGUI(ahd);
	}
	else
		if(ahd->errorflag&OPENREADFILE_ERROR)
		{
			ahd->InitHDFile();
			if(refreshgui==true)RefreshAudioFileGUI(ahd);
		}

		return ahd;
}

AudioRegion *mainAudio::FindAudioRegion(AudioHDFile *hd,AudioRegion *reg)
{
	if(hd && reg)
	{
		AudioRegion *c=hd->FirstRegion();

		while(c){
			if(c->Compare(reg)==true)return c;
			c=c->NextRegion();
		}
	}

	return 0;
}

void mainAudio::AddAudioFile(AudioHDFile *ahd,bool refreshgui)
{	
	if(ahd)
	{
		ahd->CreateFileName();
		ahd->InitHDFile();

		LockAudioFiles();
		audiohdfiles.AddEndO(ahd);
		if(!activehdfile)activehdfile=ahd;
		UnlockAudioFiles();

		if(refreshgui==true)
			RefreshAudioFileGUI(ahd);
	}
}

void mainAudio::Load(camxFile *file)
{
	int srate=48000;
	bool lockaudiofile=false;

	file->ReadChunk(&srate);

	file->ReadChunk(&defaultchannel_type);
	file->ReadChunk(&defaultchannelindex_out);
	file->ReadChunk(&defaultchannelindex_in);
	file->ReadChunk(&rafbuffersize_index);
	file->ReadChunk(&defaultchannelins);
	file->ReadChunk(&ignorecorrectaudiofiles);

	file->CloseReadChunk();

	file->LoadChunk();
}

void mainAudio::MoveRecFileToHDFiles(AudioHDFile *rec)
{
	if(rec)
	audiorecordingfiles.MoveOToEndOList(&audiohdfiles,rec);
}

void mainAudio::RemoveAudioRecordingFile(AudioHDFile *rec)
{
	audiorecordingfiles.RemoveO(rec);
}

unsigned __int64 mainAudio::GetFreeRecordingMemory(Seq_Song *song,bool *ok)
{
	unsigned __int64 i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
	BOOL  fResult = GetDiskFreeSpaceEx((LPCTSTR)song->directoryname, (PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);

	if(fResult==1)
	{
		*ok=true;
		return i64FreeBytesToCaller;
	}

	*ok=false;
	return 0;

	/*
	AudioHDFile *recfile=FirstAudioRecordingFile();

	if(!recfile)return 0;

	unsigned __int64 i64FreeBytesToCaller_all;
	bool freeset=false;

	while(recfile)
	{
	if(recfile->track)
	{
	unsigned __int64 i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
	BOOL  fResult = GetDiskFreeSpaceEx((LPCTSTR)song->directoryname, (PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);

	if(fResult==1)
	{
	*ok=true;

	if(freeset==false)
	{
	i64FreeBytesToCaller_all=i64FreeBytesToCaller;
	freeset=true;
	}
	else
	{
	if(i64FreeBytesToCaller<i64FreeBytesToCaller_all)
	i64FreeBytesToCaller_all=i64FreeBytesToCaller;
	}
	}
	}

	recfile=recfile->NextHDFile();
	}

	return i64FreeBytesToCaller_all;
	*/
}

void mainAudio::InitDefaultPorts()
{
	AudioHardware *hw=FirstAudioHardware();
	while(hw)
	{
		AudioDevice *dev=hw->FirstDevice();
		while(dev)
		{
			dev->InitDefaultPorts();
			dev=dev->NextDevice();
		}

		hw=hw->NextHardware();
	}
}

int mainAudio::GetCountOfAudioFiles()
{
	int c=0;
	AudioHDFile *hdf=FirstAudioHDFile();

	while(hdf)
	{
		if(hdf->deleted==false)c++;
		hdf=hdf->NextHDFile();
	}
	return c;
}

void mainAudio::LoadHDFiles(camxFile *file)
{
	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_AUDIOHDFILES)
	{
		int errorflags=0;

		file->ChunkFound();
		int nrhdfiles=0;
		file->ReadChunk(&nrhdfiles);
		file->CloseReadChunk();

		while(nrhdfiles--)
		{
			file->LoadChunk();

			if(file->CheckReadChunk()==false)break;

			if(file->GetChunkHeader()==CHUNK_AUDIOHDFILE)
			{
				file->ChunkFound();

				if(AudioHDFile *ahd=new AudioHDFile)
				{
					if(ahd->LoadHDFile(file,false)==false)
					{
						errorflags|=ahd->errorflag;
						DeleteHDFile(ahd); // exists
					}
					else
					{
						mainaudio->AddAudioFile(ahd,false);
						file->audiofilesadded++;
					}
				}
				else
					file->JumpOverChunk();
			}
		}

		if(errorflags)
		{
			if(errorflags&OPENREADFILE_ERROR)
				maingui->MessageBoxOk(0,Cxs[CXS_AUDIOFILEERROR_FILEUNABLE]);

			if(errorflags&AUDIOFILECHECK_ERROR)
				maingui->MessageBoxOk(0,Cxs[CXS_AUDIOFILEERROR_FILETYPE]);

			if(errorflags&AUDIOFILENAME_ERROR)
				maingui->MessageBoxOk(0,Cxs[CXS_AUDIOFILEERROR_FILENAME]);
		}
	}
}

void mainAudio::SaveHDFiles(camxFile *file)
{
	// Audio HD Files
	if(GetCountOfAudioFiles())
	{
		file->OpenChunk(CHUNK_AUDIOHDFILES);
		file->Save_Chunk(GetCountOfAudioFiles());
		file->CloseChunk();

		AudioHDFile *hdf=FirstAudioHDFile();
		while(hdf)
		{
			if(hdf->deleted==false)hdf->Save(file);
			hdf=hdf->NextHDFile();
		}
	}
}

void mainAudio::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_MAINAUDIO);
	file->Save_Chunk(samplerate);

	file->Save_Chunk(defaultchannel_type);
	file->Save_Chunk(defaultchannelindex_out);
	file->Save_Chunk(defaultchannelindex_in);
	file->Save_Chunk(rafbuffersize_index);
	file->Save_Chunk(defaultchannelins);
	file->Save_Chunk(ignorecorrectaudiofiles);

	file->CloseChunk();
}


int mainAudio::ConvertLogArrayVolumeToIntNoAdd(double vol)
{
	for(int i=0;i<AUDIOMIXER_SUB;i++)
	{
		if(logvolume[i]>=vol)
			return i;
	}

	return AUDIOMIXER_SUB;
}

double mainAudio::ConvertToLogScale(double p)
{
	if(p<0)
		p=0;
	else
		if(p>LOGVOLUME_SIZE-1)
			p=LOGVOLUME_SIZE-1;

	double h=LOGVOLUME_SIZE;
	double h2=p;

	h2/=h;

	return h2;
}

int mainAudio::FindLogArrayVolume(double vol)
{
	for(int i=0;i<LOGVOLUME_SIZE;i++)
	{
		if(logvolume[i]>=vol)
			return i;
	}

	return LOGVOLUME_SIZE;
}

double mainAudio::ConvertPPQToInternRate(double ppq)
{
	return ppq/ppqrateinternmul;
}

double mainAudio::ConvertInternRateToPPQ(double ppq)
{
	return ppq*ppqrateinternmul;
}

int mainAudio::ConvertLogArrayVolumeToInt(double vol)
{
	vol*=LOGVOLUME_SIZE;
	vol=logvolume[(int)vol];

	return FindLogArrayVolume(vol);
}

double mainAudio::ConvertDBToLogArrayFactor(double db)
{
	double fac=mainaudio->ConvertDbToFactor(db);
	double h=mainaudio->FindLogArrayVolume(fac);
	double h2=LOGVOLUME_SIZE;
	h/=h2;

	return h;
}

bool mainAudio::DeleteAudioFile(char *file)
{
	if(file)
	{
		// 1. Stop Peak Creation
		audiopeakthread->StopPeakFile(file);

		// 2. Delete existing Peak File+file
		return DeleteAudioFileAndPeakBuffer(file);
	}

	return false;
}

AudioHDFile *mainAudio::DeleteHDFile(AudioHDFile *f)
{
	f->FreeMemory();

	if(f->GetList()) // Inside List ?
		return (AudioHDFile *)audiohdfiles.RemoveO(f);

	delete f;
	return 0;
}

void mainAudio::CloseAllHDFiles()
{
	AudioHDFile *raf=FirstAudioHDFile();
	while(raf)raf=DeleteHDFile(raf);
}

int mainAudio::FindAudioHDFileInsideSong(AudioHDFile *f)
{
	int c=0;

	Seq_Project *p=mainvar->FirstProject();
	while(p)
	{
		Seq_Song *s=p->FirstSong();

		while(s)
		{
			Seq_Track *t=s->FirstTrack();

			while(t)
			{
				Seq_Pattern *p=t->FirstPattern(MEDIATYPE_AUDIO);
				while(p)
				{
					AudioPattern *ap=(AudioPattern *)p;
					if(ap->audioevent.audioefile==f)c++;
					p=p->NextPattern(MEDIATYPE_AUDIO);
				}

				t=t->NextTrack();
			}

			s=s->NextSong();
		}

		p=p->NextProject();
	}

	return c;
}

void mainAudio::RemoveFileNotFound()
{
	int deletecount=0;
	AudioHDFile *f=mainaudio->FirstAudioHDFile();
	while(f)
	{
		if(f->filenotfound==true && FindAudioHDFileInsideSong(f)==0)
		{
			if(activehdfile==f)activehdfile=0;

			AudioHDFile *next=f->NextHDFile();
			maingui->RemoveAudioHDFileFromGUI(f);
			deletecount++;

			DeleteHDFile(f);
			f=next;
		}
		else
			f=f->NextHDFile();
	}

	if(deletecount)
	{
		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			guiWindow *n=w->NextWindow();

			switch(w->GetEditorID())
			{
			case EDITORTYPE_AUDIOMANAGER:
				{
					Edit_Manager *em=(Edit_Manager *)w;

					//	if(em->refreshgui==true)
				 {
					 //	 em->refreshgui=false;

					 em->InitList();
					 em->ShowAudioFiles();
					 em->RefreshHDFile(0);

					 // em->ShowActiveHDFile();
					 // em->ShowActiveHDFile_Info();
				 }
				}
				break;

			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)w;

					/*
					if(deletecounter>0)
					{
					ar->ShowHoriz(false,false,true);
					ar->ShowPatternList();
					}
					else
					*/

					// ar->RefreshAudioRegion(deadregion); // Remove From Samples
				}
				break;

			case EDITORTYPE_EVENT:
				{
					// Edit_Event *e=(Edit_Event *)w;

					// e->ShowAllEvents();
				}
				break;
			}

			w=n;
		}
	}
}

void mainAudio::FindNotFoundFiles(guiWindow *win,bool allfoundmsg)
{
	int c=0;
	AudioHDFile *f=mainaudio->FirstAudioHDFile();
	while(f)
	{
		if(f->filenotfound==true)
			c++;

		f=f->NextHDFile();
	}

	if(c)
	{
		char h[256];
		char h2[NUMBERSTRINGLEN];

		strcpy(h,"Found ");
		mainvar->AddString(h,mainvar->ConvertIntToChar(c,h2));
		mainvar->AddString(h,"'File not found' Audio files!\nManually search for files ?");

		if(maingui->MessageBoxYesNo(0,h)==true)
		{
			int found=0;
			camxFile search;

			if(search.SelectDirectory(win,0,Cxs[CXS_SELECTSEARCHDIR])==true)
			{
				TRACE ("Find in %s\n",search.filereqname);

				AudioHDFile *f=mainaudio->FirstAudioHDFile();
				while(f)
				{
					if(f->filenotfound==true && f->GetFileName())
					{
						char *h=mainvar->GenerateString(search.filereqname,"\\",f->GetFileName());

						if(h)
						{
							TRACE ("Find %s\n",h);

							camxFile test;

							if(test.OpenRead(h)==true)
							{
								test.Close(true);

								if(mainaudio->CheckIfAudioFile(h)==true)
								{
									TRACE("Found !\n");
									found++;
									f->replacewithfile=h;
									h=0;
								}
							}
							else
								test.Close(true);

							if(h)
								delete h;
						}
					}

					f=f->NextHDFile();
				}
			}

			search.Close(true);

			if(found)
			{	
				mainaudio->ReplaceAudioFiles();

				strcpy(h,"Found/Replaced ");
				mainvar->AddString(h,mainvar->ConvertIntToChar(found,h2));
				mainvar->AddString(h,"missing 'File not found' Audio Files!");
				maingui->MessageBoxOk(0,h);
			}
			else
				maingui->MessageBoxOk(0,"No missing File found ...");
		}
	}
	else
	{
		if(allfoundmsg==true)
			maingui->MessageBoxOk(0,Cxs[CXS_NOFILESMISSED]);
	}
}

AudioHDFile *mainAudio::FirstAudioHDFile()
{
	LockAudioFiles();
	AudioHDFile *fdh=(AudioHDFile *)audiohdfiles.GetRoot();
	UnlockAudioFiles();

	return fdh;
}

AudioHDFile *mainAudio::AddAudioFile(AudioHDFile *ahd)
{
	if(!ahd)return 0;

	AudioHDFile *find=FindAudioHDFile(ahd->GetName());

	if(!find)
	{
		ahd->CreateFileName();
		ahd->InitHDFile();

		LockAudioFiles();
		audiohdfiles.AddEndO(ahd);
		UnlockAudioFiles();

		return ahd;
	}

	return find;
}

void mainAudio::AddAudioFileNoCheck(AudioHDFile *ahd,Directory *dir)
{
	ahd->CreateFileName();
	ahd->directory=dir;

	LockAudioFiles();
	audiohdfiles.AddEndO(ahd);
	UnlockAudioFiles();
}

#define CAMX_DEFAULTAUDIODATABASE "\\DataBase\\DB_AudioFiles.cxdb"

void mainAudio::LoadDataBase()
{
	if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),CAMX_DEFAULTAUDIODATABASE))
	{
		camxFile file;

		if(file.OpenRead(h)==true){
			char text[4];
			file.Read(text,4);

			if(text[0]=='C' && text[1]=='A' && text[2]=='D' && text[3]=='B')
			{
				LoadHDFiles(&file);
			}

			//	ok=MessageBoxYesNo(0,"Load Auto/Default Song ?");
		}

		file.Close(true);
		delete h;
	}
}

void mainAudio::SaveDataBase()
{
	if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),CAMX_DEFAULTAUDIODATABASE))
	{
		camxFile file;

		if(file.OpenSave(h)==true){

			char text[4];

			text[0]='C';
			text[1]='A';
			text[2]='D';
			text[3]='B';

			file.Save(text,4);

			SaveHDFiles(&file);

			//	ok=MessageBoxYesNo(0,"Load Auto/Default Song ?");
		}

		delete h;
		file.Close(true);
	}
}