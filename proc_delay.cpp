#include "proc_delay.h"
#include "objectevent.h"
#include "object_track.h"
#include "object_song.h"
#include "MIDIoutproc.h"
#include "edit_processor.h"
#include "editdata.h"
#include "languagefiles.h"
#include "MIDIPattern.h"
#include "songmain.h"

#define APPR_PROCGADGETID_START GADGET_ID_START+50

#define GADGET_DELAY_EDIT APPR_PROCGADGETID_START+3

#define GADGET_BYPASS APPR_PROCGADGETID_START+4
#define GADGET_QUANTONOFF APPR_PROCGADGETID_START+5
#define GADGET_SELQUANTIZE APPR_PROCGADGETID_START+6
#define GADGET_TRANSPOSE APPR_PROCGADGETID_START+8
#define GADGET_OCTAVE APPR_PROCGADGETID_START+9
#define GADGET_STEPS APPR_PROCGADGETID_START+10
#define GADGET_STEPLENGTH APPR_PROCGADGETID_START+11

#define GADGET_STEPS_EDIT APPR_PROCGADGETID_START+12
#define GADGET_STEPLENGTH_EDIT APPR_PROCGADGETID_START+13
#define GADGET_STEPSALL_EDIT APPR_PROCGADGETID_START+14
#define GADGET_LENGTHALL_EDIT APPR_PROCGADGETID_START+15
#define GADGET_VELOS APPR_PROCGADGETID_START+16
#define GADGET_VELOALL_EDIT APPR_PROCGADGETID_START+17
#define GADGET_VELO_EDIT APPR_PROCGADGETID_START+18
#define GADGET_TYPE APPR_PROCGADGETID_START+19

#define GADGET_DOWN APPR_PROCGADGETID_START+20
#define GADGET_UP APPR_PROCGADGETID_START+21

enum DelayIDs{
	ID_DELAY_DELAYS,
	ID_DELAY_TRANSPOSE,
	ID_DELAY_OCTAVE1,
	ID_DELAY_OCTAVE2,
	ID_DELAY_OCTAVE3,
	ID_DELAY_OCTAVE4,

	ID_DELAY_FIXVELO,
	ID_DELAY_ADDVELO,
	ID_DELAY_SUBVELO,

	ID_DELAY_FIXALLVELO,
	ID_DELAY_ADDALLVELO,
	ID_DELAY_SUBALLVELO,

	ID_DELAY_DOWN,
	ID_DELAY_UP
};

void Proc_Delay::ShowQuantize()
{
	if(g_quant)
	{
		if(char *h=mainvar->GenerateString(Cxs[CXS_QUANTIZE],":",quantstr[quantize]))
		{
			g_quant->ChangeButtonText(h);
			delete h;
		}
	}
}

void Proc_Delay::SetQuant(int q)
{
	quantize=q;

	if(win)
		ShowQuantize();
}

