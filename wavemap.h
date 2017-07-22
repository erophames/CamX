#ifndef CAMX_WAVEMAP
#define CAMX_WAVEMAP 1

#include "defines.h"
#include "object.h"
#include "objectevent.h"
#include "mempools.h"
#include "gui.h"
#include "object_song.h"

class WaveTrack:public Object
{
	friend class WaveMap;
	
public:
	WaveTrack()
	{
		colour=COLOUR_WHITE;
		id=OBJ_WAVETRACK;
		
		status=0; // undefined
	
		strcpy(name,"-");

		group="Undefined";
		
		info=0;
	
		chl=1;
		maxvalue=127;
		
		idtrack=0;
	}
	
	virtual Seq_Event *CreateNewEvent(Seq_Song *,OSTART start,OSTART end,UBYTE channel,UBYTE value){return 0;}
	
	virtual bool CheckIfEventExists(Seq_Event *e,OSTART pos)
	{
		if(e->GetEventStart()==pos)
			return CheckIfEventInside(e);
		
		return false;
	}
	
	virtual bool CheckIfEventInside(Seq_Event *e)
	{
		if(
			(e->GetStatus()==status) &&
			((!chl) || e->GetChannel()==(chl-1))
			)
			return true;
		
		return false;
	}
	
	virtual char *CreateData1Name(Seq_Song *song){return 0;}
	virtual char *CreateData2Name(Seq_Song *song){return 0;}

	virtual int GetMSB(Seq_Event *e){return -1;}
	virtual bool SetMSB(Seq_Event *e,int value){return false;}
	
	virtual WaveTrack *CloneTrack(){return 0;}
	
	WaveMap *GetMap(){return (WaveMap *)GetList();}

	void Load(camxFile *);
	void Save(camxFile *);

	void FreeMemory();

	WaveTrack *idtrack;
	
	int type;
	int colour;
	int maxvalue;
	
	char name[STANDARDSTRINGLEN+2];
	
	char *info;
	char *group;

	// MIDI 
	UBYTE status;
	
	UBYTE chl; // 0-15, or 16==ALL
	UBYTE byte1; // 0-127, or 128==ALL
	UBYTE byte2; // 0-127, or 128==ALL
	
	// UBYTE controlnumber;
	
	WaveTrack *PrevTrack() {return(WaveTrack *)prev; }
	WaveTrack *NextTrack() {return(WaveTrack *)next; }
};

// Note
class Edit_WaveTrack_Note:public WaveTrack
{
public:
		Edit_WaveTrack_Note()
	{
		strcpy(name,"Note");

		group="MIDI";

		status=NOTEON;

		byte1=128; // ALL
		velocityoff=0;
		notelength=16;
	}
	
	WaveTrack *CloneTrack() //v
	{
		Edit_WaveTrack_Note *n=new Edit_WaveTrack_Note;
		
		if(n)
		{
			n->idtrack=this;
			n->chl=chl;
			n->byte1=byte1; // key
			n->velocityoff=velocityoff;
			n->notelength=notelength;
		}
		
		return n;
	}
	
	char *CreateData1Name(Seq_Song *song)
	{
		if(byte1==128) // ALL
			return "All Keys";
		
		return maingui->ByteToKeyString(song,byte1);
	}
	
	Seq_Event *CreateNewEvent(Seq_Song *song,OSTART start,OSTART end,UBYTE channel,UBYTE value) // v
	{
		if(byte1<128) // != ALL
		{
#ifdef MEMPOOLS
			Note *note=mainpools->mempGetNote();
#else
			Note *note=new Note;
#endif

			if(note)
			{
				note->status=NOTEON|channel;
				note->key=byte1;
				note->velocity=value;
				note->velocityoff=velocityoff;
				note->off.staticostart=note->off.ostart=start+notelength;
			}
			
			return note;
		}
		
		return 0;
	}
	
