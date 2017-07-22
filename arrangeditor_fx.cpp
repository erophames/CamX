#include "songmain.h"
#include "editor.h"
#include "camxgadgets.h"
#include "editbuffer.h"
#include "imagesdefines.h"
#include "arrangeeditor.h"
#include "arrangeeditor_fx.h"
#include "groove.h"
#include "gui.h"
#include "object_song.h"
#include "object_track.h"
#include "objectpattern.h"
#include "audiofile.h"
#include "mainhelpthread.h"
#include "audiohardware.h"
#include "MIDIhardware.h"
#include "editfunctions.h"
#include "folder.h"
#include "icdobject.h"
#include "arrangeditor_defines.h"
#include "MIDIautomation.h"
#include "audioauto_volume.h"
#include "globalmenus.h"
#include "MIDIoutproc.h"
#include "audiothread.h"
#include "audiochannel.h"
// #include "freeze.h"
#include "audiomanager.h"
#include "arrangeeditor_menus.h"
#include "MIDIPattern.h"
#include "drumevent.h"
#include "languagefiles.h"
#include "sampleeditor.h"
#include "object_project.h"
#include "editdata.h"
#include "audiopattern.h"
#include "gui.h"
#include "semapores.h"

extern char *channelchannelsinfo[],*channelchannelsinfo_short[];
extern int channelschannelsnumber[];


#define TRACKLEFTSTRING " * Out Filter: **"

enum
{
	GADGETID_MIDI=GADGETID_EDITORBASE,
	GADGETID_AUDIO,
	GADGETID_MIDIPattern,
	GADGETID_AUDIOPATTERN,

	// Track
	GADGETID_TRACKNAMENUMBER,
	GADGETID_TRACKNAMESTRING,
	GADGETID_TRACKRECORD,
	GADGETID_TRACKMIDIOUTTYPE,

	GADGETID_GROUPMIDI,
	GADGETID_GROUPMETRO,
	GADGETID_GROUPEVENTFILTER,
	GADGETID_GROUPAUDIO,
	GADGETID_GROUPPATTERN,

	GADGETID_MIDICHANNEL,
	GADGETID_MIDICHANNEL_NR,
	GADGETID_NOTE,
	GADGETID_TRANSPOSE_NR,
	GADGETID_VELOCITY,
	GADGETID_VELOCITY_NR,
	GADGETID_DELAY,
	GADGETID_DELAY_NR,

	GADGETID_SENDBSL_I_CHANNEL,
	GADGETID_SENDBSL_CHANNEL,

	GADGETID_SENDBSL_I_MSB,
	GADGETID_SENDBSL_MSB,

	GADGETID_SENDBSL_I_LSB,
	GADGETID_SENDBSL_LSB,

	GADGETID_SENDPGC_I,
	GADGETID_SENDPGC,

	GADGETID_MIDIIN,
	GADGETID_MIDIIN_STRING,
	GADGETID_MIDIOUT,
	GADGETID_MIDIOUT_STRING,
	GADGETID_MIDITHRU,

	GADGETID_GROUPQUANT,

	GADGETID_AUDIOTHRU,
	GADGETID_AUDIOIO,
	GADGETID_AUDIOIOSTRING,
	GADGETID_AUDIOINPUT,
	GADGETID_AUDIOINPUTSTRING,
	GADGETID_AUDIOOUTPUT,
	GADGETID_AUDIOOUTPUTSTRING,
	GADGETID_AUDIOINMONITOR,
	GADGETID_AUDIOSETFREE,

	GADGET_QUANTIZEONOFF,
	GADGET_QUANTIZEFLAG,
	GADGET_SELECTQUANTIZE,

	GADGETID_HUMANIZE_I,
	GADGETID_HUMANIZE_Q,
	GADGETID_HUMANIZE,

	GADGET_NOTEOFFQUANTIZE,
	GADGET_NOTEOFFFIXLENGTH,
	GADGET_NOTEOFFSELECTFIXLENGTH,

	GADGET_MIDIOUTFILTER,
	GADGET_MIDIOUTFILTER_STRING,
	GADGET_MIDIINFILTER,
	GADGET_MIDIINFILTER_STRING,
	GADGET_VELOORMAINVOLUME,

	// Pattern
	GADGETID_PATTERBNAMESTRING,
	GADGET_MUTE,
	GADGET_LOOP,
	GADGET_LOOPNR,
	GADGET_LOOPENDLESS,
	GADGET_LOOPMODE,
	GADGET_PQEDITOR,
	GADGET_PMIDICHANNEL,
	GADGET_PATTERNCHANNELNR,
	GADGETID_PTRANSPOSE,
	GADGETID_PTRANSPOSE_NR,
	GADGETID_PVELOCITY,
	GADGETID_PVELOCITY_NR,
	GADGETID_ONOFFFADE,
	GADGETID_ONOFFVOLUME,

	// Metro
	GID_MHEAD,
	GID_MCHL_I,
	GID_MCHL,
	GID_MPORT_I,
	GID_MPORT,
	GID_MKEY_I,
	GID_MKEY,
	GID_MVELO_I,
	GID_MVELO,
	GID_MVELOOFF_I,
	GID_MVELOOFF,
	GID_MHHEAD,
	GID_MHCHL_I,
	GID_MHCHL,
	GID_MHPORT_I,
	GID_MHPORT,
	GID_MHKEY_I,
	GID_MHKEY,
	GID_MHVELO_I,
	GID_MHVELO,
	GID_MHVELOOFF_I,
	GID_MHVELOOFF,
	GID_MATYPE_I,
	GID_SENDMETROMIDI,
	GID_SENDMETROAUDIO,
	GID_MDEFAULT
};

void Edit_ArrangeFX::ShowTrackNumber(bool force)
{
	if(g_track)
	{
		Seq_Track *t=GetTrack();

		if(t)
		{
			int tn;

			if(t->ismetrotrack==true)
			{
				tn=t->GetIndex();
			}
			else
				tn=t->GetTrackIndex();

			if(force==true || g_track->disabled==true || tracknr!=tn)
			{
				tracknr=tn;
				char h[NUMBERSTRINGLEN];

				g_track->ChangeButtonText(mainvar->ConvertIntToChar(tracknr+1,h));
			}
		}
		else
		{
			g_track->Disable();
		}
	}
}

void Edit_ArrangeFX::ShowTrackEventType(bool force)
{
	if(g_tracktype)
	{
		Seq_Track *t=GetTrack();

		if(t)
		{
			if(force==true || g_tracktype->disabled==true || t->MIDItypesetauto!=MIDItypesetauto || t->MIDItype!=MIDItype)
			{
				MIDItypesetauto=t->MIDItypesetauto;
				MIDItype=t->MIDItype;

				if(MIDItypesetauto==true)
					g_tracktype->ChangeButtonText("Events>Auto");
				else
				{
					switch(MIDItype)
					{
					case Seq_Track::OUTPUTTYPE_MIDI:
						g_tracktype->ChangeButtonText("Events>MIDI");
						break;

					case Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT:
						g_tracktype->ChangeButtonText("Events>Plugins");
						break;

						/*
						case Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENTANDMIDI:
						g_tracktype->ChangeButtonText("Events:MIDI+Plugins");
						break;
						*/

					}

				}


			}
		}
		else
		{
			g_tracktype->Disable();
		}
	}
}

