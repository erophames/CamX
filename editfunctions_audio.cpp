#include "defines.h"
#include "gui.h"
#include "editor.h"
#include "undo.h"
#include "patternselection.h"
#include "semapores.h"
#include "object_project.h"
#include "object_song.h"
#include "objectpattern.h"
#include "songmain.h"
#include "mempools.h"
#include "audiohardware.h"
#include "MIDIhardware.h"
#include "audiofile.h"
#include "editfunctions.h"
#include "transporteditor.h"
#include "undofunctions.h"
#include "folder.h"
#include "player.h"
#include "editbuffer.h"
#include "arrangeeditor.h"
#include "undo_automation.h"
#include "undofunctions_pattern.h"
#include "audiohdfile.h"
#include "audiofilework.h"
#include "undofunctions_track.h"
#include "audiopattern.h"
#include "audiodevice.h"
#include "audiosend.h"

char *EditFunctions::InitNewAudioFile(char *filename,char *directoryname)
{
	// Same Sampling Rate
	char *newfile=mainvar->GenerateString(directoryname,"\\","Audio Imports","\\",filename);

	if(newfile)
	{
		camxFile copytest;

		if(copytest.OpenRead(newfile)==true)
		{
			copytest.Close(true); // Target File Exists

			delete newfile;
			newfile=0;

			// Check for free Number
			int nr=2;
			char h2[32];

			for(;;)
			{
				newfile=mainvar->GenerateString(directoryname,"\\","Audio Imports","\\",mainvar->ConvertIntToChar(nr,h2),filename);

				if(newfile)
				{
					if(copytest.OpenRead(newfile)==true)
					{
						nr++;
						copytest.Close(true); // Target File Exists
					}
					else
					{
						copytest.Close(true); // Target File Free
						break;
					}
				}

				if(newfile)
				{
					delete newfile;
					newfile=0;
				}

			}

			//if(newhdfile=mainaudio->AddAudioFile(newfile,false))
			//	newhdfile->Open();
		}
	}

	return newfile;
}

bool EditFunctions::InitNewAudioFile(AudioHDFile *newhdfile,char *directoryname,char **waitfornewfile,bool *doresample,AudioFileWork **afwork,AudioHDFile **usenewhdfile)
{
	// Same Sampling Rate
	if(newhdfile->camxrecorded==true)
	{
		newhdfile->Open();
	}
	else
		if(mainsettings->importaudiofiles==true)
		{
			char *newfile=mainvar->GenerateString(directoryname,"\\","Audio Imports","\\",newhdfile->GetFileName());

			if(newfile)
			{
				if(strcmp(newfile,newhdfile->GetName())==0)
				{
					// Select inside Audio Imports
					delete newfile;
					newhdfile->Open();
					return false;
				}

				camxFile copytest;
				if(copytest.OpenRead(newfile)==true)
				{
					copytest.Close(true); // Target File Exists

					if(char *h=mainvar->GenerateString(Cxs[CXS_FILEEXISTSINIMPORT],"\n",newhdfile->GetFileName()))
					{
						if(mainsettings->askimportaudiofiles==true && maingui->MessageBoxYesNo(0,h)==true)
						{
							if(newhdfile=mainaudio->AddAudioFile(newfile,false,true))
								newhdfile->Open();

							*usenewhdfile=newhdfile;
							delete newfile;
							delete h;

							return false;
						}

						delete h;
					}

					delete newfile;

					// Check for free Number
					int nr=2;
					char h2[32];

					for(;;)
					{
						newfile=mainvar->GenerateString(directoryname,"\\","Audio Imports","\\",mainvar->ConvertIntToChar(nr,h2),newhdfile->GetFileName());

						if(newfile)
						{
							if(copytest.OpenRead(newfile)==true)
							{
								nr++;
								copytest.Close(true); // Target File Exists
							}
							else
							{
								copytest.Close(true); // Target File Free
								break;
							}
						}
						else
							return false;
					}

					//if(newhdfile=mainaudio->AddAudioFile(newfile,false))
					//	newhdfile->Open();
				}

				if(AudioFileWork_CopyFile *work=new AudioFileWork_CopyFile){

					work->filename=mainvar->GenerateString(newhdfile->GetName());
					work->creatednewfile=newfile;
					work->camximport=true;

					*waitfornewfile=mainvar->GenerateString(newfile);
					*afwork=work;
					// audioworkthread->AddWork(work);
					*doresample=true;

					return true;
					//	newhdfile=0;
				}

			}
		}
		else
			newhdfile->Open();

	return false;
}

