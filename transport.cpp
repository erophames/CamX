#include "songmain.h"
#include "camxgadgets.h"
#include "camximages.h"
#include "guimenu.h"
#include "editfunctions.h"
#include "transporteditor.h"
#include "settings.h"
#include "MIDIfile.h"
#include "gui.h"
#include "object_project.h"
#include "object_song.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "MIDIhardware.h"
#include "semapores.h"
#include "globalmenus.h"
#include "stepeditor.h"
#include "languagefiles.h"
#include "audioproc.h"
#include "audiothread.h"
#include "editsettings.h"
#include "undofunctions_tempo.h"
#include "editdata.h"
#include <math.h>
#include "MIDIoutproc.h"
#include "MIDItimer.h"
#include "arrangeeditor.h"
#include "audiohardwarechannel.h"

// Menus Transport ----------------------------------------------------------------

#define TRANSMENU_NEWSONG MENU_ID_START+0
#define TRANSMENU_CLOSESONG MENU_ID_START+1

#define TRANSMENU_LOAD_MIDI MENU_ID_START+2
#define TRANSMENU_LOAD_WAVE MENU_ID_START+3

#define TRANSMENU_LOAD MENU_ID_START+4
#define TRANSMENU_SAVE MENU_ID_START+5
#define TRANSMENU_EXIT MENU_ID_START+6

#define TRANSMENU_AUDIOMANAGER MENU_ID_START+9

#define TRANSMENU_NEWPROJECT MENU_ID_START+10
#define TRANSMENU_SAVEPROJECT MENU_ID_START+11
#define TRANSMENU_CLOSEPROJECT MENU_ID_START+12
#define TRANSMENU_SELECTPROJECT MENU_ID_START+13
#define TRANSMENU_CREATELANGUAGE MENU_ID_START+14

#define TRANSMENU_SAVE_MIDI MENU_ID_START+15

//Transport Settings

#define TRANSMENU_MIDISETTINGS MENU_ID_START+20
#define TRANSMENU_AUDIOSETTINGS MENU_ID_START+21
#define TRANSMENU_SETTINGS MENU_ID_START+22

//Transport Audio
#define TRANSMENU_AUDIOMIXER MENU_ID_START+40
#define TRANSMENU_GROOVEEDITOR MENU_ID_START+41
#define TRANSMENU_AUDIOMASTER MENU_ID_START+42

// Functions
#define TRANSMENU_SEND_MIDIRESET MENU_ID_START+60
#define TRANSMENU_SEND_PANIC MENU_ID_START+61
#define TRANSMENU_DELETETEMPOCHANGES MENU_ID_START+62

#define TRANSMENU_MIDITHRU MENU_ID_START+64
#define TRANSMENU_MIDITHRU_NOTES MENU_ID_START+65
#define TRANSMENU_MIDITHRU_POLYPRESSURE MENU_ID_START+66
#define TRANSMENU_MIDITHRU_CONTROLCHANGE MENU_ID_START+67
#define TRANSMENU_MIDITHRU_PROGRAMCHANGE MENU_ID_START+68
#define TRANSMENU_MIDITHRU_CHANNELPRESSURE MENU_ID_START+69
#define TRANSMENU_MIDITHRU_PITCHBEND MENU_ID_START+70
#define TRANSMENU_MIDITHRU_SYSEX MENU_ID_START+71
#define TRANSMENU_MIDIOUTPUT MENU_ID_START+80
#define TRANSMENU_MIDIINPUT MENU_ID_START+81
#define TRANSMENU_AUDIOOUTPUT MENU_ID_START+82
#define TRANSMENU_TRIGGERRECORD MENU_ID_START+84
#define TRANSMENU_OPENTEMPOMAP MENU_ID_START+85
#define TRANSMENU_SELECT_SONG (MENU_ID_START+200)
#define TRANSMENU_SELECT_PROJECT (MENU_ID_START+300)

// Gadget Transport
enum
{
	GADGETID_STOP,
	GADGETID_PLAY,
	GADGETID_RECORD,
	GADGETID_SOLO,
	GADGETID_CYCLE,
	GADGETID_METRO,
	GADGETID_DOCK,
	GADGETID_AUTOMATIONPLAYBACK,
	GADGETID_AUTOMATIONRECORD,
	GADGETID_TEMPO,
	GADGETID_TEMPO_INFO,
	GADGETID_SONGLENGTH_INFO,
	GADGETID_SIGNATURENUMERATOR,
	GADGETID_SIGNATUREDENUMERATOR,
	GADGETID_SIGNATURE_INFO,
	GADGETID_TIMEMEASURE,
	GADGETID_TIMESMPTE,
	GADGETID_PREVMARKER,
	GADGETID_PREVMARKER_INFO,
	GADGETID_NEXTMARKER,
	GADGETID_NEXTMARKER_INFO,

	GADGETID_REW,
	GADGETID_REW_INFO,

	GADGETID_FF,
	GADGETID_FF_INFO,

	GADGETID_SPP0,
	GADGETID_SPP0_INFO,
	GADGETID_SPQ,
	GADGETID_SPQ_INFO,

	GADGETID_CYCLELEFT_INFO,
	GADGETID_CYCLERIGHT_INFO,
	GADGETID_CYCLELEFT,
	GADGETID_CYCLERIGHT,
	GADGETID_SONGLENGTH,
	GADGETID_SONGZOOM,
	GADGETID_SONGZOOM_INFO,
	GADGETID_CPU,
	GADGETID_MIDIOUT,
	GADGETID_FX,
	GADGETID_MASTERVOL,
	GADGETID_AUDIOUSAGE,
	GADGETID_MIDIININFO,
	GADGETID_MIDIOUTINFO,
	GADGETID_PROGRESS,
	GADGETID_SAMPLERATE,
	GADGETID_LATENCY,

	GADGETID_MIX,
	GADGETID_EDIT,
};

void Edit_Transport::ShowStatus()
{
	song_status=WindowSong()->status;
	song_punchrecording= WindowSong()->punchrecording;

	if(song)
		status_sync=song->MIDIsync.sync;

	if(gl_stop)
	{
		gl_stop->SetColour(COLOUR_WHITE,song && song->MIDIsync.sync!=SYNC_INTERN?COLOUR_SYNC:COLOUR_GADGETBACKGROUND);
		gl_stop->ChangeButtonImage(song && song->status==Seq_Song::STATUS_STOP?IMAGE_STOPBUTTON_ON:IMAGE_STOPBUTTON_OFF);
	}

	if(gl_start)
	{
		gl_start->SetColour(COLOUR_WHITE,song && song->MIDIsync.sync!=SYNC_INTERN?COLOUR_SYNC:COLOUR_GADGETBACKGROUND);
		gl_start->ChangeButtonImage(song && (song->status&Seq_Song::STATUS_PLAY)?IMAGE_PLAYBUTTON_ON:IMAGE_PLAYBUTTON_OFF);
	}

	if(gl_record)
	{
		gl_record->SetColour(COLOUR_WHITE,song && song->MIDIsync.sync!=SYNC_INTERN?COLOUR_SYNC:COLOUR_GADGETBACKGROUND);

		int rec_id=IMAGE_RECBUTTON_OFF;

		if(song)
		{
			if(song->status&Seq_Song::STATUS_STEPRECORD)
			{
				rec_id=IMAGE_RECSTEPBUTTON_ON;
			}
			else
			{
				if(song->status&Seq_Song::STATUS_RECORD)
				{
					if(song->punchrecording&Seq_Song::PUNCHIN)
						rec_id=IMAGE_RECBUTTON_PUNCHIN_ON;
					else
						if(song->punchrecording&Seq_Song::PUNCHOUT)
							rec_id=IMAGE_RECBUTTON_PUNCHOUT_ON;
						else
							rec_id=IMAGE_RECBUTTON_ON;
				}
				else
				{
					if(song->punchrecording&Seq_Song::PUNCHIN)
						rec_id=IMAGE_RECBUTTON_PUNCHIN_OFF;
					else
						if(song->punchrecording&Seq_Song::PUNCHOUT)
							rec_id=IMAGE_RECBUTTON_PUNCHOUT_OFF;
				}
			}
		}

		gl_record->ChangeButtonImage(rec_id);
	}
}

void CPUUsage_Callback(guiGadget_CW *g,int status)
{
	Edit_Transport *tr=(Edit_Transport *)g->from;

	switch(status)
	{
	case DB_CREATE:
		tr->cpudb=g;
		break;

	case DB_PAINT:
		{
			tr->ShowCPU(true);
		}
		break;
	}
}

