#include "songmain.h"
#include "editor.h"
#include "camxgadgets.h"
#include "editbuffer.h"
#include "imagesdefines.h"
#include "arrangeeditor.h"
#include "groove.h"
#include "gui.h"
#include "object_song.h"
#include "object_track.h"
#include "objectpattern.h"
#include "audiofile.h"
#include "mainhelpthread.h"
#include "audiohardware.h"
#include "MIDIhardware.h"
#include "editfunctions.h"
#include "folder.h"
#include "icdobject.h"
#include "arrangeditor_defines.h"
#include "MIDIautomation.h"
#include "audioauto_volume.h"
#include "globalmenus.h"

extern char *channelchannelsinfo_short[];

AutomationParameter *Edit_Arrange_AutomationTrack::FindParameterMouse(int px,int py)
{
	Edit_Arrange_AutomationTrackParameter *atp=FirstTrackParameter();
	while(atp)
	{
		if(atp->IsUnderMouse(px,py)==true)
			return atp->parameter;

		atp=atp->NextAutomationTrackObject();
	}

	return 0;
}

void Edit_Arrange_AutomationTrack::InitAutomationMouseMove()
{
	editor->SetMouseMode(EM_MOVEAUTOMATION,automationtrack);

	editor->moveautomationstartx=editor->pattern->GetMouseX();
	editor->moveautomationstarty=editor->pattern->GetMouseY();
	editor->automationtrackeditinit=mainedit->InitChangeAutomation(automationtrack);
}

void Edit_Arrange_AutomationTrack::CheckParameterMouse(AutomationParameter *ap)
{
	if(ap)
	{
		ap->Select();
		InitAutomationMouseMove();
	}
}

void Edit_Arrange_AutomationTrack::MouseClick(int px,int py,bool leftmouse)
{
	if(automationtrack->bindtoautomationobject==0 || automationtrack->bindtoautomationobject->GetAutoObject()==0)
		return;

	editor->InitMousePosition();

	AutomationParameter *ap=FindParameterMouse(px,py);

	if(leftmouse==true)
	{
		editor->UnSelectAllInPattern(automationtrack,ap);	
	}
	else
	{
		if(!ap)
		{
			if(editor->DeletePopUpMenu(true))
			{
				//popmenu->AddFMenu(0,new menu_cnewtrack(WindowSong(),WindowSong()->LastParentTrack()));
				editor->AddSetSongPositionMenu(0,0);
				editor->ShowPopMenu();
			}

			return;
		}
	}

	switch(editor->mousemode)
	{
	case EM_SELECT:
		if(leftmouse==true)
		{
			if(ap && ap->IsSelected()==false)
			{
				ap->Select();

				if(maingui->GetShiftKey()==true)
					editor->SetMouseMode(EM_SELECTOBJECTS,automationtrack);
				else
				{
					editor->SetMouseMode(EM_MOVEAUTOMATION,automationtrack);
					InitAutomationMouseMove();
				}

				return;
			}

			if(maingui->GetShiftKey()==true)
			{
				editor->SetMouseMode(EM_SELECTOBJECTS,automationtrack);
				return;
			}

			if(ap && ap->IsSelected()==true)
			{
				editor->SetMouseMode(EM_MOVEAUTOMATION,automationtrack);
				InitAutomationMouseMove();
			}
			else
				if(!ap)
				{
					editor->SetMouseMode(EM_SELECTOBJECTS,automationtrack);
				}
		}
		else
		{
			if(ap && ap->IsSelected()==true)
			{
				mainedit->ResetAutomationParameter(editor->WindowSong(),automationtrack,ap);
			}
		}
		break;

	case EM_CREATE:
		{
			if(leftmouse==true)
			{
				if(ap)
					CheckParameterMouse(ap);
				else
				{
					double h=y2-y;
					double h2=y2-py;

					h2/=h;

					TRACE ("LM %f\n",h2);

					automationtrack->bindtoautomationobject->GetAutoObject()->AutomationEdit(editor->WindowSong(),editor->GetMousePosition(),automationtrack->bindtoautomationobject_parindex,h2,AEF_FORCE|AEF_USEREDIT|AEF_UNDO);
				}
			}
			else
			{
				if(ap)
					mainedit->DeleteAutomationParameter(editor->WindowSong(),automationtrack,ap);
			}
		}
		break;

	case EM_DELETE:
		{
			if(ap)
			{
				if(ap->IsSelected()==false)
				{
					ap->Select();

					if(maingui->GetShiftKey()==true)
						editor->SetMouseMode(EM_SELECTOBJECTS,automationtrack);
				}

				if(maingui->GetShiftKey()==true)
				{
					editor->SetMouseMode(EM_SELECTOBJECTS,automationtrack);
					return;
				}

				if(ap->IsSelected()==true)
					mainedit->DeleteAutomationParameter(editor->WindowSong(),automationtrack,0);

				return;
			}
			else
			{
				editor->SetMouseMode(EM_SELECTOBJECTS,automationtrack);
			}
		}
		break;

	default:
		if(leftmouse==true)
			CheckParameterMouse(ap);
		break;
	}
}

