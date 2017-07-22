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
#include "undofunctions_text.h"
#include "editbuffer.h"
#include "MIDIoutproc.h"

void Undo_EditText::DoUndo()
{
	if(!edittext->newevents)
	{
		if(edittext->newevents=new Seq_Event*[edittext->counter])
			for(int i=0;i<edittext->counter;i++)
				edittext->newevents[i]=(Seq_Event *)edittext->event_p[i]->Clone(song);
	}

	if(edittext->newevents)
	{
		for(int i=0;i<edittext->counter;i++)
		{
			if(OSTART diff=edittext->oldevents[i]->ostart-edittext->newevents[i]->ostart)
				edittext->event_p[i]->MoveEvent(diff);

			edittext->oldevents[i]->CloneData(song,edittext->event_p[i]);
		}
	}
}

bool Undo_EditText::RefreshUndo()
{
	return true;
}

void Undo_EditText::FreeData()
{
	edittext->Delete();
}

void Undo_EditText::Do()
{
	if(edittext->newevents)
	{
		// Move ?
		for(int i=0;i<edittext->counter;i++)
		{
			if(OSTART diff=edittext->newevents[i]->ostart-edittext->oldevents[i]->ostart)
				edittext->event_p[i]->MoveEvent(diff);

			edittext->newevents[i]->CloneData(song,edittext->event_p[i]);
		}
	}
}

void EditFunctions::DeleteSelectedTexts(Seq_Song *song,bool addtoundo)
{
	if(CheckIfEditOK(song)==true)
	{
		int c=0;

		Seq_Text *t=song->textandmarker.FirstText();

		while(t)
		{
			if(t->flag&OFLAG_SELECTED)
				c++;

			t=t->NextText();
		}

		if(c>0)
		{
			UndODeInitText *pp=new UndODeInitText[c],*pointer;

			if(pointer=pp)
			{			
				Seq_Text *t=song->textandmarker.FirstText();

				while(t)
				{
					if(t->flag&OFLAG_SELECTED)
					{
						pp->text=t;
						pp++;
					}

					t=t->NextText();
				}

				if(Undo_DeleteTexts *dt=new Undo_DeleteTexts(song,pointer,c))
					OpenLockAndDo(song,dt,true);
				else
					delete pointer;
			}
		}
	}
}


void Undo_DeleteTexts::DoUndo()
{
	UndODeInitText *udt=texts;
	int i=numberoftexts;

	while(i--)
	{
		udt->text->flag CLEARBIT OFLAG_SELECTED;
		song->textandmarker.text.AddOSort(udt->text,udt->text->GetTextStart());
		udt++;
	}

	//AddSubUndoFunctions();
}

void Undo_DeleteTexts::UndoEnd()
{
}

void Undo_DeleteTexts::FreeData()
{
	if(texts)
	{
		if(inundo==true)
		{
			UndODeInitText *dt=texts;
			int c=this->numberoftexts;

			while(c--){

				dt->text->FreeMemory();
				delete dt->text;
				dt++;
			}
		}

		delete texts;
	}
}

bool Undo_DeleteTexts::RefreshUndo()
{
	if(inundo==true)
		return true;

	return true;
}

void Undo_DeleteTexts::Do()
{
	for(int i=0;i<numberoftexts;i++)
	{
		if(song->textandmarker.lastselectedtext==texts[i].text)
			song->textandmarker.lastselectedtext=0;

		song->textandmarker.text.CutQObject(texts[i].text);
	}
}

void Undo_DeleteTexts::DoEnd()
{
}
