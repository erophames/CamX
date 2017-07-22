#include "editMIDIfilter.h"
#include "languagefiles.h"
#include "gui.h"
#include "songmain.h"
#include "camxfile.h"
#include "object_song.h"

#define MIDIFILTERGADGETID_START (GADGET_ID_START+50)

enum GIDs
{
	GADGET_INFO=MIDIFILTERGADGETID_START+9,
	GADGET_NOTE,
	GADGET_OCTINFO,

	GADGET_CONTROL,
	GADGET_CPRESS ,
	GADGET_PPRESS ,
	GADGET_PITCHBEND ,
	GADGET_SYSEX,
	GADGET_PROGCHG ,
	GADGET_TOGGLE,
	GADGET_EVENTTOGGLE ,
	GADGET_RESET,
	GADGET_BYPASS,
	GADGET_INTERN,
	GADGET_EVENTCLEAR ,
	GADGET_EVENTRESET,
	GADGET_CHANNELRESET,
	GADGET_CHANNELCLEAR,
	GADGET_CHANNEL1,
	GADGET_CHANNEL2,
	GADGET_CHANNEL3,
	GADGET_CHANNEL4,
	GADGET_CHANNEL5,
	GADGET_CHANNEL6,
	GADGET_CHANNEL7,
	GADGET_CHANNEL8,
	GADGET_CHANNEL9,
	GADGET_CHANNEL10,
	GADGET_CHANNEL11,
	GADGET_CHANNEL12,
	GADGET_CHANNEL13,
	GADGET_CHANNEL14,
	GADGET_CHANNEL15,
	GADGET_CHANNEL16,

	GADGET_OCTAVE1,
	GADGET_OCTAVE2,
	GADGET_OCTAVE3,
	GADGET_OCTAVE4,

	GADGET_OCTAVE5,
	GADGET_OCTAVE6,
	GADGET_OCTAVE7,
	GADGET_OCTAVE8,

	GADGET_OCTAVE9,
	GADGET_OCTAVE10,
	GADGET_OCTAVE11
};

Edit_MIDIFilter::Edit_MIDIFilter(MIDIFilter *editfilter)
{
	editorid=EDITORTYPE_MIDIFILTER;
	filter=editfilter;
	editfilter->Clone(&backup);
	dialogstyle=true;
	ondesktop=true;
	infostring=0;
};

EditData *Edit_MIDIFilter::EditDataMessage(EditData *data)
{
	return data;
}

