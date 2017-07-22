#include "defines.h"
#include "songmain.h"
#include "MIDIoutproc.h"
#include "MIDIhardware.h"
#include "MIDIthruproc.h"
#include "semapores.h"
#include "settings.h"
#include "object_track.h"
#include "MIDIPattern.h"
#include "MIDIprocessor.h"
#include "audiochannel.h"
#include "MIDItimer.h"
#include "object_song.h"

#include <math.h>

RealtimeEvent::RealtimeEvent()
{
	realtimeid=REALTIMEID_UNKNOWN; // Unknown
	noteoff=false; // true=non song event (user event)

	Init();
}

void RealtimeEvent::SendToProcessor()
{
	if(fromtrack && fromtrack->GetProcessor()) // -> Processor
	{
		mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);

		if(MIDIPlugin *pm=fromtrack->GetProcessor()->FirstProcessorModule())
		{
			if(Seq_Event *rawevent=CreateRAWEvent())
			{
				Proc_AddEvent addevent(rawevent,fromtrack);

				// Processor
				MIDIProcessor processor(fromtrack->song,fromtrack);

				processor.AddProcEvent(rawevent,0,rawevent);

				while(pm){

					pm->LockO();
					pm->InsertEvent(&addevent,&processor);
					pm->UnlockO();

					pm=pm->NextModule();
				}
			}
		}

		mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);
	}
}

void RealtimeEvent::SendQuick()
{
	if(audioobject)
	{
		//	TRACE ("SQ Audio\n");
		audioobject->Execute_TriggerEvents();
		SendToAudio(audioobject,0);
	}
	else
	{
		//	TRACE ("SQ MIDI\n");
		SendToMIDI();
	}
}

void Seq_Realtime::BufferBeforeTempoChanges()
{
	Lock();
	RealtimeEvent *re=FirstREvent();
	while(re){
		re->rte_buffercallposition=song->timetrack.ConvertSamplesToTicks(re->rte_callsampleposition);
		re=re->NextEvent();
	}
	UnLock();
}

void Seq_Realtime::RefreshTempoBuffer()
{
	Lock();
	RealtimeEvent *re=FirstREvent();
	while(re)
	{
		re->rte_callsampleposition=song->timetrack.ConvertTicksToTempoSamples(re->rte_buffercallposition);
		re=re->NextEvent();
	}
	UnLock();
}

void Seq_Realtime::AddWaitEvent(RealtimeEvent *we)
{
	Lock();
	waitrealtimeevents.AddEndO(we);
	UnLock();
}

void Seq_Realtime::AddCDAlarmEvent(RealtimeEvent *rte,LONGLONG cdsamples)
{
	switch(type)
	{
	case REALTIMELIST_MIDI:
		{
			// MIDI + Alarm Signal
			rte->rte_inittime=maintimer->GetSystemTime();
			rte->rte_callsystime=rte->rte_inittime+maintimer->ConvertInternTicksToSysTime(cdsamples); // Count down Ticks

			//Sort
			Lock();

			RealtimeEvent *sort=FirstCDEvent();
			while(sort){
				if(sort->rte_callsystime>rte->rte_callsystime){
					realtimecdevents.AddNextO(rte,sort);
					break;
				}

				sort=sort->NextEvent();
			}

			if(!sort)
				realtimecdevents.AddEndO(rte);
			UnLock();
			MIDIrtealarmproc->SetSignal();

		}
		break;

	case REALTIMELIST_AUDIO:
		{
			// Audio
			rte->rte_callsampleposition=/*song->playback_sampleposition+*/cdsamples;

			//Sort
			Lock();

			/*
			RealtimeEvent *sort=FirstCDEvent();
			while(sort){
			if(sort->rte_callsampleposition>rte->rte_callsampleposition){
			realtimecdevents.AddNextO(rte,sort);
			UnLock();

			MIDIrtealarmproc->SetSignal();
			return;
			}

			sort=sort->NextEvent();
			}
			*/

			realtimecdevents.AddEndO(rte);
			UnLock();
		}
		break;
	}
}

void Seq_Realtime::AddRealtimeAlarmEvent(RealtimeEvent *rte,LONGLONG callsampleposition)
{
	rte->rte_callsampleposition=callsampleposition;
	
	Lock(); 

	/*
	if(rte->outputdevice && (rte->status&0xF0)==NOTEOFF) // Avoid MIDI Note Off Crossfades
	{
		RealtimeEvent *check=FirstREvent();
		while(check){

			if(check->GetRealEventSampleStart()>=callsampleposition &&
				check->outputdevice==rte->outputdevice &&
				check->status==rte->status &&  
				check->fromtrack==rte->fromtrack &&
				check->rte_frompattern==rte->rte_frompattern){
				check->SendToMIDI();

				TRACE (" +++Check %d \n",callsampleposition-check->GetRealEventSampleStart());

				check=DeleteREvent(check);
			}
			else
				check=check->NextEvent();
		}
	}
*/

	//Sort
	RealtimeEvent *sort=FirstREvent();
	while(sort){
		if(sort->GetRealEventSampleStart()>callsampleposition){
			realtimeevents.AddNextO(rte,sort);
			UnLock();
			return;
		}

		sort=sort->NextEvent();
	}

	realtimeevents.AddEndO(rte);
	UnLock();
}

