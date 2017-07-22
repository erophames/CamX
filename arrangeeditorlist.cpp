#include "arrangeeditor.h"
#include "arrangeeditorlist.h"
#include "object_song.h"
#include "gui.h"
#include "songmain.h"

enum
{
	GADGETID_SELECTLIST=GADGETID_EDITORBASE,
	GADGETID_SELECTRECORDING,
	GADGETID_GETRECORDING,
	GADGETID_SETRECORDING
};

enum{
	TABL_NUMBER,
	TABL_COLOUR,
	TABL_MUTE,
	TABL_SOLO,
	TABL_RECORD,
	TABL_FREEZE,

	TABL_MIDI,
	TABL_VU,
	TABL_CHILD,
	TABL_AUTOMATION,
	TABL_AUTOMATIONUPDOWN,
	TABL_NAME,

	TAB_TABSLIST
};

enum{
	TABR_NUMBER,
	TABR_COLOUR,
	TABR_MUTE,
	TABR_SOLO,
	TABR_RECORD,
	TABR_FREEZE,

	TABR_MIDI,
	TABR_VU,
	TABR_CHILD,

	TABR_RECORDS1,
	TABR_RECORDS2,
	TABR_RECORDS3,
	TABR_RECORDS4,
	TABR_RECORDS5,

	TABR_NAME,

	TAB_TABSREC
};

enum{
	OBJECTID_TRACKLIST=OI_LAST,
	OBJECTID_AUTOMATIONTRACKLIST
};

Edit_ArrangeList::Edit_ArrangeList(Edit_Arrange *ar)
{
	editorid=EDITORTYPE_ARRANGELIST;
	InitForms(FORM_HORZ2x1SLIDERV);

	//autovscroll=true;
	isstatic=true;

	song=ar->WindowSong();
	editor=ar;

	mode=false;
	getmode=true;
}

void ArrangeEditor_List_Callback(guiGadget_CW *g,int status)
{
	Edit_ArrangeList *al=(Edit_ArrangeList *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			g->menuindex=0;
			al->list=(guiGadget_Tab *)g;

			al->InitTabs();
		}
		break;

	case DB_PAINT:
		{
			al->ShowList();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		//	ar->MouseMoveInTracks(true);
		break;

	case DB_LEFTMOUSEDOWN:
		al->MouseClickInTracks(true);	
		break;

	case DB_LEFTMOUSEUP:
		//ar->MouseReleaseInTracks(true);	
		break;

	case DB_RIGHTMOUSEDOWN:
		al->MouseClickInTracks(false);	
		break;

	case DB_DOUBLECLICKLEFT:
		al->MouseDoubleClickInTracks(true);
		break;

	case DB_DELTA:
		//ar->DeltaInTracks();
		break;
	}
}

void Edit_ArrangeList::ScrollTo(Seq_Track *scrollto)
{
	if(OObject *f=trackobjects.FindObject(scrollto))
	{
		trackobjects.ScrollY(f);
		DrawDBBlit(list);
		ShowVSlider();
	}
}

void Edit_ArrangeList::KeyDown()
{
	editor->InitVKey(nVirtKey);
	editor->KeyDown();
}

void Edit_ArrangeList::KeyDownRepeat()
{
	editor->InitVKey(nVirtKey);
	editor->KeyDownRepeat();
}


void Edit_ArrangeList::Init()
{
	InitGadgets();
}

void Edit_ArrangeList::InitTabs()
{
	if(!list)return;

	list->InitTabs(mode==false?TAB_TABSLIST:TAB_TABSREC);

	list->InitTabWidth(TABL_NUMBER,"0000");

	list->InitTabWidth(TABL_COLOUR,">");

	list->InitTabWidth(TABL_MUTE,"##");
	list->InitTabWidth(TABL_SOLO,"##");

	int recw=maingui->GetFontSizeY()+maingui->GetFontSizeY()/4;

	list->InitTabWidth(TABL_RECORD,recw);
	list->InitTabWidth(TABL_FREEZE,"FFFF");

	list->InitTabWidth(TABL_MIDI,"#");
	list->InitTabWidth(TABL_VU,">>");
	list->InitTabWidth(TABL_CHILD,">>");

	if(mode==false)
	{
		list->InitTabWidth(TABL_AUTOMATION,">>>");
		list->InitTabWidth(TABL_AUTOMATIONUPDOWN,">>");
	}
	else
	{
		list->InitTabWidth(TABR_RECORDS1,recw);
		list->InitTabWidth(TABR_RECORDS2,recw);
		list->InitTabWidth(TABR_RECORDS3,recw);
		list->InitTabWidth(TABR_RECORDS4,recw);
		list->InitTabWidth(TABR_RECORDS5,recw);
	}

	list->InitXX2();
}

