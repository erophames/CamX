#include "MIDIfilter.h"
#include "objectevent.h"
#include "camxfile.h"
#include "chunks.h"
#include "gui.h"

bool MIDIFilter::CheckFilterNote(UBYTE status,UBYTE key)
{
	if(bypass==true)return true;
	UBYTE channel=status&0x0F;

	if(!(statusfilter&MIDIOUTFILTER_NOTEON))
		return false;

	if(!((((1<<(channel&0x0F))&channelfilter)))) // Channel ok ?
		return false;

	// Oct OK ?
	int oct=key/12;

	if(!(octavefilter&(1<<oct)))
		return false;

	return true;
}

void MIDIFilter::LoadData(camxFile *file)
{
	file->ReadChunk(&statusfilter);
	file->ReadChunk(&channelfilter);
	file->ReadChunk(&bypass);
	file->ReadChunk(&octavefilter);
}

void MIDIFilter::SaveData(camxFile *file)
{	
	file->Save_Chunk(statusfilter);
	file->Save_Chunk(channelfilter);
	file->Save_Chunk(bypass);
	file->Save_Chunk(octavefilter);
}

void MIDIFilter::Load(camxFile *file)
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_MIDIFILTER)
	{
		file->ChunkFound();
		LoadData(file);
		file->CloseReadChunk();
	}
}

void MIDIFilter::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_MIDIFILTER);
	SaveData(file);
	file->CloseChunk();
}

bool MIDIFilter::CheckFilterActive()
	{
		if(bypass==true)return false;

		if(octavefilter!=OCTAVE_RESET)
			return true;

		if(channelfilter!=0xFFFF ||
			(statusfilter!=(MIDIOUTFILTER_INTERN|MIDIOUTFILTER_NOTEON|MIDIOUTFILTER_POLYPRESSURE|MIDIOUTFILTER_CONTROLCHANGE|
			MIDIOUTFILTER_PROGRAMCHANGE|MIDIOUTFILTER_CHANNELPRESSURE|MIDIOUTFILTER_PITCHBEND|MIDIOUTFILTER_SYSEX))
			)
			return true;

		return false;
	}

void MIDIFilter::Toggle(int flag)
{
	if(maingui->GetShiftKey()==true)
	{
		statusfilter=flag;
	}
	else
	{
		if(statusfilter&flag)
			statusfilter CLEARBIT flag;
		else
			statusfilter |=flag;
	}
}

void MIDIFilter::Reset()
{
	octavefilter=OCTAVE_RESET;
	bypass=false;
	channelfilter=0xFFFF; // channel 1-16 on 
	statusfilter=MIDIOUTFILTER_INTERN|MIDIOUTFILTER_NOTEON|MIDIOUTFILTER_POLYPRESSURE|MIDIOUTFILTER_CONTROLCHANGE|
		MIDIOUTFILTER_PROGRAMCHANGE|MIDIOUTFILTER_CHANNELPRESSURE|MIDIOUTFILTER_PITCHBEND|MIDIOUTFILTER_SYSEX;
}

bool MIDIFilter::IsFilterActive()
{
	if(bypass==true)
		return false;

	if(channelfilter!=0xFFFF)// channel 1-16 on 
		return true;

	if(octavefilter!=OCTAVE_RESET)
		return true;

	int initf=MIDIOUTFILTER_INTERN|MIDIOUTFILTER_NOTEON|MIDIOUTFILTER_POLYPRESSURE|MIDIOUTFILTER_CONTROLCHANGE|MIDIOUTFILTER_PROGRAMCHANGE|MIDIOUTFILTER_CHANNELPRESSURE|MIDIOUTFILTER_PITCHBEND|MIDIOUTFILTER_SYSEX;

	if(statusfilter!=initf)
			return true;

	return false;
}

bool MIDIFilter::CheckEvent(Seq_Event *e)
{
	if(e){
		if(bypass==true)return true;
		return CheckBytes(e->status,e->GetByte1(),e->GetByte2());
	}

	return false;
}

bool MIDIFilter::CheckBytes(UBYTE status,UBYTE byte1,UBYTE byte2)
{
	if(bypass==true)return true;

	// Status Check
	switch(status&0xF0)
	{
	case SYSEX:
		if(!(statusfilter&MIDIOUTFILTER_SYSEX))
			return false;
		break;

	case NOTEON:
		if((1<<(status&0x0F))&channelfilter) // Channel ok ?
		{
			if((!(statusfilter&MIDIOUTFILTER_NOTEON)) && byte2>0) // dont filter NoteOffs
				return false;
		}
		else
			return false;

		break;

	case PITCHBEND:
		if((1<<(status&0x0F))&channelfilter) // Channel ok ?
		{
			if(!(statusfilter&MIDIOUTFILTER_PITCHBEND))
				return false;
		}
		else
			return false;
		break;

	case POLYPRESSURE:
		if((1<<(status&0x0F))&channelfilter) // Channel ok ?
		{
			if(!(statusfilter&MIDIOUTFILTER_POLYPRESSURE))
				return false;
		}
		else
			return false;
		break;

	case CONTROLCHANGE:
		if((1<<(status&0x0F))&channelfilter) // Channel ok ?
		{
			if(!(statusfilter&MIDIOUTFILTER_CONTROLCHANGE))
				return false;
		}
		else
			return false;
		break;

	case PROGRAMCHANGE:
		if((1<<(status&0x0F))&channelfilter) // Channel ok ?
		{
			if(!(statusfilter&MIDIOUTFILTER_PROGRAMCHANGE))
				return false;
		}
		else
			return false;
		break;

	case CHANNELPRESSURE:
		if((1<<(status&0x0F))&channelfilter) // Channel ok ?
		{
			if(!(statusfilter&MIDIOUTFILTER_CHANNELPRESSURE))
				return false;
			else
				return false;
			break;
		}
	}

	return true;
}