#include "songmain.h"
#include "editor.h"

#include "camxgadgets.h"
#include "editbuffer.h"
#include "object_song.h"
#include "object_track.h"
#include "imagesdefines.h"
#include "arrangeeditor.h"
#include "groove.h"

#include "gui.h"
#include "editor_monitor.h"
#include "languagefiles.h"

#include "MIDIoutdevice.h"
#include "MIDIindevice.h"
#include "MIDIhardware.h"


enum MonitorOB_ID{

	// Track
	OBJECTID_MONITORLIST=OI_LAST,
};

enum{
	GADGETID_SELECT,
	GADGETID_CLEAR,
	GADGETID_SCROLL
};

void Edit_Monitor::EditFilter()
{
	guiWindow *win=maingui->FindWindow(EDITORTYPE_MIDIFILTER,&filter,0);

	if(!win)
	{	
		mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].x=GetWinPosX()+GetWinWidth();
		mainsettings->windowpositions[EDITORTYPE_MIDIFILTER].y=GetWinPosY();

		guiWindow *w=maingui->OpenEditorStart(EDITORTYPE_MIDIFILTER,0,0,0,0,&filter,0);
		if(w)
		{
			w->calledfrom=this;
			w->guiSetWindowText("Monitor");
		}
	}
	else
		win->WindowToFront(true);	
}

void Edit_Monitor::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_SCROLL:
		{
			realtimescroll=realtimescroll==true?false:true;

			if(realtimescroll==true)
				DrawDBBlit(list);
		}
		break;

	case GADGETID_EDITORSLIDER_VERT: // Scroll V
		{
			realtimescroll=false;
			monitorobjects.InitWithSlider(vertgadget);
			DrawDBBlit(list);
		}
		break;

	case GADGETID_CLEAR:
		ClearMonitor();
		break;

	case GADGETID_SELECT:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_filter:public guiMenu
				{
				public:
					menu_filter(Edit_Monitor *r){editor=r;}
					void MenuFunction(){editor->EditFilter();} //
					Edit_Monitor *editor;
				};

				popmenu->AddFMenu("Filter",new menu_filter(this));

				popmenu->AddLine();


				class menu_AllOutPlugins:public guiMenu
				{
				public:
					menu_AllOutPlugins(Edit_Monitor *p)
					{
						editor=p;
					}

					void MenuFunction()
					{
						if(editor->showflag & SHOWALL_PLUGINS)
							editor->showflag CLEARBIT SHOWALL_PLUGINS;
						else
							editor->showflag|=SHOWALL_PLUGINS;

						editor->CreateFilterEvents();
						editor->DrawDBBlit(editor->list);
						editor->ShowFlags();

					} //

					Edit_Monitor *editor;
				};

				popmenu->AddFMenu(Cxs[CXS_SHOWALLPLUGINDEVICES],new menu_AllOutPlugins(this),showflag&SHOWALL_PLUGINS?true:false);

				class menu_AllOutDevice:public guiMenu
				{
				public:
					menu_AllOutDevice(Edit_Monitor *p)
					{
						editor=p;
					}

					void MenuFunction()
					{
						editor->singleoutdevice=0;
						editor->singleindevice=0;

						if(editor->showflag&SHOWALL_OUTDEVICES)
							editor->showflag CLEARBIT SHOWALL_OUTDEVICES;
						else
						{
							editor->showflag |=SHOWALL_OUTDEVICES;
							editor->showflag CLEARBIT SHOWSINGLE_OUTDEVICE;
						}

						editor->CreateFilterEvents();
						editor->DrawDBBlit(editor->list);
						editor->ShowFlags();
					} //

					Edit_Monitor *editor;
				};

				popmenu->AddFMenu(Cxs[CXS_ALLMIDIOutputDeviceS],new menu_AllOutDevice(this),showflag&SHOWALL_OUTDEVICES?true:false);

				class menu_OutDevice:public guiMenu
				{
				public:
					menu_OutDevice(Edit_Monitor *p,MIDIOutputDevice *out)
					{
						editor=p;
						outdevice=out;
					}

					void MenuFunction()
					{
						editor->singleoutdevice=outdevice;

						if(editor->showflag&SHOWSINGLE_OUTDEVICE)
							editor->showflag CLEARBIT SHOWSINGLE_OUTDEVICE;
						else
						{
							editor->showflag |=SHOWSINGLE_OUTDEVICE;
							editor->showflag CLEARBIT SHOWALL_OUTDEVICES;
							editor->showflag CLEARBIT SHOWALL_PLUGINS;
						}

						editor->CreateFilterEvents();
						editor->DrawDBBlit(editor->list);
						editor->ShowFlags();
					} //

					Edit_Monitor *editor;
					MIDIOutputDevice *outdevice;
				};

				{
					MIDIOutputDevice *o=mainMIDI->FirstMIDIOutputDevice();

					while(o)
					{
						bool sel;

						if((showflag&SHOWSINGLE_OUTDEVICE) && o==singleoutdevice)
							sel=true;
						else
							sel=false;

						popmenu->AddFMenu(o->name,new menu_OutDevice(this,o),sel);

						o=o->NextOutputDevice();
					}
				}
				popmenu->AddLine();

				class menu_AllInDevice:public guiMenu
				{
				public:
					menu_AllInDevice(Edit_Monitor *p)
					{
						editor=p;
					}

					void MenuFunction()
					{
						editor->singleoutdevice=0;
						editor->singleindevice=0;
						editor->showflag=SHOWALL_INDEVICES;

						editor->CreateFilterEvents();
						editor->DrawDBBlit(editor->list);
						editor->ShowFlags();
					} //

					Edit_Monitor *editor;
				};

				{
					popmenu->AddFMenu(Cxs[CXS_ALLMIDIINPUTDEVICES],new menu_AllInDevice(this),showflag&SHOWALL_INDEVICES?true:false);

					class menu_InDevice:public guiMenu
					{
					public:
						menu_InDevice(Edit_Monitor *p,MIDIInputDevice *in)
						{
							editor=p;
							indevice=in;
						}

						void MenuFunction()
						{
							editor->singleindevice=indevice;
							editor->showflag=SHOWSINGLE_INDEVICE;

							editor->CreateFilterEvents();
							editor->DrawDBBlit(editor->list);

							editor->ShowFlags();

						} //

						Edit_Monitor *editor;
						MIDIInputDevice *indevice;
					};

					MIDIInputDevice *i=mainMIDI->FirstMIDIInputDevice();

					while(i)
					{
						bool sel;

						if((showflag&SHOWSINGLE_INDEVICE) && i==singleindevice)
							sel=true;
						else
							sel=false;

						popmenu->AddFMenu(i->name,new menu_InDevice(this,i),sel);

						i=i->NextInputDevice();
					}
				}

				ShowPopMenu();
			}
		}
		break;
	}
}

