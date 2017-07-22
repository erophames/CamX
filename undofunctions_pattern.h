#ifndef CAMX_UNDOFUNCTIONS_PATTERN_H
#define CAMX_UNDOFUNCTIONS_PATTERN_H 1

#include "defines.h"
#include "undo.h"
#include "languagefiles.h"
#include "audioregion.h"

class Undo_SplitMIDIPattern:public UndoFunction
{
public:
	Undo_SplitMIDIPattern(Seq_Song *,MIDIPattern *from,int c,Seq_Track *to);

	void Do();
	void UndoGUI(); // v CLose Windows
	void DoUndo();
	void FreeData();

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_SPLITMIDIPatternTOCHANNELS]);
	}

	MIDIPattern *orgpattern;

	Seq_Track *newcreatedtrack[16], // MIDIChannel 1-16
		*totrack;
	MIDIPattern *frompattern,*newcreatedpattern;
	int splitchannel;
};

class Undo_SplitMIDIPattern_Types:public UndoFunction
{
public:
	Undo_SplitMIDIPattern_Types(MIDIPattern *from,int c,Seq_Track *to);

	void Do();
	void UndoGUI();
	void DoUndo();
	void FreeData();

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_SPLITMIDIPatternTOTYPES]);
	}

	MIDIPattern *orgpattern;

	Seq_Track *newcreatedtrack[16], // MIDIChannel 1-16
		*totrack;
	MIDIPattern *frompattern,*newcreatedpattern;
	int splitchannel;
};

class Undo_FlipMIDIPattern_Types:public UndoFunction
{
public:
	Undo_FlipMIDIPattern_Types(MIDIPattern *pattern)
	{
		id=Undo::UID_FLIPPATTERN;
		frompattern=pattern;
		newcreatedpattern=0;
		newset=false;
		initok=false;
	}

	void Do();
	void DoUndo();
	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_ROTATENOTES]);
	}

	void FreeData();

	OSTART start,end;
	MIDIPattern *frompattern,*newcreatedpattern;
	int newnrobjects,newvnrobjects,oldnrobjects,oldvnrobjects;
	Seq_Event *oldfirstevent,*oldlastevent,*oldvfirstevent,*oldvlastevent,
		*newfirstevent,*newlastevent,*newvfirstevent,*newvlastevent;
	bool newset,initok;
};

class Undo_StretchMIDIPattern:public UndoFunction
{
public:
	Undo_StretchMIDIPattern(MIDIPattern *pattern)
	{
		id=Undo::UID_STRETCHPATTERN;
		frompattern=pattern;
		newcreatedpattern=0;
		newset=false;
		initok=false;
	}

	void Do();
	void DoUndo();
	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_STRECHMIDIPattern]);
	}

	void FreeData();

	MIDIPattern *frompattern,*newcreatedpattern;
	Seq_Event *oldfirstevent,*oldlastevent,*oldvfirstevent,*oldvlastevent,
		*newfirstevent,*newlastevent,*newvfirstevent,*newvlastevent;

	int start,end,oldnrobjects,oldvnrobjects,newnrobjects,newvnrobjects;
	bool newset,initok;
};

class UndoCutPattern
{
public:
	UndoCutPattern()
	{
		pattern=0;
		region1=region2=0;
		regionpattern1=regionpattern2=bufferpattern=0;
		newMIDIPattern=0;
	}

	Seq_Pattern *pattern;

	// Audio
	AudioRegion *region1,*region2;
	AudioPattern *regionpattern1,*regionpattern2,*bufferpattern;

	// MIDI
	MIDIPattern *newMIDIPattern;
};

class UndoCopyPattern // and clone
{
public:
	UndoCopyPattern(){onlyclone=false;clone=0;}

	Seq_Pattern *pattern,*clone;
	Seq_Track *totrack;
	OSTART qposition;
	bool onlyclone;
};

class UndoMovePattern
{
public:
	UndoMovePattern(){ok=true;}

	Seq_Track *totrack,*oldtrack;
	Seq_Pattern *temp_pattern;
	OSTART oldposition;
	int index_oldtrack,index_pattern,index_newtrack,index_newpattern;
	bool ok;
};

class UndoMutePattern
{
public:
	Seq_Pattern *pattern;
	bool oldmute;
};

class Undo_ClonePattern:public UndoFunction
{
public:
	Undo_ClonePattern(Seq_Song *s,Seq_Track *t,UndoCopyPattern *p,int nr,OSTART diff,int index,int f)
	{
		id=Undo::UID_CLONEPATTERN;
		song=s;
		track=t;
		movepattern=p;
		numberofpattern=nr;
		movediff=diff;
		trackindex=index;
		flag=f;
	}	

	bool RefreshUndo();
	void Do();
	void DoUndo();
	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_CLONEPATTERN]);
	}

	void FreeData()
	{
		if(movepattern)delete movepattern;
	}

	Seq_Track *track;
	UndoCopyPattern *movepattern;
	OSTART movediff;
	int numberofpattern,trackindex;
};

