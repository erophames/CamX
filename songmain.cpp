#include "songmain.h"
#include "gui.h"
#include "camxfile.h"
#include "MIDIoutproc.h"
#include "audiofile.h"
#include "languagefiles.h"
#include "audiohdfile.h"
#include "audiohardware.h"
#include "object_project.h"
#include "object_song.h"
#include "object_track.h"
#include "settings.h"
#include "audiopattern.h"
#include "semapores.h"
#include "audiodevice.h"

char *Seq_Main::GetQuantString(int q_nr)
{
	return(quantstr[q_nr]);
}

/*
void Seq_Main::FreeStringBuffer()
{
if(stringbuffer)delete stringbuffer;
stringbuffer=0;
}
*/

/*
char *Seq_Main::GetStringBuffer(int length)
{
FreeStringBuffer();
stringbuffer=new char[length+1];
return stringbuffer;
}
*/

void Seq_Song::DeleteUnUsedAudioFiles()
{
	if(directoryname && mainsettings->flag_unusedaudiofiles!=Settings::SETTINGS_NODeInit)
	{
		// Delete all unused Audio Files in Song Directory
		camxFile list;
		list.BuildDirectoryList(directoryname,"*.*",".wav");

		if(list.FirstScan())
		{
			Seq_Track *track=FirstTrack();

			while(track)
			{
				if(track->frozen==true && track->frozenfile)
				{
					// Find in Scan
					camxScan *s=list.FirstScan();

					while(s)
					{
						if(s->flag==0 && strcmp(s->name,track->frozenfile)==0)
							s->flag=1; // mark as used

						s=s->NextScan();
					}
				}

				bool tmp_frozen=track->frozen;
				track->frozen=false;

				Seq_Pattern *ap=track->FirstPattern(MEDIATYPE_AUDIO);

				while(ap)
				{
					AudioPattern *audio=(AudioPattern *)ap;

					if(audio->audioevent.audioefile)
					{
						// Find in Scan
						camxScan *s=list.FirstScan();

						while(s)
						{
							if(s->flag==0 && strcmp(s->name,audio->audioevent.audioefile->GetName())==0)
							{
								track->frozen=tmp_frozen;
								s->flag=1; // mark as used
							}

							s=s->NextScan();
						}
					}

					ap=ap->NextPattern(MEDIATYPE_AUDIO);
				}

				track->frozen=tmp_frozen;

				track=track->NextTrack();
			}

			int c=0;
			camxScan *s=list.FirstScan();

			while(s)
			{
				if(s->flag==0) // Not used
				{
					TRACE ("UnUsed Audio File Song %s %s T:%d\n",songname,s->name,FirstTrack());
					c++;
				}

				s=s->NextScan();
			}

			if(c)
			{
				bool deleteok=false;

				if(mainsettings->flag_unusedaudiofiles==Settings::SETTINGS_AUTODeInit)
					deleteok=true;
				else
					if(mainsettings->flag_unusedaudiofiles==Settings::SETTINGS_AUDIODEINITASK)
					{
						char h2[NUMBERSTRINGLEN];

						if(char *h=mainvar->GenerateString(Cxs[CXS_DELETE]," (",mainvar->ConvertIntToChar(c,h2),") ",Cxs[CXS_UNUSEDSOUNDFILESINDIR]))
						{
							deleteok=maingui->MessageBoxYesNo(0,h);
							delete h;
						}
					}

					if(deleteok==true)
					{
						camxScan *s=list.FirstScan();

						while(s)
						{
							if(s->flag==0) // Not used
							{
								TRACE("Unused File in Song Directory %s\n",s->name);

								AudioHDFile *ad=mainaudio->FindAudioHDFile(s->name);

								if(ad)
								{
									ad->deleted=true;
									/*
									maingui->RemoveAudioHDFileFromGUI(ad);
									mainaudio->DeleteHDFile(ad);
									*/
								}

								mainaudio->DeleteAudioFile(s->name);
							}

							s=s->NextScan();
						}
					}
			}
		}
		list.ClearScan();
	}
}

AutomationTrack *Seq_Song::FirstAutomationTrackWithSelectedParameters()
{
	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(AutomationTrack *at=t->FirstAutomationTrackWithSelectedParameters())
			return at;

		t=t->NextTrack();
	}

	AudioChannel *b=audiosystem.FirstBusChannel();
	while(b)
	{
		if(AutomationTrack *at=b->FirstAutomationTrackWithSelectedParameters())
			return at;
		b=b->NextChannel();
	}

	if(AutomationTrack *at=audiosystem.masterchannel.FirstAutomationTrackWithSelectedParameters())
		return at;

	return 0;
}

