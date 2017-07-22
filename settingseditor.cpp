#include "songmain.h"
#include "semapores.h"
#include "editsettings.h"
#include "MIDIhardware.h"
#include "audiohardware.h"
#include "settings.h"
#include "vstplugins.h"
#include "editwin32audio.h"
#include "languagefiles.h"
#include "object_project.h"
#include "object_song.h"
#include "object_track.h"
#include "transporteditor.h"
#include "audioports.h"
#include "editdata.h"
#include "audiohardwarechannel.h"
#include "camxfile.h"

#define DEFSIZE bitmap.GetTextWidth("WWWW Audio System WWWW")

extern char *channelchannelsinfo[],*channelchannelsinfo_short[];
extern int channelschannelsnumber[];

int optsetsize[]=
{
	16,
	32,
	64,
	96,
	128,
	160,
	192,
	224,
	256,
	384,

	512,
	1024,
	2048,
	3072,
	4096,
	4096+1024,
	4096+2048,
	(2*4096),
};

#define OPTLATENTCY 18

// ShowSettings FLAG
#define NO_DEVICEINFOSTRING 1

// Audio Devices
#include "asio_device.h"

// Song
#define SETTINGNAME_SONG_TIMEDISPLAY Cxs[CXS_SONG_TIMEDISPLAY]
#define SETTINGNAME_SONG_NOTEDISPLAY Cxs[CXS_SONG_NOTEDISPLAY]
#define SETTINGNAME_SONG_GMDISPLAY "Song: General MIDI"
#define SETTINGNAME_SONG_METRODISPLAY Cxs[CXS_SONG_METRONOME]
#define SETTINGNAME_SONG_ROTINGDISPLAY Cxs[CXS_SONG_MIDIINPUTROUTING]

// Settings Selection
// AUDIO
#define SETTINGNAME_AUDIO "Audio System/Hardware"
#define SETTINGNAME_INPUTCHANNELS Cxs[CXS_SET_AUDIOINCHANNELS]
#define SETTINGNAME_OUTPUTCHANNELS Cxs[CXS_SET_AUDIOOUTCHANNELS]

#define SETTINGNAME_AUDIOPLUGINS "Audio Plugins"
#define SETTINGNAME_VSTDIRECTORY Cxs[CXS_SET_VSTDIRS]
#define SETTINGNAME_VSTDIRECTORY3 Cxs[CXS_SET_VSTDIRS3]

#define SETTINGNAME_VSTFX Cxs[CXS_SET_VSTFX]
#define SETTINGNAME_CRASHEDPLUGINS "Plugins *** CRASHED ***"
#define SETTINGNAME_AUDIODIRECTORY Cxs[CXS_SET_AFILESDIRS]

// MIDI
#define SETTINGNAME_MIDI "MIDI Events"
#define SETTINGNAME_MIDIOUTPUT Cxs[CXS_SET_MIDIOUT]
#define SETTINGNAME_MIDIINPUT Cxs[CXS_SET_MIDIIN]

// GUI
#define SETTINGNAME_GUI Cxs[CXS_SET_UI]

// SYNC
#define SETTINGNAME_SYNC Cxs[CXS_SET_SYNC]

// Program Change
#define SETTINGNAME_PROGRAM "Devices/Programs"

// Files
#define SETTINGNAME_FILES Cxs[CXS_SET_FILES]
#define SETTINGNAME_LANGUAGE Cxs[CXS_LANGUAGESELECTION]
#define SETTINGNAME_KEYS Cxs[CXS_SET_KEYS]

enum gID{

	GADGET_HEADERID,
	GADGET_SETTINGSID,

	//AUDIO
	GADGET_AUDIODEVICEID,
	GADGET_AUDIOSYSTEMID,
	GADGET_USEAUDIODEVICEID,
	GADGET_AUDIOCHANNELID,
	GADGET_SAMPLERATES,
	GADGET_RAFBUFFER,
	GADGET_CPUINFO,
	GADGET_SETSIZE,
	GADGET_SETSIZEQUICK,

	GADGET_AUDIOINFOID,
	GADGET_ASIOSETTINGS,
	GADGET_VSTDIRECTORY,
	GADGET_VSTEFFECTS,
	GADGET_VSTINSTRUMENTS,
	GADGET_AUTOINSTRUMENT,
	GADGET_AUTOTRACKMIDIININSTRUMENT,
	GADGET_VSTINFO,
	GADGET_PLUGINS,
	GADGET_ADDVSTDIRECTORY,
	GADGET_DELETEVSTDIRECTORY,
	GADGET_AUDIOSETTINGS,

	GADGET_AUDIOINCHANNELS,
	GADGET_AUDIOINCHANNELSPORTTYPE,
	GADGET_AUDIOINCHANNELSPORTS,
	GADGET_AUDIOINPORTS,
	GADGET_AUDIOINPORTEDIT,
	GADGET_AUDIOOUTCHANNELS,
	GADGET_AUDIOOUTCHANNELSPORTTYPE,
	GADGET_AUDIOOUTCHANNELSPORTS,
	GADGET_AUDIOOUTPORTS,
	GADGET_AUDIOOUTPORTEDIT,

	GADGET_AUDIODIRECTORY,
	GADGET_ADDAUDIODIRECTORY,
	GADGET_DELETEAUDIODIRECTORY,

	//MIDI
	GADGET_MIDIPORTID,
	GADGET_MIDIDEVICEID,
	GADGET_MIDICLOCKID,
	GADGET_MIDIMETRONOMEID,
	GADGET_MIDIMETRONOMECHANNELID,
	GADGET_MIDIMETRONOMENOTEID,
	GADGET_MIDIMETRONOMEVELOID,
	GADGET_MIDIMETRONOMECHANNELHIID,
	GADGET_MIDIMETRONOMENOTEHIID,
	GADGET_MIDIMETRONOMEVELOHIID,
	GADGET_DEFAULTDEVICEID,
	GADGET_MIDIINPORID,
	GADGET_MIDIINDEVICEID,
	GADGET_MIDIINDEVICEFILTER_ID,

	GADGET_MIDIINDEVICEINFO_ID,
	GADGET_MIDIINDEVICESYNC_ID,
	GADGET_MIDIINDEVICETEMPOQUANT_ID ,

	GADGET_MIDIOUTDEVICEINFO_ID,
	GADGET_MIDIOUTDEVICEFILTER_ID,
	//GADGET_MIDIOPTIMIZER,
	GADGET_SENDALWAYSCONTROL,
	GADGET_SENDMTCID,

	GADGET_GUIEDITOR,
	GADGET_GUIEDITOR_WINDOWFORMAT,
	GADGET_GUIEDITOR_WINDOWGMFORMAT,
	GADGET_GUIEDITOR_ARRANGEFILEDISPLAY,
	GADGET_GUIEDITOR_UNDO,
	GADGET_SOLOEDITOR,
	GADGET_FOLLOWEDITOR,
	GADGET_AUTOLOADPROJECT,
	GADGET_SHOWHEADERBOTH,

	GADGET_SYNCOUTPUTMIDICLOCKSP,
	GADGET_SYNCINPUTMIDICLOCKSP,
	GADGET_SENDMTC,

	GADGET_QUANTIZETOSPP,
	GADGET_SYNCGLOBALSMPTE,
	GADGET_SHOWOFF ,
	GADGET_SMPTEDISPLAYOFFSET,

	GADGET_MIDIFILE,
	GADGET_SPLITMIDIFILE,

	GADGET_IMPORTMIDIARR,
	GADGET_AUTOSAVE,
	GADGET_AUDIOUNUSED,
	GADGET_PEAKFILE,
	GADGET_AUDIOOUTFORMAT,
	GADGET_UNUSEDSONGFILES,
	GADGET_IMPORTAUDIO,
	GADGET_ASKIMPORTAUDIO,
	GADGET_IGNORECORRUPTAUDIOFILES,
	GADGET_IMPORTQUESTION,

	GADGET_SONG_TIMEDISPLAY, // Song
	GADGET_SONG_NOTEDISPLAY,
	GADGET_SONG_NAME,
	GADGET_SONG_GMDISPLAY,
	GADGET_SONG_METRO,
	GADGET_SONG_PLAYMETRO,
	GADGET_SONG_RECMETRO,

	GADGET_SONG_SELINPUT, // MIDI Input Routing
	GADGET_SONG_SELINPUTCHANNELS,
	GADGET_SONG_SELINPUTTRACKS,
	GADGET_SONG_SELINPUTTYPE,

	GADGET_SONG_ADDTRACK,
	GADGET_SONG_ADDACTIVETRACK,
	GADGET_SONG_DELETETRACK,

	GADGET_SHOWTOOLTIP,
	GADGET_DEVICE,
	GADGET_DEVICENAME,
	GADGET_ADDDEVICE,
	GADGET_DELETEDEVICE,
	GADGET_DEVICEPROGRAM,
	GADGET_ADDPROGRAM,
	GADGET_DELETEPROGRAM,
	GADGET_PROGRAMNAME,
	GADGET_PROGRAMCHANNEL,
	GADGET_PROGRAMPROGRAM,
	GADGET_PROGRAMBANK,
	GADGET_PROGRAMBANKSEL,
	GADGET_TRANSPORTDESKTOP,
	GADGET_DOUBLESTOP,

	GADGET_HW_SYSTEM,
	GADGET_HW_DEVICE,
	GADGET_HW_LIST,

	GADGET_PRO_NAME,
	GADGET_PRO_SR,
	GADGET_PRO_PANLAW,
	GADGET_PRO_XBOXRECORDPEAK,
	GADGET_PRO_CUTRECORDZEROSAMPLES,
	GADGET_PRO_XBOXRECORDTHRESHOLD,
	GADGET_PRO_XBOXRECORDENDTHRESHOLD,

	GADGET_AUTOUPDATE,
};


Edit_Settings::Edit_Settings()
{
	editorid=EDITORTYPE_SETTINGS;
	editorname=Cxs[CXS_SETTINGS];

	InitForms(FORM_HORZ2x1);

	minwidth=maingui->GetButtonSizeY(MINSETTINGSEDITORHEIGHT);

	minheight=maingui->GetButtonSizeY(MINSETTINGSEDITORHEIGHT);

	resizeable=true;
	ondesktop=true;

	openeditor=0;

	activeMIDIdevice=0;
	activeMIDIprogram=0;

	dontshowMIDIprogramname=dontshowMIDIdevicename=false;

	settingsselection=mainsettings->defaultsettingsselection;

	audiodevice=0;

	MIDIinputdevice=0;
	MIDIoutdevice=0;

	activeaudiodirectory=0;
	activevstdirectory=0;

	set_selectinputdevice=0;
	set_selecttypeindex=SIT_CHANNEL;

	set_selectedchannel=0;
	set_selectedtype=0;
	set_selectedtrack=0;

	focustrackname=0;
	selected_hw_device=0;
	selected_hw_system=0;

	songnamebuff=0;

	settingname_help=0;

	editproject=0;

	outportsselection=mainsettings->outportsselection;
	inportsselection=mainsettings->inportsselection;
	audioinporttypeselection=mainsettings->audioinporttypeselection;
	audiooutporttypeselection=mainsettings->audiooutporttypeselection;

	ResetGadgets();
}

void Edit_Settings::InitSelectBox()
{
	if(selectbox)
	{
		if(editproject)
		{
			selectbox->AddTooltip(Cxs[CXS_PROJECTSETTINGS]);
			selectbox->AddStringToListBox(Cxs[CXS_PROJECT],PRO_PROJECT);

			if(char *h=mainvar->GenerateString(Cxs[CXS_PROJECT],":","Audio"))
			{
				selectbox->AddStringToListBox(h,PRO_PROJECT_AUDIO);
				delete h;
			}

			if(char *h=mainvar->GenerateString(Cxs[CXS_PROJECT],":","GUI"))
			{
				selectbox->AddStringToListBox(h,PRO_PROJECT_GUI);
				delete h;
			}
		}
		else
			if(WindowSong())
			{
				selectbox->AddTooltip(Cxs[CXS_SET_SELECTSONGSETTINGS]);

				selectbox->AddStringToListBox("Song",SONG_SONG);
				selectbox->AddStringToListBox(SETTINGNAME_SYNC,SONG_SYNC);
				//	selectbox->AddStringToListBox(SETTINGNAME_SONG_NOTEDISPLAY,SONG_NOTEDISPLAY);
				selectbox->AddStringToListBox(SETTINGNAME_SONG_GMDISPLAY,SONG_GMDISPLAY);
				selectbox->AddStringToListBox(SETTINGNAME_SONG_METRODISPLAY,SONG_METRO);
				selectbox->AddStringToListBox(SETTINGNAME_SONG_ROTINGDISPLAY,SONG_ROUTING);
			}
			else
			{
				selectbox->AddTooltip(Cxs[CXS_SET_SELECTGLOBALSETTINGS]);

				selectbox->AddStringToListBox(SETTINGNAME_AUDIO,AUDIOSETTINGS);

				selectbox->AddStringToListBox(SETTINGNAME_OUTPUTCHANNELS,AUDIOSETTINGS_AUDIOOUTCHANNELS);
				selectbox->AddStringToListBox(SETTINGNAME_INPUTCHANNELS,AUDIOSETTINGS_AUDIOINCHANNELS);

				//selectbox->AddStringToListBox(SETTINGNAME_AUDIOSETTINGS_HARDWARE,AUDIOSETTINGS_HARDWARE);

				selectbox->AddStringToListBox(SETTINGNAME_AUDIODIRECTORY,AUDIOSETTINGS_AUDIODIRECTORY);

				selectbox->AddStringToListBox(SETTINGNAME_AUDIOPLUGINS,AUDIOSETTINGS_PLUGINS);

				selectbox->AddStringToListBox(SETTINGNAME_VSTDIRECTORY,AUDIOSETTINGS_VSTDIRECTORY);
				selectbox->AddStringToListBox(SETTINGNAME_VSTDIRECTORY3,AUDIOSETTINGS_VSTDIRECTORY3);

				selectbox->AddStringToListBox(SETTINGNAME_VSTFX,AUDIOSETTINGS_VSTFX);
				selectbox->AddStringToListBox(SETTINGNAME_CRASHEDPLUGINS,AUDIOSETTINGS_CRASHEDPLUGINS);

				selectbox->AddStringToListBox(SETTINGNAME_MIDI,MIDISETTINGS);
				selectbox->AddStringToListBox(Cxs[CXS_METRONOME],METROSETTINGS);
				selectbox->AddStringToListBox(SETTINGNAME_MIDIOUTPUT,MIDISETTINGS_OUTPUT);
				selectbox->AddStringToListBox(SETTINGNAME_MIDIINPUT,MIDISETTINGS_INPUT);

				selectbox->AddStringToListBox("Automation",AUTOMATIONSETTINGS);
				selectbox->AddStringToListBox(SETTINGNAME_GUI,GUISETTINGS);
				selectbox->AddStringToListBox(SETTINGNAME_FILES,FILESETTINGS);
				selectbox->AddStringToListBox(SETTINGNAME_PROGRAM,PROGRAMSETTINGS);
				selectbox->AddStringToListBox(SETTINGNAME_LANGUAGE,LANGUAGESETTINGS);
				selectbox->AddStringToListBox(SETTINGNAME_KEYS,KEYSSETTINGS);
				selectbox->AddStringToListBox("CamX",INTERNSETTINGS);
			}

			selectbox->CalcScrollWidth();
			selectbox->SetListBoxSelection(settingsselection);
	}
}


void Edit_Settings_Audio::ShowSetAudioDevices()
{
	if(set_audiodevices && mainaudio->selectedaudiohardware)
	{
		int fix=0,ix=0;

		set_audiodevices->ClearListBox();

		AudioDevice *od=mainaudio->selectedaudiohardware->FirstDevice();

		while(od)
		{
			if(mainaudio->GetActiveDevice()==od)
				fix=ix;

			set_audiodevices->AddStringToListBox(od->devname);
			ix++;
			od=od->NextDevice();
		}

		set_audiodevices->SetListBoxSelection(fix);
		set_audiodevices->CalcScrollWidth();
	}
}

void Edit_Settings_Audio::ShowBufferSize()
{
	setSize=audiodevice?audiodevice->GetSetSize():0;
	setSampleRate=mainaudio->GetGlobalSampleRate();

	if(buffersize)
	{
		char h[NUMBERSTRINGLEN];
		buffersize->ChangeButtonText(mainvar->ConvertIntToChar(setSize,h));
	}

	if(buffersizeint)
	{
		buffersizeint->SetInteger(userSetSize=setSize);
	}

	if(lat_in)
		lat_in->SetPos(audiodevice?audiodevice->GetAddToInputLatencySamples():0);

	if(lat_out)
		lat_out->SetPos(audiodevice?audiodevice->GetAddToOutputLatencySamples():0);

}

void Edit_Settings_Audio::RefreshRealtime()
{
	int nsetSize=audiodevice?audiodevice->GetSetSize():0;

	if(nsetSize!=setSize || setSampleRate!=mainaudio->GetGlobalSampleRate())
	{
		ShowBufferSize();
		ShowAudioDeviceInfo();
	}
}

void Edit_Settings_Audio::AddInfoString(AudioDevice *ad,char *name,int index)
{
	char nr[NUMBERSTRINGLEN];

	char *h0=mainvar->GenerateString(name," Size:",ad->buffersettings[index].setSize==0?"-":mainvar->ConvertIntToChar(ad->buffersettings[index].setSize,nr));
	char *h1=mainvar->GenerateString(",min Size:",ad->buffersettings[index].minSize==0?"-":mainvar->ConvertIntToChar(ad->buffersettings[index].minSize,nr));
	char *h2=mainvar->GenerateString(",max Size:",ad->buffersettings[index].maxSize==0?"-":mainvar->ConvertIntToChar(ad->buffersettings[index].maxSize,nr));
	char *h3=mainvar->GenerateString(",pref Size:",ad->buffersettings[index].prefSize==0?"-":mainvar->ConvertIntToChar(ad->buffersettings[index].prefSize,nr));

	char *h=mainvar->GenerateString(h0,h1,h2,h3);
	if(h)
	{
		audioinfo->AddStringToListBox(h);
		delete h;
	}

	if(h0)delete h0;
	if(h1)delete h1;
	if(h2)delete h2;
	if(h3)delete h3;
}

void Edit_Settings_Audio::ShowAudioDeviceInfo()
{
	if(audioinfo && audiodevice)
	{
		char h[NUMBERSTRINGLEN];
		char help[255];

		audioinfo->ClearListBox();
		//	audioinfo->AddStringToListBox(audiodevice->devname);

		mainvar->MixString(help,"Output Channels:",mainvar->ConvertIntToChar(audiodevice->out_channels,h));
		audioinfo->AddStringToListBox(help);

		mainvar->MixString(help,"Input Channels:",mainvar->ConvertIntToChar(audiodevice->in_channels,h));
		audioinfo->AddStringToListBox(help);

		mainvar->MixString(help,"Bits/Sample:",mainvar->ConvertIntToChar(audiodevice->bitresolution,h));
		audioinfo->AddStringToListBox(help);

		if(strlen(audiodevice->info1))
			audioinfo->AddStringToListBox(audiodevice->info1);

		// In latency
		if(char *inl=mainvar->GenerateString(Cxs[CXS_LATENCY]," Input:"))
		{
			mainvar->MixString(help,inl,mainvar->ConvertIntToChar(audiodevice->GetInputLatencySamples_NoAdd(),h));
			audioinfo->AddStringToListBox(help);
			delete inl;
		}

		// Out latency
		if(char *outl=mainvar->GenerateString(Cxs[CXS_LATENCY]," Output:"))
		{
			mainvar->MixString(help,outl,mainvar->ConvertIntToChar(audiodevice->GetOutputLatencySamples_NoAdd(),h));
			audioinfo->AddStringToListBox(help);
			delete outl;
		}

		/*
		int samplerates[]=
		{	
		44100,
		48000,
		88200,
		96000,

		176400,
		192000,
		352800,
		384000
		};
		*/

		if(audiodevice->IsSampleRate(ADSR_44)==true)
			AddInfoString(audiodevice,"44.1 KHz",ADSR_44);

		if(audiodevice->IsSampleRate(ADSR_48)==true)
			AddInfoString(audiodevice,"48 KHz",ADSR_48);

		if(audiodevice->IsSampleRate(ADSR_88)==true)
			AddInfoString(audiodevice,"88.1 KHz",ADSR_88);

		if(audiodevice->IsSampleRate(ADSR_96)==true)
			AddInfoString(audiodevice,"96 KHz",ADSR_96);


		if(audiodevice->IsSampleRate(ADSR_176)==true)
			AddInfoString(audiodevice,"176 KHz",ADSR_176);

		if(audiodevice->IsSampleRate(ADSR_192)==true)
			AddInfoString(audiodevice,"192 KHz",ADSR_192);

		if(audiodevice->IsSampleRate(ADSR_352)==true)
			AddInfoString(audiodevice,"352 KHz",ADSR_352);

		if(audiodevice->IsSampleRate(ADSR_384)==true)
			AddInfoString(audiodevice,"384 KHz",ADSR_384);

		audioinfo->CalcScrollWidth();
	}

}

void Edit_Settings_Audio::ShowAudioDeviceChannels()
{
	if(audiodevice && audiochannels)
	{
		audiochannels->ClearListView();

		char h2[NUMBERSTRINGLEN];

		// Input
		AudioHardwareChannel *c=audiodevice->FirstInputChannel();
		while(c)
		{
			audiochannels->AddItem(0,mainvar->ConvertIntToChar(c->channelindex+1,h2));
			audiochannels->AddItem(1,mainvar->ConvertIntToChar(c->audiochannelgroup+1,h2));

			audiochannels->AddItem(2,c->hwname);
			audiochannels->AddItem(3,"In");
			audiochannels->AddItem(4,c->GetHardwareInfo());

			if(c->notsupported==true)
				audiochannels->AddItem(5,"No Decoder!");

			c=c->NextChannel();
		}

		c=audiodevice->FirstOutputChannel();
		// Output
		while(c)
		{
			audiochannels->AddItem(0,mainvar->ConvertIntToChar(c->channelindex+1,h2));
			audiochannels->AddItem(1,mainvar->ConvertIntToChar(c->audiochannelgroup+1,h2));

			audiochannels->AddItem(2,c->hwname);
			audiochannels->AddItem(3,"Out");
			audiochannels->AddItem(4,c->GetHardwareInfo());

			if(c->notsupported==true)
				audiochannels->AddItem(5,"No Encoder!");

			c=c->NextChannel();
		}
	}
}

void Edit_Settings_AudioInput::ShowPortChannels()
{
	if(channels)
	{
		channels->ClearListView();

		char h[NUMBERSTRINGLEN];

		if(mainaudio->GetActiveDevice())
		{
			for(int i=0;i<mainaudio->GetActiveDevice()->inputaudioports[selectedtype][selectedport].channels;i++)
			{
				channels->AddItem(0,mainvar->ConvertIntToChar(i+1,h));
				AudioHardwareChannel *hwc=mainaudio->GetActiveDevice()->inputaudioports[selectedtype][selectedport].hwchannel[i];

				channels->AddItem(1,hwc?hwc->hwname:"-");
			}

			channels->SetSelection(selectedchannel);
		}
	}
}

void Edit_Settings_AudioInput::ShowPortInfo()
{

}

void Edit_Settings_AudioInput::ShowPorts()
{
	if(ports)
	{
		ports->ClearListView();

		char h[NUMBERSTRINGLEN];

		if(mainaudio->GetActiveDevice())
		{
			for(int i=0;i<CHANNELSPERPORT;i++)
			{
				ports->AddItem(0,mainvar->ConvertIntToChar(i+1,h));
				ports->AddItem(1,mainaudio->GetActiveDevice()->inputaudioports[selectedtype][i].visible==true?"X":"");			
				ports->AddItem(2,mainaudio->GetActiveDevice()->inputaudioports[selectedtype][i].name);
			}

			ports->SetSelection(selectedport);
		}
	}
}

void Edit_Settings_AudioInput::ShowTypes()
{
	if(type)
	{
		type->ClearListView();

		if(mainaudio->GetActiveDevice())
		{
			for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
				type->AddItem(0,channelchannelsinfo[i]);

			type->SetSelection(selectedtype);
		}
	}
}


void Edit_Settings_AudioInput::RefreshRealtime_Slow()
{
	if(mainaudio->GetActiveDevice())
	{
		if(mainaudio->GetActiveDevice()->inputaudioports[selectedtype][selectedport].hwchannelchanged==true)
		{
			mainaudio->GetActiveDevice()->inputaudioports[selectedtype][selectedport].hwchannelchanged=false;
			ShowPortChannels();

			if(ports)
				ports->Refresh(selectedport);

			mainsettings->Save(0);
		}
	}
}

bool Edit_Settings_AudioInput::GadgetListView(guiGadget_ListView *gl,int x,int y)
{
	if(gl==ports)
	{
		switch(x)
		{
		case 1:
			if(mainaudio->GetActiveDevice())
			{
				mainaudio->GetActiveDevice()->inputaudioports[selectedtype][y].visible=mainaudio->GetActiveDevice()->inputaudioports[selectedtype][y].visible==true?false:true;
				gl->ChangeText(x,y,mainaudio->GetActiveDevice()->inputaudioports[selectedtype][y].visible==true?"X":"");;
			}
			return true;
			break;
		}
	}
	if(gl==channels)
	{
		switch(x)
		{
		case 1:
			TRACE ("Edit_Settings_AudioInputput selected HWC \n");

			if(mainaudio->GetActiveDevice())
			{
				DeletePopUpMenu(true);

				if(popmenu)
				{
					class menu_audioporttochannel:public guiMenu
					{
					public:
						menu_audioporttochannel(AudioPort *p,AudioHardwareChannel *h,int c)
						{
							audioport=p;
							hardware=h;
							channel=c;
						}

						void MenuFunction()
						{
							mainaudio->ConnectAudioPortWithHardwareChannel(audioport,hardware,channel);
						} //

						AudioPort *audioport;
						AudioHardwareChannel *hardware;
						int channel;
					};


					AudioHardwareChannel *h=mainaudio->GetActiveDevice()->FirstInputChannel();
					while(h)
					{
						popmenu->AddFMenu(h->hwname,new menu_audioporttochannel(&mainaudio->GetActiveDevice()->inputaudioports[selectedtype][selectedport],h,y),mainaudio->GetActiveDevice()->inputaudioports[selectedtype][selectedport].hwchannel[y]==h?true:false);
						h=h->NextChannel();
					}

					ShowPopMenu();
				}
			}

			break;
		}
	}

	return false;
}

