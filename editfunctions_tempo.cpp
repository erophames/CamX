#include "defines.h"

#include "gui.h"
#include "editor.h"
#include "undo.h"

#include "patternselection.h"
#include "semapores.h"
#include "object_song.h"
#include "objectpattern.h"
#include "songmain.h"
#include "editfunctions.h"
#include "undofunctions_tempo.h"
#include "editbuffer.h"
#include "MIDIoutproc.h"
#include <math.h>

bool EditFunctions::CopySelectedTempos(Seq_Song *song,OSTART tickdiff,double tempodiff,int flag)
{
	return true;
}

void Undo_DeleteTempos::DoUndo()
{
	song->BufferBeforeTempoChanges();

	if(firsttempomoved==true)
	{
		song->timetrack.FirstTempo()->ostart=firsttempooldstart;
		firsttempomoved=false;
	}

	UndODeInitTempo *udt=tempos;
	int i=numberoftempos;

	while(i--)
	{
		udt->tempo->UnSelect();
		song->timetrack.tempomap.AddOSort(udt->tempo,udt->tempo->GetTempoStart());
		udt++;
	}

	song->timetrack.Close();
	//AddSubUndoFunctions();
}

void Undo_DeleteTempos::UndoEnd()
{
	song->timetrack.RepairTempomap();
}

bool Undo_DeleteTempos::RefreshUndo()
{
	if(inundo==true)
		return true;

	/*
	UndODeInitTempo *udt=tempos;
	int i=numberoftempos;

	while(i--)
	{
	if(song->timetrack.tempomap.GetIx(udt->tempo)==-1)
	return false;

	udt++;
	}
	*/

	return true;
}

void Undo_DeleteTempos::Do()
{
	song->BufferBeforeTempoChanges();

	for(int i=0;i<numberoftempos;i++)
	{
		if(song->timetrack.lastselectedtempo==tempos[i].tempo)
			song->timetrack.lastselectedtempo=0;

		song->timetrack.tempomap.CutQObject(tempos[i].tempo);
	}

	//AddSubUndoFunctions();

	// Move First Tempo to Position 0 ?
	if(song->timetrack.FirstTempo()->GetTempoStart()>0)
	{
		firsttempomoved=true;
		firsttempooldstart=song->timetrack.FirstTempo()->GetTempoStart();
		song->timetrack.FirstTempo()->ostart=0;
	}
	else
	{
		firsttempomoved=false;
	}

	song->timetrack.Close();
}

void Undo_DeleteTempos::DoEnd()
{
	song->timetrack.RepairTempomap();
}

void Undo_DeleteTempos::FreeData()
{
	if(tempos)
	{
		if(inundo==true)
		{
			UndODeInitTempo *dt=tempos;
			int c=numberoftempos;

			while(c--){				
				delete dt->tempo;
				dt++;
			}
		}

		delete tempos;
	}
}

void EditFunctions::DeleteSelectedTempos(Seq_Song *song,bool addtolastundo)
{
	if(CheckIfEditOK(song)==true && (!(song->status&Seq_Song::STATUS_WAITPREMETRO)))
	{
		int c=0;

		Seq_Tempo *t=song->timetrack.FirstTempo();

		t=t->NextTempo();// Skip 1. Tempo

		while(t)
		{
			if(t->IsSelected()==true)
				c++;

			t=t->NextTempo();
		}

		if(c>0 && song->timetrack.GetCountOfTempos()>c)
		{
			UndODeInitTempo *pp=new UndODeInitTempo[c],*pointer;

			if(pointer=pp)
			{			
				Seq_Tempo *t=song->timetrack.FirstTempo();

				t=t->NextTempo();// Skip 1. Tempo

				while(t)
				{
					if(t->IsSelected()==true)
					{
						pp->tempo=t;
						pp++;
					}

					t=t->NextTempo();
				}

				if(Undo_DeleteTempos *dt=new Undo_DeleteTempos(song,pointer,c))
					OpenLockAndDo(song,dt,true);
				else
					delete pointer;
			}
		}
	}
}

class Undo_ChangeTempoMap:public UndoFunction
{
public:
	Undo_ChangeTempoMap(Seq_Song *);
	void DoRedo();
	void DoUndo();

	void CheckAndAddToUndo();
	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_EDITTEMPOEVENTS]);
	}

	void FreeData()
	{
		tempos.DeleteAllO();
		temposundo.DeleteAllO();
	}

	OListStart tempos,temposundo;
};

void Undo_ChangeTempoMap::DoRedo()
{
	song->BufferBeforeTempoChanges();

	song->timetrack.tempomap.MoveListToList(&temposundo); // Buffer New
	tempos.MoveListToList(&song->timetrack.tempomap);
	song->timetrack.tempomap.DeSelectAll();
	song->timetrack.RepairTempomap();
}

