#include "songmain.h"
#include "object_song.h"
#include "audiofile.h"
#include "initplayback.h"
#include "transporteditor.h"
#include "editfunctions.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "MIDIhardware.h"
#include "gui.h"
#include "semapores.h"
#include "settings.h"
#include "MIDIoutproc.h"
#include "audioproc.h"
#include "languagefiles.h"
#include "audiohdfile.h"
#include "wavemap.h"
#include "drummap.h"
#include "MIDIPattern.h"
#include "object_track.h"
#include "arrangeeditor.h"
#include "player.h"
#include "audiopattern.h"
#include "chunks.h"
#include "arrangeeditor_fx.h"
#include "editor.h"
#include "pianoeditor.h"
#include "audiohardwarechannel.h"
#include "object_project.h"

char *Song_Playbacksettings::CreateFromToString(int index,int type)
{
	if(fromtostring)delete fromtostring;

	size_t i=name[index]?strlen(name[index]):1;

	if(fromtostring=new char[i+96])
	{
		strcpy(fromtostring,name[index]?name[index]:"?");

		mainvar->AddString(fromtostring,":");

		Seq_Pos pos(Seq_Pos::POSMODE_NORMAL);

		char h[65];
		song->timetrack.ConvertTicksToPos(cyclestart_buffer[index],&pos);
		pos.ConvertToString(song,h,64);
		mainvar->AddString(fromtostring,h);

		if(type==Seq_Marker::MARKERTYPE_DOUBLE)
		{
			mainvar->AddString(fromtostring,"<->");
			song->timetrack.ConvertTicksToPos(cycleend_buffer[index],&pos);
			pos.ConvertToString(song,h,64);
			mainvar->AddString(fromtostring,h);
		}
	}

	return fromtostring;
}

Song_Playbacksettings::Song_Playbacksettings()
{
	solo=0;
	cycleplayback=false;
	cyclestart=0;
	cycleend=2*4*SAMPLESPERBEAT;

	for(int i=0;i<8;i++){
		cyclestart_buffer[i]=0;
		cycleend_buffer[i]=2*4*SAMPLESPERBEAT;
	}

	activecycle=0;
	strcpy(name[0],"Cycle 1");
	strcpy(name[1],"Cycle 2");
	strcpy(name[2],"Cycle 3");
	strcpy(name[3],"Cycle 4");
	strcpy(name[4],"Cycle 5");
	strcpy(name[5],"Cycle 6");
	strcpy(name[6],"Cycle 7");
	strcpy(name[7],"Cycle 8");

	fromtostring=0;

	automationplayback=automationrecording=true;
}

void Song_Playbacksettings::SetCycleMeasure()
{
	cyclestart_measure=song->timetrack.ConvertTicksToMeasure(cyclestart);
	cycleend_measure=song->timetrack.ConvertTicksToMeasure(cycleend);

	if(cycleend_measure<=cyclestart_measure)cycleend_measure=cyclestart_measure+1;

	cyclestart_measure_buffer[activecycle]=song->timetrack.ConvertTicksToMeasure(cyclestart_buffer[activecycle]);
	cycleend_measure_buffer[activecycle]=song->timetrack.ConvertTicksToMeasure(cycleend_buffer[activecycle]);

	if(cycleend_measure_buffer[activecycle]<=cyclestart_measure_buffer[activecycle])
		cycleend_measure_buffer[activecycle]=cyclestart_measure_buffer[activecycle]+1;
}

void Song_Playbacksettings::ConvertTicksToTempoSampleRate()
{
	cycle_samplestart=song->timetrack.ConvertTicksToTempoSamples(cyclestart);
	cycle_sampleend=song->timetrack.ConvertTicksToTempoSamples(cycleend);
}

void Song_Playbacksettings::FreeMemory()
{
	if(fromtostring)
	{
		delete fromtostring;
		fromtostring=0;
	}
}

void Song_Playbacksettings::Load(camxFile *file)
{
	file->ReadChunk(&cycleplayback);
	file->ReadChunk(&solo);

	file->ReadChunk(&cyclestart);
	file->ReadChunk(&cycleend);
	file->ReadChunk(&cyclestart_measure);
	file->ReadChunk(&cycleend_measure);

	file->ReadChunk(&activecycle);

	for(int i=0;i<8;i++)
	{
		file->ReadChunk(&cyclestart_buffer[i]);
		file->ReadChunk(&cycleend_buffer[i]);
		file->ReadChunk(&cyclestart_measure_buffer[i]);
		file->ReadChunk(&cycleend_measure_buffer[i]);
		file->Read_ChunkString(name[i]);
	}

	file->ReadChunk(&automationplayback);
	file->ReadChunk(&automationrecording);

	file->CloseReadChunk();
}

void Song_Playbacksettings::Save(camxFile *file)
{
	file->Save_Chunk(cycleplayback);
	file->Save_Chunk(solo);
	file->Save_Chunk(cyclestart);
	file->Save_Chunk(cycleend);
	file->Save_Chunk(cyclestart_measure);
	file->Save_Chunk(cycleend_measure);

	file->Save_Chunk(activecycle);

	for(int i=0;i<8;i++)
	{
		file->Save_Chunk(cyclestart_buffer[i]);
		file->Save_Chunk(cycleend_buffer[i]);
		file->Save_Chunk(cyclestart_measure_buffer[i]);
		file->Save_Chunk(cycleend_measure_buffer[i]);
		file->Save_ChunkString(name[i]);
	}

	file->Save_Chunk(automationplayback);
	file->Save_Chunk(automationrecording);
}

void Seq_Song::RepairSongAfterLoading()
{
	solocount=0;

	CreateQTrackList();
	ConvertTicksToTempoSampleRate();

	// Refresh Master FX Channels
	audiosystem.masterchannel.io.Repair();
	audiosystem.masterchannel.io.audioeffects.SetLoadParms();
	audiosystem.masterchannel.RepairAutomationTracks();

	//audiosystem.masterchannel.io.SetChannelType(audiosystem.masterchannel.io.channel_type);

	// Refresh Channels/Bus FX Channels

	AudioChannel *ac=audiosystem.FirstBusChannel();
	while(ac)
	{
		ac->io.Repair();
		//ac->io.SetChannelType(ac->io.channel_type);
		ac->io.audioeffects.SetLoadParms();
		ac->RepairAutomationTracks();

		ac=ac->NextChannel();
	}

	{
		Seq_MetroTrack *mt=FirstMetroTrack();
		while(mt)
		{
#ifdef DEBUG
			if(mt->ismetrotrack==false)
				maingui->MessageBoxError(0,"ISMETRO");
#endif

			mt->io.Repair();// Refresh Tracks FX Channels
			mt->RepairAutomationTracks();

			mt=mt->NextMetroTrack();
		}
	}

	Seq_Track *t=FirstTrack();

	while(t)
	{
		t->t_audiofx.Repair(); // Track<->Track Record
		t->io.Repair();// Refresh Tracks FX Channels
		t->RepairAutomationTracks();

		//t->io.SetChannelType(t->io.channel_type);
		t->io.audioeffects.SetLoadParms();
		t->io.audioinputeffects.SetLoadParms();

		// Repair Audio
		Seq_Pattern *a=t->FirstPattern(MEDIATYPE_AUDIO);
		while(a)
		{
			AudioPattern *ap=(AudioPattern *)a;

			if(ap->audioevent.audioregion)
			{
				ap->audioevent.audioregion->r_audiohdfile=ap->audioevent.audioefile;
				ap->audioevent.audioregion->InitRegion();
			}

			// Offsets
			if(ap->offsetregion.r_audiohdfile=ap->audioevent.audioefile)
			{
				ap->offsetregion.InitRegion();
				ap->audioevent.audioefile->Open();// Open Audio File
			}
			else
			{
				ap->offsetstartoffset=0;
				ap->useoffsetregion=false;
			}

			a=a->NextPattern(MEDIATYPE_AUDIO);
		}

		Seq_Pattern *p=t->FirstPattern();

		while(p){

			if(p->itsaloop==false)
			{
				if(p->mainclonepattern) // Add To MainPattern
					p->mainclonepattern->AddClone(p);
			}

			p=p->NextPattern();
		}

		if(t->GetSolo()==true)
			solocount++;

		if(t->t_audiofx.recordtrack)
		{
			t->t_audiofx.AddTrackRecord(t);
		}

		t->checkcrossfade=true;
		t->frozen=t->tmp_frozen;

		t=t->NextTrack();
	}

	textandmarker.Init(); // Loop Song Position
	textandmarker.Close();

	//Freeze loaded Tracks
	t=FirstTrack();

	while(t)
	{

		Seq_Pattern *p=t->FirstPattern();

		while(p){

			if(p->itsaloop==false)
			{
				p->LoopPattern(AFTERLOAD_LOOP);// Create Pattern Loops, after textandmarker.Init(), Song Stop Position
			}

			p=p->NextPattern();
		}

		if(t->frozen==true)
		{
			t->underfreeze=false;
			t->FreezeTrack(FREEZE_LOADOLDFILE);
		}

		t=t->NextTrack();
	}



	CheckCrossFades();
	inputrouting.InitDevices(); // Add Missing Devices
	SetMuteFlags();
	//CalcTrackDelays();

	//	if(file->audiofilesadded)
	//		maingui->RefreshAllEditors(0,EDITORTYPE_AUDIOMANAGER,0);
}

