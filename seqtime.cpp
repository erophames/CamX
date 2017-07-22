#include "defines.h"
#include "object.h"
#include "seqtime.h"
#include "songmain.h"
#include "audiodevice.h"
#include "object_song.h"
#include "audiohardware.h"
#include "gui.h"
#include "languagefiles.h"
#include "arrangeeditor.h"
#include "semapores.h"
#include "object_project.h"
#include "editdata.h"
#include "chunks.h"
#include "camxfile.h"

Seq_Time::Seq_Time ()
{
	// default 120 BPM, 4/4 Signature, zoom 1/16
	// StartEvents
	zoomticks=TICK16nd;	 // 1/16nd 	
	newMIDIclocktempo_record=false;
	refreshflag=0;
	lastselectedtempo=0;
	flag=0;
	ppqsampleratemul=-1;
}

void Seq_Time::RemoveAllTimeMaps(bool full)
{
	RemoveSignatureMap(full);
	RemoveTempoMap(full);
}

Seq_Tempo *Seq_Time::RemoveTempo(Seq_Tempo *t)
{
	LockTimeTrack();
	Seq_Tempo *n=(Seq_Tempo *)tempomap.RemoveO(t);
	UnlockTimeTrack();

	return n;
}

void Seq_Time::Load(camxFile *file)
{
	file->CloseReadChunk();
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_TEMPOMAP)
		LoadTempoMap(file);

	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_SIGNATUREMAP)
	{
		file->ChunkFound();

		int nrsigs=0;
		file->ReadChunk(&nrsigs);

		if(nrsigs)
		{
			signaturemap.DeleteAllO(); // Delete old Signature

			while(nrsigs--){

				if(Seq_Signature *sig=new Seq_Signature)
				{
					sig->Load(file);
					sig->map=this; // map set later !

					signaturemap.AddOSort(sig,sig->ostart);
				}
			}

			signaturemap.Close();

			RefreshSignatureMeasures();
		}

		file->CloseReadChunk();
	}
}

void Seq_Time::LoadTempoMap(camxFile *file)
{
	file->ChunkFound();

//	OSTART songposition=song->GetSongPosition();

	int nrtempos=0;
	file->ReadChunk(&nrtempos);

	if(nrtempos)
	{
		tempomap.DeleteAllO(); // Delete old Tempomap
		
		while(nrtempos--){

			if(Seq_Tempo *tempo=new Seq_Tempo){
				tempo->Load(file);
				tempo->map=this; // Connect to map
				tempomap.AddOSort(tempo,tempo->ostart);
			}
		}

		Close();
		RefreshTempoFactor(false); // No RefreshTempoChanges
	}

	file->CloseReadChunk();
}

void Seq_Time::SaveTempoMap(camxFile *file)
{
	file->OpenChunk(CHUNK_TEMPOMAP);
	file->Save_Chunk(tempomap.GetCount());

	Seq_Tempo *t=FirstTempo();

	while(t){
		t->Save(file);
		t=t->NextTempo();
	}

	file->CloseChunk();
}

void Seq_Time::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_TIMETRACK);
	file->CloseChunk();

	// Tempo Map
	SaveTempoMap(file);

	// Signature Map
	file->OpenChunk(CHUNK_SIGNATUREMAP);
	file->Save_Chunk(signaturemap.GetCount());

	Seq_Signature *s=FirstSignature();

	while(s){
		s->Save(file);
		s=s->NextSignature();
	}

	file->CloseChunk();
}

void Seq_Time::RemoveTempoMap(bool full)
{
	Seq_Tempo *t=FirstTempo();

	if(full==false){

		OSTART songposition=song->GetSongPosition();

		LockTimeTrack();
		t->tempo=120; // default
		RefreshTempoFactor();
		UnlockTimeTrack();

		t=t->NextTempo();
	}

	while(t)
		t=RemoveTempo(t);
}

void Seq_Time::SelectTempo(Seq_Tempo *tempo,bool select)
{
	if((select==true && (!(tempo->flag&OFLAG_SELECTED))) ||
		(select==false && (tempo->flag&OFLAG_SELECTED))
		)
	{
		if(select==true)
			tempo->flag|=OFLAG_SELECTED;
		else
			tempo->flag^=OFLAG_SELECTED;
	}
}

void Seq_Time::SelectAllTempos(bool select)
{
	tempomap.Select(select);
}

void Seq_Time::CreateSignatureAndEdit(guiWindow *win,OSTART time)
{
	time=ConvertTicksToMeasureTicks(time,false); // Quant

	int measure=ConvertTicksToMeasure(time);

	Seq_Signature *check=FindSignature(time);

	if((!check) || check->GetSignatureStart()>time)
	{
		mainthreadcontrol->LockActiveSong();
		Seq_Signature *newsig=AddNewSignature(measure,4,TICK4nd);
		mainthreadcontrol->UnlockActiveSong();

		if(newsig)
		{
			maingui->RefreshAllEditors(song,REFRESHSIGNATURE_DISPLAY);
			//maingui->RefreshTimeSlider(song);

			if(EditData *edit=new EditData)
			{
				edit->song=song;
				edit->signature=newsig;

				edit->win=win;
				edit->x=win->GetWindowMouseX();
				edit->y=win->GetWindowMouseY();

				TimeString timestring;

				song->timetrack.CreateTimeString(&timestring,newsig->GetSignatureStart(),Seq_Pos::POSMODE_NORMAL);

				edit->title=mainvar->GenerateString(Cxs[CXS_NEW],":",timestring.string);
				edit->deletename=true;

				edit->id=EDIT_SIGNATURE;

				edit->type=EditData::EDITDATA_TYPE_SIGNATURE;
				edit->deletesigifcancel=true;

				maingui->EditDataValue(edit);
			}
		}
	}
}

