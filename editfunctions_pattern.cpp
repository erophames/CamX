#include "defines.h"

#include "gui.h"
#include "editor.h"
#include "undo.h"
#include "patternselection.h"
#include "semapores.h"
#include "object_song.h"
#include "objectpattern.h"
#include "songmain.h"
#include "audiohardware.h"
#include "MIDIhardware.h"
#include "audiofile.h"
#include "editfunctions.h"
#include "transporteditor.h"
#include "undofunctions.h"
#include "undofunctions_pattern.h"
#include "folder.h"
#include "editbuffer.h"
#include "MIDIoutproc.h"
#include "arrangeeditor.h"
#include "drumevent.h"
#include "audiohdfile.h"
#include "MIDIfile.h"
#include "audiofilework.h"
#include "audiopattern.h"

// GUI Function

bool Undo_DeletePattern::RefreshUndo()
{
	for(int i=0;i<patternnumber;i++)
	{
		if(inundo==true)
		{
			if(CheckPattern(pattern[i].pattern)==false)
				return false;
		}
		else
		{
			if(song->FindPattern(pattern[i].pattern)==false)
				return false;
		}
	}

	return true;
}

void Undo_DeletePattern::DoUndo()
{
	UndODeInitPattern *pp=pattern;
	int c=patternnumber;

	while(c--)
	{
		pp->pattern->visible=true;

		if(pp->pattern->mainclonepattern)
			pp->pattern->mainclonepattern->AddClone(pp->pattern);

		pp->track->AddSortPattern(pp->pattern,pp->pattern->ostart);

		pp->track->checkcrossfade=true;

		pp->pattern->RefreshAfterPaste();

		// Special Buffer
		switch(pp->pattern->mediatype)
		{
		case MEDIATYPE_AUDIO:
			{
				AudioPattern *ap=(AudioPattern *)pp->pattern;

				//if(ap->volumecurve.automationtrack)
				//	pp->pattern->track->AddAutomationTrack(ap->volumecurve.automationtrack,0,ADDSUBTRACK_TOP);
			}
			break;
		}

		// Clones
		Seq_ClonePattern *cl=pp->pattern->FirstClone();
		while(cl)
		{
			cl->pattern->visible=true;
			cl->pattern->track->AddSortPattern(cl->pattern,cl->pattern->ostart);
			cl->insideundo=false;
			cl->pattern->RefreshAfterPaste();

			cl=cl->NextClone();
		}

		pp++;
	}

	//AddSubUndoFunctions();
}

void Undo_DeletePattern::FreeData()
{
	if(inundo==true)
	{
		if(pattern)
		{
			for(int i=0;i<patternnumber;i++)
			{
				if(pattern[i].pattern)
				{
					switch(pattern[i].pattern->mediatype)
					{
					case MEDIATYPE_AUDIO:
						{
							/*
							AudioPattern *ap=(AudioPattern *)pattern[i].pattern;

							if(ap->volumecurve.automationtrack)
							{
								ap->volumecurve.automationtrack->DeleteObjects();
								delete ap->volumecurve.automationtrack;
								ap->volumecurve.automationtrack=0;
							}
							*/
						}
						break;
					}

					pattern[i].pattern->DeleteAllCrossFades(true,false); // Avoid Pattern Access
					pattern[i].pattern->Delete(true);
				}
			}
		}
	}

	if(pattern)delete pattern;
	pattern=0;
}

void Undo_DeletePattern::RefreshGUI(bool undorefresh)
{
	for(int i=0;i<patternnumber;i++)
		maingui->RemovePatternFromGUI(song,pattern[i].pattern,false);

	mainedit->CheckEditElementsForGUI(song,this,undorefresh);
}

void Undo_DeletePattern::Do()
{
	UndODeInitPattern *pp=pattern;
	int c=patternnumber;

	while(c--)
	{
		pp->pattern->StopAllofPattern();

		/*
		// Backup special
		switch(pp->pattern->mediatype)
		{
		case MEDIATYPE_AUDIO:
			{
				AudioPattern *ap=(AudioPattern *)pp->pattern;

				if(ap->volumecurve.automationtrack) // Volume Subtrack ?
				{
					pp->track->DeleteAutomationTrack(ap->volumecurve.automationtrack,false);
				}
			}
			break;
		}
*/

		pp->pattern->visible=false;
		pp->track->DeletePattern(pp->pattern,false);
		pp->track->checkcrossfade=true;
		pp++;
	}

	// More Delete Patterns
	UndoFunction *uf=FirstFunction();
	while(uf){
		uf->Do();
		uf=uf->NextFunction();
	}
}

Undo_DeletePattern *EditFunctions::DeletePattern(Seq_Song *song,Seq_SelectionList *list,Seq_Pattern *pattern,bool addtolastundo)
{
	if(CheckIfEditOK(song)==true && ((list && list->FirstSelectedPattern()) || pattern))
	{
		int c=0;
		UndODeInitPattern *patternpointer=0;

		if(list)
		{
			if(c=list->GetCountOfSelectedPattern())
			{
				Seq_SelectedPattern *selp=list->FirstSelectedPattern();

				// Remove Clone Pattern with Main Pattern in Sel List
				while(selp)
				{
					// Filter Clones and Main Pattern
					if(selp->pattern->mainclonepattern && list->FindPattern(selp->pattern->mainclonepattern))
						c--;

					selp=selp->NextSelectedPattern();
				}

				if(c>0)
				{
					if(UndODeInitPattern *pp=patternpointer=new UndODeInitPattern[c])
					{
						selp=list->FirstSelectedPattern();
						while(selp)
						{
							if((!selp->pattern->mainclonepattern) || (!list->FindPattern(selp->pattern->mainclonepattern)) )
							{
								pp->pattern=selp->pattern;
								pp->track=selp->pattern->GetTrack();
								pp++;
							}

							selp=selp->NextSelectedPattern();
						}	
					}
				}
			}
		}
		else
		{
			c=1;

			if(patternpointer=new UndODeInitPattern)
			{
				patternpointer->pattern=pattern;
				patternpointer->track=pattern->GetTrack();
			}
		}

		if(patternpointer)
		{
			if(Undo_DeletePattern *udp=new Undo_DeletePattern(song,patternpointer,c))
			{	
				if(song==mainvar->GetActiveSong())mainthreadcontrol->LockActiveSong();

				udp->Do();
				song->CheckCrossFades();

				if(song==mainvar->GetActiveSong()){
					song->CheckPlaybackRefresh();
					mainthreadcontrol->UnlockActiveSong();
				}

				song->undo.OpenUndoFunction(udp,addtolastundo);
				udp->RefreshGUI(true);

				return udp;
			}
		}
	}

	return 0;
}

bool Undo_CutPattern::RefreshUndo()
{
	return true;
}

Undo_CutPattern::Undo_CutPattern(UndoCutPattern *up,int nr,OSTART pos,Seq_Pattern_VolumeCurve **oc)
{
	id=Undo::UID_CUTPATTERN;
	cutposition=pos;
	cutpattern=up;
	curves=oc;
	counts=nr;
}

void Undo_CutPattern::Do()
{
	for(int i=0;i<counts;i++)
	{
		switch(cutpattern[i].pattern->mediatype)
		{
		case MEDIATYPE_AUDIO:
			{
				cutpattern[i].regionpattern1=cutpattern[i].regionpattern2=0;
				cutpattern[i].region1=cutpattern[i].region2=0;

				AudioPattern *ap=(AudioPattern *)cutpattern[i].pattern;

				LONGLONG cutsampleposition=song->timetrack.ConvertTicksToTempoSamplesStart(ap->GetPatternStart(),cutposition-ap->GetPatternStart());

				cutsampleposition+=ap->GetAudioSampleStart();

				if(cutsampleposition>0 && cutsampleposition<ap->GetAudioSampleEnd())
				{
					TRACE ("Cut Audio Pattern %d at %d \n",ap->GetSamples(),cutsampleposition);

					// Region 1 || Region 2
					if(AudioRegion *ar=new AudioRegion(ap->audioevent.audioefile))
					{
						cutpattern[i].region1=ar;

						ar->regionstart=ap->GetAudioSampleStart(); 
						ar->regionend=cutsampleposition-1;
						ar->InitRegion();

						//ap->audioevent.audioefile->AddVRegion(ar);

						if(AudioPattern *reg1=new AudioPattern)
						{
							reg1->patternname=mainvar->GenerateString(ap->GetName());

							ap->t_colour.Clone(&reg1->t_colour);
						
							cutpattern[i].regionpattern1=reg1;

							reg1->audioevent.audioefile=ap->audioevent.audioefile;

							reg1->audioevent.SetToRegion(ar);
							
							reg1->audioevent.staticostart=reg1->audioevent.ostart=ap->GetPatternStart();

							reg1->volumecurve.fadeinoutactive=ap->volumecurve.fadeinoutactive;
							reg1->volumecurve.dbvolume=ap->volumecurve.dbvolume;

							reg1->volumecurve.init=true; // Skip AddSort Init
							
							ap->track->AddSortPattern(reg1,ap->GetPatternStart());
							reg1->InitDefaultVolumeCurve(true);

							ap->track->checkcrossfade=true;
						}
					}

					if(AudioRegion *ar=new AudioRegion(ap->audioevent.audioefile))
					{
						cutpattern[i].region2=ar;

						ar->regionstart=cutsampleposition; 
						ar->regionend=ap->GetAudioSampleEnd();
						ar->InitRegion();

						// ap->audioevent.audioefile->AddVRegion(ar);

						if(AudioPattern *reg2=new AudioPattern)
						{
							reg2->patternname=mainvar->GenerateString(ap->GetName());

							ap->t_colour.Clone(&reg2->t_colour);
							
							cutpattern[i].regionpattern2=reg2;

							reg2->audioevent.audioefile=ap->audioevent.audioefile;

							reg2->audioevent.SetToRegion(ar);
							
							reg2->audioevent.staticostart=reg2->audioevent.ostart=cutposition+1;

							ap->volumecurve.init=true; // Skip AddSort Init
							ap->track->AddSortPattern(reg2,cutposition+1);

							reg2->volumecurve.fadeinoutactive=ap->volumecurve.fadeinoutactive;
							reg2->volumecurve.dbvolume=ap->volumecurve.dbvolume;
							reg2->InitDefaultVolumeCurve(true);

							ap->track->checkcrossfade=true;
						}

						// maingui->RefreshRegionGUI(ar);
					}

					if(cutpattern[i].region1 && cutpattern[i].region2)
					{
						ap->StopAllofPattern();

						// Backup special

						/*
						if(ap->volumecurve.automationtrack) // Volume Subtrack ?
						{
							ap->track->DeleteAutomationTrack(ap->volumecurve.automationtrack,false);
						}
*/

						ap->track->CutPattern(ap,true);
						cutpattern[i].bufferpattern=ap;
					}

				}
			}
			break;

		case MEDIATYPE_MIDI:
			{
				MIDIPattern *mp=(MIDIPattern *)cutpattern[i].pattern;

				// Find Left/Right Events
				//index_newpattern=-1;

				if(Seq_Event *re=mp->FindEventAtPosition(cutposition,SEL_ALLEVENTS,0))
				{
					if(re->PrevEvent())
					{
						if(cutpattern[i].newMIDIPattern=(MIDIPattern *)mp->CreateClone(0,CLONEFLAG_NODATA))
						{	
							// move events --> newpattern
							mp->MoveAllEvents(cutpattern[i].newMIDIPattern,re);

							// Events on both side of position
							mp->track->AddSortPattern(cutpattern[i].newMIDIPattern,cutposition);	

							//index_newpattern=mp->track->GetOfPattern(newMIDIPattern);
							mp->track->checkcrossfade=true;
						}
					}
				}

				cutpattern[i].newMIDIPattern->CloseEvents();
				mp->CloseEvents();
			}
			break;
		}
	}
}