void Edit_Settings_AudioInput::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_TYPE:
		if(g->index!=selectedtype)
		{
			selectedtype=g->index;
			ShowPorts();
			ShowPortInfo();
			ShowPortChannels();
		}
		break;

	case GID_PORTS:
		if(g->index!=selectedport)
		{
			selectedport=g->index;
			ShowPortChannels();
			ShowPortInfo();
		}
		break;

	case GID_CHANNELS:
		selectedchannel=g->index;
		break;
	}

}
void Edit_Settings_AudioInput::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_INPUTCHANNELS,GID_AI,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE/3,-1,"Info:",GID_DEVICEINFOSTRING_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER);
	glist.AddLX();
	infostring=glist.AddString(-1,-1,DEFSIZE,-1,GID_DEVICEINFOSTRING,MODE_RIGHT,0,0,"Port Info");
	glist.Return();

	type=glist.AddListView(-1,-1,DEFSIZE,-1,GID_TYPE,MODE_BOTTOM);

	if(type)
	{
		type->AddColume(Cxs[CXS_TYPE],14,true);
	}
	glist.AddLX();
	glist.AddLX(2);

	ports=glist.AddListView(-1,-1,3*DEFSIZE,-1,GID_PORTS,MODE_BOTTOM);

	if(ports)
	{
		ports->AddColume("Port",4);
		ports->AddColume(Cxs[CXS_ACTIVATED],6);
		ports->AddColume("Info",10,true);
	}

	glist.AddLX();
	glist.AddLX(2);

	channels=glist.AddListView(-1,-1,-1,-1,GID_CHANNELS,MODE_RIGHT|MODE_BOTTOM);

	if(channels)
	{
		channels->AddColume("Channel",4);
		channels->AddColume("Device Hardware",20,true);
	}

	ShowTypes();
	ShowPorts();
	ShowPortInfo();
	ShowPortChannels();
}

void Edit_Settings_AudioOutput::ShowPortChannels()
{
	if(channels)
	{
		channels->ClearListView();

		char h[NUMBERSTRINGLEN];

		if(mainaudio->GetActiveDevice())
		{
			for(int i=0;i<mainaudio->GetActiveDevice()->outputaudioports[selectedtype][selectedport].channels;i++)
			{
				channels->AddItem(0,mainvar->ConvertIntToChar(i+1,h));
				AudioHardwareChannel *hwc=mainaudio->GetActiveDevice()->outputaudioports[selectedtype][selectedport].hwchannel[i];

				channels->AddItem(1,hwc?hwc->hwname:"-");
			}

			channels->SetSelection(selectedchannel);
		}
	}
}

void Edit_Settings_AudioOutput::ShowPortInfo()
{

}

void Edit_Settings_AudioOutput::ShowPorts()
{
	if(ports)
	{
		ports->ClearListView();

		char h[NUMBERSTRINGLEN];

		if(mainaudio->GetActiveDevice())
		{
			for(int i=0;i<CHANNELSPERPORT;i++)
			{
				ports->AddItem(0,mainvar->ConvertIntToChar(i+1,h));
				ports->AddItem(1,mainaudio->GetActiveDevice()->outputaudioports[selectedtype][i].visible==true?"X":"");
				ports->AddItem(2,mainaudio->GetActiveDevice()->outputaudioports[selectedtype][i].name);
			}

			ports->SetSelection(selectedport);
		}
	}
}

void Edit_Settings_AudioOutput::ShowTypes()
{
	if(type)
	{
		type->ClearListView();

		if(mainaudio->GetActiveDevice())
		{
			for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
				type->AddItem(0,channelchannelsinfo[i]);

			type->SetSelection(selectedtype);
		}
	}
}

void Edit_Settings_AudioOutput::RefreshRealtime_Slow()
{
	if(mainaudio->GetActiveDevice())
	{
		if(mainaudio->GetActiveDevice()->outputaudioports[selectedtype][selectedport].hwchannelchanged==true)
		{
			mainaudio->GetActiveDevice()->outputaudioports[selectedtype][selectedport].hwchannelchanged=false;

			ShowPortChannels();

			if(ports)
			{
				ports->ChangeText(2,selectedport,mainaudio->GetActiveDevice()->outputaudioports[selectedtype][selectedport].name);
				ports->Refresh(selectedport);
			}

			mainsettings->Save(0);
		}
	}
}

bool Edit_Settings_AudioOutput::GadgetListView(guiGadget_ListView *gl,int x,int y)
{
	if(gl==ports)
	{
		switch(x)
		{
		case 1:
			if(mainaudio->GetActiveDevice())
			{
				mainaudio->GetActiveDevice()->outputaudioports[selectedtype][y].visible=mainaudio->GetActiveDevice()->outputaudioports[selectedtype][y].visible==true?false:true;
				gl->ChangeText(x,y,mainaudio->GetActiveDevice()->outputaudioports[selectedtype][y].visible==true?"X":"");
			}
			return true;
			break;
		}
	}
	else
		if(gl==channels)
		{
			switch(x)
			{
			case 1:
				TRACE ("Edit_Settings_AudioOutput selected HWC \n");

				if(mainaudio->GetActiveDevice())
				{
					DeletePopUpMenu(true);

					if(popmenu)
					{
						class menu_audioporttochannel:public guiMenu
						{
						public:
							menu_audioporttochannel(AudioPort *p,AudioHardwareChannel *h,int c)
							{
								audioport=p;
								hardware=h;
								channel=c;
							}

							void MenuFunction()
							{
								mainaudio->ConnectAudioPortWithHardwareChannel(audioport,hardware,channel);
							} //

							AudioPort *audioport;
							AudioHardwareChannel *hardware;
							int channel;
						};


						AudioHardwareChannel *h=mainaudio->GetActiveDevice()->FirstOutputChannel();
						while(h)
						{
							popmenu->AddFMenu(h->hwname,new menu_audioporttochannel(&mainaudio->GetActiveDevice()->outputaudioports[selectedtype][selectedport],h,y),mainaudio->GetActiveDevice()->outputaudioports[selectedtype][selectedport].hwchannel[y]==h?true:false);
							h=h->NextChannel();
						}

						ShowPopMenu();
					}
				}

				break;
			}
		}

		return false;
}

void Edit_Settings_AudioOutput::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_TYPE:
		if(g->index!=selectedtype)
		{
			selectedtype=g->index;
			ShowPorts();
			ShowPortInfo();
			ShowPortChannels();
		}
		break;

	case GID_PORTS:
		if(g->index!=selectedport)
		{
			selectedport=g->index;
			ShowPortChannels();
			ShowPortInfo();
		}
		break;

	case GID_CHANNELS:
		selectedchannel=g->index;
		break;
	}
}

void Edit_Settings_AudioOutput::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_OUTPUTCHANNELS,GID_AO,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE/3,-1,"Info:",GID_DEVICEINFOSTRING_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER);
	glist.AddLX();
	infostring=glist.AddString(-1,-1,DEFSIZE,-1,GID_DEVICEINFOSTRING,MODE_RIGHT,0,0,"Port Info");
	glist.Return();

	type=glist.AddListView(-1,-1,DEFSIZE,-1,GID_TYPE,MODE_BOTTOM);

	if(type)
	{
		type->AddColume(Cxs[CXS_TYPE],14,true);
	}
	glist.AddLX();
	glist.AddLX(2);

	ports=glist.AddListView(-1,-1,3*DEFSIZE,-1,GID_PORTS,MODE_BOTTOM);

	if(ports)
	{
		ports->AddColume("Port",4);
		ports->AddColume(Cxs[CXS_ACTIVATED],6);
		ports->AddColume("Info",10,true);
	}

	glist.AddLX();

	glist.AddLX(2);

	channels=glist.AddListView(-1,-1,-1,-1,GID_CHANNELS,MODE_RIGHT|MODE_BOTTOM);

	if(channels)
	{
		channels->AddColume("Channel",4);
		channels->AddColume("Device Hardware",20,true);
	}

	ShowTypes();
	ShowPorts();
	ShowPortChannels();
	ShowPortInfo();
}

void Edit_Settings_Project::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_PRO_NAME:
		editor->editproject->SetProjectName(g->string,true);
		break;
	}
}

void Edit_Settings_Project::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),"Project",GID_PROJECT,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	int iw=bitmap.GetTextWidth(Cxs[CXS_PROJECTNAME])+maingui->GetFontSizeY();

	glist.AddButton(-1,-1,iw,-1,Cxs[CXS_PROJECTNAME],GID_PRONAMETEXT,MODE_NOMOUSEOVER|MODE_ADDDPOINT,0);
	glist.AddLX();
	pname=glist.AddString(-1,-1,-1,-1,GID_PRO_NAME,MODE_RIGHT,0,editor->editproject->name);
	glist.Return();

	glist.AddButton(-1,-1,iw,-1,"/",GID_PRODIRECTORY_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT,0);
	glist.AddLX();
	glist.AddButton(-1,-1,-1,-1,editor->editproject->projectdirectory,GID_PRODIRECTORY,MODE_RIGHT|MODE_INFO,0);
}

void Edit_Settings_ProjectAudio::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_SR:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_psr:public guiMenu
				{
				public:
					menu_psr(Edit_Settings *s,int w)
					{
						editor=s;
						srate=w;
					}

					void MenuFunction()
					{
						mainaudio->SetGlobalSampleRate(srate);

					} //

					Edit_Settings *editor;
					int srate;
				};

				int samplerates[]=
				{	
					44100,
					48000,
					88200,
					96000,

					176400,
					192000,
					352800,
					384000
				};

				AudioDevice *device=mainaudio->GetActiveDevice();

				for(int i=0;i<8;i++)
				{
					if((!device) || (device->CheckSampleRate(samplerates[i])==true))
					{
						char h2[NUMBERSTRINGLEN];
						popmenu->AddFMenu(mainvar->ConvertIntToChar(samplerates[i],h2),new menu_psr(editor,samplerates[i]),editor->editproject->projectsamplerate==samplerates[i]?true:false);
					}
				}

				ShowPopMenu();
			}
		}
		break;

	case GID_PANLAW:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_panlaw:public guiMenu
				{
				public:
					menu_panlaw(Seq_Project *p,int w)
					{
						project=p;
						law=w;
					}

					void MenuFunction()
					{
						if(project)
						{
							project->panlaw.SetLaw(law);
							project->Save(0);
						}
					} //

					Seq_Project *project;
					int law;
				};

				popmenu->AddFMenu("0 dB",new menu_panlaw(editor->editproject,PanLaw::PAN_DB_0),editor->editproject->panlaw.panorama_db==PanLaw::PAN_DB_0?true:false);
				popmenu->AddFMenu("-3 dB",new menu_panlaw(editor->editproject,PanLaw::PAN_DB_3),editor->editproject->panlaw.panorama_db==PanLaw::PAN_DB_3?true:false);
				popmenu->AddFMenu("-4.5 dB",new menu_panlaw(editor->editproject,PanLaw::PAN_DB_4_5),editor->editproject->panlaw.panorama_db==PanLaw::PAN_DB_4_5?true:false);
				popmenu->AddFMenu("-6 dB",new menu_panlaw(editor->editproject,PanLaw::PAN_DB_6),editor->editproject->panlaw.panorama_db==PanLaw::PAN_DB_6?true:false);

				ShowPopMenu();
			}
		}
		break;

	}
}

void Edit_Settings_ProjectAudio::ShowProjectSampleRate()
{
	projectsample=editor->editproject->projectsamplerate;

	if(pro_samplerate)
	{
		char h2[NUMBERSTRINGLEN];
		pro_samplerate->ChangeButtonText(mainvar->ConvertIntToChar(editor->editproject->projectsamplerate,h2));
	}
}

void Edit_Settings_ProjectAudio::ShowProjectPanLaw()
{
	if(pro_panlaw)
	{
		switch(p_db=editor->editproject->panlaw.panorama_db)
		{
		case PanLaw::PAN_DB_0:
			pro_panlaw->ChangeButtonText("0 dB");
			break;
		case PanLaw::PAN_DB_3:
			pro_panlaw->ChangeButtonText("-3 dB");
			break;
		case PanLaw::PAN_DB_4_5:
			pro_panlaw->ChangeButtonText("-4.5 dB");
			break;
		case PanLaw::PAN_DB_6:
			pro_panlaw->ChangeButtonText("-6 dB");
			break;
		}
	}
}

void Edit_Settings_ProjectAudio::RefreshRealtime()
{
	if(projectsample!=editor->editproject->projectsamplerate)
		ShowProjectSampleRate();

	if(p_db!=editor->editproject->panlaw.panorama_db)
		ShowProjectPanLaw();
}

void Edit_Settings_ProjectAudio::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),"Project AUDIO",GID_PROJECTA,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	int iw=0;

	if(char *h=mainvar->GenerateString("Project ",Cxs[CXS_SAMPLINGRATE]))
	{
		iw=bitmap.GetTextWidth(h)+2*maingui->GetFontSizeY();
		glist.AddButton(-1,-1,iw,-1,h,GID_SRI,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
		glist.AddLX();

		pro_samplerate=glist.AddButton(-1,-1,-1,-1,GID_SR,MODE_RIGHT|MODE_MENU,h);
		ShowProjectSampleRate();
		glist.Return();
		delete h;
	}

	glist.AddButton(-1,-1,iw,-1,"Pan Law",GID_PANLAWI,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	pro_panlaw=glist.AddButton(-1,-1,-1,-1,GID_PANLAW,MODE_RIGHT|MODE_MENU);
	ShowProjectPanLaw();

	/*

	y=maingui->AddFontY(y);
	guiGadget *g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x2,y+maingui->GetFontSizeY(),GADGET_PRO_XBOXRECORDPEAK,Cxs[CXS_CHECKAUDIORECORDPEAK],0);

	if(g)
	g->SetCheckBox(editproject->checkaudiostartpeak);

	pro_xboxthreshold=settingsgadgetlist->AddButton(s_x3,y,e_x3,y+maingui->GetFontSizeY(),0,GADGET_PRO_XBOXRECORDTHRESHOLD,0,Cxs[CXS_CHECKAUDIORECORDPEAK]);

	y=maingui->AddFontY(y);
	g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x2,y+maingui->GetFontSizeY(),GADGET_PRO_CUTRECORDZEROSAMPLES,Cxs[CXS_AUTOCUTZEROSAMPLES],0);

	if(g)
	g->SetCheckBox(editproject->autocutzerosamples);

	pro_xboxrecthreshold=settingsgadgetlist->AddButton(s_x3,y,e_x3,y+maingui->GetFontSizeY(),0,GADGET_PRO_XBOXRECORDENDTHRESHOLD,0,Cxs[CXS_AUTOCUTZEROSAMPLES]);

	ShowProjectAudioThresh();
	*/
}

void Edit_Settings_ProjectGUI::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),"Project UI",GID_PROJECTUI,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	int iw=0;

	if(char *h=mainvar->GenerateString(Cxs[CXS_PROJECT]," SMPTE Format"))
	{
		iw=bitmap.GetTextWidth(h)+2*maingui->GetFontSizeY();
		glist.AddButton(-1,-1,iw,-1,h,GID_PROSMPTEI,MODE_NOMOUSEOVER|MODE_ADDDPOINT );
		delete h;
		glist.AddLX();
	}	
	pro_smpteformat=glist.AddCycle(-1,-1,-1,-1,GID_PROSMPTE,MODE_RIGHT,0);

	if(pro_smpteformat)
	{
		int i=0,sel=0;

		while(smptestring[i])
		{
			pro_smpteformat->AddStringToCycle(smptestring[i]);
			if(smptemode[i]==editor->editproject->standardsmpte)
				sel=i;

			i++;
		}

		pro_smpteformat->SetCycleSelection(sel);
	}
	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_PROJECT]," ",Cxs[CXS_MEASUREDISPLAY]))
	{
		glist.AddButton(-1,-1,iw,-1,h,GID_PROMEASUREI,MODE_NOMOUSEOVER|MODE_ADDDPOINT );
		delete h;
		glist.AddLX();
	}

	pro_measureformat=glist.AddCycle(-1,-1,-1,-1,GID_PROMEASURE,MODE_RIGHT,0);

	if(pro_measureformat)
	{
		pro_measureformat->AddStringToCycle("1 1 1 1");
		pro_measureformat->AddStringToCycle("1.1.1.1");
		pro_measureformat->AddStringToCycle("1 1 1 0");
		pro_measureformat->AddStringToCycle("1.1.1.0");
		pro_measureformat->AddStringToCycle("1 1    1");
		pro_measureformat->AddStringToCycle("1.1.   1");
		pro_measureformat->AddStringToCycle("1 1    0");
		pro_measureformat->AddStringToCycle("1.1.   0");

		pro_measureformat->SetCycleSelection(editor->editproject->projectmeasureformat);
	}
}

void Edit_Settings_ProjectGUI::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_PROSMPTE:
		if(editor->editproject)
		{
			editor->editproject->standardsmpte=smptemode[g->index];
			maingui->RefreshSMPTE(editor->editproject);
		}
		break;

	case GID_PROMEASURE:
		if(editor->editproject)
		{
			editor->editproject->projectmeasureformat=mainsettings->defaultprojectmeasureformat=g->index;
			maingui->RefreshMeasure(editor->editproject);
		}
		break;
	}
}

void Edit_Settings_Song::ShowPrefTrackType()
{
	if(preftracktype)
	{
		preftracktype->ChangeButtonText(channelchannelsinfo[WindowSong()->pref_tracktype]);
	}
}

void Edit_Settings_Song::Init()
{
	song=editor->WindowSong();

	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),"Song",GID_SONG,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	int iw=2*bitmap.GetTextWidth(Cxs[CXS_SONGNAME])+maingui->GetFontSizeY();

	glist.AddButton(-1,-1,iw,-1,Cxs[CXS_SONGNAME],GID_SONGNAMETEXT,MODE_NOMOUSEOVER|MODE_ADDDPOINT,0);
	glist.AddLX();
	sname=glist.AddString(-1,-1,-1,-1,GID_SONG_NAME,MODE_RIGHT,0,WindowSong()->GetName());
	glist.Return();

	glist.AddButton(-1,-1,iw,-1,"/",GID_SONGDIRECTORY_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT,0);
	glist.AddLX();
	glist.AddButton(-1,-1,-1,-1,WindowSong()->directoryname,GID_SONGDIRECTORY,MODE_RIGHT|MODE_INFO,0);
	glist.Return();

	glist.AddButton(-1,-1,iw,-1,Cxs[CXS_PREFTRACKTYPE],GID_PTRACKTYPE_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT,0);
	glist.AddLX();
	preftracktype=glist.AddButton(-1,-1,-1,-1,"",GID_PTRACKTYPE,MODE_RIGHT|MODE_MENU,0);
	glist.Return();

	ShowPrefTrackType();

	glist.AddButton(-1,-1,iw,-1,Cxs[CXS_NOTEDISPLAY],GADGET_SONG_NOTEDISPLAY_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT,0);
	glist.AddLX();

	set_songnotes=glist.AddCycle(-1,-1,-1,-1,GADGET_SONG_NOTEDISPLAY,MODE_RIGHT,0);

	if(set_songnotes)
	{
		char *c1,*c2,*c3;

		c1=mainvar->GenerateString(Cxs[CXS_SHOWNOTESAS],":","C-2...A,B");
		c2=mainvar->GenerateString(Cxs[CXS_SHOWNOTESAS],":","C-3...A,H");
		c3=mainvar->GenerateString(Cxs[CXS_SHOWNOTESAS],":","Do-2,Ré-2,Me");

		set_songnotes->AddStringToCycle(c1);
		set_songnotes->AddStringToCycle(c2);
		set_songnotes->AddStringToCycle(c3);

		if(c1)delete c1;
		if(c2)delete c2;
		if(c3)delete c3;

		set_songnotes->SetCycleSelection(WindowSong()->notetype);
	}

	glist.Return();

	glist.AddButton(-1,-1,iw,-1,Cxs[CXS_SMPTEDISPLAYOFFSET],GADGET_SONG_SMPTEOFFSET_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT,0);
	glist.AddLX();
	smpteoffset=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGET_SONG_SMPTEOFFSET,WINDOWDISPLAY_SMPTEOFFSET,MODE_BLACK|MODE_STATICTIME);
}

void Edit_Settings_Song::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_PTRACKTYPE:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{	
				class menu_tt:public guiMenu
				{
				public:
					menu_tt(Edit_Settings_Song *ed,int i)
					{
						editor=ed;
						index=i;
					}

					void MenuFunction()
					{
						if(index>1)index=1;

						editor->WindowSong()->pref_tracktype=index;
						editor->ShowPrefTrackType();
					}

					Edit_Settings_Song *editor;
					int index;
				};


				for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
				{	
					popmenu->AddFMenu(channelchannelsinfo[i],new menu_tt(this,i),WindowSong()->pref_tracktype==i?true:false);
				}

				ShowPopMenu();
			}
		}
		break;

	case GID_SONG_NAME:
		editor->WindowSong()->SetSongName(g->string,true);
		break;

	case GADGET_SONG_NOTEDISPLAY:
		mainsettings->defaultnotetype=editor->WindowSong()->notetype=g->index;
		break;
	}
}

void Edit_Settings_SongSync::ShowSync()
{
	if(syncintern)
	{
		syncintern->SetCheckBox(editor->WindowSong()->MIDIsync.sync==SYNC_INTERN?true:false);
	}

	if(syncmtc)
	{
		syncmtc->SetCheckBox(editor->WindowSong()->MIDIsync.sync==SYNC_MTC?true:false);
	}

	if(syncmc)
	{
		syncmc->SetCheckBox(editor->WindowSong()->MIDIsync.sync==SYNC_MC?true:false);
	}

	if(sendmtc)
	{
		sendmtc->SetCheckBox(editor->WindowSong()->MIDIsync.sendmtc);
	}

	if(sendmc)
	{
		sendmc->SetCheckBox(editor->WindowSong()->MIDIsync.sendmc);
	}
}

void Edit_Settings_SongSync::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_SYNCINTERN:
		editor->WindowSong()->SetSync(SYNC_INTERN);
		ShowSync();
		break;

	case GADGET_SYNCMTC:
		editor->WindowSong()->SetSync(SYNC_MTC);
		ShowSync();
		break;

	case GADGET_SYNCMC:
		editor->WindowSong()->SetSync(SYNC_MC);
		ShowSync();
		break;

	case GADGET_SENDMTC:
		editor->WindowSong()->MIDIsync.sendmtc=editor->WindowSong()->MIDIsync.sendmtc==true?false:true;
		break;

	case GADGET_SENDMC:
		editor->WindowSong()->MIDIsync.sendmc=editor->WindowSong()->MIDIsync.sendmc==true?false:true;
		break;
	}
}

void Edit_Settings_SongSync::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),"Song ",GID_SONGSYNC,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	syncintern=glist.AddCheckBox(-1,-1,-1,-1,GADGET_SYNCINTERN,MODE_RIGHT,"Sync Intern");
	glist.Return();

	syncmtc=glist.AddCheckBox(-1,-1,-1,-1,GADGET_SYNCMTC,MODE_RIGHT,"Sync MTC [Extern]");
	glist.Return();

	syncmc=glist.AddCheckBox(-1,-1,-1,-1,GADGET_SYNCMC,MODE_RIGHT,"Sync MIDI Clock [Extern]");
	glist.Return();

	sendmtc=glist.AddCheckBox(-1,-1,-1,-1,GADGET_SENDMTC,MODE_RIGHT,Cxs[CXS_SENDMTC],Cxs[CXS_SENDMTC_I]);
	glist.Return();

	sendmc=glist.AddCheckBox(-1,-1,-1,-1,GADGET_SENDMC,MODE_RIGHT,Cxs[CXS_SENDMTCSPP],Cxs[CXS_SENDMIDISYNCEVENTS_I]);
	glist.Return();

	ShowSync();
}

void Edit_Settings_Files::ShowRAFSize()
{
	if(rafsize)
	{
		char h2[NUMBERSTRINGLEN];

		char *h=mainvar->GenerateString(mainvar->ConvertIntToChar(rafbuffersize[mainaudio->rafbuffersize_index],h2)," ms");

		if(h)
		{
			rafsize->ChangeButtonText(h);
			delete h;
		}
	}
}

void Edit_Settings_Files::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_AUDIOUNUSED:
		mainsettings->flag_unusedaudiofiles=g->index;
		mainsettings->Save(0);
		break;

	case GADGET_SPLITMIDIFILE:
		mainsettings->splitMIDIfiletype0=g->index;
		mainsettings->Save(0);
		break;

	case GADGET_IMPORTMIDIARR:
		mainsettings->importfilequestion=mainsettings->importfilequestion==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_FILE_RAF:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_rafsize:public guiMenu
				{
				public:
					menu_rafsize(Edit_Settings_Files *ed,int i)
					{
						editor=ed;
						index=i;
					}

					void MenuFunction()
					{
						if(index!=mainaudio->rafbuffersize_index)
						{
							mainaudio->SetGlobalRafSize(index);
							editor->ShowRAFSize();
							mainsettings->Save(0);

						}

					} //

					Edit_Settings_Files *editor;
					int index;
				};

				char h2[NUMBERSTRINGLEN];

				for(int i=0;i<10;i++)
				{
					char *h=mainvar->GenerateString(mainvar->ConvertIntToChar(rafbuffersize[i],h2)," ms");

					if(h)
					{
						popmenu->AddFMenu(h,new menu_rafsize(this,i),i==mainaudio->rafbuffersize_index?true:false);
						delete h;
					}
				}

				ShowPopMenu();
			}
		}
		break;
	}
}

