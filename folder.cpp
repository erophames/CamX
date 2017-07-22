/*
#include "folder.h"

Folder *Seq_Song::OpenFolder()
{
	Folder *newfolder=new Folder;
	
	if(newfolder)
	{
		newfolder->song=this;
	}
	
	return newfolder;
}

void Seq_Song::CloseFolder(Folder *f)
{
	if(f->FirstFolderPattern())
	{
		folder.AddOSort(f,f->FirstFolderPattern()->GetPatternStart());
	}
	else
		delete f;
}

Folder *Folder::DeleteFolder()
{
	Folder *n=(Folder *)song->folder.CutObject(this); // Remove From Song Folders
	
	Seq_Pattern *d,*p=FirstFolderPattern();
	while(p)
	{
		// Delete Pattern
		d=p;
		p=p->NextPattern();
		
		d->Delete(true); // Delete Pattern+Events
	}
	
	delete this;

	return n;
}



void Seq_Song::DeleteAllFolder()
{
	Folder *n=FirstFolder();
	
	while(n)
	{
		n=n->DeleteFolder();
	}
}

void Folder::AddPattern(Seq_Pattern *p)
{
	// Pattern from Track -> Folder
	Seq_Pattern *clone=p->CreateClone(0,0);
	
	if(clone)
	{
		pattern.AddOSort(clone,p->GetPatternStart());
		p->folder=this;
		
		if((end==-1) || p->GetPatternEnd()>end)
			end=p->GetPatternEnd();
	}
}

*/