bool Seq_Song::LoadSongInfo(camxFile *file)
{
	bool infofound=false,autoclose=false;
	int flag=0;
	camxFile readfile;

	Seq_Track *loadtrack=0;

	char *sn=0;

	if((!file) && directoryname){

		if(sn=mainvar->GenerateString(directoryname,"\\cpsong.camx")){
			file=&readfile;

			if(file->OpenRead(sn)==true)
			{
			}
			else
				file=0;

			autoclose=true;
		}
	}

	if(file){

		if(file->CheckVersion()==true)
		{
			// Load Chunks	
			while(file->eof==false && infofound==false)
			{
				file->LoadChunk();

				if(file->eof==false)
					switch(file->GetChunkHeader())
				{
					case CHUNK_SONGPOINTER:
						{
							file->ChunkFound();
							file->Read_ChunkString(&songname);
							file->ReadChunk(&flag);
							file->ReadChunk(&rinfo_audiopatterncounter);

							file->CloseReadChunk();
							infofound=true;
						}
						break;

					default: // unknown 
						file->JumpOverChunk();
						break;
				}// switch
			}
		}	
		else
			file->errorflag|=CHUNK_ERROR;
	}

	if(autoclose==true)
		file->Close(true);

	if(sn)
		delete sn;

	return infofound;
}

bool Seq_Song::GetFXFlag(int flag)
{
	return fxshowflag&flag?true:false;
}

void Seq_Song::Load(camxFile *file)
{
	if(loaded==true)
		return;

	camxFile readfile;
	Seq_Track *loadtrack=0;
	char *sn=0;
	int error=0,focustrackindex=0,flag=0;
	bool autoclose=false;

	if((!file) && directoryname){

		if(sn=mainvar->GenerateString(directoryname,"\\cpsong.camx")){

			file=&readfile;

			if(file->OpenRead(sn)==true)
			{
			}
			else
				file=0;

			autoclose=true;
		}
	}

	if(file){

		if(file->CheckVersion()==true)
		{
			// Load Chunks	
			while(file->eof==false)
			{
				file->LoadChunk();

				if(file->eof==false)
					switch(file->GetChunkHeader())
				{
					case CHUNK_SONGROUTING:
						{
							file->ChunkFound();

							inputrouting.FreeMemory(); // Reset
							inputrouting.Load(file);
						}
						break;

					case CHUNK_SONGPLAYBACK:
						{
							file->ChunkFound();
							playbacksettings.Load(file);
						}
						break;

					case CHUNK_SONG_METRO:
						{
							file->ChunkFound();
							metronome.Load(file);
							file->CloseReadChunk();
						}
						break;

					case CHUNK_SONGPOINTER:
						{
							file->ChunkFound();
							file->Read_ChunkString(&songname);
							file->ReadChunk(&flag);

							file->ReadChunk(&rinfo_audiopatterncounter);
						}
						break;

					case CHUNK_SONGSETTINGS:
						{
							file->ChunkFound();

							file->ReadChunk(&notetype);
							file->ReadChunk(&generalMIDI);
							file->ReadChunk(&generalMIDI_drumchannel);
							file->ReadChunk(&songposition);

							file->ReadChunk(&songlength_measure);
							file->ReadChunk(&songlength_ticks);

							file->ReadChunk(&defaultmasterstart);
							file->ReadChunk(&defaultmasterend);
							file->Read_ChunkString(&masterfile);
							file->Read_ChunkString(&masterdirectoryselectedtracks);

							file->ReadChunk(&MIDIsync.sync);
							MIDIsync.syncbeforestart=MIDIsync.sync;

							file->ReadChunk(&smpteoffset.h);
							file->ReadChunk(&smpteoffset.m);
							file->ReadChunk(&smpteoffset.sec);
							file->ReadChunk(&smpteoffset.frame);
							file->ReadChunk(&smpteoffset.minus);

							file->ReadChunk(&MIDIsync.sendmtc);
							file->ReadChunk(&MIDIsync.sendmc);

							file->ReadChunk(&default_masterformat);
							file->ReadChunk(&default_masternormalize);
							file->ReadChunk(&default_mastersavefirst);
							file->ReadChunk(&default_masterpausems);
							file->ReadChunk(&default_masterpausesamples);
							file->ReadChunk(&fxshowflag);
							file->ReadChunk(&freezeindexcounter);
							file->ReadChunk(&mastefilename_autoset);
							file->ReadChunk(&default_masterchannels);
							file->ReadChunk(&pref_tracktype);

							file->CloseReadChunk();
						}
						break;

					case CHUNK_SONGPATTERNLINKLIST:
						{
							file->ChunkFound();
							LoadLinks(file);
						}
						break;

					case CHUNK_SONGGROUPS:
						{
							file->ChunkFound();
							LoadGroups(file);
						}
						break;

					case CHUNK_MIDIGROOVES:
						{
							file->ChunkFound();
							mainMIDI->LoadGrooves(file);
						}
						break;

					case CHUNK_TIMETRACK:
						{
							file->ChunkFound();
							timetrack.Load(file);
						}
						break;

					case CHUNK_TEXTANDMARKER:
						{
							file->ChunkFound();
							textandmarker.Load(file);
						}
						break;

					case CHUNK_SONGTRACKS: 
						{"Audio Pattern (User)Move Mode", //CXS_MOVEBUTTON
							file->ChunkFound();

						// Tracks
						file->ReadChunk(&focustrackindex);
						file->CloseReadChunk();
						}
						break;

					case CHUNK_SONGTRACKSEND:
						{
							TRACE ("Chunk CHUNK_SONGTRACKSEND \n2");

							file->ChunkFound();
							CreateQTrackList();
							file->CloseReadChunk();
						}
						break;

					case CHUNK_TRACK:
						{
							file->ChunkFound();

							if(Seq_MetroTrack *t=new Seq_MetroTrack(this))
							{
								t->Load(file);

								if(t->ismetrotrack==true)
								{
									AddMetroTrack(t);
								}
								else
								{
									TRACE ("L ChildDepth %d PIndex %d\n",t->childdepth,t->parentindex);
									if(t->childdepth>0)
										t->parent=GetTrackIndex(t->parentindex);

									AddTrack(t);
									loadtrack=t;
								}
							}
							else
								file->JumpOverChunk();
						}
						break;

					case CHUNK_MIDIDEVICES:
						{
							TRACE ("Chunk CHUNK_MIDIDEVICES \n2");

							file->ChunkFound();
							mainMIDI->LoadDevices(file);
						}
						break;

					case CHUNK_MAINAUDIO:
						{
							TRACE ("Chunk CHUNK_MAINAUDIO \n2");

							file->ChunkFound();
							mainaudio->Load(file);
						}
						break;

					case CHUNK_AUDIOSYSTEM:
						{
							TRACE ("Chunk CHUNK_AUDIOSYSTEM \n2");

							file->ChunkFound();
							audiosystem.Load(file);
						}
						break;

					case CHUNK_DRUMMAPHEADER:
						{
							TRACE ("Chunk CHUNK_DRUMMAPHEADER \n2");

							file->ChunkFound();
							LoadDrumMaps(file);
						}
						break;

					case CHUNK_WAVEMAPHEADER:
						{
							TRACE ("Chunk CHUNK_WAVEMAPHEADER \n2");

							file->ChunkFound();
							mainwavemap->Load(file);
						}
						break;

					default: // unknown 
						file->JumpOverChunk();
						break;
				}
			}	
		}
		else
			file->errorflag|=CHUNK_ERROR;

		file->RenewPointer();
		RepairSongAfterLoading();

		Seq_Track *setfocustrack=GetTrackIndex(focustrackindex);

		if(!setfocustrack)
			setfocustrack=FirstTrack();

		SetFocusTrack(setfocustrack);

		//SetSongPosition(songposition,false,SETSONGPOSITION_FORCE);
		//PRepairPlayback(GetSongPosition(),MEDIATYPE_ALL);
		if(autoclose==true)
			file->Close(true);

		if(unabletoreadplugins.GetRoot())
		{
			char *h=mainvar->GenerateString(Cxs[CXS_UNABLETOLOADPLUGINS],":","\n");
			if(h)
			{
				c_Pluginnotfound *cnf=(c_Pluginnotfound *)unabletoreadplugins.GetRoot();

				while(cnf && h)
				{
					error++;
					if(cnf->plugin)
					{
						char *h2=mainvar->GenerateString(h,cnf->plugin,"\n");
						delete h;
						h=h2;
						delete cnf->plugin;
					}
					cnf=(c_Pluginnotfound *)unabletoreadplugins.RemoveO(cnf);
				}

				if(h)
				{
					maingui->MessageBoxError(0,h);
					delete h;
				}
			}
		}

		bool ok=true;

		if(error)
		{
			if(char *h=mainvar->GenerateString("Song:",GetName(),"\n",Cxs[CXS_ASKLOADERRORSONG]))
			{
				ok=maingui->MessageBoxYesNo(0,h);
				delete h;
			}
		}

		if(ok==true)
		{
			loaded=true;
		}
	}

	if(sn)delete sn;
}