	bool CheckIfEventInside(Seq_Event *e)
	{
		if(
			(e->GetStatus()==status) &&
			((!chl) || e->GetChannel()==(chl-1)) &&
			(byte1==128 || (byte1==((Note *)e)->key))
		)
		return true;
		
		return false;
	}

	bool CheckIfEventExists(Seq_Event *e,OSTART pos)
	{
		if(e->GetEventStart()==pos &&
			e->GetStatus()==status &&
			((!chl) || e->GetChannel()==(chl-1))
			)
		{
			if(((Note *)e)->key==byte1)
				return true;
		}
		
		return false;
	}					
	
	int GetMSB(Seq_Event *e)
	{
		return ((Note *)e)->velocity;
	}
	
	bool SetMSB(Seq_Event *e,int value)
	{
		if(e->GetStatus()==NOTEON)
		{
			Note *c=(Note *)e;
			
			if(value!=c->velocity)
			{
				c->velocity=(UBYTE)value;
				
				return true;
			}
		}
		
		return false;
	}

	// Standard
	int notelength;
	UBYTE velocityoff;
};

class Edit_WaveTrack_Pitchbend:public WaveTrack
{
public:
	Edit_WaveTrack_Pitchbend()
	{
		status=PITCHBEND;

		strcpy(name,"Pitchbend");
		group="MIDI";
	}
	
	WaveTrack *CloneTrack() //v
	{
		Edit_WaveTrack_Pitchbend *n=new Edit_WaveTrack_Pitchbend;
		
		if(n)
		{
			n->chl=chl;
			n->idtrack=this;
			n->byte1=byte1;
			n->byte2=byte2;
		}
		
		return n;
	}
	
	Seq_Event *CreateNewEvent(Seq_Song *song,OSTART start,OSTART end,UBYTE channel,UBYTE value) // v
	{
#ifdef MEMPOOLS
		Pitchbend *p=mainpools->mempGetPitchbend();
#else
		Pitchbend *p=new Pitchbend;
#endif

		if(p)
		{
			p->status=PITCHBEND|channel;
			p->lsb=byte1;
			p->msb=value;
		}
		
		return p;
	}
			
	int GetMSB(Seq_Event *e)
	{
		return ((Pitchbend *)e)->msb;
	}
	
	bool SetMSB(Seq_Event *e,int value)
	{
		if(e->GetStatus()==PITCHBEND)
		{
			Pitchbend *c=(Pitchbend *)e;
			
			if(value!=c->msb)
			{
				c->msb=(UBYTE)value;
				
				return true;
			}
		}
		
		return false;
	}
};

class Edit_WaveTrack_ChannelPressure:public WaveTrack
{
public:
	Edit_WaveTrack_ChannelPressure()
	{
		status=CHANNELPRESSURE;

		group="MIDI";

		strcpy(name,"ChannelPressure");
	}
	
	WaveTrack *CloneTrack() //v
	{
		Edit_WaveTrack_ChannelPressure *n=new Edit_WaveTrack_ChannelPressure;
		
		if(n)
		{
			n->chl=chl;
			n->idtrack=this;
		}
		
		return n;
	}
	
	Seq_Event *CreateNewEvent(Seq_Song *song,OSTART start,OSTART end,UBYTE channel,UBYTE value) // v
	{
		// ChannelPressure *cp=mainpools->mempGetChannelPressure();
		ChannelPressure *cp=new ChannelPressure();

		if(cp)
		{
			cp->status=CHANNELPRESSURE|channel;
			cp->pressure=value;
		}
		
		return cp;
	}
	
	int GetMSB(Seq_Event *e)
	{
		return ((ChannelPressure *)e)->pressure;
	}
	
	bool SetMSB(Seq_Event *e,int value)
	{
		if(e->GetStatus()==CHANNELPRESSURE)
		{
			ChannelPressure *c=(ChannelPressure *)e;
			
			if(value!=c->pressure)
			{
				c->pressure=(UBYTE)value;
				
				return true;
			}
		}
		
		return false;
	}
};