void Edit_Monitor::RefreshRealtime()
{
	int added=0;

	// Plugins VST
	Seq_Song *song=mainvar->GetActiveSong();

	if(!song)return;

	if(notetype!=song->notetype)
	{
		notetype=song->notetype;
		added++;
	}

	if((showflag&SHOWSINGLE_OUTDEVICE) || (showflag&SHOWALL_OUTDEVICES) || (showflag&SHOWALL_PLUGINS))
	{
		if(outevents!=song->events_out.GetCount())
			added++;
	}
	else
		if((showflag&SHOWSINGLE_INDEVICE) || (showflag&SHOWALL_INDEVICES))
		{
			if(inevents!=song->events_in.GetCount())
				added++;
		}

		if(displaynoteoff_monitor!=mainsettings->displaynoteoff_monitor)
		{
			displaynoteoff_monitor=mainsettings->displaynoteoff_monitor;
			added++;
		}

		if(filter.Compare(&cmpfilter)==false)
		{
			filter.Clone(&cmpfilter);
			added++;
			ShowFlags();
		}

		if(added>0)
		{
			if(CreateFilterEvents()==true)
				DrawDBBlit(list);
		}

		if(g_realtimescroll)
			g_realtimescroll->Toggle(realtimescroll);

}

#ifdef OLDIE
void Edit_Monitor::InitDevices()
{
	/*
	// MIDIOut
	MIDIOutputDevice *o=mainMIDI->FirstMIDIOutputDevice();

	while(o)
	{
	if(Edit_MonitorOutDevice *emd=new Edit_MonitorOutDevice)
	{		
	emd->outdevice=o;
	emd->counter=o->monitor_eventcounter;

	outdevices.AddEndO(emd);
	}

	o=o->NextOutputDevice();
	}


	// MIDIInput
	MIDIInputDevice *i=mainMIDI->FirstMIDIInputDevice();

	while(i)
	{
	if(Edit_MonitorInDevice *emd=new Edit_MonitorInDevice)
	{		
	emd->indevice=i;
	emd->counter=i->monitor_eventcounter;

	indevices.AddEndO(emd);

	if(indevices.GetCount()>MAXMONITOREVENTS)
	indevices.RemoveO(indevices.GetRoot());
	}

	i=i->NextInputDevice();
	}

	*/

	initdevices=true;
}
#endif

