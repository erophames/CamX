#include "songmain.h"
#include "camxfile.h"

#include "languagefiles.h"
#include "gui.h"
#include "defines.h"
#include "threads.h"
#include "camxinfo.h"

#define RBUFFERSIZE 4096


#ifdef WIN64
#ifdef DEU
#define SETUPNAME "CamX DEUx64 Update"
#else
#define SETUPNAME "CamXx64 Update"
#endif
#else

#ifdef NOSSE2

#ifdef DEU
#define SETUPNAME "CamX DEU_SSE1 Update"
#else
#define SETUPNAME "CamX SSE1 Update"
#endif

#else

#ifdef DEU
#define SETUPNAME "CamX DEU Update"
#else
#define SETUPNAME "CamX Update"
#endif

#endif

#endif

enum UpdateGID
{
	GADGET_INFO=(GADGET_ID_START+50)+10,
	GADGET_STOP,
	GADGETID_VERSION,
	GADGETID_CHECKVERSION,
	GADGETID_INTERNETVERSION,
	GADGETID_DOWNLOADINTERNETVERSION,
	GADGETID_INFO,
};

int UpdaterDownloadThread::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)UpdaterDownloadFunc,(LPVOID)this, 0,0);
#endif

	return error;
}


// Extern Sync
PTHREAD_START_ROUTINE UpdaterDownloadThread::UpdaterDownloadFunc(LPVOID pParam) // MIDI Input Controll Thread, writes MIDI Buffer to Track
{
	UpdaterDownloadThread *mct=(UpdaterDownloadThread *)pParam;

	mct->started=true;
	mct->downloadcompleted=false;

	if(char *rbuffer=new char[RBUFFERSIZE])
	{
		mct->readcounter=0;

		DWORD r;
		BOOL bResult;

		// ReadLoop
		do
		{
			if(mct->stop==true)
				break;

			bResult = InternetReadFile(mct->hsetupInternet, rbuffer,RBUFFERSIZE, &r);

			if(bResult)
			{
				mct->writefile.Save(rbuffer,r);
				mct->readcounter+=r;
			}
			else
			{
				mct->error=true;
				break;
			}

		}while(r==RBUFFERSIZE);

		delete rbuffer;

		if(mct->stop==false && mct->error==false)
			mct->downloadcompleted=true;
	}
	else
		mct->error=true;

	mct->end=true;
	mct->ThreadGone();

	return(0);
}

void Seq_Main::InitUpdates()
{
	if(char *setupfile=mainvar->GenerateString(mainvar->GetUserDirectory(),"\\Updates\\update.exe"))
	{
		ShellExecute(0,"open",setupfile, NULL, NULL, SW_SHOWNORMAL);
		delete setupfile;
	}
}

guiMenu *Edit_UpDate::CreateMenu()
{
	if(menu=new guiMenu)
	{
		guiMenu *sub=menu->AddMenu("CamX UpDate",0);

		if(sub)
		{
			sub->AddMenu("CamX Audio MIDI Sequencer",0);
		}
	}

	return menu;
}

void Edit_UpDate::RefreshRealtime()
{
	if(stoprefresh==true)
		return;

	if(uploadthread.started==true)
	{
		if(infobutton)
		{
			char h2[NUMBERSTRINGLEN];

			if(infostring)
				delete infostring;

			infostring=mainvar->GenerateString("Download:",mainvar->ConvertIntToChar(uploadthread.readcounter,h2),uploadthread.end==true?"/":"...");

			if(infostring)
				infobutton->ChangeButtonText(infostring);

			if(uploadthread.end==true)
			{
				stoprefresh=true;

				if(stopbutton)
				{
					if(uploadthread.downloadcompleted==true)
					{
						stopbutton->Enable();
						stopbutton->ChangeButtonText(Cxs[CXS_UPDATERESTART]);
					}
					else
						stopbutton->Disable();
				}

				if(uploadthread.stop==true)
				{
					deletefiles=true;
					closeit=true;
				}
				else
					if(uploadthread.error==true)
					{
						if(question==false)
						{
							question=true;

							int b=MessageBox(
								0,          // handle of owner window
								Cxs[CXS_ERRORREADINGINTERNETFILE],     // address of text in message box
								"Updater Error",  // address of title of message box
								MB_OK  // style of message box
								);

							deletefiles=true;
						}
					}
					else
					{
						if(question==false)
						{
							question=true;

							char *h=mainvar->GenerateString(Cxs[CXS_UPLOADCOMPLETERESTART_Q],"\n",Cxs[CXS_EXITUPDATE_Q]);

							if(h)
							{
								/*
								int b=MessageBox(
								0,          // handle of owner window
								h,     // address of text in message box
								"Updater",  // address of title of message box
								MB_ICONQUESTION| MB_YESNO | MB_TASKMODAL | MB_TOPMOST  // style of message box
								);
								*/
								if(maingui->MessageBoxYesNo(0,h)==true)
								{
									mainvar->updatereset=true;
									mainvar->AskQuitMessage();
								}

								delete h;
							}
						}
					}
			}
		}
	}
	else
	{
		if(infobutton)
			infobutton->ChangeButtonText("...");
	}
}

