#include "songmain.h"
#include "seqtime.h"
#include "object_song.h"
#include "semapores.h"
#include "gui.h"
#include "MIDIoutproc.h"
#include "seqtime.h"
#include "audiofile.h"
#include "object_track.h"
#include "audiopattern.h"
#include "crossfade.h"

double Seq_Time::CleanTempoValue(double tempo)
{
	double h2=tempo*1000; // 120.00001 -> 120.000
	int h=(int)h2;
	h2=h;
	h2/=1000;

	return h2;
}

Seq_Tempo *Seq_Time::AddNewTempo(int newtype,OSTART startpos,double newtempo,bool repairloops)
{
	Seq_Tempo *ne=0;

	newtempo=CleanTempoValue(newtempo);

	if(newtempo>MINIMUM_TEMPO && newtempo<=MAXIMUM_TEMPO)
	{
		Seq_Tempo *e=FirstTempo();

		// Check for Tempo at same position
		while(e && e->ostart<=startpos)
		{
			if(e->ostart==startpos && e->type==newtype)
			{
				if(e->tempo!=newtempo)
				{
					e->tempo=newtempo; // Change old Tempo
					goto changed;
				}
				else
				{
					return 0;
				}
			}

			e=e->NextTempo();
		}

		if(ne = new Seq_Tempo){ // Create New

			ne->type=newtype;
			ne->map=this;
			ne->tempo=newtempo;

			goto changed;
		}
	}
	
	return 0;

changed:

	// Change or changed Tempo
	LockTimeTrack();

	if(ne)
		tempomap.AddOSort(ne,startpos);

	RefreshTempoFactor();
	UnlockTimeTrack();

	if(repairloops==true)
		song->RepairLoops(Seq_Song::RLF_NOLOCK|Seq_Song::RLF_CHECKREFRESH);

	return ne;
}

void Seq_Tempo::CloneData(Seq_Tempo *to)
{
	to->tempo=tempo;
	to->tempofactor=tempofactor;
	to->ostart=ostart;
//	to->staticostart=staticostart;
	to->map=map;
	to->type=type;
	to->flag=flag;
}

void Seq_Tempo::Load(camxFile *file)
{
	file->ReadChunk(&ostart);
	file->ReadChunk(&tempo);
	file->ReadChunk(&flag);
}

void Seq_Tempo::Save(camxFile *file)
{
	file->Save_Chunk(ostart);
	file->Save_Chunk(tempo);

	int fb=flag;
	fb CLEARBIT OFLAG_SELECTED;

	file->Save_Chunk(fb);
}

bool Seq_Tempo::ChangeTempo(Seq_Song *song,OSTART newstart,double newtempo,int iflag)
{
	if(newstart>=0 && (!(song->status&Seq_Song::STATUS_WAITPREMETRO)))
	{
		if(map->FirstTempo()==this)
			newstart=ostart; // cant change first tempo start !

		newtempo=newtempo==0?tempo:map->CleanTempoValue(newtempo);

		if(newtempo>=MINIMUM_TEMPO && newtempo<=MAXIMUM_TEMPO)
		{
			if(newtempo!=tempo || ostart!=newstart)
			{
				mainthreadcontrol->LockActiveSong();

				if(newstart!=ostart)
					map->tempomap.MoveO(this,newstart);

				tempo=newtempo;
				
				map->RefreshTempoFactor();
				//song->ResetSystemCounter();

				bool loopsrepaired=false;

				if(!(flag&TEMPOREFRESH_NOLOOPREFRESH))
					loopsrepaired=song->RepairLoops(Seq_Song::RLF_NOLOCK|Seq_Song::RLF_CHECKREFRESH);

				mainthreadcontrol->UnlockActiveSong();

				if(!(iflag&TEMPOREFRESH_NOGUI))
				{
					if(loopsrepaired==true)
						maingui->RefreshRepairedLoopsGUI(song);

					maingui->RefreshTempoGUI(song);
				}

				return true;
			}
		}
	}

	return false;
}

void Seq_Time::RefreshTempoFactor(bool refreshfull)
{
	refreshflag=0; // Reset refreshflag
	Seq_Tempo *ft=FirstTempo() /*,*lt=0*/;

	//ft->tempoposition_add=0;

	while(ft)
	{
		ft->tempofactor=120.0/ft->tempo;
		ft=ft->NextTempo();
	}

	if(refreshfull==true)
	RefreshTempoChanges();
}

void Seq_Time::RefreshTempoChanges()
{
	song->RefreshTempoBuffer();

	Seq_Track *t=song->FirstTrack();

	while(t)
	{
		/*
		// Automation Samples Starts
		AutomationTrack *sub=t->FirstAutomationTrack();

		while(sub)
		{
			// Init Samples
			if(sub->activeobject)
				sub->activeobject_samplestart=song->timetrack.ConvertTicksToTempoSamples(sub->activeobject->GetParameterStart());

			if(sub->playbackobject)
				sub->playbackobject_samplestart=song->timetrack.ConvertTicksToTempoSamples(sub->playbackobject->GetParameterStart());

			sub=sub->NextAutomationTrack();
		}
*/

		// Audio Pattern Loops Offset <>
		Seq_Pattern *p=t->FirstPattern();

		while(p)
		{
			if(p->mediatype==MEDIATYPE_AUDIO)
			{
				AudioPattern *ap=(AudioPattern *)p;

				if(ap->FirstLoopPattern())
				{
					int counter=ap->RefreshLoopPositions();
					if(counter)refreshflag|=LOOP_REFRESH;
				}
			}

			// Refresh CrossFades
			if(Seq_CrossFade *cf=p->FirstCrossFade())
			{
				do
				{
					if(cf->infade==false)
					{
						cf->SetCrossFadeSamplePositions();
						if(cf->connectwith)
							cf->connectwith->SetCrossFadeSamplePositions();
					}

					cf=cf->NextCrossFade();

				}while(cf);
			}

			p=p->NextPattern();
		}

		t=t->NextTrack();
	}

	flag |=AUDIOSTREAMREFRESH;

	if(song==mainvar->GetActiveSong() && (song->status&Seq_Song::STATUS_SONGPLAYBACK_MIDI))
	{
		//	song->ResetSystemCounter(songposition);
		mainMIDIalarmthread->SetSignal(); // Send Refresh Alarm, Recalc
	}
}