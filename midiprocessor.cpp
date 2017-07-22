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

MIDIProcessor::MIDIProcessor(Seq_Song *s,Seq_Track *t)
{
	song=s;
	track=t;
	trackfx=0;
	patternfx=0;
	indevice=0;
	inplugin=0;
	inevent=0;
	flag=0;

	createraw=false;
}

MIDIProcessor::~MIDIProcessor()
{
	Seq_Event *p=FirstProcessorEvent();

	while(p)
		p=DeleteEvent(p);
}

Seq_Event *MIDIProcessor::DeleteEvent(Seq_Event *e)
{
	Seq_Event *n=(Seq_Event *)events.CutObject(e); // Remove from Event list
	e->Delete(true); // And Delete Memory
	return n;
}

Seq_Event *MIDIProcessor::DeletedEventByModule(MIDIPlugin *module,Seq_Event *e,Seq_Track *t,MIDIPattern *p,Seq_Event *triggerevent)
{
	e->SendPreProcDelete(t,p,triggerevent);
	return DeleteEvent(e);
}

void MIDIProcessor::AddProcEvent(Seq_Event *e,MIDIPattern *triggerpattern,Seq_Event *triggerevent,int flag)
{
	if(e && triggerevent)
	{
		e->flag=flag;
		e->pattern=triggerpattern;
		events.AddOSort(e,e->ostart);
	}
}

void MIDIProcessor::AddModules(MIDIPlugin *pmodule,Seq_Event *seqevent)
{
	if(pmodule)
	{
		Proc_AddEvent addevent(seqevent,track);

		do
		{
			pmodule->LockO();
			pmodule->InsertEvent(&addevent,this);
			pmodule->UnlockO();

			// Clear Event Flag
			Seq_Event *e=FirstProcessorEvent();

			while(e){
				e->flag=0;
				e=e->NextEvent();
			}		

			pmodule=pmodule->NextModule();	
		}
		while(pmodule);
	}
}

void MIDIProcessor::AddEventsToRAW()
{
	Seq_Event *procevent=FirstProcessorEvent();

	while(procevent)
	{
		mainprocessor->AddRAWEvent(procevent);
		procevent=DeleteEvent(procevent);	
	}
}

