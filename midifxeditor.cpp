#ifdef OLDIE

#include "songmain.h"
#include "editor.h"
#include "gui.h"
#include "trackfx.h"
#include "MIDIhardware.h"
#include "editfunctions.h"
#include "arrangeditor_defines.h"
#include "groove.h"
#include "arrangeeditor.h"
#include "MIDIthruproc.h"
#include "quantizeeditor.h"
#include "editMIDIfilter.h"
#include "languagefiles.h"
#include "songmain.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "object_song.h"
#include "MIDIPattern.h"
#include "object_track.h"
#include "semapores.h"
#include "MIDIprocessor.h"
#include "editdata.h"
#include "MIDIinproc.h"

void Edit_ArrangeEditorEffects::InitPatternGadgets(int y)
{
	if(patternglist)
	{
		patternglist->RemoveGadgetList();
		patternglist=0;
	}

	if((this->status_pattern=arrangeeditor->activepattern) && frame)
	{
		showactivepattern=true;
		status_patternmediatype=arrangeeditor->activepattern->mediatype;

		// Active Pattern +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		int y2;
		int x=frame->x;
		int x2=frame->x2;
		int addy=maingui->GetFontSizeY_Sub();

		patternglist=arrangeeditor->gadgetlists.AddGadgetList(arrangeeditor);

		patternname=0;

		if(patternglist)
		{
			if((y2=y+addy)<=frame->y2)
			{
				patternonoff=patternglist->AddButton(x,y,x2,y2,FX_PATTERNONOFF_ID,0,Cxs[CXS_TPATTERNDISPLAYONOFF]);
				ShowPatternOnOff();

				y=y2+1;

				if((y2=y+addy)<=frame->y2)
				{
					patternname=patternglist->AddString(x,y,x2,y2,FX_PATTERNAME_ID,0,0,Cxs[CXS_NAMEACTPATTERN]);
					y=y2+1;
				}
			}

			if(arrangeeditor->patternonoff==true)
			{
				switch(arrangeeditor->activepattern->mediatype)
				{
				case MEDIATYPE_MIDI:
					if((y2=y+addy)<=frame->y2)
					{
						patternMIDIchannel=patternglist->AddButton(x,y,x2,y2,0,FX_PATTERNMIDICHANNEL_ID,0,"Pattern's MIDI Channel");

						y=y2+1;
					}
					else
						patternMIDIchannel=0;

					if((y2=y+addy)<=frame->y2)
					{
						patternbankselect=patternglist->AddButton(x,y,x2,y2,0,FX_PATTERNBANKSELECT_ID,0,"Patterns's MIDI Program");
						y=y2+1;
					}
					else
						patternbankselect=0;

					if((y2=y+addy)<=frame->y2)
					{
						patterntranspose=patternglist->AddButton(x,y,x2,y2,0,FX_PATTERNTRANSPOSE_ID,0,Cxs[CXS_PATTERNTRANSPOSE]);

						y=y2+1;
					}
					else
						patterntranspose=0;

					if((y2=y+addy)<=frame->y2)
					{
						patternvelocity=patternglist->AddButton(x,y,x2,y2,0,FX_PATTERNVELOCITY_ID,0,Cxs[CXS_PATTERNVELOCITY]);

						y=y2+1;
					}
					else
						patternvelocity=0;

					break;
				}// switch

				if((y2=y+addy)<=frame->y2)
				{
					patternquantize=patternglist->AddButton(x,y,x2,y2,0,FX_PATTERQUANTIZE_ID,0,Cxs[CXS_PATTERNQTSET]);
					y=y2+1;
				}
				else
					patternquantize=0;

				if((y2=y+addy)<=frame->y2)
				{
			//		patternmute=patternglist->AddCheckBox(x,y,x2,y2,FX_PATTERNMUTE_ID,"Mute","Mute Pattern");
					y=y2+1;
				}
				else
					patternmute=0;

				if((y2=y+addy)<=frame->y2)
				{
					//patternloopendless=patternglist->AddCheckBox(x,y,x2,y2,FX_PATTERNLOOPENDLESS_ID,Cxs[CXS_PATTERNLOOP],Cxs[CXS_PATTERNLOOP_I]);
					y=y2+1;
				}
				else
					patternloopendless=0;

				if((y2=y+addy)<=frame->y2)
				{
					int hx=x2-x;
					hx/=2;

					//patternloop=patternglist->AddCheckBox(x,y,x+hx,y2,FX_PATTERNLOOP_ID,"Loops:",Cxs[CXS_PATTERNLOOPONOFF]);				
					//patternloopnumber=patternglist->AddInteger(x+hx,y,x2,y2,FX_PATTERNLOOPNUMBER_ID,0,0,Cxs[CXS_PATTERNLOOPNR]);
					y=y2+1;

					// Loop Flag
					if((y2=y+addy)<=frame->y2)
					{
						patternloopflag=patternglist->AddCycle(x,y,x2,y2,FX_PATTERNLOOPFLAG_ID,0,Cxs[CXS_PATTERNLOOPTYPES]);
						y=y2+1;
					}
				}
				else
				{
					patternloopnumber=0;
					patternloop=patternloopflag=0;
				}
			}

			starttrackfx_y=y+4;

			ShowActivePattern();
		}
	}
	else
	{
		showactivepattern=false;
		starttrackfx_y=y;
	}

	//InitTrackMixer(starttrackfx_y);
}

void Edit_ArrangeEditorEffects::ShowTrackOnOff()
{
	if(trackonoff)
	{
		char *h=arrangeeditor->trackonoff==true?"Track ":"Track^ ";

		indextrack=arrangeeditor->WindowSong()->GetOfTrack(arrangeeditor->WindowSong()->GetFocusTrack());

		if(arrangeeditor->WindowSong()->GetFocusTrack())
		{
			char help[NUMBERSTRINGLEN];
			char *h2=0;

			if(arrangeeditor->WindowSong()->GetFocusTrack()->parent)
				h2=mainvar->GenerateString(h,mainvar->ConvertIntToChar(indextrack+1,help),":/Child");
			else
				h2=mainvar->GenerateString(h,mainvar->ConvertIntToChar(indextrack+1,help),":");

			if(h2)
			{
				trackonoff->ChangeButtonText(h2);
				delete h2;
			}
		}
		else
			trackonoff->ChangeButtonText(h);

	}
}

void Edit_ArrangeEditorEffects::ShowPatternOnOff()
{
	if(patternonoff)
		patternonoff->ChangeButtonText(arrangeeditor->patternonoff==true?"Pattern:":"Pattern^ :");
}