void Edit_ArrangeFX::ShowRecordType(bool force)
{
	if(g_trackrecord)
	{
		Seq_Track *t=WindowSong()->GetFocusTrack();

		if(t)
		{
			if(force==true || g_trackrecord->disabled==true || recordtracktype!=t->recordtracktype)
			{
				switch(recordtracktype=t->recordtracktype)
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
		else
		{
			g_trackrecord->Disable();
		}
	}

}

void Edit_ArrangeFX::ShowTrackName(bool force)
{
	if(g_trackstring)
	{
		Seq_Track *t=GetTrack();

		if(t)
			g_trackstring->CheckString(t->GetName(),force);
		else
			g_trackstring->Disable();
	}
}

Edit_ArrangeFX::Edit_ArrangeFX(Edit_Arrange *ar)
{
	editorid=EDITORTYPE_ARRANGEFX;
	InitForms(FORM_PLAIN1x1);

	autovscroll=true;
	isstatic=true;
	song=ar->WindowSong();

	trackMIDI.editor=
		trackmetro.editor=
		trackaudio.editor=
		trackquant.editor=
		mixer.editor=
		pattern.editor=this;
}

void Edit_ArrangeFX::Gadget(guiGadget *g)
{
	Seq_Track *t=GetTrack();

	if(!t)
		return;

	switch(g->gadgetID)
	{
	case GADGETID_TRACKMIDIOUTTYPE:
		{
			t->EditMIDIEventOutput(this);
		}
		break;

	case GADGETID_TRACKRECORD:
		{
			maingui->EditTrackRecordType(this,t);
		}
		break;

	case GADGETID_TRACKNAMESTRING:
		{
			t->SetName(g->string);
			t->ShowTrackName(0);
		}
		break;
	}
}

Seq_Track *Edit_ArrangeFX::GetTrack()
{
	switch(WindowSong()->GetFocusType())
	{
	case Seq_Song::FOCUSTYPE_METRO:
		return WindowSong()->GetFocusMetroTrack();
		break;

	default:
		return WindowSong()->GetFocusTrack();
		break;
	}
}

void Edit_ArrangeFX::RefreshRealtime_Slow()
{
	ShowTrackNumber(false);
	ShowTrackName(false);
	ShowRecordType(false);
	ShowTrackEventType(false);

	if(gr_quant)
		gr_quant->SetButtonQuickOn(GetTrack()?GetTrack()->t_trackeffects.quantizeeffect.IsInUse():false);

	if(gr_MIDI)
		gr_MIDI->SetButtonQuickOn(GetTrack()?GetTrack()->t_trackeffects.IsMIDIInOrOutFilter():false);

	// FX Show Flag
	int flag=0;

	if(gr_MIDI)
	{
		if(gr_MIDI->mode&MODE_TOGGLED)
			flag|=FX_SHOWMIDI;
	}

	if(gr_audio)
	{
		if(gr_audio->mode&MODE_TOGGLED)
			flag|=FX_SHOWAUDIO;
	}

	if(gr_quant)
	{
		if(gr_quant->mode&MODE_TOGGLED)
			flag|=FX_SHOWQUANT;
	}

	if(gr_pattern)
	{
		if(gr_pattern->mode&MODE_TOGGLED)
			flag|=FX_SHOWPATTERN;
	}

	if(gr_metro)
	{
		if(gr_metro->mode&MODE_TOGGLED)
			flag|=FX_SHOWMETRO;
	}

	if(gr_mixer)
	{
		if(gr_mixer->mode&MODE_TOGGLED)
			flag|=FX_SHOWMIXER;
	}

	WindowSong()->fxshowflag=flag;
}

void Edit_ArrangeFX::RefreshMIDI(Seq_Track *track)
{
	if(track==GetTrack())
	{
		trackMIDI.ShowTrackMIDIInput(true);
		trackMIDI.ShowTrackMIDIOutput(true);
	}
}

void Edit_ArrangeFX::InitGadgets()
{
	glist.SelectForm(0,0);

	int w=bitmap.GetTextWidth("PATTERN");

	g_track=glist.AddButton(-1,-1,w,-1,GADGETID_TRACKNAMENUMBER);
	ShowTrackNumber(true);
	glist.AddLX();
	g_trackstring=glist.AddString(-1,-1,-1,-1,GADGETID_TRACKNAMESTRING,MODE_RIGHT,0,0);
	ShowTrackName(true);
	glist.Return();

	g_trackrecord=glist.AddButton(-1,-1,-1,-1,GADGETID_TRACKRECORD,MODE_RIGHT|MODE_MENU,Cxs[CXS_MIDIAUDIORECORD]);
	ShowRecordType(true);
	glist.Return();

	g_tracktype=glist.AddButton(-1,-1,-1,-1,GADGETID_TRACKMIDIOUTTYPE,MODE_RIGHT|MODE_MENU,Cxs[CXS_MIDIOUTPUT_TYPE]);
	ShowTrackEventType(true);
	glist.Return();

	// MIDI
	gr_MIDI=glist.AddButton(-1,-1,w,-1,"MIDI",GADGETID_GROUPMIDI,song->GetFXFlag(FX_SHOWMIDI)==true?MODE_RIGHT|MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_RIGHT|MODE_GROUP|MODE_TOGGLE);

	if(gr_MIDI)
		gr_MIDI->SetButtonQuickText("F",GetTrack()?GetTrack()->t_trackeffects.IsMIDIInOrOutFilter():false);

	glist.Return();
	glist.OpenGroup(&trackMIDI,gr_MIDI,MODE_RIGHT,WindowSong());
	glist.Return();

	/*
	// Event Filter
	guiGadget *gr_filter=glist.AddButton(-1,-1,w,-1,"MIDI Event Filter",GADGETID_GROUPEVENTFILTER,showMIDIfilter==true?MODE_RIGHT|MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_RIGHT|MODE_GROUP|MODE_TOGGLE);
	glist.Return();
	glist.OpenGroup(&MIDIfilter,gr_filter,MODE_RIGHT,WindowSong());
	glist.Return();
	*/

	// Audio
	gr_audio=glist.AddButton(-1,-1,w,-1,"Audio",GADGETID_GROUPAUDIO,song->GetFXFlag(FX_SHOWAUDIO)==true?MODE_RIGHT|MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_RIGHT|MODE_GROUP|MODE_TOGGLE);
	glist.Return();
	glist.OpenGroup(&trackaudio,gr_audio,MODE_RIGHT,WindowSong());
	glist.Return();

	// Quantize
	gr_quant=glist.AddButton(-1,-1,w,-1,Cxs[CXS_QUANTIZE],GADGETID_GROUPQUANT,song->GetFXFlag(FX_SHOWQUANT)==true?MODE_RIGHT|MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_RIGHT|MODE_GROUP|MODE_TOGGLE);
	glist.Return();

	if(gr_quant)
	{
		gr_quant->SetButtonQuickText("Q",GetTrack()?GetTrack()->t_trackeffects.quantizeeffect.usequantize:false);
	}

	trackquant.song=WindowSong();
	glist.OpenGroup(&trackquant,gr_quant,MODE_RIGHT,WindowSong());
	glist.Return();

	gr_pattern=glist.AddButton(-1,-1,w,-1,"Pattern",GADGETID_GROUPPATTERN,song->GetFXFlag(FX_SHOWPATTERN)==true?MODE_RIGHT|MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_RIGHT|MODE_GROUP|MODE_TOGGLE);
	glist.Return();
	pattern.song=WindowSong();
	glist.OpenGroup(&pattern,gr_pattern,MODE_RIGHT,WindowSong());
	glist.Return();

	// Metro
	gr_metro=glist.AddButton(-1,-1,w,-1,Cxs[CXS_METRONOME],GADGETID_GROUPMETRO,song->GetFXFlag(FX_SHOWMETRO)==true?MODE_RIGHT|MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_RIGHT|MODE_GROUP|MODE_TOGGLE);
	glist.Return();
	glist.OpenGroup(&trackmetro,gr_metro,MODE_RIGHT,WindowSong());
	glist.Return();

	// Mixer
	gr_mixer=glist.AddButton(-1,-1,w,-1,"Mixer",GADGETID_GROUPAUDIO,song->GetFXFlag(FX_SHOWMIXER)==true?MODE_RIGHT|MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_RIGHT|MODE_GROUP|MODE_TOGGLE);
	glist.Return();
	mixer.song=WindowSong();
	glist.OpenGroup(&mixer,gr_mixer,MODE_RIGHT|MODE_BOTTOM,WindowSong());
}

void Edit_ArrangeFX::Init()
{
	autovscroll=true;
	InitGadgets();
}

// Track Metro

void Edit_ArrangeFX_Metronom::SendMetro(int hi)
{
	Seq_Track *t=editor->GetTrack();
	if(!t)return;

	if(t->ismetrotrack==false)
		return;

	Note note;
	MIDIOutputDevice *device=0;

	if(hi==1){

		note.status=NOTEON|(t->metrochl_m-1);
		note.key=t->metrokey_m;
		note.velocity=t->metrovelo_m;
		note.velocityoff=t->metrovelooff_m;
		device=mainMIDI->MIDIoutports[t->metroport_m].outputdevice;
	}
	else{

		note.status=NOTEON|(t->metrochl_b-1);
		note.key=t->metrokey_b;
		note.velocity=t->metrovelo_b;
		note.velocityoff=t->metrovelooff_b;
		device=mainMIDI->MIDIoutports[t->metroport_b].outputdevice;
	}

	note.ostart=0;
	note.off.ostart=TICK32nd; // Metro Click Length

	note.SendToDevicePlaybackUser(device,WindowSong(),Seq_Event::STD_CREATEREALEVENT);
}

void Edit_ArrangeFX_Metronom::Gadget(guiGadget *g)
{
	Seq_Track *t=editor->GetTrack();
	if(!t)return;
	if(t->ismetrotrack==false)return;

	switch(g->gadgetID)
	{
	case GID_SENDMETROMIDI:
		t->metrosendtoMIDI=t->metrosendtoMIDI==true?false:true;
		break;

	case GID_SENDMETROAUDIO:
		t->metrosendtoaudio=t->metrosendtoaudio==true?false:true;
		break;

	case GID_MDEFAULT:
		{
			t->SetDefaultMetroSettings();
			ShowMetro();
		}
		break;

	case GID_MHEAD:
	case GID_MHHEAD:
		SendMetro(g->gadgetID==GID_MHEAD?0:1);
		break;

	case GID_MPORT:
	case GID_MHPORT:
		{
			DeletePopUpMenu(true);

			char nr[NUMBERSTRINGLEN];

			if(popmenu)
			{	
				popmenu->AddMenu(g->gadgetID==GID_MPORT?"MIDI Port (b)":"MIDI Port",0);
				popmenu->AddLine();

				class menu_addgroup:public guiMenu
				{
				public:
					menu_addgroup(Edit_ArrangeFX_Metronom *ed,Seq_Track *tr,int t,int p)
					{
						editor=ed;
						track=tr;

						to=t;
						port=p;
					}

					void MenuFunction()
					{
						if(to==0)
							track->metroport_b=port;
						else
							track->metroport_m=port;

						//editor->SendMetro(to);
					}

					Edit_ArrangeFX_Metronom *editor;

					Seq_Track *track;
					int to,port;
				};

				for(int i=0;i<MAXMIDIPORTS;i++)
				{
					if(mainMIDI->MIDIoutports[i].visible==true)
					{
						if(char *h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(i+1,nr),":",mainMIDI->MIDIoutports[i].GetName()))
						{
							if(g->gadgetID==GID_MPORT)
								popmenu->AddFMenu(h,new menu_addgroup(this,t,0,i),t->metroport_b==i?true:false);
							else
								popmenu->AddFMenu(h,new menu_addgroup(this,t,1,i),t->metroport_m==i?true:false);

							delete h;
						}

					}
				}

				ShowPopMenu();
			}
		}
		break;

	case GID_MCHL:
		t->metrochl_b=g->GetPos();
		SendMetro(0);
		break;

	case GID_MKEY:
		t->metrokey_b=g->GetPos();
		SendMetro(0);
		break;

	case GID_MVELO:
		t->metrovelo_b=g->GetPos();
		SendMetro(0);
		break;

	case GID_MVELOOFF:
		t->metrovelo_m=g->GetPos();
		SendMetro(0);
		break;

	case GID_MHCHL:
		t->metrochl_m=g->GetPos();
		SendMetro(1);
		break;

	case GID_MHKEY:
		t->metrokey_m=g->GetPos();
		SendMetro(1);
		break;

	case GID_MHVELO:
		t->metrovelo_m=g->GetPos();
		SendMetro(1);
		break;

	case GID_MHVELOOFF:
		t->metrovelooff_m=g->GetPos();
		SendMetro(1);
		break;
	}
}

void Edit_ArrangeFX_Metronom::ShowMetro()
{
	Seq_Track *t=editor->GetTrack();

	if(t && t->ismetrotrack==false)
		t=0;

	if(t)
	{
		char nrs[NUMBERSTRINGLEN];
		char *h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(t->metroport_b+1,nrs),":",mainMIDI->MIDIoutports[t->metroport_b].GetName());

		if(mport)
			mport->ChangeButtonText(h);

		if(h)
			delete h;

		h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(t->metroport_m+1,nrs),":",mainMIDI->MIDIoutports[t->metroport_m].GetName());

		if(mhport)
			mhport->ChangeButtonText(h);

		if(h)
			delete h;
	}
	else
	{
		glist.Disable(mport);
		glist.Disable(mhport);
	}


	if(mchl_b)
	{
		if(t)
			mchl_b->SetPos(t->metrochl_b);
		else
			glist.Disable(mchl_b);
	}


	if(mkey_b)
	{
		if(t)
			mkey_b->SetPos(t->metrokey_b);
		else
			glist.Disable(mkey_b);
	}

	if(mvelo_b)
	{
		if(t)
			mvelo_b->SetPos(t->metrovelo_b);
		else
			glist.Disable(mvelo_b);
	}

	if(mvelooff_b)
	{
		if(t)
			mvelooff_b->SetPos(t->metrovelooff_b);
		else
			glist.Disable(mvelooff_b);
	}

	// Hi
	if(mchl_m)
	{
		if(t)
			mchl_m->SetPos(t->metrochl_m);
		else
			glist.Disable(mchl_m);
	}

	if(mkey_m)
	{
		if(t)
			mkey_m->SetPos(t->metrokey_m);
		else
			glist.Disable(mkey_m);
	}

	if(mvelo_m)
	{
		if(t)
			mvelo_m->SetPos(t->metrovelo_m);
		else
			glist.Disable(mvelo_m);
	}

	if(mvelooff_m)
	{
		if(t)
			mvelooff_m->SetPos(t->metrovelooff_m);
		else
			glist.Disable(mvelooff_m);
	}

	if(sendtoaudio)
	{
		if(t)
			sendtoaudio->SetCheckBox(t->metrosendtoaudio);
		else
			glist.Disable(sendtoaudio);
	}

	if(sendtoMIDI)
	{
		if(t)
			sendtoMIDI->SetCheckBox(t->metrosendtoMIDI);
		else
			glist.Disable(sendtoMIDI);
	}
}