void Undo_CutPattern::RefreshGUI(bool undorefresh)
{
	for(int i=0;i<counts;i++)
	{
		if(cutpattern[i].bufferpattern) // Audio
			maingui->RemovePatternFromGUI(song,cutpattern[i].bufferpattern);
		else
			maingui->RemovePatternFromGUI(song,cutpattern[i].pattern);
	}

	mainedit->CheckEditElementsForGUI(song,this,undorefresh);
}

void Undo_CutPattern::UndoGUI()
{
	for(int i=0;i<counts;i++)
	{
		if(cutpattern[i].bufferpattern)
		{
			// Audio
			maingui->RemovePatternFromGUI(song,cutpattern[i].regionpattern1);
			maingui->RemovePatternFromGUI(song,cutpattern[i].regionpattern2);
		}
		else
		{
			// MIDI
			maingui->RemovePatternFromGUI(song,cutpattern[i].newMIDIPattern);
		}
	}
}

void Undo_CutPattern::DoUndo()
{
	for(int i=0;i<counts;i++)
	{
		if(cutpattern[i].bufferpattern)
		{
			// Audio
			if(cutpattern[i].regionpattern1)
			{
				cutpattern[i].regionpattern1->StopAllofPattern();
				cutpattern[i].regionpattern1->track->DeletePattern(cutpattern[i].regionpattern1,true);
				//	maingui->RemovePatternFromGUI(regionpattern1);
				cutpattern[i].regionpattern1=0;
			}

			if(cutpattern[i].regionpattern2)
			{
				cutpattern[i].regionpattern2->StopAllofPattern();
				cutpattern[i].regionpattern2->track->DeletePattern(cutpattern[i].regionpattern2,true);
				//	maingui->RemovePatternFromGUI(regionpattern2);

				cutpattern[i].regionpattern2=0;
			}

			cutpattern[i].bufferpattern->track->AddSortPattern(cutpattern[i].bufferpattern,cutpattern[i].bufferpattern->GetPatternStart());
		
			if(curves[i])
			curves[i]->Clone(&cutpattern[i].bufferpattern->volumecurve);
		
			// Clones
			Seq_ClonePattern *cl=cutpattern[i].bufferpattern->FirstClone();
			while(cl)
			{
				cl->pattern->visible=true;
				cl->pattern->track->AddSortPattern(cl->pattern,cl->pattern->ostart);
				cl->insideundo=false;
				cl->pattern->RefreshAfterPaste();

				cl=cl->NextClone();
			}

			cutpattern[i].bufferpattern=0;
		}
		else
		{
			if(cutpattern[i].newMIDIPattern)
			{
				// Move newpattern->Events to pattern

				MIDIPattern *mp=(MIDIPattern *)cutpattern[i].pattern;
				MIDIPattern *nmp=(MIDIPattern *)cutpattern[i].newMIDIPattern;

				// move events --> newpattern
				nmp->MoveAllEvents(mp);
				//	maingui->RemovePatternFromGUI(newpattern);

				nmp->StopAllofPattern();
				nmp->track->DeletePattern(nmp,true);

				mp->CloseEvents();
			}
		}

		cutpattern[i].pattern->track->checkcrossfade=true;
	}
}

void Undo_CutPattern::FreeData()
{
	if(cutpattern)
	{
		for(int i=0;i<counts;i++)
		{
			if(cutpattern[i].bufferpattern)
			{
				cutpattern[i].bufferpattern->track=0;
				cutpattern[i].bufferpattern->Delete(true);
			}
		}

		delete cutpattern;
	}

	if(curves)
	{
		for(int i=0;i<counts;i++)
			if(curves[i])
			delete curves[i];

		delete curves;
	}
}

void EditFunctions::CutPattern(Seq_SelectionList *sl,OSTART cutposition)
{
	int c=0;

	if(sl && CheckIfEditOK(sl->song)==true)
	{
		Seq_SelectedPattern *check=sl->FirstSelectedPattern();

		while(check)
		{
			if(check->pattern &&
				(check->pattern->mainpattern==0) &&
				check->pattern->track && 
				check->pattern->GetPatternStart()<cutposition && 
				check->pattern->GetPatternEnd()>cutposition)
			{
				c++;
			}

			check=check->NextSelectedPattern();
		}

		if(c)
		{
			if(UndoCutPattern *ucp=new UndoCutPattern[c])
			{
				if(Seq_Pattern_VolumeCurve **oc=new Seq_Pattern_VolumeCurve*[c])
				{
					int i=0;
					Seq_SelectedPattern *check=sl->FirstSelectedPattern();

					while(check)
					{
						if(check->pattern &&
							check->pattern->mainpattern==0 &&
							check->pattern->track && 
							check->pattern->GetPatternStart()<cutposition && 
							check->pattern->GetPatternEnd()>cutposition)
						{
							if(oc[i]=new Seq_Pattern_VolumeCurve)
								check->pattern->volumecurve.Clone(oc[i]);

							ucp[i++].pattern=check->pattern;
						}

						check=check->NextSelectedPattern();
					}

					if(Undo_CutPattern *uc=new Undo_CutPattern(ucp,c,cutposition,oc))
					{
						OpenLockAndDo(sl->song,uc,true);
					}
				}
			}
		}
	}

	/*
	if(p && p->track && && p->GetPatternStart()<cutposition && p->GetPatternEnd()>cutposition)
	{
	Seq_Song *song=p->track->song;

	switch(p->mediatype)
	{
	case MEDIATYPE_MIDI:
	{
	MIDIPattern *mp=(MIDIPattern *)p;

	if(mp->GetCountOfEvents()>1)
	{
	if(Undo_CutPattern *ucp=new Undo_CutPattern(ucp,cutposition))
	{
	ucp->index_track=song->GetOfTrack(p->track);
	ucp->index_pattern=p->track->GetOfPattern(p);

	OpenLockAndDo(song,ucp,true);
	}
	}
	}
	break;

	case MEDIATYPE_AUDIO:
	{
	// Create 2 Regions
	if(Undo_CutPattern *ucp=new Undo_CutPattern(cutposition))
	{
	ucp->index_track=song->GetOfTrack(p->track);
	ucp->index_pattern=p->track->GetOfPattern(p);

	OpenLockAndDo(song,ucp,true);
	}
	}
	break;
	}
	}
	*/

}

// Create AudioPattern or Audio Region Pattern
AudioPattern *EditFunctions::CreateAudioPattern(Seq_Track *t,AudioHDFile *hd,AudioRegion *r,OSTART position,guiWindow *win)
{
	AudioFileWork *afwork=0;
	char *waitfornewfile=0;

	if(t && CheckIfEditOK(t->song)==true && position>=0)
	{
		AudioPattern *newaudio=0;

		if(t && hd)
		{
			if(UndoCreatePattern *ucp=new UndoCreatePattern)
			{
				ucp->mediatype=MEDIATYPE_AUDIO;
				ucp->position=position;
				ucp->track=t;

				Undo_CreatePattern *ufunction=new Undo_CreatePattern(t->song,ucp,1);

				if(ufunction)
				{
					bool unlock=true;
					Lock(t->song);

					ufunction->Do();

					if(newaudio=(AudioPattern *)ufunction->createpattern->newpattern)
					{
						if(hd->samplerate!=mainaudio->GetGlobalSampleRate())
						{
							AudioFileWork tmp;

							tmp.filename=mainvar->GenerateString(hd->GetName());

							char *tmph=mainaudio->GenerateSampleRateTMP();

							if(tmp.CreateTMP(tmph)==true)
							{
								AudioHDFile *prhd;

								if(mainaudio->CheckIfAudioFile(tmp.creatednewfile)==true &&
									(prhd=mainaudio->AddAudioFile(tmp.creatednewfile,false))
									)
								{
									hd=newaudio->audioevent.audioefile=prhd;
									newaudio->SetName(hd->GetName());
									hd->Open(); //Change to resampled File
								}
								else
								{
									CheckPlaybackAndUnLock();

									// Resampling/Decode
									unlock=false;

									char zz2[NUMBERSTRINGLEN],
										*h2=mainvar->ConvertIntToChar(hd->samplerate,zz2),
										*to=mainvar->GenerateString(hd->GetFileName(),"(",h2,") Hz"),
										*h=mainvar->GenerateString(Cxs[CXS_CONVERTSAMPLERATE],"?\n",to);

									if(h)
									{
										bool ok=maingui->MessageBoxYesNo(win,h);

										if(ok==true)
										{
											if(AudioFileWork_Resample *work=new AudioFileWork_Resample){

												newaudio->waitforresample=true;
												newaudio->waitforresamplefile=mainvar->GenerateString(hd->GetName());

												{
													AudioFileWork ntmp;

													ntmp.filename=mainvar->GenerateString(hd->GetName());

													char *tmph=mainaudio->GenerateSampleRateTMP();

													if(ntmp.CreateTMP(tmph)==true)
														newaudio->waitforresampleendfile=mainvar->GenerateString(ntmp.creatednewfile);

													if(tmph)delete tmph;
												}

												work->Init(hd->GetName()); // org file
												work->newsamplerate=mainaudio->GetGlobalSampleRate();

												audioworkthread->AddWork(work);
											}
										}

										delete h;
									}

									if(to)delete to;
								}
							}

							if(tmph)delete tmph;
						}
						else
						{
							if(r)
								newaudio->SetName(r->regionname);
							else
								if(hd)
									newaudio->SetName(hd->GetName());

							//	AudioHDFile *newhdfile=mainaudio->AddAudioFile(readfile,false);
							bool doresample;
							AudioHDFile *usenewhdfile=0;

							if(InitNewAudioFile(hd,t->song->directoryname,&waitfornewfile,&doresample,&afwork,&usenewhdfile)==true)
							{
								if(usenewhdfile)
								{
									hd=usenewhdfile;
								}

								if(doresample==true)
								{
									newaudio->waitforresample=true;
									newaudio->waitforresamplefile=mainvar->GenerateString(hd->GetName());

									if(waitfornewfile)
										newaudio->waitforresampleendfile=mainvar->GenerateString(waitfornewfile);

									// Copy Regions
									AudioRegion *reg=hd->FirstRegion();
									while(reg)
									{
										if(AudioRegion *nr=new AudioRegion)
										{
											if(r==reg)
												newaudio->waitforregion=nr;

											reg->CloneTo(nr);
											afwork->regions.AddEndO(nr);
										}

										reg=reg->NextRegion();
									}
								}

								// hd=0;
							}
							else
							{
								if(usenewhdfile)
								{
									hd=usenewhdfile;
								}

								// Same Sampling Rate
								newaudio->audioevent.audioefile=hd;

								if(r)
								{
									newaudio->audioevent.audioregion=new AudioRegion;

									if(newaudio->audioevent.audioregion)
									{
										r->CloneTo(newaudio->audioevent.audioregion);
										newaudio->audioevent.audioregion->InitRegion();
									}
								}

								hd->Open();
							}
						}

						t->song->CheckCrossFades();
					}

					if(unlock==true)
						CheckPlaybackAndUnLock();

					if(newaudio)
					{
						t->song->undo.OpenUndoFunction(ufunction);
						mainedit->CheckEditElementsForGUI(t->song,ufunction,true);	
					}

					if(waitfornewfile)
						delete waitfornewfile;

					if(afwork)
						audioworkthread->AddWork(afwork);

					return newaudio;

				} // if ufunction
			}
		}
	}

	if(waitfornewfile)
		delete waitfornewfile;

	if(afwork)
		audioworkthread->AddWork(afwork);

	return 0;
}

