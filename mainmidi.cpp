#include "MIDIhardware.h"
#include "songmain.h"
#include "groove.h"
#include "gui.h"
#include "trackfx.h"
#include "quantizeeditor.h"
#include "chunks.h"
#include "MIDIindevice.h"
#include "MIDIoutdevice.h"
#include "MIDIoutproc.h"

void mainMIDIBase::InitDefaultPorts()
{
	MIDIInputDevice *indev=FirstMIDIInputDevice();
	MIDIOutputDevice *outdev=FirstMIDIOutputDevice();

	bool inautoset=false,outautoset=false;

	for(int i=0;i<MAXMIDIPORTS;i++)
	{
		// Input
		
		// Restore old Device
		if(MIDIinports[i].inputdevice==0 && MIDIinports[i].idevicename && MIDIinports[i].autoset==false)
		{
			MIDIInputDevice *ci=FirstMIDIInputDevice();

			while(ci)
			{
				if(ci->name && strcmp(ci->name,MIDIinports[i].idevicename)==0)
				{
					MIDIinports[i].inputdevice=ci;

					if(ci==indev)
						indev=indev->NextInputDevice();

					if(!indev)
					{
						inautoset=true;
						indev=FirstMIDIInputDevice();
					}

					break;
				}

				ci=ci->NextInputDevice();
			}
		}

		if(MIDIinports[i].inputdevice==0 && indev)
		{
			MIDIinports[i].SetDevice(indev,false);
			MIDIinports[i].autoset=inautoset;

			indev=indev->NextInputDevice();

			if(!indev)
			{
				inautoset=true;
				indev=FirstMIDIInputDevice();
			}
		}

		//Output
		if(MIDIoutports[i].outputdevice==0 && MIDIoutports[i].odevicename && MIDIoutports[i].autoset==false)
		{
			MIDIOutputDevice *oi=FirstMIDIOutputDevice();

			while(oi)
			{
				if(oi->name && strcmp(oi->name,MIDIoutports[i].odevicename)==0)
				{
					MIDIoutports[i].outputdevice=oi;

					if(outdev==oi)
					{
						outdev=outdev->NextOutputDevice();
						if(!outdev)
						{
							outautoset=true;
							outdev=FirstMIDIOutputDevice();
						}
					}
					break;
				}

				oi=oi->NextOutputDevice();
			}
		}

		if(MIDIoutports[i].outputdevice==0 && outdev)
		{
			MIDIoutports[i].SetDevice(outdev,false);
			MIDIoutports[i].autoset=outautoset;

			outdev=outdev->NextOutputDevice();
			if(!outdev)
			{
				outautoset=true;
				outdev=FirstMIDIOutputDevice();
			}
		}

	}
}

void mainMIDIBase::Init()
{
		// MIDI
	maingui->SetInfoWindowText("Init MIDI Out");
	CollectMIDIOutputDevices();

	maingui->SetInfoWindowText("Init MIDI In");
	CollectMIDIInputDevices();

	// Settings->Devices
	MIDIOutputDevice *o=FirstMIDIOutputDevice();
	while(o)
	{
		o->displaynoteoff_monitor=mainsettings->displaynoteoff_monitor;
		o=o->NextOutputDevice();
	}

	MIDIInputDevice *i=FirstMIDIInputDevice();
	while(i)
	{
		i->displaynoteoff_monitor=mainsettings->displaynoteoff_monitor;
		i=i->NextInputDevice();
	}

	InitDefaultPorts(); // Device<->Ports

	MIDIoutproc->Init(); // Init Device SubThreads

	init=true;
}

void mainMIDIBase::DeInit()
{
	init=false;

	maingui->SetInfoWindowText("Close MIDI Hardware...");
	// MIDI
	
	RemoveMIDIInputDevices();
	RemoveMIDIOutputDevices();
}

mainMIDIBase::mainMIDIBase()
{
	init=false;

	keyboard_inputdevice=0;
	generator_inputdevice=0;

	MIDI_inputimpulse=MIDI_outputimpulse=0;

	//global MIDI Thru
	MIDIthru=true;
	MIDIthru_notes=true;
	MIDIthru_polypress=true;
	MIDIthru_controlchange=true;
	MIDIthru_programchange=true;
	MIDIthru_channelpress=true;
	MIDIthru_pitchbend=true;
	MIDIthru_sysex=false;

	MIDIoutactive=true;
	MIDIinactive=true;

	sendnoteprevcylce=true;

	baudrate=31250;

	lastsendevent_device=0;
	defaultMIDIOutputDevice=0;

	sendcontrolsolomute=true;

	receiveMIDIstart=RECEIVEMIDISTART_PLAYBACK;
	receiveMIDIstop=true;
	quantizesongpositiontoMIDIclock=true;
	
	for(int i=0;i<6;i++)
	{
		MIDIinports[i].visible=true;
		MIDIoutports[i].visible=true;
	}
}

void mainMIDIBase::ChangeMIDIInputDeviceUserInfo(MIDIInputDevice *device,char *s)
{
	strcpy(device->userinfo,s);
	device->CreateFullName();
}

void mainMIDIBase::ChangeMIDIOutputDeviceUserInfo(MIDIOutputDevice *device,char *s)
{
	strcpy(device->userinfo,s);
	device->CreateFullName();
}