void Edit_ArrangeFX_Metronom::RefreshRealtime_Slow()
{
	ShowMetro();
}

int Edit_ArrangeFX_Metronom::GetInitHeight()
{
	return maingui->GetButtonSizeY(17);
}

void Edit_ArrangeFX_Metronom::Init()
{
#define DEFSIZE bitmap.GetTextWidth("WWWWWWWWW")

	int w=bitmap.GetTextWidth(TRACKLEFTSTRING);

	glist.SelectForm(0,0);

	// Metro
	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME]," ",Cxs[CXS_BEAT]))
	{
		glist.AddButton(-1,-1,-1,2*maingui->GetButtonSizeY(),h,GID_MHEAD,MODE_TEXTCENTER|MODE_RIGHT,"Click -> Play");
		delete h;
	}

	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Chl (b)",GID_MCHL_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mchl_b=glist.AddNumberButton(-1,-1,-1,-1,GID_MCHL,1,16,mainsettings->defaultmetrochl_b,NUMBER_MIDICHANNEL,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Port (b)",GID_MPORT_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mport=glist.AddButton(-1,-1,-1,-1,GID_MPORT,MODE_MENU|MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Key (b)",GID_MKEY_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mkey_b=glist.AddNumberButton(-1,-1,-1,-1,GID_MKEY,0,127,0,NUMBER_KEYS,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Velocity (b)",GID_MVELO_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mvelo_b=glist.AddNumberButton(-1,-1,-1,-1,GID_MVELO,1,127,0,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Velocity Off (b)",GID_MVELOOFF_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mvelooff_b=glist.AddNumberButton(-1,-1,-1,-1,GID_MVELOOFF,0,127,0,NUMBER_INTEGER,MODE_RIGHT);

	// Metro Hi
	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME]," ",Cxs[CXS_MEASURE]))
	{
		glist.AddButton(-1,-1,-1,2*maingui->GetButtonSizeY(),h,GID_MHHEAD,MODE_TEXTCENTER|MODE_RIGHT,"Click -> Play");
		delete h;
	}
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Chl",GID_MHCHL_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mchl_m=glist.AddNumberButton(-1,-1,-1,-1,GID_MHCHL,1,16,1,NUMBER_MIDICHANNEL,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Port",GID_MHPORT_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mhport=glist.AddButton(-1,-1,-1,-1,GID_MHPORT,MODE_MENU|MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Key",GID_MHKEY_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mkey_m=glist.AddNumberButton(-1,-1,-1,-1,GID_MHKEY,0,127,0,NUMBER_KEYS,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Velocity",GID_MHVELO_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mvelo_m=glist.AddNumberButton(-1,-1,-1,-1,GID_MHVELO,1,127,1,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/6);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Velocity Off",GID_MHVELOOFF_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mvelooff_m=glist.AddNumberButton(-1,-1,-1,-1,GID_MHVELOOFF,0,127,0,NUMBER_INTEGER,MODE_RIGHT);

	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME],"->","MIDI"))
	{
		sendtoMIDI=glist.AddCheckBox(-1,-1,-1,-1,GID_SENDMETROMIDI,MODE_RIGHT,h);
		delete h;
	}

	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME],"->","AUDIO"))
	{
		sendtoaudio=glist.AddCheckBox(-1,-1,-1,-1,GID_SENDMETROAUDIO,MODE_RIGHT,h);
	}

	glist.Return();
	glist.AddButton(-1,-1,-1,-1,Cxs[CXS_USEDEFAULTMETRO],GID_MDEFAULT,MODE_RIGHT);

	ShowMetro();
}

// Track MIDI

void Edit_ArrangeFX_MIDI::ShowTrackTranspose(bool force)
{
	if(tracktranspose)
	{
		Seq_Track *t=editor->GetTrack();

		if(!t)
		{
			tracktranspose->Disable();
		}
		else
			if(force==true || tracktranspose->disabled==true || tracktranspose_nr!=t->t_trackeffects.GetTranspose_NoParent())
			{
				tracktranspose_nr=t->t_trackeffects.GetTranspose_NoParent();
				tracktranspose->SetPos(tracktranspose_nr);
			}
	}
}

void Edit_ArrangeFX_MIDI::ShowTrackVelocity(bool force)
{
	if(trackvelocity)
	{
		Seq_Track *t=editor->GetTrack();

		if(!t)
		{
			trackvelocity->Disable();
		}
		else
		{
			if(force==true || trackvelocity->disabled==true || trackvelocity_nr!=t->t_trackeffects.GetVelocity_NoParent())
			{
				trackvelocity_nr=t->t_trackeffects.GetVelocity_NoParent();

				trackvelocity->SetPos(trackvelocity_nr);
			}
		}
	}
}

void Edit_ArrangeFX_MIDI::ShowTrackProgramAndBankSelect(bool force)
{
	if(
		trackbsel_i_channel && trackbsel_channel &&
		trackbsel_msb && trackbsel_i_msb && 
		trackbsel_lsb && trackbsel_i_lsb &&
		trackprgsel && trackpsel_i
		)
	{
		Seq_Track *t=editor->GetTrack();

		if(t && t->ismetrotrack==true)
			t=0;

		if(!t)
		{
			trackbsel_i_channel->Disable();
			trackbsel_channel->Disable();

			trackbsel_i_msb->Disable();
			trackbsel_msb->Disable();

			trackbsel_i_lsb->Disable();
			trackbsel_lsb->Disable();

			trackpsel_i->Disable();
			trackprgsel->Disable();
		}
		else
		{

			if(force==true || trackbsel_i_channel->disabled==true ||
				t->t_trackeffects.MIDIprogram.Compare(&compareMIDIprogram)==false)
			{
				t->t_trackeffects.MIDIprogram.Clone(&compareMIDIprogram);

				trackbsel_i_channel->Enable();

				// Channel
				trackbsel_channel->SetPos(compareMIDIprogram.channel+1);

				// Bank Select MSB
				trackbsel_i_msb->Toggle(compareMIDIprogram.usebank_msb);

				if(compareMIDIprogram.usebank_msb==true)
					trackbsel_msb->SetPos(compareMIDIprogram.MIDIBank_msb);
				else
					trackbsel_msb->Disable();

				// Bank Select LSB
				trackbsel_i_lsb->Toggle(compareMIDIprogram.usebank_lsb);

				if(compareMIDIprogram.usebank_lsb==true)
					trackbsel_lsb->SetPos(compareMIDIprogram.MIDIBank_lsb);
				else
					trackbsel_lsb->Disable();

				// Program Change
				trackpsel_i->Toggle(compareMIDIprogram.useprogramchange);

				if(compareMIDIprogram.useprogramchange==true)
					trackprgsel->SetPos(compareMIDIprogram.MIDIProgram);
				else
					trackprgsel->Disable();
			}
		}

	}
}

void Edit_ArrangeFX_MIDI::ShowTrackMIDIChannel(bool force)
{
	if(trackchannel)
	{
		Seq_Track *t=editor->GetTrack();

		if(!t)
		{
			trackchannel->Disable();
		}
		else
			if(force==true || trackchannel->disabled==true || trackchannel_nr!=t->t_trackeffects.GetChannel_NoParent())
			{
				trackchannel_nr=t->t_trackeffects.GetChannel_NoParent();
				trackchannel->SetPos(trackchannel_nr);
			}
	}
}

void Edit_ArrangeFX_MIDI::ShowTrackMIDIInput(bool force)
{
	if(trackMIDIinput)
	{
		Seq_Track *t=editor->GetTrack();

		if(!t)
		{
			trackMIDIinput->Disable();
			return;
		}

		if(char *h=t->GetMIDIInputString(false))
		{
			if(force==true || trackMIDIinput->disabled==true || trackMIDIinput->text==0 || strcmp(trackMIDIinput->text,h)!=0)
				trackMIDIinput->ChangeButtonText(h);

			delete h;
		}
	}
}


void Edit_ArrangeFX_MIDI::ShowTrackMIDIOutput(bool force)
{
	if(trackMIDIoutput)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			trackMIDIoutput->Disable();
			return;
		}

		if(char *h=t->GetMIDIOutputString(false))
		{
			if(force==true || trackMIDIoutput->disabled==true || trackMIDIoutput->text==0 || strcmp(trackMIDIoutput->text,h)!=0)
				trackMIDIoutput->ChangeButtonText(h);

			delete h;
		}
	}
}

void Edit_ArrangeFX_MIDI::ShowTrackMIDIDelayType(bool force)
{
	if(trackdelaytype)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			trackdelaytype->Disable();
			return;
		}

		if(force==true || trackdelaytype->disabled==true || delaytype!=t->t_trackeffects.delaytype)
		{
			delaytype=t->t_trackeffects.delaytype;

			switch(delaytype)
			{
			case DELAYTYPE_SAMPLES:
				trackdelaytype->ChangeButtonText("Delay/Samples");

				trackdelay->SetPosType(NUMBER_INTEGER);
				trackdelay->SetFromTo(-mainaudio->GetGlobalSampleRate()/2,mainaudio->GetGlobalSampleRate()/2);
				break;

			case DELAYTYPE_MS:
				trackdelaytype->ChangeButtonText("Delay/ms");

				trackdelay->SetPosType(NUMBER_00);
				trackdelay->SetFromTo(-500,500);
				break;
			}
		}
	}
}

void Edit_ArrangeFX_MIDI::ShowTrackMIDIDelay(bool force)
{
	if(trackdelay)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			trackdelay->Disable();
			return;
		}

		if(force==true || trackdelay->disabled==true || trackdelay_nr!=t->t_trackeffects.ndelay)
		{
			trackdelay_nr=t->t_trackeffects.ndelay;

			switch(t->t_trackeffects.delaytype)
			{
			case DELAYTYPE_SAMPLES:
				{
					//int h=maintimer->ConvertSysTimeToMsConvertInternRateToPPQ(t->t_trackeffects.delay);
					trackdelay->SetPos((int)mainaudio->ConvertInternToExternSampleRate(trackdelay_nr));
				}
				break;

			case DELAYTYPE_MS:
				{
					double h=mainaudio->ConvertSamplesToMs(mainaudio->ConvertInternToExternSampleRate(trackdelay_nr));
					trackdelay->SetPos(h);
				}
				break;
			}
		}

	}
}

