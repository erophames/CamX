#include "camxgadgets.h"
#include "guigadgets.h"
#include "gui.h"
#include "object_song.h"
#include "songmain.h"
#include "languagefiles.h"
#include "editor.h"
#include "object_project.h"
#include "audiohardware.h"
#include "guiobjects.h"
#include "editdata.h"

void guiGadget_Win::SetMinWidthAndHeight(int *newwidth,int *newheight)
{
	if(win)
	{
		if(win->minheight>*newheight)
			*newheight=win->minheight;
	}
}

guiGadget::guiGadget()
{
	id=OID_BUTTON;

	menuindex=-1;
	leftmousedown=rightmousedown=false;
	bordercolour_use=false;
	useextracolour=false;
	oldfocushWnd=0;
	staticbutton=false;
	sysgadget=true;
	selected=false;
	index=0;
	text=string=0;
	object=0;
	editIndex=-1;
	camxID=0;
	justselected=false;
	skippaint=false;
	dock=0;
	childof=0;
	linkgadget=0;
	quicktext=0;

	gadgetinit=false;
	deltareturn=false;

#ifdef WIN32
	hWnd=0;
	ttWnd=0;
#endif

	mode=MODE_NORMAL;

#ifdef _DEBUG
	n[0]='G';
	n[1]='G';
	n[2]='A';
	n[3]='D';
#endif

	mouseover=disabled=getMouseMove=init=textinit=pushed=false;

	guilist=0;
	subw=subh=staticwidth=staticheight=0;
	parent=0;
	on=true;
	group=0;
	linkclickgadget=0;
	rightmousedown=false;

	tooltext=0;
	horzslider=false;
	mouseoverindex=-1;
	edittabx=-1;
}

void guiGadget::DeInit()
{
	if(ttWnd)
	{
		DestroyWindow(ttWnd);
	}

	if(tooltext)
	{
		delete tooltext;
		tooltext=0;
	}

	if(text)
	{
		delete text;
		text=0;
	}

	if(string)
	{
		delete string;
		string=0;
	}

	if(quicktext)
		delete quicktext;

	FreeMemory();
}

void guiGadget_Integer::SetInteger(int i)
{
	index=i;

	Enable();

#ifdef WIN32

	char help[NUMBERSTRINGLEN];
	char *h=mainvar->ConvertIntToChar(i,help);
	strcpy(numberstring,h);

	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)numberstring);

	// Integer->String
	//
	//SetString(mainvar->ConvertIntToChar(i,hs));
#endif	
}

void guiGadget::SetCheckBox(bool select)
{
	Enable();

	LRESULT r=SendMessage(hWnd,BM_GETCHECK,0,0);

	//TRACE ("SetCheckBox %d\n",r);

	switch(r)
	{
	case BST_CHECKED:
		if(select==false)
			PostMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
		break;

	case BST_UNCHECKED:
		if(select==true)
			PostMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
		break;

	case BST_INDETERMINATE:
		{
			int i=1;
		}
		break;

	default:
#ifdef DEBUG
		maingui->MessageBoxError(0,"SetCheckBox");
#endif

		break;
	}

	index=select;
}

void guiGadget::CheckString(char *s,bool force)
{
	if(type==GADGETTYPE_BUTTON_TEXT)
	{
		if(force==true ||
			((!text) && s) ||
			(text && s && strcmp(text,s)!=0)
			)
			ChangeButtonText(s);

		return;
	}

	if(type==GADGETTYPE_STRING)
	{
		if(force==true ||
			((!string) && s) ||
			(string && s && strcmp(string,s)!=0)
			)
			SetString(s);

		return;
	}
}

void guiGadget::SetString(char *ns)
{
	if(type==GADGETTYPE_BUTTON_TEXT)
	{
		ChangeButtonText(ns);
		return;
	}

	if(type==GADGETTYPE_STRING || type==GADGETTYPE_INTEGER)
	{
		if(string)
			delete string;

		string=mainvar->GenerateString(ns);
#ifdef WIN32
		SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)ns);
#endif

		Enable();
	}
}

void guiGadget::Delete()
{
	guilist->RemoveGadget(this);
}

int guiGadget::SetVisibleFlag()
{
	if(formchild && formchild->deactivated==true)
		return 0;

	if(x2>x && y2>y)
		return WS_VISIBLE;

	return 0;
}

void guiGadget_Time::SetTime(LONGLONG t)
{
#ifdef DEBUG
	if(t<0)
		maingui->MessageBoxError(0,"SetTime");
#endif

	if(statictimeformat==false)
	{
		// Set Time Format to Windows Format
		if(guilist && guilist->win)
			ttype=guilist->win->windowtimeformat;
	}

	t_time=t;
	DrawGadgetEnable();
}

void guiGadget_Time::Timer()
{
	Seq_Song *song=guilist && guilist->win?guilist->win->WindowSong():0;

	if(song)
	{
		switch(ttype)
		{
		case WINDOWDISPLAY_MEASURE:
			if(song->project->projectmeasureformat!=format)
				DrawGadgetEnable();
			break;
		}
	}
}

void guiGadget_Time::SetLength(OSTART spp,OSTART len)
{
#ifdef DEBUG
	if(spp<0 || len<0)
		maingui->MessageBoxError(0,"SetLength");
#endif

	if(islength==true)
	{
		if(statictimeformat==false)
		{
			// Set Time Format to Windows Format
			if(guilist && guilist->win)
				ttype=guilist->win->windowtimeformat;
		}

		t_time=spp;
		t_length=len;

		DrawGadgetEnable();
	}
}

void guiGadget_Time::SetMinTime(LONGLONG t)
{
	mintimeset=true;
	mintime=t;
}

void guiGadget_Time::SetMaxTime(LONGLONG t)
{
	maxtimeset=true;
	maxtime=t;
}

void guiGadget_Time::CheckPreCounter()
{
	if(showprecounter==true)
	{
		if(precountermode==false)
			DrawGadget();

		Seq_Song *song=guilist && guilist->win?guilist->win->WindowSong():0;

		if(song)
		{
			if(precountertodo!=mainsettings->numberofpremetronomes-song->metronome.GetPreCounterDone())
			{
				DrawGadget();
			}
		}

	}
}

void guiGadget_Time::CheckTime(LONGLONG ct,bool realtime)
{
	Enable();

	if(disabled==true || realtime==false || t_time!=ct)
		SetTime(ct);
	else
		if(ttype==WINDOWDISPLAY_SMPTE)
		{
			Seq_Song *song=guilist && guilist->win?guilist->win->WindowSong():0;

			if(song && (song->project->standardsmpte!=format || song->smpteoffset.changed==true))
				DrawGadgetEnable();
		}
		else
			if(ttype==WINDOWDISPLAY_MEASURE)
			{
				Seq_Song *song=guilist && guilist->win?guilist->win->WindowSong():0;

				if(song && 
					(song->project->projectmeasureformat!=format || song->timetrack.zoomticks !=zoom || (precountermode==true && (!(song->status&Seq_Song::STATUS_WAITPREMETRO))) ) )
					DrawGadgetEnable();
			}
}

guiGadget_Time::guiGadget_Time()
{
	type=GADGETTYPE_TIME;

	islength=false;
	precountermode=false;
	precounter=-99;

	mintimeset=maxtimeset=false;
	pairedwith=0;
	showsamples=false;
	statictimeformat=false;
	showprecounter=false;
	minusclicked=false;

}

void guiGadget_Time::Disabled()
{
	samples=-1;
	t_time=-1;
	t_length=-1;
}

void guiGadget_Time::DrawGadgetEx()
{
	Seq_Song *song=guilist && guilist->win?guilist->win->WindowSong():0;

	int sy2=GetHeight()-maingui->buttonsizeaddy/2;

	switch(ttype)
	{
	case WINDOWDISPLAY_SMPTEOFFSET:
		if(song)
		{
			Seq_Pos_Offset *offset=&song->smpteoffset;

			int tx=1+2*guilist->win->bitmap.pref_minuswidth; 

			UBYTE r,g,b;

			if(offset->minus==true)
			{
				maingui->colourtable.GetRGB(COLOUR_WHITE,&r,&g,&b);
				SetTextColor(gbitmap.hDC, RGB(r, g, b));
			}
			else
			{
				maingui->colourtable.GetRGB(COLOUR_GREY_DARK,&r,&g,&b);
				SetTextColor(gbitmap.hDC, RGB(r, g, b));
			}

			gbitmap.guiDrawTextFontY(1,sy2,tx," - ");

			minusx=1;
			minusx2=tx-1;

			maingui->colourtable.GetRGB(COLOUR_WHITE,&r,&g,&b);
			SetTextColor(gbitmap.hDC, RGB(r, g, b));

			char nr[NUMBERSTRINGLEN];

			for(int i=0;i<4;i++) // h,min,sec,frame  no qf
			{
				int tox=tx;

				if(i==0)
					tox+=gbitmap.GetTextWidth("WWW");
				else
					tox+=guilist->win->bitmap.pref_time[i];

				if(editIndex==i && getMouseMove==true)
				{
					gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEEDITINDEX);
				}
				else
				{
					if(mouseoverindex==i && mouseover==true)
						gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEOVERINDEX);
				}

				switch(i)
				{
				case 0:
					gbitmap.guiDrawTextFontY(tx,sy2,tox,mainvar->ConvertLongLongToChar(offset->h,nr));
					break;

				case 1:
					gbitmap.guiDrawTextFontY(tx,sy2,tox,mainvar->ConvertLongLongToChar(offset->m,nr));
					break;

				case 2:
					gbitmap.guiDrawTextFontY(tx,sy2,tox,mainvar->ConvertLongLongToChar(offset->sec,nr));
					break;

				case 3:
					gbitmap.guiDrawTextFontY(tx,sy2,tox,mainvar->ConvertLongLongToChar(offset->frame,nr));

					TRACE ("Frame %d\n",offset->frame);
					break;
				}

				if(i<3)
				{
					int zx=tox;
					zx-=guilist->win->bitmap.pref_space;
					gbitmap.guiDrawTextFontY(zx,sy2,tox-1,":");
				}

				subx[i]=tx;
				subx2[i]=tox-1;

				tx=tox+1;
			}

		}
		break;

	case WINDOWDISPLAY_MEASURE:
		{
			UBYTE r,g,b;

			int col;

			if(mode&MODE_INFO)
				col=COLOUR_TEXT_INFO;
			else
				col=COLOUR_TEXT;

			maingui->colourtable.GetRGB(col,&r,&g,&b);
			SetTextColor(gbitmap.hDC, RGB(r, g, b));

			if(!song)
			{
				gbitmap.guiDrawText(0,sy2,GetWidth(),"-");
			}
			else
			{
				if((song->status&Seq_Song::STATUS_WAITPREMETRO) && showprecounter==true)
				{	
					precountertodo=mainsettings->numberofpremetronomes-song->metronome.GetPreCounterDone();

					if(precountertodo>0)
					{
						char help[NUMBERSTRINGLEN];

						if(char *h=mainvar->GenerateString(Cxs[CXS_PRECOUNTER],":",mainvar->ConvertIntToChar(precountertodo,help)) )
						{
							gbitmap.guiFillRect(COLOUR_BLACK);

							UBYTE r,g,b;
							maingui->colourtable.GetRGB(COLOUR_YELLOW,&r,&g,&b);
							SetTextColor(gbitmap.hDC, RGB(r, g, b));
							gbitmap.guiDrawTextFontY(3,sy2,GetWidth()-1,h);

							//TextOut(bitmap->hDC, x+1, y+1,h, strlen(h));

							delete h;
						}

						precountermode=true;

						return;
					}

				}// PreCounter

				if(precountermode==true)
					init=false;

				precountermode=false;
				format=song->project->projectmeasureformat;
				zoom=song->timetrack.zoomticks;

				if(t_time==-1)
				{
					gbitmap.guiDrawTextFontY(0,sy2,GetWidth(),"-");
					init=false;
					return;
				}

				init=false;

				song->timetrack.CreateTimeString(&timestring,islength==true?t_length:t_time,Seq_Pos::POSMODE_NORMAL);

				/*
				PM_1111,
				PM_1p1p1p1,
				PM_1110,
				PM_1p1p1p0,
				PM_11_1,
				PM_1p1p_1,
				PM_11_0,
				PM_1p1p_0
				*/

				int tx=1+guilist->win->bitmap.pref_minuswidth; 

				for(int i=0;i<timestring.index;i++)
				{
					int tox=tx+guilist->win->bitmap.pref_time[i];

					if(tox>x2 || i==timestring.index-1)
						tox=x2;

					if(strcmp(" - ",timestring.strings[i])!=0){

						if(subvalue[i]!=timestring.pos.pos[i] || init==false)
						{
							if(editIndex==i && getMouseMove==true)
							{
								gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEEDITINDEX);
							}
							else
							{
								if(mouseoverindex==i  && mouseover==true)
									gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEOVERINDEX);
							}

							gbitmap.guiDrawTextFontY(tx,sy2,tox-1,timestring.strings[i]);

							subx[i]=tx;
							subx2[i]=tox;
							subvalue[i]=timestring.pos.pos[i];

							if(i<timestring.index-1)
							{
								switch(format)
								{
								case PM_1p1p1p1:
								case PM_1p1p1p0:
								case PM_1p1p_1:
								case PM_1p1p_0:
									{
										int zx=tox;
										zx-=guilist->win->bitmap.pref_space;
										gbitmap.guiDrawTextFontY(zx,sy2,tox-1,".");
									}
									break;
								}
							}
						}

						tx=tox;
					}
					else
					{
						subx[i]=-1;
						subvalue[i]=1; // 1.
					}

					subx2[i]=tox-1;	
				}

				init=true;
			}
		}
		break;

	case WINDOWDISPLAY_SECONDS:
		{
			if(!song)
			{
				gbitmap.guiFillRect(COLOUR_BLACK_LIGHT);
			}
			else
			{
				if(t_time==-1)
				{
					//gbitmap.guiFillRectX0(0,GetWidth(),GetHeight(),COLOUR_BLACK_LIGHT);
					init=false;
					return;
				}

				init=false;
				//gbitmap.guiFillRectX0(0,GetWidth(),GetHeight(),mouseover==true?COLOUR_BLUE_LIGHT:COLOUR_BLACK_LIGHT);

				song->timetrack.CreateTimeString(&timestring,islength==true?t_length:t_time,Seq_Pos::POSMODE_TIME);

				UBYTE r,g,b;
				maingui->colourtable.GetRGB(COLOUR_TIME,&r,&g,&b);
				SetTextColor(gbitmap.hDC, RGB(r, g, b));
				//	SetBkMode(tobitmap->hDC, TRANSPARENT); // Transparent Text etc...

				int tx=1+guilist->win->bitmap.pref_minuswidth; 

				for(int i=0;i<timestring.index;i++)
				{
					int tox=tx+guilist->win->bitmap.pref_time[i];

					if(tox<=x2)
					{
						if(i==timestring.index-1)
							tox=x2;

						if(subvalue[i]!=timestring.pos.pos[i] || init==false)
						{
							if(editIndex==i && getMouseMove==true)
							{
								gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEEDITINDEX);
							}
							else
							{
								if(mouseoverindex==i  && mouseover==true)
									gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEOVERINDEX);
							}

							subx[i]=tx;
							subx2[i]=tox;
							subvalue[i]=timestring.pos.pos[i];

							gbitmap.guiDrawTextFontY(tx,sy2,tox-1,timestring.strings[i]);

							if(i<timestring.index)
							{
								int zx=tox;
								zx-=guilist->win->bitmap.pref_space;
								gbitmap.guiDrawTextFontY(zx,sy2,tox-1,i==3?";":":");
							}
						}
					}
					else
					{
						subx[i]=-1;
						subvalue[i]=1; // 1.
					}

					subx2[i]=tox-1;
					tx=tox;
				}

				init=true;
			}
		}
		break;

	case WINDOWDISPLAY_SMPTE:
		{
			if(!song)
			{
				gbitmap.guiFillRect(COLOUR_BLACK_LIGHT);
			}
			else
			{
				if(t_time==-1)
				{
					//gbitmap.guiFillRectX0(0,GetWidth(),GetHeight(),COLOUR_BLACK_LIGHT);
					init=false;
					return;
				}

				init=false;

				format=guilist->win->WindowSong()->project->standardsmpte;

				song->timetrack.CreateTimeString(&timestring,islength==true?t_length:t_time,format);

				int tx=1+guilist->win->bitmap.pref_minuswidth; 

				//	gbitmap.guiDrawTextFontY(1,sy2,guilist->win->bitmap.pref_minuswidth,"-");

				/*
				minusx=1;
				minusx2=tx;

				if(timestring.pos.minus==true)
				{
				gbitmap.SetTextColour(COLOUR_WHITE);
				gbitmap.guiDrawTextFontY(minusx,sy2,minusx2,"-");
				}
				*/

				bool set=false;

				for(int i=0;i<timestring.index;i++)
				{
					if(timestring.pos.pos[i]>0 || i==timestring.index-1)
						set=true;

					gbitmap.SetTextColour(set==true?COLOUR_SMPTE:COLOUR_SMPTELIGHT);

					int tox=tx+guilist->win->bitmap.pref_smpte[i];

					if(tox<=x2)
					{
						if(i==timestring.index-1)
							tox=x2;

						if(subvalue[i]!=timestring.pos.pos[i] || init==false)
						{
							if(editIndex==i && getMouseMove==true)
							{
								gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEEDITINDEX);
								gbitmap.SetTextColour(COLOUR_WHITE);
							}
							else
							{
								if(mouseoverindex==i && mouseover==true)
								{
									gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEOVERINDEX);
									gbitmap.SetTextColour(COLOUR_WHITE);
								}
							}

							subx[i]=tx;
							subx2[i]=tox;
							subvalue[i]=timestring.pos.pos[i];

							gbitmap.guiDrawTextFontY(tx,sy2,tox-1,timestring.strings[i]);

							if(i<timestring.index-1)
							{
								int zx=tox;
								zx-=guilist->win->bitmap.pref_space;
								gbitmap.guiDrawTextFontY(zx,sy2,tox-1,i==3?";":":");
							}
						}
					}
					else
					{
						subx[i]=-1;
						subvalue[i]=1; // 1.
					}

					subx2[i]=tox-1;
					tx=tox;
				}

				init=true;
			}
		}
		break;

	case WINDOWDISPLAY_SAMPLES:
		{
			Seq_Song *song=guilist->win->WindowSong();

			if(!song)
			{
				gbitmap.guiFillRect(COLOUR_BLACK_LIGHT);
			}
			else
			{
				int tx=1+guilist->win->bitmap.pref_minuswidth;

				int tox=GetX2()-2;

				if(editIndex==0 && getMouseMove==true)
				{
					gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEEDITINDEX);
				}
				else
				{
					if(mouseover==true)
						gbitmap.guiFillRect(tx,0,tox,GetHeight(),COLOUR_MOUSEOVERINDEX);
				}

				UBYTE r,g,b;
				maingui->colourtable.GetRGB(COLOUR_SAMPLES,&r,&g,&b);
				SetTextColor(gbitmap.hDC, RGB(r, g, b));

				if(showsamples==false)
				{
					if(islength==true)
					{
						LONGLONG s1=song->timetrack.ConvertTicksToTempoSamples(t_time);
						LONGLONG s2=song->timetrack.ConvertTicksToTempoSamples(t_time+t_length);

						samples=s2-s1;
					}
					else
						samples=song->timetrack.ConvertTicksToTempoSamples(t_time);
				}

				char h[NUMBERSTRINGLEN];

				char *s=mainvar->ConvertLongLongToChar(samples,h);

				gbitmap.guiDrawTextFontY(tx,sy2,tox,s);
			}

		}
		break;
	}
}

