#include "gui.h"
#include "sampleeditor.h"
#include "audiofile.h"
#include "audiorealtime.h"
#include "objectpattern.h"
#include "object_track.h"
#include "seqtime.h"
#include "audiohardware.h"
#include "camxgadgets.h"
#include "songmain.h"
#include "editbuffer.h"
#include "editfunctions.h"
#include "gui.h"
#include "languagefiles.h"
#include "object_song.h"
#include "peakbuffer.h"
#include "semapores.h"
#include "object_project.h"
#include "audiohdfile.h"
#include "audiofilework.h"

#define READWRITEBUFFERSIZE 4096

// FRAMES
#define SAMPLE_FRAMEID_SAMPLE 0
#define SAMPLE_FRAMEID_HEADER 1
#define SAMPLE_FRAMEID_OVERVIEW 2


#define SAMPLEGADGETID_START GADGET_ID_START+50
#define REGIONS_ID SAMPLEGADGETID_START+1
//#define REGIONNAME_ID SAMPLEGADGETID_START+2
//#define REGIONCOPY_ID SAMPLEGADGETID_START+3


enum{
	GADGETID_PLAY=GADGETID_EDITORBASE,
	GADGETID_STOP,
	GADGETID_PLAYCLIP,

	GADGETID_INFOTIME_I,
	GADGETID_INFOTIME,
	GADGETID_INFOEND_I,
	GADGETID_INFOEND,
	GADGETID_SONGSAMPLES,
	GADGETID_SCALE,
	GADGETID_VOLUMECURVE,
	GADGETID_REGIONS,
	GADGETID_PATTERN,
	GADGETID_PATTERNNAME,

	GADGETID_FILE,
	GADGETID_FILENAME,
};

#define GADGETID_REGIONPLAY SAMPLEGADGETID_START+7
#define GADGETID_REGIONPLAYEND SAMPLEGADGETID_START+8
#define GADGETID_REGIONPLAYSTART SAMPLEGADGETID_START+9
#define GADGETID_SEEKPLAY SAMPLEGADGETID_START+10
#define GADGETID_SAMPLETIME SAMPLEGADGETID_START+11

// Menus
#define AUDIOMENU_START MENU_ID_START+50
#define SAMPLESTEP_XPIXEL 84

#define MENU_SAMPLES AUDIOMENU_START+1
#define MENU_STDMINSECMS AUDIOMENU_START+2
#define MENU_SONGMEASURE AUDIOMENU_START+3

#define MENU_NEWREGION AUDIOMENU_START+30

#define MENU_OPENMANAGER AUDIOMENU_START+31
#define MENU_CREATEAUDIOREGIONBUFFER AUDIOMENU_START+32
#define MENU_CREATEAUDIOBUFFER AUDIOMENU_START+33
#define MENU_CREATEAUDIOREGIONRANGE AUDIOMENU_START+34

#define EDITSTART_X 100
#define SAMPLE_SLIDER_RESOLUTION 1000 // 0.1

#define SAMPLEPERPIXELFACTOR 40

#define SEEKZEROSIZE 4096

class menu_createfilefromregion:public guiMenu
{
public:
	menu_createfilefromregion(Edit_Sample *ed){editor=ed;}

	void MenuFunction()
	{
		editor->CreateNewFileFromRegion();
	}

	Edit_Sample *editor;
};

class menu_normalizeaudiofile:public guiMenu
{
public:
	menu_normalizeaudiofile(Edit_Sample *ed){editor=ed;}

	void MenuFunction()
	{
		// editor->CreateNewFileFromRegion();
		editor->Normalize();
	}

	Edit_Sample *editor;
};

class menu_cutregion:public guiMenu
{
public:
	menu_cutregion(Edit_Sample *ed){editor=ed;}

	void MenuFunction()
	{
		editor->CutRange();
	}

	Edit_Sample *editor;
};

class menu_fillregion:public guiMenu
{
public:
	menu_fillregion(Edit_Sample *ed){editor=ed;}

	void MenuFunction()
	{
		editor->FillRange_Zero();
	}

	Edit_Sample *editor;
};

void menu_testregion::MenuFunction()
{
	if(editor->audiohdfile)
	{
		bool seek=false;
		bool regionok=false;

		if(editor->clipstart!=1 && editor->clipend!=-1)
			regionok=true;

		editor->DeleteTestRegion();

		LONGLONG start=-1;
		LONGLONG end=0;

		switch(mode)
		{
		case Edit_Sample::PLAYREGION_MODE_CLIP:
			{
				if(editor->clipstart>=0 && editor->clipend>=0)
				{
					start=editor->clipstart;
					end=editor->clipend;
				}
			}
			break;

		case Edit_Sample::PLAYREGION_MODE_STOPATMOUSEPOSITION:
			{
				if(editor->samplemouseposition!=1 && editor->samplestartposition<editor->samplemouseposition)
				{
					start=editor->samplestartposition;
					end=editor->samplemouseposition;
				}
			}
			break;

		case Edit_Sample::PLAYREGION_MODE_MOUSEPOSITION:
			{
				if(editor->samplemouseposition!=1)
				{
					start=editor->samplemouseposition;
					end=editor->audiohdfile->samplesperchannel;
				}
			}
			break;

		case Edit_Sample::PLAYREGION_MODE_FULL:
			{
				if(regionok==true)
				{
					start=editor->clipstart;
					end=editor->clipend;
				}
			}
			break;

		case Edit_Sample::PLAYREGION_MODE_START:
			{
				if(region)
				{
					start=region->regionstart;
					end=region->regionend;
				}
				else
				if(regionok==true)
				{
					start=editor->samplestartposition;
					end=editor->clipstart;
				}
			}
			break;

		case Edit_Sample::PLAYREGION_MODE_END:
			{
				if(regionok==true)
				{
					start=editor->clipend;
					end=editor->audiohdfile->samplesperchannel;
				}
			}
			break;

		case Edit_Sample::PLAYREGION_MODE_SAMPLEPOSITION:
			{
				start=editor->samplestartposition;
				end=editor->audiohdfile->samplesperchannel;
			}
			break;

		case Edit_Sample::PLAYREGION_MODE_SAMPLEPOSITIONSEEK:
			{
				start=editor->samplestartposition;
				end=editor->audiohdfile->samplesperchannel;

				if(regionok==true && editor->clipstart>start && editor->clipstart<end)
				{
					seek=true;
				}
			}
			break;
		}

		if(start>=0 && end>0)
		{
			editor->testregion=new AudioRegion(editor->audiohdfile);

			if(editor->testregion)
			{
				// Start Position/End
				editor->testregion->regionstart=start; 
				editor->testregion->regionend=end;

				// Seek ? <->
				editor->testregion->regionseek=seek;
				editor->testregion->seekstart=editor->clipstart;
				editor->testregion->seekend=editor->clipend;

				editor->testregion->InitRegion();
				editor->PlayTestRegion();

				editor->playmode=mode;
			}
		}
	}
}

class menu_findzerosample:public guiMenu
{
public:
	menu_findzerosample(Edit_Sample *ed,bool r)
	{
		editor=ed;
		right=r;
	}

	void MenuFunction()
	{
		if(editor->audiohdfile && editor->clipstart!=-1 && editor->clipend!=-1)
		{
			LONGLONG h;
			bool changed=false;

			if(right==false)
			{
				h=editor->audiohdfile->FindZeroSamples(editor->clipstart);

				if(h!=editor->clipstart && h<editor->clipend)
				{
					editor->ClipRange(h,editor->clipend);
					editor->ShowClipNumbers();
				}
			}
			else
			{
				h=editor->audiohdfile->FindZeroSamples(editor->clipend);

				if(h!=editor->clipend)
				{
					editor->ClipRange(editor->clipstart,h);
					editor->ShowClipNumbers();
				}
			}
		}
	}

	Edit_Sample *editor;
	bool right;
};

LONGLONG Edit_Sample::ConvertXPositionToOverviewSample(int posx)
{
	/*
	if(frameoverview.ondisplay==true && posx>=frameoverview.x && posx<=frameheader.x2)
	{
	double w=frameoverview.x2-frameoverview.x;
	double wh=posx-frameoverview.x;

	wh/=w;
	wh*=audiohdfile->samplesperchannel;
	return (LONGLONG)wh;
	}
	*/
	return -1;
}
int Edit_Sample::ConvertSamplePositionToOverviewX(LONGLONG pos)
{
	/*
	if(frameoverview.ondisplay==true && pos<=audiohdfile->samplesperchannel)
	{
	int x=frameoverview.x;
	double w=frameoverview.x2-frameoverview.x;
	double per=pos;

	per/=audiohdfile->samplesperchannel;
	w*=per;
	x+=(int)w;

	return x;
	}
	*/
	return -1;
}

int Edit_Sample::ConvertSamplePositionToX(LONGLONG pos)
{
	/*
	if(frameheader.ondisplay==true && pos>=displaystart && pos<displayend)
	{
	int x=frameheader.x;
	LONGLONG diffpos=pos-displaystart;

	diffpos/=(samplesperpixel*SAMPLEPERPIXELFACTOR);
	x+=(int)diffpos;

	return x;
	}
	*/

	return -1;
}

void Edit_Sample::RefreshRealtime_Slow()
{
	if(patternname)
		patternname->CheckString(pattern->GetName(),false);

}

void Edit_Sample::ShowSpecialEditorRange()
{
	ShowClipWave();
}

