#include "defines.h"
#include "object.h"
#include "object_song.h"
#include "gui.h"

LONGLONG OStart::GetSampleStart(Seq_Song *song)
{
	return song->timetrack.ConvertTicksToTempoSamples(ostart);
}

bool OStart::CheckIfInRange(OSTART s,OSTART e)
{
	if(e<s)
	{
		OSTART h=s;
		s=e;
		e=h;
	}

	if(ostart>=s && ostart<=e)
		return true;

	return false;
}


void OList::Reset(int nrnewobjects)
{
	accesscounter++;
	objectsinlist=nrnewobjects;
}

OList::OList()
{
	listtype=OLISTTYPE_LIST;
	parent=0;
	accesscounter=0;

	Clear();
}

void OList::Clear()
{
	accesscounter++;
	c_root=c_end=selectionstart=selectionend=0;
	activeobject=0;
	objectsinlist=0;
}

// newobject->prev=prev
void OList::AddPrevO(Object *newobject,Object *prev)
{
	/*
	// Priority <<<
	while(prev && prev->ostart==newobject->ostart && prev->priority<newobject->priority)
	prev=prev->prev;
	*/
	if(!prev)
	{
		AddStartO(newobject);
		return;
	}

	newobject->SetList(this);
	// Sort
	newobject->prev=prev; // -> Set Next

	if(newobject->next=prev->next) // <- Set Prev
		newobject->next->prev=newobject;
	else
		c_end=newobject;

	prev->next=newobject;

	objectsinlist++;
	accesscounter++;
}

// newobject->prev=prev
void OListStart::AddPrevS(OStart *newobject,OStart *prev,OSTART start)
{	
	if(!prev)
	{
		AddStartS(newobject,start);
		return;
	}

	newobject->SetList(this);
	newobject->ostart=start;

	// Sort
	newobject->prev=prev; // <- Set Prev

	if(newobject->next=prev->next) // Set ->Next
		newobject->next->prev=newobject;
	else
		c_end=newobject;

	prev->next=newobject;

	objectsinlist++;
	accesscounter++;
}

void OList::AddOToIndex(Object *o,int index) // index object->o
{
	if(index==0){
		AddStartO(o);
		return;
	}

	if(index==-1){
		AddEndO(o);
		return;
	}

	if(Object *fo=GetO(index-1)){
		AddPrevO(o,fo);
		return;
	}

	AddEndO(o);
}

void OList::AddStartO(Object *newobject)
{
	newobject->SetList(this);
	newobject->prev=0; // [ <-

	if(!(newobject->next=c_root)) // Set -> Next
	{
		c_root=c_end=newobject; // empty list
		objectsinlist=1;
	}
	else{
		c_root->prev=newobject;
		c_root=newobject;	
		objectsinlist++;
	}

	accesscounter++;
}

void OList::AddEndO(Object *newobject)
{
	newobject->SetList(this);	
	newobject->next=0; // -]

	if(!(newobject->prev=c_end)){ // <- Set Prev
		c_root=c_end=newobject;
		newobject->index=0;
		objectsinlist=1;
	}
	else{
		c_end->next=newobject;
		c_end = newobject;
		newobject->index=objectsinlist++;
	}

	accesscounter++;
}

// newobject->next=next
void OList::AddNextO(Object *newobject,Object *next)
{
	if(!next){
		AddEndO(newobject);
		return;
	}

	newobject->SetList(this);

	// Sort
	newobject->next=next; // -> Set Next

	if(newobject->prev=next->prev) // <- Set Prev
		newobject->prev->next=newobject;
	else
		c_root=newobject;

	next->prev=newobject;

	objectsinlist++;
	accesscounter++;
}

OStart *OListStartIndex::FindOBefore(OSTART start) //<-| Tempo, Signature, <=
{
	if((!GetRoot()) || GetRoot()->ostart>start)
		return 0;

	if(Getc_end()->ostart<=start)
		return (OStart *)c_end;

	OStart *startwith=GetRoot();

	if(index[0])
	{
		for(int i=1;i<OINDEXS;i++)
		{
			if((!index[i]) || index[i]->ostart>start)
			{	
				startwith=index[i-1];
				break;
			}
		}
	}

	OStart *s=startwith;
	do{
		if(s->ostart==start || (!s->Next()) || s->Next()->ostart>start)
			return s;

	}while(s=s->Next());

	return 0;
}

OStart *OListStartIndex::FindObject(OSTART start) // |->
{
#ifdef DEBUG
	if(start<0)
		maingui->MessageBoxError(0,"FindObject <0");
#endif

	if((!GetRoot()) || Getc_end()->ostart<start)
		return 0;

	if(GetRoot()->ostart>=start)
		return GetRoot();

	OStart *startwith=GetRoot();
	if(index[0])
	{
		for(int i=1;i<OINDEXS;i++)
		{
			if((!index[i]) || index[i]->ostart>start)
			{	
				startwith=index[i-1];
				break;
			}
		}
	}
	

	OStart *s=startwith;
	do{
		if(s->ostart>=start)
			return s;

	}while(s=s->Next());

	return 0;
}