void Seq_Time::DeleteSignature(guiWindow *win,Seq_Signature *sig)
{
	if(sig)
	{
		Seq_Time *map=sig->map;

		if(map && map->FirstSignature()!=sig)
		{
			mainthreadcontrol->LockActiveSong();

			LockTimeTrack();
			map->RemoveSignature(sig);
			map->RefreshSignatureMeasures();
			UnlockTimeTrack();

			mainthreadcontrol->UnlockActiveSong();

			maingui->RefreshAllEditors(win->WindowSong(),REFRESHSIGNATURE_DISPLAY);
			//maingui->RefreshTimeSlider(win->WindowSong());
		}
	}
}

void Seq_Time::EditSignature(guiWindow *win,Seq_Signature *f)
{
	if(f)
	{
		if(EditData *edit=new EditData)
		{
			edit->song=song;
			edit->signature=f;

			edit->win=win;
			edit->x=win->GetWindowMouseX();
			edit->y=win->GetWindowMouseY();

			TimeString timestring;

			song->timetrack.CreateTimeString(&timestring,f->GetSignatureStart(),Seq_Pos::POSMODE_NORMAL);

			edit->title=mainvar->GenerateString(Cxs[CXS_SIGNATURE],":",timestring.string);
			edit->deletename=true;
			edit->id=EDIT_SIGNATURE;
			edit->type=EditData::EDITDATA_TYPE_SIGNATURE;

			maingui->EditDataValue(edit);
		}
	}
}

void Seq_Time::EditSignature(guiWindow *win,OSTART time)
{
	EditSignature(win,FindSignatureBefore(time));
}

void Seq_Time::OpenPRepairTempoSelection()
{
	Seq_Tempo *t=FirstTempo();

	while(t)
	{
		if(t->flag&OFLAG_UNDERSELECTION)
		{
			t->flag|=OFLAG_OLDUNDERSELECTION;
			t->flag CLEARBIT OFLAG_UNDERSELECTION;
		}
		else
			t->flag CLEARBIT OFLAG_OLDUNDERSELECTION;

		t=t->NextTempo();
	}
}

Seq_Tempo *Seq_Time::FirstSelectedTempo()
{
	Seq_Tempo *t=FirstTempo();

	while(t)
	{
		if(t->IsSelected()==true)
			return t;

		t=t->NextTempo();
	}

	return 0;
}

void Seq_Time::CloneTempos(OListStart *list)
{
	Seq_Tempo *t=FirstTempo();

	while(t)
	{
		if(Seq_Tempo *nt=new Seq_Tempo)
		{
			t->CloneData(nt);
			list->AddEndO(nt);
		}

		t=t->NextTempo();
	}
}

double Seq_Time::GetLowestTempo()
{
	Seq_Tempo *t=FirstTempo();
	double low=t->tempo;

	t=t->NextTempo();

	while(t){
		if(t->tempo<low)
			low=t->tempo;

		t=t->NextTempo();
	}

	return low;
}

void Seq_Time::RepairTempomap()
{
	RefreshTempoFactor();
	
	refreshflag CLEARBIT Seq_Time::LOOP_REFRESH;
	song->RepairLoops(Seq_Song::RLF_NOLOCK);
}

double Seq_Time::GetHighestTempo()
{
	Seq_Tempo *t=FirstTempo();
	double low=t->tempo;

	t=t->NextTempo();

	while(t){
		if(t->tempo>low)
			low=t->tempo;

		t=t->NextTempo();
	}

	return low;
}

int Seq_Time::GetSelectedTempos()
{
	int c=0;
	Seq_Tempo *t=FirstTempo();

	while(t){
		if(t->flag&OFLAG_SELECTED)c++;
		t=t->NextTempo();
	}

	return c;
}

double Seq_Time::SubTicksToTempoTicks(double ticks)
{
	Seq_Tempo *starttempo=FirstTempo();

	if(Seq_Tempo *nexttempo=starttempo->NextTempo())
	{
		OSTART endticks=(OSTART)ticks;

		if(nexttempo->GetTempoStart()<endticks)
		{
			double tickcount=0,lastfactor=starttempo->tempofactor;
			OSTART laststart=0;

			do{
				tickcount+=(double)(nexttempo->GetTempoStart()-laststart)*lastfactor;

				laststart=nexttempo->GetTempoStart();
				lastfactor=nexttempo->tempofactor;
				nexttempo=nexttempo->NextTempo();

			}while(nexttempo && nexttempo->GetTempoStart()<endticks);

			//add rest ?
			if(laststart<endticks)
				tickcount+=(double)(endticks-laststart)*lastfactor;

			return tickcount;
			//return floor(tickcount+0.5);
		}
	}

	return ticks*starttempo->tempofactor;  //No Tempomap, or last tempo
}

double Seq_Time::SubTicksToTempoTicks(OSTART startticks,double ticks)
{
	Seq_Tempo *starttempo=GetTempo(startticks);	

	if(Seq_Tempo *nexttempo=starttempo->NextTempo())
	{
		OSTART endticks=startticks+(OSTART)ticks;

		if(nexttempo->GetTempoStart()<endticks)
		{
			double tickcount=0,lastfactor=starttempo->tempofactor;
			OSTART laststart=startticks;

			do{
				tickcount+=(double)(nexttempo->GetTempoStart()-laststart)*lastfactor;

				laststart=nexttempo->GetTempoStart();
				lastfactor=nexttempo->tempofactor;
				nexttempo=nexttempo->NextTempo();

			}while(nexttempo && nexttempo->GetTempoStart()<endticks);

			//add rest ?
			if(laststart<endticks)
				tickcount+=(double)(endticks-laststart)*lastfactor;

			return tickcount;
			//return floor(tickcount+0.5);
		}
	}

	return ticks*starttempo->tempofactor;  //No Tempomap, or last tempo
}