void MasterPeakAudio_Callback(guiGadget_CW *g,int status)
{
	Edit_Transport *tr=(Edit_Transport *)g->from;

	switch(status)
	{
	case DB_CREATE:
		tr->masterpeak=g;
		break;

	case DB_PAINT:
		{
			tr->ShowMasterPeak(true);
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		//if(ar->overview->leftmousedown==true)
		//	ar->MouseClickInOverview(true);	
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		//	if(ar->overview->rightmousedown==true)
		//	ar->MouseClickInOverview(false);	
		break;
	}

}

void MasterAudio_Callback(guiGadget_CW *g,int status)
{
	Edit_Transport *tr=(Edit_Transport *)g->from;

	switch(status)
	{
	case DB_CREATE:
		tr->mastervolume=g;
		break;

	case DB_PAINT:
		{
			tr->ShowMasterVolume(true);
		}
		break;

	case DB_LEFTMOUSEDOWN:
		tr->MouseClickInVolume(true);	
		break;

	case DB_LEFTMOUSEUP:
		tr->MouseReleaseInVolume(true);
		break;

	case DB_RIGHTMOUSEDOWN:
		tr->MouseClickInVolume(false);	
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		tr->MouseMoveInVolume();	
		break;
	}

}

void Edit_Transport::InitGadgets()
{
	//SetSong(WindowSong());
	int maxsize=3; // 3 = all

	if(screen)
	{
		int maxscreenw=screen->maximumwidth;

		if(maxscreenw>1280)
			maxsize=3;
		else
			if(maxscreenw>1024)
				maxsize=2;
			else
				maxsize=1;
	}

	int size=bitmap.GetTextWidth("9999.WWWW");
	int bsize=bitmap.GetTextWidth("999WWW");

	song_status=WindowSong()?WindowSong()->status:0;

	glist.SelectForm(0,0);

	g_mix=glist.AddButton(-1,VTABLE_1,bsize,-1,"-Mixer-",GADGETID_MIX,MODE_TEXTCENTER,"Song Audio/MIDI Mixer F7");
	g_edit=glist.AddButton(-1,VTABLE_2,bsize,-1,"-Editor-",GADGETID_EDIT,MODE_TEXTCENTER,"Song Event/Sample Editor F8");

	glist.AddLX();

	// Time
	timegadget_measure=glist.AddTimeButton(-1,VTABLE_1,bitmap.prefertimebuttonsize,-1,0,GADGETID_TIMEMEASURE,WINDOWDISPLAY_MEASURE,MODE_BLACK|MODE_STATICTIME,Cxs[CXS_SONGPOSITIONMEASURE]);

	if(timegadget_measure)
		timegadget_measure->showprecounter=true;

	timegadget_smpte=glist.AddTimeButton(-1,VTABLE_2,bitmap.prefertimebuttonsize,-1,0,GADGETID_TIMESMPTE,WINDOWDISPLAY_SMPTE,MODE_BLACK|MODE_STATICTIME,"Song Position (Frames)");

	ShowTime(false);

	glist.AddLX();

	if(maxsize>1)
	{
		glist.AddImageButton(-1,VTABLE_1,size/2,-1,IMAGE_PREVMARKER,GADGETID_PREVMARKER,0);
		glist.AddButton(-1,VTABLE_2,size/2,-1,"Mk -",GADGETID_PREVMARKER_INFO);
		glist.AddLX();

		glist.AddImageButton(-1,VTABLE_1,size/2,-1,IMAGE_REW,GADGETID_REW,MODE_PUSH);
		glist.AddButton(-1,VTABLE_2,size/2,-1,"Time -",GADGETID_REW_INFO);
		glist.AddLX();

		glist.AddImageButton(-1,VTABLE_1,size/2,-1,IMAGE_FF,GADGETID_FF,MODE_PUSH);
		glist.AddButton(-1,VTABLE_2,size/2,-1,"Time +",GADGETID_FF_INFO);
		glist.AddLX();

		glist.AddImageButton(-1,VTABLE_1,size/2,-1,IMAGE_NEXTMARKER,GADGETID_NEXTMARKER,0);
		glist.AddButton(-1,VTABLE_2,size/2,-1,"Mk +",GADGETID_NEXTMARKER_INFO);
		glist.AddLX();

		glist.AddImageButton(-1,VTABLE_1,size/2,-1,IMAGE_SPP0,GADGETID_SPP0,0);
		glist.AddButton(-1,VTABLE_2,size/2,-1,"S0",GADGETID_SPP0_INFO);
		glist.AddLX();

		glist.AddImageButton(-1,VTABLE_1,size/2,-1,IMAGE_SPQN,GADGETID_SPQ,0,Cxs[CXS_MOVESONGPOSITIONNEXTMEASURE]);
		glist.AddButton(-1,VTABLE_2,size/2,-1,"<SQ>",GADGETID_SPQ_INFO,0,Cxs[CXS_MOVESONGPOSITIONNEXTMEASURE]);
		glist.AddLX();
	}

	int sizectrl=height/2+2;

	double startstopw=sizectrl;
	startstopw*=1.5;

	// Stop Button
	gl_stop=glist.AddImageButton(-1,VTABLE_MIDCENTER,(int)startstopw,sizectrl,(WindowSong() && (WindowSong()->status==Seq_Song::STATUS_STOP))? IMAGE_STOPBUTTON_ON:IMAGE_STOPBUTTON_OFF,GADGETID_STOP,0,Cxs[CXS_STOPSONGORCYL]);

	if(gl_stop && (!WindowSong()))
		gl_stop->Disable();

	glist.AddLX();

	gl_start=glist.AddImageButton(0,VTABLE_MIDCENTER,(int)startstopw,sizectrl,(WindowSong() && (WindowSong()->status&Seq_Song::STATUS_PLAY))?IMAGE_PLAYBUTTON_ON : IMAGE_PLAYBUTTON_OFF,GADGETID_PLAY,0,Cxs[CXS_STARTSONGPLAYBACK]);
	if(gl_start && (!WindowSong()))
		gl_start->Disable();

	glist.AddLX();

	int rec_id=IMAGE_RECBUTTON_OFF;

	if(WindowSong())
	{
		if(WindowSong()->status&Seq_Song::STATUS_STEPRECORD)
			rec_id=IMAGE_RECSTEPBUTTON_ON;

		if(WindowSong()->status&Seq_Song::STATUS_RECORD)
			rec_id=IMAGE_RECBUTTON_ON;
	}

	gl_record=glist.AddImageButton(0,VTABLE_MIDCENTER,(int)startstopw,sizectrl,rec_id,GADGETID_RECORD,0,Cxs[CXS_STARTSONGREC]);
	if(!WindowSong())
		glist.Disable(gl_record);

	glist.AddLX();

	gl_solo=glist.AddImageButton(0,VTABLE_1,2*CTRL_SMALLXY,CTRL_SMALLXY,(WindowSong() && WindowSong()->playbacksettings.solo)?IMAGE_SOLOBUTTON_ON:IMAGE_SOLOBUTTON_OFF,GADGETID_SOLO,0,Cxs[CXS_SOLOATONOFF]);
	if(!WindowSong())
		glist.Disable(gl_solo);

	if(WindowSong())
		sl_solo=song->playbacksettings.solo;

	gl_cycle=glist.AddImageButton(0,VTABLE_2,2*CTRL_SMALLXY,CTRL_SMALLXY,(WindowSong() && (WindowSong()->playbacksettings.cycleplayback==true))?IMAGE_CYCLEBUTTON_ON:IMAGE_CYCLEBUTTON_OFF,GADGETID_CYCLE,0,Cxs[CXS_CYLCEONOFF]);
	glist.AddLX();

	if(WindowSong())
		sl_cycle=WindowSong()->playbacksettings.cycleplayback;

	if(!WindowSong())
		glist.Disable(gl_cycle);

	gl_metro=glist.AddButton(0,VTABLE_1,2*CTRL_SMALLXY,CTRL_SMALLXY,"Click",GADGETID_METRO,(WindowSong() && WindowSong()->metronome.on==true)?MODE_TEXTCENTER|MODE_TOGGLE|MODE_TOGGLED:MODE_TEXTCENTER|MODE_TOGGLE,Cxs[CXS_METRONOME]);
	if(!WindowSong())
		glist.Disable(gl_metro);

	if(WindowSong())
		sl_metro=WindowSong()->metronome.on;

	if(WindowSong())
		sl_solo=song->playbacksettings.solo;

	gl_dock=glist.AddButton(0,VTABLE_2,2*CTRL_SMALLXY,CTRL_SMALLXY,"Dock",GADGETID_DOCK,(WindowSong() && WindowSong()->metronome.on==true)?MODE_TEXTCENTER|MODE_TOGGLE|MODE_TOGGLED:MODE_TEXTCENTER|MODE_TOGGLE,Cxs[CXS_DOCK]);
	glist.AddLX();

	// Automation
	gl_autoplayback=glist.AddButton(0,VTABLE_1,2*CTRL_SMALLXY,CTRL_SMALLXY,"Auto",GADGETID_AUTOMATIONPLAYBACK,(WindowSong() && WindowSong()->playbacksettings.automationplayback==true)?MODE_TEXTCENTER|MODE_TOGGLE|MODE_TOGGLED:MODE_TEXTCENTER|MODE_TOGGLE,Cxs[CXS_AUTOMATIONPLAYBACK]);
	if(!WindowSong())
		glist.Disable(gl_autoplayback);

	if(WindowSong())
		sl_automationplayback=WindowSong()->playbacksettings.automationplayback;

	if(WindowSong())
		sl_automationrecording=WindowSong()->playbacksettings.automationrecording;

	gl_autorecording=glist.AddButton(0,VTABLE_2,2*CTRL_SMALLXY,CTRL_SMALLXY,"ARec",GADGETID_AUTOMATIONRECORD,(WindowSong() && WindowSong()->playbacksettings.automationrecording==true)?MODE_TEXTCENTER|MODE_TOGGLE|MODE_TOGGLED:MODE_TEXTCENTER|MODE_TOGGLE,Cxs[CXS_AUTOMATIONRECORDING]);
	if(!WindowSong())
		glist.Disable(gl_autorecording);

	glist.AddLX();

	glist.AddButton(-1,VTABLE_1,12,-1,"L",GADGETID_CYCLELEFT_INFO,0,Cxs[CXS_CYCLESTART]);
	glist.AddButton(-1,VTABLE_2,12,-1,"R",GADGETID_CYCLERIGHT_INFO,0,Cxs[CXS_CYCLEEND]);
	glist.AddLX();

	cycle_left=glist.AddTimeButton(0,VTABLE_1,bitmap.prefertimebuttonsize,-1,0,GADGETID_CYCLELEFT,WINDOWDISPLAY_MEASURE,0,Cxs[CXS_CYCLESTART]);
	cycle_right=glist.AddTimeButton(0,VTABLE_2,bitmap.prefertimebuttonsize,-1,0,GADGETID_CYCLERIGHT,WINDOWDISPLAY_MEASURE,0,Cxs[CXS_CYCLEEND]);

	if(cycle_right)
		cycle_right->BuildPair(cycle_left);

	ShowCycle(false);

	glist.AddLX();

	g_samplerate=glist.AddButton(-1,VTABLE_1,bsize,-1,GADGETID_SAMPLERATE,0,Cxs[CXS_SAMPLINGRATE]);

	{
		char *h=mainvar->GenerateString("Project/Song Audio ",Cxs[CXS_LATENCY]);
		g_latency=glist.AddButton(-1,VTABLE_2,bsize,-1,GADGETID_LATENCY,0,h);

		if(h)
			delete h;
	}

	glist.AddLX();

	ShowSampleRate(true);
	ShowLatency(true);

	tempogadget=glist.AddNumberButton(-1,VTABLE_1,bsize,-1,GADGETID_TEMPO,MINIMUM_TEMPO,MAXIMUM_TEMPO,0,NUMBER_000,0,Cxs[CXS_SONGTEMPO]);
	if(tempogadget)
	{
		//if(guiGadget *g=

		glist.AddButton(-1,VTABLE_2,bsize,-1,"Tempo",GADGETID_TEMPO_INFO,MODE_TEXTCENTER,"Tempo Map Editor");

		{
			//	g->linkgadget=tempogadget;
			//	tempogadget->linkclickgadget=g;
		}
	}

	// Signature
	glist.AddLX();
	signgadget=glist.AddSignatureButton(-1,VTABLE_1,bsize,-1,GADGETID_SIGNATURENUMERATOR,GADGETID_SIGNATUREDENUMERATOR,0);
	glist.AddButton(-1,VTABLE_2,bsize,-1,Cxs[CXS_SIGNATURE],GADGETID_SIGNATURE_INFO,MODE_TEXTCENTER,Cxs[CXS_SONGSIGNATURE]);
	glist.AddLX();

	songlengthgadget=glist.AddNumberButton(-1,VTABLE_1,bsize,-1,GADGETID_SONGLENGTH,50,12000,0,NUMBER_INTEGER,0,Cxs[CXS_SONGLENGTHMEASURE]);
	if(songlengthgadget)
	{
		if(guiGadget *g=glist.AddButton(-1,VTABLE_2,bsize,-1,Cxs[CXS_LENGTH],GADGETID_SONGLENGTH_INFO,MODE_TEXTCENTER,Cxs[CXS_SONGLENGTHMEASURE]))
		{
			g->linkgadget=songlengthgadget;
			songlengthgadget->linkclickgadget=g;
		}
	}

	glist.AddLX();

	songzoomgadget=glist.AddButton(-1,VTABLE_1,bsize,-1,GADGETID_SONGZOOM,MODE_TEXTCENTER|MODE_MENU,Cxs[CXS_DEFAULTSONGGRID]);
	glist.AddButton(-1,VTABLE_2,bsize,-1,Cxs[CXS_SONGGRID],GADGETID_SONGZOOM_INFO,MODE_TEXTCENTER,Cxs[CXS_DEFAULTSONGGRID]);
	glist.AddLX();

	showtempo=0;
	sign_dn=-1;
	sign_nn=-1;

	// SongLength Gadget
	//	songlengthgadget=glist.AddButton(-1,VTABLE_1,60,-1,GADGETID_SONGLENGTH,MODE_TEXTCENTER,Cxs[CXS_SONGLENGTHMEASURE]);


	ShowSongLength();
	ShowSongZoom();

	// MIDIIn
	MIDIininfo=glist.AddText(0,VTABLE_1,10*maingui->GetFontSizeY(),-1,lastMIDIinstring,GADGETID_MIDIININFO,0,Cxs[CXS_MIDIEVENTINPUTDISPLAY]);
	// MIDIOut
	MIDIoutinfo=glist.AddText(0,VTABLE_2,10*maingui->GetFontSizeY(),-1,lastMIDIoutstring,GADGETID_MIDIOUTINFO,0,Cxs[CXS_MIDIEVENTOUTPUTDISPLAY]);
	glist.AddLX();

	progress=glist.AddText(0,VTABLE_1,8*maingui->GetFontSizeY(),-1,progressstring,GADGETID_PROGRESS,0,Cxs[CXS_BACKGROUNDPROGRESSDISPLAY]);
	audiousage=glist.AddButton(0,VTABLE_2,6*maingui->GetFontSizeY(),-1,GADGETID_CPU,0,Cxs[CXS_CPUUSAGE]);
	glist.AddLX();
	glist.AddChildWindow(-1,VTABLE_2,2*maingui->GetFontSizeY(),-1,0,0,&CPUUsage_Callback,this);

	glist.AddLX();

	int dw=bitmap.GetTextWidth(Cxs[CXS_RECORDSETTINGS]);

	/*
	if(maxsize>2)
	{
	glist.AddButton(0,VTABLE_1,dw,-1,Cxs[CXS_RECORDSETTINGS],GADGETID_RECORDSETTINGS,0,Cxs[CXS_RECORDSETTINGS]);
	syncgadget=glist.AddButton(0,VTABLE_2,dw,-1,"Sync",GADGETID_SYNC,MODE_MENU,Cxs[CXS_SONGSYNCSETTINGS]);
	ShowSync();
	glist.AddLX();
	}
	else
	*/
	//	syncgadget=0;

	glist.AddChildWindow(-1,VTABLE_1,-1,-1,MODE_RIGHT,0,&MasterPeakAudio_Callback,this);

	masterfx=glist.AddButton(-1,VTABLE_2,3*maingui->GetFontSizeY(),-1,GADGETID_FX,MODE_TEXTCENTER,"Song Audio Effects A/B");
	glist.AddLX();

	mastervolume_db=glist.AddVolumeButtonText(-1,VTABLE_2,3*maingui->GetFontSizeY(),-1,"M ",GADGETID_MASTERVOL,0.5,0);
	glist.AddLX();

	glist.AddChildWindow(-1,VTABLE_2,-1,-1,MODE_RIGHT,0,&MasterAudio_Callback,this);
}

void Edit_Transport::ShowTime(bool realtime)
{
	if(WindowSong())
	{
		if(WindowSong()->status&Seq_Song::STATUS_WAITPREMETRO)
		{
			if(timegadget_measure)
				timegadget_measure->CheckPreCounter();
		}
		else
		{
			OSTART sp=WindowSong()->GetSongPosition();

			if(timegadget_measure)
				timegadget_measure->CheckTime(sp,true);

			if(timegadget_smpte)
				timegadget_smpte->CheckTime(sp,true);
		}
	}
	else
	{
		glist.Disable(timegadget_smpte);
		glist.Disable(timegadget_measure);
	}
}

void Edit_Transport::ShowCycle(bool realtime)
{
	if(song)
	{
		if(cycle_left)
			cycle_left->CheckTime(song->playbacksettings.cyclestart,realtime);

		if(cycle_right)
			cycle_right->CheckTime(song->playbacksettings.cycleend,realtime);
	}
	else
	{
		glist.Disable(cycle_left);
		glist.Disable(cycle_right);
	}
}

Edit_Transport::Edit_Transport()
{
	editorid=EDITORTYPE_TRANSPORT;

	InitForms(FORM_PLAIN1x1);

	editorname="Transport";

	mainvolumemousemove=false;

	switchusagecounter=0;
	showwhat=0;

	lastmonitor_outevent=lastmonitor_inputevent=0;

	impulseset=false;
	ResetMenu();
	smpteflag=mainsettings->projectstandardsmpte;
	//selectedaudiohardware=0;
	samplerate=0;
	initdevices=false;

	showaudiomaster=true;
	maxpeaksum=-1;
	showaudioclear=false;

	strcpy(lastMIDIoutstring,Cxs[CXS_NOMIDIOUTPUT]);
	strcpy(lastMIDIinstring,Cxs[CXS_NOMIDIINPUT]);

	progressstring=0;
	lastdevicesmicrosec=0;
	lastfilems=0;
	resizeable=true;

	masterpeak=mastervolume=0;

	trans_editorcount=0;
	trans_editindex=0;

	trans_mixercount=0;
	trans_mixerindex=0;
}

void Edit_Transport::ShowProgress(char *s)
{
	if(progress)
	{
		if(char *ps=mainvar->GenerateString(s))
		{
			if(progressstring)
				delete progressstring;

			progress->ChangeButtonText(progressstring=ps);
		}
	}
}

void  Edit_Transport::ShowSignature(bool realtime)
{
	if(song)
	{
		// Tempo
		Seq_Signature *sig=song->timetrack.FindSignatureBefore(song->GetSongPosition());

		if(realtime==false || sig->dn!=sign_dn || sig->nn!=sign_nn)
		{
			if(signgadget)
				signgadget->ShowSignature(sig);
		}

		sign_dn=sig->dn;
		sign_nn=sig->nn;
	}
	else
	{
		if(signgadget)
		{
			signgadget->ChangeButtonText("-");
			signgadget->Disable();
		}
	}
}

void Edit_Transport::ShowTempo(bool realtime)
{
	if(song)
	{
		// Tempo
		Seq_Tempo *tempo=song->timetrack.GetTempo(song->GetSongPosition());

		if(realtime==false || tempo->tempo!=showtempo)
		{
			showtempo=tempo->tempo;

			if(tempogadget)
			{
				tempogadget->SetPos(showtempo);
			}
		}
	}
	else
	{
		glist.Disable(tempogadget);

	}
}

void Edit_Transport::DeInitWindow()
{
	//	indevices.DeleteAllO();
	//	outdevices.DeleteAllO();

	if(progressstring)
		delete progressstring;

	lastin.FreeData();
	lastout.FreeData();
}

void Edit_Transport::InitDevices()
{
	/*
	// MIDIOut
	MIDIOutputDevice *o=mainMIDI->FirstMIDIOutputDevice();

	while(o)
	{
	if(Edit_MonitorOutDevice *emd=new Edit_MonitorOutDevice)
	{		
	emd->outdevice=o;
	emd->counter=o->monitor_eventcounter;
	outdevices.AddEndO(emd);
	}

	o=o->NextOutputDevice();
	}

	MIDIInputDevice *i=mainMIDI->FirstMIDIInputDevice();

	while(i)
	{
	if(Edit_MonitorInDevice *emd=new Edit_MonitorInDevice)
	{		
	emd->indevice=i;
	emd->counter=i->monitor_eventcounter;
	indevices.AddEndO(emd);
	}

	i=i->NextInputDevice();
	}
	*/

	initdevices=true;
}

void Edit_Transport::ShowMasterAB(bool realtime)
{
	if(!masterfx)
		return;

	if(WindowSong())
	{
		masterfx->Enable();

		if(realtime==false || masterabstatus!=WindowSong()->audiosystem.systembypassfx)
		{
			masterabstatus=WindowSong()->audiosystem.systembypassfx;

			switch(masterabstatus)
			{
			case true:
				masterfx->SetColourNoDraw(COLOUR_WHITE,COLOUR_RED);
				masterfx->ChangeButtonText("-FX");
				break;

			case false:
				masterfx->SetColourNoDraw(COLOUR_WHITE,COLOUR_BLACK);
				masterfx->ChangeButtonText("+FX");
				break;
			}
		}
	}
	else
		masterfx->Disable();
}

void Edit_Transport::Init()
{
	if(initdevices==false)
		InitDevices();

	InitGadgets();

	ShowTempo(false);
	ShowSignature(false);
	ShowTime(false);
	ShowCycle(false);
	ShowMasterAB(false);
}

void Edit_Transport::ShowLastMIDIIO(Seq_Song *song)
{
	if(song)
	{
		// Out
		if(LMIDIEvents *levent=song->events_out.LastEvent())
		{
			if(lastout.Compare(levent)==false){
				levent->Clone(&lastout);

				if(MIDIoutinfo)
				{
					lastMIDIoutstring[0]='O';
					lastMIDIoutstring[1]=':';

					maingui->ConvertMIDI2String(&lastMIDIoutstring[2],
						lastout.data[0],
						lastout.data[1],
						lastout.data[2]);

					MIDIoutinfo->ChangeButtonText(lastMIDIoutstring);
				}
			}
		}

		// IN
		if(LMIDIEvents *levent=song->events_in.LastEvent())
		{
			if(lastin.Compare(levent)==false){
				levent->Clone(&lastin);

				if(MIDIininfo)
				{
					lastMIDIinstring[0]='I';
					lastMIDIinstring[1]=':';

					maingui->ConvertMIDI2String(&lastMIDIinstring[2],
						lastin.data[0],
						lastin.data[1],
						lastin.data[2]);

					MIDIininfo->ChangeButtonText(lastMIDIinstring);
				}
			}
		}
	}
}

/*
void Edit_Transport::ShowSync()
{
waitforMIDIplayback=mainsettings->waitforMIDIplayback;

if(syncgadget)
syncgadget->ChangeButtonText(waitforMIDIplayback==true?"Sync [Trigger Start]":"Sync");
}
*/

void Edit_Transport::ShowCPU(bool force)
{
	if(!cpudb)
		return;

	guiBitmap *bitmap=&cpudb->gbitmap;

	bitmap->guiFillRectX0(0,bitmap->GetX2(),bitmap->GetY2()/2,COLOUR_BLACK);
	bitmap->guiFillRectX0(bitmap->GetY2()/2,bitmap->GetX2(),bitmap->GetY2(),COLOUR_BLUE);

	if(!mainaudio->GetActiveDevice())
		return;

	{
		AudioDevice *device=mainaudio->GetActiveDevice();

		device->LockTimerCheck_Output();
		LONGLONG timeforrefill_systime=device->timeforrefill_systime;
		LONGLONG timeforrefill_maxsystime=device->timeforrefill_maxsystime;
		device->UnlockTimerCheck_Output();

		device->LockTimerCheck_Input();
		LONGLONG timeforaudioinpurefill_systime=device->timeforaudioinpurefill_systime;
		LONGLONG timeforaudioinputrefill_maxsystime=device->timeforaudioinputrefill_maxsystime;
		device->UnlockTimerCheck_Input();

		// Current
		double c1=mainaudio->GetActiveDevice()->samplebufferms,c2=maintimer->ConvertSysTimeToMs(timeforrefill_systime+timeforaudioinpurefill_systime);

		c2/=c1;

		double h=bitmap->GetX2();

		if(c2>1)
			bitmap->guiFillRectX0(0,bitmap->GetX2(),bitmap->GetY2()/2,COLOUR_RED);
		else
		{
			h*=c2;
			bitmap->guiFillRectX0(0,h,bitmap->GetY2()/2,c2>=0.75?COLOUR_ORANGE:COLOUR_GREEN);
		}

		// Max
		c1=device->samplebufferms; // ms
		c2=maintimer->ConvertSysTimeToMs(timeforrefill_maxsystime+timeforaudioinputrefill_maxsystime);

		c2/=c1;

		h=bitmap->GetX2();

		if(c2>1)
		{
			bitmap->guiFillRectX0(bitmap->GetY2()/2+1,h,bitmap->GetY2(),COLOUR_RED);
		}
		else
		{
			h*=c2;
			bitmap->guiFillRectX0(bitmap->GetY2()/2+1,h,bitmap->GetY2(),c2>=0.75?COLOUR_ORANGE:COLOUR_GREEN);
		}

		/*
		if(cpuusage)
		{
		char *ph=mainvar->GenerateString(x," %");
		if(ph)
		{
		cpuusage->ChangeButtonText(ph);
		delete ph;		
		}
		}
		*/

		/*
		if(cpuusagemax)
		{
		char *ph=mainvar->GenerateString(x," %");
		if(ph)
		{
		cpuusagemax->ChangeButtonText(ph);
		delete ph;		
		}
		}
		*/
	}

	cpudb->Blt();
}

void Edit_Transport::ShowMasterPeak(bool force)
{
	if(!masterpeak)
		return;

	guiBitmap *bitmap=&masterpeak->gbitmap;

	if(WindowSong())
	{
		peak.channel=&song->audiosystem.masterchannel;
		peak.track=0;
		peak.bitmap=bitmap;
		peak.x=0;
		peak.y=0;
		peak.x2=masterpeak->GetX2()-6*maingui->GetFontSizeY();
		peak.y2=masterpeak->GetY2();
		peak.force=force;
		peak.horiz=true;

		peak.maxpeakx=peak.x2+1;
		peak.maxpeaky=0;
		peak.maxpeakx2=masterpeak->GetX2();
		peak.maxpeaky2=masterpeak->GetY2();
		peak.maxpeak=true;

		peak.ShowInit(true);
		peak.ShowPeakSum();

		if(peak.changed==true)
		{
			masterpeak->Blt();

			//editor->slider->Blt(maxpeakx,maxpeaky,maxpeakx2,maxpeaky2);
			//editor->slider->Blt(vux,vuy,vux2,vuy2);
		}
	}
	else
		if(force==true)
			bitmap->guiFillRect(COLOUR_BLACK);
}

void Edit_Transport::MouseClickInVolume(bool leftmouse)
{
	if(leftmouse==true)
	{
		mainvolumemousemove=true;
		ShowMasterVolume(true);
		mastervolume->Blt();
	}
	else
	{
		if(WindowSong())
		{
			double h=WindowSong()->audiosystem.masterchannel.io.audioeffects.volume.GetValue();

			if(h!=1)
			{
				OSTART atime=GetAutomationTime();
				WindowSong()->audiosystem.masterchannel.io.audioeffects.volume.AutomationEdit(WindowSong(),atime,0,0.5);
			}
		}
	}
}

void Edit_Transport::MouseReleaseInVolume(bool leftmouse)
{
	if(leftmouse==true && mainvolumemousemove==true)
	{
		mainvolumemousemove=false;
		ShowMasterVolume(true);
		mastervolume->Blt();
	}
}

void Edit_Transport::MouseMoveInVolume()
{
	if(WindowSong() && mainvolumemousemove==true)
	{
		int mx=mastervolume->GetMouseX();

		if(mx==mmx)
			return;

		double p=WindowSong()->audiosystem.masterchannel.io.audioeffects.volume.GetParm(0);
		p*=LOGVOLUME_SIZE;

		if(mx<mmx)
			p-=1;
		else
			p+=1;

		mmx=mx;

		if(p<0)p=0;
		else
			if(p>LOGVOLUME_SIZE)
				p=LOGVOLUME_SIZE;

		OSTART atime=GetAutomationTime();

		double h2=p;
		h2/=LOGVOLUME_SIZE;

		WindowSong()->audiosystem.masterchannel.io.audioeffects.volume.AutomationEdit(WindowSong(),atime,0,h2);
	}
}

void Edit_Transport::ShowMasterVolume(bool force)
{
	if(!mastervolume)
		return;

	guiBitmap *bitmap=&mastervolume->gbitmap;

	if(song)
	{
		int sliderx2=mastervolume->GetWidth();

		double h=song->audiosystem.masterchannel.io.audioeffects.volume.GetValue();

		if(force==true || dbvalue!=h)
		{
			dbvalue=h;

			int pos=LOGVOLUME_SIZE-mainaudio->ConvertLogArrayVolumeToInt(1);
			double hw=sliderx2-1;
			double ph=LOGVOLUME_SIZE;
			double hh=pos;

			hh/=ph;
			hw*=hh;

			//int tx2=(sliderx2-1)-hw;

			int tx2=(sliderx2-1)-hw;

			bitmap->guiFillRectX0(0,tx2,mastervolume->GetHeight(),COLOUR_BLACK);
			bitmap->guiFillRect(tx2,0,sliderx2-1,mastervolume->GetHeight(),COLOUR_BLACK_LIGHT);

			pos=LOGVOLUME_SIZE-mainaudio->ConvertLogArrayVolumeToInt(dbvalue);

			hw=sliderx2-1;
			ph=LOGVOLUME_SIZE;
			hh=pos;

			hh/=ph;
			hw*=hh;

			tx2=(sliderx2-1)-hw;
			for(int x=0;x<tx2;x+=2)
				bitmap->guiDrawLineX(x,1,mastervolume->GetHeight()-2,mainvolumemousemove==true?COLOUR_WHITE:COLOUR_BLUE_LIGHT);

			/*
			// Audio
			bitmap->SetAudioColour(dbvalue);

			bitmap->guiFillRect(sliderx2,0,mastervolume->GetWidth(),mastervolume->GetHeight(),COLOUR_GREY_DARK);

			if(char *h=mainaudio->ScaleAndGenerateDBString(dbvalue,false))
			{
			bitmap->guiDrawTextCenter(sliderx2,0,mastervolume->GetWidth(),mastervolume->GetHeight(),h);
			delete h;
			}
			*/

			if(force==false)
				mastervolume->Blt();

			if(mastervolume_db)
			{
				mastervolume_db->SetPos(dbvalue);
				mastervolume_db->Enable();

			}
		}
	}
	else
		if(force==true)
		{
			bitmap->guiFillRect(COLOUR_BLACK);

			if(mastervolume_db)
				mastervolume_db->Disable();
		}
}

void Edit_Transport::ShowSampleRate(bool force)
{
	int nsamplerate=WindowSong()?WindowSong()->project->projectsamplerate:mainaudio->GetGlobalSampleRate();

	if(force==true || nsamplerate!=samplerate)
	{
		samplerate=nsamplerate;

		if(g_samplerate)
		{
			char str[NUMBERSTRINGLEN];

			char *h=mainvar->GenerateString(mainvar->ConvertDoubleToChar((double)samplerate/1000,str,1)," kHz");

			if(h)
			{
				g_samplerate->ChangeButtonText(h);
				delete h;
			}
		}
	}
}

void Edit_Transport::ShowLatency(bool force)
{
	double nlatency=mainaudio->GetActiveDevice()?mainaudio->GetActiveDevice()->samplebufferms:0;

	if(force==true || latency!=nlatency)
	{
		latency=nlatency;

		if(g_latency)
		{
			char str[NUMBERSTRINGLEN];

			if(mainaudio->GetActiveDevice())
			{
				char *h=mainvar->GenerateString(mainvar->ConvertDoubleToChar(latency,str,1)," ms");

				if(h)
				{
					g_latency->ChangeButtonText(h);
					delete h;
				}
			}
			else
				g_latency->ChangeButtonText(Cxs[CXS_NOAUDIO]);
		}
	}
}

/*
void Edit_Transport::ShowMetro()
{
if(gl_metro)
{
if(song)
{
}
else
gl_metro->Disable();
}
}
*/

void Edit_Transport::ShowSongZoom()
{
	if(songzoomgadget)
	{
		if(song)
		{
			switch(song->timetrack.zoomticks)
			{
			case TICK16nd:
				songzoomgadget->ChangeButtonText("/16");
				break;

			case TICK32nd:
				songzoomgadget->ChangeButtonText("/32");
				break;

			case TICK64nd:
				songzoomgadget->ChangeButtonText("/64");
				break;
			}
		}
		else
		{
			songzoomgadget->ChangeButtonText("-");
			songzoomgadget->Disable();
		}
	}

}

void Edit_Transport::ShowSongLength()
{
	if(songlengthgadget)
	{
		if(song)
		{
			songlengthgadget->Enable();
			sl_measure=song->GetSongLength_Measure();
			songlengthgadget->SetPos(sl_measure);
		}
		else
		{
			songlengthgadget->Disable();
		}
	}
}

void Edit_Transport::RefreshSMPTE()
{
	ShowTime(false);
}

void Edit_Transport::RefreshMeasure()
{
	ShowTime(false);
}

void Edit_Transport::RefreshRealtime_Slow()
{
#ifdef DEBUG
	maingui->CheckSongBuffer(WindowSong());
#endif

	int mixercounter=0;
	int mixerix=0;

	int editorcount=0;
	int editix=0;

	guiWindow *w=maingui->FirstWindow();
	while(w)
	{
		if(w!=this && w->screen==screen && w->parentformchild)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_AUDIOMIXER:
				if(w->hide==false)
					mixerix=mixercounter;

				mixercounter++;
				break;

			case EDITORTYPE_PIANO:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_SCORE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_TEMPO:
			case EDITORTYPE_SAMPLE:
				if(w->hide==false)
					editix=editorcount;

				editorcount++;
				break;
			}
		}

		w=w->NextWindow();
	}

	if(g_mix && (mixercounter!=trans_mixercount || mixerix!=trans_mixerindex))
	{
		trans_mixercount=mixercounter;
		trans_mixerindex=mixerix;

		if(mixercounter==0)
			g_mix->ChangeButtonText("-Mixer-");
		else
			if(mixercounter<=1)
				g_mix->ChangeButtonText("Mixer");
			else
			{
				char help[NUMBERSTRINGLEN],help2[NUMBERSTRINGLEN];

				char *h=mainvar->GenerateString("Mix ","(",mainvar->ConvertIntToChar(mixerix+1,help2),"/",mainvar->ConvertIntToChar(mixercounter,help),")");
				if(h)
				{
					g_mix->ChangeButtonText(h);
					delete h;
				}
			}
	}

	if(g_edit && (editorcount!=trans_editorcount || editix!=trans_editindex))
	{
		trans_editorcount=editorcount;
		trans_editindex=editix;

		if(editorcount==0)
			g_edit->ChangeButtonText("-Editor-");
		else
			if(editorcount<=1)
				g_edit->ChangeButtonText("Editor");
			else
			{
				char help[NUMBERSTRINGLEN],help2[NUMBERSTRINGLEN];

				char *h=mainvar->GenerateString("Ed ","(",mainvar->ConvertIntToChar(editix+1,help2),"/",mainvar->ConvertIntToChar(editorcount,help),")");
				if(h)
				{
					g_edit->ChangeButtonText(h);
					delete h;
				}
			}
	}

	ShowSampleRate(false);
	ShowLatency(false);
	ShowCPU(true);
	ShowMasterAB(true);

	if(audiousage && mainaudio->GetActiveDevice())
	{
		bool error=false;
		bool force=false;

		if(++switchusagecounter==30)
		{
			showwhat++;

			if(showwhat==2 && (WindowSong()==0 || mainaudio->GetActiveDevice()==0 || mainaudio->GetActiveDevice()->FirstInputChannel()==0) )
				showwhat=0;

			if(showwhat==3)showwhat=0; // 0 == Audio Use,1==CPU Use,2==Audio Aufnahme Left Time

			force=true;
			switchusagecounter=0;
		}

		audiousage->SetColourNoDraw(COLOUR_GADGETTEXT,COLOUR_GADGETBACKGROUNDSYSTEM);

		if(showwhat==0) // Audio
		{
			bool outofsync=mainaudio->GetActiveDevice()->deviceoutofsync;

			if(WindowSong() && WindowSong()->cycleoutofsync==true)
				outofsync=true;

			if(mainaudio->GetActiveDevice()->recorderror && (!(mainaudio->GetActiveDevice()->recorderror&AudioDevice::RECORDERROR_MESSAGE)) )
			{
				mainaudio->GetActiveDevice()->recorderror |=AudioDevice::RECORDERROR_MESSAGE;

				char *h=mainvar->GenerateString(Cxs[CXS_AUDIORECORDINGERROR],"\n","Device:",mainaudio->GetActiveDevice()->devname);
				if(h)
				{
					if(mainaudio->GetActiveDevice()->recorderror & AudioDevice::RECORDERROR_BUFFEROVERFLOW)
					{
						char *h2=mainvar->GenerateString(h,"\n",Cxs[CXS_AUDIORECORDINGERROR_BUFFEROVERFLOW]);
						if(h2)
						{
							delete h;
							h=h2;
						}
					}

					maingui->MessageBoxError(0,h);
					delete h;
				}
			}

			if(outofsync==true)
			{

				/*
				#ifdef DEBUG
				if(mainaudio->GetActiveDevice())
				TRACE ("Out Of Sync Device %d\n",mainaudio->GetActiveDevice()->deviceoutofsync);

				if(WindowSong())
				TRACE ("Out Of Sync Cycle Sync %d\n",WindowSong()->cycleoutofsync);
				#endif
				*/

				audiousage->ChangeButtonText("Error:Audio Sync");
				error=true;
			}
			else
			{
				mainaudio->GetActiveDevice()->LockTimerCheck_Output();
				LONGLONG h=mainaudio->GetActiveDevice()->timeforrefill_systime;
				LONGLONG timeforrefill_maxsystime=mainaudio->GetActiveDevice()->timeforrefill_maxsystime;
				mainaudio->GetActiveDevice()->UnlockTimerCheck_Output();

				if(h!=lastdevicesmicrosec || force==true)
				{
					lastdevicesmicrosec=h;

					// Current
					double c1=mainaudio->GetActiveDevice()->samplebufferms, // ms
						c2=maintimer->ConvertSysTimeToMs(h);

					c2/=c1;
					double dh=c2*100;
					char h2[NUMBERSTRINGLEN],*x=mainvar->ConvertDoubleToChar(dh,h2,1);

					// Max
					c1=mainaudio->GetActiveDevice()->samplebufferms; // ms
					c2=maintimer->ConvertSysTimeToMs(timeforrefill_maxsystime);
					c2/=c1;

					dh=c2*100;

					if(dh>100)
						error=true;

					char h3[NUMBERSTRINGLEN],*x2=mainvar->ConvertDoubleToChar(dh,h3,1),
						*ph=mainvar->GenerateString(x,"%CPU [",x2,"]");

					if(ph){
						audiousage->ChangeButtonText(ph);
						delete ph;
					}
				}
			}
		}
		else // HD
			if(showwhat==1)
			{
				mainaudiostreamproc->LockTimerCheck();
				double h=mainaudiostreamproc->timeforrefill_ms;
				double timeforrefill_maxms=mainaudiostreamproc->timeforrefill_maxms;
				mainaudiostreamproc->UnlockTimerCheck();

				if(h!=lastfilems || force==true)
				{
					lastfilems=h;

					double c1=rafbuffersize[mainaudio->rafbuffersize_index], // ms
						c2=h;

					c2/=c1;
					double dh=c2*100;

					char h2[NUMBERSTRINGLEN],*x=mainvar->ConvertDoubleToChar(dh,h2,1);

					// Max
					c2=timeforrefill_maxms;

					c2/=c1;

					dh=c2*100;

					char h3[NUMBERSTRINGLEN],*x2=mainvar->ConvertDoubleToChar(dh,h3,1),*ph=mainvar->GenerateString(x,"%HD [",x2,"]");

					if(ph){
						audiousage->ChangeButtonText(ph);
						delete ph;
					}
				}
			}
			else
				if(showwhat==2 && song)
				{
					unsigned __int64 audiotrackschannels=0;
					Seq_Track *t=song->FirstTrack();
					while (t)
					{
						if(t->recordtracktype==TRACKTYPE_AUDIO && t->record==true)
						{
							if(t->io.in_vchannel)
							{
								audiotrackschannels+=t->io.in_vchannel->channels;
							}
						}

						t=t->NextTrack();
					}

					bool recok;
					unsigned __int64 freerecmemoryondisk=mainaudio->GetFreeRecordingMemory(song,&recok); // Bytes if recok==true

					if(recok==true)
					{
						if(audiotrackschannels==0)
						{
							audiousage->ChangeButtonText("(REC)-:-");
							//progress->ChangeButtonText("Bla");
						}
						else
						{
							//freerecmemoryondisk/=1024; // ->Kbytes

							unsigned __int64 h=mainaudio->GetGlobalSampleRate();
							unsigned __int64 h2=mainaudio->GetActiveDevice()->FirstInputChannel()->sizeofsample;

							h*=h2; // Bytes pro Channel/sek
							h*=audiotrackschannels; // Bytes all Channels
							freerecmemoryondisk/=h; // seconds

							unsigned __int64 hour=freerecmemoryondisk/3600;
							freerecmemoryondisk-=hour*3600;

							unsigned __int64 min=freerecmemoryondisk/60;

							char hourstr[NUMBERSTRINGLEN],minstr[NUMBERSTRINGLEN];

							char *hs=mainvar->GenerateString("(REC)",mainvar->ConvertIntToChar((int)hour,hourstr),"h:",mainvar->ConvertIntToChar((int)min,minstr),"min");
							if(hs)
							{
								audiousage->ChangeButtonText(hs);
								delete hs;
							}
						}

					}
					else
						audiousage->ChangeButtonText("(REC):???");
				}

				if(error==true)
					audiousage->SetColour(COLOUR_WHITE,COLOUR_ERROR);

	}
}

