#include "songmain.h"
#include "editor.h"

#include "camxgadgets.h"
#include "editbuffer.h"

#include "imagesdefines.h"
#include "arrangeeditor.h"
#include "groove.h"

#include "gui.h"
#include "editor_help.h"
#include "pianoeditor.h"
#include "globalmenus.h"
#include "object_song.h"

enum
{
	GADGETID_PIANOEDITOR,
	GADGETID_DRUMEDITOR ,
	GADGETID_EVENTEDITOR,
	GADGETID_WAVEEDITOR,
	GADGETID_SAMPLEEDITOR,
	GADGETID_TEMPOEDITOR,

	GADGETID_TRACKAUDIOMIXER,
	GADGETID_AUDIOMIXER,
	GADGETID_MASTERING,
	GADGETID_FREEZE,
	GADGETID_GROOVEEDITOR,
	GADGETID_GROUPEDITOR ,
	GADGETID_SCOREEDITOR,
	GADGETID_AUDIOMANAGER,
	GADGETID_CPUHD,
	GADGETID_UNDO,
	GADGETID_REDO,
	GADGETID_UNDOSTRING,
	GADGETID_REDOSTRING,
	GADGETID_LIBRARY,
	GADGETID_GENERATOR
};

void Edit_Toolbox::Gadget(guiGadget *g)
{
	Seq_Song *song=fromscreen->GetSong();

	switch(g->gadgetID)
	{
	case GADGETID_TEMPOEDITOR:
		{
			globmenu_TMap teditor(fromscreen);
			teditor.MenuFunction();
		}
		break;

	case GADGETID_UNDO:
		{
			if(song && song->undo.undo_menustring)
				song->undo.DoUndo();
		}
		break;

	case GADGETID_REDO:
		{
			if(song && song->undo.redo_menustring)
				song->undo.DoRedo();
		}
		break;

	case GADGETID_GENERATOR:
		{
			guiWindow *win=maingui->FindWindow(EDITORTYPE_RMG,song,0);

			if(!win)
				maingui->OpenEditorStart(EDITORTYPE_RMG,song,0,0,0,0,0);
			else
			{
				win->WindowToFront(true);
			}
		}
		break;

	case GADGETID_MASTERING:
			if(song)
			{
				guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOMASTER,song,0);

				if(!win)
					maingui->OpenEditorStart(EDITORTYPE_AUDIOMASTER,song,0,0,0,0,0);
				else
					win->WindowToFront(true);
			}
		break;

	case GADGETID_FREEZE:
		if(song)
		{
			guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOFREEZE,song,0);

			if(!win)
				maingui->OpenEditorStart(EDITORTYPE_AUDIOFREEZE,song,0,0,0,0,0);
			else
				win->WindowToFront(true);

		}break;

	case GADGETID_CPUHD:
		{
		guiWindow *win=maingui->FindWindow(EDITORTYPE_CPU,0,0);

			if(win)
				win->WindowToFront(true);
			else
				maingui->OpenEditorStart(EDITORTYPE_CPU,0,0,0,0,0,0);
		}
		break;

		/*
	case GADGETID_AUDIOMASTER:
		{
			if(song)
				maingui->OpenEditorStart(EDITORTYPE_AUDIOMIXER,song,0,0,0,(Object *)true,0);
		}
		break;
*/

	case GADGETID_AUDIOMANAGER:
		{
			guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOMANAGER,0,0);

			if(win)
				win->WindowToFront(true);
			else
				maingui->OpenEditorStart(EDITORTYPE_AUDIOMANAGER,0,0,0,0,0,0);
		}
		break;

	case GADGETID_GROUPEDITOR:
		maingui->OpenEditorStart(EDITORTYPE_GROUP,song,0,0,0,0,0);
		break;
		
	case GADGETID_GROOVEEDITOR:
		maingui->OpenEditorStart(EDITORTYPE_GROOVE,song,0,0,0,0,0);
		break;

	case GADGETID_AUDIOMIXER:
		if(song)
		{
			guiWindowSetting setting;
			setting.screen=fromscreen;

			maingui->OpenEditorStart(EDITORTYPE_AUDIOMIXER,song,0,(Seq_SelectionList *)1,&setting,0,0);
		}
		break;

		/*
	case GADGETID_ARRANGEEDITOR:
		if(song)
		{
			maingui->OpenEditorStart(EDITORTYPE_ARRANGE,song,0,0,0,0,0);
		}
		break;
*/

	case GADGETID_PIANOEDITOR:
	case GADGETID_DRUMEDITOR:
	case GADGETID_EVENTEDITOR:
	case GADGETID_SAMPLEEDITOR:
		if(song)
		{
			Edit_Arrange *ar=fromscreen->GetArrange();

			if(ar)
			{
				switch(g->gadgetID)
				{
				case GADGETID_EVENTEDITOR:
					{
						globmenu_Event editor(ar,ar->startposition);
						editor.MenuFunction();
					}
					break;

				case GADGETID_PIANOEDITOR:
					{
						globmenu_Piano editor(ar,ar->startposition);
						editor.MenuFunction();
					}
					break;

				case GADGETID_DRUMEDITOR:
					{
						globmenu_Drum editor(ar,ar->startposition);
						editor.MenuFunction();
					}
					break;

				case GADGETID_SAMPLEEDITOR:
					{
						globmenu_Sample editor(ar,ar->startposition);
						editor.MenuFunction();
					}break;
				}
			}
		}
		break;
	}
}