void Seq_Song::LoadDrumMaps(camxFile *file)
{
	file->CloseReadChunk();
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_DRUMMAP)
	{
		file->ChunkFound();
		drummap.Load(file);
	}
}

void Seq_Song::SaveDrumMaps(camxFile *file)
{
	if(drummap.GetCountOfTracks())
	{
		file->OpenChunk(CHUNK_DRUMMAPHEADER);
		file->CloseChunk();

		drummap.Save(file);
	}
}

void Seq_Song::SaveArrangement(camxFile *file)
{
	saveonlyarrangement=true;
	Save(file);
	saveonlyarrangement=false;
}

void Seq_Song::Save(camxFile *file)
{
	loaded=true;

	if(loadwhenactivated==true)
	{
		TRACE ("Song Saving skipped\n");
		return;
	}

#ifdef DEMO
	MessageBox(NULL,"Demo Version saving maximum of 4 tracks...","Info",MB_OK);
#endif

	camxFile savefile;
	bool autoclose=false;
	char *sn=0;

	if((!file) && autoloadsong==true)
	{
		if(mainvar->exitthreads==true || maingui->MessageBoxYesNo(0,Cxs[CXS_OVERWRITEAUTOLOADQ])==true)
			maingui->SaveDefaultSong(this,autoloadMIDI);
	}
	else
		if((!file) && directoryname){

			if(sn=mainvar->GenerateString(directoryname,"\\cpsong.camx")){

				file=&savefile;

				if(file->OpenSave_CheckVersion(sn)==true)
				{
				}
				else
					file=0;

				autoclose=true;
			}
		}

		if(file){

			// Info Text
			{
				char cr[3];

				cr[0]=0x0D;
				cr[1]=0x0A;
				cr[2]=0;

				// Delete old song info
				{
					camxFile scan;
					scan.ListDirectoryContents1(directoryname,"*.*",".txt");
					camxScan *s=scan.FirstScan();
					while(s)
					{
						mainvar->DeleteAFile(s->name);
						s=s->NextScan();
					}
					scan.ClearScan();
				}

				camxFile songinfo;

				if(char *it=mainvar->GenerateString(directoryname,"\\song_",GetName(),".txt")){

					if(songinfo.OpenSave(it))
					{
						songinfo.Save("Song:");
						songinfo.Save(songname);
						songinfo.Save(cr);
						songinfo.Save("--- ---");
						songinfo.Save(cr);

						songinfo.Save("Tracks:");
						char h2[NUMBERSTRINGLEN];
						songinfo.Save(mainvar->ConvertIntToChar(GetCountOfTracks(),h2));
						songinfo.Save(cr);

					}
					songinfo.Close(true);

					delete it;
				}
			}

			file->SaveVersion();

			// Pointer
			file->OpenChunk(CHUNK_SONGPOINTER);

			// Song name
			file->Save_ChunkString(songname);

			// Save Flags
			int flags=file->helpflag;
			file->Save_Chunk(flags);

			int c=GetCountOfAudioPattern();
			file->Save_Chunk(c);

			file->CloseChunk();

			// Song Settings
			file->OpenChunk(CHUNK_SONGSETTINGS);

			file->Save_Chunk(notetype);
			file->Save_Chunk(generalMIDI);
			file->Save_Chunk(generalMIDI_drumchannel);
			file->Save_Chunk(songposition);
			file->Save_Chunk(songlength_measure);
			file->Save_Chunk(songlength_ticks);

			file->Save_Chunk(defaultmasterstart);
			file->Save_Chunk(defaultmasterend);
			file->Save_ChunkString(masterfile);
			file->Save_ChunkString(masterdirectoryselectedtracks);

			file->Save_Chunk(MIDIsync.sync);

			file->Save_Chunk(smpteoffset.h);
			file->Save_Chunk(smpteoffset.m);
			file->Save_Chunk(smpteoffset.sec);
			file->Save_Chunk(smpteoffset.frame);
			file->Save_Chunk(smpteoffset.minus);

			file->Save_Chunk(MIDIsync.sendmtc);
			file->Save_Chunk(MIDIsync.sendmc);

			file->Save_Chunk(default_masterformat);
			file->Save_Chunk(default_masternormalize);
			file->Save_Chunk(default_mastersavefirst);
			file->Save_Chunk(default_masterpausems);
			file->Save_Chunk(default_masterpausesamples);
			file->Save_Chunk(fxshowflag);
			file->Save_Chunk(freezeindexcounter);
			file->Save_Chunk(mastefilename_autoset);
			file->Save_Chunk(default_masterchannels);
			file->Save_Chunk(pref_tracktype);

			file->CloseChunk();

			inputrouting.Save(file);

			// Playback Settings
			file->OpenChunk(CHUNK_SONGPLAYBACK);
			playbacksettings.Save(file);
			file->CloseChunk();

			file->OpenChunk(CHUNK_SONG_METRO);
			metronome.Save(file);
			file->CloseChunk();

			// Save Group
			SaveGroups(file);
			SaveLinks(file);

			// Save Tracks
			file->OpenChunk(CHUNK_SONGTRACKS);

			// Active Track
			int focustrackindex=0;

			if(GetFocusTrack())
				focustrackindex=GetOfTrack(GetFocusTrack());

			file->Save_Chunk(focustrackindex);
			file->CloseChunk();

			// Now write Song Tracks
			if(Seq_Track *t=FirstTrack())
			{
				while(t){
					t->Save(file);
					t=t->NextTrack();
				}

				file->OpenChunk(CHUNK_SONGTRACKSEND);
				file->CloseChunk();
			}

			// Save Metro Tracks
			if(FirstMetroTrack())
			{
				file->OpenChunk(CHUNK_SONGMETROTRACKS);

				file->Save_Chunk(metrotracks.GetCount());

				file->CloseChunk();

				// Now write Song Tracks
				Seq_MetroTrack *t=FirstMetroTrack();

				while(t){
					t->Save(file);
					t=t->NextMetroTrack();
				}
			}

			mainaudio->Save(file); // Save AudioHD Files+Regions

			// 
			// TimeTrack
			timetrack.Save(file);
			textandmarker.Save(file);

			// Audio
			audiosystem.Save(file);

			// Grooves
			mainMIDI->SaveGrooves(file);

			// Drummaps
			SaveDrumMaps(file);
			mainwavemap->Save(file);

			// Hardware
			mainMIDI->SaveDevices(file);

			errorsongwriting=file->errorwriting;

			if(errorsongwriting==true)
				maingui->MessageBoxError(0,Cxs[ERROR_SONGSAVE]);

			if(autoclose==true)
				file->Close(true);
		}

		if(sn)delete sn;
}