Undo_SplitMIDIPattern::Undo_SplitMIDIPattern(Seq_Song *song,MIDIPattern *from,int c,Seq_Track *to)
{
	id=Undo::UID_SPLITPATTERN;

	for(int i=0;i<16;i++)newcreatedtrack[i]=0;
	frompattern=from;
	totrack=to;
	splitchannel=c;
	newcreatedpattern=0;

	orgpattern=new MIDIPattern;
	from->Clone(song,orgpattern,0,0);
}

void Undo_SplitMIDIPattern::FreeData()
{
	orgpattern->Delete(true);
}

void Undo_SplitMIDIPattern::Do()
{
	if(!totrack)
	{
		int channels[16];

		for(int i=0;i<16;i++)
			channels[i]=0;

		// Check Events
		{
			Seq_Event *e=frompattern->FirstEvent();

			while(e)
			{
				if(e->CheckIfChannelMessage()==true)
					channels[e->GetChannel()]=1;

				e=e->NextEvent();
			}
		}

		bool starttrack=false;

		int trackindex=song->GetOfTrack(frompattern->track)+1;

		for(int i=0;i<16;i++)
		{
			if(channels[i])
			{
				if(starttrack==false)
					starttrack=true;
				else
				{
					newcreatedtrack[i]=mainedit->CreateNewTrack(this,song,frompattern->track,trackindex++,false,false);

					if(newcreatedtrack[i])
					{
						if(char *newname=new char[strlen(frompattern->GetName())+NUMBERSTRINGLEN])
						{
							char number[NUMBERSTRINGLEN];

							strcpy(newname,frompattern->GetName());
							size_t slen=strlen(newname);

							char *npos=mainvar->ConvertIntToChar(i+1,number);

							if(npos && slen+strlen(npos)+2<STANDARDSTRINGLEN)
							{
								strcpy(&newname[slen],"_");
								slen++;
								strcpy(&newname[slen],npos);

								newcreatedtrack[i]->SetName(newname);
							}

							//frompattern->track->CloneFx(newcreatedtrack[i]);
							newcreatedtrack[i]->t_trackeffects.SetChannel(i+1);

							MIDIPattern *newcreatedpattern=(MIDIPattern *)mainedit->CreateNewPattern(this,newcreatedtrack[i],MEDIATYPE_MIDI,frompattern->GetPatternStart(),false);

							if(newcreatedpattern)
							{
								frompattern->t_colour.Clone(&newcreatedpattern->t_colour);
								frompattern->MoveEventsWithChannel(i,newcreatedpattern);

								newcreatedpattern->CloseEvents();
							}

							delete newname;
						}

					}// new track ?
				}

			} // channels ?
			else
				newcreatedtrack[i]=0;


		}// i channels
	}// !totrack
	else
	{
		newcreatedpattern=(MIDIPattern *)mainedit->CreateNewPattern(this,totrack,MEDIATYPE_MIDI,frompattern->GetPatternStart(),false);

		if(newcreatedpattern)
		{
			frompattern->t_colour.Clone(&newcreatedpattern->t_colour);
			frompattern->MoveEventsWithChannel(splitchannel,newcreatedpattern);
			newcreatedpattern->CloseEvents();
		}
	}

	frompattern->CloseEvents();
}

void Undo_SplitMIDIPattern::DoUndo()
{
	if(!totrack)
	{
		frompattern->DeleteEvents();
		orgpattern->CloneEvents(song,frompattern);

		for(int i=0;i<16;i++)
		{		
			if(newcreatedtrack[i])
			{
				song->DeleteTrack(newcreatedtrack[i],true); // Split+no lock
			}
		}
	}
	else
	{
		if(newcreatedpattern)
		{
			mainMIDI->StopAllofPattern(song,newcreatedpattern);
			newcreatedpattern->MoveAllEvents(frompattern);
			newcreatedpattern=0;
		}
	}

	frompattern->CloseEvents();
}

void Undo_SplitMIDIPattern::UndoGUI()
{
	if(!totrack)
	{
		for(int i=0;i<16;i++)
		{		
			if(newcreatedtrack[i])
				maingui->RemoveTrackFromGUI(newcreatedtrack[i]);
		}	
	}
	else
	{
		if(newcreatedpattern)
			maingui->RemovePatternFromGUI(song,newcreatedpattern);
	}
}
bool EditFunctions::SplitPatternToChannels(MIDIPattern *pattern)
{
	if(pattern && pattern->track && CheckIfEditOK(pattern->track->song)==true)
	{
		//Seq_Track *track=pattern->track;

		if(pattern->CheckPatternChannel()==0)
		{
			if(Undo_SplitMIDIPattern *split=new Undo_SplitMIDIPattern(pattern->track->song,pattern,0,0))
			{
				OpenLockAndDo(pattern->track->song,split,true);
				return true;
			}
		}
	}

	return false;
}

Undo_SplitMIDIPattern_Types::Undo_SplitMIDIPattern_Types(MIDIPattern *from,int c,Seq_Track *to)
{
	id=Undo::UID_SPLITPATTERN;

	for(int i=0;i<16;i++)newcreatedtrack[i]=0;
	frompattern=from;
	totrack=to;
	splitchannel=c;

	orgpattern=new MIDIPattern;
	from->Clone(from->track->song,orgpattern,0,0);
}


void Undo_SplitMIDIPattern_Types::FreeData()
{
	orgpattern->Delete(true);
}

void Undo_SplitMIDIPattern_Types::Do()
{
	for(int cl=0;cl<16;cl++)
		newcreatedtrack[cl]=0;

	MIDIPatternInfo info;
	int c=0;

	frompattern->CreatePatternInfo(&info);

	int trackindex=song->GetOfTrack(frompattern->track)+1;

	for(int ix=0;ix<8;ix++)
	{
		char *add=0;
		UBYTE status=0;

		switch(ix)
		{
		case 0:
			// keep notes in pattern
			/*
			if(info.notes)
			status=NOTEON;
			*/
			break;

		case 1:
			if(info.control)
				status=CONTROLCHANGE;
			add="Ctrl";
			break;

		case 2:
			if(info.sysex)
				status=SYSEX;
			add="SysEx";
			break;

		case 3:
			if(info.pitch)
				status=PITCHBEND;
			add="Pitch";
			break;

		case 4:
			if(info.prog)
				status=PROGRAMCHANGE;
			add="Prog";
			break;

		case 5:
			if(info.polypress)
				status=POLYPRESSURE;
			add="PolyPress";
			break;

		case 6:
			if(info.cpress)
				status=CHANNELPRESSURE;
			add="ChlPress";
			break;

		case 7:
			if(info.intern)
				status=INTERN;
			add="Intern";
			break;
		}

		if(status)
		{
			newcreatedtrack[c]=mainedit->CreateNewTrack(this,frompattern->track->song,frompattern->track,trackindex++,false,false);

			if(newcreatedtrack[c])
			{
				totrack=newcreatedtrack[c];

				size_t al=add?strlen(add):0;

				if(char *newname=new char[strlen(frompattern->GetName())+al+8])
				{
					strcpy(newname,frompattern->GetName());
					if(add)
					{
						size_t slen=strlen(newname);
						strcpy(&newname[slen],"_");
						slen++;
						strcpy(&newname[slen],add);
					}

					newcreatedtrack[c]->SetName(newname);
					//frompattern->track->CloneFx(newcreatedtrack[c]);

					MIDIPattern *newcreatedpattern=(MIDIPattern *)mainedit->CreateNewPattern(this,newcreatedtrack[c],MEDIATYPE_MIDI,frompattern->GetPatternStart(),false);

					if(newcreatedpattern)
					{
						frompattern->t_colour.Clone(&newcreatedpattern->t_colour);
						frompattern->MoveEventsWithType(status,newcreatedpattern);

						newcreatedpattern->CloseEvents();
					}

					delete newname;
				}

				c++;

			}// new track ?

		} // channels ?

	}// i channels

	frompattern->CloseEvents();
}

void Undo_SplitMIDIPattern_Types::UndoGUI()
{
	for(int i=0;i<16;i++)
	{		
		if(newcreatedtrack[i])
			maingui->RemoveTrackFromGUI(newcreatedtrack[i]);
	}	
}

