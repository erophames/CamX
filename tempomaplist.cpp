#include "tempomapeditor.h"
#include "tempomaplist.h"
#include "object_song.h"
#include "gui.h"
#include "songmain.h"
#include "editfunctions.h"

#include <math.h>

enum{
	TAB_TIME,

	TAB_STATUS,
	TAB_TEMPO, // 120.xxx
	TAB_DPTEMPO, // xxx.000

	TAB_TABS
};

enum TempolOB_ID{

	// Track
	OBJECTID_TEMPOLIST=OI_LAST,
};

Edit_TempoList::Edit_TempoList(Edit_Tempo *ed)
{
	editorid=EDITORTYPE_TEMPOLIST;
	InitForms(FORM_HORZ2x1SLIDERV);

	//autovscroll=true;
	isstatic=true;
	song=ed->WindowSong();
	editor=ed;

	usemenuofwindow=ed;
}

void TempoEditor_List_Callback(guiGadget_CW *g,int status)
{
	Edit_TempoList *tl=(Edit_TempoList *)g->from;

	switch(status)
	{
	case DB_CREATE:
		{
			g->menuindex=0;
			tl->list=(guiGadget_TabStartPosition *)g;

			tl->InitTabs();			
		}
		break;

	case DB_PAINT:
		{
			tl->ShowList();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
		if(tl->list->InitDeltaY())
		{
			tl->DeltaY(tl->list);
		}
		break;

	case DB_LEFTMOUSEDOWN:
		tl->MouseClickInTempos(true);	
		break;

	case DB_LEFTMOUSEUP:
		tl->MouseReleaseInTempos(true);
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

void Edit_TempoList::InitTabs()
{
	list->InitTabs(TAB_TABS);

	list->InitTabWidth(TAB_TIME,list->GetDefaultTimeWidth()); // TIME

	list->InitTabWidth(TAB_STATUS," TEMPO **");
	list->InitTabWidth(TAB_TEMPO,"12345");

	list->InitStartPosition(TAB_TIME);

	list->InitXX2(); // X<>X2
}

void Edit_TempoList::Init()
{
	SliderCo vert;

	vert.formx=1;
	vert.formy=0;
	vert.nozoom=true;

	AddEditorSlider(0,&vert);

	glist.SelectForm(0,0);
	glist.AddTabStartPosition(-1,-1,-1,-1,MODE_RIGHT|MODE_BOTTOM,0,&TempoEditor_List_Callback,this);
}

void Edit_TempoList::StartOfNumberEdit(guiGadget *g)
{
	Seq_Tempo *t=(Seq_Tempo *)editobject;

	//if(t->IsSelected()==false)
	{
		if(maingui->GetCtrlKey()==false)
			WindowSong()->timetrack.SelectAllTempos(false);

		t->Select();
	}
}

void Edit_TempoList::EditEditorPositions(guiGadget *)
{
	OSTART maxminus=1;
	Seq_Tempo *t=editor->WindowSong()->timetrack.FirstTempo();

	t=t->NextTempo(); // Skip 1. Tempo

	while(t)
	{
		if(t->IsSelected()==true)
		{
			OSTART pos=t->GetTempoStart();
			pos+=numbereditposition_sum;

			if(pos<0)
			{
				if(maxminus==1 || -t->GetTempoStart()>maxminus)
					maxminus=-t->GetTempoStart();
			}
		}

		if(maxminus!=1)
			numbereditposition_sum=maxminus;

		t=t->NextTempo();
	}

	editsum=0;

	RefreshObjects(0,true);
	editor->RefreshObjects(0,true);
}

void Edit_TempoList::EndOfPositionEdit(guiGadget *,OSTART diff)
{
	MoveO mo;

	mo.song=WindowSong();
	mo.diff=diff;
	mo.dindex=0;

	mainedit->EditSelectedTempos(&mo);
}

void Edit_TempoList::DeltaY(guiGadget_TabStartPosition *g)
{
	switch(mousemode)
	{
	case EM_EDITTIME:
		{
			EditPositions((int)editsum);
		}
		break;

	case EM_EDIT:
		{
			switch(list->edittabx)
			{
			case TAB_TEMPO:
			case TAB_DPTEMPO:
				{
					TRACE ("Edit Sum % f\n",editsum);

					// Check Range
					Seq_Tempo *t=WindowSong()->timetrack.FirstTempo();
					while(t)
					{
						if(t->IsSelected()==true)
						{
							double v=t->tempo;

							v+=editsum;

							if(v>MAXIMUM_TEMPO)
								editsum-=v-MAXIMUM_TEMPO;
							else
								if(v<MINIMUM_TEMPO)
									editsum+=MINIMUM_TEMPO-v;
						}

						t=t->NextTempo();
					}

					RefreshObjects(0,true);
					editor->RefreshObjects(0,true);
				}
				break;
			}
		}break;

	}
}

void Edit_TempoList::MouseClickInTempos(bool leftmouse)
{
	if(IsPositionObjectClicked(list,leftmouse)==true) // Edit Time ?
	{
		list->SetEditSteps(1);
		SetEditorMode(EM_EDITTIME);
		return;
	}

	switch(editor->mousemode)
	{
	case EM_SELECT:
		{
			if(tempoobjects.Select(list,TAB_STATUS,true)==false)
				return;

			if(tempoobjects.Select(list,TAB_TEMPO)==false)
				return;
			
			if(tempoobjects.Select(list,TAB_DPTEMPO)==false)
				return;

			if(list->InitEdit(&tempoobjects,0))
			{
				// Edit
				switch(list->edittabx)
				{
				case TAB_TEMPO:
					{
						list->SetEditSteps(1);
						SetEditorMode(EM_EDIT);
					}
					break;

				case TAB_DPTEMPO:
					{
						list->SetEditSteps(0.001);
						SetEditorMode(EM_EDIT);
					}
					break;
				}
			}

		}
		break;
	}
}

void Edit_TempoList::MouseReleaseInTempos(bool leftmouse)
{
	if(IsEndOfPositionEditing(list)==true)
		return;

	switch(mousemode)
	{
	case EM_EDIT:
		{
			MoveO mo;

			mo.song=WindowSong();
			mo.diff=0;
			mo.dindex=editsum;

			list->EndEdit();

			mainedit->EditSelectedTempos(&mo);
		}
		break;
	}

	list->EndEdit();
	ResetMouseMode();
}

void Edit_TempoList::BuildTempoList()
{
	if(!list)return;

	tempoobjects.DeleteAllO(list);

	Seq_Tempo *t=WindowSong()->timetrack.FirstTempo();

	while(t)
	{
		if(t->type!=TEMPOEVENT_VIRTUAL)
		tempoobjects.AddCooObject(t,DEFAULTLISTY,0);

		t=t->NextTempo();
	}

	tempoobjects.EndBuild();
}

void Edit_TempoList::ShowList()
{
	guiobjects.RemoveOs(OBJECTID_TEMPOLIST);	

	if(!list)return;

	if(zoomvert==true)
		tempoobjects.BufferYPos();

	BuildTempoList();

	if(zoomvert==true)
		tempoobjects.RecalcYPos();

	ShowVSlider();
	list->ClearTab();

	tempoobjects.InitYStartO();

	if(tempoobjects.GetShowObject()) // first track ?
	{
		while(tempoobjects.GetShowObject() && tempoobjects.GetInitY()<list->GetHeight())
		{
			Seq_Tempo *t=(Seq_Tempo *)tempoobjects.GetShowObject()->object;

			if(Edit_TempoList_Tempo *et=new Edit_TempoList_Tempo)
			{
				et->song=WindowSong();
				et->editor=this;
				et->tempo=t;
				et->index=t->GetIndex();

				et->bitmap=&list->gbitmap;
				et->timemode=editor->ConvertWindowDisplayToTimeMode();

				guiobjects.AddTABGUIObject(0,tempoobjects.GetInitY(),list->GetWidth(),tempoobjects.GetInitY2(),list,et);
			}

			tempoobjects.NextYO();
		}

		guiObject_Pref *o=list->FirstGUIObjectPref();
		while(o)
		{
			Edit_TempoList_Tempo *et=(Edit_TempoList_Tempo *)o->gobject;
			
			et->ShowTempo();
			AddNumberOList(et,list); // Init Time

			o=o->NextGUIObjectPref();
		}

	}// if t

	tempoobjects.DrawUnUsed(list);
}

void Edit_TempoList::RefreshObjects(LONGLONG type,bool editcall)
{
	DrawDBBlit(list);
}

void Edit_TempoList::FreeEditorMemory()
{
	guiobjects.RemoveOs(0);
	tempoobjects.DeleteAllO(0);
}

void Edit_TempoList::DeInitWindow()
{	
	FreeEditorMemory();
}

void Edit_TempoList::RefreshRealtime()
{
	guiObject_Pref *o=list->FirstGUIObjectPref();
	while(o)
	{
		Edit_TempoList_Tempo *et=(Edit_TempoList_Tempo *)o->gobject;

		if(et->eflag!=et->tempo->flag)
		{
			DrawDBBlit(list);
			return;
		}

		o=o->NextGUIObjectPref();
	}
}

void Edit_TempoList::RefreshRealtime_Slow()
{
		CheckNumberObjects(list);
	
}

void Edit_TempoList::ShowVSlider()
{
	// Show Slider
	if(vertgadget)
		vertgadget->ChangeSlider(&tempoobjects,maingui->GetFontSizeY());
	//vertgadget->ChangeSliderPage(numberoftracks);
}

void Edit_TempoList::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_EDITORSLIDER_VERT: // Track Scroll
		tempoobjects.InitWithSlider(vertgadget,true);
		DrawDBBlit(list);
		break;
	}
}


Edit_TempoList_Tempo::Edit_TempoList_Tempo()
{
	id=OBJECTID_TEMPOLIST;
}

void Edit_TempoList_Tempo::ShowTempo()
{
	eflag=tempo->flag;
	object=tempo;

	OSTART pos=tempo->GetTempoStart();

	if(tempo->PrevTempo() && tempo->IsSelected()==true)
	{
		pos+=editor->numbereditposition_sum; // List Editor
		pos+=editor->editor->numbereditposition_sum; // Tempo Editor
	}

	bitmap->guiFillRect(editor->list->GetTabX(TAB_TIME),y,x2,y2,GetIndexColour());
	bitmap->SetFont(tempo);
	DrawPosition(pos,editor->list->GetTabX(TAB_TIME),editor->list->GetTabX2(TAB_TIME),tempo);

	double tv=tempo->tempo;
	
	if(tempo->IsSelected()==true)
	{
		tv+=editor->editsum;
	}

	char h[NUMBERSTRINGLEN];
	char *v=mainvar->ConvertDoubleToChar(tv,h,3);
	
	char intpart[5],fpart[5];

	mainvar->SplitDoubleString(v,intpart,fpart);

	bitmap->guiDrawText(editor->list->GetTabX(TAB_STATUS),y2,editor->list->GetTabX2(TAB_STATUS),"Tempo");
	bitmap->guiDrawText(editor->list->GetTabX(TAB_TEMPO),y2,editor->list->GetTabX2(TAB_TEMPO),intpart);
	bitmap->guiDrawText(editor->list->GetTabX(TAB_DPTEMPO),y2,editor->list->GetTabX2(TAB_DPTEMPO),fpart);
}
