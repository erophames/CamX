#include "songmain.h"
#include "editor.h"
#include "camxgadgets.h"
#include "object.h"
#include "object_song.h"
#include "audiomixer.h"
#include "audiomixeditor.h"
#include "arrangeeditor.h"
#include "gui.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "audioobject_volume.h"
#include "semapores.h"
#include "vstplugins.h"
#include "vstguiwindow.h"
#include "editbuffer.h"
#include "globalmenus.h"
#include "songmain.h"
#include "languagefiles.h"
#include "intern_audiofx.h"
#include <math.h>
#include "track_effects.h"
#include "audioports.h"
#include "editdata.h"
#include "audiosend.h"
#include "audioproc.h"
#include "audiorecord.h"
#include "objectpattern.h"
#include "editfunctions.h"

extern char *channelchannelsinfo[],*channelchannelsinfo_short[];
extern int channelschannelsnumber[];

#define NUMBERAUDIOMIXZOOM 22

double audiomixzooms[NUMBERAUDIOMIXZOOM];

#define SLIDERSTARTX 40
#define PEAKWIDTH 7 // Pixel Width Peak Display

#ifdef DEBUG
#define SHOWMIDITRACKS 1
#endif

#include "editaudiofx.h"

enum{
	GADGETID_AUDIOINPUT=GADGETID_EDITORBASE,
	GADGETID_AUDIOOUTPUT,
	GADGETID_TRACKS,
	GADGETID_BUS,
	GADGETID_AUTO,
	GADGETID_AUDIO,
	GADGETID_MIDI,
	GADGETID_IO,
	GADGETID_FX,
	GADGETID_INPUTFX,
	GADGETID_PAN,

	GADGETID_SHOWWHAT,
	GADGETID_PATTERN,
	GADGETID_SET1,
	GADGETID_SET2,
	GADGETID_SET3,
	GADGETID_SET4,
	GADGETID_SET5,
};

enum G_IDs
{
	AOUTDB_ID=64,
	MAXIMUM_ID,
	MUTECHANNEL_ID,
	RECORDCHANNEL_ID,
	SOLOCHANNEL_ID,
	AUDIOFX_ID,
	CHANNELEFFECT_ID,
	CHANNELEFFECTEDIT_ID,
	CHANNELTYPE_ID,
	CHANNELNAME_ID,
	CHANNELTHRU_ID,
	EMPTYSEND,
	CHANNEL_VOLUMEDISPLAY,
};

enum Am_EditData{

	EDIT_NAME=100,
	EDIT_SLIDERMIDIVOLUME,
	EDIT_SLIDERAUDIOVOLUME,
};

Edit_AudioMixEffects::Edit_AudioMixEffects(guiGadget_CW *db,bool ismast)
{
	ismaster=ismast;

	if(ismaster==TRUE)
	{
		id=OBJECTID_MASTEREFFECTS;
	}
	else
		id=OBJECTID_EFFECTS;

	bitmap=&db->gbitmap;
	effects=db;

	incleared=outcleared=false;
	recordmode=false;
	master=false;
	eqgadget=
		abgadget=
		abgadgetinput=
		maxdb=
		outdb=
		typegadget=
		emptyeffect=
		emptyinputeffect=
		emptysend=
		inputhardware=
		outputhardware=
		outputtype=
		channeltype=0;

	panonwindow=false;
	valueclicked=false;
	ismaster=false;
};

void Edit_AudioMixEffects::GenerateTypeString(char *cname)
{
	if(GetTrack())
	{
		Seq_Song *song=GetTrack()->song;

		/*
		bypassallfx=track->io.bypassallfx;

		if(bypassallfx==true)
		strcpy(cname,"[Fx]");
		else
		*/
		cname[0]=0;

		track_index=song->GetOfTrack(GetTrack());

		char nrs[NUMBERSTRINGLEN];
		mainvar->AddString(cname,mainvar->ConvertIntToChar(track_index+1,nrs));
	}
	else
	{
		Seq_Song *song=GetChannel()->audiosystem->song;

		/*
		bypassallfx=channel->io.bypassallfx;

		if(bypassallfx==true)
		strcpy(cname,"[Fx]");
		else
		*/
		cname[0]=0;

		switch(GetChannel()->audiochannelsystemtype)
		{
			/*
			case CHANNELTYPE_AUDIOCHANNEL:
			{
			mainvar->AddString(cname,"Chl ");

			char nrs[NUMBERSTRINGLEN];
			mainvar->AddString(cname,mainvar->ConvertIntToChar(song->audiosystem.GetAudioChannelIndex(GetChannel())+1,nrs));
			}
			break;
			*/
		case CHANNELTYPE_BUSCHANNEL:
			{
				mainvar->AddString(cname,"Bus ");

				char nrs[NUMBERSTRINGLEN];
				mainvar->AddString(cname,mainvar->ConvertIntToChar(GetChannel()->GetIndex()+1,nrs));
			}
			break;

			/*
			case CHANNELTYPE_AUDIOINSTRUMENT:
			{
			mainvar->AddString(cname,"Instr ");

			char nrs[NUMBERSTRINGLEN];
			mainvar->AddString(cname,mainvar->ConvertIntToChar(song->audiosystem.GetAudioInstrumentIndex(GetChannel())+1,nrs));
			}
			break;
			*/

		case CHANNELTYPE_DEVICEIN:
			{
				mainvar->AddString(cname,"In ");
				char nrs[NUMBERSTRINGLEN];
				mainvar->AddString(cname,mainvar->ConvertIntToChar(GetChannel()->GetIndex()+1,nrs));
			}
			break;

		case CHANNELTYPE_DEVICEOUT:
			{
				mainvar->AddString(cname,"Out ");
				char nrs[NUMBERSTRINGLEN];
				mainvar->AddString(cname,mainvar->ConvertIntToChar(GetChannel()->GetIndex()+1,nrs));
			}
			break;

		case CHANNELTYPE_MASTER:
			mainvar->AddString(cname,"Master");
			break;

		default:
			strcpy(cname,"???");
			break;
		}
	}
}

void Edit_AudioMix::Init()
{	
	if(opentrack && opentrack->GetAudioOut()->FirstChannel())
	{
		laststartchannel=opentrack->GetAudioOut()->FirstChannel()->channel;
		opentrack=0;
	}
	else
	{
		if(WindowSong()->GetFocusTrack() && WindowSong()->GetFocusTrack()->GetAudioOut()->FirstChannel())
			laststartchannel=WindowSong()->GetFocusTrack()->GetAudioOut()->FirstChannel()->channel;
		else
			laststartchannel=0;
	}

	double h=1.4*maingui->GetFontSizeX();
	double add=maingui->GetFontSizeX()/6;

	for(int i=0;i<NUMBERAUDIOMIXZOOM;i++)
	{
		audiomixzooms[i]=h;
		h+=add;
	}

	sizeslider=bitmap.GetTextWidth("000.000");

	SetMasterMode();

	FormYEnable(1,GetShowFlag()&SHOW_FX?true:false);
	CreateFilterChannels();
	InitGadgets();
}

void Edit_AudioMixSlider::FreeMemory()
{
	if(namegadget)
		namegadget->Delete();

	if(inputMIDIgadget)
		inputMIDIgadget->Delete();

	if(inputgadget)
		inputgadget->Delete();

	if(outputgadget)
		outputgadget->Delete();

	if(outputMIDIgadget)
		outputMIDIgadget->Delete();

	if(inputtypegadget)
		inputtypegadget->Delete();

	if(inputstring)
	{
		delete inputstring;
		inputstring=0;
	}

	if(outputstring)
	{
		delete outputstring;
		outputstring=0;
	}

	if(name)
	{
		delete name;
		name=0;
	}
}

bool Edit_AudioMixSlider::GetThru()
{
	return GetAudioIO()->thru;
}

void Edit_AudioMixSlider::ShowMIDIThru()
{
	if(mthru.ondisplay==false)
		return;

	slider->gbitmap.ShowThru(&mthru,mthru.status=GetTrack()->t_trackeffects.MIDIthru,true,GetBGColour());
}

void Edit_AudioMixSlider::ShowThru()
{
	if(thru.ondisplay==false)
		return;

	bitmap->ShowThru(&thru,thru.status=GetThru(),false,GetBGColour());
}

void Edit_AudioMixSlider::ShowType()
{
	if(type.ondisplay==false)
		return;

	bitmap->guiFillRect(type.x,type.y,type.x2,type.y2,bgcolour,COLOUR_GREY);
	bitmap->SetTextColour(COLOUR_BLACK);
	bitmap->guiDrawText(type.x+1,type.y2-1,type.x2-1,channelchannelsinfo[(type.type=GetAudioIO()->channel_type)]);
}

bool Edit_AudioMixSlider::GetMute()
{
	if(GetChannel() || GetTrack())
		return GetAudioIO()->audioeffects.GetMute();

	return false;
}

void Edit_AudioMixSlider::ShowMasterAB()
{
	masterab.status=editor->WindowSong()->audiosystem.systembypassfx;

	if(masterab.ondisplay==false)
		return;

	switch(masterab.status)
	{
	case true:
		bitmap->guiFillRect(masterab.x,masterab.y,masterab.x2,masterab.y2,COLOUR_RED,COLOUR_BLACK);
		bitmap->SetTextColour(COLOUR_WHITE);
		break;

	case false:
		bitmap->guiFillRect(masterab.x,masterab.y,masterab.x2,masterab.y2,COLOUR_BLACK,COLOUR_GREY);
		bitmap->SetTextColour(COLOUR_WHITE);
		break;
	}

	bitmap->guiDrawTextCenter(masterab.x,masterab.y,masterab.x2,masterab.y2,masterab.status==true?"-FX":"+FX");
}	

void Edit_AudioMixSlider::ShowMute()
{
	mute.status=GetMute();
	if(mute.ondisplay==false)
		return;

	bitmap->ShowMute(&mute,mute.status,GetBGColour());
}

int Edit_AudioMixSlider::GetSolo()
{
	if(GetTrack())
		return GetTrack()->GetSoloStatus();

	if(GetChannel())
		return GetChannel()->GetSoloStatus();

	return 0;
}

void Edit_AudioMixSlider::ShowSolo()
{
	solo.status=GetSolo();

	if(solo.ondisplay==false)
		return;

	bitmap->ShowSolo(&solo,solo.status,GetBGColour());
}

bool Edit_AudioMixSlider::GetRecord()
{
	if(GetTrack())
		return GetTrack()->record;

	return false;
}

void Edit_AudioMixSlider::ShowRecord()
{
	if(record.ondisplay==false)return;

	record.recordtracktype=GetTrack()->recordtracktype;
	bitmap->ShowRecord(&record,GetTrack(),record.status=GetRecord(),GetBGColour());
}

void Edit_AudioMixSlider::ShowInputMonitoring()
{
	if(inputmonitor.ondisplay==false)return;

	bitmap->ShowInputMonitoring(&inputmonitor,GetTrack(),inputmonitor.status=GetTrack()->io.inputmonitoring,GetBGColour());
}

void Edit_AudioMixSlider::ShowChildOpenClose()
{
	if(!GetTrack())return;

	if(child.ondisplay==false)return;
	bitmap->ShowChildTrackMode(&child,GetTrack());

	/*
	if(guiBitmap *bm=maingui->gfx.FindBitMap(==true?IMAGE_SUBTRACK_OPEN:IMAGE_SUBTRACK_CLOSE))
	{
	bitmap->guiDrawImage(child.x,child.y,child.x2,child.y2,bm);
	}
	*/
}

void Edit_AudioMixEffects::ShowTrackType()
{
	if(GetTrack())
	{
		status_MIDItype=GetTrack()->MIDItype;

		if(typegadget)
		{
			switch(status_MIDItype)
			{
			case Seq_Track::OUTPUTTYPE_MIDI:
				typegadget->ChangeButtonText("[>MIDI");
				break;

			case Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT:
				typegadget->ChangeButtonText("[>Instrument");
				break;

				/*
				case Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENTANDMIDI:
				typegadget->ChangeButtonText("[> MIDI+Instrument");
				break;
				*/
			}
		}
	}
}

void Edit_AudioMixFilterChannel::ShowIOPeak(Edit_AudioMix *editor,bool force)
{
	if(!editor->overview)
		return;

	// Overview
	peak.channel=channel;
	peak.track=track;
	peak.bitmap=&editor->overview->gbitmap;

	int px,px2;

	px=peak.x=peakx;
	peak.y=ovy+2;
	peak.x2=peakx2;

	if(peak.x2<peak.x+3)
		peak.x2=peak.x+3;

	if(peak.x2>=ovx2-1)
		return;

	px2=peak.x2;

	peak.y2=ovy2;
	peak.force=force;
	peak.noMIDItext=true;

	if(peak.x2<=ovx2-1)
	{
		if(isMIDI==true)
		{
			if(isaudio==true)
			{	
				peak.x2=peak.x+(peak.x2-peak.x)/2;
			}

			peak.ShowInit(false);
			peak.ShowPeak();

			if(peak.changed==true)
			{
				editor->overview->Blt(peak.x,peak.y,peak.x2,peak.y2);
				peak.changed=false;
			}

			if(isaudio==true)
			{
				peak.x=peak.x2+1;
				peak.x2=px2;
			}
		}

		if(isaudio==true)
		{
			peak.ShowInit(true);
			peak.ShowPeakSum();

			if(peak.changed==true)
				editor->overview->Blt(peak.x,peak.y,peak.x2,peak.y2);
		}		
	}
}

void Edit_AudioMixSlider::ShowMIDIPeak(bool force)
{
	if(isMIDI==false || m_vuy2<=m_vuy)
		return;

	mpeak.channel=GetChannel();
	mpeak.track=GetTrack();
	mpeak.bitmap=&slider->gbitmap;
	mpeak.x=m_vux;
	mpeak.y=m_vuy;
	mpeak.x2=m_vux2;
	mpeak.y2=m_vuy2;
	mpeak.force=force;

	mpeak.ShowInit(false);
	mpeak.ShowPeak();

	if(mpeak.changed==true)
		slider->Blt(m_vux,m_vuy,m_vux2,m_vuy2);
}

void Edit_AudioMixSlider::ShowIOPeak(bool force)
{
	if(editor->nopeak==true || isaudio==false || vuy2<=vuy)
		return;

	peak.channel=GetChannel();
	peak.track=GetTrack();

	if(GetTrack())
	{
		bool tinputmonitoring=GetTrack()->io.tempinputmonitoring;

		if(tinputmonitoring==true && GetTrack()->io.inputmonitoring==false && (editor->WindowSong()->status&Seq_Song::STATUS_PLAY))
			tinputmonitoring=false;

		if(force==false && peak.inputmonitoring!=tinputmonitoring)
		{
			force=true;
		}
	}

	peak.bitmap=bitmap;
	peak.x=vux;
	peak.y=vuy;
	peak.x2=vux2;
	peak.y2=vuy2;
	peak.force=force;

	peak.maxpeakx=maxpeakx;
	peak.maxpeaky=maxpeaky;
	peak.maxpeakx2=maxpeakx2;
	peak.maxpeaky2=maxpeaky2;
	peak.maxpeak=true;

	peak.ShowInit(true);
	peak.ShowPeak();

	if(peak.changed==true)
	{
		slider->Blt(maxpeakx,maxpeaky,maxpeakx2,maxpeaky2);
		slider->Blt(vux,vuy,vux2,vuy2);
	}
}

TrackHead *Edit_AudioMixSlider::GetTrackHead()
{
	if(GetTrack())
		return GetTrack();

	if(GetChannel())
		return GetChannel();

	return 0;
}

Edit_AudioMixSlider::Edit_AudioMixSlider(guiGadget_CW *db,bool ismast)
{
	ismaster=ismast;

	if(ismast==true)
	{
		id=OBJECTID_MASTERSLIDER;

		mute.id=OBJECTID_MASTERMUTE;
		//type.id=OBJECTID_MASTERTYPE;
	}
	else
		id=OBJECTID_SLIDER;

	idprio=-1;

	bitmap=&db->gbitmap;
	slider=db;

	masterab.deleteable=child.deleteable=mute.deleteable=record.deleteable=solo.deleteable=mthru.deleteable=thru.deleteable=mpan.deleteable=pan.deleteable=type.deleteable=inputmonitor.deleteable=false;
	masterab.slider=child.slider=mute.slider=record.slider=solo.slider=mthru.slider=thru.slider=mpan.slider=pan.slider=type.slider=inputmonitor.slider=this;

	name=inputstring=outputstring=0;
	masterabgadget=abgadget=namegadget=inputgadget=inputtypegadget=outputgadget=inputMIDIgadget=outputMIDIgadget=0;
}

int Edit_AudioMixRoot::GetBorder()
{
	if(GetTrack())
	{
		if(GetTrack()==GetTrack()->song->GetFocusTrack())
		{
			return GetTrack()->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT;
		}
	}

	return COLOUR_GREY;
}

int Edit_AudioMixRoot::GetBGColour()
{
	//if(track && track==editor->WindowSong()->GetFocusTrack())
	//	return bgcolour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
	if(GetChannel())
		return (bgcolour=GetChannel()->GetBgColour());

	if(GetTrack() && GetTrack()->ismetrotrack==true)
		return (bgcolour=COLOUR_METROTRACK);

	if(GetTrack() && GetTrack()->IsSelected()==true)
		return (bgcolour=GetTrack()->parent?COLOUR_BACKGROUNDCHILD_HIGHLITE:COLOUR_BACKGROUNDFORMTEXT_HIGHLITE);

	if(GetTrack() && GetTrack()->parent)
		return bgcolour=COLOUR_BACKGROUNDCHILD;

	return bgcolour=COLOUR_BACKGROUND;
}

void Edit_AudioMixSlider::ShowSlider()
{
	if(x<x2 && y<y2)
	{
		int bordercolor=GetBorder();

		switch(bordercolor)
		{
		case COLOUR_FOCUSOBJECT_SELECTED:
		case COLOUR_FOCUSOBJECT:
			{
				if(editor->solotrack==true)
					bitmap->guiFillRect(x,y,x2,y2,GetBGColour());
				else
					bitmap->guiFillRect(x,y,x2,y2,GetBGColour(),bordercolor);
			}
			break;

		default:
			{
				if(editor->solotrack==false)
				{
					bitmap->guiFillRect(x,y,x2,y2,GetBGColour());
					bitmap->guiDrawLineX(x,y,y2,COLOUR_WHITE);
					bitmap->guiDrawLineY(y,x,x2);
					bitmap->guiDrawLineX(x2,y+1,y2,COLOUR_GREY);
				}
			}
			break;
		}
	}

	///60% slider
	int ax;
	int w=x2-x;

	if(isaudio==true && isMIDI==true)
		w/=2;

	int h=w;

	h-=editor->sizeslider;

	// MIDI
	if(isMIDI==true)
	{
		int sy=y+2;

		m_maxpeakx=x+2;
		m_maxpeaky=sy;
		m_maxpeakx2=m_maxpeakx+h;
		m_maxpeaky2=sy+maingui->GetButtonSizeY();

		sy=m_maxpeaky2+1;

		m_vux=x+2;
		m_vuy=sy;
		m_vux2=x+h;

		m_sliderx=m_vux2+2;
		m_slidery=m_vuy;
		m_sliderx2=(x+w)-2;

		ax=m_sliderx2+2;
	}
	else
		ax=x;

	// Audio
	if(isaudio==true)
	{
		int sy=y+2;

		maxpeakx=ax+2;
		maxpeaky=sy;

		//if(GetChannel() && GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER)
		//	maxpeakx2=maxpeakx+h;
		//else
		maxpeakx2=x2-2;

		maxpeaky2=sy+maingui->GetButtonSizeY();

		sy=maxpeaky2+1;

		vux=ax+2;
		vuy=sy+2;
		vux2=vux+h;

		sliderx=vux+h+2;
		slidery=vuy;
		sliderx2=x2-2;
	}

	// Mute,Solo,Record
	guiGadgetList *mixergadgetlist=&editor->glist;
	mixergadgetlist->SelectForm(slider);

	slider->formchild->SetGX(x+2);

	int suby;

	if(editor->solotrack==true)
		suby=editor->GetShowFlag()&SHOW_IO?5:2;
	else
	{
		if(GetChannel() && GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER)
		{
			suby=editor->GetShowFlag()&SHOW_IO?3:2;
		}
		else
			suby=editor->GetShowFlag()&SHOW_IO?6:3;
	}

	slider->formchild->SetGYSub(suby,slider->GetY2());

	int yy=mixergadgetlist->GetLY();
	int heightmute=maingui->GetButtonSizeY();

	m_slidery2=slidery2=m_vuy2=vuy2=yy;
	//	yy-ADDYSPACE; // y2 VU+Slider

	yy+=ADDYSPACE;

	//child.ondisplay=mpan.ondisplay=pan.ondisplay=type.ondisplay=mthru.ondisplay=thru.ondisplay=mute.ondisplay=solo.ondisplay=record.ondisplay=inputmonitor.ondisplay=false;

	int addx=((x2-1)-(x+1))-2*ADDXSPACE;
	addx/=3;

	// Mute ------------------------------------
	int xx=x+1;

	editor->guiobjects.AddGUIObject(xx,yy,xx+addx-1,yy+heightmute,slider,&mute);

	xx+=addx+ADDXSPACE;

	if((GetTrack() && GetTrack()->ismetrotrack==false) || (GetChannel() && GetChannel()->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL))
	{
		editor->guiobjects.AddGUIObject(xx,yy,xx+addx-1,yy+heightmute,slider,&solo);
		xx+=addx+ADDXSPACE;
	}

	if(GetTrack() && GetTrack()->ismetrotrack==false)
	{
		editor->guiobjects.AddGUIObject(xx,yy,xx+addx-1,yy+heightmute,slider,&record);
		xx+=addx+ADDXSPACE;
	}

	slider->formchild->lastheight=heightmute;
	slider->formchild->SetGX(x+2);

	int sx;

	if(GetChannel() && GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER)
	{
		sx=xx;
	}
	else
	{
		yy+=heightmute+ADDYSPACE;
		sx=x+1;
	}

	if(GetTrack() && GetTrack()->ismetrotrack==false)
	{
		if(isMIDI==true)
		{
			// MIDI Thru
			int h=((x2-1)-(x+1))-2*ADDXSPACE;
			int smx2;

			if(isaudio==true)
			{
				h/=4;
				smx2=m_vux+h;
				addx=(x2-1)-(smx2+3*ADDXSPACE);
				addx/=3;
			}
			else
				smx2=m_sliderx2;

			editor->guiobjects.AddGUIObject(m_vux,yy,smx2,yy+heightmute,slider,&mthru);
			sx=smx2+ADDXSPACE;
		}
	}

	if(isaudio==true)
	{
		xx=sx;

		if((!GetChannel()) && GetTrack()->ismetrotrack==false)
		{
			editor->guiobjects.AddGUIObject(xx,yy,xx+addx-1,yy+heightmute,slider,&inputmonitor);
			xx+=addx+ADDXSPACE;

			editor->guiobjects.AddGUIObject(xx,yy,xx+addx-1,yy+heightmute,slider,&thru);
			xx+=addx+ADDXSPACE;
		}

		if((GetTrack() /*&& GetTrack()->ismetrotrack==false*/) || 
			(GetChannel() && GetChannel()->audiochannelsystemtype!=CHANNELTYPE_DEVICEIN && GetChannel()->audiochannelsystemtype!=CHANNELTYPE_DEVICEOUT)
			)
		{
			if(GetChannel() && GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER)
			{
				//editor->guiobjects.AddGUIObject(xx,yy,xx+(addx-1),yy+heightmute,slider,&type);

				editor->guiobjects.AddGUIObject(
					xx+addx+1,yy,
					xx+2*addx-1,yy+heightmute,
					slider,
					&masterab
					);	
			}
			else
			{
				editor->guiobjects.AddGUIObject(xx,yy,x2-1,yy+heightmute,slider,&type);
			}
		}
	}

	yy+=heightmute+ADDYSPACE;

	// IO
	if(editor->GetShowFlag()&SHOW_IO)
	{
		slider->formchild->SetGX(x+2);
		slider->formchild->SetGY(yy);

		if((!GetChannel()) || (GetChannel()->audiochannelsystemtype!=CHANNELTYPE_MASTER))
		{
			// Recording Input 
			if(GetTrack() && GetTrack()->ismetrotrack==false)
			{
				if(inputtypegadget=mixergadgetlist->AddButton(-1,-1,(x2-x)-4,-1,0,0,MODE_PARENT))
					editor->guiobjects.AddGUIObject(inputtypegadget,slider,new Edit_AudioMix_InputType(this,inputtypegadget));
			}
			else
				mixergadgetlist->AddButton(-1,-1,(x2-x)-4,-1,0,0,MODE_PARENT|MODE_DUMMY);

			yy+=heightmute+ADDYSPACE;
			slider->formchild->SetGY(yy);

			if(isMIDI==true)
			{
				// MIDI Input
				if(GetTrack() && GetTrack()->ismetrotrack==false)
				{
					if(inputMIDIgadget=mixergadgetlist->AddButton(-1,-1,m_sliderx2-m_vux,-1,0,0,MODE_PARENT))
						editor->guiobjects.AddGUIObject(inputMIDIgadget,slider,new Edit_AudioMix_MIDIInput(this,inputMIDIgadget));
				}
			}

			if(isaudio==true)
			{
				// Audio Input
				if(GetTrack() && GetTrack()->ismetrotrack==false)
				{
					slider->formchild->SetGX(vux);

					if(inputgadget=mixergadgetlist->AddButton(-1,-1,sliderx2-vux,-1,0,0,MODE_PARENT))
						editor->guiobjects.AddGUIObject(inputgadget,slider,new Edit_AudioMix_Input(this,inputgadget));
				}
				else
					mixergadgetlist->AddButton(-1,-1,sliderx2-vux,-1,0,0,MODE_PARENT|MODE_DUMMY);
			}

			yy+=heightmute+ADDYSPACE;
			slider->formchild->SetGY(yy);

			if(isMIDI==true)
			{
				// MIDI Output
				if(GetTrack()  && GetTrack()->ismetrotrack==false)
				{
					slider->formchild->SetGX(m_vux);

					if(outputMIDIgadget=mixergadgetlist->AddButton(-1,-1,m_sliderx2-m_vux,-1,0,0,MODE_PARENT))
						editor->guiobjects.AddGUIObject(outputMIDIgadget,slider,new Edit_AudioMix_MIDIOutput(this,outputMIDIgadget));
				}
			}

			if(isaudio==true)
			{
				//Output
				if(GetTrack() || (!GetChannel()->hardwarechannel) )
				{
					slider->formchild->SetGX(vux);

					if(outputgadget=mixergadgetlist->AddButton(-1,-1,sliderx2-vux,-1,0,0,MODE_PARENT))
						editor->guiobjects.AddGUIObject(outputgadget,slider,new Edit_AudioMix_Output(this,outputgadget,GetChannel() && GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER?true:false));
				}
				else
					mixergadgetlist->AddButton(-1,-1,sliderx2-vux,-1,0,0,MODE_PARENT|MODE_DUMMY);
			}
		}// if !=Master

		yy+=heightmute+ADDYSPACE;
	}


	// Name -----
	slider->formchild->SetGX(x);

	if(editor->solotrack==false)
	{	
		slider->formchild->SetGY(yy);

		if(namegadget=mixergadgetlist->AddButton(-1,-1,(x2-x)+1-ADDXSPACE,-1,0,0,MODE_PARENT|MODE_BOLD|MODE_BKCOLOUR))
			editor->guiobjects.AddGUIObject(namegadget,slider,new Edit_AudioMix_Name(this,namegadget,ismaster));
	}
	else
		namegadget=0;

	int pany2=0;

	if(pan_enable==true)
	{
		pany2=2*maingui->GetButtonSizeY();

		if(GetTrack() && isMIDI==true)
		{	
			editor->guiobjects.AddGUIObject(x,0,m_sliderx2+2,pany2,slider,&mpan);
		}

		if(isaudio==true)
		{
			editor->guiobjects.AddGUIObject(vux-2,0,x2,pany2,slider,&pan);
		}

		pany2+=2*ADDYSPACE;
	}

	if(editor->solotrack==false && GetTrack() && GetTrack()->FirstChildTrack())
	{
		editor->guiobjects.AddGUIObject((x2-1)-heightmute,pany2,x2-1,pany2+heightmute,slider,&child);	
	}

	pany2+=heightmute+2*ADDYSPACE;

	m_slidery=slidery=pany2;

	ShowMIDIPan(true);
	ShowPan(true);

	ShowMute();
	ShowSolo();
	ShowRecord();
	ShowInputMonitoring();
	ShowChildOpenClose();
	ShowMIDIThru();
	ShowThru();
	ShowType();
	ShowMIDIPeak(true);
	ShowMIDIVolumeSlider();
	ShowIOPeak(true);
	ShowVolumeSlider();

	if(GetChannel() && GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER)
		ShowMasterAB();
}

