#include "songmain.h"
#include "MIDIhardware.h"
#include "audiohardware.h"
#include "languagefiles.h"
#include "MIDIprocessor.h"
#include "settings.h"
#include "gui.h"
#include "MIDIoutdevice.h"
#include "MIDIindevice.h"
#include "MIDIoutproc.h"

#ifdef WIN32
#include <windows.h>
#include <tlhelp32.h>
#endif

#ifdef OLDIE
bool Seq_Main::CheckRunningCamXVersion()
{
	bool camxfound=false;

#ifdef WIN32
	HWND c=FindWindowEx( NULL, NULL, CAMX_SCREENNAME, NULL );   

	if(c)
		return true;

	/*
	STARTUPINFO lpStartupInfo;

	char *procname;

	GetStartupInfo(&lpStartupInfo);   // address of STARTUPINFO structure

	procname=lpStartupInfo.lpTitle;

	procname+=strlen(lpStartupInfo.lpTitle);

	DWORD currentID= GetCurrentProcessId();

	HANDLE hProcessSnap;
	//	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	//	DWORD dwPriorityClass;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
	//	printError( TEXT("CreateToolhelp32Snapshot (of processes)") );

	return( false );
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
	//	printError( TEXT("Process32First") ); // show cause of failure
	CloseHandle( hProcessSnap );          // clean the snapshot object

	return( false );
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{

	// Retrieve the priority class.
	dwPriorityClass = 0;
	hProcess = OpenProcess( PROCESS_ALL_ACCESS, false, pe32.th32ProcessID );
	if( hProcess == NULL )
	{
	// printError( TEXT("OpenProcess") );
	}
	else
	{
	dwPriorityClass = GetPriorityClass( hProcess );

	if( !dwPriorityClass )
	{
	// printError( TEXT("GetPriorityClass") );
	}

	CloseHandle( hProcess );
	}

	printf( "\n  Process ID        = 0x%08X", pe32.th32ProcessID );
	printf( "\n  Thread count      = %d",   pe32.cntThreads );
	printf( "\n  Parent process ID = 0x%08X", pe32.th32ParentProcessID );
	printf( "\n  Priority base     = %d", pe32.pcPriClassBase );

	if( dwPriorityClass )
	printf( "\n  Priority class    = %d", dwPriorityClass );


	// Threads...
	if(pe32.th32ProcessID==currentID)
	{
	int i;

	i=1;
	}

	if(pe32.th32ProcessID!=currentID)
	{
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
	THREADENTRY32 te32; 

	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
	if( hThreadSnap == INVALID_HANDLE_VALUE ) 
	return( false ); 

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32 ); 

	// Retrieve information about the first thread,
	// and exit if unsuccessful
	if( !Thread32First( hThreadSnap, &te32 ) ) 
	{
	// printError( TEXT("Thread32First") ); // show cause of failure
	CloseHandle( hThreadSnap );          // clean the snapshot object
	return( false );
	}

	// Now walk the thread list of the system,
	// and display information about each thread
	// associated with the specified process
	do 
	{ 

	} while( Thread32Next(hThreadSnap, &te32 ) ); 

	CloseHandle( hThreadSnap );
	}

	// List the modules and threads associated with this process
	//	ListProcessModules( pe32.th32ProcessID );
	//	ListProcessThreads( pe32.th32ProcessID );

	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );

	*/

#endif

	return camxfound;
}
#endif

int Seq_Main::InitHardware()
{
	int error=0;

	mainMIDI->Init();

	// AUDIO
	maingui->SetInfoWindowText("Init Audio");
	mainaudio->OpenAudio();

	return error;
}

void Seq_Main::StopAllHardware()
{
	maingui->SetInfoWindowText("Stop Hardware...Audio");
	mainaudio->StopDevices();

	maingui->SetInfoWindowText("Stop Hardware...MIDI Input");
	mainMIDI->StopMIDIInputDevices();
}

void Seq_Main::CloseAllHardware()
{
	//Audio
	mainaudio->CloseAudio();

	mainMIDI->DeInit();
}

void Seq_Main::SetUserDirectory(char *s)
{
	if(s && strlen(s)<MAX_PATH+31)
		strcpy(userdir,s);
}

void Seq_Main::SetCamXDirectory(char *s)
{
	if(s && strlen(s)<1024)
		strcpy(currentdir,s);
}

void Seq_Main::Init()
{
	mainprocessor->InitDefaultProcessorModule();
}




