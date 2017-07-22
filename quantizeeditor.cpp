#include "quantizeeditor.h"
#include "songmain.h"
#include "groove.h"
#include "objectevent.h"
#include "version.h"
#include "editfunctions.h"
#include "MIDIhardware.h"
#include "gui.h"
#include "languagefiles.h"
#include "object_track.h"
#include "editdata.h"
#include "chunks.h"
#include "objectpattern.h"
#include "midipattern.h"
#include "semapores.h"

#include <math.h>

#define QUANTIZEGADGETID_START GADGET_ID_START+50

enum GIDs
{
	GADGET_QUANTIZEFLAG=(QUANTIZEGADGETID_START+10),
	GADGET_QUANTIZEONOFF,
	GADGET_USEGROOVE,
	GADGET_SELECTQUANTIZE,
	GADGET_NOTEOFFQUANTIZE,
	GADGET_CAPTUREQUANTIZE,
	GADGET_CAPTURERANGE,
	GADGET_NOTEOFFFIXLENGTH,
	GADGET_NOTEOFFSELECTFIXLENGTH,

	GADGET_STRENGTHQUANTIZE,
	GADGET_STRENGTH,

	GADGET_HUMANQUANTIZE,
	GADGET_HUMANRANGE,

	GADGET_SELECTGROOVE,
	GADGET_RESETQUANT,
	GADGET_GROOVE,
};

#define QUANTIZEEDITOR_START MENU_ID_START+50

#define EDITDATA_CAPTURERANGE 1
#define EDITDATA_STRENGTH 2

void Edit_QuantizeEditor::RefreshEvents(bool force)
{
	if(effect->pattern)
		mainedit->QuantizePattern(effect->pattern,&edit);
	else
		if(effect->track)
			mainedit->QuantizeTrack(effect->track,&edit);
}

void Edit_QuantizeEditor::LoadSettings()
{
	camxFile sfile;

	if(sfile.OpenFileRequester(0,this,Cxs[CXS_LOADQUANTIZESETTINGS],sfile.AllFiles(camxFile::FT_QUANTIZE),true)==true)
	{
		if(sfile.OpenRead(sfile.filereqname)==true)
		{
			char check[4];

			sfile.Read(check,4);

			if(mainvar->CompareStringWithOutZero(check,"CAMX")==true)
			{
				sfile.Read(check,4);

				if(mainvar->CompareStringWithOutZero(check,"QSET")==true)
				{
					int version=0;

					sfile.Read(&version);

					edit.Load(&sfile);

					sfile.LoadChunk();

					if(sfile.GetChunkHeader()==CHUNK_GROOVE)
					{
						sfile.ChunkFound();

						edit.groove=new Groove(0,0);

						if(edit.groove)
						{
							edit.groove->Load(&sfile);
							edit.groove=mainMIDI->AddGroove(edit.groove);
						}
					}

					RefreshEvents(true);
				}
			}
		}

		sfile.Close(true);
	}
}

void Edit_QuantizeEditor::SaveSettings()
{
	camxFile sfile;

	if(sfile.OpenFileRequester(0,this,Cxs[CXS_SAVEQUANTIZESETTINGS],sfile.AllFiles(camxFile::FT_QUANTIZE),false)==true)
	{
		sfile.AddToFileName(".cxqs");

		if(sfile.OpenSave(sfile.filereqname)==true)
		{
			int version=maingui->GetVersion();

			// Header
			sfile.Save("CAMX",4);
			sfile.Save("QSET",4);
			sfile.Save(&version,sizeof(int)); // Version

			edit.Save(&sfile);

			// Add Groove ?
			if(edit.groove)
				edit.groove->Save(&sfile);
		}

		sfile.Close(true);
	}
}

