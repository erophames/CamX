#include "defines.h"
#include "editor.h"
#include "editor_event.h"
#include "sampleeditor.h"
#include "audiomixeditor.h"
#include "arrangeeditor.h"
#include "pianoeditor.h"
#include "waveeditor.h"
#include "drumeditor.h"
#include "tempomapeditor.h"
#include "editor_marker.h"
#include "editor_text.h"
#include "patternselection.h"
#include "object_song.h"
#include "songmain.h"
#include "editbuffer.h"
#include "seqtime.h"
#include "camxgadgets.h"
#include "gui.h"
#include "semapores.h"
#include "globalmenus.h"
#include "editfunctions.h"
#include "languagefiles.h"
#include "object_project.h"
#include "editdata.h"
#include "undofunctions_tempo.h"
#include "editortabs.h"

void EventEditor::AddEditorSlider(SliderCo *hor,SliderCo *vert,bool datazoom)
{
	mouseqgadget=0;
	syncgadget=horzgadget=horzzoomgadget=0;
	datazoomgadget=0;

	if(hor)
	{
		int addh;
		int w=2*bitmap.GetTextWidth("Zoom 1");

		/// Horiz
		guiForm_Child *f=glist.SelectForm(hor->formx,hor->formy);

		double h=w*2;

		h*=0.6;

		addh=h;

		mouseqgadget=glist.AddButton(-1,-1,(int)h,-1,MouseQuantizeString(),GADGETID_EDITOR_MOUSEQUANTIZE,MODE_TOP|MODE_BOTTOM,Cxs[CXS_MOUSEQUANTIZEANDTIME]);

		h=w*2;
		h*=0.4;

		addh+=h;

		glist.AddLX();
		syncgadget=glist.AddButton(-1,-1,(int)h,-1,"ED Sync",GADGETID_EDITOR_SYNC,MODE_TOGGLE,Cxs[CXS_SNYCEDITOR]);
		glist.AddLX();

		if(datazoom==true)
		{
			addh+=h;
			datazoomgadget=glist.AddNumberButton(-1,-1,(int)h,-1,GADGETID_EDITOR_DATAZOOM,100,500,100,NUMBER_INTEGER_PERCENT,0,"Data Zoom");
			glist.AddLX();
			hor->x=addh+3*ADDXSPACE;
		}
		else
			hor->x=addh+2*ADDXSPACE;
		hor->y=0;
		hor->x2=0;
		hor->y2=0;
		hor->from=0;
		hor->to=0;
		hor->pos=0;
		hor->horz=true;
		hor->subw=SIZE_HZOOM_SLIDER+1;

		horzgadget=glist.AddSlider(hor,GADGETID_EDITORSLIDER_HORIZ,MODE_TOP|MODE_BOTTOM|MODE_RIGHT,0,0);

		if(horzgadget)
		{
			RefreshTimeSlider();

			hor->x=0;
			hor->x2=0;

			hor->from=0;
			hor->to=mainvar->numberwinzooms-1;
			hor->pos=zoom->index;
			hor->horz=true;
			hor->page=1;
			hor->staticwidth=SIZE_HZOOM_SLIDER;
			hor->subh=hor->subw=0;

			horzzoomgadget=glist.AddSlider(hor,GADGETID_EDITORSLIDER_HORIZZOOM,MODE_TOP|MODE_BOTTOM|MODE_RIGHT|MODE_STATICWIDTH,0,"Zoom <->");
		}
	}

	vertgadget=vertzoomgadget=0;

	if(vert)
	{
		// Vertical
		guiForm_Child *f=glist.SelectForm(vert->formx,vert->formy);

		vert->x=0;
		vert->y=0;
		vert->x2=0;
		vert->y2=0;

		vert->horz=false;
		vert->subh=vert->nozoom==true?0:SIZE_VZOOM_SLIDER+1;

		vertgadget=glist.AddSlider(vert,GADGETID_EDITORSLIDER_VERT,MODE_LEFT|MODE_TOP|MODE_RIGHT|MODE_BOTTOM,0,0);

		if(vertgadget && vert->nozoom==false)
		{
			vert->y=0;
			vert->y2=0;

			if(minzoomy>=0)
			{
				vert->from=minzoomy;
				vert->to=ZOOMY_MAX;
			}
			else
			{
				vert->from=ZOOMY_MIN;
				vert->to=ZOOMY_MAX;
			}

			vert->page=10;
			vert->pos=zoomy;
			vert->staticheight=SIZE_VZOOM_SLIDER;
			vert->subh=vert->subw=0;

			vertzoomgadget=glist.AddSlider(vert,GADGETID_EDITORSLIDER_VERTZOOM,MODE_LEFT|MODE_BOTTOM|MODE_RIGHT|MODE_STATICHEIGHT,0,"Zoom Up/Down");
		}
	}


}

void EventEditor::ClearMouseWindow()
{	
	/*
	if(tooltip.init==true)
	{
	ClearTooltip();

	#ifdef WIN32
	DeleteDC(tooltip.hDC);
	DeleteObject(tooltip.hBitmap);
	#endif	
	tooltip.init=false;
	}
	*/
}

void EventEditor::RefreshToolTip()
{
	/*
	if(tooltip.init==true && tooltip.ondisplay==true)
	{
	BlitBitMap(&tooltip,tooltipx,tooltipy,tooltipw,tooltiph);
	}
	*/
}

void EventEditor::ToolTipOff()
{
	//tooltip.ondisplay=false;
}

void EventEditor::ClearTooltip()
{
	/*
	if(tooltip.init==true && editframe && guibuffer && tooltip.ondisplay==true)
	{
	if(tooltipx>=editframe->x && tooltipy>=editframe->y && tooltipx+tooltipw<=editframe->x2 && tooltipy+tooltiph<=editframe->y2)
	{
	BltGUIBuffer
	(
	editframe->SetToX(tooltipx),
	editframe->SetToY(tooltipy),
	editframe->SetToX(tooltipx+tooltipw),
	editframe->SetToY(tooltipy+tooltiph)
	);
	}
	}

	ToolTipOff();
	*/
}

void EventEditor::DisplayTooltip(OSTART time)
{
	/*
	ClearTooltip();

	if(time<0)return;

	int sizew=120;
	int sizeh=maingui->GetFontSizeY();

	if(editframe && guibuffer && mainsettings->displayeditortooltip==true)
	{
	char *text2=GetToolTipString1();
	char *text3=GetToolTipString2();

	if(tooltip.init==false)
	CreateCompatibleBitMap(&tooltip,sizew+8,(maingui->GetFontSizeY()*3)+16);

	if(text2)sizeh+=maingui->GetFontSizeY()+2;
	if(text3)sizeh+=maingui->GetFontSizeY()+2;

	int tox=GetMouseX()+20,toy=GetMouseY();
	bool ok=false;

	if(tox+sizew>editframe->x2)
	tox=GetMouseX()-sizew;

	if(toy+sizeh>editframe->y2)
	toy=GetMouseY()-sizeh;

	if(tooltip.init==true && tox>=editframe->x && tox+sizew<=editframe->x2 && toy>=editframe->y && toy+sizeh<=editframe->y2)
	{
	tooltip.guiFillRectX0(0,sizew,sizeh,COLOUR_GREY_LIGHT);
	tooltip.guiDrawRect(0,0,sizew-1,sizeh-1,COLOUR_BLACK); // border

	int mode=Seq_Pos::POSMODE_NORMAL;

	switch(windowdisplay)
	{
	case WINDOWDISPLAY_SMPTE:
	mode=WindowSong()->project->standardsmpte;
	break;
	}

	// 1. Time
	int y=maingui->GetFontSizeY();

	TimeString timestring;

	WindowSong()->timetrack.CreateTimeString(&timestring,time,mode);

	tooltip.SetAPen(COLOUR_BLACK);
	tooltip.guiDrawText(0,y,sizew,timestring.string);

	// 2. Text2
	if(text2){
	y=maingui->AddFontY(y);
	tooltip.guiDrawText(4,y,sizew,text2);
	}

	//3. Text3
	if(text3){
	y=maingui->AddFontY(y);
	tooltip.guiDrawText(4,y,sizew,text3);
	}

	tooltip.ondisplay=true;
	BlitBitMap(&tooltip,tooltipx=tox,tooltipy=toy,tooltipw=sizew,tooltiph=sizeh);
	}

	if(text2)delete text2;
	if(text3)delete text3;
	}
	*/
}

void EventEditor::DeActivated()
{
	SetEditorMode(EM_RESET);
}

void EventEditor::RefreshMeasure()
{
	switch(windowtimeformat)
	{
	case WINDOWDISPLAY_MEASURE:
		RefreshObjects(0,false);
		guictrlbox.ShowTime();
		break;
	}
}

void EventEditor::RefreshSMPTE()
{
	guitoolbox.ShowTimeType();

	if(timeline && (mainsettings->showbothsformatsintimeline==true || windowtimeformat==WINDOWDISPLAY_SMPTE))
		timeline->Draw();

	switch(windowtimeformat)
	{
	case WINDOWDISPLAY_SMPTE:
		RefreshObjects(0,false);
		guictrlbox.ShowTime();
		break;
	}
}

void EventEditor::RefreshTimeLine()
{
	if(timeline)
		timeline->Draw();
}

void EventEditor::DeleteMouseTooltip()
{
	ClearTooltip();
}

void EventEditor_Selection::EditSelectedEventsDelta_Tab(int edittabx)
{
	//if(!g)return;

	Seq_Event *ee=(Seq_Event *)editobject;

	if(!ee)return;

	if(editsum==0)return;
	if(editsum>0 && editsum<1)return;
	if(editsum<0 && editsum>-1)return;

	bool changed=false;

	Seq_SelectionEvent *se=patternselection.FirstSelectedEvent();

	while(se)
	{
		Seq_Event *e=se->seqevent;

		if(e->GetStatus()==ee->GetStatus())
		{
			int setbyte1=-1,setbyte2=-1;

			switch(edittabx)
			{
			case TAB_STATUS:
				break;

			case TAB_LEN:
				{
					switch(e->GetStatus())
					{
					case NOTEON:
						{
							Note *note=(Note *)e;

							switch(windowtimeformat)
							{
							case WINDOWDISPLAY_MEASURE:
								{
									WindowSong()->timetrack.ConvertTicksToLength(note->GetEventStart(),note->GetNoteLength(),displayusepos);

									switch(edit_lenidenx)
									{
									case 0:
										displayusepos->AddStartPositionEditing((int)editsum);
										break;

									case 1:
										displayusepos->AddBeatLength((int)editsum);
										break;

									case 2:
										displayusepos->AddZoomTicksLength((int)editsum);
										break;

									case 3:
										displayusepos->AddTicksLength((int)editsum);
										break;
									}

									OSTART h=WindowSong()->timetrack.ConvertPosToLength(note->GetEventStart(),displayusepos);

									if(h!=note->GetNoteLength() && h>0)
									{
										mainthreadcontrol->LockActiveSong();
										note->SetLength(h);
										mainthreadcontrol->UnlockActiveSong();

										changed=true;
									}
								}
								break;

							case WINDOWDISPLAY_SAMPLES:
								{
									LONGLONG sampless=WindowSong()->timetrack.ConvertTicksToTempoSamples(note->GetEventStart());
									LONGLONG samplese=WindowSong()->timetrack.ConvertTicksToTempoSamples(note->GetNoteEnd());

									LONGLONG samples=samplese-sampless;

									samples+=(LONGLONG)editsum;

									OSTART h=WindowSong()->timetrack.ConvertSamplesToTicks(samples);

									if(h!=note->GetNoteLength() && h>0)
									{
										mainthreadcontrol->LockActiveSong();
										note->SetLength(h);
										mainthreadcontrol->UnlockActiveSong();

										changed=true;
									}
								}
								break;

							case WINDOWDISPLAY_SECONDS:
							case WINDOWDISPLAY_SMPTE:
								{
									WindowSong()->timetrack.ConvertTicksToPos(note->GetNoteEnd(),displayusepos);

									switch(edit_lenidenx)
									{
									case 0:
										displayusepos->AddHour((LONGLONG)editsum);
										break;

									case 1:
										displayusepos->AddMin((LONGLONG)editsum);
										break;

									case 2:
										displayusepos->AddSec((LONGLONG)editsum);
										break;

									case 3:
										if(windowtimeformat==WINDOWDISPLAY_SECONDS)
										{
											displayusepos->AddSec100((int)editsum);
										}
										else
											displayusepos->AddFrame((LONGLONG)editsum);
										break;

									case 4:
										displayusepos->AddQuarterFrame((LONGLONG)editsum);
										break;
									}

									OSTART newend=WindowSong()->timetrack.ConvertPosToTicks(displayusepos);
									OSTART newlength=newend-note->GetEventStart();

									if(newlength!=note->GetNoteLength())
									{
										mainthreadcontrol->LockActiveSong();
										note->SetLength(newlength);
										mainthreadcontrol->UnlockActiveSong();

										changed=true;
									}
								}
								break;
							}
						}
						break;

					}
				}break;

			case TAB_CHANNEL:
				{
					int c=e->GetChannel();
					int oc=c;

					if(c!=-1)
					{
						c+=editsum;

						if(c>=0 && c<16 && c!=oc)
						{
							mainthreadcontrol->LockActiveSong();
							e->SetChannel(c);
							mainthreadcontrol->UnlockActiveSong();

							changed=true;
						}
					}
				}
				break;

			case TAB_BYTE1:
				{
					bool ok=false;

					switch(e->GetStatus())
					{
					case INTERN:
						{
							if(ee->GetICD()==e->GetICD()) // Velocity
								ok=true;
						}
						break;

						case INTERNCHAIN:
						{
							if(ee->GetICD()==e->GetICD()) // Velocity
								ok=true;
						}
						break;

					case CONTROLCHANGE:  // Same Controller
						//case POLYPRESSURE: // Same Key
						//case NOTEON: // Same Key
						{
							if(ee->GetByte1()==e->GetByte1())
								ok=true;
						}
						break;

					default:
						ok=true;
						break;
					}

					if(ok==true)
					{
						int c=e->GetByte1();
						int oc=c;

						if(c!=-1)
						{
							c+=editsum;

							if(c>=0 && c<128 && c!=oc)
								setbyte1=c;
						}
					}
				}
				break;

			case TAB_BYTE2:
				{
					bool ok=false;

					switch(e->GetStatus())
					{
					case INTERN:
						{
							if(ee->GetICD()==e->GetICD()) // Velocity Off
								ok=true;
						}
						break;

						case INTERNCHAIN:
						{
							if(ee->GetICD()==e->GetICD()) // Velocity Off
								ok=true;
						}
						break;

					case CONTROLCHANGE:  // Same Controller
						//case POLYPRESSURE: // Same Key
						//case NOTEON: // Same Key
						{
							if(ee->GetByte1()==e->GetByte1())
								ok=true;
						}
						break;

					default:
						ok=true;
						break;
					}

					if(ok==true)
					{
						int c=e->GetByte2();
						int oc=c;

						if(c!=-1)
						{
							c+=editsum;

							if(c>=0 && c<128 && c!=oc)
								setbyte2=c;
						}
					}
				}
				break;

			case TAB_BYTE3:
				{
					int c=e->GetByte3();
					int oc=c;

					if(c!=-1)
					{
						c+=editsum;

						if(c>=0 && c<128 && c!=oc)
						{
							mainthreadcontrol->LockActiveSong();
							e->SetByte3(c);
							mainthreadcontrol->UnlockActiveSong();
							changed=true;
						}
					}
				}
				break;

			case TAB_DATA:
				{
					switch(ee->GetStatus())
					{
					case INTERN:
						break;

					case INTERNCHAIN:
						{
							switch(ee->GetICD())
							{
							case ICD_TYPE_DRUM: // Velocity
								{
									int c=e->GetByte1();
									int oc=c;

									if(c!=-1)
									{
										c+=editsum;

										if(c>=0 && c<128 && c!=oc)
											setbyte1=c;
									}
								}
								break;

							}
						}
						break;

					case CONTROLCHANGE:
						if(ee->GetByte1()==e->GetByte1())
						{
							int c=e->GetByte2();
							int oc=c;

							if(c!=-1)
							{
								c+=editsum;

								if(c>=0 && c<128 && c!=oc)
									setbyte2=c;
							}
						}
						break;

					case NOTEON:
					case POLYPRESSURE:
						{
							int c=e->GetByte2();
							int oc=c;

							if(c!=-1)
							{
								c+=editsum;

								if(c>=0 && c<128 && c!=oc)
									setbyte2=c;
							}
						}
						break;

					case PITCHBEND:
						{
							int c=e->GetByte2();
							int oc=c;

							if(c!=-1)
							{
								c+=editsum;

								if(c>=0 && c<128 && c!=oc)
								{
									setbyte2=c;
									setbyte1=0;
								}
							}
						}
						break;

					case PROGRAMCHANGE:
					case CHANNELPRESSURE:
						{
							int c=e->GetByte1();
							int oc=c;

							if(c!=-1)
							{
								c+=editsum;

								if(c>=0 && c<128 && c!=oc)
									setbyte1=c;
							}
						}
						break;
					}
				}
				break;

				//case TAB_DATA," #Data# ");
			}// switch

			if( (setbyte1>=0 && setbyte1<128) || (setbyte2>=0 && setbyte2<128))
			{
				mainthreadcontrol->LockActiveSong();

				if(setbyte1>=0)
					e->SetByte1(setbyte1);

				if(setbyte2>=0)
					e->SetByte2(setbyte2);

				mainthreadcontrol->UnlockActiveSong();

				changed=true;
			}
		}

		se=se->NextSelectedEvent();
	}

	if(changed==true)
	{
		RefreshObjects(0,true);
		ResetEditSum();
	}
}

bool EventEditor_Selection::OpenEditSelectedEvents(int index,Seq_Event *e,guiGadget_Tab *g)
{
	int editec=0;

	if(e==0)return false;
	if(index==-1)return false;

	if(patternselection.GetCountofSelectedEvents()==0)return false;

	Seq_SelectionEvent *se=patternselection.FirstSelectedEvent();

	while(se)
	{
		Seq_Event *ce=se->seqevent;

		if(e->GetStatus()==ce->GetStatus())
			switch(e->GetStatus())
		{
			case INTERN:
				break;

			case INTERNCHAIN:
				if(e->GetICD()==ce->GetICD())
				{
					switch(e->GetICD())
					{
					case ICD_TYPE_DRUM:
						{
							switch(index)
							{
							case TAB_DATA: // Velocity -> Byte1
							case TAB_BYTE1: // Velocity

							case TAB_BYTE2: // Velocity Off
								editec++;
								break;
							}
						}
						break;
					}
				}
				break;

			case NOTEON:
				{
					switch(index)
					{
					case TAB_CHANNEL:
					case TAB_BYTE1:
					case TAB_BYTE2:
					case TAB_BYTE3:
					case TAB_DATA:
					case TAB_LEN:
						{
							editec++;
						}break;
					}
				}
				break;

			case POLYPRESSURE:
			case PITCHBEND:
			case CONTROLCHANGE:
				{
					switch(g->edittabx)
					{
					case TAB_CHANNEL:
					case TAB_BYTE1:
					case TAB_BYTE2:
						//case TAB_BYTE3:
					case TAB_DATA:
						//case TAB_LEN:
						{
							editec++;
						}break;
					}
				}
				break;

			case PROGRAMCHANGE:
			case CHANNELPRESSURE:
				{
					switch(g->edittabx)
					{
					case TAB_CHANNEL:
					case TAB_BYTE1:
						//case TAB_BYTE2:
						//case TAB_BYTE3:
					case TAB_DATA:
						//case TAB_LEN:
						{
							editec++;
						}break;
					}
				}
				break;

			case SYSEX:
				break;
		}

		se=se->NextSelectedEvent();
	}

	if(editec>0)
	{
		if(UndoEdit *ue=patternselection.CreateEditEvents(e,index))
		{
			editobject=e;

			if(g)
			{
				g->SetEditSteps(0.1);
			}

			SetUndoEditEvents(ue);
			SetEditorMode(EM_EDIT);

			return true;
		}
	}

	return false;
}

bool EventEditor_Selection::OpenEditSelectedEvents_Tab(guiGadget_Tab *g,Seq_Event *e)
{
	int editec=0;

	if(g==0 || e==0)return false;
	if(g->edittabx==-1)return false;

	OpenEditSelectedEvents(g->edittabx,e,g);

	return false;
}

