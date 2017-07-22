
#include "editor.h"
#include "gui.h"
#include "camxgadgets.h"
#include "imagesdefines.h"

#include "MIDIhardware.h"
#include "player.h"
#include "languagefiles.h"
#include "object_project.h"
#include "object_song.h"
#include "semapores.h"
#include "songmain.h"
#include "editdata.h"

#define PLAYER_START MENU_ID_START+50	

// Menus
#define PLAYER_ADDPLAYER PLAYER_START+10

// Gadgets ---
#define PLAYERGADGETID_START GADGET_ID_START+50

#define GADGET_PLAYERID PLAYERGADGETID_START+1
#define GADGET_PLAYERSTART PLAYERGADGETID_START+2
#define GADGET_PLAYERSTOP PLAYERGADGETID_START+3

#define GADGET_PROJECTS PLAYERGADGETID_START+4
#define GADGET_SONGS PLAYERGADGETID_START+5

EditData * Edit_Player::EditDataMessage(EditData *data)
{	
	if(data)
	{
		switch(data->id)
		{
		default:

			break;
		}
	}

	return 0;
}

guiMenu *Edit_Player::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		guiMenu *n=menu->AddMenu(Cxs[CXS_FILE],0);
		if(n)
		{
			class menu_addpro:public guiMenu
			{
			public:
				menu_addpro(Edit_Player *p)
				{
					player=p;
				}

				void MenuFunction()
				{	
				}

				Edit_Player *player;
			};

			n->AddFMenu(Cxs[CXS_ADDPROJECT],new menu_addpro(this));
		}

		// Piano Editor Menu
		options=n=menu->AddMenu("Player",0);

		if(n)
		{	
			class menu_looppro:public guiMenu
			{
			public:
				menu_looppro(Edit_Player *p)
				{
					player=p;
				}

				void MenuFunction()
				{
					if(mainsettings->player_loopsongs==true)
						mainsettings->player_loopsongs=false;
					else
						mainsettings->player_loopsongs=true;

					player->ShowMenu();
				} //

				Edit_Player *player;
			};

			n->AddFMenu("Loop Projects",new menu_looppro(this),mainsettings->player_loopprojects);

			class menu_loop:public guiMenu
			{
			public:
				menu_loop(Edit_Player *p)
				{
					player=p;
				}

				void MenuFunction()
				{
					if(mainsettings->player_loopsongs==true)
						mainsettings->player_loopsongs=false;
					else
						mainsettings->player_loopsongs=true;

					player->ShowMenu();
				} //

				Edit_Player *player;
			};

			n->AddFMenu("Loop Songs",new menu_loop(this),mainsettings->player_loopsongs);
		}
	}

	return menu;
}

void Edit_Player::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_PROJECTS:
		{
			Seq_Project *pro=mainvar->GetProjectIndex(g->index);

			if(pro && pro!=activeproject)
				SetActiveProject(pro);
		}
		break;

	case GADGET_PLAYERSTART:
		if(activeproject && activesong)
		{
			Start(activesong);
		}
		break;

	case GADGET_PLAYERSTOP:
		Stop();
		break;

	case GADGET_SONGS:
		if(activeproject)
		{
			Seq_Song *s=activeproject->GetSongIndex(g->index);

			if(s && s!=activesong)
			{
				SetActiveSong(s,false);
			}
		}
		break;
	}
}

void Edit_Player::FreeMemory()
{
	if(winmode&WINDOWMODE_INIT)
	{
	//	gadgetlists.RemoveAllGadgetLists();
	}
}

void Edit_Player::InitGadgets()
{
#ifdef OLDIE
	ResetGadgets();

	if(glist=gadgetlists.AddGadgetList(this))
	{
		int y=height-maingui->GetFontSizeY();
		int x=0;

		y/=2;
		y-=1;

		int hy=0;

		projects=glist->AddListBox(0,hy,width,hy+y,GADGET_PROJECTS,0,"Player Projects");
		hy=y+1;

		songs=glist->AddListBox(0,hy,width,hy+y,GADGET_SONGS,0,"Project Songs");

		y=height-maingui->GetFontSizeY();

		int w=width/2;
		start=glist->AddButton(x,y,x+w,height,"Start",GADGET_PLAYERSTART,0,"Start");
		x+=w+1;
		stop=glist->AddButton(x,y,x+w,height,"Stop",GADGET_PLAYERSTOP,0,"Stop");
	}
#endif

}