void guiGadget_Time::DoubleClickedLeftMouse()
{
	mouseoverindex=-1;

	CheckMouseOverStatus(GetMouseX(),GetMouseY());

	if(mouseoverindex==-1)
		return;

	switch(ttype)
	{
	case WINDOWDISPLAY_SMPTEOFFSET:
		{
		}
		break;

	case WINDOWDISPLAY_MEASURE:
		if(mouseoverindex>=0 && guilist->win->WindowSong())
		{
			if(EditData *edit=new EditData)
			{
				edit->song=guilist->win->WindowSong();
				edit->win=guilist->win;

				edit->x=x+subx[mouseoverindex];
				edit->y=y;
				edit->width=subx2[mouseoverindex]-subx[mouseoverindex];

				edit->title=Cxs[CXS_EDIT];
				edit->deletename=false;
				edit->id=-1;
				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->helpobject=this;

				switch(editdataindex=mouseoverindex)
				{
				case 0: // Measure
					edit->from=1;
					edit->to=99999;
					edit->value=subvalue[0];
					break;

				case 1: // Beat
					{
						Seq_Signature *sig=edit->song->timetrack.FindSignatureBefore(t_time);
						edit->from=1;
						edit->to=sig->dn;
						edit->value=subvalue[1];
					}
					break;

				case 2: // Zoom
					{
						Seq_Signature *sig=edit->song->timetrack.FindSignatureBefore(t_time);
						edit->from=1;
						edit->to=sig->dn_ticks/edit->song->timetrack.zoomticks;
						edit->value=subvalue[2];
					}
					break;

				case 3: // Ticks
					{
						Seq_Signature *sig=edit->song->timetrack.FindSignatureBefore(t_time);
						edit->from=1;
						edit->to=mainaudio->ConvertInternRateToPPQ(edit->song->timetrack.zoomticks);
						edit->value=subvalue[3];
					}
					break;
				}

				maingui->EditDataValue(edit);
			}
		}
		break;

	case WINDOWDISPLAY_SMPTE:
		if(mouseoverindex>=0 && guilist->win->WindowSong())
		{
			if(EditData *edit=new EditData)
			{
				edit->song=guilist->win->WindowSong();
				edit->win=guilist->win;

				edit->x=x+subx[mouseoverindex];
				edit->y=y;
				edit->width=subx2[mouseoverindex]-subx[mouseoverindex];

				edit->title=Cxs[CXS_EDIT];
				edit->deletename=false;
				edit->id=-1;
				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->helpobject=this;

				switch(editdataindex=mouseoverindex)
				{
				case 0: // Hour
					edit->from=0;
					edit->to=999;
					edit->value=subvalue[0];
					break;

				case 1: // Min
					{
						edit->from=0;
						edit->to=59;
						edit->value=subvalue[1];
					}
					break;

				case 2: // Sek
					{
						edit->from=0;
						edit->to=59;
						edit->value=subvalue[2];
					}
					break;

				case 3: // Frame
					{
						edit->from=0;
						edit->to=SMPTE_FPS[guilist->win->WindowSong()->project->standardsmpte];
						edit->value=subvalue[3];
					}
					break;

				case 4: // Q Frame
					{
						edit->from=0;
						edit->to=3;
						edit->value=subvalue[4];
					}
					break;
				}

				maingui->EditDataValue(edit);
			}
		}
		break;

	case WINDOWDISPLAY_SAMPLES:

		break;
	}
}

void guiGadget_Time::MouseWheel(int delta)
{
	if(mouseoverindex>=0)
	{
		deltay=-delta;
		editIndex=mouseoverindex;
		DeltaY();
	}
}

void guiGadget_Time::ChangeEditData(OSTART nv,bool refreshgui)
{
	Seq_Pos *pos=0;

	Seq_Pos pos_norm(Seq_Pos::POSMODE_NORMAL);
	Seq_Pos pos_smpte(guilist->win->WindowSong()->project->standardsmpte);

	switch(ttype)
	{
	case WINDOWDISPLAY_MEASURE:
		{
			pos=&pos_norm;
		}
		break;

	case WINDOWDISPLAY_SMPTE:
		{
			pos=&pos_smpte;
		}
		break;
	}

	if(pos)
	{
		guilist->win->WindowSong()->timetrack.ConvertTicksToPos(t_time,pos);

		pos->pos[editdataindex]=nv;

		OSTART oldtime=t_time;

		t_time=guilist->win->WindowSong()->timetrack.ConvertPosToTicks(pos);

		guilist->win->Gadget(this);

		if(oldtime!=t_time) // ->Gadget can change time!
			DrawGadget();
	}
}

void guiGadget_Time::ChangeType(int nt)
{
	if(ttype!=nt)
	{
		ttype=nt;
		mouseoverindex=editIndex=-1;
		DrawGadget();

		if(pairedwith)
		{
			pairedwith->ttype=nt;
			pairedwith->mouseoverindex=pairedwith->editIndex=-1;
			pairedwith->DrawGadget();
		}
	}
}


void guiGadget_Time::CheckMouseOverStatus(int mx,int my)
{
	minusclicked=false;

	if(Seq_Song *song=guilist->win->WindowSong())
	{
		int toi;

		switch(ttype)
		{
		case WINDOWDISPLAY_SMPTEOFFSET:
			{
				toi=4;

				if(minusx<=mx && minusx2>=mx)
				{
					minusclicked=true;
					return;
				}

			}
			break;

		case WINDOWDISPLAY_MEASURE:
			toi=4;
			break;

		case WINDOWDISPLAY_SECONDS:
		case WINDOWDISPLAY_SMPTE:
			toi=5;
			break;

		case WINDOWDISPLAY_SAMPLES:
			mouseoverindex=0;
			return;
			break;

		default:
			return;
		}

		for(int i=0;i<toi;i++)
		{
			if(subx[i]!=-1 && GetMouseX()>=subx[i] && mx<subx2[i])
			{
				if(i!=mouseoverindex)
				{
					mouseoverindex=i;
					DrawGadget();
				}

				return;
			}
		}

		if(mouseoverindex>=0)
		{
			mouseoverindex=-1;
			DrawGadget();
		}
	}
}

void guiGadget_Time::MouseMoveLeftDown()
{
	if(getMouseMove==true && mouseoverindex>=0)
	{
		InitDelta();

		if(deltay && DeltaY()==true)
		{
			//if(hdy>=hdx)
			startmousey=GetMouseY();

			//if(hdx>=hdy)
			startmousex=GetMouseX();
		}
	}
}


void guiGadget_Time::RightMouseUp()
{
	TRACE ("guiGadget_Time RightMouseUp \n");

	switch(ttype)
	{
	case WINDOWDISPLAY_MEASURE:
	case WINDOWDISPLAY_SMPTE:
	case WINDOWDISPLAY_SAMPLES:
		{
			guilist->win->CreateTimeTypePopup(this);
		}
		break;
	}
}

void guiGadget_Time::LeftMouseUp()
{
	if(editIndex!=-1)
		EndMouseMove();
}

void guiGadget_Time::LeftMouseDown()
{
	mouseoverindex=-1;

	CheckMouseOverStatus(GetMouseX(),GetMouseY());

	if(ttype==WINDOWDISPLAY_SMPTEOFFSET && minusclicked==true)
	{
		if(guilist->win->WindowSong())
		{
			// Toggle Minus
			guilist->win->WindowSong()->smpteoffset.minus=guilist->win->WindowSong()->smpteoffset.minus==true?false:true;

			DrawGadget();

			guilist->win->WindowSong()->smpteoffset.changed=true;
			maingui->RefreshSMPTE(guilist->win->WindowSong()->project);
			guilist->win->WindowSong()->smpteoffset.changed=false;
		}

		minusclicked=false;
		return;
	}

	if(mouseoverindex==-1)
		return;

	InitGetMouseMove();
	editIndex=mouseoverindex;

	switch(ttype)
	{
	case WINDOWDISPLAY_SMPTEOFFSET:
		{

		}
		break;

	case WINDOWDISPLAY_MEASURE:
		if(mouseoverindex>=0 && guilist->win->WindowSong())
		{
			if(maingui->GetShiftKey()==true)
			{
				Seq_Pos spos(Seq_Pos::POSMODE_NORMAL);
				guilist->win->WindowSong()->timetrack.ConvertTicksToPos(t_time,&spos);

				switch(mouseoverindex)
				{
				case 0:
				case 1:
					spos.pos[1]=spos.pos[2]=spos.pos[3]=1;
					break;

				case 2:
					spos.pos[2]=spos.pos[3]=1;
					break;

				case 3:
					spos.pos[3]=1;
					break;
				}

				OSTART vtime=guilist->win->WindowSong()->timetrack.ConvertPosToTicks(&spos);

				if(vtime!=t_time && vtime>=0)
				{
					OSTART oldtime=t_time;

					t_time=vtime;
					guilist->win->Gadget(this);

					if(oldtime!=t_time) // ->Gadget can change time!
					{
						DrawGadget();
						return;
					}
				}
			}
		}
		break;

	case WINDOWDISPLAY_SMPTE:
		if(mouseoverindex>=0 && guilist->win->WindowSong())
		{
			if(maingui->GetShiftKey()==true)
			{
				Seq_Pos spos(guilist->win->WindowSong()->project->standardsmpte);
				guilist->win->WindowSong()->timetrack.ConvertTicksToPos(t_time,&spos);

				switch(mouseoverindex)
				{
				case 0:
					spos.pos[1]=spos.pos[2]=spos.pos[3]=spos.pos[4]=0;
					break;

				case 1:
					spos.pos[2]=spos.pos[3]=spos.pos[4]=0;
					break;

				case 2:
					spos.pos[3]=spos.pos[4]=0;
					break;

				case 3:
				case 4:
					spos.pos[4]=0;
					break;
				}

				OSTART vtime=guilist->win->WindowSong()->timetrack.ConvertPosToTicks(&spos);

				if(vtime!=t_time && vtime>=0)
				{
					OSTART oldtime=t_time;

					t_time=vtime;
					guilist->win->Gadget(this);

					if(oldtime!=t_time) // ->Gadget can change time!
					{
						DrawGadget();
						return;
					}
				}
			}
		}
		break;
	}

	DrawGadget();
}

LONGLONG guiGadget_Time::GetTime()
{
	return islength==true?t_length:t_time;
}

bool guiGadget_Time::DeltaY()
{
	switch(ttype)
	{
	case WINDOWDISPLAY_SMPTEOFFSET:
		{
			if(guilist->win->WindowSong())
			{
				Seq_Pos spos(guilist->win->WindowSong()->project->standardsmpte);

				Seq_Pos_Offset *offset=&guilist->win->WindowSong()->smpteoffset;

				spos.pos[0]=offset->h;
				spos.pos[1]=offset->m;
				spos.pos[2]=offset->sec;
				spos.pos[3]=offset->frame;
				spos.pos[4]=0; // no qf

				spos.index=4;
				spos.offset=offset;
				spos.song=guilist->win->WindowSong();

				switch(editIndex)
				{
				case 0:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddHour(deltay);
					}
					break;

				case 1:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddMin(deltay);
					}
					break;

				case 2:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddSec(deltay);
					}
					break;

				case 3:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddFrame(deltay);
					}
					break;
				}

				if(offset->h!=spos.pos[0] ||
					offset->m!=spos.pos[1] ||
					offset->sec!=spos.pos[2] ||
					offset->frame!=spos.pos[3])
				{
					offset->h=spos.pos[0];
					offset->m=spos.pos[1];
					offset->sec=spos.pos[2];
					offset->frame=spos.pos[3];

					//guilist->win->Gadget(this);

					DrawGadget();

					offset->changed=true;
					maingui->RefreshSMPTE(guilist->win->WindowSong()->project);
					offset->changed=false;
				}
			}
		}
		break;

	case WINDOWDISPLAY_SMPTE:
		{
			if(guilist->win->WindowSong())
			{
				Seq_Pos spos(guilist->win->WindowSong()->project->standardsmpte);
				spos.song=guilist->win->WindowSong();
				spos.offset=0;

				guilist->win->WindowSong()->timetrack.ConvertTicksToPos(t_time,&spos);

				switch(editIndex)
				{
				case 0:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddHour(deltay);
					}
					break;

				case 1:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddMin(deltay);
					}
					break;

				case 2:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddSec(deltay);
					}
					break;

				case 3:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddFrame(deltay);
					}
					break;

				case 4:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddQuarterFrame(deltay);
					}
					break;
				}

				OSTART vtime=guilist->win->WindowSong()->timetrack.ConvertPosToTicks(&spos);

				if(mintimeset==true && vtime<mintime)
					vtime=mintime;

				if(maxtimeset==true && vtime>maxtime)
					vtime=maxtime;

				if(vtime!=t_time && vtime>=0)
				{
					t_time=vtime;
					guilist->win->Gadget(this);
					DrawGadget();
				}
			}
		}
		break;

	case WINDOWDISPLAY_MEASURE:
		{
			if(guilist->win->WindowSong())
			{
				Seq_Pos spos(Seq_Pos::POSMODE_NORMAL);

				guilist->win->WindowSong()->timetrack.ConvertTicksToPos(t_time,&spos);

				if(spos.index==3 && editIndex==2)
					editIndex=3; // 1.1.5, no zoom

				switch(editIndex)
				{
				case 0:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddMeasure(deltay);
					}
					break;

				case 1:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;
						spos.AddBeat(deltay);
					}
					break;

				case 2:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;

						spos.AddZoomTicks(deltay);
					}
					break;

				case 3:
					{
						if(deltay>1)deltay=1;
						if(deltay<-1)deltay=-1;

						spos.AddTicks(deltay);
					}
					break;
				}

				OSTART vtime=guilist->win->WindowSong()->timetrack.ConvertPosToTicks(&spos);

				if(mintimeset==true && vtime<mintime)
					vtime=mintime;

				if(maxtimeset==true && vtime>maxtime)
					vtime=maxtime;

				if(vtime!=t_time && vtime>=0)
				{
					OSTART oldtime=t_time;

					t_time=vtime;
					guilist->win->Gadget(this);

					if(oldtime!=t_time) // ->Gadget can change time!
						DrawGadget();
				}
			}
		}
		break;

	case WINDOWDISPLAY_SAMPLES:
		if(Seq_Song *song=guilist->win->WindowSong())
		{
			samples+=deltay;

			OSTART nt=song->timetrack.ConvertSamplesToTicks(samples);

			if(mintimeset==true && nt<mintime)
				nt=mintime;

			if(maxtimeset==true && nt>maxtime)
				nt=maxtime;

			if(nt!=t_time && nt>=0)
			{
				OSTART oldtime=t_time;

				t_time=nt;

				guilist->win->Gadget(this);

				if(oldtime!=t_time) // ->Gadget can change time!
				{
					DrawGadget();
					return true;
				}
			}

			showsamples=true;
			DrawGadget();
			showsamples=false;

		}
		break;
	}

	return true;
}

void guiGadget_Numerator::ShowSignature(Seq_Signature *sig)
{
	// Num
	char htext[NUMBERSTRINGLEN];
	htext[0]=0;

	ChangeButtonText(mainvar->ConvertIntToChar(sig->nn,htext));

	if(dn)
		dn->ChangeButtonText(sig->GetTickString());
}

void guiGadget_Numerator::Enable()
{
	guiGadget::Enable();
	if(dn)
		dn->Enable();
}

void guiGadget_Numerator::Disable()
{
	guiGadget::Disable();
	if(dn)
		dn->Disable();
}

char *guiGadget::GetString(int length)
{
	if(type==GADGETTYPE_STRING)
	{
		return string;

		/*
		char *buffer=mainvar->GetStringBuffer(length);

		if(buffer)
		#ifdef WIN32
		SendMessage(hWnd, WM_GETTEXT, length, (int)buffer);
		#endif		
		return buffer;
		*/
	}

	return 0;
}

void guiGadget_ListBox::SetListBoxSelection(int i)
{
	if(i>=0)
	{
#ifdef WIN32
		SendMessage(hWnd, LB_SETCURSEL, i, 0);
#endif		
		index=i;
	}
}

void guiGadget::SetCycleSelection(int i)
{
	if(type==GADGETTYPE_CYCLE && i>=0)
	{
#ifdef WIN32
		SendMessage(hWnd, CB_SETCURSEL, i, 0);
#endif
		index=i;
	}
}

void guiGadget_ListBox::ClearAllListBoxButtons()
{
	guiListBoxButton *lb=(guiListBoxButton *)listboxstrings.GetRoot();
	while(lb)
	{
		if(lb->string)
			delete string;

		lb=(guiListBoxButton *)lb->next;
	}

	listboxstrings.DeleteAllO();
}

void guiGadget_ListBox::CalcScrollWidth()
{
	int w=0;

	guiListBoxButton *lb=(guiListBoxButton *)listboxstrings.GetRoot();
	while(lb)
	{
		if(lb->string)
		{
			int sl=guilist->win->bitmap.GetTextWidth(lb->string);

			if(sl>w)
				w=sl;
		}

		lb=(guiListBoxButton *)lb->next;
	}

#ifdef WIN32
	SendMessage(hWnd,LB_SETHORIZONTALEXTENT,w+2,0);
#endif
}

guiListBoxButton::guiListBoxButton(int lid,char *s)
{
	id=lid;
	if(string=s)
		string=mainvar->GenerateString(s);
}

void guiGadget_ListBox::AddStringToListBox(char *string)
{
	if(!string)string="";

	if(guiListBoxButton *lb=new guiListBoxButton(-1,string))
	{
		listboxstrings.AddEndO(lb);
#ifdef WIN32
		SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)string);
#endif
	}
}

void guiGadget_ListBox::AddStringToListBox(char *string,int id)
{
	if(!string)
		string="";

	if(guiListBoxButton *lb=new guiListBoxButton(id,string))
	{
		listboxstrings.AddEndO(lb);

#ifdef WIN32
		SendMessage(hWnd,LB_ADDSTRING, 0, (LPARAM)string);
#endif

	}
}

void guiGadget_ListBox::AddNumberToListBox(int number)
{
	char nrs[NUMBERSTRINGLEN],*nrstring=mainvar->ConvertIntToChar(number,nrs);

#ifdef WIN32
	SendMessage(hWnd,LB_ADDSTRING, 0, (LPARAM)nrstring);
#endif
}

void guiGadget::ClearCycle()
{
	if(type==GADGETTYPE_CYCLE)
	{
#ifdef WIN32
		SendMessage(hWnd, CB_RESETCONTENT, 0, 0);
#endif
	}
}

void guiGadget_ListBox::ClearListBox()
{
	//if(type==GADGETTYPE_LISTVIEW)
	{
#ifdef WIN32
		SendMessage(hWnd, LB_RESETCONTENT, 0, 0);
#endif
	}
}

void guiGadget::AddTooltip(char *text)
{
	if(!text)
		return;

	tooltext=mainvar->GenerateString(text);

	/*
	if((!ttWnd) && tooltext)
	{
	// Create a ToolTip.
	ttWnd = CreateWindowEx(WS_EX_TOPMOST| WS_EX_TOOLWINDOW,
	TOOLTIPS_CLASS, NULL,
	WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP|TTS_BALLOON,		
	CW_USEDEFAULT, CW_USEDEFAULT,
	CW_USEDEFAULT, CW_USEDEFAULT,
	hWnd, NULL, maingui->hInst,NULL);

	if(ttWnd)
	{
	SetWindowPos(ttWnd, HWND_TOPMOST,
	0, 0, 0, 0,
	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Set up "tool" information.
	// In this case, the "tool" is the entire parent window.
	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hWnd;
	ti.hinst = maingui->hInst;
	ti.lpszText = tooltext;
	GetClientRect (hWnd, &ti.rect);

	// Associate the ToolTip with the "tool" window.
	SendMessage(ttWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
	}
	}
	*/


}

void guiGadget::AddStringToCycle(char *string)
{
	if(type==GADGETTYPE_CYCLE)
	{
		if(!string)
			string="?";

#ifdef WIN32
		SendMessage(hWnd,CB_ADDSTRING, 0, (LPARAM)string);
#endif
	}
}

void guiGadget::AddNumberToCycle(int number)
{
	if(type==GADGETTYPE_CYCLE)
	{
		char nrs[NUMBERSTRINGLEN],*nrstring=mainvar->ConvertIntToChar(number,nrs);

#ifdef WIN32
		SendMessage(hWnd,CB_ADDSTRING, 0, (LPARAM)nrstring);
#endif
	}
}

void guiGadget::AddItemToTree(char *text)
{ 
	HTREEITEM parent=0,brother=0;

	if(text)
	{
		TVITEM tvi;
		TVINSERTSTRUCT tvins; 
		HTREEITEM hItem; 

		tvi.mask = TVIF_TEXT;
		tvi.pszText = text; 
		tvi.cchTextMax = strlen(text);  

		// die Insert-Struktur wird vorbereitet
		tvins.item = tvi;

		if (brother) {
			tvins.hInsertAfter = brother;
		}
		else
		{
			tvins.hInsertAfter = TVI_FIRST;
		}
		if (parent==0) {
			tvins.hParent = TVI_ROOT;
		} else {
			tvins.hParent = parent;
		}

		// Nachricht zum Einbinden an das Kontrollelement versenden
		hItem = (HTREEITEM)SendMessage(this->hWnd, TVM_INSERTITEM, 0, (LPARAM)&tvins);  
		//return hItem;
	}

	// return 0;
} 

void guiGadget::AddMIDIChannelsStrings()
{
	if(type==GADGETTYPE_CYCLE)
	{
		for(int i=1;i<=16;i++)
			AddNumberToCycle(i);
	}
}

void guiGadget::AddMIDIKeysStrings(Seq_Song *song)
{
	if(type==GADGETTYPE_CYCLE)
	{
		for(int i=0;i<127;i++)
			AddStringToCycle(maingui->ByteToKeyString(song,i));
	}
}

void guiGadget::AddMIDIRange(int from,int to)
{
	if(type==GADGETTYPE_CYCLE)
	{
		for(int i=from;i<=to;i++)
			AddNumberToCycle(i);
	}
}

void guiGadget_Slider::ChangeSlider(int newpos)
{
	if(newpos!=pos)
	{
		pos=newpos;

#ifdef WIN32
		if(hWnd)
			SetScrollPos(hWnd,SB_CTL,pos,true);
#endif
	}
}

