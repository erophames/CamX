#ifndef CAMX_UNDO
#define CAMX_UNDO 1

#include "object.h"
#include <string.h>

class guiWindow;
class Seq_Song;
class Seq_Track;
class AutomationTrack;
class Seq_SelectionList;
class Seq_Event;
class Seq_Pattern;
class MIDIPatten;
class MIDIEffects;
class MIDIPattern;
class AudioObject;
class AudioChannel;
class AudioPattern;
class UndoFunction;
class Seq_Tempo;

class UndoEdit // Main Edit
{
public:
	UndoEdit()
	{
		oldindex=newindex=0;
		counter=0;
		oldevents=0;
		newevents=0;
		event_p=0;
	}

	Seq_Event **oldevents,**newevents,**event_p;
	int *oldindex,*newindex,counter;

	bool Init(int c);
	virtual bool CheckChanges();
	virtual void Delete();
	virtual UndoFunction *CreateUndoFunction(){return 0;}
};

class UndoFunction:public Object
{
	friend class Undo;

public:
	UndoFunction();
	~UndoFunction();


#ifdef _DEBUG
	char n[4];
#endif
	
	virtual void FreeData(){}
	virtual void Do(){}
	virtual void DoEnd(){};
	virtual void UndoGUI(){}
	virtual void DoUndo(){}
	virtual void UndoEnd(){}
	virtual void DoRedo(){Do();}
	virtual void AddedToUndo(){}
	virtual bool RefreshUndo(){return true;} // false=delete function
	virtual void RefreshGUI(bool undorefresh){}
	virtual void RefreshDo(){}
	virtual void RefreshPreUndo(){}
	virtual void RefreshPostUndo(){}
	virtual void RefreshPreRedo(){}

	char *GetUndoName(){return name;}
	void CreateCrossFadeBackUp(OList *);
	void DoUndoCrossFades(OList *);
	void CreateUndoString(char *);
	bool CheckPattern(Seq_Pattern *);
	UndoFunction *NextFunction(){return (UndoFunction *)next;}
	UndoFunction *PrevFunction(){return (UndoFunction *)prev;}
	UndoFunction *DeleteUndoFunction(bool full,bool cutlist);
	bool GetOpenStatus(){return open;}
	void CloseUndoFunction(){open=false;}

	// Subfunctions
	void AddFunction(UndoFunction *);
	UndoFunction *FirstFunction() {return (UndoFunction *)morefunctions.GetRoot();}
	UndoFunction *LastFunction() {return (UndoFunction *)morefunctions.Getc_end();}
	void AddSubUndoFunctions();
	void AddSubRedoFunctions();

	Seq_Song *song;
	Undo *undo;
	UndoFunction *parent;
	int id;
	bool doundo,inundo,canredo,dead,nodo;

protected:
	void DeleteTrack(Seq_Track *);
	void RefreshTrack(Seq_Track *);
	void AddClones(Seq_Track *);

private:
	OList morefunctions,crossfades_undo,crossfades_redo;
	char *name;
	bool open,tolist;
};

class Undo
{
public:
	Undo()
	{
		undo_menustring=0;
		redo_menustring=0;
	}

	~Undo()
	{
		if(undo_menustring)delete undo_menustring;
		if(redo_menustring)delete redo_menustring;
	}

