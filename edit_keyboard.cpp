#include "keyboard.h"

#include "object_song.h"
#include "MIDIhardware.h"
#include "MIDIinproc.h"
#include "camxgadgets.h"
#include "globalmenus.h"
#include "languagefiles.h"
#include "semapores.h"
#include "songmain.h"
#include "chunks.h"
#include "gui.h"
#include "camxfile.h"

#define CAMX_DEFAULTKEYMAPAUTOLOADNAME "Preferences\\Keymaps\\defaultmap.cxks"

enum
{
	GADGETID_REC,
	GADGETID_SCROLL,
	GADGETID_ZOOM,
	GADGETID_HOLD,
	GADGETID_KEYS,GADGETID_KEYSSLIDERHORZ,

	GADGETID_CHANNEL_I,
	GADGETID_CHANNEL,
	GADGETID_TRANSPOSE_I,
	GADGETID_TRANSPOSE,
	GADGETID_VELOCITY_I,
	GADGETID_VELOCITY
};

static int startkeys[]= // 68
{
	0,2,4,5,7,8,11,
	12,14,16,17,19,21,23,
	24,26,28,29,31,33,35,
	36,38,40,41,43,45,47,
	48,50,52,53,55,57,59,
	60,62,64,65,67,69,71,
	72,74,76,77,79,81,83,
	84,86,88,89,91,93,95,
	96,98,100,101,103,107,
	108,110,112,113,115,117,119,
	120,122,134,125,127
};

void Keyboardmap::Load(camxFile *f)
{
	f->LoadChunk();

	if(f->GetChunkHeader()==CHUNK_KEYMAP)
	{
		f->ChunkFound();

		for(int i=0;i<MAXKEYBOARDKEYS;i++)
		{
			f->ReadChunk(&keys[i].key);
			f->ReadChunk(&keys[i].used);
		}

		f->CloseReadChunk();
	}
}

void Keyboardmap::Save(camxFile *f)
{
	f->OpenChunk(CHUNK_KEYMAP);

	for(int i=0;i<MAXKEYBOARDKEYS;i++)
	{
		f->Save_Chunk(keys[i].key);
		f->Save_Chunk(keys[i].used);
	}

	f->CloseChunk();
}

void Keyboardmap::ResetDefault()
{
	for(int i=0;i<MAXKEYBOARDKEYS;i++)
		keys[i].used=false;

	char octave=4;

	keys[0+12*(octave+1)].key='q'; // c
	keys[0+12*(octave+1)].used=true;

	keys[1+12*(octave+1)].key='2';
	keys[1+12*(octave+1)].used=true;

	keys[2+12*(octave+1)].key='w'; //d
	keys[2+12*(octave+1)].used=true;

	keys[3+12*(octave+1)].key='3';
	keys[3+12*(octave+1)].used=true;

	keys[4+12*(octave+1)].key='e'; //e
	keys[4+12*(octave+1)].used=true;

	keys[5+12*(octave+1)].key='r'; //f
	keys[5+12*(octave+1)].used=true;

	keys[6+12*(octave+1)].key='5';
	keys[6+12*(octave+1)].used=true;

	keys[7+12*(octave+1)].key='t'; // g
	keys[7+12*(octave+1)].used=true;

	keys[8+12*(octave+1)].key='6'; 
	keys[8+12*(octave+1)].used=true;

	keys[9+12*(octave+1)].key='z'; //h
	keys[9+12*(octave+1)].used=true;

	keys[10+12*(octave+1)].key='7'; 
	keys[10+12*(octave+1)].used=true;

	keys[11+12*(octave+1)].key='u';
	keys[11+12*(octave+1)].used=true;


	// 2 ----------------

	/*
	keys[12].key='y'; // c
	keys[12].note=0+12*octave;
	keys[12].used=true;

	keys[13].key='s';
	keys[13].note=1+12*octave;
	keys[13].used=true;

	keys[14].key='x'; //d
	keys[14].note=2+12*octave;
	keys[14].used=true;

	keys[15].key='d';
	keys[15].note=3+12*octave;
	keys[15].used=true;

	keys[16].key='c'; //e
	keys[16].note=4+12*octave;
	keys[16].used=true;

	keys[17].key='v'; //f
	keys[17].note=5+12*octave;
	keys[17].used=true;

	keys[18].key='g';
	keys[18].note=6+12*octave;
	keys[18].used=true;

	keys[19].key='b'; // g
	keys[19].note=7+12*octave;
	keys[19].used=true;

	keys[20].key='h'; 
	keys[20].note=8+12*octave;
	keys[20].used=true;

	keys[21].key='n'; //h
	keys[21].note=9+12*octave;
	keys[21].used=true;

	keys[22].key='j'; 
	keys[22].note=10+12*octave;
	keys[22].used=true;

	keys[23].key='m'; 
	keys[23].note=11+12*octave;
	keys[23].used=true;

	keys[24].used=false;
	*/
}