bool Seq_Song::IsAutomationParameterSelected()
{
	Seq_Track *t=FirstTrack();
	while(t)
	{
		if(t->IsAutomationParameterSelected()==true)
			return true;

		t=t->NextTrack();
	}

	AudioChannel *bus=audiosystem.FirstBusChannel();
	while(bus)
	{
		if(bus->IsAutomationParameterSelected()==true)
			return true;

		bus=bus->NextChannel();
	}

	if(audiosystem.masterchannel.IsAutomationParameterSelected()==true)
		return true;

	return false;
}

void Seq_Song::FreeMemory(int flag)
{
	bool full=(flag&Seq_Project::DELETESONG_FULL)?true:false;
	//bool songwasactive=flag&Seq_Project::DELETESONG_SONGWASACTIVE?true:false;

	underdeconstruction=full;

	TRACE ("+++> Delete Song %s Full %d\n",songname,full);

	if(full==true && loaded==true)
		DeleteUnUsedAudioFiles();

	SendOpenEvents();

	/*	for(int i=0;i<REALTIME_LISTS;i++)
	realtimeevents[i].DeleteAllREvents(this); // remove Realtime Events
	*/

	//	skipplaybackevents.DeleteAllO();

	timetrack.RemoveAllTimeMaps(full);
	textandmarker.RemoveAllTexts();
	textandmarker.RemoveAllMarker();

	TRACE ("+++> Delete Song A \n");

	// delete songs Undo
	undo.DeleteAllUndos();

	//	DeleteAllFolder();
	DeleteAllGroups();

	drummap.FreeMemory();

	TRACE ("+++> Delete Song B \n");

	audiosystem.CloseAudioSystem(full);

	TRACE ("+++> Delete Song C \n");

	/*
	if(flag&Seq_Project::DELETESONG_NOLOCK)
	{
	Seq_Track *track=FirstTrack();

	while(track)
	track=DeleteTrack(track,true);
	}
	else // No Lock
	{
	*/
	Seq_Track *track=FirstTrack();

	while(track)
		track=DeleteTrack_R(track,true);

	Seq_MetroTrack *mt=FirstMetroTrack();
	while(mt)
	{
		mt=DeleteMetroTrack(mt);
	}

	// }

	TRACE ("+++> Delete Song CE \n");
	qtracklist.FreeMemory();
	TRACE ("+++> Delete Song D  \n");

	if(!(flag&Seq_Project::DELETESONG_ONLYCLOSE))
	{
		if(songname){
			delete songname;
			songname=0;
		}

		if(directoryname){
			delete directoryname;
			directoryname=0;
		}
	}

	loaded=false;
	loadwhenactivated=false;

	TRACE ("+++> Delete Song E  \n");
	if(full==true)
	{
		playbacksettings.FreeMemory();
		inputrouting.FreeMemory();

		if(masterfile){
			delete masterfile;
			masterfile=0;
		}

		if(masterdirectoryselectedtracks)
		{
			delete masterdirectoryselectedtracks;
			masterdirectoryselectedtracks=0;
		}

		events_in.DeleteAllO();
		events_out.DeleteAllO();

		delete this;
	}
	else
		underdeconstruction=false;

	TRACE ("+++> Delete Song Done \n");
}

void Seq_Song::RefreshLoops()
{
	Seq_Track *t=FirstTrack();

	while(t)
	{
		Seq_Pattern *p=t->FirstPattern(MEDIATYPE_ALL);

		while(p){

			if(p->loops)
				p->LoopPattern();

			p=p->NextPattern(MEDIATYPE_ALL);
		}

		t=t->NextTrack();
	}
}

void Seq_Song::DeleteAllVirtualTempos()
{
	{
		Seq_Tempo *t=timetrack.FirstTempo();

		while(t)
		{
			if(t->type==TEMPOEVENT_VIRTUAL)
				goto clearvirtual;

			t=t->NextTempo();
		}

		return;
	}

clearvirtual:

	switch(project->realtimerecordtempoevents)
	{
	case Settings::REALTIMERECTEMPOEVENTS_CHANGE:
		{
			BufferBeforeTempoChanges();

			Seq_Tempo *t=timetrack.FirstTempo();

			while(t)
			{
				if(t->type==TEMPOEVENT_VIRTUAL)
				{
					Seq_Tempo *p=t->PrevTempo();

					while(p && p->type==TEMPOEVENT_VIRTUAL)
						p=p->PrevTempo();

					if(p)
					{

					}

				}

				t=t->NextTempo();
			}

		}
		break;

	case Settings::REALTIMERECTEMPOEVENTS_CONVERT:
		{
			// Virtual -> Real
			Seq_Tempo *t=timetrack.FirstTempo();

			while(t)
			{
				if(t->type==TEMPOEVENT_VIRTUAL)
					t->type=TEMPOEVENT_REAL;

				t=t->NextTempo();
			}

			maingui->RefreshTempoGUI(this);
			return;
		}
		break;


	case Settings::REALTIMERECTEMPOEVENTS_DELETE:
		{
			BufferBeforeTempoChanges();

		}
		break;

	}

deletevirtual:

	mainthreadcontrol->LockActiveSong();

	Seq_Tempo *t=timetrack.FirstTempo();

	while(t)
	{
		if(t->type==TEMPOEVENT_VIRTUAL)
		{
			t=timetrack.RemoveTempo(t);
		}
		else
			t=t->NextTempo();

	}

	timetrack.Close();

	bool loopsrepaired=false;

	loopsrepaired=RepairLoops(Seq_Song::RLF_NOLOCK|Seq_Song::RLF_CHECKREFRESH);

	mainthreadcontrol->UnlockActiveSong();

	if(loopsrepaired==true)
		maingui->RefreshRepairedLoopsGUI(this);

	maingui->RefreshTempoGUI(this);
}

