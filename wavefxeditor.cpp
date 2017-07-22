#include "songmain.h"
#include "editor.h"

#include "gui.h"
#include "trackfx.h"
#include "MIDIhardware.h"
#include "editfunctions.h"
#include "waveeditor.h"
#include "waveeditordefines.h"
#include "languagefiles.h"

void Edit_WaveEditorEffects::ShowActiveWaveTrack()
{
	if(waveeditor->focustrack)
	{
		if(trackname)
			trackname->SetString(waveeditor->focustrack->name);

		if(trackstatus)
		{
			size_t l=strlen(waveeditor->focustrack->group);

			if(waveeditor->focustrack->name)
				l+=strlen(waveeditor->focustrack->name);

			l+=8;

			if(char *status=new char[l])
			{
				mainvar->MixString(status,waveeditor->focustrack->group,":");

				if(waveeditor->focustrack->name)
					mainvar->AddString(status,waveeditor->focustrack->name);

				trackstatus->ChangeButtonText(status);

				delete status;
			}
		}

		if(trackchannel)
		{
			if(waveeditor->focustrack->chl)
			{
				if(char *MIDI=mainvar->GenerateString(Cxs[CXS_MIDICHANNEL],":",MIDIchannels[waveeditor->focustrack->chl]))
				{
					trackchannel->ChangeButtonText(MIDI);
					delete MIDI;
				}

			}
			else
				trackchannel->ChangeButtonText("Show all Channels");
		}

		if(trackdata1)
			trackdata1->ChangeButtonText(waveeditor->focustrack->CreateData1Name(waveeditor->WindowSong()));

		if(trackdata2)
		{
			if(waveeditor->focustrack->CreateData2Name(waveeditor->WindowSong()))
				trackdata2->ChangeButtonText(waveeditor->focustrack->CreateData2Name(waveeditor->WindowSong()));
			else
				trackdata2->Disable();
		}


		/*
		// MIDI Channesl
		if(MIDIchannel)
		MIDIchannel->ChangeButtonText(MIDIchannels[status_channel=track->MIDIeffects.channel]);

		// MIDI Out Port
		if(MIDIoutput)
		{
		if(!(status_MIDIoutdevice=track->outdevice))
		c="No MIDI";
		else
		c=track->outdevice->name;

		MIDIoutput->ChangeButtonText(c);
		}

		//Audio Out Channel

		if(audiooutput)
		{
		if(status_audiochannel=track->audiochannel)
		c=track->audiochannel->name;
		else
		c="No Audio";

		audiooutput->ChangeButtonText(c);
		}

		if(quantize)// Quantize/Groove
		{
		if(status_groove=track->MIDIeffects.groove)
		{
		c=track->MIDIeffects.groove->GetGrooveName();
		status_quantize=0;
		}
		else
		c=quantstr[status_quantize=track->MIDIeffects.quantize];

		quantize->ChangeButtonText(c);
		}

		if(capture)
		{
		capture->SetCheckBox(track->MIDIeffects.capturequant);
		}

		if(noteon)
		{
		noteon->SetCheckBox(track->MIDIeffects.noteonquant);
		}

		if(noteoff)
		{
		noteoff->SetCheckBox(track->MIDIeffects.noteoffquant);
		}
		*/

	}
	else
	{
		if(trackname)
			trackname->Disable();

		if(trackstatus)
			trackstatus->Disable();

		if(trackchannel)
			trackchannel->Disable();

		if(trackdata1)
			trackdata1->Disable();

		if(trackdata2)
			trackdata2->Disable();
	}
}

bool Edit_WaveEditorEffects::Init()
{
	InitGadgets();
	ShowActiveWaveTrack();
	return true;
}

