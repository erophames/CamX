#include "songmain.h"
#include "MIDIhardware.h"
#include "MIDIindevice.h"
#include "MIDIoutdevice.h"
#include "semapores.h"
#include "object_song.h"
#include "object_track.h"
#include "settings.h"
#include "chunks.h"
#include "gui.h"
#include "camxfile.h"

#ifdef WIN32
#include <stdio.h>
#include <mmsystem.h>
#endif

void mainMIDIBase::CheckNewEventData(Seq_Song *song,NewEventData *input,Seq_Event *e)
{
	if((!song) || (!e))return;

	Seq_MIDIRouting_InputDevice *inputdevice=input->fromdev?song->inputrouting.FindInputDevice(input->fromdev):0;

	if((!input->fromdev) || inputdevice)
	{
		Seq_Track *t=song->FirstTrack();

		while(t){

			bool checkfocustrack=t->GetFX()->usealwaysthru==true?false:true;

			if(
				(checkfocustrack==false || t==song->GetFocusTrack()) &&
				(t->GetFX()->userouting==false || (!input->fromdev) || inputdevice->CheckEvent(t,e,checkfocustrack)==true)
				)
			{
				if(t->CheckMIDIInputEvent(input->fromdev,e)==true)
				{
					song->NewEventInput(input,e);
					return;
				}
			}

			t=t->NextTrack();
		}// 
	}
}

// ---- Output -------------------------
bool mainMIDIBase::CheckIfSysexActive()
{
	MIDIOutputDevice *d=FirstMIDIOutputDevice();
	while(d)
	{
		for(int i=0;i<DATABUFFERSIZE;i++)
			if(d->datasysex[i])
				return true;

		d=d->NextOutputDevice();
	}
	return false;
}

MIDIOutputDevice *mainMIDIBase::InitOutputDevice(char *name,int deid)
{
	if(name)
	{
		if(MIDIOutputDevice *d = new MIDIOutputDevice)
		{
			// Find Device with same Name
			int idck=0;
			MIDIOutputDevice *ckdev=FirstMIDIOutputDevice();

			while(ckdev)
			{
				if(ckdev->initname && strcmp(ckdev->initname,name)==0)
					idck++;

				ckdev=ckdev->NextOutputDevice();
			}

			if(idck>0)
			{
				char h2[NUMBERSTRINGLEN];
				d->name=mainvar->GenerateString(name,"_",mainvar->ConvertIntToChar(idck,h2));
			}
			else
				d->name=mainvar->GenerateString(name);

			d->initname=mainvar->GenerateString(name);
			d->id=deid;

			/*
			if(strlen(name)<SMALLSTRINGLEN-1)
			strcpy(d->name,name);
			else
			{
			strncpy (d->name,name,SMALLSTRINGLEN-1);
			d->name[SMALLSTRINGLEN-1]=0;
			}
			*/

			// Settings of Device

			// no Clock or Start/Stop on (all)Ports
			size_t i=strlen(name);
			char *ck=name;
			while(i-->3)
			{
				if(*ck=='a' && *(ck+1)=='l' && *(ck+2)=='l')
				{
				//	d->sendMIDIcontrol=false;
					break;
				}

				ck++;
			}

			//d->SendDeviceReset(true);

			return d;
		}
	}

	return 0;
}

MIDIOutputDevice *mainMIDIBase::DeleteOutputDevice(MIDIOutputDevice *dev)
{
	dev->CloseOutputDevice();

	if(dev==defaultMIDIOutputDevice)
		defaultMIDIOutputDevice=dev->NextOutputDevice()?dev->NextOutputDevice():dev->PrevOutputDevice();

	return (MIDIOutputDevice *)outputdevices.RemoveO(dev);
}

void mainMIDIBase::SetDefaultMIDIDevice(char *name)
{
	if(name)
	{
		MIDIOutputDevice *f=FirstMIDIOutputDevice();

		while(f)
		{
			if(strcmp(name,f->name)==0)
			{
				defaultMIDIOutputDevice=f;
				break;
			}

			f=f->NextOutputDevice();
		}
	}
}

void mainMIDIBase::CollectMIDIOutputDevices()
{	
#ifdef WIN32
	MIDIOUTCAPS	moc;
	int iNumDevs = midiOutGetNumDevs();

	for (int i = 0; i < iNumDevs; i++) // -1 == MIDImapper
		if (!midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)))
		{
			MIDIOutputDevice *out=InitOutputDevice(moc.szPname,i);

			if(out)
				outputdevices.AddEndO(out);
		}		
