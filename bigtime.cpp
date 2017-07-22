#include "songmain.h"
#include "camxgadgets.h"
#include "camximages.h"
#include "guimenu.h"
#include "bigtime.h"
#include "settings.h"
#include "gui.h"
#include "object_song.h"
#include "globalmenus.h"
#include "languagefiles.h"
#include "object_project.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIGTIME_FRAMEID_TIME 1

class BigNumber
{
public:
	virtual void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour){}
};

class BigNumber1:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		double h=x2-x;

		h/=2.2;

		bm->guiFillRect(x2-(int)h,y,x2,y2,colour);
	}
};

class BigNumber2:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x,y,x2,y+hy,colour);
		bm->guiFillRect(x2-hx,y,x2,y+3*hy,colour);
		bm->guiFillRect(x,y+2*hy,x2,y+3*hy,colour);
		bm->guiFillRect(x,y+3*hy,x+hx,y2,colour);
		bm->guiFillRect(x,y+4*hy,x2,y2,colour);
	}
};

class BigNumber3:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x,y,x2,y+hy,colour);
		bm->guiFillRect(x2-hx,y,x2,y2,colour);
		bm->guiFillRect(x+hx,y+2*hy,x2,y+3*hy,colour);
		bm->guiFillRect(x,y+4*hy,x2,y2,colour);
	}
};

class BigNumber4:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x2-hx,y,x2,y2,colour);
		bm->guiFillRect(x,y,x+hx,y+2*hy,colour);
		bm->guiFillRect(x,y+2*hy,x2,y+3*hy,colour);
	}
};

class BigNumber5:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x,y,x2,y+hy,colour);
		bm->guiFillRect(x,y+2*hy,x2,y+3*hy,colour);
		bm->guiFillRect(x,y+4*hy,x2,y2,colour);

		bm->guiFillRect(x,y,x+hx,y+2*hy,colour);
		bm->guiFillRect(x2-hx,y+3*hy,x2,y2,colour);
	}
};

class BigNumber6:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x,y,x2,y+hy,colour);
		bm->guiFillRect(x,y,x+hx,y2,colour);
		bm->guiFillRect(x,y+2*hy,x2,y+3*hy,colour);

		bm->guiFillRect(x,y2-hy,x2,y2,colour);
		bm->guiFillRect(x2-hx,y+3*hy,x2,y2,colour);
	}
};

class BigNumber7:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x,y,x2,y+hy,colour);
		bm->guiFillRect(x2-hx,y,x2,y2,colour);
		bm->guiFillRect(x2-(hx*3),y+2*hy,x2,y+3*hy,colour);
	}
};

class BigNumber8:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x,y,x2,y+hy,colour);
		bm->guiFillRect(x2-hx,y,x2,y2,colour);
		bm->guiFillRect(x,y,x+hx,y2,colour);
		bm->guiFillRect(x,y+2*hy,x2,y+3*hy,colour);
		bm->guiFillRect(x,y2-hy,x2,y2,colour);
	}
};

class BigNumber9:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x,y,x2,y+hy,colour);
		bm->guiFillRect(x2-hx,y,x2,y2,colour);
		bm->guiFillRect(x,y,x+hx,y+3*hy,colour);
		bm->guiFillRect(x,y+2*hy,x2,y+3*hy,colour);
		bm->guiFillRect(x,y2-hy,x2,y2,colour);
	}
};

class BigNumber0:public BigNumber
{
public:
	void Draw(guiBitmap *bm,int x,int y,int x2,int y2,int colour)
	{
		int hx=(x2-x)/3;
		int hy=(y2-y)/5;

		bm->guiFillRect(x,y,x2,y+hy,colour);
		bm->guiFillRect(x2-hx,y,x2,y2,colour);
		bm->guiFillRect(x,y,x+hx,y2,colour);
		bm->guiFillRect(x,y2-hy,x2,y2,colour);
	}
};