void Edit_Settings_Files::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_FILES,GID_FILES,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	glist.AddButton(-1,-1,2*DEFSIZE,-1,Cxs[CXS_RAFBUFFERSIZE],GID_FILE_RAFI,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	rafsize=glist.AddButton(-1,-1,-1,-1,GID_FILE_RAF,MODE_MENU|MODE_RIGHT,Cxs[CXS_RAFBUFFERSIZE]);
	ShowRAFSize();

	glist.Return();

	if(guiGadget *g=glist.AddCycle(-1,-1,-1,-1,GADGET_MIDIFILE,MODE_RIGHT,0,"SMF MIDI/GM/GS"))
	{
		g->AddStringToCycle(Cxs[CXS_MIDIFILESNOSYSEX]);
		g->AddStringToCycle(Cxs[CXS_MIDIFILESADDGM]);
		g->AddStringToCycle(Cxs[CXS_MIDIFILESADDGS]);

		g->SetCycleSelection(mainsettings->addgsgmtoMIDIfiles);
	}

	glist.Return();

	// Split MIDI File 0->Tracks
	if(guiGadget *g=glist.AddCycle(-1,-1,-1,-1,GADGET_SPLITMIDIFILE,MODE_RIGHT,0,Cxs[CXS_AUTOSPLITMIDIFILE]))
	{
		g->AddStringToCycle(Cxs[CXS_NOAUTOSPLITMIDIFILE]);
		g->AddStringToCycle(Cxs[CXS_AUTOSPLITMIDIFILE]);
		g->AddStringToCycle(Cxs[CXS_AUTOSPLITMIDIFILE_ASK]);

		g->SetCycleSelection(mainsettings->splitMIDIfiletype0);
	}

	glist.Return();

	//
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GADGET_IMPORTMIDIARR,0,Cxs[CXS_IMPORTFILEQUESTION]))
		g->SetCheckBox(mainsettings->importfilequestion);

	glist.Return();

	if(guiGadget *g=glist.AddCycle(-1,-1,-1,-1,GADGET_AUTOSAVE,MODE_RIGHT,0,"Project/Song Auto Save"))
	{
		g->AddStringToCycle(Cxs[CXS_AUTOSAVEOFF]);
		g->AddStringToCycle(Cxs[CXS_AUTOSAVE2MIN]);
		g->AddStringToCycle(Cxs[CXS_AUTOSAVE5MIN]);
		g->AddStringToCycle(Cxs[CXS_AUTOSAVE10MIN]);

		g->SetCycleSelection(mainsettings->autosavemin);
	}

	glist.Return();

	if(guiGadget *g=glist.AddCycle(-1,-1,-1,-1,GADGET_AUDIOUNUSED,MODE_RIGHT,0,0))
	{
		g->AddStringToCycle(Cxs[CXS_UNUSED_1]);
		g->AddStringToCycle(Cxs[CXS_UNUSED_2]);
		g->AddStringToCycle(Cxs[CXS_UNUSED_3]);

		g->SetCycleSelection(mainsettings->flag_unusedaudiofiles);
	}

	/*
	y=maingui->AddFontY(y)+6;



	y=maingui->AddFontY(y)+6;
	g=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_PEAKFILE,0,"Audio Peak");

	if(g)
	{
	g->AddStringToCycle(Cxs[CXS_WRITEPEAKFILE]);
	g->AddStringToCycle(Cxs[CXS_WRITENOPEAKFILE]);
	g->SetCycleSelection(mainsettings->peakfiles);
	}

	y=maingui->AddFontY(y)+6;
	g=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_AUDIOOUTFORMAT,0);

	if(g)
	{
	if(char *h=mainvar->GenerateString(Cxs[CXS_RESAMPLINGFORMAT],":",Cxs[CXS_SAMEASINPUTFILE]))
	{
	g->AddStringToCycle(h);
	delete h;
	}

	if(char *h=mainvar->GenerateString(Cxs[CXS_RESAMPLINGFORMAT],":24 Bit"))
	{
	g->AddStringToCycle(h);
	delete h;
	}

	if(char *h=mainvar->GenerateString(Cxs[CXS_RESAMPLINGFORMAT],":32 Bit"))
	{
	g->AddStringToCycle(h);
	delete h;
	}

	if(char *h=mainvar->GenerateString(Cxs[CXS_RESAMPLINGFORMAT],":64 Bit"))
	{
	g->AddStringToCycle(h);
	delete h;
	}

	g->SetCycleSelection(mainsettings->audioresamplingformat);
	}

	y=maingui->AddFontY(y)+6;


	y=maingui->AddFontY(y)+6;
	g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_IMPORTAUDIO,Cxs[CXS_IMPORTTOAUDIOSONGINPORT]);

	if(g)
	{
	g->SetCheckBox(mainsettings->importaudiofiles);
	}

	y=maingui->AddFontY(y);
	g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_ASKIMPORTAUDIO,Cxs[CXS_IMPORTTOSAMEDIRECTORY]);

	if(g)
	{
	g->SetCheckBox(mainsettings->askimportaudiofiles);
	}

	y=maingui->AddFontY(y);
	g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_IGNORECORRUPTAUDIOFILES,Cxs[CXS_IGNORECORRUPTAUDIOFILES]);

	if(g)
	{
	g->SetCheckBox(mainaudio->ignorecorrectaudiofiles);
	}

	y=maingui->AddFontY(y);
	g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_IMPORTQUESTION,Cxs[CXS_IMPORTFILEQUESTION]);

	if(g)
	{
	g->SetCheckBox(mainsettings->importfilequestion);
	}

	y=maingui->AddFontY(y);
	g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SPLITMIDIFILE,Cxs[CXS_AUTOSPLITMIDIFILE]);

	if(g)
	{
	g->SetCheckBox(mainsettings->splitMIDIfilestype0);
	}
	}
	}
	break;
	*/
}


void Edit_Settings_MIDIInput::ShowPortInfo()
{

	if(infostring)
		infostring->SetString(mainMIDI->MIDIinports[selectedport].info);
}

void Edit_Settings_UI::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_MOUSEHV:
		mainsettings->mouseonlyvertical=mainsettings->mouseonlyvertical==true?false:true;
		break;
	}
}

void Edit_Settings_UI::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_GUI,GID_UI,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_MOUSEHV,MODE_RIGHT,Cxs[CXS_ONLYVERTICALMOUSE]);
	if(g)
		g->SetCheckBox(mainsettings->mouseonlyvertical);
}

void Edit_Settings_Keys::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_KEYS,GID_KEYS,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_SPACE,MODE_RIGHT,Cxs[CXS_CHECKSPACEBACKNONFOCUS]);
	if(g)
		g->SetCheckBox(mainsettings->checkspacenonfocus);
}

void Edit_Settings_Keys::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_SPACE:
		mainsettings->checkspacenonfocus=mainsettings->checkspacenonfocus==true?false:true;
		mainsettings->Save(0);
		break;
	}
}

void Edit_Settings_Automation::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),"Automation",GID_AUTOMATION,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	// Recording
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_RECWHILEPLAYBACK,MODE_RIGHT,Cxs[CXS_AUTOMATIONRECORDINGPLAYBACK]))
		g->SetCheckBox(mainsettings->automationrecordingplayback);

	glist.Return();

	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_RECWHILERECORD,MODE_RIGHT,Cxs[CXS_AUTOMATIONRECORDINGRECORD]))
		g->SetCheckBox(mainsettings->automationrecordingrecord);
	glist.Return();

	// Audio
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_ACAUDIOVOLUME,MODE_RIGHT,Cxs[CXS_AUTOMATIONCREATEAUDIOVOLUME]))
		g->SetCheckBox(mainsettings->automation_createvolumetrack);

	glist.Return();

	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_ACAUDIOPAN,MODE_RIGHT,Cxs[CXS_AUTOMATIONCREATEAUDIOPAN]))
		g->SetCheckBox(mainsettings->automation_createpantrack);

	glist.Return();

	// MIDI
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_MIDIVOLUME,MODE_RIGHT,Cxs[CXS_AUTOMATIONCREATEMIDIVOLUME]))
		g->SetCheckBox(mainsettings->automation_createMIDIvolumetrack);

	glist.Return();
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_MIDIMODULATION,MODE_RIGHT,Cxs[CXS_AUTOMATIONCREATEMIDIMODULATION]))
		g->SetCheckBox(mainsettings->automation_createMIDImodulationtrack);

	glist.Return();
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_MIDIPITCHBEND,MODE_RIGHT,Cxs[CXS_AUTOMATIONCREATEMIDIPITCHBEND]))
		g->SetCheckBox(mainsettings->automation_createMIDIpitchbendtrack);

	glist.Return();
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_MIDIVELOCITY,MODE_RIGHT,Cxs[CXS_AUTOMATIONCREATEMIDIVELOCITY]))
		g->SetCheckBox(mainsettings->automation_createMIDIvelocitytrack);

	glist.Return();
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_MUTE,MODE_RIGHT,Cxs[CXS_AUTOMATIONCREATEMUTE]))
		g->SetCheckBox(mainsettings->automation_createmute);

	glist.Return();
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_SOLO,MODE_RIGHT,Cxs[CXS_AUTOMATIONCREATESOLO]))
		g->SetCheckBox(mainsettings->automation_createsolo);
	glist.Return();

	// Plugins
	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_APLUGINS,MODE_RIGHT,Cxs[CXS_AUTOMATIONPLUGINSCREATE]))
		g->SetCheckBox(mainsettings->automation_createpluginautomationtrack);

	glist.Return();

}

void Edit_Settings_Automation::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_RECWHILEPLAYBACK:
		mainsettings->automationrecordingplayback=mainsettings->automationrecordingplayback==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_RECWHILERECORD:
		mainsettings->automationrecordingrecord=mainsettings->automationrecordingrecord==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_MUTE:
		mainsettings->automation_createmute=mainsettings->automation_createmute==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_SOLO:
		mainsettings->automation_createsolo=mainsettings->automation_createsolo==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_APLUGINS:
		mainsettings->automation_createpluginautomationtrack=mainsettings->automation_createpluginautomationtrack==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_ACAUDIOVOLUME:
		mainsettings->automation_createvolumetrack=mainsettings->automation_createvolumetrack==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_ACAUDIOPAN:
		mainsettings->automation_createpantrack=mainsettings->automation_createpantrack==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_MIDIVOLUME:
		mainsettings->automation_createMIDIvolumetrack=mainsettings->automation_createMIDIvolumetrack==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_MIDIMODULATION:
		mainsettings->automation_createMIDImodulationtrack=mainsettings->automation_createMIDImodulationtrack==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_MIDIPITCHBEND:
		mainsettings->automation_createMIDIpitchbendtrack=mainsettings->automation_createMIDIpitchbendtrack==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_MIDIVELOCITY:
		mainsettings->automation_createMIDIvelocitytrack=mainsettings->automation_createMIDIvelocitytrack==true?false:true;
		mainsettings->Save(0);
		break;
	}
}

void Edit_Settings_MIDIInput::ShowPorts()
{
	if(ports)
	{
		char h[NUMBERSTRINGLEN];

		ports->ClearListView();
		for(int i=0;i<MAXMIDIPORTS;i++)
		{
			ports->AddItem(0,mainvar->ConvertIntToChar(i+1,h));
			ports->AddItem(1,mainMIDI->MIDIinports[i].visible==true?"X":"");
			ports->AddItem(2,mainMIDI->MIDIinports[i].inputdevice?"OK":"- -");
			ports->AddItem(3,mainMIDI->MIDIinports[i].inputdevice?mainMIDI->MIDIinports[i].inputdevice->name:"---");

			ports->AddItem(4,mainMIDI->MIDIinports[i].receivesync==true?"+":"");
			ports->AddItem(5,mainMIDI->MIDIinports[i].receivemtc==true?"+":"");

			ports->AddItem(6,mainMIDI->MIDIinports[i].info);
		}

		ports->SetSelection(selectedport);
	}
}

bool Edit_Settings_MIDIInput::GadgetListView(guiGadget_ListView *gl,int x,int y)
{
	switch(gl->gadgetID)
	{
	case GID_PORTS:
		{
			switch(x)
			{
			case 1:
				{
					mainMIDI->MIDIinports[y].visible=mainMIDI->MIDIinports[y].visible==true?false:true;
					gl->ChangeText(x,y,mainMIDI->MIDIinports[y].visible==true?"X":"");

					mainsettings->Save(0);
					return true;
				}
				break;

			case 4:
				mainMIDI->MIDIinports[y].receivesync=mainMIDI->MIDIinports[y].receivesync==true?false:true;
				gl->ChangeText(x,y,mainMIDI->MIDIinports[y].receivesync==true?"+":"");
				return true;
				break;

			case 5:
				mainMIDI->MIDIinports[y].receivemtc=mainMIDI->MIDIinports[y].receivemtc==true?false:true;
				gl->ChangeText(x,y,mainMIDI->MIDIinports[y].receivemtc==true?"+":"");
				return true;
				break;

			case 3:
				{
					DeletePopUpMenu(true);

					if(popmenu)
					{
						class menu_porttodevice:public guiMenu
						{
						public:
							menu_porttodevice(Edit_Settings_MIDIInput *ed,MIDIInputDevice *mid,int ix)
							{
								editor=ed;
								device=mid;
								index=ix;
							}

							void MenuFunction()
							{
								mainMIDI->MIDIinports[index].SetDevice(device,true);
								editor->ShowPorts();
								editor->ShowPortInfo();
							} //

							Edit_Settings_MIDIInput *editor;
							MIDIInputDevice *device;
							int index;
						};

						MIDIInputDevice *mid=mainMIDI->FirstMIDIInputDevice();

						while(mid)
						{
							popmenu->AddFMenu(mid->name?mid->name:"?",new menu_porttodevice(this,mid,y),mainMIDI->MIDIinports[selectedport].inputdevice==mid?true:false);
							mid=mid->NextInputDevice();
						}

						ShowPopMenu();
					}

				}
				break;
			}
		}
		break;
	}

	return false;
}

void Edit_Settings_MIDIInput::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_DEVICEINFOSTRING:
		mainMIDI->MIDIinports[selectedport].SetInfo(g->string);
		ShowPorts();
		break;

	case GID_PORTS:
		if(g->index!=selectedport)
		{
			selectedport=g->index;
			ShowPortInfo();
		}
		break;
	}
}



void Edit_Settings_MIDIInput::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_MIDIINPUT,GID_MIDIINPUT,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE/3,-1,"Info:",GID_DEVICEINFOSTRING_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER);
	glist.AddLX();

	infostring=glist.AddString(-1,-1,DEFSIZE,-1,GID_DEVICEINFOSTRING,MODE_RIGHT,0,0,"Port Info");
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE,-1,"Ports<->Devices",GID_MIDIDEVICEINFO_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	ports=glist.AddListView(-1,-1,-1,-1,GID_PORTS,MODE_RIGHT|MODE_BOTTOM);

	if(ports)
	{
		ports->AddColume("Port",4);
		ports->AddColume(Cxs[CXS_ACTIVATED],4);
		ports->AddColume(Cxs[CXS_HARDWAREFOUND],8);
		ports->AddColume("MIDI Input Device",12);
		ports->AddColume("MIDI Sync",6);
		ports->AddColume("MTC",6);
		ports->AddColume("Info",20);
	}

	ShowPorts();
	ShowPortInfo();

}


void Edit_Settings_MIDIOutput::ShowPortInfo()
{
	if(infostring)
		infostring->SetString(mainMIDI->MIDIoutports[selectedport].info);
}

bool Edit_Settings_MIDIOutput::GadgetListView(guiGadget_ListView *gl,int x,int y)
{
	switch(gl->gadgetID)
	{
	case GID_PORTS:
		{
			switch(x)
			{
			case 3:
				{
					DeletePopUpMenu(true);

					if(popmenu)
					{
						class menu_porttodevice:public guiMenu
						{
						public:
							menu_porttodevice(Edit_Settings_MIDIOutput *ed,MIDIOutputDevice *mid,int i)
							{
								editor=ed;
								device=mid;
								index=i;
							}

							void MenuFunction()
							{
								mainMIDI->MIDIoutports[index].SetDevice(device,true);
								editor->ShowPorts();
								editor->ShowPortInfo();
							} //

							Edit_Settings_MIDIOutput *editor;
							MIDIOutputDevice *device;
							int index;
						};

						MIDIOutputDevice *mid=mainMIDI->FirstMIDIOutputDevice();

						while(mid)
						{
							popmenu->AddFMenu(mid->name?mid->name:"?",new menu_porttodevice(this,mid,y),mid==mainMIDI->MIDIoutports[y].outputdevice?true:false);
							mid=mid->NextOutputDevice();
						}

						ShowPopMenu();
					}
				}
				break;

			case 1:
				{
					mainMIDI->MIDIoutports[y].visible=mainMIDI->MIDIoutports[y].visible==true?false:true;
					gl->ChangeText(x,y,mainMIDI->MIDIoutports[y].visible==true?"X":"");

					mainsettings->Save(0);
					return true;
				}
				break;

			case 4:
				{
					mainMIDI->MIDIoutports[y].sendsync=mainMIDI->MIDIoutports[y].sendsync==true?false:true;
					gl->ChangeText(x,y,mainMIDI->MIDIoutports[y].sendsync==true?"+":"");

					mainsettings->Save(0);

					return true;
				}
				break;

			case 5:
				{
					mainMIDI->MIDIoutports[y].sendmtc=mainMIDI->MIDIoutports[y].sendmtc==true?false:true;
					gl->ChangeText(x,y,mainMIDI->MIDIoutports[y].sendmtc==true?"+":"");

					mainsettings->Save(0);

					return true;
				}
				break;

			}//switch x
		}
		break;
	}

	return false;
}

void Edit_Settings_MIDIOutput::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_DEVICEINFOSTRING:
		mainMIDI->MIDIoutports[selectedport].SetInfo(g->string);
		ShowPorts();
		break;

		/*
		case GID_PORTACTIVATE:

		break;
		*/

	case GID_PORTS:
		if(g->index!=selectedport)
		{
			selectedport=g->index;
			ShowPortInfo();
		}
		break;

	case GID_DEVICEINFO:

		break;

	}
}

void Edit_Settings_MIDIOutput::ShowPorts()
{
	if(ports)
	{
		char h[NUMBERSTRINGLEN];

		ports->ClearListView();

		for(int i=0;i<MAXMIDIPORTS;i++)
		{
			ports->AddItem(0,mainvar->ConvertIntToChar(i+1,h));
			ports->AddItem(1,mainMIDI->MIDIoutports[i].visible==true?"X":"");
			ports->AddItem(2,mainMIDI->MIDIoutports[i].outputdevice?"OK":"- -");
			ports->AddItem(3,mainMIDI->MIDIoutports[i].outputdevice?mainMIDI->MIDIoutports[i].outputdevice->name:"---");

			ports->AddItem(4,mainMIDI->MIDIoutports[i].sendsync==true?"+":"");
			ports->AddItem(5,mainMIDI->MIDIoutports[i].sendmtc==true?"+":"");
			ports->AddItem(6,mainMIDI->MIDIoutports[i].info);
		}

		ports->SetSelection(selectedport);
	}
}

void Edit_Settings_MIDIOutput::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_MIDIOUTPUT,GID_MIDIOUTPUT,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	//portactive=glist.AddButton(-1,-1,DEFSIZE,-1,Cxs[CXS_ACTIVATED],GID_PORTACTIVATE,MODE_TOGGLE,Cxs[CXS_ACTIVATED]);
	//glist.AddLX();

	glist.AddButton(-1,-1,DEFSIZE/3,-1,"Info:",GID_DEVICEINFOSTRING_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER);
	glist.AddLX();
	infostring=glist.AddString(-1,-1,DEFSIZE,-1,GID_DEVICEINFOSTRING,MODE_RIGHT,0,0,"Port Info");
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE,-1,"Ports<->Devices",GID_MIDIDEVICEINFO_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	ports=glist.AddListView(-1,-1,-1,-1,GID_PORTS,MODE_RIGHT|MODE_BOTTOM);

	if(ports)
	{
		ports->AddColume("Port",4);
		ports->AddColume(Cxs[CXS_ACTIVATED],4);
		ports->AddColume(Cxs[CXS_HARDWAREFOUND],10);
		ports->AddColume("MIDI Output Device",12);
		ports->AddColume("MIDI Sync",6);
		ports->AddColume("MTC",6);
		ports->AddColume("Info",20);
	}

	ShowPorts();
	ShowPortInfo();
}


void Edit_Settings_MIDI::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_SENDCYCLENOTES:
		mainMIDI->sendnoteprevcylce=mainMIDI->sendnoteprevcylce==true?false:true;
		mainsettings->Save(0);
		break;
	}
}

void Edit_Settings_MIDI::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_MIDI,GID_MIDI,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	sendcyclenotes=glist.AddCheckBox(-1,-1,-1,-1,GID_SENDCYCLENOTES,MODE_RIGHT,Cxs[CXS_SENDCYCLENOTES]);
	if(sendcyclenotes)
		sendcyclenotes->SetCheckBox(mainMIDI->sendnoteprevcylce);
}

void Edit_Settings_Metronom::SendMetro(int i)
{
	if(mainvar->GetActiveSong())
	{
		Note note;
		MIDIOutputDevice *device=0;

		if(i==1){

			note.status=NOTEON|(mainsettings->defaultmetrochl_m-1);
			note.key=mainsettings->defaultmetrokey_m;
			note.velocity=mainsettings->defaultmetrovelo_m;
			note.velocityoff=mainsettings->defaultmetrovelooff_m;
			device=mainMIDI->MIDIoutports[mainsettings->defaultmetroport_m].outputdevice;
		}
		else{

			note.status=NOTEON|(mainsettings->defaultmetrochl_b-1);
			note.key=mainsettings->defaultmetrokey_b;
			note.velocity=mainsettings->defaultmetrovelo_b;
			note.velocityoff=mainsettings->defaultmetrovelooff_b;
			device=mainMIDI->MIDIoutports[mainsettings->defaultmetroport_b].outputdevice;
		}

		note.ostart=0;
		note.off.ostart=TICK32nd; // Metro Click Length

		note.SendToDevicePlaybackUser(device,mainvar->GetActiveSong(),Seq_Event::STD_CREATEREALEVENT);
	}
}

void Edit_Settings_Metronom::ShowMetroAudio()
{
	if(maudiotype)
	{
		maudiotype->ChangeButtonText(channelchannelsinfo[mainsettings->defaultmetroaudiochanneltype]);
	}

	if(maudioport && mainaudio->GetActiveDevice())
	{
		maudioport->ChangeButtonText(mainaudio->GetActiveDevice()->outputaudioports[mainsettings->defaultmetroaudiochanneltype][mainsettings->defaultmetroaudiochannelportindex].name);
	}
}

void Edit_Settings_Metronom::ShowMetroPorts()
{
	char nrs[NUMBERSTRINGLEN];
	char *h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(mainsettings->defaultmetroport_b+1,nrs),":",mainMIDI->MIDIoutports[mainsettings->defaultmetroport_b].GetName());

	if(mport)
		mport->ChangeButtonText(h);

	if(h)
		delete h;

	h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(mainsettings->defaultmetroport_m+1,nrs),":",mainMIDI->MIDIoutports[mainsettings->defaultmetroport_m].GetName());

	if(mhport)
		mhport->ChangeButtonText(h);

	if(h)
		delete h;
}


void Edit_Settings_Metronom::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_MAPORT:
		if(mainaudio->GetActiveDevice())
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{	
				class menu_cp:public guiMenu
				{
				public:
					menu_cp(Edit_Settings_Metronom *ed,int i)
					{
						editor=ed;
						index=i;
					}

					void MenuFunction()
					{
						mainsettings->defaultmetroaudiochannelportindex=index;
						editor->ShowMetroAudio();
					}

					Edit_Settings_Metronom *editor;
					int index;
				};


				for(int i=0;i<CHANNELSPERPORT;i++)
				{	
					if(mainaudio->GetActiveDevice()->outputaudioports[mainsettings->defaultmetroaudiochanneltype][i].visible==true)
					{
						popmenu->AddFMenu(mainaudio->GetActiveDevice()->outputaudioports[mainsettings->defaultmetroaudiochanneltype][i].name,new menu_cp(this,i),mainsettings->defaultmetroaudiochannelportindex==i?true:false);
					}
				}

				ShowPopMenu();
			}
		}
		break;

	case GID_MATYPE:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{	
				for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
				{
					class menu_ct:public guiMenu
					{
					public:
						menu_ct(Edit_Settings_Metronom *ed,int i)
						{
							editor=ed;
							type=i;
						}

						void MenuFunction()
						{
							mainsettings->defaultmetroaudiochanneltype=type;
							editor->ShowMetroAudio();
						}

						Edit_Settings_Metronom *editor;
						int type;
					};

					popmenu->AddFMenu(channelchannelsinfo[i],new menu_ct(this,i),mainsettings->defaultmetroaudiochanneltype==i?true:false);
				}

				ShowPopMenu();
			}
		}
		break;

	case GID_SENDMETROMIDI:
		mainsettings->defaultmetrosendMIDI=mainsettings->defaultmetrosendMIDI==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_SENDMETROAUDIO:
		mainsettings->defaultmetrosendaudio=mainsettings->defaultmetrosendaudio==true?false:true;
		mainsettings->Save(0);
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
					menu_addgroup(Edit_Settings_Metronom *ed,int t,int p)
					{
						editor=ed;
						to=t;
						port=p;
					}

					void MenuFunction()
					{
						if(to==0)
							mainsettings->defaultmetroport_b=port;
						else
							mainsettings->defaultmetroport_m=port;

						editor->SendMetro(to);
						editor->ShowMetroPorts();
					}

					Edit_Settings_Metronom *editor;
					int to,port;
				};

				for(int i=0;i<MAXMIDIPORTS;i++)
				{
					if(mainMIDI->MIDIoutports[i].visible==true)
					{
						if(char *h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(i+1,nr),":",mainMIDI->MIDIoutports[i].GetName()))
						{
							if(g->gadgetID==GID_MPORT)
								popmenu->AddFMenu(h,new menu_addgroup(this,0,i),mainsettings->defaultmetroport_b==i?true:false);
							else
								popmenu->AddFMenu(h,new menu_addgroup(this,1,i),mainsettings->defaultmetroport_m==i?true:false);

							delete h;
						}

					}
				}

				ShowPopMenu();
			}
		}
		break;

	case GID_MCHL:
		mainsettings->defaultmetrochl_b=g->GetPos();
		SendMetro(0);
		break;

	case GID_MKEY:
		mainsettings->defaultmetrokey_b=g->GetPos();
		SendMetro(0);
		break;

	case GID_MVELO:
		mainsettings->defaultmetrovelo_b=g->GetPos();
		SendMetro(0);
		break;

	case GID_MVELOOFF:
		mainsettings->defaultmetrovelooff_b=g->GetPos();
		SendMetro(0);
		break;

	case GID_MHCHL:
		mainsettings->defaultmetrochl_m=g->GetPos();
		SendMetro(1);
		break;

	case GID_MHKEY:
		mainsettings->defaultmetrokey_m=g->GetPos();
		SendMetro(1);
		break;

	case GID_MHVELO:
		mainsettings->defaultmetrovelo_m=g->GetPos();
		SendMetro(1);
		break;

	case GID_MHVELOOFF:
		mainsettings->defaultmetrovelooff_m=g->GetPos();
		SendMetro(1);
		break;
	}
}

