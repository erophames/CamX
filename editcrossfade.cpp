#include "editcrossfade.h"
#include "languagefiles.h"
#include "gui.h"
#include "songmain.h"
#include "object_track.h"
#include "object_song.h"
#include "arrangeeditor.h"
#include "audiofile.h"
#include "audiohdfile.h"
#include "audiohardware.h"
#include "crossfade.h"
#include "audiopattern.h"
#include "audiorealtime.h"
#include "semapores.h"

// Gadgets
#define CROSSFADEGADGETID_START (GADGET_ID_START+50)

enum GIDs
{
	GADGET_PLAYOUT=(CROSSFADEGADGETID_START+1),
	GADGET_PLAYOUT_RAW,
	GADGET_PLAYIN,
	GADGET_PLAYIN_RAW,
	GADGET_PLAYMIX,
	GADGET_PLAYMIX_RAW,
	GADGET_STOP,
	GADGET_INFO,
	GADGET_ENABLE,
	GADGET_TEXTSIZE
};

Edit_CrossFade::Edit_CrossFade(Seq_CrossFade *cf)
{
	editorid=EDITORTYPE_CROSSFADE;
	playback.autODeInit=playback2.autODeInit=false;
	status=STOPPED;
	audiorealtime=audiorealtime2=0;
	infile=outfile=0;
	out=cf;
	in=cf->connectwith;
	boxenable=0;
	p1name=p2name=0;
};

void Edit_CrossFade::CheckIfClose()
{
	int indextrack=WindowSong()->GetOfTrack(track);

	if(indextrack==-1)
	{
		closeit=true;
		return;
	}

	int index_out=track->GetOfPattern(outpattern,MEDIATYPE_ALL),
		index_in=track->GetOfPattern(inpattern,MEDIATYPE_ALL);

	if(index_out==-1 || index_in==-1)
	{
		closeit=true;
		return;
	}

	if(outpattern->FindCrossFade(inpattern)==0)
		closeit=true;
}

EditData *Edit_CrossFade::EditDataMessage(EditData *data)
{
	return data;
}