bool Edit_Arrange_AutomationTrackParameter::IsUnderMouse(int mx,int my)
{
	if(mx>=x && mx<=x2 && my>=y && my<=y2)
		return true;

	return false;
}

bool Edit_Arrange_AutomationTrackParameter::CheckObjectinRange(int cx,int cy,int cx2,int cy2)
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

	if(x<cx2 && y<cy2 &&x2>cx && y2>cy)
		return true;

	return false;
}


#ifdef OLDIE
void Edit_Arrange_AutomationTrack::ShowAutomationObjects()
{


	DeleteObjects();
	bool drawrect;

	switch(automationtrack->tracktype)
	{
	case AutomationTrack::SBTYPE_ABS:
		drawrect=true;
		break;

	case AutomationTrack::SBTYPE_LINEAR:
		drawrect=false;
		break;
	}

	if(automationtrack->visible==true)
	{
		switch(automationtrack->tracktype)
		{
		case AutomationTrack::SBTYPE_LINEAR:
			{
#ifdef OLDIE
				for(int x=editor->timeline->x;x<=editor->timeline->x2;x++)
				{
					OSTART p=editor->timeline->ConvertXPosToTime(x);

					//TRACE ("SB TV %f\n",automationtrack->GetSubTrackValue(p));

					int aoy=automationtrack->ConvertValueToY(y,y2,automationtrack->GetSubTrackValue(p));

					editor->guibuffer->guiDrawPixel(x,aoy);
					editor->guibuffer->guiDrawLine(x,aoy+1,x,y2,COLOUR_SUBTRACK_BACKGROUND);
				}
#endif

			}
			break;
		}

		if(automationtrack->id==OBJ_AUDIOAUTOMATIONTRACK)
		{
			// Init Pattern List
			Seq_Pattern *rp=automationtrack->track->FirstPattern(MEDIATYPE_AUDIO);		
			while(rp)
			{
				if(automationtrack->ObjectCheck(rp)==true)
				{
					OSTART start=automationtrack->track->song->timetrack.ConvertTicksToMeasureTicks(rp->GetPatternStart(),false);

					if(start>editor->endposition)
						break;

					OSTART end=automationtrack->track->song->timetrack.ConvertTicksToMeasureTicks(rp->GetPatternEnd(),true);

					if(start>=editor->startposition || end>editor->startposition){	

						Edit_Arrange_Pattern newp;

						newp.editor=editor;

						newp.accesscounter=rp->GetAccessCounter();

						newp.start=rp->GetPatternStart();
						newp.end=rp->GetPatternEnd();
						newp.numberofevents=rp->GetCountOfEvents();

						//newp.qstart=start;
						//newp.qend=end;
						newp.pattern=rp;
						newp.subpattern=true; // set subpattern FLAG

						//	newp.SetToFrame(x,y,x2,y2,&editor->frame_pattern);

						editor->CalcPatternPosition(&newp);

						newp.sy=y;
						newp.sy2=y2;

						newp.ShowPattern();
					}

					drawrect=false;
				}

				rp=rp->NextPattern(MEDIATYPE_AUDIO);
			}// while rp

		}//if audio

		int ly=-1;
		AutomationObject *ao=automationtrack->FindAutomationParameter(editor->startposition);

		if(AutomationObject *pao=ao?ao->PrevAutomationObject():automationtrack->LastObject())
		{
			int ex=(ao && ao->GetParameterStart()<editor->endposition)?editor->timeline->ConvertTimeToX(ao->GetParameterStart()):editor->timeline->x2;

			int hy=automationtrack->ConvertValueToY(y,y2,pao->value);

			if(drawrect==true)
			{
				editor->guibuffer->guiFillRect(editor->timeline->x,hy+1,ex,y2,COLOUR_SUBTRACK_BACKGROUND);
				editor->guibuffer->guiDrawLine(editor->timeline->x,hy,ex,hy,COLOUR_GREY);
			}

			char hstring[255];
			automationtrack->GetObjectString(hstring,pao);

			if(strlen(hstring)>0)
				editor->guibuffer->guiDrawText(editor->timeline->x+1,hy+maingui->GetFontSizeY(),editor->timeline->x2,hstring);

			//pao=0;
		}

		int lx,py=-1;

		while(ao && ao->GetParameterStart()<=editor->endposition)
		{
			int ex=editor->timeline->ConvertTimeToX(ao->GetParameterStart()),aoy=automationtrack->ConvertValueToY(y,y2,ao->value);

			if(py>=0){

				if(drawrect==true)
				{
					editor->guibuffer->guiFillRect(lx,py+1,ex,y2,COLOUR_SUBTRACK_BACKGROUND);
					editor->guibuffer->guiDrawLine(lx,py,ex,py,COLOUR_GREY);
				}

				// editor->guibuffer->guiDrawLine(lx,py,x,py,COLOUR_GREY);
			}

			lx=ex+1;
			py=aoy;

			int ao_x=ex,ao_y=aoy,ao_x2=ex+10,ao_y2=aoy+10;

			if(ao_x2>editor->pattern->x2){
				ao_x=editor->pattern->x2;
				ao_x2-=10;
			}

			if(ao_y2>y2){
				ao_y=y2-10;
				ao_y2=y2;
			}

			if(ao_x<ao_x2 &&ao_y<ao_y2){

				if(Edit_Arrange_AutomationTrackParameter *newob=new Edit_Arrange_AutomationTrackParameter){
					newob->x=ao_x;
					newob->y=ao_y;
					newob->x2=ao_x2;
					newob->y2=ao_y2;
					newob->object=ao;
					newob->editor=editor;
					newob->eas=this;
					newob->aoy=aoy;

					AddObject(newob);
				}
			}

			ao=ao->NextAutomationObject();

			if(drawrect==true && 
				((!ao) || (ao->GetParameterStart()>editor->endposition))
				)
			{
				editor->guibuffer->guiFillRect(lx,aoy+1,editor->timeline->x2,y2,COLOUR_SUBTRACK_BACKGROUND);
				editor->guibuffer->guiDrawLine(lx,aoy,editor->timeline->x2,aoy,COLOUR_GREY);
			}
		}

		/*
		if(py==-1){

		if(pao && pao->NextAutomationObject())
		{
		py=automationtrack->ConvertValueToY(y,y2,pao->GetValue());

		if(drawrect==true)
		editor->guibuffer->guiFillRect(x,py,x2,y2,COLOUR_SUBTRACK_BACKGROUND);

		editor->guibuffer->guiDrawLine(x,py,x2,py,COLOUR_GREY);

		char hstring[255];
		automationtrack->GetObjectString(hstring,pao);
		editor->guibuffer->guiDrawText(x+1,py,x2,hstring);
		}
		}
		*/

		Edit_Arrange_AutomationTrackParameter *fo=FirstObject();
		while(fo){
			fo->Draw(false);
			fo=fo->NextAutomationTrackObject();
		}
	}
}
#endif