bool Seq_Song::AddVirtualTempoAtPosition(OSTART position,double tempo,int flag)
{
	if(!(status&Seq_Song::STATUS_WAITPREMETRO))
	{
		BufferBeforeTempoChanges();

		mainthreadcontrol->LockActiveSong();

		int cflag=(status&Seq_Song::STATUS_RECORD) && mainsettings->allowtempochangerecording==true?TEMPOEVENT_REAL:TEMPOEVENT_VIRTUAL;

		timetrack.AddNewTempo(cflag,position,tempo);
		timetrack.Close();

		bool loopsrepaired=false;

		if(!(flag&TEMPOREFRESH_NOLOOPREFRESH))
			loopsrepaired=RepairLoops(Seq_Song::RLF_NOLOCK|Seq_Song::RLF_CHECKREFRESH);

		mainthreadcontrol->UnlockActiveSong();

		if(!(flag&TEMPOREFRESH_NOGUI))
		{
			if(loopsrepaired==true)
				maingui->RefreshRepairedLoopsGUI(this);

			maingui->RefreshTempoGUI(this);
		}

		return true;
	}

	return false;
}

void Seq_Song::BufferBeforeTempoChanges()
{
	buffer_playback_songposition=timetrack.ConvertSamplesToTicks(playback_sampleposition);
	buffer_record_songposition[0]=timetrack.ConvertSamplesToTicks(record_sampleposition[0]);
	buffer_record_songposition[1]=timetrack.ConvertSamplesToTicks(record_sampleposition[1]);

	buffer_stream_songposition=timetrack.ConvertSamplesToTicks(stream_samplestartposition);

	MIDIsync.BufferBeforeTempoChanges();

	for(int i=0;i<REALTIME_LISTS;i++)
		realtimeevents[i].BufferBeforeTempoChanges();

	for(int i=0;i<INITPLAY_MAX;i++)
	{
		buffer_nextMIDIplaybacksampleposition[i]=timetrack.ConvertSamplesToTicks(nextMIDIplaybacksampleposition[i]);
		buffer_nextMIDIchaineventsampleposition[i]=timetrack.ConvertSamplesToTicks(nextMIDIchaineventsampleposition[i]);
		
	}

	if(status&STATUS_SONGPLAYBACK_MIDI)
		buffer_MIDI_songposition=GetSongPosition();

}

void Seq_Song::RefreshTempoBuffer()
{
	metronome.RefreshTempoBuffer();
	MIDIsync.RefreshTempoBuffer();

	ConvertTicksToTempoSampleRate();

	playback_sampleposition=timetrack.ConvertTicksToTempoSamples(buffer_playback_songposition);
	
	record_sampleposition[0]=timetrack.ConvertTicksToTempoSamples(buffer_record_songposition[0]);
	record_sampleposition[1]=timetrack.ConvertTicksToTempoSamples(buffer_record_songposition[1]);

	stream_samplestartposition=timetrack.ConvertTicksToTempoSamples(buffer_stream_songposition);

	for(int i=0;i<REALTIME_LISTS;i++)
		realtimeevents[i].RefreshTempoBuffer();

	for(int i=0;i<INITPLAY_MAX;i++)
	{
		nextMIDIplaybacksampleposition[i]=timetrack.ConvertTicksToTempoSamples(buffer_nextMIDIplaybacksampleposition[i]);
		nextMIDIchaineventsampleposition[i]=timetrack.ConvertTicksToTempoSamples(buffer_nextMIDIchaineventsampleposition[i]);
	}

	if(status&STATUS_SONGPLAYBACK_MIDI)
	{
		MIDI_samplestartposition=MIDI_sampleendposition=timetrack.ConvertTicksToTempoSamples(buffer_MIDI_songposition); // <-Start
		InitMIDIIOTimer(buffer_MIDI_songposition); // Reset MIDI Engine Timer
		mainMIDIalarmthread->SetSignal();
	}
}

