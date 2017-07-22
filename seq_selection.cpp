#include "songmain.h"
#include "object_track.h"
#include "audiofile.h"
#include "gui.h"
#include "object_song.h"
#include "undo_events.h"
#include "editor.h"
#include "audioevent.h"
#include "mempools.h"
#include "objectpattern.h"
#include "editfunctions.h"

bool Seq_SelectionList::CheckIfPatternInList(Seq_Pattern *p)
{
	Seq_SelectedPattern *check=FirstSelectedPattern();

	while(check){
		if(check->pattern==p)return true;
		check=check->NextSelectedPattern();
	}

	return false;
}

Seq_SelectedPattern *Seq_SelectionList::AddPattern(Seq_Pattern *p)
{
	if(Seq_SelectedPattern *np=new Seq_SelectedPattern){
		np->pattern=p;
		np->status_nrpatternevents=!p->mainclonepattern?p->GetCountOfEvents():0;
		pattern.AddEndO(np);

		return np;
	}

	return 0;
}

void Seq_SelectionList::SelectFromTo(Seq_Event *from,Seq_Event *to)
{
	Seq_SelectionEvent *fe=FindEvent(from);
	Seq_SelectionEvent *te=FindEvent(to);

	if(fe && te)
	{
		int ixf=fe->GetIndex();
		int ixt=te->GetIndex();

		TRACE ("Select From %d <-> %d\n",ixf,ixt);
		if(ixf<ixt)
		{
			int c=ixt-ixf;

			while(c>=0)
			{
				fe->seqevent->Select();
				fe=fe->NextEvent();
				c--;
			}
		}
		else
		{
			int c=ixf-ixt;

			while(c>=0)
			{
				fe->seqevent->Select();
				fe=fe->PrevEvent();
				c--;
			}
		}
	}
}

bool Seq_SelectionList::CheckMove(MoveO *mo)
{
	if(mo->diff!=0 || mo->index!=0)
		return true;

	Seq_SelectedPattern *p=FirstSelectedPattern();

	while(p)
	{
		OSTART pos=p->pattern->GetPatternStart();
		OSTART qpos=mo->song->QuantizeWithFlag(pos,mo->flag);

		if(qpos!=pos)
			return true;

		p=p->NextSelectedPattern();
	}

	return false;
}

void Seq_SelectionList::ToggleSelection(Seq_Event *e)
{
	SelectEvent(e,e->IsSelected()==true?false:true);
}

bool Seq_SelectionList::SelectEvent(Seq_Event *e,bool select)
{
	if(e)
	{
		//lastselectedevent=e;

		if(	(select==true && ((e->flag&OFLAG_SELECTED)==0)) ||
			(select==false && (e->flag&OFLAG_SELECTED))
			)
		{
			//MessageBeep(-1);

			//	maingui->OpenEventEditors(song);
			e->SelectEvent(select);

			maingui->RefreshAllEditorsWithEvent(e->GetTrack()->song,0);
			return true;
		}
	}

	return false;
}

void Seq_SelectionList::SelectAllEvents(bool select,UBYTE status,char byte1,bool showgui)
{
	//lastselectedevent=0;

	//	if(showgui==true)maingui->OpenEventEditors(song);

	bool changed=false;

	Seq_SelectionEvent *e=FirstMixEvent();

	while(e){

		if(e->seqevent->GetStatus()==status && e->seqevent->GetByte1()==byte1)
		{
			if(e->seqevent->SelectEvent(select)==true)
				changed=true;
		}

		e=e->NextEvent();
	}

	//	if(showgui==true)maingui->CloseEventEditors(song,NOBUILD_REFRESH);

	if(changed==true)
		maingui->RefreshAllEditorsWithEvent(song,0);
}

void Seq_SelectionList::SelectAllEvents(bool select,bool showgui)
{
	//lastselectedevent=0;

	//if(showgui==true)maingui->OpenEventEditors(song);

	bool changed=false;

	Seq_SelectionEvent *e=FirstMixEvent();

	while(e){
		if(e->seqevent->SelectEvent(select)==true)
			changed=true;

		e=e->NextEvent();
	}

	if(changed==true)
		maingui->RefreshAllEditorsWithEvent(song,0);
}