double Seq_Time::AddTempoToTicks(OSTART startticks,double ticks)
{
	Seq_Tempo *starttempo=GetTempo(startticks);	

	if(Seq_Tempo *nexttempo=starttempo->NextTempo())
	{
		OSTART endticks=startticks+(OSTART)ticks;

		if(nexttempo->GetTempoStart()<endticks)
		{
			double tickcount=0,lastfactor=starttempo->tempofactor;
			OSTART laststart=startticks;

			do{
				tickcount+=(double)(nexttempo->GetTempoStart()-laststart)/lastfactor;

				laststart=nexttempo->GetTempoStart();
				lastfactor=nexttempo->tempofactor;
				nexttempo=nexttempo->NextTempo();

			}while(nexttempo && nexttempo->GetTempoStart()<endticks);

			//add rest ?
			if(laststart<endticks)
				tickcount+=(double)(endticks-laststart)/lastfactor;

			return tickcount;
			//return floor(tickcount+0.5);
		}
	}

	return ticks/starttempo->tempofactor;  //No Tempomap, or last tempo
}

OSTART Seq_Time::ConvertTempoTicksToTicks(OSTART startticks,double ticks)
{
	Seq_Tempo *starttempo=GetTempo(startticks);

	if(Seq_Tempo *nexttempo=starttempo->NextTempo())
	{
		//	double h=ticks/starttempo->tempofactor;
		//	h+=startticks;

		if(startticks+(OSTART)(ticks/starttempo->tempofactor)>nexttempo->ostart) // Next Tempo inside |start---|end
		{
			double lastfactor=starttempo->tempofactor;
			OSTART laststart=startticks,tickcount=0;

			for(;;)
			{
				OSTART diff=nexttempo->ostart-laststart;

				tickcount+=diff;
				ticks-=(double)diff*lastfactor;

				lastfactor=nexttempo->tempofactor;
				laststart=nexttempo->ostart;

				if(!(nexttempo=nexttempo->NextTempo()))break;

				//h=ticks/lastfactor;
				//h+=laststart;

				if(laststart+(OSTART)(ticks/lastfactor)<=nexttempo->ostart)
					break;
			}//while(h>nexttempo->ostart);

			ticks/=lastfactor;

			return startticks+tickcount+(OSTART)ticks;
		}
	}

	return startticks+(OSTART)(ticks/starttempo->tempofactor);
}

double Seq_Time::ConvertTempoTicksToTicks(double ticks)
{
	Seq_Tempo *starttempo=FirstTempo();

	if(Seq_Tempo *nexttempo=starttempo->NextTempo())
	{
		//double h=ticks/starttempo->tempofactor;

		if((OSTART)(ticks/starttempo->tempofactor)>nexttempo->ostart) // Next Tempo inside |start---|end
		{
			double lastfactor=starttempo->tempofactor;
			OSTART tickcount=0,laststart=0;

			for(;;){

				OSTART diff=nexttempo->ostart-laststart;
				
				tickcount+=diff;
				ticks-=(double)diff*lastfactor;

				lastfactor=nexttempo->tempofactor;
				laststart=nexttempo->ostart;

				if(!(nexttempo=nexttempo->NextTempo()))break;

				if(laststart+(OSTART)(ticks/lastfactor)<=nexttempo->ostart)
					break;

			}

			ticks/=lastfactor;
			ticks+=tickcount;

			return ticks;
		}
	}

	return ticks/starttempo->tempofactor;
}

double Seq_Time::ConvertTempoTicksToTicks(OSTART startticks,double ticks,Seq_Tempo **writetempo)
{
	Seq_Tempo *starttempo=GetTempo(startticks);

	if(Seq_Tempo *nexttempo=starttempo->NextTempo())
	{
		if((OSTART)(ticks/starttempo->tempofactor)>nexttempo->ostart) // Next Tempo inside |start---|end
		{
			double lastfactor=starttempo->tempofactor;
			OSTART tickcount=0,laststart=startticks;

			//h+=startticks;

			for(;;)
			{
				OSTART diff=nexttempo->ostart-laststart;
				tickcount+=diff;
				ticks-=(double)diff*lastfactor;

				lastfactor=nexttempo->tempofactor;
				laststart=nexttempo->ostart;

				*writetempo=nexttempo;

				if(!(nexttempo=nexttempo->NextTempo()))break;

				//h=ticks/lastfactor;
				//h+=laststart;

				if(laststart+(OSTART)(ticks/lastfactor)<=nexttempo->ostart)
					break;

			}//while(h>nexttempo->ostart);

			ticks/=lastfactor;
			ticks+=tickcount;

			return (double)startticks+ticks;
		}
	}

	*writetempo=starttempo;
	return (double)startticks+ticks/starttempo->tempofactor;
}


LONGLONG Seq_Time::ConvertTicksToTempoSamplesStart(OSTART startticks,double ticks)
{
	if(ticks<0)
		return (LONGLONG)(-SubTicksToTempoTicks(startticks,-ticks)*ppqsampleratemul);

	return (LONGLONG)(SubTicksToTempoTicks(startticks,ticks)*ppqsampleratemul);
}

