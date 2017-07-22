#include "songmain.h"
#include "transporteditor.h"

#include "audiohardware.h"
#include "audiodevice.h"

#include "gui.h"
#include "semapores.h"
#include "camxfile.h"

#include "MIDIhardware.h"
#include "MIDIfile.h"
#include "audiohdfile.h"
#include "languagefiles.h"

#include "object_project.h"
#include "object_song.h"
#include "object_track.h"
#include "audiomixeditor.h"
#include "player.h"
#include "chunks.h"

#include "editfunctions.h"

/*
int pro_measureindex[]=
{
4,
4,
4,
4,

3,
3,
3,
3
};
*/

void Seq_Project::AddSong(Seq_Song *song)
{
	song->audiosystem.device=mainaudio->GetActiveDevice();
	song->audiosystem.ConnectToDevice(false);
	song->timetrack.ppqsampleratemul=ppqsampleratemul;

	songs.AddEndO(song);
}

Seq_Song *Seq_Project::CreateNewSong(char *name,int flag,Seq_Song *prevsong,char *filename)
{
	if(Seq_Song *song=new Seq_Song(this))
	{
		//song->audiosystem.ConnectToDevice(false);

		if(!(flag&CREATESONG_LOAD)) // Init, create dir and set defaults
		{
			song->audiosystem.masterchannel.InitDefaultAutomationTracks(0,&song->audiosystem.masterchannel);

			// Create SongName
			if(name)
			{
				song->songname=mainvar->GenerateString(name);
				songcounter++;
			}
			else
			{
				bool songnamefound=false;
				char nrs[NUMBERSTRINGLEN];

				songcounter++;
				char *number=mainvar->ConvertIntToChar(songcounter,nrs);

				if(song->songname=mainvar->GenerateString("Song ",number))songnamefound=true;				
			}

			// Create Song Directory
			if(!(flag&CREATESONG_NONEWDIRECTORY))
			{
				if(song->directoryname=mainvar->GenerateString(projectdirectory,"\\",song->songname))
				{
					bool createok=mainvar->CreateNewDirectory(song->directoryname);

					if(createok==false)
					{ 
						// Add Date
						SYSTEMTIME systime;

						GetSystemTime(&systime);

						char nrsstd[NUMBERSTRINGLEN],
							nrsmin[NUMBERSTRINGLEN],
							nrssec[NUMBERSTRINGLEN];

						char *timestrstd=mainvar->ConvertIntToChar(systime.wHour,nrsstd),
							*timestrmin=mainvar->ConvertIntToChar(systime.wMinute,nrsmin),
							*timestrsec=mainvar->ConvertIntToChar(systime.wSecond,nrssec),
							*ndn=mainvar->GenerateString(song->directoryname,"_",timestrstd,timestrmin,timestrsec);

						if(ndn)
						{
							delete song->directoryname;
							song->directoryname=ndn;

							createok=mainvar->CreateNewDirectory(song->directoryname);
						}
					}

					if(createok==false)
					{
						song->FreeMemory(true);
						maingui->MessageBoxOk(0,Cxs[CXS_CANTCREATESONGDIRECTORY]);
						return 0;
					}
					else
					{
						// Create Song Sub Dirs
						if(char *h=mainvar->GenerateString(song->directoryname,"\\","Audio Imports"))
						{
							bool subcreateok=mainvar->CreateNewDirectory(h);
							delete h;
						}

						if(char *h=mainvar->GenerateString(song->directoryname,"\\",DIRECTORY_FREEZETRACKS))
						{
							bool subcreateok=mainvar->CreateNewDirectory(h);
							delete h;
						}

						if(filename && (flag&CREATESONG_IMPORTFROMFILE))
						{
							camxFile import;

							if(import.OpenRead(filename)==true)
							{
								song->Load(&import);
							}

							import.Close(true);
						}
					}
				}	
			}
			else
			{
				if(filename)
				{
					song->directoryname=mainvar->GenerateString(filename);
				}
			}

			// song->drummap=maindrummap->FirstDrummap(); // Set Default Drummap

			//if(flag&CREATESONG_CREATEAUDIOMASTER)
			//{
			//	song->audiosystem.ConnectToDevice();
			//}

			song->loaded=true;

		}//if FLAG CreateSong

		//song->timetrack.ppqsampleratemul=ppqsampleratemul;

		//	song->SongInit();

		if(prevsong)
			songs.AddPrevO(song,prevsong);
		else
			songs.AddEndO(song);

		// Dummy Channels
		if(flag&CREATESONG_CREATEAUDIOMASTER)
		{
			//song->audiosystem.ChangeAllToAudioDevice(mainaudio->GetActiveDevice(),false);

			if(!(flag&CREATESONG_IMPORTFROMFILE))
			{
				song->InitDefaultMetroTracks();
				song->audiosystem.InitDefaultBusses();
				song->InitDefaultDrumMap();
			}

			song->audiosystem.CreateChannelsFullName();
		}		


		song->RepairSongAfterLoading();

		//Set Active Song
		if(!(flag&CREATESONG_NOACTIVATE))
		{
			if((!mainvar->GetActiveSong()) || (flag&CREATESONG_ACTIVATE))
				mainvar->SetActiveSong(song);
		}

		if(!(flag&CREATESONG_NONEWDIRECTORY))
			Save(0);

		if(!(flag&CREATESONG_NOGUIREFRESH))
			maingui->RefreshProjectGUI();

		return song;
	}

	return 0;
}