void Seq_SelectionList::SelectAllEventsNot(Seq_Event *not,bool select,int filter,bool showgui)
{
	//lastselectedevent=0;

	bool changed=false;

	Seq_SelectionEvent *e=FirstMixEvent();

	while(e){
		if(e->seqevent!=not && e->seqevent->SelectEvent(select)==true)
			changed=true;

		e=e->NextEvent();
	}

	if(changed==true)
		maingui->RefreshAllEditorsWithEvent(song,0);
}

bool Seq_SelectionList::SelectPattern(Seq_Pattern *p)
{
	if(p->recordpattern==false) // dont select record media pattern
	{
		if(CheckIfPatternInList(p)==false)
			AddPattern(p);
	}

	return false;
}

bool Seq_SelectionList::UnselectPattern(Seq_Pattern *p)
{
	Seq_SelectedPattern *check=FirstSelectedPattern();

	while(check){
		if(check->pattern==p)break;
		check=check->NextSelectedPattern();
	}

	if(check)
	{			
		//if(lastselectedevent && lastselectedevent->GetPattern()==p)lastselectedevent=0;
		pattern.RemoveO(check);
		return true;
	}

	return false;
}

bool Seq_SelectionList::UnselectTrack(Seq_Track *t)
{
	Seq_Pattern *p=t->FirstPattern(MEDIATYPE_ALL);

	while(p){
		UnselectPattern(p);
		p=p->NextPattern(MEDIATYPE_ALL);
	}

	return true;
}

void Seq_SelectionList::DeleteAllPattern()
{
	pattern.DeleteAllO();
}

void Seq_SelectionList::Delete(bool full)
{
	Seq_SelectedPattern *d=FirstSelectedPattern();

	while(d){

		if(d->trackname)
		{
			delete d->trackname;
			d->trackname=0;
		}

		if(d->patternname)
		{
			delete d->patternname;
			d->patternname=0;
		}

		if(full==true)
		{
			d->pattern->Delete(true);
		}

		d=(Seq_SelectedPattern *)pattern.RemoveO(d);
	}

	if(full==true)
		delete this;
}

void Seq_SelectionList::Clone(Seq_Song *song,Seq_SelectionList *tolist,OSTART diff,int iflag)
{
	tolist->firsttracknumber=-1;

	Seq_SelectedPattern *selp=FirstSelectedPattern();

	while(selp)
	{
		if(Seq_SelectedPattern *nselp=new Seq_SelectedPattern)
		{
			bool ok=true;
			OSTART setdiff=diff;

			if(selp->pattern->mainclonepattern)
			{
				setdiff+=selp->pattern->offset;
			}

			nselp->pattern=selp->pattern->CreateClone(setdiff,iflag); // Clone to new pattern

			if(nselp->pattern)
			{
				nselp->pattern->mute=false;

				if(nselp->pattern->mainclonepattern=selp->pattern->mainclonepattern)
					nselp->pattern->mainclonepatternID=nselp->pattern->mainclonepattern->patternID;

				TRACE ("Clone Sl %d %d\n",nselp->pattern->GetPatternStart(),selp->pattern->GetPatternStart());
			}
			else
				ok=false;

			if(ok==true)
			{
				nselp->tracknumberinsong=song->GetOfTrack(selp->pattern->GetTrack());

				if(tolist->firsttracknumber==-1 || nselp->tracknumberinsong<tolist->firsttracknumber)
					tolist->firsttracknumber=nselp->tracknumberinsong;

				tolist->pattern.AddEndO(nselp);
			}
			else
				delete nselp;

			//TRACE ("Close Sel List Track Number %d\n",nselp->tracknumberinsong);
		}
		else
			break;

		selp=selp->NextSelectedPattern();
	}
}

Seq_SelectedPattern *Seq_SelectionList::MixNextSelection()
{
	Seq_SelectedPattern *foundpattern=0,*p=FirstSelectedPattern();
	OSTART fpos=-1;
	Seq_Event *fevent;

	while(p)
	{
		if( p->nextevent &&
			(!p->pattern->mainpattern) && 
			(
			fpos==-1 || 
			fpos>p->nextevent->GetEventStart() ||
			//((fpos==p->nextevent->GetEventStart()) && fevent->priority<p->nextevent->priority) ||
			(fpos==p->nextevent->GetEventStart() && fevent->GetChannel()>p->nextevent->GetChannel())
			)
			)
		{
			foundpattern=p;
			fevent=p->nextevent;
			fpos=fevent->GetEventStart();
		}

		p=p->NextSelectedPattern();
	}

	return foundpattern;
}