LONGLONG Seq_Time::ConvertTicksToTempoSamplesStart(OSTART startticks,OSTART ticks)
{
	if(ticks<0)
		return (LONGLONG)(-SubTicksToTempoTicks(startticks,(double)-ticks)*ppqsampleratemul);

	return (LONGLONG)(SubTicksToTempoTicks(startticks,(double)ticks)*ppqsampleratemul);
}

LONGLONG Seq_Time::ConvertTicksToTempoSamples(OSTART ticks)
{
	if(ticks<0)
		return (LONGLONG)(-SubTicksToTempoTicks((double)-ticks)*ppqsampleratemul);

	return (LONGLONG)(SubTicksToTempoTicks((double)ticks)*ppqsampleratemul);
}

LONGLONG Seq_Time::ConvertTicksToTempoSamples(double ticks)
{
	if(ticks<0)
		return (LONGLONG)(-SubTicksToTempoTicks(-ticks)*ppqsampleratemul);

	return (LONGLONG)(SubTicksToTempoTicks(ticks)*ppqsampleratemul);
}

OSTART Seq_Time::ConvertSamplesToOSTART(LONGLONG sampleposition)
{
	return ConvertTempoTicksToTicks((double)sampleposition/ppqsampleratemul);
}

void Seq_Time::ConvertSamplesToOSTART(LONGLONG s1,OSTART *o1,LONGLONG s2,OSTART *o2)
{
	*o1=ConvertTempoTicksToTicks((double)s1/ppqsampleratemul);
	*o2=ConvertTempoTicksToTicks((double)s2/ppqsampleratemul);
}

void Seq_Time::ConvertLengthToPos(Seq_Pos *pos,OSTART from,OSTART ticks)
{
	Seq_Signature *sig=FindSignatureBefore(from),*nextsig;
	OSTART endticks=from+ticks;

	pos->pos[0]=pos->pos[1]=pos->pos[2]=pos->pos[3]=pos->pos[4]=0;

	while(sig && sig->GetSignatureStart()<endticks && ticks)
	{
		nextsig=sig->NextSignature();

		OSTART measure;

		if(nextsig && nextsig->GetSignatureStart()<endticks)
		{
			measure=nextsig->GetSignatureStart()-from;
			measure/=sig->measurelength;
		}
		else
			measure=ticks/sig->measurelength;

		ticks-=measure*sig->measurelength;
		pos->pos[0]+=(int)measure;

		sig=nextsig;

		if(sig)
			from=sig->GetSignatureStart();
	}
}

void Seq_Time::CreateLengthString(TimeString *timestr,OSTART time,OSTART length,int format)
{
	timestr->pos.mode=format;
	timestr->pos.song=song;
	timestr->pos.measureformat=song->project->projectmeasureformat;

	ConvertTicksToLength(time,length,&timestr->pos);
	timestr->index=timestr->pos.index;
	timestr->pos.ConvertToLengthString(song,timestr->string,70,timestr->stringspointer);
}

void Seq_Time::CreateTimeString(TimeString *timestr,OSTART time,int format,int iflag)
{
	timestr->time=time;
	timestr->pos.mode=format;
	timestr->pos.offset=&song->smpteoffset;

	ConvertTicksToPos(time,&timestr->pos);

	timestr->index=timestr->pos.index;
	timestr->pos.ConvertToString(song,timestr->string,70,timestr->stringspointer,iflag);
}

OSTART Seq_Time::ConvertPosToTicks(Seq_Pos *pos)
{
	switch(pos->mode)
	{
	case Seq_Pos::POSMODE_NORMAL: case Seq_Pos::POSMODE_COMPRESS:
		{
			Seq_Signature *sig=FindSignatureBefore(ConvertMeasureToTicks(pos->pos[0]));

			// measure ******************
			OSTART m=ConvertTicksToMeasure(sig->GetSignatureStart());

			m=pos->pos[0]-m;

			OSTART t=sig->GetSignatureStart();

			t+=m*sig->measurelength;

			// x-1-x-x
			t+=sig->dn_ticks*(pos->pos[1]-1);

			if(pos->mode==Seq_Pos::POSMODE_COMPRESS)
			{
				double dh=mainaudio->ConvertPPQToInternRate(pos->pos[2]-1); // x-x-1, no Zoom
				t+=dh;
			}
			else
			{
				// x-x-1-x
				switch(song->project->projectmeasureformat)
				{
				case PM_1111:
				case PM_1p1p1p1:
				case PM_1110:
				case PM_1p1p1p0:
					t+=zoomticks*(pos->pos[2]-1);
					break;
				}

				if(pos->mode==Seq_Pos::POSMODE_NORMAL)
				{
					switch(song->project->projectmeasureformat)
					{
					case PM_1111:
					case PM_1p1p1p1:
					case PM_11_1:
					case PM_1p1p_1:
						{
#ifdef DEBUG
							if(pos->pos[pos->index-1]==0)
								maingui->MessageBoxOk(0,"POS3==0");
#endif
							double dh=mainaudio->ConvertPPQToInternRate(pos->pos[pos->index-1]-1); // x-x-x-1 or x-x-1
							t+=dh;
						}
						break;

					case PM_1110:
					case PM_1p1p1p0:
					case PM_11_0:
					case PM_1p1p_0:
						{
							double dh=mainaudio->ConvertPPQToInternRate(pos->pos[pos->index-1]); // x-x-x-0 or x-x-0
							t+=dh;
						}
						break;
					}
				}
			}

			return t;
		}
		break;

	default:
		{
			if(pos->IsSmpte()==true)
			{
				double ms=
					pos->pos[0]*3600+ // h
					pos->pos[1]*60+ // min
					pos->pos[2]; // sec

				ms*=1000;

				// Frame
				double ffact=1000;
				ffact/=SMPTE_FPS[pos->mode];

				// Frame
				ms+=ffact*pos->pos[3];

				// QF Factor
				ffact/=4;
				ms+=ffact*pos->pos[4];

				/*
				// Quantize MS<->Quarter Frames
				ms/=ffact;
				modf (ms , &ms);
				ms*=ffact;
				*/

				if(pos->offset)
				ms-=pos->offset->GetOffSetMs();

				ms*=INTERNRATEMSMUL; // ms->ticks

				Seq_Tempo *lasttempo;

				double newticks=ConvertTempoTicksToTicks(0,ms,&lasttempo),tempoticks=ConvertTempoTicksToTicks(lasttempo->ostart),
					h=newticks-tempoticks,h2=INTERNRATEMSMUL/lasttempo->tempofactor;

				TRACE ("ConvertPosToMS Ticks %f LastStart %d LastFactor %f\n",newticks,lasttempo->ostart,lasttempo->tempofactor);

				if(h>=h2)return newticks+1;
				return newticks;

				//return ceil(ticks);
				//return ConvertTicksToTempoTicks(0,ms);
				//return ConvertMsToTempoTicks(ms); // Calc Tempomap, SMPTE use tempompap
				//return mainvar->ConvertMilliSecToTicks(ms);
			}
			else
			{
				// H:M:Sec
				double ms=
					pos->pos[0]*3600+ // h
					pos->pos[1]*60+ // min
					pos->pos[2]; // sec

				ms*=1000;

				ms+=pos->pos[3]*10;

				ms*=INTERNRATEMSMUL; // ms->ticks

				Seq_Tempo *lasttempo;

				double newticks=ConvertTempoTicksToTicks(0,ms,&lasttempo),tempoticks=ConvertTempoTicksToTicks(lasttempo->ostart),
					h=newticks-tempoticks,h2=INTERNRATEMSMUL/lasttempo->tempofactor;

			//	TRACE ("ConvertPosToMS Ticks %f LastStart %d LastFactor %f\n",newticks,lasttempo->ostart,lasttempo->tempofactor);

				if(h>=h2)return newticks+1;
				return newticks;

			}

		}
		break;

	}	

	return -1; // unknown pos format ?
}

