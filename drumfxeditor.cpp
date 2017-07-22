#include "songmain.h"
#include "editor.h"
#include "gui.h"
#ifdef OLDIE

#include "trackfx.h"
#include "MIDIhardware.h"
#include "editfunctions.h"
#include "drumeditor.h"
#include "gmdrums.h"
#include "languagefiles.h"
#include "object_song.h"
#include "editdata.h"

void Edit_DrumEditorEffects::ShowTrackVolumeButton()
{
	if(drumeditor->focustrack)
	{
		if(trackvolumebutton)
		{
			char h[128];
			char th[NUMBERSTRINGLEN];

			mainvar->MixString(h,"Vol:",mainvar->ConvertIntToChar(rl_volumebutton=drumeditor->focustrack->volume,th));
			trackvolumebutton->ChangeButtonText(h);
		}
	}
}

void Edit_DrumEditorEffects::ShowActiveTrack()
{
	if(drumeditor->focustrack)
	{
		// char *c;
		
		if(drummap)
		{
			char *c=mainvar->GenerateString("Map:",drumeditor->drummap->name);
			
			if(c)
			{
				drummap->ChangeButtonText(c);
				delete c;
			}
		}
		
		if(trackname)
			trackname->SetString(drumeditor->focustrack->name);
		
		// Channel
		if(MIDIchannel)
		{
			char h[128];

			strcpy(h,"MIDI Chl:");
			mainvar->AddString(h,MIDIchannels[drumeditor->focustrack->channel+1]); // +1== no thru
			MIDIchannel->ChangeButtonText(h); 
		}

		if(MIDIoutput)
		{
			char *c;
			
			if(!drumeditor->focustrack->MIDIoutdevice)
				c="Track Device";
			else
				c=drumeditor->focustrack->MIDIoutdevice->name;
			
			if(char *MIDIout=mainvar->GenerateString("MIDI Out:",c))
			{
				MIDIoutput->ChangeButtonText(MIDIout);
				delete MIDIout;
			}
		}
		
		if(audiochannel)
		{
			char *c;
			
			if(!drumeditor->focustrack->audiochannel)
				c="Track Device";
			else
				c=drumeditor->focustrack->audiochannel->name;
			
			if(char *audioout=mainvar->GenerateString("Audio Chl:",c))
			{
				audiochannel->ChangeButtonText(audioout);
				delete audioout;
			}
		}

		if(MIDIkey)
		{
			char drumname[255];
			
			if(drumeditor->drummap->usegm==true)
			{
				GMMap gmmap;
				
				mainvar->MixString(drumname,maingui->ByteToKeyString(drumeditor->WindowSong(),drumeditor->focustrack->key)," - ");
				mainvar->AddString(drumname,gmmap.keys[drumeditor->focustrack->key]);
				
				MIDIkey->ChangeButtonText(drumname);
			}
			else
			{
				MIDIkey->ChangeButtonText(maingui->ByteToKeyString(drumeditor->WindowSong(),drumeditor->focustrack->key));
			}
		}
		
		if(drumlength)
		{
			char help[256];
			
			strcpy(help,"Ticks:");
			
			mainvar->AddString(help,mainvar->ConvertTicksToChar(drumeditor->focustrack->ticklength));
			
			drumlength->ChangeButtonText(help);
		}

		if(trackvolume)
		{
			trackvolume->ChangeSlider(drumeditor->focustrack->volume);
		}

		if(mute)
		{
			mute->SetCheckBox(drumeditor->focustrack->mute);
		}

		ShowTrackVolumeButton();
	}
	else
	{
		if(mute)
			mute->Disable();

		if(drummap)
		{
			if(drumeditor->drummap)
			{
				if(char *h=mainvar->GenerateString("Map:",drumeditor->drummap->GetName()) )
				{
					drummap->ChangeButtonText(h);
					delete h;
				}
			}
			else
				drummap->ChangeButtonText("Select Drum Map");

			drummap->Enable();
		}

		if(trackname)
			trackname->Disable();

		if(MIDIchannel)
			MIDIchannel->Disable();
		
		if(MIDIoutput)
			MIDIoutput->Disable();
		
		if(MIDIkey)
			MIDIkey->Disable();
		
		if(drumlength)
			drumlength->Disable();

		if(trackvolume)
			trackvolume->Disable();

		if(trackvolumebutton)
			trackvolumebutton->Disable();

		if(audiochannel)
			audiochannel->Disable();
	}
}