Edit_MonitorList_MonitorEvent::Edit_MonitorList_MonitorEvent()
{
	id=OBJECTID_MONITORLIST;
}

void Edit_MonitorList_MonitorEvent::ShowMonitorEvent()
{
	char *h=0;

	if((editor->showflag&SHOWALL_PLUGINS) && seqevent->plugin)
	{
		h=mainvar->GenerateString("Out:",seqevent->plugin,"->");
	}
	else
		if( ((editor->showflag&SHOWALL_OUTDEVICES) || (editor->showflag&SHOWSINGLE_OUTDEVICE))  && seqevent->outdevice)
		{
			if(editor->showmonitordevicename==true)
				h=mainvar->GenerateString("Out:",seqevent->outdevice->name,">");
			else
				h=mainvar->GenerateString("Out:>");
		}

		switch(editor->showflag)
		{
		case SHOWALL_INDEVICES:
		case SHOWSINGLE_INDEVICE:
			{
				if(editor->showmonitordevicename==true)
					h=mainvar->GenerateString("In:",seqevent->indevice->name,">");
				else
					h=mainvar->GenerateString("In:<");
			}
			break;
		}

		if(h)
		{
			maingui->ConvertMIDI2String(editor->help,seqevent->data[0],seqevent->data[1],seqevent->data[2],seqevent);
			char *h2;

			if(seqevent->track)
				h2=mainvar->GenerateString(seqevent->track->GetName(),"//",h,editor->help);
			else
				h2=mainvar->GenerateString(h,editor->help);

			delete h;

			if(h2)
			{
				bitmap->SetTextColour(COLOUR_TEXTCONTROL);
				bitmap->guiFillRect(x,y,x2,y2,COLOUR_GADGETBACKGROUND);
				bitmap->guiDrawText(x,y2,x2,h2);

				delete h2;
			}
		}
}

Edit_Monitor::Edit_Monitor()
{
	editorid=EDITORTYPE_MONITOR;
	editorname="MIDI I/O Monitor";

	InitForms(FORM_HORZ1x2SLIDERVTOOLBAR);
	minwidth=minheight=maingui->GetButtonSizeY(8);
	resizeable=true;
	ondesktop=true;
	showflag=SHOWALL_OUTDEVICES|SHOWALL_PLUGINS;
	singleoutdevice=0;
	singleindevice=0;
	showmonitordevicename=mainsettings->showmonitordevicename;
	outevents=inevents=0;
	realtimescroll=true;
	hasownmenu=true;
}

