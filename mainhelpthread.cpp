#include "defines.h"
#include "songmain.h"

#ifdef WIN32
#include <mmsystem.h>
#endif

#include "gui.h"
#include "semapores.h"
#include "mainhelpthread.h"
#include "object_song.h"
#include "MIDIhardware.h"

/*
typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)(
    LPVOID lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
*/


int MainHelpThread::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)ThreadFunc,(LPVOID)this, 0,0);
	if(!ThreadHandle)error=1;
#endif

	return error;
}

#define TIMERMS 50

void MainHelpThread::WaitLoop()
{
	while(IsExit()==false)
	{
		Lock();

		if(FirstMessage())
		{
			Unlock();
			WaitSignal(helpthreadmsdelay);
		}
		else
		{
			Unlock();
			WaitSignal(TIMERMS);
		}

		if(IsExit()==true)
			break;

		Lock();

		//	mainthreadcontrol->Lock(CS_gui);

		// Check Messages
		HelpMessage *hm=FirstMessage();

		while(hm)
		{
			if(hm->counter>0)
				hm->counter--;

			if(hm->counter==0)
			{
				guiWindow *win=hm->win;
				int msgtype=hm->msgtype;
				void *par=hm->par1;

				hm=(HelpMessage *)messages.RemoveO(hm);

				Unlock(); // avoid OS loop/lock problems
				maingui->SendGUIMessage(win,msgtype,par);
				Lock();	
			}
			else
				hm=hm->NextMessage();
		}

		Unlock();

		//maingui->TimerCall();
		//MessageBeep(-1);

		//	mainthreadcontrol->Unlock(CS_gui);

	}

	ThreadGone();
}

PTHREAD_START_ROUTINE MainHelpThread::ThreadFunc(LPVOID pParam)
{
	MainHelpThread *mht=(MainHelpThread *)pParam;
	mht->WaitLoop();
	return(0);
}

void MainHelpThread::AddMessage(int ms,guiWindow *win,int type,int intern,void *par)
{
	int counter=ms/helpthreadmsdelay;

	if(counter)
	{
		if(HelpMessage *hm=new HelpMessage)
		{
			hm->counter=counter;
			hm->win=win;
			hm->msgtype=type;
			hm->par1=par;
			hm->intern=intern;

			Lock();
			messages.AddEndO(hm);
			Unlock();

			SetSignal();
		}
	}
	else
		maingui->SendGUIMessage(win,type,par);
}

void MainHelpThread::RemoveWindowFromMessages(guiWindow *win)
{
	Lock();

	HelpMessage *hm=FirstMessage();

	while(hm)
	{
		if(hm->win==win)
			hm=(HelpMessage *)messages.RemoveO(hm);
		else
			hm=hm->NextMessage();
	}

	Unlock();
}

// Extern Sync

void MainSyncThread::WaitLoop()
{
	while(IsExit()==false)
	{
		WaitSignal();

		if(IsExit()==true)
			break;

		Lock();

		Seq_Song *song=mainvar->GetActiveSong();

		if(song)
		{
			if(song->newsongpositionset==true)
			{
				song->newsongpositionset=false;

				mainthreadcontrol->Lock(CS_SONGCONTROL);
				song->MIDIInputDeviceToSongPosition(/*STOPSELECT_SYNC|*/SETSONGPOSITION_NOGUI);
				mainthreadcontrol->Unlock(CS_SONGCONTROL);
			}
		}

		// PLAY SONG >>>>
		if(startactivesong==true)
		{
			startactivesong=false;

			switch(mainMIDI->receiveMIDIstart)
			{
			case RECEIVEMIDISTART_PLAYBACK:
				if(song)
				{
					if((song->status&(Seq_Song::STATUS_PLAY|Seq_Song::STATUS_RECORD))==0)
						song->PlaySong();
				}
				break;

			case RECEIVEMIDISTART_RECORD:
				if(song)
				{
					if((song->status&Seq_Song::STATUS_RECORD)==0)
						song->RecordSong();
				}
				break;

			case RECEIVEMIDISTART_RECORD_NOPRECOUNTER:
				if(song)
				{
					song->RecordSong(RECORD_SKIPPRECOUNTER);
				}
				break;
			}
		}

		// STOP SONG
		if(stopactivesong==true)
		{
			stopactivesong=false;

			if(song && mainMIDI->receiveMIDIstop==true)
			{
				song->StopSelected();
			}
		}

		Unlock();
	}

	ThreadGone();
}

PTHREAD_START_ROUTINE MainSyncThread::MainSyncThreadFunc(LPVOID pParam) // MIDI Input Controll Thread, writes MIDI Buffer to Track
{
	MainSyncThread *mct=(MainSyncThread *)pParam;
	mct->WaitLoop();
	return(0);
}

int MainSyncThread::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)MainSyncThreadFunc,(LPVOID)this, 0,0);
	if(!ThreadHandle)error=1;
	
	//else
	//	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL); // > Main Thread
#endif

	return error;
}
