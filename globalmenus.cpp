#include "globalmenus.h"
#include "sampleeditor.h"
#include "audiopattern.h"
#include "editor.h"
#include "editbuffer.h"
#include "editfunctions.h"
#include "gui.h"
#include "object_track.h"
#include "object_song.h"
#include "songmain.h"
#include "languagefiles.h"

void globmenu_Event::MenuFunction()
{
	guiWindowSetting settings;
	settings.screen=editor->screen;
	settings.calledfromwindow=editor;

	maingui->OpenEditorStart(EDITORTYPE_EVENT,editor->WindowSong(),0,editor->GetPatternSelection(),&settings,0,startposition);
}

void globmenu_Piano::MenuFunction()
{
	guiWindowSetting settings;
	settings.screen=editor->screen;
	settings.calledfromwindow=editor;

	maingui->OpenEditorStart(EDITORTYPE_PIANO,editor->WindowSong(),0,editor->GetPatternSelection(),&settings,0,startposition);
}

void globmenu_Drum::MenuFunction()
{
	guiWindowSetting settings;
	settings.screen=editor->screen;
	settings.calledfromwindow=editor;
	
	maingui->OpenEditorStart(EDITORTYPE_DRUM,editor->WindowSong(),0,editor->GetPatternSelection(),&settings,0,startposition);
}

void globmenu_Score::MenuFunction()
{
	guiWindowSetting settings;
	settings.screen=editor->screen;
	settings.calledfromwindow=editor;

	LONGLONG startpos=editor->GetMousePosition()>=0?editor->GetMousePosition():editor->startposition;

	maingui->OpenEditorStart(EDITORTYPE_SCORE,editor->WindowSong(),0,editor->GetPatternSelection(),&settings,0,startpos);
}

void globmenu_Wave::MenuFunction()
{
	guiWindowSetting settings;
	settings.screen=editor->screen;
	settings.calledfromwindow=editor;

	LONGLONG startpos=editor->GetMousePosition()>=0?editor->GetMousePosition():editor->startposition;
	maingui->OpenEditorStart(EDITORTYPE_WAVE,editor->WindowSong(),0,editor->GetPatternSelection(),&settings,0,startpos);
}

void globmenu_Arrange::MenuFunction()
{
	if(song)
	maingui->OpenNewScreen(song->project,song);
}

void globmenu_Sample::MenuFunction()
{
	AudioPattern *pattern=0;
	Seq_SelectionList *list=editor->GetPatternSelection();

	if(list)
	{
		Seq_SelectedPattern *selp=list->FirstSelectedPattern();
		while(selp)
		{
			if(selp->pattern->mediatype==MEDIATYPE_AUDIO)
			{					
				pattern=(AudioPattern *)selp->pattern;
				break;
			}
			selp=selp->NextSelectedPattern();
		}

		if(pattern)
		{
			guiWindowSetting settings;

			settings.calledfromwindow=editor;

			Edit_Sample_StartInit startinit;

			startinit.pattern=pattern;
			startinit.file=pattern->audioevent.audioefile;
			startinit.startposition=startposition;

			if(pattern->audioevent.audioregion)
			{
				startinit.rangestart=pattern->audioevent.audioregion->regionstart;
				startinit.rangeend=pattern->audioevent.audioregion->regionend;
			}

			maingui->OpenEditorStart(EDITORTYPE_SAMPLE,editor->WindowSong(),0,0,&settings,(Object *)&startinit,0);
		}
	}
}

void globmenu_Toolbox::MenuFunction()
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{	
		if(w->GetEditorID()==EDITORTYPE_TOOLBOX && w->WindowSong()==screen->GetSong())
		{
			w->WindowToFront(true);
			return;
		}

		w=w->NextWindow();
	}
	
	guiWindowSetting set;

	set.screen=screen;

	maingui->OpenEditorStart(EDITORTYPE_TOOLBOX,0,0,0,&set,0,0);
}