void Edit_Transport::RefreshRealtime()
{
	/*
	if(mainvar->GetActiveSong()!=song || mainvar->GetActiveProject()!=project)
	{
	if(mainvar->exitthreads==false)
	{
	song=mainvar->GetActiveSong();
	project=mainvar->GetActiveProject();

	ShowMasterPeak();
	ShowTitleBar();
	RefreshMenu(); // set active flags
	//InitGadgets();
	//RedrawOSGadgets();
	}
	}
	else
	*/

	{
		if(WindowSong())
		{
			ShowMasterPeak();
			ShowMasterVolume();
		}

		// Check Gadget Status
		if(WindowSong() && 
			(WindowSong()->status!=song_status || WindowSong()->punchrecording!=song_punchrecording)
			)
			ShowStatus();

		/*
		if( mainaudio->selectedaudiohardware && selectedaudiohardware!=mainaudio->selectedaudiohardware->activedevice)
		ShowTitleBar();
		else
		if(mainaudio->GetGlobalSampleRate()!=samplerate)
		ShowTitleBar();
		else
		if(mainaudio->GetActiveDevice() && mainaudio->GetActiveDevice()->setSize!=setSize)
		ShowTitleBar();
		*/
	}

	if(WindowSong()){

		// Check Song Stop Position Marker
		if(WindowSong()->status&Seq_Song::STATUS_PLAY)
		{
			Seq_Marker *marker=song->textandmarker.FirstMarker();

			while(marker)
			{
				if(marker->functionflag==Seq_Marker::MARKERFUNC_STOPPLAYBACK)
				{
					if(WindowSong()->GetSongPosition()>=marker->GetMarkerStart())
					{
						WindowSong()->StopSelected();
					}
				}

				marker=marker->NextMarker();
			}
		}

		ShowTime(true);
		ShowTempo(true);
		ShowCycle(true);
		ShowSignature(true);
		//	ShowImpulse();

		if(status_sync!=WindowSong()->MIDIsync.sync)
			ShowStatus();

		if(sl_measure!=WindowSong()->GetSongLength_Measure())
			ShowSongLength();

		if(gl_cycle && WindowSong()->playbacksettings.cycleplayback!=sl_cycle)
		{
			sl_cycle=WindowSong()->playbacksettings.cycleplayback;
			gl_cycle->ChangeButtonImage((WindowSong()->playbacksettings.cycleplayback==true)?IMAGE_CYCLEBUTTON_ON:IMAGE_CYCLEBUTTON_OFF);
		}

		if(gl_solo && WindowSong()->playbacksettings.solo!=sl_solo)
		{
			sl_solo=WindowSong()->playbacksettings.solo;
			gl_solo->ChangeButtonImage(sl_solo?IMAGE_SOLOBUTTON_ON:IMAGE_SOLOBUTTON_OFF);
		}

		if(gl_metro && WindowSong()->metronome.on!=sl_metro)
		{
			sl_metro=song->metronome.on;
			gl_metro->Toggle(sl_metro);
		}

		if(gl_autoplayback && WindowSong()->playbacksettings.automationplayback!=sl_automationplayback)
		{
			sl_automationplayback=WindowSong()->playbacksettings.automationplayback;
			gl_autoplayback->Toggle(sl_automationplayback);
		}

		if(gl_autorecording && WindowSong()->playbacksettings.automationrecording!=sl_automationrecording)
		{
			sl_automationrecording=WindowSong()->playbacksettings.automationrecording;
			gl_autorecording->Toggle(sl_automationrecording);
		}
	}

	ShowLastMIDIIO(song);
}