void Seq_Time::ConvertTicksToPos(OSTART time,Seq_Pos *pos,OSTART zticks)
{
	if(!zticks)
		zticks=zoomticks; // default Song Zoom ticks

	pos->song=song;

	if(song)
		pos->measureformat=song->project->projectmeasureformat;

	switch(pos->measureformat)
	{
	case PM_1p1p_1:
	case PM_1p1p_0:
	case PM_11_1:
	case PM_11_0:
		pos->nozoom=true;
		break;

	default:
		pos->nozoom=false;
		break;
	}

	switch(pos->mode)
	{
	case Seq_Pos::POSMODE_NORMAL:
	case Seq_Pos::POSMODE_COMPRESS:
		if(song)
		{
			switch(pos->measureformat)
			{
			case PM_1p1p1p1:
			case PM_1p1p_1:
			case PM_1p1p1p0:
			case PM_1p1p_0:
				pos->space=measure_str; // . . . .
				break;

			default:
				pos->space=measure_str_empty; // _ _ _ _
				break;
			}
			
			pos->usesmpte=false;

			Seq_Signature *sig; 

			if(sig=LastSignature())
			{
				while(sig && sig->GetSignatureStart()>time && sig->PrevSignature())
					sig=sig->PrevSignature();

				time-=sig->GetSignatureStart(); // - Signaturemap last found Signature Event
			}

			pos->sig=sig;
			pos->zoomticks=zticks;

			pos->pos[0]=time/sig->measurelength;
			time-=sig->measurelength*(OSTART)pos->pos[0];

			pos->pos[1]=time/sig->dn_ticks; // X-1-X-X
			time-=sig->dn_ticks*(OSTART)pos->pos[1];

			if(pos->nozoom==true || pos->mode==Seq_Pos::POSMODE_COMPRESS)
			{
				// 1.1.5000
				pos->pos[2]=mainaudio->ConvertInternRateToPPQ(time); // Rest Ticks
				pos->pos[3]=1; // set to 1

				pos->index=3;
			}
			else
			{
				pos->pos[2]=time/zticks; // X-X-1-ticks
				time-=zticks*(OSTART)pos->pos[2];
				pos->pos[3]=mainaudio->ConvertInternRateToPPQ(time); // Rest Ticks 
				pos->index=4;
			}
	
			pos->pos[0]+=sig->sig_measure;
			pos->pos[1]++;

			switch(pos->measureformat)
			{
			case PM_1111:
			case PM_1p1p1p1:
				pos->pos[2]++;
				pos->pos[3]++;
				break;

			case PM_11_1:
			case PM_1p1p_1:
				pos->pos[2]++;
				break;

			case PM_1110:
			case PM_1p1p1p0:
				pos->pos[2]++;
				break;

			case PM_11_0:
			case PM_1p1p_0:
				break;
			}
		}
		break;

	case Seq_Pos::POSMODE_SAMPLES:
		{
			pos->pos[0]=song->timetrack.ConvertTicksToTempoSamples(time);
			pos->index=1;
		}
		break;

	default:
		if(pos->IsSmpte()==true || pos->mode==Seq_Pos::POSMODE_TIME)
		{
			pos->usesmpte=true;

			double ms=SubTicksToTempoTicks(time);

			ms*=INTERNRATEMSDIV;// ticks->ms

			if(pos->IsSmpte()==true && song && pos->offset)
			{
				ms+=pos->offset->GetOffSetMs();
			}

			if(ms<0)
			{
				pos->minus=true;
				ms=-ms;
			}
			else
				pos->minus=false;

			// Hour
			double h=ms;				
			h/=1000*60*60;

			pos->pos[0]=(OSTART)h; // round down

			ms-=pos->pos[0]*1000*60*60;

#ifdef _DEBUG
			if(ms<0)
				MessageBox(NULL,"Convert Ticks to Pos Error Ms<0","Error",MB_OK);

			if(pos->pos[0]<0)
			{
				MessageBox(NULL,"Convert Ticks to Pos error Hour","Error",MB_OK);
			}
#endif
			// Min
			h=ms;
			h/=60*1000;
			pos->pos[1]=(OSTART)h; // round down

			ms-=pos->pos[1]*60*1000;

#ifdef _DEBUG
			if(ms<0)
				MessageBox(NULL,"Convert Ticks to Pos Error Ms<0","Error",MB_OK);

			if(pos->pos[1]<0)
			{
				MessageBox(NULL,"Convert Ticks to Pos error Min","Error",MB_OK);
			}
#endif

			// Sec
			h=ms;
			h/=1000;
			pos->pos[2]=(OSTART)h;

#ifdef _DEBUG
			if(ms<0)
				MessageBox(NULL,"Convert Ticks to Pos Error Ms<0","Error",MB_OK);

			if(pos->pos[2]<0)
			{
				MessageBox(NULL,"Convert Ticks to Pos error Sec","Error",MB_OK);
			}
#endif	

			if(pos->mode==Seq_Pos::POSMODE_TIME)
			{
				ms-=pos->pos[2]*1000;

				h=ms;
				h/=10;

				pos->pos[3]=h; // /sec100

				pos->index=4;
				pos->space=sec_str;
			}
			else
			{
				double framefactor=1000;

				pos->space=smpte_str;
				pos->index=5;

				ms-=pos->pos[2]*1000;

				framefactor/=SMPTE_FPS[pos->mode];

				h=ms;
				h/=framefactor;

				// frames
				pos->pos[3]=(OSTART)h;
				ms-=pos->pos[3]*framefactor;

				//	qframes
				framefactor/=4;

				//TRACE ("MSREST %f\n",ms);

				h=ms;
				h/=framefactor;
				pos->pos[4]=h;

#ifdef _DEBUG
				//ms-=pos->pos[4]*framefactor;
				//TRACE ("Rest ms %f P4 %d\n",ms,pos->pos[4]);

				if(pos->pos[3]<0)
				{
					MessageBox(NULL,"Convert Ticks to Pos error Frames","Error",MB_OK);
				}
#endif		
			}
		}
		break;
	}	
}

