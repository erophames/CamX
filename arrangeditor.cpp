#include "songmain.h"
#include "editor.h"
#include "camxgadgets.h"
#include "editbuffer.h"
#include "imagesdefines.h"
#include "arrangeeditor.h"
#include "arrangeeditor_fx.h"
#include "arrangeeditorlist.h"
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
#include "MIDIoutproc.h"
#include "audiothread.h"
#include "audiochannel.h"
#include "audiomanager.h"
#include "arrangeeditor_menus.h"
#include "MIDIPattern.h"
#include "drumevent.h"
#include "languagefiles.h"
#include "sampleeditor.h"
#include "object_project.h"
#include "editdata.h"
#include "audiopattern.h"
#include "semapores.h"
#include "undo_automation.h"
#include "audioports.h"
#include "audiohdfile.h"
#include "audiofilework.h"

extern char *channelchannelsinfo_short[];

#define SIZEV_TEMPO (2*maingui->GetButtonSizeY())
#define SIZEV_SIGNATURE maingui->GetButtonSizeY()

#define TAB_TRACKLIST 0
#define TAB_TRACKINFO 1

enum
{
	GADGETID_EFFECTS=GADGETID_EDITORBASE,
	GADGETID_LIST,
	GADGETID_TNAME,

	GADGETID_SHOWMASTER,
	GADGETID_SHOWBUS,
	GADGETID_SHOWMETRO,

	GADGETID_FOCUSPATTERN_TIME_I,
	GADGETID_FOCUSPATTERN_END_I,

	GADGETID_TPOS,
	GADGETID_TEND,

	GADGETID_EVENTEDITOR,
	GADGETID_PIANOEDITOR,
	GADGETID_DRUMEDITOR,
	GADGETID_SAMPLEEDITOR,

	GADGETID_MUTE,
	GADGETID_SOLO,
	GADGETID_NEWTRACK,
	GADGETID_NEWCHILD,

	GADGETID_FILTERTRACKS,
	GADGETID_RESETTRACKZOOM,
	GADGETID_AUDIO,
	GADGETID_MIDI,
	GADGETID_TYPE,
	GADGETID_INPUT,
	GADGETID_AUTOMATION,
	GADGETID_VOLUMECURVE,
	GADGETID_MOVEBUTTON,
	GADGETID_SET1,
	GADGETID_SET2,
	GADGETID_SET3,
	GADGETID_SET4,
	GADGETID_SET5
};

enum{
	EDIT_NAME=100,
	EDIT_CHANNELNAME,

	MESSAGE_PATTERN,
};

class menu_gotoarrange:public guiMenu
{
public:
	menu_gotoarrange(Edit_Arrange *ar,int t)
	{
		editor=ar;
		gototype=t;
	}

	void MenuFunction()
	{		
		editor->Goto(gototype);
	} //

	Edit_Arrange *editor;
	int gototype;
};

void Edit_Arrange::ToggleUseColour(Seq_Pattern *pattern)
{
	pattern->t_colour.showcolour=pattern->t_colour.showcolour==true?false:true;
	maingui->RefreshColour(pattern);

	GetPatternSelection();
	Seq_SelectedPattern *p=selection.FirstSelectedPattern();

	while(p)
	{		
		if(p->pattern!=pattern)
		{
			p->pattern->t_colour.showcolour=pattern->t_colour.showcolour;
			maingui->RefreshColour(p->pattern);
		}

		p=p->NextSelectedPattern();
	}

	maingui->ClearRefresh();
}

void Edit_Arrange::SetPatternColour(Seq_Pattern *pattern)
{
	// Pattern Colour
	colourReq req;

	req.OpenRequester(this,&pattern->t_colour);

	if(pattern)
	{
		if(pattern->t_colour.changed==true)
		{
			pattern->t_colour.showcolour=true;
			pattern->t_colour.changed=false;
			maingui->RefreshColour(pattern);
		}
	}

	GetPatternSelection();

	Seq_SelectedPattern *p=selection.FirstSelectedPattern();

	while(p)
	{		
		if(p->pattern->t_colour.Compare(&pattern->t_colour)==false)
		{
			pattern->t_colour.Clone(&p->pattern->t_colour);

			p->pattern->t_colour.showcolour=true;
			p->pattern->t_colour.changed=false;									

			maingui->RefreshColour(p->pattern);
		}

		p=p->NextSelectedPattern();
	}

	maingui->ClearRefresh();
}

bool Edit_Arrange::SelectPatternCheckKeys(Seq_Pattern *pattern,bool select,bool draw,bool toggleselection)
{
	// unselect all Pattern
	if(maingui->GetCtrlKey()==true)toggleselection=true;
	return SelectPattern(pattern,select,draw,toggleselection);
}

bool Edit_Arrange::SelectPattern(Seq_Pattern *pattern,bool select,bool draw,bool toggleselection)
{
	if(pattern->mediatype==MEDIATYPE_AUDIO_RECORD || pattern->recordpattern==true)
		return false;

	bool changed=false;

	if(toggleselection==true && pattern->IsSelected()==true)
		select=false;

	if((pattern->mainpattern==0 || pattern->mainclonepattern) && // Dont select loop pattern, clone is ok
		pattern->visible==true // Freeze ?
		) 
	{
		bool done=false;

		switch(pattern->mediatype)
		{
		case MEDIATYPE_AUDIO:
		case MEDIATYPE_MIDI:
			{
				if(select==false)
				{
					if(pattern->IsSelected()==true)
					{
						pattern->UnSelect();
						changed=true;
					}
				}
				else
				{
					if(pattern->IsSelected()==false)
					{
						pattern->Select();
						changed=true;
					}
				}
			}
			break;
		}// switch

		//if(done==true)
		//	ShowPatternList();
	}

	return changed;
}

bool Edit_Arrange_Pattern::CheckIfRegionIsInside(AudioRegion *r)
{
	AudioGFX_Region *ar=(AudioGFX_Region *)regions.GetRoot();

	while(ar){
		if(ar->region==r)return true;
		ar=ar->Next();
	}
	return false;
}

void Edit_Arrange_Pattern::ShowInsidePattern()
{
	guiObject *ot=editor->guiobjects.FirstObject();

	// create EditArrangePatternLists
	while(ot)
	{
		// Pattern Track
		switch(ot->id)
		{
		case OBJECTID_ARRANGEPATTERN:
			{
				Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)ot;

				if(eap!=this && eap->pattern->track==pattern->track )
				{
					if(eap->pattern->GetPatternStart()>=pattern->GetPatternStart() && eap->pattern->GetPatternStart()<=pattern->GetPatternEnd()) 
					{
						eap->ShowPattern_Blt();
					}
				}
			}
			break;
		}

		ot=ot->NextObject();
	}// while ot
}

void Edit_Arrange_Pattern::ShowPattern_Blt()
{
	ShowPattern();
	editor->pattern->Blt(this);
	//editor->BltGUIBuffer(editor->frame_pattern.SetToX(x),editor->frame_pattern.SetToY(y),editor->frame_pattern.SetToX(x2),editor->frame_pattern.SetToY(y2));
}

Edit_Arrange_Pattern::Edit_Arrange_Pattern()
{
	id=OBJECTID_ARRANGEPATTERN;
	subpattern=false;
	blink=false;
	//	refreshonlyblink=false;
	showpeakprogress=0;
	//offset=0;
	nonreal=false;
	undermove=false;
	track=0;
	notext=false;
	sx=-1;
	patternname=0;
	automationpattern=false;
	patterncurve=0;
}

void Edit_Arrange_Pattern::DeInit() //v
{
	if(patternname)
		delete patternname;

	regions.DeleteAllO();
	//crossfades.DeleteAllO();
}


void Edit_Arrange_Pattern::ShowPatternRaster()
{
	if(notext==true)
		sy=y+1;
	else
		sy=y+maingui->GetFontSizeY()+3;

	sy2=y2-1;

	if(notext==false)
	{
		if(sy+10>=sy2)
		{
			notext=true;
			sy=y;
			sy2=y2;
		}
		else
		{
			if(folder==false)
				notext=false;
		}
	}

	// Header Rect

	selectflag=pattern->IsSelected();
	muteflag=pattern->CheckIfPlaybackIsAble();

	if(sx2==-1)
		return;

	if(undermove==true || folder==true)
		return;

	if(automationpattern==true)
	{
		bitmap->guiFillRect(sx,sy,sx2,sy2,COLOUR_AUTOMATIONPATTERNBACKGROUND);
	}
	else
	{
		Seq_Pattern *mainpattern=pattern->mainpattern?pattern->mainpattern:pattern;

		if(mainpattern->GetColour()->showcolour==true)
			bitmap->guiFillRect_RGB(sx,sy,sx2,sy2,mainpattern->GetColour()->rgb);
		else
		{
			int bodycolour;

			if(muteflag==false)
				bodycolour=COLOUR_BACKGROUNDPATTERN_MUTED;
			else
				switch(pattern->mediatype)
			{
				case MEDIATYPE_AUDIO_RECORD:
					bodycolour=COLOUR_AUDIORECORDHEADER;
					break;

				case MEDIATYPE_AUDIO:
					{
						if(pattern->itsaloop==true)
							bodycolour=COLOUR_AUDIO_BACKGROUND_LOOP;
						else
							bodycolour=selectflag==true?COLOUR_AUDIO_BACKGROUND_SELECTED:COLOUR_AUDIO_BACKGROUND;
					}
					break;

				case MEDIATYPE_MIDI:
					if(pattern->itsaloop==true)
						bodycolour=COLOUR_MIDI_BACKGROUND_LOOP;
					else
						bodycolour=selectflag==true?COLOUR_MIDI_BACKGROUND_SELECTED:COLOUR_MIDI_BACKGROUND;
					break;

				default:
					bodycolour=COLOUR_WHITE;
					break;
			}

			bitmap->guiFillRect(sx,sy,sx2,sy2,bodycolour);
		}
	}
}

void Edit_Arrange_Pattern::ShowPattern()
{	
	int bitmapx2=bitmap->GetX2();

	focus=editor->WindowSong()->GetFocusPattern()==pattern?true:false;

	recordstatus=pattern->recordpattern;
	mediatype=pattern->mediatype;

	if(patterncurve)
	{
		volume=patterncurve->dbvolume;
		fadeinms=patterncurve->fadeinms;
		fadeoutms=patterncurve->fadeoutms;
		fadeactive=patterncurve->fadeinoutactive;
		volumeactive=patterncurve->volumeactive;
		fadeintype=patterncurve->fadeintype;
		fadeouttype=patterncurve->fadeouttype;
		fadeeditmode=patterncurve->editmode;
	}

	if(sx==-1)
		ShowPatternRaster(); // Init First

	pattern->GetColour()->Clone(&patterncolour);

	bool selected=false;

#ifdef DEBUG
	if(notext==false && folder==true)
		maingui->MessageBoxError(0,"Folder");
#endif

	if(notext==false && this->automationpattern==false)
	{
		// Header Rect
		int headercolour;

		switch(pattern->mediatype)
		{
		case MEDIATYPE_AUDIO_RECORD:
			headercolour=COLOUR_AUDIORECORDHEADER;
			break;

		case MEDIATYPE_AUDIO:
			{
				if(pattern->track->frozen==true)
					headercolour=COLOUR_FROZEN;
				else
					headercolour=COLOUR_AUDIOHEADER;

			}break;

		case MEDIATYPE_MIDI:
			headercolour=COLOUR_MIDI_BACKGROUND2;
			break;

		default:
			headercolour=COLOUR_WHITE;
			break;
		}

		bitmap->guiFillRect(sx,y,sx2,sy-1,headercolour);
	}

	if(sx2==-1)
		return;

	char *name=0;

	//if(refreshonlyblink==false)
	switch(pattern->mediatype)
	{
	case MEDIATYPE_AUDIO_RECORD:
	case MEDIATYPE_AUDIO:
		{
			editor->RemoveCrossFades(pattern);

			AudioPattern *af=(AudioPattern *)pattern;
			bool fill=true;

#ifdef OLDIE
			if(subpattern==false && undermove==false)
			{
				if(Seq_CrossFade *f=af->FirstCrossFade())
				{
					while(f && f->to>=editor->startposition && f->from<editor->endposition)
					{
						if(f->infade==true) // only outs
						{
							if(Edit_Arrange_Pattern_CrossFade *cf=new Edit_Arrange_Pattern_CrossFade)
							{
								cf->crossfade=f;
								cf->x=f->from>editor->startposition?editor->timeline->ConvertTimeToX(f->from):editor->timeline->x;

								cf->startinside=f->from>editor->startposition?true:false;

								cf->x2=f->to<editor->endposition?editor->timeline->ConvertTimeToX(f->to):editor->timeline->x2;
								cf->endinside=f->to<editor->endposition?true:false;
								cf->y=sy;

								cf->y2=sy2;

								editor->crossfades.AddEndO(cf);
							}
						}

						f=f->NextCrossFade();
					}
				}
			}
#endif

			if(fill==true)
			{
				if(af->audioevent.audioefile)
				{
					AudioGFX gfx;

					gfx.audiopattern=af;
					gfx.win=editor;
					gfx.bitmap=bitmap;
					gfx.usebitmap=true;
					gfx.timeline=editor->timeline;
					gfx.x2=sx2;
					gfx.y=sy;
					gfx.y2=sy2;
					gfx.samplezoom=editor->datazoom;

					OSTART pstart=start; //af->GetPatternStart();

					gfx.start=pstart<editor->startposition?editor->startposition:pstart;

					if(sy2-sy<af->audioevent.audioefile->channels*14)
						gfx.showmix=true;
					else
						gfx.showmix=false;

					gfx.eventstart=pstart;
					gfx.subpattern=subpattern;
					gfx.mouseselection=false;
					gfx.drawborder=false;
					gfx.undermove=undermove;

					gfx.x=editor->timeline->ConvertTimeToX(gfx.start,x2);

					if(automationpattern==true)
					{
						gfx.drawcolour=COLOUR_FRONT_NOTSELECTED;
					}
					else
					{
						if(editor->mousemode==EM_MOVEPATTERN)
							gfx.drawcolour=gfx.undermove==true?COLOUR_FRONT_SELECTED:COLOUR_FRONT_NOTSELECTED;
						else
							gfx.drawcolour=af->IsSelected()==true?COLOUR_FRONT_SELECTED:COLOUR_FRONT_NOTSELECTED;
					}

					// gfx.linecolour=COLOUR_WHITE;

					{
						if(pattern->mediatype==MEDIATYPE_AUDIO_RECORD)
						{
							AudioPattern *ap=(AudioPattern *)pattern;

							//if(pattern->track->song->status&Seq_Song::STATUS_SONGPLAYBACK_MIDI)
							gfx.drawcolour=COLOUR_AUDIORECORD;

							/*
							if(ap->audioevent.audioefile && ap->audioevent.audioefile->samplesperchannel==0 && sy+maingui->GetFontSizeY()<sy2)
							{
							if(editor->WindowSong()->project->checkaudiostartpeak==true)
							{
							if(char *h=mainaudio->GenerateDBString(editor->WindowSong()->project->checkaudiothreshold))
							{
							char *h2=mainvar->GenerateString(Cxs[CXS_NOSAMPLESRECORDED],"Threshold:",h);
							delete h;

							if(h2)
							{
							bitmap->guiDrawText(x,sy2-1,x2,h2);
							delete h2;
							}
							}
							}
							else
							bitmap->guiDrawText(x,sy2-1,x2,Cxs[CXS_NOSAMPLESRECORDED]);
							}
							*/
						}
						else
							if(subpattern)
								gfx.drawcolour=COLOUR_YELLOW_LIGHT;
					}

					// Audio GFX
					if(showpeakprogress==0 && af->audioevent.audioefile->peakbuffer){

						gfx.eventend=end; //pattern->GetPatternEnd()+offset;
						gfx.samplex2=gfx.eventend>editor->endposition?x2:editor->timeline->ConvertTimeToX(gfx.eventend,x2);
						gfx.showregionsinside=af->audioevent.audioefile->showregionsineditors;

						gfx.patternvolumecurve=patterncurve;

						if(editor->GetShowFlag()&SHOW_VOLUMECURVES)
						{
							if(pattern->track->frozen==true)
							{
								gfx.patternvolumepositions=0;
								gfx.showvolumecurve=false;
							}
							else
								gfx.patternvolumepositions=&patternvolumepositions;
						}
						else
						{
							gfx.patternvolumepositions=0;
							gfx.showvolumecurve=false;
						}

						if(af->itsaloop==true)
						{
							gfx.nonreal=true;
							gfx.drawcolour=COLOUR_NOREALPATTERN;
						}
						else
							gfx.nonreal=nonreal;

						// Delete Regions Objects
						gfx.regions.DeleteAllO();
						regions.DeleteAllO();
						af->audioevent.audioefile->ShowAudioFile(editor->WindowSong(),&gfx);

						if(gfx.regions.GetRoot())
							gfx.regions.MoveListToList(&regions); // Move GFX Regions->Edit_Arrange_Pattern
					}
				}
			}

			if(notext==false)
			{
				size_t i=0;

				switch(pattern->mediatype)
				{
				case MEDIATYPE_AUDIO_RECORD:
					i=strlen(Cxs[CXS_RECORD]);
					break;

				case MEDIATYPE_AUDIO:
					if(af->audioevent.audioefile)
					{
						if(af->audioevent.audioefile->camxrecorded==true)
							i=strlen(Cxs[CXS_RECORDED]);
						else{
							i=strlen(af->mainpattern?af->mainpattern->GetName():af->GetName());
							switch(mainsettings->arrangeeditorfiledisplay)
							{
							case 1:
								// File Name
								i+=strlen(af->audioevent.audioefile->GetName());
								break;

							case 2:
								// File Name+Path
								i+=strlen(af->audioevent.audioefile->GetFileName());
								break;
							}
						}
					}
					break;
				}

				i+=32; // Time

				if(mainsettings->arrangeeditorfiledisplay!=0)
				{
					if(af->audioevent.audioregion)
						i+=strlen(af->audioevent.audioregion->regionname);
				}

				i+=128+strlen(Cxs[CXS_PEAKPROGRESS]);

				if(name=new char[i])
				{
					if(!af->audioevent.audioefile){

						strcpy(name,af->waitforresample==true?"Working...":Cxs[CXS_NOFILE]);

						mainvar->AddString(name,":");
						mainvar->AddString(name,af->GetName());

						UBYTE r,g,b;
						maingui->colourtable.GetRGB(COLOUR_RED,&r,&g,&b);
						bitmap->SetTextColour(r,g,b);
					}
					else{

						char h2[NUMBERSTRINGLEN];

						switch(pattern->mediatype)
						{
						case MEDIATYPE_AUDIO_RECORD:
							{
								strcpy(name,Cxs[CXS_RECORD]);
								mainvar->AddString(name,"[:");
								mainvar->AddString(name,mainvar->ConvertIntToChar(af->audioevent.audioefile->channels,h2));
								mainvar->AddString(name,"]");
							}
							break;

						case MEDIATYPE_AUDIO:
							if(af->audioevent.audioefile)
							{
								switch(mainsettings->arrangeeditorfiledisplay)
								{
								case 1:
									{
										strcpy(name,pattern->itsaclone==true?"(C [:":"[:");
										mainvar->AddString(name,mainvar->ConvertIntToChar(af->audioevent.audioefile->channels,h2));
										mainvar->AddString(name,"] ");

										if(pattern->itsaclone==false && pattern->useoffsetregion==true)
										{
											if(pattern->IfOffSetStart() && pattern->IfOffSetEnd())
												mainvar->AddString(name,"|><| ");
											else
												if(pattern->IfOffSetStart())
													mainvar->AddString(name,"|> ");
												else
													if(pattern->IfOffSetEnd())
														mainvar->AddString(name,"<| ");
										}

										mainvar->AddString(name,af->audioevent.audioefile->camxrecorded==true?Cxs[CXS_RECORDED]:af->audioevent.audioefile->GetFileName());
									}
									break;

								case 2:
									{
										strcpy(name,pattern->itsaclone==true?"(C [:":"[:");

										mainvar->AddString(name,mainvar->ConvertIntToChar(af->audioevent.audioefile->channels,h2));
										mainvar->AddString(name,"] ");

										if(pattern->itsaclone==false && pattern->useoffsetregion==true)
										{
											if(pattern->IfOffSetStart() && pattern->IfOffSetEnd())
												mainvar->AddString(name,"|><| ");
											else
												if(pattern->IfOffSetStart())
													mainvar->AddString(name,"|> ");
												else
													if(pattern->IfOffSetEnd())
														mainvar->AddString(name,"<| ");
										}

										mainvar->AddString(name,af->audioevent.audioefile->camxrecorded==true?Cxs[CXS_RECORDED]:af->audioevent.audioefile->GetName());
									}
									break;

								default:
									name[0]=0;
									break;
								}

								if(mainsettings->arrangeeditorfiledisplay!=0 && af->audioevent.audioregion)
								{
									mainvar->AddString(name," (R)");
									mainvar->AddString(name,af->audioevent.audioregion->regionname);
								}

								mainvar->AddString(name,(af->mainpattern && af->mainclonepattern==0)?" Loop:":" P:");
								mainvar->AddString(name,af->mainpattern?af->mainpattern->GetName():af->GetName());
							}
							break;
						}

						// Time
						{
							char h2[64];

							LONGLONG length=af->GetSamples();

							mainvar->AddString(name,"*");
							mainvar->AddString(name,mainvar->ConvertSamplesToTime(length,0,h2));
						}

						if(pattern->link){
							mainvar->AddString(name," (Link:");

							char h2[16];

							mainvar->AddString(name,mainvar->ConvertIntToChar(pattern->link->GetIndex()+1,h2));
							mainvar->AddString(name,")");
						}

						/*
						if(pattern->mouseselection==true)
						{
						UBYTE r,g,b;

						maingui->colourtable.GetRGB(COLOUR_WHITE,&r,&g,&b);
						bitmap->SetTextColour(r,g,b);
						}
						else
						*/
						{
							UBYTE r,g,b;

							maingui->colourtable.GetRGB(COLOUR_WHITE,&r,&g,&b);
							bitmap->SetTextColour(r,g,b);
						}
					}

					/*
					if(af->GetPatternVolume()!=1)
					{
					if(char *h=mainaudio->GenerateDBString(af->GetPatternVolume()))
					{
					mainvar->AddString(name," PV:");
					mainvar->AddString(name,h);
					delete h;
					}
					}
					*/

					if(showpeakprogress>0) // Peakfile under construction ?
					{
						char h[NUMBERSTRINGLEN];

						mainvar->AddString(name," ");
						mainvar->AddString(name,Cxs[CXS_PEAKPROGRESS]);
						mainvar->AddString(name,":");
						mainvar->AddString(name,mainvar->ConvertDoubleToChar(showpeakprogress,h,2));
						mainvar->AddString(name,"%");
					}
				}
			}
		} // End Audio
		break;

	case MEDIATYPE_MIDI:
		{
			MIDIPattern *mp=(MIDIPattern *)pattern;
			OSTART estart,laststart=-1;
			int lastx,lasteventx=-1,eventcolour;

			sysstartup=mp->sendonlyatstartup;

			if(mp->IsPatternSysExPattern()==true)
			{
				SysEx *sysex=(SysEx *)mp->FirstEvent();

				if(char *t=sysex->GetSysExString())
				{
					char help[NUMBERSTRINGLEN];
					char *s=mainvar->ConvertIntToChar(sysex->length,help);

					if(mp->sendonlyatstartup==true)
					{
						if(char *a=mainvar->GenerateString("S*",t,"/:",s))
						{
							bitmap->guiDrawText(sx,sy+maingui->GetFontSizeY(),sx2,a);
							delete a;
						}
					}
					else
						if(char *a=mainvar->GenerateString(t,"/:",s))
						{
							bitmap->guiDrawText(sx,sy+maingui->GetFontSizeY(),sx2,a);
							delete a;
						}
				}
			}
			else
			{

				if(this->automationpattern==true)
					eventcolour=COLOUR_FRONT_NOTSELECTED;
				else
					if(editor->mousemode==EM_MOVEPATTERN)
						eventcolour=undermove==true?COLOUR_FRONT_SELECTED:COLOUR_FRONT_NOTSELECTED;
					else
						eventcolour=mp->IsSelected()==true?COLOUR_FRONT_SELECTED:COLOUR_FRONT_NOTSELECTED;

				// Show Open Notes
				mp->LockOpenNotes();

				if(NoteOpen *no=mp->FirstOpenNote())
				{
					class temp_OpenNote
					{
					public:
						OSTART time;
						int key;
					};

					int ct=mp->openrecnotes.GetCount();

					if(temp_OpenNote *ton=new temp_OpenNote[ct])
					{
						int i=0;

						while(no)
						{
							ton[i].time=no->ostart;
							ton[i].key=no->key;
							i++;
							no=no->NextOpenNote();
						}

						mp->UnlockOpenNotes();

						for(int i=0;i<ct;i++)
						{
							OSTART spos,epos;

							if(pattern->track->song->status&Seq_Song::STATUS_STEPRECORD)
							{
								spos=pattern->track->song->GetSongPosition();
								epos=spos+steptimes[pattern->track->song->steplength];
							}
							else
							{
								spos=ton[i].time;
								epos=pattern->track->song->GetSongPosition();
							}

							if(spos<editor->endposition && epos>editor->startposition)
							{
								int x=spos<=editor->startposition?editor->timeline->x:editor->timeline->ConvertTimeToX(spos,bitmapx2),
									x2=epos>=editor->endposition?editor->timeline->x2:editor->timeline->ConvertTimeToX(epos,bitmapx2);

								double h=ton[i].key,h2=sy2-sy;

								h/=127; // 0-1
								h2*=h;

								int ey2=(int)(sy2-h2);

								bitmap->guiDrawLineY(ey2,x,x2,COLOUR_RED);	
							}
						}

						delete ton;
					}
				}
				else
				{
					mp->UnlockOpenNotes();

					if(mp->FirstEvent()==0 && sy+maingui->GetFontSizeY()+1<sy2)
						bitmap->guiDrawText(sx+1,sy+maingui->GetFontSizeY(),sx2,"*0");
				}

				OSTART offset=start-mp->GetPatternStart();
				Seq_Event *e;
				{
					e=mp->FirstEvent();

					while(e && (estart=e->GetEventStart(mp)+offset)<editor->startposition) // <<<
					{
						switch(e->id)
						{
						case OBJ_NOTE:
							if(editor->shownotesinarrageeditor==ARRANGEEDITOR_SHOWNOTES_LINES)
							{
								Note *note=(Note *)e;

								OSTART end=note->GetNoteEnd(mp)+offset;

								if(end>=editor->startposition)
								{
									double h=note->key,h2=sy2-sy;

									h/=127; // 0-1
									h2*=h;

									int ey2=(int)(sy2-h2),ex2=end>editor->endposition?editor->timeline->x2:editor->timeline->ConvertTimeToX(end,bitmapx2);
									bitmap->guiDrawLineY(ey2,editor->timeline->x,ex2,eventcolour);	
								}
							}
							break;
						}

						e=e->NextEvent();
					}
					/*
					else
					{
					e=mp->FirstEvent();

					if(e)
					{
					estart=e->GetEventStart(mp)+offset;
					e=mp->FindEventAtPosition(estart,SEL_ALLEVENTS);
					}
					}
					*/

					while(e && (estart=e->GetEventStart(mp)+offset)<editor->endposition)
					{
						switch(e->id)
						{
						case OBJ_NOTE:
							if(editor->shownotesinarrageeditor!=ARRANGEEDITOR_SHOWNOTES_OFF)
							{
								Note *note=(Note *)e;
								int ex;

								if(estart!=laststart)
									lastx=ex=editor->timeline->ConvertTimeToX(laststart=estart,bitmapx2);
								else
									ex=lastx;

								double h=note->key,h2=sy2-sy;

								h/=127; // 0-1
								h2*=h;

								int ey2=(int)(sy2-h2);

								switch(editor->shownotesinarrageeditor)
								{
								case ARRANGEEDITOR_SHOWNOTES_LINES:
									{
										OSTART end=note->GetNoteEnd(mp)+offset;

										if(end-estart<=editor->timeline->zoom->ticksperpixel)
											bitmap->guiDrawPixel(ex,ey2,eventcolour);
										else
										{
											bitmap->guiDrawLineY(ey2,ex,end>editor->endposition?bitmapx2:editor->timeline->ConvertTimeToX(end,bitmapx2),eventcolour);
										}		
									}
									break;

								case ARRANGEEDITOR_SHOWNOTES_NOTES:
									{
										int ex2=ex+3;
										int ly=(int)h;

										if(ly<1)ly=1;

										// Show Notes
										if(ex2<=bitmapx2 /* && (ex!=lasteventx || (note->key>=64 && lastkeyup==false) || (note->key<64 && lastkeyup==true))*/ )
										{
											lasteventx=ex;

											ey2-=ly/2;

											if(ey2<sy)ey2=sy;
											if(ey2+ly>sy2)ly=sy2-ey2;

											if(ly>=1)
											{
												bitmap->guiFillRect(ex,ey2,ex2,ey2+ly,eventcolour);

												ly*=4;

												if(note->key>=64)
												{
													//	lastkeyup=true;
													bitmap->guiDrawLineX(ex,ey2,ey2+ly>sy2?sy2:ey2+ly,eventcolour);
												}
												else
												{
													//	lastkeyup=false;
													bitmap->guiDrawLineX(ex2,ey2-ly<sy?sy:ey2-ly,ey2,eventcolour);
												}
											}
										}	
									}
									break;
								}// switch note display
							}
							break;

						case OBJ_ICDEVENT:
							{
								switch(e->GetStatus())
								{
								case INTERN:
									{
									}
									break;

								case INTERNCHAIN:
									{
										ICD_Object_Seq_MIDIChainEvent *icdobject=(ICD_Object_Seq_MIDIChainEvent *)e;

										switch(icdobject->type)
										{
										case ICD_TYPE_DRUM: // Drum Event
											{
												ICD_Drum *drum=(ICD_Drum *)icdobject;
												int ex;

												if(estart!=laststart)
													lastx=ex=editor->timeline->ConvertTimeToX(laststart=estart,bitmapx2);
												else
													ex=lastx;

												bitmap->guiDrawLineX(ex,sy,sy2,COLOUR_BLUE);
											}
											break;
										}
									}
									break;
								}

							}
							break;

						case OBJ_POLYPRESSURE:
							if(editor->showcontrols==true)
							{
								PolyPressure *poly=(PolyPressure *)e;
								double h=poly->pressure,h2=sy2-sy;

								h/=127; // 0-1
								h2*=h;

								int ex,ey2=(int)(sy2-h2);

								if(estart!=laststart)
								{	
									lastx=ex=editor->timeline->ConvertTimeToX(laststart=estart,bitmapx2);

									if(ey2>y)
										bitmap->guiDrawLineX(ex,sy,sy2,COLOUR_BLUE_LIGHT);
								}
								else
									ex=lastx;

								bitmap->guiDrawLineX(ex,ey2,sy2,COLOUR_GREEN);	
							}
							break;

						case OBJ_CONTROL:
							if(editor->showcontrols==true)
							{
								ControlChange *c=(ControlChange *)e;
								double h=c->value,h2=sy2-sy;

								h/=127; // 0-1
								h2*=h;

								int ex,ey2=(int)(sy2-h2);

								if(estart!=laststart)
								{	
									lastx=ex=editor->timeline->ConvertTimeToX(laststart=estart,bitmapx2);
									if(ey2>y)bitmap->guiDrawLineX(ex,sy,sy2,COLOUR_BLUE_LIGHT);
								}
								else
									ex=lastx;

								bitmap->guiDrawLineX(ex,ey2,sy2,COLOUR_RED);
							}
							break;

						case OBJ_PROGRAM:
							if(editor->showcontrols==true)
							{
								ProgramChange *p=(ProgramChange *)e;
								double h=p->program,h2=sy2-sy;

								h/=127; // 0-1
								h2*=h;

								int ex,ey2=(int)(sy2-h2);

								if(estart!=laststart)
								{	
									lastx=ex=editor->timeline->ConvertTimeToX(laststart=estart,bitmapx2);
									if(ey2>y)bitmap->guiDrawLineX(ex,sy,sy2,COLOUR_BLUE_LIGHT);
								}
								else
									ex=lastx;

								bitmap->guiDrawLineX(ex,ey2,sy2,COLOUR_RED);
							}
							break;

						case CHANNELPRESSURE:
							if(editor->showcontrols==true)
							{
								ChannelPressure *cp=(ChannelPressure *)e;
								double h=cp->pressure,h2=sy2-sy;

								h/=127; // 0-1
								h2*=h;

								int ex,ey2=(int)(sy2-h2);

								if(estart!=laststart)
								{	
									lastx=ex=editor->timeline->ConvertTimeToX(laststart=estart,bitmapx2);
									if(ey2>y)bitmap->guiDrawLineX(ex,sy,sy2,COLOUR_BLUE_LIGHT);
								}
								else
									ex=lastx;

								bitmap->guiDrawLineX(ex,ey2,sy2,COLOUR_GREEN);	
							}
							break;

						case OBJ_PITCHBEND:
							if(editor->showcontrols==true)
							{
								Pitchbend *p=(Pitchbend *)e;
								double h=p->msb,h2=sy2-sy;

								h/=127; // 0-1
								h2*=h;

								int ex,ey2=(int)(sy2-h2);

								if(estart!=laststart)
								{	
									lastx=ex=editor->timeline->ConvertTimeToX(laststart=estart);
									if(ey2>y)bitmap->guiDrawLineX(ex,sy,sy2,COLOUR_BLUE_LIGHT);
								}
								else
									ex=lastx;

								bitmap->guiDrawLineX(ex,ey2,sy2,COLOUR_YELLOW);
							}
							break;

						case OBJ_SYSEX:
							if(editor->showcontrols==true)
							{
								SysEx *sysex=(SysEx *)e;

								int ex;

								if(estart!=laststart)
									lastx=ex=editor->timeline->ConvertTimeToX(laststart=estart);
								else
									ex=lastx;

								bitmap->guiDrawLineX(ex,sy,sy2,COLOUR_RED_LIGHT);

								if(y2-y>maingui->GetFontSizeY_Sub())
									bitmap->guiDrawText(ex+1,sy2,x2,"SysEx");
							}
							break;
						}

						e=e->NextEvent();
					}
				}
			}

			if(notext==false && automationpattern==false) // Pattern Name
			{
				if(name=new char[strlen(pattern->GetName())+64])
				{
					if(pattern->mainclonepattern)
					{
						strcpy(name,"C:");
						mainvar->AddString(name,pattern->GetName());
					}
					else
						strcpy(name,pattern->GetName());

					if(pattern->mediatype==MEDIATYPE_MIDI)
					{
						MIDIPattern *mp=(MIDIPattern *)pattern;

						if(mp->IsPatternSysExPattern()==true)
						{
							if(mp->sendonlyatstartup==true)
								mainvar->AddString(name,"S*SysEx");
							else
								mainvar->AddString(name," SysEx");
						}

					}

					if(pattern->link)
					{
						mainvar->AddString(name," (Link:");
						char h2[16];
						mainvar->AddString(name,mainvar->ConvertIntToChar(pattern->link->GetIndex()+1,h2));
						mainvar->AddString(name,")");
					}

					/*
					if(pattern->mouseselection==true)
					{
					UBYTE r,g,b;
					maingui->colourtable.GetRGB(COLOUR_WHITE,&r,&g,&b);

					bitmap->SetTextColour(r,g,b);
					}
					else
					*/
				}
			}
		}
		break;
	}// switch

	if(patternname)
	{
		delete patternname;
		patternname=0;
	}

	if(notext==false)
	{
		patternname=mainvar->GenerateString(pattern->GetName());

		/*
		if(editor->activepattern==pattern)
		{
		if(name)
		{
		char *h=mainvar->GenerateString(pattern->mute==true?"(Mute) ~~~":"[~~~ ",name," ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~]");
		delete name;
		name=h;
		}
		else
		name=mainvar->GenerateString(pattern->mute==true?"(Mute) >>>  <<<":"[>>>  <<<]");
		}
		else
		*/
		{
			if(pattern->mute==true)
			{
				if(name)
				{
					char *h=mainvar->GenerateString("(Mute)",name);
					delete name;
					name=h;
				}
				else
					name=mainvar->GenerateString("(Mute)",".");
			}
		}

		// Text
		if(name)
		{
			bitmap->SetTextColour(folder==true?COLOUR_FOLDERBORDER:COLOUR_WHITE);
			bitmap->SetFont(&maingui->smallfont);
			bitmap->guiDrawText(sx+2,sy-2,sx2,name);

			/*
			if(pattern->flag&OFLAG_SELECTED)
			bitmap->guiDrawText(sx+3,sy-1,sx2,name);
			*/
			bitmap->SetFont(&maingui->standardfont);

			UBYTE r,g,b;
			maingui->colourtable.GetRGB(COLOUR_BLACK,&r,&g,&b);

			bitmap->SetTextColour(r,g,b);
		}
	}

	if(name)delete name;

	if(automationpattern==true)
		return;

	if(undermove==true)
	{
		bitmap->guiDrawRect(x,y,x2,y2,COLOUR_BORDER_OBJECTMOVING);
	}
	else
	{
		if(x2>x+4 && y2>y+4)
		{
			int bordercolor=COLOUR_PATTERNBORDER;

			if(folder==true)
			{
				bordercolor=COLOUR_FOLDERBORDER;
			}
			else
			{
				if(pattern==editor->WindowSong()->GetFocusPattern())
				{
					bordercolor=pattern->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT;
				}
				else
					if(pattern->IsSelected()==true)
					{
						bordercolor=COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
					}

			}

			bitmap->guiDrawRect(sx,y,sx2,y2,bordercolor);
			bitmap->guiDrawRect(sx+1,y+1,sx2-1,y2-1,bordercolor);
		}

		if(pattern->flag&OFLAG_UNDERSELECTION)
			bitmap->guiInvert(x+1,y+1,x2-1,y2-1);
	}
}

Seq_SelectionList *Edit_Arrange::CreateSelectionListAll(bool all)
{
	if(Seq_SelectionList *sl=new Seq_SelectionList)
	{
		Seq_Track *t=WindowSong()->FirstTrack();

		while(t){
			Seq_Pattern *p=t->FirstPattern();

			while(p){

				if(p->IsSelected()==true || all==true)
					sl->AddPattern(p);

				p=p->NextPattern();
			}

			t=t->NextTrack();
		}

		return sl;
	}

	return 0;
}

Seq_SelectionList *Edit_Arrange::CreateSelectionList(Seq_Pattern *addpattern)
{
	selection.DeleteAllPattern();

	Seq_Track *t=WindowSong()->FirstTrack();
	while(t){

		Seq_Pattern *p=t->FirstPattern();

		while(p){
			if(p->IsSelected()==true || p==addpattern)
				selection.AddPattern(p);

			p=p->NextPattern();
		}

		t=t->NextTrack();
	}

	return &selection;
}

void Edit_Arrange::CopySelectedPattern()
{
	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		mainbuffer->OpenBuffer();
		mainbuffer->AddObjectToBuffer(sl,true);
		mainbuffer->CloseBuffer();
	}
}

void Edit_Arrange::CutSelectedPattern()
{
	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		mainbuffer->OpenBuffer();
		bool ok=mainbuffer->AddObjectToBuffer(sl,true);
		mainbuffer->CloseBuffer();

		if(ok==true)mainedit->DeletePattern(WindowSong(),sl,0,false);
	}
}

// Cut Pattern under mouse
void Edit_Arrange::CutPattern_Arrange(Seq_Pattern *pattern)
{
	Seq_SelectionList *sl=CreateSelectionList();

	if(sl)
	{
		if((pattern->flag&OFLAG_SELECTED)==0)
			sl->AddPattern(pattern);

		mainedit->CutPattern(sl,GetMousePosition());
	}
}

void Edit_Arrange::RefreshTrack(Seq_Track *track)
{
	Edit_Arrange_Track *et=FindTrack(track);

	if(et)
	{
		et->ShowInput();
		tracks->Blt(et);
	}
}

void Edit_Arrange::MoveSelectedPatternToMeasureToTick(OSTART tick,bool copy,Seq_Track *starttrack)
{
	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		int ft=-1,lt=-1,index=0;

		OSTART fppos=-1;

		// Get First Pattern Tick Position
		Seq_SelectedPattern *selp=sl->FirstSelectedPattern();

		while(selp)
		{
			if(fppos==-1 || fppos>selp->pattern->GetPatternStart())
				fppos=selp->pattern->GetPatternStart();

			int tix=WindowSong()->GetOfTrack(selp->pattern->track);

			if(ft==-1 || tix<ft)ft=tix;
			if(lt==-1 || tix>lt)lt=tix;

			selp=selp->NextSelectedPattern();
		}

		if(starttrack)
		{
			int st=WindowSong()->GetOfTrack(starttrack);

			index=st-ft;
			if(lt+index>WindowSong()->GetOfTrack(WindowSong()->LastTrack()))
				return;
		}

		if(fppos>=0 && (fppos!=tick || index!=0))
		{
			MoveO mo;

			mo.song=WindowSong();
			mo.sellist=sl;
			mo.diff=tick-fppos;
			mo.index=index;
			mo.flag=0;
			mo.quantize=false;

			copy==true?mainedit->CopyPatternList(&mo):mainedit->MovePatternList(&mo);
		}
	}
}

void Edit_Arrange::MovePatternCycle(bool right,bool selectedpattern,bool selectedtracks)
{
	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		OSTART cyclerange=WindowSong()->playbacksettings.GetCycleRange();
		MoveO mo;

		mo.song=WindowSong();
		mo.sellist=sl;
		mo.diff=right==true?cyclerange:-cyclerange;
		mo.index=0;
		mo.flag=0;
		mo.allpattern=true;
		mo.selectedpattern=selectedpattern;
		mo.selectedtracks=selectedtracks;
		mo.special=true;

		mo.cyclecheck=true;

		mainedit->MovePatternList(&mo);
	}
}

void Edit_Arrange::MoveSelectedPatternToMeasure(bool copy)
{
	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		if(EditData *edit=new EditData)
		{
			edit->win=this;
			edit->x=0;
			edit->y=0;

			if(copy==true)
			{
				edit->id=EDIT_SCOPYPATTERN;
				edit->title=Cxs[CXS_COPYSELECTEDPATTERNTOMEASURE];
			}
			else
			{
				edit->id=EDIT_SMOVEPATTERN;
				edit->title=Cxs[CXS_MOVESELPATTERNM];
			}

			edit->type=EditData::EDITDATA_TYPE_COPYMOVEPATTERN;
			edit->from=1;
			edit->to=WindowSong()->GetSongLength_Measure();
			edit->value=1;
			edit->song=WindowSong();
			edit->copyflag=copy;

			maingui->EditDataValue(edit);
		}
	}
}

void Edit_Arrange::DeselectAllAutomationParameters(AutomationTrack *not)
{
	Seq_Track *t=WindowSong()->FirstTrack();
	while(t)
	{
		t->DeselectAllAutomationParameters(not);
		t=t->NextTrack();
	}

	WindowSong()->audiosystem.masterchannel.DeselectAllAutomationParameters(not);

	AudioChannel *b=WindowSong()->audiosystem.FirstBusChannel();
	while(b)
	{
		b->DeselectAllAutomationParameters(not);
		b=b->NextChannel();
	}
}

void Edit_Arrange::SelectAllAutomationParameters(bool on)
{
	int c=0;

#ifdef OLDIE
	Seq_Track *t=song->FirstTrack();

	while(t)
	{
		AutomationTrack *at=t->FirstAutomationTrack();

		while(at)
		{
			AutomationParameter *o=at->FirstAutomationParameter();

			while(o)
			{
				if(o->IsSelected()==true)
				{
					o->flag CLEARBIT OFLAG_SELECTED;
					c++;
				}

				o=o->NextAutomationParameter();
			}

			at=at->NextAutomationTrack();
		}

		t=t->NextTrack();
	}
#endif

	if(c)
	{
		maingui->RefreshAutoTracks(0,WindowSong(),0);
	}
}

void Edit_Arrange::SelectAllPattern (bool on,Seq_Pattern *not,Seq_Track *track)
{
	Seq_Track *t=(!track)?WindowSong()->FirstTrack():track;

	while(t)
	{
		if(t->IsEditAble()==true)
		{
			if((!track) || t==track)
			{
				if(t->IsOpen()==true)
				{
					Seq_Pattern *p=t->FirstPattern(MEDIATYPE_ALL);

					while(p)
					{
						if(p!=not && p->realpattern==true)
							SelectPattern(p,on,true);

						p=p->NextPattern(MEDIATYPE_ALL);
					}
				}

				if(track)return;
			}
		}

		if(track)break;

		t=t->NextTrack();
	}
}

Editor *Edit_Arrange::OpenDoubleClickEditor(Seq_Pattern *pattern,OSTART time)
{
	Editor *e=0;
	guiWindowSetting settings;

	settings.screen=screen;
	settings.calledfromwindow=this;

	if(pattern->realpattern==true)
	{
		SelectPattern(pattern,true,true);

		switch(pattern->mediatype)
		{
		case MEDIATYPE_MIDI:
			{
				// Find Drum Events
				int drumcounter=0,divevents=0;

				if(Seq_SelectionList *sl=CreateSelectionList())
				{
					Seq_SelectedPattern *sp=sl->FirstSelectedPattern();

					while(sp)
					{
						Seq_Event *e=sp->pattern->FirstEditEvent();

						while(e)
						{
							switch(e->GetStatus())
							{
							case INTERN:
								break;

							case INTERNCHAIN:
								{
									switch(e->GetICD())
									{
									case ICD_TYPE_DRUM:
										drumcounter++;
										break;
									}
								}
								break;

							default:
								divevents++;
								break;
							}

							e=e->NextEvent();
						}

						sp=sp->NextSelectedPattern();
					}


					if(drumcounter && divevents==0)
					{
						maingui->OpenEditorStart(EDITORTYPE_DRUM,WindowSong(),0,sl,0,0,time);
					}
					else
						switch(mainsettings->opendoubleclickeditor)
					{
						case Settings::OPENDBCLK_PIANO:
							maingui->OpenEditorStart(EDITORTYPE_PIANO,WindowSong(),0,sl,&settings,0,time);
							break;

						case Settings::OPENDBCLK_EVENT:
							maingui->OpenEditorStart(EDITORTYPE_EVENT,WindowSong(),0,sl,&settings,0,time);
							break;

						case Settings::OPENDBCLK_DRUM:
							maingui->OpenEditorStart(EDITORTYPE_DRUM,WindowSong(),0,sl,&settings,0,time);
							break;

						case Settings::OPENDBCLK_WAVE:
							maingui->OpenEditorStart(EDITORTYPE_WAVE,WindowSong(),0,sl,&settings,0,time);
							break;
					}
				}
			}
			break;

		case MEDIATYPE_AUDIO:
			{
				AudioPattern *af=(AudioPattern *)pattern;

				if(af->CanBeEdited()==true)
				{
					guiWindow *win=maingui->FindWindow(EDITORTYPE_SAMPLE,af->audioevent.audioefile,0);

					if(!win)
					{
						Edit_Sample_StartInit startinit;

						startinit.file=af->audioevent.audioefile;
						startinit.pattern=af;
						startinit.startposition=time;

						if(af->audioevent.audioregion)
						{
							startinit.rangestart=af->audioevent.audioregion->regionstart;
							startinit.rangeend=af->audioevent.audioregion->regionend;
						}

						maingui->OpenEditorStart(EDITORTYPE_SAMPLE,WindowSong(),0,0,&settings,(Object *)&startinit,0);
					}
					else
						win->WindowToFront(true);
				}
			}
			break;
		}
	}

	return e;
}

void Edit_Arrange::UnSelectAllInPattern(AutomationTrack *automationtrack,AutomationParameter *ap)
{
	SelectAllPattern(false,0);
	DeselectAllAutomationParameters((ap && ap->IsSelected()==true) || maingui->GetCtrlKey()==true?automationtrack:0);
}

void Edit_Arrange::CancelEditFades()
{
	if(fadeeditpattern)
	{
		fadeeditpattern->volumecurve.SetFadeIn(fadeeditpattern->volumecurve.b_fadeinms);
		fadeeditpattern->volumecurve.SetFadeOut(fadeeditpattern->volumecurve.b_fadeoutms);
		fadeeditpattern->volumecurve.SetVolume(fadeeditpattern->volumecurve.b_dbvolume);
		fadeeditpattern->volumecurve.editmode=false;
	}
}

void Edit_Arrange::EditFades()
{
	if(fadeeditpattern)
	{
		InitMousePosition();

		switch(mousemode)
		{
		case EM_EDIT_FADEIN:
			{
				OSTART fi=GetMousePosition()-fadeeditpattern->GetPatternStart();

				if(fi<=0)
				{
					fadeeditpattern->volumecurve.SetFadeIn(0);
				}
				else
				{
					LONGLONG p=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(fadeeditpattern->GetPatternStart(),fi);
					double ms=mainaudio->ConvertSamplesToMs(p);

					fadeeditpattern->volumecurve.SetFadeIn(ms);
				}
			}
			break;

		case EM_EDIT_FADEOUT:
			{
				OSTART pend=fadeeditpattern->GetPatternEnd();

				if(GetMousePosition()>=pend)
				{
					fadeeditpattern->volumecurve.SetFadeOut(0);
				}
				else
				{
					OSTART startfo=WindowSong()->timetrack.ConvertSamplesToTempoTicks(fadeeditpattern->GetPatternStart(),fadeeditpattern->volumecurve.b_fadeoutsamples);
					OSTART fo=GetMousePosition()-startfo;

					LONGLONG p=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(startfo,fo);
					double ms=mainaudio->ConvertSamplesToMs(p);

					fadeeditpattern->volumecurve.SetFadeOut(fadeeditpattern->volumecurve.b_fadeoutms-ms);
				}
			}
			break;

		case EM_EDIT_VOLUME:
			{
				int my=pattern->GetMouseY();

				int diff=my-fademousey;

				if(diff)
				{
					fademousey=my;

					double dbv=fadeeditpattern->volumecurve.dbvolume;

					if(dbv<-18)
					{
						if(diff<0)
							dbv+=1;
						else
							dbv-=1;
					}
					else
						if(dbv>6 || dbv<-6)
						{
							if(diff<0)
								dbv+=0.5;
							else
								dbv-=0.5;
						}
						else
							if(dbv>=1 || dbv<=-1)
							{
								if(diff<0)
									dbv+=0.25;
								else
									dbv-=0.25;
							}
							else
							{
								if(fadeeditdelta==3) // 6 wait steps
								{
									fadeeditdelta=0;

									if(diff<0)
										dbv+=0.1;
									else
										dbv-=0.1;

									if(dbv>=-0.09 && dbv<=0.09)
										dbv=0;
								}
								else
									fadeeditdelta++;

							}


							if(dbv>24)
								dbv=24;
							else
								if(dbv<mainaudio->ConvertFactorToDb(mainaudio->silencefactor))
									dbv=mainaudio->ConvertFactorToDb(mainaudio->silencefactor);

							fadeeditpattern->volumecurve.SetVolume(dbv);
				}

			}
			break;
		}

	}
}

void Edit_Arrange::SetFadePattern(Seq_Pattern *p)
{
	p->volumecurve.editmode=true;
	fadeeditpattern=p;
	fadeeditstartposition=GetMousePosition();
	fademousey=pattern->GetMouseY();
	fadeeditdelta=0;
}

void Edit_Arrange::MouseClickInPattern(bool leftmouse)
{

	if(leftmouse==false)
	{
		if(EditCancel()==true)
			return;
	}
	else
	{
		if(CheckMouseClickInEditArea(pattern)==true) // Left Mouse
		{
			return;
		}
	}

	InitMousePosition();

	int px=pattern->GetMouseX();
	int py=pattern->GetMouseY();

	if(Edit_Arrange_AutomationTrack *eat=FindAutomationTrackAtY(py))
	{
		// Automation Track
		if(eat->automationtrack->visible==true)
			eat->MouseClick(px,py,leftmouse);

		return;
	}

	Seq_Track *track=FindTrackAtY(py);

	guiObject *o=pattern->CheckObjectClicked(); // Object Under Mouse ?

	if((!o) && leftmouse==false)
	{
		if(track)
		{
			if(DeletePopUpMenu(true))
			{	
				int mode=Seq_Pos::POSMODE_NORMAL;

				switch(windowtimeformat)
				{
				case WINDOWDISPLAY_SMPTE:
					mode=WindowSong()->project->standardsmpte;
					break;
				}

				TimeString timestring;

				WindowSong()->timetrack.CreateTimeString(&timestring,GetMousePosition(),mode);

				char *add=0;

				switch(track->id)
				{
				case OBJ_TRACK:
					{
						char h2[NUMBERSTRINGLEN];
						char *tracknr=mainvar->ConvertIntToChar(WindowSong()->GetOfTrack(track)+1,h2);

						add=mainvar->GenerateString("Track ",tracknr,":",track->GetName());
					}
					break;
				}

				if(add)
				{
					if(char *h=mainvar->GenerateString(add,":",timestring.string))
					{
						popmenu->AddMenu(h,0);
						delete h;
					}

					delete add;
				}
				else
					popmenu->AddMenu(timestring.string,0);

				popmenu->AddLine();

				if(track->id==OBJ_TRACK)
				{
					if(track->frozen==true)
					{
						class menu_unfreeze:public guiMenu
						{
						public:
							menu_unfreeze(Edit_Arrange *ar,Seq_Track *t){arrange=ar;track=t;}

							void MenuFunction(){
								arrange->FreezeTrack(0,false);
							} 

							Edit_Arrange *arrange;
							Seq_Track *track;
						};

						popmenu->AddMenu("# Frozen Track #",0);
						popmenu->AddLine();
						popmenu->AddFMenu("Unfreeze Track",new menu_unfreeze(this,track));
					}
					else
					{
						if(track->ismetrotrack==false)
						{
							AddCreatePatternMenu(track);
							AddPopFileMenu(track);
							AddPasteMenu(track);
						}
					}
				}
				else // SubTracl
				{
					//	popmenu->AddMenu("Subtrack",ARR_POPMENU_CREATEFOLDER);
				}

				AddActiveTrackMenu(track);
				AddSetSongPositionMenu(0,track);

				ShowPopMenu();

			}//if track

		}//if et
		else
			if(DeletePopUpMenu(true))
			{
				popmenu->AddFMenu(0,new globmenu_cnewtrack(WindowSong(),WindowSong()->LastParentTrack()));
				AddSetSongPositionMenu(0,0);
				ShowPopMenu();
			}

			return;

			// o==0  Right Mouse
	}

	Seq_Track *setfocustrack=0;

	if(leftmouse==true)
	{	
		if((!o) && mousemode==EM_CREATE && track && track->frozen==false)
		{
			mainedit->CreateNewPattern(0,track,MEDIATYPE_MIDI,GetMousePosition(),true);
			return;
		}

		if(((!o) || (o && track && track->frozen==true)) && maingui->GetCtrlKey()==false)
		{
			// Set Focus Track
			setfocustrack=track; // avoid gui problems
			UnSelectAllInPattern(0,0);
		}

		if((!o) || maingui->GetShiftKey()==true)
		{
			SetMouseMode(EM_SELECTOBJECTS);
			goto setfocus;
		}
	}

	if(!o)
		goto setfocus;

	switch(o->id)
	{
	case OBJECTID_ARRANGEPATTERN:
		{
			Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)o;

			if(eap->folder==true)
			{
			}
			else
			{
				if(leftmouse==false)
				{
					Seq_Pattern *pattern=eap->pattern;

					if(pattern->recordpattern==false)
					{
						if(pattern->visible==true && pattern->realpattern==true)
						{
							switch(mousemode)
							{
							case EM_CREATE:
								if(pattern->EditAble()==true)
								{
									SelectPattern(pattern,true,false);

									if(Seq_SelectionList *sl=CreateSelectionList())
										mainedit->DeletePattern(WindowSong(),sl,0,false);

									goto setfocus;
								}
								break;

							default:
								CheckPopMenu(o);
								break;
							}
						}
						else
							CheckPopMenu(o);
					}

				}
				else
				{
					// Left

					Seq_Pattern *pattern=eap->pattern;

					if(pattern->visible==true && pattern->realpattern==true)
					{
						if(CheckCrossFadeClick()==true)
							goto setfocus;

						switch(mousemode)
						{
						case EM_SELECT:
							{
								if(int pt=eap->patternvolumepositions.CheckXY(px,py)) // Edit Fade ?
								{
									switch(pt)
									{
									case PatternVolumePositions::IN_FADEIN:
										{
											pattern->volumecurve.StartEdit();

											SetFadePattern(pattern);
											SetEditorMode(EM_EDIT_FADEIN);
										}
										break;

									case PatternVolumePositions::IN_VOLUME:
										{
											pattern->volumecurve.StartEdit();

											SetFadePattern(pattern);
											SetEditorMode(EM_EDIT_VOLUME);
										}
										break;

									case PatternVolumePositions::IN_FADEOUT:
										{
											pattern->volumecurve.StartEdit();

											SetFadePattern(pattern);
											SetEditorMode(EM_EDIT_FADEOUT);
										}
										break;
									}

									goto setfocus;
								}

								bool set=false;

								if(pattern->EditAble()==true && pattern->SizeAble()==true)
								{
									if(pattern->GetPatternEnd()<=endposition && py>=o->y && py<=o->y2 && px<=o->x2 && px>=o->x2-6)
									{
										// Size Right
										SetEditorMode(EM_SIZEPATTERN_RIGHT);
										set=true;
									}
									else
										if(pattern->GetPatternStart()>=startposition && set==false && py>=o->y && py<=o->y2 && px>=o->x && px<=o->x+6)
										{
											SetEditorMode(EM_SIZEPATTERN_LEFT);
											set=true;
										}		

										if(set==false)
											addpattern=pattern;
								}

								if(set==false)
								{
									if(pattern->IsSelected()==false && maingui->GetCtrlKey()==false)
										SelectAllPattern(false,pattern);

									SelectPattern(pattern,true,true);

									mainhelpthread->AddMessage(MOUSEBUTTONDOWNMS,this,MESSAGE_CHECKMOUSEBUTTON,MESSAGE_PATTERN,pattern);
								}
								else
								{
									// Init Size
									sizepattern=pattern;

									if(mousemode==EM_SIZEPATTERN_LEFT)
									{
										SetMouseCursor(CURSOR_LEFT);
										modestartposition=pattern->GetPatternStart();
									}
									else
									{
										modestartposition=pattern->GetPatternEnd();
										SetMouseCursor(CURSOR_RIGHT);
									}

									selection.movediff=0; // Reset
									pattern->InitOffSetEdit(mousemode);
									SetAutoScroll(AUTOSCROLL_TIME_EDITOR,timeline->x,timeline->y,timeline->x2,timeline->y2);
								}	
							}
							break;

						case EM_DELETE:
							if(pattern->EditAble()==true)
							{
								SelectPattern(pattern,true,false);

								if(Seq_SelectionList *sl=CreateSelectionList())
									mainedit->DeletePattern(WindowSong(),sl,0,false);

								goto setfocus;
							}
							break;

						case EM_CUT:
							if(o->id==OBJECTID_ARRANGEPATTERN)
							{
								if(pattern->EditAble()==true)
								{
									CutPattern_Arrange(pattern);
									goto setfocus;
								}
							}
							break;
						}

						WindowSong()->SetFocusPattern(pattern);
						setfocustrack=pattern->track;
					}
				}
			}
		}
		break;
	}

setfocus:
	if(setfocustrack)
		WindowSong()->SetFocusTrack(setfocustrack);
}

void Edit_Arrange::ShowFilter()
{
	if(g_audio)
		g_audio->Toggle(GetShowFlag()&SHOW_AUDIO?true:false);

	if(g_MIDI)
		g_MIDI->Toggle(GetShowFlag()&SHOW_MIDI?true:false);

	if(g_type)
		g_type->Toggle(GetShowFlag()&SHOW_EVENTTYPE?true:false);

	if(g_input)
		g_input->Toggle(GetShowFlag()&SHOW_IO?true:false);

	if(g_automation)
		g_automation->Toggle(GetShowFlag()&SHOW_AUTOMATION?true:false);

	if(g_volumecurves)
		g_volumecurves->Toggle(GetShowFlag()&SHOW_VOLUMECURVES?true:false);

}

void Edit_Arrange::RefreshTracking()
{
	DrawDBBlit(tracks,pattern);
	ShowOverview();

	if(listeditor)
	{
		listeditor->DrawDBBlit(listeditor->list);
	}
}

void Edit_Arrange::ShowTracking()
{
	trackingindex=mainsettings->arrangetracking[set];

	if(g_filtertracks)
	{
		int cxs;

		switch(trackingindex)
		{
		case 0:
			cxs=CXS_AUTOALL;
			break;

		case 1:
			cxs=CXS_AUTOSELECTED;
			break;

		case 2:
			cxs=CXS_AUTOFOCUS;
			break;

		case 3:
			cxs=CXS_AUTOAUDIOORMIDI;
			break;

		case 4:
			cxs=CXS_AUTOAUDIO;
			break;

		case 5:
			cxs=CXS_AUTOMIDI;
			break;

		case 6:
			cxs=CXS_AUTOINSTR;
			break;

		case 7:
			cxs=CXS_AUTOTRACKSRECORD;
			break;
		}

		g_filtertracks->ChangeButtonText(Cxs[cxs]);
	}
}

int Edit_Arrange::GetShowFlag()
{
	return mainsettings->arrangesettings[set];
}

void Edit_Arrange::DoubleClickInEmptyTrack(Seq_Track *track)
{
	editmode=ED_TRACKS;
	SelectAll(false);

	if(track)
	{
		if(track->parent)
		{
			Seq_Track *ptrack=track;

			if(maingui->GetShiftKey()==true && track->PrevTrack() && track->PrevTrack()->parent==track->parent)
			{
				ptrack=track->PrevTrack();
			}

			mainedit->CreateNewChildTracks(WindowSong(),ptrack,1,EditFunctions::CREATETRACK_ACTIVATE,(Seq_Track *)track->parent,track);
		}
		else
		{
			int index=-1;

			if(maingui->GetShiftKey()==true)
			{
				if(track->PrevParentTrack())
					index=WindowSong()->GetOfTrack(track);
				else
					index=0;
			}

			mainedit->CreateNewTrack(0,WindowSong(),track,index,true,true,true);
		}
	}
	else
		mainedit->CreateNewTrack(0,WindowSong(),0,-1,true,true,true);
}

void Edit_Arrange::MouseDoubleClickInPattern(bool leftmouse)
{
	EditCancel();

	guiObject *o=pattern->CheckObjectClicked(); // Object Under Mouse ?

	if((!o) || maingui->GetShiftKey()==true)
	{
		//	Seq_Track *tr=FindTrackAtY(pattern->GetMouseY());
		//	DoubleClickInEmptyTrack(tr);

		DoubleClickInEditArea();
		return;
	}

	InitMousePosition();

	switch(o->id)
	{
	case OBJECTID_ARRANGEPATTERN:
		{
			Edit_Arrange_Pattern *p=(Edit_Arrange_Pattern *)o;	

			if(p->pattern->track->IsEditAble()==true)
			{
				if(p->folder==true)
				{
				}
				else
				{
					Seq_Pattern *pattern=p->pattern;
					// open editor
					if(pattern->EditAble()==true && pattern->visible==true)
					{
						if(CheckCrossFadeClick()==true)return;

						WindowSong()->SetFocusPattern(pattern);
						OpenDoubleClickEditor(pattern,GetMousePosition());
					}
				}
			}

			return;
		}
		break;

	case OBJECTID_ARRANGERECORD:
	case OBJECTID_ARRANGEMUTE:
	case OBJECTID_ARRANGESHOWAUTOMATIONTRACKS:
	case OBJECTID_ARRANGECHILDOPEN:
	case OBJECTID_ARRANGESOLO:
	case OBJECTID_ARRANGEAVOLUME:
		return;
	}	
}

void Edit_Arrange::MouseDoubleClickInTracks(bool leftmouse)
{
	guiObject *o=tracks->CheckObjectClicked(); // Object Under Mouse ?

	if(!o)
	{
		DoubleClickInEmptyTrack(0);
		return;
	}

	switch(o->id)
	{
		// Channels
	case OBJECTID_ARRANGECHANNELNAME:
		{
			Edit_Arrange_ChannelName *cn=(Edit_Arrange_ChannelName *)o;

			if(leftmouse==true)
			{
				if(EditData *edit=new EditData)
				{
					// long position;
					edit->song=0;

					edit->win=this;
					edit->parentdb=tracks;

					edit->x=o->x;
					edit->y=o->y;
					edit->width=o->x2-o->x;
					edit->id=EDIT_CHANNELNAME;
					edit->type=EditData::EDITDATA_TYPE_STRING;
					edit->helpobject=cn->channel->channel;
					edit->string=cn->channel->channel->GetName();
					edit->title="Bus/Channel Name";

					maingui->EditDataValue(edit);
				}
			}
		}
		break;

		// Tracks
	case OBJECTID_ARRANGERECORD:
		{
			Edit_Arrange_Record *rec=(Edit_Arrange_Record *)o;
			maingui->EditTrackRecordType(this,rec->track->track);
		}
		break;

	case OBJECTID_ARRANGETRACK:
		{
			Edit_Arrange_Track *et=(Edit_Arrange_Track *)o;
			Seq_Track *track=et->track;

			if(maingui->GetShiftKey()==true)
			{
				SelectAllPattern(true,0,track); // Select All Track Pattern
				return;
			}

			int my=tracks->GetMouseY();

			int hmy=et->y+(et->y2-et->y)/2;

			if(my>=hmy)
				DoubleClickInEmptyTrack(track);
			/*
			editmode=ED_TRACKS;
			SelectAll(false);

			if(track->parent)
			mainedit->CreateNewChildTracks(WindowSong(),track,1,EditFunctions::CREATETRACK_ACTIVATE,(Seq_Track *)track->parent,track);
			else
			mainedit->CreateNewTrack(0,WindowSong(),track,-1,true,true);
			*/
		}
		break;

	case OBJECTID_ARRANGETRACKNAME:
		{
			Edit_Arrange_Name *en=(Edit_Arrange_Name *)o;

			if(leftmouse==true)
			{
				if(EditData *edit=new EditData)
				{
					// long position;
					edit->song=0;

					edit->win=this;
					edit->parentdb=tracks;

					edit->x=o->x;
					edit->y=o->y;
					edit->width=o->x2-o->x;
					edit->id=EDIT_NAME;
					edit->type=EditData::EDITDATA_TYPE_STRING;
					edit->helpobject=en->track->track;
					edit->string=en->track->track->GetName();
					edit->title="Track Name";

					maingui->EditDataValue(edit);
				}
			}
		}
		break;

	}
}

void Edit_Arrange::DeltaInTracks()
{
	tracks->deltareturn=false;

	if(mousemode==EM_SIZEY)
	{
		int c=0;

		if(tracks->deltay!=0)
		{
			Seq_Track *t=WindowSong()->FirstTrack();

			while(t)
			{
				if(t->IsSelected()==true)
				{
					double h=t->sizefactor,
						mul=-0.00025;

					mul*=tracks->deltay;
					h+=mul;

					if(tracks->deltay>0)
					{
						if(h<SIZETRACKMIN)
							h=SIZETRACKMIN;
					}
					else
					{
						if(h>SIZETRACKMAX)
							h=SIZETRACKMAX;
					}

					if(h!=t->sizefactor)
					{
						t->sizefactor=h;
						c++;
					}
				}

				t=t->NextTrack();
			}
		}

		if(c)
		{
			maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_ARRANGE,0);
		}

		SetMouseCursor(CURSOR_UPDOWN);
		return;
	}

	guiObject_Pref *o=tracks->FirstGUIObjectPref();

	while(o)
	{
		switch(o->gobject->id)
		{
		case OBJECTID_ARRANGECHANNEL:
			{
				Edit_Arrange_Channel *c=(Edit_Arrange_Channel *)o->gobject;

				if(c->channel->volumeclicked==true)
				{
					double p=c->channel->io.audioeffects.volume.GetParm(0);
					p*=LOGVOLUME_SIZE;

					int deltay=tracks->deltay;

					// AUDIO
					if(p>AUDIOMIXER_ADD-10 && p<AUDIOMIXER_ADD+10)
					{
						if(deltay>-8 && deltay<8)
							return;

						if(deltay<-1)deltay=-1;
						else
							if(deltay>1)deltay=1;
					}
					else
						if(p>AUDIOMIXER_ADD-15 && p<AUDIOMIXER_ADD+15 )
						{
							if(deltay>-5 && deltay<5)
								return;

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

						tracks->deltareturn=true;

						{
							OSTART atime=GetAutomationTime();

							if(c->channel==&WindowSong()->audiosystem.masterchannel)
							{
								// 1. Master Volume

								double p=c->channel->io.audioeffects.volume.GetParm(0);
								p*=LOGVOLUME_SIZE;

								p+=deltay;

								if(p<0)
									p=0;
								else
									if(p>LOGVOLUME_SIZE)
										p=LOGVOLUME_SIZE;

								double h2=p;
								h2/=LOGVOLUME_SIZE;

								c->channel->io.audioeffects.volume.AutomationEdit(WindowSong(),atime,0,h2);

								return;
							}

							// 2. Check Bus Channels
							AudioChannel *chl=WindowSong()->audiosystem.FirstBusChannel();

							while(chl)
							{
								if(chl==c->channel || chl->IsSelected()==c->channel->IsSelected())
								{
									double p=chl->io.audioeffects.volume.GetParm(0);
									p*=LOGVOLUME_SIZE;

									p+=deltay;

									if(p<0)
										p=0;
									else
										if(p>LOGVOLUME_SIZE)
											p=LOGVOLUME_SIZE;

									double h2=p;
									h2/=LOGVOLUME_SIZE;

									chl->io.audioeffects.volume.AutomationEdit(WindowSong(),atime,0,h2);
								}

								chl=chl->NextChannel();
							}
						}

						return;
				}// Audio

				if(c->channel->channel->m_volumeclicked==true)
				{
					int p=c->channel->MIDIfx.velocity.GetVelocity(),
						deltay=tracks->deltay;

					// MIDI
					if(p>-10 && p<+10)
					{
						if(deltay>-8 && deltay<8)
							return;

						if(deltay<-1)deltay=-1;
						else
							if(deltay>1)deltay=1;
					}
					else
						if(p>-15 && p<+15 )
						{
							if(deltay>-5 && deltay<5)
								return;

							if(deltay<-2)deltay=-2;
							else
								if(deltay>2)deltay=2;
						}
						else
						{
							if(p>-30 && p<+30 )
							{
								if(deltay<-3)deltay=-3;
								else
									if(deltay>3)deltay=3;
							}
						}

						tracks->deltareturn=true;

						TRACE ("DeltaInTracks \n");

						OSTART atime=GetAutomationTime();

						if(c->channel==&WindowSong()->audiosystem.masterchannel)
						{
							// 1. MIDI Volume
							p+=deltay;
							c->channel->SetMIDIVelocity(p,atime);
							return;
						}

						{
							AudioChannel *chl=WindowSong()->audiosystem.FirstBusChannel();

							while(chl)
							{
								if(chl==c->channel || chl->IsSelected()==c->channel->IsSelected())
								{
									int p=chl->MIDIfx.velocity.GetVelocity();
									p+=deltay;
									chl->MIDIfx.SetVelocity(p,atime);
								}

								chl=chl->NextChannel();
							}
						}

						return;

				}// MIDI

			}
			break;

		case OBJECTID_ARRANGETRACK:
			{
				Edit_Arrange_Track *et=(Edit_Arrange_Track *)o->gobject;

				if(et->track->m_volumeclicked==true)
				{
					int p=et->track->t_trackeffects.GetVelocity_NoParent();
					int deltay=tracks->deltay;

					// AUDIO
					if(p>-10 && p<+10)
					{
						if(deltay>-8 && deltay<8)
							return;

						if(deltay<-1)deltay=-1;
						else
							if(deltay>1)deltay=1;
					}
					else
						if(p>-15 && p<+15 )
						{
							if(deltay>-5 && deltay<5)
								return;

							if(deltay<-2)deltay=-2;
							else
								if(deltay>2)deltay=2;
						}
						else
						{
							if(p>-30 && p<+30 )
							{
								if(deltay<-3)deltay=-3;
								else
									if(deltay>3)deltay=3;
							}
						}

						tracks->deltareturn=true;

						TRACE ("DeltaInTracks \n");

						OSTART atime=GetAutomationTime();

						if(et->track->ismetrotrack==true)
						{
							Seq_Track *track=et->track;

							int p=track->t_trackeffects.GetVelocity_NoParent();
							p+=deltay;
							track->t_trackeffects.SetVelocity(p,atime);
						}
						else
						{
							Seq_Track *track=WindowSong()->FirstTrack();

							while(track)
							{
								if(track==et->track || (maingui->GetCtrlKey()==false && track->IsSelected()==true && et->track->IsSelected()==true))
								{
									int p=track->t_trackeffects.GetVelocity_NoParent();
									p+=deltay;
									track->t_trackeffects.SetVelocity(p,atime);
								}

								track=track->NextTrack();
							}
						}

						return;
				}

				if(et->track->volumeclicked==true)
				{
					double p=et->track->io.audioeffects.volume.GetParm(0);
					p*=LOGVOLUME_SIZE;

					int deltay=tracks->deltay;

					// AUDIO
					if(p>AUDIOMIXER_ADD-10 && p<AUDIOMIXER_ADD+10)
					{
						if(deltay>-8 && deltay<8)
							return;

						if(deltay<-1)deltay=-1;
						else
							if(deltay>1)deltay=1;
					}
					else
						if(p>AUDIOMIXER_ADD-15 && p<AUDIOMIXER_ADD+15 )
						{
							if(deltay>-5 && deltay<5)
								return;

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

						tracks->deltareturn=true;

						if(et->track->ismetrotrack==true)
						{
							Seq_Track *track=et->track;

							double p=track->io.audioeffects.volume.GetParm(0);
							p*=LOGVOLUME_SIZE;
							p+=deltay;

							if(p<0)
								p=0;
							else
								if(p>LOGVOLUME_SIZE)
									p=LOGVOLUME_SIZE;

							double h2=p;
							h2/=LOGVOLUME_SIZE;

							track->io.audioeffects.volume.AutomationEdit(WindowSong(),-1,0,h2);
						}
						else
						{
							TRACE ("DeltaInTracks \n");

							OSTART atime=GetAutomationTime();

							Seq_Track *track=WindowSong()->FirstTrack();

							while(track)
							{
								if(track==et->track || (maingui->GetCtrlKey()==false && track->IsSelected()==true && et->track->IsSelected()==true))
								{
									double p=track->io.audioeffects.volume.GetParm(0);
									p*=LOGVOLUME_SIZE;
									p+=deltay;

									if(p<0)
										p=0;
									else
										if(p>LOGVOLUME_SIZE)
											p=LOGVOLUME_SIZE;

									double h2=p;
									h2/=LOGVOLUME_SIZE;

									track->io.audioeffects.volume.AutomationEdit(WindowSong(),atime,0,h2);
								}

								track=track->NextTrack();
							}
						}

						return;
				}

			}
			break;
		}

		o=o->NextGUIObjectPref();
	}
}

void Edit_Arrange::MouseReleaseInTracks(bool leftmouse)
{
	if(mousemode==EM_SIZEY)
	{
		ResetMouseMode(EM_RESET); // default mode
		return;
	}

	if(leftmouse==true)
	{
		if(tracks)
			tracks->horzslider=false;

		guiObject_Pref *o=tracks->FirstGUIObjectPref();
		while(o)
		{
			switch(o->gobject->id)
			{
			case OBJECTID_ARRANGETRACK:
				{
					Edit_Arrange_Track *et=(Edit_Arrange_Track *)o->gobject;


					if(et->track->m_volumeclicked==true)
					{
						et->track->MIDIVolumeUp();
					}

					if(et->track->volumeclicked==true)
					{
						et->track->VolumeUp();
					}
				}
				break;

			case OBJECTID_ARRANGECHANNEL:
				{
					Edit_Arrange_Channel *eac=(Edit_Arrange_Channel *)o->gobject;

					if(eac->channel->volumeclicked==true)
					{
						eac->channel->VolumeUp();
					}

					if(eac->channel->m_volumeclicked==true)
					{
						eac->channel->MIDIVolumeUp();
					}
				}
				break;
			}

			o=o->NextGUIObjectPref();
		}
	}
}

void Edit_Arrange::MouseMoveInTracks()
{
	tracks->InitDelta();

	if(tracks->deltay!=0)
		DeltaInTracks();

	tracks->ExitDelta();
}

void Edit_Arrange::MouseClickInTracks(bool leftmouse)
{
	guiObject *o=tracks->CheckObjectClicked();

	/*
	if(o && maingui->GetShiftKey()==true)
	{
	switch(o->id)
	{
	case OBJECTID_ARRANGEPATTERN:
	o=0;
	break;
	}
	}
	*/

	if(!o)
	{
		editmode=ED_TRACKS;
		SelectAll(false);
		return;
	}

	switch(o->id)
	{
		// Channels
	case OBJECTID_ARRANGECHANNEL:
		{
			Edit_Arrange_Channel *c=(Edit_Arrange_Channel *)o;

			if(leftmouse==true)
			{
				switch(c->channel->audiochannelsystemtype)
				{
				case CHANNELTYPE_BUSCHANNEL:
					WindowSong()->audiosystem.SetFocusBus(c->channel,true);
					break;

				case CHANNELTYPE_MASTER:
					WindowSong()->SetFocusType(Seq_Song::FOCUSTYPE_MASTER);
					WindowSong()->RefreshAudioFocusWindows(0,&WindowSong()->audiosystem.masterchannel,0);
					break;
				}
			}
		}
		break;

	case OBJECTID_ARRANGECHANNELNAME:
		{
			Edit_Arrange_ChannelName *cn=(Edit_Arrange_ChannelName *)o;

			if(leftmouse==true)
			{
				switch(cn->channel->channel->audiochannelsystemtype)
				{
				case CHANNELTYPE_BUSCHANNEL:
					WindowSong()->audiosystem.SetFocusBus(cn->channel->channel,true);
					break;

				case CHANNELTYPE_MASTER:
					WindowSong()->SetFocusType(Seq_Song::FOCUSTYPE_MASTER);
					break;
				}
			}
		}
		break;

	case OBJECTID_ARRANGECHANNELSOLO:
		{
			Edit_Arrange_ChannelSolo *c=(Edit_Arrange_ChannelSolo *)o;
			OSTART automationtime=GetAutomationTime();

			c->channel->channel->SetSolo(c->channel->channel->io.audioeffects.GetSolo()==true?false:true,WindowSong()==mainvar->GetActiveSong()?true:false);
		}
		break;

	case OBJECTID_ARRANGECHANNELMUTE:
		{
			Edit_Arrange_ChannelMute *c=(Edit_Arrange_ChannelMute *)o;

			if(c->channel && c->channel->channel)
			{
				c->channel->channel->ToggleMute();
			}
		}
		break;

	case OBJECTID_ARRANGECHANNELVOLUME:
		{
			Edit_Arrange_ChannelVolume *eat=(Edit_Arrange_ChannelVolume *)o;

			if(leftmouse==true)
			{
				eat->channel->channel->VolumeDown();

				if(eat->channel->channel->volumeclicked==true)
				{
					eat->channel->ShowVolume(true);
					tracks->horzslider=true;
					tracks->InitGetMouseMove();
					tracks->Blt(o);
				}

			}
			else
			{
				if(eat->channel->channel->CanAutomationObjectBeChanged(&eat->channel->channel->io.audioeffects.volume,0,0)==true)
					eat->channel->channel->io.audioeffects.volume.AutomationEdit(WindowSong(),GetAutomationTime(),0,0.5);
			}
			return;
		}
		break;

	case OBJECTID_ARRANGECHANNELMVOLUME:
		{
			Edit_Arrange_ChannelMIDIVolume *eat=(Edit_Arrange_ChannelMIDIVolume *)o;

			if(leftmouse==true)
			{
				eat->channel->channel->MIDIVolumeDown();

				if(eat->channel->channel->m_volumeclicked==true)
				{
					eat->channel->ShowMIDIVolume();
					tracks->horzslider=true;
					tracks->InitGetMouseMove();
					tracks->Blt(o);
				}
			}
			else
			{
				if(eat->channel->channel->CanAutomationObjectBeChanged(&eat->channel->channel->MIDIfx.velocity,0,0)==true)
					eat->channel->channel->MIDIfx.SetVelocity(0,GetAutomationTime());
			}

			return;
		}
		break;

		// Automation
	case OBJECTID_ARRANGEAUTOMATIONTRACKNAME:
		{
			Edit_Arrange_AutomationName *an=(Edit_Arrange_AutomationName *)o;

			WindowSong()->EditAutomation(this,an->track->automationtrack);
		}
		break;

	case OBJECTID_ARRANGEAUTOMATIONTRACKVISIBLE:
		{
			Edit_Arrange_AutomationVisible *sub=(Edit_Arrange_AutomationVisible *)o;

			// Toggle Visible
			if(sub->track->automationtrack->visible==true)
			{
				//if(sub->track->track->activeautomationtrack==sub->track->automationtrack)
				//	sub->track->track->activeautomationtrack=0;

				sub->track->automationtrack->visible=false;
			}
			else
				sub->track->automationtrack->visible=true;

			maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_ARRANGE,0);
			maingui->RefreshAllEditors(WindowSong(),EDITORTYPE_ARRANGELIST,0);

			return;
		}
		break;


	case OBJECTID_ARRANGEAUTOMATIONCHANNELSETTINGS:
		{
			Edit_Arrange_ChannelAutomationSettings *eca=(Edit_Arrange_ChannelAutomationSettings *)o;
			WindowSong()->EditAutomationSettings(this,0,eca->channel->channel);
			return;
		}
		break;

	case OBJECTID_ARRANGEAUTOMATIONCHANNELVISIBLE:
		{
			Edit_Arrange_ChannelAutomation *eca=(Edit_Arrange_ChannelAutomation *)o;

			AudioChannel *channel=eca->channel->channel;
			WindowSong()->ToggleShowAutomationChannels(channel);
			return;
		}
		break;

	case OBJECTID_ARRANGESINGLEAUTOMATIONTRACKVISIBLE:
		{
			Edit_Arrange_AutomationTrackVisible *atv=(Edit_Arrange_AutomationTrackVisible *)o;

			WindowSong()->SetVisibleOfAutomationTrack(atv->track->automationtrack,atv->track->automationtrack->visible==true?false:true);
		}
		break;

	case OBJECTID_ARRANGEAUTOMATIONTRACKMODE:
		{
			Edit_Arrange_AutomationMode *am=(Edit_Arrange_AutomationMode *)o;

			if(am->track->automationtrack->bindtoautomationobject)
			{
				class menu_automode:public guiMenu
				{
				public:
					menu_automode(Seq_Song *s,AutomationTrack *t,int m){song=s;track=t;mode=m;}

					void MenuFunction()
					{
						song->SetAutomationModeOfTracks(track,mode);
					}

					Seq_Song *song;
					AutomationTrack *track;
					int mode;
				};

				if(DeletePopUpMenu(true))
				{	
					int i=0;

					while(automationtrack_mode_names[i])
					{
						popmenu->AddFMenu(automationtrack_mode_names[i],new menu_automode(WindowSong(),am->track->automationtrack,i),i==am->track->automationtrack->automode?true:false);
						i++;
					}

					ShowPopMenu();
				}
			}
		}
		break;

		// TRACK
	case OBJECTID_ARRANGEMVOLUME:
		{
			Edit_Arrange_MIDIVolume *eat=(Edit_Arrange_MIDIVolume *)o;

			if(leftmouse==true)
			{
				eat->track->track->MIDIVolumeDown();

				if(eat->track->track->m_volumeclicked==true)
				{
					eat->track->ShowMIDIVolume();
					eat->Blt();

					tracks->horzslider=true;
					tracks->InitGetMouseMove();
				}
			}
			else
			{
				OSTART atime=GetAutomationTime();

				if(eat->track->track->ismetrotrack==true)
				{
					eat->track->track->t_trackeffects.SetVelocity(0,atime);
				}
				else
				{
					Seq_Track *track=WindowSong()->FirstTrack();

					while(track)
					{
						if(track==eat->track->track || (maingui->GetCtrlKey()==false && track->IsSelected()==true && eat->track->track->IsSelected()==true))
						{
							if(track->t_trackeffects.GetVelocity_NoParent()!=0)
								track->t_trackeffects.SetVelocity(0,atime);
						}

						track=track->NextTrack();
					}
				}
			}
			return;
		}
		break;

	case OBJECTID_ARRANGEAVOLUME:
		{
			Edit_Arrange_Volume *eat=(Edit_Arrange_Volume *)o;

			if(leftmouse==true)
			{
				eat->track->track->VolumeDown();

				if(eat->track->track->volumeclicked==true)
				{
					eat->track->ShowVolume();

					tracks->horzslider=true;
					tracks->InitGetMouseMove();
					tracks->Blt(o);
				}
			}
			else
			{
				if(eat->track->track->ismetrotrack==true)
				{
					eat->track->track->io.audioeffects.volume.AutomationEdit(WindowSong(),-1,0,0.5);
				}
				else
				{
					OSTART atime=GetAutomationTime();

					Seq_Track *track=WindowSong()->FirstTrack();

					while(track)
					{
						if(track==eat->track->track || (maingui->GetCtrlKey()==false && track->IsSelected()==true && eat->track->track->IsSelected()==true))
						{
							track->io.audioeffects.volume.AutomationEdit(WindowSong(),atime,0,0.5);
						}

						track=track->NextTrack();
					}
				}
			}

			return;
		}
		break;


	case OBJECTID_ARRANGESUBCREATESTEP:
		{
			CheckPopMenu(o);
			return;
		}
		break;

	case OBJECTID_ARRANGESUBINFO:
		{
			CheckPopMenu(o);
			return;
		}
		break;

	case OBJECTID_ARRANGEAUTOMATIONTRACK:
		{
			Edit_Arrange_AutomationTrack *eat=(Edit_Arrange_AutomationTrack *)o;

			if(eat->automationtrack->track)
			{

				WindowSong()->SetFocusTrack(eat->automationtrack->track);
			}

			return;
		}
		break;

	case OBJECTID_ARRANGEAUTOMATIONSETTINGS:
		{
			Edit_Arrange_AutomationSettings *eas=(Edit_Arrange_AutomationSettings *)o;
			WindowSong()->EditAutomationSettings(this,eas->track->track,0);
			return;
		}
		break;

	case OBJECTID_ARRANGESHOWAUTOMATIONTRACKS:
		{
			Edit_Arrange_Automation *sub=(Edit_Arrange_Automation *)o;
			Seq_Track *track=sub->track->track;
			WindowSong()->ToggleShowAutomationTracks(track);
			return;
		}
		break;

	case OBJECTID_ARRANGETRACKCHANNELTYPE:
		{
			Edit_Arrange_TrackChannelType *tct=(Edit_Arrange_TrackChannelType *)o;

			if(DeletePopUpMenu(true))
			{
				tct->track->track->CreateChannelTypeMenu(popmenu); 
				ShowPopMenu();
				return;
			}
		}
		break;

	case OBJECTID_ARRANGECHANNELCHANNELTYPE:
		{
			Edit_Arrange_ChannelChannelType *cct=(Edit_Arrange_ChannelChannelType *)o;
			if(DeletePopUpMenu(true))
			{
				cct->channel->channel->CreateChannelTypeMenu(popmenu); 

				ShowPopMenu();
			}
		}break;

	case OBJECTID_ARRANGECHILDOPEN:
		{
			Edit_Arrange_Child *child=(Edit_Arrange_Child *)o;
			child->track->track->ToggleShowChild(leftmouse);
			return;
		}
		break;

	case OBJECTID_ARRANGETRACK:
	case OBJECTID_ARRANGETRACKNAME:
	case OBJECTID_ARRANGETRACKTYPE:
	case OBJECTID_ARRANGEAINPUT:
	case OBJECTID_ARRANGEMINPUT:
		{
			Seq_Track *mousetrack=0;

			switch(o->id)
			{
			case OBJECTID_ARRANGETRACK:
				{
					mousetrack=((Edit_Arrange_Track *)o)->track;

					if(leftmouse==true)
					{
						if(mousetrack->ismetrotrack==true)
							WindowSong()->SetFocusMetroTrack(mousetrack);
						else
							WindowSong()->SetFocusTrack(mousetrack);

						if(maingui->GetShiftKey()==false)
						{
							tracks->InitGetMouseMove();
							SetMouseMode(EM_SIZEY);
						}
					}
				}
				break;

			case OBJECTID_ARRANGETRACKNAME:
				{
					mousetrack=((Edit_Arrange_Name *)o)->track->track;

					if(leftmouse==true)
					{
						if(WindowSong()->SetFocusTrack(mousetrack)==true)
							return;
					}
				}
				break;

			case OBJECTID_ARRANGETRACKTYPE:
				{
					mousetrack=((Edit_Arrange_Type *)o)->track->track;

					if(leftmouse==true)
					{
						//mousetrack->EditMIDIEventOutput(this);

						if(WindowSong()->SetFocusTrack(mousetrack)==true)
							return;
					}
				}
				break;

			case OBJECTID_ARRANGEAINPUT:
				{
					mousetrack=((Edit_Arrange_Input *)o)->track->track;

					if(leftmouse==true)
					{
						if(WindowSong()->SetFocusTrack(mousetrack)==true)
							return;
					}
				}
				break;

			case OBJECTID_ARRANGEMINPUT:
				{
					mousetrack=((Edit_Arrange_MIDIInput *)o)->track->track;

					if(leftmouse==true)
					{
						if(WindowSong()->SetFocusTrack(mousetrack)==true)
							return;
					}
				}
				break;
			}

			if(mousetrack && maingui->GetCtrlKey()==false && maingui->GetShiftKey()==false)
			{
				if(leftmouse==true)
				{
					/*
					if(maingui->GetCtrlKey()==true)
					WindowSong()->SelectTracksFromTo(mousetrack,mousetrack,false,true);
					else
					if(maingui->GetShiftKey()==true)
					WindowSong()->SelectTracksFromTo(WindowSong()->GetFocusTrack(),mousetrack);
					else
					*/
					{
						//if(WindowSong()->SetFocusTrack(mousetrack)==false && WindowSong()->GetFocusTrack()==mousetrack)
						{
							switch(o->id)
							{
							case OBJECTID_ARRANGETRACKTYPE:
								mousetrack->EditMIDIEventOutput(this);
								break;

							case OBJECTID_ARRANGETRACKNAME:
								{
								}
								//RenameTrack(mousetrack,GetMouseX(),GetMouseY());
								break;

							case OBJECTID_ARRANGEMINPUT:
								maingui->EditTrackMIDIInput(this,mousetrack);
								break;

							case OBJECTID_ARRANGEAINPUT:
								maingui->EditTrackInput(this,mousetrack);
								break;
							}
						}
					}
				}
				else
				{
					if(maingui->GetLeftMouseButton()==false)
						PopMenuTrack(mousetrack);
				}
			}
			return;
		}
		break;

	case OBJECTID_ARRANGESOLO:
		{
			Edit_Arrange_Solo *solo=(Edit_Arrange_Solo *)o;
			Seq_Track *track=solo->track->track;
			OSTART automationtime=GetAutomationTime();

			WindowSong()->SoloTrack(track,track->GetSolo()==true?false:true,automationtime);
		}
		break;

	case OBJECTID_ARRANGEMUTE:
		{
			Edit_Arrange_Mute *mute=(Edit_Arrange_Mute *)o;
			Seq_Track *track=mute->track->track;

			if(track)
				track->ToggleMute();
		}
		break;

	case OBJECTID_ARRANGEINPUT:
		{
			Edit_Arrange_AudioInputMonitoring *imon=(Edit_Arrange_AudioInputMonitoring *)o;
			Seq_Track *track=imon->track->track;

			bool im=track->io.inputmonitoring==true?false:true;

			Seq_Track *t=WindowSong()->FirstTrack();

			while(t){

				if(t->IsPartOfEditing(track,true)==true)
					t->SetInputMonitoring(im);

				t=t->NextTrack();
			}
		}
		break;

	case OBJECTID_ARRANGERECORD:
		{
			Edit_Arrange_Record *rec=(Edit_Arrange_Record *)o;

			if(leftmouse==true)
			{
				Seq_Track *track=rec->track->track;
				track->ToggleRecord();
			}
			else
				maingui->EditTrackRecordType(this,rec->track->track);
		}
		break;
	}	
}

void Edit_Arrange::MouseClickInMarker(bool leftmouse)
{
#ifdef MARKEROLDIE
	if(!guiTimeLine)return;

	OSTART mtime=QuantizeEditorMouse(timeline->ConvertXPosToTime(GetMouseX()));

	if(leftmouse==false)
	{
		if(DeletePopUpMenu(true))
		{
			TimeString timestring,timestring_measure;

			WindowSong()->timetrack.CreateTimeString(&timestring,mtime,WindowSong()->project->standardsmpte);
			WindowSong()->timetrack.CreateTimeString(&timestring_measure,mtime,Seq_Pos::POSMODE_NORMAL);

			char *h=mainvar->GenerateString("Marker ",timestring_measure.string,"/",timestring.string);

			if(h)
			{
				popmenu->AddMenu(h,0);
				delete h;
			}

			popmenu->AddLine();

			class menu_createmarker:public guiMenu
			{
			public:
				menu_createmarker(Seq_Song *s,OSTART t,OSTART e){song=s;time=t;end=e;}

				void MenuFunction()
				{
					Seq_Marker *m;

					if(m=song->textandmarker.AddMarker(time,end,"Marker"))
						maingui->RefreshAllEditorsWithMarker(song,m);
				} 

				Seq_Song *song;
				OSTART time,end;
			};

			OSTART end=WindowSong()->timetrack.ConvertTicksToMeasure(mtime);
			end++;
			end=WindowSong()->timetrack.ConvertMeasureToTicks(end);

			popmenu->AddFMenu(Cxs[CXS_CREATEMARKERPATTERN_SINGLE],new menu_createmarker(WindowSong(),mtime,-1));
			popmenu->AddFMenu(Cxs[CXS_CREATEMARKERPATTERN_DOUBLE],new menu_createmarker(WindowSong(),mtime,end));

			class menu_cyclemarker:public guiMenu
			{
			public:
				menu_cyclemarker(EventEditor *ed){editor=ed;}

				void MenuFunction()
				{
					Seq_Marker *m;

					if(m=editor->WindowSong()->textandmarker.AddMarker
						(
						editor->WindowSong()->playbacksettings.cyclestart,
						editor->WindowSong()->playbacksettings.cycleend,
						"Cycle Marker",
						-1
						)
						)
						maingui->RefreshAllEditorsWithMarker(editor->WindowSong(),m);
				} 

				EventEditor *editor;
			};
			popmenu->AddFMenu(Cxs[CXS_CREATENEWMARKERCP],new menu_cyclemarker(this));

			Seq_Marker *c=WindowSong()->textandmarker.FirstMarker();
			while(c)
			{
				if(c->functionflag==Seq_Marker::MARKERFUNC_STOPPLAYBACK)break;
				c=c->NextMarker();
			}

			if(c)
			{
				if(c->GetMarkerStart()!=mtime)
				{
					// Move Song Stop Marker
					popmenu->AddLine();

					class menu_movemarker_sp:public guiMenu
					{
					public:
						menu_movemarker_sp(Seq_Song *s,OSTART t,Seq_Marker *m){song=s;time=t;marker=m;}

						void MenuFunction()
						{
							song->textandmarker.MoveMarker(marker,time);
							maingui->RefreshAllEditorsWithMarker(song,marker);
						} 

						Seq_Song *song;
						Seq_Marker *marker;
						OSTART time;
					};

					popmenu->AddFMenu(Cxs[CXS_CHANGESONGSTOPMARKER],new menu_movemarker_sp(WindowSong(),mtime,c));
				}
			}
			else
			{
				// Create new Song Stop Position Marker
				popmenu->AddLine();

				class menu_createmarker_sp:public guiMenu
				{
				public:
					menu_createmarker_sp(Seq_Song *s,OSTART t){song=s;time=t;}

					void MenuFunction()
					{
						Seq_Marker *m;

						if(m=song->textandmarker.AddMarker(time,-1,"Stop Position"))
						{
							m->functionflag=Seq_Marker::MARKERFUNC_STOPPLAYBACK;
							maingui->RefreshAllEditorsWithMarker(song,m);
						}
					} 

					Seq_Song *song;
					OSTART time;
				};

				popmenu->AddFMenu(Cxs[CXS_CREATEPLAYBACKSTOPMARKER],new menu_createmarker_sp(WindowSong(),mtime));
			}

			ShowPopMenu(GetMouseX(),frame_markermap.y2);
		}
	} // right mouse
	else
	{
		int movetype=0;
		movemarker=0;

		// Find Marker
		Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();

		while(m && m->GetMarkerStart()<endposition)
		{
			int px=timeline->ConvertTimeToX(m->GetMarkerStart()),
				px2=timeline->ConvertTimeToX(m->GetMarkerEnd());

			if(px>0 && GetMouseX()-6<=px && GetMouseX()+6>=px)
			{
				movemarker=m;
				movetype=EM_MOVEMARKER_LEFT;
			}
			else
				if(px2>0 && GetMouseX()-6<=px2 && GetMouseX()+6>=px2)
				{
					movemarker=m;
					movetype=EM_MOVEMARKER_RIGHT;
				}
				else
					if(m->GetMarkerStart()<=mtime && m->GetMarkerEnd()>=mtime)
					{
						movemarker=m;
						modestartposition=mouseposition;
						movemarkerstartposition=m->GetMarkerStart();
						movemarkerendposition=m->GetMarkerEnd();

						movetype=EM_MOVEMARKER;
					}

					m=m->NextMarker();
		}

		if(movemarker)
		{
			SetEditorMode(movetype);
			SetAutoScroll(AUTOSCROLL_TIME_EDITOR,timeline->x,timeline->y,timeline->x2,timeline->y2);
		}
	}

#endif
}

void Edit_Arrange::MouseClickInText(bool leftmouse)
{

}

void Edit_Arrange::MouseClickInSignature(bool leftmouse)
{
	if(timeline)
	{
		OSTART time=timeline->ConvertXPosToTime(signature->GetMouseX());

		switch(mousemode)
		{
		case EM_CREATE:
			if(time>=0)
			{
				time=WindowSong()->timetrack.ConvertTicksToMeasureTicks(time,false);

				Seq_Signature *find=WindowSong()->timetrack.FindSignature(time);

				if(find && find->GetSignatureStart()==time)
					WindowSong()->timetrack.EditSignature(this,find);
				else
					WindowSong()->timetrack.CreateSignatureAndEdit(this,time);
			}
			break;

		case EM_DELETE:
			if(time>=0)
			{
				time=WindowSong()->timetrack.ConvertTicksToMeasureTicks(time,false);

				Seq_Signature *find=WindowSong()->timetrack.FindSignature(time);

				if(find && find->GetSignatureStart()==time)
					WindowSong()->timetrack.DeleteSignature(this,find);
			}
			break;

		default:
			WindowSong()->timetrack.EditSignature(this,time);
			break;
		}
	}
}

Seq_Track *Edit_Arrange::FirstTrack()
{
	if(!tracks)return 0;

	guiObject_Pref *o=tracks->FirstGUIObjectPref();

	while(o)
	{
		switch(o->gobject->id)
		{
		case OBJECTID_ARRANGETRACK:
			{
				Edit_Arrange_Track *et=(Edit_Arrange_Track *)o->gobject;

				if(et->track->ismetrotrack==false)
					return et->track;
			}
			break;
		}

		o=o->NextGUIObjectPref();
	}

	return 0;
}

Seq_Track *Edit_Arrange::LastTrack()
{
	if(!tracks)return 0;

	guiObject_Pref *o=tracks->LastGUIObjectPref();

	while(o)
	{
		switch(o->gobject->id)
		{
		case OBJECTID_ARRANGETRACK:
			{
				Edit_Arrange_Track *et=(Edit_Arrange_Track *)o->gobject;

				if(et->track->ismetrotrack==false)
					return et->track;
			}
			break;
		}

		o=o->PrevGUIObjectPref();
	}

	return 0;
}

Seq_Track *Edit_Arrange::FindTrackAtY(int y)
{
	if(!tracks)return 0;

	guiObject_Pref *o=tracks->FirstGUIObjectPref();

	while(o)
	{
		if(o->gobject->id==OBJECTID_ARRANGETRACK)
		{	
			Edit_Arrange_Track *et=(Edit_Arrange_Track *)o->gobject;

			if(et->track->ismetrotrack==false && et->y<=y && et->y2>=y)
				return et->track; 
		}

		o=o->NextGUIObjectPref();
	}

	return 0;
}

Edit_Arrange_AutomationTrack *Edit_Arrange::FindAutomationTrackAtY(int y)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_ARRANGEAUTOMATIONTRACK)
		{	
			if(o->y<=y && o->y2>=y)
				return ((Edit_Arrange_AutomationTrack *)o); 
		}

		o=o->NextObject();
	}

	return 0;
}

void Edit_Arrange::SetMouseMode(int newmode,AutomationTrack *atrack)
{
	if(!pattern)
		return;

	mouseselectionstarttrack=mouseselectionendtrack=modestarttrack=0;
	moveautomationtrack=modestartautomationtrack=atrack;

	modestartposition=GetMousePosition();

	//	modestartx=pattern->GetMouseX();
	modestarty=pattern->GetMouseY();

	if(modestartposition>=0)
	{
		if(!atrack)
		{
			modestarttrack=FindTrackAtY(modestarty);

			if(!modestarttrack)
			{
				Edit_Arrange_Track *ft=FindTrack(FirstTrack()); 
				//Edit_Arrange_Track *lt=FindTrack(LastTrack());

				if(ft && ft->y>=modestarty)
					modestarttrack=FirstTrack();
				else
					modestarttrack=LastTrack();

			}	
		}
	}

	SetEditorMode(newmode);

	if(newmode!=EM_SIZEY)
	{
		SetAutoScroll(newmode,pattern);
	}
}

void Edit_Arrange::MuteSelected(bool mute)
{
	if(editmode==ED_PATTERN)
	{
		if(Seq_SelectionList *sl=CreateSelectionList())
			mainedit->MutePattern(WindowSong(),sl, 0,mute);

		return;
	}

	if(editmode==ED_TRACKS)
	{
		OSTART automationtime=GetAutomationTime();
		Seq_Track *t=song->FirstTrack();

		while(t)
		{
			if(t->IsSelected()==true)
				song->MuteTrack(t,mute,automationtime);

			t=t->NextTrack();
		}

		return;
	}

}

void Edit_Arrange::InitNewTimeType()
{
	ShowFocusPattern();
}

void Edit_Arrange::SelectAll(bool sel)
{
	if(editmode==ED_PATTERN)
	{
		if(WindowSong()->FirstTrack())
		{
			if(hotkey==true || (maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false))
				SelectAllPattern(sel,0);
		}

		return;
	}

	if(editmode==ED_TRACKS)
	{
		if(sel==true)
			song->SelectTracksFromTo(song->FirstTrack(),song->LastTrack(),false);
		else
			song->UnSelectTracksFromTo(song->FirstTrack(),song->LastTrack());

		return;
	}
}

void Edit_Arrange::Delete()
{
	if(editmode==ED_PATTERN)
	{
		if(Seq_SelectionList *sl=CreateSelectionList())
			mainedit->DeletePattern(WindowSong(),sl,0,false);

		mainedit->DeleteAutomationParameter(WindowSong(),0,0);

		return;
	}

	if(editmode==ED_TRACKS)
	{
		mainedit->DeleteSelectedTracks(WindowSong());
		return;
	}
}

void Edit_Arrange::DragDrop(guiGadget *db)
{
	if(!db)
		return;

	if(db->CheckIf(pattern)==true || db->CheckIf(tracks)==true)
		switch(maingui->dragdropobject->id)
	{
		case OBJ_AUDIOHDFILE:
			{
				AudioHDFile *hdfile=(AudioHDFile *)maingui->dragdropobject;
				DragDropFile(hdfile->GetName(),db);
			}
			break;

		case OBJ_AUDIOREGION:
			{
				/*
				AudioRegion *region=(AudioRegion *)maingui->dragdropobject;

				if(Seq_Track *t=FindTrackAtY(GetMouseY()))
				{
				if(t->IsEditAble()==true)
				{
				OSTART pos=QuantizeEditorMouse(timeline->ConvertXPosToTime(GetMouseX()));
				mainedit->CreateAudioPattern(t,region->r_audiohdfile,region,pos,this);
				}
				}
				*/
			}
			break;
	}
}

void Edit_Arrange::DragDropFile(char *file,guiGadget *db)
{
	if(timeline && pattern==db)
	{
		if(Seq_Track *t=FindTrackAtY(pattern->GetMouseY()))
		{
			if(t->IsEditAble()==true)
			{
				OSTART position=GetMousePosition()>=0?GetMousePosition():WindowSong()->GetSongPosition();

				if(mainedit->LoadSoundFile(this,t,file,position)==false) // 1. Audio
				{
					if(mainedit->LoadMIDIFile(t,file,position,this)==false) //2. MIDI
					{
					}
					else
						UserEdit();
				}
			}
		}
	}
}

void Edit_Arrange::SetStartTrack(Seq_Track *t)
{
	if(!t)
		return;

	OObject *f=trackobjects.FindObject(t);

	if(f)
	{
		if(trackobjects.SetStartY(f->cy))
			DrawDBBlit(tracks,pattern);
	}
	else
	{
		// Track Folder Closed ?
		Seq_Track *p=(Seq_Track *)t->parent;

		while(p)
		{
			if(!p->parent)
				p->ShowAllChildTracks();

			p=(Seq_Track *)p->parent;
		}
	}
}

void Edit_Arrange::SetStartTrack(int index)
{
	if(trackobjects.AddStartY(index)==true)
	{
		tracks->DrawGadgetBlt();
		pattern->DrawGadgetBlt();
	}

	/*
	if(index>=0 && index<WindowSong()->GetCountOfTracks() && index!=firstshowtracknr)
	{
	firstshowtracknr=index;
	tracks->DrawGadgetBlt();
	pattern->DrawGadgetBlt();
	}
	*/
}

void Edit_Arrange::AddStartY(int addy)
{
	SetStartTrack(addy);
}

void Edit_Arrange::AutoScroll()
{
	DoAutoScroll();
	DoStandardYScroll(pattern);
}

void Edit_Arrange::MouseWheel(int delta,guiGadget *db)
{
	if(Edit_MouseWheel(delta)==false)
	{
		if((!db) || db==pattern || db==tracks)
		{
			if(vertgadget)
				vertgadget->DeltaY(delta);
		}
	}
}

#ifdef OLDIE
void Edit_Arrange::EditMouseMoved(bool inside)
{


	if(mousemode==EM_SELECTOBJECTSSTART || mousemode==EM_MOVEPATTERN) // Start->Mouse Recttangle
	{
		if(addpattern)
		{
			SelectPattern(addpattern,true,true);
			addpattern=0;
		}

		if(mousemode==EM_SELECTOBJECTSSTART)
			SetMouseMode(EM_SELECTOBJECTS);

		ShowMovePatternSprites();
	}

	//if(mousemode==EM_MOVEOVERVIEW)
	{
		//	MouseClickInOverview(true);
	}
	//else
	switch(mousemode)
	{
	case EM_MOVEMARKER:
		{
			OSTART diff=GetMousePosition()-modestartposition,cstart=movemarkerstartposition;

			cstart+=diff;

			if(diff && cstart>=0 && cstart!=movemarker->GetMarkerStart())
			{
				movemarker->SetMarkerStartEnd(cstart,movemarkerendposition+diff);
				maingui->RefreshMarkerGUI(WindowSong(),movemarker);
			}
		}
		break;

	case EM_MOVEMARKER_LEFT:
		if(GetMousePosition()!=movemarker->GetMarkerStart() && 
			(GetMousePosition()<movemarker->GetMarkerEnd() || movemarker->markertype==Seq_Marker::MARKERTYPE_SINGLE)
			)
		{
			movemarker->SetMarkerStart(GetMousePosition());
			maingui->RefreshMarkerGUI(WindowSong(),movemarker);
		}
		break;

	case EM_MOVEMARKER_RIGHT:
		if(GetMousePosition()!=movemarker->GetMarkerEnd() && GetMousePosition()>movemarker->GetMarkerStart())
		{
			movemarker->SetMarkerEnd(GetMousePosition());
			maingui->RefreshMarkerGUI(WindowSong(),movemarker);
		}
		break;

	case EM_MOVEAUTOMATION:
		{
			MoveAutomationParameters();
		}
		break;

	case EM_SIZEPATTERN_RIGHT:
	case EM_SIZEPATTERN_LEFT:
		ShowSizePatternSprites();
		//SizePattern(false);
		break;

	default:
		switch(mousemode)
		{
		case EM_SELECT:
			{
			}
			break;
		}// switch
		break;
	}
}
#endif

void Edit_Arrange::UserMessage(int msg,void *par)
{
	switch(msg)
	{
	case MESSAGE_CHECKMOUSEBUTTON:
		{
			if(maingui->GetShiftKey()==false && par)
			{
				// Pattern
				if(pattern)
				{
					guiObject *o=pattern->CheckObjectClicked();

					if(!o)return;

					switch(o->id)
					{
					case OBJECTID_ARRANGEPATTERN:
						{
							Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)o;

							if(eap->pattern==(Seq_Pattern *)par)
								//if(mousemode==EM_SELECTOBJECTSSTART)
							{		
								if(maingui->GetLeftMouseButton()==true)
								{
									// Reset tick diff/track diff
									selection.movediff=0;
									selection.moveobjects_vert=0;

									CreateSelectionList();

									SetMouseMode(EM_MOVEPATTERN);
									ShowMovePatternSprites(true);
								}

							}
						}
						break;
					}
				}
			}
		}
		break;
	}
}

void Edit_Arrange::ExportSelectedPattern(AudioPattern *pt,char *tofile,bool split)
{
	if(pt && tofile && pt->audioevent.audioefile)
	{
		if(AudioFileWork_ExportPatternFile *work=new AudioFileWork_ExportPatternFile){

			work->orgfile=mainvar->GenerateString(pt->audioevent.audioefile->GetName());
			work->tofile=tofile;

			work->from=pt->GetOffSetStart();
			work->to=pt->GetOffSetEnd();

			work->split=split;

			Seq_CrossFade *cf=pt->FirstCrossFade();

			while(cf)
			{
				AudioPattern *p2=(AudioPattern *)cf->connectwith->pattern;

				if(cf->used==true && p2->audioevent.audioefile)
				{
					if(Work_CrossFade *wcf=new Work_CrossFade)
					{
						cf->CopyData(&wcf->fade1);
						cf->connectwith->CopyData(&wcf->fade2);

						wcf->from2=p2->GetOffSetStart();
						wcf->to2=p2->GetOffSetEnd();

						wcf->crossfadesize=wcf->fade1.to_sample-wcf->fade1.from_sample;

						wcf->connectfile=mainvar->GenerateString(p2->audioevent.audioefile->GetName());

						work->wfades.AddEndO(wcf);
					}
				}

				cf=cf->NextCrossFade();
			}

			audioworkthread->AddWork(work);
		}
	}
	else
		if(tofile)delete tofile;
}

void Edit_Arrange::ExportSelectedPattern(bool split)
{
	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		Seq_Track *track=WindowSong()->GetFocusTrack();

		if(track)
		{
			int c=0;
			Seq_Pattern *p=track->FirstPattern(MEDIATYPE_AUDIO);

			while(p)
			{
				if(sl->FindPattern(p))c++;
				p=p->NextPattern(MEDIATYPE_AUDIO);
			}

			if(c)
			{
				camxFile req;

				if(req.SelectDirectory(this,0,Cxs[CXS_EXPORTSELECTEDPATTERN])==true)
				{
					char res[32];
					c=1;

					Seq_Pattern *p=track->FirstPattern(MEDIATYPE_AUDIO);

					while(p)
					{
						if(sl->FindPattern(p))
						{
							char *nr=mainvar->ConvertIntToChar(c,res),
								*fname=mainvar->GenerateString(req.filereqname,"\\",nr,".",p->GetName(),".wav");

							if(fname) // fname deleted in Function
							{
								ExportSelectedPattern((AudioPattern *)p,fname,split);
								c++;
							}
						}

						p=p->NextPattern(MEDIATYPE_AUDIO);
					}

				}
			}
		}
	}
}

void Edit_Arrange::AddEffectsToMenu(AutomationTrack *sub,InsertAudioEffect *iae)
{
#ifdef OLDIE
	int effecttype=iae->audioeffect->audioeffecttype;

	while(iae && iae->audioeffect->audioeffecttype==effecttype)
	{	
		AudioObject *obj=iae->audioeffect;

		if(obj->numberofparameter)
		{
			guiMenu *s=popmenu->AddMenu(iae->audioeffect->GetEffectName(),0);

			// Add Effect Parameter

			class menu_audioeffectparameter:public guiMenu
			{
			public:
				menu_audioeffectparameter(AutomationTrack *sub,AudioObject *ao,int index)
				{
					automationtrack=sub;
					audioobject=ao;
					parameterindex=index;
				}

				void MenuFunction()
				{
					automationtrack->track->ChangeSubTrackToSubTrack(automationtrack,audioobject->CreateAudioObjectTrack(parameterindex));

				} //

				AutomationTrack *automationtrack;
				AudioObject *audioobject;
				int parameterindex;
			};

			for(int i=0;i<obj->numberofparameter;i++)
				s->AddFMenu(obj->GetParmName(i),new menu_audioeffectparameter(sub,obj,i),sub->CompareWithAudio(obj,i)==true);

		}

		iae=iae->NextEffect();
	}
#endif
}

void Edit_Arrange::CreateArrangeAutoMenu(Edit_Arrange_AutomationTrack *track)
{
#ifdef OLDIE
	if(track->automationtrack->staticautomationtrack==true)
	{
	}
	else
	{
		DeletePopUpMenu(true);

		if(popmenu)
		{	
			popmenu->AddMenu(track->track->GetName(),0);
			int c=FX_POPSELECTAUTO_ID;

			// MIDI
			popmenu->AddLine();

			class menu_progchg:public guiMenu
			{
			public:
				menu_progchg(AutomationTrack *t)
				{
					sub=t;
				}

				void MenuFunction()
				{	
					sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_MIDI_ProgramChange());
				} //

				AutomationTrack *sub;
			};

			popmenu->AddFMenu("Program Change",new menu_progchg(track->automationtrack),track->automationtrack->CompareWithMIDI(PROGRAMCHANGE,0,0));

			class menu_pitch:public guiMenu
			{
			public:
				menu_pitch(AutomationTrack *t)
				{
					sub=t;
				}

				void MenuFunction()
				{	
					sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_MIDI_Pitchbend());
				} //

				AutomationTrack *sub;
			};

			popmenu->AddFMenu("Pitchbend",new menu_pitch(track->automationtrack),track->automationtrack->CompareWithMIDI(PITCHBEND,0,0));

			class menu_cpress:public guiMenu
			{
			public:
				menu_cpress(AutomationTrack *t)
				{
					sub=t;
				}

				void MenuFunction()
				{
					sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_MIDI_ChannelPressure());
				} //

				AutomationTrack *sub;
			};

			popmenu->AddFMenu("Channel Pressure",new menu_cpress(track->automationtrack),track->automationtrack->CompareWithMIDI(CHANNELPRESSURE,0,0));

			class menu_ppress:public guiMenu
			{
			public:
				menu_ppress(AutomationTrack *t)
				{
					sub=t;
				}

				void MenuFunction()
				{
					sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_MIDI_PolyPressure(64));
				} //

				AutomationTrack *sub;
			};

			popmenu->AddFMenu("Poly Pressure",new menu_ppress(track->automationtrack),track->automationtrack->CompareWithMIDI(POLYPRESSURE,0,0));

			guiMenu *s;

			class menu_MIDIctrl:public guiMenu
			{
			public:
				menu_MIDIctrl(AutomationTrack *t,UBYTE cn)
				{
					sub=t;
					ctrlnumber=cn;
				}

				void MenuFunction()
				{
					sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_MIDI_Control(ctrlnumber));
				} //

				AutomationTrack *sub;
				UBYTE ctrlnumber;
			};

			bool sel=false;

			for(int i=0;i<31;i++)
			{
				if(track->automationtrack->CompareWithMIDI(CONTROLCHANGE,i,0)==true)
				{
					sel=true;
					break;
				}
			}

			s=popmenu->AddMenu(sel==true?">>> MIDI Control 0-31":"MIDI Control 0-31",c);

			if(s)
				for(int i=0;i<31;i++)
					s->AddFMenu(maingui->ByteToControlInfo(i,-1,true),new menu_MIDIctrl(track->automationtrack,i),track->automationtrack->CompareWithMIDI(CONTROLCHANGE,i,0));

			sel=false;

			for(int i=32;i<63;i++)
			{
				if(track->automationtrack->CompareWithMIDI(CONTROLCHANGE,i,0)==true)
				{
					sel=true;
					break;
				}
			}

			s=popmenu->AddMenu(sel==true?">>> MIDI Control 32-63":"MIDI Control 32-63",c);

			if(s)
				for(int i=32;i<63;i++)
					s->AddFMenu(maingui->ByteToControlInfo(i,-1,true),new menu_MIDIctrl(track->automationtrack,i),track->automationtrack->CompareWithMIDI(CONTROLCHANGE,i,0));

			sel=false;

			for(int i=64;i<95;i++)
			{
				if(track->automationtrack->CompareWithMIDI(CONTROLCHANGE,i,0)==true)
				{
					sel=true;
					break;
				}
			}

			s=popmenu->AddMenu(sel==true?">>> MIDI Control 64-95":"MIDI Control 64-95",c);

			if(s)
			{
				for(int i=64;i<95;i++)
					s->AddFMenu(maingui->ByteToControlInfo(i,true),new menu_MIDIctrl(track->automationtrack,i),track->automationtrack->CompareWithMIDI(CONTROLCHANGE,i,0));
			}

			sel=false;
			for(int i=96;i<127;i++)
			{
				if(track->automationtrack->CompareWithMIDI(CONTROLCHANGE,i,0)==true)
				{
					sel=true;
					break;
				}
			}

			s=popmenu->AddMenu(sel==true?">>> MIDI Control 96-127":"MIDI Control 96-127",c);

			if(s)
			{
				for(int i=96;i<127;i++)
					s->AddFMenu(maingui->ByteToControlInfo(i,-1,true),new menu_MIDIctrl(track->automationtrack,i),track->automationtrack->CompareWithMIDI(CONTROLCHANGE,i,0));
			}

			popmenu->AddLine();
			class menu_tracksolo:public guiMenu
			{
			public:
				menu_tracksolo(AutomationTrack *t)
				{
					sub=t;
				}

				void MenuFunction()
				{
					sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_MIDI_TrackSolo());
				} //

				AutomationTrack *sub;
			};

			popmenu->AddFMenu("Solo",new menu_tracksolo(track->automationtrack),track->automationtrack->CompareWithSolo());

			class menu_trackmute:public guiMenu
			{
			public:
				menu_trackmute(AutomationTrack *t)
				{
					sub=t;
				}

				void MenuFunction()
				{
					sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_MIDI_TrackMute());
				} //

				AutomationTrack *sub;
			};

			popmenu->AddFMenu("Mute",new menu_trackmute(track->automationtrack),track->automationtrack->CompareWithMute()==true);

			popmenu->AddLine();

			// Track Volume
			class menu_trackvolume:public guiMenu
			{
			public:
				menu_trackvolume(AutomationTrack *t){sub=t;}

				void MenuFunction()
				{
					sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_TrackVolume(sub->track));
				} //

				AutomationTrack *sub;
			};
			popmenu->AddFMenu("Audio Track Volume",new menu_trackvolume(track->automationtrack),track->automationtrack->CompareWithAudio(&track->track->io.audioeffects.volume,0));


			AudioChannel *achannel=0;

			if(track->track->GetAudioOut()->FirstChannel())
				achannel=track->track->GetAudioOut()->FirstChannel()->channel;

			if(achannel)
			{
				// Audio ++++++++++++++++++++++++++++++++
				popmenu->AddLine();

				class menu_trackvolume:public guiMenu
				{
				public:
					menu_trackvolume(AutomationTrack *t){sub=t;}

					void MenuFunction()
					{
						sub->track->ChangeSubTrackToSubTrack(sub,new SubTrack_TrackVolume(sub->track));
					} //

					AutomationTrack *sub;
				};
				popmenu->AddFMenu("Audio Track Volume",new menu_trackvolume(track->automationtrack),track->automationtrack->CompareWithAudio(&achannel->io.audioeffects.volume,0));

				class menu_trackaudiovol:public guiMenu
				{
				public:
					menu_trackaudiovol(AutomationTrack *t){sub=t;}

					void MenuFunction()
					{
						//sub->track->ChangeSubTrackToSubTrack(sub,new AutomationTrack_Volume(sub->track->GetAudioOut()->DefaultChannel()));
					} //

					AutomationTrack *sub;
				};
				popmenu->AddFMenu("Audio Channel/s Volume",new menu_trackaudiovol(track->automationtrack),track->automationtrack->CompareWithAudio(&achannel->io.audioeffects.volume,0));

				popmenu->AddMenu("Audio Pan",c++);

#ifdef OLDIE
				if(track->automationtrack->FirstAutomationParameter())
				{
					class menu_deleteallaobjects:public guiMenu
					{
					public:
						menu_deleteallaobjects(Edit_Arrange_AutomationTrack *t){track=t;}

						void MenuFunction()
						{

						} //

						Edit_Arrange_AutomationTrack *track;
					};

					popmenu->AddLine();
					popmenu->AddFMenu(Cxs[CXS_DELETEALLAUTOOBJECTS],new menu_deleteallaobjects(track));
				}
#endif

				/*
				// Audio Instruments
				{
				InsertAudioEffect *iae=achannel->io.audioeffects.FirstActiveAudioInstrument();

				if(iae)
				{	
				popmenu->AddLine();
				AddEffectsToMenu(track->automationtrack,iae);	
				}
				}
				*/

				// Audio Effects AudioChannel/AudioBus
				{
					InsertAudioEffect *iae=achannel->io.audioeffects.FirstInsertAudioEffect();

					if(iae)
					{	
						popmenu->AddLine();
						AddEffectsToMenu(track->automationtrack,iae);	
					}
				}
			}

			ShowPopMenu();
		}
	}

#endif

}

void Edit_Arrange::AddMoveCopySelectedPatternMenu(OSTART p,Seq_Track *t,Seq_Pattern *pattern)
{
	if(popmenu && t)
	{
		if(Seq_SelectionList *sl=CreateSelectionList())
		{
			OSTART fppos=-1;
			bool othertrack=false;
			bool otherpatternposition=false;

			// Get First Pattern Tick Position
			Seq_SelectedPattern *selp=sl->FirstSelectedPattern();

			while(selp)
			{
				if(fppos==-1 || fppos>selp->pattern->GetPatternStart())
					fppos=selp->pattern->GetPatternStart();

				if(selp->pattern->track!=t)
					othertrack=true;

				if(pattern && selp->pattern!=pattern && selp->pattern->GetPatternStart()!=pattern->GetPatternStart())
					otherpatternposition=true;

				selp=selp->NextSelectedPattern();
			}

			if(fppos>=0 && (fppos!=p || othertrack==true || othertrack==true))
			{
				class menu_movespmix:public guiMenu
				{
				public:
					menu_movespmix(Edit_Arrange *ed,OSTART p,bool c,Seq_Track *t){editor=ed;pos=p;copy=c;track=t;}

					void MenuFunction()
					{
						editor->MoveSelectedPatternToMeasureToTick(pos,copy,track);
					} 

					Edit_Arrange *editor;
					OSTART pos;
					Seq_Track *track;
					bool copy;
				};

				popmenu->AddFMenu(Cxs[CXS_COPYSELPATTERNTP],new menu_movespmix(this,p,true,t));
				popmenu->AddFMenu(Cxs[CXS_MOVESELPATTERNTP],new menu_movespmix(this,p,false,t));

				if(othertrack==true)
				{
					popmenu->AddFMenu(Cxs[CXS_COPYSELPATTERNTPNOPOS],new menu_movespmix(this,fppos,true,t));
					popmenu->AddFMenu(Cxs[CXS_MOVESELPATTERNTPNOPOS],new menu_movespmix(this,fppos,false,t));
				}

				if(otherpatternposition==true)
				{
					popmenu->AddFMenu(Cxs[CXS_COPYSELPATTERNTPS],new menu_movespmix(this,pattern->GetPatternStart(),true,0));
					popmenu->AddFMenu(Cxs[CXS_MOVESELPATTERNTPS],new menu_movespmix(this,pattern->GetPatternStart(),false,0));
				}
			}
		}
	}
}

void Edit_Arrange::AddSetSongPositionMenu(Seq_Pattern *p,Seq_Track *t)
{
	class menu_setsongposition:public guiMenu
	{
	public:
		menu_setsongposition(Edit_Arrange *ed,OSTART p)
		{
			editor=ed;
			pos=p;
		}

		void MenuFunction()
		{
			editor->WindowSong()->SetSongPosition(pos,true);
		} 

		Edit_Arrange *editor;
		OSTART pos;
	};

	class menu_seteditorstart:public guiMenu
	{
	public:
		menu_seteditorstart(Edit_Arrange *ed,OSTART p)
		{
			editor=ed;
			pos=p;
		}

		void MenuFunction()
		{
			if(editor->NewStartPosition(pos,true)==true)
			{
				editor->UserEdit();
				editor->SyncWithOtherEditors();
			}
		} 

		Edit_Arrange *editor;
		OSTART pos;
	};


	if(t && t->ismetrotrack==false)
	{
		popmenu->AddLine();
		AddMoveCopySelectedPatternMenu(GetMousePosition(),p?p->track:t,p);
		popmenu->AddLine();
	}

	int setppos=0;

	if(WindowSong()->GetSongPosition()!=GetMousePosition())
	{
		if(WindowSong()->playbacksettings.cycleplayback==false || (WindowSong()->playbacksettings.cycleend>GetMousePosition()))
			setppos++;
	}

	if(p)
	{
		if(WindowSong()->GetSongPosition()!=p->GetPatternStart())
		{
			if(WindowSong()->playbacksettings.cycleplayback==false || (WindowSong()->playbacksettings.cycleend>p->GetPatternStart()))
				setppos++;
		}

		if(WindowSong()->GetSongPosition()!=p->GetPatternEnd())
		{
			if(WindowSong()->playbacksettings.cycleplayback==false || (WindowSong()->playbacksettings.cycleend>p->GetPatternEnd()))
				setppos++;
		}

		if(startposition!=p->GetPatternStart())
			setppos++;
	}

	if(setppos)
	{
		guiMenu *smenu=setppos>1?popmenu->AddMenu(Cxs[CXS_SETSPP],0):popmenu;

		if(smenu)
		{
			if(WindowSong()->GetSongPosition()!=GetMousePosition())
			{
				if(WindowSong()->playbacksettings.cycleplayback==false || (WindowSong()->playbacksettings.cycleend>GetMousePosition()))
				{
					if(setppos==1)
					{
						char *h=mainvar->GenerateString(Cxs[CXS_SETSPP],":",Cxs[CXS_SETMOUSEP]);
						if(h)
						{
							smenu->AddFMenu(h,new menu_setsongposition(this,GetMousePosition()));
							delete h;
						}

					}
					else
						smenu->AddFMenu(Cxs[CXS_SETMOUSEP],new menu_setsongposition(this,GetMousePosition()));
				}
			}

			if(p)
			{
				if(WindowSong()->GetSongPosition()!=p->GetPatternStart())
				{
					if(WindowSong()->playbacksettings.cycleplayback==false || (WindowSong()->playbacksettings.cycleend>p->GetPatternStart()))
					{
						if(setppos==1)
						{
							char *h=mainvar->GenerateString(Cxs[CXS_SETSPP],":",Cxs[CXS_SETSPP_PATTERNSTART]);
							if(h)
							{
								smenu->AddFMenu(h,new menu_setsongposition(this,p->GetPatternStart()));
								delete h;
							}

						}
						else
							smenu->AddFMenu(Cxs[CXS_SETSPP_PATTERNSTART],new menu_setsongposition(this,p->GetPatternStart()));
					}
				}

				if(WindowSong()->GetSongPosition()!=p->GetPatternEnd())
				{
					if(WindowSong()->playbacksettings.cycleplayback==false || (WindowSong()->playbacksettings.cycleend>p->GetPatternEnd()))
					{
						if(setppos==1)
						{
							char *h=mainvar->GenerateString(Cxs[CXS_SETSPP],":",Cxs[CXS_SETSPP_PATTERNEND]);
							if(h)
							{
								smenu->AddFMenu(h,new menu_setsongposition(this,p->GetPatternEnd()));
								delete h;
							}

						}
						else
							smenu->AddFMenu(Cxs[CXS_SETSPP_PATTERNEND],new menu_setsongposition(this,p->GetPatternEnd()));
					}
				}

				if(startposition!=p->GetPatternStart())
				{
					popmenu->AddFMenu(Cxs[CXS_SETEDITOR_PATTERNSTART],new menu_seteditorstart(this,p->GetPatternStart()));
				}
			}
		}
	}
}

void Edit_Arrange::AddActiveTrackMenu(Seq_Track *track)
{
	if(track!=track->song->GetFocusTrack())
	{	
		class menu_focustrack:public guiMenu
		{
		public:
			menu_focustrack(Edit_Arrange *r,Seq_Track *t)
			{
				ar=r;
				track=t;
			}

			void MenuFunction()
			{
				ar->WindowSong()->SetFocusTrack(track);
			} 

			Edit_Arrange *ar;
			Seq_Track *track;
		};
		popmenu->AddFMenu(Cxs[CXS_SETPACTIVETRACK],new menu_focustrack(this,track));
	}
}

void Edit_Arrange::AddPatternMenu(Seq_Pattern *pattern)
{
	InitMousePosition();

	AddActiveTrackMenu(pattern->track);

	class menu_muteonoff:public guiMenu
	{
	public:
		menu_muteonoff(Edit_Arrange *ed,Seq_Pattern *p,bool m){editor=ed;pattern=p;mute=m;}

		void MenuFunction()
		{
			mainedit->MutePattern(editor->WindowSong(),0,pattern,mute);
		} 

		Edit_Arrange *editor;
		Seq_Pattern *pattern;
		bool mute;
	};

	popmenu->AddFMenu(Cxs[pattern->mute==true?CXS_MUTEPOFF:CXS_MUTEPON],new menu_muteonoff(this,pattern,pattern->mute==true?false:true));

	if(pattern->mediatype!=MEDIATYPE_AUDIO_RECORD && pattern->recordpattern==false)
	{
		if(pattern->flag&OFLAG_SELECTED)
		{
			class menu_seloff:public guiMenu
			{
			public:
				menu_seloff(Edit_Arrange *ed,Seq_Pattern *p){editor=ed;pattern=p;}

				void MenuFunction()
				{
					editor->SelectPattern(pattern,false,true);
				} 

				Edit_Arrange *editor;
				Seq_Pattern *pattern;
			};
			popmenu->AddFMenu(Cxs[CXS_UNSELECTP],new menu_seloff(this,pattern));
		}
		else
		{
			class menu_sel:public guiMenu
			{
			public:
				menu_sel(Edit_Arrange *ed,Seq_Pattern *p){editor=ed;pattern=p;}

				void MenuFunction()
				{
					editor->SelectPattern(pattern,true,true);
				} 

				Edit_Arrange *editor;
				Seq_Pattern *pattern;
			};
			popmenu->AddFMenu(Cxs[CXS_SELECTP],new menu_sel(this,pattern));
		}
	}

	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		//popmenu->AddLine();
		/*
		popmenu->AddMenu("Cut selected Pattern",ARR_POPMENU_CUTALL);
		popmenu->AddMenu("Copy selected Pattern",ARR_POPMENU_COPYALL);
		popmenu->AddMenu("Delete selected Pattern",ARR_POPMENU_DELETEALL);
		*/

		/*
		if(sl->FirstSelectedPattern()!=sl->LastSelectedPattern())
		{
		popmenu->AddLine();

		class menu_createfolder:public guiMenu
		{
		public:
		menu_createfolder(Edit_Arrange *ea){editor=ea;}

		void MenuFunction()
		{
		if(Seq_SelectionList *sl=editor->CreateSelectionList())
		{
		mainedit->CreateNewFolder(editor->WindowSong(),sl->FirstPatternPosition(),sl,0);
		}
		} 

		Edit_Arrange *editor;
		};

		popmenu->AddFMenu(Cxs[CXS_CREATEFOLDER],new menu_createfolder(this));
		}
		*/
	}

	{
		class menu_createmarker:public guiMenu
		{
		public:
			menu_createmarker(Seq_Pattern *p,bool doublemk){pattern=p;doublemarker=doublemk;}

			void MenuFunction()
			{
				//	mainedit->CreateNewFolder(editor->WindowSong(),editor->patternselection.FirstPatternPosition(),&editor->patternselection,0);

				Seq_Marker *m;


				if(m=pattern->track->song->textandmarker.AddMarker(pattern,doublemarker))
					maingui->RefreshAllEditorsWithMarker(pattern->track->song,m);
			} 

			Seq_Pattern *pattern;
			bool doublemarker;
		};

		guiMenu *s=popmenu->AddMenu(Cxs[CXS_CREATEMARKERPATTERN],0);

		if(s)
		{
			s->AddFMenu(Cxs[CXS_CREATEMARKERPATTERN_SINGLE],new menu_createmarker(pattern,false));
			s->AddFMenu(Cxs[CXS_CREATEMARKERPATTERN_DOUBLE],new menu_createmarker(pattern,true));
		}

		if(WindowSong()->CheckCyclePositions(pattern->GetPatternStart(),pattern->GetPatternEnd())==false)
		{
			class menu_setcyclepattern:public guiMenu
			{
			public:
				menu_setcyclepattern(Seq_Song *s,Seq_Pattern *p){song=s;pattern=p;}

				void MenuFunction()
				{
					song->SetCycle(pattern->GetPatternStart(),pattern->GetPatternEnd());
				} 

				Seq_Song *song;
				Seq_Pattern *pattern;
			};

			popmenu->AddFMenu(Cxs[CXS_SETCYCLEWITHPATTERNPOSITIONS],new menu_setcyclepattern(WindowSong(),pattern));
		}
	}

	popmenu->AddLine();

	class menu_savepattern:public guiMenu
	{
	public:
		menu_savepattern(Edit_Arrange *ar,Seq_Pattern *p){editor=ar;pattern=p;}

		void MenuFunction()
		{
			pattern->SaveToFile(editor);
		} 

		Edit_Arrange *editor;
		Seq_Pattern *pattern;
	};

	popmenu->AddFMenu(Cxs[CXS_SAVEPATTERN],new menu_savepattern(this,pattern));

	if(pattern->mediatype==MEDIATYPE_MIDI)
	{
		class menu_savepatternMIDI:public guiMenu
		{
		public:
			menu_savepatternMIDI(Edit_Arrange *ar,Seq_Pattern *p){editor=ar;pattern=p;}

			void MenuFunction()
			{
				pattern->SaveToMIDIFile(editor);
			} 

			Seq_Pattern *pattern;
			Edit_Arrange *editor;
		};

		popmenu->AddFMenu(Cxs[CXS_SAVEPATTERNASSMF],new menu_savepatternMIDI(this,pattern));

		MIDIPattern *mp=(MIDIPattern *)pattern;

		if(mp->IsPatternSysExPattern()==true)
		{
			class menu_savepatternsysex:public guiMenu
			{
			public:
				menu_savepatternsysex(Edit_Arrange *ar,MIDIPattern *p){editor=ar;pattern=p;}

				void MenuFunction()
				{
					pattern->SaveToSysExFile(editor);
				} 

				MIDIPattern *pattern;
				Edit_Arrange *editor;
			};

			popmenu->AddFMenu(Cxs[CXS_SAVEPATTERNASSYSEX],new menu_savepatternsysex(this,mp));

		}

	}
}

void Edit_Arrange::RemoveCrossFades(Seq_Pattern *pattern)
{
	Edit_Arrange_Pattern_CrossFade *cf=FirstCrossFade();

	while(cf)
	{
		if(cf->crossfade->pattern==pattern)
		{
			cf=(Edit_Arrange_Pattern_CrossFade *)crossfades.RemoveO(cf);
		}
		else
			cf=cf->NextCF();
	}
}


/*
void Edit_Arrange::PopMenuVUType(Seq_Track *track)
{
if(DeletePopUpMenu(true))
{	
if(track)
{
class menu_trackvutype:public guiMenu
{
public:
menu_trackvutype(Seq_Track *t,int typ)
{
track=t;
type=typ;
}

void MenuFunction()
{
Seq_Track *t=track->song->FirstTrack();

while(t)
{
if(t==track || (maingui->GetCtrlKey()==false && t->IsSelected()==true && track->IsSelected()==true))
{
t->vutype=type;
}

t=t->NextTrack();
}
} //

Seq_Track *track;
int type;
};

popmenu->AddMenu(Cxs[CXS_VUTYPE],0);
popmenu->AddLine();

for(int i=0;i<2;i++)
{
popmenu->AddFMenu(i==Seq_Track::VUTYPE_AUDIO?"Audio":"MIDI",new menu_trackvutype(track,i),track->vutype==i?true:false);
}
}

ShowPopMenu();
}
}
*/

void guiWindow::PopMenuTrack(Seq_Track *track)
{
	if(!track)
		return;

	if(DeletePopUpMenu(true))
	{	
		if(track)
		{
			/*
			class menu_renametrack:public guiMenu
			{
			public:
			menu_renametrack(Edit_Arrange *ar,Seq_Track *t,int x,int y)
			{
			editor=ar;
			track=t;
			xpos=x;
			ypos=y;
			}

			void MenuFunction()
			{
			editor->RenameTrack(track,xpos,ypos);
			} //

			Edit_Arrange *editor;
			Seq_Track *track;
			int xpos,ypos;
			};
			*/

			char h2[NUMBERSTRINGLEN];
			char *tracknr=mainvar->ConvertIntToChar(WindowSong()->GetOfTrack(track)+1,h2);

			if(track->parent)
			{
				if(char *hu=mainvar->GenerateString("Track ",tracknr,":",track->GetName()))
				{
					if(char *h=mainvar->GenerateString(hu,"/",Cxs[CXS_CHILDOF],":",((Seq_Track *)track->parent)->GetName()))
					{
						popmenu->AddMenu(h,0);
						// popmenu->AddFMenu(h,0);
						delete h;
					}

					delete hu;
				}
			}
			else
			{
				if(char *h=mainvar->GenerateString("Track ",tracknr,":",track->GetName()))
				{
					popmenu->AddMenu(h,0);
					delete h;
				}
			}

			if(track->ismetrotrack==true)
				goto show;

			// Track Colour
			popmenu->AddLine();
			class menu_trackcolour:public guiMenu
			{
			public:
				menu_trackcolour(guiWindow *w,Seq_Track *t)
				{
					win=w;
					track=t;
				}

				void MenuFunction()
				{
					track->SelectTrackColour(win);
				} //

				guiWindow *win;
				Seq_Track *track;
			};
			popmenu->AddFMenu(Cxs[CXS_SELECTCOLOUR],new menu_trackcolour(this,track));

			class menu_tcolouroff:public guiMenu
			{
			public:
				menu_tcolouroff(Seq_Track *t)
				{
					track=t;
				}

				void MenuFunction()
				{
					track->SetTrackColourOnOff(track->t_colour.showcolour==true?false:true);
				} //

				Seq_Track *track;
			};

			popmenu->AddFMenu(Cxs[CXS_USECOLOUR],new menu_tcolouroff(track),track->t_colour.showcolour);

			popmenu->AddLine();

			class menu_Cuttrack:public guiMenu
			{
			public:
				menu_Cuttrack(Seq_Track *t){track=t;}

				void MenuFunction()
				{
					mainbuffer->OpenBuffer();
					mainbuffer->AddObjectToBuffer(track,true);
					mainbuffer->CloseBuffer();
					mainedit->DeleteSelectedTracks(track->song,track);
				} //

				Seq_Track *track;
			};
			popmenu->AddFMenu(Cxs[CXS_CUTTRACK],new menu_Cuttrack(track));

			class menu_Copytrack:public guiMenu
			{
			public:
				menu_Copytrack(Seq_Track *t){track=t;}

				void MenuFunction()
				{
					mainbuffer->OpenBuffer();
					mainbuffer->AddObjectToBuffer(track,true);
					mainbuffer->CloseBuffer();
				} //

				Seq_Track *track;
			};
			popmenu->AddFMenu(Cxs[CXS_COPYTRACK],new menu_Copytrack(track));

		}

		if(mainbuffer->CheckHeadBuffer(OBJ_TRACK)==true)
		{
			class menu_pastetrack:public guiMenu
			{
			public:
				menu_pastetrack(guiWindow *w,Seq_Track *t){win=w;track=t;}

				void MenuFunction()
				{
					mainbuffer->PasteBuffer(win,track->song,track,0);
				} //

				guiWindow *win;
				Seq_Track *track;
			};

			popmenu->AddFMenu(Cxs[CXS_PASTETRACK],new menu_pastetrack(this,track));
		}

		if(track)
		{
			class menu_deltrack:public guiMenu
			{
			public:
				menu_deltrack(Seq_Track *t){track=t;}
				void MenuFunction(){mainedit->DeleteSelectedTracks(track->song,track);}
				Seq_Track *track;
			};
			popmenu->AddFMenu(Cxs[CXS_DELETETRACK],new menu_deltrack(track));

			popmenu->AddLine();

			popmenu->AddFMenu(0,new globmenu_cnewtrack(WindowSong(),track));
		}
		else
		{
			popmenu->AddFMenu(0,new globmenu_cnewtrack(WindowSong(),WindowSong()->LastParentTrack()));
		}

		{
			Seq_Track *p=WindowSong()->FirstTrack(),*firstselectedtrack=0;
			while(p)
			{
				//CXS_CREATEPARENTFORSELECTEDTRACKS
				if(p->IsSelected()==true)
				{
					if(p->CanNewChildTrack()==false)
					{
						firstselectedtrack=0;
						break;
					}

					if(!firstselectedtrack)
						firstselectedtrack=p;
				}

				p=p->NextTrack();
			}

			if(firstselectedtrack)
			{
				class menu_cnewparent:public guiMenu
				{
				public:
					menu_cnewparent(Seq_Track *t,Seq_Track *c){track=t;clone=c;}

					void MenuFunction()
					{
						mainedit->CreateNewParentAndAddSelectedTracks(track->song,track,clone);
					} 

					Seq_Track *track,*clone;
				};

				popmenu->AddFMenu(Cxs[CXS_CREATEPARENTFORSELECTEDTRACKS],new menu_cnewparent(firstselectedtrack,track));
			}
		}

		if(track)
		{
			// Selected Track->Child
			if(track->CanNewChildTrack()==true)
			{
				class menu_cnewchild:public guiMenu
				{
				public:
					menu_cnewchild(Seq_Track *t){track=t;}

					void MenuFunction()
					{
						track->song->UnSelectTracksFromTo(track->song->FirstTrack(),track->song->LastTrack());
						mainedit->CreateNewChildTracks(track->song,track,1,EditFunctions::CREATETRACK_ACTIVATE,track);
					} 

					Seq_Track *track;
				};

				popmenu->AddFMenu(Cxs[CXS_CREATENEWCHILDTRACK],new menu_cnewchild(track));

				class menu_caschild:public guiMenu
				{
				public:
					menu_caschild(Seq_Track *t){track=t;}

					void MenuFunction()
					{
						mainedit->AddSelectedTrackToTrackAsChild(track->song,track);
					} //

					Seq_Track *track;
				};

				if(WindowSong()->tracks.CheckIfSelectedCanBeMovedTo(track)==true)
					popmenu->AddFMenu(Cxs[CXS_ADDOTHERTRACKASCHILD],new menu_caschild(track));

				Seq_Track *c=WindowSong()->FirstTrack();
				while(c)
				{
					if(c->parent && c->IsSelected()==true)
						break;

					c=c->NextTrack();
				}

				class menu_removechilds:public guiMenu
				{
				public:
					menu_removechilds(guiWindow *w){win=w;}

					void MenuFunction()
					{
						mainedit->RemoveSelectedTrackFromParent(win->WindowSong());
					} 

					guiWindow *win;
				};

				if(c)
					popmenu->AddFMenu(Cxs[CXS_RELEASECHILDTRACKS],new menu_removechilds(this));

			}

			popmenu->AddLine();

			/*
			class menu_Crstrack:public guiMenu
			{
			public:
			menu_Crstrack(Seq_Track *t){track=t;}

			void MenuFunction()
			{
			AutomationTrack *defautomationtrack=new SubTrack_MIDI_Control(7);

			if(defautomationtrack)
			mainedit->CreateNewSubTrack(track,defautomationtrack,track->LastAutomationTrack());
			} 

			Seq_Track *track;
			};
			popmenu->AddFMenu(Cxs[CXS_CREATENEWAUTOTRACK],new menu_Crstrack(track));
			*/
		}

		//popmenu->AddLine();

		//popmenu->AddFMenu("Mixer",new globmenu_AudioMixer(WindowSong(),track));
		//	popmenu->AddFMenu("Song (Channels/Bus) Mixer",new globmenu_SongMixer(WindowSong(),track));
		//	popmenu->AddFMenu("Song Master Mixer",new globmenu_SongMixer(WindowSong(),0,true));
show:
		ShowPopMenu();
	}
}

void Edit_Arrange::PlayAudioPattern(AudioPattern *pattern,LONGLONG playstartpostion)
{
	if(pattern && pattern->audioevent.audioefile)
	{
		if(playstartpostion>0)
		{
			// Play >>> From Startposition
			if(AudioRegion *r=new AudioRegion(pattern->audioevent.audioefile))
			{
				r->autODeInit=true;

				if(pattern->audioevent.audioregion)
				{
					r->regionstart=pattern->audioevent.audioregion->regionstart+playstartpostion;
					r->regionend=pattern->audioevent.audioregion->regionend;
				}
				else
				{
					// Start Position/End
					r->regionstart=playstartpostion; 
					r->regionend=pattern->audioevent.audioefile->samplesperchannel;
				}

				r->InitRegion();
				pattern->audioevent.audioefile->PlayRealtime(this,WindowSong(),r,0,0,pattern->GetTrack(),true);
			}
		}
		else
		{
			// Play full file
			pattern->audioevent.audioefile->PlayRealtime(this,WindowSong(),pattern->audioevent.audioregion,0,0,pattern->GetTrack(),true);
		}
	}
}

void Edit_Arrange::CreateLinks()
{
	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		if(sl->GetCountOfSelectedPattern()>1)
		{
			if(PatternLink *pl=WindowSong()->CreatePatternLink())
			{
				RemoveLinks(0,false);

				Seq_SelectedPattern *sel=sl->FirstSelectedPattern();

				while(sel)
				{
					pl->AddPattern(sel->pattern);
					maingui->RefreshAllEditorsWithPattern(WindowSong(),sel->pattern);
					sel=sel->NextSelectedPattern();
				}
			}
		}

		maingui->ClearRefresh();
	}
}

void Edit_Arrange::RemoveLinks(Seq_Pattern *p,bool refreshgui)
{
	if(p)
	{
		// Remove From Old Pattern Link
		if(p->link)
		{
			p->link->RemovePattern(p);

			if(refreshgui==true)
			{
				maingui->RefreshAllEditorsWithPattern(WindowSong(),p);
				maingui->ClearRefresh();
			}
		}
	}
	else
	{
		bool closed=false;

		if(Seq_SelectionList *sl=CreateSelectionList())
		{
			Seq_SelectedPattern *sel=sl->FirstSelectedPattern();
			while(sel)
			{
				// Remove From Old Pattern Link
				if(sel->pattern->link || closed==true)
				{
					if(sel->pattern->link && sel->pattern->link->RemovePattern(sel->pattern)==true)
						closed=true;

					if(refreshgui==true)
						maingui->RefreshAllEditorsWithPattern(WindowSong(),sel->pattern);
				}

				sel=sel->NextSelectedPattern();
			}

			maingui->ClearRefresh();
		}
	}
}

void Edit_Arrange::SelectLinks(Seq_Pattern *p)
{
	if(p && p->link)
	{
		PatternLink_Pattern *pl=p->link->FirstLinkedPattern();

		while(pl)
		{
			SelectPattern(pl->pattern,true,true);
			pl=pl->NextLink();
		}
	}
}

void Edit_Arrange::CreateLinkMenu(guiMenu *popmenu,Seq_Pattern *pattern)
{
	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		if(sl->GetCountOfSelectedPattern()>1)
		{
			// Create Link
			class menu_createlink:public guiMenu
			{
			public:
				menu_createlink(Edit_Arrange *e){editor=e;}

				void MenuFunction()
				{
					editor->CreateLinks();
				} //

				Edit_Arrange *editor;
			};

			popmenu->AddFMenu(Cxs[CXS_CREATEPATTERNLINK],new menu_createlink(this));
		}
	}

	// Remove Link Menu ?
	if(pattern && pattern->link)
	{
		char h[255],h2[16];

		strcpy(h,"Pattern Link:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(pattern->link->GetIndex()+1,h2));

		if(guiMenu *sub=popmenu->AddMenu(h,0))
		{
			class menu_removelink:public guiMenu
			{
			public:
				menu_removelink(Edit_Arrange *e,Seq_Pattern *p)
				{
					editor=e;
					pattern=p;
				}

				void MenuFunction()
				{
					editor->RemoveLinks(pattern,true);
				} //

				Edit_Arrange *editor;
				Seq_Pattern *pattern;
			};

			class menu_selectlink:public guiMenu
			{
			public:
				menu_selectlink(Edit_Arrange *e,Seq_Pattern *p)
				{
					editor=e;
					pattern=p;
				}

				void MenuFunction()
				{
					editor->SelectLinks(pattern);
				} //

				Edit_Arrange *editor;
				Seq_Pattern *pattern;
			};

			if(pattern)
				sub->AddFMenu(Cxs[CXS_REMOVEPATTERNLINK],new menu_removelink(this,pattern));

			sub->AddFMenu(Cxs[CXS_REMOVEALLPATTERNLINK],new menu_removelink(this,0));

			if(Seq_SelectionList *sl=CreateSelectionList())
			{
				if(sl->GetCountOfLinkPatternSelectedPattern(pattern->link)!=pattern->link->GetCountOfPattern())
					sub->AddFMenu(Cxs[CXS_SELECTALLPATTERNLINK],new menu_selectlink(this,pattern));
			}

			if(pattern->link)
			{
				sub->AddLine();

				strcpy(h,Cxs[CXS_LINKTOPATTERN]);

				mainvar->AddString(h,mainvar->ConvertIntToChar(pattern->link->GetIndex()+1,h2));
				mainvar->AddString(h,"...");

				sub->AddMenu(h,0);

				PatternLink_Pattern *plp=pattern->link->FirstLinkedPattern();

				while(plp)
				{
					if(plp->pattern!=pattern)
						sub->AddMenu(plp->pattern->GetName(),0);

					plp=plp->NextLink();
				}
			}
		}
	}
}

bool Edit_Arrange::CreatePopMenuCrossFade(Seq_CrossFade *cf)
{
#ifdef OLDIE

	if(DeletePopUpMenu(true))
	{
		popmenu->AddMenu("CrossFade",0);

		TimeString timestring1,timestring2;

		WindowSong()->timetrack.CreateTimeString(&timestring1,cf->from,Seq_Pos::POSMODE_NORMAL);
		WindowSong()->timetrack.CreateTimeString(&timestring2,cf->to,Seq_Pos::POSMODE_NORMAL);

		if(char *h=mainvar->GenerateString(timestring1.string,"<->",timestring2.string))
		{
			popmenu->AddMenu(h,0);
			delete h;
		}

		WindowSong()->timetrack.CreateTimeString(&timestring1,cf->from,WindowSong()->project->standardsmpte);
		WindowSong()->timetrack.CreateTimeString(&timestring2,cf->to,WindowSong()->project->standardsmpte);

		if(char *h=mainvar->GenerateString(timestring1.string,"<:>",timestring2.string))
		{
			popmenu->AddMenu(h,0);
			delete h;
		}

		popmenu->AddLine();

		class menu_enablecrossfade:public guiMenu
		{
		public:
			menu_enablecrossfade(Seq_CrossFade *cf){crossfade=cf;}

			void MenuFunction()
			{
				crossfade->Toggle();
			} //

			Seq_CrossFade *crossfade;
		};
		popmenu->AddFMenu(Cxs[CXS_ENABLED],new menu_enablecrossfade(cf),cf->used);

		popmenu->AddLine();

		if(char *h=mainvar->GenerateString("Pattern [Out -> :",cf->infade==false?cf->pattern->GetName():cf->connectwith->pattern->GetName()))
		{
			popmenu->AddMenu(h,0);
			delete h;
		}

		if(char *h=mainvar->GenerateString("Pattern <- In] :",cf->infade==true?cf->pattern->GetName():cf->connectwith->pattern->GetName()))
		{
			popmenu->AddMenu(h,0);
			delete h;
		}

		class menu_editcrossfade:public guiMenu
		{
		public:
			menu_editcrossfade(Edit_Arrange *ed,Seq_Song *s,Seq_CrossFade *cf){editor=ed;song=s;crossfade=cf;}

			void MenuFunction()
			{
				mainsettings->windowpositions[EDITORTYPE_CROSSFADE].x=editor->GetMouseX();
				mainsettings->windowpositions[EDITORTYPE_CROSSFADE].y=editor->GetMouseY();
				maingui->OpenEditorStart(EDITORTYPE_CROSSFADE,song,0,0,0,(Object *)crossfade,0);
			} //

			Edit_Arrange *editor;
			Seq_Song *song;
			Seq_CrossFade *crossfade;
		};

		popmenu->AddLine();
		popmenu->AddFMenu(Cxs[CXS_EDIT],new menu_editcrossfade(this,WindowSong(),cf->connectwith));

		ShowPopMenu();
	}

	return true;
#endif

	return true;
}

void Edit_Arrange::AddPopFileMenu(Seq_Track *track)
{
	if(track->ismetrotrack==true)
		return;

	InitMousePosition();

	guiMenu *sub=popmenu->AddMenu(Cxs[CXS_LOAD],0);

	if(!sub)return;

	class menu_cnewaf:public guiMenu
	{
	public:
		menu_cnewaf(Edit_Arrange *ed,Seq_Track *t,OSTART pos){editor=ed;track=t;position=pos;}

		void MenuFunction()
		{
			//position=editor->WindowSong()->timetrack.ConvertTicksToMeasureTicks(position,false);
			mainedit->LoadSoundFile(editor,track,0,position);
		} 

		Edit_Arrange *editor;
		Seq_Track *track;
		OSTART position;
	};
	sub->AddFMenu(Cxs[CXS_AUDIOFILE],new menu_cnewaf(this,track,GetMousePosition()));

	class menu_cnewmm:public guiMenu
	{
	public:
		menu_cnewmm(Edit_Arrange *ed,Seq_Track *t,OSTART pos){editor=ed;track=t;position=pos;}

		void MenuFunction()
		{
			//position=editor->WindowSong()->timetrack.ConvertTicksToMeasureTicks(position,false);
			mainedit->LoadMIDIFile(track,0,position,editor);
		} 

		Edit_Arrange *editor;
		Seq_Track *track;
		OSTART position;
	};
	sub->AddFMenu(Cxs[CXS_INSERTSMFFILE],new menu_cnewmm(this,track,GetMousePosition()));

	class menu_issp:public guiMenu
	{
	public:
		menu_issp(Edit_Arrange *ed,Seq_Track *t,OSTART pos){editor=ed;track=t;position=pos;}

		void MenuFunction()
		{
			mainedit->LoadPatternFile(track,0,position,editor);
		} 

		Edit_Arrange *editor;
		Seq_Track *track;
		OSTART position;
	};
	sub->AddFMenu(Cxs[CXS_OPENFILEPATTERN],new menu_issp(this,track,GetMousePosition()));

	{
		guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOMANAGER,0,0);

		if(win)
		{
			Edit_Manager *em=(Edit_Manager *)win;

			if(em->GetActiveHDFile())
			{
				class menu_cnewafh:public guiMenu
				{
				public:
					menu_cnewafh(Edit_Arrange *ed,Seq_Track *t,AudioHDFile *hd,OSTART pos)
					{editor=ed;track=t;audiohdfile=hd;position=pos;}

					void MenuFunction()
					{
						//position=editor->WindowSong()->timetrack.ConvertTicksToMeasureTicks(position,false);
						mainedit->CreateAudioPattern(track,audiohdfile,0,position,editor);
					} 

					AudioHDFile *audiohdfile;
					Edit_Arrange *editor;
					Seq_Track *track;
					OSTART position;
				};

				if(char *h=mainvar->GenerateString(Cxs[CXS_INSERTAAMFILE],em->GetActiveHDFile()->GetFileName(),")"))
				{
					popmenu->AddFMenu(h,new menu_cnewafh(this,track,em->GetActiveHDFile(),GetMousePosition()));
					delete h;
				}

				// Insert Active Region
				if(em->activeregion)
				{
					class menu_cnewafr:public guiMenu
					{
					public:
						menu_cnewafr(Edit_Arrange *ed,Seq_Track *t,AudioHDFile *hd,AudioRegion *r,OSTART pos)
						{editor=ed;track=t;audiohdfile=hd;region=r;position=pos;}

						void MenuFunction()
						{
							//position=editor->WindowSong()->timetrack.ConvertTicksToMeasureTicks(position,false);
							mainedit->CreateAudioPattern(track,audiohdfile,region,position,editor);
						} 

						AudioHDFile *audiohdfile;
						AudioRegion *region;
						Edit_Arrange *editor;
						Seq_Track *track;
						OSTART position;
					};

					if(char *h=mainvar->GenerateString(Cxs[CXS_INSERTAARFILE],em->activeregion->regionname,")"))
					{
						popmenu->AddFMenu(h,new menu_cnewafr(this,track,em->GetActiveHDFile(),em->activeregion,GetMousePosition()));
						delete h;
					}
				}

			}
		}
	}
}

void Edit_Arrange::AddCreatePatternMenu(Seq_Track *track)
{
	if(track->ismetrotrack==true)
		return;

	guiMenu *sub=popmenu->AddMenu(Cxs[CXS_CREATE],0);

	if(!sub)return;

	class menu_cnewpattern:public guiMenu
	{
	public:
		menu_cnewpattern(Edit_Arrange *ed,Seq_Track *t,bool gmadd,bool gsadd,OSTART pos)
		{
			editor=ed;
			track=t;
			addgm=gmadd;
			addgs=gsadd;
			position=pos;
		}

		void MenuFunction()
		{
			Seq_Pattern *patt=mainedit->CreateNewPattern(0,track,MEDIATYPE_MIDI,position,true);

			if(patt)
			{
				if(addgm==true)
					patt->AddGMSysEx(false,true);
				else
					if(addgs==true)
						patt->AddGMSysEx(true,true);
			}
		} 

		Edit_Arrange *editor;
		Seq_Track *track;
		bool addgm,addgs;
		OSTART position;
	};
	sub->AddFMenu(Cxs[CXS_CREATEPATTERN],new menu_cnewpattern(this,track,false,false,GetMousePosition()));

	if(Seq_SelectionList *sl=CreateSelectionList())
	{
		if(Seq_SelectedPattern *p=sl->FirstSelectedPattern())
		{
			Seq_Pattern *f,*f1;
			int realpatternfound=0;

			while(p)
			{
				if(p->pattern->mainpattern==0 || p->pattern->itsaclone==true)
				{
					f=p->pattern;
					f1=p->pattern->itsaclone==true?p->pattern->mainpattern:p->pattern;

					realpatternfound++;
				}

				p=p->NextSelectedPattern();
			}

			if(realpatternfound==1)
			{
				OSTART pos=WindowSong()->timetrack.ConvertTicksToMeasureTicks(GetMousePosition(),false),
					movediff=pos-f->GetPatternStart();

				int trackdiff=
					WindowSong()->GetOfTrack(track)-
					WindowSong()->GetOfTrack(f1->GetTrack());

				class menu_cclonepattern:public guiMenu
				{
				public:
					menu_cclonepattern(Edit_Arrange *ed,OSTART movediff,int tdiff){editor=ed;diff=movediff;trackdiff=tdiff;}

					void MenuFunction()
					{
						MoveO mo;

						mo.song=editor->WindowSong();
						mo.sellist=editor->CreateSelectionList();
						mo.diff=diff;
						mo.index=trackdiff;
						mo.flag=editor->GetMouseQuantizeFlag();

						mainedit->ClonePatternList(&mo);
					} 

					Edit_Arrange *editor;
					OSTART diff;
					int trackdiff;
				};

				if(char *h=mainvar->GenerateString(Cxs[CXS_CREATECLONEPATTERN],":",f->GetName()))
				{
					sub->AddFMenu(h,new menu_cclonepattern(this,movediff,trackdiff));
					delete h;
				}
			}
		}
	}

	sub->AddFMenu(Cxs[CXS_CREATEGMPATTERN],new menu_cnewpattern(this,track,true,false,GetMousePosition()));
	sub->AddFMenu(Cxs[CXS_CREATEGSPATTERN],new menu_cnewpattern(this,track,false,true,GetMousePosition()));

	sub->AddLine();
	sub->AddFMenu(0,new globmenu_cnewtrack(WindowSong(),track));

	popmenu->AddLine();

}

void Edit_Arrange::AddOffsetMenu(Seq_Pattern *pattern)
{
	popmenu->AddLine();

	char *offsetstring="OffSet (-)";

	if(pattern->IfOffSetStart() && pattern->IfOffSetEnd())
		offsetstring="OffSet |> <|";
	else
		if(pattern->IfOffSetStart())
			offsetstring="OffSet |>";
		else
			if(pattern->IfOffSetEnd())
				offsetstring="OffSet <|";

	guiMenu *gm=popmenu->AddMenu(offsetstring,0);
	if(gm)
	{
		if(pattern->IfOffSetStart())
		{
			class menu_resetstart:public guiMenu
			{
			public:
				menu_resetstart(Seq_Pattern *p){pattern=p;}

				void MenuFunction()
				{	
					mainedit->SizePattern(pattern,pattern->GetPatternStart()-pattern->offsetstartoffset,0,false);
				} 

				Seq_Pattern *pattern;
			};

			gm->AddFMenu(Cxs[CXS_CANCELOFFSETSTART],new menu_resetstart(pattern));
		}

		if(pattern->IfOffSetEnd())
		{
			class menu_resetend:public guiMenu
			{
			public:
				menu_resetend(Seq_Pattern *p){pattern=p;}

				void MenuFunction()
				{	
					mainedit->SizePattern(pattern,0,0,true);
				} 

				Seq_Pattern *pattern;
			};

			gm->AddFMenu(Cxs[CXS_CANCELOFFSETEND],new menu_resetend(pattern));
		}

		if(pattern->IfOffSetStart() && pattern->IfOffSetEnd())
		{
			class menu_resetall:public guiMenu
			{
			public:
				menu_resetall(Seq_Pattern *p){pattern=p;}

				void MenuFunction()
				{	
					mainedit->SizePattern(pattern,pattern->GetPatternStart()-pattern->offsetstartoffset,-1,true);
				} 

				Seq_Pattern *pattern;
			};

			gm->AddFMenu(Cxs[CXS_CANCELOFFSETBOTH],new menu_resetall(pattern));
		}

		//	"Set > Start Offset", // CXS_SETOFFSETSTART
		// "Set < End Offset", // CXS_SETOFFSETEND

		gm->AddLine();

		// Set Start
		{
			LONGLONG startoffset=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(pattern->GetPatternStart(),GetMousePosition()-pattern->GetPatternStart());
			startoffset+=pattern->GetOffSetStart();
			bool ok=pattern->SetOffSetStart(GetMousePosition(),startoffset,true);

			if(ok==true)
			{
				class menu_setstart:public guiMenu
				{
				public:
					menu_setstart(Seq_Pattern *p,OSTART pos,LONGLONG off){pattern=p;position=pos;offset=off;}

					void MenuFunction()
					{	
						pattern->InitOffSetEdit(0);
						mainedit->SizePattern(pattern,position,offset,false);
					} 

					Seq_Pattern *pattern;
					OSTART position;
					LONGLONG offset;
				};

				gm->AddFMenu(Cxs[CXS_SETOFFSETSTART],new menu_setstart(pattern,GetMousePosition(),startoffset));
			}
		}

		// Set End
		{
			LONGLONG endoffset=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(pattern->GetPatternStart(),GetMousePosition()-pattern->GetPatternStart());
			endoffset+=pattern->GetOffSetStart();

			bool ok=pattern->SetOffSetEnd(endoffset,true);

			if(ok==true)
			{
				class menu_setend:public guiMenu
				{
				public:
					menu_setend(Seq_Pattern *p,LONGLONG off){pattern=p;offset=off;}

					void MenuFunction()
					{	
						pattern->InitOffSetEdit(0);
						mainedit->SizePattern(pattern,0,offset,true);
					} 

					Seq_Pattern *pattern;
					LONGLONG offset;
				};

				gm->AddFMenu(Cxs[CXS_SETOFFSETEND],new menu_setend(pattern,endoffset));
			}
		}

		//pattern->SetOffSetStart(mouseposition
	}
}

void Edit_Arrange::AddPasteMenu(Seq_Track *track)
{
	InitMousePosition();

	if(mainbuffer->CheckAllObjectsInBuffer(OBJ_PATTERN)==true || mainbuffer->CheckAllObjectsInBuffer(OBJ_EVENTS)==true)
	{
		popmenu->AddLine();

		class menu_pastep:public guiMenu
		{
		public:
			menu_pastep(Edit_Arrange *ed,Seq_Track *t,OSTART p){editor=ed;track=t;position=p;}

			void MenuFunction()
			{
				mainbuffer->PasteBuffer(editor,editor->WindowSong(),track,position);
			} 

			Edit_Arrange *editor;
			Seq_Track *track;
			OSTART position;
		};
		popmenu->AddFMenu(Cxs[CXS_PASTEPATTERN],new menu_pastep(this,track,GetMousePosition()));
	}
	else
		if(mainbuffer->CheckHeadBuffer(OBJ_SELECTPATTERN)==true)
		{
			popmenu->AddLine();

			class menu_pastep:public guiMenu
			{
			public:
				menu_pastep(Edit_Arrange *ed,Seq_Track *t,OSTART p){editor=ed;track=t;position=p;}

				void MenuFunction()
				{
					mainbuffer->PasteBuffer(editor,editor->WindowSong(),track,position);
				} 

				Edit_Arrange *editor;
				Seq_Track *track;
				OSTART position;
			};
			popmenu->AddFMenu(Cxs[CXS_PASTE],new menu_pastep(this,track,GetMousePosition()));
		}
		else
		{
			if(mainbuffer->CheckAllObjectsInBuffer(OBJ_EVENTS)==true)
			{
				popmenu->AddLine();

				class menu_pastep:public guiMenu
				{
				public:
					menu_pastep(Edit_Arrange *ed,Seq_Track *t,OSTART p){editor=ed;track=t;position=p;}

					void MenuFunction()
					{
						mainbuffer->PasteBuffer(editor,editor->WindowSong(),track,position);
					} 

					Edit_Arrange *editor;
					Seq_Track *track;
					OSTART position;
				};
				popmenu->AddFMenu(Cxs[CXS_PASTEEPATTERN],new menu_pastep(this,track,GetMousePosition()));
			}
		}

		/*
		Seq_SelectedPattern *p=patternselection.FirstSelectedPattern();

		if(p)
		{
		bool audiopattern=false;
		bool MIDIPattern=false;

		while(p)
		{
		switch(p->pattern->mediatype)
		{
		case MEDIATYPE_AUDIO:
		audiopattern=true;
		break;

		case MEDIATYPE_MIDI:
		MIDIPattern=true;
		break;
		}

		p=p->NextSelectedPattern();
		}
		}
		*/
}

void Edit_Arrange::AddCopyPastePatternMenu(Seq_Pattern *pattern)
{
	class menu_cutpattern:public guiMenu
	{
	public:
		menu_cutpattern(Seq_Pattern *p){pattern=p;}

		void MenuFunction()
		{
			mainbuffer->OpenBuffer();
			bool ok=mainbuffer->AddObjectToBuffer(pattern,true);
			mainbuffer->CloseBuffer();

			if(ok==true)
				mainedit->DeletePattern(pattern->track->song,0,pattern,false);
		} 

		Seq_Pattern *pattern;
	};
	popmenu->AddFMenu(Cxs[CXS_CUT],new menu_cutpattern(pattern));

	class menu_copypattern:public guiMenu
	{
	public:
		menu_copypattern(Seq_Pattern *p){pattern=p;}

		void MenuFunction()
		{
			mainbuffer->OpenBuffer();
			mainbuffer->AddObjectToBuffer(pattern,true);
			mainbuffer->CloseBuffer();
		} 

		Seq_Pattern *pattern;
	};
	popmenu->AddFMenu(Cxs[CXS_COPY],new menu_copypattern(pattern));

	class menu_delpattern:public guiMenu
	{
	public:
		menu_delpattern(Seq_Pattern *p){pattern=p;}

		void MenuFunction()
		{
			mainedit->DeletePattern(pattern->track->song,0,pattern,false);
		} 

		Seq_Pattern *pattern;
	};
	popmenu->AddFMenu(Cxs[CXS_DELETE],new menu_delpattern(pattern));
	popmenu->AddLine();
}

bool Edit_Arrange::CheckCrossFadeClick()
{
#ifdef OLDIE
	Edit_Arrange_Pattern_CrossFade *cf=FirstCrossFade();

	while(cf)
	{
		if(cf->headery2!=-1)
		{
			if(cf->x<=GetMouseX() && cf->x2>=GetMouseX() && cf->y<=GetMouseY() && cf->headery2>=GetMouseY())
			{
				CreatePopMenuCrossFade(cf->crossfade);
				return true;
			}
		}
		else
		{
			if(cf->x<=GetMouseX() && cf->x2>=GetMouseX() && cf->y<=GetMouseY() && cf->y2>=GetMouseY())
			{
				CreatePopMenuCrossFade(cf->crossfade);
				return true;
			}
		}

		cf=cf->NextCF();
	}
#endif

	return false;
}

void Edit_Arrange::AddPatternAutomationMenu(Seq_Pattern *pt,guiMenu *menu)
{
	popmenu->AddLine();

	guiMenu *s=menu->AddMenu("Automation",0);

	if(s)
	{
		Seq_Track *t=pt->track;

		int c=t->GetCountSelectedAutomationParameters();

		if(c)
		{
			s->AddMenu(Cxs[CXS_BINDSELECTEDAUTOMATIONPARAMETER],0);
		}

	}

	popmenu->AddLine();
}

bool Edit_Arrange::CheckPopMenu(guiObject *o,int flag)
{
	switch(o->id)
	{	
	case OBJECTID_ARRANGESUBCREATESTEP:
		{
			Edit_Arrange_AutomationCreateTicks *type=(Edit_Arrange_AutomationCreateTicks *)o;
			return true;
		}
		break;

	case OBJECTID_ARRANGESUBINFO:
		{
			Edit_Arrange_AutomationInfo *info=(Edit_Arrange_AutomationInfo *)o;

			CreateArrangeAutoMenu(info->track);

			return true;

		}
		break;

#ifdef OLDIE
	case OBJECTID_ARRANGESUBNAME:
		{
			Edit_Arrange_AutomationName *name=(Edit_Arrange_AutomationName *)o;

			/*
			if(name->track->track!=WindowSong()->GetFocusTrack() || name->track->automationtrack->staticautomationtrack==true)
			SetFocusTrack(name->track->track);
			else
			*/
			CreateArrangeAutoMenu(name->track);

			return true;
		}
		break;
#endif

	case OBJECTID_ARRANGEAUTOMATIONTRACK:
		{
			Edit_Arrange_AutomationTrack *track=(Edit_Arrange_AutomationTrack *)o;

#ifdef OLDIE
			if(DeletePopUpMenu(true))
			{	
				popmenu->AddMenu(track->track->GetName(),0);
				popmenu->AddLine();

				if(track->automationtrack->staticautomationtrack==false)
				{
					class menu_createnewsub:public guiMenu
					{
					public:
						menu_createnewsub(AutomationTrack *t){track=t;}

						void MenuFunction()
						{
							Object *clone=track->Clone();

							if(clone)
								mainedit->CreateNewSubTrack(track->track,(AutomationTrack *)clone,track);
#ifdef _DEBUG
							else
								MessageBox(NULL,"Clone SubTrack Error","Error",MB_OK_STYLE);
#endif
						} //

						AutomationTrack *track;
					};
					popmenu->AddFMenu(Cxs[CXS_ADDAUTOTRACK],new menu_createnewsub(track->automationtrack));
				}

				if(track->automationtrack->staticautomationtrack==false)
				{
					class menu_delsub:public guiMenu
					{
					public:
						menu_delsub(AutomationTrack *t){track=t;}

						void MenuFunction()
						{
							mainedit->DeleteAutomationTrack(track);
						} //

						AutomationTrack *track;
					};
					popmenu->AddFMenu(Cxs[CXS_DELETEAUTOTRACK],new menu_delsub(track->automationtrack));	
				}

				ShowPopMenu();
			}

#endif
			return true;
		}
		break;

		/*
		case OBJECTID_FOLDER:
		{
		if(DeletePopUpMenu(true))
		{
		Edit_Arrange_Folder *eaf=(Edit_Arrange_Folder *)o;
		//	popmenu->AddMenu(Cxs[CXS_DELETEFOLDER],ARR_POPMENU_DELETEFOLDER);
		ShowPopMenu();
		}

		return true;
		}
		break;
		*/

	case OBJECTID_ARRANGETRACKNAME:
		{
			Edit_Arrange_Name *n=(Edit_Arrange_Name *)o;
			PopMenuTrack(n->track->track);
			return true;
		}
		break;

	case OBJECTID_ARRANGETRACKTYPE:
		{
			Edit_Arrange_Type *t=(Edit_Arrange_Type *)o;
			PopMenuTrack(t->track->track);
			return true;
		}
		break;

	case OBJECTID_ARRANGETRACK:
		{
			Edit_Arrange_Track *track=(Edit_Arrange_Track *)o;
			PopMenuTrack(track->track);
			return true;
		}
		break;

	case OBJECTID_ARRANGEPATTERN:
	case OBJECTID_ARRANGEPATTERNLISTPATTERN:
		{
			bool subobjectfound=false;

			// Find CrossFades
			switch(o->id)
			{
			case OBJECTID_ARRANGEPATTERN:
				{
					//Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)o;
					subobjectfound=CheckCrossFadeClick();
				}
				break;
			}

			if(subobjectfound==false)
			{
				Seq_Pattern *pattern;

				if(o->id==OBJECTID_ARRANGEPATTERN)
					pattern=((Edit_Arrange_Pattern *)o)->pattern;
				else
					pattern=((Edit_Arrange_PatternList_Pattern *)o)->pattern;

				if(pattern->recordpattern==true)
					return false;

				if(pattern->realpattern==false)
				{
					// track frozen pattern
					if(DeletePopUpMenu(true))
					{	
						InitMousePosition();

						popmenu->AddMenu("# Frozen Track Audio Data #",0);
						popmenu->AddLine();

						class menu_unfreeze:public guiMenu
						{
						public:
							menu_unfreeze(Edit_Arrange *ar,Seq_Track *t)
							{
								arrange=ar;
								track=t;
							}

							void MenuFunction()
							{
								arrange->FreezeTrack(0,false);
							} 

							Edit_Arrange *arrange;
							Seq_Track *track;
						};

						popmenu->AddFMenu("Unfreeze Track",new menu_unfreeze(this,pattern->track));

						//	popmenu->AddLine();
						AddSetSongPositionMenu(pattern,pattern->track);

						ShowPopMenu();
					}
				}
				else 
					if(pattern->visible==true)// real pattern
					{
						int mode;

						switch(windowtimeformat)
						{
						case WINDOWDISPLAY_SMPTE:
							mode=WindowSong()->project->standardsmpte;
							break;

						default:
							mode=Seq_Pos::POSMODE_NORMAL;
							break;
						}

						if(pattern->mainpattern) // Loop
						{
							if(DeletePopUpMenu(true))
							{	
								if(pattern->mainclonepattern)
								{
									if(char *h=mainvar->GenerateString("Clone"," ",Cxs[CXS_OF]," ",pattern->mainpattern->GetName()))
									{
										popmenu->AddMenu(h,0);
										delete h;
									}
								}
								else
								{
									int i=pattern->mainpattern->GetLoopIndex(pattern)+1;
									char h2[NUMBERSTRINGLEN],
										*l=mainvar->GenerateString("Loop (",mainvar->ConvertIntToChar(i,h2),")");

									if(l)
									{
										if(char *h=mainvar->GenerateString(l," ",Cxs[CXS_OF]," ",pattern->mainpattern->GetName()))
										{
											popmenu->AddMenu(h,0);
											delete h;
										}
										delete l;
									}
									//popmenu->AddMenu(">>> Loop <<<",0);
									//popmenu->AddMenu(pattern->GetName(),0);
								}

								AddCreatePatternMenu(pattern->GetTrack());

								if(pattern->mainclonepattern)
								{
									AddCopyPastePatternMenu(pattern);
									AddPasteMenu(pattern->track);
									AddPatternMenu(pattern);

									class menu_delclonepattern:public guiMenu
									{
									public:
										menu_delclonepattern(Seq_Pattern *p){pattern=p;}

										void MenuFunction()
										{
											mainedit->DeletePattern(pattern->track->song,0,pattern,false);
										} 

										Seq_Pattern *pattern;
									};
									popmenu->AddFMenu(Cxs[CXS_DELETECLONEPATTERN],new menu_delclonepattern(pattern));

									class menu_convclonepattern:public guiMenu
									{
									public:
										menu_convclonepattern(Seq_Pattern *p){pattern=p;}

										void MenuFunction()
										{
											mainedit->ConvertCloneToPattern(pattern);	
										} 

										Seq_Pattern *pattern;
									};

									popmenu->AddFMenu(Cxs[CXS_CONVERTCLONETOREAL],new menu_convclonepattern(pattern));

									AddSetSongPositionMenu(pattern,pattern->track);
								}
								else
								{
									class menu_convlooppattern:public guiMenu
									{
									public:
										menu_convlooppattern(Seq_Pattern *p,bool c){pattern=p;convert=c;}

										void MenuFunction()
										{
											mainedit->ConvertLoopToPattern(pattern,convert);	
										} 

										Seq_Pattern *pattern;
										bool convert;
									};

									popmenu->AddFMenu(Cxs[CXS_CONVERTLOOPTOPATTERN],new menu_convlooppattern(pattern,false));

									if(pattern->mainpattern->loopendless==true || pattern->mainpattern->loops>pattern->loopindex+1)
									{
										class menu_setloopend:public guiMenu
										{
										public:
											menu_setloopend(Seq_Pattern *p,int l){pattern=p;loops=l;}

											void MenuFunction()
											{
												mainedit->LoopPattern(pattern,false,true,loops);
											} 

											Seq_Pattern *pattern;
											int loops;
										};

										char h2[32],
											*h=mainvar->GenerateString(Cxs[CXS_SETLOOPEND],"(Loop:",mainvar->ConvertIntToChar(pattern->loopindex+1,h2),")");

										if(h)
										{
											popmenu->AddFMenu(h,new menu_setloopend(pattern->mainpattern,pattern->loopindex+1));
											delete h;
										}
									}

									if(pattern->mediatype==MEDIATYPE_MIDI)
										popmenu->AddFMenu(Cxs[CXS_CONVERTLOOPTOPATTERNANDADDEVENTS],new menu_convlooppattern(pattern,true));

									AddSetSongPositionMenu(pattern,pattern->track);
								}

								ShowPopMenu();
							}
						}//end loop
						else // real pattern
						{
							if(pattern->EditAble()==true && DeletePopUpMenu(true))
							{		
								/*
								class menu_renamepattern:public guiMenu
								{
								public:
								menu_renamepattern(Edit_Arrange *ar,Seq_Pattern *p,int x,int y)
								{
								editor=ar;
								pattern=p;
								xpos=x;
								ypos=y;
								}

								void MenuFunction()
								{
								editor->RenamePattern(pattern,xpos,ypos);
								} //

								Edit_Arrange *editor;
								Seq_Pattern *pattern;
								int xpos,ypos;
								};
								*/

								char *h2=0,*h=0;

								TimeString timestring;
								WindowSong()->timetrack.CreateTimeString(&timestring,GetMousePosition(),mode);
								h2=mainvar->GenerateString("[",timestring.string,"]:");

								switch(pattern->mediatype)
								{
								case MEDIATYPE_MIDI:
									{
										MIDIPattern *mp=(MIDIPattern *)pattern;

										if(mp->IsPatternSysExPattern()==true)
										{
											if(mp->sendonlyatstartup==true)
												h=mainvar->GenerateString("SysEx(STARTUP) ",h2,pattern->GetName());
											else
												h=mainvar->GenerateString("SysEx ",h2,pattern->GetName());
										}
										else
											h=mainvar->GenerateString("MIDI ",h2,pattern->GetName());
									}
									break;

								case MEDIATYPE_AUDIO:
									h=mainvar->GenerateString("Audio ",h2,pattern->GetName());
									break;
								}

								if(h){
									popmenu->AddMenu(h,0 /*new menu_renamepattern(this,pattern,o->x,o->y2)*/ );
									delete h;
									h=0;
								}

								if(h2)
									delete h2;
								h2=0;

								// Start<->End
								{
									char *l=0;
									TimeString timestring1,timestring2;
									WindowSong()->timetrack.CreateTimeString(&timestring1,pattern->GetPatternStart(),mode);
									WindowSong()->timetrack.CreateTimeString(&timestring2,pattern->GetPatternEnd(),mode);

									// Length
									TimeString length;

									if(mode==Seq_Pos::POSMODE_NORMAL || mode==Seq_Pos::POSMODE_COMPRESS)
									{
										length.pos.mode=Seq_Pos::POSMODE_ONLYMEASURE;
										WindowSong()->timetrack.ConvertLengthToPos(&length.pos,pattern->GetPatternStart(),pattern->GetPatternEnd()-pattern->GetPatternStart());
									}
									else
									{
										WindowSong()->timetrack.CreateTimeString(&length,pattern->GetPatternEnd(),mode);
										length.pos.SubPosition(&timestring1.pos);
									}

									length.pos.ConvertToString(WindowSong(),length.string,70);

									h=mainvar->GenerateString(timestring1.string,"<->",timestring2.string," *",length.string);
									if(h)
									{
										popmenu->AddMenu(h,0);
										delete h;
									}

								}

								// Offset
								if(pattern->SizeAble()==true)
									AddOffsetMenu(pattern);

								// Clones ?
								if(pattern->FirstClone())
								{
									popmenu->AddLine();

									char h2[NUMBERSTRINGLEN],
										*c=mainvar->GenerateString("Clones:",mainvar->ConvertIntToChar(pattern->clonepattern.GetCount(),h2));

									if(c){
										popmenu->AddMenu(c,0);
										delete c;
									}
								}

								switch(pattern->mediatype)
								{
								case MEDIATYPE_MIDI:
									{
										MIDIPattern *mp=(MIDIPattern *)pattern;

										if(mp->IsPatternSysExPattern()==true)
										{

											class menu_startup:public guiMenu
											{
											public:
												menu_startup(Seq_Song *s,MIDIPattern *mp,bool sendstart)
												{
													song=s;
													sendonstart=sendstart;
													mpattern=mp;

												}

												void MenuFunction()
												{
													song->SetSysExMIDIPattern(mpattern,sendonstart);
												} //

												Seq_Song *song;
												MIDIPattern *mpattern;
												bool sendonstart;
											};

											popmenu->AddFMenu(Cxs[CXS_SENDTHISPATTERNONLYSTARTUP],new menu_startup(WindowSong(),mp,mp->sendonlyatstartup==true?false:true),mp->sendonlyatstartup);
										}

									}
									break;

								case MEDIATYPE_AUDIO:
									//h=mainvar->GenerateString("Audio ",h2,pattern->GetName());
									break;
								}




								// Pattern Colour
								popmenu->AddLine();
								class menu_pcolour:public guiMenu
								{
								public:
									menu_pcolour(Edit_Arrange *ar,Seq_Pattern *p,int x,int y)
									{
										editor=ar;
										pattern=p;
										xpos=x;
										ypos=y;
									}

									void MenuFunction()
									{
										editor->SetPatternColour(pattern);
									} //

									Edit_Arrange *editor;
									Seq_Pattern *pattern;
									int xpos,ypos;
								};
								popmenu->AddFMenu(Cxs[CXS_SELECTCOLOUR],new menu_pcolour(this,pattern,o->x2,o->y2));

								/*
								if(Seq_SelectionList *sl=CreateSelectionList())
								{
								if(sl->FirstSelectedPattern() && sl->FirstSelectedPattern()->NextSelectedPattern())
								popmenu->AddFMenu(Cxs[CXS_SELPATTERNCOLOURALLP],new menu_pcolour(this,0,o->x2,o->y2));
								}
								*/	

								class menu_pcolouroff:public guiMenu
								{
								public:
									menu_pcolouroff(Edit_Arrange *ar,Seq_Pattern *p)
									{
										editor=ar;
										pattern=p;
									}

									void MenuFunction()
									{
										// Toggle
										editor->ToggleUseColour(pattern);
									} //

									Edit_Arrange *editor;
									Seq_Pattern *pattern;
								};
								popmenu->AddFMenu(Cxs[CXS_USEPATTERNCOLOUR],new menu_pcolouroff(this,pattern),pattern->t_colour.showcolour);


								AddCreatePatternMenu(pattern->GetTrack());
								AddCopyPastePatternMenu(pattern);
								AddPasteMenu(pattern->track);

								/*
								if()
								audio=true;
								else
								audio=patternselection.CheckForMediaType(MEDIATYPE_AUDIO);

								if(epattern->pattern->mediatype&MEDIATYPE_MIDI)
								MIDI=true;
								else
								MIDI=patternselection.CheckForMediaType(MEDIATYPE_MIDI);
								*/

								class menu_eeditor:public guiMenu
								{
								public:
									menu_eeditor(Edit_Arrange *ed,Seq_Pattern *p,OSTART sp){editor=ed;pattern=p;startposition=sp;}

									void MenuFunction()
									{
										if(Seq_SelectionList *sl=editor->CreateSelectionList())
										{
											editor->SelectPatternCheckKeys(pattern,true,true);
											editor->WindowSong()->SetFocusPattern(pattern);

											globmenu_Event editor(editor,startposition);
											editor.MenuFunction();
										}
									} 

									Edit_Arrange *editor;
									Seq_Pattern *pattern;
									OSTART startposition;
								};

								if(GetMousePosition()>=0 && pattern->GetPatternStart()<GetMousePosition() && pattern->GetPatternEnd()>GetMousePosition())
								{
									popmenu->AddLine();
									class menu_xcutpattern:public guiMenu
									{
									public:
										menu_xcutpattern(Edit_Arrange *ed,Seq_Pattern *p,OSTART pos){editor=ed;pattern=p;position=pos;}

										void MenuFunction()
										{
											Seq_SelectionList *sl=editor->CreateSelectionList();

											if(sl)
											{
												if((pattern->flag&OFLAG_SELECTED)==0)
													sl->AddPattern(pattern);
												mainedit->CutPattern(sl,position);
											}
										} 

										Seq_Pattern *pattern;
										Edit_Arrange *editor;
										OSTART position;
									};

									popmenu->AddFMenu(Cxs[CXS_CUTSPLIT],new menu_xcutpattern(this,pattern,GetMousePosition()));
									popmenu->AddLine();
								}

								if(pattern->mediatype&MEDIATYPE_AUDIO)
								{
									AudioPattern *apattern=(AudioPattern *)pattern;

									if(apattern->audioevent.audioefile)
									{
										popmenu->AddFMenu(Cxs[CXS_OPENEVENTEDITOR],new menu_eeditor(this,pattern,GetMousePosition()));

										if(apattern->audioevent.audioefile->errorflag==0)
										{
											class menu_seditor:public guiMenu
											{
											public:
												menu_seditor(Edit_Arrange *ed,AudioPattern *p,OSTART pos){editor=ed;pattern=p;position=pos;}

												void MenuFunction()
												{
													editor->SelectPatternCheckKeys(pattern,true,true);
													editor->WindowSong()->SetFocusPattern(pattern);

													Edit_Sample_StartInit startinit;

													guiWindowSetting settings;
													settings.calledfromwindow=editor;

													startinit.file=pattern->audioevent.audioefile;
													startinit.startposition=position;
													startinit.pattern=pattern;

													if(pattern->audioevent.audioregion)
													{
														startinit.rangestart=pattern->audioevent.audioregion->regionstart;
														startinit.rangeend=pattern->audioevent.audioregion->regionend;
													}

													maingui->OpenEditorStart(EDITORTYPE_SAMPLE,editor->WindowSong(),0,0,&settings,(Object *)&startinit,0);
												} 

												Edit_Arrange *editor;
												AudioPattern *pattern;
												OSTART position;
											};

											popmenu->AddFMenu(Cxs[CXS_OPENAUDIOEDITOR],new menu_seditor(this,apattern,GetMousePosition()));
										}
									}
									CreateLinkMenu(popmenu,pattern);

									if(mainaudio->GetActiveDevice() &&
										apattern->audioevent.audioefile &&
										apattern->audioevent.audioefile->errorflag==0 &&
										WindowSong()==mainvar->GetActiveSong()
										)
									{
										guiMenu *play=popmenu->AddMenu(Cxs[CXS_PLAY],0);

										if(play)
										{
											class menu_play:public guiMenu
											{
											public:
												menu_play(Edit_Arrange *ed,AudioPattern *p,LONGLONG startpos)
												{
													editor=ed;
													pattern=p;
													startposition=startpos;
												}

												void MenuFunction()
												{
													editor->PlayAudioPattern(pattern,startposition);
												} 

												Edit_Arrange *editor;
												AudioPattern *pattern;
												LONGLONG startposition; // ticks
											};
											LONGLONG startoffset;

											if(apattern->GetUseOffSetRegion()==true)
												startoffset=apattern->GetOffSetStart();
											else
												startoffset=0;

											play->AddFMenu(Cxs[CXS_PLAYSOP],new menu_play(this,apattern,startoffset));

											if(o->id!=OBJECTID_ARRANGEPATTERNLISTPATTERN)
											{
												if(GetMousePosition()>=apattern->GetPatternStart() && GetMousePosition()<apattern->GetPatternEnd())
												{
													LONGLONG seek=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(apattern->GetPatternStart(),GetMousePosition()-apattern->GetPatternStart());
													play->AddFMenu(Cxs[CXS_PLAYMP],new menu_play(this,apattern,seek));							
												}
											}

											OSTART spos=WindowSong()->GetSongPosition();

											if(spos>=apattern->GetPatternStart() && spos<apattern->GetPatternEnd())
											{
												LONGLONG seek=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(apattern->GetPatternStart(),spos-apattern->GetPatternStart());
												play->AddFMenu(Cxs[CXS_PLAYSP],new menu_play(this,apattern,seek));							
											}

										}

									}// If Audio Device



									AddPatternAutomationMenu(apattern,popmenu);

#ifdef OLDIE
									if(apattern->volumecurve.FirstVolumeObject())
									{
										class menu_deletevcurve:public guiMenu
										{
										public:
											menu_deletevcurve(AudioPattern *p){pattern=p;}

											void MenuFunction(){
												pattern->volumecurve.DeleteVolumeCurve();
											} 

											AudioPattern *pattern;
										};

										popmenu->AddFMenu(Cxs[CXS_DELETEVOLCU],new menu_deletevcurve(apattern));
									}
									else
									{
										class menu_createvcurve:public guiMenu
										{
										public:
											menu_createvcurve(AudioPattern *p){pattern=p;}

											void MenuFunction(){
												pattern->volumecurve.CreateVolumeCurve();
											} 

											AudioPattern *pattern;
										};

										popmenu->AddFMenu(Cxs[CXS_CREATEVOLCU],new menu_createvcurve(apattern));
									}
#endif


									/*
									class menu_editpvolume:public guiMenu
									{
									public:
									menu_editpvolume(Edit_Arrange *ar,AudioPattern *p){editor=ar,pattern=p;}

									void MenuFunction(){
									pattern->EditVolume(editor);
									} 

									Edit_Arrange *editor;
									AudioPattern *pattern;
									};
									*/

									/*
									{
									char help[128],dbstring[20];
									double conv=mainaudio->ConvertFactorToDb(apattern->GetPatternVolume());

									strcpy(help,apattern->GetPatternVolume()>1?"( +":"( ");
									mainvar->AddString(help,mainvar->ConvertDoubleToChar(conv,dbstring,3));
									mainvar->AddString(help,"dB)");

									char *h=mainvar->GenerateString(Cxs[CXS_AUDIOPATVOL],":",help);

									if(h)
									{
									popmenu->AddFMenu(h,new menu_editpvolume(this,apattern));
									delete h;
									}
									}
									*/

									/*
									if(apattern->GetPatternVolume()!=1)
									{
									class menu_resetpvolume:public guiMenu
									{
									public:
									menu_resetpvolume(AudioPattern *p){pattern=p;}

									void MenuFunction(){
									pattern->p_patternvolume=1; // +-0db
									} 

									AudioPattern *pattern;
									};

									char help[128]; // hack
									char dbstring[20];

									double conv=mainaudio->ConvertFactorToDb(apattern->GetPatternVolume());

									strcpy(help,Cxs[CXS_RESETAUDIOPVOL]);
									mainvar->AddString(help,apattern->GetPatternVolume()>1?"( +":"( ");
									mainvar->AddString(help,mainvar->ConvertDoubleToChar(conv,dbstring,3));
									mainvar->AddString(help,"dB)");

									popmenu->AddFMenu(help,new menu_resetpvolume(apattern));
									}
									*/

									class menu_replace:public guiMenu
									{
									public:
										menu_replace(Edit_Arrange *e,AudioPattern *p){editor=e;pattern=p;}

										void MenuFunction(){
											pattern->SelectAudioFile(editor,0);
										} 

										Edit_Arrange *editor;
										AudioPattern *pattern;
									};

									popmenu->AddLine();

									if(apattern->audioevent.audioefile)
									{
										if(apattern->audioevent.audioregion)
										{
											class menu_saveregion:public guiMenu
											{
											public:
												menu_saveregion(Edit_Arrange *ar,AudioHDFile *file,AudioRegion *r)
												{
													editor=ar;
													audiohdfile=file;
													region=r;
												}

												void MenuFunction(){
													mainaudio->CreateAudioRegionFile(editor,audiohdfile,region);
												}

												Edit_Arrange *editor;
												AudioHDFile *audiohdfile;
												AudioRegion *region;
											};

											popmenu->AddFMenu(Cxs[CXS_EXSAVEREGION],new menu_saveregion(this,apattern->audioevent.audioefile,apattern->audioevent.audioregion));
										}
										else
										{
											class menu_savefile:public guiMenu
											{
											public:
												menu_savefile(Edit_Arrange *ar,AudioPattern *ap){editor=ar;audiopattern=ap;}

												void MenuFunction(){
													mainaudio->CopyAudioFile(editor,audiopattern->audioevent.audioefile,audiopattern);
												}

												Edit_Arrange *editor;
												AudioPattern *audiopattern;
												AudioHDFile *audiohdfile;
											};

											popmenu->AddFMenu(Cxs[CXS_EXSAVEAFILE],new menu_savefile(this,apattern));
										}
									}

									if(apattern->GetUseOffSetRegion()==false && apattern->audioevent.audioregion==0) // No Offset, not Region ?
										popmenu->AddFMenu(Cxs[CXS_EXSELECTAFILE],new menu_replace(this,apattern));

									OSTART start=-1,end=-1;

									if(apattern->GetPatternStart()<WindowSong()->playbacksettings.cycleend &&
										apattern->GetPatternEnd()>=WindowSong()->playbacksettings.cycleend)
									{
										start=apattern->GetPatternStart()>WindowSong()->playbacksettings.cyclestart?apattern->GetPatternStart():WindowSong()->playbacksettings.cyclestart;
										end=WindowSong()->playbacksettings.cycleend;
									}
									else
										if(apattern->GetPatternStart()<WindowSong()->playbacksettings.cyclestart &&
											apattern->GetPatternEnd()>WindowSong()->playbacksettings.cyclestart)
										{
											start=WindowSong()->playbacksettings.cyclestart;
											end=apattern->GetPatternEnd()<WindowSong()->playbacksettings.cycleend?apattern->GetPatternEnd():WindowSong()->playbacksettings.cycleend;
										}

										if(start!=-1 && end!=-1)
										{
											LONGLONG sstart=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(apattern->GetPatternStart(),start-apattern->GetPatternStart()),
												send=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(apattern->GetPatternStart(),end-apattern->GetPatternStart());

											class menu_createregioncycle:public guiMenu
											{
											public:
												menu_createregioncycle(AudioPattern *p,LONGLONG s,LONGLONG e)
												{
													audiopattern=p;
													start=s;
													end=e;
												}

												void MenuFunction()
												{
													if(AudioRegion *ar=new AudioRegion(audiopattern->audioevent.audioefile))
													{
														ar->regionstart=start; 
														ar->regionend=end;
														ar->InitRegion();
														audiopattern->audioevent.audioefile->AddRegion(ar);
														maingui->RefreshRegionGUI(ar->r_audiohdfile);
													}
												}

												AudioPattern *audiopattern;
												LONGLONG start,end;
											};

											if(apattern->audioevent.audioefile->FindRegion(sstart,send)==0)
												popmenu->AddFMenu(Cxs[CXS_CREATEREGIONCYLCE],new menu_createregioncycle(apattern,sstart,send));
										}

										{
											guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOMANAGER,0,0);

											if(win)
											{
												Edit_Manager *em=(Edit_Manager *)win;

												if(em->GetActiveHDFile() && apattern->GetUseOffSetRegion()==false && apattern->audioevent.audioregion==0) // No Offset, not Region ?)
												{
													if(apattern->audioevent.audioefile && em->GetActiveHDFile()!=apattern->audioevent.audioefile)
													{
														class menu_selectmanager:public guiMenu
														{
														public:
															menu_selectmanager(Edit_Manager *m,AudioHDFile *file)
															{
																editor=m;
																audiohdfile=file;
															}

															void MenuFunction()
															{
																editor->SetActiveFile(editor->FindAudioHDFile(audiohdfile),true);
																editor->WindowToFront(false);
															}

															AudioHDFile *audiohdfile;
															Edit_Manager *editor;
														};

														if(apattern->GetUseOffSetRegion()==false && apattern->audioevent.audioregion==0) // No Offset, not Region ?
															popmenu->AddFMenu(Cxs[CXS_SELFILEINSIDEMANAGER],new menu_selectmanager(em,apattern->audioevent.audioefile));
													}

													if(em->GetActiveHDFile()!=apattern->audioevent.audioefile)
													{
														class menu_cnewafh:public guiMenu
														{
														public:
															menu_cnewafh(Edit_Arrange *ed,AudioPattern *ap,AudioHDFile *hd){editor=ed;audiopattern=ap;audiohdfile=hd;}

															void MenuFunction()
															{
																mainedit->ReplaceAudioPatternHDFile(audiopattern,audiohdfile);
															} 

															AudioHDFile *audiohdfile;
															Edit_Arrange *editor;
															AudioPattern *audiopattern;
														};

														if(char *h=mainvar->GenerateString(Cxs[CXS_SELFAMFILE],em->GetActiveHDFile()->GetFileName(),Cxs[CXS_SELFAMFILE2]))
														{
															popmenu->AddFMenu(h,new menu_cnewafh(this,apattern,em->GetActiveHDFile()));
															delete h;
														}
													}

													// Insert Active Region
													if(em->activeregion && apattern->audioevent.audioregion!=em->activeregion)
													{
														class menu_cnewafr:public guiMenu
														{
														public:
															menu_cnewafr(Edit_Arrange *ed,AudioPattern *ap,AudioHDFile *hd,AudioRegion *r)
															{editor=ed;audiopattern=ap;audiohdfile=hd;region=r;}

															void MenuFunction()
															{
																mainedit->ReplaceAudioPatternHDFile(audiopattern,audiohdfile,region);
															} 

															AudioHDFile *audiohdfile;
															AudioRegion *region;
															Edit_Arrange *editor;
															AudioPattern *audiopattern;
														};

														if(char *h=mainvar->GenerateString(Cxs[CXS_SELFAMREGION],em->activeregion->regionname,Cxs[CXS_SELFAMFILE2]))
														{
															popmenu->AddFMenu(h,new menu_cnewafr(this,apattern,em->GetActiveHDFile(),em->activeregion));
															delete h;
														}
													}
												}
											}

										}
								}// if AUDIO

								if(pattern->mediatype&MEDIATYPE_MIDI)
								{
									MIDIPattern *mp=(MIDIPattern *)pattern;

									if(guiMenu *s=popmenu->AddMenu("Editor",0))
									{
										class menu_peditor:public guiMenu
										{
										public:
											menu_peditor(Edit_Arrange *ed,MIDIPattern *p,OSTART sp){editor=ed;pattern=p;startposition=sp;}

											void MenuFunction()
											{
												if(pattern)
												{
													editor->SelectPatternCheckKeys(pattern,true,true);
													editor->WindowSong()->SetFocusPattern(pattern);
												}

												globmenu_Piano gpiano(editor,startposition);
												gpiano.MenuFunction();
											} 

											Edit_Arrange *editor;
											MIDIPattern *pattern;
											OSTART startposition;
										};

										s->AddFMenu("Event",new menu_eeditor(this,mp,GetMousePosition()));
										s->AddFMenu("Piano",new menu_peditor(this,mp,GetMousePosition()));

										class menu_weditor:public guiMenu
										{
										public:
											menu_weditor(Edit_Arrange *ed,MIDIPattern *p,OSTART sp){editor=ed;pattern=p;startposition=sp;}

											void MenuFunction()
											{
												guiWindowSetting settings;
												settings.screen=editor->screen;
												settings.calledfromwindow=editor;

												editor->SelectPatternCheckKeys(pattern,true,true);
												editor->WindowSong()->SetFocusPattern(pattern);

												if(Seq_SelectionList *sl=editor->CreateSelectionList())
												{
													sl->openstartposition=startposition;
													maingui->OpenEditorStart(EDITORTYPE_WAVE,editor->WindowSong(),NULL,sl,&settings,0,startposition);
												}
											} 

											Edit_Arrange *editor;
											MIDIPattern *pattern;
											OSTART startposition;
										};
										s->AddFMenu("Wave",new menu_weditor(this,mp,GetMousePosition()));

										class menu_deditor:public guiMenu
										{
										public:
											menu_deditor(Edit_Arrange *ed,MIDIPattern *p,OSTART sp){editor=ed;pattern=p;startposition=sp;}

											void MenuFunction()
											{
												editor->SelectPatternCheckKeys(pattern,true,true);
												editor->WindowSong()->SetFocusPattern(pattern);

												if(Seq_SelectionList *sl=editor->CreateSelectionList())
												{
													guiWindowSetting settings;
													settings.screen=editor->screen;
													settings.calledfromwindow=editor;

													sl->openstartposition=startposition;
													maingui->OpenEditorStart(EDITORTYPE_DRUM,editor->WindowSong(),NULL,sl,&settings,0,startposition);
												}
											} 

											Edit_Arrange *editor;
											MIDIPattern *pattern;
											OSTART startposition;
										};
										s->AddFMenu("Drum",new menu_deditor(this,mp,GetMousePosition()));

#ifdef OLDIE
										class menu_seditor:public guiMenu
										{
										public:
											menu_seditor(Edit_Arrange *ed,MIDIPattern *p){editor=ed;pattern=p;}

											void MenuFunction()
											{
												guiWindowSetting settings;
												settings.screen=editor->screen;
												settings.calledfromwindow=editor;

												editor->SelectPatternCheckKeys(pattern,true,true);
												editor->WindowSong()->SetFocusPattern(pattern);

												if(Seq_SelectionList *sl=editor->CreateSelectionList())
												{
													sl->openstartposition=editor->GetMousePosition();
													maingui->OpenEditorStart(EDITORTYPE_SCORE,editor->WindowSong(),NULL,sl,&settings,0,editor->GetMousePosition());
												}
											} 

											Edit_Arrange *editor;
											MIDIPattern *pattern;
										};
										s->AddFMenu("Score",new menu_seditor(this,mp));
#endif

									}

									CreateLinkMenu(popmenu,pattern);

									popmenu->AddLine();

									// Groove
									{
										Note *fnote=0,*lnote=0;
										Seq_Event *e=mp->FirstEvent();

										while(e && (!fnote))
										{
											if(e->GetStatus()==NOTEON)
												fnote=(Note *)e;

											e=e->NextEvent();
										}

										e=mp->LastEvent();

										while(e && (!lnote))
										{
											if(e->GetStatus()==NOTEON)
												lnote=(Note *)e;

											e=e->PrevEvent();
										}

										if(fnote && fnote!=lnote)
										{
											OSTART diff=lnote->GetEventStart()-fnote->GetEventStart();

											class menu_cgroove:public guiMenu
											{
											public:
												menu_cgroove(MIDIPattern *p,OSTART d,int qr){pattern=p;qdiff=d;quantraster=qr;}

												void MenuFunction()
												{
													OSTART s=quantlist[quantraster],steps=qdiff/s;

													if(steps<16)
														steps=16; // 16 q steps minimum
													else
														if(steps>32)
															steps=32;

													Groove *n=mainMIDI->AddGroove(steps,quantraster); // default

													if(n)
													{
														n->ConvertMIDIPatternToGroove(pattern);
														mainMIDI->RefreshGrooveGUI(n);
													}
												} 

												MIDIPattern *pattern;
												OSTART qdiff;
												int quantraster;
											};

											if(guiMenu *g=popmenu->AddMenu(Cxs[CXS_CREATEGROOVE],0))
											{
												for(int i=0;i<QUANTNUMBER;i++)
													g->AddFMenu(quantstr[i],new menu_cgroove(mp,diff,i));
											}
										}
									}

									if(mp->CheckPatternChannel()==0 && mp->GetCountOfEvents()!=mp->GetCountOfSysEx()) // splitable ?
									{
										class menu_psplit:public guiMenu
										{
										public:
											menu_psplit(MIDIPattern *p){pattern=p;}

											void MenuFunction()
											{
												mainedit->SplitPatternToChannels(pattern);
											} 

											MIDIPattern *pattern;
										};
										popmenu->AddFMenu(Cxs[CXS_SPLITPTOCHL],new menu_psplit(mp));
									}

									MIDIPatternInfo info;
									mp->CreatePatternInfo(&info);

									if(info.DifferentEventTypes()>1) // splitable
									{
										class menu_petsplit:public guiMenu
										{
										public:
											menu_petsplit(MIDIPattern *p){pattern=p;}

											void MenuFunction()
											{
												mainedit->SplitPatternToEvents(pattern);
											} 

											MIDIPattern *pattern;
										};

										popmenu->AddFMenu(Cxs[CXS_SPLITPTOET],new menu_petsplit(mp));
									}

									if(info.notes>1) // min 2. notes
									{
										class menu_flip:public guiMenu
										{
										public:
											menu_flip(MIDIPattern *p){pattern=p;}

											void MenuFunction()
											{
												mainedit->FlipPattern(pattern);
											} 

											MIDIPattern *pattern;
										};

										popmenu->AddFMenu(Cxs[CXS_ROTATEPATTERN],new menu_flip(mp));
									}

									if(pattern->FirstLoopPattern())
									{
										// Loops ?
										class menu_addloops:public guiMenu
										{
										public:
											menu_addloops(MIDIPattern *p){pattern=p;}

											void MenuFunction()
											{
												mainedit->ConvertAllLoopsToPattern(pattern);	
											} 

											MIDIPattern *pattern;
										};

										popmenu->AddFMenu(Cxs[CXS_CONLATOP],new menu_addloops(mp));
									}

									if(Seq_SelectionList *sl=CreateSelectionList())
									{
										if(sl->GetCountOfSelectedPattern()>1 || (sl->GetCountOfSelectedPattern()==1 && sl->FindPattern(mp)==0))
										{
											bool checkok=false;

											Seq_SelectedPattern *slp=sl->FirstSelectedPattern();

											while(slp)
											{
												if(slp->pattern!=mp && slp->pattern->mediatype==mp->mediatype)
													checkok=true;

												slp=slp->NextSelectedPattern();
											}

											if(checkok==true)
											{
												popmenu->AddLine();
												// Mix Pattern ?
												class menu_mixp:public guiMenu
												{
												public:
													menu_mixp(Seq_SelectionList *l,MIDIPattern *p,bool a)
													{
														list=l;
														pattern=p;
														add=a;
													}

													void MenuFunction()
													{
														//	mainedit->ConvertAllLoopsToPattern(pattern);	
														mainedit->MixAllSelectedPatternToPattern(list,pattern,add);	
													} 

													Seq_SelectionList *list;
													MIDIPattern *pattern;
													bool add;
												};

												popmenu->AddFMenu(Cxs[CXS_MIXOSELPTOP],new menu_mixp(sl,mp,false));
												popmenu->AddFMenu(Cxs[CXS_ADDOSELPTOP],new menu_mixp(sl,mp,true));

												// Add Pattern<->Pattern

												// Connect Pattern ?
												class menu_connectp:public guiMenu
												{
												public:
													menu_connectp(Seq_SelectionList *l,MIDIPattern *p)
													{
														list=l;
														pattern=p;
													}

													void MenuFunction()
													{
														//	mainedit->ConvertAllLoopsToPattern(pattern);	
														mainedit->MixAllSelectedPatternToPattern(list,pattern,true,true);	
													} 

													Seq_SelectionList *list;
													MIDIPattern *pattern;
												};

												checkok=false;

												slp=sl->FirstSelectedPattern();
												while(slp)
												{
													if(slp->pattern!=mp && slp->pattern->mediatype==mp->mediatype && slp->pattern->GetTrack()==mp->GetTrack())
														checkok=true;

													slp=slp->NextSelectedPattern();
												}

												if(checkok==true)
													popmenu->AddFMenu(Cxs[CXS_CONNECTSELECTP],new menu_connectp(sl,mp));

												popmenu->AddLine();
											}
										}
									}

								}// mediatype MIDI

								popmenu->AddLine();

								AddPatternMenu(pattern);

								if(o->id==OBJECTID_ARRANGEPATTERN)
									AddSetSongPositionMenu(pattern,pattern->track);

								ShowPopMenu();
							}
						}
					}
			} // if sub object

			return true;
		}
		break;
	}

	return false;
}

void Edit_Arrange::ChangeTrackIcon(Seq_Track *track,TrackIcon *icon)
{
	if(icon!=track->icon)
	{
		track->icon=icon;

		if(track->trackimage)
		{
			delete track->trackimage;
			track->trackimage=0;
		}

		if(track->icon)
		{
			track->trackimage=mainvar->GenerateString(track->icon->filename);
		}

		guiWindow *w=maingui->FirstWindow();

		while(w)
		{
			if(w->WindowSong()==track->song)
				switch(w->GetEditorID())
			{
				case EDITORTYPE_ARRANGE:
					{
						Edit_Arrange *ed=(Edit_Arrange *)w;
						//if(ed->WindowSong()->GetFocusTrack()==track)ed->trackfx.ShowActiveTrack();
						if(Edit_Arrange_Track *et=ed->FindTrack(track))et->ShowTrack(true);
					}
					break;

			}

			w=w->NextWindow();
		}
	}
}

bool Edit_Arrange::EditCancel()
{
	switch(mousemode)
	{
	case EM_EDIT_FADEIN:
	case EM_EDIT_FADEOUT:
	case EM_EDIT_VOLUME:
		{
			CancelEditFades();
			SetEditorMode(EM_RESET);
			return true;
		}
		break;

	case EM_SIZEPATTERN_LEFT:
	case EM_SIZEPATTERN_RIGHT:
		SetEditorMode(EM_RESET);
		return true;
		break;

	case EM_MOVEPATTERN:
		{
			SetEditorMode(EM_RESET);
			DrawDBBlit(pattern);
			return true;
		}
		break;

	case EM_SELECTOBJECTSSTART:
		SetEditorMode(EM_RESET);
		break;

	case EM_SELECTOBJECTS:
		{
			if(modestartautomationtrack)
			{
				if(modestartautomationtrack->bindtoautomationobject)
				{
					AutomationParameter *ap=modestartautomationtrack->FirstAutomationParameter();

					while(ap)
					{
						ap->flag CLEARBIT OFLAG_UNDERSELECTION;
						ap=ap->NextAutomationParameter();
					}
				}

				SetEditorMode(EM_RESET);
				ShowCycleAndPositions(editarea);
				DrawDBSpriteBlit(editarea);
			}
			else
			{
				WindowSong()->ClearAllUnderSelectionPattern();
				SetEditorMode(EM_RESET);
				DrawDBBlit(pattern);
			}

			return true;
		}
		break;

	case EM_MOVEAUTOMATION:
		{
			if(automationtrackeditinit)
			{
				automationtrackeditinit->Reset();
				automationtrackeditinit=0;
			}

			SetEditorMode(EM_RESET);
			return true;
		}
		break;
	}

	return false;
}

void Edit_Arrange::GotoSoloTrack()
{
	int c=0;
	Seq_Track *t=WindowSong()->FirstTrack();

	while(t)
	{
		if(t->GetSolo()==true)
		{
			/*
			if(firstshowtracknr!=c)
			{
			firstshowtracknr=c;
			ShowHoriz(true,false,false);
			SetFocusTrack(t);
			}
			*/

			break;
		}

		// Jump Automation Tracks
		if(t->showautomationstracks==true)
		{
			AutomationTrack *at=t->FirstAutomationTrack();

			while(at){
				c++;
				at=at->NextAutomationTrack();
			}
		}

		c++;

		t=t->NextTrack();
	}
}


EditData *Edit_Arrange::EditDataMessage(EditData *data)
{
	if(CheckStandardDataMessage(data)==true)
		return 0;

	//data=trackfx.EditDataMessage(data);

	if(data)
		data=Editor_DataMessage(data);

	if(data)
	{
		switch(data->id)
		{
		case EDIT_CHANNELNAME:
			{
				AudioChannel *ac=(AudioChannel *)data->helpobject;
				ac->SetName(data->newstring);
				ac->ShowChannelName(0);
			}
			break;

		case EDIT_NAME:
			{
				Seq_Track *t=(Seq_Track *)data->helpobject;
				t->SetName(data->newstring);
				t->ShowTrackName(0);
			}
			break;

		case EDIT_SCOPYPATTERN:
			{
				TRACE ("Copy sel Pattern %d\n",data->newvalue);
				MoveSelectedPatternToMeasureToTick(data->newvalue,true);
			}
			break;

		case EDIT_SMOVEPATTERN:
			{
				TRACE ("Move sel Pattern %d\n",data->newvalue);
				MoveSelectedPatternToMeasureToTick(data->newvalue,false);
			}
			break;


		case EDIT_MARKERNAME:
			{
				Seq_Marker *mk=(Seq_Marker *)data->helpobject;
				maingui->ChangeMarker(mk,data->newstring,0,0);
			}
			break;


		case EDIT_TRACKNAME:
			{
				Seq_Track *track=(Seq_Track *)data->helpobject;

				track->SetName(data->newstring);
				track->ShowTrackName(0);
			}
			break;

		case EDIT_PATTERNNAME:
			{
				Seq_Pattern *p=(Seq_Pattern *)data->helpobject;

				p->SetName(data->newstring);
				p->ShowPatternName(0);
			}
			break;
		}
	}

	return 0;
}


void Edit_Arrange::SetSet(int s)
{
	if(set!=s)
	{
		set=s;
		mainsettings->lastselectarrangeset=s;

		RefreshTracking();

		ShowFilter();
		for(int i=0;i<5;i++)
			if(g_set[i])
				g_set[i]->Toggle(i==set?true:false);
	}
}

void Edit_Arrange::SetAutoMode(int tracking)
{
	if(tracking!=mainsettings->arrangetracking[set])
	{
		mainsettings->arrangetracking[set]=tracking;

		RefreshTracking();
		ShowTracking();
	}
}


void Edit_Arrange::RefreshAllArrangeWithSameSet()
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(w->WindowSong()==WindowSong())
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)w;

					if(ar->set==set)
					{
						ar->RefreshObjects(0,false);
						ar->ShowFilter();

						if(ar->listeditor)
							ar->listeditor->DrawDBBlit(ar->listeditor->list);
					}
				}
				break;
			}
		}

		w=w->NextWindow();
	}
}

void Edit_Arrange::ClearFlag(int f)
{
	mainsettings->arrangesettings[set] CLEARBIT f;

	RefreshAllArrangeWithSameSet();
}

void Edit_Arrange::AddFlag(int f)
{
	mainsettings->arrangesettings[set] |= f;
	RefreshAllArrangeWithSameSet();
}

// Buttons,Slider ...
void Edit_Arrange::Gadget(guiGadget *gadget)
{	
	if(!Editor_Gadget(gadget))return;

	switch(gadget->gadgetID)
	{
	case GADGETID_TOOLBOX_QUANTIZE:
		if(Seq_SelectionList *sl=CreateSelectionList())
			mainedit->QuantizePatternMenu(this,sl);
		break;

	case GADGETID_SHOWMASTER:
		showmaster=showmaster==true?false:true;
		mainsettings->showarrangemaster=showmaster;
		ShowShowStatus();
		break;

	case GADGETID_SHOWBUS:
		showbus=showbus==true?false:true;
		mainsettings->showarrangebus=showbus;
		ShowShowStatus();
		break;

	case GADGETID_SHOWMETRO:
		showmetro=showmetro==true?false:true;
		mainsettings->showarrangemetro=showmetro;
		ShowShowStatus();
		break;

	case GADGETID_EVENTEDITOR:
		{
			globmenu_Event editor(this,startposition);
			editor.MenuFunction();
		}break;

	case GADGETID_PIANOEDITOR:
		{
			globmenu_Piano editor(this,startposition);
			editor.MenuFunction();
		}break;

	case GADGETID_DRUMEDITOR:
		{
			globmenu_Drum editor(this,startposition);
			editor.MenuFunction();
		}break;

	case GADGETID_SAMPLEEDITOR:
		{
			globmenu_Sample editor(this,startposition);
			editor.MenuFunction();
		}
		break;

	case GADGETID_TNAME:
		if(WindowSong()->GetFocusPattern())
			Goto(GOTO_FOCUSPATTERN);
		break;

	case GADGETID_MUTE:
		{
			if(track_muted==false)
				WindowSong()->RefreshMuteBuffer();
			else
				WindowSong()->UnMuteAllTracks();
		}
		break;

	case GADGETID_SOLO:
		{
			if(track_soloed==false)
				WindowSong()->RefreshSoloBuffer();
			else
				WindowSong()->UnSoloAllTracks();
		}
		break;

	case GADGETID_NEWTRACK:
		{
			globmenu_cnewtrack newtrack(WindowSong(),WindowSong()->GetFocusTrack());
			newtrack.MenuFunction();
		}
		break;

	case GADGETID_NEWCHILD:
		{
			if(WindowSong()->GetFocusTrack() && WindowSong()->GetFocusTrack()->CanNewChildTrack()==true)
			{
				WindowSong()->UnSelectTracksFromTo(WindowSong()->FirstTrack(),WindowSong()->LastTrack());
				mainedit->CreateNewChildTracks(WindowSong(),WindowSong()->GetFocusTrack(),1,EditFunctions::CREATETRACK_ACTIVATE,WindowSong()->GetFocusTrack());
			}
		}
		break;

	case GADGETID_RESETTRACKZOOM:
		WindowSong()->ResetTrackZoom();
		break;

	case GADGETID_FILTERTRACKS:
		{
			if(DeletePopUpMenu(true))
			{
				class menu_selecttrackauto:public guiMenu
				{
				public:
					menu_selecttrackauto(Edit_Arrange *e,int i){editor=e;index=i;}

					void MenuFunction()
					{
						editor->SetAutoMode(index);
					} //

					Edit_Arrange *editor;
					int index;
				};

				for(int i=0;i<AUTO_ENDOFFLAGS;i++)
				{
					int cxs;

					switch(i)
					{
					case 0:
						cxs=CXS_AUTOALL;
						break;

					case 1:
						cxs=CXS_AUTOSELECTED;
						break;

					case 2:
						cxs=CXS_AUTOFOCUS;
						break;

					case 3:
						cxs=CXS_AUTOAUDIOORMIDI;
						break;

					case 4:
						cxs=CXS_AUTOAUDIO;
						break;

					case 5:
						cxs=CXS_AUTOMIDI;
						break;

					case 6:
						cxs=CXS_AUTOINSTR;
						break;

					case 7:
						cxs=CXS_AUTOTRACKSRECORD;
						break;
					}

					popmenu->AddFMenu(Cxs[cxs],new menu_selecttrackauto(this,i),i==mainsettings->arrangetracking[set]?true:false);
				}

				ShowPopMenu();
			}
		}
		break;

	case GADGETID_AUDIO:
		{
			if(GetShowFlag()&SHOW_AUDIO)
				ClearFlag(SHOW_AUDIO);
			else
				AddFlag(SHOW_AUDIO);
		}
		break;

	case GADGETID_MIDI:
		{
			if(GetShowFlag()&SHOW_MIDI)
				ClearFlag(SHOW_MIDI);
			else
				AddFlag(SHOW_MIDI);
		}
		break;

	case GADGETID_TYPE:
		if(GetShowFlag()&SHOW_EVENTTYPE)
			ClearFlag(SHOW_EVENTTYPE);
		else
			AddFlag(SHOW_EVENTTYPE);
		break;

	case GADGETID_INPUT:
		if(GetShowFlag()&SHOW_IO)
			ClearFlag(SHOW_IO);
		else
			AddFlag(SHOW_IO);
		break;

	case GADGETID_AUTOMATION:
		if(GetShowFlag()&SHOW_AUTOMATION)
			ClearFlag(SHOW_AUTOMATION);
		else
			AddFlag(SHOW_AUTOMATION);
		break;

	case GADGETID_VOLUMECURVE:
		if(GetShowFlag()&SHOW_VOLUMECURVES)
			ClearFlag(SHOW_VOLUMECURVES);
		else
			AddFlag(SHOW_VOLUMECURVES);
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

	case GADGETID_LIST:
		{
			showlist=showlist==true?false:true;
			FormEnable(TAB_TRACKLIST,1,showlist);
			mainsettings->showarrangelist=showlist;
		}
		break;

	case GADGETID_EFFECTS:
		{
			showeffects=showeffects==true?false:true;
			FormEnable(TAB_TRACKINFO,1,showeffects);
			mainsettings->showarrangeeffects=showeffects;
		}
		break;

	case GADGETID_EDITORSLIDER_VERT: // Track Scroll
		trackobjects.InitWithSlider(vertgadget,true);
		DrawDBBlit(tracks,pattern);
		break;
	}
}

void Edit_Arrange::RefreshMIDI(Seq_Track *t)
{
	if( (GetShowFlag()&SHOW_MIDI) && (GetShowFlag()&SHOW_IO) && tracks)
	{
		Edit_Arrange_Track *et=FindTrack(t);

		if(et)
		{
			et->ShowMIDIInput();
			tracks->Blt(et);
		}
	}
}

Edit_Arrange::Edit_Arrange()
{	
	editorid=EDITORTYPE_ARRANGE;

	InitForms(FORM_HORZ4x1BAR_SLIDERHV);

	EditForm(TAB_TRACKLIST,1,CHILD_HASWINDOW); // List
	EditForm(TAB_TRACKINFO,1,CHILD_HASWINDOW); // FX

	InitFormEnable(TAB_TRACKLIST,1,showlist=mainsettings->showarrangelist);
	InitFormEnable(TAB_TRACKINFO,1,showeffects=mainsettings->showarrangeeffects);

	resizeable=true;
	modestartautomationtrack=0;
	editorname="Arrange";
	blinkrecordcounter_toggle=true;

	SetMinZoomY(maingui->GetButtonSizeY()+4);

	zoomybuffermul=3;

	shownotesinarrageeditor=mainsettings->shownotesinarrageeditor;
	showcontrols=mainsettings->showarrangecontrols;
	showpatternlist=mainsettings->showarrangepatternlist;

	showtextmap=mainsettings->showarrangetextmap;
	showmarkermap=mainsettings->showarrangemarkermap;

	trackonoff=true;
	patternonoff=true;

	focustrackwindowname=0;

	focustrackwindowname_string=0;
	activepatternwindowname_string=0;
	followsongposition=mainsettings->followeditor;

	track_soloed=false;
	track_muted=false;

	addpattern=0;
	sizepattern=0;
	blinkrecordcounter=0;

	getcountselectedtracks=getcounttracks=getcountselectedpattern=getcountpattern=-1;
	mouseselectionstarttrack=0;

	set=mainsettings->lastselectarrangeset;

	showmaster=mainsettings->showarrangemaster;
	showmetro=mainsettings->showarrangemetro;
	showbus=mainsettings->showarrangebus;

	automationtrackeditinit=0;
}

bool Edit_Arrange::ZoomGFX(int z,bool horiz)
{
	if(SetZoomGFX(z,horiz)==true)
	{
		DrawDBBlit(tracks,pattern,horiz==false?overview:0);
		return true;
	}

	return false;
}

Edit_Arrange_AutomationTrack *Edit_Arrange::FindAutomationTrack(AutomationTrack *track)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_ARRANGEAUTOMATIONTRACK)
		{
			Edit_Arrange_AutomationTrack *c=(Edit_Arrange_AutomationTrack *)o;
			if(c->automationtrack==track)return c;
		}

		o=o->NextObject();
	}

	return 0;
}

Edit_Arrange_Track *Edit_Arrange::FindTrack(Seq_Track *track)
{
	if(!tracks)return 0;

	guiObject_Pref *o=tracks->FirstGUIObjectPref();
	while(o)
	{
		Edit_Arrange_Track *et=(Edit_Arrange_Track *)o->gobject;

		if(et->track==track)
			return et;

		o=o->NextGUIObjectPref();
	}

	return 0;
}

Edit_Arrange_Pattern *Edit_Arrange::FindAudioHDFile(AudioHDFile *hd)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_ARRANGEPATTERN)
		{
			Edit_Arrange_Pattern *c=(Edit_Arrange_Pattern *)o;

			if(c->pattern->mediatype==MEDIATYPE_AUDIO)
			{
				AudioPattern *ap=(AudioPattern *)c->pattern;
				if(ap->audioevent.audioefile==hd)return c;
			}
		}

		o=o->NextObject();
	}

	return 0;
}

Edit_Arrange_Pattern *Edit_Arrange::FindAudioRegion(AudioRegion *region)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_ARRANGEPATTERN)
		{
			Edit_Arrange_Pattern *c=(Edit_Arrange_Pattern *)o;

			if(c->pattern->mediatype==MEDIATYPE_AUDIO)
			{
				AudioPattern *ap=(AudioPattern *)c->pattern;

				if(ap->audioevent.audioregion==region)
					return c;

				// Region inside Audio HD File ?
				if(ap->audioevent.audioregion==0)
				{
					AudioGFX_Region *agr=(AudioGFX_Region *)c->regions.GetRoot();

					while(agr)
					{
						if(agr->region==region)
							return c;

						agr=agr->Next();
					}
				}
			}
		}

		o=o->NextObject();
	}

	return 0;
}

Edit_Arrange_Pattern *Edit_Arrange::FindPattern(Seq_Pattern *pattern)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_ARRANGEPATTERN)
		{
			Edit_Arrange_Pattern *c=(Edit_Arrange_Pattern *)o;
			if(c->pattern==pattern)return c;
		}

		o=o->NextObject();
	}

	return 0;
}

void Edit_Arrange::ShowOverviewVertPosition(int *y,int *y2)
{
	*y=0;
	*y2=overview->GetY2();

	if(trackobjects.height==0 || (!tracks))
		return;

	double h=trackobjects.height;
	double h2=trackobjects.starty;
	double height=overview->GetY2();

	h2/=h;
	h2*=height;

	*y=(int)h2;

	h2=trackobjects.starty+trackobjects.guiheight;
	h2/=h;
	h2*=height;

	*y2=(int)h2;
}

void Edit_Arrange::ShowOverview()
{
	if(!overview)return;

	guiBitmap *bitmap=&overview->gbitmap;
	bitmap->guiFillRect(COLOUR_OVERVIEW_BACKGROUND);

	overviewlenght=WindowSong()->GetSongLength_Ticks();

	if(int i=overviewtrackobjects.GetCount())
	{
		// Real Pattern/Events
		OObject *oo=(OObject *)overviewtrackobjects.GetRoot();

		double oh=overview->GetY2();
		double lh=overviewtrackobjects.height;

		if(lh==0)
			return;

		while(oo)
		{
			switch(oo->object->id)
			{
			case OBJ_TRACK:
				{
					Seq_Track *t=(Seq_Track *)oo->object;

					double h=oo->cy;
					h/=lh;
					h*=oh;
					int dy=(int)h;

					h=oo->cy+oo->ch;
					h/=lh;
					h*=oh;
					int dy2=(int)h;

					Seq_Pattern *p=t->FirstPattern();

					while(p)
					{
						OSTART s=p->GetPatternStart(),e=p->GetPatternEnd();

						int hx=ConvertTimeToOverviewX(s);
						int hx2=ConvertTimeToOverviewX(e);

						if(hx2<0)
							hx2=bitmap->GetX2();
						else
							if(hx2==hx)
							{
								if(hx2<bitmap->GetX2())
									hx2++;
							}

							if(dy==dy2)
								bitmap->guiDrawLineY(dy,hx,hx2,t==WindowSong()->GetFocusTrack()?COLOUR_OVERVIEWFOCUSOBJECT:COLOUR_OVERVIEWOBJECT);
							else
								bitmap->guiFillRect(hx,dy,hx2,dy2,t==WindowSong()->GetFocusTrack()?COLOUR_OVERVIEWFOCUSOBJECT:COLOUR_OVERVIEWOBJECT);

							p=p->NextPattern();
					}
				}break;
			}

			oo=(OObject *)oo->next;
		}// while

		// Display Record Pattern/Events MIDI+AUDIO
		if(WindowSong()->status&Seq_Song::STATUS_RECORD)
		{
			OObject *oo=(OObject *)overviewtrackobjects.GetRoot();

			while(oo)
			{
				switch(oo->object->id)
				{
				case OBJ_TRACK:
					{
						Seq_Track *t=(Seq_Track *)oo->object;

						double h=oo->cy;
						h/=lh;
						h*=oh;
						int dy=(int)h;

						h=oo->cy+oo->ch;
						h/=lh;
						h*=oh;
						int dy2=(int)h;

						if(t->record_MIDIPattern) // MIDI
						{
							OSTART s=t->record_MIDIPattern->GetPatternStart(),e=t->record_MIDIPattern->GetPatternEnd();

							int hx=ConvertTimeToOverviewX(s),hx2=ConvertTimeToOverviewX(e);

							if(hx2<0)
								hx2=bitmap->GetX2();
							else
								if(hx2==hx)
								{
									if(hx2<bitmap->GetX2())
										hx2++;
								}

								if(dy==dy2)
									bitmap->guiDrawLineY(dy,hx,hx2,COLOUR_RED);
								else
									bitmap->guiFillRect(hx,dy,hx2,dy2,COLOUR_RED);
						}

						for(int i=0;i<MAXRECPATTERNPERTRACK;i++) // AUDIO
							if(t->audiorecord_audiopattern[i])
							{
								OSTART s=t->audiorecord_audiopattern[i]->GetPatternStart(),e=t->audiorecord_audiopattern[i]->GetPatternEnd();

								int hx=ConvertTimeToOverviewX(s),hx2=ConvertTimeToOverviewX(e);

								if(hx2<0)
									hx2=bitmap->GetX2();
								else
									if(hx2==hx)
									{
										if(hx2<bitmap->GetX2())
											hx2++;
									}

									if(dy==dy2)
										bitmap->guiDrawLineY(dy,hx,hx2,COLOUR_RED_LIGHT);
									else
										bitmap->guiFillRect(hx,dy,hx2,dy2,COLOUR_RED_LIGHT);
							}
					}break;

				}

				oo=(OObject *)oo->next;
			}// while
		}

	}// if i
}

void Edit_Arrange::ShowHideAutomationTracks(bool show)
{
	Seq_Track *t=WindowSong()->FirstTrack();
	while(t)
	{
		t->ShowHideAutomationTracks(show);
		t=t->NextTrack();
	}

	WindowSong()->audiosystem.masterchannel.ShowHideAutomationTracks(show);

	AudioChannel *bus=WindowSong()->audiosystem.FirstBusChannel();
	while(bus)
	{
		bus->ShowHideAutomationTracks(show);
		bus=bus->NextChannel();
	}

	maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
	maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGELIST,0);
}

int Edit_Arrange::ShowTextMap()
{	
#ifdef OLDIE
	if(frame_textmap.on==true)
	{
		int y=frame_textmap.y;

		if(timeline)
		{
			frame_textmap.y2=frame_textmap.y+frame_textmap.value;

			if(frame_textmap.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ)==true)
			{
				y=frame_textmap.y2+1;

				/*
				guiDrawText
				(frame_tracks.x,frame_textmap.y2-2,frame_tracks.x2,"Text");
				*/

				frame_textmap.Fill(guibuffer,COLOUR_WHITE);

				Seq_Text *text=WindowSong()->textandmarker.FirstText();
				while(text && text->GetTextStart()<endposition)
				{
					if(text->GetTextStart()>=startposition)
					{
						int x=timeline->ConvertTimeToX(text->GetTextStart(),frame_textmap.x2);
						if(x<=0)break;
						guibuffer->guiDrawText(x,frame_textmap.y2-2,frame_textmap.x2,text->string);
					}

					text=text->NextText();
				}

				if(!(winmode&WINDOWMODE_FRAMES))
					BltGUIBuffer_Frame(&frame_textmap);
			}
		}
		else
			frame_textmap.ondisplay=false;

		return y;
	}
#endif
	return -1;
}

int Edit_Arrange::ShowMarkerMap()
{	
#ifdef OLDIE
	guiobjects.RemoveOs(OBJECTID_ARRANGEMARKER);

	if(frame_markermap.on==true)
	{
		int y=frame_markermap.y;

		if(timeline)
		{
			frame_markermap.y2=y+frame_markermap.value;

			if(frame_markermap.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ)==true)
			{
				y=frame_markermap.y2+1;

				frame_markermap.Fill(guibuffer,COLOUR_WHITE_LIGHT);

				Seq_Marker *mk=WindowSong()->textandmarker.FirstMarker();
				while(mk && mk->GetMarkerStart()<endposition)
				{
					if((mk->markertype==Seq_Marker::MARKERTYPE_SINGLE && mk->GetMarkerStart()>=startposition) || (mk->markertype==Seq_Marker::MARKERTYPE_DOUBLE && mk->GetMarkerEnd()>startposition))
					{
						int x,x2;

						if(mk->GetMarkerStart()>startposition)
							x=timeline->ConvertTimeToX(mk->GetMarkerStart(),frame_markermap.x2);
						else
							x=timeline->x;

						if(Edit_Arrange_Marker *nmk=new Edit_Arrange_Marker(mk))
						{
							bool dbl=false;

							if(mk->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
							{
								if(mk->GetMarkerEnd()<endposition)
								{
									x2=timeline->ConvertTimeToX(mk->GetMarkerEnd(),frame_markermap.x2);
									dbl=true;
								}
								else
									x2=timeline->x2;

								guibuffer->guiFillRect_RGB(x,frame_markermap.y+1,x2,frame_markermap.y2-1,mk->colour.rgb);
							}
							else
							{
								x2=x;

								guibuffer->guiDrawLine(x,frame_markermap.y,x,frame_markermap.y2,COLOUR_BLACK);

								for(int hx=x+1,i=4;i>0;hx+=2,i--)
								{
									if(hx<frame_markermap.x2)
										guibuffer->guiDrawLine_RGB(x2=hx,frame_markermap.y+1,hx,frame_markermap.y2-1,mk->colour.rgb);
								}
							}

							guiobjects.AddGUIObject(x,frame_markermap.y,x2,frame_markermap.y2,&frame_markermap,nmk);

							if(dbl==true)
							{
								// Double Line Y
								if(x2-2>timeline->x)
									guibuffer->guiDrawLine(x2-2,frame_markermap.y+1,x2-2,frame_markermap.y2-1,COLOUR_GREY);	

								guibuffer->guiDrawLine(x2,frame_markermap.y+1,x2,frame_markermap.y2-1,COLOUR_BLACK);
								guibuffer->guiDrawLine(x,frame_markermap.y2-1,x2,frame_markermap.y2-1,COLOUR_BLACK);
							}

							guibuffer->guiDrawText(x+2,frame_markermap.y2-1,frame_markermap.x2,mk->string);
						}

					}

					mk=mk->NextMarker();
				}

				if(!(winmode&WINDOWMODE_FRAMES))
					BltGUIBuffer_Frame(&frame_markermap);
			}
		}
		else
			frame_markermap.ondisplay=false;

		return y;
	}
#endif

	return -1;
}

void Edit_Arrange::ShowSignature(Seq_Signature *s)
{
	if(s)
	{
		guiBitmap *bitmap=&signature->gbitmap;

		int x=timeline->ConvertTimeToX(s->GetSignatureStart(),bitmap->width);			

		if(x==-1)
		{
			// Signature <<<---- |
			bitmap->SetFont(&maingui->standardfont);
			x=0;
		}
		else
		{
			bitmap->SetFont(&maingui->standard_bold);
			bitmap->guiDrawLineX(x,COLOUR_GREY);
			x+=2;

		}

		char text[NUMBERSTRINGLEN],h[NUMBERSTRINGLEN];

		text[0]=0;

		mainvar->AddString(text,mainvar->ConvertIntToChar(s->nn,h));
		mainvar->AddString(text,"/");
		mainvar->AddString(text,mainvar->ConvertIntToChar(s->dn,h));

		bitmap->guiDrawText(x,maingui->GetButtonSizeY(),bitmap->width,text);	
	}
}

int Edit_Arrange::ShowSignatureMap()
{	
	int y=0;

	if(signature)
	{
		signature->gbitmap.guiFillRect(COLOUR_GREY_LIGHT);
		signature->gbitmap.SetTextColour(COLOUR_BLACK);

		Seq_Signature *t=WindowSong()->timetrack.FindSignature(startposition);

		if(!t)
			ShowSignature(WindowSong()->timetrack.FindSignatureBefore(startposition));
		else
		{
			while(t && t->GetSignatureStart()<endposition)
			{
				ShowSignature(t);
				t=t->NextSignature();
			}// while
		}

		//if(!(winmode&WINDOWMODE_FRAMES))
		//	BltGUIBuffer_Frame(&frame_signaturemap);
	}

	return y;
}

int Edit_Arrange::ShowTempoMap()
{
	int y=0;

	if(tempo)
	{
		guiBitmap *bitmap=&tempo->gbitmap;
		int x,lastx=-1,lasty;
		Seq_Tempo *lastdrawtempo=0,*tempobeforestart;
		double h2,h=tempo->GetHeight();
		char tempostring[20];

		h/=(MAXIMUM_TEMPO-MINIMUM_TEMPO);

		//y=frame_tempomap.y2+1;

		/*
		guiDrawText(frame_tracks.x,frame_tempomap.y2-2,frame_tracks.x2,"Tempo");
		*/

		bitmap->guiFillRect(COLOUR_BLUE_LIGHT);
		//frame_tempomap.Fill(guibuffer,COLOUR_BLUE_LIGHT);

		// find tempo b
		tempobeforestart=WindowSong()->timetrack.FirstTempo();

		while(tempobeforestart && tempobeforestart->GetTempoStart()<startposition)
			tempobeforestart=tempobeforestart->NextTempo();

		Seq_Tempo *t=WindowSong()->timetrack.GetTempo(startposition);

		while(t && t->GetTempoStart()<endposition)
		{
			if(t->type!=TEMPOEVENT_VIRTUAL)
			{
				bool inside=false;

				if(t->GetTempoStart()>=startposition && t->GetTempoStart()<endposition)
				{
					x=timeline->ConvertTimeToX(t->GetTempoStart(),tempo->GetWidth());
					inside=true;
				}
				else
					if(t->GetTempoStart()<startposition)
						x=1;
					else
						x=tempo->GetWidth()-1;

				if(x>=0)
				{
					// Draw Line To Last Tempo
					h2=h*t->tempo;
					int y2=(int)h2;

					int w2=x+4;
					if(w2>tempo->GetWidth())
						w2=tempo->GetWidth();

					/*
					if(t->GetTempoStart()>startposition)
					{
					// find last tempoevent
					Seq_Tempo *lt=t->PrevTempo();

					if(lt)
					{
					lastx=frame_tempomap.x;
					lasty=frame_tempomap.y2-(int)(h*lt->tempo);
					}
					}
					*/

					if(inside==true)
						bitmap->guiDrawLineX(x,COLOUR_GREY);

					// connect tempo<->tempo
					if(lastx>=0)
						bitmap->guiDrawLine(lastx,lasty,x,tempo->GetHeight()-y2,COLOUR_BLUE);

					lastdrawtempo=t;
					lasty=tempo->GetHeight()-y2;
					lastx=w2;

					// if(t->GetTempoStart()<endposition)
					{
						if(t->GetTempoStart()>=startposition && w2>x)
							bitmap->guiDrawRect(x,tempo->GetHeight()-y2,w2,tempo->GetHeight(),COLOUR_BLACK);

						mainvar->ConvertDoubleToChar(t->tempo,tempostring,3);

						int hx=bitmap->GetTextWidth(tempostring);
						bool cset=false;

						if(t->GetTempoStart()<startposition)
						{
							bitmap->SetTextColour(0x0a,0x0a,0x11);

							hx+=x;

							if(hx+4<=tempo->x2)
							{
								hx+=4;
								lastx=hx;

								bitmap->guiDrawLine_RGB(hx,1,hx,tempo->GetHeight()-1,0xDFBB33);
								bitmap->guiDrawLine_RGB(hx-4,1,hx,1,0xDFBB33);
								bitmap->guiDrawLine_RGB(hx-4,tempo->GetHeight()-1,hx,tempo->GetHeight()-1,0xDFBB33);
							}
							else
								lastx=hx;
						}
						else
						{
							UBYTE r,g,b;
							maingui->colourtable.GetRGB(COLOUR_BLACK,&r,&g,&b);
							bitmap->SetTextColour(r,g,b);
							lastx=x+hx+1;
							cset=true;
						}

						bitmap->guiDrawText(x,tempo->GetHeight()-2,tempo->GetWidth(),tempostring);

						if(cset==false)
						{
							UBYTE r,g,b;
							maingui->colourtable.GetRGB(COLOUR_BLACK,&r,&g,&b);
							bitmap->SetTextColour(r,g,b);
						}
					}
				}
			}

			t=t->NextTempo();
		}// while

		if(lastdrawtempo && t)
		{
			mainvar->ConvertDoubleToChar(t->tempo,tempostring,3);
			int hx=bitmap->GetTextWidth(tempostring),xp=tempo->GetWidth()-hx;

			if(xp<0)xp=0;

			double h2=h*t->tempo;
			int y2=(int)h2;

			bitmap->guiDrawLine_RGB(lastx,lasty,xp-1,tempo->GetHeight()-y2,0xCEBB33);

			if(xp-1+4<=tempo->x2)
			{
				bitmap->guiDrawLine_RGB(xp-1,1,xp-1,tempo->GetHeight()-1,0xDFBB33);
				bitmap->guiDrawLine_RGB(xp-1,1,xp+4,1,0xDFBB33);
				bitmap->guiDrawLine_RGB(xp-1,tempo->GetHeight()-1,xp+4,tempo->GetHeight()-1,0xDFBB33);
			}

			bitmap->SetTextColour(0x00,0x00,0xAA);
			bitmap->guiDrawText(xp,tempo->GetHeight()-2,tempo->GetWidth(),tempostring);

			UBYTE r,g,b;
			maingui->colourtable.GetRGB(COLOUR_BLACK,&r,&g,&b);
			bitmap->SetTextColour(r,g,b);
		}
	}

	return y;
}

void Edit_Arrange_Pattern::CheckMouseXY(int x,int y)
{
	int t=patternvolumepositions.CheckXY(x,y);
	if(t!=-1)
		patternvolumepositions.SetMouse(editor,t);
}

bool Edit_Arrange_Pattern::RefreshPositions()
{
	bool refresh=false;

	switch(pattern->mediatype)
	{
	case MEDIATYPE_MIDI:
		{
			if(pattern->GetAccessCounter()!=accesscounter)
			{
				accesscounter=pattern->GetAccessCounter();
				return true;
			}

			//if(track->track->song->status&Seq_Song::STATUS_STEPRECORD==0)
			{
				MIDIPattern *mp=(MIDIPattern *)pattern;

				mp->LockOpenNotes();
				if(mp->FirstOpenNote())
				{
					mp->UnlockOpenNotes();
					return true;
				}
				mp->UnlockOpenNotes();
			}
		}
		break;

	case MEDIATYPE_AUDIO:
		if(pattern->GetAccessCounter()!=accesscounter)
		{
			accesscounter=pattern->GetAccessCounter();
			return true;
		}
		break;

	case MEDIATYPE_AUDIO_RECORD:
		return true;
		break;
	}

	return false;
}

void Edit_Arrange::CalcPatternPosition(Edit_Arrange_Pattern *ep)
{		
	if(ep->start<startposition)
		ep->sx=timeline->x;
	else
		if(ep->start<=endposition)
			ep->sx=timeline->ConvertTimeToX(ep->start);
		else
			ep->sx=ep->sx2=-1;

	if(ep->end==ep->start)
		ep->x2=ep->sx;
	else
		ep->sx2=ep->end<=endposition?timeline->ConvertTimeToX(ep->end):timeline->x2;

	ep->x=ep->sx;
	ep->x2=ep->sx2;

	OSTART qstart=WindowSong()->timetrack.ConvertTicksToMeasureTicks(ep->start,false);

	ep->frame_x=qstart<startposition?-1:timeline->ConvertTimeToX(qstart);

	if(ep->start==ep->end)
	{
		OSTART qend=WindowSong()->timetrack.ConvertTicksToMeasureTicks(ep->end+1,true);

		ep->frame_x2=qend>endposition?-1:timeline->ConvertTimeToX(qend);

		ep->x=ep->sx=ep->frame_x;
		ep->x2=ep->sx2=ep->frame_x2;
	}
	else
	{
		OSTART qend=WindowSong()->timetrack.ConvertTicksToMeasureTicks(ep->end,true);
		ep->frame_x2=qend>endposition?-1:timeline->ConvertTimeToX(qend);
	}		
}

void Edit_Arrange::ShowAutomationTracks()
{
	guiObject_Pref *o=tracks->FirstGUIObjectPref();
	while(o)
	{
		switch(o->gobject->id)
		{
		case OBJECTID_ARRANGEAUTOMATIONTRACK:
			{
				Edit_Arrange_AutomationTrack *et=(Edit_Arrange_AutomationTrack *)o->gobject;
				et->DrawPattern(false,false,true);
			}
			break;
		}

		o=o->NextGUIObjectPref();
	}
}

void Edit_Arrange::FreePatternRegionMemory()
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		if(ot->id==OBJECTID_ARRANGEPATTERN)
		{
			Edit_Arrange_Pattern *ep=(Edit_Arrange_Pattern *)ot;

			if(ep->regions.GetRoot())
				ep->regions.DeleteAllO();
		}

		ot=ot->NextObject();
	}
}

void Edit_Arrange::ShowAllPattern()
{
	FreePatternRegionMemory();
	crossfades.DeleteAllO();

	if(timeline)
	{
		ShowPattern();

		if(mousemode==EM_MOVEPATTERN && selection.FirstSelectedPattern())
		{
			Edit_Arrange_Pattern eap;

			Seq_SelectedPattern *p=selection.FirstSelectedPattern();

			while(p)
			{
				OSTART start=p->pattern->GetPatternStart()+selection.movediff;

				start=QuantizeEditorMouse(start);

				int i=WindowSong()->GetOfTrack(p->pattern->GetTrack());	
				i+=selection.moveobjects_vert;

				Seq_Track *newtrack=WindowSong()->GetTrackIndex(i);
				Edit_Arrange_Track *ptrack;

				if(newtrack && (ptrack=FindTrack(newtrack)))
				{
					OSTART pend;

					switch(p->pattern->mediatype)
					{
					case MEDIATYPE_AUDIO:
						{
							AudioPattern *ap=(AudioPattern *)p->pattern;

							pend=WindowSong()->timetrack.ConvertSamplesToTempoTicks(start,ap->GetSamples());
						}

						break;

					default:
						pend=start+(p->pattern->GetPatternEnd()-p->pattern->GetPatternStart());
						break;
					}

					if(start<endposition && pend>=startposition)
					{
						eap.bitmap=&pattern->gbitmap;
						eap.y=eap.sy=ptrack->y+3;
						eap.y2=eap.sy2=ptrack->y2-4;
						eap.editor=this;
						eap.nonreal=true;
						eap.pattern=p->pattern;
						eap.undermove=true; // don't fill background, no crossfades
						eap.patterncurve=p->pattern->GetVolumeCurve();
						//eap.offset=selection.movediff;

						eap.start=start;
						eap.end=pend;
						eap.notext=false;

						CalcPatternPosition(&eap);

						eap.ShowPatternRaster();
						eap.ShowPattern();

					}//if plen
				}//if ptrack

				p=p->NextSelectedPattern();
			}// while p
		}
	}
}

void Edit_Arrange::ShowPatternCrossFades()
{
	/*
	//Show Crossfades
	Edit_Arrange_Pattern_CrossFade *cf=FirstCrossFade();

	while(cf)
	{
	guibuffer->guiInvert(cf->x,cf->y,cf->x2,cf->y2);

	if(cf->y+maingui->GetFontSizeY()<cf->y2)
	{
	cf->headery2=cf->y+maingui->GetFontSizeY();

	guibuffer->guiDrawLineY(cf->y,cf->x,cf->x2,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);

	if(cf->crossfade->used==false)
	guibuffer->guiDrawLineY(cf->headery2,cf->x,cf->x2,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);

	if(cf->startinside==true)
	guibuffer->guiDrawLine(cf->x,cf->y,cf->x,cf->headery2,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);

	if(cf->endinside==true)
	guibuffer->guiDrawLine(cf->x2,cf->y,cf->x2,cf->headery2,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);
	//editor->guibuffer->guiDrawRect(cf->x,cf->y,cf->x2,,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);

	if(cf->x2>cf->x+12)
	{
	guibuffer->guiDrawLine(cf->x,cf->y,cf->x+12,cf->headery2,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);
	guibuffer->guiDrawLine(cf->x,cf->headery2,cf->x+12,cf->y,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);
	}

	if(cf->x2-12>cf->x && cf->x2-12>cf->x+12)
	{
	guibuffer->guiDrawLine(cf->x2-12,cf->y,cf->x2,cf->headery2,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);
	guibuffer->guiDrawLine(cf->x2-12,cf->headery2,cf->x2,cf->y,cf->crossfade->used==true?COLOUR_RED:COLOUR_BLUE);
	}
	}
	else
	cf->headery2=-1;

	double height=cf->y2-cf->y;

	if(cf->crossfade->used==true)
	{
	// Draw Curve
	for(int x=cf->x;x<cf->x2;x++)
	{
	double dh=(double)timeline->ConvertXPosToTime(x),h2=(double)dh-cf->crossfade->from;

	dh=cf->crossfade->to-cf->crossfade->from;

	h2/=dh;

	ARES out=cf->crossfade->connectwith->ConvertToVolume(h2,false); // Out
	out*=height;
	guibuffer->guiDrawPixel(x,cf->y2-out,COLOUR_BLUE);

	ARES in=cf->crossfade->ConvertToVolume(h2,true); // Out
	in*=height;
	guibuffer->guiDrawPixel(x,cf->y2-in,COLOUR_RED);
	}
	}

	cf=cf->NextCF();
	}
	*/
}

void Edit_Arrange::ShowPattern()
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{	
		// 1. Raster
		if(ot->id==OBJECTID_ARRANGEPATTERN)
		{
			Edit_Arrange_Pattern *ep=(Edit_Arrange_Pattern *)ot;
			CalcPatternPosition(ep);
			//if(ep->startposition<endposition)
			ep->ShowPatternRaster();
		}

		ot=ot->NextObject();
	}// while ot

	ot=guiobjects.FirstObject();

	while(ot)
	{	
		if(ot->id==OBJECTID_ARRANGEPATTERN)
		{
			Edit_Arrange_Pattern *ep=(Edit_Arrange_Pattern *)ot;

			//if(ep->startposition<endposition)
			ep->ShowPattern();
		}

		ot=ot->NextObject();
	}// while ot

	ShowPatternCrossFades();
}

Edit_Arrange_Track::Edit_Arrange_Track()
{
	id=OBJECTID_ARRANGETRACK;

	//	automationtrack=0;
	//	showpiano=showwave=showdrum=false;

	info.track=
		child.track=automation.track=audioinputmonitoring.track=mute.track=record.track=solo.track=
		volume.track=name.track=type.track=input.track=MIDIvolume.track=MIDIinput.track=channels.track=automationsettings.track=this;

	volume.deleteable=
		MIDIvolume.deleteable=
		MIDIinput.deleteable=
		automation.deleteable=
		child.deleteable=
		mute.deleteable=
		audioinputmonitoring.deleteable=
		record.deleteable=
		solo.deleteable=
		name.deleteable=
		type.deleteable=
		channels.deleteable=
		automationsettings.deleteable=
		input.deleteable=
		info.deleteable=
		false;

	freezeprogress=0;
	showfreeze_flag=0;

	blinkrecord=false;
	namestring=0;

	MIDIdisplay=audiooutdisplay=0;
}

void Edit_Arrange_Track::ShowVolume()
{
	if(volume.ondisplay==false)
		return;

	volumeeditable=track->CanAutomationObjectBeChanged(&track->io.audioeffects.volume,0,0);
	volume.volume=track->io.audioeffects.volume.value;
	volume.insert=track->io.audioeffects.FirstInsertAudioEffect() /* || track->io.audioeffects.FirstActiveAudioInstrument()*/?true:false;
	volumeclicked=track->volumeclicked;

	if(volumeeditable==false)
		bitmap->guiFillRect(volume.x,volume.y,volume.x2,volume.y2,COLOUR_AUTOMATIONTRACKSUSED);
	else
		bitmap->guiFillRect(volume.x,volume.y,volume.x2,volume.y2,volumeclicked==true?COLOUR_BLACK:COLOUR_AUDIOINVOLUME_BACKGROUND);

	int dbv=bitmap->SetAudioColour(volume.volume);

	if(char *h=mainaudio->ScaleAndGenerateDBString(volume.volume,false))
	{
		guiFont *old=bitmap->SetFont(volume.insert==true?&maingui->standard_bold:&maingui->standardfont);

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

	int tx2=lx2-hw;

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

	if(tx2+1<lx2)
		bitmap->guiFillRect(tx2+1,ly,lx2,ly2,COLOUR_BLACK_LIGHT);
}

void Edit_Arrange_Track::ShowMIDIVolume()
{
	if(MIDIvolume.ondisplay==false)
		return;

	MIDIvolume.MIDIvelocity=track->t_trackeffects.GetVelocity_NoParent();
	MIDIvolume.editable=track->CanAutomationObjectBeChanged(&track->MIDIfx.velocity,0,0);
	m_volumeclicked=track->m_volumeclicked;

	if(MIDIvolume.editable==false)
		bitmap->guiFillRect(MIDIvolume.x,MIDIvolume.y,MIDIvolume.x2,MIDIvolume.y2,COLOUR_AUTOMATIONTRACKSUSED);
	else
		bitmap->guiFillRect(MIDIvolume.x,MIDIvolume.y,MIDIvolume.x2,MIDIvolume.y2,m_volumeclicked==true?COLOUR_BLACK:COLOUR_AUDIOINVOLUME_BACKGROUND);

	bitmap->SetFont(&maingui->smallfont);

	int mx=bitmap->GetTextWidth("*Velocity*");
	int sx=MIDIvolume.x2-1-mx;

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
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

	int tx2=lx2-hw;

	if(tx2>lx)
		bitmap->guiFillRect(lx,ly,tx2,ly2,COLOUR_YELLOW);

	if(tx2+1<lx2)
		bitmap->guiFillRect(tx2+1,ly,lx2,ly2,COLOUR_BLACK);
}

void Edit_Arrange_Track::ShowTrackNumber()
{
	if(track->ismetrotrack==true)
		return;

	int number=track->GetTrackIndex()+1;
	bitmap->guiDrawNumber(1,y+maingui->GetButtonSizeY(),startnamex-1,number);
}

void Edit_Arrange_Track::ShowTrackRaster()
{
	trackselected=track->IsSelected();

	if(track->ismetrotrack==true)
		bgcolour=COLOUR_METROTRACK;
	else
		if(track->IsSelected()==true)
			bgcolour=track->parent?COLOUR_BACKGROUNDCHILD_HIGHLITE:COLOUR_BACKGROUNDFORMTEXT_HIGHLITE;
		else
			bgcolour=track->parent?COLOUR_BACKGROUNDCHILD:COLOUR_BACKGROUND;

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

	ShowTrackNumber();

	bitmap->guiFillRect(startnamex,y,x2,y2,bgcolour);

	bitmap->guiDrawLineX(0,y,y2,col);
	bitmap->guiDrawLineX(startnamex-1,y,y2,col);

	bitmap->guiDrawLineYX0(y,x2,col);
	bitmap->guiDrawLineYX0(y2,x2,col2);

	if(y2>y+8 && x2>x+8)
	{
		if(track->icon && type.ondisplay==true)
		{
			int ix=x2-track->icon->bitmap->width-2,iy=y2-track->icon->bitmap->height-2;

			if(ix>=type.x && iy>type.y2)
				bitmap->guiDrawImage(type.x,type.y2,x2,y2,track->icon->bitmap);
		}
	}
}

void Edit_Arrange_Track::ShowAudioDisplay(bool force)
{
	if(datadisplay==false)
		return;

	bool forceblit=false;
	bool tinputmonitoring=track->io.tempinputmonitoring;

	if(tinputmonitoring==true && track->io.inputmonitoring==false && (editor->WindowSong()->status&Seq_Song::STATUS_PLAY))
		tinputmonitoring=false;

	if(force==false && outpeak.inputmonitoring!=tinputmonitoring)
	{
		force=true;
		forceblit=true;
	}

	outpeak.showactivated=false;
	outpeak.force=force;
	outpeak.peak=track->GetPeak();

	int	flag=0,audiox,hmx=(dataoutputx2-dataoutputx)/2;

	audiox=dataoutputx+hmx+1;

	outpeak.channel=0;
	outpeak.track=track;

	outpeak.x=audiox;
	outpeak.x2=dataoutputx2;
	outpeak.y=y+1;
	outpeak.y2=y2-1;

	outpeak.active=mainvar->GetActiveSong()==editor->WindowSong()?true:false;
	outpeak.bitmap=&editor->tracks->gbitmap;

	outpeak.ShowInit(true);
	outpeak.ShowPeak();

	//	audiodisplay=outpeak.current_sum;
	//	audiodisplay_max=outpeak.current_max;

	if(forceblit==true || (force==false && outpeak.changed==true))
	{
		editor->tracks->Blt(audiox,y+1,dataoutputx2,y2-1);
	}
}

void Edit_Arrange_Track::ShowMIDIDisplay(bool force)
{
	if(datadisplay==false)
		return;

	int	MIDIx,MIDIx2,hmx=dataoutputx+(dataoutputx2-dataoutputx)/2;

	MIDIx=dataoutputx;
	MIDIx2=hmx-1;

	m_outpeak.channel=0;
	m_outpeak.track=track;
	m_outpeak.bitmap=&editor->tracks->gbitmap;
	m_outpeak.x=MIDIx;
	m_outpeak.y=y+1;
	m_outpeak.x2=MIDIx2;
	m_outpeak.y2=y2-1;
	m_outpeak.force=force;
	m_outpeak.noMIDItext=true;

	m_outpeak.ShowInit(false);
	m_outpeak.ShowPeak();

	if(force==false && m_outpeak.changed==true)
		editor->tracks->Blt(MIDIx,y+1,MIDIx2,y2-1);
}

void Edit_Arrange::InitPattern(Edit_Arrange_Track *et,Seq_Track *tr,bool folder)
{
	// Init Pattern List
	Seq_Pattern *rp=tr->FirstPattern(MEDIATYPE_ALL);

	while(rp)
	{
		if(rp->visible==true)
		{
			OSTART ps=rp->GetPatternStart();

			if(ps>endposition)
				break;

			OSTART pe=rp->GetPatternEnd();

			if(pe==ps)
			{
				pe=WindowSong()->timetrack.ConvertTicksToMeasureTicks(pe+1,true);
			}

			if(/*ps>=startposition || */ pe>=startposition)
			{	
				if(Edit_Arrange_Pattern *newp=new Edit_Arrange_Pattern)
				{
					newp->bitmap=&pattern->gbitmap;
					newp->start=ps;
					newp->end=pe;
					newp->track=et;
					newp->pattern=rp;
					newp->editor=this;
					newp->accesscounter=rp->GetAccessCounter();
					newp->numberofevents=rp->GetCountOfEvents();
					newp->folder=folder;

					if(rp->mediatype==MEDIATYPE_AUDIO)
						newp->patterncurve=rp->GetVolumeCurve();

					if(folder==true)
						newp->idprio=-1;

					newp->notext=folder;

					guiobjects.AddGUIObject(-1,et->y+3,-1,et->y2-4,pattern,newp);
				}
			}
		}

		rp=rp->NextPattern(MEDIATYPE_ALL);
	}// while rp

}

void Edit_Arrange_AutomationTrack::RefreshRealtime()
{
	if(automationtrack->bindtoautomationobject && parmvalue!=automationtrack->bindtoautomationobject->GetParm(automationtrack->bindtoautomationobject_parindex))
	{
		parmvalue=automationtrack->bindtoautomationobject->GetParm(automationtrack->bindtoautomationobject_parindex);

		if(value.ondisplay==true)
		{
			ShowValue();
			value.Blt();
		}

		if(vu.ondisplay==true)
		{
			ShowVU();
			vu.Blt();
		}
	}

	if(mode.ondisplay==true && mode.mode!=automationtrack->automode)
	{
		ShowMode();
		mode.Blt();
	}

	Edit_Arrange_AutomationTrackParameter *atp=FirstTrackParameter();

	while(atp)
	{
		if(atp->flag!=atp->parameter->flag)
		{
			DrawPattern(true,true,true);
			editor->pattern->Blt(0,y,editor->pattern->GetX2(),y2);
			break;
		}

		atp=atp->NextAutomationTrackObject();
	}
}

void Edit_Arrange_AutomationTrack::ShowPattern()
{
	if(!editor->pattern)
		return;

	if(!automationtrack->track)
		return;

	if(automationtrack->track->ismetrotrack==true)
		return;

	Edit_Arrange_Pattern eap;

	pattern.DeleteAllO();

	Seq_Pattern *p=automationtrack->track->FirstPattern();

	while(p)
	{
		OSTART start=p->GetPatternStart();
		OSTART pend=p->GetPatternEnd();

		if(start<editor->endposition && pend>=editor->startposition)
		{
			eap.bitmap=&editor->pattern->gbitmap;
			eap.y=eap.sy=y+3;
			eap.y2=eap.sy2=y2-4;
			eap.editor=editor;
			eap.nonreal=true;
			eap.pattern=p;
			eap.undermove=false; // don't fill background, no crossfades
			eap.notext=true;
			eap.automationpattern=true;
			eap.patterncurve=p->GetVolumeCurve();
			eap.start=start;
			eap.end=pend;

			editor->CalcPatternPosition(&eap);

			eap.ShowPatternRaster();
			eap.ShowPattern();

			if(Edit_Arrange_AutomationPattern *ap=new Edit_Arrange_AutomationPattern)
			{
				ap->pattern=p;
				ap->patterncurve=eap.patterncurve;
				ap->fadeactive=eap.fadeactive;
				ap->fadeinms=eap.fadeinms;
				ap->fadeoutms=eap.fadeoutms;
				ap->volume=eap.volume;
				ap->volumeactive=eap.volumeactive;
				ap->fadeintype=eap.fadeintype;
				ap->fadeouttype=eap.fadeouttype;

				pattern.AddEndO(ap);
			}
		}

		p=p->NextPattern();

	}// while p
}

void Edit_Arrange_AutomationTrack::DrawPatternParameters()
{
	if(!editor->pattern)
		return;

	if(automationtrack->visible==false)
		return;

	parameter.DeleteAllO();

	guiBitmap *bitmap=&editor->pattern->gbitmap;

	if(AutomationObject *ao=automationtrack->bindtoautomationobject)
	{
		accesscount=automationtrack->parameter.accesscounter;

		int index=automationtrack->bindtoautomationobject_parindex;

		AutomationParameter *ap=automationtrack->FindAutomationParameter(editor->startposition);

		double yh=y2-y;
		int fonty=maingui->GetFontSizeY(),lastvtextx2=-1;
		int fontw=fonty/3;

		bool showtext=(y2-y)>2*fonty?true:false;

		/*
		{
		AutomationParameter *abs=automationtrack->FindAutomationParameterBefore(editor->startposition);

		if(abs)
		{
		if(char *v=ao->GetParmValueStringPar(index,abs->GetParameterValue()))
		{
		ARES h=ap->GetParameterValue(); // 0 - 1
		h*=yh;


		int yp=y2-(int)h;

		int ry=yp-fontw;
		int ry2=yp+fontw;

		if(ry<y)ry=y;
		if(ry2>y2)ry2=y2;

		bitmap->SetTextColour(COLOUR_AUTOMATIONSCALE);
		bitmap->guiDrawText(0,yp,bitmap->GetX2(),v);
		}
		}

		}
		*/

		while(ap && ap->GetParameterStart()<editor->endposition)
		{
			double h=ap->GetParameterValue(); // 0 - 1
			h*=yh;

			int xp=editor->timeline->ConvertTimeToX(ap->GetParameterStart());
			int yp=y2-(int)h;

			//TRACE ("V %f\n",ap->GetParameterStart());
			//TRACE ("X %d Y %d\n",xp,yp);

			// Rect

			int rx=xp-fontw;
			int rx2=xp+fontw;
			int ry=yp-fontw;
			int ry2=yp+fontw;

			if(ry<y)ry=y;
			if(ry2>y2)ry2=y2;

			if(Edit_Arrange_AutomationTrackParameter *eat=new Edit_Arrange_AutomationTrackParameter)
			{
				eat->x=rx;
				eat->y=ry;
				eat->x2=rx2;
				eat->y2=ry2;

				eat->eat=this;
				eat->object=ao;
				eat->parameter=ap;
				eat->flag=ap->flag;

				AddTrackParameter(eat);

				if(eat->flag&OFLAG_MOUSEOVER)
					bitmap->guiFillRect(rx,ry,rx2,ry2,COLOUR_AUTOMATIONSCALE);
				else
					if(eat->flag&OFLAG_UNDERSELECTION)
						bitmap->guiFillRect(rx,ry,rx2,ry2,COLOUR_GREEN);
					else
						if(eat->IsSelected()==true)
							bitmap->guiFillRect(rx,ry,rx2,ry2,COLOUR_WHITE);
						else
							bitmap->guiDrawRect(rx,ry,rx2,ry2,COLOUR_AUTOMATIONSCALE);
			}

			/*
			if(ap->IsSelected()==true)
			bitmap->guiDrawLineX(xp,yp,y2,COLOUR_WHITE);
			else
			bitmap->guiDrawLineX(xp,yp,y2,(ap->flag&OFLAG_MOUSEOVER)?COLOUR_GREEN:COLOUR_RED);
			*/

			// Text ?

			if(showtext==true && xp>lastvtextx2 && yh)
			{
				int mid=y+(y2-y)/2;

				if(ap->IsSelected()==true)
					bitmap->SetTextColour(COLOUR_WHITE);
				else
					bitmap->SetTextColour((ap->flag&OFLAG_MOUSEOVER)?COLOUR_GREEN:COLOUR_AUTOMATIONTRACKS);

				if(yp>mid)
					yp=ry;
				else
					yp=ry2+fonty;

				if(char *v=ao->GetParmValueStringPar(index,ap->GetParameterValue()))
				{
					lastvtextx2=xp+bitmap->GetTextWidth(v);
					bitmap->guiDrawText(rx2+fontw,yp,bitmap->GetX2(),v);
				}
			}

			ap=ap->NextAutomationParameter();
		}

	}
}

void Edit_Arrange_AutomationTrack::DrawPatternScale(bool withtimeline)
{
	if(!editor->pattern)
		return;

	if(automationtrack->visible==false)
		return;

	guiBitmap *bitmap=&editor->pattern->gbitmap;

	editor->timeline->DrawPositionRaster(bitmap,y,y2);

	if(automationtrack->bindtoautomationobject)
	{
		ShowPattern();

		double h=y2-y;
		double h2=h;
		bool draw=false;

		h2*=automationtrack->bindtoautomationobject->scalefactor;

		for(int x=0;x<=editor->pattern->GetX2();x++)
		{
			OSTART xtime=editor->timeline->ConvertXPosToTime(x);

			if(draw==true || automationtrack->FindAutomationParameterBefore(xtime))
			{
				draw=true;
				h2=automationtrack->GetValueAtPosition(xtime);
				h2*=h;

				int yp=y2-(int)h2;

				if(yp>y)
					bitmap->guiDrawPixel(x,yp-1,COLOUR_AUTOMATIONSCALE2);

				bitmap->guiDrawPixel(x,yp,COLOUR_AUTOMATIONSCALE);

				if(yp<y2)
					bitmap->guiDrawPixel(x,yp+1,COLOUR_AUTOMATIONSCALE2);
			}
		}
	}
}

void Edit_Arrange_AutomationTrack::DrawPattern(bool withbackgroundclear,bool withmarker,bool withscaleandparameters)
{
	if(withbackgroundclear==true)
		ShowPatternBGRaster();

	if(withmarker==true)
		editor->ShowMarker(y,y2);

	if(withscaleandparameters==true)
	{
		DrawPatternScale(withbackgroundclear);
		DrawPatternParameters();
	}
}

void Edit_Arrange_AutomationTrack::ShowPatternBGRaster()
{
	if(!editor->pattern)
		return;

	if(automationtrack->visible==false)
		return;

	guiBitmap *bitmap=&editor->pattern->gbitmap;

	/*
	if(track->GetColour()->showcolour==true)
	bitmap->guiFillRect_RGB(0,y,bitmap->GetX2(),y2,track->GetColour()->rgb);
	else
	{
	// Group Colour
	Seq_Group_GroupPointer *p=track->GetGroups()->FirstGroup();
	while(p)
	{
	if(p->underdeconstruction==false && p->group->colour.showcolour==true)
	{
	bitmap->guiFillRect_RGB(0,y,bitmap->GetX2(),y2,p->group->colour.rgb);
	break;
	}

	p=p->NextGroup();
	}

	// Standard BG
	if(!p)
	{
	int col;

	if(track->IsSelected()==true)
	col=track->CheckIfPlaybackIsAble()==false?COLOUR_BACKGROUNDEDITOR_GFX_MUTEDHIGHLITE:COLOUR_BACKGROUNDEDITOR_GFX_HIGHLITE;
	else
	col=track->CheckIfPlaybackIsAble()==false?COLOUR_BACKGROUNDEDITOR_GFX_MUTED:COLOUR_BACKGROUNDEDITOR_GFX;

	bitmap->guiFillRectX0(y,bitmap->GetX2(),y2,col);
	}
	}
	*/

	/*
	if(automationtrack)
	{
	nulline=automationtrack->DrawScale(bitmap,editor->timeline->x,y,editor->timeline->x2,y2);
	bitmap->guiFillRect(1,nulline+1,bitmap->GetX2(),y2-1,COLOUR_BLUE_LIGHT);
	}
	*/

	// Track Lines

	bitmap->guiFillRectX0(y,bitmap->GetX2(),y2,COLOUR_BACKGROUNDAUTOMATIONARRANGE);



	Seq_Track *track=automationtrack->track;
	if(track && track->song->GetFocusTrack()==track)
	{
		bitmap->guiDrawLineYX0(y,bitmap->GetX2(),track->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT);
		bitmap->guiDrawLineYX0(y2,bitmap->GetX2());
	}
	else
	{
		bitmap->guiDrawLineYX0(y,bitmap->GetX2(),COLOUR_RASTER1);
		bitmap->guiDrawLineYX0(y2,bitmap->GetX2(),COLOUR_RASTER2);
	}
}


void Edit_Arrange_Track::ShowPatternBGRaster()
{
	if(!editor->pattern)
		return;

	guiBitmap *bitmap=&editor->pattern->gbitmap;

	if(track->GetColour()->showcolour==true)
		bitmap->guiFillRect_RGB(0,y,bitmap->GetX2(),y2,track->GetColour()->rgb);
	else
	{
		// Group Colour
		Seq_Group_GroupPointer *p=track->GetGroups()->FirstGroup();
		while(p)
		{
			if(p->underdeconstruction==false && p->group->colour.showcolour==true)
			{
				bitmap->guiFillRect_RGB(0,y,bitmap->GetX2(),y2,p->group->colour.rgb);
				break;
			}

			p=p->NextGroup();
		}

		// Standard BG
		if(!p)
		{
			int col;

			if(track->IsSelected()==true)
				col=playable==false?COLOUR_BACKGROUNDEDITOR_GFX_MUTEDHIGHLITE:COLOUR_BACKGROUNDEDITOR_GFX_HIGHLITE;
			else
				col=playable==false?COLOUR_BACKGROUNDEDITOR_GFX_MUTED:COLOUR_BACKGROUNDEDITOR_GFX;

			bitmap->guiFillRectX0(y,bitmap->GetX2(),y2,col);
		}
	}

	/*
	if(track->record==true)
	{
	int sx=0,
	sx2=editor->pattern->x2,
	sy=y,
	sy2=y2;

	sx+=3;
	sx2-=3;
	sy+=2;
	sy2-=2;

	if(sx<=sx2 && sy<=sy2)
	bitmap->guiFillRect(sx,sy,sx2,sy2,COLOUR_RED);
	else
	bitmap->guiFillRectX0(y,bitmap->width,y2,COLOUR_RED);
	}
	*/

	/*
	if(automationtrack)
	{
	nulline=automationtrack->DrawScale(bitmap,editor->timeline->x,y,editor->timeline->x2,y2);
	bitmap->guiFillRect(1,nulline+1,bitmap->GetX2(),y2-1,COLOUR_BLUE_LIGHT);
	}
	*/

	// Track Lines

	if(track->song->GetFocusTrack()==track)
	{
		bitmap->guiDrawLineYX0(y,bitmap->GetX2(),track->IsSelected()==true?COLOUR_FOCUSOBJECT_SELECTED:COLOUR_FOCUSOBJECT);
		bitmap->guiDrawLineYX0(y2,bitmap->GetX2());
	}
	else
	{
		bitmap->guiDrawLineYX0(y,bitmap->GetX2(),COLOUR_RASTER1);
		bitmap->guiDrawLineYX0(y2,bitmap->GetX2(),COLOUR_RASTER2);
	}
}

bool Edit_Arrange::SizePattern(bool test)
{
	if(sizepattern)
	{
		//	MessageBeep(-1);

		if(GetMousePosition()>=0)
			switch(mousemode)
		{
			case EM_SIZEPATTERN_RIGHT:
				{
					TRACE ("Size Pattern End %d Now %d\n",modestartposition,GetMousePosition());

					/// Right <<<<< |
					//int endposition=sizepattern->GetPatternEnd();

					//if(endposition<=posnow)
					{
						LONGLONG endoffset=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(GetMousePosition(),modestartposition-GetMousePosition());
						endoffset=sizepattern->GetOffSetEnd()-endoffset;

						bool ok=sizepattern->SetOffSetEnd(endoffset,true);

						if(ok==false && GetMousePosition()>modestartposition && test==false)
							ok=sizepattern->SetOffSetEnd(endoffset=0,true);

						if(ok==true && test==false)
							mainedit->SizePattern(sizepattern,GetMousePosition(),endoffset,true);

						return ok;
					}
				}
				break;

			case EM_SIZEPATTERN_LEFT:
				{
					TRACE ("Size Pattern Start %d Now %d\n",modestartposition,GetMousePosition());

					/// Left <<<<< |
					//int startposition=sizepattern->GetPatternStart();

					//if(startposition>=posnow)
					{
						OSTART pos=GetMousePosition();

						LONGLONG startoffset=WindowSong()->timetrack.ConvertTicksToTempoSamplesStart(modestartposition,GetMousePosition()-modestartposition);
						startoffset+=sizepattern->GetOffSetStart();
						bool ok=sizepattern->SetOffSetStart(GetMousePosition(),startoffset,true);

						if(ok==false && GetMousePosition()<modestartposition && 
							test==false && sizepattern->GetUseOffSetRegion()==true &&
							sizepattern->offsetstartoffset!=0)
						{
							pos=sizepattern->GetPatternStart()-sizepattern->offsetstartoffset;
							if(pos>=0)ok=sizepattern->SetOffSetStart(pos,startoffset=0,true);
						}

						if(ok==true && test==false)
							mainedit->SizePattern(sizepattern,pos,startoffset,false);

						return ok;
					}
				}
				break;
		}
	}

	return false;
}

void Edit_Arrange::MoveAutomationParameters()
{
	int my=pattern->GetMouseY();

	if(moveautomationtrack)
	{
		Edit_Arrange_AutomationTrack *eat=FindAutomationTrack(moveautomationtrack);

		if(eat)
		{
			InitMousePosition();
			OSTART p=GetMousePosition();

			if(p<0)
				return;

			bool copy=maingui->GetCtrlKey();

			if(CanMoveX()==true)
			{
				// Horizontal <>
				OSTART diff=p-modestartposition;

				if(diff)
				{
					mainthreadcontrol->LockActiveSong();

					moveautomationtrack->MoveOrCopySelectedAutomationParameters(diff,copy);

					moveautomationtrack->Refresh();
					mainthreadcontrol->UnlockActiveSong();

					if(copy==false)
						modestartposition=p;
				}
			}

			if(my!=moveautomationstarty && CanMoveY()==true)
			{
				// Value Vert
				double hy=eat->y2-eat->y;

				if(hy==0)
					return;

				double diff=moveautomationstarty-my;

				diff/=hy;

				if(copy==false)
					moveautomationstarty=my;

				double max=2;

				AutomationParameter *ap=moveautomationtrack->FirstAutomationParameter();

				while(ap)
				{
					if( (copy==true && (ap->flag&AutomationParameter::AP_COPYIED)) || 
						(copy==false && ap->IsSelected()==true))
					{
						double v=ap->GetParameterValue();

						if(diff>0)
						{
							if(v==1)
								return;

							v+=diff;

							if(v>1)
							{
								double h=1-ap->GetParameterValue();

								if(max==2 || h<max)
									max=h;
							}
						}
						else
						{
							if(v==0)
								return;

							v+=diff;
							if(v<0)
							{
								double h=-ap->GetParameterValue();

								if(max==2 || h>max)
									max=h;
							}
						}
					}

					ap=ap->NextAutomationParameter();
				}

				if(max!=2)
				{
					diff=max;
				}

				if(diff)
				{
					mainthreadcontrol->LockActiveSong();

					ap=moveautomationtrack->FirstAutomationParameter();

					while(ap)
					{
						if( (copy==true && (ap->flag&AutomationParameter::AP_COPYIED)) || 
							(copy==false && ap->IsSelected()==true))
						{
							double v=ap->GetParameterValue();

							v+=diff;

							if(ap->value!=v)
							{
								moveautomationtrack->parameter.accesscounter++;
								ap->value=v;

#ifdef DEBUG
								if(ap->value>1)
									maingui->MessageBoxError(0,"AP >1");
								if(ap->value<0)
									maingui->MessageBoxError(0,"AP <0");
#endif
							}
						}

						ap=ap->NextAutomationParameter();
					}

					mainthreadcontrol->UnlockActiveSong();
				}
			}
		}
	}
}

void Edit_Arrange::ShowSizePatternSprites()
{
#ifdef OLDIE
	if(mousemode==EM_SIZEPATTERN_RIGHT || mousemode==EM_SIZEPATTERN_LEFT)
	{
		OSTART newdiff=mouseposition-modestartposition;

		if(selection.movediff!=newdiff)
		{
			selection.movediff=newdiff;

			if(SizePattern(true)==true)
			{
				RemoveAllSprites(guiSprite::SPRITEDISPLAY_MOVE);

				TRACE ("Size %d\n",newdiff);
				Edit_Arrange_Track *et=FindTrack(sizepattern->track);

				if(et)
				{
					if(guiSprite *es=new guiSprite(guiSprite::SPRITETYPE_RECT,guiSprite::SPRITEDISPLAY_MOVE))
					{
						OSTART start,end;

						if(modestartposition<mouseposition)
						{
							start=modestartposition;
							end=mouseposition;
						}
						else
						{
							start=mouseposition;
							end=modestartposition;
						}

						if(start<startposition)
						{
							start=startposition;
							es->subtype=SPRITETYPE_RECT_NOLEFT;
						}

						es->x=timeline->ConvertTimeToX(start,frame_pattern.x2);

						if(end<endposition)
							es->x2=timeline->ConvertTimeToX(end,frame_pattern.x2);
						else
						{
							es->x2=frame_pattern.x2;
							es->subtype|=SPRITETYPE_RECT_NORIGHT;
						}

						es->y=et->y;
						es->y2=et->y2;
						es->colour=COLOUR_GREEN;

						if(es->y2>frame_pattern.y2)
							es->y2=frame_pattern.y2;

						AddSprite(es);
					}//if es

					ShowAllSprites();
				}
			}
		}
	}
#endif

}

void Edit_Arrange::ShowMovePatternSprites(bool force)
{
	InitMousePosition();

	if(GetMousePosition()==-1)
		return;

	int my=pattern->GetMouseY();

	Seq_Track *track=FindTrackAtY(my);

	if(!track)
		track=my<0?FirstTrack():LastTrack();

	if(track)
	{
		OSTART newdiff=CanMoveX()==true?GetMousePosition()-modestartposition:0,
			firststartpos=selection.FirstPatternPosition();

		// Ticks Diff -------------
		if(newdiff<-firststartpos)newdiff=-firststartpos;

		// Track Move
		int firststarttrack=selection.FirstTrackNumber(),lasttrack=selection.LastTrackNumber()+1,
			moveobjects_vert=CanMoveY()==true?WindowSong()->GetOfTrack(track)-WindowSong()->GetOfTrack(modestarttrack):0;

		if(moveobjects_vert<-firststarttrack) // Top
			moveobjects_vert=-firststarttrack;
		else
		{
			if(lasttrack+moveobjects_vert>WindowSong()->GetCountOfTracks()) // Bottom
				moveobjects_vert=WindowSong()->GetCountOfTracks()-lasttrack;
		}

		if(force==true || selection.moveobjects_vert!=moveobjects_vert || selection.movediff!=newdiff)
		{
			selection.moveobjects_vert=moveobjects_vert;
			selection.movediff=newdiff;

			TRACE ("NEWMOVE %d NEWDIFF %d\n",moveobjects_vert,newdiff);

			DrawDBBlit(pattern);
		}

	}
}

void Edit_Arrange_Track::ShowMute()
{
	mute.status=track->GetMute();

	if(mute.ondisplay==false)
		return;

	bitmap->ShowMute(&mute,mute.status,bgcolour);

	//bitmap->guiDrawImage(mute.x,mute.y,mute.x2,mute.y2,maingui->gfx.FindBitMap(track->t_mute==false?IMAGE_TRACKMUTEOFF:IMAGE_TRACKMUTEON));
}

void Edit_Arrange_Track::ShowAudioInputMonitoring()
{
	audioinputmonitoring.status=track->io.inputmonitoring;

	if(audioinputmonitoring.ondisplay==false)
		return;

	bitmap->ShowInputMonitoring(&audioinputmonitoring,track,audioinputmonitoring.status,bgcolour);
}

void Edit_Arrange_Track::ShowSolo()
{
	solostatus=track->GetSoloStatus();

	if(solo.ondisplay==false)
		return;

	bitmap->ShowSolo(&solo,solostatus,bgcolour);
}

void Edit_Arrange_Track::ShowRecordMode()
{
	record.recordmode=track->recordtracktype;
	record.status=track->record;

	if(record.ondisplay==false)
		return;

	bitmap->ShowRecord(&record,track,record.status,bgcolour);
}

void Edit_Arrange::ShowHoriz(bool showtracks,bool showheader,bool showoverview)
{
	DrawDBBlit(showtracks==true?tracks:0,pattern,showoverview==true?overview:0);
	if(showheader==true)
		DrawDBBlit(header);
}

#ifdef OLDIE
void Edit_Arrange::TempoChanged()
{
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{
		if(ot->id==OBJECTID_ARRANGETRACK)
		{
			Edit_Arrange_Track *et=(Edit_Arrange_Track *)ot;

			if(et->track->FirstPattern(MEDIATYPE_AUDIO))
				goto show;
		}

		ot=ot->NextObject();
	}

	return;

show:
	ShowHoriz(false,false,true);
}
#endif

void Edit_Arrange_Track::ShowName()
{
	if(namestring)delete namestring;
	namestring=0;

	if(name.ondisplay==false)
		return;


	//if(name.ondisplay==true)
	{
		/*
		int colour;

		if(track->parent)
		{
		colour=COLOUR_YELLOW;
		}
		else
		{
		if(track->flag & OFLAG_SELECTED)
		colour=COLOUR_WHITE;
		else
		colour=COLOUR_GREY_LIGHT;
		}

		if(track->record==true)
		colour=COLOUR_RED_LIGHT;
		*/

		//int h=(name.y2-name.y)/2;

		bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
		bitmap->SetTextColour(COLOUR_BLACK);
		bitmap->guiFillRect(name.x,name.y,name.x2,name.y2,bgcolour);
		//bitmap->guiFillRect(name.x,name.y+h,name.x2,name.y2,track->recordtracktype==TRACKTYPE_AUDIO?COLOUR_YELLOW_LIGHT:COLOUR_GREY_LIGHT);
		//bitmap->guiDrawRect(name.x,name.y,name.x2,name.y2,(track->flag & OFLAG_SELECTED)?COLOUR_BLACK:COLOUR_GREY_DARK);
		//}

		//char hnr[NUMBERSTRINGLEN];

		namestring=mainvar->GenerateString(track->GetName());

		//	char *hnamestring=mainvar->GenerateString(mainvar->ConvertIntToChar(track->song->GetOfTrack(track)+1,hnr),track->parent?"c ":" ",track->GetName());

		if(namestring)
		{
			size_t nl=strlen(namestring);

			if(showfreeze_flag)
				nl+=64;

			if(track->parent)
				nl+=8;

			if(char *h=new char[nl+1])
			{
				h[0]=0;

				if(showfreeze_flag)
				{
					char h2[NUMBERSTRINGLEN];

					if(showfreeze_flag&SHOWFREEZE_CREATING){
						strcpy(h,mainvar->ConvertDoubleToChar(freezeprogress,h2,2));
						mainvar->AddString(h,"% Freezing ");
					}
					else
						if(showfreeze_flag&SHOWFREEZE_WAITING)
							strcpy(h,"Waiting(Freezing) ");

					mainvar->AddString(h,track->GetName());
				}
				else
				{
					strcpy(h,namestring);
				}

				bitmap->guiDrawText(name.x+1,name.y2,name.x2-1,h);
				//if(track->IsSelected()==true)bitmap->guiDrawText(name.x+2,name.y2-1,name.x2-1,h);

				delete h;
			}
			//	delete hnamestring;
		}
	}
}

int Edit_Arrange_Track::GetType()
{
	int status=0;

	Seq_Pattern *p=track->FirstPattern();

	while(p)
	{
		switch(p->mediatype)
		{
		case MEDIATYPE_MIDI:
			status|=ETRACKTYPE_MIDI;
			break;

		case MEDIATYPE_AUDIO:
			status|=ETRACKTYPE_AUDIOFILE;
			break;
		}

		p=p->NextPattern();
	}

	if(track->MIDItype==Seq_Track::OUTPUTTYPE_AUDIOINSTRUMENT)
		status|=ETRACKTYPE_AUDIOINSTRUMENT;

	return status;
}

void Edit_Arrange_Track::ShowTrackInfo()
{
	if(info.ondisplay==false)
		return;

	frozen=track->frozen;
	hasinstruments=track->io.audioeffects.CheckIfEffectHasOnInstruments();
	hasfx=track->io.audioeffects.CheckIfEffectHasNonInstrumentFX();

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);

	int col;
	char *f=0;

	if(track->frozen==true)
	{
		col=COLOUR_FROZEN;

		bitmap->SetTextColour(COLOUR_WHITE);

		if(hasinstruments==true)
		{
			if(hasfx==true)
				f="*iX";
			else
				f="*i";
		}
		else
		{
			if(hasfx==true)
				f="*X";
			else
				f="*";
		}
	}
	else
	{
		col=COLOUR_GREY;

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
	}

	if(f)
	{
		bitmap->guiFillRect(info.x,info.y,info.x2,info.y2,col,COLOUR_INFOBORDER);
		bitmap->guiDrawTextCenter(info.x+2,info.y,info.x2,info.y2,f);
	}
	else
		bitmap->guiFillRect(info.x,info.y,info.x2,info.y2,bgcolour);

}

void Edit_Arrange_Track::ShowMIDIInput()
{
	if(MIDIinput.ondisplay==false)
		return;

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->guiFillRect(MIDIinput.x,MIDIinput.y,MIDIinput.x2,MIDIinput.y2,COLOUR_AUDIOINVOLUME_BACKGROUND);
	bitmap->SetTextColour(COLOUR_YELLOW);

	char *n=track->GetMIDIInputString(false);

	if(n)
	{
		bitmap->guiDrawText(MIDIinput.x+2,MIDIinput.y2,MIDIinput.x2-2,n);
		delete n;
	}
}

void Edit_Arrange_Track::ShowInput()
{
	if(input.ondisplay==false)
		return;

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->guiFillRect(input.x,input.y,input.x2,input.y2,COLOUR_AUDIOINVOLUME_BACKGROUND);
	bitmap->SetTextColour(COLOUR_TEXT);

	char *n=0;

	if(track->io.in_vchannel)
		n=mainvar->GenerateString(track->io.in_vchannel->name);

	if(n)
	{
		bitmap->guiDrawText(input.x+2,input.y2,input.x2-2,n);
		delete n;
	}
}

void Edit_Arrange_Track::ShowChannels()
{
	channels.channeltype=track->io.channel_type;

	if(channels.ondisplay==false)
		return;

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->guiFillRect(channels.x,channels.y,channels.x2,channels.y2,COLOUR_AUDIOTYPE_BACKGROUND);
	bitmap->SetTextColour(COLOUR_TEXT);

	bitmap->guiDrawText(channels.x+1,channels.y2-1,channels.x2,channelchannelsinfo_short[channels.channeltype]);
}

void Edit_Arrange_Track::ShowType()
{
	type.status=GetType();

	if(type.ondisplay==false)
		return;

	bitmap->SetFont(track->IsSelected()==true?&maingui->standard_bold:&maingui->standardfont);
	bitmap->guiFillRect(type.x,type.y,type.x2,type.y2,COLOUR_AUDIOTYPE_BACKGROUND);
	bitmap->SetTextColour(COLOUR_TEXT);

	char h[128];

	h[0]=0;

	if(track->parent)strcpy(h,"Child:");

	if(type.status==0 || track->FirstPattern()==0)
	{
		if(type.status&ETRACKTYPE_AUDIOINSTRUMENT)
			mainvar->AddString(h,"-Instr-"); // empty Instruments
		else
			mainvar->AddString(h,"- - -"); // empty
	}
	else
	{
		if(type.status&ETRACKTYPE_AUDIOINSTRUMENT)
			mainvar->AddString(h,"Instr");

		if(type.status&ETRACKTYPE_MIDI)
			mainvar->AddString(h,"MIDI ");

		if(type.status&ETRACKTYPE_AUDIOFILE)
			mainvar->AddString(h,"Audio ");
	}

	bitmap->guiDrawText(type.x+1,type.y2-1,type.x2,h);
}


void Edit_Arrange_Track::Refresh(int flag)
{
	if(flag&REFRESH_EAT_PATTERNANDTRACK)
	{
		ShowTrack(true);
	}

	if(flag&(REFRESH_EAT_PATTERNANDTRACK|REFRESH_EAT_PATTERN))
	{
		if(track->ismetrotrack==false)
		{
			ShowPatternBGRaster();

			// Draw Pattern
			guiObject *ot=editor->guiobjects.FirstObject();

			// create EditArrangePatternLists
			while(ot)
			{
				switch(ot->id)
				{	
				case OBJECTID_ARRANGEPATTERN:
					{
						Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)ot;

						if(eap->pattern->track==track)	
							eap->ShowPattern();
					}
				}

				ot=ot->NextObject();
			}
		}
	}

	/*
	if(flag&REFRESH_EAT_COLOUR)
	{
	guiObject *ot=editor->guiobjects.FirstObject();

	// create EditArrangePatternLists
	while(ot)
	{
	switch(ot->id)
	{	
	case OBJECTID_ARRANGEAUTOMATIONTRACK:
	{
	Edit_Arrange_AutomationTrack *est=(Edit_Arrange_AutomationTrack *)ot;

	if(est->automationtrack->track=this->track)
	{	
	est->ShowRaster();
	est->ShowAutomationObjects();
	}
	}
	}

	ot=ot->NextObject();
	}
	}
	*/
}

void Edit_Arrange_Track::ShowTrack(bool refreshbgcolour)
{
	playable=track->CheckIfPlaybackIsAble();

	if(refreshbgcolour==true)
	{
		ShowTrackRaster();
	}

	track->GetColour()->Clone(&trackcolour);

	// Mute / Solo / Record
	ShowMute();
	ShowSolo();

	ShowAudioInputMonitoring();

	ShowRecordMode();

	ShowName();
	ShowType();
	ShowChannels();
	ShowVolume();
	ShowMIDIVolume();

	ShowInput();
	ShowMIDIInput();
	ShowTrackInfo();

	ShowAutomationSettings();
	ShowAutomationMode();

	ShowChildTrackMode();
	ShowMIDIDisplay(true);
	ShowAudioDisplay(true);
}

void Edit_Arrange::EditMarker(Seq_Marker *marker,int x,int y)
{
	if(EditData *edit=new EditData)
	{
		// long position;
		edit->song=WindowSong();
		edit->win=this;
		edit->x=x;
		edit->y=y;
		edit->title=Cxs[CXS_EDIT];
		edit->deletename=false;
		edit->id=EDIT_MARKERNAME;
		edit->type=EditData::EDITDATA_TYPE_STRING;
		edit->helpobject=marker;
		edit->string=marker->string;

		maingui->EditDataValue(edit);
	}
}


void Edit_Arrange::MarkerMenu(Edit_Arrange_Marker *mk,bool leftmouse)
{
#ifdef OLDIE
	Seq_Marker *marker=mk->marker;

	if(DeletePopUpMenu(true))
	{	
		class menu_editmarkername:public guiMenu
		{
		public:
			menu_editmarkername(Edit_Arrange *ed,Seq_Marker *m,int x,int y){editor=ed;marker=m;px=x;py=y;}
			void MenuFunction(){editor->EditMarker(marker,px,py);}

			Edit_Arrange *editor;
			Seq_Marker *marker;
			int px,py;
		};

		popmenu->AddFMenu(marker->GetString(),new menu_editmarkername(this,marker,GetMouseX(),frame_markermap.y2));
		popmenu->AddLine();

		class menu_deletemarker:public guiMenu
		{
		public:
			menu_deletemarker(Seq_Marker *m){marker=m;}

			void MenuFunction()
			{
				marker->GetSong()->textandmarker.DeleteMarker_CalledByGUI(marker);
			} //

			Seq_Marker *marker;
		};
		popmenu->AddFMenu(Cxs[CXS_DELETEMARKER],new menu_deletemarker(marker));

		class menu_editposition:public guiMenu
		{
		public:
			menu_editposition(Edit_Arrange *ed,OSTART p){editor=ed;position=p;}

			void MenuFunction()
			{
				if(editor->NewStartPosition(position,true)==true)
					editor->SyncWithOtherEditors();
			} //

			Edit_Arrange *editor;
			OSTART position;
		};

		class menu_songposition:public guiMenu
		{
		public:
			menu_songposition(Edit_Arrange *ed,OSTART p){editor=ed;position=p;}

			void MenuFunction()
			{
				editor->WindowSong()->SetSongPosition(position,true);
			} //

			Edit_Arrange *editor;
			OSTART position;
		};

		popmenu->AddLine();

		bool sp=false;

		if(startposition!=marker->GetMarkerStart())
		{
			popmenu->AddFMenu(Cxs[CXS_EDITORMARKERSTART],new menu_editposition(this,marker->GetMarkerStart()));
			sp=true;
		}

		if(marker->markertype==Seq_Marker::MARKERTYPE_DOUBLE && startposition!=marker->GetMarkerEnd())
		{
			popmenu->AddFMenu(Cxs[CXS_EDITORMARKEREND],new menu_editposition(this,marker->GetMarkerEnd()));
			sp=true;
		}

		if(sp==true)
			popmenu->AddLine();

		popmenu->AddFMenu(Cxs[CXS_SPPMARKERSTART],new menu_songposition(this,marker->GetMarkerStart()));

		if(marker->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
			popmenu->AddFMenu(Cxs[CXS_SPPMARKEREND],new menu_songposition(this,marker->GetMarkerEnd()));

		if(marker->GetMarkerStart()!=WindowSong()->playbacksettings.cyclestart ||
			(marker->markertype==Seq_Marker::MARKERTYPE_DOUBLE && marker->GetMarkerEnd()!=WindowSong()->playbacksettings.cycleend)
			)
		{
			class menu_changemarkercycle:public guiMenu
			{
			public:
				menu_changemarkercycle(Edit_Arrange *ed,Seq_Marker *m){editor=ed;marker=m;}

				void MenuFunction()
				{
					marker->SetMarkerStartEnd(editor->WindowSong()->playbacksettings.cyclestart,editor->WindowSong()->playbacksettings.cycleend);
					maingui->RefreshMarkerGUI(editor->WindowSong(),marker);
				} //


				Edit_Arrange *editor;
				Seq_Marker *marker;
			};

			popmenu->AddLine();
			popmenu->AddFMenu(Cxs[CXS_CHANGEMARKERPOSITIONTOCYCLEPOSITION],new menu_changemarkercycle(this,marker));

			if(marker->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
			{
				popmenu->AddLine();
				class menu_setcyclemarker:public guiMenu
				{
				public:
					menu_setcyclemarker(Seq_Song *s,Seq_Marker *m){song=s;marker=m;}

					void MenuFunction()
					{
						song->SetCycle(marker->GetMarkerStart(),marker->GetMarkerEnd());
					} 

					Seq_Song *song;
					Seq_Marker *marker;
				};

				popmenu->AddFMenu(Cxs[CXS_SETCYCLEWITHMARKERPOSITIONS],new menu_setcyclemarker(WindowSong(),marker));
			}
		}

		ShowPopMenu();
	}
#endif

}

void Edit_Arrange::FreeEditorPatternMemory()
{
	FreePatternRegionMemory();

	// Sub Track Objects
	guiObject *ot=guiobjects.FirstObject();

	while(ot)
	{
		if(ot->id==OBJECTID_ARRANGEAUTOMATIONTRACK)
		{
			Edit_Arrange_AutomationTrack *et=(Edit_Arrange_AutomationTrack *)ot;
			et->DeleteObjects(); // Pattern+Automation Parameter
		}

		ot=ot->NextObject();
	}
}

void Edit_Arrange::FreeEditorMemory()
{
	FreeEditorPatternMemory();
	guiobjects.RemoveOs(0);
	crossfades.DeleteAllO();
	trackobjects.DeleteAllO(0);
	overviewtrackobjects.DeleteAllO(0);
}

// Automation Tracks
void Edit_Arrange::BuildAddAutomationTracks(OListCoosY *list,TrackHead *t,double h,bool withautomationtracks)
{
	if(t->showautomationstracks==true && withautomationtracks==true)
	{
		AutomationTrack *at=t->FirstAutomationTrack();

		while(at)
		{
			/*
			if((at->bindtoautomationobject==0) ||
			(at->bindtoautomationobject->IsSystem()==true)
			((GetShowFlag()&SHOW_AUDIO) &&  at->bindtoautomationobject->IsAudio()==true) ||
			((GetShowFlag()&SHOW_MIDI) && at->bindtoautomationobject->IsMIDI()==true)
			)
			*/
			{
				if(at->visible==true)
					list->AddCooObject(at,(int)h,0);
				else
					list->AddCooObject(at,maingui->GetFontSizeY()+4,0);
			}

			at=at->NextAutomationTrack();
		}
	}
}

void Edit_Arrange::BuildTrackList(OListCoosY *list,guiGadget_Tab *db,int zoomy,bool withautomationtracks,bool autozoom)
{
	if(!db)return;

	list->DeleteAllO(db);

	if(showmaster==true)
	{	
		int hy;

		AudioChannel *c=&WindowSong()->audiosystem.masterchannel;
		if(autozoom==true)
		{
			double h=zoomybuffermul*zoomy;

			list->AddCooObject(c,hy=(int)h,0);
		}
		else
			list->AddCooObject(c,hy=zoomy,0);

		BuildAddAutomationTracks(list,c,hy,withautomationtracks);
	}

	if(showbus==true)
	{
		AudioChannel *c=WindowSong()->audiosystem.FirstBusChannel();

		while(c)
		{
			int hy;

			if(autozoom==true)
			{
				double h;

				if(c==WindowSong()->audiosystem.GetFocusBus())
					h=zoomybuffermul*zoomy;
				else
					h=zoomy;

				h*=c->sizefactor;

				list->AddCooObject(c,hy=(int)h,0);
			}
			else
				list->AddCooObject(c,hy=zoomy,0);

			BuildAddAutomationTracks(list,c,hy,withautomationtracks);

			c=c->NextChannel();
		}
	}

	if(showmetro==true)
	{
		Seq_MetroTrack *t=WindowSong()->FirstMetroTrack();
		while(t)
		{
			int hy;

			if(autozoom==true)
			{
				double h;

				if(t==WindowSong()->GetFocusMetroTrack())
					h=zoomybuffermul*zoomy;
				else
					h=zoomy;

				if(zoomy<2*maingui->GetButtonSizeY())
					zoomy=2*maingui->GetButtonSizeY();

				list->AddCooObject(t,hy=(int)h,0);
			}
			else
				list->AddCooObject(t,hy=zoomy,0);

			t=t->NextMetroTrack();
		}
	}

	Seq_Track *t=WindowSong()->FirstTrack();
	while(t)
	{
		if(t->CheckTracking(mainsettings->arrangetracking[set])==true)
		{
			int hy;

			if(autozoom==true)
			{
				double h;

				if(t==WindowSong()->GetFocusTrack())
					h=zoomybuffermul*zoomy;
				else
					h=zoomy;

				h*=t->sizefactor;

				list->AddCooObject(t,hy=(int)h,0);
			}
			else
				list->AddCooObject(t,hy=zoomy,0);

			BuildAddAutomationTracks(list,t,hy,withautomationtracks);
		}

		t=t->NextTrack();
	}

	list->EndBuild();
}

int Edit_Arrange::ShowTracks()
{
	FreeEditorPatternMemory();

	guiobjects.RemoveOsFromTo(OBJECTID_ARRANGETRACK,LASTARRANGETRACKID);	

	if(!tracks)return 0;

	if(zoomvert==true)
		trackobjects.BufferYPos();

	BuildTrackList(&trackobjects,tracks,zoomy,GetShowFlag()&SHOW_AUTOMATION?true:false);
	BuildTrackList(&overviewtrackobjects,tracks,zoomy,false);

	if(zoomvert==true)
		trackobjects.RecalcYPos();

	ShowVSlider();
	trackobjects.InitYStartO();

	tracks->ClearTab();

	TRACE ("AR Tracks Init Y %d\n",trackobjects.GetInitY());

	if(trackobjects.GetShowObject()) // first track ?
	{
		tracks->gbitmap.SetFont(&maingui->standardfont);

		int sizemininame=tracks->gbitmap.GetTextWidth("ABCDEF");
		int startnumberw=tracks->gbitmap.GetTextWidth("1234.");
		int w=tracks->gbitmap.GetTextWidth(" mR ");
		int wrt=tracks->gbitmap.GetTextWidth("AA Rec:Audio");
		int wdataoutput=maingui->GetFontSizeY();

		// Create Track List
		while(trackobjects.GetShowObject() && trackobjects.GetInitY()<tracks->GetHeight())
		{
			switch(trackobjects.GetShowObject()->object->id)
			{
			case OBJ_AUDIOCHANNEL:
				{
					AudioChannel *c=(AudioChannel *)trackobjects.GetShowObject()->object;

					if(Edit_Arrange_Channel *et=new Edit_Arrange_Channel)
					{
						int startx=startnumberw;

						et->channel=c;
						et->startnamex=startx;
						et->editor=this;
						et->bitmap=&tracks->gbitmap;

						guiobjects.AddTABGUIObject(0,trackobjects.GetInitY(),tracks->GetX2(),trackobjects.GetInitY2(),tracks,et);

						int xx=startx+1;
						int hxstart=xx;
						int namexs=xx+w+2;
						int datax=tracks->GetX2();

						if(namexs+sizemininame<datax)
						{
							et->dataoutputx=datax-2*wdataoutput;
							et->dataoutputx2=datax-1;
							et->datadisplay=true;
						}
						else
						{
							et->dataoutputx=datax;
							et->datadisplay=false;
						}

						int sy1=trackobjects.GetInitY()+1;
						int sy2=sy1+maingui->GetButtonSizeY();

						// Mute ------------------------------------
						guiobjects.AddGUIObject(xx,sy1,xx+w,sy2,tracks,&et->mute);
						xx+=w+1;

						if(c->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL)
						{
							// Solo
							guiobjects.AddGUIObject(xx,sy1,xx+w,sy2,tracks,&et->solo);
						}

						if(sy2+maingui->GetButtonSizeY()<et->y2)
							guiobjects.AddGUIObject(2,sy2,startx-2,sy2+maingui->GetButtonSizeY(),tracks,&et->channeltype); // Channels

						xx+=w+1;
						xx+=w+1; // Record
						xx+=w+1; // Input

						// Info
						guiobjects.AddGUIObject(xx,sy1,xx+w,sy2,tracks,&et->info);

						xx+=w+1;

						// Name
						guiobjects.AddGUIObject(xx,sy1,datax-1,sy2,tracks,&et->name);

						int endx2=et->datadisplay==true?et->dataoutputx-3:datax-2;

						if(GetShowFlag()&SHOW_AUTOMATION)
						{
							if(et->y2-et->y<5*maingui->GetButtonSizeY())
								endx2-=2*sizemininame;
						}

						int vy=sy2+ADDYSPACE+1;
						int vy2=vy+3+maingui->GetButtonSizeY();

						if(GetShowFlag()&SHOW_AUDIO)
						{
							// Audio ?
							if(vy2<et->y2 && endx2>hxstart+ADDXSPACE)
							{
								// Audio Input
								int vx=hxstart;

								// Volume
								guiobjects.AddGUIObject(vx,vy,endx2,vy2,tracks,&et->volume);

								vy2+=ADDYSPACE+1;
								vy=vy2;
								vy2=vy+3+maingui->GetButtonSizeY();
							}
						}

						if((GetShowFlag()&SHOW_MIDI) && c->audiochannelsystemtype==CHANNELTYPE_MASTER)
						{
							// MIDI ?
							if(vy2<et->y2 && endx2>hxstart+ADDXSPACE)
							{
								// Audio Input
								int vx=hxstart;

								// MIDI Volume
								guiobjects.AddGUIObject(vx,vy,endx2,vy2,tracks,&et->MIDIvolume);

								vy2+=ADDYSPACE+1;
								vy=vy2;
								vy2=vy+3+maingui->GetButtonSizeY();
							}
						}

						if(GetShowFlag()&SHOW_AUTOMATION)
						{
							// Auto Settings
							et->automationsettings.x=et->dataoutputx-2*sizemininame;
							et->automationsettings.y=trackobjects.GetInitY2()-maingui->GetFontSizeY();
							et->automationsettings.x2=(et->dataoutputx-2)-maingui->GetFontSizeY();
							et->automationsettings.y2=trackobjects.GetInitY2();

							if(et->automationsettings.x>startx)
							{
								guiobjects.AddGUIObject(&et->automationsettings,tracks);

								et->automation.x=et->dataoutputx-maingui->GetFontSizeY();
								et->automation.y=trackobjects.GetInitY2()-maingui->GetFontSizeY();
								et->automation.x2=et->dataoutputx;
								et->automation.y2=trackobjects.GetInitY2();

								if(et->automation.x>startx)
									guiobjects.AddGUIObject(&et->automation,tracks);
							}
						}
					}
				}
				break;

			case OBJ_TRACK:
				{
					Seq_Track *t=(Seq_Track *)trackobjects.GetShowObject()->object;

					if(Edit_Arrange_Track *et=new Edit_Arrange_Track)
					{
						int startx=startnumberw+w*t->childdepth;

						et->trackselected=t->IsSelected();
						et->startnamex=startx;
						et->editor=this;
						et->bitmap=&tracks->gbitmap;
						et->track=t;

						// Zoom Y2
						//et->dataoutputy2=trackobjects.GetInitY2();

						guiobjects.AddTABGUIObject(0,trackobjects.GetInitY(),tracks->GetX2(),trackobjects.GetInitY2(),tracks,et);

						int xx=startx+1;

						if(et->track->FirstChildTrack())
							xx+=w;

						int hxstart=xx;
						int sy1=trackobjects.GetInitY()+1;
						int sy2=sy1+maingui->GetButtonSizeY();

						// Mute ------------------------------------
						guiobjects.AddGUIObject(xx,sy1,xx+w,sy2,tracks,&et->mute);
						xx+=w+1;

						if(sy2+maingui->GetButtonSizeY()<et->y2)
							guiobjects.AddGUIObject(2,sy2,startx-2,sy2+maingui->GetButtonSizeY(),tracks,&et->channels); // Channels

						if(et->track->ismetrotrack==false)
						{
							// Solo
							guiobjects.AddGUIObject(xx,sy1,xx+w,sy2,tracks,&et->solo);
							xx+=w+1;

							// Record -----------------------------
							guiobjects.AddGUIObject(xx,sy1,xx+w,sy2,tracks,&et->record);
							xx+=w+1;

							// Input Monitoring -----------------------------
							guiobjects.AddGUIObject(xx,sy1,xx+w,sy2,tracks,&et->audioinputmonitoring);


							// Info
							xx+=w+1;

							guiobjects.AddGUIObject(xx,sy1,xx+w,sy2,tracks,&et->info);
						}

						/*
						tracks->formchild->SetGXY(startx+1,trackobjects.GetInitY()+1+17);

						if(guiGadget_Volume *g=glist.AddVolumeButton(-1,-1,w,-1,0,et->track->io.audioeffects.volume.value,MODE_PARENT))
						{
						//guiobjects.AddGUIObject(g,tracks,new Edit_AudioMix_SendEdit(this,as,g,as->sendvolume));
						//sendvolumegadgets.AddEndO(g);
						}
						*/

						int namexs=xx+w+2;
						int datax=tracks->GetX2();

						if(namexs+sizemininame<datax)
						{
							et->dataoutputx=datax-2*wdataoutput;
							et->dataoutputx2=datax-1;
							et->datadisplay=true;
						}
						else
						{
							et->dataoutputx=datax;
							et->datadisplay=false;
						}

						int endx2=et->datadisplay==true?et->dataoutputx-3:datax-2;

						if(GetShowFlag()&SHOW_AUTOMATION)
						{
							if(et->y2-et->y<5*maingui->GetButtonSizeY())
								endx2-=2*sizemininame;
						}

						int vy=sy2+ADDYSPACE+1;
						int vy2=vy+3+maingui->GetButtonSizeY();

						if(GetShowFlag()&SHOW_AUDIO)
						{
							// Audio ?
							if(vy2<et->y2 && endx2>hxstart+ADDXSPACE)
							{
								// Audio Input
								int vx=hxstart;

								if(et->track->ismetrotrack==false)
								{
									if(GetShowFlag()&SHOW_IO)
									{
										int midx=(endx2-hxstart)-ADDXSPACE;
										midx/=2;

										// Input
										guiobjects.AddGUIObject(vx,vy,vx+midx-1,vy2,tracks,&et->input);
										vx+=midx+ADDXSPACE+1;
									}
								}

								// Volume
								guiobjects.AddGUIObject(vx,vy,endx2,vy2,tracks,&et->volume);

								vy2+=ADDYSPACE+1;
								vy=vy2;
								vy2=vy+3+maingui->GetButtonSizeY();
							}
						}

						if(GetShowFlag()&SHOW_MIDI)
						{
							// MIDI ?
							if(vy2<et->y2 && endx2>hxstart+ADDXSPACE)
							{
								// Audio Input
								int vx=hxstart;

								if(et->track->ismetrotrack==false)
								{
									if(GetShowFlag()&SHOW_IO)
									{
										int midx=(endx2-hxstart)-ADDXSPACE;
										midx/=2;

										// Volume Input
										guiobjects.AddGUIObject(vx,vy,vx+midx-1,vy2,tracks,&et->MIDIinput);

										vx+=midx+ADDXSPACE+1;
									}
								}

								// MIDI Volume
								guiobjects.AddGUIObject(vx,vy,endx2,vy2,tracks,&et->MIDIvolume);

								vy2+=ADDYSPACE+1;
								vy=vy2;
								vy2=vy+3+maingui->GetButtonSizeY();
							}
						}

						// Name
						guiobjects.AddGUIObject(namexs,sy1,endx2,sy2,tracks,&et->name);

						if(et->track->ismetrotrack==false)
						{
							if(GetShowFlag()&SHOW_EVENTTYPE)
							{
								if(vy2<et->y2)
								{
									// Type
									guiobjects.AddGUIObject(namexs,vy,endx2,vy2,tracks,&et->type);
								}
							}

							// Child Tracks
							if(et->track->FirstChildTrack())
							{
								et->child.x=et->startnamex;
								et->child.y=trackobjects.GetInitY()+1;
								et->child.x2=et->child.x+w;
								et->child.y2=trackobjects.GetInitY2();

								guiobjects.AddGUIObject(&et->child,tracks);
							}

							if(GetShowFlag()&SHOW_AUTOMATION)
							{
								// Auto Settings
								et->automationsettings.x=et->dataoutputx-2*sizemininame;
								et->automationsettings.y=trackobjects.GetInitY2()-maingui->GetFontSizeY();
								et->automationsettings.x2=(et->dataoutputx-2)-maingui->GetFontSizeY();
								et->automationsettings.y2=trackobjects.GetInitY2();

								if(et->automationsettings.x>startx)
								{
									guiobjects.AddGUIObject(&et->automationsettings,tracks);

									// Auto Up/Down Visible

									et->automation.x=et->dataoutputx-maingui->GetFontSizeY();

									if(et->automation.x>startx)
									{
										et->automation.y=trackobjects.GetInitY2()-maingui->GetFontSizeY();
										et->automation.x2=et->dataoutputx;
										et->automation.y2=trackobjects.GetInitY2();

										guiobjects.AddGUIObject(&et->automation,tracks);
									}
								}
							}
						}

						//	y=y2+1;	

						// Colour
						/*
						if(t->IsSelected()==true)
						et->bgcolour=COLOUR_SELECTED;
						else
						if(t==WindowSong()->GetFocusTrack())
						et->bgcolour=COLOUR_YELLOW_LIGHT;
						else
						et->bgcolour=COLOUR_RED;
						*/

						//et->ShowTrackRaster();
						//	if(showtracknumber==true)et->ShowTrackNumber();

					} // if et
				}
				break;

			case OBJ_AUTOMATIONTRACK:
				{
					if(Edit_Arrange_AutomationTrack *eat=new Edit_Arrange_AutomationTrack)
					{
						AutomationTrack *at=(AutomationTrack *)trackobjects.GetShowObject()->object;

						int startx;

						if(at->track)
							startx=startnumberw+w*at->track->childdepth;
						else
							startx=startnumberw;

						eat->startnamex=startx;
						eat->editor=this;
						eat->bitmap=&tracks->gbitmap;
						eat->automationtrack=at;

						guiobjects.AddTABGUIObject(0,trackobjects.GetInitY(),tracks->GetX2(),trackobjects.GetInitY2(),tracks,eat);

						int sy1=trackobjects.GetInitY()+1;
						int sy2=sy1+maingui->GetButtonSizeY();

						int datax=tracks->GetX2()-wdataoutput;

						if((startx+1)+sizemininame<datax)
						{
							eat->dataoutputx=datax;
							eat->dataoutputx2=datax-1;
							eat->datadisplay=true;
						}
						else
						{
							eat->dataoutputx=datax;
							eat->datadisplay=false;
						}

						guiobjects.AddGUIObject(4,sy1+1,startx-8,sy2-1,tracks,&eat->visible);

						// Name
						guiobjects.AddGUIObject(startx+1,sy1,datax-1,sy2,tracks,&eat->name);

						// Mode
						int vy=sy2+ADDYSPACE+1;
						int vy2=vy+3+maingui->GetButtonSizeY();

						if(vy2<eat->y2)
						{
							int w=bitmap.GetTextWidth("AUTOMODE");

							guiobjects.AddGUIObject(startx+1,vy,startx+1+w,vy2,tracks,&eat->mode);
							guiobjects.AddGUIObject(startx+1+w+2,vy,datax-1,vy2,tracks,&eat->value);
						}

						guiobjects.AddGUIObject(datax+1,trackobjects.GetInitY()+1,tracks->GetX2()-1,trackobjects.GetInitY2()-1,tracks,&eat->vu);

						//eat->ShowTrackRaster();
					}
				}
				break;

#ifdef DEBUG
			default:
				maingui->MessageBoxError(0,"Show Tracks ?");
				break;
#endif
			}

			trackobjects.NextYO();

		}// while list

		//	TRACE ("numberoftracks %d\n",numberoftracks);

		// Show Tracks/Automation

		guiObject_Pref *o=tracks->FirstGUIObjectPref();
		while(o)
		{
			switch(o->gobject->id)
			{
			case OBJECTID_ARRANGETRACK:
				{
					Edit_Arrange_Track *et=(Edit_Arrange_Track *)o->gobject;
					et->ShowTrack(true);
				}
				break;

			case OBJECTID_ARRANGEAUTOMATIONTRACK:
				{
					Edit_Arrange_AutomationTrack *eat=(Edit_Arrange_AutomationTrack *)o->gobject;
					eat->ShowTrack(true);
				}
				break;

			case OBJECTID_ARRANGECHANNEL:
				{
					Edit_Arrange_Channel *eac=(Edit_Arrange_Channel *)o->gobject;
					eac->ShowChannel(true);
				}

				break;


			}

			o=o->NextGUIObjectPref();
		}

	}// if t

	trackobjects.DrawUnUsed(tracks);

	return -1;
}

void Edit_Arrange::ShowMarker(int y,int y2)
{
	if(!pattern)
		return;

	if(!timeline)
		return;

	guiBitmap *bitmap=&pattern->gbitmap;

	// Marker ?
	if(y+4<y2)
	{
		Seq_Marker *m=WindowSong()->textandmarker.FirstMarker();

		while(m && m->GetMarkerStart()<=endposition)
		{
			int mx=-1,mx2=-1;

			if(m->GetMarkerStart()>=startposition)
			{
				mx=timeline->ConvertTimeToX(m->GetMarkerStart());

				if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
					mx2=m->GetMarkerEnd()>=endposition?timeline->x2:timeline->ConvertTimeToX(m->GetMarkerEnd());

			}
			else // Start <<....|
				if(m->markertype==Seq_Marker::MARKERTYPE_DOUBLE)
				{
					if(m->GetMarkerEnd()>startposition)
					{
						mx=timeline->x;
						mx2=m->GetMarkerEnd()>=endposition?timeline->x2:timeline->ConvertTimeToX(m->GetMarkerEnd());
					}
				}

				if(mx!=-1)
				{
					int toy2=y2-1;

					if(m->markertype==Seq_Marker::MARKERTYPE_SINGLE)
					{
						bitmap->guiDrawLineX(mx,y+1,toy2,COLOUR_BLACK);

						if(m->colour.showcolour==true)
						{
							for(int hx=mx+1,i=4;i>0;hx+=2,i--)
							{
								if(hx<timeline->x2)
									bitmap->guiDrawLine_RGB(hx,y+1,hx,toy2,m->colour.rgb);
							}
						}
						else
						{
							for(int hx=mx+1,i=4;i>0;hx+=2,i--)
							{
								if(hx<timeline->x2)
									bitmap->guiDrawLineX(hx,y+1,toy2,COLOUR_BLACK);
							}
						}
					}
					else
					{
						if(mx2!=-1)
						{
							bitmap->guiFillRect_RGB(mx,y+1,mx2,toy2,m->colour.showcolour==true?m->colour.rgb:RGB(200,200,255));
						}
					}
				}

				m=m->NextMarker();
		}
	}
}

void Edit_Arrange::ScrollTo(Seq_Track *scrollto)
{
	if(OObject *f=trackobjects.FindObject(scrollto))
	{
		trackobjects.ScrollY(f);
		DrawDBBlit(tracks,pattern);
		ShowVSlider();
	}
}

void Edit_Arrange::KeyDownRepeat()
{
	Editor_KeyDown();

	switch(nVirtKey)
	{
	case KEY_UP10:
	case KEY_CURSORUP:
		{

			if(OObject *no=trackobjects.FindPrevSameObject(trackobjects.FindObject(WindowSong()->GetFocusTrack())))
			{
				WindowSong()->SetFocusTrackLock((Seq_Track *)no->object,true,this);
				ScrollTo(WindowSong()->GetFocusTrack());

				if(listeditor)
					listeditor->ScrollTo(WindowSong()->GetFocusTrack());
			}
		}
		break;

	case KEY_DOWN10:
	case KEY_CURSORDOWN:
		{
			// Active Track ++
			if(OObject *no=trackobjects.FindNextSameObject(trackobjects.FindObject(WindowSong()->GetFocusTrack())))
			{
				WindowSong()->SetFocusTrackLock((Seq_Track *)no->object,true,this);
				ScrollTo(WindowSong()->GetFocusTrack());

				if(listeditor)
					listeditor->ScrollTo(WindowSong()->GetFocusTrack());
			}	
		}
		break;
	}
}

void Edit_Arrange::KeyDown()
{
	Editor_KeyDown();

	switch(nVirtKey)
	{

		/*
		case '1':
		SetSet(0);
		break;

		case '2':
		SetSet(1);
		break;

		case '3':
		SetSet(2);
		break;

		case '4':
		SetSet(3);
		break;

		case '5':
		SetSet(4);
		break;
		*/

	default:
		KeyDownRepeat();
		break;
	}
}

void Edit_Arrange::KeyUp()
{
	Editor_KeyUp();

	switch(nVirtKey)
	{
	case KEYF1: // Event
		{
			globmenu_Event editor(this,startposition);
			editor.MenuFunction();
		}
		break;

	case KEYF2: // Piano
		{
			globmenu_Piano editor(this,startposition);
			editor.MenuFunction();
		}
		break;

	case KEYF4: // Drum
		{
			globmenu_Drum editor(this,startposition);
			editor.MenuFunction();
		}
		break;

	case KEYF6: // Sample
		{
			globmenu_Sample editor(this,startposition);
			editor.MenuFunction();
		}
		break;
	}
}

void Edit_Arrange::ResetMoreEventFlags()
{
	modestartautomationtrack=0;
}

void Edit_Arrange::ShowShowStatus()
{
	if(g_tmaster)
		g_tmaster->Toggle(showmaster);

	if(g_tbus)
		g_tbus->Toggle(showbus);

	if(g_tmetro)
		g_tmetro->Toggle(showmetro);
}

void Edit_Arrange::ShowFocusPattern()
{
	Seq_Pattern *p=WindowSong()->GetFocusPattern();

	if(p)
	{
		if(g_info_name)
			g_info_name->ChangeButtonText(p->GetName());

		if(g_info_pos)
		{
			g_info_pos->SetTime(p->GetPatternStart());
		}

		if(g_info_end)
		{
			g_info_end->SetTime(p->GetPatternEnd());
		}
	}
	else
	{
		glist.Disable(g_info_name);
		glist.Disable(g_info_pos);
		glist.Disable(g_info_end);
	}
}

void Edit_Arrange::Goto(int to)
{
	UserEdit();

	if(CheckStandardGoto(to)==true)
		return;

	switch(to)
	{
	case GOTO_FIRSTTRACK:
		{
			SetStartTrack(WindowSong()->FirstTrack());
		}
		break;

	case GOTO_LASTTRACK:
		{
			SetStartTrack(WindowSong()->LastTrack());
		}
		break;

	case GOTO_FIRSTSELECTEDTRACK:
		{
			SetStartTrack(WindowSong()->FirstSelectedTrack());
		}
		break;

	case GOTO_SOLOTRACK:
		SetStartTrack(WindowSong()->GetSoloTrack(WindowSong()->GetFocusTrack()));
		break;

	case GOTO_MUTETRACK:
		SetStartTrack(WindowSong()->GetMuteTrack(WindowSong()->GetFocusTrack()));
		break;

	case GOTO_FOCUSTRACK:
		SetStartTrack(WindowSong()->GetFocusTrack());
		break;

	case GOTO_FOCUSPATTERN:
		{
			if(WindowSong()->GetFocusPattern())
			{
				SetStartTrack(WindowSong()->GetFocusPattern()->GetTrack());

				if(NewStartPosition(WindowSong()->GetFocusPattern()->GetPatternStart(),true)==true)
					SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_LASTSELECTEDTRACK:
		{
			SetStartTrack(WindowSong()->LastSelectedTrack());
		}
		break;

	case GOTO_FIRST:
		if(WindowSong()->GetFocusTrack())
		{
			Seq_Pattern *p=WindowSong()->GetFocusTrack()->FirstPattern();

			if(p && NewStartPosition(p->GetPatternStart(),true)==true)
				SyncWithOtherEditors();
		}
		break;

	case GOTO_LAST:
		if(WindowSong()->GetFocusTrack())
		{
			Seq_Pattern *p=WindowSong()->GetFocusTrack()->LastPattern();

			if(p && NewStartPosition(p->GetPatternStart(),true)==true)
				SyncWithOtherEditors();
		}
		break;

	case GOTO_FIRSTSONG:
		{
			Seq_Pattern *f=0;
			Seq_Track *t=WindowSong()->FirstTrack();

			while(t)
			{
				if(t->FirstPattern() &&  (f==0 || t->FirstPattern()->GetPatternStart()<f->GetPatternStart()))
					f=t->FirstPattern();

				t=t->NextTrack();
			}

			if(f)
			{
				WindowSong()->SetFocusTrack(f->GetTrack());
				SetStartTrack(WindowSong()->GetFocusTrack());

				if(NewStartPosition(f->GetPatternStart(),true)==true)
					SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_LASTSONG:
		{
			Seq_Pattern *f=0;
			Seq_Track *t=WindowSong()->FirstTrack();

			while(t)
			{
				if(t->LastPattern() && (f==0 || t->LastPattern()->GetPatternStart()>f->GetPatternStart()))
					f=t->LastPattern();

				t=t->NextTrack();
			}

			if(f)
			{
				WindowSong()->SetFocusTrack(f->GetTrack());
				SetStartTrack(WindowSong()->GetFocusTrack());

				if(NewStartPosition(f->GetPatternStart(),true)==true)
					SyncWithOtherEditors();
			}
		}
		break;
	}
}

char *Edit_Arrange::GetToolTipString1() //v
{
#ifdef OLDIE
	char *string=0;

	if(guiObject *o=guiobjects.CheckObjectClicked(GetMouseX(),GetMouseY())) // Track selected ?
	{
		switch(o->id)
		{		
		case OBJECTID_ARRANGEPATTERN:
			{
				Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)o;

				if(eap->pattern->visible==true)
				{
					if(eap->pattern->realpattern==false)
					{
						string=mainvar->GenerateString("Frozen Track Audio");
					}
					else
					{
						if(eap->pattern->itsaclone==true)
							string=mainvar->GenerateString("P_Clone:",eap->pattern->mainpattern->GetName());
						else
							string=mainvar->GenerateString("P:",eap->pattern->GetName());
					}
				}
			}
			break;
		}
	}

	return string;
#endif

	return 0;
}

char *Edit_Arrange::GetToolTipString2()
{
#ifdef OLDIE
	char *string=0;

	if(guiObject *o=guiobjects.CheckObjectClicked(GetMouseX(),GetMouseY())) // Track selected ?
	{
		switch(o->id)
		{

		}
	}

	return string;
#endif

	return 0;
}

Seq_SelectionList *Edit_Arrange::GetPatternSelection()
{
	return CreateSelectionList();
}

void Edit_Arrange::Paste()
{
	if(!pattern)
		return;

	Seq_Track *t=0;

	OSTART position=WindowSong()->GetSongPosition();

	int mx=pattern->GetMouseX();
	int my=pattern->GetMouseY();

	if(mx>=0 && mx<=pattern->GetX2() && my>=0 && my<=pattern->GetY2())
	{
		t=FindTrackAtY(my);
		position=timeline->ConvertXPosToTime(mx);

		// Mouse Quantize
		position=QuantizeEditorMouse(position);
	}

	if(!t)
		t=WindowSong()->GetFocusTrack();

	if(t && t->ismetrotrack==false)
		mainbuffer->PasteBuffer(this,WindowSong(),t,position);
}

void Edit_Arrange::CreateGotoMenu()
{
	DeletePopUpMenu(true);

	AddStandardGotoMenu();

	if(popmenu)
	{
		if(WindowSong()->GetCountOfPattern(0,false))
		{
			popmenu->AddLine();
			popmenu->AddFMenu(Cxs[CXS_FIRSTSONGPATTERN],new menu_gotoarrange(this,GOTO_FIRSTSONG));
			popmenu->AddFMenu(Cxs[CXS_LASTSONGPATTERN],new menu_gotoarrange(this,GOTO_LASTSONG));
			popmenu->AddLine();

			if(WindowSong()->GetFocusTrack() && WindowSong()->GetFocusTrack()->FirstPattern())
			{
				popmenu->AddFMenu(Cxs[CXS_FIRSTTRACKPATTERN],new menu_gotoarrange(this,GOTO_FIRST));
				popmenu->AddFMenu(Cxs[CXS_LASTTRACKPATTERN],new menu_gotoarrange(this,GOTO_LAST));
				popmenu->AddLine();
			}
		}

		if(WindowSong()->FirstTrack())
		{
			popmenu->AddFMenu(Cxs[CXS_FIRSTTRACK],new menu_gotoarrange(this,GOTO_FIRSTTRACK));
			popmenu->AddFMenu(Cxs[CXS_LASTTRACK],new menu_gotoarrange(this,GOTO_LASTTRACK));

			if(WindowSong()->FirstSelectedTrack())
			{
				popmenu->AddFMenu(Cxs[CXS_FIRSTSELECTEDTRACK],new menu_gotoarrange(this,GOTO_FIRSTSELECTEDTRACK));
				popmenu->AddFMenu(Cxs[CXS_LASTSELECTEDTRACK],new menu_gotoarrange(this,GOTO_LASTSELECTEDTRACK));
			}	
		}

		if(WindowSong()->GetSoloTrack())
		{
			popmenu->AddLine();
			popmenu->AddFMenu("Soloed Track",new menu_gotoarrange(this,GOTO_SOLOTRACK));
		}

		if(WindowSong()->GetMuteTrack())
		{
			popmenu->AddLine();
			popmenu->AddFMenu("Muted Track",new menu_gotoarrange(this,GOTO_MUTETRACK));
		}

		if(WindowSong()->GetFocusTrack())
		{
			popmenu->AddLine();
			popmenu->AddFMenu("Focus Track",new menu_gotoarrange(this,GOTO_FOCUSTRACK));
		}
		if(WindowSong()->GetFocusPattern())
		{
			popmenu->AddFMenu("Focus Pattern",new menu_gotoarrange(this,GOTO_FOCUSPATTERN));
		}
	}

}

void Edit_Arrange::AddEditorMenu(guiMenu *menu)
{
	if(guiMenu *n=menu->AddMenu("Editor",0))
	{		
		CreateEditorMenu(n);
	}

	if(guiMenu *n=menu->AddMenu(Cxs[CXS_OPTIONS],0))
	{		
		class menu_showtextmap:public guiMenu
		{
		public:
			menu_showtextmap(Edit_Arrange *ed){editor=ed;}

			void MenuFunction()
			{
				editor->showtextmap=editor->showtextmap==true?false:true;
				editor->menu_showtextmap->menu->Select(editor->menu_showtextmap->index,editor->showtextmap);
				editor->ShowHoriz(true,false,false);
			} 

			Edit_Arrange *editor;
		};

		n->AddFMenu(Cxs[CXS_SHOWTEXTMAP],this->menu_showtextmap=new menu_showtextmap(this),showtextmap);

		class menu_showmarkermap:public guiMenu
		{
		public:
			menu_showmarkermap(Edit_Arrange *ed){editor=ed;}

			void MenuFunction()
			{
				editor->showmarkermap=editor->showmarkermap==true?false:true;
				editor->menu_showmarkermap->menu->Select(editor->menu_showmarkermap->index,editor->showmarkermap);
				editor->ShowHoriz(true,false,false);
			} 

			Edit_Arrange *editor;
		};
		n->AddFMenu(Cxs[CXS_SHOWMARKERMAP],this->menu_showmarkermap=new menu_showmarkermap(this),showtextmap);
		n->AddLine();

		menu_shownotesmenu=n->AddMenu(Cxs[CXS_SHOWNOTES],0);
		if(menu_shownotesmenu)
		{
			class menu_shownotes:public guiMenu
			{
			public:
				menu_shownotes(Edit_Arrange *ed,int f){editor=ed;flag=f;}

				void MenuFunction()
				{
					editor->shownotesinarrageeditor=mainsettings->shownotesinarrageeditor=flag;
					editor->DrawDBBlit(editor->pattern);
				} 

				Edit_Arrange *editor;
				int flag;
			};

			menu_shownotesmenu->AddFMenu(Cxs[CXS_OFF],this->menu_shownotesmenu_off=new menu_shownotes(this,ARRANGEEDITOR_SHOWNOTES_OFF),shownotesinarrageeditor==ARRANGEEDITOR_SHOWNOTES_OFF?true:false);
			menu_shownotesmenu->AddFMenu(Cxs[CXS_ASLINES],this->menu_shownotesmenu_lines=new menu_shownotes(this,ARRANGEEDITOR_SHOWNOTES_LINES),shownotesinarrageeditor==ARRANGEEDITOR_SHOWNOTES_LINES?true:false);
			menu_shownotesmenu->AddFMenu(Cxs[CXS_ASNOTES],this->menu_shownotesmenu_notes=new menu_shownotes(this,ARRANGEEDITOR_SHOWNOTES_NOTES),shownotesinarrageeditor==ARRANGEEDITOR_SHOWNOTES_NOTES?true:false);
		}

		/*
		class menu_showtracknumer:public guiMenu
		{
		public:
		menu_showtracknumer(Edit_Arrange *ed){editor=ed;}

		void MenuFunction()
		{
		editor->showtracknumber=editor->showtracknumber==true?false:true;
		mainsettings->showarrangetracknumber=editor->showtracknumber;
		} 

		Edit_Arrange *editor;
		};

		n->AddFMenu(Cxs[CXS_SHOWTRACKNUMBER],new menu_showtracknumer(this),showtracknumber);
		*/

		class menu_showcontrol:public guiMenu
		{
		public:
			menu_showcontrol(Edit_Arrange *ed){editor=ed;}

			void MenuFunction()
			{
				mainsettings->showarrangecontrols=editor->showcontrols=editor->showcontrols==true?false:true;
				editor->DrawDBBlit(editor->pattern);

				//editor->ShowAllPattern(false);
				//editor->BltGUIBuffer_Frame(&editor->frame_pattern);
				//editor->menu_showcontrol->menu->Select(editor->menu_showcontrol->index,editor->showcontrols);
			} 

			Edit_Arrange *editor;
		};	
		n->AddFMenu(Cxs[CXS_SHOWMIDICONTROL],this->menu_showcontrol=new menu_showcontrol(this),showcontrols);

		n->AddLine();

		class menu_showallautomationtracks:public guiMenu
		{
		public:
			menu_showallautomationtracks(Edit_Arrange *ed){editor=ed;}

			void MenuFunction()
			{
				editor->ShowHideAutomationTracks(true);
			} 

			Edit_Arrange *editor;
		};

		n->AddFMenu(Cxs[CXS_SHOWALLAUTOTRACKS],new menu_showallautomationtracks(this));

		class menu_hideallautomationtracks:public guiMenu
		{
		public:
			menu_hideallautomationtracks(Edit_Arrange *ed){editor=ed;}

			void MenuFunction()
			{
				editor->ShowHideAutomationTracks(false);
			} 

			Edit_Arrange *editor;
		};

		n->AddFMenu(Cxs[CXS_HIDEALLAUTOTRACKS],new menu_hideallautomationtracks(this));

		/*
		// Folder
		if(n=menu->AddMenu("Folder",0))
		{
		class menu_showfolder:public guiMenu
		{
		public:
		menu_showfolder(Edit_Arrange *ed,bool oo){editor=ed;onoff=oo;}

		void MenuFunction()
		{
		bool refresh=false;
		Seq_Track *t=editor->WindowSong()->FirstTrack();
		while(t)
		{
		if(t->showchilds!=onoff)
		{
		t->showchilds=onoff;
		refresh=true;
		}

		t=t->NextTrack();
		}

		if(refresh==true)
		maingui->RefreshAllEditors(editor->WindowSong(),EDITORTYPE_ARRANGE,0);	
		} 

		Edit_Arrange *editor;
		bool onoff;
		};

		n->AddFMenu(Cxs[CXS_SHOWALLFOLDER],new menu_showfolder(this,true));
		n->AddFMenu(Cxs[CXS_HIDEALLFOLDER],new menu_showfolder(this,false));
		n->AddLine();
		}
		*/

	}
}

guiMenu *Edit_Arrange::CreateMenu()
{
	//	ResetUndoMenu();
	DeletePopUpMenu(true);

	if(guiMenu *menu=popmenu)
	{
		menu->AddMenu(" TRACKS ",0);
		menu->AddLine();

		guiMenu *n=menu->AddMenu(Cxs[CXS_FILE],0);

		if(n)
		{
			class menu_LoadWave:public guiMenu
			{
			public:
				menu_LoadWave(guiWindow *win){window=win;}

				void MenuFunction()
				{
					mainedit->LoadSoundFileNewTracks
						(
						window,
						window->WindowSong()->GetFocusTrack(),
						window->WindowSong()->GetSongPosition()
						);
				} //

				guiWindow *window;
			};

			n->AddFMenu(Cxs[CXS_LOADWAVE],new menu_LoadWave(this));

			class menu_LoadAudioFileAndSplit:public guiMenu
			{
			public:
				menu_LoadAudioFileAndSplit(guiWindow *win){window=win;}

				void MenuFunction()
				{
					mainedit->LoadSoundFileAndSplitNewTracks
						(
						window,
						window->WindowSong()->GetFocusTrack(),
						window->WindowSong()->GetSongPosition()
						);
				} //

				guiWindow *window;
			};
			n->AddFMenu(Cxs[CXS_LOADAUDIOFILEANDSPLITTOCHANNELS],new menu_LoadAudioFileAndSplit(this));

			class menu_LoadDirectory:public guiMenu
			{
			public:
				menu_LoadDirectory(guiWindow *win){window=win;}

				void MenuFunction()
				{
					mainedit->LoadSoundFileDirectoryNewTracks
						(
						window,
						window->WindowSong()->GetFocusTrack(),
						window->WindowSong()->GetSongPosition()
						);
				} //

				guiWindow *window;
			};

			n->AddFMenu(Cxs[CXS_LOADDIRECTORY],new menu_LoadDirectory(this));	
		}

		n=menu->AddMenu(Cxs[CXS_EDIT],0);
		if(n)
		{
			//maingui->AddUndoMenu(n);

			if(guiMenu *mt=n)
			{
				class menu_delseltracks:public guiMenu
				{
				public:
					menu_delseltracks(Edit_Arrange *ar){editor=ar;}

					void MenuFunction()
					{	
						editor->editmode=ED_TRACKS;
						editor->Delete();
					} //

					Edit_Arrange *editor;
				};

				if(WindowSong()->FirstSelectedTrack())
					mt->AddFMenu(Cxs[CXS_DELETE],new menu_delseltracks(this),SK_DEL);

				mt->AddLine();

				mt->AddFMenu(0,new globmenu_cnewtrack(WindowSong(),0));

				class menu_newTracks:public guiMenu
				{
				public:
					void MenuFunction()
					{
						guiWindow *win=maingui->FindWindow(EDITORTYPE_CREATETRACKS,0,0);

						if(win)
							win->WindowToFront(true);
						else
							maingui->OpenEditorStart(EDITORTYPE_CREATETRACKS,0,0,0,0,0,0);
					}
				};
				mt->AddFMenu(Cxs[CXS_CREATEXNEWTRACKS],new menu_newTracks(),"Shift+X");

				mt->AddLine();
				class menu_newBus:public guiMenu
				{
				public:
					void MenuFunction()
					{
						guiWindow *win=maingui->FindWindow(EDITORTYPE_CREATEBUS,0,0);

						if(win)
							win->WindowToFront(true);
						else
							maingui->OpenEditorStart(EDITORTYPE_CREATEBUS,0,0,0,0,0,0);
					}
				};
				mt->AddFMenu(Cxs[CXS_CREATEXNEWBUS],new menu_newBus(),"Shift+B");

				class menu_delBus:public guiMenu
				{
				public:
					menu_delBus(guiWindow *w){win=w;}
					void MenuFunction()
					{
						mainedit->DeleteBusChannels(win->WindowSong());							
					}
					guiWindow *win;
				};

				mt->AddFMenu(Cxs[CXS_DELETEBUS],new menu_delBus(this));

				mt->AddLine();

				class menu_muteall:public guiMenu
				{
				public:
					menu_muteall(Seq_Song *s,bool x,bool sel)
					{
						song=s;
						ex=x;
						selected=sel;
					}

					void MenuFunction()
					{		
						Seq_Track *t=song->FirstTrack();

						while(t)
						{
							if((ex==false || t!=song->GetFocusTrack()) && 
								(selected==false || (t->IsSelected()==true))
								)
								song->MuteTrack(t,true);

							t=t->NextTrack();
						}
					} //

					Seq_Song *song;
					bool ex,selected;
				};

				class menu_muteallexecptsel:public guiMenu
				{
				public:
					menu_muteallexecptsel(Seq_Song *s){song=s;}

					void MenuFunction()
					{		
						Seq_Track *t=song->FirstTrack();

						while(t)
						{
							if(t->IsSelected()==false)
								song->MuteTrack(t,true);

							t=t->NextTrack();
						}
					} //

					Seq_Song *song;
				};

				class menu_muteseltracks:public guiMenu
				{
				public:
					menu_muteseltracks(Edit_Arrange *ed,bool m){editor=ed;mute=m;}

					void MenuFunction()
					{	
						editor->editmode=Edit_Arrange::ED_TRACKS;
						editor->MuteSelected(mute);

					} //

					Edit_Arrange *editor;
					bool mute;
				};

				mt->AddLine();

				mt->AddFMenu(Cxs[CXS_MUTEALLTRACKS],new menu_muteall(WindowSong(),false,false));
				mt->AddFMenu(Cxs[CXS_MUTEALLSELTRACKS],new menu_muteseltracks(this,true),SK_MUTE);
				mt->AddFMenu(Cxs[CXS_MUTEALLTRACKSA],new menu_muteall(WindowSong(),true,false));
				mt->AddFMenu(Cxs[CXS_MUTEALLTRACKSASEL],new menu_muteallexecptsel(WindowSong()),"Ctrl+M");

				mt->AddFMenu(Cxs[CXS_UNMUTEALLTRACKS],new menu_muteoff(WindowSong(),false));
				mt->AddFMenu(Cxs[CXS_UNMUTEALLTRACKSSELT],new menu_muteseltracks(this,false),SK_UNMUTE);
				mt->AddLine();
				mt->AddFMenu(Cxs[CXS_SOLOOFFTRACKS],new menu_solooff(WindowSong()));

				/*
				class freeze:public guiMenu
				{
				public:
				freeze(Edit_Arrange *ar,bool f)
				{
				editor=ar;
				freezeonoff=f;
				}

				void MenuFunction()
				{		
				editor->FreezeTrack(0,freezeonoff);
				} //

				Edit_Arrange *editor;
				bool freezeonoff;
				};
				*/
				//mt->AddFMenu("Freeze Track",new freeze(this,true));
				//mt->AddFMenu("UnFreeze Track",new freeze(this,false));

				mt->AddLine();

				class menu_delempty:public guiMenu
				{
				public:
					menu_delempty(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{		
						mainedit->DeleteAllEmptyTracks(arrangeeditor->WindowSong());	
					} //

					Edit_Arrange *arrangeeditor;
				};
				mt->AddFMenu(Cxs[CXS_DELETEALLEMPTYTRACKS],new menu_delempty(this));

				class menu_sort:public guiMenu
				{
				public:
					menu_sort(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{		
						mainedit->SortTracks(arrangeeditor->WindowSong());	
					}

					Edit_Arrange *arrangeeditor;
				};

				mt->AddFMenu(Cxs[CXS_SORTTRACKSBYTYPE],new menu_sort(this));

				class menu_trackup:public guiMenu
				{
				public:
					menu_trackup(Edit_Arrange *ed){editor=ed;}

					void MenuFunction()
					{
						mainedit->MoveTracks(editor->WindowSong(),-1);
					} 

					Edit_Arrange *editor;
				};

				if(char *cu=mainvar->GenerateString("Ctrl+",Cxs[CXS_CURSORUP]))
				{
					mt->AddFMenu(Cxs[CXS_MOVETRACKUP],new menu_trackup(this),cu);
					delete cu;
				}

				class menu_trackdown:public guiMenu
				{
				public:
					menu_trackdown(Edit_Arrange *ed){editor=ed;}

					void MenuFunction()
					{
						mainedit->MoveTracks(editor->WindowSong(),1);
					} 

					Edit_Arrange *editor;
				};

				if(char *cd=mainvar->GenerateString("Ctrl+",Cxs[CXS_CURSORDOWN]))
				{
					mt->AddFMenu(Cxs[CXS_MOVETRACKDOWN],new menu_trackdown(this),cd);
					delete cd;
				}
			}

			n->AddLine();

			class menu_solomutereset:public guiMenu
			{
			public:
				menu_solomutereset(Seq_Song *s){song=s;}

				void MenuFunction()
				{		
					song->ResetSoloMute();
				} //

				Seq_Song *song;
			};
			n->AddFMenu(Cxs[CXS_RESETSOLOMUTE],new menu_solomutereset(WindowSong()));
		}

		n=menu->AddMenu(Cxs[CXS_SELECT],0);
		if(n)
		{
			if(guiMenu *mt=n)
			{
				class menu_SelTracks:public guiMenu
				{
				public:
					menu_SelTracks(Edit_Arrange *ed,bool s)
					{
						editor=ed;
						sel=s;					
					}

					void MenuFunction()
					{
						editor->editmode=ED_TRACKS;
						editor->SelectAll(sel);
					}

					Edit_Arrange *editor;
					bool sel;
				};

				mt->AddFMenu(Cxs[CXS_SELECTALLTRACKS],new menu_SelTracks(this,true),SK_SELECTALL);
				mt->AddFMenu(Cxs[CXS_UNSELECTALLTRACKS],new menu_SelTracks(this,false),SK_DESELECTALL);
			}
		}

		/// ------------------------------- Functions ------------------------------------
		if(n=menu->AddMenu(Cxs[CXS_FUNCTIONS],0))
		{
			if(guiMenu *mt=n)
			{
				class menu_splitMIDItrack:public guiMenu
				{
				public:
					menu_splitMIDItrack(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{		
						mainedit->SplitTrackToChannels(arrangeeditor->WindowSong()->GetFocusTrack());
					} //

					Edit_Arrange *arrangeeditor;
				};

				mt->AddFMenu(Cxs[CXS_SPLITTRACKCHANNELS],new menu_splitMIDItrack(this));

				mt->AddLine();

				class menu_recordalltracks:public guiMenu
				{
				public:
					menu_recordalltracks(Edit_Arrange *ed,bool s,bool o){editor=ed;selected=s;on=o;}

					void MenuFunction()
					{
						Seq_Track *t=editor->WindowSong()->FirstTrack();

						while(t)
						{
							if(selected==false || t->IsSelected()==true)
								t->record=on;

							t=t->NextTrack();
						}
					} 

					Edit_Arrange *editor;
					bool selected,on;
				};

				if(char *h=mainvar->GenerateString(Cxs[CXS_ALLTRACKS]," ",Cxs[CXS_RECORD],":",Cxs[CXS_ON]))
				{
					mt->AddFMenu(h,new menu_recordalltracks(this,false,true));
					delete h;
				}

				if(char *h=mainvar->GenerateString(Cxs[CXS_ALLTRACKS]," ",Cxs[CXS_RECORD],":",Cxs[CXS_OFF]))
				{
					mt->AddFMenu(h,new menu_recordalltracks(this,false,false));
					delete h;
				}

				if(char *h=mainvar->GenerateString(Cxs[CXS_ALLSELECTEDTRACKS]," ",Cxs[CXS_RECORD],":",Cxs[CXS_ON]))
				{
					mt->AddFMenu(h,new menu_recordalltracks(this,true,true));
					delete h;
				}

				if(char *h=mainvar->GenerateString(Cxs[CXS_ALLSELECTEDTRACKS]," ",Cxs[CXS_RECORD],":",Cxs[CXS_OFF]))
				{
					mt->AddFMenu(h,new menu_recordalltracks(this,true,false));
					delete h;
				}

				mt->AddLine();

				class menu_removechilds:public guiMenu
				{
				public:
					menu_removechilds(Edit_Arrange *ed){editor=ed;}

					void MenuFunction()
					{
						mainedit->RemoveSelectedTrackFromParent(editor->WindowSong());
					} 

					Edit_Arrange *editor;
				};

				mt->AddFMenu(Cxs[CXS_RELEASECHILDTRACKS],new menu_removechilds(this));
			}
		}

		if(WindowSong()->GetFocusTrack() || WindowSong()->GetFocusPattern())
		{
			if(n=menu->AddMenu("Goto",0))
			{
				if(WindowSong()->GetFocusTrack())
				{
					n->AddFMenu("Focus Track",new menu_gotoarrange(this,GOTO_FOCUSTRACK),"T");
				}

				if(WindowSong()->GetFocusPattern())
				{
					n->AddFMenu("Focus Pattern",new menu_gotoarrange(this,GOTO_FOCUSPATTERN),"F");
				}
			}
		}

		AddEditorMenu(menu);
	}

	//maingui->AddCascadeMenu(this,menu);

	return menu;
}


guiMenu *Edit_Arrange::CreateMenu2()
{
	// Pattern

	DeletePopUpMenu(true);

	if(guiMenu *menu=popmenu)
	{
		menu->AddMenu(" PATTERN ",0);
		menu->AddLine();

		guiMenu *n=menu->AddMenu(Cxs[CXS_EDIT],0);
		if(n)
		{
			//maingui->AddUndoMenu(n);

			if(guiMenu *mp=n)
			{
				class menu_Cut:public guiMenu
				{
				public:
					menu_Cut(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{
						arrangeeditor->CutSelectedPattern();
					} //

					Edit_Arrange *arrangeeditor;
				};

				if(WindowSong()->FirstSelectedPattern())
					mp->AddFMenu(Cxs[CXS_CUT],new menu_Cut(this),SK_CUT);

				class menu_Copy:public guiMenu
				{
				public:
					menu_Copy(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{
						arrangeeditor->CopySelectedPattern();
					} //

					Edit_Arrange *arrangeeditor;
				};

				if(WindowSong()->FirstSelectedPattern())
					mp->AddFMenu(Cxs[CXS_COPY],new menu_Copy(this),SK_COPY);

				class menu_Paste:public guiMenu
				{
				public:
					menu_Paste(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{
						arrangeeditor->Paste();
					}

					Edit_Arrange *arrangeeditor;
				};

				if(mainbuffer->CheckBuffer(this,WindowSong(),WindowSong()->GetFocusTrack())==true)
					mp->AddFMenu(Cxs[CXS_PASTE],new menu_Paste(this),SK_PASTE);

				class menu_Delete:public guiMenu
				{
				public:
					menu_Delete(Edit_Arrange *ar){editor=ar;}

					void MenuFunction()
					{
						editor->editmode=ED_PATTERN;
						editor->Delete();
					}

					Edit_Arrange *editor;
				};

				if(WindowSong()->FirstSelectedPattern() || WindowSong()->IsAutomationParameterSelected()==true)
					mp->AddFMenu(Cxs[CXS_DELETE],new menu_Delete(this),SK_DEL);

				mp->AddLine();
				class menu_newPattern:public guiMenu
				{
				public:
					menu_newPattern(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{
						Seq_Track *track=arrangeeditor->WindowSong()->GetFocusTrack();

						if(track)
							mainedit->CreateNewPattern(0,track,MEDIATYPE_MIDI,arrangeeditor->WindowSong()->GetSongPosition(),true);
					} //

					Edit_Arrange *arrangeeditor;
				};

				if(WindowSong()->GetFocusTrack())
					mp->AddFMenu(Cxs[CXS_CREATENEWPATTERN],new menu_newPattern(this));

				mp->AddLine();

				class menu_muteonoff_patternsel:public guiMenu
				{
				public:
					menu_muteonoff_patternsel(Edit_Arrange *ed,bool m)
					{
						editor=ed;
						mute=m;
					}

					void MenuFunction()
					{	
						editor->editmode=Edit_Arrange::ED_PATTERN;
						editor->MuteSelected(mute);
					} //

					Edit_Arrange *editor;
					bool mute;
				};

				mp->AddFMenu(Cxs[CXS_MUTESELPATTERN],new menu_muteonoff_patternsel(this,true),SK_MUTE);
				mp->AddFMenu(Cxs[CXS_UNMUTESELPATTERN],new menu_muteonoff_patternsel(this,false),SK_UNMUTE);

				class menu_muteonoff_patternall:public guiMenu
				{
				public:
					menu_muteonoff_patternall(Edit_Arrange *ed,bool m)
					{
						editor=ed;
						mute=m;
					}

					void MenuFunction()
					{	
						if(Seq_SelectionList *sl=editor->CreateSelectionListAll(true))
						{
							mainedit->MutePattern(editor->WindowSong(),sl, 0,mute);
							sl->DeleteAllPattern();
							delete sl;
						}
					} //

					Edit_Arrange *editor;
					bool mute;
				};

				mp->AddFMenu(Cxs[CXS_MUTEALLPATTERN],new menu_muteonoff_patternall(this,true));
				mp->AddFMenu(Cxs[CXS_UNMUTEALLPATTERN],new menu_muteonoff_patternall(this,false));
			}
		}

		n=menu->AddMenu(Cxs[CXS_SELECT],0);
		if(n)
		{
			if(guiMenu *mp=n)
			{
				class menu_selAll:public guiMenu
				{
				public:
					menu_selAll(Edit_Arrange *ar,bool s){arrangeeditor=ar;sel=s;}

					void MenuFunction()
					{
						arrangeeditor->editmode=Edit_Arrange::ED_PATTERN;
						arrangeeditor->SelectAll(sel);
					} //

					Edit_Arrange *arrangeeditor;
					bool sel;
				};
				mp->AddFMenu(Cxs[CXS_SELECTALLPATTERN],new menu_selAll(this,true),SK_SELECTALL);
				mp->AddFMenu(Cxs[CXS_UNSELECTALLPATTERN],new menu_selAll(this,false),SK_DESELECTALL);

				class menu_selAllOfTrack:public guiMenu
				{
				public:
					menu_selAllOfTrack(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{
						if(Seq_Track *t=arrangeeditor->WindowSong()->GetFocusTrack())
						{
							Seq_Pattern *p=t->FirstPattern(MEDIATYPE_ALL);

							while(p){
								arrangeeditor->SelectPattern(p,true,true);
								p=p->NextPattern(MEDIATYPE_ALL);
							}
						}	
					} //

					Edit_Arrange *arrangeeditor;
				};
				mp->AddFMenu(Cxs[CXS_SELECTALLPATTERNAT],new menu_selAllOfTrack(this));

				class menu_selAllCycle:public guiMenu
				{
				public:
					menu_selAllCycle(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{	
						Seq_Track *t=arrangeeditor->WindowSong()->FirstTrack();

						while(t)
						{
							Seq_Pattern *p=t->FirstPattern(MEDIATYPE_ALL);

							while(p)
							{
								if(p->CheckIfInRange(arrangeeditor->WindowSong()->playbacksettings.cyclestart,
									arrangeeditor->WindowSong()->playbacksettings.cycleend,false)==true)
									arrangeeditor->SelectPattern(p,true,true);

								p=p->NextPattern(MEDIATYPE_ALL);
							}

							t=t->NextTrack();
						}
					} //

					Edit_Arrange *arrangeeditor;
				};

				mp->AddFMenu(Cxs[CXS_SELECTALLPATTERNCP],new menu_selAllCycle(this));

				class menu_selAllCycleTrack:public guiMenu
				{
				public:
					menu_selAllCycleTrack(Edit_Arrange *ar){arrangeeditor=ar;}

					void MenuFunction()
					{		
						if(Seq_Track *t=arrangeeditor->WindowSong()->GetFocusTrack())
						{
							Seq_Pattern *p=t->FirstPattern(MEDIATYPE_ALL);

							while(p)
							{
								if( p->CheckIfInRange(arrangeeditor->WindowSong()->playbacksettings.cyclestart,arrangeeditor->WindowSong()->playbacksettings.cycleend,false)==true)
									arrangeeditor->SelectPattern(p,true,true);

								p=p->NextPattern(MEDIATYPE_ALL);
							}
						}
					} //

					Edit_Arrange *arrangeeditor;
				};

				mp->AddFMenu(Cxs[CXS_SELECTALLPATTERNTCP],new menu_selAllCycleTrack(this));
			}
		}

		/// ------------------------------- Functions ------------------------------------
		n=menu->AddMenu(Cxs[CXS_FUNCTIONS],0);
		if(n)
		{
			if(guiMenu *mp=n)
			{
				class menu_cmix:public guiMenu
				{
				public:
					menu_cmix(Edit_Arrange *ed){editor=ed;}

					void MenuFunction()
					{
						if(Seq_SelectionList *sl=editor->CreateSelectionList())
							mainedit->CreateMixBuffer(sl);
					} 

					Edit_Arrange *editor;
				};

				class menu_movemix:public guiMenu
				{
				public:
					menu_movemix(Edit_Arrange *ed){editor=ed;}

					void MenuFunction()
					{
						editor->MoveSelectedPatternToMeasure(false);
					} 

					Edit_Arrange *editor;
				};

				mp->AddLine();
				mp->AddFMenu(Cxs[CXS_MOVESELPATTERNM],new menu_movemix(this));

				class menu_movespmix:public guiMenu
				{
				public:
					menu_movespmix(Edit_Arrange *ed){editor=ed;}

					void MenuFunction()
					{
						editor->MoveSelectedPatternToMeasureToTick(editor->WindowSong()->GetSongPosition(),false);
					} 

					Edit_Arrange *editor;
				};

				mp->AddFMenu(Cxs[CXS_MOVESELPATTERNSP],new menu_movespmix(this));

				mp->AddLine();
				class menu_movepattern:public guiMenu
				{
				public:
					menu_movepattern(Edit_Arrange *ed,bool r,bool selp,bool selt){editor=ed;right=r;selectedpattern=selp;selectedtracks=selt;}

					void MenuFunction()
					{
						editor->MovePatternCycle(right,selectedpattern,selectedtracks);
					} 

					Edit_Arrange *editor;
					bool right,selectedpattern,selectedtracks;
				};

				guiMenu *l=mp->AddMenu(Cxs[CXS_MOVEPATTERNCYCLERANGE_LEFT],0),*r=mp->AddMenu(Cxs[CXS_MOVEPATTERNCYCLERANGE_RIGHT],0);

				if(l)
				{
					l->AddFMenu(Cxs[CXS_ALLPATTERN],new menu_movepattern(this,false,false,false));
					l->AddFMenu(Cxs[CXS_SELECTEDPATTERN],new menu_movepattern(this,false,true,false));
					l->AddFMenu(Cxs[CXS_ALLPATTERNSELECTEDTRACKS],new menu_movepattern(this,false,false,true));	
				}

				if(r)
				{
					r->AddFMenu(Cxs[CXS_ALLPATTERN],new menu_movepattern(this,true,false,false));
					r->AddFMenu(Cxs[CXS_SELECTEDPATTERN],new menu_movepattern(this,true,true,false));
					r->AddFMenu(Cxs[CXS_ALLPATTERNSELECTEDTRACKS],new menu_movepattern(this,true,false,true));	
				}

				mp->AddLine();
				class menu_copymix:public guiMenu
				{
				public:
					menu_copymix(Edit_Arrange *ed){editor=ed;}

					void MenuFunction()
					{
						editor->MoveSelectedPatternToMeasure(true);
					} 

					Edit_Arrange *editor;
				};

				mp->AddFMenu(Cxs[CXS_COPYSELPATTERNM],new menu_copymix(this));

				class menu_copyspmix:public guiMenu
				{
				public:
					menu_copyspmix(Edit_Arrange *ed){editor=ed;}

					void MenuFunction()
					{
						editor->MoveSelectedPatternToMeasureToTick(editor->WindowSong()->GetSongPosition(),true);
					} 

					Edit_Arrange *editor;
				};

				mp->AddFMenu(Cxs[CXS_COPYSELPATTERNSP],new menu_copyspmix(this));
				mp->AddLine();
			}
		}

		if(WindowSong()->GetFocusTrack() || WindowSong()->GetFocusPattern())
		{
			if(n=menu->AddMenu("Goto",0))
			{
				if(WindowSong()->GetFocusTrack())
				{
					n->AddFMenu("Focus Track",new menu_gotoarrange(this,GOTO_FOCUSTRACK),"T");
				}

				if(WindowSong()->GetFocusPattern())
				{
					n->AddFMenu("Focus Pattern",new menu_gotoarrange(this,GOTO_FOCUSPATTERN),"F");
				}
			}
		}

		if(n=menu->AddMenu("Audio",0))
		{
			class menu_ExportSelectedPattern:public guiMenu
			{
			public:
				menu_ExportSelectedPattern(Edit_Arrange *ed){editor=ed;}

				void MenuFunction()
				{
					editor->ExportSelectedPattern(false);
				}

				Edit_Arrange *editor;
			};

			n->AddFMenu(Cxs[CXS_EXPORTSELECTEDPATTERN],new menu_ExportSelectedPattern(this));

			class menu_ExportSplitSelectedPattern:public guiMenu
			{
			public:
				menu_ExportSplitSelectedPattern(Edit_Arrange *ed){editor=ed;}

				void MenuFunction()
				{
					editor->ExportSelectedPattern(true);
				}

				Edit_Arrange *editor;
			};

			n->AddFMenu(Cxs[CXS_EXPORTSELECTEDPATTERNSPLIT],new menu_ExportSplitSelectedPattern(this));

		}

		AddEditorMenu(menu);
	}

	//maingui->AddCascadeMenu(this,menu);

	return menu;
}

void Edit_Arrange::ShowMenu()
{
	ShowEditorMenu();
}

void Edit_Arrange::DeInitWindow()
{	
	FreeEditorMemory();

	CloseHeader();
	//ResetAllGadgets();
	//	trackfx.FreeMemory();

	selection.DeleteAllPattern();

	if(focustrackwindowname)delete focustrackwindowname;
	if(focustrackwindowname_string)delete focustrackwindowname_string;
	if(activepatternwindowname_string)delete activepatternwindowname_string;
}

void Edit_Arrange::RefreshAutomationObject(AutomationObject *ao)
{
	// Pattern
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		switch(o->id)
		{
		case OBJECTID_ARRANGEAUTOMATIONTRACK:
			{
				Edit_Arrange_AutomationTrack *automationtrack=(Edit_Arrange_AutomationTrack *)o;
				Edit_Arrange_AutomationTrackParameter *so=automationtrack->FirstTrackParameter();

				while(so)
				{
					if(so->object==ao)
					{
						//so->Draw(true);
						break;
					}

					so=so->NextAutomationTrackObject();
				}
			}
			break;
		}

		o=o->NextObject();
	}
}


void Edit_Arrange::RefreshAudioPeak()
{
	// Check Peak File Progress
	if(audiopeakthread->GetRunningFile()) // Create Peak File ?
	{
		guiObject *o=guiobjects.FirstObject();

		while(o)
		{
			switch(o->id)
			{
			case OBJECTID_ARRANGEPATTERN:
				{
					Edit_Arrange_Pattern *p=(Edit_Arrange_Pattern *)o;

					switch(p->pattern->mediatype)
					{
					case MEDIATYPE_AUDIO:
						{
							AudioPattern *ap=(AudioPattern *)p->pattern;

							// Peak Creation ?
							if(ap->audioevent.audioefile && ap->audioevent.audioefile==audiopeakthread->GetRunningFile())
							{
								double b=p->showpeakprogress;

								p->showpeakprogress=audiopeakthread->createprogress;

								if(p->showpeakprogress>0 && p->showpeakprogress!=b)
									p->ShowPattern_Blt();

								p->showpeakprogress=0;
							}

						}
						break;
					}
				}
				break;
			}// switch

			o=o->NextObject();
		}// while o
	}

#ifdef OLDIE
	// Check Freeze Track
	if(audiofreezethread->createfrozentrack) // Create Peak File ?
	{
		guiObject *o=guiobjects.FirstObject();

		while(o)
		{
			switch(o->id)
			{
			case OBJECTID_ARRANGETRACK:
				{
					Edit_Arrange_Track *track=(Edit_Arrange_Track *)o;

					if(track->track->underfreeze==true)
					{
						if(track->track==audiofreezethread->createfrozentrack)
						{
							track->showfreeze_flag=SHOWFREEZE_CREATING;
							track->freezeprogress=audiofreezethread->createfreezeprogress;
							track->ShowName();

							track->showfreeze_flag=0;
							track->freezeprogress=0;
						}
						else
						{
							track->showfreeze_flag=SHOWFREEZE_WAITING;
							track->freezeprogress=0;
							track->ShowName();

							track->showfreeze_flag=0;
							track->freezeprogress=0;
						}

					}
				}
				break;
			} // switch

			o=o->NextObject();
		}// while o

	}// if freeze
#endif

}


void Edit_Arrange::FreezeTrack(Seq_Track *track,bool freeze)
{
	/*
	if(WindowSong())
	{
	if(!track)
	track=WindowSong()->GetFocusTrack();

	if(freeze==true)
	{
	if((WindowSong()->status&(Seq_Song::STATUS_RECORD|Seq_Song::STATUS_WAITPREMETRO|Seq_Song::STATUS_STEPRECORD))==0)
	track->FreezeTrack();
	}
	else
	track->UnFreezeTrack();
	}
	*/
}

void Edit_Arrange::ShowVSlider()
{
	// Show Slider
	if(vertgadget)
		vertgadget->ChangeSlider(&trackobjects,zoomy);
	//vertgadget->ChangeSliderPage(numberoftracks);
}


void ArrangeEditor_Tracks_Callback(guiGadget_CW *g,int status)
{
	Edit_Arrange *ar=(Edit_Arrange *)g->from;

	switch(status)
	{
	case DB_CREATE:
		g->menuindex=0;
		ar->tracks=(guiGadget_Tab *)g;
		break;

	case DB_PAINT:
		{
			ar->ShowTracks();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		ar->MouseMoveInTracks();
		break;

	case DB_LEFTMOUSEDOWN:
		ar->MouseClickInTracks(true);	
		break;

	case DB_LEFTMOUSEUP:
		ar->MouseReleaseInTracks(true);	
		break;

	case DB_RIGHTMOUSEDOWN:
		ar->MouseClickInTracks(false);	
		break;

	case DB_DOUBLECLICKLEFT:
		ar->MouseDoubleClickInTracks(true);
		break;

		//	case DB_DELTA:
		//		ar->DeltaInTracks();
		//		break;
	}
}

void Edit_Arrange::InitMouseEditRange()
{
	if(!pattern)
		return;

	int mx=pattern->GetMouseX(),my=pattern->GetMouseY();
	int sy,sy2;

	if(modestartautomationtrack)
	{
		Edit_Arrange_AutomationTrack *eat=FindAutomationTrack(modestartautomationtrack);

		if(!eat)
		{
			mouseeditx=-1;
			return;
		}

		sy=eat->y;
		sy2=eat->y2;
	}
	else
	{
		mouseselectionstarttrack=modestarttrack;

		Seq_Track *tum=FindTrackAtY(my);

		if(tum)
			mouseselectionendtrack=tum;
		else
		{
			if(mouseselectionstarttrack)
			{
				Edit_Arrange_Track *ft=FindTrack(FirstTrack());
				mouseselectionendtrack=ft && ft->y>my?FirstTrack():LastTrack();
			}
		}

		if(!mouseselectionstarttrack)
		{
			Edit_Arrange_Track *ft=FindTrack(FirstTrack());
			mouseselectionendtrack=ft && ft->y>my?FirstTrack():LastTrack();
		}

		if(!mouseselectionendtrack)
		{
			mouseeditx=-1;
			return;
		}

		if(mouseselectionstarttrack && mouseselectionstarttrack!=mouseselectionendtrack)
		{
			if(mouseselectionstarttrack->GetTrackIndex()>mouseselectionendtrack->GetTrackIndex()){
				Seq_Track *h=mouseselectionendtrack;
				mouseselectionendtrack=mouseselectionstarttrack;
				mouseselectionstarttrack=h;
			}
		}

		Edit_Arrange_Track *st=FindTrack(mouseselectionstarttrack),*se=FindTrack(mouseselectionendtrack);

		sy=st?st->y:0;
		sy2=se?se->y2:pattern->GetY2();
	}

	msposition=modestartposition;

	msendposition=mx<pattern->GetWidth()?timeline->ConvertXPosToTime(mx):endposition;

	if(msposition>msendposition){
		OSTART h=msendposition;
		msendposition=msposition;
		msposition=h;
	}

	int sx=timeline->ConvertTimeToX(msposition),sx2=timeline->ConvertTimeToX(msendposition);

	mouseeditx=sx==-1?0:sx;
	mouseeditx2=sx2==-1?pattern->GetX2():sx2;
	mouseedity=sy;
	mouseedity2=sy2;
}

void Edit_Arrange::MouseReleaseInPattern(bool leftmouse)
{
	if(leftmouse==true)
		switch(mousemode)
	{
		case EM_EDIT_FADEIN:
		case EM_EDIT_FADEOUT:
		case EM_EDIT_VOLUME:
			fadeeditpattern->volumecurve.editmode=false;
			break;

		case EM_SELECTOBJECTS:
			{	
				if(modestartautomationtrack)
				{
					if(modestartautomationtrack->bindtoautomationobject)
					{
						bool changes=true;

						AutomationParameter *ap=modestartautomationtrack->FirstAutomationParameter();

						while(ap)
						{
							if(ap->flag&OFLAG_UNDERSELECTION)
							{
								ap->flag CLEARBIT OFLAG_UNDERSELECTION;
								ap->Select();

								changes=true;
							}

							ap=ap->NextAutomationParameter();
						}

						if(changes==true)
							modestartautomationtrack->parameter.accesscounter++;
					}

					modestartautomationtrack=0;
				}
				else
					if(mouseselectionstarttrack && mouseselectionendtrack)
					{
						bool changes=false;

						Seq_Track *t=mouseselectionstarttrack;

						while(t && t->GetTrackIndex()<=mouseselectionendtrack->GetTrackIndex())
						{
							if(t->IsEditAble()==true)
							{
								Seq_Pattern *p=t->FirstPattern();

								while(p)
								{
									if(p->mediatype!=MEDIATYPE_AUDIO_RECORD && p->recordpattern==false)
									{
										if(p->flag&OFLAG_UNDERSELECTION)
										{
											p->flag CLEARBIT OFLAG_UNDERSELECTION;
											p->Select();

											changes=true;
										}
									}

									p=p->NextPattern();
								}
							}

							t=t->NextTrack();
						}

						if(changes==true)
							maingui->RefreshAllEditorsWithPattern(WindowSong(),0);

						mouseselectionstarttrack=mouseselectionendtrack=0;
					}
			}
			break;

		case EM_MOVEPATTERN:
			{	
				ResetMouseMode();

				MoveO mo;

				mo.song=WindowSong();
				mo.sellist=&selection;
				mo.diff=selection.movediff;
				mo.index=selection.moveobjects_vert;
				mo.flag=GetMouseQuantizeFlag();

				if(selection.CheckMove(&mo)==true)
				{	
					if(maingui->GetCtrlKey()==true)
					{
						// Clone
						if(maingui->GetShiftKey()==true)
							mainedit->ClonePatternList(&mo);
						// Copy
						else
							mainedit->CopyPatternList(&mo);
					}
					else
						mainedit->MovePatternList(&mo);
				}
				else
					DrawDBBlit(pattern);

				return;
			}
			break;

		case EM_MOVEAUTOMATION:
			{
				if(automationtrackeditinit)
				{
					automationtrackeditinit->CheckAndAddToUndo();
					automationtrackeditinit=0;
				}
			}
			break;
	}

	ResetMouseMode();
}

void Edit_Arrange::CheckAutomationParameterUnderMouse()
{
	if(!pattern)
		return;

	int px=pattern->GetMouseX();
	int py=pattern->GetMouseY();

	bool found=false;

	Edit_Arrange_AutomationTrack *eat=FindAutomationTrackAtY(py);

	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_ARRANGEAUTOMATIONTRACK)
		{	
			Edit_Arrange_AutomationTrack *ceat=((Edit_Arrange_AutomationTrack *)o);

			if(ceat!=eat)
			{
				Edit_Arrange_AutomationTrackParameter *atp=ceat->FirstTrackParameter();

				while(atp)
				{
					atp->parameter->flag CLEARBIT OFLAG_MOUSEOVER;
					atp=atp->NextAutomationTrackObject();
				}
			}
		}

		o=o->NextObject();
	}

	if(eat)
	{
		Edit_Arrange_AutomationTrackParameter *atp=eat->FirstTrackParameter();

		while(atp)
		{
			if(found==false && atp->IsUnderMouse(px,py)==true)
			{
				atp->parameter->flag |=OFLAG_MOUSEOVER;
				found=true;
			}
			else
			{
				atp->parameter->flag CLEARBIT OFLAG_MOUSEOVER;
			}

			atp=atp->NextAutomationTrackObject();
		}
	}
}

void Edit_Arrange::MouseMoveInPattern(bool leftmouse)
{
	if(CheckMouseMovePosition(pattern)==true)
		return;

	switch(mousemode)
	{
	case EM_EDIT_FADEIN:
	case EM_EDIT_FADEOUT:
	case EM_EDIT_VOLUME:
		EditFades();
		break;

	case EM_SELECTOBJECTS:
		{
			ShowCycleAndPositions(editarea);
			DrawDBSpriteBlit(editarea);

			if(modestartautomationtrack)
			{
				if(modestartautomationtrack->bindtoautomationobject)
				{
					AutomationParameter *ap=modestartautomationtrack->FirstAutomationParameter();

					while(ap)
					{
						if((ap->flag&OFLAG_UNDERSELECTION) && ap->CheckIfInRange(msposition,msendposition)==false)
						{
							ap->flag CLEARBIT OFLAG_UNDERSELECTION;
						}
						else
							if((!(ap->flag&OFLAG_UNDERSELECTION)) && ap->CheckIfInRange(msposition,msendposition)==true)
							{
								ap->flag|=OFLAG_UNDERSELECTION;
							}

							ap=ap->NextAutomationParameter();
					}
				}
			}
			else
				if(mouseselectionstarttrack && mouseselectionendtrack)
				{
					maingui->OpenPRepairSelection_Pattern(WindowSong());

					Seq_Track *t=WindowSong()->FirstTrack();
					while(t)
					{
						if(t->IsOpen()==true && t->IsEditAble()==true)
						{
							Seq_Pattern *p=t->FirstPattern();
							while(p)
							{
								if( p->CheckIfInRange(msposition,msendposition,false)==true &&
									t->GetTrackIndex()>=mouseselectionstarttrack->GetTrackIndex() &&
									t->GetTrackIndex()<=mouseselectionendtrack->GetTrackIndex())
									p->PRepairSelection();

								p=p->NextRealPattern();
							}
						}

						t=t->NextTrack();
					}

					maingui->ClosePRepairSelection_Pattern(WindowSong());
				}
		}break;

	case EM_MOVEPATTERN:
		{
			ShowMovePatternSprites();
		}
		break;

	case EM_MOVEAUTOMATION:
		{
			MoveAutomationParameters();
		}
		break;

	case EM_DELETE:
		if(leftmouse==true)
		{
		}
		break;

	default:
		{
			// Fades/X Fades
			guiObject *o=guiobjects.FirstObject();
			while(o)
			{
				switch(o->id)
				{
				case OBJECTID_ARRANGEPATTERN:
					{
						Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)o;
						eap->CheckMouseXY(pattern->GetMouseX(),pattern->GetMouseY());
					}
					break;
				}

				o=o->NextObject();
			}
		}
		break;
	}
}

void Edit_Arrange::ShowPatternAndAutomation()
{
	if(!pattern)return;

	guiBitmap *bitmap=&pattern->gbitmap;

	bitmap->guiFillRect(COLOUR_UNUSED);

	// Init  Tracks+Pattern
	{
		guiobjects.RemoveOs(OBJECTID_ARRANGEPATTERN); // remove all Pattern

		guiObject_Pref *o=tracks->FirstGUIObjectPref();
		while(o)
		{
			switch(o->gobject->id)
			{	
			case OBJECTID_ARRANGEAUTOMATIONTRACK:
				{
					Edit_Arrange_AutomationTrack *eat=(Edit_Arrange_AutomationTrack *)o->gobject;

					eat->DrawPattern(true,false,false);
				}
				break;

			case OBJECTID_ARRANGETRACK:
				{
					Edit_Arrange_Track *et=(Edit_Arrange_Track *)o->gobject;

					if(et->track->ismetrotrack==false)
					{
						et->ShowPatternBGRaster();

						if(et->y2>=et->y+8)
						{
							InitPattern(et,et->track,false);

							// Child Track/Pattern
							Seq_Track *ct=et->track->FirstChildTrack();

							while(ct && ct->IsTrackChildOfTrack(et->track)==true)
							{
								InitPattern(et,ct,true);
								ct=ct->NextTrack();
							}
						}
					}
				}
				break;
			}

			o=o->NextGUIObjectPref();
		}
	}

	timeline->DrawPositionRaster(&pattern->gbitmap);

	ShowMarker(0,bitmap->GetY2());

	// 1. Pattern
	ShowAllPattern(); // MIDI Audio

	// 2. Automation
	ShowAutomationTracks();

	// 3. Cycle Sprites
	ShowCycleAndPositions(pattern);
}

void ShowPattern_Callback(guiGadget_CW *g,int status)
{
	Edit_Arrange *ar=(Edit_Arrange *)g->from;

	switch(status)
	{
	case DB_CREATE:
		g->menuindex=1;
		ar->pattern=g;
		break;

	case DB_PAINT:
		{	
			ar->ShowPatternAndAutomation();
		}
		break;

	case DB_PAINTSPRITE:
		ar->ShowCycleAndPositions(ar->pattern);
		break;

		//	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		ar->MouseClickInPattern(true);	
		break;

	case DB_LEFTMOUSEUP:
		ar->MouseReleaseInPattern(true);
		break;

	case DB_RIGHTMOUSEDOWN:
		ar->MouseClickInPattern(false);
		break;

	case DB_DOUBLECLICKLEFT:
		ar->MouseDoubleClickInPattern(true);
		break;

	case DB_MOUSEMOVE:
	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		ar->MouseMoveInPattern(true);	
		break;
	}
}

void ArrangeEditor_Overview_Callback(guiGadget_CW *g,int status)
{
	Edit_Arrange *ar=(Edit_Arrange *)g->from;

	switch(status)
	{
	case DB_CREATE:
		ar->overview=g;
		break;

	case DB_PAINT:
		{
			ar->ShowOverview();
			ar->ShowOverviewCycleAndPositions();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		if(ar->overview->leftmousedown==true)
			ar->MouseClickInOverview(true);	
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		if(ar->overview->rightmousedown==true)
			ar->MouseClickInOverview(false);	
		break;
	}
}

void ShowTempo_Callback(guiGadget_CW *g,int status)
{
	Edit_Arrange *ar=(Edit_Arrange *)g->from;

	switch(status)
	{
	case DB_CREATE:
		ar->tempo=g;
		break;

	case DB_PAINT:
		{
			ar->ShowTempoMap();
			ar->ShowCycleAndPositions(g);
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:

		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:

		break;
	}
}

void ShowSignature_Callback(guiGadget_CW *g,int status)
{
	Edit_Arrange *ar=(Edit_Arrange *)g->from;

	switch(status)
	{
	case DB_CREATE:
		ar->signature=g;
		break;

	case DB_PAINT:
		{
			ar->ShowSignatureMap();
			ar->ShowCycleAndPositions(g);
		}
		break;

	case DB_LEFTMOUSEDOWN:
		ar->MouseClickInSignature(true);
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:

		break;
	}
}

void Edit_Arrange::Init()
{	
	glist.SelectForm(0,0);
	guitoolbox.CreateToolBox(TOOLBOXTYPE_ARRANGE);

	glist.Return();

	int addw=INFOSIZE;

	glist.AddButton(-1,-1,addw,-1,"TList",GADGETID_LIST,showlist==true?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED|MODE_AUTOTOGGLE:MODE_GROUP|MODE_TOGGLE|MODE_AUTOTOGGLE,"Track List+Groups");
	glist.AddLX();
	FormEnable(0,1,showlist);

	glist.AddButton(-1,-1,addw,-1,"TInfo",GADGETID_EFFECTS,showeffects==true?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED|MODE_AUTOTOGGLE:MODE_GROUP|MODE_TOGGLE|MODE_AUTOTOGGLE,"Track/Pattern Effects+Mixer");
	glist.AddLX();
	FormEnable(1,1,showeffects);

	g_tmaster=glist.AddButton(-1,-1,addw,-1,"tMaster",GADGETID_SHOWMASTER,showmaster==true?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Master Channel");
	glist.AddLX();

	g_tbus=glist.AddButton(-1,-1,addw,-1,"tBus",GADGETID_SHOWBUS,showbus==true?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Bus");
	glist.AddLX();

	g_tmetro=glist.AddButton(-1,-1,addw,-1,"tMetro",GADGETID_SHOWMETRO,showmetro==true?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"Metro");
	glist.AddLX();

	g_automation=glist.AddButton(-1,-1,addw,-1,"Automation",GADGETID_AUTOMATION,GetShowFlag()&SHOW_AUTOMATION?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,Cxs[CXS_INFOAUTOMATION]);
	glist.AddLX();

	g_volumecurves=glist.AddButton(-1,-1,addw,-1,"Volume",GADGETID_VOLUMECURVE,GetShowFlag()&SHOW_VOLUMECURVES?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,Cxs[CXS_INFOVOLUME]);
	glist.AddLX();

	g_movebutton=glist.AddButton(-1,-1,addw,-1,"<>",GADGETID_VOLUMECURVE,MODE_MENU,Cxs[CXS_MOVEBUTTON]);
	glist.AddLX();

	eventeditor=glist.AddButton(-1,-1,2*addw,-1,"Event [F1]",GADGETID_EVENTEDITOR,MODE_TEXTCENTER,0);
	glist.AddLX();
	pianoeditor=glist.AddButton(-1,-1,2*addw,-1,"Piano [F2]",GADGETID_PIANOEDITOR,MODE_TEXTCENTER,0);
	glist.AddLX();
	drumeditor=glist.AddButton(-1,-1,2*addw,-1,"Drum [F4]",GADGETID_DRUMEDITOR,MODE_TEXTCENTER,0);
	glist.AddLX();
	sampleeditor=glist.AddButton(-1,-1,2*addw,-1,"Sample [F6]",GADGETID_SAMPLEEDITOR,MODE_TEXTCENTER,0);

	glist.Return();

	g_info_name=glist.AddButton(-1,-1,4*addw,-1,GADGETID_TNAME,MODE_INFO);
	glist.AddLX();

	int iw=bitmap.GetTextWidth("WWWw");

	glist.AddButton(-1,-1,iw,-1,"S",GADGETID_FOCUSPATTERN_TIME_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	g_info_pos=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,-1,GADGETID_TPOS,WINDOWDISPLAY_MEASURE,MODE_INFO,"Position");
	glist.AddLX();

	glist.AddButton(-1,-1,iw,-1,"E",GADGETID_FOCUSPATTERN_END_I,MODE_TEXTCENTER|MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	g_info_end=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,-1,GADGETID_TEND,WINDOWDISPLAY_MEASURE,MODE_INFO);

	if(g_info_end)
		g_info_end->BuildPair(g_info_pos);

	ShowFocusPattern();

	// Editors
	glist.Return();

	int offsettracksy=SIZEV_OVERVIEW+SIZEV_TEMPO+SIZEV_SIGNATURE+SIZEV_HEADER+4*(ADDYSPACE);

	SliderCo horz,vert;

	horz.formx=0;
	horz.formy=2;

	vert.formx=4;
	vert.formy=1;
	vert.offsety=offsettracksy;
	vert.from=0;
	vert.to=0; // trackobjects.GetCount()-numberoftracks;
	vert.pos=0; //firstshowtracknr;

	AddEditorSlider(&horz,&vert,true);

	// List
	glist.SelectForm(0,1);
	glist.form->BindWindow(listeditor=new Edit_ArrangeList(this));

	// Fx
	glist.SelectForm(1,1);

	if(Edit_ArrangeFX *nfx=new Edit_ArrangeFX(this))
	{
		nfx->arrangeeditor=this;
		glist.form->BindWindow(nfx);
	}

	// Track
	glist.SelectForm(2,1);
	g_mute=glist.AddButton(-1,-1,-1,-1,0,GADGETID_MUTE,MODE_LEFTTOMID|MODE_TEXTCENTER|MODE_BOLD);

	glist.AddLX();
	g_solo=glist.AddButton(-1,-1,-1,-1,0,GADGETID_SOLO,MODE_MIDTORIGHT|MODE_TEXTCENTER|MODE_BOLD);

	ShowSoloMute(true);

	glist.Return();

	g_createnewtrack=glist.AddButton(-1,-1,-1,-1,"+ Track +",GADGETID_NEWTRACK,MODE_LEFTTOMID|MODE_BOLD|MODE_TEXTCENTER,Cxs[CXS_CREATENEWTRACK]);
	glist.AddLX();
	g_createnewchildtrack=glist.AddButton(-1,-1,-1,-1,"/ +Child+ ",GADGETID_NEWCHILD,MODE_MIDTORIGHT|MODE_TEXTCENTER,Cxs[CXS_CREATENEWCHILDTRACK]);
	glist.Return();

	g_filtertracks=glist.AddButton(-1,-1,-1,-1,0,GADGETID_FILTERTRACKS,MODE_RIGHT|MODE_MENU);
	ShowTracking();
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,"ZoomR",GADGETID_RESETTRACKZOOM,MODE_RIGHT|MODE_TEXTCENTER,"Reset Track Zoom");
	glist.Return();

	{
		int addw=bitmap.GetTextWidth("WWWW");

		g_audio=glist.AddButton(-1,-1,addw,-1,"Audio",GADGETID_AUDIO,GetShowFlag()&SHOW_AUDIO?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
		glist.AddLX();
		g_MIDI=glist.AddButton(-1,-1,addw,-1,"MIDI",GADGETID_MIDI,GetShowFlag()&SHOW_MIDI?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
		glist.AddLX();

		g_type=glist.AddButton(-1,-1,addw,-1,Cxs[CXS_TYPE],GADGETID_TYPE,GetShowFlag()&SHOW_EVENTTYPE?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,Cxs[CXS_TYPE]);
		glist.AddLX();

		g_input=glist.AddButton(-1,-1,addw,-1,"In",GADGETID_INPUT,GetShowFlag()&SHOW_IO?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE,"MIDI+Audio Input");
		glist.Return();

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

		//ShowFilter();
	}

	glist.AddTab(-1,offsettracksy,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,&ArrangeEditor_Tracks_Callback,this);


	// Pattern etc..
	glist.NextHForm();

	glist.AddChildWindow(-1,-1,-1,SIZEV_OVERVIEW,MODE_RIGHT|MODE_SPRITE,0,&ArrangeEditor_Overview_Callback,this);
	glist.Return();

	glist.AddChildWindow(-1,-1,-1,SIZEV_HEADER,MODE_RIGHT|MODE_SPRITE,0,&Editor_Header_Callback,this);
	glist.Return();

	editarea2=glist.AddChildWindow(-1,-1,-1,SIZEV_TEMPO,MODE_RIGHT|MODE_SPRITE,0,&ShowTempo_Callback,this);
	glist.Return();

	// Signature
	editarea3=glist.AddChildWindow(-1,-1,-1,SIZEV_SIGNATURE,MODE_RIGHT|MODE_SPRITE,0,&ShowSignature_Callback,this);
	glist.Return();

	editarea=glist.AddChildWindow(-1,offsettracksy,-1,-1,MODE_BOTTOM|MODE_RIGHT|MODE_SPRITE,0,&ShowPattern_Callback,this);
}

void Edit_Arrange::RefreshPattern(Seq_Pattern *p)
{
	guiObject *o=guiobjects.FirstObject();

	while(o)
	{
		if(o->id==OBJECTID_ARRANGEPATTERN)
		{
			Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)o;

			if(eap->pattern==p)
			{
				eap->ShowPattern_Blt();
			}
		}

		o=o->NextObject();
	}
}

void Edit_Arrange::RefreshObjects(LONGLONG type,bool editcall)
{
	DrawHeader(); // Tempo Changes etc..
	DrawDBBlit(tracks,pattern,overview);
	DrawDBBlit(tempo,signature);
}

void Edit_Arrange::RefreshAudio(AudioHDFile *hdfile)
{
	guiObject *to=guiobjects.FirstObject();

	while(to)
	{
		if(to->id==OBJECTID_ARRANGEPATTERN)
		{
			Edit_Arrange_Pattern *eap=(Edit_Arrange_Pattern *)to;

			if(eap->pattern->mediatype==MEDIATYPE_AUDIO)
			{
				AudioPattern *af=(AudioPattern *)eap->pattern;

				if(af->audioevent.audioefile==hdfile)
				{
					if(pattern)
						pattern->DrawGadgetBlt();

					return;
				}
			}
		}

		to=to->NextObject();
	}
}

void Edit_Arrange::RefreshAudioRegion(AudioRegion *r)
{
	guiObject *o=guiobjects.FirstObject();

	while(o){

		switch(o->id)
		{
		case OBJECTID_ARRANGEPATTERN:
			{
				Edit_Arrange_Pattern *p=(Edit_Arrange_Pattern *)o;

				if(p->CheckIfRegionIsInside(r)==true)
					p->ShowPattern_Blt();
			}
			break;
		}

		o=o->NextObject();
	}
}

void Edit_Arrange::ShowSoloMute(bool force)
{
	// Refresh Solo/Mute
	int mutecount=0,solocount=0;

	Seq_Track *t=WindowSong()->FirstTrack();
	while(t && (mutecount==0 || solocount==0))
	{
		if(t->GetMute()==true)
			mutecount++;

		if(t->GetSolo()==true)
			solocount++;

		t=t->NextTrack();
	}

	if(force==true || track_muted!=mutecount || track_soloed!=solocount)
	{
		track_muted=mutecount;
		track_soloed=solocount;

		if(g_mute)
		{
			if(track_muted)
			{
				char nr[NUMBERSTRINGLEN];
				char *h=mainvar->GenerateString("Mute (",mainvar->ConvertIntToChar(track_muted,nr),")");

				if(h)
				{
					g_mute->ChangeButtonText(h);
					delete h;
				}
			}
			else
				g_mute->ChangeButtonText("Mute");

			g_mute->SetColour(track_muted>0?COLOUR_RED:-1,track_muted>0?COLOUR_BLACK:-1);
		}

		if(g_solo)
		{
			if(track_soloed)
			{
				char nr[NUMBERSTRINGLEN];
				char *h=mainvar->GenerateString("Solo (",mainvar->ConvertIntToChar(track_soloed,nr),")");

				if(h)
				{
					g_solo->ChangeButtonText(h);
					delete h;
				}
			}
			else
				g_solo->ChangeButtonText("Solo");

			g_solo->SetColour(track_soloed>0?COLOUR_YELLOW:-1,track_soloed>0?COLOUR_BLACK:-1);
		}
	}
}

void Edit_Arrange::RefreshRealtime_Slow()
{
	CheckAutomationParameterUnderMouse();

	guiObject *o=guiobjects.FirstObject();
	while(o)
	{
		switch(o->id)
		{
		case OBJECTID_ARRANGECHANNELINFO:
			{
				Edit_Arrange_ChannelInfo *eaf=(Edit_Arrange_ChannelInfo *)o;

				if(AudioChannel *chl=eaf->channel->channel)
				{
					if(
						eaf->channel->hasinstruments!=chl->io.audioeffects.CheckIfEffectHasOnInstruments() ||
						eaf->channel->hasfx!=chl->io.audioeffects.CheckIfEffectHasNonInstrumentFX())
					{
						eaf->channel->ShowChannelInfo();
						eaf->Blt();
					}
				}
			}
			break;

		case OBJECTID_ARRANGETRACKINFO:
			{
				Edit_Arrange_TrackInfo *eaf=(Edit_Arrange_TrackInfo *)o;

				if(Seq_Track *track=eaf->track->track)
				{
					if(eaf->track->frozen!=track->frozen ||
						eaf->track->hasinstruments!=track->io.audioeffects.CheckIfEffectHasOnInstruments() ||
						eaf->track->hasfx!=track->io.audioeffects.CheckIfEffectHasNonInstrumentFX())
					{
						eaf->track->ShowTrackInfo();
						eaf->Blt();
					}
				}
			}
			break;

		case OBJECTID_ARRANGEAUTOMATIONCHANNELSETTINGS:
			{
				Edit_Arrange_ChannelAutomationSettings *eca=(Edit_Arrange_ChannelAutomationSettings *)o;

				if(eca->channel->channel->automationon != eca->onoff || eca->channel->channel->HasAutomation()!=eca->used)
				{
					eca->channel->ShowAutomationSettings();
					eca->Blt();
				}
			}
			break;

		case OBJECTID_ARRANGEAUTOMATIONSETTINGS:
			{
				Edit_Arrange_AutomationSettings *eas=(Edit_Arrange_AutomationSettings *)o;

				if(eas->track->track->automationon != eas->onoff || eas->track->track->HasAutomation()!=eas->used)
				{
					eas->track->ShowAutomationSettings();
					eas->Blt();
				}
			}
			break;

		case OBJECTID_ARRANGEAUTOMATIONTRACK:
			{
				Edit_Arrange_AutomationTrack *eat=(Edit_Arrange_AutomationTrack *)o;
				eat->RefreshRealtime();
			}
			break;
		}

		o=o->NextObject();
	}

	if(trackingindex!=mainsettings->arrangetracking[set])
	{
		ShowTracking();
	}

	if(Seq_Pattern *p=WindowSong()->GetFocusPattern())
	{
		if(g_info_name)
			g_info_name->CheckString(p->GetName(),false);

		if(g_info_pos)
			g_info_pos->CheckTime(p->GetPatternStart(),true);

		if(g_info_end)
			g_info_end->CheckTime(p->GetPatternEnd(),true);
	}

	/*
	return;

	int h_getcountselectedtracks=WindowSong()->GetCountSelectedTracks(),
	h_getcounttracks=WindowSong()->GetCountOfTracks(),
	h_getcountselectedpattern=WindowSong()->GetCountOfPattern(0,true,false),
	h_getcountpattern=WindowSong()->GetCountOfPattern(0,false,false);

	if(h_getcountselectedtracks!=getcountselectedtracks ||
	h_getcounttracks!=getcounttracks ||
	h_getcountselectedpattern!=getcountselectedpattern ||
	h_getcountpattern!=getcountpattern
	)
	{
	SongNameRefresh();
	}
	*/
}

bool Edit_Arrange::CheckAuto()
{
	BuildTrackList(&realtimecheck,tracks,zoomy,GetShowFlag()&SHOW_AUTOMATION?true:false);

	OObject *cf=(OObject *)realtimecheck.GetRoot(),*ff=(OObject *)trackobjects.GetRoot();

	if(!cf)
	{
		if(ff)
			goto refresh;

		goto exit;
	}

	for(;;)
	{
		if(cf && (!ff))
		{
			TRACE ("1\n");
			goto refresh;
		}

		if((!cf) && ff)
		{
			TRACE ("2\n");
			goto refresh;
		}

		if(cf && ff)
		{
			if(cf->object!=ff->object) // Track == Track ?
			{
				//maingui->MessageBoxOk(0,"3");
				TRACE ("3\n");
				goto refresh;
			}
		}
		else 
			break;

		cf=(OObject *)cf->next;
		ff=(OObject *)ff->next;
	}

exit:
	realtimecheck.DeleteAllO(0);
	return false;

refresh:

	realtimecheck.DeleteAllO(0);
	RefreshTracking();
	ShowOverview();

	return true;
}

void Edit_Arrange::RefreshRealtime()
{	
	if(CheckAuto()==true)
		return;

	bool refreshtracks=false,refreshpattern=false,showsprites=false,refreshoverview=false;

	if(RefreshEventEditorRealtime()==true)	
	{
		if(pattern)
		{
			ShowCycleAndPositions(pattern);
			pattern->DrawSpriteBlt();
		}

		if(tempo)
		{
			ShowCycleAndPositions(tempo);
			tempo->DrawSpriteBlt();
		}

		if(signature)
		{
			ShowCycleAndPositions(signature);
			signature->DrawSpriteBlt();
		}
	}

	// MIDIEffect Editor
	//trackfx.RefreshRealtime();

	guiObject *o=guiobjects.FirstObject();
	while(o)
	{
		bool reset=false;

		switch(o->id)
		{
		case OBJECTID_ARRANGEAUTOMATIONTRACK:
			if(refreshpattern==false)
			{
				Edit_Arrange_AutomationTrack *at=(Edit_Arrange_AutomationTrack *)o;

				Edit_Arrange_AutomationPattern *ap=(Edit_Arrange_AutomationPattern *)at->pattern.GetRoot();

				while(ap)
				{
					// Fade In/Out Volume
					if(ap->patterncurve)
					{
						if( ap->fadeinms!=ap->patterncurve->fadeinms || 
							ap->fadeoutms!=ap->patterncurve->fadeoutms || 
							ap->volume!=ap->patterncurve->dbvolume || 
							ap->fadeactive!=ap->patterncurve->fadeinoutactive ||
							ap->volumeactive!=ap->patterncurve->volumeactive ||
							ap->fadeintype!=ap->patterncurve->fadeintype ||
							ap->fadeouttype!=ap->patterncurve->fadeouttype
							)
						{
							refreshpattern=true;
							break;
						}
					}

					ap=(Edit_Arrange_AutomationPattern *)ap->next;
				}
			}
			break;

		case OBJECTID_ARRANGEPATTERN:
			if(refreshpattern==false)
			{
				Edit_Arrange_Pattern *p=(Edit_Arrange_Pattern *)o;

#ifdef DEBUG
				if(p->pattern->itsaloop==true)
				{
					int i=0;
				}

#endif

				// Check Pattern Name
				if(p->notext==false && p->patternname && p->pattern->GetName() && (strcmp(p->patternname,p->pattern->GetName())!=0))
					p->ShowPattern_Blt();
				else
				{
					// Fade In/Out Volume
					if(p->patterncurve)
					{
						if( p->fadeinms!=p->patterncurve->fadeinms || 
							p->fadeoutms!=p->patterncurve->fadeoutms || 
							p->volume!=p->patterncurve->dbvolume || 
							p->fadeactive!=p->patterncurve->fadeinoutactive ||
							p->volumeactive!=p->patterncurve->volumeactive ||
							p->fadeintype!=p->patterncurve->fadeintype ||
							p->fadeouttype!=p->patterncurve->fadeouttype ||
							p->fadeeditmode!=p->patterncurve->editmode
							)
							refreshpattern=true;
					}
				}

				if(p->pattern->mediatype==MEDIATYPE_MIDI)
				{
					MIDIPattern *mp=(MIDIPattern *)p->pattern;

					if(p->sysstartup!=mp->sendonlyatstartup)
						refreshpattern=true;

				}

				bool focus=WindowSong()->GetFocusPattern()==p->pattern?true:false;

				if(p->selectflag!=p->pattern->IsSelected() ||
					focus!=p->focus ||

					//	p->mouseselection!=p->pattern->mouseselection ||
					p->muteflag!=p->pattern->CheckIfPlaybackIsAble() ||
					p->mediatype!=p->pattern->mediatype || 
					p->recordstatus!=p->pattern->recordpattern ||
					p->pattern->GetColour()->Compare(&p->patterncolour)==false)
					refreshpattern=true;

			}
			break;

		case OBJECTID_ARRANGECHANNEL:
			{
				Edit_Arrange_Channel *c=(Edit_Arrange_Channel *)o;

				if(c->channel->IsSelected()!=c->channelselected)
					refreshtracks=refreshpattern=true;

				// Check Mute Button
				if(c->mute.ondisplay==true && c->mute.status!=c->channel->io.audioeffects.GetMute())
					refreshtracks=refreshpattern=true;

				if(c->name.ondisplay==true && 
					((c->namestring && strcmp(c->channel->GetName(),c->namestring)!=0))
					)
					refreshtracks=true;

				// Check Solo Button
				if(c->solo.ondisplay==true)
				{
					if(c->solo.status!=c->channel->GetSoloStatus())
					{
						refreshtracks=refreshpattern=true;
					}
				}

				c->ShowMIDIDisplay(false);
				c->ShowAudioDisplay(false);	

				if(c->volume.ondisplay==true)
				{
					if(c->channel->io.audioeffects.volume.value!=c->volume.volume ||
						c->volumeeditable!=c->channel->CanAutomationObjectBeChanged(&c->channel->io.audioeffects.volume,0,0) ||
						c->volumeclicked!=c->channel->volumeclicked
						)
					{
						c->ShowVolume(true);
						c->volume.Blt();
					}
				}

				if(c->MIDIvolume.ondisplay==true)
				{
					if(c->channel->MIDIfx.GetVelocity()!=c->MIDIvolume.MIDIvelocity ||
						c->m_volumeclicked!=c->channel->m_volumeclicked
						)
					{
						c->ShowMIDIVolume();
						c->MIDIvolume.Blt();
					}
				}
			}
			break;

		case OBJECTID_ARRANGETRACK:
			{
				// Mute, Record Object ?
				Edit_Arrange_Track *et=(Edit_Arrange_Track *)o;

				// Data Tracks

				if(et->track->GetColour()->Compare(&et->trackcolour)==false)
				{
					refreshtracks=refreshpattern=true;
				}

				if(et->track->IsSelected()!=et->trackselected)
					refreshtracks=refreshpattern=true;

				if(et->record.ondisplay==true && et->record.recordmode!=et->track->recordtracktype)
					refreshtracks=true;

				if(et->name.ondisplay==true && 
					((et->namestring && strcmp(et->track->GetName(),et->namestring)!=0))
					)
					refreshtracks=true;

				if(et->MIDIvolume.ondisplay==true)
				{
					if(et->track->t_trackeffects.GetVelocity_NoParent()!=et->MIDIvolume.MIDIvelocity ||
						et->MIDIvolume.editable!=et->track->CanAutomationObjectBeChanged(&et->track->MIDIfx.velocity,0,0) ||
						et->m_volumeclicked!=et->track->m_volumeclicked
						)
					{
						et->ShowMIDIVolume();
						et->MIDIvolume.Blt();
					}
				}

				if(et->volume.ondisplay==true)
				{
					if(et->track->io.audioeffects.volume.value!=et->volume.volume ||
						et->volumeeditable!=et->track->CanAutomationObjectBeChanged(&et->track->io.audioeffects.volume,0,0) ||
						et->volumeclicked!=et->track->volumeclicked
						)
					{
						et->ShowVolume();
						et->volume.Blt();
					}
				}

				if(et->audioinputmonitoring.ondisplay==true)
				{
					if(et->track->io.inputmonitoring!=et->audioinputmonitoring.status)
					{
						et->ShowAudioInputMonitoring();
						et->audioinputmonitoring.Blt();
					}
				}

				// Check Mute Button
				if(et->mute.ondisplay==true && et->mute.status!=et->track->GetMute())
					refreshtracks=refreshpattern=true;

				if(et->playable!=et->track->CheckIfPlaybackIsAble())
					refreshtracks=refreshpattern=true;

				// Check Solo Button
				//if(et->solo.ondisplay==true)
				{
					int status=et->track->GetSoloStatus();

					if(status!=et->solostatus)
					{
						et->solostatus=status;
						refreshtracks=refreshpattern=true;
					}
				}

				// Check Record Button
				if(et->record.ondisplay==true && et->record.status!=et->track->record){

					refreshtracks=true;
					refreshpattern=true;
				}

				if(et->type.ondisplay==true && et->type.status!=et->GetType())
					refreshtracks=true;
				//et->ShowType(true);

				// MIDI 
				//if(et->record.ondisplay==true && et->dataoutputx>et->record.x2){

				et->ShowMIDIDisplay(false);
				et->ShowAudioDisplay(false);	
				//}

				// refresh Audio Recording Pattern
				if(et->track->record_MIDIPattern)
				{
					if(rrt_slowcounter>=10)
						refreshoverview=true;

					if(refreshpattern==false)
					{
						if(Edit_Arrange_Pattern *fp=FindPattern(et->track->record_MIDIPattern))
						{
							if(fp->RefreshPositions()==true)
								refreshpattern=true;
						}
						else
						{
							if(et->track->record_MIDIPattern->CheckIfInRange(startposition,endposition,true))
								refreshpattern=true; // new pattern found ?	
						}
					}
				}

				// refresh Audio Recording Pattern
				if(
					(!(WindowSong()->status&Seq_Song::STATUS_MIDIWAIT)) && (WindowSong()->status&Seq_Song::STATUS_RECORD) 
					)
				{
					for(int i=0;i<MAXRECPATTERNPERTRACK;i++){
						if(
							et->track->audiorecord_audiopattern[i] && 
							et->track->audiorecord_audiopattern[i]->audioevent.audioefile &&
							et->track->audiorecord_audiopattern[i]->audioevent.audioefile->recordingactive==true
							){
								if(rrt_slowcounter>=10)
									refreshoverview=true;

								if(refreshpattern==false)
								{
									if(Edit_Arrange_Pattern *fp=FindPattern(et->track->audiorecord_audiopattern[i])){

										if(fp->RefreshPositions()==true)
											refreshpattern=true;
									}
									else
										if(et->track->audiorecord_audiopattern[i]->CheckIfInRange(startposition,endposition,true))						
											refreshpattern=true; // new pattern found ?
								}
						}
					}
				}
			}
			break;

		}// switch

		o=o->NextObject();
	}

	//BlinkActivePattern(false);
	RefreshAudioPeak(); // +Freeze

	/*
	if(focustrackindex!=WindowSong()->GetOfTrack(WindowSong()->GetFocusTrack()) || activewindownamepattern!=activepattern ||
	(activepattern && ((!activepatternwindowname_string) || strcmp(activepattern->GetName(),activepatternwindowname_string)!=0)) ||
	(WindowSong()->GetFocusTrack() && ((!focustrackwindowname_string) || strcmp(WindowSong()->GetFocusTrack()->GetName(),focustrackwindowname_string)!=0))
	)
	{
	//RefreshWindowTitle();
	}
	*/

	if(refreshtracks==true)
		DrawDBBlit(tracks);

	if(refreshpattern==true)
		DrawDBBlit(pattern);
	else
	{
		// Refresh Autoparameter

		guiObject *o=guiobjects.FirstObject();
		while(o)
		{
			bool reset=false;

			switch(o->id)
			{
			case OBJECTID_ARRANGEAUTOMATIONTRACK:
				if(pattern)
				{
					Edit_Arrange_AutomationTrack *eat=(Edit_Arrange_AutomationTrack *)o;

					if(eat->automationtrack->bindtoautomationobject && eat->accesscount!=eat->automationtrack->parameter.accesscounter)
					{
						eat->DrawPattern(true,true,true);
						pattern->Blt(0,eat->y,pattern->GetX2(),eat->y2);
					}
				}
				break;

			}// switch

			o=o->NextObject();
		}
	}

	if(refreshoverview==true)
		DrawDBBlit(overview);

	ShowSoloMute(false);
}


void Edit_Arrange_PatternList_Pattern::ShowName(bool blit)
{
#ifdef OLDIE
	int colour;

	/*
	if(pattern->mouseselection==true)
	{
	colour=COLOUR_BLACK;
	}
	else
	*/
	{
		//if(pattern->folder)
		//	colour=COLOUR_YELLOW_LIGHT;
		//else
		{
			if(pattern->flag&OFLAG_SELECTED)
				colour=COLOUR_SELECTED;
			else
			{
				if(pattern->mainpattern)
					colour=COLOUR_BLUE_LIGHT;
				else
					if(pattern->mainclonepattern)
						colour=COLOUR_GREEN;
					else
						colour=COLOUR_GREY_LIGHT;
			}
		}
	}

	bitmap->guiFillRect(posx2+1,y,namex2,y+maingui->GetFontSizeY(),colour);
	bitmap->guiDrawText(posx2+1,y+maingui->GetFontSizeY(),namex2,pattern->GetName());

	if(blit==true)
		editor->BltGUIBuffer(this);
#endif

}

void Edit_Arrange_PatternList_Pattern::ShowPatternPos()
{
#ifdef OLDIE
	char h[255],h2[NUMBERSTRINGLEN];

	Seq_Pos spos(Seq_Pos::POSMODE_COMPRESS);

	OSTART s=pattern->track->song->timetrack.ConvertTicksToMeasureTicks(pattern->GetPatternStart(),false),
		e=pattern->track->song->timetrack.ConvertTicksToMeasureTicks(pattern->GetPatternEnd(),true);

	pattern->track->song->timetrack.ConvertTicksToPos(s,&spos);

	strcpy(h,mainvar->ConvertIntToChar(spos.pos[0],h2));
	mainvar->AddString(h,"-");

	pattern->track->song->timetrack.ConvertTicksToPos(e,&spos);
	mainvar->AddString(h,mainvar->ConvertIntToChar(spos.pos[0],h2));

	bitmap->guiDrawText(posx+1,y+maingui->GetFontSizeY(),posx2,h);
#endif
}


void Edit_Arrange_PatternList_Pattern::ShowPattern()
{
#ifdef OLDIE
	ShowMute();
	ShowPatternPos();
	ShowName(false);
#endif
}