void Edit_WaveEditorEffects::AddKeysToMenu(guiMenu *m,WaveTrack *track)
{
	guiMenu *s;

	class menu_MIDInote:public guiMenu
	{
	public:
		menu_MIDInote(Edit_WaveEditorEffects *e,int n)
		{
			ed=e;
			note=n;
		}

		void MenuFunction()
		{
			if(ed->waveeditor->focustrack->status!=NOTEON ||
				ed->waveeditor->focustrack->byte1!=note //  Key
				)
			{
				if(Edit_WaveTrack_Note *ewc=new Edit_WaveTrack_Note())
				{
					ewc->byte1=note;
					ed->waveeditor->ReplaceTrackWithTrack(ed->waveeditor->focustrack,ewc);
				}
			}
		} 

		Edit_WaveEditorEffects *ed;
		int note;
	};

	m->AddFMenu("All Notes",new menu_MIDInote(this,128),track->byte1==128?true:false);

	for(int k=0;k<128;k+=24)
	{
		char *h=mainvar->GenerateString("MIDI Keys ",maingui->ByteToKeyString(waveeditor->WindowSong(),k),"-",maingui->ByteToKeyString(waveeditor->WindowSong(),k+24<127?k+23:127));

		if(h)
		{
			if(s=m->AddMenu(h,0))
			{
				for(int i=k;i<k+24 && i<128;i++)
				{
					s->AddFMenu(maingui->ByteToKeyString(waveeditor->WindowSong(),i),new menu_MIDInote(this,i),track->byte1==i?true:false);
					
				}
			}
			delete h;
		}

	}

}

void Edit_WaveEditorEffects::AddCtrlToMenu(guiMenu *menu,WaveTrack *track)
{
	guiMenu *s;

	class menu_MIDIctrl:public guiMenu
	{
	public:
		menu_MIDIctrl(Edit_WaveEditorEffects *e,int co)
		{
			ed=e;
			ctrlnumber=co;
		}

		void MenuFunction()
		{
			if(ed->waveeditor->focustrack->status!=CONTROLCHANGE ||
				ed->waveeditor->focustrack->byte1!=ctrlnumber
				)
			{
				Edit_WaveTrack_Control *ewc=new Edit_WaveTrack_Control();

				if(ewc)
				{
					ewc->byte1=ctrlnumber;
					ed->waveeditor->ReplaceTrackWithTrack(ed->waveeditor->focustrack,ewc);
				}
			}
		} //

		Edit_WaveEditorEffects *ed;
		int ctrlnumber;
	};

	int i=0;

	for(int a=0;a<4;a++)
	{
		char *m=0;

		switch(a)
		{
		case 0:
			if(track->status==CONTROLCHANGE && track->byte1>=0 && track->byte1<=32)
				m=">>> MIDI Control 0-31";
			else
				m="MIDI Control 0-31";
			break;

		case 1:
			if(track->status==CONTROLCHANGE && track->byte1>=32 && track->byte1<=63)
				m=">>> MIDI Control 32-63";
			else
				m="MIDI Control 32-63";
			break;

		case 2:
			if(track->status==CONTROLCHANGE && track->byte1>=64 && track->byte1<=95)
				m=">>> MIDI Control 64-95";
			else
				m="MIDI Control 64-95";
			break;

		case 3:
			if(track->status==CONTROLCHANGE && track->byte1>=96)
				m=">>> MIDI Control 96-127";
			else
				m="MIDI Control 96-127";
			break;
		}

		s=menu->AddMenu(m,0);

		if(s)
		{
			int c=32;

			while(c--)
			{
				bool sel;

				if(track->status==CONTROLCHANGE && track->byte1==i)
					sel=true;
				else
					sel=false;

				s->AddFMenu(maingui->ByteToControlInfo(i,-1,true),new menu_MIDIctrl(this,i),sel);

				i++;
			}
		}
	}

	/*	
	if(s)
	{
	for(i=0;i<31;i++)
	{

	}
	}

	s=m->AddMenu("MIDI Control 32-63",0);
	if(s)
	{
	for(i=32;i<63;i++)
	{
	bool sel;

	if(track->status==CONTROLCHANGE && track->byte1==i)
	sel=true;
	else
	sel=false;

	s->AddFMenu(maingui->ByteToControlInfo(i,true),new menu_MIDIctrl(this,i),sel);
	}
	}

	s=m->AddMenu("MIDI Control 64-95",0);
	if(s)
	{
	for(i=64;i<95;i++)
	{
	bool sel;

	if(track->status==CONTROLCHANGE && track->byte1==i)
	sel=true;
	else
	sel=false;

	s->AddFMenu(maingui->ByteToControlInfo(i,true),new menu_MIDIctrl(this,i),sel);
	}
	}

	s=m->AddMenu("MIDI Control 96-127",0);
	if(s)
	{
	for(i=96;i<127;i++)
	{
	bool sel;

	if(track->status==CONTROLCHANGE && track->byte1==i)
	sel=true;
	else
	sel=false;

	s->AddFMenu(maingui->ByteToControlInfo(i,true),new menu_MIDIctrl(this,i),sel);
	}
	}
	*/
}