Undo_CreatePattern *EditFunctions::InitNewAudioPattern(guiWindow *win,Seq_Track *track,char *readfile,OSTART sfposition,bool lock,AudioPattern *clonefrom,AudioPattern *mainclonepattern,Undo_CreateTracks *addto)
{
	char *waitfornewfile=0;
	bool fileok=false,readfilestringcreated=false;
	AudioFileWork *afwork=0;

	if(!readfile) // Use FileRequester
	{
		camxFile soundfile;

		if(char *reqname=mainvar->GenerateString(Cxs[CXS_LOADWAVEFILE]," Track:",track->GetName()))
		{
			if(soundfile.OpenFileRequester(0,win,reqname,soundfile.AllFiles(camxFile::FT_WAVES_EX),true)==true)
			{
				if(readfile=mainvar->GenerateString(soundfile.filereqname))
					readfilestringcreated=true;
			}

			delete reqname;
		}
	}

	if(!readfile)return false;

	int csamplerate=0; // 0=decoder file

	TRACE ("Size HD %d\n",sizeof(AudioHDFile));

	if(mainaudio->CheckIfAudioFile(readfile,&csamplerate,true)==true)
	{
		bool ok=false,doresample=false;
		AudioHDFile *newhdfile=csamplerate==0?0:mainaudio->AddAudioFile(readfile,false);

		if(csamplerate!=mainaudio->GetGlobalSampleRate()) // search for previous resampled file
		{
			doresample=true;

			AudioFileWork tmp;

			tmp.filename=mainvar->GenerateString(readfile);

			char *tmph=mainaudio->GenerateSampleRateTMP();
			int samplerate;

			if(tmp.CreateTMP(tmph)==true && mainaudio->CheckIfAudioFile(tmp.creatednewfile,&samplerate)==true)
			{
				AudioHDFile *checkprevresample=mainaudio->AddAudioFile(tmp.creatednewfile,false);

				if(checkprevresample){
					newhdfile=checkprevresample;
					doresample=false;
				}		
			}

			if(tmph)
				delete tmph;	
		}

		if(newhdfile || doresample==true)
		{
			ok=true;

			if(newhdfile)
			{
				if(doresample==true)
				{
					char zz2[NUMBERSTRINGLEN],
						*h2=mainvar->ConvertIntToChar(newhdfile->samplerate,zz2),
						*to=mainvar->GenerateString(newhdfile->GetFileName(),"(",h2,") Hz"),
						*h=mainvar->GenerateString(Cxs[CXS_CONVERTSAMPLERATE],"?\n",to);

					if(h){
						doresample=ok=maingui->MessageBoxYesNo(win,h);

						if(ok==true){
							if(AudioFileWork_Resample *work=new AudioFileWork_Resample){
								work->Init(newhdfile->GetName()); // org file
								work->newsamplerate=mainaudio->GetGlobalSampleRate();
								afwork=work;
							}
						}

						delete h;
					}

					if(to)delete to;
				}
				else
				{
					AudioHDFile *usenewhdfile=0;

					if(InitNewAudioFile(newhdfile,track->song->directoryname,&waitfornewfile,&doresample,&afwork,&usenewhdfile)==true)
						newhdfile=0;

					if(usenewhdfile)
					{
						newhdfile=usenewhdfile;
					}
				}
			}
			else
			{
				// Init Dekoder
				if(AudioFileWork_Converter *con=new AudioFileWork_Converter){

					AudioFileWork tmp;

					tmp.filename=mainvar->GenerateString(readfile);

					char *tmph=mainaudio->GenerateSampleRateTMP();

					if(tmp.CreateTMP(tmph)==true)
					{
						if(mainsettings->importaudiofiles==true)
						{
							tmp.CreateFileNameTMP();

							con->outputfile=mainvar->GenerateString(track->song->directoryname,"\\","Audio Imports","\\",tmp.creatednewfile);
							waitfornewfile=mainvar->GenerateString(con->outputfile);
							con->camximport=true;
						}
						else
						{
							con->outputfile=mainvar->GenerateString(tmp.creatednewfile);
							waitfornewfile=mainvar->GenerateString(con->outputfile);
						}
					}

					if(tmph)delete tmph;

					con->inputfile=mainvar->GenerateString(readfile);
					con->newsamplerate=mainaudio->GetGlobalSampleRate();

					afwork=con;

					//audioworkthread->AddWork(con);
				}
			}

		}//if newhdfile

		Undo_CreatePattern *ufunction=0;

		if(ok==true)
		{
			if(UndoCreatePattern *ucp=new UndoCreatePattern)
			{
				TRACE ("Size UndoCreatePattern %d\n",sizeof(UndoCreatePattern));

				ucp->mediatype=MEDIATYPE_AUDIO;
				ucp->position=sfposition;
				ucp->track=track;
				ucp->mainclonepattern=mainclonepattern;

				if(ufunction=new Undo_CreatePattern(track->song,ucp,1))
				{
					TRACE ("Size Undo_CreatePattern %d\n",sizeof(Undo_CreatePattern));

					if(!addto)
					{
						if(lock==true)Lock(track->song);
					}

					ufunction->Do();

					if(AudioPattern *newaudio=(AudioPattern *)ufunction->createpattern->newpattern)
					{
						newaudio->audioevent.audioefile=doresample==false?newhdfile:0; // Full HD File

						if(clonefrom)
						{
							clonefrom->CloneFX(newaudio);

							// Region
							if(newaudio->audioevent.audioefile && clonefrom->audioevent.audioefile)
							{
								if(clonefrom->audioevent.audioregion)
								{
									newaudio->audioevent.audioregion=new AudioRegion;
									if(newaudio->audioevent.audioregion)
									{
										clonefrom->audioevent.audioregion->CloneTo(newaudio->audioevent.audioregion);
										newaudio->audioevent.audioregion->InitRegion();
									}
								}
							}

							clonefrom->offsetregion.CloneTo(&newaudio->offsetregion);

							newaudio->useoffsetregion=clonefrom->useoffsetregion;
							newaudio->offsetstartoffset=clonefrom->offsetstartoffset;

							newaudio->offsetregion.r_audiohdfile=newaudio->audioevent.audioefile;
							newaudio->offsetregion.InitRegion();
						}

						if(doresample==true)
						{
							newaudio->waitforresample=true;
							newaudio->waitforresamplefile=mainvar->GenerateString(readfile);

							if(waitfornewfile)
								newaudio->waitforresampleendfile=mainvar->GenerateString(waitfornewfile);
							else
							{
								AudioFileWork tmp;

								tmp.filename=mainvar->GenerateString(readfile);

								char *tmph=mainaudio->GenerateSampleRateTMP();

								if(tmp.CreateTMP(tmph)==true)
									newaudio->waitforresampleendfile=mainvar->GenerateString(tmp.creatednewfile);

								if(tmph)
									delete tmph;
							}
						}
						else
						{
							/*
							if(copytosongdir==true)
							{
							}
							else
							*/
							if(!addto)
								track->song->CheckCrossFades();
						}

						newaudio->SetName(readfile);
						fileok=true;

						newaudio->RefreshAfterPaste();
						newaudio->track->AddEffectsToPattern(newaudio); // New Track Effects
					}

					if(addto)
					{
						ufunction->FreeData();
						delete ufunction;
						ufunction=0;
					}
					else
						if(lock==true && (!addto))
						{
							CheckPlaybackAndUnLock();

							if(fileok==true)
							{
								track->song->undo.OpenUndoFunction(ufunction);
								CheckEditElementsForGUI(track->song,ufunction,true);

								// Refresh Manager
								//maingui->RefreshAllEditors(0,EDITORTYPE_AUDIOMANAGER,0);
							}
						}
				}
				else
				{
					delete ucp;
					ucp=0;
				}
			}
		}

		if(readfilestringcreated==true)
			delete readfile;

		if(waitfornewfile)
			delete waitfornewfile;

		if(afwork)
			audioworkthread->AddWork(afwork);

		return ufunction;
	}

	// Unknown File Typ
	if(readfilestringcreated==true)
		delete readfile;

	if(waitfornewfile)
		delete waitfornewfile;

	return false;
}