void guiGadget_Slider::ChangeSlider(OListCoos *ol,int page)
{
	int newfrom=0,newto,newpos;

	if(ol->type==COOSTYPE_VERT)
	{
		newto=(ol->height-ol->guiheight)+page;
		newpos=ol->starty;
	}
	else
	{
		newto=(ol->zoomwidth-ol->guiwidth)+page;
		newpos=ol->startx;
	}

	if(newto<0)newto=0;

	ChangeSlider(newfrom,newto,newpos,page);
}

void guiGadget_Slider::ChangeSlider(int newfrom,int newto,int newpos,int newpage)
{
	if(newfrom!=from || to!=newto || pos!=newpos || page!=newpage)
	{
		bool newrange;

		if(from!=newfrom || to!=newto)
			newrange=true;
		else
			newrange=false;

		from=newfrom;
		to=newto;
		pos=newpos;
		page=newpage;

		//if(type==GADGETTYPE_VOLUME)
		//	DrawGadget();
		//else

#ifdef WIN32
		if(hWnd && from!=to)
		{
			EnableWindow(hWnd,TRUE);

			SCROLLINFO sif;
			sif.cbSize=sizeof(SCROLLINFO);
			sif.fMask=newrange?SIF_RANGE|SIF_PAGE|SIF_POS:SIF_POS|SIF_PAGE;
			sif.nPos=newpos;
			sif.nMin=newfrom;
			sif.nMax=newto;
			sif.nPage=newpage;

			SetScrollInfo(hWnd,SB_CTL,&sif,TRUE);
			//SetScrollRange(hWnd,SB_CTL,newfrom,newto,false);
			//SetScrollPos(hWnd,SB_CTL,pos,true);
			return;
		}
#endif
	}

#ifdef WIN32
	if(hWnd)
		EnableWindow(hWnd,from==to?false:true);
#endif
}

void guiGadget_Slider::DeltaY(int deltay)
{
	int p=pos;

	p+=deltay*page;

	if(p<from)
		p=from;
	else
		if(p>to)
			p=to;

	ChangeSlider(p);
	guilist->win->Gadget(this);
}

void guiGadget_Slider::ChangeSliderPage(int newpage)
{
	page=newpage;
}

guiGadget *guiGadgetList::AddTree(int x,int y,int x2,int y2,int gadgetID)
{
#ifdef OLDIE
	if(x2<x+20 || y2<y+2*maingui->GetFontSizeY_Sub())return 0;

	if(y2>=win->height)y2=win->height-1;
	if(x2>=win->width)x2=win->width-1;

	if(x>=0 && y>=0 && x2>x && y2>y)
	{
		if(guiGadget *s=new guiGadget)
		{	
			s->x=x;
			s->x2=x2;
			s->y=y;
			s->y2=y2;
			s->type=GADGETTYPE_TREE;

			InitCommonControls();

			AddGadget(s);

			s->hWnd = CreateWindowEx(0, WC_TREEVIEW , "Tree", 
				WS_VISIBLE|WS_CHILD|WS_BORDER| TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS,
				x, y, x2, y2, // Positionen auf dem Elternfenster
				win->hWnd, 0, maingui->hInst, 0);


			return s;
		}
	}

	if(guiGadget *g=win->gadgetlists.gadgets[gadgetID])
		DestroyWindow(g->hWnd);

	win->gadgetlists.gadgets[gadgetID]=0;
#endif

	return 0;
}

guiGadget_Integer *guiGadgetList::AddInteger(int x,int y,int w,int h,int gadgetID,char *text,int integer,char *tool)
{
	if(guiGadget_Integer *g=new guiGadget_Integer)
	{
		InitXY(g,&y,&w,&h);

		h+=3;

		int lx=GetLX();
		int ly=GetLY();

		if(text)
		{
			if(g->gtext=new guiGadget) // Add Text
			{
				g->gtext->x=lx;
				g->gtext->x2=g->gtext->x+80;

				w-=80;

				g->gtext->y=ly;
				g->gtext->y2=ly+h;
				g->gtext->gadgetID=gadgetID+100;
				g->gtext->type=GADGETTYPE_BUTTON_TEXT;
				g->gtext->ownergadget=false;

				g->childof=g;

				if(text)
					g->gtext->text=mainvar->GenerateString(text,":");

				AddGadget(g->gtext,false);
				form->gx+=80;

				int flag=WS_CHILD|SS_RIGHT;

				flag|=g->gtext->SetVisibleFlag();

				//	char *h=mainvar->GenerateString(text,":");

				g->gtext->hWnd = CreateWindowEx(0, "STATIC", g->gtext->text, flag,g->gtext->x, g->gtext->y, g->gtext->GetWidth(), g->gtext->GetHeight(), hWnd,(HMENU)g->gtext->gadgetindex, maingui->hInst, 0);
				SendMessage(g->gtext->hWnd, WM_SETFONT,(WPARAM)maingui->standardfont.hfont,0);

				//g->gtext->AddTooltip(tool);
			}
		}

		g->x=GetLX();

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=ly;
		g->y2=h==0?g->y:y+(h-1);

		//		g->x2=g->x+w;
		//		g->y=ly;
		//		g->y2=ly+h;

		g->index=integer;
		g->gadgetID=gadgetID;

		if(text)
			g->text=mainvar->GenerateString(text);

		AddGadget(g);

#ifdef WIN32

		int flag=WS_CHILD|ES_AUTOHSCROLL/* |ES_NUMBER */;

		flag|=g->SetVisibleFlag();

		/*
		g->hWnd=CreateWindowEx
		(NULL,"COMBOBOX",NULL,WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,
		g->x, g->y, g->GetWidth(), g->GetHeight(), win->hWnd,(HMENU)g->gadgetindex,
		maingui->hInst,NULL);
		*/
		g->hWnd = CreateWindowEx(WS_EX_STATICEDGE, "EDIT", 0, flag,g->x, g->y, g->GetWidth(), g->GetHeight(), hWnd,(HMENU)g->gadgetindex, maingui->hInst, 0);

		//g->y2+=4;

		if(g->hWnd)
		{
			SendMessage(g->hWnd, WM_SETFONT,(WPARAM)maingui->standardfont.hfont,0);
			g->SetInteger(integer);
		}
#endif
		g->AddTooltip(tool);

		return g;
	}

	return 0;
}

void guiGadgetList::SetHorzScroll(int pos,int range)
{
	if(win->horzscroll==true)
	{
		scrollh_range=range;
		scrollh_pos=pos;

		SCROLLINFO info;

		info.cbSize=sizeof(SCROLLINFO);

		info.fMask=SIF_POS|SIF_RANGE|SIF_PAGE;

		info.nMin=0;
		info.nMax=range;
		info.nPage=4;
		info.nPos=pos;

		SetScrollInfo(win->hWnd,SB_HORZ,&info,true);

		/*
		SetScrollInfo(win->hWnd,SB_HORZ,
		SetScrollRange(win->hWnd, SB_HORZ, 0, range, TRUE);
		SetScrollPos(win->hWnd, SB_HORZ, pos, TRUE);
		*/
	}
}

void guiGadgetList::SetVertScroll(int pos,int range)
{
	if(win->vertscroll==true)
	{
		scrollv_range=range;
		scrollv_pos=pos;

		SCROLLINFO info;

		info.cbSize=sizeof(SCROLLINFO);

		info.fMask=SIF_POS|SIF_RANGE|SIF_PAGE;

		info.nMin=0;
		info.nMax=range;
		info.nPage=4;
		info.nPos=pos;

		SetScrollInfo(win->hWnd,SB_VERT,&info,true);

		/*
		SetScrollInfo(win->hWnd,SB_HORZ,
		SetScrollRange(win->hWnd, SB_HORZ, 0, range, TRUE);
		SetScrollPos(win->hWnd, SB_HORZ, pos, TRUE);
		*/
	}
}

guiGadget_Slider *guiGadgetList::AddSlider(SliderCo *co,int gadgetID,int mode,Object *object,char *tool)
{
	if(guiGadget_Slider *s=new guiGadget_Slider)
	{	
		s->mode=mode;

		//ConvertModeToDock(g);

		s->x=co->x;
		s->x2=co->x2;
		s->y=co->y;
		s->y2=co->y2;

		s->object=object;
		s->gadgetID=gadgetID;
		s->from=co->from;
		s->to=co->to;
		s->pos=co->pos;
		s->page=co->page;
		s->horz=co->horz;
		s->gadgetID=gadgetID;
		s->subw=co->subw;
		s->subh=co->subh;
		s->staticwidth=co->staticwidth;
		s->staticheight=co->staticheight;

		ConvertModeToDock(s);
		AddGadget(s);

#ifdef WIN32

		int flag=co->horz==true?WS_CHILD|SBS_HORZ:WS_CHILD|SBS_VERT;

		flag|=s->SetVisibleFlag();

		s->hWnd = CreateWindowEx(
			0, 
			"SCROLLBAR",
			0,
			flag,
			s->x,
			s->y,
			s->GetWidth(),
			s->GetHeight(),
			hWnd,
			(HMENU)s->gadgetindex,
			maingui->hInst,
			NULL);

		if(s->hWnd)
		{
			/*
			SetScrollRange(s->hWnd,SB_CTL,from,to,false);
			SetScrollPos(s->hWnd,SB_CTL,initpos,false);
			*/

			//SendMessage(s->hWnd, WM_SETFONT,(WPARAM)gui->smallfont.hfont,0);

			SCROLLINFO info;

			info.cbSize=sizeof(SCROLLINFO);

			info.fMask=SIF_POS|SIF_RANGE|SIF_PAGE;

			info.nMin=s->from;
			info.nMax=s->to;
			info.nPage=s->page;
			info.nPos=s->pos;

			SetScrollInfo(s->hWnd,SB_CTL,&info,true);

			//	ShowWindow(s->hWnd, SW_SHOW);

			s->AddTooltip(tool);

			return s;
		}
#endif
	}

	return 0;
}

guiGadget_Slider *guiGadgetList::AddSlider(SliderCo *co,int gadgetID,Object *object,char *tool)
{
	if(guiGadget_Slider *s=new guiGadget_Slider)
	{			
		s->x=co->x;
		s->x2=co->x2;
		s->y=co->y;
		s->y2=co->y2;
		s->object=object;
		s->gadgetID=gadgetID;
		s->from=co->from;
		s->to=co->to;
		s->pos=co->pos;
		s->page=co->page;
		s->horz=co->horz;

		AddGadget(s);

#ifdef WIN32

		int flag=co->horz==true?WS_CHILD|SBS_HORZ:WS_CHILD|SBS_VERT;

		flag|=s->SetVisibleFlag();

		s->hWnd = CreateWindowEx(
			0, 
			"SCROLLBAR",
			0,
			flag,
			s->x,
			s->y,
			s->GetWidth(),
			s->GetHeight(),
			hWnd,0,
			maingui->hInst,
			NULL);

		if(s->hWnd)
		{
			/*
			SetScrollRange(s->hWnd,SB_CTL,from,to,false);
			SetScrollPos(s->hWnd,SB_CTL,initpos,false);
			*/

			//SendMessage(s->hWnd, WM_SETFONT,(WPARAM)gui->smallfont.hfont,0);

			SCROLLINFO info;

			info.cbSize=sizeof(SCROLLINFO);

			info.fMask=SIF_POS|SIF_RANGE|SIF_PAGE;

			info.nMin=s->from;
			info.nMax=s->to;
			info.nPage=s->page;
			info.nPos=s->pos;

			SetScrollInfo(s->hWnd,SB_CTL,&info,true);

			//	ShowWindow(s->hWnd, SW_SHOW);

			s->AddTooltip(tool);

			return s;
		}
#endif
	}

	return 0;
}

void guiGadget::ChangeButtonText(char *nt,int rgb_fg,int rgb_bkg)
{
	if(type==GADGETTYPE_BUTTON_TEXT)
	{
		Enable();

		on=true;
		if(text)delete text;
		text=mainvar->GenerateString(nt);

		DrawGadget();
	}
}

bool guiGadget::ChangeButtonText(char *nt,bool force)
{
	if(type==GADGETTYPE_BUTTON_TEXT || type==GADGETTYPE_BUTTON_VOLUME)
	{
		Enable();

		on=true;

		if(force==false && text && nt && strcmp(text,nt)==0)
			return false;

		if(text)delete text;

		text=mainvar->GenerateString(nt);
		DrawGadget();

		return true;
	}

	return false;
}

void guiGadget::ChangeButtonImage(int imageID,int iflag)
{
	if(type==GADGETTYPE_BUTTON_IMAGE && (buttontype!=imageID || iflag!=-1))
	{
		buttontype=imageID;

		/*
		if(iflag!=-1)
		{
		if(iflag==0)
		mode CLEARBIT MODE_HIGHLIGHT;
		else
		mode |=MODE_HIGHLIGHT;
		}
		*/

		DrawGadget();
	}
}

void guiGadget::MouseOver(int mx,int my)
{
	if(disabled==false && on==true)
	{
		mouseoverx=mx;
		mouseovery=my;
		DrawGadget();
	}
}

void guiGadget::Enable()
{
	on=true;
	if(disabled==false)return;

	disabled=false;

	if(linkclickgadget)
		linkclickgadget->Enable();

	if(ownergadget==true)
	{
		DrawGadget();
	}
	else
	{
#ifdef WIN32
		if(hWnd)
			EnableWindow(hWnd,true);
#endif
	}
}

void guiGadget::Disable()
{
	if(disabled==true)return;
	disabled=true;

	if(linkclickgadget)
		linkclickgadget->Disable();

	if(IsSystemGadget()==false)
		DrawGadget();
	else
	{
		if(type==GADGETTYPE_STRING)
		{
			if(string)
				delete string;

			string=mainvar->GenerateString("-");
#ifdef WIN32
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)string);
#endif
		}

#ifdef WIN32
		if(hWnd)
			EnableWindow(hWnd,false);
#endif
	}

	Disabled();
}

void guiGadget::MouseMove()
{
	int mx,my;
	maingui->GetMouseOnScreen(&mx,&my);

	int dy=startmousey-my;
	int dx=mx-startmousex;

	int hdx=dx<0?-dx:dx;
	int hdy=dy<0?-dy:dy;

	if(horzslider==true)
	{
		deltay=dx;
	}
	else
		if(mainsettings->mouseonlyvertical==true)
			deltay=dy;
		else
		{
			if(hdx>hdy)
				deltay=dx;
			else
				deltay=dy;
		}

		if(deltay && DeltaY()==true)
		{
			//if(hdy>=hdx)
			startmousey=GetMouseY();

			//if(hdx>=hdy)
			startmousex=GetMouseX();
		}
}

void guiGadget::EndMouseMove()
{
	mouseover=false;
	getMouseMove=false;
	init=false;
	editIndex=-1;
	mouseoverindex=-1;

	DrawGadget();
}

bool guiGadget::CheckMouseOver(int mx,int my)
{
	int gx=x;
	int gy=y;
	int gx2=x2;
	int gy2=y2;

	if(parent)
	{
		gx+=parent->formchild->x;
		gx2+=parent->formchild->x;

		gy+=parent->formchild->y;
		gy2+=parent->formchild->y;

		gx+=parent->x;
		gx2+=parent->x;

		gy+=parent->y;
		gy2+=parent->y;
	}
	else
	{
		gx+=formchild->x;
		gx2+=formchild->x;

		gy+=formchild->y;
		gy2+=formchild->y;
	}

	if(gx<=mx && gx2>=mx && gy<=my && gy2>=my)
		return true;

	/*
	if(x+formchild->x<=mx && x2+formchild->x>=mx && y+formchild->y<=my && y2+formchild->y>=my)
	return true;
	*/

	return false;
}

void guiGadget_Number::SetPos(double p)
{
	//if(on==false || disabled==true)
	{
		vnumber=p;
		Enable();
		DrawGadget();
	}
}

void guiGadget_Number::SetPosType(int t)
{
	nrtype=t;
}

void guiGadget_Number::ChangeEditData(OSTART newvalue,bool refreshgui)
{
	if(newvalue>vto)
		newvalue=vto;
	else
		if(newvalue<vfrom)
			newvalue=vfrom;

	if(newvalue!=vnumber)
	{
		vnumber=newvalue;

		if(refreshgui==true)
			guilist->win->Gadget(this);

		DrawGadget();
	}
}

void guiGadget_Number::MouseWheel(int delta)
{
	double h=vnumber;

	h+=-delta;

	if(h>vto)
		h=vto;
	else
		if(h<vfrom)
			h=vfrom;

	if(h!=vnumber)
	{
		TRACE ("GNR %d\n",h);

		vnumber=h;
		guilist->win->Gadget(this);
		DrawGadget();

		TRACE ("GNR2 %d\n",(int)vnumber);
	}

}

void guiGadget_Number::LeftMouseDown()
{
	InitGetMouseMove();
}

void guiGadget_Number::LeftMouseUp()
{
	if(getMouseMove==true)
	{
		EndMouseMove();
	}
}

void guiGadget_Number::MouseMoveLeftDown()
{
	if(getMouseMove==true)
	{
		InitDelta();

		if(deltay && DeltaY()==true)
		{
			//if(hdy>=hdx)
			startmousey=GetMouseY();

			//if(hdx>=hdy)
			startmousex=GetMouseX();
		}
	}
}

void guiGadget_Number::LeftMouseDoubleClick()
{
	switch(nrtype)
	{
	case NUMBER_MIDICHANNEL:
		{
			if(EditData *edit=new EditData)
			{
				// long position;
				edit->song=0;
				edit->win=guilist->win;

				edit->x=x;
				edit->y=y2;
				edit->width=GetWidth();

				edit->title=Cxs[CXS_EDIT];
				edit->deletename=false;
				edit->id=-1;
				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->helpobject=this;

				edit->from=0;
				edit->to=16;
				edit->value=GetPos();

				maingui->EditDataValue(edit);
			}
		}
		break;

		// Integer
	case NUMBER_INTEGER:
		{
			if(EditData *edit=new EditData)
			{
				// long position;
				edit->song=0;
				edit->win=guilist->win;

				edit->x=x;
				edit->y=y2;
				edit->width=GetWidth();

				edit->title=Cxs[CXS_EDIT];
				edit->deletename=false;
				edit->id=-1;
				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->helpobject=this;

				edit->from=vfrom;
				edit->to=vto;
				edit->value=GetPos();

				maingui->EditDataValue(edit);
			}
		}
		break;

		// Keys
	case NUMBER_KEYS:
		{
			if(EditData *edit=new EditData)
			{
				// long position;
				edit->song=guilist->win->WindowSong();
				edit->win=guilist->win;

				edit->x=x;
				edit->y=y2;
				edit->width=GetWidth();

				edit->title=Cxs[CXS_EDIT];
				edit->deletename=false;
				edit->id=-1;
				edit->type=EditData::EDITDATA_TYPE_KEYS;
				edit->helpobject=this;

				edit->from=vfrom;
				edit->to=vto;
				edit->value=GetPos();

				maingui->EditDataValue(edit);
			}
		}
		break;

		// double/float
	case NUMBER_0:
	case NUMBER_00:
	case NUMBER_000:
		if(EditData *edit=new EditData)
		{
			// long position;
			edit->song=0;
			edit->win=guilist->win;

			edit->x=x;
			edit->y=y2;
			edit->width=GetWidth();

			edit->title=Cxs[CXS_EDIT];
			edit->deletename=false;
			edit->id=-1;
			edit->type=EditData::EDITDATA_TYPE_DOUBLE;
			edit->helpobject=this;

			edit->dfrom=vfrom;
			edit->dto=vto;
			edit->dvalue=GetDoublePos();

			switch(nrtype)
			{
			case NUMBER_0:
				edit->doubledigits=1;
				break;

			case NUMBER_00:
				edit->doubledigits=2;
				break;

			case NUMBER_000:
				edit->doubledigits=3;
				break;
			}

			maingui->EditDataValue(edit);
		}
		break;
	}
}

bool guiGadget_Number::DeltaY()
{
	double h=vnumber;
	int delta=deltay<0?-deltay:deltay;

	double range=vto-vfrom; // range

	if(range<10)
	{
		if(delta<7)
			return false;

		if(deltay<0)deltay=-1;
		else
			deltay=1;
	}
	else
		if(range<20)
		{
			if(delta<5)
				return false;

			if(deltay<0)deltay=-1;
			else
				deltay=1;
		}
		else
			if(range<30)
			{
				if(delta<2)
					return false;

				if(deltay<0)deltay=-1;
				else
					deltay=1;
			}
			else
				if(range>1000)
				{
					deltay*=8;
				}
				else
					if(range>500)
					{
						delta*=4;
					}
					else
						if(range>200)
							delta*=2;

	h+=deltay;

	if(h>vto)
		h=vto;
	else
		if(h<vfrom)
			h=vfrom;

	if(h!=vnumber)
	{
		TRACE ("GNR %d\n",h);

		vnumber=h;
		guilist->win->Gadget(this);
		DrawGadget();

		TRACE ("GNR2 %d\n",(int)vnumber);
	}

	return true;
}

void guiGadget_Volume::RightMouseDown()
{
	ResetValue();
}

