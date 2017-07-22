#include "pluginwindow.h"
#include "audioobjects.h"
#include "languagefiles.h"
#include "songmain.h"
#include "audioeffects.h"
#include "object_track.h"
#include "audiochannel.h"
#include "audiohardware.h"
#include "object_song.h"
#include "gui.h"

enum{
	TAB_INDEX,
	TAB_NAME,
	TAB_VALUE,
	TAB_VALUEINFO,
	TAB_SLIDER,

	TAB_TABS
};

void PluginWindow::ShowPlugInProgram()
{
	if(programtext)
		programtext->ChangeButtonText(insertaudioeffect->audioeffect->GetProgramName());

	if(programuser)
		programuser->SetPos(insertaudioeffect->audioeffect->GetProgram()+1);

	if(programselect)
	{
		char h1[NUMBERSTRINGLEN],h2[NUMBERSTRINGLEN];

		char *h=mainvar->GenerateString(":",mainvar->ConvertIntToChar(insertaudioeffect->audioeffect->GetProgram()+1,h1),"/",mainvar->ConvertIntToChar(insertaudioeffect->audioeffect->numberofprograms,h2));

		if(h)
		{
			programselect->ChangeButtonText(h);
			delete h;
		}
	}
}

void PluginWindow::InitPluginEditor()
{
	SetName(false);

	glist.SelectForm(0,0);

	
	int w=bitmap.GetTextWidth("EDITOR")+2*maingui->GetFontSizeY();

	bypass=glist.AddImageButton(-1,-1,pluginbypasswidth=2*maingui->GetFontSizeY(),2*maingui->GetFontSizeY(),IMAGE_BYPASS_OFF,ID_BYPASS,0,"ByPass");


		//glist.AddCheckBox(-1,-1,w,-1,ID_BYPASS,0,"Bypass");

	ShowBypass();

	glist.AddLX();

	if(insertaudioeffect->audioeffect->GetOwnEditor()==true)
	{
		g_mode_editor=glist.AddButton(-1,-1,w,-1,"Editor",ID_EDITOR,MODE_TOGGLE|MODE_TOGGLED);
		mode=PIN_EDITOR;
		glist.AddLX();
	}
	else
	{
		g_mode_editor=0;
		mode=PIN_TAB;
	}

	g_mode_tab=glist.AddButton(-1,-1,w,-1,"Tab",ID_TAB,mode==PIN_TAB?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);

	if(insertaudioeffect->audioeffect->iovolume==true)
	{
		glist.AddLX();

		involume=glist.AddVolumeButtonText(-1,-1,w,-1,"In:",ID_INVOLUME,insertaudioeffect->audioeffect->involume);
		glist.AddLX();
		outvolume=glist.AddVolumeButtonText(-1,-1,w,-1,"Out:",ID_OUTVOLUME,insertaudioeffect->audioeffect->outvolume);
	}
	else
		involume=outvolume=0;

	glist.AddLX();
	outputs=glist.AddButton(-1,-1,w,-1,"Outs",ID_OUTS,0,Cxs[CXS_SEPARATEPLUGINOUTS]);
	glist.AddLX();

	g_MIDIfilter=glist.AddButton(-1,-1,2*w,-1,0,ID_MFILTER,MODE_TEXTCENTER,"Plugin MIDI Input Event Filter");
	ShowMIDIFilter();
	glist.AddLX();

	onoff=glist.AddCheckBox(-1,-1,w,-1,ID_ONOFF,0,Cxs[CXS_ON],Cxs[CXS_PLUGINONOFF]);
	ShowOnOff();

	glist.Return();

	glist.AddLX(pluginbypasswidth);

	automate=glist.AddButton(-1,-1,w,-1,"Automate",ID_AUTOMATE,MODE_TOGGLE,Cxs[CXS_PLUGINAUTOMATE]);

	glist.AddLX();

	if(insertaudioeffect->audioeffect->numberofprograms>1)
	{
		programuser=glist.AddNumberButton(-1,-1,w,-1,ID_PROGRAMUSER,1,insertaudioeffect->audioeffect->numberofprograms,1,NUMBER_INTEGER);
		glist.AddLX();
		programtext=glist.AddText(-1,-1,4*w,-1,0,ID_PROGRAMTEXT,0);
		glist.AddLX();
		programselect=glist.AddButton(-1,-1,w,-1,0,ID_PROGRAMSELECT,0,"Program/Requester");
		glist.AddLX();

		ShowPlugInProgram();
	}
	else
	{
		programuser=0;
		programselect=0;
		programtext=0;
	}
}