void Edit_MIDIFilter::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_BYPASS:
		filter->bypass=filter->bypass==true?false:true;
		filter->Clone(&c_filter);
		break;

	case GADGET_RESET:
		{
			filter->Reset();
			ShowFilter();
		}
		break;

	case GADGET_EVENTCLEAR:
		{
			if(filter->statusfilter)
			{
				filter->statusfilter=0;
				ShowFilter();
			}
		}
		break;

		case GADGET_EVENTRESET:
		{
			filter->statusfilter=MIDIOUTFILTER_INTERN|MIDIOUTFILTER_NOTEON|MIDIOUTFILTER_POLYPRESSURE|MIDIOUTFILTER_CONTROLCHANGE|MIDIOUTFILTER_PROGRAMCHANGE|MIDIOUTFILTER_CHANNELPRESSURE|MIDIOUTFILTER_PITCHBEND|MIDIOUTFILTER_SYSEX;
			ShowFilter();
		}
		break;

		case GADGET_CHANNELRESET:
		{
			filter->channelfilter=0xFFFF; // channel 1-16 on 
			ShowFilter();
		}
		break;

	case GADGET_CHANNELCLEAR:
		{
			if(filter->channelfilter){
				filter->channelfilter=0;
				ShowFilter();
			}
		}
		break;

	case GADGET_EVENTTOGGLE:
		{
			if(filter->statusfilter&MIDIOUTFILTER_NOTEON)
				filter->statusfilter CLEARBIT MIDIOUTFILTER_NOTEON;
			else
				filter->statusfilter |=MIDIOUTFILTER_NOTEON;

			if(filter->statusfilter&MIDIOUTFILTER_CONTROLCHANGE)
				filter->statusfilter CLEARBIT MIDIOUTFILTER_CONTROLCHANGE;
			else
				filter->statusfilter |=MIDIOUTFILTER_CONTROLCHANGE;

			if(filter->statusfilter&MIDIOUTFILTER_CHANNELPRESSURE)
				filter->statusfilter CLEARBIT MIDIOUTFILTER_CHANNELPRESSURE;
			else
				filter->statusfilter |=MIDIOUTFILTER_CHANNELPRESSURE;

			if(filter->statusfilter&MIDIOUTFILTER_POLYPRESSURE)
				filter->statusfilter CLEARBIT MIDIOUTFILTER_POLYPRESSURE;
			else
				filter->statusfilter |=MIDIOUTFILTER_POLYPRESSURE;

			if(filter->statusfilter&MIDIOUTFILTER_PITCHBEND)
				filter->statusfilter CLEARBIT MIDIOUTFILTER_PITCHBEND;
			else
				filter->statusfilter |=MIDIOUTFILTER_PITCHBEND;

			if(filter->statusfilter&MIDIOUTFILTER_SYSEX)
				filter->statusfilter CLEARBIT MIDIOUTFILTER_SYSEX;
			else
				filter->statusfilter |=MIDIOUTFILTER_SYSEX;

			if(filter->statusfilter&MIDIOUTFILTER_INTERN)
				filter->statusfilter CLEARBIT MIDIOUTFILTER_INTERN;
			else
				filter->statusfilter |=MIDIOUTFILTER_INTERN;

			if(filter->statusfilter&MIDIOUTFILTER_PROGRAMCHANGE)
				filter->statusfilter CLEARBIT MIDIOUTFILTER_PROGRAMCHANGE;
			else
				filter->statusfilter |=MIDIOUTFILTER_PROGRAMCHANGE;

			ShowFilter();
		}
		break;

	case GADGET_TOGGLE:
		{
			for(int i=0;i<16;i++)
			{
				if(filter->channelfilter&(1<<i))
				{
					filter->channelfilter CLEARBIT (1<<i);

					if(boxchannel[i])
						boxchannel[i]->SetCheckBox(false);
				}
				else
				{
					filter->channelfilter|=1<<i;

					if(boxchannel[i])
						boxchannel[i]->SetCheckBox(true);
				}
			}

			filter->Clone(&c_filter);

		}
		break;

	case GADGET_NOTE:
		if(g->index)
			filter->statusfilter |= MIDIOUTFILTER_NOTEON;
		else
			filter->statusfilter CLEARBIT MIDIOUTFILTER_NOTEON;

		filter->Clone(&c_filter);

		break;

	case GADGET_CONTROL:
		if(g->index)
			filter->statusfilter |= MIDIOUTFILTER_CONTROLCHANGE;
		else
			filter->statusfilter CLEARBIT MIDIOUTFILTER_CONTROLCHANGE;

		filter->Clone(&c_filter);

		break;

	case GADGET_CPRESS:
		if(g->index)
			filter->statusfilter |= MIDIOUTFILTER_CHANNELPRESSURE;
		else
			filter->statusfilter CLEARBIT MIDIOUTFILTER_CHANNELPRESSURE;

		filter->Clone(&c_filter);

		break;

	case GADGET_PPRESS:
		if(g->index)
			filter->statusfilter |= MIDIOUTFILTER_POLYPRESSURE;
		else
			filter->statusfilter CLEARBIT MIDIOUTFILTER_POLYPRESSURE;

		filter->Clone(&c_filter);

		break;

	case GADGET_PITCHBEND:
		if(g->index)
			filter->statusfilter |= MIDIOUTFILTER_PITCHBEND;
		else
			filter->statusfilter CLEARBIT MIDIOUTFILTER_PITCHBEND;

		filter->Clone(&c_filter);

		break;

	case GADGET_SYSEX:
		if(g->index)
			filter->statusfilter |= MIDIOUTFILTER_SYSEX;
		else
			filter->statusfilter CLEARBIT MIDIOUTFILTER_SYSEX;

		filter->Clone(&c_filter);

		break;

	case GADGET_INTERN:
		if(g->index)
			filter->statusfilter |= MIDIOUTFILTER_INTERN;
		else
			filter->statusfilter CLEARBIT MIDIOUTFILTER_INTERN;

		filter->Clone(&c_filter);

		break;

	case GADGET_PROGCHG:
		if(g->index)
			filter->statusfilter |= MIDIOUTFILTER_PROGRAMCHANGE;
		else
			filter->statusfilter CLEARBIT MIDIOUTFILTER_PROGRAMCHANGE;

		filter->Clone(&c_filter);

		break;

	default:
		if(g->gadgetID>=GADGET_CHANNEL1 && g->gadgetID<=GADGET_CHANNEL16)
		{
			int c=g->gadgetID-GADGET_CHANNEL1;

			if(g->index)
				filter->channelfilter |= 1<<c;
			else
				filter->channelfilter CLEARBIT (1<<c);

			filter->Clone(&c_filter);
		}
		else
			if(g->gadgetID>=GADGET_OCTAVE1 && g->gadgetID<=GADGET_OCTAVE11)
			{
				int c=g->gadgetID-GADGET_OCTAVE1;

				if(filter->octavefilter&(1<<c))
					filter->octavefilter CLEARBIT (1<<c);
				else
					filter->octavefilter |=1<<c;

				filter->Clone(&c_filter);
			}
			break;
	}
}