guiMenu *Edit_QuantizeEditor::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		guiMenu *s=menu->AddMenu(Cxs[CXS_FILE],0);

		if(s)
		{
			class menu_loadquantizesettings:public guiMenu
			{
			public:
				menu_loadquantizesettings(Edit_QuantizeEditor *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					editor->LoadSettings();
				}

				Edit_QuantizeEditor *editor;
			};

			s->AddFMenu(Cxs[CXS_LOADQUANTIZESETTINGS],new menu_loadquantizesettings(this));

			class menu_savequantizesettings:public guiMenu
			{
			public:
				menu_savequantizesettings(Edit_QuantizeEditor *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					editor->SaveSettings();	
				}

				Edit_QuantizeEditor *editor;
			};

			s->AddFMenu(Cxs[CXS_SAVEQUANTIZESETTINGS],new menu_savequantizesettings(this));
		}
	}

	return menu;
}

void Edit_QuantizeEditor::CreateGroovePopUp(guiGadget *g,int id)
{
	if(DeletePopUpMenu(true))
	{
		class menu_gqt:public guiMenu
		{
		public:
			menu_gqt(Edit_QuantizeEditor *ed,Groove *g)
			{
				editor=ed;
				groove=g;
			}

			void MenuFunction()
			{
				if(groove!=editor->edit.groove)
				{
					editor->edit.groove=groove;

					// Refresh Events
					editor->RefreshEvents(false);

					editor->ShowQuantizeStatus();
				}
			} //

			Edit_QuantizeEditor *editor;
			Groove *groove;
			int mode;
		};

		// Groove
		Groove *groove=mainMIDI->FirstGroove();

		while(groove)
		{
			popmenu->AddFMenu(groove->GetGrooveName(),new menu_gqt(this,groove),effect->groove==groove?true:false);
			groove=groove->NextGroove();
		}

		ShowPopMenu();
	}
}

void Edit_QuantizeEditor::CreateQuantizePopUp(guiGadget *g,int flag)
{
	DeletePopUpMenu(true);

	if(popmenu)
	{
		class menu_qt:public guiMenu
		{
		public:
			menu_qt(Edit_QuantizeEditor *ed,int m,int f)
			{
				editor=ed;
				mode=m;
				flag=f;
			}

			void MenuFunction()
			{
				if(editor->edit.quantize!=mode)
				{
					if(flag==QPUFLAG_QUANTIZE)
					{
						if(editor->edit.quantize==mode)return;
						editor->edit.quantize=mode;
					}

					if(flag==QPUFLAG_NOTELENGTH)
					{
						if(editor->edit.notelength==mode)return;
						editor->edit.notelength=mode;
					}

					// Refresh Events
					editor->RefreshEvents(false);
					editor->ShowQuantizeStatus();
				}
			} //

			Edit_QuantizeEditor *editor;
			int mode;
			int flag;
		};

		// Quantize
		for(int a=0;a<QUANTNUMBER;a++)
		{
			bool sel;

			if(flag==QPUFLAG_QUANTIZE && a==effect->quantize)
				sel=true;
			else
				if(flag==QPUFLAG_NOTELENGTH && a==effect->notelength)
					sel=true;
				else
					sel=false;

			popmenu->AddFMenu(quantstr[a],new menu_qt(this,a,flag),sel);
		}

		ShowPopMenu();
	}
}

void Edit_QuantizeEditor::CreateHumanPopUp(guiGadget *g)
{
	if(DeletePopUpMenu(true))
	{
		class menu_hqt:public guiMenu
		{
		public:
			menu_hqt(Edit_QuantizeEditor *ed,int m)
			{
				editor=ed;
				mode=m;
			}

			void MenuFunction()
			{
				if(	editor->edit.humanrange!=mode)
				{
					editor->edit.humanrange=mode;
					// Refresh Events
					editor->RefreshEvents(false);
					editor->ShowQuantizeStatus();
				}
			} //

			Edit_QuantizeEditor *editor;
			int mode;
		};

		// Quantize
		for(int a=0;a<QUANTNUMBER;a++)
			popmenu->AddFMenu(quantstr[a],new menu_hqt(this,a),effect->humanrange==a?true:false);

		ShowPopMenu();
	}
}

Edit_QuantizeEditor::Edit_QuantizeEditor(QuantizeEffect *qeff)
{
	editorid=EDITORTYPE_QUANTIZEEDITOR;

	effect=qeff;

	qeff->Clone(&backup);
	qeff->Clone(&edit);
}

