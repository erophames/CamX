#include "proc_chord.h"
#include "objectevent.h"

void Proc_Chord::InsertEvent(Proc_AddEvent *e,MIDIProcessor *proc)
{
	Seq_Event *pe=proc->FirstProcessorEvent();
	
	while(pe)
	{
		if(!(pe->flag&EVENTFLAG_ADDEDBYTHISMODULE))
		{
			switch(pe->GetStatus())
			{
			case NOTEON: // Note down
				{
					Note *note=(Note *)pe;
				
					for(int i=0;i<adds;i++)
					{
						int addkey=keys[i]+note->key;
						
						if(addkey<128 && addkey>=0)
						{
							Note *newnote=(Note *)note->Clone(0);
							
							if(newnote)
							{
								newnote->key=addkey;
								
								proc->AddProcEvent(newnote,0,e->triggerevent,EVENTFLAG_ADDEDBYTHISMODULE);
							}
						}
					}
				}
				break;
				
			case NOTEOFF:
				{
					NoteOff_Raw *off=(NoteOff_Raw *)pe;
					
					for(int i=0;i<adds;i++)
					{
						int addkey=keys[i]+off->key;
						
						if(addkey<128 && addkey>=0)
						{
							NoteOff_Raw *newnoteoff=(NoteOff_Raw *)off->Clone(0);
							
							if(newnoteoff)
							{
								newnoteoff->key=addkey;
								proc->AddProcEvent(newnoteoff,0,e->triggerevent,EVENTFLAG_ADDEDBYTHISMODULE);
							}
						}
					}
				}
				break;
			}
		}
		
		pe=pe->NextEvent();
	}	
}
