#ifndef CAMX_UNDOFUNCTIONS_PLUGINS_H
#define CAMX_UNDOFUNCTIONS_PLUGINS_H 1

#include "defines.h"
#include "undo.h"
#include "languagefiles.h"


class Undo_ReplacePlugin:public UndoFunction
{
public:
	Undo_ReplacePlugin(Seq_Song *,InsertAudioEffect *oldae,InsertAudioEffect *newae);
	
	void Do();
	void DoUndo();
	void RefreshPreRedo();
	void RefreshPreUndo();

	void AddedToUndo(){CreateUndoString(Cxs[CXS_REPLACEPLUGIN]);}
	void FreeData();

	AudioEffects *effects;
	AudioObject *old_audioobject,*new_audioobject;
	AutomationTrack **automationtracks;

	int automationtrack_c;
	bool skipinsert;
};

class Undo_CreatePlugin:public UndoFunction
{
public:
	Undo_CreatePlugin(Seq_Song *,InsertAudioEffect *);
	Undo_CreatePlugin(){}

	void Init(Seq_Song *,InsertAudioEffect *);
	void Do();
	void DoUndo();
	void RefreshPreUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_CREATEPLUGIN]);}
	void FreeData();

	AudioEffects *effects;
	AudioObject *audioobject;
};

class Undo_CreatePlugins:public UndoFunction // Group
{
public:
	Undo_CreatePlugins(Seq_Song *,AudioEffects *,AudioObject **,int count);
	void AddedToUndo(){CreateUndoString(Cxs[CXS_PASTINSTRUMENTSANDEFFECT]);}
	void FreeData();

	void RefreshPreUndo();
	void DoUndo();
	void Do();

	AudioEffects *effects;
	AudioObject **audioobjects;
	int counter;
};


class Undo_DeletePlugin:public UndoFunction
{
public:
	Undo_DeletePlugin(Seq_Song *,InsertAudioEffect *);
	
	void RefreshPreRedo();
	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_DELETEPLUGIN]);}
	void FreeData();

	AudioEffects *effects;
	AudioObject *audioobject;
	AutomationTrack **automationtracks;
	InsertAudioEffect *addnext;

	int automationtrack_c;
};

class Undo_DeletePlugins:public UndoFunction
{
public:
	Undo_DeletePlugins(Seq_Song *,AudioEffects *,AudioObject **,int c);
	
	void RefreshPreRedo();
	void Do();
	void DoUndo();
	void AddedToUndo(){CreateUndoString(Cxs[CXS_DELETEALLIE]);}
	void FreeData();

	AudioEffects *effects;
	AudioObject **audioobjects;
	int count,*autos;

	AutomationTrack **automationtracks;
	int automationtrack_c;

	//InsertAudioEffect *insertaudioeffect,*next;
	//AudioObject *audioobject;
	//AutomationTrack **automationtracks;
	//int automationtrack_c;
};

#endif