#include "MIDIprocessor.h"
#include "proc_chord.h"
#include "proc_appr.h"
#include "proc_delay.h"
#include "semapores.h"
#include "object_track.h"
#include "MIDIoutproc.h"
#include "gui.h"
#include "chunks.h"

void mainProcessor::Load(camxFile *ffile)
{
	camxFile file;

	if(file.OpenRead(mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_PROCESSORS))==true && file.CheckVersion()==true)
	{
		int procnr=0;
		bool ok=false;

		file.LoadChunk();

		if(file.GetChunkHeader()==CHUNK_SETTINGSPROCESSOR)
		{
			file.ChunkFound();

			char text[4];

			file.ReadChunk(text,4);

			if(text[0]=='C' &&
				text[1]=='A' &&
				text[2]=='P' &&
				text[3]=='C')
			{
				ok=true;
				file.ReadChunk(&procnr);
			}
#ifdef _DEBUG
			else
				maingui->MessageBoxOk(0,"Error: Settings File (C-A-P-C)");
#endif
			file.CloseReadChunk();
		}
#ifdef _DEBUG
		else
			maingui->MessageBoxOk(0,"Error: Settings File (HEADER)");
#endif

		if(ok==true)
		{
			while(procnr--)
			{
				file.LoadChunk();

				if(file.GetChunkHeader()==CHUNK_PROCESSORIO)
				{
					file.ChunkFound();
					file.CloseReadChunk();

					file.LoadChunk();
					if(file.GetChunkHeader()==CHUNK_PROCESSOR)
					{
						file.ChunkFound();
						if(Processor *p=new Processor)
						{
							p->Load(&file);
							AddProcessor(p);

							TRACE ("Load Processor %s\n",p->name);
						}
						else
							file.CloseReadChunk();
					}
				}
			}
		}

		file.Close(true);	
	}
}

void mainProcessor::Save(camxFile *file)
{	
	char *dirname=mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_PROCESSORS);

	if(dirname)
	{
		camxFile write;

		if(write.OpenSave_CheckVersion(dirname)==true)
		{
			write.SaveVersion();

			// Header
			write.OpenChunk(CHUNK_SETTINGSPROCESSOR);
			write.Save_Chunk("CAPC",4);
			write.Save_Chunk(processorlist.GetCount());
			write.CloseChunk();

			Processor *pro=FirstProcessor();

			while(pro)
			{
				write.OpenChunk(CHUNK_PROCESSORIO);
				write.CloseChunk();

				pro->Save(&write);
				pro=pro->NextProcessor();
			}
		}

		write.Close(true);
	}
}

ProcessorModulePointer *mainProcessor::InitProcessorModule(MIDIPlugin *pm)
{
	ProcessorModulePointer *nm=new ProcessorModulePointer;

	if(nm){
		nm->module=pm;
		processormodulelist.AddEndO(nm);
	}

	return nm;
}

void mainProcessor::AddRAWEvent(Seq_Event *seqevent)
{
	Seq_Event *clone=(Seq_Event *)seqevent->Clone(0);

	if(clone)
	{
		clone->pattern=seqevent->pattern;
		rawevents.AddOSort(clone,seqevent->ostart);
	}
}

Seq_Event *mainProcessor::DeleteRAWEvent(Seq_Event *seqevent,OListStart *sortto)
{
	Seq_Event *n=(Seq_Event *)rawevents.CutObject(seqevent);

	sortto->AddOSort(seqevent,seqevent->ostart);

	//event->Delete(true);

	return n;
}

Processor *mainProcessor::DeleteProcessor(Processor *def)
{
	def->DeleteAllProcessorModule();

	return (Processor *)processorlist.RemoveO(def);
}

void mainProcessor::DeleteAllProcessor()
{
	Processor *d=FirstProcessor();

	while(d)
		d=DeleteProcessor(d);

	// Delete Module List
	ProcessorModulePointer *pmp=FirstProcessorModule();

	while(pmp)
	{
		pmp->module->FreeProcessorModuleMemory();
		delete pmp->module;

		pmp=(ProcessorModulePointer *)processormodulelist.RemoveO(pmp);
	}
}

Processor *mainProcessor::AddProcessor(Processor *p,Processor *prev)
{
	if(prev)
		processorlist.AddPrevO(p,prev);
	else
		processorlist.AddEndO(p);

	return p;
}

void mainProcessor::InitDefaultProcessorModule()
{
	ProcessorModulePointer *chord=InitProcessorModule(new Proc_Chord);
	ProcessorModulePointer *appr=InitProcessorModule(new Proc_Appr);
	ProcessorModulePointer *delay=InitProcessorModule(new Proc_Delay);

	Load(0);

	if(FirstProcessorModule() && (!FirstProcessor()))
	{
		// Chord
		if(chord)
		{
			Processor *p=new Processor;

			if(p)
			{
				p->SetName("Chord");

				AddProcessor(p);

				p->AddProcessorModule(chord->module->CreateClone());
			}
		}

		// Appr
		if(appr)
		{
			Processor *p=new Processor;

			if(p)
			{
				p->SetName("Arpeggiator");
				AddProcessor(p);
				p->AddProcessorModule(appr->module->CreateClone());
			}
		}

		// Delay
		if(delay)
		{
			Processor *p=new Processor;
			if(p)
			{
				p->SetName("Delay");
				AddProcessor(p);
				p->AddProcessorModule(delay->module->CreateClone());
			}
		}
	}
}