void Edit_Sample::RefreshRealtime()
{
	if(status==AR_STARTED && endstatus_realtime==true)
	{
		status=AR_STOPPED;
		audiorealtime=0;
	}

	if(timeline && songmode==true)
	{
		if(RefreshEventEditorRealtime()==true)	
		{
			if(samples)
			{
				ShowCycleAndPositions(samples);
				samples->DrawSpriteBlt();
			}
		}
	}

	if(songmode==true)
	{
		// Fade In/Out Volume
		if(patternvolumecurve)
		{
			if(fadeinms!=patternvolumecurve->fadeinms || 
				fadeoutms!=patternvolumecurve->fadeoutms || 
				volume!=patternvolumecurve->dbvolume || 
				fadeactive!=patternvolumecurve->fadeinoutactive ||
				volumeactive!=patternvolumecurve->volumeactive ||
				fadeintype!=patternvolumecurve->fadeintype ||
				fadeouttype!=patternvolumecurve->fadeouttype ||
				fadeeditmode!=patternvolumecurve->editmode
				)
			{
				DrawDBBlit(samples,overview);
				return;
			}

		}
	}

	bool refreshsprites=false;

	if(audiorealtime) // File playback ?
	{
		// Check If Realtime Audio Stopped
		mainthreadcontrol->Lock(CS_audiorealtime);
		if(mainaudioreal->FindAudioRealtime(audiorealtime)==false)
		{
			audiorealtime=0;
		}
		mainthreadcontrol->Unlock(CS_audiorealtime);

		if(audiorealtime)
		{

#ifdef OLDIE
			if(frameoverview.ondisplay==true)
			{
				//LONGLONG fpos=(audiorealtime->audiopattern.audioevent.fileposition-audiorealtime->audiopattern.audioevent.audioefile->datastart)/audiorealtime->audiopattern.audioevent.audioefile->samplesize_all_channels;

				if(audiorealtime->audiopattern.audioevent.sampleposition!=frealtimepos)
				{
					frealtimepos=audiorealtime->audiopattern.audioevent.sampleposition;

					if(samplepos)
					{
						Seq_Pos spos;
						spos.InitWithWindowDisplay(mainvar->GetActiveProject(),windowdisplay);

						char slen[NUMBERSTRINGLEN];

						mainaudio->ConvertSamplesToTime(frealtimepos,&spos);
						spos.ConvertToString(WindowSong(),slen,NUMBERSTRINGLEN);

						samplepos->ChangeButtonText(slen);
					}
				}

				int x=ConvertSamplePositionToOverviewX(frealtimepos);

				if(x==-1)
					RemoveSprite(&overview_filepositionsprite);
				else
					if(x!=overview_filepositionsprite.x || overview_filepositionsprite.ondisplay==false)
					{
						ClearSprite(&overview_filepositionsprite);

						overview_filepositionsprite.colour=COLOUR_BLUE;
						overview_filepositionsprite.x=x;
						overview_filepositionsprite.y=frameoverview.y;
						overview_filepositionsprite.y2=frameoverview.y2;

						AddSprite(&overview_filepositionsprite);
						refreshsprites=true;
					}
			}

			if(frameheader.ondisplay==true)
			{
				int x=ConvertSamplePositionToX(audiorealtime->audiopattern.audioevent.sampleposition);

				if(x==-1)
				{
					RemoveSprite(&header_filepositionsprite);
					RemoveSprite(&sample_filepositionsprite);
				}
				else
					if(x!=header_filepositionsprite.x || header_filepositionsprite.ondisplay==false)
						// header sprite
					{
						ClearSprite(&header_filepositionsprite);

						header_filepositionsprite.colour=COLOUR_BLUE;
						header_filepositionsprite.x=x;
						header_filepositionsprite.y=frameheader.y;
						header_filepositionsprite.y2=frameheader.y2;

						AddSprite(&header_filepositionsprite);

						refreshsprites=true;

						if(framesample.ondisplay==true) // sample sprite
						{
							ClearSprite(&sample_filepositionsprite);

							sample_filepositionsprite.colour=COLOUR_BLUE;
							sample_filepositionsprite.x=x;
							sample_filepositionsprite.y=framesample.y;
							sample_filepositionsprite.y2=framesample.y2;

							AddSprite(&sample_filepositionsprite);
						}
					}
			}
#endif

		}
	}


	if(status!=playmode)
	{
		ShowStatus();
	}

#ifdef OLDIE

	// Start Position Pointer
	if(audiohdfile)
	{
		if(frameoverview.ondisplay==true)
		{
			int x=ConvertSamplePositionToOverviewX(samplestartposition);

			if(x==-1)
				RemoveSprite(&startpositionsprite_overviwe);
			else
				if(x!=startpositionsprite_overviwe.x || startpositionsprite_overviwe.ondisplay==false)
				{
					ClearSprite(&startpositionsprite_overviwe);

					startpositionsprite_overviwe.colour=COLOUR_GREEN;
					startpositionsprite_overviwe.x=x;
					startpositionsprite_overviwe.y=frameoverview.y;
					startpositionsprite_overviwe.y2=frameoverview.y2;

					AddSprite(&startpositionsprite_overviwe);
					refreshsprites=true;
				}
		}

		// Sample
		if(framesample.ondisplay==true)
		{
			// Check Peak File Progress
			if(audiohdfile==audiopeakthread->GetRunningFile())
				ShowPeakProgress(audiopeakthread->createprogress);

			int x=ConvertSamplePositionToX(samplestartposition);

			if(x==-1)
				RemoveSprite(&startpositionsprite);
			else
				if(x!=startpositionsprite.x || startpositionsprite.ondisplay==false)
				{
					ClearSprite(&startpositionsprite);

					startpositionsprite.colour=COLOUR_GREEN;
					startpositionsprite.x=x;
					startpositionsprite.y=framesample.y;
					startpositionsprite.y2=framesample.y2;

					AddSprite(&startpositionsprite);

					refreshsprites=true;
				}
		}
	}

	if(audiorealtime) // File playback ?
	{
		// Check If Realtime Audio Stopped
		mainthreadcontrol->Lock(CS_audiorealtime);
		if(mainaudioreal->FindAudioRealtime(audiorealtime)==false)
		{
			audiorealtime=0;
		}
		mainthreadcontrol->Unlock(CS_audiorealtime);

		if(audiorealtime)
		{
			if(frameoverview.ondisplay==true)
			{
				//LONGLONG fpos=(audiorealtime->audiopattern.audioevent.fileposition-audiorealtime->audiopattern.audioevent.audioefile->datastart)/audiorealtime->audiopattern.audioevent.audioefile->samplesize_all_channels;

				if(audiorealtime->audiopattern.audioevent.sampleposition!=frealtimepos)
				{
					frealtimepos=audiorealtime->audiopattern.audioevent.sampleposition;

					if(samplepos)
					{
						Seq_Pos spos;
						spos.InitWithWindowDisplay(mainvar->GetActiveProject(),windowdisplay);

						char slen[NUMBERSTRINGLEN];

						mainaudio->ConvertSamplesToTime(frealtimepos,&spos);
						spos.ConvertToString(WindowSong(),slen,NUMBERSTRINGLEN);

						samplepos->ChangeButtonText(slen);
					}
				}

				int x=ConvertSamplePositionToOverviewX(frealtimepos);

				if(x==-1)
					RemoveSprite(&overview_filepositionsprite);
				else
					if(x!=overview_filepositionsprite.x || overview_filepositionsprite.ondisplay==false)
					{
						ClearSprite(&overview_filepositionsprite);

						overview_filepositionsprite.colour=COLOUR_BLUE;
						overview_filepositionsprite.x=x;
						overview_filepositionsprite.y=frameoverview.y;
						overview_filepositionsprite.y2=frameoverview.y2;

						AddSprite(&overview_filepositionsprite);
						refreshsprites=true;
					}
			}

			if(frameheader.ondisplay==true)
			{
				int x=ConvertSamplePositionToX(audiorealtime->audiopattern.audioevent.sampleposition);

				if(x==-1)
				{
					RemoveSprite(&header_filepositionsprite);
					RemoveSprite(&sample_filepositionsprite);
				}
				else
					if(x!=header_filepositionsprite.x || header_filepositionsprite.ondisplay==false)
						// header sprite
					{
						ClearSprite(&header_filepositionsprite);

						header_filepositionsprite.colour=COLOUR_BLUE;
						header_filepositionsprite.x=x;
						header_filepositionsprite.y=frameheader.y;
						header_filepositionsprite.y2=frameheader.y2;

						AddSprite(&header_filepositionsprite);

						refreshsprites=true;

						if(framesample.ondisplay==true) // sample sprite
						{
							ClearSprite(&sample_filepositionsprite);

							sample_filepositionsprite.colour=COLOUR_BLUE;
							sample_filepositionsprite.x=x;
							sample_filepositionsprite.y=framesample.y;
							sample_filepositionsprite.y2=framesample.y2;

							AddSprite(&sample_filepositionsprite);
						}
					}
			}	
		}
	}
	else
	{
		if(frealtimepos!=-1)
		{
			frealtimepos=-1;

			if(samplepos)
				samplepos->ChangeButtonText(Cxs[CXS_NOPLAYBACK]);
		}
	}

	if(audiohdfile)
	{
		if(clipshow==false && clip==true)
		{
			//	ShowClipWave();
		}

		if(clipoverview==false)
			ShowClipOverview();

		if( (audiohdfile->FirstRegion() && noregions==true) || (audiohdfile->FirstRegion()==0 && noregions==false))
		{
			winmode=WINDOWMODE_INIT;
			Init();
		}
	}

	if(refreshsprites==true)
	{
		ShowAllSprites();
	}
#endif

}