bool Edit_Keyboard::CheckKeyDown()
{
	if(nVirtKey>='A' && nVirtKey<='Z')
		nVirtKey+=32;

	bool found=false;

	if(edit==true)
	{
		int k=FindKeyUnderMouse();

		if(k>=0 && (map.keys[k].key!=nVirtKey || map.keys[k].used==false || hold==true))
		{
			if(hold==false)
			{
				//CheckNoteOff();

				for(int i=0;i<MAXKEYBOARDKEYS;i++)
					if(map.keys[i].key==nVirtKey)
						map.keys[i].used=false;

				map.keys[k].key=nVirtKey;
				map.keys[k].used=true;

				ShowKeys();
			}
		}
	}

	Seq_Song *song=mainvar->GetActiveSong();

	if(song && song->GetFocusTrack())
	{
		// Note On
		for(int i=0;i<MAXKEYBOARDKEYS;i++)
		{
			if(map.keys[i].key==nVirtKey)
				found=true;

			if(map.keys[i].used==true && map.keys[i].key==nVirtKey)
			{
				if(hold==true)
				{
					if(map.keys[i].counter==0)
						PlayKey(i,FLAG_KEYBOARD_KEY);
					else
					{
						SendNoteOff(i,FLAG_MOUSE_KEY|FLAG_KEYBOARD_KEY,true);
					}
				}
				else
					PlayKey(i,FLAG_KEYBOARD_KEY);
				break;
			}
		}
	}

	return found;
}

void Edit_Keyboard::KeyDown()
{
	if(CheckKeyDown()==false){

		// Send Key to all other Keyboard Windows
		guiWindow *w=maingui->FirstWindow();
		while(w)
		{
			if(w!=this)
			{
				if(w->GetEditorID()==EDITORTYPE_KEYBOARD)
				{
					Edit_Keyboard *kb=(Edit_Keyboard *)w;
					kb->CheckKeyDown();
				}
			}

			w=w->NextWindow();
		}
	}
}

void Edit_Keyboard::SendNoteOff(int key,int flag,bool force)
{
	Seq_Song *song=mainvar->GetActiveSong();

	if(!song)
		return;

	if(map.keys[key].counter && map.keys[key].flag&flag)
	{
		if(hold==true && force==false)
		{
			map.keys[key].hold=true;
			return;
		}

		NewEventData input;

		input.fromdev=mainMIDI->keyboard_inputdevice;

		Note note;

		note.ostart=song->GetSongPosition();
		note.off.ostart=-1; // no auto off

		note.status=NOTEON;
		note.key=map.keys[key].playedkey;
		note.velocity=0; // Velocity 0=Off
		note.velocityoff=0;

		mainMIDI->CheckNewEventData(song,&input,&note);

		if(mainMIDI->keyboard_inputdevice)
			mainMIDI->keyboard_inputdevice->AddMonitor(note.status,note.key,note.velocity);

		map.keys[key].counter--;
		map.keys[key].hold=false;
	}
}

bool Edit_Keyboard::CheckNoteOff()
{
	bool found=false;

	// Note Ons
	for(int i=0;i<MAXKEYBOARDKEYS;i++)
	{
		if(map.keys[i].key==nVirtKey)
			found=true;

		if(map.keys[i].used==true && map.keys[i].key==nVirtKey)
		{
			if(map.keys[i].counter)
				SendNoteOff(i,FLAG_KEYBOARD_KEY);

			break;
		}
	}

	return found;
}

bool Edit_Keyboard::CheckKeyUp()
{
	if(nVirtKey>='A' && nVirtKey<='Z')
		nVirtKey+=32;

	return CheckNoteOff();
}

void Edit_Keyboard::KeyUp()
{
	if(CheckKeyUp()==false)
	{
		// Send Key to all other Keyboard Windows
		guiWindow *w=maingui->FirstWindow();
		while(w)
		{
			if(w!=this)
			{
				if(w->GetEditorID()==EDITORTYPE_KEYBOARD)
				{
					Edit_Keyboard *kb=(Edit_Keyboard *)w;
					kb->CheckKeyUp();
				}
			}

			w=w->NextWindow();
		}
	}
}