void Edit_Transport::ShowTransportMenu()
{
	/*
	if(menu_MIDIthru)
	{
	if(mainMIDI->MIDIthru==true)
	menu_MIDIthru->Select(0,true);
	else
	menu_MIDIthru->Select(0,false);

	if(mainMIDI->MIDIthru_notes==true)
	menu_MIDIthru->Select(1,true);
	else
	menu_MIDIthru->Select(1,false);

	if(mainMIDI->MIDIthru_polypress==true)
	menu_MIDIthru->Select(2,true);
	else
	menu_MIDIthru->Select(2,false);

	if(mainMIDI->MIDIthru_controlchange==true)
	menu_MIDIthru->Select(3,true);
	else
	menu_MIDIthru->Select(3,false);

	if(mainMIDI->MIDIthru_programchange==true)
	menu_MIDIthru->Select(4,true);
	else
	menu_MIDIthru->Select(4,false);

	if(mainMIDI->MIDIthru_channelpress==true)
	menu_MIDIthru->Select(5,true);
	else
	menu_MIDIthru->Select(5,false);

	if(mainMIDI->MIDIthru_pitchbend==true)
	menu_MIDIthru->Select(6,true);
	else
	menu_MIDIthru->Select(6,false);

	if(mainMIDI->MIDIthru_sysex==true)
	menu_MIDIthru->Select(7,true);
	else
	menu_MIDIthru->Select(7,false);
	}

	if(menu_options)
	{
	if(mainsettings->waitforMIDIplayback==true)
	menu_options->Select(4,true);
	else
	menu_options->Select(4,false);

	if(mainsettings->waitforMIDIrecord==true)
	menu_options->Select(5,true);
	else
	menu_options->Select(5,false);

	if(mainMIDI->MIDIoutactive==true)
	menu_options->Select(7,true);
	else
	menu_options->Select(7,false);

	if(mainMIDI->MIDIinactive==true)
	menu_options->Select(8,true);
	else
	menu_options->Select(8,false);

	if(mainaudio->audiooutactive==true)
	menu_options->Select(9,true);
	else
	menu_options->Select(9,false);
	}
	*/

	//ShowUndoMenu();
}


