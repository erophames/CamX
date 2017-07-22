#include "songmain.h"
#include "arrangeeditor.h"
#include "object_track.h"
#include "MIDIhardware.h"
#include "editfunctions.h"
#include "semapores.h"
#include "audiofile.h"
#include "gui.h"
#include "undo.h"
// #include "freeze.h"
#include "audiohardware.h"
#include "drumevent.h"
#include "MIDIautomation.h"
#include "audioauto_volume.h"
#include "audioauto_vst.h"
#include "MIDIoutproc.h"
#include "editMIDIfilter.h"
#include "audiothread.h"
#include "editsettings.h"
#include "quantizeeditor.h"
#include "audiohdfile.h"
#include "audiomixeditor.h"
#include "MIDIprocessor.h"
#include "initplayback.h"
#include "crossfade.h"
#include "audiopattern.h"
#include "object_project.h"
#include "vstguiwindow.h"
#include "edit_audiointern.h"
#include "languagefiles.h"
#include "chunks.h"
#include "audioauto_volume.h"
#include "mastering.h"
#include "audioports.h"
#include "object_song.h"

extern char *channelchannelsinfo[];

TrackHead::TrackHead()
{
	MIDIfx.trackhead=this;

	showautomationstracks=false;
	automationon=true;
	track=0;
	channel=0;
	volumeclicked=m_volumeclicked=false;

	mastering=0;
	freezing=0;

	sizefactor=1; // Arrange Editor
}

void TrackHead::UserEditAutomation(AutomationObject *ao,int parindex)
{
	AutomationTrack *at=FirstAutomationTrack();
	while(at)
	{
		if(parindex==at->bindtoautomationobject_parindex)
		{
			AutomationObject *check=at->bindtoautomationobject->GetContainerAutoObject();

			if(check==ao)
			{
				switch(at->automode)
				{
				case ATMODE_OFF:
					at->automode=ATMODE_READ; // Off->Read
					break;

				default:

					break;
				}

				break;
			}
		}

		at=at->NextAutomationTrack();
	}
}

/*
bool TrackHead::CanAutomate(AutomationObject *ao,int parindex)
{
	AutomationTrack *at=FirstAutomationTrack();
	while(at)
	{
		if(parindex==at->bindtoautomationobject_parindex)
		{
			AutomationObject *check=at->bindtoautomationobject->GetContainerAutoObject();

			if(check==ao)
			{
				switch(at->automode)
				{
					//case ATMODE_OFF:
					//	return false;
					//	break;

				case ATMODE_READ:
					return false;
					break;

				default:
					return true;
					break;
				}
			}
		}

		at=at->NextAutomationTrack();
	}

	return true;
}
*/

AutomationTrack **TrackHead::FillAutomationListUsing(AutomationTrack **list,AutomationObject *ao)
{
	AutomationTrack *at=FirstAutomationTrack();
	while(at)
	{
		AutomationObject *check=at->bindtoautomationobject->GetContainerAutoObject();

		if(check==ao)
		{
			*list=at;
			list++;
		}

		at=at->NextAutomationTrack();
	}

	return list;
}

int TrackHead::GetCountOfAutomationTracksUsing(AutomationObject *ao)
{
	int c=0;

	AutomationTrack *at=FirstAutomationTrack();
	while(at)
	{
		AutomationObject *check=at->bindtoautomationobject->GetContainerAutoObject();

		if(check==ao)
			c++;

		at=at->NextAutomationTrack();
	}

	return c;
}

void TrackHead::CheckPlugins()
{
	// Input/Output
	for(int i=0;i<2;i++)
	{
		AudioEffects *fx;

		if(i==0)
			fx=track?&track->io.audioeffects:&channel->io.audioeffects;
		else
			fx=track?&track->io.audioinputeffects:&channel->io.audioinputeffects;

		InsertAudioEffect *iae=fx->FirstInsertAudioEffect();
		while(iae)
		{
			iae->audioeffect->LockAutomation();

			if(iae->audioeffect->automationeditparameter.checkcounter!=iae->audioeffect->automationeditparameter.automationeditparameter_counter)
			{
				// Automate
				iae->audioeffect->AutomationEdit
					(
					song,
					iae->audioeffect->automationeditparameter.time[iae->audioeffect->automationeditparameter.checkcounter],
					iae->audioeffect->automationeditparameter.index[iae->audioeffect->automationeditparameter.checkcounter],
					iae->audioeffect->automationeditparameter.value[iae->audioeffect->automationeditparameter.checkcounter],
					AEF_FROMOBJECT);

				// Learn
				maingui->LearnFromPluginChange(iae,iae->audioeffect,iae->audioeffect->automationeditparameter.time[iae->audioeffect->automationeditparameter.checkcounter],
					iae->audioeffect->automationeditparameter.index[iae->audioeffect->automationeditparameter.checkcounter],
					iae->audioeffect->automationeditparameter.value[iae->audioeffect->automationeditparameter.checkcounter]);

				if(iae->audioeffect->automationeditparameter.checkcounter==MAXPLUGINAUTOMATIONPARAMETER-1)
					iae->audioeffect->automationeditparameter.checkcounter=0;
				else
					iae->audioeffect->automationeditparameter.checkcounter++;
			}

			iae->audioeffect->UnlockAutomation();

			iae=iae->NextEffect();
		}
	}
}

