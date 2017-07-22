#include "songmain.h"
#include "textandmarker.h"
#include "seqtime.h"
#include "object_song.h"
#include "objectpattern.h"
#include "arrangeeditor.h"
#include "editor_marker.h"
#include "gui.h"
#include "chunks.h"
#include "camxfile.h"
#include "semapores.h"

void Seq_Text::CloneData(Seq_Song *song,Seq_Event *e)
{
	Seq_Text *to=(Seq_Text *)e;

	to->ostart=ostart;
	to->staticostart=staticostart;
}

Object *Seq_Text::Clone(Seq_Song *song)
{
	if(Seq_Text *t=new Seq_Text){
		CloneData(song,t);
		return t;
	}

	return 0;
}

bool Seq_Text::Compare(Seq_Event *c)
{
	return false;
}

void Seq_Text::Load(camxFile *file)
{
	file->ReadChunk(&ostart);
	file->ReadChunk(&type);

	int dummy;
	file->ReadChunk(&dummy);

	file->Read_ChunkString(&string);
}

void Seq_Text::Save(camxFile *file)
{
	file->Save_Chunk(ostart);
	file->Save_Chunk(type);
	file->Save_Chunk(flag);

	file->Save_ChunkString(string);

	/*
	file->Save_Chunk(length+1);
	file->Save_Chunk(string,length+1);
	*/
}

void Seq_Text::ChangeText(char *newtext)
{
	if(string)
		delete string;

	string=0;

	if(newtext)
		string=mainvar->GenerateString(newtext);
}

Seq_Song *Seq_Text::GetSong()
{
	return map->song;
}

void Seq_Text::FreeMemory()
{
	if(string)
		delete string;
}

Seq_Song *Seq_Marker::GetSong()
{
	return map->song;
}

char *Seq_Marker::CreateFromToString()
{
	if(fromtostring)
		delete fromtostring;

	size_t i=string?strlen(string):1;

	fromtostring=new char[i+96];

	if(fromtostring)
	{
		strcpy(fromtostring,string?string:"?");

		mainvar->AddString(fromtostring,":");

		Seq_Pos pos(Seq_Pos::POSMODE_NORMAL);

		char h[65];
		GetSong()->timetrack.ConvertTicksToPos(GetMarkerStart(),&pos);
		pos.ConvertToString(GetSong(),h,64);
		mainvar->AddString(fromtostring,h);

		if(markertype==MARKERTYPE_DOUBLE)
		{
			mainvar->AddString(fromtostring,"<->");

			GetSong()->timetrack.ConvertTicksToPos(GetMarkerEnd(),&pos);
			pos.ConvertToString(GetSong(),h,64);
			mainvar->AddString(fromtostring,h);
		}
	}

	return fromtostring;
}

char *Seq_Marker::CreateFromString()
{
	if(fromstring)
		delete fromstring;

	Seq_Pos pos(Seq_Pos::POSMODE_NORMAL);

	char h[65];
	GetSong()->timetrack.ConvertTicksToPos(GetMarkerStart(),&pos);
	pos.ConvertToString(GetSong(),h,64);

	return fromstring=mainvar->GenerateString(string?string:"?",":",h);
}

bool Seq_TextandMarker::ChangeTextPosition(Seq_Text *t,OSTART pos)
{
	if(pos>=0 && pos!=t->GetTextStart())
	{
		text.MoveO(t,pos);
		return true;
	}

	return false;
}

Seq_Text *Seq_TextandMarker::AddText(OSTART pos,char *string,size_t length)
{
	if(length==-1)
		length=strlen(string);

	if(string && length && length<MAXTEXTMARKERLEN)
	{
		if(Seq_Text *newtext=new Seq_Text)
		{
			char *newc=new char[length+1]; // + 0 Buffer

			if(newc)
			{
				*(newc+length)=0;

				strncpy(newc,string,length);
				newtext->string=newc;
				newtext->map=this;
				text.AddOSort(newtext,pos);

				return newtext;
			}

			delete newtext;
		}
	}

	return 0;	
}

bool Seq_TextandMarker::ChangeMarkerPosition(Seq_Marker *m,OSTART pos)
{
	if(pos>=0 && pos!=m->GetMarkerStart() && (m->markertype==Seq_Marker::MARKERTYPE_SINGLE || pos<m->GetMarkerEnd()))
	{
		marker.MoveO(m,pos);
		return true;
	}

	return false;
}

Seq_Marker *Seq_TextandMarker::AddMarker(Seq_Pattern *p,bool doublemarker)
{
	return AddMarker(p->GetPatternStart(),doublemarker==true?p->GetPatternEnd():-1,p->GetName());
}