Edit_UpDate::Edit_UpDate(bool errmsg)
{
	editorid=EDITORTYPE_UPDATE;

	errormsg=errmsg;
	stoprefresh=false;
	infostring=0;
	question=false;
	deletefiles=false;

	InitForms(FORM_PLAIN1x1);
	resizeable=true;
	ondesktop=true;
	dialogstyle=true;

	foundnewversion=false;

	minwidth=maingui->GetButtonSizeY(24);
	maxwidth=maingui->GetButtonSizeY(28);
	maxheight=minheight=maingui->GetButtonSizeY(6);
}

void Edit_UpDate::CheckUpdate()
{
	if(foundnewversion==true)
		return;

	UpdateInfo uinfo;

	if(downloadinetversion)
	{
		downloadinetversion->Disable();
	}

	int uflag=mainvar->CheckForUpdates(false,&uinfo);

	if(uinfo.iversion>0)
	{
		if(uinfo.iversion>maingui->GetVersion())
		{
			char versionstring[128];

			strcpy(versionstring,"Internet Version: ");
			char h2[NUMBERSTRINGLEN];

			int f=uinfo.iversion/1000;
			int p=uinfo.iversion-(f*1000);

			mainvar->AddString(versionstring,mainvar->ConvertIntToChar(f,h2));

			if(p<10)
				mainvar->AddString(versionstring,".00");
			else
				if(p<100)
					mainvar->AddString(versionstring,".0");
				else
					mainvar->AddString(versionstring,".");

			mainvar->AddString(versionstring,mainvar->ConvertIntToChar(p,h2));

			if(inetversion)
				inetversion->ChangeButtonText(versionstring);

			foundnewversion=true;

			if(downloadinetversion)
			{
				downloadinetversion->Enable();
				downloadinetversion->ChangeButtonText(Cxs[CXS_DOWNLOADUPDATE]);
			}
		}
		else
		{
			if(inetversion)
				inetversion->ChangeButtonText(Cxs[CXS_UPTODATE]);
		}

		return;
	}
	else
	{
		if(inetversion)
			inetversion->ChangeButtonText(Cxs[CXS_NOURLINFO]);
	}
}

void Edit_UpDate::StartUpdate()
{
	if(foundnewversion==false)
		return;

	if(uploadthread.downloadcompleted==true)
	{
		return;
	}

	if(uploadthread.started==false)
	{
		int flag=0;

		uploadthread.setupfile=mainvar->GenerateString(mainvar->GetUserDirectory(),"\\Updates\\update.exe");
		if(!uploadthread.setupfile)
			return ;

		uploadthread.versionfile=mainvar->GenerateString(mainvar->GetUserDirectory(),"\\Updates\\version.txt");
		if(!uploadthread.versionfile)
		{
			delete uploadthread.setupfile;
			return;
		}

		//Initializes an application's use of the WinINet functions.
		if(uploadthread.hSession = InternetOpen("CamX Update Check", INTERNET_OPEN_TYPE_DIRECT, "", "", 0))
		{
			if(char *setupname=mainvar->GenerateString("http://www.camx.de/",SETUPNAME,".exe"))
			{
				// Copy Inet Setup->Updates
				if(uploadthread.hsetupInternet = InternetOpenUrl(uploadthread.hSession, setupname, "", 0, INTERNET_FLAG_NO_CACHE_WRITE, 0))
				{
					if(uploadthread.writefile.OpenSave(uploadthread.setupfile)==true) // + Delete Old Version File
					{
						uploadthread.started=true;

						if(stopbutton)
						{
							stopbutton->Enable();
						}

						uploadthread.StartThread();
					}
					else
					{
						InternetCloseHandle(uploadthread.hsetupInternet);
						InternetCloseHandle(uploadthread.hSession);
					}
				}
				else
					InternetCloseHandle(uploadthread.hSession);

				delete setupname;
			}
			else
				InternetCloseHandle(uploadthread.hSession);
		}
	}
}

