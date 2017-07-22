#include "songmain.h"
#include "editor_text.h"
#include "gui.h"
#include "camxgadgets.h"
#include "guigadgets.h"
#include "seqtime.h"
#include "object_song.h"
#include "editfunctions.h"
#include "settings.h"
#include "languagefiles.h"
#include "undofunctions_text.h"
#include "editdata.h"

#define TEXT_FRAMEID_POSITION 0
#define TEXT_FRAMEID_TEXT 1
#define TEXT_FRAMEID_TEXTSTRING 2

#define TEXT_POSITIONX_STD 1
#define TEXT_POSITIONX_MIN TEXT_POSITIONX_STD +35
#define TEXT_POSITIONX_SEC TEXT_POSITIONX_STD	+60
#define TEXT_POSITIONX_TK TEXT_POSITIONX_STD	+90
#define TEXT_POSITIONX_QF TEXT_POSITIONX_STD+120

// Note Length
#define TEXTEDIT_NOTELENGTH_1000 30
#define TEXTEDIT_NOTELENGTH_0100 31
#define TEXTEDIT_NOTELENGTH_0010 32
#define TEXTEDIT_NOTELENGTH_0001 33

#define TEXTEDIT_NOTELENGTH_HOUR 40
#define TEXTEDIT_NOTELENGTH_MIN 41
#define TEXTEDIT_NOTELENGTH_SEC 42
#define TEXTEDIT_NOTELENGTH_FRAME 43
#define TEXTEDIT_NOTELENGTH_QFRAME 44

enum TEditDataIDs
{
	EDITID_TEXT=EventEditor::EDITOREDITDATA_ID
};

bool Edit_Text::Scroll(int diff,bool refreshdisplay)
{
	/*
	if(diff && FirstText())
	{
	bool scroll=false;
	long nrTEXTs=patternselection();

	if(diff<0)
	{
	Edit_Text_Text *fe=FirstText();
	long c=firsttext_index+diff;

	if(c>=0)
	{
	firsttext_index=c;
	scroll=true;
	}
	}
	else
	{
	long nrTEXTs=patternselection.GetCountOfTEXTs();
	long ondisplay=firsttext_index+numberofdisplaytexts;

	if(ondisplay+diff<=nrTEXTs)
	{
	firsttext_index+=diff;
	scroll=true;
	}
	}

	if(scroll==true && refreshdisplay==true)
	{
	ShowTEXTs();
	}

	return scroll;
	}
	*/

	return false;
}

void Edit_Text::CheckMouseButtonDown()
{
	//	MessageBeep(-1);

	if(editdata!=NOEVENTEDIT)
	{
		addtolastundo=true;

		if(refreshmousebuttonright==true)
			EditTexts(editdata,1);
		else
			EditTexts(editdata,-1);
	}
}

bool Edit_Text::KeepTextinMid(Seq_Text *e,bool scroll)
{
	if(e)
	{
		int c=numberofdisplaytexts;
		int ce=cursor_index=e->GetIndex();

		cursortext=e;

		if(ce>=0)
		{
			c/=2;
			int h=ce-c;

			if(h>=0 && scroll==true)
				firsttext_index=h;

			cursortext=e;

			ShowTexts();

			ShowCursorText();

			return true;
		}
#ifdef _DEBUG
		else
		{
			if(ce<0)
				MessageBox(NULL,"Illegal Keep in Mid","Error",MB_OK);
		}
#endif

	}

	return false;
}

void Edit_Text::KeyDown()
{
	Editor_KeyDown();

	switch(nVirtKey)
	{
	}
}

void Edit_Text::ShowCursorText()
{
	/*
	if(infotext && cursortext)
	{
	Seq_Text *TEXT=cursortext->text;

	Seq_Pattern *p=TEXt->GetPattern();
	Seq_Track *t=TEXt->GetTrack();

	if(p && t)
	{
	long l=strlen(p->GetName())+strlen(t->name);

	l+=16;

	char *h=new char[l];

	if(h)
	{
	strcpy(h,t->name);
	mainvar->AddString(h," / ");
	mainvar->AddString(h,p->GetName());

	infotext->SetString(h);

	delete h;
	}
	}
	else
	infotext->SetString("TEXT Error");
	}
	*/
}

void Edit_Text::KeyUp()
{

}

void Edit_Text::MouseWheel(int delta,guiGadget *db)
{
	if(Edit_MouseWheel(delta)==true)
	{
	}
}

bool Edit_Text::ZoomGFX(int z,bool horiz)
{
#ifdef OLDIE
	if(SetZoomGFX(z,horiz)==true)
	{
		CreateGUIBuffer();
		ShowAllText();
		BlitGUIInfos();

		return true;
	}
#endif

	return false;
}

