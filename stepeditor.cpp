#include "stepeditor.h"
#include "gui.h"
#include "object_song.h"
#include "editfunctions.h"
#include "settings.h"
#include "MIDIoutproc.h"
#include "languagefiles.h"
#include "songmain.h"
#include "MIDIhardware.h"

enum RecEditorGID
{
	GADGET_METRO,
	GADGET_STEPONOFF,
	GADGET_STEPSTEP,
	GADGET_STEPLENGTH ,
	GADGET_STEPLEFT,
	GADGET_STEPRIGHT,
	GADGET_CYCLENEWTRACK,
	GADGET_CYCLENEWCHILDTRACK,
	GADGET_MUTEPARENTTRACKS,
	GADGET_METROCLICK,
	GADGET_METROCLICKPLAYBACK,
	GADGET_PRE_I,
	GADGET_PREMETRO,
	GADGET_MIDIRECORDING,
	GADGET_AUDIORECORDING,
	GADGET_PREMETRONUMBER,
	GADGET_PREMETROTYPE,
	GADGET_PREMETRONOTE,
	GADGET_PUNCHIN,
	GADGET_PUNCHOUT,
	GADGET_TRIGGERNOTE,
	GADGET_RECORDTOFPATTERN,
	GADGET_ALLOWTEMPOCHANGERECORDING,
	GADGET_AUTORECORDTRACK
};

Edit_RecordingSettings::Edit_RecordingSettings()
{
	editorid=EDITORTYPE_RECORDEDITOR;
	editorname=Cxs[CXS_RECORDSETTINGS];
	dialogstyle=true;
	ondesktop=true;
};

