#include "proc_appr.h"
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

#define GADGET_TYPE APPR_PROCGADGETID_START+3
#define GADGET_BYPASS APPR_PROCGADGETID_START+4
#define GADGET_QUANTONOFF APPR_PROCGADGETID_START+5
#define GADGET_SELQUANTIZE APPR_PROCGADGETID_START+6
#define GADGET_SORT APPR_PROCGADGETID_START+7
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

enum ID_Appr
{
	ID_APPR_TRANSPOSE=1,
	ID_APPR_OCTAVE1,
	ID_APPR_OCTAVE2,
	ID_APPR_OCTAVE3,
	ID_APPR_OCTAVE4,

	ID_APPR_FIXVELO,
	ID_APPR_ADDVELO,

	ID_APPR_FIXALLVELO,
	ID_APPR_ADDALLVELO
};

void Proc_Appr::ShowOctaves()
{
	if(g_octave)
	{
		char h[128];

		strcpy(h,"Add Notes (Octave):");

		for(int i=0;i<4;i++)
		{
			char h2[16];
			mainvar->AddString(h,"[");
			mainvar->AddString(h,mainvar->ConvertIntToChar(addoctave[i],h2));
			mainvar->AddString(h,"] ");
		}

		g_octave->ChangeButtonText(h);
	}
}

void Proc_Appr::ShowTranspose()
{
	if(g_transpose)
	{
		char h[255];

		strcpy(h,"Transpose:");

		char h2[NUMBERSTRINGLEN];
		mainvar->AddString(h,mainvar->ConvertIntToChar(transpose,h2));
		g_transpose->ChangeButtonText(h);
	}
}

void Proc_Appr::ShowQuantize()
{
	if(g_quant)
	{
		char h[255];

		strcpy(h,"Quantize:");
		mainvar->AddString(h,quantstr[quantize]);
		g_quant->ChangeButtonText(h);
	}
}

void Proc_Appr::SetQuant(int q)
{
	quantize=q;

	if(win)
		ShowQuantize();
}

void Proc_Appr::ShowSize()
{
	if(g_steps)
	{
		g_steps->ClearCycle();

		for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
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

void Proc_Appr::ShowVelo()
{
	if(g_velolist)
	{
		g_velolist->ClearCycle();

		for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
		{
			char h[256];
			char h2[NUMBERSTRINGLEN];

			strcpy(h,"Velocity ");
			mainvar->AddString(h,mainvar->ConvertIntToChar(i+1,h2));
			mainvar->AddString(h,":");
			switch(velocityflag[i])
			{
			case APPR_KEEPVELO:
				mainvar->AddString(h,"Input");
				break;

			case APPR_ADDVELO:
				mainvar->AddString(h,"Add=");
				mainvar->AddString(h,mainvar->ConvertIntToChar(addvelocity[i],h2));
				break;

			case APPR_FIXVELO:
				mainvar->AddString(h,"Fix=");
				mainvar->AddString(h,mainvar->ConvertIntToChar(setvelocity[i],h2));
				break;

			case APPR_LOWESTVELO:
				mainvar->AddString(h,"Lowest");
				break;

			case APPRO_HIGHESTVELO:
				mainvar->AddString(h,"Highest");
				break;
			}

			g_velolist->AddStringToCycle(h);
			g_velolist->SetCycleSelection(act_stepvelo);
		}
	}
}

void Proc_Appr::ShowLength()
{
	if(g_length)
	{
		g_length->ClearCycle();

		for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
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

void Proc_Appr::EditDataMessage(EditData *data)
{
	switch(data->id)
	{
	case ID_APPR_ADDALLVELO:
		for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
		{
			velocityflag[i]=APPR_ADDVELO;
			addvelocity[i]=(char)data->newvalue;
		}
		ShowVelo();
		break;

	case ID_APPR_FIXALLVELO:
		for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
		{
			velocityflag[i]=APPR_FIXVELO;
			setvelocity[i]=(char)data->newvalue;
		}
		ShowVelo();
		break;

	case ID_APPR_FIXVELO:
		setvelocity[act_stepvelo]=(char)data->newvalue;
		ShowVelo();
		break;

	case ID_APPR_ADDVELO:
		addvelocity[act_stepvelo]=(char)data->newvalue;
		ShowVelo();
		break;

	case ID_APPR_OCTAVE1:
		addoctave[0]=(char)data->newvalue;
		ShowOctaves();
		break;

	case ID_APPR_OCTAVE2:
		addoctave[1]=(char)data->newvalue;
		ShowOctaves();
		break;

	case ID_APPR_OCTAVE3:
		addoctave[2]=(char)data->newvalue;
		ShowOctaves();
		break;

	case ID_APPR_OCTAVE4:
		addoctave[3]=(char)data->newvalue;
		ShowOctaves();
		break;

	case ID_APPR_TRANSPOSE:
		transpose=data->newvalue;
		ShowTranspose();
		break;
	}
}

void Proc_Appr::ChangeVelo(guiGadget *g,int step,int type)
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
		for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
		{
			velocityflag[i]=type;
			ShowVelo();	
		}
	}

	switch(type)
	{
	case APPR_FIXVELO:
		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			if(step==-1)
			{
				edit->title="Arp. Fix all Velocity";
				edit->id=ID_APPR_FIXALLVELO;
				edit->value=127;
			}
			else
			{
				edit->title="Arp. Fix Velocity";
				edit->id=ID_APPR_FIXVELO;
				edit->value=setvelocity[act_stepvelo];
			}

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=1;
			edit->to=127;

			maingui->EditDataValue(edit);
		}
		break;

	case APPR_ADDVELO:

		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			if(step==-1)
			{
				edit->title="Arp. Add to all Velocity";

				edit->id=ID_APPR_ADDALLVELO;
				edit->value=0;
			}
			else
			{
				edit->title="Arp. Add to Velocity";
				edit->id=ID_APPR_ADDVELO;
				edit->value=addvelocity[act_stepvelo];
			}

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=-127;
			edit->to=127;

			maingui->EditDataValue(edit);
		}
		break;
	}
}