struct BigNumbers
{
public:
	BigNumbers()
	{
		nr[0]=&bt0;
		nr[1]=&bt1;
		nr[2]=&bt2;
		nr[3]=&bt3;
		nr[4]=&bt4;
		nr[5]=&bt5;
		nr[6]=&bt6;
		nr[7]=&bt7;
		nr[8]=&bt8;
		nr[9]=&bt9;
	}

	BigNumber0 bt0;
	BigNumber1 bt1;
	BigNumber2 bt2;
	BigNumber3 bt3;
	BigNumber4 bt4;
	BigNumber5 bt5;
	BigNumber6 bt6;
	BigNumber7 bt7;
	BigNumber8 bt8;
	BigNumber9 bt9;

	BigNumber *nr[10];
};

void Edit_BigTime::ShowTime()
{
	if(!time)
		return;

	guiBitmap *bitmap=&time->gbitmap;

	if(WindowSong())
	{
		if(WindowSong()->status&Seq_Song::STATUS_WAITPREMETRO)
		{
			int precountertodo=mainsettings->numberofpremetronomes-WindowSong()->metronome.GetPreCounterDone();

			if(precounter!=precountertodo)
			{
				precounter=precountertodo;

				bitmap->guiFillRect(COLOUR_WHITE);

				double dx2;
				int addx,hx=width;

				hx/=4;

				dx2=hx;

				dx2*=0.9;

				addx=hx;
				hx=(int)dx2;

				int x=0;

				BigNumbers number;

				int h=mainsettings->numberofpremetronomes;

				h/=10;
				number.nr[h]->Draw(bitmap,x,height/2,x+hx,height,COLOUR_GREY);
				h=mainsettings->numberofpremetronomes-(10*h);
				x+=addx;
				number.nr[h]->Draw(bitmap,x,height/2,x+hx,height,COLOUR_GREY);
				x+=addx;

				// Pre Measure
				h=precounter;
				h/=10;
				number.nr[h]->Draw(bitmap,x,0,x+hx,height,COLOUR_BLACK);
				h=precounter-10*h;
				x+=addx;
				number.nr[h]->Draw(bitmap,x,0,x+hx,height,COLOUR_BLACK);
			}
		}
		else
		{
			precounter=-1;

			BigNumbers number;
			activesongposition=WindowSong()->GetSongPosition();

			int bgcol,fgcol;

			bgcol=COLOUR_BLACK;
			
			bitmap->guiFillRect(bgcol);

			if(windowtimeformat==WINDOWDISPLAY_MEASURE)
			{
				fgcol=COLOUR_TEXT;

				Seq_Pos pos(Seq_Pos::POSMODE_NORMAL);

				WindowSong()->timetrack.ConvertTicksToPos(activesongposition,&pos);

				pos.Clone(&realtimepos);

				if(pos.pos[0]>=100000) // MAX 99999
					return;

				int x=0;

				// Measure
				// --- 
				int addx,hx=width;

				if(pos.pos[0]>=10000)
					hx/=13;
				else
					hx/=12;

				int hhelp=height;
				int midy=hhelp/2,hz=hhelp/6;
				int pointy=bitmap->GetY2()-(hz+1);

				midy-=hz/2;
				double dhx=hx,addminus;

				dhx*=0.9;

				addx=hx;
				hx=(int)dhx;

				addminus=addx;
				addminus*=1.15;

				int colour=COLOUR_GREY_DARK;

				// Measure

				int mouseovercolour=COLOUR_GREEN;

				posx[0]=x;

				LONGLONG h;

				if(pos.pos[0]>=10000)
				{
					h=pos.pos[0]/10000;

					colour=fgcol;
					number.nr[h]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==0?mouseovercolour:colour);
					pos.pos[0]-=h*10000;
					x+=addx;
				}
				
				h=pos.pos[0]/1000;

				if(pos.pos[0]>=1000)
					colour=fgcol;

				number.nr[h]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==0?mouseovercolour:colour);

				pos.pos[0]-=h*1000;
				x+=addx;

				if(pos.pos[0]>=100)
					colour=fgcol;

				h=pos.pos[0]/100;

				number.nr[h]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==0?mouseovercolour:colour);

				pos.pos[0]-=h*100;
				x+=addx;

				if(pos.pos[0]>=10 )
					colour=fgcol;

				h=pos.pos[0]/10;

				number.nr[h]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==0?mouseovercolour:colour);

				pos.pos[0]-=h*10;
				x+=addx;

				number.nr[(int)pos.pos[0]]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==0?mouseovercolour:fgcol);
				x+=addx;

				// -
				bitmap->guiFillRect(x+hx/4,pointy,x+hx-hx/4,pointy+hz,fgcol);

				posx2[0]=x;

				x+=addx;

				// Beat
				posx[1]=x;

				if(pos.pos[1]>9)
				{
					LONGLONG h=pos.pos[1];
					h/=10;

					number.nr[h]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==1?mouseovercolour:fgcol);
					x+=addx;

					pos.pos[1]-=10*h;
				}

				number.nr[(int)pos.pos[1]]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==1?mouseovercolour:fgcol);
				x+=addx;
				posx2[1]=x;

				// -
				bitmap->guiFillRect(x+hx/4,pointy,x+hx-hx/4,pointy+hz,fgcol);
				x+=addx;

				int tickindex;

				switch(pos.measureformat)
				{
				case PM_1111:
				case PM_1110:
				case PM_1p1p1p1:
				case PM_1p1p1p0:
					{
						tickindex=3;

						posx[2]=x;

						// Zoom
						if(pos.pos[2]>9)
						{
							LONGLONG h=pos.pos[2];
							h/=10;

							number.nr[h]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==2?mouseovercolour:fgcol);
							x+=addx;

							pos.pos[2]-=10*h;
						}

						number.nr[(int)pos.pos[2]]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==2?mouseovercolour:fgcol);
						x+=(int)addminus;

						posx2[2]=x;

						// -
						bitmap->guiFillRect(x+hx/4,pointy,x+hx-hx/4,pointy+hz,fgcol);
						x+=addx;
						numbers=4;
					}
					break;

				default:
					tickindex=2;
					numbers=3;
					break;
				}

				// Ticks
				posx[tickindex]=x;

				h=pos.pos[tickindex]/100;
				number.nr[h]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==tickindex?mouseovercolour:fgcol);
				pos.pos[tickindex]-=h*100;
				x+=addx;

				h=pos.pos[tickindex]/10;
				number.nr[h]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==tickindex?mouseovercolour:fgcol);
				pos.pos[tickindex]-=h*10;
				x+=addx;

				number.nr[(int)pos.pos[tickindex]]->Draw(bitmap,x,1,x+hx,hhelp,mouseoverindex==tickindex?mouseovercolour:fgcol);
				x+=addx;

				posx2[tickindex]=x;
			}
			else // SMPTE
			{
				int mode=Seq_Pos::POSMODE_TIME;

				// Frames
				switch(windowtimeformat)
				{
				case WINDOWDISPLAY_SECONDS:
					fgcol=COLOUR_TEXT;
					mode=Seq_Pos::POSMODE_TIME;
					numbers=3;
					break;

				case WINDOWDISPLAY_SMPTE:
					fgcol=COLOUR_SMPTE;
					mode=WindowSong()->project->standardsmpte;
					numbers=5;
					break;
				}

				Seq_Pos pos(mode);

				if(windowtimeformat==WINDOWDISPLAY_SMPTE)
				{
					pos.song=WindowSong();
					pos.offset=&WindowSong()->smpteoffset;
				}

				WindowSong()->timetrack.ConvertTicksToPos(activesongposition,&pos);
			
				pos.Clone(&realtimepos);

				if(pos.pos[0]>=100)
					return;

				int x=0;
				int addx,hx=width;

				if(windowtimeformat==WINDOWDISPLAY_SECONDS)
					hx/=9;
				else
					hx/=14;

				int hhelp=height,midy=hhelp/2,hz=hhelp/6;

				double dhx=hx,addminus;

				dhx*=0.9;

				addx=hx;
				hx=(int)dhx;

				addminus=addx;
				addminus*=1.15;

				int colour=COLOUR_SMPTELIGHT;
				int mouseovercolour=COLOUR_GREEN;

				// std
				int dy=maingui->GetFontSizeY()-1;

				bitmap->SetTextColour(COLOUR_TEXT);
				bitmap->guiDrawText(x,maingui->GetFontSizeY(),x+2*hx,Cxs[CXS_HOUR]);

				if(pos.minus==true)
				{
					int cx=x+2,cx2=x+hx-2;
					int cy=bitmap->GetY2()-maingui->GetFontSizeY();

					cy/=2;

					cy+=maingui->GetFontSizeY();
					cy-=hz/2;

					int cy2=cy+hz;

					bitmap->guiFillRect(cx,cy,cx2,cy2,fgcol);
				}

				x+=addx; // minus

				LONGLONG h=pos.pos[0]/10;
				if(h>0)
					colour=fgcol;

				posx[0]=x;
				number.nr[h]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==0?mouseovercolour:colour);

				pos.pos[0]-=h*10;
				x+=addx;

				if(pos.pos[0]>0)
					colour=fgcol;

				number.nr[(int)pos.pos[0]]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==0?mouseovercolour:colour);
				x+=addx;
				posx2[0]=x;

				// :
				bitmap->guiFillRect(x,midy-2*hz,x+hx/3,midy-hz,fgcol);
				bitmap->guiFillRect(x,midy+hz,x+hx/3,midy+2*hz,fgcol);
				x+=addx;

				// Min
				h=pos.pos[1]/10;

				posx[1]=x;
				bitmap->guiDrawText(x,maingui->GetFontSizeY(),x+2*hx,Cxs[CXS_MINUTE]);

				if(h>0)colour=fgcol;

				number.nr[h]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==1?mouseovercolour:fgcol);

				pos.pos[1]-=h*10;
				x+=addx;

				if(pos.pos[1]>0)
					colour=fgcol;

				number.nr[(int)pos.pos[1]]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==1?mouseovercolour:fgcol);
				x+=addx;
				posx2[1]=x;

				// :
				bitmap->guiFillRect(x,midy-2*hz,x+hx/3,midy-hz,fgcol);
				bitmap->guiFillRect(x,midy+hz,x+hx/3,midy+2*hz,fgcol);
				x+=addx;

				// Sek
				h=pos.pos[2]/10;

				posx[2]=x;
				bitmap->guiDrawText(x,maingui->GetFontSizeY(),x+2*hx,Cxs[CXS_SECOND]);

				number.nr[h]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==2?mouseovercolour:fgcol);

				pos.pos[2]-=h*10;
				x+=addx;

				number.nr[(int)pos.pos[2]]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==2?mouseovercolour:fgcol);

				posx2[2]=x+hx;

				if(windowtimeformat!=WINDOWDISPLAY_SECONDS)
				{
					x+=(int)addminus;

					// :
					bitmap->guiFillRect(x,midy-2*hz,x+hx/3,midy-hz,fgcol);
					bitmap->guiFillRect(x,midy+hz,x+hx/3,midy+2*hz,fgcol);
					x+=addx;

					bitmap->guiDrawText(x,maingui->GetFontSizeY(),x+2*hx,"Frame");

					// Frames
					posx[3]=x;
					h=pos.pos[3]/10;

					number.nr[h]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==3?mouseovercolour:fgcol);

					pos.pos[3]-=h*10;
					x+=addx;

					number.nr[(int)pos.pos[3]]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==3?mouseovercolour:fgcol);
					x+=addx;
					posx2[3]=x;
					
					// QFrames
					// ;
					bitmap->guiFillRect(x,midy-2*hz,x+hx/3,midy-hz,fgcol);
					bitmap->guiFillRect(x,midy+hz,x+hx/3,time->GetY2(),fgcol);
					x+=addx;

					h=pos.pos[4];

					posx[4]=x;
					number.nr[h]->Draw(bitmap,x,dy,x+hx,hhelp,mouseoverindex==4?mouseovercolour:fgcol);
					posx2[4]=x+hx;
				}
			}
		}
	}
	else
		bitmap->guiFillRect(COLOUR_BLACK);
}

