#include "defines.h"
#include "object_project.h"

#include "songmain.h"
#include "MIDIfile.h"
#include "seqtime.h"
#include "editbuffer.h"
#include "languagefiles.h"
#include "gui.h"
#include "audiofile.h"
#include "audiodevice.h"

#include "audiohardware.h"
#include "MIDIhardware.h"
#include "semapores.h"
#include "MIDIinproc.h"
#include "MIDIoutproc.h"
#include "MIDIthruproc.h"
#include "audiothread.h"
#include "mainhelpthread.h"
#include "settings.h"
#include "audiorecord.h"
#include "editfunctions.h"
#include "audioproc.h"
#include "audiorealtime.h"
#include "wavemap.h"
#include "drummap.h"
#include "MIDIprocessor.h"
#include "rmg.h"
#include "database.h"
#include "audiomanager.h"
#include "MIDItimer.h"
#include "audiothreads.h"

#include <shlobj.h>

#ifdef _DEBUG
int defcounter=0;
#endif

// main Classes
Seq_Main *mainvar;
mainMIDIBase *mainMIDI;
mainMIDIRecord *mainMIDIrecord;
DataBase *maindatabase; 

mainAudio *mainaudio;
mainAudioRealtime *mainaudioreal;
mainWaveMap *mainwavemap;
mainRMGMap *mainrmgmap;
mainProcessor *mainprocessor;

#ifdef MEMPOOLS
mainPools *mainpools;
#endif

ThreadControl *mainthreadcontrol;
EditFunctions *mainedit;
EditBuffer *mainbuffer;
Settings *mainsettings;
GUI *maingui;

// Sub Thread 
PluginMIDIInputProc *plugininproc;

MIDIInProc *MIDIinproc;
MIDIPlaybackThread *mainMIDIalarmthread;
MIDIRTEProc *MIDIrtealarmproc;
MIDIOutProc *MIDIoutproc;
MIDIProcessorProc *MIDIalarmprocessorproc;
MIDIMTCThread *MIDImtcproc;
MIDIStartThread *MIDIstartthread;
MIDIThruThread *mainMIDIthruthread;

AudioPeakFileThread *audiopeakthread;
//AudioFreezeThread *audiofreezethread;
AudioWorkFileThread *audioworkthread;

AudioRecordMainThread *audiorecordthread;
AudioRealtimeThread *mainaudiorealtimethread;
AudioCoreAudioInputProc *mainaudioinproc;
AudioCoreAndStreamProc *mainaudiostreamproc;

// Win32 Audio Device Threads
AudioDeviceDeviceOutThread *maindevicerefillthread;
AudioDeviceDeviceInThread *maindeviceinputthread;

MainHelpThread *mainhelpthread;
MainSyncThread *mainsyncthread;

sysTimer *maintimer;

#include "colourrequester.h"

#ifdef WIN32
// Colours
HBRUSH colour_hBrush[LASTCOLOUR];
HPEN colour_hPen[LASTCOLOUR];
COLORREF colour_ref[LASTCOLOUR];
#endif

double logvolume[(LOGVOLUME_SIZE+1)]; // Silence,0.1->6 db,//+-0// -0.1 -> - 80dB
float logvolume_f[(LOGVOLUME_SIZE+1)];

guiZoom guizoom[NUMBEROFWINZOOMS];

#ifdef _DEBUG
//#include "e:/compiler/phonostar_new/ex_wave.h"
extern void TestWaveFile();
extern void timermain();
extern void resample_test();
#endif

UINT uDragMsg; // some global variable (or static in your main window procedure)

#ifdef WIN32
#include "cpuid.h"
_p_info maincpuinfo; // CPU info
#endif

