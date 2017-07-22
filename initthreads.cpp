#include "defines.h"
#include "songmain.h"

#include "editbuffer.h"
#include "gui.h"
#include "MIDIhardware.h"
#include "MIDIinproc.h"
#include "MIDIoutproc.h"
#include "MIDIthruproc.h"
#include "mainhelpthread.h"
#include "audiothread.h"
#include "audiohardware.h"
#include "audiorecord.h"
#include "audioproc.h"
#include "audiorealtime.h"
#include "settings.h"
#include "rmg.h"
// #include "freeze.h"
#include "languagefiles.h"
#include "drummap.h"
#include "wavemap.h"
#include "semapores.h"
#include "MIDItimer.h"
#include "MIDIprocessor.h"
#include "audiothreads.h"
#include "object_track.h"

#include <stdio.h>

void Seq_Main::AskQuitMessage(bool save)
{
	// Audio File Operations 
	mainthreadcontrol->Lock(CS_audiowork);
	if(audioworkthread->FirstWork())
	{
		mainthreadcontrol->Unlock(CS_audiowork);
		maingui->MessageBoxOk(0,Cxs[CXS_PLEASEWAITAUDIOOP]);

		return;
	}

	mainthreadcontrol->Unlock(CS_audiowork);

	if(char *h=GenerateString(Cxs[save==true?CXS_EXITCAMX:CXS_EXITCAMX_DONTSAVE],"?"))
	{
		if(maingui->MessageBoxYesNo(0,h)==true)
		{
			saveonexit=save;
			SetExitProgram();
		}

		delete h;
	}
}

bool Seq_Main::CreateNewDirectory(char *name)
{
	if(!name)
		return false;

#ifdef WIN32
	if(CreateDirectory(name,0))
		return true;


	DWORD error=GetLastError();

	if(error==ERROR_ALREADY_EXISTS)
	{
		TRACE ("Create Directy EXISTS %s\n",name);
		return true;
	}

	TRACE ("Create Directory Error %s\n",name);

	// Error
	char h2[NUMBERSTRINGLEN];

	char *ec=mainvar->ConvertIntToChar(error,h2);

	if(char *h=GenerateString(Cxs[CXS_UNABLETOCREATEDIR],"\nErrCode:",ec,"\n",name))
	{
		maingui->MessageBoxOk(0,h);
		delete h;
	}

#endif

	return false;
}

int Seq_Main::GetDirectoryLengthFromFileString(char *string)
{
	/*
	if(string)
	{
	int i=strlen(string);

	if(i>0)
	{
	i--;

	char *h=&string[i];

	while(i--)
	{
	if(*h=='/' ||
	*h=='\' ||
	*h==':')
	return i;

	h--;
	}
	}
	}
	*/

	return 0;
}

bool Seq_Main::DeleteADirectory(char *dir)
{
	if(dir && strlen(dir)<MAX_PATH-2)
	{
		//	2. Using win32 API
		// Delete the current directory tempDir directory (subdirectories will be deleted)
		SHFILEOPSTRUCT FileOp;
		memset(&FileOp, 0, sizeof(SHFILEOPSTRUCT)); 

		FileOp.hNameMappings = NULL;
		FileOp.hwnd = NULL;
		FileOp.lpszProgressTitle = NULL;

		TRACE ("Delete Dir %s\n",dir);

		char pszFrom[MAX_PATH];

		memset(pszFrom,0,MAX_PATH);
		strcpy(pszFrom,dir);

		FileOp.pFrom = pszFrom; //"C:\\Documents and Settings\\kingkong\\My Documents\\CamX\\Projects\\demo\\Song 1";
		FileOp.pTo = NULL;
		FileOp.wFunc = FO_DELETE;
		FileOp.fFlags = FOF_NOCONFIRMATION; 

		SHFileOperation (& FileOp); 
	}

	return false;
}

bool Seq_Main::DeleteAFile(char *name)
{
	if(name)
	{
#ifdef WIN32
		return DeleteFile(name);
#endif
	}

	return false;
}

