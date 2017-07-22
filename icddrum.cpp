#include "defines.h"
#include "drummap.h"
#include "drumevent.h"
#include "MIDIPattern.h"
#include "audiochannel.h"
#include "object_track.h"
#include "object_song.h"
#include "semapores.h"
#include "songmain.h"
#include "gui.h"
#include "camxfile.h"
#include "midiprocessor.h"

void ICD_Drum::LoadICDData(camxFile *file)
{
	file->AddPointer((CPOINTER)&drumtrack);

	file->ReadChunk(&velocity);
	file->ReadChunk(&velocityoff);
}

void ICD_Drum::SaveICDData(camxFile *file)
{
#ifdef DEBUG
	if(!drumtrack)
		maingui->MessageBoxError(0,"Drumtrack 0 !");
#endif

	file->Save_Chunk((CPOINTER)drumtrack);

	file->Save_Chunk(velocity);
	file->Save_Chunk(velocityoff);
}

bool ICD_Drum::CheckMute()
{
	if(drumtrack->mute==true || (drumtrack->map->solomode==true && drumtrack->solo==false))
		return false;

	return true;
}

void ICD_Drum::MoveIndex(int index)
{
	int ti=drumtrack->map->GetIndexOfTrack(drumtrack);
	Drumtrack *nt;

	if(nt=drumtrack->map->GetTrackWithIndex(ti+index))
		drumtrack=nt;
}

void ICD_Drum::AddSortToPattern(MIDIPattern *p,OSTART start) // v
{	
	pattern=p;

	QuantizeEffect *fx=GetQuantizer();

	if(fx->quantize) // Quantize ON ?
		start=fx->Quantize(start);

	p->AddSortEvent(this,start);
}

bool ICD_Drum::QuantizeEvent(QuantizeEffect *fx) // v
{		
	if(fx->quantize) // Quantize ON ?
	{	
		OSTART qpos=fx->Quantize(staticostart);

		if(qpos!=ostart)
		{
			MoveEventNoStatic(qpos);

			return true;
		}	
	}
	else // Quantize OFF
	{	
		if(staticostart!=ostart)
		{
			MoveEventNoStatic(staticostart);
			return true;
		}
	}

	return false;	
}

void ICD_Drum::SendToAudio(MIDIPattern *frompattern,AudioEffects *fx,int iflag,int offset,AudioObject *dontsendtoaudioobject)
{	
	if(CheckMute()==true)
	{
		int velo=velocity+drumtrack->volume;

		if(velo>127)
			velo=127;
		else
			if(velo<1)
				velo=1;

		InsertAudioEffect *ai=fx->FirstActiveAudioEffect();

		while(ai)
		{
			if(ai->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT &&
				ai->audioeffect!=dontsendtoaudioobject &&
				ai->MIDIfilter.CheckEvent(this)==true &&
				ai->audioeffect->Do_TriggerEvent(offset,(UBYTE)(NOTEON|drumtrack->GetMIDIChannel()),drumtrack->key,(char)velo,0)==true)
			{


#ifdef MEMPOOLS
				NoteOff_Realtime *noteoff=mainpools->mempGetNoteOff_Realtime();
#else
				NoteOff_Realtime *noteoff=new NoteOff_Realtime;
#endif

				if(noteoff)
				{
					// Init NoteOff
					//noteoff->audiochannel=c;
					noteoff->audioobject=ai->audioeffect;
					noteoff->outputdevice=0;

					noteoff->fromevent=0;
					noteoff->rte_frompattern=frompattern;
					noteoff->fromtrack=0;
					noteoff->status=NOTEOFF|drumtrack->GetMIDIChannel();
					noteoff->key=drumtrack->key;
					noteoff->velocityoff=drumtrack->velocityoff;
					noteoff->iflag=iflag;

					LONGLONG notelength_samples=fx->GetSong()->timetrack.ConvertTicksToTempoSamplesStart(ostart,drumtrack->ticklength);

					fx->GetSong()->realtimeevents[REALTIMELIST_AUDIO].AddCDAlarmEvent(noteoff,notelength_samples);
				}
			}

			ai=ai->NextActiveEffect();
		}
	}
}