#ifdef NOSSE2
bool InitCPU()
{
	/*
	MessageBox(
	0,          // handle of owner window
	"_cpuid",     // address of text in message box
	Cxs[CXS_INFO],  // address of title of message box
	MB_OK_STYLE // style of message box
	);
	*/

	_cpuid(&maincpuinfo);

#ifndef DEBUG
	if(maincpuinfo.feature & _CPU_FEATURE_SSE2)
	{
		/*
		MessageBox(
		0,          // handle of owner window
		"_CPU_FEATURE_SSE2",     // address of text in message box
		Cxs[CXS_INFO],  // address of title of message box
		MB_OK_STYLE // style of message box
		);
		*/

		MessageBox(
			0,          // handle of owner window
			Cxs[CXS_SSEINITMSG],     // address of text in message box
			Cxs[CXS_INFO],  // address of title of message box
			MB_OK_STYLE // style of message box
			);

		return true;
	}
#endif

	/*
	MessageBox(
	0,          // handle of owner window
	"SSE Check",     // address of text in message box
	Cxs[CXS_INFO],  // address of title of message box
	MB_OK_STYLE // style of message box
	);
	*/


	//SSE Check
	if(maincpuinfo.feature & _CPU_FEATURE_SSE)
	{
		/*
		MessageBox(
		0,          // handle of owner window
		"_CPU_FEATURE_SSE",     // address of text in message box
		Cxs[CXS_INFO],  // address of title of message box
		MB_OK_STYLE // style of message box
		);
		*/

		return true;
	}

	MessageBox(
		0,          // handle of owner window
		Cxs[CXS_CPUNOSSEERROR],     // address of text in message box
		Cxs[CXS_ERROR],  // address of title of message box
		MB_ERROR_STYLE // style of message box
		);

	return false;
}
#endif

#ifndef NOSSE2
bool InitCPU()
{
#ifdef WIN64
	return true;
#endif

#ifndef WIN64
	// SSE1
	_cpuid(&maincpuinfo);

	if(maincpuinfo.feature & _CPU_FEATURE_SSE2)
	{
		return true;
	}
#endif


	//SSE Check
	/*
	if(maincpuinfo.feature & _CPU_FEATURE_SSE)
	return true;
	*/

	MessageBox(
		0,          // handle of owner window
		Cxs[CXS_SSEERROR],     // address of text in message box
		Cxs[CXS_ERROR],  // address of title of message box
		MB_ERROR_STYLE // style of message box
		);

	return false;
	/*
	if (mask & _CPU_FEATURE_MMX) {
	printf("\t%s\t_CPU_FEATURE_MMX\n",
	avail & _CPU_FEATURE_MMX ? "yes" : "no");
	}
	if (mask & _CPU_FEATURE_SSE) {
	printf("\t%s\t_CPU_FEATURE_SSE\n",
	avail & _CPU_FEATURE_SSE ? "yes" : "no");
	}
	if (mask & _CPU_FEATURE_SSE2) {
	printf("\t%s\t_CPU_FEATURE_SSE2\n",
	avail & _CPU_FEATURE_SSE2 ? "yes" : "no");
	}
	if (mask & _CPU_FEATURE_3DNOW) {
	printf("\t%s\t_CPU_FEATURE_3DNOW\n",
	avail & _CPU_FEATURE_3DNOW ? "yes" : "no");
	}*/

	return true;
	/*
	printf("name:\t\t%s\n", info.name);
	printf("model:\t\t%s\n", info.model_name);
	printf("family:\t\t%d\n", info.family);
	printf("model:\t\t%d\n", info.model);
	printf("stepping:\t%d\n", info.stepping);
	printf("feature:\t%08x\n", info.feature);
	expand(info.feature, info.checks);
	printf("os_support:\t%08x\n", info.os_support);
	expand(info.os_support, info.checks);
	printf("checks:\t\t%08x\n", info.checks);
	*/
}
#endif

