#ifndef CAMX_WAVEEDITOR_MENUH
#define CAMX_WAVEEDITOR_MENUH 1

#include "wavemap.h"

class menu_waveeditor_selectmap:public guiMenu
{
public:
	menu_waveeditor_selectmap(Edit_Wave *e,WaveMap *map) // track or NULL
	{
		ed=e;
		smap=map;
	}
	
	void MenuFunction()
	{	
		ed->SelectMap(smap);
	}
	
	Edit_Wave *ed;
	WaveMap *smap;
};

class menu_waveeditor_createnewmap:public guiMenu
{
public:
	menu_waveeditor_createnewmap(Edit_Wave *e) // track or NULL
	{
		ed=e;
	}
	
	void MenuFunction()
	{	
		WaveMap *mp=mainwavemap->AddWaveMap();
		
		if(mp)
		{
			ed->SelectMap(mp);
			
			guiWindow *w=maingui->FirstWindow();
			
			while(w)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_WAVE:
					{
						Edit_Wave *wave=(Edit_Wave *)w;
						
						if(wave!=ed)
						{
							wave->RefreshMenu();
						}
					}
					break;
				}
				
				w=w->NextWindow();
			}
			
		}	
	}
	
	Edit_Wave *ed;
};

class menu_waveeditor_deletemap:public guiMenu
{
public:
	menu_waveeditor_deletemap(Edit_Wave *e) // track or NULL
	{
		ed=e;
	}
	
	void MenuFunction()
	{	
		if(ed->wavedefinition)
		{
			WaveMap *old=ed->wavedefinition;
			WaveMap *mp=ed->wavedefinition->NextMap();
			
			if(!mp)
				mp=ed->wavedefinition->PrevMap();
			
			mainwavemap->DeleteWaveMap(ed->wavedefinition);
			
			guiWindow *w=maingui->FirstWindow();
			
			while(w)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_WAVE:
					{
						Edit_Wave *wave=(Edit_Wave *)w;
						
						if(wave->wavedefinition==old)
							wave->SelectMap(mp);
						else
							wave->RefreshMenu();
					}
					break;
				}
				
				w=w->NextWindow();
			}
			
			// maingui->RefreshAllEditors(ed->WindowSong(),EDITORTYPE_WAVE,1); // 1=Full refresh
		}	
	}
	
	Edit_Wave *ed;
};

class menu_waveeditor_createnewtrack:public guiMenu
{
public:
	menu_waveeditor_createnewtrack(Edit_Wave *e,WaveTrack *t) // track or NULL
	{
		ed=e;
		track=t;
	}
	
	void MenuFunction()
	{	
		ed->CreateNewTrack(track);
	}
	
	Edit_Wave *ed;
	WaveTrack *track;
};

class menu_waveeditor_deletetrack:public guiMenu
{
public:
	menu_waveeditor_deletetrack(Edit_Wave *e,WaveTrack *t) // track or NULL
	{
		ed=e;
		track=t;
	}
	
	void MenuFunction()
	{	
		ed->DeleteTrack(track);
	}
	
	Edit_Wave *ed;
	WaveTrack *track;
};

#endif