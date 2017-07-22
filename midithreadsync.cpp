/*
#include "defines.h"
#include "songmain.h"
// #include "freeze.h"

#ifdef WIN32

//#include <windows.h>
#include <mmsystem.h>
#endif

#include "semapores.h"


// Audio Thread Sync
void ThreadControl::SendAudioBufferSignal(int id)
{
#ifdef WIN32
	audiobuffersignal_playback[id].SetEvent();

	audiobuffersignal_record[id].SetEvent();
#endif
}
*/