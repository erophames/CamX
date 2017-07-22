#ifndef CAMX_MIDIPROCESSOR_PROCAPPR_H
#define CAMX_MIDIPROCESSOR_PROCAPPR_H 1

#include "MIDIprocessor.h"

#include "seq_realtime.h"
#include "icdobject.h"
#include "groove.h"
#include "objectevent.h"

class Proc_Appr;

class Proc_ApprOpenNote:public Object
{
public:
	Proc_ApprOpenNote(Seq_Track *t,MIDIPattern *p,Note *on)
	{
		track=t;
		pattern=p;

		inputkey=on->key;
		inputvelo=on->velocity;

		note.status=on->status;
		note.key=on->key;
		note.velocity=on->velocity;
		note.pattern=on->pattern;
		played=false;
		octavenote=false;
	}

	Proc_ApprOpenNote *NextOpenNote(){return (Proc_ApprOpenNote *)next;}
	Proc_ApprOpenNote *PrevOpenNote(){return (Proc_ApprOpenNote *)prev;}
	Seq_Track *track;
	Note note;
	MIDIPattern *pattern;

	char inputkey;
	char inputvelo;
	bool played; // rnd
	bool octavenote;
};

class Proc_ApprCloseNote:public Object
{
public:
	Seq_Track *track;
	Note note;
	NoteOff_Raw off;
};

#define MAXNUMBER_APPRNOTES 16

class Proc_Appr:public MIDIPlugin
{
	enum AprrType{
		TYPE_UP,
		TYPE_DOWN,
		TYPE_UPDOWNDOWNUP,
		TYPE_RND
	};

	enum AprrInfo{
		APPR_AINFO_PLAYNOTE=1,
		APPR_AINFO_STOPNOTE=2
	};

	enum AprVelo{
		APPR_KEEPVELO=0,
		APPR_FIXVELO,
		APPR_ADDVELO,
		APPR_LOWESTVELO,
		APPRO_HIGHESTVELO
	};

public:
	Proc_Appr()
	{
		InitModule();
		moduletype=PM_ARPEGGIATOR;
		strcpy(staticname,"Arpeggiator");
		Reset();
	}

	void Reset();

	void EditDataMessage(EditData *);
	guiGadget *Gadget(guiGadget *);
	void ShowGUI();

	int GetEditorSizeX(){return 250;}
	int GetEditorSizeY(){return 240;}

	void InitModuleGUI(Edit_ProcMod *,int x,int y,int x2,int y2);
	MIDIPlugin *CreateClone();

	void Load(camxFile *);
	void Save(camxFile *);

	void Alarm(Proc_Alarm *);
	void InsertEvent(Proc_AddEvent *,MIDIProcessor *);

	void FreeProcessorModuleMemory() //v
	{
		Proc_ApprOpenNote *n=FirstOpenNote();
		while(n)
			n=(Proc_ApprOpenNote *)notes.RemoveO(n);
	}

private:
	char AddTranspose(char key);
	char AddVelocity(int step,char velo);
	OSTART GetNextTick();
	void AddNote (Proc_ApprOpenNote *);
	Proc_ApprOpenNote *RemoveNote(Proc_ApprOpenNote *n);

	void ShowQuantize();
	void ShowTranspose();
	void ShowOctaves();
	void ShowSize();
	void ShowLength();
	void ShowVelo();
	void ChangeVelo(guiGadget *,int step,int type);

	Proc_ApprOpenNote *FirstOpenNote (){return (Proc_ApprOpenNote *)notes.GetRoot();}
	Proc_ApprOpenNote *LastOpenNote (){return (Proc_ApprOpenNote *)notes.Getc_end();}
	Proc_ApprOpenNote *GetNoteAtIndex(int ix){return (Proc_ApprOpenNote *)notes.GetO(ix);}

	void SetQuant(int);

	OList notes; // Proc_ApprOpenNote
	int type;
	bool usequantize,sortbynotes;
	int quantize,steps,act_stepsize,act_steplength,act_stepvelo;
	char stepsize[MAXNUMBER_APPRNOTES],steplength[MAXNUMBER_APPRNOTES];
	int velocityflag[MAXNUMBER_APPRNOTES]; // keep
	char addvelocity[MAXNUMBER_APPRNOTES],setvelocity[MAXNUMBER_APPRNOTES],addoctave[4];
	Groove *groove;
	bool up,running;
	int transpose,ringcounter;

	guiGadget *g_bypass,
	 *g_type,
	 *g_quant,
	 *g_sort,
	 *g_transpose,
	 *g_octave,
	 *g_length,
	 *g_steps,
	 *g_quantonoff,
	 *g_velolist,
	 *g_editvelo,
	 *g_editallvelo;
};

#endif