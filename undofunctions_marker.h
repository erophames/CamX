#ifndef CAMX_UNDOFUNCTIONSTEXT_H
#define CAMX_UNDOFUNCTIONSTEXT_H 1

#include "defines.h"
#include "undo.h"
#include "languagefiles.h"

class UndoEditMarker:public UndoEdit
{
public:
	UndoFunction *CreateUndoFunction();
};

class Undo_EditMarker:public UndoFunction
{
public:
	Undo_EditMarker(UndoEditMarker *ee)
	{
		id=Undo::UID_EDITMARKER;
		edittext=ee;
	}
	bool RefreshUndo(); //

	//	void RefreshGUI(bool undorefresh);

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_EDITMARKEREVENTS]);}

	void FreeData();

	UndoEditMarker *edittext;
};


class UndODeInitMarker
{
public:
	Seq_Marker *marker;
};

class Undo_DeleteMarkers:public UndoFunction
{
public:
	Undo_DeleteMarkers(Seq_Song *s,UndODeInitMarker *te,int tnr)
	{
		id=Undo::UID_DELETEMARKER;
		song=s;
		markers=te;
		numberofmarkers=tnr;
	}

	bool RefreshUndo(); //v
	void Do();
	void DoEnd();

	void DoUndo();
	void UndoEnd();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_DELETEMARKEREVENTS]);}

	void FreeData();
	UndODeInitMarker *markers;
	int numberofmarkers;
};

#endif