OSTART Seq_Time::ConvertPosToLength(OSTART from,Seq_Pos *pos)
{
	switch(pos->mode)
	{
	case Seq_Pos::POSMODE_NORMAL: case Seq_Pos::POSMODE_COMPRESS:
		{
			pos->sig=FindSignatureBefore(from);
			pos->zoomticks=zoomticks;

			OSTART l=
				pos->pos[0]*pos->sig->measurelength+
				pos->pos[1]*pos->sig->dn_ticks+
				pos->pos[2]*zoomticks;

			double dh=mainaudio->ConvertPPQToInternRate(pos->pos[3]);

			l+=dh;

			return l;
		}
		break;

		case Seq_Pos::POSMODE_TIME:
			{
				double ms=
						pos->pos[0]*3600+ // h
						pos->pos[1]*60+ // min
						pos->pos[2]; // sec

					ms*=1000;

					ms*=INTERNRATEMSMUL; // ms->ticks

					double h1,h2=modf(ms,&h1);// 121.1 > 122, 121.0 = 121
					if(h2>0)
						ms++;

					return SubTicksToTempoTicks(from,ms); // Ticks->Tempo Ticks
			}
			break;

		default:
			{
				if(pos->IsSmpte()==true)
				{
					double ms=
						pos->pos[0]*3600+ // h
						pos->pos[1]*60+ // min
						pos->pos[2]; // sec

					ms*=1000;

					// Frame
					double ffact=1000;

					ffact/=SMPTE_FPS[pos->mode];

					// Frame
					ms+=ffact*pos->pos[3];

					// QF Factor
					ffact/=4;
					ms+=ffact*pos->pos[4];

					ms*=INTERNRATEMSMUL; // ms->ticks

					double h1,h2=modf(ms,&h1);// 121.1 > 122, 121.0 = 121
					if(h2>0)
						ms++;

					return SubTicksToTempoTicks(from,ms); // Ticks->Tempo Ticks
				}
			}
			break;

	}	

	return 0;
}