OStart *OListStartIndex::FindObjectAtPos(OSTART start) //==
{
	if((!GetRoot()) || Getc_end()->ostart<start)
		return 0;

	if(GetRoot()->ostart>start)
		return 0;

	OStart *startwith=GetRoot();

	if(index[0])
	{
		for(int i=1;i<OINDEXS;i++)
		{
			if((!index[i]) || index[i]->ostart>start)
			{	
				startwith=index[i-1];
				break;
			}
		}
	}

	OStart *s=startwith;
	do{
		if(s->ostart==start)
			return s;

	}while(s=s->Next());

	return 0;
}

void OListStartIndex::ResetIndex()
{
	index[0]=0;
}

void OListStartIndex::EndIndex()
{
	int step=GetCount();

	step/=OINDEXS;

	// Set Steps
	int c=0;
	int s=0;

	OStart *o=GetRoot();
	while(o)
	{
		if(s==0)
		{
			index[c++]=o;
			s=step;
		}
		else
			s--;

		o=o->Next();
	}

#ifdef DEBUG
	if(c>OINDEXS)
		maingui->MessageBoxError(0,"End Index");
#endif

	if(c<OINDEXS)
		index[c]=0;
}

// newobject->next=next
void OListStart::AddNextS(OStart *newobject,OStart *next,OSTART start)
{	
	if(!next){
		AddEndS(newobject,start);
		return;
	}

	newobject->ostart=start;
	newobject->SetList(this);

	// Sort
	newobject->next=next; // -> Set Next

	if(newobject->prev=next->prev) // <- Set Prev
		newobject->prev->next=newobject;
	else
		c_root=newobject;

	next->prev=newobject;

	objectsinlist++;
	accesscounter++;
}

Object *OList::RemoveO(Object *object) // Cut + Delete
{
	Object *n=object->next;

	objectsinlist--;
	accesscounter++;

	if(!object->prev){
		if(c_root=n)
			n->prev=0;
	}
	else
		object->prev->next=n;

	if(!n){
		if(c_end=object->prev)
			c_end->next=0;
	}
	else
		n->prev=object->prev;

	//object->ODeInit();

	delete object;

	selectionstart=selectionend=0;

	return n;
}

Object *OList::CutObject(Object *object) // Cut Object from List
{
	Object *n=object->next;

	objectsinlist--;
	accesscounter++;

#ifdef DEBUG
	if(objectsinlist<0)
		maingui->MessageBoxError(0,"CutObject <0");
#endif

	if(!object->prev){
		c_root=n;
		//n->prev=0;
	}
	else
		object->prev->next=n;

	if(!n){
		if(c_end=object->prev)
			c_end->next=0;
	}
	else
		n->prev=object->prev;

	object->prev=object->next=0;
	selectionstart=selectionend=0;

	return n;
}

void OList::CutQObject(Object *object) // Cut Object from List
{
#ifdef DEBUG
	if(object==0)
		maingui->MessageBoxError(0,"CutQObject Object 0");

	if(object->olist!=this)
		maingui->MessageBoxError(0,"CutQObject Object!=List");
#endif

	Object *n=object->next;

	objectsinlist--;
	accesscounter++;

#ifdef DEBUG
	if(objectsinlist<0)
		maingui->MessageBoxError(0,"CutQObject <0");
#endif

	if(!object->prev){
		c_root=n;
		//n->prev=0;
	}
	else
		object->prev->next=n;

	if(!n){
		if(c_end=object->prev)
			c_end->next=0;
	}
	else
		n->prev=object->prev;

	selectionstart=selectionend=0;
}

#ifdef DEBUG
bool OList::CheckIndex()
{
	int index=0;
	Object *o=c_root;

	while(o)
	{
		if(o->index!=index)
			return false;

		index++;

		o=o->next;
	}

	return true;
}

#endif

void OList::Close()
{
	int ix=0;

	Object *o=c_root;

	while(o)
	{
		o->index=ix++;
		o=o->next;
	}
}

int OList::GetCountSelectedObjects()
{
	int c=0;

	Object *o=c_root;

	while(o)
	{
		if(o->IsSelected()==true)
			c++;

		o=o->next;
	}

	return c;
}

void OList::DeleteAllO()
{
	if(Object *o=c_root)
	{
		do{
			Object *n=o->next; // Buffer Next

			//o->ODeInit();

			delete o;

			o=n;

		}while(o);

		Clear();
	}
}

#ifdef OLDIE
void OList::ReplaceO(Object *old,Object *n)
{
	n->prev=old->prev;
	
	if(!n->prev)
		c_root=n;
	else
		n->prev->next=n;

	n->next=old->next;

	if(!n->next)
		c_end=n;
	else
		n->next->prev=n;

	if(old==selectionstart)
		selectionstart=n;
	else
		if(old==selectionend)
			selectionend=n;
}
#endif

void OListStart::MoveAll(OSTART ticks)
{
	if(!ticks)return;

	if(OStart *m=GetRoot()){
		while(m){
			m->ostart+=ticks;
			m=m->Next();
		}

		accesscounter++;
	}
}

void OList::MoveListToList(OList *list)
{
	{
		Object *o=list->c_root=c_root; 
		while(o)// Init New List
		{
			o->olist=list;
			o=o->next;
		}
	}

	list->c_end=c_end;
	list->objectsinlist=objectsinlist;
	list->accesscounter++;

	Clear();
}

/******************************
// Object List Time -------------------------------------------------------
*//////////////////////////////

