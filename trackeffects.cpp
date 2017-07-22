#include "trackeffect.h"
#include "chunks.h"
#include "object_track.h"
#include "object_song.h"
#include "songmain.h"
#include "camxfile.h"

TrackEffects::TrackEffects()
{
	ndelay=0;
	delaytype=DELAYTYPE_MS;

	noMIDIinput=false;
	useallinputdevices=true;
	usealwaysthru=false;
	userouting=true;
	setalwaysthruautomatic=false;
	MIDIthru=false;
}

int TrackEffects::GetTranspose()
{
	int v=t_MIDI_transpose.transpose;
	if(track->parent)v+=((Seq_Track *)track->parent)->t_trackeffects.GetTranspose();
	return v;
}

int TrackEffects::GetTranspose_NoParent()
{
	return t_MIDI_transpose.transpose;
}

void TrackEffects::SetTranspose(int v)
{
	t_MIDI_transpose.transpose=v;
}

void TrackEffects::SetDelay(OSTART v)
{
	if(v!=ndelay)
	{
		ndelay=v;
	}
}

OSTART TrackEffects::GetDelay()
{
	OSTART v=ndelay;
	if(track->parent)v+=((Seq_Track *)track->parent)->t_trackeffects.GetDelay();
	return v;
}

bool TrackEffects::IsMIDIInOrOutFilter()
{
	if(filter.IsFilterActive()==true || inputfilter.IsFilterActive()==true)
		return true;

	return false;
}

int TrackEffects::GetChannel()
{
	if(t_MIDI_channel.channel>=1)return t_MIDI_channel.channel;

	if(track->parent)
	{
		int rv=((Seq_Track *)track->parent)->t_trackeffects.GetChannel();
		if(rv>=1)return rv;
	}

	return t_MIDI_channel.channel;
}

int TrackEffects::GetChannel_NoParent()
{
	return t_MIDI_channel.channel;
}

void TrackEffects::SetChannel(int v)
{
	t_MIDI_channel.channel=v;
}

bool TrackEffects::CompareOutput(TrackEffects *c)
{
	if(c->filter.Compare(&filter)==false)return false;

	return true;
}

bool TrackEffects::CompareInput(TrackEffects *c)
{
	if(c->noMIDIinput!=noMIDIinput)return false;
	if(c->useallinputdevices!=useallinputdevices)return false;
	if(c->usealwaysthru!=usealwaysthru)return false;
	if(c->noMIDIinput!=noMIDIinput)return false;

	if(c->inputfilter.Compare(&inputfilter)==false)return false;

	return true;
}

void TrackEffects::Clone(TrackEffects *to)
{
	to->t_MIDI_channel.channel=t_MIDI_channel.channel;
	to->t_MIDI_transpose.transpose=t_MIDI_transpose.transpose;
	
	to->ndelay=ndelay;
	to->delaytype=delaytype;

	to->noMIDIinput=noMIDIinput;
	to->useallinputdevices=useallinputdevices;
	to->usealwaysthru=usealwaysthru;
	to->userouting=userouting;

	filter.Clone(&to->filter);
	inputfilter.Clone(&to->filter);
	quantizeeffect.Clone(&to->quantizeeffect);
	MIDIprogram.Clone(&to->MIDIprogram);
}

void TrackEffects::Load(camxFile *file)
{
	file->LoadChunk();

	switch(file->GetChunkHeader())
	{
	case CHUNK_TRACKFX:
		{
			file->ChunkFound();

			int ch=0;
			file->ReadChunk(&ch);
			t_MIDI_channel.channel=ch;

			file->ReadChunk(&t_MIDI_transpose.transpose);

			int exvelocity;
			file->ReadChunk(&exvelocity);

			int exdelay=0;
			file->ReadChunk(&exdelay);

			file->ReadChunk(&noMIDIinput);
			file->ReadChunk(&useallinputdevices);
			file->ReadChunk(&usealwaysthru);
			file->ReadChunk(&userouting);
			file->ReadChunk(&setalwaysthruautomatic);
			file->ReadChunk(&MIDIthru);
			file->ReadChunk(&delaytype);
			file->ReadChunk(&ndelay);

			file->CloseReadChunk();

			quantizeeffect.Load(file);
			filter.Load(file);
			inputfilter.Load(file);
			MIDIprogram.Load(file);

			// MIDI Automation
			t_MIDI_channel.Load(file);
			t_MIDI_transpose.Load(file);
		}
		break;

#ifdef _DEBUG
	default:
		MessageBox(NULL,"No Track FX found","Error",MB_OK);
#endif
	}
}