void Seq_Project::CloseAllButSong(guiScreen *screen,Seq_Song *song)
{
	if(!song)
		return;

	char *h=mainvar->GenerateString(Cxs[CXS_CLOSEALLOTHERSONGS],"?\n",Cxs[CXS_PROJECT],":",name,"\nSong:",song->GetName());

	if(h)
	{
		if(maingui->MessageBoxYesNo(0,h)==true)
		{
			mainvar->SetActiveSong(song); 

			Seq_Song *s=FirstSong();

			while(s)
			{
				if(s->loaded==true && s!=song)
				{
					mainedit->DeleteSong(screen,s,EditFunctions::DELETESONG_FLAG_NOGUIREFRESH);
					s=FirstSong();
				}
				else
					s=s->NextSong();
			}

			maingui->RefreshProjectScreens(this);
		}

		delete h;
	}
}

bool Seq_Main::CheckSampleRateOfNewDevice(AudioDevice *device)
{
	if(!device)return false;

	Seq_Project *p=FirstProject();

	while(p)
	{
		if(p->loadok==true && (p->GetCountOfAudioPattern()==0 || device->CheckSampleRate(p->projectsamplerate)==true))
			return true;

		p=p->NextProject();
	}

	return true;
}

int Seq_Project::GetCountOfAudioPattern()
{
	int c=0;

	Seq_Song *s=FirstSong();

	while(s)
	{
		c+=s->GetCountOfAudioPattern();
		s=s->NextSong();
	}

	return c;
}

Seq_Song *Seq_Project::DeleteSong(Seq_Song *song,int flag)
{
	Seq_Song *n=song->NextSong();

	// mainthreadcontrol->LockActiveSong();
	if(!(flag&DELETESONG_ONLYCLOSE))
		songs.CutObject(song); // remove from list

	song->FreeMemory(flag);
	//mainthreadcontrol->UnlockActiveSong();
	return n;
}

Seq_Song *Seq_Main::GetActiveSong()
{
	return activeproject?activeproject->activesong:0;
}

void Seq_Main::ChangeAllToSong(Seq_Song *song,Seq_Project *toproject,bool setactive)
{
	if(song==mainvar->GetActiveSong())
		return;

	bool error=false;

	//maingui->MessageBoxOk(0,"T1");

	Seq_Song *oldsong=mainvar->GetActiveSong();

	if(song && (song->loadwhenactivated==true || song->loaded==false))
	{
		//	maingui->MessageBoxOk(0,"T2D");
		TRACE ("Load Song... %s\n",song->songname);
		song->loadwhenactivated=false;
		song->Load(0);

		if(song->loaded==false)
		{
			song->FreeMemory(Seq_Project::DELETESONG_ONLYCLOSE);
			error=true;
		}
	}

	if(oldsong && setactive==true){

		oldsong->StopSong(0,oldsong->GetSongPosition()); // Stop no Thru/Fx Refresh
		mainaudioreal->StopAllRealtimeEvents();
	}

	//maingui->MessageBoxOk(0,"T3");

	if(setactive==true)
	{
		mainthreadcontrol->LockActiveSong();

		if(oldsong)
		{
			oldsong->audiosystem.ResetChannels(); // Reset Peaks ...
			oldsong->SendAllOpenNotes();
		}
	}

	if(GetActiveProject() || toproject)
	{
		if(song && error==false)
		{
			song->startaudioinit=false;
			song->startMIDIinit=false;
			song->status=Seq_Song::STATUS_STOP; // Reset
			song->CreateQTrackList();
		}

		if(setactive==true)
		{
			if(error==true)
			{
				if(toproject)
					toproject->activesong=0;
				else
					GetActiveProject()->activesong=0;
			}
			else
			{
				if(toproject)
					toproject->activesong=song;
				else
					GetActiveProject()->activesong=song;
			}
		}
	}

	if(setactive==true)
	{
		if(song && error==false)
		{
			if(!toproject)
				activeproject=song->project;
		}
	}

	if(song && error==false)
	{
		song->Activated(); // Refresh Buffer etc..

		if(setactive==true)
		{
			mainthreadcontrol->UnlockActiveSong();
			mainMIDI->SendSongPosition(song,song->GetSongPosition(),true);
		}
	}
	else
	{
		if(setactive==true)
			mainthreadcontrol->UnlockActiveSong();
	}
}

void Seq_Main::LoadProject(guiScreen *screen)
{
	camxFile profile;

	if(profile.OpenFileRequester(screen,0,Cxs[CXS_OPEN_PROJECT],profile.AllFiles(camxFile::FT_PROJECT),true)==true)
	{
		if(OpenProject(screen,profile.fpath)){
			mainsettings->Save(0);
			//mainaudio->FindNotFoundFiles();
			maingui->RefreshProjectGUI();
		}

		profile.Close(true);
	}
}

void Seq_Main::QuestionCloseProject(Seq_Project *project)
{
	if(!project)return;

	char h[NUMBERSTRINGLEN];
	char *os=mainvar->ConvertIntToChar(project->GetCountOfOpenSongs(),h);

	if(char *h=GenerateString(Cxs[CXS_CLOSEPROJECT_Q],"\n",project->name,"\n",Cxs[CXS_SONGSOPEN],os))
	{
		if(maingui->MessageBoxYesNo(0,h)==true)
			CloseProject(project);

		delete h;
	}	
}

void Seq_Main::SaveProject(Seq_Project *pro)
{
	if(pro)
		pro->Save(0);
}

