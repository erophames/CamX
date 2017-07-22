#include "defines.h"
#include "settings.h"
#include "audiodevice.h"
#include "MIDIhardware.h"
#include "vstplugins.h"
#include "audiohardware.h"
#include "transporteditor.h"
#include "gui.h"
#include "object_project.h"
#include "object_song.h"
#include "songmain.h"
#include "stepeditor.h"
#include "audiomaster.h"
#include "chunks.h"
#include "camxfile.h"

void Settings::InitPrevProjects()
{
	int i=5;
	Seq_Project *p=mainvar->LastProject();

	while(i && p)
	{
		if(p!=mainvar->GetActiveProject())
		{
			SetPrevProject(p);
			i--;
		}

		p=p->PrevProject();
	}

	if(mainvar->GetActiveProject())
		SetPrevProject(mainvar->GetActiveProject());
}

void Settings::SetPrevProject(Seq_Project *pro)
{
	if(pro)
	{
		for(int i=0;i<6;i++)
		{
			if(!prevprojects_dirname[i])break;
			if(prevprojects_dirname[i] && strcmp(prevprojects_dirname[i],pro->projectdirectory)==0)
				return; // in list
		}

		if(prevprojects_dirname[5])delete prevprojects_dirname[5];
		if(prevprojects_proname[5])delete prevprojects_proname[5];

		for(int i=5;i>0;i--)
		{
			prevprojects_dirname[i]=prevprojects_dirname[i-1];
			prevprojects_proname[i]=prevprojects_proname[i-1];
		}

		prevprojects_dirname[0]=mainvar->GenerateString(pro->projectdirectory);
		prevprojects_proname[0]=mainvar->GenerateString(pro->name);
	}
}

Directory *Settings::DeleteVSTDirectory(int ix,Directory *del)
{
	if(del)
	{
		Directory *r=(Directory *)del->NextOrPrev();
		if(del->dir)delete del->dir;
		vstdirectories[ix].RemoveO(del);
		return r;
	}

	return 0;
}

void Settings::SetPlaybackTrigger(bool on)
{
	waitforMIDIplayback=on;
	Save(0);
}

void Settings::InitDefaultWindowPositions()
{
	windowpositions[EDITORTYPE_UPDATE].x=0;
	windowpositions[EDITORTYPE_UPDATE].y=0;
	windowpositions[EDITORTYPE_UPDATE].width=300;
	windowpositions[EDITORTYPE_UPDATE].height=150;
	windowpositions[EDITORTYPE_UPDATE].set=true;

	// Sample
	windowpositions[EDITORTYPE_SAMPLE].x=80;
	windowpositions[EDITORTYPE_SAMPLE].y=10;
	windowpositions[EDITORTYPE_SAMPLE].width=500;
	windowpositions[EDITORTYPE_SAMPLE].height=200;
	windowpositions[EDITORTYPE_SAMPLE].set=true;


	// Groove
	windowpositions[EDITORTYPE_GROOVE].x=80;
	windowpositions[EDITORTYPE_GROOVE].y=10;
	windowpositions[EDITORTYPE_GROOVE].width=400;
	windowpositions[EDITORTYPE_GROOVE].height=300;
	windowpositions[EDITORTYPE_GROOVE].set=true;

	// Group
	windowpositions[EDITORTYPE_GROUP].x=80;
	windowpositions[EDITORTYPE_GROUP].y=10;
	windowpositions[EDITORTYPE_GROUP].width=400;
	windowpositions[EDITORTYPE_GROUP].height=300;
	windowpositions[EDITORTYPE_GROUP].set=true;

	/*
	// MIDICHANNEL
	windowpositions[EDITORTYPE_MIDICHANNEL].startposition_x=80;
	windowpositions[EDITOR_MIDICHANNEL_ID].startposition_y=10;
	windowpositions[EDITOR_MIDICHANNEL_ID].startwidth=200;
	windowpositions[EDITOR_MIDICHANNEL_ID].startheight=200;
	*/

	// Arrange
	windowpositions[EDITORTYPE_ARRANGE].x=0;
	windowpositions[EDITORTYPE_ARRANGE].y=0;
	windowpositions[EDITORTYPE_ARRANGE].width=500;
	windowpositions[EDITORTYPE_ARRANGE].height=390;
	windowpositions[EDITORTYPE_ARRANGE].set=true;

	//Transport
	windowpositions[EDITORTYPE_TRANSPORT].x=130;
	windowpositions[EDITORTYPE_TRANSPORT].y=440;
	windowpositions[EDITORTYPE_TRANSPORT].width=350;
	windowpositions[EDITORTYPE_TRANSPORT].height=70;
	windowpositions[EDITORTYPE_TRANSPORT].set=true;

	//Event
	windowpositions[EDITORTYPE_EVENT].x=10;
	windowpositions[EDITORTYPE_EVENT].y=10;
	windowpositions[EDITORTYPE_EVENT].width=600;
	windowpositions[EDITORTYPE_EVENT].height=400;
	windowpositions[EDITORTYPE_EVENT].set=true;

	//Piano
	windowpositions[EDITORTYPE_PIANO].x=10;
	windowpositions[EDITORTYPE_PIANO].y=10;
	windowpositions[EDITORTYPE_PIANO].width=300;
	windowpositions[EDITORTYPE_PIANO].height=400;
	windowpositions[EDITORTYPE_PIANO].set=true;


	// Wave
	windowpositions[EDITORTYPE_WAVE].x=10;
	windowpositions[EDITORTYPE_WAVE].y=10;
	windowpositions[EDITORTYPE_WAVE].width=300;
	windowpositions[EDITORTYPE_WAVE].height=400;
	windowpositions[EDITORTYPE_WAVE].set=true;

	//maingui->MessageBoxOk(0,"Blab3");
	//maindrummap->InitDefaultDrumMap();

	// Drum
	windowpositions[EDITORTYPE_DRUM].x=10;
	windowpositions[EDITORTYPE_DRUM].y=10;
	windowpositions[EDITORTYPE_DRUM].width=300;
	windowpositions[EDITORTYPE_DRUM].height=300;
	windowpositions[EDITORTYPE_DRUM].set=true;

	windowpositions[EDITORTYPE_CREATEAUTOMATIONTRACKS].x=10;
	windowpositions[EDITORTYPE_CREATEAUTOMATIONTRACKS].y=10;
	windowpositions[EDITORTYPE_CREATEAUTOMATIONTRACKS].width=22*maingui->GetButtonSizeY();;
	windowpositions[EDITORTYPE_CREATEAUTOMATIONTRACKS].height=30*maingui->GetButtonSizeY();;
	windowpositions[EDITORTYPE_CREATEAUTOMATIONTRACKS].set=true;

	// Score
	windowpositions[EDITORTYPE_SCORE].x=10;
	windowpositions[EDITORTYPE_SCORE].x=10;
	windowpositions[EDITORTYPE_SCORE].width=300;
	windowpositions[EDITORTYPE_SCORE].height=300;
	windowpositions[EDITORTYPE_SCORE].set=true;

	/*
	//MIDIfx
	windowpositions[EDITORTYPE_MIDIEFFECT].startposition_x=800;
	windowpositions[EDITOR_MIDIEFFECT_ID].startposition_y=10;
	windowpositions[EDITOR_MIDIEFFECT_ID].startwidth=200;
	windowpositions[EDITOR_MIDIEFFECT_ID].startheight=380;
	*/

	//settings
	windowpositions[EDITORTYPE_SETTINGS].x=10;
	windowpositions[EDITORTYPE_SETTINGS].y=10;
	windowpositions[EDITORTYPE_SETTINGS].width=600;
	windowpositions[EDITORTYPE_SETTINGS].height=360;
	windowpositions[EDITORTYPE_SETTINGS].set=true;

	//audiomixer
	windowpositions[EDITORTYPE_AUDIOMIXER].x=10;
	windowpositions[EDITORTYPE_AUDIOMIXER].y=10;
	windowpositions[EDITORTYPE_AUDIOMIXER].width=400;
	windowpositions[EDITORTYPE_AUDIOMIXER].height=600;
	windowpositions[EDITORTYPE_AUDIOMIXER].set=true;

	/*
	//trackaudiofx
	windowpositions[EDITORTYPE_TRACKMIXER].startposition_x=10;
	windowpositions[EDITOR_TRACKMIX_ID].startposition_y=10;
	windowpositions[EDITOR_TRACKMIX_ID].startwidth=100;
	windowpositions[EDITOR_TRACKMIX_ID].startheight=500;
	*/

	// Monitor
	windowpositions[EDITORTYPE_MONITOR].x=0;
	windowpositions[EDITORTYPE_MONITOR].y=0;
	windowpositions[EDITORTYPE_MONITOR].width=30*maingui->GetButtonSizeY();;
	windowpositions[EDITORTYPE_MONITOR].height=40*maingui->GetButtonSizeY();;
	windowpositions[EDITORTYPE_MONITOR].set=true;

	/*
	// Create Tracks
	windowpositions[EDITORTYPE_CREATETRACKS].x=10;
	windowpositions[EDITORTYPE_CREATETRACKS].y=10;
	windowpositions[EDITORTYPE_CREATETRACKS].width=35*maingui->GetFontSizeY();
	windowpositions[EDITORTYPE_CREATETRACKS].height=5*maingui->GetFontSizeY();
	windowpositions[EDITORTYPE_CREATETRACKS].set=true;
	*/

	//audiomanager
	windowpositions[EDITORTYPE_AUDIOMANAGER].x=0;
	windowpositions[EDITORTYPE_AUDIOMANAGER].y=0;
	windowpositions[EDITORTYPE_AUDIOMANAGER].width=45*maingui->GetButtonSizeY();;
	windowpositions[EDITORTYPE_AUDIOMANAGER].height=30*maingui->GetButtonSizeY();;
	windowpositions[EDITORTYPE_AUDIOMANAGER].set=true;

	//Edit Data
	windowpositions[EDITORTYPE_EDITDATA].x=0;
	windowpositions[EDITORTYPE_EDITDATA].y=0;
	windowpositions[EDITORTYPE_EDITDATA].width=200;
	windowpositions[EDITORTYPE_EDITDATA].height=40;
	windowpositions[EDITORTYPE_EDITDATA].set=true;

	// Quantize Editor
	windowpositions[EDITORTYPE_QUANTIZEEDITOR].x=0;
	windowpositions[EDITORTYPE_QUANTIZEEDITOR].y=0;
	windowpositions[EDITORTYPE_QUANTIZEEDITOR].width=30*maingui->GetButtonSizeY();
	windowpositions[EDITORTYPE_QUANTIZEEDITOR].height=40*maingui->GetButtonSizeY();
	windowpositions[EDITORTYPE_QUANTIZEEDITOR].set=true;

	// Audio Master
	windowpositions[EDITORTYPE_AUDIOMASTER].x=0;
	windowpositions[EDITORTYPE_AUDIOMASTER].y=0;
	windowpositions[EDITORTYPE_AUDIOMASTER].width=30*maingui->GetButtonSizeY();
	windowpositions[EDITORTYPE_AUDIOMASTER].height=20*maingui->GetFontSizeY();
	windowpositions[EDITORTYPE_AUDIOMASTER].set=true;

	// Audio Freeze
	windowpositions[EDITORTYPE_AUDIOFREEZE].x=0;
	windowpositions[EDITORTYPE_AUDIOFREEZE].y=0;
	windowpositions[EDITORTYPE_AUDIOFREEZE].width=30*maingui->GetButtonSizeY();
	windowpositions[EDITORTYPE_AUDIOFREEZE].height=20*maingui->GetFontSizeY();
	windowpositions[EDITORTYPE_AUDIOFREEZE].set=true;

	// StepEditor
	windowpositions[EDITORTYPE_RECORDEDITOR].x=0;
	windowpositions[EDITORTYPE_RECORDEDITOR].y=0;
	windowpositions[EDITORTYPE_RECORDEDITOR].width=77;
	windowpositions[EDITORTYPE_RECORDEDITOR].height=RECORDEDITORLINES*maingui->GetFontSizeY();
	windowpositions[EDITORTYPE_RECORDEDITOR].set=true;

	windowpositions[EDITORTYPE_SYNCEDITOR].x=0;
	windowpositions[EDITORTYPE_SYNCEDITOR].y=0;
	windowpositions[EDITORTYPE_SYNCEDITOR].width=320;
	windowpositions[EDITORTYPE_SYNCEDITOR].height=SYNCEDITORLINES*maingui->GetFontSizeY();
	windowpositions[EDITORTYPE_SYNCEDITOR].set=true;


	/*
	// Drummap
	windowpositions[EDITORTYPE_DRUMMAP].x=0;
	windowpositions[EDITORTYPE_DRUMMAP].y=0;
	windowpositions[EDITORTYPE_DRUMMAP].width=200;
	windowpositions[EDITORTYPE_DRUMMAP].height=170;
	windowpositions[EDITORTYPE_DRUMMAP].set=true;
	*/

	// BigTime
	windowpositions[EDITORTYPE_BIGTIME].x=0;
	windowpositions[EDITORTYPE_BIGTIME].y=0;
	windowpositions[EDITORTYPE_BIGTIME].width=400;
	windowpositions[EDITORTYPE_BIGTIME].height=120;
	windowpositions[EDITORTYPE_BIGTIME].set=true;

	// RMG
	windowpositions[EDITORTYPE_RMG].x=0;
	windowpositions[EDITORTYPE_RMG].y=0;
	windowpositions[EDITORTYPE_RMG].width=400;
	windowpositions[EDITORTYPE_RMG].height=350;
	windowpositions[EDITORTYPE_RMG].set=true;

	// Library
	windowpositions[EDITORTYPE_LIBRARY].x=0;
	windowpositions[EDITORTYPE_LIBRARY].y=0;
	windowpositions[EDITORTYPE_LIBRARY].width=300;
	windowpositions[EDITORTYPE_LIBRARY].height=450;

	// RMG
	/*
	windowpositions[EDITORTYPE_WIN32AUDIO].x=0;
	windowpositions[EDITORTYPE_WIN32AUDIO].y=0;
	windowpositions[EDITORTYPE_WIN32AUDIO].width=280;
	windowpositions[EDITORTYPE_WIN32AUDIO].height=100;
	windowpositions[EDITORTYPE_WIN32AUDIO].set=true;
	*/

	// Player
	windowpositions[EDITORTYPE_PLAYER].x=0;
	windowpositions[EDITORTYPE_PLAYER].y=0;
	windowpositions[EDITORTYPE_PLAYER].width=200;
	windowpositions[EDITORTYPE_PLAYER].height=170;
	windowpositions[EDITORTYPE_PLAYER].set=true;

	// Processor
	windowpositions[EDITORTYPE_PROCESSOR].x=0;
	windowpositions[EDITORTYPE_PROCESSOR].y=0;
	windowpositions[EDITORTYPE_PROCESSOR].width=500;
	windowpositions[EDITORTYPE_PROCESSOR].height=170;
	windowpositions[EDITORTYPE_PROCESSOR].set=true;

	// Keyboard
	windowpositions[EDITORTYPE_KEYBOARD].x=0;
	windowpositions[EDITORTYPE_KEYBOARD].y=0;
	windowpositions[EDITORTYPE_KEYBOARD].width=500;
	windowpositions[EDITORTYPE_KEYBOARD].height=170;
	windowpositions[EDITORTYPE_KEYBOARD].set=true;

	// Text
	windowpositions[EDITORTYPE_TEXT].x=0;
	windowpositions[EDITORTYPE_TEXT].y=0;
	windowpositions[EDITORTYPE_TEXT].width=500;
	windowpositions[EDITORTYPE_TEXT].height=400;
	windowpositions[EDITORTYPE_TEXT].set=true;

	// CF
	windowpositions[EDITORTYPE_CROSSFADE].x=0;
	windowpositions[EDITORTYPE_CROSSFADE].y=0;
	windowpositions[EDITORTYPE_CROSSFADE].width=400;
	windowpositions[EDITORTYPE_CROSSFADE].height=300;
	windowpositions[EDITORTYPE_CROSSFADE].set=true;

	// Marker
	windowpositions[EDITORTYPE_MARKER].x=0;
	windowpositions[EDITORTYPE_MARKER].y=0;
	windowpositions[EDITORTYPE_MARKER].width=600;
	windowpositions[EDITORTYPE_MARKER].height=400;
	windowpositions[EDITORTYPE_MARKER].set=true;

	// Toolbox
	windowpositions[EDITORTYPE_TOOLBOX].x=0;
	windowpositions[EDITORTYPE_TOOLBOX].y=0;
	windowpositions[EDITORTYPE_TOOLBOX].width=100;
	windowpositions[EDITORTYPE_TOOLBOX].height=300;
	windowpositions[EDITORTYPE_TOOLBOX].set=true;

	// CPU
	windowpositions[EDITORTYPE_CPU].x=0;
	windowpositions[EDITORTYPE_CPU].y=0;
	windowpositions[EDITORTYPE_CPU].width=32*maingui->GetFontSizeY();
	windowpositions[EDITORTYPE_CPU].height=100;
	windowpositions[EDITORTYPE_CPU].set=true;
	windowpositions[EDITORTYPE_CPU].dontchange=true;

	//MIDI Filter
	windowpositions[EDITORTYPE_MIDIFILTER].x=0;
	windowpositions[EDITORTYPE_MIDIFILTER].y=0;
	windowpositions[EDITORTYPE_MIDIFILTER].width=300;
	windowpositions[EDITORTYPE_MIDIFILTER].height=320;
	windowpositions[EDITORTYPE_MIDIFILTER].set=true;

	// Tempo Editor
	windowpositions[EDITORTYPE_TEMPO].x=0;
	windowpositions[EDITORTYPE_TEMPO].y=0;
	windowpositions[EDITORTYPE_TEMPO].width=450;
	windowpositions[EDITORTYPE_TEMPO].height=350;
	windowpositions[EDITORTYPE_TEMPO].set=true;
}