void Edit_Arrange_Channel::ShowAutomationSettings()
{
	automationsettings.onoff=channel->channel->automationon;
	automationsettings.used=channel->channel->HasAutomation();

	if(automationsettings.ondisplay==false)
		return;

	bitmap->ShowAutomationSettings(automationsettings.x,automationsettings.y,automationsettings.x2,automationsettings.y2,channel->channel);
}

void Edit_Arrange_Channel::ShowAutomationMode()
{
	if(automation.ondisplay==false)
		return;

	bitmap->ShowAutomationMode(automation.x,automation.y,automation.x2,automation.y2,channel->channel);
}

void Edit_Arrange_Channel::ShowMIDIVolume()
{
	if(MIDIvolume.ondisplay==false)
		return;

	MIDIvolume.MIDIvelocity=channel->MIDIfx.velocity.GetVelocity();
	m_volumeclicked=channel->m_volumeclicked;

	bitmap->guiFillRect(MIDIvolume.x,MIDIvolume.y,MIDIvolume.x2,MIDIvolume.y2,m_volumeclicked==true?COLOUR_BLACK:COLOUR_AUDIOINVOLUME_BACKGROUND);
	bitmap->SetFont(&maingui->smallfont);

	int mx=bitmap->GetTextWidth("*Velocity*");
	int sx=MIDIvolume.x2-1-mx;

	bitmap->SetFont(&maingui->standardfont);
	bitmap->SetTextColour(COLOUR_YELLOW);

	char help[NUMBERSTRINGLEN];

	if(char *h=mainvar->ConvertIntToChar(MIDIvolume.MIDIvelocity,help))
	{
		int sx2=MIDIvolume.x+2+bitmap->GetTextWidth(h);
		bitmap->guiDrawText(MIDIvolume.x+2,MIDIvolume.y2-5,MIDIvolume.x2-1,h);

		if(sx2+1<sx)
			bitmap->guiDrawText(sx,MIDIvolume.y2-5,MIDIvolume.x2-1,"Velocity");
	}

	int lx=MIDIvolume.x+1;
	int lx2=MIDIvolume.x2-1;
	int ly=MIDIvolume.y2-4;
	int ly2=MIDIvolume.y2-1;

	double hw=lx2-lx;
	double ph=255;
	double hh=127-MIDIvolume.MIDIvelocity;

	hh/=ph;
	hw*=hh;

	int tx2=lx2-(int)hw;

	if(tx2>lx)
		bitmap->guiFillRect(lx,ly,tx2,ly2,COLOUR_YELLOW);

	if(tx2+1<lx2)
		bitmap->guiFillRect(tx2+1,ly,lx2,ly2,COLOUR_BLACK);
}