void guiGadget_Volume::SetPos(double i)
{
	volume=i;

	DrawGadget();
	//DrawGadgetEx();
	//Blt();
}

void guiGadget_Volume::DrawGadgetEx()
{
	//UBYTE r,g,b;

	gbitmap.SetAudioColour(volume);

	/*
	maingui->colourtable.GetRGB(COLOUR_TEXT,&r,&g,&b);
	SetTextColor(gbitmap.hDC, RGB(r, g, b));
	*/

	RECT rect;

	rect.left=GetX();
	rect.top=GetY();
	rect.right=rect.left+GetWidth();
	rect.bottom=rect.top+GetHeight();

	if(char *h=mainaudio->ScaleAndGenerateDBString(volume,false))
	{
		if(addtext)
		{
			char *h2=mainvar->GenerateString(addtext,h);
			if(h2)
			{
				delete h;
				h=h2;
			}
		}

		DrawText(gbitmap.hDC,h , strlen(h), &rect, DT_SINGLELINE | DT_VCENTER|DT_CENTER);
		delete h;
	}
}

void guiGadget_Volume::ResetValue()
{
	/*
	if(volume<=mainaudio->silencefactor)
	volume=1;
	else
	if(volume>1)
	volume=1;
	else
	if(vole
	*/

	if(volume!=0.5)
	{
		volume=0.5;
		guilist->win->Gadget(this);
		DrawGadget();
		if(parent)
			parent->Blt(x,y,x2,y2);
	}
}

void guiGadget_Volume::MouseWheel(int delta)
{
	ARES p=volume;
	p*=LOGVOLUME_SIZE;

	p-=delta;

	if(p<0)
		p=0;
	else
		if(p>LOGVOLUME_SIZE)
			p=LOGVOLUME_SIZE;

	ARES h2=p;
	h2/=LOGVOLUME_SIZE;

	if(h2!=volume)
	{
		volume=h2;
		guilist->win->Gadget(this);
		DrawGadget();
		if(parent)
			parent->Blt(x,y,x2,y2);
	}
}

void guiGadget_Volume::LeftMouseDown()
{
	InitGetMouseMove();
}

void guiGadget_Volume::LeftMouseUp()
{
	if(getMouseMove==true)
	{
		EndMouseMove();

		if(parent)
			parent->Blt(x,y,x2,y2);
	}
}

void guiGadget_Volume::MouseMoveLeftDown()
{
	if(getMouseMove==true)
	{
		InitDelta();

		if(deltay && DeltaY()==true)
		{
			//if(hdy>=hdx)
			startmousey=GetMouseY();

			//if(hdx>=hdy)
			startmousex=GetMouseX();
		}
	}
}

void guiGadget_Volume::LeftMouseDoubleClick()
{
}


