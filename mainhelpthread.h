#ifndef CAMX_MAINHELPTHREAD_H
#define CAMX_MAINHELPTHREAD_H 1

#include "defines.h"
#include "threads.h"

class guiWindow;

class HelpMessage:public Object
{
	friend class MainHelpThread;

public:
	HelpMessage *NextMessage() {return(HelpMessage *)next;}

	guiWindow *win;
	int counter,msgtype,intern;
	void *par1;
};

class MainHelpThread:public Thread // GUI Messages etc..
{
public:
#define helpthreadmsdelay 20

	HelpMessage *FirstMessage(){return (HelpMessage *)messages.GetRoot();}
	void AddMessage(int ms,guiWindow *win,int type,int intern,void *par1);
	void RemoveWindowFromMessages(guiWindow *win);

#ifdef WIN32
	static PTHREAD_START_ROUTINE ThreadFunc(LPVOID pParam);
	int StartThread();
	void WaitLoop();
#endif

private:
	OList messages;
};

class MainSyncThread:public Thread
{
public:
	MainSyncThread()
	{
		startactivesong=stopactivesong=false;
	}

	int StartThread();
	void WaitLoop();

#ifdef WIN32
	static PTHREAD_START_ROUTINE MainSyncThreadFunc (LPVOID pParam);
#endif
	
	bool startactivesong,stopactivesong;
};
#endif