void Edit_Arrange_Channel::ShowVolume(bool force)
{
	if(volume.ondisplay==false)
		return;

	volumeeditable=channel->CanAutomationObjectBeChanged(&channel->io.audioeffects.volume,0,0);
	volume.volume=channel->io.audioeffects.volume.value;
	volumeclicked=channel->volumeclicked;

	{
		int col;

		if(volumeeditable==false)
			col=COLOUR_AUTOMATIONTRACKSUSED;
		else
			col=volumeclicked==true?COLOUR_BLACK:COLOUR_AUDIOINVOLUME_BACKGROUND;

		bitmap->guiFillRect(volume.x,volume.y,volume.x2,volume.y2,col);
	}

	int dbv=bitmap->SetAudioColour(volume.volume);

	if(char *h=mainaudio->ScaleAndGenerateDBString(volume.volume,false))
	{
		guiFont *old=bitmap->SetFont(&maingui->standardfont);
		bitmap->guiDrawText(volume.x+2,volume.y2-5,volume.x2-1,h);
		bitmap->SetFont(old);
		delete h;
	}

	int lx=volume.x+1;
	int lx2=volume.x2-1;
	int ly=volume.y2-4;
	int ly2=volume.y2-1;
	int pos=LOGVOLUME_SIZE-mainaudio->ConvertLogArrayVolumeToInt(volume.volume);

	double hw=lx2-lx;
	double ph=LOGVOLUME_SIZE;
	double hh=pos;

	hh/=ph;
	hw*=hh;

	int tx2=lx2-(int)hw;

	{
	int rgb;

	if(dbv==1) // 1 >
		rgb=RGB(255,150,150);
	else
		if(dbv==0) // 1
			rgb=RGB(100,230,255);
		else // 1<
			rgb=RGB(244,244,255);

	if(tx2>lx)
		bitmap->guiFillRect_OSRGB(lx,ly,tx2,ly2,rgb);
	}

	if(tx2+1<lx2)
		bitmap->guiFillRect(tx2+1,ly,lx2,ly2,COLOUR_BLACK_LIGHT);
}