void Seq_Main::DeletePeakFile(char *name)
{
	if(char *peakname=mainvar->GenerateString(name))
	{
		// Replace . = _
		{
			char *r=peakname;
			size_t i=strlen(peakname);

			while(i--)
			{
				if(*r=='.')*r='_';
				r++;
			}
		}

		if(char *peakfilename=mainvar->GenerateString(peakname,PEAKFILENAME)){
			mainvar->DeleteAFile(peakfilename);
			delete peakfilename;
		}

		delete peakname;
	}
}

void GUI::MessageLoop()
{
#ifdef WIN32

	MSG Msg;
	BOOL bRet;

	while( (bRet = GetMessage( &Msg, NULL, 0, 0 )) != 0) // GetMessage = Peek+Wait
	{ 
		if(Msg.message == WM_KEYDOWN && Msg.wParam == VK_RETURN)
		{
			// Edit Data Focus
			HWND a=GetActiveWindow();

			guiWindow *win=ConvertSystemWindowToGUI(a);

			if(win && win->GetEditorID()==EDITORTYPE_EDITDATA)
				win->closeit=true;
		}
		else
		{
			if (bRet == -1)
			{
				// handle the error and possibly exit
			}
			else
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);

				guiWindow *win=FirstWindow();
				while(win)
				{
					if(win->closeit==true)
					{
						win->closeit=false; // Avoid dead loop
						SendMessage(win->hWnd,WM_CLOSE,0,0);
						win=FirstWindow();
					}
					else
						win=win->NextWindow();
				}
			}

			if(mainvar->exitprogram_flag==true || (mainvar->updateflag&UPDATE_FLAG_RESTART))
				break;

			CheckGUIMessages();
			mainvar->CheckAutoSave();
			mainaudio->DeleteUnusedPeaks();
		}
	}
#endif
}

#ifdef DEBUG
void GUI::CheckSongBuffer(Seq_Song *s)
{
	if(s)
	{
		Seq_Track *t=s->FirstTrack();

		while(t)
		{
			t->mix.CheckBuffer();
			t=t->NextTrack();
		}
	}
}
#endif

void Seq_Main::ExitProgram()
{
#ifdef DEBUG
	maingui->CheckSongBuffer(GetActiveSong());
#endif

	maingui->SaveSettings();
//	maingui->OpenInfoWindow(false);

	// Stop Active Song
	if(Seq_Song *song=GetActiveSong())
	{
		maingui->SetInfoWindowText("Stop Song. ..");
		song->StopSong(0,song->GetSongPosition());
	}

	mainthreadcontrol->LockActiveSong();
	exitthreads=true; // Set Exit Program Flag
	mainthreadcontrol->UnlockActiveSong();

	maingui->SetInfoWindowText("Close Audio Devices...");
	mainaudio->StopDevices(); // Stop Audio Engine

	maingui->SetInfoWindowText("Close All Windows...");

	maingui->CloseAllWindowsExecptInfo();

	maingui->SetInfoWindowText("Close Threads...");
	
	CloseAllThreads();

	maingui->SetInfoWindowText("Save Processor...");

	mainprocessor->Save(0);

	maingui->SetInfoWindowText("Save Device Programs...");
	mainsettings->SaveDevicePrograms(0);

	mainsettings->InitPrevProjects();

	maingui->SetInfoWindowText("Save Settings...");
	mainsettings->Save(0);

	//maingui->SetInfoWindowText("Save Database...");
	mainaudio->SaveDataBase();

	if(saveonexit==true)
	{
		maingui->SetInfoWindowText("Save Projects/Songs...");
		SaveAllProject();
	}

	StopAllHardware(); // close devices

	maingui->SetInfoWindowText("Close Projects/Songs...");
	CloseAllProjects();
	maingui->CloseAllAutoLoadSongs();

	maingui->SetInfoWindowText("Close Buffer...");
	mainbuffer->DeleteBuffer();

	maingui->SetInfoWindowText("Close Grooves...");
	mainMIDI->RemoveAllGrooves();

	/*
	if(maindrummap->FirstDrummap())
	{
	maingui->SetInfoWindowText("Close Drummaps...");
	maindrummap->RemoveAllDrummaps();
	}
	*/

	maingui->SetInfoWindowText("Close Wavemaps...");
	mainwavemap->DeleteAllWaveMaps();

	maingui->SetInfoWindowText("Close Synth Devices...");
	mainsettings->synthdevices.DeleteAllDevices();

	maingui->SetInfoWindowText("Close VST/Audio...");
	mainsettings->DeleteAllVSTDirectories();
	mainsettings->DeleteAllAudioDirectories();

	maingui->SetInfoWindowText("Close Processor...");
	mainprocessor->DeleteAllProcessor();

	maingui->SetInfoWindowText("Close Generator...");
	mainrmgmap->DeleteAllMaps();

	maingui->SetInfoWindowText("Close Memory Pools...");

	maingui->SetInfoWindowText("Close Hardware...");
	CloseAllHardware(); 

	maingui->SetInfoWindowText("Goodbye");

//	maingui->SetInfoWindowText("Exit: Close CamX Windows");

	maingui->CloseGUI();

	maintimer->Exit();
}