bool guiGadget_Volume::DeltaY()
{
	int deltay=this->deltay;

	ARES p=volume;
	p*=LOGVOLUME_SIZE;

	if(p>AUDIOMIXER_ADD-10 && p<AUDIOMIXER_ADD+10)
	{
		if(deltay>-8 && deltay<8)
			return false;

		if(deltay<-1)deltay=-1;
		else
			if(deltay>1)deltay=1;
	}
	else
		if(p>AUDIOMIXER_ADD-15 && p<AUDIOMIXER_ADD+15)
		{
			if(deltay>-5 && deltay<5)
				return false;

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

		p+=deltay;

		if(p<0)
			p=0;
		else
			if(p>LOGVOLUME_SIZE)
				p=LOGVOLUME_SIZE;

		ARES h2=p;
		h2/=LOGVOLUME_SIZE;

		if(h2!=volume)
		{
			volume=h2;
			guilist->win->Gadget(this);
			DrawGadget();
			if(parent)
				parent->Blt(x,y,x2,y2);
		}

		return true;
}

void guiGadget_Number::DrawGadgetEx()
{
	UBYTE r,g,b;

	maingui->colourtable.GetRGB((mode&MODE_INFO)?COLOUR_TEXT_INFO:COLOUR_TEXT,&r,&g,&b);
	SetTextColor(gbitmap.hDC, RGB(r, g, b));

	RECT rect;

	rect.left=GetX()+1;
	rect.top=GetY()+1;
	rect.right=rect.left+GetX2();
	rect.bottom=rect.top+GetY2();

	if(on==false)
	{
		DrawText(gbitmap.hDC,"-" ,1, &rect, DT_SINGLELINE | DT_CENTER);
		return;
	}

	switch(nrtype)
	{
	case NUMBER_MIDICHANNEL:
		{
			char *h=vnumber==0?"All":mainvar->ConvertIntToChar((int)vnumber,numberstring);
			DrawText(gbitmap.hDC,h , strlen(h), &rect, DT_SINGLELINE | DT_CENTER);
		}
		break;

	case NUMBER_INTEGER:
		{
			char *h=mainvar->ConvertIntToChar((int)vnumber,numberstring);
			DrawText(gbitmap.hDC,h , strlen(h), &rect, DT_SINGLELINE | DT_CENTER);
		}	
		break;

	case NUMBER_INTEGER_PERCENT:
		{
			char *h=mainvar->ConvertIntToChar((int)vnumber,numberstring);

			char *h2=mainvar->GenerateString(h," %");

			if(h2)
			{
				DrawText(gbitmap.hDC,h2 , strlen(h2), &rect, DT_SINGLELINE | DT_CENTER);
				delete h2;
			}
		}
		break;

	case NUMBER_0:
		{
			char *h=mainvar->ConvertDoubleToChar(vnumber,numberstring,1);
			DrawText(gbitmap.hDC,h , strlen(h), &rect, DT_SINGLELINE | DT_CENTER);
		}
		break;

	case NUMBER_00:
		{
			char *h=mainvar->ConvertDoubleToChar(vnumber,numberstring,2);
			DrawText(gbitmap.hDC,h , strlen(h), &rect, DT_SINGLELINE | DT_CENTER);
		}
		break;

	case NUMBER_000:
		{
			char *h=mainvar->ConvertDoubleToChar(vnumber,numberstring,3);
			DrawText(gbitmap.hDC,h , strlen(h), &rect, DT_SINGLELINE | DT_CENTER);
		}
		break;

	case NUMBER_KEYS:
		{
			char *h=maingui->ByteToKeyString(guilist->win->WindowSong(),(UBYTE)vnumber);

			DrawText(gbitmap.hDC,h , strlen(h), &rect, DT_SINGLELINE | DT_CENTER);
		}
		break;

	}
}

int guiGadget::GetBackGroundColour()
{
	if(mode&MODE_NOMOUSEOVER)
	{
		return COLOUR_NOMOUSEOVER;
	}

	if(mode&MODE_DUMMY)
	{
		if(useextracolour==true)
			return bgcolour;

		return COLOUR_BACKGROUNDWINDOWS;
	}

	switch(type)
	{
	case GADGETTYPE_BUTTON_TEXT:
	case GADGETTYPE_BUTTON_IMAGE:
		{
			if(mouseover==true)
			{
				//if(mode&MODE_TOGGLE)
				//	return toggled==true?COLOUR_TOGGLEON:COLOUR_TOGGLEOFF;

				if(staticbutton==true)
					return COLOUR_MOUSEOVERSTATIC;

				return COLOUR_MOUSEOVER;
			}

			if(mode&MODE_TOGGLE)
				return mode&MODE_TOGGLED?COLOUR_TOGGLEON:COLOUR_TOGGLEOFF;

			if(useextracolour==true)
				return bgcolour;

			if(mode&MODE_GROUP)
				return (mode&MODE_TOGGLED)?COLOUR_GROUPTOGGLEON:COLOUR_GROUPTOGGLEOFF;

			if(mode&MODE_PUSH)
				return COLOUR_GADGETBACKGROUNDPUSHBUTTON;

			if(mode&MODE_OS)
				return COLOUR_GADGETBACKGROUNDOSBUTTON;

			if(staticbutton==true)
				return COLOUR_GADGETBACKGROUNDSTATIC;

			/*
			if(mode&MODE_NOTACTIVE)
			return COLOUR_GADGETBACKGROUNDNOTACTIVE;
			*/

			if(sysgadget==true)
				return COLOUR_GADGETBACKGROUNDSYSTEM;

			return COLOUR_GADGETBACKGROUND;
		}
		break;

	case GADGETTYPE_TIME:
		{
			guiGadget_Time *gt=(guiGadget_Time *)this;

			switch(gt->type)
			{
			case WINDOWDISPLAY_MEASURE:
				{
					Seq_Song *song=guilist->win->WindowSong();

					if(!song)
					{
						return mouseover==true?COLOUR_MOUSEOVER:COLOUR_GREY_DARK;
					}
					else
					{
						if(gt->islength==true)
						{
							return mouseover==true?COLOUR_BLUE_LIGHT:COLOUR_GREY_DARK;
						}

						if(gt->showprecounter==true)
						{
							if(song->status&Seq_Song::STATUS_WAITPREMETRO)
							{
								return mouseover==true?COLOUR_GREY:COLOUR_BLACK;
							}
						}

						if(gt->t_time==-1)
							return COLOUR_BLACK;

						if(getMouseMove==true)
							return COLOUR_GAGDGETFOCUS;

						return mouseover==true?COLOUR_MOUSEOVER:COLOUR_GREY_DARK;
					}
				}
				break;

			case WINDOWDISPLAY_SMPTE:
				{
					Seq_Song *song=guilist->win->WindowSong();

					if(!song)
					{
						return COLOUR_BLACK_LIGHT;
					}
					else
					{
						if(gt->islength==true)
						{
							return mouseover==true?COLOUR_BLUE_LIGHT:COLOUR_GREY_DARK;
						}

						if(gt->t_time==-1)
							return COLOUR_BLACK_LIGHT;

						if(getMouseMove==true)
							return COLOUR_GAGDGETFOCUS;

						return mouseover==true?COLOUR_MOUSEOVER:COLOUR_BLACK_LIGHT;
					}
				}
				break;
			}
		}
		break;

	case GADGETTYPE_DOUBLEBUFFER:
		return COLOUR_GADGETBACKGROUNDSYSTEM;
		break;

	case GADGETTYPE_BUTTON_NUMBER:
	case GADGETTYPE_BUTTON_VOLUME:
		{
			if(getMouseMove==true)
				return COLOUR_GAGDGETFOCUS;

			return mouseover==true?COLOUR_MOUSEOVER:COLOUR_GADGETBACKGROUNDNUMBER;
		}

		break;
	}

	return COLOUR_GADGETBACKGROUND;
}

void guiGadget::DrawGadgetEnable()
{
	if(disabled==true)
		Enable();
	else
		DrawGadget();
}

void guiGadget::DrawGadget()
{
	if(!hWnd)
		return;

	if(formchild && (formchild->deactivated==true || formchild->enable==false))
		return;

	switch(type)
	{
	case GADGETTYPE_DOUBLEBUFFER:
		{
			DrawGadgetEx();
			// Blt by WM_PAINT
		}
		break;

	case GADGETTYPE_BUTTON_TEXT:
	case GADGETTYPE_BUTTON_IMAGE:
	case GADGETTYPE_BUTTON_NUMBER:
	case GADGETTYPE_BUTTON_VOLUME:
	case GADGETTYPE_TIME:
		{
			switch(type)
			{
			case GADGETTYPE_BUTTON_TEXT:
			case GADGETTYPE_BUTTON_IMAGE:
			case GADGETTYPE_BUTTON_NUMBER:
			case GADGETTYPE_BUTTON_VOLUME:
			case GADGETTYPE_TIME:
				{	

#define XX2 GetX()+GetX2()
#define YY2 GetY()+GetY2()

#define ADDX 2
#define WIDTH (GetWidth()-4)

					if(disabled==true)
					{
						gbitmap.guiFillRect(GetX(),GetY(),XX2,YY2,COLOUR_BLACK);
					}
					else
						if(on==false)
						{
							gbitmap.guiFillRect(GetX(),GetY(),XX2,YY2,COLOUR_UNUSED);
						}
						else
						{
							if(mode&MODE_GROUP)
							{
								int bk=GetBackGroundColour();

								gbitmap.guiFillRect(GetX(),GetY(),XX2,YY2,COLOUR_BACKGROUNDWINDOWS);
								gbitmap.guiFillRect(GetX()+GetY2()/2,GetY(),XX2,YY2,bk);

								gbitmap.guiDrawCircle(GetX(),GetY(),GetX()+GetY2(),YY2,bk);

								if(mode&MODE_TOGGLED)
								{
									if(GetX()+8<GetX2() && GetY()+8<GetY2())
										gbitmap.guiDrawCircle(GetX()+3,GetY()+3,(GetX()+GetY2())-4,YY2-4,COLOUR_GREEN);
								}
							}
							else
							{
								if((mode&MODE_BKCOLOUR) && mouseover==false)
									gbitmap.guiFillRect_RGB(GetX(),GetY(),XX2,YY2,bgcolour_rgb);
								else
									gbitmap.guiFillRect(GetX(),GetY(),XX2,YY2,GetBackGroundColour());

								if((!(mode&MODE_INFO)))
								{
									gbitmap.guiDrawLineX(GetX(),GetY(),YY2,(mode&MODE_TOGGLE)?COLOUR_BORDERGADGET:COLOUR_BORDERGADGETLEFT);

									if(!(mode&MODE_TOGGLE))
									{
										//gbitmap.guiDrawLineX(XX2-1,GetY(),YY2);
										gbitmap.guiDrawLineX(XX2,GetY(),YY2,COLOUR_BORDERGADGETRIGHT);

									}
								}
							}
						}

						if(mode&MODE_INFO)
						{
							gbitmap.guiDrawRect(GetX(),GetY(),XX2,YY2,COLOUR_INFOBORDER);
						}


						if(mode&MODE_LENGTHTIME)
						{
							gbitmap.guiDrawRect(GetX(),GetY(),XX2,YY2,COLOUR_LENGTH);
						}

						if(bordercolour_use==true)
						{
							gbitmap.guiDrawRect_RGB(GetX(),GetY(),XX2,YY2,bordercolour_rgb);
						}
				}
				break;
			}

			if(disabled==false)
				switch(type)
			{
				case GADGETTYPE_BUTTON_IMAGE:
					{
						switch(buttontype)
						{
						case IMAGE_WINDOWDOWN:
							{
								//gbitmap.guiFillRect(COLOUR_GADGETBACKGROUNDOSBUTTON);

								int mx=WIDTH/2;
								int my=GetHeight()/2;

								double h=mx;
								h*=0.35;

								int x1=mx-(int)h;
								int x2=mx+(int)h;

								h=my;
								h*=0.4;

								int y1=my-(int)h;
								int y2=my+(int)h;

								gbitmap.guiDrawLine(mx,y2,x2,y1,COLOUR_GREY_LIGHT);
								gbitmap.guiDrawLineY(y1,x2,x1);
								gbitmap.guiDrawLine(x1,y1,mx,y2);
								gbitmap.guiFill(mx,my,COLOUR_GREY_LIGHT);

								//gbitmap.guiDrawLine(x1+2,y2,x2+3,y1-1,COLOUR_WHITE);
							}
							break;

						case IMAGE_SOLOBUTTON_ON:
						case IMAGE_SOLOBUTTON_OFF:
							{
								int mx=GetX2()-4;
								int my=GetY2()-4;

								mx/=4;
								my/=4;

								gbitmap.guiDrawCircle(2,2,GetX2()-2,GetY2()-2,buttontype==IMAGE_SOLOBUTTON_ON?COLOUR_YELLOW:COLOUR_GREY_LIGHT);

								gbitmap.SetTextColour(buttontype==IMAGE_SOLOBUTTON_ON?COLOUR_BLACK:COLOUR_GREY_DARK);

								mx=2+(GetX2()-2);

								mx/=2;

								mx-=gbitmap.GetTextWidth("S");

								mx+=3;
								gbitmap.guiDrawText(mx,GetHeight(),GetX2()-2,"S");
								gbitmap.guiDrawText(mx+1,GetHeight(),GetX2()-2,"S");

								//	gbitmap.guiDrawCircle(2+mx,2+my,GetX2()-2-mx,GetY2()-2-my,GetBackGroundColour());
								//	gbitmap.guiFillRect(2,2+GetY2()/2,GetX2()/2-2,GetY2(),GetBackGroundColour());
							}
							break;

						case IMAGE_CYCLEBUTTON_ON:
						case IMAGE_CYCLEBUTTON_OFF:
							{
								int mx=GetX2()-4;
								int my=GetY2()-4;

								mx/=4;
								my/=4;

								gbitmap.guiDrawCircle(2,2,GetX2()-2,GetY2()-2,buttontype==IMAGE_CYCLEBUTTON_ON?COLOUR_GREEN:COLOUR_GREY_LIGHT);
								gbitmap.guiDrawCircle(2+mx,2+my,GetX2()-2-mx,GetY2()-2-my,GetBackGroundColour());
								gbitmap.guiFillRect(2,2+GetY2()/2,GetX2()/2-2,GetY2(),GetBackGroundColour());
							}
							break;

						case IMAGE_WINDOWUP:
							{
								//gbitmap.guiFillRect(COLOUR_GADGETBACKGROUNDOSBUTTON);

								int mx=WIDTH/2;
								int my=GetHeight()/2;

								double h=mx;
								h*=0.35;

								int x1=mx-(int)h;
								int x2=mx+(int)h;

								h=my;
								h*=0.4;

								int y1=my-(int)h;
								int y2=my+(int)h;

								gbitmap.guiDrawLine(mx,y1,x2,y2,COLOUR_GREY_LIGHT);
								gbitmap.guiDrawLine(x2,y2,x1,y2);
								gbitmap.guiDrawLine(x1,y2,mx,y1);
								gbitmap.guiFill(mx,my,COLOUR_GREY);
							}
							break;

						case IMAGE_CLOSEWINDOW:
							{
								//gbitmap.guiFillRect(COLOUR_GADGETBACKGROUNDOSBUTTON);

								int mx=WIDTH/2;
								int my=GetHeight()/2;

								double h=mx;
								h*=0.25;

								int x1=mx-(int)h;
								int x2=mx+(int)h;

								h=my;
								h*=0.3;

								int y1=my-(int)h;
								int y2=my+(int)h;

								gbitmap.guiDrawLine(x1-1,y1,x2,y2+1,COLOUR_RED);
								gbitmap.guiDrawLine(x1,y1,x2+1,y2+1);
								gbitmap.guiDrawLine(x1+1,y1,x2+2,y2+1);
								gbitmap.guiDrawLine(x1+2,y1,x2+3,y2+1);

								gbitmap.guiDrawLine(x1-1,y2,x2,y1-1);
								gbitmap.guiDrawLine(x1,y2,x2+1,y1-1);
								gbitmap.guiDrawLine(x1+1,y2,x2+2,y1-1);
								gbitmap.guiDrawLine(x1+2,y2,x2+3,y1-1);
							}
							break;

						case IMAGE_RECSTEPBUTTON_ON:
							{
								double h=WIDTH;
								h*=0.25;

								int fx=h;
								int tx=3*h;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=COLOUR_RED;
								gbitmap.guiDrawCircle(fx,fy,tx,ty,col);

								h=tx-fx;
								h*=0.25;

								fx+=h-1;
								tx-=h;

								h=ty-fy;
								h*=0.25;
								fy+=h-1;
								ty-=h;

								gbitmap.guiFillRect(fx,fy,tx,ty,COLOUR_WHITE);
							}
							break;

						case IMAGE_BYPASS_ON:
						case IMAGE_BYPASS_OFF:
							{
								int bcol=buttontype==IMAGE_BYPASS_ON?COLOUR_GREY:COLOUR_BLUE_LIGHT2;
								int fcol=buttontype==IMAGE_BYPASS_ON?COLOUR_GREY_DARK:COLOUR_BLACK;

								gbitmap.guiFillRect(1,1,gbitmap.GetX2()-1,gbitmap.GetY2()-1,bcol);

								int w=maingui->GetFontSizeY()/4;

								gbitmap.guiDrawCircle(3,3,gbitmap.GetX2()-3,gbitmap.GetY2()-3,fcol);
								gbitmap.guiDrawCircle(3+w,3+w,(gbitmap.GetX2()-3)-w,(gbitmap.GetY2()-3)-w,bcol);

								int midx=(gbitmap.GetX2()-3)-3;
								midx/=2;
								//midx-=w;

								gbitmap.guiFillRect(midx,1,midx+2*w,GetY2()/2,bcol);

								double wh=w;

								wh*=0.65;

								int wi=wh<2?2:(int)wh;

								gbitmap.guiFillRect(midx+wi,w,(midx+2*w)-wi,GetY2()/2,fcol);
							}
							break;

						case IMAGE_RECBUTTON_ON:
						case IMAGE_RECBUTTON_PUNCHOUT_ON:
						case IMAGE_RECBUTTON_PUNCHIN_ON:
						case IMAGE_RECBUTTON_OFF:
						case IMAGE_RECBUTTON_PUNCHOUT_OFF:
						case IMAGE_RECBUTTON_PUNCHIN_OFF:
							{
								double h=WIDTH;
								h*=0.25;

								int fx=h;
								int tx=3*h;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col;

								switch(buttontype)
								{
								case IMAGE_RECBUTTON_ON:
								case IMAGE_RECBUTTON_PUNCHOUT_ON:
								case IMAGE_RECBUTTON_PUNCHIN_ON:
									col=COLOUR_RECORD;
									break;

								default:
									col=COLOUR_GREY;
									break;
								}

								gbitmap.guiDrawCircle(fx,fy,tx,ty,col);

								switch(buttontype)
								{
								case IMAGE_RECBUTTON_PUNCHOUT_ON:
								case IMAGE_RECBUTTON_PUNCHOUT_OFF:
									gbitmap.SetTextColour(COLOUR_WHITE);
									gbitmap.guiDrawTextCenter(0,gbitmap.GetY2()-maingui->GetFontSizeY(),gbitmap.GetX2(),gbitmap.GetY2(),"P.OUT");
									break;

								case IMAGE_RECBUTTON_PUNCHIN_ON:
								case IMAGE_RECBUTTON_PUNCHIN_OFF:
									gbitmap.SetTextColour(COLOUR_WHITE);
									gbitmap.guiDrawTextCenter(0,gbitmap.GetY2()-maingui->GetFontSizeY(),gbitmap.GetX2(),gbitmap.GetY2(),"P.IN");
									break;
								}
							}
							break;

						case IMAGE_SPQN:
							{
								double h=WIDTH/2;
								h*=0.25;

								int fx=h;
								int tx=GetWidth()/2;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=COLOUR_BLUE_LIGHT2;

								gbitmap.guiDrawLineX(fx,fy,ty,col);
								gbitmap.guiDrawLine(fx,fy,tx,fy+(ty-fy)/2);
								gbitmap.guiDrawLine(fx,ty,tx+1,fy+(ty-fy)/2);

								h=WIDTH/3;

								gbitmap.guiDrawRect(tx,fy-1,tx+h,ty+1,col);

								int midx=(tx+h)-tx;

								midx/=2;
								midx+=tx;

								gbitmap.guiDrawLineX(midx,fy,ty,COLOUR_WHITE);



								//gbitmap.guiFill(fx+1,fy+(ty-fy)/2,col);
							}
							break;

						case IMAGE_SPP0:
						case IMAGE_PREVMARKER:
							{
								double h=WIDTH;
								h*=0.25;

								int fx=h;
								int tx=3*h;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=COLOUR_GREY;

								gbitmap.guiDrawLineX(tx,fy,ty,col);
								gbitmap.guiDrawLine(fx,fy+(ty-fy)/2,tx,fy);
								gbitmap.guiDrawLine(fx,fy+(ty-fy)/2,tx,ty);
								gbitmap.guiFill(tx-1,fy+(ty-fy)/2,col);

								if(buttontype==IMAGE_SPP0)
								{
									h=tx-fx;
									h*=0.05;

									gbitmap.guiFillRect(fx-h,fy,fx+h,ty,col);
								}
							}
							break;

						case IMAGE_REW:
							{
								double h=WIDTH/2;
								h*=0.25;

								int fx=h;
								int tx=GetWidth()/2;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=pushed==true?COLOUR_WHITE:COLOUR_GREY;

								gbitmap.guiDrawLineX(tx,fy,ty,col);
								gbitmap.guiDrawLine(fx,fy+(ty-fy)/2,tx,fy);
								gbitmap.guiDrawLine(fx,fy+(ty-fy)/2,tx,ty);
								gbitmap.guiFill(tx-1,fy+(ty-fy)/2,col);

								gbitmap.Blt(fx,0,(tx-fx)+1,GetHeight(),tx+2,0);
							}
							break;

						case IMAGE_FF:
							{
								double h=WIDTH/2;
								h*=0.25;

								int fx=h;
								int tx=GetWidth()/2;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=pushed==true?COLOUR_WHITE:COLOUR_GREY;

								gbitmap.guiDrawLineX(fx,fy,ty,col);
								gbitmap.guiDrawLine(fx,fy,tx,fy+(ty-fy)/2);
								gbitmap.guiDrawLine(fx,ty,tx+1,fy+(ty-fy)/2);
								gbitmap.guiFill(fx+1,fy+(ty-fy)/2,col);

								gbitmap.Blt(fx,0,(tx-fx)+1,GetHeight(),tx+2,0);
							}
							break;

						case IMAGE_NEXTMARKER:
							{
								double h=WIDTH;
								h*=0.25;

								int fx=h;
								int tx=3*h;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=COLOUR_GREY;

								gbitmap.guiDrawLineX(fx,fy,ty,col);
								gbitmap.guiDrawLine(fx,fy,tx,fy+(ty-fy)/2);
								gbitmap.guiDrawLine(fx,ty,tx+1,fy+(ty-fy)/2);
								gbitmap.guiFill(fx+1,fy+(ty-fy)/2,col);
							}
							break;

						case IMAGE_STOPBUTTON_SMALL_ON:
						case IMAGE_STOPBUTTON_SMALL_OFF:

						case IMAGE_STOPBUTTON_ON:
						case IMAGE_STOPBUTTON_OFF:
							{
								double h=WIDTH;
								h*=0.25;

								int fx=h;
								int tx=3*h;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=buttontype==IMAGE_STOPBUTTON_OFF || buttontype==IMAGE_STOPBUTTON_SMALL_OFF?COLOUR_GREY:COLOUR_WHITE;

								gbitmap.guiFillRect(fx,fy,tx,ty,col);
							}
							break;


						case IMAGE_PLAYBUTTON_SMALL_ON:
						case IMAGE_PLAYBUTTON_SMALL_OFF:

						case IMAGE_PLAYBUTTON_ON:
						case IMAGE_PLAYBUTTON_OFF:
							{
								double h=WIDTH;
								h*=0.25;

								int fx=h;
								int tx=3*h;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=buttontype==IMAGE_PLAYBUTTON_ON || buttontype==IMAGE_PLAYBUTTON_SMALL_ON?COLOUR_GREEN:COLOUR_GREY;

								gbitmap.guiDrawLineX(fx,fy,ty,col);
								gbitmap.guiDrawLine(fx,fy,tx+1,fy+(ty-fy)/2+1);
								gbitmap.guiDrawLine(fx,ty,tx+1,fy+(ty-fy)/2);
								gbitmap.guiFill(fx+1,fy+(ty-fy)/2,col);
							}
							break;

						case IMAGE_PLAYBUTTONCLIP_OFF:
						case IMAGE_PLAYBUTTONCLIP_ON:
							{
								double h=WIDTH;
								h*=0.25;

								int fx=h;
								int tx=3*h;

								h=GetHeight();
								h*=0.25;

								int fy=h;
								int ty=3*h;

								int col=buttontype==IMAGE_PLAYBUTTONCLIP_ON?COLOUR_GREEN:COLOUR_GREY;


								gbitmap.guiDrawLineX(fx,fy,ty,col);
								gbitmap.guiDrawLine(fx,fy,tx+1,fy+(ty-fy)/2+1);
								gbitmap.guiDrawLine(fx,ty,tx+1,fy+(ty-fy)/2);
								gbitmap.guiFill(fx+1,fy+(ty-fy)/2,col);

								gbitmap.guiDrawRect(fx-1,fy-1,tx+1,ty+1,COLOUR_YELLOW);

							}
							break;

						default:
							if(guiBitmap *imagebitmap=maingui->gfx.FindBitMap(buttontype))
							{
								if(HDC hMemDC = CreateCompatibleDC(gbitmap.hDC))
								{
									SelectObject(hMemDC,imagebitmap->hBitmap);

									BITMAP bm;
									GetObject(imagebitmap->hBitmap, sizeof(BITMAP), &bm );

									int hx=WIDTH;
									hx/=2;
									hx-=imagebitmap->width/2;

									int hy=GetHeight()-2;
									hy/=2;
									hy-=imagebitmap->height/2;

									if(hx<0)
										hx=0;

									if(hy<0)
										hy=0;

									BitBlt(
										gbitmap.hDC, // handle to destination device context
										GetX()+hx,  // x-coordinate of destination rectangle's upper-left 
										GetY()+hy,  // y-coordinate of destination rectangle's upper-left 
										imagebitmap->width,  // width of destination rectangle
										imagebitmap->height, // height of destination rectangle
										hMemDC,  // handle to source device context
										0,   // x-coordinate of source rectangle's upper-left 
										0,   // y-coordinate of source rectangle's upper-left 
										SRCCOPY  // raster operation code
										);	

									DeleteDC(hMemDC);
								}
							}	
							break;
						}
					}
					break;

				case GADGETTYPE_BUTTON_VOLUME:
				case GADGETTYPE_BUTTON_NUMBER:
				case GADGETTYPE_TIME:
					{
						DrawGadgetEx();
					}
					break;

				case GADGETTYPE_BUTTON_TEXT:
					{
						if(on==false)
						{
							UBYTE r,g,b;

							if(mode&MODE_INFO)
								maingui->colourtable.GetRGB(COLOUR_TEXT_INFO,&r,&g,&b);
							else
								maingui->colourtable.GetRGB(sysgadget==true?COLOUR_TEXT:COLOUR_GREY_LIGHT,&r,&g,&b);

							SetTextColor(gbitmap.hDC, RGB(r, g, b));

							RECT rect;

							rect.left=GetX()+ADDX;
							rect.top=GetY()+1;
							rect.right=rect.left+WIDTH;
							rect.bottom=rect.top+(GetHeight()-2);

							DrawText(gbitmap.hDC, "-", 1, &rect, DT_TOP|DT_SINGLELINE);
						}
						else
						{
							if(mode&MODE_MENU)
							{
								int sx;
								int sx2=GetX2()-1;
								int sy=1;
								int sy2=GetY2()-1;

								if(sy2>sy+maingui->GetFontSizeY_Sub())
									sy2=sy+maingui->GetFontSizeY_Sub();

								sx=sx2-maingui->GetFontSizeY_Sub();

								gbitmap.guiDrawLineY(sy,sx,sx2,COLOUR_GREY);

								int mx=(sx2-sx)/2;

								gbitmap.guiDrawLine(sx,sy,sx+mx,sy2);
								gbitmap.guiDrawLine(sx2,sy,sx+mx,sy2,COLOUR_BLACK);
								//gbitmap.guiFill(sx+mx,sy+(sy2-sy)/2,COLOUR_BACKGROUND);
							}

							if(text)
							{
								if(useextracolour==true)
								{
									UBYTE r,g,b;
									maingui->colourtable.GetRGB(fgcolour,&r,&g,&b);
									SetTextColor(gbitmap.hDC, RGB(r, g, b));
								}
								else
								{
									if(mode&MODE_ALERT)
										SetTextColor(gbitmap.hDC, RGB(255, 100, 100));
									else
										if(mode&MODE_BLACK)
											SetTextColor(gbitmap.hDC, RGB(240, 255, 240));
										else
											if(mode&MODE_TOGGLE)
											{
												if(mode&MODE_GROUP)
												{
													if(mode&MODE_TOGGLED)
													{
														//UBYTE r,g,b;
														//maingui->colourtable.GetRGB(COLOUR_TEXT,&r,&g,&b);
														//SetTextColor(gbitmap.hDC,RGB(r, g, b));

														SetTextColor(gbitmap.hDC, mouseover==true?RGB(124,252,0):RGB(	127,255,0));
														//245,245,220
													}
													else
														SetTextColor(gbitmap.hDC, mouseover==true?RGB(	144,238,144):RGB(69,139,0));
												}
												else
												{
													if(mode&MODE_TOGGLED)
													{
														//UBYTE r,g,b;
														//maingui->colourtable.GetRGB(COLOUR_TEXT,&r,&g,&b);
														//SetTextColor(gbitmap.hDC,RGB(r, g, b));

														SetTextColor(gbitmap.hDC, mouseover==true?RGB(72,209,204):RGB(0,191,255));
														//245,245,220
													}
													else
														SetTextColor(gbitmap.hDC, mouseover==true?RGB(154,192,205):RGB(104,131,139));
												}

											}
											else
												//if((mode&MODE_NOTACTIVE) && mouseover==false)
												//	SetTextColor(gbitmap.hDC, RGB(200, 200, 250));
												//else
											{
												UBYTE r,g,b;

												if(mode&MODE_INFO)
													maingui->colourtable.GetRGB(COLOUR_TEXT_INFO,&r,&g,&b);
												else
												{
													if(mouseover==true)
													{
														maingui->colourtable.GetRGB(sysgadget==true?COLOUR_TEXT:COLOUR_GADGETTEXT_MOUSEOVER,&r,&g,&b);
													}
													else
														maingui->colourtable.GetRGB(sysgadget==true?COLOUR_GADGETTEXT:COLOUR_GREY_LIGHT,&r,&g,&b);
												}

												SetTextColor(gbitmap.hDC, RGB(r, g, b));
											}
								}
								//tobitmap->SetFont(maingui->smallfont);

								//SelectObject(tobitmap->hDC,maingui->smallfont.hfont);
								//SetBkMode(tobitmap->hDC, TRANSPARENT); // Transparent Text etc...

								RECT rect;

								rect.left=GetX()+ADDX;
								rect.top=GetY();
								rect.right=rect.left+WIDTH;
								rect.bottom=rect.top+(GetHeight()-2);

								if(mode&MODE_ADDDPOINT)
								{
									char *h=mainvar->GenerateString(text,":");
									if(h)
									{
										size_t slen=strlen(h);
										DrawText(gbitmap.hDC, h, slen, &rect, (mode&MODE_TEXTCENTER)? DT_SINGLELINE | DT_VCENTER|DT_CENTER:DT_SINGLELINE | DT_VCENTER);
										delete h;
									}
								}
								else
								{
									size_t slen=strlen(text);

									if(mode&MODE_TOGGLE)
									{
										if(mode&MODE_GROUP)
										{
											rect.left+=GetHeight()/2;
										}

										DrawText(gbitmap.hDC, text, slen, &rect, DT_SINGLELINE | DT_CENTER|DT_VCENTER);
									}
									else
									{
										DrawText(gbitmap.hDC, text, slen, &rect, (mode&MODE_TEXTCENTER)? DT_SINGLELINE | DT_VCENTER|DT_CENTER:DT_SINGLELINE | DT_VCENTER);

									}
								}

								if(quicktext && quicktext_onoff==true)
								{
									size_t slen=strlen(quicktext);
									int tw=gbitmap.GetTextWidth(quicktext);

									rect.left=rect.right-tw;

									gbitmap.guiDrawLineX(rect.left-1,1,rect.bottom-1,COLOUR_GREY_DARK);
									DrawText(gbitmap.hDC, quicktext, slen, &rect, DT_SINGLELINE | DT_CENTER|DT_VCENTER);
								}
							}
						}
					}
					break;
			}

			if((mode&MODE_PARENT)==0)
				Blt();

			break;
		}

	default:
		TRACE ("Unknown User Gadget...\n");
		break;
	}
}

guiGadget *guiGadgetList::AddCycle(int x,int y,int w,int h,int gadgetID,int mode,char *text,char *tool)
{
	if(guiGadget *g=new guiGadget)
	{
		g->mode=mode;
		g->x=GetLX();

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=GetLY();
		g->y2=h==0?g->y:y+(h-1);

		//g->x2=g->x+w;
		//g->y=GetLY();
		//g->y2=g->y+h;

		g->gadgetID=gadgetID;
		g->type=GADGETTYPE_CYCLE;

		AddGadget(g);

		//AddLY(maingui->borderhorzsize);

#ifdef WIN32
		int flag=WS_CHILD|WS_VSCROLL|CBS_DROPDOWNLIST|CBS_HASSTRINGS;

		flag|=g->SetVisibleFlag();

		g->hWnd = 
			CreateWindowEx
			(
			WS_EX_WINDOWEDGE, 
			"COMBOBOX", 
			0, 
			flag, 
			g->x, g->y, g->GetWidth(), 32*g->GetHeight(),
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			NULL);

		SendMessage(g->hWnd, WM_SETFONT,(WPARAM)maingui->standardfont.hfont,0);
		//MoveWindow(g->hWnd,g->x,g->y,g->GetWidth(),g->GetHeight()-2,TRUE);

		RECT maxrect;
		GetClientRect(g->hWnd,&maxrect);

		form->lastheight+=maxrect.bottom-maingui->GetButtonSizeY();
#endif	

		g->AddTooltip(tool);

		return g;
	}	

	return 0;
}

guiGadget_ListBox * guiGadgetList::AddListBox(int ox,int y,int w,int h,int gadgetID,int mode,char *tool,bool readonly)
{
	if(guiGadget_ListBox *g=new guiGadget_ListBox(mode))
	{
		// Add Info
		InitXY(g,&y,&w,&h);

		int y=GetLY();

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		//g->x2=w;
		//g->y=y;
		//g->y2=g->y+h;

		g->gadgetID=gadgetID;

		AddGadget(g);

#ifdef WIN32
		//	int iflag=WS_CHILD|WS_VISIBLE|BS_OWNERDRAW;

		//if(g->mode&MODE_HVSLIDER)
		//	flag|=WS_HSCROLL;

		/*
		// Owner
		g->infohWnd = CreateWindowEx
		(
		0,
		CAMX_BUTTONNAME, 
		0,
		iflag,
		g->x, g->y,50, maingui->GetFontSizeY(), 
		win->hWnd,
		(HMENU)g->gadgetindex, 
		maingui->hInst, 
		g);
		*/

		y+=maingui->GetFontSizeY()+2;

		int flag=WS_CHILD|WS_HSCROLL|WS_VSCROLL|LBS_NOINTEGRALHEIGHT|LBS_NOTIFY;

		if(readonly==true)
			flag|=LBS_NOSEL;

		flag|=g->SetVisibleFlag();

		g->hWnd = 
			CreateWindowEx
			(
			0,
			"LISTBOX", 
			"", 
			flag, //style
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			0);

		if(g->hWnd)
		{
			MakeDragList(g->hWnd);
			SendMessage(g->hWnd, WM_SETFONT,(WPARAM)maingui->standardfont.hfont,0);
		}
#endif	

		g->AddTooltip(tool);

		return g;
	}	

	return 0;
}

guiGadget_ListView::guiGadget_ListView(int m)
{
	type=GADGETTYPE_LISTVIEW;
	columc=0;
	mode=m;
	defaultheight=70;
}

void guiGadget_ListView::AddColume(char *string,int widthmul,bool last)
{
	int width=widthmul*maingui->GetFontSizeY();

	guiListViewColum *nc=new guiListViewColum;
	column.AddEndO(nc);

#ifdef WIN32
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH|LVCF_TEXT | LVCF_SUBITEM;

	lvc.iSubItem = columc;
	lvc.pszText = string;
	lvc.cx = width;               // Width of column in pixels.
	lvc.fmt = LVCFMT_LEFT; // Left-aligned column.

	ListView_InsertColumn(hWnd, columc++, &lvc);
#endif
}

void guiGadget_ListView::ClearListView()
{
	SendMessage(hWnd, LVM_DELETEALLITEMS, 0, 0);
	guiListViewColum *col=(guiListViewColum *)column.GetRoot();
	while(col)
	{
		col->FreeMemory();
		col=(guiListViewColum *)col->next;
	}
}

void guiGadget_ListView::SetSelection(int ix)
{
	//ListView_SetItemState(hWnd, -1, 0, LVIS_SELECTED); // deselect all items
	SendMessage(hWnd,LVM_ENSUREVISIBLE ,
		(WPARAM)ix,FALSE); // if item is far, scroll to it

	ListView_SetItemState(hWnd,ix,
		LVIS_SELECTED ,LVIS_SELECTED); // select item
	ListView_SetItemState(hWnd,ix,
		LVIS_FOCUSED ,LVIS_FOCUSED); // optional

	SendMessage(hWnd,LVM_ENSUREVISIBLE ,
		(WPARAM)ix,FALSE); // if item is far, scroll to it

	/*
	ListView_SetItemState (hWnd,         // handle to listview
	ix,         // index to listview item
	LVIS_FOCUSED | LVIS_SELECTED, // item state
	0x000F);                      // mask
	*/

	//ListView_SetSelectionMark(hWnd,ix);
}

guiListViewText::guiListViewText(char *s)
{
	size_t sl=strlen(s)+1;
	string=new char[sl];
	if(string)memcpy(string,s,sl);
}

void guiGadget_ListView::Refresh(int y)
{
	ListView_RedrawItems(hWnd,y,y);
}

void guiGadget_ListView::ChangeText(int x,int y,char *newtext)
{
	guiListViewColum *col=(guiListViewColum *)column.GetO(x);

	if(col)
	{
		guiListViewText *lvt=(guiListViewText *)col->objects.GetO(y);

		if(lvt)
		{
			if(lvt->string)
				delete lvt->string;

			size_t sl=strlen(newtext)+1;
			lvt->string=new char[sl];
			if(lvt->string)memcpy(lvt->string,newtext,sl);
		}
	}
}

void guiGadget_ListView::AddItem(int colindex,char *string)
{
	if(colindex<column.GetCount())
	{
		guiListViewColum *col=(guiListViewColum *)column.GetO(colindex);

		if(col)
		{
			if(guiListViewText *nl=new guiListViewText(string))
			{
				int index=col->objects.GetCount();

				col->objects.AddEndO(nl);

				LVITEM lvI;

				// Initialize LVITEM members that are common to all items.
				lvI.pszText   = LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
				lvI.mask      = LVIF_TEXT | LVIF_IMAGE |LVIF_STATE;
				lvI.stateMask = 0;
				lvI.iSubItem  = colindex;
				lvI.state     = 0;

				lvI.iItem  = index;
				lvI.iImage = index;

				ListView_InsertItem(hWnd, &lvI);
			}
		}
	}
}

void guiListViewColum::FreeMemory()
{
	guiListViewText *vt=(guiListViewText *)objects.GetRoot();

	while(vt)
	{
		if(vt->string)
			delete vt->string;
		vt=(guiListViewText *)objects.RemoveO(vt);
	}
}

void guiGadget_ListView::FreeMemory()
{
	guiListViewColum *col=(guiListViewColum *)column.GetRoot();
	while(col)
	{
		col->FreeMemory();
		col=(guiListViewColum *)column.RemoveO(col);
	}
}

guiGadget_ListView * guiGadgetList::AddListView(int ox,int y,int w,int h,int gadgetID,int mode,char *tool)
{
	if(guiGadget_ListView *g=new guiGadget_ListView(mode))
	{
		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=GetLY();
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=gadgetID;

		AddGadget(g);

#ifdef WIN32
		INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
		icex.dwICC = ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&icex);

		//int lStyle = LVS_LIST  | WS_CHILD | WS_VISIBLE | LVS_SINGLESEL | LVS_SHOWSELALWAYS|LVS_EDITLABELS,LVS_REPORT

		/*
		int xflag=
		//WS_EX_CLIENTEDGE|
		LVS_EX_UNDERLINEHOT|
		LVS_EX_FULLROWSELECT|
		LVS_EX_GRIDLINES;
		*/

		/*
		| LVS_EX_DOUBLEBUFFER 
		| LVS_EX_LABELTIP 
		| LVS_EX_SUBITEMIMAGES 
		| LVS_EX_TRANSPARENTBKGND;
		*/

		int flag=WS_CHILD|LVS_SHOWSELALWAYS|LVS_REPORT|LVS_SINGLESEL;

		flag|=g->SetVisibleFlag();

		g->hWnd = 
			CreateWindowEx
			(
			0,
			WC_LISTVIEW, 
			"", 
			flag, //|  LVS_SINGLESEL|LVS_EDITLABELS, //style
			g->x, g->y, g->GetWidth(), g->GetHeight()/2, 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			0);

		if(g->hWnd)
		{
			// SendMessage(g->hWnd, WM_SETFONT,(WPARAM)maingui->standardfont.hfont,0);
			ListView_SetExtendedListViewStyle(g->hWnd,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_HEADERDRAGDROP|LVS_EX_GRIDLINES);
		}

#endif

		g->AddTooltip(tool);

		return g;
	}	

	return 0;
}

