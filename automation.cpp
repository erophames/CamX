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
#include "languagefiles.h"
#include "edit_createtracks.h"
#include "settings.h"

char *automationtrack_mode_names[]=
{
	"Off",
	"Read",
	"Touch",
	"Latch",
	"Write",
	0
};

AutomationTrack *AutomationTrack::NextAudioAutomationTrack()
{
	AutomationTrack *at=NextAutomationTrack();
	while(at)
	{
		if(at->bindtoautomationobject && (at->bindtoautomationobject->IsAudio()==true || at->bindtoautomationobject->IsSystem()==true))
			return at;

		at=at->NextAutomationTrack();
	}
	return 0;
}

void AutomationTrack::ResetAutomationParameter(AutomationParameter *ap)
{
	bindtoautomationobject->ResetAutomationParameter(ap,bindtoautomationobject_parindex);
}

void AutomationTrack::MoveOrCopySelectedAutomationParameters(OSTART diff,bool copy)
{
	if(bindtoautomationobject && diff)
	{
		if(diff<0)
		{
			OSTART minus=0;

			AutomationParameter *ap=FirstAutomationParameter();

			while(ap)
			{
				if(ap->IsSelected()==true && (!(ap->flag&AutomationParameter::AP_DONTMOVE)) )
				{
					OSTART npos=ap->GetParameterStart()+diff;

					if(npos<0)
					{
						if(minus==0 || npos>minus)
							minus=npos;
					}
				}

				ap=ap->NextAutomationParameter();
			}

			if(minus<0)
				diff-=minus;
		}

		if(diff==0)
			return;

		if(copy==true)
		{
			AutomationParameter *ap=FirstAutomationParameter();

			int c=0;

			while(ap)
			{
				if(ap->IsSelected()==true)
					c++;

				ap=ap->NextAutomationParameter();
			}

			if(!c)
				return;

			parameter.ResetIndex();

			// Copy <->
			ap=FirstAutomationParameter();

			while(ap)
			{
				AutomationParameter *nap=ap->NextAutomationParameter();

				if(ap->flag&AutomationParameter::AP_COPYIED) // Delete copyied Parameter
					DeleteParameter(ap,true);

				ap=nap;
			}

			ap=FirstAutomationParameter();

			while(ap)
			{
				if(ap->IsSelected()==true)
				{
					if(AutomationParameter *nap=new AutomationParameter)
					{
						OSTART npos=ap->GetParameterStart()+diff;

						nap->flag=AutomationParameter::AP_COPYIED;
						nap->curvetype=ap->curvetype;
						nap->ostart=npos;
						nap->value=ap->value;

						parameter.AddOSort(nap);
					}
				}

				ap=ap->NextAutomationParameter();
			}

			parameter.EndIndex();
		}
		else
		{
			// Move <->

			AutomationParameter *ap=FirstAutomationParameter();

			int c=0;

			while(ap)
			{
				if(ap->IsSelected()==true && (!(ap->flag&AutomationParameter::AP_DONTMOVE)) )
					c++;

				ap=ap->NextAutomationParameter();
			}

			if(!c)
				return;

			AutomationParameter **mpar=0;
			OSTART *mpos=0;
			int pc=0;

			if(mpar=new AutomationParameter*[c])
			{
				mpos=new OSTART[c];

				if(!mpos)
					goto exit;

				AutomationParameter *ap=FirstAutomationParameter();

				while(ap)
				{
					if(ap->IsSelected()==true && (!(ap->flag&AutomationParameter::AP_DONTMOVE)))
					{
						OSTART npos=ap->GetParameterStart()+diff;

						mpar[pc]=ap;
						mpos[pc++]=npos;				
					}

					ap=ap->NextAutomationParameter();
				}

				parameter.ResetIndex();

				// 1. Cut
				for(int i=0;i<c;i++)
					parameter.CutObject(mpar[i]);

				for(int i=0;i<c;i++)
				{
					while(parameter.FindObjectAtPos(mpos[i]))
					{
						for(int a=i;a<c;a++)
							mpos[a]++; // Avoid 2x AutomationParameter at same position
					}

					parameter.AddOSort(mpar[i],mpos[i]);	
				}

				parameter.EndIndex();
			}

exit:
			if(mpar)
				delete mpar;

			if(mpos)
				delete mpos;
		}
	}
}

