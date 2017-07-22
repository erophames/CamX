#include "defines.h"
#include "MIDIhardware.h"
#include "songmain.h"
#include "groove.h"

#include "gui.h"
#include "trackfx.h"
#include "quantizeeditor.h"

#include "MIDIPattern.h"
#include "chunks.h"
#include "object_project.h"
#include "object_song.h"
#include "object_track.h"
#include "semapores.h"

void Groove::ChangeInSongs()
{
	Seq_Project *p=mainvar->FirstProject();

	while(p)
	{
		Seq_Song *s=p->FirstSong();

		while(s)
		{
			bool changed=false;

			// Tracks
			Seq_Track *t=s->FirstTrack();

			while(t)
			{
				if(t->GetFX()->quantizeeffect.groove==this && t->GetFX()->quantizeeffect.usegroove==true)
				{
					mainthreadcontrol->LockActiveSong();

					Seq_Pattern *p=t->FirstPattern();

					while(p)
					{
						if(p->QuantizePattern(&t->GetFX()->quantizeeffect))
							changed=true;

						p=p->NextPattern();
					}

					mainthreadcontrol->UnlockActiveSong();;
				}
				else
				{
					Seq_Pattern *p=t->FirstPattern();

					while(p)
					{
						if(p->quantizeeffect.groove==this && p->quantizeeffect.usegroove==true)
						{
							mainthreadcontrol->LockActiveSong();

							if(p->QuantizePattern(&p->quantizeeffect))
								changed=true;

							mainthreadcontrol->UnlockActiveSong();;
						}

						p=p->NextPattern();
					}
				}

				t=t->NextTrack();
			}

			if(changed==true)
				maingui->RefreshAllEditors(s,0);

			s=s->NextSong();
		}

		p=p->NextProject();
	}
}

void Groove::InitGroove(OSTART r)
{
	GrooveElement *e;
	OSTART gnumber=raster;
	OSTART start=0; // always start with 0

	while(gnumber--)
	{
		if(e=new GrooveElement)
		{
			e->groove=this;

			grooves.AddEndS(e,start);
		}

		start+=tickraster;
	}

	grooveend=start;
}

void Groove::SetName(char *n)
{
	if(n)
	{
		if(groovename)
			delete groovename;

		groovename=mainvar->GenerateString(n);
	}
}

void GrooveElement::Load(camxFile *file)
{
	file->ReadChunk(&initcounter);
	file->ReadChunk(&initdiff);
	file->ReadChunk(&quantdiff);

	file->CloseReadChunk();
}

void GrooveElement::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_GROOVEELEMENT);

	file->Save_Chunk(initcounter);
	file->Save_Chunk(initdiff);
	file->Save_Chunk(quantdiff);

	file->CloseChunk();
}

void Groove::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);

	file->Read_ChunkString(&groovename);
	file->Read_ChunkString(&fullgroovename);

	file->ReadChunk(&tickraster);
	file->ReadChunk(&raster);
	file->ReadChunk(&qrasterid);
	file->ReadChunk(&grooveend);

	file->CloseReadChunk();

	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_GROOVEELEMENTHEADER)
	{
		int nr=0;

		file->ChunkFound();

		file->ReadChunk(&nr);

		file->CloseReadChunk();

		// --- Groove Elements

		while(nr--)
		{
			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_GROOVEELEMENT)
			{
				file->ChunkFound();

				GrooveElement *e=new GrooveElement();

				if(e)
				{
					e->Load(file);

					e->groove=this;
					grooves.AddEndO(e);
				}
			}
		}
	}		
}

void Groove::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_GROOVE);

	file->Save_Chunk((CPOINTER)this);

	file->Save_ChunkString(groovename);
	file->Save_ChunkString(fullgroovename);

	file->Save_Chunk(tickraster);
	file->Save_Chunk(raster);
	file->Save_Chunk(qrasterid);
	file->Save_Chunk(grooveend);

	file->CloseChunk();

	// Groove Elements
	file->OpenChunk(CHUNK_GROOVEELEMENTHEADER);
	file->Save_Chunk(grooves.GetCount());
	file->CloseChunk();

	// --------------------------------------------
	GrooveElement *f=FirstGrooveElement();

	while(f)
	{
		f->Save(file);
		f=f->NextGrooveElement();
	}
}