bool InitGlobalClasses()
{
#ifdef MEMPOOLS
	mainpools=0;
#endif

	mainvar=0;
	mainMIDI=0;
	mainMIDIrecord=0;
	maindatabase=0;
	mainaudio=0;
	mainaudioreal=0;
	mainwavemap=0;
	mainrmgmap=0;
	mainprocessor=0;
	mainthreadcontrol=0;
	mainedit=0;
	mainbuffer=0;
	mainsettings=0;
	maingui=0;
	// Sub Thread 
	MIDIinproc=0;
	plugininproc=0;
	mainMIDIalarmthread=0;
	MIDIrtealarmproc=0;
	MIDIoutproc=0;
	MIDIalarmprocessorproc=0;
	MIDImtcproc=0;
	mainMIDIthruthread=0;
	audiopeakthread=0;
//	audiofreezethread=0;
	audioworkthread=0;
	audiorecordthread=0;
	//audiorecordbufferthread=0;
	mainaudiorealtimethread=0;
	maindevicerefillthread=0;
	maindeviceinputthread=0;
	mainhelpthread=0;
	mainsyncthread=0;
	mainaudioinproc=0;
	mainaudiostreamproc=0;
	maintimer=0;

	// Classes

#ifdef MEMPOOLS
	mainpools=new mainPools;
	if(!mainpools)return false;
#endif

	mainvar=new Seq_Main;
	if(!mainvar)return false;

	mainMIDI=new mainMIDIBase;
	if(!mainMIDI)return false;

	mainMIDIrecord=new mainMIDIRecord;
	if(!mainMIDIrecord)return false;

	maindatabase=new DataBase;
	if(!maindatabase)return false;

	mainaudio=new mainAudio;
	if(!mainaudio)return false;
	

	mainaudioreal=new mainAudioRealtime;
	if(!mainaudioreal)return false;

	mainwavemap=new mainWaveMap;
	if(!mainwavemap)return false;

	//maindrummap=new mainDrumMap;
	//if(!maindrummap)return false;

	mainrmgmap=new mainRMGMap;
	if(!mainrmgmap)return false;

	mainprocessor=new mainProcessor;
	if(!mainprocessor)return false;

	mainthreadcontrol=new ThreadControl;
	if(!mainthreadcontrol)return false;

	mainedit=new EditFunctions;
	if(!mainedit)return false;

	mainbuffer=new EditBuffer;
	if(!mainbuffer)return false;

	mainsettings=new Settings;
	if(!mainsettings)return false;

	maingui=new GUI;
	if(!maingui)return false;


	// Sub Thread 
	MIDIinproc=new MIDIInProc;
	if(!MIDIinproc)return false;

	plugininproc=new PluginMIDIInputProc;
	if(!plugininproc)return false;

	mainMIDIalarmthread=new MIDIPlaybackThread;
	if(!mainMIDIalarmthread)return false;

	MIDIrtealarmproc=new MIDIRTEProc;
	if(!MIDIrtealarmproc)return false;

	MIDIoutproc=new MIDIOutProc;
	if(!MIDIoutproc)return false;

	MIDIalarmprocessorproc=new MIDIProcessorProc;
	if(!MIDIalarmprocessorproc)return false;

	MIDImtcproc=new MIDIMTCThread;
	if(!MIDImtcproc)return false;

	MIDIstartthread=new MIDIStartThread;
	if(!MIDIstartthread)return false;

	mainMIDIthruthread=new MIDIThruThread;
	if(!mainMIDIthruthread)return false;

	audiopeakthread=new AudioPeakFileThread;
	if(!audiopeakthread)return false;

	//audiofreezethread=new AudioFreezeThread;
	//if(!audiofreezethread)return false;

	audioworkthread=new AudioWorkFileThread;
	if(!audioworkthread)return false;

	audiorecordthread=new AudioRecordMainThread;
	if(!audiorecordthread)return false;

	//audiorecordbufferthread=new AudioRecordBufferThread;
	//if(!audiorecordbufferthread)return false;

	mainaudiorealtimethread=new AudioRealtimeThread;
	if(!mainaudiorealtimethread)return false;

	maindevicerefillthread=new AudioDeviceDeviceOutThread;
	if(!maindevicerefillthread)return false;

	maindeviceinputthread=new AudioDeviceDeviceInThread;
	if(!maindeviceinputthread)return false;

	mainhelpthread=new MainHelpThread;
	if(!mainhelpthread)return false;

	mainsyncthread=new MainSyncThread;
	if(!mainsyncthread)return false;

	mainaudiostreamproc=new AudioCoreAndStreamProc;
	if(!mainaudiostreamproc)return false;

	mainaudioinproc=new AudioCoreAudioInputProc;
	if(!mainaudioinproc)return false;

	// Timer
	maintimer=new sysTimer;
	if(!maintimer)return false;

	return true;
}