void Proc_Delay::ShowDelays()
{
	if(g_delays)
	{
		char h[255];
		char h2[NUMBERSTRINGLEN];

		strcpy(h,"Delay Notes:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(steps,h2));

		g_delays->ChangeButtonText(h);
	}
}

void Proc_Delay::ShowType()
{
	if(g_type)
	{
		g_type->SetCycleSelection(type);
	}
}

void Proc_Delay::ShowDown()
{
	if(g_down)
	{
		char h[255];
		char h2[NUMBERSTRINGLEN];

		strcpy(h,"Velocity Down to:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(downto,h2));
		g_down->ChangeButtonText(h);
	}
}

void Proc_Delay::ShowUp()
{
	if(g_up)
	{
		char h[255];
		char h2[NUMBERSTRINGLEN];

		strcpy(h,"Velocity Up to:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(upto,h2));
		g_up->ChangeButtonText(h);
	}
}

void Proc_Delay::ShowSize()
{
	if(g_steps)
	{
		g_steps->ClearCycle();

		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			char h[256];
			char h2[NUMBERSTRINGLEN];

			strcpy(h,"Step ");
			mainvar->AddString(h,mainvar->ConvertIntToChar(i+1,h2));
			mainvar->AddString(h,":");
			mainvar->AddString(h,quantstr[stepsize[i]]);

			g_steps->AddStringToCycle(h);
			g_steps->SetCycleSelection(act_stepsize);
		}
	}
}

void Proc_Delay::ShowVelo()
{
	if(g_velolist)
	{
		g_velolist->ClearCycle();

		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			char h[256];
			char h2[NUMBERSTRINGLEN];

			strcpy(h,"Velocity ");
			mainvar->AddString(h,mainvar->ConvertIntToChar(i+1,h2));
			mainvar->AddString(h,":");

			switch(velocityflag[i])
			{
			case DELAY_KEEPVELO:
				mainvar->AddString(h,"Input");
				break;

			case DELAY_ADDVELO:
				mainvar->AddString(h,"Add=");
				mainvar->AddString(h,mainvar->ConvertIntToChar(addvelocity[i],h2));
				break;

			case DELAY_SUBVELO:
				mainvar->AddString(h,"Sub=");
				mainvar->AddString(h,mainvar->ConvertIntToChar(subvelocity[i],h2));
				break;

			case DELAY_FIXVELO:
				mainvar->AddString(h,"Fix=");
				mainvar->AddString(h,mainvar->ConvertIntToChar(setvelocity[i],h2));
				break;
			}

			g_velolist->AddStringToCycle(h);
			g_velolist->SetCycleSelection(act_stepvelo);
		}
	}
}

void Proc_Delay::ShowLength()
{
	if(g_length)
	{
		g_length->ClearCycle();

		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			char h[256];
			char h2[NUMBERSTRINGLEN];

			strcpy(h,"Length");
			mainvar->AddString(h,mainvar->ConvertIntToChar(i+1,h2));
			mainvar->AddString(h,":");
			mainvar->AddString(h,quantstr[steplength[i]]);

			g_length->AddStringToCycle(h);
			g_length->SetCycleSelection(act_steplength);
		}
	}
}

void Proc_Delay::EditDataMessage(EditData *data)
{
	switch(data->id)
	{
	case ID_DELAY_DOWN:
		downto=data->newvalue;
		ShowDown();
		break;

	case ID_DELAY_UP:
		upto=data->newvalue;
		ShowUp();
		break;

	case ID_DELAY_DELAYS:
		{
			steps=data->newvalue;
			ShowDelays();
		}
		break;

	case ID_DELAY_ADDALLVELO:
		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			velocityflag[i]=DELAY_ADDVELO;
			addvelocity[i]=(char)data->newvalue;
		}
		ShowVelo();
		break;

	case ID_DELAY_SUBALLVELO:
		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			velocityflag[i]=DELAY_SUBVELO;
			subvelocity[i]=(char)data->newvalue;
		}
		ShowVelo();
		break;

	case ID_DELAY_FIXALLVELO:
		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			velocityflag[i]=DELAY_FIXVELO;
			setvelocity[i]=(char)data->newvalue;
		}
		ShowVelo();
		break;

	case ID_DELAY_FIXVELO:
		setvelocity[act_stepvelo]=(char)data->newvalue;
		ShowVelo();
		break;

	case ID_DELAY_ADDVELO:
		addvelocity[act_stepvelo]=(char)data->newvalue;
		ShowVelo();
		break;

	case ID_DELAY_SUBVELO:
		subvelocity[act_stepvelo]=(char)data->newvalue;
		ShowVelo();
		break;
	}
}