void Edit_ArrangeFX_MIDI::ShowTrackMIDIThru(bool force)
{
	if(trackMIDIthru)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			trackMIDIthru->Disable();
			return;
		}

		if(force==true || trackMIDIthru->disabled==true || t->t_trackeffects.MIDIthru!=MIDIthru)
		{
			MIDIthru=t->t_trackeffects.MIDIthru;
			trackMIDIthru->Toggle(MIDIthru);
		}
	}
}

void Edit_ArrangeFX_MIDI::Gadget(guiGadget *g)
{
	Seq_Track *t=editor->GetTrack();

	if(!t)return;

	switch(g->gadgetID)
	{
	case GADGETID_SENDBSL_CHANNEL:
		t->t_trackeffects.MIDIprogram.channel=g->GetPos()-1; // 0-15
		break;

	case GADGETID_SENDBSL_I_MSB:
		t->t_trackeffects.MIDIprogram.usebank_msb=t->t_trackeffects.MIDIprogram.usebank_msb==true?false:true;
		ShowTrackProgramAndBankSelect(true);
		break;

	case GADGETID_SENDBSL_MSB:
		t->t_trackeffects.MIDIprogram.MIDIBank_msb=g->GetPos();
		break;

	case GADGETID_SENDBSL_I_LSB:
		t->t_trackeffects.MIDIprogram.usebank_lsb=t->t_trackeffects.MIDIprogram.usebank_lsb==true?false:true;
		ShowTrackProgramAndBankSelect(true);
		break;

	case GADGETID_SENDBSL_LSB:
		t->t_trackeffects.MIDIprogram.MIDIBank_lsb=g->GetPos();
		break;

	case GADGETID_SENDPGC_I:
		t->t_trackeffects.MIDIprogram.useprogramchange=t->t_trackeffects.MIDIprogram.useprogramchange==true?false:true;
		ShowTrackProgramAndBankSelect(true);
		break;

	case GADGETID_SENDPGC:
		t->t_trackeffects.MIDIprogram.MIDIProgram=g->GetPos();
		break;


	case GADGETID_MIDIIN:
	case GADGETID_MIDIIN_STRING:
		maingui->EditTrackMIDIInput(this,t);
		break;

	case GADGETID_MIDIOUT:
	case GADGETID_MIDIOUT_STRING:
		maingui->EditTrackMIDIOutput(this,t);
		break;

	case GADGETID_MIDITHRU:
		t->t_trackeffects.MIDIthru=t->t_trackeffects.MIDIthru==true?false:true;
		ShowTrackMIDIThru(true);
		break;

	case GADGETID_DELAY:
		switch(t->t_trackeffects.delaytype)
		{
		case DELAYTYPE_SAMPLES:
			t->t_trackeffects.delaytype=DELAYTYPE_MS;
			break;

		case DELAYTYPE_MS:
			t->t_trackeffects.delaytype=DELAYTYPE_SAMPLES;
			break;
		}

		ShowTrackMIDIDelayType(true);
		ShowTrackMIDIDelay(true);
		break;

	case GADGETID_DELAY_NR:
		{
			switch(t->t_trackeffects.delaytype)
			{
			case DELAYTYPE_SAMPLES:
				t->t_trackeffects.SetDelay(mainaudio->ConvertExternToInternSampleRate(g->GetPos()));
				break;

			case DELAYTYPE_MS:
				t->t_trackeffects.SetDelay(mainaudio->ConvertExternToInternSampleRate(mainaudio->ConvertMsToSamples(g->GetPos())));
				break;
			}			
		}
		break;

	case GADGETID_MIDICHANNEL_NR:
		{
			t->t_trackeffects.SetChannel(trackchannel_nr=g->GetPos());
		}
		break;

	case GADGETID_TRANSPOSE_NR:
		{
			t->t_trackeffects.SetTranspose(tracktranspose_nr=g->GetPos());
		}
		break;

	case GADGETID_VELOCITY_NR:
		{
			if(t->CanAutomationObjectBeChanged(&t->MIDIfx.velocity,0,0)==true)
				t->t_trackeffects.SetVelocity(trackvelocity_nr=g->GetPos(),t->song->GetSongPosition());
		}
		break;

	case GADGET_MIDIOUTFILTER:
		t->t_trackeffects.filter.bypass=t->t_trackeffects.filter.bypass==true?false:true;
		break;

	case GADGET_MIDIOUTFILTER_STRING:
		maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,WindowSong(),0,0,0,&t->t_trackeffects.filter,0);
		break;

	case GADGET_MIDIINFILTER:
		t->t_trackeffects.inputfilter.bypass=t->t_trackeffects.inputfilter.bypass==true?false:true;
		break;

	case GADGET_MIDIINFILTER_STRING:
		maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,WindowSong(),0,0,0,&t->t_trackeffects.inputfilter,0);
		break;
	}
}

void Edit_ArrangeFX_MIDI::Refresh(bool t)
{
	ShowTrackMIDIChannel(t);
	ShowTrackTranspose(t);
	ShowTrackVelocity(t);
	ShowTrackMIDIThru(t);

	ShowTrackMIDIInput(t);
	ShowTrackMIDIOutput(t);

	ShowTrackMIDIDelayType(t);
	ShowTrackMIDIDelay(t);

	ShowInFilter(t);
	ShowOutFilter(t);
	ShowTrackProgramAndBankSelect(t);
}

void Edit_ArrangeFX_MIDI::RefreshRealtime_Slow()
{
	Refresh(false);	
}

int Edit_ArrangeFX_MIDI::GetInitHeight()
{
	return maingui->GetButtonSizeY(13);
}

void Edit_ArrangeFX_MIDI::Init()
{
	int w=bitmap.GetTextWidth(TRACKLEFTSTRING);

	glist.SelectForm(0,0);

	glist.AddButton(-1,-1,w,-1,"Channel",GADGETID_MIDICHANNEL,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	trackchannel=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_MIDICHANNEL_NR,0,16,1,NUMBER_MIDICHANNEL,MODE_RIGHT);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Transpose",GADGETID_NOTE,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	tracktranspose=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_TRANSPOSE_NR,-127,127,0,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Velocity",GADGETID_VELOCITY,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	trackvelocity=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_VELOCITY_NR,-127,127,0,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	trackdelaytype=glist.AddButton(-1,-1,w,-1,0,GADGETID_DELAY,MODE_ADDDPOINT,0);
	glist.AddLX();
	trackdelay=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_DELAY_NR,-127,127,1,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	// Send Bank/ProgramChange
	trackbsel_i_channel=glist.AddButton(-1,-1,2*w,-1,"BSEL/Prog/Channel",GADGETID_SENDBSL_I_CHANNEL,MODE_NOMOUSEOVER|MODE_ADDDPOINT,Cxs[CXS_USEBANKSELECTCHANNEL]);
	glist.AddLX();
	trackbsel_channel=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_SENDBSL_CHANNEL,1,16,1,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	trackbsel_i_msb=glist.AddButton(-1,-1,w,-1,"BankSel MSB",GADGETID_SENDBSL_I_MSB,MODE_TOGGLE,Cxs[CXS_SENDBANKSELECT]);
	glist.AddLX();
	trackbsel_msb=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_SENDBSL_MSB,0,127,1,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	trackbsel_i_lsb=glist.AddButton(-1,-1,w,-1,"BankSel LSB",GADGETID_SENDBSL_I_LSB,MODE_TOGGLE,Cxs[CXS_SENDBANKSELECT]);
	glist.AddLX();
	trackbsel_lsb=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_SENDBSL_LSB,0,127,1,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	trackpsel_i=glist.AddButton(-1,-1,w,-1,"Prg Chg",GADGETID_SENDPGC_I,MODE_TOGGLE,Cxs[CXS_SENDPROGRAMCHANGE]);
	glist.AddLX();
	trackprgsel=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_SENDPGC,0,127,1,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	trackMIDIthru=glist.AddButton(-1,-1,-1,-1,"MIDI Thru",GADGETID_MIDITHRU,MODE_RIGHT|MODE_TOGGLE,Cxs[CXS_MIDITHRUINFO]);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Input",GADGETID_MIDIIN,MODE_ADDDPOINT);
	glist.AddLX();
	trackMIDIinput=glist.AddButton(-1,-1,w,-1,GADGETID_MIDIIN_STRING,MODE_RIGHT|MODE_MENU);
	glist.Return();

	g_inby=glist.AddButton(-1,-1,w,-1,"In Filter:",GADGET_MIDIINFILTER,MODE_TOGGLE);
	glist.AddLX();
	g_instring=glist.AddButton(-1,-1,-1,-1,0,GADGET_MIDIINFILTER_STRING,MODE_TEXTCENTER|MODE_RIGHT);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Output",GADGETID_MIDIOUT,MODE_ADDDPOINT);
	glist.AddLX();
	trackMIDIoutput=glist.AddButton(-1,-1,w,-1,GADGETID_MIDIOUT_STRING,MODE_RIGHT|MODE_MENU);
	glist.Return();

	g_outby=glist.AddButton(-1,-1,w,-1,"Out Filter:",GADGET_MIDIOUTFILTER,MODE_TOGGLE);
	glist.AddLX();
	g_outstring=glist.AddButton(-1,-1,-1,-1,0,GADGET_MIDIOUTFILTER_STRING,MODE_TEXTCENTER|MODE_RIGHT);
	glist.Return();

	//g_veloormainvolume=glist.AddButton(-1,-1,w,-1,"<Veloctiy>",GADGET_VELOORMAINVOLUME,MODE_TOGGLE|MODE_RIGHT);
	//glist.AddLX();

	Refresh(true);
}

void Edit_ArrangeFX_MIDI::ShowOutFilter(bool force)
{
	Seq_Track *t=editor->GetTrack();

	if(g_outby)
	{
		if(!t)
		{
			g_outby->Disable();
			goto out2;
		}

		if(force==true || g_outby->disabled==true || t->t_trackeffects.filter.bypass!=outbypass)
		{
			outbypass=t->t_trackeffects.filter.bypass;
			g_outby->Toggle(outbypass==true?false:true);
		}
	}

out2:
	if(g_outstring)
	{		
		if(!t)
		{
			g_outstring->Disable();
			return;
		}

		if(force==true || g_outstring->disabled==true || t->t_trackeffects.filter.Compare(&compareoutfilter)==false)
		{
			t->t_trackeffects.filter.Clone(&compareoutfilter);
			g_outstring->ChangeButtonText(t->t_trackeffects.filter.IsFilterActive()==true?"Filter!":Cxs[CXS_NOFILTER]);
		}
	}
}