void DeInitGlobalClasses()
{
#ifdef MEMPOOLS
	if(mainpools)
	{
		mainpools->CloseAllMemoryPools();
		delete mainpools;
	}
#endif

	if(mainvar)
		delete mainvar;

	if(mainMIDI)
		delete mainMIDI;

	if(mainMIDIrecord)
		delete mainMIDIrecord;

	if(maindatabase)
		delete maindatabase; 

	if(mainaudio)
		delete mainaudio;

	if(mainaudioreal)
		delete mainaudioreal;

	if(mainwavemap)
		delete mainwavemap;

	//if(maindrummap)
	//delete maindrummap;

	if(mainrmgmap)
		delete mainrmgmap;

	if(mainprocessor)
		delete mainprocessor;

	if(mainthreadcontrol)
		delete mainthreadcontrol;

	if(mainedit)
		delete mainedit;

	if(mainbuffer)
		delete mainbuffer;

	if(mainsettings)
		delete mainsettings;

	// Sub Thread 
	if(MIDIinproc)
		delete MIDIinproc;

	if(plugininproc)
		delete plugininproc;

	if(mainMIDIalarmthread)
		delete mainMIDIalarmthread;

	if(MIDIalarmprocessorproc)
		delete MIDIalarmprocessorproc;

	if(MIDImtcproc)
		delete MIDImtcproc;

	if(mainMIDIthruthread)
		delete mainMIDIthruthread;

	if(audiopeakthread)
		delete audiopeakthread;

//	if(audiofreezethread)
//		delete audiofreezethread;

	if(audioworkthread)
		delete audioworkthread;

	if(audiorecordthread)
		delete audiorecordthread;

	//if(audiorecordbufferthread)
	//delete audiorecordbufferthread;

	if(mainaudiorealtimethread)
		delete mainaudiorealtimethread;

	if(maindevicerefillthread)
		delete maindevicerefillthread;

	if(maindeviceinputthread)
		delete maindeviceinputthread;

	if(mainhelpthread)
		delete mainhelpthread;

	if(mainsyncthread)
		delete mainsyncthread;

	if(mainaudiostreamproc)
		delete mainaudiostreamproc;

	if(mainaudioinproc)
		delete mainaudioinproc;

	if(maintimer)
		delete maintimer;

	if(maingui)
		delete maingui; // Always Last ! gui->Message

	if(MIDIrtealarmproc)
		delete MIDIrtealarmproc;

	if(MIDIoutproc)
		delete MIDIoutproc;
}

// class templates
template <class T>
class mypair:public Object {
	T a, b;
public:
	mypair (T first, T second)
	{a=first; b=second;}
	T getmax ();
};

template <class T>
T mypair<T>::getmax ()
{
	T retval;
	retval = a>b? a : b;
	return retval;
}

/*
void DisplayVolumePaths(
__in PWCHAR VolumeName
)
{
DWORD  CharCount = MAX_PATH + 1;
PWCHAR Names     = NULL;
PWCHAR NameIdx   = NULL;
BOOL   Success   = FALSE;

for (;;) 
{
//
//  Allocate a buffer to hold the paths.
Names = (PWCHAR) new BYTE [CharCount * sizeof(WCHAR)];

if ( !Names ) 
{
//
//  If memory can't be allocated, return.
return;
}

//
//  Obtain all of the paths
//  for this volume.
Success = GetVolumePathNamesForVolumeNameW(
VolumeName, Names, CharCount, &CharCount
);

if ( Success ) 
{
break;
}

if ( GetLastError() != ERROR_MORE_DATA ) 
{
break;
}

//
//  Try again with the
//  new suggested size.
delete [] Names;
Names = NULL;
}

if ( Success )
{
//
//  Display the various paths.
for ( NameIdx = Names; 
NameIdx[0] != L'\0'; 
NameIdx += wcslen(NameIdx) + 1 ) 
{
wprintf(L"  %s", NameIdx);
}
wprintf(L"\n");
}

if ( Names != NULL ) 
{
delete [] Names;
Names = NULL;
}

return;
}

void GetVolumes()
{
DWORD  CharCount            = 0;
WCHAR  DeviceName[MAX_PATH] = L"";
DWORD  Error                = ERROR_SUCCESS;
HANDLE FindHandle           = INVALID_HANDLE_VALUE;
BOOL   Found                = FALSE;
size_t Index                = 0;
BOOL   Success              = FALSE;
WCHAR  VolumeName[MAX_PATH] = L"";

//
//  Enumerate all volumes in the system.
FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

if (FindHandle == INVALID_HANDLE_VALUE)
{
Error = GetLastError();
wprintf(L"FindFirstVolumeW failed with error code %d\n", Error);
return;
}

for (;;)
{
//
//  Skip the \\?\ prefix and remove the trailing backslash.
Index = wcslen(VolumeName) - 1;

if (VolumeName[0]     != L'\\' ||
VolumeName[1]     != L'\\' ||
VolumeName[2]     != L'?'  ||
VolumeName[3]     != L'\\' ||
VolumeName[Index] != L'\\') 
{
Error = ERROR_BAD_PATHNAME;
wprintf(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
break;
}

//
//  QueryDosDeviceW does not allow a trailing backslash,
//  so temporarily remove it.
VolumeName[Index] = L'\0';

CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 

VolumeName[Index] = L'\\';

if ( CharCount == 0 ) 
{
Error = GetLastError();
wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
break;
}

TRACE("\nFound a device:\n %s", DeviceName);
TRACE(L"\nVolume name: %s", VolumeName);
TRACE(L"\nPaths:");
DisplayVolumePaths(VolumeName);

//
//  Move on to the next volume.
Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

if ( !Success ) 
{
Error = GetLastError();

if (Error != ERROR_NO_MORE_FILES) 
{
wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
break;
}

//
//  Finished iterating
//  through all the volumes.
Error = ERROR_SUCCESS;
break;
}
}

FindVolumeClose(FindHandle);
FindHandle = INVALID_HANDLE_VALUE;
}

*/

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,int nCmdShow) 
{
#ifdef WIN32
 // neuen Mutex creieren
    HANDLE hInstanceMutex = ::CreateMutex(NULL,TRUE, "CAMXSTARTMUTEX");
 
    // wenn Mutex "MUTEXNAME" schon existiert, d.h. die Anwendung schon
    // einmal geöffnet war, wird der Fehler ERROR_ALREADY_EXISTS erzeugt
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // Handle auf den Mutex zerstören
        if (hInstanceMutex) CloseHandle(hInstanceMutex);
 
        // Anwendung beenden
        return 0;
    }