Edit_Text::Edit_Text()
{
	editorname="Text";
	editorid=EDITORTYPE_TEXT;
	firsttext_index=cursortext_index=0;

	zoomy=20;

	editdata=NOEVENTEDIT;

	filtergadget=0;
	selectmode=false;

	numberofdisplaytexts=0;

	movetexts=false;
	lengthchanged=false;

	cursor=CURSORPOS1000;
	cursortext=0;

	//frame.textname.infostring="Text";
	//frame.textname.infostringflag=INFOSTRINGFLAG_TOP;
	ResetGadgets();

	followsongposition=mainsettings->followeditor;
}

void Edit_Text::EditTextPosition()
{
#ifdef OLDIE
	editdata=NOEVENTEDIT;

	Edit_Text_Text *f=FindTextA(GetMouseX(),GetMouseY());	

	edittext=0;

	if(f)
	{
		edittext=f->text;

		if(maingui->GetCtrlKey()==false)
		{
			// Positions
			if(frame.position.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
			{
				if(edittext)
				{
					if(position_1000_x<=GetMouseX() && position_0100_x>GetMouseX())
					{
						switch(windowdisplay)
						{
						case WINDOWDISPLAY_MEASURE:
							editdata=EVENTEDIT_1000;
							break;

						case WINDOWDISPLAY_SMPTE:

							editdata=EVENTEDIT_HOUR;
							break;
						}
					}
					else
						if(position_0100_x<=GetMouseX() && position_0010_x>GetMouseX())
						{
							switch(windowdisplay)
							{
							case WINDOWDISPLAY_MEASURE:
								editdata=EVENTEDIT_0100;
								break;

							case WINDOWDISPLAY_SMPTE:

								editdata=EVENTEDIT_MIN;
								break;
							}
						}
						else
							if(position_0010_x<=GetMouseX() && position_0001_x>GetMouseX())
							{
								switch(windowdisplay)
								{
								case WINDOWDISPLAY_MEASURE:
									editdata=EVENTEDIT_0010;
									break;

								case WINDOWDISPLAY_SMPTE:

									editdata=EVENTEDIT_SEC;
									break;
								}
							}
							else
								if(position_0001_x<=GetMouseX() && position_0001QF_x>GetMouseX())
								{
									switch(windowdisplay)
									{
									case WINDOWDISPLAY_MEASURE:
										editdata=EVENTEDIT_0001;
										break;

									case WINDOWDISPLAY_SMPTE:

										editdata=EVENTEDIT_FRAME;
										break;
									}
								}
								else
									if(position_0001_x<=GetMouseX())
									{
										switch(windowdisplay)
										{
										case WINDOWDISPLAY_MEASURE:
											editdata=EVENTEDIT_0001;
											break;

										case WINDOWDISPLAY_SMPTE:

											editdata=EVENTEDIT_QFRAME;
											break;
										}
									}
				}//if e
			}//if frame
		}// ctrl key ?
	}// if f

	if(editdata!=NOEVENTEDIT)
	{
		if(left_mousekey==MOUSEKEY_DOWN)
		{
			refreshmousebuttondown=true;
			refreshmousebuttonright=false; // RMB
			EditTexts(editdata,-1);
		}
		else
		{
			if(right_mousekey==MOUSEKEY_DOWN)
			{
				refreshmousebuttondown=true;
				refreshmousebuttonright=true; // LMB
				EditTexts(editdata,1);
			}
		}
	}
#endif

}

void Edit_Text::EditText(Seq_Text *t)
{
	if(t)
	{
		EditData *edit=new EditData;

		if(edit)
		{
			// long position;
			edit->song=WindowSong();
			edit->win=this;
			edit->x=GetWindowMouseX();
			edit->y=GetWindowMouseY();

			edit->title="Edit Text";
			edit->deletename=false;

			edit->id=EDITID_TEXT;

			edit->type=EditData::EDITDATA_TYPE_STRING;

			edit->helpobject=t;
			edit->string=t->string;

			maingui->EditDataValue(edit);
		}
	}
}

Edit_Text_Text *Edit_Text::FindTextText(int x,int y)
{
	Edit_Text_Text *e=FirstText();

	while(e)
	{
		if(e->y<=y && e->y2>y)
			return e;

		e=e->NextText();
	}

	return 0;
}

void Edit_Text::MouseButton(int flag)
{
#ifdef OLDIE
	if(CheckEditorMouseButton()==false)return;

	selectmode=false;

	if(flag&(MOUSEKEY_LEFT_UP|MOUSEKEY_RIGHT_UP)) // Mouse Released
	{
		editdata=NOEVENTEDIT;
	}
	else
	{
		if(frame.position.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
		{
			EditTextPosition();
		}
		else
			switch(left_mousekey)
		{
			case MOUSEKEY_DOWN:
				{
					if(frame.textname.CheckIfInFrame(GetMouseX(),GetMouseY())==true)
					{
						if(maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false)
							SelectAllTexts(false);

						if(Edit_Text_Text *text=FindTextText(GetMouseX(),GetMouseY()))
						{
							if(maingui->GetShiftKey()==true)
							{
								int ix_f=WindowSong()->textandmarker.text.GetIx(text->text);

								if(!WindowSong()->textandmarker.lastselectedtext)
									WindowSong()->textandmarker.lastselectedtext=text->text;

								int ix_t=WindowSong()->textandmarker.text.GetIx(WindowSong()->textandmarker.lastselectedtext);

								Seq_Text *from=ix_f<ix_t?text->text:WindowSong()->textandmarker.lastselectedtext;
								Seq_Text *to=ix_f<ix_t?WindowSong()->textandmarker.lastselectedtext:text->text;

								for(;;)
								{
									from->flag|=OFLAG_SELECTED;
									if(from==to)break;
									from=from->NextText();
								}
							}
							else
							{
								WindowSong()->textandmarker.lastselectedtext=text->text;

								if(text->text->flag&OFLAG_SELECTED)
								{
									selecttype=false;
									text->text->flag CLEARBIT OFLAG_SELECTED;
								}
								else
								{
									selecttype=true;
									text->text->flag|=OFLAG_SELECTED;
								}
							}

						}
						else
						{
							selecttype=true;
						}

						selectmode=true;
					}
					else
						if (frame.textname.CheckIfInFrame(GetMouseX(),GetMouseY())==true ||
							frame.textstring.CheckIfInFrame(GetMouseX(),GetMouseY())==true
							)
						{
							switch(mousemode)
							{
							case EM_CREATE:
								{
									Seq_Text *t=WindowSong()->textandmarker.AddText(WindowSong()->GetSongPosition(),"---");

									if(t){
										AddSteptoSongPosition();

										maingui->RefreshAllEditorsWithText(WindowSong());
										EditText(t);
									}
								}
								break;

							case EM_DELETE:
								{
									Edit_Text_Text *t=FindText(GetMouseX(),GetMouseY());

									if(t){
										WindowSong()->textandmarker.DeleteText(t->text);
										maingui->RefreshAllEditorsWithText(WindowSong());
									}
								}
								break;

							case EM_EDIT:
								{
									Edit_Text_Text *t=FindText(GetMouseX(),GetMouseY());

									if(t)
										EditText(t->text);

								}
								break;
							}

						}
				}
				break; // down
		}
	}
#endif

}

void Edit_Text::EditTexts(int what,int datadiff)
{
	/*
	int pos=-1;

	if(edittext)
	switch(editdata)
	{
	// Positions
	case EVENTEDIT_1000: // Measure
	{
	Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(edittext->GetTextStart());
	pos=edittext->GetTextStart()+(datadiff*sig->measurelength);
	}
	break;

	case EVENTEDIT_0100: // Beat
	{
	Seq_Signature *sig=WindowSong()->timetrack.FindSignatureBefore(edittext->GetTextStart());
	pos=edittext->GetTextStart()+(datadiff*sig->dn_ticks);
	}
	break;

	case EVENTEDIT_0010: // zoom
	pos=edittext->GetTextStart()+datadiff*WindowSong()->timetrack.zoomticks;
	break;

	case EVENTEDIT_0001: // Ticks
	{
	pos=edittext->GetTextStart()+datadiff; // +1/-1
	}
	break;

	case EVENTEDIT_HOUR:
	case EVENTEDIT_MIN:
	case EVENTEDIT_SEC:
	case EVENTEDIT_FRAME:
	case EVENTEDIT_QFRAME:
	{
	WindowSong()->timetrack.ConvertTicksToPos(edittext->GetTextStart(),displayusepos);

	switch(what)
	{
	case EVENTEDIT_HOUR:
	displayusepos->pos[0]+=datadiff;
	break;

	case EVENTEDIT_MIN:
	displayusepos->pos[1]+=datadiff;
	break;

	case EVENTEDIT_SEC:
	displayusepos->pos[2]+=datadiff;
	break;

	case EVENTEDIT_FRAME:
	displayusepos->pos[3]+=datadiff;
	break;

	case EVENTEDIT_QFRAME:
	displayusepos->pos[4]+=datadiff;
	break;
	}

	if(displayusepos->PositionChanged()==true)
	{
	pos=WindowSong()->timetrack.ConvertPosToTicks(displayusepos,true);
	//WindowSong()->timetrack.ConvertTicksToPos(pos,displayusepos);
	}
	}
	break;
	}

	// edit ?

	if(pos!=-1)
	{
	if(WindowSong()->textandmarker.ChangeTextPosition(edittext,pos)==true)
	maingui->RefreshAllEditorsWithText(WindowSong());
	}
	*/
}

void Edit_Text::SelectAllTexts(bool on)
{
	Seq_Text *t=WindowSong()->textandmarker.FirstText();

	while(t)
	{
		if(on==true)
			t->Select();
		else
			t->UnSelect();

		t=t->NextText();
	}
}

void Edit_Text::MouseMove(bool inside)
{
#ifdef OLDIE
	bool used=EditorMouseMove(inside);

	TRACE ("Select Mode %d\n",selectmode);

	if(selectmode==true)
	{
		Edit_Text_Text *text=FindTextText(GetMouseX(),GetMouseY());

		if(text)
		{
			int ix_f=text->text->GetIndex();

			if(!WindowSong()->textandmarker.lastselectedtext)
				WindowSong()->textandmarker.lastselectedtext=text->text;

			int ix_t=WindowSong()->textandmarker.lastselectedtext->GetIndex();

			Seq_Text *from=ix_f<ix_t?text->text:WindowSong()->textandmarker.lastselectedtext;
			Seq_Text *to=ix_f<ix_t?WindowSong()->textandmarker.lastselectedtext:text->text;

			for(;;)
			{
				if(selecttype==true)
					from->flag|=OFLAG_SELECTED;
				else
					from->flag CLEARBIT OFLAG_SELECTED;

				if(from==to)break;

				from=from->NextText();
			}
		}
	}
	else
		if(used==false && editdata!=NOEVENTEDIT) // Edit MIDI TEXT Data
		{
			int diffx=GetMouseX()-editstart_x;
			int diffy=GetMouseY()-editstart_y;
			int datadiff;

			datadiff=diffy;
		}
#endif

}

EditData *Edit_Text::EditDataMessage(EditData *data)
{
	if(CheckStandardDataMessage(data)==true)
		return 0;

	switch(data->id)
	{
	case EDITID_TEXT:
		{
			Seq_Text *t=(Seq_Text *)data->helpobject;

			maingui->ChangeText(t,data->newstring);
		}
		break;

	default:
		return data;
		break;
	}

	return 0;
}

char *Edit_Text::GetWindowName()
{
	char h[64],h2[32];
	strcpy(h,"-T:");
	mainvar->AddString(h,mainvar->ConvertIntToChar(getcountselectedtexts=WindowSong()->textandmarker.GetCountOfSelectedTexts(),h2));
	mainvar->AddString(h,"/");
	mainvar->AddString(h,mainvar->ConvertIntToChar(getcounttexts=WindowSong()->textandmarker.GetCountOfTexts(),h2));

	if(windowname)
	{
		if(char *hn=mainvar->GenerateString(windowname,h))
		{
			delete windowname;
			windowname=hn;
		}
	}

	return windowname;
}

void Edit_Text::RefreshRealtime_Slow()
{
	int h_getcountselectedtexts=WindowSong()->textandmarker.GetCountOfSelectedTexts(),
		h_getcounttexts=WindowSong()->textandmarker.GetCountOfTexts();

	if(h_getcountselectedtexts!=getcountselectedtexts ||
		h_getcounttexts!=getcounttexts)
		SongNameRefresh();
}

// Buttons,Slider ...
void Edit_Text::Gadget(guiGadget *gadget)
{	
	gadget=Editor_Gadget(gadget);

	if(gadget)
		switch(gadget->gadgetID)
	{		
		case GADGETID_EDITORSLIDER_VERT: // Track Scroll
			{
				firsttext_index=gadget->GetPos();
				ShowAllText();
			}
			break;

		case GADGETID_EDITORSLIDER_VERTZOOM:
			ZoomGFX(gadget->GetPos());
			break;
	}
}

Edit_Text_Text *Edit_Text::FindText(int x,int y)
{
	Edit_Text_Text *e=FirstText();

	while(e){
		if(e->y<=y && e->y2>y)
			return e;

		e=e->NextText();
	}

	return 0;
}

void Edit_Text::ShowTexts()
{
#ifdef OLDIE
	Edit_Text_Text *lasttext=0;
	bool grey;
	int y=frame.position.y;

	numberofdisplaytexts=0;

	textsineditor.DeleteAllO();
	DeleteAllNumberObjects();

	if(WindowSong()->textandmarker.FirstText())
	{
		// Start TEXT
		Seq_Text *stext=WindowSong()->textandmarker.GetTextAtIndex(firsttext_index);

		if(!stext){
			int maxindex=WindowSong()->textandmarker.GetCountOfTexts();

			if(maxindex){
				maxindex--;

				if(firsttext_index>maxindex)
					firsttext_index=maxindex;
				else
					firsttext_index=0;

				stext=WindowSong()->textandmarker.GetTextAtIndex(firsttext_index);
			}
		}

		// Cursor TEXT
		cursortext=WindowSong()->textandmarker.GetTextAtIndex(cursortext_index);

		if(!cursortext){
			int maxindex=WindowSong()->textandmarker.GetCountOfTexts();

			if(maxindex){
				maxindex--;

				if(cursortext_index>maxindex)
					cursortext_index=maxindex;
				else
					cursortext_index=0;

				cursortext=WindowSong()->textandmarker.GetTextAtIndex(cursortext_index);
			}
		}

		ClearTextDisplay();

		if(stext)
		{
			InitDisplay();

			if(frame.position.ondisplay==true) // Positions
			{
				position_1000_x=frame.position.x;
				position_0100_x=frame.position.x+TEXT_POSITIONX_MIN;
				position_0010_x=frame.position.x+TEXT_POSITIONX_SEC;
				position_0001_x=frame.position.x+TEXT_POSITIONX_TK;
				position_0001QF_x=frame.position.x+TEXT_POSITIONX_QF;
			}

			if(stext){
				int ix=WindowSong()->textandmarker.GetOfText(stext);
				grey=ix&1?true:false;
			}

			while(stext && y<=frame.position.y2) {

				if(Edit_Text_Text *newe=new Edit_Text_Text){
					lasttext=newe;

					newe->text=stext;
					newe->grey=grey;
					newe->y=y;
					newe->y2=y+(zoomy-1);
					newe->editor=this;

					textsineditor.AddEndO(newe);

					newe->Draw(); // Draw + Fill numberobjects

					AddNumberOList(newe);

					if(newe->y2<=frame.position.y2)
						numberofdisplaytexts++;
				}

				grey=grey==true?false:true;
				y+=zoomy;

				stext=stext->NextText();
			}	
		}
	}

	if(lasttext){
		if(frame.textstring.ondisplay==true)
			guibuffer->guiFillRect(frame.textstring.x,lasttext->y2+1,frame.textstring.x2,frame.textstring.y2,COLOUR_UNUSED);

		if(frame.textname.ondisplay==true)
			guibuffer->guiFillRect(frame.textname.x,lasttext->y2+1,frame.textname.x2,frame.textname.y2,COLOUR_UNUSED);

		if(frame.position.ondisplay==true)
			guibuffer->guiFillRect(frame.position.x,lasttext->y2+1,frame.position.x2,frame.position.y2,COLOUR_UNUSED);
	}
	else{
		frame.textname.Fill(guibuffer,COLOUR_UNUSED);
		frame.textstring.Fill(guibuffer,COLOUR_UNUSED);
		frame.position.Fill(guibuffer,COLOUR_UNUSED);
	}

	if(!(winmode&WINDOWMODE_FRAMES))
	{
		BltGUIBuffer_Frame(&frame.position);
		BltGUIBuffer_Frame(&frame.textname);
		BltGUIBuffer_Frame(&frame.textstring);
	}

	// Show Slider
	if(vertgadget){

		int nrTEXTs=WindowSong()->textandmarker.GetCountOfTexts();

		nrTEXTs-=numberofdisplaytexts;

		vertgadget->ChangeSlider(0,nrTEXTs,firsttext_index);
		vertgadget->ChangeSliderPage(numberofdisplaytexts);
	}
#endif

}

void Edit_Text::RefreshText(Seq_Text *t)
{
	Edit_Text_Text *f=FirstText();

	while(f){

		if(f->text==t){

			f->Draw(true);
			break;
		}

		f=f->NextText();
	}
}

void Edit_Text::RefreshRealtime()
{
	RefreshEventEditorRealtime();

	Edit_Text_Text *t=FirstText();

	while(t){
		if(t->flag!=t->text->flag)t->Draw(true);
		t=t->NextText();
	}
}

void Edit_Text::RefreshObjects(LONGLONG type,bool editcall)
{
	ShowAllText();
}

void Edit_Text::ClearTextDisplay()
{
#ifdef OLDIE
	frame.textname.Fill(guibuffer,COLOUR_WHITE);
	frame.textstring.Fill(guibuffer,COLOUR_WHITE);
	frame.position.Fill(guibuffer,COLOUR_WHITE);
#endif

	textsineditor.DeleteAllO();
}

void Edit_Text::ShowMenu()
{
	ShowEditorMenu();
}

void Edit_Text::Goto(int to)
{
	UserEdit();

	if(CheckStandardGoto(to)==true)
		return;

	switch(to)
	{
	case GOTO_FIRST:
		{
			Seq_Text *txt=WindowSong()->textandmarker.FirstText();

			if(txt)
			{
				if(NewStartPosition(txt->GetTextStart(),true)==true)
					SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_LAST:
		{
			Seq_Text *txt=WindowSong()->textandmarker.LastText();

			if(txt)
			{
				if(NewStartPosition(txt->GetTextStart(),true)==true)
					SyncWithOtherEditors();
			}
		}
		break;

	case GOTO_FIRSTSELECTED:
		{
			Seq_Text *t=WindowSong()->textandmarker.FirstText();

			while(t)
			{
				if(t->flag&OFLAG_SELECTED)
				{
					if(NewStartPosition(t->GetTextStart(),true)==true)
						SyncWithOtherEditors();
					return;
				}

				t=t->NextText();
			}
		}
		break;

	case GOTO_LASTSELECTED:
		{
			Seq_Text *t=WindowSong()->textandmarker.LastText();
			while(t)
			{
				if(t->flag&OFLAG_SELECTED)
				{
					if(NewStartPosition(t->GetTextStart(),true)==true)
						SyncWithOtherEditors();
					return;
				}

				t=t->PrevText();
			}
		}
		break;
	}
}

void Edit_Text::DeleteTexts()
{
	mainedit->DeleteSelectedTexts(WindowSong(),addtolastundo);
	addtolastundo=true;
}

guiMenu *Edit_Text::CreateMenu()
{
#ifdef OLDIE
//	ResetUndoMenu();

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		// Standard Editor Menu
		// Piano Editor Menu
		if(editmenu=menu->AddMenu(Cxs[CXS_EDIT],0))
		{
			maingui->AddUndoMenu(editmenu);

			class menu_selecttext:public guiMenu{
			public:
				menu_selecttext(Edit_Text *e,bool o){editor=e;on=o;}
				void MenuFunction(){editor->SelectAllTexts(on);}
				Edit_Text *editor;
				bool on;
			};
			editmenu->AddFMenu(Cxs[CXS_SELECTALLTEXTEVENTS],new menu_selecttext(this,true));
			editmenu->AddFMenu(Cxs[CXS_UNSELECTALLTEXTEVENTS],new menu_selecttext(this,false));

			class menu_deletetext:public guiMenu
			{
			public:
				menu_deletetext(Edit_Text *e){editor=e;}
				void MenuFunction(){editor->DeleteTexts();}
				Edit_Text *editor;
			};
			editmenu->AddFMenu(Cxs[CXS_DELETE],new menu_deletetext(this),SK_DEL);

			AddStandardGotoMenu();
		}

		guiMenu *n=menu->AddMenu("Editor",0);
		if(n)
		{	
			CreateEditorMenu(n);
			AddStepMenu(n);
		}
	}

	maingui->AddCascadeMenu(this,menu);
#endif

	return menu;
}

void Edit_Text::InitGadgets()
{
#ifdef OLDIE
	if(!(winmode&WINDOWMODE_FRAMES))
	{
		ResetGadgets();

		if(height>100 && frame.position.ondisplay==true){

			SliderCo vert;

			vert.x=width-EDITOR_SLIDER_SIZE_VERT;
			vert.y=frame.position.y;
			vert.x2=width;
			vert.y2=height-EDITOR_SLIDER_SIZE_HORZ;

			vert.from=0;
			vert.horz=false;
			vert.to=textsineditor.GetCount();
			vert.pos=firsttext_index;
			vert.page=5;

			gadgetlists.DrawEditorSlider(this,&vert,SLIDER_VERT_ZOOM);

			if(vertgadget){

				/*
				if(guiGadgetList *gl=gadgetlists.AddGadgetList(this)){

					int x2=guitoolbox.x2+bitmap.prefertimebuttonsize;
					if(x2>vertgadget->x-1)
						x2=vertgadget->x-1;

					timebutton=gl->AddTimeButton(guitoolbox.x2,0,x2,0+maingui->GetFontSizeY(),WindowSong()->GetSongPosition(),GADGETID_EVENTEDITOR_TIMEBUTTON,MODE_BLACK,"Song Position");
					smptebutton=gl->AddSMPTEButton(guitoolbox.x2,0+maingui->GetFontSizeY()+1,x2,0+2*maingui->GetFontSizeY()+1,WindowSong()->GetSongPosition(),GADGETID_EVENTEDITOR_SMPTEBUTTON,MODE_BLACK,"Song Position SMPTE");
				}
				*/
			}
		}
	}
#endif

}

void Edit_Text_Text::Draw(bool single)
{
#ifdef OLDIE
	int iflag=NO_SHOWNUMBER;

	if(text->flag&OFLAG_SELECTED)
		iflag|=NO_SELECTED;

	int fillcolour;

	flag=text->flag;

	if(text->flag&OFLAG_SELECTED) // TEXT Selected
		fillcolour=COLOUR_SELECTED;
	else
		grey==true?fillcolour=COLOUR_GREY_LIGHT:fillcolour=COLOUR_WHITE;

	if(editor->frame.position.ondisplay==true)
		editor->guibuffer->guiFillRect(editor->frame.position.x+1,y,editor->frame.position.x2-1,y2,fillcolour);

	if(editor->frame.textname.ondisplay==true)
		editor->guibuffer->guiFillRect(editor->frame.textname.x+1,y,editor->frame.textname.x2-1,y2,fillcolour);	

	if(editor->frame.textstring.ondisplay==true)
		editor->guibuffer->guiFillRect(editor->frame.textstring.x+1,y,editor->frame.textstring.x2-1,y2,fillcolour);		

	// TEXT Position
	editor->CreatePos(text->GetTextStart());

	if(editor->frame.position.ondisplay==true)
	{
		// Minus+Hour
		{
			int hourpos=editor->displayusepos->pos[0];

			if(editor->displayusepos->minus==true)
				hourpos*=-1;

			editor->guibuffer->guiDrawNumberObject(editor->position_1000_x,y2,editor->position_0010_x,hourpos,&time_1000,iflag);
		}

		editor->guibuffer->guiDrawText(editor->position_0100_x-editor->spacesize,y2,editor->position_0010_x,editor->space,iflag);
		editor->guibuffer->guiDrawNumberObject(editor->position_0100_x,y2,editor->position_0010_x,editor->displayusepos->pos[1],&time_0100,iflag);

		if(editor->displayusepos->nozoom==false)
		{
			editor->guibuffer->guiDrawText(editor->position_0010_x-editor->spacesize,y2,editor->position_0001_x,editor->space,iflag);
			editor->guibuffer->guiDrawNumberObject(editor->position_0010_x,y2,editor->position_0001_x,editor->displayusepos->pos[2],&time_0010,iflag);
			editor->guibuffer->guiDrawText(editor->position_0001_x-editor->spacesize,y2,editor->frame.position.x2,editor->space,iflag);
		}
		else
		{
			editor->guibuffer->guiDrawText(editor->position_0010_x,y2,editor->position_0001_x,"-",iflag);
		}

		editor->guibuffer->guiDrawNumberObject(editor->position_0001_x,y2,editor->frame.position.x2,editor->displayusepos->GetPos3(editor->WindowSong()),&time_0001,iflag);

		if(editor->displayusepos->showquarterframe==true)
		{
			editor->guibuffer->guiDrawText(editor->position_0001QF_x-editor->spacesize,y2,editor->frame.position.x2,";",iflag);
			editor->guibuffer->guiDrawNumberObject(editor->position_0001QF_x,y2,editor->frame.position.x2,editor->displayusepos->pos[4],&time_sf,iflag);
		}

		/*
		if(refreshfont==true)
		editor->guibuffer->SetFont(&maingui->standardfont);
		*/
	}	

	if(editor->frame.textname.ondisplay==true)
		editor->guibuffer->guiDrawText(editor->frame.textname.x,y2-1,editor->frame.textname.x2,"Text",iflag);

	if(editor->frame.textstring.ondisplay==true)
		editor->guibuffer->guiDrawText(editor->frame.textstring.x,y2-1,editor->frame.textstring.x2,text->string,iflag);

	if(single==true){
		editor->BltGUIBuffer_Frame(&editor->frame.position,y,y2);
		editor->BltGUIBuffer_Frame(&editor->frame.textname,y,y2);
		editor->BltGUIBuffer_Frame(&editor->frame.textstring,y,y2);
	}

	set=true;
#endif

}



void Edit_Text::Init()
{
#ifdef OLDIE
	if(!(winmode&WINDOWMODE_FRAMES))
		FreeMemory();
	else
	{
		ToolTipOff();
		InitFrames();
	}

	if(guibuffer && width && height){
		if((winmode&WINDOWMODE_INIT) &&
			((winmode&(WINDOWMODE_RESIZE|WINDOWMODE_FRAMES))==0)
			)
			ShowMenu();

		if((winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE)) && (!(winmode&WINDOWMODE_FRAMES))){	

			InitFrames();
			RemoveAllSprites();

			guitoolbox.CreateToolBox(TOOLBOXTYPE_EDITOR);
			guictrlbox.CreateControlBox(this);
		}

		frame.textname.y=frame.textstring.y=frame.position.y=guictrlbox.GetY2(0)+maingui->GetFontSizeY()+2;
		frame.textname.y2=frame.textstring.y2=frame.position.y2=height;

		frame.textname.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,0);
		frame.textstring.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,0);
		frame.position.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,0);

		if(!(winmode&WINDOWMODE_FRAMES))
			InitGadgets();

		frame.textname.on=true;
		frame.textstring.on=true;
		frame.position.on=true;

		AddFrame(&frame.position,TEXT_FRAMEID_POSITION,GUIFRAME_RIGHT);
		AddFrame(&frame.textname,TEXT_FRAMEID_TEXT,GUIFRAME_RIGHT);
		AddFrame(&frame.textstring,TEXT_FRAMEID_TEXTSTRING,0);

		ShowTexts();
		ShowCursorText();
	}