void Edit_ArrangeFX_MIDI::ShowInFilter(bool force)
{
	Seq_Track *t=editor->GetTrack();

	if(g_inby)
	{
		if(!t)
		{
			g_inby->Disable();
			goto in2;
		}

		if(force==true || g_inby->disabled==true || t->t_trackeffects.inputfilter.bypass!=inbypass)
		{
			inbypass=t->t_trackeffects.inputfilter.bypass;
			g_inby->Toggle(inbypass==true?false:true);
		}
	}

in2:
	if(g_instring)
	{
		if(!t)
		{
			g_instring->Disable();
			return;
		}

		if(force==true || g_instring->disabled==true || t->t_trackeffects.inputfilter.Compare(&compareinfilter)==false)
		{
			t->t_trackeffects.inputfilter.Clone(&compareinfilter);

			g_instring->ChangeButtonText(t->t_trackeffects.inputfilter.IsFilterActive()==true?"Filter!":Cxs[CXS_NOFILTER]);
		}
	}
}

// Quantize
void Edit_ArrangeFX_Quantize::InitEffect(bool copy)
{
	Seq_Track *t=editor->GetTrack();
	if(!t)
	{
		effect=0;
		return;
	}
	else
	{
		effect=&t->t_trackeffects.quantizeeffect;

		if(copy==true)
			effect->Clone(&compareeffect);
	}
}

void Edit_ArrangeFX_Quantize::ShowFlag()
{
	if(flag)
	{
		if(effect)
		{
			char h[400];

			strcpy(h,"Q Events:");

			if(effect->flag&QUANTIZE_NOTES)
				mainvar->AddString(h," Notes");

			if(effect->flag&QUANTIZE_CONTROL)
				mainvar->AddString(h," Control");

			if(effect->flag&QUANTIZE_PITCHBEND)
				mainvar->AddString(h," Pitch");

			if(effect->flag&QUANTIZE_POLYPRESSURE)
				mainvar->AddString(h," PPress");

			if(effect->flag&QUANTIZE_CHANNELPRESSURE)
				mainvar->AddString(h," CPress");

			if(effect->flag&QUANTIZE_SYSEX)
				mainvar->AddString(h," SysEx");

			if(effect->flag&QUANTIZE_PROGRAMCHANGE)
				mainvar->AddString(h," Prog");

			if(effect->flag&QUANTIZE_INTERN)
				mainvar->AddString(h," Intern");

			flag->ChangeButtonText(h);
		}
		else
			flag->Disable();
	}
}

void Edit_ArrangeFX_Quantize::ShowBox()
{
	if(boxquantize)
	{
		if(effect)
			boxquantize->SetCheckBox(effect->usequantize);
		else
			boxquantize->Disable();
	}
}

void Edit_ArrangeFX_Quantize::ShowSelectQuant()
{
	if(selectquant)
	{
		if(effect)
		{
			if(char *h=mainvar->GenerateString(Cxs[CXS_QUANTIZETO],"<",effect->usequantize==false?Cxs[CXS_DISABLED]:"Q",">",quantstr[effect->quantize]) )
			{
				selectquant->ChangeButtonText(h);
				delete h;
			}
		}
		else
			selectquant->Disable();
	}
}

void Edit_ArrangeFX_Quantize::ShowAll()
{
	ShowBox();
	ShowFlag();
	ShowSelectQuant();

	if(boxnoteoffquant)
	{
		if(effect)
			boxnoteoffquant->SetCheckBox(effect->noteoffquant);
		else
			boxnoteoffquant->Disable();
	}

	if(boxsetnotelength)
	{
		if(effect)
			boxsetnotelength->SetCheckBox(effect->setnotelength);
		else
			boxsetnotelength->Disable();
	}

	if(notelength)
	{
		if(effect)
		{
			if(char *h=mainvar->GenerateString(Cxs[CXS_FIXNOTELENGTH],"<",effect->setnotelength==false?Cxs[CXS_DISABLED]:"F",">",quantstr[effect->notelength]) )
			{
				notelength->ChangeButtonText(h);
				delete h;
			}
		}
		else
			notelength->Disable();
	}

	if(humanize)
	{
		if(effect)
		{
			humanize->Toggle(effect->usehuman);
		}
		else
			humanize->Disable();
	}

	if(humamizeq)
	{
		if(effect)
		{
			switch(effect->humanq)
			{
			case 0:
				humamizeq->ChangeButtonText("r/Track QT");
				break;

			case TICK8nd:
				humamizeq->ChangeButtonText("r/8");
				break;

			case TICK16nd:
				humamizeq->ChangeButtonText("r/16");
				break;

			case TICK32nd:
				humamizeq->ChangeButtonText("r/32");
				break;

			case TICK64nd:
				humamizeq->ChangeButtonText("r/64");
				break;
			}
		}
		else
			humanizevalue->Disable();
	}

	if(humanizevalue)
	{
		if(effect)
		{
			humanizevalue->SetPos(effect->humanrange);
		}
		else
			humanizevalue->Disable();
	}
}

void Edit_ArrangeFX_Quantize::Init()
{
	glist.SelectForm(0,0);

	InitEffect(true);

	boxquantize=glist.AddCheckBox(-1,-1,-1,-1,GADGET_QUANTIZEONOFF,MODE_RIGHT,Cxs[CXS_QUANTIZE],Cxs[CXS_QUANTIZE]);
	glist.Return();

	flag=glist.AddButton(-1,-1,-1,-1,GADGET_QUANTIZEFLAG,MODE_RIGHT,Cxs[CXS_EVENTQTYPES]);
	glist.Return();

	selectquant=glist.AddButton(-1,-1,-1,-1,GADGET_SELECTQUANTIZE,MODE_RIGHT,Cxs[CXS_SELECTQRASTER]);
	glist.Return();

	//
	int w=bitmap.GetTextWidth(TRACKLEFTSTRING);
	humanize=glist.AddButton(-1,-1,w,-1,"Humanize:",GADGETID_HUMANIZE_I,MODE_TOGGLE,Cxs[CXS_USEHUMANQUANTIZE]);
	glist.AddLX();

	humamizeq=glist.AddButton(-1,-1,w,-1,GADGETID_HUMANIZE_Q,0,Cxs[CXS_HUMANQUANTIZERANGE]);
	glist.AddLX();

	humanizevalue=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_HUMANIZE,1,100,1,NUMBER_INTEGER_PERCENT,MODE_RIGHT);	
	glist.Return();

	// Length/Off
	boxnoteoffquant=glist.AddCheckBox(-1,-1,-1,-1,GADGET_NOTEOFFQUANTIZE,MODE_RIGHT,Cxs[CXS_QUANTIZENOTEOFFEND],Cxs[CXS_QUANTIZENOTEOFFPOSITION]);
	glist.Return();

	boxsetnotelength=glist.AddCheckBox(-1,-1,-1,-1,GADGET_NOTEOFFFIXLENGTH,MODE_RIGHT,Cxs[CXS_FIXNOTELENGTH],Cxs[CXS_FIXNOTELENGTH]);
	glist.Return();

	notelength=glist.AddButton(-1,-1,-1,-1,GADGET_NOTEOFFSELECTFIXLENGTH,MODE_RIGHT,Cxs[CXS_SELECTFIXNOTELENGTH]);
	glist.Return();

	/*
	qeditor=glist.AddButton(-1,-1,-1,-1,"Q Editor - Track",GADGET_QEDITOR,MODE_TEXTCENTER|MODE_RIGHT,Cxs[CXS_QUANTIZEEDITOREX]);
	glist.Return();
	*/

	ShowAll();

	/*

	boxgroove=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_USEGROOVE,"Groove",Cxs[CXS_GROOVEQUANTIZE]);
	y=maingui->AddFontY(y);

	selectgroove=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_SELECTGROOVE,0,Cxs[CXS_SELECTGROOVERASTER]);
	y=maingui->AddFontY(y);

	boxnoteoffquant=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_NOTEOFFQUANTIZE,Cxs[CXS_QUANTIZENOTEOFFEND],Cxs[CXS_QUANTIZENOTEOFFPOSITION]);
	y=maingui->AddFontY(y);

	boxsetnotelength=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_NOTEOFFFIXLENGTH,Cxs[CXS_FIXNOTELENGTH],Cxs[CXS_FIXNOTELENGTH]);
	y=maingui->AddFontY(y);

	notelength=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_NOTEOFFSELECTFIXLENGTH,0,Cxs[CXS_SELECTFIXNOTELENGTH]);
	y=maingui->AddFontY(y);

	// CAPTURE
	boxcapturequant=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_CAPTUREQUANTIZE,Cxs[CXS_CAPTUREQUANTIZE],Cxs[CXS_CAPTUREQUANTIZE]);
	y=maingui->AddFontY(y);

	capturerange=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_CAPTURERANGE,0,Cxs[CXS_RANGEOFCAPTURE]);
	y=maingui->AddFontY(y);

	boxstrength=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_STRENGTHQUANTIZE,Cxs[CXS_STRENGTHQUANTIZE],Cxs[CXS_STRENGTHQUANTIZE]);
	y=maingui->AddFontY(y);

	strength=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_STRENGTH,0,"Strength");
	y=maingui->AddFontY(y);

	boxhuman=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_HUMANQUANTIZE,Cxs[CXS_HUMANQUANTIZE],Cxs[CXS_USEHUMANQUANTIZE]);
	y=maingui->AddFontY(y);

	human=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_HUMANRANGE,0,Cxs[CXS_HUMANQUANTIZERANGE]);
	y=maingui->AddFontY(y);

	reset=gadgetlist->AddButton(x,y,x+((x2-x)/2),y+maingui->GetFontSizeY_Sub(),"Reset",GADGET_RESETQUANT,0,Cxs[CXS_RESETQSETTINGS]);

	gadgetlist->AddButton(x+((x2-x)/2)+1,y,x2,y+maingui->GetFontSizeY_Sub(),"Groove Editor",GADGET_GROOVE,0,"Groove Editor");
	ShowQuantizeStatus();
	*/
}

int Edit_ArrangeFX_Quantize::GetInitHeight()
{
	return maingui->GetButtonSizeY(7);
}