Seq_Project *Seq_Main::GetActiveProject()
{
	if(activeproject && activeproject->underdestruction==true)
		return (Seq_Project *)activeproject->NextOrPrev();

	return activeproject;
}

Seq_Project *Seq_Main::CloseProject(Seq_Project *project)
{
	if(!project)return 0;

	project->underdestruction=true;

	if(exitthreads==false)
	{
		maingui->RemoveProjectFromGUI(project);

		if(activeproject==project)
		{
			mainthreadcontrol->LockActiveSong();
			activeproject=(Seq_Project *)activeproject->NextOrPrev();
			mainthreadcontrol->UnlockActiveSong();
		}
	}

	project->CloseAllSongs();
	project->FreeMemory();

	Seq_Project *rn=(Seq_Project *)projects.RemoveO(project);

	if(exitthreads==false)
		maingui->RefreshProjectGUI();

	return rn;
}

void Seq_Main::SetActiveProject(Seq_Project *project,Seq_Song *song)
{
	if(project && GetActiveProject()!=project){

		if(GetActiveSong())
		{
			GetActiveSong()->StopSong(0,GetActiveSong()->GetSongPosition()); // Stop no Thru/Fx Refresh
			mainaudioreal->StopAllRealtimeEvents();
			GetActiveSong()->DeActivated();
		}

		bool error=false;

		if(!song)
		{
			Seq_Song *s=project->FirstSong();

			while(s)
			{
				if(s->loaded==false && s->loadwhenactivated==true)
				{
					song=s;
					break;
				}

				s=s->NextSong();
			}
		}

		if(song && song->loaded==false)
		{
			song->Load(0);

			if(song->loaded==false)
			{
				song->FreeMemory(Seq_Project::DELETESONG_ONLYCLOSE);
				error=true;
			}
		}

		project->SetSampleRate(mainaudio->GetGlobalSampleRate());
		mainaudio->SetGlobalSampleRate(project->projectsamplerate);

		mainthreadcontrol->LockActiveSong();
		activeproject=project;
		activeproject->activesong=0; // force refresh
		ChangeAllToSong(song,activeproject);

		mainthreadcontrol->UnlockActiveSong();

		activeproject->Save(0);

		maingui->RefreshProjectGUI();
	}
	else
		if(!project)
		{
			ChangeAllToSong(0,0);

			mainthreadcontrol->LockActiveSong();
			activeproject=0;
			mainthreadcontrol->UnlockActiveSong();
		}
}


void Seq_Main::SetActiveSong(Seq_Song *csong)
{
	Seq_Song *oldsong=GetActiveSong();

	if(csong==oldsong)
		return;

	if(!csong){

		ChangeAllToSong(0,activeproject);
		if(oldsong)oldsong->DeActivated();
	}
	else
	{
		if(GetActiveProject())
		{
			Seq_Song *s=GetActiveProject()->FirstSong();

			while(s){

				if(s!=csong && (s->status&(Seq_Song::STATUS_RECORD|Seq_Song::STATUS_STEPRECORD)))
				{
					return; // Other Song Recording
					//ChangeAllToSong(csong,activeproject,false);
				}

				s=s->NextSong();
			}
		}

		if(csong->project!=GetActiveProject())
		{
			SetActiveProject(csong->project,csong);
			csong->SendSysExStartupMIDIPattern(); // send SysEx Startup

			goto exit;
		}

		if( (csong->project==GetActiveProject() || csong->autoloadsong==true) && GetActiveSong()!=csong){

			ChangeAllToSong(csong,GetActiveProject());
			csong->SendSysExStartupMIDIPattern(); // send SysEx Startup

			if(GetActiveProject())
				GetActiveProject()->Save(0);
		}


	}
exit:
	if(oldsong!=GetActiveSong())
	{
		maingui->RefreshScreenNames();

		if(oldsong)oldsong->DeActivated();
	}
}

void Seq_Project::CloseAllSongs()
{
	if(mainvar->exitthreads==false && mainvar->GetActiveSong() && mainvar->GetActiveSong()->project==this)
		mainvar->SetActiveSong(0);

	Seq_Song *s=FirstSong();
	while(s)s=DeleteSong(s,DELETESONG_FULL);
}

void Seq_Project::InitPPQ()
{
	ppqsampleratemul=projectsamplerate;
	ppqsampleratemul/=(SAMPLESPERBEAT*2);
}

Seq_Project::Seq_Project()
{
	underdestruction=false;
	activesong=0;
	projectdirectory=0;
	songcounter=0;

	projectsamplerate=mainaudio->GetGlobalSampleRate();
	InitPPQ();

	strcpy(name,"Project");

	//	panlaw_files.SetLaw(PanLaw::PAN_DB_6); // Stereo->Mono Files etc..

	checkaudiostartpeak=mainsettings->usecheckaudiothreshold;
	checkaudiothreshold=mainsettings->checkaudiothreshhold;
	autocutzerosamples=mainsettings->autocutzerosamples;

	standardsmpte=mainsettings->projectstandardsmpte;
	projectmeasureformat=mainsettings->defaultprojectmeasureformat;

	editprojectname=false;
	errorprojectwriting=false;

	realtimerecordtempoevents=mainsettings->realtimerecordtempoevents;
}

void Seq_Project::RefreshRealtime_Slow()
{
	Seq_Song *song=FirstOpenSong();

	while(song)
	{
		if(song->loaded==true)
		{
			song->RefreshRealtime_Slow();
		}

		song=song->NextSong();
	}
}