void Edit_BigTime::ShowSongWindowName()
{
	if(WindowSong())
	{
		size_t i=strlen(WindowSong()->GetName());
		char *s=new char[i+64];

		if(s)
		{
			switch(windowtimeformat)
			{
			case WINDOWDISPLAY_SECONDS:
				{
				char *h=mainvar->GenerateString(Cxs[CXS_HOUR],":",Cxs[CXS_MINUTE],":",Cxs[CXS_SECOND]);

					if(h)
					{
						strcpy(s,h);
			delete h;
					}
				}
				break;

			case WINDOWDISPLAY_SMPTE:
				{
					strcpy(s,smpte_modestring[WindowSong()->project->standardsmpte]);
				}				
				break;

			default:
				strcpy(s,Cxs[CXS_MEASURE]);
				break;
			}

			mainvar->AddString(s," ");
			mainvar->AddString(s,WindowSong()->GetName());

			guiSetWindowText(s);
			delete s;
		}
	}
	else
	{
		guiSetWindowText(Cxs[CXS_NOSONG]);
	}
}

void Time_Callback(guiGadget_CW *g,int status)
{
	Edit_BigTime *bt=(Edit_BigTime *)g->from;

	switch(status)
	{
	case DB_CREATE:
		bt->time=g;
		break;

	case DB_PAINT:
		{
			bt->ShowTime();
		}
		break;

	case DB_MOUSEMOVE:
		//bt->CheckMouseOver();
		break;

	case DB_LEFTMOUSEDOWN:
		{
		bt->MouseClickInTime(true);
		}
		break;

	case DB_LEFTMOUSEUP:
		bt->MouseReleaseInTime(true);
		break;

	case DB_KILLFOCUS:
		break;

	case DB_KEYDOWN:
	
		break;

	case DB_KEYUP:
	
		break;
	}
}