void AutomationTrack::Automate(OSTART atime,double value,int iflag)
{
	/*
	Off
	Off disables the current track automation data without deleting it. No automation data
	is written, read, or played back. If the current automation mode is Off, any edits to track
	automation data in the Arrange area automatically switch the automation mode to Read.
	This ensures that the data, as currently edited, will be played.
	Given that track automation can be recorded during playback mode, Off is the default
	setting, as any mix automation recording may prove disconcerting while arranging.

	Read
	Read mode automates the current track, using the existing automation data.
	The data cannot be changed by moving the channel strip controls, or using an external
	automation controller, when in Read mode.

	Touch
	Touch mode plays back automation data in the same fashion as Read mode.
	If a channel strip or an external (touch-sensitive) automation controller is touched, the
	existing track automation data of the active parameter is replaced by any controller
	movements—for as long as the fader or knob is touched. When you release the controller,
	the automation parameter returns to its original (recorded) value. The time required by
	a parameter to return to its previously recorded setting is set via Logic Pro > Preferences
	> Automation > Ramp Time.
	Touch is the most useful mode for creating a mix, and is directly comparable to “riding
	the faders” on a hardware mixing console. It allows you to correct and improve the mix
	at any time, when automation is active.

	Latch
	Latch mode basically works like Touch mode, but the current value replaces any existing
	automation data after releasing the fader or knob, when Logic Pro is in playback (or
	record) mode.
	To finish, or to end parameter editing, stop playback (or recording).

	Write
	In Write mode, existing track automation data is erased as the playhead passes it.
	If you move any of the Mixer’s (or an external unit’s) controls, this movement is recorded;
	if you don’t, existing data is simply deleted as the playhead passes it.
	*/

	/*
	Aus

	Der Modus "Aus" deaktiviert die aktuellen Spurautomationsdaten, ohne diese zu löschen. Es werden dann keine Automationsdaten geschrieben, gelesen oder wiedergegeben. Ist der Automationsmodus "Aus" eingestellt, wird für alle Änderungen an Spurautomationsdaten im Arrangierbereich automatisch in den Automationsmodus "Read" gewechselt. So wird sichergestellt, dass die Daten mit den aktuellen Änderungen wiedergegeben werden.

	Da die Spurautomation auch im Wiedergabe-Modus aufgezeichnet werden kann, ist standardmäßig "Aus" eingestellt, da die Automationsaufzeichnung beim Arrangieren im Mix verwirrend sein kann.
	Read

	Der Read-Modus automatisiert die ausgewählte Spur mithilfe der bestehenden Automationsdaten.

	Wenn "Read" eingestellt ist, können die Daten nicht durch Bewegen der Bedienelemente im Channel-Strip oder mithilfe eines externen Automation-Controllers verändert werden.
	Touch

	Der Touch-Modus gibt Automationsdaten in derselben Weise wieder wie der Read-Modus.

	Wenn ein Bedienelement eines Kanalzugs oder ein externer (touch-sensitiver) Automation-Controller bewegt wird, werden die bestehenden Spurautomationsdaten des aktivierten Parameters durch die entsprechenden Controller-Bewegungen ersetzt, und zwar so lange wie der Fader oder Regler bewegt wird. Wenn Sie den Controller loslassen, geht der Automationsparameter wieder auf seinen bestehenden (aufgezeichneten) Wert zurück. Die Zeit, mit der sich der Wert wieder den zuvor aufgezeichneten Daten für den Parameter anpasst, stellen Sie ein unter "Logic Pro" > "Einstellungen" > "Automation" > "Rampenzeit".

	Touch ist der beste Modus für das Erstellen einer Mischung und ist vergleichbar mit der Fader-Automation auf einem Hardware-Mischpult. Sie können so die Mischung bei aktivierter Automation jederzeit korrigieren und optimieren.
	Latch

	Der Latch-Modus funktioniert im Grunde wie der Touch-Modus. Im Wiedergabe- und Aufnahme-Modus ist es jedoch so, dass der letzte Wert nach Loslassen des Faders oder Reglers alle bestehenden Automationsdaten ersetzt, bis die Wiedergabe in Logic Pro gestoppt wird.

	Um die Aufzeichnung oder Bearbeitung der Parameter zu beenden, muss die Wiedergabe (oder Aufnahme) gestoppt werden.
	Write

	Im Write-Modus werden die bestehenden Spurautomationsdaten mit dem Fortschreiten der Positionslinie entsprechend überschrieben.

	Wenn Sie ein Bedienelement im Mixer (oder an einem externen Gerät) bewegen, wird diese Bewegung aufgezeichnet. Werden keine Bedienelemente bewegt, werden die bestehenden Daten einfach mit dem Fortschreiten der Positionslinie gelöscht. 

	*/

	if(!(iflag & AEF_USEREDIT))
	{
		if(GetSong()->playbacksettings.automationrecording==false)
			return;
	}

	AutomationParameter *fp=FindAutomationParameter(atime); // Avoid 2x Automation at X Position
	if(fp && fp->GetParameterStart()==atime)
	{
		return;
	}

	if(bindtoautomationobject)
		value=bindtoautomationobject->ConvertValueToAutomationSteps(value);

	if(iflag&AEF_FORCE)
	{
		CreateAndAddParameter(atime,value,GetSong(),iflag);
		return;
	}

	switch(automode)
	{
	case ATMODE_OFF:
		break;

	case ATMODE_READ:
		break;

	case ATMODE_TOUCH:
		if( ((GetSong()->status&Seq_Song::STATUS_RECORD) && mainsettings->automationrecordingrecord==true) || 
			((GetSong()->status&Seq_Song::STATUS_PLAY) && mainsettings->automationrecordingplayback==true)
			)
		{
			CreateAndAddParameter(atime,value,GetSong(),iflag);
		}
		break;

	case ATMODE_LATCH:
		{
			if( ((GetSong()->status&Seq_Song::STATUS_RECORD) && mainsettings->automationrecordingrecord==true) || 
				((GetSong()->status&Seq_Song::STATUS_PLAY) && mainsettings->automationrecordingplayback==true)
				)
			{
				CreateAndAddParameter(atime,value,GetSong(),iflag);
			}
		}
		break;

	case ATMODE_WRITE:
		{
			if( ((GetSong()->status&Seq_Song::STATUS_RECORD) && mainsettings->automationrecordingrecord==true) || 
				((GetSong()->status&Seq_Song::STATUS_PLAY) && mainsettings->automationrecordingplayback==true)
				)
			{
				CreateAndAddParameter(atime,value,GetSong(),iflag);
			}
		}
		break;
	}
}