void Seq_Project::FreeMemory()
{
	if(projectdirectory)delete projectdirectory;
	projectdirectory=0;
}

void Seq_Project::InitSongsTimeTrackPPQ()
{
	Seq_Song *s=FirstSong();
	while(s)
	{
		s->InitInternSampleRate();
		s=s->NextSong();
	}
}

bool Seq_Project::CheckNewSampleRateIsOk(int nsrate)
{
	Seq_Song *s=FirstSong();
	while(s)
	{
		if(s->loaded==true)
		{
			if(s->GetCountOfAudioPattern()>0)
				return false;
		}
		else
		{
			if(s->rinfo_audiopatterncounter>0)
				return false;
		}

		s=s->NextSong();
	}

	return true;
}

void Seq_Project::SetSampleRate(int srate)
{
	if(srate!=projectsamplerate)
	{
		if(GetCountOfAudioPattern()>0)
		{
			projectsamplerate=mainaudio->GetGlobalSampleRate();

			InitPPQ();
			InitSongsTimeTrackPPQ();

			return;
		}

		Seq_Song *s=FirstSong();
		while(s)
		{
			s->ConvertTicksToTempoSampleRate();
			s=s->NextSong();
		}

		projectsamplerate=srate;
	}

	InitPPQ();
	InitSongsTimeTrackPPQ();
}

bool Seq_Project::OpenSong(guiScreen *screen)
{
	Seq_Song *newsong=0;

	camxFile load;

	if (load.OpenFileRequester(screen,0,Cxs[CXS_OPENADD_SONG],load.AllFiles(camxFile::FT_SONGS),true)==true)
	{	
		bool ok=true;

		// Avoid double insert

		// Song inside Project SOng List ?
		Seq_Song *check=FirstSong();
		while(check)
		{
			if(strlen(load.fpath)==strlen(check->directoryname))
			{
				char *c1=load.fpath;
				char *c2=check->directoryname;
				size_t i=strlen(check->directoryname);
				char r1,r2;

				ok=false;
				while(i--)
				{
					r1=toupper(*c1++);
					r2=toupper(*c2++);

					if(r1!=r2)
					{
						ok=true;
						break;
					}
				}

				if(ok==false)
				{
					bool copy=maingui->MessageBoxYesNo(0,Cxs[CXS_SONGALREADYCOPY_Q]);
					if(copy==true)ok=true;
					break;
				}
			}

			check=check->NextSong();
		}

		if(ok==true)
		{
			// Check File
			{
				camxFile testfile;
				Seq_Song *testsong=new Seq_Song(this);

				if(testfile.OpenRead(load.filereqname)==true)
					ok=testsong->LoadSongInfo(&testfile);

				testfile.Close(true);
				testsong->FreeMemory(DELETESONG_FULL);
			}

			if(ok==true)
			{
				int flag=Seq_Project::CREATESONG_NOACTIVATE;

				// Song inside Project Directory ?
				bool insideprojectdir=false;

				if(strlen(load.fpath)>strlen(projectdirectory))
				{
					char *c1=load.fpath;
					char *c2=projectdirectory;
					size_t i=strlen(projectdirectory);
					char r1,r2;

					insideprojectdir=true;
					while(i--)
					{
						r1=toupper(*c1++);
						r2=toupper(*c2++);

						if(r1!=r2)
						{
							insideprojectdir=false;
							break;
						}
					}

					if(insideprojectdir==true)
						flag|=CREATESONG_NONEWDIRECTORY;
				}

				Seq_Song *song=CreateNewSong(0,flag,0,load.fpath);

				if(song && load.filereqname)
				{
					//	song->SetSongDirectory(load.fpath);
					camxFile file;

					if(file.OpenRead(load.filereqname)==true)
					{
						song->Load(&file);

						if(file.errorflag&CHUNK_ERROR)
						{
							file.Close(true);

							// Error
							if(char *h=mainvar->GenerateString(Cxs[CXS_UNABLETOLOADSONG],":\n","Chunk Error"))
							{
								maingui->MessageBoxError(0,h);
								delete h;
							}

							mainedit->DeleteSong(screen,song,Seq_Project::DELETESONG_FULL|Seq_Project::DELETESONG_NOLOCK); // Remove Song Files/Directory etc...
							ok=false;
						}
						else
						{
							file.Close(true);
							maingui->InitNewFileSong(song,screen); // +Save
						}
					}
					else
						file.Close(true);
				}
			}
		}

		load.Close(true);

		return ok;
	}

	return false;
}