void Edit_CrossFade::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_STOP:
		StopPlayback();
		break;

	case GADGET_PLAYMIX_RAW:
		{
			StopPlayback();

			playback.r_audiohdfile=outfile;
			playback.regionstart=out->from_sample_file;
			playback.regionend=out->to_sample_file;
			playback.InitRegion();

			if(playback.crossfade)
			{
				playback.crossfade->DeInit();
				delete playback.crossfade;
				playback.crossfade=0;
			}

			audiorealtime=outfile->PlayRealtime(this,mainvar->GetActiveSong(),&playback,0,&endstatus_realtime);

			playback2.r_audiohdfile=infile;
			playback2.regionstart=in->from_sample_file;
			playback2.regionend=in->to_sample_file;
			playback2.InitRegion();

			if(playback2.crossfade)
			{
				playback2.crossfade->DeInit();
				delete playback2.crossfade;
				playback2.crossfade=0;
			}

			audiorealtime2=infile->PlayRealtime(this,mainvar->GetActiveSong(),&playback2,0,&endstatus_realtime2);

			if(audiorealtime && audiorealtime2)
				status=STARTEDMIX;
		}
		break;

	case GADGET_PLAYMIX:
		{
			StopPlayback();

			playback.r_audiohdfile=outfile;
			playback.regionstart=out->from_sample_file;
			playback.regionend=out->to_sample_file;
			playback.InitRegion();

			if(playback.crossfade)
			{
				playback.crossfade->DeInit();
				delete playback.crossfade;
			}

			playback.crossfade=(Seq_CrossFade *)out->Clone();

			audiorealtime=outfile->PlayRealtime(this,mainvar->GetActiveSong(),&playback,0,&endstatus_realtime);
		
			playback2.r_audiohdfile=infile;
			playback2.regionstart=in->from_sample_file;
			playback2.regionend=in->to_sample_file;
			playback2.InitRegion();

			if(playback2.crossfade)
			{
				playback2.crossfade->DeInit();
				delete playback2.crossfade;
			}

			playback2.crossfade=(Seq_CrossFade *)in->Clone();

			audiorealtime2=infile->PlayRealtime(this,mainvar->GetActiveSong(),&playback2,0,&endstatus_realtime2);

			if(audiorealtime && audiorealtime2)
				status=STARTEDMIX;
		}
		break;

	case GADGET_PLAYOUT:
		{
			StopPlayback();

			playback.r_audiohdfile=outfile;
			playback.regionstart=out->from_sample_file;
			playback.regionend=out->to_sample_file;
			playback.InitRegion();

			if(playback.crossfade)
			{
				playback.crossfade->DeInit();
				delete playback.crossfade;
			}

			playback.crossfade=(Seq_CrossFade *)out->Clone();

			audiorealtime=outfile->PlayRealtime(this,mainvar->GetActiveSong(),&playback,0,&endstatus_realtime);

			if(audiorealtime)
				status=STARTEDOUT;
		}
		break;

	case GADGET_PLAYOUT_RAW:
		{
			StopPlayback();

			playback.r_audiohdfile=outfile;
			playback.regionstart=out->from_sample_file;
			playback.regionend=out->to_sample_file;
			playback.InitRegion();

			if(playback.crossfade)
			{
				playback.crossfade->DeInit();
				delete playback.crossfade;
				playback.crossfade=0;
			}

			audiorealtime=outfile->PlayRealtime(this,mainvar->GetActiveSong(),&playback,0,&endstatus_realtime);

			if(audiorealtime)
				status=STARTEDOUT;
		}
		break;

	case GADGET_PLAYIN:
		{
			StopPlayback();

			playback.r_audiohdfile=infile;
			playback.regionstart=in->from_sample_file;
			playback.regionend=in->to_sample_file;
			playback.InitRegion();

			if(playback.crossfade)
			{
				playback.crossfade->DeInit();
				delete playback.crossfade;
			}

			playback.crossfade=(Seq_CrossFade *)in->Clone();

			audiorealtime=infile->PlayRealtime(this,mainvar->GetActiveSong(),&playback,0,&endstatus_realtime);

			if(audiorealtime)
				status=STARTEDIN;
		}
		break;

	case GADGET_PLAYIN_RAW:
		{
			StopPlayback();

			playback.r_audiohdfile=infile;
			playback.regionstart=in->from_sample_file;
			playback.regionend=in->to_sample_file;
			playback.InitRegion();

			if(playback.crossfade)
			{
				playback.crossfade->DeInit();
				delete playback.crossfade;
				playback.crossfade=0;
			}

			audiorealtime=infile->PlayRealtime(this,mainvar->GetActiveSong(),&playback,0,&endstatus_realtime);

			if(audiorealtime)
				status=STARTEDIN;
		}
		break;

	case GADGET_ENABLE:
		out->Toggle();
		enable=out->used;
		break;
	}
}

void Edit_CrossFade::MouseButton(int flag)
{
#ifdef OLDIE

	switch(left_mousekey)
	{
	case MOUSEKEY_DOWN:
		{
			for(int i=0;i<2;i++)
			{
				for(int c=0;c<8;c++)
				{
					int cx=i==0?o_x[c]:i_x[c];
					int cy=i==0?o_y[c]:i_y[c];
					int cx2=i==0?o_x2[c]:i_x2[c];
					int cy2=i==0?o_y2[c]:i_y2[c];

					if(GetMouseX()>=cx && GetMouseX()<cx2 && GetMouseY()>cy && GetMouseY()<=cy2)
					{
						bool ok=false;

						if(i==0)
						{
							ok=out->ChangeType(c);
						}
						else
						{
							ok=in->ChangeType(c);
						}

						if(ok==true)
						{
							if(i==0)ShowOutCurve();
							else ShowInCurve();

							ShowMix();
							ShowCrossFades();
						}
					}
				}
			}

		}
		break;
	}
#endif

}