bool TrackHead::AutomateBeginEdit(AutomationObject *ao,OSTART atime,int pindex)
{
	return SetAutomationTracksTouchLatch(ao,pindex,atime);
}

void TrackHead::AutomateEndEdit(AutomationObject *ao,OSTART atime,int pindex)
{
	SetAutomationTracksTouchLatch(ao,pindex,atime,true);
}

int TrackHead::GetCountSelectedAutomationParameters()
{
	int c=0;

	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		c+=at->GetCountOfSelectedAutomationParameter();

		at=at->NextAutomationTrack();
	}

	return c;
}

bool TrackHead::CanAutomationObjectBeChanged(AutomationObject *ao,int iflag,int pindex)
{
	if(automationon==false)
		return true; // Auto OFF 

	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(iflag&AEF_PLUGINCONTROL)
		{
			if(at->bindtoautomationobject && at->bindtoautomationobject->GetContainerAutoObject()==ao && at->bindtoautomationobject->automationobjectid==pindex)
				goto check;
		}
		else
		{
			if(at->bindtoautomationobject && at->bindtoautomationobject->GetAutoObject()==ao && at->bindtoautomationobject_parindex==pindex)
				goto check;
		}

		at=at->NextAutomationTrack();
	}

	return true;

check:

	switch(at->automode)
	{
	case ATMODE_OFF:
		return true;
		break;

	case ATMODE_LATCH:
		return true;
		break;

	case ATMODE_TOUCH:
		return true;
		break;
	}

	if(at->FirstAutomationParameter()==0)
		return true;

	return false;
}

bool TrackHead::CheckAutomate(AutomationObject *ao,OSTART atime,int pindex,double value,int iflag)
{
	AutomationTrack *at=FirstAutomationTrack();

	while(at)
	{
		if(iflag&AEF_PLUGINCONTROL)
		{
			if(at->bindtoautomationobject && at->bindtoautomationobject->GetContainerAutoObject()==ao && at->bindtoautomationobject->automationobjectid==pindex)
			{
				at->Automate(atime,value,iflag);
				return true;
			}
		}
		else
		{
			if(at->bindtoautomationobject && at->bindtoautomationobject->GetAutoObject()==ao && at->bindtoautomationobject_parindex==pindex)
			{
				at->Automate(atime,value,iflag);
				return true;
			}
		}

		at=at->NextAutomationTrack();
	}

	return false;
}

bool Seq_Song::AutomateBeginEdit(AutomationObject *ao,OSTART atime,int pindex)
{
	// 1. Tracks
	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->AutomateBeginEdit(ao,atime,pindex)==false)
			return false;

		t=t->NextTrack();
	}

	// 2. Master Channel
	if(audiosystem.masterchannel.AutomateBeginEdit(ao,atime,pindex)==false)
		return false;

	// 3. Bus
	AudioChannel *ac=audiosystem.FirstChannelType(CHANNELTYPE_BUSCHANNEL);
	while(ac)
	{
		if(ac->AutomateBeginEdit(ao,atime,pindex)==false)
			return false;

		ac=ac->NextChannel();
	}

	return true;
}

void Seq_Song::AutomateEndEdit(AutomationObject *ao,OSTART atime,int index)
{
	// 1. Tracks
	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->AutomateEndEdit(ao,atime,index);
		t=t->NextTrack();
	}

	// 2. Master Channel
	audiosystem.masterchannel.AutomateEndEdit(ao,atime,index);

	// 3. Bus
	AudioChannel *ac=audiosystem.FirstChannelType(CHANNELTYPE_BUSCHANNEL);
	while(ac)
	{
		ac->AutomateEndEdit(ao,atime,index);
		ac=ac->NextChannel();
	}
}

void Seq_Song::Automate(AutomationObject *ao,OSTART atime,int pindex,double value,int iflag)
{
	if(atime<0)
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"Automate<0");
#endif
		return;
	}

	// 1. Tracks
	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->CheckAutomate(ao,atime,pindex,value,iflag)==true)
			return;

		t=t->NextTrack();
	}

	// 2. Master Channel
	if(audiosystem.masterchannel.CheckAutomate(ao,atime,pindex,value,iflag)==true)
		return;

	// 3. Bus
	AudioChannel *ac=audiosystem.FirstChannelType(CHANNELTYPE_BUSCHANNEL);
	while(ac)
	{
		if(ac->CheckAutomate(ao,atime,pindex,value,iflag)==true)
			return;

		ac=ac->NextChannel();
	}

	// New Automation Tracks ?
}

AutomationParameter *AutomationTrack::FirstAutomationParameter()
{
	AutomationParameter *ap=(AutomationParameter *)parameter.GetRoot();

	while(ap && (ap->flag&AutomationParameter::AP_DELETED))
		ap=ap->NextAutomationParameter();

	return ap;
}