void Seq_Project::Load(camxFile *file)
{
	if(!projectdirectory)
		return;

	TRACE ("Load Project %s\n",projectdirectory);
	char *pn=0;

	camxFile sfile; 
	bool autoclose=false;
	loadactivesong=0;

	if(!file){

		if(pn=mainvar->GenerateString(projectdirectory,"\\",PROJECTNAME)){

			file=&sfile;

			if(file->OpenRead(pn)==false)
				file=0;

			autoclose=true;
		}
	}

	if(file){

		if(file->CheckVersion())
		{
			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_PROJECT){

				file->ChunkFound();
				file->Read_ChunkString(name);

				int nrsong=0;
				file->ReadChunk(&nrsong);
				file->ReadChunk(&songcounter);
				file->ReadChunk(&projectsamplerate);
				InitPPQ();

				file->ReadChunk(&checkaudiostartpeak);

				ARES check=-1;
				file->ReadChunk(&check);

				if(check>=0)
					checkaudiothreshold=check;

				file->ReadChunk(&projectmeasureformat);
				file->ReadChunk(&standardsmpte);

				check=-1;
				file->ReadChunk(&check);

				if(check>=0)
					checkaudioendthreshold=check;

				file->ReadChunk(&autocutzerosamples);

				file->CloseReadChunk();

				file->LoadChunk();
				if(file->GetChunkHeader()==CHUNK_PANLAW)
				{
					file->ChunkFound();
					panlaw.Load(file);
					//	panlaw_files.Load(file);

					file->CloseReadChunk();
				}

				file->LoadChunk();

				if(file->GetChunkHeader()==CHUNK_PROJECTSONGS){
					file->ChunkFound();

					loadactivesong=0;

					while(nrsong--){	

						bool actsong=false;

						file->ReadChunk(&actsong);

						if(Seq_Song *song=new Seq_Song(this)){

							bool ok=true;

							file->Read_ChunkString(&song->directoryname);

							// Convert Dir
							if(char *conc=mainvar->GenerateString(song->directoryname))
							{
								size_t sl=strlen(conc);
								char *s2=&conc[sl-1];

								while(sl--)
								{
									if(*s2=='\\' || *s2=='//' || *s2==':')
										break;
									s2--;
								}

								delete song->directoryname;

								song->directoryname=mainvar->GenerateString(projectdirectory,s2);

								delete conc;
							}

							if(song->directoryname)
							{
								// Check if song exists...
								camxFile test;

								if(char *h=mainvar->GenerateString(song->directoryname,"\\cpsong.camx"))
								{
									if(test.OpenRead(h)==true){

										AddSong(song);

										song->LoadSongInfo(0); // Read Song Name etc... no data

										if(actsong==true)
										{
											song->loadwhenactivated=true;
											loadactivesong=song;
										}

										TRACE ("Load Project Song %s\n",song->songname);
									}
									else
									{
										TRACE ("Load Project Song not open\n");
										ok=false;
									}

									test.Close(true);
									delete h;
								}
								else
									ok=false;
							}
							else{
								TRACE ("Project Song has empty directory\n");
								ok=false;
							}

							if(ok==false)
								delete song;
						}
						else
							break;	
					}

					file->CloseReadChunk();
				}

				loadok=true;
			}
		}

		// Read Project
		if(autoclose==true)
			file->Close(true);
	}

	if(pn)
		delete pn;
}

Seq_Project *Seq_Main::OpenProject(guiScreen *screen,char *directory)
{
	if(directory){

		// Check Existing Projects
		Seq_Project *f=FirstProject();
		while(f){

			if(f->projectdirectory && strcmp(f->projectdirectory,directory)==0)
			{
				maingui->MessageBoxOk(0,Cxs[CXS_PROJECTALREADYIN]);
				return 0; // Project already in List
			}

			f=f->NextProject();
		}

		if(char *pn=mainvar->GenerateString(directory)){

			if(Seq_Project *newproject=new Seq_Project){

				newproject->projectdirectory=pn;
				newproject->loadok=false;
				newproject->loadactivesong=0;
				newproject->Load(0);

				//newproject->projectsamplerate=1;
				if(newproject->projectsamplerate>0 && mainaudio->GetActiveDevice() && mainaudio->GetActiveDevice()->CheckSampleRate(newproject->projectsamplerate)==false)
				{
					char h2[NUMBERSTRINGLEN];
					char *h=mainvar->GenerateString(Cxs[CXS_PROJECTUSESNONEXISTINGSAMPLERATE],"\n[Project Sample Rate:",mainvar->ConvertIntToChar(newproject->projectsamplerate,h2),"]");

					if(h)
					{
						maingui->MessageBoxOk(0,h);
						delete h;
					}

					newproject->loadok=false;
				}
				else
					mainaudio->SetGlobalSampleRate(newproject->projectsamplerate);

				if(newproject->loadok==true){

					projects.AddEndO(newproject);

					if(newproject->loadactivesong)
					{
						newproject->loadactivesong->Load(0);
						newproject->loadactivesong->OpenIt(screen);
					}
					else
					{
						if(screen)
						{
							screen->SetNewProject(newproject,0);
						}
						else
							maingui->OpenNewScreen(newproject,newproject->FirstSong());
					}

					mainvar->SetActiveProject(newproject,newproject->loadactivesong);

					maingui->RefreshScreenMenus();

					guiWindow *win=maingui->FirstWindow();
					while(win)
					{
						switch(win->GetEditorID())
						{
						case EDITORTYPE_PLAYER:
							{
								Edit_Player *edp=(Edit_Player *)win;

								edp->ShowProjects();
								edp->ShowSongs();
							}
							break;
						}

						win=win->NextWindow();
					}

					return newproject;
				}
				else{
					newproject->CloseAllSongs();
					newproject->FreeMemory();
					delete newproject;
					return 0;
				}
			}

			delete pn;
		}
	}

	return 0;
}

