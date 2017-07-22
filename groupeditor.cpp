#include "songmain.h"
#include "editor.h"
#include "gui.h"
#include "camxgadgets.h"
#include "imagesdefines.h"
#include "MIDIhardware.h"
#include "groupeditor.h"
#include "arrangeeditor.h"
#include "languagefiles.h"
#include "object_song.h"
#include "object_track.h"
#include "editdata.h"
#include "chunks.h"
#include "camxfile.h"

#define GROUP_START MENU_ID_START+50	

// Menus
#define GROUP_ADDGROUP GROUP_START+10

// Gadgets ---
#define GROUPGADGETID_START GADGET_ID_START+50

#define GADGET_GROUPID GROUPGADGETID_START+1
#define GADGET_GROUPDIFFID GROUPGADGETID_START+2
#define GADGET_GROUPNAME GROUPGADGETID_START+3
#define GADGET_GROUPINFO GROUPGADGETID_START+4
#define FX_GROUPSOLO_ID GROUPGADGETID_START+28
#define FX_GROUPMUTE_ID GROUPGADGETID_START+29
#define FX_GROUPREC_ID GROUPGADGETID_START+30
#define FX_GROUPCOLOUR_ID GROUPGADGETID_START+31

void Edit_Group::ShowActiveGroupColour()
{
#ifdef OLDIE
	if(colour_x!=-1)
	{
		bitmap.guiFillRect(colour_x,colour_y,colour_x2,colour_y2,COLOUR_BACKGROUND);

		if(activegroup)
		{
			if(activegroup->colour.showcolour==true)
			{
				bitmap.guiFillRect_RGB(colour_x,colour_y,colour_x2,colour_y2,activegroup->colour.rgb);
			}
			else
			{
				bitmap.guiDrawRect(colour_x,colour_y,colour_x2,colour_y2,COLOUR_WHITE);
				bitmap.guiDrawLine(colour_x,colour_y,colour_x2,colour_y2,COLOUR_BLACK);
				bitmap.guiDrawLine(colour_x,colour_y2,colour_x2,colour_y,COLOUR_BLACK);
			}
		}
	}
#endif
}

void Edit_Group::ShowActiveGroupStatus()
{
	if(activegroup)
	{
		if(group_colour)
			group_colour->Enable();

		status_group_mute=activegroup->mute;

		if(group_mute)
		{
			if(status_group_mute==true)
				group_mute->ChangeButtonImage(IMAGE_TRACKMUTEON);
			else
				group_mute->ChangeButtonImage(IMAGE_TRACKMUTEOFF);

			group_mute->Enable();
		}

		status_group_solo=activegroup->solo;

		if(group_solo)
		{
			if(status_group_solo==true)
				group_solo->ChangeButtonImage(IMAGE_TRACKSOLOON);
			else
				group_solo->ChangeButtonImage(IMAGE_TRACKSOLOOFF);

			group_solo->Enable();
		}

		status_group_rec=activegroup->rec;
		if(group_rec)
		{
			if(status_group_rec==true)
				group_rec->ChangeButtonImage(IMAGE_TRACKRECORDON);
			else
				group_rec->ChangeButtonImage(IMAGE_TRACKRECORDOFF);

			group_rec->Enable();
		}
	}
	else
	{
		if(group_mute)
			group_mute->Disable();

		if(group_solo)
			group_solo->Disable();

		if(group_rec)
			group_rec->Disable();

		if(group_colour)
			group_colour->Disable();
	}
}

void Edit_Group::DeleteAllTrackObjects()
{
	Edit_Group_Tracks *egt=FirstGroupTrack();

	while(egt)
	{
		if(egt->name)
			delete egt->name;

		egt=egt->NextGroupTrack();
	}

	tracks.DeleteAllO();
}

