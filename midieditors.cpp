#include "songmain.h"
#include "editor.h"
#include "editor_event.h"

#include "editsettings.h"

#include "object_song.h"
#include "audiohardware.h"
#include "camxgadgets.h"
#include "audiomixeditor.h"
#include "sampleeditor.h"
#include "audiomanager.h"
#include "arrangeeditor.h"
#include "transporteditor.h"
#include "waveeditor.h"
#include "pianoeditor.h"
#include "groove.h"
#include "editdata.h"
#include "MIDIhardware.h"
#include "quantizeeditor.h"
#include "audiomaster.h"
#include "editMIDIfilter.h"
#include "vstguiwindow.h"
#include "tempomapeditor.h"
#include "editor_help.h"
#include "vstplugins.h"
#include "stepeditor.h"
#include "keyboard.h"
#include "edit_audiointern.h"
#include "editor_text.h"
#include "editor_marker.h"
#include "wavemap.h"
#include "groupeditor.h"
#include "edit_processor.h"
#include "player.h"
#include "bigtime.h"
#include "score.h"
#include "rmg.h"
#include "editwin32audio.h"
#include "library.h"
#include "object_project.h"

#ifdef WIN32
#include<winuser.h>
#endif

#include <stdio.h>
#include <stdlib.h>

void guiOList::RemoveOsFromTo(int from,int to)
{
	guiObject *ol=LastObject();

	// Last->First, cause of static frame objects
	while(ol)
	{
		if(ol->id>=from && ol->id<=to)
			ol=ol->RemoveGuiObject(0);
		else
			ol=ol->PrevObject();
	}
}

void guiOList::RemoveOs(int rid)
{
	guiObject *ol=LastObject();

	// Last->First, cause of static frame objects
	while(ol)
		ol=ol->RemoveGuiObject(rid);
}

guiObject *guiObject::RemoveGuiObject(int rid)
{
	guiObject *n=PrevObject();

	if(id==rid || rid==0)
	{	
		guiOList *OList=(guiOList *)GetList();
		DeInit();
		if(deleteable==true)OList->objects.RemoveO(this);
		else OList->objects.CutQObject(this); // part of class or so
	}

	return n;
}

void guiObject::Blt()
{
	if(parent)
	{
		parent->Blt(this);
		return;
	}

	if(gadget)
	{
		gadget->Blt(this);
		return;
	}
}

guiObject *guiOList::CheckObjectClicked(int posx,int posy)
{
	return 0;

	/*
	guiObject *f=FirstObject(),*found=0;

	while(f)
	{		
	int x=f->x,y=f->y,x2=f->x2,y2=f->y2;

	if(f->child)
	{
	if(f->child->formchild)
	{
	x+=f->child->x+f->child->formchild->x;
	y+=f->child->y+f->child->formchild->y;
	x2+=f->child->x+f->child->formchild->x;
	y2+=f->child->y+f->child->formchild->y;
	}
	else
	{
	x+=f->child->x;
	y+=f->child->y;
	x2+=f->child->x;
	y2+=f->child->y;
	}
	}

	if(posx>=x && posx<=x2 && posy>=y && posy<=y2)
	found=f;

	f=f->NextObject();
	}

	return found;
	*/
}

bool guiObject::CheckObjectinRange(int cx,int cy,int cx2,int cy2)
{
	// x<cx2
	// y<cy2
	// x2>cx
	// y2>cy

	if(cx>cx2)
	{
		int h=cx;
		cx=cx2;
		cx2=h;
	}

	if(cy>cy2)
	{
		int h=cy;
		cy=cy2;
		cy2=h;
	}

	if(x<cx2 && y<cy2 && x2>cx && y2>cy)return true;

	return false;
}

void guiOList::AddGUIObject(guiGadget *ga,guiGadget_CW *g,guiObject *o)
{
	if(ga)
	{
		o->parent=g;
		o->ondisplay=true;

		o->x=ga->x;
		o->x2=ga->x2;
		o->y=ga->y;
		o->y2=ga->y2;

		objects.AddEndO(o);
	}
}

void guiOList::AddGUIObject(guiObject *o,guiGadget_CW *db)
{
	if(o)
	{
		o->parent=db;
		o->ondisplay=true;
		objects.AddEndO(o);
	}
}

void guiOList::AddGUIObject(int x,int y,int x2,int y2,guiGadget_CW *g,guiObject *o)
{
	if(o)
	{
		o->parent=g;

		if(x<=x2 && y<=y2)
			o->ondisplay=true;

		o->x=x;
		o->x2=x2;
		o->y=y;
		o->y2=y2;

		objects.AddEndO(o);
	}
}

void guiOList::AddTABGUIObject(int x,int y,int x2,int y2,guiGadget_Tab *g,guiObject *o)
{
	if(o)
	{
		o->parent=g;

		if(x<=x2 && y<=y2)
			o->ondisplay=true;

		o->x=x;
		o->x2=x2;
		o->y=y;
		o->y2=y2;

		if(guiObject_Pref *pf=new guiObject_Pref(o))
			g->preflist.AddEndO(pf);

		objects.AddEndO(o);
	}
}