bool Edit_Monitor::CreateFilterEvents()
{
	filterevents.DeleteAllO();
	LMIDIEvents *e=0;

	Seq_Song *song=mainvar->GetActiveSong();

	if(!song)return false;

	if((showflag&SHOWSINGLE_OUTDEVICE) || (showflag&SHOWALL_OUTDEVICES) || (showflag&SHOWALL_PLUGINS))
	{
		e=song->events_out.FirstEvent();
		outevents=song->events_out.GetCount();
	}
	else
		if((showflag&SHOWSINGLE_INDEVICE) || (showflag&SHOWALL_INDEVICES))
		{
			e=song->events_in.FirstEvent();
			inevents=song->events_in.GetCount();
		}

		while(e)
		{
			bool show=false;

			if(showflag&SHOWSINGLE_OUTDEVICE)
			{
				if(e->outdevice==singleoutdevice)
					show=true;
			}
			else
				if(showflag&SHOWSINGLE_INDEVICE)
				{
					if(e->indevice==singleindevice)
					{
						show=true;
					}
				}
				else
					if((showflag&SHOWALL_OUTDEVICES) || (showflag&SHOWALL_INDEVICES))
					{
						show=true;
					}

					if(show==true)
					{
						UBYTE status=e->data[0],byte2=e->data[2];

						if((status&0xF0)==NOTEOFF)
						{
							if(mainsettings->displaynoteoff_monitor==true)
							{
								status=NOTEON|(status&0x0F);
								byte2=127;
							}
							else
								show=false;
						}

						if(show==true && filter.CheckBytes(status,e->data[1],byte2)==true)
						{
							if(LMIDIEvents *ne=new LMIDIEvents)
							{
								e->Clone(ne);
								filterevents.AddEvent(ne);
							}

						}
					}

					e=e->NextEvent();
		}

		return true;
}

void Edit_Monitor::ShowFlags()
{
	if(selectgadget)
	{
		char *h=0;

		if((showflag&SHOWALL_PLUGINS) && (showflag&SHOWALL_OUTDEVICES))
		{
			if(filter.CheckFilterActive()==true)
				h=mainvar->GenerateString(Cxs[CXS_ALLMIDIOutputDeviceS],"+",Cxs[CXS_SHOWALLPLUGINDEVICES],":Filter");
			else
				h=mainvar->GenerateString(Cxs[CXS_ALLMIDIOutputDeviceS],"+",Cxs[CXS_SHOWALLPLUGINDEVICES]);
		}
		else
			if(showflag&SHOWALL_PLUGINS)
			{
				if(filter.CheckFilterActive()==true)
					h=mainvar->GenerateString(Cxs[CXS_SHOWALLPLUGINDEVICES],":Filter");
				else
					h=mainvar->GenerateString(Cxs[CXS_SHOWALLPLUGINDEVICES]);
			}
			else
				if(showflag&SHOWALL_OUTDEVICES)
				{
					if(filter.CheckFilterActive()==true)
						h=mainvar->GenerateString(Cxs[CXS_ALLMIDIOutputDeviceS],":Filter");
					else
						h=mainvar->GenerateString(Cxs[CXS_ALLMIDIOutputDeviceS]);
				}

				if(showflag&SHOWSINGLE_OUTDEVICE)
				{
					if(singleoutdevice)
					{
						if(filter.CheckFilterActive()==true)
							h=mainvar->GenerateString("MIDI Out:",singleoutdevice->name,":Filter");
						else
							h=mainvar->GenerateString("MIDI Out:",singleoutdevice->name);
					}
					else
						h=mainvar->GenerateString("MIDI Out:???");
				}

				switch(showflag)
				{

				case SHOWALL_INDEVICES:
					if(filter.CheckFilterActive()==true)
						h=mainvar->GenerateString(Cxs[CXS_ALLMIDIINPUTDEVICES],":Filter");
					else
						h=mainvar->GenerateString(Cxs[CXS_ALLMIDIINPUTDEVICES]);

					break;


				case SHOWSINGLE_INDEVICE:
					{
						if(singleindevice)
						{
							if(filter.CheckFilterActive()==true)
								h=mainvar->GenerateString("MIDI In:",singleindevice->name,":Filter");
							else
								h=mainvar->GenerateString("MIDI In:",singleindevice->name);
						}
					}
					break;
				}

				if(h)
				{
					selectgadget->ChangeButtonText(h);
					delete h;
				}
	}
}


void Edit_Monitor::ClearMonitorData()
{
	filterevents.DeleteAllO();
	outevents=inevents=0;
	DrawDBBlit(list);	
}

void Edit_Monitor::ClearMonitor()
{
	//events_in.DeleteAllO();
	//events_out.DeleteAllO();

	if(mainvar->GetActiveSong())
	{
		mainvar->GetActiveSong()->events_in.DeleteAllO();
		mainvar->GetActiveSong()->events_out.DeleteAllO();
	}

	guiWindow *w=maingui->FirstWindow();
	while(w)
	{
		switch(w->GetEditorID())
		{
		case EDITORTYPE_MONITOR:
			Edit_Monitor *ed=(Edit_Monitor *)w;
			ed->ClearMonitorData();
			break;
		}

		w=w->NextWindow();
	}
}