void Edit_Sample::DeInitWindow()
{
	if(overviewostartx)
		delete overviewostartx;

	overviewostartx=0;

	if(winmode&WINDOWMODE_DESTROY)
	{
		StopPlayback();
		DeleteTestRegion();
	}

	WindowSong()->RemoveFromSoloOnOff(this);
	patternselection.DeleteSelectionList();
}

void Edit_Sample::SaveFile()
{
	mainaudio->CopyAudioFile(this,audiohdfile);
}

guiMenu *Edit_Sample::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		guiMenu *file=menu->AddMenu(Cxs[CXS_FILE],0);
		if(file)
		{
			class menu_savefile:public guiMenu
			{
			public:
				menu_savefile(Edit_Sample *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					editor->SaveFile();
				}

				Edit_Sample *editor;
			};

			file->AddFMenu(Cxs[CXS_EXSAVEAFILE],new menu_savefile(this));
			file->AddFMenu(Cxs[CXS_EXSAVEREGION],new menu_createfilefromregion(this));
		}

		guiMenu *s=menu->AddMenu(Cxs[CXS_FUNCTIONS],0);
		if(s)
		{
			s->AddFMenu("Cut selected Range",new menu_cutregion(this));
			s->AddFMenu("Fill selected Range with 0 Samples",new menu_fillregion(this));

			s->AddLine();
			s->AddFMenu("Normalize File",new menu_normalizeaudiofile(this));
		}

		s=menu->AddMenu("Region",0);
		if(s)
		{	
			s->AddFMenu(Cxs[CXS_CREATEREGION],new menu_createregion(this));
			//s->AddFMenu(Cxs[CXS_DELETEREGION],new menu_deleteregion(this));
			s->AddLine();


			if(char *h=mainvar->GenerateString(Cxs[CXS_PLAYREGIONSE],"\tCtrl+P"))
			{
				s->AddFMenu(h,new menu_testregion(this,PLAYREGION_MODE_FULL));
				delete h;
			}

			if(char *h=mainvar->GenerateString(Cxs[CXS_PLAYREGIONFROMEND],"\tCtrl+E"))
			{
				s->AddFMenu(h,new menu_testregion(this,PLAYREGION_MODE_END));
				delete h;
			}

			if(char *h=mainvar->GenerateString(Cxs[CXS_PLAYANDSTOPATREGION],"\tCtrl+S"))
			{
				s->AddFMenu(h,new menu_testregion(this,PLAYREGION_MODE_START));
				delete h;
			}

			//s->AddFMenu("Play (Stop at Region start)\tCtrl+S",new menu_testregion(this,PLAYREGION_MODE_START));

			if(char *h=mainvar->GenerateString(Cxs[CXS_PLAYAUDIOFILECURSOR],"\tCtrl+O"))
			{
				s->AddFMenu(h,new menu_testregion(this,PLAYREGION_MODE_SAMPLEPOSITION));
				delete h;
			}

			//s->AddFMenu("Play from Cursor Position\tCtrl+O",new menu_testregion(this,PLAYREGION_MODE_SAMPLEPOSITION));

			if(char *h=mainvar->GenerateString(Cxs[CXS_PLAYANDSJUMPOVERREGION],"\tCtrl+L"))
			{
				s->AddFMenu(h,new menu_testregion(this,PLAYREGION_MODE_SAMPLEPOSITIONSEEK));
				delete h;
			}

			// s->AddFMenu("Play from Cursor Position and seek over Region\tCtrl+L",new menu_testregion(this,PLAYREGION_MODE_SAMPLEPOSITIONSEEK));
			s->AddLine();

			s->AddFMenu("Find Zero Sample (Region Start)",new menu_findzerosample(this,false));
			s->AddFMenu("Find Zero Sample (Region End)",new menu_findzerosample(this,true));
			s->AddLine();

			class menu_createaudiobuffer:public guiMenu
			{
			public:
				menu_createaudiobuffer(Edit_Sample *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					mainbuffer->CreateAudioBuffer(editor->audiohdfile,0);
				}

				Edit_Sample *editor;
			};

			s->AddFMenu(Cxs[CXS_COPYAUDIOFILE_CLIPBOARD],new menu_createaudiobuffer(this));
			s->AddLine();

			/*
			class menu_createaudioregionbuffer:public guiMenu
			{
			public:
				menu_createaudioregionbuffer(Edit_Sample *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					if(editor->activeregion)
						mainbuffer->CreateAudioBuffer(editor->audiohdfile,editor->activeregion);
				}

				Edit_Sample *editor;
			};

			s->AddFMenu(Cxs[CXS_COPYREGION_CLIPBOARD],new menu_createaudioregionbuffer(this));
*/

			/*
			class menu_createaudioregionrange:public guiMenu
			{
			public:
				menu_createaudioregionrange(Edit_Sample *ed)
				{
					editor=ed;
				}

				void MenuFunction()
				{
					if(editor->activeregion)
						mainedit->ChangeRegion(editor->activeregion,editor->clipstart,editor->clipend);
				}

				Edit_Sample *editor;
			};

			s->AddFMenu("Change Audio Region Range",new menu_createaudioregionrange(this));
			*/
		}

		s=menu->AddMenu("Editor",0);
		if(s)
		{
			class menu_openmanager:public guiMenu
			{
			public:
				void MenuFunction()
				{
					guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOMANAGER,0,0);

					if(win)
						win->WindowToFront(true);
					else
						maingui->OpenEditorStart(EDITORTYPE_AUDIOMANAGER,0,0,0,0,0,0);
				}
			};

			s->AddFMenu("Audio Manager",new menu_openmanager);
		}
	}

	maingui->AddCascadeMenu(this,menu);
	return menu;
}

void Edit_Sample::ShowMenu()
{
	if(displaymenu)
	{
		displaymenu->Select(0,false);
		displaymenu->Select(1,false);
		displaymenu->Select(2,false);

		displaymenu->Select(0,true);
	}
}

void Edit_Sample::RefreshTimeSlider_Ex()
{

}

void Edit_Sample::RefreshObjects(LONGLONG type,bool editcall)
{
	if(pattern)
	{
		if(audiohdfile!=pattern->audioevent.audioefile)
		{
			audiohdfile=pattern->audioevent.audioefile;

			if(patternname)
				patternname->ChangeButtonText(pattern->GetName());

			if(filename)
				filename->ChangeButtonText(audiohdfile->GetFileName());
		}

		ShowPatternPositions();
	}

	DrawDBBlit(editarea,overview);
}

void Edit_Sample::ShowTimeMode()
{
	if(time)
	{
		switch(windowtimeformat)
		{
		case WINDOWDISPLAY_SAMPLES:
			time->ChangeButtonImage(IMAGE_SAMPLES_SMALL);
			break;

		case WINDOWDISPLAY_SMPTE:
			{
				time->ChangeButtonText(smpte_modestring[mainvar->GetActiveProject()?mainvar->GetActiveProject()->standardsmpte:mainsettings->projectstandardsmpte]);
			}
			break;
		}
	}
}

void Edit_Sample::ShowOverview()
{
	if((!overview) || overview->GetWidth()==0)
		return;

	guiBitmap *bitmap=&overview->gbitmap;
	bitmap->guiFillRect(COLOUR_OVERVIEW_BACKGROUND);

	if(overviewostartx)
	{
		if(overviewostartsize!=overview->GetWidth())
		{
			delete overviewostartx;
			overviewostartx=0;
		}
	}

	if(!overviewostartx)
	{
		if(overviewostartsize=overview->GetWidth())
			overviewostartx=new LONGLONG[overviewostartsize+1]; // Add 1
	}

	if(songmode==true)
	{
		overviewlenght=WindowSong()->GetSongLength_Ticks();

		if(overviewostartx)
		{
			for(int x=0;x<overviewostartsize+1;x++) // +1
			{
				overviewostartx[x]=WindowSong()->timetrack.ConvertTicksToTempoSamples(GetOverviewTime(x));
			}
		}

		int hx=ConvertTimeToOverviewX(pattern->GetPatternStart());

		if(hx<0)
			return;

		int hx2=ConvertTimeToOverviewX(pattern->GetPatternEnd());

		if(hx2<0)
			hx2=bitmap->GetX2();
		else
			if(hx2==hx)
			{
				if(hx2<bitmap->GetX2())
					hx2++;
			}

			AudioGFX g;

			double sppixel;

			sppixel=WindowSong()->timetrack.ConvertTicksToTempoSamples(overviewlenght);

			sppixel/=overview->GetWidth();

			g.showmix=true;
			//	g.perstart=0; // sample position 0-1
			g.samplesperpixel=(int)floor(sppixel+0.5); 
			g.win=this;
			g.bitmap=bitmap;
			g.usebitmap=true;
			//g.dontclearbackground=true;
			g.drawcolour=COLOUR_OVERVIEWOBJECT;
			g.audiopattern=pattern;
			g.patternvolumecurve=pattern->GetVolumeCurve();

			g.x=hx;
			g.samplex2=g.x2=hx2;
			g.y=0;
			g.y2=bitmap->GetY2();
			g.ostartx=overviewostartx;
			g.eventstart=g.start=pattern->GetPatternStart();
			g.patternvolumecurve=0;
			g.showvolumecurve=false;

			bitmap->guiDrawRect(hx,0,hx2,bitmap->GetY2(),COLOUR_BLUE_LIGHT2);

			if(g.y+12<g.y2) // 12 Y Pixel min
			{
				audiohdfile->ShowAudioFile(WindowSong(),&g);
				clipoverview=false;
			}
	}
	else
	{
		AudioGFX g;

		double sppixel;

		sppixel=audiohdfile->samplesperchannel;
		sppixel/=overview->GetWidth();

		g.showmix=false;
		//g.perstart=0; // sample position 0
		g.samplesperpixel=(int)floor(sppixel+0.5); 
		g.win=this;
		g.bitmap=bitmap;
		g.usebitmap=true;
		g.showmix=true;

		g.drawcolour=COLOUR_OVERVIEWOBJECT;
		g.dontclearbackground=true;
		g.x=0;
		g.samplex2=g.x2=bitmap->GetX2();

		g.y=0;
		g.y2=bitmap->GetY2();

		if(g.y+12<g.y2) // 12 Y Pixel min
		{
			audiohdfile->ShowAudioFile(&g);

			/*
			if(activeregion && showregions==true)
			{
			int x=ConvertSamplePositionX(activeregion->regionstart),
			x2=ConvertSamplePositionX(activeregion->regionend);

			if(x!=-1 && x2!=-1)
			bitmap->guiInvert(x,g.y,x2,g.y2);
			}
			*/
		}
	}
}

