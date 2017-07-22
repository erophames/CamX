#ifndef CAMX_LMIDIEVENT
#define CAMX_LMIDIEVENT 1

#define MAXMONITORINLIST 2048
#define MAXMONITOREVENTS 64 // MIDI Monitor I/O
#define MAXMONITORBYTES 16

class MIDIOutputDevice;
class MIDIInputDevice;
class Seq_Track;

class LMIDIEvents:public Object
{
public:
	LMIDIEvents()
	{
		plugin=0;
		outdevice=0;
		indevice=0;
		track=0;

		data[0]=0;
		datalength=data_l_length=0;
	}

	void FreeData()
	{
		data[0]=0;
		if(plugin){delete plugin;plugin=0;}
	}

	void Init(UBYTE status,UBYTE b1,UBYTE b2,int datalen,int data_l_len)
	{
		data[0]=status;
		data[1]=b1;
		data[2]=b2;
		datalength=datalen;
		data_l_length=data_l_len;
	}

	bool Compare(LMIDIEvents *);
	void Clone(LMIDIEvents *);

	inline void SetData(UBYTE status,char b1,char b2){data[0]=status;data[1]=b1;data[2]=b2;datalength=data_l_length=0;}
	LMIDIEvents *PrevEvent(){return (LMIDIEvents *)prev;}
	LMIDIEvents *NextEvent(){return (LMIDIEvents *)next;}

	Seq_Track *track;
	char *plugin;
	MIDIOutputDevice *outdevice;
	MIDIInputDevice *indevice;
	UBYTE data[MAXMONITORBYTES+2];
	int datalength,data_l[4],data_l_length,x,y,x2,y2; // GUI Pos
};

class LMIDIList
{
public:
	LMIDIList(){counter=0;}

	void DeleteAllO()
	{
		LMIDIEvents *e=FirstEvent();
		while(e){
			e->FreeData();
			e=(LMIDIEvents *)lMIDIevents.RemoveO(e);
		}

		counter=0;
	}

	LMIDIEvents *FirstEvent(){return(LMIDIEvents *)lMIDIevents.GetRoot();}
	LMIDIEvents *LastEvent(){return(LMIDIEvents *)lMIDIevents.Getc_end();}

	void AddEvent(LMIDIEvents *e)
	{
		lMIDIevents.AddEndO(e);

		if(lMIDIevents.GetCount()>MAXMONITORINLIST){
			LMIDIEvents *e=FirstEvent();
			e->FreeData();
			lMIDIevents.RemoveO(e);
		}

		counter++;
	}

	LMIDIEvents *GetO(int index){return (LMIDIEvents *)lMIDIevents.GetO(index);}
	int GetCount(){return counter;}

	OList lMIDIevents;
	int counter;
};
#endif