class Edit_WaveTrack_PolyPressure:public WaveTrack
{
public:
	Edit_WaveTrack_PolyPressure()
	{
		status=POLYPRESSURE;

		group="MIDI";

		strcpy(name,"PolyPressure");
		
		byte1=64;
	}
	
	char *CreateData1Name(Seq_Song *song)
	{
		return maingui->ByteToKeyString(song,byte1);
	}

	WaveTrack *CloneTrack() //v
	{
		Edit_WaveTrack_PolyPressure *n=new Edit_WaveTrack_PolyPressure;
		
		if(n)
		{
			n->idtrack=this;
			n->chl=chl;
			n->byte1=byte1;
		}
		
		return n;
	}
	
	Seq_Event *CreateNewEvent(Seq_Song *song,OSTART start,OSTART end,UBYTE channel,UBYTE value) // v
	{
		PolyPressure *cp=new PolyPressure();

		if(cp)
		{
			cp->status=POLYPRESSURE|channel;
			cp->key=byte1;
			cp->pressure=value;
		}
		
		return cp;
	}
	
	bool CheckIfEventExists(Seq_Event *e,OSTART pos)
	{
		if(e->GetEventStart()==pos &&
			e->GetStatus()==status &&
			((!chl) || e->GetChannel()==(chl-1))
			)
		{
			PolyPressure *c=(PolyPressure *)e;
			
			if(c->key==byte1)
				return true;
		}
		
		return false;
	}				

	int GetMSB(Seq_Event *e)
	{
		return ((PolyPressure *)e)->pressure;
	}
	
	bool SetMSB(Seq_Event *e,int value)
	{
		if(e->GetStatus()==POLYPRESSURE)
		{
			PolyPressure *c=(PolyPressure *)e;
			
			if(value!=c->pressure)
			{
				c->pressure=(UBYTE)value;
				
				return true;
			}
		}
		
		return false;
	}
};

class Edit_WaveTrack_Program:public WaveTrack
{
public:
	Edit_WaveTrack_Program()
	{
		status=PROGRAMCHANGE;

		group="MIDI";

		strcpy(name,"ProgramChange");

		program=0;
	}
	
	WaveTrack *CloneTrack() //v
	{
		Edit_WaveTrack_Program *n=new Edit_WaveTrack_Program;
		
		if(n)
		{
			n->idtrack=this;
			n->chl=chl;
		}
		
		return n;
	}
	
	Seq_Event *CreateNewEvent(Seq_Song *song,OSTART start,OSTART end,UBYTE channel,UBYTE value) // v
	{
		ProgramChange *cp=new ProgramChange();

		if(cp)
		{
			cp->status=PROGRAMCHANGE|channel;
			cp->program=value;
		}
		
		return cp;
	}
	
	int GetMSB(Seq_Event *e)
	{
		return ((ProgramChange *)e)->program;
	}
	
	bool SetMSB(Seq_Event *e,int value)
	{
		if(e->GetStatus()==PROGRAMCHANGE)
		{
			ProgramChange *c=(ProgramChange *)e;
			
			if(value!=c->program)
			{
				c->program=(UBYTE)value;
				
				return true;
			}
		}
		
		return false;
	}

	UBYTE program;
};

class Edit_WaveTrack_Control:public WaveTrack
{
public:
	Edit_WaveTrack_Control()
	{
		group="MIDI";
		strcpy(name,"Control Change");

		status=CONTROLCHANGE;
		byte1=7;
	}
	
	char *CreateData1Name(Seq_Song *song)
	{
		return maingui->ByteToControlInfo(byte1,-1,true);
	}

	WaveTrack *CloneTrack() //v
	{
		Edit_WaveTrack_Control *n=new Edit_WaveTrack_Control;
		
		if(n)
		{
			n->idtrack=this;
			n->chl=chl;
			n->byte1=byte1;
		}
		
		return n;
	}

