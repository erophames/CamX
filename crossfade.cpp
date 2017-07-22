#include "gui.h"
#include "object_track.h"
#include "object_song.h"
#include "arrangeeditor.h"
#include "audiofile.h"
#include "audiohdfile.h"
#include "crossfade.h"
#include "audiopattern.h"
#include "chunks.h"

#include <math.h>

void Seq_CrossFade::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);

	Seq_CrossFade *olda,*oldb;

	file->AddPointer((CPOINTER)&connectwith);
	file->AddPointer((CPOINTER)&olda);
	file->AddPointer((CPOINTER)&oldb);

	file->ReadChunk(&used);
	file->ReadChunk(&infade);
	file->ReadChunk(&type);
	file->ReadChunk(&from);
	file->ReadChunk(&to);
}

void Seq_CrossFade::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOCROSSFADE);

	Seq_CrossFade *olda=0,*oldb=0;

	file->Save_Chunk((CPOINTER)this);
	file->Save_Chunk((CPOINTER)connectwith);
	file->Save_Chunk((CPOINTER)olda);
	file->Save_Chunk((CPOINTER)oldb);

	file->Save_Chunk(used);
	file->Save_Chunk(infade);
	file->Save_Chunk(type);
	file->Save_Chunk(from);
	file->Save_Chunk(to);

	file->CloseChunk();
}

void Seq_CrossFade::SetCrossFadeSamplePositions()
{
	Seq_Song *song=pattern->track->song;
	AudioPattern *audiopattern=(AudioPattern *)pattern;
	LONGLONG offset=0;

	if(audiopattern->GetUseOffSetRegion()==true)
	{
		offset=audiopattern->GetOffSetStart();
		//to_sample=audiopattern->GetOffSetEnd();
	}
	else
		if(audiopattern->itsaclone==true)
		{
			AudioPattern *mainaudiopattern=(AudioPattern *)audiopattern->mainpattern;

			if(mainaudiopattern->audioevent.audioregion)
			{
				offset=mainaudiopattern->audioevent.audioregion->regionstart; // Start of Region
		//		to_sample=mainaudiopattern->audioevent.audioregion->regionend;
			}
			
		}
		else
		{
			if(audiopattern->audioevent.audioregion)
			{
				offset=audiopattern->audioevent.audioregion->regionstart; // Start of Region
				//to_sample=audiopattern->audioevent.audioregion->regionend;
			}
		}

		from_sample=song->timetrack.ConvertTicksToTempoSamples(from);
		to_sample=song->timetrack.ConvertTicksToTempoSamples(to);

		from_sample_file=offset+song->timetrack.ConvertTicksToTempoSamplesStart(audiopattern->GetPatternStart(),from-audiopattern->GetPatternStart());
		to_sample_file=offset+song->timetrack.ConvertTicksToTempoSamplesStart(audiopattern->GetPatternStart(),to-audiopattern->GetPatternStart());

	//	from_sample+=offset;
	//	to_sample+=offset;

#ifdef DEBUG
		if(from_sample_file<0 || from_sample_file>audiopattern->audioevent.audioefile->samplesperchannel ||
			to_sample_file<0 || to_sample_file>audiopattern->audioevent.audioefile->samplesperchannel)
			maingui->MessageBoxError(0,"SetCrossFadeSamplePositions");
#endif

	/*
		if(infade==true)
		{
			// InFade <<<<<
			to_sample=from_sample+song->timetrack.ConvertTicksToTempoSamples(pattern->GetPatternStart(),to-from);
		}
		else
		{
			// Out Fade >>>>
			from_sample+=song->timetrack.ConvertTicksToTempoSamples(pattern->GetPatternStart(),from-pattern->GetPatternStart());
		}
		*/
}

void Seq_CrossFade::Toggle()
{
	used=used==true?false:true;
	if(connectwith)connectwith->used=used;

	RefreshCrossFadeGUI();
}

void Seq_CrossFade::RefreshCrossFadeGUI()
{
	guiWindow *win=maingui->FirstWindow();

	while(win)
	{
		if(pattern->track->song==win->WindowSong())
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ear=(Edit_Arrange *)win;
					Edit_Arrange_Pattern *eap=ear->FindPattern(pattern),*eap2=ear->FindPattern(connectwith->pattern);

					if(eap || eap)ear->ShowHoriz(true,false,false);
			
				}
				break;
			}
		}

		win=win->NextWindow();
	}
}

void Seq_CrossFade::CopyData(Seq_CrossFade *cf)
{
	cf->type=type;
	cf->infade=infade;
	cf->used=used;

	cf->from=from; // ticks
	cf->to=to; // ticks
	cf->from_sample=from_sample;
	cf->to_sample=to_sample;
	cf->from_sample_file=from_sample_file;
	cf->to_sample_file=to_sample_file;
}

Object *Seq_CrossFade::Clone()
{
	if(Seq_CrossFade *cf=new Seq_CrossFade)
	{
		CopyData(cf);

		cf->pattern=pattern;
		cf->connectwith=connectwith;
	
		return cf;
	}

	return 0;
}

bool Seq_CrossFade::CheckIfInRange(LONGLONG start,LONGLONG end)
{
	if(from_sample<=start && to_sample>start)
		return true;

	if(from_sample>=start && from_sample<end)
		return true;

	return false;
}

bool Seq_CrossFade::CheckIfInRange_File(LONGLONG start,LONGLONG end)
{
	if(from_sample_file<=start && to_sample_file>start)
		return true;

	if(from_sample_file>=start && from_sample_file<end)
		return true;

	return false;
}

bool Seq_CrossFade::ChangeType(int t)
{
	if(t!=type)
	{
		type=t;
		RefreshCrossFadeGUI();
		return true;
	}

	return false;
}

ARES Seq_CrossFade::ConvertToVolume(double x,bool in,int intype)
{
#ifdef DEBUG
	if(x<0 || x>1)
		maingui->MessageBoxError(0,"CF ConvertToVolume");
#endif

	int checktype=intype==-1?type:intype;

	/*
	CFTYPE_SIN1,
	CFTYPE_SIN2,
	CFTYPE_SIN3,
	CFTYPE_LINEAR,
	CFTYPE_CURVE,
	CFTYPE_COS1,
	CFTYPE_COS2,
	CFTYPE_COS3
	*/

	// Type
	if(in==true)
	{
		// In
		switch(checktype)
		{
		case Seq_CrossFade::CFTYPE_SIN1:
				return x*x;

		case Seq_CrossFade::CFTYPE_SIN2:
			return x*x*x;

		case Seq_CrossFade::CFTYPE_SIN3:
			return x*x*x*x*x;

		case Seq_CrossFade::CFTYPE_LINEAR:
			return x;

		case CFTYPE_CURVE:
			return 1-(cos(x*PI)+1)/2;

		case CFTYPE_COS1:
			return sqrt(x);

		case CFTYPE_MAX:
			return 1;

		case CFTYPE_OFF:
			return 0;
		}
	}
	else // Out
		switch(checktype)
	{
		case Seq_CrossFade::CFTYPE_SIN1:
			return 1-x*x;

		case Seq_CrossFade::CFTYPE_SIN2:
			return 1-x*x*x;

		case Seq_CrossFade::CFTYPE_SIN3:
			return 1-x*x*x*x*x;

		case Seq_CrossFade::CFTYPE_LINEAR:
			return 1-x;

		case CFTYPE_CURVE:
			return (cos(x*PI)+1)/2;

		case CFTYPE_COS1:
			return 1-sqrt(x);

		case CFTYPE_MAX:
			return 1;

		case CFTYPE_OFF:
			return 0;
	}

	return 1;
}