void Seq_Time::ConvertTicksToLength(OSTART from,OSTART length,Seq_Pos *pos)
{
	//length++; // Min 1 Tick

	//pos->startposition=from;

	pos->song=song;
	pos->length=true;

	switch(pos->mode)
	{
	case Seq_Pos::POSMODE_NORMAL: case Seq_Pos::POSMODE_COMPRESS:
		{
			pos->usesmpte=false;

			switch(pos->measureformat)
			{
			case PM_1p1p1p1:
			case PM_1p1p_1:
			case PM_1p1p1p0:
			case PM_1p1p_0:
				pos->space=measure_str; // . . . .
				break;

			default:
				pos->space=measure_str_empty; // _ _ _ _
				break;
			}

			pos->sig=FindSignatureBefore(from);
			pos->zoomticks=zoomticks;

			//	int h=sig->measurelength;			// 1-X-X-X
			pos->pos[0]=(int)(length/pos->sig->measurelength);
			length-=pos->sig->measurelength*pos->pos[0];

			pos->pos[1]=(int)(length/pos->sig->dn_ticks); // X-1-X-X

			length-=pos->sig->dn_ticks*pos->pos[1];

			if(pos->nozoom==true)
			{
				pos->pos[2]=0;
				pos->pos[3]=(int)mainaudio->ConvertInternRateToPPQ(length);

				pos->index=4;
			}
			else
			{
				if(pos->mode==Seq_Pos::POSMODE_NORMAL)
				{
					switch(pos->measureformat)
					{
					case PM_1p1p1p1:
					case PM_1p1p1p0:
					case PM_1111:
					case PM_1110:
						{
							pos->pos[2]=(int)(length/zoomticks); // X-X-1-ticks
							length-=zoomticks*pos->pos[2];
							pos->pos[3]=(int)mainaudio->ConvertInternRateToPPQ(length); // Rest Ticks
							pos->index=4;

							/*
							switch(pos->measureformat)
							{
							case PM_1p1p1p1:
							case PM_1111:
								pos->pos[3]++;
								break;
							}
							*/
						}
						break;

					default:
						pos->pos[2]=(int)mainaudio->ConvertInternRateToPPQ(length); // Rest Ticks, 1-1-ticks
						pos->index=3;

						/*
						switch(pos->measureformat)
						{
						case PM_11_1:
						case PM_1p1p_1:
							pos->pos[2]++;
							break;
						}*/

						break;
					}
				}
				else
				{
					pos->pos[2]=(int)mainaudio->ConvertInternRateToPPQ(length);	// compress mode : 1-1-ticks
					pos->index=3;
				}
			}

			// Offset
			//if(withmap==true)
			{
				//	pos->pos[0]+=sig->measure;

				//pos->pos[1]++;
				//pos->pos[2]++;
				//pos->pos[3]++;
			}
		}
		break;

	case Seq_Pos::POSMODE_SAMPLES:
		{
			LONGLONG s=ConvertTicksToTempoSamplesStart(from,length);

			pos->usesmpte=false;
			pos->pos[0]=s;
			pos->index=1;
		}
		break;

	default:
		{
			if(pos->IsSmpte()==true || pos->mode==Seq_Pos::POSMODE_TIME)
			{
				double samples=ConvertTicksToTempoSamplesStart(from,length);
				double h2=song->project->projectsamplerate;

				// Hour
				double h=h2*60*60;				
				double hr=floor(samples/h);

				pos->pos[0]=(int)hr; // round down
				samples-=hr*h;

				// Min
				h=h2*60;
				double min=floor(samples/h);

				pos->pos[1]=(int)min; // round down
				samples-=min*h;

				// Sec
				h=h2;
				double sec=floor(samples/h);

				pos->pos[2]=(int)sec; // round down
				samples-=sec*h;

				if(pos->IsSmpte()==true) // Frames
				{
					pos->usesmpte=true;
					pos->space=smpte_str;

					h=h2/SMPTE_FPS[pos->mode];

					double frames=floor(samples/h);

					pos->pos[3]=(int)frames;
					samples-=frames*h;

					// QF
					h/=4;
					double qf=samples/h;
					pos->pos[4]=(int)qf;

					pos->index=5;
				}
				else
				{

					pos->space=sec_str;
					pos->index=4;

					h=samples/h2;

					h*=100;

					pos->pos[3]=(int)h;
				}
			}
		}
		break;
	}	
}


int Seq_Time::ConvertTicksToMeasure(OSTART ticks)
{
	Seq_Pos spos(Seq_Pos::POSMODE_COMPRESS);
	ConvertTicksToPos(ticks,&spos);
	return spos.pos[0];
}

OSTART Seq_Time::ConvertMeasureToTicks(OSTART measure) // 1.1.1.0 or more, 1 min
{
#ifdef _DEBUG
	if(measure<1)
		MessageBox(NULL,"Illegal Convert Measure To Ticks","Error",MB_OK);
#endif
	if(measure>=0)
	{
		Seq_Signature *sig=FirstSignature();	

		while(sig)
		{
			if(sig->sig_measure==measure)
				return sig->GetSignatureStart();

			if(sig->sig_measure<measure)
				return sig->ostart+sig->measurelength*(measure-sig->sig_measure);

			sig=sig->NextSignature();
		}
	}

	return 0;
}

LONGLONG Seq_Time::ConvertSamplesToNextSamples(LONGLONG tsamples,LONGLONG step_samples)
{
	LONGLONG c=tsamples/step_samples;
	LONGLONG qsamples=c*step_samples;

	if(qsamples==tsamples)
		return tsamples;

	return qsamples+step_samples;
}

OSTART Seq_Time::ConvertTicksToNextMs(OSTART time,double sec,bool forceup)
{
	double h=sec*mainaudio->GetGlobalSampleRate();

	LONGLONG msstep=(LONGLONG)h;
	LONGLONG tsamples=ConvertTicksToTempoSamples(time);

	if(forceup==true)
		return ConvertSamplesToTicks(tsamples+msstep);

	LONGLONG c=tsamples/msstep;
	LONGLONG qsamples=c*msstep;

	if(tsamples==qsamples)
		return time;

	return ConvertSamplesToTicks(qsamples+msstep);
}

OSTART Seq_Time::ConvertTicksToNextMeasureTicks(OSTART time)
{
	Seq_Signature *sig=FindSignatureBefore(time);

	OSTART mul=(time-sig->GetSignatureStart())/sig->measurelength;

	if(time-(mul*sig->measurelength+sig->ostart)) // rest ?
		mul++;

	return sig->GetSignatureStart()+mul*sig->measurelength;
}