void Edit_ArrangeList::ShowMode()
{
	InitTabs();

	if(g_list)
		g_list->Toggle(mode==false?true:false);

	if(g_recording)
		g_recording->Toggle(mode==true?true:false);

	if(g_recordingget)
	{
		if(mode==false)
			g_recordingget->Disable();
		else
		{
			g_recordingget->Enable();
			g_recordingget->Toggle(getmode);
		}
	}

	if(g_recordingset)
	{
		if(mode==false)
			g_recordingset->Disable();
		else
		{
			g_recordingset->Enable();
			g_recordingset->Toggle(getmode==true?false:true);
		}
	}
}

void Edit_ArrangeList::InitGadgets()
{
	SliderCo vert;

	vert.formx=1;
	vert.formy=0;
	vert.nozoom=true;

	AddEditorSlider(0,&vert);

	glist.SelectForm(0,0);

	int offsety=3*maingui->GetFontSizeY();

	g_list=glist.AddButton(-1,-1,-1,-1,"List",GADGETID_SELECTLIST,MODE_TOGGLE|MODE_LEFTTOMID);
	glist.AddLX();
	g_recording=glist.AddButton(-1,-1,-1,-1,"Recording",GADGETID_SELECTRECORDING,MODE_TOGGLE|MODE_MIDTORIGHT);
	glist.Return();

	g_recordingget=glist.AddButton(-1,-1,-1,-1,"<Get REC>",GADGETID_GETRECORDING,MODE_TOGGLE|MODE_TEXTCENTER|MODE_LEFTTOMID);
	glist.AddLX();
	g_recordingset=glist.AddButton(-1,-1,-1,-1,">Set REC<",GADGETID_SETRECORDING,MODE_TOGGLE|MODE_TEXTCENTER|MODE_MIDTORIGHT);
	glist.Return();

	glist.AddTab(-1,offsety,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,&ArrangeEditor_List_Callback,this);

	ShowMode();

}

void Edit_ArrangeList::MouseDoubleClickInTracks(bool leftmouse)
{
}

void Edit_ArrangeList::MouseClickInTracks(bool leftmouse)
{
	guiObject *o=list->CheckObjectClicked(); // Object Under Mouse ?

	if(o)
	{
		switch(o->id)
		{
		case OBJECTID_AUTOMATIONTRACKLIST:
			{
				Edit_ArrangeList_AutomationTrack *at=(Edit_ArrangeList_AutomationTrack *)o;
				WindowSong()->EditAutomation(this,at->automationtrack);
			}
			break;

		case OBJECTID_TRACKLIST:
			{
				Edit_ArrangeList_Track *eat=(Edit_ArrangeList_Track *)o;

				if(leftmouse==true)
				{
					int index=list->GetMouseClickTabIndex();

					switch(index)
					{
					case TABL_MUTE:
						{
							if(eat->track)
							{
								eat->track->ToggleMute();
							}
							else
							{
								if(eat->channel)
									eat->channel->channel->ToggleMute();
							}
						}
						break;

					case TABL_SOLO:
						{
							if(eat->track)
							{
								OSTART automationtime=eat->track->song->GetSongPosition();

								eat->track->song->SoloTrack(eat->track,eat->track->GetSolo()==true?false:true,automationtime);
							}
							else
							{
								if(eat->channel)
								{
									OSTART automationtime=eat->channel->audiosystem->song->GetSongPosition();

									eat->channel->SetSolo(eat->channel->io.audioeffects.GetSolo()==true?false:true,eat->channel->audiosystem->song==mainvar->GetActiveSong()?true:false);
								}
							}
						}
						break;

					case TABL_CHILD:
						{
							if(eat->track)
								eat->track->ToggleShowChild(leftmouse);
						}
						break;

					case TABL_RECORD:
						if(eat->track)
							eat->track->ToggleRecord();
						break;
					}

					if(mode==false)
					{
						switch(index)
						{
						case TABL_AUTOMATION:
							if(editor->GetShowFlag()&SHOW_AUTOMATION)
							{
								WindowSong()->EditAutomationSettings(this,eat->track,eat->channel);
							}
							break;

						case TABL_AUTOMATIONUPDOWN:
							if(editor->GetShowFlag()&SHOW_AUTOMATION)
							{
								// + case TABR_RECORDS2:

								if(eat->track)
								{
									if(editor->GetShowFlag()&SHOW_AUTOMATION)
										WindowSong()->ToggleShowAutomationTracks(eat->track);
								}
								else
								{
									if(editor->GetShowFlag()&SHOW_AUTOMATION)
										WindowSong()->ToggleShowAutomationChannels(eat->channel);
								}
							}
							break;

						default:
							goto setfocus;
							break;
						}

					} // mode==false
					else
					{
						switch(index)
						{
						case TABL_AUTOMATION:
							{
								eat->track->SetOrGetRecordingSettings(0,getmode);
							}
							break;

						case TABL_AUTOMATIONUPDOWN:
							{
								eat->track->SetOrGetRecordingSettings(1,getmode);
							}
							break;

						case TABR_RECORDS3:
							eat->track->SetOrGetRecordingSettings(2,getmode);
							break;

						case TABR_RECORDS4:
							eat->track->SetOrGetRecordingSettings(3,getmode);
							break;

						case TABR_RECORDS5:
							eat->track->SetOrGetRecordingSettings(4,getmode);
							break;

						default:
							goto setfocus;
							break;

						}
					}//mode==true

					return;

setfocus:

					if(eat->track && eat->track->ismetrotrack==false)
					{
						WindowSong()->SetFocusTrack(eat->track);

						if(maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false)
							editor->Goto(GOTO_FOCUSTRACK);
					}
					else
						if(eat->channel)
						{
							WindowSong()->audiosystem.SetFocusBus(eat->channel,true);
						}
				}
				else
				{
					int index=list->GetMouseClickTabIndex();

					switch(index)
					{
					case TABL_CHILD:
						{
							if(eat->track)
								eat->track->ToggleShowChild(leftmouse);
						}
						break;

					default:
						if(eat->track)
							PopMenuTrack(eat->track);
						break;
					}
				}
			}
			break;
		}
	}
}