void Edit_CrossFade::LoadSettings()
{

}

void Edit_CrossFade::SaveSettings()
{

}

guiMenu *Edit_CrossFade::CreateMenu()
{
	if(menu)menu->RemoveMenu();

	if(menu=new guiMenu)
	{

	}

	return menu;
}

void Edit_CrossFade::DeInitWindow()
{
	StopPlayback();

	if(p1name)delete p1name;
	p1name=0;

	if(p2name)delete p2name;
	p2name=0;
}

void Edit_CrossFade::ResetGadgets()
{
	p1name_gadget=p2name_gadget=boxenable=sizetext=0;
}

void Edit_CrossFade::ShowOutName()
{
	if(p1name)delete p1name;
	p1name=0;

	if(p1name_gadget)
	{
		p1name=mainvar->GenerateString(outpattern->GetName());

		if(char *h=mainvar->GenerateString("[> Out]:",outpattern->GetName()))
		{
			p1name_gadget->ChangeButtonText(h);
			delete h;
		}
	}
}

void Edit_CrossFade::ShowInName()
{
	if(p2name)delete p2name;
	p2name=0;

	if(p2name_gadget)
	{
		p2name=mainvar->GenerateString(inpattern->GetName());

		if(char *h=mainvar->GenerateString("[< In]:",inpattern->GetName()))
		{
			p2name_gadget->ChangeButtonText(h);
			delete h;
		}
	}
}

void Edit_CrossFade::ShowLength()
{
	if(sizetext)
	{
		TimeString string;

		mainaudio->ConvertSamplesToTime(in_end-in_start,&string.pos);
		string.pos.ConvertToString(WindowSong(),string.string,70);

		char *h=mainvar->GenerateString(Cxs[CXS_LENGTH],":",string.string);

		if(h)
		{
			sizetext->ChangeButtonText(h);
			delete h;
		}
	}
}

int Edit_CrossFade::ConvertSamplePositionX(LONGLONG pos)
{
	if(audiorealtime && audiorealtime->audiopattern.audioevent.audioregion && pos<=audiorealtime->audiopattern.audioevent.audioregion->regionend){

		pos-=audiorealtime->audiopattern.audioevent.audioregion->regionstart;

		int x=audiorealtime_x;
		double w=audiorealtime_x2-audiorealtime_x,per=pos;

		per/=audiorealtime->audiopattern.audioevent.audioregion->GetLength();
		w*=per;
		x+=(int)floor(w+0.5f);

		return x;
	}

	return -1;
}

