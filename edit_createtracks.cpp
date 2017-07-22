#include "edit_createtracks.h"
#include "object_song.h"
#include "object_track.h"
#include "camxgadgets.h"
#include "editfunctions.h"
#include "songmain.h"
#include "gui.h"
#include "languagefiles.h"
#include "editortypes.h"

extern char*channelchannelsinfo[];

char *plugincontrolnames[]=
{
	"Bypass",
};

char *syscontrolnames[]=
{
	"Mute",
	"Solo",
	"Audio Volume",
	"Audio Pan",
	"MIDI Velocity",
};

enum{
	GADGETID_FOCUS_I,
	GADGETID_FOCUS,

	GADGETID_RECTYPE,

	GADGETID_MIDIIN_I,
	GADGETID_MIDIIN,

	GADGETID_MIDIOUT_I,
	GADGETID_MIDIOUT,

	GADGETID_AUDIOIO_I,
	GADGETID_AUDIOIO,

	GADGETID_AUDIOIN_I,
	GADGETID_AUDIOIN,

	GADGETID_AUDIOOUT_I,
	GADGETID_AUDIOOUT,

	GADGETID_CREATE,
	GADGETID_CANCEL,
	GADGETID_TRACKS,
	GADGETID_TRACKSINT,
	GADGETID_CREATECLONE,
	GADGETID_CREATECHILD
};


void Edit_CreateTracks::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_RECTYPE:
		{
			maingui->EditTrackRecordType(this,&clonetrack);
		}
		break;

	case GADGETID_MIDIIN:
		maingui->EditTrackMIDIInput(this,&clonetrack);
		break;

	case GADGETID_MIDIOUT:
		maingui->EditTrackMIDIOutput(this,&clonetrack);
		break;

	case GADGETID_AUDIOIO:
		if(DeletePopUpMenu(true))
		{
			clonetrack.CreateChannelTypeMenu(popmenu);
			ShowPopMenu();
		}
		break;

	case GADGETID_AUDIOIN:
		maingui->EditTrackInput(this,&clonetrack);
		break;

	case GADGETID_AUDIOOUT:
		maingui->EditTrackOutput(this,&clonetrack);
		break;

	case GADGETID_TRACKS:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_createnewtracks:public guiMenu
				{
				public:
					menu_createnewtracks(Edit_CreateTracks *et,int f)
					{
						number=f;
						editor=et;
					}

					void MenuFunction()
					{
						editor->trackstocreate=number;
						if(editor->g_int)
							editor->g_int->SetPos(number);
					} //

					Edit_CreateTracks *editor;
					int number;
				};

				popmenu->AddFMenu("1",new menu_createnewtracks(this,1));
				popmenu->AddFMenu("2",new menu_createnewtracks(this,2));
				popmenu->AddFMenu("3",new menu_createnewtracks(this,3));
				popmenu->AddFMenu("4",new menu_createnewtracks(this,4));
				popmenu->AddFMenu("5",new menu_createnewtracks(this,5));
				popmenu->AddFMenu("6",new menu_createnewtracks(this,6));
				popmenu->AddFMenu("7",new menu_createnewtracks(this,7));
				popmenu->AddFMenu("8",new menu_createnewtracks(this,8));
				popmenu->AddFMenu("9",new menu_createnewtracks(this,9));
				popmenu->AddFMenu("10",new menu_createnewtracks(this,10));

				popmenu->AddFMenu("15",new menu_createnewtracks(this,15));
				popmenu->AddFMenu("20",new menu_createnewtracks(this,20));
				popmenu->AddFMenu("30",new menu_createnewtracks(this,30));

				ShowPopMenu();
			}
		}
		break;

	case GADGETID_TRACKSINT:
		trackstocreate=g->GetPos();
		break;

	case GADGETID_CREATECLONE:
		if(mainvar->GetActiveSong() && mainvar->GetActiveSong()->GetFocusTrack())
			mainedit->CreateNewTracks(mainvar->GetActiveSong(),mainvar->GetActiveSong()->GetFocusTrack(),trackstocreate,0,mainvar->GetActiveSong()->GetFocusTrack());
		break;

	case GADGETID_CREATE:
		if(mainvar->GetActiveSong())
		{
			//Seq_Track *totrack=mainvar->GetActiveSong()->GetFocusTrack();

			if(editorid==EDITORTYPE_CREATETRACKS)
			mainedit->CreateNewTracks(mainvar->GetActiveSong(),mainvar->GetActiveSong()->GetFocusTrack(),trackstocreate,0,&clonetrack,0);
			else
			{
				mainedit->CreateNewBusChannels(mainvar->GetActiveSong(),mainvar->GetActiveSong()->audiosystem.GetFocusBus(),trackstocreate,&clonetrack);
			}
		}

		break;

	case GADGETID_CREATECHILD:
		if(mainvar->GetActiveSong() && mainvar->GetActiveSong()->GetFocusTrack())
		{
			mainedit->CreateNewChildTracks(mainvar->GetActiveSong(),mainvar->GetActiveSong()->GetFocusTrack(),trackstocreate,0,mainvar->GetActiveSong()->GetFocusTrack(),&clonetrack);	
		}
		break;

	case GADGETID_CANCEL:
		closeit=true;
		break;
	}
}