void Undo_SplitMIDIPattern_Types::DoUndo()
{
	for(int i=0;i<16;i++)
	{		
		if(newcreatedtrack[i])
		{
			frompattern->DeleteEvents();
			orgpattern->CloneEvents(song,frompattern);

			song->DeleteTrack(newcreatedtrack[i],true); // Split, no lock
		}
	}

	frompattern->CloseEvents();
}

bool EditFunctions::SplitPatternToEvents(MIDIPattern *pattern)
{
	if(pattern && pattern->track && CheckIfEditOK(pattern->track->song)==true)
	{
		if(Undo_SplitMIDIPattern_Types *split=new Undo_SplitMIDIPattern_Types(pattern,0,0))
		{
			OpenLockAndDo(pattern->track->song,split,true);
			return true;
		}
	}

	return false;
}

void Undo_FlipMIDIPattern_Types::Do()
{
	if(initok==true && newset==false)
	{
		mainMIDI->StopAllofPattern(frompattern->track->song,frompattern);

		TRACE ("C1 Old FE %d NE %d\n",frompattern->FirstEvent(),newfirstevent);

		frompattern->SetFirstEvent(newfirstevent);
		TRACE ("C2 FE %d\n",frompattern->FirstEvent());

		frompattern->SetLastEvent(newlastevent);
		frompattern->SetFirstVirtualEvent(newvfirstevent);
		frompattern->SetLastVirtualEvent(newvlastevent);
		frompattern->events.Reset(newnrobjects);
		frompattern->virtualevents.Reset(newvnrobjects);

		Seq_Event *e=frompattern->FirstEvent();
		while(e)
		{
			e->pattern=frompattern;
			e=e->NextEvent();
		}

		e=frompattern->FirstVirtualEvent();
		while(e)
		{
			e->pattern=frompattern;
			e=e->NextEvent();
		}

		newcreatedpattern->SetFirstEvent(oldfirstevent);
		newcreatedpattern->SetLastEvent(oldlastevent);
		newcreatedpattern->SetFirstVirtualEvent(oldvfirstevent);
		newcreatedpattern->SetLastVirtualEvent(oldvlastevent);
		newcreatedpattern->events.Reset(oldnrobjects);
		newcreatedpattern->virtualevents.Reset(oldvnrobjects);

		e=newcreatedpattern->FirstEvent();
		while(e)
		{
			e->pattern=newcreatedpattern;
			e=e->NextEvent();
		}

		e=newcreatedpattern->FirstVirtualEvent();
		while(e)
		{
			e->pattern=newcreatedpattern;
			e=e->NextEvent();
		}
		newset=true;

		newcreatedpattern->CloseEvents();

#ifdef _DEBUG
		int co=0;
		Seq_Event *ce=frompattern->FirstEvent();
		while(ce)
		{
			co++;
			ce=ce->NextEvent();
		}

		if(co!=frompattern->events.GetCount())
			maingui->MessageBoxOk(0,"Flip Event <Pattern NUMBER Error>");

		Seq_Event *c=frompattern->FirstEvent();

		while(c)
		{
			if(c->pattern!=frompattern)
				maingui->MessageBoxOk(0,"Flip Event <Pattern Error>");

			c=c->NextEvent();
		}

		c=frompattern->FirstVirtualEvent();

		while(c)
		{
			if(c->pattern!=frompattern)
				maingui->MessageBoxOk(0,"Flip V Event <Pattern Error>");

			c=c->NextEvent();
		}

		if(frompattern->FirstEvent()->prev)
			maingui->MessageBoxOk(0,"Flip Event PREV <Pattern Error>");

		if(frompattern->LastEvent()->next)
			maingui->MessageBoxOk(0,"Flip Event NEXT <Pattern Error>");
#endif
	}
}

void Undo_FlipMIDIPattern_Types::DoUndo()
{
	if(initok==true && newset==true)
	{
		mainMIDI->StopAllofPattern(frompattern->track->song,frompattern);

		frompattern->SetFirstEvent(oldfirstevent);
		frompattern->SetLastEvent(oldlastevent);
		frompattern->SetFirstVirtualEvent(oldvfirstevent);
		frompattern->SetLastVirtualEvent(oldvlastevent);
		frompattern->events.Reset(oldnrobjects);
		frompattern->virtualevents.Reset(oldvnrobjects);

		Seq_Event *e=frompattern->FirstEvent();
		while(e)
		{
			e->pattern=frompattern;
			e=e->NextEvent();
		}

		e=frompattern->FirstVirtualEvent();
		while(e)
		{
			e->pattern=frompattern;
			e=e->NextEvent();
		}
		newcreatedpattern->SetFirstEvent(newfirstevent);
		newcreatedpattern->SetLastEvent(newlastevent);
		newcreatedpattern->SetFirstVirtualEvent(newvfirstevent);
		newcreatedpattern->SetLastVirtualEvent(newvlastevent);
		newcreatedpattern->events.Reset(newnrobjects);
		newcreatedpattern->virtualevents.Reset(newvnrobjects);

		e=newcreatedpattern->FirstEvent();
		while(e)
		{
			e->pattern=newcreatedpattern;
			e=e->NextEvent();
		}

		e=newcreatedpattern->FirstVirtualEvent();
		while(e)
		{
			e->pattern=newcreatedpattern;
			e=e->NextEvent();
		}

		newset=false;

		newcreatedpattern->CloseEvents();
	}
}

void Undo_FlipMIDIPattern_Types::FreeData()
{
	if(newcreatedpattern)
		newcreatedpattern->Delete(true);
}

bool EditFunctions::StretchPattern(MIDIPattern *pattern,int to)
{
	return false;
}

bool EditFunctions::FlipPattern(MIDIPattern *pattern)
{
	if(pattern && (!pattern->mainpattern))
	{
		if(Undo_FlipMIDIPattern_Types *flip=new Undo_FlipMIDIPattern_Types(pattern))
		{
			flip->oldfirstevent=pattern->FirstEvent();
			flip->oldlastevent=pattern->LastEvent();
			flip->oldvfirstevent=pattern->FirstVirtualEvent();
			flip->oldvlastevent=pattern->LastVirtualEvent();
			flip->oldnrobjects=pattern->events.GetCount();
			flip->oldvnrobjects=pattern->virtualevents.GetCount();

			if(flip->newcreatedpattern=(MIDIPattern *)pattern->CreateClone(0,CLONEFLAG_NOFX))
			{
				flip->newcreatedpattern->SetName("RotBuffer");

				Seq_Event *firstnoteon=flip->newcreatedpattern->FirstEvent();
				while(firstnoteon && firstnoteon->GetStatus()!=NOTEON)
					firstnoteon=firstnoteon->NextEvent();

				Seq_Event *lastnoteon=flip->newcreatedpattern->LastEvent();
				while(lastnoteon && lastnoteon->GetStatus()!=NOTEON)
					lastnoteon=lastnoteon->PrevEvent();


				if(firstnoteon && lastnoteon && firstnoteon!=lastnoteon)
				{
					flip->start=lastnoteon->GetEventStart();
					flip->end=firstnoteon->GetEventStart();

					int notecounter=0;
					Seq_Event *note=flip->newcreatedpattern->FirstEvent();
					while(note)
					{
						if(note->GetStatus()==NOTEON)notecounter++;
						note=note->NextEvent();
					}

					if(Seq_Event **notelist=new Seq_Event*[notecounter])
					{
						int c=0;
						note=flip->newcreatedpattern->FirstEvent();

						while(note)
						{
							if(note->GetStatus()==NOTEON)
							{
								notelist[c++]=note;
								note=flip->newcreatedpattern->DeleteEvent(note,false);
							}
							else
								note=note->NextEvent();
						}

						for(int i=0;i<notecounter;i++)
						{
							OSTART newstart=flip->start-(notelist[i]->GetEventStart()-flip->end);
							notelist[i]->AddSortToPattern(flip->newcreatedpattern,newstart);
						}

						delete notelist;

						flip->initok=true;
					}
				}
			}

			if(flip->initok==true)
			{
				flip->newfirstevent=flip->newcreatedpattern->FirstEvent();
				flip->newlastevent=flip->newcreatedpattern->LastEvent();
				flip->newvfirstevent=flip->newcreatedpattern->FirstVirtualEvent();
				flip->newvlastevent=flip->newcreatedpattern->LastVirtualEvent();
				flip->newnrobjects=flip->newcreatedpattern->events.GetCount();
				flip->newvnrobjects=flip->newcreatedpattern->virtualevents.GetCount();
			}

			OpenLockAndDo(pattern->track->song,flip,true);

			return true;
		}
	}

	return false;
}

void Undo_ConvertClonePattern::Do()
{
	Seq_ClonePattern *clp=mainpattern->FirstClone();

	while(clp && clp->pattern!=pattern)
		clp=clp->NextClone();

	if(clp)
	{
		mainpattern->DeleteClone(clp,false); // Delete Clone
		pattern->track->DeletePattern(pattern,false);

		if(newpattern=mainpattern->CreateClone(position-mainpattern->GetPatternStart(),0))
		{
			pattern->CloneFX(newpattern);
			pattern->t_colour.Clone(&newpattern->t_colour);
			totrack->AddSortPattern(newpattern,position);
			newpattern->RefreshAfterPaste();
			totrack->AddEffectsToPattern(newpattern); // New Track Effects
		}

		mainpattern->CloseEvents();
	}
}

Undo_ConvertLoopPattern::Undo_ConvertLoopPattern(Seq_Pattern *p,int ix,bool conv)
{
	id=Undo::UID_CONVERTLOOPPATTERN;

	mainpattern=p;
	newpattern=0;
	index=ix;
	convert=conv;
	oldpattern=0;
}

void Undo_ConvertLoopPattern::FreeData()
{
	if(oldpattern)
	{
		oldpattern->Delete(true);
		oldpattern=0;
	}

	if(inundo==false && newpattern)
	{
		newpattern->Delete(true);
	}
}

void Undo_ConvertClonePattern::DoUndo()
{
	if(newpattern)
	{
		newpattern->track->DeletePattern(newpattern,true);
		newpattern=0;

		mainpattern->AddClone(pattern);
		totrack->AddSortPattern(pattern,position);
		pattern->RefreshAfterPaste();

		pattern->CloseEvents();
	}
}

Undo_ConvertClonePattern::Undo_ConvertClonePattern(Seq_Pattern *p)
{
	id=Undo::UID_CONVERTCLONEPATTERN;

	pattern=p;
	position=pattern->GetPatternStart();
	mainpattern=pattern->mainpattern;
	totrack=pattern->track;
	newpattern=0;
}

void Undo_ConvertClonePattern::FreeData()
{
	if(newpattern)newpattern->Delete(true);
}