void Edit_ArrangeList::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_EDITORSLIDER_VERT: // Track Scroll
		trackobjects.InitWithSlider(vertgadget,true);
		DrawDBBlit(list);
		break;

	case GADGETID_SELECTLIST:
		if(mode==true)
		{
			mode=false;
			ShowMode();

			DrawDBBlit(list);
		}
		break;

	case GADGETID_SELECTRECORDING:
		if(mode==false)
		{
			mode=true;
			ShowMode();

			DrawDBBlit(list);
		}
		break;

	case GADGETID_GETRECORDING:
		getmode=true;
		ShowMode();
		break;

	case GADGETID_SETRECORDING:
		getmode=false;
		ShowMode();
		break;
	}
}


void Edit_ArrangeList::BuildTrackList()
{
	if(!list)return;

	int zoomy=maingui->GetFontSizeY()+maingui->GetFontSizeY()/4;

	if(mode==false)
	{
		editor->BuildTrackList(&trackobjects,list,zoomy,editor->GetShowFlag()&SHOW_AUTOMATION?true:false,false);
	}
	else
	{
		// Track Recording
		trackobjects.DeleteAllO(list);

		Seq_Track *t=editor->WindowSong()->FirstTrack();
		while(t)
		{
			if(t->CheckTracking(mainsettings->arrangetracking[editor->set])==true)
			{
				trackobjects.AddCooObject(t,zoomy,0);
			}

			t=t->NextTrack();
		}

		trackobjects.EndBuild();
	}
}

void Edit_ArrangeList::ShowVSlider()
{
	// Show Slider
	if(vertgadget)
		vertgadget->ChangeSlider(&trackobjects,maingui->GetFontSizeY());

	//vertgadget->ChangeSliderPage(numberoftracks);
}

Edit_ArrangeList_Track::Edit_ArrangeList_Track()
{
	id=OBJECTID_TRACKLIST;
	namestring=0;
	focus=false;

	track=0;
	channel=0;
}

Edit_ArrangeList_AutomationTrack::Edit_ArrangeList_AutomationTrack()
{
	id=OBJECTID_AUTOMATIONTRACKLIST;
	namestring=0;
}

void Edit_ArrangeList_Track::ShowChildOpenClose()
{
	if(!track)
		return;

	if(track->FirstChildTrack())
	{
		bitmap->ShowChildTrackMode(editor->list->GetTabX(TABL_CHILD),y,editor->list->GetTabX2(TABL_CHILD),y2,track);
	}
}

