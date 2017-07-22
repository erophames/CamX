#include "defines.h"
#include "object_song.h"
#include "object_track.h"
#include "initplayback.h"
#include "automation.h"
#include "camxfile.h"
#include "gui.h"
#include "songmain.h"
#include "chunks.h"
#include "editfunctions.h"
#include "semapores.h"
#include "editortypes.h"
#include "MIDIoutproc.h"
#include "audiohardware.h"

AT_MIDI_Control::AT_MIDI_Control()
{
	automationobjectid=ID_MIDICONTROL;
	staticobject=false;
	hasspecialdata=true;
	curvetype=CT_LINEAR;
	MIDIvalue=0;
	ctrlevent.pattern=0;
	lastsendevent.status=0; // force send
}

AT_MIDI_Control::AT_MIDI_Control(int c)
{
	automationobjectid=ID_MIDICONTROL;
	staticobject=false;
	hasspecialdata=true;
	controller=c;
	curvetype=CT_LINEAR;
	MIDIvalue=0;
	ctrlevent.pattern=0;
	lastsendevent.status=0; 
}

char *AT_MIDI_Control::GetParmName(int index)
{
	char *h=maingui->ByteToControlInfo_NOMSBLSB(controller,MIDIvalue);

	if(h)
	{
		char *h2=mainvar->GenerateString("MIDI Ctrl:",h);

		if(h2)
		{
			if(strlen(h2)>38) // Max 38
			{
				strncpy(valuestring,h2,38);
				valuestring[38]=0;
			}
			else
				strcpy(valuestring,h2);

			delete h2;
		}

		return valuestring;
	}

	return "MC?";
}

void AT_MIDI_Control::LoadSpecialData(camxFile *file)
{
	file->ReadChunk(&controller);
}

void AT_MIDI_Control::SaveSpecialData(camxFile *file)
{
	file->Save_Chunk(controller);
}

void AT_MIDI_Control::CreateAutomationStartParameters(AutomationTrack *at)
{
	switch(controller)
	{
	case 7:
		at->CreateStartParameter001(); // Main Volume Start 127
		break;

	default: // Modulation
		at->CreateStartParameter0005();
		break;
	}

}

void AT_MIDI_Control::ConvertValueToIntern()
{
	double h=127;
	h*=value;
	// 0 - 127

	MIDIvalue=(int)h;
}

void AT_MIDI_Control::SendNewValue(AutomationTrack *atrack)
{
	ConvertValueToIntern();

	// Send TO MIDI/Plugins
	ctrlevent.status=atrack->track?CONTROLCHANGE|atrack->track->t_trackeffects.GetChannel():CONTROLCHANGE;
	ctrlevent.controller=controller;
	ctrlevent.value=MIDIvalue;

	if(lastsendevent.Compare(&ctrlevent)==false)
	{
		ctrlevent.CloneData(0,&lastsendevent);

		if(atrack->track)
			atrack->track->SendOutEvent_Automation(&ctrlevent,true);
	}
}

char *AT_MIDI_Control::GetParmValueStringPar(int index,double par)
{
	double h=127;
	h*=par;

	char t[NUMBERSTRINGLEN];
	strcpy(valuestring,mainvar->ConvertIntToChar((int)h,t));

	return valuestring;
}

char *AT_MIDI_Control::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

// Program Change
char *AT_MIDI_Program::GetParmName(int index)
{
	return "MIDI Program";
}

void AT_MIDI_Program::LoadSpecialData(camxFile *file)
{
	file->ReadChunk(&program);
}

void AT_MIDI_Program::SaveSpecialData(camxFile *file)
{
	file->Save_Chunk(program);
}

void AT_MIDI_Program::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter0005();
}

void AT_MIDI_Program::ConvertValueToIntern()
{
	double h=127;
	h*=value;
	program=(int)h;
}