void Edit_UpDate::Init()
{
	glist.SelectForm(0,0);

	char versionstring[128];

	strcpy(versionstring,"CamX ");
	char h2[NUMBERSTRINGLEN];

	int f=maingui->GetVersion()/1000;
	int p=maingui->GetVersion()-(f*1000);

	mainvar->AddString(versionstring,mainvar->ConvertIntToChar(f,h2));

	if(p<10)
		mainvar->AddString(versionstring,".00");
	else
		if(p<100)
			mainvar->AddString(versionstring,".0");
		else
			mainvar->AddString(versionstring,".");

	mainvar->AddString(versionstring,mainvar->ConvertIntToChar(p,h2));

	glist.AddButton(-1,-1,-1,-1,versionstring,GADGETID_VERSION,MODE_RIGHT|MODE_TEXTCENTER|MODE_NOMOUSEOVER);
	glist.Return();

	glist.AddButton(-1,-1,-1,-1,Cxs[CXS_CHECKFORUPDATES],GADGETID_CHECKVERSION,MODE_RIGHT);
	glist.Return();

	inetversion=glist.AddButton(-1,-1,-1,-1,"-Version ?-",GADGETID_INTERNETVERSION,MODE_RIGHT|MODE_TEXTCENTER|MODE_NOMOUSEOVER);
	glist.Return();

	downloadinetversion=glist.AddButton(-1,-1,-1,-1,"-",GADGETID_DOWNLOADINTERNETVERSION,MODE_RIGHT);
	if(downloadinetversion)
		downloadinetversion->Disable();

	glist.Return();

	infobutton=glist.AddButton(-1,-1,-1,-1,"...",GADGETID_INFO,MODE_LEFTTOMID|MODE_BOTTOM|MODE_TEXTCENTER|MODE_NOMOUSEOVER);
	glist.AddLX();
	stopbutton=glist.AddButton(-1,-1,-1,-1,"< Stop",GADGET_STOP,MODE_MIDTORIGHT|MODE_BOTTOM|MODE_TEXTCENTER);
	if(stopbutton)
		stopbutton->Disable();

	glist.Return();
}

void Edit_UpDate::Gadget(guiGadget *gadget)
{
	switch(gadget->gadgetID)
	{
	case GADGET_STOP:
		if(uploadthread.downloadcompleted==true)
		{
			mainvar->updatereset=true;
			mainvar->AskQuitMessage();
		}
		else
			uploadthread.stop=true;
		break;

	case GADGETID_CHECKVERSION:
		CheckUpdate();
		break;

	case GADGETID_DOWNLOADINTERNETVERSION:
		StartUpdate();
		break;
	}
}

void Edit_UpDate::DeInitWindow()
{
	if(uploadthread.started==true && uploadthread.end==false)
	{
		uploadthread.stop=true; // Stop Thread

		while(uploadthread.end==false) // Poll
		{
			Sleep(50);
		}
	}

	uploadthread.writefile.Close(true);

	// Delete Update Files
	if(deletefiles==true)
	{
		if(uploadthread.versionfile)
			mainvar->DeleteAFile(uploadthread.versionfile);

		if(uploadthread.setupfile)
			mainvar->DeleteAFile(uploadthread.setupfile);
	}

	if(uploadthread.versionfile)
		delete uploadthread.versionfile;

	if(uploadthread.setupfile)
		delete uploadthread.setupfile;

	if(uploadthread.hsetupInternet)
		InternetCloseHandle(uploadthread.hsetupInternet);

	if(uploadthread.hSession)
		InternetCloseHandle(uploadthread.hSession);

	if(infostring)
		delete infostring;
}