void Edit_ArrangeList_Track::ShowColour()
{
	if(!track)
		return;

	if(track->GetColour()->showcolour==true)
	{
		bitmap->guiFillRect_RGB(editor->list->GetTabX(TABL_COLOUR),y,editor->list->GetTabX2(TABL_COLOUR),y2,track->GetColour()->rgb);
	}
}

void Edit_ArrangeList_Track::ShowRecordSettings()
{
	if(!track)
		return;

	for(int i=0;i<5;i++)
	{
		recs[i]=track->recordsettings_record[i];

		int col=track->recordsettings_record[i]==true?COLOUR_RECSETTINGSON:COLOUR_GREY_DARK;

		bitmap->guiDrawCircle(editor->list->GetTabX(TABR_RECORDS1+i)+1,y+1,editor->list->GetTabX2(TABR_RECORDS1+i)-1,y2-1,col);
	}

	/*
	if(record_status==true)
	col=track->FirstChildTrack()?COLOUR_RECORDCHILDS:COLOUR_RECORD;
	else
	col=COLOUR_GREY;
	*/

}

void  Edit_ArrangeList_Track::ShowAudioDisplay(bool force)
{
	bool forceblit=false;
	bool tinputmonitoring;

	if(track)
	{
		tinputmonitoring=track->io.tempinputmonitoring;

		if(tinputmonitoring==true && track->io.inputmonitoring==false && (editor->WindowSong()->status&Seq_Song::STATUS_PLAY))
			tinputmonitoring=false;
	}
	else
		tinputmonitoring=false;

	if(force==false && outpeak.inputmonitoring!=tinputmonitoring)
	{
		force=true;
		forceblit=true;
	}

	outpeak.showactivated=false;
	outpeak.force=force;
	outpeak.peak=track?track->GetPeak():channel->GetPeak();

	int	flag=0,audiox=editor->list->GetTabX(TABL_VU)+1;

	outpeak.channel=channel;
	outpeak.track=track;

	outpeak.x=audiox;
	outpeak.x2=editor->list->GetTabX2(TABL_VU);
	outpeak.y=y+1;
	outpeak.y2=y2-1;

	outpeak.active=mainvar->GetActiveSong()==editor->WindowSong()?true:false;
	outpeak.bitmap=bitmap;

	outpeak.ShowInit(true);
	outpeak.ShowPeak();

	//	audiodisplay=outpeak.current_sum;
	//	audiodisplay_max=outpeak.current_max;

	if(forceblit==true || (force==false && outpeak.changed==true))
	{
		editor->list->Blt(outpeak.x,y+1,outpeak.x2,y2-1);
	}
}

void Edit_ArrangeList_Track::ShowRecordStatus(bool force)
{
	if(!track)
		return;

	if(track->ismetrotrack==true)
		return;

	if(force==true || track->record!=record_status || track->recordtracktype!=record_tracktype)
	{
		record_status=track->record;
		record_tracktype=track->recordtracktype;

		bitmap->guiFillRect(editor->list->GetTabX(TABL_RECORD),y,editor->list->GetTabX2(TABL_RECORD),y2,bgcolour);

		int col;

		if(record_status==true)
			col=track->FirstChildTrack()?COLOUR_RECORDCHILDS:COLOUR_RECORD;
		else
			col=COLOUR_GREY;

		bitmap->guiDrawCircle(editor->list->GetTabX(TABL_RECORD)+1,y+1,editor->list->GetTabX2(TABL_RECORD)-1,y2-1,col);

		if(track->recordtracktype==TRACKTYPE_MIDI)
		{
			bitmap->SetTextColour(col==COLOUR_GREY?COLOUR_BLACK:COLOUR_TEXTCONTROL);

			int midy=y2-y;
			midy/=2;
			midy+=y;
			midy-=maingui->GetFontSizeY()/2;

			bitmap->guiDrawTextCenter(editor->list->GetTabX(TABL_RECORD)+1,midy,editor->list->GetTabX2(TABL_RECORD)-1,midy+maingui->GetFontSizeY(),"m");
		}

		if(force==false)
			editor->list->Blt(editor->list->GetTabX(TABL_RECORD),y,editor->list->GetTabX2(TABL_RECORD),y2);
	}
}