#ifdef OLDIE
void Edit_Transport::MenuSelect(int menuid)
{
	// MessageBeep(-1);

	// Song Select
	if(menuid>=TRANSMENU_SELECT_SONG && menuid<TRANSMENU_SELECT_PROJECT)
	{
		if(mainvar->GetActiveProject())
		{
			Seq_Song *song=mainvar->GetActiveProject()->GetSongIndex(menuid-TRANSMENU_SELECT_SONG);

			if(song)
				mainvar->SetActiveSong(song);
		}
	}
	else
		if(menuid>=TRANSMENU_SELECT_PROJECT)
		{
			Seq_Project *project=mainvar->GetProjectIndex(menuid-TRANSMENU_SELECT_PROJECT);

			if(project!=mainvar->GetActiveProject())
				mainvar->SetActiveProject(project,0);

		}
		else
			switch(menuid)
		{

#ifdef _DEBUG
			/*
			case TRANSMENU_CREATELANGUAGE:
			{
			language.CreateLanguageFiles();
			}
			break;
			*/
#endif

			/*
			case TRANSMENU_TRIGGERRECORD:
			if(mainsettings->waitforMIDIrecord==true)
			mainsettings->waitforMIDIrecord=false;
			else
			mainsettings->waitforMIDIrecord=true;

			ShowTransportMenu();
			break;
			*/

			case TRANSMENU_AUDIOOUTPUT:
				mainaudio->audiooutactive=mainaudio->audiooutactive==true?false:true;
				ShowTransportMenu();
				break;

			case TRANSMENU_MIDIOUTPUT:
				if(mainMIDI->MIDIoutactive==true)
					mainMIDI->MIDIoutactive=false;
				else
					mainMIDI->MIDIoutactive=true;

				ShowTransportMenu();
				break;

			case TRANSMENU_MIDITHRU:
				if(mainMIDI->MIDIthru==true)
					mainMIDI->MIDIthru=false;
				else
					mainMIDI->MIDIthru=true;

				ShowTransportMenu();
				break;

			case TRANSMENU_MIDITHRU_NOTES:
				if(mainMIDI->MIDIthru_notes==true)
					mainMIDI->MIDIthru_notes=false;
				else
					mainMIDI->MIDIthru_notes=true;

				ShowTransportMenu();
				break;

			case TRANSMENU_MIDITHRU_POLYPRESSURE:
				if(mainMIDI->MIDIthru_polypress==true)
					mainMIDI->MIDIthru_polypress=false;
				else
					mainMIDI->MIDIthru_polypress=true;

				ShowTransportMenu();
				break;

			case TRANSMENU_MIDITHRU_CONTROLCHANGE:
				if(mainMIDI->MIDIthru_controlchange==true)
					mainMIDI->MIDIthru_controlchange=false;
				else
					mainMIDI->MIDIthru_controlchange=true;

				ShowTransportMenu();
				break;


			case TRANSMENU_MIDITHRU_PROGRAMCHANGE:
				if(mainMIDI->MIDIthru_programchange==true)
					mainMIDI->MIDIthru_programchange=false;
				else
					mainMIDI->MIDIthru_programchange=true;

				ShowTransportMenu();
				break;


			case TRANSMENU_MIDITHRU_CHANNELPRESSURE:
				if(mainMIDI->MIDIthru_channelpress==true)
					mainMIDI->MIDIthru_channelpress=false;
				else
					mainMIDI->MIDIthru_channelpress=true;

				ShowTransportMenu();
				break;

			case TRANSMENU_MIDITHRU_PITCHBEND:
				if(mainMIDI->MIDIthru_pitchbend==true)
					mainMIDI->MIDIthru_pitchbend=false;
				else
					mainMIDI->MIDIthru_pitchbend=true;

				ShowTransportMenu();
				break;


			case TRANSMENU_MIDITHRU_SYSEX:
				if(mainMIDI->MIDIthru_sysex==true)
					mainMIDI->MIDIthru_sysex=false;
				else
					mainMIDI->MIDIthru_sysex=true;

				ShowTransportMenu();
				break;

			case TRANSMENU_DELETETEMPOCHANGES:
				{
					Seq_Song *song=mainvar->GetActiveSong();

					if(song)
					{
						mainthreadcontrol->LockActiveSong();
						song->timetrack.StopTimeTrack(song);
						mainthreadcontrol->UnlockActiveSong();
					}
				}
				break;
		}
}
#endif