void Edit_Settings_Metronom::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),Cxs[CXS_METRONOME],GID_METRO,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	// Metro
	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME],"->","MIDI"))
	{
		guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_SENDMETROMIDI,MODE_RIGHT,h);
		if(g)
			g->SetCheckBox(mainsettings->defaultmetrosendMIDI);
		delete h;
	}

	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME],"->","AUDIO"))
	{
		guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_SENDMETROAUDIO,MODE_RIGHT,h);
		if(g)
			g->SetCheckBox(mainsettings->defaultmetrosendaudio);
	}

	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME]," MIDI ",Cxs[CXS_BEAT]))
	{
		glist.AddButton(-1,-1,-1,2*maingui->GetButtonSizeY(),h,GID_MHEAD,MODE_TEXTCENTER|MODE_RIGHT,"Click -> Play");
		delete h;
	}

	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Chl (b)",GID_MCHL_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,-1,-1,GID_MCHL,1,16,mainsettings->defaultmetrochl_b,NUMBER_MIDICHANNEL,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Port (b)",GID_MPORT_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mport=glist.AddButton(-1,-1,-1,-1,GID_MPORT,MODE_MENU|MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Key (b)",GID_MKEY_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,-1,-1,GID_MKEY,0,127,mainsettings->defaultmetrokey_b,NUMBER_KEYS,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Velo (b)",GID_MVELO_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,-1,-1,GID_MVELO,1,127,mainsettings->defaultmetrovelo_b,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI VeloOff (b)",GID_MVELOOFF_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,-1,-1,GID_MVELOOFF,0,127,mainsettings->defaultmetrovelooff_b,NUMBER_INTEGER,MODE_RIGHT);

	// Metro Hi
	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME]," MIDI ",Cxs[CXS_MEASURE]))
	{
		glist.AddButton(-1,-1,-1,2*maingui->GetButtonSizeY(),h,GID_MHHEAD,MODE_TEXTCENTER|MODE_RIGHT,"Click -> Play");
		delete h;
	}
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Chl",GID_MHCHL_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,-1,-1,GID_MHCHL,1,16,mainsettings->defaultmetrochl_m,NUMBER_MIDICHANNEL,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Port",GID_MHPORT_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	mhport=glist.AddButton(-1,-1,-1,-1,GID_MHPORT,MODE_MENU|MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Key",GID_MHKEY_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,-1,-1,GID_MHKEY,0,127,mainsettings->defaultmetrokey_m,NUMBER_KEYS,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI Velo",GID_MHVELO_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,-1,-1,GID_MHVELO,1,127,mainsettings->defaultmetrovelo_m,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();

	glist.AddLX(DEFSIZE/4);
	glist.AddButton(-1,-1,DEFSIZE,-1,"MIDI VeloOff",GID_MHVELOOFF_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,-1,-1,GID_MHVELOOFF,0,127,mainsettings->defaultmetrovelooff_m,NUMBER_INTEGER,MODE_RIGHT);

	ShowMetroPorts();

	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME]," Audio Track ",Cxs[CXS_TYPE]))
	{
		glist.AddButton(-1,-1,DEFSIZE+DEFSIZE/4,-1,h,GID_MATYPE_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
		delete h;
	}

	glist.AddLX();
	maudiotype=glist.AddButton(-1,-1,-1,-1,GID_MATYPE,MODE_MENU|MODE_RIGHT);

	glist.Return();

	if(char *h=mainvar->GenerateString(Cxs[CXS_METRONOME]," Audio Port"))
	{
		glist.AddButton(-1,-1,DEFSIZE+DEFSIZE/4,-1,h,GID_MAPORT_I,MODE_ADDDPOINT|MODE_NOMOUSEOVER);
		delete h;
	}

	glist.AddLX();
	maudioport=glist.AddButton(-1,-1,-1,-1,GID_MAPORT,MODE_MENU|MODE_RIGHT);

	ShowMetroAudio();
}

void Edit_Settings_Plugins::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
#ifdef TRYCATCH
	case GID_CHECKPLUGINS:
		{
			mainsettings->plugincheck=g->index;
			mainsettings->Save(0);
		}
		break;
#endif

	case GID_PLIST:
		{
			selection=g->index;

			Directory *idir=mainsettings->FirstVSTDirectory(vstindex);
			int ix=selection;

			while(idir && ix>=0)
			{
				if(ix==0)
				{
					if(idir!=activevstdirectory)
					{
						activevstdirectory=idir;
						ShowDeleteButton();
						break;
					}
				}

				idir=idir->NextDirectory();
				ix--;
			}

		}
		break;

	case GID_ADD:
		{
			camxFile dir;

			if(dir.SelectDirectory(this,0,Cxs[CXS_ADDVSTDIRECTORY])==true)
			{
				if(Directory *vstdir=mainsettings->AddVSTDirectory(vstindex,dir.filereqname))
				{
					mainsettings->Save(0);

					add->ChangeButtonText(Cxs[CXS_CHECKING]);
					mainaudio->CollectVSTPlugins(vstindex,vstdir,false);
					add->ChangeButtonText(Cxs[CXS_ADDDIR]);

					ShowVSTDirectories();
					maingui->RefreshProjectScreens(0);
				}
			}

			dir.Close(true);
		}
		break;

	case GID_DELETE:
		if(activevstdirectory && activevstdirectory->dontdelete==false)
		{
			activevstdirectory=mainsettings->DeleteVSTDirectory(vstindex,activevstdirectory);
			ShowVSTDirectories();
			mainsettings->Save(0);
		}
		break;
	}
}

void Edit_Settings_Plugins::Init(char *h,int index)
{
	activevstdirectory=0;
	selection=0;
	vstindex=index;

	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),h,GID_PLUGIN,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

#ifdef TRYCATCH
	checkplugins=glist.AddCycle(-1,-1,-1,-1,GID_CHECKPLUGINS,MODE_RIGHT,0);
	if(checkplugins)
	{
		/*
		PLUGINCHECK_NOCHECKS,
		PLUGINCHECK_TRYCATCH,
		PLUGINCHECK_TRYCATCHANDTIMER
		*/

		checkplugins->AddStringToCycle(Cxs[CXS_VSTNOCHECKS]);
		checkplugins->AddStringToCycle(Cxs[CXS_VSTTRYCATCH]);
		checkplugins->AddStringToCycle(Cxs[CXS_VSTTRYCATCHANDTIMER]);

		checkplugins->SetCycleSelection(mainsettings->plugincheck);
	}

	glist.Return();
#endif


	add=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_ADDDIR],GID_ADD,MODE_TEXTCENTER|MODE_LEFTTOMID);
	del=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_DELETE],GID_DELETE,MODE_TEXTCENTER|MODE_MIDTORIGHT);
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE,-1,Cxs[CXS_VSTPLUGINDIRS],GID_PINFO,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	vstdirectory=glist.AddListBox(-1,-1,-1,(MINSETTINGSEDITORHEIGHT-3)*maingui->GetFontSizeY(),GID_PLIST,MODE_RIGHT|MODE_BOTTOM);
	glist.Return();

	ShowVSTDirectories();
	//			vstdelete=settingsgadgetlist->AddButton(s_x2+h+1,height-29,e_x3,height,Cxs[CXS_DELETE],GADGET_DELETEAUDIODIRECTORY,0,Cxs[CXS_REMOVEAUDIODIR_I]);

}

void Edit_Settings_Plugins::ShowDeleteButton()
{
	if(!del)
		return;

	if(!activevstdirectory)
	{
		del->Disable();
		return;
	}

	if(activevstdirectory->dontdelete==true)
	{
		del->Disable();
		return;
	}

	del->Enable();
}

void Edit_Settings_Plugins::ShowVSTDirectories()
{
	if(vstdirectory)
	{
		vstdirectory->ClearListBox();

		Directory *vd=mainsettings->FirstVSTDirectory(vstindex);

		if(!activevstdirectory)
			activevstdirectory=vd;

		if(vd)
		{
			while(vd)
			{
				if(vd->dir)
				{
					char *h=0;

					if(vd->vstcount)
					{
						char h2[NUMBERSTRINGLEN];
						h=mainvar->GenerateString(vd->dir,":","VST PlugIns:",mainvar->ConvertIntToChar(vd->vstcount,h2));
					}
					else
						h=mainvar->GenerateString(vd->dir," - ",Cxs[CXS_NOVSTPLUGINS]);

					if(h)
					{
						vstdirectory->AddStringToListBox(h);
						delete h;
					}

				}
				else
					vstdirectory->AddStringToListBox("???");

				vd=vd->NextDirectory();
			}

			vstdirectory->CalcScrollWidth();
			vstdirectory->SetListBoxSelection(selection);


			//	vstdirectories->SetListBoxSelection(mainsettings->GetOfVSTDir(index,activevstdirectory));


		}
		else
		{
			vstdirectory->AddStringToListBox(Cxs[CXS_NOVSTDIRS]);
			vstdirectory->Disable();
		}
	}

	ShowDeleteButton();
}

void Edit_Settings_VST2::Init()
{
	Edit_Settings_Plugins::Init(SETTINGNAME_VSTDIRECTORY,VST2);
}

void Edit_Settings_VST3::Init()
{
	Edit_Settings_Plugins::Init(SETTINGNAME_VSTDIRECTORY3,VST3);
}

void Edit_Settings_VSTFX::ShowVSTEffects()
{
	if(vstview)
	{
		int fix=0,ix=0;
		vstview->ClearListView();

		char nr[NUMBERSTRINGLEN];
		char nr1[NUMBERSTRINGLEN];

		for(int i=0;i<2;i++)
		{
			VSTPlugin *vst=0;

			if(i==0 && (flag&SHOW_VSTFX))
				vst=mainaudio->FirstVSTEffect();

			if(i==1 && (flag&SHOW_VSTINSTRUMENTS))
				vst=mainaudio->FirstVSTInstrument();

			while(vst)
			{
				if((vst->IsInstrument()==true && (flag&SHOW_VSTINSTRUMENTS)) ||
					(vst->IsInstrument()==false && (flag&SHOW_VSTFX))
					)
				{
					if(activevsteffect==0)
						activevsteffect=vst;

					if(activevsteffect==vst)
						fix=ix;

					vstview->AddItem(0,vst->GetEffectName());

					char *h=mainvar->GenerateString(mainvar->ConvertIntToChar(vst->GetInputPins(),nr),"/",mainvar->ConvertIntToChar(vst->GetOutputPins(),nr1));
					if(h)
					{
						vstview->AddItem(1,h);
						delete h;
					}

					vstview->AddItem(2,vst->IsActive()==true?Cxs[CXS_ON]:Cxs[CXS_OFF]);

					vstview->AddItem(3,vst->IsInstrument()==true?"Instr":"FX");
					vstview->AddItem(4,vst->GenerateInfoString());

					//Version
					double hv=vst->version;
					hv/=1000;

					vstview->AddItem(5,mainvar->ConvertDoubleToChar(hv,nr,4));

					if(vst->directory)
						vstview->AddItem(6,vst->directory->dir);

					/*
					if(char *vstinfo=vst->GenerateInfoString())
					{
					if(char *h=mainvar->GenerateString(i==0?"E:":"I:",vst->GetEffectName()," ",vstinfo))
					{
					vstdirectory->AddStringToListBox(h);
					delete h;
					}

					delete vstinfo;
					}
					*/
				}

				vst=vst->NextVSTPlugin();
			}		
		}

		vstview->SetSelection(fix);

		//ShowVSTInfo(true);
	}
}

void Edit_Settings_VSTFX::ShowInfo()
{
	if(activevsteffect)
	{
		if(info)
			info->ChangeButtonText(activevsteffect->fulldllname);
	}
	else
	{
		if(info)
			info->Disable();
	}
}

bool Edit_Settings_VSTFX::GadgetListView(guiGadget_ListView *gl,int x,int y)
{
	switch(gl->gadgetID)
	{
	case GID_PLIST:
		{
			int ix=0;

			for(int i=0;i<2;i++)
			{
				VSTPlugin *vst=0;

				if(i==0 && (flag&SHOW_VSTFX))
					vst=mainaudio->FirstVSTEffect();

				if(i==1 && (flag&SHOW_VSTINSTRUMENTS))
					vst=mainaudio->FirstVSTInstrument();

				while(vst)
				{
					if(ix==y)
					{
						switch(x)
						{
						case 2:
							vst->plugin_active=vst->plugin_active==true?false:true;
							gl->ChangeText(x,y,vst->plugin_active==true?Cxs[CXS_ON]:Cxs[CXS_OFF]);

							return true;
							break;
						}

						return false;
					}

					ix++;
					vst=vst->NextVSTPlugin();
				}		
			}
		}
		break;
	}

	return false;
}

void Edit_Settings_VSTFX::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_PLIST:
		{
			int ix=0;

			for(int i=0;i<2;i++)
			{
				VSTPlugin *vst=0;

				if(i==0 && (flag&SHOW_VSTFX))
					vst=mainaudio->FirstVSTEffect();

				if(i==1 && (flag&SHOW_VSTINSTRUMENTS))
					vst=mainaudio->FirstVSTInstrument();

				while(vst)
				{
					if(ix==g->index)
					{
						activevsteffect=vst;
						goto show;
					}

					ix++;
					vst=vst->NextVSTPlugin();
				}		
			}

show:
			ShowInfo();
		}
		break;

	case GID_FXVST2:
		{
			if(flag&SHOW_VST2)
				flag CLEARBIT SHOW_VST2;
			else
				flag |=SHOW_VST2;

			activevsteffect=0; // Reset

			g->Toggle(flag&SHOW_VST2?true:false);
			ShowVSTEffects();
			ShowInfo();
		}
		break;

	case GID_FXVST3:
		if(flag&SHOW_VST3)
			flag CLEARBIT SHOW_VST3;
		else
			flag |=SHOW_VST3;

		activevsteffect=0; // Reset
		g->Toggle(flag&SHOW_VST3?true:false);
		ShowVSTEffects();
		ShowInfo();
		break;

	case GID_FXFX:
		if(flag&SHOW_VSTFX)
			flag CLEARBIT SHOW_VSTFX;
		else
			flag |=SHOW_VSTFX;

		activevsteffect=0; // Reset
		g->Toggle(flag&SHOW_VSTFX?true:false);
		ShowVSTEffects();
		ShowInfo();
		break;

	case GID_FXINSTRUMENTS:
		if(flag&SHOW_VSTINSTRUMENTS)
			flag CLEARBIT SHOW_VSTINSTRUMENTS;
		else
			flag |=SHOW_VSTINSTRUMENTS;

		activevsteffect=0; // Reset
		g->Toggle(flag&SHOW_VSTINSTRUMENTS?true:false);	
		ShowVSTEffects();
		ShowInfo();
		break;
	}
}

void Edit_Settings_VSTFX::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_VSTFX,GID_FXINFO,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE,-1,"VST2",GID_FXVST2,flag&SHOW_VST2?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
	glist.AddLX();

	glist.AddButton(-1,-1,DEFSIZE,-1,"VST3",GID_FXVST3,flag&SHOW_VST3?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
	glist.AddLX();

	glist.AddButton(-1,-1,DEFSIZE,-1,"Effects",GID_FXFX,flag&SHOW_VSTFX?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
	glist.AddLX();

	glist.AddButton(-1,-1,DEFSIZE,-1,"Instruments",GID_FXINSTRUMENTS,flag&SHOW_VSTINSTRUMENTS?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE,-1,"Plugin Info",GID_PTINFO,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	info=glist.AddButton(-1,-1,-1,-1,0,GID_PTINFOTEXT,MODE_RIGHT);
	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE,-1,"Plugins",GID_PINFO,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	vstview=glist.AddListView(-1,-1,-1,maingui->GetButtonSizeY(MINSETTINGSEDITORHEIGHT-4),GID_PLIST,MODE_RIGHT|MODE_BOTTOM);

	if(vstview)
	{
		vstview->AddColume(Cxs[CXS_NAME],10);
		vstview->AddColume("Channels",5);
		vstview->AddColume("Status",5);
		vstview->AddColume(Cxs[CXS_TYPE],4);
		vstview->AddColume("Info",15);
		vstview->AddColume("Version",8);

		vstview->AddColume(Cxs[CXS_DIRECTORY],30);
	}

	glist.Return();

	ShowVSTEffects();
	ShowInfo();
}

void Edit_Settings_CrashedPlugins::ShowCrashedPlugins()
{
	if(crashed)
	{
		crashed->ClearListBox();

		if(mainaudio->crashplugins.GetCount())
		{
			// Close Crashed Plugins
			CrashedPlugin *cp=(CrashedPlugin *)mainaudio->crashplugins.GetRoot();
			while(cp)
			{
				crashed->AddStringToListBox(cp->fulldllname);
				cp=(CrashedPlugin *)cp->next;
			}

			crashed->SetListBoxSelection(selected);
			crashed->Enable();

			if(del)
				del->Enable();
		}
		else
		{
			crashed->Disable();

			if(del)
				del->Disable();
		}
	}
}

void Edit_Settings_CrashedPlugins::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_PLIST:
		selected=g->index;
		break;

	case GID_DELETE:
		mainaudio->DeleteCrashedPlugin((CrashedPlugin *)mainaudio->crashplugins.GetO(selected));
		selected=0;
		ShowCrashedPlugins();
		mainsettings->SaveCrashedPlugins();
		break;

	}
}

void Edit_Settings_CrashedPlugins::Init()
{
	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_CRASHEDPLUGINS,GID_CPINFO,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	del=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_DELETE],GID_DELETE,MODE_RIGHT);
	glist.Return();

	crashed=glist.AddListBox(-1,-1,-1,-1,GID_PLIST,MODE_RIGHT|MODE_BOTTOM);

	ShowCrashedPlugins();
}

void Edit_Settings_AudioFiles::ShowAudioDirectories()
{
	// Use VST gadgets
	if(directory)
	{
		directory->ClearListView();

		Directory *vd=mainsettings->FirstAudioDirectory();

		if(!activedirectory)
			activedirectory=vd;

		char h2[NUMBERSTRINGLEN];

		if(vd)
		{
			while(vd)
			{
				directory->AddItem(0,vd->dir);

				if(vd->dir)
				{
					vd->audiofilecount=mainaudio->GetCountOfAudioFilesInDirectoy(vd);
					directory->AddItem(1,mainvar->ConvertIntToChar(vd->audiofilecount,h2));
				}

				vd=vd->NextDirectory();
			}

			directory->Enable();
			directory->SetSelection(activedirectory->GetIndex());
		}
		else
		{
			directory->Disable();
		}
	}
	else
	{
		if(directory)
			directory->Disable();
	}

	if(activedirectory)
	{
		if(del)del->Enable();
	}
	else
	{
		if(del)del->Disable();
	}

}

void Edit_Settings_AudioFiles::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_ADD:
		{
			camxFile dir;

			if(dir.SelectDirectory(this,0,Cxs[CXS_ADDAÙDIOFILESDIR_I])==true)
			{
				if(Directory *adir=mainsettings->AddAudioDirectory(dir.filereqname))
				{
					mainsettings->Save(0);
					mainaudio->CollectAudioFiles(dir.filereqname,adir);
					ShowAudioDirectories();
				}
			}

			dir.Close(true);
		}
		break;

	case GID_DELETE:
		if(activedirectory)
		{
			Directory *np=(Directory *)activedirectory->NextOrPrev();

			mainsettings->DeleteAudioDirectory(activedirectory);
			activedirectory=np;
			ShowAudioDirectories();

			mainsettings->Save(0);
		}
		break;

	case GID_ALIST:
		{
			Directory *idir=mainsettings->FirstAudioDirectory();
			int ix=g->index;

			while(idir && ix>=0)
			{
				if(ix==0)
				{
					if(idir!=activedirectory)
					{
						activedirectory=idir;
						break;
					}
				}

				idir=idir->NextDirectory();
				ix--;
			}
		}
		break;
	}
}

void Edit_Settings_AudioFiles::Init()
{
	activedirectory=0;
	selection=0;

	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),Cxs[CXS_SET_AFILESDIRS],GID_AAF,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	add=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_ADDDIR],GID_ADD,MODE_TEXTCENTER|MODE_LEFTTOMID);
	del=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_DELETE],GID_DELETE,MODE_TEXTCENTER|MODE_MIDTORIGHT);

	glist.Return();

	glist.AddButton(-1,-1,DEFSIZE,-1,Cxs[CXS_SET_AFILESDIRS],GID_AINFO,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	directory=glist.AddListView(-1,-1,-1,(MINSETTINGSEDITORHEIGHT-3)*maingui->GetFontSizeY(),GID_ALIST,MODE_RIGHT|MODE_BOTTOM,Cxs[CXS_LISTOFAUDIOCHANNELS_I]);

	if(directory)
	{
		directory->AddColume(Cxs[CXS_DIRECTORY],20);
		directory->AddColume(Cxs[CXS_FILES],10);
	}

	glist.Return();

	ShowAudioDirectories();
}

void Edit_Settings_Audio::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GID_LI:
		if(mainaudio->GetActiveDevice())
		{
			mainaudio->GetActiveDevice()->SetAddInputLatency(g->GetPos());
		}
		break;

	case GID_LO:
		if(mainaudio->GetActiveDevice())
		{
			mainaudio->GetActiveDevice()->SetAddOutputLatency(g->GetPos());
		}
		break;

	case GID_ASIOQUESTION:
		mainsettings->askforasio=mainsettings->askforasio==true?false:true;
		mainsettings->Save(0);
		break;

	case GID_AUDIOSYSTEM:
		{
			if(mainaudio->SetAudioSystem(g->index)==true)
			{
				audiodevice=mainaudio->GetActiveDevice();

				if(usedevice)usedevice->Disable();

				ShowSetAudioDevices();
				ShowAudioDeviceChannels();
				ShowAudioDeviceInfo();

				ShowBufferSize();

				maingui->RefreshProjectScreens(0);
			}
			else
				maingui->MessageBoxOk(0,Cxs[CXS_UNABLETOSETSYSTEM]);
		}
		break;

	case GID_USEAUDIODEVICE:
		{
			if(mainaudio->selectedaudiohardware && audiodevice && audiodevice!=mainaudio->GetActiveDevice() && mainvar->CheckSampleRateOfNewDevice(audiodevice)==true)
			{
				mainaudio->selectedaudiohardware->SetActiveDevice(audiodevice);
				if(usedevice)usedevice->Disable();
				ShowBufferSize();

				maingui->RefreshProjectScreens(0);
			}
		}
		break;

	case GID_AUDIODEVICES:
		if(mainaudio->selectedaudiohardware)
		{
			AudioDevice *seldev=mainaudio->selectedaudiohardware->GetAudioDeviceIndex(g->index);

			if(seldev!=audiodevice)
			{
				audiodevice=seldev;

				if(audiodevice==mainaudio->GetActiveDevice())
				{
					if(usedevice)usedevice->Disable();
				}
				else
				{
					if(usedevice)usedevice->Enable();
				}

				ShowAudioDeviceChannels();
				ShowAudioDeviceInfo();
			}
		}
		break;

	case GID_BUFFERSIZEINT:
		{
			userSetSize=g->index;
		}break;

	case GID_BUFFERSIZEUSE:
		{
			mainaudio->SetSamplesSize(mainaudio->GetActiveDevice(),userSetSize);
		}break;

	case GID_AUDIOAUTOINPUT:
		mainsettings->autoaudioinput=g->index==0?false:true;
		break;

	case GDI_DEVICEPREFS:
		{
			if(AudioDevice *dev=mainaudio->GetActiveDevice())
			{
				switch(dev->audiosystemtype)
				{
				case AUDIOCAMX_WIN32:
					{
						//maingui->OpenEditorStart(EDITORTYPE_WIN32AUDIO,0,0,0,0,this,0);
						maingui->MessageBoxOk(0,Cxs[CXS_AUDIODEVICEHASNOEDITOR]);
					}
					break;

				case AUDIOCAMX_ASIO:
					ASIOControlPanel();
					break;
				}
			}
		}
		break;

	case GID_BUFFERSIZE:
		{
			if(mainaudio->GetActiveDevice()){

				DeletePopUpMenu(true);

				if(popmenu)
				{
					class menu_devicesetsize:public guiMenu
					{
					public:
						menu_devicesetsize(int s)
						{
							setSize=s;
						}

						void MenuFunction()
						{
							mainaudio->SetSamplesSize(mainaudio->GetActiveDevice(),setSize);
						} //

						int setSize;
					};

					for(int i=0;i<OPTLATENTCY;i++)
					{
						char h2[NUMBERSTRINGLEN];
						popmenu->AddFMenu(mainvar->ConvertIntToChar(optsetsize[i],h2),new menu_devicesetsize(optsetsize[i]),mainaudio->GetActiveDevice()->GetSetSize()==optsetsize[i]?true:false,0);
					}

					ShowPopMenu();
				}
			}
		}break;
	}
}