void PluginWindow::InitFormSize()
{
	if(insertaudioeffect->audioeffect->GetOwnEditor()==true)
	{
		plugineditorwidth=0;
		plugineditorheight=0;

		insertaudioeffect->audioeffect->InitOwnEditor(&plugineditorwidth,&plugineditorheight);
	}
	else
	{
		plugineditorwidth=40*maingui->GetFontSizeY();
		plugineditorheight=30*maingui->GetFontSizeY();
	}

	GetForm(0,1)->height=plugineditorheight;
}

PluginWindow::PluginWindow()
{
	minwidth=maingui->GetButtonSizeY(30);
	minheight=maingui->GetButtonSizeY(12);
	ondesktop=true;
	listeditor=0;
	editornameisdeleteable=true;
}

void PluginWindow::ShowMode()
{
	if(g_mode_tab)
	{
		g_mode_tab->Toggle(mode==PIN_TAB?true:false);
	}

	if(g_mode_editor)
	{
		g_mode_editor->Toggle(mode==PIN_EDITOR?true:false);
	}
}

int PluginWindow::GetCountParameter()
{
	return insertaudioeffect->audioeffect->GetCountOfParameter();
}

void PluginWindow::ShowOnOff()
{
	onoff_status=insertaudioeffect->audioeffect->plugin_on;

	if(onoff)
		onoff->SetCheckBox(onoff_status);
}

void PluginWindow::ShowBypass()
{
	bypass_status=insertaudioeffect->audioeffect->plugin_bypass;

	if(bypass)
	{
		bypass->ChangeButtonImage(bypass_status==true?IMAGE_BYPASS_ON:IMAGE_BYPASS_OFF);
	}
	//	bypass->SetCheckBox(bypass_status);
}

void PluginWindow::ShowMIDIFilter()
{
	if(g_MIDIfilter)
	{
		if(insertaudioeffect->MIDIfilter.CheckFilterActive()==true)
		{
			char *h=mainvar->GenerateString("MIDI In Filter",">",Cxs[CXS_ON],"<");
			if(h)
			{
				g_MIDIfilter->ChangeButtonText(h);
				delete h;
			}
		}
		else
			g_MIDIfilter->ChangeButtonText("MIDI In Filter");
	}

	insertaudioeffect->MIDIfilter.Clone(&comparefilter);
}

void PluginWindow::PluginRefreshRealtime_Slow()
{
	ShowPlugInProgram();

	if(comparefilter.Compare(&insertaudioeffect->MIDIfilter)==false)
		ShowMIDIFilter();
}

void PluginWindow::PluginRefreshRealtime()
{
	if(onoff_status!=insertaudioeffect->audioeffect->plugin_on)
		ShowOnOff();

	if(bypass_status!=insertaudioeffect->audioeffect->plugin_bypass)
		ShowBypass();

}

guiGadget *PluginWindow::PluginGadget(guiGadget *g)
{
	if(g==g_MIDIfilter)
	{
		guiWindow *win=maingui->FindWindow(EDITORTYPE_MIDIFILTER,&insertaudioeffect->MIDIfilter,0);

		if(!win)
		{	
			mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].x=GetWinPosX()+g->x2;
			mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].y=GetWinPosY()+g->y;

			guiWindow *w=maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,WindowSong(),0,0,0,&insertaudioeffect->MIDIfilter,0);
			if(w)
			{
				w->calledfrom=this;


				char *h=mainvar->GenerateString(insertaudioeffect->audioeffect->GetEffectName(),":","MIDI Input Filter");

				if(h)
				{
				w->guiSetWindowText(h);
				delete h;
				}
			}
		}
		else
			win->WindowToFront(true);	

		return 0;
	}

	if(g==g_mode_tab)
	{
		if(mode!=PIN_TAB)
		{
			mode=PIN_TAB;
			InitPluginParameterEditor();
			ShowMode();
		}

		return 0;
	}

	if(g==g_mode_editor)
	{
		if(mode!=PIN_EDITOR)
		{
			mode=PIN_EDITOR;
			InitPluginParameterEditor();
			ShowMode();
		}

		return 0;
	}

	// bypass
	if(g==onoff)
	{
		insertaudioeffect->audioeffect->TogglePlugInOnOff();
		onoff_status=insertaudioeffect->audioeffect->plugin_on;
		return 0;
	}

	if(g==bypass)
	{
		insertaudioeffect->audioeffect->User_TogglePluginBypass();
	//	bypass_status=insertaudioeffect->audioeffect->plugin_bypass;
		return 0;
	}

	if(g==involume)
	{
		//insertaudioeffect->audioeffect-InitAutomationTime();
		insertaudioeffect->audioeffect->SetInVolume(involume->volume);
		return 0;
	}

	if(g==outvolume)
	{
		insertaudioeffect->audioeffect->SetOutVolume(outvolume->volume);
		return 0;
	}


	if(
		(programselect && programselect==g) ||
		(programtext && g==programtext)
		)
	{
		mainaudio->SelectPluginProgram(this,insertaudioeffect);
		return 0;
	}

	if(programuser && programuser==g)
	{
		insertaudioeffect->audioeffect->SetProgram(programuser->GetPos()-1);
	}

	return g;
}