void Edit_RecordingSettings::Gadget(guiGadget *g)
{
	Seq_Song *song=mainvar->GetActiveSong();

	switch(g->gadgetID)
	{
	case GADGET_AUTORECORDTRACK:
		mainsettings->setfocustrackautotorecord=mainsettings->setfocustrackautotorecord==true?false:true;
		mainsettings->Save(0);
		break;

	case GADGET_PREMETRONUMBER:
			{
				if( (!mainvar->GetActiveSong()) || (!(mainvar->GetActiveSong()->status&Seq_Song::STATUS_WAITPREMETRO)) )
				{
					mainsettings->numberofpremetronomes=g->GetPos();
					mainsettings->Save(0);
				}
				else
					ShowGadgets();
			}
		break;

	case GADGET_METRO:
		if(song)
		{
		sl_metro=song->metronome.on=song->metronome.on==true?false:true;
		g->SetCheckBox(sl_metro);
		}
		break;

	case GADGET_ALLOWTEMPOCHANGERECORDING:
		mainsettings->allowtempochangerecording=mainsettings->allowtempochangerecording==true?false:true;
		mainsettings->Save(0);
		break;

	case GADGET_RECORDTOFPATTERN:
		mainsettings->recordtofirstselectedMIDIPattern=mainsettings->recordtofirstselectedMIDIPattern==true?false:true;
		mainsettings->Save(0);
		break;

	case GADGET_MIDIRECORDING:
		mainsettings->recording_MIDI=mainsettings->recording_MIDI==true?false:true;
		mainsettings->Save(0);
		break;

	case GADGET_AUDIORECORDING:
		mainsettings->recording_audio=mainsettings->recording_audio==true?false:true;
		mainsettings->Save(0);
		break;

	case GADGET_PUNCHIN:
		if(song && (!(song->status&Seq_Song::STATUS_RECORD))) // dont change punchrecording during RECORD !
		{
			if(song->punchrecording&Seq_Song::PUNCHIN)
				song->punchrecording CLEARBIT Seq_Song::PUNCHIN;
			else
			{
				song->punchrecording |=Seq_Song::PUNCHIN;

				if(song->punchrecording&Seq_Song::PUNCHOUT)
				{
					song->punchrecording CLEARBIT Seq_Song::PUNCHOUT;
					ShowGadgets();
				}
			}
		}
		else
			ShowGadgets();

		break;

	case GADGET_PUNCHOUT:
		if(song && (!(song->status&Seq_Song::STATUS_RECORD))) // dont change punchrecording during RECORD !
		{
			if(song->punchrecording&Seq_Song::PUNCHOUT)
				song->punchrecording CLEARBIT Seq_Song::PUNCHOUT;
			else
			{
				song->punchrecording |=Seq_Song::PUNCHOUT;

				if(song->punchrecording&Seq_Song::PUNCHIN)
				{
					song->punchrecording CLEARBIT Seq_Song::PUNCHIN;
					ShowGadgets();
				}
			}
		}
		else
			ShowGadgets();

		break;

	case GADGET_STEPLEFT:
		if(song && song->status==Seq_Song::STATUS_STEPRECORD)
		{
			song->SetSongPosition(song->GetSongPosition()-song->GetStepTime(),false);
		}
		break;

	case GADGET_STEPRIGHT:
		if(song && song->status==Seq_Song::STATUS_STEPRECORD)
		{
			song->SetSongPosition(song->GetSongPosition()+song->GetStepTime(),false);
		}
		break;

	case GADGET_STEPONOFF:
		if(song)
		{	
			if(song->steprecordingonoff==true)
				song->steprecordingonoff=false;
			else
				song->steprecordingonoff=true;
		}
		break;

	case GADGET_STEPSTEP:
		if(song)
		{
			song->stepstep=g->index;
		}
		break;

	case GADGET_MUTEPARENTTRACKS:
		{
			mainsettings->automutechildsparent=(mainsettings->automutechildsparent==true)?false:true;
		}
		break;

	case GADGET_STEPLENGTH:
		if(song)
		{
			song->steplength=g->index;
		}
		break;

	case GADGET_CYCLENEWTRACK:
		{
			if(g->index)
			{
				mainsettings->createnewtrackaftercycle=true;
			}
			else
				mainsettings->createnewtrackaftercycle=false;

			mainsettings->Save(0);
		}
		break;

	case GADGET_CYCLENEWCHILDTRACK:
		{
			if(g->index)
			{
				mainsettings->createnewchildtrackwhenrecordingstarts=true;	
			}
			else
				mainsettings->createnewchildtrackwhenrecordingstarts=false;

			mainsettings->Save(0);
		}
		break;

	case GADGET_METROCLICK:
		if(song)
		{
			if(g->index)
				song->metronome.record=true;
			else
				song->metronome.record=false;
		}
		break;

	case GADGET_METROCLICKPLAYBACK:
		if(song)
		{
			if(g->index)
				song->metronome.playback=true;
			else
				song->metronome.playback=false;
		}
		break;

	case GADGET_PREMETRONOTE:
		{
			if(g->index)
				mainsettings->noteendsprecounter=true;
			else
				mainsettings->noteendsprecounter=false;

			mainsettings->Save(0);
		}
		break;

	case GADGET_PREMETRO:
		{
			if(g->index)
				mainsettings->usepremetronome=true;
			else
				mainsettings->usepremetronome=false;

			mainsettings->Save(0);
		}
		break;

	case GADGET_PREMETROTYPE:
		{
			mainsettings->precountertype=g->index;
			mainsettings->Save(0);
		}
		break;

	case GADGET_TRIGGERNOTE:
		if(mainsettings->waitforMIDIrecord==true)
			mainsettings->waitforMIDIrecord=false;
		else
			mainsettings->waitforMIDIrecord=true;
		mainsettings->Save(0);
		break;

	}
}

void Edit_RecordingSettings::RefreshRealtime()
{
	if(metroonoff && sl_metro!=WindowSong()->metronome.on)
		metroonoff->SetCheckBox(sl_metro=WindowSong()->metronome.on);

}