class Undo_CopyPattern:public UndoFunction
{
public:
	Undo_CopyPattern(Seq_Song *s,Seq_Track *t,UndoCopyPattern *p,int nr,OSTART diff,int index,int f)
	{
		id=Undo::UID_COPYPATTERN;
		song=s;
		track=t;
		movepattern=p;
		numberofpattern=nr;
		movediff=diff;
		trackindex=index;
		flag=f;
	}

	bool RefreshUndo();
	void Do();
	void UndoGUI();
	void DoUndo();
	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_COPYPATTERN]);
	}
	void FreeData();

	Seq_Track *track;
	UndoCopyPattern *movepattern;
	OSTART movediff;
	int numberofpattern,trackindex;
};

class Undo_SizePattern:public UndoFunction
{
public:
	Undo_SizePattern()
	{
		id=Undo::UID_SIZEPATTERN;
	}

	void Do();
	void DoUndo();
	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_SIZEPATTERN]);
	}

	void FreeData()
	{
		oldoffsetregion.FreeMemory();
	}

	AudioRegion oldoffsetregion;
	Seq_Pattern *pattern;
	OSTART position,oldoffsetposition,offset,oldoffset_left,oldoffset_right;
	bool right,olduseoffsetregion;
};

class Undo_MovePattern:public UndoFunction
{
public:
	Undo_MovePattern(UndoMovePattern *p,int nr,OSTART diff,int index,int f,bool q)
	{
		id=Undo::UID_MOVEPATTERN;
		movepattern=p;
		numberofpattern=nr;
		movediff=diff;
		trackindex=index;
		qflag=f;
		quantize=q;
	}

	bool RefreshUndo();
	void Do();
	void DoUndo();
	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_MOVEPATTERN]);
	}

	void FreeData();


private:
	UndoMovePattern *movepattern;
	OSTART movediff;
	int numberofpattern,trackindex,qflag;
	bool quantize;
};

class Undo_MutePattern:public UndoFunction
{
public:
	Undo_MutePattern(UndoMutePattern *p,int nr,bool m)
	{
		id=Undo::UID_MUTEPATTERN;

		mutepattern=p;
		numberofpattern=nr;
		mute=m;
	}

	bool RefreshUndo();
	void Do();
	void DoUndo();
	void AddedToUndo()
	{
		CreateUndoString("Mute Pattern");
	}

	void FreeData()
	{
		if(mutepattern)
			delete mutepattern;
	}

	UndoMutePattern *mutepattern;
	int numberofpattern;
	bool mute;
};

class Undo_CutPattern:public UndoFunction
{
public:
	Undo_CutPattern(UndoCutPattern *up,int nr,OSTART pos,Seq_Pattern_VolumeCurve **oc);
	bool RefreshUndo();
	void Do();
	void DoUndo();
	void UndoGUI();
	void RefreshGUI(bool undorefresh);

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_CUTPATTERN]);
	}

	void FreeData();

	Seq_Pattern_VolumeCurve **curves;
	UndoCutPattern *cutpattern;
	OSTART cutposition;
	int counts;
};

class UndODeInitPattern
{
public:
	Seq_Pattern *pattern;
	Seq_Track *track;
};

class Undo_ConvertLoopPattern:public UndoFunction
{
public:
	Undo_ConvertLoopPattern(Seq_Pattern *p,int ix,bool conv);
	void FreeData();
	bool RefreshUndo();
	void Do();
	void DoUndo();
	void RefreshGUI(bool ur);

	void AddedToUndo()
	{
		CreateUndoString(Cxs[convert==true?CXS_F_CONVERTLOOPEVENT:CXS_F_CONVERTPATTERN]);
	}

	Seq_Pattern *mainpattern,*newpattern;
	MIDIPattern *oldpattern; // <-> Connect Buffer
	OSTART newposition;
	int oldloops,index;
	bool convert,oldloopwithloops,oldloopendless;
};

class Undo_ConvertClonePattern:public UndoFunction
{
public:
	Undo_ConvertClonePattern(Seq_Pattern *p);
	void FreeData();
	bool RefreshUndo();
	void Do();
	void DoUndo();

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_CONVERTCLONEREALPATTERN]);
	}

	Seq_Pattern *mainpattern,*pattern,*newpattern;
	Seq_Track *totrack;
	OSTART position;
};

class Undo_MixPatternToPattern:public UndoFunction
{
public:
	Undo_MixPatternToPattern(MIDIPattern **l,MIDIPattern *p,int c,bool a,bool con);
	void FreeData();

	void Do();
	void DoEnd();
	void DoUndo();

	void AddedToUndo()
	{
		CreateUndoString(Cxs[add==true?CXS_F_ADDPATTERNTOPATTERN:CXS_F_MIXPATTERNTOPATTERN]);
	}

	MIDIPattern **list,*topattern,*oldpattern;
	int nrpattern;
	bool add,connect,patternbuffered;
};

class Undo_AddLoopsToPattern:public UndoFunction
{
public:
	Undo_AddLoopsToPattern(MIDIPattern *p);
	void FreeData();