bool Edit_AudioMix_FX::ShowEffectName()
{
	bool changed=false;

	if(iae->audioeffect)
	{
		onoff=iae->audioeffect->plugin_on;
		bypass=iae->audioeffect->plugin_bypass;

		char *fxname=0;

		if(programname)
			delete programname;

		programname=iae->audioeffect->GetProgramName();

		if(bypass==true)
		{
			if(onoff==false)
			{
				if(programname && strlen(programname)>0)
					fxname=mainvar->GenerateString("[By][X] ",iae->audioeffect->GetEffectName(),":",programname);
				else
					fxname=mainvar->GenerateString("[By][X] ",iae->audioeffect->GetEffectName());
			}
			else
			{
				if(programname && strlen(programname)>0)
					fxname=mainvar->GenerateString("[By]",iae->audioeffect->GetEffectName(),":",programname);
				else
					fxname=mainvar->GenerateString("[By]",iae->audioeffect->GetEffectName());
			}
		}
		else
		{
			if(onoff==false)
			{
				if(programname && strlen(programname)>0)
					fxname=mainvar->GenerateString("[X] ",iae->audioeffect->GetEffectName(),":",programname);
				else
					fxname=mainvar->GenerateString("[X] ",iae->audioeffect->GetEffectName());
			}
			else
			{
				if(programname && strlen(programname)>0)
					fxname=mainvar->GenerateString(iae->audioeffect->GetEffectName(),":",programname);
				else
					fxname=mainvar->GenerateString(iae->audioeffect->GetEffectName());
			}
		}

		if(fxname)
		{
			changed=gadget->ChangeButtonText(fxname);
			delete fxname;
		}
	}

	return changed;
}

void Edit_AudioMix_Send::ShowSendName()
{
	if(sendname)
		delete sendname;

	if(send->sendbypass==true)
		sendname=mainvar->GenerateString("[By]",send->sendchannel->name);
	else
		sendname=mainvar->GenerateString(send->sendchannel->name);

	sendpost=send->sendpost;
	gadget->SetColourNoDraw(COLOUR_WHITE,sendpost==true?COLOUR_SENDPOST:COLOUR_SENDPRE);
	gadget->ChangeButtonText(sendname,true); // true=force COLOUR Refresh
}

char *Edit_AudioMix_Name::GetName()
{
	if(slider->GetTrack())
		return slider->GetTrack()->GetName();

	if(slider->GetChannel())
		return slider->GetChannel()->name;

	return 0;
}

void Edit_AudioMix_Name::EditName(char *newname)
{
	if(slider->GetTrack())
	{
		slider->GetTrack()->SetName(newname);
		slider->GetTrack()->ShowTrackName(0);
		return;
	}

	if(slider->GetChannel())
	{
		slider->GetChannel()->SetName(newname);
		return ;
	}
}

void Edit_AudioMix_Name::ShowName()
{
	char *h=0;

	gadget->bgcolour_rgb=0; // Default Black

	if(slider->GetTrack())
	{
		if(slider->GetTrack()->GetColour()->showcolour==true)
		{
			// Track Colour
			gadget->bordercolour_rgb=slider->GetTrack()->GetColour()->rgb;
			gadget->bordercolour_use=true;
		}
		else
			gadget->bordercolour_use=false;

		char hnr[NUMBERSTRINGLEN];
		h=mainvar->GenerateString(mainvar->ConvertIntToChar(slider->GetTrack()->song->GetOfTrack(slider->GetTrack())+1,hnr)," ",slider->GetTrack()->GetName()); // + Index
	}
	else
		if(slider->GetChannel())
		{
			if(slider->GetChannel()->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL)
			{
				char hnr[NUMBERSTRINGLEN];
				h=mainvar->GenerateString(mainvar->ConvertIntToChar(slider->GetChannel()->GetIndex()+1,hnr)," ",slider->GetChannel()->name); // + Index
			}
			else
				h=mainvar->GenerateString(slider->GetChannel()->name);
		}

		if(h){
			gadget->ChangeButtonText(h,true); // force Border Colour
			delete h;
		}

		if(slider->name)
			delete slider->name;

		slider->name=mainvar->GenerateString(GetName());
}

void Edit_AudioMix_InputType::ShowInputType()
{
	if(slider->GetTrack())
	{
		switch(recordtracktype=slider->GetTrack()->recordtracktype)
		{
		case TRACKTYPE_MIDI:
			gadget->ChangeButtonText("Rec:MIDI");
			break;

		case TRACKTYPE_AUDIO:
			gadget->ChangeButtonText("Rec:Audio");
			break;
		}
	}
}

void Edit_AudioMix_MIDIOutput::ShowMIDIOutput()
{
	if(slider->isMIDI==false)
		return;

	if(char *h=slider->GetTrack()->GetMIDIOutputString())
	{
		gadget->ChangeButtonText(h);
		delete h;
	}
}

void Edit_AudioMix_MIDIInput::ShowMIDIInput()
{
	if(slider->isMIDI==false)
		return;

	if(char *h=slider->GetTrack()->GetMIDIInputString())
	{
	gadget->ChangeButtonText(h);
	delete h;
	}
}

void Edit_AudioMix_Input::ShowInput()
{	
	if(slider->isaudio==false)
		return;

	if(gadget)
	{
		char *n=0;

		if(slider->GetTrack())
		{
			n=slider->GetTrack()->GetAudioFX()->GetAudioInputString();
		}
		else
			if(slider->GetChannel()->GetVIn())
			{
				if(slider->GetChannel()->io.bypass_input==true)
					n=mainvar->GenerateString("[By]",slider->GetChannel()->GetVIn()->name);
				else
					n=mainvar->GenerateString(slider->GetChannel()->GetVIn()->name);
			}
			else
				n=mainvar->GenerateString("-In-");

		if(slider->inputstring)
			delete slider->inputstring;

		slider->inputstring=mainvar->GenerateString(n);

		gadget->ChangeButtonText(mainvar->GenerateString("I:",n));
		if(n)delete n;
	}
}

void Edit_AudioMix_Output::ShowOutput()
{
	if(slider->isaudio==false)
		return;

	char *n=0;

	if(slider->GetTrack())
	{
		// Track
		TAudioOut tao;
		Seq_AudioIO ao;

		tao.audiochannelouts=&ao;
		tao.simple=true;

		slider->GetTrack()->ShowAudioOutput(&tao);

		n=mainvar->GenerateString(tao.returnstring);
	}
	else
	{
		// Channel
		char *c=0;

		if(slider->GetChannel()->GetVOut())
			c=slider->GetChannel()->GetVOut()->name;
		else
			c=channelchannelsinfo[slider->GetChannel()->io.channel_type];

		n=mainvar->GenerateString(c);
	}

	slider->outputstring=mainvar->GenerateString("O:",n);
	gadget->ChangeButtonText(slider->outputstring);

	if(n)delete n;
}


void Edit_AudioMix_ABFX::ShowAB()
{
	if(isMIDI==true)
	{
		ab=false;

		if(ab==false)
			gadget->mode |= MODE_TOGGLED;
		else
			gadget->mode CLEARBIT MODE_TOGGLED;

		gadget->ChangeButtonText(ab==false?"+ MFX":"- MFX");
	}
	else
	{
		ab=effects->GetAudioIO()->bypassallfx;

		if(ab==false)
			gadget->mode |= MODE_TOGGLED;
		else
			gadget->mode CLEARBIT MODE_TOGGLED;

		gadget->ChangeButtonText(ab==false?"+ FX >":"- FX >");
	}
}

void Edit_AudioMix_ABINPUTFX::ShowAB()
{
	ab=effects->GetAudioIO()->bypassallinputfx;

	if(ab==false)
		gadget->mode |= MODE_TOGGLED;
	else
		gadget->mode CLEARBIT MODE_TOGGLED;

	gadget->ChangeButtonText(ab==false?"> iFX +":"> iFX -");
}

void Edit_AudioMixEffects::ShowEffects()
{
	int bordercolor=GetBorder();

	switch(bordercolor)
	{
	case COLOUR_FOCUSOBJECT_SELECTED:
	case COLOUR_FOCUSOBJECT:
		{
			if(editor->solotrack==true)
				bitmap->guiFillRect(x,y,x2,y2,GetBGColour());
			else
				bitmap->guiFillRect(x,y,x2,y2,GetBGColour(),bordercolor);
		}
		break;

	default:
		{
			bitmap->guiFillRect(x,y,x2,y2,GetBGColour());
			bitmap->guiDrawLineX(x,y,y2,COLOUR_WHITE);
			bitmap->guiDrawLineY(y,x,x2);
			bitmap->guiDrawLineX(x2,y+1,y2,COLOUR_GREY);
		}
		break;
	}

	guiGadgetList *mixergadgetlist=&editor->glist;

	///60% slider

	int w=x2-x;
	int si;

	if(isMIDI==true &&
		(GetTrack() || GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER))
	{
		si=0;

		if(isaudio==true && isMIDI==true)
			w/=2;
		else
			w=0;
	}
	else
	{
		// Audio Only

		si=1;
		w=0;
	}

	int gid=0;

	for(int i=si;i<2;i++)
	{
		if(i==1 && isaudio==false)
			return;

		int ax=i==0?x:x+w;

		int sx=ax+2;
		int sx2;

		if(w==0)
			sx2=x2;
		else
			sx2=ax+w;

		mixergadgetlist->SelectForm(effects);
		effects->formchild->SetGXY(sx+1,y+1);

		switch(i)
		{
		case 0:
			{
				char cname[255];

				GenerateTypeString(cname);

				mainvar->AddString(cname,"  MIDI");

				mixergadgetlist->AddButton(-1,-1,sx2-sx,EQHEIGHT,"- - -",0,MODE_PARENT);
				mixergadgetlist->AddLY();

				if(editor->solotrack==false)
				{
					mixergadgetlist->AddButton(-1,-1,sx2-sx,-1,cname,0,MODE_PARENT);
					mixergadgetlist->AddLY();
				}

				// Output
				effects->formchild->SetGX(sx+1);
				if(abMIDIgadget=mixergadgetlist->AddButton(-1,-1,(sx2-sx),-1,0,0,MODE_PARENT|MODE_TOGGLE))
				{
					editor->guiobjects.AddGUIObject(abMIDIgadget,effects,new Edit_AudioMix_ABFX(this,abMIDIgadget,true));
					mixergadgetlist->AddLY();
				}

				effects->formchild->SetGX(sx+1);

				if(emptyMIDIeffect=mixergadgetlist->AddImageButton(-1,-1,sx2-sx,-1,IMAGE_MIDIEFFECT,gid,MODE_PARENT))
				{
					editor->guiobjects.AddGUIObject(emptyMIDIeffect,effects,new Edit_AudioMix_EmptyFX(this,emptyMIDIeffect,true));
					mixergadgetlist->AddLY();		
				}
			}
			break;

		case 1: //AUDIO
			{
				// Channel Type
				char cname[255];

				GenerateTypeString(cname);

				mainvar->AddString(cname,"  AUDIO");

				eqgadget=mixergadgetlist->AddButton(-1,-1,sx2-sx,EQHEIGHT,"- EQ -",0,MODE_PARENT);
				mixergadgetlist->AddLY();

				if(editor->solotrack==false)
				{
					typegadget=mixergadgetlist->AddButton(-1,-1,sx2-sx,-1,cname,0,MODE_PARENT,Cxs[CXS_SELECTANDINFO]);
					mixergadgetlist->AddLY();
				}
				else
					typegadget=0;

				int edw=bitmap->GetTextWidth("EEDD");
				int sdw=(sx2-sx)/2;

				// ### Effects ###
				if(isaudio==true)
				{
					if(editor->GetShowFlag()&SHOW_INPUTFX)
					{
						effects->formchild->SetGX(sx+1);

						if(!GetTrack())
						{
							abgadgetinput=0;
							emptyinputeffect=0;

							mixergadgetlist->AddButtonColour(-1,-1,sx2-sx,-1,0,0,MODE_PARENT|MODE_DUMMY,COLOUR_YELLOW,COLOUR_GADGETBACKGROUND);
							mixergadgetlist->AddLY();

							for(int i=0;i<editor->maxinputeffects;i++)
							{
								effects->formchild->SetGX(sx+1);
								//mixergadgetlist->AddLY();
								mixergadgetlist->AddButtonColour(-1,-1,sx2-sx,-1,0,0,MODE_PARENT|MODE_DUMMY,COLOUR_YELLOW,COLOUR_GADGETBACKGROUND);
								//g->SetColour(COLOUR_WHITE,COLOUR_YELLOW);

								mixergadgetlist->AddLY();
							}

							mixergadgetlist->AddButtonColour(-1,-1,sx2-sx,-1,0,0,MODE_PARENT|MODE_DUMMY,COLOUR_YELLOW,COLOUR_GADGETBACKGROUND);
							mixergadgetlist->AddLY();
						}
						else
						{
							if(abgadgetinput=mixergadgetlist->AddButtonColour(-1,-1,sx2-sx,-1,0,0,MODE_PARENT|MODE_TOGGLE,COLOUR_YELLOW,COLOUR_GADGETBACKGROUND))
							{
								editor->guiobjects.AddGUIObject(abgadgetinput,effects,new Edit_AudioMix_ABINPUTFX(this,abgadgetinput));
								mixergadgetlist->AddLY();
							}

							InsertAudioEffect *iae=GetInputFX()->FirstInsertAudioEffect();

							while(iae)
							{
								effects->formchild->SetGX(sx+1);

								if(guiGadget *g=mixergadgetlist->AddButtonColour(-1,-1,(sx2-sx)-edw,-1,0,0,MODE_PARENT,COLOUR_YELLOW,COLOUR_GADGETBACKGROUND))
								{
									editor->guiobjects.AddGUIObject(g,effects,new Edit_AudioMix_FX(this,iae,g));
									effectgadgets.AddEndO(g);
								}

								effects->formchild->SetGX(sx2-(edw-2));
								if(guiGadget *g=mixergadgetlist->AddButtonColour(-1,-1,edw-1,-1,"ED",0,MODE_PARENT|MODE_TEXTCENTER,COLOUR_YELLOW,COLOUR_GADGETBACKGROUND))
								{
									editor->guiobjects.AddGUIObject(g,effects,new Edit_AudioMix_FXEDIT(this,iae,g));
									effecteditgadgets.AddEndO(g);
								}

								mixergadgetlist->AddLY();
								iae=iae->NextEffect();
							}

							// End Input
							effects->formchild->SetGX(sx+1);

							if(emptyinputeffect=mixergadgetlist->AddImageButtonColour(-1,-1,sx2-sx,-1,IMAGE_AUDIOEFFECT,gid,MODE_PARENT,COLOUR_YELLOW,COLOUR_GADGETBACKGROUND))
							{
								editor->guiobjects.AddGUIObject(emptyinputeffect,effects,new Edit_AudioMix_EmptyInputFX(this,emptyinputeffect));
								mixergadgetlist->AddLY();		
							}

							if(GetInputFX()->GetCountEffects()<editor->maxinputeffects)
							{
								for(int i=GetInputFX()->GetCountEffects();i<editor->maxinputeffects;i++)
								{
									effects->formchild->SetGX(sx+1);
									//mixergadgetlist->AddLY();
									mixergadgetlist->AddButtonColour(-1,-1,sx2-sx,-1,0,0,MODE_PARENT|MODE_DUMMY,COLOUR_YELLOW,COLOUR_GADGETBACKGROUND);
									//g->SetColour(COLOUR_WHITE,COLOUR_YELLOW);

									mixergadgetlist->AddLY();
								}
							}
						}

					} // End Input

					// Output

					effects->formchild->SetGX(sx+1);
					if(abgadget=mixergadgetlist->AddButton(-1,-1,(sx2-sx),-1,0,0,MODE_PARENT|MODE_TOGGLE,"AB FX"))
					{
						editor->guiobjects.AddGUIObject(abgadget,effects,new Edit_AudioMix_ABFX(this,abgadget,false));
						mixergadgetlist->AddLY();
					}

					InsertAudioEffect *iae=GetFX()->FirstInsertAudioEffect();

					while(iae)
					{
						effects->formchild->SetGX(sx+1);

						if(guiGadget *g=mixergadgetlist->AddButton(-1,-1,(sx2-sx)-edw,-1,0,MODE_PARENT,"Eff"))
						{
							editor->guiobjects.AddGUIObject(g,effects,new Edit_AudioMix_FX(this,iae,g));
							effectgadgets.AddEndO(g);
						}

						effects->formchild->SetGX(sx2-(edw-2));
						if(guiGadget *g=mixergadgetlist->AddButton(-1,-1,edw-1,-1,"ED",0,MODE_PARENT|MODE_TEXTCENTER,"ED"))
						{
							editor->guiobjects.AddGUIObject(g,effects,new Edit_AudioMix_FXEDIT(this,iae,g));
							effecteditgadgets.AddEndO(g);
						}

						mixergadgetlist->AddLY();
						iae=iae->NextEffect();
					}

					effects->formchild->SetGX(sx+1);

					if(emptyeffect=mixergadgetlist->AddImageButton(-1,-1,sx2-sx,-1,IMAGE_AUDIOEFFECT,gid,MODE_PARENT,Cxs[CXS_ADDEFFECT]))
					{
						editor->guiobjects.AddGUIObject(emptyeffect,effects,new Edit_AudioMix_EmptyFX(this,emptyeffect,false));
						mixergadgetlist->AddLY();		
					}

					if(GetFX()->GetCountEffects()<editor->maxeffects)
					{
						for(int i=GetFX()->GetCountEffects();i<editor->maxeffects;i++)
						{
							effects->formchild->SetGX(sx+1);
							//mixergadgetlist->AddLY();
							mixergadgetlist->AddButton(-1,-1,sx2-sx,-1,0,MODE_PARENT|MODE_DUMMY);
							mixergadgetlist->AddLY();
						}
					}

					AudioSend *as=GetAudioIO()->FirstSend();

					while(as)
					{
						effects->formchild->SetGX(sx+1);

						if(guiGadget *g=mixergadgetlist->AddButton(-1,-1,(sx2-sx)-sdw,-1,0,MODE_PARENT,"Send"))
						{
							editor->guiobjects.AddGUIObject(g,effects,new Edit_AudioMix_Send(this,as,g));
							sendgadgets.AddEndO(g);
							//mixergadgetlist->AddLY();
						}

						effects->formchild->SetGX((sx2-sdw)+4);

						if(guiGadget_Volume *g=mixergadgetlist->AddVolumeButton(-1,-1,sdw-4,-1,0,as->sendvolume,MODE_PARENT,"Send Volume"))
						{
							editor->guiobjects.AddGUIObject(g,effects,new Edit_AudioMix_SendEdit(this,as,g,as->sendvolume));
							sendvolumegadgets.AddEndO(g);
						}

						mixergadgetlist->AddLY();

						as=as->NextSend();
					}

					if(GetAudioIO()->GetCountSends()<editor->maxsends)
					{
						for(int i=GetAudioIO()->GetCountSends();i<editor->maxsends;i++)
						{
							effects->formchild->SetGX(sx+1);

							mixergadgetlist->AddButton(-1,-1,sx2-sx,-1,0,MODE_PARENT|MODE_DUMMY);
							mixergadgetlist->AddLY();
						}

						//	mixergadgetlist->AddLY();
					}
					// Empy Sends Button

					effects->formchild->SetGX(sx+1);
					//	mixergadgetlist->AddLY();

					if(GetTrack()){

						if(emptysend=mixergadgetlist->AddButton(-1,-1,sx2-sx,-1,"Send",gid,MODE_PARENT,Cxs[CXS_ADDSEND]))
							editor->guiobjects.AddGUIObject(emptysend,effects,new Edit_AudioMix_EmptySend(this,emptysend));
					}
					else
						emptysend=0;

					mixergadgetlist->AddLY();
				}
			}break;
		}
	}
	editor->effectobjects.SetMaxHeight(y,mixergadgetlist->GetLY());

	// End Sends
}