OSTART Seq_Time::ConvertTicksToNextBeatTicks(OSTART time)
{
	Seq_Signature *sig=FindSignatureBefore(time);
	OSTART mul=(time-sig->GetSignatureStart())/sig->dn_ticks;

	if(time-(mul*sig->dn_ticks+sig->ostart)) // rest ?
		mul++;

	return sig->GetSignatureStart()+mul*sig->dn_ticks;
}

OSTART Seq_Time::ConvertTicksToMeasureTicks(OSTART time,bool up)
{
	Seq_Signature *sig=FindSignatureBefore(time);
	OSTART h=time-sig->GetSignatureStart();	

	h/=sig->measurelength;

	//	if(time-(h*sig->measurelength+sig->ostart) && up==true) // rest ?
	//		h+=1;


	h=sig->GetSignatureStart()+h*sig->measurelength;

	if(up==true)
		return time==h?h:h+sig->measurelength;

	return h;
}

OSTART Seq_Time::ConvertTicksToNextZoomTicks(OSTART time,OSTART zticks)
{
	OSTART h=time/zticks;

	h*=zticks;

	if(h==time)
		return time;

	return h+zticks;
}

OSTART Seq_Time::ConvertTicksToFrameTicks(OSTART time)
{
	Seq_Pos pos(song->project->standardsmpte);
	pos.song=song;
	pos.offset=0;

	ConvertTicksToPos(time,&pos);

	if(pos.pos[4]<2)
	{
		pos.pos[4]=0;
	}
	else
	{
		pos.pos[4]=0;
		pos.AddFrame(1);
	}

	return ConvertPosToTicks(&pos);
}

OSTART Seq_Time::ConvertTicksToQFrameTicks(OSTART time)
{
	Seq_Pos pos(song->project->standardsmpte);
	pos.song=song;
	pos.offset=0;

	ConvertTicksToPos(time,&pos);

	return ConvertPosToTicks(&pos);
}

OSTART Seq_Time::ConvertTicksLeftQuantizeTicks(OSTART time,OSTART qticks)
{
	return mainvar->SimpleQuantizeLeft(time,qticks);
}

OSTART Seq_Time::ConvertTicksQuantizeTicks(OSTART time,OSTART qticks)
{
	Seq_Signature *sig=FindSignatureBefore(time);
	return sig->GetSignatureStart()+mainvar->SimpleQuantize(time-sig->GetSignatureStart(),qticks);
}

OSTART Seq_Time::ConvertTicksToBeatTicks(OSTART time,bool up)
{
	Seq_Signature *sig=FindSignatureBefore(time);

	if(up==true)
		return sig->dn_ticks+sig->GetSignatureStart()+mainvar->SimpleQuantize(time-sig->GetSignatureStart(),sig->dn_ticks);

	return sig->GetSignatureStart()+mainvar->SimpleQuantize(time-sig->GetSignatureStart(),sig->dn_ticks);
}

bool Seq_Pos::PositionChanged()
{
	// Check QFrames
	if(IsSmpte()==true)
		{
			if(pos[4]>3)
			{
				pos[3]++;
				pos[4]=0;
			}
			else
				if(pos[4]<0)
				{
					pos[3]--;
					pos[4]=3;
				}
	}

	// Check Frames
	switch(mode)
	{
	case POSMODE_SMPTE_239:
	case POSMODE_SMPTE_24:
		{
			if(pos[3]>23)
			{
				pos[2]++;
				pos[3]=0;
			}
			else
				if(pos[3]<0)
				{
					pos[2]--;
					pos[3]=23;
				}	
		}
		break;

	case POSMODE_SMPTE_249:
	case POSMODE_SMPTE_25:
		{
			if(pos[3]>24)
			{
				pos[2]++;
				pos[3]=0;
			}
			else
				if(pos[3]<0)
				{
					pos[2]--;
					pos[3]=24;
				}	
		}
		break;

	case POSMODE_SMPTE_2997df:
	case POSMODE_SMPTE_30df:
	case POSMODE_SMPTE_2997:
	case POSMODE_SMPTE_30:
		{
			if(pos[3]>29)
			{
				pos[2]++;
				pos[3]=0;
			}
			else
				if(pos[3]<0)
				{
					pos[2]--;
					pos[3]=29;
				}	
		}
		break;

	case POSMODE_SMPTE_599:
	case POSMODE_SMPTE_60:
		{
			if(pos[3]>59)
			{
				pos[2]++;
				pos[3]=0;
			}
			else
				if(pos[3]<0)
				{
					pos[2]--;
					pos[3]=59;
				}	
		}
		break;

		case POSMODE_SMPTE_48:
		if(pos[3]>47)
			{
				pos[2]++;
				pos[3]=0;
			}
			else
				if(pos[3]<0)
				{
					pos[2]--;
					pos[3]=47;
				}	
		break;

	case POSMODE_SMPTE_50:
		if(pos[3]>49)
			{
				pos[2]++;
				pos[3]=0;
			}
			else
				if(pos[3]<0)
				{
					pos[2]--;
					pos[3]=49;
				}	
		break;

	}

	// Check Sec
	if(IsSmpte()==true)
	{
		if(pos[2]>59)
		{
			pos[1]++;
			pos[2]=0;
		}
		else
			if(pos[2]<0)
			{
				pos[1]--;
				pos[2]=59;
			}	

			// Check Min
			if(pos[1]>59)
			{
				pos[0]++;
				pos[1]=0;
			}
			else
				if(pos[1]<0)
				{
					pos[0]--;
					pos[1]=59;
				}	

				// Check HOUR

				if(pos[0]<0)pos[0]=0;
				if(pos[0]<24) // max 24 h
					return true;
	}

	return false;
}