void Edit_CreateTracks::RefreshRealtime_Slow()
{
	if(editorid==EDITORTYPE_CREATEBUS)
	{
		if(mainvar->GetActiveSong())
		{
			ShowAudio();
			ShowRecordType();
		}

		return;
	}

	// Tracks
	if(mainvar->GetActiveSong())
	{
		if(mainvar->GetActiveSong()->GetFocusTrack())
		{
			if(g_clone)
				g_clone->Enable();

			if(focustrack)
				focustrack->ChangeButtonText(mainvar->GetActiveSong()->GetFocusTrack()->GetName());

			if(mainvar->GetActiveSong()->GetFocusTrack()->CanNewChildTrack()==true)
			{
				if(g_child)
					g_child->Enable();
			}
			else
			{
				if(g_child)
					g_child->Disable();
			}

		}
		else
		{
			glist.Disable(focustrack);
			glist.Disable(g_clone);
			glist.Disable(g_child);
		}

		ShowMIDI();
		ShowAudio();
		ShowRecordType();
	}
	else
	{
		glist.Disable(focustrack);
		glist.Disable(g_clone);
		glist.Disable(g_child);
	}
}


void Edit_CreateTracks::ShowRecordType()
{
	if(editorid==EDITORTYPE_CREATEBUS)
		return;

	if(g_trackrecord)
	{
		switch(clonetrack.recordtracktype)
		{
		case TRACKTYPE_MIDI:
			g_trackrecord->ChangeButtonText(Cxs[CXS_RECONLYMIDI]);
			break;

		case TRACKTYPE_AUDIO:
			g_trackrecord->ChangeButtonText(Cxs[CXS_RECONLYAUDIO]);
			break;
		}
	}
}

void Edit_CreateTracks::Init()
{
	glist.SelectForm(0,0);

	int w=2*bitmap.GetTextWidth("* Tracks *");

	if(editorid==EDITORTYPE_CREATETRACKS)
	{
		glist.AddButton(-1,-1,w,-1,"Focus Track",GADGETID_FOCUS_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
		glist.AddLX();
		focustrack=glist.AddButton(-1,-1,-1,-1,GADGETID_FOCUS,MODE_RIGHT|MODE_INFO|MODE_NOMOUSEOVER);
		glist.Return();

		g_trackrecord=glist.AddButton(-1,-1,-1,-1,GADGETID_RECTYPE,MODE_RIGHT|MODE_MENU,Cxs[CXS_MIDIAUDIORECORD]);
		glist.Return();

		glist.AddButton(-1,-1,w,-1,"MIDI In",GADGETID_MIDIIN_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
		glist.AddLX();
		MIDIin=glist.AddButton(-1,-1,-1,-1,GADGETID_MIDIIN,MODE_RIGHT|MODE_MENU);
		glist.Return();

		glist.AddButton(-1,-1,w,-1,"MIDI Out",GADGETID_MIDIOUT_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
		glist.AddLX();
		MIDIout=glist.AddButton(-1,-1,-1,-1,GADGETID_MIDIOUT,MODE_RIGHT|MODE_MENU);
		glist.Return();
	}

	glist.AddButton(-1,-1,w,-1,"Audio IO",GADGETID_AUDIOIO_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	audioio=glist.AddButton(-1,-1,-1,-1,GADGETID_AUDIOIO,MODE_RIGHT|MODE_MENU);
	glist.Return();

	if(editorid==EDITORTYPE_CREATETRACKS)
	{
	glist.AddButton(-1,-1,w,-1,"Audio In",GADGETID_AUDIOIN_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	audioin=glist.AddButton(-1,-1,-1,-1,GADGETID_AUDIOIN,MODE_RIGHT|MODE_MENU);
	glist.Return();
	}

	glist.AddButton(-1,-1,w,-1,"Audio Out",GADGETID_AUDIOOUT_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	audioout=glist.AddButton(-1,-1,-1,-1,GADGETID_AUDIOOUT,MODE_RIGHT|MODE_MENU);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,Cxs[editorid==EDITORTYPE_CREATETRACKS?CXS_NEWTRACKS:CXS_NEWBUS],GADGETID_TRACKS,MODE_ADDDPOINT|MODE_MENU);
	glist.AddLX();
	g_int=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_TRACKSINT,1,32,4,NUMBER_INTEGER,MODE_RIGHT,Cxs[CXS_NRTRACKS]);
	glist.Return();

	g_create=glist.AddButton(-1,-1,-1,-1,Cxs[editorid==EDITORTYPE_CREATETRACKS?CXS_CREATETRACKS:CXS_CREATEBUS],GADGETID_CREATE,editorid==EDITORTYPE_CREATETRACKS?MODE_LEFTTOMID|MODE_TEXTCENTER:MODE_TEXTCENTER|MODE_RIGHT,editorid==EDITORTYPE_CREATETRACKS?Cxs[CXS_CREATENEWTRACKSNEXT]:0);

	if(editorid==EDITORTYPE_CREATETRACKS)
	{
		glist.AddLX();
		g_child=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_CREATECHILDS],GADGETID_CREATECHILD,MODE_MIDTORIGHT|MODE_TEXTCENTER,Cxs[CXS_CREATENEWCHILDTRACKSNEXT]);


		glist.Return();
		g_clone=glist.AddButton(-1,-1,-1,-1,"Clone Focus Track",GADGETID_CREATECLONE,MODE_RIGHT|MODE_TEXTCENTER,Cxs[CXS_CREATETRACKCLONES]);
	}

	glist.Return();
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),Cxs[CXS_CANCEL],GADGETID_CANCEL,MODE_RIGHT|MODE_TEXTCENTER);

	if(mainvar->GetActiveSong())
	{
		if(mainvar->GetActiveSong()->GetFocusTrack())
		{
			mainvar->GetActiveSong()->GetFocusTrack()->CloneFx(&clonetrack);
		}
	}

	ShowMIDI();
	ShowAudio();
	ShowRecordType();
}