void Edit_Group::ShowActiveGroup()
{
	DeleteAllTrackObjects();

	// Raster
	if(activegroup)
	{
		if(groupname)
		{
			groupname->Enable();
			groupname->SetString(activegroup->name);
		}

		if(groupgadget)
			groupgadget->Enable();

		if(grouptracks)
		{
			grouptracks->ClearListBox();
			grouptracks->Enable();

			char h2[NUMBERSTRINGLEN];

			Seq_Track *t=WindowSong()->FirstTrack();

			while(t)
			{
				if(!t->parent)
				{
					if(t->t_groups.FindGroup(activegroup)==true)
					{
						if(Edit_Group_Tracks *egt=new Edit_Group_Tracks)
						{
							egt->track=t;
							egt->name=mainvar->GenerateString(t->GetName());
							egt->index=WindowSong()->GetOfTrack(t);

							tracks.AddEndO(egt);
						}

						char *h=mainvar->GenerateString("[",mainvar->ConvertIntToChar(WindowSong()->GetOfTrack(t)+1,h2),"] ",t->GetName());

						if(h)
						{
							grouptracks->AddStringToListBox(h);
							delete h;
						}
					}
				}

				t=t->NextTrack();
			}
		}

		ShowActiveGroupColour();
	}
	else
	{
		if(groupname)
		{
			groupname->SetString("-:-");
			groupname->Disable();
		}

		if(groupgadget)
			groupgadget->Disable();

		if(grouptracks)
		{
			grouptracks->ClearListBox();
			grouptracks->Disable();
		}

		ShowActiveGroupColour();
	}

	ShowActiveGroupStatus();
}


EditData * Edit_Group::EditDataMessage(EditData *data)
{
	if(data)
	{
		switch(data->id)
		{
		default:
			{
			}
			break;
		}
	}

	return 0;
}

void Edit_Group::DeleteActiveGroup()
{
	if(activegroup)
	{
		Seq_Group *del=activegroup;
		activegroup=(Seq_Group *)activegroup->NextOrPrev();
		WindowSong()->DeleteGroup(del);
		ShowGroups();
		ShowActiveGroup();
		//	WindowSong()->undo.RefreshUndos();
	}
}

void Edit_Group::AddSelectedTracks()
{
	if(!activegroup)
	{
		activegroup=WindowSong()->CreateNewGroup();
		ShowGroups();
		ShowActiveGroup();
	}

	if(activegroup)
	{
		int c=0;
		Seq_Track *t=WindowSong()->FirstTrack();

		while(t)
		{
			if((!t->parent) && (t->flag&OFLAG_SELECTED))
			{
				if(t->t_groups.FindGroup(activegroup)==0)
				{
					t->t_groups.AddToGroup(activegroup);
					c++;
				}
			}

			t=t->NextTrack();
		}

		if(c)
			ShowActiveGroup();
	}
}

void Edit_Group::AddActiveTrack()
{
	Seq_Track *t=WindowSong()->GetFocusTrack();

	if(activegroup && t && (!t->parent))
	{
		if(t->t_groups.FindGroup(activegroup)==0)
		{
			t->t_groups.AddToGroup(activegroup);
			ShowActiveGroup();
		}
	}
}

void Edit_Group::RemoveSelectedTracks()
{
	if(activegroup)
	{
		int c=0;
		Seq_Track *t=WindowSong()->FirstTrack();

		while(t)
		{
			if((!t->parent) && (t->flag&OFLAG_SELECTED))
			{
				if(t->t_groups.FindGroup(activegroup))
				{
					t->t_groups.RemoveBusFromGroup(activegroup);
					c++;
				}
			}

			t=t->NextTrack();
		}

		if(c)
			ShowActiveGroup();
	}
}

void Edit_Group::RemoveActiveTrack()
{
	Seq_Track *t=WindowSong()->GetFocusTrack();

	if(activegroup && t && (!t->parent))
	{
		if(t->t_groups.FindGroup(activegroup))
		{
			t->t_groups.RemoveBusFromGroup(activegroup);
			ShowActiveGroup();
		}
	}
}