EditData *Edit_DrumEditorEffects::EditDataMessage(EditData *data)
{
	switch(data->id)
	{
	case EDIT_DRUMTRACKFX_TICKS:
		{
			if(drumeditor->focustrack->ticklength!=data->newvalue)
			{
				drumeditor->focustrack->ticklength=data->newvalue;
				drumeditor->drummap->RefreshAllEditors();
			}
		}
		break;

	default:
		return data;
		break;
	}

	return 0;
}

void Edit_DrumEditorEffects::CreateNewSongDrumMap()
{
	if(drumeditor->WindowSong())
	{
		if(Drummap *dm=new Drummap)
		{
			drumeditor->drummap=dm;
			drumeditor->WindowSong()->AddDrumMap(dm);

			drumeditor->ShowDrumsHoriz(SHOWEVENTS_TRACKS|SHOWEVENTS_EVENTS);

			drumeditor->SongNameRefresh();
			ShowActiveTrack();
		}

	}
}

void Edit_DrumEditorEffects::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case FX_DRUM_TRACKNAME_ID:
		if(drumeditor->focustrack)
		{
			drumeditor->SetName(drumeditor->focustrack,g->string);
		}
		break;

	case FX_DRUM_DRUMMAP_ID:
		if(drumeditor->WindowSong())
		{
			drumeditor->DeletePopUpMenu(true);

			if(drumeditor->popmenu)
			{
				// Song's Drummaps
				if(drumeditor->WindowSong()->FirstDrumMap())
				{
					class menu_selectsongdrummap:public guiMenu
					{
					public:
						menu_selectsongdrummap(Edit_Drum *ed,Drummap *dm)
						{
							editor=ed;
							map=dm;
						}

						void MenuFunction()
						{

							editor->SelectDrumMap(map);
						} //

						Edit_Drum *editor;
						Drummap *map;
					};

					guiMenu *smenu=drumeditor->popmenu->AddMenu("Select Song Drum Map",0);

					if(smenu)
					{
						char h2[NUMBERSTRINGLEN];
						Drummap *sdm=drumeditor->WindowSong()->FirstDrumMap();
						while(sdm)
						{
							if(char *h=mainvar->GenerateString(sdm->GetName()," <Tracks:",mainvar->ConvertIntToChar(sdm->GetCountOfTracks(),h2),">"))
							{
								smenu->AddFMenu(h,new menu_selectsongdrummap(drumeditor,sdm),sdm==drumeditor->drummap?true:false);
								delete h;
							}
							sdm=sdm->NextMap();
						}
					}

					if(drumeditor->drummap)
					{
						class menu_renamesongdrummap:public guiMenu
						{
						public:
							menu_renamesongdrummap(Edit_Drum *ed,Drummap *dm,int x,int y)
							{
								editor=ed;
								map=dm;
								posx=x;
								posy=y;
							}

							void MenuFunction()
							{
								editor->RenameDrumMap(map,posx,posy);
							} //

							Edit_Drum *editor;
							Drummap *map;
							int posx,posy;
						};

						char *h=mainvar->GenerateString("Rename DrumMap:",drumeditor->drummap->GetName());
						if(h)
						{
							drumeditor->popmenu->AddFMenu(h,new menu_renamesongdrummap(drumeditor,drumeditor->drummap,g->x2,g->y));
							delete h;
						}
					}

				}
				else
				{
					drumeditor->popmenu->AddMenu("Song has no Drum Map...",0);
				}

				// Add new Drum Map -> Song
				class menu_newsongdrummap:public guiMenu
				{
				public:
					menu_newsongdrummap(Edit_DrumEditorEffects *ed)
					{
						editor=ed;
					}

					void MenuFunction()
					{
						editor->CreateNewSongDrumMap();
					} //

					Edit_DrumEditorEffects *editor;
				};

				drumeditor->popmenu->AddFMenu("Create new Song Drum Map",new menu_newsongdrummap(this));
				drumeditor->popmenu->AddLine();

				// Open Drum Map Editor
				drumeditor->popmenu->AddLine();

				class menu_drummapeditor:public guiMenu
				{
				public:
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_DRUMMAP,0,0,0,0,0,0);
					} //

				};

				drumeditor->popmenu->AddFMenu("Open Drum Map Editor",new menu_drummapeditor);
				drumeditor->popmenu->AddLine();

				if(maindrummap->FirstDrummap())
				{
					class menu_drummap:public guiMenu
					{
					public:
						menu_drummap(Edit_Drum *ed,Drummap *d)
						{
							editor=ed;
							drummap=d;
						}

						void MenuFunction()
						{
							editor->ChangeToDrummap(drummap);
						} //

						Edit_Drum *editor;
						Drummap *drummap;
					};

					drumeditor->popmenu->AddMenu("Show/Edit Drummap",0);
					drumeditor->popmenu->AddLine();

					Drummap *d=maindrummap->FirstDrummap();

					while(d)
					{
						drumeditor->popmenu->AddFMenu(d->name,new menu_drummap(drumeditor,d));
						d=d->NextMap();
					}
				}

				drumeditor->ShowPopMenu();
			}

		}
		break;

	case FX_DRUM_TRACKVOLUMEBUTTON_ID:
		if(drumeditor->focustrack)
		{
			if(drumeditor->focustrack->volume!=0)
			{
				drumeditor->focustrack->volume=0;	
			}
		}
		break;

	case FX_DRUM_TRACKMUTE_ID:
		if(drumeditor->focustrack)
		{
			if(drumeditor->focustrack->mute==true)
				drumeditor->focustrack->mute=false;
			else
				drumeditor->focustrack->mute=true;

			Edit_Drum_Track *ft=drumeditor->FindTrack(drumeditor->focustrack);

			if(ft)
				ft->ShowMute(true);
		}
		break;

	case FX_DRUM_TRACKVOLUME_ID:
		if(drumeditor->focustrack)
		{
			if(drumeditor->focustrack->volume!=g->GetPos())
			{
				drumeditor->focustrack->volume=g->GetPos();

				//ShowTrackVolumeButton();
			}
		}
		break;

	case FX_DRUM_MIDILENGTH_ID:
		if(drumeditor->focustrack)
		{
			class menu_MIDIlength:public guiMenu
			{
			public:
				menu_MIDIlength(Edit_Drum *ed,int c)
				{
					editor=ed;
					ticks=c;
				}

				void MenuFunction()
				{
					if(editor->focustrack->ticklength!=ticks)
					{
						editor->focustrack->ticklength=ticks;
						editor->drummap->RefreshAllEditors();
					}
				} //

				Edit_Drum *editor;
				int ticks;
			};

			class menu_MIDIlength_edit:public guiMenu
			{
			public:
				menu_MIDIlength_edit(Edit_Drum *ed,int px,int py)
				{
					editor=ed;
					x=px;
					y=py;
				}

				void MenuFunction()
				{
					EditData *edit=new EditData;

					if(edit)
					{
						edit->win=editor;
						edit->x=x;
						edit->y=y;

						edit->id=EDIT_DRUMTRACKFX_TICKS;
						edit->name="Drumtrack Ticks";

						edit->type=EditData::EDITDATA_TYPE_INTEGER;
						edit->from=1;
						edit->to=TICK1nd*4;
						edit->value=editor->focustrack->ticklength;

						maingui->EditDataValue(edit);
					}
				} //

				Edit_Drum *editor;
				int x,y;
			};

			drumeditor->DeletePopUpMenu(true);

			if(drumeditor->popmenu)
			{
				drumeditor->popmenu->AddMenu("Ticks/Length Drumtrack",0);
				drumeditor->popmenu->AddLine();

				bool sel;
				if(drumeditor->focustrack->ticklength==TICK1nd)
					sel=true;
				else
					sel=false;

				drumeditor->popmenu->AddFMenu("1/1",new menu_MIDIlength(drumeditor,TICK1nd),sel);

				if(drumeditor->focustrack->ticklength==TICK2nd)
					sel=true;
				else
					sel=false;

				drumeditor->popmenu->AddFMenu("1/2",new menu_MIDIlength(drumeditor,TICK2nd),sel);

				if(drumeditor->focustrack->ticklength==TICK4nd)
					sel=true;
				else
					sel=false;

				drumeditor->popmenu->AddFMenu("1/4",new menu_MIDIlength(drumeditor,TICK4nd),sel);

				if(drumeditor->focustrack->ticklength==TICK8nd)
					sel=true;
				else
					sel=false;

				drumeditor->popmenu->AddFMenu("1/8",new menu_MIDIlength(drumeditor,TICK8nd),sel);

				if(drumeditor->focustrack->ticklength==TICK16nd)
					sel=true;
				else
					sel=false;

				drumeditor->popmenu->AddFMenu("1/16",new menu_MIDIlength(drumeditor,TICK16nd),sel);

				if(drumeditor->focustrack->ticklength==TICK32nd)
					sel=true;
				else
					sel=false;

				drumeditor->popmenu->AddFMenu("1/32",new menu_MIDIlength(drumeditor,TICK32nd),sel);

				if(drumeditor->focustrack->ticklength==TICK64nd)
					sel=true;
				else
					sel=false;

				drumeditor->popmenu->AddFMenu("1/64",new menu_MIDIlength(drumeditor,TICK64nd),sel);

				drumeditor->popmenu->AddLine();
				drumeditor->popmenu->AddFMenu(Cxs[CXS_EDIT],new menu_MIDIlength_edit(drumeditor,g->x2,g->y));

				drumeditor->ShowPopMenu();
			}
		}
		break;

	case FX_DRUM_MIDICHANNEL_ID:
		if(drumeditor->focustrack){

			class menu_MIDIchl:public guiMenu
			{
			public:
				menu_MIDIchl(Edit_Drum *ed,int c)
				{
					editor=ed;
					channel=c;
				}

				void MenuFunction()
				{
					if(editor->focustrack->channel!=channel)
					{
						editor->focustrack->channel=channel;
						editor->drummap->RefreshAllEditors();

						editor->PlayFocusTrack();
					}	
				} //

				Edit_Drum *editor;
				int channel;
			};

			drumeditor->DeletePopUpMenu(true);

			if(drumeditor->popmenu)
			{
				for(int m=1;m<17;m++) // no thru=0
				{
					bool sel;

					if(drumeditor->focustrack->channel+1==m)
						sel=true;
					else
						sel=false;

					drumeditor->popmenu->AddFMenu(MIDIchannels[m],new menu_MIDIchl(drumeditor,m-1),sel);
				}

				drumeditor->ShowPopMenu();
			}
		}
		break;

	case FX_DRUM_MIDIKEY_ID:
		if(drumeditor->focustrack)
		{
			drumeditor->DeletePopUpMenu(true);

			if(drumeditor->popmenu)
			{
				class menu_MIDIkey:public guiMenu
				{
				public:
					menu_MIDIkey(Edit_Drum *ed,int k)
					{
						editor=ed;
						key=k;
					}

					void MenuFunction()
					{
						if(editor->drummap->usegm==true)
							key+=35;

						if(editor->focustrack->key!=key)
						{
							editor->focustrack->key=key;
							editor->drummap->RefreshAllEditors();

							editor->PlayFocusTrack();
						}
					} //

					Edit_Drum *editor;
					int key;
				};

				if(drumeditor->drummap->usegm==true)
				{
					GMMap map;

					for(int i=0;i<47;i++)
					{	
						bool sel;

						if(drumeditor->focustrack->key==i+35)
							sel=true;
						else
							sel=false;

						drumeditor->popmenu->AddFMenu(map.keys[i+35],new menu_MIDIkey(drumeditor,i),sel);
					}
				}
				else
				{
				}

				drumeditor->ShowPopMenu();
			}
		}
		break;
	}
}