void AT_MIDI_Program::SendNewValue(AutomationTrack *atrack)
{	
	ConvertValueToIntern();

	// Send TO MIDI/Plugins
	programevent.status=atrack->track?PROGRAMCHANGE|atrack->track->t_trackeffects.GetChannel():PROGRAMCHANGE;
	programevent.program=(UBYTE)program;

	if(lastsendevent.Compare(&programevent)==false)
	{
		programevent.CloneData(0,&lastsendevent);

		if(atrack->track)
			atrack->track->SendOutEvent_Automation(&programevent,true);
	}
}

char *AT_MIDI_Program::GetParmValueStringPar(int index,double par)
{
	double h=127;
	h*=par;

	h+=1;

	char t[NUMBERSTRINGLEN];
	strcpy(valuestring,mainvar->ConvertIntToChar((int)h,t));

	return valuestring;
}

char *AT_MIDI_Program::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

AT_MIDI_Program::AT_MIDI_Program()
{
	automationobjectid=ID_MIDIPROGRAM;
	staticobject=false;
	hasspecialdata=true;
	curvetype=CT_ABSOLUT;
	programevent.pattern=0;
	lastsendevent.status=0; // force send
	program=0;
}

bool AT_MIDI_Program::CompareWithMIDI(int status,int b1,int b2)
{
	if((status&0xF0)==PROGRAMCHANGE)
		return true;

	return false;
}

// ChannelPressure
char *AT_MIDI_Channelpressure::GetParmName(int index)
{
	return "MIDI Channel Pressure";
}

void AT_MIDI_Channelpressure::LoadSpecialData(camxFile *file)
{
	file->ReadChunk(&pressure);
}

void AT_MIDI_Channelpressure::SaveSpecialData(camxFile *file)
{
	file->Save_Chunk(pressure);
}

void AT_MIDI_Channelpressure::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter0005();
}

void AT_MIDI_Channelpressure::ConvertValueToIntern()
{
	double h=127;
	h*=value;

	pressure=(int)h;
}

void AT_MIDI_Channelpressure::SendNewValue(AutomationTrack *atrack)
{
	ConvertValueToIntern();

	// Send TO MIDI/Plugins
	channelevent.status=atrack->track?CHANNELPRESSURE|atrack->track->t_trackeffects.GetChannel():CHANNELPRESSURE;
	channelevent.pressure=(UBYTE)pressure;

	if(lastsendevent.Compare(&channelevent)==false)
	{
		channelevent.CloneData(0,&lastsendevent);

		if(atrack->track)
			atrack->track->SendOutEvent_Automation(&channelevent,true);
	}
}

char *AT_MIDI_Channelpressure::GetParmValueStringPar(int index,double par)
{
	double h=127;
	h*=par;

	char t[NUMBERSTRINGLEN];
	strcpy(valuestring,mainvar->ConvertIntToChar((int)h,t));

	return valuestring;
}

char *AT_MIDI_Channelpressure::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

AT_MIDI_Channelpressure::AT_MIDI_Channelpressure()
{
	automationobjectid=ID_MIDICHANNELPRESSURE;
	staticobject=false;
	hasspecialdata=true;
	curvetype=CT_LINEAR;
	channelevent.pattern=0;
	lastsendevent.status=0; // force send
	pressure=0;
}

bool AT_MIDI_Channelpressure::CompareWithMIDI(int status,int b1,int b2)
{
	if((status&0xF0)==CHANNELPRESSURE)
		return true;

	return false;
}

// Polypressure
char *AT_MIDI_Polypressure::GetParmName(int index)
{
	strcpy(valuestring,"MIDI Polypressure ");
	mainvar->AddString(valuestring,maingui->ByteToKeyString(automationtrack->GetSong(),pkey));

	return valuestring;
}

void AT_MIDI_Polypressure::LoadSpecialData(camxFile *file)
{
	file->ReadChunk(&pkey);
	file->ReadChunk(&pressure);
}

void AT_MIDI_Polypressure::SaveSpecialData(camxFile *file)
{
	file->Save_Chunk(pkey);
	file->Save_Chunk(pressure);
}