guiMenu *Edit_Monitor::CreateMenu()
{
	guiMenu *n;

	//ResetUndoMenu();

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		n=menu->AddMenu("MIDI Monitor",0);
		if(n)
		{
			class menu_Clear:public guiMenu
			{
			public:
				menu_Clear(Edit_Monitor *r)
				{
					editor=r;
				}

				void MenuFunction()
				{
					editor->ClearMonitor();
				} //

				Edit_Monitor *editor;
			};
			n->AddFMenu(Cxs[CXS_CLEAR],new menu_Clear(this));

			class menu_Monitor:public guiMenu
			{
				void MenuFunction()
				{
					maingui->OpenEditorStart(EDITORTYPE_MONITOR,mainvar->GetActiveSong(),0,0,0,0,0);
				}
			};
			n->AddFMenu(Cxs[CXS_OPENNEWMONITOR],new menu_Monitor);

			n->AddLine();

			class menu_ShowDN:public guiMenu
			{
			public:
				menu_ShowDN(Edit_Monitor *r)
				{
					editor=r;
				}

				void MenuFunction()
				{
					if(editor->showmonitordevicename==true)
						editor->showmonitordevicename=false;
					else
						editor->showmonitordevicename=true;

					editor->DrawDBBlit(editor->list);

					editor->menu_showmonitordevicename->menu->Select(editor->menu_showmonitordevicename->index,editor->showmonitordevicename);
				} //

				Edit_Monitor *editor;
			};

			menu_showmonitordevicename=n->AddFMenu(Cxs[CXS_SHOWDEVICENAME],new menu_ShowDN(this),showmonitordevicename);

			n->AddLine();

			class menu_filter:public guiMenu
			{
			public:
				menu_filter(Edit_Monitor *r)
				{
					editor=r;
				}

				void MenuFunction()
				{
					editor->EditFilter();
				} //

				Edit_Monitor *editor;
			};

			n->AddFMenu("Filter",new menu_filter(this));
		}
	}

	maingui->AddCascadeMenu(this,menu);
	return menu;
}


void Edit_Monitor::BuildMonitorList()
{
	if(!list)return;

	monitorobjects.DeleteAllO(list);

	LMIDIEvents *le=FirstEvent();
	while(le)
	{
		monitorobjects.AddCooObject(le,maingui->GetFontSizeY(),0);
		le=le->NextEvent();
	}

	if(realtimescroll==true)
		monitorobjects.ScrollToEnd();

	monitorobjects.EndBuild();
}

void Edit_Monitor::ShowVSlider()
{
	// Show Slider
	if(vertgadget)
		vertgadget->ChangeSlider(&monitorobjects,maingui->GetFontSizeY());
	//vertgadget->ChangeSliderPage(numberoftracks);
}

void Edit_Monitor::ShowList()
{
	guiobjects.RemoveOs(0);	

	if(!list)return;

	//if(zoomvert==true)
	//	monitorobjects.BufferYPos();

	BuildMonitorList();

	//if(zoomvert==true)
	//	monitorobjects.RecalcYPos();

	ShowVSlider();
	monitorobjects.InitYStartO();

	list->ClearTab();

	//int sizewchildopen=tracks->gbitmap.GetTextWidth("MM");

	TRACE ("Tracks Init Y %d\n",monitorobjects.GetInitY());

	if(monitorobjects.GetShowObject()) // first track ?
	{
		//guiBitmap *automationtrackbitmap=maingui->gfx.FindBitMap(IMAGE_SUBTRACK_CLOSE);

		/*
		int sizemininame=list->gbitmap.GetTextWidth("ABCDEF");
		int startnumberw=list->gbitmap.GetTextWidth("1234.");
		int w=list->gbitmap.GetTextWidth(" mR ");
		int wrt=list->gbitmap.GetTextWidth("AA Rec:Audio");
		*/
		// Create Track List
		while(monitorobjects.GetShowObject() && monitorobjects.GetInitY()<list->GetHeight())
		{
			LMIDIEvents *t=(LMIDIEvents *)monitorobjects.GetShowObject()->object;

			if(Edit_MonitorList_MonitorEvent *et=new Edit_MonitorList_MonitorEvent)
			{
				et->editor=this;
				et->seqevent=t;
				et->bitmap=&list->gbitmap;

				guiobjects.AddTABGUIObject(0,monitorobjects.GetInitY(),list->GetWidth(),monitorobjects.GetInitY2(),list,et);

			} // if et

			monitorobjects.NextYO();

		}// while list

		guiObject_Pref *o=list->FirstGUIObjectPref();
		while(o)
		{
			Edit_MonitorList_MonitorEvent *et=(Edit_MonitorList_MonitorEvent *)o->gobject;
			et->ShowMonitorEvent();
			o=o->NextGUIObjectPref();
		}

	}// if t

	monitorobjects.DrawUnUsed(list);
}