void OListStart::AddStartS(OStart *newobject,OSTART start)
{
	newobject->ostart=start;
	newobject->SetList(this);
	newobject->prev=0;

	if(!(newobject->next=c_root))
		c_root=c_end=newobject; // empty list
	else
	{
		c_root->prev=newobject;
		c_root=newobject;
	}

	objectsinlist++;
	accesscounter++;
}

void OListStart::AddEndS(OStart *newobject,OSTART start)
{
	newobject->ostart=start;
	newobject->SetList(this);
	newobject->next=0; // -]

	if(!(newobject->prev=c_end)) // <- Set Prev
		c_root=c_end=newobject;
	else
	{
		c_end->next=newobject;
		c_end = newobject;
	}

	objectsinlist++;
	accesscounter++;
}

void OList::MoveO(Object *o,int diff)
{
	int ix=o->GetIndex();
	ix+=diff;
	CutQObject(o);
	AddOToIndex(o,ix);
}

Object *OList::GetO(int ix)
{
	if(ix<0)return 0;

	Object *f=c_root;

	while(f){
		if(!ix)return f;
		ix--;
		f=f->next;
	}

	return 0;
}

int FolderList::GetIx(FolderObject *object)
{
	if(!object)
		return -1;

	int index=0;
	FolderObject *f=FirstFolderObject();
	while(f)
	{
		if(object==f)
			return index;

		index++;
		f=f->NextFolderObject();
	}

	return -1;
}

bool FolderList::CheckIfSelectedCanBeMovedTo(FolderObject *to)
{
	if(!to)return false;

	int c=0;

	FolderObject *f=FirstFolderObject();
	while(f)
	{
		if( (f->flag&OFLAG_SELECTED) && f!=to && f->parent!=to)
		{
			bool parentselected=false;

			FolderObject *p=f->parent;
			while(p)
			{
				if(p->IsSelected()==true)
				{
					parentselected=true;
					break;
				}

				p=p->parent;
			}

			if(parentselected==false)
			{
				if(f->childdepth+to->childdepth>MAX_CHILDS)
					return false;

				c++;
			}
		}

		f=f->NextFolderObject();
	}

	return c>0?true:false;
}

void FolderList::Close()
{
	int ix=0;

	FolderObject *f=FirstFolderObject();
	while(f)
	{
		f->childs.Close();
		f->index=ix++;

		{
			FolderObject *fo=f->FirstChild();
			while(fo)
			{
				fo->Close();
				fo=fo->NextChild();
			}
		}

		f=f->NextFolder();
	}
}

void FolderList::RepairChilds()
{
	FolderObject *f=FirstFolderObject();
	while(f)
	{
		if(f->parent)
			f->childdepth=f->parent->childdepth+1;
		else
			f->childdepth=0;

		f=f->NextFolderObject();
	}
}

void FolderList::ReleaseSelectedChildObjects(OOList *l,FolderList *tolist)
{
	for(int i=0;i<l->number;i++)
	{
		FolderObject *f=(FolderObject *)l->objects[i];

		if(f->IsSelected()==true && f->olist!=tolist)
		{
			bool parentselected=false;

			FolderObject *p=f->parent;
			while(p)
			{
				if(p->IsSelected()==true)
				{
					parentselected=true;
					break;
				}

				p=p->parent;
			}

			if(parentselected==false)
			{
				f->olist->CutQObject(f);
				f->parent=0;
				tolist->AddEndO(f);
			}
		}
	}

	RepairChilds();
}

void FolderList::MoveObjectsToObject(OOList *l,FolderObject *to)
{
	if(!to)return;

	for(int i=0;i<l->number;i++)
	{
		FolderObject *f=(FolderObject *)l->objects[i];

#ifdef DEBUG
		if(!f)
			maingui->MessageBoxError(0,"MoveSelectedObjectsToObject =0");
#endif

		if(f!=to && f->parent!=to)
		{
			f->olist->CutQObject(f);

			f->parent=to;
			f->childdepth=to->childdepth+1;

			to->childs.AddEndO(f);
		}
	}

	RepairChilds();
}

void FolderList::MoveSelectedObjectsToObject(OOList *l,FolderObject *to)
{
	if(!to)return;

	for(int i=0;i<l->number;i++)
	{
		FolderObject *f=(FolderObject *)l->objects[i];

#ifdef DEBUG
		if(!f)
			maingui->MessageBoxError(0,"MoveSelectedObjectsToObject =0");
#endif

		if(f->IsSelected()==true && f!=to && f->parent!=to)
		{
			bool parentselected=false;

			FolderObject *p=f->parent;
			while(p)
			{
				if(p->IsSelected()==true)
				{
					parentselected=true;
					break;
				}

				p=p->parent;
			}

			if(parentselected==false)
			{
				f->olist->CutQObject(f);

				f->parent=to;
				f->childdepth=to->childdepth+1;

				to->childs.AddEndO(f);
			}
		}
	}

	RepairChilds();
}

void FolderList::MoveObjects_Undo(OOList *list)
{
	//  TRACE ("MoveObjects_Undo");

	for(int i=0;i<list->number;i++)
		list->objects[i]->olist->CutQObject(list->objects[i]);

	for(int i=0;i<list->number;i++)
	{
		FolderObject *f=(FolderObject *)list->objects[i];
		f->parent=(FolderObject *)list->list[i]->parent;
		TRACE ("Undo Parent %d\n",f->parent);
		list->list[i]->AddEndO(f);
	}

	RepairChilds();
}

