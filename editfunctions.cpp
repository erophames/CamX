#include "defines.h"

#include "gui.h"
#include "editor.h"
#include "undo.h"
#include "patternselection.h"
#include "semapores.h"
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
#include "undofunctions_pattern.h"
#include "undofunctions_track.h"
#include "folder.h"
#include "player.h"
#include "editbuffer.h"
#include "MIDIoutproc.h"
#include "arrangeeditor.h"
#include "drumevent.h"
#include "MIDIfile.h"
#include "pianoeditor.h"
#include "undo_automation.h"
#include "editsettings.h"
#include "editor_event.h"
#include "languagefiles.h"
#include "audiohdfile.h"
#include "drummap.h"
#include "MIDIthruproc.h"
#include "audiofilework.h"
#include "audiopattern.h"
#include "sampleeditor.h"
#include "undo_plugins.h"
#include "object_project.h"


void EditFunctions::LockAndDoFunction(Seq_Song *song,UndoFunction *f,bool undocheck)
{
	if((!f) || (!song))return;

	Lock(song);

	if(f->nodo==false) // nodo==After Recording
	{
		f->Do(); // Execute Function
		f->DoEnd();
	}
	else
		f->nodo=false; // Reset

	song->CreateQTrackList();
	song->CheckCrossFades();
	song->SetMuteFlags();

	if(song==mainvar->GetActiveSong())
		song->CheckPlaybackRefresh();

	UnLock();

	f->RefreshGUI(undocheck);
	f->RefreshDo();

	UndoFunction *sf=f->FirstFunction();
	while(sf)
	{
		sf->RefreshGUI(undocheck);
		sf->RefreshDo();
		sf=sf->NextFunction();
	}

	CheckEditElementsForGUI(song,f,undocheck);
}

void EditFunctions::OpenLockAndDo(Seq_Song *song,UndoFunction *f,bool undocheck)
{
	if(!f)return;
	song->undo.OpenUndoFunction(f); // Add Function
	LockAndDoFunction(song,f,undocheck);
}

void EditFunctions::OpenLockAndDo(Seq_Song *song,UndoFunction *f,bool undocheck,bool addtoundo)
{
	// +Add to last UndoFunction
	if(!f)
		return;

	if(addtoundo==false)
		song->undo.OpenUndoFunction(f); // Add Function
	else
	{
		if(UndoFunction *lundo=song->undo.LastUndo())
			lundo->AddFunction(f);
		else
			song->undo.OpenUndoFunction(f); // Add Function
	}
	LockAndDoFunction(song,f,undocheck);
}

bool EditFunctions::CheckIfWindowRefreshObjects(Seq_Song *song,guiWindow *win,UndoFunction *function)
{
	bool refresh=false;

	switch(function->id)
	{
	case Undo::UID_PASTEPATTERNBUFFER:
		switch(win->GetEditorID())
		{
		case EDITORTYPE_ARRANGE:
		case EDITORTYPE_AUDIOMANAGER:
			refresh=true;
			break;
		}
		break;

	case Undo::UID_DELETEMARKER:
	case Undo::UID_EDITMARKER:
		switch(win->GetEditorID())
		{
		case EDITORTYPE_MARKER:

		case EDITORTYPE_ARRANGE:
		case EDITORTYPE_EVENT:
		case EDITORTYPE_PIANO:
		case EDITORTYPE_WAVE:
		case EDITORTYPE_DRUM:
		case EDITORTYPE_SCORE:
		case EDITORTYPE_TEMPO:
			refresh=true;
			break;
		}
		break;

	case Undo::UID_EDITTEXT:
	case Undo::UID_DELETETEXT:
		switch(win->GetEditorID())
		{
		case EDITORTYPE_TEXT:
			refresh=true;
			break;

		case EDITORTYPE_ARRANGE:
			{
				Edit_Arrange *ar=(Edit_Arrange *)win;

				//if(ar->text==true)
				//	refresh=true;
			}	
			break;
		}
		break;

	case Undo::UID_QUANTIZEEVENT:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					refresh=true;
				}
				break;

			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)win;

					if(ar->shownotesinarrageeditor!=ARRANGEEDITOR_SHOWNOTES_OFF)
						refresh=true;
				}	
				break;
			}
		}
		break;

	case Undo::UID_SETNOTELENGTH:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_SCORE:
				//	case EDITORTYPE_WAVE:
				{
					refresh=true;
				}
				break;

			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)win;

					if(ar->shownotesinarrageeditor!=ARRANGEEDITOR_SHOWNOTES_OFF)
						refresh=true;
				}	
				break;
			}
		}
		break;

	case Undo::UID_CREATENEWTEMPOS:
	case Undo::UID_EDITTEMPOS:
	case Undo::UID_DELETETEMPOS:
	case Undo::UID_MOVETEMPOS:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_SAMPLE:
				{
					Edit_Sample *es=(Edit_Sample *)win;

					if(es->songmode==true)
						refresh=true;
				}
				break;

			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					EventEditor *ee=(EventEditor *)win;

					if(ee->timeline && ee->timeline->format!=WINDOWDISPLAY_MEASURE)
						refresh=true;
				}
				break;

			case EDITORTYPE_EVENT:
				{
					Edit_Event *ed=(Edit_Event *)win;

					if(win->windowtimeformat!=WINDOWDISPLAY_MEASURE || ed->FindAudioEventOnDisplay()==true)
						refresh=true;
				}
				break;

			case EDITORTYPE_MARKER:
			case EDITORTYPE_TEXT:
				if(win->windowtimeformat!=WINDOWDISPLAY_MEASURE)
					refresh=true;
				break;

			case EDITORTYPE_TRANSPORT:
			case EDITORTYPE_TEMPO:
			case EDITORTYPE_TEMPOLIST:
			case EDITORTYPE_ARRANGE: // Tempomap or Audio Display refresh
				refresh=true;
				break;
			}
		}
		break;

	case Undo::UID_CHANGEAUTOMATIONTRACK:
	case Undo::UID_CREATEAUTOMATIONTRACK:
	case Undo::UID_DELETEAUTOMATIONTRACK:
	case Undo::UID_MUTEPATTERN:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_ARRANGELIST:
				refresh=true;
				break;
			}
		}
		break;

	case Undo::UID_CREATEBUS:
	case Undo::UID_DELETEBUS:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_ARRANGELIST:
				refresh=true;
				break;

			case EDITORTYPE_AUDIOMIXER:
				refresh=true; 
				break;
			}
		}
		break;

	case Undo::UID_MOVETRACK:
	case Undo::UID_ADDASCHILDTRACK:
	case Undo::UID_CREATEPARENTTRACK:
	case Undo::UID_REMOVETRACKSFROMPARENT:
	case Undo::UID_DELETEALLEMPTYTRACKS:
	case Undo::UID_SORTTRACKS:
	case Undo::UID_INSERTSOUNDFILE:
	case Undo::UID_CHANGEAUDIOTRACKCHANNEL:
	case Undo::UID_CREATENEWTRACK:
	case Undo::UID_CREATENEWTRACKS:
	case Undo::UID_CREATEFOLDER:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_GROUP:
				{
					if(function->id==Undo::UID_MOVETRACK || function->id==Undo::UID_SORTTRACKS || function->id==Undo::UID_ADDASCHILDTRACK || function->id==Undo::UID_REMOVETRACKSFROMPARENT)
						refresh=true;
				}
				break;

			case EDITORTYPE_SETTINGS:
				if(function->id==Undo::UID_CREATENEWTRACK || function->id==Undo::UID_CREATENEWTRACKS)
				{
					Edit_Settings *set=(Edit_Settings *)win;

					if(set->WindowSong())
					{
						switch(set->settingsselection)
						{
						case Edit_Settings::SONG_ROUTING:
							set->ShowTargetTracks();
							break;
						}
					}
				}
				break;

			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_ARRANGELIST:
				refresh=true;
				break;

			case EDITORTYPE_AUDIOMIXER:
				refresh=true; 
				break;
			}
		}
		break;

	case Undo::UID_SIZEOFNOTES: // length note
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_SCORE:
				refresh=true;
				break;
			}
		}
		break;

	case Undo::UID_SPLITPATTERN:
	case Undo::UID_SPLITTRACK:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGELIST:
			case EDITORTYPE_AUDIOMIXER:
				{
					refresh=true;
				}
				break;

			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					win->refreshflag|=REFRESHEVENT_LIST;
					refresh=true;
				}
				break;
			}
		}
		break;

		/*
		case Undo::UID_REPLACEAUDIOPATTERNFILE:
		{
		switch(win->GetEditorID())
		{
		case EDITORTYPE_SAMPLE:
		{
		Undo_ReplaceAudioPatternFile *rap=(Undo_ReplaceAudioPatternFile *)function;
		Edit_Sample *es=(Edit_Sample*)win;

		if(es->pattern && es->pattern==rap->audiopattern)
		{
		es->audiohdfile=rap->audiopattern->audioevent.audioefile;
		refresh=true;
		}
		}
		break;

		}
		}
		break;
		*/

	case Undo::UID_CUTNOTE:
	case Undo::UID_MIXSELPATTERNTOPATTERN:
	case Undo::UID_COPYEVENTS:
	case Undo::UID_MOVEEVENTS:
	case Undo::UID_DELETEEVENT:
	case Undo::UID_CUTPATTERN:
	case Undo::UID_CONVERTDRUMSTONOTES:
	case Undo::UID_CONVERTNOTESTODRUMS:
	case Undo::UID_FLIPPATTERN:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					win->refreshflag|=REFRESHEVENT_LIST;
					refresh=true;
				}
				break;
			}
		}
		break;

	case Undo::UID_DELETEPATTERN:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					win->refreshflag|=REFRESHEVENT_LIST;
					refresh=true;
				}
				break;
			}
		}
		break;

	case Undo::UID_REPLACEPLUGIN:
		{
			Undo_ReplacePlugin *rp=(Undo_ReplacePlugin *)function;

			switch(win->GetEditorID())
			{
			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *am=(Edit_AudioMix *)win;

					if(am->IsEffect(rp->effects)==true)
					{
						am->refresheffectsonly=true;
						refresh=true;
					}
				}
				break;
			}
		}
		break;

	case Undo::UID_CREATEPLUGIN:
		{
			Undo_CreatePlugin *cp=(Undo_CreatePlugin *)function;

			switch(win->GetEditorID())
			{
			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *am=(Edit_AudioMix *)win;

					if(am->IsEffect(cp->effects)==true)
					{
						am->refresheffectsonly=true;
						refresh=true;
					}
				}
				break;
			}
		}
		break;

	case Undo::UID_CREATEPLUGINS:
		{
			Undo_CreatePlugins *cps=(Undo_CreatePlugins *)function;

			switch(win->GetEditorID())
			{
			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *am=(Edit_AudioMix *)win;

					if(am->IsEffect(cps->effects)==true)
					{
						am->refresheffectsonly=true;
						refresh=true;
					}
				}
				break;
			}
		}
		break;

	case Undo::UID_DELETEPLUGIN:
		{
			Undo_DeletePlugin *dp=(Undo_DeletePlugin *)function;

			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_ARRANGELIST:
				{
					if(dp->automationtracks)
						refresh=true;
				}
				break;

			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *am=(Edit_AudioMix *)win;

					if(am->IsEffect(dp->effects)==true)
					{
						am->refresheffectsonly=true;
						refresh=true;
					}
				}
				break;
			}
		}
		break;

	case Undo::UID_DELETEPLUGINS:
		{
			Undo_DeletePlugins *dps=(Undo_DeletePlugins *)function;

			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_ARRANGELIST:
				{
					if(dps->automationtracks)
						refresh=true;
				}
				break;

			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *am=(Edit_AudioMix *)win;

					if(am->IsEffect(dps->effects)==true)
					{
						am->refresheffectsonly=true;
						refresh=true;
					}
				}
				break;
			}
		}
		break;

	case Undo::UID_DELETETRACK:
	case Undo::UID_DELETESELECTEDTRACKS:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_GROUP:
				refresh=true;
				break;

			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_ARRANGELIST:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					win->refreshflag|=REFRESHEVENT_LIST;
					refresh=true;
				}
				break;

			case EDITORTYPE_AUDIOMIXER:
				refresh=true; 
				break;
			}
		}
		break;

	case Undo::UID_CLONEPATTERN:
	case Undo::UID_COPYPATTERN:
	case Undo::UID_LOOPPATTERN:
	case Undo::UID_RECORDPATTERN:
	case Undo::UID_CREATENEWPATTERN:
	case Undo::UID_CONVERTLOOPPATTERN:
	case Undo::UID_CONVERTCLONEPATTERN:
	case Undo::UID_ADDLOOPSTOPATTERN:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
				{
					refresh=true;
				}
				break;

			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					win->refreshflag|=REFRESHEVENT_LIST;
					refresh=true;
				}
				break;
			}
		}
		break;

	case Undo::UID_EDITEVENTS:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				refresh=true;
				break;
			}
		}
		break;

	case Undo::UID_SIZEPATTERN:
	case Undo::UID_MOVEPATTERN:
	case Undo::UID_CREATENEWEVENTS:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_SAMPLE:
				{
					Edit_Sample *es=(Edit_Sample *)win;

					if(es->songmode==true)
						refresh=true;
				}
				break;

			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				win->refreshflag|=REFRESHEVENT_LIST;
				refresh=true;
				break;
			}
		}
		break;

	case Undo::UID_QUANTIZETRACK:
	case Undo::UID_QUANTIZEPATTERN:
	case Undo::UID_QUANTIZESTARTPATTERN:
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				win->refreshflag|=REFRESHEVENT_LIST;
				refresh=true;
				break;
			}
		}
		break;
	}

	return refresh;
}

