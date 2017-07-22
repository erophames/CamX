#ifndef CAMX_TEMPOEDITORLIST_H
#define CAMX_TEMPOEDITORLIST_H 1

#include "editor.h"

class Edit_Tempo;

class Edit_TempoList_Tempo:public NumberOListStartPosition
{
	friend class Edit_TempoList;

public:
	Edit_TempoList_Tempo();

	void ShowTempo();

	Edit_TempoList *editor;
	Seq_Tempo *tempo;
	int eflag;
};

class Edit_TempoList:public EventEditor
{
public:
	Edit_TempoList(Edit_Tempo *);

	void InitTabs();
	void Init();
	void DeInitWindow();
	void FreeEditorMemory();

	void Gadget(guiGadget *);
	void ShowList();
	void RefreshObjects(LONGLONG type,bool editcall);
	void ShowVSlider();
	void BuildTempoList();

	void RefreshRealtime();
	void RefreshRealtime_Slow();
	void MouseClickInTempos(bool leftmouse);
	void MouseReleaseInTempos(bool leftmouse);
	
	void StartOfNumberEdit(guiGadget *);
	void EditEditorPositions(guiGadget *);
	void EndOfPositionEdit(guiGadget *,OSTART sum);
	void DeltaY(guiGadget_TabStartPosition *);

	OListCoosY tempoobjects;
	guiGadget_TabStartPosition *list;
	Edit_Tempo *editor;
};

#endif