void AT_MIDI_Polypressure::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter0005();
}

void AT_MIDI_Polypressure::ConvertValueToIntern()
{
	double h=127;
	h*=value;

	pressure=(int)h;
}

void AT_MIDI_Polypressure::SendNewValue(AutomationTrack *atrack)
{
	ConvertValueToIntern();

	// Send TO MIDI/Plugins
	polyevent.status=atrack->track?POLYPRESSURE|atrack->track->t_trackeffects.GetChannel():POLYPRESSURE;
	polyevent.key=pkey;
	polyevent.pressure=(UBYTE)pressure;

	if(lastsendevent.Compare(&polyevent)==false)
	{
		polyevent.CloneData(0,&lastsendevent);

		if(atrack->track)
			atrack->track->SendOutEvent_Automation(&polyevent,true);
	}
}

char *AT_MIDI_Polypressure::GetParmValueStringPar(int index,double par)
{
	double h=127;
	h*=par;

	char t[NUMBERSTRINGLEN];
	strcpy(valuestring,mainvar->ConvertIntToChar((int)h,t));

	return valuestring;
}

char *AT_MIDI_Polypressure::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

AT_MIDI_Polypressure::AT_MIDI_Polypressure(int key)
{
	automationobjectid=ID_MIDIPOLYPRESSURE;
	staticobject=false;
	hasspecialdata=true;
	curvetype=CT_LINEAR;
	polyevent.pattern=0;
	lastsendevent.status=0; // force send
	pressure=0;
	pkey=key;
}

bool AT_MIDI_Polypressure::CompareWithMIDI(int status,int b1,int b2)
{
	if((status&0xF0)==POLYPRESSURE && b1==pkey)
		return true;

	return false;
}

// Pitchbend
char *AT_MIDI_Pitchbend::GetParmName(int index)
{
	return "MIDI Pitchbend";
}

void AT_MIDI_Pitchbend::LoadSpecialData(camxFile *file)
{
	file->ReadChunk(&pitchbend);
}

void AT_MIDI_Pitchbend::SaveSpecialData(camxFile *file)
{
	file->Save_Chunk(pitchbend);
}

void AT_MIDI_Pitchbend::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter0005();
}

void AT_MIDI_Pitchbend::ConvertValueToIntern()
{
	double h=16383;
	h*=value;
	h-=8192;

	// -8192 - 0 - 8191

	pitchbend=(int)h;
}

void AT_MIDI_Pitchbend::SendNewValue(AutomationTrack *atrack)
{
	ConvertValueToIntern();

	// Send TO MIDI/Plugins
	pitchevent.status=atrack->track?PITCHBEND|atrack->track->t_trackeffects.GetChannel():PITCHBEND;

	pitchevent.SetPitchbend(pitchbend);

	if(lastsendevent.Compare(&pitchevent)==false)
	{
		pitchevent.CloneData(0,&lastsendevent);

		if(atrack->track)
			atrack->track->SendOutEvent_Automation(&pitchevent,true);
	}
}

char *AT_MIDI_Pitchbend::GetParmValueStringPar(int index,double par)
{
	double h=16383;
	h*=par;
	h-=8192;

	char t[NUMBERSTRINGLEN];
	strcpy(valuestring,mainvar->ConvertIntToChar((int)h,t));

	return valuestring;
}

char *AT_MIDI_Pitchbend::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

AT_MIDI_Pitchbend::AT_MIDI_Pitchbend()
{
	automationobjectid=ID_MIDIPITCHBEND;
	staticobject=false;
	hasspecialdata=true;
	curvetype=CT_LINEAR;
	pitchevent.pattern=0;
	lastsendevent.status=0; // force send
	pitchbend=0;
}

bool AT_MIDI_Pitchbend::CompareWithMIDI(int status,int b1,int b2)
{
	if((status&0xF0)==PITCHBEND)
		return true;

	return false;
}

