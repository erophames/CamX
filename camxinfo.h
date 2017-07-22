#ifndef CAMX_INFOGUI_H
#define CAMX_INFOGUI_H 1

#include "wininet.h"
#include "editortypes.h"

enum InfoFlag{
	CINFO_STOPBUTTON=1
};

class Edit_CamXInfo:public guiWindow
{
public:
	Edit_CamXInfo();
	guiMenu *CreateMenu();

	void ShowInfo();
	void Init();
	void DeInitWindow();

	guiGadget_CW *text;

private:
	int infoflag;
};

class UpdaterDownloadThread:public Thread
{
public:
	UpdaterDownloadThread()
	{
		error=false;
		stop=false;
		end=false;
		started=false;
		downloadcompleted=false;

		hSession=0;
		hsetupInternet=0;
		setupfile=0;
		versionfile=0;
	}

	int StartThread();

#ifdef WIN32
	static PTHREAD_START_ROUTINE UpdaterDownloadFunc (LPVOID pParam);
#endif

	camxFile writefile;

	int readcounter;
	bool end;
	bool stop;
	bool error;
	bool started;
	bool downloadcompleted;

	char *setupfile;
	char *versionfile;

	HINTERNET hSession;
	HINTERNET hsetupInternet;
};


class Edit_UpDate:public guiWindow
{
public:
	Edit_UpDate(bool errmsg);
	guiMenu *CreateMenu();
	void Init();
	void Gadget(guiGadget *);
	void RedrawGfx(){Init();}
	void RefreshRealtime();

	void CheckUpdate();
	void StartUpdate();
	void DeInitWindow();

	UpdaterDownloadThread uploadthread;

private:
	char *infostring;
	bool foundnewversion;
	bool errormsg;
	bool stoprefresh;
	guiGadget *inetversion,*downloadinetversion,*infobutton,*stopbutton;

	bool question;
	bool deletefiles;
};

#endif