void Edit_Transport::OpenMixer()
{
	if(screen)
	{
		if(screen->Mixer()==true)
			return;
	}

	if(song)
	{
		guiWindowSetting setting;
		setting.screen=screen;

		maingui->OpenEditorStart(EDITORTYPE_AUDIOMIXER,song,0,(Seq_SelectionList *)1,&setting,0,0);
	}
}

guiWindow *Edit_Transport::OpenEditor(int id)
{
	if(screen)
		return screen->Editor(id);

	return 0;
}

void Edit_Transport::Gadget(guiGadget *g)
{
	if(WindowSong())
		switch(g->gadgetID)
	{
		case GADGETID_SIGNATURENUMERATOR:
			{
				Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(WindowSong()->GetSongPosition());

				DeletePopUpMenu(true);

				if(popmenu)
				{
					class menu_numerator:public guiMenu
					{
					public:
						menu_numerator(Seq_Song *s,Seq_Signature *sg,int n){song=s;sign=sg;num=n;}

						void MenuFunction()
						{
							song->timetrack.SetNumerator(sign,num);
							maingui->RefreshSignature(song);
						}

						Seq_Song *song;
						Seq_Signature *sign;
						int num;
					};

					char nr[NUMBERSTRINGLEN];

					for(int i=1;i<24;i++)
						popmenu->AddFMenu(mainvar->ConvertIntToChar(i,nr),new menu_numerator(WindowSong(),sig,i),sig->nn==i?true:false);

					ShowPopMenu();
				}
			}
			break;

		case GADGETID_SIGNATUREDENUMERATOR:
			{
				Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(WindowSong()->GetSongPosition());

				DeletePopUpMenu(true);

				if(popmenu)
				{
					class menu_denumerator:public guiMenu
					{
					public:
						menu_denumerator(Seq_Song *s,Seq_Signature *sig,OSTART dn){song=s;sign=sig;dnum=dn;}

						void MenuFunction()
						{
							song->timetrack.SetDeNumerator(sign,dnum);
							maingui->RefreshSignature(song);
						}

						Seq_Song *song;
						Seq_Signature *sign;
						OSTART dnum;
					};

					popmenu->AddFMenu("2",new menu_denumerator(WindowSong(),sig,TICK2nd),sig->dn_ticks==TICK2nd?true:false);
					popmenu->AddFMenu("4",new menu_denumerator(WindowSong(),sig,TICK4nd),sig->dn_ticks==TICK4nd?true:false);
					popmenu->AddFMenu("8",new menu_denumerator(WindowSong(),sig,TICK8nd),sig->dn_ticks==TICK8nd?true:false);
					popmenu->AddFMenu("16",new menu_denumerator(WindowSong(),sig,TICK16nd),sig->dn_ticks==TICK16nd?true:false);

					ShowPopMenu();
				}
			}
			break;

		case GADGETID_AUTOMATIONPLAYBACK:
			WindowSong()->playbacksettings.automationplayback=WindowSong()->playbacksettings.automationplayback==true?false:true;
			break;

		case GADGETID_AUTOMATIONRECORD:
			WindowSong()->playbacksettings.automationrecording=WindowSong()->playbacksettings.automationrecording==true?false:true;
			break;

		case GADGETID_MASTERVOL:
			{
				OSTART atime=GetAutomationTime();
				WindowSong()->audiosystem.masterchannel.io.audioeffects.volume.AutomationEdit(WindowSong(),atime,0,mastervolume_db->volume);
			}
			break;

		case GADGETID_FX:
			WindowSong()->audiosystem.SetSystemByPass(WindowSong()->audiosystem.systembypassfx==true?false:true);
			break;

		case GADGETID_CPU:
			{
				globmenu_cpu menu;
				menu.MenuFunction();
			}
			break;

		case GADGETID_METRO:
			{
				sl_metro=WindowSong()->metronome.on=WindowSong()->metronome.on==true?false:true;
				gl_metro->Toggle(WindowSong()->metronome.on);
			}
			break;

		case GADGETID_MIX:
			{
				OpenMixer();
			}
			break;

		case GADGETID_EDIT:
			OpenEditor();
			break;

		case GADGETID_SPQ:
		case GADGETID_SPQ_INFO:
			{
				WindowSong()->SetSongPositionPrevOrNextMeasure(maingui->GetShiftKey()==true?true:false);
			}
			break;

		case GADGETID_SPP0:
			WindowSong()->SetSongPosition(0,true);
			break;

		case GADGETID_PREVMARKER:
			WindowSong()->SetSongPositionWithMarker(WindowSong()->GetSongPosition(),false);
			break;

		case GADGETID_NEXTMARKER:
			WindowSong()->SetSongPositionWithMarker(WindowSong()->GetSongPosition(),true);
			break;

		case GADGETID_REW:
			WindowSong()->REW();
			break;

		case GADGETID_FF:
			WindowSong()->FF();
			break;

		case GADGETID_CYCLELEFT_INFO:
			WindowSong()->SetSongPosition(WindowSong()->playbacksettings.cyclestart,true);
			break;

		case GADGETID_CYCLERIGHT_INFO:

			if(WindowSong()->status==Seq_Song::STATUS_STOP || WindowSong()->playbacksettings.cycleplayback==false)
				WindowSong()->SetSongPosition(WindowSong()->playbacksettings.cycleend,true);
			break;

		case GADGETID_SONGZOOM:
			{
				DeletePopUpMenu(true);

				if(popmenu)
				{
					class menu_songzoom:public guiMenu
					{
					public:
						menu_songzoom(Edit_Transport *tr,int t){editor=tr;ticks=t;}

						void MenuFunction()
						{
							editor->WindowSong()->SetGUIZoom(ticks);

						} //

						Edit_Transport *editor;
						int ticks;
					};

					popmenu->AddMenu(Cxs[CXS_DEFAULTSONGGRID],0);
					popmenu->AddLine();

					popmenu->AddFMenu("/16",new menu_songzoom(this,TICK16nd),WindowSong()->timetrack.zoomticks==TICK16nd?true:false);
					popmenu->AddFMenu("/32",new menu_songzoom(this,TICK32nd),WindowSong()->timetrack.zoomticks==TICK32nd?true:false);
					popmenu->AddFMenu("/64",new menu_songzoom(this,TICK64nd),WindowSong()->timetrack.zoomticks==TICK64nd?true:false);

					ShowPopMenu();
				}
			}
			break;

		case GADGETID_SONGLENGTH:
			{
				WindowSong()->SetSongLength(g->GetPos());
			}
			break;

		case GADGETID_CYCLELEFT:
			WindowSong()->SetCycleStart(cycle_left->GetTime(),false);
			cycle_left->t_time=WindowSong()->playbacksettings.cyclestart;
			break;

		case GADGETID_CYCLERIGHT:
			WindowSong()->SetCycleEnd(cycle_right->GetTime());
			cycle_right->t_time=WindowSong()->playbacksettings.cycleend;
			break;

		case GADGETID_TIMEMEASURE:
			WindowSong()->SetSongPosition(timegadget_measure->GetTime(),true);
			break;

		case GADGETID_TIMESMPTE:
			WindowSong()->SetSongPosition(timegadget_smpte->GetTime(),true);
			break;

		case GADGETID_TEMPO:
			WindowSong()->ChangeTempoAtPosition(-1,g->GetDoublePos(),0);
			ShowTempo(false);
			break;

		case GADGETID_TEMPO_INFO:
			{
				if(maingui->WindowToFront(WindowSong(),EDITORTYPE_TEMPO)==0)
				{
					globmenu_TMap teditor(screen);
					teditor.MenuFunction();
				}
			}
			break;

		case GADGETID_SOLO:
			if(WindowSong()->CanStatusBeChanged()==true)
			{		
				WindowSong()->playbacksettings.solo^=1;

				//	if(song->playbacksettings.solo)
				//		song->SoloPlayback();
			}
			break;

		case GADGETID_CYCLE:
			{		
				WindowSong()->ToggleCycle();
			}
			break;

		case GADGETID_STOP:
			{
				TRACE ("-> STOP Song\n");
				WindowSong()->StopSelected();
			}
			break;

		case GADGETID_PLAY:
			{
				TRACE ("-> PLAY Song\n");

				WindowSong()->SetSync(SYNC_INTERN,true);
				WindowSong()->PlaySong();
			}
			break;	

		case GADGETID_RECORD:
			TRACE ("-> RECORD Song\n");
			WindowSong()->SetSync(SYNC_INTERN,true);
			WindowSong()->RecordSong();
			break;	
	}	
}

/*
void Edit_Transport::ResetGadgets()
{
signgadget=0;

gl_solo=
gl_cycle=
gl_metro=
gl_start=
gl_stop=
gl_record=
timegadget_measure=
timegadget_smpte=
syncgadget=
cycle_left=
cycle_right=
songzoomgadget=
MIDIininfo=
MIDIoutinfo=
audiousage=
progress=0;
}
*/

void Edit_Transport::RefreshSong(Seq_Song *newsong)
{
	song=newsong;

	glist.Enable(tempogadget,song?true:false);
	glist.Enable(signgadget,song?true:false);
	glist.Enable(songlengthgadget,song?true:false);
	glist.Enable(cycle_left,song?true:false);
	glist.Enable(cycle_right,song?true:false);
	glist.Enable(timegadget_measure,song?true:false);
	glist.Enable(timegadget_smpte,song?true:false);


	if(song)
	{
		ShowTempo(false);
		ShowSignature(false);
		ShowSongLength();
		ShowCycle(false);

		ShowMasterPeak(true);
		if(masterpeak)
			masterpeak->Blt();

		ShowMasterVolume(true);
		if(mastervolume)
			mastervolume->Blt();

		ShowTime(false);
	}
}