/*
void Edit_Sample::ShowRegionName()
{
	if(activeregion)
	{
		if(regionname_gadget)
		{
			regionname_gadget->SetString(activeregion->regionname);
			regionname_gadget->Enable();
		}
	}
	else
	{
		if(regionname_gadget)
		{
			regionname_gadget->SetString("-");
			regionname_gadget->Disable();
		}
	}
}
*/

void Edit_Sample::ClipRange(LONGLONG s,LONGLONG e) // start<->end
{
	if(s!=-1 && e!=-1)
	{
		if(e<s) // swap<->
		{
			LONGLONG h=s;
			s=e;
			e=h;
		}

		clipstart=s;
		clipend=e;

		ShowCycleAndPositions(samples);	
		DrawDBSpriteBlit(editarea);

		ShowOverview();
		ShowClipNumbers();
	}
}

void Edit_Sample::PlayTestRegion()
{
	if(testregion)
	{
		StopPlayback();
		audiorealtime=audiohdfile->PlayRealtime(this,WindowSong(),testregion,0 /*channel*/,&endstatus_realtime,0,true);
	}
}

void Edit_Sample::DeleteTestRegion()
{
	if(testregion)
	{
		testregion->FreeMemory();
		delete testregion;

		testregion=0;
	}
}

LONGLONG AudioHDFile::FindZeroSamples(LONGLONG samplespos)
{
	LONGLONG found=samplespos;

	if(samplespos<=samplesperchannel && channels<255)
	{
		LONGLONG samples=samplesperchannel-samplespos;
		// Seek

		double *rbuffer=new double[SEEKZEROSIZE*channels]; // source
		ARES *samplebuffer=new ARES[SEEKZEROSIZE*channels]; // 24bit destination

		if(rbuffer && samplebuffer)
		{
			camxFile file;		// Read file
			int rbytes;
			LONGLONG size;
			int blockschecked=16; 

			ARES maxsum=1;

			if(file.OpenRead(name))
			{
				file.SeekBegin(GetStartOfSample(samplespos));

				while(samplespos<=samplesperchannel && maxsum!=0 && blockschecked)
				{	
					if(samples>=SEEKZEROSIZE)
					{
						rbytes=file.Read(rbuffer,SEEKZEROSIZE*samplesize_all_channels);

						samples-=SEEKZEROSIZE;
						size=SEEKZEROSIZE;
					}
					else
					{
						// rest
						rbytes=file.Read(rbuffer,samples*samplesize_all_channels);

						size=samples;
						samples=0;
					}

					if(rbytes)
					{
						AudioHardwareBuffer buffer;

						buffer.outputbufferARES=buffer.static_outputbufferARES=samplebuffer;

						ConvertReadBufferToSamples
							(
							rbuffer,
							&buffer,
							size,
							channels
							);			

						// Check Buffer
						ARES *ap=samplebuffer;

						while(size && maxsum!=0)
						{
							ARES sum=0;

							for(int i=0;i<channels;i++)
							{
								if(*ap<0)
									sum-=*ap++;
								else
									sum+=*ap++;
							}

							// Check sum
							if(sum<maxsum)
							{
								maxsum=sum;
								found=samplespos;
							}

							samplespos++;
							size--;
						}
					}
					else 
						break;

					blockschecked--;

				}// while

				file.Close(true);
			}
		}

		if(rbuffer)
			delete rbuffer;

		if(samplebuffer)
			delete samplebuffer;
	}

	return found;
}

void Edit_Sample::ClearClip()
{
#ifdef OLDIE
	if(clip==true)
	{
		clip=false;

		guibuffer->guiInvert(clipbitmapx,clipbitmapy,clipbitmapx2,clipbitmapy2);
		BltGUIBuffer(clipbitmapx,clipbitmapy,clipbitmapx2,clipbitmapy2);
	}
#endif

}

void Edit_Sample::ClearClipOverview()
{
#ifdef OLDIE
	if(clipoverview==true)
	{
		clipoverview=false;

		guibuffer->guiInvert(clipbitmapx_overview,clipbitmapy_overview,clipbitmapx2_overview,clipbitmapy2_overview);
		BltGUIBuffer(clipbitmapx_overview,clipbitmapy_overview,clipbitmapx2_overview,clipbitmapy2_overview);
	}
#endif
}

void Edit_Sample::ShowClipOverview()
{
	/*
	if(!(winmode&WINDOWMODE_RESIZE))
	{
	RemoveSpriteAndShow(FindSprite(guiSprite::SPRITEDISPLAY_OVERVIEWRANGE));

	clipoverview=true;

	if(frameoverview.ondisplay==true && frameheader.ondisplay==true && audiohdfile)
	{
	double start=(double)displaystart,
	end=(double)displayend,
	h=(double)audiohdfile->samplesperchannel;

	if(h>start)
	{
	start/=h;
	end/=h;

	int x,x2;

	// x
	x=x2=frameoverview.x;
	h=frameoverview.x2-frameoverview.x;
	h*=start;
	x+=(int)h;

	h=frameoverview.x2-frameoverview.x;
	h*=end;
	x2+=(int)h;

	if(x2>frameoverview.x2)
	x2=frameoverview.x2;

	guiSprite *s=new guiSprite(guiSprite::SPRITETYPE_RECT,guiSprite::SPRITEDISPLAY_OVERVIEWRANGE);

	if(s)
	{
	s->x=x;
	s->x2=x2;
	s->y=frameoverview.y;
	s->y2=frameoverview.y2;
	s->colour=COLOUR_YELLOW;

	AddSpriteShow(s); 
	}
	}
	}
	}
	*/
}

void Edit_Sample::ShowClipWave(bool autoclip)
{
	guiBitmap *bm=&samples->spritebitmap;

	if(!bm)
		return;

	if(clipstart!=-1 && clipend!=-1)
	{
		int cx,cx2;

		if(songmode==true)
		{
			LONGLONG sstartposition=WindowSong()->timetrack.ConvertTicksToTempoSamples(pattern->GetPatternStart());
			LONGLONG sendposition=sstartposition+clipend;

			sstartposition+=clipstart;

			OSTART os=WindowSong()->timetrack.ConvertSamplesToTicks(sstartposition);
			OSTART oe=WindowSong()->timetrack.ConvertSamplesToTicks(sendposition);

			if(os<endposition && oe>=startposition)
			{
				if(os<=startposition){
					clipx=-1;
					cx=0;
				}
				else
					cx=clipx=timeline->ConvertTimeToX(os);

				if(oe>=endposition){
					clipx2=-1;
					cx2=samples->GetX2();
				}
				else
					cx2=clipx2=timeline->ConvertTimeToX(oe);

				bm->guiFillRect(cx,0,cx2,samples->GetY2(),COLOUR_BLUE);
			}

		}// songmode
	}
}

void Edit_Sample::ShowClipWaveOverview(bool autoclip)
{
#ifdef OLDIE

	// BltGUIBuffer_Frame(&framesample);

	if(clipstart!=-1 && clipend!=-1)
	{
		int x=clipx_overview=ConvertSamplePositionToOverviewX(clipstart);
		int x2=clipx2_overview=ConvertSamplePositionToOverviewX(clipend);

		int cx=x,cx2=x2;


		/*
		bool noclip=false;

		if(autoclip==true &&
		clipoverview==true
		)
		{
		if(
		(cx==clipbitmapx_overview) && 
		(cx2>clipbitmapx2_overview)
		)
		cx=clipbitmapx2_overview;
		else
		if(
		(cx==clipbitmapx_overview) && 
		(cx2<clipbitmapx2_overview)
		)
		{
		cx=cx2;
		cx2=clipbitmapx2_overview;
		}
		else
		if(
		(cx2==clipbitmapx2_overview) && 
		(cx<clipbitmapx_overview)
		)
		cx2=clipbitmapx_overview;
		else
		if(
		(cx2==clipbitmapx2_overview) && 
		(cx>clipbitmapx_overview)
		)
		{
		cx2=cx;
		cx=clipbitmapx_overview;
		}
		else
		{
		if((cx!=clipbitmapx_overview) || (cx2!=clipbitmapx2_overview))
		ClearClipOverview();
		else
		noclip=true;
		}
		}

		if(noclip==false)
		*/
		{
			guibuffer->guiInvert(cx,frameoverview.y,cx2,frameoverview.y2);

			if(autoclip==true)
				BltGUIBuffer(cx,frameoverview.y,cx2,frameoverview.y2);

			clipbitmapx_overview=x;
			clipbitmapy_overview=frameoverview.y;

			clipbitmapx2_overview=x2;
			clipbitmapy2_overview=frameoverview.y2;
			clipoverview=true;
			clipshowoverview=true;
		}
	}

#endif

}