bool EditFunctions::LoadSoundFile(guiWindow *win,Seq_Track *track,char *readfile,OSTART position)
{
	if((!track) || CheckIfEditOK(track->song)==false)
		return false;

	Undo_CreatePattern *ucp=InitNewAudioPattern(win,track,readfile,position,true);

	return ucp?true:false;
}

bool EditFunctions::LoadSoundFileAndSplitNewTracks(guiWindow *win,Seq_Track *track,OSTART position)
{
	Seq_Song *song=win->WindowSong();

	if(CheckIfEditOK(song)==false)
		return false;

	char *readfile=0,*filename=0;
	camxFile soundfile;
	bool ok=false;

	if(char *reqname=mainvar->GenerateString(Cxs[CXS_LOADAUDIOFILEANDSPLITTOCHANNELS]," ->",song->GetName()))
	{
		if(soundfile.OpenFileRequester(0,win,reqname,soundfile.AllFiles(camxFile::FT_WAVES_EX),true)==true)
		{
			readfile=mainvar->GenerateString(soundfile.filereqname);
			filename=mainvar->GenerateString(soundfile.fname);
		}

		delete reqname;
	}

	AudioFileInfo info;

	if(mainaudio->CheckIfAudioFileInfo(readfile,&info)==true)
	{
		int index=0;

		if(!track)
			track=song->GetFocusTrack();

		if(track)
		{
			index=song->GetOfTrack(track)+1;
		}

		if(info.channels>0)
		{
			CreateNewTrackAudio newaudio;

			newaudio.position=position;
			newaudio.list=0;
			newaudio.createtracklist=true;

			CreateNewTracks(song,track,info.channels,0,track,&newaudio);

			if(newaudio.tracks)
			{
				ok=true;

				char help[NUMBERSTRINGLEN];

				if(AudioFileWork_SplitFileInChannels *split=new AudioFileWork_SplitFileInChannels)
				{
					if(split){

						split->song=song;
						split->position=position;

						split->newfiles=new char *[newaudio.count];
						split->samplebits=info.bits;
						split->samplerate=info.samplerate;

						split->count=newaudio.count;

						char *cs=mainvar->ClearString(readfile);

						int audioport=0;

						// Create Pattern
						for(int i=0;i<newaudio.count;i++)
						{
							char *ntn=mainvar->GenerateString(cs,"_",mainvar->ConvertIntToChar(i+1,help));

							if(ntn)
							{
								newaudio.tracks[i]->SetVType(song->audiosystem.device,CT_MONO,false,false); // Set Mono

								if(song->audiosystem.device)
									newaudio.tracks[i]->io.SetOutput(&song->audiosystem.device->outputaudioports[newaudio.tracks[i]->io.channel_type][audioport]);

								if(audioport==CHANNELSPERPORT-1)
									audioport=0;
								else
									audioport++;

								newaudio.tracks[i]->SetName(ntn);

								delete ntn;
							}

							char *h=mainvar->GenerateString(newaudio.tracks[i]->GetName(),".wav");

							if(h)
							{
								char *nf=InitNewAudioFile(h,song->directoryname);

								if(split->newfiles)
									split->newfiles[i]=nf;

								delete h;
							}

						}

						if(cs)
							delete cs;

						split->tracks=newaudio.tracks;
						newaudio.tracks=0;

						split->Init(readfile); // org file

						audioworkthread->AddWork(split);
					}
				}
			}
		}
	}

	return ok;
}