void Edit_BigTime::Zoom(int z)
{
	editmouseindex=-1; // Reset 

	if(z!=zoombig)
	{
	mainsettings->defaultbigtimezoom=zoombig=z;
	SetWindowSize(GetZoomWidth(),GetZoomHeight());
	ShowMenu();
	}
}

void Edit_BigTime::RemoveSong(Seq_Song *)
{
	song=0; // Avoid Close Window...
	ShowSongWindowName();
	DrawDBBlit(time);
}

bool Edit_BigTime::DeltaY()
{
	if(!WindowSong())
		return false;

	switch(windowtimeformat)
	{
	case WINDOWDISPLAY_SECONDS:
	case WINDOWDISPLAY_SMPTE:
		{
			Seq_Pos spos(windowtimeformat==WINDOWDISPLAY_SMPTE?WindowSong()->project->standardsmpte:Seq_Pos::POSMODE_TIME);

			WindowSong()->timetrack.ConvertTicksToPos(WindowSong()->GetSongPosition(),&spos);

			switch(editmouseindex)
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

			OSTART vtime=WindowSong()->timetrack.ConvertPosToTicks(&spos);
			WindowSong()->SetSongPosition(vtime,true);
		}
		break;

	case WINDOWDISPLAY_MEASURE:
		{
			Seq_Pos spos(Seq_Pos::POSMODE_NORMAL);

				WindowSong()->timetrack.ConvertTicksToPos(WindowSong()->GetSongPosition(),&spos);

			if(spos.index==3 && editmouseindex==2)
				editmouseindex=3; // 1.1.5, no zoom

			switch(editmouseindex)
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

			OSTART vtime=WindowSong()->timetrack.ConvertPosToTicks(&spos);
				WindowSong()->SetSongPosition(vtime,true);
		}
		break;

		/*
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
		*/
	}

	return true;
}