#endif

	/*
	char *s=new char[9];
	strcpy(s,"Hallo");
	return 0;
	*/

#ifdef DEBUG
	double h=2;
	double h2=1.5;

	LONGLONG x=(LONGLONG)(h*h2);

#endif

	//	GetVolumes();

	/*
	MessageBox(
	0,          // handle of owner window
	"Start",     // address of text in message box
	Cxs[CXS_INFO],  // address of title of message box
	MB_OK_STYLE // style of message box
	);
	*/

	if(InitCPU()==false) // SSE/SSE2 Check
	{
		/*
		MessageBox(
		0,          // handle of owner window
		"InitCPU Fehler",     // address of text in message box
		Cxs[CXS_INFO],  // address of title of message box
		MB_OK_STYLE // style of message box
		);
		*/

		return 0;
	}

	//WPARAM msg=0;

	if(InitGlobalClasses()==false)
		return 0;

	//TRACE ("+3dB: %f\n",mainaudio->ConvertDbToFactor(3));
	//TRACE ("-3dB: %f\n",mainaudio->ConvertDbToFactor(-3));

	if(maintimer->Init()==false)
		return 0;

	// call this in your WM_CREATE/WM_INITDIALOG
	uDragMsg = RegisterWindowMessage(DRAGLISTMSGSTRING);

	char szBuffer[MAX_PATH];

	// Set CPU Cores
#ifdef WIN32
	maingui->hInst=hInstance;
	maingui->cmdshow=nCmdShow;