void EditFunctions::CheckFunctionForGUI(Seq_Song *song,UndoFunction *function,bool checknext)
{	
	do
	{
		if(checknext==true)
		{
			if(UndoFunction *sub=function->FirstFunction())
				CheckFunctionForGUI(song,sub,true);
		}

		guiWindow *win=maingui->FirstWindow();

		while(win)
		{	
			if(win->closeit==false)
			{
				if(win->WindowSong()==song)
				{
					if(win->editrefresh==false)
						win->editrefresh=CheckIfWindowRefreshObjects(song,win,function);
				}
			}

			win=win->NextWindow();
		}

		if(checknext==true)
			function=function->NextFunction();
		else
			return;

	}while(function);
}

void EditFunctions::CheckEditElementsForGUI(Seq_Song *song,UndoFunction *function,bool undocheck)
{
	// Reset Refresh Flag
	guiWindow *win=maingui->FirstWindow();

	while(win)
	{
		win->refreshflag=0;

		if(win->closeit==false)
			win->CheckIfClose();

		win=win->NextWindow();
	}

	// Show Undo/Redo
	if(undocheck==true)
		maingui->RefreshUndoGUI(song);

	if(function)
	{	
		CheckFunctionForGUI(song,function,false); // Header Function

		if(UndoFunction *sub=function->FirstFunction())
			CheckFunctionForGUI(song,sub,true); // Sub Functions

		// Refresh selection lists
		win=maingui->FirstWindow();

		while(win)
		{
			if(win->closeit==false && win->editrefresh==false)
			{
				switch(win->GetEditorID())
				{
				case EDITORTYPE_PIANO:
				case EDITORTYPE_DRUM:
				case EDITORTYPE_WAVE:
				case EDITORTYPE_EVENT:
				case EDITORTYPE_SCORE:
					{	
						if(((EventEditor_Selection *)win)->patternselection.refresh==true)
							win->editrefresh=true;
					}
					break;
				}
			}

			win=win->NextWindow();
		}

		// Refresh GUI ------------------------------------
		win=maingui->FirstWindow();

		while(win)
		{
			if(win->closeit==false)
			{
				if(win->editrefresh==true && win->editrefreshdone==false)
				{
					win->editrefresh=false;
					win->RefreshObjects(0,true);
					win->refreshflag=0; // Reset Flag
				}

				win->editrefreshdone=false;
			}

			win=win->NextWindow();
		}
	}
}

bool EditFunctions::LoadMIDIFile(Seq_Track *track,char *readfile,OSTART mfposition,guiWindow *win)
{
	if(track && CheckIfEditOK(track->song)==true)
	{
		bool fileok=false,readfilestringcreated=false;

		if(!readfile) // Use FileRequester
		{
			camxFile soundfile;

			if(char *reqname=mainvar->GenerateString(Cxs[CXS_LOADSMFFILE],":",track->GetName()))
			{
				if(soundfile.OpenFileRequester(0,win,reqname,soundfile.AllFiles(camxFile::FT_MIDIFILE),true)==true)
				{
					if(readfile=mainvar->GenerateString(soundfile.filereqname))
						readfilestringcreated=true;
				}

				delete reqname;
			}
		}

		MIDIFile check;

		if(readfile && check.CheckFile(&check.file,readfile)==true)
		{
			if(UndoCreatePattern *ucp=new UndoCreatePattern)
			{
				ucp->mediatype=MEDIATYPE_MIDI;
				ucp->position=mfposition;
				ucp->track=track;

				if(Undo_CreatePattern *ufunction=new Undo_CreatePattern(track->song,ucp,1))
				{
					if(ufunction->MIDIfile=mainvar->GenerateString(readfile))
					{
						Lock(track->song);

						ufunction->Do();

						track->song->CheckCrossFades();

						if(MIDIPattern *newMIDI=(MIDIPattern *)ufunction->createpattern->newpattern)
						{
							newMIDI->SetName(readfile);
							fileok=true;
						}

						CheckPlaybackAndUnLock();	

						if(fileok==true)
						{
							track->song->undo.OpenUndoFunction(ufunction);
							CheckEditElementsForGUI(track->song,ufunction,true);
							// Refresh Manager
							//maingui->RefreshAllEditors(0,EDITORTYPE_AUDIOMANAGER,0);
						}
					}
					else
					{
						delete ufunction;
						delete ucp;
					}
				}
				else
					delete ucp;
			}
		}

		check.file.Close(true);	

		if(readfilestringcreated==true)
			delete readfile;

		return fileok;
	}

	return false;
}

bool EditFunctions::LoadPatternFile(Seq_Track *track,char *readfile,OSTART mfposition,guiWindow *win)
{
	if(track && CheckIfEditOK(track->song)==true)
	{
		bool fileok=false,readfilestringcreated=false;

		if(!readfile) // Use FileRequester
		{
			camxFile soundfile;

			if(soundfile.OpenFileRequester(0,win,Cxs[CXS_LOADPATTERN],"Pattern (*.pcmx)|*.pcmx;|All Files (*.*)|*.*||",true)==true)
			{
				if(readfile=mainvar->GenerateString(soundfile.filereqname))
					readfilestringcreated=true;
			}
		}

		if(readfile)
		{
			// Test file and get Pattern type
			camxFile test;

			if(test.OpenRead(readfile)==true)
			{
				char header[5];

				header[0]=
					header[4]=0;

				test.Read(header,4);

				if(strcmp("CAMP",header)==0)
				{
					int mediatype=0;

					char type[5];

					type[0]=type[4]=0;

					test.Read(type,4);

					if(strcmp("MIDI",type)==0)
						mediatype=MEDIATYPE_MIDI;
					else
					{
						if(strcmp("AUDI",type)==0)
							mediatype=MEDIATYPE_AUDIO;
					}

					if(mediatype)
					{
						if(UndoCreatePattern *ucp=new UndoCreatePattern)
						{
							ucp->mediatype=mediatype;
							ucp->position=mfposition;
							ucp->track=track;

							if(Undo_CreatePattern *ufunction=new Undo_CreatePattern(track->song,ucp,1))
							{
								if(ufunction->patternfile=mainvar->GenerateString(readfile))
								{
									Lock(track->song);
									ufunction->Do();
									track->song->CheckCrossFades();

									if(ufunction->createpattern->newpattern)
										fileok=true;

									CheckPlaybackAndUnLock();	

									if(fileok==true)
									{
										track->song->undo.OpenUndoFunction(ufunction);
										CheckEditElementsForGUI(track->song,ufunction,true);
									}
								}
								else
								{
									delete ufunction;
									delete ucp;
								}
							}
							else
								delete ucp;
						}//if ucp

					}//if mediatype
				}

				test.Close(true);

			}//if test
		}

		if(readfilestringcreated==true)
			delete readfile;

		return fileok;
	}

	return false;
}