void Edit_Sample::ShowClipNumbers()
{
	return;

	Seq_Pos spos;
	spos.InitWithWindowDisplay(mainvar->GetActiveProject(),windowtimeformat);

	char slen[NUMBERSTRINGLEN];

	if(left_smpte)
	{
		if(clipstart>=0)
		{
			mainaudio->ConvertSamplesToTime(clipstart,&spos);
			spos.ConvertToString(WindowSong(),slen,NUMBERSTRINGLEN);

			left_smpte->SetString(slen);
		}
		else
		{
			left_smpte->SetString("-");
			left_smpte->Disable();
		}
	}

	if(left_samples)
	{
		if(clipstart>=0)
		{
			left_samples->SetInteger(clipstart);
		}
		else
		{
			left_samples->SetInteger(0);
			left_samples->Disable();
		}
	}

	if(right_smpte)
	{
		if(clipend>=0)
		{
			mainaudio->ConvertSamplesToTime(clipend,&spos);
			spos.ConvertToString(WindowSong(),slen,NUMBERSTRINGLEN);

			right_smpte->SetString(slen);
		}
		else
		{
			right_smpte->SetString("-");
			right_smpte->Disable();
		}
	}

	if(right_samples)
	{
		if(clipend>=0)
		{
			right_samples->SetInteger(clipend);
		}
		else
		{
			right_samples->SetInteger(0);
			right_samples->Disable();
		}
	}
}

void Edit_Sample::ShowStatus()
{
	status=playmode;

	switch(status)
	{
	case PLAYREGION_MODE_STOPPED:
		if(start)
			start->ChangeButtonImage(IMAGE_PLAYBUTTON_OFF);

		if(g_clipstart)
			g_clipstart->ChangeButtonImage(IMAGE_PLAYBUTTONCLIP_OFF);

		if(stop)
			stop->ChangeButtonImage(IMAGE_STOPBUTTON_ON);
		break;

	default:
		{
			switch(playmode)
			{
			case PLAYREGION_MODE_CLIP:
				{
					if(g_clipstart)
						g_clipstart->ChangeButtonImage(IMAGE_PLAYBUTTONCLIP_ON);
				}
				break;

			default:

				if(start)
					start->ChangeButtonImage(IMAGE_PLAYBUTTON_ON);
				break;
			}

			if(stop)
				stop->ChangeButtonImage(IMAGE_STOPBUTTON_OFF);
		}
		break;
	}

}

void Edit_Sample::KeyDown()
{
	Editor_KeyDown();
}

void Edit_Sample::KeyDownRepeat()
{
	Editor_KeyDown();
}

void Edit_Sample::MouseClickInOverview_Ex(bool leftmouse)
{
	// Sample Mode
}

void Edit_Sample::ShowOverviewPositions_Ex()
{
	// Sample Mode
}

void Edit_Sample::ShowWave()
{
	patternvolumecurve=0;

	if((!samples) || (!timeline))
		return;

	guiBitmap *bitmap=&samples->gbitmap;

	bitmap->guiFillRect(COLOUR_UNUSED);

	if(songmode==true)
	{
		if(pattern->GetPatternStart()<=endposition && pattern->GetPatternEnd()>=startposition)
		{
			AudioGFX gfx;

			gfx.audiopattern=pattern;

			gfx.patternvolumecurve=patternvolumecurve=pattern->GetVolumeCurve();
			gfx.patternvolumepositions=&patternvolumepositions;
			gfx.showvolumecurve=showvolume;

			gfx.win=this;
			gfx.bitmap=bitmap;
			gfx.usebitmap=true;
			gfx.timeline=timeline;
			gfx.x2=bitmap->GetX2();
			gfx.y=maingui->GetFontSizeY()/2;
			gfx.y2=bitmap->GetY2()-maingui->GetFontSizeY()/2;
			gfx.samplezoom=datazoom;

			OSTART pstart=pattern->GetPatternStart(); //af->GetPatternStart();

			gfx.start=pstart<startposition?startposition:pstart;

			if(bitmap->GetY2()<audiohdfile->channels*14)
				gfx.showmix=true;
			else
				gfx.showmix=false;

			gfx.eventstart=pstart;
			gfx.subpattern=false;
			gfx.mouseselection=false;
			gfx.drawborder=false;

			gfx.x=timeline->ConvertTimeToX(gfx.start);
			gfx.drawcolour=COLOUR_SAMPLESPAINT;

			gfx.eventend=pattern->GetPatternEnd(); //pattern->GetPatternEnd()+offset;
			gfx.samplex2=gfx.eventend>endposition?bitmap->GetX2():timeline->ConvertTimeToX(gfx.eventend);

			gfx.dontclearbackground=true;

			bitmap->guiFillRect(gfx.x,gfx.y,gfx.samplex2,gfx.y2,COLOUR_SAMPLEBACKGROUND);

			timeline->DrawPositionRaster(bitmap);

			// Audio GFX
			if(audiohdfile->peakbuffer && gfx.y2>gfx.y+8){

				//gfx.showregionsinside=af->audioevent.audioefile->showregionsineditors;

				// Delete Regions Objects
				//gfx.regions.DeleteAllO();
				//regions.DeleteAllO();
				audiohdfile->ShowAudioFile(WindowSong(),&gfx);

				//	if(gfx.regions.GetRoot())
				//		gfx.regions.MoveListToList(&regions); // Move GFX Regions->Edit_Arrange_Pattern
			}

			// Fade In/Out Volume
			if(patternvolumecurve)
			{
				fadeinms=patternvolumecurve->fadeinms;
				fadeoutms=patternvolumecurve->fadeoutms;
				volume=patternvolumecurve->dbvolume; 
				fadeactive=patternvolumecurve->fadeinoutactive;
				volumeactive=patternvolumecurve->volumeactive;
				fadeintype=patternvolumecurve->fadeintype;
				fadeouttype=patternvolumecurve->fadeouttype;
				fadeeditmode=patternvolumecurve->editmode;
			}
		}
		else
			timeline->DrawPositionRaster(bitmap);

	}
	else
	{
		AudioGFX gfx;

		gfx.win=this;
		gfx.bitmap=bitmap;
		gfx.usebitmap=true;
		gfx.timeline=0;
		gfx.x2=bitmap->GetX2();
		gfx.y=maingui->GetFontSizeY()/2;
		gfx.y2=bitmap->GetY2()-maingui->GetFontSizeY()/2;

		if(bitmap->GetY2()<audiohdfile->channels*14)
			gfx.showmix=true;
		else
			gfx.showmix=false;

		gfx.subpattern=false;
		gfx.mouseselection=false;
		gfx.drawborder=false;

		gfx.x=0;
		gfx.drawcolour=COLOUR_SAMPLESPAINT;
		gfx.samplex2=bitmap->GetX2();

		gfx.samplesperpixel=mainaudio->ConvertInternToExternSampleRate(zoom->dticksperpixel);

		//gfx.dontclearbackground=true;

		//bitmap->guiFillRect(gfx.x,gfx.y,gfx.samplex2,gfx.y2,COLOUR_SAMPLEBACKGROUND);

		//timeline->DrawPositionRaster(bitmap);

		// Audio GFX
		if(audiohdfile->peakbuffer && gfx.y2>gfx.y+8){

			//gfx.showregionsinside=af->audioevent.audioefile->showregionsineditors;

			// Delete Regions Objects
			//gfx.regions.DeleteAllO();
			//regions.DeleteAllO();
			audiohdfile->ShowAudioFile(&gfx);

			//	if(gfx.regions.GetRoot())
			//		gfx.regions.MoveListToList(&regions); // Move GFX Regions->Edit_Arrange_Pattern
		}
	}

	//ShowClipWave();
}

void Edit_Sample::StopPlayback()
{
	if(audiorealtime){
		mainaudioreal->StopAudioRealtime(audiorealtime,&endstatus_realtime);
		audiorealtime=0;
	}

	playmode=PLAYREGION_MODE_STOPPED;
}

void Edit_Sample::SetWindowName()
{
	if(!audiohdfile)
		return;

	if(windowname)
	{
		delete windowname;
		windowname=0;
	}

	char *h1="Sample";
	char *h2=0; // song

	if(WindowSong())
		h2=WindowSong()->CreateWindowTitle();

	if(h1 && h2)
	{
		windowname=mainvar->GenerateString(h1," ",h2," /",audiohdfile->GetFileName());
	}
	else
	{
		windowname=mainvar->GenerateString(h1," /",audiohdfile->GetFileName());
	}

	if(h2)
		delete h2;

}

Edit_Sample::Edit_Sample()
{
	editorid=EDITORTYPE_SAMPLE;

	InitForms(FORM_HORZ2x1BAR_SLIDERHV);
	EditForm(0,1,CHILD_HASWINDOW);

	editorname="Sample";

	audiorealtime=0;
	audiohdfile=0;
	//activeregion=0;
	samplestartperc=0;
	samplesperpixel=MAXSAMPLEZOOM/2; // 50% zoom
	//regionsgadget=0;
	displaymenu=0;

	clipshowoverview=false;

	clipoverview=false;
	clipstart=-1;
	clipend=-1;
	testregion=0;
	windowtimeformat=WINDOWDISPLAY_SAMPLES;

	samplemouseposition=-1;
	samplestartposition=0;

	frealtimepos=-1;
	mousetime=-1;

	status=playmode=PLAYREGION_MODE_STOPPED;

	showscale=mainsettings->showsamplescale;
	showregions=mainsettings->showsampleregions;
	showvolume=mainsettings->showsamplevolume;

	pattern=0;

	overviewostartx=0;
	samples=0;
	patternvolumecurve=0;

	clipx=-1;
	clipx2=-1;
}