void Seq_SelectedPattern::SetPrevEvent(int filter,int icdtype)
{
	if(prevevent)
	{
		prevevent=prevevent->PrevEvent(); // set to next event

		if(filter!=SEL_ALLEVENTS)
		{
			// Find next Filter Event
			while(prevevent && prevevent->CheckSelectFlag(filter,icdtype)==false)
				prevevent=prevevent->PrevEvent();
		}
	}
}

void Seq_SelectedPattern::SetNextEvent(int filter,int icdtype)
{
	if(nextevent)
	{
		nextevent=nextevent->NextEvent(); // set to next event

		if(filter!=SEL_ALLEVENTS)
		{
			// Find next Filter Event
			while(nextevent && nextevent->CheckSelectFlag(filter,icdtype)==false)
				nextevent=nextevent->NextEvent();
		}
	}
}

Seq_SelectedPattern* Seq_SelectionList::FindSelectedPattern(Seq_Pattern *patt)
{
	Seq_SelectedPattern *c=FirstSelectedPattern();
	
	while(c && c->pattern!=patt)
		c=c->NextSelectedPattern();
	
	return c;
}

Seq_SelectedPattern* Seq_SelectionList::FindPattern(Seq_Pattern *patt)
{
	Seq_SelectedPattern *c=FirstSelectedPattern();

	while(c && c->pattern!=patt)
		c=c->NextSelectedPattern();
	
	return c;
}

OSTART Seq_SelectionList::FirstPatternPosition()
{
	OSTART pos=-1;

	Seq_SelectedPattern *p=FirstSelectedPattern();
	while(p)
	{	
		if(pos==-1 || p->pattern->GetPatternStart()<pos) // < 0 ?
			pos=p->pattern->GetPatternStart();

		p=p->NextSelectedPattern();
	}

	return pos;
}

int Seq_SelectionList::FirstTrackNumber()
{
	int pos=-1;

	Seq_SelectedPattern *p=FirstSelectedPattern();
	while(p)
	{	
		if(pos==-1 || p->pattern->GetTrack()->song->GetOfTrack(p->pattern->GetTrack())<pos) // < 0 ?
			pos=p->pattern->GetTrack()->song->GetOfTrack(p->pattern->GetTrack());

		p=p->NextSelectedPattern();
	}

	return pos;
}

int Seq_SelectionList::LastTrackNumber()
{
	int pos=-1;

	Seq_SelectedPattern *p=FirstSelectedPattern();
	while(p)
	{	
		if(pos==-1 || p->pattern->GetTrack()->song->GetOfTrack(p->pattern->GetTrack())>pos) // > 0 ?
			pos=p->pattern->GetTrack()->song->GetOfTrack(p->pattern->GetTrack());

		p=p->NextSelectedPattern();
	}

	return pos;
}

Seq_SelectionEvent *Seq_SelectionList::FirstSelectedEvent()
{
	Seq_SelectionEvent *e=FirstMixEvent();

	while(e){
		if(e->seqevent->IsSelected()==true)return e;
		e=e->NextEvent();
	}

	return 0;
}

Seq_SelectionEvent *Seq_SelectionList::FirstSelectedEvent(Seq_Event *startevent)
{
	Seq_SelectionEvent *e=FirstMixEvent();

	while(e)
	{
		if(e->seqevent->IsSelected()==true && e->seqevent->GetStatus()==startevent->GetStatus())
			return e;

		e=e->NextEvent();
	}

	return 0;
}