void Settings::SetMultiEditing(bool ed)
{
	openmultieditor=ed;

	guiWindow *win=maingui->FirstWindow();
	while(win)
	{
		if(win->menu_multiedit)
		{
			win->menu_multiedit->menu->Select(win->menu_multiedit->index,openmultieditor);
		}

		win=win->NextWindow();
	}
}

void Settings::SetLastAudioManagerFile(char *file)
{
	if(lastaudiomanagerfile)
		delete lastaudiomanagerfile;

	lastaudiomanagerfile=mainvar->GenerateString(file);
}

char *Settings::GetSettingsFileName(int type)
{
	if(settingsfilename[type])
		return settingsfilename[type];

	char *maindir=mainvar->GetUserDirectory();

	if(maindir)
	{
		settingsfilename[type]=new char[strlen(maindir)+100];

		if(settingsfilename[type])
		{
			strcpy(settingsfilename[type],maindir);

			switch(type)
			{
			case SETTINGSFILE_CRASHEDPLUGINS:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_CRASHEDPLUGINS_FILE);
				break;

			case SETTINGSFILE_MIDIDEVICES:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_MIDIDEVICES_DIR);
				break;

			case SETTINGSFILE_AUDIODEVICES:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_AUDIODEVICES_DIR);
				break;

			case SETTINGSFILE_PROCESSORS:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_PROCESSORS_FILE);
				break;

			case SETTINGSFILE_GUI:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_GUI_FILE);
				break;

			case SETTINGSFILE_SETTINGS:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_SETTINGS_FILE);
				break;

			case SETTINGSFILE_PLUGINS:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_SETTINGS_PLUGINS);
				break;

			case SETTINGSFILE_PLUGINTEST:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_PLUGINTEST_FILE);
				break;

			case SETTINGSFILE_PROJECTS:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_SETTINGS_PROJECTS);
				break;

			case SETTINGSFILE_DEVICEPROGRAMS:
				mainvar->AddString(settingsfilename[type],SETTINGSFILE_DEVICEPROGRAMS_DIR);
				break;
			}
		}
	}

	return settingsfilename[type];
}

void guiWindowPosition::Save(camxFile *file)
{
	file->Save_Chunk(x);
	file->Save_Chunk(y);
	file->Save_Chunk(width);
	file->Save_Chunk(height);
	file->Save_Chunk(maximized);
	file->Save_Chunk(set);
}

void guiWindowPosition::Load(camxFile *file)
{
	int hx,hy,hwidth,hheight;
	bool hset,hmaximized;

	file->ReadChunk(&hx);
	file->ReadChunk(&hy);
	file->ReadChunk(&hwidth);
	file->ReadChunk(&hheight);
	file->ReadChunk(&hmaximized);
	file->ReadChunk(&hset);

	if(hset==true && dontchange==false)
	{
		x=hx;
		y=hy;
		width=hwidth;
		height=hheight;
		set=true;
		maximized=hmaximized;
	}
}