void Edit_Arrange_Channel::ShowMIDIDisplay(bool force)
{
}

void Edit_Arrange_Channel::ShowAudioDisplay(bool force)
{
	if(datadisplay==false)
		return;

	bool forceblit=false;

	outpeak.showactivated=false;
	outpeak.force=force;

	int	flag=0,audiox,hmx=(dataoutputx2-dataoutputx)/2;

	audiox=dataoutputx+hmx+1;

	outpeak.channel=channel;
	outpeak.track=0;

	outpeak.x=audiox;
	outpeak.x2=dataoutputx2;
	outpeak.y=y+1;
	outpeak.y2=y2-1;

	outpeak.active=mainvar->GetActiveSong()==editor->WindowSong()?true:false;
	outpeak.bitmap=&editor->tracks->gbitmap;

	outpeak.ShowInit(true);
	outpeak.ShowPeak();

	if(forceblit==true || (force==false && outpeak.changed==true))
	{
		editor->tracks->Blt(audiox,y+1,dataoutputx2,y2-1);
	}
}

void Edit_Arrange_Channel::ShowChannelChannelType()
{
	channeltype.channeltype=channel->io.channel_type;

	if(channeltype.ondisplay==false)
		return;

	bitmap->SetFont(channel->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->guiFillRect(channeltype.x,channeltype.y,channeltype.x2,channeltype.y2,COLOUR_AUDIOTYPE_BACKGROUND);
	bitmap->SetTextColour(COLOUR_TEXT);
	bitmap->guiDrawText(channeltype.x+1,channeltype.y2-1,channeltype.x2,channelchannelsinfo_short[channeltype.channeltype]);
}

void Edit_Arrange_Channel::ShowChannelNumber()
{
	if(channel->channel->audiochannelsystemtype!=CHANNELTYPE_BUSCHANNEL)
		return;

	bitmap->guiDrawNumber(1,y+maingui->GetButtonSizeY(),startnamex-1,channel->channel->GetChannelIndex()+1);
}

void Edit_Arrange_Channel::ShowChannelInfo()
{
	if(info.ondisplay==false)
		return;

	hasinstruments=channel->io.audioeffects.CheckIfEffectHasOnInstruments();
	hasfx=channel->io.audioeffects.CheckIfEffectHasNonInstrumentFX();

	bitmap->SetFont(channel->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);

	char *f=0;
	int col=COLOUR_GREY;

	bitmap->SetTextColour(COLOUR_BLACK);

	if(hasinstruments==true)
	{
		if(hasfx==true)
			f="iX";
		else
			f="i";
	}
	else
	{
		if(hasfx==true)
			f="X";
	}

	if(f)
	{
		bitmap->guiFillRect(info.x,info.y,info.x2,info.y2,col,COLOUR_INFOBORDER);
		bitmap->guiDrawTextCenter(info.x+2,info.y,info.x2,info.y2,f);
	}
	else
		bitmap->guiFillRect(info.x,info.y,info.x2,info.y2,bgcolour);

}

Edit_Arrange_Channel::Edit_Arrange_Channel()
{
	id=OBJECTID_ARRANGECHANNEL;

	info.channel=this;
	info.deleteable=false;

	name.channel=this;
	name.deleteable=false; // part of

	mute.channel=this;
	mute.deleteable=false; // part of

	solo.channel=this;
	solo.deleteable=false; // part of

	volume.channel=this;
	volume.deleteable=false; // part of

	automationsettings.channel=this;
	automationsettings.deleteable=false;

	automation.channel=this;
	automation.deleteable=false; // part of

	MIDIvolume.channel=this;
	MIDIvolume.deleteable=false;

	channeltype.channel=this;
	channeltype.deleteable=false;

	namestring=0;
}


void Edit_Arrange_Channel::ShowChannelRaster()
{
	channelselected=channel->IsSelected();

	switch(channel->audiochannelsystemtype)
	{
	case CHANNELTYPE_MASTER:
		bgcolour=COLOUR_MASTERCHANNEL;
		break;

	case CHANNELTYPE_BUSCHANNEL:
		bgcolour=channel->IsSelected()==true?COLOUR_BUSCHANNELSELECTED:COLOUR_BUSCHANNEL;
		break;

	default:
		bgcolour=COLOUR_BACKGROUND;
		break;
	}

	int col,col2;

	col=COLOUR_WHITE;
	col2=COLOUR_GREY_DARK;

	bitmap->guiFillRectX0(y,x2,y2,bgcolour);

	bitmap->guiDrawLineX(0,y,y2,col);
	bitmap->guiDrawLineX(startnamex-1,y,y2,col);

	bitmap->guiDrawLineYX0(y,x2,col);
	bitmap->guiDrawLineYX0(y2,x2,col2);
}

void Edit_Arrange_Channel::ShowMute()
{
	bitmap->ShowMute(&mute,mute.status=channel->io.audioeffects.mute.mute,bgcolour);
}

void Edit_Arrange_Channel::ShowSolo()
{
	if(solo.ondisplay==true)
	{
		bitmap->ShowSolo(&solo,solo.status=channel->GetSoloStatus(),bgcolour);
	}
}

void Edit_Arrange_Channel::ShowChannel(bool refreshbgcolour)
{
	ShowChannelRaster();
	ShowChannelNumber();

	ShowVolume(true);
	ShowMIDIVolume();

	ShowAudioDisplay(true);
	ShowMIDIDisplay(true);
	ShowMute();
	ShowSolo();
	ShowAutomationSettings();
	ShowAutomationMode();
	ShowChannelChannelType();

	if(namestring)delete namestring;
	namestring=0;

	// Name
	if(name.ondisplay==true)
	{
		namestring=mainvar->GenerateString(channel->GetName());

		bitmap->SetFont(/*track->IsSelected()==true?&maingui->standard_bold:*/&maingui->standardfont);
		bitmap->SetTextColour(COLOUR_BLACK);
		//bitmap->guiFillRect(name.x,name.y,name.x2,name.y2,COLOUR_GREY_DARK,COLOUR_BLACK);
		bitmap->guiDrawText(name.x+2,name.y2,name.x2,channel->GetName());
	}

	ShowChannelInfo();
}

Edit_Arrange_AutomationTrack::Edit_Arrange_AutomationTrack()
{
	id=OBJECTID_ARRANGEAUTOMATIONTRACK;

	name.track=this;
	name.deleteable=false; // part of

	vu.track=this;
	vu.deleteable=false; // part of

	mode.track=this;
	mode.deleteable=false; // part of

	info.track=this;
	info.deleteable=false; // part of

	//pattern.track=this;
	//pattern.deleteable=false;

	//visible.track=this;
	//visible.deleteable=false;

	createticks.track=this;
	createticks.deleteable=false;

	value.track=this;
	value.deleteable=false;

	mousevalue.track=this;
	mousevalue.deleteable=false;

	visible.track=this;
	visible.deleteable=false;

	fgcolour=COLOUR_AUTOMATIONTRACKS;
}

void Edit_Arrange_AutomationTrack::ShowChannelRaster()
{
	AudioChannel *channel=automationtrack->audiochannel;

	int bgcolour;

	switch(channel->audiochannelsystemtype)
	{
	case CHANNELTYPE_BUSCHANNEL:
		bgcolour=COLOUR_BUSCHANNEL;
		break;

	case CHANNELTYPE_MASTER:
		bgcolour=COLOUR_MASTERCHANNEL;
		break;
	}

	bitmap->guiFillRect(startnamex,y,x2,y2,bgcolour);
	bitmap->guiFillRectX0(y,startnamex,y2,bgcolour);
	ShowVisible();
	bitmap->guiDrawLineYX0(y,x2,COLOUR_WHITE);
	bitmap->guiDrawLineYX0(y2,x2,COLOUR_GREY_DARK);
}

void Edit_Arrange_AutomationTrack::ShowTrackRaster()
{
	Seq_Track *track=automationtrack->track;

	int bgcolour;

	if(track->IsSelected()==true)
		bgcolour=COLOUR_BACKGROUNDAUTOMATION_HIGHLITE;
	else
		bgcolour=COLOUR_BACKGROUNDAUTOMATION;

	// Colour
	int col,col2;

	if(track==track->song->GetFocusTrack())
	{
		col=col2=track->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT;
	}
	else
	{
		col=COLOUR_WHITE;
		col2=COLOUR_GREY_DARK;
	}

	bool rgb=false;
	int rgbcol;

	if(track->GetColour()->showcolour==true)
	{
		rgbcol=track->GetColour()->rgb;
		rgb=true;
		//bitmap->guiFillRect_RGB(x+1,y+1,dx,y2-1,);
	}
	else
	{
		// Group Colour
		Seq_Group_GroupPointer *p=track->GetGroups()->FirstGroup();
		while(p)
		{
			if(p->underdeconstruction==false && p->group->colour.showcolour==true){
				rgbcol=p->group->colour.rgb;
				rgb=true;

				//		bitmap->guiFillRect_RGB(x+1,y+1,dx,y2-1,p->group->colour.rgb);
				break;
			}

			p=p->NextGroup();
		}
	}

	// Track Numer
	if(rgb==true)
		bitmap->guiFillRect_RGB(0,y,startnamex,y2,rgbcol);
	else
		bitmap->guiFillRectX0(y,startnamex,y2,bgcolour);

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->SetTextColour(COLOUR_BLACK);

	ShowVisible();

	bitmap->guiFillRect(startnamex,y,x2,y2,bgcolour);

	bitmap->guiDrawLineX(0,y,y2,col);
	bitmap->guiDrawLineX(startnamex-1,y,y2,col);

	bitmap->guiDrawLineYX0(y,x2,col);
	bitmap->guiDrawLineYX0(y2,x2,col2);
}

void Edit_Arrange_AutomationTrack::ShowVisible()
{
	if(visible.ondisplay==true)
	{
		bitmap->ShowAutomationTrackVisible(visible.x,visible.y,visible.x2,visible.y2,automationtrack);
	}
}

void Edit_Arrange_AutomationTrack::ShowVU()
{
	// VU
	if(vu.ondisplay==true)
	{
		bitmap->SetFont(selected==true?&maingui->standard_bold:&maingui->standardfont);
		bitmap->SetTextColour(fgcolour);
		bitmap->guiFillRect(vu.x,vu.y,vu.x2,vu.y2,COLOUR_GREY_DARK,COLOUR_BLACK);

		if(automationtrack->bindtoautomationobject)
		{
			double h=automationtrack->bindtoautomationobject->GetParm(automationtrack->bindtoautomationobject_parindex);
			double h2=(vu.y2-vu.y)-2;

			h2*=h;

			int my=vu.y2-(int)h2;
			bitmap->guiFillRect(vu.x+1,my,vu.x2-1,vu.y2-1,fgcolour);
		}
	}
}

void Edit_Arrange_AutomationTrack::ShowValue()
{	
	if(value.ondisplay==true)
	{
		if(automationtrack->bindtoautomationobject)
		{
			parmvalue=automationtrack->bindtoautomationobject->GetAutoObject()->GetParm(automationtrack->bindtoautomationobject_parindex);

			bitmap->SetFont(selected==true?&maingui->standard_bold:&maingui->standardfont);
			bitmap->SetTextColour(fgcolour);
			bitmap->guiFillRect(value.x,value.y,value.x2,value.y2,COLOUR_GREY_DARK,COLOUR_BLACK);
			bitmap->guiDrawText(value.x+2,value.y2,value.x2,automationtrack->bindtoautomationobject->GetAutoObject()->GetParmValueString(automationtrack->bindtoautomationobject_parindex));
		}
	}
}

void Edit_Arrange_AutomationTrack::ShowMode()
{
	mode.mode=automationtrack->automode;

	if(mode.ondisplay==true)
	{
		bitmap->SetFont(selected==true?&maingui->standard_bold:&maingui->standardfont);
		bitmap->SetTextColour(fgcolour);
		bitmap->guiFillRect(mode.x,mode.y,mode.x2,mode.y2,COLOUR_GREY_DARK,COLOUR_BLACK);

		bitmap->guiDrawText(mode.x+2,mode.y2,mode.x2-2,automationtrack_mode_names[mode.mode]);
	}
}

void Edit_Arrange_AutomationTrack::ShowTrack(bool refreshbgcolour)
{
	if(AudioChannel *ac=automationtrack->audiochannel)
	{
		selected=ac->IsSelected();

		if(refreshbgcolour==true)
		{
			ShowChannelRaster();
		}
	}

	if(Seq_Track *track=automationtrack->track)
	{
		selected=track->IsSelected();

		if(refreshbgcolour==true)
		{
			ShowTrackRaster();
		}
	}

	// Name
	if(name.ondisplay==true)
	{
		bitmap->SetFont(selected==true?&maingui->standard_bold:&maingui->standardfont);
		bitmap->SetTextColour(fgcolour);
		bitmap->guiFillRect(name.x,name.y,name.x2,name.y2,COLOUR_GREY_DARK,COLOUR_BLACK);
		bitmap->guiDrawText(name.x+2,name.y2,name.x2,automationtrack->GetAutomationTrackName());
	}


	ShowValue();
	ShowVU();
	ShowMode();

}

void Edit_Arrange_Track::ShowAutomationMode()
{
	if(automation.ondisplay==false)
		return;

	bitmap->ShowAutomationMode(automation.x,automation.y,automation.x2,automation.y2,track);
}

void Edit_Arrange_Track::ShowAutomationSettings()
{
	automationsettings.onoff=track->automationon;
	automationsettings.used=track->HasAutomation();

	if(automationsettings.ondisplay==false)
		return;

	bitmap->ShowAutomationSettings(automationsettings.x,automationsettings.y,automationsettings.x2,automationsettings.y2,track);
}

void Edit_Arrange_Track::ShowChildTrackMode()
{
	if(track->FirstChildTrack())
	{
		bitmap->ShowChildTrackMode(&child,track,true);
	}
}