#endif

	//if(mainvar->CheckRunningCamXVersion()==false)
	{
		{
			char appPath[1035];
			// _getcwd(appPath,1035);

			GetModuleFileName(0,appPath,1034);
			*strrchr(appPath, '\\') = '\0';

			if(SHGetSpecialFolderPath(HWND_DESKTOP, szBuffer, CSIDL_PERSONAL, FALSE)) // +Create User Directory
			{
				char *h=mainvar->GenerateString(szBuffer,"\\CamX Data");

				if(h)
				{
					if(mainvar->CreateNewDirectory(h)==true)
					{
						mainvar->SetUserDirectory(h);

						char *dirtocreate[]=
						{
							"Preferences",
							SETTINGSFILE_AUDIODEVICES_DIR,
							SETTINGSFILE_MIDIDEVICES_DIR,
							"Preferences\\MIDI Devices\\Input",
							"Preferences\\MIDI Devices\\Output",
#ifdef WIN32

#ifdef WIN64
							"AutoloadX64",
#else
							"AutoloadX32",
#endif

#endif
							"Database",
							"Database\\DrumMaps",
							"Database\\MIDIFilter",
							"Generator Maps",
							"Groups",
							"Library",
							"Peakfiles",
#ifdef WIN32

#ifdef WIN64
							"ProjectsX64",
#else
							"ProjectsX32",
#endif

#endif
							"Saved Pattern",
							"Updates",
							0
						};

						for(int i=0;dirtocreate[i];i++)
						{
							if(char *prefs=mainvar->GenerateString(h,"\\",dirtocreate[i]))
							{
								mainvar->CreateNewDirectory(prefs); // Create SubDirs User
								delete prefs;
							}
						}
					}

					delete h;
				}
				else
					return 0;
			}

			mainvar->SetCamXDirectory(appPath);

#ifdef WIN32
			// Vst C::Programs
			if(SHGetSpecialFolderPath(HWND_DESKTOP, szBuffer, CSIDL_PROGRAM_FILES, FALSE))
			{
				char *h=mainvar->GenerateString(szBuffer,"\\VstPlugIns");
				if(h)
				{
					if(Directory *dir=mainsettings->AddVSTDirectory(VST2,h))
						dir->dontdelete=true;

					delete h;
				}
			}
#endif

#ifdef WIN64
			// Vst C::Programs
			if(SHGetSpecialFolderPath(HWND_DESKTOP, szBuffer, CSIDL_PROGRAM_FILES, FALSE))
			{
				char *h=mainvar->GenerateString(szBuffer,"\\VstPlugIns64");
				if(h)
				{
					if(Directory *dir=mainsettings->AddVSTDirectory(VST2,h))
						dir->dontdelete=true;

					delete h;
				}
			}
#endif
		}

		maingui->InitGUI();

		Seq_SelectionList *sel=0;

		bool initupdatefile=false;

		TRACE ("Size of Int %d\n",sizeof(int));

		mainsettings->InitDefaultWindowPositions();
		mainaudio->LoadDataBase();

		mainsettings->Load(mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_SETTINGS));

		// #ifndef _DEBUG
		if(mainsettings->autoupdatequestion==false)
		{
			mainsettings->autoupdatequestion=true;
			mainvar->autoupdatecheck=maingui->MessageBoxYesNo(0,Cxs[CXS_AUTOUPDATEQUESTION]);
			mainsettings->Save(0);
		}

		if(mainvar->autoupdatecheck==true)
		{
			maingui->allwindowsoondesktop=true;

			//	mainvar->autoupdatecheck=true;
			mainvar->updateflag=mainvar->CheckForUpdates(false);
			maingui->allwindowsoondesktop=false;
			initupdatefile=(mainvar->updateflag&UPDATE_FLAG_INITUPDATEFILE)?true:false;
		}
		// #endif

		if(initupdatefile==true)
		{
			mainwavemap->DeleteAllWaveMaps();
			mainsettings->synthdevices.DeleteAllDevices();
			mainsettings->DeleteAllVSTDirectories();
			mainsettings->DeleteAllAudioDirectories();

			maintimer->Exit();
			maingui->CloseGUI();

			mainvar->InitUpdates();

			return 0;
		}

		//if(maingui->FirstSmaingui->OpenScreen(0,true)==true) // Screen open ?

		maingui->OpenScreen(0,true);

		//maingui->OpenInfoWindow(false);

		Seq_Song *song=0;

		maingui->SetInfoWindowText("Init Threads...");

		if(mainvar->InitThreads()==0)
		{
			maingui->SetInfoWindowText("Init RMG Maps...");
			mainrmgmap->InitMaps();

			maingui->SetInfoWindowText("Drum Maps...");
			mainMIDI->CollectDrumsMaps();

			//	maingui->SetInfoWindowText("Settings File...");
			//	mainsettings->Load(mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_SETTINGS));

			maingui->SetInfoWindowText("Init Hardware...#1");

			if(mainvar->InitHardware()==0)
			{
				maingui->SetInfoWindowText("Init Hardware...#2");
				mainsettings->LoadDevices();
				
				maingui->SetInfoWindowText("Init Projects...");
				mainsettings->LoadProjects();

				if(mainaudio->GetActiveDevice())
				{
					if(char *h=mainvar->GenerateString("Audio...Start Device ",mainaudio->GetActiveDevice()->GetDeviceName()))
					{
						maingui->SetInfoWindowText(h);
						delete h;
					}				
				}

			//	mainaudio->StartDevices();

				maingui->SetInfoWindowText("Settings 2...");

				// Load Settings -> after InitHardware !
				mainsettings->LoadDevicePrograms();

				{
					maingui->SetInfoWindowText("Init Main...");
					mainvar->Init();

					// Start ...
					if(mainsettings->autoloadlastusedproject==true && mainsettings->prevprojects_dirname[0]){

						if(char *h=mainvar->GenerateString("Load Project ",mainsettings->prevprojects_dirname[0],"..."))
						{
							maingui->SetInfoWindowText(h);
							delete h;
						}

						mainvar->OpenProject(maingui->FirstScreen(),mainsettings->prevprojects_dirname[0]);
					}

					maingui->SetInfoWindowText("Project/Song...");

					if(!mainvar->GetActiveProject()) // Create Demo Project
					{
						if(maingui->MessageBoxYesNo(0,Cxs[CXS_Q_CREATEDEMOPROJECT])==true)
						{
							bool exists;

							if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),
#ifdef WIN32
#ifdef WIN64
								"\\ProjectsX64\\demo")
#else
								"\\ProjectsX32\\demo")
#endif

#endif

								)
							{
								if(Seq_Project *project=mainvar->CreateProject(h,&exists,false))
									project->Save(0);
								else
								{
									if(exists==true)
									{
										// maingui->MessageBoxOk(0,"Load Init Project");
										mainvar->OpenProject(0,h);
									}
								}

								delete h;
							}
						}
					}

					// Update Check



					/*
					if(updateflag&UPDATE_FLAG_RESTART)						
					{
					}

					*/

					maingui->SetInfoWindowText("CamX Start");

					//	maingui->CloseInfoWindow();

					if(!mainwavemap->FirstWaveMap())
						mainwavemap->InitDefaultWaveDefinitions();

					if(mainvar->GetActiveProject())
					{
						song=mainvar->GetActiveSong();

						if(!song)
						{
							if(!(song=mainvar->GetActiveProject()->FirstSong()))
							{
								// Load Demo Song
								song=mainvar->GetActiveProject()->CreateNewSong(0,Seq_Project::CREATESONG_CREATEAUDIOMASTER,0,0);

								if(song){
									MIDIFile MIDIfile;
									MIDIfile.ReadMIDIFileToSong(song,"BellaCiao.mid"); // Fill our song with MIDI-File
									song->Save(0);
									song->PRepairPlayback(0,MEDIATYPE_ALL);

									song->OpenIt(maingui->GetActiveScreen());

									//	mainvar->SetActiveProject(mainvar->GetActiveProject(),song);
								}
							}
						}
					}

					if(mainaudio->newvstpluginsadded>0)
						mainsettings->Save(0);

					maingui->SetInfoWindowText(0);

					