void Edit_RecordingSettings::ShowGadgets()
{
	if(metroonoff)
		metroonoff->SetCheckBox(sl_metro=WindowSong()->metronome.on);

	if(audiorecording)
		audiorecording->SetCheckBox(mainsettings->recording_audio);

	if(MIDIrecording)
		MIDIrecording->SetCheckBox(mainsettings->recording_MIDI);

	if(premetronote)
		premetronote->SetCheckBox(mainsettings->noteendsprecounter);

	if(premetro)
		premetro->SetCheckBox(mainsettings->usepremetronome);

	if(premetroint)
		premetroint->SetPos(mainsettings->numberofpremetronomes);

	if(recordtofirstpattern)
		recordtofirstpattern->SetCheckBox(mainsettings->recordtofirstselectedMIDIPattern);

	if(setrecord)
		setrecord->SetCheckBox(mainsettings->setfocustrackautotorecord);

	if(metro)
	{
		metro->SetCheckBox(status_metrorecord=song->metronome.record);
		metro->Enable();
	}

	if(metroplayback)
	{
		metroplayback->SetCheckBox(status_metroplayback=song->metronome.playback);
		metroplayback->Enable();
	}

	if(steponoff)
	{
		steponoff->SetCheckBox(status_onoff=song->steprecordingonoff);
		steponoff->Enable();
	}

	if(songpunchin)
	{
		if(song->punchrecording&Seq_Song::PUNCHIN)
			songpunchin->SetCheckBox(true);
		else
			songpunchin->SetCheckBox(false);

		songpunchin->Enable();
	}

	if(songpunchout)
	{
		if(song->punchrecording&Seq_Song::PUNCHOUT)
			songpunchout->SetCheckBox(true);
		else
			songpunchout->SetCheckBox(false);

		songpunchout->Enable();
	}

	status_punch=song->punchrecording;

	if(stepstep)
	{
		stepstep->SetCycleSelection(status_stepstep=song->stepstep);
		stepstep->Enable();
	}

	if(steplength)
	{
		steplength->SetCycleSelection(status_steplength=song->steplength);
		steplength->Enable();
	}

	if(preccountertype)
	{
		preccountertype->SetCycleSelection(mainsettings->precountertype);
	}

	if(cyclecreatenewtrack)
		cyclecreatenewtrack->SetCheckBox(mainsettings->createnewtrackaftercycle);

	if(cyclecreatenewchild)
		cyclecreatenewchild->SetCheckBox(mainsettings->createnewchildtrackwhenrecordingstarts);

	if(triggerrecordnote)
		triggerrecordnote->SetCheckBox(mainsettings->waitforMIDIrecord);
}

void Edit_RecordingSettings::KeyDown()
{
	switch(nVirtKey)
	{
	case KEY_CURSORLEFT:
		Gadget(stepleft);
		break;

	case KEY_CURSORRIGHT:
		Gadget(stepright);
		break;
	}
}

guiMenu *Edit_RecordingSettings::CreateMenu()
{
	guiMenu *n;

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		n=menu->AddMenu(Cxs[CXS_RECORDSETTINGS],0);
	}

	return menu;
}