void Edit_AudioMixSlider::ShowMIDIVolumeSlider()
{
	if(isMIDI==false)
		return;

	//Colour *col=fx->track?fx->track->GetColour():&fx->channel->colour;

	double h=m_sliderx2-m_sliderx;
	h*=0.27;

	int mx=m_sliderx+(int)h,fromx=m_sliderx;

	m_valueclicked=GetTrackHead()->m_volumeclicked;

	MIDIvolumeeditable=GetTrackHead()->CanAutomationObjectBeChanged(&GetTrackHead()->MIDIfx.velocity,0,0);

	int bgc=MIDIvolumeeditable==false?COLOUR_AUTOMATIONTRACKSUSED:GetBGColour();

	{
		UBYTE r,g,b;
		maingui->colourtable.GetRGB(bgc,&r,&g,&b,m_valueclicked==true?48: -24);
		bitmap->guiFillRect_RGB(m_sliderx,m_slidery,m_sliderx2,m_vuy2,RGB(r,g,b));
	}

	int from_y=m_slidery;
	int to_y=m_slidery2-2*maingui->GetFontSizeY();
	int from=127;
	int to=-127;
	int pos=velocity=GetChannel()?GetChannel()->MIDIfx.velocity.GetVelocity():GetTrack()->t_trackeffects.GetVelocity_NoParent();
	int y;
	int nully;

	bool fullslider;

	if(from_y<to_y)
	{
		m_valueondisplay=fullslider=true;
		//bitmap->guiDrawRect(fromx,from_y,mx+1,to_y,COLOUR_RED);		

		int nullpos=0;
		double h2;

		h2=from-to;
		nullpos+=from;
		pos=-(pos+to);

		// Add 0
		h2++;
		nullpos++;
		pos++;

		h=nullpos;

		h/=h2;
		h2=(to_y-from_y)-2;
		h*=h2;

		nully=from_y+(int)h;

		h2=from-to;
		h2++; // NULL
		h=pos;

		h/=h2;
		h2=(to_y-from_y)-2;
		h*=h2;

		y=(int)h;
		y+=from_y;

		mx++;
		from_y++;
	}
	else
	{
		m_valueondisplay=false;
		fullslider=false;

		nully=y=m_slidery2-2*maingui->GetButtonSizeY();
		to_y=m_slidery2-1;
	}

	if(m_valueondisplay==true)
	{
		guiFont *old=bitmap->SetFont(&maingui->standard_bold);

		// MIDI
		if(velocity>0)
		{
			//	fillcol=RGB(255,150,150);
			bitmap->SetTextColour(200,144,144);
		}
		else
			if(velocity==0)
			{
				//		fillcol=RGB(122,122,122);
				bitmap->SetTextColour(167,255,255);
			}
			else
			{
				//		fillcol=RGB(0,11,14);
				bitmap->SetTextColour(255,255,255);
			}

			bitmap->guiDrawLineY(nully-1,fromx,m_sliderx2,COLOUR_GREY_LIGHT);
			bitmap->guiDrawLineY(nully,fromx,m_sliderx2,COLOUR_BLACK);
			bitmap->guiDrawLineY(nully+1,fromx,m_sliderx2,COLOUR_GREY);

			int click_x=0;
			int click_x2=m_sliderx2-2;
			int click_y=y;
			int click_y2=y+2*maingui->GetFontSizeY()-8;

			m_valuex=fromx+1;
			m_valuey=y;
			m_valuex2=m_sliderx2;
			m_valuey2=click_y2;

			bitmap->guiDrawRect(fromx,y,m_sliderx2,click_y2,m_valueclicked==true?COLOUR_BLUE:COLOUR_GREY_DARK);
			bitmap->guiDrawRect(fromx+1,y+1,m_sliderx2-1,click_y2-1,m_valueclicked==true?COLOUR_BLUE_LIGHT:COLOUR_GREY);
			bitmap->guiFillRect(fromx+2,y,m_sliderx2-2,y+maingui->GetButtonSizeY(),m_valueclicked==true?COLOUR_BLACK:COLOUR_BLACK_LIGHT);

			char h2[NUMBERSTRINGLEN];
			bitmap->guiDrawTextCenter(fromx+3,y,m_sliderx2-2,y+maingui->GetButtonSizeY(),mainvar->ConvertIntToChar(velocity,h2));
			bitmap->SetFont(old);
	}

	if(fullslider==true)
	{
		if(to_y>from_y+16)
		{
			bitmap->guiDrawLineX(fromx,from_y,to_y-1,COLOUR_GREY_DARK);
			bitmap->guiDrawLineX(fromx+2,from_y+2,to_y-3,COLOUR_BLACK_LIGHT);
			bitmap->guiDrawLineX(fromx+4,from_y+4,to_y-5,COLOUR_BLACK);
			bitmap->guiDrawLineX(fromx+6,from_y+7,to_y-8,COLOUR_BLACK);
			bitmap->guiDrawLineX(fromx+8,from_y+10,to_y-11,COLOUR_BLACK);
		}
	}

}

double mv[]=
{
	9,
	6,
	3,
	-3,
	0,
	-4.5,
	-6,
	-12,
	-24
};

void Edit_AudioMixSlider::ShowVolumeSlider()
{
	if(isaudio==false || slidery2<=slidery)
		return;

	double h=sliderx2-sliderx;
	h*=0.18;

	if(h<7)h=7;
	else
		if(h>10)h=10;

	int mx=sliderx+(int)h,fromx=sliderx;

	valueclicked=GetTrackHead()->volumeclicked;
	volumeeditable=GetTrackHead()->CanAutomationObjectBeChanged(&GetTrackHead()->io.audioeffects.volume,0,0);

	int bgc=volumeeditable==false?COLOUR_AUTOMATIONTRACKSUSED:GetBGColour();

	{
		UBYTE r,g,b;
		maingui->colourtable.GetRGB(bgc,&r,&g,&b,valueclicked==true?30:0);
		bitmap->guiFillRect_RGB(sliderx,slidery,sliderx2,slidery2,RGB(r,g,b));
	}

	int from_y=slidery;
	int to_y=slidery2-maingui->GetButtonSizeY();
	int from=0;
	int to=LOGVOLUME_SIZE;
	int pos=LOGVOLUME_SIZE-mainaudio->ConvertLogArrayVolumeToInt(dbvalue=GetFX()->volume.GetValue());
	int y;
	int nully;

	bool fullslider;

	if(from_y<to_y)
	{
		valueondisplay=fullslider=true;

		int nullpos=LOGVOLUME_SIZE-mainaudio->ConvertLogArrayVolumeToInt(1);
		double h2;
		h2=to-from;

		h=nullpos;

		h/=h2;
		h2=(to_y-from_y)-2;
		h*=h2;

		nully=from_y+(int)h;

		h2=to-from;

		h=pos;

		h/=h2;
		h2=(to_y-from_y)-2;
		h*=h2;

		y=(int)h;
		y+=from_y;

		mx++;
		from_y++;
	}
	else
	{
		valueondisplay=false;
		fullslider=false;

		y=slidery2-2*maingui->GetFontSizeY();
		nully=y+1;
		to_y=slidery2-1;
	}

	bitmap->SetAudioColour(dbvalue);
	if(valueondisplay==true)
	{	

		if(fullslider==true)
		{
			{
				UBYTE r,g,b;
				maingui->colourtable.GetRGB(bgc,&r,&g,&b,valueclicked==true?48: -24);
				bitmap->guiFillRect_RGB(mx+2,y+2,sliderx2-2,to_y-1,RGB(r,g,b));
			}

			if(to_y>from_y+60)
			{
				for(int i=0;i<9;i++)
				{
					int pos=LOGVOLUME_SIZE-mainaudio->ConvertLogArrayVolumeToInt(mainaudio->ConvertDbToFactor(mv[i]));
					double h2;
					h2=to-from;

					h=pos;

					h/=h2;
					h2=(to_y-from_y)-2;
					h*=h2;

					int dy=from_y+(int)h;

					UBYTE r,g,b;

					maingui->colourtable.GetRGB(GetBGColour(),&r,&g,&b,mv[i]==0?-64:-32);
					bitmap->guiDrawLine_RGB(fromx,dy,sliderx2-2,dy,RGB(r,g,b));

					maingui->colourtable.GetRGB(GetBGColour(),&r,&g,&b,mv[i]==0?10:20);
					bitmap->guiDrawLine_RGB(fromx,dy+1,sliderx2-2,dy+1,RGB(r,g,b));
				}
			}

			{
				UBYTE r,g,b;
				maingui->colourtable.GetRGB(GetBGColour(),&r,&g,&b,-40);
				bitmap->guiFillRect_RGB(fromx+5,from_y+2,mx-1,to_y-1,RGB(r,g,b));
			}
		}

		int click_x=0;
		int click_x2=sliderx2-1;
		int click_y=y;
		int click_y2=y+2*maingui->GetFontSizeY()-8;

		valuex=fromx+1;
		valuey=y;
		valuex2=sliderx2;
		valuey2=click_y2;

		bitmap->guiFillRect(fromx+2,y,sliderx2-1,y+maingui->GetButtonSizeY(),valueclicked==true?COLOUR_BLACK:COLOUR_BLACK_LIGHT,valueclicked==true?COLOUR_BLUE:COLOUR_GREY_DARK);

		//TRACE ("SC %f\n",dbvalue);

		if(char *h=mainaudio->ScaleAndGenerateDBString(dbvalue,false))
		{
			bitmap->guiDrawTextCenter(fromx+2,y,sliderx2-1,y+maingui->GetFontSizeY()+2,h);
			//	bitmap->guiDrawTextCenter(fromx+3,y,sliderx2-2,y+maingui->GetFontSizeY()+2,h);
			delete h;
		}
	}
}

bool Edit_AudioMixSlider::ShowMIDIPan(bool force,bool mouseedit)
{
	if(mpan.ondisplay==false)
		return false;

	if(force==true)
	{
		bitmap->guiFillRect(mpan.x,mpan.y,mpan.x2,mpan.y2,GetBGColour());
		return true;
	}

	return false;
}

bool Edit_AudioMixSlider::ShowPan(bool force,bool mouseedit)
{
	if(pan.ondisplay==false)
		return false;

	if(force==true || cspan.CheckPanValues()==true)
	{
		int bordercolor=GetBorder();

		switch(bordercolor)
		{
		case COLOUR_FOCUSOBJECT_SELECTED:
		case COLOUR_FOCUSOBJECT:
			bitmap->guiFillRect(pan.x,pan.y,pan.x2,pan.y2,GetBGColour(),bordercolor);
			break;

		default:
			{
				bitmap->guiFillRect(pan.x,pan.y,pan.x2,pan.y2,GetBGColour());
				bitmap->guiDrawLineX(pan.x,pan.y,pan.y2,COLOUR_WHITE);
				bitmap->guiDrawLineY(pan.y,pan.x,pan.x2);
				bitmap->guiDrawLineX(pan.x2,pan.y+1,pan.y2,COLOUR_GREY);
				bitmap->guiDrawLineY(pan.y2,pan.x,pan.x2,COLOUR_GREY_DARK);
			}
			break;
		}

		cspan.x=pan.x+1;
		cspan.y=pan.y+1;
		cspan.x2=pan.x2-1;
		cspan.y2=pan.y2-1;
		cspan.channel=GetChannel();
		cspan.track=GetTrack();
		cspan.isMIDI=false;
		cspan.bitmap=bitmap;
		cspan.colour=GetBGColour();
		cspan.song=editor->WindowSong();

		pan.editable=cspan.ShowPan(pan.clicked);

		return true;
	}

	return false;
}

Peak *Edit_AudioMixSlider::GetInPeak()
{
	return GetChannel()?&GetChannel()->mix.peak:GetTrack()->GetPeak();
}

Peak *Edit_AudioMixSlider::GetOutPeak()
{
	return GetChannel()?&GetChannel()->mix.peak:GetTrack()->GetPeak();
}

void Edit_AudioMix::DeInitWindow()
{		
	filter.DeleteAllO();
	ClearEMCs();

	sliderxobjects.DeleteAllO(0);
	effectxobjects.DeleteAllO(0);

	effectobjects.DeleteAllO(0);
}

void Edit_AudioMix::RefreshRealtime_Slow()
{
	if(solotrack==false)
	{
		if(trackingindex!=mainsettings->autotracking[set])
			ShowTracking();

		if(patteringindex!=mainsettings->autopattering[set])
			ShowPattering();
	}

	if(GetShowFlag()&SHOW_AUTO)
	{
		guiObject *ot=guiobjects.FirstObject();

		while(ot)
		{	
			// 1. Raster
			if(ot->id==OBJECTID_SLIDER)
			{
				Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)ot;

				if(c->GetTrack())
				{
					if(CompareIsMIDIAUDIO(c)==false)
					{
						ShowAll(true);
						return;
					}
				}
			}

			ot=ot->NextObject();
		}
	}

	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{
		switch(ot->id)
		{
		case OBJECTID_FX:
		case OBJECTID_MASTERFX:
			{
				Edit_AudioMix_FX *fx=(Edit_AudioMix_FX *)ot;

				if(fx->ShowEffectName()==true)
					fx->Blt();
			}
			break;

		case OBJECTID_NAME:
		case OBJECTID_MASTERNAME:
			{
				Edit_AudioMix_Name *m=(Edit_AudioMix_Name*)ot;

				int rgb=0;
				bool colouruse;

				if(m->slider->GetTrack() && m->slider->GetTrack()->GetColour()->showcolour==true)
				{
					rgb=m->slider->GetTrack()->GetColour()->rgb;
					colouruse=true;
				}
				else
				{
					colouruse=false;
				}

				if(colouruse!=m->gadget->bordercolour_use || (colouruse==true && rgb!=m->gadget->bordercolour_rgb) )
				{
					m->ShowName();
					m->Blt();
				}
				else
					if(m->slider->name && m->GetName() && strcmp(m->slider->name,m->GetName())!=0)
					{
						m->ShowName();
						m->Blt();
					}
			}
			break;

		case OBJECTID_SEND:
			{
				Edit_AudioMix_Send *send=(Edit_AudioMix_Send *)ot;

				if( (send->sendname && strcmp(send->send->sendchannel->name,send->sendname)!=0) ||
					send->sendpost!=send->send->sendpost
					)
				{
					send->ShowSendName();
					send->Blt();
				}
			}
			break;

		case OBJECTID_INPUTTYPE:
			{
				Edit_AudioMix_InputType *it=(Edit_AudioMix_InputType *)ot;

				if(it->recordtracktype!=it->slider->GetTrack()->recordtracktype)
				{
					it->ShowInputType();
					it->Blt();
				}
			}
			break;

		case OBJECTID_IOINPUT:
			{
				Edit_AudioMix_Input *ip=(Edit_AudioMix_Input *)ot;

				if(ip->gadget && ip->slider->GetTrack() && ip->slider->GetTrack()->GetAudioFX()->CheckAudioInputString(ip->slider->inputstring)==false)
				{
					ip->ShowInput();
					ip->Blt();
				}
			}
			break;

		case OBJECTID_IOOUTPUT:
			{
				Edit_AudioMix_Output *op=(Edit_AudioMix_Output *)ot;

				if(op->gadget && op->slider->GetTrack() && op->slider->GetTrack()->GetAudioFX()->CheckAudioOutputString(op->slider->outputstring,"O:")==false)
				{
					op->ShowOutput();
					op->Blt();
				}
			}
			break;

		case OBJECTID_RECORD:
			{
				Edit_AudioMix_Record *r=(Edit_AudioMix_Record *)ot;

				if(r->slider->GetTrack()->recordtracktype!=r->recordtracktype)
				{
					r->slider->ShowRecord();
					r->Blt();
				}
			}
			break;
		}

		ot=ot->NextObject();
	}
}

void Edit_AudioMix::SetSet(int s)
{
	if(set!=s)
	{
		set=s;
		mainsettings->lastselectmixset=s;

		FormYEnable(1,GetShowFlag()&SHOW_FX?true:false);

		ShowAll(true);

		ShowFilter();
		for(int i=0;i<5;i++)
			if(g_set[i])
				g_set[i]->Toggle(i==set?true:false);
	}
}

void Edit_AudioMix::ShowFilter()
{
	if(g_fx)
		g_fx->Toggle(GetShowFlag()&SHOW_FX?true:false);

	if(g_inputfx)
		g_inputfx->Toggle(GetShowFlag()&SHOW_INPUTFX?true:false);

	if(g_pan)
		g_pan->Toggle(GetShowFlag()&SHOW_PAN?true:false);

	if(g_io)
		g_io->Toggle(GetShowFlag()&SHOW_IO?true:false);

	if(g_tracks)
		g_tracks->Toggle(GetShowFlag()&SHOW_TRACKS?true:false);

	if(g_auto)
		g_auto->Toggle(GetShowFlag()&SHOW_AUTO?true:false);

	if(g_audio)
		g_audio->Toggle(GetShowFlag()&SHOW_AUDIO?true:false);

	if(g_MIDI)
		g_MIDI->Toggle(GetShowFlag()&SHOW_MIDI?true:false);

	if(g_bus)
		g_bus->Toggle(GetShowFlag()&SHOW_BUS?true:false);

	if(g_audioinput)
		g_audioinput->Toggle(GetShowFlag()&SHOW_AUDIOINPUT?true:false);

	if(g_audiooutput)
		g_audiooutput->Toggle(GetShowFlag()&SHOW_AUDIOOUTPUT?true:false);
}

void Edit_AudioMix::ShowAll(bool refreshfilter)
{
	if(refresheffectsonly==true) // Plugin Effect refresh only
	{
		refresheffectsonly=false;

		if((GetShowFlag()&SHOW_INPUTFX) || (GetShowFlag()&SHOW_FX))
		{
			if(mastermode==true)
				DrawDBBlit(effects);
			else
				DrawDBBlit(effects,mastereffects);
		}

		return;
	}

	if(refreshfilter==true)
		CreateFilterChannels();

	DrawDBBlit(masterslider,slider,overview);
	DrawDBBlit(effects,mastereffects);
}

void Edit_AudioMix::RefreshTrack(Seq_Track *track)
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster
		if(ot->id==OBJECTID_SLIDER)
		{
			Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)ot;

			if(c->GetTrack()==track)
			{
				ShowAll();
				return;
			}
		}

		ot=ot->NextObject();
	}
}

void Edit_AudioMix::RefreshRealtime()
{	
	CheckXButtons();

	if(solotrack==false && CheckAuto()==true)
		return;

	// Background Colour
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster
		if(ot->id==OBJECTID_SLIDER)
		{
			Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)ot;
			int obgc=c->bgcolour;

			if(c->GetBGColour()!=obgc)
			{
				ShowAll();
				return;
			}
		}

		ot=ot->NextObject();
	}

	ot=guiobjects.FirstObject();

	while(ot)
	{	
		switch(ot->id)
		{
		case OBJECTID_RECORD:
			{
				Edit_AudioMix_Record *m=(Edit_AudioMix_Record*)ot;

				if(m->status!=m->slider->GetRecord())
				{
					DrawDBBlit(slider); // + Refresh Peak
					return;
				}
			}
			break;

		case OBJECTID_INPUTMONITORING:
			{
				Edit_AudioMix_InputMonitor *m=(Edit_AudioMix_InputMonitor*)ot;

				if(m->status!=m->slider->GetTrack()->io.inputmonitoring)
				{
					DrawDBBlit(slider); // + Refresh Peak
					return;
				}
			}
			break;
		}

		ot=ot->NextObject();
	}

	ot=guiobjects.FirstObject();

	while(ot)
	{	
		switch(ot->id)
		{
		case OBJECTID_SLIDER:
		case OBJECTID_MASTERSLIDER:
			{
				Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)ot;
				bool refresh=false;
				bool valueclicked=c->GetTrackHead()->volumeclicked,m_valueclicked=c->GetTrackHead()->m_volumeclicked;
				
				if(c->isMIDI==true)
				{
					c->ShowMIDIPeak();

					if(c->velocity!=c->GetTrackHead()->MIDIfx.GetVelocity() || 
						c->m_valueclicked!=m_valueclicked ||
						c->MIDIvolumeeditable!=c->GetTrackHead()->CanAutomationObjectBeChanged(&c->GetTrackHead()->MIDIfx.velocity,0,0)
						)
					{
						c->ShowMIDIVolumeSlider();
						refresh=true;
					}
				}

				if(c->isaudio==true)
				{
					c->ShowIOPeak();

					if(c->dbvalue!=c->GetFX()->volume.GetValue() ||
						c->valueclicked!=valueclicked ||
						c->volumeeditable!=c->GetTrackHead()->CanAutomationObjectBeChanged(&c->GetFX()->volume,0,0)
						)
					{
						c->ShowVolumeSlider();
						refresh=true;
					}
				}

				if(refresh==true)
					c->Blt();
			}
			break;

		case OBJECTID_PAN:
			{
				Edit_AudioMix_Pan *c=(Edit_AudioMix_Pan *)ot;

				if(c->slider->ShowPan(false,false)==true)
					c->Blt();
			}
			break;

		case OBJECTID_MASTERAB:
			{
				Edit_AudioMix_MasterAB *c=(Edit_AudioMix_MasterAB *)ot;

				if(c->status!=WindowSong()->audiosystem.systembypassfx)
				{
					c->slider->ShowMasterAB();
					c->Blt();
				}
			}
			break;

		case OBJECTID_MUTE:
		case OBJECTID_MASTERMUTE:
			{
				Edit_AudioMix_Mute *m=(Edit_AudioMix_Mute*)ot;

				if(m->status!=m->slider->GetMute())
				{
					m->slider->ShowMute();
					m->Blt();
				}
			}
			break;

		case OBJECTID_SOLO:
			{
				Edit_AudioMix_Solo *m=(Edit_AudioMix_Solo*)ot;

				if(m->status!=m->slider->GetSolo())
				{
					m->slider->ShowSolo();
					m->Blt();
				}
			}
			break;

		case OBJECTID_MIDITHRU:
			{
				Edit_AudioMix_MIDIThru *m=(Edit_AudioMix_MIDIThru*)ot;

				if(m->status!=m->slider->GetTrack()->t_trackeffects.MIDIthru)
				{
					m->slider->ShowMIDIThru();
					m->Blt();
				}
			}
			break;

		case OBJECTID_THRU:
			{
				Edit_AudioMix_Thru *m=(Edit_AudioMix_Thru*)ot;

				if(m->status!=m->slider->GetThru())
				{
					m->slider->ShowThru();
					m->Blt();
				}
			}
			break;

		case OBJECTID_FX:
		case OBJECTID_MASTERFX:
			{
				Edit_AudioMix_FX *fx=(Edit_AudioMix_FX *)ot;

				if(fx->onoff!=fx->iae->audioeffect->plugin_on || fx->bypass!=fx->iae->audioeffect->plugin_bypass)
				{
					fx->ShowEffectName();
					fx->Blt();
				}
			}
			break;

		case OBJECTID_ABFX:
		case OBJECTID_MASTERABFX:
			{
				Edit_AudioMix_ABFX *fx=(Edit_AudioMix_ABFX *)ot;

				if(fx->ab!=fx->effects->GetAudioIO()->bypassallfx)
				{				
					fx->ShowAB();
					fx->Blt();
				}
			}
			break;

		case OBJECTID_ABFXINPUT:
			{
				Edit_AudioMix_ABINPUTFX *fx=(Edit_AudioMix_ABINPUTFX *)ot;

				if(fx->ab!=fx->effects->GetAudioIO()->bypassallinputfx)
				{				
					fx->ShowAB();
					fx->Blt();
				}
			}
			break;

		case OBJECTID_SENDEDIT:
			{
				Edit_AudioMix_SendEdit *sed=(Edit_AudioMix_SendEdit *)ot;

				if(sed->volume!=sed->send->sendvolume)
				{
					sed->volume=sed->send->sendvolume;

					if(sed->gadget)
						sed->gadget->DrawGadget();
					sed->Blt();
				}
			}
			break;
		}

		ot=ot->NextObject();
	}

	if(solotrack==false && nooverview==false) // Overview
	{
		Edit_AudioMixFilterChannel *f=FirstFilterChannel();

		while(f)
		{
			f->ShowIOPeak(this);
			f=f->NextChannel();
		}
	}
}

Edit_AudioMix::Edit_AudioMix()
{
	editorid=EDITORTYPE_AUDIOMIXER;

	InitForms(FORM_HORZ1x2BAR_SLIDERV);

	resizeable=true;

	showoverviewonpaint=false;
	skipoverview=true;

	minwidth=minheight=maingui->GetButtonSizeY(10);

	editorname="Audio/MIDI Mixer";
	audiomasteronly=false;

	opentrack=0;

	// Menus
	menu_showdevice=0;

	// Flags
	flag_showdevices=true;
	laststartchannel=0;

	blinkrecordcounter=0;
	blinkrecordcounter_toggle=true;
	track=0; // 1 Track
	solotrack=false;
	simple=false;
	showmasterio=mainsettings->showmasterio;
	mixerzoom=mainsettings->mixerzoom;

	overview=0;
	horzgadget=0;
	horzzoomgadget=0;
	effectvertgadget=0;

	set=mainsettings->lastselectmixset;

	for(int i=0;i<5;i++)
		g_set[i]=0;

	g_tracks=g_audio=g_MIDI=g_bus=g_audioinput=g_audiooutput=g_auto=g_pattern=g_io=g_pan=g_fx=fx_fxgadget=fx_fx2gadget=0;

	slider=masterslider=0;
	effects=mastereffects=0;

	refresheffectsonly=false;
}

void Edit_AudioMix::RefreshSoloTrack()
{
	ClearEMCs();
	SetMasterMode();
	RefreshObjects(0,true);
}

void Edit_AudioMix::ShowDisplayMenu()
{
	if(menu_showdevice)
	{
		menu_showdevice->menu->Select(menu_showdevice->index,flag_showdevices);
	}
}

void Edit_AudioMix::CreateWindowMenu()
{
	if(NewWindowMenu())
	{
		CreateMenuList(menu);
		//AddEditMenu(menu);
	}
}