OSTART Groove::Quantize(OSTART pos)
{
	OSTART mul=pos/grooveend;
	OSTART c;
	OSTART tick2=tickraster/2;

	pos-=mul*grooveend;
	c=pos/tickraster;

	GrooveElement *e=GetGrooveElementAtIndex(c);

#ifdef _DEBUG
	if(e)
	{
#endif		
		if((e->GetGrooveStart()+tick2)<pos)
		{
			e=e->NextGrooveElement();

			if(!e)
				e=FirstGrooveElement();
		}

#ifdef _DEBUG
	}

	if(!e)
	{
		MessageBox(NULL,"Illegal Groove","Error",MB_OK_STYLE);
	}

#endif

	c=(mul*grooveend)+e->GetGrooveStart()+e->quantdiff;

	if(c<0)
		return 0;
	
	return c;
}

void Groove::ConvertMIDIPatternToGroove(MIDIPattern *pattern)
{
	Seq_Event *firstnote=pattern->FindEventAtPosition(pattern->GetPatternStart(),SEL_NOTEON,0);

	if(firstnote)
	{
		size_t sl=strlen(pattern->GetName());

		sl+=5; // + [P2G]

		if(groovename)
			delete groovename;

		if(groovename=new char[sl+1])
		{
			groovename[0]='[';
			groovename[1]='P';
			groovename[2]='2';
			groovename[3]='G';
			groovename[4]=']';

			strcpy(&groovename[5],pattern->GetName());
		}

		Seq_Event *lastnote=pattern->LastEvent();

		while(lastnote && (lastnote->GetStatus()!=NOTEON))
			lastnote=lastnote->PrevEvent();

		if(lastnote)
		{
			Seq_Event *qe=firstnote;
			GrooveElement *groove;

			OSTART qpoint;
			OSTART qleft,qright;
			OSTART sum;
			int sumc;

			InitGroove(qrasterid);

			OSTART ticks=tickraster;

			groove=FirstGrooveElement();

			qpoint=mainvar->SimpleQuantize(pattern->GetPatternStart(),ticks);

			if(groove) // Init ok ?
			{
				while(groove)
				{
					// Start Point
					qleft=qpoint-(ticks/2);
					qright=qpoint+(ticks/2);

					qe=pattern->FindEventInRange(qleft,qright,SEL_NOTEON); // Find QLeft Note

					if(qe) // Note
					{
						sum=0;
						sumc=0;

						while(qe && qe->GetEventStart()<qright)
						{	
							sum+=qe->GetEventStart()-qpoint;
							sumc++;

							qe=qe->NextEvent();
							while(qe && 
								qe->GetStatus()!=NOTEON && 
								qe->GetEventStart()<qright)
								qe=qe->NextEvent();
						}

						if(sumc)
						{
							sum/=sumc;

							groove->initdiff+=sum;
							groove->initcounter++;
						}
					}

					qpoint+=ticks;

					// Find Note next QPoint
					groove=groove->NextGrooveElement();	// next raster

					if(!groove)
					{
						if(qpoint+grooveend<pattern->LastEvent()->GetEventStart())
							groove=FirstGrooveElement(); // Loop
					}
				}

				// Create Groove Raster
				groove=FirstGrooveElement();

				while(groove)
				{
					if(groove->initcounter)
					{
						groove->quantdiff=groove->initdiff/groove->initcounter;

						groove->initdiff=0;
						groove->initcounter=0;
					}

					groove=groove->NextGrooveElement();
				}
			}

		}
	}
}

GrooveElement *Groove::RemoveGrooveElement(GrooveElement *e)
{
	GrooveElement *n=e->NextGrooveElement();

	grooves.RemoveO(e);

	return n;
}

