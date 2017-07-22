#ifndef CAMX_MIDIPROCESSOR_PROCDELAY_H
#define CAMX_MIDIPROCESSOR_PROCDELAY_H 1

#include "MIDIprocessor.h"

#include "seq_realtime.h"
#include "icdobject.h"
#include "groove.h"
#include "objectevent.h"

class Proc_Delay;

class Proc_DelayOpenNote:public Object
{
public:
	Proc_DelayOpenNote(Seq_Track *t,MIDIPattern *p,Note *on)
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

		counter=0;
	}

	Proc_DelayOpenNote *NextOpenNote(){return (Proc_DelayOpenNote *)next;}
	Proc_DelayOpenNote *PrevOpenNote(){return (Proc_DelayOpenNote *)prev;}
	Seq_Track *track;
	Note note;
	MIDIPattern *pattern;

	char inputkey;
	char inputvelo;

	bool played; // rnd
	bool octavenote;

	int counter;
};

class Proc_DelayCloseNote:public Object
{
public:
	Seq_Track *track;
	Note note;
	NoteOff_Raw off;
};

#define MAXNUMBER_DELAYNOTES 16

class Proc_Delay:public MIDIPlugin
{
	enum DelayInfo{
		DELAY_AINFO_PLAYNOTE=1,
		DELAY_AINFO_STOPNOTE=2
	};

	enum DelayVelo{
		DELAY_KEEPVELO=0,
		DELAY_FIXVELO,
		DELAY_ADDVELO,
		DELAY_SUBVELO
	};

	enum UseType{
		DELAY_USE_MANUAL,
		DELAY_USE_DOWN,
		DELAY_USE_UP
	};

public:
	Proc_Delay()
	{
		InitModule();
		moduletype=PM_DELAY;
		strcpy(staticname,"Delay");

		Reset();
	}

	void Reset()
	{
		groove=0;
		usequantize=false;
		quantize=10; // 1/8
		
		steps=4;

		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			steplength[i]=
			stepsize[i]=10; // 1/8
			
			velocityflag[i]=DELAY_KEEPVELO; // keep
			
			setvelocity[i]=127;
		}

		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			addvelocity[i]=127/MAXNUMBER_DELAYNOTES*(i+1);
			subvelocity[i]=127/MAXNUMBER_DELAYNOTES*(i+1);
		}

		act_stepsize=0;
		act_steplength=0;
		act_stepvelo=0;

		type=DELAY_USE_DOWN;
		downto=20;
		upto=127;
	}

	void EditDataMessage(EditData *data);
	guiGadget *Gadget(guiGadget *);
	void ShowGUI();

	int GetEditorSizeX(){return 250;}

	void InitModuleGUI(Edit_ProcMod *ed,int x,int y,int x2,int y2);
	MIDIPlugin *CreateClone();

	void Load(camxFile *);
	void Save(camxFile *);

	void Alarm(Proc_Alarm *procalarm);
	void InsertEvent(Proc_AddEvent *addevent,MIDIProcessor *ol);

	void FreeProcessorModuleMemory() //v
	{
		Proc_DelayOpenNote *n=FirstOpenNote();
		while(n)n=(Proc_DelayOpenNote *)notes.RemoveO(n);
	}

private:
	char AddVelocity(int step,char velo);
	OSTART GetNextTick(int step);
	void AddNote (Proc_DelayOpenNote *n);
	Proc_DelayOpenNote *RemoveNote(Proc_DelayOpenNote *n);

	void ShowType();
	void ShowQuantize();
	void ShowSize();
	void ShowDelays();
	void ShowLength();
	void ShowVelo();
	void ShowDown();
	void ShowUp();

	void ChangeVelo(guiGadget *g,int step,int type);

	Proc_DelayOpenNote *FirstOpenNote (){return (Proc_DelayOpenNote *)notes.GetRoot();}
	Proc_DelayOpenNote *LastOpenNote (){return (Proc_DelayOpenNote *)notes.Getc_end();}
	Proc_DelayOpenNote *GetNoteAtIndex(int ix){return (Proc_DelayOpenNote *)notes.GetO(ix);}
	
	void SetQuant(int q);

	OList notes; // Proc_ApprOpenNote

	int type;
	bool usequantize;
	int quantize,
	 steps,
	 act_stepsize,
	 act_steplength,
	 act_stepvelo;

	char stepsize[MAXNUMBER_DELAYNOTES];
	char steplength[MAXNUMBER_DELAYNOTES];
	int velocityflag[MAXNUMBER_DELAYNOTES]; // keep
	
	char addvelocity[MAXNUMBER_DELAYNOTES];
	char subvelocity[MAXNUMBER_DELAYNOTES];
	char setvelocity[MAXNUMBER_DELAYNOTES];

	char downto;
	char upto;

	Groove *groove;

	guiGadget *g_type;
	guiGadget *g_down;
	guiGadget *g_up;

	guiGadget *g_bypass;
	guiGadget *g_delays;
	guiGadget *g_quant;

	guiGadget *g_length;
	guiGadget *g_steps;
	guiGadget *g_quantonoff;
	guiGadget *g_velolist;
	guiGadget *g_editvelo;
	guiGadget *g_editallvelo;
};

#endif