#include "songmain.h"
#include "pianoeditor.h"
#include "gui.h"
#include "undo.h"
#include "camxgadgets.h"
#include "object_song.h"
#include "objectpattern.h"
#include "objectevent.h"
#include "mainhelpthread.h"
#include "editfunctions.h"
#include "undofunctions.h"
#include "object_track.h"
#include "editbuffer.h"
#include "MIDIhardware.h"
#include "audiohardware.h"
#include "settings.h"
#include "MIDIPattern.h"
#include "editortabs.h"

enum{
	MESSAGE_NOTE
};

enum{
	GADGETID_EDITNOTE_TIME=GADGETID_EDITORBASE,
	GADGETID_EDITNOTE_TIME_I,
	GADGETID_EDITNOTE_END_I,
	GADGETID_EDITNOTE_END,
	GADGETID_EDITNOTE_CHL,
	GADGETID_EDITNOTE_CHL_I,

	GADGETID_EDITNOTE_KEY,
	GADGETID_EDITNOTE_KEY_I,
	GADGETID_EDITNOTE_VELO,
	GADGETID_EDITNOTE_VELO_I,
	GADGETID_EDITNOTE_VELOOFF,
	GADGETID_EDITNOTE_VELOOFF_I,

	GADGETID_EDITNOTE_LENGTH,
	GADGETID_EDITNOTE_LENGTH_I,

	GADGETID_EDITNOTE_CHANNEL_NEWCHANNEL_I,
	GADGETID_EDITNOTE_CHANNEL_NEWCHANNEL,

	GADGETID_EDITNOTE_CHANNEL_NEWVELO_I,
	GADGETID_EDITNOTE_CHANNEL_NEWVELO,

	GADGETID_EDITNOTE_CHANNEL_NEWVELOOFF_I,
	GADGETID_EDITNOTE_CHANNEL_NEWVELOOFF,

	GADGETID_EDITNOTE_CHANNEL_NEWLENGTH_I,
	GADGETID_EDITNOTE_CHANNEL_NEWLENGTH,

	GADGETID_PLAYMOUSE,
	GADGETID_VELOMODE,
	GADGETID_DATA
};

enum
{
	GADGETID_STATUS=GADGETID_EDITORBASE,
	GADGETID_SET1,
	GADGETID_SET2,
	GADGETID_SET3,
	GADGETID_SET4,
	GADGETID_SET5,
	GADGETID_STATUSNAME,
	GADGETID_CHANNEL,
	GADGETID_CHANNELNR,
	GADGETID_CONTROL,
	GADGETID_CONTROLNAME

};

void Edit_Piano::DeleteNotes()
{
	TRACE ("DeleteNotes Undo = %d\n",addtolastundo);
	mainedit->DeleteEvents(&patternselection,addtolastundo);
	addtolastundo=true;
	//if(c)return true;
	//return false;
}

void Edit_Piano::DeleteNote()
{
	if(Edit_Piano_Note *e=FindNoteUnderMouse())
		mainedit->DeleteEvent(e->sevent->seqevent,addtolastundo);
}

Note *Edit_Piano::CreateNote()
{
	int key=FindKeyAtPosition(noteraster->GetMouseY());

	if(key!=-1 && insertpattern && insertpattern->mediatype==MEDIATYPE_MIDI)
	{
		OSTART pos=GetMousePosition(),end;
		int status=NOTEON;

		if(default_notelengthuseprev==true && mainsettings->lastsetnotelength!=-1)
			end=pos+mainsettings->lastsetnotelength;
		else
			end=pos+default_notelength;

		status|=(newchannel-1);

		// Find Drum Event in insertpattern
		Seq_Event *check=insertpattern->FindEventAtPosition(0,SEL_NOTEON,0);

		while(check && check->GetEventStart()<=pos)
		{
			if(check->status==status)
			{
				Note *n=(Note *)check;
				if(n->key==key && n->GetNoteEnd()>=pos)
					return 0;
			}

			check=check->NextEvent();
		}

#ifdef MEMPOOLS
		Note *note=mainpools->mempGetNote();
#else
		Note *note=new Note;
#endif

		if(note)
		{
			note->ostart=note->staticostart=pos;
			note->off.ostart=note->off.staticostart=end;

			note->status=status;
			note->key=key;
			note->velocity=newvelocity;
			note->velocityoff=newvelocityoff;

			editevent.song=WindowSong();
			editevent.pattern=(MIDIPattern *)insertpattern;
			editevent.seqevent=note;

			editevent.position=pos;
			editevent.endposition=end;

			editevent.doundo=true;
			editevent.addtolastundo=addtolastundo;
			editevent.playit=true;

			addtolastundo=true;

			mainedit->CreateNewMIDIEvent(&editevent);

			return note;
		}
	}

	addtolastundo=true;
	return 0;
}

void Edit_Piano::CreateGotoMenu()
{
	DeletePopUpMenu(true);
	AddStandardGotoMenu();

	if(popmenu && WindowSong()->GetFocusEvent())
	{
		popmenu->AddLine();
		popmenu->AddFMenu("Focus Event",new menu_gotoeventeditor(this,GOTO_FOCUS),"F");
	}
}

void Edit_Piano::CreateWindowMenu()
{
	if(NewWindowMenu())
	{
		CreateMenuList(menu);
		//AddEditMenu(menu);
	}
}

void Edit_Piano::CreateMenuList(guiMenu *menu)
{
	if(!menu)
		return;

	AddEventEditorMenu(menu);

	if(menu->functionsmenu)
	{
		class menu_setnotelength:public guiMenu
		{
		public:
			menu_setnotelength(Edit_Piano *piano){editor=piano;}

			void MenuFunction()
			{
				mainedit->SetNoteLength(editor,&editor->patternselection);	
			}

			Edit_Piano *editor;
		};

		menu->functionsmenu->AddFMenu(Cxs[CXS_SETLENGTHOFNOTES],new menu_setnotelength(this));
	}
}

guiMenu *Edit_Piano::CreateMenu()
{
	if(DeletePopUpMenu(true))
	{
		CreateMenuList(popmenu);	
	}

	return 0;
}

// Buttons,Slider ...
void Edit_Piano::Gadget(guiGadget *gadget)
{	
	if(!Editor_Gadget(gadget))return;

	Note *infonote=WindowSong()->GetFocusEvent() && WindowSong()->GetFocusEvent()->GetStatus()==NOTEON?(Note *)WindowSong()->GetFocusEvent():0;

	if(gadget)
	{
		switch(gadget->gadgetID)
		{
		case GADGETID_TOOLBOX_QUANTIZE:
			mainedit->QuantizeEventsMenu(this,&patternselection);
			break;

		case GADGETID_VELOMODE:
			editvelocity=editvelocity==true?false:true;
			ShowVeloMode();
			DrawDBBlit(noteraster,showwave==true?waveraster:0);
			break;

		case GADGETID_DATA:
			{
				showwave=showwave==true?false:true;
				FormYEnable(2,showwave);
			}
			break;

		case GADGETID_EDITNOTE_CHL:
			if(infonote)
			{
				infonote->SetChannel(gadget->GetPos());
				maingui->RefreshAllEditorsWithEvent(WindowSong(),infonote);
			}
			break;

		case GADGETID_EDITNOTE_KEY:
			if(infonote)
			{
				infonote->SetByte1(gadget->GetPos());
				maingui->RefreshAllEditorsWithEvent(WindowSong(),infonote);
			}
			break;

		case GADGETID_EDITNOTE_VELO:
			if(infonote)
			{
				infonote->SetByte2(gadget->GetPos());
				maingui->RefreshAllEditorsWithEvent(WindowSong(),infonote);
			}
			break;

		case GADGETID_EDITNOTE_VELOOFF:
			if(infonote)
			{
				infonote->SetByte3(gadget->GetPos());
				maingui->RefreshAllEditorsWithEvent(WindowSong(),WindowSong()->GetFocusEvent());
			}
			break;

		case GADGETID_EDITNOTE_CHANNEL_NEWCHANNEL:
			newchannel=gadget->GetPos();
			break;

		case GADGETID_EDITNOTE_CHANNEL_NEWVELO:
			newvelocity=gadget->GetPos();
			break;

		case GADGETID_EDITNOTE_CHANNEL_NEWVELOOFF:
			newvelocityoff=gadget->GetPos();
			break;

		case GADGETID_EDITNOTE_CHANNEL_NEWLENGTH:
			{
				default_notelength=gadget->GetPos();
			}
			break;


		case GADGETID_PLAYMOUSE:
			{
				playmouseovernotes=playmouseovernotes==true?false:true;
				mainsettings->playmouseover=playmouseovernotes;
			}
			break;


			/*
			case GADGETID_PIANOEDITOR_NOTECHANNEL:
			{
			DeletePopUpMenu(true);

			if(popmenu)
			{
			class menu_MIDIchl:public guiMenu
			{
			public:
			menu_MIDIchl(Edit_Piano *p,UBYTE c)
			{
			editor=p;
			channel=c;
			}

			void MenuFunction()
			{
			editor->default_channel=channel;
			editor->ShowNoteLengthButton();
			} //

			Edit_Piano *editor;
			UBYTE channel;
			};

			popmenu->AddMenu(Cxs[CXS_CHANNELOFNEWNOTES],0);
			popmenu->AddLine();

			for(int m=0;m<17;m++)
			popmenu->AddFMenu(MIDIchannels[m],new menu_MIDIchl(this,m),default_channel==m?true:false);

			ShowPopMenu();
			}
			}
			break;
			*/

			/*
			case GADGETID_PIANOEDITOR_NOTELENGTH:
			{
			DeletePopUpMenu(true);

			if(popmenu)
			{
			class menu_notelength:public guiMenu
			{
			public:
			menu_notelength(Edit_Piano *ed,OSTART t)
			{
			editor=ed;
			ticks=t;
			}

			void MenuFunction()
			{
			if(ticks==-1)
			{
			editor->default_notelengthuseprev=editor->default_notelengthuseprev==true?false:true;
			mainsettings->piano_default_notelengthuseprev=editor->default_notelengthuseprev;
			}
			else
			{
			mainsettings->lastsetnotelength=editor->default_notelength=ticks;
			}

			editor->ShowNoteLengthButton();
			editor->ShowCursor();
			} //

			Edit_Piano *editor;
			OSTART ticks;
			};

			popmenu->AddMenu(Cxs[CXS_NEWNOTELENGTH],0);
			popmenu->AddLine();

			popmenu->AddFMenu(Cxs[CXS_USEPREVIOUSNOTELENGTH],new menu_notelength(this,-1),default_notelengthuseprev);

			// Quantize
			for(int a=0;a<QUANTNUMBER;a++)
			popmenu->AddFMenu(quantstr[a],new menu_notelength(this,quantlist[a]),quantlist[a]==default_notelength?true:false);

			ShowPopMenu();
			}
			}
			break;
			*/

		case GADGETID_EDITORSLIDER_VERT: // Scroll V
			{
				keyobjects.InitWithSlider(vertgadget);
				ShowPianoHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_TRACKS|SHOWEVENTS_PIANOWAVETRACK);

				//SetStartKey(127-gadget->GetPos());
			}
			break;
		}
	}
}

/*
void Edit_Piano::ShowEvent(Seq_SelectionEvent *e,bool direct)
{

if(guibuffer)
{
Edit_Piano_Note *pn=(Edit_Piano_Note *)notes.GetRoot();

while(pn){
if(pn->note==e->event)pn->Draw();
pn=(Edit_Piano_Note *)pn->next;
}
}
}
*/


void Edit_PianoWave::SetSet(int s)
{
	if(set!=s)
	{
		set=s;
		mainsettings->defaultpianosettings=s;

		//	RefreshTracking();
		//	ShowFilter();

		for(int i=0;i<5;i++)
			if(g_set[i])
				g_set[i]->Toggle(i==set?true:false);
	}
}

void Edit_Piano_Note::Init(OSTART songposition)
{
	OSTART nstart=note->GetEventStart();

	if(nstart<editor->startposition)
	{
		flag|=Edit_Piano_Note::NOSTART;
		x=0;
	}
	else
		x=editor->timeline->ConvertTimeToX(nstart,editor->noteraster->GetX2());

	OSTART nend=note->GetNoteEnd();

	if(songposition!=-1 && sevent)
	{
		if(nstart<=songposition && nend>=songposition)
		{
			sevent->flag |= SEQSEL_REALTIMEACTIVATED;
		}
	}

	if(note==editor->newmousenote && editor->newmouselength>0)
	{
		nend=nstart+editor->newmouselength;
	}

	if(nend>=editor->endposition)
	{
		flag|=Edit_Piano_Note::NOEND;
		x2=editor->noteraster->GetX2();
	}
	else
		x2=editor->timeline->ConvertTimeToX(nend,editor->noteraster->GetX2());

	y=editor->keyposy[note->key];
	y2=editor->keyposy2[note->key];
}