void Seq_Song::RefreshMuteBuffer()
{
	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->io.audioeffects.SetMute(t->t_mutebuffer);
		t=t->NextTrack();
	}

	SetMuteFlags();
}

void Seq_Song::UnMuteAllTracks()
{
	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->t_mutebuffer=t->io.audioeffects.GetMute();
		t->io.audioeffects.SetMute(false);

		t=t->NextTrack();
	}

	SetMuteFlags();
}

void Seq_Song::RefreshSoloBuffer()
{
	mainthreadcontrol->LockActiveSong();

	Seq_Track *t=FirstTrack();

	while(t){

		t->SetSolo(t->solobuffer);

		t=t->NextTrack();
	}

	mainthreadcontrol->UnlockActiveSong();

	SetMuteFlags();
}

void Seq_Song::UnSoloAllTracks()
{
	mainthreadcontrol->LockActiveSong();

	Seq_Track *t=FirstTrack();

	while(t){

		t->solobuffer=t->GetSolo();
		t->SetSolo(false);

		t=t->NextTrack();
	}

	mainthreadcontrol->UnlockActiveSong();

	SetMuteFlags();
}

void Seq_Song::ClearAllUnderSelectionPattern()
{
	Seq_Track *t=FirstTrack();

	while(t)
	{
		Seq_Pattern *p=t->FirstPattern();
		while(p)
		{
			p->flag CLEARBIT OFLAG_UNDERSELECTION;
			p=p->NextPattern();
		}

		t=t->NextTrack();
	}
}

char *Seq_Song::CreateWindowTitle()
{
	return mainvar->GenerateString("Song:",songname?songname:"SN?"," Project:",project?project->name:"PN?");
}

void Seq_Song::Delete()
{
	FreeMemory(Seq_Project::DELETESONG_FULL);
}

OSTART Seq_Main::SimpleQuantize(OSTART pos,OSTART qticks)
{
	OSTART l=(pos/qticks)*qticks,r=l+qticks;
	if(pos-l<=r-pos)return l;
	return r;
}

OSTART Seq_Main::SimpleQuantizeLeft(OSTART p,OSTART qticks)
{
	return (p/qticks)*qticks;
}

OSTART Seq_Main::SimpleQuantizeRight(OSTART p,OSTART qticks)
{
	return (p/qticks)*qticks+qticks;
}

void Seq_Main::SetError(int error)
{
	errorflag|=error;
	MessageBeep(-1);

	maingui->MessageBoxOk(0,"Error");
}

void Seq_Main::NewSong(Seq_Project *project,guiScreen *screen)
{
	if(project)
	{
		Seq_Song *song=0;
		bool loaded=false;

		if(mainsettings->importfilequestion==true && maingui->MessageBoxYesNo(0,Cxs[CXS_IMPORTARRANGEMENT_Q])==true)
		{
			camxFile file;
			if(file.OpenFileRequester(screen,0,"Song","CamX Arrangement (*.caar)|*.caar;|All Files (*.*)|*.*||",true)==true)
			{
				if(file.OpenRead(file.filereqname)==true)
				{
					if(project->CheckIfFileIsSong(&file)==true)
					{
						char *h=mainvar->GenerateString(file.filereqname);

						file.Close(true);

						if(h)
						{
							song=project->CreateNewSong(0,Seq_Project::CREATESONG_IMPORTFROMFILE|Seq_Project::CREATESONG_ACTIVATE|Seq_Project::CREATESONG_CREATEAUDIOMASTER,0,h);
							loaded=true;
							delete h;
						}
					}
					else
						file.Close(true);
				}
			}
		}

		if(!song)
			song=project->CreateNewSong(0,Seq_Project::CREATESONG_ACTIVATE|Seq_Project::CREATESONG_CREATEAUDIOMASTER,0,0);

		if(song)
		{
			if(loaded==false)
			{
				if(maingui->AskForDefaultSong(GUI::ID_AUTOLOAD_CAMX,true)==true)
					maingui->LoadDefaultSong(screen,song,false);
			}

			if(screen)
				screen->InitNewSong(song);
		}
	}
}

bool Seq_Main::GetFileName(char *fname,char *string,int l)
{
	if(fname && string && l>1)
	{
		camxFile clone;

		if(clone.OpenRead(fname)==true)
		{
#ifdef WIN32
#undef strncpy
			strncpy(string,clone.file.GetFileName(),l-1);
			string[l]=0;
#endif
			clone.Close(true);

			return true;
		}

		*string=0;
	}

	return false;
}