/*
Object *Edit_Sample::GetDragDrop(HWND wnd,int index)
{
	if(regionsgadget && regionsgadget->hWnd==wnd)
	{
		return audiohdfile->GetRegionIndex(index);
	}

	return 0;
}
*/

/*
void Edit_Sample::SelectRegion(AudioRegion *r)
{
	if(r && r!=activeregion){
		activeregion=r;

		clipstart=r->regionstart;
		clipend=r->regionend;

		ShowWave();
		ShowOverview();
		ShowRegionName();
	}
}
*/

void Edit_Sample::NewZoom()
{
	RefreshStartPosition();
}

void Edit_Sample::RefreshStartPosition()
{
	DrawHeader();

	ShowCycleAndPositions(samples);
	DrawDBBlit(samples); ///*showlist==true?waveraster:*/0);
	ShowOverviewCycleAndPositions();
	DrawDBSpriteBlit(overview);
}

void Edit_Sample::AutoScroll()
{
	DoAutoScroll();
	//DoStandardYScroll(pattern);
}

// Buttons,Slider ...
void Edit_Sample::Gadget(guiGadget *gadget)
{	
	if(!Editor_Gadget(gadget))
		return;

	bool shownew=false;

	switch(gadget->gadgetID)
	{
	case GADGETID_VOLUMECURVE:
		{
			showvolume=showvolume==true?false:true;
			mainsettings->showsamplevolume=showvolume;
			DrawDBBlit(editarea);
		}
		break;

	case GADGETID_REGIONS:
		showregions=showregions==true?false:true;
		mainsettings->showsampleregions=showregions;
		FormEnable(0,1,showregions);
		break;

	case GADGETID_SCALE:
		showscale=showscale==true?false:true;
		mainsettings->showsamplescale=showscale;
		DrawDBBlit(editarea);
		break;

	case GADGETID_SONGSAMPLES:
		{
			songmode=songmode==true?false:true;

			if(timeline)
			{
				//timeline->SetSongMode(songmode);
				NewTimeFormat();
				DrawDBBlit(overview);
				songsamplemode->ChangeButtonText(songmode==true?"Song":"Samples");
			}
		}
		break;

		/*
	case REGIONCOPY_ID:
		if(activeregion){
			mainbuffer->CreateAudioBuffer(audiohdfile,activeregion);
		}
		break;
*/

	case GADGETID_REGIONPLAY:
		{
			menu_testregion play(this,PLAYREGION_MODE_FULL);
			play.MenuFunction();
		}
		break;

	case GADGETID_REGIONPLAYEND:
		{
			menu_testregion play(this,PLAYREGION_MODE_END);
			play.MenuFunction();
		}
		break;

	case GADGETID_REGIONPLAYSTART:
		{
			menu_testregion play(this,PLAYREGION_MODE_START);
			play.MenuFunction();
		}
		break;

	case GADGETID_PLAY:
		{
			menu_testregion play(this,PLAYREGION_MODE_SAMPLEPOSITION);
			play.MenuFunction();
		}
		break;

	case GADGETID_PLAYCLIP:
		{
			menu_testregion play(this,PLAYREGION_MODE_CLIP);
			play.MenuFunction();
		}
		break;

	case GADGETID_SEEKPLAY:
		{
			menu_testregion play(this,PLAYREGION_MODE_SAMPLEPOSITIONSEEK);
			play.MenuFunction();
		}
		break;

	case GADGETID_STOP:
		{
			if(audiorealtime)
				StopPlayback();
			else
				SetStartPosition(0); // reset startposition
		}
		break;

		/*
	case REGIONS_ID:
		{
			AudioRegion *r=audiohdfile->GetRegionIndex(gadget->index);
			SelectRegion(r);
		}
		break;

	case REGIONNAME_ID:
		if(activeregion){
			activeregion->SetName(gadget->string,true,this);
		}
		break;
*/

	case GADGETID_EDITORSLIDER_HORIZZOOM:
		{
			if(MAXSAMPLEZOOM-gadget->GetPos()!=samplesperpixel){
				samplesperpixel=MAXSAMPLEZOOM-gadget->GetPos();
				shownew=true;
			}
		}
		break;

	case GADGETID_EDITORSLIDER_HORIZ:
		{
			double h=gadget->GetPos();

			h/=1000;

			if(h!=samplestartperc){
				if(h<0)
					h=0;
				else
					if(h>1)
						h=1;

				samplestartperc=h; // 0 - 1.0
				shownew=true;
			}
		}
		break;
	}

	if(shownew==true && audiohdfile){

		ShowWave();
		ShowClipOverview();
	}
}

void Edit_Sample::DeleteRegion(AudioRegion *region)
{
	if(audiohdfile && region){
		bool ok=true;

		/*
		if(activeregion->GetUsedCounter()>0)
			ok=maingui->MessageBoxYesNo(this,Cxs[CXS_REGIONUSEBYPATTERN_Q]);
*/

		if(ok==true){

		//	mainedit->RemoveAudioRegionFromProjects(activeregion);

		
			audiohdfile->DeleteRegion(region);
			maingui->RemoveAudioRegionFromGUI(audiohdfile,region); // activeregion=dead pointer !
			//maingui->RefreshRegionGUI(audiohdfile);
		}
	}
}

void Edit_Sample::UserMessage(int msgid)
{
	switch(msgid)
	{
	case MESSAGE_REFRESHSAMPLEEDITOR: // sample stop or played till end
		audiorealtime=0;
		break;
	}
}

void Edit_Sample::CutRange()
{
	if(audiohdfile && clipstart!=1 && clipend!=-1){
		bool doit=true;

		if(audiohdfile->FindRegionInside(clipstart,clipend))
		{
			doit=maingui->MessageBoxYesNo(0,Cxs[CXS_REGIONAFFECTED_Q]);
		}

		if(doit==true && audioworkthread->CheckIfWorkPossible(audiohdfile,AudioFileWork::AWORK_CUT)==true)
		{
			if(AudioFileWork_CutRange *work=new AudioFileWork_CutRange)
			{
				work->Init(audiohdfile->GetName());

				work->samplestart=clipstart;
				work->sampleend=clipend;
				work->crossover=0;

				audioworkthread->AddWork(work);

				/*
				work.Start();

				if(work.ok==true)
				{
				mainaudio->ChangeAudioFileToFile(audiohdfile->GetName(),work.creatednewfile);
				}
				*/
			}
		}
	}
}

void Edit_Sample::FillRange_Zero()
{
	if(audiohdfile && clipstart!=1 && clipend!=-1){

		if(audioworkthread->CheckIfWorkPossible(audiohdfile,AudioFileWork::AWORK_FILLZERO)==true){
			if(AudioFileWork_FillZero *work=new AudioFileWork_FillZero){
				work->Init(audiohdfile->GetName());

				work->samplestart=clipstart;
				work->sampleend=clipend;

				audioworkthread->AddWork(work);
			}
		}
	}
}

void Edit_Sample::CreateNewFileFromRegion()
{
	if(audiohdfile && clipstart!=-1 && clipend!=-1)
	{
		AudioRegion tmp(audiohdfile);

		tmp.regionstart=clipstart;
		tmp.regionend=clipend;

		mainaudio->CreateAudioRegionFile(this,audiohdfile,&tmp);

		/*
		int i=32+strlen(audiohdfile->GetFileName());
		char *rname=new char[i];

		if(rname)
		{
		strcpy(rname,audiohdfile->GetFileName());
		mainvar->AddString(rname,"_region");

		camxFile write;
		if(write.OpenFileRequester("Export Region as","Wave (*wav)|*wav|All Files (*.*)|*.*||",false,rname)==true)
		{
		write.AddToFileName(".wav");

		if(write.filereqname){
		char *newfile=new char[strlen(write.filereqname)+1];

		if(newfile){
		strcpy(newfile,write.filereqname);

		AudioFileWork_CreateNewFile *work=new AudioFileWork_CreateNewFile; // From

		if(work){
		work->Init(audiohdfile->GetName()); // org file

		work->creatednewfile=newfile;
		work->samplestart=clipstart;
		work->sampleend=clipend;

		audioworkthread->AddWork(work);

		// work.CreateNewFile(write.filereqname,clipstart,clipend); // To
		}
		}
		}
		}

		delete rname;
		}
		*/
	}
}

void Edit_Sample::RefreshHorzSlider()
{
	if(horz_slider)
	{
		horz_slider->ChangeSlider((int)(SAMPLE_SLIDER_RESOLUTION*samplestartperc));
	}
}