MIDIOutputDevice *mainMIDIBase::FindMIDIOutputDevice(char *name)
{
	if(name){
		MIDIOutputDevice *s=FirstMIDIOutputDevice();

		while(s){
			if(strcmp(s->FullName(),name)==0)return s;
			s=s->NextOutputDevice();
		}
	}
	return 0;
}

MIDIInputDevice* mainMIDIBase::FindMIDIInputDevice(char *name)
{
	if(name){
		MIDIInputDevice *s=FirstMIDIInputDevice();

		while(s){
			if(strcmp(s->FullName(),name)==0)return s;
			s=s->NextInputDevice();
		}
	}

	return 0;
}

void mainMIDIBase::LoadDevices(camxFile *file)
{
	file->CloseReadChunk();

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_MIDIOUTDEVICES)
	{
		CPOINTER old;
		int devnr=0;

		file->ChunkFound();
		file->ReadChunk(&devnr);
		file->CloseReadChunk();

		while(devnr--)
		{
			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_MIDIOUTDEVICE)
			{
				MIDIOutputDevice *founddevice=0;
				file->ChunkFound();
				file->ReadChunk(&old);

				int l=0;
				file->ReadChunk(&l); // name

				if(l)
				{
					char *dname=new char[l];

					if(dname)
					{
						file->ReadChunk(dname,l);

						MIDIOutputDevice *c=FirstMIDIOutputDevice();

						while(c)
						{
							if(strcmp(c->name,dname)==0)
							{
								founddevice=c;
								break;
							}

							c=c->NextOutputDevice();
						}

						// Refresh Pointer
						Pointer *p=file->FirstPointer();
						while(p)
						{
							if(p->oldpointer==old)
							{
								p->autorefresh=false; 
								MIDIOutputDevice **dev=(MIDIOutputDevice **)p->newpointer;
								*dev=founddevice;
							}

							p=p->NextPointer();
						}

						delete dname;
					}
				}

				file->CloseReadChunk();
			}
		}
	}

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_MIDIINDEVICES)
	{
		CPOINTER old;
		int devnr=0;
		MIDIInputDevice *founddevice=0;

		file->ChunkFound();
		file->ReadChunk(&devnr);

		while(devnr--)
		{
			if(file->CheckReadChunk()==false)
				break;

			file->ReadChunk(&old);

			int l=0;
			file->ReadChunk(&l); // name

			if(l)
			{
				char *dname=new char[l];

				if(dname)
				{
					file->ReadChunk(dname,l);

					MIDIInputDevice *c=FirstMIDIInputDevice();

					while(c)
					{
						if(strcmp(c->name,dname)==0)
						{
							founddevice=c;
							break;
						}

						c=c->NextInputDevice();
					}

					// Refresh Pointer
					Pointer *p=file->FirstPointer();
					while(p )
					{
						if(p->oldpointer==old)
						{
							p->autorefresh=false;
							MIDIInputDevice **dev=(MIDIInputDevice **)p->newpointer;
							*dev=founddevice;
						}

						p=p->NextPointer();
					}

					delete dname;
				}
			}
		}

		file->CloseReadChunk();
	}
}

void mainMIDIBase::SaveDevices(camxFile *file)
{
	file->OpenChunk(CHUNK_MIDIDEVICES);
	file->CloseChunk();

	file->OpenChunk(CHUNK_MIDIOUTDEVICES); // Save Output Device Names
	file->Save_Chunk(outputdevices.GetCount());
	file->CloseChunk();

	{
		MIDIOutputDevice *f=FirstMIDIOutputDevice();
		while(f)
		{
			file->OpenChunk(CHUNK_MIDIOUTDEVICE);
			file->Save_Chunk((CPOINTER)f);

			int l=strlen(f->name)+1;
			file->Save_Chunk(l);
			file->Save_Chunk(f->name,l);
			file->CloseChunk();

			f->Save(file);

			f=f->NextOutputDevice();
		}
	}
	
	// MIDI Input Devices
	file->OpenChunk(CHUNK_MIDIINDEVICES); // Save Input Devices Names

	file->Save_Chunk(inputdevices.GetCount());

	{
		MIDIInputDevice *i=FirstMIDIInputDevice();
		while(i)
		{
			file->Save_Chunk((CPOINTER)i);

			int l=strlen(i->name)+1;

			file->Save_Chunk(l);
			file->Save_Chunk(i->name,l);

			i=i->NextInputDevice();
		}
	}

	file->CloseChunk();
}

int mainMIDIBase::LoadGrooves(camxFile *file)
{
	int c=0;
	int grnr=0;
	file->ReadChunk(&grnr);
	file->CloseReadChunk();

	while(grnr--)
	{
		file->LoadChunk();

		if(file->GetChunkHeader()==CHUNK_GROOVE)
		{
			file->ChunkFound();

			if(Groove *groove=new Groove(0,0))
			{
				groove->Load(file);
				mainMIDI->AddGroove(groove);
				c++;
			}
			else
				file->CloseReadChunk();
		}
	}

	return c;
}

void mainMIDIBase::SaveGrooves(camxFile *file)
{
	if(FirstGroove())
	{
		file->OpenChunk(CHUNK_MIDIGROOVES);

		int grnr=GetNrGrooves();
		file->Save_Chunk(grnr);
		file->CloseChunk();

		// Save Grooves
		Groove *g=FirstGroove();
		while(g)
		{
			g->Save(file);
			g=g->NextGroove();
		}
	}
}
