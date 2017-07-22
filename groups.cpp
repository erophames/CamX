#include "folder.h"
#include "gui.h"
#include "groupeditor.h"
#include "semapores.h"
#include "songmain.h"
#include "chunks.h"
#include "object_song.h"
#include "object_track.h"
#include "camxfile.h"

// Groups
void Seq_Song::SetGroupRec(Seq_Group *gr,bool rec)
{
	gr->rec=rec;

	OSTART pos=GetSongPosition();
	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->GetGroups()->FindGroup(gr)==true)
			t->SetRecordMode(rec,pos);

		t=t->NextTrack();
	}

	SetMIDIRecordingFlag();
}

void Seq_Song::SetGroupSolo(Seq_Group *gr,bool solo)
{
	if(solo==true)
	{
		Seq_Track *t=FirstTrack();

		while(t)
		{
			if(t->GetGroups()->FindGroup(gr)==true)
				break;

			t=t->NextTrack();
		}

		if(!t)
		{
			if(solo==true)
				solo=false;
		}
	}

	if(solo!=gr->solo)
	{
		mainthreadcontrol->LockActiveSong();

		groupsoloed=solo;

		Seq_Group *g=FirstGroup();

		while(g){
			g->solo=gr==g?solo:false;
			g=g->NextGroup();
		}

		mainthreadcontrol->UnlockActiveSong();
	}	
}

void Seq_Song::SetTrackGroup(Seq_Track *t,Seq_Group *g)
{
	if(t && g)
	{
		Seq_Group_GroupPointer *sgp=t->GetGroups()->FirstGroup();
		Seq_Group *oldgroup;

		if(sgp)
			oldgroup=sgp->group;
		else
			oldgroup=0;

		mainthreadcontrol->LockActiveSong();

		t->GetGroups()->AddToGroup(g);

		mainthreadcontrol->UnlockActiveSong();

		if( (oldgroup && g && oldgroup->colour.Compare(&g->colour)==false) ||
			((!oldgroup) && g && g->colour.showcolour==true)
			)
			maingui->RefreshColour(g);

		// Group Editor
		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			if(w->WindowSong()==this && w->GetEditorID()==EDITORTYPE_GROUP)
			{
				Edit_Group *ed=(Edit_Group *)w;

				ed->ShowGroups();
				ed->ShowActiveGroup();
			}

			w=w->NextWindow();
		}

		maingui->ClearRefresh();
	}
}

Seq_Group* Seq_Song::CreateNewGroup()
{
	if(Seq_Group *ng=new Seq_Group)
	{
		ng->song=this;
		groups.AddEndO(ng);
		return ng;
	}

	return 0;
}

void Seq_Song::LoadGroups(camxFile *file)
{
	int nrtracks=0;

	file->ReadChunk(&nrtracks);
	file->ReadChunk(&groupsoloed);
	file->CloseReadChunk();

	while(nrtracks--)
	{
		file->LoadChunk();

		if(file->CheckReadChunk()==false)
			break;

		if(file->GetChunkHeader()==CHUNK_SONGGROUP)
		{
			file->ChunkFound();

			if(Seq_Group *g=CreateNewGroup())
				g->Load(file);
			else
				file->CloseReadChunk();
		}
	}
}

void Seq_Song::SaveGroups(camxFile *file)
{
	if(FirstGroup())
	{
		file->OpenChunk(CHUNK_SONGGROUPS);
		file->Save_Chunk(groups.GetCount());
		file->Save_Chunk(groupsoloed);

		file->CloseChunk();

		// Groups
		Seq_Group *g=FirstGroup();

		while(g){
			g->Save(file);
			g=g->NextGroup();
		}
	}
}

Seq_Group* Seq_Song::DeleteGroup(Seq_Group *g,bool removefromtracks)
{
	if(removefromtracks==true)
	{
		if(groupsoloed==true)
		{
			if(g->solo==true)
				groupsoloed=false;
		}

		mainthreadcontrol->LockActiveSong();

		//for(int undo=0;undo<2;undo++)
		{
			Seq_Track *t=FirstTrack();

			while(t){
				if(!t->parent)
					t->t_groups.RemoveBusFromGroup(g);

				t=t->NextTrack();
			}
		}

		mainthreadcontrol->UnlockActiveSong();
	}

	return (Seq_Group *)groups.RemoveO(g);
}

void Seq_Song::DeleteAllGroups()
{
	Seq_Group *g=FirstGroup();

	while(g)
		g=DeleteGroup(g,false);
}