	enum UndoIDs{
		UID_DELETEEVENT,
		UID_DELETEPATTERN,
		UID_INSERTSOUNDFILE,
		UID_SPLITTRACK,
		UID_DELETEALLEMPTYTRACKS,
		UID_DELETETRACK,
		UID_DELETESELECTEDTRACKS,
		UID_CREATENEWTRACK,
		UID_CREATENEWEVENTS,
		UID_PASTEPATTERN,
		UID_CREATENEWPATTERN,
		UID_QUANTIZETRACK,
		UID_QUANTIZEPATTERN,
		UID_QUANTIZESTARTPATTERN,
		UID_RECORDPATTERN,
		UID_MOVEPATTERN,
		UID_SIZEOFNOTES,
		UID_MOVEEVENTS,
		UID_LOOPPATTERN,
		UID_CREATEAUTOMATIONTRACK,
		UID_MUTEPATTERN,
		UID_COPYPATTERN,
		UID_EDITEVENTS,
		UID_CHANGEAUDIOTRACKCHANNEL,
		UID_SPLITPATTERN,
		UID_FLIPPATTERN,
		UID_STRETCHPATTERN,
		UID_PASTEPATTERNBUFFER,
		UID_CREATEFOLDER,
		UID_COPYEVENTS,
		UID_CUTPATTERN,
		UID_PASTETRACK,
		UID_MOVETRACK,
		UID_EDITTEMPOS,
		UID_CREATENEWTEMPOS,
		UID_MOVETEMPOS,
		UID_DELETETEMPOS,
		UID_CUTNOTE,
		UID_DELETEAUTOMATIONTRACK,
		UID_CREATEAUTOMATIONPARAMETER,
		UID_CHANGEAUTOMATIONTRACK,
		UID_DELETEAUTOMATIONPARAMETER,
		UID_RESETAUTOMATIONPARAMETER,
		UID_CONVERTLOOPPATTERN,
		UID_CONVERTCLONEPATTERN,
		UID_MOVEAUTOMATIONPARAMETER,
		UID_CLONEPATTERN,
		UID_DELETELASTRECORD,
		UID_SETNOTELENGTH,
		UID_QUANTIZEEVENT,
		UID_ADDLOOPSTOPATTERN,
		UID_SORTTRACKS,
		UID_REMOVEAUDIOREGION,
		UID_CONVERTDRUMSTONOTES,
		UID_CONVERTNOTESTODRUMS,
		UID_SETMIDIOUT,
		UID_SETAUDIOIO,
		UID_MIXSELPATTERNTOPATTERN,
		UID_CREATENEWTRACKS,
		UID_SETMIDIIN,
		UID_REPLACEAUDIOPATTERNFILE,
		UID_EDITTEXT,
		UID_EDITMARKER,
		UID_DELETETEXT,
		UID_DELETEMARKER,
		UID_SIZEPATTERN,
		UID_ADDASCHILDTRACK,
		UID_REMOVETRACKSFROMPARENT,
		UID_CREATEPARENTTRACK,
		UID_DELETEPLUGIN,
		UID_CREATEPLUGIN,
		UID_REPLACEPLUGIN,
		UID_CREATEPLUGINS, //Group
		UID_DELETEPLUGINS, //Group
		UID_CREATEBUS,
		UID_DELETEBUS,
	};

	void RefreshUndos(); // check undo functions
	void OpenUndoFunction(UndoFunction *,bool addtolastundo=false);

	void CloseLastUndoFunction()
	{
		if(UndoFunction *uf=LastUndo())uf->CloseUndoFunction();
	}

	UndoFunction *DeleteUndoFunction(UndoFunction *uf)
	{
		if(!uf)return 0;
		UndoFunction *rf=(UndoFunction *)undos.CutObject(uf);
		uf->DeleteUndoFunction(true,false);
		return rf;
	}

	bool DoUndo();
	bool DoRedo();
	void DeleteAllUndos();
	UndoFunction *FirstUndo() {return (UndoFunction *)undos.GetRoot();}
	UndoFunction *FirstRedo() {return (UndoFunction *)redos.GetRoot();}
	UndoFunction *LastUndo() {return (UndoFunction *)undos.Getc_end();}
	UndoFunction *LastRedo() {return (UndoFunction *)redos.Getc_end();}
	int GetNrUndos(){return undos.GetCount();}
	int GetNrRedos(){return redos.GetCount();}
	char *GetUndoString();
	char *GetRedoString();

	Seq_Song *song;
	char *undo_menustring,*redo_menustring;

private:	
	OList undos,redos, //,// In Undo Objects
		tracks;
};
#endif