void Edit_RecordingSettings::Init()
{
	glist.SelectForm(0,0);

	int w=40*maingui->GetFontSizeY();

	if(char *hs=mainvar->GenerateString(Cxs[CXS_RECORD],":","MIDI"))
	{
		MIDIrecording=glist.AddCheckBox(-1,-1,w,-1,GADGET_MIDIRECORDING,0,hs,hs);
		delete hs;
	}

	glist.AddLY();

	if(char *hs=mainvar->GenerateString(Cxs[CXS_RECORD],":","Audio"))
	{
		audiorecording=glist.AddCheckBox(-1,-1,w,-1,GADGET_AUDIORECORDING,0,hs,hs);
		delete hs;
	}

	glist.AddLY();

	metroonoff=glist.AddCheckBox(-1,-1,w,-1,GADGET_METRO,0,Cxs[CXS_METRONOME],Cxs[CXS_METRONOME]);
	glist.Return();

	metro=glist.AddCheckBox(-1,-1,w,-1,GADGET_METROCLICK,0,Cxs[CXS_METROCLICKWHILEREC],Cxs[CXS_METROCLICKWHILEREC]);
	glist.AddLY();

	metroplayback=glist.AddCheckBox(-1,-1,w,-1,GADGET_METROCLICKPLAYBACK,0,Cxs[CXS_METROCLICKWHILEPLAYBACK],Cxs[CXS_METROCLICKWHILEPLAYBACK]);
	glist.AddLY();

	premetro=glist.AddCheckBox(-1,-1,w,-1,GADGET_PREMETRO,0,Cxs[CXS_METRONOMEPRECOUNTER],Cxs[CXS_METRONOMEPRECOUNTER]);
	glist.AddLY();

	glist.AddButton(-1,-1,w/2,-1,Cxs[CXS_METRONOMEPRECOUNTER],GADGET_PRE_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();

	premetroint=glist.AddNumberButton(-1,-1,w/2,-1,GADGET_PREMETRONUMBER,1,8,mainsettings->numberofpremetronomes,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	preccountertype=glist.AddCycle(-1,-1,w,-1,GADGET_PREMETROTYPE,0,0);
	if(preccountertype)
	{
		preccountertype->AddStringToCycle(Cxs[CXS_PRECOUNTERATPOSITIONONE]);
		preccountertype->AddStringToCycle(Cxs[CXS_PRECOUNTERALWAYS]);
	}

	glist.Return();

	premetronote=glist.AddCheckBox(-1,-1,w,-1,GADGET_PREMETRONOTE,0,Cxs[CXS_NOTEENDSPRECOUNTER],Cxs[CXS_NOTEENDSPRECOUNTER_I]);
	glist.AddLY();

	triggerrecordnote=glist.AddCheckBox(-1,-1,w,-1,GADGET_TRIGGERNOTE,0,Cxs[CXS_RECWAITSFORFIRSTNOTE],Cxs[CXS_RECWAITSFORFIRSTNOTE_I]);
	glist.AddLY();

	recordtofirstpattern=glist.AddCheckBox(-1,-1,w,-1,GADGET_RECORDTOFPATTERN,0,Cxs[CXS_RECTOFIRSTPATTERN],Cxs[CXS_RECTOFIRSTPATTERN]);
	glist.AddLY();

	setrecord=glist.AddCheckBox(-1,-1,w,-1,GADGET_AUTORECORDTRACK,0,Cxs[CXS_SETAUTORECORD],Cxs[CXS_SETAUTORECORD]);
	glist.AddLY();

	songpunchin=glist.AddCheckBox(-1,-1,w,-1,GADGET_PUNCHIN,0,Cxs[CXS_PUNCHINRECORDING],Cxs[CXS_PUNCHINRECORDING_I]);
	glist.AddLY();

	songpunchout=glist.AddCheckBox(-1,-1,w,-1,GADGET_PUNCHOUT,0,Cxs[CXS_PUNCHINRECORDING_OUT],Cxs[CXS_PUNCHINRECORDING_OUT_I]);
	glist.AddLY();

	steponoff=glist.AddCheckBox(-1,-1,w,-1,GADGET_STEPONOFF,0,Cxs[CXS_STEPRECORDING]);
	glist.AddLY();

	stepstep=glist.AddCycle(-1,-1,w,-1,GADGET_STEPSTEP,0,Cxs[CXS_STEPRECORDINGRES]);

	if(stepstep)
	{
		stepstep->AddStringToCycle("Step: 1/1");
		stepstep->AddStringToCycle("Step: 1/2");
		stepstep->AddStringToCycle("Step: 1/4");
		stepstep->AddStringToCycle("Step: 1/8");
		stepstep->AddStringToCycle("Step: 1/16");
		stepstep->AddStringToCycle("Step: 1/32");
		stepstep->AddStringToCycle("Step: 1/64");
	}

	glist.AddLY();

	steplength=glist.AddCycle(-1,-1,w,-1,GADGET_STEPLENGTH,0,Cxs[CXS_STEPRECORDINGLENGTH]);

	if(steplength)
	{
		steplength->AddStringToCycle("Note: 1/1");
		steplength->AddStringToCycle("Note: 1/2");
		steplength->AddStringToCycle("Note: 1/4");
		steplength->AddStringToCycle("Note: 1/8");
		steplength->AddStringToCycle("Note: 1/16") ;
		steplength->AddStringToCycle("Note: 1/32");
		steplength->AddStringToCycle("Note: 1/64");
	}

	glist.AddLY();
	stepleft=glist.AddButton(-1,-1,w/2,-1,"<<<",GADGET_STEPLEFT,0,Cxs[CXS_SPSTEPLEFT]);
	glist.AddLX();

	stepright=glist.AddButton(-1,-1,w/2,-1,">>>",GADGET_STEPRIGHT,0,Cxs[CXS_SPSTEPRIGHT]);
	glist.ResetLX();
	glist.AddLY();

	cyclecreatenewtrack=glist.AddCheckBox(-1,-1,w,-1,GADGET_CYCLENEWTRACK,0,Cxs[CXS_CREATENEWRECORDINGTRACKCYCLE],Cxs[CXS_CREATENEWRECORDINGTRACKCYCLE_I]);
	glist.AddLY();

	cyclecreatenewchild=glist.AddCheckBox(-1,-1,w,-1,GADGET_CYCLENEWCHILDTRACK,0,Cxs[CXS_CREATENEWRECORDINGCHILDTRACKCYCLE],Cxs[CXS_CREATENEWRECORDINGTRACKCYCLE]);
	glist.AddLY();

	guiGadget *mute=glist.AddCheckBox(-1,-1,w,-1,GADGET_MUTEPARENTTRACKS,0,Cxs[CXS_MUTEPARENTCYLCETRACKS],Cxs[CXS_MUTEPARENTCYLCETRACKS]);
	if(mute)
		mute->SetCheckBox(mainsettings->automutechildsparent);

	glist.AddLY();

	tempochange=glist.AddCheckBox(-1,-1,w,-1,GADGET_ALLOWTEMPOCHANGERECORDING,0,Cxs[CXS_ALLOWTEMPORECORDING],Cxs[CXS_ALLOWTEMPORECORDING]);
	if(tempochange)
		tempochange->SetCheckBox(mainsettings->allowtempochangerecording);

	ShowGadgets();
}

// Track Recording Settings Editor

// Sync Editor
Edit_SyncEditor::Edit_SyncEditor()
	{
		editorid=EDITORTYPE_SYNCEDITOR;
		editorname=Cxs[CXS_SONGSYNCSETTINGS];
		dialogstyle=true;
		ondesktop=true;
	};


void Edit_SyncEditor::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_RECEIVEMIDISTARTSTOP:
		mainMIDI->SetReceiveMIDIStart(RECEIVEMIDISTART_OFF);
		break;

	case GADGET_RECEIVEMIDISTARTPLAYBACK:
		mainMIDI->SetReceiveMIDIStart(RECEIVEMIDISTART_PLAYBACK);
		break;

	case GADGET_RECEIVEMIDISTARTRECORD:
		mainMIDI->SetReceiveMIDIStart(RECEIVEMIDISTART_RECORD);
		break;

	case GADGET_RECEIVEMIDISTARTRECORDNOPRE:
		mainMIDI->SetReceiveMIDIStart(RECEIVEMIDISTART_RECORD_NOPRECOUNTER);
		break;

	case GADGET_RECEIVEMIDISTOP:
		{
			mainMIDI->receiveMIDIstop=mainMIDI->receiveMIDIstop==true?false:true;
			mainsettings->Save(0);
		}
		break;

	case GADGET_WAITFORMIDIPLAYBACK:
		mainsettings->SetPlaybackTrigger(mainsettings->waitforMIDIplayback==true?false:true);
		break;
	}

}

