#include "defines.h"
#include <stdio.h>
#include <string.h>
#include "songmain.h"
#include "object_project.h"
#include "object_song.h"
#include "object_track.h"
#include "audiofile.h"
#include "audiohardware.h"
#include "gui.h"
#include "semapores.h"
#include "audiothread.h"
#include "arrangeeditor.h"
#include "MIDIoutproc.h"
#include "editsettings.h"

#ifdef WIN32
#include "asio/asio.h"
#endif

#include "audiopeakbuffer.h"
#include "vstplugins.h"
#include "audioauto_volume.h"
#include "sampleeditor.h"
#include "audiomanager.h"
#include "editor_event.h"
#include "editfunctions.h"
#include "audiopattern.h"
#include "audiofilework.h"

void mainAudio::CheckWorkedFiles()
{
	AudioHDFile *existinghdfilefound=0;
	bool fileusedinactivesong=false,
		regionsdeleted=false,
		refreshfinder=false,
		fileused;

	mainthreadcontrol->Lock(CS_audiowork);
	AudioWorkedFile *f=audioworkthread->FirstWorkedFile();
	mainthreadcontrol->Unlock(CS_audiowork);

	while(f)
	{
		if(f->type==AudioWorkedFile::AUDIOWORKED_TYPE_FINDER)
		{
			refreshfinder=true;
		}
		else
		{
			AudioHDFile *newhdfile=0;

			switch(f->type)
			{
			case AudioWorkedFile::AUDIOWORKED_TYPE_SPLITTED:
				{
					AudioFileWork_SplitFileInChannels *split=(AudioFileWork_SplitFileInChannels *)f->fromfilework;

					split->EndWork();
					goto nextloop;
				}
				break;

			case AudioWorkedFile::AUDIOWORKED_TYPE_COPYIED:
			case AudioWorkedFile::AUDIOWORKED_TYPE_CONVERTED:
				{
					newhdfile=mainaudio->AddAudioFile(f->createnewfile,false,f->camximport);
					if(newhdfile) // Add Regions
						f->regions.MoveListToList(&newhdfile->regions);
				}
				break;
			}

			// HD File -> new File
			if(!newhdfile)
			{
				AudioHDFile *hdfile=FirstAudioHDFile();

				while(hdfile){

					if(strcmp(hdfile->GetName(),f->filename)==0){
						existinghdfilefound=hdfile;
						break;
					}

					hdfile=hdfile->NextHDFile();
				}
			}

			//if(existinghdfilefound && existinghdfilefound->GetOpenCounter()>0)
			if(f->createnewfile)
			{
				// Create Clone
				if(!newhdfile)
				{
					newhdfile=AddAudioFile(f->createnewfile,false);

					if(newhdfile) // Add Regions
						f->regions.MoveListToList(&newhdfile->regions);
				}

				/*
				if(newhdfile)
					newhdfile->Open();
*/

				fileused=false;

				// Find File in Song
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
								Seq_Pattern *pp=t->FirstPattern(MEDIATYPE_AUDIO);

								while(pp)
								{
									AudioPattern *ap=(AudioPattern *)pp;

									if(ap->waitforresample==true && 
										ap->waitforresampleendfile &&
										f->createnewfile && 
										strcmp(ap->waitforresampleendfile,f->createnewfile)==0)
									{					
										fileused=true;
										if(s==mainvar->GetActiveSong())fileusedinactivesong=true;
									}
									else
										if(existinghdfilefound && ap->audioevent.audioefile==existinghdfilefound) // Replace Pattern->new hdfile	
										{
											fileused=true;
											if(s==mainvar->GetActiveSong())fileusedinactivesong=true;
										}

										pp=pp->NextPattern(MEDIATYPE_AUDIO);
								}

								t=t->NextTrack();
							}
						}

						s=s->NextSong();
					}

					p=p->NextProject();
				}
			}

			// Check and Refresh GUI
			if(fileused==true && newhdfile)
			{
				//newhdfile->Open();

				//Find Regions+Delete
				if(existinghdfilefound && existinghdfilefound->FirstRegion())
				{
					// Regions affected ?
					switch(f->type)
					{
					case AudioWork::AWORK_CUT:
						{
							AudioRegion *r=existinghdfilefound->FirstRegion();

							while(r)
							{
								if(r->CheckIfInRegion(f->fromsample,f->tosample)==true)
								{
									AudioRegion *n=r->NextRegion();

									mainedit->RemoveAudioRegionFromProjects(r);
									existinghdfilefound->DeleteRegion(r);
									maingui->RemoveAudioRegionFromGUI(existinghdfilefound,r); // r=dead pointer !

									r=n;
								}
								else
								{
									if(AudioRegion *nr=new AudioRegion(newhdfile))
									{
										r->CloneTo(nr);

										// Offset
										if(f->fromsample<r->regionstart)
										{
											LONGLONG cut=f->tosample-f->fromsample;

											nr->regionstart-=cut;
											nr->regionend-=cut;

											nr->InitRegion();
										}

										nr->clonedfrom=r;

										newhdfile->AddRegion(nr,true);
									}

									r=r->NextRegion();
								}
							}
						}
						break;
					}
				}// REgions

				if(fileusedinactivesong==true)
					mainthreadcontrol->LockActiveSong();

				// Find File in Song
				Seq_Project *p=mainvar->FirstProject();

				while(p){
					bool checkcrossfade=false;
					Seq_Song *s=p->FirstSong();

					while(s){

						if(s->loaded==true)
						{
							bool checkplayback=false;
							Seq_Track *t=s->FirstTrack();

							while(t)
							{
								Seq_Pattern *pp=t->FirstPattern(MEDIATYPE_AUDIO);

								while(pp)
								{
									AudioPattern *ap=(AudioPattern *)pp;

									if(ap->waitforresample==true && 
										ap->waitforresampleendfile && 
										f->createnewfile && 
										strcmp(ap->waitforresampleendfile,f->createnewfile)==0)
									{	
										AudioRegion *newregion=0;

										if(ap->waitforregion)
										{
											newregion=newhdfile->FindRegion(ap->waitforregion);
											ap->waitforregion=0;
										}

										checkplayback=true;
										checkcrossfade=t->checkcrossfade=true;

										if(ap->waitforresamplefile)delete ap->waitforresamplefile;
										ap->waitforresamplefile=0;

										if(ap->waitforresampleendfile)delete ap->waitforresampleendfile;
										ap->waitforresampleendfile=0;

										ap->ReplaceAudioHDFileWith(newhdfile,newregion,REPLACEAUDIOFILE_CHECKFORREGION);
										ap->waitforresample=false;
									}
									else
										if(existinghdfilefound && ap->audioevent.audioefile==existinghdfilefound) // Replace Pattern->new hdfile	
										{
											ap->ReplaceAudioHDFileWith(newhdfile,0,REPLACEAUDIOFILE_CHECKFORREGION);
											checkplayback=true;
										}

										pp=pp->NextPattern(MEDIATYPE_AUDIO);
								}

								t=t->NextTrack();
							}

							if(checkcrossfade==true)
								s->CheckCrossFades();

							if(checkplayback==true && s==mainvar->GetActiveSong())
								s->CheckPlaybackRefresh();
						}

						s=s->NextSong();
					}

					p=p->NextProject();
				}

				if(fileusedinactivesong==true)
					mainthreadcontrol->UnlockActiveSong();
			}

			{
				guiWindow *w=maingui->FirstWindow();

				while(w)
				{
					if(w->WindowSong())
					{
						switch(w->GetEditorID())
						{
						case EDITORTYPE_EVENT:
							{
								bool found=false;
								Edit_Event *ev=(Edit_Event *)w;
								Edit_Event_Event *evt=ev->FirstEvent();

								while(evt && found==false)
								{
									switch(evt->seqevent->GetStatus())
									{
									case AUDIO:
										{
											AudioEvent *audio=(AudioEvent *)evt->seqevent;

											if(audio->audioefile==newhdfile)
											{
												ev->ShowEventsHoriz(SHOWEVENTS_EVENTS);
												found=true;
											}
										}
										break;
									}

									evt=evt->NextEvent();
								}
							}
							break;

						case EDITORTYPE_ARRANGE:
							{
								Edit_Arrange *ar=(Edit_Arrange *)w;
								ar->ShowHoriz(true,false,true);
							}
							break;

						case EDITORTYPE_SAMPLE:
							{
								Edit_Sample *es=(Edit_Sample *)w;
								if(es->audiohdfile==existinghdfilefound)es->ChangeHDFile(newhdfile);
							}
							break;

							/*
							case EDITORTYPE_AUDIOMANAGER:
							{
							Edit_Manager *em=(Edit_Manager *)w;

							if(em->GetActiveHDFile()==existinghdfilefound)
							em->ChangeSortFile(em->FindAudioHDFile(newhdfile));

							em->InitList();
							em->ShowAudioFiles();
							}
							break;
							*/
						}
					}

					w=w->NextWindow();
				}
			} // while w

nextloop:
			if(newhdfile)
				RefreshAudioFileGUI(newhdfile);
		}

		mainthreadcontrol->Lock(CS_audiowork);
		f=audioworkthread->DeleteWorkedFile(f);
		mainthreadcontrol->Unlock(CS_audiowork);
	}

	if(refreshfinder==true)
	{
		//maingui->RefreshAllEditors(0,EDITORTYPE_AUDIOMANAGER);

		guiWindow *w=maingui->FirstWindow();
		while(w)
		{
			if(w->GetEditorID()==EDITORTYPE_SETTINGS)
				((Edit_Settings *)w)->RefreshAudioDirectories();

			w=w->NextWindow();
		}

		//mainaudio->SaveDataBase();
	}

}