void Edit_ArrangeEditorEffects::InitTrackGadgets(int y)
{
	int y2,x=frame->x,x2=frame->x2,addy=maingui->GetFontSizeY_Sub();

	if(trackglist)
		trackglist->RemoveGadgetList();

	trackglist=arrangeeditor->gadgetlists.AddGadgetList(arrangeeditor);

	if(trackglist)
	{		
		if((y2=y+addy)<=frame->y2)
		{
			trackonoff=trackglist->AddButton(x,y,x2,y2,FX_TRACKONOFF_ID,0,Cxs[CXS_TRACKDISPLAYONOFF]);
			ShowTrackOnOff();

			y=y2+1;

			if((y2=y+addy)<=frame->y2)
			{
				trackname=trackglist->AddString(x,y,x2,y2,FX_TRACKNAME_ID,0,0,Cxs[CXS_TRACKDNAME]);
				y=y2+1;
			}
		}

		if(arrangeeditor->trackonoff==true)
		{
			// TrackImage
			if((y2=y+addy)<=frame->y2)
			{
				trackimage=trackglist->AddButton(x,y,x2,y2,0,FX_TRACKIMAGE,0,"Track Icon");
				y=y2+8;
			}

			if((y2=y+addy)<=frame->y2)
			{
				char *h=mainvar->GenerateString("Track ",Cxs[CXS_RECORD]);

				if(h)
				{
					trackstatus=trackglist->AddButton(x,y,x2,y2,0,FX_TRACKSTATUS_ID,0,h);
					delete h;
				}

				y=y2+1;
			}

			y+=3;

			// MIDI In Port
			if((y2=y+addy)<=frame->y2)
			{
				MIDIinput=trackglist->AddButton(x,y,x2,y2,0,FX_MIDIIN_ID,0,"Track MIDI Input Device");
				y=y2+1;
			}

			
			// Filter
			if((y2=y+addy)<=frame->y2)
			{
				MIDIinfilter=trackglist->AddButton(x,y,x2,y2,0,FX_INFILTER_ID,0,Cxs[CXS_TRACKINPUTEVENTFILTER]);
				y=y2+1;
			}

			// MIDI Out
			y+=3;

			// MIDI Out Port
			if((y2=y+addy)<=frame->y2)
			{
				MIDIoutput=trackglist->AddButton(x,y,x2,y2,0,FX_MIDIOUT_ID,0,"Track MIDI Output Device");
				y=y2+1;
			}

			// Out Filter
			if((y2=y+addy)<=frame->y2)
			{
				filter=trackglist->AddButton(x,y,x2,y2,0,FX_FILTER_ID,0,Cxs[CXS_TRACKEVENTFILTER]);
				y=y2+1;
			}

			y+=3;

			// MIDI Channesl
			if((y2=y+addy)<=frame->y2)
			{
				MIDIchannel=trackglist->AddButton(x,y,x2,y2,0,FX_MIDICHANNEL_ID,0,"Track MIDI Output Channel");
				y=y2+1;
			}

			// MIDI Thru
			if((y2=y+addy)<=frame->y2)
			{
				MIDIthru=trackglist->AddButton(x,y,x2,y2,"MIDI Thru",FX_MIDITHRU_ID,MODE_TOGGLE,"Track MIDI Thru");
				y=y2+1;
			}

			if((y2=y+addy)<=frame->y2)
			{
				MIDIoutprogram=trackglist->AddButton(x,y,x2,y2,0,FX_MIDIPROGRAM_ID,0,"Track MIDI Program");
				y=y2+1;
			}

			// Transpose
			if((y2=y+addy)<=frame->y2)
			{
				transpose=trackglist->AddButton(x,y,x2,y2,0,FX_TRANSPOSE_ID,0,Cxs[CXS_TRACKNOTETRANS]);
				y=y2+1;
			}

			int w=(x2-x)/2;

			// Delay
			if((y2=y+addy)<=frame->y2)
			{
				delay=trackglist->AddButton(x,y,x+w,y2,0,FX_DELAY_ID,0,Cxs[CXS_TRACKDELAY]);
				//y=y2+1;
			}

			// Velocity
			if((y2=y+addy)<=frame->y2)
			{
				velocity=trackglist->AddButton(x+w+1,y,x2,y2,0,FX_VELOCITY_ID,0,Cxs[CXS_TRACKVELOCITY]);

				y=y2+1;
			}

			// Quantize/Groove
			if((y2=y+addy)<=frame->y2)
			{
				quantize=trackglist->AddButton(x,y,x2,y2,0,FX_QUANTIZE_ID,0,Cxs[CXS_TRACKQTSET]);
				y=y2+1;
			}

			// Processor
			if((y2=y+addy)<=frame->y2)
			{
				processor=trackglist->AddButton(x,y,x2,y2,0,FX_PROCESSOR_ID,0,"Track Event Processor");
				y=y2+1;
			}

			// Group
			if((y2=y+addy)<=frame->y2)
			{
				group=trackglist->AddButton(x,y,x2,y2,"Group ---",FX_GROUP_ID,0,"Track Group/s");

				if(group)
				{
					y=y2+1;
					y2=y+addy;

					int hx=x;
					int w=(x2-x)/3;

					w-=1;

					group_mute=trackglist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKMUTEOFF,FX_GROUPMUTE_ID,0,"Mute Track Group");
					hx+=w+1;
					group_solo=trackglist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKSOLOOFF,FX_GROUPSOLO_ID,0,Cxs[CXS_PLAYTGSOLO]);

					hx+=w+1;
					group_rec=trackglist->AddImageButton(hx,y,hx+w,y2,IMAGE_TRACKRECORDOFF,FX_GROUPREC_ID,0,Cxs[CXS_ACTRACKGROUPREC]);
				}

				y=y2+1;
			}
		}

		ShowActiveTrack();
	}

	endtracky=y;

}// if track

void Edit_ArrangeEditorEffects::ShowTrackIcon()
{
	status_icon=status_track->icon;

	if(trackimage)
	{
		if(status_track->icon)
		{
			if(char *h=mainvar->GenerateString("Track Icon:",mainvar->stripExtension(status_track->icon->GetIconName())))
			{
				trackimage->ChangeButtonText(h);
				delete h;
			}
		}
		else
			trackimage->ChangeButtonText("Track Icon:-");
	}
}

void Edit_ArrangeEditorEffects::ShowActiveTrack()
{
	if((status_track=arrangeeditor->WindowSong()->GetFocusTrack()))
	{	
		if(trackname)
		{
			trackname->SetString(status_track->GetName());
			/*
			char *h=new char[strlen(status_track->name)+16];

			if(h)
			{
			if(showtrack==true)
			strcpy(h,"^Track:");
			else
			strcpy(h,">Track:");

			mainvar->AddString(h,status_track->name);

			trackname->ChangeButtonText(h);

			delete h;
			}*/
		}

		ShowTrackIcon();

		// MIDI Channel
		ShowMIDIChannel();

		ShowMIDITranspose();

		ShowMIDIVelocity();

		ShowMIDIDelay();

		// MIDI In Port
		ShowMIDIInput();

		// MIDI Out Port
		ShowMIDIOutput();
		ShowMIDIProgram();

		// Group
		ShowGroup();

		//Audio Out Channel
		//	ShowAudioInput();
		//	ShowAudioOutput();

		if(quantize)// Quantize/Groove
			quantize->ChangeButtonText(status_track->GetFX()->quantizeeffect.GetQuantizeInfo());

		status_track->GetFX()->quantizeeffect.Clone(&status_trackquantize);

		if(filter)
			filter->Enable();

		ShowProcessor();
		ShowMIDIInputFilter();
		ShowMIDIOutputFilter();
		ShowTrackType();
		//	ShowMIDIType();

		/*
		if(audiofx)
		{
		audiofx->Enable();
		}
		*/

		if(group)
		{
			group->Enable();
		}

		if(MIDIthru)
		{
		//	MIDIthru->SetToggle(status_MIDIthru=status_track->t_trackeffects.MIDIthru);
		}
	}
	else
	{
		/*
		if(audiofx)
		{
		audiofx->Disable();
		}
		*/

		if(trackimage)
		{
			trackimage->ChangeButtonText("-");
			trackimage->Disable();
		}

		if(trackstatus)
		{
			trackstatus->ChangeButtonText("-");
			trackstatus->Disable();
		}

		if(trackname)
		{
			trackname->SetString("-");
			trackname->Disable();
		}

		if(MIDIchannel)
		{
			MIDIchannel->ChangeButtonText("-");
			MIDIchannel->Disable();
		}

		if(MIDIinput)
		{
			MIDIinput->ChangeButtonText("-");
			MIDIinput->Disable();
		}

		if(MIDIoutput)
		{
			MIDIoutput->ChangeButtonText("-");
			MIDIoutput->Disable();
		}

		if(MIDIoutprogram)
		{
			MIDIoutprogram->ChangeButtonText("-");
			MIDIoutprogram->Disable();
		}

		if(quantize)
		{
			quantize->ChangeButtonText("-");
			quantize->Disable();
		}

		if(delay)
		{
			delay->ChangeButtonText("-");
			delay->Disable();
		}

		if(transpose)
		{
			transpose->ChangeButtonText("-");
			transpose->Disable();
		}

		if(velocity)
		{
			velocity->ChangeButtonText("-");
			velocity->Disable();
		}

		if(filter)
		{
			filter->ChangeButtonText("-");
			filter->Disable();
		}

		if(group)
		{
			group->Disable();
		}

		if(group_mute)
		{
			group_mute->Disable();
		}

		if(group_solo)
		{
			group_solo->Disable();
		}

		if(group_rec)
		{
			group_rec->Disable();
		}

		if(processor)
		{
			processor->ChangeButtonText("-");
			processor->Disable();
		}
	}
}

void Edit_ArrangeEditorEffects::ShowPatternProgram(MIDIPattern *mpattern)
{
	if(mpattern)
	{
		mpattern->MIDIprogram.Clone(&status_MIDIPatternprogram);

		if(patternbankselect)
		{
			ShowProgram(patternbankselect,&status_MIDIPatternprogram);
		}
	}
}