EditData *Edit_QuantizeEditor::EditDataMessage(EditData *data)
{
	switch(data->id)
	{
	case EDITDATA_STRENGTH:
		{
			edit.capturestrength=data->newvalue;
			RefreshEvents(true);
			ShowQuantizeStatus();
		}
		break;

	case EDITDATA_CAPTURERANGE:
		{
			edit.capturerange=data->newvalue;
			RefreshEvents(true);
			ShowQuantizeStatus();
		}
		break;

	default:
		return data;
	}

	return 0;
}

void Edit_QuantizeEditor::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGET_QUANTIZEFLAG:
		{
			DeletePopUpMenu(true);

			if(popmenu)
			{
				class menu_qflag:public guiMenu
				{
				public:
					menu_qflag(Edit_QuantizeEditor *e,QuantizeEffect *fx,int f)
					{
						editor=e;
						effect=fx;
						flag=f;
					}

					void MenuFunction()
					{
						if(effect->flag&flag)
							effect->flag CLEARBIT flag;
						else
							effect->flag |=flag;

						editor->RefreshEvents(true);
					} //

					Edit_QuantizeEditor *editor;
					QuantizeEffect *effect;
					int flag;
				};

				popmenu->AddFMenu("Notes",new menu_qflag(this,&edit,QUANTIZE_NOTES),edit.flag&QUANTIZE_NOTES?true:false);
				popmenu->AddFMenu("Control Change",new menu_qflag(this,&edit,QUANTIZE_CONTROL),edit.flag&QUANTIZE_CONTROL?true:false);
				popmenu->AddFMenu("Pitchbend",new menu_qflag(this,&edit,QUANTIZE_PITCHBEND),edit.flag&QUANTIZE_PITCHBEND?true:false);
				popmenu->AddFMenu("Poly Pressure",new menu_qflag(this,&edit,QUANTIZE_POLYPRESSURE),edit.flag&QUANTIZE_POLYPRESSURE?true:false);
				popmenu->AddFMenu("Channel Pressure",new menu_qflag(this,&edit,QUANTIZE_CHANNELPRESSURE),edit.flag&QUANTIZE_CHANNELPRESSURE?true:false);
				popmenu->AddFMenu("Program Change",new menu_qflag(this,&edit,QUANTIZE_PROGRAMCHANGE),edit.flag&QUANTIZE_PROGRAMCHANGE?true:false);
				popmenu->AddFMenu("SysEx",new menu_qflag(this,&edit,QUANTIZE_SYSEX),edit.flag&QUANTIZE_SYSEX?true:false);

				ShowPopMenu();	
			}// if popmenu
		}
		break;

	case GADGET_NOTEOFFFIXLENGTH:
		{
			edit.setnotelength=g->index?true:false;
			RefreshEvents(true);
		}
		break;

	case GADGET_NOTEOFFSELECTFIXLENGTH:
		CreateQuantizePopUp(g,QPUFLAG_NOTELENGTH);
		break;

	case GADGET_GROOVE:
		{
			maingui->OpenEditorStart(EDITORTYPE_GROOVE,mainvar->GetActiveSong(),0,0,0,0,0);
		}
		break;

	case GADGET_HUMANQUANTIZE:
		{
			edit.usehuman=g->index?true:false;
			RefreshEvents(true);
		}
		break;

	case GADGET_HUMANRANGE:
		CreateHumanPopUp(g);
		break;

	case GADGET_NOTEOFFQUANTIZE:
		{
			edit.noteoffquant=g->index?true:false;
			RefreshEvents(true);
		}
		break;

	case GADGET_CAPTURERANGE:
		{
			if(EditData *edit=new EditData)
			{
				edit->win=this;
				edit->x=g->x2;
				edit->y=g->y2;

				edit->id=EDITDATA_CAPTURERANGE;
				edit->title="Capture Range (%)";

				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->from=0;
				edit->to=100;
				edit->value=effect->capturerange;

				maingui->EditDataValue(edit);
			}
		}
		break;

	case GADGET_CAPTUREQUANTIZE:
		{
			edit.capturequant=g->index?true:false;
			RefreshEvents(true);
		}
		break;

	case GADGET_STRENGTH:
		{
			if(EditData *edit=new EditData)
			{
				edit->win=this;
				edit->x=g->x2;
				edit->y=g->y2;

				edit->id=EDITDATA_STRENGTH;
				edit->title="Quantize Strength (%)";

				edit->type=EditData::EDITDATA_TYPE_INTEGER;
				edit->from=0;
				edit->to=100;
				edit->value=effect->capturestrength;

				maingui->EditDataValue(edit);
			}
		}
		break;

	case GADGET_STRENGTHQUANTIZE:
		{
			edit.strengthquant=g->index?true:false;
			RefreshEvents(true);
		}
		break;

	case GADGET_QUANTIZEONOFF:
		{
			if(edit.usequantize==false)
			{
				edit.usegroove=false;
				edit.usequantize=true;
			}
			else
				edit.usequantize=false;

			RefreshEvents(true);
		}
		break;

	case GADGET_USEGROOVE:
		{
			edit.usegroove=true;
			edit.usequantize=false;
			RefreshEvents(true);
		}
		break;

	case GADGET_SELECTQUANTIZE:
		{
			CreateQuantizePopUp(g,QPUFLAG_QUANTIZE);
		}
		break;

	case GADGET_SELECTGROOVE:
		{
			CreateGroovePopUp(g,0);
		}
		break;

	case GADGET_RESETQUANT:
		{
			QuantizeEffect c;

			edit.Clone(&c);
			edit.Reset();

			if(c.Compare(&edit)==true)
				RefreshEvents(true);
		}
		break;
	}

	ShowQuantizeStatus();
}