void Edit_AudioMix::CreateMenuList(guiMenu *menu)
{
	return;

	if(!menu)
		return;

	if(audiomasteronly==true)
	{
		if(guiMenu *n=menu->AddMenu(Cxs[CXS_OPTIONS],0))
		{
			class menu_showio:public guiMenu
			{
			public:
				menu_showio(Edit_AudioMix *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					/*
					editor->showmasterio=editor->showmasterio==true?false:true;
					editor->ShowChannels();
					editor->Repaint();

					editor->menu_showmasterio->menu->Select(editor->menu_showmasterio->index,editor->showmasterio==true?true:false);
					mainsettings->showmasterio=editor->showmasterio;
					*/
				} //

				Edit_AudioMix *editor;
			};

			menu_showmasterio=n->AddFMenu(Cxs[CXS_SHOWAUDIODEVICEIO],new menu_showio(this),showmasterio);
		}
	}

	if(guiMenu *n=menu->AddMenu(Cxs[CXS_FILE],0))
	{
		class menu_load:public guiMenu
		{
		public:
			menu_load(Edit_AudioMix *e)
			{
				editor=e;
			}

			void MenuFunction()
			{
				//editor->CopyFX(0);
			} //

			Edit_AudioMix *editor;
		};

		n->AddFMenu("Load (Active Channel Effects)",new menu_load(this));

		class menu_save:public guiMenu
		{
		public:
			menu_save(Edit_AudioMix *e)
			{
				editor=e;
			}

			void MenuFunction()
			{
				//	editor->PasteFX(0);	
			} //

			Edit_AudioMix *editor;
		};
		n->AddFMenu("Save (Active Channel Effects)",new menu_save(this));
	}

	if(guiMenu *n=menu->AddMenu(Cxs[CXS_EDIT],0))
	{
		maingui->AddUndoMenu(n);

		//	AddEditMenu(headmenu->editmenu);


		/*
		class menu_copy:public guiMenu
		{
		public:
		menu_copy(Edit_AudioMix *e)
		{
		editor=e;
		}

		void MenuFunction()
		{
		editor->CopyFX(0);
		} //

		Edit_AudioMix *editor;
		};

		n->AddFMenu(Cxs[CXS_COPYINSTRUMENTSANDEFFECT],new menu_copy(this));

		class menu_paste:public guiMenu
		{
		public:
		menu_paste(Edit_AudioMix *e)
		{
		editor=e;
		}

		void MenuFunction()
		{
		editor->PasteFX(0);	
		} //

		Edit_AudioMix *editor;
		};
		n->AddFMenu(Cxs[CXS_PASTE],new menu_paste(this));

		class menu_deletefx:public guiMenu
		{
		public:
		menu_deletefx(Edit_AudioMix *e,int f)
		{
		deleteflag=f;
		editor=e;
		}

		void MenuFunction()
		{
		editor->DeleteFX(0,deleteflag,true);	
		} //

		Edit_AudioMix *editor;
		int deleteflag;
		};
		*/

		//n->AddFMenu(Cxs[CXS_DELETEALLINSTRUMENTSANDFX],new menu_deletefx(this,AudioEffects::DELETE_INSTRUMENTS|AudioEffects::DELETE_FX));
		//n->AddFMenu(Cxs[CXS_DELETEALLINSTRUMENTS],new menu_deletefx(this,AudioEffects::DELETE_INSTRUMENTS));
		//n->AddFMenu(Cxs[CXS_DELETEALLFX],new menu_deletefx(this,AudioEffects::DELETE_FX));

		n->AddLine();

#ifdef OLDIE
		class menu_CreateAudioChannel:public guiMenu
		{
		public:
			menu_CreateAudioChannel(Seq_Song *s)
			{
				song=s;
			}

			void MenuFunction()
			{
				song->audiosystem.CreateChannel(0,0,0,0,song->audiosystem.activechannel);
				song->audiosystem.CreateChannelsFullName();
				maingui->RefreshAllEditors(song,EDITORTYPE_AUDIOMIXER,0);	
			} //

			Seq_Song *song;
		};
		n->AddFMenu(Cxs[CXS_CREATEAUDIOCHANNEL],new menu_CreateAudioChannel(WindowSong()),"CTRL+l");
#endif

		/*
		class menu_CreateAudioBus:public guiMenu
		{
		public:
		menu_CreateAudioBus(Seq_Song *s)
		{
		song=s;
		}

		void MenuFunction()
		{
		song->audiosystem.CreateBus(0,0);
		song->audiosystem.CreateChannelsFullName();
		maingui->RefreshAllEditors(song,EDITORTYPE_AUDIOMIXER,0);
		} //

		Seq_Song *song;
		};
		n->AddFMenu(Cxs[CXS_CREATEAUDIOBUSCHANNEL],new menu_CreateAudioBus(WindowSong()));
		*/

		/*
		class menu_CreateAudioInstr:public guiMenu
		{
		public:
		menu_CreateAudioInstr(Seq_Song *s){song=s;}

		void MenuFunction()
		{
		song->audiosystem.CreateAudioInstrumentChannel(0,0,0,0);
		song->audiosystem.CreateChannelsFullName();
		maingui->RefreshAllEditors(song,EDITORTYPE_AUDIOMIXER,0);	
		} //

		Seq_Song *song;
		};

		n->AddFMenu(Cxs[CXS_CREATEAUDIOINSTRUMENTCHANNEL],new menu_CreateAudioInstr(WindowSong()));
		*/

		n->AddLine();

		/*
		class menu_DeleteAudioChannel:public guiMenu
		{
		public:
		menu_DeleteAudioChannel(Edit_AudioMix *ed)
		{
		editor=ed;
		}

		void MenuFunction()
		{
		editor->DeleteAudioChannel();
		} //

		Edit_AudioMix *editor;
		};

		n->AddFMenu(Cxs[CXS_DELETECHANNEL],new menu_DeleteAudioChannel(this),SK_DEL);
		*/

		class menu_muteall:public guiMenu
		{
		public:
			menu_muteall(Seq_Song *s)
			{
				song=s;
			}

			void MenuFunction()
			{		
				for(int i=0;i<LASTSYNTHCHANNEL;i++)
				{
					AudioChannel *c=song->audiosystem.FirstChannelType(i);

					while(c)
					{
						c->io.audioeffects.SetMute(true);
						c=c->NextChannel();
					}
				}
			} //

			Seq_Song *song;
		};

		n->AddLine();

		n->AddFMenu(Cxs[CXS_MUTEALLCHANNELS],new menu_muteall(WindowSong()));

		class menu_muteoff:public guiMenu
		{
		public:
			menu_muteoff(Seq_Song *s)
			{
				song=s;
			}

			void MenuFunction()
			{		
				for(int i=0;i<LASTSYNTHCHANNEL;i++)
				{
					AudioChannel *c=song->audiosystem.FirstChannelType(i);

					while(c)
					{
						c->io.audioeffects.SetMute(false);
						c=c->NextChannel();
					}
				}
			} //

			Seq_Song *song;
		};

		n->AddFMenu(Cxs[CXS_UNMUTEALLCHANNELS],new menu_muteoff(WindowSong()));

		class menu_solooff:public guiMenu
		{
		public:
			menu_solooff(Seq_Song *s)
			{
				song=s;
			}

			void MenuFunction()
			{		
				for(int i=0;i<LASTSYNTHCHANNEL;i++)
				{
					AudioChannel *c=song->audiosystem.FirstChannelType(i);

					while(c)
					{
						if(c->io.audioeffects.GetSolo()==true)
							c->SetSolo(false,false);

						c=c->NextChannel();
					}
				}
			} //

			Seq_Song *song;
		};

		n->AddFMenu("Solo off (Audio Channels)",new menu_solooff(WindowSong()));

		/*
		class menu_bypassallsystem:public guiMenu
		{
		public:
		menu_bypassallsystem(Edit_AudioMix *e){editor=e;}

		void MenuFunction()
		{		
		editor->WindowSong()->audiosystem.SetSystemByPass(editor->WindowSong()->audiosystem.systembypassfx==true?false:true);
		editor->ShowActiveChannel();
		} //

		Edit_AudioMix *editor;
		};

		n->AddLine();
		bypassallfx=n->AddFMenu(Cxs[CXS_AUDIOMIXER_FX],new menu_bypassallsystem(this),WindowSong()->audiosystem.systembypassfx==true?false:true);
		*/
		/*
		class menu_resetchannelbypass:public guiMenu
		{
		public:
		menu_resetchannelbypass(Seq_Song *s,bool m)
		{
		song=s;
		mode=m;
		}

		void MenuFunction()
		{		
		song->audiosystem.DisableChannelsBypass(mode);
		} //

		Seq_Song *song;
		bool mode;
		};

		n->AddFMenu("Disable Channels Bypass",new menu_resetchannelbypass(WindowSong(),false));
		n->AddFMenu("Disable all Channels Effects Bypass",new menu_resetchannelbypass(WindowSong(),true));
		*/
	}

	/*
	n=menu->AddMenu("Audio Effects",0);
	if(n)
	{
	n->AddMenu("Effect Editor",AMIXMENU_FXEDITOR);
	}

	n=menu->AddMenu("Select",0);
	if(n)
	{
	n->AddMenu("All",AMIXMENU_SELECTALL);
	}
	*/

	if(guiMenu *n=menu->AddMenu(Cxs[CXS_FUNCTIONS],0))
	{
		class menu_solooff:public guiMenu
		{
		public:
			menu_solooff(Seq_Song *s)
			{
				song=s;
			}

			void MenuFunction()
			{		
				for(int i=0;i<LASTSYNTHCHANNEL;i++)
				{
					AudioChannel *c=song->audiosystem.FirstChannelType(i);

					while(c)
					{
						if(c->io.audioeffects.GetSolo()==true)
							c->SetSolo(false,false);

						c=c->NextChannel();
					}
				}
			} //

			Seq_Song *song;
		};

		/*
		// Functions Channel/System
		class menu_resetchannel:public guiMenu
		{
		public:
		menu_resetchannel(Edit_AudioMix *e,int f)
		{
		fflag=f;
		editor=e;
		}

		void MenuFunction()
		{		
		editor->ResetChannels(fflag);
		} //

		Edit_AudioMix *editor;
		int fflag;
		};

		if(guiMenu *sub=n->AddMenu("Audio Channel",0))
		{
		sub->AddFMenu("Reset Volume",new menu_resetchannel(this,RESET_CHANNEL_VOLUME));
		sub->AddFMenu("Reset Pan",new menu_resetchannel(this,RESET_CHANNEL_PAN));
		sub->AddFMenu("Reset Audio Effects",new menu_resetchannel(this,RESET_CHANNEL_FX));
		sub->AddFMenu("Reset Audio Instruments",new menu_resetchannel(this,RESET_CHANNEL_INSTRUMENTS));
		sub->AddFMenu("Reset All",new menu_resetchannel(this,RESETL_CHANNEL_ALL));
		}
		*
		n->AddLine();
		if(guiMenu *sub=n->AddMenu("All Channels",0))
		{
		sub->AddFMenu("Reset Volume",new menu_resetchannel(this,RESET_CHANNEL_VOLUME|RESET_CHANNEL_ALLCHANNELS));
		sub->AddFMenu("Reset Pan",new menu_resetchannel(this,RESET_CHANNEL_PAN|RESET_CHANNEL_ALLCHANNELS));
		sub->AddFMenu("Reset Audio Effects",new menu_resetchannel(this,RESET_CHANNEL_FX|RESET_CHANNEL_ALLCHANNELS));
		sub->AddFMenu("Reset Audio Instruments",new menu_resetchannel(this,RESET_CHANNEL_INSTRUMENTS|RESET_CHANNEL_ALLCHANNELS));
		sub->AddFMenu("Reset All",new menu_resetchannel(this,RESETL_CHANNEL_ALL|RESET_CHANNEL_ALLCHANNELS));
		}

		*/
	}

	if(guiMenu *n=menu->AddMenu(Cxs[CXS_DISPLAY],0))
	{
		class menu_display:public guiMenu
		{
		public:
			menu_display(Edit_AudioMix *e,int f)
			{
				fflag=f;
				editor=e;
			}

			void MenuFunction()
			{		
				if(editor->flag_showdevices==true)
					editor->flag_showdevices=false;
				else
					editor->flag_showdevices=true;

				//editor->ClearMixer();
				//	editor->ShowChannels();
				//	editor->RedrawOSGadgets();

				editor->ShowDisplayMenu();
			} //

			Edit_AudioMix *editor;
			int fflag;
		};

		// menu_showdevice=n->AddFMenu("Show Audio Device Channels",new menu_display(this,TOGGLE_SHOWDEVICE));
	}

	if(guiMenu *n=menu->AddMenu("Editor",0))
	{
		//n->AddFMenu("Mixer",new globmenu_AudioMixer(WindowSong(),0),"F7");
		//n->AddFMenu("Song (Channels/Bus) Mixer",new globmenu_SongMixer(WindowSong()),"F8");
		//n->AddFMenu("Song Master Mixer",new globmenu_SongMixer(WindowSong(),0,true),"F9");
		//n->AddFMenu("Audio Mastering/Bounce/Freeze Tracks",new globmenu_AMaster);
	}

	//maingui->AddCascadeMenu(menu);
}

void Edit_AudioMix::Delete()
{
	if(editmode==ED_TRACKS)
	{
		mainedit->DeleteSelectedTracks(WindowSong());
		return;
	}
}

guiMenu *Edit_AudioMix::CreateMenu2()
{
	if(DeletePopUpMenu(true))
	{
		if(guiMenu *menu=popmenu)
		{
			menu->AddMenu(" BUS CHANNELS ",0);
			menu->AddLine();

			guiMenu *n=menu->AddMenu(Cxs[CXS_EDIT],0);
			if(n)
			{
				//maingui->AddUndoMenu(n);

				if(guiMenu *mt=n)
				{
					class menu_newBus:public guiMenu
					{
					public:
						void MenuFunction()
						{
							guiWindow *win=maingui->FindWindow(EDITORTYPE_CREATEBUS,0,0);

							if(win)
								win->WindowToFront(true);
							else
								maingui->OpenEditorStart(EDITORTYPE_CREATEBUS,0,0,0,0,0,0);
						}
					};

					mt->AddFMenu(Cxs[CXS_CREATEXNEWBUS],new menu_newBus(),"Shift+B");


					class menu_delBus:public guiMenu
					{
					public:
						menu_delBus(guiWindow *w){win=w;}
						void MenuFunction()
						{
							mainedit->DeleteBusChannels(win->WindowSong());							
						}
						guiWindow *win;
					};

					mt->AddFMenu(Cxs[CXS_DELETEBUS],new menu_delBus(this));


#ifdef OLDIE
					class menu_muteall:public guiMenu
					{
					public:
						menu_muteall(Seq_Song *s,bool x,bool sel)
						{
							song=s;
							ex=x;
							selected=sel;
						}

						void MenuFunction()
						{		
							Seq_Track *t=song->FirstTrack();

							while(t)
							{
								if((ex==false || t!=song->GetFocusTrack()) && 
									(selected==false || (t->IsSelected()==true))
									)
									song->MuteTrack(t,true);

								t=t->NextTrack();
							}
						} //

						Seq_Song *song;
						bool ex,selected;
					};

					class menu_muteallexecptsel:public guiMenu
					{
					public:
						menu_muteallexecptsel(Seq_Song *s){song=s;}

						void MenuFunction()
						{		
							Seq_Track *t=song->FirstTrack();

							while(t)
							{
								if(t->IsSelected()==false)
									song->MuteTrack(t,true);

								t=t->NextTrack();
							}
						} //

						Seq_Song *song;
					};

					class menu_muteseltracks:public guiMenu
					{
					public:
						menu_muteseltracks(Edit_Arrange *ed,bool m){editor=ed;mute=m;}

						void MenuFunction()
						{	
							editor->editmode=Edit_Arrange::ED_TRACKS;
							editor->MuteSelected(mute);

						} //

						Edit_Arrange *editor;
						bool mute;
					};

					mt->AddLine();

					mt->AddFMenu(Cxs[CXS_MUTEALLTRACKS],new menu_muteall(WindowSong(),false,false));
					mt->AddFMenu(Cxs[CXS_MUTEALLSELTRACKS],new menu_muteseltracks(this,true),SK_MUTE);
					mt->AddFMenu(Cxs[CXS_MUTEALLTRACKSA],new menu_muteall(WindowSong(),true,false));
					mt->AddFMenu(Cxs[CXS_MUTEALLTRACKSASEL],new menu_muteallexecptsel(WindowSong()),"Ctrl+M");

					mt->AddFMenu(Cxs[CXS_UNMUTEALLTRACKS],new menu_muteoff(WindowSong(),false));
					mt->AddFMenu(Cxs[CXS_UNMUTEALLTRACKSSELT],new menu_muteseltracks(this,false),SK_UNMUTE);
					mt->AddLine();
					mt->AddFMenu(Cxs[CXS_SOLOOFFTRACKS],new menu_solooff(WindowSong()));

					/*
					class freeze:public guiMenu
					{
					public:
					freeze(Edit_Arrange *ar,bool f)
					{
					editor=ar;
					freezeonoff=f;
					}

					void MenuFunction()
					{		
					editor->FreezeTrack(0,freezeonoff);
					} //

					Edit_Arrange *editor;
					bool freezeonoff;
					};
					*/
					//mt->AddFMenu("Freeze Track",new freeze(this,true));
					//mt->AddFMenu("UnFreeze Track",new freeze(this,false));

					mt->AddLine();

					class menu_delempty:public guiMenu
					{
					public:
						menu_delempty(Edit_Arrange *ar){arrangeeditor=ar;}

						void MenuFunction()
						{		
							mainedit->DeleteAllEmptyTracks(arrangeeditor->WindowSong());	
						} //

						Edit_Arrange *arrangeeditor;
					};
					mt->AddFMenu(Cxs[CXS_DELETEALLEMPTYTRACKS],new menu_delempty(this));

					class menu_sort:public guiMenu
					{
					public:
						menu_sort(Edit_Arrange *ar){arrangeeditor=ar;}

						void MenuFunction()
						{		
							mainedit->SortTracks(arrangeeditor->WindowSong());	
						}

						Edit_Arrange *arrangeeditor;
					};

					mt->AddFMenu(Cxs[CXS_SORTTRACKSBYTYPE],new menu_sort(this));

					class menu_trackup:public guiMenu
					{
					public:
						menu_trackup(Edit_Arrange *ed){editor=ed;}

						void MenuFunction()
						{
							mainedit->MoveTracks(editor->WindowSong(),-1);
						} 

						Edit_Arrange *editor;
					};

					if(char *cu=mainvar->GenerateString("Ctrl+",Cxs[CXS_CURSORUP]))
					{
						mt->AddFMenu(Cxs[CXS_MOVETRACKUP],new menu_trackup(this),cu);
						delete cu;
					}

					class menu_trackdown:public guiMenu
					{
					public:
						menu_trackdown(Edit_Arrange *ed){editor=ed;}

						void MenuFunction()
						{
							mainedit->MoveTracks(editor->WindowSong(),1);
						} 

						Edit_Arrange *editor;
					};

					if(char *cd=mainvar->GenerateString("Ctrl+",Cxs[CXS_CURSORDOWN]))
					{
						mt->AddFMenu(Cxs[CXS_MOVETRACKDOWN],new menu_trackdown(this),cd);
						delete cd;
					}

#endif
				}

				/*
				n->AddLine();

				class menu_solomutereset:public guiMenu
				{
				public:
				menu_solomutereset(Seq_Song *s){song=s;}

				void MenuFunction()
				{		
				song->ResetSoloMute();
				} //

				Seq_Song *song;
				};
				n->AddFMenu(Cxs[CXS_RESETSOLOMUTE],new menu_solomutereset(WindowSong()));
				*/
			}

#ifdef OLDIE
			if(n=menu->AddMenu(Cxs[CXS_SELECT],0))
			{
				if(guiMenu *mt=n)
				{
					class menu_SelTracks:public guiMenu
					{
					public:
						menu_SelTracks(Edit_Arrange *ed,bool s)
						{
							editor=ed;
							sel=s;					
						}

						void MenuFunction()
						{
							editor->editmode=ED_TRACKS;
							editor->SelectAll(sel);
						}

						Edit_Arrange *editor;
						bool sel;
					};

					mt->AddFMenu(Cxs[CXS_SELECTALLTRACKS],new menu_SelTracks(this,true),SK_SELECTALL);
					mt->AddFMenu(Cxs[CXS_UNSELECTALLTRACKS],new menu_SelTracks(this,false),SK_DESELECTALL);
				}
			}
#endif

		}

	}

	return 0;
}

guiMenu *Edit_AudioMix::CreateMenu()
{
	if(DeletePopUpMenu(true))
	{
		if(guiMenu *menu=popmenu)
		{
			menu->AddMenu(" TRACKS ",0);
			menu->AddLine();

			guiMenu *n=menu->AddMenu(Cxs[CXS_EDIT],0);
			if(n)
			{
				//maingui->AddUndoMenu(n);

				if(guiMenu *mt=n)
				{
					class menu_delseltracks:public guiMenu
					{
					public:
						menu_delseltracks(Edit_AudioMix *ar){editor=ar;}

						void MenuFunction()
						{	
							editor->editmode=ED_TRACKS;
							editor->Delete();
						} //

						Edit_AudioMix *editor;
					};

					if(WindowSong()->FirstSelectedTrack())
						mt->AddFMenu(Cxs[CXS_DELETE],new menu_delseltracks(this),SK_DEL);

					mt->AddLine();

					mt->AddFMenu(0,new globmenu_cnewtrack(WindowSong(),0));

					class menu_newTracks:public guiMenu
					{
					public:
						void MenuFunction()
						{
							guiWindow *win=maingui->FindWindow(EDITORTYPE_CREATETRACKS,0,0);

							if(win)
								win->WindowToFront(true);
							else
								maingui->OpenEditorStart(EDITORTYPE_CREATETRACKS,0,0,0,0,0,0);
						}
					};

					mt->AddFMenu(Cxs[CXS_CREATEXNEWTRACKS],new menu_newTracks(),"Shift+X");


#ifdef OLDIE
					class menu_muteall:public guiMenu
					{
					public:
						menu_muteall(Seq_Song *s,bool x,bool sel)
						{
							song=s;
							ex=x;
							selected=sel;
						}

						void MenuFunction()
						{		
							Seq_Track *t=song->FirstTrack();

							while(t)
							{
								if((ex==false || t!=song->GetFocusTrack()) && 
									(selected==false || (t->IsSelected()==true))
									)
									song->MuteTrack(t,true);

								t=t->NextTrack();
							}
						} //

						Seq_Song *song;
						bool ex,selected;
					};

					class menu_muteallexecptsel:public guiMenu
					{
					public:
						menu_muteallexecptsel(Seq_Song *s){song=s;}

						void MenuFunction()
						{		
							Seq_Track *t=song->FirstTrack();

							while(t)
							{
								if(t->IsSelected()==false)
									song->MuteTrack(t,true);

								t=t->NextTrack();
							}
						} //

						Seq_Song *song;
					};

					class menu_muteseltracks:public guiMenu
					{
					public:
						menu_muteseltracks(Edit_Arrange *ed,bool m){editor=ed;mute=m;}

						void MenuFunction()
						{	
							editor->editmode=Edit_Arrange::ED_TRACKS;
							editor->MuteSelected(mute);

						} //

						Edit_Arrange *editor;
						bool mute;
					};

					mt->AddLine();

					mt->AddFMenu(Cxs[CXS_MUTEALLTRACKS],new menu_muteall(WindowSong(),false,false));
					mt->AddFMenu(Cxs[CXS_MUTEALLSELTRACKS],new menu_muteseltracks(this,true),SK_MUTE);
					mt->AddFMenu(Cxs[CXS_MUTEALLTRACKSA],new menu_muteall(WindowSong(),true,false));
					mt->AddFMenu(Cxs[CXS_MUTEALLTRACKSASEL],new menu_muteallexecptsel(WindowSong()),"Ctrl+M");

					mt->AddFMenu(Cxs[CXS_UNMUTEALLTRACKS],new menu_muteoff(WindowSong(),false));
					mt->AddFMenu(Cxs[CXS_UNMUTEALLTRACKSSELT],new menu_muteseltracks(this,false),SK_UNMUTE);
					mt->AddLine();
					mt->AddFMenu(Cxs[CXS_SOLOOFFTRACKS],new menu_solooff(WindowSong()));

					/*
					class freeze:public guiMenu
					{
					public:
					freeze(Edit_Arrange *ar,bool f)
					{
					editor=ar;
					freezeonoff=f;
					}

					void MenuFunction()
					{		
					editor->FreezeTrack(0,freezeonoff);
					} //

					Edit_Arrange *editor;
					bool freezeonoff;
					};
					*/
					//mt->AddFMenu("Freeze Track",new freeze(this,true));
					//mt->AddFMenu("UnFreeze Track",new freeze(this,false));

					mt->AddLine();

					class menu_delempty:public guiMenu
					{
					public:
						menu_delempty(Edit_Arrange *ar){arrangeeditor=ar;}

						void MenuFunction()
						{		
							mainedit->DeleteAllEmptyTracks(arrangeeditor->WindowSong());	
						} //

						Edit_Arrange *arrangeeditor;
					};
					mt->AddFMenu(Cxs[CXS_DELETEALLEMPTYTRACKS],new menu_delempty(this));

					class menu_sort:public guiMenu
					{
					public:
						menu_sort(Edit_Arrange *ar){arrangeeditor=ar;}

						void MenuFunction()
						{		
							mainedit->SortTracks(arrangeeditor->WindowSong());	
						}

						Edit_Arrange *arrangeeditor;
					};

					mt->AddFMenu(Cxs[CXS_SORTTRACKSBYTYPE],new menu_sort(this));

					class menu_trackup:public guiMenu
					{
					public:
						menu_trackup(Edit_Arrange *ed){editor=ed;}

						void MenuFunction()
						{
							mainedit->MoveTracks(editor->WindowSong(),-1);
						} 

						Edit_Arrange *editor;
					};

					if(char *cu=mainvar->GenerateString("Ctrl+",Cxs[CXS_CURSORUP]))
					{
						mt->AddFMenu(Cxs[CXS_MOVETRACKUP],new menu_trackup(this),cu);
						delete cu;
					}

					class menu_trackdown:public guiMenu
					{
					public:
						menu_trackdown(Edit_Arrange *ed){editor=ed;}

						void MenuFunction()
						{
							mainedit->MoveTracks(editor->WindowSong(),1);
						} 

						Edit_Arrange *editor;
					};

					if(char *cd=mainvar->GenerateString("Ctrl+",Cxs[CXS_CURSORDOWN]))
					{
						mt->AddFMenu(Cxs[CXS_MOVETRACKDOWN],new menu_trackdown(this),cd);
						delete cd;
					}

#endif
				}

				n->AddLine();

				class menu_solomutereset:public guiMenu
				{
				public:
					menu_solomutereset(Seq_Song *s){song=s;}

					void MenuFunction()
					{		
						song->ResetSoloMute();
					} //

					Seq_Song *song;
				};
				n->AddFMenu(Cxs[CXS_RESETSOLOMUTE],new menu_solomutereset(WindowSong()));
			}

#ifdef OLDIE
			if(n=menu->AddMenu(Cxs[CXS_SELECT],0))
			{
				if(guiMenu *mt=n)
				{
					class menu_SelTracks:public guiMenu
					{
					public:
						menu_SelTracks(Edit_Arrange *ed,bool s)
						{
							editor=ed;
							sel=s;					
						}

						void MenuFunction()
						{
							editor->editmode=ED_TRACKS;
							editor->SelectAll(sel);
						}

						Edit_Arrange *editor;
						bool sel;
					};

					mt->AddFMenu(Cxs[CXS_SELECTALLTRACKS],new menu_SelTracks(this,true),SK_SELECTALL);
					mt->AddFMenu(Cxs[CXS_UNSELECTALLTRACKS],new menu_SelTracks(this,false),SK_DESELECTALL);
				}
			}
#endif

		}

	}


	//maingui->AddCascadeMenu(this,menu);
	return 0;
}