void Edit_SyncEditor::RefreshRealtime_Slow()
{
	ShowReceiveMIDIStartStop(false);
	ShowReceiveMIDIStartPlayback(false);
	ShowReceiveMIDIStartRecord(false);
	ShowReceiveMIDIStartRecordNoPre(false);
	ShowReceiveMIDIStop(false);
	ShowWaitForMIDIPlayback(false);
}

guiMenu *Edit_SyncEditor::CreateMenu()
{
	guiMenu *n;

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		n=menu->AddMenu(Cxs[CXS_SONGSYNCSETTINGS],0);
	}

	return menu;
}


void Edit_SyncEditor::ShowReceiveMIDIStartStop(bool force)
{
	if(force==true || status_receiveMIDIstartstop!=mainMIDI->receiveMIDIstart)
	{
		status_receiveMIDIstartstop=mainMIDI->receiveMIDIstart;

		if(receiveMIDIstartstop)
			receiveMIDIstartstop->SetCheckBox(status_receiveMIDIstartstop==RECEIVEMIDISTART_OFF?true:false);
	}
}

void Edit_SyncEditor::ShowReceiveMIDIStartPlayback(bool force)
{
	if(force==true || status_receiveMIDIstartplayback!=mainMIDI->receiveMIDIstart)
	{
		status_receiveMIDIstartplayback=mainMIDI->receiveMIDIstart;

		if(receiveMIDIstartplayback)
			receiveMIDIstartplayback->SetCheckBox(status_receiveMIDIstartplayback==RECEIVEMIDISTART_PLAYBACK?true:false);
	}
}

