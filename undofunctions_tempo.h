#ifndef CAMX_UNDOFUNCTIONSTEMPO_H
#define CAMX_UNDOFUNCTIONSTEMPO_H 1

#include "defines.h"
#include "undo.h"
#include "languagefiles.h"

class Undo_EditTempo:public UndoEdit
{
public:
	Undo_EditTempo()
	{
		lpos=hpos=-1;
		ltempo=htempo=-1;
	}
	UndoFunction *CreateUndoFunction();
	OSTART lpos,hpos;
	double ltempo,htempo;
};

class Undo_CreateTempo:public UndoFunction
{
public:
	Undo_CreateTempo(Seq_Song *s,OSTART pos,double temp)
	{
		id=Undo::UID_CREATENEWTEMPOS;
		song=s;
		position=pos;
		tempo=temp;
		refreshloopgui=true;
	}

	void AddedToUndo(){CreateUndoString(Cxs[CXS_CREATETEMPOEVENTS]);}

	bool RefreshUndo(); //
	void Do();
	void DoEnd();
	void DoUndo();
	void UndoEnd();

	Seq_Tempo *newtempo;
	OSTART position;
	double tempo;
	bool refreshloopgui;
};

class UndODeInitTempo
{
public:
	Seq_Tempo *tempo;
};

class Undo_DeleteTempos:public UndoFunction
{
public:
	Undo_DeleteTempos(Seq_Song *s,UndODeInitTempo *te,int tnr)
	{
		id=Undo::UID_DELETETEMPOS;
		song=s;
		tempos=te;
		numberoftempos=tnr;
		firsttempomoved=false;
		refreshloopgui=false;
	}

	bool RefreshUndo(); //v
	void Do();
	void DoEnd();
	void DoUndo();
	void UndoEnd();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_DELETETEMPOEVENTS]);}

	//void RefreshGUI(bool undorefresh);
	void FreeData();

	UndODeInitTempo *tempos;
	OSTART firsttempooldstart;
	int numberoftempos;
	bool firsttempomoved,refreshloopgui;
};
#endif