AutomationParameter *AutomationTrack::FindAutomationParameterBefore(OSTART pos)
{
	AutomationParameter *ap=(AutomationParameter *)parameter.FindOBefore(pos);

	while(ap && (ap->flag&AutomationParameter::AP_DELETED))
		ap=ap->NextAutomationParameter();

	while(ap && ap->NextAutomationParameter() && ap->NextAutomationParameter()->GetParameterStart()==pos)
		ap=ap->NextAutomationParameter();

	return ap; 
}

AutomationParameter *AutomationTrack::FindAutomationParameter(OSTART pos)
{
	AutomationParameter *ap=(AutomationParameter *)parameter.FindObject(pos);

	while(ap && (ap->flag&AutomationParameter::AP_DELETED))
		ap=ap->NextAutomationParameter();

	return ap;
}

AutomationParameter *AutomationParameter::NextAutomationParameter()
{
	AutomationParameter *ap=(AutomationParameter *)next;

	while(ap && (ap->flag&AutomationParameter::AP_DELETED))
		ap=ap->NextAutomationParameter();

	return ap;
}

void AutomationTrack::CreateAndAddParameter(OSTART atime,double value,Seq_Song *song,int iflag)
{
	if(AutomationParameter *nap=new AutomationParameter)
	{
		nap->ostart=atime;
		nap->value=value;

		if(touchlatch==true)
		{
			DeleteTouchLatchParameter(atime,true);
			nap->flag=AutomationParameter::AP_TOUCHLATCH;
		}

		/*
		if(iflag&AEF_USEREDIT)
		{
		nap->curvetype=CT_LINEAR;
		}
		else
		{
		//if(touchlatch==true)
		nap->curvetype=CT_ABSOLUT;
		//else
		//	parm->curvetype=curvetype;
		}
		*/
		nap->curvetype=curvetype;

		AddParameter(nap,0);

		parameter.EndIndex();

		if(song)
		{
			if(iflag&AEF_UNDO)
				mainedit->CreateAutomationParameter(song,this,nap);
		}
	}
}

void AutomationTrack::CloneParameter(OListStart *list)
{
	AutomationParameter *ap=FirstAutomationParameter();

	while(ap)
	{
		if(AutomationParameter *nap=new AutomationParameter)
		{
			nap->curvetype=ap->curvetype;
			nap->ostart=ap->ostart;
			nap->value=ap->value;
			nap->flag=ap->flag;

			list->AddEndO(nap);
		}

		ap=ap->NextAutomationParameter();
	}
}

void AutomationTrack::SetAutomationMode(int m)
{
	if(m!=automode)
	{
		automode=m;

		OSTART spp=GetSong()->GetSongPosition();
		automationstarttime=automationendtime=spp;
		touchlatch=false;
	}
}

void AutomationTrack::DeleteTouchLatchParameter(OSTART atime,bool deletetrue)
{
	if(touchlatch==true || automode==ATMODE_WRITE)
	{
		// Delete Parameter START <-> atime
		AutomationParameter *ap=FindAutomationParameter(automationstarttime);

		while(ap && ap->GetParameterStart()<=atime)
		{
			AutomationParameter *nap=ap->NextAutomationParameter();

			if(deletetrue==true && (ap->flag&AutomationParameter::AP_DELETED))
				DeleteParameter(ap,true);
			else
			{
				if(!(ap->flag&AutomationParameter::AP_TOUCHLATCH))
				{
					if(deletetrue==true)
						DeleteParameter(ap,true);
					else
					{
						ap->UnSelect();
						ap->flag|=AutomationParameter::AP_DELETED;
						parameter.accesscounter++;
					}
				}
			}

			ap=nap;
		}
	}
}

void AutomationTrack::DeleteParameter(AutomationParameter *parm,bool full)
{
	if(full==true)
		parameter.RemoveO(parm);
	else
		parameter.CutObject(parm); // undo etc..
}

void AutomationTrack::SortParameter(AutomationParameter *parm)
{
	parameter.AddOSort(parm);
}

void AutomationTrack::CreateStartParameter000()
{
	AddParameter(new AutomationParameter(0,AutomationParameter::AP_DONTDELETE|AutomationParameter::AP_DONTMOVE),0,APINIT_SETCURVE);
}

void AutomationTrack::CreateStartParameter001()
{
	AddParameter(new AutomationParameter(1,AutomationParameter::AP_DONTDELETE|AutomationParameter::AP_DONTMOVE),0,APINIT_SETCURVE);
}

void AutomationTrack::CreateStartParameter0005()
{
	AddParameter(new AutomationParameter(0.5,AutomationParameter::AP_DONTDELETE|AutomationParameter::AP_DONTMOVE),0,APINIT_SETCURVE);
}


void AutomationTrack::AddParameter(AutomationParameter *parm,int iflag)
{
	if(iflag&APINIT_SETCURVE)
	{
		parm->curvetype=curvetype;
	}

	parameter.AddOSort(parm);
}

void AutomationTrack::AddParameter(AutomationParameter *parm,OSTART s,int iflag)
{
	parm->ostart=s;
	AddParameter(parm,iflag);
}

void AutomationTrack::InsertOldIndex()
{
	if(track)
		track->automationtracks.AddOToIndex(this,old_index);
	else
		if(audiochannel)
			track->automationtracks.AddOToIndex(this,old_index);
}