UndoEdit *Seq_SelectionList::CreateEditEvents(Seq_Event *startevent,int tab) // Selected Events+Same as StartEvent
{
	// Buffer Events+Data
	if(int c=GetCountofSelectedEvents(startevent))
	{
		if(UndoEditEvents *uee=new UndoEditEvents())
		{
			if(uee->Init(c)==true)
			{
				// Reset Edit Flag
				{
					Seq_SelectionEvent *e=FirstMixEvent();

					while(e)
					{
						e->seqevent->flag CLEARBIT EVENTFLAG_UNDEREDIT;
						e=e->NextEvent();
					}
				}

				{
					int i=0;
					Seq_SelectionEvent *fse=FirstSelectedEvent(startevent);

					while(fse){

						fse->seqevent->flag|=EVENTFLAG_UNDEREDIT;

						uee->event_p[i]=fse->seqevent;
						uee->oldindex[i]=fse->seqevent->GetIndex();
						uee->oldevents[i++]=(Seq_Event *)fse->seqevent->Clone(song);

						fse=fse->NextSelectedEvent(startevent);
					}
				}

				edittab=tab;

				return uee;
			}

			delete uee;
		}
	}

	return 0;
}


int Seq_SelectionList::GetOfRealPattern(Seq_Pattern *p)
{
	int c=0;
	Seq_SelectedPattern *sel=FirstSelectedPattern();

	while(sel){

		if(!sel->pattern->mainpattern){

			if(sel->pattern==p)return c;
			c++;
		}

		sel=sel->NextSelectedPattern();
	}

	return c;
}

int Seq_SelectionList::GetCountOfRealSelectedPattern()
{
	int c=0;
	Seq_SelectedPattern *sel=FirstSelectedPattern();

	while(sel){

		if(!sel->pattern->mainpattern)c++;
		sel=sel->NextSelectedPattern();
	}

	return c;
}

int Seq_SelectionList::GetCountOfLinkPatternSelectedPattern(PatternLink *pl)
{
	int c=0;
	Seq_SelectedPattern *sel=FirstSelectedPattern();

	while(sel){

		if(pl==sel->pattern->link)c++;
		sel=sel->NextSelectedPattern();
	}

	return c;
}

int Seq_SelectionList::GetCountOfLinkPatternSelectedPattern()
{
	int c=0;
	Seq_SelectedPattern *sel=FirstSelectedPattern();

	while(sel){

		if(PatternLink *l=sel->pattern->link){
			PatternLink_Pattern *pl=l->FirstLinkedPattern();

			while(pl){
				if(!FindSelectedPattern(pl->pattern))c++;
				pl=pl->NextLink();
			}
		}

		sel=sel->NextSelectedPattern();
	}

	return c;
}

int Seq_SelectionList::GetCountofSelectedEvents()
{
	int c=0;
	Seq_SelectionEvent *e=FirstMixEvent();

	while(e){
		if(e->seqevent->IsSelected()==true)c++;
		e=e->NextEvent();
	}

	return c;
}


int Seq_SelectionList::GetCountofSelectedEvents(Seq_Event *startevent)
{
	int c=0;
	Seq_SelectionEvent *e=FirstSelectedEvent(startevent);

	while(e){

		if(e->seqevent->IsSelected()==true && startevent->GetStatus()==e->seqevent->GetStatus())
			c++;

		e=e->NextEvent();
	}

	return c;
}

int Seq_SelectionList::GetCountofSelectedEvents_Filter(int filter,int icdtype)
{
	int c=0;
	Seq_SelectionEvent *e=FirstMixEvent();

	while(e)
	{
		if(e->seqevent->IsSelected()==true && e->seqevent->CheckSelectFlag(filter|OFLAG_SELECTED,icdtype)==true)
			c++;

		e=e->NextEvent();
	}

	return c;
}

Seq_SelectionEvent *Seq_SelectionList::FirstMixEvent(OSTART start)
{
	return (Seq_SelectionEvent *)events.FindObject(start);
}

Seq_Event *Seq_SelectionList::FirstSelectionEvent(OSTART start,int addfilter) // Init Pattern's FirstEvents and return first Event
{
	Seq_SelectedPattern *p=FirstSelectedPattern();

	// Reset Selection
	while(p)
	{
		if(!p->pattern->mainpattern)
			p->nextevent=!p->pattern->mainclonepattern?p->pattern->FindEventAtPosition(start,buildfilter|addfilter,buildicdtype):0;
		else
			p->nextevent=0;

		p=p->NextSelectedPattern();
	}

	if(Seq_SelectedPattern *foundpattern=MixNextSelection())
	{
		if(Seq_Event *e=foundpattern->nextevent)
		{
			foundpattern->SetNextEvent(buildfilter|addfilter,buildicdtype);
			return e;
		}
	}

	return 0;
}