// Plugin Data List 

void PL_Callback(guiGadget_CW *g,int status)
{
	Edit_PluginList *pl=(Edit_PluginList *)g->from;

	switch(status)
	{
	case DB_FREEOBJECTS:
		{
			//mix->FreeEffects();
		}
		break;

	case DB_CREATE:
		{
			pl->pllist=(guiGadget_Tab *)g;

			pl->pllist->InitTabs(TAB_TABS);

			int w=g->gbitmap.GetTextWidth("12345");

			pl->pllist->InitTabWidth(TAB_INDEX,w);
			pl->pllist->InitTabWidth(TAB_NAME,6*w);
			pl->pllist->InitTabWidth(TAB_VALUE,2*w);
			pl->pllist->InitTabWidth(TAB_VALUEINFO,2*w);

			pl->pllist->InitTabWidth(TAB_SLIDER,-1);
		}
		break;

	case DB_PAINT:
		{
#ifdef DEBUG
			if(g->formchild->enable==false)
				maingui->MessageBoxError(0,"MixerEffects_Callback");
#endif
			pl->ShowList();
		}
		break;

	case DB_DOUBLECLICKLEFT:
		//mix->MouseDoubleClickInEffects(true);	
		break;

	case DB_LEFTMOUSEDOWN:
		pl->MouseClickInList(true);	
		break;

	case DB_RIGHTMOUSEDOWN:
		pl->MouseClickInList(false);	
		break;

	case DB_LEFTMOUSEUP:
		pl->MouseReleaseInList(true);
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		pl->MouseMoveInList(true);
		break;

	case DB_DELTA:
		//mix->DeltaInEffects();
		break;
	}
}


void Edit_PluginList_Parameter::ShowParameter()
{
	AudioObject *ao=editor->editor->insertaudioeffect->audioeffect;

	bitmap->guiFillRectX0(y,bitmap->GetX2(),y2,index&1?COLOUR_GADGETBACKGROUNDSYSTEM:COLOUR_GADGETBACKGROUND);

	bitmap->SetTextColour(mouseclicked==true?COLOUR_YELLOW:COLOUR_TEXT);

	char indexstr[NUMBERSTRINGLEN],*ixstr;
	ixstr=mainvar->ConvertIntToChar(index+1,indexstr);

	bitmap->guiDrawTextFontY(editor->pllist->GetTabX(TAB_INDEX),y2,editor->pllist->GetTabX2(TAB_INDEX),ixstr);
	bitmap->guiDrawTextFontY(editor->pllist->GetTabX(TAB_NAME),y2,editor->pllist->GetTabX2(TAB_NAME),ao->GetParmName(index));
	bitmap->guiDrawTextFontY(editor->pllist->GetTabX(TAB_VALUE),y2,editor->pllist->GetTabX2(TAB_VALUE),ao->GetParmValueString(index));
	bitmap->guiDrawTextFontY(editor->pllist->GetTabX(TAB_VALUEINFO),y2,editor->pllist->GetTabX2(TAB_VALUEINFO),ao->GetParmTypeValueString(index));

	// Rect
	int rx=editor->pllist->GetTabX(TAB_SLIDER);
	int ry=y+1;
	int ry2=y2-1;
	int rx2=bitmap->GetX2()-1;

	bitmap->guiFillRect(rx,ry,rx2,ry2,COLOUR_BLACK,mouseclicked==true?COLOUR_WHITE:COLOUR_GREY);

	double h=ao->GetParm(index); // -1 <> +1

#ifdef DEBUG
	//if(h>1)
	//	maingui->MessageBoxError(0,"AO Parm >1");

	//if(h<0)
	//	maingui->MessageBoxError(0,"AO Parm <0");
#endif

	if(h>1)
		h=1;
	else
		if(h<0)
			h=0;

	int w=(rx2-rx)/2;
	int mid=rx+w;
	bitmap->guiDrawLineX(mid,ry+1,ry2-2,COLOUR_YELLOW);

	double h3=rx2-rx;
	h3*=h;

	int x=rx+1;
	int x2=x;
	x2+=(int)h3;

	if(x2<x)
	{
		bitmap->guiFillRect(x,ry+2,rx2-1,ry2-3,COLOUR_RED); // Value error ?
		error=true;
	}
	else
	{
		bitmap->guiFillRect(x,ry+2,x2,ry2-3,mouseclicked==true?COLOUR_YELLOW:COLOUR_BLUE);
		error=false;
	}

	//bitmap->guiDrawTextFontY(editor->pllist->GetTabX(TAB_VALUE),y2,editor->pllist->GetTabX2(TAB_VALUE),ao->GetP(index));
}