	void Do();
	void DoUndo();

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_CONVERTLOPPTOPATTERN]);
	}

	MIDIPattern *mainpattern,*oldpattern; // <-> Connect Buffer
	int oldloops;
	bool oldloopwithloops,oldloopendless;
};

class Undo_DeletePattern:public UndoFunction
{
public:
	Undo_DeletePattern(Seq_Song *s,UndODeInitPattern *p,int pnr)
	{
		id=Undo::UID_DELETEPATTERN;
		song=s;
		pattern=p;
		patternnumber=pnr;
	}

	bool RefreshUndo();
	void Do();
	void DoUndo();
	void RefreshGUI(bool undorefresh);

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_DELETEPATTERN]);
	}

	void FreeData();

	UndODeInitPattern *pattern; // Array Pointer Seq_Pattern
	int patternnumber;
};

class UndoCreatePattern
{
public:
	UndoCreatePattern(){newpattern=0;recordeventsadded=false;recordeventsaddedcounter=0;recordeventslist=0;mainclonepattern=0;}

	Seq_Track *track;
	Seq_Pattern *newpattern,*mainclonepattern;
	Seq_Event **recordeventslist;
	OSTART position;
	int recordeventsaddedcounter,mainclonepatternid,mediatype;
	bool recordeventsadded;
};

class Undo_CreatePattern:public UndoFunction
{
public:
	Undo_CreatePattern(Seq_Song *s,UndoCreatePattern *ucp,int np)
	{
		id=Undo::UID_CREATENEWPATTERN;

		song=s;
		createpattern=ucp;
		numberofpattern=np;

		MIDIfile=0;
		patternfile=0;

#ifdef _DEBUG
		n[0]='U';
		n[1]='C';
		n[2]='P';
		n[3]='A';
#endif

		undorecording=false;
	}

#ifdef _DEBUG
	char n[4];
#endif

	bool RefreshUndo();
	void Do();
	void UndoGUI();
	void DoUndo();
	void DoRedo();

	void AddedToUndo()
	{
		if(undorecording==true)
			CreateUndoString(Cxs[CXS_DELETELASTRECORDING]);
		else
			CreateUndoString(Cxs[CXS_F_CREATEPATTERN]);
	}

	void FreeData();

	UndoCreatePattern *createpattern;
	char *MIDIfile,*patternfile;
	int numberofpattern;
	// MIDI File Import
	bool undorecording;
};

class Undo_ReplaceAudioPatternFile:public UndoFunction
{
public:
	Undo_ReplaceAudioPatternFile(AudioPattern *pattern,AudioHDFile *newhd,AudioRegion *r);
	bool RefreshUndo();
	void Do();
	void DoUndo();
	void RefreshGUI(bool undorefresh);

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_CHANGEPAUDIOFILE]);
	}

	Seq_Pattern_VolumeCurve oldvolume;
	AudioPattern *audiopattern;
	AudioHDFile *newhdfile,*oldhdfile;
	AudioRegion *newregion,*oldregion;
};

class Undo_QuantizeSelP
{
public:
	Seq_Pattern *pattern;
	OSTART oldstart,newsimplestart;
};

class Undo_QuantizeSelectedPattern:public UndoFunction
{
public:
	Undo_QuantizeSelectedPattern(Undo_QuantizeSelP *l,int c,OSTART tx)
	{
		id=Undo::UID_QUANTIZESTARTPATTERN;
		list=l;
		listcounter=c;
		ticks=tx;
	}

	bool RefreshUndo();
	void Do();
	void DoUndo();

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_QUANTIZEPATTERNSP]);
	}

	void FreeData()
	{
		if(list)delete list;
		list=0;
	}

	Undo_QuantizeSelP *list;
	OSTART ticks;
	int listcounter;
};

class Undo_QuantizePattern:public UndoFunction
{
public:
	Undo_QuantizePattern(Seq_Pattern *p,QuantizeEffect *old,QuantizeEffect *newfx,bool peffect);
	bool RefreshUndo();
	void Do();
	void DoUndo();

	void AddedToUndo()
	{
		CreateUndoString(Cxs[CXS_F_QUANTIZEPATTERN]);
	}

	QuantizeEffect oldeffects,neweffects; // oldbuffer
	Seq_Pattern *pattern;
	OSTART changedpositions,oldpatternposition;
	bool patterneffect;
};

class Undo_LoopPattern:public UndoFunction
{
public:
	Undo_LoopPattern(int trackindex,int patternindex,bool endless,bool lwithloops,int l)
	{
		id=Undo::UID_LOOPPATTERN;

		index_track=trackindex;
		index_pattern=patternindex;
		loopendless=endless;
		loopwithloops=lwithloops;
		loops=l;
		pattern=0;
	}

	bool RefreshUndo();
	void Do();
	void DoUndo();

	void AddedToUndo()
	{
		CreateUndoString("Loop Pattern");
	}

	Seq_Pattern *pattern;
	int index_track,index_pattern,loops,oldloops;
	bool oldloopendless,oldloopwithloops,loopendless,loopwithloops;
};
#endif