guiGadget *guiGadgetList::AddCheckBox(int x,int y,int w,int h,int gadgetID,int mode,char *text,char *tool)
{
	if(guiGadget *g=new guiGadget)
	{
		g->mode=mode;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=GetLY();
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=gadgetID;
		g->type=GADGETTYPE_CHECKBOX;
		g->index=0; // default unchecked

		AddGadget(g);

#ifdef WIN32
		int flag=WS_CHILD |BS_AUTOCHECKBOX|WS_TABSTOP;

		flag|=g->SetVisibleFlag();

		g->hWnd = CreateWindowEx
			(
			0, 
			CAMX_BUTTONNAME,
			text, 
			flag, 
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, // ID
			maingui->hInst, 
			NULL);

		SendMessage(g->hWnd, WM_SETFONT,(WPARAM)maingui->standardfont.hfont,0);

#endif	
		g->AddTooltip(tool);

		return g;
	}

	return 0;
}

guiGadget *guiGadgetList::AddImageButtonColour(int oldx,int y,int w,int h,int imageID,int gadgetID,int mode,int fgcol,int bgcol,char *tool,int key)
{
	if(guiGadget *g=new guiGadget)
	{
		g->mode=mode;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=gadgetID;
		g->type=GADGETTYPE_BUTTON_IMAGE;
		g->buttontype=imageID;			
		g->guilist=this;

		if(key>=0)
		{
			g->key.SetKey(key);
		}

		g->SetColourNoDraw(fgcol,bgcol);
		AddGadget(g,true);

		if(key>=0)
		{
			char help[8];
			help[0]=' ';
			help[1]='[';
			help[2]=key;
			help[3]=']';
			help[4]=0;

			if(char *h=mainvar->GenerateString(tool,help)){
				g->AddTooltip(h);
				delete h;
			}
		}
		else
			g->AddTooltip(tool);

		return g;
	}

	return 0;
}

guiGadget *guiGadgetList::AddImageButton(int oldx,int y,int w,int h,int imageID,int gadgetID,int mode,char *tool,int key)
{
	if(guiGadget *g=new guiGadget)
	{
		g->mode=mode;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=gadgetID;
		g->type=GADGETTYPE_BUTTON_IMAGE;
		g->buttontype=imageID;			
		g->guilist=this;

		if(key>=0)
		{
			g->key.SetKey(key);
		}

		AddGadget(g,true);

		if(key>=0)
		{
			char help[8];
			help[0]=' ';
			help[1]='[';
			help[2]=key;
			help[3]=']';
			help[4]=0;

			if(char *h=mainvar->GenerateString(tool,help)){
				g->AddTooltip(h);
				delete h;
			}
		}
		else
			g->AddTooltip(tool);

		return g;
	}

	return 0;
}

guiGadget * guiGadgetList::AddText(int oldx,int y,int w,int h,char *text,int gadgetID,int mode,char *tool)
{
	if(guiGadget *g=new guiGadget)
	{
		g->mode=mode;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=gadgetID;
		g->type=GADGETTYPE_BUTTON_TEXT;
		g->staticbutton=true;

		if(text)
			g->text=mainvar->GenerateString(text);

		AddGadget(g,true);

		g->AddTooltip(tool);

		return g;
	}

	return 0;
}

guiGadget *guiGadgetList::AddString(int ox,int y,int w,int h,int gadgetID,int mode,char *name,char *text,char *tool)
{
	if(guiGadget *s=new guiGadget)
	{
		s->mode=mode;

		InitXY(s,&y,&w,&h);

		s->x2=w==0?s->x:s->x+(w-1);
		s->y=GetLY();
		s->y2=h==0?s->y:s->y+(h-1);

		s->type=GADGETTYPE_STRING;
		s->gadgetID=gadgetID;

		s->string=mainvar->GenerateString(text);

		AddGadget(s);

#ifdef WIN32
		int flag=WS_CHILD|ES_AUTOHSCROLL;

		flag|=s->SetVisibleFlag();

		s->hWnd = CreateWindowEx(0, "EDIT", s->string, flag,s->x, s->y,s->GetWidth(),s->GetHeight(), hWnd,(HMENU)s->gadgetindex, maingui->hInst, 0);

		if(s->hWnd)
		{
			/*
			if(text)
			s->SetString(text);
			*/
			SendMessage(s->hWnd, WM_SETFONT,(WPARAM)maingui->standardfont.hfont,0);

			if(mode&MODE_ACTIVATE)
			{
				//	SendMessage(s->hWnd, EM_TAKEFOCUS ,0,0);
			}
		}

#endif

		s->AddTooltip(tool);
		return s;
	}

	return 0;
}

guiGadget_Numerator *guiGadgetList::AddSignatureButton(int oldx,int y,int w,int h,int gadgetID_nn,int gadgetID_dn,int mode)
{
	int startx;
	int tx;
	int xw;

	if(guiGadget_Numerator *ng=new guiGadget_Numerator)
	{
		InitXY(ng,&y,&w,&h);

		startx=GetLX();
		tx=startx;
		xw=w/5;

		// Numerator
		ng->mode=mode|MODE_TEXTCENTER;

		ng->x=tx;
		ng->x2=tx+2*xw;
		ng->y=y;
		ng->y2=h==0?ng->y:ng->y+(h-1);

		ng->gadgetID=gadgetID_nn;
		ng->type=GADGETTYPE_BUTTON_TEXT;

		AddGadget(ng,true);

		tx=ng->x2+ADDXSPACE;

		// /
		if(guiGadget *g=new guiGadget)
		{
			g->mode=MODE_TEXTCENTER;
			g->x=tx;

			tx=startx+3*xw;

			g->x2=tx-ADDXSPACE;
			g->y=y;
			g->y2=h==0?g->y:g->y+(h-1);

			g->gadgetID=0;
			g->type=GADGETTYPE_BUTTON_TEXT;
			g->text=mainvar->GenerateString("/");

			AddGadget(g,true);
		}

		// Denumerator
		if(ng->dn=new guiGadget_Denumerator)
		{
			ng->dn->nn=ng;

			ng->dn->mode=mode|MODE_TEXTCENTER;

			ng->dn->x=startx+3*xw;
			ng->dn->x2=startx+w-1;
			ng->dn->y=y;
			ng->dn->y2=h==0?y:y+(h-1);

			ng->dn->gadgetID=gadgetID_dn;
			ng->dn->type=GADGETTYPE_BUTTON_TEXT;

			AddGadget(ng->dn,true);
		}

		return ng;
	}

	return 0;
}

void guiGadget_CW::Init()
{
	type=GADGETTYPE_DOUBLEBUFFER;
	scroller_h=0;
	skippaint=false;
	mx=my=-1;
}

void guiGadget_CW::DrawOnInit()
{
	callback(this,DB_CREATE);

	//if(formchild->enable==false)
	//	return;

	//callback(this,DB_PAINT);
	//MixSprite();
}

void guiGadget_CW::DrawOnNewSize()
{
	//callback(this,DB_CREATE);

	if(formchild->enable==false)
		return;

	callback(this,DB_NEWSIZE);

	if(skippaint==true)
		skippaint=false;
	else
		callback(this,DB_PAINT);

	MixSprite();
}

void guiGadget_CW::DrawGadgetBlt()
{
	if(formchild->enable==false)
		return;

	if(skippaint==true)
		skippaint=false;
	else
		callback(this,DB_PAINT);
	DrawSpriteBlt();
}

void guiGadget_CW::DrawSpriteBlt()
{
	if(formchild->enable==false)
		return;

	MixSprite();
	Blt();
}

void guiGadget_CW::MixSprite()
{
	if(mode&MODE_SPRITE)
	{
#ifdef DEBUG
		if(gbitmap.width!=mixbitmap.width || gbitmap.height!=mixbitmap.height)
			maingui->MessageBoxError(0,"MixSprite!");

		if(gbitmap.width!=spritebitmap.width || gbitmap.height!=spritebitmap.height)
			maingui->MessageBoxError(0,"MixSprite 2!");
#endif

		// Mix Sprite+Double Buffer -> mixbitmap
		BitBlt(
			mixbitmap.hDC, // handle to destination device context
			0,  // x-coordinate of destination rectangle's upper-left 
			0,  // y-coordinate of destination rectangle's upper-left 
			gbitmap.width,  // width of destination rectangle
			gbitmap.height, // height of destination rectangle
			gbitmap.hDC,  // handle to source device context
			0,   // x-coordinate of source rectangle's upper-left 
			0,   // y-coordinate of source rectangle's upper-left 
			SRCCOPY  // raster operation code
			);

		//spritebitmap.guiFillRectX0(0,spritebitmap.width/2,spritebitmap.height,COLOUR_RED);
		//spritebitmap.guiFillRect(spritebitmap.width/2,0,spritebitmap.width,spritebitmap.height,COLOUR_BLUE);

		BitBlt(
			mixbitmap.hDC, // handle to destination device context
			0,  // x-coordinate of destination rectangle's upper-left 
			0,  // y-coordinate of destination rectangle's upper-left 
			spritebitmap.width,  // width of destination rectangle
			spritebitmap.height, // height of destination rectangle
			spritebitmap.hDC,  // handle to source device context
			0,   // x-coordinate of source rectangle's upper-left 
			0,   // y-coordinate of source rectangle's upper-left 
			SRCPAINT //SRCAND  // raster operation code
			);
	}
}

int guiGadget_CW::GetCursorX()
{
	POINT lpPoint;
	GetCursorPos(&lpPoint);

	BOOL r=ScreenToClient(hWnd,&lpPoint);

	//TRACE ("MX %d\n",lpPoint.x);

	if(r==1)
		return lpPoint.x;

	return -1;
}

int guiGadget::GetMouseX()
{
	return parent?parent->mx:mx;
}

int guiGadget::GetMouseY()
{
	return parent?parent->my:my;
}

void guiGadget::SetMouse(int x,int y)
{
	mx=x;
	my=y;
}

guiObject *guiGadget::CheckObjectClicked()
{
	mx=GetMouseX();
	my=GetMouseY();

	if(mx<0 || mx>GetWidth() || my<0 || my>GetHeight())
		return 0;

	//TRACE ("MX %d MY %d\n",mx,my);

	guiObject *found=0,*o=guilist->win->guiobjects.FirstObject();

	while(o)
	{
		if(o->parent==this)
		{
			//TRACE ("OPARENT %d %d %d %d\n",o->x,o->y,o->x2,o->y2);
			if(o->x<=mx && o->x2>=mx && o->y<=my && o->y2>=my)
			{
				if((!found) || found->idprio<=o->idprio)
					found=o;
			}
		}

		o=o->NextObject();
	}

	return found;
}

guiGadget_Tab *guiGadgetList::AddTab(int oldx,int y,int w,int h,int mode,char *tool,void (*callback) (guiGadget_CW *,int status),void *from)
{
	if(guiGadget_Tab *g=new guiGadget_Tab)
	{
		g->mode=mode;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=5555;
		g->callback=callback;
		g->from=from;

		AddGadget(g,false);
		AddChildGadget(g);

		g->ownergadget=false;

		// Owner
		g->hWnd = CreateWindowEx
			(
			WS_EX_NOPARENTNOTIFY,
			CAMX_CHILDWINDOWNAME, 
			0,
			WS_CHILD|WS_VISIBLE,
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			g);

		g->AddTooltip(tool);

		return g;
	}

	return 0;
}

guiGadget_TabStartPosition *guiGadgetList::AddTabStartPosition(int oldx,int y,int w,int h,int mode,char *tool,void (*callback) (guiGadget_CW *,int status),void *from)
{
	if(guiGadget_TabStartPosition *g=new guiGadget_TabStartPosition)
	{
		g->mode=mode;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=5555;
		g->callback=callback;
		g->from=from;

		AddGadget(g,false);
		AddChildGadget(g);

		g->ownergadget=false;

		// Owner
		g->hWnd = CreateWindowEx
			(
			WS_EX_NOPARENTNOTIFY,
			CAMX_CHILDWINDOWNAME, 
			0,
			WS_CHILD|WS_VISIBLE,
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			g);

		g->AddTooltip(tool);

		return g;
	}

	return 0;
}

void guiGadgetList::AddChildGadget(guiGadget *g)
{
	g_cw[gcwc++]=g;
}

guiGadget_CW *guiGadgetList::AddChildWindow(int oldx,int y,int w,int h,int mode,char *tool,void (*callback) (guiGadget_CW *,int status),void *from)
{
	if(gcwc>MAXCHILDWINDOWSPERWINDOW)
	{
		return 0;
	}

	if(guiGadget_CW *g=new guiGadget_CW(mode))
	{
		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=5555;
		g->callback=callback;
		g->from=from;

		AddGadget(g,false);
		AddChildGadget(g);

		g->ownergadget=false;

		g->hWnd = CreateWindowEx
			(
			WS_EX_NOPARENTNOTIFY,
			CAMX_CHILDWINDOWNAME, 
			0,
			WS_CHILD|WS_VISIBLE,
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			g);

#ifdef DEBUG
		if(!g->hWnd)
			maingui->MessageBoxError(0,"AddChildWindow");
#endif

		g->AddTooltip(tool);

		return g;
	}

	return 0;
}

guiGadget *guiGadgetList::AddButtonColour(int oldx,int y,int w,int h,char *text,int gadgetID,int mode,int fgcol,int bgcol,char *tool)
{
	if(guiGadget *g=new guiGadget)
	{
		g->mode=mode;
		if(text) // Check for Menu
		{
			/*
			char *found1=0;
			size_t i=strlen(text);

			char *c=text;
			while(i--)
			{
			if(*c=='[')
			found1=c;

			if(found1 && *c==']')
			{
			g->menubutton=true;

			g->text=mainvar->GenerateString(found1+1);

			if(g->text)
			{
			size_t i2=strlen(g->text);

			if(i2>1)
			g->text[i2-1]=0;
			}
			goto go;
			}

			c++;
			}
			*/

			g->text=mainvar->GenerateString(text);
		}

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		//int tw=g->GetWidth();
		//int th=g->GetHeight();

		g->gadgetID=gadgetID;
		g->type=GADGETTYPE_BUTTON_TEXT;

		g->SetColourNoDraw(fgcol,bgcol);
		AddGadget(g,true);

		if(mode&MODE_DUMMY)
		{
			g->FreeMemory();
			delete g;
		}
		else
		{
			g->AddTooltip(tool);
			return g;
		}
	}

	return 0;
}

guiGadget *guiGadgetList::AddButton(int oldx,int y,int w,int h,char *text,int gadgetID,int mode,char *tool)
{
	if(guiGadget *g=new guiGadget)
	{
		g->mode=mode;
		if(text) // Check for Menu
		{
			/*
			char *found1=0;
			size_t i=strlen(text);

			char *c=text;
			while(i--)
			{
			if(*c=='[')
			found1=c;

			if(found1 && *c==']')
			{
			g->menubutton=true;

			g->text=mainvar->GenerateString(found1+1);

			if(g->text)
			{
			size_t i2=strlen(g->text);

			if(i2>1)
			g->text[i2-1]=0;
			}
			goto go;
			}

			c++;
			}
			*/

			g->text=mainvar->GenerateString(text);
		}

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		//int tw=g->GetWidth();
		//int th=g->GetHeight();

		g->gadgetID=gadgetID;
		g->type=GADGETTYPE_BUTTON_TEXT;

		AddGadget(g,true);

		if(mode&MODE_DUMMY)
		{
			g->FreeMemory();
			delete g;
		}
		else
		{
			g->AddTooltip(tool);
			return g;
		}
	}

	return 0;
}

void GVolume_Callback(guiGadget_CW *g,int status)
{
	guiGadget_Volume *gt=(guiGadget_Volume *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			gt->DrawGadget();
		}
		break;

	case DB_PAINT:
		{
			gt->DrawGadget();
		}
		break;

	case DB_LEFTMOUSEDOWN:
		gt->LeftMouseDown();
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		{
			gt->MouseMoveLeftDown();
		}
		break;

	case DB_LEFTMOUSEUP:
		gt->LeftMouseUp();
		break;

	case DB_RIGHTMOUSEUP:
		gt->RightMouseUp();
		break;

	case DB_DOUBLECLICKLEFT:
		gt->LeftMouseDoubleClick();
		break;

	case DB_DELTA:
		//ar->DeltaInTracks();
		break;
	}
}

guiGadget_Volume *guiGadgetList::AddVolumeButtonText(int x,int y,int w,int h,char *addtext,int gadgetID,double vol,int flag,char *tool)
{
	if(guiGadget_Volume *g=new guiGadget_Volume)
	{
		g->addtext=mainvar->GenerateString(addtext);

		g->mode=flag;
		g->volume=vol;
		g->gadgetID=gadgetID;
		g->callback=GVolume_Callback;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		AddGadget(g,false);
		AddChildGadget(g);

		g->ownergadget=false;
		g->from=g;

		if(!(g->mode&MODE_PARENT))
			g->hWnd = CreateWindowEx
			(
			WS_EX_NOPARENTNOTIFY,
			CAMX_CHILDWINDOWNAME, 
			0,
			WS_CHILD|WS_VISIBLE,
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			g);

		g->AddTooltip(tool);
		return g;
	}

	return 0;
}

guiGadget_Volume *guiGadgetList::AddVolumeButton(int x,int y,int w,int h,int gadgetID,double vol,int flag,char *tool)
{
	if(guiGadget_Volume *g=new guiGadget_Volume)
	{
		g->mode=flag;
		g->volume=vol;
		g->gadgetID=gadgetID;
		g->callback=GVolume_Callback;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		AddGadget(g,false);
		AddChildGadget(g);

		g->ownergadget=false;
		g->from=g;

		if(!(g->mode&MODE_PARENT))
			g->hWnd = CreateWindowEx
			(
			WS_EX_NOPARENTNOTIFY,
			CAMX_CHILDWINDOWNAME, 
			0,
			WS_CHILD|WS_VISIBLE,
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			g);

		g->AddTooltip(tool);
		return g;
	}

	return 0;
}