void Seq_Song::InitSongPlayback(InitPlay *init)
{	
	if(/*onlycycle==false && */init && (init->mode&MEDIATYPE_MIDI)){

		//	skipplaybackevents.DeleteAllO();
		// Reset next MIDI Event
		nextMIDIPlaybackEvent[init->initindex]=0;
		nextMIDIchainlistevent[init->initindex]=0;
		nextMIDIplaybackpattern[init->initindex]=nextMIDIchainlistplaybackpattern[init->initindex]=0;
	}

	if(Seq_Track *t=FirstTrack()) // Empty Song ?
	{
		//int index=init?init->initindex:INITPLAY_MIDITRIGGER;

		/*
		InitPlay cycle(index);

		cycle.mode=init?init->mode:MEDIATYPE_ALL;
		cycle.position=playbacksettings.cyclestart;
		cycle.cycleplay=true;

		cycle.solotrack=init?init->solotrack:0;
		*/

		while(t){
			if(/*onlycycle==false && */init)t->InitPlayback(init);
			//		if((!init) || init->nocycle==false)t->InitPlayback(&cycle);
			t=t->NextTrack();
		}
	}
}

void Seq_Song::RefreshAudioBuffer()
{
	InitMetroTrack_Clicks();

	{
		Seq_MetroTrack *mt=FirstMetroTrack();

		while(mt)
		{
			mt->t_audiofx.CreateTrackMix(audiosystem.device);
			mt->io.RefreshEffects(audiosystem.device);

			mt=mt->NextMetroTrack();
		}
	}

	{
		Seq_Track *t=FirstTrack();

		while(t)
		{
			t->t_audiofx.CreateTrackMix(audiosystem.device);
			t->io.RefreshEffects(audiosystem.device);

			t=t->NextTrack();
		}
	}

	for(int i=0;i<LASTSYNTHCHANNEL;i++) // Channels/Bus/Instr
	{
		AudioChannel *c=audiosystem.FirstChannelType(i);

		while(c)
		{
			c->CreateChannelBuffers(audiosystem.device);
			c->io.RefreshEffects(audiosystem.device);
			c=c->NextChannel();
		}
	}

	audiosystem.masterchannel.io.RefreshEffects(audiosystem.device);
}

// Init Tracks -> Record Files
void Seq_Song::InitAudioRecording()
{
	// Reset Audio Record Pattern
	Seq_Track *t=FirstTrack();

	while(t)
	{
		// Reset Audio Track Punch Buffer
		for(int i=0;i<MAXRECPATTERNPERTRACK;i++)
			t->audiorecord_audiopattern[i]=0;

		t=t->NextTrack();
	}

	AudioInputNeeded();
}

void Seq_Song::GetNextMIDIPlaybackEvent(cMIDIPlaybackEvent *pe) 
{
	if(nextMIDIPlaybackEvent[pe->index] && nextMIDIchainlistevent[pe->index]) // nonNotes + Chain
	{
		if(nextMIDIplaybacksampleposition[pe->index]==nextMIDIchaineventsampleposition[pe->index]) // ==, use index
		{
			// Check Index
			if(nextMIDIPlaybackEvent[pe->index]->index<nextMIDIchainlistevent[pe->index]->index)
				goto events;

			goto chainnotes;
		}

		if(nextMIDIplaybacksampleposition[pe->index]<nextMIDIchaineventsampleposition[pe->index]) // Events < Notes
			goto events;

		goto chainnotes;
	}

	if(nextMIDIPlaybackEvent[pe->index]) // Events
	{
events:
		pe->ischainnote=false;
		pe->playbackevent=nextMIDIPlaybackEvent[pe->index];
		pe->playbackpattern=nextMIDIplaybackpattern[pe->index];
		pe->nextMIDIplaybacksampleposition=nextMIDIplaybacksampleposition[pe->index];

		return;
	}

	if(nextMIDIchainlistevent[pe->index]) // Note Chain  
	{
chainnotes:
		pe->ischainnote=true;
		pe->playbackevent=nextMIDIchainlistevent[pe->index];
		pe->playbackpattern=nextMIDIchainlistplaybackpattern[pe->index];
		pe->nextMIDIplaybacksampleposition=nextMIDIchaineventsampleposition[pe->index];

		return;
	}

	// no nonNote Events or ChainList Notes
	pe->playbackevent=0;
	pe->playbackpattern=0;
}