guiGadget *Proc_Appr::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_VELOALL_EDIT:
		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_velotype:public guiMenu
			{
			public:
				menu_velotype(Proc_Appr *p,guiGadget *g,int t)
				{
					module=p;
					gadget=g;
					type=t;
				}

				void MenuFunction()
				{
					module->ChangeVelo(gadget,-1,type);
				} //

				Proc_Appr *module;
				guiGadget *gadget;
				int type;
			};

			win->popmenu->AddFMenu("Type:Use Velocity of Input Note",new menu_velotype(this,g_editallvelo,APPR_KEEPVELO));
			win->popmenu->AddFMenu("Type:Fix Velocity",new menu_velotype(this,g_editallvelo,APPR_FIXVELO));
			win->popmenu->AddFMenu("Type:Add to Velocity",new menu_velotype(this,g_editallvelo,APPR_ADDVELO));
			win->popmenu->AddFMenu("Type:Use lowest Velocity of all Notes",new menu_velotype(this,g_editallvelo,APPR_LOWESTVELO));
			win->popmenu->AddFMenu("Type:Use highest Velocity of all Notes",new menu_velotype(this,g_editallvelo,APPRO_HIGHESTVELO));

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
				menu_velotype(Proc_Appr *p,guiGadget *g,int s,int t)
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

				Proc_Appr *module;
				guiGadget *gadget;
				int step;
				int type;
			};
			win->popmenu->AddFMenu("Type:Use Velocity of Input Note",new menu_velotype(this,g_editvelo,act_stepvelo,APPR_KEEPVELO),velocityflag[act_stepvelo]==APPR_KEEPVELO?true:false);
			win->popmenu->AddFMenu("Type:Fix Velocity",new menu_velotype(this,g_editvelo,act_stepvelo,APPR_FIXVELO),velocityflag[act_stepvelo]==APPR_FIXVELO?true:false);
			win->popmenu->AddFMenu("Type:Add to Velocity",new menu_velotype(this,g_editvelo,act_stepvelo,APPR_ADDVELO),velocityflag[act_stepvelo]==APPR_ADDVELO?true:false);
			win->popmenu->AddFMenu("Type:Use lowest Velocity of all Notes",new menu_velotype(this,g_editvelo,act_stepvelo,APPR_LOWESTVELO),velocityflag[act_stepvelo]==APPR_LOWESTVELO?true:false);
			win->popmenu->AddFMenu("Type:Use highest Velocity of all Notes",new menu_velotype(this,g_editvelo,act_stepvelo,APPRO_HIGHESTVELO),velocityflag[act_stepvelo]==APPRO_HIGHESTVELO?true:false);
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
				menu_stepsall(Proc_Appr *p,char ns)
				{
					module=p;
					newstep=ns;
				}

				void MenuFunction()
				{
					for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
						module->stepsize[i]=newstep;

					module->ShowSize();
				} //

				Proc_Appr *module;
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
					menu_steps(Proc_Appr *p,int step,char ns)
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

					Proc_Appr *module;
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
				menu_lengthsall(Proc_Appr *p,char nl)
				{
					module=p;
					newlength=nl;
				}

				void MenuFunction()
				{
					for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
						module->steplength[i]=newlength;

					module->ShowLength();
				} //

				Proc_Appr *module;
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
					menu_stepsl(Proc_Appr *p,int step,char nl)
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

					Proc_Appr *module;
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

	case GADGET_TRANSPOSE:
		if(EditData *edit=new EditData)
		{
			edit->win=win;
			edit->x=g->x2;
			edit->y=g->y;

			edit->id=ID_APPR_TRANSPOSE;
			edit->title="Arp. Notes Transpose";

			edit->type=EditData::EDITDATA_TYPE_INTEGER;
			edit->from=-127;
			edit->to=127;
			edit->value=transpose;

			maingui->EditDataValue(edit);
		}
		break;

	case GADGET_OCTAVE:
		{
			win->DeletePopUpMenu(true);

			if(win->popmenu)
			{
				class menu_adoctave:public guiMenu
				{
				public:
					menu_adoctave(Proc_Appr *p,int oct,guiGadget *g)
					{
						module=p;
						octave=oct;
						gadget=g;
					}

					void MenuFunction()
					{
						if(EditData *edit=new EditData)
						{
							edit->win=module->win;
							edit->x=gadget->x2;
							edit->y=gadget->y;

							switch(octave)
							{
							case 0:
								edit->id=ID_APPR_OCTAVE1;
								edit->title="Arp. Add Notes Octave [1]";
								break;
							case 1:
								edit->id=ID_APPR_OCTAVE2;
								edit->title="Arp. Add Notes Octave [2]";
								break;
							case 2:
								edit->id=ID_APPR_OCTAVE3;
								edit->title="Arp. Add Notes Octave [3]";
								break;
							case 3:
								edit->id=ID_APPR_OCTAVE4;
								edit->title="Arp. Add Notes Octave [4]";
								break;
							}		

							edit->type=EditData::EDITDATA_TYPE_INTEGER;
							edit->from=-12;
							edit->to=12;
							edit->value=module->addoctave[octave];

							maingui->EditDataValue(edit);
						}
					} //

					Proc_Appr *module;
					int octave;
					guiGadget *gadget;
				};

				// Quantize
				for(int i=0;i<4;i++)
				{
					char h[256];
					char h2[NUMBERSTRINGLEN];

					if(addoctave[i]==0)
						strcpy(h,"[Off]");
					else
					{
						strcpy(h,"Add Note (Octave):");
						mainvar->AddString(h,mainvar->ConvertIntToChar(addoctave[i],h2));
					}

					win->popmenu->AddFMenu(h,new menu_adoctave(this,i,g));
				}

				win->ShowPopMenu();
			}
		}
		break;

	case GADGET_SELQUANTIZE:
		{
			win->DeletePopUpMenu(true);

			if(win->popmenu)
			{
				class menu_qt:public guiMenu
				{
				public:
					menu_qt(Proc_Appr *p,int qt)
					{
						module=p;
						quantticks=qt;
					}

					void MenuFunction()
					{
						module->SetQuant(quantticks);
					} //

					Proc_Appr *module;
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

	case GADGET_SORT:
		{
			if(sortbynotes==true)
				sortbynotes=false;
			else
				sortbynotes=true;
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

	case GADGET_TYPE:
		{
			LockO();
			type=g->index;
			UnlockO();
		}
		break;

	default:
		return g;
	}

	return 0;
}

void Proc_Appr::ShowGUI()
{
	if(g_bypass)
		g_bypass->SetCheckBox(bypass);

	ShowSize();
	ShowLength();
	ShowQuantize();
	ShowTranspose();
	ShowOctaves();
	ShowVelo();

	if(g_type)
		g_type->SetCycleSelection(type);

	if(g_sort)
		g_sort->SetCheckBox(sortbynotes);

	if(g_quantonoff)
		g_quantonoff->SetCheckBox(usequantize);
}

void Proc_Appr::Reset()
{
	type=TYPE_UPDOWNDOWNUP;

	groove=0;
	usequantize=false;
	quantize=10; // 1/8

	steps=4;
	sortbynotes=true;

	for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
	{
		steplength[i]=
			stepsize[i]=10; // 1/8

		velocityflag[i]=APPR_KEEPVELO; // keep
		addvelocity[i]=0;
		setvelocity[i]=127;
	}

	for(int i=0;i<4;i++)
		addoctave[i]=0;

	up=true;
	ringcounter=0;
	transpose=0;
	running=false;
	act_stepsize=0;
	act_steplength=0;
	act_stepvelo=0;
}

void Proc_Appr::InitModuleGUI(Edit_ProcMod *ed,int x,int y,int x2,int y2)
{
#ifdef OLDIE
	g_quant=g_transpose=g_octave=0;
	g_steps=g_length=0;
	g_quantonoff=0;
	g_velolist=0;

	if(guiGadgetList *gl=ed->gadgetlists.AddGadgetList(ed))
	{
		g_bypass=gl->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY(),GADGET_BYPASS,0,"Bypass");

		int wx2=x2-x;
		wx2/=3;
		wx2*=2;

		y+=maingui->GetFontSizeY();
		g_steps=gl->AddCycle(x,y,x+wx2,y+maingui->GetFontSizeY_Sub(),GADGET_STEPS,0,"Note Step");
		gl->AddButton(x+wx2+1,y,x2,y+maingui->GetFontSizeY(),"Edit Step",GADGET_STEPS_EDIT,0,"Edit Note Step");
		y+=maingui->GetFontSizeY()+8;
		gl->AddButton(x,y,x2,y+maingui->GetFontSizeY(),"Edit All Steps",GADGET_STEPSALL_EDIT,0,"Edit all Note Steps");
		y+=maingui->GetFontSizeY();
		g_length=gl->AddCycle(x,y,x+wx2,y+maingui->GetFontSizeY_Sub(),GADGET_STEPLENGTH,0,"Note Length");
		gl->AddButton(x+wx2+1,y,x2,y+maingui->GetFontSizeY(),"Edit Length",GADGET_STEPLENGTH_EDIT,0,"Edit Note Length");

		y+=maingui->GetFontSizeY()+8;
		gl->AddButton(x,y,x2,y+maingui->GetFontSizeY(),"Edit All Length",GADGET_LENGTHALL_EDIT,0,"Edit all Note Length");
		y+=maingui->GetFontSizeY();

		g_velolist=gl->AddCycle(x,y,x+wx2,y+maingui->GetFontSizeY_Sub(),GADGET_VELOS,0,"Note Velocity");
		g_editvelo=gl->AddButton(x+wx2+1,y,x2,y+maingui->GetFontSizeY(),"Edit Velocity",GADGET_VELO_EDIT,0,"Edit Note Velocity");
		y+=maingui->GetFontSizeY()+8;

		g_editallvelo=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY(),"Edit All Velocity",GADGET_VELOALL_EDIT,0,"Edit all Note Velocity");
		y+=maingui->GetFontSizeY();

		g_type=gl->AddCycle(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_TYPE,0,"Arpeggiator Type");
		// Type
		if(g_type)
		{
			g_type->AddStringToCycle("Type:Up");
			g_type->AddStringToCycle("Type:Down");
			g_type->AddStringToCycle("Type:UpDownDownUp");
			g_type->AddStringToCycle("Type:Random");
		}

		y+=maingui->GetFontSizeY()+8;
		g_sort=gl->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_SORT,0,"Sort Notes","Sort Notes (example:D3-C2-F4->C2-D3-F4)");

		y+=maingui->GetFontSizeY();
		g_quantonoff=gl->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_QUANTONOFF,0,"Use Quantize");
		y+=maingui->GetFontSizeY();
		g_quant=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),0,GADGET_SELQUANTIZE,0,"Select Quantize");
		y+=maingui->GetFontSizeY();
		g_transpose=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),0,GADGET_TRANSPOSE,0,"Transpose Notes");
		y+=maingui->GetFontSizeY();

		g_octave=gl->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),0,GADGET_OCTAVE,0,"Add Notes (Octaves)");
	}

	ShowGUI();