bool FolderList::CheckIfMoveSelected(int diff)
{
	int okc=0;

	FolderObject *f=FirstFolderObject();

	while(f)
	{
		if(f->IsSelected()==true)
		{
			if(f->CheckIfCanBeMove(diff)==false)
				return false;

			okc++;
		}

		f=f->NextFolderObject();
	}

	return okc>0?true:false;
}

void FolderList::CreateSelectedList(OOList *l)
{
	int c=0;
	FolderObject *fo=FirstFolderObject();

	while(fo){

		if(fo->IsSelected()==true)
		{
			bool parentselected=false;

			FolderObject *p=fo->parent;
			while(p)
			{
				if(p->IsSelected()==true)
				{
					parentselected=true;
					break;
				}

				p=p->parent;
			}

			if(parentselected==false)
				c++;

		}

		fo=fo->NextFolderObject();
	}

	if(l->number!=c)
		l->InitOO(c);

	if(l->objects)
	{
		int c=0;
		FolderObject *fo=FirstFolderObject();

		while(fo){

			if(fo->IsSelected()==true)
			{
				bool parentselected=false;

				FolderObject *p=fo->parent;
				while(p)
				{
					if(p->IsSelected()==true)
					{
						parentselected=true;
						break;
					}

					p=p->parent;
				}

				if(parentselected==false)
				{
					l->index[c]=fo->GetIndex(); // Index
					l->list[c]=fo->olist;
					l->objects[c++]=fo; // Object
				}
			}

			fo=fo->NextFolderObject();
		}

#ifdef DEBUG
		if(c!=l->number)
			maingui->MessageBoxError(0,"CreateList OFLAG_SELECTED");
#endif

	}
}

void FolderList::CreateList(OOList *l)
{
	int c=0;
	FolderObject *fo=FirstFolderObject();

	while(fo){
		c++;
		fo=fo->NextFolderObject();
	}

	if(l->number!=c)
		l->InitOO(c);

	if(l->objects)
	{
		int c=0;
		FolderObject *fo=FirstFolderObject();

		while(fo){

			fo->qlist=l;
			fo->qindex=c;

			l->index[c]=fo->GetIndex(); // Index
			l->list[c]=fo->olist;
			l->objects[c++]=fo; // Object

			fo=fo->NextFolderObject();
		}

#ifdef DEBUG
		if(c!=l->number)
			maingui->MessageBoxError(0,"CreateList");
#endif

	}
}

void FolderList::ListToFolder(OOList *l)
{
	for(int i=0;i<l->number;i++)
		l->list[i]->Clear();

	//TRACE ("ListToFolder %d\n",this);

	for(int i=0;i<l->number;i++)
	{
		//TRACE ("List %d\n",l->list[i]);

		l->list[i]->AddEndO(l->objects[i]);
	}
}

FolderObject *FolderList::GetO(int ix)
{
	if(ix<0)
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"FolderList::GetO <0");
#endif
		return 0;
	}

	// All Objects+Childs
	FolderObject *f=(FolderObject *)c_root;

	while(f){
		if(!ix)return f;
		ix--;
		f=f->NextFolderObject();
	}

#ifdef DEBUG
	//	maingui->MessageBoxError(0,"FolderList::GetO ==0");
#endif
	return 0;
}


bool OList::MoveOToIndex(Object *object,int toindex)
{
	// LATER

	int index=object->GetIndex();

	if(toindex!=index)
	{
		int diff=toindex-index;

		if(diff<0) //Up
		{
			Object *pobj=object->prev;
			diff++;

			CutQObject(object);

			while(pobj)
			{
				if(!diff)
				{		
					AddNextO(object,pobj);
					return true;
				}

				diff++;
				pobj=pobj->prev;
			}

			AddStartO(object);
			return true;
		}

		// Down
		Object *pobj=object->next;
		diff--;

		CutQObject(object);

		while(pobj)
		{
			if(!diff)
			{		
				AddNextO(object,pobj);
				return true;
			}

			diff--;
			pobj=pobj->next;
		}

		AddEndO(object);

		return true;
	}

	return false;
}

Object *OList::SearchO(Object *o)
{
	Object *s=c_root;
	while(s){if(s==o)return s;s=s->next;}
	return 0;
}

void OList::SelectAll()
{
	Object *s=c_root;
	while(s)
	{
		s->Select();
		s=s->next;
	}

	selectionstart=c_root;
	selectionend=c_end;
}

void OList::DeSelectAll()
{
	Object *s=c_root;
	while(s)
	{
		s->UnSelect();
		s=s->next;
	}

	selectionstart=selectionend=0;
}

void OList::SetSelectionStart(Object *os)
{
	selectionstart=os;

	if(!selectionend)
		selectionend=os;
		
}

void OList::SelectSelection()
{
	if(selectionstart && selectionend)
	{
		Object *start=selectionstart;
		Object *end=selectionend;

		int is=selectionstart->GetIndex();
		int ie=selectionend->GetIndex();

		if(is>ie)
		{
			Object *h=start;
			start=end;
			end=h;
		}

		while(start)
		{
			start->Select();

			if(start==end)
				break;

			start=start->next;
		}

	}

}