void Edit_SyncEditor::ShowReceiveMIDIStartRecord(bool force)
{
	if(force==true || status_receiveMIDIstartrecord!=mainMIDI->receiveMIDIstart)
	{
		status_receiveMIDIstartrecord=mainMIDI->receiveMIDIstart;

		if(receiveMIDIstartrecord)
			receiveMIDIstartrecord->SetCheckBox(status_receiveMIDIstartrecord==RECEIVEMIDISTART_RECORD?true:false);
	}
}

void Edit_SyncEditor::ShowReceiveMIDIStartRecordNoPre(bool force)
{
	if(force==true || status_receiveMIDIstartrecordnopre!=mainMIDI->receiveMIDIstart)
	{
		status_receiveMIDIstartrecordnopre=mainMIDI->receiveMIDIstart;

		if(receiveMIDIstartrecordnopre)
			receiveMIDIstartrecordnopre->SetCheckBox(status_receiveMIDIstartrecordnopre==RECEIVEMIDISTART_RECORD_NOPRECOUNTER?true:false);
	}
}

void Edit_SyncEditor::ShowReceiveMIDIStop(bool force)
{
	if(force==true || status_receiveMIDIstop!=mainMIDI->receiveMIDIstop)
	{
		status_receiveMIDIstop=mainMIDI->receiveMIDIstop;

		if(receiveMIDIstop)
			receiveMIDIstop->SetCheckBox(status_receiveMIDIstop);
	}
}

void Edit_SyncEditor::ShowWaitForMIDIPlayback(bool force)
{
	if(force==true || status_waitforMIDIplayback!=mainsettings->waitforMIDIplayback)
	{
		status_waitforMIDIplayback=mainsettings->waitforMIDIplayback;

		if(waitforMIDIplayback)
			waitforMIDIplayback->SetCheckBox(status_waitforMIDIplayback);
	}
}

void Edit_SyncEditor::Init()
{
	glist.SelectForm(0,0);

	int w=40*maingui->GetFontSizeY();

	receiveMIDIstartstop=glist.AddCheckBox(-1,-1,w,-1,GADGET_RECEIVEMIDISTARTSTOP,0,Cxs[CXS_RECEIVEMIDISTARTOFF]);
	ShowReceiveMIDIStartStop(true);
	glist.AddLY();
	receiveMIDIstartplayback=glist.AddCheckBox(-1,-1,w,-1,GADGET_RECEIVEMIDISTARTPLAYBACK,0,Cxs[CXS_RECEIVEMIDISTARTPLAYBACK]);
	ShowReceiveMIDIStartPlayback(true);

	glist.AddLY();
	receiveMIDIstartrecord=glist.AddCheckBox(-1,-1,w,-1,GADGET_RECEIVEMIDISTARTRECORD,0,Cxs[CXS_RECEIVEMIDISTARTRECORD]);
	ShowReceiveMIDIStartRecord(true);

	glist.AddLY();
	receiveMIDIstartrecordnopre=glist.AddCheckBox(-1,-1,w,-1,GADGET_RECEIVEMIDISTARTRECORDNOPRE,0,Cxs[CXS_RECEIVEMIDISTARTRECORDNOPRE]);
	ShowReceiveMIDIStartRecordNoPre(true);

	glist.AddLY();
	receiveMIDIstop=glist.AddCheckBox(-1,-1,w,-1,GADGET_RECEIVEMIDISTOP,0,Cxs[CXS_RECEIVEMIDISTOP]);
	ShowReceiveMIDIStop(true);

	glist.AddLY();
	waitforMIDIplayback=glist.AddCheckBox(-1,-1,w,-1,GADGET_WAITFORMIDIPLAYBACK,0,Cxs[CXS_TRIGGERSONGPLAYBACK]);
	ShowWaitForMIDIPlayback(true);
}