void Edit_ArrangeList_Track::ShowMute(bool force)
{
	TrackHead *th;

	if(track)
	{
		th=track;
	}
	else
		th=channel;

	if(force==true || mute!=th->io.audioeffects.mute.mute)
	{
		mute=th->io.audioeffects.mute.mute;

		bitmap->ShowMute(editor->list->GetTabX(TABL_MUTE),y,editor->list->GetTabX2(TABL_MUTE),y2,mute,focus==true?COLOUR_WHITE:COLOUR_GREY);

		if(force==false)
			editor->list->Blt(editor->list->GetTabX(TABL_MUTE),y,editor->list->GetTabX2(TABL_MUTE),y2);
	}
}

void Edit_ArrangeList_Track::ShowTrackInfo(bool force)
{
	if(channel)
	{
		if(force==true ||
			hasinstruments!=channel->io.audioeffects.CheckIfEffectHasOnInstruments() ||
			hasfx!=channel->io.audioeffects.CheckIfEffectHasNonInstrumentFX()
			)
		{
			hasinstruments=channel->io.audioeffects.CheckIfEffectHasOnInstruments();
			hasfx=channel->io.audioeffects.CheckIfEffectHasNonInstrumentFX();

			bitmap->SetFont(channel->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);

			char *f=0;
			int col=COLOUR_GREY;

			bitmap->SetTextColour(COLOUR_BLACK);

			if(hasinstruments==true)
			{
				if(hasfx==true)
					f="iX";
				else
					f="i";
			}
			else
			{
				if(hasfx==true)
					f="X";
			}

			if(f)
			{
				bitmap->guiFillRect(editor->list->GetTabX(TABL_FREEZE),y,editor->list->GetTabX2(TABL_FREEZE),y2,col,COLOUR_INFOBORDER);
				bitmap->guiDrawText(editor->list->GetTabX(TABL_FREEZE),y2,editor->list->GetTabX2(TABL_FREEZE),f);
			}
			else
				bitmap->guiFillRect(editor->list->GetTabX(TABL_FREEZE),y,editor->list->GetTabX2(TABL_FREEZE),y2,bgcolour);

			if(force==false)
				editor->list->Blt(editor->list->GetTabX(TABL_FREEZE),y,editor->list->GetTabX2(TABL_FREEZE),y2);
		}
	}
	else
		if(track && track->ismetrotrack==false)
		{
			if(force==true ||
				frozen!=track->frozen ||
				hasinstruments!=track->io.audioeffects.CheckIfEffectHasOnInstruments() ||
				hasfx!=track->io.audioeffects.CheckIfEffectHasNonInstrumentFX()
				)
			{
				frozen=track->frozen;
				hasinstruments=track->io.audioeffects.CheckIfEffectHasOnInstruments();
				hasfx=track->io.audioeffects.CheckIfEffectHasNonInstrumentFX();

				bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);

				char *f=0;
				int col;

				if(track->frozen==true)
				{
					col=COLOUR_FROZEN;

					bitmap->SetTextColour(COLOUR_WHITE);

					if(hasinstruments==true)
					{
						if(hasfx==true)
							f="*iX";
						else
							f="*i";
					}
					else
					{
						if(hasfx==true)
							f="*X";
						else
							f="*";
					}
				}
				else
				{
					col=COLOUR_GREY;

					bitmap->SetTextColour(COLOUR_BLACK);

					if(hasinstruments==true)
					{
						if(hasfx==true)
							f="iX";
						else
							f="i";
					}
					else
					{
						if(hasfx==true)
							f="X";
					}
				}

				if(f)
				{
					bitmap->guiFillRect(editor->list->GetTabX(TABL_FREEZE),y,editor->list->GetTabX2(TABL_FREEZE),y2,col,COLOUR_INFOBORDER);
					bitmap->guiDrawText(editor->list->GetTabX(TABL_FREEZE),y2,editor->list->GetTabX2(TABL_FREEZE),f);
				}
				else
					bitmap->guiFillRect(editor->list->GetTabX(TABL_FREEZE),y,editor->list->GetTabX2(TABL_FREEZE),y2,bgcolour);

				if(force==false)
					editor->list->Blt(editor->list->GetTabX(TABL_FREEZE),y,editor->list->GetTabX2(TABL_FREEZE),y2);
			}

		}
}