Seq_Marker *Seq_TextandMarker::AddMarker(OSTART pos,OSTART end,char *string,size_t length)
{
	bool single=false;

	if(end==-1) // Single Marker
		single=true;

	if(end<pos)
		end=pos+1;

	if(!string)
		string="Mk";

	if(length==-1)
		length=strlen(string);

	if(length>0 && length<MAXTEXTMARKERLEN)
	{
		if(Seq_Marker *newtext=new Seq_Marker)
		{
			char *newc=new char[length+4]; // + 4Buffer

			if(newc)
			{
				*(newc+length)=0;

				if(single==true)
					newtext->markertype=Seq_Marker::MARKERTYPE_SINGLE;

				newtext->endposition=end;
				strncpy(newc,string,length);

				newtext->string=newc;

				// AutoColouring
				int r,g,b;

				switch(marker.GetCount())
				{
				case 0:
					r=200;
					g=250;
					b=150;
					break;

				case 1:
					r=136;
					g=219;
					b=247;
					break;

				case 2:
					r=247;
					g=140;
					b=143;
					break;

				case 3:
					r=128;
					g=128;
					b=247;
					break;

				case 4:
					r=234;
					g=239;
					b=167;
					break;

				case 5:
					r=234;
					g=173;
					b=228;
					break;

				case 6:
					r=186;
					g=252;
					b=215;
					break;

				case 7:
					r=255;
					g=190;
					b=125;
					break;

				default: // 9+
					r=235;
					g=255;
					b=180;
					break;
				}

#ifdef WIN32
				// Intel Format ?
				newtext->colour.rgb=(b<<16)|(g<<8)|r;
#endif

				newtext->map=this;
				marker.AddOSort(newtext,pos);

				return newtext; // return ok;
			}
			else
			{
				delete newtext;
				newtext=0;
			}
		}
	}

	return 0; // return failure
}
Seq_Text *Seq_TextandMarker::DeleteText(Seq_Text *t)
{
	t->FreeMemory();
	return (Seq_Text *)text.RemoveO(t);
}

void Seq_TextandMarker::DeleteMarker_CalledByGUI(Seq_Marker *m)
{
	CutMarker(m);

	maingui->RefreshAllEditorsWithMarker(song,m);
	m->FreeMemory();
	delete m;

	/*
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
	if(w->WindowSong()==song)

	switch(w->GetEditorID())
	{
	case EDITORTYPE_ARRANGE:
	{
	Edit_Arrange *ed=(Edit_Arrange *)w;
	ed->ShowMarkerMap();
	}
	break;

	case EDITORTYPE_MARKER:
	{
	Edit_Marker *em=(Edit_Marker *)w;

	em->ShowAllText();
	}
	break;

	}

	w=w->NextWindow();
	}
	*/
}

void Seq_TextandMarker::MoveMarker(Seq_Marker *m,OSTART time)
{
	marker.MoveO(m,time);
}

Seq_Marker *Seq_TextandMarker::DeleteMarker(Seq_Marker *t)
{
	t->FreeMemory();
	return (Seq_Marker *)marker.RemoveO(t);
}

Seq_Marker *Seq_TextandMarker::CutMarker(Seq_Marker *t)
{
	return (Seq_Marker *)marker.CutObject(t);
}

int Seq_TextandMarker::GetCountOfSelectedTexts()
{
	int c=0;
	Seq_Text *t=FirstText();

	while(t){
		if(t->flag&OFLAG_SELECTED)c++;
		t=t->NextText();
	}

	return c;
}

void Seq_TextandMarker::RemoveAllTexts()
{
	Seq_Text *t=FirstText();
	while(t)t=DeleteText(t);
}

void Seq_TextandMarker::RemoveAllMarker()
{
	Seq_Marker *t=FirstMarker();

	while(t)t=DeleteMarker(t);
}

Seq_Marker *Seq_TextandMarker::FindMarkerID(int id)
{
	Seq_Marker *t=FirstMarker();

	while(t)
	{
		if(t->functionflag==id)
			return t;

		t=t->NextMarker();
	}

	return 0;
}

void Seq_TextandMarker::Init()
{
	if(!FindMarkerID(Seq_Marker::MARKERFUNC_STOPPLAYBACK))
	{
		OSTART end=song->GetSongEnd_Pattern();

		if(end==0)
			end=song->GetSongLength_Ticks();

		InitSongStopMarker(end+mainvar->ConvertMilliSecToTicks(3000));
	}
}

void Seq_TextandMarker::InitSongStopMarker(OSTART sspm)
{
	if(Seq_Marker *mk=FindMarkerID(Seq_Marker::MARKERFUNC_STOPPLAYBACK))
	{
		mainthreadcontrol->LockActiveSong();
		MoveMarker(mk,sspm);
		mainthreadcontrol->UnlockActiveSong();
		return;
	}

	if(Seq_Marker *m=AddMarker(sspm,-1,"Stop Position"))
	{
		m->functionflag=Seq_Marker::MARKERFUNC_STOPPLAYBACK;
	}
}