void OList::SetSelectionEnd(Object *oe)
{
	selectionend=oe;

	if(!selectionstart)
		selectionstart=oe;
}

void OList::MoveSelectedObjects(OOList *l,int diff)
{
	if(l && diff)
	{
		OList *olist=0;

		//	1. Cut
		for(int i=0;i<l->number;i++)
		{
			Object *o=l->objects[i];
			if(o->IsSelected()==true)
			{
				o->olist->CutQObject(o);
				olist=o->olist; // No Multi List Move !
			}

		}

		//	2. Add+Diff
		for(int i=0;i<l->number;i++)
		{
			Object *o=l->objects[i];
			if(o->IsSelected()==true)
				o->olist->AddOToIndex(o,l->index[i]+diff);
		}

		if(olist)
			olist->Close(); // Set Indexs
	}
}

void OList::MoveObjects_Undo(OOList *list)
{
	//  TRACE ("MoveObjects_Undo");

	for(int i=0;i<list->number;i++)
	{
		//TRACE ("%d Index:%d\n",i,list->index[i]);

		list->objects[i]->olist->CutQObject(list->objects[i]);
	}

	for(int i=0;i<list->number;i++)
	{
		list->objects[i]->olist->AddEndO(list->objects[i]);
	}

}

bool OList::MoveOIndex(Object *object,int idiff)
{
	// LATER

	// Index - up, + down
	if(idiff!=0){

		int index=object->GetIndex();
		index+=idiff;

		if(idiff<0) // UP
		{
			if(index>=0){

				Object *n=GetO(index);
				if(n!=object){
					CutQObject(object);
					AddNextO(object,n);
					return true;
				}
			}
		}
		else // DOWN
		{
			Object *n=GetO(index);

			if(n && n!=object){
				CutQObject(object);
				AddPrevO(object,n);
				return true;
			}
		}
	}

	return false;
}

OStart *OListStart::FindOBefore(OSTART start) //<-| Tempo, Signature
{
	if((!GetRoot()) || GetRoot()->ostart>start)
		return 0;

	if(Getc_end()->ostart<=start)
		return (OStart *)c_end;

	OStart *s=GetRoot();
	do{
		if(s->ostart==start || (!s->Next()) || s->Next()->ostart>start)
			return s;

	}while(s=s->Next());

	return 0;
}

OStart *OListStart::FindObject(OSTART start) // |->
{
#ifdef DEBUG
	if(start<0)
		maingui->MessageBoxError(0,"FindObject <0");
#endif

	if((!GetRoot()) || Getc_end()->ostart<start)
		return 0;

	if(GetRoot()->ostart>=start)
		return GetRoot();

	if(Getc_end()->ostart-start<start-GetRoot()->ostart)
	{
		OStart *p=Getc_end();

		do{
			if((!p->prev) || p->Prev()->ostart<start)
				return p;

		}while(p=p->Prev());

		return 0;
	}

	// Start ...
	OStart *s=GetRoot();
	do{
		if(s->ostart>=start)
			return s;

	}while(s=s->Next());

	return 0;
}

OStart *OListStart::FindObjectAtPos(OSTART start)
{
	if((!GetRoot()) || Getc_end()->ostart<start)
		return 0;

	if(GetRoot()->ostart>start)
		return 0;

	// Start ...
	OStart *s=GetRoot();
	do{
		if(s->ostart==start)
			return s;

	}while(s=s->Next());

	return 0;
}

void OListStart::MoveO(OStart *object,OSTART newstart)
{
	if(newstart==object->ostart)return;

#ifdef FINDQUICK
	findquick=findquick_before=0;
#endif
	accesscounter++;
	object->ostart=newstart;

	if(object->Prev() && object->Prev()->ostart>=newstart) // Sort/Move Left <------
	{
		// Cut element from list
		if(object->Next())
			object->Next()->prev=object->prev;
		else 
			c_end=object->Prev();

		object->prev->next=object->next;

		// Sort <<<----
		OStart *pe=object->Prev()->Prev();
		while(pe && pe->ostart>=newstart)
			pe=pe->Prev();

		if(pe){	
			object->prev=pe;
			object->next=pe->next;
			pe->next->prev=object;
			pe->next=object;
		}
		else // First Element
		{
			if(object->next=c_root)
				c_root->prev=object;

			c_root=object;
			object->prev=0;
		}
	}
	else
		if(object->next && object->Next()->ostart<=newstart) // Sort/Move Right
		{
			// Cut element from list
			if(object->prev)
				object->prev->next=object->next;
			else
				c_root=object->Next();

			object->next->prev=object->prev;

			OStart *ne=object->Next()->Next();
			while(ne && ne->ostart<=newstart)
				ne=ne->Next();

			if(ne){
				object->next=ne;
				object->prev=ne->prev;

				ne->prev->next=object;
				ne->prev=object;
			}
			else // Last Element
			{
				object->next=0;

				if(object->prev=c_end)
					c_end->next=object;

				c_end=object;
			}
		}
}

FolderObject *FolderObject::LastFolderObject()
{
	if(FolderObject *lf=(FolderObject *)childs.Getc_end())
	{
		if(FolderObject *ll=lf->LastFolderObject())return ll;
		return lf;
	}
	return 0;
}

