#include "defines.h"
#include "gui.h"
#include "drumeditor.h"

#include "gmdrums.h"
#include "icdobject.h"
#include "drumevent.h"
#include "object_project.h"
#include "object_song.h"
#include "object_track.h"
#include "songmain.h"
#include "semapores.h"
#include "chunks.h"
#include "camxfile.h"

// GM 1
char *gmproggroups[16]=
{
	"Piano",
	"Chromatic Percussion",
	"Organ",
	"Guitar",
	"Bass",
	"Strings",
	"Ensemble",
	"Brass",
	"Reed",
	"Pipe",
	"Synth Lead",
	"Synth Pad",
	"Synth Effects",
	"Ethnic",
	"Percussive",
	"Sound Effects"
};

char *gmprognames[128]=
{
	"Acoustic Grand Piano",
	"Bright Acoustic Piano",
	"Electric Grand Piano",
	"Honky-Tonk-Piano",
	"E-Piano 1",
	"E-Piano 2",
	"Harpsichord",
	"Clavinet",
	"Celesta",
	"Glockenspiel",

	"Music Box",
	"Vibraphone",
	"Marimba",
	"Xylophone",
	"Tubular Bell",
	"Dulcimer",
	"Drawbar Organ",
	"Percussive Organ",
	"Rock Organ",
	"Church Organ",

	"Reed Organ",
	"French Accordeon",
	"Harmonica",
	"Tango Arccordeon",
	"Nylon-String-Guitar",
	"Steel-String-Guitar",
	"Jazz Guitar",
	"Clean Guitar",
	"Muted Guitar",
	"Overdrive Guitar",

	"Distortion Guitar",
	"Guitar Harmonics",
	"Acoustic Bass",
	"Fingered Bass",
	"Picked Bass",
	"Fretless Bass",
	"Slap Bass 1",
	"Slap Bass 2",
	"Synth Bass 1",
	"Synth Bass 2",

	"Violin",
	"Viola",
	"Cello",
	"Contrabass",
	"Tremolo Strings",
	"Pizzicato Strings",
	"Harp",
	"Timpani",
	"Strings",
	"Slow Strings",

	"Syn-Strings 1",
	"Syn-Strings 2",
	"Choir Aahs",
	"Voice Oohs",
	"Syn Vox",
	"Orchestra Hit",
	"Trumpet",
	"Trombone",
	"Tuba",
	"Muted Trumpet",

	"French Horn",
	"Brass 1",
	"Synth Brass 1",
	"Synth Brass 2",
	"Soprano Sax",
	"Alto Sax",
	"Tenor Sax",
	"Baritone Sax",
	"Oboe",
	"English Horn",

	"Bassoon",
	"Clarinet",
	"Piccolo",
	"Flute",
	"Recorder",
	"Pan Flute",
	"Bottle Blow",
	"Shakuhachi",
	"Whistle",
	"Ocarina",

	"Lead1:Square Wave",
	"Lead2:Saw Wave",
	"Lead3:Synth Calliope",
	"Lead4:Chiffer Lead",
	"Lead5:Charang",
	"Lead6:Solo Vox",
	"Lead7:5th Saw Wave",
	"Lead8:Bass & Lead",
	"Pad1:Fantasia",
	"Pad2:Warm Pad",

	"Pad3:Polysynth",
	"Pad4:Space Voice",
	"Pad5:Bowed Glass",
	"Pad6:Metal Pad",
	"Pad7:Halo Pad",
	"Pad8:Sweep Pad",
	"FX1:Ice Rain",
	"FX2:Soundtrack",
	"FX3:Crystal",
	"FX4:Atmosphere",

	"FX5:Brightness",
	"FX6:Goblin",
	"FX7:Echo Drops",
	"FX8:Star Theme",
	"Sitar",
	"Banjo",
	"Shamisen",
	"Koto",
	"Kalimba",
	"Bag Pipe",

	"Fiddle",
	"Shannai",
	"Tinkle Bell",
	"Agogo",
	"Steel Drums",
	"Woodblock",
	"Taiko",
	"Melodic Tom",
	"Synth Drum",
	"Reverse Cymbal",

	"Guitar Fret Noise",
	"Breath Noise",
	"Seashore",
	"Bird",
	"Telephone 1",
	"Helicopter",
	"Applause",
	"Gun Shot"
};