void TrackHead::ShowHideAutomationTracks(bool show)
{
	showautomationstracks=show;

	AutomationTrack *at=FirstAutomationTrack();
	while(at)
	{
		at->visible=show;
		at=at->NextAutomationTrack();
	}
}

bool TrackHead::HasAutomation()
{
	if(automationon==false)
		return false;

	AutomationTrack *at=FirstAutomationTrack();
	while(at)
	{
		if(at->automode!=ATMODE_OFF && at->parameter.GetRoot())
			return true;

		at=at->NextAutomationTrack();
	}

	return false;
}

void TrackHead::DeleteDeletedAutomationParameter()
{
	{
		AutomationTrack *at=FirstAutomationTrack();
		while(at)
		{
			AutomationParameter *ap=(AutomationParameter *)at->parameter.GetRoot();
			while(ap)
			{
				if(ap->flag&AutomationParameter::AP_DELETED)
					goto found;

				ap=(AutomationParameter *)ap->next;
			}

			at=at->NextAutomationTrack();
		}
	}
	return;

found:
	mainthreadcontrol->LockActiveSong();

	{
		AutomationTrack *at=FirstAutomationTrack();
		while(at)
		{
			bool found=false;

			AutomationParameter *ap=(AutomationParameter *)at->parameter.GetRoot();
			while(ap)
			{
				AutomationParameter *nap=(AutomationParameter *)ap->next;

				if(ap->flag&AutomationParameter::AP_DELETED)
				{
					at->parameter.RemoveO(ap);
					found=true;
				}

				ap=nap;
			}

			if(found==true)
				at->parameter.EndIndex();

			at=at->NextAutomationTrack();
		}
	}

	mainthreadcontrol->UnlockActiveSong();
}

void TrackHead::RepairAutomationTracks()
{
	AutomationTrack *at=FirstAutomationTrack();
	while(at)
	{
		if(at->bindtoautomationobject)
		{
			at->bindtoautomationobject->automationtrack=at;
			at->bindtoautomationobject->ConvertValueToIntern();
		}

		at=at->NextAutomationTrack();
	}

}

void TrackHead::ResetAutomationTracks()
{
	DeleteDeletedAutomationParameter();

	AutomationTrack *at=FirstAutomationTrack();
	while(at)
	{
		at->touchlatch=false;

		AutomationParameter *ap=at->FirstAutomationParameter();

		while(ap)
		{
			ap->flag CLEARBIT AutomationParameter::AP_TOUCHLATCH;
			ap=ap->NextAutomationParameter();
		}

		at=at->NextAutomationTrack();
	}
}

guiMenu *TrackHead::CreateChannelTypeMenu(guiMenu *to)
{
	//if(record==true && (song->status&Seq_Song::STATUS_RECORD))
	//	return 0;

	if(to)
	{
		for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
		{
			class menu_ct:public guiMenu
			{
			public:
				menu_ct(TrackHead *th,int i)
				{
					trackh=th;
					type=i;
				}

				void MenuFunction()
				{
					trackh->SetVType(trackh->song?trackh->song->audiosystem.device:mainaudio->GetActiveDevice(),type,true,true);
				}

				TrackHead *trackh;
				int type;
			};

			to->AddFMenu(channelchannelsinfo[i],new menu_ct(this,i),io.channel_type==i?true:false);
		}
	}

	return 0;
}


