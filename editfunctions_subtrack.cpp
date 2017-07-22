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
#include "undofunctions.h"
#include "undo_automation.h"
#include "folder.h"
#include "MIDIoutproc.h"
#include "editbuffer.h"
#include "initplayback.h"
#include "object_track.h"

void Undo_DeleteAutomationParameter::DoUndo()
{
	automationtrack->parameter.MoveListToList(&parameterundo);
	parameter.MoveListToList(&automationtrack->parameter);
	automationtrack->parameter.EndIndex();
	automationtrack->DeSelectAllParameter();
}

void Undo_DeleteAutomationParameter::UndoEnd()
{
	automationtrack->Refresh();
}

void Undo_DeleteAutomationParameter::Do()
{
	automationtrack->parameter.MoveListToList(&parameter);
	parameterundo.MoveListToList(&automationtrack->parameter);
	automationtrack->parameter.EndIndex();
	automationtrack->DeSelectAllParameter();
}

void Undo_DeleteAutomationParameter::DoEnd()
{
	automationtrack->Refresh();
}

void EditFunctions::DeleteAutomationParameter(Seq_Song *song,AutomationTrack *at,AutomationParameter *autop)
{
	if(CheckIfEditOK(song)==true)
	{
		bool isselected=false;

		if(autop && (!(autop->flag&AutomationParameter::AP_DONTDELETE)))
			goto selectedfound;
		else
		{
			if(!at)
			{
				at=song->FirstAutomationTrackWithSelectedParameters();
				if(at)
					goto selectedfound;
			}

			if(at)
			{
				AutomationParameter *ap=at->FirstAutomationParameter();
				while(ap)
				{
					if(ap->IsSelected()==true && (!(ap->flag&AutomationParameter::AP_DONTDELETE)) )
					goto selectedfound;

					ap=ap->NextAutomationParameter();
				}
			}
		}

		return;

selectedfound:

		if(Undo_DeleteAutomationParameter *nct=new Undo_DeleteAutomationParameter(at))
		{
			song->undo.OpenUndoFunction(nct);

			if(song==mainvar->GetActiveSong())
				mainthreadcontrol->LockActiveSong();

			if(autop)
				at->DeleteParameter(autop,true);
			else
			{
				AutomationParameter *ap=at->FirstAutomationParameter();
				while(ap)
				{
					AutomationParameter *np=ap->NextAutomationParameter();

					if(ap->IsSelected()==true && (!(ap->flag&AutomationParameter::AP_DONTDELETE)) )
						at->DeleteParameter(ap,true);

					ap=np;
				}
			}

			at->parameter.EndIndex();
			nct->DoEnd(); 

			if(song==mainvar->GetActiveSong())
				mainthreadcontrol->UnlockActiveSong();

			CheckEditElementsForGUI(song,nct,true);
		}
	}
}

void Undo_ResetAutomationParameter::DoUndo()
{
	automationtrack->parameter.MoveListToList(&parameterundo);
	parameter.MoveListToList(&automationtrack->parameter);
	automationtrack->parameter.EndIndex();
}

void Undo_ResetAutomationParameter::UndoEnd()
{
	automationtrack->Refresh();
}

void Undo_ResetAutomationParameter::Do()
{
	automationtrack->parameter.MoveListToList(&parameter);
	parameterundo.MoveListToList(&automationtrack->parameter);
	automationtrack->parameter.EndIndex();
}

void Undo_ResetAutomationParameter::DoEnd()
{
	automationtrack->Refresh();
}