OSTART guiTimeLine::ConvertXPosToTime(int xpos)
{		
	if(xpos>=x && xpos<=x2)return win->startposition+((xpos-x)*win->zoom->ticksperpixel);
	return -1;
}

guiTimeLinePos* guiTimeLine::FindPositionX(int posx)
{
	if(posx>=x && posx<=x2)
	{
		guiTimeLinePos *p=FirstPosition();

		while(p)
		{
			guiTimeLinePos *n=p->NextPosition();
			if((!n) || (p->x<=posx && n->x>=posx))return p;
			p=p->NextPosition();
		}
	}

	return 0;
}

void guiTimeLine::DrawPositionRaster(guiBitmap *bitmap)
{
	guiTimeLinePos *pos=FirstPosition();

	//	bitmap->SetAPen(COLOUR_GREY_LIGHT);

	while(pos)
	{
		//if(pos->showtext==true)
		switch(pos->type)
		{
		case HEAD_POSITION_1xxx:
			bitmap->guiDrawLineX(pos->x,pos->showtext==true?COLOUR_BLACK:COLOUR_BLACK_LIGHT);
			break;

		case HEAD_POSITION_x1xx:
			bitmap->guiDrawLineX(pos->x,COLOUR_GREY_DARK);
			break;

		case HEAD_POSITION_xx1x:
			bitmap->guiDrawLineX(pos->x,COLOUR_11x1);
			break;
		}

		pos=pos->NextPosition();
	}
}


void guiTimeLine::DrawPositionRaster(guiBitmap *bitmap,int y,int y2)
{
	guiTimeLinePos *pos=FirstPosition();

	//	bitmap->SetAPen(COLOUR_GREY_LIGHT);

	while(pos)
	{
		//if(pos->showtext==true)
		switch(pos->type)
		{
		case HEAD_POSITION_1xxx:
			bitmap->guiDrawLineX(pos->x,y,y2,pos->showtext==true?COLOUR_BLACK:COLOUR_GREY);
			break;

		case HEAD_POSITION_x1xx:
			bitmap->guiDrawLineX(pos->x,y,y2,COLOUR_GREY_DARK);
			break;
		case HEAD_POSITION_xx1x:
			bitmap->guiDrawLineX(pos->x,y,y2,COLOUR_11x1);
			break;
		}

		pos=pos->NextPosition();
	}
}

void guiTimeLine::BufferOldMidPosition()
{
	if(oldmidposition==-1)
	{
		if(headertimex>=0)
		{
			midmode=MID_SONGPOSITION;
			oldmidposition=win->WindowSong()->GetSongPosition();
			oldmidposx=headertimex;
		}
		else
		{
			midmode=MID_MIDPOSITION;
			oldmidposition=ConvertXPosToTime((oldmidposx=(x2-x)/2));
		}
	}
}

bool guiTimeLine::CheckMousePositionAndSongPosition()
{
	if(headertime!=win->WindowSong()->GetSongPosition() || win->GetMousePosition()!=mousetime)
	{
		int cheadertimex=ConvertTimeToX(headertime=win->WindowSong()->GetSongPosition());

		if(cheadertimex!=headertimex)
			return false;

		int cmousetimex=ConvertTimeToX(mousetime=win->GetMousePosition());

		if(cmousetimex!=mousetimex)
			return false;
	}

	return true;
}

void guiTimeLine::RemoveAllPositions()
{
	guiTimeLinePos *p=FirstPosition();
	while(p)p=RemovePosition(p);
}

guiTimeLine::guiTimeLine()
{
	pos.mode=Seq_Pos::POSMODE_COMPRESS;
	dbgadget=0;
	sampleposition=0;
	mousetime=-1;
	movesongposition=false;
	oldmidposition=-1;
	headertimex=mousetimex=-1;
	headersongstoppositionx=-1;

	showzoom=-1; // Default off
}

void guiTimeLine::DeInit()
{
	RemoveAllPositions();
	if(sampleposition)
	{
		delete sampleposition;
		sampleposition=0;
	}
}

guiTimeLinePos *guiTimeLine::AddPosition(OSTART time,int x,int type,bool showtext,OSTART measure,OSTART beat,OSTART zoom)
{
	if(guiTimeLinePos *p=new guiTimeLinePos)
	{
		lpos.AddEndS(p,time);

		p->x=x;
		p->type=type;
		p->showtext=showtext;
		p->measure=measure;
		p->beat=beat;
		p->zoom=zoom;

		return p;
	}

	return 0;
}

int guiTimeLine::ConvertTimeToX(OSTART time,int cx2) // -1,0 or x co
{
	if(time<win->startposition)return -1;
	if(time>win->endposition)return 0;

	int xh=x; // Start Header
	double h=(double)(time-win->startposition);

	h/=zoom->dticksperpixel;
	xh+=(int)floor(h+0.5);

	if(xh>cx2)return cx2;
	return xh;	
}

int guiTimeLine::ConvertTimeToX(OSTART time) // -1,0 or x co
{
	if(time<win->startposition)return -1;
	if(time>win->endposition)return -1;

	double h=(double)(time-win->startposition);	
	h/=zoom->dticksperpixel;

	return x+(int)floor(h+0.5);	
}