void Seq_Realtime::RemoveRObject(Object *o)
{
	Lock();

	RealtimeEvent *re=FirstREvent();
	while(re)re=re->audioobject==o?DeleteREvent(re):re->NextEvent();

	re=FirstWEvent();
	while(re)re=re->audioobject==o?DeleteWEvent(re):re->NextEvent();

	UnLock();
}

void Seq_Realtime::DeleteAllREvents(Seq_Song *song,int flag)
{
	Lock();

	RealtimeEvent *re=FirstREvent();

	while(re){
		re->SendQuick();
		re=DeleteREvent(re);
	}

	re=FirstWEvent();

	while(re){
		re->SendQuick();
		re=DeleteWEvent(re);
	}

	re=FirstCDEvent();

	while(re){
		re->SendQuick();
		re=DeleteCDEvent(re);
	}

	UnLock();
}

Seq_Event *NoteOff_Realtime::CreateRAWEvent()
	{
		if(NoteOff_Raw *raw=new NoteOff_Raw(status,key,velocityoff)){
			raw->status=status;
			raw->key=key;
			raw->velocityoff=velocityoff;
			return raw;
		}

		return 0;
	}

void NoteOff_Realtime::SendToMIDI()
{
	SendToProcessor();

	if(outputdevice)
		outputdevice->sendNoteOff(status,key,velocityoff);
}

void Control_SustainOn::SendToMIDI()
{
	SendToProcessor();

	if(outputdevice)
		outputdevice->sendControlChange(status,64,0); // Sustain Off
}

void Control_Realtime::SendToMIDI()
{
	if(!(rteflag&RTE_MIDIPatternSTART_EVENT))
		SendToProcessor();

	Seq_Group_MIDIOutPointer *sgp=fromtrack->GetMIDIOut()->FirstDevice();

	while(sgp)
	{
		bool send=true;

		if(rteflag&RTE_MIDIPatternSTART_EVENT) // Create Status+Outputdevice
		{
			if(outputdevice=sgp->GetDevice())
			{
#ifdef OLDIE
				if(rte_frompattern && /*rte_frompattern->MIDIprogram.on==true &&*/ rte_frompattern->MIDIprogram.usebank==true)
				{
					int channel;
					controller=0; // Bank
					value=(UBYTE)rte_frompattern->MIDIprogram.MIDIBank;

					if(channel=rte_frompattern->GetMIDIFX()->GetChannel())
						status=(UBYTE)(CONTROLCHANGE|(channel-1));
					else
						if(channel=fromtrack->GetFX()->GetChannel())
							status=(UBYTE)(CONTROLCHANGE|(channel-1));
						else
							send=false;
				}
				else
#endif
					send=false;

			}
			else
				send=false;
		}

		if(send==true && sgp->GetDevice())
			sgp->GetDevice()->sendControlChange(status,controller,value);

		sgp=sgp->NextGroup();
	}
}

void Program_Realtime::SendToMIDI()
{
	if(!(rteflag&RTE_MIDIPatternSTART_EVENT))
		SendToProcessor();

	Seq_Group_MIDIOutPointer *sgp=fromtrack->GetMIDIOut()->FirstDevice();

	while(sgp)
	{
		bool send=true;

		if(rteflag&RTE_MIDIPatternSTART_EVENT) // Create Status+Outputdevice
		{
			if(outputdevice=sgp->GetDevice())
			{
				if(rte_frompattern /*&& rte_frompattern->MIDIprogram.on==true*/ )
				{
					int channel;
					program=rte_frompattern->MIDIprogram.MIDIProgram;

					if(channel=rte_frompattern->GetMIDIFX()->GetChannel())
						status=(UBYTE)(PROGRAMCHANGE|(channel-1));
					else
						if(channel=fromtrack->GetFX()->GetChannel())
							status=(UBYTE)(PROGRAMCHANGE|(channel-1));
						else
							send=false;
				}
				else
					send=false;
			}
			else
				send=false;
		}

		if(send==true && sgp->GetDevice())
		{
			sgp->GetDevice()->sendProgramChange(status,program);
		}

		sgp=sgp->NextGroup();
	}
}