guiMenu *Edit_Keyboard::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		class menu_load:public guiMenu
		{
		public:
			menu_load(Edit_Keyboard *k)
			{
				editor=k;
			}

			void MenuFunction(){

				editor->LoadMap();
			}

			Edit_Keyboard *editor;
		};

		class menu_save:public guiMenu
		{
		public:
			menu_save(Edit_Keyboard *k)
			{
				editor=k;
			}

			void MenuFunction(){
				editor->SaveMap();
			}
			Edit_Keyboard *editor;
		};

		class menu_autoload:public guiMenu
		{
		public:
			menu_autoload(Edit_Keyboard *k)
			{
				editor=k;
			}

			void MenuFunction(){

				editor->LoadAutoMap(false);
			}

			Edit_Keyboard *editor;
		};

		class menu_autosave:public guiMenu
		{
		public:
			menu_autosave(Edit_Keyboard *k)
			{
				editor=k;
			}

			void MenuFunction(){
				editor->SaveAutoMap();
			}
			Edit_Keyboard *editor;
		};

		guiMenu *s=menu->AddMenu(Cxs[CXS_FILE],0);

		if(s){
			s->AddFMenu("Load Key Map",new menu_load(this));
			s->AddFMenu("Save Key Map",new menu_save(this));

			s->AddLine();
			s->AddFMenu("Load Autoload Map",new menu_autoload(this));
			s->AddFMenu("Save this Key Map as Autoload Map",new menu_autosave(this));
		}

		class menu_filter:public guiMenu
		{
		public:
			menu_filter(Edit_Keyboard *k,int chl)
			{
				editor=k;
				channel=chl;
			}

			void MenuFunction()
			{
				if(channel==100)
				{
					// Reset
					for (int i=0;i<16;i++)
						editor->filter.channel|=1<<i;
				}
				else
					if(channel==101)
					{
						// Toggle

						for (int i=0;i<16;i++)
						{
							if(editor->filter.channel&(1<<i))
								editor->filter.channel CLEARBIT (1<<i);
							else
								editor->filter.channel |= (1<<i);
						}
					}
					else
					{
						if(editor->filter.channel&(1<<channel))
							editor->filter.channel CLEARBIT (1<<channel);
						else
							editor->filter.channel |= (1<<channel);
					}

					editor->ShowFilter();
			} //

			Edit_Keyboard *editor;
			int channel; // 0-15
		};

		filtermenu=menu->AddMenu("Filter",0);

		if(filtermenu)
		{
			for(int i=0;i<16;i++)
			{
				char h[100],h2[NUMBERSTRINGLEN];

				strcpy(h,"MIDI Channel:");

				mainvar->AddString(h,mainvar->ConvertIntToChar(i+1,h2));

				filtermenu->AddFMenu(h,new menu_filter(this,i),(filter.channel&(1<<i))?true:false);
			}

			filtermenu->AddLine();

			filtermenu->AddFMenu("Reset",new menu_filter(this,100)); // Reset
			filtermenu->AddFMenu("Toggle",new menu_filter(this,101)); // Toggle
		}

		s=menu->AddMenu("Editor",0);

		if(s)
		{
			s->AddFMenu(("Keyboard"),new globmenu_Keyboard);
		}
	}

	return menu;
}

void Edit_Keyboard::SendOpenKeys(int flag)
{
	for(int i=0;i<MAXKEYBOARDKEYS;i++)
		SendNoteOff(i,flag,true);
}

void Edit_Keyboard::ClearKeyUnderMouse()
{
	/*
	if(guibuffer && clearkey>=0)
	{
	if(clearkey>=startkey && clearkey<lastkey)
	{
	keyactive[clearkey]=false;

	guibuffer->guiFill(
	keyposx[clearkey]+1,
	keyposy2[clearkey]-1,
	keyblack[clearkey]==true?COLOUR_BLACK:COLOUR_WHITE
	);

	BltGUIBuffer(
	keyposx[clearkey],
	keyposy[clearkey],
	keyposx2[clearkey],
	keyposy2[clearkey]
	);
	}

	clearkey=-1;
	}
	*/
}

