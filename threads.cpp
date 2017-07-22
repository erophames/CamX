#include "threads.h"

void Thread::ThreadGone()
{
	gone.SetEvent();
}

void Thread::SendQuit()
{
	exithread=true;

	for(;;)
	{
		SetSignal();
	
		DWORD status=WaitForSingleObject(gone, 10 /*INFINITE*/);

		if(status==WAIT_OBJECT_0)
			break;
	}
}