void Proc_Delay::ChangeVelo(guiGadget *g,int step,int type)
{
	if(step>=0)
	{
		if(type!=velocityflag[step])
		{
			velocityflag[step]=type;
			ShowVelo();	
		}
	}
	else
	{
		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			velocityflag[i]=type;
			ShowVelo();	
		}
	}

	switch(type)
	{
	case DELAY_FIXVELO:
		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			if(step==-1)
			{
				edit->title="Delay Fix all Velocity";
				edit->id=ID_DELAY_FIXALLVELO;
				edit->value=127;
			}
			else
			{
				edit->title="Delay Fix Velocity";
				edit->id=ID_DELAY_FIXVELO;
				edit->value=setvelocity[act_stepvelo];
			}

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=1;
			edit->to=127;

			maingui->EditDataValue(edit);
		}
		break;

	case DELAY_ADDVELO:

		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			if(step==-1)
			{
				edit->title="Delay: Add to Input Velocity (ALL)";
				edit->id=ID_DELAY_ADDALLVELO;
				edit->value=0;
			}
			else
			{
				edit->title="Delay: Add to Input Velocity";
				edit->id=ID_DELAY_ADDVELO;
				edit->value=addvelocity[act_stepvelo];
			}

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=1;
			edit->to=127;

			maingui->EditDataValue(edit);
		}
		break;

	case DELAY_SUBVELO:

		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			if(step==-1)
			{
				edit->title="Delay: Sub from Input Velocity (ALL)";

				edit->id=ID_DELAY_SUBALLVELO;
				edit->value=0;
			}
			else
			{
				edit->title="Delay: Sub from Input Velocity";
				edit->id=ID_DELAY_SUBVELO;
				edit->value=subvelocity[act_stepvelo];
			}

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=1;
			edit->to=127;

			maingui->EditDataValue(edit);
		}
		break;
	}
}