void Edit_QuantizeEditor::ShowQuantizeStatus()
{
	if(gadgetlist)
	{
		if(flag)
		{
			char h[400];

			strcpy(h,"Q Events:");

			if(effect->flag&QUANTIZE_NOTES)
				mainvar->AddString(h," Notes");

			if(effect->flag&QUANTIZE_CONTROL)
				mainvar->AddString(h," Control");

			if(effect->flag&QUANTIZE_PITCHBEND)
				mainvar->AddString(h," Pitch");

			if(effect->flag&QUANTIZE_POLYPRESSURE)
				mainvar->AddString(h," PPress");

			if(effect->flag&QUANTIZE_CHANNELPRESSURE)
				mainvar->AddString(h," CPress");

			if(effect->flag&QUANTIZE_SYSEX)
				mainvar->AddString(h," SysEx");

			if(effect->flag&QUANTIZE_PROGRAMCHANGE)
				mainvar->AddString(h," Prog");

			if(effect->flag&QUANTIZE_INTERN)
				mainvar->AddString(h," Intern");

			flag->ChangeButtonText(h);
		}

		if(boxsetnotelength)
			boxsetnotelength->SetCheckBox(effect->setnotelength);

		if(notelength)
		{
			if(char *h=mainvar->GenerateString(Cxs[CXS_FIXNOTELENGTH],"<",effect->setnotelength==false?Cxs[CXS_DISABLED]:"F",">",quantstr[effect->notelength]) )
			{
				notelength->ChangeButtonText(h);
				delete h;
			}
		}

		if(boxquantize)
			boxquantize->SetCheckBox(effect->usequantize);

		if(boxgroove)
		{
			if(effect->usegroove==true)
			{
				boxgroove->SetCheckBox(true);
				boxgroove->Disable();
			}
			else
			{
				boxgroove->SetCheckBox(false);

				if((!mainMIDI->FirstGroove()) || (!effect->groove))
					boxgroove->Disable();
			}
		}

		if(selectquant)
		{
			if(char *h=mainvar->GenerateString(Cxs[CXS_QUANTIZETO],"<",effect->usequantize==false?Cxs[CXS_DISABLED]:"Q",">",quantstr[effect->quantize]) )
			{
				selectquant->ChangeButtonText(h);
				delete h;
			}
		}

		if(selectgroove)
		{
			if(!mainMIDI->FirstGroove())
			{
				selectgroove->ChangeButtonText(Cxs[CXS_NOGROOVESAVAILABLE]);
				selectgroove->Disable();
			}
			else
			{
				selectgroove->ChangeButtonText(effect->groove?effect->groove->GetGrooveName():Cxs[CXS_NOGROOVE]);
			}
		}

		if(boxnoteoffquant)
			boxnoteoffquant->SetCheckBox(effect->noteoffquant);

		if(boxcapturequant)
			boxcapturequant->SetCheckBox(effect->capturequant);

		if(capturerange)
		{
			char th[NUMBERSTRINGLEN];

			if(char *h=mainvar->GenerateString("Capture:",mainvar->ConvertIntToChar(effect->capturerange,th),"%"))
			{
				capturerange->ChangeButtonText(h);
				delete h;
			}
		}

		if(boxstrength)
			boxstrength->SetCheckBox(effect->strengthquant);

		if(strength)
		{
			char th[NUMBERSTRINGLEN];

			if(char *h=mainvar->GenerateString("Strength:",mainvar->ConvertIntToChar(effect->capturestrength,th),"%"))
			{
				strength->ChangeButtonText(h);
				delete h;
			}
		}

		if(boxhuman)
			boxhuman->SetCheckBox(effect->usehuman);

		if(human)
		{
			if(char *h=mainvar->GenerateString(Cxs[CXS_HUMANQUANTIZE],"<",effect->usehuman==false?Cxs[CXS_DISABLED]:"HQ",">",quantstr[effect->humanrange]) )
			{
				human->ChangeButtonText(h);
				delete h;
			}
		}
	}

	effect->Clone(&effectcompare);
}