bool EditFunctions::LoadSoundFileNewTracks(guiWindow *win,Seq_Track *track,OSTART position)
{
	Seq_Song *song=win->WindowSong();

	if(CheckIfEditOK(song)==false)
		return false;

	char *readfile=0,*filename=0;
	camxFile soundfile;

	bool ok=false;

	if(char *reqname=mainvar->GenerateString(Cxs[CXS_LOADWAVEFILE]," ->",song->GetName()))
	{
		if(soundfile.OpenFileRequester(0,win,reqname,soundfile.AllFiles(camxFile::FT_WAVES_EX),true)==true)
		{
			readfile=mainvar->GenerateString(soundfile.filereqname);
			filename=mainvar->GenerateString(soundfile.fname);
		}

		delete reqname;
	}

	AudioFileInfo audiofileinfo;

	if(mainaudio->CheckIfAudioFile(readfile,0,false,&audiofileinfo)==true)
	{
		int index=0;

		if(!track)
			track=song->GetFocusTrack();

		if(track)
		{
			index=song->GetOfTrack(track)+1;
		}

		if(Seq_Track *newtrack=new Seq_Track)
		{
			newtrack->song=song;
			newtrack->SetName(filename);
			newtrack->InitAudioIO(song,&audiofileinfo);

			if(Seq_Track **tlist=new Seq_Track*[1])
			{
				tlist[0]=newtrack;

				if(Undo_CreateTracks *nt=new Undo_CreateTracks(song,tlist,1,index,track,track,0))
				{
					nt->activate=true;

					InitNewAudioPattern(win,newtrack,readfile,position,false,0,0,nt); // Create Pattern
					song->undo.OpenUndoFunction(nt);
					LockAndDoFunction(song,nt,true);
				}
			}
		}

		ok=true;
	}

	if(filename)delete filename;
	if(readfile)delete readfile;

	return ok;
}