guiTimeLinePos *guiTimeLine::RemovePosition(guiTimeLinePos *p)
{
	return (guiTimeLinePos *)lpos.RemoveO(p);
}

bool guiTimeLine::CheckIfInHeader(OSTART pos)
{
	if(pos>=win->startposition && pos<=win->endposition)return true;
	return false;
}

bool guiTimeLine::CheckIfCycle(int cx,int cy)
{
	//if(cycleonheader==true && cx>=cyclex && cx<=cyclex2 && cy>=cycley && cy<=cycley2)return true;
	return false;
}

bool guiTimeLine::CheckMousePosition(int cx)
{
	//if(cx>=x && cx<=x2)return true;
	return false;
}

bool guiTimeLine::CheckMousePosition(int cx,int cy)
{
	//if(cx>=x && cx<=x2 && cy>=y && cy<=y2)return true;
	return false;
}

void guiTimeLine::ShowCycleAndPositions()
{
	guiBitmap *bitmap=&dbgadget->spritebitmap;

	bitmap->guiFillRect(COLOUR_BLACK);

	if(win->songmode==false)
		return;

	// Draw Header Cycle Range
	if(win->WindowSong()->playbacksettings.cyclestart<win->endposition && win->WindowSong()->playbacksettings.cycleend>win->startposition)
	{
		cycleonheader=true;
		cycleon=win->WindowSong()->playbacksettings.cycleplayback;

		//	cycley2=splity-1;

		int col=cycleon==true?COLOUR_BLUE:COLOUR_GREY_DARK;

		if(win->WindowSong()->playbacksettings.cyclestart<win->startposition)
		{
			cyclex=0;
			cyclepos_x=-1; // not on window
		}
		else
		{
			cyclepos_x=cyclex=ConvertTimeToX(win->WindowSong()->playbacksettings.cyclestart);
			bitmap->guiFillRect(cyclepos_x,cycley2,cyclepos_x+7,bitmap->GetY2(),col);
			//bitmap->guiDrawLine(cyclepos_x,y,cyclepos_x,bitmap->height,col);
		}

		if(win->WindowSong()->playbacksettings.cycleend>win->endposition)
		{
			cyclepos_x2=-1; // not on window
			cyclex2=x2;
		}
		else
		{
			cyclepos_x2=cyclex2=ConvertTimeToX(win->WindowSong()->playbacksettings.cycleend);
			bitmap->guiFillRect(cyclepos_x2-6,cycley2,cyclepos_x2,bitmap->GetY2(),col);
			//bitmap->guiDrawLine(cyclepos_x2,y,cyclepos_x2,bitmap->height,col);
		}

		bitmap->guiFillRect(cyclex,1,cyclex2,cycley2,col);
		bitmap->guiDrawLineY(0,cyclex,cyclex2,cycleon==true?COLOUR_YELLOW:COLOUR_GREY);

		//	bm->guiInvert(cyclex,cycley,cyclex2,cycley2);

		/*
		// < >
		if(cyclex2>cyclex+12)
		{
		if(arrowleft==true)
		{
		int midy=cycley+((cycley2-cycley)/2),arrowx2=cyclex+12;

		bitmap->guiDrawLine(cyclex+1,midy,cyclex+12,cycley+1,COLOUR_YELLOW_LIGHT);
		bitmap->guiDrawLine(cyclex+1,midy,cyclex+12,cycley2-1,COLOUR_YELLOW_LIGHT);
		bitmap->guiDrawLine(cyclex+12,cycley+1,cyclex+12,cycley2-1,COLOUR_YELLOW_LIGHT);
		}

		if(arrowright==true)
		{
		int midy=cycley+((cycley2-cycley)/2),arrowx=cyclex2-12;

		bitmap->guiDrawLine(arrowx,cycley+1,cyclex2-1,midy,COLOUR_YELLOW_LIGHT);
		bitmap->guiDrawLine(arrowx,cycley2-1,cyclex2-1,midy,COLOUR_YELLOW_LIGHT);
		bitmap->guiDrawLine(arrowx,cycley+1,arrowx,cycley2-1,COLOUR_YELLOW_LIGHT);
		}
		}
		*/
	}
	else
		cycleonheader=false;

	movesongposition=win->mousemode==EM_MOVESONGPOSITION?true:false;

	headertimex=ConvertTimeToX(headertime=win->WindowSong()->GetSongPosition());

	if(headertimex>=0)
		bitmap->guiFillRect(headertimex,0,headertimex2=headertimex+maingui->GetFontSizeY()/3,bitmap->GetY2(),movesongposition==false?COLOUR_TIMEPOSITION:COLOUR_TIMEPOSITIONMOUSEMOVE);

	mousetimex=ConvertTimeToX(mousetime=win->GetMousePosition());

	if(mousetimex>=0)
		bitmap->guiDrawLineX(mousetimex,COLOUR_YELLOW);

	// Song Stop Marker
	if(Seq_Marker *mk=win->WindowSong()->textandmarker.FindMarkerID(Seq_Marker::MARKERFUNC_STOPPLAYBACK))
	{
		headersongstoppositionx=ConvertTimeToX(mk->GetMarkerStart());
		headersongstoppositionx2=headersongstoppositionx+maingui->GetFontSizeY()/2;

	}
	else
	{
		headersongstoppositionx=-1;
	}

}