void MIDIProcessor::SendProcessorEvents(NewEventData *ned,int proccheckflag,int sampleoffset,int iflag)
{
	// Thru Processor
	Seq_Event *procevent=FirstProcessorEvent();

	if(!procevent)return;

	bool audioevent=track->CheckIfTrackIsAudioInstrument();

	while(procevent)
	{
		if(!(proccheckflag&SPE_INPUT))
		{
			if(mainMIDI->MIDI_outputimpulse<=50)
				mainMIDI->MIDI_outputimpulse=100;
		}

		if((proccheckflag&SPE_THRUCHECK) && procevent->pattern==0) // Thru Events pattern =0
		{
			switch(procevent->GetStatus())
			{
			case NOTEON: // Buffer Thru NoteOn's
				{
					Note *note=(Note *)procevent;

					if(audioevent==false)
					{
						if(!(ned->flag&NED_NOMIDI))
						{
							// Add Thru Off Pointer to MIDI Output Devices
							Seq_Group_MIDIOutPointer *sgp=track->GetMIDIOut()->FirstDevice();

							while(sgp)
							{
								mainMIDIthruthread->AddThruOff(
									sgp->GetDevice(),
									indevice,
									track,
									0,
									inevent->status, // Incoming Note Status
									((Note *)inevent)->key,
									note->status,
									note->key
									);

								//TRACE ("Add THRU Off Note\n");

								sgp=sgp->NextGroup();
							}
						}
					}
					else
					{
						if(!(ned->flag&NED_NOAUDIO))
						{
							// Audio Instruments Thru Off

							// Check Audio Instrument Output
							// Track Instruments

							Seq_Track *t=track;

							do
							{

								// Input or Output
								InsertAudioEffect *iae=proccheckflag&SPE_INPUT?t->io.audioinputeffects.FirstInsertAudioEffect():t->io.audioeffects.FirstInsertAudioEffect();

								while(iae)
								{
									if(iae->audioeffect!=inplugin &&
										iae->audioeffect->plugin_on==true &&
										iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT &&
										iae->MIDIfilter.CheckEvent(note)==true
										)
										mainMIDIthruthread->AddThruOffToPlugin(
										iae,
										indevice,
										t,
										0,
										inevent->status, // Incoming Note Status
										((Note *)inevent)->key,
										note->status,
										note->key
										);

									iae=iae->NextEffect();
								}

								if(!(proccheckflag&SPE_INPUT))
								{
									Seq_AudioIOPointer *acp=t->GetAudioOut()->FirstChannel();

									while(acp)
									{
										InsertAudioEffect *iae=acp->channel->io.audioeffects.FirstInsertAudioEffect();

										while(iae)
										{
											if(iae->audioeffect!=inplugin && 
												iae->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT &&
												iae->audioeffect->plugin_on==true &&
												iae->MIDIfilter.CheckEvent(note)==true
												)
											{
												mainMIDIthruthread->AddThruOffToPlugin
													(
													iae,
													indevice,
													t,
													0,
													inevent->status, // Incoming Note Status
													((Note *)inevent)->key,
													note->status,
													note->key
													);
											}

											iae=iae->NextEffect();
										}

										acp=acp->NextChannel();
									}
								}

								t=(Seq_Track *)t->parent;

							}while(t);
						}
					}
				}
				break;
			}

		}// if pattern ==0 

		if(audioevent==true)
		{
			//track->SendOutEvent_User_Audio(0,procevent,-1,false,offset); // >>> Audio dont create offs
			//bool realtimeeventcreated=false;

			int sflag=(proccheckflag&SPE_THRUCHECK)?Seq_Track::SOE_THRUEVENT:0;

			if(proccheckflag&SPE_INPUT)
				sflag|=Seq_Track::SOE_INPUT;

			if(!(ned->flag&NED_NOAUDIO))
			{
				if(procevent->pattern && (!(procevent->flag&EVENTFLAG_PROCALARM))) // Thru, Pattern=0
				{
					MIDIProcessor processor(song,track);

					// MIDI Output Processor
					processor.EventInput((MIDIPattern *)procevent->pattern,procevent,procevent->GetEventStart());

					Seq_Event *oevent=processor.FirstProcessorEvent();

					while(oevent){

						track->SendOutEvent_AudioEvent(oevent,sflag,sampleoffset,inplugin);

						//if(oevent->SendTriggerImpulse()==true)
						//	realtimeeventcreated=true;

						oevent=processor.DeleteEvent(oevent);	
					}

				}
				else // Thru Event or User Event
				{
					track->SendOutEvent_AudioEvent(procevent,sflag,sampleoffset,inplugin);

					//if(procevent->SendTriggerImpulse()==true)
					//	realtimeeventcreated=true;
				}
			}
		}
		else
		{
			if(!(ned->flag&NED_NOMIDI))
			{
				track->SendOutEvent_User(0,procevent,false); // >>> MIDI dont create offs
			}
		}

		procevent=DeleteEvent(procevent);	

	}// while Processor Events
}

void MIDIProcessor::AddStaticFX(MIDIPattern *pattern)
{
	int channel;

	// Static FX
	Seq_Event *fx=FirstProcessorEvent();
	while(fx)
	{
		// Change Channel ?
		if(patternfx && (channel=patternfx->GetChannel())) // Pattern Channel ?
		{
			fx->SetChannel((UBYTE)channel-1);
		}
		else
		{
			if(trackfx && (channel=trackfx->GetChannel())) // Track Channel
				fx->SetChannel((UBYTE)channel-1);
		}

		fx->DoEventProcessor(this); // Event intern FX

		if(track && (!pattern) && (!(fx->flag&EVENTFLAG_PROCALARM))) // Thru Event, User Event-create NoteOn Stack Event
		{
			switch(fx->GetStatus())
			{
			case NOTEON:
				{
					Note *note=(Note *)fx;

					if(OpenOutputNote *onote=new OpenOutputNote)
					{
						onote->status=note->status;
						onote->key=note->key;

						track->LockOpenOutputNotes();
						track->AddOpenOutputNote(onote);
						track->UnlockOpenOutputNotes();
					}
				}
				break;
			}
		}

		fx=fx->NextEvent();
	}
}