bool Undo_ConvertClonePattern::RefreshUndo()
{
	return false;
}

void EditFunctions::ConvertCloneToPattern(Seq_Pattern *pattern)
{
	if(pattern && pattern->mainclonepattern && pattern->itsaclone==true && CheckIfEditOK(pattern->track->song)==true)
	{
		if(Undo_ConvertClonePattern *con=new Undo_ConvertClonePattern(pattern))
			OpenLockAndDo(pattern->track->song,con,true);
	}
}

bool Undo_ConvertLoopPattern::RefreshUndo()
{
	return CheckPattern(mainpattern);
}

void Undo_ConvertLoopPattern::Do()
{
	if(mainpattern)
	{
		if(!newpattern)
		{
			if(Seq_LoopPattern *slp=mainpattern->GetLoop(index))
			{
				OSTART diff=slp->pattern->GetPatternStart()-mainpattern->GetPatternStart();

				oldloops=mainpattern->loops;
				oldloopwithloops=mainpattern->loopwithloops;
				oldloopendless=mainpattern->loopendless;

				switch(mainpattern->id)
				{
				case OBJ_MIDIPattern:
					{
						MIDIPattern *mp=(MIDIPattern *)new MIDIPattern;

						if(newpattern=mp)
						{
							mainpattern->Clone(song,mp,diff,0);

							if(convert==true) // Convert Loop To Events and add to main pattern
							{
								oldpattern=(MIDIPattern *)new MIDIPattern; // Backup old mainpattern

								if(oldpattern)
									mainpattern->Clone(song,oldpattern,0,0); // Buffer old patte

								mainpattern->StopAllofPattern();
								mp->StopAllofPattern();

								MIDIPattern *mainMIDIPattern=(MIDIPattern *)mainpattern;

								mp->MoveAllEvents(mainMIDIPattern);

								mainpattern->loopwithloops=false;
								mainpattern->loops=0;
								mainpattern->loopendless=false;

								mainMIDIPattern->CloseEvents();
								mp->Delete(true); // Delete Clone Buffer

								newpattern=0; // Convert, no new Pattern !
							}
							else
								newposition=mainpattern->GetPatternStart()+diff;
						}
					}
					break;

				case OBJ_AUDIOPATTERN:
					{
						AudioPattern *ap=(AudioPattern *)new AudioPattern;

						if(newpattern=ap)
						{
							mainpattern->Clone(song,ap,diff,0);
							newposition=mainpattern->GetPatternStart()+diff;
						}
					}
					break;
				}
			}
		}

		if(newpattern || convert==true)
		{
			mainpattern->DeleteLoops();

			if(newpattern)
			{
				newpattern->ResetLoops();
				mainpattern->track->AddSortPattern(newpattern,newposition);
			}

			if(convert==false)
				mainpattern->LoopPattern();
		}
	}
}

void Undo_ConvertLoopPattern::DoUndo()
{
	if(newpattern || oldpattern)
	{
		mainpattern->StopAllofPattern();

		if(oldpattern)
		{
			switch(mainpattern->mediatype)
			{
			case MEDIATYPE_MIDI:
				{
					MIDIPattern *mp=(MIDIPattern *)mainpattern;
					mp->DeleteEvents(); // Delete mixed events

					oldpattern->MoveAllEvents(mp);
					oldpattern->Delete(true);
					oldpattern=0;
				}
				break;
			}
		}
		else
			newpattern->track->DeletePattern(newpattern,false);

		mainpattern->loopendless=oldloopendless;
		mainpattern->loopwithloops=oldloopwithloops;
		mainpattern->loops=oldloops;

		mainpattern->LoopPattern();
	}
}

void Undo_ConvertLoopPattern::RefreshGUI(bool undorefresh)
{
	if(newpattern && undorefresh==false)
		maingui->RemovePatternFromGUI(song,newpattern,false); // dead pointer

	mainedit->CheckEditElementsForGUI(song,this,undorefresh);
}

void EditFunctions::ConvertLoopToPattern(Seq_Pattern *pattern,bool convert)
{
	if(pattern && pattern->mainpattern && CheckIfEditOK(pattern->track->song)==true)
	{
		Seq_Song *song=pattern->mainpattern->track->song; // buffer song
		Undo_ConvertLoopPattern *con=new Undo_ConvertLoopPattern(pattern->mainpattern,pattern->mainpattern->GetLoopIndex(pattern),convert);

		if(con)
			OpenLockAndDo(song,con,true);
	}
}

Undo_MixPatternToPattern::Undo_MixPatternToPattern(MIDIPattern **l,MIDIPattern *p,int c,bool a,bool con)
{
	id=Undo::UID_MIXSELPATTERNTOPATTERN;
	list=l;
	topattern=p;
	nrpattern=c;
	oldpattern=0;
	add=a;
	connect=con;
	patternbuffered=false;
}

void Undo_MixPatternToPattern::FreeData()
{
	if(oldpattern)oldpattern->Delete(true);

	if(list)
	{
		if(patternbuffered==true)
		{
			for(int i=0;i<nrpattern;i++)
				list[i]->Delete(true);
		}
		delete list;
	}
}

void Undo_MixPatternToPattern::DoEnd()
{
	for(int i=0;i<nrpattern;i++)
	{
		maingui->RemovePatternFromGUI(song,list[i]);
	}
}

void Undo_MixPatternToPattern::Do()
{
	if(list && topattern && nrpattern)
	{
		if(oldpattern=new MIDIPattern)
		{
			topattern->Clone(song,oldpattern,0,CLONEFLAG_NOFX); // backup org pattern

			// Mix Pattern->toppattern
			for(int i=0;i<nrpattern;i++)
			{
				if(add==true)
					list[i]->MixEventsToPattern(song,topattern); // Add
				else
					list[i]->CloneMixEventsToPattern(song,topattern); // 1-1 Mix

				if(connect==true)
				{
					list[i]->StopAllofPattern();
					topattern->track->DeletePattern(list[i],false);
					patternbuffered=true;
				}

			}

			topattern->track->AddEffectsToPattern(topattern); // New Track Effects
		}
	}
}

void Undo_MixPatternToPattern::DoUndo()
{
	if(oldpattern){

		topattern->StopAllofPattern();
		topattern->DeleteEvents(); // Delete Mix
		oldpattern->MoveAllEvents(topattern);
	
		oldpattern->Delete(true);
		oldpattern=0;

		if(connect==true)
		{
			for(int i=0;i<nrpattern;i++)
			{
				topattern->track->AddSortPattern(list[i],list[i]->GetPatternStart());
			}

			patternbuffered=false;
		}
	}
}

void EditFunctions::MixAllSelectedPatternToPattern(Seq_SelectionList *list,MIDIPattern *pattern,bool add,bool connect)
{
	if(list && pattern && pattern->mediatype==MEDIATYPE_MIDI && pattern->mainpattern==0 && CheckIfEditOK(pattern->track->song)==true)
	{
		int c=0,connectc=0;

		Seq_SelectedPattern *sl=list->FirstSelectedPattern();

		while(sl)
		{
			if(sl->pattern!=pattern && sl->pattern->mediatype==MEDIATYPE_MIDI)
			{
				if(connect==true)
				{
					if(sl->pattern->GetTrack()==pattern->GetTrack())
					{
						c++;
						connectc++;
					}
				}
				else
					c++;
			}

			sl=sl->NextSelectedPattern();
		}

		if(c)
		{
			if(MIDIPattern **sel=new MIDIPattern*[c])
			{
				int i=0;

				Seq_SelectedPattern *sl=list->FirstSelectedPattern();

				while(sl)
				{
					if(sl->pattern!=pattern && sl->pattern->mediatype==MEDIATYPE_MIDI)
					{
						if(connect==true)
						{
							if(sl->pattern->GetTrack()==pattern->GetTrack())
								sel[i++]=(MIDIPattern *)sl->pattern;
						}
						else
							sel[i++]=(MIDIPattern *)sl->pattern;
					}

					sl=sl->NextSelectedPattern();
				}

				Undo_MixPatternToPattern *mix=new Undo_MixPatternToPattern(sel,pattern,c,add,connect);

				if(mix)
					OpenLockAndDo(pattern->track->song,mix,true);
				else
					delete sel;
			}
		}
	}
}

Undo_AddLoopsToPattern::Undo_AddLoopsToPattern(MIDIPattern *p)
{
	id=Undo::UID_ADDLOOPSTOPATTERN;
	mainpattern=p;
	oldpattern=0;
}

void Undo_AddLoopsToPattern::FreeData()
{
	if(oldpattern)
	{
		oldpattern->Delete(true);
		oldpattern=0;
	}
}

void Undo_AddLoopsToPattern::Do()
{
	if(mainpattern && mainpattern->FirstLoopPattern() && mainpattern->mediatype==MEDIATYPE_MIDI)
	{
		// backup loop vars
		oldloops=mainpattern->loops;
		oldloopwithloops=mainpattern->loopwithloops;
		oldloopendless=mainpattern->loopendless;

		if(oldpattern=new MIDIPattern)
		{
			mainpattern->Clone(song,oldpattern,0,CLONEFLAG_NOFX); // Buffer old Pattern

			MIDIPattern buffer;

			Seq_LoopPattern *loop=mainpattern->FirstLoopPattern();			
			while(loop)
			{
				mainMIDI->StopAllofPattern(mainpattern->track->song,loop->pattern);

				OSTART diff=loop->pattern->GetPatternStart()-mainpattern->GetPatternStart();
				mainpattern->Clone(song,&buffer,diff,CLONEFLAG_NOFX); // New Event Buffer |.....
				loop=loop->NextLoop();
			}

			buffer.MoveAllEvents(mainpattern); // Move Buffer To Pattern	

			mainpattern->loopwithloops=false;
			mainpattern->loops=0;
			mainpattern->loopendless=false;

			mainpattern->DeleteLoops();
		}
	}
}

void Undo_AddLoopsToPattern::DoUndo()
{
	if(oldpattern)
	{
		mainMIDI->StopAllofPattern(mainpattern->track->song,mainpattern);

		mainpattern->DeleteOpenNotes();
		mainpattern->DeleteEvents();

		oldpattern->MoveAllEvents(mainpattern);
		oldpattern->Delete(true);

		oldpattern=0;

		mainpattern->loops=oldloops;
		mainpattern->loopwithloops=oldloopwithloops;
		mainpattern->loopendless=oldloopendless;

		mainpattern->LoopPattern();
	}
}