Settings::Settings()
{
	showsamplescale=true;
	showsampleregions=true;
	allowtempochangerecording=true;
	plugincheck=PLUGINCHECK_TRYCATCH;
	precountertype=PRECOUNTER_ATMEASUREONE;
	automutechildsparent=true;
	loadsettingfile_version=0;
	audiodefaultdevice=0;
	defaultsettingsselection=0;
	openmultieditor=true;

	recordtofirstselectedMIDIPattern=false;
	showonlyaudiotracks=true;

	flag_unusedaudiofiles=SETTINGS_AUDIODEINITASK;

	lastaudiomanagerfile=0;
	defaultsonglength_measure=500;

	// Screen
	followeditor=true;
	transportdesktop=false;

	undosteps=UNDOSTEPS_50;

	opendoubleclickeditor=0;
	editordefaulttimeformat=WINDOWDISPLAY_MEASURE;

	createnewtrackaftercycle=false;
	createnewchildtrackwhenrecordingstarts=false;
	createnewtrack_trackadded=false;

	// Arrange
	shownotesinarrageeditor=ARRANGEEDITOR_SHOWNOTES_NOTES;
	showarrangecontrols=true;
	showarrangetracknumber=true;
	showarrangepatternlist=false;
	showarrangeshowallpattern=false; // only track

	showarrangetextmap=true;
	showarrangemarkermap=true;

	soloeditor=false; // 1 arrange per song etc...

	showbothsformatsintimeline=false;

	for(int i=0;i<LASTSETTINGS;i++)
		settingsfilename[i]=0;

	// Arrange
	arr_trackswidth=100;
	arr_patternlistwidth=100;
	arr_textmapheight=arr_markermapheight=24;
	arr_signaturemapheight=30;
	arr_tempomapheight=26;
	arr_foldermapheight=40;
	arr_overviewheight=50;

	// Drums
	drum_trackswidth=100;

	// Piano
	defaultpianoeditorlength=SAMPLESPERBEAT/4;

	lastsetnotelength=-1; // -1 not set
	piano_keyswidth=40;
	piano_overviewheight=40;
	piano_showwave=true;
	piano_keyheight=0.2; // 20% height

	piano_default_notelengthuseprev=true;
	playmouseover=true;
	defaultpianosettings=0;

	// Wave 
	wave_trackswidth=100;

	// Tempo
	tempo_timewidth=100;
	tempo_tempostringwidth=50;
	tempo_mapwidth=40;
	tempo_statuswidth=50;

	// Marker
	marker_positionwidth=100;
	marker_textwidth=100;
	marker_textnamewidth=70;
	marker_endwidth=100;
	marker_colourwidth=40;

	//Text
	text_positionwidth=100;
	text_textnamewidth=40;
	text_textwidth=40;
	text_textstringwidth=100;

	// Editor
	event_timewidth=100;
	event_statuswidth=50;
	event_channelwidth=30;
	event_byte1width=50;
	event_byte2width=50;
	event_byte3width=50;
	event_infowidth=70;
	event_curvewidth=40;
	event_showgfx=true;

	eventeditorgmmode=GM_OFF;

	// Big Time
	bigtime_showbw=false;

	//Manager
	manager_showinfo=true;
	manager_showregions=true;
	manager_showfullpath=false;
	manager_showtime=true;
	manager_showmb=false;
	manager_shownotfound=false;
	manager_showrecorded=false;
	manager_showonlysamplerate=true;
	manager_showintern=true;

	// All editors
	headerheight=30;

	waitforMIDIplayback=false;
	waitforMIDIrecord=false;

	usepremetronome=true;
	numberofpremetronomes=1;
	noteendsprecounter=true;

	sendprectrl=true;

#ifdef DEU
	projectstandardsmpte=Seq_Pos::POSMODE_SMPTE_25;
#else
	projectstandardsmpte=Seq_Pos::POSMODE_SMPTE_24;
#endif

	addgsgmtoMIDIfiles=ADDGS_TOMIDIFILE;
	autosavemin=2; // 0=0ff
	player_loopsongs=player_loopprojects=false;
	defaultnotetype=NOTETYPE_B;
	sendmetroplayback=false;
	displaynoteoff_monitor=false;
	displayeditortooltip=true;

#ifdef DEMO
	peakfiles=PEAKFILES_NOPEAKFILE;
#else
	peakfiles=PEAKFILES_TOFILE;
#endif

	// SMPTE Display offset
	smpteoffset_h=smpteoffset_m=smpteoffset_s=smpteoffset_f=smpteoffset_sf=0;
	showmonitordevicename=true;

	autoloadlastusedproject=true;

	arrangeeditorfiledisplay=1;

	for(int i=0;i<6;i++)prevprojects_dirname[i]=prevprojects_proname[i]=0;

	doublestopeditrefresh=true;
	audioresamplingformat=0; // no changes

	showmasterio=true;

	recording_MIDI=recording_audio=true;

	autocutzerosamples=true;

	usecheckaudiothreshold=false;
	checkaudiothreshhold=0.015;
	checkaudioendthreshold=0.005;

	autoupdatequestion=false;

	defaultprojectmeasureformat=PM_1p1p1p1;

	askimportaudiofiles=true;
	importaudiofiles=true;
	defaultmousequantize=MOUSEQUANTIZE_ZOOM;
	autoinstrument=true;

	defaultzoomx=15;
	defaultzoomy=30;

	outportsselection=
		inportsselection=
		audioinporttypeselection=
		audiooutporttypeselection=0;

	default_masternormalize=false;
	default_mastersavefirst=false;
	default_masterpausesamples=false;
	default_masterpausems=1000;

	default_masterformat=MASTERFORMAT_16BIT;
	default_masterchannels=CHANNELTYPE_STEREO;

	defaultrecordtracktype=TRACKTYPE_MIDI;
	autotrackMIDIinthru=true;
	autoaudioinput=true;
	importfilequestion=true;
	mixerzoom=3;

	for(int i=0;i<5;i++)
	{
		audiomixersettings[i]=SHOW_AUTO|SHOW_AUDIO|SHOW_MIDI|SHOW_TRACKS|SHOW_BUS|SHOW_PAN|SHOW_FX;
		audiomixersettings_solo[i]=SHOW_AUTO|SHOW_AUDIO|SHOW_MIDI|SHOW_PAN|SHOW_FX;
		autotracking[i]=AUTO_ALL;
		autopattering[i]=PATTERING_OFF;

		arrangetracking[i]=AUTO_ALL;
		arrangesettings[i]=SHOW_AUDIO|SHOW_AUTOMATION;

		pianosettings[i].status=NOTEON;
		pianosettings[i].channel=0;
		pianosettings[i].controlnr=0;
	}

	lastselectmixset=lastselectarrangeset=lastselectpianoset=0;

	showarrangelist=false;
	showarrangeeffects=true;
	defaulttemporange=0;

	showdrumlist=false;
	showdrumeffects=true;

	defaultmetrochl_b=10;
	defaultmetroport_b=0;
	defaultmetrokey_b=32;
	defaultmetrovelo_b=100;
	defaultmetrovelooff_b=0;

	defaultmetrochl_m=10;
	defaultmetroport_m=0;
	defaultmetrokey_m=75;
	defaultmetrovelo_m=127;
	defaultmetrovelooff_m=0;

	defaultmetrosendaudio=true; // default no MIDI metronom click
	defaultmetrosendMIDI=false;

	showarrangemaster=true;
	showarrangebus=false;
	showarrangemetro=false;

	defaultmetroaudiochanneltype=CHANNELTYPE_STEREO;
	defaultmetroaudiochannelportindex=0;
	mouseonlyvertical=true;
	automation_createpluginautomationtrack=false;
	automation_createvolumetrack=true;
	automation_createpantrack=true;

	automation_createMIDIvolumetrack=true;
	automation_createMIDImodulationtrack=automation_createMIDIpitchbendtrack=automation_createMIDIvelocitytrack=false;
	automation_createmute=automation_createsolo=false;

	automationrecordingrecord=automationrecordingplayback=true;

	defaultbigtimezoom=2;

#ifdef WIN32
	askforasio=true;
#endif

	checkspacenonfocus=true;
	showsamplevolume=true;
	
	realtimerecordtempoevents=REALTIMERECTEMPOEVENTS_CHANGE;

	splitMIDIfiletype0=SPLITMIDIFILE0_OFF;
	setfocustrackautotorecord=true;
};

Settings::~Settings()
{
	if(audiodefaultdevice)delete audiodefaultdevice;
	if(lastaudiomanagerfile)delete lastaudiomanagerfile;

	for(int i=0;i<LASTSETTINGS;i++)
		if(settingsfilename[i])delete settingsfilename[i];

	VSTPlugin_Settings *f=(VSTPlugin_Settings *)settingsreadvsteffects.GetRoot();
	while(f){
		f->FreeMemory();
		f=(VSTPlugin_Settings *)settingsreadvsteffects.RemoveO(f);
	}

	f=(VSTPlugin_Settings *)settingsreadvstinstruments.GetRoot();
	while(f){
		f->FreeMemory();
		f=(VSTPlugin_Settings *)settingsreadvstinstruments.RemoveO(f);
	}

	for(int i=0;i<6;i++){
		if(prevprojects_dirname[i])
			delete prevprojects_dirname[i];

		if(prevprojects_proname[i])
			delete prevprojects_proname[i];
	}

}

void Settings::DeleteAllVSTDirectories()
{
	for(int i=0;i<MAXVSTTYPES;i++)
	{
		Directory *f=FirstVSTDirectory(i);

		while(f){
			if(f->dir)
				delete f->dir;

			f=(Directory *)vstdirectories[i].RemoveO(f);
		}
	}
}

Directory *Settings::AddVSTDirectory(int ix,char *name)
{
	if(!name)return 0;

	// Dir exists ?
	Directory *c=FirstVSTDirectory(ix);
	while(c)
	{	
		if(strcmp(c->dir,name)==0)
			return 0;

		c=c->NextDirectory();
	}

	if(char *h=mainvar->GenerateString(name))
	{
		if(Directory *d=new Directory){

			d->dir=h;

			// Sort
			c=(Directory *)vstdirectories[ix].GetRoot();
			while(c)
			{
				if(mainvar->strcmp_allsmall(c->dir,d->dir)>0)
				{
					vstdirectories[ix].AddNextO(d,c);
					vstdirectories[ix].Close();

					return d;
				}

				c=c->NextDirectory();
			}

			vstdirectories[ix].AddEndO(d);
			return d;
		}

		delete h;
		return 0;
	}

	return 0;
}