guiGadget *Proc_Delay::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_DOWN:
		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			edit->id=ID_DELAY_DOWN;
			edit->title="Delay: Velocity Down Value";

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=1;
			edit->to=127;
			edit->value=downto;

			maingui->EditDataValue(edit);
		}
		break;

	case GADGET_UP:
		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			edit->id=ID_DELAY_UP;
			edit->title="Delay: Velocity Up Value";

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=1;
			edit->to=127;
			edit->value=upto;

			maingui->EditDataValue(edit);
		}
		break;

	case GADGET_TYPE:
		{
			type=g->index;
		}
		break;

	case GADGET_DELAY_EDIT:
		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			edit->id=ID_DELAY_DELAYS;
			edit->title="Delay: Number of Delay Notes";

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=1;
			edit->to=MAXNUMBER_DELAYNOTES;
			edit->value=steps;

			maingui->EditDataValue(edit);
		}
		break;

	case GADGET_VELOALL_EDIT:
		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_velotype:public guiMenu
			{
			public:
				menu_velotype(Proc_Delay *p,guiGadget *g,int t)
				{
					module=p;
					gadget=g;
					type=t;
				}

				void MenuFunction()
				{
					module->ChangeVelo(gadget,-1,type);
				} //

				Proc_Delay *module;
				guiGadget *gadget;
				int type;
			};

			win->popmenu->AddFMenu("Type:Use Velocity of Input Note",new menu_velotype(this,g_editallvelo,DELAY_KEEPVELO));
			win->popmenu->AddFMenu("Type:Add to Input Velocity",new menu_velotype(this,g_editallvelo,DELAY_ADDVELO));
			win->popmenu->AddFMenu("Type:Sub from Input Velocity",new menu_velotype(this,g_editallvelo,DELAY_SUBVELO));

			win->popmenu->AddLine();
			win->popmenu->AddFMenu("Type:Fix Output Velocity",new menu_velotype(this,g_editallvelo,DELAY_FIXVELO));

			win->ShowPopMenu();
		}
		break;

	case GADGET_VELO_EDIT:
		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_velotype:public guiMenu
			{
			public:
				menu_velotype(Proc_Delay *p,guiGadget *g,int s,int t)
				{
					module=p;
					gadget=g;
					step=s;
					type=t;
				}

				void MenuFunction()
				{
					module->ChangeVelo(gadget,step,type);
				} //

				Proc_Delay *module;
				guiGadget *gadget;
				int step;
				int type;
			};
			win->popmenu->AddFMenu("Type:Use Velocity of Input Note",new menu_velotype(this,g_editvelo,act_stepvelo,DELAY_KEEPVELO),velocityflag[act_stepvelo]==DELAY_KEEPVELO?true:false);

			win->popmenu->AddFMenu("Type:Add to Input Velocity",new menu_velotype(this,g_editvelo,act_stepvelo,DELAY_ADDVELO),velocityflag[act_stepvelo]==DELAY_ADDVELO?true:false);
			win->popmenu->AddFMenu("Type:Sub from Input Velocity",new menu_velotype(this,g_editvelo,act_stepvelo,DELAY_SUBVELO),velocityflag[act_stepvelo]==DELAY_ADDVELO?true:false);

			win->popmenu->AddLine();
			win->popmenu->AddFMenu("Type:Fix Velocity",new menu_velotype(this,g_editvelo,act_stepvelo,DELAY_FIXVELO),velocityflag[act_stepvelo]==DELAY_FIXVELO?true:false);

			win->popmenu->AddLine();
			win->popmenu->AddFMenu(Cxs[CXS_EDIT],new menu_velotype(this,g_editvelo,act_stepvelo,velocityflag[act_stepvelo]));
			win->ShowPopMenu();
		}
		break;

	case GADGET_VELOS:
		act_stepvelo=g->index;
		break;

	case GADGET_STEPSALL_EDIT:
		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_stepsall:public guiMenu
			{
			public:
				menu_stepsall(Proc_Delay *p,char ns)
				{
					module=p;
					newstep=ns;
				}

				void MenuFunction()
				{
					for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
						module->stepsize[i]=newstep;

					module->ShowSize();
				} //

				Proc_Delay *module;
				char newstep;
			};

			// Quantize
			for(int a=0;a<QUANTNUMBER;a++)
				win->popmenu->AddFMenu(quantstr[a],new menu_stepsall(this,a));

			win->ShowPopMenu();
		}
		break;

	case GADGET_STEPS_EDIT:
		{
			win->DeletePopUpMenu(true);

			if(win->popmenu)
			{
				class menu_steps:public guiMenu
				{
				public:
					menu_steps(Proc_Delay *p,int step,char ns)
					{
						module=p;
						setstep=step;
						newstep=ns;
					}

					void MenuFunction()
					{
						module->stepsize[setstep]=newstep;
						module->ShowSize();
					} //

					Proc_Delay *module;
					int setstep;
					char newstep;
				};

				// Quantize
				for(int a=0;a<QUANTNUMBER;a++)
				{
					bool sel;

					if(this->stepsize[act_stepsize]==a)
						sel=true;
					else
						sel=false;

					win->popmenu->AddFMenu(quantstr[a],new menu_steps(this,act_stepsize,a),sel);
				}

				win->ShowPopMenu();
			}
		}
		break;

	case GADGET_LENGTHALL_EDIT:
		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_lengthsall:public guiMenu
			{
			public:
				menu_lengthsall(Proc_Delay *p,char nl)
				{
					module=p;
					newlength=nl;
				}

				void MenuFunction()
				{
					for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
						module->steplength[i]=newlength;

					module->ShowLength();
				} //

				Proc_Delay *module;
				char newlength;
			};

			// Quantize
			for(int a=0;a<QUANTNUMBER;a++)
				win->popmenu->AddFMenu(quantstr[a],new menu_lengthsall(this,a));

			win->ShowPopMenu();
		}
		break;

	case GADGET_STEPLENGTH_EDIT:
		{
			win->DeletePopUpMenu(true);

			if(win->popmenu)
			{
				class menu_stepsl:public guiMenu
				{
				public:
					menu_stepsl(Proc_Delay *p,int step,char nl)
					{
						module=p;
						setstep=step;
						newlength=nl;
					}

					void MenuFunction()
					{
						module->steplength[setstep]=newlength;
						module->ShowLength();
					} //

					Proc_Delay *module;
					int setstep;
					char newlength;
				};

				// Quantize
				for(int a=0;a<QUANTNUMBER;a++)
				{
					bool sel;

					if(steplength[act_steplength]==a)
						sel=true;
					else
						sel=false;

					win->popmenu->AddFMenu(quantstr[a],new menu_stepsl(this,act_steplength,a),sel);
				}

				win->ShowPopMenu();
			}
		}	
		break;

	case GADGET_STEPS:
		act_stepsize=g->index;
		break;

	case GADGET_STEPLENGTH:
		act_steplength=g->index;
		break;

	case GADGET_SELQUANTIZE:
		{
			win->DeletePopUpMenu(true);

			if(win->popmenu)
			{
				class menu_qt:public guiMenu
				{
				public:
					menu_qt(Proc_Delay *p,int qt)
					{
						module=p;
						quantticks=qt;
					}

					void MenuFunction()
					{
						module->SetQuant(quantticks);
					} //

					Proc_Delay *module;
					int quantticks;
				};

				// Quantize
				for(int a=0;a<QUANTNUMBER;a++)
				{
					bool sel;

					if(quantize==a)
						sel=true;
					else
						sel=false;

					win->popmenu->AddFMenu(quantstr[a],new menu_qt(this,a),sel);
				}

				win->ShowPopMenu();
			}
		}
		break;

	case GADGET_QUANTONOFF:
		{
			if(usequantize==true)
				usequantize=false;
			else
				usequantize=true;
		}
		break;

	case GADGET_BYPASS:
		{
			if(bypass==true)
				bypass=false;
			else
				bypass=true;
		}
		break;

	default:
		return g;
	}

	return 0;
}