guiGadget  *Edit_WaveEditorEffects::Gadget(guiGadget *g)
{
	WaveTrack *track=waveeditor->focustrack;

	if(track)
	{
		waveeditor->DeletePopUpMenu(true);

		if(waveeditor->popmenu)
		{	
			switch(g->gadgetID)
			{
			case FX_WAVE_TRACKNAME_ID:
				{
					return 0;
				}
				break;

			case FX_WAVE_TRACKSTATUS_ID: // Track Status
				{
					// MIDI
					class menu_note:public guiMenu
					{
					public:
						menu_note(Edit_WaveEditorEffects *e)
						{
							ed=e;
						}

						void MenuFunction()
						{	
							if(ed->waveeditor->focustrack->status!=NOTEON)
								ed->waveeditor->ReplaceTrackWithTrack(ed->waveeditor->focustrack,new Edit_WaveTrack_Note());
						}

						Edit_WaveEditorEffects *ed;
					};

					bool sel;

					if(track->status==NOTEON)
						sel=true;
					else
						sel=false;

					waveeditor->popmenu->AddFMenu("Note",new menu_note(this),sel);

					class menu_progchg:public guiMenu
					{
					public:
						menu_progchg(Edit_WaveEditorEffects *e)
						{
							ed=e;
						}

						void MenuFunction()
						{	
							if(ed->waveeditor->focustrack->status!=PROGRAMCHANGE)
								ed->waveeditor->ReplaceTrackWithTrack(ed->waveeditor->focustrack,new Edit_WaveTrack_Program());
						} //

						Edit_WaveEditorEffects *ed;
					};

					if(track->status==PROGRAMCHANGE)
						sel=true;
					else
						sel=false;

					waveeditor->popmenu->AddFMenu("Program Change",new menu_progchg(this),sel);

					class menu_pitch:public guiMenu
					{
					public:
						menu_pitch(Edit_WaveEditorEffects *e)
						{
							ed=e;
						}

						void MenuFunction()
						{	
							if(ed->waveeditor->focustrack->status!=PITCHBEND)
								ed->waveeditor->ReplaceTrackWithTrack(ed->waveeditor->focustrack,new Edit_WaveTrack_Pitchbend());
						} //

						Edit_WaveEditorEffects *ed;
					};

					if(track->status==PITCHBEND)
						sel=true;
					else
						sel=false;

					waveeditor->popmenu->AddFMenu("Pitchbend",new menu_pitch(this),sel);

					class menu_cpress:public guiMenu
					{
					public:
						menu_cpress(Edit_WaveEditorEffects *e)
						{
							ed=e;
						}

						void MenuFunction()
						{
							if(ed->waveeditor->focustrack->status!=CHANNELPRESSURE)
								ed->waveeditor->ReplaceTrackWithTrack(ed->waveeditor->focustrack,new Edit_WaveTrack_ChannelPressure());	
						} //

						Edit_WaveEditorEffects *ed;
					};

					if(track->status==CHANNELPRESSURE)
						sel=true;
					else
						sel=false;

					waveeditor->popmenu->AddFMenu("Channel Pressure",new menu_cpress(this),sel);

					class menu_ppress:public guiMenu
					{
					public:
						menu_ppress(Edit_WaveEditorEffects *e)
						{
							ed=e;
						}

						void MenuFunction()
						{
							if(ed->waveeditor->focustrack->status!=POLYPRESSURE)
								ed->waveeditor->ReplaceTrackWithTrack(ed->waveeditor->focustrack,new Edit_WaveTrack_PolyPressure());	
						} //

						Edit_WaveEditorEffects *ed;
					};

					if(track->status==POLYPRESSURE)
						sel=true;
					else
						sel=false;

					waveeditor->popmenu->AddFMenu("Poly Pressure",new menu_ppress(this),sel);

					AddCtrlToMenu(waveeditor->popmenu,track);

					waveeditor->ShowPopMenu();

					return 0;
				}
				break;

			case FX_WAVE_TRACKDATA1_ID:
				{
					switch(track->status)
					{
					case NOTEON:
						{
							AddKeysToMenu(waveeditor->popmenu,track);
							waveeditor->ShowPopMenu();
						}
						break;

					case CONTROLCHANGE: // select ctrl
						{
							AddCtrlToMenu(waveeditor->popmenu,track);

							waveeditor->ShowPopMenu();
						}
						break;
					}

					return 0;
				}
				break;

			case FX_WAVE_TRACKDATA2_ID:
				{
					return 0;
				}
				break;

			case FX_WAVE_TRACKCHANNEL_ID: // MIDI Channel
				{
					class menu_tchl:public guiMenu
					{
					public:
						menu_tchl(Edit_WaveEditorEffects *e,int chl)
						{
							ed=e;
							channel=chl;
						}

						void MenuFunction()
						{	
							if(ed->waveeditor->focustrack->chl!=channel)
								ed->waveeditor->ReplaceTrackChannel(ed->waveeditor->focustrack,channel);
						} //

						Edit_WaveEditorEffects *ed;
						int channel; // 0==ALL, 1-16
					};

					for(int i=0;i<17;i++)
					{
						char *MIDI;

						if(!i)
							MIDI=mainvar->GenerateString("ALL Channels");
						else
							MIDI=mainvar->GenerateString("MIDI Channel:",MIDIchannels[i]); // 0==THRu!

						if(MIDI)
						{
							waveeditor->popmenu->AddFMenu(MIDI,new menu_tchl(this,i),track->chl==i?true:false);
							delete MIDI;
						}		
					}

					waveeditor->ShowPopMenu();

					return 0;
				}
				break;
			}

		}	
	}

	return g;
}