void Edit_Keyboard::RefreshRealtime()
{
	Seq_Song *song=mainvar->GetActiveSong();

	if(keys && song)
	{
		bool noteopen=song->FindOpenNote(-1,&filter);
		int changed=0;

		if(notetype!=song->notetype)
		{
			notetype=song->notetype;
			changed++;
		}


		// Piant Piano Keys
		for(int i=startkey;i<lastkey;i++)
		{
			int c=0;
			bool invert=false;

			if(keyundermouse==i)
			{
				c=COLOUR_GREEN;
				invert=true;
			}

			if(keyactive[i]==false) // Check
			{
				if(song->mastering==false)
				{
					if(map.keys[i].counter || (noteopen==true && song->FindOpenNote(i,&filter)==true))
					{
						keyactive[i]=true;
						invert=true;
					}
				}
			}
			else
			{
				if(map.keys[i].counter==0 && song->FindOpenNote(i)==false)
				{
					keyactive[i]=false;
					invert=true;
				}
			}

			if(invert==true)
			{
				if(keyactive[i]==true)
				{
					c=COLOUR_GREY;
					/*
					if(keyblack[i]==true)
					c=COLOUR_BLUE;
					else
					c=COLOUR_BLUE_LIGHT;
					*/
				}
				else
				{
					if(keyundermouse==i)
						c=COLOUR_GREEN;
					else
					{
						if(keyblack[i]==true)
							c=COLOUR_BLACK;
						else
							c=COLOUR_WHITE;
					}
				}

				//keys->gbitmap.guiFill(keyposx[i]+1,keyposy[i]+1,c);
				changed++;

				/*
				BltGUIBuffer(
				keyposx[i],
				keyposy[i],
				keyposx2[i],
				keyposy2[i]
				);
				*/
			}
		}

		if(changed)
			DrawDBBlit(keys);
		//keys->Blt();
	}
}

void Edit_Keyboard::DeActivated()
{
	if(hold==false)
		SendOpenKeys(FLAG_KEYBOARD_KEY|FLAG_MOUSE_KEY);

	//ReleaseMouseKeys(hold==true?false:true);
}

void Edit_Keyboard::ShowFilter()
{
	if(menu && filtermenu)
	{
		for(int i=0;i<16;i++)
		{
			filtermenu->Select(i,filter.channel&(1<<i)?true:false);
		}
	}
}

/*
void Edit_Keyboard::ReleaseMouseKeys(int flag,bool sendopennotes)
{
for(int i=startkey;i<lastkey;i++)
{
if(sendopennotes==true)
SendNoteOff(i,flag);

mousekeyactive[i]=false;
}
}
*/

int Edit_Keyboard::FindKeyUnderMouse()
{
	int found=-1;

	if(keys)
	{
		int mx=keys->SetToXX2(keys->GetMouseX());
		int my=keys->SetToYY2(keys->GetMouseY());

		//	TRACE ("Mouse %d %d\n",mx,my);

		for(int i=startkey;i<lastkey;i++)
		{

			//TRACE ("KP %d %d -  X - %d # %d - Y - %d\n",i, keyposx[i],keyposx2[i],keyposy[i],keyposy2[i]);

			if(keyposx[i]<=mx && keyposx2[i]>=mx &&
				keyposy[i]<=my &&  keyposy2[i]>=my)
			{
				found=i;

				if(keyblack[i]==true)
					return i;
			}
		}	
	}

	return found;
}

void Edit_Keyboard::PlayKey(int key,int flag)
{
	Seq_Song *song=mainvar->GetActiveSong();

	if(!song)
		return;

	if(key>=0 && key<MAXKEYBOARDKEYS)
	{
		if(map.keys[key].counter==0)
		{
			//if(hold==false && this->left_mousekey==MOUSEKEY_DOWN)
			//	ReleaseMouseKeys();

			mousekeyactive[key]=true;

			Note note;

			note.ostart=song->GetSongPosition();
			note.off.ostart=-1; // no auto off

			int playkey=key;

			playkey+=transpose;
			if(playkey<0)
				playkey=0;
			else
				if(playkey>=MAXKEYBOARDKEYS)
					playkey=MAXKEYBOARDKEYS-1;

			note.status=NOTEON|(channel-1); // Chl 1
			note.key=(UBYTE)playkey;
			note.velocity=velocity; // Velocity
			note.velocityoff=0;
			note.pattern=0;

			// Thru
			if(mainMIDI->keyboard_inputdevice)
			{
				NewEventData input;

				input.fromdev=mainMIDI->keyboard_inputdevice;
				mainMIDI->keyboard_inputdevice->AddMonitor(note.status,note.key,note.velocity);
				mainMIDI->CheckNewEventData(song,&input,&note); // record possible

				if(keys && (flag&FLAG_KEYBOARD_KEY))
					keys->SetKeyFocus();
			}

			map.keys[key].playedkey=note.key;
			map.keys[key].counter++;
			map.keys[key].flag=flag;
		}
	}
}