void Edit_PluginList::ShowVSlider()
{
	// Show Slider
	if(vertgadget)
		vertgadget->ChangeSlider(&parameterobjects,maingui->GetFontSizeY());
	//vertgadget->ChangeSliderPage(numberoftracks);
}

void Edit_PluginList::ShowList()
{
	guiobjects.RemoveOs(0);	

	if(!pllist)return;

	pllist->InitXX2();
	parameterobjects.DeleteAllO(pllist);

	int parms=editor->GetCountParameter();

	for(int i=0;i<parms;i++)
	{
		parameterobjects.AddCooObject((Object *)i,maingui->GetFontSizeY()+2,0);
	}

	parameterobjects.EndBuild();

	ShowVSlider();
	parameterobjects.InitYStartO();

	pllist->ClearTab();

	if(parameterobjects.GetShowObject()) // first track ?
	{
		while(parameterobjects.GetShowObject() && parameterobjects.GetInitY()<pllist->GetHeight())
		{
			if(Edit_PluginList_Parameter *p=new Edit_PluginList_Parameter)
			{
				p->editor=this;
				p->index=(int)(parameterobjects.GetShowObject()->object);
				p->bitmap=&pllist->gbitmap;

				guiobjects.AddTABGUIObject(0,parameterobjects.GetInitY(),pllist->GetWidth(),parameterobjects.GetInitY2(),pllist,p);
			}

			parameterobjects.NextYO();

		}// while list

		guiObject_Pref *o=pllist->FirstGUIObjectPref();
		while(o)
		{
			Edit_PluginList_Parameter *et=(Edit_PluginList_Parameter *)o->gobject;
			et->ShowParameter();
			o=o->NextGUIObjectPref();
		}

	}// if t


	parameterobjects.DrawUnUsed(pllist);
}

void Edit_PluginList::FreeEditorMemory()
{
	guiobjects.RemoveOs(0);
}

void Edit_PluginList::DeInitWindow()
{	
	FreeEditorMemory();
}

void Edit_PluginList::MouseMoveInList(bool leftmouse)
{
	if(leftmouse==true)
	{
		OSTART atime=GetAutomationTime();

		int mx=pllist->GetMouseX();
		guiObject_Pref *o=pllist->FirstGUIObjectPref();
		while(o)
		{
			Edit_PluginList_Parameter *et=(Edit_PluginList_Parameter *)o->gobject;

			if(et->mouseclicked==true)
			{
				AudioObject *ao=editor->insertaudioeffect->audioeffect;

				if(et->mmx!=mx)
				{
					double v=et->mmxpar;

					if(mx>et->mmx)
					{
						if(v==1)
							return;

						v+=0.01;

						if(v>1)
							v=1;
					}
					else
					{
						if(v==0)
							return;

						v-=0.01;
						if(v<0)
							v=0;
					}

					et->mmxpar=v;

					ao->AutomationEdit(WindowSong(),atime,et->index,v);
					et->mmx=mx;
				}

				et->ShowParameter();
				pllist->Blt(et);
			}

			o=o->NextGUIObjectPref();
		}
	}
}

