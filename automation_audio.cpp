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

// Audio Plugin Bypass

void AT_AUDIO_PluginByPass::CreateAutomationStartParameters(AutomationTrack *at)
{
	at->CreateStartParameter000();
}

AutomationObject *AT_AUDIO_PluginByPass::GetContainerAutoObject()
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return 0;

	return insertaudiofx->audioeffect;
}

bool AT_AUDIO_PluginByPass::CompareWithPluginCtrl(AudioObject *ao,int type)
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return false;

	if(ao==insertaudiofx->audioeffect && type==PSYS_BYPASS)
		return true;

	return false;
}

double AT_AUDIO_PluginByPass::ConvertValueToAutomationSteps(double v)
{
	if(v>=0.5)
		return 1;
	
	return 0;
}

char *AT_AUDIO_PluginByPass::GetParmName(int index)
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return "Err";

	char *h=mainvar->GenerateString("Bypass",":",insertaudiofx->audioeffect->GetEffectName());

	if(h)
	{
		if(strlen(h)>62)
		{
			strncpy(valuestring,h,62);
			valuestring[62]=0;
		}
		else
			strcpy(valuestring,h);

		delete h;

		return valuestring;
	}

	return "?";
}

void AT_AUDIO_PluginByPass::LoadSpecialData(camxFile *file)
{
	file->AddPointer((CPOINTER)&insertaudiofx);
}

void AT_AUDIO_PluginByPass::SaveSpecialData(camxFile *file)
{
	file->Save_Chunk((CPOINTER)insertaudiofx);
}

void AT_AUDIO_PluginByPass::SendNewValue(AutomationTrack *)
{
	if(insertaudiofx && insertaudiofx->audioeffect)
		insertaudiofx->audioeffect->Bypass(value==1?true:false);
}

char *AT_AUDIO_PluginByPass::GetParmValueStringPar(int index,double par)
{
	return par==1?"ByPass":"[>";
}

char *AT_AUDIO_PluginByPass::GetParmValueString(int index)
{
	return GetParmValueStringPar(index,value);
}

void AT_AUDIO_PluginByPass::SetNewInsertAudioEffect(InsertAudioEffect *iae)
{
	insertaudiofx=iae;
}

	
// Audio Plugin
AT_AUDIO_Plugin::AT_AUDIO_Plugin(InsertAudioEffect *iae,int i)
	{
		automationobjectid=ID_AUDIOPLUGIN;
		staticobject=false;
		hasspecialdata=true;
		curvetype=CT_LINEAR;
		insertaudiofx=iae;
		index=i;
	}

AutomationObject *AT_AUDIO_Plugin::GetAutoObject()
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return 0;

	return insertaudiofx->audioeffect;
}

char *AT_AUDIO_Plugin::GetParmName(int i_index)
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return "Err";

	char *h=mainvar->GenerateString(insertaudiofx->audioeffect->GetParmName(index),":",insertaudiofx->audioeffect->GetEffectName());

	if(h)
	{
		if(strlen(h)>62)
		{
			strncpy(valuestring,h,62);
			valuestring[62]=0;
		}
		else
			strcpy(valuestring,h);

		delete h;

		return valuestring;
	}

	return insertaudiofx->audioeffect->GetParmName(index);
}

void AT_AUDIO_Plugin::LoadSpecialData(camxFile *file)
{
	file->AddPointer((CPOINTER)&insertaudiofx);
	file->ReadChunk(&index);
}

void AT_AUDIO_Plugin::SaveSpecialData(camxFile *file)
{
	file->Save_Chunk((CPOINTER)insertaudiofx);
	file->Save_Chunk(index);
}

void AT_AUDIO_Plugin::CreateAutomationStartParameters(AutomationTrack *)
{
}

void AT_AUDIO_Plugin::SendNewValue(AutomationTrack *at)
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return;

	insertaudiofx->audioeffect->SetParm(index,value);
}

char *AT_AUDIO_Plugin::GetParmValueStringPar(int i_index,double par)
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return "Err";

	return insertaudiofx->audioeffect->GetParmValueString(i_index);
}

char *AT_AUDIO_Plugin::GetParmValueString(int i_index)
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return "Err";

	return insertaudiofx->audioeffect->GetParmTypeValueString(index);
}

bool AT_AUDIO_Plugin::CompareWithPlugin(AudioObject *ao,int ix)
{
	if((!insertaudiofx) || (!insertaudiofx->audioeffect))
		return false;

	return ao==insertaudiofx->audioeffect && index==ix?true:false;
}

void AT_AUDIO_Plugin::SetNewInsertAudioEffect(InsertAudioEffect *iae)
{
	insertaudiofx=iae;
}