bool EditFunctions::CheckIfEditOK(Seq_Song *song)
{
	if(song)
	{
		if(locked>0)
			return false;

		if(song->mastering==true)
			return false;

		return true;
	}

	return false;
}

void EditFunctions::Lock(Seq_Song *song)
{
#ifdef _DEBUG
	if(unlock)
		MessageBox(NULL,"Lock Song Error Double Lock","Error",MB_OK_STYLE);
#endif

	if(song==mainvar->GetActiveSong())
	{
		unlock=song;
		mainthreadcontrol->LockActiveSong();
	}
	else
		unlock=0;
}

void EditFunctions::UnLock()
{
#ifdef _DEBUG
	if(!unlock)
		MessageBox(NULL,"UnLock Song Error","Error",MB_OK_STYLE);
#endif

	if(unlock)
		mainthreadcontrol->UnlockActiveSong();	

	unlock=0;
}

void EditFunctions::CheckPlaybackAndUnLock()
{
	if(unlock)
	{
		unlock->CheckPlaybackRefresh();	
		mainthreadcontrol->UnlockActiveSong();	
	}

	unlock=0;
}

void EditFunctions::AddMIDIOutput(Seq_Track *track,int portindex)
{
	if(!track)
		return;


	bool ismetro=track->ismetrotrack;

	if(track->song)
		mainthreadcontrol->LockActiveSong();

	if(ismetro==true)
	{
		Seq_MetroTrack *t=track->song?track->song->FirstMetroTrack():(Seq_MetroTrack *)track;

		while(t)
		{
			if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
				t->GetMIDIOut()->AddToGroup(portindex);

			if(!t->song)
				break;

			t=t->NextMetroTrack();
		}
	}
	else
	{
		Seq_Track *t=track->song?track->song->FirstTrack():track;

		while(t)
		{
			if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
				t->GetMIDIOut()->AddToGroup(portindex);

			if(!t->song)
				break;

			t=t->NextTrack();
		}
	}

	if(track->song)
	{
		mainthreadcontrol->UnlockActiveSong();
		maingui->RefreshMIDI(track);
	}
}

void EditFunctions::ReplaceMIDIOutput(Seq_Track *track,int oldindex,int newindex)
{
	if(!track)
		return;

	bool ismetro=track->ismetrotrack;


	if(track->song)
		mainthreadcontrol->LockActiveSong();

	if(ismetro==true)
	{
		Seq_MetroTrack *t=track->song?track->song->FirstMetroTrack():(Seq_MetroTrack *)track;

		while(t)
		{
			if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
				t->GetMIDIOut()->Replace(oldindex,newindex);

			if(!t->song)
				break;

			t=t->NextMetroTrack();
		}
	}
	else
	{
		Seq_Track *t=track->song?track->song->FirstTrack():track;

		while(t)
		{
			if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
				t->GetMIDIOut()->Replace(oldindex,newindex);

			if(!t->song)
				break;

			t=t->NextTrack();
		}
	}

	if(track->song)
	{
		mainthreadcontrol->UnlockActiveSong();
		maingui->RefreshMIDI(track);
	}
}

void EditFunctions::RemoveMIDIOutput(Seq_Track *track,int portindex)
{
	if(!track)
		return;

	bool metro=track->ismetrotrack;


	if(track->song)
		mainthreadcontrol->LockActiveSong();

	if(metro==true)
	{
		Seq_MetroTrack *t=track->song?track->song->FirstMetroTrack():(Seq_MetroTrack *)track;

		while(t)
		{
			if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
				t->GetMIDIOut()->RemoveBusFromGroup(portindex);

			if(!t->song)
				break;

			t=t->NextMetroTrack();
		}
	}
	else
	{
		Seq_Track *t=track->song?track->song->FirstTrack():track;

		while(t)
		{
			if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
				t->GetMIDIOut()->RemoveBusFromGroup(portindex);

			if(!t->song)
				break;

			t=t->NextTrack();
		}
	}

	if(track->song)
	{
		mainthreadcontrol->UnlockActiveSong();
		maingui->RefreshMIDI(track);
	}
}

Undo_SetMIDIOut::Undo_SetMIDIOut(Seq_Track *t,bool sel)
{
	id=Undo::UID_SETMIDIOUT;

	oldbuffer=0;

	track=t;

	t->t_trackeffects.filter.Clone(&filter);
	t->GetMIDIOut()->CloneToGroup(&devicegroup);
	selected=sel;
	nrtracks=0;
}

void Undo_SetMIDIOut::DoUndo()
{
	if(oldbuffer)
	{
		for(int i=0;i<nrtracks;i++)
		{
			mainMIDIthruthread->DeleteAllThruOffsTrack(oldbuffer[i].track);
			oldbuffer[i].track->LockOpenOutputNotes();
			oldbuffer[i].track->SendAllOpenOutputNotes();

			oldbuffer[i].oldfilter.Clone(&oldbuffer[i].track->t_trackeffects.filter);
			oldbuffer[i].olddevices.CloneToGroup(oldbuffer[i].track->GetMIDIOut());

			oldbuffer[i].track->UnlockOpenOutputNotes();		
		}
	}
}

void Undo_SetMIDIOut::Do()
{
	if(!oldbuffer)
	{
		Seq_Track *t=song->FirstTrack();

		while(t)
		{
			if(track!=t && track->CompareMIDIOut(t)==false && (t->IsSelected()==true || selected==false))
				nrtracks++;

			t=t->NextTrack();
		}
	}

	if(nrtracks && (!oldbuffer))
	{
		if(oldbuffer=new UndoSetMIDIOutBuffer[nrtracks])
		{
			// Buffer old Track MIDI Devices
			int i=0;
			Seq_Track *t=song->FirstTrack();

			while(t){

				if(track!=t && track->CompareMIDIOut(t)==false && ((t->flag&OFLAG_SELECTED) || selected==false))
				{
					oldbuffer[i].track=t;
					t->GetMIDIOut()->CloneToGroup(&oldbuffer[i].olddevices);
					t->t_trackeffects.filter.Clone(&oldbuffer[i].oldfilter);
					i++;
				}

				t=t->NextTrack();
			}
		}
	}

	if(oldbuffer)
	{
		for(int i=0;i<nrtracks;i++)
		{
			mainMIDIthruthread->DeleteAllThruOffsTrack(oldbuffer[i].track);

			oldbuffer[i].track->LockOpenOutputNotes();
			oldbuffer[i].track->SendAllOpenOutputNotes();

			devicegroup.CloneToGroup(oldbuffer[i].track->GetMIDIOut());
			filter.Clone(&oldbuffer[i].track->t_trackeffects.filter);

			oldbuffer[i].track->UnlockOpenOutputNotes();
		}
	}
}

void Undo_SetMIDIOut::FreeData()
{
	if(oldbuffer)
	{
		for(int i=0;i<nrtracks;i++)
			oldbuffer[i].olddevices.Delete();

		delete oldbuffer;
	}

	devicegroup.Delete();
}

void Undo_SetMIDIOut::AddedToUndo()
{
	CreateUndoString(selected==true?Cxs[CXS_CHANGEALLTRACKSMIDIOUT_SEL]:Cxs[CXS_CHANGEALLTRACKSMIDIOUT]);
}

void EditFunctions::SetSongTracksToMIDIOutput(Seq_Track *track,bool selected)
{
	if(track)
	{
		if(track->song->CheckForOtherMIDI(track,true,selected)==true)
		{
			if(Undo_SetMIDIOut *usm=new Undo_SetMIDIOut(track,selected))
			{
				track->song->undo.OpenUndoFunction(usm);
				usm->Do();
				CheckEditElementsForGUI(track->song,usm,true);
			}
		}
		else
			maingui->MessageBoxOk(0,Cxs[CXS_NOCHANGES]);

	}
}

Undo_SetMIDIIn::Undo_SetMIDIIn(Seq_Track *t,bool sel)
{
	id=Undo::UID_SETMIDIIN;

	track=t;

	noMIDIinput=t->t_trackeffects.noMIDIinput;
	useallinputdevices=t->t_trackeffects.useallinputdevices;					
	usealwaysthru=t->t_trackeffects.usealwaysthru;
	userouting=t->t_trackeffects.userouting;

	t->GetMIDIIn()->CloneToGroup(&devicegroup);
	t->t_trackeffects.inputfilter.Clone(&inputfilter);

	oldbuffer=0;
	nrtracks=0;
	selected=sel;
}

void Undo_SetMIDIIn::DoUndo()
{
	if(oldbuffer)
	{
		for(int i=0;i<nrtracks;i++)
		{
			mainMIDIthruthread->DeleteAllThruOffsTrack(oldbuffer[i].track);
			oldbuffer[i].track->LockOpenOutputNotes();
			oldbuffer[i].track->SendAllOpenOutputNotes();

			oldbuffer[i].olddevices.CloneToGroup(oldbuffer[i].track->GetMIDIIn());
			oldbuffer[i].oldfilter.Clone(&oldbuffer[i].track->t_trackeffects.inputfilter);

			oldbuffer[i].track->t_trackeffects.noMIDIinput=oldbuffer[i].oldnoMIDIinput;
			oldbuffer[i].track->t_trackeffects.useallinputdevices=oldbuffer[i].olduseallinputdevices;
			oldbuffer[i].track->t_trackeffects.usealwaysthru=oldbuffer[i].oldusealwaysthru;
			oldbuffer[i].track->t_trackeffects.userouting=oldbuffer[i].olduserouting;

			oldbuffer[i].track->UnlockOpenOutputNotes();		
		}
	}
}

