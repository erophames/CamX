#ifndef CAMX_MIDIPROCESSOR_PROCCHORD_H
#define CAMX_MIDIPROCESSOR_PROCCHORD_H 1

#include "MIDIprocessor.h"
#include "icdobject.h"

class Proc_Chord:public MIDIPlugin
{
public:
	Proc_Chord()
	{
		InitModule();
		strcpy(staticname,"Chord");
		
		Reset();
	}
	
	void ShowGUI(){}

	void Reset()
	{
				// default
		adds=2; // Max 32
		keys[0]=4;
		keys[1]=7;
	}

	void Load(camxFile *file){}
	void Save(camxFile *file){}

	void InsertEvent(Proc_AddEvent *addevent,MIDIProcessor *ol);
	void NoteOff(Proc_AddEvent *e,MIDIProcessor *proc,UBYTE status,UBYTE key,UBYTE velooff);
	
	MIDIPlugin *CreateClone()
	{
		Proc_Chord *pc=new Proc_Chord;
		
		if(pc)
		{
		}
		
		return pc;
	}
	
	void FreeProcessorModuleMemory() //v
	{
	}
	
	char keys[32];
	int adds;
};

#endif