void globmenu_cpu::MenuFunction()
{
	guiWindow *w=maingui->FirstWindow();

	while(w && w->GetEditorID()!=EDITORTYPE_CPU)
	{	
		w=w->NextWindow();
	}

	if(w)
		w->WindowToFront(true);
	else
	maingui->OpenEditorStart(EDITORTYPE_CPU,0,0,0,0,0,0);	
}

void globmenu_TMap::MenuFunction()
{
	guiWindowSetting settings;
	settings.screen=screen;
	settings.calledfromwindow=screen->FindWindow(EDITORTYPE_ARRANGE);

	if(screen && screen->GetSong())
		maingui->OpenEditorStart(EDITORTYPE_TEMPO,screen->GetSong(),0,0,&settings,0,0);
}

void globmenu_TEd::MenuFunction()
{
	Seq_Song *s;

	if((s=song)==0)
		s=mainvar->GetActiveSong();

	if(s)
		maingui->OpenEditorStart(EDITORTYPE_TEXT,s,0,0,0,0,0);
}

void globmenu_Marker::MenuFunction()
{
	if(Seq_Song *s=(song==0)?mainvar->GetActiveSong():song)
		maingui->OpenEditorStart(EDITORTYPE_MARKER,s,0,0,0,0,0);
}

void globmenu_AudioMixer::MenuFunction()
{
	Seq_Song *s=song?song:mainvar->GetActiveSong();

	if(s)
	{
		guiWindowSetting settings;
		settings.screen=screen;

		maingui->OpenEditorStart(EDITORTYPE_AUDIOMIXER,s,starttrack,(Seq_SelectionList *)1,&settings,0,0);
	}
}

void globmenu_Editor::MenuFunction()
{
	screen->Editor();
}

void globmenu_AManager::MenuFunction()
{
	guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOMANAGER,0,0);

	if(win)
		win->WindowToFront(true);
	else
		maingui->OpenEditorStart(EDITORTYPE_AUDIOMANAGER,0,0,0,0,0,0);
}

void globmenu_Keyboard::MenuFunction(){maingui->OpenEditorStart(EDITORTYPE_KEYBOARD,0,0,0,0,0,0);}
void globmenu_Bigtime::MenuFunction(){maingui->OpenEditorStart(EDITORTYPE_BIGTIME,0,0,0,0,0,0);}

void globmenu_cnewtrack::MenuFunction()
{
	if(!track)
		track=song->GetFocusTrack();

	song->UnSelectTracksFromTo(song->FirstTrack(),song->LastTrack());

	if(track && track->parent)
		mainedit->CreateNewChildTracks(song,track,1,EditFunctions::CREATETRACK_ACTIVATE,(Seq_Track *)track->parent,track);
	else
		mainedit->CreateNewTrack(0,song,track,-1,true,true);
}

char *globmenu_cnewtrack::GetMenuName(){return Cxs[CXS_CREATENEWTRACK];}

void globmenu_AMaster::MenuFunction()
{
	if(freeze==true)
	{
		guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOFREEZE,mainvar->GetActiveSong(),0);

		if(!win)
			maingui->OpenEditorStart(EDITORTYPE_AUDIOFREEZE,mainvar->GetActiveSong(),0,0,0,0,0);
		else
			win->WindowToFront(true);

		return;
	}

	guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOMASTER,mainvar->GetActiveSong(),0);

	if(!win)
		maingui->OpenEditorStart(EDITORTYPE_AUDIOMASTER,mainvar->GetActiveSong(),0,0,0,0,0);
	else
		win->WindowToFront(true);
}

void globmenu_RMG::MenuFunction()
{
	guiWindow *win=maingui->FindWindow(EDITORTYPE_RMG,mainvar->GetActiveSong(),0);

	if(!win)
		maingui->OpenEditorStart(EDITORTYPE_RMG,mainvar->GetActiveSong(),0,0,0,0,0);
	else
		win->WindowToFront(true);
}