	bool CheckIfEventExists(Seq_Event *e,OSTART pos)
	{
		if(e->GetEventStart()==pos &&
			e->GetStatus()==status &&
				((!chl) || e->GetChannel()==(chl-1))
			)
		{
			ControlChange *c=(ControlChange *)e;
			
			if(c->controller==byte1)
				return true;
		}
		
		return false;
	}				
	
	Seq_Event *CreateNewEvent(Seq_Song *song,OSTART start,OSTART end,UBYTE channel,UBYTE value) // v
	{
		#ifdef MEMPOOLS
		ControlChange *c=mainpools->mempGetControl();
#else
		ControlChange *c=new ControlChange;
#endif

		if(c)
		{
			c->status=CONTROLCHANGE|channel;
			c->controller=byte1;
			c->value=value;
		}
		
		return c;
	}

	int GetMSB(Seq_Event *e)
	{
		return ((ControlChange *)e)->value;
	}
	
	bool SetMSB(Seq_Event *e,int value)
	{
		if(e->GetStatus()==CONTROLCHANGE)
		{
			ControlChange *c=(ControlChange *)e;
			
			if(value!=c->value)
			{
				c->value=(UBYTE)value;
				
				return true;
			}
		}
		
		return false;
	}

};

// End of tracks
class WaveMap:public Object
{
	friend class Edit_Wave;
	friend class mainMIDIBase;
	
public:
	WaveMap()
	{
		id_number=0;
		
		strcpy(name,"Wave Map");

		AddTrackType(&tracktypenote); // Note
		AddTrackType(&tracktypecontrol); // Control
		AddTrackType(&tracktypepitchbend);
		AddTrackType(&tracktypeprogramchange);
		AddTrackType(&tracktypechannelpressure);
		AddTrackType(&tracktypepolypressure);
	}
	
	WaveTrack* FirstTrack() {return (WaveTrack *)tracks.GetRoot();}
	WaveTrack* LastTrack() {return (WaveTrack *)tracks.Getc_end();}

	WaveMap *NextMap() { return (WaveMap*)next;}
	WaveMap *PrevMap() { return (WaveMap*)prev;}

	WaveTrack *AddNewWaveTrack(WaveTrack *prev=0,WaveTrack *nt=0);

	int GetCountOfWaveTracks(){return tracks.GetCount();}
	WaveTrack *GetWaveTrackIndex(int index){return (WaveTrack *)tracks.GetO(index);}

	WaveTrack* DeleteTrack(WaveTrack *t);
	void CloseDefinition();
	
	int id_number;
	
	// Types
	WaveTrack *FirstTrackType(){return (WaveTrack *)tracktypes.GetRoot();};
	
	void Load(camxFile *);
	void Save(camxFile *);

	char name[STANDARDSTRINGLEN+2];

private:
	// Track Types
	Edit_WaveTrack_Note tracktypenote;
	Edit_WaveTrack_Control tracktypecontrol;
	Edit_WaveTrack_Pitchbend tracktypepitchbend;
	Edit_WaveTrack_ChannelPressure tracktypechannelpressure;
	Edit_WaveTrack_PolyPressure tracktypepolypressure;
	Edit_WaveTrack_Program tracktypeprogramchange;
	
	void AddTrackType(WaveTrack *ewt)
	{
		tracktypes.AddEndO(ewt);
	}
	
	OList tracktypes; // Available Tracks
	OList tracks; // Edit_Wave_DefinitonTrack
};

class mainWaveMap
{
public:
	WaveMap* FirstWaveMap() {return (WaveMap *)maps.GetRoot();}
	WaveMap* LastWaveMap() {return (WaveMap *)maps.Getc_end();}
	
	WaveMap* AddWaveMap();
	WaveMap *DeleteWaveMap(WaveMap *);

	void DeleteAllWaveMaps();
	void InitDefaultWaveDefinitions();

	void Load(camxFile *);
	void Save(camxFile *);

private:
	OList maps; 
};

#endif