void Undo_SetMIDIIn::Do()
{
	if(!oldbuffer)
	{
		nrtracks=0;

		Seq_Track *t=song->FirstTrack();

		while(t){

			if(t!=track && ((t->flag&&OFLAG_SELECTED) || selected==false) && track->CompareMIDIIn(t)==false)
				nrtracks++;

			t=t->NextTrack();
		}
	}

	if(nrtracks && (!oldbuffer))
	{
		if(oldbuffer=new UndoSetMIDIInBuffer[nrtracks])
		{
			Seq_Track *t=song->FirstTrack();
			int i=0;

			while(t){

				if(t!=track && ((t->flag&&OFLAG_SELECTED) || selected==false) && track->CompareMIDIIn(t)==false)
				{
					// Buffer old

					oldbuffer[i].track=t;
					oldbuffer[i].oldnoMIDIinput=t->t_trackeffects.noMIDIinput;
					oldbuffer[i].olduseallinputdevices=t->t_trackeffects.useallinputdevices;
					oldbuffer[i].oldusealwaysthru=t->t_trackeffects.usealwaysthru;
					oldbuffer[i].olduserouting=t->t_trackeffects.userouting;

					t->GetMIDIIn()->CloneToGroup(&oldbuffer[i].olddevices);
					t->t_trackeffects.inputfilter.Clone(&oldbuffer[i].oldfilter);

					i++;
				}

				t=t->NextTrack();
			}
		}
	}

	if(oldbuffer)
	{
		for(int i=0;i<nrtracks;i++)
		{
			mainMIDIthruthread->DeleteAllThruOffsTrack(oldbuffer[i].track);

			oldbuffer[i].track->LockOpenOutputNotes();
			oldbuffer[i].track->SendAllOpenOutputNotes();

			devicegroup.CloneToGroup(oldbuffer[i].track->GetMIDIIn());
			inputfilter.Clone(&oldbuffer[i].track->t_trackeffects.inputfilter);

			oldbuffer[i].track->t_trackeffects.noMIDIinput=noMIDIinput;
			oldbuffer[i].track->t_trackeffects.useallinputdevices=useallinputdevices;
			oldbuffer[i].track->t_trackeffects.usealwaysthru=usealwaysthru;
			oldbuffer[i].track->t_trackeffects.userouting=userouting;

			oldbuffer[i].track->UnlockOpenOutputNotes();
		}
	}
}

void Undo_SetMIDIIn::FreeData()
{
	if(oldbuffer)
	{
		for(int i=0;i<nrtracks;i++)
			oldbuffer[i].olddevices.Delete();

		delete oldbuffer;
		oldbuffer=0;
		nrtracks=0;
	}

	devicegroup.Delete();
}

void Undo_SetMIDIIn::AddedToUndo()
{
	CreateUndoString(selected==true?Cxs[CXS_CHANGEALLTRACKSMIDIIN_SEL]:Cxs[CXS_CHANGEALLTRACKSMIDIIN]);
}

void EditFunctions::SetMIDIInputAllDevices(Seq_Track *track,bool onoff)
{
	if(!track)
		return;

	Seq_Track *t=track->song?track->song->FirstTrack():track;

	if(track->song)
		mainthreadcontrol->LockActiveSong();

	while(t)
	{
		if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
		{
			if(onoff==true)
			{
				t->GetFX()->useallinputdevices=true;
				t->GetFX()->noMIDIinput=false;
			}
			else
				t->GetFX()->useallinputdevices=false;
		}

		if(!t->song)
			break;

		t=t->NextTrack();
	}

	if(track->song)
	{
		mainthreadcontrol->UnlockActiveSong();
		maingui->RefreshMIDI(track);
	}
}

void EditFunctions::AddMIDIInput(Seq_Track *track,int index)
{
	if(!track)
		return;

	Seq_Track *t=track->song?track->song->FirstTrack():track;

	if(track->song)
		mainthreadcontrol->LockActiveSong();

	while(t)
	{
		if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
		{
			t->GetMIDIIn()->AddToGroup(index);
			t->GetFX()->useallinputdevices=false;
			t->GetFX()->noMIDIinput=false;
		}

		if(!t->song)
			break;

		t=t->NextTrack();
	}

	if(track->song)
	{
		mainthreadcontrol->UnlockActiveSong();
		maingui->RefreshMIDI(track);
	}
}

void EditFunctions::RemoveMIDIInput(Seq_Track *track,MIDIInputDevice *device)
{
	if(!track)
		return;

	Seq_Track *t=track->song?track->song->FirstTrack():track;

	if(track->song)
		mainthreadcontrol->LockActiveSong();

	while(t)
	{
		if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
			t->GetMIDIIn()->RemoveBusFromGroup(device);

		if(!t->song)
			break;

		t=t->NextTrack();
	}

	if(track->song)
	{
		mainthreadcontrol->UnlockActiveSong();
		maingui->RefreshMIDI(track);
	}
}

void EditFunctions::ReplaceMIDIInput(Seq_Track *track,int oldindex,int newindex)
{
	if(!track)
		return;

	Seq_Track *t=track->song?track->song->FirstTrack():track;

	if(track->song)
		mainthreadcontrol->LockActiveSong();

	while(t)
	{
		if(t==track || (track->IsSelected()==true && t->IsSelected()==true))
			t->GetMIDIIn()->Replace(oldindex,newindex);

		if(!t->song)
			break;

		t=t->NextTrack();
	}

	if(track->song)
	{
		mainthreadcontrol->UnlockActiveSong();
		maingui->RefreshMIDI(track);
	}
}

void EditFunctions::SetSongTracksToMIDIInput(Seq_Track *track,bool selected)
{
	if(track)
	{
		if(track->song->CheckForOtherMIDI(track,false,selected)==true)
		{
			if(Undo_SetMIDIIn *usm=new Undo_SetMIDIIn(track,selected))
			{
				track->song->undo.OpenUndoFunction(usm);
				usm->Do();
				CheckEditElementsForGUI(track->song,usm,true);
			}
		}
		else
			maingui->MessageBoxOk(0,Cxs[CXS_NOCHANGES]);
	}
}

void Undo_SplitMIDITrack::DoRedo()
{
	DeleteUndoFunction(false,true);
	Do();
}

void Undo_SplitMIDITrack::UndoGUI()
{
	// Delete Tracks
	for(int i=0;i<16;i++)
	{		
		if(newcreatedtrack[i])
			maingui->RemoveTrackFromGUI(newcreatedtrack[i]);
	}
}

void Undo_SplitMIDITrack::DoUndo()
{
	// Pattern -> oldpattern
	{
		UndoFunction *fp=LastFunction();
		while(fp)
		{
			fp->DoUndo();
			fp=fp->DeleteUndoFunction(true,true);
		}
	}

	// Delete Tracks
	for(int i=0;i<16;i++)
	{		
		if(newcreatedtrack[i])
		{
			song->DeleteTrack(newcreatedtrack[i],true); // Split+no lock
		}
	}		
}

void Undo_SplitMIDITrack::Do()
{
	int channels[16];
	MIDIPattern *p=(MIDIPattern *)track->FirstPattern(MEDIATYPE_MIDI);

	split=false;

	for(int i=0;i<16;i++)
		channels[i]=0;

	int c=0;

	while(p && c!=16)
	{
		if(p->CheckPatternChannel()==0) // different MIDI Channels ?
		{
			Seq_Event *e=p->FirstEvent();

			while(e)
			{
				if(e->CheckIfChannelMessage()==true)
				{	
					if(channels[e->GetChannel()]==0)
					{
						channels[e->GetChannel()]=1;
						c++;
					}

					split=true;
				}

				e=e->NextEvent();
			}
		}

		p=(MIDIPattern *)p->NextPattern(MEDIATYPE_MIDI);
	}

	if(split==true)
	{	
		int trackindex=song->GetOfTrack(track)+1;
		bool doit=false;

		for(int i=0;i<16;i++)
		{
			// Sort 1..2...5...16
			if(channels[i])
			{
				if(doit==true)
				{
					if(newcreatedtrack[i]=mainedit->CreateNewTrack(this,song,track,trackindex++,false,false))
					{
						if(char *newname=new char[strlen(track->GetName())+16])
						{
							char number[NUMBERSTRINGLEN];

							strcpy(newname,track->GetName());
							size_t slen=strlen(newname);

							if(char *npos=mainvar->ConvertIntToChar(i+1,number))
							{
								strcpy(&newname[slen],"_");
								slen++;
								strcpy(&newname[slen],npos);
								newcreatedtrack[i]->SetName(newname);
							}

							delete newname;

							track->CloneFx(newcreatedtrack[i]);
							newcreatedtrack[i]->GetFX()->SetChannel(i+1);

							MIDIPattern *p=(MIDIPattern *)track->FirstPattern(MEDIATYPE_MIDI);

							while(p)
							{
								if(p->CheckEventsWithChannel(i)==true)
								{
									if(Undo_SplitMIDIPattern *smp=new Undo_SplitMIDIPattern(song,p,i,newcreatedtrack[i]))
									{
										AddFunction(smp);
										smp->Do();
									}
								}

								p=(MIDIPattern *)p->NextPattern(MEDIATYPE_MIDI);
							}
						}
					}
				}
				else
					doit=true;
			}
		}// i channels

	}// split true ?	
}