void Seq_Group::SetName(char *h)
{
	if(h)
	{
		if(strlen(h)>STANDARDSTRINGLEN)
		{
			strncpy(name,h,STANDARDSTRINGLEN);
			name[STANDARDSTRINGLEN]=0;
		}
		else
			strcpy(name,h);
	}
}

void Seq_Group::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);

	file->Read_ChunkString(name);

	file->ReadChunk(&mute);
	file->ReadChunk(&solo);
	file->ReadChunk(&rec);

	colour.LoadChunk(file);

	file->CloseReadChunk();
}

void Seq_Group::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_SONGGROUP);

	file->Save_Chunk((CPOINTER)this);

	file->Save_ChunkString(name);

	file->Save_Chunk(mute);
	file->Save_Chunk(solo);
	file->Save_Chunk(rec);

	colour.SaveChunk(file);

	file->CloseChunk();
}

Seq_Group_GroupPointer *Seq_Group_Group::AddToGroup(Seq_Group *g)
{
	if(g)
	{
		Seq_Group_GroupPointer *f=FirstGroup();

		while(f){
			if(f->group==g)return f;
			f=f->NextGroup();
		}

		if(Seq_Group_GroupPointer *sgp=new Seq_Group_GroupPointer){
			sgp->group=g;

			groups.AddEndO(sgp);

			if(activegroup==0)
				activegroup=g;

			return sgp;
		}
	}

	return 0;
}

void Seq_Group_Group::RemoveBusFromGroup(Seq_Group *g)
{
	Seq_Group_GroupPointer *f=FirstGroup();
	while(f){

		if(f->group==g){

			if(activegroup==g)
				activegroup=(Seq_Group *)g->NextOrPrev();

			groups.RemoveO(f);
			break;
		}

		f=f->NextGroup();
	}
}

void Seq_Group_Group::Delete()
{
	Seq_Group_GroupPointer *f=FirstGroup();

	while(f)
		f=(Seq_Group_GroupPointer *)groups.RemoveO(f);
}

// MIDI Out
Seq_Group_MIDIOutPointer *Seq_Group_MIDIOutputDevice::AddToGroup(int portindex)
{
		Seq_Group_MIDIOutPointer *f=FirstDevice();

		while(f){
			if(f->portindex==portindex)
				return f;

			f=f->NextGroup();
		}

		if(Seq_Group_MIDIOutPointer *sgp=new Seq_Group_MIDIOutPointer)
		{
			sgp->portindex=portindex;
			groups.AddEndO(sgp);
			return sgp;
		}

	return 0;
}

void Seq_Group_MIDIOutputDevice::Replace(int oldindex,int newindex)
{
		Seq_Group_MIDIOutPointer *f=FirstDevice();

		while(f){

			if(f->portindex==oldindex){
				f->portindex=newindex;
				break;
			}

			f=f->NextGroup();
		}
}

void Seq_Group_MIDIOutputDevice::RemoveBusFromGroup(int portindex)
{
		Seq_Group_MIDIOutPointer *f=FirstDevice();

		while(f){

			if(f->portindex==portindex){
				groups.RemoveO(f);
				break;
			}

			f=f->NextGroup();
		}
}

bool Seq_Group_MIDIOutputDevice::FindPort(int index)
{
	Seq_Group_MIDIOutPointer *sgp=FirstDevice();

	while(sgp)
	{
		if(sgp->portindex==index)
			return true;

		sgp=sgp->NextGroup();
	}

	return false;
}

void Seq_Group_MIDIOutputDevice::CloneToGroup(Seq_Group_MIDIOutputDevice *sg)
{
	sg->Delete();
	Seq_Group_MIDIOutPointer *fd=FirstDevice();
	while(fd)
	{
		sg->AddToGroup(fd->portindex);
		fd=fd->NextGroup();
	}
}

bool Seq_Group_MIDIOutputDevice::CompareWithGroup(Seq_Group_MIDIOutputDevice *sg)
{
	Seq_Group_MIDIOutPointer *d1=sg->FirstDevice();
	Seq_Group_MIDIOutPointer *d2=FirstDevice();

	while(d1 && d2)
	{
		if(d1->GetDevice()!=d2->GetDevice())
			return false;

		d2=d2->NextGroup();
		d1=d1->NextGroup();
	}

	if(d1 || d2)
		return false;

	return true;
}

void Seq_Group_MIDIOutputDevice::Delete()
{
	Seq_Group_MIDIOutPointer *f=FirstDevice();

	while(f)
		f=(Seq_Group_MIDIOutPointer *)groups.RemoveO(f);
}