// ###############################################
// Processor #####################################
// ###############################################

void Processor::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_PROCESSOR);
	file->Save_Chunk((CPOINTER)this);
	file->Save_ChunkString(name);
	file->Save_Chunk(modules.GetCount());
	file->Save_Chunk(bypass);
	file->CloseChunk();

	MIDIPlugin *pm=FirstProcessorModule();
	while(pm)
	{
		file->OpenChunk(CHUNK_PROCESSORMODULE);

		file->Save_Chunk(pm->moduletype);
		pm->Save(file);
		file->CloseChunk();

		pm=pm->NextModule();
	}
}

void Processor::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);

	file->Read_ChunkString(name);

	int nr=0;
	file->ReadChunk(&nr);
	file->ReadChunk(&bypass);
	file->CloseReadChunk();

	while(nr--)
	{
		file->LoadChunk();

		if(file->GetChunkHeader()==CHUNK_PROCESSORMODULE)
		{
			file->ChunkFound();

			int type=MIDIPlugin::PM_UNKNOWN;
			file->ReadChunk(&type);

			MIDIPlugin *module=0;

			switch(type)
			{
			case MIDIPlugin::PM_ARPEGGIATOR:
				module=new Proc_Appr;
				break;

			case MIDIPlugin::PM_DELAY:
				module=new Proc_Delay;
				break;
			}

			if(module)
			{
				module->Load(file);
				AddProcessorModule(module);
			}

			file->CloseReadChunk();
		}
	}
}

void Processor::SetName(char *n)
{
	if(strlen(n)<STANDARDSTRINGLEN)
		strcpy(name,n);
	else
	{
		strncpy(name,n,STANDARDSTRINGLEN);
		name[STANDARDSTRINGLEN]=0;
	}
}

void Processor::Delete(bool full)
{
	DeleteAllProcessorModule();
	delete this;
}

void Processor::DeleteThruOffs(Seq_Track *t,MIDIInputDevice *indev,UBYTE instatus,UBYTE key,UBYTE veloff)
{
	TRACE ("Delete Thru Off from Processor %s %d %d %d \n",t->GetName(),instatus,key,veloff);

	mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);

	if(MIDIPlugin *pm=FirstProcessorModule())
	{
		if(NoteOff_Raw *noteoff=new NoteOff_Raw(instatus,key,veloff))
		{
			Proc_AddEvent addevent(noteoff,t);

			/*
			noteoff->status=instatus;
			noteoff->key=key;
			noteoff->velocityoff=veloff;
*/

			// Processor
			MIDIProcessor processor(t->song,t);

			processor.AddProcEvent(noteoff,0,noteoff);

			while(pm){
				pm->LockO();
				pm->InsertEvent(&addevent,&processor);
				pm->UnlockO();
				pm=pm->NextModule();
			}
		}
	}

	mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);
}

Processor *Processor::Clone()
{
	if(Processor *p=new Processor)
	{
		strcpy(p->name,name);

		// Clone Modules
		MIDIPlugin *pm=FirstProcessorModule();

		while(pm)
		{
			MIDIPlugin *clone=pm->CreateClone();
			if(clone)
			{
				p->AddProcessorModule(clone);
			}

			pm=pm->NextModule();
		}

		return p;
	}

	return 0;
}

void Processor::AddProcessorModule(MIDIPlugin *pm,MIDIPlugin *prev)
{
	if(pm){
		pm->processor=this;

		if(prev)
			modules.AddPrevO(pm,prev);
		else
			modules.AddEndO(pm);
	}
}

MIDIPlugin *Processor::DeleteProcessorModule(MIDIPlugin *m)
{
	MIDIalarmprocessorproc->DeleteAllAlarms(m,0);
	m->FreeProcessorModuleMemory();
	return (MIDIPlugin *)modules.RemoveO(m);
}

void Processor::RemoveProcessorFromAlarms()
{
	mainthreadcontrol->Lock(CS_mainMIDIalarmthreadessor);

	MIDIPlugin *m=FirstProcessorModule();

	while(m)
	{
		MIDIalarmprocessorproc->DeleteAllAlarms(m,0);
		m=m->NextModule();
	}

	mainthreadcontrol->Unlock(CS_mainMIDIalarmthreadessor);
}

void Processor::DeleteAllProcessorModule()
{
	MIDIPlugin *m=FirstProcessorModule();

	while(m)
	{
		m=DeleteProcessorModule(m);
	}
}