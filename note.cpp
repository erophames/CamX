#include "defines.h"

// #include "mempools.h"
#include "objectevent.h"

void Note::DeleteFromPattern()
{
	pattern->events.DeleteObject(this); // Remove from Event list
	pattern->DeleteVirtual(&off);
}

void Note::AddSortToPattern(MidiPattern *p,long start)
{
	pattern=p;
	
	QuantizeEffect *fx=GetQuantizer();
	
	off.pattern=p;
	off.objectstart=off.staticstartposition;
	
	objectstart=start;
	
	if(fx)
		fx->QuantizeNote(this);
	
	p->AddSortEvent(this,objectstart);
	p->AddSortVirtual(&off,off.objectstart);
}

void Note::AddSortToPattern(MidiPattern *p)
{
	p->AddSortEvent(this,objectstart);
	p->AddSortVirtual(&off,off.objectstart);
}

Seq_Event *Note::CloneNew()
{
	Note *n=mainpools.mempGetNote();
	
	if(n)
		CloneData(n);
	
	return n;
}

FLAG Note::QuantizeEvent(QuantizeEffect *fx)
{
	qflag=fx->QuantizeNote(this);
}

void Note::SetStatic()
{
	if(objectstart!=staticstartposition)
		list->MoveObject(this,staticstartposition);
	
	if(off.staticstartposition!=off.objectstart)
		list->MoveObject(&off,off.staticstartposition);
}

Seq_Event *Note::CloneNewAndSortToPattern(MidiPattern *p)
{
	Note *newnote=(Note *)CloneNew();
	
	if(newnote)
	{
		p->AddSortEvent(newnote);
		p->AddSortVirtual(&newnote->off);
	}
	
	return newnote;
}

Seq_Event *Note::CloneNewAndSortToPattern(MidiPattern *p,long diff)
{
	Note *newnote=(Note *)CloneNew();
	
	if(newnote)
	{
		newnote->objectstart+=diff;
		newnote->off.objectstart+=diff;
		
		newnote->staticstartposition+=diff;
		newnote->off.staticstartposition+=diff;
		
		p->AddSortEvent(newnote);
		p->AddSortVirtual(&newnote->off);
	}
	
	return newnote;
}