guiMenu *Edit_Group::CreateMenu()
{
	guiMenu *n;

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		n=menu->AddMenu(Cxs[CXS_FILE],0);

		if(n)
		{
			class menu_addgroup:public guiMenu
			{
			public:
				menu_addgroup(Edit_Group *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->AddGroupFromFile();
				} //

				Edit_Group *editor;
			};
			n->AddFMenu("Load Group/s",new menu_addgroup(this));

			class menu_saveallgroups:public guiMenu
			{
			public:
				menu_saveallgroups(Edit_Group *ed,bool a)
				{
					editor=ed;
					all=a;
				}

				void MenuFunction()
				{
					editor->SaveAllGroups(all);
				} //

				Edit_Group *editor;
				bool all;
			};

			n->AddFMenu("Save all Groups",new menu_saveallgroups(this,true));
			n->AddFMenu("Save active Group",new menu_saveallgroups(this,false));
		}

		n=menu->AddMenu("Groups",0);

		if(n)
		{	
			class menu_newgroup:public guiMenu
			{
			public:
				menu_newgroup(Edit_Group *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					Seq_Group *g=editor->WindowSong()->CreateNewGroup();

					if(g)
					{
						editor->activegroup=g;
						editor->ShowGroups();
						editor->ShowActiveGroup();
					}
				} //

				Edit_Group *editor;
			};

			n->AddFMenu(Cxs[CXS_CREATENEWGROUP],new menu_newgroup(this));

			class menu_delgroup:public guiMenu
			{
			public:
				menu_delgroup(Edit_Group *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->DeleteActiveGroup();
				} //

				Edit_Group *editor;
			};

			n->AddFMenu(Cxs[CXS_DELETEGROUP],new menu_delgroup(this));

			n->AddLine();
			class menu_aacttrack:public guiMenu
			{
			public:
				menu_aacttrack(Edit_Group *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->AddActiveTrack();
				} //

				Edit_Group *editor;
			};

			n->AddFMenu(Cxs[CXS_ADDACTIVETRACKTOGROUP],new menu_aacttrack(this));

			class menu_aselacttrack:public guiMenu
			{
			public:
				menu_aselacttrack(Edit_Group *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->AddSelectedTracks();
				} //

				Edit_Group *editor;
			};

			n->AddFMenu(Cxs[CXS_ADDSELECTEDTRACKSTOGROUP],new menu_aselacttrack(this));

			n->AddLine();

			class menu_racttrack:public guiMenu
			{
			public:
				menu_racttrack(Edit_Group *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->RemoveActiveTrack();
				} //

				Edit_Group *editor;
			};

			n->AddFMenu(Cxs[CXS_REMOVEACTIVETRACKFROMGROUP],new menu_racttrack(this));

			class menu_rselacttrack:public guiMenu
			{
			public:
				menu_rselacttrack(Edit_Group *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->RemoveSelectedTracks();
				} //

				Edit_Group *editor;
			};

			n->AddFMenu(Cxs[CXS_REMOVESELECTEDTRACKSFROMGROUP],new menu_rselacttrack(this));
		}
	}

	//	maingui->AddCascadeMenu(menu);
	return menu;
}

void Edit_Group::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case FX_GROUPCOLOUR_ID:
		if(activegroup)
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				// Track Colour
				class menu_gcolour:public guiMenu
				{
				public:
					menu_gcolour(Edit_Group *e,Seq_Group *g,int x,int y)
					{
						editor=e;
						group=g;
						xpos=x;
						ypos=y;
					}

					void MenuFunction()
					{
						colourReq req;
						int rgb=group->colour.rgb;
						req.OpenRequester(editor,&group->colour);
						if(rgb!=group->colour.rgb)
						{
							group->colour.showcolour=true;
							editor->ShowActiveGroupColour();
							maingui->RefreshColour(group);
							maingui->ClearRefresh();
						}

					} //

					Edit_Group *editor;
					Seq_Group *group;
					int xpos,ypos;
				};
				popmenu->AddFMenu(Cxs[CXS_SELECTCOLOUR],new menu_gcolour(this,activegroup,g->x2,g->y2));

				class menu_gcolouroff:public guiMenu
				{
				public:
					menu_gcolouroff(Edit_Group *e,Seq_Group *g,int x,int y)
					{
						editor=e;
						group=g;;
						xpos=x;
						ypos=y;
					}

					void MenuFunction()
					{
						// Toggle
						if(group->colour.showcolour==true)
							group->colour.showcolour=false;
						else
							group->colour.showcolour=true;

						editor->ShowActiveGroupColour();
						maingui->RefreshColour(group);
						maingui->ClearRefresh();
					} //

					Edit_Group *editor;
					Seq_Group *group;
					int xpos,ypos;
				};

				popmenu->AddFMenu(Cxs[CXS_USECOLOUR],new menu_gcolouroff(this,activegroup,g->x2,g->y2),activegroup->colour.showcolour);

				ShowPopMenu();
			}
		}
		break;

	case GADGET_GROUPID:
		{	
			activegroup=WindowSong()->GetGroupIndex(g->index);
			ShowActiveGroup();
		}
		break;

	case FX_GROUPMUTE_ID:
		if(activegroup)
		{
			// Toggle Group Mute
			if(activegroup->mute==true)
				activegroup->mute=false;
			else 
				activegroup->mute=true;
		}
		break;

	case FX_GROUPSOLO_ID:
		if(activegroup)
		{
			WindowSong()->SetGroupSolo(activegroup,activegroup->solo==true?false:true);
		}
		break;

	case FX_GROUPREC_ID:
		if(activegroup)
		{
			WindowSong()->SetGroupRec(activegroup,activegroup->rec==true?false:true);
		}
		break;

	case GADGET_GROUPNAME:
		if(activegroup)
		{
			activegroup->SetName(g->string);
			ShowGroups();
			RefreshGroupGUI();
		}
		break;
	}
}