void Edit_ArrangeList_Track::ShowSolo(bool force)
{
	int status;

	TrackHead *th;

	if(track)
	{
		if(track->ismetrotrack==true)
			return;

		th=track;
		status=track->GetSoloStatus();

	}
	else
	{
		if(channel->audiochannelsystemtype!=CHANNELTYPE_BUSCHANNEL)
			return;

		th=channel;
		status=channel->GetSoloStatus();
	}

	if(force==true || solostatus!=status)
	{
		solostatus=status;

		bitmap->ShowSolo(editor->list->GetTabX(TABL_SOLO),y,editor->list->GetTabX2(TABL_SOLO),y2,status,focus==true?COLOUR_WHITE:COLOUR_GREY);

		if(force==false)
			editor->list->Blt(editor->list->GetTabX(TABL_SOLO),y,editor->list->GetTabX2(TABL_SOLO),y2);
	}

}

void Edit_ArrangeList_Track::ShowAutomation(bool force)
{
	if(editor->mode==true)
		return;

	TrackHead *th;

	if(track)
	{
		if(track->ismetrotrack==true)
			return;

		th=track;
	}
	else
		th=channel;

	if(force==true || automationonoff!=th->automationon || automationused!=th->HasAutomation())
	{
		automationonoff=th->automationon;
		automationused=th->HasAutomation();

		if(editor->editor->GetShowFlag()&SHOW_AUTOMATION)
		{
			bitmap->ShowAutomationSettings(editor->list->GetTabX(TABL_AUTOMATION),y,editor->list->GetTabX2(TABL_AUTOMATION),y2,th);

			if(force==false)
				editor->list->Blt(editor->list->GetTabX(TABL_AUTOMATION),y,editor->list->GetTabX2(TABL_AUTOMATION),y2);
		}
	}
}

void Edit_ArrangeList_Track::ShowAutomationMode()
{
	if(editor->mode==true)
		return;

	TrackHead *th;

	if(track)
	{
		if(track->ismetrotrack==true)
			return;

		th=track;
	}
	else
		th=channel;

	if(editor->editor->GetShowFlag()&SHOW_AUTOMATION)
		bitmap->ShowAutomationMode(editor->list->GetTabX(TABL_AUTOMATIONUPDOWN),y,editor->list->GetTabX2(TABL_AUTOMATIONUPDOWN),y2,th);
}

void  Edit_ArrangeList_Track::ShowMIDIDisplay(bool force)
{
	if(!track)
		return;

	m_outpeak.channel=0;
	m_outpeak.track=track;
	m_outpeak.bitmap=bitmap;
	m_outpeak.x=editor->list->GetTabX(TABL_MIDI);
	m_outpeak.y=y+1;
	m_outpeak.x2=editor->list->GetTabX2(TABL_MIDI);
	m_outpeak.y2=y2-1;
	m_outpeak.force=force;
	m_outpeak.noMIDItext=true;

	m_outpeak.ShowInit(false);
	m_outpeak.ShowPeak();

	if(force==false && m_outpeak.changed==true)
		editor->list->Blt(m_outpeak.x,y+1,m_outpeak.x2,y2-1);
}

void Edit_ArrangeList_Track::ShowNumber()
{
	bitmap->SetTextColour(COLOUR_TEXTCONTROL);

	bitmap->guiFillRect(editor->list->GetTabX(TABL_NUMBER),y,editor->list->GetTabX2(TABL_NUMBER),y2,bgcolour);

	if(channel)
	{
		if(channel->audiochannelsystemtype==CHANNELTYPE_MASTER)
			bitmap->guiDrawText(editor->list->GetTabX(TABL_NUMBER),y2,editor->list->GetTabX2(TABL_NUMBER),"Mtr");
		else
			bitmap->guiDrawNumber(editor->list->GetTabX(TABL_NUMBER),y2,editor->list->GetTabX2(TABL_NUMBER),channel->GetChannelIndex()+1);
	}
	else
	{
		if(track->ismetrotrack==false)
		{
			if(track==editor->WindowSong()->GetFocusTrack())
			{
				bitmap->SetFont(&maingui->standard_bold);
				focus=true;
			}
			else
				bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);

			bitmap->guiDrawNumber(editor->list->GetTabX(TABL_NUMBER),y2,editor->list->GetTabX2(TABL_NUMBER),track->GetTrackIndex()+1);
		}
	}
}