void TrackHead::InitDefaultAutomationTracks(Seq_Track *track,AudioChannel *channel)
{
	if(!FirstAutomationTrack())
	{
		// 1. Audio Volume
		if(mainsettings->automation_createvolumetrack==true)
			AddAutomationTrack(&io.audioeffects.volume,0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);

		if(track || (channel && channel->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL))
		{
			if(mainsettings->automation_createpantrack==true)
				AddAutomationTrack(&io.audioeffects.pan,0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);
		}

		// 2. MIDI Velocity
		if(track)
		{
			if(mainsettings->automation_createmute==true)
				AddAutomationTrack(&track->io.audioeffects.mute,0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);// Mute

			if(mainsettings->automation_createsolo==true)
				AddAutomationTrack(&track->io.audioeffects.solo,0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);// Solo

			if(mainsettings->automation_createMIDIvolumetrack==true)
				AddAutomationTrack(new AT_MIDI_Control(7),0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);// Volume

			if(mainsettings->automation_createMIDImodulationtrack==true)
				AddAutomationTrack(new AT_MIDI_Control(1),0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);// Modulation

			if(mainsettings->automation_createMIDIpitchbendtrack==true)
				AddAutomationTrack(new AT_MIDI_Pitchbend,0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);

			if(mainsettings->automation_createMIDIvelocitytrack==true)
				AddAutomationTrack(&track->MIDIfx.velocity,0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);
		}

		if(channel)
		{
			if(mainsettings->automation_createmute==true)
				AddAutomationTrack(&channel->io.audioeffects.mute,0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);// Mute

			if(channel->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL && mainsettings->automation_createsolo==true)
				AddAutomationTrack(&channel->io.audioeffects.solo,0,new AutomationTrack,0,ADDSUBTRACK_CREATESTARTOBJECTS,track,channel);// Solo
		}
	}
}

void TrackHead::ShowAutomationTracks(bool onoff)
{
	showautomationstracks=onoff;

	maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
	maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGELIST,0);
}

void TrackHead::SetActiveAutomationTrack(AutomationTrack *automationtrack)
{
	if(activeautomationtrack!=automationtrack){
		activeautomationtrack=automationtrack;
		maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
	}
}

void TrackHead::SendAudioAutomation(OSTART position,OSTART endposition)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		// Remove Touch/Latch/Write
		at->DeleteTouchLatchParameter(endposition!=1?endposition:position);

		if( 
			(at->bindtoautomationobject->IsAudio()==true || at->bindtoautomationobject->IsSystem()==true) && 
			at->CanPlayAutomation()==true
			)
		{
			double nv=at->GetValueAtPosition(position);

			if(nv!=at->bindtoautomationobject->GetParm(at->bindtoautomationobject_parindex))
				at->bindtoautomationobject->SetValue(at,at->GetValueAtPosition(position));
		}

		at=at->NextAutomationTrack();
	}
}

void TrackHead::SendMIDIAutomation(OSTART position)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		at->DeleteTouchLatchParameter(position);

		if(
			(at->bindtoautomationobject->IsMIDI()==true  || at->bindtoautomationobject->IsSystem()==true) && 
			at->CanPlayAutomation()==true
			)
		{
			double nv=at->GetValueAtPosition(position);

			if(nv!=at->bindtoautomationobject->GetParm(at->bindtoautomationobject_parindex))
				at->bindtoautomationobject->SetValue(at,nv);
		}

		at=at->NextAutomationTrack();
	}
}

AutomationTrack *TrackHead::FirstAutomationTrackWithSelectedParameters()
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(at->bindtoautomationobject)
		{
			AutomationParameter *ap=at->FirstAutomationParameter();
			while(ap)
			{
				if(ap->IsSelected()==true)
					return at;

				ap=ap->NextAutomationParameter();
			}
		}

		at=at->NextAutomationTrack();
	}

	return 0;
}

bool TrackHead::IsAutomationParameterSelected()
{
	if(FirstAutomationTrackWithSelectedParameters())
		return true;

	return false;
}

AutomationTrack *TrackHead::FirstAudioAutomationTrack()
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(at->bindtoautomationobject && (at->bindtoautomationobject->IsAudio()==true || at->bindtoautomationobject->IsSystem()==true))
			return at;

		at=at->NextAutomationTrack();
	}

	return 0;
}

void TrackHead::ResetAutomationTracksCycle(OSTART cycleposition,int index)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(at->bindtoautomationobject)
		{
			switch(index)
			{
			case INITPLAY_AUDIOTRIGGER:
				if(at->bindtoautomationobject->IsAudio()==true || at->bindtoautomationobject->IsSystem()==true)
				{
					if(at->CanAutomation()==true && at->CanPlayAutomation()==true)
						at->bindtoautomationobject->SetValue(at,at->GetValueAtPosition(cycleposition));
				}
				break;
			}
		}

		at=at->NextAutomationTrack();
	}
}