void Edit_BigTime::MouseWheel(int delta,guiGadget *db)
{
	if(mouseoverindex>=0)
	{
		deltay=-delta;
		editmouseindex=mouseoverindex;
		DeltaY();
		editmouseindex=-1; // Reset
	}
}

void Edit_BigTime::MouseClickInTime(bool leftmouse)
{
	if(mouseoverindex!=-1)
	{
		editmouseindex=mouseoverindex;
		editmousestarty=GetScreenMouseY();
	}
}

void Edit_BigTime::MouseReleaseInTime(bool leftmouse)
{
	editmouseindex=-1; // Reset
}

void Edit_BigTime::Init()
{
	song=mainvar->GetActiveSong();

	ShowSongWindowName();
	ShowMenu();

	glist.SelectForm(0,0);
	time=glist.AddChildWindow(-1,-1,-1,-2,MODE_LEFT|MODE_RIGHT|MODE_BOTTOM,0,&Time_Callback,this);
	glist.Return();
}

void Edit_BigTime::RefreshSMPTE()
{
	switch(windowtimeformat)
	{
	case WINDOWDISPLAY_SMPTE:
		ShowSongWindowName();
		DrawDBBlit(time);
		break;
	}
}

int Edit_BigTime::GetZoomWidth()
{
	switch(zoombig)
	{
	case 0:
		return 20*maingui->GetFontSizeY();
		break;

		case 1:
		return 30*maingui->GetFontSizeY();
		break;

		case 2:
		return 40*maingui->GetFontSizeY();
		break;

		case 3:
		return 60*maingui->GetFontSizeY();
		break;

		case 4:
		return 80*maingui->GetFontSizeY();
		break;
	}

	return 100;
}