Directory *Settings::AddAudioDirectory(char *name)
{
	if(!name)return 0;

	// Dir exists ?
	Directory *c=FirstAudioDirectory();

	while(c)
	{
		if(strcmp(c->dir,name)==0)
			return 0;

		c=c->NextDirectory();
	}

	if(char *h=mainvar->GenerateString(name))
	{
		if(Directory *d=new Directory){

			d->dir=h;

			// Sort
			c=(Directory *)audiodirectores.GetRoot();
			while(c)
			{
				if(mainvar->strcmp_allsmall(c->dir,d->dir)>0)
				{
					audiodirectores.AddNextO(d,c);
					audiodirectores.Close();
					return d;
				}

				c=c->NextDirectory();
			}

			audiodirectores.AddEndO(d);
			return d;
		}

		delete h;

	}//if h

	return 0;
}

void Settings::DeleteAllAudioDirectories()
{
	Directory *f=FirstAudioDirectory();

	while(f)
	{
		if(f->dir)
			delete f->dir;

		f=(Directory *)audiodirectores.RemoveO(f);
	}
}

Directory *Settings::DeleteAudioDirectory(Directory *del)
{
	if(del)
	{
		Directory *r=(Directory *)del->NextOrPrev();

		if(del->dir)
			delete del->dir;

		audiodirectores.RemoveO(del);
		return r;
	}

	return 0;
}

void Settings::LoadProjects()
{
	char *filename=GetSettingsFileName(SETTINGSFILE_PROJECTS);

	camxFile file;

	if(file.OpenRead(filename)==true && file.CheckVersion()==true)
	{
		file.LoadChunk();

		switch(file.GetChunkHeader())
		{
		case CHUNK_SETTINGSPROJECTS:
			{
				file.ChunkFound();

				if(!prevprojects_dirname[0]){

					// Last Active Project=0
					//prevprojects_dirname[0]=prevprojects_proname[0]=0;

					bool active=false;

					file.ReadChunk(&active);

					if(active==true)
					{
						char *old=0;
						char *old2=0;
						file.Read_ChunkString(&old);
						if(old)delete old;
						file.Read_ChunkString(&old2);
						if(old2)delete old2;
					}

					int prevpro=0;
					file.ReadChunk(&prevpro);

					//1-5 next Projects
					if(prevpro>6)
						prevpro=6;

					for(int i=0;i<prevpro;i++){

						prevprojects_dirname[i]=prevprojects_proname[i]=0;
						file.Read_ChunkString(&prevprojects_dirname[i]);
						file.Read_ChunkString(&prevprojects_proname[i]);
					}

#ifdef _DEBUG
					TRACE ("PrevProjects %d\n",prevpro);

					for(int i=0;i<6;i++)
					{
						TRACE ("Prev Projects %s\n",prevprojects_dirname[i]);

						if(!prevprojects_dirname[i])break; //Prev Project Directories

						camxFile checkpro;

						if(char *h=mainvar->GenerateString(prevprojects_dirname[i],"\\",PROJECTNAME))
						{
							if(checkpro.OpenRead(h)==false)
							{
								if(prevprojects_dirname[i])delete prevprojects_dirname[i];
								prevprojects_dirname[i]=0;

								if(prevprojects_proname[i])delete prevprojects_proname[i];
								prevprojects_proname[i]=0;

								for(int i2=i;i2<5;i2++)
								{
									prevprojects_dirname[i2]=prevprojects_dirname[i2+1];
									prevprojects_proname[i2]=prevprojects_proname[i2+1];
								}

								i=-1; // Reset to 0, for i++

								prevprojects_dirname[5]=prevprojects_proname[5]=0;
							}

							delete h;
						}

						checkpro.Close(true);
					}
#endif
				}

				file.CloseReadChunk();
			}
			break;
		}

		file.Close(true);
	}
}

void Settings::LoadPluginSettings()
{
	char *filename=GetSettingsFileName(SETTINGSFILE_PLUGINS);

	camxFile file;

	if(file.OpenRead(filename)==true && file.CheckVersion()==true)
	{
		file.LoadChunk();

		switch(file.GetChunkHeader())
		{
		case CHUNK_VSTSORT:
			{
				file.ChunkFound();

				int nr[2];
				nr[0]=nr[1]=0;

				file.ReadChunk(&nr[0]); // Effects
				file.ReadChunk(&nr[1]); // Instruments
				file.CloseReadChunk();

				// Effects ------------------------

				for(int i=0;i<2;i++)
				{
					while(nr[i]--){

						file.LoadChunk();

#ifdef WIN64
						if(file.GetChunkHeader()==CHUNK_VSTSORTFX64)
#else
						if(file.GetChunkHeader()==CHUNK_VSTSORTFX)
#endif								
						{
							file.ChunkFound();

							if(VSTPlugin_Settings *np=new VSTPlugin_Settings){

								OList *list=0;
								bool add=true;

								np->ReadSettings(&file);

								if(np->fulldllname && np->dllname)
								{
									if(i==0)
										list=&settingsreadvsteffects;
									else
										list=&settingsreadvstinstruments;

									// Check for Double...

									for(int i2=0;i2<2;i2++)
									{
										OList *clist;

										if(i2==0)
											clist=&settingsreadvsteffects;
										else
											clist=&settingsreadvstinstruments;

										VSTPlugin_Settings *check=(VSTPlugin_Settings *)list->GetRoot();
										while(check)
										{
											if(strcmp(check->fulldllname,np->fulldllname)==0)
											{
												add=false;
												break;
											}

											check=(VSTPlugin_Settings *)check->next;
										}
									}

									// dll exists ?
									if(add==true)
									{
										camxFile test;

										if(test.OpenRead(np->fulldllname)==false)
										{
											add=false;
#ifdef DEBUG
											maingui->MessageBoxOk(0,"VST Removed ?");
#endif
										}

										test.Close(true);
									}

									//loadsettingfile_version!=GetVersion force ReCheck of VST Plugins
									if(add==true && loadsettingfile_version==maingui->GetVersion())
									{
										if(list)
											list->AddEndO(np);
									}
									else
									{
										np->FreeMemory();
										delete np;
									}
								}
								else
								{
									np->FreeMemory();
									delete np;
								}
							}
						}
						else
						{
#ifdef WIN64
							if(file.GetChunkHeader()==CHUNK_VSTSORTFX)
								file.ChunkFound();
#else
							if(file.GetChunkHeader()==CHUNK_VSTSORTFX64)
								file.ChunkFound();
#endif	
						}

						file.CloseReadChunk();
					}
				}
			}
			break;
		}

		file.LoadChunk();

		if(file.GetChunkHeader()==CHUNK_SETTINGSVST) // VST2
		{
			file.ChunkFound();

			int nr=0;
			file.ReadChunk(&nr);

			while(nr--)
			{
				char *dir=0;

				file.Read_ChunkString(&dir);

				if(dir)
				{
					if(Directory *vstnew=AddVSTDirectory(VST2,dir))
					{
						// Check Directory
						//mainaudio->CollectVSTPlugins(VSTX86,vstnew,true);
					}

					delete dir;
				}
			}

			file.CloseReadChunk();
		}

		file.LoadChunk();

		if(file.GetChunkHeader()==CHUNK_SETTINGSVST3) // VST2
		{
			file.ChunkFound();

			int nr=0;
			file.ReadChunk(&nr);

			while(nr--)
			{
				char *dir=0;

				file.Read_ChunkString(&dir);

				if(dir)
				{
					if(Directory *vstnew=AddVSTDirectory(VST3,dir))
					{
						// Check Directory
						// mainaudio->CollectVSTPlugins(VST3X86,vstnew,true);
					}

					delete dir;
				}
			}

			file.CloseReadChunk();
		}

		file.Close(true);
	}

	maingui->SetInfoWindowText("Collect VST 2...");
	mainaudio->CollectVSTPlugins(VST2,0,true);
}