bool EditFunctions::SplitTrackToChannels(Seq_Track *track)
{
	if(track && CheckIfEditOK(track->song)==true)
	{
		Seq_Pattern *p=track->FirstPattern(MEDIATYPE_MIDI);
		bool ok=false;

		while(p && ok==false) // Check MIDI Pattern
		{
			if(p->GetCountOfEvents()>0 && ((MIDIPattern *)p)->CheckPatternChannel()==0)
				ok=true;

			p=p->NextPattern(MEDIATYPE_MIDI);
		}

		if(ok==true)
		{
			if(Undo_SplitMIDITrack *redosplit=new Undo_SplitMIDITrack(track,0))
			{
				OpenLockAndDo(track->song,redosplit,true);
				return true;
			}
		}
	}

	return false;
}

void Undo_MoveTrack::Do()
{
	song->tracks.MoveSelectedObjects(&list,diff);
}

void Undo_MoveTrack::DoUndo()
{
	song->tracks.MoveObjects_Undo(&list);
}

void EditFunctions::MoveTracks(Seq_Song *song,int diff)
{
	if(song && song->tracks.CheckIfMoveSelected(diff)==true)
	{
		if(Undo_MoveTrack *umt=new Undo_MoveTrack(song,diff))
			OpenLockAndDo(song,umt,true);
	}
}

Seq_Track *EditFunctions::CreateNewTrack(UndoFunction *openuf,Seq_Song *song,Seq_Track *clone,int index,bool doundo,bool activate,bool createclonename)
{
	if(clone && clone->ismetrotrack==true)
		return 0;

	if(CheckIfEditOK(song)==true)
	{
		Seq_Track *newtrack = 0;

		// Clone FX
		if(clone)
		{
			if(index==-1)
				index=clone->GetTrackIndex()+1;

			if(newtrack = clone->Clone())
			{
				if(createclonename==true)
				{
					if(char *h=mainvar->GenerateString(clone->GetName()))
					{
						if(size_t sz=strlen(h))
						{
							sz--;

							char *e=&h[sz];
							switch(*e)
							{
							case '.':
								{
									*e=0;

									char *h2=mainvar->GenerateString(h,".1");
									delete h;
									h=h2;
								}
								break;

							case '0':
								*e='1';
								break;

							case '1':
								*e='2';
								break;

							case '2':
								*e='3';
								break;

							case '3':
								*e='4';
								break;

							case '4':
								*e='5';
								break;

							case '5':
								*e='6';
								break;

							case '6':
								*e='7';
								break;

							case '7':
								*e='8';
								break;

							case '8':
								*e='9';
								break;

							default:
								{
									delete h;
									h=mainvar->GenerateString(clone->GetName(),".1");
								}
								break;
							}
						}

						if(h)
						{
							Seq_Track *c=song->FirstTrack();
							while(c)
							{
								if(c->GetName() && strcmp(c->GetName(),h)==0)
								{
									delete h;
									h=mainvar->GenerateString(clone->GetName(),".");
									break;
								}

								c=c->NextTrack();
							}

							newtrack->SetName(h);
							delete h;
						}
					}
				}

				clone->CloneFx(newtrack);
				newtrack->parent=clone->parent;
				newtrack->childdepth=clone->childdepth;
			}
		}
		else
			newtrack=new Seq_Track;

		if(!newtrack)return 0;

		newtrack->InitDefaultAutomationTracks(newtrack,0);

		Seq_Track **tlist=new Seq_Track*[1];

		if(tlist)
		{
			tlist[0]=newtrack;

			if(Undo_CreateTracks *uct=new Undo_CreateTracks(song,tlist,1,index,0,clone,0))
			{
				uct->activate=activate;

				if(doundo==true)
				{
					if(!openuf)
					{
						song->undo.OpenUndoFunction(uct);
						Lock(song);
					}
					else
						openuf->AddFunction(uct);
				}

				uct->Do();

				if(doundo==true)
				{
					song->CreateQTrackList();

					if(!openuf){	
						UnLock();
						CheckEditElementsForGUI(song,uct,true);
					}
				}
				else
				{
					if(!clone)
						song->CreateQTrackList();

					uct->DeleteUndoFunction(true,true);
				}

				return newtrack;
			}
		}

		delete newtrack;
	}

	return 0;
}

Undo_CreateTracks::Undo_CreateTracks(Seq_Song *s,Seq_Track **tl,int tracknumber,int ix,Seq_Track *par,Seq_Track *cl,int f)
{
	id=Undo::UID_CREATENEWTRACKS;		
	song=s;
	// output
	tracks=tl;
	number=tracknumber;
	index=ix;
	parent=par;
	clonetrack=cl;
	flag=f;
	activate=false;

	for(int i=0;i<number;i++)
	{
		tracks[i]->song=song;

		//if(!parent)
		{
			tracks[i]->SetDefaultAudio(false,parent?parent:clonetrack);
		}

		// Track Number -> Name ---------------------------
		if(!tracks[i]->trackname)
			tracks[i]->SetName(Cxs[CXS_NEWTRACK]);
	}
}

/*
void Undo_CreateTracks::UndoGUI()
{
// Delete Tracks
if(tracks && number)
{
for(long i=0;i<number;i++)
maingui->RemoveTrackFromGUI(&tracks[i]);
}
}
*/

void Undo_CreateTracks::DoUndo()
{
	if(tracks)
	{
		for(int i=0;i<number;i++)
			DeleteTrack(tracks[i]);
	}

	song->tracks.Close();
}

void Undo_CreateTracks::FreeData()
{
	if(inundo==false)
	{
		if(tracks)
			for(int i=0;i<number;i++)
			{
				//	song->undo.CutUTrack(tracks[i]);
				tracks[i]->Delete(true);
			}
	}

	if(tracks)
		delete tracks;
}

void Undo_CreateTracks::RefreshPostUndo()
{
	if(tracks)
	{
		for(int i=0;i<number;i++)
			tracks[i]->RefreshDo();
	}
}

void Undo_CreateTracks::RefreshPreRedo()
{
	if(tracks)
	{
		for(int i=0;i<number;i++)
			tracks[i]->PreRefreshDo();
	}
}

void Undo_CreateTracks::Do()
{	
	int ix=index;
	OSTART position=song->GetSongPosition();

	for(int i=0;i<number;i++)
	{
		tracks[i]->song=song;

		song->AddTrack(tracks[i],ix++);

		// MIDI
		//if(!parent)
		{
			if(!tracks[i]->GetMIDIOut()->FirstDevice())
				tracks[i]->GetMIDIOut()->AddToGroup(mainMIDI->GetDefaultDevice());
		}

		if(tracks[i]==song->GetFocusTrack() && (song->status&Seq_Song::STATUS_RECORD))
			tracks[i]->SetRecordMode(true,position);

		if(activate==true)
		{
			song->SetFocusTrackNoGUIRefresh(tracks[i]);
		}
	}

	song->tracks.Close();
}

void Undo_AddTracksAsChild::Do()
{
	song->tracks.MoveSelectedObjectsToObject(&list,totrack);
}

void Undo_AddTracksAsChild::DoUndo()
{
	song->tracks.MoveObjects_Undo(&list);
}

Undo_AddTracksAsChild *EditFunctions::AddSelectedTrackToTrackAsChild(Seq_Song *song,Seq_Track *totrack,bool doit)
{
	if(totrack && song->tracks.CheckIfSelectedCanBeMovedTo(totrack)==true)
	{
		if(Undo_AddTracksAsChild *atac=new Undo_AddTracksAsChild(song,totrack))
		{
			if(doit==true)
				OpenLockAndDo(song,atac,true);

			return atac;
		}
	}

	return 0;
}

void Undo_RemoveTracksFromChild::Do()
{
	song->tracks.ReleaseSelectedChildObjects(&list,&song->tracks);
}

void Undo_RemoveTracksFromChild::DoUndo()
{
	song->tracks.MoveObjects_Undo(&list);
}

void EditFunctions::RemoveSelectedTrackFromParent(Seq_Song *song)
{
	int ct=0;

	Seq_Track *c=song->FirstTrack();
	while(c)
	{
		if(c->parent && (c->flag&OFLAG_SELECTED) )
			ct++;

		c=c->NextTrack();
	}

	if(ct)
	{
		if(Undo_RemoveTracksFromChild *atac=new Undo_RemoveTracksFromChild(song))
			OpenLockAndDo(song,atac,true);
	}
}

Seq_Track **EditFunctions::CreateNewChildTracks(Seq_Song *song,Seq_Track *prev,int number,int flag,Seq_Track *parent,Seq_Track *clone)
{
	if(prev && prev->ismetrotrack==true)
		return 0;

	if(parent)
	{
		if(number>0 && number<99 && CheckIfEditOK(song)==true)
		{
			if(Seq_Track **tracks=new Seq_Track*[number])
			{
				bool ok=true;

				for(int i=0;i<number;i++)
				{
					// Clone FX
					if(clone)
					{
						if(tracks[i] = clone->Clone())
							clone->CloneFx(tracks[i]);

						//TRACE ("Clone %s Parent %d\n",clone->GetName(),newtrack->parent);
					}
					else
					{
						tracks[i]=new Seq_Track;
					}

					if(!tracks[i])
					{
						ok=false;
						break;
					}
					else
					{
						tracks[i]->parent=parent;
						if(parent && (!clone))
							parent->CloneFx(tracks[i]);

						tracks[i]->InitDefaultAutomationTracks(tracks[i],0);
					}
				}

				if(ok==true)
				{
					int ix;

					if(clone && clone->song)
						ix=clone->GetTrackIndex()+1;
					else
						ix=0;

					if(Undo_CreateTracks *uct=new Undo_CreateTracks(song,tracks,number,ix,parent,clone,flag))
					{
						if(!(flag&EditFunctions::CREATETRACK_NODOLOCK))
						{
							OpenLockAndDo(song,uct,true);
						}
						else
							uct->Do();

						if(flag&EditFunctions::CREATETRACK_ACTIVATE)
						{
							if(uct->tracks)
								song->SetFocusTrack(uct->tracks[0]);
						}
					}
				}

				return tracks;
			}
		}
	}

	return 0;
}


