#include "songmain.h"
#include "mainhelpthread.h"
#include "semapores.h"
#include "object_song.h"
#include "MIDIoutproc.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "MIDIthruproc.h"
#include "MIDIinproc.h"
#include "audioproc.h"
#include "audiorecord.h"

ThreadControl::ThreadControl()
{
	for(int i=0;i<CS_CRITICALSECTIONSEND;i++)
	{
		InitializeCriticalSection(&cs[i]);

		/*
		// Initialize the critical section one time only.
		if (!InitializeCriticalSectionAndSpinCount(&cs[i], 
		0x00000400) ) 
		return;
		*/
	}
}

ThreadControl::~ThreadControl()
{
	for(int i=0;i<CS_CRITICALSECTIONSEND;i++)
	{
		DeleteCriticalSection(&cs[i]);
	}
}

void ThreadControl::LockActiveSong()
{
	mainhelpthread->Lock();
	mainsyncthread->Lock();
	mainMIDIthruthread->Lock();
	mainMIDIalarmthread->Lock();
	MIDIinproc->Lock();
	plugininproc->Lock();
	mainaudiostreamproc->Lock();
	Lock(CS_audioplayback);
	audiorecordthread->Lock();
	Lock(CS_audioinput);
	MIDIrtealarmproc->Lock();
}

void ThreadControl::UnlockActiveSong()
{
	MIDIrtealarmproc->Unlock();

	Unlock(CS_audioinput);
	
	audiorecordthread->Unlock();
	Unlock(CS_audioplayback);

	mainaudiostreamproc->Unlock();
	plugininproc->Unlock();
	MIDIinproc->Unlock();
	mainMIDIalarmthread->Unlock();
	mainMIDIthruthread->Unlock();
	
	mainsyncthread->Unlock();
	mainhelpthread->Unlock();
}