void Edit_ArrangeList_Track::ShowName(bool force)
{
	if(namestring)
	{
		delete namestring;
		namestring=0;
	}

	bitmap->SetTextColour(COLOUR_TEXTCONTROL);

	if(channel)
	{
		bitmap->SetTextColour(COLOUR_BLACK);

		if(channel==editor->WindowSong()->audiosystem.GetFocusBus())
		{
			bitmap->SetFont(&maingui->standard_bold);
			focus=true;
		}
		else
		{
			bitmap->SetFont(&maingui->standardfont);
		}
	}
	else
	{
		if(track==editor->WindowSong()->GetFocusTrack())
		{
			bitmap->SetTextColour(COLOUR_FOCUSOBJECT);
			bitmap->SetFont(&maingui->standard_bold);
			focus=true;
		}
		else
		{
			bitmap->SetTextColour(COLOUR_TEXT);
			bitmap->SetFont(/*track->IsSelected()==true?&maingui->standard_bold:*/&maingui->standardfont);
		}
	}

	int tabx=editor->mode==false?TABL_NAME:TABR_NAME;

	bitmap->guiFillRect(editor->list->GetTabX(tabx),y,editor->list->GetTabX2(tabx),y2,bgcolour);
	namestring=mainvar->GenerateString(track?track->GetName():channel->GetFullName());
	bitmap->guiDrawTextFontY(editor->list->GetTabX(tabx),y2,editor->list->GetTabX2(tabx),namestring);

	if(force==false)
		editor->list->Blt(editor->list->GetTabX(tabx),y,editor->list->GetTabX2(tabx),y2);
}

void Edit_ArrangeList_Track::ShowTrack(bool refreshbgcolour)
{	
	if(channel)
	{
		if(channel->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL)
			bgcolour=channel->IsSelected()==true?COLOUR_BUSCHANNELSELECTED:COLOUR_BUSCHANNEL;
		else
			bgcolour=COLOUR_MASTERCHANNEL;
	}
	else
	{
		if(track->ismetrotrack==true)
			bgcolour=COLOUR_METROTRACK;
		else
			if(track->IsSelected()==true)
				bgcolour=COLOUR_SELECTEDARTRACK;
			else
				bgcolour=track->parent?COLOUR_BACKGROUNDCHILD:COLOUR_GADGETBACKGROUND;
	}

	bitmap->guiFillRectX0(y,editor->list->GetX2(),y2,bgcolour);

	ShowNumber();
	ShowName(true);
	ShowChildOpenClose();
	ShowColour();

	ShowRecordStatus(true);
	ShowAudioDisplay(true);
	ShowMIDIDisplay(true);

	if(editor->mode==false)
	{
		ShowAutomation(true);
		ShowAutomationMode();
	}
	else
		ShowRecordSettings();

	ShowMute(true);
	ShowSolo(true);
	ShowTrackInfo(true);
}

void Edit_ArrangeList_AutomationTrack::ShowName()
{
	if(namestring)delete namestring;
	namestring=0;

	int bgcolour;

	if(track==editor->WindowSong()->GetFocusTrack())
	{
		bitmap->SetFont(&maingui->standard_bold);
		bitmap->SetTextColour(COLOUR_FOCUSOBJECT);
	}
	else
	{
		//bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
		bitmap->SetTextColour(COLOUR_TEXTCONTROL);
	}

	if(track)
		bgcolour=track->parent?COLOUR_BACKGROUNDCHILD:COLOUR_GADGETBACKGROUND;
	else
		bgcolour=COLOUR_GADGETBACKGROUND;

	bitmap->guiFillRect(x,y,x2,y2,bgcolour);

	bitmap->SetTextColour(COLOUR_AUTOMATIONTRACKS);
	bitmap->guiDrawTextFontY(editor->list->GetTabX(TABL_NAME),y2,editor->list->GetTabX2(TABL_NAME),automationtrack->GetAutomationTrackName());
}

void Edit_ArrangeList_AutomationTrack::ShowTrack(bool refreshbgcolour)
{
	ShowName();
}