void Edit_AudioMix::CreateChannelTypePopMenu(Edit_AudioMixSlider *chl)
{
	if(DeletePopUpMenu(true))
	{
		if(chl->GetChannel())
		{
			if(char *h=mainvar->GenerateString("Output(Bus):",chl->GetChannel()->name))
			{
				popmenu->AddMenu(h,0);
				delete h;
			}

			popmenu->AddLine();
			chl->GetChannel()->CreateChannelTypeMenu(popmenu); 
		}
		else
			if(chl->GetTrack() && chl->GetTrack()->CanChangeType()==true)
			{
				char h2[NUMBERSTRINGLEN],*tracknr=mainvar->ConvertIntToChar(WindowSong()->GetOfTrack(chl->GetTrack())+1,h2);

				if(char *h=mainvar->GenerateString("Output (Track) ",tracknr,":",chl->GetTrack()->GetName()))
				{
					popmenu->AddMenu(h,0);
					delete h;
				}

				chl->GetTrack()->CreateChannelTypeMenu(popmenu); 
			}

			ShowPopMenu();
	}
}

void Edit_AudioMix::CreateOutputPopMenu(Edit_AudioMixSlider *chl,bool isMIDI)
{
	if(isMIDI==true)
	{
		maingui->EditTrackMIDIOutput(this,chl->GetTrack());
		return;
	}

	if(chl->GetTrack())
	{
		maingui->EditTrackOutput(this,chl->GetTrack());
		return;
	}

	if(DeletePopUpMenu(true))
	{
		if(char *h=mainvar->GenerateString("Output/Bus:",chl->GetChannel()->name,"(",channelchannelsinfo[chl->GetChannel()->io.channel_type],")"))
		{
			popmenu->AddMenu(h,0);
			delete h;
		}

		popmenu->AddLine();

		if(mainaudio->GetActiveDevice())
		{
			chl->GetChannel()->CreateChannelOutputMenu(popmenu);
		}

		ShowPopMenu();
	}
}

void Edit_AudioMix::CreateInputPopMenu(Edit_AudioMixSlider *chl,bool isMIDI)
{
	if(chl->GetTrack())
	{
		if(!(WindowSong()->status&Seq_Song::STATUS_RECORD))
		{
			if(isMIDI==true)
			{
				maingui->EditTrackMIDIInput(this,chl->GetTrack());
				return;
			}

			maingui->EditTrackInput(this,chl->GetTrack());
		}
	}
	else
	{
		DeletePopUpMenu();

		if(AudioDevice *dev=WindowSong()->audiosystem.device)
		{
			if(DeletePopUpMenu(true))
			{
				char *h;

				if(chl->GetChannel()->GetVIn())
				{
					char h2[NUMBERSTRINGLEN];
					h=mainvar->GenerateString("Audio In [",mainvar->ConvertIntToChar(chl->GetChannel()->GetVIn()->channels,h2),"] ",chl->GetChannel()->GetVIn()->name);
				}
				else
					h=mainvar->GenerateString("Audio In:",Cxs[CXS_NOINPUTHARDWARE]);

				if(h)
				{
					popmenu->AddMenu(h,0);

					delete h;

					class menu_achlin:public guiMenu
					{
					public:
						menu_achlin(AudioDevice *d,AudioChannel *chl,AudioPort *hwl)
						{
							channel=chl;
							devicechl=hwl;
							device=d;
						}

						void MenuFunction()
						{
							if(device)
							{
								mainaudio->defaultchannelins=devicechl->channels;
								//mainaudio->defaultchannelindex_in=device->GetOfVInputChannel(devicechl,devicechl->channels);
							}

							channel->SetRecordChannel(devicechl);
							maingui->RefreshAllEditors(channel->audiosystem->song,EDITORTYPE_AUDIOMIXER,REFRESH_INCHANNELS);
						} //

						AudioDevice *device;
						AudioChannel *channel;
						AudioPort *devicechl;
					};

					popmenu->AddLine();
					if(chl->GetChannel()->GetVIn())
					{
						popmenu->AddFMenu(Cxs[CXS_NOINPUTHARDWARE],new menu_achlin(0,chl->GetChannel(),0));

						class menu_achlinbypass:public guiMenu
						{
						public:
							menu_achlinbypass(AudioChannel *chl)
							{
								channel=chl;
							}

							void MenuFunction()
							{
								channel->SetAudioInBypass(channel->io.bypass_input==true?false:true); // Toggle
								maingui->RefreshAllEditors(channel->audiosystem->song,EDITORTYPE_AUDIOMIXER,REFRESH_INCHANNELS);
							} //

							AudioChannel *channel;
						};

						popmenu->AddFMenu("Audio In Bypass",new menu_achlinbypass(chl->GetChannel()),chl->GetChannel()->io.bypass_input);
					}

					popmenu->AddLine();

					int chls=chl->GetChannel()->io.GetChannels();

					for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
					{
						if(channelschannelsnumber[i]<=chls)
						{
							if(guiMenu *sub=popmenu->AddMenu(channelchannelsinfo[i],0))
							{
								for(int p=0;p<CHANNELSPERPORT;p++)
								{
									AudioPort *inport=&dev->inputaudioports[i][p];
									sub->AddFMenu(inport->name,new menu_achlin(dev,chl->GetChannel(),inport),chl->GetChannel()->GetVIn()==inport?true:false);
								}
							}
							else
								break;
						}
					}

					/*
					if(guiMenu *s=popmenu->AddMenu(dev->devname,0))
					{
					int chls=chl->channel->io.GetChannels();

					for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
					{
					if(channelschannelsnumber[i]<=chls)
					{
					for(int p=0;p<CHANNELSPERPORT;p++)
					{
					AudioPort *inport=&dev->inputaudioports[i][p];

					s->AddFMenu(inport->name,new menu_achlin(dev,chl->channel,inport),chl->channel->GetVIn()==inport?true:false);
					}
					}
					else
					break;
					}



					AudioVHardwareChannel *ahc=dev->FirstVInputChannel();

					while(ahc){
					s->AddFMenu(ahc->name,new menu_achlin(dev,chl->channel,ahc),chl->channel->GetVIn()==ahc?true:false);
					ahc=ahc->NextVChannel();
					}

					}
					*/

					ShowPopMenu();
				}
			}
		}
	}
}

/*
void Edit_AudioMix::AddOnOffMenu(InsertAudioEffect *oldeffect)
{
if(oldeffect->audioeffect)
{
class menu_onofftoggle:public guiMenu
{
public:
menu_onofftoggle(InsertAudioEffect *e){effect=e;}

void MenuFunction()
{
effect->audioeffect->OnOff(effect->audioeffect->plugin_on==false?true:false);
//	maingui->RefreshBypass(effect);
} //

InsertAudioEffect *effect;
};

popmenu->AddFMenu(Cxs[CXS_ON],new menu_onofftoggle(oldeffect),oldeffect->audioeffect->plugin_on);

class menu_bypasstoggle:public guiMenu
{
public:
menu_bypasstoggle(InsertAudioEffect *e){effect=e;}

void MenuFunction()
{
effect->audioeffect->Bypass(effect->audioeffect->plugin_bypass==false?true:false);
//	maingui->RefreshBypass(effect);
} //

InsertAudioEffect *effect;
};

popmenu->AddFMenu("Bypass",new menu_bypasstoggle(oldeffect),oldeffect->audioeffect->plugin_bypass);
}
}
*/

void Edit_AudioMix::MouseWheel(int delta,guiGadget *db)
{
	if(db==effects)
	{
		if(effectvertgadget)
			effectvertgadget->DeltaY(delta);

		return;
	}

	if(db==slider || db==masterslider)
	{
		guiObject *o=db->CheckObjectClicked(); // Object Under Mouse ?

		if(!o)
			return;

		int mx=db->GetMouseX(),my=db->GetMouseY();

		switch(o->id)
		{	
		case OBJECTID_SLIDER:
			{
				Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)o;

				// MIDI
				if(c->isMIDI==true && c->m_valueondisplay==true && maingui->CheckIfInRange(mx,my,c->x,c->m_sliderx2,c->y,c->y2)==true)
				{
					OSTART atime=GetAutomationTime();

					Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

					while(flt)
					{
						if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio)
						{
							// MIDI Velocity +127 - -127
							if(flt->GetFX()==c->GetFX() || (flt->isMIDI==true && c->GetTrack() && flt->track && maingui->GetCtrlKey()==false && c->GetTrack()->IsSelected()==true && flt->track->IsSelected()==true))
							{
								int p=c->GetChannel()?c->GetChannel()->MIDIfx.GetVelocity():flt->track->t_trackeffects.GetVelocity_NoParent();

								p-=delta;

								if(c->GetChannel())
									c->GetChannel()->SetMIDIVelocity(p,atime);
								else
									flt->track->t_trackeffects.SetVelocity(p,atime);
							}
						}

						flt=flt->NextChannel();
					}

					return;
				}

				if(c->isaudio==true && c->valueondisplay==true && maingui->CheckIfInRange(mx,my,c->vux,c->x2,c->y,c->y2)==true)
				{
					OSTART atime=GetAutomationTime();

					Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

					while(flt)
					{
						if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio)
						{
							if(flt->GetFX()==c->GetFX() || (c->GetTrack() && flt->track && ((flt->GetFX()==c->GetFX() || maingui->GetCtrlKey()==false)&& c->GetTrack()->IsSelected()==true && flt->track->IsSelected()==true)))
							{
								// Audio
								//int p=mainaudio->ConvertLogArrayVolumeToInt(flt->GetFX()->volume.GetValue());

								double p=flt->GetFX()->volume.GetParm(0);
								p*=LOGVOLUME_SIZE;


								flt->GetFX()->volume.AutomationEdit(WindowSong(),atime,0,mainaudio->ConvertToLogScale(p-delta));
							}
						}

						flt=flt->NextChannel();
					}

					return;
				}

				return;
			}
			break;

		case OBJECTID_MASTERSLIDER:
			{
				Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)o;

				// MIDI
				if(c->isMIDI==true && c->m_valueondisplay==true && maingui->CheckIfInRange(mx,my,c->x,c->m_sliderx2,c->y,c->y2)==true)
				{
					OSTART atime=GetAutomationTime();

					int p=c->GetChannel()->MIDIfx.GetVelocity();

					p-=delta;

					if(c->GetChannel())
						c->GetChannel()->SetMIDIVelocity(p,atime);

					return;
				}

				if(c->isaudio==true && c->valueondisplay==true && maingui->CheckIfInRange(mx,my,c->vux,c->x2,c->y,c->y2)==true)
				{
					OSTART atime=GetAutomationTime();
					double p=c->GetFX()->volume.GetParm(0);
					p*=LOGVOLUME_SIZE;
					c->GetFX()->volume.AutomationEdit(WindowSong(),atime,0,mainaudio->ConvertToLogScale(p-delta));

					return;
				}
			}
			break;
		}
	}
}

void Edit_AudioMix::DoubleClickGadget(guiGadget *g)
{	
	guiObject *o=guiobjects.FirstObject();
	while(o)
	{
		if(o->gadget==g)
		{
			switch(o->id)
			{
			case OBJECTID_NAME:
			case OBJECTID_MASTERNAME:
				{
					Edit_AudioMix_Name *c=(Edit_AudioMix_Name *)o;

					if(EditData *edit=new EditData)
					{
						// long position;
						edit->song=0;
						edit->win=this;
						edit->parentdb=c->slider->slider;

						edit->x=o->x;
						edit->y=o->y;
						edit->width=o->x2-o->x;
						edit->id=EDIT_NAME;
						edit->type=EditData::EDITDATA_TYPE_STRING;
						edit->helpobject=c;
						edit->string=c->GetName();

						maingui->EditDataValue(edit);
					}	
				}
				break;
			}

			return;
		}

		o=o->NextObject();
	}

}

bool Edit_AudioMix::CheckAuto()
{
	realtimefilter.DeleteAllO();
	CreateTrackFilter(&realtimefilter);

	Edit_AudioMixFilterChannel *cf=(Edit_AudioMixFilterChannel *)realtimefilter.GetRoot(),*ff=FirstFilterChannel();

	if(!cf)
	{
		if(ff && ff->track)
			goto refresh;

		goto exit;
	}

	for(;;)
	{
		if(cf && (!ff))
		{
			TRACE ("1\n");
			goto refresh;
		}

		if((!cf) && ff)
		{
			TRACE ("2\n");
			goto refresh;
		}

		if(cf && ff)
		{
			if(cf->track!=ff->track)
			{
				TRACE ("3\n");
				goto refresh;
			}
		}
		else 
			break;

		cf=cf->NextChannel();
		ff=ff->NextChannel();

		if(!cf)
		{
			if(ff && ff->track)
			{
				TRACE ("4\n");
				goto refresh;
			}

			break;
		}
	}

exit:
	realtimefilter.DeleteAllO();
	return false;

refresh:

	realtimefilter.DeleteAllO();
	ShowAll(true);

	return true;
}

void Edit_AudioMix::SetAutoMode(int tracking,int pattering)
{
	if(tracking!=mainsettings->autotracking[set] || mainsettings->autopattering[set]!=pattering)
	{
		mainsettings->autotracking[set]=tracking;
		mainsettings->autopattering[set]=pattering;

		ShowAll(true);
		ShowTracking();
		ShowPattering();
	}
}

void Edit_AudioMix::Gadget(guiGadget *g)
{
	g=CheckToolBox(g);
	if(!g)return;

	guiObject *o=guiobjects.FirstObject();
	while(o)
	{
		if(o->gadget==g)
		{
			switch(o->id)
			{
			case OBJECTID_SENDEDIT:
				{
					Edit_AudioMix_SendEdit *sed=(Edit_AudioMix_SendEdit *)o;
					guiGadget_Volume *vol=(guiGadget_Volume *)g;

					sed->send->sendvolume=vol->volume;
				}
				break;
			}

			return;
		}

		o=o->NextObject();
	}


	switch(g->gadgetID)
	{
	case GADGETID_SHOWWHAT:
		{
			if(DeletePopUpMenu(true))
			{
				class menu_selecttrackauto:public guiMenu
				{
				public:
					menu_selecttrackauto(Edit_AudioMix *mix,int i){editor=mix;index=i;}

					void MenuFunction()
					{
						editor->SetAutoMode(index,mainsettings->autopattering[editor->set]);
					} //

					Edit_AudioMix *editor;
					int index;
				};

				for(int i=0;i<AUTO_ENDOFFLAGS;i++)
				{
					int cxs;

					switch(i)
					{
					case 0:
						cxs=CXS_AUTOALL;
						break;

					case 1:
						cxs=CXS_AUTOSELECTED;
						break;

					case 2:
						cxs=CXS_AUTOFOCUS;
						break;

					case 3:
						cxs=CXS_AUTOAUDIOORMIDI;
						break;

					case 4:
						cxs=CXS_AUTOAUDIO;
						break;

					case 5:
						cxs=CXS_AUTOMIDI;
						break;

					case 6:
						cxs=CXS_AUTOINSTR;
						break;

					case 7:
						cxs=CXS_AUTOTRACKSRECORD;
						break;
					}

					popmenu->AddFMenu(Cxs[cxs],new menu_selecttrackauto(this,i),i==mainsettings->autotracking[set]?true:false);
				}

				ShowPopMenu();
			}
		}
		break;

	case GADGETID_PATTERN:
		{
			if(DeletePopUpMenu(true))
			{
				class menu_selectpatternauto:public guiMenu
				{
				public:
					menu_selectpatternauto(Edit_AudioMix *mix,int i){editor=mix;index=i;}

					void MenuFunction()
					{
						editor->SetAutoMode(mainsettings->autotracking[editor->set],index);
					} //

					Edit_AudioMix *editor;
					int index;
				};

				for(int i=0;i<3;i++)
					popmenu->AddFMenu(Cxs[CXS_NOPATTERNCHECK+i],new menu_selectpatternauto(this,i),i==mainsettings->autopattering[set]?true:false);

				ShowPopMenu();
			}
		}
		break;

	case GADGETID_EDITORSLIDER_HORIZ:
		{
			sliderxobjects.InitWithSlider(horzgadget);
			effectxobjects.InitWithSlider(horzgadget);
			ShowAll();
		}
		break;

	case GADGETID_EDITORSLIDER_VERT:
		effectobjects.InitWithSlider(effectvertgadget);
		DrawDBBlit(effects,mastereffects);
		break;

	case GADGETID_EDITORSLIDER_HORIZZOOM:
		mainsettings->mixerzoom=mixerzoom=g->GetPos();
		ShowAll();
		break;

	case GADGETID_SET1:
		SetSet(0);
		break;

	case GADGETID_SET2:
		SetSet(1);
		break;

	case GADGETID_SET3:
		SetSet(2);
		break;

	case GADGETID_SET4:
		SetSet(3);
		break;

	case GADGETID_SET5:
		SetSet(4);
		break;

	case GADGETID_PAN:
		{
			if(GetShowFlag()&SHOW_PAN)
				ClearFlag(SHOW_PAN);
			else
				AddFlag(SHOW_PAN);
		}
		break;

	case GADGETID_FX:
		{
			if(GetShowFlag()&SHOW_FX)
				ClearFlag(SHOW_FX);
			else
				AddFlag(SHOW_FX);
		}
		break;

	case GADGETID_INPUTFX:
		{
			if(GetShowFlag()&SHOW_INPUTFX)
				ClearFlag(SHOW_INPUTFX);
			else
				AddFlag(SHOW_INPUTFX);
		}
		break;

	case GADGETID_TRACKS:
		{
			if(GetShowFlag()&SHOW_TRACKS)
			{
				if(maingui->GetShiftKey()==true)
				{
					ClearFlag(SHOW_CHANNELS|SHOW_INSTRUMENTS|SHOW_BUS|/*SHOW_MASTER|*/SHOW_AUDIOINPUT|SHOW_AUDIOOUTPUT);
				}
				else
					ClearFlag(SHOW_TRACKS);
			}
			else
				AddFlag(SHOW_TRACKS);
		}
		break;

	case GADGETID_BUS:
		{
			if(GetShowFlag()&SHOW_BUS)
			{
				if(maingui->GetShiftKey()==true)
					ClearFlag(SHOW_TRACKS|SHOW_CHANNELS|SHOW_INSTRUMENTS|/*SHOW_MASTER|*/SHOW_AUDIOINPUT|SHOW_AUDIOOUTPUT);
				else
					ClearFlag(SHOW_BUS);
			}
			else
				AddFlag(SHOW_BUS|SHOW_AUDIO);
		}
		break;

	case GADGETID_AUDIOINPUT:
		{
			if(GetShowFlag()&SHOW_AUDIOINPUT)
			{
				if(maingui->GetShiftKey()==true)
					ClearFlag(SHOW_TRACKS|SHOW_CHANNELS|SHOW_INSTRUMENTS|/*SHOW_MASTER|*/SHOW_BUS|SHOW_AUDIOOUTPUT);
				else
					ClearFlag(SHOW_AUDIOINPUT);
			}
			else
				AddFlag(SHOW_AUDIOINPUT|SHOW_AUDIO);
		}
		break;

	case GADGETID_AUDIOOUTPUT:
		{
			if(GetShowFlag()&SHOW_AUDIOOUTPUT)
				ClearFlag(SHOW_AUDIOOUTPUT);
			else
				AddFlag(SHOW_AUDIOOUTPUT|SHOW_AUDIO);
		}
		break;

	case GADGETID_AUTO:
		{
			if(GetShowFlag()&SHOW_AUTO)
			{
				ClearFlag(SHOW_AUTO);
			}
			else
				AddFlag(SHOW_AUTO);
		}
		break;

	case GADGETID_AUDIO:
		{
			if(GetShowFlag()&SHOW_AUDIO)
			{
				ClearFlag(SHOW_AUDIO);
				AddFlag(SHOW_MIDI);
			}
			else
				AddFlag(SHOW_AUDIO);
		}
		break;

	case GADGETID_MIDI:
		{
			if(GetShowFlag()&SHOW_MIDI)
			{
				ClearFlag(SHOW_MIDI);
				AddFlag(SHOW_AUDIO);
			}
			else
				AddFlag(SHOW_MIDI);
		}
		break;

	case GADGETID_IO:
		if(GetShowFlag()&SHOW_IO)
			ClearFlag(SHOW_IO);
		else
			AddFlag(SHOW_IO);
		break;
	}
}

bool Edit_AudioMix::IsEffect(AudioEffects *fx)
{
	if(!fx)
		return false;

	if(fx->GetSong()!=WindowSong())
		return false;

	if(!(GetShowFlag()&SHOW_FX))
		return false;

	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster
		if(ot->id==OBJECTID_EFFECTS || ot->id==OBJECTID_MASTEREFFECTS)
		{
			Edit_AudioMixEffects *ame=(Edit_AudioMixEffects *)ot;

			if(ame->GetFX()==fx || ame->GetInputFX()==fx)
				return true;
		}

		ot=ot->NextObject();
	}

	return false;
}

void Edit_AudioMixEffects::FreeMemory()
{
	guiGadget *g=(guiGadget *)effectgadgets.GetRoot();
	while(g)
	{
		g->DeInit();
		g=g->NextGadget();
	}
	effectgadgets.DeleteAllO();

	guiGadget *eg=(guiGadget *)effecteditgadgets.GetRoot();
	while(eg)
	{
		eg->DeInit();
		eg=eg->NextGadget();
	}
	effecteditgadgets.DeleteAllO();

	guiGadget *sg=(guiGadget *)sendgadgets.GetRoot();
	while(sg)
	{
		sg->DeInit();
		sg=sg->NextGadget();
	}
	sendgadgets.DeleteAllO();

	guiGadget *seg=(guiGadget *)sendvolumegadgets.GetRoot();
	while(seg)
	{
		seg->DeInit();
		seg=seg->NextGadget();
	}
	sendvolumegadgets.DeleteAllO();

	if(eqgadget)
		eqgadget->Delete();

	if(typegadget)
		typegadget->Delete();

	if(emptyeffect)
		emptyeffect->Delete();

	if(emptysend)
		emptysend->Delete();

	if(abgadget)
		abgadget->Delete();

	//audiochannelouts.Delete();	
}

void Edit_AudioMix::ClearEMCEffects()
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		guiObject *not=ot->NextObject();

		// 1. Raster

		switch(ot->id)
		{
		case OBJECTID_EFFECTS:
			{
				Edit_AudioMixEffects *ea=(Edit_AudioMixEffects *)ot;
				ea->FreeMemory();
			}
			break;

		case OBJECTID_FX:
			{
				Edit_AudioMix_FX *fx=(Edit_AudioMix_FX *)ot;
				fx->FreeMemory();
			}
			break;

		case OBJECTID_SEND:
			{
				Edit_AudioMix_Send *send=(Edit_AudioMix_Send *)ot;
				send->FreeMemory();
			}
			break;
		}

		ot=not;

	}// while ot

	guiobjects.RemoveOsFromTo(OBJECTID_EFFECTS,LASTEFFECTID);
}

void Edit_AudioMix::ClearEMCMasterEffects()
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster

		switch(ot->id)
		{
		case OBJECTID_MASTEREFFECTS:
			{
				Edit_AudioMixEffects *ea=(Edit_AudioMixEffects *)ot;
				ea->FreeMemory();
			}
			break;

		case OBJECTID_MASTERFX:
			{
				Edit_AudioMix_FX *fx=(Edit_AudioMix_FX *)ot;
				fx->FreeMemory();
			}
			break;
		}

		ot=ot->NextObject();
	}// while ot

	guiobjects.RemoveOsFromTo(OBJECTID_MASTEREFFECTS,LASTMASTEREFFECTID);
}

void Edit_AudioMix::ClearEMCMasterSlider()
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster
		if(ot->id==OBJECTID_MASTERSLIDER)
		{
			Edit_AudioMixSlider *ea=(Edit_AudioMixSlider *)ot;
			ea->FreeMemory();
		}

		ot=ot->NextObject();
	}// while ot

	guiobjects.RemoveOsFromTo(OBJECTID_MASTERSLIDER,LASTERMASTERID);
}

void Edit_AudioMix::ClearEMCSlider()
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster
		if(ot->id==OBJECTID_SLIDER)
		{
			Edit_AudioMixSlider *ea=(Edit_AudioMixSlider *)ot;
			ea->FreeMemory();
		}

		ot=ot->NextObject();
	}// while ot

	guiobjects.RemoveOsFromTo(OBJECTID_SLIDER,LASTSLIDERID);
}

void Edit_AudioMix::ClearEMCs()
{
	ClearEMCEffects();
	ClearEMCSlider();

	ClearEMCMasterSlider();
	ClearEMCMasterEffects();
}