FolderObject *FolderObject::PrevFolderObject()
{
	if(prev)
	{
		if(FolderObject *lastf=((FolderObject *)prev)->LastFolderObject())return lastf;
		return (FolderObject *)prev;
	}

	if(parent)return parent;

	return 0;
}

FolderObject *FolderObject::NextFolderObject()
{
	if(childs.GetRoot())return (FolderObject *)childs.GetRoot();
	if(next)return (FolderObject *)next;

	FolderObject *p=parent;

	while(p)
	{
		if(p->next)
			return (FolderObject *)p->next;

		p=p->parent;
	}

	return 0;
}

FolderObject *FolderObject::PrevOpenFolderObject()
{
	FolderObject *pv=PrevFolderObject();
	while(pv)
	{
		bool ok=true;

		FolderObject *p=pv->parent;
		while(p)
		{
			if(p->showchilds==false)
			{				
				ok=false;
				break;
			}

			p=p->parent;
		}

		if(ok==true)
			return pv;

		pv=pv->PrevFolderObject();
	}

	return 0;
}

FolderObject *FolderObject::NextOpenFolderObject()
{
	FolderObject *n=NextFolderObject();
	while(n)
	{
		bool ok=true;

		FolderObject *p=n->parent;
		while(p)
		{
			if(p->showchilds==false)
			{				
				ok=false;
				break;
			}

			p=p->parent;
		}

		if(ok==true)
			return n;

		n=n->NextFolderObject();
	}

	return 0;
}


void FolderObject::AddChildObject(FolderObject *o,int index)
{
	if(!o)return;

	o->parent=this;
	o->childdepth=childdepth+1;

	childs.AddOToIndex(o,index);
}

FolderObject *FolderObject::NextChildNot(FolderObject *root)
{
	if(FirstChild())return FirstChild();
	if(next)return (FolderObject *)next;

	FolderObject *par=parent;
	while(par)
	{
		if(par==root)
			return 0;
		if(par->next)return (FolderObject *)par->next;
		par=par->parent;
	}

	return 0;
}

FolderObject *FolderObject::LastChild()
{
	FolderObject *f=(FolderObject *)childs.Getc_end(),*lf;
	while(f){
		if(!(lf=(FolderObject *)f->childs.Getc_end()))return f;
		f=lf;
	}

	return 0;
}

bool FolderObject::IsOpen()
{
	if(!parent)
		return true;

	FolderObject *p=parent;

	while(p)
	{
		if(p->showchilds==false)
			return false;

		p=p->parent;
	}

	return true;
}

void FolderObject::Close()
{
	childs.Close();

	FolderObject *fo=FirstChild();
	while(fo)
	{
		fo->Close();
		fo=fo->NextChild();
	}
}

bool FolderObject::CheckIfCanBeMove(int diff)
{
	if(diff<0)
	{
		// up
		FolderObject *p=(FolderObject *)prev;

		while(diff<0)
		{
			if(!p)return false;
			p=(FolderObject *)p->prev;

			diff++;
		}
	}
	else
	{
		// down
		FolderObject *n=(FolderObject *)next;

		while(diff)
		{
			if(!n)return false;
			n=(FolderObject *)n->next;
			diff--;
		}
	}

	return true;
}

OObject *OListCoos::FindObject(Object *o)
{
	OObject *c=(OObject *)GetRoot();

	while(c){

		if(c->object==o)
			return c;

		c=(OObject *)c->next;
	}

	return 0;
}

OObject *OListCoos::FindPrevSameObject(OObject *oo)
{
	if(oo)
	{
		OObject *on=(OObject *)oo->prev;
		while(on)
		{
			if(on->object->id==oo->object->id)
				return on;

			on=(OObject *)on->prev;
		}
	}

	return 0;
}

OObject *OListCoos::FindNextSameObject(OObject *oo)
{
	if(oo)
	{
		OObject *on=(OObject *)oo->next;
		while(on)
		{
			if(on->object->id==oo->object->id)
				return on;

			on=(OObject *)on->next;
		}
	}

	return 0;
}

void OListCoos::DeleteAllO(guiGadget_CW *db)
{
	OList::DeleteAllO();

	width=height=zoomwidth=0;

	if(db)
	{
		guiwidth=db->GetWidth();
		guiheight=db->GetHeight();
	}
	else
		guiwidth=guiheight=0;
}

// Y
int OListCoosY::AddInitY(int y)
{
	inity+=y;
	return inity;
}

void OListCoosY::SetGUIHeight(guiGadget_CW *db)
{
	guiheight=db?db->GetHeight():0;

	if(starty+guiheight>height)
		starty=height-guiheight;

	if(starty<0)
		starty=0;
}

void OListCoosY::SetSlider(guiGadget_Slider *sl)
{
	if(sl)
	{
		sl->ChangeSlider(0,height<guiheight?0:height-guiheight,starty);
	}
}