char *gmkeynames[47]=
{
	"Acoustic Bass Drum",     
	"Bass Drum 1",           
	"Side Stick",            
	"Acoustic Snare",         
	"Hand Clap",      
	"Electric Snare",       
	"Low Floor Tom",          
	"Closed Hi-Hat",          
	"High Floor Tom",      
	"Pedal Hi-Hat",    

	"Low Tom",        
	"Open Hi-Hat",           
	"Low-Mid Tom",          
	"Hi-Mid Tom",          
	"Crash Cymbal 1",        
	"High Tom",       
	"Ride Cymbal 1",         
	"Chinese Cymbal",       
	"Ride Bell",      
	"Tambourine",            

	"Splash Cymbal",         
	"Cowbell",        
	"Crash Cymbal 2",   
	"Vibraslap",
	"Ride Cymbal 2",
	"Hi Bongo",
	"Low Bongo",
	"Mute Hi Conga",
	"Open Hi Conga",
	"Low Conga",

	"High Timbale",
	"Low Timbale",
	"High Agogo",
	"Low Agogo",
	"Cabasa",
	"Maracas",
	"Short Whistle",
	"Long Whistle",
	"Short Guiro",
	"Long Guiro",

	"Claves",
	"Hi Wood Block",
	"Low Wood Block",
	"Mute Cuica",
	"Open Cuica",
	"Mute Triangle",
	"Open Triangle"
};

Drummap::Drummap()
{
	strcpy(name,"Drum Map");
	usegm=true; 
	solomode=false;
	dmfilename=0;
	song=0;
}

Drumtrack *Drummap::GetTrackWithIndex(int index)
{
	Drumtrack *t=FirstTrack();

	while(t)
	{
		if(t->index==index)return t;
		t=t->NextTrack();
	}

	return t;
}

bool Drummap::CheckIfDrumEvent(Drumtrack *start,Drumtrack *end,ICD_Drum *drum)
{
	if(!drum)
		return false;

	if(drum->drumtrack->map!=this)
		return false;

	return mainvar->CheckIfInIndex(start->index,end->index,drum->drumtrack->index);
}

void Drummap::SetTrackIndexs()
{
	int ix=0;
	Drumtrack *t=FirstTrack();

	while(t)
	{
		t->index=ix++;
		t=t->NextTrack();
	}
}

int Drummap::GetCountOfTracks()
{
	return tracks.GetCount();
}

void Drummap::RefreshAllEditors(bool refreshmixlist)
{
#ifdef OLDIE
	guiWindow *f=maingui->FirstWindow();

	while(f)
	{	
		switch(f->GetEditorID())
		{
		case EDITORTYPE_DRUM:
			{
				Edit_Drum *d=(Edit_Drum *)f;

				if(d->drummap==this)
				{
					if(!d->focustrack)
						d->starttrack=d->focustrack=FirstTrack();

					if(refreshmixlist==true)
						d->patternselection.BuildEventList(SEL_ALLEVENTS,0); // Mix new List, events maybe moved/deleted

					//	d->trackfx.ShowActiveTrack();

					d->ShowDrumTracks();
					d->ShowEvents();
				}
			}
			break;

		case EDITORTYPE_DRUMMAP:
			{
				Edit_Drummap *edmap=(Edit_Drummap *)f;

				edmap->ShowDrumMaps();

				if(edmap->activedrummap==this)
				{
					edmap->ShowDrumTracks();
				}

			}
			break;

		}

		f=f->NextWindow();
	}
#endif

}

Drumtrack* Drummap::CreateDrumTrack(int index)
{
	if(Drumtrack *dt=new Drumtrack(this))
	{
		if(index>=0)
			tracks.AddOToIndex(dt,index);
		else
			tracks.AddEndO(dt);

		return dt;
	}

	return 0;
}

Drumtrack *Drummap::DeleteDrumTrack(Drumtrack *dtrack)
{
	Drumtrack *n=dtrack->NextTrack();
	tracks.RemoveO(dtrack);
	return n;
}

void Drummap::ResetSolo()
{
	solomode=false;
	Drumtrack *t=FirstTrack();

	while(t)
	{
		t->solo=false;
		t=t->NextTrack();
	}
}

void Drummap::DeleteAllDrumTracks()
{
	Drumtrack *t=FirstTrack();

	while(t)
		t=DeleteDrumTrack(t);
}

int Drummap::GetIndexOfTrack(Drumtrack *c){return c->index;}