void Edit_QuantizeEditor::RefreshObjects(LONGLONG type,bool editcall)
{
	ShowQuantizeStatus();
}

void Edit_QuantizeEditor::RefreshRealtime()
{
	if(effectcompare.Compare(effect)==true)
		ShowQuantizeStatus();
}

void Edit_QuantizeEditor::ResetGadgets()
{
	flag=
		boxquantize=
		boxgroove=
		boxnoteoffquant=
		boxcapturequant=
		boxstrength=
		selectquant=
		selectgroove=
		capturerange=
		strength=
		boxhuman=
		boxsetnotelength=
		notelength=
		human=
		reset=0;
}

void Edit_QuantizeEditor::ShowTitle()
{
	char *n=0;

	if(effect->track)
		n=mainvar->GenerateString(Cxs[CXS_QUANTIZETRACK],":",effect->track->GetName());
	else
		if(effect->pattern)
			n=mainvar->GenerateString(Cxs[CXS_QPATTERN],":",effect->pattern->GetName());

	if(n)
	{
		guiSetWindowText(n);
		delete n;
	}
}

void Edit_QuantizeEditor::Init()
{
#ifdef OLDIE
	gadgetlists.RemoveAllGadgetLists();
	gadgetlist=gadgetlists.AddGadgetList(this);

	ResetGadgets();

	if(gadgetlist)
	{
		int x=1;
		int x2=width;
		int y=1;

		/*
		boxquantize=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_QUANTIZEONOFF,Cxs[CXS_QUANTIZE],Cxs[CXS_QUANTIZE]);
		y=maingui->AddFontY(y);

		flag=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_QUANTIZEFLAG,0,Cxs[CXS_EVENTQTYPES]);

		y=maingui->AddFontY(y);

		selectquant=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_SELECTQUANTIZE,0,Cxs[CXS_SELECTQRASTER]);
		y=maingui->AddFontY(y);

		boxgroove=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_USEGROOVE,"Groove",Cxs[CXS_GROOVEQUANTIZE]);
		y=maingui->AddFontY(y);

		selectgroove=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_SELECTGROOVE,0,Cxs[CXS_SELECTGROOVERASTER]);
		y=maingui->AddFontY(y);

		boxnoteoffquant=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_NOTEOFFQUANTIZE,Cxs[CXS_QUANTIZENOTEOFFEND],Cxs[CXS_QUANTIZENOTEOFFPOSITION]);
		y=maingui->AddFontY(y);

		boxsetnotelength=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_NOTEOFFFIXLENGTH,Cxs[CXS_FIXNOTELENGTH],Cxs[CXS_FIXNOTELENGTH]);
		y=maingui->AddFontY(y);

		notelength=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_NOTEOFFSELECTFIXLENGTH,0,Cxs[CXS_SELECTFIXNOTELENGTH]);
		y=maingui->AddFontY(y);

		// CAPTURE
		boxcapturequant=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_CAPTUREQUANTIZE,Cxs[CXS_CAPTUREQUANTIZE],Cxs[CXS_CAPTUREQUANTIZE]);
		y=maingui->AddFontY(y);

		capturerange=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_CAPTURERANGE,0,Cxs[CXS_RANGEOFCAPTURE]);
		y=maingui->AddFontY(y);

		boxstrength=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_STRENGTHQUANTIZE,Cxs[CXS_STRENGTHQUANTIZE],Cxs[CXS_STRENGTHQUANTIZE]);
		y=maingui->AddFontY(y);

		strength=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_STRENGTH,0,"Strength");
		y=maingui->AddFontY(y);

		boxhuman=gadgetlist->AddCheckBox(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_HUMANQUANTIZE,Cxs[CXS_HUMANQUANTIZE],Cxs[CXS_USEHUMANQUANTIZE]);
		y=maingui->AddFontY(y);

		human=gadgetlist->AddButton(x,y,x2,y+maingui->GetFontSizeY_Sub(),GADGET_HUMANRANGE,0,Cxs[CXS_HUMANQUANTIZERANGE]);
		y=maingui->AddFontY(y);

		reset=gadgetlist->AddButton(x,y,x+((x2-x)/2),y+maingui->GetFontSizeY_Sub(),"Reset",GADGET_RESETQUANT,0,Cxs[CXS_RESETQSETTINGS]);

		gadgetlist->AddButton(x+((x2-x)/2)+1,y,x2,y+maingui->GetFontSizeY_Sub(),"Groove Editor",GADGET_GROOVE,0,"Groove Editor");
		ShowQuantizeStatus();
		*/
	}
#endif

}