Seq_Event *Seq_SelectionList::NextSelectionEvent(int addfilter)
{	
	if(Seq_SelectedPattern *foundpattern=MixNextSelection())
	{
		Seq_Event *fevent=foundpattern->nextevent;
		foundpattern->SetNextEvent(buildfilter|addfilter,buildicdtype);
		return fevent;
	}

	return 0;
}

Seq_SelectionList::Seq_SelectionList()
{
	id=OBJ_SELECTPATTERN;
	deletethis=false;
	refresh=false;
	patternremoved=false;
	buildfilter=SEL_ALLEVENTS;
	buildicdtype=0;
	solopattern=0;
	openstartposition=0;
	showonlyselectedevents=false;
	playsolo=false;
	cursorevent=0;
	edittab=-1;
	seqlist=0;
}

void Seq_SelectionList::ClearFlags()
{
	Seq_SelectionEvent *c=FirstMixEvent();

	while(c){
		c->ClearMoreEvents();
		c->flag CLEARBIT SEQSEL_ONDISPLAY;
		c=c->NextEvent();
	}
}

void Seq_SelectionList::ClearFlag(int flag)
{
	Seq_SelectionEvent *c=FirstMixEvent();

	while(c)
	{
		c->ClearMoreEvents();
		c->flag CLEARBIT flag;
		c=c->NextEvent();
	}
}

bool Seq_SelectionList::CheckForMediaType(int t)
{
	Seq_SelectedPattern *p=FirstSelectedPattern();

	while(p){
		if(p->pattern->mediatype==t)return true;
		p=p->NextSelectedPattern();
	}

	return false;
}

Seq_SelectionEvent *Seq_SelectionList::FindEvent(Seq_Event *e)
{
	Seq_SelectionEvent *c=FirstMixEvent();

	while(c){
		if(c->seqevent==e)return c;
		c=c->NextEvent();
	}

	return 0;
}

void Seq_SelectionList::CopyStatus()
{
	Seq_SelectedPattern *p=FirstSelectedPattern();

	while(p)
	{
		p->status_nrpatternevents=p->pattern->GetCountOfEvents();
		p=p->NextSelectedPattern();
	}
}

void Seq_SelectionList::ResetSelection()
{
	Seq_SelectionEvent *sel=FirstMixEvent();

	while(sel)
	{
		sel->seqevent->flag CLEARBIT OFLAG_UNDERSELECTION;
		sel=sel->NextEvent();
	}
}

void Seq_SelectionList::ResetChanges()
{
	Seq_SelectionEvent *sel=FirstMixEvent();

	while(sel)
	{
		sel->changedlength=0;
		//sel->changedposition=0;

		sel=sel->NextEvent();
	}
}

bool Seq_SelectionList::Compare(Seq_SelectionList *comp)
{
	if(comp)
	{
		if(comp->FirstSelectedPattern()==0 && FirstSelectedPattern()==0)return true;
		if(comp->GetCountOfSelectedPattern()!=GetCountOfSelectedPattern())return false;

		Seq_SelectedPattern *in=FirstSelectedPattern();

		while(in){
			if(comp->FindPattern(in->pattern)==0)return false;
			in=in->NextSelectedPattern();
		}

		return true;
	}

	return false;
}

void Seq_SelectionList::ClearEventList()
{
	cursorevent=0;

	if(seqlist)
	{
		delete seqlist;
		seqlist=0;
	}

	events.Clear();
}

Seq_Track *Seq_SelectionList::FindTrack(Seq_Track *track)
{
	Seq_SelectedPattern *s=FirstSelectedPattern();

	while(s){
		if(s->pattern->track==track)return track;
		s=s->NextSelectedPattern();
	}

	return 0;
}

int Seq_SelectionList::GetOfPattern(Seq_Pattern *p)
{
	int c=0;
	Seq_SelectedPattern *s=FirstSelectedPattern();

	while(s){
		if(s->pattern==p)return c;

		if(s->pattern->mediatype==p->mediatype) // same MEDIATYPE ?
			c++;

		s=s->NextSelectedPattern();
	}

	return -1;
}