void Edit_Settings_Audio::Init()
{
	audiodevice=mainaudio->GetActiveDevice();

	glist.SelectForm(0,0);
	glist.AddButton(-1,-1,-1,2*maingui->GetFontSizeY(),SETTINGNAME_AUDIO,GID_AUDIO,MODE_TEXTCENTER|MODE_RIGHT|MODE_BOLD|MODE_NOMOUSEOVER);
	glist.Return();

	set_audiohardware=glist.AddCycle(-1,-1,-1,-1,GID_AUDIOSYSTEM,MODE_RIGHT,0,"Audio Hardware System");

	if(set_audiohardware)
	{
		int fix=0,ix=0;
		AudioHardware *ad=mainaudio->FirstAudioHardware();

		while(ad)
		{
			if(char *h=mainvar->GenerateString("System",":",ad->name))
			{
				set_audiohardware->AddStringToCycle(h);
				if(ad==mainaudio->selectedaudiohardware)
					fix=ix;

				delete h;
			}

			ix++;
			ad=ad->NextHardware();
		}

		set_audiohardware->SetCycleSelection(fix);

		glist.Return();

		glist.AddButton(-1,-1,DEFSIZE,-1,"Devices",GID_AUDIODEVICES_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
		glist.AddLX();

		set_audiodevices=glist.AddListBox(-1,-1,-1,5*maingui->GetFontSizeY(),GID_AUDIODEVICES,MODE_RIGHT,"Audio Devices");
		ShowSetAudioDevices();
		glist.Return();

		glist.AddLX(DEFSIZE);
		usedevice=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_USEDEVICE],GID_USEAUDIODEVICE,MODE_TEXTCENTER|MODE_RIGHT);
		if(usedevice)usedevice->Disable();
		glist.Return();

		glist.AddButton(-1,-1,DEFSIZE,-1,"Device Channels",GID_AUDIODEVICECHANNELS_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
		glist.AddLX();
		audiochannels=glist.AddListView(-1,-1,-1,12*maingui->GetFontSizeY(),GADGET_AUDIOCHANNELID,MODE_RIGHT,Cxs[CXS_LISTOFAUDIOCHANNELS_I]);

		if(audiochannels)
		{
			audiochannels->AddColume("Channel",4);
			audiochannels->AddColume("Group",4);

			audiochannels->AddColume(Cxs[CXS_NAME],10);
			audiochannels->AddColume("I/O",5);
			audiochannels->AddColume("Info",15);
			audiochannels->AddColume("*",15);
		}

		glist.Return();

		ShowAudioDeviceChannels();

		glist.AddButton(-1,-1,DEFSIZE,-1,"Device Info",GID_AUDIODEVICEINFO_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
		glist.AddLX();
		audioinfo=glist.AddListBox(-1,-1,-1,8*maingui->GetFontSizeY(),GID_AUDIOINFOID,MODE_RIGHT,"Info Audio Device",true);
		glist.Return();



		glist.AddButton(-1,-1,DEFSIZE,-1,Cxs[CXS_AUDIOSETDEVICESIZE],GID_BSI,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
		glist.AddLX();

		buffersize=glist.AddButton(-1,-1,-1,-1,GID_BUFFERSIZE,MODE_RIGHT|MODE_MENU,Cxs[CXS_AUDIOSETDEVICESIZE]);
		glist.Return();

		glist.AddLX(DEFSIZE);
		buffersizeint=glist.AddInteger(-1,-1,DEFSIZE,-1,GID_BUFFERSIZEINT,0,0,Cxs[CXS_AUDIOSETDEVICESIZE]);
		glist.AddLX();

		glist.AddButton(-1,-1,-1,-1,Cxs[CXS_USEUSERBLOCKSIZE],GID_BUFFERSIZEUSE,MODE_RIGHT);
		glist.Return();


		if(char *h=mainvar->GenerateString(Cxs[CXS_LATENCY]," Input +/-"))
		{
			glist.AddButton(-1,-1,DEFSIZE,-1,h,GID_LI_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
		}

		glist.AddLX();
		lat_in=glist.AddNumberButton(-1,-1,-1,-1,GID_LI,-1000,1000,0,NUMBER_INTEGER,MODE_RIGHT,"+/-");

		glist.Return();

		if(char *h=mainvar->GenerateString(Cxs[CXS_LATENCY]," Output +/-"))
		{
			glist.AddButton(-1,-1,DEFSIZE,-1,h,GID_LO_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
		}
		glist.AddLX();

		lat_out=glist.AddNumberButton(-1,-1,-1,-1,GID_LO,-1000,1000,0,NUMBER_INTEGER,MODE_RIGHT,"+/-");

		glist.Return();

		ShowAudioDeviceInfo();
		ShowBufferSize();


		devicesettings=glist.AddButton(-1,-1,-1,-1,Cxs[CXS_AUDIOHARDWARESETTINGS_I],GDI_DEVICEPREFS,MODE_TEXTCENTER|MODE_RIGHT);

		glist.Return();

		set_autoaudioinput=glist.AddCycle(-1,-1,-1,-1,GID_AUDIOAUTOINPUT,MODE_RIGHT,0,"Audio Input");
		if(set_autoaudioinput)
		{
			set_autoaudioinput->AddStringToCycle(Cxs[CXS_AUDIOINPUTALWAYSON]);
			set_autoaudioinput->AddStringToCycle(Cxs[CXS_AUDIOINPUTONLYNEED]);

			if(mainsettings->autoaudioinput==true)
				set_autoaudioinput->SetCycleSelection(1);
			else
				set_autoaudioinput->SetCycleSelection(0);
		}
		glist.Return();
	}

	if(guiGadget *g=glist.AddCheckBox(-1,-1,-1,-1,GID_ASIOQUESTION,0,Cxs[CXS_USEASIOQUESTION],Cxs[CXS_USEASIOQUESTION_I]))
		g->SetCheckBox(mainsettings->askforasio);



#ifdef OLDIE

	if(mainaudio->selectedaudiohardware)
	{			
		set_audiodevices=0;
		audiochannels=0;

		audioinfo=0;
		samplerates=
			audiodeviceprefs=
			useaudiodevice=
			set_rafbuffer=
			set_setsize=
			set_setsizecycle=
			set_autoaudioinput=
			set_audiohardware=0;

		if(settingsgadgetlist)
		{
			// Audio Hardware ASIO/Directs....
			int y;


			if(mainaudio->selectedaudiohardware)
			{
				useaudiodevice=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_USEAUDIODEVICEID,Cxs[CXS_USEDEVICE],Cxs[CXS_USEDEVICE_I]);
				y+=maingui->GetFontSizeY();

				audiodeviceprefs=settingsgadgetlist->AddButton(s_x3,y,e_x3,y+2*maingui->GetFontSizeY_Sub()-1,Cxs[CXS_AUDIOHARDWARESETTINGS],GADGET_AUDIOSETTINGS,0,Cxs[CXS_AUDIOHARDWARESETTINGS_I]);
				y+=2*maingui->GetFontSizeY();
			}

			/*
			lockaudiofiles=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_LOCKAUDIOFILESID,"Lock Audio Files");
			y+=maingui->GetFontSizeY();
			*/


			//if(mainaudio->selectedaudiohardware)
			{
				//y+=maingui->GetFontSizeY_Sub()+6;


			}

			set_setsize=settingsgadgetlist->AddButton(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub()+2,GADGET_SETSIZE,0,Cxs[CXS_LATENCYAUDIODEVICE]);
			y+=maingui->GetFontSizeY_Sub()+2;

			set_setsizecycle=settingsgadgetlist->AddCycle(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub()+2,GADGET_SETSIZEQUICK,0,Cxs[CXS_LATENCYAUDIODEVICE]);
			y+=maingui->GetFontSizeY_Sub()+6;

			samplerates=settingsgadgetlist->AddCycle(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub()+2,GADGET_SAMPLERATES,"",Cxs[CXS_AUDIOSYSTEMSAMPLERATE]);
			y+=maingui->GetFontSizeY_Sub()+6;

			set_rafbuffer=settingsgadgetlist->AddCycle(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub()+2,GADGET_RAFBUFFER,"",Cxs[CXS_RAFBUFFERSIZE]);

			y+=maingui->GetFontSizeY_Sub()+6;

			set_autoaudioinput=settingsgadgetlist->AddCycle(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub()+2,GADGET_AUTOAUDIOINPUT,"");

			y+=maingui->GetFontSizeY_Sub()+6+2;

			g_cpucores=settingsgadgetlist->AddText(s_x3,y,e_x3-10,y+maingui->GetFontSizeY_Sub(),0,GADGET_CPUINFO,"Info CPU Cores");
			y+=maingui->GetFontSizeY_Sub()+6;
			//y+=maingui->GetFontSizeY_Sub()*2;


		}
	}

#endif

}

void Edit_Settings::InitEditField()
{
	guiWindow *closeoldeditor=openeditor;

	openeditor=0;

	glist.SelectForm(1,0);

	if(editproject)
	{
		// Project
		switch(settingsselection)
		{
		case PRO_PROJECT:
			openeditor=new Edit_Settings_Project(this);
			break;

		case PRO_PROJECT_AUDIO:
			openeditor=new Edit_Settings_ProjectAudio(this); 
			break;

		case PRO_PROJECT_GUI:
			openeditor=new Edit_Settings_ProjectGUI(this); 
			break;
		}
	}
	else
		if(WindowSong())
		{
			// Song
			switch(settingsselection)
			{
			case SONG_SONG:
				openeditor=new Edit_Settings_Song(this);
				break;

			case SONG_SYNC:
				openeditor=new Edit_Settings_SongSync(this);
				break;
			}
		}
		else
			switch(settingsselection)
		{
			case AUDIOSETTINGS:
				openeditor=new Edit_Settings_Audio(this);
				break;

			case AUDIOSETTINGS_AUDIODIRECTORY:
				openeditor=new Edit_Settings_AudioFiles(this);
				break;

			case MIDISETTINGS:
				openeditor=new Edit_Settings_MIDI(this);
				break;

			case METROSETTINGS:
				openeditor=new Edit_Settings_Metronom(this);
				break;

			case MIDISETTINGS_INPUT:
				openeditor=new Edit_Settings_MIDIInput(this);
				break;

			case MIDISETTINGS_OUTPUT:
				openeditor=new Edit_Settings_MIDIOutput(this);
				break;

			case AUDIOSETTINGS_VSTDIRECTORY:
				openeditor=new Edit_Settings_VST2(this);
				break;

			case AUDIOSETTINGS_VSTDIRECTORY3:
				openeditor=new Edit_Settings_VST3(this);
				break;

			case AUDIOSETTINGS_VSTFX:
				openeditor=new Edit_Settings_VSTFX(this);
				break;

			case AUDIOSETTINGS_CRASHEDPLUGINS:
				openeditor=new Edit_Settings_CrashedPlugins(this);
				break;

			case GUISETTINGS:
				openeditor=new Edit_Settings_UI(this);
				break;

			case KEYSSETTINGS:
				openeditor=new Edit_Settings_Keys(this);
				break;

			case FILESETTINGS:
				openeditor=new Edit_Settings_Files(this);
				break;

			case AUDIOSETTINGS_AUDIOINCHANNELS:
				openeditor=new Edit_Settings_AudioInput(this);
				break;

			case AUDIOSETTINGS_AUDIOOUTCHANNELS:
				openeditor=new Edit_Settings_AudioOutput(this);
				break;

			case AUTOMATIONSETTINGS:
				openeditor=new Edit_Settings_Automation(this);
				break;
		}

		glist.form->BindWindow(openeditor);


		if(closeoldeditor)
			maingui->CloseWindow(closeoldeditor);


}

void Edit_Settings::InitGadgets()
{	
	glist.SelectForm(0,0);
	selectbox=glist.AddListBox(-1,-1,-1,-1,GADGET_SETTINGSID,MODE_TOP|MODE_LEFT|MODE_RIGHT|MODE_BOTTOM);
	InitSelectBox();

	// 
	InitEditField();
}

guiMenu *Edit_Settings::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		menu->AddMenu(Cxs[WindowSong()?CXS_SONGSETTINGS:CXS_SETTINGS],0);
	}

	return menu;
}

void Edit_Settings::ShowMIDIDeviceProgramInfo()
{
	if(activeMIDIprogram)
	{
		if(programchannel)
		{
			programchannel->Enable();

			if(char *h=mainvar->GenerateString(Cxs[CXS_MIDICHANNEL],":",MIDIchannels[activeMIDIprogram->MIDIChannel]))
			{
				programchannel->ChangeButtonText(h);
				delete h;
			}
		}

		if(programbank)
		{
			char t[64];
			char h2[NUMBERSTRINGLEN];

			strcpy(t,"Bank:");
			mainvar->AddString(t,mainvar->ConvertIntToChar(activeMIDIprogram->MIDIBank+1,h2));
			mainvar->AddString(t,activeMIDIprogram->usebanksel==true?" *On*":" (Off)");

			programbank->ChangeButtonText(t);
			programbank->Enable();
		}

		if(programbankuse)
		{
			programbankuse->SetCheckBox(activeMIDIprogram->usebanksel);
			programbankuse->Enable();
		}

		if(programprogram)
		{
			char t[64];
			char h2[NUMBERSTRINGLEN];

			strcpy(t,"Program:");
			mainvar->AddString(t,mainvar->ConvertIntToChar(activeMIDIprogram->MIDIProgram+1,h2));

			programprogram->ChangeButtonText(t);
			programprogram->Enable();
		}
	}
	else
	{
		if(programbankuse)
		{
			programbankuse->SetCheckBox(false);
			programbankuse->Disable();
		}

		if(programchannel)
		{
			programchannel->ChangeButtonText("");
			programchannel->Disable();
		}

		if(programprogram)
		{
			programprogram->ChangeButtonText("");
			programprogram->Disable();
		}

		if(programbank)
		{
			programbank->ChangeButtonText("");
			programbank->Disable();
		}
	}
}

void Edit_Settings::ShowMIDIDevicePrograms()
{
	if(deviceprograms)
	{
		deviceprograms->ClearListBox();

		if(activeMIDIdevice && activeMIDIdevice->FirstProgram())
		{
			if(!activeMIDIprogram)
				activeMIDIprogram=activeMIDIdevice->FirstProgram();

			deviceprograms->Enable();

			DeviceProgram *dp=activeMIDIdevice->FirstProgram();
			while(dp)
			{
				char t[128];

				strcpy(t,dp->name);

				mainvar->AddString(t," ");

				char h2[NUMBERSTRINGLEN];

				if(dp->usebanksel==true)
				{
					mainvar->AddString(t,"BK:");
					mainvar->AddString(t,mainvar->ConvertIntToChar(dp->MIDIBank+1,h2));
					mainvar->AddString(t," ");
				}

				mainvar->AddString(t,"PG:");
				mainvar->AddString(t,mainvar->ConvertIntToChar(dp->MIDIProgram+1,h2));

				deviceprograms->AddStringToListBox(t);
				dp=dp->NextProgram();
			}

			if(activeMIDIprogram)
			{
				deviceprograms->SetListBoxSelection(activeMIDIprogram->GetIndex());
			}

			ShowMIDIDeviceProgramInfo();
			ShowMIDIProgramName();
		}
		else
		{
			activeMIDIprogram=0;
			ShowMIDIDeviceProgramInfo();
			ShowMIDIProgramName();
			deviceprograms->Disable();
		}
	}
}

void Edit_Settings::ShowMIDIProgramName()
{
	if(programname && dontshowMIDIprogramname==false)
	{
		if(activeMIDIprogram)
		{
			programname->Enable();
			programname->SetString(activeMIDIprogram->name);
		}
		else
		{
			programname->SetString("");
			programname->Disable();
		}
	}
}

void Edit_Settings::ShowMIDIDeviceName()
{
	if(MIDIdevicename && dontshowMIDIdevicename==false)
	{
		if(activeMIDIdevice)
		{
			MIDIdevicename->Enable();
			MIDIdevicename->SetString(activeMIDIdevice->name);
		}
		else
		{
			MIDIdevicename->SetString("");
			MIDIdevicename->Disable();
		}
	}
}

void Edit_Settings::ShowDefaultSongLength()
{
	if(songlength)
	{
		char h2[NUMBERSTRINGLEN];
		char *h=mainvar->GenerateString(Cxs[CXS_DEFAULTSONGLENGTH],":",mainvar->ConvertIntToChar(mainsettings->defaultsonglength_measure,h2));

		if(h)
		{
			songlength->ChangeButtonText(h);
			delete h;
		}
	}
}

void Edit_Settings::ShowTargetTracks()
{
	if(set_selecttrack)
	{
		set_selecttrack->ClearListBox();

		if(set_selectinputdevice)
		{
			switch(set_selecttypeindex)
			{
			case SIT_CHANNEL:
			case SIT_TYPE:
				{
					Seq_MIDIRouting_InputDevice *ip=song->inputrouting.FirstInputDevice();

					while(ip)
					{
						if(ip->device==set_selectinputdevice)
						{
							Seq_MIDIRouting_Router_Track *ft=0;

							if(set_selecttypeindex==SIT_CHANNEL)
							{
								// MIDI Channel

								if(set_selectedtrack>=ip->router_channels[set_selectedchannel].tracks.GetCount())
									set_selectedtrack=0;

								ft=ip->router_channels[set_selectedchannel].FirstTrack();
							}
							else
								if(set_selecttypeindex==SIT_TYPE)
								{
									// MIDI Events

									if(set_selectedtrack>=ip->router_events[set_selectedtype].tracks.GetCount())
										set_selectedtrack=0;

									ft=ip->router_events[set_selectedtype].FirstTrack();
								}


								if(set_deletetrack)
								{
									ft?set_deletetrack->Enable():set_deletetrack->Disable();
								}

								// Tracks 
								bool zerotrack=false;

								while(ft)
								{
									char *h;

									if(ft->track)
									{
										char h2[NUMBERSTRINGLEN];
										char *tracknr=mainvar->ConvertIntToChar(WindowSong()->GetOfTrack(ft->track)+1,h2);

										h=mainvar->GenerateString("Track ",tracknr,":",ft->track->GetName());

									}
									else
									{
										zerotrack=true;
										h=mainvar->GenerateString(Cxs[CXS_ACTIVETRACK]);
									}

									if(h)
									{
										set_selecttrack->AddStringToListBox(h);
										delete h;
									}

									ft=ft->NextTrack();
								}

								set_selecttrack->SetListBoxSelection(set_selectedtrack);

								if(set_sendtofocustrack)
								{
									zerotrack==false?set_sendtofocustrack->Enable():set_sendtofocustrack->Disable();
								}
								break;
						}

						ip=ip->NextDevice();
					}
				}
				break;
			}
		}
	}
}

void Edit_Settings::AddRoutingInfo(Seq_MIDIRouting_Router *rt,char *h)
{
	// Add Info
	if(rt->FirstTrack())
	{
		int tracks=rt->tracks.GetCount();

		if(rt->FirstTrack()->track==0)
		{
			mainvar->AddString(h," [AT]");
			tracks--;
		}

		if(tracks)
		{
			char h2[NUMBERSTRINGLEN];

			if(rt->FirstTrack()->track==0)
				mainvar->AddString(h," |+T:");
			else
				mainvar->AddString(h," |T:");

			mainvar->AddString(h,mainvar->ConvertIntToChar(tracks,h2));
			mainvar->AddString(h,"|");
		}
	}
	else
		mainvar->AddString(h," [-]");
}


void Edit_Settings::ShowAudioOutports()
{
	if(audiooutports)
	{
		audiooutports->ClearListBox();

		if(mainaudio->selectedaudiohardware && (audiodevice==mainaudio->selectedaudiohardware->GetActiveDevice()))
		{
			for(int c=0;c<channelschannelsnumber[audiooutporttypeselection];c++)
			{
				AudioPort *port=&audiodevice->outputaudioports[audiooutporttypeselection][outportsselection];

				if(port->hwchannel)
				{
					audiooutports->AddStringToListBox(port->hwchannel[c]->hwname);
				}
				else
					audiooutports->AddStringToListBox("-");

			}
		}
		else
			audiooutports->Disable();

	}
}

void Edit_Settings::ShowSettingsData(int flag)
{
#ifdef OLDIE

	if(WindowSong())
	{
		// Song 
		switch(settingsselection)
		{
		case SONG_GMDISPLAY:
			if(set_songgm)
				set_songgm->SetCycleSelection(WindowSong()->generalMIDI);
			break;

		case SONG_ROUTING:
			if(set_selectrouting)
			{
				set_selectrouting->ClearCycle();

				MIDIInputDevice *ip=mainMIDI->FirstMIDIInputDevice();

				if(ip)
				{
					set_selectrouting->Enable();

					if(!set_selectinputdevice)
						set_selectinputdevice=ip;

					while(ip)
					{
						if(char *h=mainvar->GenerateString("MIDI Input Device:",ip->name))
						{
							set_selectrouting->AddStringToCycle(h);
							delete h;
						}
						else
							set_selectrouting->AddStringToCycle("?");

						ip=ip->NextInputDevice();
					}

					set_selectrouting->SetCycleSelection(mainMIDI->GetOfMIDIInputDevice(set_selectinputdevice));
				}
				else
					set_selectrouting->Disable();
			}

			if(set_selectchannel)
			{
				set_selectchannel->ClearListBox();

				if(set_selectinputdevice)
				{
					Seq_MIDIRouting_InputDevice *fd=WindowSong()->inputrouting.FindInputDevice(set_selectinputdevice);

					set_selectchannel->Enable();

					switch(set_selecttypeindex)
					{
					case SIT_CHANNEL:
						{
							char h[128],h2[NUMBERSTRINGLEN];

							for(int i=0;i<16;i++)
							{
								strcpy(h,Cxs[CXS_MIDICHANNEL]);
								mainvar->AddString(h,":");
								mainvar->AddString(h,mainvar->ConvertIntToChar(i+1,h2));

								if(fd)
									AddRoutingInfo(&fd->router_channels[i],h);

								mainvar->AddString(h," ->");

								set_selectchannel->AddStringToListBox(h);
							}

							set_selectchannel->SetListBoxSelection(set_selectedchannel);
						}
						break;

					case SIT_TYPE:
						{
							char h[128];

							strcpy(h,"Notes ->");
							if(fd)
								AddRoutingInfo(&fd->router_events[Seq_MIDIRouting_InputDevice::NOTES],h);
							set_selectchannel->AddStringToListBox(h);

							strcpy(h,"Program Change ->");
							if(fd)
								AddRoutingInfo(&fd->router_events[Seq_MIDIRouting_InputDevice::PCHANGE],h);
							set_selectchannel->AddStringToListBox(h);

							strcpy(h,"Channel Pressure ->");
							if(fd)
								AddRoutingInfo(&fd->router_events[Seq_MIDIRouting_InputDevice::CPRESS],h);
							set_selectchannel->AddStringToListBox(h);

							strcpy(h,"Poly Pressure ->");
							if(fd)
								AddRoutingInfo(&fd->router_events[Seq_MIDIRouting_InputDevice::PPRESS],h);
							set_selectchannel->AddStringToListBox(h);

							strcpy(h,"Pitchbend ->");
							if(fd)
								AddRoutingInfo(&fd->router_events[Seq_MIDIRouting_InputDevice::PITCH],h);
							set_selectchannel->AddStringToListBox(h);

							strcpy(h,"Control Change ->");
							if(fd)
								AddRoutingInfo(&fd->router_events[Seq_MIDIRouting_InputDevice::CCHANGE],h);
							set_selectchannel->AddStringToListBox(h);

							strcpy(h,"SysEx ->");
							if(fd)
								AddRoutingInfo(&fd->router_events[Seq_MIDIRouting_InputDevice::SYS],h);
							set_selectchannel->AddStringToListBox(h);

							set_selectchannel->SetListBoxSelection(set_selectedtype);
						}
						break;
					}
				}
			}
			else
				set_selectchannel->Disable();

			ShowTargetTracks();
			break;
		}
	}
	else
	{
		switch(settingsselection)
		{
		case AUDIOSETTINGS_PLUGINS:
			{
				if(pluginsettings)
					pluginsettings->SetCycleSelection(mainsettings->plugincheck);
			}
			break;

		case PROGRAMSETTINGS:
			{
				if(MIDIdevices)
				{
					MIDIdevices->ClearListBox();

					SynthDevice *s=mainsettings->synthdevices.FirstDevice();

					if(s)
					{
						if(!activeMIDIdevice)
						{
							activeMIDIdevice=s;
							activeMIDIprogram=s->FirstProgram();
						}

						MIDIdevices->Enable();

						while(s)
						{
							MIDIdevices->AddStringToListBox(s->name);
							s=s->NextDevice();
						}

						if(activeMIDIdevice)
						{
							MIDIdevices->SetListBoxSelection(mainsettings->synthdevices.devices.GetIx(activeMIDIdevice));
						}
					}
					else
					{
						activeMIDIdevice=0;
						activeMIDIprogram=0;
						MIDIdevices->Disable();
					}
				}

				ShowMIDIDevicePrograms();
				ShowMIDIDeviceName();
				ShowMIDIProgramName();
			}
			break;

		case INTERNSETTINGS:
			break;

		case SYNCSETTINGS:
			{
				if(smptedisplayoffset)
				{
					char h[255],h2[NUMBERSTRINGLEN];

					strcpy(h,"SMPTE ");
					mainvar->AddString(h,Cxs[CXS_DISPLAY]);
					mainvar->AddString(h," Offset:");

					mainvar->AddString(h,mainvar->ConvertIntToChar(mainsettings->smpteoffset_h,h2));
					mainvar->AddString(h,":");

					mainvar->AddString(h,mainvar->ConvertIntToChar(mainsettings->smpteoffset_m,h2));
					mainvar->AddString(h,":");

					mainvar->AddString(h,mainvar->ConvertIntToChar(mainsettings->smpteoffset_s,h2));
					mainvar->AddString(h,":");

					mainvar->AddString(h,mainvar->ConvertIntToChar(mainsettings->smpteoffset_f,h2));
					mainvar->AddString(h,";");

					mainvar->AddString(h,mainvar->ConvertIntToChar(mainsettings->smpteoffset_sf,h2));

					smptedisplayoffset->ChangeButtonText(h);
				}

				if(sendMIDIclocksp)
					sendMIDIclocksp->SetCheckBox(mainMIDI->sendMIDIcontrol);

				if(quantsongpositiontoMIDIclockres)
					quantsongpositiontoMIDIclockres->SetCheckBox(mainMIDI->quantizesongpositiontoMIDIclock);

				if(receiveMIDIclocksp)
					receiveMIDIclocksp->SetCheckBox(mainMIDI->receiveMIDIcontrol);

				if(sendmtc)
					sendmtc->SetCheckBox(mainMIDI->sendmtc);

				if(receivemtc)
					receivemtc->SetCheckBox(mainMIDI->receivemtc);
			}
			break;

		case AUDIOSETTINGS_AUDIOINCHANNELS:
			if(audioinlist)
			{
				audioinlist->ClearListBox();

				if(mainaudio->selectedaudiohardware && (audiodevice==mainaudio->selectedaudiohardware->GetActiveDevice()))
				{
					// Input
					AudioHardwareChannel *c=audiodevice->FirstInputChannel();
					while(c)
					{
						//if(h=mainvar->GenerateString(c->name," ",Cxs[CXS_CHANNELS],":",mainvar->ConvertIntToChar(c->channels,h2)) )
						{
							audioinlist->AddStringToListBox(c->hwname);
							//	delete h;
						}

						c=c->NextChannel();
					}

					audioinlist->Enable();


				}
				else
				{
					audioinlist->AddStringToListBox(Cxs[CXS_NOAUDIOINPUTCHLS]);
					audioinlist->Disable();
				}

				if(audioinporttype)
				{
					audioinporttype->ClearCycle();

					for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
					{
						audioinporttype->AddStringToCycle(channelchannelsinfo[i]);
					}

					audioinporttype->SetCycleSelection(audioinporttypeselection);
				}

				if(audioinportlist)
				{
					audioinportlist->ClearListBox();

					if(mainaudio->selectedaudiohardware && (audiodevice==mainaudio->selectedaudiohardware->GetActiveDevice()))
					{
						char h2[32];
						for(int i=0;i<CHANNELSPERPORT;i++)
						{
							char *h=mainvar->GenerateString("Input Port ",mainvar->ConvertIntToChar(i+1,h2),":",audiodevice->inputaudioports[audioinporttypeselection][i].name);
							if(h)
							{
								audioinportlist->AddStringToListBox(h);
								delete h;
							}
						}

						audioinportlist->SetListBoxSelection(inportsselection);
					}
					else
						audioinportlist->Disable();
				}

				ShowAudioInports();
			}
			break;

		case AUDIOSETTINGS_AUDIOOUTCHANNELS:
			if(audiooutlist)
			{
				audiooutlist->ClearListBox();

				if(mainaudio->selectedaudiohardware && (audiodevice==mainaudio->selectedaudiohardware->GetActiveDevice()))
				{
					// Ouput
					AudioHardwareChannel *c=audiodevice->FirstOutputChannel();
					while(c)
					{
						//	if(h=mainvar->GenerateString(c->name," ",Cxs[CXS_CHANNELS],":",mainvar->ConvertIntToChar(c->channels,h2)) )
						{
							audiooutlist->AddStringToListBox(c->hwname);
							//	delete h;
						}

						c=c->NextChannel();
					}

					audiooutlist->Enable();
				}
				else
				{
					audiooutlist->AddStringToListBox(Cxs[CXS_NOAUDIOOUTPUTCHLS]);
					audiooutlist->Disable();
				}

				if(audiooutporttype)
				{
					audiooutporttype->ClearCycle();

					for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
					{
						audiooutporttype->AddStringToCycle(channelchannelsinfo[i]);
					}

					audiooutporttype->SetCycleSelection(audiooutporttypeselection);
				}

				if(audiooutportlist)
				{
					audiooutportlist->ClearListBox();
					if(mainaudio->selectedaudiohardware && (audiodevice==mainaudio->selectedaudiohardware->GetActiveDevice()))
					{
						char h2[32];
						for(int i=0;i<CHANNELSPERPORT;i++)
						{
							char *h=mainvar->GenerateString("Output Port ",mainvar->ConvertIntToChar(i+1,h2),":",audiodevice->outputaudioports[audiooutporttypeselection][i].name);
							if(h)
							{
								audiooutportlist->AddStringToListBox(h);
								delete h;
							}
						}

						audiooutportlist->SetListBoxSelection(outportsselection);
					}
					else
					{
						audiooutportlist->Disable();
					}
				}

				ShowAudioOutports();
			}
			break;


		}
	}
}
break;

			case MIDISETTINGS:
				break;

			case MIDISETTINGS_INPUT:
				//if(MIDIinputdevice)
				//{

				if(MIDIinputports)
				{
					MIDIinputports->ClearListBox();
					for(int i=0;i<MAXMIDIPORTS;i++)
					{
						if(mainMIDI->MIDIinports[i].device)
						{
							if(char *h=mainvar->GenerateString(mainMIDI->MIDIinports[i].name,mainMIDI->MIDIinports[i].device->FullName()))
							{
								MIDIinputports->AddStringToListBox(h);
								delete h;
							}
						}
						else
						{
							if(char *h=mainvar->GenerateString(mainMIDI->MIDIinports[i].name," ---------"))
							{
								MIDIinputports->AddStringToListBox(h);
								delete h;
							}

						}
					}
				}

				if(MIDIinputdevices)
				{
					MIDIinputdevices->ClearListBox();

					if(MIDIInputDevice *id=mainMIDI->FirstMIDIInputDevice())
					{
						while(id){
							MIDIinputdevices->AddStringToListBox(id->FullName());
							id=id->NextInputDevice();
						}

						MIDIinputdevices->SetListBoxSelection(mainMIDI->GetOfMIDIInputDevice(MIDIinputdevice));
					}
					else
					{
						MIDIinputdevices->AddStringToListBox(Cxs[CXS_NOMIDIINPUTDEVICES]);
						MIDIinputdevices->Disable();
					}
				}

				if(MIDIinputstring && (flag!=NO_DEVICEINFOSTRING))
				{
					MIDIinputstring->SetString(MIDIinputdevice->userinfo);
				}

				if(MIDIinputsync)
					MIDIinputsync->SetCheckBox(MIDIinputdevice->receivesync);

				if(MIDIintempoquant)
				{
					char h[255];

					strcpy (h,Cxs[CXS_QUANTIZETEMPO]);
					mainvar->AddString(h,":");

					if(MIDIinputdevice->MIDIclockraster==-1)
						mainvar->AddString(h,Cxs[CXS_OFF]);
					else
						mainvar->AddString(h,quantstr[MIDIinputdevice->MIDIclockraster]);

					MIDIintempoquant->ChangeButtonText(h);
				}
				//}
				/*
				else
				{
				if(MIDIinputstring)
				MIDIinputstring->Disable();

				if(MIDIinputdevices)
				{
				MIDIinputdevices->AddStringToListBox(Cxs[CXS_NOMIDIINPUTDEVICES]);
				MIDIinputdevices->Disable();
				}

				if(MIDIinputfilter)
				MIDIinputfilter->Disable();
				}
				*/
				break;

			case MIDISETTINGS_OUTPUT:
				if(MIDIoutdevice)
				{
					if(MIDIoutputports)
					{
						MIDIoutputports->ClearListBox();
						for(int i=0;i<MAXMIDIPORTS;i++)
						{
							if(mainMIDI->MIDIoutports[i].device)
							{
								if(char *h=mainvar->GenerateString(mainMIDI->MIDIoutports[i].name,mainMIDI->MIDIoutports[i].device->FullName()))
								{
									MIDIoutputports->AddStringToListBox(h);
									delete h;
								}
							}
							else
							{
								if(char *h=mainvar->GenerateString(mainMIDI->MIDIoutports[i].name," ---------"))
								{
									MIDIoutputports->AddStringToListBox(h);
									delete h;
								}

							}
						}
					}

					if(MIDIOutputDevices)
					{
						MIDIOutputDevices->ClearListBox();

						if(MIDIOutputDevice *od=mainMIDI->FirstMIDIOutputDevice())
						{
							while(od)
							{
								if(od==mainMIDI->GetDefaultDevice())
								{
									if(char *h=mainvar->GenerateString(od->FullName(),"(Default)"))
									{
										MIDIOutputDevices->AddStringToListBox(h);
										delete h;
									}
									else
										MIDIOutputDevices->AddStringToListBox(od->name);
								}
								else
									MIDIOutputDevices->AddStringToListBox(od->name);

								od=od->NextOutputDevice();
							}

							MIDIOutputDevices->SetListBoxSelection(mainMIDI->GetOfMIDIOutputDevice(MIDIoutdevice));
						}
						else
						{
							MIDIOutputDevices->AddStringToListBox(Cxs[CXS_NOMIDIOutputDeviceS]);
							MIDIOutputDevices->Disable();
						}
					}

					if(MIDIoutputstring && flag!=NO_DEVICEINFOSTRING)
					{
						MIDIoutputstring->SetString(MIDIoutdevice->userinfo);
					}

					if(defaultdevice)
					{
						if(MIDIoutdevice==mainMIDI->GetDefaultDevice())
						{
							defaultdevice->SetCheckBox(true);
							defaultdevice->Disable();
						}
						else
							defaultdevice->SetCheckBox(false);
					}

					if(MIDIclockout)
						MIDIclockout->SetCheckBox(MIDIoutdevice->sendMIDIcontrol);

					if(mtcout)
						mtcout->SetCheckBox(MIDIoutdevice->sendmtc);

					//	if(MIDIoptimizer)
					//		MIDIoptimizer->SetCheckBox(MIDIoutdevice->MIDIoutputfilter);

					if(MIDIsendsolomutecontrol)
						MIDIsendsolomutecontrol->SetCheckBox(mainMIDI->sendcontrolsolomute);

					if(MIDIoutmetronome)
						MIDIoutmetronome->SetCheckBox(MIDIoutdevice->sendmetronome);

					if(MIDIoutmetrochannel)
						MIDIoutmetrochannel->SetCycleSelection(MIDIoutdevice->sendmetronome_channel);

					if(MIDIoutmetronote)
						MIDIoutmetronote->SetCycleSelection(MIDIoutdevice->sendmetronome_key);

					if(MIDIoutmetrovelo)
						MIDIoutmetrovelo->SetCycleSelection(MIDIoutdevice->sendmetronome_velocity);

					if(MIDIoutmetrochannel_hi)
						MIDIoutmetrochannel_hi->SetCycleSelection(MIDIoutdevice->sendmetronome_channel_hi);

					if(MIDIoutmetronote_hi)
						MIDIoutmetronote_hi->SetCycleSelection(MIDIoutdevice->sendmetronome_key_hi);

					if(MIDIoutmetrovelo_hi)
						MIDIoutmetrovelo_hi->SetCycleSelection(MIDIoutdevice->sendmetronome_velocity_hi);
				}
				/*
				else
				{
				if(MIDIoutputstring)
				MIDIoutputstring->Disable();

				if(MIDIOutputDevices)
				{
				MIDIOutputDevices->AddStringToListBox(Cxs[CXS_NOMIDIOutputDeviceS]);
				MIDIOutputDevices->Disable();
				}

				if(defaultdevice)
				defaultdevice->Disable();

				if(MIDIclockout)
				MIDIclockout->Disable();

				if(mtcout)
				mtcout->Disable();

				if(MIDIoutmetronome)
				MIDIoutmetronome->Disable();

				if(MIDIoutmetrochannel)
				MIDIoutmetrochannel->Disable();

				if(MIDIoutmetronote)
				MIDIoutmetronote->Disable();

				if(MIDIoutmetrovelo)
				MIDIoutmetrovelo->Disable();

				if(MIDIoutmetrochannel_hi)
				MIDIoutmetrochannel_hi->Disable();

				if(MIDIoutmetronote_hi)
				MIDIoutmetronote_hi->Disable();

				if(MIDIoutmetrovelo_hi)
				MIDIoutmetrovelo_hi->Disable();
				}
				*/

				break;
}
		}