void Edit_BigTime::DeActivated()
{
	if(mouseoverindex!=1)
			{
				mouseoverindex=-1;
				DrawDBBlit(time);
			}
}

void Edit_BigTime::CheckMouseOver()
{
	if(editmouseindex!=-1) // Edit Mode
		return;

	if(!WindowSong())
		return;

	if(IsFocusWindow()==true)
	{
		int x=time->GetMouseX();
		int y=time->GetMouseY();

		TRACE ("Y %d\n",y);

		if(y>0 && y<time->GetY2())
		{
			for(int i=0;i<numbers;i++)
			{
				if(posx[i]<=x && posx2[i]>=x)
				{
					if(mouseoverindex!=i)
					{
						mouseoverindex=i;
						DrawDBBlit(time);
					}

					return;
				}
			}
		}
	}

	if(mouseoverindex!=-1)
	{
		mouseoverindex=-1;
		DrawDBBlit(time);
	}
}

int Edit_BigTime::GetZoomHeight()
{
	double h=GetZoomWidth();

	h*=0.34;

	return (int)h;
}

void Edit_BigTime::RefreshMeasure()
{
	if(windowtimeformat==WINDOWDISPLAY_MEASURE)
	DrawDBBlit(time);
}

void Edit_BigTime::RefreshRealtime_Slow()
{
	CheckMouseOver();
}