void Edit_ArrangeEditorEffects::ShowActivePattern()
{
	if(arrangeeditor->activepattern && patternglist)
	{
		if(patternnamestring)delete patternnamestring;
		patternnamestring=0;

		if(patternname)
		{
			if(arrangeeditor->activepattern->mainclonepattern)
			{
				patternnamestring=mainvar->GenerateString(arrangeeditor->activepattern->mainclonepattern->GetName());

				if(dontshowpatternname==false)
					patternname->SetString(patternnamestring);
				patternname->Disable();
			}
			else
			{
				patternnamestring=mainvar->GenerateString(arrangeeditor->activepattern->GetName());

				if(dontshowpatternname==false)
					patternname->SetString(patternnamestring);
			}
		}

		if(patternloopendless)patternloopendless->SetCheckBox(arrangeeditor->activepattern->loopendless);

		status_patternloopendless=arrangeeditor->activepattern->loopendless;

		if(patternloop)patternloop->SetCheckBox(arrangeeditor->activepattern->loopwithloops);

		status_patternloopwithloops=arrangeeditor->activepattern->loopwithloops;

		if(patternloopnumber)patternloopnumber->SetInteger(arrangeeditor->activepattern->loops); // 0=endless

		status_patternloop=arrangeeditor->activepattern->loops;

		if(patternmute)patternmute->SetCheckBox(arrangeeditor->activepattern->mute);
		status_patternmute=arrangeeditor->activepattern->mute;

		if(patternquantize)// Quantize/Groove
		{
			patternquantize->ChangeButtonText(arrangeeditor->activepattern->quantizeeffect.GetQuantizeInfo());
		}

		arrangeeditor->activepattern->quantizeeffect.Clone(&status_patternquantize);

		status_loopflag=arrangeeditor->activepattern->loopflag;

		if(patternloopflag)
		{
			patternloopflag->ClearCycle();

			patternloopflag->AddStringToCycle(Cxs[CXS_LOOPMEASURE]);
			patternloopflag->AddStringToCycle(Cxs[CXS_LOOPBEAT]);

			switch(arrangeeditor->activepattern->mediatype)
			{
			case MEDIATYPE_MIDI:
				patternloopflag->AddStringToCycle(Cxs[CXS_LOOPDIRECT]);
				break;

			case MEDIATYPE_AUDIO:
				patternloopflag->AddStringToCycle("Loop:Sample");
				break;
			}

			patternloopflag->SetCycleSelection(arrangeeditor->activepattern->loopflag);
		}

		switch(arrangeeditor->activepattern->mediatype)
		{
		case MEDIATYPE_MIDI:
			{
				MIDIPattern *MIDIPattern=(MIDIPattern *)arrangeeditor->activepattern;

				// MIDI Channesl
				if(patternMIDIchannel)
				{
					if(char *MIDI=mainvar->GenerateString(Cxs[CXS_MIDICHANNEL],":",MIDIchannels[MIDIPattern->GetMIDIFX()->channel]))
					{
						patternMIDIchannel->ChangeButtonText(MIDI);
						delete MIDI;
					}
				}

				ShowPatternProgram(MIDIPattern);

				if(patterntranspose)
				{
					char h[200];
					char th[NUMBERSTRINGLEN];

					mainvar->MixString(h,"Transpose:",mainvar->ConvertIntToChar(MIDIPattern->GetMIDIFX()->transpose,th));
					patterntranspose->ChangeButtonText(h);
				}

				if(patternvelocity)
				{
					char h[200];
					char th[NUMBERSTRINGLEN];

					mainvar->MixString(h,"Velocity:",mainvar->ConvertIntToChar(MIDIPattern->GetMIDIFX()->velocity,th));
					patternvelocity->ChangeButtonText(h);
				}

				MIDIPattern->GetMIDIFX()->Clone(&status_MIDIPatternfx);

			}
			break; // Mediatype MIDI
		}
	}
}

void Edit_ArrangeEditorEffects::ShowMIDIVelocity()
{
	if(status_track)
	{
		status_velocity=status_track->GetFX()->MIDI_velocity;

		if(velocity)
		{
			char h[200],th[NUMBERSTRINGLEN];
			mainvar->MixString(h,"Veloc.:",mainvar->ConvertIntToChar(status_velocity,th));
			velocity->ChangeButtonText(h);
		}
	}
}

void Edit_ArrangeEditorEffects::ShowProcessor()
{
	if(status_processorname)delete status_processorname;
	status_processorname=0;

	if(status_track)
	{
		if(processor)
		{
			if(status_processor=status_track->GetProcessor())
			{
				status_processorbypass=status_track->GetProcessor()->bypass;
				status_processorname=mainvar->GenerateString(status_track->GetProcessor()->ProcessorName());

				char *h=new char[32+strlen(status_track->GetProcessor()->ProcessorName())];

				if(h)
				{
					strcpy(h,status_track->GetProcessor()->bypass==true?"Proc<ByPass>:":"Proc:");

					mainvar->AddString(h,status_track->GetProcessor()->ProcessorName());
					processor->ChangeButtonText(h);
					delete h;
				}
			}
			else
			{
				processor->ChangeButtonText(Cxs[CXS_NOPROCESSOR]);
			}
		}
	}
}

void Edit_ArrangeEditorEffects::ShowMIDIInputFilter()
{
	if(MIDIinfilter)
	{
		if(status_track)
		{	
			status_track->GetFX()->inputfilter.Clone(&status_inputfilter);

			char *MIDIfilter=0;

			if(status_track->GetFX()->inputfilter.CheckFilterActive()==true)
				MIDIfilter=mainvar->GenerateString("MIDI In Filter",":",Cxs[CXS_ON]);
			else
				MIDIfilter=mainvar->GenerateString("MIDI In Filter");

			if(MIDIinfilter)
			{
				MIDIinfilter->ChangeButtonText(MIDIfilter);
				delete MIDIfilter;
			}
		}
	}
}

void Edit_ArrangeEditorEffects::ShowMIDIOutputFilter()
{
	if(filter)
	{
		if(status_track)
		{	
			status_track->GetFX()->filter.Clone(&status_outputfilter);

			char *MIDIfilter=0;

			if(status_track->GetFX()->filter.CheckFilterActive()==true)
				MIDIfilter=mainvar->GenerateString("MIDI Out Filter",":",Cxs[CXS_ON]);
			else
				MIDIfilter=mainvar->GenerateString("MIDI Out Filter");

			if(MIDIfilter)
			{
				filter->ChangeButtonText(MIDIfilter);
				delete MIDIfilter;
			}
		}
	}
}

void Edit_ArrangeEditorEffects::ShowMIDIDelay()
{
	if(status_track)
	{
		status_delay=status_track->GetFX()->delay;
		if(delay)
		{
			char h[200],th[NUMBERSTRINGLEN];

			mainvar->MixString(h,"Delay:",mainvar->ConvertIntToChar(status_delay,th));
			delay->ChangeButtonText(h);
		}
	}
}

void Edit_ArrangeEditorEffects::ShowMIDITranspose()
{
	if(status_track)
	{
		status_transpose=status_track->GetFX()->MIDI_transpose;

		if(transpose)
		{
			char h[200],th[NUMBERSTRINGLEN];			
			mainvar->MixString(h,"Transpose:",mainvar->ConvertIntToChar(status_transpose,th));
			transpose->ChangeButtonText(h);
		}
	}
}

void Edit_ArrangeEditorEffects::ShowProgram(guiGadget *button,MIDIOutputProgram *program)
{
	if(program->on==true)
	{
		char h[128];

		strcpy(h,program->device);
		mainvar->AddString(h,":");
		mainvar->AddString(h,program->info);
		mainvar->AddString(h," (");

		if(program->usebank==true)
		{
			char h2[NUMBERSTRINGLEN];
			mainvar->AddString(h," BK:");
			mainvar->AddString(h,mainvar->ConvertIntToChar(program->MIDIBank+1,h2));
		}

		{
			char h2[NUMBERSTRINGLEN];
			mainvar->AddString(h," PG:");
			mainvar->AddString(h,mainvar->ConvertIntToChar(program->MIDIProgram+1,h2));
		}

		mainvar->AddString(h,")");
		button->ChangeButtonText(h);
	}
	else
		button->ChangeButtonText(Cxs[CXS_NOPROGRAM]);
}

void Edit_ArrangeEditorEffects::ShowMIDIProgram()
{
	if(status_track)
	{
		status_track->GetFX()->MIDIprogram.Clone(&status_MIDIprogram);

		if(MIDIoutprogram)
			ShowProgram(MIDIoutprogram,&status_MIDIprogram);
	}
}

void Edit_ArrangeEditorEffects::ShowMIDIOutput()
{
	if(status_track)
	{
		Seq_Group_MIDIOutPointer *sgp=status_track->GetMIDIOut()->FirstDevice();

		if(sgp)
			status_MIDIoutdevice=sgp->device;
		else
			status_MIDIoutdevice=0;

		if(MIDIoutput)
		{
			char *c;

			if(!status_MIDIoutdevice)
				c=Cxs[CXS_NOMIDIOUT];
			else
				c=status_MIDIoutdevice->name;

			if(char *MIDIout=mainvar->GenerateString("MIDI Out:",c))
			{
				MIDIoutput->ChangeButtonText(MIDIout);
				delete MIDIout;
			}
		}
	}
}