void Proc_Delay::ShowGUI()
{
	if(g_bypass)
		g_bypass->SetCheckBox(bypass);

	ShowType();
	ShowDown();
	ShowUp();

	ShowDelays();
	ShowSize();
	ShowLength();
	ShowQuantize();

	ShowVelo();

	if(g_quantonoff)
		g_quantonoff->SetCheckBox(usequantize);
}

void Proc_Delay::InitModuleGUI(Edit_ProcMod *ed,int x,int y,int x2,int y2)
{
#ifdef OLDIE
	g_down=0;
	g_up=0;
	g_type=0;
	g_quant=0;
	g_steps=g_length=0;
	g_quantonoff=0;
	g_velolist=0;
	g_delays=0;

	if(guiGadgetList *gl=ed->gadgetlists.AddGadgetList(ed))
	{
		g_bypass=gl->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY(),GADGET_BYPASS,0,"Bypass");

		int wx2=x2-x;
		wx2/=3;
		wx2*=2;

		y+=maingui->GetFontSizeY();

		g_type=gl->AddCycle(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_TYPE,0,"Delay Type");

		if(g_type)
		{
			g_type->AddStringToCycle("Velocity Type:Manual");
			g_type->AddStringToCycle("Velocity Type:Down");
			g_type->AddStringToCycle("Velocity Type:Up");
		}

		y+=maingui->GetFontSizeY()+8;

		g_down=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),0,GADGET_DOWN,0,"Velocity < Down Value");
		y+=maingui->GetFontSizeY();
		g_up=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),0,GADGET_UP,0,"Velocity > Up Value");
		y+=maingui->GetFontSizeY();

		g_delays=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),0,GADGET_DELAY_EDIT,0,"Number of Delay Notes");
		y+=maingui->GetFontSizeY();
		g_steps=gl->AddCycle(x,y,x+wx2,y+maingui->GetFontSizeY_Sub(),GADGET_STEPS,0,"Note Step");
		gl->AddButton(x+wx2+1,y,x2,y+maingui->GetFontSizeY(),"Edit Step",GADGET_STEPS_EDIT,0,"Edit Note Step");
		y+=maingui->GetFontSizeY()+8;
		gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),"Edit All Steps",GADGET_STEPSALL_EDIT,0,"Edit all Note Steps");
		y+=maingui->GetFontSizeY();
		g_length=gl->AddCycle(x,y,x+wx2,y+maingui->GetFontSizeY_Sub(),GADGET_STEPLENGTH,0,"Note Length");
		gl->AddButton(x+wx2+1,y,x2,y+maingui->GetFontSizeY(),"Edit Length",GADGET_STEPLENGTH_EDIT,0,"Edit Note Length");

		y+=maingui->GetFontSizeY()+8;
		gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),"Edit All Length",GADGET_LENGTHALL_EDIT,0,"Edit all Note Length");
		y+=maingui->GetFontSizeY();

		g_velolist=gl->AddCycle(x,y,x+wx2,y+maingui->GetFontSizeY_Sub(),GADGET_VELOS,0,"Note Velocity");
		g_editvelo=gl->AddButton(x+wx2+1,y,x2,y+maingui->GetFontSizeY(),"Edit Velocity",GADGET_VELO_EDIT,0,"Edit Note Velocity");
		y+=maingui->GetFontSizeY()+8;

		g_editallvelo=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),"Edit All Velocity",GADGET_VELOALL_EDIT,0,"Edit all Note Velocity");
		y+=maingui->GetFontSizeY();
		g_quantonoff=gl->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_QUANTONOFF,0,"Use Quantize");
		y+=maingui->GetFontSizeY();
		g_quant=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),0,GADGET_SELQUANTIZE,0,"Select Quantize");
	}

	ShowGUI();