#ifndef _DEBUG

		{
			char versionstring[64];

			strcpy(versionstring,"CamX ");

			char h2[NUMBERSTRINGLEN];

			int f=maingui->GetVersion()/1000;
			int p=maingui->GetVersion()-(f*1000);

			mainvar->AddString(versionstring,mainvar->ConvertIntToChar(f,h2));

			if(p<10)
				mainvar->AddString(versionstring,".00");
			else
				if(p<100)
					mainvar->AddString(versionstring,".0");
				else
					mainvar->AddString(versionstring,".");

			mainvar->AddString(versionstring,mainvar->ConvertIntToChar(p,h2));

			if(char *h=mainvar->GenerateString(versionstring," UI v.2 Alpha\n","01.10.2015\n",Cxs[CXS_DONTUSETOCREATESONGS]))
			{
				maingui->MessageBoxOk(0,h);
				delete h;
			}
		}
#endif

					maingui->MessageLoop(); // +1 x Update Check
				}
			}
		}
		else
			maingui->MessageBoxError(NULL,Cxs[CXS_INITTHREADERROR]);

		mainvar->ExitProgram();

#ifdef _DEBUG
		if(defcounter!=0)
			MessageBox(NULL,"DefParms Exists",Cxs[CXS_ERROR],MB_OK);
#endif
	}
	

	if(mainvar->updatereset==true)
	{
		mainvar->InitUpdates();
	}

	DeInitGlobalClasses(); // Free Class Memory etc.

	//ExitProcess(0);

	  // nach normalem Beenden der Anwendung den Mutex wieder freigeben
#ifdef WIN32
	ReleaseMutex(hInstanceMutex);
    CloseHandle(hInstanceMutex);
#endif

	return 0;
}
