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
#include "undofunctions_track.h"
#include "folder.h"
#include "player.h"
#include "editbuffer.h"
#include "MIDIoutproc.h"
#include "arrangeeditor.h"
#include "drumevent.h"
#include "MIDIfile.h"
#include "undo_automation.h"
#include "pianoeditor.h"

Undo_DeleteAllEmptyTracks::Undo_DeleteAllEmptyTracks(Seq_Song *s)
{
	id=Undo::UID_DELETEALLEMPTYTRACKS;
	song=s;
	trackscounter=0;
	tracks=0;
	song->tracks.CreateList(&list); // Buffer old
}

void Undo_DeleteAllEmptyTracks::Do()
{
	if(!tracks)
	{
		for(int i=0;i<2;i++)
		{
			trackscounter=0;
			Seq_Track *t=song->FirstTrack();

			while(t)
			{
				// Empty ?
				bool empty=false;

				// Empty ?
				if(!t->FirstPattern(MEDIATYPE_ALL))
				{
					empty=true;

					// Check Childs
					Seq_Track *ct=t->FirstChildTrack();

					while(ct && ct->IsTrackChildOfTrack(t)==true)
					{
						if(ct->FirstPattern(MEDIATYPE_ALL))
						{
							empty=false;
							break;
						}

						ct=ct->NextTrack();
					}
				}

				if(empty==true)
				{
					if(i==1)
						tracks[trackscounter]=t;

					trackscounter++;
					t=t->LastChildTrack()?t->LastChildTrack()->NextTrack():t->NextTrack();
				}
				else
					t=t->NextTrack();
			}

			if(i==0)
			{
				if(trackscounter)
				{
					tracks=new Seq_Track*[trackscounter];
				}
				else
					return;
			}
		}
	}

	if(tracks)
	{
		for(int i=0;i<trackscounter;i++)
			DeleteTrack(tracks[i]);
	}
}

void Undo_DeleteAllEmptyTracks::DoUndo()
{
//	for(int i=0;i<trackscounter;i++)
//		song->undo.CutUTrack(tracks[i]);

	song->tracks.ListToFolder(&list);

	for(int i=0;i<trackscounter;i++)
		RefreshTrack(tracks[i]);
}

void Undo_DeleteAllEmptyTracks::FreeData()
{
	if(inundo==true)
	{
		for(int i=0;i<trackscounter;i++)
		{
		//	song->undo.CutUTrack(tracks[i]);
			tracks[i]->DeleteTrackData(true);
		}
	}

	if(tracks)
		delete tracks;

	list.FreeMemory();
}

void Undo_DeleteAllEmptyTracks::RefreshGUI(bool undorefresh)
{
	for(int i=0;i<trackscounter;i++)
		maingui->RemoveTrackFromGUI(tracks[i]);

	mainedit->CheckEditElementsForGUI(song,this,undorefresh);
}

void Undo_DeleteAllEmptyTracks::RefreshDo()// Delete Plugins Memory etc
{
	for(int i=0;i<trackscounter;i++)
	{
		tracks[i]->RefreshDo();
	}
}

void Undo_DeleteAllEmptyTracks::RefreshPreUndo() // Delete Plugins Memory etc
{
	for(int i=0;i<trackscounter;i++)
	{
		tracks[i]->PreRefreshDo();
	}
}

void EditFunctions::DeleteAllEmptyTracks(Seq_Song *song)
{
	if(CheckIfEditOK(song)==true && song->FirstTrack())
	{
		Seq_Track *t=song->FirstTrack();

		while(t)
		{
			bool trackempty=false;

			// Empty ?
			if(!t->FirstPattern(MEDIATYPE_ALL))
			{
				trackempty=true;

				// Check Childs
				Seq_Track *ct=t->FirstChildTrack();

				while(ct && ct->IsTrackChildOfTrack(t)==true)
				{
					if(ct->FirstPattern(MEDIATYPE_ALL))
					{
						trackempty=false;
						break;
					}

					ct=ct->NextTrack();
				}
			}

			if(trackempty==true)
				break;

			t=t->NextTrack();
		}

		if(t)
		{
			if(Undo_DeleteAllEmptyTracks *ude=new Undo_DeleteAllEmptyTracks(song))
			{
				OpenLockAndDo(song,ude,true);

				/*
				char h2[64];
				if(char *nc=mainvar->GenerateString(Cxs[CXS_EMPTYTRACKSFOUND],":",mainvar->ConvertIntToChar(ude->emptycounter,h2)))
				{
				maingui->MessageBoxOk(0,nc);
				delete nc;
				}
				*/

			}
		}
		else
			maingui->MessageBoxOk(0,Cxs[CXS_NOEMPTYTRACKSFOUND]);
	}
}

Undo_DeleteTracks::Undo_DeleteTracks(Seq_Song *s,Seq_Track *st)
{
	id=Undo::UID_DELETESELECTEDTRACKS;
	song=s;
	trackscounter=0;
	tracks=0;
	singletrack=st;
	song->tracks.CreateList(&list); // Buffer old
}