void Edit_PluginList::MouseReleaseInList(bool leftmouse)
{
	guiObject_Pref *o=pllist->FirstGUIObjectPref();
	while(o)
	{
		Edit_PluginList_Parameter *et=(Edit_PluginList_Parameter *)o->gobject;

		if(et->mouseclicked==true)
		{
			et->mouseclicked=false;
			et->ShowParameter();
			pllist->Blt(et);
		}

		o=o->NextGUIObjectPref();
	}
}

void Edit_PluginList::MouseClickInList(bool leftmouse)
{
	guiObject *o=pllist->CheckObjectClicked(); // Object Under Mouse ?

	if(o)
	{
		switch(o->id)
		{
		case OBJECTID_PARAMETERLIST:
			{
				Edit_PluginList_Parameter *eat=(Edit_PluginList_Parameter *)o;
				AudioObject *ao=editor->insertaudioeffect->audioeffect;

				if(leftmouse==true && eat->error==false)
				{
					int index=pllist->GetMouseClickTabIndex();

					switch(index)
					{
					case TAB_SLIDER:
						{
							eat->mmx=pllist->GetMouseX();
							eat->mouseclicked=true;
							eat->mmxpar=ao->GetParm(eat->index);

							eat->ShowParameter();
							pllist->Blt(eat);
						}
						break;
					}
				}
				else
				{
					int index=pllist->GetMouseClickTabIndex();

					switch(index)
					{
					case TAB_SLIDER:
						{
							OSTART atime=GetAutomationTime();

							ao->AutomationEdit(WindowSong(),atime,eat->index,0.5);
							eat->ShowParameter();
							pllist->Blt(eat);
						}
						break;
					}
				}

			}
			break;
		}
	}
}


void Edit_PluginList::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_EDITORSLIDER_VERT: // Scroll V
		{
			parameterobjects.InitWithSlider(vertgadget);
			DrawDBBlit(pllist);
		}
		break;
	}
}

void Edit_PluginList::Init()
{
	SliderCo vert;

	vert.formx=1;
	vert.formy=0;
	vert.nozoom=true;

	AddEditorSlider(0,&vert);

	glist.SelectForm(0,0);
	glist.AddTab(-1,-1,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,&PL_Callback,this);
}

Edit_PluginList::Edit_PluginList(PluginWindow *w)
{
	editorid=EDITORTYPE_PLUGINLISTEDITOR;
	InitForms(FORM_HORZ2x1SLIDERV);

	editor=w;
	song=editor->WindowSong();
}

void PluginWindow::InitPlugInList()
{
	mode=PIN_TAB;

	glist.SelectForm(0,1);
	glist.form->BindWindow(listeditor=new Edit_PluginList(this));
}

void PluginWindow::CloseListEditor()
{
	if(listeditor)
	{
		maingui->CloseWindow(listeditor);
		listeditor=0;
	}
}

void PluginWindow::SetName(bool refreshmenu)
{
	if(editorname) // editornameisdeleteable=true !
		delete editorname;

	editorname=0;

	size_t i=1;

	i+=strlen(insertaudioeffect->audioeffect->GetEffectName())+1;

	if(insertaudioeffect->effectlist && (insertaudioeffect->effectlist->channel || insertaudioeffect->effectlist->track)) // Audio Channel Effect
	{
		//	i+=strlen(ieffect->effectlist->GetSong()->songname)+1;
		i+=strlen(insertaudioeffect->effectlist->channel?insertaudioeffect->effectlist->channel->name:insertaudioeffect->effectlist->track->GetName());
		i+=3;

		editorname=new char[i];

		if(editorname)
		{
			strcpy(editorname,":");
			mainvar->AddString(editorname,insertaudioeffect->audioeffect->GetEffectName());
			mainvar->AddString(editorname," ");

			//	mainvar->AddString(nwinname,ieffect->effectlist->GetSong()->songname);
			//	mainvar->AddString(nwinname,"/");
			if(insertaudioeffect->effectlist->track)
			{
				mainvar->AddString(editorname,"T:");
				mainvar->AddString(editorname,insertaudioeffect->effectlist->track->GetName());
			}
			else
			{
				mainvar->AddString(editorname,"C:");
				mainvar->AddString(editorname,insertaudioeffect->effectlist->channel->name);
			}
		}
	}

	if(refreshmenu==true)
	{
		guiSetWindowText(editorname);
	}

	/*
	if(windowname)
	{
	guiSetWindowText(nwinname);
	delete nwinname;
	}
	else
	guiSetWindowText("Plugin");
	*/
}