#endif
}


void Edit_Settings::ShowActiveTrack()
{
	if(focustrackname)
	{
		delete focustrackname;
		focustrackname=0;
	}

	if(WindowSong())
	{
		switch(settingsselection)
		{
		case SONG_ROUTING:
			if(set_addsongsfocustrack)
			{
				if(WindowSong()->GetFocusTrack())
				{
					focustrackname=mainvar->GenerateString("Add Track",":",WindowSong()->GetFocusTrack()->GetName());

					if(focustrackname)
						set_addsongsfocustrack->ChangeButtonText(focustrackname);

					set_addsongsfocustrack->Enable();
				}
				else
				{
					set_addsongsfocustrack->ChangeButtonText("No Track");
					set_addsongsfocustrack->Disable();

				}
			}
			break;
		}
	}
}

void Edit_Settings::ShowHWDevices()
{
	if(hw_device)
	{
		hw_device->ClearCycle();

		AudioHardware *hw=mainaudio->GetAudioAudioHardware(selected_hw_system);

		if(hw)
		{
			AudioDevice *ad=hw->FirstDevice();

			while(ad)
			{
				if(char *h=mainvar->GenerateString("Device",":",ad->devname))
				{
					hw_device->AddStringToCycle(h);
					delete h;
				}

				ad=ad->NextDevice();
			}

			hw_device->SetCycleSelection(selected_hw_device);
		}
	}
}


void Edit_Settings::ShowProjectAudioThresh()
{
	if(pro_xboxthreshold)
	{
		if(char *h=mainaudio->GenerateDBString(editproject->checkaudiothreshold))
		{
			pro_xboxthreshold->ChangeButtonText(h);
			delete h;
		}
	}

	if(pro_xboxrecthreshold)
	{
		if(char *h=mainaudio->GenerateDBString(editproject->checkaudioendthreshold))
		{
			pro_xboxrecthreshold->ChangeButtonText(h);
			delete h;
		}
	}

}