void Undo_DeleteTracks::Do()
{
	if(!tracks)
	{
		for(int i=0;i<2;i++)
		{
			trackscounter=0;

			if(singletrack==0)
			{
				Seq_Track *t=song->FirstTrack();

				while(t)
				{
					if(t->IsSelected()==true)
					{
						if(i==1)
							tracks[trackscounter]=t;

						trackscounter++;

						t=t->LastChildTrack()?t->LastChildTrack()->NextTrack():t->NextTrack();
					}
					else
						t=t->NextTrack();
				}
			}
			else
			{
				if(i==1)
					tracks[trackscounter]=singletrack;

				trackscounter++;
			}

			if(i==0)
			{
				if(trackscounter)
				{
					tracks=new Seq_Track*[trackscounter];
					if(!tracks)
						return;
				}
				else
					return;
			}
		}
	}

	if(tracks)
	{
		//TRACE ("Ct %d\n",song->tracks.GetCount());

		for(int i=0;i<trackscounter;i++)
			DeleteTrack(tracks[i]);
	}

	song->tracks.Close();
}

void Undo_DeleteTracks::DoUndo()
{
	//for(int i=0;i<trackscounter;i++)
	//	song->undo.CutUTrack(tracks[i]);
	
	Seq_Track *focustrack=song->GetFocusTrack();

	song->tracks.ListToFolder(&list);

	for(int i=0;i<trackscounter;i++)
		RefreshTrack(tracks[i]);

	song->tracks.Close();

	if(focustrack)
	{
		int index=song->GetOfTrack(focustrack);

		if(index>=0)
			song->SetFocusTrack(focustrack);
	}
}

void Undo_DeleteTracks::RefreshGUI(bool undorefresh)
{
	for(int i=0;i<trackscounter;i++)
		maingui->RemoveTrackFromGUI(tracks[i]);

	mainedit->CheckEditElementsForGUI(song,this,undorefresh);
}

void Undo_DeleteTracks::FreeData()
{
	if(inundo==true)
	{
		for(int i=0;i<trackscounter;i++)
		{
		//	song->undo.CutUTrack(tracks[i]);
			tracks[i]->DeleteTrackData(true);
		}
	}

	if(tracks)
	{
		delete tracks;
		tracks=0;
	}

	list.FreeMemory();
}

void Undo_DeleteTracks::RefreshDo()// Delete Plugins Memory etc
{
	for(int i=0;i<trackscounter;i++)
	{
		tracks[i]->RefreshDo();
	}
}

void Undo_DeleteTracks::RefreshPreUndo() // Delete Plugins Memory etc
{
	for(int i=0;i<trackscounter;i++)
	{
		tracks[i]->PreRefreshDo();
	}
}

void EditFunctions::DeleteSelectedTracks(Seq_Song *song,Seq_Track *singletrack)
{
	if(CheckIfEditOK(song)==true && song->FirstTrack())
	{
		Seq_Track *t=song->FirstTrack();

		while(t){
			if(t->IsSelected()==true)break;
			t=t->NextTrack();
		}
		
		if(t || singletrack){
			if(Undo_DeleteTracks *ude=new Undo_DeleteTracks(song,singletrack))
				OpenLockAndDo(song,ude,true);
		}
	}
}


void Undo_SortAllTracks::Do()
{
	for(int c=1;c<3;c++)
	{
		bool foundother=false;
		Seq_Track *sortbefore=0;

		for(int i=0;i<count;i++)
		{
			if(list[i].type==c)
			{
				if(sortbefore)
				{
					//Move Track+Childs Before Sort Track

					int childs=list[i].track->GetCountChildTracks();

					if(childs>0)
					{
						// Track+Childs
						if(Seq_Track **st=new Seq_Track*[childs])
						{
							int ci=0;
							Seq_Track *cc=list[i].track->NextTrack();
							while(cc && cc->parent)
							{
								st[ci++]=cc;
								cc=cc->NextTrack();
							}

							song->tracks.CutQObject(list[i].track);
							for(int i2=0;i2<childs;i2++)
								song->tracks.CutQObject(st[i2]);

							song->tracks.AddNextO(list[i].track,sortbefore);
							for(int i2=0;i2<childs;i2++)
								song->tracks.AddNextO(st[i2],sortbefore);

							delete st;
						}
					}
					else // 1 Track
					{
						song->tracks.CutObject(list[i].track);
						song->tracks.AddNextO(list[i].track,sortbefore);
					}

					song->tracks.Close();
				}
			}
			else
				if(list[i].type>c)
				{
					if(!sortbefore)
						sortbefore=list[i].track;
					foundother=true;
				}
		}
	}
}