// Thread - song is locked
void Seq_Song::MixNextMIDIPlaybackEvent(cMIDIPlaybackEvent *pe) // Now 'mix' our Pattern/Events, called by MIDI Out Thread
{
	if(pe->playbackevent) // Played Event, set net Event...
	{
		if(pe->ischainnote==true) // Next Chain Note
		{
			Seq_MIDIChainEvent *n=(Seq_MIDIChainEvent *)pe->playbackevent;
			pe->playbackpattern->playback_nextchainevent[pe->index][0]=n->next_chainevent; // set next CL Note
		}
		else // Next Event, Skip all Notes ...
		{
			pe->playbackpattern->playback_MIDIevent[pe->index][0]=pe->playbackevent->NextEvent();
			pe->playbackpattern->SkipPlaybackMIDIEvents(pe->index,0);
		}
	}

	// Reset
	nextMIDIPlaybackEvent[pe->index]=0;
	nextMIDIchainlistevent[pe->index]=0;
	nextMIDIplaybackpattern[pe->index]=nextMIDIchainlistplaybackpattern[pe->index]=0;
	nextMIDIplaybacksampleposition[pe->index]=nextMIDIchaineventsampleposition[pe->index]=0;

	OSTART nextposition,nextchainposition;

	Seq_Track *t=FirstTrack();

	while(t){

		// 1. Check All nonNotes |- Songposition MIDI Pattern  -----
		{
			MIDIPattern *p=t->playback_MIDIPattern[pe->index][0];

			while(p) 
			{
				// 1. nonNotes
				if(Seq_Event *nonnote=p->playback_MIDIevent[pe->index][0]) // Or Empty Pattern
				{
					OSTART h=nonnote->GetPlaybackStart(p,t); //v

					if((!nextMIDIPlaybackEvent[pe->index]) || h<nextposition){

						nextposition=h;
						nextMIDIPlaybackEvent[pe->index]=nonnote;
						nextMIDIplaybackpattern[pe->index]=p;
					}
				}

				p=(MIDIPattern *)p->NextPattern(MEDIATYPE_MIDI); 
			}
		}

		// 2. Check All Chain Notes |- Songposition MIDI Pattern  -----
		{
			MIDIPattern *p=t->playback_chainnoteMIDIPattern[pe->index][0];

			while(p) 
			{
				// 2. Chain Notes
				if(Seq_MIDIChainEvent *chainnote=p->playback_nextchainevent[pe->index][0]) // Note Chain
				{
					OSTART h=chainnote->GetPlaybackStart(p,t); //v

					if((!nextMIDIchainlistevent[pe->index]) || h<nextchainposition){

						nextchainposition=h;
						nextMIDIchainlistevent[pe->index]=chainnote;
						nextMIDIchainlistplaybackpattern[pe->index]=p;
					}
				}

				p=(MIDIPattern *)p->NextPattern(MEDIATYPE_MIDI); 
			}
		}

		t=t->NextTrack_NoUILock();
	}

	if(nextMIDIPlaybackEvent[pe->index]) // MIDI next pattern ?
	{
#ifdef DEBUG
		if(nextposition<0)
			maingui->MessageBoxError(0,"nextMIDIPlaybackEvent nextposition<0 678");
#endif

		pe->nextMIDIplaybacksampleposition=nextMIDIplaybacksampleposition[pe->index]=timetrack.ConvertTicksToTempoSamples(nextposition);

		if(!(nextMIDIplaybackpattern[pe->index]->playback_MIDIevent[pe->index][0]=nextMIDIPlaybackEvent[pe->index]->NextEvent()))// All nonNotes of pattern mixed ? 
		{
			MIDIPattern *np=(MIDIPattern *)nextMIDIplaybackpattern[pe->index]->NextPattern(MEDIATYPE_MIDI);

			while(np && (!np->playback_MIDIevent[pe->index])) // filter empty pattern
				np=(MIDIPattern *)np->NextPattern(MEDIATYPE_MIDI);

			nextMIDIPlaybackEvent[pe->index]->pattern->GetTrack()->playback_MIDIPattern[pe->index][0]=np;
		}	
	}

	if(nextMIDIchainlistevent[pe->index]) // Chain Notes ----- 
	{
#ifdef DEBUG
		if(nextchainposition<0)
			maingui->MessageBoxError(0,"nextMIDIPlaybackEvent nextchainposition<0 679");
#endif

		pe->nextMIDIplaybacksampleposition=nextMIDIchaineventsampleposition[pe->index]=timetrack.ConvertTicksToTempoSamples(nextchainposition);

		if(nextMIDIchainlistevent[pe->index]==nextMIDIchainlistplaybackpattern[pe->index]->lastchainevent)// All Notes of pattern mixed ? 
		{
			MIDIPattern *np=(MIDIPattern *)nextMIDIchainlistplaybackpattern[pe->index]->NextPattern(MEDIATYPE_MIDI);

			while(np && (!np->playback_MIDIevent[pe->index])) // filter empty pattern
				np=(MIDIPattern *)np->NextPattern(MEDIATYPE_MIDI);

			nextMIDIchainlistplaybackpattern[pe->index]->GetTrack()->playback_chainnoteMIDIPattern[pe->index][0]=np;
		}	
	}
}

void Seq_Song::CopyInitPlay(int to,int from)
{
	if(to==from)return;

	Seq_Track *t=FirstTrack();

	while(t)
	{
		// Track Start Pattern
		for(int i=0;i<2;i++)
		{
			t->playback_MIDIPattern[to][i]=t->playback_MIDIPattern[from][i];
			t->playback_chainnoteMIDIPattern[to][i]=t->playback_chainnoteMIDIPattern[from][i];
		}

		// Copy MIDI Pattern CHECK->SONG
		MIDIPattern *copy=(MIDIPattern *)t->FirstPattern(MEDIATYPE_MIDI);

		while(copy)
		{
			for(int i=0;i<2;i++)
			{
				copy->playback_MIDIevent[to][i]=copy->playback_MIDIevent[from][i];
				copy->playback_nextchainevent[to][i]=copy->playback_nextchainevent[from][i];
			}

			copy=(MIDIPattern *)copy->NextPattern(MEDIATYPE_MIDI);
		}

		t=t->NextTrack();
	}			

	// Copy new Mix
	nextMIDIPlaybackEvent[to]=nextMIDIPlaybackEvent[from];
	nextMIDIchainlistevent[to]=nextMIDIchainlistevent[from];

	nextMIDIplaybackpattern[to]=nextMIDIplaybackpattern[from];
	nextMIDIchainlistplaybackpattern[to]=nextMIDIchainlistplaybackpattern[from];

	nextMIDIplaybacksampleposition[to]=nextMIDIplaybacksampleposition[from];
	nextMIDIchaineventsampleposition[to]=nextMIDIchaineventsampleposition[from];
}

bool Seq_Song::CheckPlaybackRefresh(int flag)
{
	bool refresh=false;

	RefreshLoops();

	if(status&(STATUS_PLAY|STATUS_RECORD))
	{
		InitPlay init(INITPLAY_MIDITRIGGER);
		init.position=GetSongPosition();
		init.mode=MEDIATYPE_MIDI|MEDIATYPE_AUDIO;

		InitSongPlayback(&init/*,false*/);

		cMIDIPlaybackEvent nmpe(INITPLAY_MIDITRIGGER);
		MixNextMIDIPlaybackEvent(&nmpe); // INITPLAY_MIDITRIGGER: Mix new Audio+MIDI

		// Copy Events Init
		for(int to=INITPLAY_MIDITRIGGER+1;to<INITPLAY_MAX;to++)
		{
			Seq_Track *t=FirstTrack();

			while(t)
			{
				// Track Start Pattern
				for(int i=0;i<2;i++)
				{
					t->playback_MIDIPattern[to][i]=t->playback_MIDIPattern[INITPLAY_MIDITRIGGER][i];
					t->playback_chainnoteMIDIPattern[to][i]=t->playback_chainnoteMIDIPattern[INITPLAY_MIDITRIGGER][i];
				}

				// Copy MIDI Pattern CHECK->SONG
				MIDIPattern *copy=(MIDIPattern *)t->FirstPattern(MEDIATYPE_MIDI);

				while(copy)
				{
					for(int i=0;i<2;i++)
					{
						copy->playback_MIDIevent[to][i]=copy->playback_MIDIevent[INITPLAY_MIDITRIGGER][i];
						copy->playback_nextchainevent[to][i]=copy->playback_nextchainevent[INITPLAY_MIDITRIGGER][i];
					}

					copy=(MIDIPattern *)copy->NextPattern(MEDIATYPE_MIDI);
				}

				t=t->NextTrack();
			}			

			// Copy new Mix
			nextMIDIPlaybackEvent[to]=nextMIDIPlaybackEvent[INITPLAY_MIDITRIGGER];
			nextMIDIchainlistevent[to]=nextMIDIchainlistevent[INITPLAY_MIDITRIGGER];

			nextMIDIplaybackpattern[to]=nextMIDIplaybackpattern[INITPLAY_MIDITRIGGER];
			nextMIDIchainlistplaybackpattern[to]=nextMIDIchainlistplaybackpattern[INITPLAY_MIDITRIGGER];

			nextMIDIplaybacksampleposition[to]=nextMIDIplaybacksampleposition[INITPLAY_MIDITRIGGER];
			nextMIDIchaineventsampleposition[to]=nextMIDIchaineventsampleposition[INITPLAY_MIDITRIGGER];
		}

		// Init MIDI Pre-Events+Send
		PRepairPreStartAndCycleEvents(init.position);

		//if(!(flag&PLAYBACKREFRESH_NOMIDISEND))
		SendPRepairControl(false,SENDPRE_AUDIO|SENDPRE_MIDI);

		//if(oldalarm!=nextMIDIplaybacksampleposition[INITPLAY_MIDITRIGGER])
		mainMIDIalarmthread->SetSignal(); // Send Refresh Alarm
	}
	else
		PRepairPlayback(GetSongPosition(),MEDIATYPE_ALL);

	return refresh;
}