#endif

		MIDIOutputDevice *out=FirstMIDIOutputDevice();
		while(out)
		{
			out->OpenOutputDevice(0);

			if(out->init) // Init ok ?
			{
				if(!defaultMIDIOutputDevice)
					defaultMIDIOutputDevice=out;
			}

			out=out->NextOutputDevice();
		}
}

// ---- Input -------------------------
MIDIInputDevice *mainMIDIBase::AddInputDevice(char *name,int id)
{
	if(name)
	{
		if(MIDIInputDevice *d = new MIDIInputDevice)
		{
			// Find Device with same Name
			int idck=0;
			MIDIInputDevice *ckdev=FirstMIDIInputDevice();

			while(ckdev)
			{
				if(ckdev->initname && strcmp(ckdev->initname,name)==0)
					idck++;

				ckdev=ckdev->NextInputDevice();
			}

			if(idck>0)
			{
				char h2[NUMBERSTRINGLEN];
				d->name=mainvar->GenerateString(name,"_",mainvar->ConvertIntToChar(idck,h2));
			}
			else
				d->name=mainvar->GenerateString(name);

			d->initname=mainvar->GenerateString(name);



			/*
			if(strlen(name)<SMALLSTRINGLEN-1)
			strcpy(d->name,name);
			else
			{
			strncpy (d->name,name,SMALLSTRINGLEN-1);
			d->name[SMALLSTRINGLEN-1]=0;
			}

			*/

			return d;
		}
	}

	return 0;
}

MIDIInputDevice *mainMIDIBase::DeleteInputDevice(MIDIInputDevice *dev)
{
	dev->CloseInputDevice();
	return (MIDIInputDevice *)inputdevices.RemoveO(dev);
}

void mainMIDIBase::RefreshMIDIDevices()
{
#ifdef WIN32
	MIDIINCAPS mic;

	// Go through all of those devices, displaying their names

	{
		int inindex=0;
		MIDIInputDevice *mid=FirstMIDIInputDevice();

		while(mid)
		{
			if(mid->type==MIDIInputDevice::OS_MIDIINTERFACE)
				mid->foundrealtime=false;

			mid=mid->NextInputDevice();
		}
	}

	UINT iNumDevs = midiInGetNumDevs();

	for (UINT i = 0; i < iNumDevs; i++)
	{	
		// Get info about the next device 
		if (!midiInGetDevCaps(i, &mic, sizeof(MIDIINCAPS)))
		{	
			MIDIInputDevice *mid=FirstMIDIInputDevice();

			while(mid)
			{
				if(mid->type==MIDIInputDevice::OS_MIDIINTERFACE)
				{
					if(mid->id==i)
					{
						if(mid->name && mic.szPname && strcmp(mic.szPname,mid->name)==0)
							mid->foundrealtime=true;

						break;
					}
				}

				mid=mid->NextInputDevice();
			}

			/*
			if(MIDIInputDevice *in=AddInputDevice(mic.szPname,i))
			{
			in->type=MIDIInputDevice::OS_MIDIINTERFACE;
			in->id=i;
			inputdevices.AddEndO(in);
			}
			*/
		}
	}	
#endif

	{
		// Device Removed ?
		MIDIInputDevice *mid=FirstMIDIInputDevice();

		while(mid)
		{
			if(mid->type==MIDIInputDevice::OS_MIDIINTERFACE)
			{
				if(mid->foundrealtime==false)
				{
					int i=0;
				}
			}

			mid=mid->NextInputDevice();
		}
	}
}