void Undo_SortAllTracks::DoUndo()
{
	Seq_Track *t=song->FirstTrack();

	while(t)
	{
		Seq_Track *nt=t->NextTrack();

		if(!t->parent)
		{
			// Find old pos
			for(int i=0;i<song->GetCountOfTracks();i++)
			{
				if(oldsorttracks[i]==t)
				{
					int moveindex=i-song->GetOfTrack(t);
					song->MoveTrackIndex(t,moveindex);			
					break;
				}
			}
		}

		t=nt;
	}
}

void Undo_SortAllTracks::FreeData()
{
	if(oldsorttracks)
		delete oldsorttracks;

	if(list)
		delete list;
}

void EditFunctions::SortTracks(Seq_Song *song)
{
	int tcount=song->GetCountOfTracksWithOutChilds();

	if(tcount>1)
	{
		bool sortfound=false;
		TrackTypeList *tt=new TrackTypeList[tcount];

		if(!tt)return;

		int c=0;
		Seq_Track *t=song->FirstTrack();

		while(t)
		{
			if(!t->parent)
			{
				tt[c].track=t;
				tt[c].type=3;

				// Find Audio
				if(t->FirstPattern(MEDIATYPE_AUDIO))
					tt[c].type=1;

				Seq_Track *child=t->FirstChildTrack();
				while(child && child->parent)
				{
					if(child->FirstPattern(MEDIATYPE_AUDIO))
						tt[c].type=1;

					child=child->NextTrack();
				}

				// Find MIDI
				if(tt[c].type==3)
				{
					if(t->FirstPattern(MEDIATYPE_MIDI))
						tt[c].type=2;

					Seq_Track *child=t->FirstChildTrack();
					while(child && child->parent)
					{
						if(child->FirstPattern(MEDIATYPE_MIDI))
							tt[c].type=2;

						child=child->NextTrack();
					}
				}

				c++;
			}

			t=t->NextTrack();
		}

#ifdef DEBUG
		for(int i=0;i<tcount;i++)
			TRACE("Type %d\n",tt[i].type);
#endif

		for(int c=1;c<3;c++)
		{
			bool found=false,foundother=false;

			for(int i=0;i<tcount;i++)
			{
				if(tt[i].type==c)
				{
					found=true;
					if(foundother==true)
						sortfound=true;
				}
				else
					if(tt[i].type>c)
						foundother=true;
			}
		}

		if(sortfound==false)
		{
			delete tt;
			return;
		}

		if(Undo_SortAllTracks *ude=new Undo_SortAllTracks(song,tt,tcount))
		{
			if(Seq_Track **tracks=new Seq_Track*[song->GetCountOfTracks()])
			{
				// Init Old Tracks
				ude->oldsorttracks=tracks;

				Seq_Track *t=song->FirstTrack();

				while(t)
				{
					*tracks++=t;
					t=t->NextTrack();
				}

				OpenLockAndDo(song,ude,true);
			}
			else
				delete ude;
		}
	}
}

void EditFunctions::DeleteSong(guiScreen *screen,Seq_Song *song,int flag)
{
	if(CheckIfEditOK(song)==true) // song Can be NULL !
	{
		song->underdeconstruction=true;

		Seq_Project *project=song->project;
		//Seq_Song *prevornextsong=(Seq_Song *)song->NextOrPrev();

		// active Song ?
		if(song==mainvar->GetActiveSong())
		{	
			song->StopSong(NO_SONGPRepair,song->GetSongPosition());
			mainvar->SetActiveSong(/*(flag&DELETESONG_FLAG_SETNEXTSONGAUTO)?prevornextsong:*/0);
		}

		maingui->RemoveSongFromGUI(screen,song);

		if(flag&DELETESONG_FLAG_DELETEFILES)
		{
			// Remove
			char *dirtODeInit=mainvar->GenerateString(song->directoryname);

			//	mainthreadcontrol->LockActiveSong();
			project->DeleteSong(song,Seq_Project::DELETESONG_FULL|Seq_Project::DELETESONG_NOLOCK);
			//	mainthreadcontrol->UnlockActiveSong();

			// Delete Files
			if(dirtODeInit)
			{
				mainvar->DeleteADirectory(dirtODeInit);
				delete dirtODeInit;
			}

			project->Save(0);
		}
		else
		{
			// Close
			song->Save(0);
			//mainthreadcontrol->LockActiveSong();
			project->DeleteSong(song,Seq_Project::DELETESONG_NOLOCK|Seq_Project::DELETESONG_ONLYCLOSE);
			//song->CloseSong(Seq_Project::DELETESONG_NOLOCK);
			//mainthreadcontrol->UnlockActiveSong();
		}

		if(!(flag&DELETESONG_FLAG_NOGUIREFRESH))
		{
			maingui->RefreshProjectScreens(project);

			// Refresh GUI
			guiWindow *w=maingui->FirstWindow();
			while(w)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_PLAYER:
					{
						Edit_Player *p=(Edit_Player *)w;

						if(p->activeproject==project)
						{
							if(p->activesong==song)
								p->activesong=0;

							p->ShowSongs();
						}
					}
					break;
				}

				w=w->NextWindow();
			}
		}

	}
}