void Edit_CrossFade::RefreshRealtime()
{
	bool showmix=false;

	#ifdef OLDIE

	AudioPattern *aout=(AudioPattern *)outpattern,*ain=(AudioPattern *)inpattern;

#ifdef DEBUG
	Seq_CrossFade *c=aout->FirstCrossFade();
	while(c)
	{
		if(out==c)break;
		c=c->NextCrossFade();
	}

	if(!c)
		maingui->MessageBoxOk(0,"Out CF Illegal");

	c=ain->FirstCrossFade();
	while(c)
	{
		if(in==c)break;
		c=c->NextCrossFade();
	}

	if(!c)
		maingui->MessageBoxOk(0,"In CF Illegal");

#endif

	if(infile!=ain->audioevent.audioefile || 
		in->from_sample_file!=in_start ||
		in->to_sample_file!=in_end
		)
	{
		ShowInCurve();
		showmix=true;
	}

	if(outfile!=aout->audioevent.audioefile ||
		out->from_sample_file!=out_start ||
		out->to_sample_file!=out_end
		)
	{
		ShowOutCurve();
		showmix=true;
	}

	if(showmix==true)
	{
		ShowLength();
		ShowMix();
	}

	if(audiorealtime)
	{
		// Check If Realtime Audio Stopped
		mainthreadcontrol->Lock(CS_audiorealtime);
		if(mainaudioreal->FindAudioRealtime(audiorealtime)==false)
		{
			audiorealtime=0;
			status=STOPPED;
		}
		mainthreadcontrol->Unlock(CS_audiorealtime);

		if(audiorealtime)
		{
			switch(status)
			{
			case STARTEDMIX:
				audiorealtime_x=mix_x;
				audiorealtime_x2=mix_x2;
				audiorealtime_y=mix_y;
				audiorealtime_y2=mix_y2;
				break;

			case STARTEDOUT:
				audiorealtime_x=xfrom;
				audiorealtime_x2=x2from;
				audiorealtime_y=yfrom;
				audiorealtime_y2=y2from;
				break;

			case STARTEDIN:
				audiorealtime_x=xto;
				audiorealtime_x2=x2to;
				audiorealtime_y=yto;
				audiorealtime_y2=y2to;
				break;
			}

			//TRACE (audiorealtime->audiopattern.audioevent.audioregion->
			int x=ConvertSamplePositionX(audiorealtime->audiopattern.audioevent.sampleposition);

			if(x==-1)
				RemoveSprite(&filepositionsprite);
			else
				if(x!=filepositionsprite.x || filepositionsprite.ondisplay==false)
				{
					ClearSprite(&filepositionsprite);

					filepositionsprite.x=x;
					filepositionsprite.y=audiorealtime_y;
					filepositionsprite.y2=audiorealtime_y2;

					//AddSprite(&filepositionsprite);
					ShowAllSprites();
				}
		}
		else
			ClearSprite(&filepositionsprite);

	}

	if(boxenable)
	{
		if(enable!=out->used)boxenable->SetCheckBox(enable=out->used);

		if(p1name_gadget)
		{
			if((!p1name) || strcmp(outpattern->GetName(),p1name)!=0)
				ShowOutName();
		}

		if(p2name_gadget)
		{
			if((!p2name) || strcmp(inpattern->GetName(),p2name)!=0)
				ShowInName();
		}
	}
#endif

}


void Edit_CrossFade::RedrawGfx()
{
	ShowCrossFades();
	ShowOutCurve();
	ShowInCurve();
	ShowMix();
}

void Edit_CrossFade::StopPlayback()
{
	status=STOPPED;

	if(audiorealtime){
		mainaudioreal->StopAudioRealtime(audiorealtime,&endstatus_realtime);
		audiorealtime=0;
	}

	if(audiorealtime2){
		mainaudioreal->StopAudioRealtime(audiorealtime2,&endstatus_realtime2);
		audiorealtime2=0;
	}
}

void Edit_CrossFade::ShowInCurve()
{
	#ifdef OLDIE
	guibuffer->guiFillRect(xto,yto,x2to,y2to,COLOUR_BACKGROUND);

	for(int x=xto;x<x2to;x++)
	{
		double h2=x2to-xto,h1=x-xto;
		h1/=h2;
		double h=in->ConvertToVolume(h1,true),y=y2to,hy=y2to-yto;
		hy*=h;
		y-=hy;

		guibuffer->guiDrawLine(x,y2to,x,y,COLOUR_RED_LIGHT);
	}

	AudioPattern *audiopattern=(AudioPattern *)inpattern;
	AudioGFX g;

	g.dontclearbackground=true;
	g.showmix=false;
	g.showregion=true;

	in_start=g.regionstart=in->from_sample_file;
	in_end=g.regionend=in->to_sample_file;

	double sppixel=in->to_sample_file-in->from_sample_file;
	sppixel/=x2to-xto;
	g.samplesperpixel=(int)floor(sppixel+0.5); 
	g.win=this;
	g.bitmap=guibuffer;
	g.usebitmap=true;
	g.x=xto;
	g.samplex2=g.x2=x2to;
	g.y=yto;
	g.y2=y2to;
	g.drawcolour=COLOUR_BLACK;

	infile=audiopattern->audioevent.audioefile;
	infile->ShowAudioFile(&g);

	BltGUIBufferDirect(xto,yto,x2to,y2to);
#endif

}