void Edit_ArrangeFX_Pattern::Init()
{
	int w=bitmap.GetTextWidth(TRACKLEFTSTRING);

	glist.SelectForm(0,0);

	g_patternstring=glist.AddString(-1,-1,-1,-1,GADGETID_PATTERBNAMESTRING,MODE_RIGHT,0,0);
	glist.Return();

	g_mute=glist.AddButton(-1,-1,-1,-1,"Mute",GADGET_MUTE,MODE_TOGGLE|MODE_AUTOTOGGLE|MODE_RIGHT);
	glist.Return();

	g_loop=glist.AddButton(-1,-1,w,-1,"Loop",GADGET_LOOP,MODE_TOGGLE,Cxs[CXS_PATTERNLOOPONOFF]);
	glist.AddLX();
	g_loopnr=glist.AddNumberButton(-1,-1,-1,-1,GADGET_LOOPNR,1,500,1,NUMBER_INTEGER,MODE_TEXTCENTER|MODE_RIGHT,Cxs[CXS_PATTERNLOOPNR]);
	glist.Return();
	g_loopendless=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_PATTERNLOOP],GADGET_LOOPENDLESS,MODE_TOGGLE|MODE_TEXTCENTER|MODE_RIGHT);
	glist.Return();

	g_loopmode=glist.AddButton(-1,-1,-1,-1,0,GADGET_LOOPMODE,MODE_TEXTCENTER|MODE_RIGHT|MODE_MENU,Cxs[CXS_PATTERNLOOPTYPES]);
	glist.Return();

	// MIDI
	g_MIDIchannel=glist.AddButton(-1,-1,w,-1,0,GADGET_PMIDICHANNEL,MODE_ADDDPOINT);
	glist.AddLX();
	g_MIDIchannelnr=glist.AddNumberButton(-1,-1,-1,-1,GADGET_PATTERNCHANNELNR,0,16,1,NUMBER_MIDICHANNEL,MODE_RIGHT);
	glist.Return();

	patterntranspose=glist.AddButton(-1,-1,w,-1,0,GADGETID_PTRANSPOSE,MODE_ADDDPOINT);
	glist.AddLX();
	patterntransposenr=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_PTRANSPOSE_NR,-127,127,0,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	patternvelocity=glist.AddButton(-1,-1,w,-1,0,GADGETID_PVELOCITY,MODE_ADDDPOINT);
	glist.AddLX();
	patternvelocitynr=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_PVELOCITY_NR,-127,127,0,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	onoff=glist.AddButton(-1,-1,-1,-1,GADGETID_ONOFFFADE,MODE_TOGGLE|MODE_LEFTTOMID);
	glist.AddLX();
	onoffvolume=glist.AddButton(-1,-1,-1,-1,GADGETID_ONOFFVOLUME,MODE_TOGGLE|MODE_MIDTORIGHT);
	glist.Return();

	qeditor=glist.AddButton(-1,-1,-1,-1,"Q Editor - Pattern",GADGET_PQEDITOR,MODE_TEXTCENTER|MODE_RIGHT);
	glist.Return();

	RefreshAll();
}

void Edit_ArrangeFX_Pattern::ChangeLoopFlag(int flag)
{
	Seq_Pattern *p=WindowSong()->GetFocusPattern();

	if(!p)
		return;

	if(flag!=p->loopflag)
	{
		p->loopflag=flag;

		if(p->loopendless==true || p->loopwithloops==true)
		{
			if(WindowSong()==mainvar->GetActiveSong())
			{
				mainthreadcontrol->LockActiveSong();
				p->LoopPattern();
				WindowSong()->CheckPlaybackRefresh();
				mainthreadcontrol->UnlockActiveSong();
			}
			else
				p->LoopPattern();
		}

		maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_ARRANGE,0);
	}
}

void Edit_ArrangeFX_Pattern::Gadget(guiGadget *g)
{
	Seq_Pattern *p=WindowSong()->GetFocusPattern();

	if(!p)
		return;

	switch(g->gadgetID)
	{
	case GADGET_MUTE:
		{
			if(Seq_SelectionList *sl=editor->arrangeeditor->CreateSelectionList(p))
				mainedit->MutePattern(editor->WindowSong(),sl, 0,p->mute==true?false:true);
		}
		break;

	case GADGET_PMIDICHANNEL:
		{
			if(p->itsaclone==true)
				p=p->mainclonepattern;

			AudioPattern *ap=p->mediatype==MEDIATYPE_AUDIO?(AudioPattern*)p:0;

			if(ap)
			{
				ap->volumecurve.SetVolume(0);
			}
		}
		break;

	case GADGETID_PTRANSPOSE:
		{
			if(p->itsaclone==true)
				p=p->mainclonepattern;

			MIDIPattern *mp=p->mediatype==MEDIATYPE_MIDI?(MIDIPattern*)p:0;

			if(mp)
			{
				mp->t_MIDIeffects.SetTranspose(0);
			}

			AudioPattern *ap=p->mediatype==MEDIATYPE_AUDIO?(AudioPattern*)p:0;

			if(ap)
			{
				ap->volumecurve.SetFadeIn(0);
			}
		}
		break;

	case GADGETID_PVELOCITY:
		{
			if(p->itsaclone==true)
				p=p->mainclonepattern;

			MIDIPattern *mp=p->mediatype==MEDIATYPE_MIDI?(MIDIPattern*)p:0;

			if(mp)
			{
				mp->t_MIDIeffects.SetVelocity(0);
			}

			AudioPattern *ap=p->mediatype==MEDIATYPE_AUDIO?(AudioPattern*)p:0;

			if(ap)
			{
				ap->volumecurve.SetFadeOut(0);
			}
		}
		break;

	case GADGETID_ONOFFFADE:
		{
			if(p->itsaclone==true)
				p=p->mainclonepattern;

			MIDIPattern *mp=p->mediatype==MEDIATYPE_MIDI?(MIDIPattern*)p:0;
			AudioPattern *ap=p->mediatype==MEDIATYPE_AUDIO?(AudioPattern*)p:0;

			if(ap)
			{
				ap->volumecurve.fadeinoutactive=ap->volumecurve.fadeinoutactive==true?false:true;
				onoff->Toggle(ap->volumecurve.fadeinoutactive);
			}
		}
		break;

	case GADGETID_ONOFFVOLUME:
		{
			if(p->itsaclone==true)
				p=p->mainclonepattern;

			MIDIPattern *mp=p->mediatype==MEDIATYPE_MIDI?(MIDIPattern*)p:0;
			AudioPattern *ap=p->mediatype==MEDIATYPE_AUDIO?(AudioPattern*)p:0;

			if(ap)
			{
				ap->volumecurve.volumeactive=ap->volumecurve.volumeactive==true?false:true;
				onoffvolume->Toggle(ap->volumecurve.volumeactive);
			}
		}
		break;

	case GADGET_PATTERNCHANNELNR: // + Volume
		{
			MIDIPattern *mp=p->mediatype==MEDIATYPE_MIDI?(MIDIPattern*)p:0;
			AudioPattern *ap=p->mediatype==MEDIATYPE_AUDIO?(AudioPattern*)p:0;

			if(mp)
				mp->t_MIDIeffects.SetMIDIChannel(MIDIchannel=g->GetPos());

			if(ap)
			{
				if(ap->itsaclone==true)
					ap=(AudioPattern *)ap->mainclonepattern;

				double h=g->GetDoublePos();

				ap->volumecurve.dbvolume=h;
			}
		}
		break;

	case GADGETID_PTRANSPOSE_NR: // +FADE In
		{
			MIDIPattern *mp=p->mediatype==MEDIATYPE_MIDI?(MIDIPattern*)p:0;
			AudioPattern *ap=p->mediatype==MEDIATYPE_AUDIO?(AudioPattern*)p:0;

			if(mp)
				mp->t_MIDIeffects.SetTranspose(MIDItranspose=g->GetPos());

			if(ap)
			{

				if(ap->itsaclone==true)
					ap=(AudioPattern *)ap->mainclonepattern;

				ap->volumecurve.SetFadeIn(g->GetDoublePos()*1000);
			}
		}
		break;

	case GADGETID_PVELOCITY_NR: // + Fade Out
		{
			MIDIPattern *mp=p->mediatype==MEDIATYPE_MIDI?(MIDIPattern*)p:0;
			AudioPattern *ap=p->mediatype==MEDIATYPE_AUDIO?(AudioPattern*)p:0;

			if(mp)
				mp->t_MIDIeffects.SetVelocity(MIDIvelocity=g->GetPos());

			if(ap)
			{
				if(ap->itsaclone==true)
					ap=(AudioPattern *)ap->mainclonepattern;

				ap->volumecurve.SetFadeOut(g->GetDoublePos()*1000);
			}
		}
		break;

	case GADGETID_PATTERBNAMESTRING:
		p->SetName(g->string);
		break;

	case GADGET_LOOPNR:
		{
			if(p->loopwithloops==true)
			{
				mainedit->LoopPattern(p,false,true,g->GetPos());
			}
			else
				p->loops=g->GetPos();
		}
		break;

	case GADGET_LOOP:
		mainedit->LoopPattern(p,false,p->loopwithloops==true?false:true,p->loops);
		break;

	case GADGET_LOOPENDLESS:
		{
			mainedit->LoopPattern(p,p->loopendless==true?false:true,p->loopendless==true && p->loopwithloops==true?true:false,p->loops);
		}
		break;

	case GADGET_LOOPMODE:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_lmode:public guiMenu
				{
				public:
					menu_lmode(Edit_ArrangeFX_Pattern *p,int f){flag=f;editor=p;}

					void MenuFunction()
					{
						editor->ChangeLoopFlag(flag);
					} //

					Edit_ArrangeFX_Pattern *editor;
					int flag;
				};

				popmenu->AddFMenu(Cxs[CXS_LOOPMEASURE],new menu_lmode(this,Seq_Pattern::PATTERNLOOP_MEASURE),p->loopflag==Seq_Pattern::PATTERNLOOP_MEASURE?true:false);
				popmenu->AddFMenu(Cxs[CXS_LOOPBEAT],new menu_lmode(this,Seq_Pattern::PATTERNLOOP_BEAT),p->loopflag==Seq_Pattern::PATTERNLOOP_BEAT?true:false);
				popmenu->AddFMenu(Cxs[CXS_LOOPDIRECT],new menu_lmode(this,Seq_Pattern::PATTERNLOOP_NOOFFSET),p->loopflag==Seq_Pattern::PATTERNLOOP_NOOFFSET?true:false);

				ShowPopMenu();
			}
		}
		break;
	}
}