void Edit_ArrangeEditorEffects::ShowMIDIInput()
{
	if(status_track)
	{
		Seq_Group_MIDIInPointer *sgp=status_track->GetMIDIIn()->FirstDevice();

		if(sgp)
			status_MIDIindevice=sgp->device;
		else
			status_MIDIindevice=0;

		status_noMIDIinput=status_track->GetFX()->noMIDIinput;
		status_useallinputdevices=status_track->GetFX()->useallinputdevices;

		if(MIDIinput)
		{
			char *c;

			if(status_track->GetFX()->noMIDIinput==true)
				c=Cxs[CXS_NOMIDIIN];
			else
			{
				if(status_track->GetFX()->useallinputdevices==true)
					c=Cxs[CXS_USEALLMIDIINDEVICES];
				else{
					if(status_MIDIindevice)
						c=status_MIDIindevice->name;
					else
						c=Cxs[CXS_NOMIDIINDEV];
				}
			}

			char *MIDIin=new char[strlen(c)+128];

			if(MIDIin)
			{
				mainvar->MixString(MIDIin,"MIDI In:",c);

				/*
				if(status_track->GetFX()->inputfilter.CheckFilterActive()==true)
					mainvar->AddString(MIDIin,"(Filter)");
*/
				MIDIinput->ChangeButtonText(MIDIin);

//				status_track->GetFX()->inputfilter.Clone(&status_inputfilter);

				delete MIDIin;
			}
		}
	}
}

void Edit_ArrangeEditorEffects::ShowTrackType()
{
	if(status_track)
	{
		if(trackstatus)
		{
			status_tracktype=status_track->recordtracktype;

			switch(status_tracktype)
			{
				/*case TRACKTYPE_ALL:
				trackstatus->ChangeButtonText(Cxs[CXS_RECALL]);
				break;
				*/

			case TRACKTYPE_MIDI:
				trackstatus->ChangeButtonText(Cxs[CXS_RECONLYMIDI]);
				break;

			case TRACKTYPE_AUDIO:
				trackstatus->ChangeButtonText(Cxs[CXS_RECONLYAUDIO]);
				break;
			}
		}
	}
}

void Edit_ArrangeEditorEffects::ShowMIDIChannel()
{
	if(status_track)
	{
		status_channel=status_track->GetFX()->MIDI_channel;

		if(MIDIchannel)
		{
			if(char *MIDI=mainvar->GenerateString(Cxs[CXS_MIDICHANNEL],":",MIDIchannels[status_channel]))
			{
				MIDIchannel->ChangeButtonText(MIDI);
				delete MIDI;
			}
		}
	}
}

void Edit_ArrangeEditorEffects::ShowGroup()
{
	status_group=0;

	status_group_rec=status_group_mute=status_group_solo=false;

	if(status_track)
	{
		if(group)
		{
			Seq_Group_GroupPointer *sgp=status_track->GetGroups()->FirstGroup();

			if(status_group=status_track->GetGroups()->activegroup)
			{
				// 1 Group
				if(char *gname=new char[strlen(status_group->name)+64])
				{
					mainvar->MixString(gname,"Group:",status_group->name);

					if(status_track->GetGroups()->GetCountGroups()>1)
					{
						mainvar->AddString(gname," (");

						char h2[16];
						mainvar->AddString(gname,mainvar->ConvertIntToChar(status_track->GetGroups()->GetCountGroups(),h2));
						mainvar->AddString(gname,")");
					}

					group->ChangeButtonText(gname);

					delete gname;
				}

				status_group_mute=status_group->mute;

				if(group_mute)
				{
					group_mute->ChangeButtonImage(status_group_mute==true?IMAGE_TRACKMUTEON:IMAGE_TRACKMUTEOFF);
					group_mute->Enable();
				}

				status_group_solo=status_group->solo;

				if(group_solo)
				{
					group_solo->ChangeButtonImage(status_group_solo==true?IMAGE_TRACKSOLOON:IMAGE_TRACKSOLOOFF);
					group_solo->Enable();
				}

				status_group_rec=status_group->rec;
				if(group_rec)
				{
					group_rec->ChangeButtonImage(status_group_rec==true?IMAGE_TRACKRECORDON:IMAGE_TRACKRECORDOFF);
					group_rec->Enable();
				}
			}
			else
			{
				if(group)
					group->ChangeButtonText(Cxs[CXS_NOGROUP]);

				if(group_mute)
				{
					group_mute->ChangeButtonImage(IMAGE_TRACKMUTEOFF);
					group_mute->Disable();
				}

				if(group_solo)
				{
					group_solo->ChangeButtonImage(IMAGE_TRACKSOLOOFF);
					group_solo->Disable();
				}

				if(group_rec)
				{
					group_rec->ChangeButtonImage(IMAGE_TRACKRECORDOFF);
					group_rec->Disable();
				}
			}

		}
	}

}

void Edit_ArrangeEditorEffects::RefreshRealtime()
{	
	return;

	if(status_track!=arrangeeditor->WindowSong()->GetFocusTrack())
	{
		status_track=arrangeeditor->WindowSong()->GetFocusTrack();
		ShowActiveTrack();	
	}
	else
		if(status_track)
		{
			if(status_group!=status_track->GetGroups()->activegroup ||
				(
				status_track->GetGroups()->activegroup &&
				(
				status_group_rec!=status_track->GetGroups()->activegroup->rec ||
				status_group_solo!=status_track->GetGroups()->activegroup->solo ||
				status_group_mute!=status_track->GetGroups()->activegroup->mute
				)
				)
				)
				ShowGroup();

			/*
			if(status_MIDItype!=status_track->MIDItype)
			ShowMIDIType();
			*/

			if(status_trackquantize.Compare(&status_track->GetFX()->quantizeeffect)==true)
				ShowActiveTrack();

			if(status_icon!=status_track->icon)
				ShowTrackIcon();

			if(status_delay!=status_track->GetFX()->delay)
				ShowMIDIDelay();

			// Check set flag
			if(status_track->GetFX()->MIDIprogram.set==true)
			{
				if(status_track->GetFX()->MIDIprogram.MIDIChannel>=1 && status_track->GetFX()->MIDIprogram.MIDIChannel<=16)
					status_track->GetFX()->SetChannel(status_track->GetFX()->MIDIprogram.MIDIChannel);

				status_track->GetFX()->MIDIprogram.set=false; // Reset Flag
			}

			if(status_channel!=status_track->GetFX()->MIDI_channel)
				ShowMIDIChannel();

			if(status_transpose!=status_track->GetFX()->MIDI_transpose)
				ShowMIDITranspose();

			if(status_velocity!=status_track->GetFX()->MIDI_velocity)
				ShowMIDIVelocity();

			if((status_MIDIoutdevice && (!status_track->GetMIDIOut()->FirstDevice())) ||
				(status_track->GetMIDIOut()->FirstDevice() && status_MIDIoutdevice!=status_track->GetMIDIOut()->FirstDevice()->device)
				)
				ShowMIDIOutput();

			if(status_MIDIindevice!=status_track->indevice || 
				status_useallinputdevices!=status_track->GetFX()->useallinputdevices ||
				status_noMIDIinput!=status_track->GetFX()->noMIDIinput ||
				status_inputfilter.Compare(&status_track->GetFX()->inputfilter)==false
				)	
				ShowMIDIInput();

			if(status_track->GetFX()->MIDIprogram.Compare(&status_MIDIprogram)==false)
				ShowMIDIProgram();

			if(status_processor!=status_track->GetProcessor() || 
				(status_track->GetProcessor() && status_processorbypass!=status_track->GetProcessor()->bypass) || 
				(status_processorname && status_track->GetProcessor() && strcmp(status_processorname,status_track->GetProcessor()->ProcessorName())!=0))
				ShowProcessor();

			if(status_inputfilter.Compare(&status_track->GetFX()->inputfilter)==false)
				ShowMIDIInputFilter();

			if(status_outputfilter.Compare(&status_track->GetFX()->filter)==false)
				ShowMIDIOutputFilter();

			if(status_tracktype!=status_track->recordtracktype)
				ShowTrackType();

			if(MIDIthru && status_MIDIthru!=status_track->t_trackeffects.MIDIthru)
			{
				//MIDIthru->SetToggle(status_MIDIthru=status_track->t_trackeffects.MIDIthru);
			}

		}

		if((showactivepattern==true && (!arrangeeditor->activepattern)) ||
			(showactivepattern==false && arrangeeditor->activepattern) ||
			(showactivepattern==true && arrangeeditor->activepattern && arrangeeditor->activepattern->mediatype!=status_patternmediatype)
			)
		{
			InitPatternGadgets(endtracky);
			InitTrackMixer(starttrackfx_y);
		}
		else
		{
			// Refresh Pattern Data
			if(status_pattern!=arrangeeditor->activepattern)
			{
				InitPatternGadgets(endtracky);
				//	if(patternglist)patternglist->RefreshList();
			}
			else
			{
				if(status_pattern)
				{
					bool refresh=false;

					if(patternname && patternnamestring)
					{
						char *c=0;

						if(arrangeeditor->activepattern->mainclonepattern)
							c=arrangeeditor->activepattern->mainclonepattern->GetName();
						else
							c=arrangeeditor->activepattern->GetName();

						if(c && strcmp(c,patternnamestring)!=0)
							refresh=true;
					}

					if(status_patternmute!=status_pattern->mute)refresh=true;
					if(status_patternloopendless!=status_pattern->loopendless)refresh=true;
					if(status_patternloopwithloops!=status_pattern->loopwithloops)refresh=true;
					if(status_patternloop!=status_pattern->loops)refresh=true;	
					if(status_loopflag!=status_pattern->loopflag)refresh=true;
					if(status_patternquantize.Compare(&arrangeeditor->activepattern->quantizeeffect)==true)refresh=true;	

					if(status_pattern->mediatype==MEDIATYPE_MIDI)
					{
						MIDIPattern *mp=(MIDIPattern *)status_pattern;

						if(mp->GetMIDIFX()->Compare(&status_MIDIPatternfx)==true)refresh=true;

						// Check set flag
						if(mp->MIDIprogram.set==true)
						{
							if(mp->MIDIprogram.MIDIChannel>=1 && mp->MIDIprogram.MIDIChannel<=16)
								mp->GetMIDIFX()->channel=mp->MIDIprogram.MIDIChannel;

							mp->MIDIprogram.set=false; // Reset Flag
						}

						if(refresh==false)
						{
							if(mp->MIDIprogram.Compare(&status_MIDIPatternprogram)==false)
								ShowPatternProgram(mp);
						}
					}

					if(refresh==true)ShowActivePattern();
				}
			}
		}

		if(trackonoff)
		{
			int aindex=arrangeeditor->WindowSong()->GetOfTrack(arrangeeditor->WindowSong()->GetFocusTrack());

			if(indextrack!=aindex)
			{
				ShowTrackOnOff();
			}
		}
}