void Seq_Project::Save(camxFile *file)
{
	if(projectdirectory){

		camxFile sfile;
		bool autoclose=false;
		char *pn=0;

		if(!file){

			if(pn=mainvar->GenerateString(projectdirectory,"\\",PROJECTNAME)){

				file=&sfile;

				if(file->OpenSave_CheckVersion(pn)==true){
				}
				else
					file=0;

				autoclose=true;
			}

			if(file){

				//Save Song List
				char *sl=mainvar->GenerateString(projectdirectory,"\\","Song List.txt");
				if(sl)
				{
					char cr[3];

					cr[0]=0x0D;
					cr[1]=0x0A;
					cr[2]=0;

					camxFile list;

					if(list.OpenSave(sl)==true)
					{
						list.Save("Project:");
						list.Save(name);
						list.Save(cr);
						list.Save("---- Songs ----");
						list.Save(cr);

						Seq_Song *s=FirstSong();

						if(!s)
							list.Save("Empty");

						while(s){

							list.Save(s->songname);
							list.Save(":");

							// Convert Dir
							char *conc=mainvar->GenerateString(s->directoryname);
							if(conc)
							{
								size_t sl=strlen(conc);
								char *s2=&conc[sl-1];

								while(sl--)
								{
									if(*s2=='\\' || *s2=='//' || *s2==':')
									{	
										s2++;
										break;
									}

									s2--;
								}

								list.Save(s2);
								delete conc;
							}

							list.Save(cr);					
							s=s->NextSong();
						}
					}

					list.Close(true);

					delete sl;
				}

				file->SaveVersion();

				// Write Project
				file->OpenChunk(CHUNK_PROJECT);
				file->Save_ChunkString(name);

				file->Save_Chunk(GetCountOfSongs());
				file->Save_Chunk(songcounter);
				file->Save_Chunk(projectsamplerate);
				file->Save_Chunk(checkaudiostartpeak);
				file->Save_Chunk(checkaudiothreshold);
				file->Save_Chunk(projectmeasureformat);
				file->Save_Chunk(standardsmpte);
				file->Save_Chunk(checkaudioendthreshold);
				file->Save_Chunk(autocutzerosamples);

				file->CloseChunk();

				file->OpenChunk(CHUNK_PANLAW);
				panlaw.Save(file);
				//panlaw_files.Save(file);
				file->CloseChunk();

				// Songs -------------
				file->OpenChunk(CHUNK_PROJECTSONGS);

				Seq_Song *s=FirstSong();

				while(s){

					bool act=s==activesong?true:false;

					file->Save_Chunk(act);
					file->Save_ChunkString(s->directoryname);

					s=s->NextSong();
				}

				file->CloseChunk();

				errorprojectwriting=file->errorwriting;

				if(errorprojectwriting==true)
					maingui->MessageBoxError(0,Cxs[ERROR_PROJECTSAVE]);

				if(autoclose==true)
					file->Close(true);
			}
		}

		if(pn)
			delete pn;
	}
}

Seq_Project *Seq_Main::CreateNewProjectWithNewDirectory(guiScreen *screen,int flag)
{
	/*
	if(!screen)
	return;

	Edit_Transport *trans=screen->GetTransport();

	if(!trans)
	return;

	if(trans)
	{
	// Rename Song
	class menu_RenameSong:public guiMenu
	{
	public:
	menu_RenameSong(Edit_Transport *t,Seq_Song *s)
	{
	trans=t;
	song=s;
	}

	void MenuFunction()
	{
	if(song)
	{
	if(EditData *edit=new EditData)
	{
	edit->win=trans;
	edit->x=0;
	edit->y=0;
	edit->name=Cxs[CXS_EDIT_SONG_NAME];
	edit->id=Edit_Transport::EDIT_SONGNAME;
	edit->type=EditData::EDITDATA_TYPE_STRING_TITLE;
	edit->string=song->songname;

	maingui->EditDataValue(edit);
	}
	}	
	} 

	Edit_Transport *trans;
	Seq_Song *song;
	};

	if(char *asng=mainvar->GenerateString(Cxs[CXS_EDIT_SONG_NAME],":",song->songname))
	{
	n->AddFMenu(asng,new menu_RenameSong(trans,song));
	delete asng;
	}
	}

	*/


	camxFile file;

	if(file.SelectDirectory(0,screen,Cxs[CXS_SELECTPROJECTDIR])==true)
	{
		bool ok=false;

		if(char *h=mainvar->GenerateString(Cxs[CXS_CREATEPROJECT_Q],"\n",file.filereqname))
		{
			ok=maingui->MessageBoxYesNo(0,h);
			delete h;
		}

		if(ok)
		{
			// Existiing project ?
			bool exists;
			Seq_Project *newproject=CreateProject(file.filereqname,&exists,true);

			if(newproject){

				//	newproject->samplerate=mainaudio->GetGlobalSampleRate(); // Sample Rate?

				// Ask for new Song...
				if(maingui->MessageBoxYesNo(0,Cxs[CXS_CREATENEWPROJECTSONG])==true)
				{
					Seq_Song *song=newproject->CreateNewSong(0,Seq_Project::CREATESONG_CREATEAUDIOMASTER|flag,0,0);

					if(song)
					{
						if(maingui->AskForDefaultSong(GUI::ID_AUTOLOAD_CAMX,true)==true) // Default ?
							maingui->LoadDefaultSong(screen,song,false);

						//maingui->OpenEditorStart(EDITORTYPE_ARRANGE,song,0,0,0,0,0);
					}
				}

				newproject->Save(0);
				mainsettings->Save(0);

				maingui->OpenNewScreen(newproject,newproject->FirstSong());
				mainvar->SetActiveProject(newproject,newproject->FirstSong());

				return newproject;
			}
		}
	}

	return 0;
}