void Edit_Piano_Note::Draw()
{
	eflag=note->flag;

	int colour,bordercolour;
	bool forcenocolour=false;

	if(sevent && (sevent->flag&SEQSEL_REALTIMEACTIVATED))  // sevent can be NULL !
	{
		colour=COLOUR_GREEN;
		forcenocolour=true;
	}
	else
		if(note==editor->WindowSong()->GetFocusEvent())
		{
			infonote=true;
			colour=note->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT;
		}
		else
			if(eflag&EVENTFLAG_MUTED)
				colour=COLOUR_RED;
			else
			{
				if(note->IsSelected()==true)
					colour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
				else
					colour=COLOUR_BACKGROUND;
			}

			if(eflag&EVENTFLAG_MOVEEVENT)
			{
				bordercolour=COLOUR_BORDER_OBJECTMOVING;
			}
			else
			{
				if(note->IsSelected()==true)
					bordercolour=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
				else
				{
					bordercolour=note->pattern==editor->insertpattern?COLOUR_BLACK:COLOUR_GREY_DARK;
				}
			}

			int px=x,py=y,px2=x2,py2=y2;

			if(x2>x+4)
			{
				// Rect ?

				if((!(flag&NOSTART)) && (!(flag&NOEND)))
					bitmap->guiDrawRect(px,py,px2,py2,bordercolour);
				else
				{
					bitmap->guiDrawLineY(py,px,px2,bordercolour);
					bitmap->guiDrawLineY(py2,px,px2);

					if(!(flag&NOSTART))
						bitmap->guiDrawLineX(px,py,py2);

					if(!(flag&NOEND))
						bitmap->guiDrawLineX(px2,py,py2);
				}

				if(flag&MOUSEOVERLEFT)
				{
					bitmap->guiDrawLineY(py,px,px2,COLOUR_RED);

					bitmap->guiDrawLineX(px,py,py2);
					bitmap->guiDrawLineX(px+1,py,py2);

					px++;
					//	bitmap->guiDrawLineY(py2,px,px2);

				}

				if(flag&MOUSEOVERRIGHT)
				{
					bitmap->guiDrawLineX(px2-1,py,py2,COLOUR_RED);
					bitmap->guiDrawLineX(px2,py,py2);
					bitmap->guiDrawLineY(py2,px,px2);
					px2--;
				}

				//if(note->flag&EVENTFLAG_MUTED)
				//	bitmap->guiDrawRect(px,py,px2,py2,note->pattern==editor->insertpattern?COLOUR_RED:COLOUR_RED_LIGHT);

				px+=1;
				px2-=1;
				py+=1;
				py2-=1;

				if(px2>px && py2>py)
				{
					rgb.InitColour(note);

					if(note->IsSelected()==false && rgb.showcolour==true && forcenocolour==false)
						bitmap->guiFillRect_RGB(px,py,px2,py2,rgb.rgb);
					else
						bitmap->guiFillRect(px,py,px2,py2,colour);

					int velo=editor->editvelocity==true?note->velocity:note->velocityoff;

					// Velocity ?
					if(px2>px+5 && py2>py+1)
					{
						double h=(px2-px)-2,h2;

						bool text;

						h2=velo;
						h2/=127;

						h*=h2;

						int hy;

						if(py2>py+maingui->GetFontSizeY())
						{
							text=true;
							hy=py+maingui->GetFontSizeY();
						}
						else
						{
							hy=py;
							text=false;
						}

						if(hy<py2)
						{
							int hx2;
							UBYTE r,g,b;
							maingui->colourtable.GetRGB(editor->editvelocity==true?COLOUR_BLUE:COLOUR_TOGGLEON,&r,&g,&b,127-velo);

							bitmap->guiFillRect_RGB(px+1,hy+1,hx2=px+1+(int)h,py2-1,RGB(r,g,b));
							if(hx2<px2-1)
								bitmap->guiFillRect(hx2+1,hy+1,px2-1,py2-1,COLOUR_BLACK_LIGHT);

							char nr[NUMBERSTRINGLEN];

							char *h=mainvar->ConvertIntToChar(velo,nr);

							int sizex=bitmap->GetTextWidth(h);

							if(sizex<x2-x)
							{
								bitmap->SetFont(&maingui->smallfont);
								bitmap->SetTextColour(COLOUR_WHITE);
								bitmap->guiDrawText(px+1,py+2*maingui->GetFontSizeY()+1,px2,h);
								bitmap->SetFont(&maingui->standardfont);
							}

						}

						// Piano Text
						if(text==true && note->pattern==editor->insertpattern)
						{
							char *t=maingui->ByteToKeyString(editor->WindowSong(),note->key);

							int sizex=bitmap->GetTextWidth(t);

							if(sizex<x2-x)
							{
								bitmap->SetTextColour(COLOUR_BLACK);
								bitmap->SetFont(&maingui->smallfont);
								bitmap->guiDrawText(px+1,py+maingui->GetFontSizeY()+1,px2,t);
								bitmap->SetFont(&maingui->standardfont);
							}
						}
					}

					if(eflag&OFLAG_UNDERSELECTION)
						bitmap->guiInvert(px,py,px2,py2);

					return;
				}

			}

			if(px2<=px)
			{
				bitmap->guiDrawLineX(px,py,py2,bordercolour);

				if(eflag&OFLAG_UNDERSELECTION)
					bitmap->guiInvert(px,py,px2,py2);

				return;
			}

			bitmap->guiFillRect(px,py,px2,py2,bordercolour);

			if(flag&MOUSEOVERLEFT)
			{
				bitmap->guiDrawLineY(py,px,px2,COLOUR_RED);

				bitmap->guiDrawLineX(px,py,py2);
				bitmap->guiDrawLineX(px+1,py,py2);


				//	bitmap->guiDrawLineY(py2,px,px2);

			}

			if(flag&MOUSEOVERRIGHT)
			{
				bitmap->guiDrawLineX(px2-1,py,py2,COLOUR_RED);
				bitmap->guiDrawLineX(px2,py,py2);
				bitmap->guiDrawLineY(py2,px,px2);
			}


			if(eflag&OFLAG_UNDERSELECTION)
				bitmap->guiInvert(px,py,px2,py2);



			/*
			if(note->flag&EVENTFLAG_MUTED)
			{
			guibuffer->guiDrawLine(px,py,px2,py2,COLOUR_RED);
			}
			*/

			//	if(direct==true)
			//		editor->BltGUIBuffer(x,y,x2,y2);
}

void Edit_Piano::RefreshEvents()
{
	patternselection.ClearFlags();
	//	ShowRaster();

	//noteraster->DrawGadgetBlt();
	patternselection.CopyStatus();

	Edit_Piano_Note *epn=(Edit_Piano_Note *)notes.GetRoot();

	while(epn)
	{
		epn->Draw();
		epn=(Edit_Piano_Note *)epn->next;
	}

	DrawDBSpriteBlit(noteraster);
}

void Edit_Piano::ShowEvents()
{	
	patternselection.ClearFlags();
	notes.DeleteAllO();

	ShowRaster();
	patternselection.CopyStatus();

	OSTART songposition=WindowSong()->status==Seq_Song::STATUS_STOP?-1:WindowSong()->GetSongPosition();

	Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

	// NoteOns ---------------------------------------------------------
	while(selevent && selevent->GetOStart()<=endposition)
	{	
		if(selevent->seqevent->GetStatus()==NOTEON)
		{
			Note *note=(Note *)selevent->seqevent;

			if(keyondisplay[note->key]==true &&
				(note->GetEventStart()>=startposition || note->GetNoteEnd()>startposition || (note->flag&EVENTFLAG_UNDEREDIT))
				)
			{
				if(Edit_Piano_Note *nnote=new Edit_Piano_Note(this,selevent,note,&noteraster->gbitmap))
				{
					notes.AddEndO(nnote);
					selevent->flag|=SEQSEL_ONDISPLAY;
					nnote->Init(songposition);
					nnote->Draw();
				}
			}
		}

		selevent=selevent->NextEvent();

	}//while event

	if(mousemode==EM_SIZENOTES_LEFT || mousemode==EM_SIZENOTES_RIGHT)
	{
		if(Note *n=(Note *)patternselection.FirstSelectionEvent(0,SEL_NOTEON|SEL_SELECTED))
		{
			Note sizenote;
			Edit_Piano_Note enote(this,0,&sizenote,&noteraster->gbitmap);

			n=(Note *)patternselection.FirstSelectionEvent(0,SEL_NOTEON|SEL_SELECTED);

			while(n)
			{
				OSTART start=n->GetEventStart();
				OSTART end=n->GetNoteEnd();

				if(mousemode==EM_SIZENOTES_LEFT)
				{
					start+=patternselection.movediff;
					if(start>=end)
						start=end;
				}
				else
				{
					end+=patternselection.movediff;

					if(end<=start)
						end=start+1;
				}

				if( ((start>=startposition && start<=endposition) || (start<startposition && end>=startposition)) &&
					n->key<=startkey &&
					n->key>=lastkey
					)
				{
					sizenote.flag=EVENTFLAG_MOVEEVENT;
					sizenote.pattern=n->pattern;
					sizenote.ostart=start;
					sizenote.off.ostart=end;
					sizenote.key=n->key;
					sizenote.velocity=n->velocity;
					sizenote.velocityoff=n->velocityoff;

					enote.Init(songposition);
					enote.Draw();
				}

				n=(Note *)patternselection.NextSelectionEvent(SEL_NOTEON|SEL_SELECTED);
			}// while n
		}
	}
	else
		if(mousemode==EM_MOVEOS)
		{
			if(Note *n=(Note *)patternselection.FirstSelectionEvent(0,SEL_NOTEON|SEL_SELECTED))
			{
				if(patternselection.movediff<-n->GetEventStart()) // FirstPosition
					patternselection.movediff=-n->GetEventStart();

				// Find Lowest/Highest Key		
				{
					int lowest,highest;

					highest=lowest=n->key;

					while(n)
					{
						if(n->key<lowest)
							lowest=n->key;

						if(n->key>highest)
							highest=n->key;

						n=(Note *)patternselection.NextSelectionEvent(SEL_NOTEON|SEL_SELECTED);
					}

					if(patternselection.moveobjects_vert<0)
					{
						if(lowest+patternselection.moveobjects_vert<0)
							patternselection.moveobjects_vert=-lowest;
					}
					else
					{
						if(highest+patternselection.moveobjects_vert>127)
							patternselection.moveobjects_vert=127-highest;
					}
				}

				Note movenote;
				Edit_Piano_Note enote(this,0,&movenote,&noteraster->gbitmap);

				n=(Note *)patternselection.FirstSelectionEvent(0,SEL_NOTEON|SEL_SELECTED);

				while(n)
				{
					OSTART start=n->GetEventStart()+patternselection.movediff;
					int key=n->key+patternselection.moveobjects_vert;

					// Insert Quantize ?
					{
						QuantizeEffect *qeff=n->pattern->GetQuantizer();

						if(qeff)
							start=qeff->Quantize(start);
					}

					OSTART end=start+n->GetNoteLength();

					if(start>=startposition &&
						start<=endposition &&
						end>startposition &&
						keyondisplay[key]==true
						)
					{
						movenote.flag=EVENTFLAG_MOVEEVENT;
						movenote.pattern=n->pattern;
						movenote.ostart=start;
						movenote.off.ostart=end;
						movenote.key=key;

						TRACE ("Key %d %d\n",key,patternselection.moveobjects_vert);

						movenote.velocity=n->velocity;
						movenote.velocityoff=n->velocityoff;

						enote.Init(songposition);
						enote.Draw();
					}

					n=(Note *)patternselection.NextSelectionEvent(SEL_NOTEON|SEL_SELECTED);
				}// while n
			}
		}

		//ShowPianoWave(0);
}

void Edit_Piano::ClearCursor()
{
	//ClearSprite(&cursorsprite);
	//RemoveSprite(&cursorsprite);
}

void Edit_Piano::SetCursorToMousePosition()
{
	/*
	if(frame_notes.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
	{
	int key=FindKeyAtPosition(GetMouseY());

	if(cursor.key!=key || cursor.ostart!=mouseposition)
	{
	cursor.ostart=mouseposition;
	cursor.key=key;

	ShowCursor();
	}
	}*/
}

int Edit_Piano::FindKeyAtKeyMousePosition()
{
	int mx=keys->GetMouseX(),my=keys->GetMouseY();

	// 1. Black, 2. White

	for(int b=0;b<2;b++)
		for(int i=startkey;i>=lastkey;i--)
		{
			if(keyblack[i]==(b==0)?true:false)
			{
				//	TRACE ("i=%d MX %d MY %d ---> %d %d %d %d \n",i,mx,my,keyposx[i],keyposx2[i],keyposwhitey[i],keyposwhitey2[i]);
				if(keyposx[i]<=mx && keyposx2[i]>=mx && keyposwhitey[i]<=my && keyposwhitey2[i]>=my)
				{
					//PlayKeyNote(true,i);
					return i;
				}
			}

		}

		return -1;
}