int Seq_Main::CheckForUpdates(bool errormsg,UpdateInfo *updateinfo)
{
	/*
	guiWindow *w=maingui->FirstWindow();
	while(w)
	{
		if(w->GetEditorID()==EDITORTYPE_UPDATE)return 0;
		w=w->NextWindow();
	}
*/

	if(updateinfo)
	{
		updateinfo->iversion=-1;
	}

	int versioninternet=0,patchversion=0; // 0= Not found
	DWORD readed;
	char buffer[256];

	int flag=0;

	char  *setupfile=mainvar->GenerateString(mainvar->GetUserDirectory(),"\\Updates\\update.exe");
	if(!setupfile)return 0;

	char *versionfile=mainvar->GenerateString(mainvar->GetUserDirectory(),"\\Updates\\version.txt");
	if(!versionfile)
	{
		delete setupfile;
		return 0;
	}

	// Read Version
	camxFile readupdate;
	if(readupdate.OpenRead(setupfile)==true)
	{
		camxFile read;

		if(read.OpenRead(versionfile)==true)
		{
			char vbuffer[32];
			int r=read.Read(vbuffer,32);

			if(r>0)
			{
				char version[5];

				version[0]=vbuffer[0];
				version[1]=vbuffer[1];
				version[2]=vbuffer[2];
				version[3]=vbuffer[3];
				version[4]=0;

				patchversion=mainvar->ConvertCharToInt(version);
			}
		}

		read.Close(true);
	}
	readupdate.Close(true);

	//Initializes an application's use of the WinINet functions.
	if(HINTERNET hSession = InternetOpen("CamX Update Check", INTERNET_OPEN_TYPE_DIRECT, "", "", 0))
	{
		TRACE ("InterNet Session open\n");
		if(HINTERNET hInternet = InternetOpenUrl(hSession, "http://www.camx.de/update.txt", "", 0, INTERNET_FLAG_NO_CACHE_WRITE, 0))
		{
			TRACE ("Zugriff auf Update Text\n");
			BOOL bResult = InternetReadFile(hInternet, buffer,255, &readed);

			if(bResult && readed>=4)
			{
				char v[5];
				memcpy(v,buffer,4);
				v[4]=0; // Version

				versioninternet=mainvar->ConvertCharToInt(buffer);

				if(updateinfo)
				{
				updateinfo->iversion=versioninternet;
				}

				if(versioninternet>patchversion)
				{
					flag=maingui->GetVersion()<versioninternet?UPDATE_FLAG_FOUNDNEWVERSION:UPDATE_FLAG_NONEWVERSION;
				}


#ifdef DEBUG
				// flag|=UPDATE_FLAG_FOUNDNEWVERSION;
#endif

				if(!updateinfo)
				{
					if(flag&UPDATE_FLAG_FOUNDNEWVERSION)
					{
						char nversionstring[64],thisversionstring[64],h2[NUMBERSTRINGLEN];
						int f=versioninternet/1000,p=versioninternet-(f*1000);

						strcpy(nversionstring,mainvar->ConvertIntToChar(f,h2));
						if(p<10)
							mainvar->AddString(nversionstring,".00");
						else
							if(p<100)
								mainvar->AddString(nversionstring,".0");
							else
								mainvar->AddString(nversionstring,".");

						mainvar->AddString(nversionstring,mainvar->ConvertIntToChar(p,h2));

						f=maingui->GetVersion()/1000;
						p=maingui->GetVersion()-(f*1000);

						strcpy(thisversionstring,mainvar->ConvertIntToChar(f,h2));

						if(p<10)
							mainvar->AddString(thisversionstring,".00");
						else
							if(p<100)
								mainvar->AddString(thisversionstring,".0");
							else
								mainvar->AddString(thisversionstring,".");

						mainvar->AddString(thisversionstring,mainvar->ConvertIntToChar(p,h2));

						char *vh=mainvar->GenerateString(Cxs[CXS_FOUNDNEWVERSION],":",nversionstring,"\n");

						if(vh)
						{
							char *comp=mainvar->GenerateString(vh,Cxs[CXS_THSISVERSIONIS],":",thisversionstring,"\n");
							delete vh;

							if(comp)
							{
								char *q=mainvar->GenerateString(comp,Cxs[CXS_DOWNLOADUPDATE],"?");
								delete comp;

								if(q)
								{
									if(maingui->MessageBoxYesNo(0,q)==true)
										flag=UPDATE_FLAG_INITUPDATE;

									/*
									int b=MessageBox(
									0,          // handle of owner window
									q,     // address of text in message box
									"Updater",  // address of title of message box
									MB_ICONQUESTION| MB_YESNO | MB_TASKMODAL | MB_TOPMOST  // style of message box
									);

									if(b==IDYES)
									flag=UPDATE_FLAG_INITUPDATE;
									*/

									delete q;
								}
							}
						}
					}

				}//new version
				else
				{
					if(errormsg==true)
					{
						maingui->MessageBoxOk(0,Cxs[CXS_UPTODATE]);
					}

					/*
						MessageBox(
						0,          // handle of owner window
						Cxs[CXS_UPTODATE],     // address of text in message box
						"CamX Update",  // address of title of message box
						MB_OK // style of message box
						);
*/

				}
			}
			else
			{
				if(errormsg==true)
					maingui->MessageBoxOk(0,Cxs[CXS_ERRORREADINGINTERNETFILE]);

				/*
					MessageBox(
					0,          // handle of owner window
					Cxs[CXS_ERRORREADINGINTERNETFILE],     // address of text in message box
					"CamX Update Error",  // address of title of message box
					MB_OK // style of message box
					);
*/

				flag=UPDATE_FLAG_NOFILE;
			}

			InternetCloseHandle(hInternet);
		}
		else
		{
			if(errormsg==true)
				maingui->MessageBoxOk(0,Cxs[CXS_NOURLINFO]);

			/*
				MessageBox(
				0,          // handle of owner window
				Cxs[CXS_NOURLINFO],     // address of text in message box
				"CamX Update Error",  // address of title of message box
				MB_OK // style of message box
				);
*/

			flag=UPDATE_FLAG_NOURL;
		}

		InternetCloseHandle(hSession);

		if(flag&UPDATE_FLAG_INITUPDATE) // Load setupexe->Update
		{
			// Write Update Text
			camxFile write;

			if(write.OpenSave(versionfile)==true)
			{
				write.Save(buffer,readed);
				write.Close(true);
			}

			maingui->OpenEditorStart(EDITORTYPE_UPDATE,0,0,0,0,errormsg?(Object *)1:0,0);
		
		}

#ifdef DEBUG
		//maingui->OpenEditorStart(EDITORTYPE_UPDATE,0,0,0,0,errormsg?(Object *)1:0,0);
#endif

	} // if hsession
	else
	{
		if(errormsg==true)
			maingui->MessageBoxOk(0,Cxs[CXS_NOINTERNET]);

		/*
			MessageBox(
			0,          // handle of owner window
			Cxs[CXS_NOINTERNET],     // address of text in message box
			"CamX Update Error",  // address of title of message box
			MB_OK_STYLE // style of message box
			);
*/

		flag=UPDATE_FLAG_NOINTERNET;
	}

	if(!(flag&UPDATE_FLAG_INITUPDATE))
	{
		if(patchversion>maingui->GetVersion())
		{

			/*
			int b=MessageBox(
				0,          // handle of owner window
				Cxs[CXS_EXITUPDATE_Q],     // address of text in message box
				"Updater",  // address of title of message box
				MB_ICONQUESTION| MB_YESNO | MB_TASKMODAL | MB_TOPMOST  // style of message box
				);
*/

			if(maingui->MessageBoxYesNo(0,Cxs[CXS_EXITUPDATE_Q])==true)
			{
				mainvar->updatereset=true;

				if(maingui->allwindowsoondesktop==false)
					mainvar->AskQuitMessage();

				flag |=UPDATE_FLAG_INITUPDATEFILE;
			}
		}
	}

	delete versionfile;
	delete setupfile;

	return flag;
}