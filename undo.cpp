#include "defines.h"
#include "undo.h"
#include "objectpattern.h"
#include "object_track.h"
#include "objectevent.h"
#include "object_song.h"
#include "semapores.h"
#include "songmain.h"
#include "MIDIeffects.h"
#include "gui.h"
#include "editfunctions.h"
#include "audiofile.h"
#include "audiohdfile.h"
#include "audiohardware.h"
#include "settings.h"
#include "crossfade.h"
#include "audiopattern.h"

void UndoFunction::DoUndoCrossFades(OList *crossfades)
{
	Seq_Track *t=song->FirstTrack();
	while(t)
	{
		Seq_Pattern *p=t->FirstPattern();
		while(p)
		{
			p->MarkAllCrossFades();
			p=p->NextPattern();
		}

		t=t->NextTrack();
	}

	Seq_CrossFade *cf=(Seq_CrossFade *)crossfades->GetRoot();
	while(cf)
	{
		if(cf->infade==false)
		{
			cf->patterninsong=song->FindPattern(cf->pattern);

			if(cf->patterninsong==true)
			{
				cf->connectwith->patterninsong=song->FindPattern(cf->connectwith->pattern);

				if(cf->connectwith->patterninsong==true)
				{
					cf->deletethis=false; // Temp Flag

					if(Seq_CrossFade *check=cf->pattern->FindCrossFade(cf->connectwith->pattern))
					{
						cf->deletethis=true;
						check->deletethis=check->connectwith->deletethis=false;

						check->CopyData(cf);
						check->connectwith->CopyData(cf->connectwith);
					}
				}
			}
		}

		cf=cf->NextCrossFade();
	}

	t=song->FirstTrack();
	while(t)
	{
		Seq_Pattern *p=t->FirstPattern();
		while(p)
		{
			p->DeleteMarkedCrossFades();
			p=p->NextPattern();
		}

		t=t->NextTrack();
	}

	cf=(Seq_CrossFade *)crossfades->GetRoot();
	while(cf)
	{
		if(cf->infade==false && cf->patterninsong==true && cf->connectwith->patterninsong==true && cf->deletethis==false)
		{
			Seq_CrossFade *cf1=(Seq_CrossFade *)cf->Clone(),*cf2=(Seq_CrossFade *)cf->connectwith->Clone();

			if(cf1 && cf2)
			{
				cf1->connectwith=cf2;
				cf2->connectwith=cf1;

				cf->pattern->AddCrossFade(cf1);
				cf2->pattern->AddCrossFade(cf2);
			//	cf->pattern->crossfades.AddEndO(cf1);
			//	cf2->pattern->crossfades.AddEndO(cf2);
			}
		}

		cf->deletethis=false; // Reset Temp Flag
		cf=cf->NextCrossFade();
	}

}

void UndoFunction::CreateCrossFadeBackUp(OList *tolist)
{
	Seq_Track *t=song->FirstTrack();
	while(t)
	{
		Seq_Pattern *p=t->FirstPattern();
		while(p)
		{
			Seq_CrossFade *cf=p->FirstCrossFade();
			while(cf)
			{
				if(cf->infade==false)
				{
					bool add=true;

					// Inside ?
					Seq_CrossFade *check=(Seq_CrossFade *)tolist->GetRoot();
					while(check)
					{
						if(check->infade==false)
						{
							if(check->pattern==cf->pattern && check->connectwith->pattern==cf->connectwith->pattern)
							{
								cf->CopyData(check);
								cf->connectwith->CopyData(check->connectwith);
								add=false;
								break;
							}
						}

						check=check->NextCrossFade();
					}

					if(add==true)
					{
						// Add
						if(Seq_CrossFade *clone=(Seq_CrossFade *)cf->Clone())
						{
							tolist->AddEndO(clone);

							if(Seq_CrossFade *cloneconnect=(Seq_CrossFade *)cf->connectwith->Clone())
							{
								cloneconnect->connectwith=clone;
								clone->connectwith=cloneconnect;
								tolist->AddEndO(cloneconnect);
							}
						}
					}
				}

				cf=cf->NextCrossFade();
			}

			p=p->NextPattern();
		}

		t=t->NextTrack();
	}
}

void Undo::OpenUndoFunction(UndoFunction *uf,bool addtolastundo)
{
	if(!uf)return;

	uf->song=song;

	if(addtolastundo==true)
	{
		UndoFunction *last=LastUndo();

		if(last && last->id==uf->id) // add ?
		{
#ifdef DEBUG
			if(last==uf)
				maingui->MessageBoxError(0,"OpenUndoFunction");
#endif

			last->AddFunction(uf);
		}
	}
	else
	{
		UndoFunction *f=FirstUndo();
		while(f)
		{
			if(f==uf)return;
			f=f->NextFunction();
		}

		int maxundo=-1; // no limit

		switch(mainsettings->undosteps)
		{
		case 0:
			maxundo=25;
			break;

		case 1:
			maxundo=50;
			break;

		case 2:
			maxundo=75;
			break;

		case 3:
			maxundo=100;
			break;
		}

		if(maxundo!=-1){
			int undonr=undos.GetCount();

#ifdef _DEBUG
			if(undonr==2)
				undonr=maxundo+1;
#endif
			if(undonr>=maxundo)
			{
				TRACE ("Cut Undo \n");

				DeleteUndoFunction(FirstUndo());
			}
		}

		// Disable Redo !
		if(LastRedo())LastRedo()->canredo=false;
		uf->CreateCrossFadeBackUp(&uf->crossfades_undo);
		undos.AddEndO(uf); // always last in undo list
	}

	uf->AddedToUndo();
}