void Edit_MIDIFilter::LoadSettings()
{
	if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),"\\DataBase\\MIDIFilter\\"))
	{
		camxFile sfile;

		if(sfile.OpenFileRequester(0,this,Cxs[CXS_LOADMIDIFILTER],sfile.AllFiles(camxFile::FT_MIDIFILTER),true,h)==true)
		{
			if(sfile.OpenRead(sfile.filereqname)==true)
			{
				char check[4];

				sfile.Read(check,4);

				if(mainvar->CompareStringWithOutZero(check,"CAMX")==true)
				{
					sfile.Read(check,4);

					if(mainvar->CompareStringWithOutZero(check,"FSET")==true)
					{
						int version=0;
						sfile.Read(&version);
						filter->Load(&sfile);
						ShowFilter();
					}
				}
			}

			sfile.Close(true);
		}

		delete h;
	}
}

void Edit_MIDIFilter::SaveSettings()
{
	if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),"\\DataBase\\MIDIFilter\\"))
	{
		camxFile sfile;

		if(sfile.OpenFileRequester(0,this,Cxs[CXS_SAVEMIDIFILTER],sfile.AllFiles(camxFile::FT_MIDIFILTER),false,h)==true)
		{
			sfile.AddToFileName(".cxfs");

			if(sfile.OpenSave(sfile.filereqname)==true)
			{
				int version=maingui->GetVersion();

				// Header
				sfile.Save("CAMX",4);
				sfile.Save("FSET",4);

				sfile.Save(&version,sizeof(int)); // Version

				filter->Save(&sfile);
			}

			sfile.Close(true);
		}

		delete h;
	}
}

void Edit_MIDIFilter::CreateWindowMenu()
{
	if(NewWindowMenu())
	{
		guiMenu *s=menu->AddMenu(Cxs[CXS_FILE],0);

		if(s)
		{
			class menu_loadfilter:public guiMenu
			{
			public:
				menu_loadfilter(Edit_MIDIFilter *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					editor->LoadSettings();
				}

				Edit_MIDIFilter *editor;
			};

			s->AddFMenu(Cxs[CXS_LOADMIDIFILTER],new menu_loadfilter(this));

			class menu_savefilter:public guiMenu
			{
			public:
				menu_savefilter(Edit_MIDIFilter *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					editor->SaveSettings();	
				}

				Edit_MIDIFilter *editor;
			};

			s->AddFMenu(Cxs[CXS_SAVEMIDIFILTER],new menu_savefilter(this));
		}
	}
}

/*
MIDIFilter::~MIDIFilter()
{
guiWindow *w=maingui->FirstWindow();

while(w)
{	
if(w->GetEditorID()==EDITORTYPE_MIDIFILTER && ((Edit_MIDIFilter *)w)->filter==this)
w=w->CloseWindow(false);
else
w=w->NextWindow();
}
}
*/

void Edit_MIDIFilter::ShowFilter()
{
	notetype=!WindowSong()?mainsettings->defaultnotetype:WindowSong()->notetype;

	if(filter)
	{
		filter->Clone(&c_filter);

		if(filterbybpass)
			filterbybpass->SetCheckBox(filter->bypass);

		if(boxintern)
			boxintern->SetCheckBox(filter->statusfilter&MIDIOUTFILTER_INTERN?true:false);

		if(boxsysex)
			boxsysex->SetCheckBox(filter->statusfilter&MIDIOUTFILTER_SYSEX?true:false);

		if(boxpitchbend)
			boxpitchbend->SetCheckBox(filter->statusfilter&MIDIOUTFILTER_PITCHBEND?true:false);

		if(boxprogchg)
			boxprogchg->SetCheckBox(filter->statusfilter&MIDIOUTFILTER_PROGRAMCHANGE?true:false);

		if(boxcontrol)
			boxcontrol->SetCheckBox(filter->statusfilter&MIDIOUTFILTER_CONTROLCHANGE?true:false);

		if(boxnote)
			boxnote->SetCheckBox(filter->statusfilter&MIDIOUTFILTER_NOTEON?true:false);

		if(boxchannelpressure)
			boxchannelpressure->SetCheckBox(filter->statusfilter&MIDIOUTFILTER_CHANNELPRESSURE?true:false);

		if(boxpolypressure)
			boxpolypressure->SetCheckBox(filter->statusfilter&MIDIOUTFILTER_POLYPRESSURE?true:false);

		for(int i=0;i<16;i++)
			if(boxchannel[i])
				boxchannel[i]->SetCheckBox((1<<i)&filter->channelfilter?true:false);

		for(int i=0;i<11;i++)
			if(boxoctave[i])
				boxoctave[i]->Toggle((1<<i)&filter->octavefilter?true:false);


	}
}