void guiTimeLine::RecalcSamplePositions()
{
	if(sampleposition) // Init TempoSamples
	{
		if(win->songmode==true)
		{
			Seq_Time *time=&win->WindowSong()->timetrack;
			OSTART tx=win->startposition;

			// Init
			for(int i=0;i<sampleposition_size+4;i++) // +4
			{
				sampleposition[i]=time->ConvertTicksToTempoSamples(tx);
				tx+=win->zoom->ticksperpixel;
			}
		}
		else
		{
			LONGLONG sstart=win->samplestartposition;

			// Init
			for(int i=0;i<sampleposition_size+4;i++) // +4
			{
				sampleposition[i]=sstart;
				sstart+=win->zoom->samples;
			}
		}

	}
}

void guiTimeLine::Draw()
{
	if(!win)return;

	RemoveAllPositions(); // clear old positions

	if(sampleposition && sampleposition_size!=x2-x)
	{
		delete sampleposition;
		sampleposition=0;
	}

	if(x2<=x)
	{
		win->endposition=win->startposition;
		win->sampleendposition=win->samplestartposition;
		return;
	}

	if(!sampleposition)
	{
		sampleposition_size=x2-x;
		sampleposition=new LONGLONG[sampleposition_size+4]; // Add 4
	}

	// calc end
	if(win->songmode==true)
	{
		// Mid ? Zoom <>
		if(oldmidposition>=0)
		{
			OSTART lpos=0;

			switch(midmode)
			{
			case MID_SONGPOSITION:
				{
					OSTART spos=win->WindowSong()->GetSongPosition();
					lpos=spos-oldmidposx*win->zoom->ticksperpixel;
				}
				break;

			case MID_MIDPOSITION:
				{
					int w=(x2-x)/2;
					lpos=oldmidposition-w*win->zoom->ticksperpixel;
				}
				break;
			}

			OSTART qt;

			if(lpos<0)qt=0;
			else
			{
				qt=lpos/win->zoom->ticksperpixel;
				qt*=win->zoom->ticksperpixel;
			}

			win->startposition=qt;
			win->RefreshTimeSlider();

			//oldmidposition=-1;
		}

		// Song Mode
		win->endposition=win->startposition+((x2-x)*win->zoom->ticksperpixel);
	}
	else
	{
		// Sample Mode
		samplesperpixel=mainaudio->ConvertInternToExternSampleRate(win->zoom->dticksperpixel);
		win->sampleendposition=win->samplestartposition+((double)(x2-x)*samplesperpixel);
	}

	RecalcSamplePositions();

	if(guiBitmap *bm=bitmap)
	{
		int ty2=y+maingui->GetButtonSizeY();

		cycley2=ty2-maingui->buttonsizeaddy;

		bm->guiFillRect(x,y,x2,cycley2,COLOUR_BLACK_LIGHT);
		bm->guiFillRect(x,cycley2+1,x2,y2,COLOUR_BLACK);

		UBYTE r,g,b;
		maingui->colourtable.GetRGB(COLOUR_WHITE,&r,&g,&b);
		bm->SetTextColour(r,g,b);

		double getminitempo=win->WindowSong()->timetrack.GetLowestTempo();
		double getmaxitempo=win->WindowSong()->timetrack.GetHighestTempo();

		if(win->songmode==true)
			switch(format)
		{
			case WINDOWDISPLAY_MEASURE: 
			case WINDOWDISPLAY_SMPTE: //frames
			case WINDOWDISPLAY_SECONDS: // min - ms
			case WINDOWDISPLAY_SAMPLES:
				{
					OSTART time;
					double zoomsec;
					LONGLONG samplepos,samples;

					// Quantize -> Next
					switch(format)
					{
					case WINDOWDISPLAY_SAMPLES:
						{
							samples=win->zoom->samples;

							if(getminitempo<=120.0) // low tempo pixel correction
							{
								double h=180.0/getminitempo;
								samples*=(LONGLONG)h;
							}
							else
								if(getmaxitempo>=180.0) // high tempo pixel correction
								{
									double h=getmaxitempo/120.0;
									samples/=(LONGLONG)h;
								}

								samples/=100;

								if(samples==0)
									samples=1;

								samples*=100;

								samplepos=win->WindowSong()->timetrack.ConvertSamplesToNextSamples(win->WindowSong()->timetrack.ConvertTicksToTempoSamples(win->startposition),samples);
								time=win->WindowSong()->timetrack.ConvertSamplesToOSTART(samplepos);
						}
						break;

					case WINDOWDISPLAY_SMPTE:
					case WINDOWDISPLAY_SECONDS: // sec
						{
							zoomsec=win->zoom->sec;

							if(getminitempo<=120) // low tempo pixel correction
							{
								double h=120/getminitempo,h2=(int)h;
								zoomsec*=h2;
							}
							else
								if(getmaxitempo>=180) // high tempo pixel correction
								{
									double h=getmaxitempo/90,h2=(int)h;
									zoomsec/=h2;
								}

								if(format==WINDOWDISPLAY_SMPTE)
								{
									pos.mode=win->WindowSong()->project->standardsmpte;
								}
								else
									pos.mode=Seq_Pos::POSMODE_TIME;

								time=win->WindowSong()->timetrack.ConvertTicksToNextMs(win->startposition,zoomsec);
						}
						break;

					case WINDOWDISPLAY_MEASURE:
						{
							pos.mode=Seq_Pos::POSMODE_NORMAL;

							if(win->zoom->withzoom==true && showzoom!=-1)
								time=win->WindowSong()->timetrack.ConvertTicksToNextZoomTicks(win->startposition,showzoom);
							else
								time=win->zoom->withzoom==false?win->WindowSong()->timetrack.ConvertTicksToNextMeasureTicks(win->startposition):win->WindowSong()->timetrack.ConvertTicksToNextBeatTicks(win->startposition);
						}
						break;
					}

					LONGLONG lastbeat;

					switch(format)
					{
					case WINDOWDISPLAY_MEASURE:
						{
							win->WindowSong()->timetrack.ConvertTicksToPos(time,&pos,win->zoom->withzoom==true && showzoom!=-1?showzoom:0);
							lastbeat=pos.pos[1];
						}
						break;
					}

					while(time<=win->endposition)
					{
						switch(format)
						{
						case WINDOWDISPLAY_SAMPLES:
							{
								int px=ConvertTimeToX(time);

								bm->guiDrawLineX(px,y2/2,y2,COLOUR_WHITE); // With Number
								AddPosition(samplepos,px,HEAD_POSITION_1xxx,true,0);

								samplepos+=samples;
								time=win->WindowSong()->timetrack.ConvertSamplesToOSTART(samplepos);
							}
							break;

						case WINDOWDISPLAY_SECONDS:
						case WINDOWDISPLAY_SMPTE:
							{
								int px=ConvertTimeToX(time);

								bm->guiDrawLineX(px,y2/2,y2,COLOUR_WHITE); // With Number
								AddPosition(time,px,HEAD_POSITION_1xxx,true,0);

								time=win->WindowSong()->timetrack.ConvertTicksToNextMs(time,zoomsec,true);
							}
							break;

						case WINDOWDISPLAY_MEASURE:
							{
								bool checkzoom=win->zoom->withzoom;

								win->WindowSong()->timetrack.ConvertTicksToPos(time,&pos,win->zoom->withzoom==true && showzoom!=-1?showzoom:0);

								bool showbeat=false;

								switch(pos.measureformat)
								{
								case PM_1111:
								case PM_1p1p1p1:
									if(pos.pos[2]==1 && pos.pos[3]==1)  /// 1.1.1.1
										showbeat=true;
									break;

								case PM_1110:
								case PM_1p1p1p0:
									if(pos.pos[2]==1 && pos.pos[3]==0) /// 1.1.1.0
										showbeat=true;
									break;

								case PM_11_1:
								case PM_1p1p_1:
									if(pos.pos[2]==1) /// 1.1.1
										showbeat=true;
									break;

								case PM_11_0:
								case PM_1p1p_0:
									if(pos.pos[2]==0) /// 1.1.0
										showbeat=true;
									break;
								}

								if(pos.pos[1]==1 && showbeat==true) // 125.1.1.1, 125.1.1
								{
									lastbeat=1;

									if(win->zoom->withzoom==false)
									{
										LONGLONG q=pos.pos[0]-1,qh=q/win->zoom->measureraster;
										qh*=win->zoom->measureraster;

										if(q!=qh)
											goto noshow_ms;
									}	

									checkzoom=false; // skip zoom

									int px=ConvertTimeToX(time);

									bm->guiDrawLineX(px,y2/2,y2,COLOUR_WHITE); // With Number
									AddPosition(time,px,HEAD_POSITION_1xxx,true,pos.pos[0]);
noshow_ms:
									if(win->zoom->withzoom==true && showzoom!=-1)
										time+=showzoom;
									else
										time=win->zoom->withzoom==false?win->WindowSong()->timetrack.ConvertTicksToNextMeasureTicks(time+1):win->WindowSong()->timetrack.ConvertTicksToNextBeatTicks(time+1);	
								}

								if(checkzoom==true)
								{
									int px=ConvertTimeToX(time);

									if(win->zoom->withzoom==true && showzoom!=-1)
									{
										if(lastbeat==pos.pos[1])
										{
											bm->guiDrawLineX(px,splity,y2,COLOUR_GREY_DARK);
											AddPosition(time,px,HEAD_POSITION_xx1x,false,pos.pos[0],pos.pos[1],pos.pos[2]);
										}
										else
										{
											lastbeat=pos.pos[1];

											// 1.x.1.1
											bm->guiDrawLineX(px,splity,y2,COLOUR_GREY);

											if(win->zoom->show1x==true)
												AddPosition(time,px,HEAD_POSITION_x1xx,true,pos.pos[0],pos.pos[1]);
											else
												AddPosition(time,px,HEAD_POSITION_x1xx,false);
										}

										time+=showzoom;
									}
									else
									{
										// 1.x.1.1

										bm->guiDrawLineX(px,splity,y2,COLOUR_GREY);

										if(win->zoom->show1x==true)
											AddPosition(time,px,HEAD_POSITION_x1xx,true,pos.pos[0],pos.pos[1]);
										else
											AddPosition(time,px,HEAD_POSITION_x1xx,false);

										time=win->WindowSong()->timetrack.ConvertTicksToNextBeatTicks(time+1);
									}
								}

							}
							break;
						}// switch
					}// while
				}
				break;
		}
		else // No Song Format
		{
			// Format MEASURE not possible

			switch(format)
			{
			case WINDOWDISPLAY_SMPTE: //frames
			case WINDOWDISPLAY_SECONDS: // min - ms
			case WINDOWDISPLAY_SAMPLES:
				{
					//OSTART time;
					//double zoomsec;
					//LONGLONG samplepos,samples;

					// Quantize -> Next
					switch(format)
					{
					case WINDOWDISPLAY_SAMPLES:
						{

						}
						break;

					case WINDOWDISPLAY_SECONDS:
						{

						}
						break;

					case WINDOWDISPLAY_SMPTE:

						break;
					}

					//LONGLONG 
					/*
					while(time<=win->endposition)
					{
					switch(format)
					{
					case WINDOWDISPLAY_SAMPLES:
					{
					int px=ConvertTimeToX(time);

					bm->guiDrawLineX(px,y2/2,y2,COLOUR_WHITE); // With Number
					AddPosition(samplepos,px,HEAD_POSITION_1xxx,true,0);

					samplepos+=samples;
					time=win->WindowSong()->timetrack.ConvertSamplesToOSTART(samplepos);
					}
					break;

					case WINDOWDISPLAY_SECONDS:
					{
					int px=ConvertTimeToX(time);

					bm->guiDrawLineX(px,y2/2,y2,COLOUR_WHITE); // With Number
					AddPosition(time,px,HEAD_POSITION_1xxx,true,0);

					time=win->WindowSong()->timetrack.ConvertTicksToNextMs(time,zoomsec,true);
					}
					break;

					case WINDOWDISPLAY_SMPTE:
					{
					bool checkzoom=win->zoom->withzoom;

					win->WindowSong()->timetrack.ConvertTicksToPos(time,&pos);
					if(pos.pos[1]==1 && pos.pos[2]==1 && pos.pos[3]==1)
					{
					if(win->zoom->withzoom==false)
					{
					int q=pos.pos[0]-1,qh=q/win->zoom->measureraster;
					qh*=win->zoom->measureraster;

					if(q!=qh)
					goto noshow_ms;
					}	

					checkzoom=false;

					int px=ConvertTimeToX(time);

					bm->guiDrawLineX(px,y2/2,y2,COLOUR_WHITE); // With Number
					AddPosition(time,px,HEAD_POSITION_1xxx,true,pos.pos[0]);

					noshow_ms:
					time=win->zoom->withzoom==false?win->WindowSong()->timetrack.ConvertTicksToNextMeasureTicks(time+1):win->WindowSong()->timetrack.ConvertTicksToNextBeatTicks(time+1);	
					}

					if(checkzoom==true)
					{
					int px=ConvertTimeToX(time);

					bm->guiDrawLineX(px,splity,y2,COLOUR_GREY_DARK);

					if(win->zoom->show1x==true)
					AddPosition(time,px,HEAD_POSITION_x1xx,true,pos.pos[0],pos.pos[1]);
					else
					AddPosition(time,px,HEAD_POSITION_x1xx,false);

					time=win->WindowSong()->timetrack.ConvertTicksToNextBeatTicks(time+1);
					}

					}
					break;
					}// switch
					}// while
					*/
				}
				break;
			} // switch

		}//else

		switch(format)
		{
		case WINDOWDISPLAY_SAMPLES:
			bm->SetTextColour(COLOUR_SAMPLES);
			break;

		case WINDOWDISPLAY_SECONDS:
			bm->SetTextColour(COLOUR_TIME);
			break;

		case WINDOWDISPLAY_SMPTE:
			bm->SetTextColour(COLOUR_SMPTE);
			break;

		default:
			bm->SetTextColour(COLOUR_WHITE);
			break;
		}

		bool textshow=false;

		bm->SetFont(&maingui->standard_bold);

		char *h=0;
		char nrs[NUMBERSTRINGLEN];
		int lx2=-1;

		// Show Pos
		guiTimeLinePos *pos=FirstPosition();

		while(pos)
		{
			h=0;

			if(pos->showtext==true && (lx2==-1 || lx2+maingui->GetFontSizeY()<=pos->x))
			{
				switch(pos->type)
				{
				case HEAD_POSITION_1xxx:
					{
						/*
						if(mainsettings->showbothsformatsintimeline==true && y2-2*maingui->GetFontSizeY()>y+2)
						{
						int hy=y+1+maingui->GetFontSizeY();

						bm->guiDrawNumber(pos->x,hy,maxx2,pos->measure);

						hy=maingui->AddFontY(hy);
						song->timetrack.CreateTimeString(&timestring,pos->ostart,song->project->standardsmpte);
						bm->guiDrawText(pos->x+2,hy,maxx2,timestring.string);
						}
						else
						*/

						switch(format)
						{
						case WINDOWDISPLAY_MEASURE:
							{
								h=mainvar->ConvertLongLongToChar(pos->measure,nrs);
								bm->guiDrawText(pos->x,ty2,x2,h);
							}
							break;

						case WINDOWDISPLAY_SAMPLES:
							{	
								h=mainvar->ConvertLongLongToChar(pos->ostart,nrs);
								bm->guiDrawText(pos->x,ty2,x2,h);
							}
							break;

						case WINDOWDISPLAY_SECONDS:
							{
								win->WindowSong()->timetrack.CreateTimeString(&timestring,pos->ostart,Seq_Pos::POSMODE_TIME,Seq_Time::TIMESIMPLE);
								bm->guiDrawText(pos->x,ty2,x2,h=timestring.string);
							}
							break;

						case WINDOWDISPLAY_SMPTE:
							{
								win->WindowSong()->timetrack.CreateTimeString(&timestring,pos->ostart,win->WindowSong()->project->standardsmpte/*Seq_Time::TIMESIMPLE*/);
								bm->guiDrawText(pos->x,ty2,x2,h=timestring.string);
							}
							break;

						}// switch format

						textshow=true;
					}
					break;

				case HEAD_POSITION_x1xx:
					{
						if(mainsettings->showbothsformatsintimeline==true && y2-2*maingui->GetFontSizeY()>y+2)
						{
							int hy=ty2;

							bm->guiDrawNumber(pos->x,hy,x2,pos->measure);
							hy=maingui->AddFontY(hy);
							win->WindowSong()->timetrack.CreateTimeString(&timestring,pos->ostart,win->WindowSong()->project->standardsmpte);
							bm->guiDrawText(pos->x+2,hy,x2,h=timestring.string);
						}
						else
							switch(format)
						{
							case WINDOWDISPLAY_MEASURE:
								{
									char h2[32],h3[32],
										*hs=mainvar->ConvertLongLongToChar(pos->measure,h2),
										*hs2=mainvar->ConvertLongLongToChar(pos->beat,h3),
										*s=mainvar->GenerateString(hs,".",hs2);

									if(s)
									{
										bm->SetFont(&maingui->smallfont);

										//										bm->guiFillRect(pos->x,0,pos->x+40,y+1+maingui->GetFontSizeY(),COLOUR_BLACK);
										bm->guiDrawText(pos->x,ty2,x2,s);
										lx2=pos->x+bm->GetTextWidth(s); // new font
										bm->SetFont(&maingui->standard_bold);
										delete s;
									}

									//bm->guiDrawNumber(pos->x,y+1+maingui->GetFontSizeY(),maxx2,pos->measure);
								}
								break;

							default: //SMPTE
								{
									win->WindowSong()->timetrack.CreateTimeString(&timestring,pos->ostart,win->WindowSong()->project->standardsmpte,Seq_Time::TIMESIMPLE);
									bm->guiDrawText(pos->x,ty2,x2,h=timestring.string);
								}
								break;
						}// switch format

						textshow=true;

					}break;
				}
			}

			if(h)
				lx2=pos->x+bm->GetTextWidth(h);

			pos=pos->NextPosition();
		}

		if(textshow==false)
		{
			bm->SetTextColour(COLOUR_BLUE_LIGHT);

			guiTimeLinePos *pos=FirstPosition();

			while(pos)
			{
				switch(pos->type)
				{
				case HEAD_POSITION_xx1x:
					{
						switch(format)
						{
						case WINDOWDISPLAY_MEASURE:
							{
								char h2[32],h3[32],h4[32],
									*hs=mainvar->ConvertLongLongToChar(pos->measure,h2),
									*hs2=mainvar->ConvertLongLongToChar(pos->beat,h3),
									*hs3=mainvar->ConvertLongLongToChar(pos->zoom,h4),
									*s=mainvar->GenerateString(hs,".",hs2,".",hs3);

								if(s)
								{
									bm->SetFont(&maingui->smallfont);

									//										bm->guiFillRect(pos->x,0,pos->x+40,y+1+maingui->GetFontSizeY(),COLOUR_BLACK);
									bm->guiDrawText(pos->x,ty2,x2,s);
									bm->SetFont(&maingui->standard_bold);

									delete s;
								}

								textshow=true;
							}
							break;
						}// switch format

					}break;
				}

				pos=pos->NextPosition();
			}
		}

		if(win->songmode==true)
		{
			if(format==WINDOWDISPLAY_MEASURE)
			{
				// Signature
				bm->SetFont(&maingui->standardfont);
				maingui->colourtable.GetRGB(COLOUR_BLUE_LIGHT,&r,&g,&b);
				bm->SetTextColour(r,g,b);

				char text[NUMBERSTRINGLEN],h[NUMBERSTRINGLEN];

				Seq_Signature *sig=win->WindowSong()->timetrack.FindSignatureBefore(win->startposition);
				while(sig && sig->GetSignatureStart()<win->endposition)
				{
					OSTART sx=sig->GetSignatureStart()<=win->startposition?0:ConvertTimeToX(sig->GetSignatureStart());

					sx+=2;

					text[0]=0;

					mainvar->AddString(text,mainvar->ConvertIntToChar(sig->nn,h));
					mainvar->AddString(text,"/");
					mainvar->AddString(text,mainvar->ConvertIntToChar(sig->dn,h));

					bm->guiDrawTextFontY(sx,y2,x2,text);

					sig=sig->NextSignature();
				}
			}

			// Marker ?
			if(y+12<splity)
			{
				Seq_Marker *m=win->WindowSong()->textandmarker.FirstMarker();

				while(m && m->GetMarkerStart()<=win->endposition)
				{
					int mx=-1,mx2=-1;

					if(m->GetMarkerStart()>=win->startposition)
					{
						mx=ConvertTimeToX(m->GetMarkerStart());

						if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
						{
							mx2=m->GetMarkerEnd()>=win->endposition?2:ConvertTimeToX(m->GetMarkerEnd());
						}
					}
					else // Start <<....|
					{
						if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
						{
							if(m->GetMarkerEnd()>win->startposition)
							{
								mx=x;
								mx2=m->GetMarkerEnd()>=win->endposition?x2:ConvertTimeToX(m->GetMarkerEnd());
							}
						}
					}

					if(mx!=-1 && (mx2!=-1 || m->markertype==Seq_Marker::MARKERTYPE_SINGLE))
					{
						/*
						if(m->colour.showcolour==true)
						{
						bm->guiFillRect_RGB
						(
						mx,
						splity-4,
						mx2,
						splity,
						m->colour.rgb);
						}
						*/

						bm->guiDrawText(mx,splity,x2,m->string);
					}

					m=m->NextMarker();
				}
			}
		}
	}
}