int Seq_Main::InitThreads()
{
	// MIDI
	int error=0;

	error+=MIDIinproc->StartThread();
	error+=plugininproc->StartThread();

	error+=mainMIDIalarmthread->StartThread();
	error+=MIDIrtealarmproc->StartThread();

	error+=MIDIoutproc->StartThread();

	error+=MIDIalarmprocessorproc->StartThread();
	error+=MIDImtcproc->StartThread();
	error+=MIDIstartthread->StartThread();

	error+=mainMIDIthruthread->StartThread();	
	error+=mainhelpthread->StartThread();
	error+=mainsyncthread->StartThread();

	//Audio
	maindevicerefillthread->StartThread();
	maindeviceinputthread->StartThread();

	if(audiorecordthread->Init()==false)error++;

	//	error+=audiorecordbufferthread->StartThread();
	error+=audiopeakthread->StartThread();
	error+=audioworkthread->StartThread();
	//error+=audiofreezethread->StartThread();

	error+=mainaudiostreamproc->StartThread();
	error+=mainaudioinproc->StartThread();
	error+=mainaudiorealtimethread->StartThread();

#ifdef CAMXGUIHTREADS
	error+maingui->winproc.StartThread();
#endif

	//FreeStringBuffer();
	return error;
}

void Seq_Main::CloseAllThreads()
{
	maingui->SetInfoWindowText("Close Thread MIDI In");
	MIDIinproc->StopThread();

	maingui->SetInfoWindowText("Close Thread VST In");
	plugininproc->StopThread();

	maingui->SetInfoWindowText("Close Thread MIDI Alarm");
	mainMIDIalarmthread->StopThread();

	maingui->SetInfoWindowText("Close Thread MIDI RTE Alarm");
	MIDIrtealarmproc->StopThread();

	maingui->SetInfoWindowText("Close Thread MIDI Out");
	MIDIoutproc->DeInit();

	maingui->SetInfoWindowText("Close Thread MIDI Proc");
	MIDIalarmprocessorproc->StopThread();

	maingui->SetInfoWindowText("Close Thread MIDI MTC");
	MIDImtcproc->StopThread();

	maingui->SetInfoWindowText("Close Thread MIDI Start");
	MIDIstartthread->StopThread();

	maingui->SetInfoWindowText("Close Thread MIDI Thru");
	mainMIDIthruthread->StopThread();

	maingui->SetInfoWindowText("Close Thread MIDI Help");
	mainhelpthread->StopThread();

	maingui->SetInfoWindowText("Close Thread MIDI Sync");
	mainsyncthread->StopThread();

	// Audio
	maingui->SetInfoWindowText("Close Thread Audio Out");
	maindevicerefillthread->StopThread();

	maingui->SetInfoWindowText("Close Thread Audio In");
	maindeviceinputthread->StopThread();

	maingui->SetInfoWindowText("Close Thread Audio Work");
	audioworkthread->StopThread();

	maingui->SetInfoWindowText("Close Thread Audio Peak");
	audiopeakthread->StopThread();

	//	maingui->SetInfoWindowText("Close Thread Audio Record Buffer");
	//	audiorecordbufferthread->StopThread(); // Stop before audiorecordthread->StopThread(); !

	maingui->SetInfoWindowText("Close Thread Audio Record");
	audiorecordthread->DeInit();	

	//maingui->SetInfoWindowText("Close Thread Audio Freeze");
	//audiofreezethread->StopThread();

	maingui->SetInfoWindowText("Close Thread Audio Out/Stream Proc");
	mainaudiostreamproc->StopThread();

	maingui->SetInfoWindowText("Close Thread Audio In Proc");
	mainaudioinproc->StopThread();

	maingui->SetInfoWindowText("Close Thread Audio Realtime");
	mainaudiorealtimethread->StopThread();

#ifdef CAMXGUIHTREADS
	maingui->winproc.StopThread();
#endif
}