// MIDI In
Seq_Group_MIDIInPointer *Seq_Group_MIDIInputDevice::AddToGroup(int portindex)
{
		Seq_Group_MIDIInPointer *f=FirstDevice();

		while(f){
			if(f->portindex==portindex)
				return f;

			f=f->NextGroup();
		}

		if(Seq_Group_MIDIInPointer *sgp=new Seq_Group_MIDIInPointer)
		{
			sgp->portindex=portindex;
			groups.AddEndO(sgp);
			return sgp;
		}

		return 0;
}

void Seq_Group_MIDIInputDevice::Replace(int oldindex,int newindex)
{
		Seq_Group_MIDIInPointer *f=FirstDevice();

		while(f){

			if(f->portindex==oldindex){
				f->portindex=newindex;
				break;
			}

			f=f->NextGroup();
		}
}

void Seq_Group_MIDIInputDevice::RemoveBusFromGroup(MIDIInputDevice *g)
{
	if(!g)return;

	Seq_Group_MIDIInPointer *f=FirstDevice();

	while(f){

		if(f->GetDevice()==g){
			groups.RemoveO(f);
			break;
		}

		f=f->NextGroup();
	}
}

bool Seq_Group_MIDIInputDevice::FindPort(int index)
{
	Seq_Group_MIDIInPointer *sgp=FirstDevice();

	while(sgp){
		if(sgp->portindex==index)return true;
		sgp=sgp->NextGroup();
	}

	return false;
}

void Seq_Group_MIDIInputDevice::CloneToGroup(Seq_Group_MIDIInputDevice *sg)
{
	sg->Delete();

	Seq_Group_MIDIInPointer *fd=FirstDevice();

	while(fd){
		sg->AddToGroup(fd->portindex);
		fd=fd->NextGroup();
	}
}

bool Seq_Group_MIDIInputDevice::CompareWithGroup(Seq_Group_MIDIInputDevice *sg)
{
	Seq_Group_MIDIInPointer *d1=sg->FirstDevice(),*d2=FirstDevice();

	while(d1 && d2)
	{
		if(d1->GetDevice()!=d2->GetDevice())return false;

		d2=d2->NextGroup();
		d1=d1->NextGroup();
	}

	if(d1 || d2)
		return false;

	return true;
}

void Seq_Group_MIDIInputDevice::Delete()
{
	Seq_Group_MIDIInPointer *f=FirstDevice();
	while(f)
	{
		f=(Seq_Group_MIDIInPointer *)groups.RemoveO(f);
	}

}

Seq_AudioIOPointer *Seq_AudioIO::FirstOutputBus()
{
	Seq_AudioIOPointer *f=FirstChannel();

	while(f)
	{
		if(f->bypass==false)
			return f;

		f=f->NextChannel();
	}

	return 0;
}

// Audio Channels
Seq_AudioIOPointer *Seq_AudioIO::AddToGroup(AudioChannel *g)
{
	if(g && (!FindChannel(g)))
	{
		if(Seq_AudioIOPointer *sgp=new Seq_AudioIOPointer){

			sgp->channel=g;

			busgroups.AddEndO(sgp);
			busgroups.Close(); // Index

			if(!defaultrecordchannel)
				defaultrecordchannel=g;

			if(!defaultchannel)
				defaultchannel=g;

			if(track)
				track->SetMIDIType(-1);

			return sgp;
		}
	}

	return 0;
}

Seq_AudioIOPointer *Seq_AudioIO::AddToGroup(AudioChannel *g,int index)
{
	if(g && (!FindChannel(g)))
	{
		if(Seq_AudioIOPointer *sgp=new Seq_AudioIOPointer){

			sgp->channel=g;

			busgroups.AddOToIndex(sgp,index);
			busgroups.Close(); // Index

			if(!defaultrecordchannel)
				defaultrecordchannel=g;

			if(!defaultchannel)
				defaultchannel=g;

			if(track)
				track->SetMIDIType(-1);

			return sgp;
		}
	}

	return 0;
}

void Seq_AudioIO::ReplaceChannel(AudioChannel *o,AudioChannel *n)
{
	Seq_AudioIOPointer *f=FirstChannel();

	while(f)
	{
		if(f->channel==o){

			if(defaultrecordchannel==o)
				defaultrecordchannel=n;

			if(defaultchannel==o)
				defaultchannel=n;

			f->channel=n;
			f->bypass=false;

			if(track)
			track->SetMIDIType(-1);
		}

		f=f->NextChannel();
	}
}