void EditFunctions::ConvertAllLoopsToPattern(MIDIPattern *pattern)
{
	if(pattern && pattern->mainpattern==0 && CheckIfEditOK(pattern->track->song)==true)
	{
		if(Undo_AddLoopsToPattern *con=new Undo_AddLoopsToPattern(pattern))
		{
			OpenLockAndDo(pattern->track->song,con,true);

			// nr split pattern

			/*
			pattern->track->song->undo.OpenUndoFunction(con);
			Lock(pattern->track->song);
			con->Do();
			if(	pattern->track->song==mainvar->GetActiveSong() )
			pattern->track->song->CheckPlaybackRefresh();
			UnLock();
			mainedit->CheckEditElementsForGUI(pattern->track->song,con,true);
			*/
		}
	}
}

void Undo_CreatePattern::UndoGUI()
{
	if(createpattern)
	{
		for(int i=0;i<numberofpattern;i++)
		{
			if(createpattern[i].newpattern && createpattern[i].recordeventsadded==false)
			{
				createpattern[i].newpattern->visible=false;
				maingui->RemovePatternFromGUI(song,createpattern[i].newpattern);
			}
		}	
	}
}

void Undo_CreatePattern::DoUndo()
{
	if(createpattern)
	{
		for(int i=0;i<numberofpattern;i++)
		{
			if(createpattern[i].newpattern)
			{
				if(createpattern[i].recordeventsadded==true)
				{
					if(createpattern[i].recordeventslist)
					{
						MIDIPattern *mp=(MIDIPattern *)createpattern[i].newpattern;
						for(int ec=0;ec<createpattern[i].recordeventsaddedcounter;ec++)
						{
							// Delete Events
							mainMIDI->StopAllofEvent(song,createpattern[i].recordeventslist[ec]);
							mp->DeleteEvent(createpattern[i].recordeventslist[ec],false);
						}
					}
				}
				else
				{
					createpattern[i].newpattern->visible=false;
					createpattern[i].newpattern->StopAllofPattern();

					/*
					// Backup special
					switch(createpattern[i].newpattern->mediatype)
					{
					case MEDIATYPE_AUDIO:
						{
							AudioPattern *ap=(AudioPattern *)createpattern[i].newpattern;

							if(ap->volumecurve.automationtrack) // Volume Subtrack ?
							{
								createpattern[i].newpattern->GetTrack()->DeleteAutomationTrack(ap->volumecurve.automationtrack,true);
								ap->volumecurve.automationtrack=0;
							}
						}
						break;
					}
*/

					createpattern[i].newpattern->GetTrack()->checkcrossfade=true;

					if(inundo==false)
					{
						createpattern[i].newpattern->GetTrack()->DeletePattern(createpattern[i].newpattern,true); // Split Track
						createpattern[i].newpattern=0;
					}
					else
						createpattern[i].newpattern->GetTrack()->DeletePattern(createpattern[i].newpattern,false);
				}
			}
		}	
	}
}

void Undo_CreatePattern::DoRedo()
{
	for(int i=0;i<numberofpattern;i++)
	{
		if(createpattern[i].newpattern)
		{
			if(createpattern[i].recordeventsadded==true)
			{
				if(createpattern[i].recordeventslist)
				{
					MIDIPattern *mp=(MIDIPattern *)createpattern[i].newpattern;
					for(int ec=0;ec<createpattern[i].recordeventsaddedcounter;ec++)
					{
						// Add Events
						mp->AddSortEvent(createpattern[i].recordeventslist[ec]);
					}
				}
			}
			else
			{
				createpattern[i].newpattern->visible=true;
				createpattern[i].track->AddSortPattern(createpattern[i].newpattern,createpattern[i].position);
				createpattern[i].track->checkcrossfade=true;
				createpattern[i].newpattern->RefreshAfterPaste();
			}
		}
	}	
}

bool Undo_CreatePattern::RefreshUndo()
{
	bool ok=true;

	/*
	if(inundo==true)
	{
		for(int c=0;c<numberofpattern;c++)
		{	
			if(createpattern[c].track->pattern.GetIx(createpattern[c].newpattern)==-1)
				return false;
		}
	}
	else
	{
		for(int c=0;c<numberofpattern;c++)
		{	
			if(CheckPattern(createpattern[c].newpattern)==false)
				return false;
		}
	}
*/

	return ok;
}

void Undo_CreatePattern::Do()
{
	for(int i=0;i<numberofpattern;i++)
	{	
		switch(createpattern[i].mediatype)
		{
		case MEDIATYPE_MIDI:
			{
				MIDIPattern *mp=new MIDIPattern;

				if(createpattern[i].newpattern=mp)
				{
					size_t sl=strlen(createpattern[i].track->GetName());

					if(sl+strlen("_MIDI")<STANDARDSTRINGLEN-1)
					{
						if(char *nn=mainvar->GenerateString(createpattern[i].track->GetName(),"_MIDI"))
						{
							createpattern[i].newpattern->SetName(nn);
							delete nn;
						}
					}
					else
						createpattern[i].newpattern->SetName("MIDI");
				}
			}
			break;

		case MEDIATYPE_AUDIO_RECORD:
		case MEDIATYPE_AUDIO:
			{
				AudioPattern *ap=new AudioPattern;

				if(createpattern[i].newpattern=ap)
				{
					// Mediatype _ Record = avoid Peak check etc...
					size_t sl=strlen(createpattern[i].track->GetName());

					if(sl+strlen("_Audio")<STANDARDSTRINGLEN-1)
					{
						if(char *nn=mainvar->GenerateString(createpattern[i].track->GetName(),"_Audio"))
						{
							createpattern[i].newpattern->SetName(nn);
							delete nn;
						}
					}
					else
						createpattern[i].newpattern->SetName("Audio");

					ap->mediatype=createpattern[i].mediatype;
					ap->audioevent.staticostart=ap->audioevent.ostart=createpattern[i].position;
				}	
			}
			break;
		}

		if(createpattern[i].newpattern)
		{
			createpattern[i].track->AddSortPattern(createpattern[i].newpattern,createpattern[i].position);
			createpattern[i].track->checkcrossfade=true;

			if(createpattern[i].mainclonepattern)
			{
				if(song->FindPattern(createpattern[i].mainclonepattern))
				{
					createpattern[i].mainclonepattern->AddClone(createpattern[i].newpattern);
					//createpattern[i].mainclonepattern->SetClonesName();
					createpattern[i].mainclonepattern->SetClonesOffset();
					createpattern[i].mainclonepattern->RefreshAfterPaste();
				}
				else
					createpattern[i].mainclonepattern=0;
			}

			// Load MIDI File
			if(MIDIfile){
				MIDIFile smf;
				smf.ReadMIDIFileToPattern(song,createpattern[i].track,(MIDIPattern *)createpattern[i].newpattern,MIDIfile);
			}
			else
				if(patternfile)
					createpattern[i].newpattern->LoadFromFile(patternfile);
		}

	}// while c

	//	refreshmode|=song->RefreshLoops();
}

void Undo_CreatePattern::FreeData()
{
	if(inundo==false && createpattern)
	{
		for(int i=0;i<numberofpattern;i++)
		{
			if(createpattern[i].newpattern && createpattern[i].recordeventsadded==false)
			{
				switch(createpattern[i].newpattern->mediatype)
				{
				case MEDIATYPE_AUDIO:
					{
						/*
						AudioPattern *ap=(AudioPattern *)createpattern[i].newpattern;

						if(ap->volumecurve.automationtrack)
						{
							ap->volumecurve.automationtrack->DeleteObjects();
							delete ap->volumecurve.automationtrack;
							ap->volumecurve.automationtrack=0;
						}
						*/
					}
					break;
				}

				// Reset CF
				createpattern[i].newpattern->DeleteAllCrossFades(true,false); // Avoid Pattern Access
				createpattern[i].newpattern->Delete(true);
			}
		}
	}

	if(createpattern)
	{
		for(int i=0;i<numberofpattern;i++)
		{
			if(createpattern[i].recordeventslist)
			{
				if(inundo==false)
				{
					for(int ec=0;ec<createpattern[i].recordeventsaddedcounter;ec++)
						createpattern[i].recordeventslist[ec]->Delete(true);
				}

				delete createpattern[i].recordeventslist;
			}
		}

		delete createpattern;
		createpattern=0;
		numberofpattern=0;
	}

	if(MIDIfile)delete MIDIfile;
	if(patternfile)delete patternfile;
}

Seq_Pattern *EditFunctions::CreateNewPattern(UndoFunction *openuf,Seq_Track *track,int type,OSTART startpos,bool doundo,int flag,OList *list,Seq_Pattern *mainclonepattern)
{
	if(track && ((flag&CNP_NOEDITOKCHECK) || CheckIfEditOK(track->song)==true) )
	{
		bool lockf=false;

		if(UndoCreatePattern *np=new UndoCreatePattern)
		{
			np->mediatype=type;
			np->track=track;
			np->position=startpos;
			np->mainclonepattern=mainclonepattern;

			Undo_CreatePattern *ucp=new Undo_CreatePattern(track->song,np,1);

			if((flag&CNP_ADDTOLASTUNDO) && (!openuf))
			{
				// Add to last
				openuf=track->song->undo.LastUndo();

				if(openuf)
					doundo=true;
			}

			if(ucp)
			{
				// Undo ?
				if(doundo==true)
				{	
					if(!openuf)
					{
						track->song->undo.OpenUndoFunction(ucp);
						lockf=true;
					}
					else
						openuf->AddFunction(ucp);
				}

				if(lockf==true)
					Lock(track->song);

				ucp->Do();

				// Add Events ?
				if(list)
				{
					for(int i=0;i<ucp->numberofpattern;i++)
					{	
						if(ucp->createpattern[i].newpattern)
							switch(ucp->createpattern[i].newpattern->mediatype)
						{
							case MEDIATYPE_MIDI:
								{
									MIDIPattern *mp=(MIDIPattern *)ucp->createpattern[i].newpattern;

									// Move List -> Pattern
									Seq_Event *fe=(Seq_Event *)list->GetRoot();

									while(fe)
									{
										Seq_Event *n=fe->NextEvent();
										fe->AddSortToPattern(mp);
										fe=n;
									}
								}
								break;
						}
					}
				}

				if(lockf==true)
				{
					if(!(flag&CNP_NOCHECKPLAYBACK))
					{
						if(track->song==mainvar->GetActiveSong())
							track->song->CheckPlaybackRefresh();
					}

					UnLock();

					if(!(flag&CNP_NOGUIREFRESH))
						CheckEditElementsForGUI(track->song,ucp,true);
				}

				Seq_Pattern *r=0;

				if(ucp->createpattern)
					r=ucp->createpattern->newpattern;

				if(doundo==false)
					ucp->DeleteUndoFunction(true,true);

				return r;	
			}

			delete np;
		}
	}

	return 0;
}