void QuantizeEffect::Load(camxFile *file)
{
	file->LoadChunk();

	switch(file->GetChunkHeader())
	{
	case CHUNK_QUANTIZEFX:	
		{
			file->ChunkFound();

			file->ReadAndAddClass((CPOINTER)this);

			file->ReadChunk(&capturequant);
			file->ReadChunk(&noteonquant);
			file->ReadChunk(&noteoffquant);
			file->ReadChunk(&groovequant);
			file->ReadChunk(&strengthquant);

			file->ReadChunk(&quantize);
			file->ReadChunk(&capturestrength);
			file->ReadChunk(&capturerange);
			file->ReadChunk(&strengthquant);

			file->ReadChunk(&usequantize);
			file->ReadChunk(&usegroove);
			file->ReadChunk(&usehuman);

			file->ReadChunk(&humanrange);

			file->ReadChunk(&setnotelength);
			file->ReadChunk(&notelength);

			file->ReadChunk(&flag);
			file->ReadChunk(&humanq);

			file->CloseReadChunk();
		}
		break;
	}
}

void QuantizeEffect::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_QUANTIZEFX);

	file->Save_Chunk((CPOINTER)groove); // Pointer

	file->Save_Chunk(capturequant);
	file->Save_Chunk(noteonquant);
	file->Save_Chunk(noteoffquant);
	file->Save_Chunk(groovequant);
	file->Save_Chunk(strengthquant);

	file->Save_Chunk(quantize);
	file->Save_Chunk(capturestrength);
	file->Save_Chunk(capturerange);
	file->Save_Chunk(strengthquant);

	file->Save_Chunk(usequantize);
	file->Save_Chunk(usegroove);
	file->Save_Chunk(usehuman);

	file->Save_Chunk(humanrange);

	file->Save_Chunk(setnotelength);
	file->Save_Chunk(notelength);

	file->Save_Chunk(flag);

	file->Save_Chunk(humanq);

	file->CloseChunk();
}

