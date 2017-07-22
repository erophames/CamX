#include "drumeditorfx.h"
#include "songmain.h"
#include "object_song.h"
#include "languagefiles.h"
#include "semapores.h"
#include "drumeditorlist.h"

enum
{
	GADGETID_TRACKNAMENUMBER=GADGETID_EDITORBASE,
	GADGETID_TRACKNAMESTRING,
	GADGETID_MIDICHANNEL,
	GADGETID_MIDICHANNEL_NR,
	GADGETID_VELOCITY,
	GADGETID_VELOCITY_NR,
	GADGETID_KEY,
	GADGETID_KEY_NR,
	GADGETID_LENGTH,
	GADGETID_LENGTH_L,
	GADGETID_MIDIOUT,
	GADGETID_MIDIOUT_STRING
};

Edit_DrumFX::Edit_DrumFX(Edit_Drum *ar)
{
	editorid=EDITORTYPE_DRUMFX;
	InitForms(FORM_PLAIN1x1);

	autovscroll=true;
	isstatic=true;
	song=ar->WindowSong();
	editor=ar;
}

void Edit_DrumFX::Gadget(guiGadget *g)
{
	if(editor->focustrack)
	{
		switch(g->gadgetID)
		{
		case GADGETID_LENGTH:
		case GADGETID_LENGTH_L:
			{
				DeletePopUpMenu(true);

				if(popmenu)
				{
					class menu_dlen:public guiMenu
					{
					public:
						menu_dlen(Edit_Drum *e,Drumtrack *tr,int ix){editor=e;track=tr;index=ix;}

						void MenuFunction()
						{
							mainthreadcontrol->LockActiveSong();
							track->ticklength=quantlist[index];
							mainthreadcontrol->UnlockActiveSong();

							editor->PlayFocusTrack();

						} //

						Edit_Drum *editor;
						Drumtrack *track;
						int index;
					};

					popmenu->AddMenu(Cxs[CXS_LENGTH],0);
					popmenu->AddLine();

					for(int i=0;i<QUANTNUMBER;i++)
					{
						popmenu->AddFMenu(quantstr[i],new menu_dlen(editor,editor->focustrack,i),editor->focustrack->ticklength==quantlist[i]?true:false);
					}
				
					ShowPopMenu();
				}
			}
			break;

		case GADGETID_TRACKNAMESTRING:
			{
				editor->focustrack->SetName(g->string);

				if(editor->tracks)
				{
					editor->ShowTracks();
					editor->tracks->Blt();
				}

				if(editor->showlist==true && editor->drumlist && editor->drumlist->list)
				{
					editor->drumlist->ShowList();
					editor->drumlist->list->Blt();
				}
			}
			break;

		case GADGETID_MIDICHANNEL_NR:
			editor->focustrack->SetChannel(g->GetPos());
			editor->PlayFocusTrack();
			break;

		case GADGETID_VELOCITY_NR:
			editor->focustrack->SetVelocity(g->GetPos());
			break;

			/*
			case GADGETID_KEY:
			{
			editor->focustrack->key=g->index;
			}
			break;
			*/

		case GADGETID_KEY_NR:
			{
				editor->focustrack->SetKey(g->GetPos());
				editor->PlayFocusTrack();
			}
			break;



		}
	}
}

void Edit_DrumFX::RefreshRealtime_Slow()
{
	ShowTrackNumber(false);
	ShowTrackName(false);
	ShowTrackChannel(false);
	ShowTrackVelocity(false);
	ShowTrackLength(false);

	if(notetype!=editor->WindowSong()->notetype)
	{
		notetype=editor->WindowSong()->notetype;
		ShowTrackKey(true);		
	}
	else
		ShowTrackKey(false);

}

void Edit_DrumFX::Init()
{
	InitGadgets();
	notetype=editor->WindowSong()->notetype;
}