void Seq_AudioIO::RemoveBusFromGroup(AudioChannel *g)
{
	Seq_AudioIOPointer *f=FirstChannel();

	while(f)
	{
		if(f->channel==g){

			if(defaultrecordchannel==g){
				Seq_AudioIOPointer *nop=(Seq_AudioIOPointer *)f->NextOrPrev();
				defaultrecordchannel=nop?nop->channel:0;
			}

			if(defaultchannel==g){

				Seq_AudioIOPointer *nop=(Seq_AudioIOPointer *)f->NextOrPrev();
				defaultchannel=nop?nop->channel:0;
			}

			busgroups.RemoveO(f);
			busgroups.Close(); // index

			return;
		}

		f=f->NextChannel();
	}
}

bool Seq_AudioIO::CompareWithGroup(Seq_AudioIO *sg)
{
	Seq_AudioIOPointer *d1=sg->FirstChannel(),*d2=FirstChannel();

	while(d1 && d2)
	{
		if(d1->channel!=d2->channel || d1->bypass!=d2->bypass)
			return false;

		d2=d2->NextChannel();
		d1=d1->NextChannel();
	}

	if(d1 || d2)
		return false;

	return true;
}

void Seq_AudioIO::Delete()
{
	Seq_AudioIOPointer *f=FirstChannel();

	while(f)
		f=(Seq_AudioIOPointer *)busgroups.RemoveO(f);

	busgroups.Close();

	defaultchannel=0;
}

bool Seq_AudioIO::FindChannel(AudioChannel *d)
{
	Seq_AudioIOPointer *sgp=FirstChannel();

	while(sgp)
	{
		if(sgp->channel==d)
			return true;

		sgp=sgp->NextChannel();
	}

	return false;
}

void Seq_AudioIO::CloneToGroup(Seq_AudioIO *sg)
{
	sg->Delete();

	Seq_AudioIOPointer *fd=FirstChannel();
	while(fd){

		if(Seq_AudioIOPointer *p=sg->AddToGroup(fd->channel))
		{
			p->bypass=fd->bypass;
		}

		fd=fd->NextChannel();
	}

	sg->busgroups.Close();
}

bool Seq_AudioIO::Compare(Seq_AudioIO *comp)
{
	Seq_AudioIOPointer *fd=FirstChannel(),*fd_comp=comp->FirstChannel();

	if((fd==0 && fd_comp) || (fd_comp==0 && fd))
		return false;

	if(fd && fd_comp)
	{
		while(fd && fd_comp)
		{
			if(fd->channel!=fd_comp->channel)
				return false;

			if(fd->bypass!=fd_comp->bypass)
				return false;

			fd_comp=fd_comp->NextChannel();
			fd=fd->NextChannel();
		}

		if(fd || fd_comp) // rest ?
			return false;
	}

	return true;
}

void Seq_Song::RemoveBusFromGroupFromSelectedTracks(Seq_Group_GroupPointer *pointer)
{
	if(pointer)
	{
		Seq_Group *group=pointer->group;

		pointer->underdeconstruction=true;
		maingui->RefreshAllArrangeWithGroup(group); // refresh colour

		Seq_Track *t=FirstTrack();

		if(mainvar->GetActiveSong()==this)
			mainthreadcontrol->LockActiveSong();

		while(t)
		{
			if((t->flag&OFLAG_SELECTED) && (!t->parent))
				t->GetGroups()->RemoveBusFromGroup(group);

			t=t->NextTrack();
		}

		if(mainvar->GetActiveSong()==this)
			mainthreadcontrol->UnlockActiveSong();

		RefreshGroupSolo();
		maingui->RefreshAllEditors(0,EDITORTYPE_GROUP,(LONGLONG)group);
	}
}

void Seq_Song::AddGroupToSelectedTracks(Seq_Group *group)
{
	Seq_Track *t=FirstTrack();

	if(mainvar->GetActiveSong()==this)
		mainthreadcontrol->LockActiveSong();

	while(t)
	{
		if(t->IsSelected()==true && (!t->parent))
			t->GetGroups()->AddToGroup(group);

		t=t->NextTrack();
	}

	if(mainvar->GetActiveSong()==this)
		mainthreadcontrol->UnlockActiveSong();

	maingui->RefreshAllEditors(0,EDITORTYPE_GROUP,(LONGLONG)group);
	maingui->RefreshAllArrangeWithGroup(group);
}