void AutomationTrack::Cut()
{
	old_index=GetIndex();

	if(track)
	{	
		track->automationtracks.CutObject(this);
	}
	else
		if(audiochannel)
		{
			audiochannel->automationtracks.CutObject(this);
		}
}

void AutomationTrack::InitPlayback(OSTART position)
{
	automationstarttime=automationendtime=position;

	if(CanAutomation()==true && CanPlayAutomation()==true)
		bindtoautomationobject->SetValue(this,GetValueAtPosition(position),true);	
}

AutomationObject::AutomationObject(){

	scalefactor=0.5;
	value=0.5;
	staticobject=true;
	hasspecialdata=false;
	automationobjectid=ID_STATICOBJECT;

	id=-1;
	sysid=-1;
	automationtrack=0;
}

bool AutomationObject::BeginEdit(Seq_Song *song,int index)
{
	if(song)
	{
		OSTART spp=song->GetSongPosition();

		return song->AutomateBeginEdit(this,spp,index);
	}

	return false;

	//return automationtrack->SetAutomationTracksTouchLatch(&io.audioeffects.volume,0,spp);
}

void AutomationObject::EndEdit(Seq_Song *song,int index)
{
	if(song)
	{
		OSTART spp=song->GetSongPosition();
		song->AutomateEndEdit(this,spp,index);
	}

	//	SetAutomationTracksTouchLatch(this,0,song->GetSongPosition(),true); // Reset
}

bool AutomationObject::AutomationEdit(Seq_Song *song,OSTART time,int index,double par,int iflag)
{
#ifdef DEBUG
	if(par<0 || par>1)
		maingui->MessageBoxOk(0,"Auto Value");
#endif

	TRACE ("Automation Time %d Index %d \n",time,index);
	TRACE ("New Par  %f\n",par);
	TRACE ("Old Par (0) %f\n",value);

	/*
	• Yellow for Volume automation
	• Green for Pan automation
	• Yellow for Solo automation
	*/

	bool ok=false;

	if(mainedit->CheckIfEditOK(song)==false)
		return false;


	//if(song->CanAutomate(this,index)==false)
	//	return false;

	if(iflag & AEF_USEREDIT)
	{
		song->UserEditAutomation(this,index);
	}

	if(song==mainvar->GetActiveSong())
	{
		mainthreadcontrol->Lock(CS_audioplayback);
		mainMIDIalarmthread->Lock();
	}

	if(iflag & AEF_USEREDIT)
	{
		// Mouse, User Edit
		if(song)
		{
			GetAutoObject()->SetParm(index,par); // Plugins, Audio
			GetAutoObject()->ConvertValueToIntern(); // MIDI,Sys

			song->Automate(GetAutoObject(),time,index,par,iflag);
			ok=true;
		}
	}
	else // Realtime
	{
		if(!(iflag&AEF_FROMOBJECT))
		{
			ok=GetAutoObject()->SetParm(index,par);  // Plugins, Audio
			GetAutoObject()->ConvertValueToIntern();// MIDI,Sys
		}
		else
			ok=true;

		if(song && ok==true)
		{
			song->Automate(GetAutoObject(),time,index,par,iflag);
		}
	}

	if(song==mainvar->GetActiveSong())
	{
		mainMIDIalarmthread->Unlock();
		mainthreadcontrol->Unlock(CS_audioplayback);
	}

	return ok;
}

void Seq_Song::SetVisibleOfAutomationTrack(AutomationTrack *t,bool visible)
{
	t->visible=visible;
	maingui->RefreshAllEditors(this,EDITORTYPE_ARRANGE,0);
}

void Seq_Song::SetAutomationModeOfTracks(AutomationTrack *at,int mode)
{
	if(!at)
		return;

	at->SetAutomationMode(mode);
}

void Seq_Song::ToggleShowAutomationChannels(AudioChannel *t)
{
	if(!t)
		return;

	bool visible=t->showautomationstracks==true?false:true;

	for(int i=0;i<LASTCHANNELTYPE;i++)
	{
		AudioChannel *c=i==CHANNELTYPE_MASTER?&audiosystem.masterchannel:audiosystem.FirstChannelType(i);

		while(c)
		{
			if(c==t || (maingui->GetCtrlKey()==false && c->IsSelected()==true && t->IsSelected()==true))
			{
				c->showautomationstracks=visible;
			}

			c=c->NextChannel();
		}
	}


	maingui->RefreshAllEditors(this,EDITORTYPE_ARRANGE,0);
	maingui->RefreshAllEditors(this,EDITORTYPE_ARRANGELIST,0);
}

void Seq_Song::ToggleShowAutomationTracks(Seq_Track *t)
{
	if(!t)
		return;

	if(t->ismetrotrack==true)
		return;

	bool visible=t->showautomationstracks==true?false:true;

	Seq_Track *track=FirstTrack();

	while(track)
	{
		if(track==t || (maingui->GetCtrlKey()==false && track->IsSelected()==true && t->IsSelected()==true))
		{
			track->showautomationstracks=visible;
		}

		track=track->NextTrack();
	}

	maingui->RefreshAllEditors(this,EDITORTYPE_ARRANGE,0);
	maingui->RefreshAllEditors(this,EDITORTYPE_ARRANGELIST,0);
}