void SampleEditor_Overview_Callback(guiGadget_CW *g,int status)
{
	Edit_Sample *p=(Edit_Sample *)g->from;

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

void SampleEditor_SamplesCallback(guiGadget_CW *g,int status)
{
	Edit_Sample *p=(Edit_Sample *)g->from;

	switch(status)
	{
	case DB_CREATE:
		p->samples=g;
		break;

	case DB_PAINT:
		{
			p->ShowWave();
			p->ShowCycleAndPositions(g);

			//p->ShowOverviewCycleAndPositions();
		}
		break;

	case DB_LEFTMOUSEDOWN:
		p->MouseClickInSamples(true);
		break;

	case DB_LEFTMOUSEUP:
		p->MouseReleaseInSamples(true);
		break;

	case DB_DOUBLECLICKLEFT:
		p->MouseDoubleClickInSamples(true);
		break;

	case DB_MOUSEMOVE:
	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		p->MouseMoveInSamples(true);	
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		// p->MouseClickInOverview(false);	
		break;
	}
}

void Edit_Sample::ShowPatternPositions()
{
	if(!pattern)
		return;

	if(info_time)
	{
		info_time->SetTime(pattern->GetPatternStart());
	}

	if(info_end)
		info_end->SetTime(pattern->GetPatternEnd());
}

void Edit_Sample::NewDataZoom()
{
	DrawDBBlit(editarea);
}

void Edit_Sample::InitGadgets()
{
	if(songmode==true)
		followsongposition=true;

	glist.SelectForm(0,0);
	guitoolbox.CreateToolBox(TOOLBOXTYPE_EDITOR,(!pattern)?guiToolBox::CTB_NOOVERVIEWVERT|guiToolBox::CTB_NOSPP:guiToolBox::CTB_NOOVERVIEWVERT);
	glist.Return();

	int startstopw=3*maingui->GetFontSizeY();

	glist.AddButton(-1,-1,startstopw*3,-1,"Regions",GADGETID_REGIONS,showregions==true?MODE_GROUP|MODE_TEXTCENTER|MODE_AUTOTOGGLE|MODE_TOGGLE|MODE_TOGGLED:MODE_GROUP|MODE_TEXTCENTER|MODE_AUTOTOGGLE|MODE_TOGGLE,Cxs[CXS_DBSCALE]);
	glist.AddLX();
	FormEnable(0,1,showregions);

	if(pattern)
	{
		songsamplemode=glist.AddButton(-1,-1,bitmap.GetTextWidth("- Samples -"),-1,songmode==true?"Song":"Samples",GADGETID_SONGSAMPLES,MODE_TEXTCENTER|MODE_TOGGLE|MODE_TOGGLED,Cxs[CXS_SONGORSAMPLES]);
		glist.AddLX();
	}
	else
		songsamplemode=0;

	scale=glist.AddButton(-1,-1,startstopw,-1,"dB",GADGETID_SCALE,showscale==true?MODE_TEXTCENTER|MODE_AUTOTOGGLE|MODE_TOGGLE|MODE_TOGGLED:MODE_TEXTCENTER|MODE_AUTOTOGGLE|MODE_TOGGLE,Cxs[CXS_DBSCALE]);
	glist.AddLX();

	g_volumecurves=glist.AddButton(-1,-1,2*startstopw,-1,"Volume",GADGETID_VOLUMECURVE,showvolume==true?MODE_TOGGLE|MODE_AUTOTOGGLE|MODE_TOGGLED:MODE_AUTOTOGGLE|MODE_TOGGLE,Cxs[CXS_INFOVOLUME]);
	glist.AddLX();


	if(pattern)
	{
		int iw=bitmap.GetTextWidth("WWWw");

		glist.AddButton(-1,-1,iw,-1,"S",GADGETID_INFOTIME_I,MODE_TEXTCENTER|MODE_ADDDPOINT);
		glist.AddLX();
		info_time=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGETID_INFOTIME,WINDOWDISPLAY_MEASURE,MODE_INFO,"Song Position (Pattern)");
		glist.AddLX();

		glist.AddButton(-1,-1,iw,-1,"E",GADGETID_INFOEND_I,MODE_TEXTCENTER|MODE_ADDDPOINT);
		glist.AddLX();

		info_end=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGETID_INFOEND,windowtimeformat,MODE_INFO,"Song End Position (Pattern)");
		glist.AddLX();

		ShowPatternPositions();
	}
	else
	{
		info_time=info_end=0;
		songsamplemode=0;
	}

	glist.Return();

	// Stop Button
	stop=glist.AddImageButton(-1,-1,(int)startstopw,-1,IMAGE_STOPBUTTON_OFF,GADGETID_STOP,0,Cxs[CXS_STOPAUDIOFILEPLAYBACK]);
	glist.AddLX();

	// Play
	start=glist.AddImageButton(-1,-1,(int)startstopw,-1,IMAGE_PLAYBUTTON_OFF,GADGETID_PLAY,0,Cxs[CXS_PLAYAUDIOFILECURSOR]);
	glist.AddLX();

	g_clipstart=glist.AddImageButton(-1,-1,(int)startstopw,-1,IMAGE_PLAYBUTTONCLIP_OFF,GADGETID_PLAYCLIP,0,Cxs[CXS_PLAYAUDIOFILECLIP]);
	glist.AddLX();

	if(pattern)
	{
		glist.AddButton(-1,-1,2*startstopw,-1,"Pattern",GADGETID_PATTERN,MODE_TEXTCENTER|MODE_INFO);
		glist.AddLX();
		patternname=glist.AddButton(-1,-1,8*startstopw,-1,pattern->GetName(),GADGETID_PATTERNNAME,0,"Pattern");
		glist.AddLX();
	}
	else
		patternname=0;

	glist.AddButton(-1,-1,2*startstopw,-1,Cxs[CXS_FILE],GADGETID_FILE,MODE_TEXTCENTER|MODE_INFO);
	glist.AddLX();
	filename=glist.AddButton(-1,-1,8*startstopw,-1,audiohdfile?audiohdfile->GetFileName():"-?",GADGETID_FILENAME,0,Cxs[CXS_AUDIOFILE]);

	int offsettracksy=SIZEV_OVERVIEW+SIZEV_HEADER+2*(ADDYSPACE+1);

	SliderCo horz,vert;

	horz.formx=0;
	horz.formy=2;

	vert.formx=2;
	vert.formy=1;
	vert.offsety=offsettracksy;
	vert.from=0;
	vert.to=0; // trackobjects.GetCount()-numberoftracks;
	vert.pos=0; //firstshowtracknr;

	AddEditorSlider(&horz,&vert,true);

	// List
	glist.SelectForm(0,1);
	glist.form->BindWindow(regionlisteditor=new Edit_RegionList(this));

	glist.SelectForm(1,1);
	glist.AddChildWindow(-1,-1,-1,SIZEV_OVERVIEW,MODE_RIGHT|MODE_SPRITE,0,&SampleEditor_Overview_Callback,this);
	glist.Return();

	glist.AddChildWindow(-1,-1,-1,SIZEV_HEADER,MODE_RIGHT|MODE_SPRITE,0,&Editor_Header_Callback,this);

	editarea=glist.AddChildWindow(-1,offsettracksy,-1,-2,MODE_BOTTOM|MODE_RIGHT|MODE_SPRITE,0,&SampleEditor_SamplesCallback,this);
}

bool Edit_Sample::SetStartPosition(LONGLONG s)
{
	if(audiohdfile && s>=0 && s<audiohdfile->samplesperchannel && s!=samplestartposition){

		samplestartposition=s;

		return true;
	}

	return false;
}

void Edit_Sample::MouseWheel(int delta,guiGadget *db)
{
}

void Edit_Sample::MouseDoubleClickInSamples(bool leftmouse)
{
	if(songmode==true)
	{
		int *te=0;

		if( (!te) || maingui->GetShiftKey()==true)
		{
			DoubleClickInEditArea();
			return;
		}
	}
}

void Edit_Sample::MouseMoveInSamples(bool leftmouse)
{
	if(songmode==true)
	{
		if(CheckMouseMovePosition(samples)==true)
			return;

		int t=patternvolumepositions.CheckXY(samples->GetMouseX(),samples->GetMouseY());

		if(t!=-1)
		{
			if(patternvolumepositions.SetMouse(this,t)==true)
				return;
		}
	}

	InitMousePosition();

	switch(mousemode)
	{
	case EM_SELECTRANGE_MOVE:
		{
			LONGLONG s=GetMouseSamplePosition(pattern);

			if(s!=selectstart)
			{
				LONGLONG diff=s-selectstart;
				selectstart+=diff;

				LONGLONG cliprange=clipend-clipstart;

				LONGLONG ncps=clipstart+diff;
				LONGLONG ncpe=clipend+diff;

				if(ncps<0)
				{
					ncps=0;
					ncpe=cliprange;
				}
				else
					if(ncpe>=audiohdfile->samplesperchannel)
					{
						ncpe=audiohdfile->samplesperchannel-1;
						ncps=ncpe-cliprange;
					}

					ClipRange(ncps,ncpe);
			}
		}
		break;

	case EM_SELECTRANGE_LEFT:
		{
			LONGLONG s=GetMouseSamplePosition(pattern);

			if(s!=-1)ClipRange(clipstart=s,clipend);
			SetMouseCursor(CURSOR_LEFT);

			TRACE("MOVE EM_SELECTRANGE_LEFT %d\n",s);
		}
		break;

	case EM_SELECTRANGE_RIGHT:
		{
			LONGLONG s=GetMouseSamplePosition(pattern);

			if(s!=-1)ClipRange(clipstart,clipend=s);

			SetMouseCursor(CURSOR_RIGHT);
			TRACE("MOVE EM_SELECTRANGE_RIGHT %d\n",s);
		}
		break;

	case EM_SELECTRANGE:
		{
			LONGLONG s=GetMouseSamplePosition(pattern);
			if(s!=-1)ClipRange(selectstart,s);
			SetMouseCursor(CURSOR_LEFTRIGHT);
			TRACE("MOVE EM_SELECTRANGE %d <> %d\n",selectstart,s);
		}
		break;

	default:
		{
			// Check Region

			int mx=samples->GetMouseX();

			if(clipx!=-1 && clipx-4<=mx && clipx+4>=mx){
				SetMouseCursor(CURSOR_LEFT);
				TRACE("Mouse Over Clip X");
			}

			if(clipx2!=-1 && clipx2-4<=mx && clipx2+4>=mx){
				SetMouseCursor(CURSOR_RIGHT);
				TRACE("Mouse Over Clip X2");
			}

		}
		break;
	}
}

void Edit_Sample::MouseReleaseInSamples(bool leftmouse)
{
	switch(mousemode)
	{

	}

	ResetMouseMode();
}