void Edit_CrossFade::ShowOutCurve()
{
	#ifdef OLDIE
	guibuffer->guiFillRect(xfrom,yfrom,x2from,y2from,COLOUR_BACKGROUND);

	for(int x=xfrom;x<x2from;x++)
	{
		double h2=x2from-xfrom,h1=x;
		h1/=h2;
		double h=out->ConvertToVolume(h1,false),y=y2from,hy=y2from-yfrom;
		hy*=h;
		y-=hy;
		guibuffer->guiDrawLine(x,y2from,x,y,COLOUR_BLUE_LIGHT);
	}

	AudioPattern *audiopattern=(AudioPattern *)outpattern;
	AudioGFX g;

	g.dontclearbackground=true;
	g.showmix=false;
	g.showregion=true;
	out_start=g.regionstart=out->from_sample_file;
	out_end=g.regionend=out->to_sample_file;

	double sppixel=out->to_sample_file-out->from_sample_file;
	sppixel/=x2from-xfrom;
	g.samplesperpixel=(int)floor(sppixel+0.5); 
	g.win=this;
	g.bitmap=guibuffer;
	g.usebitmap=true;
	g.x=xfrom;
	g.samplex2=g.x2=x2from;
	g.y=yfrom;
	g.y2=y2from;

	g.drawcolour=COLOUR_BLACK;

	outfile=audiopattern->audioevent.audioefile;
	outfile->ShowAudioFile(&g);

	BltGUIBufferDirect(xfrom,yfrom,x2from,y2from);

#endif
}

void Edit_CrossFade::ShowMix()
{
	#ifdef OLDIE

	guibuffer->guiFillRect(mix_x,mix_y,mix_x2,mix_y2,COLOUR_AUDIO_BACKGROUND);

	AudioPattern *audiopattern=(AudioPattern *)outpattern;
	AudioGFX g;

	g.dontclearbackground=true;
	g.showmix=true;
	g.showregion=true;
	g.regionstart=out->from_sample_file;
	g.regionend=out->to_sample_file;

	double sppixel=out->to_sample_file-out->from_sample_file;
	sppixel/=mix_x2-mix_x;
	g.samplesperpixel=(int)floor(sppixel+0.5); 
	g.win=this;
	g.bitmap=guibuffer;
	g.usebitmap=true;
	g.x=mix_x;
	g.samplex2=g.x2=mix_x2;
	g.y=mix_y;
	g.y2=mix_y2;
	g.crossfade=out;
	g.drawcolour=COLOUR_BLUE;

	//outfile=audiopattern->audioevent.audioefile;
	outfile->ShowAudioFile(&g);

	// IN
	audiopattern=(AudioPattern *)inpattern;
	//infile=audiopattern->audioevent.audioefile;

	g.crossfade=in;
	g.drawcolour=COLOUR_RED;

	g.regionstart=in->from_sample_file;
	g.regionend=in->to_sample_file;

	infile->ShowAudioFile(&g);
	BltGUIBufferDirect(mix_x,mix_y,mix_x2,mix_y2);
#endif
}