void TrackEffects::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_TRACKFX);

	file->Save_Chunk(t_MIDI_channel.channel);
	file->Save_Chunk(t_MIDI_transpose.transpose);

	int exvelocity=0;
	file->Save_Chunk(exvelocity);

	int exdelay=0;
	file->Save_Chunk(exdelay);

	file->Save_Chunk(noMIDIinput);
	file->Save_Chunk(useallinputdevices);
	file->Save_Chunk(usealwaysthru);
	file->Save_Chunk(userouting);
	file->Save_Chunk(setalwaysthruautomatic);
	file->Save_Chunk(MIDIthru);
	file->Save_Chunk(delaytype);
	file->Save_Chunk(ndelay);

	file->CloseChunk();

	quantizeeffect.Save(file);
	filter.Save(file);
	inputfilter.Save(file);
	MIDIprogram.Save(file);

	// MIDI Automation
	t_MIDI_channel.Save(file);
	t_MIDI_transpose.Save(file);
}

int TrackEffects::GetVelocity()
{
	int v=track->MIDIfx.velocity.velocity;
	if(track->parent)v+=((Seq_Track *)track->parent)->t_trackeffects.GetVelocity();
	return v;
}

int TrackEffects::GetVelocity_NoParent()
{
	return track->MIDIfx.velocity.velocity;
}

void TrackEffects::SetVelocity(int nv,OSTART automationtime)
{
	track->MIDIfx.SetVelocity(nv,automationtime);
}

// Velocity
AT_MIDISYS_Velocity::AT_MIDISYS_Velocity()
{
	id=AID_MIDIVELOCITY;
	sysid=SYS_MIDIVOLUME;
	curvetype=CT_LINEAR;
	hasspecialdata=true;

	velocity=0;
}

void AT_MIDISYS_Velocity::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter0005();
}

void AT_MIDISYS_Velocity::ConvertValueToIntern()
{
	double h=254;

	h*=value;
	h-=127;

	// -127 - 0 - 127
	velocity=(int)h;

	TRACE ("Va %f\n",value);
	TRACE ("V %d\n",velocity);
}

void AT_MIDISYS_Velocity::SendNewValue(AutomationTrack *at)
{
	ConvertValueToIntern();
}

char *AT_MIDISYS_Velocity::GetParmValueStringPar(int index,double par)
{
	double h=254;

	h*=par;
	h-=127;

	// -127 - 0 - 127

	char t[NUMBERSTRINGLEN];
	strcpy(valuestring,mainvar->ConvertIntToChar((int)h,t));

	return valuestring;
}

char *AT_MIDISYS_Velocity::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

// Main Volume
AT_MIDISYS_MVolume::AT_MIDISYS_MVolume()
{
	id=AID_MIDIMVOLUME;
	sysid=SYS_MIDIMAINVOLUME;
	curvetype=CT_LINEAR;
	hasspecialdata=true;

	velocity=0;
}

void AT_MIDISYS_MVolume::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter0005();
}

void AT_MIDISYS_MVolume::ConvertValueToIntern()
{
	double h=254;

	h*=value;
	h-=127;

	// -127 - 0 - 127
	velocity=(int)h;

	TRACE ("Va %f\n",value);
	TRACE ("V %d\n",velocity);
}

void AT_MIDISYS_MVolume::SendNewValue(AutomationTrack *at)
{
	ConvertValueToIntern();
}

char *AT_MIDISYS_MVolume::GetParmValueStringPar(int index,double par)
{
	double h=254;

	h*=par;
	h-=127;

	// -127 - 0 - 127

	char t[NUMBERSTRINGLEN];
	strcpy(valuestring,mainvar->ConvertIntToChar((int)h,t));

	return valuestring;
}

char *AT_MIDISYS_MVolume::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}