void Edit_AudioMix::FreeEffects()
{
	if(mastermode==true)
	{
		ClearEMCMasterSlider();
		ClearEMCMasterEffects();
		effectobjects.DeleteAllO(mastereffects);
	}
	else
	{
		ClearEMCEffects();
		effectxobjects.DeleteAllO(effects);
	}
}

void Edit_AudioMix::ShowMasterEffects()
{
	if(!mastereffects)
		return;

	guiBitmap *bitmap=&mastereffects->gbitmap;
	bitmap->guiFillRect(COLOUR_MASTERCHANNEL);


#ifdef OLDIE
	ClearEMCMasterEffects();
	effectobjects.DeleteAllO(mastereffects);


	BuildMinMaxEffects();

	effectobjects.StartBuild(mastereffects);

	if(Edit_AudioMixEffects *s=new Edit_AudioMixEffects(mastereffects,true))
	{
		s->isaudio=(GetShowFlag()&SHOW_AUDIO)?true:false;
		s->isMIDI=(GetShowFlag()&SHOW_MIDI)?true:false;
		s->editor=this;			
		s->ismaster=true;
		s->filterchannel=&masterfilterchannel;

		masterfilterchannel.channel=&WindowSong()->audiosystem.masterchannel;
		masterfilterchannel.track=0;

		guiobjects.AddGUIObject(0,0 -effectobjects.starty,mastereffects->GetX2(),mastereffects->GetY2(),mastereffects,s);

		s->ShowEffects();
	}
#endif
}


void Edit_AudioMix::BuildMinMaxEffects()
{
	maxeffects=maxinputeffects=0;
	maxsends=0;

	{
		Edit_AudioMixFilterChannel *c=FirstFilterChannel();

		while(c)
		{
			if(c->GetFX()->GetCountEffects()>maxeffects)
				maxeffects=c->GetFX()->GetCountEffects();

			if(c->GetIO()->GetCountSends()>maxsends)
				maxsends=c->GetIO()->GetCountSends();

			if(c->GetIO()->audioinputeffects.GetCountEffects()>maxinputeffects)
				maxinputeffects=c->GetIO()->audioinputeffects.GetCountEffects();

			c=c->NextChannel();
		}
	}

	if(solotrack==true)
		return;

	// MasterChannel
	{
		AudioIOFX *io=&WindowSong()->audiosystem.masterchannel.io;
		AudioEffects *fx=&io->audioeffects;

		if(fx->GetCountEffects()>maxeffects)
			maxeffects=fx->GetCountEffects();

		if(io->GetCountSends()>maxsends)
			maxsends=io->GetCountSends();

		if(io->audioinputeffects.GetCountEffects()>maxinputeffects)
			maxinputeffects=io->audioinputeffects.GetCountEffects();
	}
}

void Edit_AudioMix::ShowEffects()
{
	FreeEffects();

	effects->gbitmap.guiFillRect(COLOUR_UNUSED);

	BuildMinMaxEffects();

	CreateMixerObjects(effects->formchild,&effectxobjects,false); // Effect<>Slider

	effectxobjects.EndBuild();
	effectxobjects.InitXStartO();
	effectxobjects.OptimizeXStart();
	effectobjects.StartBuild(effects);

	while(effectxobjects.GetShowObject() && effectxobjects.GetInitX()<effects->GetWidth())
	{
		Edit_AudioMixFilterChannel *fc=(Edit_AudioMixFilterChannel *)effectxobjects.GetShowObject()->object;

		if(Edit_AudioMixEffects *amixc=new Edit_AudioMixEffects(effects,false))
		{
			amixc->isaudio=fc->isaudio;
			amixc->isMIDI=fc->isMIDI;
			amixc->editor=this;
			amixc->filterchannel=fc;

			guiobjects.AddGUIObject(effectxobjects.GetInitX(),-effectobjects.starty,effectxobjects.GetInitX2(),effects->GetHeight(),effects,amixc);

			amixc->ShowEffects();
		}

		effectxobjects.NextXO();
	}

	effectobjects.EndBuild(effectvertgadget);
}


void MixerSlider_Callback(guiGadget_CW *g,int status)
{
	Edit_AudioMix *mix=(Edit_AudioMix *)g->from;
	bool ismaster=mix->masterslider==g?true:false;

	if(mix->mastermode==true)
	{
		ismaster=true;
		mix->masterslider=g;
	}
	else if(mix->solotrack==true)
	{
		mix->masterslider=0;
	}

	switch(status)
	{
	case DB_CREATE:
		{
			if(!mix->slider)
			{
				mix->showoverviewonpaint=true;
				mix->slider=g;
			}
			else
				mix->masterslider=g;
		}
		break;

	case DB_PAINT:
		{
			if(ismaster==true)
				mix->ShowFullMasterSlider();
			else
			{
				mix->ShowFullSlider();

				if(mix->showoverviewonpaint==true)
				{
					mix->showoverviewonpaint=false;
					mix->DrawDBBlit(mix->overview);
				}
			}
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		mix->MouseMoveInSlider(g);
		break;

	case DB_RIGHTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		mix->MouseClickInSlider(g,status==DB_LEFTMOUSEDOWN?true:false);	
		break;

	case DB_LEFTMOUSEUP:
		mix->MouseReleaseInSlider(g,true);
		break;

	case DB_DOUBLECLICKLEFT:
		mix->MouseDoubleClickInSlider(g,true);	
		break;

		//case DB_DELTA:
		//	mix->DeltaInSlider(mix->slider);
		//	break;
	}
}

void MixerEffects_Callback(guiGadget_CW *g,int status)
{
	Edit_AudioMix *mix=(Edit_AudioMix *)g->from;
	bool ismaster=mix->mastereffects==g?true:false;

	if(mix->mastermode==true)
	{
		ismaster=true;
		mix->mastereffects=g;
	}
	else if(mix->solotrack==true)
	{
		mix->mastereffects=0;
	}

	switch(status)
	{
	case DB_CREATE:
		{
			if(!mix->effects)
				mix->effects=g;
			else
				mix->mastereffects=g;
		}
		break;

	case DB_FREEOBJECTS:
		{
			if(ismaster==true)
				mix->ClearEMCMasterEffects();
			else
				mix->FreeEffects();
		}
		break;

	case DB_PAINT:
		{	
#ifdef DEBUG
			if(g->formchild->enable==false)
				maingui->MessageBoxError(0,"MixerEffects_Callback");
#endif

			if(ismaster==true)
				mix->ShowMasterEffects();
			else
				mix->ShowEffects();
		}
		break;

	case DB_DOUBLECLICKLEFT:
		mix->MouseDoubleClickInEffects(g,true);	
		break;

	case DB_LEFTMOUSEDOWN:
		mix->MouseClickInEffects(g,true);	
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		mix->MouseMoveInEffects(g);
		break;

	case DB_RIGHTMOUSEDOWN:
		mix->MouseClickInEffects(g,false);	
		break;

	case DB_LEFTMOUSEUP:
		mix->MouseReleaseInEffects(g,true);
		break;

	case DB_DELTA:
		//	mix->DeltaInEffects();
		break;
	}
}


void Edit_AudioMix::DeltaInSlider(guiGadget_CW *slider)
{
	slider->deltareturn=false;

	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		if(ot->id==OBJECTID_PAN)
		{
			OSTART atime=GetAutomationTime();

			Edit_AudioMix_Pan *c=(Edit_AudioMix_Pan *)ot;

			if(c->clicked==true)
			{
				slider->deltareturn=c->slider->cspan.EditPan(slider->deltay,atime);
				return;
			}
		}

		if(ot->id==OBJECTID_SLIDER || ot->id==OBJECTID_MASTERSLIDER)
		{
			// Volume 
			Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)ot;

			bool m_valueclicked=c->GetTrackHead()->m_volumeclicked;

			if(m_valueclicked==true)
			{
				int deltay=slider->deltay;

				deltay=-deltay;

				TrackHead *th=c->GetTrackHead();

				int p=th->MIDIfx.velocity.GetVelocity();

				if(p>-10 && p<10)
				{
					if(deltay>-8 && deltay<8)
						return;

					if(deltay<-1)deltay=-1;
					else
						if(deltay>1)deltay=1;
				}
				else
					if(p>-15 && p<+15 )
					{
						if(deltay>-5 && deltay<5)
							return;

						if(deltay<-2)deltay=-2;
						else
							if(deltay>2)deltay=2;
					}
					else
					{
						if(p>-30 && p<+30)
						{
							if(deltay<-3)deltay=-3;
							else
								if(deltay>3)deltay=3;
						}
					}

					slider->deltareturn=true;

					OSTART atime=GetAutomationTime();

					if(c->GetChannel())
					{
						p-=deltay;

						c->GetChannel()->SetMIDIVelocity(p,atime);
					}
					else
					{
						// Tracks
						Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

						while(flt)
						{
							if(flt->track && c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio)
							{
								// MIDI Velocity +127 - -127

								if( 
									flt->isMIDI==true && 
									(
									flt->GetFX()==c->GetFX() || 
									(c->GetTrack() && flt->track && maingui->GetCtrlKey()==false && c->GetTrack()->IsSelected()==true && flt->track->IsSelected()==true)
									//(c->GetChannel() && flt->channel && c->GetChannel()->audiochannelsystemtype==flt->channel->audiochannelsystemtype && maingui->GetCtrlKey()==false && c->GetChannel()->IsSelected()==true && flt->channel->IsSelected()==true)
									)

									)
								{
									int p=flt->track->MIDIfx.velocity.GetVelocity();

									p-=deltay;

									if(p>=-127 && p<=127)
									{
										// -127 <-> +127
										double h=p;
										h+=127;

										h/=254;

										flt->track->MIDIfx.velocity.AutomationEdit(WindowSong(),atime,0,h);
									}
								}
							}

							flt=flt->NextChannel();
						}
					}

					return;
			}

			bool valueclicked=c->GetTrackHead()->volumeclicked;

			if(valueclicked==true)
			{
				int deltay=slider->deltay;

				double p=c->GetFX()->volume.GetParm(0);
				p*=LOGVOLUME_SIZE;

				// AUDIO Volume
				if(p>AUDIOMIXER_ADD-10 && p<AUDIOMIXER_ADD+10)
				{
					if(deltay>-8 && deltay<8)
						return;

					if(deltay<-1)deltay=-1;
					else
						if(deltay>1)deltay=1;
				}
				else
					if(p>AUDIOMIXER_ADD-15 && p<AUDIOMIXER_ADD+15 )
					{
						if(deltay>-5 && deltay<5)
							return;

						if(deltay<-2)deltay=-2;
						else
							if(deltay>2)deltay=2;
					}
					else
					{
						if(p>AUDIOMIXER_ADD-30 && p<AUDIOMIXER_ADD+30 )
						{
							if(deltay<-3)deltay=-3;
							else
								if(deltay>3)deltay=3;
						}
					}

					slider->deltareturn=true;

					OSTART atime=GetAutomationTime();

					if(c->ismaster==true)
					{
						// Master Volume
						double p=c->GetFX()->volume.GetParm(0);
						p*=LOGVOLUME_SIZE;

						p+=deltay;

						if(p<0)
							p=0;
						else
							if(p>LOGVOLUME_SIZE)
								p=LOGVOLUME_SIZE;

						double h2=p;
						h2/=LOGVOLUME_SIZE;

						c->GetFX()->volume.AutomationEdit(WindowSong(),atime,0,h2);
					}
					else
					{
						Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

						while(flt)
						{
							if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio)
							{
								if( 
									flt->GetFX()==c->GetFX() || 
									(c->GetTrack() && flt->track && maingui->GetCtrlKey()==false && c->GetTrack()->IsSelected()==true && flt->track->IsSelected()==true) ||
									(c->GetChannel() && flt->channel && c->GetChannel()->audiochannelsystemtype==flt->channel->audiochannelsystemtype && maingui->GetCtrlKey()==false && c->GetChannel()->IsSelected()==true && flt->channel->IsSelected()==true)
									)
								{
									double p=flt->GetFX()->volume.GetParm(0);
									p*=LOGVOLUME_SIZE;

									p+=deltay;

									if(p<0)
										p=0;
									else
										if(p>LOGVOLUME_SIZE)
											p=LOGVOLUME_SIZE;

									double h2=p;
									h2/=LOGVOLUME_SIZE;

									flt->GetFX()->volume.AutomationEdit(WindowSong(),atime,0,h2);
								}
							}

							flt=flt->NextChannel();
						}
					}

					return;
			}
		}

		ot=ot->NextObject();
	}
}

void Edit_AudioMix::MouseClickInOverview(bool leftmouse)
{
	if((!slider) || nooverview==true)return;

	double h2=overviewx2;
	if(h2<=0)return;

	double mx=overview->GetMouseX();

	mx/=h2;

	if(mx>1)
		mx=1;

	if(sliderxobjects.ZoomX(mx)==true)
	{
		effectxobjects.ZoomX(mx);
		ShowAll();	
	}
}

void Edit_AudioMix::MouseDoubleClickInSlider(guiGadget_CW *slider,bool leftmouse)
{
	guiObject *o=slider->CheckObjectClicked(); // Object Under Mouse ?

	if(!o)
		return;

	int mx=slider->GetMouseX();
	int my=slider->GetMouseY();

	switch(o->id)
	{
	case OBJECTID_SLIDER:
	case OBJECTID_MASTERSLIDER:
		if(leftmouse==true)
		{
			Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)o;

			// MIDI
			if(c->m_valueondisplay==true && 
				maingui->CheckIfInRange(mx,my,c->m_valuex,c->m_valuex2,c->m_valuey,c->m_valuey2)==true
				)
			{
				if(c->GetTrackHead()->CanAutomationObjectBeChanged(&c->GetTrackHead()->MIDIfx.velocity,0,0)==true)
				{
					if(EditData *edit=new EditData)
					{
						// long position;
						edit->song=0;
						edit->win=this;

						edit->x=c->m_valuex2;
						edit->y=GetWindowMouseY();
						edit->width=c->m_valuex2-c->m_valuex;

						edit->title=Cxs[CXS_EDIT];
						edit->deletename=false;
						edit->id=EDIT_SLIDERMIDIVOLUME;
						edit->type=EditData::EDITDATA_TYPE_INTEGER;
						edit->helpobject=c;

						edit->from=-127;
						edit->to=127;
						edit->value=c->GetChannel()?c->GetChannel()->MIDIfx.GetVelocity():c->GetTrack()->GetFX()->GetVelocity_NoParent();

						maingui->EditDataValue(edit);
					}
				}
				return;
			}

			if(c->valueondisplay==true && maingui->CheckIfInRange(mx,my,c->valuex,c->valuex2,c->valuey,c->valuey2)==true)
			{
				if(c->GetTrackHead()->CanAutomationObjectBeChanged(&c->GetFX()->volume,0,0)==true)
				{
					if(EditData *edit=new EditData)
					{
						// long position;
						edit->song=0;
						edit->win=this;

						edit->x=c->valuex2;
						edit->y=GetWindowMouseY();
						edit->width=c->valuex2-c->valuex;

						edit->title=Cxs[CXS_EDIT];
						edit->deletename=false;
						edit->id=EDIT_SLIDERAUDIOVOLUME;
						edit->type=EditData::EDITDATA_TYPE_DOUBLE;
						edit->doubledigits=1;
						edit->helpobject=c;

						edit->dfrom=mainaudio->ConvertFactorToDb(mainaudio->silencefactor);
						edit->dto=AUDIO_MAXDB;
						edit->dvalue=c->GetFX()->volume.GetDB();

						maingui->EditDataValue(edit);
					}
				}
				return;
			}

		}
		break;
	}
}

void Edit_AudioMix::MouseMoveInSlider(guiGadget_CW *slider)
{
	slider->InitDelta();

	if(slider->deltay!=0)
		DeltaInSlider(slider);

	slider->ExitDelta();
}

void Edit_AudioMix::MouseClickInSlider(guiGadget_CW *slider,bool leftmouse)
{
	guiObject *o=slider->CheckObjectClicked(); // Object Under Mouse ?

	if(!o)
	{
		ClickInEmptyArea(leftmouse);
		return;
	}

	int mx=slider->GetMouseX(),my=slider->GetMouseY();

	switch(o->id)
	{	
		// 1. Raster
	case OBJECTID_SLIDER:
	case OBJECTID_MASTERSLIDER:
		if(leftmouse==true)
		{
			Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)o;

			if(c->isMIDI==true && c->m_valueondisplay==true)
			{
				if(maingui->CheckIfInRange(mx,my,c->m_valuex,c->m_valuex2,c->m_vuy,c->m_vuy2)==true || 
					(solotrack==true && maingui->CheckIfInRange(mx,my,c->m_vux,c->m_vux2,c->m_vuy,c->m_vuy2)==true) // ||
					//	(c->GetChannel() && c->GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER)
					)
				{
					// MIDI
					c->GetTrackHead()->MIDIVolumeDown();

					if(c->GetTrackHead()->m_volumeclicked==true)
					{
						slider->InitGetMouseMove();
						c->ShowMIDIVolumeSlider();
						c->Blt();
					}

					return;
				}
			}

			if(c->isaudio==true && c->valueondisplay==true)
			{
				// Audio
				if(maingui->CheckIfInRange(mx,my,c->valuex,c->valuex2,c->vuy,c->vuy2)==true || 
					(solotrack==true && maingui->CheckIfInRange(mx,my,c->vux,c->vux2,c->vuy,c->vuy2)==true) //||
					//(c->GetChannel() && c->GetChannel()->audiochannelsystemtype==CHANNELTYPE_MASTER)
					)
				{
					c->GetTrackHead()->VolumeDown();

					if(c->GetTrackHead()->volumeclicked==true)
					{
						slider->InitGetMouseMove();
						c->ShowVolumeSlider();
						c->Blt();
					}

					return;
				}
			}

			if(c->GetTrack())
			{
				WindowSong()->SetFocusTrack(c->GetTrack());
				return;
			}

			if(c->GetChannel())
			{
				WindowSong()->audiosystem.SetFocusBus(c->GetChannel(),true);
			}

			return;
		}

		// Right Mouse
		{
			Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)o;

			OSTART atime=GetAutomationTime();

			if(c->m_valueondisplay==true && 
				maingui->CheckIfInRange(mx,my,c->m_valuex,c->m_valuex2,c->m_valuey,c->m_valuey2)==true &&
				c->GetTrackHead()->CanAutomationObjectBeChanged(&c->GetTrackHead()->MIDIfx.velocity,0,0)==true
				)
			{
				// MIDI Reset
				if(c->GetChannel())
				{
					c->GetChannel()->SetMIDIVelocity(0,atime);
				}
				else
				{
					Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

					while(flt)
					{
						if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio)
						{
							// MIDI Velocity +127 - -127
							if(flt->GetFX()==c->GetFX() || (flt->isMIDI==true && c->GetTrack() && flt->track && (maingui->GetCtrlKey()==false && c->GetTrack()->IsSelected()==true && flt->track->IsSelected()==true)))
							{
								flt->track->t_trackeffects.SetVelocity(0,atime);
							}
						}

						flt=flt->NextChannel();
					}
				}

				return;
			}

			if(c->valueondisplay==true && 
				maingui->CheckIfInRange(mx,my,c->valuex,c->valuex2,c->valuey,c->valuey2)==true && 
				c->GetTrackHead()->CanAutomationObjectBeChanged(&c->GetFX()->volume,0,0)==true
				)
			{
				// Audio Reset
				if(c->ismaster==true)
				{
					c->GetFX()->volume.AutomationEdit(WindowSong(),atime,0,0.5);
				}
				else
				{
					Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

					while(flt)
					{
						if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio)
						{
							if(flt->GetFX()==c->GetFX() || 
								(c->GetTrack() && flt->track && ((flt->GetFX()==c->GetFX() || maingui->GetCtrlKey()==false)&& c->GetTrack()->IsSelected()==true && flt->track->IsSelected()==true)))
							{
								flt->GetFX()->volume.AutomationEdit(WindowSong(),atime,0,0.5);
							}
						}

						flt=flt->NextChannel();
					}
				}

				return;
			}

			if(maingui->GetLeftMouseButton()==false)
			{
				if(c->GetTrack())
					PopMenuTrack(c->GetTrack());
			}
		}
		break;

	case OBJECTID_MASTERAB:
		{
			WindowSong()->audiosystem.SetSystemByPass(WindowSong()->audiosystem.systembypassfx==true?false:true);
		}
		break;

	case OBJECTID_CHILDOPEN:
		{
			Edit_AudioMix_ChildOpen *child=(Edit_AudioMix_ChildOpen *)o;
			Edit_AudioMixSlider *c=child->slider;

			c->GetTrack()->ToggleShowChild(leftmouse);
			return;
		}
		break;

	case OBJECTID_MUTE:
	case OBJECTID_MASTERMUTE:
		{
			if(leftmouse==true)
			{
				Edit_AudioMix_Mute *m=(Edit_AudioMix_Mute *)o;
				Edit_AudioMixSlider *c=m->slider;
				OSTART automationtime=GetAutomationTime();

				if(c->GetChannel())
				{
					c->GetChannel()->io.audioeffects.Mute(c->GetAudioIO()->audioeffects.GetMute()==true?false:true,WindowSong()==mainvar->GetActiveSong()?true:false);
				}
				else
					if(c->GetTrack())
					{
						bool mute=c->GetTrack()->GetMute()==true?false:true;
						Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

						while(flt)
						{
							if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio && flt->track)
							{
								if(flt->track->IsPartOfEditing(c->GetTrack())==true)
									WindowSong()->MuteTrack(flt->track,mute,automationtime);
							}

							flt=flt->NextChannel();
						}	
					}
			}
		}
		break;

	case OBJECTID_SOLO:
		if(leftmouse==true)
		{
			Edit_AudioMix_Solo *m=(Edit_AudioMix_Solo *)o;
			Edit_AudioMixSlider *c=m->slider;
			OSTART automationtime=GetAutomationTime();

			if(c->GetChannel())
				c->GetChannel()->SetSolo(c->GetChannel()->io.audioeffects.GetSolo()==true?false:true,WindowSong()==mainvar->GetActiveSong()?true:false);
			else
				if(c->GetTrack())
					WindowSong()->SoloTrack(c->GetTrack(),c->GetTrack()->GetSolo()==true?false:true,automationtime);
		}
		break;

	case OBJECTID_RECORD:
		if(leftmouse==true)
		{
			Edit_AudioMix_Record *m=(Edit_AudioMix_Record *)o;
			Edit_AudioMixSlider *c=m->slider;

			if(c->GetTrack())
			{
				if(solotrack==true)
				{
					c->GetTrack()->ToggleRecord();
				}
				else
				{
					bool record=c->GetTrack()->record==true?false:true;
					OSTART pos=WindowSong()->GetSongPosition();

					Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

					while(flt)
					{
						if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio && flt->track)
						{
							if(flt->track->IsPartOfEditing(c->GetTrack(),true)==true)
								flt->track->SetRecordMode(record,pos);
						}

						flt=flt->NextChannel();
					}	

					WindowSong()->SetMIDIRecordingFlag();
				}
			}
		}
		else
		{
			Edit_AudioMix_Record *m=(Edit_AudioMix_Record *)o;
			maingui->EditTrackRecordType(this,m->slider->GetTrack());
		}
		break;

	case OBJECTID_INPUTMONITORING:
		if(leftmouse==true)
		{
			Edit_AudioMix_InputMonitor *m=(Edit_AudioMix_InputMonitor *)o;
			Edit_AudioMixSlider *c=m->slider;

			if(c->GetTrack())
			{
				bool im=c->GetTrack()->io.inputmonitoring==true?false:true;

				Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

				while(flt)
				{
					if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio && flt->track)
					{
						if(flt->track->IsPartOfEditing(c->GetTrack(),true)==true)
							flt->track->SetInputMonitoring(im);
					}

					flt=flt->NextChannel();
				}	
			}
		}
		else
		{
			Edit_AudioMix_InputMonitor *m=(Edit_AudioMix_InputMonitor *)o;
			maingui->EditTrackRecordType(this,m->slider->GetTrack());
		}
		break;

	case OBJECTID_INPUTTYPE:
		{
			Edit_AudioMix_InputType *t=(Edit_AudioMix_InputType *)o;
			maingui->EditTrackRecordType(this,t->slider->GetTrack());
		}
		break;

	case OBJECTID_IOMIDIINPUT:
		{
			Edit_AudioMix_MIDIInput *in=(Edit_AudioMix_MIDIInput *)o;
			CreateInputPopMenu(in->slider,true);
		}
		break;

	case OBJECTID_IOINPUT:
		{
			Edit_AudioMix_Input *in=(Edit_AudioMix_Input *)o;
			CreateInputPopMenu(in->slider,false);
		}
		break;

	case OBJECTID_IOMIDIOUTPUT:
		{
			Edit_AudioMix_MIDIOutput *out=(Edit_AudioMix_MIDIOutput *)o;
			CreateOutputPopMenu(out->slider,true);
		}
		break;

	case OBJECTID_IOOUTPUT:
	case OBJECTID_MASTERIOOUTPUT:
		{
			Edit_AudioMix_Output *out=(Edit_AudioMix_Output *)o;
			CreateOutputPopMenu(out->slider,false);
		}
		break;

	case OBJECTID_TYPE:
		//case OBJECTID_MASTERTYPE:
		{
			Edit_AudioMix_ChannelType *ct=(Edit_AudioMix_ChannelType *)o;
			Edit_AudioMixSlider *c=ct->slider;

			CreateChannelTypePopMenu(c);
		}
		break;

	case OBJECTID_MIDITHRU:
		if(leftmouse==true)
		{
			Edit_AudioMix_Thru *m=(Edit_AudioMix_Thru *)o;
			Edit_AudioMixSlider *c=m->slider;

			if(c->GetTrack())
			{
				bool thru=c->GetTrack()->t_trackeffects.MIDIthru==true?false:true;
				Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

				while(flt)
				{
					if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio && flt->track)
					{
						if(flt->track->IsPartOfEditing(c->GetTrack())==true)
							flt->track->t_trackeffects.MIDIthru=thru;
					}

					flt=flt->NextChannel();
				}	
			}
		}
		break;

	case OBJECTID_THRU:
		if(leftmouse==true)
		{
			Edit_AudioMix_Thru *m=(Edit_AudioMix_Thru *)o;
			Edit_AudioMixSlider *c=m->slider;

			if(c->GetTrack())
			{
				bool thru=c->GetAudioIO()->thru==true?false:true;
				Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

				while(flt)
				{
					if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio && flt->track)
					{
						if(flt->track->IsPartOfEditing(c->GetTrack())==true)
							flt->GetIO()->thru=thru;
					}

					flt=flt->NextChannel();
				}	
			}
		}
		break;

	case OBJECTID_NAME:
	case OBJECTID_MASTERNAME:
		{
			Edit_AudioMix_Name *m=(Edit_AudioMix_Name *)o;
			Edit_AudioMixSlider *c=m->slider;

			if(c->GetTrack())
				WindowSong()->SetFocusTrack(c->GetTrack());
		}
		break;

	case OBJECTID_PAN:
		if(leftmouse==true)
		{
			Edit_AudioMix_Pan *p=(Edit_AudioMix_Pan *)o;
			Edit_AudioMixSlider *c=p->slider;

			slider->InitGetMouseMove();
			p->clicked=true;
			c->ShowPan(true,true);
			p->Blt();	
		}
		else
		{
			Edit_AudioMix_Pan *p=(Edit_AudioMix_Pan *)o;
			Edit_AudioMixSlider *c=p->slider;

			if(c->GetAudioIO()->audioeffects.pan.GetParm(0)!=0.5)
			{
				OSTART atime=GetAutomationTime();

				c->GetAudioIO()->audioeffects.pan.AutomationEdit(WindowSong(),atime,0,0.5);
				c->ShowPan(true,false);
				p->Blt();
			}
		}
		break;
	}
}