Edit_Toolbox::Edit_Toolbox()
{
	editorid=EDITORTYPE_TOOLBOX;
	InitForms(FORM_PLAIN1x1);
	resizeable=true;
	ondesktop=true;
	dialogstyle=true;

	minwidth=maingui->GetButtonSizeY(7);
	maxwidth=maingui->GetButtonSizeY(14);
	maxheight=minheight=maingui->GetButtonSizeY(16);
}

void Edit_Toolbox::RefreshRealtime_Slow()
{
	Seq_Song *song=fromscreen->GetSong();

	if(song)
	{
		if(song->undo.undo_menustring)
		{
			glist.Enable(undo,true);

			if(undo)
				undo->ChangeButtonText(song->undo.undo_menustring);
		}
		else
		{
			glist.Disable(undo);
			if(undo)
				undo->ChangeButtonText("");
		}

		if(song->undo.redo_menustring)
		{
			glist.Enable(redo,true);

			if(redo)
				redo->ChangeButtonText(song->undo.redo_menustring);
		}
		else
		{
			glist.Disable(redo);
			if(redo)
				redo->ChangeButtonText("");
		}
	}
	else
	{
		glist.Disable(undo);
		glist.Disable(redo);

		if(undo)
			undo->ChangeButtonText("");

		if(redo)
			redo->ChangeButtonText("");
	}
}

void Edit_Toolbox::Init()
{
	glist.SelectForm(0,0);

//	glist.AddButton(-1,-1,-1,-1,"Arrange",GADGETID_ARRANGEEDITOR,MODE_RIGHT|MODE_TEXTCENTER);
//	glist.Return();
	glist.AddButton(-1,-1,-1,-1,"Event",GADGETID_EVENTEDITOR,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();
	glist.AddButton(-1,-1,-1,-1,"Piano",GADGETID_PIANOEDITOR,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();
	glist.AddButton(-1,-1,-1,-1,"Drum",GADGETID_DRUMEDITOR,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();
	glist.AddButton(-1,-1,-1,-1,"Sample",GADGETID_SAMPLEEDITOR,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();
	glist.AddButton(-1,-1,-1,-1,"TempoMap",GADGETID_TEMPOEDITOR,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"Mixer",GADGETID_AUDIOMIXER,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();
	glist.AddButton(-1,-1,-1,-1,"Audio Manager",GADGETID_AUDIOMANAGER,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"Mastering/Bounce",GADGETID_MASTERING,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();
	glist.AddButton(-1,-1,-1,-1,"Freeze/UnFreeze",GADGETID_FREEZE,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"CPU/HD",GADGETID_CPUHD,MODE_RIGHT|MODE_TEXTCENTER);
	glist.Return();

	undo=glist.AddButton(-1,-1,-1,2*maingui->GetButtonSizeY(),"",GADGETID_UNDO,MODE_RIGHT);
	glist.Return();

	redo=glist.AddButton(-1,-1,-1,2*maingui->GetButtonSizeY(),"",GADGETID_REDO,MODE_RIGHT);
	glist.Return();

	RefreshRealtime_Slow();
			
/*
			if(y+addy>=height)return;
			gl->AddButton(0,y,width,y+addy-1,"Wave",GADGETID_WAVEEDITOR,0);
			y+=addy;
*/			
			
			/*
			if(y+addy>=height)return;
			gl->AddButton(0,y,width,y+addy-1,"Score",GADGETID_SCOREEDITOR,0);
			y+=addy;
*/

#ifdef OLDIE
			
			
		
			
			if(y+addy>=height)return;
			gl->AddButton(0,y,width,y+addy-1,"Library",GADGETID_LIBRARY,0);
			y+=addy;

			if(y+addy>=height)return;
			gl->AddButton(0,y,width,y+addy-1,"Groove",GADGETID_GROOVEEDITOR,0);
			y+=addy;

			if(y+addy>=height)return;
			gl->AddButton(0,y,width,y+addy-1,"Group",GADGETID_GROUPEDITOR,0);

			y+=addy;
			gl->AddButton(0,y,width,y+addy-1,"Generator",GADGETID_GENERATOR,0);
		}
#endif
}