void Monitor_List_Callback(guiGadget_CW *g,int status)
{
	Edit_Monitor *tl=(Edit_Monitor *)g->from;

	switch(status)
	{
	case DB_CREATE:
		g->menuindex=0;
		tl->list=(guiGadget_Tab *)g;
		break;

	case DB_PAINT:
		{
			tl->ShowList();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		//	ar->MouseMoveInTracks(true);
		break;

	case DB_LEFTMOUSEDOWN:
		//ar->MouseClickInTracks(true);	
		break;

	case DB_LEFTMOUSEUP:
		//ar->MouseReleaseInTracks(true);	
		break;

	case DB_RIGHTMOUSEDOWN:
		//ar->MouseClickInTracks(false);	
		break;

	case DB_DOUBLECLICKLEFT:
		//ar->MouseDoubleClickInTracks(true);
		break;

	case DB_DELTA:
		//ar->DeltaInTracks();
		break;
	}
}

void Edit_Monitor::InitGadgets()
{
	SliderCo vert;

	vert.formx=1;
	vert.formy=1;
	vert.nozoom=true;

	AddEditorSlider(0,&vert);

	glist.SelectForm(0,0);
	selectgadget=glist.AddButton(-1,-1,-1,-1,GADGETID_SELECT,MODE_RIGHT|MODE_TEXTCENTER|MODE_MENU);
	glist.Return();

	int w=2*bitmap.GetTextWidth("Scroll");
	g_realtimescroll=glist.AddButton(-1,-1,w,-1,"Scroll",GADGETID_SCROLL,realtimescroll==true?MODE_TOGGLE|MODE_TOGGLED:MODE_TOGGLE);
	glist.AddLX();
	cleargadget=glist.AddButton(-1,-1,w,-1,Cxs[CXS_CLEAR],GADGETID_CLEAR,MODE_TEXTCENTER);

	glist.SelectForm(0,1);
	glist.AddTab(-1,-1,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,&Monitor_List_Callback,this);

	ShowFlags();
}

void Edit_Monitor::Init()
{
	//if(initdevices==false)
	//	InitDevices();

	InitGadgets();

	if(mainvar->GetActiveSong())
		notetype=mainvar->GetActiveSong()->notetype;
}

void Edit_Monitor::FreeEditorMemory()
{
	guiobjects.RemoveOs(0);
	filterevents.DeleteAllO();
}

void Edit_Monitor::DeInitWindow()
{	
	FreeEditorMemory();
}

bool LMIDIEvents::Compare(LMIDIEvents *to)
{
	if(data[0]==0 && to->data[0]==0)return true;

	for(int i=0;i<MAXMONITORBYTES;i++)
	{
		if(to->data[i]!=data[i])
			return false;
	}

	return true;
}

void LMIDIEvents::Clone(LMIDIEvents *to)
{
	to->FreeData();

	for(int i=0;i<MAXMONITORBYTES;i++)to->data[i]=data[i];
	for(int i=0;i<4;i++)to->data_l[i]=data_l[i];

	if(plugin)to->plugin=mainvar->GenerateString(plugin);

	to->track=track;
	to->outdevice=outdevice;
	to->indevice=indevice;
	to->datalength=datalength;
	to->data_l_length=data_l_length;
}