void Edit_Keyboard::PlayMouseKey(bool mousemove)
{
	int key=FindKeyUnderMouse();

	if(key==-1)
	{
		if(hold==false)
			SendOpenKeys(FLAG_MOUSE_KEY);

		return;
	}

	if(hold==true)
	{
		if(map.keys[key].counter==0)
			PlayKey(key,FLAG_MOUSE_KEY);
		else
		{
			SendNoteOff(key,FLAG_MOUSE_KEY|FLAG_KEYBOARD_KEY,true);
		}
	}
	else
		if(map.keys[key].counter==0)
		{
			if(mousemove==true)
				SendOpenKeys(FLAG_MOUSE_KEY);

			PlayKey(key,FLAG_MOUSE_KEY);
		}
}

int Edit_Keyboard::LongToKeylist(int k)
{
	for(int i=0;i<68;i++)
		if(startkeys[i]==k)
			return i;

	return 0;
}

void Keys_Callback(guiGadget_CW *g,int status)
{
	Edit_Keyboard *kb=(Edit_Keyboard *)g->from;

	switch(status)
	{
	case DB_CREATE:
		kb->keys=g;
		break;

	case DB_PAINT:
		{
			kb->ShowKeys();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		{
			kb->PlayMouseKey((status&DB_MOUSEMOVE)?true:false);
		}
		break;

	case DB_LEFTMOUSEUP:
		if(kb->hold==false)
			kb->SendOpenKeys(FLAG_MOUSE_KEY);
		break;

	case DB_KILLFOCUS:
		if(kb->hold==false)
			kb->SendOpenKeys(FLAG_KEYBOARD_KEY|FLAG_MOUSE_KEY);
		break;

	case DB_KEYDOWN:
		{
			kb->SetKey(g->vkey);
			kb->KeyDown();
		}
		break;

	case DB_KEYUP:
		{
			kb->SetKey(g->vkey);
			kb->KeyUp();
		}
		break;

	}
}

void Edit_Keyboard::ScrollHoriz()
{
	int pos=glist.scrollh_pos;

	if(startkeys[pos]!=startkey)
	{
		startkey=startkeys[pos];
		ShowKeys();

		if(keys)
			keys->Blt();
	}
}

void Edit_Keyboard::Init()
{
	guiSetWindowText("Keyboard");

	zoomy=15;

	glist.SelectForm(0,0);

	recgadget=glist.AddButton(-1,-1,6*maingui->GetFontSizeY(),-1,0,GADGETID_REC,0,Cxs[CXS_PLAYOREDIT]);
	glist.AddLX();

	if(recgadget)
		ShowEditMode();

	holdgadget=glist.AddButton(-1,-1,6*maingui->GetFontSizeY(),-1,"Hold",GADGETID_HOLD,MODE_TOGGLE);
	glist.AddLX();
	if(holdgadget)
		ShowHoldMode();

	int w=bitmap.GetTextWidth(" * Transpose * ");

	glist.AddButton(-1,-1,w,-1,"Channel",GADGETID_CHANNEL_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();

	glist.AddNumberButton(-1,-1,3*maingui->GetFontSizeY(),-1,GADGETID_CHANNEL,1,16,channel,NUMBER_INTEGER);

	glist.AddLX();
	glist.AddButton(-1,-1,w,-1,"Transpose",GADGETID_TRANSPOSE_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,3*maingui->GetFontSizeY(),-1,GADGETID_TRANSPOSE,-127,127,transpose,NUMBER_INTEGER);

	glist.AddLX();
	glist.AddButton(-1,-1,w,-1,"Velocity",GADGETID_VELOCITY_I,MODE_NOMOUSEOVER|MODE_ADDDPOINT);
	glist.AddLX();
	glist.AddNumberButton(-1,-1,3*maingui->GetFontSizeY(),-1,GADGETID_VELOCITY,1,127,velocity,NUMBER_INTEGER);

	glist.Return();

	keys=glist.AddChildWindow(-1,-1,-1,-2,MODE_LEFT|MODE_RIGHT|MODE_BOTTOM,0,&Keys_Callback,this);
	glist.Return();

	glist.SetHorzScroll(LongToKeylist(startkey),68);

	notetype=mainvar->GetActiveSong()?mainvar->GetActiveSong()->notetype:mainsettings->defaultnotetype;
}

Edit_Keyboard::Edit_Keyboard()
{
	editorid=EDITORTYPE_KEYBOARD;

	InitForms(FORM_PLAIN1x1);
	resizeable=true;
	ondesktop=true;

	minheight=90;
	maxheight=12*maingui->GetFontSizeY();

	minwidth=150;

	horzscroll=true;

	startkey=60;
	filtermenu=0;
	clearkey=keyundermouse=-1;

	showkeyundermouse=false;

	for(int i=0;i<MAXKEYBOARDKEYS;i++)mousekeyactive[i]=keyactive[i]=false;
	zoomc=18;
	edit=false;

	LoadAutoMap(true);
	dialogstyle=true;
	hold=false;

	channel=1;
	transpose=0;
	velocity=127;
}

void Edit_Keyboard::LoadMap()
{
	camxFile sfile;

	if(sfile.OpenFileRequester(0,this,"Load Keyboard Settings File","Keyboard Settings (*.cxks)|*.cxks;|All Files (*.*)|*.*||",true)==true)
	{
		if(sfile.OpenRead(sfile.filereqname)==true)
		{
			char check[4];

			sfile.Read(check,4);

			if(mainvar->CompareStringWithOutZero(check,"CAMX")==true)
			{
				sfile.Read(check,4);

				if(mainvar->CompareStringWithOutZero(check,"KBST")==true)
				{
					int version=0;

					sfile.Read(&version);
					map.Load(&sfile);

					ShowKeys();
				}
			}
		}

		sfile.Close(true);
	}
}

void Edit_Keyboard::SaveMap()
{
	camxFile sfile;

	if(sfile.OpenFileRequester(0,this,"Save Keyboard Settings File","Keyboard Settings (*.cxks)|*.cxks;|All Files (*.*)|*.*||",false)==true)
	{
		sfile.AddToFileName(".cxks");

		if(sfile.OpenSave(sfile.filereqname)==true)
		{
			int version=maingui->GetVersion();

			// Header
			sfile.Save("CAMX",4);
			sfile.Save("KBST",4);

			sfile.Save(&version,sizeof(int)); // Version

			map.Save(&sfile);
		}

		sfile.Close(true);
	}
}

void Edit_Keyboard::LoadAutoMap(bool nomessage)
{
	camxFile sfile;

	if(sfile.OpenRead(CAMX_DEFAULTKEYMAPAUTOLOADNAME)==true)
	{
		char check[4];
		sfile.Read(check,4);

		if(mainvar->CompareStringWithOutZero(check,"CAMX")==true)
		{
			sfile.Read(check,4);

			if(mainvar->CompareStringWithOutZero(check,"KBST")==true)
			{
				int version=0;
				sfile.Read(&version);

				map.Load(&sfile);
				ShowKeys();
			}
		}
	}
	else{

		if(nomessage==false)
			maingui->MessageBoxOk(0,"No Autoload Keymap found");
	}

	sfile.Close(true);
}

void Edit_Keyboard::SaveAutoMap()
{
	camxFile sfile;

	if(sfile.OpenSave(CAMX_DEFAULTKEYMAPAUTOLOADNAME)==true)
	{
		int version=maingui->GetVersion();

		// Header
		sfile.Save("CAMX",4);
		sfile.Save("KBST",4);

		sfile.Save(&version,sizeof(int)); // Version

		map.Save(&sfile);
	}
	//else
	//	maingui->MessageBoxOk(0,"Unable to save to Autoload Keymap");

	sfile.Close(true);
}

void Edit_Keyboard::KillFocus()
{	
	//SendOpenKeys(FLAG_KEYBOARD_KEY|FLAG_MOUSE_KEY);
}

void Edit_Keyboard::MouseWheel(int delta,guiGadget *db)
{
	int sk=startkey-delta;

	if(sk<0)
		sk=0;
	else
		if(sk>127)
			sk=127;

	if(sk!=startkey){
		startkey=sk;
		ShowKeys();
	}
}

void Edit_Keyboard::MouseMove(bool inside)
{
	/*
	if(frame_keys.ondisplay==true){

	if(frame_keys.CheckIfInFrame(GetMouseX(),GetMouseY())==true){

	if(inside==false){

	if(keyundermouse!=1){
	clearkey=keyundermouse;
	keyundermouse=-1;
	ClearKeyUnderMouse();
	}
	}
	else{
	int k=FindKeyUnderMouse();

	if(k!=keyundermouse){
	if(clearkey==-1)
	clearkey=keyundermouse;
	ClearKeyUnderMouse();
	keyundermouse=k;
	}

	if(left_mousekey==MOUSEKEY_DOWN && hold==false){
	PlayMouseKey(true);
	}
	}
	}
	else
	{	
	if(keyundermouse!=-1)
	clearkey=keyundermouse;

	keyundermouse=-1;

	ClearKeyUnderMouse();

	if(hold==false)
	SendOpenKeys(FLAG_MOUSE_KEY);

	//	ReleaseMouseKeys(FLAG_MOUSE_KEY);
	}
	}
	*/
}

void Edit_Keyboard::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_CHANNEL:
		channel=g->GetPos();
		break;

	case GADGETID_TRANSPOSE:
		transpose=g->GetPos();
		break;

	case GADGETID_VELOCITY:
		velocity=g->GetPos();
		break;

	case GADGETID_HOLD:
		{
			hold=hold==true?false:true;
			ShowHoldMode();
			SendOpenKeys(FLAG_KEYBOARD_KEY|FLAG_MOUSE_KEY);
		}
		break;

	case GADGETID_REC:
		{
			edit=edit==true?false:true;
			ShowEditMode();
		}
		break;

	case GADGETID_ZOOM:
		{
			if(g->GetPos()!=zoomc)
			{
				zoomc=g->GetPos();
				ShowKeys();
			}
		}
		break;

		/*
		case GADGETID_KEYSSLIDERHORZ:
		{
		if(startkeys[g->GetPos()]!=startkey)
		{
		startkey=startkeys[g->GetPos()];
		ShowKeys();
		}
		}
		break;
		*/

	}
}