bool QuantizeEffect::QuantizeEvent(Seq_Event *seqevent)
{
	if(usequantize==true ||	usegroove==true ) // Quantize ON ?
	{
		OSTART qpos;

		if(usegroove==true)
			qpos=groove?groove->Quantize(seqevent->staticostart):seqevent->ostart;
		else
			qpos=Quantize(seqevent->staticostart);

		if(seqevent->ostart!=qpos)
		{
			seqevent->MoveEventNoStatic(qpos);

			return true;
		}
	}
	else // Quantize OFF
	{	
		if(seqevent->staticostart!=seqevent->ostart)
		{
			seqevent->MoveEventNoStatic(seqevent->staticostart);

			return true;
		}
	}

	return false;
}

bool QuantizeEffect::IsInUse()
{
	if(usequantize==true)
		return true;

	return false;
}

void QuantizeEffect::Reset()
{
	flag=QUANTIZE_NOTES;

	groove=0;

	capturerange=100; // 100 % capture
	capturestrength=100;

	noteoffquant=false;
	noteonquant=true;

	capturequant=false;
	groovequant=false;

	usequantize=false;
	usegroove=false;
	usehuman=false;

	strengthquant=false;

	quantize=10; // 1/8
	humanrange=25; // 25 %

	setnotelength=false;
	notelength=10; // 1/8
}

bool QuantizeEffect::Compare(QuantizeEffect *e2) // true==not the same
{
	if(flag!=e2->flag ||
		groove!=e2->groove ||

		capturequant!=e2->capturequant ||
		capturerange!=e2->capturestrength ||
		capturestrength!=e2->capturestrength ||

		noteoffquant!=e2->noteoffquant ||
		noteonquant!=e2->noteonquant ||

		groovequant!=e2->groovequant ||

		usequantize!=e2->usequantize ||
		usegroove!=e2->usegroove ||
		usehuman!=e2->usehuman ||

		quantize!=e2->quantize ||
		strengthquant!=e2->strengthquant ||
		humanrange!=e2->humanrange ||
		humanq!=e2->humanq ||

		notelength!=e2->notelength ||
		setnotelength!=e2->setnotelength
		)
		return false;

	return true;
}

void QuantizeEffect::Clone(QuantizeEffect *to)
{
	to->flag=flag;

	to->groove=groove;

	to->capturequant=capturequant;
	to->capturerange=capturerange;
	to->capturestrength=capturestrength;

	to->noteoffquant=noteoffquant;
	to->noteonquant=noteonquant;

	to->groovequant=groovequant;

	to->usequantize=usequantize;
	to->usegroove=usegroove;

	to->quantize=quantize;

	to->usehuman=usehuman;
	to->humanrange=humanrange;
	to->humanq=humanq;

	to->strengthquant=strengthquant;

	to->setnotelength=setnotelength;
	to->notelength=notelength;
}

OSTART QuantizeEffect::QuantizeNoteOff(OSTART p)
{
	OSTART qticks=quantlist[quantize],r=(p/qticks)+1;
	return r*qticks;
}

OSTART QuantizeEffect::Quantize(OSTART p)
{
	if(usequantize==true) // quantize ==0, quantize off
	{
		OSTART qticks=quantlist[quantize],l,r;

		l=r=p/qticks;

		r++;

		l*=qticks;
		r*=qticks;

		if(capturequant==true)
		{
			double dh,perc;

			perc=(double)capturerange;
			perc/=100; // 0-1

			dh=(double)qticks/2;
			dh*=perc;

			OSTART ldh=(OSTART)floor(dh+0.5);

			if(p>=(r-ldh)) // ---- |
				return r;

			if(p<=(l+ldh)) // | ----
				return l;

			// not in range
		}	
		else
		{
			// Standard Quant
			if(p-l<=r-p)
				return l;

			return r;
		}
	}
	else
		if(groovequant==true)
		{
			// Groove Quantize
		}

		return(p);
}