EditData *Edit_ArrangeEditorEffects::EditDataMessage(EditData *data)
{
	if(Seq_Track *track=arrangeeditor->WindowSong()->GetFocusTrack())
	{
		switch(data->id)
		{
		case TRACKFX_DELAY:
			track->GetFX()->delay=data->newvalue;
			break;

		case TRACKFX_VELOCITY:
			track->GetFX()->SetVelocity(data->newvalue);
			//ShowActiveTrack();
			break;

		case TRACKFX_TRANSPOSE:
			{
				track->LockOpenOutputNotes();
				track->SendAllOpenOutputNotes();
				track->GetFX()->SetTranspose(data->newvalue);
				track->UnlockOpenOutputNotes();
			}
			break;

		case TRACKFX_TRANSPOSE_PATTERN:
			{
				MIDIPattern *mp=(MIDIPattern *)arrangeeditor->activepattern;

				if(mp)
				{
					mp->GetMIDIFX()->transpose=data->newvalue;
					//	ShowActivePattern();
				}
			}
			break;

		case TRACKFX_VELOCITY_PATTERN:
			{
				MIDIPattern *mp=(MIDIPattern *)arrangeeditor->activepattern;

				if(mp)
				{
					mp->GetMIDIFX()->velocity=data->newvalue;
					// ShowActivePattern();
				}
			}
			break;

			/*
			case TRACKFX_PATTERN_BANKSELECT:
			{
			MIDIPattern *mp=(MIDIPattern *)arrangeeditor->activepattern;

			if(mp)
			{
			if(data->onoffstatus==false)
			mp->MIDIeffects.bankselect=-1;
			else
			mp->MIDIeffects.bankselect=data->newvalue;

			ShowActivePattern();
			}
			}
			break;

			case TRACKFX_PATTERN_PROGRAMSELECT:
			{
			MIDIPattern *mp=(MIDIPattern *)arrangeeditor->activepattern;

			if(mp)
			{
			if(data->onoffstatus==false)
			mp->MIDIeffects.programselect=-1;
			else
			mp->MIDIeffects.programselect=data->newvalue;

			ShowActivePattern();
			}
			}
			break;
			*/

		default:
			return data;
			break;
		}

		return 0;
	}

	return data;
}