bool Undo::DoRedo() // First -> Last
{
	if(mainedit->CheckIfEditOK(song)==false)
		return false;

	UndoFunction *last=LastRedo();
	bool checkplayback=false;

	if(last && last->canredo==true)
	{
		if(song==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		last->inundo=true;
		last->canredo=false;

		//last->CreateCrossFadeBackUp();

		last->AddSubRedoFunctions();
		
		last->RefreshPreRedo();
		last->DoRedo();
		last->DoEnd();
		
		//3. UndoEnd
		UndoFunction *rf=last->FirstFunction();
		while(rf){
			rf->DoEnd();
			rf->RefreshPreRedo();

			rf=rf->NextFunction();
		}

		song->CreateQTrackList();
		last->DoUndoCrossFades(&last->crossfades_redo);
		song->CheckCrossFades();
		song->SetMuteFlags();

		if(!song->GetFocusTrack())
			song->SetFocusTrack(song->FirstTrack());

		if(song==mainvar->GetActiveSong()){

			song->CheckPlaybackRefresh();
			mainthreadcontrol->UnlockActiveSong();
		}

		mainaudio->CheckPeakFiles();
		last->RefreshGUI(false);
		last->RefreshDo();

		rf=last->FirstFunction();
		while(rf){
			rf->RefreshGUI(false);
			rf->RefreshDo();

			rf=rf->NextFunction();
		}

		redos.MoveOToEndOList(&undos,last); // Add to undolist

		mainedit->CheckEditElementsForGUI(song,last,false);
		maingui->RefreshUndoGUI(song);

		return true;
	}

	return false;
}

bool Undo::DoUndo() // Last <- First
{
	if(mainedit->CheckIfEditOK(song)==false)
		return false;

	if(UndoFunction *last=LastUndo())
	{
		// 1. Remove Objects from GUI
		UndoFunction *uf=last->FirstFunction();
		while(uf){
			uf->UndoGUI();
			uf->RefreshPreUndo();

			uf=uf->NextFunction();
		}

		last->CreateCrossFadeBackUp(&last->crossfades_redo);
		last->UndoGUI();
		last->RefreshPreUndo();

		if(song==mainvar->GetActiveSong()) // Lock active Song
			mainthreadcontrol->LockActiveSong();

		// 2. DoUndo
		last->AddSubUndoFunctions();
		last->DoUndo();

		//3. UndoEnd
		uf=last->FirstFunction();
		while(uf){
			uf->UndoEnd();
			uf=uf->NextFunction();
		}

		last->UndoEnd();

		song->CreateQTrackList();
		last->DoUndoCrossFades(&last->crossfades_undo);
		song->CheckCrossFades();
		song->SetMuteFlags();

		if(!song->GetFocusTrack())
			song->SetFocusTrack(song->FirstTrack());

		if(song==mainvar->GetActiveSong())
		{
			song->CheckPlaybackRefresh(); // Refresh Objects Playback etc..
			mainthreadcontrol->UnlockActiveSong(); // Unlock active Song 
		}

		mainaudio->CheckPeakFiles();

		// Check Edit
		last->RefreshPostUndo();
		last->RefreshGUI(false);

		uf=last->FirstFunction();
		while(uf){

			uf->RefreshPostUndo();
			uf->RefreshGUI(false);
		
			uf=uf->NextFunction();
		}

		last->inundo=false;
		undos.MoveOToEndOList(&redos,last); // Add to redolist
		last->canredo=true;

		mainedit->CheckEditElementsForGUI(song,last,false);
		maingui->RefreshUndoGUI(song); // Menus etc..

		return true;
	}

	return false;
}

void UndoFunction::AddSubUndoFunctions()
{
	UndoFunction *uf=FirstFunction();

	while(uf){
		uf->DoUndo();
		uf=uf->NextFunction();
	}
}

void UndoFunction::AddSubRedoFunctions()
{
	UndoFunction *uf=FirstFunction();

	while(uf){
		uf->DoRedo();
		uf=uf->NextFunction();
	}
}

void UndoFunction::CreateUndoString(char *n)
{
	if(n)
	{
		if(name=new char[strlen(n)+1])
			strcpy(name,n);
	}
}

bool UndoFunction::CheckPattern(Seq_Pattern *p)
{
	if(!p)return false;

	switch(p->mediatype)
	{
	case MEDIATYPE_AUDIO:
		{
			AudioPattern *ap=(AudioPattern *)p;

			if(ap->audioevent.audioefile && ap->audioevent.audioregion) // Region deleted ?
			{
				int ix=ap->audioevent.audioregion->GetIndex();
				if(ix==-1)return false;
			}
		}
		break;
	}

	return true;
}

UndoFunction::UndoFunction()
{
	song=0;
	open=true;
	inundo=true;
	canredo=false;
	doundo=false;
	dead=false;
	parent=0;
	name=0;
	nodo=false;
	//	activatetrack=0;

#ifdef _DEBUG
	n[0]='U';
	n[1]='F';
	n[2]='C';
	n[3]='T';
#endif
}

UndoFunction::~UndoFunction()
{
	if(name)delete name;
	crossfades_undo.DeleteAllO();
	crossfades_redo.DeleteAllO();
}

void UndoFunction::AddFunction(UndoFunction *uf)
{
	if(uf){
		uf->song=song;
		uf->inundo=inundo;
		uf->parent=this;

		morefunctions.AddEndO(uf); // always last in undo list
	}
}

UndoFunction *UndoFunction::DeleteUndoFunction(bool full,bool cutlist)
{
	UndoFunction *p=PrevFunction();

	// Sub Function
	UndoFunction *f=LastFunction();
	while(f)
	{
		f->inundo=inundo;
		f=f->DeleteUndoFunction(true,true); // always delete
	}

	if(full==true)
	{
		if(cutlist==true)
		{
			if(GetList())GetList()->CutQObject(this);
		}

		FreeData();
		delete this;
	}

	return p;
}

/*
bool Undo::CompareWithLastUndo(UndoFunction *uf)
{
	UndoFunction *f=LastUndo();

	if(!f)return false;
	return f->CompareAndChangeParms(uf);
}
*/

void Undo::RefreshUndos()
{
	int deadfunctions=0;

	for(int i=0;i<2;i++)
	{
		UndoFunction *f=i==0?LastUndo():LastRedo();

		while(f)
		{
			bool deletethis=false;

			// Check Sub Undo Functions
			UndoFunction *s=f->FirstFunction();

			while(s && deletethis==false)
			{
				if(s->RefreshUndo()==false)deletethis=true;
				s=s->NextFunction();
			}

			if(deletethis==false && f->RefreshUndo()==false)deletethis=true;

			UndoFunction *prev=f->PrevFunction();

			if(deletethis==true)
			{
				deadfunctions++;
				f->DeleteUndoFunction(true,true);
			}

			f=prev;
		}
	}

	if(deadfunctions)
	{
#ifdef _DEBUG
		maingui->MessageBoxOk(0,"Dead Undo/Redo");
#endif
		maingui->RefreshUndoGUI(song);
	}
}

void Undo::DeleteAllUndos()
{
	UndoFunction *f=LastUndo();
	while(f)f=f->DeleteUndoFunction(true,true);

	// Redos
	f=LastRedo();
	while(f)f=f->DeleteUndoFunction(true,true);
}

char *Undo::GetUndoString()
{	
	UndoFunction *f=LastUndo();
	if(f && f->inundo==true)return f->GetUndoName();
	return 0;
}

char *Undo::GetRedoString()
{	
	UndoFunction *f=LastRedo();

	if(f && f->canredo==true)return f->GetUndoName();
	return 0;
}

void UndoFunction::DeleteTrack(Seq_Track *t)
{
	//if(song->GetFocusTrack()==t)
	//	activatetrack=t;
	
	song->DeleteTrack(t,false);
	//song->undo.AddUTrack(t);
}

void UndoFunction::RefreshTrack(Seq_Track *track)
{
	Seq_Pattern *p=track->FirstPattern();
	while(p){
		p->RefreshAfterPaste();
		p=p->NextPattern();
	}

	Seq_Track *c=track->FirstChildTrack(),*lc=track->LastChildTrack();
	while(c)
	{
		Seq_Pattern *p=c->FirstPattern();
		while(p){
			p->RefreshAfterPaste();
			p=p->NextPattern();
		}

		if(c==lc)break;
		c=c->NextTrack();
	}

	// Add Clones
	AddClones(track);

	c=track->FirstChildTrack();
	lc=track->LastChildTrack();
	while(c)
	{
		AddClones(c);

		if(c==lc)break;
		c=c->NextTrack();
	}

	// Refresh Audio Input
	if(Seq_TrackRecord *tr=track->t_audiofx.FirstTrackRecord())
	{
		while(tr)
		{
			tr->track->t_audiofx.recordtrack=track;
			tr=tr->NextTrackRecord();
		}
	}

	if(track->t_audiofx.recordtrack)
		track->t_audiofx.recordtrack->t_audiofx.AddTrackRecord(track);
}

void UndoFunction::AddClones(Seq_Track *track)
{
	// Add Clones
	Seq_Pattern *p=track->FirstPattern();
	while(p)
	{
		// Clones
		Seq_ClonePattern *cl=p->FirstClone();

		while(cl)
		{
			cl->pattern->track->AddSortPattern(cl->pattern,cl->pattern->ostart);
			cl->insideundo=false;
			cl->pattern->RefreshAfterPaste();

			cl=cl->NextClone();
		}

		p=p->NextPattern();
	}
}