void Edit_Group::FreeMemory()
{
	if(winmode&WINDOWMODE_INIT)
	{
		DeleteAllTrackObjects();
		//gadgetlists.RemoveAllGadgetLists();

		if(winmode&WINDOWMODE_DESTROY)
		{
		}
		else
			Clear();
	}
}

void Edit_Group::ShowGroups()
{
	if(!activegroup)
	{
		activegroup=WindowSong()->FirstGroup();

		//	ShowActiveGROUP();
		//	ListActiveGROUP();
	}

	if(glist && groupgadget)
	{
		Seq_Group *g=WindowSong()->FirstGroup();

		groupgadget->ClearListBox();

		while(g)
		{
			if(!activegroup)
				activegroup=g;

			groupgadget->AddStringToListBox(g->name);

			g=g->NextGroup();
		}

		if(activegroup)
			groupgadget->SetListBoxSelection(activegroup->GetIndex());
	}
}

void Edit_Group::InitGadgets()
{
#ifdef OLDIE
	ResetGadgets();

	glist=gadgetlists.AddGadgetList(this);

	if(glist)
	{
		groupname=glist->AddString(frame_groups.x,0,frame_groups.x2/2,frame_groups.y-1,GADGET_GROUPNAME,0,0,"Name of selected group");
		//groupgadget=glist->AddListView(frame_groups.x,frame_groups.y,frame_groups.x2/2,frame_groups.y2,GADGET_GROUPID,"Song's Groups");
		//grouptracks=glist->AddListView(frame_groups.x2/2+1,frame_groups.y,frame_groups.x2,frame_groups.y2,GADGET_GROUPDIFFID,"Tracks using selected Group");

		int hx=frame_groups.x;
		int y=frame_groups.y2+1;
		int y2;
		int w=(frame_groups.x2-frame_groups.x)/4;

		w-=1;

		y2=y+maingui->GetFontSizeY_Sub();

		group_mute=glist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKMUTEOFF,FX_GROUPMUTE_ID,0,"Mute Group");

		hx+=w+1;
		group_solo=glist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKSOLOOFF,FX_GROUPSOLO_ID,0,"Solo Playback Group");

		hx+=w+1;
		group_rec=glist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKRECORDOFF,FX_GROUPREC_ID,0,"Set Tracks using Group in Record Mode");

		hx+=w+1;
		group_colour=glist->AddButton(hx,y,hx+w,y2,Cxs[CXS_COLOUR],FX_GROUPCOLOUR_ID,0,Cxs[CXS_SELECTGROUPCOLOUR]);

		if(group_colour)
		{
			colour_x=group_colour->x;
			colour_y=group_colour->y2+1;
			colour_y2=height-1;
			colour_x2=group_colour->x2;
		}

		ShowGroups();
	}
#endif

}

void Edit_Group::RefreshRealtime()
{
	if(activegroup)
	{
		if(status_group_rec!=activegroup->rec ||
			status_group_solo!=activegroup->solo ||
			status_group_mute!=activegroup->mute)
			ShowActiveGroupStatus();

		Edit_Group_Tracks *egt=FirstGroupTrack();

		while(egt)
		{
			if(strcmp(egt->name,egt->track->GetName())!=0)
			{
				ShowActiveGroup();
				break;
			}

			egt=egt->NextGroupTrack();
		}
	}
}

void Edit_Group::RedrawGfx()
{
	ShowActiveGroupColour();
}