void Edit_Player::RedrawGfx()
{

}

void Edit_Player::MouseMove(bool inside)
{

}

void Edit_Player::MouseButton(int flag)
{

}

void Edit_Player::RefreshRealtime()
{
	if(mainvar->GetActiveProject() && mainvar->GetActiveSong())
	{
		cMIDIPlaybackEvent nmpbe(INITPLAY_MIDITRIGGER);

		mainvar->GetActiveSong()->GetNextMIDIPlaybackEvent(&nmpbe);

		if(playerplayback==true && nmpbe.playbackevent==0)
		{
			Seq_Song *song=mainvar->GetActiveSong()->NextSong();

			if(song)
			{
				Start(song);
			}
			else
				if(mainsettings->player_loopsongs==true && mainvar->GetActiveProject()->FirstSong())
				{
					Start(mainvar->GetActiveProject()->FirstSong());
				}
				else
					playerplayback=false;
		}
	}
	else
		playerplayback=false;
}

void Edit_Player::Start(Seq_Song *song)
{
	if(song)
	{
		if(mainvar->GetActiveSong())
			song->StopSong(0,song->GetSongPosition());

		mainvar->SetActiveSong(song);

		mainthreadcontrol->LockActiveSong();
		song->SetSongPosition(0,false);
		song->PlaySong();
		mainthreadcontrol->UnlockActiveSong();

		playerplayback=true;
	}
}

void Edit_Player::Stop()
{
	if(playerplayback==true)
	{
		playerplayback=false;

		if(activesong)
			activesong->StopSelected();
	}
}

void Edit_Player::ShowMenu()
{
	if(options)
	{
		options->Select(0,mainsettings->player_loopprojects);
		options->Select(1,mainsettings->player_loopsongs);
	}
}

void Edit_Player::Init()
{
	FreeMemory();

	if(width && height)
	{			
		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
		{
			InitGadgets();

			ShowProjects();
			ShowSongs();
		}
	}
}

void Edit_Player::ShowProjects()
{
	if(!activeproject)activeproject=mainvar->GetActiveProject();

	if(activeproject)
	{
		if(projects)
		{
			projects->ClearListBox();
			projects->Enable();

			Seq_Project *p=mainvar->FirstProject();
			int i=0;

			while(p)
			{
				if(p->underdestruction==false)
				{
					projects->AddStringToListBox(p->name);

					if(p==activeproject)
						projects->SetListBoxSelection(i);

					i++;
				}

				p=p->NextProject();
			}
		}
	}
	else
	{
		if(projects)
		{
			projects->ClearListBox();
			projects->Disable();
		}
	}

}

void Edit_Player::ShowSongs()
{
	bool nosong=true;

	if(activeproject)
	{
		if(!activesong)activesong=activeproject->FirstSong();

		if(activesong && songs)
		{
			songs->ClearListBox();

			Seq_Song *s=activeproject->FirstSong();
			int i=0;

			while(s)
			{
				if(s->underdeconstruction==false)
				{
					songs->AddStringToListBox(s->songname);

					if(s==activesong)
						songs->SetListBoxSelection(i);

					i++;
				}
				s=s->NextSong();

			}

			nosong=false;
		}
	}
	else activesong=0;

	if(!activesong)
	{
		if(start)start->Disable();
		if(stop)stop->Disable();
	}
	else
	{
		if(start)start->Enable();
		if(stop)stop->Enable();
	}

	if(nosong==true)
	{
		if(songs)
		{
			songs->ClearListBox();
			songs->Disable();
		}
	}
	else
		if(songs)
			songs->Enable();
}

void Edit_Player::SetActiveProject(Seq_Project *pro)
{
	activeproject=pro;
	activesong=pro->FirstSong();

	mainvar->SetActiveProject(pro,pro->FirstSong());

	ShowSongs();
}

void Edit_Player::SetActiveSong(Seq_Song *s,bool showsongs)
{
	activesong=s;

	if(showsongs==true)
		ShowSongs();
}