void Edit_AudioMix::MouseReleaseInEffects(guiGadget_CW *effects,bool leftmouse)
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster
		if(ot->id==OBJECTID_SENDEDIT)
		{
			Edit_AudioMix_SendEdit *sed=(Edit_AudioMix_SendEdit *)ot;

			if(sed->gadget_DB->getMouseMove==true)
			{
				sed->gadget_DB->Call(DB_LEFTMOUSEUP);
				return;
			}

		}

		ot=ot->NextObject();
	}
}


void Edit_AudioMix::MouseDoubleClickInEffects(guiGadget_CW *effects,bool leftmouse)
{
	guiObject *o=effects->CheckObjectClicked(); // Object Under Mouse ?

	if(!o)
		return;

	switch(o->id)
	{
		/*
		case OBJECTID_SENDEDIT:
		if(leftmouse==true)
		{
		Edit_AudioMix_SendEdit *sed=(Edit_AudioMix_SendEdit *)o;

		if(EditData *edit=new EditData)
		{
		// long position;
		edit->song=0;
		edit->win=this;

		edit->x=sed->x2;
		edit->y=GetMouseY();
		edit->width=sed->x2-sed->x;

		edit->name=Cxs[CXS_EDIT];
		edit->deletename=false;
		edit->id=EDIT_AUDIOSENDVOLUME;

		edit->type=EditData::EDITDATA_TYPE_DOUBLE;
		edit->doubledigits=1;
		edit->helpobject=sed;

		edit->dfrom=mainaudio->ConvertFactorToDb(mainaudio->silencefactor);
		edit->dto=AUDIO_MAXDB;
		edit->dvalue=mainaudio->ConvertFactorToDb(sed->send->sendvolume);

		maingui->EditDataValue(edit);
		}
		}
		break;
		*/
	}
}


void Edit_AudioMix::DeltaInEffects(guiGadget_CW *effects)
{
	effects->deltareturn=false;

	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster
		if(ot->id==OBJECTID_SENDEDIT)
		{
			Edit_AudioMix_SendEdit *se=(Edit_AudioMix_SendEdit *)ot;

			if(se->gadget_DB->getMouseMove==true)
			{
				se->gadget_DB->Call(DB_MOUSEMOVE|DB_LEFTMOUSEDOWN);
				effects->deltareturn=true;

				return;
			}
		}

		ot=ot->NextObject();
	}
}

void Edit_AudioMix::MouseMoveInEffects(guiGadget_CW *effects)
{
	effects->InitDelta();

	if(effects->deltay!=0)
		DeltaInEffects(effects);

	effects->ExitDelta();
}

void Edit_AudioMix::ClickInEmptyArea(bool leftmouse)
{
	if(leftmouse==true)
	{
		WindowSong()->SetFocusTrack(WindowSong()->GetFocusTrack());
	}
}

void Edit_AudioMix::MouseClickInEffects(guiGadget_CW *effects,bool leftmouse)
{
	guiObject *o=effects->CheckObjectClicked(); // Object Under Mouse ?

	if(leftmouse==true)
	{	
		if(!o)
		{
			ClickInEmptyArea(leftmouse);
			return;
		}

		switch(o->id)
		{	
			// 1. Raster
		case OBJECTID_EFFECTS:
			{
				Edit_AudioMixEffects *c=(Edit_AudioMixEffects *)o;

				if(c->GetTrack())
				{
					WindowSong()->SetFocusTrack(c->GetTrack());
					return;
				}

				if(c->GetChannel())
				{
					WindowSong()->audiosystem.SetFocusBus(c->GetChannel(),true);
					return;
				}
			}
			break;

		case OBJECTID_FX:
		case OBJECTID_MASTERFX:
			{
				Edit_AudioMix_FX *fx=(Edit_AudioMix_FX *)o;
				maingui->CreateEffectPopUp(fx->iae->effectlist,this,fx->iae);
			}
			break;

		case OBJECTID_FXEDIT:
		case OBJECTID_MASTERFXEDIT:
			{
				Edit_AudioMix_FXEDIT *fx=(Edit_AudioMix_FXEDIT *)o;

				guiWindow *w=maingui->FirstWindow(),*open=0;

				while(w && open==0){
					open=fx->iae->audioeffect->CheckIfWindowIsEditor(w);
					w=w->NextWindow();
				}

				if(open)
					open->WindowToFront(true);
				else
				{
					guiWindowSetting setting;
					setting.startposition_x=GetScreenMouseX();
					setting.startposition_y=GetScreenMouseY();
					fx->iae->audioeffect->OpenGUI(WindowSong(),fx->iae,&setting);
				}
			}
			break;

		case OBJECTID_EMPTYFX:
		case OBJECTID_MASTEREMPTYFX:
			{
				Edit_AudioMix_EmptyFX *fx=(Edit_AudioMix_EmptyFX *)o;

				//TRACE ("OBJECTID_EMPTYFX \n");
				if(fx->isMIDI==true)
				{
				}
				else
					maingui->CreateEffectPopUp(&fx->effects->GetAudioIO()->audioeffects,this,0);
			}
			break;

		case OBJECTID_ABFX:
		case OBJECTID_MASTERABFX:
			{
				Edit_AudioMix_ABFX *fx=(Edit_AudioMix_ABFX *)o;
				bool bypass=fx->effects->GetAudioIO()->bypassallfx==true?false:true;

				if(fx->isMIDI==true)
				{
				}
				else
				{
					if(fx->ismaster==true)
					{
						fx->effects->GetAudioIO()->SetFXBypass(bypass);
					}
					else
						if(maingui->GetCtrlKey()==false && fx->effects->GetTrack() && fx->effects->GetTrack()->IsSelected()==true)
						{
							Edit_AudioMixFilterChannel *f=FirstFilterChannel();

							while(f)
							{
								if(f->track && f->track->IsSelected()==true)
								{
									f->GetIO()->SetFXBypass(bypass);
								}

								f=f->NextChannel();
							}
						}
						else
							fx->effects->GetAudioIO()->SetFXBypass(bypass);
				}
			}
			break;

		case OBJECTID_EMPTYINPUTFX:
			{
				Edit_AudioMix_EmptyInputFX *fx=(Edit_AudioMix_EmptyInputFX *)o;

				//TRACE ("OBJECTID_EMPTYFX \n");
				maingui->CreateEffectPopUp(&fx->effects->GetAudioIO()->audioinputeffects,this,0);
			}
			break;

		case OBJECTID_ABFXINPUT:
			{
				Edit_AudioMix_ABINPUTFX *fx=(Edit_AudioMix_ABINPUTFX *)o;
				bool bypass=fx->effects->GetAudioIO()->bypassallinputfx==true?false:true;

				if(maingui->GetCtrlKey()==false && fx->effects->GetTrack() && fx->effects->GetTrack()->IsSelected()==true)
				{
					Edit_AudioMixFilterChannel *f=FirstFilterChannel();

					while(f)
					{
						if(f->track && f->track->IsSelected()==true)
						{
							f->GetIO()->SetInputFXBypass(bypass);
						}

						f=f->NextChannel();
					}
				}
				else
					fx->effects->GetAudioIO()->SetInputFXBypass(bypass);
			}
			break;

		case OBJECTID_SEND:
			{
				Edit_AudioMix_Send *se=(Edit_AudioMix_Send *)o;
				mainaudio->CreateSendPopMenu(this,se->effects->GetAudioIO(),se->send);
			}
			break;

		case OBJECTID_SENDEDIT:
			{
				Edit_AudioMix_SendEdit *se=(Edit_AudioMix_SendEdit *)o;
				se->gadget_DB->Call(DB_LEFTMOUSEDOWN);
			}
			break;

		case OBJECTID_EMPTYSEND:
			{
				Edit_AudioMix_EmptySend *es=(Edit_AudioMix_EmptySend *)o;
				mainaudio->CreateSendPopMenu(this,es->effects->GetAudioIO(),0);
			}
			break;
		}
	}
	else // right mouse
	{
		if(!o)
		{
			//
			return;
		}

		AudioEffects *afx=0;

		switch(o->id)
		{
			// 1. Raster
		case OBJECTID_EFFECTS:
			{
				Edit_AudioMixEffects *c=(Edit_AudioMixEffects *)o;

				if(c->GetTrack())
					PopMenuTrack(c->GetTrack());
			}
			break;

		case OBJECTID_FX:
			{
				Edit_AudioMix_FX *fx=(Edit_AudioMix_FX *)o;
				afx=fx->iae->effectlist;
			}
			break;

		case OBJECTID_FXEDIT:
			{
				Edit_AudioMix_FXEDIT *fx=(Edit_AudioMix_FXEDIT *)o;
				afx=fx->iae->effectlist;
			}
			break;

		case OBJECTID_EMPTYFX:
			{
				Edit_AudioMix_EmptyFX *fx=(Edit_AudioMix_EmptyFX *)o;
				afx=fx->effects->GetFX();
			}
			break;

		case OBJECTID_ABFX:
			{
				Edit_AudioMix_ABFX *fx=(Edit_AudioMix_ABFX *)o;
				afx=fx->effects->GetFX();
			}
			break;

		case OBJECTID_EMPTYINPUTFX:
			{
				Edit_AudioMix_EmptyInputFX *fx=(Edit_AudioMix_EmptyInputFX *)o;
				afx=fx->effects->GetInputFX();
			}
			break;

		case OBJECTID_ABFXINPUT:
			{
				Edit_AudioMix_ABINPUTFX *fx=(Edit_AudioMix_ABINPUTFX *)o;
				afx=fx->effects->GetInputFX();
			}
			break;
		}

		if(afx)
		{
			if(afx->IsInput()==false)
			{
				char *h=0;

				if(afx->track)
					h=mainvar->GenerateString("Track Output:",afx->track->GetName());
				else
					if(afx->channel)
						h=mainvar->GenerateString("Channel Output:",afx->channel->GetFullName());

				maingui->CreateEffectListPopUp(afx,this,h);

				if(h)
					delete h;
			}
			else
			{
				char *h=0;

				if(afx->track)
					h=mainvar->GenerateString("*Track Input:",afx->track->GetName());
				else
					if(afx->channel)
						h=mainvar->GenerateString("*Channel Input:",afx->channel->GetFullName());

				maingui->CreateEffectListPopUp(afx,this,h);

				if(h)
					delete h;
			}
		}
	}
}

void Edit_AudioMix::MouseReleaseInSlider(guiGadget_CW *slider,bool leftmouse)
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		if(ot->id==OBJECTID_SLIDER || ot->id==OBJECTID_MASTERSLIDER)
		{
			if(leftmouse==true)
			{
				Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)ot;

				bool m_valueclicked=c->GetTrackHead()->m_volumeclicked;
				bool blt=false;

				if(m_valueclicked==true)
				{
					c->GetTrackHead()->MIDIVolumeUp();
					c->ShowMIDIVolumeSlider();
					blt=true;
				}

				bool valueclicked=c->GetTrackHead()->volumeclicked;

				if(valueclicked==true)
				{
					c->GetTrackHead()->VolumeUp();
					c->ShowVolumeSlider();
					blt=true;
				}

				if(blt==true)
					slider->Blt(c);
			}
		}

		if(ot->id==OBJECTID_PAN)
		{
			if(leftmouse==true)
			{
				Edit_AudioMix_Pan *c=(Edit_AudioMix_Pan *)ot;

				if(c->clicked==true)
				{
					c->clicked=false;
					c->slider->ShowPan(true);
					slider->Blt(c);
				}
			}
		}

		ot=ot->NextObject();
	}
}

int Edit_AudioMix::CreateMixerObjects(guiForm_Child *child,OListCoosX *olistx,bool slider)
{
	if(!child)
		return 0;

	int addwidth,daddwidth;

	if(solotrack==true)
	{
		addwidth=forms[0][2].width-2;

		//if( (GetShowFlag()&SHOW_MIDI) && (GetShowFlag()&SHOW_AUDIO))
		//	addwidth/=2;

		if(/*slider==true && */addwidth>audiomixzooms[NUMBERAUDIOMIXZOOM-1])
		{
			double h=audiomixzooms[NUMBERAUDIOMIXZOOM-1];
			h*=1.1;

			if(h<audiomixzooms[NUMBERAUDIOMIXZOOM-1])
				h=audiomixzooms[NUMBERAUDIOMIXZOOM-1];

			addwidth=h;

			Edit_AudioMixFilterChannel *c=FirstFilterChannel();
			if(c && c->isaudio==true && c->isMIDI==true)
				addwidth/=2;
		}
	}
	else
	{
		// Ratio 1.3
		double h=audiomixzooms[mixerzoom];
		//h*=1.1;

		if(h<audiomixzooms[mixerzoom])
			h=audiomixzooms[mixerzoom];

		addwidth=(int)h;
	}

	Edit_AudioMixFilterChannel *c=FirstFilterChannel();
	while(c)
	{
		if(c->isaudio==true && c->isMIDI==true)
			daddwidth=2*addwidth;
		else
			daddwidth=addwidth;

		olistx->AddCooObject(c,daddwidth,daddwidth,1);
		c=c->NextChannel();
	}

	return addwidth;
}

int Edit_AudioMix::GetShowFlag()
{
	return solotrack==true?mainsettings->audiomixersettings_solo[set]:mainsettings->audiomixersettings[set];
}

void Edit_AudioMix::RefreshAllMixerWithSameSet(int f)
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(w->WindowSong()==WindowSong())
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *mixer=(Edit_AudioMix *)w;

					if(mixer->set==set && mixer->solotrack==solotrack)
					{
						if(f==SHOW_FX)
						{
							mixer->FormYEnable(1,GetShowFlag()&SHOW_FX?true:false);
							mixer->ShowFilter();
						}
						else
						{
							if(f==SHOW_IO)
							{
							}
							else
								mixer->CreateFilterChannels();

							mixer->ShowAll();
							mixer->ShowFilter();
						}
					}
				}
				break;
			}
		}

		w=w->NextWindow();
	}
}

void Edit_AudioMix::ClearFlag(int f)
{
	if(solotrack==true)
		mainsettings->audiomixersettings_solo[set] CLEARBIT f;
	else
		mainsettings->audiomixersettings[set] CLEARBIT f;

	RefreshAllMixerWithSameSet(f);
}

void Edit_AudioMix::AddFlag(int f)
{
	if(solotrack==true)
		mainsettings->audiomixersettings_solo[set] |= f;
	else
		mainsettings->audiomixersettings[set] |= f;

	RefreshAllMixerWithSameSet(f);
}

void Edit_AudioMix::ShowFullMasterSlider()
{
	if(!masterslider)
		return;

	guiBitmap *bitmap=&masterslider->gbitmap;
	bitmap->guiFillRect(COLOUR_MASTERCHANNEL);

	ClearEMCMasterSlider();

	if(Edit_AudioMixSlider *s=new Edit_AudioMixSlider(masterslider,true))
	{
		s->filterchannel=&masterfilterchannel;

		masterfilterchannel.channel=&WindowSong()->audiosystem.masterchannel;
		masterfilterchannel.track=0;

		s->isaudio=(GetShowFlag()&SHOW_AUDIO)?true:false;
		s->isMIDI=(GetShowFlag()&SHOW_MIDI)?true:false;
		s->editor=this;
		s->pan_enable=false; // Master No Panning

		guiobjects.AddGUIObject(0,0,masterslider->GetX2(),masterslider->GetY2(),masterslider,s);

		s->ShowSlider();
	}

}

void Edit_AudioMix::ShowFullSlider()
{
	if(!slider)
		return;

	guiBitmap *bitmap=&slider->gbitmap;
	bitmap->guiFillRect(COLOUR_UNUSED);

	// Init Slider
	ClearEMCSlider();

	sliderxobjects.DeleteAllO(slider);
	int zoomx=CreateMixerObjects(slider->formchild,&sliderxobjects,true);

	sliderxobjects.EndBuild();
	sliderxobjects.InitXStartO();
	sliderxobjects.OptimizeXStart();

	while(sliderxobjects.GetShowObject() && sliderxobjects.GetInitX()<slider->GetWidth())
	{
		Edit_AudioMixFilterChannel *fc=(Edit_AudioMixFilterChannel *)sliderxobjects.GetShowObject()->object;
		int sy,minpanh;

		if(solotrack==true)
			minpanh=GetShowFlag()&SHOW_IO?10:8;
		else 
			minpanh=GetShowFlag()&SHOW_IO?11:9;

		minpanh++;

		bool pan_enable;

		if((GetShowFlag()&SHOW_PAN) && slider->GetHeight()>minpanh*maingui->GetFontSizeY())
		{
			if(fc->track || (fc->channel && fc->channel->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL))
			{
				sy=2*maingui->GetButtonSizeY();
				pan_enable=true;
			}
			else
			{
				sy=0;
				pan_enable=false;
			}
		}
		else{
			sy=0;
			pan_enable=false;
		}

		if(Edit_AudioMixSlider *s=new Edit_AudioMixSlider(slider,false))
		{
			s->filterchannel=fc;
			s->isaudio=fc->isaudio;
			s->isMIDI=fc->isMIDI;
			s->editor=this;
			s->pan_enable=pan_enable;

			guiobjects.AddGUIObject(sliderxobjects.GetInitX(),sy,sliderxobjects.GetInitX2(),solotrack==true?slider->GetY2():slider->GetY2()-maingui->GetButtonSizeY(),slider,s);

			s->ShowSlider();
		}

		sliderxobjects.NextXO();
	}

	if(horzgadget)
		horzgadget->ChangeSlider(&sliderxobjects,zoomx);
}

void Edit_AudioMix::ShowOverview()
{
	if(!slider)return;

	guiBitmap *bitmap=&overview->gbitmap;
	//int addy;
	//double h=(double)(overview->GetWidth()),stepy=overview->GetHeight()-6,stepc=0;
	//OSTART slen=overviewlenght=WindowSong()->GetSongLength_Ticks();

	bitmap->guiFillRect(COLOUR_OVERVIEW_BACKGROUND);

	if(filter.GetCount())
	{
		int x=0;

		int w=bitmap->GetX2();
		w/=filter.GetCount();

		if(w<1)
		{
			nooverview=true;
			return;
		}

		overviewx2=filter.GetCount()*w;

		if(w<5)
			nopeak=true;
		else
			nopeak=false;

		nooverview=false;

		int tc=0;
		Edit_AudioMixFilterChannel *c=FirstFilterChannel();

		while(c)
		{
			// For Realtime Refresh
			c->ovx=x;
			c->ovy=1;

			if(w>4)
				c->ovx2=x+(w-4);
			else
			{
				c->ovx2=w==1?x:x+(w-2);
			}

			c->ovy2=bitmap->GetY2()-1;

			if(nopeak==false)
			{
				c->peakx=x+2;
				c->peakx2=c->peakx+((c->ovx2-c->peakx)/2);

				if(c->peakx2>c->peakx+maingui->GetFontSizeY())
					c->peakx2=c->peakx+maingui->GetFontSizeY();
			}

			int colour=COLOUR_OVERVIEW_BACKGROUND;

			if(c->channel)
				colour=c->channel->GetBgColour();
			else
				if(c->track)
				{
					if(c->track->IsSelected()==true)
						colour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
					else
						colour=COLOUR_BACKGROUNDEDITOR_GFX_HIGHLITE;
				}

				if(nopeak==true)
				{
					bitmap->guiFillRect(x,0,c->ovx2,bitmap->GetY2(),colour);
				}
				else
				{
					bitmap->guiFillRect(x,0,c->peakx2+2,bitmap->GetY2(),colour);
					bitmap->guiFillRect(c->peakx2+1,bitmap->GetY2()/2,c->ovx2,bitmap->GetY2(),colour);
					c->ShowIOPeak(this,true);
				}

				x+=w;
				c=c->NextChannel();
		}
	}
	else
		nooverview=true;
}

void Edit_AudioMix::ShowOverviewPositions()
{
	if(!overview)
		return;

	guiBitmap *bitmap=&overview->spritebitmap;
	bitmap->guiFillRect(COLOUR_BLACK);

	if((!slider) || sliderxobjects.zoomwidth==0 || nooverview==true)
		return;

	if(filter.GetCount()>0)
	{
		double h=sliderxobjects.zoomwidth;
		double h2=sliderxobjects.startx;
		double hx2=sliderxobjects.startx+slider->GetWidth();

		h2/=h;
		hx2/=h;

		if(hx2>1)
			hx2=1;

		h2*=overviewx2;
		hx2*=overviewx2;

		int ix=(int)h2;
		int ix2=(int)hx2;

		bitmap->guiFillRect(ix,0,ix2,bitmap->GetY2(),COLOUR_BLACK_LIGHT);
		bitmap->guiDrawRect(ix,0,ix2,bitmap->GetY2(),COLOUR_GREEN);

		//double h2=mixerobject
		//startx
	}
}

void MixerOverview_Callback(guiGadget_CW *g,int status)
{
	Edit_AudioMix *mix=(Edit_AudioMix *)g->from;

	switch(status)
	{
	case DB_CREATE:
		mix->overview=g;
		mix->skipoverview=true;
		break;

	case DB_PAINT:
		{
			if(mix->skipoverview==true)
				mix->skipoverview=false;
			else
			{
				mix->ShowOverview();
				mix->ShowOverviewPositions();
			}
		}
		break;

	case DB_LEFTMOUSEDOWN:
	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		if(mix->overview->leftmousedown==true)
			mix->MouseClickInOverview(true);
		break;

	case DB_LEFTMOUSEUP:
		//mix->MouseReleaseInSlider(true);
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		//p->MouseClickInOverview(false);	
		break;
	}
}

void Edit_AudioMix::ShowPattering()
{
	if(g_pattern)
	{
		patteringindex=mainsettings->autopattering[set];

		g_pattern->ChangeButtonText(Cxs[CXS_NOPATTERNCHECK+patteringindex]);
	}
}

void Edit_AudioMix::ShowTracking()
{
	trackingindex=mainsettings->autotracking[set];

	if(g_showwhat)
	{
		int cxs;

		switch(trackingindex)
		{
		case 0:
			cxs=CXS_AUTOALL;
			break;

		case 1:
			cxs=CXS_AUTOSELECTED;
			break;

		case 2:
			cxs=CXS_AUTOFOCUS;
			break;

		case 3:
			cxs=CXS_AUTOAUDIOORMIDI;
			break;

		case 4:
			cxs=CXS_AUTOAUDIO;
			break;

		case 5:
			cxs=CXS_AUTOMIDI;
			break;

		case 6:
			cxs=CXS_AUTOINSTR;
			break;

		case 7:
			cxs=CXS_AUTOTRACKSRECORD;
			break;
		}

		g_showwhat->ChangeButtonText(Cxs[cxs]);
	}
}