void Settings::Load(char *filename)
{
	if(filename)
	{
		// Get old File Version
		camxFile file;

		if(file.OpenRead(filename)==true && file.CheckVersion()==true)
		{
			// Get Settings File Version
			{
				camxFile testversion;

				if(testversion.OpenRead(filename)==true)
				{
					loadsettingfile_version=testversion.GetVersion();
					testversion.Close(true);
				}
			}

			bool ok=false;

			file.LoadChunk();

			if(file.GetChunkHeader()==CHUNK_SETTINGSHEADER)
			{
				file.ChunkFound();

				char text[4];
				text[0]=0;

				file.ReadChunk(text,4);

				if(text[0]=='C' &&
					text[1]=='A' &&
					text[2]=='P' &&
					text[3]=='X')
					ok=true;
				else
					maingui->MessageBoxOk(0,"Error: Settings File (C-A-P-X)");

				file.CloseReadChunk();
			}
			else
				maingui->MessageBoxOk(0,"Error: Settings File (HEADER)");

			if(ok==true)
			{
				while(file.eof==false)
				{
					file.LoadChunk();

					if(file.eof==false)
						switch(file.GetChunkHeader())
					{
						case CHUNK_SETTINGSPOINTER:
							{
								file.ChunkFound();

								file.ReadChunk(&opendoubleclickeditor);
								file.ReadChunk(&editordefaulttimeformat);

								// Arrange
								file.ReadChunk(&shownotesinarrageeditor);
								file.ReadChunk(&showarrangecontrols);
								file.ReadChunk(&showarrangetracknumber);
								file.ReadChunk(&showarrangepatternlist);
								file.ReadChunk(&showarrangeshowallpattern);

								file.ReadChunk(&showarrangetextmap);
								file.ReadChunk(&showarrangemarkermap);

								file.ReadChunk(&waitforMIDIplayback);
								file.ReadChunk(&waitforMIDIrecord);
								file.ReadChunk(&usepremetronome);
								file.ReadChunk(&numberofpremetronomes);
								file.ReadChunk(&sendprectrl);
								file.ReadChunk(&projectstandardsmpte);

#ifdef DEU
								if(projectstandardsmpte<Seq_Pos::POSMODE_SMPTE_24 || projectstandardsmpte<Seq_Pos::POSMODE_SMPTE_60)
									projectstandardsmpte=Seq_Pos::POSMODE_SMPTE_25;
#else
								if(projectstandardsmpte<Seq_Pos::POSMODE_SMPTE_24 || projectstandardsmpte<Seq_Pos::POSMODE_SMPTE_60)
									projectstandardsmpte=Seq_Pos::POSMODE_SMPTE_24;
#endif

								file.ReadChunk(&defaultpianoeditorlength);
								file.ReadChunk(&addgsgmtoMIDIfiles);
								file.ReadChunk(&createnewtrackaftercycle);
								file.ReadChunk(&noteendsprecounter);
								file.ReadChunk(&autosavemin);
								file.ReadChunk(&sendmetroplayback);

								file.ReadChunk(&displaynoteoff_monitor);

								file.ReadChunk(&smpteoffset_h);
								file.ReadChunk(&smpteoffset_m);
								file.ReadChunk(&smpteoffset_s);
								file.ReadChunk(&smpteoffset_f);
								file.ReadChunk(&smpteoffset_sf);

								file.ReadChunk(&displayeditortooltip);
								file.ReadChunk(&arrangeeditorfiledisplay);
								file.ReadChunk(&followeditor);
								file.ReadChunk(&flag_unusedaudiofiles);

								file.ReadChunk(&createnewchildtrackwhenrecordingstarts);
								file.ReadChunk(&transportdesktop);
								file.ReadChunk(&doublestopeditrefresh);

								file.ReadChunk(&defaultsonglength_measure);

								file.Read_ChunkString(&lastaudiomanagerfile);

								file.ReadChunk(&piano_default_notelengthuseprev);
								file.ReadChunk(&showonlyaudiotracks);

								file.ReadChunk(&mainvar->autoupdatecheck);
								file.ReadChunk(&openmultieditor);

								file.ReadChunk(&showmasterio);

								file.ReadChunk(&recording_MIDI);
								file.ReadChunk(&recording_audio);

								file.ReadChunk(&usecheckaudiothreshold);

								ARES check;
								file.ReadChunk(&check);

								if(check>=0)
									checkaudiothreshhold=check;

								file.ReadChunk(&autoupdatequestion);
								file.ReadChunk(&importaudiofiles);

								file.ReadChunk(&defaultmousequantize);
								file.ReadChunk(&autoinstrument);

								file.ReadChunk(&defaultzoomx);
								if(defaultzoomx>=mainvar->numberwinzooms)
									defaultzoomx=mainvar->numberwinzooms-1;

								file.ReadChunk(&defaultzoomy);

								file.ReadChunk(&outportsselection);
								file.ReadChunk(&inportsselection);
								file.ReadChunk(&audioinporttypeselection);
								file.ReadChunk(&audiooutporttypeselection);

								file.ReadChunk(&askimportaudiofiles);

								file.ReadChunk(&default_masternormalize);
								file.ReadChunk(&default_mastersavefirst);
								file.ReadChunk(&default_masterformat);

								file.ReadChunk(&defaultrecordtracktype);
								file.ReadChunk(&recordtofirstselectedMIDIPattern);

								file.ReadChunk(&autocutzerosamples);
								file.ReadChunk(&autotrackMIDIinthru);
								file.ReadChunk(&autoaudioinput);

								file.ReadChunk(&importfilequestion);
								file.ReadChunk(&automutechildsparent);

								file.ReadChunk(&checkaudioendthreshold);
								file.ReadChunk(&precountertype);
								file.ReadChunk(&allowtempochangerecording);
								file.ReadChunk(&ex_splitMIDIfilestype0);

								file.ReadChunk(&mixerzoom);

								for(int i=0;i<5;i++)
									file.ReadChunk(&audiomixersettings[i]);

								file.ReadChunk(&lastselectmixset);

								for(int i=0;i<5;i++)
								{
									file.ReadChunk(&autotracking[i]);
									file.ReadChunk(&autopattering[i]);
								}

								for(int i=0;i<5;i++)
									file.ReadChunk(&audiomixersettings_solo[i]);

								for(int i=0;i<5;i++)
									file.ReadChunk(&arrangetracking[i]);

								file.ReadChunk(&lastselectarrangeset);

								for(int i=0;i<5;i++)
									file.ReadChunk(&arrangesettings[i]);

								file.ReadChunk(&defaultpianosettings);
								file.ReadChunk(&defaulttemporange);
								file.ReadChunk(&showsamplescale);

								file.ReadChunk(&showarrangeeffects);
								file.ReadChunk(&showarrangelist);
								file.ReadChunk(&showdrumeffects);
								file.ReadChunk(&showdrumlist);

								file.ReadChunk(&defaultmetrochl_b);
								file.ReadChunk(&defaultmetroport_b);
								file.ReadChunk(&defaultmetrokey_b);
								file.ReadChunk(&defaultmetrovelo_b);
								file.ReadChunk(&defaultmetrovelooff_b);

								file.ReadChunk(&defaultmetrochl_m);
								file.ReadChunk(&defaultmetroport_m);
								file.ReadChunk(&defaultmetrokey_m);
								file.ReadChunk(&defaultmetrovelo_m);
								file.ReadChunk(&defaultmetrovelooff_m);

								file.ReadChunk(&defaultmetrosendMIDI);
								file.ReadChunk(&defaultmetrosendaudio);
								file.ReadChunk(&mouseonlyvertical);
								file.ReadChunk(&automation_createpluginautomationtrack);
								file.ReadChunk(&automation_createvolumetrack);
								file.ReadChunk(&automation_createpantrack);

								file.ReadChunk(&automation_createMIDIvolumetrack);
								file.ReadChunk(&automation_createMIDImodulationtrack);
								file.ReadChunk(&automation_createMIDIpitchbendtrack);
								file.ReadChunk(&automation_createMIDIvelocitytrack);
								file.ReadChunk(&defaultbigtimezoom);
								file.ReadChunk(&automation_createmute);
								file.ReadChunk(&automation_createsolo);

								file.ReadChunk(&automationrecordingrecord);
								file.ReadChunk(&automationrecordingplayback);
								file.ReadChunk(&askforasio);
								file.ReadChunk(&default_masterpausesamples);
								file.ReadChunk(&default_masterpausems);
								file.ReadChunk(&checkspacenonfocus);
								file.ReadChunk(&showsampleregions);
								file.ReadChunk(&showsamplevolume);
								file.ReadChunk(&default_masterchannels);
								file.ReadChunk(&splitMIDIfiletype0);
								file.ReadChunk(&setfocustrackautotorecord);
								
								file.CloseReadChunk();
							}
							break;

						case CHUNK_SETTINGSWINDOWPOSITIONS:
							{
								file.ChunkFound();
								int nr=0;
								file.ReadChunk(&nr);

								for(int i=0;i<nr;i++)
									mainsettings->windowpositions[i].Load(&file);

								file.CloseReadChunk();
							}
							break;


						case CHUNK_SETTINGSAUDIODIR:
							{
								file.ChunkFound();

								int nr=0;
								file.ReadChunk(&nr);

								while(nr--)
								{
									char *dir=0;

									file.Read_ChunkString(&dir);

									if(dir)
									{
										Directory *adnew;

										if(adnew=AddAudioDirectory(dir))
										{
											// Check Directory
											mainaudio->CollectAudioFiles(dir,adnew);
										}

										delete dir;
									}
								}

								file.CloseReadChunk();
							}
							break;

						case CHUNK_SETTINGSMIDIINPUTPORTS:
							{
								mainMIDI->LoadPorts(&file);
							}
							break;

						case CHUNK_SETTINGSDEFAULT:
							{
								file.ChunkFound();

								//char *defaultaudio=0;
								//char *defaultsoundcard=0;
								char *defaultMIDI=0;

								file.Read_ChunkString(&mainaudio->old_hardwarename); // Driver System
								file.Read_ChunkString(&mainaudio->old_hardwaredevice); // Soundcard

								//mainaudio->SetAudioSystem(defaultaudio);
								//mainaudio->SetSoundCard(defaultsoundcard);

								file.Read_ChunkString(&defaultMIDI); // MIDI OutputDevice

								mainMIDI->SetDefaultMIDIDevice(defaultMIDI);

								if(defaultMIDI)
								{
									delete defaultMIDI;
									defaultMIDI=0;
								}

								file.ReadChunk(&mainMIDI->sendcontrolsolomute);
								file.ReadChunk(&peakfiles);
								file.ReadChunk(&defaultnotetype);
								file.ReadChunk(&undosteps);
								file.ReadChunk(&soloeditor);
								file.ReadChunk(&showbothsformatsintimeline);

								bool exmtc;

								file.ReadChunk(&exmtc);
								file.ReadChunk(&exmtc);

								file.ReadChunk(&mainMIDI->receiveMIDIstart);
								file.ReadChunk(&mainMIDI->receiveMIDIstop);

								file.ReadChunk(&exmtc);
								file.ReadChunk(&exmtc);

								file.ReadChunk(&mainMIDI->quantizesongpositiontoMIDIclock);

								file.ReadChunk(&showmonitordevicename);
								file.ReadChunk(&autoloadlastusedproject);

								file.ReadChunk(&mainMIDI->sendnoteprevcylce);
								file.ReadChunk(&audioresamplingformat);

								file.ReadChunk(&playmouseover);
								file.ReadChunk(&defaultprojectmeasureformat);

								file.Read_ChunkString(&audiodefaultdevice);
								file.ReadChunk(&plugincheck);

								file.ReadChunk(&showarrangemaster);
								file.ReadChunk(&showarrangebus);
								file.ReadChunk(&showarrangemetro);

								file.ReadChunk(&defaultmetroaudiochanneltype);
								file.ReadChunk(&defaultmetroaudiochannelportindex);

								file.ReadChunk(&manager_showregions);
								file.ReadChunk(&realtimerecordtempoevents);

								file.CloseReadChunk();
							}
							break;

							/*
							case CHUNK_SETTINGSMIDIINFILTER:
							{
							file.ChunkFound();

							MIDIFilter dummy;

							long nrfilter=0;

							file.ReadChunk(&nrfilter);

							while(nrfilter--)
							{
							char *name=0;

							file.Read_ChunkString(&name);

							dummy.LoadData(&file);

							// Find Devices
							MIDIInputDevice *id=mainMIDI->FirstMIDIInputDevice();
							while(id)
							{
							if(strcmp(id->name,name)==0)
							{
							dummy.Clone(&id->inputfilter);
							break;
							}

							id=id->NextInputDevice();
							}

							if(name)
							delete name;
							}

							file.CloseReadChunk();
							}
							break;
							*/

							/*
							case CHUNK_SETTINGSMIDIINDEVICEINFO:
							{
							file.ChunkFound();

							int nr=0;

							file.ReadChunk(&nr);

							while(nr>0)
							{
							bool receivesync;
							int MIDIintempoquant;
							char *name=0;
							char *info=0;
							MIDIFilter filter;

							file.ReadChunk(&receivesync);
							file.ReadChunk(&MIDIintempoquant);
							file.Read_ChunkString(&name);
							file.Read_ChunkString(&info);

							filter.LoadData(&file);

							if(info && name)
							{
							// Find Input Device 
							MIDIInputDevice *id=mainMIDI->FirstMIDIInputDevice();
							while(id)
							{
							if(strcmp(id->name,name)==0)
							{
							id->receivesync=receivesync;
							id->MIDIclockraster=MIDIintempoquant;

							strcpy(id->userinfo,info);
							id->CreateFullName();
							filter.Clone(&id->inputfilter); // Copy Filter

							break;
							}

							id=id->NextInputDevice();
							}
							}

							if(name)
							delete name;

							if(info)
							delete info;

							nr--;
							}

							file.CloseReadChunk();
							}
							break;

							case CHUNK_SETTINGSMIDIOUTDEVICEINFO:
							{
							file.ChunkFound();
							int nr=0;
							file.ReadChunk(&nr);
							file.CloseReadChunk();

							while(nr>0)
							{
							file.LoadChunk();

							if(file.GetChunkHeader()==CHUNK_SETTINGSMIDIOUTDEVICE)
							{
							file.ChunkFound();

							char *name=0;
							char *info=0;

							bool sendMIDIcontrol=true;
							bool sendmtc=false;

							MIDIFilter filter;

							file.Read_ChunkString(&name);
							file.Read_ChunkString(&info);

							file.ReadChunk(&sendMIDIcontrol);
							file.ReadChunk(&sendmtc);

							filter.LoadData(&file);

							// End Read Data

							if(info && name)
							{
							// Find Output Device
							MIDIOutputDevice *od=mainMIDI->FirstMIDIOutputDevice();
							while(od)
							{
							if(strcmp(od->name,name)==0) // Same Device found
							{
							// Copy Data
							strcpy(od->userinfo,info);
							od->CreateFullName();

							filter.Clone(&od->device_eventfilter); // Copy Filter

							od->sendmtc=sendmtc;
							od->sendMIDIcontrol=sendMIDIcontrol;

							break;
							}

							od=od->NextOutputDevice();
							}
							}

							if(name)
							delete name;

							if(info)
							delete info;

							file.CloseReadChunk();

							}
							else
							break;

							nr--;
							}
							}
							break;
							*/

						default: // unknown 

							TRACE ("Unknown Settings CHUNK %d\n",file.GetChunkHeader());

							file.JumpOverChunk();
							break;
					}
				}
			}
		}
#ifdef _DEBUG
		else
			MessageBox(NULL,"Not Settings File found","Error",MB_OK);
#endif

		file.Close(true);	
	}
}