void Edit_Piano::MouseClickInKeys(bool leftmouse)
{
	int key=FindKeyAtKeyMousePosition();

	if(key!=-1)
		PlayKeyNote(true,key);
}

void Edit_Piano::MouseDoubleClickInKeys(bool leftmouse)
{
	if(leftmouse==true)
	{
		int key=FindKeyAtKeyMousePosition();

		if(key!=-1)
		{
			//PlayKeyNote(key);

			if(maingui->GetShiftKey()==true)
			{
				patternselection.SelectAllEvents(true,NOTEON,key,true);
			}
		}
	}
}

void Edit_Piano::MouseDoubleClickInNotes(bool leftmouse)
{
	if(leftmouse==true)
	{
		Edit_Piano_Note *e=FindNoteUnderMouse();

		if((!e) || maingui->GetShiftKey()==true)
		{
			DoubleClickInEditArea();
			return;
		}

		Seq_Event *found=e->sevent->seqevent;

		Seq_SelectionEvent *es=e->sevent;

		while(es)
		{
			if(es->seqevent->GetStatus()==found->GetStatus() && es->seqevent->GetByte1()==found->GetByte1())
			{
				es->seqevent->flag|=OFLAG_SELECTED;
			}

			es=es->NextEvent();
		}
	}
}

void Edit_Piano::MouseUpInKeys(bool leftmouse)
{
	if(leftmouse==true)
	{
		lastplaykey=-1;
	}
}

void Edit_Piano::ShowDefaultNoteLength()
{
	if(newnote_len && default_notelength>=0)
		newnote_len->SetLength(WindowSong()->GetSongPosition(),default_notelength);
}

void Edit_Piano::ShowFocusEvent()
{
	Note *infonote=WindowSong()->GetFocusEvent() && WindowSong()->GetFocusEvent()->GetStatus()==NOTEON?(Note *)WindowSong()->GetFocusEvent():0;

	if(infonote)
	{
		focus_start=infonote->GetEventStart();
		focus_end=infonote->GetNoteEnd();
		focus_channel=infonote->GetChannel();
		focus_key=infonote->key;
		focus_velo=infonote->velocity;
		focus_velooff=infonote->velocityoff;
		focus_length=infonote->GetNoteLength();

		if(infonote_time)
			infonote_time->SetTime(focus_start);

		if(infonote_end)
			infonote_end->SetTime(focus_end);

		if(infonote_chl)
			infonote_chl->SetPos(focus_channel+1);

		if(infonote_key)
			infonote_key->SetPos(focus_key);

		if(infonote_velo)
			infonote_velo->SetPos(focus_velo);

		if(infonote_velooff)
			infonote_velooff->SetPos(focus_velooff);

		if(infonote_length)
			infonote_length->SetLength(focus_start,focus_length);
	}
	else
	{
		glist.Disable(infonote_time);
		glist.Disable(infonote_end);
		glist.Disable(infonote_chl);
		glist.Disable(infonote_key);
		glist.Disable(infonote_velo);
		glist.Disable(infonote_velooff);
		glist.Disable(infonote_length);
	}
}

void Edit_Piano::MouseReleaseInNotes(bool leftmouse)
{
	switch(mousemode)
	{
	case EM_EDIT:
		{
			ReleaseEdit();
		}
		break;

	case EM_CREATE:
		{
			newmousenote=0;
			newmouselength=-1;
			ResetMouseMode(EM_RESET); // default mode
			return;
		}
		break;

	case EM_SIZENEWNOTE:
		{
			if(newmousenote && newmouselength>0 && newmouselength!=newmousenote->GetNoteLength())
			{
				mainsettings->lastsetnotelength=newmouselength;
				mainedit->SizeNote(WindowSong(),newmousenote,newmouselength,false);
			}

			newmousenote=0;
			newmouselength=-1;
			ResetMouseMode(EM_RESET); // default mode
			return;
		}
		break;

	case EM_SELECTOBJECTS:
		{	
			SelectAllEvents();
		}
		break;

	case EM_SIZENOTES_LEFT:
	case EM_SIZENOTES_RIGHT:
		{
			bool startorend=mousemode==EM_SIZENOTES_LEFT?true:false;

			EditCancel(true); // Dont Change Note Length

			if(patternselection.movediff)
				mainedit->SizeSelectedNotesInPatternList(WindowSong(),&patternselection,patternselection.movediff,flag,startorend);
		}
		break;

	case EM_MOVEOS:
		{
			EditCancel();

			MoveO mo;

			mo.song=WindowSong();
			mo.sellist=&patternselection;
			mo.diff=patternselection.movediff;
			mo.index=patternselection.moveobjects_vert;
			mo.flag=GetMouseQuantizeFlag();
			mo.filter=SEL_NOTEON;

			if(maingui->GetCtrlKey()==true)
				mainedit->CopySelectedEventsInPatternList(&mo);
			else
				mainedit->MoveSelectedEventsInPatternList(&mo);
		}
		break;
	}

	reset_oldnotelength=-1;
	noteraster->EndEdit();
	ResetMouseMode();
}

void Edit_Piano::MouseMoveInData(bool leftmouse)
{
	if(CheckMouseMovePosition(waveraster)==true)
		return;
}

void Edit_Piano::MouseMoveInNotes(bool leftmouse)
{
	if(CheckMouseMovePosition(noteraster)==true)
		return;

	PlayMouseMoveKey();

	switch(mousemode)
	{
	case EM_EDIT: // Velocity
		if(leftmouse==true)
		{
			if(noteraster->InitDeltaY())
			{
				EditSelectedEventsDelta_Tab(editvelocity==true?TAB_BYTE2:TAB_BYTE3);
			}
		}
		break;

	case EM_SIZENEWNOTE:
		{
			if(newmousenote)
			{
				InitMousePosition();

				if(GetMousePosition()>newmousenote->GetEventStart())
				{
					OSTART o_newmouselength=newmouselength;

					newmouselength=GetMousePosition()-newmousenote->GetEventStart();

					if(newmouselength!=o_newmouselength)
					{
						default_notelength=newmouselength;
						ShowDefaultNoteLength();

						noteraster->DrawGadgetBlt();
					}
				}
			}
		}
		break;

	case EM_SELECTOBJECTS:
		{
			ShowCycleAndPositions(editarea);
			DrawDBSpriteBlit(editarea);

			maingui->OpenPRepairSelection(&patternselection);

			Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

			// NoteOns ---------------------------------------------------------
			while(selevent && selevent->GetOStart()<=endposition)
			{	
				if(selevent->seqevent->GetStatus()==NOTEON)
				{
					Note *note=(Note *)selevent->seqevent;

					if( (mainvar->CheckIfInPosition(msposition,msendposition,note->GetEventStart())==true || mainvar->CheckIfInPosition(msposition,msendposition,note->GetNoteEnd())==true) &&
						mainvar->CheckIfInIndex(mskey,mekey,note->key)==true)
						note->PRepairSelection();
				}

				selevent=selevent->NextEvent();

			}//while event

			maingui->ClosePRepairSelection(&patternselection);
		}
		break;

	case EM_MOVEOS:
		{
			ShowMoveNotesSprites();
		}
		break;

	case EM_SIZENOTES_LEFT:
	case EM_SIZENOTES_RIGHT:
		{
			ShowSizeNotesSprites();
		}
		break;

	case EM_CREATE:
		{
			InitMousePosition();

			if(maingui->GetShiftKey()==true && maingui->GetLeftMouseButton()==true)
				CreateNote();
		}
		break;

	case EM_DELETE:
		if(maingui->GetLeftMouseButton()==true)
			DeleteNote();
		break;

	default:
		{
			/*
			// Change Size
			Edit_Piano_Note *sel=(Edit_Piano_Note *)notes.GetRoot();

			while(sel)
			{
			if(sel->y<=GetMouseY() && sel->y2>=GetMouseY() && sel->x<=GetMouseX() && sel->x+6>=GetMouseX())
			{
			SetMouseCursor(CURSOR_LEFT);
			mouseoverchangelength_left=true;
			break;
			}

			if(sel->y<=GetMouseY() && sel->y2>=GetMouseY() && sel->x2>GetMouseX() && sel->x2-6<=GetMouseX())
			{
			SetMouseCursor(CURSOR_LEFT);
			mouseoverchangelength_right=true;
			break;
			}

			sel=(Edit_Piano_Note *)sel->next;
			}

			if(!sel)
			{
			if(left_mousekey==MOUSEKEY_DOWN)
			{
			//if(frame_notes.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
			{
			//	SetMouseMode(EM_SELECTOBJECTS);
			}
			}
			}
			*/

		}
		break;
	}
}

void Edit_Piano::DeltaY(guiGadget *g)
{
	switch(mousemode)
	{
	case EM_EDIT:
		{
			//	EditSelectedEventsDelta_Tab(notes);
		}break;
	}
}

void Edit_Piano::MouseClickInNotes(bool leftmouse)
{
	if(leftmouse==false)
	{
		if(EditCancel()==true)
			return;
	}
	else
	{
		if(CheckMouseClickInEditArea(noteraster)==true) // Left Mouse
		{
			return;
		}
	}

	Edit_Piano_Note *e=FindNoteUnderMouse();
	Note *note=e?(Note *)e->note:0;

	if(mouseoverchangelength_left==true || mouseoverchangelength_right==true)
	{
		if(mouseoverchangelength_left==true)
		{
			if(note)
			{
				patternselection.SelectEvent(note,true);
				SetMouseMode(EM_SIZENOTES_LEFT,note->GetEventStart());
			}
			else
				SetMouseMode(EM_SIZENOTES_LEFT,-1);

			ShowSizeNotesSprites();
		}
		else
			if(mouseoverchangelength_right==true)
			{
				if(note)
				{
					patternselection.SelectEvent(note,true);
					SetMouseMode(EM_SIZENOTES_RIGHT,note->GetNoteEnd());
				}
				else
					SetMouseMode(EM_SIZENOTES_RIGHT,-1);

				ShowSizeNotesSprites();
			}

			return;
	}

	InitMousePosition();

	switch(mousemode) // -> Select Lasso ?
	{
	case EM_EDIT:
	case EM_SELECT:
	case EM_CUT:
	case EM_DELETE:
		{
			if(leftmouse==true)
			{
				if((!e) && maingui->GetCtrlKey()==false)
					patternselection.SelectAllEvents(false);

				if((!e) || maingui->GetShiftKey()==true)
				{
					SetMouseMode(EM_SELECTOBJECTS,-1);
					return;
				}
			}

			if((!e) && mousemode!=EM_DELETE)
				return;
		}
		break;
	}

	// Note clicked (e!=0)
	switch(mousemode)
	{
	case EM_EDIT:
		{
			if(leftmouse==true && note)
			{
				WindowSong()->SetFocusEvent(note);
				patternselection.SelectEvent(note,true);
				RefreshEvents();

				noteraster->SetEditSteps(1);
				noteraster->SetStartMouseY();

				OpenEditSelectedEvents(editvelocity==true?TAB_BYTE2:TAB_BYTE3,note);
			}
		}
		break;

	case EM_CREATE:
		if(leftmouse==true)
		{
			newmouselength=-1;
			newmousenote=CreateNote();

			if(newmousenote && maingui->GetShiftKey()==false)
			{
				newmouselength=newmousenote->GetNoteLength();
				SetMouseMode(EM_SIZENEWNOTE,-1);
			}
		}
		else
		{
			DeleteNote();
		}
		break;

	case EM_CUT:
		if(leftmouse==true && note)
		{
			mainedit->CutNote(note,GetMousePosition());
		}
		break;

	case EM_DELETE:
		if(leftmouse==true)
		{
			addtolastundo=false;
			FindAndDeleteNotes(note);
		}
		break;

	default:
		if(leftmouse==true && note)
		{
			WindowSong()->SetFocusEvent(note);
			patternselection.SelectEvent(note,true);
			RefreshEvents();

			//SetEditorMode(EM_SELECTOBJECTSSTART);
			mainhelpthread->AddMessage(MOUSEBUTTONDOWNMS,this,MESSAGE_CHECKMOUSEBUTTON,MESSAGE_NOTE,note);
		}
		break;
	}
}

void Edit_Piano::MouseClickInData(bool leftmouse)
{
	if(leftmouse==false)
	{
		if(EditCancel()==true)
			return;
	}
	else
	{
		if(CheckMouseClickInEditArea(waveraster)==true) // Left Mouse
		{
			return;
		}
	}
}

void Edit_Piano::MouseReleaseInData(bool leftmouse)
{
	ResetMouseMode();
}

