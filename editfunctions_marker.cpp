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
#include "undofunctions_marker.h"
#include "editbuffer.h"
#include "MIDIoutproc.h"

void Undo_EditMarker::DoUndo()
{
	if(!edittext->newevents)
	{
		if(edittext->newevents=new Seq_Event*[edittext->counter])
			for(int i=0;i<edittext->counter;i++)
				edittext->newevents[i]=(Seq_Event *)edittext->event_p[i]->Clone(0);
	}

	if(edittext->newevents)
	{
		for(int i=0;i<edittext->counter;i++)
		{
			if(OSTART diff=edittext->oldevents[i]->ostart-edittext->newevents[i]->ostart)
				edittext->event_p[i]->MoveEvent(diff);

			edittext->oldevents[i]->CloneData(0,edittext->event_p[i]);
		}
	}
}

bool Undo_EditMarker::RefreshUndo()
{
	return true;
}

void Undo_EditMarker::FreeData()
{
	edittext->Delete();
}

void Undo_EditMarker::Do()
{
	if(edittext->newevents)
	{
		// Move ?
		for(int i=0;i<edittext->counter;i++)
		{
			if(OSTART diff=edittext->newevents[i]->ostart-edittext->oldevents[i]->ostart)
				edittext->event_p[i]->MoveEvent(diff);

			edittext->newevents[i]->CloneData(0,edittext->event_p[i]);
		}
	}
}


void EditFunctions::DeleteSelectedMarkers(Seq_Song *song,bool addtoundo)
{
	if(CheckIfEditOK(song)==true)
	{
		int c=0;

		Seq_Marker *t=song->textandmarker.FirstMarker();

		while(t)
		{
			if(t->flag&OFLAG_SELECTED)
				c++;

			t=t->NextMarker();
		}

		if(c>0)
		{
			UndODeInitMarker *pp=new UndODeInitMarker[c],*pointer;

			if(pointer=pp)
			{			
				Seq_Marker *t=song->textandmarker.FirstMarker();

				while(t)
				{
					if(t->flag&OFLAG_SELECTED)
					{
						pp->marker=t;
						pp++;
					}

					t=t->NextMarker();
				}

				if(Undo_DeleteMarkers *dt=new Undo_DeleteMarkers(song,pointer,c))
					OpenLockAndDo(song,dt,true);
				else
					delete pointer;
			}
		}
	}
}


void Undo_DeleteMarkers::DoUndo()
{
	UndODeInitMarker *udt=markers;
	int i=numberofmarkers;

	while(i--)
	{
		udt->marker->UnSelect();
		song->textandmarker.marker.AddOSort(udt->marker,udt->marker->GetMarkerStart());
		udt++;
	}

	//AddSubUndoFunctions();
}

void Undo_DeleteMarkers::UndoEnd()
{
}

void Undo_DeleteMarkers::FreeData()
{
	if(markers)
	{
		if(inundo==true)
		{
			UndODeInitMarker *dt=markers;
			int c=this->numberofmarkers;

			while(c--){
				dt->marker->FreeMemory();
				delete dt->marker;
				dt++;
			}
		}

		delete markers;
	}
}

bool Undo_DeleteMarkers::RefreshUndo()
{
	if(inundo==true)
		return true;

	/*
	UndODeInitText *udt=texts;
	int i=numberoftexts;

	while(i--)
	{
		if(song->textandmarker.GetIx(udt->text)==-1)
			return false;
		udt++;
	}
*/
	return true;
}

void Undo_DeleteMarkers::Do()
{
	for(int i=0;i<numberofmarkers;i++)
	{
		if(song->textandmarker.lastselectedmarker==markers[i].marker)
			song->textandmarker.lastselectedmarker=0;

		song->textandmarker.marker.CutQObject(markers[i].marker);
	}
}

void Undo_DeleteMarkers::DoEnd()
{
}