void Edit_MIDIFilter::ShowOctaves()
{
		int key=0;
		int oi=0;

	for(int i=0;i<11;i++)
	{
		if(boxoctave[oi])
			boxoctave[oi]->ChangeButtonText(maingui->ByteToOctaveString(WindowSong(),key));

		oi++;
		key+=12;
	}
}

void Edit_MIDIFilter::RefreshRealtime()
{
	if(filter)
	{
		if(c_filter.Compare(filter)==false)
			ShowFilter();
		else
		{
			int cnotetype=!WindowSong()?mainsettings->defaultnotetype:WindowSong()->notetype;

			if(cnotetype!=notetype)
			{
				ShowOctaves();
				ShowFilter();
			}
		}
	}
}

void Edit_MIDIFilter::SetInfo(char *infotext)
{
	/*
	if(infotext)
	{
		if(infostring)delete infostring;
		infostring=mainvar->GenerateString(infotext);

		if(info)
			info->ChangeButtonText(infotext);
	}
	*/
}

void Edit_MIDIFilter::Init()
{
	glist.SelectForm(0,0);

	//	info=gadgetlist->AddText(x,y,x2,y+maingui->GetFontSizeY_Sub(),infostring,GADGET_INFO,"Information");
	//	y+=maingui->GetFontSizeY();

	int w=16*maingui->GetFontSizeY();

	filterbybpass=glist.AddCheckBox(-1,-1,w,-1,GADGET_BYPASS,0,"Filter Bypass");
	glist.Return();

	boxnote=glist.AddCheckBox(-1,-1,w,-1,GADGET_NOTE,0,"Note");
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Octave/Notes :",GADGET_OCTINFO,MODE_RIGHT|MODE_TEXTCENTER,0);
	glist.Return();

	int w2=w/6;
	int oi=0,o=1,gdid=GADGET_OCTAVE1;
	int key=0;

	for(int i=0;i<6;i++)
	{
		boxoctave[oi++]=glist.AddButton(-1,-1,w2,-1,maingui->ByteToOctaveString(WindowSong(),key),gdid++,MODE_TOGGLE|MODE_AUTOTOGGLE);
		key+=12;
		glist.AddLX();
	}
	glist.Return();

	for(int i=0;i<5;i++)
	{
		boxoctave[oi++]=glist.AddButton(-1,-1,w2,-1,maingui->ByteToOctaveString(WindowSong(),key),gdid++,MODE_TOGGLE|MODE_AUTOTOGGLE);
		key+=12;
		glist.AddLX();
	}
	glist.Return();

	boxcontrol=glist.AddCheckBox(-1,-1,w,-1,GADGET_CONTROL,0,"Control");
	glist.Return();

	boxpitchbend=glist.AddCheckBox(-1,-1,w,-1,GADGET_PITCHBEND,0,"Pitchbend");
	glist.Return();

	boxprogchg=glist.AddCheckBox(-1,-1,w,-1,GADGET_PROGCHG,0,"Program Change");
	glist.Return();

	boxchannelpressure=glist.AddCheckBox(-1,-1,w,-1,GADGET_CPRESS,0,"Channel Pressure");
	glist.Return();

	boxpolypressure=glist.AddCheckBox(-1,-1,w,-1,GADGET_PPRESS,0,"Poly Pressure");
	glist.Return();

	boxsysex=glist.AddCheckBox(-1,-1,w,-1,GADGET_SYSEX,0,"SysEx");
	glist.Return();

	boxintern=glist.AddCheckBox(-1,-1,w,-1,GADGET_INTERN,0,"Intern Events");
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Toggle Event Flags",GADGET_EVENTTOGGLE,MODE_TEXTCENTER);
	glist.Return();
	
	glist.AddButton(-1,-1,w,-1,"Clear Event Flags",GADGET_EVENTCLEAR,MODE_TEXTCENTER);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Reset Event Flags",GADGET_EVENTRESET,MODE_TEXTCENTER);
	glist.Return();

	int i=0;
	int ww=w/4-ADDXSPACE;

	for(int y=0;y<4;y++)
	{
		for(int x=0;x<4;x++)
		{
			boxchannel[i]=glist.AddCheckBox(-1,-1,ww,-1,GADGET_CHANNEL1+i,0,numbers_pos[i+1]);
			i++;
			glist.AddLX();
		}

		glist.Return();
	}

	glist.AddButton(-1,-1,w,-1,"Toggle Channel Flags",GADGET_TOGGLE,MODE_TEXTCENTER);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Clear Channel Flags",GADGET_CHANNELCLEAR,MODE_TEXTCENTER);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Reset Channel Flags",GADGET_CHANNELRESET,MODE_TEXTCENTER);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"- Reset -",GADGET_RESET,MODE_TEXTCENTER);
	glist.Return();

	//

	ShowFilter();
}