void Edit_Piano::PlayCursor()
{
	if(insertpattern && insertpattern->mediatype==MEDIATYPE_MIDI)
	{
		Note note;
		int chl=((MIDIPattern *)insertpattern)->GetDominantMIDIChannel();

		if(chl==-1)
		{
			chl=((MIDIPattern *)insertpattern)->track->GetFX()->GetChannel(); // dummy

			if(chl)
				chl=chl-1;	
		}

		note.status=NOTEON|chl;
		note.key=cursor.key;
		note.velocity=127;
		note.velocityoff=0;

		note.ostart=0;
		note.off.ostart=default_notelength;

		insertpattern->track->SendOutEvent_User((MIDIPattern *)insertpattern,&note,true);
	}
}

void Edit_Piano::ShowCursor()
{
	return;

	/*
	ClearCursor();

	if(timeline && frame_notes.ondisplay==true)
	{
	if(cursor.ostart>=startposition && 
	cursor.ostart<endposition &&
	keyondisplay[cursor.key]==true
	)
	{
	//	MessageBeep(-1);
	OSTART end=cursor.ostart+default_notelength;

	cursorsprite.x=timeline->ConvertTimeToX(cursor.ostart,frame_notes.x2);

	if(timeline->CheckIfInHeader(end)==true)
	cursorsprite.x2=timeline->ConvertTimeToX(end,frame_notes.x2);
	else
	cursorsprite.x2=frame_notes.x2;

	cursorsprite.y=keyposy[cursor.key];
	cursorsprite.y2=keyposy2[cursor.key];
	cursorsprite.colour=COLOUR_RED;
	cursorsprite.type=guiSprite::SPRITETYPE_RECT;

	AddSprite(&cursorsprite);
	ShowAllSprites();
	}
	}
	*/
}

void  Edit_Piano::ShowOverviewVertPosition(int *y,int *y2)
{
	*y=0;
	*y2=overview->GetY2();

	if(keyobjects.height==0)
		return;

	double h=keyobjects.height;
	double h2=keyobjects.starty;
	double height=overview->GetY2();

	h2/=h;
	if(h==0)
		return;

	h2*=height;

	*y=(int)h2;

	h2=keyobjects.starty+keyobjects.guiheight;
	h2/=h;
	h2*=height;

	*y2=(int)h2;
}

void Edit_Piano::ShowOverview()
{
	if(!overview)return;

	guiBitmap *bitmap=&overview->gbitmap;
	bitmap->guiFillRect(COLOUR_OVERVIEW_BACKGROUND);

	OSTART slen=overviewlenght=WindowSong()->GetSongLength_Ticks();

	if(!slen)return;

	double hx=(double)overview->GetX2(),hy=(double)overview->GetY2();

	bitmap->SetAPen(COLOUR_OVERVIEWOBJECT);

	//Draw Notes
	Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

	// NoteOns ---------------------------------------------------------
	while(selevent && selevent->GetOStart()<=slen)
	{	
		if(selevent->seqevent->GetStatus()==NOTEON)
		{
			Note *note=(Note *)selevent->seqevent;

			// x+x2
			double x=(double)note->GetEventStart(),x2;
			x/=slen;
			x*=hx;

			if(note->GetNoteEnd()>slen)
				x2=overview->GetX2();
			else
			{
				x2=(double)note->GetNoteEnd();
				x2/=slen;
				x2*=hx;
			}

			double y=127-note->key;
			y/=127;
			y*=hy;

			if(x2>x)
				bitmap->guiDrawLineY((int)y,(int)x,(int)x2);
			else
				bitmap->guiDrawPixel((int)x,(int)y,COLOUR_OVERVIEWOBJECT);
		}

		selevent=selevent->NextEvent();

	}//while event
}

void Edit_Piano::ShowKeys()
{
	if(!keys)return;

	double a;
	int blacklenx,keyh,octave; //lasty;
	bool showkeytext;

	// Reset Piano Display
	ResetKeysOnDisplay();

	blacklenx=keys->GetX2();
	blacklenx/=3;
	blacklenx*=2;

	guiBitmap *bitmap=&keys->gbitmap;

	bitmap->guiFillRect(COLOUR_PIANOKEY);
	bitmap->guiDrawRect(0,0,bitmap->GetX2(),bitmap->GetY2(),COLOUR_GREY);

	// 0 = c-2 - c8
	RefreshActiveKeys(false);

	numberofkeys=keys->GetHeight()/zoomy;

	if(zoomvert==true)
		keyobjects.BufferYPos();

	keyobjects.DeleteAllO(keys);
	keyobjects.SetHeight(128*zoomy);
	keyobjects.CalcStartYOff(zoomy,numberofkeys,0,127);

	keyobjects.EndBuild();

	if(zoomvert==true)
		keyobjects.RecalcYPos();

	ShowSlider();

	// show black&white keys
	for(int i=0;i<2;i++)
	{
		//0=white
		//1=black

		bool forcekeytext=true;
		int lastwhitekey=-1;

		keyobjects.InitYStart(zoomy);

		int key=startkey=127-keyobjects.startobjectint;
		//	TRACE ("StartInit Key %d SY %d IY %d SOBInt %d\n",key,keyobjects.starty,keyobjects.inity,keyobjects.startobjectint);

		while(keyobjects.GetInitY()<keys->GetY2() && key>=0)
		{		
			lastkey=key;

			keyposy[key]=keyobjects.GetInitY(); // y
			keyposy2[key]=keyobjects.AddInitY(zoomy)-1;

			//		TRACE ("Start Key %d Key %d Y=%d Y2=%d\n",startkey,key,keyposy[key],keyposy2[key]);

			octave=key/12;
			keyh=key-(12*octave);

			//keyactive[key]=false;
			keyondisplay[key]=true;

			showkeytext=forcekeytext;

			bool show=false;

			switch(keyh)
			{
			case 1:
			case 3:
			case 6:
			case 8:
			case 10:
				if(i==1)
				{
					show=true;
					// black
					keyblack[key]=true;

					//	keyposy[key]=y;

					bitmap->guiFillRect(
						keyposx[key]=0,
						keyposwhitey[key]=keyposy[key],
						keyposx2[key]=blacklenx,
						keyposwhitey2[key]=keyposy2[key],
						COLOUR_BLACK);

					bitmap->guiDrawLineY(keyposy[key],keyposx[key],keyposx2[key],COLOUR_GREY_DARK);

					bitmap->guiDrawLine(
						keyposx[key],
						keyposy[key],
						keyposx[key],
						keyposy2[key],
						COLOUR_GREY_DARK
						);

					bitmap->guiDrawLine(
						keyposx[key]+1,
						keyposy[key],
						keyposx[key]+1,
						keyposy2[key],
						COLOUR_GREY_DARK
						);

					bitmap->guiDrawLine(
						keyposx2[key],
						keyposy[key],
						keyposx2[key],
						keyposy2[key],
						COLOUR_GREY
						);

					bitmap->guiDrawLineY(keyposy2[key],keyposx[key],keyposx2[key],COLOUR_GREY);

					if(keyactive[key]==true)
					{
						bitmap->guiFillRect(
							keyposx[key]+2,
							keyposwhitey[key]+2,
							keyposx2[key]-2,
							keyposwhitey2[key]-2,
							COLOUR_GREY
							);
					}
				}
				break;

			case 2:
			case 7:
			case 9:
			case 0:
			case 5:
				if(i==0)
				{
					show=true;

					switch(keyh)
					{
					case 2:
						a=((double)zoomy)*0.33;
						break;

					case 7:
						a=((double)zoomy)*0.45;
						break;

					case 9:
						a=((double)zoomy)*0.25;
						break;

					case 0:
						{
							a=((double)zoomy)*0.62;
							showkeytext=true;
							keyisc[key]=true;
						}
						break;

					case 5:
						a=((double)zoomy)*0.70;
						break;
					}

					a=((double)keyposy[key])-a;

					keyposx[key]=0;
					keyposx2[key]=keys->GetX2();

					if(a<0)a=0;

					bitmap->guiDrawLineY(keyposwhitey[key]=(int)a,keyposx[key],keyposx2[key],COLOUR_GREY);

					if(lastwhitekey!=-1)
					{
						keyposwhitey2[lastwhitekey]=keyposwhitey[key]-1;

						if(keyactive[lastwhitekey]==true)
						{
							bitmap->guiFillRect(
								keyposx[lastwhitekey]+2,
								keyposwhitey[lastwhitekey]+2,
								keyposx2[lastwhitekey]-2,
								keyposwhitey2[lastwhitekey]-2,
								COLOUR_GREY
								);
						}
					}

					lastwhitekey=key;

					keyblack[key]=false;

					if(key==0)
					{
						bitmap->guiDrawLine(
							keyposy2[key],
							keyposx[key],
							keyposx2[key],

							COLOUR_BLACK);

						bitmap->guiFillRect(keyposx[key],keyposy2[key]+1,keyposx2[key],keys->GetY2(),COLOUR_UNUSED);
					}
				}
				break;

			case 4:
			case 11:
				if(i==0)
				{
					show=true;
					bitmap->guiDrawLineY(keyposwhitey[key]=keyposy[key],keyposx[key]=0,keyposx2[key]=keys->GetX2(),COLOUR_BLACK_LIGHT);
					keyblack[key]=false;

					if(lastwhitekey!=-1)
					{
						keyposwhitey2[lastwhitekey]=keyposwhitey[key]-1;

						if(keyactive[lastwhitekey]==true)
						{
							bitmap->guiFillRect(
								keyposx[lastwhitekey]+2,
								keyposwhitey[lastwhitekey]+2,
								keyposx2[lastwhitekey]-2,
								keyposwhitey2[lastwhitekey]-2,
								COLOUR_GREY
								);
						}
					}

					lastwhitekey=key;
				}
				break;
			}

			if(show==true)
			{
				if(showkeytext==true && keyblack[key]==false)
				{
					bitmap->guiDrawText(bitmap->width-40,keyposy[key]+maingui->GetFontSizeY(),bitmap->width,maingui->ByteToKeyString(WindowSong(),key));
					showkeytext=false;
					forcekeytext=false;
				}
			}

			//y=y2;
			key--;
		}

		if(i==0)
		{
			if(lastwhitekey!=-1)
			{
				keyposwhitey2[lastwhitekey]=keyposy2[lastwhitekey];

				if(keyactive[lastwhitekey]==true)
				{
					bitmap->guiFillRect(
						keyposx[lastwhitekey]+2,
						keyposwhitey[lastwhitekey]+2,
						keyposx2[lastwhitekey]-2,
						keyposwhitey2[lastwhitekey]-2,
						COLOUR_GREY
						);
				}
			}
		}
	}
}

void Edit_Piano::ShowRaster()
{
	if(!noteraster)return;

	guiBitmap *bitmap=&noteraster->gbitmap;

	int ly=-1;
	bool lastkeywhite=false;
	//	guiTimeLinePos *pos=timeline->FirstPosition();

	// w/b raster
	for(int k=startkey;k>=lastkey;k--)
	{
		int c,dly2;

		dly2=ly=keyposy2[k];

		if(keyblack[k]==true)
		{
			c=COLOUR_PIANO_BLACK_BACKGROUND;
			lastkeywhite=false;
		}
		else
		{
			int octave=k/12;
			int keyh=k-(12*octave);

			if(keyh==0)
			{
				c=COLOUR_PIANO_WHITE_BACKGROUND_C;

				bitmap->guiDrawLineYX0(dly2,noteraster->GetX2(),COLOUR_GREY_DARK);

				dly2--;
			}
			else
			{
				if(keyh==5)
					c=COLOUR_PIANO_WHITE_BACKGROUND_F;
				else
					c=COLOUR_PIANO_WHITE_BACKGROUND;
			}

			lastkeywhite=true;
		}

		bitmap->guiFillRectX0(keyposy[k],noteraster->GetX2(),dly2,c);

	}

	if(ly==-1 || ly+1<noteraster->GetY2())
		bitmap->guiFillRectX0(ly+1,bitmap->GetX2(),bitmap->GetY2(),COLOUR_UNUSED);


	/*
	// Marker ?
	if(frame_notes.y+4<frame_notes.y2)
	{
	Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();

	while(m && m->GetMarkerStart()<=endposition)
	{
	int mx=-1,mx2=-1;

	if(m->GetMarkerStart()>=startposition)
	{
	mx=timeline->ConvertTimeToX(m->GetMarkerStart());

	if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
	{
	if(m->GetMarkerEnd()>=endposition)
	mx2=timeline->x2;
	else
	mx2=timeline->ConvertTimeToX(m->GetMarkerEnd());
	}
	}
	else // Start <<....|
	{
	if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
	{
	if(m->GetMarkerEnd()>startposition)
	{
	mx=timeline->x;

	if(m->GetMarkerEnd()>=endposition)
	mx2=timeline->x2;
	else
	mx2=timeline->ConvertTimeToX(m->GetMarkerEnd());
	}
	}
	}

	if(mx!=-1 && (mx2!=-1 || m->flag==Seq_Marker::MARKERTYPE_SINGLE))
	{
	if(m->markertype==Seq_Marker::MARKERTYPE_SINGLE)
	{
	bitmap->guiDrawLine(mx,0,mx,noteraster->GetY2(),COLOUR_BLACK);

	if(m->colour.showcolour==true)
	{
	for(int hx=mx+1,i=4;i>0;hx+=2,i--)
	{
	if(hx<timeline->x2)
	bitmap->guiDrawLine_RGB(hx,1,hx,noteraster->GetY2()-1,m->colour.rgb);
	}
	}
	else
	{
	for(int hx=mx+1,i=4;i>0;hx+=2,i--)
	{
	if(hx<timeline->x2)
	bitmap->guiDrawLine(hx,1,hx,noteraster->GetY2()-1,COLOUR_BLACK);
	}
	}
	}
	else
	{
	if(m->colour.showcolour==true)
	{
	bitmap->guiFillRect_RGB
	(
	mx,0,
	mx2,noteraster->GetY2(),
	m->colour.rgb);
	}
	else
	{
	bitmap->guiFillRect_RGB
	(
	mx,0,
	mx2,noteraster->GetY2(),
	COLOUR_BLUE_LIGHT);
	}
	}
	}

	m=m->NextMarker();
	}
	}
	*/

	timeline->DrawPositionRaster(bitmap);
}