void EventEditor_Selection::SelectEvent(Seq_Event *e,bool nodeselected)
{
	if(nodeselected==false && maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false)
	{
		patternselection.SelectAllEventsNot(e,false,0);	
	}

	if(patternselection.GetCursor() && maingui->GetCtrlKey()==true)
	{
		patternselection.ToggleSelection(e);
	}
	else
		if(patternselection.GetCursor() && maingui->GetShiftKey()==true)
		{
			patternselection.SelectFromTo(patternselection.GetCursor(),e);
		}
		else
			patternselection.SelectEvent(e,true);

	patternselection.SetCursor(e);

}

void EventEditor_Selection::RemoveEvent(Seq_Event *e)
{
	if(patternselection.GetCursor()==e)
		patternselection.SetCursor(0);
}

void EventEditor_Selection::OtherRealtime()
{
	if(patternedit)
	{
		Seq_SelectedPattern *slp=patternselection.FirstSelectedPattern();

		while(slp)
		{
			if(slp->pattern->GetCountOfEvents()!=slp->nrevents ||
				WindowSong()->GetOfTrack(slp->pattern->track)!=slp->trackindex
				)
			{
				ShowSelectionPattern();
				break;
			}

			slp=slp->NextSelectedPattern();
		}
	}
}

void EventEditor_Selection::DeInitWindow()
{
	//patternselection.ClearEventList(); // Delete Event List
	CloseHeader();

	//ClearMouseWindow();

	// Disable Solo Play

	WindowSong()->RemoveFromSoloOnOff(this);
	patternselection.DeleteSelectionList();

	//
	//CloseAllBoundEditors();
}

void EventEditor_Selection::SelectAllEvents()
{
	if(Seq_SelectionEvent *ee=patternselection.FirstMixEvent())
	{
		bool changes=false;

		while(ee)
		{	
			if(ee->seqevent->flag&OFLAG_UNDERSELECTION)
			{
				ee->seqevent->flag CLEARBIT OFLAG_UNDERSELECTION;
				if(ee->seqevent->SelectEvent(true)==true)
					changes=true;
			}

			ee=ee->NextEvent();
		}

		if(changes==true)
			maingui->RefreshAllEditorsWithEvent(WindowSong(),0);
	}
}

void EventEditor_Selection::UnmuteEvents()
{
	if(Seq_Event *e=patternselection.FirstSelectionEvent(0,SEL_SELECTED))
	{
		while(e){
			e->flag CLEARBIT EVENTFLAG_MUTED;
			e=patternselection.NextSelectionEvent(SEL_SELECTED);
		}
	}
}

void EventEditor_Selection::MuteSelectedEvents(bool onoff)
{
	if(patternselection.GetCountofSelectedEvents()) // Selected Events
	{
		if(Seq_Event *e=patternselection.FirstSelectionEvent(0,SEL_SELECTED))
		{
			while(e)
			{
				if(onoff==true)
					e->flag|=EVENTFLAG_MUTED;
				else
					e->flag CLEARBIT EVENTFLAG_MUTED;

				e=patternselection.NextSelectionEvent(SEL_SELECTED);
			}
		}
	}
}

void EventEditor_Selection::CopySelectedEvents()
{
	if(patternselection.GetCountofSelectedEvents()) // Selected Events
	{
		if(Seq_Event *e=patternselection.FirstSelectionEvent(0,SEL_SELECTED))
		{
			mainbuffer->OpenBuffer();

			while(e)
			{
				mainbuffer->AddObjectToBuffer(e,true);
				e=patternselection.NextSelectionEvent(SEL_SELECTED);
			}

			mainbuffer->CloseBuffer();
		}
	}
}

OSTART Seq_Song::QuantizeWithFlag(OSTART start,int qflag)
{
	switch(qflag)
	{
	case MOUSEQUANTIZE_MEASURE: // Measure
		return timetrack.ConvertTicksToMeasureTicks(start,false);

	case MOUSEQUANTIZE_FRAME:
		return timetrack.ConvertTicksToFrameTicks(start);

	case MOUSEQUANTIZE_QFRAME:
		return timetrack.ConvertTicksToQFrameTicks(start);

	case MOUSEQUANTIZE_BEAT:
		return timetrack.ConvertTicksToBeatTicks(start,false);

	case MOUSEQUANTIZE_1:
		return timetrack.ConvertTicksQuantizeTicks(start,TICK1nd);

	case MOUSEQUANTIZE_12:
		return timetrack.ConvertTicksQuantizeTicks(start,TICK2nd);

	case MOUSEQUANTIZE_14:
		return timetrack.ConvertTicksQuantizeTicks(start,TICK4nd);

	case MOUSEQUANTIZE_18:
		return timetrack.ConvertTicksQuantizeTicks(start,TICK8nd);

	case MOUSEQUANTIZE_16:
		return timetrack.ConvertTicksQuantizeTicks(start,TICK16nd);

	case MOUSEQUANTIZE_ZOOM:
		return timetrack.ConvertTicksLeftQuantizeTicks(start,timetrack.zoomticks);
	}

	return start;
}

OSTART EventEditor::QuantizeEditorMouse(OSTART ostart)
{
	if(ostart<0)return 0;

	return WindowSong()->QuantizeWithFlag(ostart,mousequantize);
}

bool EventEditor_Selection::RemoveTrackFromWindow(Seq_Track *t)
{
	bool deleted=false;

	// Find Track in patternselection
	Seq_SelectedPattern *f=patternselection.FirstSelectedPattern();

	while(f)
	{
		if(f->pattern->track==t)
		{
			f=patternselection.DeletePattern(f);
			deleted=true;
		}
		else
			f=f->NextSelectedPattern();
	}

	if(deleted==true)
	{
		if(!patternselection.FirstSelectedPattern()) // No more selected Pattern, Close Window
		{
			switch(GetEditorID())
			{
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					return true; // Close
				}
				break;
			}
		}
		else
		{
			patternselection.patternremoved=true; // refresh Pattern Selection
			RedrawDeletedPattern();
		}
	}

	return false;
}

void EventEditor_Selection::RemovePattern(Seq_Pattern *pattern,bool redraw)
{
	// Remove From Selection
	if(Seq_SelectedPattern *f=patternselection.FindPattern(pattern))
	{
		if(pattern==patternselection.solopattern)
		{
			Seq_SelectedPattern *prevornext=f->NextOrPrevSelectedPattern();
			patternselection.solopattern=prevornext?prevornext->pattern:0;
		}

		patternselection.DeletePattern(f);

		if(!patternselection.FirstSelectedPattern()) // No more selected Pattern, Close Window
			closeit=true;
		else
		{
			patternselection.patternremoved=true; // refresh Pattern Selection

			if(redraw==true)
			{
				refreshflag|=REFRESHEVENT_LIST;
				RedrawDeletedPattern();
			}
		}
	}			
}

void EventEditor::InitDisplay()
{
	space[1]=' ';
	space[2]=0;

	switch(windowtimeformat)
	{
	case WINDOWDISPLAY_SECONDS:
		displayusepos=&pos_seconds;
		break;

	case WINDOWDISPLAY_SAMPLES:
		{
			displayusepos=&pos_samples;
		}
		break;

	case WINDOWDISPLAY_MEASURE:
		{
			displayusepos=&pos_measure;

			switch(WindowSong()->project->projectmeasureformat)
			{
			case PM_1111:
			case PM_1110:
			case PM_11_1:
			case PM_11_0:
				space[0]=' ';
				break;

			default:
				space[0]='.';
			}

			showquarterframe=false;
		}
		break;

	case WINDOWDISPLAY_SMPTE:
		{
			switch(WindowSong()->project->standardsmpte)
			{
			case Seq_Pos::POSMODE_SMPTE_24:
				displayusepos=&pos24;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_25:
				displayusepos=&pos25;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_48:
				displayusepos=&pos48;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_50:
				displayusepos=&pos50;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_2997:
				displayusepos=&pos2997;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_30:
				displayusepos=&pos30;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_2997df:
				displayusepos=&pos2997df;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_30df:
				displayusepos=&pos30df;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_239:
				displayusepos=&pos239;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_249:
				displayusepos=&pos249;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_599:
				displayusepos=&pos599;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_60:
				displayusepos=&pos60;
				space[0]=':';
				showquarterframe=true;
				break;
			}
		}
		break;
	}

	displayusepos->measureformat=WindowSong()->project->projectmeasureformat;
	displayusepos->mode=ConvertWindowDisplayToTimeMode();

	spacesize=GetSizeOfString(space);
}

void EventEditor::PasteMouse(guiGadget_CW *db)
{
	if(!db)return;

	int mx=db->GetMouseX(),my=db->GetMouseY();
	OSTART position;

	if(timeline && mx>=0 && mx<=db->GetX2() && my>=0 && my<=db->GetY2())
	{
		InitMousePosition();
		position=GetMousePosition();
	}
	else
		position=WindowSong()->GetSongPosition();

	mainbuffer->PasteBuffer(this,WindowSong(),insertpattern,position);
}

int EventEditor::GetMouseQuantizeFlag()
{
	return mousequantize;

	/*
	switch(mousequantize)
	{
	case MOUSEQUANTIZE_MEASURE:
	return MOVEPATTERN_FLAGMEASUREQUANTIZE;

	case MOUSEQUANTIZE_1: 
	return MOVEPATTERN_FLAGQUANTIZE1;

	case MOUSEQUANTIZE_12: 
	return MOVEPATTERN_FLAGQUANTIZE2;

	case MOUSEQUANTIZE_14: 
	return MOVEPATTERN_FLAGQUANTIZE4;

	case MOUSEQUANTIZE_18: 
	return MOVEPATTERN_FLAGQUANTIZE8;

	case MOUSEQUANTIZE_16: 
	return MOVEPATTERN_FLAGQUANTIZE16;
	}

	return 0;
	*/
}

void guiWindow::SetAutoScroll(int mode,guiGadget_CW *g)
{
	autoscrollwin=g;
}

void guiWindow::ResetAutoScrolling()
{
	if(autoscrollwin)
	{
		autoscrollwin=0;
	}
}

#define ASCROLLX 10
#define ASCROLLY 10

void guiWindow::AddUndoMenu()
{
	if(menu)
	{
		if(guiMenu *n=menu->AddMenu(Cxs[CXS_EDIT],0))
			maingui->AddUndoMenu(n);
	}
}

void guiWindow::DoStandardYScroll(guiGadget_CW *db)
{
	if(autoscrollwin==db)
	{
		if(autoscrollstatus&AUTOSCROLL_UP)
			AddStartY(-1);

		if(autoscrollstatus&AUTOSCROLL_UPFAST)
			AddStartY(-3);

		if(autoscrollstatus&AUTOSCROLL_UPTURBO)
			AddStartY(-7);

		if(autoscrollstatus&AUTOSCROLL_DOWN)
			AddStartY(1);

		if(autoscrollstatus&AUTOSCROLL_DOWNFAST)
			AddStartY(3);

		if(autoscrollstatus&AUTOSCROLL_DOWNTURBO)
			AddStartY(7);
	}
}

void guiWindow::CheckXButtons()
{
	if(mousexbuttonleftdown==true || mousexbuttonrightdown==true)
	{
		if(xbuttoncounter<=0)
		{
			int bufferkey=nVirtKey;

			nVirtKey=mousexbuttonleftdown==true?KEY_CURSORLEFT:KEY_CURSORRIGHT;

			KeyDownRepeat();

			nVirtKey=bufferkey;
		}

		if(xbuttoncounter==-1)
			xbuttoncounter=5;
		else
			xbuttoncounter++;

		if(xbuttoncounter==4)
			xbuttoncounter=0;

		if(xbuttoncounter==30) // 1. Repeat Delay
			xbuttoncounter=0;
	}
}

void guiWindow::CheckAutoScroll()
{	
	if(autoscrollwin)
	{
		// X
		int mx=autoscrollwin->GetMouseX();

		//	TRACE ("MX %d\n",mx);
		autoscrollstatus=0;

		if(mx<-ASCROLLX)
			autoscrollstatus|=AUTOSCROLL_LEFTTURBO;
		else
			if(mx<0)
				autoscrollstatus|=AUTOSCROLL_LEFTFAST;
			else
				if(mx<ASCROLLX)
					autoscrollstatus|=AUTOSCROLL_LEFT;
				else
					if(mx>autoscrollwin->GetWidth()+ASCROLLX)
						autoscrollstatus|=AUTOSCROLL_RIGHTTURBO;
					else
						if(mx>autoscrollwin->GetWidth())
							autoscrollstatus|=AUTOSCROLL_RIGHTFAST;
						else
							if(mx>autoscrollwin->GetWidth()-ASCROLLX)
								autoscrollstatus|=AUTOSCROLL_RIGHT;	

		if(timeline && autoscrollwin==timeline->dbgadget)
		{
			if(autoscrollstatus)
			{
				DoAutoScroll();
				autoscrollwin->Call(DB_LEFTMOUSEDOWN|DB_MOUSEMOVE);
			}
		}
		else
		{
			int my=autoscrollwin->GetMouseY();

			if(my<-ASCROLLY)
				autoscrollstatus|=AUTOSCROLL_UPTURBO;
			else
				if(my<0)
					autoscrollstatus|=AUTOSCROLL_UPFAST;
				else
					if(my<ASCROLLY)
						autoscrollstatus|=AUTOSCROLL_UP;
					else
						if(my>autoscrollwin->GetHeight()+ASCROLLY)
							autoscrollstatus|=AUTOSCROLL_DOWNTURBO;
						else
							if(my>autoscrollwin->GetHeight())
								autoscrollstatus|=AUTOSCROLL_DOWNFAST;
							else
								if(my>autoscrollwin->GetHeight()-ASCROLLY)
									autoscrollstatus|=AUTOSCROLL_DOWN;

			if(autoscrollstatus)
				AutoScroll();
		}

	}
}

void guiWindow::SetAutoScroll(int mode,int x,int y,int x2,int y2)
{	
	/*
	autoscroll=true;
	autoscrollmode=mode;
	autoscrollx=x;
	autoscrollx2=x2;
	autoscrolly=y;
	autoscrolly2=y2;
	*/
}

int EventEditor::ConvertTimeToOverviewX(OSTART time)
{
	if(overviewlenght==0 || time>=overviewlenght)
		return -1;

	double h=overview->spritebitmap.GetX2(),h2=(double)time;
	h2/=overviewlenght;
	h2*=h;

	return (int)h2;
}

void EventEditor::ShowOverviewCycleAndPositions()
{
	guiBitmap *bitmap=&overview->spritebitmap;

	bitmap->guiFillRect(COLOUR_BLACK);

	if(songmode==true)
	{		
		int ws=ConvertTimeToOverviewX(startposition);
		if(ws>=0)
		{
			int we=ConvertTimeToOverviewX(endposition);
			if(we==-1)
			{
				bitmap->guiDrawLineX(ws,0,bitmap->GetY2(),COLOUR_GREEN);
				bitmap->guiDrawLineYX0(ws,bitmap->GetX2());
				bitmap->guiDrawLineY(bitmap->GetY2(),ws,bitmap->GetX2());
			}
			else
				bitmap->guiFillRect(ws,0,we,bitmap->GetY2(),COLOUR_BLACK,COLOUR_GREEN);
		}

		int cs=ConvertTimeToOverviewX(WindowSong()->playbacksettings.cyclestart),ce;

		if(cs>=0)
		{
			ce=ConvertTimeToOverviewX(WindowSong()->playbacksettings.cycleend);

			if(ce==-1)
				ce=bitmap->GetX2();

			bitmap->guiFillRect(cs,0,ce,bitmap->GetY2(),COLOUR_OVERVIEWCYCLE);
		}

		int oy=-1,oy2;
		ShowOverviewVertPosition(&oy,&oy2);

		if(oy>-1)
		{
			int sy2;

			sy2=overview_y2=oy2;

			if(sy2>bitmap->GetY2())
				sy2=bitmap->GetY2();

			bitmap->guiDrawLineYX0(overview_y=oy,bitmap->GetX2(),COLOUR_GREEN);
			bitmap->guiDrawLineYX0(sy2,bitmap->GetX2());

			//			if(ws>=0 && sy2>oy)
			//				bitmap->guiFillRect(ws,oy,we,sy2,COLOUR_GREY_DARK,COLOUR_GREEN);
		}

		//bitmap->guiDrawRect(cs,0,ce,bitmap->GetY2(),COLOUR_GREY);

		overviewsppx=ConvertTimeToOverviewX(overviewtime=WindowSong()->GetSongPosition());
		if(overviewsppx>=0)
			bitmap->guiFillRect(overviewsppx,0,overviewsppx+2,bitmap->GetY2(),COLOUR_TIMEPOSITION);
	}
	else
		ShowOverviewPositions_Ex();
}

void EventEditor::TimeChange()
{
	//TRACE ("EE TimeChange ToolOff\n");
	ToolTipOff();
	//TRACE ("EE TimeChange RefreshStartPosition guiTimeLine %d\n",guiTimeLine);
	RefreshStartPosition();	
	if(refreshslider==true)
	{
		//	TRACE ("EE TimeChange RefreshTimeSlider\n");
		RefreshTimeSlider();
	}
	//TRACE ("EE TimeChange End\n");
}

void Edit_Arrange::NewZoom()
{
	RefreshStartPosition();
}

void Edit_Arrange::NewDataZoom()
{
	DrawDBBlit(pattern);
}

void Edit_Arrange::NewYPosition(double y,bool draw)
{
	if(trackobjects.InitWithPercent(y)==true)
		DrawDBBlit(tracks,draw==true?pattern:0);
}

void Edit_Arrange::RefreshStartPosition()
{
	DrawHeader();

	DrawDBBlit(tempo,signature,pattern);

	// Refresh Sprite only... so no overview->DrawGadgetBlt
	ShowOverviewCycleAndPositions();
	DrawDBSpriteBlit(overview);

	//InitMousePosition();
	//ShowMovePatternSprites();
	//ShowOverview(-1);
}

void Edit_Drum::RefreshStartPosition()
{
	DrawHeader();

	ShowCycleAndPositions(eventsgfx);
	DrawDBBlit(eventsgfx); ///*showlist==true?waveraster:*/0);

	ShowOverviewCycleAndPositions();
	DrawDBSpriteBlit(overview);
}

void Edit_Piano::NewYPosition(double y,bool draw)
{
	keyobjects.InitWithPercent(y);

	if(draw==true)
		DrawDBBlit(keys,noteraster);
	else
		DrawDBBlit(keys);
}

void Edit_Piano::RefreshStartPosition()
{
	DrawHeader();

	ShowCycleAndPositions(noteraster);

	if(showwave==true)
		ShowCycleAndPositions(waveraster);

	DrawDBBlit(noteraster,showwave==true?waveraster:0);

	ShowOverviewCycleAndPositions();
	DrawDBSpriteBlit(overview);
}

void Edit_Wave::RefreshStartPosition()
{
	ShowWavesHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_HEADER);
	InitMousePosition();
}

void Edit_Event::RefreshStartPosition()
{
	DrawHeader();

	ShowCycleAndPositions(eventsgfx);
	DrawDBBlit(eventobjects.InitYStartOStart(startposition)==true?events:0,showgfx==true?eventsgfx:0);
}

bool EventEditor::NewStartPosition(OSTART position,bool setwaitrealtime,Seq_SelectionEvent *startevent)
{
	if(songmode==false)
		return false;

	if(position<0)
		position=0;

	if(timeline)
		timeline->oldmidposition=-1; // Reset

	if(startposition!=position || startevent)
	{
		OSTART qt=position/zoom->ticksperpixel;
		qt*=zoom->ticksperpixel;

		startposition=qt;

		//if(startevent)startevent=NewStartEvent(startevent);

		if((!startevent) || startposition!=position) // No Event Refresh
		{
			if(setwaitrealtime==true)
				realtimewaitcounter=40;

			TimeChange();
		}

		return true;
	}

	return false;
}

void EventEditor::SelectAllEvents(int flag)
{
	/*
	Seq_Event *event=patternselection.FirstSelectionEvent(0,SEL_ALLEVENTS);

	while(event)
	{
	if((event->flag&OFLAG_SELECTED)!=flag)
	{
	event->Select(flag);
	}

	event=patternselection.NextSelectionEvent(SEL_ALLEVENTS);
	}
	*/
}

void EventEditor::SetEditorMode(int nmode)
{
	if(nmode==EM_RESET)
	{
		if(mousemode!=selectedmousemode)
		{
			mousemode=selectedmousemode;
			ResetMoreEventFlags(); // Arrange etc
		}
		/*
		else
		{
		#ifdef DEBUG
		maingui->MessageBoxError(0,"mousemode==resetmousemode");
		#endif
		}
		*/
		mousepointer=CURSOR_STANDARD;
	}
	else
		mousemode=nmode;
}