void TrackHead::LoadAutomationTracks(camxFile *file,Seq_Track *track,AudioChannel *channel)
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUTIOMATIONTRACKHEADER)
	{
		file->ChunkFound();
		int sn=0;

		file->ReadChunk(&sn);

		TRACE ("AUTOTRACKS READ %d\n",sn);

		file->ReadChunk(&showautomationstracks);
		file->ReadChunk(&automationon);
		file->CloseReadChunk();

		while(sn--){

			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_AUTOMATIONTRACK)
			{
				file->ChunkFound();

				if(AutomationTrack *nat=new AutomationTrack)
				{
					AddAutomationTrack(0,0,nat,0,0,track,channel);
					nat->Load(file);
				}
				else
					file->CloseReadChunk();		
			}
			else
				break;

		}// while	
	}

}

void TrackHead::SaveAutomationTracks(camxFile *file)
{
	if(int sn=GetCountAutomationTracks())
	{
		file->OpenChunk(CHUNK_AUTIOMATIONTRACKHEADER);

		file->Save_Chunk(sn);
		file->Save_Chunk(showautomationstracks);
		file->Save_Chunk(automationon);

		file->CloseChunk();

		AutomationTrack *st=FirstAutomationTrack();

		while(st){

			st->Save(file);
			st=st->NextAutomationTrack();
		}
	}
}

void TrackHead::RefreshDo()
{
	io.RefreshDo();
}

void TrackHead::PreRefreshDo()
{
	io.PreRefreshDo();
}

AutomationTrack *TrackHead::FindPluginControlAutomationTrack(AudioObject *ao,int type)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(AutomationObject *ao1=at->bindtoautomationobject->GetAutoObject())
		{
			if(ao1->CompareWithPluginCtrl(ao,type)==true)
				return at;
		}

		at=at->NextAutomationTrack();
	}

	return 0;
}

AutomationTrack *TrackHead::FindMIDISysAutomationTrack(int systype)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(at->bindtoautomationobject && at->bindtoautomationobject->GetAutoObject()->sysid==systype)
			return at;

		at=at->NextAutomationTrack();
	}

	return 0;
}

AutomationTrack *TrackHead::FindSysAutomationTrack(int systype)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(at->bindtoautomationobject && at->bindtoautomationobject->GetAutoObject()->sysid==systype)
			return at;

		at=at->NextAutomationTrack();
	}

	return 0;
}
AutomationTrack *TrackHead::FindPluginAutomationTrack(AudioObject *ao,int index)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(AutomationObject *ao1=at->bindtoautomationobject)
		{
			AutomationObject *ao2=ao1->GetAutoObject();

			if(ao1==ao2) // No Container
			{
				if(ao1->CompareWithPlugin(ao,index)==true)
					return at;
			}
			else
			{
				// Container ... Plugins

				if(ao1->CompareWithPlugin(ao,index)==true)
					return at;

				if(ao2->CompareWithPlugin(ao,index)==true)
					return at;
			}
		}

		at=at->NextAutomationTrack();
	}

	return 0;
}

AutomationTrack *TrackHead::FindMIDIAutomationTrack(int status,int value)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(at->bindtoautomationobject && at->bindtoautomationobject->GetAutoObject()->CompareWithMIDI(status,value,0)==true)
			return at;

		at=at->NextAutomationTrack();
	}

	return 0;
}