#endif

}

MIDIPlugin *Proc_Delay::CreateClone()
{
	if(Proc_Delay *pc=new Proc_Delay)
	{
		pc->downto=downto;
		pc->upto=upto;
		pc->type=type;

		pc->quantize=quantize;
		pc->usequantize=usequantize;

		pc->act_stepsize=act_stepsize;
		pc->act_steplength=act_steplength;
		pc->act_stepvelo=act_stepvelo;

		for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
		{
			pc->steplength[i]=steplength[i];
			pc->stepsize[i]=stepsize[i];

			pc->velocityflag[i]=velocityflag[i];
			pc->addvelocity[i]=addvelocity[i];
			pc->setvelocity[i]=setvelocity[i];
		}

		return pc;
	}

	return 0;
}

void Proc_Delay::Load(camxFile *file)
{
	LoadStandards(file);

	file->ReadChunk(&type);
	file->ReadChunk(&usequantize);
	file->ReadChunk(&quantize);
	file->ReadChunk(&steps);
	file->ReadChunk(&act_stepsize);
	file->ReadChunk(&act_steplength);
	file->ReadChunk(&act_stepvelo);

	file->ReadChunk(&downto);
	file->ReadChunk(&upto);

	for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
	{
		file->ReadChunk(&stepsize[i]);
		file->ReadChunk(&steplength[i]);
		file->ReadChunk(&velocityflag[i]); // keep

		file->ReadChunk(&addvelocity[i]);
		file->ReadChunk(&subvelocity[i]);
		file->ReadChunk(&setvelocity[i]);
	}

}