void Edit_CreateTracks::ShowMIDI()
{
	if(editorid==EDITORTYPE_CREATEBUS)
		return;

	if(MIDIin)
	{
		// MIDI In
		if(char *h=clonetrack.GetMIDIInputString(false))
		{
			MIDIin->ChangeButtonText(h);
			delete h;
		}
	}

	if(MIDIout)
	{
		if(char *h=clonetrack.GetMIDIOutputString(false))
		{
			MIDIout->ChangeButtonText(h);
			delete h;
		}
	}
}

void Edit_CreateTracks::ShowAudio()
{
	if(audioio)
		audioio->ChangeButtonText(channelchannelsinfo[clonetrack.io.channel_type]);

	if(editorid==EDITORTYPE_CREATETRACKS)
	{
		if(audioin)
		{
			if(char *h=clonetrack.GetAudioFX()->GetAudioInputString())
			{
				audioin->ChangeButtonText(h);
				delete h;
			}
		}
	}

	if(audioout)
	{
		TAudioOut tao;
		Seq_AudioIO ao;

		tao.audiochannelouts=&ao;
		tao.simple=true;

		clonetrack.ShowAudioOutput(&tao);

		audioout->ChangeButtonText(tao.returnstring);
	}
}


void Edit_CreateTracks::FreeMemory()
{
	if(winmode&WINDOWMODE_DESTROY)
	{
		delete this;
	}
}

Edit_CreateTracks::Edit_CreateTracks()
{
	editorid=EDITORTYPE_CREATETRACKS;
	trackstocreate=4;

	InitForms(FORM_PLAIN1x1);
	ondesktop=true;
	dontchangesettings=true;
}

// Bus
Edit_CreateBus::Edit_CreateBus()
{
	editorid=EDITORTYPE_CREATEBUS;
}

// Automation

Edit_CreateAutomationTrack::Edit_CreateAutomationTrack(Seq_Track *t,AudioChannel *c)
{
	editorid=EDITORTYPE_CREATEAUTOMATIONTRACKS;

	InitForms(FORM_PLAIN1x1);
	ondesktop=true;
	track=t;
	audiochannel=c;

	minwidth=maingui->GetButtonSizeY(45);
	maxheight=minheight=maingui->GetButtonSizeY(10);

	resizeable=true;
	plugindataset=MIDIdataset=syscontrolset=pluginctrlset=MIDIsyscontrolset=false;
	channel=1;
	key=64;

	prevautomationtrack=0;

	learnpluginaudioeffect=plugincontroleffect=0;
	learnpluginaudioobject=0;
	learnandcreate=false;
}

void Edit_CreateAutomationTrack::FreeMemory()
{
}


enum{
	GADGETID_INFO=GADGETID_EDITORBASE,
	GADGETID_INFOBUTTON,
	GADGETID_LEARN,
	GADGETID_LEARNANDCREATE,
	GADGETID_MIDICHANNELI,
	GADGETID_MIDICHANNEL_NR,
	GADGETID_EVENT,
	GADGETID_MIDINOTEI,
	GADGETID_MIDINOTE,
	GADGETID_CREATEMIDIAUTOMATION,
	GADGETID_PLUGINFO,
	GADGETID_PLUGININFOBUTTON,
	GADGETID_CREATEPLUGINAUTOMATION,
	GADGETID_SYSCONTROL,
	GADGETID_SYSCONTROLINFO,
	GADGETID_PLUGINCONTROL,
	GADGETID_PLUGINCONTROLINFO,
	GADGETID_CREATESYSAUTOMATION,
	GADGETID_CREATEPLUGINCONTROLAUTOMATION,