void Edit_Settings::ShowSettings()
{
#ifdef OLDIE
	if(settingsgadgetlist)settingsgadgetlist->RemoveGadgetList();
	settingsgadgetlist=0;

	set_songname=0;
	if(songnamebuff)delete songnamebuff;
	songnamebuff=0;

	if(WindowSong())
	{
		// SONG
		switch(settingsselection)
		{
		case SONG_SONG:
			{
				int y=45;
				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				if(settingsgadgetlist)
				{
					settingsgadgetlist->AddText(s_x2,y,e_x2,y+maingui->GetFontSizeY(),Cxs[CXS_SONGNAME],0);
					set_songname=settingsgadgetlist->AddString(s_x3,y,e_x3,y+maingui->GetFontSizeY(),GADGET_SONG_NAME,0,WindowSong()->songname);
					songnamebuff=mainvar->GenerateString(WindowSong()->songname);
				}
			}
			break;



		case SONG_GMDISPLAY:
			{
				int y=45;

				set_songgm=0;

				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				if(settingsgadgetlist)
				{
					set_songgm=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY(),GADGET_SONG_GMDISPLAY,0,Cxs[CXS_GMSETTINGS_I]);

					if(set_songgm)
					{
						set_songgm->AddStringToCycle(Cxs[CXS_NOGM]);
						set_songgm->AddStringToCycle(Cxs[CXS_USEGM]);
					}
				}
			}
			break;

		case SONG_METRO:
			{
				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				if(settingsgadgetlist)
				{
					int y=45;

					// Metro Playback
					guiGadget *g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY(),GADGET_SONG_RECMETRO,Cxs[CXS_METRORECORDING],Cxs[CXS_METROCLICKWHILEREC]);

					if(g)
						g->SetCheckBox(WindowSong()->metronome.record);

					y=maingui->AddFontY(y)+2;

					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY(),GADGET_SONG_PLAYMETRO,Cxs[CXS_METROPLAYBACK],Cxs[CXS_METROCLICKWHILEPLAYBACK]);

					if(g)
						g->SetCheckBox(WindowSong()->metronome.playback);

				}
			}
			break;

		case SONG_ROUTING:
			{
				set_selectchannel=set_selecttrack=0;

				set_selectinputtype=
					set_selectrouting=	
					set_addsongsfocustrack=
					set_sendtofocustrack=
					set_deletetrack=0;

				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				if(settingsgadgetlist)
				{
					int hx=e_x3-s_x2;
					hx/=2;

					int y=45;

					// MIDI Input Devices
					set_selectrouting=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY(),GADGET_SONG_SELINPUT,0,"MIDI Input Devices");

					y=maingui->AddFontY(y)+4;

					set_selectinputtype=settingsgadgetlist->AddCycle(s_x2,y,s_x2+hx,y+maingui->GetFontSizeY(),GADGET_SONG_SELINPUTTYPE,0,"MIDI Channels/MIDI Event Types");

					if(set_selectinputtype)
					{
						set_selectinputtype->AddStringToCycle("Event Channel");
						set_selectinputtype->AddStringToCycle("Event Type");

						set_selectinputtype->SetCycleSelection(set_selecttypeindex);
					}

					y=maingui->AddFontY(y)+4;

					int y2=height-4*maingui->GetFontSizeY();

					set_selectchannel=settingsgadgetlist->AddListBox(s_x2,y,s_x2+hx,height,GADGET_SONG_SELINPUTCHANNELS,0,"Device MIDI Channels");
					set_selecttrack=settingsgadgetlist->AddListBox(s_x2+hx+1,y,e_x3,y2,GADGET_SONG_SELINPUTTRACKS,0,"Song Target Track");

					y=y2+4;

					set_addsongsfocustrack=settingsgadgetlist->AddButton(s_x2+hx+1,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SONG_ADDTRACK,0,"Add selected (active) Track");

					y=maingui->AddFontY(y);
					set_sendtofocustrack=settingsgadgetlist->AddButton(s_x2+hx+1,y,e_x3,y+maingui->GetFontSizeY_Sub(),"Send always to active Song Track",GADGET_SONG_ADDACTIVETRACK,0,"Send Events always to Songs active Track");

					y=maingui->AddFontY(y);
					set_deletetrack=settingsgadgetlist->AddButton(s_x2+hx+1,y,e_x3,y+maingui->GetFontSizeY_Sub(),Cxs[CXS_DELETE],GADGET_SONG_DELETETRACK,0,"Remove Track from Routing");
				}
			}
			break;
		}
	}
	else
	{
		// Global

		switch(settingsselection)
		{
		case PROGRAMSETTINGS:
			{
				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				int y=50;

				deviceprograms=0;
				MIDIdevices=0;
				MIDIdevicename=programname=addMIDIdevice=deleteMIDIdevice=addMIDIprogram=deleteMIDIprogram=0;

				programbankuse=programchannel=
					programprogram=
					programbank=0;

				if(settingsgadgetlist)
				{
					int yh;

					MIDIdevices=settingsgadgetlist->AddListBox(s_x2,y,e_x2-1,yh=height-2*(maingui->GetFontSizeY()+2),GADGET_DEVICE,0,"Synthesizer Devices");

					int h=(e_x2-s_x2)/2;

					if(yh>y)
					{
						MIDIdevicename=settingsgadgetlist->AddString(s_x2,yh+1,e_x2,height-maingui->GetFontSizeY(),GADGET_DEVICENAME,0,0,"Device Name");
						addMIDIdevice=settingsgadgetlist->AddButton(s_x2,height-maingui->GetFontSizeY(),s_x2+h-1,height,"Add Device",GADGET_ADDDEVICE,0,"Add MIDI Program Device");
						deleteMIDIdevice=settingsgadgetlist->AddButton(s_x2+h+1,height-maingui->GetFontSizeY(),e_x2,height,"Delete Device",GADGET_DELETEDEVICE,0,"Delete MIDI Program Device");
					}

					deviceprograms=settingsgadgetlist->AddListBox(e_x2+1,y,e_x3,yh=height-6*maingui->GetFontSizeY(),GADGET_DEVICEPROGRAM,0,"Device Programs");	

					if(yh>y)
					{
						programname=settingsgadgetlist->AddString(e_x2,yh+1,e_x3,yh+maingui->GetFontSizeY(),GADGET_PROGRAMNAME,0,0,"Program Name");
						yh+=maingui->GetFontSizeY()+2;
						h=(e_x3-e_x2)/2;
						addMIDIprogram=settingsgadgetlist->AddButton(e_x2,yh,e_x2+h-1,yh+maingui->GetFontSizeY(),"Add Program",GADGET_ADDPROGRAM,0,"Add Program");
						deleteMIDIprogram=settingsgadgetlist->AddButton(e_x2+h+1,yh,e_x3,yh+maingui->GetFontSizeY(),"Delete Program",GADGET_DELETEPROGRAM,0,"Delete Program");

						yh+=maingui->GetFontSizeY();
						programchannel=settingsgadgetlist->AddButton(e_x2,yh,e_x3,yh+maingui->GetFontSizeY(),"",GADGET_PROGRAMCHANNEL,0,"Program Channel");
						yh+=maingui->GetFontSizeY();
						programbank=settingsgadgetlist->AddButton(e_x2,yh,e_x3,yh+maingui->GetFontSizeY(),"",GADGET_PROGRAMBANK,0,"Program Bank Select");
						yh+=maingui->GetFontSizeY();
						programbankuse=settingsgadgetlist->AddCheckBox(e_x2,yh,e_x3,yh+maingui->GetFontSizeY(),GADGET_PROGRAMBANKSEL,"Send Bank Select","Send Bank Select before Program Change");
						yh+=maingui->GetFontSizeY();
						programprogram=settingsgadgetlist->AddButton(e_x2,yh,e_x3,yh+maingui->GetFontSizeY(),"",GADGET_PROGRAMPROGRAM,0,"Program Program Change Number");
					}
				}
			}
			break;

		case INTERNSETTINGS:
			{
				int y=70;

				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				if(settingsgadgetlist)
				{
					guiGadget *g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_AUTOUPDATE,Cxs[CXS_AUTOUPDATE]);
					if(g)
						g->SetCheckBox(mainvar->autoupdatecheck);

					y+=maingui->GetFontSizeY();
				}
			}
			break;

		case SYNCSETTINGS:
			{
				int y=70;

				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				//#
				globalsmpte=
					sendMIDIclocksp=
					quantsongpositiontoMIDIclockres=
					receiveMIDIclocksp=
					sendmtc=
					receivemtc=
					smptedisplayoffset=
					0;

				if(settingsgadgetlist)
				{
					// SMPTE 
					globalsmpte=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY(),GADGET_SYNCGLOBALSMPTE,0);

					if(globalsmpte)
					{
						globalsmpte->AddTooltip("Default SMPTE Frame Rate");
						int i=0,sel=0;
						for(;;)
						{
							if(!smptestring[i])break;

							if(char *h=mainvar->GenerateString("SMPTE:",smptestring[i]))
							{
								globalsmpte->AddStringToCycle(h);
								delete h;

								if(mainsettings->projectstandardsmpte==smptemode[i])
									sel=i;
							}

							i++;
						}

						globalsmpte->SetCycleSelection(sel);

						y+=maingui->GetFontSizeY()+4;
					}

					// SMPTE display offset
					smptedisplayoffset=settingsgadgetlist->AddButton(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SMPTEDISPLAYOFFSET,0,"SMPTE Display Offset");
					y+=maingui->GetFontSizeY();

					// Output
					sendMIDIclocksp=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SYNCOUTPUTMIDICLOCKSP,Cxs[CXS_SENDMTCSPP]);
					y+=maingui->GetFontSizeY();

					quantsongpositiontoMIDIclockres=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_QUANTIZETOSPP,Cxs[CXS_QUANTIZESPP]);
					y+=maingui->GetFontSizeY();

					// Input
					receiveMIDIclocksp=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SYNCINPUTMIDICLOCKSP,Cxs[CXS_RECEIVEMTCSPP]);
					y+=maingui->GetFontSizeY();

					// MTC
					sendmtc=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SENDMTC,Cxs[CXS_SENDMTC]);
					y+=maingui->GetFontSizeY();

					// Input
					receivemtc=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_RECEIVEMTC,Cxs[CXS_RECEIVEMTC]);
				}
			}
			break;



		case AUDIOSETTINGS_AUDIOOUTCHANNELS:
			{
				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				audiooutlist=audiooutportlist=audiooutports=0;
				audiooutporttype=0;

				if(settingsgadgetlist)
				{

					if(settingsgadgetlist)
					{
						int h=height-50;

						h/=3;

						audiooutlist=settingsgadgetlist->AddListBox(s_x2,50,e_x3,50+h,GADGET_AUDIOOUTCHANNELS,0,"Audio Hardware Output Channels");
						audiooutporttype=settingsgadgetlist->AddCycle(s_x2,50+h+2,e_x3,50+h+2+maingui->GetFontSizeY()+2,GADGET_AUDIOOUTCHANNELSPORTTYPE,0);

						int w=e_x3-s_x2;
						w/=2;
						audiooutportlist=settingsgadgetlist->AddListBox(s_x2,50+h+2+maingui->GetFontSizeY()+4,s_x2+w,height-maingui->GetFontSizeY(),GADGET_AUDIOOUTCHANNELSPORTS,0);
						audiooutports=settingsgadgetlist->AddListBox(s_x2+w+1,50+h+2+maingui->GetFontSizeY()+4,e_x3,height-maingui->GetFontSizeY(),GADGET_AUDIOOUTPORTS,0);

						settingsgadgetlist->AddButton(s_x2,height-maingui->GetFontSizeY(),e_x3,height,Cxs[CXS_EDIT],GADGET_AUDIOOUTPORTEDIT);
					}
				}
			}
			break;

		case AUDIOSETTINGS_VSTEFFECTS:
			{
				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				vstdirectories=0;
				vstinfo=0;

				if(settingsgadgetlist)
				{
					vstdirectories=settingsgadgetlist->AddListBox(s_x2,50,e_x3,height-30,GADGET_VSTEFFECTS,0,"VST Audio Effect Plugins");
					vstinfo=settingsgadgetlist->AddText(s_x2,height-27,e_x3,height,0,GADGET_VSTINFO,"VST Audio Effect Information");
					ShowVSTEffects();
				}
			}
			break;

		case AUDIOSETTINGS_VSTINSTRUMENTS:
			{
				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				vstdirectories=0;
				vstinfo=0;

				if(settingsgadgetlist)
				{
					int y=50;

					vstdirectories=settingsgadgetlist->AddListBox(s_x2,y,e_x3,height-30,GADGET_VSTINSTRUMENTS,0,"VST Audio Instrument PlugIns");
					vstinfo=settingsgadgetlist->AddText(s_x2,height-27,e_x3,height,0,GADGET_VSTINFO,"VST Audio Instrument Information");
					ShowVSTInstruments();
				}
			}
			break;


		case AUDIOSETTINGS_PLUGINS:
			{
				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				pluginsettings=0;

				if(settingsgadgetlist)
				{
					int y=50;

					pluginsettings=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY()+2,GADGET_PLUGINS,0);
					if(pluginsettings)
					{
						pluginsettings->AddStringToCycle(Cxs[CXS_NOPLUGINCHECK]);
						pluginsettings->AddStringToCycle(Cxs[CXS_PLUGINCHECK1]);
						pluginsettings->AddStringToCycle(Cxs[CXS_PLUGINCHECK2]);

						y+=maingui->GetFontSizeY()+6;
					}

					guiGadget *g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,maingui->AddFontY(y),GADGET_AUTOINSTRUMENT,Cxs[CXS_AUTOINSTRUMENT],Cxs[CXS_AUTOINSTRUMENT]);

					if(g)
					{
						g->SetCheckBox(mainsettings->autoinstrument);
						y=maingui->AddFontY(y);
					}

					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,maingui->AddFontY(y),GADGET_AUTOTRACKMIDIININSTRUMENT,Cxs[CXS_AUTOTRACKMIDITHRU],Cxs[CXS_AUTOINSTRUMENT]);

					if(g)
					{
						g->SetCheckBox(mainsettings->autotrackMIDIinthru);
						y=maingui->AddFontY(y);
					}
				}
			}
			break;

		case AUDIOSETTINGS_VSTDIRECTORY:
		case AUDIOSETTINGS_VSTDIRECTORY64:
		case AUDIOSETTINGS_VSTDIRECTORY386:
		case AUDIOSETTINGS_VSTDIRECTORY364:
			{
				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				vstdirectories=0;

				vstadd=
					vstdelete=0;

				switch(settingsselection)
				{
				case AUDIOSETTINGS_VSTDIRECTORY:
					vstindex=VSTX86;
					break;

				case AUDIOSETTINGS_VSTDIRECTORY64:
					vstindex=VSTX64;
					break;

				case AUDIOSETTINGS_VSTDIRECTORY386:
					vstindex=VST3X86;
					break;

				case AUDIOSETTINGS_VSTDIRECTORY364:
					vstindex=VST3X64;
					break;
				}

				if(settingsgadgetlist)
				{
					int h=(e_x3-s_x2)/2;

					vstdirectories=settingsgadgetlist->AddListBox(s_x2,50,e_x3,height-30,GADGET_VSTDIRECTORY,0,Cxs[CXS_VSTPLUGINDIRS]);
					vstadd=settingsgadgetlist->AddButton(s_x2,height-29,s_x2+h,height,Cxs[CXS_ADDDIR],GADGET_ADDVSTDIRECTORY,0,Cxs[CXS_ADDVSTDIRECTORY]);
					vstdelete=settingsgadgetlist->AddButton(s_x2+h+1,height-29,e_x3,height,Cxs[CXS_DELETE],GADGET_DELETEVSTDIRECTORY,0,Cxs[CXS_REMOVEVSTDIR_I]);

					ShowVSTDirectories(vstindex);
				}
			}
			break;

			/*
			case AUDIOSETTINGS_VSTDIRECTORY64:
			{
			settingsgadgetlist=gadgetlists.AddGadgetList(this);

			vstdirectories=
			vstadd=
			vstdelete=0;

			if(settingsgadgetlist)
			{
			int h=(e_x3-s_x2)/2;

			vstdirectories=settingsgadgetlist->AddListView(s_x2,50,e_x3,height-30,GADGET_VSTDIRECTORY,Cxs[CXS_VSTPLUGINDIRS]);
			vstadd=settingsgadgetlist->AddButton(s_x2,height-29,s_x2+h,height,Cxs[CXS_ADDDIR],GADGET_ADDVSTDIRECTORY,0,Cxs[CXS_ADDVSTDIRECTORY]);
			vstdelete=settingsgadgetlist->AddButton(s_x2+h+1,height-29,e_x3,height,Cxs[CXS_DELETE],GADGET_DELETEVSTDIRECTORY,0,Cxs[CXS_REMOVEVSTDIR_I]);

			ShowVSTDirectories(VSTX64);
			}
			}
			break;
			*/




		case GUISETTINGS:
			{
				songlength=defaulttime=0;

				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				if(settingsgadgetlist)
				{
					int y=45;

					guiGadget *g=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_GUIEDITOR,0,Cxs[CXS_DOUBLECLICKPATTERN_I]);
					if(g)
					{
						g->AddStringToCycle("Double Click Pattern:Piano Editor");
						g->AddStringToCycle("Double Click Pattern:Event Editor");
						g->AddStringToCycle("Double Click Pattern:Drum Editor");
						g->AddStringToCycle("Double Click Pattern:Wave Editor");
						g->AddStringToCycle("Double Click Pattern:Score Editor");

						g->SetCycleSelection(mainsettings->opendoubleclickeditor);

						y=maingui->AddFontY(y)+8;
					}



					defaulttime=g=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_GUIEDITOR_WINDOWFORMAT,0,Cxs[CXS_DEFAULTTIMEDISPLAY]);

					if(g)
					{
						if(char *h=mainvar->GenerateString(Cxs[CXS_DEFAULTDISPLAYFORMAT],":",Cxs[CXS_MEASURE]))
						{
							g->AddStringToCycle(h);
							delete h;
						}

						if(char *h=mainvar->GenerateString(Cxs[CXS_DEFAULTDISPLAYFORMAT],":","SMPTE"))
						{
							g->AddStringToCycle(h);
							delete h;
						}

						g->SetCycleSelection(defaulttime_status=mainsettings->editordefaulttimeformat);

						y=maingui->AddFontY(y)+8;
					}

					g=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_GUIEDITOR_UNDO,0,Cxs[CXS_UNDOMEMORYUSAGE]);

					if(g)
					{
						if(char *h=mainvar->GenerateString(Cxs[CXS_UNDOSTEPS],":25"))
						{
							g->AddStringToCycle(h);
							delete h;
						}
						if(char *h=mainvar->GenerateString(Cxs[CXS_UNDOSTEPS],":50"))
						{
							g->AddStringToCycle(h);
							delete h;
						}
						if(char *h=mainvar->GenerateString(Cxs[CXS_UNDOSTEPS],":75"))
						{
							g->AddStringToCycle(h);
							delete h;
						}
						if(char *h=mainvar->GenerateString(Cxs[CXS_UNDOSTEPS],":100"))
						{
							g->AddStringToCycle(h);
							delete h;
						}

						g->AddStringToCycle(Cxs[CXS_UNDONOLIMIT]);

						g->SetCycleSelection(mainsettings->undosteps);
						y=maingui->AddFontY(y)+8;
					}	

					g=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_GUIEDITOR_WINDOWGMFORMAT,0,"General MIDI (GM)");

					if(g)
					{
						if(char *h=mainvar->GenerateString("Default General MIDI:",Cxs[CXS_OFF]))
						{
							g->AddStringToCycle(h);
							delete h;
						}

						if(char *h=mainvar->GenerateString("Default General MIDI:",Cxs[CXS_ON]))
						{
							g->AddStringToCycle(h);
							delete h;
						}

						g->SetCycleSelection(mainsettings->eventeditorgmmode);
						y=maingui->AddFontY(y)+8;
					}

					g=settingsgadgetlist->AddCycle(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_GUIEDITOR_ARRANGEFILEDISPLAY,0,Cxs[CXS_DISPLAYAUDIOFILESINARRANGE]);

					if(g)
					{
						g->AddStringToCycle(Cxs[CXS_NOAUDIOFILESNAMESAR]);
						g->AddStringToCycle(Cxs[CXS_AUDIOFILESNAMESAR]);
						g->AddStringToCycle(Cxs[CXS_AUDIOFILESPATHNAMESAR]);

						g->SetCycleSelection(mainsettings->arrangeeditorfiledisplay);
						y=maingui->AddFontY(y)+8;
					}

					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_AUTOLOADPROJECT,Cxs[CXS_LOADLASTUSEDPROJECT]);

					if(g)
						g->SetCheckBox(mainsettings->autoloadlastusedproject);	

					y=maingui->AddFontY(y);

					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SOLOEDITOR,"Solo Editor","Single/Multi Window Editing");
					if(g)
						g->SetCheckBox(mainsettings->soloeditor);
					y=maingui->AddFontY(y);

					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_FOLLOWEDITOR,Cxs[CXS_ALLEDITORSFOLLOWSONGPOSITION],Cxs[CXS_ALLEDITORSFOLLOWSONGPOSITION]);
					if(g)
						g->SetCheckBox(mainsettings->followeditor);

					y=maingui->AddFontY(y);

					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SHOWHEADERBOTH,Cxs[CXS_SHOWMEASUREANDSMPTETIMELINE]);

					if(g)
						g->SetCheckBox(mainsettings->showbothsformatsintimeline);

					y=maingui->AddFontY(y);
					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SHOWOFF,Cxs[CXS_SHOWNOTEOFFSINMONITOR],Cxs[CXS_SHOWNOTEOFFSINMONITOR]);
					if(g)
						g->SetCheckBox(mainsettings->displaynoteoff_monitor);

					y=maingui->AddFontY(y);
					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SHOWTOOLTIP,Cxs[CXS_SHOWEDITORTOOLTIPS]);
					if(g)
						g->SetCheckBox(mainsettings->displayeditortooltip);

					y=maingui->AddFontY(y);
					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_TRANSPORTDESKTOP,Cxs[CXS_TRANSPORTONDESKTOP],Cxs[CXS_TRANSPORTONDESKTOP_I]);
					if(g)
						g->SetCheckBox(mainsettings->transportdesktop);

					y=maingui->AddFontY(y);
					g=settingsgadgetlist->AddCheckBox(s_x2,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_DOUBLESTOP,Cxs[CXS_DOUBLESTOP],Cxs[CXS_DOUBLESTOP]);
					if(g)
						g->SetCheckBox(mainsettings->doublestopeditrefresh);

				}
			}
			break;

		case MIDISETTINGS_OUTPUT:
			{
				if(!MIDIoutdevice)
					MIDIoutdevice=mainMIDI->FirstMIDIOutputDevice();

				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				MIDIoutputports=MIDIOutputDevices=0;

				MIDIoutputstring=

					//sendcyclenotes=
					MIDIoutmetrovelo_hi=
					MIDIoutmetronote_hi=
					MIDIoutmetrochannel_hi=
					MIDIoutmetrovelo=
					MIDIoutmetronote=
					MIDIoutmetrochannel=
					MIDIoutmetronome=
					MIDIclockout=
					mtcout=
					defaultdevice=
					MIDIsendsolomutecontrol=
					MIDIdeviceoutputfilter=
					//MIDIoptimizer=
					0;

				if(settingsgadgetlist)
				{
					int y=50,y2=height-10;

					y2=y+(y2-y)/2;

					MIDIoutputports=settingsgadgetlist->AddListBox(s_x2,y,e_x2,y2-1,GADGET_MIDIPORTID,0,"MIDI Output Ports");
					MIDIOutputDevices=settingsgadgetlist->AddListBox(s_x2,y2,e_x2,height-10,GADGET_MIDIDEVICEID,0,"MIDI Output Devices");

					if(mainMIDI->FirstMIDIOutputDevice())
					{
						defaultdevice=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_DEFAULTDEVICEID,Cxs[CXS_DEFAULTDEVICE],Cxs[CXS_DEFAULTDEVICE_INFO]);
						y+=maingui->GetFontSizeY();

						MIDIsendsolomutecontrol=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SENDALWAYSCONTROL,Cxs[CXS_CONTROLALWAYS],Cxs[CXS_CONTROLALWAYS_I]);
						y+=maingui->GetFontSizeY();

						MIDIdeviceoutputfilter=settingsgadgetlist->AddButton(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub()-1,GetMIDIOutFilterName(),GADGET_MIDIOUTDEVICEFILTER_ID,0,"MIDI Output Device Event Filter");
						y+=maingui->GetFontSizeY();

						if(defaultdevice)
						{
							if(MIDIoutdevice==mainMIDI->GetDefaultDevice())
								defaultdevice->Disable();
						}

						MIDIclockout=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_MIDICLOCKID,Cxs[CXS_SENDMIDISYNCEVENTS],Cxs[CXS_SENDMIDISYNCEVENTS_I]);
						y+=maingui->GetFontSizeY();

						mtcout=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_SENDMTCID,Cxs[CXS_SENDMTC],Cxs[CXS_SENDMTC_I]);
						y+=maingui->GetFontSizeY();

						//MIDIoptimizer=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIOPTIMIZER,"Optimize MIDI Stream");
						//y+=maingui->GetFontSizeY();

						MIDIoutmetronome=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIMETRONOMEID,Cxs[CXS_METRONOME],Cxs[CXS_SENDMETRONOMETOMIDIOUT]);

						y+=maingui->GetFontSizeY();

						int h=e_x3-s_x3;

						h/=3;

						int m_sx1=s_x3;
						int m_ex1=m_sx1+h-1;

						int m_sx2=m_ex1+1;
						int m_ex2=m_sx2+h-1;

						int m_sx3=m_ex2+1;
						int m_ex3=m_sx3+h-1;

						// Metronome Settings
						MIDIoutmetrochannel=settingsgadgetlist->AddCycle(m_sx1,y,m_ex1,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIMETRONOMECHANNELID,0,"MIDI Output Device, Metronome Note Channel");
						if(MIDIoutmetrochannel)
							MIDIoutmetrochannel->AddMIDIChannelsStrings();

						MIDIoutmetronote=settingsgadgetlist->AddCycle(m_sx2,y,m_ex2,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIMETRONOMENOTEID,0,"MIDI Output Device, Metronome Note Key");
						if(MIDIoutmetronote)
							MIDIoutmetronote->AddMIDIKeysStrings(mainvar->GetActiveSong());

						MIDIoutmetrovelo=settingsgadgetlist->AddCycle(m_sx3,y,m_ex3,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIMETRONOMEVELOID,0,"MIDI Output Device, Metronome Note Velocity");
						y+=maingui->GetFontSizeY()+4;
						if(MIDIoutmetrovelo)
							MIDIoutmetrovelo->AddMIDIRange(1,127);

						// metro hi
						MIDIoutmetrochannel_hi=settingsgadgetlist->AddCycle(m_sx1,y,m_ex1,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIMETRONOMECHANNELHIID,0);

						if(MIDIoutmetrochannel_hi)
							MIDIoutmetrochannel_hi->AddMIDIChannelsStrings();

						MIDIoutmetronote_hi=settingsgadgetlist->AddCycle(m_sx2,y,m_ex2,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIMETRONOMENOTEHIID,0);

						if(MIDIoutmetronote_hi)
							MIDIoutmetronote_hi->AddMIDIKeysStrings(mainvar->GetActiveSong());

						MIDIoutmetrovelo_hi=settingsgadgetlist->AddCycle(m_sx3,y,m_ex3,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIMETRONOMEVELOHIID,0);
						y+=maingui->GetFontSizeY()+4;

						if(MIDIoutmetrovelo_hi)
							MIDIoutmetrovelo_hi->AddMIDIRange(1,127);

						y+=maingui->GetFontSizeY();
						MIDIoutputstring=settingsgadgetlist->AddString(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIOUTDEVICEINFO_ID,0,0,"MIDI Output Device Information String");
					}
				}
			}
			break;

		case MIDISETTINGS_INPUT:
			{
				if(!MIDIinputdevice)
					MIDIinputdevice=mainMIDI->FirstMIDIInputDevice();

				settingsgadgetlist=gadgetlists.AddGadgetList(this);

				int y=50;

				MIDIinputports=MIDIinputdevices=0;

				MIDIinputstring=MIDIinputfilter=MIDIinputsync=MIDIintempoquant=0;

				if(settingsgadgetlist)
				{
					int y=50,y2=height-10;

					y2=y+(y2-y)/2;

					MIDIinputports=settingsgadgetlist->AddListBox(s_x2,y,e_x2,y2-1,GADGET_MIDIINPORID,0,"MIDI Input Ports");

					MIDIinputdevices=settingsgadgetlist->AddListBox(s_x2,y2,e_x2,height-10,GADGET_MIDIINDEVICEID,0,"MIDI Input Devices");

					if(MIDIinputdevice)
					{
						MIDIinputfilter=settingsgadgetlist->AddButton(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GetMIDIInFilterName(),GADGET_MIDIINDEVICEFILTER_ID,0,"MIDI Input Device Event Filter");
						y+=maingui->GetFontSizeY();

						MIDIinputsync=settingsgadgetlist->AddCheckBox(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIINDEVICESYNC_ID,"MIDI Sync","MIDI Input Device Syncronisation (Song Position,MTC,MIDI Clock)");
						y+=maingui->GetFontSizeY();

						MIDIintempoquant=settingsgadgetlist->AddButton(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),"?",GADGET_MIDIINDEVICETEMPOQUANT_ID,0,"MIDI Input Device Quantize Tempo");
						y+=maingui->GetFontSizeY();

						MIDIinputstring=settingsgadgetlist->AddString(s_x3,y,e_x3,y+maingui->GetFontSizeY_Sub(),GADGET_MIDIINDEVICEINFO_ID,0,0,"MIDI Input Device User Info (Name)");
					}
				}
			}
			break;
		}
	}
#endif
}

char *Edit_Settings::GetMIDIOutFilterName()
{
	if(MIDIoutdevice)
	{
		MIDIoutputfilter_status=MIDIoutdevice->device_eventfilter.CheckFilterActive();

		switch(MIDIoutputfilter_status)
		{
		case true:
			return "MIDI Output Filter [+]";

		default:
			return "MIDI Output Filter (-)";
		}
	}

	return Cxs[CXS_NODEVICE];
}

char *Edit_Settings::GetMIDIInFilterName()
{
	if(MIDIinputdevice)
	{
		MIDIinputfilter_status=MIDIinputdevice->inputfilter.CheckFilterActive();

		switch(MIDIinputfilter_status)
		{
		case true:
			return "MIDI Input Filter [+]";

		default:
			return "MIDI Input Filter (-)";
		}
	}

	return Cxs[CXS_NODEVICE];
}

void Edit_Settings::RefreshRealtime()
{
	if(openeditor)
		openeditor->RefreshRealtime();
	return;

	if(editproject)
	{
		switch(settingsselection)
		{
		case PRO_PROJECT:
			if(songnamebuff && set_songname)
			{
				if(strcmp(songnamebuff,editproject->name)!=0)
				{
					delete songnamebuff;
					songnamebuff=mainvar->GenerateString(editproject->name);

					if(set_songname)
						set_songname->SetString(songnamebuff);
				}
			}
			break;
		}
	}
	else
		if(WindowSong())
		{
			switch(settingsselection)
			{
			case SONG_SONG:

				if(songnamebuff)
				{
					if(strcmp(songnamebuff,WindowSong()->GetName())!=0)
					{
						delete songnamebuff;
						songnamebuff=mainvar->GenerateString(WindowSong()->GetName());

						if(set_songname)
							set_songname->SetString(songnamebuff);
					}
				}

				break;

			case SONG_ROUTING:

				{
					if(WindowSong()->GetFocusTrack() && (!focustrackname))
						ShowActiveTrack();
					else
						if((!WindowSong()->GetFocusTrack()) && focustrackname)
							ShowActiveTrack();
						else
							if(WindowSong()->GetFocusTrack() && focustrackname)
							{
								if(strcmp(WindowSong()->GetFocusTrack()->GetName(),focustrackname)!=0)
									ShowActiveTrack();
							}
				}
				break;
			}
		}
		else
			switch(settingsselection)
		{
			case MIDISETTINGS_INPUT:
				{
					if(MIDIinputdevice && MIDIinputfilter)
					{
						bool oldstatus=MIDIinputfilter_status;
						if(MIDIinputdevice->inputfilter.CheckFilterActive()!=oldstatus)
							MIDIinputfilter->ChangeButtonText(GetMIDIInFilterName());
					}
				}
				break;

			case MIDISETTINGS_OUTPUT:
				{
					if(MIDIoutdevice && MIDIdeviceoutputfilter)
					{
						bool oldstatus=MIDIoutputfilter_status;
						if(MIDIoutdevice->device_eventfilter.CheckFilterActive()!=oldstatus)
							MIDIdeviceoutputfilter->ChangeButtonText(GetMIDIOutFilterName());
					}
				}
				break;

			case GUISETTINGS:
				if(defaulttime && defaulttime_status!=mainsettings->editordefaulttimeformat)
				{
					defaulttime->SetCycleSelection(defaulttime_status=mainsettings->editordefaulttimeformat);
				}
				break;
		}
}

EditData *Edit_Settings::EditDataMessage(EditData *data)
{
#ifdef OLDIE
	switch(data->id)
	{
	case EDIT_AUDIOTHRESH:
		{
			Seq_Project *pro=mainvar->GetActiveProject();
			while(pro)
			{
				pro->checkaudiothreshold=data->volume;
				pro=pro->NextProject();
			}

			mainsettings->checkaudiothreshhold=data->volume;
			ShowProjectAudioThresh();
		}
		break;

	case EDIT_AUDIOENDTHRESH:
		{
			Seq_Project *pro=mainvar->GetActiveProject();
			while(pro)
			{
				pro->checkaudioendthreshold=data->volume;
				pro=pro->NextProject();
			}

			mainsettings->checkaudioendthreshold=data->volume;
			ShowProjectAudioThresh();
		}
		break;

	case EDIT_DEFAULTSONGLENGTH:
		mainsettings->defaultsonglength_measure=data->newvalue;
		ShowDefaultSongLength();
		break;

	case EDIT_PROGRAM_PROGRAMSELECT:
		if(activeMIDIprogram)
		{
			activeMIDIprogram->ChangeProgram((char)(data->newvalue-1));
			ShowMIDIDevicePrograms();
			ShowMIDIDeviceProgramInfo();
		}
		break;

	case EDIT_PROGRAM_BANKSELECT:
		if(activeMIDIprogram)
		{
			activeMIDIprogram->ChangeBank(data->newvalue-1,data->onoffstatus);
			ShowMIDIDevicePrograms();
			ShowMIDIDeviceProgramInfo();
		}
		break;

	case EDIT_SMPTEOFFSET:
		{
			bool refresh=false;
			EditData_SMPTE *s=(EditData_SMPTE *)data;

			if(s->hour!=mainsettings->smpteoffset_h)
			{
				mainsettings->smpteoffset_h=s->hour;
				refresh=true;
			}

			if(s->min!=mainsettings->smpteoffset_m)
			{
				mainsettings->smpteoffset_m=s->min;
				refresh=true;
			}

			if(s->sec!=mainsettings->smpteoffset_s)
			{
				mainsettings->smpteoffset_s=s->sec;
				refresh=true;
			}

			if(s->frame!=mainsettings->smpteoffset_f)
			{
				mainsettings->smpteoffset_f=s->frame;
				refresh=true;
			}

			if(s->qframe!=mainsettings->smpteoffset_sf)
			{
				mainsettings->smpteoffset_sf=s->qframe;
				refresh=true;
			}

			if(refresh==true)
			{
				maingui->RefreshSMPTE(0);
				ShowSettingsData();
			}
		}
		break;
	}
#endif

	return data;
}

void Edit_Settings::NewProjectMeasureFormat()
{
	guiWindow *win=maingui->FirstWindow();

	while(win)
	{
		if(win->WindowSong() && win->WindowSong()->project==mainvar->GetActiveProject())
		{
			if(win->IsWinEventEditor()==true)
			{
				EventEditor *ee=(EventEditor *)win;
				ee->ShowTime();
			}

			switch(win->GetEditorID())
			{
			case EDITORTYPE_TRANSPORT:
				{
					Edit_Transport *et=(Edit_Transport *)win;

					et->ShowTime(false);
					et->ShowCycle(false);
				}
				break;

			default:
				win->RefreshObjects(0,false);
				break;

			}
		}

		win=win->NextWindow();
	}
}

void Edit_Settings::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_SETTINGSID:
		if(settingsselection!=g->index)
		{
			settingsselection=g->index;
			InitEditField();
		}
		break;
	}