void Settings::LoadDevices()
{
	// MIDIInput Device Info
	MIDIInputDevice *iinfo=mainMIDI->FirstMIDIInputDevice();

	while(iinfo)
	{
		if(char *filename=GetSettingsFileName(SETTINGSFILE_MIDIDEVICES)){

			if(char *clearstring=mainvar->CreateSimpleASCIIString(iinfo->name))
			{
				if(char *h=mainvar->GenerateString(filename,"Input\\",clearstring))
				{
					camxFile file;

					if(file.OpenRead(h)==true && file.CheckVersion()==true)
					{
						file.LoadChunk();

						if(file.GetChunkHeader()==CHUNK_SETTINGSMIDIINDEVICEINFO)
						{
							file.ChunkFound();

							bool exreceivesync;
							file.ReadChunk(&exreceivesync);
							file.ReadChunk(&iinfo->MIDIclockraster);
							file.Read_ChunkString(iinfo->userinfo);

							iinfo->inputfilter.LoadData(&file);

							file.CloseReadChunk();
						}
					}

					file.Close(true);
					delete h;
				}
				delete clearstring;
			}
		}

		iinfo=iinfo->NextInputDevice();
	}

	MIDIOutputDevice *oinfo=mainMIDI->FirstMIDIOutputDevice();

	while(oinfo)
	{
		if(char *filename=GetSettingsFileName(SETTINGSFILE_MIDIDEVICES)){

			if(char *clearstring=mainvar->CreateSimpleASCIIString(oinfo->name))
			{
				if(char *h=mainvar->GenerateString(filename,"Output\\",clearstring))
				{
					camxFile file;

					if(file.OpenRead(h)==true && file.CheckVersion()==true)
					{
						file.LoadChunk();

						if(file.GetChunkHeader()==CHUNK_SETTINGSMIDIOUTDEVICEINFO)
						{
							file.ChunkFound();

							file.Read_ChunkString(oinfo->userinfo);

							bool exMIDIsync;
							file.ReadChunk(&exMIDIsync);
							bool exMIDImtc;
							file.ReadChunk(&exMIDImtc);

							oinfo->device_eventfilter.LoadData(&file);

							/*
							file.ReadChunk(&oinfo->sendmetronome);
							file.ReadChunk(&oinfo->sendmetronome_channel);
							file.ReadChunk(&oinfo->sendmetronome_key);
							file.ReadChunk(&oinfo->sendmetronome_velocity);
							file.ReadChunk(&oinfo->sendmetronome_channel_hi);
							file.ReadChunk(&oinfo->sendmetronome_key_hi);
							file.ReadChunk(&oinfo->sendmetronome_velocity_hi);
							*/
							file.CloseReadChunk();
						}
					}

					file.Close(true);
					delete h;
				}
				delete clearstring;
			}
		}

		oinfo=oinfo->NextOutputDevice();
	}
}

void Settings::SaveDevices()
{
	// MIDIInput Device Info
	MIDIInputDevice *iinfo=mainMIDI->FirstMIDIInputDevice();

	while(iinfo)
	{
		if(char *filename=GetSettingsFileName(SETTINGSFILE_MIDIDEVICES)){

			if(char *clearstring=mainvar->CreateSimpleASCIIString(iinfo->name))
			{
				if(char *h=mainvar->GenerateString(filename,"Input\\",clearstring))
				{
					camxFile file;

					if(file.OpenSave_CheckVersion(h)==true)
					{
						file.SaveVersion();

						file.OpenChunk(CHUNK_SETTINGSMIDIINDEVICEINFO);

						bool exreceivesync=false;
						file.Save_Chunk(exreceivesync);

						file.Save_Chunk(iinfo->MIDIclockraster);
						file.Save_ChunkString(iinfo->userinfo);

						iinfo->inputfilter.SaveData(&file); // Filter

						file.CloseChunk();
					}

					file.Close(true);
					delete h;
				}
				delete clearstring;
			}
		}

		iinfo=iinfo->NextInputDevice();
	}

	MIDIOutputDevice *oinfo=mainMIDI->FirstMIDIOutputDevice();

	while(oinfo)
	{
		if(char *filename=mainsettings->GetSettingsFileName(SETTINGSFILE_MIDIDEVICES)){

			if(char *clearstring=mainvar->CreateSimpleASCIIString(oinfo->name))
			{
				if(char *h=mainvar->GenerateString(filename,"Output\\",clearstring))
				{
					camxFile file;

					if(file.OpenSave_CheckVersion(h)==true)
					{
						file.SaveVersion();

						file.OpenChunk(CHUNK_SETTINGSMIDIOUTDEVICEINFO);

						file.Save_ChunkString(oinfo->userinfo);

						bool exMIDIsync=true;
						file.Save_Chunk(exMIDIsync);
						bool exmtc=false;
						file.Save_Chunk(exmtc);

						oinfo->device_eventfilter.SaveData(&file); // Filter

						/*
						file.Save_Chunk(oinfo->sendmetronome);
						file.Save_Chunk(oinfo->sendmetronome_channel);
						file.Save_Chunk(oinfo->sendmetronome_key);
						file.Save_Chunk(oinfo->sendmetronome_velocity);
						file.Save_Chunk(oinfo->sendmetronome_channel_hi);
						file.Save_Chunk(oinfo->sendmetronome_key_hi);
						file.Save_Chunk(oinfo->sendmetronome_velocity_hi);
						*/

						file.CloseChunk();
					}

					file.Close(true);
					delete h;
				}
				delete clearstring;
			}
		}

		oinfo=oinfo->NextOutputDevice();
	}

	// Audio Devices
	AudioHardware *ahw=mainaudio->FirstAudioHardware();
	while(ahw)
	{
		AudioDevice *ad=ahw->FirstDevice();
		while(ad)
		{
			if(char *filename=GetSettingsFileName(SETTINGSFILE_AUDIODEVICES)){

				if(char *clearstring=mainvar->CreateSimpleASCIIString(ad->devname))
				{
					if(char *h=mainvar->GenerateString(filename,ahw->name,"_",clearstring))
					{
						camxFile file;

						if(file.OpenSave_CheckVersion(h)==true)
						{
							file.SaveVersion();

							ad->Save(&file);

							/*
							file.OpenChunk(CHUNK_SETTINGSMIDIOUTDEVICEINFO);

							file.Save_ChunkString(oinfo->userinfo);
							file.Save_Chunk(oinfo->sendMIDIcontrol);
							file.Save_Chunk(oinfo->sendmtc);

							oinfo->device_eventfilter.SaveData(&file); // Filter
							*/

							//file.CloseChunk();
						}

						file.Close(true);
						delete h;
					}
					delete clearstring;
				}
			}

			ad=ad->NextDevice();
		}

		ahw=ahw->NextHardware();
	}
}

void Settings::SaveProjects()
{
	char *filename=GetSettingsFileName(SETTINGSFILE_PROJECTS);

	camxFile file;

	if(file.OpenSave_CheckVersion(filename)==true)
	{
		file.SaveVersion();

		file.OpenChunk(CHUNK_SETTINGSPROJECTS);

		// Active Project
		bool active;

		if(mainvar->GetActiveProject()) // 1. Active Project
		{
			active=true;
			file.Save_Chunk(active);
			file.Save_ChunkString(mainvar->GetActiveProject()->projectdirectory); // Save Active Project Name
			file.Save_ChunkString(mainvar->GetActiveProject()->name);
		}
		else
		{
			active=false;
			file.Save_Chunk(active);
		}

		int c;

		// Write Max 5 Last Projects

		for(c=0;c<6;c++)if(!prevprojects_dirname[c])break;

		file.Save_Chunk(c); // save number projects

		for(int i=0;i<c;i++)
		{
			file.Save_ChunkString(prevprojects_dirname[i]);
			file.Save_ChunkString(prevprojects_proname[i]);
		}

		file.CloseChunk();

		file.Close(true);	
	}
}

void Settings::LoadCrashedPlugins()
{
	char *filename=GetSettingsFileName(SETTINGSFILE_CRASHEDPLUGINS);

	camxFile file;

	if(file.OpenRead(filename)==true)
	{
		long c=0;

		file.Read(&c,sizeof(long));

		while(c--)
		{
			short l=0;
			file.Read(&l,sizeof(short));

			if(char *fl=new char[l+1])
			{
				file.Read(fl,l);
				fl[l]=0;

				mainaudio->AddCrashedPlugin(fl,false);
				delete fl;
			}
		}

		file.Close(true);	
	}
}