Seq_Project *Seq_Main::CreateProject(char *directory,bool *exists,bool warning)
{
	if(exists)*exists=false;
	if((!directory) || strlen(directory)==0)return 0;

	if(mainvar->CreateNewDirectory(directory)==false)
	{
		if(warning==true)
			maingui->MessageBoxOk(0,Cxs[CXS_CANTCREATEPROJECTDIRECTORY]);

		return 0;
	}

	// Check existing Project
	camxFile file; 
	char *pn=mainvar->GenerateString(directory,"\\",PROJECTNAME);

	if(!pn)return 0;

	if(file.OpenRead(pn)==true){

		file.Close(true);
		if(exists)*exists=true;

		//	maingui->MessageBoxOk(0,"Load Init Project Exists");
		if(warning==true)
			maingui->MessageBoxOk(0,Cxs[CXS_ACAMXPROALREADYEXISTS]);

		delete pn;
		return 0;
	}

	delete pn;

	if(Seq_Project *newproject=new Seq_Project){

		newproject->projectdirectory=mainvar->GenerateString(directory);

		size_t sl=strlen(directory);
		char *s=&directory[sl-1];
		char *proname=0;

		while(sl--)
		{
			if(*s=='\\' || *s=='/' || *s==':')
				break;

			proname=s;
			s--;
		}

		if(proname)
		{
			sl=strlen(proname);
			if(sl>STANDARDSTRINGLEN-1)
			{
				sl=STANDARDSTRINGLEN-1;
				strncpy(newproject->name,proname,sl);
				proname[sl]=0;
			}
			else
				strcpy(newproject->name,proname);

		}

		projects.AddEndO(newproject);

		if(!activeproject)
			SetActiveProject(newproject,0);

		return newproject;
	}

	return 0;
}

void Seq_Main::SaveAllProject()
{
	bool error,loop;

	do
	{
		Seq_Project *p=FirstProject();

		loop=false;
		error=false;

		while(p){

			p->Save(0);
			if(p->errorprojectwriting==true)
				error=true;

			Seq_Song *s=p->FirstSong();

			while(s){
				if(s->loaded==true)
				{
					s->Save(0);
					if(s->errorsongwriting==true)
						error=true;
				}
				s=s->NextSong();
			}

			p=p->NextProject();
		}

		if(error==true)
		{
			loop=maingui->MessageBoxYesNo(0,Cxs[ERROR_PROJECTSONGSAVE]);
		}

	}
	while(loop==true);
}

void Seq_Main::CloseAllProjects()
{
	Seq_Project *p=FirstProject();
	while(p)p=CloseProject(p);
}

void Seq_Project::SetProjectName(char *nname,bool refreshgui)
{
	if(nname && strcmp(nname,name)!=0)
	{
		size_t sl=strlen(nname);

		if(sl>STANDARDSTRINGLEN-1)
			sl=STANDARDSTRINGLEN-1;

		strncpy(name,nname,sl);
		name[sl]=0;

		if(refreshgui==true)
		{
			maingui->RefreshProjectScreens(this);
		}
	}
}

bool Seq_Project::CheckIfFileIsSong(camxFile *file)
{
	if(file)
	{
		if(file->CheckVersion()==true)
		{
			// Load Chunks	
			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_SONGPOINTER) // First Pointer
				return true;
		}
	}

	return false;
}

Seq_Song *Seq_Project::ImportMIDIFile(guiScreen *screen)
{
	camxFile cMIDIfile;

	if (cMIDIfile.OpenFileRequester(screen,0,Cxs[CXS_LOADSMFFILE],cMIDIfile.AllFiles(camxFile::FT_MIDIFILE),true)==true)
	{		
		int flag=0;
		Seq_Song *song=0;

		switch(mainsettings->splitMIDIfiletype0)
		{
		case Settings::SPLITMIDIFILE0_OFF:
			break;

		case Settings::SPLITMIDIFILE0_ON:
			flag|=MIDIFile::FLAG_CREATEAUTOSPLITRACK;
			break;

		case Settings::SPLITMIDIFILE0_ASKMESSAGE:
			{
				if(maingui->MessageBoxYesNo(0,Cxs[CXS_ASKSPLITMIDIFILE0])==true)
					flag|=MIDIFile::FLAG_CREATEAUTOSPLITRACK;
			}
			break;
		}		

		// Autoload SMF Song ?
		if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),CAMX_DEFAULTAUTOLOADSMFNAME))
		{
			camxFile file;

			if(file.OpenRead(h)==true)
			{
				if(maingui->MessageBoxYesNo(0,Cxs[CXS_LOADAUTOMIDIQ])==true)
				{
					song=CreateNewSong(0,0,0,0);

					if(song)
					{
						song->Load(&file);

						if(file.errorflag&CHUNK_ERROR)
						{
							file.Close(true);
							DeleteSong(song,Seq_Project::DELETESONG_FULL);
							song=0;
						}
						else
						{
							file.Close(true);
							flag=MIDIFile::FLAG_AUTOLOAD;
						}
					}
				}
				else
					file.Close(true);

			}
			else
				file.Close(true);

			delete h;
		}

		if(!song)
		{
			if(mainsettings->importfilequestion==true && maingui->MessageBoxYesNo(0,Cxs[CXS_IMPORTARRANGEMENT_Q])==true)
			{
				camxFile file;

				if(file.OpenFileRequester(screen,0,"Song","CamX Arrangement (*.caar)|*.caar;|All Files (*.*)|*.*||",true)==true)
				{
					if(file.OpenRead(file.filereqname)==true)
					{
						if(CheckIfFileIsSong(&file)==true)
						{
							char *h=mainvar->GenerateString(file.filereqname);

							file.Close(true);

							if(h)
							{
								song=CreateNewSong(0,Seq_Project::CREATESONG_IMPORTFROMFILE|CREATESONG_NOACTIVATE|Seq_Project::CREATESONG_CREATEAUDIOMASTER,0,h);

								flag=MIDIFile::FLAG_FINDTRACK;
								flag|=MIDIFile::FLAG_CREATEAUTOSPLITRACK;

								delete h;
							}
						}
						else
							file.Close(true);
					}
				}
			}

			if(!song)
				song=CreateNewSong(0,CREATESONG_CREATEAUDIOMASTER|CREATESONG_NOACTIVATE|CREATESONG_NOGUIREFRESH,0,0);
		}

		if(song){

			MIDIFile MIDIfile;

			if(MIDIfile.ReadMIDIFileToSong(song,cMIDIfile.filereqname,flag)==true) // Fill our song with MIDI-File
			{
				maingui->InitNewFileSong(song,screen); // +Save
				return song;
			}

			// Error ...
			if(char *h=mainvar->GenerateString(Cxs[CXS_UNABLETOLOADSONG],":\n",Cxs[CXS_NOMIDIFILEHEADER]))
			{
				maingui->MessageBoxError(0,h);
				delete h;
			}

			mainedit->DeleteSong(screen,song,Seq_Project::DELETESONG_FULL|Seq_Project::DELETESONG_NOLOCK); // Remove Song Files/Directory etc...
		}
		else
		{
			if(char *h=mainvar->GenerateString(Cxs[CXS_UNABLETOLOADSONG],":\n",Cxs[CXS_MEMORY]))
			{
				maingui->MessageBoxError(0,h);
				delete h;
			}
		}
	}

	cMIDIfile.Close(true);

	return 0;
}