void EventEditor_Selection::ResetMoreEventFlags()
{
	// Reset Event Flags
	Seq_SelectionEvent *ee=patternselection.FirstMixEvent();

	while(ee)
	{
		if(ee->seqevent->flag&OFLAG_UNDERSELECTION){
			ee->seqevent->flag CLEARBIT OFLAG_UNDERSELECTION;
			maingui->ShowMouseSelection_Event(ee->seqevent);
		}

		ee=ee->NextEvent();
	}
}

void EventEditor::EditSongStartPosition(int x,int y)
{
	int mode=Seq_Pos::POSMODE_NORMAL;

	switch(windowtimeformat)
	{
	case WINDOWDISPLAY_SMPTE:
		mode=WindowSong()->project->standardsmpte;
		break;
	}

	WindowSong()->EditSongPosition(this,x,y,mode,EDIT_TIME);
}

OSTART EventEditor::GetTimeLineGrid()
{
	return WindowSong()->timetrack.zoomticks; // default Grid
}

guiGadget *EventEditor::SpecialGadgets(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_GOTOSONGPOSITION:
	case GADGETID_GOTOCYCLELEFT:
	case GADGETID_GOTOCYCLERIGHT:
		{
			OSTART pos=0;

			switch(g->gadgetID)
			{
			case GADGETID_GOTOSONGPOSITION:
				pos=WindowSong()->GetSongPosition();
				break;

			case GADGETID_GOTOCYCLELEFT:
				pos=WindowSong()->playbacksettings.cyclestart;
				break;

			case GADGETID_GOTOCYCLERIGHT:
				pos=WindowSong()->playbacksettings.cycleend;
				break;
			}

			if(NewStartPosition(pos,true)==true)
			{
				SyncWithOtherEditors();
				UserEdit();
			}

			followsongposition_stopped=false; // Reset
			return 0;
		}
		break;
	}

	return g;
}

guiGadget *EventEditor::Editor_Gadget(guiGadget *g)
{
	if(!Editor_Slider(g))return 0;
	if(!CheckToolBox(g))return 0;

	if(!CheckControlBox(g))return 0;

	return SpecialGadgets(g);

	/*
	switch(g->gadgetID)
	{
	case GADGETID_EVENTEDITOR_TIMEBUTTON:
	case GADGETID_EVENTEDITOR_SMPTEBUTTON:
	{
	song->SetSongPosition(g->time,true);
	return 0;
	}
	break;
	}
	*/

	//return g;
}

guiGadget *EventEditor_Selection::Editor_Gadget(guiGadget *g)
{
	if(!Editor_Slider(g))return 0;
	if(!CheckToolBox(g))return 0;

	/*
	g=CheckControlBox(g);
	if(!g)return 0;


	*/

	if(!SpecialGadgets(g))return 0;

	switch(g->gadgetID)
	{
		/*
		case GADGETID_EVENTEDITOR_TIMEBUTTON:
		case GADGETID_EVENTEDITOR_SMPTEBUTTON:
		{
		song->SetSongPosition(g->time,true);
		return 0;
		}
		break;
		*/


	case GADGETID_EVENTEDITOR_SELECTPATTERNSOLO:
		{
			if(g->index)
			{
				patternselection.solopattern=insertpattern;
				editsolopattern=true;
			}
			else
			{
				editsolopattern=false;
				patternselection.solopattern=0;
			}

			SoloPattern();
			patternselection.BuildEventList(patternselection.buildfilter,0);
			AfterSolo();
			return 0;
		}

	case GADGETID_EVENTEDITOR_SELECTPATTERN:
		{
			Seq_SelectedPattern *sp=patternselection.FirstSelectedPattern();
			int index=g->index;

			while(sp)
			{
				if(!sp->pattern->mainpattern)
				{
					if(!index)
					{
						if(insertpattern!=sp->pattern)
						{
							insertpattern=sp->pattern;

							if(editsolopattern==true)
							{
								patternselection.solopattern=editsolopattern==true?insertpattern:0;
								SoloPattern();
							}

							if(patternselection.solopattern)
								patternselection.BuildEventList(patternselection.buildfilter,0);

							AfterSolo();
						}

						return 0;
					}

					index--;
				}

				sp=sp->NextSelectedPattern();	
			}
		}
		return 0;
	}

	return g;
}

void EventEditor::RefreshTimeSlider()
{
	if(songmode==true)
	{
		if(horzgadget)
		{	
			OSTART beats=WindowSong()->ConvertTicksToBeats(WindowSong()->GetSongLength_Ticks()),posbeats=WindowSong()->ConvertTicksToBeats(startposition);
			if(posbeats>beats)posbeats=beats;

			horzgadget->ChangeSlider(0,(int)beats,(int)posbeats);
		}
	}
	else
		RefreshTimeSlider_Ex();

}

void EventEditor::AddSteptoSongPosition()
{
	if(WindowSong()->status==Seq_Song::STATUS_STOP)
	{
		OSTART newposition=WindowSong()->GetSongPosition();

		switch(addsongpositionstep)
		{	
		case ADDSTEP_OFF:
			break;

		case ADDSTEP_1:
			newposition+=TICK1nd;
			break;

		case ADDSTEP_12:
			newposition+=TICK2nd;
			break;

		case ADDSTEP_14:
			newposition+=TICK4nd;
			break;

		case ADDSTEP_18:
			newposition+=TICK8nd;
			break;

		case ADDSTEP_16:
			newposition+=TICK16nd;
			break;

		case ADDSTEP_DYNAMIC:
		case ADDSTEP_MEASURE: // to next Measure
			{
				Seq_Pos spos(Seq_Pos::POSMODE_COMPRESS);

				WindowSong()->timetrack.ConvertTicksToPos(newposition,&spos);

				if(addsongpositionstep==ADDSTEP_MEASURE)
					spos.pos[0]++;
				else
					spos.pos[1]++;

				newposition=WindowSong()->timetrack.ConvertPosToTicks(&spos);
			}
			break;
		}

		WindowSong()->SetSongPosition(newposition,false);
	}
}

void EventEditor::CheckRealtimeHeader()
{
	if(timeline)
	{
		if(WindowSong()->GetSongPosition()<startposition || WindowSong()->GetSongPosition()>endposition)
		{
			OSTART nsigticks=WindowSong()->timetrack.ConvertTicksToMeasureTicks(WindowSong()->GetSongPosition(),false);
			NewStartPosition(nsigticks,false);
			return;
		}

		OSTART h=endposition-startposition;
		h/=2;
		h+=startposition; // Mid

		/*
		// Find Next Sig
		int nsigticks=WindowSong()->timetrack.ConvertTicksToMeasureTicks(h,true);

		if(nsigticks>=endposition)
		nsigticks=WindowSong()->timetrack.ConvertTicksToMeasureTicks(h,false);
		*/

		if(WindowSong()->GetSongPosition()>=h)
		{
			//Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(WindowSong()->GetSongPosition());
			//int nsigticks=WindowSong()->timetrack.ConvertTicksToMeasureTicks(h+sig->dn_ticks,true);
			NewStartPosition(startposition+WindowSong()->GetSongPosition()-h,false);
		}
	}
}

void EventEditor::RefreshEventEditorRealtime_Slow()
{
}

bool EventEditor::RefreshEventEditorRealtime(bool force)
{
	bool refreshtimelines=false;

	CheckXButtons();

	if(songmode==true && timeline && timeline->dbgadget)
	{
		/*
		if(timeline->oldmidposition>=0)
		{
		// Check Zoom Keys & Left Mouse
		if(maingui->GetLeftMouseButton()==false &&
		maingui->GetShiftKey()==false
		)
		{
		timeline->oldmidposition=-1;
		}
		}
		*/

		int mx=timeline->dbgadget->GetCursorX();

		if(mx<0 || mx>timeline->dbgadget->GetX2())
			goto check;

		bool ineditarea;

		if(timeline->dbgadget->hWnd==maingui->mousehWnd)
		{
			//int my=timeline->dbgadget->GetMouseY();

			//if(my<0 || my>timeline->dbgadget->GetY2())
			ineditarea=true;
		}
		else
			ineditarea=false;

		if(ineditarea==false)
		{
			if(editarea && editarea->hWnd==maingui->mousehWnd)
			{
				int my=editarea->GetMouseY();

				if(my>=0 && my<=editarea->GetY2())
					ineditarea=true;
			}
		}

		if(ineditarea==false)
		{
			if(editarea2 && editarea2->formchild->InUse()==true && editarea2->hWnd==maingui->mousehWnd)
			{
				int my=editarea2->GetMouseY();

				if(my>=0 && my<=editarea2->GetY2())
					ineditarea=true;
			}
		}

		if(ineditarea==false)
		{
			if(editarea3 && editarea3->formchild->InUse()==true && editarea3->hWnd==maingui->mousehWnd)
			{
				int my=editarea3->GetMouseY();

				if(my>=0 && my<=editarea3->GetY2())
					ineditarea=true;
			}
		}

		if(ineditarea==true)
		{	
			WindowSong()->SetMousePosition(QuantizeEditorMouse(timeline->ConvertXPosToTime(mx)));		
		}

check:
		if( //songmode==true &&
			(
			(timeline->cycleonheader==true && timeline->cycleon!=WindowSong()->playbacksettings.cycleplayback) ||
			timeline->CheckMousePositionAndSongPosition()==false ||
			(mousemode==EM_MOVESONGPOSITION && timeline->movesongposition==false) ||
			(mousemode!=EM_MOVESONGPOSITION && timeline->movesongposition==true)
			)
			)
		{
			timeline->ShowCycleAndPositions();
			timeline->dbgadget->DrawSpriteBlt();

			refreshtimelines=true;
		}
	}//if timeline

	if(overview && songmode==true)
	{
		if(overviewtime!=WindowSong()->GetSongPosition() && overviewsppx!=ConvertTimeToOverviewX(overviewtime=WindowSong()->GetSongPosition()))
		{
			ShowOverviewCycleAndPositions();
			overview->DrawSpriteBlt();
		}
		else
		{
			int y,y2;

			if(overview_y!=-1)
			{
				ShowOverviewVertPosition(&y,&y2);

				if(overview_y!=y || overview_y2!=y2)
				{
					ShowOverviewCycleAndPositions();
					overview->DrawSpriteBlt();
				}
			}
		}
	}

	CheckAutoScroll();

	// Mouse Quantize
	if(mouseqgadget)
	{
		if(mousequantize!=rt_mousequantize || windowtimeformat!=rt_windowdisplay)
			mouseqgadget->ChangeButtonText(MouseQuantizeString());
	}

	guitoolbox.RefreshRealtime();

	// Scroll ?
	if(
		followsongposition==true &&
		WindowSong() &&
		followsongposition_stopped==false && 
		songmode==true
		)
	{
		if( 
			//((WindowSong()->status&Seq_Song::STATUS_SONGPLAYBACK_MIDI) || force==true) && 
			left_mousekey!=MOUSEKEY_DOWN && autoscroll==false
			)
		{
			if(realtimewaitcounter)
			{
				realtimewaitcounter--;
			}

			if((!realtimewaitcounter) || force==true)
			{
				//realtimewaitcounter=40;

				switch(GetEditorID())
				{
				case EDITORTYPE_ARRANGE:
				case EDITORTYPE_DRUM:
				case EDITORTYPE_PIANO:
				case EDITORTYPE_WAVE:
				case EDITORTYPE_SCORE:
				case EDITORTYPE_TEMPO:
				case EDITORTYPE_SAMPLE:
					{
						CheckRealtimeHeader();
					}
					break;

				case EDITORTYPE_EVENT:
					{
						Edit_Event *ee=(Edit_Event *)this;
						OSTART wsp=WindowSong()->GetSongPosition();
						int oldix=ee->firstevent_index;

						if(ee->FirstEvent() && ee->LastEvent())
						{
							OSTART start=ee->FirstEvent()->seqevent->GetEventStart(),end=ee->LastEvent()->seqevent->GetEventStart();

							if( (wsp>end && (ee->LastEvent()->seqevent->NextEvent() || timeline)) ||
								(wsp<start && (ee->FirstEvent()->seqevent->PrevEvent() || timeline)) 
								)
								ee->SetStartPosition(wsp);
							else
							{
								CheckRealtimeHeader();
								oldix=ee->firstevent_index;
							}
						}
						else
						{
							// Empty '?
							if(ee->patternselection.LastMixEvent() && ee->patternselection.LastMixEvent()->seqevent->GetEventStart()>=wsp)
								ee->SetStartPosition(wsp);
							else
							{
								CheckRealtimeHeader();
								oldix=ee->firstevent_index;
							}
						}

						if(ee->firstevent_index!=oldix)
						{
							ee->startposition=wsp;
							ee->RefreshStartPosition();
						}
					}
					break;
				}
			}
		}
	}
	else
		realtimewaitcounter=0;

	return refreshtimelines;
}

bool EventEditor::Edit_MouseWheel(int delta)
{
	if(maingui->GetShiftKey()==true && vertzoomgadget)
	{
		zoomvert=true;
		vertzoomgadget->DeltaY(delta);
		zoomvert=false;
		return true;
	}

	/*
	NumberOListRef *r=FirstNumberOListRef();

	while(r){	
	if(activenumberobject=r->FindNumberObject(GetMouseX(),GetMouseY()))break;
	r=r->NextOListRef();
	}

	if(activenumberobject)
	{
	TRACE ("MW %d\n",delta);
	editnumber_diffx=0;
	editnumber_diffy=delta>0?1:-1;
	EditNumberObject(activenumberobject,0);
	return false;
	}

	if(maingui->GetShiftKey()==true)
	{
	TRACE ("Delta %d\n",delta);

	nVirtKey=delta<0?KEY_CURSORUP:KEY_CURSORDOWN;
	Editor_KeyDown();
	return false;
	}
	*/

	return false;
}

void EventEditor::InitMousePosition()
{
	if(editarea && timeline)
	{
		int mx=editarea->GetMouseX();

		if(mx<0)
			mx=0;
		else
			if(mx>editarea->GetX2())
				mx=editarea->GetX2();

		WindowSong()->SetMousePosition(QuantizeEditorMouse(timeline->ConvertXPosToTime(mx)));
	}
	else
		WindowSong()->SetMousePosition(-1);
}

bool EventEditor::EditorMouseMove(bool inside)
{
#ifdef OLDIE
	bool insideedit=false;
	addtolastundo=true;

	if(editactivenumberobject_left==true)
	{
		if(activenumberobject)
		{
			//TRACE ("MM %d %d -> %d %d Vl %d %d\n",editactivenumberobject_mousestartx,editactivenumberobject_mousestarty,GetMouseX(),GetMouseY(),activenumberobject->from,activenumberobject->to);

			editnumber_diffx=GetMouseX()-editactivenumberobject_mousestartx;
			editnumber_diffy=GetMouseY()-editactivenumberobject_mousestarty;

			editnumber_diffy/=2; // Y-Speed

			//TRACE ("Mouse Diff %d\n",editnumber_diffy);

			if(editnumber_diffy!=0)
			{
				editactivenumberobject_mousestartx=GetMouseX();
				editactivenumberobject_mousestarty=GetMouseY();

				EditNumberObject(activenumberobject,activenumberobject_flag);

				//activenumberobject_flag|=ACTIVENUMBEROBJECT_EDIT;
			}

			return true;
		}

		editactivenumberobject_left=false;
	}

	if(timeline)
	{
		//insideedit=InitMousePosition();

		/*
		if(insideedit==true && autoscroll==true && autoscrollmode==AUTOSCROLL_TIME_EDITOR) // change cycle
		{
		switch(mousemode)
		{
		//case EM_SIZEY:
		//	SetMouseCursor(CURSOR_UPDOWN);
		//	break;

		case EM_SIZEPATTERN_LEFT:
		SetMouseCursor(CURSOR_LEFT);
		break;

		case EM_SIZEPATTERN_RIGHT:
		SetMouseCursor(CURSOR_RIGHT);
		break;

		case EM_MOVESONGPOSITION:
		{
		if(WindowSong()->CanSongPositionBeChanged(mouseposition)==true)
		WindowSong()->SetSongPosition(mouseposition,true);
		SetMouseCursor(CURSOR_LEFTRIGHT);
		return true;
		}
		break;

		case EM_MOVECYCLE:
		{
		TRACE ("MCylce CP=%d MP=%d MODEP=%d\n",modecyclestartposition,mouseposition,modestartposition);

		OSTART diff=mouseposition-modestartposition,cstart=modecyclestartposition;

		cstart+=diff;

		if(diff && cstart>=0)
		WindowSong()->SetCycleStart(cstart,true); // + move full cycle range

		return true;
		}
		break;

		case EM_SETCYCLE_START:
		{
		WindowSong()->SetCycleStart(mouseposition,false);
		SetMouseCursor(CURSOR_LEFT);
		return true;
		}
		break;

		case EM_SETCYCLE_END:
		{
		WindowSong()->SetCycleEnd(mouseposition);
		SetMouseCursor(CURSOR_RIGHT);
		return true;
		}
		break;

		case EM_SETCYCLE_SETCYCLE:
		{
		int startmeasure=WindowSong()->timetrack.ConvertTicksToMeasure(modestartposition),
		endmeasure=WindowSong()->timetrack.ConvertTicksToMeasure(mouseposition);

		if(startmeasure==endmeasure)
		endmeasure=modestartposition<mouseposition?startmeasure+1:startmeasure-1;
		else
		if(mouseposition>modestartposition)
		endmeasure++;

		OSTART hend=WindowSong()->timetrack.ConvertMeasureToTicks(endmeasure),
		hstart=WindowSong()->timetrack.ConvertMeasureToTicks(startmeasure);

		WindowSong()->SetCycle(hstart,hend);
		SetMouseCursor(CURSOR_LEFT);
		return true;
		}
		break;
		}
		}
		*/

		if(insideedit==true)
		{
			/*
			if(mouseposition>=0)
			{
			if(autoscroll==false && timeline->cycleonheader==true && GetMouseY()>=timeline->y && GetMouseY()<=timeline->splity)
			{
			if(timeline->cyclepos_x && timeline->cyclepos_x>=GetMouseX()-6 && timeline->cyclepos_x<=GetMouseX()+6)
			{		
			// Mouse Over Header |>
			timeline->arrowcycleleft=true;
			SetMouseCursor(CURSOR_LEFT);
			return true;
			}

			if(timeline->cyclepos_x2 && timeline->cyclepos_x2-6<=GetMouseX() && timeline->cyclepos_x2+6>=GetMouseX())
			{
			// Mouse Over Header <|
			timeline->arrowcycleright=true;
			SetMouseCursor(CURSOR_RIGHT);
			return true;
			}

			timeline->ClearMouseMoveArrows();
			}
			else
			timeline->ClearMouseMoveArrows();
			}
			else
			maingui->ClearMouseLine(WindowSong(),false); // clear only mouse window
			*/
		}
		else
			maingui->ClearMouseLine(WindowSong(),false);// clear only mouse window
	}
	else
		mouseposition=-1;

	// Check Mouse Over SongPosition
	mouseoversongposition=false;

	/*
	if(timeline && clocktimesprite.ondisplay==true)
	{
	if(GetMouseX()>=clocktimesprite.x-3 && GetMouseX()<=clocktimesprite.x+3 && GetMouseY()>=clocktimesprite.y && GetMouseY()<clocktimesprite.y2)
	{
	SetMouseCursor(CURSOR_LEFTRIGHT);
	mouseoversongposition=true;
	}
	}
	*/

	if(inside==true) // Check Edit Values
	{
		NumberOListRef *r=FirstNumberOListRef();
		NumberObject *found=0;

		while(r){	

			if(NumberObject *no=r->FindNumberObject(GetMouseX(),GetMouseY()))
				found=no;

			r=r->NextOListRef();
		}

		if(found)
		{
			if(activenumberobject!=found){

				// Toggle old
				if(activenumberobject){

					if(guibuffer && activenumberobject->invert==true){						
						guibuffer->guiInvert(activenumberobject->x,activenumberobject->y,activenumberobject->GetX2(),activenumberobject->y2);
						BltGUIBuffer(activenumberobject->x,activenumberobject->y,activenumberobject->GetX2(),activenumberobject->y2);
					}

					activenumberobject->invert=false;
				}

				activenumberobject=found;

				//	TRACE("Mouse Number Found under Mouse %d %d %d %d\n",no->x,no->y,no->GetX2(),no->y2);

				activenumberobject->invert=true;

				if(guibuffer){
					guibuffer->guiInvert(activenumberobject->x,activenumberobject->y,activenumberobject->GetX2(),activenumberobject->y2);
					BltGUIBuffer(activenumberobject->x,activenumberobject->y,activenumberobject->GetX2(),activenumberobject->y2);
				}
			}
		}
		else
		{
			// Toggle old
			if(activenumberobject){

				if(guibuffer && activenumberobject->invert==true){						
					guibuffer->guiInvert(activenumberobject->x,activenumberobject->y,activenumberobject->GetX2(),activenumberobject->y2);
					BltGUIBuffer(activenumberobject->x,activenumberobject->y,activenumberobject->GetX2(),activenumberobject->y2);
				}

				activenumberobject->invert=false;
			}

			activenumberobject=0;
			DisplayTooltip(mouseposition);
		}
	}

	EditMouseMoved(inside);
#endif

	return false;
}