void Edit_Piano::ShowMenu()
{
	ShowEditorMenu();
}

void Edit_Piano::ShowWaveEvents()
{
	if(!waveraster)
		return;

	guiBitmap *bitmap=&waveraster->gbitmap;

	bitmap->guiFillRect(COLOUR_BACKGROUNDEDITOR_GFX);

	timeline->DrawPositionRaster(bitmap);

	Seq_SelectionEvent *selevent=patternselection.FirstMixEvent(startposition);

	//TRACE ("se %d\n",selevent->ostart);
	//TRACE ("ee %d\n",selevent->event->GetEventStart());

	PianoSettings *settings=wave->GetSet();

	// NoteOns ---------------------------------------------------------
	while(selevent && selevent->GetOStart()<=endposition)
	{	
		if(selevent->seqevent->GetStatus()==settings->status)
		{
			switch(settings->status)
			{
			case NOTEON:
				{
					Note *note=(Note *)selevent->seqevent;

					if(keyondisplay[note->key]==true)
					{

						// Velocity
						int x=timeline->ConvertTimeToX(selevent->seqevent->GetEventStart());

						double h=note->velocity;

						h/=127;

						double h2=bitmap->GetY2();
						h2*=h;

						bitmap->guiDrawLine(x,h2,x,bitmap->GetY2(),COLOUR_BLUE);
					}
				}
				break;

			case NOTEOFF:

				break;

			case CONTROLCHANGE:

				break;

			case PROGRAMCHANGE:

				break;

			case POLYPRESSURE:

				break;

			case CHANNELPRESSURE:

				break;

			case PITCHBEND:

				break;
			}
		}

		selevent=selevent->NextEvent();

	}//while event
}

#ifdef OLDIE
void Edit_Piano::ShowPianoWaveTrack()
{
	if(!wavetrack)
		return;

	guiBitmap *bitmap=&wavetrack->gbitmap;

	bitmap->guiFillRect(COLOUR_BACKGROUND);

	/*
	if(frame_keys.ondisplay==true)
	{
	frame_wavetrack.y=frame_keys.y2+1;	
	frame_wavetrack.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ);

	guiobjects.RemoveOs(OBJECTID_TRACKTYPE);	
	guiobjects.RemoveOs(OBJECTID_TRACKCHANNEL);	
	guiobjects.RemoveOs(OBJECTID_TRACKBYTE1);

	if(frame_wavetrack.ondisplay==true && guibuffer)
	{
	guibuffer->guiFillRect(frame_wavetrack.x,frame_wavetrack.y,frame_wavetrack.x2,frame_wavetrack.y2,COLOUR_YELLOW);

	int sy=frame_wavetrack.y;
	int sy2=sy+maingui->GetFontSizeY()+4;

	// type
	if(sy2<frame_wavetrack.y2)
	{
	char *c="?";

	Edit_PianoWave_Type *et=new Edit_PianoWave_Type;

	guiobjects.AddGUIObject(frame_wavetrack.x,sy,frame_wavetrack.x2,sy2,&frame_wavetrack,et);

	guibuffer->guiFillRect3D(frame_wavetrack.x,sy,frame_wavetrack.x2,sy2,COLOUR_GREY_LIGHT);

	switch(wave.status)
	{
	case NOTEON:
	c="Velocity";
	break;

	case NOTEOFF:
	c="Velocity Off";
	break;

	case CONTROLCHANGE:
	c="Control Change";
	break;

	case PROGRAMCHANGE:
	c="Program Change";
	break;

	case POLYPRESSURE:
	c="Poly Pressure";
	break;

	case CHANNELPRESSURE:
	c="Channel Pressure";
	break;

	case PITCHBEND:
	c="Pitchbend";
	break;
	}

	if(c)
	guibuffer->guiDrawText(frame_wavetrack.x+2,sy2-2,frame_wavetrack.x2,c);

	sy+=maingui->GetFontSizeY()+4;
	sy2+=maingui->GetFontSizeY()+4;
	}

	// Channel
	if(sy2<frame_wavetrack.y2)
	{
	char h[64];

	Edit_PianoWave_Channel *et=new Edit_PianoWave_Channel;

	guiobjects.AddGUIObject(frame_wavetrack.x,sy,frame_wavetrack.x2,sy2,&frame_wavetrack,et);

	if(wave.channel==-1)
	{
	strcpy(h,"All MIDI Channels");
	}
	else
	{
	char h2[NUMBERSTRINGLEN];

	strcpy(h,"Channel:");
	mainvar->AddString(h,mainvar->ConvertIntToChar(wave.channel+1,h2));
	}

	guibuffer->guiFillRect3D(frame_wavetrack.x,sy,frame_wavetrack.x2,sy2,COLOUR_GREY_LIGHT);
	guibuffer->guiDrawText(frame_wavetrack.x+2,sy2-2,frame_wavetrack.x2,h);

	sy+=maingui->GetFontSizeY()+4;
	sy2+=maingui->GetFontSizeY()+4;
	}

	//1. Byte
	if(sy2<frame_wavetrack.y2)
	{
	char string[128];
	char *c=0;

	switch(wave.status)
	{
	case CONTROLCHANGE:
	{
	strcpy(string,"Controller:");
	mainvar->AddString(string,maingui->ByteToControlInfo(wave.byte1,-1,true));
	c=string;
	}
	break;
	}

	if(c)
	{
	Edit_PianoWave_Byte1 *et=new Edit_PianoWave_Byte1;

	guiobjects.AddGUIObject(frame_wavetrack.x,sy,frame_wavetrack.x2,sy2,&frame_wavetrack,et);

	guibuffer->guiFillRect3D(frame_wavetrack.x,sy,frame_wavetrack.x2,sy2,COLOUR_GREY_LIGHT);
	guibuffer->guiDrawText(frame_wavetrack.x+2,sy2-2,frame_wavetrack.x2,c);
	}

	sy+=maingui->GetFontSizeY()+4;
	sy2+=maingui->GetFontSizeY()+4;
	}

	if(!(winmode&WINDOWMODE_FRAMES))
	BltGUIBuffer_Frame(&frame_wavetrack);	
	}
	}
	*/
}
#endif

void Edit_Piano::AfterSolo()
{
	DrawDBBlit(noteraster,overview);
}

void Edit_Piano::SelectAllKeys(int key)
{
	Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

	while(selevent)
	{
		if(selevent->seqevent->IsSelected()==false)
		{
			switch(selevent->seqevent->GetStatus())
			{
			case NOTEON:
				{
					Note *n=(Note *)selevent->seqevent;

					if(n->key==key)
					{
						n->flag |= OFLAG_SELECTED;
					}
				}
				break;
			}
		}

		selevent=selevent->NextEvent();
	}
}

void Edit_Piano::ShowPianoHoriz(int flag)
{
	if(flag&SHOWEVENTS_TRACKS)
		DrawDBBlit(keys);

	if(flag&SHOWEVENTS_EVENTS)
		DrawDBBlit(noteraster,showwave==true?waveraster:0);

	if(flag&SHOWEVENTS_OVERVIEW)
		DrawDBBlit(overview);
}

char *Edit_Piano::GetWindowName()
{
	char h[64],h2[32];
	strcpy(h,"-N:");
	mainvar->AddString(h,mainvar->ConvertIntToChar(getcountselectedevents=patternselection.GetCountofSelectedEvents(),h2));
	mainvar->AddString(h,"/");
	mainvar->AddString(h,mainvar->ConvertIntToChar(getcountevents=patternselection.GetCountOfEvents(),h2));

	if(windowname)
	{
		if(char *hn=mainvar->GenerateString(windowname,h))
		{
			delete windowname;
			windowname=hn;
		}
	}

	return windowname;
}

void Edit_Piano::DeInitWindow()
{		
	notes.DeleteAllO();
	ClearCursor();
	PWaveEvents.DeleteAllO();
	EventEditor_Selection::DeInitWindow();
}

Edit_Piano::Edit_Piano()
{
	editorid=EDITORTYPE_PIANO;
	editorname="Piano";

	InitForms(FORM_HORZ2x2BAR_SLIDERHV);
	EditForm(0,2,CHILD_HASWINDOW);

	resizeable=true;
	minwidth=minheight=maingui->GetButtonSizeY(8);

	default_notelength=SAMPLESPERBEAT/4;
	startkey=29+24;
	lastplaykey=-1;

	for(int i=0;i<128;i++)keyactive[i]=false;

	lastfillkey=-1;

	showwave=mainsettings->piano_showwave;

	cursor.ostart=0;
	cursor.key=startkey;

	piano_keyheight=mainsettings->piano_keyheight;
	mouseoverchangelength_left=mouseoverchangelength_right=false;
	newmouselength=-1;
	newmousenote=0;
	followsongposition=mainsettings->followeditor;
	default_notelengthuseprev=mainsettings->piano_default_notelengthuseprev;
	playkeys=false;
	playmouseovernotes=mainsettings->playmouseover;
	setstarttomid=false;

	newchannel=1;
	newvelocity=127;
	newvelocityoff=0;
	editvelocity=true;
	reset_oldnotelength=-1;
}

void Edit_Piano::ShowVeloMode()
{
	if(velomode)
	{
		if(editvelocity==true)
			velomode->ChangeButtonText("Edit:Velocity");
		else
			velomode->ChangeButtonText("Edit:Velocity Off");
	}
}

void Edit_Piano::Paste()
{
	PasteMouse(noteraster);
}

bool Edit_Piano::ZoomGFX(int z,bool horiz)
{
	if(SetZoomGFX(z,horiz)==true)
	{
		DrawDBBlit(keys,noteraster,showwave==true?waveraster:0);
		return true;
	}

	return false;
}

void Edit_Piano::CheckDefaultNoteLength()
{
	if(reset_oldnotelength!=-1)
	{
		default_notelength=reset_oldnotelength;
		ShowDefaultNoteLength();

		reset_oldnotelength=-1;
	}
}

bool Edit_Piano::EditCancel(bool dontchangenotelength)
{
	switch(mousemode)
	{
	case EM_MOVEOS:
	case EM_SIZENOTES_LEFT:
	case EM_SIZENOTES_RIGHT:
		{
			SetEditorMode(EM_RESET);
			DrawDBBlit(noteraster,showwave==true?waveraster:0);

			if(dontchangenotelength==false)
				CheckDefaultNoteLength();

			return true;
		}
		break;

	case EM_SELECTOBJECTS:
		{
			maingui->ResetPRepairSelection(&patternselection);
			ResetMouseMode();
			return true;
		}
		break;
	}

	return false;
}

void Edit_Piano::ShowSlider()
{
	if(vertgadget)
		vertgadget->ChangeSlider(&keyobjects,zoomy);
}