void Undo_ChangeTempoMap::DoUndo()
{
	song->BufferBeforeTempoChanges();

	song->timetrack.tempomap.MoveListToList(&tempos); // Buffer New
	temposundo.MoveListToList(&song->timetrack.tempomap);

#ifdef DEBUG
	if(song->timetrack.FirstTempo()==0)
		maingui->MessageBoxError(0,"0 Tempos !");

	if(song->timetrack.FirstTempo()->GetTempoStart()!=0)
		maingui->MessageBoxError(0,"0 Tempo Start !");
#endif

	song->timetrack.tempomap.DeSelectAll();
	song->timetrack.RepairTempomap();
}

Undo_ChangeTempoMap::Undo_ChangeTempoMap(Seq_Song *s)
{
	id=Undo::UID_MOVETEMPOS;
	song=s;
	s->timetrack.CloneTempos(&temposundo);
}

void EditFunctions::EditSelectedTempos(MoveO *mo)
{
	Seq_Song *song=mo->song;

	if(CheckIfEditOK(song)==true && (mo->diff!=0 || mo->dindex!=0 || mo->resetdbvalues==true))
	{
		int cdiff=0;
		int cmove=0;

		if(mo->resetdbvalues==true)
		{
			Seq_Tempo *t=song->timetrack.FirstTempo();

			while(t)
			{
				if(t->IsSelected()==true)
				{
					double param, fractpart, intpart;

					param = t->tempo;
					fractpart = modf (param , &intpart);

					if(fractpart!=0)
						cdiff++;
				}

				t=t->NextTempo();
			}
		}
		else
		{
			Seq_Tempo *t=song->timetrack.FirstTempo();

			if(t->IsSelected()==true)
				cdiff++;

			t=t->NextTempo(); // Skip 1. Tempo - no move

			while(t)
			{
				if(t->IsSelected()==true)
				{
					cdiff++;
					cmove++;
				}

				t=t->NextTempo();
			}
		}

		if(cdiff || cmove)
		{
			if(Undo_ChangeTempoMap *uct=new Undo_ChangeTempoMap(song))
			{
				if(song==mainvar->GetActiveSong())
					mainthreadcontrol->LockActiveSong();

				song->BufferBeforeTempoChanges();

				if(mo->resetdbvalues==true)
				{
					Seq_Tempo *t=song->timetrack.FirstTempo();

					while(t)
					{	
						if(t->IsSelected()==true)
						{
							double param, fractpart, intpart;

							param = t->tempo;
							fractpart = modf (param , &intpart);

							t->tempo=intpart;
						}

						t=t->NextTempo();
					}

				}
				else
				{
					Seq_Tempo **st=0;
					int mc=0;

					if(cmove)
						st=new Seq_Tempo*[cmove];

					Seq_Tempo *t=song->timetrack.FirstTempo();

					while(t)
					{
						Seq_Tempo *nt=t->NextTempo();

						if(t->IsSelected()==true)
						{
							if(mo->diff)
							{
								if(t!=song->timetrack.FirstTempo() && st)
								{
									song->timetrack.tempomap.CutObject(t);
									st[mc++]=t;
									//song->timetrack.tempomap.MoveO(t,t->ostart+mo->diff);
								}
							}

							if(mo->dindex)
							{
								t->tempo+=mo->dindex;

								if(t->tempo<MINIMUM_TEMPO)
									t->tempo=MINIMUM_TEMPO;
							}
						}

						t=nt;
					}

					if(cmove && st)
					{
						for(int i=0;i<mc;i++)
						{
							st[i]->ostart+=mo->diff;
							song->timetrack.tempomap.AddOSort(st[i]);
						}

						delete st;
					}
				}

				song->timetrack.Close();

				song->timetrack.RepairTempomap();

				if(song==mainvar->GetActiveSong())
					mainthreadcontrol->UnlockActiveSong();

				song->undo.OpenUndoFunction(uct);

				CheckEditElementsForGUI(song,uct,true);
			}
		}
	}
}

void Undo_CreateTempo::DoUndo()
{
	song->BufferBeforeTempoChanges();
	if(newtempo)
		song->timetrack.RemoveTempo(newtempo);

	song->timetrack.Close();

	//AddSubUndoFunctions();
}

void Undo_CreateTempo::UndoEnd()
{
	song->timetrack.RepairTempomap();
}

bool Undo_CreateTempo::RefreshUndo()
{
	if(inundo==false /* || song->timetrack.tempomap.GetIx(newtempo)>=0*/ )
		return true;

	return false;
}

void Undo_CreateTempo::Do()
{
	song->BufferBeforeTempoChanges();
	newtempo=song->timetrack.AddNewTempo(TEMPOEVENT_REAL,position,tempo);
	song->timetrack.Close();

	AddSubRedoFunctions();
}

void Undo_CreateTempo::DoEnd()
{
	song->timetrack.RepairTempomap();
}

void EditFunctions::CreateNewTempo(Seq_Song *song,OSTART pos,double tempo,bool addtolastundo)
{
	TRACE ("Create New Tempo %d\n",addtolastundo);

	if(CheckIfEditOK(song)==true && (!(song->status&Seq_Song::STATUS_WAITPREMETRO)))
	{
		if(Undo_CreateTempo *nct=new Undo_CreateTempo(song,pos,tempo))
			OpenLockAndDo(song,nct,true,addtolastundo);
	}
}