int EditFunctions::CreateNewTracks(Seq_Song *song,Seq_Track *prev,int number,int flag,Seq_Track *clone,CreateNewTrackAudio *newaudiotracks)
{
	if(number>0 && number<500 && CheckIfEditOK(song)==true)
	{
		if(Seq_Track **tracks=new Seq_Track*[number])
		{
			bool ok=true,importstop=false;

			for(int i=0;i<number;i++)
			{
				tracks[i]=new Seq_Track;

				if(!tracks[i])
					ok=false;
				else
				{
					tracks[i]->song=song;

					if(newaudiotracks && newaudiotracks->list)
					{
						AudioHDFile *hd=(AudioHDFile *)newaudiotracks->list->GetO(i);

						if(hd)
						{
							tracks[i]->SetName(hd->GetName());

							tracks[i]->InitAudioIOWithNewAudioFile(song,hd);

							// Check Sample Rate
							if(importstop==false && hd->samplerate!=song->project->projectsamplerate)
							{
								if(maingui->MessageBoxYesNo(0,Cxs[CXS_Q_IMPORTSTOP])==false)
									importstop=true;
							}

							if(importstop==false)
								InitNewAudioPattern(0,tracks[i],hd->GetName(),newaudiotracks->position,false,0,0,0); // Create Pattern
						}
					}

					tracks[i]->InitDefaultAutomationTracks(tracks[i],0);
					tracks[i]->parent=prev?prev->parent:0;
				}
			}

			if(ok==true)
			{
				if(newaudiotracks)
				{
					newaudiotracks->ok=true;
					newaudiotracks->count=number;

					if(newaudiotracks->createtracklist==true)
					{
						newaudiotracks->tracks=new Seq_Track*[number];

						if(newaudiotracks->tracks)
						{
							for(int i=0;i<number;i++)
								newaudiotracks->tracks[i]=tracks[i];
						}

					}
				}

				int ix;

				if(clone /*&& (!parent)*/)
				{
					for(int i=0;i<number;i++)
						clone->CloneFx(tracks[i]);
				}

				if(prev)
					ix=prev->GetTrackIndex()+1;
				else
					ix=0;

				if(Undo_CreateTracks *uct=new Undo_CreateTracks(song,tracks,number,ix,0,clone,0))
					OpenLockAndDo(song,uct,true);
			}
			else
			{
				if(tracks)
					delete tracks;
			}

		}

		return number;
	}

	return 0;
}

Undo_CreateParent::Undo_CreateParent(Seq_Song *s,Seq_Track *nt,Seq_Track *clone)
{
	id=Undo::UID_CREATEPARENTTRACK;

	done=false;
	song=s;
	nexttrack=nt;
	clonetrack=clone;
	newtrack=0;

	s->tracks.CreateSelectedList(&sellist);
	s->tracks.CreateList(&list); // Buffer old
}

void Undo_CreateParent::Do()
{
	if(!newtrack)
	{
		if(newtrack=new Seq_Track)
		{
			if(clonetrack)
			{
				clonetrack->CloneToTrack(newtrack,false);

				if(char *h=mainvar->GenerateString("F:",clonetrack->GetName()))
				{
					newtrack->SetName(h);
					delete h;
				}
			}
			else
				newtrack->SetName("Folder");
		}
	}

	if(newtrack)
	{
		done=true;
		int index=nexttrack?nexttrack->GetTrackIndex():0;
		song->AddTrack(newtrack,index);
		song->tracks.MoveObjectsToObject(&sellist,newtrack);
	}
}

void Undo_CreateParent::DoUndo()
{
	if(done==true)
	{
		done=false;

		if(newtrack)
			newtrack->song->DeleteTrack(newtrack,false);

		song->tracks.MoveObjects_Undo(&list);
	}
}

void Undo_CreateParent::FreeData()
{
	if(inundo==false)
	{
		if(newtrack)
		{
			//	song->undo.CutUTrack(newtrack);
			newtrack->Delete(true);
		}
	}

	sellist.FreeMemory();
	list.FreeMemory();
}

void EditFunctions::CreateNewParentAndAddSelectedTracks(Seq_Song *song,Seq_Track *next,Seq_Track *clone)
{
	if(!next)return;

	if(Undo_CreateParent *uct=new Undo_CreateParent(song,next,clone))
		OpenLockAndDo(song,uct,true);
}

void Undo_PastePattern::Do()
{
	if(UndoFunction *uf=FirstFunction())
		uf->DoRedo();
}

void Undo_PastePattern::UndoGUI()
{
	maingui->RemovePatternFromGUI(song,newpattern);	
}

void Undo_PastePattern::DoUndo()
{
	if(UndoFunction *uf=FirstFunction())
		uf->DoUndo();
}

UndoFunction *EditFunctions::PasteObjectToTrack(Seq_Track *totrack,Object *object,OSTART position,int flag)
{
	if(totrack && CheckIfEditOK(totrack->song)==true)
	{
		switch(object->id)
		{
		case OBJ_TRACK:
			{
				// Track -> Track
				Seq_Track *fromtrack=(Seq_Track *)object;

				if(fromtrack->FirstPattern(MEDIATYPE_ALL))
				{
					if(Undo_PasteTrack *pt=new Undo_PasteTrack(totrack))
					{
						bool lock=false;

						if(!(flag&EditBuffer::PASTE_NOOPENUNDO))
						{
							totrack->song->undo.OpenUndoFunction(pt);

							if(!(flag&EditBuffer::PASTE_NOSYSTEMLOCK))
							{
								Lock(totrack->song);
								lock=true;
							}
						}

						// Copy Pattern
						int flag=EditBuffer::PASTE_NOSYSTEMLOCK|EditBuffer::PASTE_NOOPENUNDO|EditBuffer::PASTE_NOPLAYBACKCHECK|EditBuffer::PASTE_NOGUIREFRESH;
						Seq_Pattern *p=fromtrack->FirstPattern(MEDIATYPE_ALL);

						while(p)
						{
							UndoFunction *ifunc=PasteObjectToTrack(totrack,p,p->GetPatternStart(),flag); // no new undo
							pt->AddFunction(ifunc);
							p=p->NextPattern(MEDIATYPE_ALL);
						}

						if(lock==true) // Redo ?
						{
							CheckPlaybackAndUnLock();
							CheckEditElementsForGUI(totrack->song,pt,true);
						}

						return pt;
					}
				}
			}
			break;

		case OBJ_AUDIOPATTERN:
			{
				AudioPattern *ap=(AudioPattern *)object;

				if(ap->audioevent.audioefile)
				{
					AudioPattern *mainclonepattern=(AudioPattern *)ap->mainclonepattern;

					if(Undo_CreatePattern *ucp=mainedit->InitNewAudioPattern(0,totrack,ap->audioevent.audioefile->GetName(),position,(!(flag&EditBuffer::PASTE_NOSYSTEMLOCK))?true:false,ap,mainclonepattern))
					{		
						if(!(flag&EditBuffer::PASTE_NOOPENUNDO))
							totrack->song->undo.OpenUndoFunction(ucp);

						if(!(flag&EditBuffer::PASTE_NOPLAYBACKCHECK))
							CheckPlaybackAndUnLock();

						if(!(flag&EditBuffer::PASTE_NOGUIREFRESH))
							CheckEditElementsForGUI(totrack->song,ucp,true);

						return ucp;
					}
				}
			}
			break;

		case OBJ_MIDIPattern:
			{
				Seq_Pattern *pattern=(Seq_Pattern *)object;

				if(UndoCreatePattern *np=new UndoCreatePattern)
				{
					np->mediatype=pattern->mediatype;
					np->track=totrack;
					np->position=position;
					np->mainclonepattern=pattern->mainclonepattern;

					if(Undo_CreatePattern *ucp=new Undo_CreatePattern(totrack->song,np,1))
					{
						if(!(flag&EditBuffer::PASTE_NOOPENUNDO))
							totrack->song->undo.OpenUndoFunction(ucp);

						if(!(flag&EditBuffer::PASTE_NOSYSTEMLOCK))
							Lock(totrack->song);

						ucp->Do();

						if(np->newpattern && (!np->mainclonepattern))
						{
							pattern->Clone(totrack->song,np->newpattern,position-pattern->GetPatternStart(),0);
							np->newpattern->RefreshAfterPaste();
							totrack->AddEffectsToPattern(np->newpattern); // New Track Effects
						}

						if(!(flag&EditBuffer::PASTE_NOPLAYBACKCHECK))
							CheckPlaybackAndUnLock();
						else
							if(!(flag&EditBuffer::PASTE_NOSYSTEMLOCK))
								UnLock();

						if(!(flag&EditBuffer::PASTE_NOGUIREFRESH))
							CheckEditElementsForGUI(totrack->song,ucp,true);

						return ucp;
					}

					delete np;
				}	
			}
			break;
		}
	}

	return 0;
}