bool QuantizeEffect::QuantizeNote(Note *note)
{
	if(usequantize==true ||	usegroove==true || setnotelength==true) // Quantize ON ?
	{
		OSTART qpos,notelen=note->off.staticostart-note->staticostart; // Static Length

		if(noteonquant==true)
		{
			// Quantize Start
			if(usegroove==true)
				qpos=groove?groove->Quantize(note->staticostart):note->ostart;
			else
				qpos=Quantize(note->staticostart);
		}
		else
			qpos=note->ostart;

		// NoteOff
		{
			OSTART noteoffpos;

			if(setnotelength==true)
			{
				// Fix Note Length
				noteoffpos=qpos+quantlist[notelength];
			}
			else
				if(noteoffquant==true)
				{
					// Quant NoteOff
					if(usegroove==true)
						noteoffpos=groove?groove->Quantize(note->off.staticostart):note->off.ostart;
					else
						noteoffpos=QuantizeNoteOff(note->off.staticostart);			
				}
				else
					noteoffpos=qpos+notelen; // keep Note Length

			if(noteoffpos<=qpos) // Min 1 Tick NoteOn->NoteOff
				noteoffpos=qpos+1;

			OSTART len=noteoffpos-qpos;
			noteoffpos=qpos+len;

			if(note->olist && (note->ostart!=qpos || noteoffpos!=note->off.ostart))
			{
				note->MoveNote(qpos,noteoffpos);
				return true;
			}
		}
	}
	else // Quantize OFF
	{	
		if(note->olist && (note->staticostart!=note->ostart || note->off.staticostart!=note->off.ostart))
		{
			note->MoveNote(note->staticostart,note->off.staticostart);
			return true;
		}
	}

	return false;
}

char *QuantizeEffect::GetQuantizeInfo()
{
	char mix[200];

	mix[0]=0;

	if(usequantize==true)
	{
		mainvar->MixString(mix,"<Q>:",quantstr[quantize]);

		mainvar->AddString(mix,"-");

		if(noteonquant==true)
			mainvar->AddString(mix,"<N>");

		if(noteoffquant==true)
			mainvar->AddString(mix,"<Off>");

		if(capturequant==true)
			mainvar->AddString(mix,"<CP>");

		if(strengthquant==true)
			mainvar->AddString(mix,"<ST>");
	}
	else
		if(usegroove==true && groove)
			mainvar->MixString(mix,"[G]:",groove->GetGrooveInfo());
		else
			strcpy(mix,Cxs[CXS_NOQUANTIZE]);

	if(setnotelength==true)
		mainvar->AddString(mix,"<NL>");

	if(usehuman==true)
	{
		mainvar->AddString(mix,"<HU>:");
		mainvar->AddString(mix,quantstr[humanrange]);
	}

	if(strlen(mix))
	{
		if(qinfo)
			delete qinfo;

		qinfo=mainvar->GenerateString(mix);
	}

	return qinfo;
}

void QuantizeEffect::SetHumanRange(int r)
{
	mainthreadcontrol->LockActiveSong();
	humanrange=r;

	if(usehuman==true)
		Humanize(true);

	mainthreadcontrol->UnlockActiveSong();

}

void QuantizeEffect::SetHumanQ(int q)
{
	mainthreadcontrol->LockActiveSong();

	humanq=q;

	if(usehuman==true)
		Humanize(true);

	mainthreadcontrol->UnlockActiveSong();
}

void QuantizeEffect::Humanize(bool useh)
{
	mainthreadcontrol->LockActiveSong();

	usehuman=useh; // Use Humanize On/Off

	Seq_Pattern *p=track->FirstPattern(MEDIATYPE_MIDI);

	while(p)
	{
		MIDIPattern *mp=(MIDIPattern *)p;
		mp->InitSwing();
		p=p->NextPattern(MEDIATYPE_MIDI);
	}

	mainthreadcontrol->UnlockActiveSong();
}