void Settings::SaveCrashedPlugins()
{
	char *filename=GetSettingsFileName(SETTINGSFILE_CRASHEDPLUGINS);

	long c=mainaudio->crashplugins.GetCount();

	if(c>0)
	{
		camxFile file;

		if(file.OpenSave(filename)==true)
		{
			file.Save(&c,sizeof(long));

			// Close Crashed Plugins
			CrashedPlugin *cp=(CrashedPlugin *)mainaudio->crashplugins.GetRoot();
			while(cp)
			{
				short i=strlen(cp->fulldllname);

				file.Save(&i,sizeof(short));
				file.Save(cp->fulldllname);

				cp=(CrashedPlugin *)cp->next;

			}

			file.Close(true);	
		}
	}
	else
		mainvar->DeleteAFile(filename);
}

void Settings::SavePluginSettings()
{
	char *filename=GetSettingsFileName(SETTINGSFILE_PLUGINS);

	camxFile file;

	if(file.OpenSave_CheckVersion(filename)==true)
	{
		file.SaveVersion();

		file.OpenChunk(CHUNK_VSTSORT); // Save BEFORE CHUNK CHUNK_SETTINGSVST
		{
			{
				int fxc=0;
				int istc=0;

				VSTPlugin *fx=mainaudio->FirstVSTEffect();
				while(fx)
				{
					if(fx->CheckSettings()==true)
						fxc++;

					fx=fx->NextVSTPlugin();
				}

				VSTPlugin *ins=mainaudio->FirstVSTInstrument();
				while(ins)
				{
					if(ins->CheckSettings()==true)
						istc++;

					ins=ins->NextVSTPlugin();
				}

				file.Save_Chunk(fxc);
				file.Save_Chunk(istc);
			}

			file.CloseChunk();

			// Effects ------------------------
			VSTPlugin *vfx=mainaudio->FirstVSTEffect();
			while(vfx)
			{
				if(vfx->CheckSettings()==true)
				{
#ifdef WIN64
					file.OpenChunk(CHUNK_VSTSORTFX64);
#else
					file.OpenChunk(CHUNK_VSTSORTFX);
#endif
					vfx->SaveSettings(&file);
					file.CloseChunk();
				}

				vfx=vfx->NextVSTPlugin();
			}

			// Instruments --------------------
			vfx=mainaudio->FirstVSTInstrument();
			while(vfx)
			{
				if(vfx->CheckSettings()==true)
				{
#ifdef WIN64
					file.OpenChunk(CHUNK_VSTSORTFX64);
#else
					file.OpenChunk(CHUNK_VSTSORTFX);
#endif
					vfx->SaveSettings(&file);
					file.CloseChunk();
				}

				vfx=vfx->NextVSTPlugin();
			}
		}

		// Save VST Dir's
		if(Directory *vst=FirstVSTDirectory(VST2))
		{
			file.OpenChunk(CHUNK_SETTINGSVST);
			file.Save_Chunk(GetCountOfVSTDirectories(VST2));

			while(vst)
			{
				file.Save_ChunkString(vst->dir);
				vst=vst->NextDirectory();
			}

			file.CloseChunk();
		}

		// Save VST 3 Dir's
		if(Directory *vst=FirstVSTDirectory(VST3))
		{
			file.OpenChunk(CHUNK_SETTINGSVST3);
			file.Save_Chunk(GetCountOfVSTDirectories(VST3));

			while(vst)
			{
				file.Save_ChunkString(vst->dir);
				vst=vst->NextDirectory();
			}

			file.CloseChunk();
		}

		file.Close(true);	
	}
}

void Settings::Save(char *filename)
{
	if(!filename)filename=GetSettingsFileName(SETTINGSFILE_SETTINGS);

	if(filename){
		camxFile file;

		if(file.OpenSave_CheckVersion(filename)==true)
		{
			file.SaveVersion();

			// Header
			file.OpenChunk(CHUNK_SETTINGSHEADER);
			file.Save_Chunk("CAPX",4);
			file.CloseChunk();

			// Pointer
			file.OpenChunk(CHUNK_SETTINGSPOINTER);

			file.Save_Chunk(opendoubleclickeditor);
			file.Save_Chunk(editordefaulttimeformat);

			// Arrange
			file.Save_Chunk(shownotesinarrageeditor);
			file.Save_Chunk(showarrangecontrols);
			file.Save_Chunk(showarrangetracknumber);
			file.Save_Chunk(showarrangepatternlist);
			file.Save_Chunk(showarrangeshowallpattern);

			file.Save_Chunk(showarrangetextmap);
			file.Save_Chunk(showarrangemarkermap);

			// Trigger
			file.Save_Chunk(waitforMIDIplayback);
			file.Save_Chunk(waitforMIDIrecord);

			file.Save_Chunk(usepremetronome);
			file.Save_Chunk(numberofpremetronomes);
			file.Save_Chunk(sendprectrl);
			file.Save_Chunk(projectstandardsmpte);
			file.Save_Chunk(defaultpianoeditorlength);
			file.Save_Chunk(addgsgmtoMIDIfiles);
			file.Save_Chunk(createnewtrackaftercycle);
			file.Save_Chunk(noteendsprecounter);
			file.Save_Chunk(autosavemin);
			file.Save_Chunk(sendmetroplayback);
			file.Save_Chunk(displaynoteoff_monitor);

			file.Save_Chunk(smpteoffset_h);
			file.Save_Chunk(smpteoffset_m);
			file.Save_Chunk(smpteoffset_s);
			file.Save_Chunk(smpteoffset_f);
			file.Save_Chunk(smpteoffset_sf);

			file.Save_Chunk(displayeditortooltip);
			file.Save_Chunk(arrangeeditorfiledisplay);

			file.Save_Chunk(followeditor);
			file.Save_Chunk(flag_unusedaudiofiles);

			file.Save_Chunk(createnewchildtrackwhenrecordingstarts);
			file.Save_Chunk(transportdesktop);
			file.Save_Chunk(doublestopeditrefresh);

			file.Save_Chunk(defaultsonglength_measure);

			file.Save_ChunkString(lastaudiomanagerfile);
			file.Save_Chunk(piano_default_notelengthuseprev);
			file.Save_Chunk(showonlyaudiotracks);
			file.Save_Chunk(mainvar->autoupdatecheck);
			file.Save_Chunk(openmultieditor);

			file.Save_Chunk(showmasterio);

			file.Save_Chunk(recording_MIDI);
			file.Save_Chunk(recording_audio);

			file.Save_Chunk(usecheckaudiothreshold);
			file.Save_Chunk(checkaudiothreshhold);

			file.Save_Chunk(autoupdatequestion);
			file.Save_Chunk(importaudiofiles);

			file.Save_Chunk(defaultmousequantize);
			file.Save_Chunk(autoinstrument);

			file.Save_Chunk(defaultzoomx);
			file.Save_Chunk(defaultzoomy);

			file.Save_Chunk(outportsselection);
			file.Save_Chunk(inportsselection);
			file.Save_Chunk(audioinporttypeselection);
			file.Save_Chunk(audiooutporttypeselection);

			file.Save_Chunk(askimportaudiofiles);

			file.Save_Chunk(default_masternormalize);
			file.Save_Chunk(default_mastersavefirst);
			file.Save_Chunk(default_masterformat);

			file.Save_Chunk(defaultrecordtracktype);
			file.Save_Chunk(recordtofirstselectedMIDIPattern);

			file.Save_Chunk(autocutzerosamples);
			file.Save_Chunk(autotrackMIDIinthru);
			file.Save_Chunk(autoaudioinput);

			file.Save_Chunk(importfilequestion);
			file.Save_Chunk(automutechildsparent);

			file.Save_Chunk(checkaudioendthreshold);
			file.Save_Chunk(precountertype);
			file.Save_Chunk(allowtempochangerecording);
			file.Save_Chunk(ex_splitMIDIfilestype0);

			file.Save_Chunk(mixerzoom);

			for(int i=0;i<5;i++)
				file.Save_Chunk(audiomixersettings[i]);

			file.Save_Chunk(lastselectmixset);

			for(int i=0;i<5;i++)
			{
				file.Save_Chunk(autotracking[i]);
				file.Save_Chunk(autopattering[i]);
			}

			for(int i=0;i<5;i++)
				file.Save_Chunk(audiomixersettings_solo[i]);

			for(int i=0;i<5;i++)
				file.Save_Chunk(arrangetracking[i]);

			file.Save_Chunk(lastselectarrangeset);

			for(int i=0;i<5;i++)
				file.Save_Chunk(arrangesettings[i]);

			file.Save_Chunk(defaultpianosettings);
			file.Save_Chunk(defaulttemporange);
			file.Save_Chunk(showsamplescale);

			file.Save_Chunk(showarrangeeffects);
			file.Save_Chunk(showarrangelist);
			file.Save_Chunk(showdrumeffects);
			file.Save_Chunk(showdrumlist);

			file.Save_Chunk(defaultmetrochl_b);
			file.Save_Chunk(defaultmetroport_b);
			file.Save_Chunk(defaultmetrokey_b);
			file.Save_Chunk(defaultmetrovelo_b);
			file.Save_Chunk(defaultmetrovelooff_b);

			file.Save_Chunk(defaultmetrochl_m);
			file.Save_Chunk(defaultmetroport_m);
			file.Save_Chunk(defaultmetrokey_m);
			file.Save_Chunk(defaultmetrovelo_m);
			file.Save_Chunk(defaultmetrovelooff_m);

			file.Save_Chunk(defaultmetrosendMIDI);
			file.Save_Chunk(defaultmetrosendaudio);
			file.Save_Chunk(mouseonlyvertical);
			file.Save_Chunk(automation_createpluginautomationtrack);
			file.Save_Chunk(automation_createvolumetrack);
			file.Save_Chunk(automation_createpantrack);

			file.Save_Chunk(automation_createMIDIvolumetrack);
			file.Save_Chunk(automation_createMIDImodulationtrack);
			file.Save_Chunk(automation_createMIDIpitchbendtrack);
			file.Save_Chunk(automation_createMIDIvelocitytrack);

			file.Save_Chunk(defaultbigtimezoom);

			file.Save_Chunk(automation_createmute);
			file.Save_Chunk(automation_createsolo);

			file.Save_Chunk(automationrecordingrecord);
			file.Save_Chunk(automationrecordingplayback);
			file.Save_Chunk(askforasio);

			file.Save_Chunk(default_masterpausesamples);
			file.Save_Chunk(default_masterpausems);
			file.Save_Chunk(checkspacenonfocus);
			file.Save_Chunk(showsampleregions);
			file.Save_Chunk(showsamplevolume);
			file.Save_Chunk(default_masterchannels);
			file.Save_Chunk(splitMIDIfiletype0);
			file.Save_Chunk(setfocustrackautotorecord);

			file.CloseChunk();

			file.OpenChunk(CHUNK_SETTINGSWINDOWPOSITIONS);
			int nr=EDITORTYPE_LASTEDITOR;
			file.Save_Chunk(nr);

			for(int i=0;i<nr;i++)
				mainsettings->windowpositions[i].Save(&file);

			file.CloseChunk();

			// Save Audio Dir's
			if(Directory *adir=FirstAudioDirectory())
			{
				file.OpenChunk(CHUNK_SETTINGSAUDIODIR);
				file.Save_Chunk(GetCountOfAudioDirectories());

				while(adir)
				{
					file.Save_ChunkString(adir->dir);
					adir=adir->NextDirectory();
				}

				file.CloseChunk();
			}

			file.OpenChunk(CHUNK_SETTINGSDEFAULT);
			// Default AudioHardware

			// ASIO, Directs...
			char *defaultaudio=0,*activedevice=0;

			if(mainaudio->selectedaudiohardware)
			{
				defaultaudio=mainaudio->selectedaudiohardware->name;

				if(mainaudio->selectedaudiohardware->GetActiveDevice())
					activedevice=mainaudio->selectedaudiohardware->GetActiveDevice()->devname;
			}

			file.Save_ChunkString(defaultaudio); // Driver System, ASIO...
			file.Save_ChunkString(activedevice); // Soundcard name

			// Default MIDI

			char *defaultMIDI=0;

			if(mainMIDI->defaultMIDIOutputDevice)
				defaultMIDI=mainMIDI->defaultMIDIOutputDevice->name;

			file.Save_ChunkString(defaultMIDI);

			file.Save_Chunk(mainMIDI->sendcontrolsolomute);
			file.Save_Chunk(peakfiles);
			file.Save_Chunk(defaultnotetype);

			file.Save_Chunk(undosteps);
			file.Save_Chunk(soloeditor);

			file.Save_Chunk(showbothsformatsintimeline);

			bool exmtc=false;

			file.Save_Chunk(exmtc);
			file.Save_Chunk(exmtc);

			file.Save_Chunk(mainMIDI->receiveMIDIstart);
			file.Save_Chunk(mainMIDI->receiveMIDIstop);

			file.Save_Chunk(exmtc);
			file.Save_Chunk(exmtc);

			file.Save_Chunk(mainMIDI->quantizesongpositiontoMIDIclock);

			file.Save_Chunk(showmonitordevicename);
			file.Save_Chunk(autoloadlastusedproject);

			file.Save_Chunk(mainMIDI->sendnoteprevcylce);
			file.Save_Chunk(audioresamplingformat);

			file.Save_Chunk(playmouseover);

			file.Save_Chunk(defaultprojectmeasureformat);

			file.Save_ChunkString(audiodefaultdevice);
			file.Save_Chunk(plugincheck);

			file.Save_Chunk(showarrangemaster);
			file.Save_Chunk(showarrangebus);
			file.Save_Chunk(showarrangemetro);

			file.Save_Chunk(defaultmetroaudiochanneltype);
			file.Save_Chunk(defaultmetroaudiochannelportindex);

			file.Save_Chunk(manager_showregions);
			file.Save_Chunk(realtimerecordtempoevents);

			file.CloseChunk();

			mainMIDI->SavePorts(&file);

			file.Close(true);	

			SaveDevices();
			SavePluginSettings();
			SaveProjects();
		}
#ifdef _DEBUG
		else
			MessageBox(NULL,"Unable to Open Settings File for saving","Error",MB_OK);
#endif
	}
}