int Seq_TextandMarker::GetCountOfSelectedMarker()
{
	int c=0;

	Seq_Marker *t=FirstMarker();

	while(t)
	{
		if(t->IsSelected()==true)c++;
		t=t->NextMarker();
	}

	return c;
}

void Seq_TextandMarker::Load(camxFile *file)
{
	file->CloseReadChunk();
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_TEXT_)
	{
		file->ChunkFound();

		int nrtext=0;
		file->ReadChunk(&nrtext);

		while(nrtext--)
		{
			if(Seq_Text *ntext=new Seq_Text)
			{
				ntext->Load(file);
				ntext->map=this;
				text.AddOSort(ntext,ntext->ostart);
			}
		}

		file->CloseReadChunk();
		file->LoadChunk();
	}

	if(file->GetChunkHeader()==CHUNK_MARKER)
	{
		file->ChunkFound();

		int nrmk=0;
		file->ReadChunk(&nrmk);

		while(nrmk--)
		{
			if(Seq_Marker *mk=new Seq_Marker)
			{
				mk->Load(file);
				mk->map=this;
				marker.AddOSort(mk,mk->ostart);
			}
		}

		file->CloseReadChunk();
	}

	Close();
}

void Seq_TextandMarker::Save(camxFile *file)
{
	if(FirstText() || FirstMarker())
	{
		file->OpenChunk(CHUNK_TEXTANDMARKER);
		file->CloseChunk();

		if(FirstText()) // Text ?
		{
			file->OpenChunk(CHUNK_TEXT_);
			file->Save_Chunk(text.GetCount());

			Seq_Text *t=FirstText();

			while(t){
				t->Save(file);
				t=t->NextText();
			}

			file->CloseChunk();
		}

		if(FirstMarker()) // Marker ?
		{
			file->OpenChunk(CHUNK_MARKER);
			file->Save_Chunk(marker.GetCount());

			Seq_Marker *m=FirstMarker();

			while(m){
				m->Save(file);
				m=m->NextMarker();
			}

			file->CloseChunk();
		}
	}
}

Seq_Marker::Seq_Marker()
{
	functionflag=MARKERFUNC_NONE;

	endposition=0;
	markertype=MARKERTYPE_DOUBLE;
	string=0;
	fromtostring=0;
	fromstring=0;

	colour.showcolour=true;
	colour.rgb=0xFFEEDD;
}

void Seq_Marker::ChangeText(char *newtext)
{
	if(string)delete string;
	string=mainvar->GenerateString(newtext);
}

void Seq_Marker::SetMarkerStart(OSTART npos)
{
	if(npos<endposition || markertype==MARKERTYPE_SINGLE)
	{
		GetList()->MoveO(this,npos); // new startposition, sort ?
		//ostart=npos;
	}
}

void Seq_Marker::SetMarkerEnd(OSTART npos)
{
	if(endposition>ostart)
	{
		endposition=npos;
	}
}

void Seq_Marker::SetMarkerStartEnd(OSTART s,OSTART e)
{
	if(s<e)
	{
		GetList()->MoveO(this,s); // new startposition, sort ?

		//ostart=s;
		endposition=e;
	}
}

void Seq_Marker::FreeMemory()
{
	if(fromstring){
		delete fromstring;
		fromstring=0;
	}

	if(fromtostring){
		delete fromtostring;
		fromtostring=0;
	}

	if(string){
		delete string;
		string=0;
	}
}

void Seq_Marker::CloneData(Seq_Song *song,Seq_Event *e)
{
	Seq_Marker *to=(Seq_Marker *)e;

	colour.Clone(&to->colour);
	to->endposition=endposition;
	to->functionflag=functionflag;
	to->markertype=markertype;
	to->ostart=ostart;
	to->staticostart=staticostart;
}

Object *Seq_Marker::Clone(Seq_Song *song)
{
	if(Seq_Marker *t=new Seq_Marker){
		CloneData(song,t);
		return t;
	}

	return 0;
}

bool Seq_Marker::Compare(Seq_Event *c)
{
	Seq_Marker *to=(Seq_Marker *)c;

	if(markertype==to->markertype && 
		to->endposition==endposition && 
		to->functionflag==to->functionflag
		)
		return true;

	return false;
}

void Seq_Marker::Load(camxFile *file)
{
	file->ReadChunk(&ostart);
	file->ReadChunk(&markertype);

	int dummy;
	file->ReadChunk(&dummy);
	file->ReadChunk(&endposition);

	file->Read_ChunkString(&string);

	colour.LoadChunk(file);

	file->ReadChunk(&functionflag);
}

void Seq_Marker::Save(camxFile *file)
{
	file->Save_Chunk(ostart);
	file->Save_Chunk(markertype);
	file->Save_Chunk(flag);
	file->Save_Chunk(endposition);

	file->Save_ChunkString(string);

	colour.SaveChunk(file);

	file->Save_Chunk(functionflag);
}