void mainMIDIBase::CollectMIDIInputDevices()
{
#ifdef WIN32
	MIDIINCAPS mic;

	// Go through all of those devices, displaying their names
	UINT iNumDevs = midiInGetNumDevs();

	for (UINT i = 0; i < iNumDevs; i++)
	{	
		// Get info about the next device 
		if (!midiInGetDevCaps(i, &mic, sizeof(MIDIINCAPS)))
		{
			if(MIDIInputDevice *in=AddInputDevice(mic.szPname,i))
			{
				in->type=MIDIInputDevice::OS_MIDIINTERFACE;
				in->id=i;
				inputdevices.AddEndO(in);
			}
		}
	}	
#endif

	MIDIInputDevice *indev=FirstMIDIInputDevice();
	while(indev)
	{
		indev->OpenInputDevice();
		indev=indev->NextInputDevice();
	}

	// Keyboard
	if(keyboard_inputdevice=AddInputDevice("iKeyboard",0))
	{
		keyboard_inputdevice->type=MIDIInputDevice::CAMX_INTERN;
		keyboard_inputdevice->deviceinit=true;
		inputdevices.AddEndO(keyboard_inputdevice);
	}

	// Generator
	if(generator_inputdevice=AddInputDevice("iGenerator",0))
	{
		generator_inputdevice->type=MIDIInputDevice::CAMX_INTERN;

		generator_inputdevice->deviceinit=true;
		inputdevices.AddEndO(generator_inputdevice);
	}
}

void mainMIDIBase::ResetMIDIInputTimer()
{
	MIDIInputDevice *i=FirstMIDIInputDevice();

	while(i)
	{
		i->StartInputTime();	
		i=i->NextInputDevice();
	}
}

/*
void mainMIDIBase::ResetMIDIInClocks(Seq_Song *song)
{
MIDIInputDevice *i=FirstMIDIInputDevice();

while(i)
{
i->inputMIDIclockcounter=0; // reset MIDI Clock
i=i->NextInputDevice();
}
}
*/

void mainMIDIBase::ResetMIDIInTimer(bool setrecordtimer)
{	
	//MIDIinproc->Lock(); // No Locks !!! Else Dead Lock Audio/MIDI Sync

	if(setrecordtimer==true)
		ResetMIDIInputTimer();

	//ResetMIDIInClocks(song);
	//MIDIinproc->Unlock();
}

// ---- main
void mainMIDIBase::RemoveMIDIOutputDevices()
{
	MIDIOutputDevice *o=FirstMIDIOutputDevice();

	while(o)
		o=DeleteOutputDevice(o);
}

/*
void mainMIDIBase::EnableMIDIOutFilter()
{
MIDIOutputDevice *o=FirstMIDIOutputDevice();

while(o)
{
o->MIDIoutputfilter=true;
o=o->NextOutputDevice();
}
}

void mainMIDIBase::DisableMIDIOutFilter() // Default
{
MIDIOutputDevice *o=FirstMIDIOutputDevice();

while(o)
{
o->MIDIoutputfilter=false;
o=o->NextOutputDevice();
}
}
*/

void mainMIDIBase::RemoveMIDIInputDevices()
{
	MIDIInputDevice *i=FirstMIDIInputDevice();

	while(i)
		i=DeleteInputDevice(i);
}

void mainMIDIBase::StopMIDIInputDevices()
{
	MIDIInputDevice *i=FirstMIDIInputDevice();

	while(i){

		if(char *h=mainvar->GenerateString("Stop Hardware...MIDI Input ",i->fullname?i->fullname:"?"))
		{
			maingui->SetInfoWindowText(h);
			delete h;
		}

		i->StopMIDIDevice();

		i=i->NextInputDevice();
	}
}

void mainMIDIBase::SetReceiveMIDIStart(int flag)
{
	if(flag!=receiveMIDIstart)
	{
		receiveMIDIstart=flag;
		mainsettings->Save(0);
	}
}

void mainMIDIBase::LoadPorts(camxFile *file)
{
	file->ChunkFound();
	file->CloseReadChunk();

	// Input
	for(int i=0;i<MAXMIDIPORTS;i++)
	{
		file->LoadChunk();

		if(file->GetChunkHeader()==CHUNK_SETTINGSMIDIINPUTPORT)							
		{
			file->ChunkFound();

			file->Read_ChunkString(&MIDIinports[i].idevicename);
			file->Read_ChunkString(MIDIinports[i].info);
			file->ReadChunk(&MIDIinports[i].autoset);
			file->ReadChunk(&MIDIinports[i].visible);

			file->ReadChunk(&MIDIinports[i].receivesync);
			file->ReadChunk(&MIDIinports[i].receivemtc);

			file->CloseReadChunk();

			MIDIinports[i].filter.Load(file);
		}
	}

	// Output
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_SETTINGSMIDIOUTPUTPORTS)
	{
		file->ChunkFound();
		file->CloseReadChunk();

		for(int i=0;i<MAXMIDIPORTS;i++)
		{
			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_SETTINGSMIDIOUTPUTPORT)							
			{
				file->ChunkFound();

				file->Read_ChunkString(&MIDIoutports[i].odevicename);
				file->Read_ChunkString(MIDIoutports[i].info);
				file->ReadChunk(&MIDIoutports[i].autoset);
				file->ReadChunk(&MIDIoutports[i].visible);
				file->ReadChunk(&MIDIoutports[i].sendsync);
				file->ReadChunk(&MIDIoutports[i].sendmtc);

				file->CloseReadChunk();

				MIDIoutports[i].filter.Load(file);
			}
		}
	}
}