void Edit_Sample::MouseClickInSamples(bool leftmouse)
{
	if(leftmouse==false)
	{
		//if(EditCancel()==true)
		//	return;
	}
	else
	{
		if(songmode==true)
		{
			if(CheckMouseClickInEditArea(samples)==true) // Left Mouse
			{
				return;
			}
		}

		InitMousePosition();
		int mx=samples->GetMouseX();
		LONGLONG h=GetMouseSamplePosition(pattern);

		if(h!=-1){

			// Change Region Range ?
			if(clipx!=-1 && clipx<=mx+8 && clipx>=mx-8){

				// ClipStart ?

				oldclipstart=clipstart;
				oldclipend=clipend;

				SetEditorMode(EM_SELECTRANGE_LEFT);
				SetMouseCursor(CURSOR_LEFT);
				SetAutoScroll(0,samples);

				TRACE ("SetMouseMode %d\n",EM_SELECTRANGE_LEFT);
				return;
			}

			if(clipx2!=-1 && clipx2<=(mx+8) && clipx2>=(mx-8))
			{
				// ClipEnd ?

				oldclipstart=clipstart;
				oldclipend=clipend;

				SetEditorMode(EM_SELECTRANGE_RIGHT);
				SetMouseCursor(CURSOR_RIGHT);
				SetAutoScroll(0,samples);

				TRACE ("SetMouseMode %d\n",EM_SELECTRANGE_RIGHT);
				return;
			}

			if(clipx!=-1 && clipx2!=-1)
			{
				if(mx>=clipx && mx<=clipx2)
				{
					selectstart=h;
					SetEditorMode(EM_SELECTRANGE_MOVE);
					SetMouseCursor(CURSOR_LEFTRIGHT);
					SetAutoScroll(0,samples);

					return;
				}
			}

			if( (clipx!=-1 && clipx2==-1 && mx>clipx) ||
				(clipx2!=-1 && clipx==-1 && mx<clipx2)
				)
			{
				selectstart=h;
				SetEditorMode(EM_SELECTRANGE_MOVE);
				SetMouseCursor(CURSOR_LEFTRIGHT);
				SetAutoScroll(0,samples);

				return;
			}

			selectstart=h;
			SetEditorMode(EM_SELECTRANGE);
			SetAutoScroll(0,samples);

			TRACE ("SetMouseMode %d\n",EM_SELECTRANGE);
		}
	}
}

void Edit_Sample::MouseClickInsideOverview(int mx,int my)
{
	// MessageBeep(-1);
	LONGLONG otime=ConvertXPositionToOverviewSample(mx);

	if(otime!=-1)
	{
		double per=otime;
		per/=audiohdfile->samplesperchannel;

		if(per!=samplestartperc){

			samplestartperc=per; // 0 - 1.0

			ShowWave();
			ShowClipOverview();
			RefreshHorzSlider();
		}
	}
}

void Edit_Sample::MouseButton(int flag)
{
#ifdef OLDIE
	if(right_mousekey!=MOUSEKEY_UP)
	{
		switch(mousemode)
		{
		case EM_SELECTRANGE_LEFT:
		case EM_SELECTRANGE_RIGHT:
			{
				clipstart=oldclipstart;
				clipend=oldclipend;
				ClipRange(clipstart,clipend);
				SetEditorMode(EM_RESET);
			}
			break;

		case EM_SELECTRANGE:
			SetEditorMode(EM_RESET);
			break;

		default:
			{
				DeletePopUpMenu(true);

				if(popmenu)
				{
					popmenu->AddFMenu(Cxs[CXS_CREATEREGION],new menu_createregion(this));

					if(activeregion)
						popmenu->AddFMenu(Cxs[CXS_DELETEREGION],new menu_deleteregion(this));

					popmenu->AddLine();
					popmenu->AddFMenu(Cxs[CXS_PLAYMP],new menu_testregion(this,PLAYREGION_MODE_MOUSEPOSITION));

					if(samplemouseposition!=1 && samplestartposition<samplemouseposition)
						popmenu->AddFMenu(Cxs[CXS_PLAYSTOPMP],new menu_testregion(this,PLAYREGION_MODE_STOPATMOUSEPOSITION));

					if(clipstart!=-1 && clipend!=-1)
						popmenu->AddFMenu(Cxs[CXS_PLAYREGIONSE],new menu_testregion(this,PLAYREGION_MODE_FULL));

					ShowPopMenu();
				}
			}
			break;
		}
	}
	else
		switch(left_mousekey)
	{
		case MOUSEKEY_UP:
			SetEditorMode(EM_RESET);
			break;

		case MOUSEKEY_DOWN:
			{
				SetEditorMode(EM_RESET);

				if(frameoverview.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
				{
					MouseClickInsideOverview(GetMouseX(),GetMouseY());
				}
				else
					if(frameheader.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
					{
						// Set Start Position
						LONGLONG h=ConvertXPositionToSample(GetMouseX());

						if(h!=1){
							if(SetStartPosition(h)==true){
								if(audiorealtime && playmode==PLAYREGION_MODE_SAMPLEPOSITION)
								{
									StopPlayback();
									menu_testregion play(this,PLAYREGION_MODE_SAMPLEPOSITION);
									play.MenuFunction();
								}
							}
						}
					}
					else
						if (framesample.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
						{
							LONGLONG h=ConvertXPositionToSample(GetMouseX());

							if(h!=-1){
								bool changemode=true;

								// Change Region Range ?
								if(clipx!=-1) // ClipStart ?
								{
									if(clipx<=GetMouseX()+8 && clipx>=GetMouseX()-8){
										oldclipstart=clipstart;
										oldclipend=clipend;

										SetEditorMode(EM_SELECTRANGE_LEFT);
										SetMouseCursor(CURSOR_LEFT);
										TRACE ("SetMouseMode %d\n",EM_SELECTRANGE_LEFT);
										changemode=false;
									}
								}

								if(clipx2!=-1) // ClipStart ?
								{
									if((clipx2<=(GetMouseX()+8)) && (clipx2>=(GetMouseX()-8))){
										oldclipstart=clipstart;
										oldclipend=clipend;

										SetEditorMode(EM_SELECTRANGE_RIGHT);
										SetMouseCursor(CURSOR_RIGHT);
										TRACE ("SetMouseMode %d\n",EM_SELECTRANGE_RIGHT);
										changemode=false;
									}
								}

								if(changemode==true){
									selectstart=h;
									SetEditorMode(EM_SELECTRANGE);
									TRACE ("SetMouseMode %d\n",EM_SELECTRANGE);
								}
							}
						}
			}
			break;
	}
#endif

}

void Edit_Sample::ResetGadgets()
{
	horz_slider=0;
//	regionsgadget=0;

	left_samples=right_samples=0;

	start=
		stop=
		samplepos=
		regionplayseek=
	//	regionname_gadget=
		regionplay=
		regionplayend=
		regionplaystart=
		time=
		horzslider=
		horzzoomslider=
		left_smpte=
		right_smpte=0;
}

void Edit_Sample::Normalize()
{
	if(audiohdfile && audiohdfile->peakbuffer){
		if(audiohdfile->peakbuffer->maxpeakfound==0){
			maingui->MessageBoxOk(this,"No Samples Values>0 found in File !");
		}
		else
			if(audiohdfile->peakbuffer->maxpeakfound==1){
				maingui->MessageBoxOk(this,"Maximum Sample Value found in File, no Normalize possible");
			}
			else{
				if(maingui->MessageBoxYesNo(this,"Normalize possible, Normalize file ?")==true){

					if(AudioFileWork_Normalize *work=new AudioFileWork_Normalize){
						work->maxpeak=audiohdfile->peakbuffer->maxpeakfound;
						work->Init(audiohdfile->GetName()); // org file
						audiohdfile->peakbuffer->maxpeakfound=1; // avoid double work call
						audioworkthread->AddWork(work);
					}
				}
			}
	}
	else
		maingui->MessageBoxOk(this,"No File or Peakbuffer");
}

void Edit_Sample::ChangeHDFile(AudioHDFile *n)
{
	if(n && n!=audiohdfile){
		StopPlayback();

		if(audiohdfile){
			if(audiohdfile->samplesperchannel!=n->samplesperchannel){
				clipstart=-1;
				clipend=-1;
			}

			// audiohdfile->Close();
		}
		else{
			clipstart=-1;
			clipend=-1;
		}

		audiohdfile=n;
		audiohdfile->Open();
	//	activeregion=0;
		samplestartposition=0;
		/*
		winmode|=WINDOWMODE_INIT;
		Init();
		winmode CLEARBIT WINDOWMODE_INIT;

		RedrawOSGadgets();
		*/
	}
}

void Edit_Sample::ShowPeakProgress(double per)
{
#ifdef OLDIE

	if(guibuffer && framesample.ondisplay==true)
	{
		char h2[16];

		if(char *h=mainvar->GenerateString(Cxs[CXS_PEAKPROGRESS],":",mainvar->ConvertDoubleToChar(per,h2,2),"%"))
		{
			guibuffer->guiFillRect(framesample.x,framesample.y,framesample.x2,framesample.y2,COLOUR_YELLOW);

			if(framesample.GetHeight()>maingui->GetFontSizeY()+4)
				guibuffer->guiDrawText(framesample.x,framesample.y2-1,framesample.x2,h);

			BltGUIBuffer_Frame(&framesample);
			delete h;
		}
	}
#endif

}

void Edit_Sample::Init()
{
	if(pattern)
		patternselection.AddPattern(pattern);

	InitGadgets();
}

void Edit_Sample::InitNewTimeType()
{
	ShowPatternPositions();
}