void Edit_DrumFX::InitGadgets()
{
	glist.SelectForm(0,0);

	int w=bitmap.GetTextWidth("* Velocity *");

	g_track=glist.AddButton(-1,-1,w,-1,GADGETID_TRACKNAMENUMBER);
	ShowTrackNumber(true);
	glist.AddLX();
	g_trackstring=glist.AddString(-1,-1,-1,-1,GADGETID_TRACKNAMESTRING,MODE_RIGHT,0,0);
	ShowTrackName(true);
	glist.Return();

	glist.AddButton(-1,-1,w,-1,"Channel:",GADGETID_MIDICHANNEL);
	glist.AddLX();
	trackchannel=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_MIDICHANNEL_NR,1,16,1,NUMBER_MIDICHANNEL,MODE_RIGHT);
	glist.Return();
	ShowTrackChannel(true);

	glist.AddButton(-1,-1,w,-1,"Velocity:",GADGETID_VELOCITY);
	glist.AddLX();
	trackvelocity=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_VELOCITY_NR,-127,127,0,NUMBER_INTEGER,MODE_RIGHT);
	glist.Return();
	ShowTrackVelocity(true);

	glist.AddButton(-1,-1,w,-1,"Note:",GADGETID_KEY);
	glist.AddLX();
	trackkey=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_KEY_NR,0,127,0,NUMBER_KEYS,MODE_RIGHT);
	glist.Return();
	ShowTrackKey(true);

	glist.AddButton(-1,-1,w,-1,Cxs[CXS_LENGTH],GADGETID_LENGTH,MODE_ADDDPOINT|MODE_MENU);
	glist.AddLX();
	tracklength=glist.AddButton(-1,-1,-1,-1,GADGETID_LENGTH_L,MODE_MENU|MODE_TEXTCENTER|MODE_RIGHT);
	glist.Return();
	ShowTrackLength(true);

	/*
	glist.AddButton(-1,-1,w,-1,"Ticks",GADGETID_LENGTHTICKS,MODE_ADDDPOINT);
	glist.AddLX();
	tracklength_ticks=glist.AddButton(-1,-1,-1,-1,GADGETID_LENGTH_L,MODE_MENU|MODE_TEXTCENTER|MODE_RIGHT);
	glist.Return();
	ShowTrackLength(true);
*/

	glist.AddButton(-1,-1,w,-1,"Output:",GADGETID_MIDIOUT);
	glist.AddLX();
	trackMIDIoutput=glist.AddButton(-1,-1,w,-1,GADGETID_MIDIOUT_STRING,MODE_RIGHT);
	glist.Return();
}

void Edit_DrumFX::ShowTrackName(bool force)
{
	if(g_trackstring)
	{
		if(editor->focustrack)
			g_trackstring->CheckString(editor->focustrack->GetName(),force);
		else
			g_trackstring->Disable();
	}
}

void Edit_DrumFX::ShowTrackNumber(bool force)
{
	if(g_track)
	{
		if(editor->focustrack)
		{
			int tn=editor->focustrack->map->GetIndexOfTrack(editor->focustrack);

			if(force==true || g_track->disabled==true || tracknr!=tn)
			{
				tracknr=tn;
				char h[NUMBERSTRINGLEN];

				g_track->ChangeButtonText(mainvar->ConvertIntToChar(tracknr+1,h));
			}
		}
		else
			g_track->Disable();

	}
}

void Edit_DrumFX::ShowTrackChannel(bool force)
{
	if(trackchannel)
	{
		if(editor->focustrack)
		{
			if(force==true || g_track->disabled==true || track_channel!=editor->focustrack->GetMIDIChannel())
			{
				track_channel=editor->focustrack->GetMIDIChannel();
				trackchannel->SetPos(track_channel+1);
			}
		}
		else
			trackchannel->Disable();

	}
}

void Edit_DrumFX::ShowTrackVelocity(bool force)
{
	if(trackvelocity)
	{
		if(editor->focustrack)
		{
			if(force==true || g_track->disabled==true || track_volume!=editor->focustrack->volume)
			{
				track_volume=editor->focustrack->volume;
				trackvelocity->SetPos(track_volume);
			}
		}
		else
			trackvelocity->Disable();

	}
}

void Edit_DrumFX::ShowTrackLength(bool force)
{
	if(tracklength)
	{
		if(editor->focustrack)
		{
			if(force==true || g_track->disabled==true || track_length!=editor->focustrack->ticklength)
			{
				track_length=editor->focustrack->ticklength;

				for(int i=0;i<QUANTNUMBER;i++)
				{
					if(quantlist[i]==track_length)
					{
						tracklength->ChangeButtonText(quantstr[i]);
						return;
					}
				}

				char h[NUMBERSTRINGLEN];

				tracklength->ChangeButtonText(mainvar->ConvertLongLongToChar(track_length,h));

				//trackkey->SetPos(track_key);
			}
		}
		else
			tracklength->Disable();

	}
}

void Edit_DrumFX::ShowTrackKey(bool force)
{
	if(trackkey)
	{
		if(editor->focustrack)
		{
			if(force==true || g_track->disabled==true || track_key!=editor->focustrack->key)
			{
				track_key=editor->focustrack->key;
				trackkey->SetPos(track_key);
			}
		}
		else
			trackkey->Disable();

	}
}