void Edit_Keyboard::DeInitWindow()
{
	SendOpenKeys(FLAG_KEYBOARD_KEY|FLAG_MOUSE_KEY);
}

void Edit_Keyboard::ShowEditMode()
{
	if(recgadget)
		recgadget->ChangeButtonText(Cxs[edit==true?CXS_EDIT:CXS_USE]);
}

void Edit_Keyboard::ShowHoldMode()
{
	if(holdgadget)
		holdgadget->Toggle(hold);
}

void Edit_Keyboard::ShowKeys()
{
	if(keys)
	{
		guiBitmap *bitmap=&keys->gbitmap;

		int blackleny,key,keyh,octave,x,hx,lastlinex=-1;
		double a;

		clearkey=keyundermouse=-1;

		blackleny=keys->GetY2();
		blackleny/=3;
		blackleny*=2;

		bitmap->guiFillRect(COLOUR_PIANOKEY);
		bitmap->guiDrawLineX(0,0,keys->GetX2(),COLOUR_BLACK_LIGHT);
		//bitmap->guiDrawLineY(0,keys->GetY2(),keys->GetX2(),keys->GetY2(),COLOUR_RED);

		// 0 = c-2 - c8

		// show black&white keys
		for(int i=0;i<2;i++)
		{
			x=0;
			numberofkeys=0;
			lastkey=key=startkey;

			while(x<keys->GetX2() && key<MAXKEYBOARDKEYS)
			{		
				hx=x+zoomc;
				numberofkeys++;

				//keyactive[key]=false;
				keyposx[key]=x; // x
				keyposx2[key]=hx; // x2

				octave=key/12;
				keyh=key-(12*octave);

				switch(keyh)
				{
				case 1:
				case 3:
				case 6:
				case 8:
				case 10:
					{
						if(i==1)
						{
							// black
							keyblack[key]=true;

							bitmap->guiFillRect(
								x,keyposy[key]=1,
								hx,keyposy2[key]=blackleny,
								COLOUR_BLACK);

							if(keyactive[key]==true)
								bitmap->guiFillRect(
								x+2,keyposy[key]+2,
								hx-1,keyposy2[key]-2,
								COLOUR_GREY_DARK);

							bitmap->guiDrawLineX(x,keyposy[key],keyposy2[key],COLOUR_GREY_LIGHT);
							bitmap->guiDrawLineY(keyposy[key],x,hx,COLOUR_GREY_LIGHT);
							//bitmap->guiDrawLine(hx,keyposy[key],hx,keyposy[key],COLOUR_GREY);
							bitmap->guiDrawLineY(keyposy2[key],x,hx,COLOUR_GREY);
						}
					}
					break;

				case 0:
				case 2:
				case 7:
				case 9:
				case 5:
					{
						if(i==0)
						{
							if(lastlinex!=-1)keyposx[key]=lastlinex;

							bool showkeytext=false;

							switch(keyh)
							{
							case 2:
								a=((double)zoomy)*0.33;
								break;

							case 7:
								a=((double)zoomy)*0.45;
								break;

							case 9:
								a=((double)zoomy)*0.25;
								break;

							case 0:
								{
									// c
									a=((double)zoomy)*0.62;
									showkeytext=true;
								}
								break;

							case 5:
								a=((double)zoomy)*0.70;
								break;
							}

							a=((double)hx)+a;

							keyposx2[key]=(int)a;

							lastlinex=keyposx2[key]; // x
							keyposy[key]=1; // y
							keyposy2[key]=keys->GetY2(); // y2

							if(keyactive[key]==true)
								bitmap->guiFillRect(
								keyposx[key]+1,keyposy[key]+1,
								keyposx2[key],keyposy2[key]-1,
								COLOUR_WHITE);

							if(key==127)
								bitmap->guiFillRect(keyposx2[key],0,width,keys->GetY2(),COLOUR_GREY);
							else
								bitmap->guiDrawLineX(keyposx2[key],blackleny,keyposy2[key],COLOUR_BLACK_LIGHT);

							bitmap->guiDrawLineY(keyposy2[key],keyposx[key]+1,keyposx2[key]-1,COLOUR_GREY_DARK);

							keyblack[key]=false;

							if(showkeytext==true && mainvar->GetActiveSong())
							{
								bitmap->SetTextColour(0,0,0);
								bitmap->guiDrawText(keyposx[key]+2,keys->GetY2()-2,keys->GetX2()-1,maingui->ByteToKeyString(mainvar->GetActiveSong(),key));
								showkeytext=false;
							}
						}
					}
					break;

				case 4:
				case 11:
					{
						if(i==0)
						{
							if(lastlinex!=-1)keyposx[key]=lastlinex;

							if(keyactive[key]==true)
								bitmap->guiFillRect(
								keyposx[key]+1,keyposy[key]+1,
								keyposx2[key],keyposy2[key]-1,
								COLOUR_GREY);

							bitmap->guiDrawLineX
								(
								lastlinex=hx,
								keyposy[key]=1,
								keyposy2[key]=keys->GetY2(),
								COLOUR_BLACK_LIGHT
								);

							bitmap->guiDrawLineY(keyposy2[key],keyposx[key]+1,keyposx2[key]-1,COLOUR_GREY_DARK);


							keyblack[key]=false;
						}
					}
					break;
				}

				if((i==0 && keyblack[key]==false) || (i==1 && keyblack[key]==true))
				{
					if(keyblack[key]==false)
					{
						char text[16];

						bitmap->SetTextColour(0xAA,0xAA,0xAA);
						bitmap->SetFont(&maingui->smallfont);

						bitmap->guiDrawText(x+1,keyposy[key]+1+maingui->GetFontSizeY(),hx-1,mainvar->ConvertIntToChar(key,text));
						bitmap->SetFont(&maingui->standardfont);
					}

					if(map.keys[key].used==true)
					{
						if(keyactive[key])
							bitmap->SetTextColour(0,0,0);
						else
						{
							if(keyblack[key]==false)
								bitmap->SetTextColour(139,87,66);
							else
								bitmap->SetTextColour(255,235,205);
						}

						char text[16];

						text[0]=map.keys[key].key;
						text[1]=0;

						int yh;

						if(keyblack[key]==true)
							yh=keyposy2[key]-3;
						else
							yh=keyposy2[key]-(3+maingui->GetFontSizeY());

						if(yh>keyposy[key]+maingui->GetFontSizeY()+2)
							bitmap->guiDrawTextCenter(keyposx[key]+1,yh-maingui->GetFontSizeY(),keyposx2[key]-1,yh,text);
					}
				}

				x=hx;
				key++;
				lastkey=key;
			}
		}

		//BltGUIBuffer_Frame(&frame_keys);

		// Show Slider
		if(horzgadget)
		{
			//	horzgadget->ChangeSlider(0,68,this->LongToKeylist(startkey));
			//vertgadget->ChangeSliderPage(numberofkeys);
		}
	}

}