bool EditFunctions::LoadSoundFileDirectoryNewTracks(guiWindow *win,Seq_Track *track,OSTART position)
{
	Seq_Song *song=win->WindowSong();

	if(CheckIfEditOK(song)==false)
		return false;

	AudioFileWork_FinderList finder;
	camxFile soundfile;
	bool ok=false;

	if(char *reqname=mainvar->GenerateString(Cxs[CXS_LOADDIRECTORY]," ->",song->GetName()))
	{
		if(soundfile.SelectDirectory(win,0,reqname)==true)
		{
			finder.filename=mainvar->GenerateString(soundfile.filereqname);
		}

		delete reqname;
	}
	else
		return false;

	finder.nogui=true;
	finder.Start();

	if(finder.list.GetCount())
	{
		int index=0;

		if(!track)
			track=song->GetFocusTrack();

		if(track)
		{
			index=song->GetOfTrack(track)+1;
		}

		CreateNewTrackAudio newaudio;

		newaudio.position=position;
		newaudio.list=&finder.list;

		CreateNewTracks(song,track,finder.list.GetCount(),0,track,&newaudio);

		ok=true;
	}

	return ok;
}

Undo_SetAudioIO::Undo_SetAudioIO(Seq_Track *t,bool sel)
{
	id=Undo::UID_SETAUDIOIO;

	t->GetAudioOut()->CloneToGroup(&channelgroup);

	selected=sel;
	oldchannels=0;
	nrtracks=0;
	usedirecttodevice=t->usedirecttodevice;
}

void Undo_SetAudioIO::DoUndo()
{
	if(oldchannels)
	{
		for(int i=0;i<nrtracks;i++)
		{
			oldchannels[i].CloneToGroup(oldchannels[i].track->GetAudioOut());
		}
	}
}

void Undo_SetAudioIO::Do()
{
	if(!oldchannels)
	{
		Seq_Track *t=song->FirstTrack();

		while(t)
		{
			if((!t->parent) && 
				((t->flag&OFLAG_SELECTED) || selected==false) &&
				(channelgroup.CompareWithGroup(t->GetAudioOut())==false || t->usedirecttodevice!=usedirecttodevice)
				)
				nrtracks++;

			t=t->NextTrack();
		}
	}

	if(nrtracks)
	{
		if(!oldchannels)
		{
			if(oldchannels=new Seq_AudioIO[nrtracks])
			{
				// Buffer old Track MIDI Devices
				int i=0;
				Seq_Track *t=song->FirstTrack();

				while(t){

					if((!t->parent) && 
						((t->flag&OFLAG_SELECTED) || selected==false) &&
						(channelgroup.CompareWithGroup(t->GetAudioOut())==false || t->usedirecttodevice!=usedirecttodevice)
						)
					{
						oldchannels[i].track=t;
						t->GetAudioOut()->CloneToGroup(&oldchannels[i++]);
					}

					t=t->NextTrack();
				}
			}
		}
	}

	if(oldchannels)
	{
		for(int i=0;i<nrtracks;i++)
		{
			channelgroup.CloneToGroup(oldchannels[i].track->GetAudioOut());
			oldchannels[i].track->usedirecttodevice=usedirecttodevice;
		}
	}

}

void Undo_SetAudioIO::FreeData()
{
	if(oldchannels)
	{
		for(int i=0;i<nrtracks;i++)
			oldchannels[i].Delete();

		delete oldchannels;
	}

	channelgroup.Delete();
}

void Undo_SetAudioIO::AddedToUndo()
{
	CreateUndoString(selected==true?Cxs[CXS_CHANGEALLTRACKSAUDIOIO_SEL]:Cxs[CXS_CHANGEALLTRACKSAUDIOIO]);
}