bool Edit_DrumEditorEffects::Init()
{
	InitGadgets();
	
	//	ShowTrackFX();
	
	return true;
}

void Edit_DrumEditorEffects::RefreshRealtime()
{
	if(drumeditor->focustrack)
	{
		if(trackvolume && drumeditor->focustrack->volume!=rl_volumeslider)
		{
			trackvolume->ChangeSlider(rl_volumeslider=drumeditor->focustrack->volume);
		}
		
		if(trackvolumebutton && drumeditor->focustrack->volume!=rl_volumebutton)
		{
			ShowTrackVolumeButton();
		}
	}
}

void Edit_DrumEditorEffects::InitGadgets()
{
#ifdef OLDIE
	// Reset gadgets
	ResetGadgets();
	
	int y=frame->y;
	int y2;
	
	int x=frame->x;
	int x2=frame->x2;
	
	guiGadgetList *nl=drumeditor->gadgetlists.AddGadgetList(drumeditor);
	
	int addy=maingui->GetFontSizeY();
	
	if(nl)
	{	
		// Drummap
		if((y2=y+addy)<=frame->y2)
		{
			drummap=nl->AddButton(x,y,x2,y2,0,FX_DRUM_DRUMMAP_ID,0,"Select, Create/Delete Drum Map");
			y=y2+1;
		}

		if((y2=y+addy)<=frame->y2)
		{
			trackname=nl->AddString(x,y,x2,y2,FX_DRUM_TRACKNAME_ID,0,0,"Track Name");
			y=y2+1;
		}
		
		
		// MIDI Channesl
		if((y2=y+addy)<=frame->y2)
		{
			MIDIchannel=nl->AddButton(x,y,x2,y2,0,FX_DRUM_MIDICHANNEL_ID,0,Cxs[CXS_MIDICHANNEL]);
			y=y2+1;
		}
		
		// MIDI Out Port
		if((y2=y+addy)<=frame->y2)
		{
			MIDIoutput=nl->AddButton(x,y,x2,y2,0,FX_DRUM_MIDIOUT_ID,0,Cxs[CXS_MIDIOUTPUT]);
			y=y2+1;
		}	
		
		// MIDI Out Port
		if((y2=y+addy)<=frame->y2)
		{
			audiochannel=nl->AddButton(x,y,x2,y2,0,FX_DRUM_AUDIOOUT_ID,0,"Audio Channel");
			y=y2+1;
		}	
		
		// MIDI Out Port
		if((y2=y+addy)<=frame->y2)
		{
			MIDIkey=nl->AddButton(x,y,x2,y2,0,FX_DRUM_MIDIKEY_ID,0,"MIDI Key");
			y=y2+1;
		}	
		
		// MIDI Note Length
		if((y2=y+addy)<=frame->y2)
		{
			drumlength=nl->AddButton(x,y,x2,y2,0,FX_DRUM_MIDILENGTH_ID,0,"Note Length");
			y=y2+1;
		}	
		
		// mute
		if((y2=y+addy)<=frame->y2)
		{
			//mute=nl->AddCheckBox(x,y,x2,y2,FX_DRUM_TRACKMUTE_ID,"Mute Track","Mute Track");
			y=y2+1;
		}
		
		// Track Volume
		if((y2=y+addy)<=frame->y2)
		{
			double h=x2-x;
			
			h/=3;
			
			SliderCo horz;
			
			// pos
			horz.x=x;
			horz.y=y;
			horz.x2=x+(int)(h*2);
			horz.y2=y2;
			horz.horz=true;
			horz.page=10; // 10%
			
			horz.pos=rl_volumeslider=0;
			horz.from=-127;
			horz.to=127; // 10.000 = 100.00 %percent
			
			trackvolume=nl->AddSlider(&horz,FX_DRUM_TRACKVOLUME_ID,0);
			
			trackvolumebutton=nl->AddButton(horz.x2+1,y,x2,y2,FX_DRUM_TRACKVOLUMEBUTTON_ID);
			
			y=y2+1;
		}	
		
		ShowActiveTrack();
	}
#endif

}// if track


#endif