#endif

}

void Edit_Text::FreeMemory()
{
	textsineditor.DeleteAllO();


		//ResetAllGadgets();

}

// Edit Number ...
bool Edit_Text::EditNumberObject_Text(Edit_Text_Text *eee,NumberObject *no,Seq_Text *text)
{
#ifdef OLDIE
	bool edit=false;

	// Time
	switch(displayusepos->mode)
	{
	case Seq_Pos::POSMODE_TIME: // std-min-sec
	case Seq_Pos::POSMODE_SMPTE_24: // std-min-sec-frame.subframe
	case Seq_Pos::POSMODE_SMPTE_25: // std-min-sec-frame.subframe
		case Seq_Pos::POSMODE_SMPTE_48: // std-min-sec-frame.subframe
	case Seq_Pos::POSMODE_SMPTE_50: // std-min-sec-frame.subframe
	case Seq_Pos::POSMODE_SMPTE_2997: // std-min-sec-frame.subframe
	case Seq_Pos::POSMODE_SMPTE_30: // std-min-sec-frame.subframe
		//case Seq_Pos::POSMODE_SMPTE_2997DF: // std-min-sec-frame.subframe
		//case Seq_Pos::POSMODE_SMPTE_30DF: // std-min-sec-frame.subframe
		{
			if(no==&eee->time_1000 || no==&eee->time_0100 || no==&eee->time_0010 || no==&eee->time_0001 || no==&eee->time_sf)
			{
				WindowSong()->timetrack.ConvertTicksToPos(text->GetTextStart(),displayusepos);

				if(no==&eee->time_1000)
					displayusepos->AddHour(editnumber_diffy<0?1:-1);
				else
					if(no==&eee->time_0100)
						displayusepos->AddMin(editnumber_diffy<0?1:-1);
					else
						if(no==&eee->time_0010)
							displayusepos->AddSec(editnumber_diffy<0?1:-1);
						else
							if(no==&eee->time_0001)
								displayusepos->AddFrame(editnumber_diffy<0?1:-1);
							else
								if(no==&eee->time_sf)
									displayusepos->AddQuarterFrame(editnumber_diffy<0?1:-1);

				OSTART h=WindowSong()->timetrack.ConvertPosToTicks(displayusepos);

				if(h>=0 && h!=text->GetTextStart())
				{
					text->MoveTextAbs(h);
					editnumberobjectflag|=EO_TIMECHANGE;
				}

				return false; // no playit
			}
		}
		break;

	case Seq_Pos::POSMODE_NORMAL: case Seq_Pos::POSMODE_COMPRESS:
		{
			if(no==&eee->time_1000 || no==&eee->time_0100 || no==&eee->time_0010 || no==&eee->time_0001)
			{
				WindowSong()->timetrack.ConvertTicksToPos(text->GetTextStart(),displayusepos);

				if(no==&eee->time_1000)
					displayusepos->AddMeasure(editnumber_diffy<0?1:-1);
				else
					if(no==&eee->time_0100)
						displayusepos->AddBeat(editnumber_diffy<0?1:-1);
					else
						if(no==&eee->time_0010)
							displayusepos->AddZoomTicks(editnumber_diffy<0?1:-1);
						else
							if(no==&eee->time_0001)
								displayusepos->AddTicks(editnumber_diffy<0?1:-1);

				OSTART h=WindowSong()->timetrack.ConvertPosToTicks(displayusepos);

				if(h>=0 && h!=text->GetTextStart())
				{
					text->MoveTextAbs(h);
					editnumberobjectflag|=EO_TIMECHANGE;
				}

				return false; // no playit
			}

		}break;
	}

	return edit;
#endif

	return false;
}