void EventEditor_Selection::RefreshSelectionGadget()
{
	if(patternedit)
	{
		Seq_SelectedPattern *p=patternselection.FirstSelectedPattern();

		while(p)
		{
			if(p->patternname)
			{
				if(strcmp(p->patternname,p->pattern->GetName())!=0)
					goto refresh;
			}

			if(p->trackname)
			{
				if(strcmp(p->trackname,p->pattern->track->GetName())!=0)
					goto refresh;
			}

			if(p->trackindex!=p->pattern->track->GetTrackIndex())
				goto refresh;


			p=p->NextSelectedPattern();
		}

		if(eventcounter_display!=patternselection.events.GetCount())
			goto refresh;
	}

	return;

refresh:
	ShowSelectionPattern();
}

void EventEditor_Selection::ShowSelectionPattern()
{
	TRACE ("ShowSelectionPattern \n");

	bool solo=false;

	if(patternedit)
	{
		patternedit->ClearCycle();

		Seq_SelectedPattern *p=patternselection.FirstSelectedPattern();

		int nrMIDIPattern=patternselection.GetCountOfRealSelectedPattern();

		// insertpattern inside list ? removed ?
		if(patternselection.CheckIfPatternInList(insertpattern)==false)
			insertpattern=0;

		char nrs[NUMBERSTRINGLEN];
		int i=1;

		while(p){

			if(p->trackname)
			{
				delete p->trackname;
				p->trackname=0;
			}

			if(p->patternname)
			{
				delete p->patternname;
				p->patternname=0;
			}

			p->nrevents=p->pattern->GetCountOfEvents();
			p->trackindex=p->pattern->track->song->GetOfTrack(p->pattern->track);

			if(!p->pattern->mainpattern)
			{
				p->patternname=mainvar->GenerateString(p->pattern->GetName());

				char *ph=new char[strlen(p->patternname)+strlen(p->pattern->track->GetName())+(NUMBERSTRINGLEN*4)];

				if(ph)
				{
					if(!insertpattern)
						insertpattern=(MIDIPattern *)p->pattern; // set edit pattern

					if(nrMIDIPattern>1)
					{
						strcpy(ph,"<");
						char *h=mainvar->ConvertIntToChar(i++,nrs);
						mainvar->AddString(ph,h);
						mainvar->AddString(ph,"/");
						h=mainvar->ConvertIntToChar(nrMIDIPattern,nrs);
						mainvar->AddString(ph,h);
						mainvar->AddString(ph,"> ");
					}
					else
						ph[0]=0;

					mainvar->AddString(ph,p->pattern->GetName());

					if(p->nrevents=p->pattern->GetCountOfEvents()){

						mainvar->AddString(ph," [E:");
						mainvar->AddString(ph,mainvar->ConvertIntToChar(p->nrevents,nrs));
						mainvar->AddString(ph,"]");	
					}

					mainvar->AddString(ph," [T (");
					char tint[NUMBERSTRINGLEN];
					mainvar->AddString(ph,mainvar->ConvertIntToChar(p->trackindex+1,tint));
					mainvar->AddString(ph,"):");

					p->trackname=mainvar->GenerateString(p->pattern->track->GetName());
					mainvar->AddString(ph,p->trackname);
					mainvar->AddString(ph,"]");

					patternedit->AddStringToCycle(ph);

					delete ph;
				}
			}

			p=p->NextSelectedPattern();
		}

		if(insertpattern)
			patternedit->SetCycleSelection(patternselection.GetOfRealPattern(insertpattern));
	}

	if(patterneditsolo)
		patterneditsolo->SetCheckBox(editsolopattern);

	eventcounter_display=patternselection.events.GetCount();
}

void EventEditor::ResetAllGadgets()
{
	guictrlbox.glist=0;
	ResetEditorGadgets();
	ResetGadgets_SelectPattern();
	ResetSlider();
}

void EventEditor::ResetEditorGadgets()
{
	timebutton=patterneditsolo=patternedit=0;
}

EventEditor::EventEditor()
{
	ResetSlider();

	songmodepossible=true;

	overview=0;
	mouseoversongposition=false;
	followsongposition_stopped=false;

	mousequantizemenu=0;
	displaymenu=0;
	followmenu=0;
	syncmenu=0;

	//frame_header.settingsvar=&mainsettings->headerheight;
	mousequantize=MOUSEQUANTIZE_BEAT; // Measure
	addsongpositionstep=ADDSTEP_14; // 1/4 Default
	bound=false;

	selectedmousemode=mousemode=EM_SELECT;

	refresheditorevents=false;
	insertpattern=0;
	headerflag=0;

	addtolastundo=false;
	editsolopattern=false;
	followsongposition=false;
	syncsongposition=false;

	guictrlbox.editor=this;

	refreshslider=true;
	eventeditortime=-1;

	ResetAllGadgets();

	pos_measure.mode=Seq_Pos::POSMODE_NORMAL;

	pos24.mode=Seq_Pos::POSMODE_SMPTE_24;
	pos25.mode=Seq_Pos::POSMODE_SMPTE_25;
	pos48.mode=Seq_Pos::POSMODE_SMPTE_48;
	pos50.mode=Seq_Pos::POSMODE_SMPTE_50;

	pos2997.mode=Seq_Pos::POSMODE_SMPTE_2997;
	pos30.mode=Seq_Pos::POSMODE_SMPTE_30;
	pos239.mode=Seq_Pos::POSMODE_SMPTE_239;
	pos249.mode=Seq_Pos::POSMODE_SMPTE_249;
	pos2997df.mode=Seq_Pos::POSMODE_SMPTE_2997df;
	pos30df.mode=Seq_Pos::POSMODE_SMPTE_30df;
	pos599.mode=Seq_Pos::POSMODE_SMPTE_599;
	pos60.mode=Seq_Pos::POSMODE_SMPTE_60;

	pos_samples.mode=Seq_Pos::POSMODE_SAMPLES;
	pos_seconds.mode=Seq_Pos::POSMODE_TIME;

	windowtimeformat=mainsettings->editordefaulttimeformat;
	gotocalled=false;
	mousequantize=mainsettings->defaultmousequantize;

	mouseeditx=-1;
	overview_y=-1;
	addtolastundo=false;
	//InitDisplay();
}

void EventEditor::ResetMouseMode(int nmode)
{
	addtolastundo=false;

	int om=mousemode;

	SetEditorMode(EM_RESET); // default mode

	if(autoscrollwin)
	{
		ResetAutoScrolling();

		if(editarea && om==EM_SELECTOBJECTS)
		{
			editarea->callback(editarea,DB_PAINTSPRITE);
			editarea->DrawSpriteBlt();
		}
	}
}

OSTART EventEditor::GetOverviewTime(int x)
{
	double w=overview->GetWidth(),h2=x;

	h2/=w;
	h2*=WindowSong()->GetSongLength_Ticks();

	//OSTART mtime=(OSTART)h2;

	return (OSTART)h2;
}

void EventEditor::MouseClickInOverview(bool leftmouse)
{
	if(songmode==true)
	{
		int mx=overview->SetToXX2(overview->GetMouseX()),
			my=overview->SetToYY2(overview->GetMouseY());

		//TRACE ("OMX %d \n",mx);

		//double w=overview->GetWidth(),h2=mx;

		//h2/=w;
		//h2*=WindowSong()->GetSongLength_Ticks();

		OSTART mtime=GetOverviewTime(mx);

		double h=overview->GetHeight(),hy=my;

		hy/=h;

		if(leftmouse==false)
		{
			WindowSong()->SetSongPosition(mtime,true); // right MB
		}
		else
		{
			switch(overviewmode)
			{
			case OV_HORZ:
				NewStartPosition(mtime,false); // left MB
				SyncWithOtherEditors();
				UserEdit();
				break;

			case OV_VERT:
				NewYPosition(hy);
				break;

			case OV_BOTH:
				{
					NewYPosition(hy,startposition!=mtime?false:true);
					NewStartPosition(mtime,false); // left MB
					SyncWithOtherEditors();
					UserEdit();
				}
				break;
			}
		}
	}
	else
		MouseClickInOverview_Ex(leftmouse);

}

void EventEditor::DoAutoScroll()
{
	if(autoscrollstatus&AUTOSCROLL_LEFTTURBO)
		ScrollSliderHoriz(-4);

	if(autoscrollstatus&AUTOSCROLL_LEFTFAST)
		ScrollSliderHoriz(-2);

	if(autoscrollstatus&AUTOSCROLL_LEFT)
		ScrollSliderHoriz(-1);

	if(autoscrollstatus&AUTOSCROLL_RIGHTTURBO)
		ScrollSliderHoriz(4);

	if(autoscrollstatus&AUTOSCROLL_RIGHTFAST)
		ScrollSliderHoriz(2);

	if(autoscrollstatus&AUTOSCROLL_RIGHT)
		ScrollSliderHoriz(1);
}

void EventEditor::StartSong()
{
	followsongposition_stopped=false;
}

void EventEditor::StopSong()
{
	followsongposition_stopped=false;
}

void EventEditor::UserEdit()
{
	//followsongposition_stopped_before=followsongposition_stopped;
	//if(WindowSong()->status&Seq_Song::STATUS_SONGPLAYBACK_MIDI)
	followsongposition_stopped=true;
}

void EventEditor::ShowCycleAndPositions(guiGadget_CW *g)
{
	if((!g) || (!timeline))return;

	guiBitmap *bitmap=&g->spritebitmap;

	if(!bitmap)
		return;

	bitmap->guiFillRect(COLOUR_BLACK);

	if(songmode==false)
	{
		ShowSpecialEditorRange(); //v
		return;
	}

	// Song Range ...

	// Cycle 
	if(WindowSong()->playbacksettings.cyclestart<endposition && WindowSong()->playbacksettings.cycleend>startposition){
		int mx=WindowSong()->playbacksettings.cyclestart>=startposition?timeline->ConvertTimeToX(WindowSong()->playbacksettings.cyclestart):timeline->x;
		int mx2=WindowSong()->playbacksettings.cycleend>endposition?mx2=timeline->x2:timeline->ConvertTimeToX(WindowSong()->playbacksettings.cycleend);

		if(mx2!=-1)
			bitmap->guiFillRect(mx,0,mx2,bitmap->GetY2(),COLOUR_CYCLE_PIANO);
	}

	// Song Time
	int psx=timeline->ConvertTimeToX(eventeditortime=WindowSong()->GetSongPosition());

	if(psx>=0)
		bitmap->guiFillRect(psx,0,psx+maingui->GetFontSizeY()/3,bitmap->GetY2(),timeline->movesongposition==false?COLOUR_TIMEPOSITION:COLOUR_TIMEPOSITIONMOUSEMOVE);

	// Mouse Position

	psx=timeline->ConvertTimeToX(eventeditormousetime=GetMousePosition());

	if(psx>=0)
		bitmap->guiDrawLineX(psx,0,bitmap->GetY2(),COLOUR_YELLOW);

	if(autoscrollwin && editarea && autoscrollwin==g && mousemode==EM_SELECTOBJECTS){

		mouseeditx=-1;

		InitMouseEditRange();

		if(mouseeditx!=-1){
			if(mouseeditx+2<mouseeditx2 && mouseedity+2<mouseedity2)
				bitmap->guiFillRect(mouseeditx,mouseedity,mouseeditx2,mouseedity2,COLOUR_LASSO,COLOUR_WHITE);
			else
				bitmap->guiFillRect(mouseeditx,mouseedity,mouseeditx2,mouseedity2,COLOUR_LASSO);
		}
	}

	ShowSpecialEditorRange(); //v
}

bool guiWindow::CanMoveX()
{
	if(xmove==false)
		return false;

	if(maingui->CheckIfKeyDown('y')==true)
		return false;

	return true;
}

bool guiWindow::CanMoveY()
{
	if(ymove==false)
		return false;

	if(maingui->CheckIfKeyDown('x')==true)
		return false;

	return true;
}

bool guiWindow::CheckInLasso(int x,int y,int x2,int y2)
{
	if(mouseeditx!=-1)
	{
		if(x2<x)
		{
			int h=x2;
			x2=x;
			x=h;
		}

		if(y2<y)
		{
			int h=y2;
			y2=y;
			y=h;
		}

		if( (x<=mouseeditx && x2>=mouseeditx) ||
			(x>=mouseeditx && x2<=mouseeditx2) ||
			(x<=mouseeditx2 && x2>=mouseeditx2) 
			)
		{
			if( (y<=mouseedity && y2>=mouseedity) ||
				(y>=mouseedity && y2<=mouseedity2) ||
				(y<=mouseedity2 && y2>=mouseedity2) 
				)
				return true;
		}

	}

	return false;
}

int EventEditor::CheckMouseInTimeLine()
{
	int my=timeline->dbgadget->GetMouseY();

	if(my<=timeline->cycley2)
	{
		if(timeline->cyclepos_x>=0 || timeline->cyclepos_x2>=0)
		{
			int mx=timeline->dbgadget->GetMouseX();

			//if(mx<0 || mx>=timeline->dbgadget->GetWidth())
			//	return -4;

			if(timeline->cyclepos_x>=0)
			{
				if(mx>=timeline->cyclepos_x-3 && mx<=timeline->cyclepos_x+3)
				{
					SetMouseCursor(CURSOR_LEFT);
					return -1;
				}
			}

			if(timeline->cyclepos_x2>=0)
			{
				if(mx>=timeline->cyclepos_x2-3 && mx<=timeline->cyclepos_x2+3)
				{
					SetMouseCursor(CURSOR_RIGHT);
					return 1;
				}
			}

			int cx1=timeline->cyclepos_x==-1?0:timeline->cyclepos_x;
			int cx2=timeline->cyclepos_x2==-1?timeline->x2:timeline->cyclepos_x2;

			if(mx>=cx1 && mx<=cx2)
			{
				SetMouseCursor(CURSOR_HAND);
				return -3;
			}

		}

		return -2;
	}

	return 0;
}

void EventEditor::MouseReleaseInTimeLine(int status)
{
	switch(status)
	{
	case DB_RIGHTMOUSEUP:
		break;

	case DB_LEFTMOUSEUP:
		ResetMouseMode();
		break;
	}
}

bool EventEditor::CheckMouseClickInEditArea(guiGadget_CW *db)
{
	if(timeline && timeline->headertimex>=0 && (editarea==db || editarea2==db || editarea3==db))
	{
		int mx=db->GetMouseX();

		if(mx>=0 && mx<=db->GetX2() && mx>=timeline->headertimex && mx<=timeline->headertimex2)
		{
			InitMouseMoveSPP(db);
			return true;
		}

		// Song Stop Marker
		if(Seq_Marker *mk=WindowSong()->textandmarker.FindMarkerID(Seq_Marker::MARKERFUNC_STOPPLAYBACK))
		{
			if(mx>=0 && mx<=db->GetX2() && mx>=timeline->headersongstoppositionx && mx<=timeline->headersongstoppositionx2)
			{
				InitSongStopMarker(db);
				return true;
			}
		}
	}

	return false;
}

void EventEditor::MouseDoubleClickInTimeLine()
{
	if(!WindowSong())
		return;

	int mx=timeline->dbgadget->GetMouseX(),my=timeline->dbgadget->GetMouseY();

	OSTART time=QuantizeEditorMouse(timeline->ConvertXPosToTime(mx));

	if(time>=WindowSong()->playbacksettings.cyclestart && time<=WindowSong()->playbacksettings.cycleend)
		WindowSong()->ToggleCycle();
}

void EventEditor::MouseClickInTimeLine(int status)
{
	if(!WindowSong())
		return;

	int mx=timeline->dbgadget->GetMouseX(),my=timeline->dbgadget->GetMouseY();

	bool insidetimeline=true;

	if(mx<0)
	{
		mx=0;
		insidetimeline=false;
	}
	else
	{
		if(mx>timeline->dbgadget->GetX2())
		{
			mx=timeline->dbgadget->GetX2();
			insidetimeline=false;
		}
	}

	OSTART time=QuantizeEditorMouse(timeline->ConvertXPosToTime(mx));

	if(status&DB_RIGHTMOUSEDOWN)
	{
		if(maingui->GetLeftMouseButton()==true || songmode==false)
			return;

		if(DeletePopUpMenu(true))
		{
			TimeString timestring;

			int mode;

			switch(windowtimeformat)
			{
			case WINDOWDISPLAY_SMPTE:
				mode=WindowSong()->project->standardsmpte;
				break;

			default:
				mode=Seq_Pos::POSMODE_NORMAL;
				break;
			}

			WindowSong()->timetrack.CreateTimeString(&timestring,time,mode);
			popmenu->AddMenu(timestring.string,0);
			popmenu->AddLine();

			AddTimeDisplayMenu(popmenu);

			popmenu->AddLine();

			class menu_setsongposition:public guiMenu
			{
			public:
				menu_setsongposition(EventEditor *ed,OSTART p)
				{
					editor=ed;
					pos=p;
				}

				void MenuFunction()
				{
					editor->WindowSong()->SetSongPosition(pos,true);
				} 

				EventEditor *editor;
				OSTART pos;
			};

			if(WindowSong()->GetSongPosition()!=time)
			{
				if(WindowSong()->playbacksettings.cycleplayback==false || (WindowSong()->playbacksettings.cycleend>time))
					popmenu->AddFMenu(Cxs[CXS_SETSPP],new menu_setsongposition(this,time));
			}

			class menu_cycleplaybackonoff:public guiMenu
			{
			public:
				menu_cycleplaybackonoff(Seq_Song *s)
				{
					song=s;
				}

				void MenuFunction()
				{
					song->ToggleCycle();
				} 

				Seq_Song *song;
			};

			popmenu->AddFMenu("Cycle Playback",new menu_cycleplaybackonoff(WindowSong()),WindowSong()->playbacksettings.cycleplayback);

			popmenu->AddLine();

			if(time!=WindowSong()->playbacksettings.cyclestart)
			{
				class menu_setcyclestart:public guiMenu
				{
				public:
					menu_setcyclestart(EventEditor *ed,OSTART p)
					{
						editor=ed;
						pos=p;
					}

					void MenuFunction()
					{
						editor->WindowSong()->SetCycleStart(pos,editor->WindowSong()->playbacksettings.cycleend>pos?false:true);
					} 

					EventEditor *editor;
					OSTART pos;
				};

				popmenu->AddFMenu(Cxs[CXS_SETCYCLESTARTHERE],new menu_setcyclestart(this,time));
			}

			if(time!=WindowSong()->playbacksettings.cycleend && time>WindowSong()->playbacksettings.cyclestart)
			{
				class menu_setcycleend:public guiMenu
				{
				public:
					menu_setcycleend(EventEditor *ed,OSTART p)
					{
						editor=ed;
						pos=p;
					}

					void MenuFunction()
					{
						editor->WindowSong()->SetCycleEnd(pos);
					} 

					EventEditor *editor;
					OSTART pos;
				};

				popmenu->AddFMenu(Cxs[CXS_SETCYCLEENDHERE],new menu_setcycleend(this,time));
			}

			ShowPopMenu();

			return;
		}
	}
	else
	{
		if(songmode==false)
			return;

		int mb=0;

		if(mousemode!=EM_SETCYCLE_START && mousemode!=EM_SETCYCLE_END && mousemode!=EM_SETCYCLE_SETCYCLE && mousemode!=EM_MOVECYCLE)
		{
			SetAutoScroll(0,timeline->dbgadget);

			if(status==DB_LEFTMOUSEDOWN && insidetimeline==true)
			{
				mb=CheckMouseInTimeLine();

			}
		}

		TRACE ("MB %d  - %d\n",mb,time);

		switch(mb)
		{
		case -1:
			// Cycle Left
			SetEditorMode(EM_SETCYCLE_START);
			break;

		case 1:
			SetEditorMode(EM_SETCYCLE_END);
			break;

		case -2:
			SetEditorMode(EM_SETCYCLE_SETCYCLE);
			break;

		case -3:
			{
				SetEditorMode(EM_MOVECYCLE);
				timeline->addtomovecycle=time-WindowSong()->playbacksettings.cyclestart;
			}
			break;

		case 0:
			{
				switch(mousemode)
				{
				case EM_SETCYCLE_SETCYCLE:
					{
						if(time>=0)
						{
							WindowSong()->SetCycle(time,time+1);
							SetEditorMode(EM_SETCYCLE_END);
						}
					}
					break;

				case EM_MOVECYCLE:
					if(time>=0)
					{
						WindowSong()->SetCycleStart(time-timeline->addtomovecycle,true);
					}
					break;

				case EM_SETCYCLE_START:

					if(time>=0)
					{
						WindowSong()->SetCycleStart(time,false);
					}
					break;

				case EM_SETCYCLE_END:
					if(time>=0)
					{
						WindowSong()->SetCycleEnd(time);
					}
					break;

				default:
					if(time>=0)
					{
						SetEditorMode(EM_MOVESONGPOSITION);
						WindowSong()->SetSongPosition(time,true);
					}
					break;
				}
			}
			break;
		}
	}
}