Seq_Main::Seq_Main() 
{
	// Set CPU Cores
#ifdef WIN32
	SYSTEM_INFO systeminfo;
	GetSystemInfo(&systeminfo);

	cpucores=systeminfo.dwNumberOfProcessors;
#endif

	if(cpucores>MAXCORES)
		cpucores=MAXCORES;

	if(cpucores<=0)
		cpucores=1;

	//cpucores=1;

	updateflag=0;
	/*
	double h=500;

	h/=PPQRATE;

	ppqmsfactor=h;

	// Calc Clock Rate
	h=500; // 500ms
	h/=24;
	clockimpulse_ms=(int)h;

	clockimpulse_ppqfactor=PPQRATE/24;

	clockimpuse_msfactor=2*PPQRATE;
	clockimpuse_msfactor/=1000;
	*/

	ResetAutoSave();
	ResetPeakCheck();

	userdir[0]=currentdir[0]=0;
	errorflag=0;
	saveonexit=true;

	//stringbuffer=0;
	//songlength=300;

	exitprogram_flag=false;

	exithardware=false;
	exitthreads=false;

	activeproject=0;
	updatereset=false;

	stripExtension_string=0;

	// DEFAULT_HEADRASTER * numberofbars

	// Zooms
	guizoom[0].prev=false;

	// -----------------------

	int c=0;

	//	guizoom[c].xpixel=1;
	//guizoom[c++].withzoom=false;

	guizoom[c].xpixel=2;
	guizoom[c].measureraster=32;
	guizoom[c].sec=60;
	guizoom[c].samples=10000000;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=4;
	guizoom[c].sec=60;
	guizoom[c].samples=10000000;
	guizoom[c].measureraster=32;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=8;
	guizoom[c].sec=30;
	guizoom[c].samples=5000000;
	guizoom[c].measureraster=16;
	guizoom[c++].withzoom=false;

	// ------------------------
	guizoom[c].xpixel=16;
	guizoom[c].sec=20;
	guizoom[c].samples=2000000;
	guizoom[c].measureraster=16;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=24; //16
	guizoom[c].sec=10;
	guizoom[c].samples=2000000;
	guizoom[c].measureraster=8;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=32; //24
	guizoom[c].sec=10;
	guizoom[c].samples=1000000;
	guizoom[c].measureraster=8;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=40;
	guizoom[c].sec=5;
	guizoom[c].samples=1000000;
	guizoom[c].measureraster=8;
	guizoom[c++].withzoom=false;

	// --------------------------
	guizoom[c].xpixel=48;
	guizoom[c].sec=5;
	guizoom[c].samples=1000000;
	guizoom[c].measureraster=8;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=56;
	guizoom[c].sec=5;
	guizoom[c].samples=1000000;
	guizoom[c].measureraster=4;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=64;
	guizoom[c].sec=5;
	guizoom[c].samples=1000000;
	guizoom[c].measureraster=4;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=72;
	guizoom[c].sec=5;
	guizoom[c].samples=500000;
	guizoom[c].measureraster=4;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=80;
	guizoom[c].sec=5;
	guizoom[c].samples=500000;
	guizoom[c].measureraster=4;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=88;
	guizoom[c].sec=4;
	guizoom[c].samples=500000;
	guizoom[c].measureraster=4;
	guizoom[c++].withzoom=false;

	guizoom[c].xpixel=96;
	guizoom[c].sec=3;
	guizoom[c].samples=200000;
	guizoom[c++].withzoom=true;		
	// --------------------------
	guizoom[c].xpixel=128;
	guizoom[c].sec=2;
	guizoom[c].samples=200000;
	guizoom[c++].withzoom=true;

	guizoom[c].xpixel=192;
	guizoom[c].sec=1;
	guizoom[c].samples=100000;
	guizoom[c++].withzoom=true;

	guizoom[c].xpixel=240;
	guizoom[c].sec=1;
	guizoom[c].samples=100000;
	guizoom[c++].withzoom=true;

	guizoom[c].xpixel=256;
	guizoom[c].sec=1;
	guizoom[c].samples=100000;
	guizoom[c++].withzoom=true;

	// ------------------------

	guizoom[c].xpixel=320;
	guizoom[c].sec=0.7;
	guizoom[c].samples=100000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=384;
	guizoom[c].sec=0.7;
	guizoom[c].samples=100000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=480;
	guizoom[c].sec=0.5;
	guizoom[c].samples=50000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=576;
	guizoom[c].sec=0.5;
	guizoom[c].samples=50000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	// ----------------------------
	guizoom[c].xpixel=640;
	guizoom[c].sec=0.4;
	guizoom[c].samples=10000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=768;
	guizoom[c].sec=0.4;
	guizoom[c].samples=10000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=960;
	guizoom[c].sec=0.3;
	guizoom[c].samples=10000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=1280;
	guizoom[c].sec=0.3;
	guizoom[c].samples=10000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=1600;
	guizoom[c].sec=0.2;
	guizoom[c].samples=5000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=1920;
	guizoom[c].sec=0.2;
	guizoom[c].samples=5000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=2240;
	guizoom[c].sec=0.1;
	guizoom[c].samples=5000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=2560;
	guizoom[c].sec=0.1;
	guizoom[c].samples=4000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=2880;
	guizoom[c].sec=0.1;
	guizoom[c].samples=4000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=3200;
	guizoom[c].sec=0.1;
	guizoom[c].samples=4000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=3520;
	guizoom[c].sec=0.1;
	guizoom[c].samples=4000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=3840;
	guizoom[c].sec=0.1;
	guizoom[c].samples=3000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=4160;
	guizoom[c].sec=0.05;
	guizoom[c].samples=3000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=4160+640+640;
	guizoom[c].sec=0.05;
	guizoom[c].samples=3000;
	guizoom[c].withzoom=guizoom[c].show1x=true;c++;

	guizoom[c].xpixel=4160+640+640+640+640;
	guizoom[c].sec=0.05;
	guizoom[c].samples=2000;
	guizoom[c].withzoom=guizoom[c].show1x=true;

	guizoom[0].prev=false;
	guizoom[c].next=false;

#ifdef DEBUG
	if(c!=NUMBEROFWINZOOMS-1)
		maingui->MessageBoxError(0,"NUMBEROFWINZOOMS");
#endif

	numberwinzooms=NUMBEROFWINZOOMS;

	// init zoom nr
	for(int i=0;i<NUMBEROFWINZOOMS;i++)
	{
		guizoom[i].next=guizoom[i].prev=true;
		guizoom[i].index=i;

		OSTART h=/*guizoom[i].numberofbars**/DEFAULT_HEADRASTER;
		h/=guizoom[i].xpixel;

		guizoom[i].dticksperpixel=h;
		guizoom[i].ticksperpixel=h;

#ifdef DEBUG
		if(guizoom[i].dticksperpixel!=guizoom[i].ticksperpixel)
			maingui->MessageBoxError(0,"guizoom[i].dticksperpixel!=guizoom[i].ticksperpixel");
#endif
	}

	autoupdatecheck=true;
}