void Edit_CrossFade::ShowCrossFades()
{
	#ifdef OLDIE
	for(int i=0;i<2;i++)
	{
		int x=i==0?outx:inx,
			x2=i==0?outx2:inx2,
			y=i==0?outy:iny,
			y2=i==0?outy2:iny2,
			pxw=(x2-x)/2,
			pyw=(y2-y)/4,
			dx=x,
			dy,
			c=0;

		for(int px=0;px<2;px++)
		{
			dy=y;

			for(int py=0;py<4;py++)
			{
				Seq_CrossFade *cf=i==0?out:in;
				int sx,sy,sx2,sy2;

				if(dx<dx+pxw-1 && dy<dy+pyw-1)
				{
					guibuffer->guiDrawLine(dx,dy,dx+pxw-1,dy,COLOUR_WHITE);
					guibuffer->guiDrawLine(dx,dy,dx,dy+pyw-1,COLOUR_WHITE);
					guibuffer->guiDrawLine(dx,dy+pyw-1,dx+pxw-1,dy+pyw-1,COLOUR_GREY_DARK);
					guibuffer->guiDrawLine(dx+pxw-1,dy,dx+pxw-1,dy+pyw-1,COLOUR_GREY_DARK);

					if(cf->type==c)
						guibuffer->guiFillRect(dx+1,dy+1,dx+pxw-2,dy+pyw-2,i==0?COLOUR_BLUE_LIGHT:COLOUR_RED_LIGHT);
					else
						guibuffer->guiFillRect(dx+1,dy+1,dx+pxw-2,dy+pyw-2,COLOUR_GREY_LIGHT);

					sx=dx+1;
					sy=dy+1;
					sx2=dx+pxw-1;
					sy2=dy+pyw-1;

					if(sx<sx2 && sy<sy2)
					{
						if(i==0)
						{
							o_x[c]=dx;
							o_x2[c]=dx+pxw-1;
							o_y[c]=dy;
							o_y2[c]=dy+pyw-1;
						}
						else
						{
							i_x[c]=dx;
							i_x2[c]=dx+pxw-1;
							i_y[c]=dy;
							i_y2[c]=dy+pyw-1;
						}

						for(int cx=sx;cx<sx2;cx++)
						{
							double h2=sx2-sx,h1=cx-sx;
							h1/=h2;
							double h=out->ConvertToVolume(h1,i==0?false:true,c),cy=sy2,hy=sy2-sy;
							hy*=h;
							cy-=hy;
							guibuffer->guiDrawPixel(cx,cy);
						}
					}

					BltGUIBufferDirect(dx,dy,dx+pxw+1,dy+pyw);
				}

				dy+=pyw;
				c++;
			}

			dx+=pxw;
		}
	}
#endif
}