bool OListCoosY::Select(guiGadget_TabStartPosition *tab,int tabindex,bool unselectother)
{
	if(tabindex>=tab->tabs)
		return false;

	int mx=tab->GetMouseX();

	if(mx>=tab->tabx[tabindex] && mx<=tab->tabx2[tabindex])
	{
		int my=tab->GetMouseY();

		InitYStartO();

		if(GetShowObject()) // first track ?
		{
			OObject *so;

			while((so=GetShowObject()) && GetInitY()<tab->GetHeight())
			{
				int sy=so->cy;
				int sy2=sy+so->ch;

				sy-=starty;
				sy2-=starty;

				if(my>=sy && my<=sy2)
				{
					bool r=true;

					if(so->object)
					{
						if(so->object->IsSelected()==true && maingui->GetCtrlKey()==true)
						{
							so->object->UnSelect();
							r=false;
						}

						if(so->object->IsSelected()==false)
						{
							so->object->Select();
							r=false;
						}

						if(OList *list=so->object->olist)
						{
							if(maingui->GetShiftKey()==false)
							{
								list->SetSelectionStart(so->object);

								if(maingui->GetCtrlKey()==false && (r==false || unselectother==true))
								{
									UnSelectAll(so);
									// UnSelect all other
								}
							}
							else
							{
								list->SetSelectionEnd(so->object);
								list->SelectSelection();
							}
						}
#ifdef DEBUG
						else
							maingui->MessageBoxError(0,"0 List !");
#endif
					}

					return r;
				}

				NextYO();
			}

			UnSelectAll(0);
		}
	}

	return true;
}

int OListCoosY::GetYPosHiLo(double cy,double range) //linear hi->low
{
	double rh=height/range;
	double ch=cy*rh;

	int ypos=height-(int)ch;

	if(ypos<starty)
		return -1; // Top

	if(ypos>starty+guiheight)
		return -2; // Bottom

	return guiheight-((starty+guiheight)-ypos);
}

void OListCoosY::CalcStartYOff(int zoomy,int objectsondisplay,int from,int to)
{
	if(offsetymul)
	{
		if(objectsondisplay>2)
		{
			offsetymul-=(objectsondisplay-1)/2;
			if(offsetymul<from)
				offsetymul=from;
		}

		starty=zoomy*offsetymul;
		offsetymul=0;
	}
}

void OListCoosY::InitWithSlider(guiGadget_Slider *sl,bool set)
{
	starty=sl->pos;
	startsetbyslider=set;

	EndBuild();

	TRACE ("InitWithSlider Y %d\n",starty);
}

void OListCoosY::RecalcYPos()
{
	double h=height;
	h*=startpery;

	starty=(int)h;
	if(starty+guiheight>height)
		starty=height-guiheight;

	if(starty<0)
		starty=0;
}

void OListCoosY::BufferYPos()
{
	if(height>0)
	{
		double h=height;
		startpery=starty;
		startpery/=h;
	}
	else
		startpery=0;
}

bool OListCoosY::InitWithPercent(double per)
{
	if(per<0)
		per=0;

	if(per>1)
		per=1;

	//	TRACE ("OV Y %f\n",per);

	double h=height-guiheight;
	h*=per;

	int oldsy=starty;
	starty=(int)h;

	return starty!=oldsy?true:false;
}

void OListCoosY::StartBuild(guiGadget_CW *db)
{
	height=0;
	DeleteAllO(db);
}

void OListCoosY::DrawUnUsed(guiGadget_CW *db)
{
	if(height<guiheight)
	{
		int y2=db->GetY2();
		int y=y2-(guiheight-height);

		db->gbitmap.guiFillRectX0(y,db->GetX2(),y2,COLOUR_UNUSED);
	}
}

void OListCoosY::AddCooObject(Object *o,int h,int offseth)
{
	if(OObject *oo=new OObject(o))
	{
		oo->cx=0;
		oo->cy=height;
		oo->cw=width;
		oo->ch=h;

		height+=h+offseth;
		oo->offseth=offseth;
		OList::AddEndO(oo);
	}
}

OObject *OListCoosY::AddCooObjectR(Object *o,int h,int offseth)
{
	if(OObject *oo=new OObject(o))
	{
		oo->cx=0;
		oo->cy=height;
		oo->cw=width;
		oo->ch=h;

		height+=h+offseth;
		oo->offseth=offseth;
		OList::AddEndO(oo);

		return oo;
	}

	return 0;
}

OObject *OListCoosY::AddCooObjectRStart(Object *o,OSTART s,int h,int offseth)
{
	if(OObject *oo=new OObject(o))
	{
		oo->cx=0;
		oo->cy=height;
		oo->cw=width;
		oo->ch=h;
		oo->startposition=s;

		height+=h+offseth;
		oo->offseth=offseth;
		OList::AddEndO(oo);

		return oo;
	}

	return 0;
}

void OListCoosY::ScrollToEnd()
{
	if(height>=guiheight)
		starty=height-guiheight;
	else
		starty=0;
}

void OListCoosY::EndBuild(guiGadget_Slider *sl)
{
	if(starty+guiheight>height)
	{
	//	TRACE ("End Build Y starty reset...\n");

		starty=height-guiheight;
	}

	if(starty<0)
		starty=0;

	if(sl)
	{
		sl->ChangeSlider(this,1);

		
	}
}

bool OListCoosY::AddStartY(int addy)
{
	int oldsy=starty;
	starty+=addy;

	EndBuild();
	return oldsy!=starty?true:false;
}