#ifdef OLDIE
	if(g->gadgetID==GADGET_SETTINGSID)
	{
		guiGadget_ListBox *lb=(guiGadget_ListBox *)g;

		if(lb->GetListBoxID(lb->index)!=-1 && lb->GetListBoxID(lb->index)!=settingsselection)
		{

			mainsettings->defaultsettingsselection=settingsselection=lb->GetListBoxID(g->index);

			ShowSettings();
			ShowSettingsData();
			ShowHeader();

			//if(settingsgadgetlist)
			//	settingsgadgetlist->RefreshList();
		}
	}
	else
	{
		if(editproject)
		{
			switch(g->gadgetID)
			{

			case GADGET_PRO_XBOXRECORDPEAK:
				editproject->checkaudiostartpeak=editproject->checkaudiostartpeak==true?false:true;
				mainsettings->usecheckaudiothreshold=editproject->checkaudiostartpeak;
				break;

			case GADGET_PRO_CUTRECORDZEROSAMPLES:
				editproject->autocutzerosamples=editproject->autocutzerosamples==true?false:true;
				mainsettings->autocutzerosamples=editproject->autocutzerosamples;
				break;

			case GADGET_PRO_XBOXRECORDTHRESHOLD:
				if(EditData *edit=new EditData)
				{
					edit->song=WindowSong();
					edit->win=this;
					edit->x=g->x2;
					edit->y=g->y;

					edit->name="|>";
					edit->id=EDIT_AUDIOTHRESH;
					edit->type=EditData::EDITDATA_TYPE_VOLUMEDBNOADD;
					edit->volume=editproject->checkaudiothreshold;

					maingui->EditDataValue(edit);
				}
				break;

			case GADGET_PRO_XBOXRECORDENDTHRESHOLD:
				if(EditData *edit=new EditData)
				{
					edit->song=WindowSong();
					edit->win=this;
					edit->x=g->x2;
					edit->y=g->y;

					edit->name="<|";
					edit->id=EDIT_AUDIOENDTHRESH;
					edit->type=EditData::EDITDATA_TYPE_VOLUMEDBNOADD;
					edit->volume=editproject->checkaudioendthreshold;

					maingui->EditDataValue(edit);
				}
				break;


			case GADGET_PRO_MEASUREGUI:
				mainsettings->defaultprojectmeasureformat=editproject->projectmeasureformat=g->index;
				NewProjectMeasureFormat();
				break;
			}
		}
		else
			if(WindowSong())
			{
				// Song Settings
				switch(g->gadgetID)
				{
				case GADGET_SONG_GMDISPLAY:
					WindowSong()->generalMIDI=g->index;
					break;

				case GADGET_SONG_PLAYMETRO:
					WindowSong()->metronome.playback=WindowSong()->metronome.playback==true?false:true;
					break;

				case GADGET_SONG_RECMETRO:
					WindowSong()->metronome.record=WindowSong()->metronome.record==true?false:true;
					break;

				case GADGET_SONG_SELINPUT:
					{
						set_selectinputdevice=mainMIDI->GetMIDIInputDevice(g->index);
						ShowSettingsData(0);
					}
					break;

				case GADGET_SONG_SELINPUTTYPE:
					set_selecttypeindex=g->index;
					ShowSettingsData(0);
					break;

				case GADGET_SONG_SELINPUTCHANNELS:
					switch(set_selecttypeindex)
					{
					case SIT_CHANNEL:
						if(set_selectedchannel!=g->index)
						{
							set_selectedchannel=g->index;
							ShowTargetTracks();
						}
						break;

					case SIT_TYPE:
						{
							set_selectedtype=g->index;
							ShowSettingsData(0);
						}
						break;
					}
					break;

				case GADGET_SONG_SELINPUTTRACKS:
					set_selectedtrack=g->index;
					break;

				case GADGET_SONG_ADDTRACK:
					if(WindowSong()->GetFocusTrack() && set_selectinputdevice)
					{
						Seq_MIDIRouting_InputDevice *fd=WindowSong()->inputrouting.FindInputDevice(set_selectinputdevice);

						if(fd)
						{
							bool show=false;

							fd->LockO();

							switch(set_selecttypeindex)
							{
							case SIT_CHANNEL:
								if(fd->router_channels[set_selectedchannel].AddTrack(WindowSong()->GetFocusTrack()))
									show=true;
								break;

							case SIT_TYPE:
								if(fd->router_events[set_selectedtype].AddTrack(WindowSong()->GetFocusTrack()))
									show=true;
								break;
							}

							fd->UnlockO();

							if(show==true)
								ShowSettingsData(0);
						}
					}
					break;

				case GADGET_SONG_ADDACTIVETRACK:
					if(set_selectinputdevice)
					{
						Seq_MIDIRouting_InputDevice *fd=WindowSong()->inputrouting.FindInputDevice(set_selectinputdevice);

						if(fd)
						{
							bool show=false;

							fd->LockO();

							switch(set_selecttypeindex)
							{
							case SIT_CHANNEL:
								if(fd->router_channels[set_selectedchannel].AddTrack(0))
									show=true;
								break;

							case SIT_TYPE:
								if(fd->router_events[set_selectedtype].AddTrack(0))
									show=true;
								break;
							}

							fd->UnlockO();

							if(show==true)
								ShowSettingsData(0);
						}
					}
					break;

				case GADGET_SONG_DELETETRACK:
					if(set_selectinputdevice)
					{
						Seq_MIDIRouting_InputDevice *fd=WindowSong()->inputrouting.FindInputDevice(set_selectinputdevice);

						if(fd)
						{
							bool show=false;

							fd->LockO();

							switch(set_selecttypeindex)
							{
							case SIT_CHANNEL:
								{
									Seq_MIDIRouting_Router_Track *rt=fd->router_channels[set_selectedchannel].GetTrackIndex(set_selectedtrack);

									if(rt)
										fd->router_channels[set_selectedchannel].RemoveTrack(rt);
								}
								break;

							case SIT_TYPE:
								{
									Seq_MIDIRouting_Router_Track *rt=fd->router_events[set_selectedtype].GetTrackIndex(set_selectedtrack);

									if(rt)
										fd->router_events[set_selectedtype].RemoveTrack(rt);
								}
								break;
							}

							fd->UnlockO();

							ShowSettingsData(0);
						}
					}
					break;
				}
			}
			else
			{
				// Global Settings
				switch(g->gadgetID) //index
				{
				case GADGET_AUDIOINCHANNELSPORTTYPE:
					mainsettings->audioinporttypeselection=audioinporttypeselection=g->index;
					ShowSettingsData();
					break;

				case GADGET_AUDIOOUTCHANNELSPORTTYPE:
					mainsettings->audiooutporttypeselection=audiooutporttypeselection=g->index;
					ShowSettingsData();
					break;

				case GADGET_AUDIOOUTCHANNELSPORTS:
					mainsettings->outportsselection=outportsselection=g->index;
					ShowAudioOutports();
					break;

				case GADGET_AUDIOINCHANNELSPORTS:
					mainsettings->inportsselection=inportsselection=g->index;
					ShowAudioInports();
					break;

					/*
					case GADGET_HW_DEVICE:
					selected_hw_device=g->index;
					ShowHDDeviceList();
					break;
					*/

				case GADGET_HW_SYSTEM:
					selected_hw_system=g->index;
					selected_hw_device=0;
					ShowHWDevices();
					//ShowHDDeviceList();
					break;

				case GADGET_PROGRAMCHANNEL:
					if(activeMIDIprogram)
					{
						DeletePopUpMenu(true);

						if(popmenu)
						{
							class menu_MIDIchl:public guiMenu
							{
							public:
								menu_MIDIchl(Edit_Settings *s,char c)
								{
									editor=s;
									channel=c;
								}

								void MenuFunction()
								{
									editor->activeMIDIprogram->MIDIChannel=channel;
									editor->ShowMIDIDevicePrograms();
									editor->ShowMIDIDeviceProgramInfo();
								} //

								Edit_Settings *editor;
								char channel;
							};

							for(int m=0;m<17;m++)
								popmenu->AddFMenu(MIDIchannels[m],new menu_MIDIchl(this,m),activeMIDIprogram->MIDIChannel==m?true:false);

							ShowPopMenu();
						}
					}
					break;


				case GADGET_PROGRAMPROGRAM:
					if(activeMIDIprogram)
					{
						if(EditData *edit=new EditData)
						{
							edit->win=this;
							edit->x=g->x2;
							edit->y=g->y;

							edit->onoff=false;

							edit->value=activeMIDIprogram->MIDIProgram+1;
							edit->id=EDIT_PROGRAM_PROGRAMSELECT;
							edit->name="Program Change Select";

							edit->type=EditData::EDITDATA_TYPE_INTEGER;
							edit->from=1;
							edit->to=128;

							maingui->EditDataValue(edit);
						}
					}
					break;

				case GADGET_PROGRAMBANKSEL:
					if(activeMIDIprogram)
					{
						activeMIDIprogram->usebanksel=activeMIDIprogram->usebanksel==true?false:true;
						ShowMIDIDevicePrograms();
					}
					break;

				case GADGET_PROGRAMBANK:
					if(activeMIDIprogram)
					{
						if(EditData *edit=new EditData)
						{
							edit->win=this;
							edit->x=g->x2;
							edit->y=g->y;

							edit->onoff=true;
							edit->onoffstatus=activeMIDIprogram->usebanksel;
							edit->onoffstring="Send Bank Select ON/OFF";

							edit->value=activeMIDIprogram->MIDIBank+1;
							edit->id=EDIT_PROGRAM_BANKSELECT;
							edit->name="Program Bank Select";

							edit->type=EditData::EDITDATA_TYPE_INTEGER;
							edit->from=1;
							edit->to=128;

							maingui->EditDataValue(edit);
						}
					}
					break;

				case GADGET_DEVICENAME:
					if(activeMIDIdevice)
					{
						dontshowMIDIdevicename=true;

						size_t sl=strlen(g->string);

						if(sl>31)
						{
							strncpy(activeMIDIdevice->name,g->string,31);
							activeMIDIdevice->name[31]=0;
						}
						else
							strcpy(activeMIDIdevice->name,g->string);

						ShowSettingsData(0);
						dontshowMIDIdevicename=false;
					}
					break;

				case GADGET_DEVICE:
					{
						activeMIDIdevice=(SynthDevice *)mainsettings->synthdevices.devices.GetO(g->index);
						ShowMIDIDevicePrograms();
						ShowMIDIDeviceName();
					}
					break;

				case GADGET_ADDDEVICE:
					{
						if(SynthDevice *dev=new SynthDevice)
						{
							strcpy(dev->name,"Device");
							mainsettings->synthdevices.AddDevice(dev);

							if(!activeMIDIdevice)
								activeMIDIdevice=dev;

							ShowSettingsData(0);
							ShowMIDIDevicePrograms();
						}
					}
					break;

				case GADGET_DELETEDEVICE:
					if(activeMIDIdevice)
					{
						SynthDevice *nextprev=(SynthDevice *)activeMIDIdevice->NextOrPrev();

						mainsettings->synthdevices.DeleteDevice(activeMIDIdevice);
						activeMIDIdevice=nextprev;

						ShowSettingsData(0);
						ShowMIDIDevicePrograms();
					}
					break;

				case GADGET_PROGRAMNAME:
					if(activeMIDIprogram)
					{
						dontshowMIDIprogramname=true;

						size_t sl=strlen(g->string);

						if(sl>31)
						{
							strncpy(activeMIDIprogram->name,g->string,31);
							activeMIDIprogram->name[31]=0;
						}
						else
							strcpy(activeMIDIprogram->name,g->string);

						ShowSettingsData(0);
						dontshowMIDIprogramname=false;
					}
					break;

				case GADGET_DEVICEPROGRAM:
					if(activeMIDIdevice)
					{
						activeMIDIprogram=(DeviceProgram *)activeMIDIdevice->programs.GetO(g->index);
						ShowMIDIDeviceProgramInfo();
						ShowMIDIProgramName();
					}
					break;

				case GADGET_ADDPROGRAM:
					if(activeMIDIdevice)
					{
						if(DeviceProgram *program=new DeviceProgram)
						{
							activeMIDIprogram=program;
							strcpy(program->name,"Program");
							activeMIDIdevice->AddProgram(program);
							ShowMIDIDevicePrograms();
						}
					}
					break;

				case GADGET_DELETEPROGRAM:
					if(activeMIDIprogram && activeMIDIdevice)
					{
						DeviceProgram *nextprev=(DeviceProgram *)activeMIDIprogram->NextOrPrev();

						activeMIDIdevice->DeleteProgram(activeMIDIprogram);
						activeMIDIprogram=nextprev;

						ShowMIDIDevicePrograms();
					}
					break;

				case GADGET_SMPTEDISPLAYOFFSET:
					{
						if(EditData_SMPTE *edit=new EditData_SMPTE)
						{
							// long position;
							edit->song=0;
							edit->win=this;
							edit->x=g->x;
							edit->y=g->y;

							edit->name="SMPTE Offset";
							edit->deletename=false;

							edit->id=EDIT_SMPTEOFFSET;

							edit->type=EditData::EDITDATA_TYPE_SMPTEONLY;

							edit->hour=mainsettings->smpteoffset_h;
							edit->min=mainsettings->smpteoffset_m;
							edit->sec=mainsettings->smpteoffset_s;
							edit->frame=mainsettings->smpteoffset_f;
							edit->qframe=mainsettings->smpteoffset_sf;

							maingui->EditDataValue(edit);
						}
					}
					break;

				case GADGET_AUTOUPDATE:
					mainvar->autoupdatecheck=mainvar->autoupdatecheck==true?false:true;
					Save();
					break;

				case GADGET_SYNCGLOBALSMPTE:
					{
						int newsmpte=smptemode[g->index];

						if(newsmpte!=mainsettings->projectstandardsmpte)
						{
							mainsettings->projectstandardsmpte=newsmpte;
							//maingui->RefreshSMPTE();
							Save();
						}
					}
					break;

				case GADGET_SYNCOUTPUTMIDICLOCKSP:
					{
						mainMIDI->sendMIDIcontrol=mainMIDI->sendMIDIcontrol==true?false:true;
						Save();
					}
					break;

				case GADGET_QUANTIZETOSPP:
					{
						mainMIDI->quantizesongpositiontoMIDIclock=mainMIDI->quantizesongpositiontoMIDIclock==true?false:true;
						Save();
					}
					break;

				case GADGET_SYNCINPUTMIDICLOCKSP:
					{
						mainMIDI->receiveMIDIcontrol=mainMIDI->receiveMIDIcontrol==true?false:true;
						Save();
					}
					break;

				case GADGET_SENDMTC:
					{
						mainMIDI->sendmtc=mainMIDI->sendmtc==true?false:true;
						Save();
					}
					break;

				case GADGET_RECEIVEMTC:
					{
						mainMIDI->receivemtc=mainMIDI->receivemtc==true?false:true;
						Save();
					}
					break;

				case GADGET_SENDALWAYSCONTROL:
					{
						mainMIDI->sendcontrolsolomute=mainMIDI->sendcontrolsolomute==true?false:true;
						Save();
					}
					break;

				case GADGET_MIDIINDEVICEINFO_ID:
					{
						if(MIDIinputdevice)
							mainMIDI->ChangeMIDIInputDeviceUserInfo(MIDIinputdevice,g->string);

						ShowSettingsData(NO_DEVICEINFOSTRING);
					}
					break;

				case GADGET_MIDIOUTDEVICEINFO_ID:
					{
						if(MIDIoutdevice)
							mainMIDI->ChangeMIDIOutputDeviceUserInfo(MIDIoutdevice,g->string);

						ShowSettingsData(NO_DEVICEINFOSTRING);
					}
					break;

				case GADGET_AUTOINSTRUMENT:
					{
						mainsettings->autoinstrument=mainsettings->autoinstrument==true?false:true;
						mainsettings->Save(0);
					}
					break;

				case GADGET_AUTOTRACKMIDIININSTRUMENT:
					{
						mainsettings->autotrackMIDIinthru=mainsettings->autotrackMIDIinthru==true?false:true;
						mainsettings->Save(0);
					}
					break;


				case GADGET_VSTINSTRUMENTS:
					{
						int ix=g->index;

						activevstinstrument=mainaudio->FirstVSTInstrument();

						while(ix--)
						{
							if(activevstinstrument)
								activevstinstrument=activevstinstrument->NextVSTPlugin();
						}
						ShowVSTInfo(false);
					}
					break;

				case GADGET_PLUGINS:
					{
						mainsettings->plugincheck=g->index;
						Save();
					}
					break;


				case GADGET_SETSIZE:
					maingui->OpenEditorStart(EDITORTYPE_WIN32AUDIO,0,0,0,0,this,0);
					break;

				case GADGET_SETSIZEQUICK:
					if(mainaudio->GetActiveDevice() && g->index>=1 && g->index<=OPTLATENTCY)
					{
						int setSize=optsetsize[g->index-1];
						mainaudio->SetSamplesSize(mainaudio->GetActiveDevice(),setSize);
					}

					ShowSettingsData();
					break;

				case GADGET_AUDIOSYSTEMID:
					{
						if(mainaudio->SetAudioSystem(g->index)==true)
						{
							CloseAllAudioDeviceWindows();

							audiodevice=mainaudio->GetActiveDevice();

							ShowSetAudioDevices();
							ShowSettingsData(0);
						}
					}
					break;

				case GADGET_AUDIODEVICEID:
					if(mainaudio->selectedaudiohardware)
					{
						AudioDevice *seldev=mainaudio->selectedaudiohardware->GetAudioDeviceIndex(g->index);

						if(seldev!=audiodevice)
						{
							audiodevice=seldev;
							ShowSettingsData(0);
							CloseAllAudioDeviceWindows();
						}
					}
					break;

				case GADGET_USEAUDIODEVICEID:
					if(mainaudio->selectedaudiohardware && audiodevice)
					{
						mainaudio->SelectOtherDevice(audiodevice);
						ShowSetAudioDevices();
						ShowSettingsData(0);
						CloseAllAudioDeviceWindows();
					}
					break;

					/*
					case GADGET_LOCKAUDIOFILESID:
					{
					Seq_Song *asong=mainvar->GetActiveSong();

					if(asong && asong->status==Seq_Song::STATUS_STOP) // no change while playback/record
					{
					if(g->index)
					{
					mainaudio->lockaudiofiles=true;
					//	asong->audiosystem.OpenAllAudioFiles();
					}
					else
					{
					mainaudio->lockaudiofiles=false;
					//asong->audiosystem.CloseAllAudioFiles();
					}
					}
					else
					ShowSettingsData();
					}
					break;
					*/

				case GADGET_MIDIFILE:
					mainsettings->addgsgmtoMIDIfiles=g->index;
					Save();
					break;

				case GADGET_AUTOSAVE:
					mainsettings->autosavemin=g->index;
					Save();
					break;

				case GADGET_PEAKFILE:
					mainsettings->peakfiles=g->index;
					Save();
					break;

				case GADGET_UNUSEDSONGFILES:
					mainsettings->flag_unusedaudiofiles=g->index;
					Save();
					break;

				case GADGET_ASKIMPORTAUDIO:
					mainsettings->askimportaudiofiles=mainsettings->askimportaudiofiles==true?false:true;
					Save();
					break;

				case GADGET_IGNORECORRUPTAUDIOFILES:
					mainaudio->ignorecorrectaudiofiles=mainaudio->ignorecorrectaudiofiles==true?false:true;
					break;

				case GADGET_IMPORTQUESTION:
					mainsettings->importfilequestion=mainsettings->importfilequestion==true?false:true;
					Save();
					break;

				case GADGET_SPLITMIDIFILE:
					mainsettings->splitMIDIfilestype0=mainsettings->splitMIDIfilestype0==true?false:true;
					Save();
					break;

				case GADGET_IMPORTAUDIO:
					mainsettings->importaudiofiles=mainsettings->importaudiofiles==true?false:true;
					Save();
					break;

				case GADGET_AUDIOOUTFORMAT:
					mainsettings->audioresamplingformat=g->index;
					break;

				case GADGET_SHOWOFF:
					{
						mainsettings->displaynoteoff_monitor=mainsettings->displaynoteoff_monitor==true?false:true;
						MIDIOutputDevice *o=mainMIDI->FirstMIDIOutputDevice();
						while(o)
						{
							o->displaynoteoff_monitor=mainsettings->displaynoteoff_monitor;
							o=o->NextOutputDevice();
						}

						MIDIInputDevice *i=mainMIDI->FirstMIDIInputDevice();
						while(i)
						{
							i->displaynoteoff_monitor=mainsettings->displaynoteoff_monitor;
							i=i->NextInputDevice();
						}

						Save();
					}
					break;

				case GADGET_DOUBLESTOP:
					mainsettings->doublestopeditrefresh=mainsettings->doublestopeditrefresh==true?false:true;
					Save();
					break;

				case GADGET_TRANSPORTDESKTOP:
					mainsettings->transportdesktop=mainsettings->transportdesktop==true?false:true;
					Save();
					break;

				case GADGET_SHOWTOOLTIP:
					mainsettings->displayeditortooltip=mainsettings->displayeditortooltip==true?false:true;
					Save();
					break;

				case GADGET_SHOWHEADERBOTH:
					mainsettings->showbothsformatsintimeline=mainsettings->showbothsformatsintimeline==true?false:true;
					Save();
					maingui->RefreshAllHeaders(0);
					break;

				case GADGET_FOLLOWEDITOR:
					mainsettings->followeditor=mainsettings->followeditor==true?false:true;
					Save();
					break;

				case GADGET_SOLOEDITOR:
					mainsettings->soloeditor=mainsettings->soloeditor==true?false:true;
					Save();
					break;

				case GADGET_AUTOLOADPROJECT:
					mainsettings->autoloadlastusedproject=mainsettings->autoloadlastusedproject==true?false:true;
					Save();
					break;

				case GADGET_GUIEDITOR_WINDOWFORMAT:
					mainsettings->editordefaulttimeformat=g->index;
					Save();
					break;

				case GADGET_GUIEDITOR_WINDOWGMFORMAT:
					{
						mainsettings->eventeditorgmmode=g->index;
						Save();
					}
					break;

				case GADGET_GUIEDITOR_ARRANGEFILEDISPLAY:
					if(g->index!=mainsettings->arrangeeditorfiledisplay)
					{
						mainsettings->arrangeeditorfiledisplay=g->index;
						Save();
						maingui->RefreshAllEditors(0,EDITORTYPE_ARRANGE,0);
					}
					break;

				case GADGET_GUIEDITOR:
					mainsettings->opendoubleclickeditor=g->index;
					Save();
					break;

				case GADGET_GUIEDITOR_UNDO:
					mainsettings->undosteps=g->index;
					Save();
					break;


				case GADGET_DEFAULTDEVICEID:
					if(MIDIoutdevice)
					{
						mainMIDI->SetDefaultDevice(MIDIoutdevice);
						ShowSettingsData();
					}
					break;

				case GADGET_MIDIINDEVICEID:
					{
						MIDIinputdevice=mainMIDI->GetMIDIInputDevice(g->index);
						ShowSettingsData();
					}
					break;

				case GADGET_MIDIINDEVICESYNC_ID:
					if(MIDIinputdevice)
					{
						if(MIDIinputdevice->receivesync==true)
							MIDIinputdevice->receivesync=false;
						else
							MIDIinputdevice->receivesync=true;

						MIDIinputdevice->receivesync=MIDIinputdevice->receivesync==true?false:true;
						Save();
					}
					break;

				case GADGET_MIDIINDEVICETEMPOQUANT_ID:
					if(MIDIinputdevice)
					{
						if(DeletePopUpMenu(true))
						{
							class menu_hqt:public guiMenu
							{
							public:
								menu_hqt(Edit_Settings *ed,int m)
								{
									editor=ed;
									mode=m;
								}

								void MenuFunction()
								{
									editor->MIDIinputdevice->MIDIclockraster=mode;
									editor->ShowSettingsData();
								} //

								Edit_Settings *editor;
								int mode;
							};

							popmenu->AddFMenu(Cxs[CXS_NOQUANTIZE],new menu_hqt(this,-1),MIDIinputdevice->MIDIclockraster==-1?true:false);
							popmenu->AddLine();

							// Quantize
							for(int a=0;a<QUANTNUMBER;a++)
								popmenu->AddFMenu(quantstr[a],new menu_hqt(this,a),a==MIDIinputdevice->MIDIclockraster?true:false);

							ShowPopMenu();
						}
					}
					break;

				case GADGET_MIDIINDEVICEFILTER_ID:
					if(MIDIinputdevice)
					{
						guiWindow *win=maingui->FindWindow(EDITORTYPE_MIDIFILTER,&MIDIinputdevice->inputfilter,0);

						if(!win)
						{
							guiWindowSetting set;

							set.startposition_x=GetWinPosX()+g->x2;
							set.startposition_y=GetWinPosY()+g->y;

							guiWindow *w=maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,0,0,0,&set,&MIDIinputdevice->inputfilter,0);

							if(w)
							{
								w->parent=this;

								if(char *h=mainvar->GenerateString(MIDIinputdevice->name," ","MIDI-In Device Filter"))
								{
									w->guiSetWindowText(h);
									delete h;
								}
								else
									w->guiSetWindowText("MIDI-In Device Filter");
							}
						}
						else
						{
							win->WindowToFront(true);
						}		
					}
					break;

				case GADGET_MIDIOUTDEVICEFILTER_ID:
					if(MIDIoutdevice)
					{
						guiWindow *win=maingui->FindWindow(EDITORTYPE_MIDIFILTER,&MIDIoutdevice->device_eventfilter,0);

						if(!win)
						{
							mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].x=GetWinPosX()+g->x2;
							mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].y=GetWinPosY()+g->y;

							guiWindow *w=maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,0,0,0,0,&MIDIoutdevice->device_eventfilter,0);

							if(w)
							{
								w->parent=this;

								if(char *h=mainvar->GenerateString(MIDIoutdevice->name," ","MIDI-Out Device Filter"))
								{
									w->guiSetWindowText(h);
									delete h;
								}
								else
									w->guiSetWindowText("MIDI-Out Device Filter");
							}
						}
						else
						{
							win->WindowToFront(true);
						}		
					}
					break;

				case GADGET_MIDIDEVICEID:
					{
						MIDIoutdevice=mainMIDI->GetMIDIOutputDevice(g->index);
						ShowSettingsData();
					}
					break;

					/*
					case GADGET_MIDIOPTIMIZER:
					if(MIDIoutdevice)
					{
					MIDIoutdevice->MIDIoutputfilter=g->index?true:false;
					Save();
					}
					break;

					*/

				case GADGET_SENDMTCID:
					if(MIDIoutdevice)
					{
						MIDIoutdevice->sendmtc=g->index?true:false;
						Save();
					}
					break;

				case GADGET_MIDIMETRONOMEID:
					if(MIDIoutdevice)
					{
						MIDIoutdevice->sendmetronome=g->index?true:false;
						Save();
					}
					break;

				case GADGET_MIDIMETRONOMECHANNELID:
					if(MIDIoutdevice)
					{
						MIDIoutdevice->sendmetronome_channel=(UBYTE)g->index;
						Save();
					}
					break;

				case GADGET_MIDIMETRONOMENOTEID:
					if(MIDIoutdevice)
					{
						MIDIoutdevice->sendmetronome_key=(UBYTE)g->index;
						Save();
					}
					break;

				case GADGET_MIDIMETRONOMEVELOID:
					if(MIDIoutdevice)
					{
						MIDIoutdevice->sendmetronome_velocity=(UBYTE)g->index;
						Save();
					}
					break;

				case GADGET_MIDIMETRONOMECHANNELHIID:
					if(MIDIoutdevice)
					{
						MIDIoutdevice->sendmetronome_channel_hi=(UBYTE)g->index;
						Save();
					}
					break;

				case GADGET_MIDIMETRONOMENOTEHIID:
					if(MIDIoutdevice)
					{
						MIDIoutdevice->sendmetronome_key_hi=(UBYTE)g->index;
						Save();
					}
					break;

				case GADGET_MIDIMETRONOMEVELOHIID:
					if(MIDIoutdevice)
					{
						MIDIoutdevice->sendmetronome_velocity_hi=(UBYTE)g->index;
						Save();
					}
					break;
				}

			}// global
	}

#endif
}

void Edit_Settings::ResetGadgets()
{
	vstdirectories=
		MIDIOutputDevices=
		audioinlist=
		audiochannels=
		set_audiodevices=
		MIDIinputdevices=
		audiooutlist=0;

	pro_samplerate=
		pro_panlaw=
		g_cpucores=
		vstadd=
		vstdelete=

		set_songgm=
		set_songtime=
		MIDIinputfilter=
		MIDIinputstring=

		// MIDI Out
		defaultdevice=
		MIDIclockout=
		MIDIoutmetronome=
		MIDIoutmetrochannel=
		MIDIoutmetronote=
		MIDIoutmetrovelo=
		//MIDIoptimizer=
		MIDIsendsolomutecontrol=
		MIDIoutmetrochannel_hi=
		MIDIoutmetronote_hi=
		MIDIoutmetrovelo_hi=
		sendcyclenotes=
		MIDIoutputstring=
		MIDIdeviceoutputfilter=
		set_audiohardware=
		useaudiodevice=
		samplerates=
		audiodeviceprefs=
		infotext=0;
}

void Edit_Settings::Save()
{
	mainsettings->Save(0);
}

void Edit_Settings::Init()
{
	//if(!audiodevice)
	//	audiodevice=mainaudio->GetActiveDevice();

	InitGadgets();


	/*
	ShowSettings();	

	ShowSettingsData();
	ShowHeader();
	*/
}

void Edit_Settings::RefreshAudioDirectories()
{
	if(openeditor && openeditor->GetEditorID()==EDITORTYPE_SETTINGS_AUDIOFILES)
	{
		Edit_Settings_AudioFiles *esa=(Edit_Settings_AudioFiles *)openeditor;
		esa->ShowAudioDirectories();
	}
}


void Edit_Settings::CloseAllAudioDeviceWindows()
{
#ifdef OLDIE
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		guiWindow *n=w->NextWindow();

		switch(w->GetEditorID())
		{
		case EDITORTYPE_WIN32AUDIO:
			{
				//Edit_Win32Audio *ed=(Edit_Win32Audio *)w;
				//if(ed->editor==this)maingui->CloseWindow(w);
			}
			break;
		}

		w=n;
	}
#endif

}

void Edit_Settings::FreeMemory()
{
	if(songnamebuff)delete songnamebuff;
	songnamebuff=0;

	if(winmode&WINDOWMODE_DESTROY)
	{
		CloseAllAudioDeviceWindows();
		Save();
		if(focustrackname)delete focustrackname;
	}
}