void Edit_AudioMix::InitGadgets()
{
	glist.SelectForm(0,0);

	int addw=bitmap.GetTextWidth("WWWw");

	if(solotrack==false)
	{
		guitoolbox.CreateToolBox(TOOLBOXTYPE_AUDIOMIXER,guiToolBox::CTB_NOSPP);

		g_showwhat=glist.AddButton(-1,-1,bitmap.GetTextWidthCxs(CXS_AUTOALL,CXS_AUTOMIDI),-1,GADGETID_SHOWWHAT,MODE_MENU);
		ShowTracking();

		glist.AddLX();
		g_pattern=glist.AddButton(-1,-1,bitmap.GetTextWidthCxs(CXS_AUTOALL,CXS_ONLYPATTERNATSONGPOSITION),-1,GADGETID_PATTERN,MODE_MENU);
		ShowPattering();
	}

	glist.Return();

	g_fx=glist.AddButton(-1,-1,INFOSIZE,-1,"FX",GADGETID_FX,GetShowFlag()&SHOW_FX?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_GROUP|MODE_TOGGLE,Cxs[CXS_AUDIOMIDIFX]);
	glist.AddLX();

	g_inputfx=glist.AddButton(-1,-1,addw,-1,"iFX",GADGETID_INPUTFX,GetShowFlag()&SHOW_INPUTFX?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,Cxs[CXS_AUDIOMIDIINPUTFX]);
	glist.AddLX();

	g_io=glist.AddButton(-1,-1,addw,-1,"I/O",GADGETID_IO,GetShowFlag()&SHOW_IO?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Audio+MIDI I/O");
	glist.AddLX();

	g_pan=glist.AddButton(-1,-1,addw,-1,"Pan",GADGETID_PAN,GetShowFlag()&SHOW_PAN?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Audio+MIDI Panorama");
	glist.AddLX();

	if(solotrack==false)
	{
		g_tracks=glist.AddButton(-1,-1,addw,-1,"Track",GADGETID_TRACKS,GetShowFlag()&SHOW_TRACKS?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Tracks");
		glist.AddLX();

		g_bus=glist.AddButton(-1,-1,4*maingui->GetFontSizeY(),-1,"Bus",GADGETID_BUS,GetShowFlag()&SHOW_BUS?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Bus Channels");
		glist.AddLX();

		g_audioinput=glist.AddButton(-1,-1,addw,-1,"Input",GADGETID_AUDIOINPUT,GetShowFlag()&SHOW_AUDIOINPUT?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Hardware Input Channels");
		glist.AddLX();

		g_audiooutput=glist.AddButton(-1,-1,addw,-1,"Output",GADGETID_AUDIOOUTPUT,GetShowFlag()&SHOW_AUDIOOUTPUT?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Hardware Output Channels");
		glist.AddLX();
	}

	if(solotrack==true)
		glist.Return();


	g_auto=glist.AddButton(-1,-1,addw,-1,"Auto",GADGETID_AUTO,GetShowFlag()&SHOW_AUTO?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,Cxs[CXS_AUTOMIX]);
	glist.AddLX();

	g_MIDI=glist.AddButton(-1,-1,addw,-1,"MIDI",GADGETID_MIDI,GetShowFlag()&SHOW_MIDI?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
	glist.AddLX();
	g_audio=glist.AddButton(-1,-1,addw,-1,"Audio",GADGETID_AUDIO,GetShowFlag()&SHOW_AUDIO?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
	glist.AddLX();

	g_set[0]=glist.AddButton(-1,-1,addw/2,-1,"1",GADGETID_SET1,set==0?MODE_TOGGLE|MODE_TOGGLED|MODE_TEXTCENTER:MODE_TOGGLE|MODE_TEXTCENTER);
	glist.AddLX();
	g_set[1]=glist.AddButton(-1,-1,addw/2,-1,"2",GADGETID_SET2,set==1?MODE_TOGGLE|MODE_TOGGLED|MODE_TEXTCENTER:MODE_TOGGLE|MODE_TEXTCENTER);
	glist.AddLX();
	g_set[2]=glist.AddButton(-1,-1,addw/2,-1,"3",GADGETID_SET3,set==2?MODE_TOGGLE|MODE_TOGGLED|MODE_TEXTCENTER:MODE_TOGGLE|MODE_TEXTCENTER);
	glist.AddLX();
	g_set[3]=glist.AddButton(-1,-1,addw/2,-1,"4",GADGETID_SET4,set==3?MODE_TOGGLE|MODE_TOGGLED|MODE_TEXTCENTER:MODE_TOGGLE|MODE_TEXTCENTER);
	glist.AddLX();
	g_set[4]=glist.AddButton(-1,-1,addw/2,-1,"5",GADGETID_SET5,set==4?MODE_TOGGLE|MODE_TOGGLED|MODE_TEXTCENTER:MODE_TOGGLE|MODE_TEXTCENTER);
	glist.Return();

	if(solotrack==false)
	{
		glist.AddChildWindow(-1,-1,-2,-2,MODE_LEFT|MODE_RIGHT|MODE_BOTTOM|MODE_SPRITE,0,&MixerOverview_Callback,this);

		glist.SelectForm(0,3);

		// Horz
		SliderCo hor;

		hor.from=0;
		hor.to=0;
		hor.pos=0;
		hor.horz=true;
		hor.subw=SIZE_HZOOM_SLIDER+1;

		horzgadget=glist.AddSlider(&hor,GADGETID_EDITORSLIDER_HORIZ,MODE_LEFT|MODE_TOP|MODE_BOTTOM|MODE_RIGHT,0,0);

		if(horzgadget)
		{
			hor.from=0;
			hor.to=NUMBERAUDIOMIXZOOM-1;
			hor.pos=mixerzoom;
			hor.horz=true;
			hor.page=1;
			hor.staticwidth=SIZE_HZOOM_SLIDER;
			hor.subh=hor.subw=0;

			horzzoomgadget=glist.AddSlider(&hor,GADGETID_EDITORSLIDER_HORIZZOOM,MODE_TOP|MODE_BOTTOM|MODE_RIGHT|MODE_STATICWIDTH,0,"Zoom <->");
		}
	}
	else
	{
		horzgadget=horzzoomgadget=0;
		overview=0;
	}

	SliderCo vert;
	// Vert - Effects
	glist.SelectForm(1,1);

	vert.horz=false;
	vert.from=0;
	vert.to=0;
	vert.pos=0;
	vert.subh=0;

	effectvertgadget=glist.AddSlider(&vert,GADGETID_EDITORSLIDER_VERT,MODE_LEFT|MODE_TOP|MODE_BOTTOM|MODE_RIGHT,0,0);

	int widthmaster=7*maingui->GetFontSizeY();

	glist.SelectForm(0,1);
	glist.AddChildWindow(-1,-1,solotrack==true?-2:widthmaster,-2,solotrack==true?MODE_RIGHT|MODE_BOTTOM:MODE_RIGHT|MODE_BOTTOM|MODE_SUBRIGHT,0,&MixerEffects_Callback,this);
	glist.AddLX();

	glist.SelectForm(0,2);
	glist.AddChildWindow(-1,-1,solotrack==true?-2:widthmaster,-2,solotrack==true?MODE_RIGHT|MODE_BOTTOM:MODE_RIGHT|MODE_BOTTOM|MODE_SUBRIGHT,0,&MixerSlider_Callback,this);
	glist.AddLX();

	if(solotrack==false)
	{
		glist.SelectForm(0,1);
		glist.AddChildWindow(-1,-1,widthmaster,-2,MODE_RIGHT|MODE_BOTTOM|MODE_STATICWIDTH,0,&MixerEffects_Callback,this);

		glist.SelectForm(0,2);
		glist.AddChildWindow(-1,-1,widthmaster,-2,MODE_RIGHT|MODE_BOTTOM|MODE_STATICWIDTH,0,&MixerSlider_Callback,this);
	}
}


void Edit_AudioMix::RefreshObjects(LONGLONG type,bool editcall)
{
	ShowAll(true);
}

void Edit_AudioMix::KeyDownRepeat()
{

	switch(nVirtKey)
	{
	case KEY_LEFT10:
	case KEY_CURSORLEFT:
		{
			if(maingui->GetShiftKey()==true)
			{
				if(horzzoomgadget)
					horzzoomgadget->DeltaY(-1);
			}
			else
				if(horzgadget)
					horzgadget->DeltaY(-1);
		}
		break;

	case KEY_RIGHT10:
	case KEY_CURSORRIGHT:
		{
			if(maingui->GetShiftKey()==true)
			{
				if(horzzoomgadget)
					horzzoomgadget->DeltaY(1);
			}
			else
				if(horzgadget)
					horzgadget->DeltaY(1);
		}
		break;

	case KEY_DIVIDE: // 10er /
		{
			if(horzzoomgadget)
				horzzoomgadget->DeltaY(-1);
		}
		break;

	case KEY_MULTIPLY: // 10er *
		{
			if(horzzoomgadget)
				horzzoomgadget->DeltaY(1);
		}
		break;

	case KEY_UP10:
	case KEY_CURSORUP:
		{
			/*
			if(startchannel && startchannel->NextChannel())
			{
			int pindex=filterindex+channels.GetCount();
			Edit_AudioMixFilterChannel *f=GetFilterAtIndex(pindex);

			if(!f)
			{
			filterindex=filter.GetCount()>0?filter.GetCount()-1:0;
			SetStartChannel(LastFilterChannel(),filterindex);
			}
			else
			{
			filterindex=pindex;
			SetStartChannel(f,filterindex);
			}

			ChangeChannelSlider();
			}
			*/
		}
		break;

	case KEY_DOWN10:
	case KEY_CURSORDOWN:
		{
			/*
			if(startchannel && startchannel->PrevChannel())
			{
			int pindex=filterindex-channels.GetCount();
			Edit_AudioMixFilterChannel *f=GetFilterAtIndex(pindex);

			if(!f)
			{
			filterindex=0;
			SetStartChannel(FirstFilterChannel(),filterindex);
			}
			else
			{
			filterindex=pindex;
			SetStartChannel(f,filterindex);
			}

			ChangeChannelSlider();
			}
			*/
		}
		break;
	}
}

void Edit_AudioMix::KeyDown()
{
	switch(nVirtKey)
	{
	case '1':
		SetSet(0);
		break;

	case '2':
		SetSet(1);
		break;

	case '3':
		SetSet(2);
		break;

	case '4':
		SetSet(3);
		break;

	case '5':
		SetSet(4);
		break;

	default:
		KeyDownRepeat();
		break;
	}
}

void Edit_AudioMix::CopyFX(AudioChannel *from)
{
	//if(!from)
	//	from=WindowSong()->audiosystem.activechannel;

	if(from)
		mainbuffer->CopyEffectList(&from->io.audioeffects);
}

void Edit_AudioMix::PasteFX(AudioChannel *to)
{
	//	if(!to)
	//		to=WindowSong()->audiosystem.activechannel;

	if(to)
		mainbuffer->PasteBufferToEffectList(&to->io.audioeffects);
}

#ifdef OLDIE
void Edit_AudioMix::DeleteFX(AudioChannel *channel,int flag,bool refreshgui)
{
	if(!channel)
		channel=WindowSong()->audiosystem.activechannel;

	if(channel){

		/*
		bool refresh=false;
		bool refreshautomationtracks=false;

		if(flag&DELETE_INSTRUMENTS){

		InsertAudioEffect *iae=channel->io.audioeffects.FirstActiveAudioInstrument();

		while(iae){

		maingui->RemoveAudioEffectFromGUI(iae);

		if(WindowSong()->RemoveAudioObjectFromSong(iae->audioeffect)==true)
		refreshautomationtracks=true;

		iae=channel->io.audioeffects.DeleteInsertAudioEffect(iae,true);
		refresh=true;
		}
		}

		if(flag&DELETE_FX){

		InsertAudioEffect *iae=channel->io.audioeffects.FirstInsertAudioEffect();

		while(iae){
		maingui->RemoveAudioEffectFromGUI(iae);

		if(WindowSong()->RemoveAudioObjectFromSong(iae->audioeffect)==true)
		refreshautomationtracks=true;

		iae=channel->io.audioeffects.DeleteInsertAudioEffect(iae,true);
		refresh=true;
		}
		}

		*/

		bool refresh=channel->io.audioeffects.DeleteEffectsFlag(flag);

		if(refresh==true && refreshgui==true)
		{
			//	if(refreshautomationtracks==true)
			//		maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_ARRANGE,0);

			maingui->RefreshEffects(&channel->io.audioeffects);
		}
	}
}
#endif

#ifdef OLDIE
guiMenu *Edit_TrackAudioMix::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		if(guiMenu *n=menu->AddMenu("Editor",0))
		{
			if(track)
			{
				n->AddFMenu("Mixer",new globmenu_AudioMixer(WindowSong(),track),"F7");
			}

			//n->AddFMenu("Song (Channels/Bus) Mixer",new globmenu_SongMixer(WindowSong(),0),"F8");
			//n->AddFMenu("Song Master Mixer",new globmenu_SongMixer(WindowSong(),0,true),"F9");
		}

		if(!track)
		{
			if(guiMenu *n=menu->AddMenu(Cxs[CXS_OPTIONS],0))
			{
				if(!track)
				{
					class m_showonlyaudiotracks:public guiMenu
					{
					public:
						m_showonlyaudiotracks(Edit_TrackAudioMix *e)
						{
							editor=e;
						}

						void MenuFunction()
						{
							mainsettings->showonlyaudiotracks=editor->showonlyaudiotracks=editor->showonlyaudiotracks==true?false:true;

							editor->Repaint();
							//editor->ShowChannels();
							//editor->RedrawOSGadgets();

							editor->menu_showonlyaudiotracks->menu->Select(editor->menu_showonlyaudiotracks->index,editor->showonlyaudiotracks);
						} //

						Edit_TrackAudioMix *editor;
					};

					menu_showonlyaudiotracks=n->AddFMenu(Cxs[CXS_SHOWONLYTRACKSWITHAUDIO],new m_showonlyaudiotracks(this),showonlyaudiotracks);
				}
			}
		}
	}

	return menu;
}

bool Edit_TrackAudioMix::RefreshCheck()
{
	/*
	Edit_AudioMixEffects *em=FirstAudioChannel();

	if(em)
	{
	while(em)
	{
	Seq_Track *indextrack=WindowSong()->GetTrackIndex(em->trackindex);

	if(indextrack!=em->track)
	{
	return true;
	}

	em=em->NextChannel();
	}
	}
	*/
	return true;
}

void Edit_TrackAudioMix::ShowActiveChannel()
{
	SetWindowName();

	if(windowname)
	{
		char *h;

		if(h=mainvar->GenerateString(windowname,"[Track:",WindowSong()->GetFocusTrack()?WindowSong()->GetFocusTrack()->GetName():"-"," ]"))
		{
			delete windowname;
			windowname=h;
			guiSetWindowText(windowname);
		}
	}
}
#endif

void Edit_AudioMix::CreateTrackFilter(OList *list)
{
	if((GetShowFlag()&SHOW_TRACKS) && 
		((GetShowFlag()&SHOW_MIDI) || (GetShowFlag()&SHOW_AUDIO) || (GetShowFlag()&SHOW_AUTO)) 
		)
	{
		OSTART spp=WindowSong()->GetSongPosition();

		// AUDIO
		Seq_Track *t=WindowSong()->FirstTrack();

		while(t)
		{
			bool add=t->CheckTracking(mainsettings->autotracking[set]);

			// Pattern Check
			if(add==true)
			{
				switch(mainsettings->autopattering[set])
				{
				case PATTERING_OFF:
					break;

				case PATTERING_FROMTO:
				case PATTERING_UNDER:
					{
						int type=MEDIATYPE_AUDIO|MEDIATYPE_AUDIO_RECORD|MEDIATYPE_MIDI;

						switch(mainsettings->autotracking[set])
						{
						case AUTO_WITHAUDIOORMIDI:
							type=MEDIATYPE_AUDIO|MEDIATYPE_AUDIO_RECORD|MEDIATYPE_MIDI;
							break;

						case AUTO_WITHAUDIO:
							type=MEDIATYPE_AUDIO|MEDIATYPE_AUDIO_RECORD;
							break;

						case AUTO_WITHMIDI:
							type=MEDIATYPE_MIDI;
							break;
						}

						Seq_Pattern *p=t->FirstPattern(type);

						while(p)
						{
							if(mainsettings->autopattering[set]==PATTERING_FROMTO && (p->GetPatternStart()>=spp || (p->GetPatternStart()<=spp && p->GetPatternEnd()>=spp)) )
								goto okcheck;

							if(mainsettings->autopattering[set]==PATTERING_UNDER && p->GetPatternStart()<=spp && p->GetPatternEnd()>=spp)
								goto okcheck;

							p=p->NextPattern(type);
						}

						add=false;
					}
					break;
				}
			}

okcheck:
			if(add==true)
			{	
				if((!t->parent) || t->ParentShowChilds()==true)
				{

					if(Edit_AudioMixFilterChannel *eam=new Edit_AudioMixFilterChannel)
					{
						eam->channel=0;
						eam->track=t;

						InitIsMIDIAUDIO(eam);

						list->AddEndO(eam);
					}
				}
			}

			t=t->NextTrack();
		}

	} // End Tracks
}

void Edit_AudioMix::InitIsMIDIAUDIO(Edit_AudioMixFilterChannel *eam)
{
	if(eam->track && eam->track->ismetrotrack==true)
	{
		eam->isMIDI=eam->isaudio=true;
		return;
	}

	if(eam->track && (GetShowFlag()&SHOW_AUTO))
	{
		eam->isMIDI=eam->track->CheckIfTrackIsMIDI();

		if(eam->track->recordtracktype==TRACKTYPE_MIDI)
			eam->isMIDI=true;

		eam->isaudio=true;
	}
	else
	{
		eam->isaudio=(GetShowFlag()&SHOW_AUDIO)?true:false;
		eam->isMIDI=(GetShowFlag()&SHOW_MIDI)?true:false;
	}
}

bool Edit_AudioMix::CompareIsMIDIAUDIO(Edit_AudioMixSlider *c)
{
	comparefilterchannel.channel=c->GetChannel();
	comparefilterchannel.track=c->GetTrack();

	InitIsMIDIAUDIO(&comparefilterchannel);

	if(comparefilterchannel.isaudio!=c->isaudio || comparefilterchannel.isMIDI!=c->isMIDI)
		return false;

	return true;
}

void Edit_AudioMix::CreateFilterChannels()
{
	filter.DeleteAllO();

	if(solotrack==true)
	{
		switch(WindowSong()->GetFocusType())
		{
		case Seq_Song::FOCUSTYPE_MASTER:

			if(Edit_AudioMixFilterChannel *eam=new Edit_AudioMixFilterChannel)
			{
				eam->channel=&WindowSong()->audiosystem.masterchannel;
				eam->track=0;

				InitIsMIDIAUDIO(eam);

				filter.AddEndO(eam);
			}
			break;

		case Seq_Song::FOCUSTYPE_BUS:
			{
				if(WindowSong()->audiosystem.GetFocusBus())
				{
					if(Edit_AudioMixFilterChannel *eam=new Edit_AudioMixFilterChannel)
					{
						eam->isaudio=true;
						eam->isMIDI=false;

						eam->channel=WindowSong()->audiosystem.GetFocusBus();
						eam->track=0;

						filter.AddEndO(eam);
					}
				}
			}
			break;

		case Seq_Song::FOCUSTYPE_METRO:
			{
				if(track=WindowSong()->GetFocusMetroTrack())
				{
					if(Edit_AudioMixFilterChannel *eam=new Edit_AudioMixFilterChannel)
					{
						eam->channel=0;
						eam->track=track;

						InitIsMIDIAUDIO(eam);

						filter.AddEndO(eam);
					}
				}
			}
			break;

		case Seq_Song::FOCUSTYPE_TRACK:
			{
				if(track=WindowSong()->GetFocusTrack())
				{
					if(Edit_AudioMixFilterChannel *eam=new Edit_AudioMixFilterChannel)
					{
						eam->channel=0;
						eam->track=track;

						InitIsMIDIAUDIO(eam);

						filter.AddEndO(eam);
					}
				}
			}
			break;

		}
	}
	else
	{
		CreateTrackFilter(&filter);

		if((GetShowFlag()&SHOW_AUDIO) || (GetShowFlag()&SHOW_AUTO))
		{
			// Bus
			if(GetShowFlag()&SHOW_BUS)
			{
				AudioChannel *b=WindowSong()->audiosystem.FirstBusChannel();
				while(b)
				{
					if(Edit_AudioMixFilterChannel *eam=new Edit_AudioMixFilterChannel)
					{
						eam->channel=b;
						eam->track=0;

						eam->isaudio=true;
						eam->isMIDI=false;

						filter.AddEndO(eam);
					}

					b=b->NextChannel();
				}
			}
			// Master

			if(GetShowFlag()&SHOW_AUDIOINPUT)
			{
				AudioChannel *b=WindowSong()->audiosystem.FirstDeviceInChannel();
				while(b)
				{
					if(Edit_AudioMixFilterChannel *eam=new Edit_AudioMixFilterChannel)
					{
						eam->channel=b;
						eam->track=0;

						eam->isaudio=true;
						eam->isMIDI=false;

						filter.AddEndO(eam);
					}

					b=b->NextChannel();
				}
			}

			if(GetShowFlag()&SHOW_AUDIOOUTPUT)
			{
				AudioChannel *b=WindowSong()->audiosystem.FirstDeviceOutChannel();
				while(b)
				{
					if(Edit_AudioMixFilterChannel *eam=new Edit_AudioMixFilterChannel)
					{
						eam->channel=b;
						eam->track=0;

						eam->isaudio=true;
						eam->isMIDI=false;

						filter.AddEndO(eam);
					}

					b=b->NextChannel();
				}
			}
		}
	}
}

void Edit_AudioMix::SetMasterMode()
{
	mastermode=false;

	if(solotrack==true)
	{
		if(WindowSong()->GetFocusType()==Seq_Song::FOCUSTYPE_MASTER)
			mastermode=true;
	}
}

EditData *Edit_AudioMix::EditDataMessage(EditData *data)
{
	if(data)
	{
		switch(data->id)
		{
		case EDIT_NAME:
			{
				Edit_AudioMix_Name *c=(Edit_AudioMix_Name *)data->helpobject;
				c->EditName(data->newstring);
			}
			break;

		case EDIT_SLIDERAUDIOVOLUME:
			{
				Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)data->helpobject;

				double volumeh=mainaudio->ConvertDBToLogArrayFactor(data->dnewvalue);
				OSTART atime=GetAutomationTime();

				Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

				while(flt)
				{
					if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio)
					{
						if(flt->GetFX()==c->GetFX() || (c->GetTrack() && flt->track && c->GetTrack()->IsSelected()==true && flt->track->IsSelected()==true))
						{
							flt->GetFX()->volume.AutomationEdit(WindowSong(),atime,0,volumeh,AEF_USEREDIT);
						}
					}

					flt=flt->NextChannel();
				}
			}
			break;

		case EDIT_SLIDERMIDIVOLUME:
			{
				Edit_AudioMixSlider *c=(Edit_AudioMixSlider *)data->helpobject;

				OSTART atime=GetAutomationTime();

				if(c->GetChannel())
				{
					c->GetChannel()->SetMIDIVelocity(data->newvalue,atime);
				}
				else
				{
					Edit_AudioMixFilterChannel *flt=FirstFilterChannel();

					while(flt)
					{
						if(c->isMIDI==flt->isMIDI && c->isaudio==flt->isaudio)
						{
							if(flt->GetFX()==c->GetFX() || (c->GetTrack() && flt->track && c->GetTrack()->IsSelected()==true && flt->track->IsSelected()==true))
							{
								flt->track->t_trackeffects.SetVelocity(data->newvalue,atime);
							}
						}

						flt=flt->NextChannel();
					}
				}
			}
			break;
		}
	}

	return 0;
}

// Master ID<>Track/Channel ID
Edit_AudioMix_Output::Edit_AudioMix_Output(Edit_AudioMixSlider *s,guiGadget *g,bool ismaster)
{
	id=ismaster==true?OBJECTID_MASTERIOOUTPUT:OBJECTID_IOOUTPUT;
	slider=s;
	gadget=g;
	ShowOutput();
}

Edit_AudioMix_FXEDIT::Edit_AudioMix_FXEDIT(Edit_AudioMixEffects *fx,InsertAudioEffect *i,guiGadget *g)
{
	id=fx->ismaster==true?OBJECTID_MASTERFXEDIT:OBJECTID_FXEDIT;
	effects=fx;
	iae=i;
	gadget=g;
}

Edit_AudioMix_EmptyFX::Edit_AudioMix_EmptyFX(Edit_AudioMixEffects *fx,guiGadget *g,bool is_MIDI)
{
	id=fx->ismaster==true?OBJECTID_MASTEREMPTYFX:OBJECTID_EMPTYFX;
	effects=fx;
	gadget=g;
	isMIDI=is_MIDI;
}

Edit_AudioMix_FX::Edit_AudioMix_FX(Edit_AudioMixEffects *fx,InsertAudioEffect *i,guiGadget *g)
{
	id=fx->ismaster==true?OBJECTID_MASTERFX:OBJECTID_FX;
	effects=fx;
	iae=i;
	gadget=g;
	programname=0;
	ShowEffectName();
}

Edit_AudioMix_ABFX::Edit_AudioMix_ABFX(Edit_AudioMixEffects *fx,guiGadget *g,bool is_MIDI)
{
	id=fx->ismaster==true?OBJECTID_MASTERABFX:OBJECTID_ABFX;
	effects=fx;
	gadget=g;
	isMIDI=is_MIDI;

	ShowAB();
}

Edit_AudioMix_ABINPUTFX::Edit_AudioMix_ABINPUTFX(Edit_AudioMixEffects *fx,guiGadget *g)
{
	id=fx->ismaster==true?OBJECTID_ABFXMASTERINPUT:OBJECTID_ABFXINPUT;
	effects=fx;
	gadget=g;
	ShowAB();
}