bool Seq_SelectionList::CheckEventList()
{
	Seq_SelectionEvent *se=FirstMixEvent();

	while(se && se->NextEvent()){
		if(se->NextEvent()->seqevent->GetEventStart()<se->seqevent->GetEventStart())return true;
		se=se->NextEvent();
	}

	return false;
}

int Seq_SelectionList::GetIndexOfEvent(Seq_Event *seqevent)
{
	int index=0;

	Seq_SelectionEvent *se=FirstMixEvent();

	while(se){

		if(se->seqevent==seqevent)
			return index;

		index++;
		se=se->NextEvent();
	}

	return -1;
}

bool Seq_SelectionList::CheckEventList(Seq_Event *e,MIDIFilter *displayfilter)
{
	switch(e->status)
	{
	case AUDIO:
		{
			if(solopattern)
			{
				if(solopattern->mediatype==MEDIATYPE_AUDIO)
				{
					AudioPattern *solo=(AudioPattern *)solopattern,*ap=((AudioEvent *)e)->GetAudioPattern();

					if(solo==ap) // Solo Pattern
						return true;	
				}
			}
			else
				return true; // Audio No Filter
		}
		break;

	default: // MIDI/ICD
		if( ((!displayfilter) || e->CheckFilter(displayfilter)==true) && // Filter
			((!solopattern) || e->pattern==solopattern) // Solo Pattern
			)
			return true;

		break;
	}

	return false;
}

void Seq_SelectionList::BuildEventList(int filter,MIDIFilter *displayfilter,int icdtype)
{
	buildfilter=filter;
	buildicdtype=icdtype;

	int c=0;

	// Reset Start Events
	{
		Seq_Event *e=FirstSelectionEvent(0,0);

		while(e)
		{
			if(CheckEventList(e,displayfilter)==true &&
				(showonlyselectedevents==false || e->IsSelected()==true) // Only Selected
				)
			{
				c++;
				e->flag|=OFLAG_PSELECTION;
			}

			e=NextSelectionEvent(0);
		}
	}

	if(c)
	{
		if((!seqlist) || c!=events.objectsinlist)
			seqlist=new Seq_SelectionEvent[c];

		if(seqlist)
		{
			Seq_SelectionEvent *prev=0;
			int i=0;

			Seq_Event *e=FirstSelectionEvent(0,0);

			while(e)
			{
				if(e->flag&OFLAG_PSELECTION)
				{
					e->flag CLEARBIT OFLAG_PSELECTION;

					seqlist[i].prev=prev;
					seqlist[i].next=0;

					seqlist[i].ostart=e->GetEventStart();
					seqlist[i].seqevent=e;

					if(prev)
						prev->next=&seqlist[i];

					prev=&seqlist[i];

					i++;
				}

				e=NextSelectionEvent(0);
			}

			events.c_root=&seqlist[0];
			events.c_end=&seqlist[c-1];

			events.objectsinlist=c;
			events.accesscounter++;
		}
		else
		{
			Seq_Event *e=FirstSelectionEvent(0,0);

			while(e)
			{
				if(e->flag&OFLAG_PSELECTION) // Only Selected
					e->flag CLEARBIT OFLAG_PSELECTION;

				e=NextSelectionEvent(0);
			}
		}

	}
	else 
		ClearEventList();

	events.Close();
}

Seq_SelectionEvent *Seq_SelectionEvent::NextEventOnDisplay()
{
	Seq_SelectionEvent *c=NextEvent();

	while(c){
		if(c->flag&SEQSEL_ONDISPLAY)return c;
		c=c->NextEvent();
	}

	return 0;
}

Seq_SelectionEvent *Seq_SelectionEvent::NextSelectedEvent()
{
	Seq_SelectionEvent *c=NextEvent();

	while(c){
		if(c->seqevent->IsSelected()==true)return c;
		c=c->NextEvent();
	}

	return 0;
}

Seq_SelectionEvent *Seq_SelectionEvent::NextSelectedEvent(Seq_Event *startevent)
{
	Seq_SelectionEvent *c=NextEvent();

	while(c){
		if(c->seqevent->IsSelected()==true && c->seqevent->GetStatus()==startevent->GetStatus())return c;
		c=c->NextEvent();
	}

	return 0;
}