	GADGETID_MIDICONTROL,
	GADGETID_MIDICONTROLINFO,
	GADGETID_CREATESYSMIDIAUTOMATION
};

extern char *controlnames_number[];

void Edit_CreateAutomationTrack::CreatePluginCtrl()
{
	Seq_Song *song;
	TrackHead *th; //?track:audiochannel;

	if(track)
	{
		th=track;
		song=track->song;
	}
	else
	{
		song=audiochannel->audiosystem->song;
		th=audiochannel;
	}

	if(pluginctrlset==true)
	{
		if(mainedit->CheckIfEditOK(song)==true)
		{
			if(th->FindPluginControlAutomationTrack(plugincontroleffect->audioeffect,plugincontroltype)==0)
			{
				switch(plugincontroltype)
				{
				case PSYS_BYPASS:
					th->AddAutomationTrack(new AT_AUDIO_PluginByPass(plugincontroleffect),0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
					break;
				}
			}
		}
	}
}

void Edit_CreateAutomationTrack::CreateMIDISysAutomation()
{
	Seq_Song *song;
	TrackHead *th; //?track:audiochannel;

	if(track)
	{
		th=track;
		song=track->song;
	}
	else
	{
		song=audiochannel->audiosystem->song;
		th=audiochannel;
	}

	if(MIDIsyscontrolset==true)
	{
		if(mainedit->CheckIfEditOK(song)==true)
		{
			if(th->FindMIDISysAutomationTrack(MIDIcontroltype)==0)
				switch(MIDIcontroltype)
			{
				case SYS_MIDIVOLUME:
					th->AddAutomationTrack(&th->MIDIfx.velocity,0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
					break;
			}
		}
	}
}

void Edit_CreateAutomationTrack::CreateSysAutomation()
{
	Seq_Song *song;
	TrackHead *th; //?track:audiochannel;

	if(track)
	{
		th=track;
		song=track->song;
	}
	else
	{
		song=audiochannel->audiosystem->song;
		th=audiochannel;
	}

	if(syscontrolset==true)
	{
		if(mainedit->CheckIfEditOK(song)==true)
		{
			if(th->FindSysAutomationTrack(systype)==0)
			{
				switch(systype)
				{
				case SYS_MUTE:
					th->AddAutomationTrack(&th->io.audioeffects.mute,0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
					break;

				case SYS_SOLO:
					if(track || audiochannel->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL)
						th->AddAutomationTrack(&th->io.audioeffects.solo,0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
					break;

				case SYS_VOLUME:
					th->AddAutomationTrack(&th->io.audioeffects.volume,0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
					break;

				case SYS_PAN:
					th->AddAutomationTrack(&th->io.audioeffects.pan,0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
					break;
				}
			}
		}
	}
}

void Edit_CreateAutomationTrack::CreatePluginAutomation()
{
	Seq_Song *song;
	TrackHead *th; //?track:audiochannel;

	if(track)
	{
		th=track;
		song=track->song;
	}
	else
	{
		song=audiochannel->audiosystem->song;
		th=audiochannel;
	}
	if(plugindataset==true)
	{
		if(mainedit->CheckIfEditOK(song)==true)
		{
			if(th->FindPluginAutomationTrack(learnpluginaudioobject,learnpluginindex)==0)
			{
				th->AddAutomationTrack(new AT_AUDIO_Plugin(learnpluginaudioobject->inserteffect,learnpluginindex),learnpluginindex,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
			}
		}
	}
}

void Edit_CreateAutomationTrack::CreateMIDIAutomation()
{
	Seq_Song *song;
	TrackHead *th; //?track:audiochannel;

	if(track)
	{
		th=track;
		song=track->song;
	}
	else
	{
		song=audiochannel->audiosystem->song;
		th=audiochannel;
	}

	if(MIDIdataset==true)
	{
		if(mainedit->CheckIfEditOK(song)==true)
		{
			switch(data[0]&0xF0)
			{
			case PROGRAMCHANGE:
				if(th->FindMIDIAutomationTrack(PROGRAMCHANGE,0)==0)
					th->AddAutomationTrack(new AT_MIDI_Program,0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
				break;

			case CHANNELPRESSURE:
				if(th->FindMIDIAutomationTrack(CHANNELPRESSURE,0)==0)
					th->AddAutomationTrack(new AT_MIDI_Channelpressure,0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
				break;

			case POLYPRESSURE:
				if(th->FindMIDIAutomationTrack(POLYPRESSURE,key)==0)
					th->AddAutomationTrack(new AT_MIDI_Polypressure(key),0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
				break;

			case PITCHBEND:
				if(th->FindMIDIAutomationTrack(PITCHBEND,0)==0)
					th->AddAutomationTrack(new AT_MIDI_Pitchbend,0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
				break;

			case CONTROLCHANGE:
				if(th->FindMIDIAutomationTrack(CONTROLCHANGE,data[1])==0)
					th->AddAutomationTrack(new AT_MIDI_Control(data[1]),0,new AutomationTrack,prevautomationtrack,ADDSUBTRACK_UNDO|ADDSUBTRACK_CREATESTARTOBJECTS|ADDSUBTRACK_LOCK,track,audiochannel);
				break;
			}
		}
	}
}

void Edit_CreateAutomationTrack::Gadget(guiGadget *g)
{
	Seq_Song *song;
	TrackHead *th; //?track:audiochannel;

	if(track)
	{
		th=track;
		song=track->song;
	}
	else
	{
		song=audiochannel->audiosystem->song;
		th=audiochannel;
	}

	switch(g->gadgetID)
	{
	case GADGETID_CREATEPLUGINCONTROLAUTOMATION:
		CreatePluginCtrl();
		break;

	case GADGETID_CREATESYSAUTOMATION:
		CreateSysAutomation();
		break;

	case GADGETID_CREATEPLUGINAUTOMATION:
		CreatePluginAutomation();
		break;

	case GADGETID_CREATEMIDIAUTOMATION:
		CreateMIDIAutomation();
		break;

	case GADGETID_CREATESYSMIDIAUTOMATION:
		CreateMIDISysAutomation();
		break;

	case GADGETID_MIDINOTE:
		key=data[1]=g->GetPos();
		ShowInfo();
		break;

	case GADGETID_MIDICHANNEL_NR:
		channel=g->GetPos();

		if(MIDIdataset==true)
		{
			data[0]=(data[0]&0xF0)|(channel-1);
			ShowInfo();
		}
		break;

	case GADGETID_LEARNANDCREATE:
		learnandcreate=learnandcreate==true?false:true;

		learn=learnandcreate;

		if(glearn)
			glearn->Toggle(learn);

		if(glearnandcreate)
			glearnandcreate->Toggle(learnandcreate);
		break;

	case GADGETID_LEARN:
		learn=learn==true?false:true;
		if(learnandcreate==true)
		{
			learnandcreate=false;
			learn=true;
		}

		if(glearn)
			glearn->Toggle(learn);

		if(glearnandcreate)
			glearnandcreate->Toggle(learnandcreate);
		break;

	case GADGETID_MIDICONTROL:
		if(DeletePopUpMenu(true))
		{	
			class menu_selMIDIsyscontrol:public guiMenu
			{
			public:
				menu_selMIDIsyscontrol(Edit_CreateAutomationTrack *ed,int t)
				{
					editor=ed;
					type=t;
				}

				void MenuFunction()
				{
					editor->UserSelectMIDIControl(type);
				} //

				Edit_CreateAutomationTrack *editor;
				int type;
			};

			popmenu->AddFMenu("MIDI Volume/Velocity",new menu_selMIDIsyscontrol(this,SYS_MIDIVOLUME));

			ShowPopMenu();
		}
		break;

	case GADGETID_PLUGINCONTROL:
		if(DeletePopUpMenu(true))
		{	
			class menu_selplugincontrol:public guiMenu
			{
			public:
				menu_selplugincontrol(Edit_CreateAutomationTrack *ed,InsertAudioEffect *iae,int t)
				{
					editor=ed;
					insertaudioeffect=iae;
					type=t;
				}

				void MenuFunction()
				{
					editor->UserSelectPluginControl(insertaudioeffect,type);
				} //

				Edit_CreateAutomationTrack *editor;
				InsertAudioEffect *insertaudioeffect;
				int type;
			};

			for(int i=0;i<2;i++)
			{
				InsertAudioEffect *iae=i==1?th->io.audioeffects.FirstInsertAudioEffect():th->io.audioinputeffects.FirstInsertAudioEffect();

				if(iae)
				{
					guiMenu *sub1=popmenu->AddMenu(i==0?"Input":"Output",0);

					if(sub1)
						while(iae)
						{
							guiMenu *sub2=sub1->AddMenu(iae->audioeffect->GetEffectName(),0);

							if(sub2)
							{
								sub2->AddFMenu(plugincontrolnames[PSYS_BYPASS],new menu_selplugincontrol(this,iae,PSYS_BYPASS));
							}

							iae=iae->NextEffect();
						}
				}
			}

			ShowPopMenu();
		}
		break;

	case GADGETID_SYSCONTROL:
		if(DeletePopUpMenu(true))
		{	
			class menu_selsyscontrol:public guiMenu
			{
			public:
				menu_selsyscontrol(Edit_CreateAutomationTrack *ed,int t)
				{
					editor=ed;
					type=t;
				}

				void MenuFunction()
				{
					editor->UserSelectSysControl(type);
				} //

				Edit_CreateAutomationTrack *editor;
				int type;
			};

			// Sys
			popmenu->AddFMenu(syscontrolnames[SYS_MUTE],new menu_selsyscontrol(this,SYS_MUTE));
			popmenu->AddFMenu(syscontrolnames[SYS_SOLO],new menu_selsyscontrol(this,SYS_SOLO));

			popmenu->AddLine(); // - Audio
			popmenu->AddFMenu(syscontrolnames[SYS_VOLUME],new menu_selsyscontrol(this,SYS_VOLUME));
			popmenu->AddFMenu(syscontrolnames[SYS_PAN],new menu_selsyscontrol(this,SYS_PAN));

			ShowPopMenu();
		}
		break;

	case GADGETID_EVENT:

		if(DeletePopUpMenu(true))
		{	
			class menu_selcevent:public guiMenu
			{
			public:
				menu_selcevent(Edit_CreateAutomationTrack *ed,int t,int ctrl)
				{
					editor=ed;
					type=t;
					control=ctrl;
				}

				void MenuFunction()
				{
					editor->UserSelect(type,control);
				} //

				Edit_CreateAutomationTrack *editor;
				int type,control;
			};

			//	popmenu->AddFMenu("Note",new menu_selcevent(this,CREATE_NOTEON),createeventtype==CREATE_NOTEON?true:false);

			if(guiMenu *ctrl=popmenu->AddMenu("ControlChange 0-31",0))
			{
				for(int i=0;i<32;i++)
					ctrl->AddFMenu(controlnames_number[i],new menu_selcevent(this,CONTROLCHANGE,i));
			}

			if(guiMenu *ctrl=popmenu->AddMenu("ControlChange 32-63",0))
			{
				for(int i=32;i<64;i++)
					ctrl->AddFMenu(controlnames_number[i],new menu_selcevent(this,CONTROLCHANGE,i));
			}

			if(guiMenu *ctrl=popmenu->AddMenu("ControlChange 64-95",0))
			{
				for(int i=64;i<96;i++)
					ctrl->AddFMenu(controlnames_number[i],new menu_selcevent(this,CONTROLCHANGE,i));
			}

			if(guiMenu *ctrl=popmenu->AddMenu("ControlChange 96-128",0))
			{
				for(int i=96;i<128;i++)
					ctrl->AddFMenu(controlnames_number[i],new menu_selcevent(this,CONTROLCHANGE,i));
			}
			popmenu->AddFMenu("Pitchbend",new menu_selcevent(this,PITCHBEND,0));
			popmenu->AddFMenu("ProgramChange",new menu_selcevent(this,PROGRAMCHANGE,0));
			popmenu->AddFMenu("ChannelPressure",new menu_selcevent(this,CHANNELPRESSURE,0));
			popmenu->AddFMenu("PolyPressure",new menu_selcevent(this,POLYPRESSURE,0));

			//	popmenu->AddFMenu("SysEx",new menu_selcevent(this,CREATE_SYSEX),createeventtype==CREATE_SYSEX?true:false);

			ShowPopMenu();
		}
		break;
	}

}

void Edit_CreateAutomationTrack::Init()
{
	glist.SelectForm(0,0);

	int w=bitmap.GetTextWidth("WWWWWWWWWWWWWWWWWW");
	int w2=bitmap.GetTextWidth("WWWWWWWW");



	glist.AddButton(-1,-1,w,-1,"MIDI Event",GADGETID_EVENT,MODE_TEXTCENTER|MODE_MENU);
	glist.AddLX();
	glist.AddButton(-1,-1,w2,-1,"MIDI Channel",GADGETID_MIDICHANNELI,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	MIDIchannel=glist.AddNumberButton(-1,-1,w/2,-1,GADGETID_MIDICHANNEL_NR,1,16,1,NUMBER_MIDICHANNEL);
	glist.AddLX();
	MIDInote_i=glist.AddButton(-1,-1,w2,-1,"MIDI Note",GADGETID_MIDINOTEI,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	MIDInote=glist.AddNumberButton(-1,-1,w/2,-1,GADGETID_MIDINOTE,0,127,0,NUMBER_KEYS);
	glist.AddLX();

	if(char *h=mainvar->GenerateString(Cxs[CXS_CREATENEWAUTOTRACK]," MIDI"))
	{
		MIDIcreateautomationtrack=glist.AddButton(-1,-1,-1,-1,h,GADGETID_CREATEMIDIAUTOMATION,MODE_RIGHT|MODE_TEXTCENTER);
		delete h;
	}
	else
		MIDIcreateautomationtrack=0;

	glist.Return();

	glist.AddButton(-1,-1,w,-1,"MIDI Info",GADGETID_INFO,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	info=glist.AddButton(-1,-1,-1,-1,GADGETID_INFOBUTTON,MODE_RIGHT|MODE_INFO,"MIDI Info");
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"MIDI Automation",GADGETID_MIDICONTROL,MODE_MENU|MODE_TEXTCENTER);
	glist.AddLX();
	MIDIcontrolinfo=glist.AddButton(-1,-1,w,-1,GADGETID_MIDICONTROLINFO,MODE_NOMOUSEOVER);

	glist.AddLX();

	if(char *h=mainvar->GenerateString(Cxs[CXS_CREATENEWAUTOTRACK]," MIDI Automation"))
	{
		MIDIsyscreateautomationtrack=glist.AddButton(-1,-1,-1,-1,h,GADGETID_CREATESYSMIDIAUTOMATION,MODE_RIGHT|MODE_TEXTCENTER);
		delete h;
	}
	else
		MIDIsyscreateautomationtrack=0;

	glist.Return();

	int wi=w2+w/2+w2+w/2;

	glist.AddButton(-1,-1,w,-1,"Audio+Sys Automation",GADGETID_SYSCONTROL,MODE_MENU|MODE_TEXTCENTER);
	glist.AddLX();
	syscontrolinfo=glist.AddButton(-1,-1,wi,-1,GADGETID_SYSCONTROLINFO,MODE_NOMOUSEOVER);

	glist.AddLX();

	if(char *h=mainvar->GenerateString(Cxs[CXS_CREATENEWAUTOTRACK]," Audio+Sys Automation"))
	{
		syscreateautomationtrack=glist.AddButton(-1,-1,-1,-1,h,GADGETID_CREATESYSAUTOMATION,MODE_RIGHT|MODE_TEXTCENTER);
		delete h;
	}
	else
		syscreateautomationtrack=0;

	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Plugin Automation",GADGETID_PLUGINCONTROL,MODE_MENU|MODE_TEXTCENTER);
	glist.AddLX();
	plugincontrolinfo=glist.AddButton(-1,-1,wi,-1,GADGETID_PLUGINCONTROLINFO,MODE_NOMOUSEOVER);

	glist.AddLX();

	if(char *h=mainvar->GenerateString(Cxs[CXS_CREATENEWAUTOTRACK]," Plugin Automation"))
	{
		plugincontrolcreateautomationtrack=glist.AddButton(-1,-1,-1,-1,h,GADGETID_CREATEPLUGINCONTROLAUTOMATION,MODE_RIGHT|MODE_TEXTCENTER);
		delete h;
	}
	else
		plugincontrolcreateautomationtrack=0;

	glist.Return();


	glist.AddButton(-1,-1,w,-1,"Plugin Info",GADGETID_PLUGINFO,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	plugininfo=glist.AddButton(-1,-1,wi,-1,GADGETID_PLUGININFOBUTTON,MODE_INFO,"Plugin Info");

	glist.AddLX();

	if(char *h=mainvar->GenerateString(Cxs[CXS_CREATENEWAUTOTRACK]," Plugin"))
	{
		plugincreateautomationtrack=glist.AddButton(-1,-1,-1,-1,h,GADGETID_CREATEPLUGINAUTOMATION,MODE_RIGHT|MODE_TEXTCENTER);
		delete h;
	}
	else
		plugincreateautomationtrack=0;

	glist.Return();
	glearn=glist.AddButton(-1,-1,-1,2*maingui->GetButtonSizeY(),"Learn",GADGETID_LEARN,MODE_TEXTCENTER|MODE_AUTOTOGGLE|MODE_TOGGLE|MODE_RIGHT,Cxs[CXS_LEARN]);
	glist.Return();

	glearnandcreate=glist.AddButton(-1,-1,-1,2*maingui->GetButtonSizeY(),Cxs[CXS_LEARNANDCREATE],GADGETID_LEARNANDCREATE,MODE_TEXTCENTER|MODE_AUTOTOGGLE|MODE_TOGGLE|MODE_RIGHT,Cxs[CXS_LEARNANDCREATE]);
	glist.Return();

	ShowInfo();
	ShowPlugInInfo();
	ShowSysControlInfo();
	ShowPluginControlInfo();
	ShowMIDISysInfo();
}

void Edit_CreateAutomationTrack::UserSelectSysControl(int type)
{
	systype=type;
	syscontrolset=true;
	ShowSysControlInfo();
}

void Edit_CreateAutomationTrack::UserSelectPluginControl(InsertAudioEffect *iae,int type)
{
	plugincontroleffect=iae;
	plugincontroltype=type;
	pluginctrlset=true;

	ShowPluginControlInfo();
}

void Edit_CreateAutomationTrack::UserSelectMIDIControl(int type)
{
	MIDIsyscontrolset=true;
	MIDIcontroltype=type;
	ShowMIDISysInfo();

}

void Edit_CreateAutomationTrack::UserSelect(int type,int ctrl)
{
	data[0]=type|(channel-1);
	data[1]=ctrl;
	MIDIdataset=true;
	ShowInfo();
}

bool Edit_CreateAutomationTrack::CheckIfObjectInside(Object *o)
{
	if(o==track)
		return true;

	if(o==audiochannel)
		return true;

	return false;
}

void Edit_CreateAutomationTrack::RemoveAudioEffect(InsertAudioEffect *iae)
{
	if(iae==learnpluginaudioeffect)
	{
		learnpluginaudioeffect=0;
		learnpluginaudioobject=0;

		plugindataset=false;
		ShowPlugInInfo();
	}

	if(iae==plugincontroleffect)
	{
		plugincontroleffect=0;
		pluginctrlset=false;
		ShowPluginControlInfo();
	}
}

void Edit_CreateAutomationTrack::LearnFromPluginChange(InsertAudioEffect *iae,AudioObject *ao,OSTART time,int index,double value)
{
	plugindataset=true;
	learnpluginaudioeffect=iae;
	learnpluginaudioobject=ao;
	learnplugintime=time;
	learnpluginindex=index;
	learnpluginvalue=value;

	if(learnandcreate==true)
		CreatePluginAutomation();

	ShowPlugInInfo();
}

void Edit_CreateAutomationTrack::LearnFromMIDIEvent(LMIDIEvents *le)
{
	data[0]=(le->data[0]&0xF0)|(channel-1);
	data[1]=le->data[1];
	data[2]=le->data[2];

	switch(data[0]&0xF0)
	{
	case NOTEON:
		key=data[1];
		break;

	case POLYPRESSURE:
		key=data[1];
		break;
	}

	MIDIdataset=true;

	if(learnandcreate==true)
		CreateMIDIAutomation();

	ShowInfo();
}

void Edit_CreateAutomationTrack::ShowPluginControlInfo()
{
	if(pluginctrlset==false)
	{
		if(plugincontrolinfo)
			plugincontrolinfo->Disable();

		if(plugincontrolcreateautomationtrack)
			plugincontrolcreateautomationtrack->Disable();

		return;
	}

	if(plugincontroleffect)
	{
		char *h=mainvar->GenerateString(plugincontrolnames[plugincontroltype],":",plugincontroleffect->audioeffect->GetEffectName());

		if(h)
		{
			if(plugincontrolinfo)
				plugincontrolinfo->ChangeButtonText(h);

			delete h;
		}
	}

	if(plugincontrolcreateautomationtrack)
		plugincontrolcreateautomationtrack->Enable();
}

void Edit_CreateAutomationTrack::ShowMIDISysInfo()
{
	if(MIDIsyscontrolset==false)
	{
		if(MIDIcontrolinfo)
			MIDIcontrolinfo->Disable();

		if(MIDIsyscreateautomationtrack)
			MIDIsyscreateautomationtrack->Disable();

		return;
	}

	if(MIDIcontrolinfo)
		MIDIcontrolinfo->ChangeButtonText(syscontrolnames[MIDIcontroltype]);

	if(MIDIsyscreateautomationtrack)
		MIDIsyscreateautomationtrack->Enable();
}

void Edit_CreateAutomationTrack::ShowSysControlInfo()
{
	if(syscontrolset==false)
	{
		if(syscontrolinfo)
			syscontrolinfo->Disable();

		if(syscreateautomationtrack)
			syscreateautomationtrack->Disable();

		return;
	}

	if(syscontrolinfo)
		syscontrolinfo->ChangeButtonText(syscontrolnames[systype]);

	if(syscreateautomationtrack)
		syscreateautomationtrack->Enable();
}

void Edit_CreateAutomationTrack::ShowPlugInInfo()
{
	if(!learnpluginaudioobject)
		plugindataset=false;

	if(plugindataset==false)
	{
		if(plugininfo)
			plugininfo->Disable();

		if(plugincreateautomationtrack)
			plugincreateautomationtrack->Disable();

		return;
	}

	char *name=learnpluginaudioobject->GetEffectName();
	char *index=learnpluginaudioobject->GetParmName(learnpluginindex);

	if(char *h=mainvar->GenerateString(index," ",name))
	{
		if(plugininfo)
			plugininfo->ChangeButtonText(h);

		delete h;
	}

	if(plugincreateautomationtrack)
		plugincreateautomationtrack->Enable();

}

void Edit_CreateAutomationTrack::ShowInfo()
{
	if(MIDIdataset==false)
	{
		if(MIDInote_i)
			MIDInote_i->Disable();

		if(MIDInote)
			MIDInote->Disable();

		if(MIDIcreateautomationtrack)
			MIDIcreateautomationtrack->Disable();

		return;
	}

	if(MIDIcreateautomationtrack)
		MIDIcreateautomationtrack->Enable();

	switch(data[0]&0xF0)
	{
	case NOTEON:
	case POLYPRESSURE:
		if(MIDInote_i)
			MIDInote_i->Enable();

		if(MIDInote)
			MIDInote->Enable();
		break;

	default:
		if(MIDInote_i)
			MIDInote_i->Disable();

		if(MIDInote)
			MIDInote->Disable();
		break;
	}

	char lastMIDIinstring[255];

	maingui->ConvertMIDI2String(lastMIDIinstring,
		data[0],
		data[1],
		-1);

	if(info)
		info->ChangeButtonText(lastMIDIinstring);
}