void EditFunctions::PasteSelectionListToSong(Seq_SelectionList *list,Seq_Song *song,Seq_Track *firsttrack,OSTART position)
{
	if(CheckIfEditOK(song)==true)
	{
		int mediatype=0;

		if(firsttrack)
		{
			if(Seq_SelectedPattern *selp=list->FirstSelectedPattern())
			{
				int index=song->GetOfTrack(firsttrack),firstindex=-1;
				OSTART firststartposition=-1;

				// find first TrackIndex
				while(selp)
				{
					mediatype|=selp->pattern->mediatype;

					if(firstindex==-1 || selp->tracknumberinsong<firstindex)
						firstindex=selp->tracknumberinsong;

					TRACE ("PasteSelectionListToSong Start %d\n",selp->pattern->GetPatternStart());

					if(firststartposition==-1 || selp->pattern->GetPatternStart()<firststartposition)
						firststartposition=selp->pattern->GetPatternStart();

					selp=selp->NextSelectedPattern();
				}

				if(firstindex>-1)
				{
					if(Undo_PastePatternBuffer *pp=new Undo_PastePatternBuffer(mediatype))
					{
						selp=list->FirstSelectedPattern();
						int c=0;

						Lock(song);

						while(selp)
						{		
							// TrackPosition
							if(Seq_Track *totrack=song->GetTrackIndex((selp->tracknumberinsong-firstindex)+index)) // Track
							{
								if(totrack->IsEditAble()==true)
								{
									OSTART diff=selp->pattern->GetPatternStart()-firststartposition;

									if(selp->pattern->mainclonepattern)
									{
										if((!song->FindPattern(selp->pattern->mainclonepattern)) || selp->pattern->mainclonepatternID!=selp->pattern->mainclonepattern->patternID)
											selp->pattern->mainclonepattern=0;
									}

									switch(selp->pattern->mediatype)
									{
									case MEDIATYPE_AUDIO:
										{
											AudioPattern *ap=(AudioPattern *)selp->pattern;

											if(ap->audioevent.audioefile)
											{
												if(Undo_CreatePattern *ucp=mainedit->InitNewAudioPattern(0,totrack,ap->audioevent.audioefile->GetName(),position+diff,false,ap,(AudioPattern *)ap->mainclonepattern))
												{		
													c++;
													pp->AddFunction(ucp);
												}

												mediatype |=MEDIATYPE_AUDIO;
											}
										}
										break;

									default:
										if(Seq_Pattern *np=CreateNewPattern(pp,totrack,selp->pattern->mediatype,position+diff,true,CNP_NOGUIREFRESH,0,selp->pattern->mainclonepattern))
										{
											if(!np->mainclonepattern)
											{
												selp->pattern->Clone(totrack->song,np,(position+diff)-selp->pattern->GetPatternStart(),0);
												np->RefreshAfterPaste();
												totrack->AddEffectsToPattern(np); // New Track Effects
											}

											c++;
										}
										break;
									}
								}
							}

							selp=selp->NextSelectedPattern();
						}// while selp

						if(c)
						{
							song->undo.OpenUndoFunction(pp);

							CheckPlaybackAndUnLock();

							if(mediatype)
								CheckEditElementsForGUI(song,pp,true);
						}
						else
						{
							delete pp;
							UnLock();
						}
					}
				}

			} // if selp
		}
	}
}

void Undo_QuantizeSelectedPattern::Do()
{
	for(int i=0;i<listcounter;i++)
	{
		OSTART diff=list[i].newsimplestart-list[i].oldstart;
		list[i].pattern->MovePattern(diff,Seq_Pattern::MOVENO_STATIC);
	}
}

void Undo_QuantizeSelectedPattern::DoUndo()
{
	for(int i=0;i<listcounter;i++)
	{
		OSTART diff=list[i].oldstart-list[i].newsimplestart;
		list[i].pattern->MovePattern(diff,Seq_Pattern::MOVENO_STATIC);
	}
}

bool Undo_QuantizeSelectedPattern::RefreshUndo()
{
	for(int i=0;i<listcounter;i++)
	{
		if(song->FindPattern(list[i].pattern)==false)
			return false;

		if(CheckPattern(list[i].pattern)==false)
			return false;
	}

	return true;
}

void EditFunctions::QuantizePattern_Execute(Seq_Song *song,Seq_SelectionList *list,int notelength) // called by menu
{
	bool qok=false;
	int c=0;
	Seq_SelectedPattern *sp=list->FirstSelectedPattern();

	while(sp)
	{
		OSTART pos=sp->pattern->GetPatternStart();
		Seq_Signature *sig=song->timetrack.FindSignatureBefore(pos);
		OSTART measure=song->timetrack.ConvertTicksToMeasure(pos);
		OSTART sigq=sig->measurelength*(measure-1);
		OSTART qpos=sigq+mainvar->SimpleQuantize(pos-sigq,quantlist[notelength]);

		if(qpos!=pos)
			c++;

		sp=sp->NextSelectedPattern();
	}

	if(c)
	{
		if(Undo_QuantizeSelP *ql=new Undo_QuantizeSelP[c])
		{
			Undo_QuantizeSelectedPattern *selp=new Undo_QuantizeSelectedPattern(ql,c,quantlist[notelength]);

			if(selp)
			{
				int ic=0;
				Seq_SelectedPattern *sp=list->FirstSelectedPattern();

				while(sp)
				{
					OSTART pos=sp->pattern->GetPatternStart();
					Seq_Signature *sig=song->timetrack.FindSignatureBefore(pos);
					OSTART measure=song->timetrack.ConvertTicksToMeasure(pos);
					OSTART sigq=sig->measurelength*(measure-1);
					OSTART qpos=sigq+mainvar->SimpleQuantize(pos-sigq,quantlist[notelength]);

					if(qpos!=pos)
					{
						ql[ic].oldstart=pos;
						ql[ic].newsimplestart=qpos;
						ql[ic++].pattern=sp->pattern;
					}

					sp=sp->NextSelectedPattern();
				}

				OpenLockAndDo(song,selp,true);
			}
			else
				delete ql;
		}
	}
}

void EditFunctions::QuantizePatternMenu(guiWindow *win,Seq_SelectionList *plist,bool force)
{
	if(win && win->WindowSong() && plist && plist->FirstSelectedPattern())
	{
		TRACE ("QuantizePattern ...");

		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_qpattern:public guiMenu
			{
			public:
				menu_qpattern(Seq_Song *s,Seq_SelectionList *l,int qt)
				{
					song=s;
					list=l;
					qtx=qt;
				}

				void MenuFunction()
				{
					mainedit->QuantizePattern_Execute(song,list,qtx);
				} //

				Seq_Song *song;
				Seq_SelectionList *list;
				int qtx;
			};

			TRACE ("QP Menu \n");

			win->skipdeletepopmenu=true;

			win->popmenu->AddMenu(Cxs[CXS_QUANTSELPATTERN],0);

			for(int m=0;m<QUANTNUMBER;m++)
				win->popmenu->AddFMenu(quantstr[m],new menu_qpattern(win->WindowSong(),plist,m));

			win->ShowPopMenu();
		}
	}
}

void EditFunctions::CreateMixBuffer(Seq_SelectionList *plist) // Mix selected MIDI Pattern->Buffer
{
	if(plist && plist->FirstSelectedPattern())
	{
		bool MIDIdatafound=false;
		Seq_SelectedPattern *sp=plist->FirstSelectedPattern();

		while(sp)
		{
			if(sp->pattern->mediatype==MEDIATYPE_MIDI){
				MIDIPattern *mp=(MIDIPattern *)sp->pattern;

				if(mp->FirstEvent())
				{
					MIDIdatafound=true;
					break;
				}
			}

			sp=sp->NextSelectedPattern();
		}

		if(MIDIdatafound==true)
		{
			if(MIDIPattern *nMIDIp=new MIDIPattern){

				nMIDIp->SetName("MIDI Mix Pattern");

				mainbuffer->OpenBuffer();
				mainbuffer->AddObjectToBuffer(nMIDIp,false); // no clone !

				Seq_SelectedPattern *sp=plist->FirstSelectedPattern();

				while(sp)
				{
					if(sp->pattern->mediatype==MEDIATYPE_MIDI){
						MIDIPattern *mp=(MIDIPattern *)sp->pattern;
						mp->CloneMixEventsToPattern(plist->song,nMIDIp);
					}

					sp=sp->NextSelectedPattern();
				}

				//mainbuffer->AddObjectToBuffer(&patternselection);
				mainbuffer->CloseBuffer();
			}
		}
	}
}

// Quantize Pattern ***************************************************************
Undo_QuantizePattern::Undo_QuantizePattern(Seq_Pattern *p,QuantizeEffect *old,QuantizeEffect *newfx,bool peffect)
{
	id=Undo::UID_QUANTIZEPATTERN;
	pattern=p;
	patterneffect=peffect;
	old->Clone(&oldeffects);
	newfx->Clone(&neweffects);
	changedpositions=0;
	oldpatternposition=p->GetPatternStart();
}

bool Undo_QuantizePattern::RefreshUndo()
{
	if(song->FindPattern(pattern)==false)
		return false;

	return CheckPattern(pattern);
}

void Undo_QuantizePattern::Do()
{
	if(patterneffect==true)
		neweffects.Clone(&pattern->quantizeeffect);

	changedpositions=pattern->QuantizePattern(&neweffects);

	pattern->CloseEvents();
}

void Undo_QuantizePattern::DoUndo()
{
	if(patterneffect==true)
		oldeffects.Clone(&pattern->quantizeeffect);

	if(pattern->SetStart(oldpatternposition)==false) // Audio/Video=true
		changedpositions=pattern->QuantizePattern(&oldeffects);
	else
		changedpositions=1;

	pattern->CloseEvents();
}

// Quantize Track    ***************************************************************
Undo_QuantizeTrack::Undo_QuantizeTrack(Seq_Track *t,QuantizeEffect *backup,QuantizeEffect *newq)
{
	id=Undo::UID_QUANTIZETRACK;
	track=t;
	mediatypes=t->GetMediaTypes();
	newq->Clone(&neweffects);
	backup->Clone(&backupeffects);
	changedpositions=0;
}

void Undo_QuantizeTrack::Do()
{
	neweffects.Clone(&track->GetFX()->quantizeeffect); // New Effect->Track

	Seq_Pattern *p=track->FirstPattern(MEDIATYPE_ALL);
	while(p)
	{
		if(Undo_QuantizePattern *qp=new Undo_QuantizePattern(p,&backupeffects,&neweffects,false))
		{
			qp->Do();

			if(qp->changedpositions)
			{
				changedpositions+=qp->changedpositions;
				AddFunction(qp);
			}
			else
				qp->DeleteUndoFunction(true,true);
		}

		p=p->NextPattern(MEDIATYPE_ALL);
	}
}