void MIDIProcessor::EventInput_CalledByProcAlarm(Proc_Alarm *procalarm,MIDIPlugin *module,Seq_Event *seqevent)
{
	inevent=seqevent;
	trackfx=track->GetFX();

	if(seqevent->CheckFilter(&trackfx->filter)==true) // Input Filter
	{
		Seq_Event *clone=(Seq_Event *)seqevent->Clone(0);

		if(!clone)return;

		if(procalarm->createraw==true)
			clone->ostart=procalarm->songposition;

		AddProcEvent(clone,0,seqevent,EVENTFLAG_PROCALARM); // Start Event create copy

		MIDIPlugin *pmodule=module->NextModule(); // Call only NEXT Modules...->

		if(pmodule) // Next Modules ....
			AddModules(pmodule,seqevent);

		AddStaticFX(0);

		if(procalarm->createraw==false)
			SendProcessorEvents(0,0,procalarm->offset_sample); // No Thru Check
		else
			AddEventsToRAW();

	}// Filter ok ?
}

void MIDIProcessor::EventInput(MIDIPattern *pattern,Seq_Event *seqevent,OSTART pos)
{
	inevent=seqevent;

	Processor *processor;

	if(track)
	{
		trackfx=track->GetFX();
		processor=track->GetProcessor();

		if(seqevent->CheckFilter(&track->GetFX()->filter)==false)
			return;

		// Check Parent Filter
		Seq_Track *p=(Seq_Track *)track->parent;
		while(p)
		{
			if(seqevent->CheckFilter(&p->GetFX()->filter)==false)
				return;

			p=(Seq_Track *)p->parent;
		}
	}
	else
	{
		trackfx=0;
		processor=0;
	}

	patternfx=pattern?pattern->GetMIDIFX():0;

	if(((!patternfx) || seqevent->CheckFilter(&patternfx->filter)==true)
		) // Input Filter
	{
		AddProcEvent((Seq_Event *)seqevent->Clone(song),pattern,seqevent); // Start Event

		if(createraw==false)
		{
			MIDIPlugin *pmodule;

			mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);

			if(processor && processor->bypass==false && (pmodule=processor->FirstProcessorModule())) // Processor+Modules ?
				AddModules(pmodule,seqevent);

			mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);

			AddStaticFX(pattern);
		}

	}// Filter ok ?
}

void MIDIProcessor::EventInput_RAW(MIDIPattern *pattern,Seq_Event *seqevent)
{
	inevent=seqevent;

	Processor *processor;

	if(track)
	{
		trackfx=track->GetFX();
		processor=track->GetProcessor();
	}
	else
	{
		trackfx=0;
		processor=0;
	}

	patternfx=pattern?pattern->GetMIDIFX():0;

	AddProcEvent((Seq_Event *)seqevent->Clone(0),pattern,seqevent); // Start Event

	if(pattern)
	{
		MIDIPlugin *pmodule;

		mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);
		if(processor && processor->bypass==false && (pmodule=processor->FirstProcessorModule())) // Processor+Modules ?
			AddModules(pmodule,seqevent);
		mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);

		AddStaticFX(pattern);
	}
}


// Processor Process
PTHREAD_START_ROUTINE MIDIProcessorProc::mainMIDIalarmthreadessorThreadFunc(LPVOID pParam) 
{
	MIDIProcessorProc *proc=(MIDIProcessorProc *)pParam;
	LONGLONG nextpulse=-1;	

	while(proc->IsExit()==false) // Signal Loop
	{
		proc->WaitSignal((int)nextpulse);

		if(proc->IsExit()==true) // Exit Signal ?
			break;		

		/*
		mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);

		nextpulse=-1; // not set

		if(Seq_Song *asong=mainvar->GetActiveSong()){
		do // Loop ?
		{
		Proc_Alarm *p=proc->FirstAlarm();

		while(p)
		{
		if(p->alarmsystemtime_ms<=maintimer->GetTime())
		{
		//TRACE ("Call Module (Alarm) %d\n",p->alarmsystemtime);
		MIDIPlugin *module=p->module;

		module->LockO();

		p->calledbyalarm=true;
		module->Alarm(p);
		p->forcealarm=false; // Reset Force Flag
		p=proc->RemoveAlarm(p);

		module->UnlockO();
		}
		else
		{
		if(nextpulse<0 || p->alarmsystemtime_ms-maintimer->GetTime()<nextpulse)
		nextpulse=p->alarmsystemtime_ms-maintimer->GetTime();

		if(nextpulse<0)
		nextpulse=0;

		p=p->NextAlarm();
		}
		}

		}while(nextpulse==0); // Loop pulse==0

		}// if asong
		else
		nextpulse=-1; // sleep no active song

		mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);
		*/

	}// while exitthreads

	proc->DeleteAllAlarms(0,0);

	proc->ThreadGone();

	return 0;
}