void Settings::LoadDevicePrograms()
{
	char *dirname=GetSettingsFileName(SETTINGSFILE_DEVICEPROGRAMS);

	if(dirname)
	{
		// Scan Track Images Directory...
		TRACE ("Scan Devices ...\n");

		HANDLE hdl;	
		WIN32_FIND_DATA data;

		if(char *help=mainvar->GenerateString(dirname,"*.cxd")){

			hdl=FindFirstFile(help,&data);
			delete help;
		}
		else
			return;

		if(hdl!=INVALID_HANDLE_VALUE){

			do{

				if(char *directname=mainvar->GenerateString(dirname,data.cFileName))
				{
					camxFile read;
					//read.nobuffer=true;

					if(read.OpenRead(directname)==true && read.GetLength()>=8)
					{
						TRACE ("Device found %s %s\n",directname,data.cFileName);

						if(SynthDevice *dev=new SynthDevice)
						{
							if(char *cs=mainvar->ClearString(data.cFileName))
							{
								strcpy(dev->name,cs);
								delete cs;
							}

							dev->oldfilename=mainvar->GenerateString(directname);

							synthdevices.AddDevice(dev);

							// Scan File
							int l=(int)read.GetLength();

							if(char *scanbuffer=new char[l+32])
							{
								char *end=&scanbuffer[l];
								char *sp=scanbuffer;
								char *st=scanbuffer;

								read.Read(scanbuffer,l);
								// Scan For Programs

								// CH:-;BK:-;PG:3;NM:Knobi
								// CH=1
								// BK=2
								// PG=3
								// Name=4

								bool insert=false;
								bool programfound=false;
								char inth[64];
								DeviceProgram program;

								while(st!=end)
								{
									bool found=false;

									if(*st==':')
									{
										char test[3];

										if(st==sp+2)
										{
											test[0]=*sp;
											test[1]=*(sp+1);
											test[2]=0;

											if(strcmp(test,"CH")==0)
											{
												st++;
												if(st==end)break;

												if(*st=='-')
												{
													found=true;
													program.MIDIChannel=0;
													st++;
												}
												else
												{
													sp=st;

													while(*st!=';' && *st!=0x0D)
													{
														st++;
														if(st==end)break;
													}

													if(st-sp<32)
													{
														strncpy(inth,sp,st-sp);
														inth[st-sp]=0;

														int channel=mainvar->ConvertCharToInt(inth);

														if(channel>=1 && channel<=16)
														{
															program.MIDIChannel=channel;
															found=true;
														}
													}
												}
											}
											else
												if(strcmp(test,"BK")==0)
												{
													st++;
													if(st==end)break;

													if(*st=='-')
													{
														found=true;
														program.MIDIBank=0;
														program.usebanksel=false;
														st++;

													}
													else
													{
														sp=st;

														while(*st!=';' && *st!=0x0D)
														{
															st++;
															if(st==end)break;
														}

														if(st-sp<32)
														{
															strncpy(inth,sp,st-sp);
															inth[st-sp]=0;

															int bank=mainvar->ConvertCharToInt(inth);

															if(bank>=1 && bank<=128)
															{
																program.MIDIBank=bank-1;
																program.usebanksel=true;
																found=true;
															}
														}
													}
												}
												else
													if(strcmp(test,"PG")==0)
													{
														st++;
														if(st==end)break;

														sp=st;

														while(*st!=';' && *st!=0x0D)
														{
															st++;
															if(st==end)break;
														}

														if(st-sp<32)
														{
															strncpy(inth,sp,st-sp);
															inth[st-sp]=0;

															int prog=mainvar->ConvertCharToInt(inth);

															if(prog>=1 && prog<=128)
															{
																program.MIDIProgram=prog-1;
																found=true;
															}
														}
													}
													else
														if(strcmp(test,"NM")==0)
														{
															st++;
															if(st==end)break;

															sp=st;

															while(*st!=';' && *st!=0x0D)
															{
																st++;
																if(st==end)break;
															}

															if(st-sp<31)
															{
																strncpy(program.name,sp,st-sp);
																program.name[st-sp]=0;
																found=true;
																insert=true;
															}
														}
										}
									}

									st++;

									if(*st==0x0D)
										st++;

									bool reset=false;

									if(*st==0x0A)
									{
										st++;
										sp=st; // Reset Line
										reset=true;
									}

									if(found==true)
									{
										sp=st;
										found=false;
										programfound=true;

										if(insert==true)
										{
											insert=false;
											programfound=false;

											if(DeviceProgram *newprogram=new DeviceProgram)
											{
												program.CloneTo(newprogram);
												dev->AddProgram(newprogram);
											}

											reset=true;
										}
									}

									if(reset==true)
										program.Reset();
								}

								delete scanbuffer;
							}
						}
					}

					read.Close(true);

					delete directname;
				}

			}while(FindNextFile(hdl,&data));

			FindClose(hdl);
		}

		TRACE ("Scan Devices End\n");

	}
}

void Settings::SaveDevicePrograms(SynthDevice *singledev)
{
	char *dirname=GetSettingsFileName(SETTINGSFILE_DEVICEPROGRAMS);

	if(dirname)
	{
		SynthDevice *dev=synthdevices.FirstDevice();

		while(dev)
		{
			if((!singledev) || dev==singledev)
			{
				if(dev->oldfilename)
				{
					mainvar->DeleteAFile(dev->oldfilename);
					delete dev->oldfilename;
					dev->oldfilename=0;
				}

				if(char *writename=mainvar->GenerateString(dirname,dev->name,".cxd"))
				{
					camxFile write;

					if(write.OpenSave(writename)==true)
					{
						// Write Programs
						DeviceProgram *p=dev->FirstProgram();
						char h[256];

						while(p)
						{
							char h2[NUMBERSTRINGLEN];

							strcpy(h,"CH:");

							if(p->MIDIChannel>0)
								mainvar->AddString(h,mainvar->ConvertIntToChar(p->MIDIChannel,h2));
							else
								mainvar->AddString(h,"-");

							mainvar->AddString(h,";");

							if(p->usebanksel==true)
							{
								mainvar->AddString(h,"BK:");
								mainvar->AddString(h,mainvar->ConvertIntToChar(p->MIDIBank+1,h2));
							}
							else
								mainvar->AddString(h,"BK:-");

							mainvar->AddString(h,";PG:");
							mainvar->AddString(h,mainvar->ConvertIntToChar(p->MIDIProgram+1,h2));
							mainvar->AddString(h,";NM:");
							mainvar->AddString(h,p->name);

							size_t i=strlen(h);
							h[i++]=0x0D;
							h[i++]=0x0A;

							write.Save(h,i);

							p=p->NextProgram();
						}

					}

					write.Close(true);
					delete writename;
				}

			}

			dev=dev->NextDevice();
		}
	}
}