void Drummap::RemoveDrumtrackFromSongs(Drumtrack *dt)
{
	Drummap *map=dt->map;
	int deleted=0;

	guiWindow *f=maingui->FirstWindow();

	while(f)
	{	
		switch(f->GetEditorID())
		{
		case EDITORTYPE_DRUM:
			{
				Edit_Drum *d=(Edit_Drum *)f;

				//if(d->starttrack==dt)
				//	d->starttrack=(Drumtrack *)dt->NextOrPrev();

				if(d->focustrack==dt)
					d->focustrack=(Drumtrack *)dt->NextOrPrev();


				deleted=1;
			}
			break;

			/*
			case EDITORTYPE_DRUMMAP:
			{
			Edit_Drummap *edmap=(Edit_Drummap *)f;

			edmap->ShowDrumMaps();

			if(edmap->activedrummap==map)
			{
			edmap->ShowDrumTracks();
			}

			}
			break;
			*/
		}

		f=f->NextWindow();
	}

	Seq_Project *p=mainvar->FirstProject();

	while(p)
	{
		Seq_Song *song=p->FirstSong();

		while(song)
		{
			int songdeleted=0;

			Seq_Track *t=song->FirstTrack();

			while(t)
			{
				Seq_Pattern *p=t->FirstPattern(MEDIATYPE_MIDI);

				while(p)
				{
					MIDIPattern *mp=(MIDIPattern *)p;
					Seq_Event *e=mp->FirstEvent(),*ne;

					while(e)
					{
						ne=e->NextEvent();

						if(e->GetICD()==ICD_TYPE_DRUM)
						{
							ICD_Drum *drum=(ICD_Drum *)e;

							if(drum->drumtrack==dt)
							{
								mainthreadcontrol->LockActiveSong();
								mp->DeleteEvent(e,true);
								mainthreadcontrol->UnlockActiveSong();
								songdeleted++;
								deleted++;
							}
						}

						e=ne;
					}

					p=p->NextPattern(MEDIATYPE_MIDI);
				}

				t=t->NextTrack();
			}

			if(songdeleted)
				song->undo.RefreshUndos();

			song=song->NextSong();
		}

		p=p->NextProject();
	}

	map->DeleteDrumTrack(dt);

	if(deleted)
	{
		map->RefreshAllEditors(true); // with build mix events
	}
}

void Drumtrack::CloneData(Drumtrack *dt)
{
	strcpy(dt->name,name);

	dt->MIDIoutdevice=MIDIoutdevice;
	dt->dtr_channel=dtr_channel;
	dt->key=key;
	dt->ticklength=ticklength;
	dt->mute=mute;
	dt->velocityoff=velocityoff;
	dt->volume=volume;
}

bool Drumtrack::CheckEvent(Seq_Event *seqevent)
{
	if(seqevent->GetICD()==ICD_TYPE_DRUM)
	{
		ICD_Drum *drum=(ICD_Drum *)seqevent;

		if(drum->drumtrack==this)
			return true;
	}

	return false;
}

void Drumtrack::SetName(char *nname)
{
	if(nname)
	{
		if(strlen(nname)>DRUMTRACKNAME_LENGTH)
		{
			strncpy(name,nname,DRUMTRACKNAME_LENGTH);
			name[DRUMTRACKNAME_LENGTH]=0;
		}
		else
			strcpy(name,nname);
	}

	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(w->WindowSong()==map->song)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_DRUM:
				{
					Edit_Drum *d=(Edit_Drum *)w;

					w->DrawDBBlit(d->tracks);
				}
				break;

			}

		}

		w=w->NextWindow();
	}

}

void Drumtrack::SetChannel(int nchannel)
{
	if(nchannel>0 && nchannel<=16)
		dtr_channel=nchannel;
}

void Drumtrack::SetKey(int nkey)
{
	if(key>=0 && key<=127)
	{
		key=nkey;
	}
}

void Drumtrack::SetVelocity(int nvelo)
{
	if(nvelo>-127 && nvelo<=127)
	{
		volume=nvelo;
	}
}


Drumtrack::Drumtrack(Drummap *m)
{
	map=m;

	MIDIoutdevice=0; // use track device
	dtr_channel=10;
	key=60;
	ticklength=TICK32nd;
	velocityoff=127;
	volume=0;

	mute=false;
	solo=false;

	strcpy(name,"Drum");

	colour=COLOUR_WHITE;
}


void Drumtrack::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);

	file->Read_ChunkString(name);
	file->ReadChunk(&dtr_channel);
	file->ReadChunk(&key);
	file->ReadChunk(&mute);
	file->ReadChunk(&ticklength);
	file->ReadChunk(&velocityoff);
	file->ReadChunk(&colour);
	file->ReadChunk(&volume);

	file->CloseReadChunk();
}

