#ifndef CAMX_MIDIMONITOR
#define CAMX_MIDIMONITOR 1

#include "defines.h"

#ifdef WIN32
#include <AFXMT.h> // Semaphores
#endif

class MIDIMonitorKey
{
public:
	MIDIMonitorKey()
	{
		Reset();
		lastchannel=0;
	}

	void Reset()
	{
		endposition=-1;
		activecounter=thrucounter=0;
		active=false;
	}

	OSTART endposition;
	int  // active while playback
		thrucounter, // active thru
		activecounter;

	UBYTE lastchannel; //0-15
	bool active;
};

class MIDIMonitor
{
public:
	// Thru 
	void OpenThru(int key)
	{
		if(key<128)
		{
			Lock();

			keys[key].thrucounter++;
			keys[key].active=true;

			UnLock();
		}
#ifdef _DEBUG
		else
			MessageBox(NULL,"Illegal Monitor Key","Error",MB_OK);
#endif
	}

	void CloseThru(int key)
	{
		if(key<128)
		{
			Lock();

			if(keys[key].thrucounter)
			{
				if((--keys[key].thrucounter)==0)
					keys[key].active=false;	
			}
			else
				keys[key].active=false;

			UnLock();
		}
#ifdef _DEBUG
		else
			MessageBox(NULL,"Illegal Monitor Key","Error",MB_OK);
#endif
	}

	void OpenKey(UBYTE status,int key,OSTART endposition)
	{
		if(key<128)
		{
			Lock();

			keys[key].lastchannel=status&0x0F; //0-15
			keys[key].active=true;
			keys[key].activecounter++;

			if(endposition>keys[key].endposition)keys[key].endposition=endposition;

			UnLock();
		}
#ifdef _DEBUG
		else
			MessageBox(NULL,"Illegal Monitor Key","Error",MB_OK);
#endif
	}

	void CloseKey(int key)
	{
		if(key<128)
		{
			Lock();

			if(keys[key].activecounter)
			{
				if(--keys[key].activecounter==0)
					keys[key].active=false;
			}

			UnLock();
		}	
#ifdef _DEBUG
		else
			MessageBox(NULL,"Illegal Monitor Key","Error",MB_OK);
#endif
	}

	void ResetKeys()
	{
		Lock();
		for(int i=0;i<128;i++)keys[i].Reset();
		UnLock();
	}

	void Lock()
	{
#ifdef WIN32
		semaphore.Lock();
#endif
	}

	void UnLock()
	{
#ifdef WIN32
		semaphore.Unlock();
#endif
	}

	MIDIMonitorKey keys[128];

private:

#ifdef WIN32
	CCriticalSection semaphore;
#endif
};

#endif