void EditFunctions::SetSongTracksToAudioIO(Seq_Track *track,bool selected)
{
	if(track)
	{
		Seq_Track *t=track->song->FirstTrack();

		while(t)
		{
			if((!t->parent) && 
				(t!=track) &&
				((t->flag&OFLAG_SELECTED) || selected==false) &&
				(track->GetAudioOut()->CompareWithGroup(t->GetAudioOut())==false || 
				track->usedirecttodevice!=t->usedirecttodevice /*|| 
															   track->io.Compare(&t->io)==false*/)
															   )
			{
				if(Undo_SetAudioIO *usm=new Undo_SetAudioIO(track,selected))
				{
					track->song->undo.OpenUndoFunction(usm);
					usm->Do();
					CheckEditElementsForGUI(track->song,usm,true);
				}

				break;
			}

			t=t->NextTrack();
		}

		if(!t)
			maingui->MessageBoxOk(0,Cxs[CXS_NOCHANGES]);
	}
}

int EditFunctions::RemoveAudioRegionFromProjects(AudioRegion *r)
{
	r->destructive=true; // Don't show in GUI

	maingui->StopRegionInGUI(r); // Stop Playback

	int deletecounter=0;

	// Remove From GUI
	{
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

						if(ap->audioevent.audioregion==r) // Region Pattern ?
						{
							ap->visible=false;
							maingui->RemovePatternFromGUI(s,p);
							deletecounter++;
						}

						p=p->NextPattern(MEDIATYPE_AUDIO);
					}

					t=t->NextTrack();
				}

				s=s->NextSong();
			}

			p=p->NextProject();
		}
	}

	if(deletecounter)
	{
		mainthreadcontrol->LockActiveSong();
		Seq_Project *p=mainvar->FirstProject();

		while(p)
		{
			int insongdeleted=0;
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
						//Seq_Pattern *np=p->NextPattern(MEDIATYPE_AUDIO);

						if(ap->audioevent.audioregion==r)
						{
							p=t->DeletePattern(p,true);
							insongdeleted++;
						}
						else 
							p=p->NextPattern(MEDIATYPE_AUDIO);
					}

					t=t->NextTrack();
				}

				if(insongdeleted && s==mainvar->GetActiveSong())
					s->CheckPlaybackRefresh();

				s=s->NextSong();
			}

			p=p->NextProject();
		}

		mainthreadcontrol->UnlockActiveSong();
	}

	return deletecounter;
}

bool EditFunctions::ChangeRegion(AudioRegion *r,LONGLONG start,LONGLONG end)
{
	bool changed=false;

	if(r)
	{
		int used=r->GetUsedCounter();

		if(used)
			mainthreadcontrol->LockActiveSong();

		changed=r->ChangeRegionPosition(start,end);

		if(used && mainvar->GetActiveSong())
			mainvar->GetActiveSong()->CheckPlaybackRefresh();

		if(used)
			mainthreadcontrol->UnlockActiveSong();

		if(changed==true)
			maingui->RefreshAudioHDFile(r->r_audiohdfile,r);	
	}

	return false;
}

void EditFunctions::ReplaceAudioPatternHDFile(AudioPattern *p,AudioHDFile *newfile,AudioRegion *region)
{
	if(p && newfile)
	{
		Seq_Song *song=p->track->song;

		Undo_ReplaceAudioPatternFile *ap=new Undo_ReplaceAudioPatternFile(p,newfile,region);

		if(ap)
		{
			Lock(song);
			ap->Do();
			CheckPlaybackAndUnLock();
			song->CheckCrossFades();

			ap->RefreshGUI(false);
			song->undo.OpenUndoFunction(ap);
			CheckEditElementsForGUI(p->track->song,ap,true);
		}
	}
}

void Undo_CreateBus::FreeData()
{
	if(inundo==false)
	{
		if(channels)
		{
			for(int i=0;i<number;i++)
				channels[i]->Delete(true);
		}

		if(ucb)
		{
			for(int i=0;i<ucbnr;i++)
				delete ucb[i].send;

			delete ucb;
		}

		if(pdc)
			delete pdc;
	}

	if(channels)
		delete channels;
}

void Undo_CreateBus::Do()
{
	if(created==false)
	{
		AudioChannel *prev=prevchannel;

		for(int i=0;i<number;i++)
		{
			channels[i]=song->audiosystem.CreateBus(prev,outport);
			prev=channels[i];
		}

		created=true;
	}
	else
	{
		AudioChannel *prev=prevchannel;

		for(int i=0;i<number;i++)
		{
			song->audiosystem.AddBus(prev,channels[i]);
			prev=channels[i];
		}

		if(ucb)
		{
			for(int i=0;i<ucbnr;i++)
			{
				ucb[i].track->io.AddSend(ucb[i].send,ucb[i].index);
				ucb[i].track->io.sends.Close();
			}

			delete ucb;
			ucb=0;
		}

		if(pdc)
		{
			for(int i=0;i<pdcnr;i++)
			{
				pdc[i].track->GetAudioOut()->AddToGroup(pdc[i].bus,pdc[i].index);
				pdc[i].track->usedirecttodevice=pdc[i].usedirecttodevice;
			}

			delete pdc;
			pdc=0;
		}
	}
}