void Edit_WaveEditorEffects::InitGadgets()
{
#ifdef OLDIE
	// Reset gadgets
	ResetGadgets();

	int y=frame->y;
	int y2;

	int x=frame->x;
	int x2=frame->x2;

	guiGadgetList *nl=waveeditor->gadgetlists.AddGadgetList(waveeditor);

	int addy=20;

	if(nl)
	{		
		if((y2=y+addy)<=frame->y2)
		{
			trackname=nl->AddString(x,y,x2,y2,FX_WAVE_TRACKNAME_ID,0,0,"Name");
			y=y2+1;
		}

		// Status
		if((y2=y+addy)<=frame->y2)
		{
			trackstatus=nl->AddButton(x,y,x2,y2,0,FX_WAVE_TRACKSTATUS_ID,0,"Status");
			y=y2+1;
		}

		if((y2=y+addy)<=frame->y2)
		{
			trackchannel=nl->AddButton(x,y,x2,y2,0,FX_WAVE_TRACKCHANNEL_ID,0,"MIDI Channel");
			y=y2+1;
		}


		// Data1
		if((y2=y+addy)<=frame->y2)
		{
			trackdata1=nl->AddButton(x,y,x2,y2,0,FX_WAVE_TRACKDATA1_ID,0,"Value 1");
			y=y2+1;
		}

		if((y2=y+addy)<=frame->y2)
		{
			trackdata2=nl->AddButton(x,y,x2,y2,0,FX_WAVE_TRACKDATA2_ID,0,"Value 2");
			y=y2+1;
		}

		//ShowActiveWaveTrack();
	}
#endif

}// if track