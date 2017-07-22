#include "MIDItimer.h"
#include "languagefiles.h"
#include "gui.h"

HANDLE gDoneEvent;
DWORD stime;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired) {

	stime=GetTickCount();
	SetEvent(gDoneEvent);
}

void timermain() {
	HANDLE hTimer = NULL,hTimerQueue = NULL;
	int whatever = 123;

	gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hTimerQueue = CreateTimerQueue();

	/*
	BOOL WINAPI CreateTimerQueueTimer(
	__out     PHANDLE phNewTimer,
	__in_opt  HANDLE TimerQueue,
	__in      WAITORTIMERCALLBACK Callback,
	__in_opt  PVOID Parameter,
	__in      DWORD DueTime,
	__in      DWORD Period,
	__in      ULONG Flags
	);
	*/

	LARGE_INTEGER time,time2,freq;

	QueryPerformanceFrequency(&freq);

	TRACE ("Freq %d\n",freq.QuadPart);

	CreateTimerQueueTimer( 
		&hTimer, 
		hTimerQueue, 
		(WAITORTIMERCALLBACK)TimerRoutine, 
		&whatever, 
		33,  // 100ms
		0, 
		0);

	QueryPerformanceCounter(&time);
	int t1=GetTickCount();
	WaitForSingleObject(gDoneEvent, 4);
	int t2=GetTickCount();

	QueryPerformanceCounter(&time2);

	LONGLONG diff=(time2.QuadPart-time.QuadPart)*1000;
	diff/=freq.QuadPart;

	TRACE ("Tick Test %d Stime %d %d\n",t2-t1,stime-t1,diff);

	DeleteTimerQueue(hTimerQueue);
}

bool sysTimer::Init()
{
	hTimerQueue = CreateTimerQueue();

	if(!hTimerQueue)
	{
		maingui->MessageBoxError(NULL,Cxs[CXS_TIMERINITERROR]);
		return false;
	}

	if(!QueryPerformanceFrequency(&freq))
	{
		maingui->MessageBoxError(NULL,Cxs[CXS_TIMERINITERROR]);
		return false;
	}

	sysfreq=freq.QuadPart;

	double h=freq.QuadPart;
	double h2=1000;

	systomsmul=h2/h;
	mstosysmul=h/h2;

	h=SAMPLESPERBEAT*2,h2=sysfreq;

	systointernratemul=h/h2;
	internratetosysmul=h2/h;

	//tickfreqfactor=msfreqfactor*INTERNRATEMSMUL;
	//freq_div=freq.QuadPart/1000; //ms

#ifdef DEBUG
	LONGLONG syst=ConvertMsToSysTime(1);
	double ms1=ConvertSysTimeToMs(syst);
#endif

	return true;
}

void sysTimer::Exit()
{
	DeleteTimerQueue(hTimerQueue);
}

/*
void sysTimer::GetRelativeSystemTime(SYSTEMTIME *t1, SYSTEMTIME *t2, SYSTEMTIME *rel)
{
    FILETIME v_ftime;
    ULARGE_INTEGER v_ui;
    __int64 v_right,v_left,v_res;
    SystemTimeToFileTime(t2,&v_ftime);

    v_ui.LowPart=v_ftime.dwLowDateTime;
    v_ui.HighPart=v_ftime.dwHighDateTime;
    v_right=v_ui.QuadPart;

    SystemTimeToFileTime(t1,&v_ftime);
    v_ui.LowPart=v_ftime.dwLowDateTime;
    v_ui.HighPart=v_ftime.dwHighDateTime;
    v_left=v_ui.QuadPart;

    v_res=v_right-v_left;

    v_ui.QuadPart=v_res;
    v_ftime.dwLowDateTime=v_ui.LowPart;
    v_ftime.dwHighDateTime=v_ui.HighPart;
    FileTimeToSystemTime(&v_ftime,rel);
}
*/