void mainMIDIBase::RefreshGrooveGUI(Groove *groove)
{
	guiWindow *f=maingui->FirstWindow();

	while(f)
	{
		switch(f->GetEditorID())
		{
		case EDITORTYPE_GROOVE:
			((Edit_Groove *)f)->ShowGrooves();
			break;

		case EDITORTYPE_QUANTIZEEDITOR:
			((Edit_QuantizeEditor *)f)->ShowQuantizeStatus();
			break;
		}

		f=f->NextWindow();
	}
}

Groove::Groove()
{
	groovename=0;
	fullgroovename=0;
}

Groove::Groove(OSTART grooveraster,int qid)
{
	groovename=0;
	fullgroovename=0;

	raster=grooveraster;
	qrasterid=qid;
	tickraster=quantlist[qrasterid];
}

void Groove::InitGrooveName()
{
	char h[256];
	char res[NUMBERSTRINGLEN];

	if(fullgroovename)
	{
		delete fullgroovename;

		fullgroovename=0;
	}

	strcpy(h,quantstr[qrasterid]);
	mainvar->AddString(h," Steps:");
	mainvar->AddString(h,mainvar->ConvertLongLongToChar(raster,res));

	if(groovename)
	{
		mainvar->AddString(h,"-");
		mainvar->AddString(h,groovename);
	}

	fullgroovename=mainvar->GenerateString(h);
}

Groove *mainMIDIBase::AddGroove(Groove *g)
{
	if(g)
	{
		if(g->tickraster>0 && g->qrasterid>=0)
		{
			grooves.AddEndO(g);
		}
		else
		{
#ifdef _DEBUG
			MessageBox(NULL,"Illegal Groove","Error",MB_OK_STYLE);
#endif

			delete g;

			return 0;
		}
	}

	return g;
}

Groove* mainMIDIBase::AddGroove(OSTART rastersteps,int rasterid)
{
	if(rastersteps<=256)
	{
		if(Groove *g=new Groove(rastersteps,rasterid))
		{
			g->SetName("New");
			g->InitGrooveName();

			g=AddGroove(g);

			return g;
		}
	}

	return 0;

}

Groove* mainMIDIBase::RemoveGroove(Groove *g)
{
	// Delete Elements
	GrooveElement *e=g->FirstGrooveElement();

	while(e)
		e=(GrooveElement *)g->grooves.RemoveO(e);

	if(g->groovename)
		delete g->groovename;

	if(g->fullgroovename)
		delete g->fullgroovename;

	return (Groove *)grooves.RemoveO(g);
}

void mainMIDIBase::RemoveAllGrooves()
{
	Groove *g=FirstGroove();

	while(g)
	{
		g=RemoveGroove(g);
	}
}

void mainMIDIBase::DeleteGroove(Groove *g)
{
	mainthreadcontrol->LockActiveSong();

	// Remove Groove from Objects
	Seq_Project *p=mainvar->FirstProject();

	while(p)
	{
		Seq_Song *s=p->FirstSong();
		while(s)
		{
			Seq_Track *t=s->FirstTrack();

			while(t)
			{
				if(t->GetFX()->quantizeeffect.groove==g)
				{
					t->GetFX()->quantizeeffect.groove=0;
					t->GetFX()->quantizeeffect.groovequant=false;
				}

				Seq_Pattern *p=t->FirstPattern();
				while(p)
				{
					if(p->quantizeeffect.groove==g)
					{
						p->quantizeeffect.groove=0;
						p->quantizeeffect.groovequant=false;
					}

					p=p->NextPattern();
				}

				t=t->NextTrack();
			}

			s=s->NextSong();
		}

		p=p->NextProject();
	}

	RemoveGroove(g);

	mainthreadcontrol->UnlockActiveSong();

	/*
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
	switch(w->GetEditorID())
	{
	case EDITORTYPE_ARRANGE:
	{
	EventEditor *e=(EventEditor *)w;

	}
	break;

	}

	w=w->NextWindow();
	}
	*/
}