void Proc_Delay::Save(camxFile *file)
{
	SaveStandards(file);

	file->Save_Chunk(type);
	file->Save_Chunk(usequantize);
	file->Save_Chunk(quantize);
	file->Save_Chunk(steps);
	file->Save_Chunk(act_stepsize);
	file->Save_Chunk(act_steplength);
	file->Save_Chunk(act_stepvelo);

	file->Save_Chunk(downto);
	file->Save_Chunk(upto);

	for(int i=0;i<MAXNUMBER_DELAYNOTES;i++)
	{
		file->Save_Chunk(stepsize[i]);
		file->Save_Chunk(steplength[i]);
		file->Save_Chunk(velocityflag[i]); // keep

		file->Save_Chunk(addvelocity[i]);
		file->Save_Chunk(subvelocity[i]);
		file->Save_Chunk(setvelocity[i]);
	}
}

char Proc_Delay::AddVelocity(int step,char velo)
{
	switch(type)
	{
	case DELAY_USE_UP:
		{
			if(upto>velo)
			{
				int h=upto-velo;

				h/=steps;
				h=velo+h*(step+1);

				if(h>upto)
					velo=upto;
				else
					if(h<1)
						velo=1;
					else
						velo=(char)h;

			}else
				velo=upto;
		}
		break;

	case DELAY_USE_DOWN:
		{
			if(downto<velo)
			{
				int h=velo-downto;

				h/=steps;
				h=velo-h*(step+1);

				if(h>127)
					velo=127;
				else
					if(h<downto)
						velo=downto;
					else
						velo=(char)h;

			}else
				velo=downto;
		}
		break;

	case DELAY_USE_MANUAL:
		switch(velocityflag[step])
		{
		case DELAY_ADDVELO:
			{
				int h=velo;

				for(int i=0;i<=step;i++)
					h+=addvelocity[step];

				if(h>127)
					velo=127;
				else
					if(h<1)
						velo=1;
					else
						velo=(char)h;
			}
			break;

		case DELAY_SUBVELO:
			{
				int h=velo;

				for(int i=0;i<=step;i++)
					h-=subvelocity[step];

				if(h>127)
					velo=127;
				else
					if(h<1)
						velo=1;
					else
						velo=(char)h;
			}
			break;

		case DELAY_FIXVELO:
			velo=(char)setvelocity[step];
			break;
		}
		break;

	}

	return velo;
}

OSTART Proc_Delay::GetNextTick(int step)
{
	OSTART nexttick=quantlist[stepsize[step]];

	if(usequantize==true)
	{
		if(processor->track)
		{
			Seq_Song *song=processor->track->song;

			OSTART quantticks=quantlist[quantize];
			OSTART songposition=song->GetSongPosition();
			OSTART qsp=mainvar->SimpleQuantizeRight(songposition+nexttick,quantticks);

			if(qsp<songposition)
				nexttick=0;
			else
				nexttick=qsp-songposition;	
		}
	}

	return nexttick;
}

void Proc_Delay::AddNote(Proc_DelayOpenNote *n)
{
	TRACE ("Proc Delay: Add Note %d\n",n->note.key);
	notes.AddEndO(n);
}

Proc_DelayOpenNote *Proc_Delay::RemoveNote(Proc_DelayOpenNote *n)
{
	Proc_DelayOpenNote *rn=n->NextOpenNote();
	notes.RemoveO(n);
	return rn;
}