void mainMIDIBase::SavePorts(camxFile *file)
{
	file->OpenChunk(CHUNK_SETTINGSMIDIINPUTPORTS);
	file->CloseChunk();

	// Input
	for(int i=0;i<MAXMIDIPORTS;i++)
	{
		file->OpenChunk(CHUNK_SETTINGSMIDIINPUTPORT);

		file->Save_ChunkString(MIDIinports[i].idevicename);
		file->Save_ChunkString(MIDIinports[i].info);
		file->Save_Chunk(MIDIinports[i].autoset);
		file->Save_Chunk(MIDIinports[i].visible);

		file->Save_Chunk(MIDIinports[i].receivesync);
		file->Save_Chunk(MIDIinports[i].receivemtc);

		file->CloseChunk();

		MIDIinports[i].filter.Save(file);
	}

	file->OpenChunk(CHUNK_SETTINGSMIDIOUTPUTPORTS);
	file->CloseChunk();

	// Output
	for(int i=0;i<MAXMIDIPORTS;i++)
	{
		file->OpenChunk(CHUNK_SETTINGSMIDIOUTPUTPORT);

		file->Save_ChunkString(MIDIoutports[i].odevicename);
		file->Save_ChunkString(MIDIoutports[i].info);
		file->Save_Chunk(MIDIoutports[i].autoset);
		file->Save_Chunk(MIDIoutports[i].visible);
		file->Save_Chunk(MIDIoutports[i].sendsync);
		file->Save_Chunk(MIDIoutports[i].sendmtc);

		file->CloseChunk();

		MIDIoutports[i].filter.Save(file);
	}

}

MIDIInPort::MIDIInPort()
{
	inputdevice=0;
	idevicename=0;
	fullname=0;
	info[0]=0;
	autoset=false;
	visible=false;
	receivesync=true;
	receivemtc=true;
}

void MIDIInPort::SetInfo(char *i)
{
	if(!i)
		return;

	if(strlen(i)>31)
	{
		strncpy(info,i,31);
		info[31]=0;
	}
	else
		strcpy(info,i);
}

char *MIDIInPort::GetName()
{
	if(fullname)
		delete fullname;

	if(strlen(info)>0)
	{
		fullname=mainvar->GenerateString(idevicename?idevicename:"ID","/",info);
		return fullname;
	}

	return idevicename?idevicename:"ID";
}

void MIDIInPort::SetDevice(MIDIInputDevice *in,bool user)
{
	if(idevicename)
		delete idevicename;

	idevicename=0;
	inputdevice=in;

	if(user==true)
		autoset=false;

	if(inputdevice)
		idevicename=mainvar->GenerateString(inputdevice->name);
}

MIDIOutputPort::MIDIOutputPort()
{
	outputdevice=0;
	odevicename=0;
	fullname=0;
	info[0]=0;
	autoset=false;
	visible=false;
	sendsync=true;
	sendmtc=false;
}

MIDIOutputPort::~MIDIOutputPort()
{
	if(odevicename)
		delete odevicename;

	if(fullname)
		delete fullname;
}

void MIDIOutputPort::SetDevice(MIDIOutputDevice *out,bool user)
{
	if(odevicename)
		delete odevicename;

	odevicename=0;

	outputdevice=out;

	if(user==true)
		autoset=false;

	if(outputdevice)
		odevicename=mainvar->GenerateString(outputdevice->name);
}

void MIDIOutputPort::SetInfo(char *i)
{
	if(!i)
		return;

	if(strlen(i)>31)
	{
		strncpy(info,i,31);
		info[31]=0;
	}
	else
		strcpy(info,i);
}

char *MIDIOutputPort::GetName()
{
	if(fullname)
		delete fullname;

	if(strlen(info)>0)
	{
		fullname=mainvar->GenerateString(odevicename?odevicename:"OD","/",info);
		return fullname;
	}

	return odevicename?odevicename:"OD";
}