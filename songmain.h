#ifndef CAMX_SONGMAIN_H
#define CAMX_SONGMAIN_H 1

#include "object.h"
#include "defines.h"
#include "progress.h"

class Seq_Song;
class Seq_Project;
class Seq_Marker;
class Seq_Track;
class Seq_Pattern;
class Seq_Group;
class AudioChannel;
class guiWindow;
class Colour;
class Drumtrack;
class guiScreen;
class AudioDevice;

enum
{
	UPDATE_FLAG_NOINTERNET=1,
	UPDATE_FLAG_NOFILE=2,
	UPDATE_FLAG_NONEWVERSION=4,
	UPDATE_FLAG_FOUNDNEWVERSION=8,
	UPDATE_FLAG_NOURL=16,
	UPDATE_FLAG_INITUPDATE=32,
	UPDATE_FLAG_RESTART=64,
	UPDATE_FLAG_INITUPDATEFILE=128
};

// Root Class **************************
class UpdateInfo
{
public:
	int iversion;
};

class Seq_Main // Main Class
{
public:
	Seq_Main();
	~Seq_Main(){if(stripExtension_string)delete stripExtension_string;}

	void SetExitProgram(){exitprogram_flag=true;}

	void InitUpdates();
	int CheckForUpdates(bool errormsg,UpdateInfo *updateinfo=0);

	void ResetPeakCheck()
	{
#ifdef WIN32
		lastpeakcheck=GetTickCount();
#endif
	}

	// Autosave;
	void ResetAutoSave()
	{
#ifdef WIN32
		lastautosaved=GetTickCount();
#endif
	}
	void CheckAutoSave();

	void InitQuantList(int ppqrate);

	int strcmp_allsmall(char *s1,char *s2);
	//bool CheckRunningCamXVersion();
	int InitHardware();
	void CloseAllHardware();
	void StopAllHardware();
	void AskQuitMessage(bool save=true);
	void ExitProgram();
	
	int InitThreads();
	void CloseAllThreads();
	bool GetFileName(char *fname,char *string,int length);

	void NewSong(Seq_Project *,guiScreen *screen=0);

	// Some tool functions
	void SetCamXDirectory(char *);
	char *GetCamXDirectory(){return currentdir;}
	void SetUserDirectory(char *);
	char *GetUserDirectory(){return userdir;}
	void AddString(char *string,char *add);

	char *GenerateString(char *);
	char *GenerateString(char *,char *);
	char *GenerateString(char *,char *,char *);
	char *GenerateString(char *,char *,char *,char *);
	char *GenerateString(char *,char *,char *,char *,char *);
	char *GenerateString(char *,char *,char *,char *,char *,char *);
	char *GenerateString(char *,char *,char *,char *,char *,char *,char *);

	bool CheckIfInIndex(int startindex,int endindex,int index);
	bool CheckIfInIndex(double startindex,double endindex,double index);

	bool CheckIfInPosition(OSTART p1,OSTART p2,OSTART pos);

	char *stripExtension(char *);
	bool CompareStringWithOutZero(char *,char *);
	void MixString(char *to,char *from1,char *from2);
	char *GetQuantString(int q_nr);
	char *ClearString(char *);
	char *CreateSimpleASCIIString(char *);
	char *ConvertTicksToString(char *,OSTART ticks);
	double ConvertCharToDouble(char *,int digits=-1);
	int ConvertCharToInt(char *);
	char *ConvertFloatToChar(float,char *,int digits);
	char *ConvertDoubleToChar(double,char *,int digits);
	void SplitDoubleString(char *,char *intpart,char *fpart);
	char *ConvertIntToChar(int,char *);


	char *ConvertLongLongToChar(LONGLONG,char *);

	// Tools
	OSTART ConvertMilliSecToTicks(double ms);
	char *ConvertTicksToChar(int);
	char *ConvertSamplesToTime(LONGLONG,int timetype,char *string);
	// Tools 

	// Quantize
	OSTART SimpleQuantize(OSTART,OSTART qticks);
	OSTART SimpleQuantizeLeft(OSTART,OSTART qticks);
	OSTART SimpleQuantizeRight(OSTART,OSTART qticks);

	// DOS
	bool CreateNewDirectory(char *);
	bool DeleteAFile(char *);
	void DeletePeakFile(char *);
	bool DeleteADirectory(char *);
	int GetDirectoryLengthFromFileString(char *);
	void SetError(int errorflag);

	// Projects ----------
	bool CheckSampleRateOfNewDevice(AudioDevice *);
	Seq_Project* FirstProject(){return (Seq_Project *)projects.GetRoot();}
	Seq_Project* LastProject(){return (Seq_Project *)projects.Getc_end();}

	Seq_Song* GetActiveSong();
	void ChangeAllToSong(Seq_Song *,Seq_Project *,bool setactive=true);
	void SetActiveSong(Seq_Song *);
	void SetActiveProject(Seq_Project *,Seq_Song *);
	void LoadProject(guiScreen *);
	void SaveProject(Seq_Project *);
	void QuestionCloseProject(Seq_Project *);
	Seq_Project* GetActiveProject();
	Seq_Project* GetProjectIndex(int index){ return (Seq_Project *)projects.GetO(index); }
	Seq_Project *CreateNewProjectWithNewDirectory(guiScreen *,int flag=0);
	Seq_Project *CreateProject(char *name,bool *exists,bool warning);
	Seq_Project *CloseProject(Seq_Project *);
	Seq_Project *OpenProject(guiScreen *,char *directory);
	void SaveAllProject();
	void CloseAllProjects();
	void Init(); // main Init

	Progress *FirstProgress(){return (Progress *)progress.GetRoot();}

	// Progress
	void AddProgress(Progress *p){progress.AddEndO(p);}
	void RemoveProgress(Progress *p){progress.CutQObject(p);}

	bool FindWindow(guiWindow *);

	OList progress;
	char tickstochar[32];
	int lastpeakcheck,lastautosaved,updateflag,cpucores,numberwinzooms;

	// End of program , 0 while running
	bool saveonexit,exitprogram_flag,exithardware,exitthreads,updatereset,autoupdatecheck;

private:
	OList projects;
	Seq_Project *activeproject;
	int errorflag;
	char currentdir[1026],userdir[MAX_PATH+32],*stripExtension_string;
};
#endif