void PianoEditor_Keys_Callback(guiGadget_CW *g,int status)
{
	Edit_Piano *p=(Edit_Piano *)g->from;

	switch(status)
	{
	case DB_CREATE:
		p->keys=g;
		break;


	case DB_PAINT:
		{
			p->ShowKeys();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		p->MouseClickInKeys(true);	
		break;

	case DB_LEFTMOUSEUP:
		p->MouseUpInKeys(true);
		break;

	case DB_DOUBLECLICKLEFT:
		p->MouseDoubleClickInKeys(true);
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInOverview(false);	
		break;
	}
}

void PianoEditor_Overview_Callback(guiGadget_CW *g,int status)
{
	Edit_Piano *p=(Edit_Piano *)g->from;

	switch(status)
	{
	case DB_CREATE:
		p->overview=g;
		break;

	case DB_PAINT:
		{
			p->ShowOverview();
			p->ShowOverviewCycleAndPositions();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		p->MouseClickInOverview(true);	
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		p->MouseClickInOverview(false);	
		break;
	}
}

void ShowNotes_Callback(guiGadget_CW *g,int status)
{
	Edit_Piano *p=(Edit_Piano *)g->from;

	switch(status)
	{
	case DB_CREATE:
		g->menuindex=0;
		p->noteraster=g;
		break;

	case DB_PAINT:
		{
			p->ShowEvents();
			p->ShowCycleAndPositions(g);
		}
		break;

	case DB_PAINTSPRITE:
		p->ShowCycleAndPositions(p->noteraster);
		break;

	case DB_LEFTMOUSEDOWN:
		p->MouseClickInNotes(true);
		break;

	case DB_DOUBLECLICKLEFT:
		p->MouseDoubleClickInNotes(true);
		break;

	case DB_LEFTMOUSEUP:
		p->MouseReleaseInNotes(true);
		break;

	case DB_RIGHTMOUSEDOWN:
		p->MouseClickInNotes(false);
		break;

	case DB_MOUSEMOVE:
		p->MouseMoveInNotes(false);	
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		p->MouseMoveInNotes(true);	
		break;

		//case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
		//case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInOverview(false);	
		//break;
	}
}

void PianoEditor_WaveTrackEvents_Callback(guiGadget_CW *g,int status)
{
	Edit_PianoWave *p=(Edit_PianoWave *)g->from;

	switch(status)
	{
	case DB_CREATE:
		g->menuindex=0;
		p->editor->waveraster=g;
		break;

	case DB_PAINT:
		{
			p->editor->ShowWaveEvents();
			p->editor->ShowCycleAndPositions(g);
		}
		break;

	case DB_LEFTMOUSEDOWN:
		p->editor->MouseClickInData(true);
		break;

	case DB_LEFTMOUSEUP:
		p->editor->MouseReleaseInData(true);
		break;

	case DB_MOUSEMOVE:
	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		p->editor->MouseMoveInData(true);
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInOverview(false);	
		break;
	}
}

void Edit_Piano::MouseWheel(int delta,guiGadget *db)
{
	if(Edit_MouseWheel(delta)==false)
	{
		if((!db) || db==noteraster || db==keys)
		{
			if(vertgadget)
				vertgadget->DeltaY(delta);
		}
	}
}

void Edit_Piano::InitGadgets()
{
	glist.SelectForm(0,0);
	guitoolbox.CreateToolBox(TOOLBOXTYPE_EDITOR);
	glist.Return();

	InitSelectionPatternGadget();

	int addw=INFOSIZE;

	glist.AddButton(-1,-1,addw,-1,"Data",GADGETID_DATA,showwave==true?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED|MODE_AUTOTOGGLE:MODE_GROUP|MODE_TOGGLE|MODE_AUTOTOGGLE);
	glist.AddLX();

	// Note Info/Edit Buttons
	// Start

	int iw=bitmap.GetTextWidth("WWWw");

	glist.AddButton(-1,-1,iw/2,-1,"S",GADGETID_EDITNOTE_TIME_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	infonote_time=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGETID_EDITNOTE_TIME,windowtimeformat,MODE_INFO,"Start Position (Focus Note)");
	glist.AddLX();

	glist.AddButton(-1,-1,iw/2,-1,"E",GADGETID_EDITNOTE_END_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	infonote_end=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGETID_EDITNOTE_END,windowtimeformat,MODE_INFO,"End Position (Focus Note)");
	glist.AddLX();

	glist.AddButton(-1,-1,iw/2,-1,"C",GADGETID_EDITNOTE_CHL_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	infonote_chl=glist.AddNumberButton(-1,-1,2*maingui->GetFontSizeY(),-1,GADGETID_EDITNOTE_CHL,1,16,1,NUMBER_INTEGER,MODE_INFO,"Channel (Focus Note)");
	glist.AddLX();

	glist.AddButton(-1,-1,iw/2,-1,"N",GADGETID_EDITNOTE_KEY_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	infonote_key=glist.AddNumberButton(-1,-1,3*maingui->GetFontSizeY(),-1,GADGETID_EDITNOTE_KEY,0,127,0,NUMBER_KEYS,MODE_INFO,"Key (Focus Note)");
	glist.AddLX();

	glist.AddButton(-1,-1,iw/2,-1,"V",GADGETID_EDITNOTE_VELO_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	infonote_velo=glist.AddNumberButton(-1,-1,3*maingui->GetFontSizeY(),-1,GADGETID_EDITNOTE_VELO,1,127,1,NUMBER_INTEGER,MODE_INFO,"Velocity (Focus Note)");
	glist.AddLX();

	glist.AddButton(-1,-1,iw/2,-1,"Vo",GADGETID_EDITNOTE_VELOOFF_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	infonote_velooff=glist.AddNumberButton(-1,-1,3*maingui->GetFontSizeY(),-1,GADGETID_EDITNOTE_VELOOFF,0,127,0,NUMBER_INTEGER,MODE_INFO,"Velocity Off (Focus Note)");
	glist.AddLX();

	glist.AddButton(-1,-1,iw/2,-1,"L",GADGETID_EDITNOTE_LENGTH_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	infonote_length=glist.AddLengthButton(-1,-1,bitmap.prefertimebuttonsize,-1,-1,0,GADGETID_EDITNOTE_LENGTH,windowtimeformat,MODE_INFO,Cxs[CXS_LENGTH]);

	glist.AddLX();

	glist.AddButton(-1,-1,iw,-1,"nC",GADGETID_EDITNOTE_CHANNEL_NEWCHANNEL_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	newnote_chl=glist.AddNumberButton(-1,-1,2*maingui->GetFontSizeY(),-1,GADGETID_EDITNOTE_CHANNEL_NEWCHANNEL,1,16,newchannel,NUMBER_INTEGER,0,"Channel");
	glist.AddLX();

	glist.AddButton(-1,-1,iw,-1,"nV",GADGETID_EDITNOTE_CHANNEL_NEWVELO_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	newnote_velo=glist.AddNumberButton(-1,-1,3*maingui->GetFontSizeY(),-1,GADGETID_EDITNOTE_CHANNEL_NEWVELO,1,127,newvelocity,NUMBER_INTEGER,0,"Velocity");
	glist.AddLX();

	glist.AddButton(-1,-1,iw,-1,"nVOff",GADGETID_EDITNOTE_CHANNEL_NEWVELOOFF_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	newnote_velooff=glist.AddNumberButton(-1,-1,3*maingui->GetFontSizeY(),-1,GADGETID_EDITNOTE_CHANNEL_NEWVELOOFF,0,127,newvelocityoff,NUMBER_INTEGER,0,"Velocity Off");
	glist.AddLX();

	glist.AddButton(-1,-1,iw,-1,"nL",GADGETID_EDITNOTE_CHANNEL_NEWLENGTH_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	newnote_len=glist.AddLengthButton(-1,-1,bitmap.prefertimebuttonsize,-1,-1,0,GADGETID_EDITNOTE_CHANNEL_NEWLENGTH,windowtimeformat,0,Cxs[CXS_LENGTH]);

	ShowDefaultNoteLength();
	ShowFocusEvent();

	int offsettracksy=SIZEV_OVERVIEW+SIZEV_HEADER+2*(ADDYSPACE+1);

	glist.SelectForm(0,1);

	SliderCo horz,vert;

	horz.formx=0;
	horz.formy=3;

	vert.formx=2;
	vert.formy=1;
	vert.offsety=offsettracksy;
	vert.from=0;
	vert.to=0; // trackobjects.GetCount()-numberoftracks;
	vert.pos=0; //firstshowtracknr;

	AddEditorSlider(&horz,&vert);

	glist.SelectForm(0,1);
	glist.AddButton(-1,-1,-1,-1,Cxs[CXS_PLAYPIANOMOUSEOVER],GADGETID_PLAYMOUSE,playmouseovernotes==true?MODE_AUTOTOGGLE|MODE_TOGGLE|MODE_RIGHT|MODE_TOGGLED:MODE_AUTOTOGGLE|MODE_TOGGLE|MODE_RIGHT,Cxs[CXS_PLAYPIANOMOUSEOVER]);
	glist.Return();
	velomode=glist.AddButton(-1,-1,-1,-1,"",GADGETID_VELOMODE,MODE_RIGHT|MODE_TEXTCENTER,"Velocity/Velocity Off");
	ShowVeloMode();

	glist.AddChildWindow(-1,offsettracksy,-2,-2,MODE_RIGHT|MODE_BOTTOM,0,&PianoEditor_Keys_Callback,this);

	glist.SelectForm(1,1);
	glist.AddChildWindow(-1,-1,-1,SIZEV_OVERVIEW,MODE_RIGHT|MODE_SPRITE,0,&PianoEditor_Overview_Callback,this);
	glist.Return();
	glist.AddChildWindow(-1,-1,-1,SIZEV_HEADER,MODE_RIGHT|MODE_SPRITE,0,&Editor_Header_Callback,this);

	editarea=glist.AddChildWindow(-1,offsettracksy,-1,-2,MODE_BOTTOM|MODE_RIGHT|MODE_SPRITE,0,&ShowNotes_Callback,this);

	// Wave
	glist.SelectForm(0,2);
	glist.form->BindWindow(wave=new Edit_PianoWave(this));

	notetype=WindowSong()->notetype;
}

void Edit_Piano::InitNewTimeType()
{
	ShowFocusEvent();
	ShowDefaultNoteLength();
}

void Edit_Piano::Init()
{	
	patternselection.BuildEventList(SEL_ALLEVENTS,0); // Mix new List, events maybe moved/deleted

	Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

	int min_key=-1,max_key=-1;

	// Init Start Key
	// NoteOns ---------------------------------------------------------
	while(selevent)
	{	
		if(selevent->seqevent->GetStatus()==NOTEON)
		{
			Note *note=(Note *)selevent->seqevent;

			if(max_key==-1 || note->key>max_key)
				max_key=note->key;

			if(min_key==-1 || note->key<min_key)
				min_key=note->key;

			/*
			if(note->GetEventStart()>=startposition || note->GetNoteEnd()>=startposition)
			{
			keyobjects.InitStartOffSetY(127-note->key);
			break;
			}
			*/
		}

		selevent=selevent->NextEvent();
	}

	if(max_key!=-1)
	{
		//int mid=max_key-(max_key-min_key)/2;
		keyobjects.InitStartOffSetY(127-min_key);
	}
	else
		keyobjects.InitStartOffSetY(64);

	InitGadgets();
}

void Edit_Piano::RedrawDeletedPattern()
{
	patternselection.BuildEventList(SEL_ALLEVENTS,0); // Mix new List, events maybe moved/deleted
	RefreshObjects(0,false);
}

void Edit_Piano::RefreshObjects(LONGLONG type,bool editcall)
{
	if(editcall==true)
	{
		if(((refreshflag&REFRESHEVENT_LIST)==0) && patternselection.CheckEventList()==true)
		{
			refreshflag|=REFRESHEVENT_LIST;
		}
	}

	if(patternselection.patternremoved==true)
	{
		ShowSelectionPattern();
	}

	DrawHeader(); // Tempo Changes etc..
	ShowAllEvents(refreshflag&REFRESHEVENT_LIST?0:NOBUILD_REFRESH);

	DrawDBBlit(noteraster,showwave==true?waveraster:0,overview);

	patternselection.patternremoved=false; // reset
}

void Edit_Piano::SetMouseMode(int newmode,OSTART mp,int key)
{
	if(mp==-1)
	{
		InitMousePosition();
		modestartposition=GetMousePosition(); // X
	}
	else
		modestartposition=mp;

	if(modestartposition>=0)
	{
		if(key==-1)
			modestartkey=FindKeyAtPosition(noteraster->GetMouseY()); // Y
		else
			modestartkey=key;

		if(modestartkey!=-1)
		{
			SetEditorMode(newmode);
			SetAutoScroll(newmode,noteraster);
		}
	}
}

void Edit_Piano::Goto(int to)
{
	UserEdit();

	Seq_SelectionEvent *el=0;

	if(CheckStandardGoto(to)==true)
		return;

	switch(to)
	{
	case GOTO_FOCUS:
		el=patternselection.FindEvent(WindowSong()->GetFocusEvent());
		break;

	case GOTO_FIRST:
		{
			el=patternselection.FirstMixEvent();

			while(el && el->seqevent->GetStatus()!=NOTEON) // Note
				el=el->NextEvent();
		}
		break;

	case GOTO_LAST:
		{
			el=patternselection.LastMixEvent();

			while(el && el->seqevent->GetStatus()!=NOTEON) // Note
				el=el->PrevEvent();
		}
		break;

	case GOTO_FIRSTSELECTED:
		{
			el=patternselection.FirstMixEvent();

			while(el && 
				(el->seqevent->GetStatus()!=NOTEON || ((el->seqevent->flag&OFLAG_SELECTED)==0))) // Note
				el=el->NextEvent();
		}
		break;

	case GOTO_LASTSELECTED:
		{
			el=patternselection.LastMixEvent();

			while(el && 
				(el->seqevent->GetStatus()!=NOTEON || ((el->seqevent->flag&OFLAG_SELECTED)==0))) // Note
				el=el->PrevEvent();
		}
		break;

	case GOTO_FIRSTEVENTTRACK:
		if(WindowSong()->GetFocusTrack())
		{
			el=patternselection.FirstMixEvent();
			Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

			while(el && 
				(el->seqevent->GetPattern()->GetTrack()!=WindowSong()->GetFocusTrack() ||
				el->seqevent->GetStatus()!=NOTEON
				)
				)
				el=el->NextEvent();
		}
		break;

	case GOTO_LASTEVENTTRACK:
		if(WindowSong()->GetFocusTrack())
		{
			el=patternselection.LastMixEvent();
			Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

			while(el && 
				(el->seqevent->GetPattern()->GetTrack()!=WindowSong()->GetFocusTrack() ||
				el->seqevent->GetStatus()!=NOTEON
				)
				)
				el=el->PrevEvent();
		}
		break;
	}

	if(el && el->seqevent->GetStatus()==NOTEON) // Scroll Up<->
	{
		Note *n=(Note *)el->seqevent;

		// Mid Key
		int nskey=n->key+numberofkeys/2;

		if(nskey>127)nskey=127;

		SetStartKey(nskey);
		if(NewStartPosition(el->seqevent->GetEventStart(),true)==true)
			SyncWithOtherEditors();
	}
}

void Edit_Piano::AddStartY(int addy)
{
	if(keyobjects.AddStartY(addy)==true)
		ShowPianoHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_TRACKS|SHOWEVENTS_PIANOWAVETRACK);
}

void Edit_Piano::SetStartKey(int key)
{
	if(key<=127 && key>=0)
	{
		int spy=(127-key)*zoomy;

		if(keyobjects.SetStartY(spy)==true)
			ShowPianoHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_TRACKS|SHOWEVENTS_PIANOWAVETRACK);
	}
}

void Edit_Piano::InitMouseEditRange()
{
	int mx=noteraster->GetMouseX();
	int my=noteraster->GetMouseY();

	mskey=modestartkey;
	mekey=FindKeyAtPosition(my);

	if(mekey==-1)
	{
		if(my<0)mekey=startkey;
		else mekey=lastkey;
	}

	msposition=modestartposition;
	msendposition=mx<noteraster->GetX2()?timeline->ConvertXPosToTime(mx):endposition;

	if(msposition>msendposition){
		OSTART h=msendposition;
		msendposition=msposition;
		msposition=h;
	}

	if(mskey<mekey){
		int h=mekey;
		mekey=mskey;
		mskey=h;
	}

	int sx=timeline->ConvertTimeToX(msposition);
	int sx2=timeline->ConvertTimeToX(msendposition);
	int sy=keyondisplay[mskey]==true?keyposy[mskey]:0;
	int sy2=keyondisplay[mekey]==true?keyposy2[mekey]:noteraster->GetY2();

	mouseeditx=sx==-1?0:sx;
	mouseeditx2=sx2==-1?noteraster->GetX2():sx2;
	mouseedity=sy;
	mouseedity2=sy2;
}

void Edit_Piano::AutoScroll()
{
	DoAutoScroll();
	DoStandardYScroll(noteraster);
}

void Edit_Piano::FindAndDeleteNotes(Note *note)
{	
	if(note)
		note->Select();

	DeleteNotes();
}

void Edit_Piano::ShowSizeNotesSprites()
{
	InitMousePosition();

	if(GetMousePosition()>=0)
	{
		OSTART diff=GetMousePosition()-modestartposition;

		Note *n=(Note *)patternselection.FirstSelectionEvent(0,SEL_NOTEON|SEL_SELECTED);

		while(n)
		{
			OSTART start=n->GetEventStart(),end=n->GetNoteEnd();

			if(mousemode==EM_SIZENOTES_LEFT) // Start Position
			{
				start+=diff;
				if(start>=end)
				{
					diff=0;
					break;
				}
			}
			else // End Position -> Length
			{
				end+=diff;

				if(n==WindowSong()->GetFocusEvent())
				{
					if(reset_oldnotelength==-1)
						reset_oldnotelength=n->GetNoteEnd()-n->GetEventStart();

					default_notelength=end-start;
					ShowDefaultNoteLength();
				}

				if(end<=start)
				{
					diff=0;
					break;
				}
			}

			n=(Note *)patternselection.NextSelectionEvent(SEL_NOTEON|SEL_SELECTED);
		}

		patternselection.movediff=diff;

		noteraster->DrawGadgetBlt();
	}
}

void Edit_Piano::ShowMoveNotesSprites()
{
	int my=noteraster->GetMouseY(),keynow=FindKeyAtPosition(my);
	InitMousePosition();

	if(GetMousePosition()>=0 && keynow!=-1)
	{
		patternselection.moveobjects_vert=CanMoveY()==true?keynow-modestartkey:0;

		//		TRACE ("P Move V %d\n",patternselection.moveobjects_vert);

		patternselection.movediff=CanMoveX()==true?GetMousePosition()-modestartposition:0;
		noteraster->DrawGadgetBlt();
	}
}

void Edit_Piano::UserMessage(int msg,void *par)
{
	switch(msg)
	{
	case MESSAGE_CHECKMOUSEBUTTON:
		{
			if(maingui->GetShiftKey()==false && par)
			{
				Note *note=(Note *)par;

				if(maingui->GetLeftMouseButton()==true)
				{	
					Edit_Piano_Note *epn=(Edit_Piano_Note *)notes.GetRoot();

					while(epn)
					{
						if(epn->note==note)
						{
							SetMouseMode(EM_MOVEOS,-1,note->key);
							ShowMoveNotesSprites();
							return;
						}

						epn=(Edit_Piano_Note *)epn->next;
					}

					return;
				}

				patternselection.SelectAllEventsNot(note,false,0);
			}
		}
		break;
	}
}

int Edit_Piano::FindKeyAtPosition(int y) // -1 no key
{
	for(int i=startkey;i>=lastkey;i--)
	{
		if(keyposy[i]<=y && keyposy2[i]>=y)
			return i;
	}

	return -1;
}

void Edit_Piano::FillMouseKey()
{
	/*
	if(guibuffer)
	{
	int key=FindKeyAtPosition(GetMouseY());

	if(key!=lastfillkey)
	{	
	lastfillkey=key;
	ShowKeys();
	}
	}
	*/
}

Edit_Piano_Note *Edit_Piano::FindNoteUnderMouse(int flag)
{
	if(!noteraster)
		return 0;

	int x=noteraster->GetMouseX(),y=noteraster->GetMouseY();

	Edit_Piano_Note *found=0,*pn=(Edit_Piano_Note *)notes.GetRoot();

	while(pn){

		if(pn->x<=x && pn->x2>=x && pn->y<y && pn->y2>=y)
		{
			if((pn->note->flag&flag)==0)
				found=pn;		
		}
		else
			pn->note->flag CLEARBIT flag;

		pn=(Edit_Piano_Note *)pn->next;
	}

	if(found)
	{
		found->note->flag|=flag;
		return found;
	}

	return 0;
}

void Edit_Piano::PlayMouseMoveKey()
{
	if(playmouseovernotes==true)
	{
		if(Edit_Piano_Note *pn=FindNoteUnderMouse(SEQSEL_MOUSEOVER))
		{
			Note *fnote=pn->note,note;

			note.status=fnote->status;;
			note.key=fnote->key;
			note.velocity=127;
			note.velocityoff=0;

			note.ostart=0;

			OSTART length=fnote->GetNoteLength();

			if(length>MAXUSERTICKS)
				length=MAXUSERTICKS;

			note.off.ostart=length;

			MIDIPattern *mp=fnote->GetMIDIPattern();

			mp->track->SendOutEvent_User(mp,&note,true);
		}
	}
}

int Edit_Piano::PlayKeyNote(bool check,int key)
{
	// Key Clicked ?
	//int key=FindKeyAtPosition(mousey);

	if(key!=-1 && (check==false || lastplaykey!=key))
	{
		lastplaykey=key;

		// Play Key Sound
		if(insertpattern && insertpattern->mediatype==MEDIATYPE_MIDI)
		{
			Note note;

			note.status=NOTEON|(newchannel-1);
			note.key=key;
			note.velocity=newvelocity;
			note.velocityoff=newvelocityoff;

			note.pattern=insertpattern;
			note.ostart=WindowSong()->GetSongPosition();
			note.off.ostart=note.ostart+default_notelength;

			insertpattern->track->SendOutEvent_User((MIDIPattern *)insertpattern,&note,true);
		}
	}

	return key;
}

Note *Edit_Piano::FindNoteUnderCursor()
{
	Seq_SelectionEvent *selevent=patternselection.FirstMixEvent();

	// NoteOns ---------------------------------------------------------
	while(selevent && selevent->seqevent->GetEventStart()<=cursor.ostart)
	{	
		if(selevent->seqevent->GetStatus()==NOTEON)
		{
			Note *note=(Note *)selevent->seqevent;

			if(note->GetNoteEnd()>=cursor.ostart+default_notelength && note->key==cursor.key)
				return note;
		}

		selevent=selevent->NextEvent();
	}//while event

	return 0;
}

void Edit_Piano::KeyDownRepeat()
{
	Editor_KeyDown();
}

void Edit_Piano::KeyDown()
{
	Editor_KeyDown();

	switch(nVirtKey)
	{
	case '1':
		wave->SetSet(0);
		break;

	case '2':
		wave->SetSet(1);
		break;

	case '3':
		wave->SetSet(2);
		break;

	case '4':
		wave->SetSet(3);
		break;

	case '5':
		wave->SetSet(4);
		break;

	case KEY_CURSORUP:
		if(lastkey<127)
		{
			startkey++;
			ShowPianoHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_TRACKS|SHOWEVENTS_PIANOWAVETRACK);
			ShowSlider();
		}
		break;

	case KEY_CURSORDOWN:
		if(startkey>0)
		{
			startkey--;
			ShowPianoHoriz(SHOWEVENTS_EVENTS|SHOWEVENTS_TRACKS|SHOWEVENTS_PIANOWAVETRACK);
			ShowSlider();
		}
		break;

		// Cursor ?
	case 'W':
		if(cursor.key<127)
		{
			cursor.key++;
			ShowCursor();
			PlayCursor();
		}
		break;

	case 'S':
		if(cursor.key>0)
		{
			cursor.key--;
			ShowCursor();
			PlayCursor();
		}
		break;

	case 'Q':
		{
			SetCursorToMousePosition();
			PlayCursor();
		}
		break;

	case 'R': // Delete Note under Cursor
		{
			Note *f=FindNoteUnderCursor();

			if(f)
				mainedit->DeleteEvent(f,false);
		}
		break;

	case 'E': // create Note under Cursor
		if(insertpattern && insertpattern->mediatype==MEDIATYPE_MIDI)
		{
#ifdef MEMPOOLS
			Note *note=mainpools->mempGetNote();
#else
			Note *note=new Note;
#endif

			if(note)
			{
				OSTART pos=cursor.ostart;
				int status=NOTEON;
				int chl=((MIDIPattern *)insertpattern)->GetDominantMIDIChannel();

				if(chl==-1) // different channels
				{
					chl=insertpattern->track->GetFX()->GetChannel(); // dummy

					if(chl)
						chl=chl-1;	
				}

				status|=chl;

				// Insert Quantize ?
				{
					QuantizeEffect *qeff=insertpattern->GetQuantizer();

					if(qeff)
						pos=qeff->Quantize(pos);
				}

				OSTART end=pos+default_notelength;

				note->ostart=note->staticostart=pos;
				note->off.ostart=note->off.staticostart=end;

				note->status=status;
				note->key=cursor.key;
				note->velocity=newvelocity;
				note->velocityoff=newvelocityoff;

				editevent.song=WindowSong();
				editevent.pattern=(MIDIPattern *)insertpattern;
				editevent.seqevent=note;

				editevent.position=pos;
				editevent.endposition=end;

				editevent.doundo=true;
				editevent.addtolastundo=false;
				editevent.playit=true;

				//	mainMIDI->SendMIDIRealtimeEvent(mp->track,mp,note,end-pos); // Play Note

				mainedit->CreateNewMIDIEvent(&editevent);
			}
		}
		break;

	case 'A':
	case 'D':
		{
			OSTART add;

			switch(mousequantize)
			{
			case MOUSEQUANTIZE_MEASURE: // Measure
				{
					OSTART h=WindowSong()->timetrack.ConvertTicksToMeasureTicks(cursor.ostart,false);
					Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(h);
					add=(h+sig->measurelength)-cursor.ostart;
				}
				break;

			case MOUSEQUANTIZE_BEAT: // Measure
				{
					OSTART h=WindowSong()->timetrack.ConvertTicksToBeatTicks(cursor.ostart,false);
					Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(h);
					add=(h+sig->dn_ticks)-cursor.ostart;
				}
				break;

			case MOUSEQUANTIZE_1:
				add=TICK1nd;
				break;

			case MOUSEQUANTIZE_12:
				add=TICK2nd;
				break;

			case MOUSEQUANTIZE_14:
				add=TICK4nd;
				break;

			case MOUSEQUANTIZE_18:
				add=TICK8nd;
				break;

			case MOUSEQUANTIZE_16:
				add=TICK16nd;
				break;

			case MOUSEQUANTIZE_ZOOM:
				add=WindowSong()->timetrack.zoomticks;
				break;

			default:
				add=1; // 1 tick
				break;
			}

			if(nVirtKey=='A')
				add*=-1;

			OSTART newstart=cursor.ostart+add;

			if(newstart<0)
				newstart=0;

			if(cursor.ostart!=newstart)
			{
				cursor.ostart=newstart;
				ShowCursor();
				PlayCursor();
			}
		}
		break;
	}
}

void Edit_Piano::RefreshActiveKeys(bool blit)
{
	int from=blit==true?startkey:127,to=blit==true?lastkey:0;
	bool invert=false,noteopen=WindowSong()->FindOpenNote(-1);

#ifdef DEBUG
	if(from>127 || from<0 || to>127 || to<0)
		maingui->MessageBoxError(0,"RefreshActiveKeys");
#endif

	// Piant Piano Keys
	for(int i=from;i>=to;i--)
	{
		if(keyactive[i]==false) // Check
		{
			if(noteopen==true && WindowSong()->FindOpenNote(i)==true)
			{
				keyactive[i]=true;

				if(keyondisplay[i]==true)
					invert=true;
			}
		}
		else
		{
			if(WindowSong()->FindOpenNote(i)==false)
			{
				keyactive[i]=false;

				if(keyondisplay[i]==true)
					invert=true;
			}
		}
	}

	if(invert==true)
	{
		DrawDBBlit(keys);
	}
}

void Edit_Piano::RefreshRealtime_Slow()
{
	RefreshEventEditorRealtime_Slow();
	RefreshSelectionGadget();

	if(notetype!=WindowSong()->notetype)
	{
		notetype=WindowSong()->notetype;
		DrawDBBlit(keys,noteraster);
	}

	/*
	int h_getcountselectedevents=patternselection.GetCountofSelectedEvents(),h_getcountevents=patternselection.GetCountOfEvents();

	if(h_getcountselectedevents!=getcountselectedevents ||
	h_getcountevents!=getcountevents)
	SongNameRefresh();
	*/


	// Mouse over Start/End Check

	int mx=noteraster->GetMouseX();
	int my=noteraster->GetMouseY();

	bool omouseoverchangelength_left=mouseoverchangelength_left;
	bool omouseoverchangelength_right=mouseoverchangelength_right;

	mouseoverchangelength_left=mouseoverchangelength_right=false;

	if(mousemode==EM_SELECT)
	{
		if(mx>=0 && my<=noteraster->GetX2() && my>=0 && my<=noteraster->GetY2())
		{
			Edit_Piano_Note *c=(Edit_Piano_Note *)notes.GetRoot();

			while(c)
			{
				if(c->y<=my && c->y2>=my)
				{
					if(c->x-MOUSERANGEX<=mx && c->x+MOUSERANGEX>=mx && ((c->flag&Edit_Piano_Note::NOSTART)==0))
					{
						//if(mouseoverchangelength_left==false)
						//SetMouseCursor(CURSOR_LEFT);
						mouseoverchangelength_left=true;

						if(!(c->flag&Edit_Piano_Note::MOUSEOVERLEFT))
						{
							c->flag|=Edit_Piano_Note::MOUSEOVERLEFT;
							c->flag CLEARBIT Edit_Piano_Note::MOUSEOVERRIGHT;

							RefreshEvents();
						}

						goto jumpovercheck;
					}

					if(c->x2+MOUSERANGEX>mx && c->x2-MOUSERANGEX<=mx && ((c->flag&Edit_Piano_Note::NOEND)==0))
					{
						//if(mouseoverchangelength_right==false)
						//SetMouseCursor(CURSOR_RIGHT);
						mouseoverchangelength_right=true;

						if(!(c->flag&Edit_Piano_Note::MOUSEOVERRIGHT))
						{
							c->flag|=Edit_Piano_Note::MOUSEOVERRIGHT;
							c->flag CLEARBIT Edit_Piano_Note::MOUSEOVERLEFT;

							RefreshEvents();
						}

						goto jumpovercheck;
					}
				}

				c=(Edit_Piano_Note *)c->next;
			}
		}
	}

	if(omouseoverchangelength_right!=mouseoverchangelength_right || omouseoverchangelength_left!=mouseoverchangelength_left)
	{
		Edit_Piano_Note *c=(Edit_Piano_Note *)notes.GetRoot();

		while(c)
		{
			c->flag CLEARBIT (Edit_Piano_Note::MOUSEOVERLEFT+Edit_Piano_Note::MOUSEOVERRIGHT);
			c=(Edit_Piano_Note *)c->next;
		}

		RefreshEvents();
	}

jumpovercheck:

	return;
}

void Edit_Piano::RefreshFocusEvent()
{
	Note *infonote=WindowSong()->GetFocusEvent() && WindowSong()->GetFocusEvent()->GetStatus()==NOTEON?(Note *)WindowSong()->GetFocusEvent():0;

	if(infonote)
	{
		if(infonote->GetEventStart()!=focus_start)
		{
			ShowFocusEvent();
			return;
		}

		if(infonote->GetChannel()!=focus_channel)
		{
			ShowFocusEvent();
			return;
		}

		if(infonote->key!=focus_key)
		{
			ShowFocusEvent();
			return;
		}

		if(infonote->velocity!=focus_velo)
		{
			ShowFocusEvent();
			return;
		}

		if(infonote->velocityoff!=focus_velooff)
		{
			ShowFocusEvent();
			return;
		}

		if(infonote->GetNoteLength()!=focus_length)
		{
			ShowFocusEvent();
			return;
		}
	}

}

void Edit_Piano::RefreshRealtime()
{
	bool refreshnotes=false,refreshwave=false;

	RefreshFocusEvent();

	// New Events added - Recording ?
	{
		Seq_SelectedPattern *s=patternselection.FirstSelectedPattern();
		while(s)
		{
			if(s->status_nrpatternevents!=s->pattern->GetCountOfEvents())
			{
				ShowAllEvents();
				break;
			}

			s=s->NextSelectedPattern();
		}
	}

	if(WindowSong()->mastering==false)
	{
		OSTART songposition=WindowSong()->GetSongPosition();
		Edit_Piano_Note *pn=(Edit_Piano_Note *)notes.GetRoot();

		while(pn){

			if(pn->eflag!=pn->sevent->seqevent->flag)  // Selection On/Off etc
				refreshnotes=true;

			if(pn->sevent->seqevent==WindowSong()->GetFocusEvent() && pn->infonote==false)
				refreshnotes=true;

			if(WindowSong()->status==Seq_Song::STATUS_STOP)
			{
				if(pn->sevent->flag&SEQSEL_REALTIMEACTIVATED)
				{
					pn->sevent->flag CLEARBIT SEQSEL_REALTIMEACTIVATED;
					refreshnotes=true;
				}
			}
			else
			{
				if((!(pn->sevent->flag&SEQSEL_REALTIMEACTIVATED)))
				{
					if(pn->note->GetEventStart()<=songposition && pn->note->GetNoteEnd()>songposition)
					{
						pn->sevent->flag |= SEQSEL_REALTIMEACTIVATED;
						refreshnotes=true;
					}
				}
				else
					if(songposition>pn->note->GetNoteEnd() || songposition<pn->note->GetEventStart() ) // |--- or cycle
					{
						pn->sevent->flag CLEARBIT SEQSEL_REALTIMEACTIVATED;
						refreshnotes=true;
					}
			}	

			pn=(Edit_Piano_Note *)pn->next;
		}
	}

	if(RefreshEventEditorRealtime()==true)
	{
		if(noteraster)
		{
			ShowCycleAndPositions(noteraster);
			if(refreshnotes==false)
				noteraster->DrawSpriteBlt();
		}

		if(waveraster && waveraster->formchild->InUse()==true)
		{
			ShowCycleAndPositions(waveraster);
			if(refreshwave==false)
				waveraster->DrawSpriteBlt();
		}
	}

	if(refreshnotes==true)
		RefreshEvents();

	//RefreshSprites();

	if(WindowSong()->mastering==false)
		RefreshActiveKeys(true);
	//RefreshToolTip();
}

char *Edit_Piano::GetToolTipString1() //v
{
	char *string=0;

#ifdef OLDIE
	Seq_SelectionEvent *e=patternselection.FindEventOnDisplay(GetMouseX(),GetMouseY());

	if(e) // Note clicked
	{
		if(e->seqevent->GetStatus()==NOTEON)
		{
			Note *note=(Note *)e->seqevent;

			string=new char[64];

			if(string)
			{
				char h2[16];

				strcpy(string,maingui->ByteToKeyString(WindowSong(),note->key));
				mainvar->AddString(string," C:");
				mainvar->AddString(string,mainvar->ConvertIntToChar(note->GetChannel(),h2));
				mainvar->AddString(string," V:");
				mainvar->AddString(string,mainvar->ConvertIntToChar(note->velocity,h2));
				mainvar->AddString(string," O:");
				mainvar->AddString(string,mainvar->ConvertIntToChar(note->velocityoff,h2));
			}
		}
	}
	else
	{
		int k=FindKeyAtPosition(GetMouseY());

		if(k!=1)
			string=mainvar->GenerateString(maingui->ByteToKeyString(WindowSong(),k));
	}
#endif

	return string;
}

char *Edit_Piano::GetToolTipString2() //v
{
	char *string=0;

#ifdef OLDIE
	Seq_SelectionEvent *e=patternselection.FindEventOnDisplay(GetMouseX(),GetMouseY());

	if(e) // Note clicked
	{
		if(e->seqevent->GetStatus()==NOTEON)
		{
			Note *note=(Note *)e->seqevent;
			string=mainvar->GenerateString("P:",note->pattern->GetName());
		}
	}
#endif

	return string;
}


Edit_PianoWave::Edit_PianoWave(Edit_Piano *ep)
{
	editorid=EDITORTYPE_PIANOWAVE;
	//isstatic=true;
	InitForms(FORM_PLAIN1x1);

	editor=ep;
	song=ep->WindowSong();

	//vertscroll=true;
	set=mainsettings->defaultpianosettings;
}

void Edit_PianoWave::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_CHANNELNR:
		GetSet()->channel=g->GetPos();
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
	}
}

void Edit_PianoWave::ShowWaveChannel()
{
	if(g_channel)
	{
		g_channel->SetPos(GetSet()->channel);
	}

}

void Edit_PianoWave::ShowWaveStatus()
{
	if(g_wave)
	{
		char *c="?";

		switch(GetSet()->status)
		{
		case NOTEON:
			c="Velocity";
			break;

		case NOTEOFF:
			c="Velocity Off";
			break;

		case CONTROLCHANGE:
			c="Control Change";
			break;

		case PROGRAMCHANGE:
			c="Program Change";
			break;

		case POLYPRESSURE:
			c="Poly Pressure";
			break;

		case CHANNELPRESSURE:
			c="Channel Pressure";
			break;

		case PITCHBEND:
			c="Pitchbend";
			break;
		}

		g_wave->ChangeButtonText(c);
	}
}

void Edit_PianoWave::ShowWaveControl()
{
	if(g_control)
	{
		if(GetSet()->status==CONTROLCHANGE)
		{
		}
		else
			g_control->Disable();

	}
}

PianoSettings *Edit_PianoWave::GetSet()
{
	return &mainsettings->pianosettings[set];
}

void Edit_PianoWave::Init()
{ 
	glist.SelectForm(0,0);

	int addw=INFOSIZE;

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

	glist.AddButton(-1,-1,-1,-1,"Channel",GADGETID_CHANNEL,MODE_LEFTTOMID|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	g_channel=glist.AddNumberButton(-1,-1,-1,-1,GADGETID_CHANNELNR,0,16,1,NUMBER_MIDICHANNEL,MODE_MIDTORIGHT);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"Status",GADGETID_STATUS,MODE_LEFTTOMID|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	g_wave=glist.AddButton(-1,-1,-1,-1,GADGETID_STATUSNAME,MODE_MIDTORIGHT);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"Control",GADGETID_CONTROL,MODE_LEFTTOMID|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	g_control=glist.AddButton(-1,-1,-1,-1,GADGETID_CONTROLNAME,MODE_MIDTORIGHT);
	glist.Return();

	ShowWaveChannel();
	ShowWaveStatus();
	ShowWaveControl();

	// Init Wave Events
	editor->glist.SelectForm(1,2);
	editor->editarea2=editor->glist.AddChildWindow(-1,-1,-2,-2,MODE_RIGHT|MODE_BOTTOM|MODE_SPRITE,0,&PianoEditor_WaveTrackEvents_Callback,this);
}