int Seq_Song::GetSubTrackPosition(AutomationTrack *st)
{
	int c=0;

	Seq_Track *t=FirstTrack();
	while(t)
	{
		AutomationTrack *s=t->FirstAutomationTrack();
		while(s)
		{
			if(s==st)return c;
			c++;

			s=s->NextAutomationTrack();
		}

		t=t->NextTrack();
	}

	return -1;
}

void Seq_Song::SetSongName(char *nname,bool refreshgui)
{
	if(!nname)return;

	char *cs=mainvar->CreateSimpleASCIIString(nname);

	if(!cs)return;

	if((!songname) || strcmp(cs,songname)!=0)
	{
		if(songname){
			delete songname;
			songname=0;
		}

		if(songname=mainvar->GenerateString(cs))
		{
			if(refreshgui==true)
			{
				// Refresh GUI
				guiWindow *w=maingui->FirstWindow();
				while(w)
				{
					if(w->WindowSong()==this)
						w->SongNameRefresh();
					else 
						switch(w->GetEditorID())
					{
						case EDITORTYPE_PLAYER:
							{
								Edit_Player *edp=(Edit_Player *)w;

								if(edp->activeproject==project)
									edp->ShowSongs();
							}
							break;
					}
					w=w->NextWindow();
				}
			}

			maingui->RefreshProjectGUI();
		}
	}

	delete cs;
}

void Seq_Song::SetCycleIndex(int index)
{
	if((status&STATUS_RECORD) && playbacksettings.cycleplayback==true)
		return;

	if(index!=playbacksettings.activecycle){
		playbacksettings.activecycle=index;
		SetCycle(playbacksettings.cyclestart_buffer[playbacksettings.activecycle],playbacksettings.cycleend_buffer[playbacksettings.activecycle]);
	}
}

bool Seq_Song::CheckCyclePositions(OSTART start,OSTART end)
{
	if(start>=0 && end>=0)
	{
		if(start>end) //<->
		{
			OSTART h=start;
			start=end;
			end=h;
		}

		start=timetrack.ConvertTicksToMeasureTicks(start,false);

		if(timetrack.ConvertTicksToMeasureTicks(end,false)!=end)
			end=timetrack.ConvertTicksToMeasureTicks(end,true); // Up

		if(end>start && start==playbacksettings.cyclestart && end==playbacksettings.cycleend)
			return true;
	}

	return false;
}

void Seq_Song::SetCycle(OSTART start,OSTART end)
{
	if((status&STATUS_RECORD) && playbacksettings.cycleplayback==true)
		return;

	if(start>=0 && end>=0)
	{
		if(start>end) //<->
		{
			OSTART h=start;
			start=end;
			end=h;
		}

		start=timetrack.ConvertTicksToMeasureTicks(start,false);

		if(timetrack.ConvertTicksToMeasureTicks(end,false)!=end)
			end=timetrack.ConvertTicksToMeasureTicks(end,true); // Up

		if(end>start && (start!=playbacksettings.cyclestart || end!=playbacksettings.cycleend))
		{
			if(this==mainvar->GetActiveSong())
				mainthreadcontrol->LockActiveSong();

			bool checkstart=start!=playbacksettings.cyclestart?true:false;

			playbacksettings.Lock(); // VST etc..

			playbacksettings.cyclestart=start;
			playbacksettings.cycleend=end;

			playbacksettings.ConvertTicksToTempoSampleRate();
			playbacksettings.Unlock();

			// Index
			playbacksettings.cyclestart_buffer[playbacksettings.activecycle]=start;
			playbacksettings.cycleend_buffer[playbacksettings.activecycle]=end;
			playbacksettings.SetCycleMeasure();

			CheckForCycleReset();

			/*
			if(this==mainvar->GetActiveSong())
			{	
			if((status&STATUS_SONGPLAYBACK_MIDI) && playbacksettings.cycleplayback==true)
			{
			RestartSong(start,0,0);
			}
			else
			{
			//InitSongPlayback(0,true); // init only cycle pattern
			//DeleteAllPreEvents(DELETE_PRepair_NOTEOFFS|DELETE_PRepair_CONTROL|DELETE_PRepair_CYCLE); // Delete All Cycle Events
			//PRepairPreStartCycleEvents();
			}

			if(lock==true)

			}
			*/

			if(this==mainvar->GetActiveSong())
			{
				if(checkstart==true && (status&STATUS_SONGPLAYBACK_MIDI))
					InitSongPlaybackPatternNewCycleStart(); // Refresh MIDI Pattern und Pre Events

				mainthreadcontrol->UnlockActiveSong();
			}

			maingui->SongCycleChanged(this);
		}
	}
}

void Seq_Song::SetCycleStart(OSTART position,bool movecycleend)
{
	if((status&STATUS_RECORD) && playbacksettings.cycleplayback==true)
		return;

	if(position>=0)
	{
		OSTART range_measure=playbacksettings.cycleend_measure-playbacksettings.cyclestart_measure;

		if(range_measure<1)
			range_measure=1;

		position=timetrack.ConvertTicksToMeasureTicks(position,false);

		if(position!=playbacksettings.cyclestart && (position<playbacksettings.cycleend || movecycleend==true))
		{
			if(movecycleend==true)
			{
				OSTART mstart=timetrack.ConvertTicksToMeasure(position),mend=timetrack.ConvertMeasureToTicks(mstart+range_measure);
				SetCycle(position,mend);
			}
			else
				SetCycle(position,playbacksettings.cycleend);
		}
	}
}

void Seq_Song::SetCycleEnd(OSTART position)
{
	if((status&STATUS_RECORD) && playbacksettings.cycleplayback==true)
		return;

	if(position>=0)
	{
		position=timetrack.ConvertTicksToMeasureTicks(position,true);

		if(position!=playbacksettings.cycleend && position>playbacksettings.cyclestart)
		{
			SetCycle(playbacksettings.cyclestart,position);
		}
	}
}

void Seq_Song::RestartSong(OSTART pos,int playsongflag,int flag)
{
	int oldstatus=status;

	if(status&STATUS_PLAY)
	{
		songposition=StopSong(flag,pos,false); // false = no set songposition 

		SetAutomationTracksPosition(pos);
		PlaySong(playsongflag);
	}
	else
	{
		songposition=pos;

		InitNewSongPositionToStreams();
		SetAutomationTracksPosition(pos);

		if(status!=STATUS_STEPRECORD){

			PRepairPlayback(pos,MEDIATYPE_ALL);

			if(!(flag&SETSONGPOSITION_NOMIDIOUTPUT))
				mainMIDI->SendSongPosition(this,pos,false);
		}
	}
}