void GNumber_Callback(guiGadget_CW *g,int status)
{
	guiGadget_Number *gt=(guiGadget_Number *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			gt->DrawGadget();
		}
		break;

	case DB_PAINT:
		{
			gt->DrawGadget();
		}
		break;

	case DB_LEFTMOUSEDOWN:
		gt->LeftMouseDown();
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		{
			gt->MouseMoveLeftDown();
		}
		break;

	case DB_LEFTMOUSEUP:
		gt->LeftMouseUp();
		break;

	case DB_RIGHTMOUSEUP:
		gt->RightMouseUp();
		break;

	case DB_DOUBLECLICKLEFT:
		gt->LeftMouseDoubleClick();
		break;

	case DB_DELTA:
		//ar->DeltaInTracks();
		break;
	}
}

guiGadget_Number *guiGadgetList::AddNumberButton(int x,int y,int w,int h,int gadgetID,double from,double to,double nr,int type,int flag,char *tool)
{
	if(guiGadget_Number *g=new guiGadget_Number)
	{
		g->mode=flag;
		g->nrtype=type;

		g->vfrom=from;
		g->vto=to;
		g->vnumber=nr;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		g->gadgetID=gadgetID;
		g->callback=GNumber_Callback;

		AddGadget(g,false);
		AddChildGadget(g);

		g->ownergadget=false;
		g->from=g;

		g->hWnd = CreateWindowEx
			(
			WS_EX_NOPARENTNOTIFY,
			CAMX_CHILDWINDOWNAME, 
			0,
			WS_CHILD|WS_VISIBLE,
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			g);

		g->AddTooltip(tool);
		return g;
	}

	return 0;
}

void guiGadgetList::ConvertModeToDock(guiGadget *g)
{
	if((g->mode&MODE_RIGHT) || (g->mode&MODE_MIDTORIGHT))
		g->dock|=DOCK_RIGHT;

	if(g->mode&MODE_TOP)
		g->dock|=DOCK_TOP;
	if(g->mode&MODE_LEFT)
		g->dock|=DOCK_LEFT;
	if(g->mode&MODE_BOTTOM)
		g->dock|=DOCK_BOTTOM;
}

void guiGadgetList::InitXY(guiGadget *g,int *y,int *w,int *h)
{
	ConvertModeToDock(g);

	if(g->mode&MODE_LEFTTOMID)
		g->x=STARTOFGADGETSX;
	else
		if(g->mode&MODE_MIDTORIGHT)
			g->x=form->width/2+ADDXSPACE;
		else
			g->x=GetLX();

	*y=ConvertYTable(g,*y,*h);
	*h=ConvertHTable(g,*h,*y);
	*w=ConvertWTable(g,*w);
}

void GTime_Callback(guiGadget_CW *g,int status)
{
	guiGadget_Time *gt=(guiGadget_Time *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			gt->DrawGadget();
		}
		break;

	case DB_PAINT:
		{
			gt->DrawGadget();
		}
		break;

	case DB_LEFTMOUSEDOWN:
		gt->LeftMouseDown();
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		{
			gt->MouseMoveLeftDown();
		}
		break;

	case DB_LEFTMOUSEUP:
		gt->LeftMouseUp();
		break;

	case DB_RIGHTMOUSEUP:
		gt->RightMouseUp();
		break;

	case DB_DOUBLECLICKLEFT:
		gt->DoubleClickedLeftMouse();
		break;

	case DB_DELTA:
		//ar->DeltaInTracks();
		break;
	}
}


guiGadget_Time *guiGadgetList::AddTimeButton(int oldx,int y,int w,int h,LONGLONG time,int gadgetID,int type,int mode,char *tool)
{
	if(guiGadget_Time *g=new guiGadget_Time)
	{
		if(mode&MODE_STATICTIME)
			g->statictimeformat=true;

		g->ttype=type;
		g->t_time=time;
		g->mode=mode;
		g->callback=GTime_Callback;
		g->gadgetID=gadgetID;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		AddGadget(g,false);
		AddChildGadget(g);

		g->ownergadget=true;
		g->from=g;

		g->hWnd = CreateWindowEx
			(
			WS_EX_NOPARENTNOTIFY,
			CAMX_CHILDWINDOWNAME, 
			0,
			WS_CHILD|WS_VISIBLE,
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			g);

#ifdef DEBUG
		if(!g->hWnd)
			maingui->MessageBoxError(0,"AddChildWindow");
#endif

		g->AddTooltip(tool);

		return g;
	}

	return 0;
}

guiGadget_Time *guiGadgetList::AddLengthButton(int oldx,int y,int w,int h,OSTART pos,OSTART length,int gadgetID,int type,int iflag,char *tool)
{
	if(guiGadget_Time *g=new guiGadget_Time)
	{
		g->islength=true;

		g->ttype=type;
		g->mode=iflag|MODE_LENGTHTIME;
		g->gadgetID=gadgetID;
		g->callback=GTime_Callback;

		g->t_time=pos;
		g->t_length=length;

		InitXY(g,&y,&w,&h);

		g->x2=w==0?g->x:g->x+(w-1);
		g->y=y;
		g->y2=h==0?g->y:y+(h-1);

		AddGadget(g,false);
		AddChildGadget(g);

		g->ownergadget=true;
		g->from=g;

		g->hWnd = CreateWindowEx
			(
			WS_EX_NOPARENTNOTIFY,
			CAMX_CHILDWINDOWNAME, 
			0,
			WS_CHILD|WS_VISIBLE,
			g->x, g->y, g->GetWidth(), g->GetHeight(), 
			hWnd,
			(HMENU)g->gadgetindex, 
			maingui->hInst, 
			g);

		g->AddTooltip(tool);

		return g;
	}

	return 0;
}

/*
guiGadget *guiGadgetList::AddSMPTEButton(int x,int y,int w,int h,OSTART pos,int gadgetID,int flag,char *tool)
{
if(guiGadget *g=new guiGadget)
{
InitXY(g,&y,&w,&h);

g->time=pos;
g->mode=flag;

g->x2=w==0?g->x:g->x+(w-1);
g->y=y;
g->y2=h==0?g->y:y+(h-1);

g->gadgetID=gadgetID;
g->type=GADGETTYPE_SMPTE;

AddGadget(g,true);
g->AddTooltip(tool);
return(g);
}

return 0;
}
*/

guiGadget *guiGadgetList::AddButton(int x,int y,int w,int h,int gadgetID,int flag,char *tool)
{
	return AddButton(x,y,w,h,0,gadgetID,flag,tool);
}

void guiGadgetList::InitForm()
{
	//TRACE (" guiGadgetList::InitForm \n");

	/*
	if(win->formcounter==1)
	hWnd=win->hWnd;
	else
	{
	*/
	hWnd=win->forms[0][0].fhWnd;
	//}
}

void guiGadgetList::FreeObjects(guiForm_Child *form)
{
	for(int i=0;i<gc;i++)
	{
		guiGadget *g=gadgets[i];

		if(g->formchild==form)
		{
			switch(g->type)
			{
			case GADGETTYPE_DOUBLEBUFFER:
				{
					guiGadget_CW *db=(guiGadget_CW *)g;
					db->callback(db,DB_FREEOBJECTS);
				}
				break;
			}
		}
	}
}

void guiGadgetList::Resize(int width,int height,guiForm_Child *form)
{
	// form=0, window size changed
	for(int i=0;i<gc;i++)
	{
		guiGadget *g=gadgets[i];

		if(g->formchild==form)
		{
			if(form->InUse()==false)
			{
				maingui->MessageBoxError(0,"Resize");
			}
			else
			{
				int ox=g->x;
				int oy=g->y;
				int ow=g->GetWidth();
				int oh=g->GetHeight();

				int wx2=width<0?0:width-1;
				int wy2=height<0?0:height-1;

				if(g->mode&MODE_LEFTTOMID)
				{
					g->x2=(wx2/2)-1;
				}

				if(g->mode&MODE_MIDTORIGHT)
					g->x=wx2/2+ADDXSPACE;

				if(g->dock&DOCK_RIGHT)
				{
					g->x2=wx2-g->subw;

					if(g->mode&MODE_STATICWIDTH)
						g->x=g->x2-(g->staticwidth-1);
				}

				if(g->dock&DOCK_BOTTOM)
				{
					g->y2=wy2-g->subh;

					if(g->mode&MODE_STATICHEIGHT)
						g->y=g->y2-(g->staticheight-1);
				}

				int newwidth=g->GetWidth()<0?0:g->GetWidth();
				int newheight=g->GetHeight()<0?0:g->GetHeight();

				g->SetMinWidthAndHeight(&newwidth,&newheight);

				if(ox!=g->x || oy!=g->y || ow!=newwidth || oh!=newheight)
				{
					if(form->fhWnd && g->type!=GADGETTYPE_DOUBLEBUFFER)
					{
						g->eraserect.left=ox;
						g->eraserect.top=oy;
						g->eraserect.right=ox+ow;
						g->eraserect.bottom=oy+oh;

						InvalidateRect(form->fhWnd,&g->eraserect,TRUE);
					}

					if(g->ownergadget==true || g->IsDoubleBuffered()==true)
					{
						if(ow!=newwidth || oh!=newheight)
							g->InitNewDoubleBuffer();
						else
							g->DrawOnInit();

						if(g->type==GADGETTYPE_DOUBLEBUFFER)
						{
							g->Blt();
							MoveWindow(g->hWnd,g->x,g->y,newwidth,newheight,TRUE);
						}
						else
						{
							if(ow!=newwidth || oh!=newheight)
								g->DrawGadget();

							MoveWindow(g->hWnd,g->x,g->y,newwidth,newheight,FALSE);
							InvalidateRect(g->hWnd,NULL,FALSE); // force Repaint/ReBlit
						}

						//ShowWindow(g->hWnd,SW_SHOWNA);

						//TRACE ("Size #### OG %d %d %d %d \n",g->x,g->y,g->GetWidth(),g->GetHeight());	
					}
					else // OS Gadget
					{
						if(g->hWnd)
						{
							if(form->enable==true)
							{
								switch(g->type)
								{
								case GADGETTYPE_WINDOW:
									{
										guiGadget_Win *gwin=(guiGadget_Win *)g;

										bool on=true;

										if(gwin->toggler)
										{
											if(!(gwin->toggler->mode&MODE_TOGGLED))
												on=false;
										}

										if(on==true)
										{
											MoveWindow(g->hWnd,g->x,g->y,newwidth,newheight,FALSE);
											InvalidateRect(g->hWnd,NULL,FALSE); // force Repaint
											ShowWindow(g->hWnd,SW_SHOWNA);

											//UpdateWindow(g->hWnd);
										}
									}
									break;

								default:
									MoveWindow(g->hWnd,g->x,g->y,newwidth,newheight,TRUE); // OS Gadget
									//InvalidateRect(g->hWnd,NULL,FALSE); // force Repaint
									ShowWindow(g->hWnd,SW_SHOWNA);
									//
									//UpdateWindow(g->hWnd);
									break;

								}
							}
						}
						//UpdateWindow(g->hWnd);
					}


					//if(g->hWnd)
					//	ShowWindow(g->hWnd,SW_SHOW);
				}
				else
					if(form->forceredraw==true)
					{
						switch(g->type)
						{
						case GADGETTYPE_DOUBLEBUFFER:
							{
								guiGadget_CW *db=(guiGadget_CW *)g;

								if(db->skippaint==true)
									db->skippaint=false;
								else
									db->callback(db,DB_PAINT);
							}
							break;
						}
					}
			}
		}
	}

	// Refresh Background
	if(form->fhWnd)
		UpdateWindow(form->fhWnd);

	win->InitAutoVScroll();
}

int guiGadgetList::ConvertHTable(guiGadget *g,int h,int ypos)
{
	switch(h)
	{
	case -1:
		return maingui->GetButtonSizeY()+ADDYSPACE;
	}

	if(h<=0)
		if(g->dock&DOCK_BOTTOM)
			h=form->GetY2()-ypos;

	if(h<0)h=0;

	return h;
}

int guiGadgetList::ConvertWTable(guiGadget *g,int w)
{
	int width=w;

	if(g->mode&MODE_LEFTTOMID)
	{
		return form->GetX2()/2-g->x;
	}

	if(g->mode&MODE_MIDTORIGHT)
	{
		return form->GetX2()-g->x;
	}

	if(g->mode&MODE_TEXTWIDTH)
	{
		if(g->text)
		{
			return win->bitmap.GetTextWidth(g->text)+4;
		}

		return 0;
	}

	if(g->mode&MODE_SUBRIGHT)
	{
		g->subw=w;
		width=form->GetX2()-g->x;
		width-=w;
	}
	else
		if(g->mode&MODE_STATICWIDTH)
		{
			g->staticwidth=width=w;
		}
		else
			switch(w)
		{
			case -1:
			case -2:
				width=form->GetX2()-g->x;
				break;
		}

		return width;
}

int guiGadgetList::ConvertYTable(guiGadget *g,int y,int h)
{
	if(y>=0)return y;

	switch(y)
	{
	case VTABLE_1:
		{
			int t=form->height/2;

			h=ConvertHTable(g,h,y);

			t-=h;
			t-=1;

			if(t<0)t=0;

			return t;
		}
		break;

	case VTABLE_2:
		{
			int t=form->height/2;

			t+=1;
			return t;
		}
		break;

	case VTABLE_MIDCENTER:
		{
			int t=form->height/2;

			h=ConvertHTable(g,h,y);

			t-=h/2;
			return t;
		}
		break;
	}

	return form->gy;
}

void guiGadget::Blt()
{
	if(formchild && formchild->deactivated==true)return;

	guiBitmap *source=mode&MODE_SPRITE?&mixbitmap:&gbitmap;

	if(!source->hDC)
		return;

#ifdef WIN32
	BitBlt(
		ghDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left 
		0,  // y-coordinate of destination rectangle's upper-left 
		GetWidth(),  // width of destination rectangle
		GetHeight(), // height of destination rectangle
		source->hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
		0,   // y-coordinate of source rectangle's upper-left 
		SRCCOPY  // raster operation code
		);
#endif

}

void guiGadget::Blt(guiObject *o)
{
	if(o && o->ondisplay==true)
		Blt(o->x,o->y,o->x2,o->y2);
}

void guiGadget::Blt(int x,int y,int x2,int y2)
{
	if(ownergadget==true || type==GADGETTYPE_DOUBLEBUFFER)
	{
		if(formchild && formchild->deactivated==true)
			return;

		if(mode&MODE_SPRITE)
		{
			if(mixbitmap.hDC==0 || spritebitmap.hDC==0 || ghDC==0)
				return;

			int w=GetWidth();
			int h=GetHeight();

			// Mix Sprite+Double Buffer -> mixbitmap
			BitBlt(
				mixbitmap.hDC, // handle to destination device context
				x,  // x-coordinate of destination rectangle's upper-left 
				y,  // y-coordinate of destination rectangle's upper-left 
				w,  // width of destination rectangle
				h, // height of destination rectangle
				gbitmap.hDC,  // handle to source device context
				x,   // x-coordinate of source rectangle's upper-left 
				y,   // y-coordinate of source rectangle's upper-left 
				SRCCOPY  // raster operation code
				);

			//spritebitmap.guiFillRectX0(0,spritebitmap.width/2,spritebitmap.height,COLOUR_RED);
			//spritebitmap.guiFillRect(spritebitmap.width/2,0,spritebitmap.width,spritebitmap.height,COLOUR_BLUE);

			BitBlt(
				mixbitmap.hDC, // handle to destination device context
				x,  // x-coordinate of destination rectangle's upper-left 
				y,  // y-coordinate of destination rectangle's upper-left 
				w,  // width of destination rectangle
				h, // height of destination rectangle
				spritebitmap.hDC,  // handle to source device context
				x,   // x-coordinate of source rectangle's upper-left 
				y,   // y-coordinate of source rectangle's upper-left 
				SRCPAINT //SRCAND  // raster operation code
				);

			BitBlt(
				ghDC, // handle to destination device context
				x,  // x-coordinate of destination rectangle's upper-left 
				y,  // y-coordinate of destination rectangle's upper-left 
				w,  // width of destination rectangle
				h, // height of destination rectangle
				mixbitmap.hDC,  // handle to source device context
				x,   // x-coordinate of source rectangle's upper-left 
				y,   // y-coordinate of source rectangle's upper-left 
				SRCCOPY  // raster operation code
				);

			return;
		}

		if(ghDC==0 || gbitmap.hDC==0)
			return;

#ifdef WIN32
		BitBlt(
			ghDC, // handle to destination device context
			x,  // x-coordinate of destination rectangle's upper-left 
			y,  // y-coordinate of destination rectangle's upper-left 
			GetWidth(),  // width of destination rectangle
			GetHeight(), // height of destination rectangle
			gbitmap.hDC,  // handle to source device context
			x,   // x-coordinate of source rectangle's upper-left 
			y,   // y-coordinate of source rectangle's upper-left 
			SRCCOPY  // raster operation code
			);
	}
#endif
}

void guiGadget::ExitDelta()
{
	if(deltareturn==true)
	{
		startmousex=GetMouseX();
		startmousey=GetMouseY();
	}
}
void guiGadget::InitDelta()
{
	int dy=startmousey-GetMouseY();
	int dx=GetMouseX()-startmousex;

	if(horzslider==true)
		deltay=dx;
	else
		if(mainsettings->mouseonlyvertical==true)
			deltay=dy;
		else
		{
			int hdx=dx<0?-dx:dx;
			int hdy=dy<0?-dy:dy;

			if(hdx>hdy)
				deltay=dx;
			else
				deltay=dy;
		}
}

void guiGadget::InitGetMouseMove()
{
	//	maingui->GetMouseOnScreen(&startmousex,&startmousey);

	if(parent)
	{
		startmousex=parent->mx;
		startmousey=parent->my;
	}
	else
	{
		startmousex=mx;
		startmousey=my;
	}

	getMouseMove=true;
}