guiGadget *Edit_ArrangeEditorEffects::Gadget(guiGadget *g) // called by editor
{
	Seq_Track *track=arrangeeditor->WindowSong()->GetFocusTrack();

	switch(g->gadgetID)
	{
	case FX_MIDITYPE_ID:
		{
			if(track)
			{
				arrangeeditor->DeletePopUpMenu(true);

				if(arrangeeditor->popmenu)
				{
					class menu_MIDItype:public guiMenu
					{
					public:
						menu_MIDItype(Seq_Track *tr,int t)
						{
							track=tr;
							type=t;
						}

						void MenuFunction()
						{
							track->SetMIDIType(type,true);
							//track->MIDItype=type;
						} //

						Seq_Track *track;
						int type;
					};

					arrangeeditor->popmenu->AddFMenu("Track->MIDI",new menu_MIDItype(track,Seq_Track::OUTPUTTYPE_MIDI),track->MIDItype==Seq_Track::OUTPUTTYPE_MIDI?true:false);
					arrangeeditor->popmenu->AddFMenu("Track->Audio Instruments",new menu_MIDItype(track,Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT),track->MIDItype==Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT?true:false);
					arrangeeditor->popmenu->AddFMenu("Track->Audio Instruments+MIDI",new menu_MIDItype(track,Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENTANDMIDI),track->MIDItype==Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENTANDMIDI?true:false);

					arrangeeditor->ShowPopMenu();
				}
			}
		}
		break;

	case FX_AUDIOIN_ID:
		if(!(arrangeeditor->WindowSong()->status&Seq_Song::STATUS_RECORD))
		{
			if(track)
				maingui->EditTrackInput(arrangeeditor,track);
		}
		break;

	case FX_MIDIPROGRAM_ID:
		if(track)
		{
			mainsettings->synthdevices.BuildPopUp(&track->GetFX()->MIDIprogram,arrangeeditor,g->x2,g->y,"Edit Track Device Program");
		}
		break;

	case FX_PATTERNONOFF_ID:
		{
			arrangeeditor->patternonoff=arrangeeditor->patternonoff==true?false:true;

			Init();

			//if(trackglist)trackglist->RefreshList();
			//if(patternglist)patternglist->RefreshList();
		}
		break;

	case FX_TRACKONOFF_ID:
		{
			arrangeeditor->trackonoff=arrangeeditor->trackonoff==true?false:true;
			Init();
			//if(trackglist)trackglist->RefreshList();
			//if(patternglist)patternglist->RefreshList();
		}
		break;

	case FX_TRACKIMAGE:
		{
			if(track)
			{
				arrangeeditor->DeletePopUpMenu(true);

				if(arrangeeditor->popmenu)
				{
					class menu_trackimage:public guiMenu
					{
					public:
						menu_trackimage(Edit_Arrange *ed,Seq_Track *t,TrackIcon *i)
						{
							editor=ed;
							track=t;
							icon=i;
						}

						void MenuFunction()
						{
							editor->ChangeTrackIcon(track,icon);
						} //

						Edit_Arrange *editor;
						Seq_Track *track;
						TrackIcon *icon;
					};

					if(track->icon){
						arrangeeditor->popmenu->AddFMenu(Cxs[CXS_NOICON],new menu_trackimage(arrangeeditor,track,0));
						arrangeeditor->popmenu->AddLine();
					}

					TrackIcon *icon=maingui->gfx.FirstTrackIcon();

					while(icon)
					{
						arrangeeditor->popmenu->AddFMenu(mainvar->stripExtension(icon->filename),new menu_trackimage(arrangeeditor,track,icon),icon==track->icon?true:false);
						icon=icon->NextIcon();
					}

					arrangeeditor->ShowPopMenu();
				}
			}
		}
		break;

	case FX_PROCESSOR_ID:
		{
			if(track)
			{
				arrangeeditor->DeletePopUpMenu(true);

				if(arrangeeditor->popmenu)
				{
					class menu_trackprocessor:public guiMenu
					{
					public:
						menu_trackprocessor(Processor *p)
						{
							processor=p;
						}

						void MenuFunction()
						{
							if(!processor)
							{
								guiWindow *win=maingui->FindWindow(EDITORTYPE_PROCESSOR,0,0);

								if(!win)
									maingui->OpenEditorStart(EDITORTYPE_PROCESSOR,0,0,0,0,0,0);
								else
									win->WindowToFront(true);
							}
						} //

						Processor *processor;
					};

					arrangeeditor->popmenu->AddFMenu("Processor Editor",new menu_trackprocessor(0));

					if(track->GetProcessor())
					{
						arrangeeditor->popmenu->AddLine();

						class menu_editmod:public guiMenu
						{
						public:
							menu_editmod(Seq_Track *t,MIDIPlugin *pm,int px,int py)
							{
								track=t;
								module=pm;
								x=px;
								y=py;
							}

							void MenuFunction()
							{
								guiWindowSetting settings;

								settings.startposition_x=x;
								settings.startposition_y=y;
								module->OpenGUI(&settings);
							} //

							Seq_Track *track;
							MIDIPlugin *module;
							int x,y;
						};

						{
							if(char *n=mainvar->GenerateString(Cxs[CXS_EDIT],":(",track->GetProcessor()->name,")"))
							{
								guiMenu *ed=arrangeeditor->popmenu->AddMenu(n,0);
								delete n;

								if(ed)
								{
									MIDIPlugin *pm=track->GetProcessor()->FirstProcessorModule();

									while(pm)
									{
										if(pm->bypass==true)
											n=mainvar->GenerateString(pm->staticname,"(ByPass)");
										else
											n=mainvar->GenerateString(pm->staticname);

										if(n){
											ed->AddFMenu(n,new menu_editmod(track,pm,arrangeeditor->GetWinPosX()+g->x2,arrangeeditor->GetWinPosX()+g->y));
											delete n;
										}
										pm=pm->NextModule();
									}
								}
							}
						}

						class menu_bypassprocessor:public guiMenu
						{
						public:
							menu_bypassprocessor(Seq_Track *t,bool b)
							{
								track=t;
								bypass=b;
							}

							void MenuFunction()
							{
								track->SetProcessorBypass(bypass==true?false:true);
							} //

							Seq_Track *track;
							bool bypass;
						};

						arrangeeditor->popmenu->AddFMenu("Processor Bypass",new menu_bypassprocessor(track,track->GetProcessor()->bypass),track->GetProcessor()->bypass);

						class menu_noprocessor:public guiMenu
						{
						public:
							menu_noprocessor(Seq_Track *t){track=t;}

							void MenuFunction()
							{
								track->SetProcessor(0);
							} //

							Seq_Track *track;
						};

						arrangeeditor->popmenu->AddFMenu(Cxs[CXS_USENOPROCESSOR],new menu_noprocessor(track));
					}

					if(mainprocessor->FirstProcessor())
					{
						class menu_selprocessor:public guiMenu
						{
						public:
							menu_selprocessor(Seq_Track *t,Processor *p)
							{
								track=t;
								processor=p;
							}

							void MenuFunction()
							{
								track->SetProcessor(processor);
							} //

							Seq_Track *track;
							Processor *processor;
						};

						arrangeeditor->popmenu->AddLine();

						Processor *p=mainprocessor->FirstProcessor();

						while(p){
							bool sel=track->GetProcessor()==p?true:false;
							arrangeeditor->popmenu->AddFMenu(p->ProcessorName(),new menu_selprocessor(track,p),sel);

							p=p->NextProcessor();
						}
					}

					arrangeeditor->ShowPopMenu();
				}
			}
		}
		break;

		// Pattern -------------
	case FX_PATTERNTRANSPOSE_ID:
		if(arrangeeditor->activepattern)
		{
			MIDIPattern *mp=(MIDIPattern *)arrangeeditor->activepattern;
			if(EditData *edit=new EditData)
			{
				edit->win=arrangeeditor;
				edit->x=g->x2;
				edit->y=g->y;

				edit->id=TRACKFX_TRANSPOSE_PATTERN;
				edit->name=Cxs[CXS_PATTERNTRANSPOSE_2];

				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->from=-127;
				edit->to=127;
				edit->value=mp->GetMIDIFX()->transpose;

				maingui->EditDataValue(edit);
			}
		}
		break;

	case FX_PATTERNVELOCITY_ID:
		if(arrangeeditor->activepattern)
		{
			MIDIPattern *mp=(MIDIPattern *)arrangeeditor->activepattern;

			if(EditData *edit=new EditData)
			{
				edit->win=arrangeeditor;
				edit->x=g->x2;
				edit->y=g->y;

				edit->id=TRACKFX_VELOCITY_PATTERN;
				edit->name=Cxs[CXS_PATTERNVELOCITY_2];

				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->from=-127;
				edit->to=127;
				edit->value=mp->GetMIDIFX()->velocity;

				maingui->EditDataValue(edit);
			}
		}
		break;

	case FX_PATTERNMUTE_ID:
		if(arrangeeditor->activepattern)
		{
			bool mute=g->index?true:false;
			status_patternmute=mute;
			mainedit->MutePattern(arrangeeditor->WindowSong(),0,arrangeeditor->activepattern,mute);
		}
		break;

	case FX_PATTERNLOOPENDLESS_ID:
		if(arrangeeditor->activepattern)
		{
			bool loop=g->index?true:false;

			if(arrangeeditor->activepattern->track->record_MIDIPattern!=arrangeeditor->activepattern)
				mainedit->LoopPattern(arrangeeditor->activepattern,loop,false,arrangeeditor->activepattern->loops);
			else
				ShowActivePattern(); // dont loop record pattern
		}
		break;

	case FX_PATTERNLOOPFLAG_ID:
		if(arrangeeditor->activepattern)
		{
			if(g->index!=arrangeeditor->activepattern->loopflag)
			{
				arrangeeditor->activepattern->loopflag=g->index;

				if((arrangeeditor->activepattern->loops && arrangeeditor->activepattern->loopwithloops==true) || arrangeeditor->activepattern->loopendless==true)
				{
					if(arrangeeditor->WindowSong()==mainvar->GetActiveSong())
					{
						mainthreadcontrol->LockActiveSong();
						arrangeeditor->activepattern->LoopPattern();
						arrangeeditor->WindowSong()->CheckPlaybackRefresh();
						mainthreadcontrol->UnlockActiveSong();
					}
					else
						arrangeeditor->activepattern->LoopPattern();

					maingui->RefreshAllEditors(arrangeeditor->WindowSong(),EDITORTYPE_ARRANGE,0);
				}
			}
		}
		break;

	case FX_PATTERNLOOP_ID:
		if(arrangeeditor->activepattern)
		{
			if(arrangeeditor->activepattern->track->record_MIDIPattern!=arrangeeditor->activepattern)
				mainedit->LoopPattern(arrangeeditor->activepattern,false,g->index?true:false,arrangeeditor->activepattern->loops);
			else
				ShowActivePattern(); // dont loop record pattern
		}
		break;

	case FX_PATTERNLOOPNUMBER_ID:
		if(arrangeeditor->activepattern)
		{
			int nr=g->index;

			if(nr>0 && nr!=arrangeeditor->activepattern->loops)
			{
				if(arrangeeditor->activepattern->loopwithloops==true)
					mainedit->LoopPattern(arrangeeditor->activepattern,arrangeeditor->activepattern->loopendless,arrangeeditor->activepattern->loopwithloops,nr);
				else
					arrangeeditor->activepattern->loops=nr;
			}
		}
		break;

	case FX_PATTERNAME_ID:
		dontshowpatternname=true;
		if(arrangeeditor->activepattern)
		{
			arrangeeditor->activepattern->SetName(g->GetString(STANDARDSTRINGLEN));
			arrangeeditor->activepattern->ShowPatternName(arrangeeditor);
		}
		dontshowpatternname=false;
		break;

	case FX_PATTERQUANTIZE_ID:
		if(arrangeeditor->activepattern)
		{
			guiWindow *w=maingui->FirstWindow();

			while(w)
			{
				if(w->GetEditorID()==EDITORTYPE_QUANTIZEEDITOR && ((Edit_QuantizeEditor *)w)->effect==&arrangeeditor->activepattern->quantizeeffect)
					break;

				w=w->NextWindow();
			}

			if(!w)
			{
				// Pattern Quantize
				guiWindowSetting set;

				set.startposition_x=arrangeeditor->GetWinPosX()+g->x2;
				set.startposition_y=arrangeeditor->GetWinPosY()+g->y;
				set.startwidth=300;
				set.startheight=300;

				w=maingui->OpenEditorStart(EDITORTYPE_QUANTIZEEDITOR,arrangeeditor->WindowSong(),0,0,&set,&arrangeeditor->activepattern->quantizeeffect,0);
			}
			else
				w->WindowToFront(true);
		}
		break;

	case FX_PATTERNMIDICHANNEL_ID:
		if(arrangeeditor->activepattern)
		{
			MIDIPattern *MIDIPattern=(MIDIPattern *)arrangeeditor->activepattern;

			arrangeeditor->DeletePopUpMenu(true);

			if(arrangeeditor->popmenu)
			{
				class menu_MIDIchl:public guiMenu
				{
				public:
					menu_MIDIchl(MIDIPattern *mp,UBYTE c)
					{
						MIDIPattern=mp;
						channel=c;
					}

					void MenuFunction()
					{
						MIDIPattern->GetMIDIFX()->channel=channel;
					} //

					MIDIPattern *mpattern;
					UBYTE channel;
				};

				for(int m=0;m<17;m++)
					arrangeeditor->popmenu->AddFMenu(MIDIchannels[m],new menu_MIDIchl(MIDIPattern,m),MIDIPattern->GetMIDIFX()->channel==m?true:false);

				arrangeeditor->ShowPopMenu();
			}
		}
		break;

	case FX_PATTERNBANKSELECT_ID:
		if(arrangeeditor->activepattern)
		{
			MIDIPattern *MIDIPattern=(MIDIPattern *)arrangeeditor->activepattern;

			mainsettings->synthdevices.BuildPopUp(&MIDIPattern->MIDIprogram,arrangeeditor,g->x2,g->y,"Edit Pattern Device Program");
		}
		break;

		// Track ----------------------------------
	case FX_GROUPMUTE_ID:
		if(track && track->GetGroups()->activegroup)
		{
			// Toggle Group Mute
			if(track->GetGroups()->activegroup->mute==true)
				track->GetGroups()->activegroup->mute=false;
			else 
				track->GetGroups()->activegroup->mute=true;
		}
		break;

	case FX_GROUPSOLO_ID:
		if(track && track->GetGroups()->activegroup)
		{
			if(track->GetGroups()->activegroup->solo==true)
				track->song->SetGroupSolo(track->GetGroups()->activegroup,false);
			else
				track->song->SetGroupSolo(track->GetGroups()->activegroup,true);

			//	ShowGroup();
		}
		break;

	case FX_GROUPREC_ID:
		if(track && track->GetGroups()->activegroup)
		{
			if(track->GetGroups()->activegroup->rec==true)
				track->song->SetGroupRec(track->GetGroups()->activegroup,false);
			else
				track->song->SetGroupRec(track->GetGroups()->activegroup,true);
		}
		break;

	case FX_GROUP_ID:
		if(track)
		{
			arrangeeditor->DeletePopUpMenu(true);

			if(arrangeeditor->popmenu)
			{	
				class menu_opengroupeditor:public guiMenu
				{
				public:
					menu_opengroupeditor(Seq_Song *s){song=s;}

					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_GROUP,song,0,0,0,0,0);
					}

					Seq_Song *song;
				};

				arrangeeditor->popmenu->AddFMenu("Group Editor",new menu_opengroupeditor(arrangeeditor->WindowSong()));

				{
					Seq_Group_GroupPointer *sgp=track->GetGroups()->FirstGroup();

					if(sgp)
					{
						class menu_editgroup:public guiMenu
						{
						public:
							menu_editgroup(Seq_Track *t,Seq_Group *g)
							{
								track=t;
								group=g;
							}

							void MenuFunction()
							{
								track->GetGroups()->activegroup=group;
								maingui->RefreshAllArrangeWithGroup(group);
							} //

							Seq_Track *track;
							Seq_Group *group;
						};

						arrangeeditor->popmenu->AddLine();

						while (sgp)
						{
							if(char *h=mainvar->GenerateString("<+> ",sgp->group->name))
							{

								if(guiMenu *g=arrangeeditor->popmenu->AddMenu(h,0))
								{
									class menu_nogroup:public guiMenu
									{
									public:
										menu_nogroup(Seq_Song *s,Seq_Group_GroupPointer *g)
										{
											song=s;
											pointer=g;
										}

										void MenuFunction()
										{
											song->RemoveBusFromGroupFromSelectedTracks(pointer);
										} //

										Seq_Song *song;
										Seq_Group_GroupPointer *pointer;
									};

									char *h=mainvar->GenerateString(Cxs[CXS_DELETE],"<>Track<->Group:",sgp->group->name);

									if(h)
									{
										g->AddFMenu(h,new menu_nogroup(arrangeeditor->WindowSong(),sgp));
										delete h;
									}

									if(track->GetGroups()->activegroup!=sgp->group)
									{
										g->AddFMenu(Cxs[CXS_EDIT],new menu_editgroup(track,sgp->group));
									}

								}

								delete h;
							}

							sgp=sgp->NextGroup();
						}
					}
				}

				// Add
				if(track->song->FirstGroup())
				{
					bool add=true;
					Seq_Group *sg=track->song->FirstGroup();

					while (sg)
					{
						class menu_addgroup:public guiMenu
						{
						public:
							menu_addgroup(Seq_Song *s,Seq_Group *g)
							{
								song=s;
								group=g;
							}

							void MenuFunction()
							{
								song->AddGroupToSelectedTracks(group);
							} //

							Seq_Song *song;
							Seq_Group *group;
						};

						if(track->GetGroups()->FindGroup(sg)==false)
						{
							if(add==true)
							{
								arrangeeditor->popmenu->AddLine();
								add=false;
							}

							if(char *h=mainvar->GenerateString("[-] ",sg->name))
							{
								guiMenu *g=arrangeeditor->popmenu->AddFMenu(h,new menu_addgroup(track->song,sg));
								delete h;
							}
						}

						sg=sg->NextGroup();
					}
				}

				arrangeeditor->ShowPopMenu();
			}
		}
		break;

	case FX_DELAY_ID:
		if(track)
		{
			if(EditData *edit=new EditData)
			{
				edit->win=arrangeeditor;
				edit->x=g->x2;
				edit->y=g->y;

				edit->id=TRACKFX_DELAY;
				edit->name=mainvar->GenerateString("Track Delay",":",track->GetName());
				edit->deletename=true;

				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->from=-SAMPLESPERBEAT;
				edit->to=SAMPLESPERBEAT;
				edit->value=track->GetFX()->delay;

				maingui->EditDataValue(edit);
			}
		}
		break;

	case FX_TRANSPOSE_ID:
		if(track)
		{
			if(EditData *edit=new EditData)
			{
				edit->win=arrangeeditor;
				edit->x=g->x2;
				edit->y=g->y;

				edit->id=TRACKFX_TRANSPOSE;
				edit->name=mainvar->GenerateString(Cxs[CXS_TRANSTRACK],":",track->GetName());
				edit->deletename=true;

				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->from=-127;
				edit->to=127;
				edit->value=track->GetFX()->MIDI_transpose;

				maingui->EditDataValue(edit);
			}
		}
		break;

	case FX_VELOCITY_ID:
		if(track)
		{
			if(EditData *edit=new EditData)
			{
				edit->win=arrangeeditor;
				edit->id=TRACKFX_VELOCITY;

				edit->x=g->x2;
				edit->y=g->y;
				edit->name=mainvar->GenerateString("Track Velocity",":",track->GetName());
				edit->deletename=true;


				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->from=-127;
				edit->to=127;
				edit->value=track->GetFX()->GetVelocity();

				maingui->EditDataValue(edit);
			}
		}
		break;

	case FX_TRACKSTATUS_ID:
		if(track)
		{
			//arrangeeditor->EditTrackRecordType(track);
		}
		break;

	case FX_MIDITHRU_ID:
		if(track)
		{
			track->t_trackeffects.MIDIthru=track->t_trackeffects.MIDIthru==true?false:true;
		}
		break;

	case FX_MIDICHANNEL_ID:
		if(track)
		{
			arrangeeditor->DeletePopUpMenu(true);

			if(arrangeeditor->popmenu)
			{
				class menu_MIDIchl:public guiMenu
				{
				public:
					menu_MIDIchl(Seq_Track *t,int c)
					{
						track=t;
						channel=c;
					}

					void MenuFunction()
					{
						track->LockOpenOutputNotes();
						track->SendAllOpenOutputNotes();
						track->GetFX()->SetChannel(channel);
						track->UnlockOpenOutputNotes();
					} //

					Seq_Track *track;
					int channel;
				};

				for(int m=0;m<17;m++)
					arrangeeditor->popmenu->AddFMenu(MIDIchannels[m],new menu_MIDIchl(track,m),track->GetFX()->MIDI_channel==m?true:false);

				arrangeeditor->ShowPopMenu();
			}
		}
		break;

	case FX_TRACKNAME_ID:
		if(track)
		{
			char *name=g->GetString(STANDARDSTRINGLEN);

			if(name)
			{
				track->SetName(name);
				track->ShowTrackName(arrangeeditor);
			}
		}
		break;

	case FX_MIDIIN_ID:
		if(track)
		{
		
		}
		break;

	case FX_INFILTER_ID:
		if(track)
		{
			guiWindow *win=maingui->FindWindow(EDITORTYPE_MIDIFILTER,&track->GetFX()->inputfilter,0);

			if(!win)
			{
				mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].x=arrangeeditor->GetWinPosX()+g->x2;
				mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].y=arrangeeditor->GetWinPosY()+g->y;

				guiWindow *w=maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,0,0,0,0,&track->GetFX()->inputfilter,0);

				if(w)
				{
					w->parent=arrangeeditor;
					w->guiSetWindowText("Track MIDI Input Filter");
					Edit_MIDIFilter *em=(Edit_MIDIFilter *)w;
					em->SetInfo(track->GetName());
				}
			}
			else
				win->WindowToFront(true);
		}
		break;

	case FX_MIDIOUT_ID:
		if(track)
		{
			
		}
		break;

		/*
		case FX_AUDIOOUTFX_ID:
		if(track)
		{
		if(track->parent)
		track=track->parent;

		maingui->OpenEditorStart(EDITORTYPE_TRACKAUDIOMIXER,track->song,track,0,0,0,0);
		}
		break;

		case FX_AUDIOOUT_ID:
		maingui->EditTrackOutput(arrangeeditor,g->x2,g->y,track);
		break;
		*/

	case FX_QUANTIZE_ID:
		if(track)
		{
			guiWindow *w=maingui->FirstWindow();

			while(w)
			{
				if(w->GetEditorID()==EDITORTYPE_QUANTIZEEDITOR && ((Edit_QuantizeEditor *)w)->effect==&track->GetFX()->quantizeeffect)
					break;

				w=w->NextWindow();
			}

			if(!w)
			{
				mainsettings->windowpositions[EDITORTYPE_QUANTIZEEDITOR].x=arrangeeditor->GetWinPosX()+g->x2;
				mainsettings->windowpositions[EDITORTYPE_QUANTIZEEDITOR].y=arrangeeditor->GetWinPosY()+g->y;

				w=maingui->OpenEditorStart(EDITORTYPE_QUANTIZEEDITOR,arrangeeditor->WindowSong(),0,0,0,&track->GetFX()->quantizeeffect,0);
			}
			else
				w->WindowToFront(true);
		}
		break;

	case FX_FILTER_ID:
		if(track)
		{
			guiWindow *win=maingui->FindWindow(EDITORTYPE_MIDIFILTER,&track->GetFX()->filter,0);

			if(!win)
			{
				mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].x=arrangeeditor->GetWinPosX()+g->x2;
				mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].y=arrangeeditor->GetWinPosY()+g->y;

				guiWindow *w=maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,0,0,0,0,&track->GetFX()->filter,0);

				if(w)
				{
					w->parent=arrangeeditor;
					w->guiSetWindowText(Cxs[CXS_MIDIOUTFILTER]);

					Edit_MIDIFilter *mf=(Edit_MIDIFilter *)w;

					char h2[NUMBERSTRINGLEN];
					char *tracknr=mainvar->ConvertIntToChar(arrangeeditor->WindowSong()->GetOfTrack(track)+1,h2);

					if(char *i=mainvar->GenerateString("Track ",tracknr,":",track->GetName()))
					{
						mf->SetInfo(i);
						delete i;
					}
				}
			}
			else
				win->WindowToFront(true);		
		}
		break;

	default:
		return g;
	}

	return 0;
}