void Edit_Group::RefreshGroupGUI()
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		switch(w->GetEditorID())
		{
		case EDITORTYPE_ARRANGE:
			{
				Edit_Arrange *ar=(Edit_Arrange *)w;

				if(ar->WindowSong()->GetFocusTrack())
				{
					if(ar->WindowSong()->GetFocusTrack()->GetGroups()->FindGroup(activegroup)==true)
					{
						//ar->trackfx.ShowActiveTrack();
					}
				}
			}
			break;
		}

		w=w->NextWindow();
	}
}

void Edit_Group::RefreshObjects(LONGLONG par,bool editcall)
{
	Seq_Group *g=(Seq_Group *)par;

	if(!g)
	{
		if(activegroup)
		{
			// Check Track
			Edit_Group_Tracks *egt=FirstGroupTrack();

			if(!egt)
			{
				Seq_Track *t=WindowSong()->FirstTrack();

				while(t)
				{
					if(!t->parent)
					{
						if(t->t_groups.FindGroup(activegroup)==true)
						{
							ShowActiveGroup();
							break;
						}
					}

					t=t->NextTrack();
				}

			}
			else
				while(egt)
				{
					//Find Track
					if(WindowSong()->FindTrack(egt->track)==false ||
						egt->index!=WindowSong()->GetOfTrack(egt->track))
					{
						ShowActiveGroup();
						break;
					}

					egt=egt->NextGroupTrack();
				}
		}
	}
	else
		if(activegroup && activegroup==g)
		{
			ShowActiveGroup();
		}
}

void Edit_Group::Init()
{
#ifdef OLDIE
	FreeMemory();

	if(width && height)
	{			
		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
		{
			double h=height;

			h-=maingui->GetFontSizeY()*3;

			frame_groups.on=true;
			frame_groups.x=0;
			frame_groups.x2=width;
			frame_groups.y=22;
			frame_groups.y2=(int)h;

			InitGadgets();	
		}

		ShowActiveGroup();
		RedrawGfx();
	}
#endif
}

void Edit_Group::AddGroupFromFile()
{
	camxFile sfile;

	if(sfile.OpenFileRequester(0,this,"Load Group/s","Groups (*.cxgp)|*.cxgp;|All Files (*.*)|*.*||",true)==true)
	{
		if(sfile.OpenRead(sfile.filereqname)==true)
		{
			char check[4];

			sfile.Read(check,4);

			if(mainvar->CompareStringWithOutZero(check,"CAMX")==true)
			{
				sfile.Read(check,4);

				if(mainvar->CompareStringWithOutZero(check,"GRPP")==true)
				{
					int version=0,groups=0;

					sfile.Read(&version);
					sfile.Read(&groups);

					if(groups)
					{
						while(groups--)
						{
							Seq_Group *g=WindowSong()->CreateNewGroup();

							if(g)
							{
								activegroup=g;
								sfile.LoadChunk();

								if(sfile.GetChunkHeader()==CHUNK_SONGGROUP)
								{
									sfile.ChunkFound();
									g->Load(&sfile);
								}
								else
									break;
							}
						}

						ShowGroups();
						ShowActiveGroup();
					}
				}
			}
		}

		sfile.Close(true);
	}
}

void Edit_Group::SaveAllGroups(bool all)
{
	if(WindowSong()->FirstGroup() && (all==true || activegroup))
	{
		camxFile sfile;

		char *s;
		int groups_nr;

		if(all==true)
		{
			s="Save all Groups";
			groups_nr=WindowSong()->GetCountGroups();
		}
		else
		{
			s="Save active Group";
			groups_nr=1;
		}

		if(sfile.OpenFileRequester(0,this,s,"Groups (*.cxgp)|*.cxgp;|All Files (*.*)|*.*||",false)==true)
		{
			sfile.AddToFileName(".cxgp");

			if(sfile.OpenSave(sfile.filereqname)==true)
			{
				int version=maingui->GetVersion();

				// Header
				sfile.Save("CAMX",4);
				sfile.Save("GRPP",4);

				sfile.Save(&version,sizeof(int)); // Version
				sfile.Save(&groups_nr,sizeof(groups_nr)); // number

				if(all==false)
				{	
					activegroup->Save(&sfile); // single
				}
				else
				{
					// save all groups

					Seq_Group *f=WindowSong()->FirstGroup();

					while(f)
					{
						f->Save(&sfile);
						f=f->NextGroup();
					}
				}
			}

			sfile.Close(true);
		}
	}
}

