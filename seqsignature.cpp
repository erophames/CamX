#include "defines.h"
#include "songmain.h"
#include "object_song.h"
#include "object.h"
#include "seqtime.h"
#include "camxfile.h"
#include "semapores.h"

void Seq_Time::AddeptSignaturesChangesToPositions()
{
	if(song)
	{
		OSTART cs=song->timetrack.ConvertMeasureToTicks(song->playbacksettings.cyclestart_measure);
		OSTART ce=song->timetrack.ConvertMeasureToTicks(song->playbacksettings.cycleend_measure);

		if((cs!=song->playbacksettings.cyclestart) || (ce!=song->playbacksettings.cycleend))
		{
			song->SetCycle(cs,ce);
		}
	}
}

void Seq_Time::RefreshSignatureMeasures()
{
	Seq_Signature *sig=FirstSignature()->NextSignature(); // 1. Signature always Measure 1, Tick 0

	while(sig){
		sig->ostart=sig->PrevSignature()->ostart+sig->PrevSignature()->measurelength*(sig->sig_measure-sig->PrevSignature()->sig_measure);
		sig=sig->NextSignature();
	}
}

void Seq_Signature::SetDNValue()
{
	// Calc Denumerator
	if(dn_ticks>=TICK1nd)
		dn=1;
	else
		if(dn_ticks>=TICK2nd)
			dn=2;
		else
			if(dn_ticks>=TICK4nd)
				dn=4;
			else
				if(dn_ticks>=TICK8nd)
					dn=8;
				else
					if(dn_ticks>=TICK16nd)
						dn=16;
					else
						if(dn_ticks>=TICK32nd)
							dn=32;
						else
							if(dn_ticks>=TICK64nd)
								dn=64;
							else
								dn=128;
}

void Seq_Signature::SetSignature(int newnn,OSTART newdn_ticks,bool lock)
{
	if(lock==true)
		mainthreadcontrol->LockActiveSong();

	nn=newnn;
	dn_ticks=newdn_ticks;

	SetDNValue();

	measurelength=nn*dn_ticks;

	if(map)
	{
		map->RefreshSignatureMeasures();
		map->AddeptSignaturesChangesToPositions();
	}

	if(lock==true)
		mainthreadcontrol->UnlockActiveSong();
}

bool Seq_Signature::Compare(Seq_Signature *c)
{
	if(c->dn_ticks !=dn_ticks || c->nn !=nn)
		return false;

	return true;
}

void Seq_Signature::CopyData(Seq_Signature *to)
{
	to->dn_ticks=dn_ticks;
	to->nn=nn;

	to->SetSignature(nn,dn_ticks,false);
}

void Seq_Signature::Load(camxFile *file)
{
	file->ReadChunk(&ostart);

	file->ReadChunk(&nn);
	file->ReadChunk(&dn_ticks);
	file->ReadChunk(&sig_measure);

	SetSignature(nn,dn_ticks,false);
}

void Seq_Signature::Save(camxFile *file)
{
	file->Save_Chunk(ostart);
	file->Save_Chunk(nn);
	file->Save_Chunk(dn_ticks);
	file->Save_Chunk(sig_measure);
}

char *Seq_Signature::GetTickString()
{	
	switch(dn_ticks)
	{
	case TICK1nd:
		return "1";

	case TICK2nd:
		return "2";

	case TICK4nd:
		return "4";

	case TICK8nd:
		return "8";

	case TICK16nd:
		return "16";

	case TICK32nd:
		return "32";
	}

	return 0;
}

Seq_Signature *Seq_Time::FindSignature_Measure(int measure)
{
	Seq_Signature *c=FirstSignature();

	while(c)
	{
		if(c->sig_measure==measure)
			return c;

		c=c->NextSignature();
	}

	return 0;
}


Seq_Signature * Seq_Time::AddNewSignature(int measure,int new_nn,OSTART new_dntx)
{
	if(measure>0)
	{
		Seq_Signature *f=FindSignature_Measure(measure);

		if(!f) // Signature exists ?
		{
			if(new_nn>=2)
			{
				switch(new_dntx)
				{
				//case TICK1nd:
				case TICK2nd:
				case TICK4nd:
				case TICK8nd:
				case TICK16nd:
					{
						if(Seq_Signature *e = new Seq_Signature) // dn=0=1/1,1=1/2,2=1/4,3=1/8
						{
							e->sig_measure=measure;

							e->nn=new_nn;
							e->dn_ticks=new_dntx;
							e->measurelength=new_nn*new_dntx;
							e->map=this;

							e->SetDNValue();

							LockTimeTrack();
							signaturemap.AddOSort(e,ConvertMeasureToTicks(measure));

							RefreshSignatureMeasures();
							UnlockTimeTrack();

							return e;
						}

					}
					break;
				}
			}
		}
	}

	return 0;
}

void Seq_Time::SetNumerator(OSTART pos,int num)
{
	Seq_Signature *sig=FindSignatureBefore(pos);

	if(sig && sig->nn!=num)
	{
		sig->SetSignature(num,sig->dn_ticks,true);
	}
}

void Seq_Time::SetNumerator(Seq_Signature *sig,int num)
{
	if(sig && sig->nn!=num)
		sig->SetSignature(num,sig->dn_ticks,true);
}

void Seq_Time::SetDeNumerator(OSTART pos,OSTART dnum)
{
	Seq_Signature *sig=FindSignatureBefore(pos);

	if(sig && sig->dn_ticks!=dnum)
	{
		sig->SetSignature(sig->nn,dnum,true);
	}
}

void Seq_Time::SetDeNumerator(Seq_Signature *sig,OSTART dnum)
{
	if(sig && sig->dn_ticks!=dnum)
		sig->SetSignature(sig->nn,dnum,true);
}

void Seq_Signature::ChangeSignature(int new_nn,OSTART new_dntx)
{
	if(new_nn>=2 && (new_nn!=nn || new_dntx!=dn_ticks))
	{
		switch(new_dntx)
		{
		case TICK2nd:
		case TICK4nd:
		case TICK8nd:
			SetSignature(new_nn,new_dntx,true);			
			break;
		}
	}
}

Seq_Signature *Seq_Time::RemoveSignature(Seq_Signature *t)
{
	return (Seq_Signature *)signaturemap.RemoveO(t);
}

void Seq_Time::RemoveSignatureMap(bool full)
{
	Seq_Signature *t=FirstSignature(); // Remove Signaturemap

	// dont remove firt signature
	if(full==false && t)
	{
		//default signature 4/4
		t->SetSignature(4,TICK4nd,false);

		t=t->NextSignature();
	}

	while(t)
		t=RemoveSignature(t);
}