void Seq_Song::SetSongPositionWithMarker(OSTART pos,bool nextmarker)
{
	if(nextmarker==true)
	{
		Seq_Marker *mk=textandmarker.FindMarker(pos+1);

		if(mk)
		{
			mk=mk->NextMarker();

			if(mk)
				SetSongPosition(mk->GetMarkerStart(),true);
		}
		else
		{
			mk=textandmarker.FirstMarker();

			if(mk)
				SetSongPosition(mk->GetMarkerStart(),true);
		}
	}
	else
	{
		if(pos<=0)
			return;

		Seq_Marker *mk=textandmarker.FindMarker(pos-1);

		if(mk)
		{
			SetSongPosition(mk->GetMarkerStart(),true);
		}
		else
		{
			SetSongPosition(0,true);
		}
	}

}

void Seq_Song::REW()
{
	OSTART sp=GetSongPosition();

	sp-=timetrack.zoomticks;

	if(sp<0)sp=0;

	SetSongPosition(sp,true);
}

void Seq_Song::FF()
{
	OSTART sp=GetSongPosition();
	sp+=timetrack.zoomticks;
	SetSongPosition(sp,true);
}

void Seq_Song::SetSongPositionPrevOrNextMeasure(bool prev)
{
	Seq_Pos pos(Seq_Pos::POSMODE_NORMAL);

	timetrack.ConvertTicksToPos(GetSongPosition(),&pos);

	pos.pos[1]=pos.pos[2]=pos.pos[3]=pos.pos[4]=1;

	if(prev==true)
	{
		if(pos.pos[0]>1)
			pos.pos[0]--;
		else
			return;
	}
	else
	{
		pos.pos[0]++;
	}

	SetSongPosition(timetrack.ConvertPosToTicks(&pos),true);
}

void Seq_Song::SetSongPosition(OSTART pos,bool restart,int flag)
{
	if(pos==songposition)return;

	int playsongflag=flag;

	TRACE ("Set SPP %d\n",pos);

	/*
	// Sync CamX Song Position<>MIDI Song Position 
	if(mainMIDI->quantizesongpositiontoMIDIclock==true && mainMIDI->sendMIDIcontrol==true && (!(flag&SETSONGPOSITION_NOQUANTIZE)))
	{
	pos=mainvar->SimpleQuantizeLeft(pos,PPQRATE/4);
	}
	*/

	/*
	if(!(flag&SETSONGPOSITION_NOLOCK))
	{
	// LockSync();
	playsongflag|=SETSONGPOSITION_NOLOCK;
	}
	*/

#ifdef _DEBUG
	if(pos<0)
		MessageBox(NULL,"Illegal Set Song Position","Error",MB_OK);
#endif

	if((flag&SETSONGPOSITION_FORCE) ||
		(
		CanStatusBeChanged()==true && 
		pos>=0 && 
		(!(status&STATUS_WAITPREMETRO)) && 
		(!(status&STATUS_RECORD))
		)
		)
	{
		if(this==mainvar->GetActiveSong())
		{
			RestartSong(pos,playsongflag,flag);
		}
		else // Not the active song
		{
			songposition=pos;
			InitNewSongPositionToStreams();
			SetAutomationTracksPosition(pos); // 0= all tracks
		}
	}

	//if(!(flag&SETSONGPOSITION_NOLOCK))
	//	UnlockSync();

	//if(refreshgui==true && (!(flag&SETSONGPOSITION_NOGUI)) )
	//	maingui->SongPositionChanged(this,pos);

	TRACE ("Set SPP DONE %d\n",pos);
}

AudioHardwareChannel *Seq_Song::GetUsedAudioDeviceChannel()
{
	LockCore();
	AudioHardwareChannel *uac=firstfreecoredevicechannel;
	firstfreecoredevicechannel=uac?uac->nextcorechannel:0;
	UnlockCore();
	return uac;
}

AudioChannel *Seq_Song::GetUsedAudioChannel()
{
	LockCore();
	AudioChannel *uac=firstfreecorechannel;
	firstfreecorechannel=uac?uac->nextcorechannel:0;
	UnlockCore();
	return uac;
}

AudioChannel *Seq_Song::GetUsedAudioBus()
{
	LockCore();
	AudioChannel *uac=firstfreecorebus;
	firstfreecorebus=uac?uac->nextcorebus:0;
	UnlockCore();
	return uac;
}

Seq_Track *Seq_Song::GetUsedAudioTrack()
{
	LockCore();
	Seq_Track *uat=firstfreecoretrack;
	firstfreecoretrack=uat?uat->nextcoretrack:0;
	UnlockCore();
	return uat;
}

Seq_Track *Seq_Song::GetUsedAudioThruTrack()
{
	LockCore();
	Seq_Track *uat=firstfreethrutrack;
	firstfreethrutrack=uat?uat->nextcorethrutrack:0;
	UnlockCore();
	return uat;
}

Seq_Track *Seq_Song::GetUsedAudioInputTrack()
{
	LockCore();
	Seq_Track *uat=firstfreeaudioinputtrack;
	firstfreeaudioinputtrack=uat?uat->nextcoreaudioinputtrack:0;
	UnlockCore();
	return uat;
}

Seq_Track *Seq_Song::GetUsedAudioTrackTrackIO()
{
	LockCore();
	Seq_Track *uat=firstfreecoretrack;
	firstfreecoretrack=uat?uat->nextcoretrack:0;
	UnlockCore();
	return uat;
}

int Seq_Song::GetCountOfTrackswithMIDIData()
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t){
		if(t->FirstPattern(MEDIATYPE_MIDI))c++;
		t=t->NextTrack();
	}

	return c;
}

Seq_Track *Seq_Song::GetMuteTrack(Seq_Track *st)
{
	Seq_Track *t=st==0?FirstTrack():st->NextTrack();

	while(t){
		if(t->GetMute()==true)
			return t;

		t=t->NextTrack();
	}

	if((!t) && st)
	{
		Seq_Track *t=st->PrevTrack();

		while(t){
			if(t->GetMute()==true)
				return t;

			t=t->PrevTrack();
		}
	}

	return 0;
}

Seq_Track *Seq_Song::GetSoloTrack(Seq_Track *st)
{
	Seq_Track *t=st==0?FirstTrack():st->NextTrack();

	while(t){
		if(t->GetSolo()==true)
			return t;

		t=t->NextTrack();
	}

	if((!t) && st)
	{
		Seq_Track *t=st->PrevTrack();

		while(t){
			if(t->GetSolo()==true)
				return t;

			t=t->PrevTrack();
		}
	}

	return 0;
}

int Seq_Song::GetCountOfTracksWithOutChilds()
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t){
		if(!t->parent)c++;
		t=t->NextTrack();
	}

	return c;
}

int Seq_Song::GetCountOfPattern(OSTART position,Seq_SelectionList *sellist,bool onlyselected,bool onlyselectedtracks)
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t){

		if(onlyselectedtracks==false || (t->flag&OFLAG_SELECTED))
		{
			Seq_Pattern *p=t->FirstPattern();

			while(p){

				if(
					(onlyselected==false || sellist->FindSelectedPattern(p)) &&
					p->itsaloop==false && 
					p->GetPatternStart()>=position
					)
					c++;

				p=p->NextPattern();
			}
		}

		t=t->NextTrack();
	}

	return c;
}

int Seq_Song::GetCountOfPattern(OSTART position,bool onlyselected,bool onlyselectedtracks)
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t){

		if(onlyselectedtracks==false || (t->flag&OFLAG_SELECTED))
		{
			Seq_Pattern *p=t->FirstPattern();

			while(p){

				if(
					(onlyselected==false || (p->flag&OFLAG_SELECTED)) &&
					p->itsaloop==false && 
					p->GetPatternStart()>=position
					)
					c++;

				p=p->NextPattern();
			}
		}

		t=t->NextTrack();
	}

	return c;
}