void TrackHead::AddAutomationTrack(AutomationObject *bobject,int bindex,AutomationTrack *automationtrack,AutomationTrack *prev,int iflag,Seq_Track *track,AudioChannel *channel)
{
#ifdef DEBUG
	if(track==0 && channel==0)
		maingui->MessageBoxError(0,"track==0 && channel==0");
#endif

	if(!automationtrack)
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"automationtrack==0");
#endif
		return;
	}

	automationtrack->track=track;
	automationtrack->audiochannel=channel;
	automationtrack->bindtoautomationobject=bobject;
	automationtrack->bindtoautomationobject_parindex=bindex;

	if(bobject)
	{
		bobject->automationtrack=automationtrack;
		automationtrack->curvetype=bobject->curvetype; // Set Default CT
	}

	/*
	if(bobject && bobject->GetAutoObject()->IsAudio()==true)
	{
	int channels=track?track->io.GetChannels():channel->io.GetChannels();

	AudioObject *audioobject=(AudioObject *)bobject->GetAutoObject();
	audioobject->InitIOChannels(channels);
	}
	*/

	if(iflag&ADDSUBTRACK_LOCK)
		mainthreadcontrol->LockActiveSong();

	if(iflag&ADDSUBTRACK_TOP)
		automationtracks.AddStartO(automationtrack);
	else
	{
		if(prev)
			automationtracks.AddPrevO(automationtrack,prev);
		else
			automationtracks.AddEndO(automationtrack);
	}

	if(!activeautomationtrack)
		activeautomationtrack=automationtrack;

	if(iflag&ADDSUBTRACK_CREATESTARTOBJECTS)
	{
		bobject->GetAutoObject()->CreateAutomationStartParameters(automationtrack);
		automationtrack->parameter.EndIndex();
	}

	if(iflag&ADDSUBTRACK_LOCK)
	{
		mainthreadcontrol->UnlockActiveSong();
	}

	if(iflag&ADDSUBTRACK_UNDO)
		mainedit->CreateAutomationTrack(automationtrack);
}

AutomationTrack *TrackHead::DeleteAutomationTrack(AutomationTrack *automationtrack,bool full)
{
	AutomationTrack *p=automationtrack->PrevAutomationTrack(),*n=automationtrack->NextAutomationTrack();

	automationtracks.CutQObject(automationtrack);

	if(automationtrack==activeautomationtrack)
	{
		if(n)
			activeautomationtrack=n;
		else
			activeautomationtrack=p;
	}

	if(full==true)
	{
		delete automationtrack;
	}

	return n;
}


void TrackHead::DeleteAllAutomationTracks()
{
	AutomationTrack *s=FirstAutomationTrack();

	while(s)
		s=DeleteAutomationTrack(s,true);
}

void TrackHead::DeselectAllAutomationParameters(AutomationTrack *not)
{
	AutomationTrack *s=FirstAutomationTrack();

	while(s)
	{
		if(s!=not && s->bindtoautomationobject)
		{
			AutomationParameter *ap=s->FirstAutomationParameter();

			while(ap)
			{
				ap->UnSelect();
				ap=ap->NextAutomationParameter();
			}
		}

		s=s->NextAutomationTrack();
	}
}

bool TrackHead::SetAutomationTracksTouchLatch(AutomationObject *ao,int pindex,OSTART atime,bool reset)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(at->bindtoautomationobject->GetContainerAutoObject()==ao && at->bindtoautomationobject_parindex==pindex)
		{
			if(reset==true)
			{
				if(at->automode==ATMODE_TOUCH) // LATCH continue 
					at->touchlatch=false; // End

				at->automationendtime=atime;
			}
			else
			{
				switch(at->automode)
				{
				case ATMODE_OFF:
				case ATMODE_TOUCH:
				case ATMODE_LATCH:
				case ATMODE_WRITE:
					at->touchlatch=true; // Start
					at->automationstarttime=atime;
					return true;
					break;

				case ATMODE_READ:
					if(automationon==true)
					return false;
					else
						return true;

					break;
				}
			}
		}

		at=at->NextAutomationTrack();
	}

	return true;
}



void TrackHead::MIDIVolumeDown()
{
	if(m_volumeclicked==false && CanAutomationObjectBeChanged(&MIDIfx.velocity,0,0)==true)
	{
		if(MIDIfx.velocity.BeginEdit(song,0)==true)
			m_volumeclicked=true;
	}
}

void TrackHead::MIDIVolumeUp()
{
	if(m_volumeclicked==true)
	{
		MIDIfx.velocity.EndEdit(song,0);
		m_volumeclicked=false;
	}
}

void TrackHead::VolumeDown()
{
	if(volumeclicked==false && CanAutomationObjectBeChanged(&io.audioeffects.volume,0,0)==true)
	{
		if(io.audioeffects.volume.BeginEdit(song,0)==true)
			volumeclicked=true;
	}
}

void TrackHead::VolumeUp()
{
	if(volumeclicked==true)
	{
		io.audioeffects.volume.EndEdit(song,0);
		volumeclicked=false;
	}
}

void TrackHead::CloseMastering(bool freememory,bool canceled)
{
	if(mastering)
	{
		mastering->canceled=canceled;
		mastering->Close(freememory);
		delete mastering;
		mastering=0;
	}

	if(freezing)
	{
		freezing->canceled=canceled;

		freezing->Close(freememory);
		delete freezing;
		freezing=0;
	}

}