void Seq_Song::SetAutomationTracksPosition(OSTART pos)
{
	InitPlay init(INITPLAY_MIDITRIGGER);

	init.audioautomationstart=init.position=pos;

	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->InitAutomationTracks(&init);
		t=t->NextTrack();
	}

	audiosystem.masterchannel.InitAutomationTracks(&init);

	AudioChannel *ac=audiosystem.FirstBusChannel();
	while(ac)
	{
		ac->InitAutomationTracks(&init);
		ac=ac->NextChannel();
	}
}


/*
void AudioObjectTrack::Delete(bool full)
{
AudioObject *d,*ao=FirstAudioObject();

while(ao)
{
d=ao;
ao=ao->NextAudioObject();

d->Delete(full);
}

if(full==true)
track->audioautomationtracks.RemoveO(this);
else
track->audioautomationtracks.DeleteObject(this);
}*/


char *AutomationTrack::GetAutomationTrackName()
{
	if(bindtoautomationobject)
		return bindtoautomationobject->GetParmName(bindtoautomationobject_parindex);

	return 0;
}

void AutomationTrack::Refresh()
{
	InitPlayback(GetSong()->GetSongPosition());
}

void AutomationTrack::InitAutomationTrackPlayback(InitPlay *init,bool send)
{
	if(!bindtoautomationobject)
		return;

	OSTART position;

	if(bindtoautomationobject->IsAudio()==true)
	{
		position=init->audioautomationstart;
	}
	else
		position=init->position;

	InitPlayback(position);
}

AutomationTrack::AutomationTrack() // MIDI/AUDIO
{
	id=OBJ_AUTOMATIONTRACK;

	// id set later
	flag=0;

	track=0;
	audiochannel=0;

	visible=false;
	tracktype=SBTYPE_ABS;

	bindtoautomationobject=0; // Should NEVER be 0
	bindtoautomationobject_parindex=0;

	automode=ATMODE_OFF;
	automationstarttime=automationendtime=0; // Touch Latch ...
	touchlatch=false;
}

void AutomationTrack::FreeMemory()
{
	parameter.DeleteAllO();

	if(bindtoautomationobject && bindtoautomationobject->staticobject==false)
	{		
		delete bindtoautomationobject;
		bindtoautomationobject=0;
	}
}

void AutomationTrack::SetLinear(LONGLONG start)
{
#ifdef OLDIE
	//if(activeobject && activeobject->NextAutomationObject() && activeobject->GetParameterStart()<=start && activeobject->NextAutomationObject()->GetParameterStart()>start)
	{
		ARES v=activeobject->GetValue(),v2=activeobject->NextAutomationObject()->GetValue();
		//OSTART s1=activeobject->GetParameterStart(),s2=activeobject->NextAutomationObject()->GetParameterStart();

		double diff=playbackobject_samplestart-activeobject_samplestart,
			sdiff=start-activeobject_samplestart;

		sdiff/=diff;

		double vdiff=v2-v;

		vdiff*=sdiff;
		v+=vdiff; // Add/Sub

		SendAutoAudioOut(audiochannel,activeobject,v);
	}
#endif
}

bool AutomationTrack::CanAutomation()
{
	if(!FirstAutomationParameter())
		return false;

	if(track && track->automationon==false)
		return false;

	if(audiochannel && audiochannel->automationon==false)
		return false;

	return automode!=ATMODE_OFF?true:false;
}

bool AutomationTrack::CanPlayAutomation()
{
	if(GetSong()->playbacksettings.automationplayback==false)
		return false;

	if(track && track->automationon==false)
		return false;

	if(audiochannel && audiochannel->automationon==false)
		return false;

	switch(automode)
	{
	case ATMODE_OFF:
		return false;
		break;

	case ATMODE_READ:
		return true;
		break;

	case ATMODE_LATCH:
	case ATMODE_TOUCH:
		if(touchlatch==true)
			return false;
		else
			return true;
		break;

		/*
		case ATMODE_LATCH:
		return true;
		break;

		case ATMODE_WRITE:
		return true;
		break;
		*/
	}

	return false;
}

void AutomationObject::LoadWithOutID(camxFile *file)
{
	if(file->GetChunkHeader()==CHUNK_AUTOMATIONOBJECTDATA)
	{
		file->ChunkFound();
		file->ReadAndAddClass((CPOINTER)this);
		file->ReadChunk(&value);
		file->CloseReadChunk();
	}
}

void AutomationObject::ResetAutomationParameter(AutomationParameter *ap,int index)
{
	ap->value=0.5;
}

void AutomationObject::Load(camxFile *file)
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUTOMATIONOBJECT)
	{
		file->ChunkFound();
		int rid;
		file->ReadChunk(&rid);

#ifdef DEBUG
		if(rid!=id)
			maingui->MessageBoxError(0,"AO Load rid!=id");
#endif

		file->CloseReadChunk();

		file->LoadChunk();
		LoadWithOutID(file);
		ConvertValueToIntern();
	}
}

void AutomationObject::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUTOMATIONOBJECT);
	file->Save_Chunk(id);
	file->CloseChunk();

	file->OpenChunk(CHUNK_AUTOMATIONOBJECTDATA);
	file->Save_Chunk((CPOINTER)this);
	file->Save_Chunk(value);
	file->CloseChunk();
}