bool OListCoosY::SetStartY(int y)
{
	int oldsy=starty;
	starty=y;
	EndBuild();
	return oldsy!=starty?true:false;
}

bool OListCoosY::SetStartY_Mid(int y)
{
	int h=guiheight;

	h/=2;

	y-=h;
	if(y<0)y=0;

	int oldsy=starty;
	starty=y;
	EndBuild();
	return oldsy!=starty?true:false;
}

void OListCoosY::ScrollY(OObject *oo)
{
	if(!oo)
	{
		SetStartY(0);
		return;
	}

	int i=oo->cy;
	int sy=starty;
	int ey=starty+guiheight;

	if(guiheight<=oo->ch)
	{
		SetStartY(oo->cy);
		return;
	}

	if(oo->cy<sy)
	{
		SetStartY(oo->cy);
		return;
	}

	if(oo->cy+oo->ch>ey)
	{
		SetStartY(starty+((oo->cy+oo->ch)-ey));
		return;
	}
}

void OListCoosY::InitStartOffSetY(int y)
{
	offsetymul=y;
}

bool OListCoosY::InitYStartOStart(OSTART start)
{
	OObject *o=(OObject *)OList::GetRoot();

	while(o)
	{
		if(o->startposition>=start)
		{		
			if(SetStartY(o->cy)==true)
			{
				InitYStartO();
				return true;
			}

			return false;
		}

		o=(OObject *)o->next;
	}

	// > 

	if(SetStartY(height-guiheight)==true)
	{
		InitYStartO();

		EndBuild();
		return true;
	}

	return false;
}

// Y
void OListCoos::InitYStart(int divy)
{
	if(divy>0){
		// 1 Object, Zoom Y
		startobjectint=starty/divy;
		inity=startobjectint*divy-starty;

		return;
	}

	startobjectint=0;
}

void OListCoos::InitYStartO()
{
	OObject *o=(OObject *)OList::GetRoot();

	while(o){
		if(o->cy<=starty && o->cy+o->ch+o->offseth>=starty){
			inity=o->cy-starty;
			showobject=o;
			return ;
		}

		o=(OObject *)o->next;
	}

	showobject=0;
}

void OListCoos::NextYO()
{
	if(showobject){
		inity+=showobject->ch+showobject->offseth;
		showobject=(OObject *)showobject->next;
	}
}

void OListCoos::UnSelectAll(OObject *not)
{
	OObject *o=(OObject *)OList::GetRoot();

	while(o){
		
		if(o!=not)
			o->object->UnSelect();

		o=(OObject *)o->next;
	}
}

// X

void OListCoos::NextXO()
{
	if(showobject){
		initx+=showobject->zoomw+showobject->offsetw;
		showobject=(OObject *)showobject->next;
	}
}

void OListCoos::OptimizeXStart()
{
	if(guiwidth>width)
	{
		int mid=guiwidth-width;
		mid/=2;
		initx+=mid;
	}
}

void OListCoos::InitXStart(int divx)
{
	if(divx>0){
		// 1 Object, Zoom Y
		startobjectint=startx/divx;
		initx=startobjectint*divx-startx;
		return;
	}

	startobjectint=0;
}

void OListCoos::InitXStartO()
{
	OObject *o=(OObject *)OList::GetRoot();

	while(o)
	{
		if(o->zx<=startx && o->zx+o->zoomw+o->offsetw>=startx)
		{
			initx=o->zx-startx;
			showobject=o;
			return;
		}

		o=(OObject *)o->next;
	}

	showobject=0;
}

int OListCoosX::AddInitX(int x)
{
	initx+=x;
	return initx;
}

void OListCoosX::InitWithSlider(guiGadget_Slider *sl)
{
	startx=sl->pos;
}

void OListCoosX::AddCooObject(Object *o,int w,int zoomw,int offsetw)
{
	if(OObject *oo=new OObject(o))
	{
		oo->cx=width;
		oo->zx=zoomwidth;
		oo->cy=0;
		oo->cw=w;
		oo->ch=height;
		oo->zoomw=zoomw;

		width+=w+offsetw;
		zoomwidth+=zoomw+offsetw;

		oo->offsetw=offsetw;

		//height+=h;
		OList::AddEndO(oo);
	}
}

void OListCoosX::EndBuild()
{
	if(startx+guiwidth>zoomwidth)
		startx=zoomwidth-guiwidth;

	if(startx<0)
		startx=0;
}

bool OListCoosX::AddStartX(int addx)
{
	int oldsx=startx;
	startx+=addx;

	EndBuild();
	return oldsx!=startx?true:false;
}

bool OListCoosX::ZoomX(double p) // 0-1
{
	if(!zoomwidth)return false;

	double h=zoomwidth;
	h*=p;

	if((int)h!=startx)
	{
		startx=(int)h;

		return true;
	}

	return false;
}

void OOList::InitOO(int c)
{
	if(c==number)return;

	FreeMemory();

	if(number=c)
	{
		objects=new Object*[c];
		index=new int[c];
		list=new OList*[c];

		if(list==0 || objects==0 || index==0)
			FreeMemory();
	}
}

void OOList::FreeMemory()
{
	if(list){delete list;list=0;}
	if(objects){delete objects;objects=0;}
	if(index){delete index;index=0;}
	number=/*childnumber=*/0;
}