void Undo_SizePattern::DoUndo()
{
	pattern->StopAllofPattern();

	if(offset==-1)
	{
		pattern->SetOffSetEnd(oldoffset_right,false);
		pattern->SetOffSetStart(oldoffsetposition,oldoffset_left,false);
	}
	else
	{
		if(right==true)
			pattern->SetOffSetEnd(oldoffset_right,false);
		else
			pattern->SetOffSetStart(oldoffsetposition,oldoffset_left,false);
	}

	pattern->track->checkcrossfade=true;

	pattern->useoffsetregion=olduseoffsetregion;

	if(pattern->mediatype==MEDIATYPE_AUDIO)
	{
		AudioPattern *ap=(AudioPattern *)pattern;
		oldoffsetregion.CloneTo(&ap->offsetregion);
	}

	Seq_ClonePattern *scp=pattern->FirstClone();
	while(scp)
	{
		// scp->pattern->StopAllofPattern();

		/*
		if(offset==-1)
		{
		scp->pattern->SetOffSetEnd(oldoffset_right,false);
		scp->pattern->SetOffSetStart(oldoffsetposition,oldoffset_left,false);
		}
		else
		{
		if(right==true)
		scp->pattern->SetOffSetEnd(oldoffset_right,false);
		else
		scp->pattern->SetOffSetStart(oldoffsetposition,oldoffset_left,false);
		}
		*/

		scp->pattern->track->checkcrossfade=true;

		scp=scp->NextClone();
	}
}

void Undo_SizePattern::Do()
{
	pattern->StopAllofPattern();

	if(offset==-1) // Reset
	{
		pattern->SetOffSetEnd(0,false);
		pattern->SetOffSetStart(position,0,false);
		pattern->useoffsetregion=false;
	}
	else
	{
		if(right==true)
			pattern->SetOffSetEnd(offset,false); // <- End
		else
			pattern->SetOffSetStart(position,offset,false); // -> Start

	}

	pattern->track->checkcrossfade=true;

	Seq_ClonePattern *scp=pattern->FirstClone();
	while(scp)
	{
		scp->pattern->track->checkcrossfade=true;
		scp=scp->NextClone();
	}
}


void EditFunctions::SizePattern(Seq_Pattern *pattern,OSTART position,LONGLONG sampleoffset,bool right)
{
	if(position<0)position=0;

	if(Undo_SizePattern *usp=new Undo_SizePattern)
	{
		usp->position=position;
		usp->pattern=pattern;
		usp->offset=sampleoffset;
		usp->right=right;

		usp->olduseoffsetregion=pattern->useoffsetregion;

		if(pattern->mediatype==MEDIATYPE_AUDIO)
		{
			AudioPattern *ap=(AudioPattern *)pattern;
			ap->offsetregion.CloneTo(&usp->oldoffsetregion);
		}

		usp->oldoffsetposition=pattern->GetPatternStart();

		usp->oldoffset_left=pattern->GetOffSetStart();
		usp->oldoffset_right=pattern->GetOffSetEnd();

		OpenLockAndDo(pattern->track->song,usp,true);
	}
}

bool Undo_MovePattern::RefreshUndo()
{
	for(int c=0;c<numberofpattern;c++)
	{
		if(song->FindPattern(movepattern[c].temp_pattern)==false)return false;
	}

	return true;
}

void Undo_MovePattern::DoUndo()
{
	UndoMovePattern *pp=movepattern;

	// Init
	for(int c=0;c<numberofpattern;c++)
	{
		if(pp->ok==true)
		{
			Seq_Track *track=song->GetTrackIndex(pp->index_newtrack),*oldtrack=song->GetTrackIndex(pp->index_oldtrack);

			if(track && oldtrack)
				pp->temp_pattern=track->GetPatternIndex(pp->index_newpattern);
			else
				pp->ok=false;
		}

		pp++;
	}

	// Do 
	pp=movepattern;

	for(int c=0;c<numberofpattern;c++)
	{
		if(pp->ok==true)
		{
			Seq_Track *track=song->GetTrackIndex(pp->index_newtrack),*oldtrack=song->GetTrackIndex(pp->index_oldtrack);

			if(track && oldtrack)
			{
				bool moved=false;

				pp->temp_pattern->StopAllofPattern();

				// Move to new track ?
				if(oldtrack!=track)
				{	
					track->MovePatternToTrack(pp->temp_pattern,oldtrack,pp->oldposition);
					track->checkcrossfade=true;
					oldtrack->checkcrossfade=true;
					moved=true;
				}
				else
				{
					if(pp->oldposition!=pp->temp_pattern->GetPatternStart())
					{
						pp->temp_pattern->MovePattern(pp->oldposition-pp->temp_pattern->GetPatternStart(),0); // new position
						oldtrack->checkcrossfade=true;
						moved=true;
					}
				}

				if(moved==true)
				{
					oldtrack->AddEffectsToPattern(pp->temp_pattern); // New Track Effects

					/*

					// Restore old CrossFades
					if(pp->crossfades.GetRoot())
					{
					// Find Existing CrossFade
					Seq_CrossFade *cf=(Seq_CrossFade *)pp->crossfades.GetRoot(),*ccf=(Seq_CrossFade *)pp->crossfades_connect.GetRoot();

					while(cf && ccf)
					{
					if(Seq_CrossFade *found=cf->pattern->FindCrossFade(ccf->pattern))
					{
					found->CopyData(cf);
					found->connectwith->CopyData(ccf);

					Seq_CrossFade *fcf=found->connectwith;
					found->pattern->DeleteCrossFade(found);
					fcf->pattern->DeleteCrossFade(fcf);
					}

					cf->dontdeletethis=true;
					ccf->dontdeletethis=true;

					cf->connectwith=ccf;
					ccf->connectwith=cf;

					ccf=ccf->NextCrossFade();
					cf=cf->NextCrossFade();
					}

					pp->temp_pattern->crossfades.DeleteAllO(); // Dont use DeleteAllCrossFades!
					pp->crossfades.MoveListToList(&pp->temp_pattern->crossfades);

					//Connect Move To Pattern
					ccf=(Seq_CrossFade *)pp->crossfades_connect.GetRoot();
					while(ccf)
					{
					Seq_CrossFade *nccf=ccf->NextCrossFade();
					ccf->pattern->crossfades.AddEndO(ccf);
					ccf=nccf;
					}

					pp->crossfades_connect.Clear();
					}

					*/

				}
			}
#ifdef _DEBUG
			else
				MessageBox(NULL,"Illegal Undo Move Pattern (Track) ","Error",MB_OK);
#endif
		}

		pp++;
	}
}

void Undo_MovePattern::FreeData()
{
	if(movepattern)
		delete movepattern;
}

void Undo_MovePattern::Do()
{
	UndoMovePattern *pp=movepattern;

	// Init
	for(int c=0;c<numberofpattern;c++)
	{
		pp->temp_pattern=0;

		if(pp->oldtrack=pp->totrack=song->GetTrackIndex(pp->index_oldtrack))
		{
			if(pp->temp_pattern=pp->oldtrack->GetPatternIndex(pp->index_pattern))
			{
				if(movediff+pp->temp_pattern->GetPatternStart()<0)
				{
					pp->temp_pattern=0;
				}
				else
				{
					if(trackindex!=0) // move up or down ?
					{
						pp->totrack=song->GetTrackIndex(pp->index_oldtrack+trackindex);

					}
				}
			}
#ifdef _DEBUG
			else
				MessageBox(NULL,"Move Pattern Error","Error",MB_OK);
#endif
		}
#ifdef _DEBUG
		else
		{
			MessageBox(NULL,"Move Pattern Track Error","Error",MB_OK);
		}

#endif
		pp++;
	}// while

	// Do
	pp=movepattern;
	for(int c=0;c<numberofpattern;c++)
	{
		if(pp->totrack && pp->totrack->IsEditAble()==true && pp->temp_pattern)
		{
			pp->temp_pattern->StopAllofPattern();

			OSTART qposition=movediff+pp->temp_pattern->GetPatternStart();

			if(quantize==true)
			qposition=song->QuantizeWithFlag(qposition,qflag);

			bool moved=false;

			// Move to new track ?
			if(pp->oldtrack!=pp->totrack)
			{
				pp->oldtrack->MovePatternToTrack(pp->temp_pattern,pp->totrack,qposition);
				pp->oldtrack->checkcrossfade=true;
				pp->totrack->checkcrossfade=true;
				moved=true;
			}
			else
			{
				if(qposition!=pp->temp_pattern->GetPatternStart())
				{
					pp->temp_pattern->MovePattern(qposition-pp->temp_pattern->GetPatternStart(),0); // new position
					pp->oldtrack->checkcrossfade=true;
					moved=true;
				}
			}

			if(moved==true)
				pp->totrack->AddEffectsToPattern(pp->temp_pattern); // New Track Effects	
		}
		else
			pp->ok=false;

		pp++;
	}

	// Set Index
	pp=movepattern;

	for(int c=0;c<numberofpattern;c++)
	{
		if(pp->ok==true)
		{
			pp->index_newtrack=song->GetOfTrack(pp->totrack);
			pp->index_newpattern=pp->totrack->GetOfPattern(pp->temp_pattern);
		}

		pp++;
	}
}