void ICD_Drum::SendToAudioPlayback(AudioObject *o,LONGLONG samplestart,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *fevent,int iflag,int sampleoffset,bool createreal)
{	
	int velo=velocity+drumtrack->volume;

	if(velo>127)
		velo=127;
	else
		if(velo<1)
			velo=1;

	if(CheckMute()==true && o->Do_TriggerEvent(sampleoffset,NOTEON|drumtrack->GetMIDIChannel(),drumtrack->key,(char)velo,drumtrack->velocityoff)==true)
	{
		if(createreal==true)
		{
#ifdef MEMPOOLS
			NoteOff_Realtime *noteoff=mainpools->mempGetNoteOff_Realtime();
#else
			NoteOff_Realtime *noteoff=new NoteOff_Realtime; 
#endif

			if(noteoff)
			{
				// Init NoteOff
				noteoff->audioobject=o;
				noteoff->outputdevice=0;

				noteoff->fromevent=fevent;
				noteoff->rte_frompattern=fpattern;
				noteoff->fromtrack=track;
				noteoff->status=NOTEOFF|drumtrack->GetMIDIChannel();
				noteoff->key=drumtrack->key;
				noteoff->velocityoff=drumtrack->velocityoff;
				noteoff->iflag=iflag;

				track->song->realtimeevents[REALTIMELIST_AUDIO].AddRealtimeAlarmEvent(noteoff,GetSampleEnd(track->song,fpattern));
			}
		}
	}
}

void ICD_Drum::EraseCloneData() // Clipboard Buffer
{
	drumtrack=0;
}

void ICD_Drum::DoEventProcessor(MIDIProcessor *proc)
{
	// Velocity
	int h=GetVelocity();

	if(proc->trackfx)
		h+=proc->trackfx->GetVelocity();

	if(proc->patternfx)
	{
		h+=proc->patternfx->GetVelocity();

		if(pattern && pattern->itsaloop==true)
		{
			MIDIPattern *mp=(MIDIPattern *)pattern;

			h+=mp->t_MIDIeffects.GetVelocity();
		}
	}

	// Add Master Velocity
	h+=proc->song->audiosystem.masterchannel.MIDIfx.velocity.GetVelocity();

	if(h<1)
		velocity=1; // 0== 0ff
	else
		if(h>127)
			velocity=127;
		else
			velocity=(char)h;		
}

bool ICD_Drum::Compare(Seq_Event *e)
{
	if(e->status==status)
	{
		ICD_Object_Seq_MIDIChainEvent *sme=(ICD_Object_Seq_MIDIChainEvent *)e;

		if(sme->type==type)
		{
			ICD_Drum *ie=(ICD_Drum *)e;

			if(ie->velocity==velocity &&
				ie->velocityoff==velocityoff &&
				ie->drumtrack==drumtrack)
				return true;
		}
	}

	return false;
}

void ICD_Drum::SetMIDIImpulse(Seq_Track *t)
{
	double h=100;
	h/=127;
	h*=GetVelocity();

	if((int)h>t->MIDIoutputimpulse)
		t->MIDIoutputimpulse=(int)h;
}

bool ICD_Drum::ClonePossible(Seq_Song *song)
{
	if(!song)
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"ICD song=0");
#endif

		return false;
	}

	if(drumtrack)
		return true;

	// 1. Index
	Drummap *map=&song->drummap;

	if(Drumtrack *dt=map->GetTrackIndex(dt_index))
	{
		if(dt->dtr_channel==dtr_channel &&
			dt->key==dtr_key)
		{
			return true;
		}
	}

	// 2. Channel && key
	Drumtrack *dt=map->FirstTrack();
	while(dt)
	{
		if(dt->dtr_channel==dtr_channel &&
			dt->key==dtr_key)
		{
			return true;
		}

		dt=dt->NextTrack();
	}

#ifdef DEBUG
	maingui->MessageBoxError(0,"ICD Drum ClonePossible=false");
#endif

	return false;
}

Object *ICD_Drum::Clone(Seq_Song *song)
{
#ifdef DEBUG
	if(!song)
		maingui->MessageBoxError(0,"ICD_Drum::Clone(Seq_Song *song=0");
#endif

	if(ICD_Drum *p=new ICD_Drum)
	{
		CloneData(song,p);
		return p;
	}

	return 0;
}