void Seq_Song::FillAutomationListUsing(AutomationTrack **list,int c,AutomationObject *ao)
{
	Seq_Track *t=FirstTrack();

	while(t){
		list=t->FillAutomationListUsing(list,ao);
		t=t->NextTrack();
	}

	list=audiosystem.masterchannel.FillAutomationListUsing(list,ao);
	AudioChannel *ac=audiosystem.FirstBusChannel();
	while(ac)
	{
		list=ac->FillAutomationListUsing(list,ao);
		ac=ac->NextChannel();
	}
}

int Seq_Song::GetCountOfAutomationTracksUsing(AutomationObject *ao)
{
	int c=0;

	Seq_Track *t=FirstTrack();

	while(t){
		c+=t->GetCountOfAutomationTracksUsing(ao);
		t=t->NextTrack();
	}

	c+=audiosystem.masterchannel.GetCountOfAutomationTracksUsing(ao);

	AudioChannel *ac=audiosystem.FirstBusChannel();
	while(ac)
	{
		c+=ac->GetCountOfAutomationTracksUsing(ao);
		ac=ac->NextChannel();
	}

	return c;
}

void Seq_Song::UserEditAutomation(AutomationObject *ao,int parindex)
{
	Seq_Track *t=FirstTrack();

	while(t){
		t->UserEditAutomation(ao,parindex);
		t=t->NextTrack();
	}

	audiosystem.masterchannel.UserEditAutomation(ao,parindex);

	AudioChannel *ac=audiosystem.FirstBusChannel();
	while(ac)
	{
		ac->UserEditAutomation(ao,parindex);
		ac=ac->NextChannel();
	}
}

int Seq_Song::GetCountOfPattern()
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t){
		c+=t->GetCountOfPattern(MEDIATYPE_ALL);
		t=t->NextTrack();
	}

	return c;
}

int Seq_Song::GetCountSelectedPattern()
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t)
	{
		c+=t->GetCountSelectedPattern();

		t=t->NextTrack();
	}

	return c;
}

int Seq_Song::GetCountOfAudioPattern()
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->FirstPattern(MEDIATYPE_AUDIO))
			c++;

		t=t->NextTrack();
	}

	return c;
}

void Seq_Song::SetFocusMetroTrack(Seq_Track *t)
{
	SetFocusType(FOCUSTYPE_METRO);
	RefreshAudioFocusWindows(t,0,0);
}

bool Seq_Song::SetFocusTrack(Seq_Track *t)
{
	if(t)
	{
		if(t->ismetrotrack==true)
		{
			SetFocusMetroTrack(t);
			return true;
		}

		if(maingui->GetShiftKey()==true)
			SelectTracksFromTo(GetFocusTrack(),t,true,false);
		else
			if(maingui->GetCtrlKey()==false)
				SelectSingleTrack(t);
			else
				t->Select();

		SetFocusType(FOCUSTYPE_TRACK);
		//	RefreshAudioFocusWindows(t,0,0);
		//}
		//else

		SetFocusTrackLock(t,false,0);

		return true;	
	}

	return false;
}

void Seq_Song::ResetTrackZoom()
{
	int c=0;
	Seq_Track *t=FirstTrack();

	while(t)
	{
		if(t->sizefactor!=1)
		{
			t->sizefactor=1;
			c++;
		}

		t=t->NextTrack();
	}

	if(c)
	{
		maingui->RefreshAllEditors(this,EDITORTYPE_ARRANGE,0);
	}
}

void Seq_Song::SetFocusEvent(Seq_Event *e)
{
	activeevent=e;

	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(this==w->WindowSong())
			switch(w->GetEditorID())	
		{
			case EDITORTYPE_PIANO:
				{
					Edit_Piano *ar=(Edit_Piano *)w;

					ar->ShowFocusEvent();

				}
				break;
		}

		w=w->NextWindow();
	}
}

void Seq_Song::SetFocusPattern(Seq_Pattern *p)
{
	activepattern=p;

	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(this==w->WindowSong())
			switch(w->GetEditorID())	
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)w;
					ar->RefreshObjects(0,false);
					ar->ShowFocusPattern();
				}
				break;

			case EDITORTYPE_ARRANGEFXPATTERN:
				{
					Edit_ArrangeFX_Pattern *p=(Edit_ArrangeFX_Pattern *)w;
					p->RefreshAll();
				}
				break;
		}

		w=w->NextWindow();
	}
}

void Seq_Song::SetRecordTrackTypes(Seq_Track *track,int type)
{
	Seq_Track *t=FirstTrack();

	while(t){

		if(t->IsPartOfEditing(track,true)==true)
			t->SetRecordTrackType(type);

		t=t->NextTrack();
	}
}

void Seq_Song::SetFocusTrackNoGUIRefresh(Seq_Track *t)
{
	if(t)
		t->Select();

	if(GetFocusTrack())
	{
		GetFocusTrack()->UnSelect();
	}

	tracks.activeobject=t;
}

void Seq_Song::SetFocusTrackLock(Seq_Track *t,bool unselectoldfocus,guiWindow *dontrefresh,bool refreshaudiomixgui)
{
	if(t!=GetFocusTrack())
	{
		if(unselectoldfocus==true)
		{
			if(GetFocusTrack())
				GetFocusTrack()->UnSelect();
		}

		mainthreadcontrol->LockActiveSong();
		tracks.activeobject=t;
		mainthreadcontrol->UnlockActiveSong();

		if(t)
			t->Select();
	}

	RefreshAudioFocusWindows(t,0,dontrefresh);
}

void Seq_Song::SetMasterFileName(char *mastername)
{
	if(!mastername)
		return;

	size_t i=strlen(mastername);
	bool foundpoint=false;

	if(i)
	{
		// Find .
		while(i--)
			if(mastername[i]=='.')
			{
				foundpoint=true;
				break;
			}
	}

	if(masterfile)
		delete masterfile;

	if(foundpoint==true)
		masterfile=mainvar->GenerateString(mastername);
	else
		masterfile=mainvar->GenerateString(mastername,".wav");
}

void Seq_Song::InitDefaultMasterName()
{
	char nr1[NUMBERSTRINGLEN],nr2[NUMBERSTRINGLEN];

	char *t1=mainvar->ConvertIntToChar(GetIndex()+1,nr1);
	char *t2=mainvar->ConvertIntToChar(project->GetCountOfSongs(),nr2);

	char *tt=mainvar->GenerateString("_",t1,"_",t2);

	char *mp=mainvar->GenerateString(project->projectdirectory,"\\","Master Audio","\\",GetName(),tt);

	if(mp)
	{
		SetMasterFileName(mp);
		delete mp;

		mastefilename_autoset=true;
	}

	if(tt)
		delete tt;
}

void Seq_Song::RefreshAudioFocusWindows(Seq_Track *t,AudioChannel *channel,guiWindow *dontrefresh)
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(this==w->WindowSong() && w!=dontrefresh)
			switch(w->GetEditorID())	
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)w;
					ar->RefreshObjects(0,false);
				}
				break;

			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *mix=(Edit_AudioMix *)w;

					if(mix->solotrack==true)
					{	
						bool refresh=false;

						if(t)
						{
							if(t->ismetrotrack==false && GetFocusType()==FOCUSTYPE_TRACK)
								refresh=true;

							if(t->ismetrotrack==true && GetFocusType()==FOCUSTYPE_METRO)
								refresh=true;
						}
						else
							if(channel)
							{
								if(channel->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL && GetFocusType()==FOCUSTYPE_BUS)
									refresh=true;

								if(channel->audiochannelsystemtype==CHANNELTYPE_MASTER && GetFocusType()==FOCUSTYPE_MASTER)
									refresh=true;
							}

							if(refresh==true)
								mix->RefreshSoloTrack();
					}
					else
						mix->ShowAll();
				}
				break;
		}

		w=w->NextWindow();
	}
}