#endif

}

MIDIPlugin *Proc_Appr::CreateClone()
{
	if(Proc_Appr *pc=new Proc_Appr)
	{
		pc->type=type;
		pc->quantize=quantize;
		pc->usequantize=usequantize;
		pc->sortbynotes=sortbynotes;
		pc->transpose=transpose;

		pc->act_stepsize=act_stepsize;
		pc->act_steplength=act_steplength;
		pc->act_stepvelo=act_stepvelo;

		for(int i=0;i<4;i++)
			pc->addoctave[i]=addoctave[i];

		for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
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

void Proc_Appr::Load(camxFile *file)
{
	LoadStandards(file);

	file->ReadChunk(&type);
	file->ReadChunk(&usequantize);
	file->ReadChunk(&quantize);
	file->ReadChunk(&sortbynotes);
	file->ReadChunk(&transpose);

	for(int i=0;i<4;i++)
		file->ReadChunk(&addoctave[i]);

	for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
	{
		file->ReadChunk(&stepsize[i]);
		file->ReadChunk(&steplength[i]);

		file->ReadChunk(&velocityflag[i]);
		file->ReadChunk(&addvelocity[i]);
		file->ReadChunk(&setvelocity[i]);
	}
}

void Proc_Appr::Save(camxFile *file)
{
	SaveStandards(file);

	file->Save_Chunk(type);
	file->Save_Chunk(usequantize);
	file->Save_Chunk(quantize);
	file->Save_Chunk(sortbynotes);

	file->Save_Chunk(transpose);

	for(int i=0;i<4;i++)
		file->Save_Chunk(addoctave[i]);

	for(int i=0;i<MAXNUMBER_APPRNOTES;i++)
	{
		file->Save_Chunk(stepsize[i]);
		file->Save_Chunk(steplength[i]);

		file->Save_Chunk(velocityflag[i]);
		file->Save_Chunk(addvelocity[i]);
		file->Save_Chunk(setvelocity[i]);
	}
}

char Proc_Appr::AddVelocity(int step,char velo)
{
	switch(velocityflag[step])
	{
	case APPR_ADDVELO:
		{
		 	int h=velo;

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

	case APPR_FIXVELO:
		velo=(char)setvelocity[step];
		break;

	case APPR_LOWESTVELO:
		{
			char lowest=127;
			Proc_ApprOpenNote *on=FirstOpenNote();
			while(on)
			{
				if(on->inputvelo<lowest)
					lowest=on->inputvelo;

				on=on->NextOpenNote();
			}
			velo=lowest;
		}
		break;

		case APPRO_HIGHESTVELO:
		{
			char highest=1;
			Proc_ApprOpenNote *on=FirstOpenNote();
			while(on)
			{
				if(on->inputvelo>highest)
					highest=on->inputvelo;

				on=on->NextOpenNote();
			}
			velo=highest;
		}
		break;
	}

	return velo;
}

char Proc_Appr::AddTranspose(char key)
{
	int tkey=key;

	tkey+=transpose;

	if(tkey>127)
		tkey=127;
	else
		if(tkey<0)
			tkey=0;

	return (char)tkey;
}

OSTART Proc_Appr::GetNextTick()
{
	OSTART nexttick;

	if(FirstOpenNote())
		nexttick=quantlist[stepsize[ringcounter]];
	else
		nexttick=0;

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

void Proc_Appr::AddNote(Proc_ApprOpenNote *n)
{
	if(notes.GetCount()<MAXNUMBER_APPRNOTES)
	{
		TRACE ("Proc Appr: Add Note %d\n",n->note.key);

		Proc_ApprOpenNote *on=0;
		if(sortbynotes==true)
		{
			// Sort by Input Key
			on=FirstOpenNote();
			while(on && on->note.key<=n->note.key)
				on=on->NextOpenNote();
		}

		if(on)
		{
			TRACE ("Proc Appr: Add Note Next %d\n",on->note.key);
			notes.AddNextO(n,on);
		}
		else
		{
			TRACE ("Proc Appr: Add Note <<< End\n",n->note.key);
			notes.AddEndO(n);
		}

		// Init Counter
		if(running==false)
			switch(type)
		{
			case TYPE_UPDOWNDOWNUP:
			case TYPE_UP: // >>>
				{
					up=true;
					ringcounter=0;
				}
				break;

			case TYPE_DOWN:
				{
					up=false;
					if(notes.GetCount()>1)
						ringcounter=notes.GetCount()-1;				
					else
						ringcounter=0;
				}
				break;
		}
	}
}

Proc_ApprOpenNote *Proc_Appr::RemoveNote(Proc_ApprOpenNote *n)
{
	Proc_ApprOpenNote *rn=n->NextOpenNote();
	notes.RemoveO(n);

	if(!FirstOpenNote())running=false;

	return rn;
}

void Proc_Appr::Alarm(Proc_Alarm *procalarm)
{
	switch(procalarm->infoflag)
	{
	case APPR_AINFO_PLAYNOTE:
		if(ringcounter>=notes.GetCount())
			ringcounter=notes.GetCount()-1;

		if(ringcounter>=0)
		{
			Proc_ApprOpenNote *on=GetNoteAtIndex(ringcounter);

		//	TRACE ("RC %d %d\n",ringcounter,on);

			if(on)
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

					tnote.key=AddTranspose(tnote.key); // Add Transpose
					tnote.velocity=AddVelocity(ringcounter,tnote.velocity);

					{
						MIDIProcessor processor(procalarm->song,on->track);
						processor.EventInput_CalledByProcAlarm(procalarm,this,&tnote);
					}

					// Alarm Note Off *************
					if(Proc_Alarm *palarm_off=new Proc_Alarm(0,procalarm->song,this,quantlist[steplength[ringcounter]])){

						palarm_off->songposition=procalarm->songposition;
						palarm_off->createraw=procalarm->createraw;

						if(Proc_ApprCloseNote *noff=new Proc_ApprCloseNote)
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
							palarm_off->calledbyalarm=true;
							palarm_off->infoflag=APPR_AINFO_STOPNOTE;

							MIDIalarmprocessorproc->AddAlarm(palarm_off,0,procalarm);
						}
						else
							delete palarm_off;
					}
				}

				// Trigger Next Note<-> ?
				{
					switch(type)
					{
					case TYPE_UP: // >>>
						{
							up=true;
							ringcounter++;

							if(ringcounter>=notes.GetCount())
								ringcounter=0;
						}
						break;

					case TYPE_DOWN:
						{
							up=false;

							if(ringcounter==0)
								ringcounter=notes.GetCount()-1;				
							else
								ringcounter--;
						}
						break;

					case TYPE_UPDOWNDOWNUP:
						{
							if(up==true)
							{
								ringcounter++;
								if(ringcounter>=notes.GetCount())
								{
									up=false;
									ringcounter=notes.GetCount()-1;
									if(ringcounter>0)
										ringcounter--;
								}
							}
							else
							{
								if(ringcounter==0)
								{
									up=true;
									if(notes.GetCount()>1)
										ringcounter=1;
								}
								else
									ringcounter--;
							}
						}
						break;

					case TYPE_RND:
						{
							// Rnd List
							int i=0;
							Proc_ApprOpenNote *nextrnd=0;
							Proc_ApprOpenNote *list[MAXNUMBER_APPRNOTES];
							Proc_ApprOpenNote *f=FirstOpenNote();
							while(f)
							{
								if(f->played==false)
									list[i++]=f;

								f=f->NextOpenNote();
							}

							bool reset=false;

							if(i)
							{
								i--;
								// Rnd 0-i
								if(i>0)
								{
									int rnd=rand()%i;
									nextrnd=list[rnd];

									TRACE ("Rnd A %d = %d\n",i,rnd);
								}
								else
								{
									nextrnd=list[0];
									reset=true;
								}
							}
							else reset=true;

							if(reset==true) // Reset
							{
								Proc_ApprOpenNote *f=FirstOpenNote();
								while(f)
								{
									f->played=false;
									list[i++]=f;

									f=f->NextOpenNote();
								}

								if(i && (!nextrnd))
								{
									int rnd=rand()%i;
									nextrnd=list[rnd];
									TRACE ("Rnd B %d = %d\n",i,rnd);
								}
							}

							if(!nextrnd)
								nextrnd=FirstOpenNote();

							if(nextrnd)
								ringcounter=nextrnd->GetIndex();
							else
								ringcounter=-1;
						}
						break;
					}

				//	TRACE ("RingCounter %d Notes %d\n",ringcounter,notes.GetCount());
					if(ringcounter>=0 && bypass==false)
					{
						if(Proc_ApprOpenNote *nexton=GetNoteAtIndex(ringcounter))
						{
							OSTART nextticks=GetNextTick();

							if(Proc_Alarm *palarm_on=new Proc_Alarm(0,procalarm->song,this,nextticks)){
								
								palarm_on->createraw=procalarm->createraw;
								palarm_on->infoflag=APPR_AINFO_PLAYNOTE;
								palarm_on->calledbyalarm=procalarm->calledbyalarm;
								palarm_on->songposition=procalarm->songposition;

								MIDIalarmprocessorproc->AddAlarm(palarm_on,0,procalarm);
							}
						}
					}
				}
			}
		}
		else
			TRACE ("RC? %d\n",ringcounter);
		break;

	case APPR_AINFO_STOPNOTE:
		if(Proc_ApprCloseNote *cnote=(Proc_ApprCloseNote *)procalarm->object)
		{
			// TRACE ("Appr Alarm Note Off\n");

			MIDIProcessor processor(procalarm->song,cnote->track);
			processor.EventInput_CalledByProcAlarm(procalarm,this,&cnote->off);
		}
		break;
	}// switch info
}

void Proc_Appr::InsertEvent(Proc_AddEvent *pae,MIDIProcessor *proc)
{
	Seq_Event *pe=proc->FirstProcessorEvent();

	while(pe)
	{
		bool deleteevent=false;

		if(!(pe->flag&EVENTFLAG_ADDEDBYTHISMODULE))
		{
			switch(pe->GetStatus()) // Trigger Start ?
			{
			case NOTEON: // Trigger ON ?
				if(bypass==false)
				{
					Note *note=(Note *)pe;

					// Key Already in List ?
					Proc_ApprOpenNote *check=FirstOpenNote();
					while(check){
						if(check->inputkey==note->key)
						{
							TRACE ("Appr Note Already in List %d %d\n",note->status,note->key);
							break;
						}

						check=check->NextOpenNote();
					}

					if(!check)
					{
						if(running==false)
							ringcounter=0; // Reset

						// Add Org Note
						if(Proc_ApprOpenNote *buffernote=new Proc_ApprOpenNote(pae->track,pe->GetMIDIPattern(),note))
						{
							OSTART alarmticks=GetNextTick(); // ticks

							AddNote(buffernote); // Sort Note to Buffer C4-D4 etc...

							// Add Octave Notes
							for(int i=0;i<4;i++)
							{
								if(addoctave[i])
								{
									Proc_ApprOpenNote *octnote=new Proc_ApprOpenNote(pae->track,pe->GetMIDIPattern(),note);
									if(octnote)
									{
										octnote->octavenote=true;
										int ik=note->key;

										ik+=addoctave[i]*12;

										if(ik>127)
											octnote->note.key=127;
										else
											if(ik<0)
												octnote->note.key=0;
											else
												octnote->note.key=(UBYTE)ik;

										TRACE ("Add Octave Note\n");
										AddNote(octnote);
									}
								}
							}

							if(running==false)
							{
								running=true; // Set Start

								if(Proc_Alarm *palarm=new Proc_Alarm(proc,proc->song,this,alarmticks)) // Init Alarm
								{
									palarm->object=buffernote;
									palarm->infoflag=APPR_AINFO_PLAYNOTE;
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
					}

					deleteevent=true;
				}
				break; // Note

			case NOTEOFF: // Trigger Off End ?
				{
					NoteOff_Raw *off=(NoteOff_Raw *)pe;

					Proc_ApprOpenNote *check=FirstOpenNote();

					while(check)
					{
						if(off->key==check->inputkey)
						{
							//		TRACE ("Delete Pressed Appr Key %d\n",off->key);
							check=RemoveNote(check);
							deleteevent=true;	
						}
						else
							check=check->NextOpenNote();
					}
				}
				break;
			}
		}

		if(deleteevent==true)
			pe=proc->DeletedEventByModule(this,pe,pae->track,pe->GetMIDIPattern(),pae->triggerevent);
		else
			pe=pe->NextEvent();
	}
}