void guiControlBox::ShowTime()
{
	if(timebutton)
	{
		switch(editor->windowtimeformat)
		{
		case WINDOWDISPLAY_MEASURE:
			//timebutton->ChangeButtonImage(IMAGE_MEASURE_SMALL);
			timebutton->ChangeButtonText("Time");
			break;

		case WINDOWDISPLAY_SMPTE:
			{
				timebutton->ChangeButtonText(smpte_modestring[editor->WindowSong()->project->standardsmpte]);
				/*
				switch(editor->WindowSong()->project->standardsmpte)
				{
				case Seq_Pos::POSMODE_SMPTE_24:
				timebutton->ChangeButtonImage(IMAGE_24FPS_SMALL);
				break;

				case Seq_Pos::POSMODE_SMPTE_25:
				timebutton->ChangeButtonImage(IMAGE_25FPS_SMALL);
				break;

				case Seq_Pos::POSMODE_SMPTE_2997:
				timebutton->ChangeButtonImage(IMAGE_297FPS_SMALL);
				break;

				case Seq_Pos::POSMODE_SMPTE_30:
				timebutton->ChangeButtonImage(IMAGE_30FPS_SMALL);
				break;
				}
				*/
			}
			break;
		}
	}
}

void guiControlBox::ShowStatus(int nstatus)
{
	if(nstatus!=status)
	{
		status=nstatus;

		if(startbutton)
			startbutton->ChangeButtonImage(status&Seq_Song::STATUS_PLAY?IMAGE_PLAYBUTTON_SMALL_ON:IMAGE_PLAYBUTTON_SMALL_OFF);

		if(recordbutton)
			recordbutton->ChangeButtonImage((status&(Seq_Song::STATUS_RECORD|Seq_Song::STATUS_WAITPREMETRO|Seq_Song::STATUS_STEPRECORD))?IMAGE_RECORDBUTTON_SMALL_ON:IMAGE_RECORDBUTTON_SMALL_OFF);

		if(stopbutton)
			stopbutton->ChangeButtonImage(status==Seq_Song::STATUS_STOP?IMAGE_STOPBUTTON_SMALL_ON:IMAGE_STOPBUTTON_SMALL_OFF);
	}
}


void EventEditor::ShowEditorHeader(guiGadget_CW *db)
{
	if(db)
	{
		if(!timeline)
			timeline=new guiTimeLine;

		if(timeline)
		{
			timeline->showzoom=GetTimeLineGrid();

			timeline->zoom=zoom;
			timeline->dbgadget=db;

			timeline->x=0;
			timeline->y=0;
			timeline->x2=db->GetX2();
			timeline->y2=db->GetY2();

			int h2=db->GetHeight()/2;
			int h3=h2+((db->GetHeight()-h2)/4);
			//int redy=h3+((db->GetHeight()-h3)/2);

			timeline->format=windowtimeformat;
			timeline->win=this;
			timeline->bitmap=&db->gbitmap;
			//timeline->song=WindowSong();
			timeline->splity=h3;
			//timeline->smallbeaty=h3-(h3/3);
			//	timeline->redy=redy;
			timeline->flag=headerflag;

			timeline->Draw();
			timeline->ShowCycleAndPositions();
		}
	}
}