void Editor_Header_Callback(guiGadget_CW *g,int status)
{
	EventEditor *ee=(EventEditor *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			ee->header=g;

			g->skippaint=true;
			ee->ShowEditorHeader(g);

			if(ee->overview)
			{
				ee->overview->DrawGadgetBlt();
			}
		}
		break;

	case DB_NEWSIZE:
		ee->timeline->oldmidposition=-1; // Reset
		break;

	case DB_PAINT:
		ee->ShowEditorHeader(g);
		break;

	case DB_MOUSEMOVE:
		ee->CheckMouseInTimeLine();
		break;

	case DB_LEFTMOUSEUP:
	case DB_RIGHTMOUSEUP:
		ee->MouseReleaseInTimeLine(status);
		break;

	case DB_DOUBLECLICKLEFT:
		ee->MouseDoubleClickInTimeLine();
		break;

	case DB_LEFTMOUSEDOWN:
	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		if(ee->timeline->dbgadget->leftmousedown==true)
			ee->MouseClickInTimeLine(status);
		break;

		//case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		ee->MouseClickInTimeLine(status);
		break;
	}
}

char *EventEditor::MouseQuantizeString()
{
	if(songmodepossible==true)
	{
		strcpy(mousequantstring,"Q:");

		switch(rt_mousequantize=mousequantize)
		{
		case MOUSEQUANTIZE_MEASURE:
			mainvar->AddString(mousequantstring,Cxs[CXS_MEASURE]);
			break;

		case MOUSEQUANTIZE_FRAME:
			mainvar->AddString(mousequantstring,"FRM");
			break;

		case MOUSEQUANTIZE_QFRAME:
			mainvar->AddString(mousequantstring,"QFRM");
			break;

		case MOUSEQUANTIZE_BEAT:
			mainvar->AddString(mousequantstring,"Beat");
			break;

		case MOUSEQUANTIZE_1: 
			mainvar->AddString(mousequantstring,quantstr[1]);
			break;

		case MOUSEQUANTIZE_12: 
			mainvar->AddString(mousequantstring,quantstr[4]);
			break;

		case MOUSEQUANTIZE_14: 
			mainvar->AddString(mousequantstring,quantstr[7]);
			break;

		case MOUSEQUANTIZE_18: 
			mainvar->AddString(mousequantstring,quantstr[10]);
			break;

		case MOUSEQUANTIZE_16: 
			mainvar->AddString(mousequantstring,quantstr[13]);
			break;

		case MOUSEQUANTIZE_FREE:
			mainvar->AddString(mousequantstring,Cxs[CXS_FREE]);
			break;

		case MOUSEQUANTIZE_ZOOM:
			mainvar->AddString(mousequantstring,"Zoom");
			break;

			//default free
		}
		mainvar->AddString(mousequantstring,".");
	}
	else
		strcpy(mousequantstring,"T:");

	switch(rt_windowdisplay=windowtimeformat)
	{
	case WINDOWDISPLAY_MEASURE:
		mainvar->AddString(mousequantstring,Cxs[CXS_MEASURE]);
		break;

	case WINDOWDISPLAY_SMPTE:
		mainvar->AddString(mousequantstring,"Frames");
		break;

		//case WINDOWDISPLAY_SMPTEANDMEASURE:
		//	break;

	case WINDOWDISPLAY_SECONDS:
		mainvar->AddString(mousequantstring,Cxs[CXS_SECONDS]);
		break;

	case WINDOWDISPLAY_SAMPLES:
		mainvar->AddString(mousequantstring,"Samples");
		break;
	}

	return mousequantstring;
}

void EventEditor::CreatePos(OSTART pos)
{
	WindowSong()->timetrack.ConvertTicksToPos(pos,displayusepos);
	//displayusepos->AddOffSet();

	switch(windowtimeformat)
	{
	case WINDOWDISPLAY_MEASURE:
		{
			switch(WindowSong()->project->projectmeasureformat)
			{
			case PM_1p1p_1:
			case PM_1p1p_0:
				space[0]='.';
				displayusepos->nozoom=true;
				break;

			case PM_1p1p1p1:
			case PM_1p1p1p0:
				space[0]='.';
				displayusepos->nozoom=false;
				break;

			case PM_11_1:
			case PM_11_0:
				space[0]=' ';
				displayusepos->nozoom=true;
				break;

			case PM_1111:
			case PM_1110:
				space[0]=' ';
				displayusepos->nozoom=false;
				break;

			default:
				space[0]='.';
				displayusepos->nozoom=false;
				break;
			}

			displayusepos->showquarterframe=false;
		}
		break;

	case WINDOWDISPLAY_SMPTE:
		{
			switch(WindowSong()->project->standardsmpte)
			{
			case Seq_Pos::POSMODE_SMPTE_24:
				displayusepos=&pos24;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_25:
				displayusepos=&pos25;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_48:
				displayusepos=&pos48;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_50:
				displayusepos=&pos50;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_2997:
				displayusepos=&pos2997;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_30:
				displayusepos=&pos30;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_2997df:
				displayusepos=&pos2997df;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_30df:
				displayusepos=&pos30df;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_239:
				displayusepos=&pos239;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_249:
				displayusepos=&pos249;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_599:
				displayusepos=&pos599;
				space[0]=':';
				showquarterframe=true;
				break;

			case Seq_Pos::POSMODE_SMPTE_60:
				displayusepos=&pos60;
				space[0]=':';
				showquarterframe=true;
				break;
			}

			displayusepos->nozoom=false;
			displayusepos->showquarterframe=true;
		}
		break;
	}

}

bool EventEditor::CheckSelectPlayback(Seq_Event *e)
{
	if(!e)return false;
	if(WindowSong()!=mainvar->GetActiveSong())return false;

	switch(e->GetStatus())
	{
	case NOTEON:
		return true;

	case PROGRAMCHANGE:
		if(WindowSong()->status==Seq_Song::STATUS_STOP)
			return true;
		break;

	case INTERN:
		break;

	case INTERNCHAIN:
		{
			ICD_Object *obj=(ICD_Object *)e;

			switch(obj->GetICD())
			{
			case ICD_TYPE_DRUM:
				return true;
			}
		}
		break;

	}

	return false;
}

void EventEditor_Selection::ReleaseEdit()
{
	if(GetUndoEditEvents())
	{
		// Reset Edit Flag
		Seq_SelectionEvent *e=patternselection.FirstMixEvent();

		while(e)
		{
			e->seqevent->flag CLEARBIT EVENTFLAG_UNDEREDIT;
			e=e->NextEvent();
		}

		if(GetUndoEditEvents()->CheckChanges()==false)
		{
			GetUndoEditEvents()->Delete();
		}
		else
		{
			// -> Undo
			mainedit->EditEvents(WindowSong(),GetUndoEditEvents());
		}

		SetUndoEditEvents(0);
	}
}

bool EventEditor::CheckStandardDataMessage(EditData *data)
{
	switch(data->id)
	{
	case GOTOMEASURE_ID:
		{
			if(NewStartPosition(data->newvalue,true)==true)
				SyncWithOtherEditors();
		}
		break;

	default:
		return false;
	}

	return true;
}

bool EventEditor::CheckStandardGoto(int to)
{
	switch(to)
	{
	case GOTO_MEASURE:
		{
			if(EditData *edit=new EditData){

				edit->checksongrealtimeposition=false;
				edit->song=WindowSong();
				edit->win=this;
				edit->x=0;
				edit->y=0;
				edit->title="Test";
				edit->deletename=false;

				edit->id=GOTOMEASURE_ID;

				edit->type=EditData::EDITDATA_TYPE_TIME;
				edit->smpteflag=WindowSong()->project->standardsmpte;
				edit->time=startposition;

				edit->from=0;
				edit->to=WindowSong()->GetSongLength_Ticks();

				maingui->EditDataValue(edit);
			}
		}
		break;

	case GOTO_CYCLESTART:
		if(NewStartPosition(WindowSong()->playbacksettings.cyclestart,true)==true)
		{
			SyncWithOtherEditors();
		}
		UserEdit();
		break;

	case GOTO_CYCLEEND:
		if(NewStartPosition(WindowSong()->playbacksettings.cycleend,true)==true)
		{
			SyncWithOtherEditors();
		}
		UserEdit();
		break;

	case GOTO_SONG:
		if(NewStartPosition(WindowSong()->GetSongPosition(),true)==true)
		{
			SyncWithOtherEditors();
		}
		UserEdit();
		break;

	default:
		return false;
	}

	return true;
}