void Edit_BigTime::RefreshRealtime()
{
	if(mainvar->GetActiveSong()!=WindowSong())
	{
		song=mainvar->GetActiveSong();
		ShowSongWindowName();
		DrawDBBlit(time);
	}

	if(++rt_counter>=3)
	{
		rt_counter=0;

		if(WindowSong())
		{
			if((WindowSong()->status&Seq_Song::STATUS_WAITPREMETRO) ||
				(WindowSong()->GetSongPosition()!=activesongposition)
				)
			{
				DrawDBBlit(time);
			}
			else
			{
				// SMPTE Tempo Changes etc...
				int mode=Seq_Pos::POSMODE_TIME;

				// Frames
				switch(windowtimeformat)
				{
				case WINDOWDISPLAY_SECONDS:
					mode=Seq_Pos::POSMODE_TIME;
					break;

				case WINDOWDISPLAY_SMPTE:
					mode=WindowSong()->project->standardsmpte;
					break;
				}

				if(mode!=Seq_Pos::POSMODE_TIME)
				{
					Seq_Pos pos(mode);

					WindowSong()->timetrack.ConvertTicksToPos(activesongposition,&pos);
					//pos.AddOffSet();

					if(pos.Compare(&realtimepos)==false)
						DrawDBBlit(time);
				}
			}
		}
	}

	if(editmouseindex!=-1)
	{
		int my=GetScreenMouseY();
		
		deltay=editmousestarty-my;

		deltay/=6; // 1/6 Speed

		if(deltay)
		{
			DeltaY();
			editmousestarty=my;
		}
	}
}

void Edit_BigTime::ShowMenu()
{
	if(displaymenu)
	{
		// menu_measure=menu_showbeat=menu_showtime=menu_showsmpte=menu_showbw=menu_showwb

		if(menu_measure)menu_measure->menu->Select(menu_measure->index,false);
		if(menu_showtime)menu_showtime->menu->Select(menu_showtime->index,false);
		if(menu_showsmpte)menu_showsmpte->menu->Select(menu_showsmpte->index,false);

		switch(windowtimeformat)
		{
		case WINDOWDISPLAY_SMPTE:
			if(menu_showsmpte)menu_showsmpte->menu->Select(menu_showsmpte->index,true);
			break;

		case WINDOWDISPLAY_SECONDS:
			if(menu_showtime)menu_showtime->menu->Select(menu_showtime->index,true);
			break;

		default:
			if(menu_measure)menu_measure->menu->Select(menu_measure->index,true);
			break;
		}

	//	if(menu_showbeat)menu_showbeat->menu->Select(menu_showbeat->index,showbeats);
	//	if(menu_showbw)menu_showbw->menu->Select(menu_showbw->index,showbw==true?false:true);
	//	if(menu_showwb)menu_showwb->menu->Select(menu_showwb->index,showbw==true?true:false);

		for(int i=0;i<5;i++)
		{
			if(menu_zoom[i])
				menu_zoom[i]->menu->Select(menu_zoom[i]->index,zoombig==i?true:false);
		}

	}
}

Edit_BigTime::Edit_BigTime()
{
	editorid=EDITORTYPE_BIGTIME;

	InitForms(FORM_PLAIN1x1);

	activesongposition=-1;
	precounter=-1;
	rt_counter=0;
	zoombig=mainsettings->defaultbigtimezoom;
	showbeats=false;
	showbw=mainsettings->bigtime_showbw;
	hasownmenu=true;
	mouseoverindex=editmouseindex=-1;
}

void Edit_BigTime::ChangeFormat(int format)
{
	if(windowtimeformat!=format)
	{
		windowtimeformat=format;
		mouseoverindex=-1; // Reset

		DrawDBBlit(time);
		ShowMenu();
		ShowSongWindowName();
	}
}