bool EditFunctions::MovePatternList(MoveO *mo)
{
	if(mo && CheckIfEditOK(mo->song)==true)
	{
		if(mo->song && (mo->allpattern==true || (mo->sellist && mo->sellist->FirstSelectedPattern())) && (mo->sellist && mo->sellist->CheckMove(mo)==true) ) // <-> or up/down ?
		{
			int c=0;

			if(mo->special==false)
			{
				if(mo->sellist)
					c=mo->sellist->GetCountOfSelectedPattern()+mo->sellist->GetCountOfLinkPatternSelectedPattern();
			}
			else
			{
				c=mo->song->GetCountOfPattern(mo->cyclecheck==true?mo->song->playbacksettings.cyclestart:0,mo->sellist,mo->selectedpattern,mo->selectedtracks);
			}

			if(c)
			{
				UndoMovePattern *patternpointer,*pp=patternpointer=new UndoMovePattern[c];
				OSTART checkstartposition=-1;

				if(patternpointer)
				{
					if(mo->allpattern==true)
					{
						Seq_Track *t=mo->song->FirstTrack();
						while(t)
						{
							if(mo->selectedtracks==false || (t->flag&OFLAG_SELECTED))
							{
								Seq_Pattern *p=t->FirstPattern();
								while(p)
								{
									if(mo->selectedpattern==false || mo->sellist->FindSelectedPattern(p))
									{
										if(p->itsaloop==false &&
											(mo->cyclecheck==false || p->GetPatternStart()>=mo->song->playbacksettings.cyclestart))
										{
											pp->index_pattern=t->GetOfPattern(p);
											pp->index_oldtrack=mo->song->GetOfTrack(t);
											pp->oldposition=p->GetPatternStart();

											if(checkstartposition==-1 || pp->oldposition<checkstartposition)
												checkstartposition=pp->oldposition;

											pp++;
										}
									}

									p=p->NextPattern();
								}
							}

							t=t->NextTrack();
						}
					}
					else
					{
						Seq_SelectedPattern *selp=mo->sellist->FirstSelectedPattern();

						// Real Pattern first
						while(selp)
						{
							if(selp->pattern->mainpattern==0)
							{
								pp->index_pattern=selp->pattern->track->GetOfPattern(selp->pattern);
								pp->index_oldtrack=mo->song->GetOfTrack(selp->pattern->track);
								pp->oldposition=selp->pattern->GetPatternStart();

								if(checkstartposition==-1 || pp->oldposition<checkstartposition)
									checkstartposition=pp->oldposition;

								pp++;
							}

							selp=selp->NextSelectedPattern();
						}

						// Linked Pattern
						selp=mo->sellist->FirstSelectedPattern();

						while(selp)
						{
							if(selp->pattern->mainpattern==0 && selp->pattern->link)
							{
								PatternLink *pl=selp->pattern->link;
								PatternLink_Pattern *plp=pl->FirstLinkedPattern();

								while(plp)
								{
									if(!mo->sellist->FindSelectedPattern(plp->pattern))
									{
										pp->index_pattern=plp->pattern->track->GetOfPattern(plp->pattern);
										pp->index_oldtrack=mo->song->GetOfTrack(plp->pattern->track);
										pp->oldposition=plp->pattern->GetPatternStart();

										if(checkstartposition==-1 || pp->oldposition<checkstartposition)
											checkstartposition=pp->oldposition;

										pp++;
									}

									plp=plp->NextLink();
								}
							}

							selp=selp->NextSelectedPattern();
						}

						// Clones
						selp=mo->sellist->FirstSelectedPattern();

						while(selp)
						{
							if(selp->pattern->mainpattern)
							{
								pp->index_pattern=selp->pattern->track->GetOfPattern(selp->pattern);
								pp->index_oldtrack=mo->song->GetOfTrack(selp->pattern->track);
								pp->oldposition=selp->pattern->GetPatternStart();

								if(checkstartposition==-1 || pp->oldposition<checkstartposition)
									checkstartposition=pp->oldposition;

								pp++;
							}

							selp=selp->NextSelectedPattern();
						}
					}

					if(Undo_MovePattern *ump=new Undo_MovePattern(patternpointer,c,mo->diff,mo->index,mo->flag,mo->quantize))
					{
						OpenLockAndDo(mo->song,ump,true);
						return true;
					}	


					delete patternpointer;
				}
			}
		}
	}

	return false;
}

bool Undo_MutePattern::RefreshUndo()
{
	for(int c=0;c<numberofpattern;c++)
	{
		if(CheckPattern(mutepattern[c].pattern)==false)return false;
	}

	return true;
}

void Undo_MutePattern::DoUndo()
{
	UndoMutePattern *pp=mutepattern;

	for(int c=0;c<numberofpattern;c++){
		pp->pattern->mute=pp->oldmute;

		// Mute/Unmute Loops
		Seq_LoopPattern *l=pp->pattern->FirstLoopPattern();
		while(l){
			l->pattern->mute=pp->oldmute;
			l=l->NextLoop();
		}

		pp++;
	}
}

void Undo_MutePattern::Do()
{
	UndoMutePattern *pp=mutepattern;
	for(int c=0;c<numberofpattern;c++)
	{
		//mainMIDI->StopAllofPattern(pp->pattern->track->song,pp->pattern);

		pp->pattern->mute=mute;

		// Mute/Unmute Loops
		Seq_LoopPattern *l=pp->pattern->FirstLoopPattern();
		while(l){
			l->pattern->mute=mute;
			l=l->NextLoop();
		}

		pp++;
	}	
}

void EditFunctions::MutePattern(Seq_Song *song,Seq_SelectionList *list, Seq_Pattern *single,bool mute)
{	
	// Check
	if(single && (!list))
	{
		if(single->mute==mute)return;
	}
	else
		if(list && single)
		{
			bool ok=false;

			if(single->mute!=mute)ok=true;

			Seq_SelectedPattern *selp=list->FirstSelectedPattern();

			while(selp){
				if(selp->pattern->mute!=mute)ok=true;
				selp=selp->NextSelectedPattern();
			}

			if(ok==false)return;
		}
		else
			if(list)
			{
				bool ok=false;

				Seq_SelectedPattern *selp=list->FirstSelectedPattern();

				while(selp){
					if(selp->pattern->mute!=mute)ok=true;
					selp=selp->NextSelectedPattern();
				}

				if(ok==false)return;
			}
			else
				return;

	UndoMutePattern *patternpointer=0;
	int c=0;
	if(single && list && list->FirstSelectedPattern())
	{
		c=list->GetCountOfSelectedPattern();

		if(!list->FindPattern(single))c++;

		if(c)
		{
			UndoMutePattern *pp=patternpointer=new UndoMutePattern[c];

			if(patternpointer)
			{
				Seq_SelectedPattern *selp=list->FirstSelectedPattern();

				while(selp){
					pp->pattern=selp->pattern;
					pp->oldmute=selp->pattern->mute;
					pp++;
					selp=selp->NextSelectedPattern();
				}

				if(!list->FindPattern(single))
				{
					patternpointer->pattern=single;
					patternpointer->oldmute=single->mute;
				}
			}
		}
	}
	else
		if(single) // 1 Pattern
		{
			if(patternpointer=new UndoMutePattern[c=1])
			{
				patternpointer->pattern=single;
				patternpointer->oldmute=single->mute;
			}
		}
		else
			if(list && list->FirstSelectedPattern())
			{	
				if(c=list->GetCountOfSelectedPattern())
				{
					UndoMutePattern *pp=patternpointer=new UndoMutePattern[c];

					if(patternpointer)
					{
						Seq_SelectedPattern *selp=list->FirstSelectedPattern();

						while(selp){
							pp->pattern=selp->pattern;
							pp->oldmute=selp->pattern->mute;
							pp++;
							selp=selp->NextSelectedPattern();
						}
					}
				}
			}

			if(c && patternpointer)
			{
				if(Undo_MutePattern *ump=new Undo_MutePattern(patternpointer,c,mute))
				{
					// no Lock
					ump->Do();
					song->undo.OpenUndoFunction(ump);
					CheckEditElementsForGUI(song,ump,true);
				}
				else
					delete patternpointer;
			}
}

bool Undo_LoopPattern::RefreshUndo()
{
	return CheckPattern(pattern);
}

void Undo_LoopPattern::DoUndo()
{
	if(pattern=song->GetPatternIndex(index_track,index_pattern,MEDIATYPE_ALL))
	{
		pattern->loopwithloops=oldloopwithloops;
		pattern->loopendless=oldloopendless;
		pattern->loops=oldloops;
		pattern->LoopPattern();
	}
}

void Undo_LoopPattern::Do()
{
	if(pattern=song->GetPatternIndex(index_track,index_pattern,MEDIATYPE_ALL))
	{
		oldloopwithloops=pattern->loopwithloops;
		oldloopendless=pattern->loopendless;
		oldloops=pattern->loops;

		pattern->loopendless=loopendless;
		pattern->loopwithloops=loopwithloops;
		pattern->loops=loops;
		pattern->LoopPattern();
	}
#ifdef _DEBUG
	else
		MessageBox(NULL,"Loop Pattern Index not found","Error",MB_OK);
#endif
}

bool EditFunctions::LoopPattern(Seq_Pattern *pattern, bool endless,bool withloops,int newloops)
{
	if(pattern && CheckIfEditOK(pattern->track->song)==true)
	{
		Seq_Song *song=pattern->GetTrack()->song;

		if( (newloops!=pattern->loops || withloops!=pattern->loopwithloops || endless!=pattern->loopendless) && 
			(pattern->mediatype==MEDIATYPE_MIDI || pattern->mediatype==MEDIATYPE_AUDIO)
			)
		{
			Undo_LoopPattern *ulp=new Undo_LoopPattern(song->GetOfTrack(pattern->track),pattern->track->GetOfPattern(pattern),endless,withloops,newloops);

			if(ulp){
				OpenLockAndDo(song,ulp,true);
				return true;
			}
		}
	}

	return false;
}

Undo_ReplaceAudioPatternFile::Undo_ReplaceAudioPatternFile(AudioPattern *pattern,AudioHDFile *newhd,AudioRegion *r)
{
	id=Undo::UID_REPLACEAUDIOPATTERNFILE;

	audiopattern=pattern;
	newhdfile=newhd;
	newregion=r;
	oldhdfile=pattern->audioevent.audioefile;
	oldregion=pattern->audioevent.audioregion;
	
	audiopattern->volumecurve.Clone(&oldvolume);
}

bool Undo_ReplaceAudioPatternFile::RefreshUndo()
{
	if(newhdfile)newregion=newhdfile->FindRegion(newregion);
	if(oldhdfile)oldregion=oldhdfile->FindRegion(oldregion);
	return true;
}

void Undo_ReplaceAudioPatternFile::Do()
{
	audiopattern->track->checkcrossfade=true;
	audiopattern->ReplaceAudioHDFileWith(newhdfile,newregion);
	audiopattern->SetName(newhdfile->GetFileName());
}

void Undo_ReplaceAudioPatternFile::DoUndo()
{
	audiopattern->track->checkcrossfade=true;
	audiopattern->ReplaceAudioHDFileWith(oldhdfile,oldregion);

	oldvolume.Clone(&audiopattern->volumecurve);

	audiopattern->SetName(oldhdfile?oldhdfile->GetFileName():0);
}

void Undo_ReplaceAudioPatternFile::RefreshGUI(bool undorefresh)
{
	maingui->RefreshAudioPattern(audiopattern);

	// Clones
	Seq_ClonePattern *c=audiopattern->FirstClone();

	while(c){
		AudioPattern *cp=(AudioPattern *)c->pattern;
		maingui->RefreshAudioPattern(cp);
		c=c->NextClone();
	}
}