void Edit_ArrangeEditorEffects::ResetGadgets()
{
	patternloopnumber=0;

	MIDIthru=
	trackimage=
		MIDIinfilter=
		trackstatus=
		trackonoff=
		patternonoff=
		group=
		group_solo=
		group_mute=
		group_rec=
		patternloopendless=
		delay=
		trackname=
		MIDIchannel=
		MIDIoutput=
		MIDIinput=
		quantize=
		transpose=
		velocity=
		patternname=
		patternquantize=
		patternMIDIchannel=
		patternloop=
		patternloopflag=
		patternmute=
		patternvelocity=
		patterntranspose=
		patternbankselect=
		filter=
		processor=
		MIDIoutprogram=
		0;
}

void Edit_ArrangeEditorEffects::InitTrackMixer(int sy)
{
#ifdef OLDIE
	return ;
	Seq_Track *oldtrack=trackmix.track;

	trackmix.SetSong(arrangeeditor->WindowSong());

	trackmix.track=arrangeeditor->WindowSong()->GetFocusTrack(); // track or NULL
	//if(trackmix.track && trackmix.track->parent)
	//	trackmix.track=trackmix.track->parent;

	trackmix.solotrack=true;
	trackmix.simple=true;

	bool newwin=true;
	int newwidth=frame->x2-frame->x,newheight=frame->y2-sy;

	HWND deletewin=0;

	if(trackmix.hWnd)
	{
		if(frame->y!=oldframey || frame->x!=oldframex || trackmix.width!=newwidth || trackmix.height!=newheight)
		{
			deletewin=trackmix.hWnd;
			//DestroyWindow(trackmix.hWnd);
			trackmix.hWnd=0;
		}
		else newwin=false;
	}

	if(newwin==false && trackmix.track)
	{
		if(Edit_AudioMixEffects *eam=trackmix.FirstAudioChannel())
		{
			if(eam->effectgadgets.GetCount()==trackmix.track->io.audioeffects.GetCountEffects() &&
				eam->sendgadgets.GetCount()==trackmix.track->io.GetCountSends()
				)
			{	
				eam->ChangeToTrack(trackmix.track);

				//eam->ShowInputPeak(true);
				eam->ShowIOPeak(true);

				eam->ShowPlaybackChannel(false);
				eam->ShowRecordChannel();

				return;
			}
		}
	}

	trackmix.FreeMemory();

	if(trackmix.track)
	{
		// Open win inside window
		if(newwin==true)
		{
			trackmix.hWnd=
				CreateWindowEx
				(
				WS_EX_TOPMOST,
				"PIP Window", 
				0,
				WS_CHILD|WS_CLIPSIBLINGS,
				frame->x,
				sy,
				newwidth,
				newheight,  
				arrangeeditor->hWnd,
				0, // Screen Menu Global
				maingui->hInst,
				NULL
				);

			oldframex=frame->x;
			oldframey=frame->y;
		}

		if(trackmix.hWnd)
		{
			if(newwin==true)
				trackmix.InitChildWindow(arrangeeditor,frame->x2-frame->x,frame->y2-sy);
			else
			{
				RECT repaint_rect;
				GetClientRect(trackmix.hWnd, &repaint_rect);
				FillRect(trackmix.bitmap.hDC, &repaint_rect, (HBRUSH)(COLOR_WINDOW));
			}

			//if((arrangeeditor->winmode&WINDOWMODE_INIT)==0)
			trackmix.winmode|=WINDOWMODE_INIT;
			//trackmix.Init();

			guiWindowSetting settings;

			maingui->InitNewWindow(&trackmix,&settings);
			//ShowWindow(trackmix.hWnd,SW_SHOW);

			//if(newwin==false && arrangeeditor->winmode==WINDOWMODE_NORMAL)
			//	trackmix.RedrawOSGadgets();
		}

		if(deletewin)
			DestroyWindow(deletewin);
	}
#endif
}

bool Edit_ArrangeEditorEffects::Init()
{
	// Reset gadgets
	ResetGadgets();

	if(frame && frame->ondisplay==true && arrangeeditor->showeffects==true && arrangeeditor->guictrlbox.stopbutton)
	{
		frame->y=arrangeeditor->guictrlbox.stopbutton->y2+2;

		InitTrackGadgets(frame->y);
		InitPatternGadgets(endtracky);
		InitTrackMixer(starttrackfx_y);

		return true;
	}

	return false;
}

Edit_ArrangeEditorEffects::Edit_ArrangeEditorEffects()
{	
	frame=0;

	status_track=0;
	status_pattern=0;

	showtrack=true;
	showpattern=true;

	trackglist=patternglist=0;

	status_MIDIoutdevice=0;
	status_MIDIindevice=0;
	patternnamestring=0;

	dontshowpatternname=false;
	status_processorname=0;

	ResetGadgets();
};

void Edit_ArrangeEditorEffects::FreeMemory()
{
	if(patternnamestring)delete patternnamestring;
	patternnamestring=0;

	if(status_processorname)delete status_processorname;
	status_processorname=0;

	ResetGadgets();
	trackglist=patternglist=0;

	trackmix.FreeMemory();
}
#endif