void EditFunctions::ResetAutomationParameter(Seq_Song *song,AutomationTrack *at,AutomationParameter *apar)
{
	AutomationParameter *ap=at->FirstAutomationParameter();
	while(ap)
	{
		if(ap->IsSelected()==true)
			goto ok;

		ap=ap->NextAutomationParameter();
	}

	return;

ok:

	if(Undo_ResetAutomationParameter *nct=new Undo_ResetAutomationParameter(at))
	{
		Seq_Song *song=at->GetSong();

		song->undo.OpenUndoFunction(nct);

		if(song==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		AutomationParameter *ap=at->FirstAutomationParameter();
		while(ap)
		{
			if(ap->IsSelected()==true)
				at->ResetAutomationParameter(ap);

			ap=ap->NextAutomationParameter();
		}

		at->parameter.accesscounter++;
		at->parameter.EndIndex();
		nct->DoEnd(); 

		if(song==mainvar->GetActiveSong())
			mainthreadcontrol->UnlockActiveSong();

		CheckEditElementsForGUI(song,nct,true);
	}
}

void Undo_CreateAutomationParameter::DoUndo()
{
	automationtrack->DeleteParameter(automationparameter,false);
	automationtrack->parameter.EndIndex();
}

void Undo_CreateAutomationParameter::UndoEnd()
{
	automationtrack->Refresh();
}

void Undo_CreateAutomationParameter::Do()
{
	automationtrack->SortParameter(automationparameter);
	automationtrack->parameter.EndIndex();
}

void Undo_CreateAutomationParameter::DoEnd()
{
	automationtrack->Refresh();
}

void EditFunctions::CreateAutomationParameter(Seq_Song *song,AutomationTrack *t,AutomationParameter *ap)
{
	if(Undo_CreateAutomationParameter *nct=new Undo_CreateAutomationParameter(t,ap))
	{
		song->undo.OpenUndoFunction(nct);
		CheckEditElementsForGUI(song,nct,true);
	}
}

void EditFunctions::CreateAutomationTrack(AutomationTrack *track)
{
	if(Undo_CreateAutomationTrack *cat=new Undo_CreateAutomationTrack(track))
	{
		track->GetSong()->undo.OpenUndoFunction(cat);
		CheckEditElementsForGUI(track->GetSong(),cat,true);
	}
}

void Undo_ChangeAutomationParameter::Do()
{
	automationtrack->parameter.MoveListToList(&parameter);
	parameterundo.MoveListToList(&automationtrack->parameter);
	automationtrack->parameter.EndIndex();

	AutomationParameter *ap=automationtrack->FirstAutomationParameter();

	while(ap)
	{
		ap->UnSelect();
		ap->flag CLEARBIT OFLAG_UNDERSELECTION;
		ap->flag CLEARBIT OFLAG_MOUSEOVER;

		ap=ap->NextAutomationParameter();
	}
}

void Undo_ChangeAutomationParameter::DoEnd()
{
	automationtrack->Refresh();
}

void Undo_ChangeAutomationParameter::DoUndo()
{
	automationtrack->parameter.MoveListToList(&parameterundo);
	parameter.MoveListToList(&automationtrack->parameter);
	automationtrack->parameter.EndIndex();

	AutomationParameter *ap=automationtrack->FirstAutomationParameter();

	while(ap)
	{
		ap->UnSelect();
		ap->flag CLEARBIT OFLAG_UNDERSELECTION;
		ap->flag CLEARBIT OFLAG_MOUSEOVER;

		ap=ap->NextAutomationParameter();
	}
}

void Undo_ChangeAutomationParameter::UndoEnd()
{
	automationtrack->Refresh();
}

void Undo_ChangeAutomationParameter::Reset() // break, cancel
{
	mainthreadcontrol->LockActiveSong();

	automationtrack->parameter.DeleteAllO();
	parameter.MoveListToList(&automationtrack->parameter);
	automationtrack->parameter.EndIndex();

	automationtrack->Refresh();

	mainthreadcontrol->UnlockActiveSong();

	FreeData();
	delete this;
}

void Undo_ChangeAutomationParameter::CheckAndAddToUndo()
{
	{
		AutomationParameter *cp=automationtrack->FirstAutomationParameter();

		while(cp)
		{
			cp->flag CLEARBIT AutomationParameter::AP_COPYIED;
			cp=cp->NextAutomationParameter();
		}
	}

	AutomationParameter *cp=automationtrack->FirstAutomationParameter();
	AutomationParameter *ap=(AutomationParameter *)parameter.GetRoot();

	while(cp && ap)
	{
		if(cp->GetParameterStart()!=ap->GetParameterStart() ||
			cp->GetParameterValue()!=ap->GetParameterValue() ||
			cp->curvetype!=ap->curvetype)
			goto changed;

		ap=ap->NextAutomationParameter();
		cp=cp->NextAutomationParameter();
	}

	if(cp)
		goto changed;

	if(ap)
		goto changed;

	FreeData();
	delete this;

	return;

changed:

	automationtrack->CloneParameter(&parameterundo);
	song->undo.OpenUndoFunction(this);
	mainedit->CheckEditElementsForGUI(song,this,true);
}

Undo_ChangeAutomationParameter *EditFunctions::InitChangeAutomation(AutomationTrack *at)
{
	if(at)
	{
		Seq_Song *song=at->GetSong();

		if(Undo_ChangeAutomationParameter *cap=new Undo_ChangeAutomationParameter(at))
		{
			cap->song=song;
			return cap;
		}
	}

	return 0;
}

void Undo_DeleteAutomationTrack::Do()
{
	track->automationtracks.CutQObject(automationtrack);
}

void Undo_DeleteAutomationTrack::DoUndo()
{	
	track->automationtracks.AddPrevO(automationtrack,previoustrack);
}

void EditFunctions::DeleteAutomationTrack(AutomationTrack *at)
{
	if(at && CheckIfEditOK(at->GetSong())==true)
	{
		if(at->audiochannel)
			OpenLockAndDo(at->GetSong(),new Undo_DeleteAutomationTrack(at->audiochannel,at,at->PrevAutomationTrack()),true);
		else
			OpenLockAndDo(at->GetSong(),new Undo_DeleteAutomationTrack(at->track,at,at->PrevAutomationTrack()),true);
	}
}

void Undo_CreateAutomationTrack::DoUndo()
{
	automationtrackindex=automationtrack->GetIndex();

	if(automationtrack->track)
	{	
		automationtrack->track->DeleteAutomationTrack(automationtrack,false);
	}
	else
	{
		automationtrack->audiochannel->DeleteAutomationTrack(automationtrack,false);
	}
}

void Undo_CreateAutomationTrack::DoRedo()
{
	if(automationtrack->track)
	{
		automationtrack->track->automationtracks.AddOToIndex(automationtrack,automationtrackindex);
	}
	else
	{
		automationtrack->audiochannel->automationtracks.AddOToIndex(automationtrack,automationtrackindex);
	}
}