void Drumtrack::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_DRUMTRACK);

	file->Save_Chunk((CPOINTER)this);
	file->Save_ChunkString(name);
	file->Save_Chunk(dtr_channel);
	file->Save_Chunk(key);
	file->Save_Chunk(mute);
	file->Save_Chunk(ticklength);
	file->Save_Chunk(velocityoff);
	file->Save_Chunk(colour);
	file->Save_Chunk(volume);

	file->CloseChunk();
}

void Drummap::Load(camxFile *file)
{
	int tracks=0;

	file->ReadAndAddClass((CPOINTER)this);
	file->Read_ChunkString(name);
	file->ReadChunk(&tracks);
	readcheck=false;
	file->ReadChunk(&readcheck);

	file->CloseReadChunk();

	if(readcheck==false)
		return;

	while(tracks--)
	{
		file->LoadChunk();

		if(file->CheckReadChunk()==false)
			break;

		if(file->GetChunkHeader()==CHUNK_DRUMTRACK)
		{
			file->ChunkFound();

			if(Drumtrack *dt=CreateDrumTrack(-1))
				dt->Load(file);
			else
				file->CloseReadChunk();
		}
	}

	SetTrackIndexs();
}

void Drummap::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_DRUMMAP);
	file->Save_Chunk((CPOINTER)this);
	file->Save_ChunkString(name);
	file->Save_Chunk(tracks.GetCount());
	bool check=true;
	file->Save_Chunk(check);
	file->CloseChunk();

	// Tracks...
	Drumtrack *t=FirstTrack();
	while(t)
	{
		t->Save(file);
		t=t->NextTrack();
	}
}

// class GMPercussionMap()
void Drummap::InitGMDrumMap()
{
	DeleteAllDrumTracks();

	strcpy(name,"GM Map");
	// Init General MIDI Drummap

	GMMap gmmap;

	for(int i=35+0;i<35+47;i++)
	{
		Drumtrack *nt=CreateDrumTrack(-1); 

		if(nt)
		{
			nt->key=i;
			strncpy(nt->name,gmmap.keys[i],DRUMTRACKNAME_LENGTH);
		}
	}

	SetTrackIndexs();
}

void Drummap::FreeMemory()
{
	DeleteAllDrumTracks();
}

char *Drummap::GetName()
{
	if(strlen(name)==0)
		return "DM";
	return name;
}

bool Drummap::SetName(char *string)
{
	bool ok=true;

	if(string)
	{
		char *newdrummapname=new char[STANDARDSTRINGLEN+2];

		if(newdrummapname)
		{
			if(strlen(string)<1)
			{
				strcpy(newdrummapname,"DM");
				ok=false;
			}
			else
			{
				if(strlen(string)>STANDARDSTRINGLEN-4)
				{
					strncpy(newdrummapname,string,STANDARDSTRINGLEN-4);
					newdrummapname[STANDARDSTRINGLEN-4]=0;
					ok=false;
				}
				else
					strcpy(newdrummapname,string);
			}

			// Clear String ASCII
			size_t sl=strlen(newdrummapname);
			for(size_t i=0;i<sl;i++)
			{
				if((newdrummapname[i]>=48 && newdrummapname[i]<=57) ||
					(newdrummapname[i]>=65 && newdrummapname[i]<=90) ||
					(newdrummapname[i]>=97 && newdrummapname[i]<=122) ||
					newdrummapname[i]==32
					)
				{
				}
				else
				{
					newdrummapname[i]='_';
					ok=false;
				}
			}


			strcpy(name,newdrummapname); // Song Map
			delete newdrummapname;
		}
	}

	return ok;
}

/*
void mainDrumMap::LoadDrumMaps(camxFile *file)
{
long nrmaps=0;
file->ReadChunk(&nrmaps);
file->CloseReadChunk();

while(nrmaps--)
{
file->LoadChunk();

if(file->CheckReadChunk()==false)
break;

if(file->GetChunkHeader()==CHUNK_DRUMMAP)
{
file->ChunkFound();

if(Drummap *ndmap=AddDrummap())
ndmap->Load(file);
else
file->CheckReadChunk();
}
}
}

*/

/*
void mainDrumMap::SaveDrumMaps(camxFile *file)
{
if(FirstDrummap())
{
file->OpenChunk(CHUNK_DRUMMAPHEADER);
file->Save_Chunk(drummaps.GetCount());
file->CloseChunk();

Drummap *d=FirstDrummap();
while(d)
{
d->Save(file);	
d=d->NextMap();
}
}
}
*/