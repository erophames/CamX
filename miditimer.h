#ifndef CAMX_MTIMER_H
#define CAMX_MTIMER_H 1

#include "defines.h"
#include <windows.h>

class sysTimer
{
public:
	bool Init();
	void Exit();

	inline double ConvertSysTimeToMs(LONGLONG systicks){return (double)systicks*systomsmul;}
	inline LONGLONG ConvertMsToSysTime(double ms){return (LONGLONG)(ms*mstosysmul);}
	inline double ConvertSysTimeToInternTicks(LONGLONG systicks){return (double)systicks*systointernratemul;}
	inline LONGLONG ConvertInternTicksToSysTime(LONGLONG ticks){return (LONGLONG)((double)ticks*internratetosysmul);}

	inline LONGLONG GetSystemTime(){
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		return time.QuadPart;
	}

	HANDLE hTimerQueue;
	LONGLONG sysfreq;
	LARGE_INTEGER freq;
	double systomsmul,mstosysmul,systointernratemul,internratetosysmul;
};
#endif