void Undo_CreateBus::DoUndo()
{
	// Save Sends 
	int dc=0;
	int sc=0;

	{
		Seq_Track *t=song->FirstTrack();

		while(t)
		{
			AudioSend *s=t->io.FirstSend();

			while(s)
			{
				for(int i=0;i<number;i++)
				{
					if(s->sendchannel==channels[i])
						sc++;
				}

				s=s->NextSend();
			}

			// Direct Connection
			for(int i=0;i<number;i++)
			{
				Seq_AudioIOPointer *aio=t->GetAudioOut()->FirstChannel();
				while(aio)
				{
					if(aio->channel==channels[i])
						dc++;

					aio=aio->NextChannel();
				}
			}

			t=t->NextTrack();
		}
	}

	if(sc)
	{
		int i=0;

		if(ucb=new undocreatebussends[sc])
		{
			ucbnr=sc;

			Seq_Track *t=song->FirstTrack();
			while(t)
			{
				AudioSend *s=t->io.FirstSend();

				while(s)
				{
					for(int i=0;i<number;i++)
					{
						if(s->sendchannel==channels[i])
						{
							ucb[i].track=t;
							ucb[i].send=s;
							ucb[i].index=s->GetIndex();
						}
					}

					s=s->NextSend();
				}

				t=t->NextTrack();
			}
		}
	}

	if(dc)
	{
		// Buffer direct Track->Buffer  
		int ix=0;

		if(pdc=new undotrackdirectbus[pdcnr=dc])
		{
			Seq_Track *t=song->FirstTrack();

			while(t)
			{
				for(int i=0;i<number;i++)
				{
					Seq_AudioIOPointer *aio=t->GetAudioOut()->FirstChannel();
					while(aio)
					{
						if(aio->channel==channels[i])
						{
							pdc[ix].track=t;
							pdc[ix].usedirecttodevice=t->usedirecttodevice;

							pdc[ix].bus=aio->channel;
							pdc[ix].index=aio->GetIndex();

							ix++;
						}

						aio=aio->NextChannel();
					}
				}

				t=t->NextTrack();
			}
		}
	}

	for(int i=0;i<number;i++)
		song->audiosystem.DeleteAudioChannel(channels[i],false);
}

void Undo_CreateBus::RefreshPostUndo()
{
	for(int i=0;i<number;i++)
		channels[i]->RefreshDo();
}

void Undo_CreateBus::RefreshPreRedo()
{
	for(int i=0;i<number;i++)
		channels[i]->PreRefreshDo();
}

int EditFunctions::CreateNewBusChannels(Seq_Song *song,AudioChannel *prev,int number,Seq_Track *clone)
{
	if(number>0 && number<500 && CheckIfEditOK(song)==true)
	{
		AudioPort *out=clone?clone->io.out_vchannel:0;

		if(AudioChannel **channels=new AudioChannel*[number])
		{
			if(Undo_CreateBus *uct=new Undo_CreateBus(song,channels,number,out,prev))
				OpenLockAndDo(song,uct,true);
		}
	}

	return 0;
}

void Undo_DeleteBus::RefreshGUI(bool undorefresh)
{
	for(int i=0;i<number;i++)
		maingui->RemoveAudioChannelFromGUI(song,channels[i]);

	mainedit->CheckEditElementsForGUI(song,this,undorefresh);
}

void Undo_DeleteBus::FreeData()
{
	if(inundo==true)
	{
		if(channels)
		{
			for(int i=0;i<number;i++)
				channels[i]->Delete(true);
		}

		if(ucb)
		{
			for(int i=0;i<ucbnr;i++)
				delete ucb[i].send;

			delete ucb;
		}

		if(pdc)
			delete pdc;
	}

	if(channels)
		delete channels;

	if(index)
		delete index;
}