void Edit_ArrangeFX_Pattern::RefreshRealtime_Slow()
{
	Seq_Pattern *p=WindowSong()->GetFocusPattern();

	if(!p)
		return;

	if(g_patternstring)
		g_patternstring->CheckString(p->GetName(),false);

	if(mute!=p->mute)
	{
		if(g_mute)
			g_mute->Toggle(mute=p->mute);
	}

	if(p->itsaloop==false)
	{
		if(loopwithloops!=p->loopwithloops || loopnr!=p->loops)
		{
			if(g_loopnr)
				g_loopnr->SetPos(loopnr=p->loops);
		}

		if(loopwithloops!=p->loopwithloops)
		{
			if(g_loop)
				g_loop->Toggle(loopwithloops=p->loopwithloops);
		}

		if(loopendless!=p->loopendless)
		{
			if(g_loopendless)
				g_loopendless->Toggle(loopendless=p->loopendless);
		}

		if(loopflag!=p->loopflag)
			ShowLoopMode(p);
	}

	switch(p->mediatype)
	{
	case MEDIATYPE_MIDI:
		{
			MIDIPattern *mp=(MIDIPattern *)p;

			if(g_MIDIchannelnr)
			{
				if(mp->t_MIDIeffects.GetChannel()!=MIDIchannel)
					g_MIDIchannelnr->SetPos(MIDIchannel=mp->t_MIDIeffects.GetChannel());
			}

			if(patterntransposenr)
			{
				if(mp->t_MIDIeffects.GetTranspose()!=MIDItranspose)
					patterntransposenr->SetPos(MIDItranspose=mp->t_MIDIeffects.GetTranspose());
			}

			if(patternvelocitynr)
			{
				if(mp->t_MIDIeffects.GetVelocity()!=MIDIvelocity)
					patternvelocitynr->SetPos(MIDIvelocity=mp->t_MIDIeffects.GetVelocity());
			}
		}
		break;

	case MEDIATYPE_AUDIO:
		{
			AudioPattern *ap=(AudioPattern *)p;

			if(dbvolume!=ap->volumecurve.dbvolume ||
				fadeinms!=ap->volumecurve.fadeinms ||
				fadeoutms!=ap->volumecurve.fadeoutms)
				RefreshAll();
		}
		break;
	}
}

void Edit_ArrangeFX_Pattern::ShowLoopMode(Seq_Pattern *p)
{
	if(g_loopmode)
	{
		char *h="?";
		switch(loopflag=p->loopflag)
		{
		case Seq_Pattern::PATTERNLOOP_MEASURE:
			h=Cxs[CXS_LOOPMEASURE];
			break;

		case Seq_Pattern::PATTERNLOOP_BEAT:
			h=Cxs[CXS_LOOPBEAT];
			break;

		case Seq_Pattern::PATTERNLOOP_NOOFFSET:
			h=Cxs[CXS_LOOPDIRECT];
			break;
		}

		g_loopmode->ChangeButtonText(h);
	}
}

void Edit_ArrangeFX_Pattern::DisableEventChannelTransVelo()
{
	glist.Disable(g_MIDIchannel);
	glist.Disable(g_MIDIchannelnr);
	glist.Disable(patterntranspose);
	glist.Disable(patterntransposenr);
	glist.Disable(patternvelocity);
	glist.Disable(patternvelocitynr);
}

void Edit_ArrangeFX_Pattern::RefreshAll()
{
	Seq_Pattern *p=WindowSong()->GetFocusPattern();

	if(p)
	{
		loopwithloops=p->loopwithloops;
		loopnr=p->loops;
		loopendless=p->loopendless;
		mute=p->mute;

		if(p->itsaloop==true)
		{
			glist.Disable(g_loop);
			glist.Disable(g_loopnr);
			glist.Disable(g_loopendless);
			glist.Disable(g_loopmode);
		}
		else
		{
			// Loop
			if(g_loop)
				g_loop->Toggle(p->loopwithloops);

			if(g_loopnr)
				g_loopnr->SetPos(p->loops);

			if(g_loopendless)
				g_loopendless->Toggle(p->loopendless);

			ShowLoopMode(p);
		}

		if(g_patternstring)
			g_patternstring->SetString(p->GetName());

		if(g_mute)
			g_mute->Toggle(p->mute);

		if(qeditor)
			qeditor->Enable();

		if(p->mediatype==MEDIATYPE_MIDI)
		{
			glist.Disable(onoff);
			glist.Disable(onoffvolume);

			MIDIPattern *mp=(MIDIPattern *)p;

			if(g_MIDIchannel)
			{
				g_MIDIchannel->ChangeButtonText("P Channel");
			}

			if(g_MIDIchannelnr)
			{
				g_MIDIchannelnr->SetPosType(NUMBER_MIDICHANNEL);
				g_MIDIchannelnr->SetFromTo(0,16);
				g_MIDIchannelnr->SetPos(MIDIchannel=mp->t_MIDIeffects.GetChannel());
			}

			if(patterntranspose)
				patterntranspose->ChangeButtonText("P Transpose");

			if(patterntransposenr)
			{
				patterntransposenr->SetPosType(NUMBER_INTEGER);
				patterntransposenr->SetFromTo(-127,127);
				patterntransposenr->SetPos(MIDItranspose=mp->t_MIDIeffects.GetTranspose());
			}

			if(patternvelocity)
				patternvelocity->ChangeButtonText("P Velocity");

			if(patternvelocitynr)
			{
				patternvelocitynr->SetPosType(NUMBER_INTEGER);
				patternvelocitynr->SetFromTo(-127,127);
				patternvelocitynr->SetPos(MIDIvelocity=mp->t_MIDIeffects.GetVelocity());
			}
		}
		else
		{
			if(p->itsaclone)
				p=p->mainclonepattern;

			AudioPattern *ap=(AudioPattern *)p;

			dbvolume=ap->volumecurve.dbvolume;
			fadeinms=ap->volumecurve.fadeinms;
			fadeoutms=ap->volumecurve.fadeoutms;

			if(g_MIDIchannel)
			{
				g_MIDIchannel->ChangeButtonText("Volume/dB");

				if(g_MIDIchannelnr)
				{
					g_MIDIchannelnr->SetPosType(NUMBER_0);
					g_MIDIchannelnr->SetFromTo(mainaudio->ConvertFactorToDb(mainaudio->silencefactor),24);
					g_MIDIchannelnr->SetPos(ap->volumecurve.dbvolume);
				}	
			}

			if(patterntranspose)
			{
				patterntranspose->ChangeButtonText("Fade In/sec");

				if(patterntransposenr)
				{
					LONGLONG samples=ap->GetSamples();
					double ms=mainaudio->ConvertSamplesToMs(samples/2);

					ms/=1000; // sek

					patterntransposenr->SetPosType(NUMBER_0);
					patterntransposenr->SetFromTo(0,ms);
					patterntransposenr->SetPos(ap->volumecurve.fadeinms/1000);
				}
			}

			if(patternvelocity)
			{
				patternvelocity->ChangeButtonText("Fade Out/sec");

				if(patternvelocitynr)
				{
					LONGLONG samples=ap->GetSamples();
					double ms=mainaudio->ConvertSamplesToMs(samples/2);

					ms/=1000; // sek

					patternvelocitynr->SetPosType(NUMBER_0);
					patternvelocitynr->SetFromTo(0,ms);
					patternvelocitynr->SetPos(ap->volumecurve.fadeoutms/1000);
				}
			}

			if(onoff)
			{
				onoff->ChangeButtonText("Fade In/Out");
				onoff->Toggle(ap->volumecurve.fadeinoutactive);

				onoff->Enable();
			}

			if(onoffvolume)
			{
				onoffvolume->ChangeButtonText("P Volume");
				onoffvolume->Toggle(ap->volumecurve.volumeactive);
				onoffvolume->Enable();
			}
		}
	}
	else
	{
		glist.Disable(g_mute);
		glist.Disable(onoff);
		glist.Disable(onoffvolume);

		glist.Disable(g_loop);
		glist.Disable(g_loopnr);
		glist.Disable(g_loopendless);
		glist.Disable(g_loopmode);
		glist.Disable(g_patternstring);

		glist.Disable(qeditor);

		DisableEventChannelTransVelo();
	}
}

int Edit_ArrangeFX_Pattern::GetInitHeight()
{
	return maingui->GetButtonSizeY(10);
}

void GUI::CreateQuantizePopUp(guiWindow *win,QuantizeEffect *effect,int flag)
{
	win->DeletePopUpMenu(true);

	if(win->popmenu)
	{
		class menu_qt:public guiMenu
		{
		public:
			menu_qt(QuantizeEffect *ef,int m,int f)
			{
				effect=ef;
				mode=m;
				flag=f;
			}

			void MenuFunction()
			{
				if(effect->quantize!=mode)
				{
					if(flag==QPUFLAG_QUANTIZE)
					{
						if(effect->quantize==mode)
							return;

						effect->quantize=mode;

						if(effect->humanq==0)
							effect->SetHumanQ(0);
					}

					if(flag==QPUFLAG_NOTELENGTH)
					{
						if(effect->notelength==mode)
							return;
						effect->notelength=mode;
					}

					// Refresh Events
					if(effect->pattern)
						mainedit->QuantizePattern(effect->pattern,effect,true);
					else
						if(effect->track)
							mainedit->QuantizeTrack(effect->track,effect,true);


				}
			} //

			QuantizeEffect *effect;
			int mode;
			int flag;
		};

		// Quantize
		for(int a=0;a<QUANTNUMBER;a++)
		{
			bool sel;

			if(flag==QPUFLAG_QUANTIZE && a==effect->quantize)
				sel=true;
			else
				if(flag==QPUFLAG_NOTELENGTH && a==effect->notelength)
					sel=true;
				else
					sel=false;

			win->popmenu->AddFMenu(quantstr[a],new menu_qt(effect,a,flag),sel);
		}

		win->ShowPopMenu();
	}
}

