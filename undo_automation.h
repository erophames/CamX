#ifndef CAMX_UNDOFUNCTIONS_AUTOMATION_H
#define CAMX_UNDOFUNCTIONS_AUTOMATION_H 1

#include "defines.h"
#include "undo.h"
#include "languagefiles.h"

class Undo_ChangeAutomationParameter:public UndoFunction
{
public:

	Undo_ChangeAutomationParameter(AutomationTrack *at)
	{
		id=Undo::UID_MOVEAUTOMATIONPARAMETER;
		automationtrack=at;
		at->CloneParameter(&parameter);
	}

	void Do();
	void DoEnd();

	void DoUndo();
	void UndoEnd();

	void CheckAndAddToUndo();
	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_MOVEAUTOMATIONOBJ]);
	}

	void FreeData()
	{
		parameter.DeleteAllO();
		parameterundo.DeleteAllO();
	}

	void Reset();

	OListStart parameter,parameterundo;
	AutomationTrack *automationtrack;
};

class Undo_CreateAutomationTrack:public UndoFunction
{
public:
	Undo_CreateAutomationTrack(AutomationTrack *t)
	{
		id=Undo::UID_CREATEAUTOMATIONTRACK;
		automationtrack=t;
	}

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_CREATENEWAUTOTRACK]);
	}

	void DoUndo();
	void DoRedo();

	void FreeData()
	{
		if(inundo==false)
		{
			delete automationtrack;
			automationtrack=0;
		}
	}

	AutomationTrack *automationtrack;
	int automationtrackindex;
};

class Undo_CreateAutomationParameter:public UndoFunction
{
public:
	Undo_CreateAutomationParameter(AutomationTrack *t,AutomationParameter *ap)
	{
		id=Undo::UID_CREATEAUTOMATIONPARAMETER;
		automationtrack=t;
		automationparameter=ap;
	}

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_CREATEAUTOOBJECTS]);
	}

	void Do();
	void DoEnd();
	void DoUndo();
	void UndoEnd();

	void FreeData()
	{
		if(inundo==false && automationparameter)
		{
			delete automationparameter;
			automationparameter=0;
		}
	}

	AutomationTrack *automationtrack;
	AutomationParameter *automationparameter;
};

class Undo_DeleteAutomationParameter:public UndoFunction
{
public:
	Undo_DeleteAutomationParameter(AutomationTrack *at)
	{
		id=Undo::UID_DELETEAUTOMATIONPARAMETER;
		automationtrack=at;
		at->CloneParameter(&parameter);
	}

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_DELETEAUTOOBJ]);
	}

	void Do();
	void DoEnd();
	void DoUndo();
	void UndoEnd(); 
	void FreeData()
	{
		parameter.DeleteAllO();
		parameterundo.DeleteAllO();
	}

	OListStart parameter,parameterundo;
	AutomationTrack *automationtrack;
};

class Undo_ResetAutomationParameter:public UndoFunction
{
public:
	Undo_ResetAutomationParameter(AutomationTrack *at)
	{
		id=Undo::UID_RESETAUTOMATIONPARAMETER;
		automationtrack=at;
		at->CloneParameter(&parameter);
	}

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_RESETAUTOOBJ]);
	}

	void Do();
	void DoEnd();
	void DoUndo();
	void UndoEnd(); 
	void FreeData()
	{
		parameter.DeleteAllO();
		parameterundo.DeleteAllO();
	}

	OListStart parameter,parameterundo;
	AutomationTrack *automationtrack;
};

class Undo_DeleteAutomationTrack:public UndoFunction
{
public:
	Undo_DeleteAutomationTrack(TrackHead *t,AutomationTrack *sub,AutomationTrack *prev)
	{
		id=Undo::UID_DELETEAUTOMATIONTRACK;		
		track=t;
		automationtrack=sub;
		previoustrack=prev;
	}

	void Do();
	void DoUndo();

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_DELETEAUTOTRACK]);
	}

	void FreeData()
	{
		if(inundo==true)
			delete automationtrack;
	}

	TrackHead *track;
	AutomationTrack *automationtrack,*previoustrack;
};

#endif