void Edit_CrossFade::Init()
{
	#ifdef OLDIE
	ResetGadgets();

	gadgetlists.RemoveAllGadgetLists();

	if(p1name)delete p1name;
	p1name=0;

	if(p2name)delete p2name;
	p2name=0;

	track=out->pattern->track;

	outpattern=out->pattern;
	inpattern=in->pattern;

	if(gadgetlist=gadgetlists.AddGadgetList(this))
	{
		int x=0,x2=width/2,y=maingui->GetFontSizeY();

		boxenable=gadgetlist->AddCheckBox(x,0,x2,y,GADGET_ENABLE,0,Cxs[CXS_ENABLED]);
		if(boxenable)boxenable->SetCheckBox(enable=out->used);

		sizetext=gadgetlist->AddText(x2,0,width,y,0,GADGET_TEXTSIZE,0);

		y+=1;

		p1name_gadget=gadgetlist->AddText(x,y,width,y+maingui->GetFontSizeY()," ",GADGET_INFO,0,"Pattern Fade:Out");
		yfrom=y+maingui->GetFontSizeY()+1;

		int y1=y+((height-y)/3),w=width/3;

		y2from=y1-1;
		xfrom=xto=0;
		x2from=x2to=width-w;
		y2to=height-height/3;

		p2name_gadget=gadgetlist->AddText(x,y1,width,y1+maingui->GetFontSizeY()," ",GADGET_INFO,0,"Pattern Fade:In");

		ShowOutName();
		ShowInName();

		yto=y1+maingui->GetFontSizeY()+1;

		double h;

		h=glist.form->GetWidth(-1)-(x2from+1);
		h/=3;
		outx=x2from+h;
		outy=yfrom;
		outx2=glist.form->GetWidth(-1);
		outy2=y2from;

		inx=x2from+h;
		iny=yto;
		inx2=glist.form->GetWidth(-1);
		iny2=y2to;

		mix_x=xfrom;
		mix_x2=x2from;
		mix_y=y2to+1;
		mix_y2=height-1;

		//Playback Gadgets;

		// out
		if(guiBitmap *bit=maingui->gfx.FindBitMap(IMAGE_PLAYBUTTONCROSSFADE_SMALL_ON))
		{
			if(guiBitmap *bit2=maingui->gfx.FindBitMap(IMAGE_PLAYBUTTON_SMALL_ON))
			{
				gadgetlist->AddImageButton(x2from,outy,x2from+h-1,outy+bit->height,IMAGE_PLAYBUTTONCROSSFADE_SMALL_ON,GADGET_PLAYOUT,0,Cxs[CXS_PLAYOUTCROSSFADE_WITHCROSSFADE]);
				gadgetlist->AddImageButton(x2from,outy+bit->height+1,x2from+h-1,outy+1+bit->height+bit2->height,IMAGE_PLAYBUTTON_SMALL_ON,GADGET_PLAYOUT_RAW,0,Cxs[CXS_PLAYOUTCROSSFADE_WITHOUTCROSSFADE]);
				gadgetlist->AddImageButton(x2from,outy+bit->height+bit2->height+2,x2from+h-1,outy2,IMAGE_STOPBUTTON_SMALL_ON,GADGET_STOP,0);
			}
		}

		//in
		if(guiBitmap *bit=maingui->gfx.FindBitMap(IMAGE_PLAYBUTTONCROSSFADE_SMALL_ON))
		{
			if(guiBitmap *bit2=maingui->gfx.FindBitMap(IMAGE_PLAYBUTTON_SMALL_ON))
			{
				gadgetlist->AddImageButton(x2from,iny,x2from+h-1,iny+bit->height,IMAGE_PLAYBUTTONCROSSFADE_SMALL_ON,GADGET_PLAYIN,0,Cxs[CXS_PLAYINCROSSFADE_WITHCROSSFADE]);
				gadgetlist->AddImageButton(x2from,iny+bit->height+1,x2from+h-1,iny+1+bit->height+bit2->height,IMAGE_PLAYBUTTON_SMALL_ON,GADGET_PLAYIN_RAW,0,Cxs[CXS_PLAYINCROSSFADE_WITHOUTCROSSFADE]);
				gadgetlist->AddImageButton(x2from,iny+bit->height+bit2->height+2,x2from+h-1,iny2,IMAGE_STOPBUTTON_SMALL_ON,0,GADGET_STOP);
			}
		}

		//mix
		if(guiBitmap *bit=maingui->gfx.FindBitMap(IMAGE_PLAYBUTTONCROSSFADE_SMALL_ON))
		{
			if(guiBitmap *bit2=maingui->gfx.FindBitMap(IMAGE_PLAYBUTTON_SMALL_ON))
			{
				gadgetlist->AddImageButton(x2from,mix_y,x2from+h-1,mix_y+bit->height,IMAGE_PLAYBUTTONCROSSFADE_SMALL_ON,GADGET_PLAYMIX,0,Cxs[CXS_PLAYCROSSFADEMIX]);
				gadgetlist->AddImageButton(x2from,mix_y+bit->height+1,x2from+h-1,mix_y+1+bit->height+bit2->height,IMAGE_PLAYBUTTON_SMALL_ON,GADGET_PLAYMIX_RAW,0,Cxs[CXS_PLAYCROSSFADEMIX_WITHOUTVOLUME]);
				gadgetlist->AddImageButton(x2from,mix_y+bit->height+bit2->height+2,x2from+h-1,mix_y2,IMAGE_STOPBUTTON_SMALL_ON,GADGET_STOP,0);
			}
		}

		RedrawGfx();
		ShowLength();
	}
#endif

}