void MIDIProcessorProc::InitAlarm(Proc_Alarm *palarm,Seq_Event *orgevent,Proc_Alarm *calledbyalarm)
{
	if(palarm->createraw==true)
	{
		if(orgevent && orgevent->pattern)
			palarm->songposition=orgevent->GetEventStart();

		//	palarm->songposition+=(int)floor(palarm->song->timetrack.AddTempoToTicksWithStart(palarm->songposition,palarm->alarmticks)+0.5);
		palarm->songposition+=(OSTART)floor(palarm->song->timetrack.SubTicksToTempoTicks(palarm->songposition,palarm->alarmticks)+0.5);
	}
	else
	{
		if(calledbyalarm // && calledbyalarm->offset_ms
			) // Audio Offsets...
		{
			palarm->systemtime_ms=calledbyalarm->alarmsystemtime_ms;
		}
		else
			palarm->systemtime_ms=maintimer->GetSystemTime(); // ms

		palarm->songposition=palarm->song->GetSongPosition();
		//double sysalarm=palarm->song->timetrack.AddTempoToTicksWithStart(palarm->songposition,palarm->alarmticks);

		double sysalarm=palarm->song->timetrack.SubTicksToTempoTicks(palarm->songposition,palarm->alarmticks);
		//	palarm->alarmtime=(long)floor(sysalarm+0.5); // ticks

		sysalarm*=INTERNRATEMSDIV;	// Ticks -> ms

		palarm->alarmsystemtime_ms=palarm->systemtime_ms + (LONGLONG)floor(sysalarm+0.5); //ms
	}
}

Proc_Alarm *MIDIProcessorProc::DeleteRAWAlarm(Proc_Alarm *alarm)
{
	if(alarm->deleteobject)
		delete alarm->deleteobject; // Delete Alarm Object ?

	return (Proc_Alarm *)rawalarms.RemoveO(alarm);
}

bool MIDIProcessorProc::AddAlarm(Proc_Alarm *palarm,Seq_Event *seqevent,Proc_Alarm *calledbyalarm)
{
	if(palarm){

		InitAlarm(palarm,seqevent,calledbyalarm);

		if(palarm->createraw==true)
			rawalarms.AddOSort(palarm,palarm->songposition);
		else
		{
			alarms.AddEndO(palarm);

			if(palarm->calledbyalarm==false) // User Event ?
				SetSignal(); // wake alarm thread
		}

		return true;
	}

	return false;
}

void MIDIProcessorProc::DeleteAllAlarms(MIDIPlugin *module,Object *obj)
{
	Proc_Alarm *pa=FirstAlarm();

	while(pa){

		if((pa->module==module || module==0) && (pa->object==obj || obj==0))
		{
			if(!module)
				pa->forcealarm=false;

			pa=RemoveAlarm(pa);
		}
		else
			pa=pa->NextAlarm();
	}
}

Proc_Alarm *MIDIProcessorProc::RemoveAlarm(Proc_Alarm *palarm)
{
	if(palarm->forcealarm==true) // Note Offs etc..
	{
		//palarm->module->LockO();
		palarm->calledbyalarm=true;
		palarm->module->Alarm(palarm);
		//palarm->module->UnlockO();
	}

	if(palarm->deleteobject)
		delete palarm->deleteobject;

	return (Proc_Alarm *)alarms.RemoveO(palarm);
}

int MIDIProcessorProc::StartThread()
{
	int error=0;
#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)mainMIDIalarmthreadessorThreadFunc,(LPVOID)this, 0,0);

	if(ThreadHandle)
		SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);
	else
		error=1;
#endif

	return error;
}

void MIDIProcessorProc::StopThread()
{
	SendQuit();
}