void Undo_QuantizeTrack::DoUndo()
{
	backupeffects.Clone(&track->GetFX()->quantizeeffect);

	UndoFunction *uf=FirstFunction();
	while(uf)
	{
		uf->DoUndo();
		uf=uf->DeleteUndoFunction(true,true);
	}
}

void EditFunctions::QuantizeTrack(Seq_Track *track,QuantizeEffect *qeff,bool force)
{
	if(track && CheckIfEditOK(track->song)==true)
	{
		if(force==true || qeff->Compare(&track->GetFX()->quantizeeffect)==false)
		{
			bool nochanges=false,addnewundo=true;
			Undo_QuantizeTrack *qt=0;

			UndoFunction *last=track->song->undo.LastUndo();

			if(last && last->id==Undo::UID_QUANTIZETRACK)
			{
				qt=(Undo_QuantizeTrack *)last;

				if(qt->track==track)
				{
					if(qt->neweffects.Compare(qeff)==false)
					{	
						qeff->Clone(&qt->neweffects);
						addnewundo=false;
					}
					else
						nochanges=true;
				}		
			}

			if(nochanges==false)
			{
				if(addnewundo==true)
					qt=new Undo_QuantizeTrack(track,&track->GetFX()->quantizeeffect,qeff);

				if(qt)
				{
					if(addnewundo==true)
						track->song->undo.OpenUndoFunction(qt);

					Lock(track->song);
					qt->Do();

					if(qt->changedpositions)
					{
						CheckPlaybackAndUnLock();
						CheckEditElementsForGUI(track->song,qt,true);
					}
					else
						UnLock();
				}
			}
		}
	}
}

void EditFunctions::QuantizePattern(Seq_Pattern *p,QuantizeEffect *qeff,bool force)
{	
	if(p && CheckIfEditOK(p->track->song)==true)
	{
		if(p && (force==true || qeff->Compare(&p->quantizeeffect)==true))
		{
			bool nochanges=false,addnewundo=true;
			Undo_QuantizePattern *qp=0;
			UndoFunction *last=p->track->song->undo.LastUndo();

			if(last && last->id==Undo::UID_QUANTIZEPATTERN)
			{
				qp=(Undo_QuantizePattern *)last;

				if(qp->pattern==p)
				{
					if(qp->neweffects.Compare(qeff)==false)
					{	
						qeff->Clone(&qp->neweffects);
						addnewundo=false;
					}
					else
						nochanges=true;
				}		
			}

			if(nochanges==false)
			{
				if(addnewundo==true)
					qp=new Undo_QuantizePattern(p,&p->quantizeeffect,qeff,true);

				if(qp)
				{
					Lock(p->track->song);
					qp->Do();

					if(qp->changedpositions)
					{
						p->track->song->CheckPlaybackRefresh();	
						UnLock();

						if(addnewundo==true)
							p->track->song->undo.OpenUndoFunction(qp);

						CheckEditElementsForGUI(p->track->song,qp,true);
					}
					else
						UnLock();
				}
			}
		}
	}
}


bool Undo_ClonePattern::RefreshUndo()
{
	UndoCopyPattern *pp=movepattern;
	for(int i=0;i<numberofpattern;i++)
	{		
		if(inundo==true)
		{
			if(CheckPattern(pp->clone)==false)
				return false;
		}
		else
		{
			if(CheckPattern(pp->pattern)==false)
				return false;
		}

		pp++;
	}

	return true;
}

void Undo_ClonePattern::DoUndo()
{	
	UndoCopyPattern *pp=movepattern;
	int c=numberofpattern;

	while(c--)
	{		
		if(pp->clone)
		{
			mainMIDI->StopAllofPattern(pp->clone->track->song,pp->clone);
			maingui->RemovePatternFromGUI(song,pp->clone);

			pp->clone->GetTrack()->checkcrossfade=true;
			pp->clone->GetTrack()->DeletePattern(pp->clone,true);
		}

		pp++;
	}
}

void Undo_ClonePattern::Do()
{
	UndoCopyPattern *pp=movepattern;
	int c=numberofpattern;

	while(c--)
	{
		OSTART qposition=pp->pattern->GetPatternStart()+movediff;
		Seq_Track *totrack;

		if(trackindex) // move up or down ?
		{
			int ix=song->GetOfTrack(pp->totrack);
			totrack=song->GetTrackIndex(ix+trackindex);
		}
		else
			totrack=pp->totrack;

		if(totrack)
		{
			if(pp->clone=pp->pattern->CreateLoopPattern(0,qposition,0))
			{	
				pp->pattern->AddClone(pp->clone);
				totrack->AddSortPattern(pp->clone,qposition);
				//pp->pattern->SetClonesName();
				pp->pattern->SetClonesOffset();
				pp->clone->RefreshAfterPaste();

				totrack->checkcrossfade=true;
				// No Tracks Effects !
			}
		}

		pp++;
	}
}

bool EditFunctions::ClonePatternList(MoveO *mo)
{
	if(mo->sellist && mo->sellist->FirstSelectedPattern() && (mo->diff || mo->index))
	{
		Seq_SelectedPattern *selp=mo->sellist->FirstSelectedPattern();
		UndoCopyPattern *pp,*patternpointer=0;

		int c=mo->sellist->GetCountOfSelectedPattern();

		if(c && (pp=patternpointer=new UndoCopyPattern[c]))

			while(selp){

				pp->pattern=selp->pattern->itsaclone==true?selp->pattern->mainpattern:selp->pattern;
				pp->totrack=pp->pattern->GetTrack();
				pp->onlyclone=true;
				pp++;
				selp=selp->NextSelectedPattern();
			}

			if(patternpointer)
			{
				if(Undo_ClonePattern *ump=new Undo_ClonePattern(mo->song,mo->track,patternpointer,c,mo->diff,mo->index,mo->flag))
					OpenLockAndDo(mo->song,ump,true);
			}
	}

	return false;
}


void Undo_CopyPattern::Do()
{
	UndoCopyPattern *pp=movepattern;

	for(int c=0;c<numberofpattern;c++){

		Seq_Track *totrack;

		if(trackindex) // move up or down ?
		{
			int ix=song->GetOfTrack(pp->totrack);
			totrack=song->GetTrackIndex(ix+trackindex);
		}
		else
			totrack=pp->totrack;

		if(totrack && totrack->IsEditAble()==true)
		{
			if(pp->pattern->itsaclone==true)
			{
				OSTART qposition=pp->pattern->GetPatternStart()+movediff;
				Seq_Pattern *mainpattern=pp->pattern->mainpattern;

				if(!pp->clone)
					pp->clone=mainpattern->CreateLoopPattern(0,qposition,0);

				if(pp->clone)
				{	
					mainpattern->AddClone(pp->clone);
					totrack->AddSortPattern(pp->clone,qposition);
					totrack->checkcrossfade=true;
					//mainpattern->SetClonesName();
					mainpattern->SetClonesOffset();
					pp->clone->RefreshAfterPaste();
					// No Tracks Effects !
				}
			}
			else
			{
				if(!pp->clone)
				{
					pp->qposition=pp->pattern->GetPatternStart()+movediff+pp->pattern->offset;
					pp->clone=pp->pattern->CreateClone(movediff+pp->pattern->offset,0);
				}

				if(pp->clone)
				{
					totrack->AddSortPattern(pp->clone,pp->qposition);
					pp->clone->RefreshAfterPaste();
					totrack->AddEffectsToPattern(pp->clone); // New Track Effects
					totrack->checkcrossfade=true;
				}
			}
		}

		pp++;
	}
}

void Undo_CopyPattern::DoUndo()
{	
	UndoCopyPattern *pp=movepattern;

	for(int i=0;i<numberofpattern;i++)
	{		
		if(pp->clone)
		{
			mainMIDI->StopAllofPattern(song,pp->clone);
			//	maingui->RemovePatternFromGUI(pp->clone);

			pp->clone->GetTrack()->checkcrossfade=true;
			pp->clone->GetTrack()->DeletePattern(pp->clone,false);
		}

		pp++;
	}
}

void Undo_CopyPattern::UndoGUI()
{
	for(int i=0;i<numberofpattern;i++)
		maingui->RemovePatternFromGUI(song,movepattern[i].clone,false);
}

void Undo_CopyPattern::FreeData()
{
	if(movepattern)
	{
		if(inundo==false)
		{
			for(int i=0;i<numberofpattern;i++)
			{
				if(movepattern[i].clone)
					movepattern[i].clone->Delete(true);
			}
		}

		delete movepattern;
	}
}

bool Undo_CopyPattern::RefreshUndo()
{
	UndoCopyPattern *pp=movepattern;

	for(int i=0;i<numberofpattern;i++)
	{		
		if(inundo==true){
			if(CheckPattern(pp->clone)==false)
				return false;
		}
		else{
			if(CheckPattern(pp->pattern)==false)
				return false;
		}

		pp++;
	}

	return true;
}

bool EditFunctions::CopyPatternList(MoveO *mo)
{
	if(mo->sellist && mo->sellist->FirstSelectedPattern() && (mo->diff || mo->index))
	{
		Seq_SelectedPattern *selp=mo->sellist->FirstSelectedPattern();
		UndoCopyPattern *pp,*patternpointer=0;

		int c=mo->sellist->GetCountOfSelectedPattern();

		if(c){

			if(pp=patternpointer=new UndoCopyPattern[c])

				while(selp){
					pp->pattern=selp->pattern;
					pp->totrack=selp->pattern->GetTrack();

					pp++;
					selp=selp->NextSelectedPattern();
				}
		}

		if(patternpointer){
			if(Undo_CopyPattern *ump=new Undo_CopyPattern(mo->song,mo->track,patternpointer,c,mo->diff,mo->index,mo->flag))
				OpenLockAndDo(mo->song,ump,true);
		}
	}

	return false;
}