double AutomationTrack::GetValueAtPosition(OSTART pos)
{
	if(AutomationParameter *ap=FindAutomationParameterBefore(pos))
	{
		if(AutomationParameter *nap=ap->NextAutomationParameter())
		{
			switch(ap->curvetype)
			{	
			case CT_LINEAR:
				{
					double p=nap->GetParameterStart()-ap->GetParameterStart();
					double h=pos-ap->GetParameterStart();

					h/=p; // 0-1

					double p1=ap->GetParameterValue();
					double p2=nap->GetParameterValue();
					double diff=p2-p1;

					diff*=h;

					//	ARES v=p1+diff;

					//	TRACE ("CT L %f \n",v);

					return p1+diff;
				}
				break;
			}
		}

		return ap->GetParameterValue();
	}

	return bindtoautomationobject->GetParm(bindtoautomationobject_parindex);
}

double AutomationTrack::GetValueAtSamplePosition(Seq_Song *song,LONGLONG s)
{
	return GetValueAtPosition(song->timetrack.ConvertSamplesToTicks(s));
}

void AutomationObject::SetValue(AutomationTrack *at,double v,bool force)
{
	v=ConvertValueToAutomationSteps(v);

	if(value!=v || force==true)
	{
		value=v;
		SendNewValue(at); // v
	}
}

Seq_Song *AutomationTrack::GetSong()
{
	return track?track->song:audiochannel->song;
}

void AutomationTrack::Load(camxFile *file)
{
	file->AddPointer((CPOINTER)&bindtoautomationobject);

	file->ReadChunk(&bindtoautomationobject_parindex);
	file->ReadChunk(&visible);
	file->ReadChunk(&flag);
	file->ReadChunk(&automode);
	file->ReadChunk(&curvetype);

	// ID of AObject
	int aoid=0;
	file->ReadChunk(&aoid);

	switch(aoid)
	{
	case ID_MIDICONTROL:
		bindtoautomationobject=new AT_MIDI_Control;
		break;

	case ID_MIDIPITCHBEND:
		bindtoautomationobject=new AT_MIDI_Pitchbend;
		break;

	case ID_MIDICHANNELPRESSURE:
		bindtoautomationobject=new AT_MIDI_Channelpressure;
		break;

	case ID_MIDIPOLYPRESSURE:
		bindtoautomationobject=new AT_MIDI_Polypressure(0);
		break;

	case ID_MIDIPROGRAM:
		bindtoautomationobject=new AT_MIDI_Program;
		break;

	case ID_AUDIOPLUGIN:
		bindtoautomationobject=new AT_AUDIO_Plugin(0,0);
		break;

	case ID_AUDIOPLUGINBYPASS:
		bindtoautomationobject=new AT_AUDIO_PluginByPass(0);
		break;
	}

	if(bindtoautomationobject)
	{
		bindtoautomationobject->automationtrack=this;
	}

	file->CloseReadChunk();

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_AUTOMATIONTRACKPARAMETERS)
	{
		file->ChunkFound();
		int nr=0;
		file->ReadChunk(&nr);
		file->CloseReadChunk();

		while(nr--)
		{
			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_AUTOMATIONTRACKPARAMETER)
			{
				file->ChunkFound();

				if(AutomationParameter *nao=new AutomationParameter)
				{
					file->ReadChunk(&nao->ostart);
					file->ReadChunk(&nao->value);
					file->ReadChunk(&nao->flag);
					file->ReadChunk(&nao->curvetype);

					parameter.AddEndO(nao);
				}

				file->CloseReadChunk();
			}
		}

		parameter.EndIndex();
	}// End Parameter

	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUTOMATIONOBJECTSPECIALDATA)
	{
		file->ChunkFound();

		if(bindtoautomationobject)
			bindtoautomationobject->LoadSpecialData(file);

		file->CloseReadChunk();

		file->LoadChunk();
	}

	if(bindtoautomationobject && bindtoautomationobject->GetAutoObject() && bindtoautomationobject->GetAutoObject()->IsAudio()==true)
	{
		int channels=track?track->io.GetChannels():audiochannel->io.GetChannels();

		AudioObject *audioobject=(AudioObject *)bindtoautomationobject->GetAutoObject();
		audioobject->InitIOChannels(channels);
	}
}

void AutomationTrack::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUTOMATIONTRACK);

	file->Save_Chunk((CPOINTER)bindtoautomationobject);
	file->Save_Chunk(bindtoautomationobject_parindex);
	file->Save_Chunk(visible);
	file->Save_Chunk(flag);
	file->Save_Chunk(automode);
	file->Save_Chunk(curvetype);

	// ID of AObject
	file->Save_Chunk(bindtoautomationobject->automationobjectid);

	file->CloseChunk();

	if(int onr=parameter.GetCount())
	{
		file->OpenChunk(CHUNK_AUTOMATIONTRACKPARAMETERS);
		file->Save_Chunk(onr);
		file->CloseChunk();

		AutomationParameter *o=FirstAutomationParameter();
		while(o){

			file->OpenChunk(CHUNK_AUTOMATIONTRACKPARAMETER);

			file->Save_Chunk(o->ostart);
			file->Save_Chunk(o->value);

			int bflag=o->flag;

			bflag CLEARBIT OFLAG_SELECTED;

			file->Save_Chunk(bflag);
			file->Save_Chunk(o->curvetype);

			file->CloseChunk();

			o=o->NextAutomationParameter();
		}
	}

	if(bindtoautomationobject->hasspecialdata==true)
	{
		file->OpenChunk(CHUNK_AUTOMATIONOBJECTSPECIALDATA);
		bindtoautomationobject->SaveSpecialData(file);
		file->CloseChunk();
	}
}