void Edit_ArrangeList::ShowList()
{
	guiobjects.RemoveOs(0);	
	if(!list)return;

	BuildTrackList();
	ShowVSlider();

	trackobjects.InitYStartO();
	list->ClearTab();

	TRACE ("SL STARTY %d\n",trackobjects.starty);

	if(trackobjects.GetShowObject()) // first track ?
	{
		// Create Track List
		while(trackobjects.GetShowObject() && trackobjects.GetInitY()<list->GetHeight())
		{
			switch(trackobjects.GetShowObject()->object->id)
			{
			case OBJ_AUDIOCHANNEL: // Master/Bus
				{
					AudioChannel *ac=(AudioChannel *)trackobjects.GetShowObject()->object;

					if(Edit_ArrangeList_Track *et=new Edit_ArrangeList_Track)
					{
						et->trackselected=ac->IsSelected();
						et->startnamex=0;
						et->editor=this;
						et->bitmap=&list->gbitmap;
						et->channel=ac;

						guiobjects.AddTABGUIObject(0,trackobjects.GetInitY(),list->GetWidth(),trackobjects.GetInitY2(),list,et);
					} 
				}
				break;

			case OBJ_TRACK:
				{
					Seq_Track *t=(Seq_Track *)trackobjects.GetShowObject()->object;

					if(Edit_ArrangeList_Track *et=new Edit_ArrangeList_Track)
					{
						et->trackselected=t->IsSelected();
						et->startnamex=0;
						et->editor=this;
						et->bitmap=&list->gbitmap;
						et->track=t;

						guiobjects.AddTABGUIObject(0,trackobjects.GetInitY(),list->GetWidth(),trackobjects.GetInitY2(),list,et);
					} 
				}
				break;

			case OBJ_AUTOMATIONTRACK:
				{
					AutomationTrack *at=(AutomationTrack *)trackobjects.GetShowObject()->object;

					if(Edit_ArrangeList_AutomationTrack *eat=new Edit_ArrangeList_AutomationTrack)
					{
						eat->startnamex=0;
						eat->editor=this;
						eat->bitmap=&list->gbitmap;

						eat->track=at->track;
						eat->channel=at->audiochannel;

						eat->automationtrack=at;

						guiobjects.AddTABGUIObject(0,trackobjects.GetInitY(),list->GetWidth(),trackobjects.GetInitY2(),list,eat);	
					}
				}
				break;
			}

			trackobjects.NextYO();

		}// while list

		guiObject_Pref *o=list->FirstGUIObjectPref();
		while(o)
		{
			switch(o->gobject->id)
			{
			case OBJECTID_TRACKLIST:
				{
					Edit_ArrangeList_Track *et=(Edit_ArrangeList_Track *)o->gobject;
					et->ShowTrack(false);
				}
				break;

			case OBJECTID_AUTOMATIONTRACKLIST:
				{
					Edit_ArrangeList_AutomationTrack *eat=(Edit_ArrangeList_AutomationTrack *)o->gobject;
					eat->ShowTrack(false);
				}
				break;
			}

			o=o->NextGUIObjectPref();
		}

	}// if t

	trackobjects.DrawUnUsed(list);
}

void Edit_ArrangeList::FreeEditorMemory()
{
	guiobjects.RemoveOs(0);
	trackobjects.DeleteAllO(0);
}

void Edit_ArrangeList::DeInitWindow()
{	
	FreeEditorMemory();
}

void Edit_ArrangeList::RefreshObjects(LONGLONG type,bool editcall)
{
	DrawDBBlit(list);
}

void Edit_ArrangeList::RefreshRealtime()
{
	guiObject_Pref *o=list->FirstGUIObjectPref();

	while(o)
	{
		switch(o->gobject->id)
		{
		case OBJECTID_TRACKLIST:
			{
				Edit_ArrangeList_Track *et=(Edit_ArrangeList_Track *)o->gobject;

				if(et->track)
				{
					if(et->trackselected!=et->track->IsSelected())
					{
						DrawDBBlit(list);
						return;
					}

					bool isfocus=et->track==WindowSong()->GetFocusTrack()?true:false;

					if(et->focus!=isfocus)
					{
						DrawDBBlit(list);
						return;
					}
				}
				else if(et->channel)
				{
					if(et->trackselected!=et->channel->IsSelected())
					{
						DrawDBBlit(list);
						return;
					}
				}


				if(((et->namestring && strcmp(et->track?et->track->GetName():et->channel->GetFullName(),et->namestring)!=0))
					)
					et->ShowName(false);

				if(mode==true)
				{
					for(int i=0;i<5;i++)
					{
						if(et->track->recordsettings_record[i]!=et->recs[i])
						{
							DrawDBBlit(list);
							return;
						}
					}
				}
				else
					et->ShowAutomation(false);


				et->ShowAudioDisplay(false);
				et->ShowMIDIDisplay(false);
				et->ShowRecordStatus(false);
				et->ShowMute(false);
				et->ShowSolo(false);
				et->ShowTrackInfo(false);

			}
			break;
		}

		o=o->NextGUIObjectPref();
	}
}