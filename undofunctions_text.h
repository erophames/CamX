#ifndef CAMX_UNDOFUNCTIONSTEXT_H
#define CAMX_UNDOFUNCTIONSTEXT_H 1

#include "defines.h"
#include "undo.h"
#include "languagefiles.h"

class UndoEditText:public UndoEdit
{
public:
	UndoFunction *CreateUndoFunction();
};

class Undo_EditText:public UndoFunction
{
public:
	Undo_EditText(UndoEditText *ee)
	{
		id=Undo::UID_EDITTEXT;
		edittext=ee;
	}
	bool RefreshUndo(); //

	//	void RefreshGUI(bool undorefresh);

	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_EDITTEXTEVENTS]);}

	void FreeData();

	UndoEditText *edittext;
};

class UndODeInitText
{
public:
	Seq_Text *text;
};

class Undo_DeleteTexts:public UndoFunction
{
public:
	Undo_DeleteTexts(Seq_Song *s,UndODeInitText *te,int tnr)
	{
		id=Undo::UID_DELETETEXT;
		song=s;
		texts=te;
		numberoftexts=tnr;
	}

	bool RefreshUndo(); //v
	void Do();
	void DoEnd();

	void DoUndo();
	void UndoEnd();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_DELETETEXTEVENTS]);}

	//void RefreshGUI(bool undorefresh);
	void FreeData();

	UndODeInitText *texts;
	int numberoftexts;
};
#endif