void guiGadget::SetToolTip()
{
	if((!ttWnd) && tooltext)
	{
		// Create a ToolTip.
		ttWnd = CreateWindowEx(WS_EX_TOPMOST| WS_EX_TOOLWINDOW,
			TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP|TTS_BALLOON,		
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			hWnd, NULL, maingui->hInst,NULL);

		if(ttWnd)
		{
			SetWindowPos(ttWnd, HWND_TOPMOST,
				0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE|SWP_NOOWNERZORDER);

			// Set up "tool" information.
			// In this case, the "tool" is the entire parent window.
			TOOLINFO ti = { 0 };
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_SUBCLASS;
			ti.hwnd = hWnd;
			ti.hinst = maingui->hInst;
			ti.lpszText = tooltext;

			GetClientRect (hWnd, &ti.rect);

			// Associate the ToolTip with the "tool" window.
			SendMessage(ttWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
		}
	}

	/*
	}
	else
	{
	if(ttWnd)
	{
	DestroyWindow(ttWnd);
	ttWnd=0;
	}
	}
	*/
}

guiGadget *guiGadget::CheckGadgetTimer(int mx,int my,guiObject *o,int iflag)
{
	if(type==GADGETTYPE_WINDOW)
		return this;

	if((!(iflag&CGT_RIGHTMOUSEDOWN)) && rightmousedown==true)
	{
		TRACE ("RB T \n");
		rightmousedown=false;
		RightMouseUp();
	}


	if(IsDoubleBuffered()==false && getMouseMove==true)
	{
		if(iflag&CGT_LEFTMOUSEDOWN)
			MouseMove();
		else
		{
			EndMouseMove();

			if(parent)
				o->Blt();
		}

		return 0;
	}

	if(type==GADGETTYPE_DOUBLEBUFFER)
	{
		return this;
	}

	// Mouse Over
	if((iflag&CGT_INSIDE) && (iflag&CGT_LEFTMOUSEDOWN) && mouseover==true)
	{
		if(mode&MODE_PUSH)
		{
			if(pushed==false && GetFocus()==hWnd)
			{
				pushed=true;
				DrawGadget();

				if(parent)
					o->Blt();

				guilist->win->Gadget(this);
			}
		}
		else
			switch(type)
		{
			case GADGETTYPE_BUTTON_NUMBER:
			case GADGETTYPE_BUTTON_VOLUME:
				{
					InitGetMouseMove();
					DrawGadget();

					if(parent)
						o->Blt();

					return 0;
				}
				break;
		}

		if(pushed==true)
			guilist->win->Gadget(this);

		return this;
	}

	if(pushed==true && (!(iflag&CGT_LEFTMOUSEDOWN)) )
	{
		pushed=false;
		DrawGadget();

		if(parent)
			o->Blt();
	}

	if(pushed==true)
		guilist->win->Gadget(this);

	if((iflag&CGT_INSIDE) && (iflag&CGT_RIGHTMOUSEDOWN))
	{
		// Right Down
		if(rightmousedown==false)
		{
			rightmousedown=true;
			RightMouseDown();

			return 0;
		}
	}
	else
	{
		if(mouseover==false && (iflag&CGT_INSIDE) && (!(iflag&CGT_LEFTMOUSEDOWN)) && (!(mode&MODE_NOMOUSEOVER)))
		{
			mouseover=true;
			MouseOver(mx,my);

			if(parent)
				o->Blt();
		}
		else
			if(mouseover==true)
			{
				if((iflag&CGT_INSIDE) /*&& CheckMouseOver(mx,my)==true*/)
					CheckMouseOverStatus(mx,my);
				else
				{
					mouseover=false;
					MouseOver(mx,my);

					if(parent)
						o->Blt();
				}
			}
	}

	return this;
}

bool guiGadget::IsSystemGadget()
{
	switch(type)
	{
	case GADGETTYPE_CHECKBOX:
	case GADGETTYPE_CYCLE:
	case GADGETTYPE_INTEGER:
	case GADGETTYPE_LISTBOX:
	case GADGETTYPE_LISTVIEW:
	case GADGETTYPE_SLIDER:
	case GADGETTYPE_STRING:
		return true;
		break;
	}

	return false;
}

void guiGadget::Toggle(bool on)
{
	Enable();

	if(mode&MODE_TOGGLE)
	{
		if(on==true)
		{
			if(!(mode&MODE_TOGGLED))
			{
				mode|=MODE_TOGGLED;
				DrawGadget();
			}
		}
		else
		{
			if(mode&MODE_TOGGLED)
			{
				mode CLEARBIT MODE_TOGGLED;
				DrawGadget();
			}
		}
	}
}

void guiGadget::SetKeyFocus()
{
#ifdef WIN32
	SetFocus(hWnd);
	//SetCapture(hWnd);
#endif
}

void guiGadget::SetButtonQuickText(char *text,bool on)
{
	quicktext_onoff=on;

	if(quicktext)
		delete quicktext;

	quicktext=mainvar->GenerateString(text);

	if(on==true)
		DrawGadget();
}

void guiGadget::SetButtonQuickOn(bool on)
{
	if(quicktext_onoff!=on)
	{
		quicktext_onoff=on;
		DrawGadget();
	}
}

void guiGadget::On()
{
	on=true;
	DrawGadget();
}

void guiGadget::Off()
{
	on=false;
	DrawGadget();
}

/*
int guiGadget::GetMouseX()
{
if(parent)
{
return parent->mx;
}


POINT lpPoint;

GetCursorPos(&lpPoint);
BOOL r=ScreenToClient(hWnd,&lpPoint);

//TRACE ("MX %d\n",lpPoint.x);

if(r==1)
return lpPoint.x;

return -1;
}

int guiGadget::GetMouseY()
{
if(parent)
{
return parent->my;
}

POINT lpPoint;

GetCursorPos(&lpPoint);
BOOL r=ScreenToClient(hWnd,&lpPoint);

if(r==1)
return lpPoint.y;

return -1;
}
*/

int guiGadget::GetDefaultTimeWidth()
{
	return gbitmap.prefertimebuttonsize+gbitmap.prefertimebuttonsize/4;
}

void guiGadget::InitNewDoubleBuffer()
{
	if(mode&MODE_PARENT)
		return;

	skippaint=false;

	if(gbitmap.hBitmap)
	{
		DeleteObject(gbitmap.hBitmap);
		gbitmap.hBitmap=0;
	}

	if(textinit==false)
	{
		textinit=true;

		// Init Text etc..
		switch(type)
		{
		case GADGETTYPE_DOUBLEBUFFER:
			{
				gbitmap.SetFont(&maingui->standardfont);
				gbitmap.InitTextWidth();

				SetBkMode(gbitmap.hDC, TRANSPARENT); // Transparent Text etc...
			}break;

		case GADGETTYPE_BUTTON_NUMBER:
		case GADGETTYPE_BUTTON_VOLUME:
			{
				gbitmap.SetFont(&maingui->standardfont);
				SetBkMode(gbitmap.hDC, TRANSPARENT); // Transparent Text etc...
			}
			break;

		case GADGETTYPE_TIME:
			{
				gbitmap.SetFont(&maingui->standard_bold);
				SetBkMode(gbitmap.hDC, TRANSPARENT); // Transparent Text etc...
			}
			break;

		case GADGETTYPE_BUTTON_TEXT:
		case GADGETTYPE_BUTTON_IMAGE:
			{
				gbitmap.SetFont((mode&MODE_BOLD)?&maingui->standard_bold:&maingui->standardfont);
				gbitmap.InitTextWidth();
				SetBkMode(gbitmap.hDC, TRANSPARENT); // Transparent Text etc...
			}
			break;
		}
	}

	if( (gbitmap.width=GetWidth())>0 && (gbitmap.height=GetHeight())>0)
	{
		// Init or NEW Size
		gbitmap.hBitmap=CreateCompatibleBitmap(
			ghDC,        // handle to device context
			gbitmap.width,     // width of bitmap, in pixels
			gbitmap.height     // height of bitmap, in pixels
			);

		if(gbitmap.hBitmap)
			SelectObject(gbitmap.hDC,gbitmap.hBitmap);
	}
#ifdef DEBUG
	else
		maingui->MessageBoxOk(0,"InitNewDoubleBuffer");
#endif

	if(mode&MODE_SPRITE) // Tripple Buffer
	{
		if(spritebitmap.hBitmap)
		{
			DeleteObject(spritebitmap.hBitmap);
			spritebitmap.hBitmap=0;
		}

		if( (spritebitmap.width=GetWidth())>0 && (spritebitmap.height=GetHeight())>0)
		{
			// Init or NEW Size
			spritebitmap.hBitmap=CreateCompatibleBitmap(
				ghDC,        // handle to device context
				spritebitmap.width,     // width of bitmap, in pixels
				spritebitmap.height     // height of bitmap, in pixels
				);

			if(spritebitmap.hBitmap)
				SelectObject(spritebitmap.hDC,spritebitmap.hBitmap);
		}

		// MixBitmap Triple Buffer
		if(mixbitmap.hBitmap)
		{
			DeleteObject(mixbitmap.hBitmap);
			mixbitmap.hBitmap=0;
		}

		if( (mixbitmap.width=GetWidth())>0 && (mixbitmap.height=GetHeight())>0)
		{
			// Init or NEW Size
			mixbitmap.hBitmap=CreateCompatibleBitmap(
				ghDC,        // handle to device context
				mixbitmap.width,     // width of bitmap, in pixels
				mixbitmap.height     // height of bitmap, in pixels
				);

			if(mixbitmap.hBitmap)
				SelectObject(mixbitmap.hDC,mixbitmap.hBitmap);
		}
	}

	if(gadgetinit==false)
		DrawOnInit();
	else
		DrawOnNewSize();

	gadgetinit=true;
}

void guiGadgetList::AddGadget(guiGadget *g,bool ownergadget)
{
	if(gc>=1024)
	{
		maingui->MessageBoxError(0,"AddGadget");
		return;
	}

	g->ownergadget=ownergadget;
	g->guilist=this;

	if(!(g->mode&MODE_PARENT))
	{
		gadgets[gc]=g;
		g->gadgetindex=gc++;
	}

	g->formchild=form;
	//g->formchild->InitID(gc++);

	if(g->x+g->GetWidth()>form->maxx2)
		form->maxx2=g->x+g->GetWidth();

	if(g->y+g->GetHeight()>form->maxy2)
		form->maxy2=g->y+g->GetHeight();

	if(g->GetHeight()>form->lastheight)
	{
		switch(g->type)
		{
		case GADGETTYPE_WINDOW:
			{
				guiGadget_Win *gwin=(guiGadget_Win *)g;

				if(gwin->toggler && (!(gwin->toggler->mode&MODE_TOGGLED)))
				{
					TRACE ("Skip Height %d\n", g->GetHeight());
					goto skip;
				}
			}
			break;
		}
		//if((!g->toggler) || (g->to
		form->lastheight=g->GetHeight();
	}

skip:
	if(g->mode&MODE_PARENT)
	{
		g->parent=formgadget;

		g->hWnd=formgadget->hWnd;
		g->gbitmap.hBitmap=formgadget->gbitmap.hBitmap;
		g->gbitmap.hDC=formgadget->gbitmap.hDC;

		//g->gbitmap.hDC=form->h
		g->DrawGadget();
	}
	else
		if(ownergadget==true)
		{
			// Owner
			g->hWnd = CreateWindowEx
				(
				0,
				CAMX_BUTTONNAME, 
				0,
				WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,
				g->x, g->y, g->GetWidth(),g->GetHeight(), 
				hWnd, // parent
				(HMENU)g->gadgetindex, 
				maingui->hInst, 
				NULL);
		}

		/*
		else
		if(g->type!=GADGETTYPE_DOUBLEBUFFER && g->IsDoubleBuffered()==true)
		{
		g->hWnd = CreateWindowEx
		(
		0,
		CAMX_BUTTONNAME, 
		0,
		WS_CHILD|WS_VISIBLE,
		g->x, g->y, g->GetWidth(),g->GetHeight(), 
		hWnd, // parent
		(HMENU)g->gadgetindex, 
		maingui->hInst, 
		NULL);
		}
		*/

}

void guiGadgetList::Enable(guiGadget *g,bool onoff)
{
	if(!g)return;

	if(onoff==true)
		g->Enable();
	else
		g->Disable();
}

void guiGadgetList::RemoveGadget(int from,int to)
{
	if(from>=0)
	{
		for(int i=from;i<=to;i++)
		{
			if(gadgets[i])
			{
				gadgets[i]->FreeMemory();
				delete gadgets[i];
			}
		}
		//memcpy(&gadgets[from],&gadgets[to+1],sizeof(guiGadget)*rest);
	}
}

guiGadgetList::guiGadgetList()
{
	dontdelete=false;
	gc=gcwc=0;
	win=0;
	scrollh_pos=scrollv_pos=0;
}

int guiGadgetList::GetMaxHeight()
{
	int hy=0;

	for(int i=0;i<gc;i++)
	{
		guiGadget *g=gadgets[i];

		int hg=0;

		switch(g->type)
		{
		case GADGETTYPE_WINDOW:
			{
				guiGadget_Win *gwin=(guiGadget_Win *)g;

				if(gwin->toggler && gwin->win && (gwin->toggler->mode&MODE_TOGGLED)) // Open ?
				{
					hg=gwin->win->GetWinHeight();

					int min=gwin->win->GetInitHeight();

					if(min>hg)
						hg=min;
				}

			}
			break;

		default:
			hg=g->GetHeight();
			break;
		}

		if(g->y+hg>hy)
			hy=g->y+hg;

		if(g->group && (!(g->mode&MODE_TOGGLED)) )
			i++;
	}

	return hy;
}

void guiGadget::DeleteDoubleBuffers()
{
	if(!(mode&MODE_PARENT))
	{
		if(ghDC)
		{
			DeleteDC(ghDC);
			ghDC=0;
		}

		if(gbitmap.hDC)
		{
			DeleteDC(gbitmap.hDC);
			gbitmap.hDC=0;
		}

		if(gbitmap.hBitmap)
		{
			DeleteObject(gbitmap.hBitmap);
			gbitmap.hBitmap=0;
		}

		if(spritebitmap.hDC)
		{
			DeleteDC(spritebitmap.hDC);
			spritebitmap.hDC=0;
		}

		if(spritebitmap.hBitmap)
		{
			DeleteObject(spritebitmap.hBitmap);
			spritebitmap.hBitmap=0;
		}

		if(mixbitmap.hDC)
		{
			DeleteDC(mixbitmap.hDC);
			mixbitmap.hDC=0;
		}

		if(mixbitmap.hBitmap)
		{
			DeleteObject(mixbitmap.hBitmap);
			mixbitmap.hBitmap=0;
		}
	}
}

void guiGadgetList::RemoveGadget(guiGadget *f)
{
	if(!f)
	{
		// Child Window ... VST etc...
		return;
	}

	f->DeleteDoubleBuffers();
	//f->FreeString();
	f->FreeMemory();

#ifdef WIN32
	//	if(win->close==false) // Delete Single Gadget ?
	{
		/*
		if(f->ttWnd) // tooltip
		{
		//guilist->toolwnd[guilist->oldwindelete]=f->ttWnd;
		DestroyWindow(f->ttWnd);
		f->ttWnd=0;
		}

		if(!(f->mode&MODE_PARENT))
		{
		if(f->hWnd)
		{
		//guilist->wnd[guilist->oldwindelete++]=f->hWnd;
		DestroyWindow(f->hWnd);
		f->hWnd=0;
		}
		}
		*/
	}

#endif

	if(!(f->mode&MODE_PARENT))
		gadgets[f->gadgetindex]=0;

	delete f;
}

void guiGadgetList::RemoveGadgetList()
{
	for(int i=0;i<gc;i++)
		RemoveGadget(gadgets[i]);
	gc=0;
}

void guiGadgetList::SelectForm(guiGadget_CW *g)
{
	form=g->formchild;
	formgadget=g;

	hWnd=form->fhWnd?form->fhWnd:win->hWnd;
}

// Grid
guiForm_Child *guiGadgetList::SelectForm(int x,int y)
{
	//maxx2=0;
	//maxy2=0;

	// plain to window
	form_h=x;
	form_v=y;

	//InitLXY();

	if(win)
	{
		form=&win->forms[x][y];
		hWnd=form->fhWnd?form->fhWnd:win->hWnd;
		return GetActiveForm();
	}

	return 0;
}

void guiGadgetList::NextHForm()
{
	Return();

	//int ly=GetLY();
	//win->forms[form_h][form_v].maxx2=maxx2;
	//win->forms[form_h][form_v].maxy2=maxy2;

	// Add V Splitter
	if(win->forms_horz>form_h+1)
	{
		if(win->forms[form_h+1][form_v].deactivated==false)
			SelectForm(form_h+1,form_v);
	}
}

void guiGadgetList::NextVForm()
{
	Return();

	//int ly=GetLY();
	//win->forms[form_h][form_v].maxx2=maxx2;
	//win->forms[form_h][form_v].maxy2=maxy2;

	// Add V Splitter
	if(win->forms_vert>form_v+1)
	{
		if(win->forms[form_h][form_v+1].deactivated==false)
			SelectForm(form_h,form_v+1);
	}
}

void guiGadget_Tab::ClearTab()
{
	//gbitmap.guiFillRect(COLOUR_UNUSED);
	preflist.DeleteAllO();

	if(guilist && guilist->win)
		guilist->win->DeleteAllNumberObjects(this);
}

void guiGadget_Tab::InitTabs(int ts)
{
	tabs=ts;
	for(int i=0;i<ts;i++)
		tabwidth[i]=-1;
}

void guiGadget_Tab::InitTabWidth(int index,int width)
{
	if(index<16 && index<tabs)
	{
		tabwidth[index]=width;
	}
}

void guiGadget_Tab::InitTabWidth(int index,char *text)
{
	InitTabWidth(index,gbitmap.GetTextWidth(text));
}

void guiGadget_Tab::InitXX2()
{
	int x=0;

	for(int i=0;i<tabs;i++)
	{
		tabx[i]=x;
		tabx2[i]=tabwidth[i]==-1?x2:x+tabwidth[i]-1;

		x+=tabwidth[i];
	}
}

int guiGadget_Tab::GetTabX2(int index)
{
	if(index>=tabs-1)
		return gbitmap.GetX2();

	return tabx2[index];
}


int guiGadget_Tab::GetMouseClickTabIndex()
{
	for(int i=0;i<tabs;i++)
	{
		if(mx>=GetTabX(i) && mx<=GetTabX2(i))
			return i;
	}

	return -1;
}

void guiGadget::SetStartMouseY()
{
	editmousey=GetMouseY();
}

int guiGadget::InitDeltaY()
{
	if(!guilist->win->editobject)
	{
		TRACE ("Edit Object 0 \n");
		//return 0;
	}

	int mmy=GetMouseY();

	deltay=editmousey-mmy;
	editmousey=mmy;

	if(deltay)
	{
		double h=deltay;
		h*=editstepsy;

		guilist->win->editsum+=h;

		TRACE ("Edit Sum %f\n",guilist->win->editsum);
	}

	return deltay;
}

void guiGadget::EndEdit()
{
	guilist->win->editobject=0;
	guilist->win->editsum=0;

	edittabx=-1;
}

Object *guiGadget_TabStartPosition::InitEdit(OListCoosY *list,double ysteps)
{
	int mx=GetMouseX();

	edittabx=-1;

	guilist->win->editobject=0;
	guilist->win->editsum=0;

	SetEditSteps(ysteps);
	SetStartMouseY();

	// X
	for(int i=0;i<tabs;i++)
	{
		if(mx>=tabx[i] && mx<=tabx2[i])
		{
			edittabx=i;
			break;
		}
	}

	if(edittabx==-1)
		return 0;

	// Y Object
	if(list)
	{
		int mmy=GetMouseY();

		list->InitYStartO();

		if(list->GetShowObject()) // first track ?
		{
			OObject *so;

			while((so=list->GetShowObject()) && list->GetInitY()<GetHeight())
			{
				int sy=so->cy;
				int sy2=sy+so->ch;

				sy-=list->starty;
				sy2-=list->starty;

				if(mmy>=sy && mmy<=sy2)
				{
					guilist->win->editobject=so->object;

					TRACE ("EditObject set \n");

					return guilist->win->editobject;
				}

				list->NextYO();
			}
		}
	}

	return 0;
}

void guiGadget_TabStartPosition::InitStartPosition(int tabindex)
{
	tabstart=tabindex;

	for(int i=0;i<4;i++)
		timewidth_time[i]=gbitmap.pref_time[i];

	for(int i=0;i<5;i++)
		timewidth_smpte[i]=gbitmap.pref_smpte[i];

	startinit=true;
}

void guiGadgetList::OpenGroup(guiWindow *gr,guiGadget *gr_Toggler,int flag,Seq_Song *song)
{
	if(guiGadget_Win *gwin=new guiGadget_Win(gr))
	{
		gr->SetSong(song);

		gr->parentwindow=win;
		gwin->mode=flag;

		gr_Toggler->group=gwin;
		gwin->toggler=gr_Toggler;

		int y=-1,w=-1,h=gr->GetInitHeight();

		InitXY(gwin,&y,&w,&h);

		gwin->x2=gwin->x+w;
		gwin->y=y;
		gwin->y2=h==0?y:y+(h-1);

		maingui->AddWindow(gr);

		AddGadget(gwin);

		gwin->hWnd=CreateWindowEx
			(
			gr->flagex=WS_EX_NOPARENTNOTIFY,
			CAMX_WINDOWNAME, 
			0,
			gr->style=WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
			gwin->x, gwin->y,gwin->GetWidth(),gwin->GetHeight(),
			hWnd,
			(HMENU)0, // Screen Menu Global
			maingui->hInst,
			gr
			);

		if(gwin->hWnd && (gr_Toggler->mode&MODE_TOGGLED))
		{
			UpdateWindow(gwin->hWnd);
			ShowWindow(gwin->hWnd,SW_SHOWNA);

			//	SetTimer(gwin->hWnd, NULL,USER_TIMER_MINIMUM ,NULL);
		}
	}
}

void guiGadgetList::GroupToggled(guiGadget *g)
{
	if(g && g->group)
	{
		if(g->mode&MODE_TOGGLED)
		{
			if(g->group->hWnd)
			{
				int scrolly=g->group->GetHeight();

				for(int i=g->gadgetindex+2;i<gc;i++) // index+1, next is group
				{
					guiGadget *ng=gadgets[i];

					if(ng && ng->hWnd)
					{
						ng->y+=scrolly;
						ng->y2+=scrolly;

						MoveWindow(ng->hWnd,ng->x,ng->y,ng->GetWidth()<0?0:ng->GetWidth(),ng->GetHeight()<0?0:ng->GetHeight(),TRUE);
					}
				}

				int newwidth=g->group->GetWidth();
				int newheight=g->group->GetHeight();

				if(g->group->win)
				{
					g->group->SetMinWidthAndHeight(&newwidth,&newheight);

					// Force Refresh
					g->group->win->RefreshRealtime();
					g->group->win->RefreshRealtime_Slow();
				}

				MoveWindow(g->group->hWnd,g->group->x,g->group->y,newwidth<0?0:newwidth,newheight<0?0:newheight,TRUE);

				UpdateWindow(g->group->hWnd);
				ShowWindow(g->group->hWnd,SW_SHOWNA);

				//	SetTimer(g->group->hWnd, NULL,USER_TIMER_MINIMUM ,NULL);

			}
		}
		else
		{
			if(g->group->hWnd) // Close
			{
				//KillTimer(g->group->hWnd,NULL);
				ShowWindow(g->group->hWnd,SW_HIDE);

				int scrolly=g->group->GetHeight();

				for(int i=g->gadgetindex+2;i<gc;i++) // index+1, next is group
				{
					guiGadget *ng=gadgets[i];

					if(ng && ng->hWnd)
					{
						ng->y-=scrolly;
						ng->y2-=scrolly;

						MoveWindow(ng->hWnd,ng->x,ng->y,ng->GetWidth()<0?0:ng->GetWidth(),ng->GetHeight()<0?0:ng->GetHeight(),TRUE);
					}
				}
			}
		}

		win->ReDraw(true);
	}
}