void Edit_BigTime::CreateWindowMenu()
{
	// Time
	if(menu)
		menu->RemoveMenu();

	displaymenu=0;

	menu_measure=menu_showtime=menu_showsmpte=0;

	if(menu=new guiMenu)
	{
		displaymenu=menu->AddMenu(Cxs[CXS_DISPLAY],0);

		if(displaymenu)
		{
			class menu_dmeasure:public guiMenu
			{
			public:
				menu_dmeasure(Edit_BigTime *ee)
				{
					editor=ee;
				}

				void MenuFunction()
				{	
					editor->ChangeFormat(WINDOWDISPLAY_MEASURE);
				
				} //

				Edit_BigTime *editor;
			};
			menu_measure=displaymenu->AddFMenu(Cxs[CXS_MEASURE],new menu_dmeasure(this));

			/*
			class menu_mshowbeat:public guiMenu
			{
			public:
				menu_mshowbeat(Edit_BigTime *ee)
				{
					editor=ee;
				}

				void MenuFunction()
				{	
					if(editor->showbeats==true)
						editor->showbeats=false;
					else
						editor->showbeats=true;

					editor->ShowTime();
					editor->ShowMenu();
					editor->ShowSongWindowName();
				} //

				Edit_BigTime *editor;
			};
			{
				char *h=mainvar->GenerateString(Cxs[CXS_DISPLAY]," ",Cxs[CXS_MEASURE],"+",Cxs[CXS_BEAT]);
				if(h)
				{
					menu_showbeat=displaymenu->AddFMenu(h,new menu_mshowbeat(this),showbeats);
					delete h;
				}
			}

			displaymenu->AddLine();
*/

			class menu_dtime:public guiMenu
			{
			public:
				menu_dtime(Edit_BigTime *ee)
				{
					editor=ee;
				}

				void MenuFunction()
				{	
						editor->ChangeFormat(WINDOWDISPLAY_SECONDS);
				} //

				Edit_BigTime *editor;
			};

			char *h=mainvar->GenerateString(Cxs[CXS_HOUR],":",Cxs[CXS_MINUTE],":",Cxs[CXS_SECOND]);

			if(h)
			{
			menu_showtime=displaymenu->AddFMenu(h,new menu_dtime(this));
			delete h;
			}

			class menu_dsmpte:public guiMenu
			{
			public:
				menu_dsmpte(Edit_BigTime *ee)
				{
					editor=ee;
				}

				void MenuFunction()
				{	
					editor->ChangeFormat(WINDOWDISPLAY_SMPTE);
				} 

				Edit_BigTime *editor;
			};
			menu_showsmpte=displaymenu->AddFMenu("Frames",new menu_dsmpte(this));

			displaymenu->AddLine();

			class m_zoom:public guiMenu
			{
			public:
				m_zoom(Edit_BigTime *ee,int z)
				{
					editor=ee;
					zoom=z;
				}

				void MenuFunction()
				{	
					editor->Zoom(zoom);
				} 

				Edit_BigTime *editor;
				int zoom;
			};


			menu_zoom[0]=displaymenu->AddFMenu("Zoom 1",new m_zoom(this,0));
			menu_zoom[1]=displaymenu->AddFMenu("Zoom 2",new m_zoom(this,1));
			menu_zoom[2]=displaymenu->AddFMenu("Zoom 3",new m_zoom(this,2));
			menu_zoom[3]=displaymenu->AddFMenu("Zoom 4",new m_zoom(this,3));
			menu_zoom[4]=displaymenu->AddFMenu("Zoom 5",new m_zoom(this,4));

			/*
			displaymenu->AddLine();

			class menu_showcol:public guiMenu
			{
			public:
				menu_showcol(Edit_BigTime *ee,bool bw)
				{
					editor=ee;
					blackwhite=bw;
				}

				void MenuFunction()
				{	
					mainsettings->bigtime_showbw=editor->showbw=blackwhite;

					editor->ShowTime();
					editor->ShowMenu();
					editor->ShowSongWindowName();
				} //

				Edit_BigTime *editor;
				bool blackwhite;
			};

			menu_showbw=displaymenu->AddFMenu("Colour Grey/White",new menu_showcol(this,false),showbw==false?true:false);
			menu_showwb=displaymenu->AddFMenu("Colour White/Black",new menu_showcol(this,true),showbw==true?true:false);
			*/
		}
	}
}