class menu_autoonoff:public guiMenu
{
public:
	menu_autoonoff(Seq_Track *t,AudioChannel *c,bool o)
	{
		track=t;
		channel=c;
		on=o;
	}
	void MenuFunction();
	Seq_Track *track;
	AudioChannel *channel;
	bool on;
};

void menu_autoonoff::MenuFunction()
{
	if(track)
	{
		Seq_Track *t=track->song->FirstTrack();

		while(t)
		{
			if(t->IsPartOfEditing(track)==true)
			{
				t->automationon=on;
			}

			t=t->NextTrack();
		}

		return;
	}

	if(channel)
	{
		channel->automationon=on;
	}
}

void GUI::CreateAutomationTrack(Seq_Track *track,AudioChannel *channel,AutomationTrack *prev)
{
	Edit_CreateAutomationTrack *cat=(Edit_CreateAutomationTrack *)OpenEditor(EDITORTYPE_CREATEAUTOMATIONTRACKS,track?track->song:channel->audiosystem->song,0,0,0,track,channel);

	if(cat)
	{
		cat->prevautomationtrack=prev;
	}
}

class menu_createnewautomationtrack:public guiMenu
{
public:
	menu_createnewautomationtrack(Seq_Track *t,AudioChannel *c,AutomationTrack *p)
	{
		track=t;
		channel=c;
		prev=p;
	}

	void MenuFunction();
	Seq_Track *track;
	AudioChannel *channel;
	AutomationTrack *prev;
};

void menu_createnewautomationtrack::MenuFunction()
{
	maingui->CreateAutomationTrack(track,channel,prev);
}

void Seq_Song::EditAutomation(guiWindow *win,AutomationTrack *at)
{
	if(win && at)
	{
		if(win->DeletePopUpMenu(true))
		{	
			win->popmenu->AddMenu(at->GetAutomationTrackName(),0);
			win->popmenu->AddLine();

			class menu_selectmode:public guiMenu
			{
			public:
				menu_selectmode(AutomationTrack *t,int m){track=t;mode=m;}

				void MenuFunction()
				{
					track->SetAutomationMode(mode);
				} //

				AutomationTrack *track;
				int mode;
			};

			for(int i=0;i<5;i++)
			{
				win->popmenu->AddFMenu(automationtrack_mode_names[i],new menu_selectmode(at,i),at->automode==i?true:false);
			}

			win->popmenu->AddLine();

			class menu_delautomationtrack:public guiMenu
			{
			public:
				menu_delautomationtrack(AutomationTrack *t){track=t;}

				void MenuFunction()
				{
					mainedit->DeleteAutomationTrack(track);
				} //

				AutomationTrack *track;
			};
			win->popmenu->AddFMenu(Cxs[CXS_DELETEAUTOTRACK],new menu_delautomationtrack(at));	

			win->popmenu->AddLine();
			win->popmenu->AddFMenu(Cxs[CXS_CREATENEWAUTOTRACK],new menu_createnewautomationtrack(at->track,at->audiochannel,at));
			win->ShowPopMenu();
		}
	}
}

void Seq_Song::EditAutomationSettings(guiWindow *win,Seq_Track *track,AudioChannel *channel)
{
	if(track && track->ismetrotrack==true)
		return;

	if(win)
	{
		TrackHead *head;

		if(track)
			head=track;
		else
			head=channel;

		if(win->DeletePopUpMenu(true))
		{	
			win->popmenu->AddFMenu(Cxs[CXS_AUTOMATIONON],new menu_autoonoff(track,channel,true),head->automationon==true?true:false);
			win->popmenu->AddFMenu(Cxs[CXS_AUTOMATIONOFF],new menu_autoonoff(track,channel,false),head->automationon==false?true:false);

			win->popmenu->AddLine();
			win->popmenu->AddFMenu(Cxs[CXS_CREATENEWAUTOTRACK],new menu_createnewautomationtrack(track,channel,0));
			win->ShowPopMenu();
		}
	}
}

void Seq_Song::DeleteDeletedAutomationParameter()
{
	// 1. Tracks
	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->DeleteDeletedAutomationParameter();

		t=t->NextTrack();
	}

	// 2. Master Channel
	audiosystem.masterchannel.DeleteDeletedAutomationParameter();

	// 3. Bus
	AudioChannel *ac=audiosystem.FirstChannelType(CHANNELTYPE_BUSCHANNEL);
	while(ac)
	{
		ac->DeleteDeletedAutomationParameter();

		ac=ac->NextChannel();
	}
}

void Seq_Song::ResetAutomationTracks()
{
	// 1. Tracks
	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->ResetAutomationTracks();
		t=t->NextTrack();
	}

	// 2. Master Channel
	audiosystem.masterchannel.ResetAutomationTracks();

	// 3. Bus
	AudioChannel *ac=audiosystem.FirstChannelType(CHANNELTYPE_BUSCHANNEL);
	while(ac)
	{
		ac->ResetAutomationTracks();

		ac=ac->NextChannel();
	}
}