void Edit_Text::ShowAllTextTexts()
{
#ifdef OLDIE
	if(!activenumberobject)
	{
		ShowAllText();
		return;
	}

	Edit_Text_Text *e=FirstText();

	while(e)
	{
		e->Draw();
		e=e->NextText();
	}

	BltGUIBuffer_Frame(&frame.position);
	BltGUIBuffer_Frame(&frame.textname);
	BltGUIBuffer_Frame(&frame.textstring);
#endif

}


bool Edit_Text::InitEdit(Seq_Event *seqevent)
{
	if(!seqevent)return false;

	if(!GetUndoEditEvents())
	{
		if(seqevent->IsSelected()==false)
		{
			//if(maingui->GetShiftKey()==false && maingui->GetCtrlKey()==false)
			//	SelectAllTempos(false);

			seqevent->flag |=OFLAG_SELECTED;
		}

		int c=0;
		Seq_Text *t=WindowSong()->textandmarker.FirstText();

		while(t){
			if(t->flag&OFLAG_SELECTED)c++;
			t=t->NextText();
		}

		if(c)
		{
			TRACE ("Selected Txt %d\n",c);

			if(UndoEditText *uet=new UndoEditText)
			{
				// Buffer Tempo Events+Data
				if(uet->Init(c)==true)
				{
					int i=0;
					t=WindowSong()->textandmarker.FirstText();

					while(t){

						if(t->flag&OFLAG_SELECTED)
						{
							uet->event_p[i]=t;
							uet->oldindex[i]=t->GetIndex();
							uet->oldevents[i++]=(Seq_Event *)t->Clone(0);
						}

						t=t->NextText();
					}

					SetUndoEditEvents(uet);

					return true;
				}

				delete uet;
			}
		}
	}		

	return false;
}

void Edit_Text::ClickOnNumberObject(NumberObject *obj)
{
}

void Edit_Text::EditNumberObject(NumberObject *obj,int flag)
{
#ifdef OLDIE
	Edit_Text_Text *ett=FirstText();

	while(ett)
	{
		if(ett->FindNumberObject(obj))
		{
			if(!GetUndoEditEvents())
			{
				if(InitEdit(ett->text)==false)
					return;
			}

			bool refreshgui=false;

			editnumberobjectflag=0;

			Seq_Text *t=WindowSong()->textandmarker.FirstText();

			while(t)
			{
				if(t->flag&OFLAG_SELECTED)
				{
					bool changed=EditNumberObject_Text(ett,obj,t);

					if(changed==true || editnumberobjectflag!=0)
						refreshgui=true;
				}

				t=t->NextText();
			}

			if(refreshgui==true)
				maingui->RefreshTextGUI(song);

			return;
		}

		ett=ett->NextText();
	}

#endif

}

void Edit_Text::EditNumberObjectReleased(NumberObject *obj,int flag)
{
	ReleaseEdit();
}

UndoFunction *UndoEditText::CreateUndoFunction()
{
	return new Undo_EditText(this);
}