void ICD_Drum::CloneData(Seq_Song *song,Seq_Event *e)
{
	ICD_Drum *sys=(ICD_Drum *)e;

	e->ostart=ostart;
	e->staticostart=staticostart;

	sys->velocity=velocity;
	sys->velocityoff=velocityoff;

	// for Copy/Paste
	if(sys->drumtrack=drumtrack)
	{
		sys->dt_volume=drumtrack->volume;
		sys->dt_index=drumtrack->index;

		sys->dtr_channel=drumtrack->dtr_channel;
		sys->dtr_key=drumtrack->key;
		sys->dtr_velocityoff=drumtrack->velocityoff;
	}
	else
	{
		if(song)
		{
			// 1. Index
			Drummap *map=&song->drummap;

			if(Drumtrack *dt=map->GetTrackIndex(dt_index))
			{
				if(dt->dtr_channel==dtr_channel &&
					dt->key==dtr_key)
				{
					sys->drumtrack=dt;
					return;
				}
			}

			// 2. Channel && key
			Drumtrack *dt=map->FirstTrack();
			while(dt)
			{
				if(dt->dtr_channel==dtr_channel &&
					dt->key==dtr_key)
				{
					sys->drumtrack=dt;
					return;
				}

				dt=dt->NextTrack();
			}

#ifdef DEBUG
			maingui->MessageBoxError(0,"ICD_Drum::CloneData drumtrack=0");
#endif

		}
#ifdef DEBUG
		else
			maingui->MessageBoxError(0,"ICD_Drum::CloneData song=0");
#endif

	}
}

LONGLONG ICD_Drum::GetSampleEnd(Seq_Song *song,Seq_Pattern *p)
{
	return song->timetrack.ConvertTicksToTempoSamples(GetEventStart(p)+drumtrack->ticklength);
}

LONGLONG ICD_Drum::GetSampleSize(Seq_Song *song,Seq_Pattern *p)
{
	return GetSampleEnd(song,p)-GetSampleStart(song,p);
}

int ICD_Drum::GetVelocity()
{
	int velo=velocity+drumtrack->volume;

	if(velo>127)
		return 127;

	if(velo<1)
		return 1;

	return velo;
}

void ICD_Drum::SendToDevicePlaybackUser(MIDIOutputDevice *device,Seq_Song *song,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	int velo=GetVelocity();

	device->sendNote(NOTEON|drumtrack->GetMIDIChannel(),drumtrack->key,(UBYTE)velo);

	if((createflag&Seq_Event::STD_CREATEREALEVENT)==0) // no off
		return;

#ifdef MEMPOOLS
	NoteOff_Realtime *noteoff=mainpools->mempGetNoteOff_Realtime();
#else
	NoteOff_Realtime *noteoff=new NoteOff_Realtime;
#endif

	if(noteoff)
	{
		// Init NoteOff
		noteoff->audioobject=0;
		noteoff->outputdevice=device;

		noteoff->fromevent=0;
		noteoff->rte_frompattern=0;
		noteoff->fromtrack=0;
		noteoff->status=NOTEOFF|drumtrack->GetMIDIChannel();
		noteoff->key=drumtrack->key;
		noteoff->velocityoff=drumtrack->velocityoff;
		noteoff->iflag=0;

		song->realtimeevents[REALTIMELIST_MIDI].AddCDAlarmEvent(noteoff,drumtrack->ticklength);
	}
}

void ICD_Drum::SendToDevicePlayback(MIDIOutputDevice *device,Seq_Song *song,Seq_Track *track,MIDIPattern *fpattern,Seq_Event *seqevent,int createflag)
{
	if(!device){
#ifdef DEBUG
		maingui->MessageBoxError(0,"MIDI Device"); 
#endif

		return;
	}

	if(CheckMute()==true)
	{

#ifdef MEMPOOLS
		NoteOff_Realtime *noteoff=mainpools->mempGetNoteOff_Realtime();
#else
		NoteOff_Realtime *noteoff=new NoteOff_Realtime;
#endif

		if(noteoff)
		{
			int velo=velocity+drumtrack->volume;

			if(velo>127)
				velo=127;
			else
				if(velo<1)
					velo=1;

			device->sendNote(NOTEON|drumtrack->GetMIDIChannel(),drumtrack->key,(UBYTE)velo);

			// Init NoteOff
			noteoff->outputdevice=device;
			noteoff->audioobject=0;

			noteoff->fromevent=seqevent;
			noteoff->rte_frompattern=fpattern;
			noteoff->fromtrack=track;
			noteoff->status=NOTEOFF|drumtrack->GetMIDIChannel();
			noteoff->key=drumtrack->key;
			noteoff->velocityoff=drumtrack->velocityoff;
			noteoff->iflag=0;

			track->song->realtimeevents[REALTIMELIST_MIDI].AddRealtimeAlarmEvent(noteoff,GetSampleEnd(track->song,fpattern));
		}
	}
}