int Seq_Project::GetCountOfOpenSongs()
{
	int c=0;
	Seq_Song *s=FirstSong();

	while(s)
	{
		if(s->loaded==true)c++;
		s=s->NextSong();
	}

	return c;
}

Seq_Song *Seq_Project::FirstOpenSong()
{
	Seq_Song *s=FirstSong();

	while(s)
	{
		if(s->loaded==true)return s;
		s=s->NextSong();
	}

	return 0;
}

void Seq_Project::SaveAllSongs()
{	
}

int Seq_Main::strcmp_allsmall(char *s1,char *s2)
{
	char *su=GenerateString(s1),*su2=GenerateString(s2);

	if(su && su2)
	{
		// to tupp
		size_t i=strlen(su);
		for(size_t a=0;a<i;a++)
			su[a]=toupper(su[a]);

		i=strlen(su2);
		for(size_t a=0;a<i;a++)
			su2[a]=toupper(su2[a]);

		int rv=strcmp(su,su2);

		delete su;
		delete su2;

		return rv;
	}

	return 0;
}

void Seq_Main::InitQuantList(int ppqrate)
{
	// Init QuantList
	quantlist[0]=ppqrate*6;
	quantlist[1]=ppqrate*4;
	quantlist[2]=ppqrate*3;
	quantlist[3]=(int)((double)ppqrate*2.667);
	quantlist[4]=ppqrate*2;
	quantlist[5]=(int)((double)ppqrate*1.5);
	quantlist[6]=(int)((double)ppqrate*1.334);
	quantlist[7]=ppqrate;
	quantlist[8]=(int)((double)ppqrate*0.75);

	quantlist[9]=(int)((double)ppqrate*0.667);
	quantlist[10]=ppqrate/2;
	quantlist[11]=(int)((double)ppqrate*0.375);
	quantlist[12]=(int)((double)ppqrate*0.334);
	quantlist[13]=ppqrate/4;
	quantlist[14]=(int)((double)ppqrate*0.1875);
	quantlist[15]=(int)((double)ppqrate*0.1667);
	quantlist[16]=ppqrate/8;
	quantlist[17]=(int)((double)ppqrate/10.667);
	quantlist[18]=ppqrate/12;

	quantlist[19]=ppqrate/16;
	quantlist[20]=(int)((double)ppqrate/21.334);
	quantlist[21]=ppqrate/24;
	quantlist[22]=ppqrate/32;
	quantlist[23]=(int)((double)ppqrate/42.667);
	quantlist[24]=ppqrate/64;
	quantlist[25]=ppqrate/96;
	quantlist[26]=ppqrate/128;
	quantlist[27]=ppqrate/192;
	quantlist[28]=ppqrate/256;
	quantlist[29]=ppqrate/384;
}

void Seq_Main::CheckAutoSave()
{
	if(mainsettings->autosavemin!=0){
#ifdef WIN32
		int timenow=GetTickCount();
#endif	
		int checktime=lastautosaved;

		switch(mainsettings->autosavemin)
		{
		case 1:
			checktime+=(60*1000)*2; // 2min
			break;

		case 2:
			checktime+=(60*1000)*5; // 5min
			break;

		case 3:
			checktime+=(60*1000)*10; // 10min
			break;
		}

		if(checktime<=timenow)
		{
			ResetAutoSave();

			if(GetActiveSong())
			{
				GetActiveSong()->DoAutoSave();
				/*
				Seq_Project *p=FirstProject();

				while(p)
				{
				p->SaveAllSongs();

				p=p->NextProject();
				}
				*/
			}
		}
	}
}