void Edit_ArrangeFX_Quantize::Gadget(guiGadget *g)
{
	if(!effect)return;

	switch(g->gadgetID)
	{
	case GADGET_QUANTIZEFLAG:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_qflag:public guiMenu
				{
				public:
					menu_qflag(QuantizeEffect *fx,int f)
					{
						effect=fx;
						flag=f;
					}

					void MenuFunction()
					{
						if(effect->flag&flag)
							effect->flag CLEARBIT flag;
						else
							effect->flag |=flag;

						if(effect->usequantize==true)
							mainedit->QuantizeTrack(effect->track,effect,true);
					} //

					QuantizeEffect *effect;
					int flag;
				};

				popmenu->AddFMenu("Notes",new menu_qflag(effect,QUANTIZE_NOTES),effect->flag&QUANTIZE_NOTES?true:false);
				popmenu->AddLine();
				popmenu->AddFMenu("Control Change",new menu_qflag(effect,QUANTIZE_CONTROL),effect->flag&QUANTIZE_CONTROL?true:false);
				popmenu->AddFMenu("Pitchbend",new menu_qflag(effect,QUANTIZE_PITCHBEND),effect->flag&QUANTIZE_PITCHBEND?true:false);
				popmenu->AddFMenu("Poly Pressure",new menu_qflag(effect,QUANTIZE_POLYPRESSURE),effect->flag&QUANTIZE_POLYPRESSURE?true:false);
				popmenu->AddFMenu("Channel Pressure",new menu_qflag(effect,QUANTIZE_CHANNELPRESSURE),effect->flag&QUANTIZE_CHANNELPRESSURE?true:false);
				popmenu->AddFMenu("Program Change",new menu_qflag(effect,QUANTIZE_PROGRAMCHANGE),effect->flag&QUANTIZE_PROGRAMCHANGE?true:false);
				popmenu->AddFMenu("SysEx",new menu_qflag(effect,QUANTIZE_SYSEX),effect->flag&QUANTIZE_SYSEX?true:false);

				ShowPopMenu();	
			}// if popmenu
		}
		break;

	case GADGET_QUANTIZEONOFF:
		{
			if(effect->usequantize==false)
			{
				effect->usegroove=false;
				effect->usequantize=true;
			}
			else
				effect->usequantize=false;

			mainedit->QuantizeTrack(effect->track,effect,true);
		}
		break;

	case GADGET_SELECTQUANTIZE:
		maingui->CreateQuantizePopUp(this,effect,QPUFLAG_QUANTIZE);
		break;

	case GADGETID_HUMANIZE_I:
		{
			effect->Humanize(effect->usehuman==true?false:true);
		}
		break;

	case GADGETID_HUMANIZE_Q:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_hqt:public guiMenu
				{
				public:
					menu_hqt(QuantizeEffect *fx,int m)
					{
						effect=fx;
						q=m;
					}

					void MenuFunction()
					{
						if(q!=effect->humanq)
							effect->SetHumanQ(q);
					}

					QuantizeEffect *effect;
					int q;
				};

				popmenu->AddFMenu("R=Track Q",new menu_hqt(effect,0),effect->humanq==0?true:false);
				popmenu->AddLine();

				popmenu->AddFMenu("R 1/8",new menu_hqt(effect,TICK8nd),effect->humanq==TICK8nd?true:false);
				popmenu->AddFMenu("R 1/16",new menu_hqt(effect,TICK16nd),effect->humanq==TICK16nd?true:false);
				popmenu->AddFMenu("R 1/32",new menu_hqt(effect,TICK32nd),effect->humanq==TICK32nd?true:false);
				popmenu->AddFMenu("R 1/64",new menu_hqt(effect,TICK64nd),effect->humanq==TICK64nd?true:false);

				ShowPopMenu();
			}
		}
		break;

	case GADGETID_HUMANIZE:
		{
			effect->SetHumanRange(g->GetPos());
		}
		break;

	case GADGET_NOTEOFFQUANTIZE:
		effect->noteoffquant=effect->noteoffquant==true?false:true;

		if(effect->usequantize==true)
			mainedit->QuantizeTrack(effect->track,effect,true);
		break;

	case GADGET_NOTEOFFFIXLENGTH:
		{
			effect->setnotelength=effect->setnotelength==true?false:true;

			if(effect->usequantize==true)
				mainedit->QuantizeTrack(effect->track,effect,true);
		}
		break;

	case GADGET_NOTEOFFSELECTFIXLENGTH:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_qt:public guiMenu
				{
				public:
					menu_qt(QuantizeEffect *fx,int m)
					{
						effect=fx;
						mode=m;
					}

					void MenuFunction()
					{
						effect->notelength=mode;

						// Refresh Events
						if(effect->setnotelength==true && effect->usequantize==true)
							mainedit->QuantizeTrack(effect->track,effect,true);
					} //

					QuantizeEffect *effect;
					int mode;
				};

				// Quantize
				for(int a=0;a<QUANTNUMBER;a++)
				{
					bool sel;

					if(a==effect->notelength)
						sel=true;
					else
						sel=false;

					popmenu->AddFMenu(quantstr[a],new menu_qt(effect,a),sel);
				}

				ShowPopMenu();
			}
		}
		break;
	}
}

void Edit_ArrangeFX_Quantize::RefreshRealtime_Slow()
{
	QuantizeEffect *old=effect;
	InitEffect(false);

	if( (old && (!effect)) ||
		((!old) && effect)
		)
		goto draw;

	if(effect && effect->Compare(&compareeffect)==false)
		goto draw;

	return;

draw:
	InitEffect(true);
	ShowAll();
}

int Edit_ArrangeFX_Mixer::GetInitHeight()
{
	return maingui->GetButtonSizeY(12);
}

Edit_ArrangeFX_Mixer::Edit_ArrangeFX_Mixer()
{
	InitForms(FORM_HORZ1x2BAR_MIXER1TRACK);
	isstatic=true;
	solotrack=true;

	minheight=GetInitHeight();
}

// Audio

void Edit_ArrangeFX_Audio::ShowAudioInput(bool force)
{
	if(g_instring)
	{
		Seq_Track *t=editor->GetTrack();

		if(!t)
		{
			g_instring->Disable();
			return;
		}

		if(force==true || g_instring->disabled==true || t->GetAudioFX()->CheckAudioInputString(g_instring->text)==false)
		{
			char *n=t->GetAudioFX()->GetAudioInputString();

			g_instring->ChangeButtonText(n);
			if(n)
				delete n;
		}
	}
}

void Edit_ArrangeFX_Audio::ShowAudioOutput(bool force)
{
	if(g_outstring)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			g_outstring->Disable();
		}
		else
			if(force==true || g_outstring->disabled==true || t->GetAudioFX()->CheckAudioOutputString(g_outstring->text,0)==false)
			{
				TAudioOut tao;
				Seq_AudioIO ao;

				tao.audiochannelouts=&ao;
				tao.simple=true;

				t->ShowAudioOutput(&tao);

				g_outstring->ChangeButtonText(tao.returnstring);

				//			g_outstring->ChangeButtonText(t->t_audiofx.CheckAudioOutputString(g_outstring->text,0));
			}
	}
}

void Edit_ArrangeFX_Audio::Gadget(guiGadget *g)
{
	Seq_Track *t=editor->GetTrack();
	if(!t)return;

	switch(g->gadgetID)
	{
	case GADGETID_AUDIOIOSTRING:
		if(t->CanChangeType()==true)
		{
			if(DeletePopUpMenu(true))
			{
				t->CreateChannelTypeMenu(popmenu);
				ShowPopMenu();
			}
		}
		break;

	case GADGETID_AUDIOSETFREE:
		{
			if(t->song==mainvar->GetActiveSong())
			{
				mainthreadcontrol->LockActiveSong();
				t->RemoveAllOtherTracksRecording();
				mainthreadcontrol->UnlockActiveSong();
			}
			else
				t->RemoveAllOtherTracksRecording();

			t->outputisinput=t->outputisinput==true?false:true;
			ShowAudioIsInput(true);
		}
		break;

	case GADGETID_AUDIOINMONITOR:
		if(t->ismetrotrack==false)
			t->SetInputMonitoring(t->io.inputmonitoring==true?false:true);

		ShowAudioInputMonitoring(true);

		break;

	case GADGETID_AUDIOTHRU:
		if(t->ismetrotrack==false)
			t->io.thru=t->io.thru==true?false:true;

		ShowAudioThru(true);
		break;

	case GADGETID_AUDIOINPUT:
	case GADGETID_AUDIOINPUTSTRING:
		if(t->ismetrotrack==false)
			maingui->EditTrackInput(this,t);
		break;

	case GADGETID_AUDIOOUTPUT:
	case GADGETID_AUDIOOUTPUTSTRING:
		maingui->EditTrackOutput(this,t);
		break;
	}
}

void Edit_ArrangeFX_Audio::ShowAudioInputMonitoring(bool force)
{
	if(g_audioinputmonitoring)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			g_audioinputmonitoring->Disable();
			return;
		}

		if(force==true || g_audioinputmonitoring->disabled==true || t->io.inputmonitoring!=inputmonitoring)
		{
			inputmonitoring=t->io.inputmonitoring;
			g_audioinputmonitoring->Toggle(inputmonitoring);
		}
	}
}

void Edit_ArrangeFX_Audio::ShowAudioIsInput(bool force)
{
	if(g_audiooutsetfree)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			g_audiooutsetfree->Disable();
			return;
		}

		if(force==true || g_audiooutsetfree->disabled==true || t->outputisinput!=isinput)
		{
			isinput=t->outputisinput;
			g_audiooutsetfree->Toggle(isinput);
		}
	}
}

void Edit_ArrangeFX_Audio::ShowAudioIO(bool force)
{
	if(g_iostring)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			g_iostring->Disable();
			return;
		}

		if(force==true || g_audiothru->disabled==true || t->io.channel_type!=channel_type)
		{
			g_iostring->ChangeButtonText(channelchannelsinfo[channel_type=t->io.channel_type]);
		}
	}
}

void Edit_ArrangeFX_Audio::ShowAudioThru(bool force)
{
	if(g_audiothru)
	{
		Seq_Track *t=editor->GetTrack();
		if(!t)
		{
			g_audiothru->Disable();
			return;
		}

		if(force==true || g_audiothru->disabled==true || t->io.thru!=audiothru)
		{
			audiothru=t->io.thru;
			g_audiothru->Toggle(audiothru);
		}
	}
}

void Edit_ArrangeFX_Audio::Refresh(bool t)
{
	ShowAudioIO(t);
	ShowAudioThru(t);
	ShowAudioInput(t);
	ShowAudioOutput(t);
	ShowAudioInputMonitoring(t);
	ShowAudioIsInput(t);
}

void Edit_ArrangeFX_Audio::Init()
{
	glist.SelectForm(0,0);

	int w=bitmap.GetTextWidth(TRACKLEFTSTRING);

	glist.AddButton(-1,-1,w,-1,"IO",GADGETID_AUDIOIO,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	g_iostring=glist.AddButton(-1,-1,-1,-1,GADGETID_AUDIOIOSTRING,MODE_RIGHT|MODE_MENU);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Input",GADGETID_AUDIOINPUT,MODE_ADDDPOINT);
	glist.AddLX();
	g_instring=glist.AddButton(-1,-1,-1,-1,GADGETID_AUDIOINPUTSTRING,MODE_RIGHT|MODE_MENU);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Output",GADGETID_AUDIOOUTPUT,MODE_ADDDPOINT);
	glist.AddLX();
	g_outstring=glist.AddButton(-1,-1,-1,-1,GADGETID_AUDIOOUTPUTSTRING,MODE_RIGHT|MODE_MENU);
	glist.Return();

	g_audiothru=glist.AddButton(-1,-1,-1,-1,"Audio Thru",GADGETID_AUDIOTHRU,MODE_RIGHT|MODE_TOGGLE);
	glist.Return();

	g_audioinputmonitoring=glist.AddButton(-1,-1,-1,-1,"Input Monitoring",GADGETID_AUDIOINMONITOR,MODE_RIGHT|MODE_TOGGLE);
	glist.Return();

	g_audiooutsetfree=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_SETTRACKOUTPUTFREE],GADGETID_AUDIOSETFREE,MODE_RIGHT|MODE_TOGGLE);
	glist.Return();

	Refresh(true);
}

int Edit_ArrangeFX_Audio::GetInitHeight()
{
	return maingui->GetButtonSizeY(6);
}

void Edit_ArrangeFX_Audio::RefreshRealtime_Slow()
{
	Refresh(false);
}