void Undo_DeleteBus::Do()
{
	// Save Sends
	int sc=0,dc=0;

	{
		Seq_Track *t=song->FirstTrack();

		while(t)
		{
			AudioSend *s=t->io.FirstSend();

			while(s)
			{
				for(int i=0;i<number;i++)
				{
					if(s->sendchannel==channels[i])
						sc++;
				}

				s=s->NextSend();
			}

			// Direct Connection
			for(int i=0;i<number;i++)
			{
				Seq_AudioIOPointer *aio=t->GetAudioOut()->FirstChannel();
				while(aio)
				{
					if(aio->channel==channels[i])
						dc++;

					aio=aio->NextChannel();
				}
			}

			t=t->NextTrack();
		}
	}

	if(sc)
	{
		int i=0;

		if(ucb=new undocreatebussends[ucbnr=sc])
		{

			Seq_Track *t=song->FirstTrack();
			while(t)
			{
				AudioSend *s=t->io.FirstSend();

				while(s)
				{
					for(int i=0;i<number;i++)
					{
						if(s->sendchannel==channels[i])
						{
							ucb[i].track=t;
							ucb[i].send=s;
							ucb[i].index=s->GetIndex();

#ifdef DEBUG
							if(ucb[i].index<0 || ucb[i].index>500)
								maingui->MessageBoxError(0,"Bus Index Error!");
#endif

						}
					}

					s=s->NextSend();
				}

				t=t->NextTrack();
			}
		}
	}

	if(dc)
	{
		int ix=0;

		if(pdc=new undotrackdirectbus[pdcnr=dc])
		{
			Seq_Track *t=song->FirstTrack();

			while(t)
			{
				for(int i=0;i<number;i++)
				{
					Seq_AudioIOPointer *aio=t->GetAudioOut()->FirstChannel();

					while(aio)
					{
						if(aio->channel==channels[i])
						{
							pdc[ix].track=t;
							pdc[ix].usedirecttodevice=t->usedirecttodevice;

							pdc[ix].bus=aio->channel;
							pdc[ix].index=aio->GetIndex();

#ifdef DEBUG
							if(pdc[ix].index<0 || pdc[ix].index>500)
								maingui->MessageBoxError(0,"Bus Index Error!");
#endif

							ix++;
						}

						aio=aio->NextChannel();
					}
				}

				t=t->NextTrack();
			}
		}
	}

	for(int i=0;i<number;i++)
		song->audiosystem.DeleteAudioChannel(channels[i],false);
}

void Undo_DeleteBus::DoUndo()
{
	for(int i=0;i<number;i++)
	{
#ifdef DEBUG
		if(index[i]<0 || index[i]>500)
			maingui->MessageBoxError(0,"Bus Index ErrorHHH!");
#endif

		song->audiosystem.AddBus(channels[i],index[i]);
	}

	if(ucb)
	{
		for(int i=0;i<ucbnr;i++)
		{

#ifdef DEBUG
			if(ucb[i].index<0 || ucb[i].index>500)
				maingui->MessageBoxError(0,"Add Send Index Error!");
#endif

			ucb[i].track->io.AddSend(ucb[i].send,ucb[i].index);



			ucb[i].track->io.sends.Close();
		}

		delete ucb;
		ucb=0;
	}

	if(pdc)
	{
		for(int i=0;i<pdcnr;i++)
		{
			pdc[i].track->GetAudioOut()->AddToGroup(pdc[i].bus,pdc[i].index);
			pdc[i].track->usedirecttodevice=pdc[i].usedirecttodevice;
		}

		delete pdc;
		pdc=0;
	}

}

void Undo_DeleteBus::RefreshPostUndo()
{
	for(int i=0;i<number;i++)
		channels[i]->PreRefreshDo();
}

void Undo_DeleteBus::RefreshPreRedo()
{
	for(int i=0;i<number;i++)
		channels[i]->RefreshDo();
}

void EditFunctions::DeleteBusChannels(Seq_Song *song)
{
	if(CheckIfEditOK(song)==true)
	{
		int co=0;

		{
			AudioChannel *c=song->audiosystem.FirstBusChannel();
			while(c)
			{
				if(c->IsSelected()==true)
					co++;

				c=c->NextChannel();
			}
		}

		if(co)
		{
			if(int *index=new int[co])
			{
				if(AudioChannel **channels=new AudioChannel*[co])
				{
					int ix=0;

					AudioChannel *c=song->audiosystem.FirstBusChannel();
					while(c)
					{
						if(c->IsSelected()==true)
						{
							channels[ix]=c;
							index[ix]=c->GetIndex();

							ix++;
						}

						c=c->NextChannel();
					}

					if(Undo_DeleteBus *db=new Undo_DeleteBus(song,channels,co,index))
						OpenLockAndDo(song,db,true);
				}
			}
		}
	}
}