void Proc_Delay::Alarm(Proc_Alarm *procalarm)
{
	switch(procalarm->infoflag)
	{
	case DELAY_AINFO_PLAYNOTE:
		{
			if(Proc_DelayOpenNote *on=(Proc_DelayOpenNote *)procalarm->object)
			{
				bool mute;

				if(on->pattern)
					mute=on->pattern->CheckIfPlaybackIsAble();
				else
					mute=on->track->CheckIfPlaybackIsAble();

				if(mute==true)
				{
					//TRACE ("Appr Alarm Note On\n");

					// Note On
					Note tnote;
					on->note.CloneData(0,&tnote);
					tnote.pattern=on->note.pattern;

					tnote.velocity=AddVelocity(on->counter,on->inputvelo);

					{
						MIDIProcessor processor(procalarm->song,on->track);
						processor.EventInput_CalledByProcAlarm(procalarm,this,&tnote);
					}

					// Alarm Note Off *************
					if(Proc_Alarm *palarm_off=new Proc_Alarm(0,procalarm->song,this,quantlist[steplength[on->counter]])){

						palarm_off->songposition=procalarm->songposition;
						palarm_off->createraw=procalarm->createraw;

						if(Proc_DelayCloseNote *noff=new Proc_DelayCloseNote)
						{
							tnote.CloneData(0,&noff->note);

							noff->track=on->track;

							// Init Note Off
							noff->off.status=NOTEOFF|tnote.GetChannel();
							noff->off.key=tnote.key;
							noff->off.velocityoff=0;
							noff->off.pattern=0;
							on->played=true;

							palarm_off->object=noff;
							palarm_off->deleteobject=noff;
							palarm_off->forcealarm=true; // avoid dead notes
							palarm_off->calledbyalarm=procalarm->calledbyalarm;
							palarm_off->infoflag=DELAY_AINFO_STOPNOTE;

							MIDIalarmprocessorproc->AddAlarm(palarm_off,0,procalarm);
						}
						else
							delete palarm_off;
					}
				}

				if(on->counter<steps)
				{
					OSTART nextticks=GetNextTick(on->counter);

					if(Proc_Alarm *palarm_on=new Proc_Alarm(0,procalarm->song,this,nextticks)){

						palarm_on->object=on;
						palarm_on->createraw=procalarm->createraw;
						palarm_on->infoflag=DELAY_AINFO_PLAYNOTE;
						palarm_on->calledbyalarm=procalarm->calledbyalarm;
						palarm_on->songposition=procalarm->songposition;

						MIDIalarmprocessorproc->AddAlarm(palarm_on,0,procalarm);
					}

					on->counter++;
				}
				else
					RemoveNote(on);
			}
		}
		break;

	case DELAY_AINFO_STOPNOTE:
		if(Proc_DelayCloseNote *cnote=(Proc_DelayCloseNote *)procalarm->object)
		{
			// TRACE ("Appr Alarm Note Off\n");

			MIDIProcessor processor(procalarm->song,cnote->track);
			processor.EventInput_CalledByProcAlarm(procalarm,this,&cnote->off);
		}
		break;
	}// switch info
}

void Proc_Delay::InsertEvent(Proc_AddEvent *pae,MIDIProcessor *proc)
{
	Seq_Event *pe=proc->FirstProcessorEvent();

	while(pe)
	{
		if(!(pe->flag&EVENTFLAG_ADDEDBYTHISMODULE))
		{
			switch(pe->GetStatus()) // Trigger Start ?
			{
			case NOTEON: // Trigger ON ?
				if(bypass==false)
				{
					Note *note=(Note *)pe;

					// Add Org Note
					if(Proc_DelayOpenNote *buffernote=new Proc_DelayOpenNote(pae->track,pe->GetMIDIPattern(),note))
					{
						OSTART alarmticks=GetNextTick(buffernote->counter); // ticks

						AddNote(buffernote); // Sort Note to Buffer C4-D4 etc...

						if(Proc_Alarm *palarm=new Proc_Alarm(proc,proc->song,this,alarmticks)) // Init Alarm
						{
							palarm->object=buffernote;
							palarm->infoflag=DELAY_AINFO_PLAYNOTE;
							palarm->createraw=proc->createraw;

							switch(alarmticks)
							{
							case 0: // Output now
								{
									TRACE ("Appr Note 0\n");
									MIDIalarmprocessorproc->InitAlarm(palarm,pe,0);
									Alarm(palarm);
									delete palarm;
								}
								break;

							default:
								TRACE ("Appr Future Note \n");
								MIDIalarmprocessorproc->AddAlarm(palarm,pe,0);
								break;
							}
						}
					}
				}
				break; // Note
			}
		}

		pe=pe->NextEvent();
	}
}