void EventEditor_Selection::Goto(int to)
{
	UserEdit();

	gotocalled=true;

	switch(to)
	{
	case GOTO_FIRST:
		{
			Seq_SelectionEvent *el=patternselection.FirstMixEvent();

			if(el && NewStartPosition(el->seqevent->GetEventStart(),true,el)==true)
			{
				SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_FIRSTSELECTED:
		{
			Seq_SelectionEvent *el=patternselection.FirstMixEvent();

			while(el && el->seqevent->IsSelected()==false)
				el=el->NextEvent();

			if(el && NewStartPosition(el->seqevent->GetEventStart(),true,el)==true)
			{
				SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_LAST:
		{
			Seq_SelectionEvent *el=patternselection.LastMixEvent();

			if(el && NewStartPosition(el->seqevent->GetEventStart(),true,el)==true)
			{
				SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_LASTSELECTED:
		{
			Seq_SelectionEvent *el=patternselection.LastMixEvent();

			while(el && el->seqevent->IsSelected()==false)
				el=el->PrevEvent();

			if(el && NewStartPosition(el->seqevent->GetEventStart(),true,el)==true)
			{
				SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_FIRSTEVENTTRACK:
		if(WindowSong()->GetFocusTrack())
		{
			Seq_SelectionEvent *el=patternselection.FirstMixEvent();
			//	Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

			while(el && el->seqevent->GetPattern()->GetTrack()!=WindowSong()->GetFocusTrack())
				el=el->NextEvent();

			if(el && NewStartPosition(el->seqevent->GetEventStart(),true,el)==true)
			{
				SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_LASTEVENTTRACK:
		if(WindowSong()->GetFocusTrack())
		{
			Seq_SelectionEvent *el=patternselection.LastMixEvent();
			// Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

			while(el && el->seqevent->GetPattern()->GetTrack()!=WindowSong()->GetFocusTrack())
				el=el->PrevEvent();

			if(el && NewStartPosition(el->seqevent->GetEventStart(),true,el)==true)
			{
				SyncWithOtherEditors();
			}
		}
		break;

	default:
		if(Seq_SelectionEvent *special=Goto_Special(to))
		{
			if(special && NewStartPosition(special->seqevent->GetEventStart(),true,special)==true)
			{
				SyncWithOtherEditors();
			}
		}
		break;
	}

	GotoEnd();

	gotocalled=false;
}

void EventEditor::AddStandardGotoMenu()
{
	guiMenu *gotomenu=popmenu;

	if(gotomenu)
	{
		gotomenu->AddFMenu("Song Position",new menu_gotoeventeditor(this,GOTO_SONG));
		gotomenu->AddFMenu(Cxs[CXS_MEASURE],new menu_gotoeventeditor(this,GOTO_MEASURE));
		gotomenu->AddLine();

		gotomenu->AddFMenu(Cxs[CXS_CYCLESTART],new menu_gotoeventeditor(this,GOTO_CYCLESTART));
		gotomenu->AddFMenu(Cxs[CXS_CYCLEEND],new menu_gotoeventeditor(this,GOTO_CYCLEEND));
		gotomenu->AddLine();

		if(IfEvents()==true)
		{
			gotomenu->AddFMenu(Cxs[CXS_FIRSTEVENT],new menu_gotoeventeditor(this,GOTO_FIRST));
			gotomenu->AddFMenu(Cxs[CXS_LASTEVENT],new menu_gotoeventeditor(this,GOTO_LAST));

			if(IfSelectedEvents()==true)
			{
				gotomenu->AddFMenu(Cxs[CXS_FIRSTSELEVENT],new menu_gotoeventeditor(this,GOTO_FIRSTSELECTED));
				gotomenu->AddFMenu(Cxs[CXS_LASTSELEVENT],new menu_gotoeventeditor(this,GOTO_LASTSELECTED));
			}

			switch(editorid)
			{
			case EDITORTYPE_PIANO:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_SCORE:
			case EDITORTYPE_DRUM:
				{
					gotomenu->AddFMenu(Cxs[CXS_FIRSTEVENTAT],new menu_gotoeventeditor(this,GOTO_FIRSTEVENTTRACK));
					gotomenu->AddFMenu(Cxs[CXS_LASTEVENTAT],new menu_gotoeventeditor(this,GOTO_LASTEVENTTRACK));
				}
				break;
			}

			AddSpecialGoto(gotomenu);
		}

	}

}

void EventEditor_Selection::InitSelectionPatternGadget()
{
	ResetGadgets_SelectPattern();

	int x=0;
	int x2;

	if(patternselection.GetCountOfSelectedPattern()>1)
		x2=glist.form->GetWidth(-50);
	else
		x2=glist.form->GetWidth(0); // no sizebox

	int hx2=x2-80;

	int tx2=x+bitmap.prefertimebuttonsize;
	if(tx2>hx2-1)
		tx2=hx2-1;

	if(tx2-x>=bitmap.prefertimebuttonsize)
	{
		//int ty2=frame_patternedit.y+maingui->GetFontSizeY()+4;

		//	timebutton=gl->AddTimeButton(x,frame_patternedit.y,tx2,ty2,WindowSong()->GetSongPosition(),GADGETID_EVENTEDITOR_TIMEBUTTON,MODE_BLACK,"Song Position");
		//	smptebutton=gl->AddSMPTEButton(x,ty2+1,tx2,ty2+maingui->GetFontSizeY()+4,WindowSong()->GetSongPosition(),GADGETID_EVENTEDITOR_SMPTEBUTTON,MODE_BLACK,"Song Position SMPTE");

		hx2=tx2+1;
	}
	else
	{
		hx2=x;
		timebutton=0;
	}

	//if(timebutton && timebutton->x2<hx2)
	{
		int hx3;
		bool showsologadget=false;

		if(x2-25>hx2+80 && patternselection.GetCountOfRealSelectedPattern()>1)
		{
			hx3=x2-25;
			showsologadget=true;
		}
		else
			hx3=glist.form->GetWidth(-1); // no sizebox

		if(showsologadget==true)
		{
			// Create Pattern Solo Gadget
			patterneditsolo=glist.AddCheckBox(
				-1,
				-1,
				50,
				-1,
				GADGETID_EVENTEDITOR_SELECTPATTERNSOLO,
				0,
				"Solo",
				Cxs[CXS_DISPLAYONLYAP]
			);

			glist.AddLX();
		}

		// create Cycle Gadget
		patternedit=glist.AddCycle(
			-1,
			-1,
			-1,
			-1,
			GADGETID_EVENTEDITOR_SELECTPATTERN,
			MODE_RIGHT,
			0,
			Cxs[CXS_PATTERNUSEDINEDITOR]);
	}

	glist.Return();

	ShowSelectionPattern();
}

bool EventEditor::ScrollMeasures(int measures)
{
	bool scroll=false;

	if(measures)
	{
		int m=WindowSong()->timetrack.ConvertTicksToMeasure(startposition);
		OSTART oldstartposition=startposition;

		m+=measures;

		if(m>0)
		{
			startposition=WindowSong()->timetrack.ConvertMeasureToTicks(m);
			if(oldstartposition!=startposition)scroll=true;
		}
	}	
	return scroll;
}

void EventEditor::InitSongStopMarker(guiGadget_CW *db)
{
	SetEditorMode(EM_MOVESONGSTOPPOSITION);
	modestartposition=WindowSong()->GetSongPosition();
	SetAutoScroll(0,db);
}

void EventEditor::InitMouseMoveSPP(guiGadget_CW *db)
{
	SetEditorMode(EM_MOVESONGPOSITION);
	modestartposition=WindowSong()->GetSongPosition();
	SetAutoScroll(0,db);
}

bool EventEditor::CheckMouseMovePosition(guiGadget_CW *db)
{
	switch(mousemode)
	{
	case EM_MOVESONGPOSITION:
		{
			int mx=db->GetMouseX();

			if(mx>=0 && mx<=db->GetX2())
			{
				OSTART time=QuantizeEditorMouse(timeline->ConvertXPosToTime(mx));

				if(time!=WindowSong()->GetSongPosition())
					WindowSong()->SetSongPosition(time,true);
			}

			return true;
		}
		break;

	case EM_MOVESONGSTOPPOSITION:
		{
			int mx=db->GetMouseX();

			if(mx>=0 && mx<=db->GetX2())
			{
				OSTART time=QuantizeEditorMouse(timeline->ConvertXPosToTime(mx));

				Seq_Marker *mk=WindowSong()->textandmarker.FindMarkerID(Seq_Marker::MARKERFUNC_STOPPLAYBACK);

				if(mk)
				{
					if(time!=mk->GetMarkerStart())
					{
						mainthreadcontrol->LockActiveSong();
						WindowSong()->textandmarker.MoveMarker(mk,time);
						mainthreadcontrol->UnlockActiveSong();

						maingui->RefreshAllEditorsWithMarker(WindowSong(),mk);
					}
				}
			}

			return true;
		}
		break;

	}

	return false;
}

void EventEditor_Selection::AddEventEditorMenu(guiMenu *headmenu)
{
	if(headmenu)
	{
		headmenu->editmenu=headmenu->AddMenu(Cxs[CXS_EDIT],0);

		if(headmenu->editmenu)
		{
			maingui->AddUndoMenu(headmenu->editmenu);
			AddEditMenu(headmenu->editmenu);
		}

		headmenu->selectmenu=headmenu->AddMenu(Cxs[CXS_SELECT],0);

		if(headmenu->selectmenu)
		{
			class menuEvent_selall:public guiMenu
			{
			public:
				menuEvent_selall(EventEditor *ed){editor=ed;}
				void MenuFunction(){editor->SelectAll(true);}
				EventEditor *editor;
			};
			headmenu->selectmenu->AddFMenu(Cxs[CXS_SELECTALL],new menuEvent_selall(this),SK_SELECTALL);

			class menuEvent_selalloff:public guiMenu
			{
			public:
				menuEvent_selalloff(EventEditor *ed){editor=ed;}
				void MenuFunction(){editor->SelectAll(false);}
				EventEditor *editor;
			};
			headmenu->selectmenu->AddFMenu(Cxs[CXS_UNSELECTALL],new menuEvent_selalloff(this),SK_DESELECTALL);

			headmenu->selectmenu->AddLine();

			class menu_ddevents:public guiMenu
			{
			public:
				menu_ddevents(EventEditor_Selection *ee){eventeditor=ee;}
				void MenuFunction(){eventeditor->SelectDoubleEvents(0);}
				EventEditor_Selection *eventeditor;
			};

			headmenu->selectmenu->AddFMenu(Cxs[CXS_SELECTALLDOUBLE],new menu_ddevents(this));

			class menu_ddeventspos:public guiMenu
			{
			public:
				menu_ddeventspos(EventEditor_Selection *ee){eventeditor=ee;}
				void MenuFunction(){eventeditor->SelectDoubleEvents(-1);}
				EventEditor_Selection *eventeditor;
			};

			headmenu->selectmenu->AddFMenu(Cxs[CXS_SELECTALLDOUBLEEVENTSPOSITION],new menu_ddeventspos(this));
		}

		headmenu->functionsmenu=headmenu->AddMenu(Cxs[CXS_FUNCTIONS],0);
		if(headmenu->functionsmenu)
		{
			AddFunctionsMenu(headmenu->functionsmenu);
		}

		headmenu->editormenu=headmenu->AddMenu("Editor",0);
		if(headmenu->editormenu)
		{	
			CreateEditorMenu(headmenu->editormenu);
			AddStepMenu(headmenu->editormenu);
		}
	}
}

void EventEditor_Selection::MoveSelectedEventsToMeasure(bool copy)
{
	if(patternselection.FirstSelectedEvent())
	{
		if(EditData *edit=new EditData)
		{
			edit->win=this;
			edit->x=0;
			edit->y=0;

			if(copy==true)
			{
				edit->id=EDIT_COPYMEASURE;
				edit->title=Cxs[CXS_COPYSELEVENTSTOMEASURE];
			}
			else
			{
				edit->id=EDIT_MOVEMEASURE;
				edit->title=Cxs[CXS_MOVESELEVENTSTOMEASURE];
			}

			edit->type=EditData::EDITDATA_TYPE_INTEGER_OKCANCEL;
			edit->from=1;
			edit->to=WindowSong()->GetSongLength_Measure();
			edit->value=1;

			maingui->EditDataValue(edit);
		}
	}
}

void EventEditor_Selection::AddFunctionsMenu(guiMenu *n)
{
	/*
	class menuEvent_quant:public guiMenu
	{
	public:
	menuEvent_quant(EventEditor_Selection *ed){
	editor=ed;
	}

	void MenuFunction(){
	mainedit->QuantizeEvents(editor,&editor->patternselection);
	}

	EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_QUANTIZEEVENTS],new menuEvent_quant(this),"Ctrl+Q");
	*/

	class menuEvent_move:public guiMenu
	{
	public:
		menuEvent_move(EventEditor_Selection *ed){
			editor=ed;
		}

		void MenuFunction(){
			editor->MoveSelectedEventsToMeasure(false);
		}

		EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_MOVESELEVENTSTOMEASURE],new menuEvent_move(this));
	class menuEvent_copy:public guiMenu
	{
	public:
		menuEvent_copy(EventEditor_Selection *ed){
			editor=ed;
		}

		void MenuFunction(){
			editor->MoveSelectedEventsToMeasure(true);
		}

		EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_COPYSELEVENTSTOMEASURE],new menuEvent_copy(this));

	n->AddLine();

	class menuEvent_spmove:public guiMenu
	{
	public:
		menuEvent_spmove(EventEditor_Selection *ed){
			editor=ed;
		}

		void MenuFunction(){
			editor->MoveSelectedEventsToTick(editor->WindowSong()->GetSongPosition(),false);
		}

		EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_MOVESELEVENTSTOSP],new menuEvent_spmove(this));
	class menuEvent_spcopy:public guiMenu
	{
	public:
		menuEvent_spcopy(EventEditor_Selection *ed){
			editor=ed;
		}

		void MenuFunction(){
			editor->MoveSelectedEventsToTick(editor->WindowSong()->GetSongPosition(),true);
		}

		EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_COPYSELEVENTSTOSP],new menuEvent_spcopy(this));
}

int EventEditor_Selection::SelectDoubleEvents(int selflag)
{
	int c=0;

	if(selflag==-1) // Same Position
	{
		Seq_SelectionEvent *sele=patternselection.FirstMixEvent();

		while(sele)
		{
			Seq_SelectionEvent *pn=sele->NextEvent();

			while(pn && pn->seqevent->GetEventStart()==sele->seqevent->GetEventStart())
			{
				if(pn->seqevent->IsSelected()==false && pn->seqevent->GetStatus()==sele->seqevent->GetStatus())
				{
					pn->seqevent->Select();
					c++;
				}

				pn=pn->NextEvent();
			}

			sele=sele->NextEvent();
		}
	}
	else
	{
		Seq_SelectionEvent *sele=patternselection.FirstMixEvent();

		while(sele)
		{
			Seq_SelectionEvent *pn=sele->NextEvent();

			while(pn && pn->seqevent->GetEventStart()==sele->seqevent->GetEventStart())
			{
				if(pn->seqevent->IsSelected()==false && pn->seqevent->Compare(sele->seqevent)==true)
				{
					pn->seqevent->Select();
					c++;
				}

				pn=pn->NextEvent();
			}

			sele=sele->NextEvent();
		}
	}

	if(c)
		maingui->RefreshAllEditorsWithEvent(WindowSong(),0);
	else
		maingui->MessageBoxOk(0,Cxs[CXS_NODOUBLEEVENTSFOUND]);

	return c;
}

void EventEditor_Selection::SelectAll(bool on)
{
	bool found=false;
	Seq_SelectionEvent *e=patternselection.FirstMixEvent();

	while(e)
	{
		if(
			(on==true && e->seqevent->IsSelected()==false) || 
			(on==false && e->seqevent->IsSelected()==true)
			)
		{
			on==true?e->seqevent->flag|=OFLAG_SELECTED:e->seqevent->flag CLEARBIT OFLAG_SELECTED;
			found=true;
		}

		e=e->NextEvent();
	}

	if(found==true)
		maingui->RefreshAllEditorsWithEvent(WindowSong(),0);
}

void EventEditor::AddStepMenu(guiMenu *n)
{
	if(n)
	{
		n->AddLine();
		guiMenu *stepmenu=n->AddMenu("Create Event/s add to Song Position",0);

		if(stepmenu)
		{
			class menu_off:public guiMenu
			{
			public:
				menu_off(EventEditor *ee){eventeditor=ee;}

				void MenuFunction()
				{		
					eventeditor->addsongpositionstep=ADDSTEP_OFF;
					eventeditor->ShowEditorMenu();
				} //

				EventEditor *eventeditor;
			};
			stepmenu->AddFMenu("off",new menu_off(this));

			class menu_q1:public guiMenu
			{
			public:
				menu_q1(EventEditor *ee)
				{
					eventeditor=ee;
				}

				void MenuFunction()
				{		
					eventeditor->addsongpositionstep=ADDSTEP_1;
					eventeditor->ShowEditorMenu();
				} //

				EventEditor *eventeditor;
			};
			stepmenu->AddFMenu("1",new menu_q1(this));

			class menu_q2:public guiMenu
			{
			public:
				menu_q2(EventEditor *ee)
				{
					eventeditor=ee;
				}

				void MenuFunction()
				{		
					eventeditor->addsongpositionstep=ADDSTEP_12;
					eventeditor->ShowEditorMenu();
				} //

				EventEditor *eventeditor;
			};
			stepmenu->AddFMenu("1/2",new menu_q2(this));

			class menu_q4:public guiMenu
			{
			public:
				menu_q4(EventEditor *ee)
				{
					eventeditor=ee;
				}

				void MenuFunction()
				{		
					eventeditor->addsongpositionstep=ADDSTEP_14;
					eventeditor->ShowEditorMenu();
				} //

				EventEditor *eventeditor;
			};
			stepmenu->AddFMenu("1/4",new menu_q4(this));

			class menu_q8:public guiMenu
			{
			public:
				menu_q8(EventEditor *ee)
				{
					eventeditor=ee;
				}

				void MenuFunction()
				{		
					eventeditor->addsongpositionstep=ADDSTEP_18;
					eventeditor->ShowEditorMenu();
				} //

				EventEditor *eventeditor;
			};
			stepmenu->AddFMenu("1/8",new menu_q8(this));

			class menu_q16:public guiMenu
			{
			public:
				menu_q16(EventEditor *ee)
				{
					eventeditor=ee;
				}

				void MenuFunction()
				{		
					eventeditor->addsongpositionstep=ADDSTEP_16;
					eventeditor->ShowEditorMenu();
				} //

				EventEditor *eventeditor;
			};
			stepmenu->AddFMenu("1/16",new menu_q16(this));

			class menu_qfree:public guiMenu
			{
			public:
				menu_qfree(EventEditor *ee)
				{
					eventeditor=ee;
				}

				void MenuFunction()
				{		
					eventeditor->addsongpositionstep=ADDSTEP_DYNAMIC;
					eventeditor->ShowEditorMenu();
				} //

				EventEditor *eventeditor;
			};
			stepmenu->AddFMenu(">Beat",new menu_qfree(this));

			class menu_measure:public guiMenu
			{
			public:
				menu_measure(EventEditor *ee)
				{
					eventeditor=ee;
				}

				void MenuFunction()
				{		
					eventeditor->addsongpositionstep=ADDSTEP_MEASURE;
					eventeditor->ShowEditorMenu();
				} //

				EventEditor *eventeditor;
			};
			stepmenu->AddFMenu(">Measure",new menu_measure(this));
		}
	}
}

void EventEditor_Selection::AddEditMenu(guiMenu *n)
{
	class menuEvent_copy:public guiMenu
	{
	public:
		menuEvent_copy(EventEditor_Selection *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			editor->CopySelectedEvents();
		}

		EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_COPY],new menuEvent_copy(this),SK_COPY);

	class menuEvent_cut:public guiMenu
	{
	public:
		menuEvent_cut(EventEditor_Selection *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			editor->CopySelectedEvents();
			mainedit->DeleteEvents(&editor->patternselection,false);
		}

		EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_CUT],new menuEvent_cut(this),SK_CUT);

	class menuEvent_paste:public guiMenu
	{
	public:
		menuEvent_paste(EventEditor_Selection *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			editor->Paste();
		}

		EventEditor_Selection *editor;
	};

	//if(mainbuffer->CheckBuffer(this,WindowSong(),insertpattern)==true)
	n->AddFMenu(Cxs[CXS_PASTE],new menuEvent_paste(this),SK_PASTE);

	class menuEvent_delete:public guiMenu
	{
	public:
		menuEvent_delete(EventEditor_Selection *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			mainedit->DeleteEvents(&editor->patternselection,false);
		}

		EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_DELETE],new menuEvent_delete(this),SK_DEL);
	n->AddLine();

	class menuEvent_muteselectedevents:public guiMenu
	{
	public:
		menuEvent_muteselectedevents(EventEditor_Selection *ed,bool on)
		{
			editor=ed;
			onoff=on;
		}

		void MenuFunction()
		{
			editor->MuteSelectedEvents(onoff);
		}

		EventEditor_Selection *editor;
		bool onoff;
	};

	class menuEvent_unmuteevents:public guiMenu
	{
	public:
		menuEvent_unmuteevents(EventEditor_Selection *ed)
		{
			editor=ed;
		}

		void MenuFunction()
		{
			editor->UnmuteEvents();
		}

		EventEditor_Selection *editor;
	};

	n->AddFMenu(Cxs[CXS_MUTESELECTEDEVENTS],new menuEvent_muteselectedevents(this,true),SK_MUTE);
	n->AddFMenu(Cxs[CXS_UNMUTESELECTEDEVENTS],new menuEvent_muteselectedevents(this,false),SK_UNMUTE);
	n->AddFMenu(Cxs[CXS_UNMUTEALLEVENTS],new menuEvent_unmuteevents(this));
}

#ifdef OLDIE
bool Editor::InitTempoEdit(Seq_Song *song,Seq_Tempo *tempo)
{
	if(!GetUndoEditEvents())
	{
		if(UndoEditTempo *uet=new UndoEditTempo)
		{
			// Buffer Tempo Events+Data
			if(uet->Init(1)==true)
			{
				uet->event_p[0]=tempo;
				uet->oldindex[0]=0;
				uet->oldevents[0]=(Seq_Event *)tempo->Clone();

				uet->ltempo=uet->htempo=tempo->tempo;
				uet->lpos=uet->hpos=tempo->GetTempoStart();

				SetUndoEditEvents(uet);
				mainedit->EditEvents(song,uet);
				return true;
			}

			delete uet;
			return false;
		}
	}	

	return true;
}
#endif

void GUI::AddUndoMenu(guiMenu *n)
{
	Seq_Song *song=n->GetSong();

	class menu_Undo:public guiMenu
	{
	public:
		void MenuFunction()
		{
			Seq_Song *song=GetSong();

			if(song)
				song->undo.DoUndo();
		} 
	};

	if((!song) || (!song->undo.undo_menustring))
	{
		if(n->menu->undomenu=n->AddFMenu(Cxs[CXS_NOUNDO],new menu_Undo(),SK_UNDO) )
			n->menu->undomenu->Disable();
	}
	else
		n->menu->undomenu=n->AddFMenu(song->undo.undo_menustring,new menu_Undo(),SK_UNDO);

	class menu_Redo:public guiMenu
	{
	public:
		void MenuFunction()
		{
			Seq_Song *song=GetSong();

			if(song)
				song->undo.DoRedo();
		}
	};

	if((!song) || (!song->undo.redo_menustring))
	{
		if(n->menu->redomenu=n->AddFMenu(Cxs[CXS_NOREDO],new menu_Redo(),SK_REDO))
			n->menu->redomenu->Disable();
	}
	else
		n->menu->redomenu=n->AddFMenu(song->undo.redo_menustring,new menu_Redo(),SK_REDO);

	n->AddLine();
}

Seq_Song *guiMenu::GetSong()
{
	guiMenu *p=this;

	while(p && p->menu)
		p=p->menu;

	if((!p->screen) && (!p->window))
		return 0;

	return p->screen?p->screen->GetSong():p->window->WindowSong();
}

void guiMenu::ShowUndoMenu(Seq_Song *song)
{
	if(undomenu)
	{
		if(song)
		{
			if(undomenu->name)delete undomenu->name;
			undomenu->name=0;

			if(!song->undo.undo_menustring)
			{
				undomenu->name=mainvar->GenerateString(Cxs[CXS_NOUNDO]);
				undomenu->Disable(Cxs[CXS_NOUNDO]);
			}
			else
			{
				undomenu->shortkey=mainvar->GenerateString(SK_UNDO);
				undomenu->name=mainvar->GenerateString(song->undo.undo_menustring," \t",undomenu->shortkey);
				undomenu->Enable();
			}
		}
		else
			undomenu->Disable(Cxs[CXS_NOSONGUNDO]);
	}

	if(redomenu)
	{
		if(song)
		{
			if(redomenu->name)delete redomenu->name;
			redomenu->name=0;

			if(!song->undo.redo_menustring)
			{
				redomenu->name=mainvar->GenerateString(Cxs[CXS_NOREDO]);
				redomenu->Disable(Cxs[CXS_NOREDO]);
			}
			else
			{
				redomenu->shortkey=mainvar->GenerateString(SK_REDO);
				redomenu->name=mainvar->GenerateString(song->undo.redo_menustring," \t",redomenu->shortkey);

				redomenu->Enable();
			}
		}
		else
			redomenu->Disable(Cxs[CXS_NOSONGREDO]);
	}
}

void EventEditor::DoubleClickInEditArea()
{
	if(WindowSong())
	{
		InitMousePosition();
		WindowSong()->SetSongPosition(GetMousePosition(),true);
	}
}

void EventEditor::ShowEditorMenu()
{
	/*
	if(mousequantizemenu)
	{
	mousequantizemenu->Select(MOUSEQUANTIZE_MEASURE,false);
	mousequantizemenu->Select(MOUSEQUANTIZE_BEAT,false);
	mousequantizemenu->Select(MOUSEQUANTIZE_1,false);
	mousequantizemenu->Select(MOUSEQUANTIZE_12,false);
	mousequantizemenu->Select(MOUSEQUANTIZE_14,false);
	mousequantizemenu->Select(MOUSEQUANTIZE_18,false);
	mousequantizemenu->Select(MOUSEQUANTIZE_16,false);
	mousequantizemenu->Select(MOUSEQUANTIZE_FREE,false);
	mousequantizemenu->Select(MOUSEQUANTIZE_ZOOM,false);

	mousequantizemenu->Select(mousequantize,true);
	}

	if(displaymenu)
	{
	displaymenu->Select(WINDOWDISPLAY_MEASURE,false);
	displaymenu->Select(WINDOWDISPLAY_SMPTE,false);
	displaymenu->Select(windowdisplay,true);
	}

	if(stepmenu)
	{
	stepmenu->Select(ADDSTEP_OFF,false);
	stepmenu->Select(ADDSTEP_1,false);
	stepmenu->Select(ADDSTEP_12,false);
	stepmenu->Select(ADDSTEP_14,false);
	stepmenu->Select(ADDSTEP_18,false);
	stepmenu->Select(ADDSTEP_16,false);
	stepmenu->Select(ADDSTEP_DYNAMIC,false);
	stepmenu->Select(ADDSTEP_MEASURE,false);

	stepmenu->Select(addsongpositionstep,true);
	}

	if(followmenu)
	followmenu->menu->Select(followmenu->index,followsongposition==true);

	if(syncmenu)
	syncmenu->menu->Select(syncmenu->index,syncsongposition==true);

	ShowUndoMenu();
	*/
}

void EventEditor::RefreshMarker()
{
	//	GenerateOSMenu(true);
	//ShowUndoMenu();
	//	ShowEditorMenu();
}

void EventEditor::AddSetMarkerPositions(guiMenu *menu)
{
	// Marker -> Cycle
	guiMenu *marker=menu->AddMenu(Cxs[CXS_SETCYCLEPOSITIONMARKER],0);

	if(marker)
	{
		class menu_setmk:public guiMenu
		{
		public:
			menu_setmk(EventEditor *ee,Seq_Marker *m)
			{
				eventeditor=ee;
				marker=m;
			}

			void MenuFunction()
			{		
				eventeditor->WindowSong()->SetCycle(marker->GetMarkerStart(),marker->GetMarkerEnd()); // no quantize
			}

			EventEditor *eventeditor;
			Seq_Marker *marker;
		};

		Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();
		while(m)
		{
			marker->AddFMenu(m->CreateFromToString(),new menu_setmk(this,m));
			m=m->NextMarker();
		}
	}

	// Marker -> Cycle
	marker=menu->AddMenu(Cxs[CXS_SETSPWITHMARKERSTART],0);

	if(marker)
	{
		class menu_setmk:public guiMenu
		{
		public:
			menu_setmk(EventEditor *ee,Seq_Marker *m)
			{
				eventeditor=ee;
				marker=m;
			}

			void MenuFunction()
			{		
				eventeditor->WindowSong()->SetSongPosition(marker->GetMarkerStart(),true);
			}

			EventEditor *eventeditor;
			Seq_Marker *marker;
		};

		Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();
		while(m)
		{
			marker->AddFMenu(m->CreateFromString(),new menu_setmk(this,m));
			m=m->NextMarker();
		}
	}
}

void EventEditor::CreateMarkerMenu(guiMenu *menu)
{
	menu->AddLine();

	if(menu && WindowSong()->textandmarker.FirstMarker())
	{
		guiMenu *marker=menu->AddMenu(Cxs[CXS_GOTOMARKER],0);

		if(marker)
		{
			class menu_marker:public guiMenu
			{
			public:
				menu_marker(EventEditor *ee,Seq_Marker *m)
				{
					eventeditor=ee;
					marker=m;
				}

				void MenuFunction()
				{		
					eventeditor->NewStartPosition(marker->GetMarkerStart(),true);
				}

				EventEditor *eventeditor;
				Seq_Marker *marker;
			};

			Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();
			while(m)
			{
				marker->AddFMenu(m->CreateFromString(),new menu_marker(this,m));
				m=m->NextMarker();
			}
		}

		AddSetMarkerPositions(menu);
	}

	class menu_cyclemarker:public guiMenu
	{
	public:
		menu_cyclemarker(EventEditor *ed){editor=ed;}

		void MenuFunction()
		{
			Seq_Marker *m;

			if(m=editor->WindowSong()->textandmarker.AddMarker
				(
				editor->WindowSong()->playbacksettings.cyclestart,
				editor->WindowSong()->playbacksettings.cycleend,
				"Cycle Marker",
				-1
				)
				)
				maingui->RefreshAllEditorsWithMarker(editor->WindowSong(),m);
		} 

		EventEditor *editor;
	};
	menu->AddFMenu(Cxs[CXS_CREATENEWMARKERCP],new menu_cyclemarker(this));

	class menu_cyclemarkersp:public guiMenu
	{
	public:
		menu_cyclemarkersp(EventEditor *ed){editor=ed;}

		void MenuFunction()
		{
			Seq_Marker *m;

			if(m=editor->WindowSong()->textandmarker.AddMarker
				(
				editor->WindowSong()->GetSongPosition(),
				-1,
				"Song Position Marker",
				-1
				)
				)
				maingui->RefreshAllEditorsWithMarker(editor->WindowSong(),m);
		} 

		EventEditor *editor;
	};
	menu->AddFMenu(Cxs[CXS_CREATENEWMARKERSP],new menu_cyclemarkersp(this));

	menu->AddLine();
}

void EventEditor::AddTimeDisplayMenu(guiMenu *popmenu)
{
	guiMenu *s=popmenu->AddMenu(Cxs[CXS_TIMEDISPLAY],0);

	if(s)
	{
		class menu_dsek:public guiMenu
		{
		public:
			menu_dsek(EventEditor *ee)
			{
				eventeditor=ee;
			}

			void MenuFunction()
			{	
				if(eventeditor->windowtimeformat!=WINDOWDISPLAY_SECONDS)
				{
					eventeditor->windowtimeformat=WINDOWDISPLAY_SECONDS;
					eventeditor->NewTimeFormat();
				}
			} //

			EventEditor *eventeditor;
		};
		s->AddFMenu(Cxs[CXS_SECONDS],new menu_dsek(this),windowtimeformat==WINDOWDISPLAY_SECONDS?true:false);

		class menu_dsamples:public guiMenu
		{
		public:
			menu_dsamples(EventEditor *ee){eventeditor=ee;}

			void MenuFunction()
			{	
				if(eventeditor->windowtimeformat!=WINDOWDISPLAY_SAMPLES)
				{
					eventeditor->windowtimeformat=WINDOWDISPLAY_SAMPLES;
					eventeditor->NewTimeFormat();
				}
			} //

			EventEditor *eventeditor;
		};
		s->AddFMenu("Samples",new menu_dsamples(this),windowtimeformat==WINDOWDISPLAY_SAMPLES?true:false);

		if(songmodepossible==true)
		{
			class menu_dmeasure:public guiMenu
			{
			public:
				menu_dmeasure(EventEditor *ee)
				{
					eventeditor=ee;
				}

				void MenuFunction()
				{	
					if(eventeditor->windowtimeformat!=WINDOWDISPLAY_MEASURE)
					{
						eventeditor->windowtimeformat=WINDOWDISPLAY_MEASURE;
						eventeditor->NewTimeFormat();
					}
				} //

				EventEditor *eventeditor;
			};

			s->AddFMenu(Cxs[CXS_MEASURE],new menu_dmeasure(this),windowtimeformat==WINDOWDISPLAY_MEASURE?true:false);
		}

		class menu_dsmpte:public guiMenu
		{
		public:
			menu_dsmpte(EventEditor *ee)
			{
				eventeditor=ee;
			}

			void MenuFunction()
			{	
				if(eventeditor->windowtimeformat!=WINDOWDISPLAY_SMPTE)
				{
					eventeditor->windowtimeformat=WINDOWDISPLAY_SMPTE;
					eventeditor->NewTimeFormat();
				}
			} //

			EventEditor *eventeditor;
		};

		if(char *h=mainvar->GenerateString("Frames (",smpte_modestring[WindowSong()->project->standardsmpte],")"))
		{
			s->AddFMenu(h,new menu_dsmpte(this),windowtimeformat==WINDOWDISPLAY_SMPTE?true:false);
			delete h;
		}


	}
}

void EventEditor::AddMouseQuantizeMenu(guiMenu *hmenu)
{

	guiMenu *menu=hmenu->AddMenu(Cxs[CXS_MOUSEQUANTIZE],0);

	if(!menu)
		return;

	class menu_measure:public guiMenu
	{
	public:
		menu_measure(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_MEASURE;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};
	menu->AddFMenu(Cxs[CXS_MEASURE],new menu_measure(this),mousequantize==MOUSEQUANTIZE_MEASURE?true:false);

	class menu_beat:public guiMenu
	{
	public:
		menu_beat(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_BEAT;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};
	menu->AddFMenu(Cxs[CXS_BEAT],new menu_beat(this),mousequantize==MOUSEQUANTIZE_BEAT?true:false);

	menu->AddLine();
	class menu_frame:public guiMenu
	{
	public:
		menu_frame(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_FRAME;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};

	menu->AddFMenu("Frame",new menu_frame(this),mousequantize==MOUSEQUANTIZE_FRAME?true:false);

	class menu_qframe:public guiMenu
	{
	public:
		menu_qframe(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_QFRAME;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};

	menu->AddFMenu("QFrame",new menu_qframe(this),mousequantize==MOUSEQUANTIZE_QFRAME?true:false);
	menu->AddLine();

	class menu_q1:public guiMenu
	{
	public:
		menu_q1(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_1;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};
	menu->AddFMenu("1",new menu_q1(this),mousequantize==MOUSEQUANTIZE_1?true:false);

	class menu_q2:public guiMenu
	{
	public:
		menu_q2(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_12;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};
	menu->AddFMenu("1/2",new menu_q2(this),mousequantize==MOUSEQUANTIZE_12?true:false);

	class menu_q4:public guiMenu
	{
	public:
		menu_q4(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_14;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};
	menu->AddFMenu("1/4",new menu_q4(this),mousequantize==MOUSEQUANTIZE_14?true:false);

	class menu_q8:public guiMenu
	{
	public:
		menu_q8(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_18;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};
	menu->AddFMenu("1/8",new menu_q8(this),mousequantize==MOUSEQUANTIZE_18?true:false);

	class menu_q16:public guiMenu
	{
	public:
		menu_q16(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_16;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};
	menu->AddFMenu("1/16",new menu_q16(this),mousequantize==MOUSEQUANTIZE_16?true:false);

	menu->AddLine();
	class menu_qfree:public guiMenu
	{
	public:
		menu_qfree(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_FREE;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};
	menu->AddFMenu(Cxs[CXS_FREE],new menu_qfree(this),mousequantize==MOUSEQUANTIZE_FREE?true:false);

	class menu_qzoom:public guiMenu
	{
	public:
		menu_qzoom(EventEditor *ee)
		{
			eventeditor=ee;
		}

		void MenuFunction()
		{		
			mainsettings->defaultmousequantize=eventeditor->mousequantize=MOUSEQUANTIZE_ZOOM;
			eventeditor->ShowEditorMenu();
		} //

		EventEditor *eventeditor;
	};

	char *h="?";
	switch(this->GetTimeLineGrid())
	{
	case TICK16nd:
		h="1/16";
		break;

	case TICK32nd:
		h="1/32";
		break;

	case TICK64nd:
		h="1/64";
		break;
	}

	char *h2=mainvar->GenerateString(Cxs[CXS_SONGGRID],":",h);

	if(h2)
	{
		menu->AddFMenu(h2,new menu_qzoom(this),mousequantize==MOUSEQUANTIZE_ZOOM?true:false);
		delete h2;
	}
}

void EventEditor::CreateEditorMenu(guiMenu *menu)
{	
	if(menu)
	{
		/*
		if(GetEditorID()!=EDITORTYPE_TEMPO &&
		GetEditorID()!=EDITORTYPE_MARKER &&
		GetEditorID()!=EDITORTYPE_TEXT
		)
		{
		menu->AddFMenu("Event",new globmenu_Event(this),"F1");
		menu->AddFMenu("Piano",new globmenu_Piano(this),"F2");
		menu->AddFMenu("Wave",new globmenu_Wave(this),"F3");
		menu->AddFMenu("Drum",new globmenu_Drum(this),"F4");
		//	menu->AddFMenu("Score",new globmenu_Score(this),"F5");
		menu->AddFMenu("Sample",new globmenu_Sample(this),"F6");
		}*/


		//menu->AddFMenu("Arrange Editor",new globmenu_ArrangeSong(this));
		//menu->AddFMenu("Transport",new globmenu_Transport());
		//menu->AddFMenu("Big Time Display",new globmenu_Bigtime);

		CreateMarkerMenu(menu);

		mousequantizemenu=menu->AddMenu(Cxs[CXS_MOUSEQUANTIZE],0);

		if(mousequantizemenu)
		{
			AddMouseQuantizeMenu(mousequantizemenu);
		}

		class menu_followsongposition:public guiMenu
		{
		public:
			menu_followsongposition(EventEditor *ee)
			{
				eventeditor=ee;
			}

			void MenuFunction()
			{	
				eventeditor->followsongposition=eventeditor->followsongposition==true?false:true;
				eventeditor->ShowEditorMenu();
			} //

			EventEditor *eventeditor;
		};

		menu->AddFMenu(Cxs[CXS_EDITORFOLLOWSSP],followmenu=new menu_followsongposition(this),followsongposition);

		class menu_syncsongpositions:public guiMenu
		{
		public:
			menu_syncsongpositions(EventEditor *ee){eventeditor=ee;}

			void MenuFunction()
			{	
				eventeditor->syncsongposition=eventeditor->syncsongposition==true?false:true;
				eventeditor->ShowEditorMenu();
			} //

			EventEditor *eventeditor;
		};

		menu->AddFMenu(Cxs[CXS_SNYCEDITOR],syncmenu=new menu_syncsongpositions(this),syncsongposition);
	}
}

guiGadget *EventEditor::CheckControlBox(guiGadget *g)
{
	if(!guictrlbox.glist)return g;

	switch(g->gadgetID)
	{
	case GADGETID_CONTROLBOX_STOP:
		WindowSong()->StopSelected();
		break;

	case GADGETID_CONTROLBOX_START:
		{
			WindowSong()->PlaySong();
		}
		break;

	case GADGETID_CONTROLBOX_RECORD:
		{
			WindowSong()->RecordSong();
		}
		break;

	case GADGETID_CONTROLBOX_TIME:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class time:public guiMenu
				{
				public:
					time(EventEditor *e,int ttype)
					{
						ed=e;
						timetype=ttype;
					}

					void MenuFunction()
					{	
						if(ed->windowtimeformat!=timetype)
						{
							mainsettings->editordefaulttimeformat=ed->windowtimeformat=timetype;
							ed->NewTimeFormat();
						}
					} //

					EventEditor *ed;
					int timetype;
				};

				popmenu->AddFMenu(Cxs[CXS_MEASURE],new time(this,WINDOWDISPLAY_MEASURE),windowtimeformat==WINDOWDISPLAY_MEASURE?true:false);

				char *h=mainvar->GenerateString("SMPTE (",smpte_modestring[WindowSong()->project->standardsmpte],")");

				if(h)
				{
					popmenu->AddFMenu(h,new time(this,WINDOWDISPLAY_SMPTE),windowtimeformat==WINDOWDISPLAY_SMPTE?true:false);
					delete h;
				}

				ShowPopMenu();
			}
		}
		break;

	default:
		return g;
	}

	return 0;
}

guiControlBox::guiControlBox()
{
	status=Seq_Song::STATUS_STOP;
	x=0;
	x2=x+DEFAULT_TOOLSIZE;
	ResetGadgets();
};

void guiControlBox::ResetGadgets()
{
	glist=0;
	startbutton=
		stopbutton=
		recordbutton=
		timebutton=0;
}

int guiControlBox::GetY2(int hy)
{
	if(glist)
	{
		if(guiGadget *fg=glist->FirstGadget())
			return fg->y2+1;
	}

	return hy;
}

#ifdef OLDIE
void guiControlBox::CreateControlBox(guiWindow *win)
{
	ResetGadgets();

	if(glist=win->gadgetlists.AddGadgetList((Editor *)win))
	{
		int xp=x,yp=y,size=x2-x;

		size/=4;

		status=Seq_Song::STATUS_STOP;

		stopbutton=glist->AddImageButton(xp,yp,xp+size-1,yp+30,IMAGE_STOPBUTTON_SMALL_ON,GADGETID_CONTROLBOX_STOP,0,Cxs[CXS_STOPSONGORCYL]);
		xp+=size;
		startbutton=glist->AddImageButton(xp,yp,xp+size-1,yp+30,IMAGE_PLAYBUTTON_SMALL_OFF,GADGETID_CONTROLBOX_START,0,Cxs[CXS_STARTSONGPLAYBACK]);
		xp+=size;
		recordbutton=glist->AddImageButton(xp,yp,xp+size-1,yp+30,IMAGE_RECORDBUTTON_SMALL_OFF,GADGETID_CONTROLBOX_RECORD,0,Cxs[CXS_STARTSONGREC]);
		xp+=size;

		timebutton=glist->AddButton(xp,yp,x2,yp+30,GADGETID_CONTROLBOX_TIME,0,Cxs[CXS_SELECTTIMEDISPLAY]);

		//x2=xp+30;
		y2=yp+30;

		ShowTime();
		ShowStatus(win->WindowSong()->status);
	}
}
#endif

guiToolBox::guiToolBox()
{
	status=0;
	editor=0;

	timetype=selectgadget=drawgadget=editgadget=cutgadget=deletegadget=rangegadget=statustext=overview=sppsync=xmove=ymove=quantgadget=0;
}

void guiToolBox::RefreshRealtime()
{
	if(sppsync && 
		(sppstatus!=editor->followsongposition || sppstopped!=editor->followsongposition_stopped)
		)
	{
		if(sppstopped!=editor->followsongposition_stopped)
		{
			sppstopped=editor->followsongposition_stopped;
			sppsync->ChangeButtonText(sppstopped==true?"[X]SPP":"...SP");
		}

		sppstatus=editor->followsongposition;
		sppsync->Toggle(sppstatus);
	}
}

void guiToolBox::ShowStatus()
{
	// off
	if(selectgadget)
		selectgadget->ChangeButtonImage(editor->selectedmousemode==EM_SELECT?IMAGE_TOOLBOX_MOUSE:IMAGE_TOOLBOX_MOUSE_NSEL,editor->selectedmousemode==EM_SELECT?MODE_TOGGLE:0);

	if(deletegadget)
		deletegadget->ChangeButtonImage(editor->selectedmousemode==EM_DELETE?IMAGE_TOOLBOX_DELETE:IMAGE_TOOLBOX_DELETE_NSEL,editor->selectedmousemode==EM_DELETE?MODE_TOGGLE:0);

	if(editgadget)
		editgadget->ChangeButtonImage(editor->selectedmousemode==EM_EDIT?IMAGE_TOOLBOX_EDIT:IMAGE_TOOLBOX_EDIT_NSEL,editor->selectedmousemode==EM_EDIT?MODE_TOGGLE:0);

	if(cutgadget)
		cutgadget->ChangeButtonImage(editor->selectedmousemode==EM_CUT?IMAGE_TOOLBOX_CUT:IMAGE_TOOLBOX_CUT_NSEL,editor->selectedmousemode==EM_CUT?MODE_TOGGLE:0);

	if(drawgadget)
		drawgadget->ChangeButtonImage(editor->selectedmousemode==EM_CREATE?IMAGE_TOOLBOX_PENCIL:IMAGE_TOOLBOX_PENCIL_NSEL,editor->selectedmousemode==EM_CREATE?MODE_TOGGLE:0);

	if(statustext)
	{
		switch(editor->selectedmousemode)
		{
		case EM_SELECT:
			statustext->ChangeButtonText(Cxs[CXS_SELECT]);
			break;

		case EM_DELETE:
			statustext->ChangeButtonText(Cxs[CXS_DELETE]);
			break;

		case EM_EDIT:
			statustext->ChangeButtonText(Cxs[CXS_EDIT]);
			break;

		case EM_CUT:
			statustext->ChangeButtonText(Cxs[CXS_CUTSPLIT]);
			break;

		case EM_CREATE:
			statustext->ChangeButtonText(Cxs[CXS_CREATE]);
			break;
		}
	}

	if(overview)
	{
		switch(editor->overviewmode)
		{
		case OV_HORZ:
			overview->ChangeButtonText("Ov >");
			break;

		case OV_VERT:
			overview->ChangeButtonText("Ov ^");
			break;

		case OV_BOTH:
			overview->ChangeButtonText("Ov ^>");
			break;
		}
	}

	if(xmove)
		xmove->Toggle(editor->xmove);

	if(ymove)
		ymove->Toggle(editor->ymove);
}

void guiToolBox::ShowTimeType()
{
	if(timetype && editor && editor->WindowSong())
	{
		switch(editor->windowtimeformat)
		{
		case WINDOWDISPLAY_SECONDS:
			timetype->ChangeButtonText(Cxs[CXS_SECONDS]);
			break;

		case WINDOWDISPLAY_SAMPLES:
			timetype->ChangeButtonText("Samples");
			break;

		case WINDOWDISPLAY_MEASURE:
			timetype->ChangeButtonText(Cxs[CXS_MEASURE]);
			break;

		case WINDOWDISPLAY_SMPTE:
			{
				if(char *h=mainvar->GenerateString("Frames (",smpte_modestring[ editor->WindowSong()->project->standardsmpte],")"))
				{
					timetype->ChangeButtonText(h);
					delete h;
				}
			}
			break;
		}
	}
}

bool guiToolBox::Key_Down(int key)
{
	switch(key)
	{
	case '1':
		editor->CheckToolBox(selectgadget);
		break;

	case '2':
		editor->CheckToolBox(drawgadget);
		break;

	case '3':
		editor->CheckToolBox(editgadget);
		break;

	case '4':
		editor->CheckToolBox(cutgadget);
		break;

	case '5':
		editor->CheckToolBox(deletegadget);
		break;
	}

	return false;
}

bool guiToolBox::CreateToolBox(int types,int iflag)
{
	edittypes=types;
	box_ok=true;

	int c=0;
	if(types&TOOLBOXTYPE_SELECT)
		c++;
	if(types&TOOLBOXTYPE_CREATE)
		c++;

	if(types&TOOLBOXTYPE_EDIT)
		c++;

	if(types&TOOLBOXTYPE_CUT)
		c++;

	if(types&TOOLBOXTYPE_DELETE)
		c++;

	if(types&TOOLBOXTYPE_RANGE)
		c++;

	int sizex=editor->INFOSIZE;

	//sizey=2*maingui->GetFontSizeY();

	if(!(types&TOOLBOXTYPE_NOWINDOWCHANGING))
	{
		// Add Close/etc

		editor->glist.AddImageButton(-1,-1,sizex/2,-1,IMAGE_CLOSEWINDOW,GADGETID_TOOLBOX_CLOSEWINDOW,MODE_OS);
		editor->glist.AddLX();
		editor->glist.AddImageButton(-1,-1,sizex/2,-1,editor->parentformchild?IMAGE_WINDOWUP:IMAGE_WINDOWDOWN,GADGETID_TOOLBOX_RELEASEWINDOW,MODE_OS);
		//	editor->glist.AddButton(-1,-1,sizex,sizey,"[/]",GADGETID_TOOLBOX_RELEASEWINDOW,0);
		editor->glist.AddLX();
		//	editor->glist.AddButton(-1,-1,sizex,sizey,"[#]",GADGETID_TOOLBOX_MAXWINDOW,0);
		//		editor->glist.AddLX();
	}

	if(types&TOOLBOXTYPE_ARRANGEMENUS)
	{
		editor->glist.AddButton(-1,-1,sizex,-1,"Tracks",GADGETID_TOOLBOX_MENU,MODE_MENU); // Menu 1
		editor->glist.AddLX();
		editor->glist.AddButton(-1,-1,sizex,-1,"Pattern",GADGETID_TOOLBOX_MENU2,MODE_MENU); // Menu 2
		editor->glist.AddLX();
	}
	else
		if(types&TOOLBOXTYPE_AUDIOMIXER)
		{
			editor->glist.AddButton(-1,-1,sizex,-1,"Tracks",GADGETID_TOOLBOX_MENU,MODE_MENU); // Menu 1
			editor->glist.AddLX();

			editor->glist.AddButton(-1,-1,sizex,-1,"Bus",GADGETID_TOOLBOX_MENU2,MODE_MENU); // Menu 2
			editor->glist.AddLX();
		}
		else // Default Menu
		{
			editor->glist.AddButton(-1,-1,sizex,-1,editor->GetMenuName(),GADGETID_TOOLBOX_MENU,MODE_MENU);
			editor->glist.AddLX();
		}

		editor->glist.AddButton(-1,-1,sizex,-1,"Goto",GADGETID_TOOLBOX_GOTO,MODE_MENU);
		editor->glist.AddLX();

		if(iflag&CTB_NOSPP)
		{
			//timetype=0;
		}
		else
		{
			timetype=editor->glist.AddButton(-1,-1,2*sizex,-1,"-",GADGETID_TOOLBOX_TIMETYPE,MODE_MENU|MODE_TEXTCENTER,Cxs[CXS_TIMEDISPLAY]);
			editor->glist.AddLX();
			ShowTimeType();
		}

		if(c)
		{
			if(types&TOOLBOXTYPE_SELECT)
			{
				selectgadget=editor->glist.AddImageButton(-1,-1,sizex/2,-1,IMAGE_TOOLBOX_MOUSE,GADGETID_TOOLBOX_MOUSE,0,Cxs[CXS_SELECT],'1');
				editor->glist.AddLX();

				if(!selectgadget)
					box_ok=false;
			}

			if((types&TOOLBOXTYPE_CREATE) && box_ok==true)
			{
				drawgadget=editor->glist.AddImageButton(-1,-1,sizex/2,-1,IMAGE_TOOLBOX_PENCIL,GADGETID_TOOLBOX_PENCIL,0,Cxs[CXS_CREATE],'2');

				editor->glist.AddLX();
				if(!drawgadget)
					box_ok=false;
				else
					drawgadget->key.SetKey('2');
			}

			if((types&TOOLBOXTYPE_EDIT) && box_ok==true)
			{
				editgadget=editor->glist.AddImageButton(-1,-1,sizex/2,-1,IMAGE_TOOLBOX_EDIT,GADGETID_TOOLBOX_EDIT,0,Cxs[CXS_EDIT],'3');
				editor->glist.AddLX();
				if(!editgadget)
					box_ok=false;
			}

			if((types&TOOLBOXTYPE_CUT) && box_ok==true)
			{
				cutgadget=editor->glist.AddImageButton(-1,-1,sizex/2,-1,IMAGE_TOOLBOX_CUT,GADGETID_TOOLBOX_CUT,0,Cxs[CXS_CUTSPLIT],'4');
				editor->glist.AddLX();

				if(!cutgadget)
					box_ok=false;
			}

			if((types&TOOLBOXTYPE_DELETE) && box_ok==true)
			{
				deletegadget=editor->glist.AddImageButton(-1,-1,sizex/2,-1,IMAGE_TOOLBOX_DELETE,GADGETID_TOOLBOX_DELETE,0,Cxs[CXS_DELETE],'5');

				editor->glist.AddLX();
				if(!deletegadget)
					box_ok=false;
			}



			/*
			if((types&TOOLBOXTYPE_RANGE) && box_ok==true)
			{
			rangegadget=editor->glist.AddImageButton(-1,-1,sizex/2,-1,IMAGE_TOOLBOX_SELECT,GADGETID_TOOLBOX_SELECT,0,"Range");
			editor->glist.AddLX();
			if(!rangegadget)
			box_ok=false;
			}
			*/

			//	y=2*maingui->GetFontSizeY()+2;

			if(box_ok==true)
			{
				double h=sizex;

				h*=1.5;

				statustext=editor->glist.AddText(-1,-1,(int)h,-1,0,GADGETID_TOOLBOX_TEXT,MODE_TEXTCENTER,Cxs[CXS_STATUSMOUSE]);
				editor->glist.AddLX();

				if(types&TOOLBOXTYPE_XY)
				{
					xmove=editor->glist.AddButton(-1,-1,sizex/2,-1,"X",GADGETID_TOOLBOX_X,MODE_TEXTCENTER|MODE_TOGGLE|MODE_BOLD,Cxs[CXS_XMOVE]);
					editor->glist.AddLX();
					ymove=editor->glist.AddButton(-1,-1,sizex/2,-1,"Y",GADGETID_TOOLBOX_Y,MODE_TEXTCENTER|MODE_TOGGLE|MODE_BOLD,Cxs[CXS_YMOVE]);
					editor->glist.AddLX();
				}

				/*
				if(types&TOOLBOXTYPE_FOLLOWSPP)
				{
				if(editor->followsongposition==true && editor->followsongposition_stopped==false)
				timesync_status=IMAGE_TIME_ON;
				else
				timesync_status=IMAGE_TIME_OFF;

				sppsync=editor->glist.AddImageButton(-1,-1,sizex,sizey,timesync_status,GADGETID_TOOLBOX_SPP,0,Cxs[CXS_SCROLLEDITORWITHSONGPOSITION]);
				editor->glist.AddLX();
				}
				*/

				if((types&TOOLBOXTYPE_QUANTIZE) && box_ok==true)
				{
					quantgadget=editor->glist.AddButton(-1,-1,sizex/2,-1,"Qt",GADGETID_TOOLBOX_QUANTIZE,MODE_MENU,Cxs[CXS_QUANTIZE]);

					editor->glist.AddLX();
					if(!quantgadget)
						box_ok=false;
				}

				if(iflag&CTB_NOOVERVIEWVERT)
				{
					overview=0;
				}
				else
					if(types&TOOLBOXTYPE_OVERVIEW)
					{
						overview=editor->glist.AddButton(-1,-1,sizex-sizex/3,-1,GADGETID_TOOLBOX_OVERVIEW,0,Cxs[CXS_OVERVIEWMOUSE]);
						editor->glist.AddLX();
					}

					if(iflag&CTB_NOSPP)
					{
						sppsync=0;
					}
					else
						if(types&TOOLBOXTYPE_FOLLOWSPP)
						{
							sppstatus=editor->followsongposition;
							sppstopped=editor->followsongposition_stopped;

							sppsync=editor->glist.AddButton(-1,-1,sizex-sizex/3,-1,sppstopped==true?"[X]SPP":"...SP",GADGETID_TOOLBOX_SPP,sppstatus==true?MODE_TOGGLE|MODE_TOGGLED|MODE_AUTOTOGGLE:MODE_TOGGLE|MODE_AUTOTOGGLE,Cxs[CXS_SCROLLEDITORWITHSONGPOSITION]);
							editor->glist.AddLX();
							editor->glist.AddButton(-1,-1,sizex-sizex/3,-1,"[>SP",GADGETID_GOTOSONGPOSITION,0,Cxs[CXS_SCROLLEDITORTOSPP]);
							editor->glist.AddLX();
							editor->glist.AddButton(-1,-1,sizex-sizex/3,-1,"[CyL",GADGETID_GOTOCYCLELEFT,0,Cxs[CXS_SCROLLEDITORTOCYCLELEFT]);
							editor->glist.AddLX();
							editor->glist.AddButton(-1,-1,sizex-sizex/3,-1,"CyR]",GADGETID_GOTOCYCLERIGHT,0,Cxs[CXS_SCROLLEDITORTOCYCLERIGHT]);
							editor->glist.AddLX();
						}

						if(Seq_SelectionList *ps=editor->GetWindowPatternSelection())
						{
							editor->glist.AddButton(-1,-1,sizex-sizex/3,-1,"SOLO >",GADGETID_TOOLBOX_SOLOPLAYBACK,ps->playsolo==true?MODE_AUTOTOGGLE|MODE_TOGGLE|MODE_TOGGLED:MODE_AUTOTOGGLE|MODE_TOGGLE,Cxs[CXS_PLAYPATTERNSOLO]);
						}
			}

			ShowStatus();
		}
		else box_ok=false;

		return box_ok;
}

guiGadget *guiWindow::CheckToolBox(guiGadget *g)
{
	if(!g)
		return 0;

	guiGadget *rg=g;

	switch(g->gadgetID)
	{
	case GADGETID_TOOLBOX_X:
		xmove=xmove==true?false:true;
		rg=0;
		break;

	case GADGETID_TOOLBOX_Y:
		ymove=ymove==true?false:true;
		rg=0;
		break;

	case GADGETID_TOOLBOX_SOLOPLAYBACK:
		{
			if(Seq_SelectionList *ps=GetWindowPatternSelection())
			{
				ps->playsolo=ps->playsolo==true?false:true;
				WindowSong()->TempSoloOnOff(this,ps);
			}

			rg=0;
		}
		break;

	case GADGETID_TOOLBOX_TIMETYPE:
		CreateTimeTypePopup();
		return 0;
		break;

	case GADGETID_TOOLBOX_CLOSEWINDOW:
		{
			closeit=true;
			return 0;
		}
		break;

	case GADGETID_TOOLBOX_RELEASEWINDOW:
		{
			maingui->ToggleChildDesktop(this);
			g->ChangeButtonImage(parentformchild?IMAGE_WINDOWUP:IMAGE_WINDOWDOWN);
			return 0;
		}
		break;

		/*
		case GADGETID_TOOLBOX_MAXWINDOW:
		{
		maingui->ToggleChildDesktop(this,true);
		}
		break;
		*/

	case GADGETID_TOOLBOX_MOUSE:
		selectedmousemode=mousemode=EM_SELECT;
		rg=0;
		break;

	case GADGETID_TOOLBOX_EDIT:
		selectedmousemode=mousemode=EM_EDIT;
		rg=0;
		break;

	case GADGETID_TOOLBOX_PENCIL:
		selectedmousemode=mousemode=EM_CREATE;
		rg=0;
		break;

	case GADGETID_TOOLBOX_CUT:
		selectedmousemode=mousemode=EM_CUT;
		rg=0;
		break;

	case GADGETID_TOOLBOX_DELETE:
		selectedmousemode=mousemode=EM_DELETE;
		rg=0;
		break;

	case GADGETID_TOOLBOX_SPP:
		{
			if(followsongposition_stopped==true)
			{
				followsongposition_stopped=false;
			}
			else
				followsongposition=followsongposition==true?false:true;
			//=false; // Reset
			rg=0;
		}
		break;

	case GADGETID_TOOLBOX_MENU:
		{
			CreateMenu();
			ShowPopMenu();
			return 0;
		}
		break;

	case GADGETID_TOOLBOX_MENU2:
		{
			CreateMenu2();
			ShowPopMenu();
			return 0;
		}
		break;

	case GADGETID_TOOLBOX_GOTO:
		{
			CreateGotoMenu();
			if(popmenu)
				ShowPopMenu();

			return 0;
		}
		break;

	case GADGETID_TOOLBOX_OVERVIEW:
		{
			if(overviewmode==OV_HORZ)
				overviewmode=OV_VERT;
			else
				if(overviewmode==OV_VERT)
					overviewmode=OV_BOTH;
				else
					overviewmode=OV_HORZ;

			rg=0;
		}
		break;

	default:
		return g;
	}

	guitoolbox.ShowStatus();

	return rg;
}

void EventEditor::SyncWithOtherEditors()
{
	if(syncsongposition==true) 
	{
		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			if(w->WindowSong()==WindowSong() && w!=this && w->songmode==true)
			{
				switch(w->GetEditorID())
				{
				case EDITORTYPE_EVENT:
				case EDITORTYPE_PIANO:
				case EDITORTYPE_ARRANGE:
				case EDITORTYPE_DRUM:
				case EDITORTYPE_WAVE:
				case EDITORTYPE_SCORE:
				case EDITORTYPE_TEMPO:
				case EDITORTYPE_SAMPLE:
					((EventEditor *)w)->NewStartPosition(startposition,true);
					w->UserEdit();
					break;
				}
			}

			w=w->NextWindow();
		}
	}
}

void EventEditor::Editor_KeyUp()
{
	int back=nVirtKey;

	nVirtKey=0;

	switch(back)
	{
	case KEY_RETURN:
		//case KEY_ENTER:
		{
			if(timeline)
			{
				if(GetMousePosition()!=-1)
					WindowSong()->SetSongPosition(GetMousePosition(),true);
				else
					nVirtKey=back;
			}
		}
		break;

	default:
		nVirtKey=back;
		break;
	}
}

void EventEditor::Editor_KeyDown()
{
	if(guitoolbox.Key_Down(nVirtKey)==true)
	{
		nVirtKey=0;
		return;
	}

	int back=nVirtKey;

	nVirtKey=0;

	switch(back)
	{
	case KEY_TAB: // TAB
		{
			if(timeline)
			{
				if(maingui->GetShiftKey()==false)
				{
					// ->
					int smeasure=WindowSong()->timetrack.ConvertTicksToMeasure(startposition),
						measure=WindowSong()->timetrack.ConvertTicksToMeasure(endposition);

					if(measure==smeasure)
						measure++;

					OSTART h=WindowSong()->timetrack.ConvertMeasureToTicks(measure);

					if(NewStartPosition(h,true)==true)
					{
						UserEdit();
						SyncWithOtherEditors();
					}
				}
				else
				{
					// <-
					int endmeasure=WindowSong()->timetrack.ConvertTicksToMeasure(endposition),
						startmeasure=WindowSong()->timetrack.ConvertTicksToMeasure(startposition);

					int size=endmeasure-startmeasure;
					if(size<1)size=1;

					OSTART h=startmeasure-size>0?WindowSong()->timetrack.ConvertMeasureToTicks(startmeasure-size):0;

					if(NewStartPosition(h,true)==true)
					{
						UserEdit();
						SyncWithOtherEditors();
					}

				}
			}
		}
		break;

	case KEY_MINUS10: //- 
		{
			if(vertzoomgadget && zoomy>vertzoomgadget->from)
			{
				zoomvert=true;
				if(ZoomGFX(zoomy-1,false)==true)
					vertzoomgadget->ChangeSlider(zoomy);	

				zoomvert=false;
			}
		}
		break;

	case KEY_UP10:
	case KEY_CURSORUP:
		if(maingui->GetShiftKey()==true)
		{
			if(vertzoomgadget && zoomy>vertzoomgadget->from)
			{
				zoomvert=true;
				if(ZoomGFX(zoomy-1,false)==true)
					vertzoomgadget->ChangeSlider(zoomy);

				zoomvert=false;
			}
		}
		else
			nVirtKey=back;

		break;

	case KEY_ADD10: // + 10er
		{
			if(vertzoomgadget && zoomy<vertzoomgadget->to)
			{
				zoomvert=true;
				if(ZoomGFX(zoomy+1,false)==true)
					vertzoomgadget->ChangeSlider(zoomy);

				zoomvert=false;
			}
		}
		break;

	case KEY_DOWN10:
	case KEY_CURSORDOWN:
		if(maingui->GetShiftKey()==true)
		{
			if(vertzoomgadget && zoomy<vertzoomgadget->to)
			{
				zoomvert=true;
				if(ZoomGFX(zoomy+1,false)==true)
					vertzoomgadget->ChangeSlider(zoomy);

				zoomvert=false;
			}
		}
		else
			nVirtKey=back;
		break;

	case KEY_ESCAPE:
		{
			if(repeatkey==false)
				EditCancel(); // or RMB
		}
		break;

	case KEY_DIVIDE: // 10er /
	case KEY_MULTIPLY: // 10er *
		{
			for(int i=0;i<mainvar->numberwinzooms;i++)
			{
				if(zoom==&guizoom[i]){
					if(SetZoomGFX(back==KEY_DIVIDE?i-1:i+1,true)==true)
					{
						NewZoom();
						if(horzzoomgadget)horzzoomgadget->ChangeSlider(zoom->index);
					}
					break;
				}
			}
		}
		break;

	case KEY_LEFT10:
	case KEY_CURSORLEFT:
		{
			if(maingui->GetShiftKey()==true)
			{
				for(int i=0;i<mainvar->numberwinzooms;i++)
				{
					if(zoom==&guizoom[i]){
						if(SetZoomGFX(i-1,true)==true)
						{
							NewZoom();
							if(horzzoomgadget)horzzoomgadget->ChangeSlider(zoom->index);
						}
						break;
					}
				}
			}
			else
				if(timeline)
				{
					Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(startposition);

					if(sig)
					{
						OSTART h=startposition;

						h-=sig->measurelength;
						if(h<0)h=0;

						if(NewStartPosition(h,true)==true)
						{
							UserEdit();
							SyncWithOtherEditors();
						}
					}
				}
		}
		break;

	case KEY_RIGHT10:
	case KEY_CURSORRIGHT:
		{
			if(maingui->GetShiftKey()==true)
			{
				for(int i=0;i<mainvar->numberwinzooms;i++)
				{
					if(zoom==&guizoom[i]){
						if(SetZoomGFX(i+1,true)==true)
						{
							NewZoom();
							if(horzzoomgadget)horzzoomgadget->ChangeSlider(zoom->index);
						}
						break;
					}
				}
			}
			else
				if(timeline)
				{
					if(Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(startposition))
					{
						OSTART h=startposition;
						h+=sig->measurelength;

						if(NewStartPosition(h,true)==true)
						{
							UserEdit();
							SyncWithOtherEditors();
						}
					}
				}
		}
		break;

	default:
		nVirtKey=back;
		/*
		if(repeatkey==false)
		{
		guiGadgetList *gl=gadgetlists.FirstGadgetList();

		while(gl)
		{
		guiGadget *g=gl->FirstGadget();
		while(g)
		{
		if(g->key.use==true && g->key.key==nVirtKey)
		{
		Gadget(g);
		return;
		}

		g=g->NextGadget();
		}

		gl=gl->NextGadgetList();
		}

		}
		*/
		break;
	}
}

void EventEditor_Selection::MoveSelectedEventsToTick(OSTART tick,bool copy)
{
	if(patternselection.FirstSelectedEvent())
	{
		MoveO mo;

		OSTART diff=tick-patternselection.FirstSelectedEvent()->seqevent->GetEventStart();

		mo.song=WindowSong();
		mo.sellist=&patternselection;
		mo.diff=diff;
		mo.index=0;
		mo.flag=0;
		mo.filter=SEL_ALLEVENTS;
		mo.filter2=0;

		if(copy==true)
			mainedit->CopySelectedEventsInPatternList(&mo);
		else
			mainedit->MoveSelectedEventsInPatternList(&mo);
	}
}

EditData *EventEditor::Editor_DataMessage(EditData *data)
{
	if(data)
	{
		if(CheckStandardDataMessage(data)==true)return 0;

		switch(data->id)
		{
		case EDIT_TIME:
			{
				if(data->newvalue>=0 && data->newvalue<song->GetSongLength_Ticks())
					song->SetSongPosition(data->newvalue,true);
#ifdef _DEBUG
				else
				{
					if(data->newvalue<0)
						MessageBox(NULL,"Illegal Song Position","Error",MB_OK_STYLE);
				}
#endif
			}
			break;

		default:
			return SpecialEdit(data);
		}
	}

	return 0;
}

EditData *EventEditor_Selection::SpecialEdit(EditData *data)
{
	switch(data->id)
	{
	case EDIT_COPYMEASURE:
		MoveSelectedEventsToTick(WindowSong()->timetrack.ConvertMeasureToTicks(data->newvalue),true);
		break;

	case EDIT_MOVEMEASURE:
		MoveSelectedEventsToTick(WindowSong()->timetrack.ConvertMeasureToTicks(data->newvalue),false);
		break;

	default:
		return data;
	}

	return 0;
}

void EventEditor::ScrollSliderHoriz(OSTART diff)
{
	OSTART sp=startposition,add=diff*zoom->ticksperpixel;
	sp+=add;

	if(NewStartPosition(sp,true)==true)
	{
		SyncWithOtherEditors();
		UserEdit();
	}
}

guiGadget *EventEditor::Editor_Slider(guiGadget *gadget)
{
	switch(gadget->gadgetID)
	{
	case GADGETID_EDITOR_DATAZOOM:
		{
			double h=gadget->GetPos();

			h/=100;
			datazoom=h;
			NewDataZoom();
		}
		break;

	case GADGETID_EDITOR_SYNC:
		{
			syncsongposition=syncsongposition==true?false:true;
			syncgadget->Toggle(syncsongposition);

			if(syncsongposition==true)
			{
				SyncWithOtherEditors();
				UserEdit();
			}

			return 0;
		}
		break;

	case GADGETID_EDITOR_MOUSEQUANTIZE:
		{
			if(DeletePopUpMenu(true))
			{
				if(songmodepossible==true)
					AddMouseQuantizeMenu(popmenu);

				AddTimeDisplayMenu(popmenu);
				ShowPopMenu();
			}
		}
		break;

	case GADGETID_EDITORSLIDER_HORIZ:// windows Startposition <->
		{		
			if(songmode==true)
			{
				//TRACE ("Edit Beats %d\n",gadget->GetPos());
				OSTART newticks=WindowSong()->ConvertBeatsToTicks(gadget->GetPos());

				refreshslider=false; // slider changed posiotion

				if(NewStartPosition(newticks,true)==true)
				{
					SyncWithOtherEditors();
					UserEdit();
				}

				refreshslider=true;
				return 0;
			}
		}
		break;

	case GADGETID_EDITORSLIDER_HORIZZOOM: // win zoom <>
		{
			if(SetZoomGFX(gadget->GetPos(),true)==true)
				NewZoom();
			return 0;
		}
		break;

	case GADGETID_EDITORSLIDER_VERTZOOM:
		{
			zoomvert=true;
			ZoomGFX(gadget->GetPos(),false);
			zoomvert=false;

			return 0;
		}
		break;
	}

	return gadget;
}
