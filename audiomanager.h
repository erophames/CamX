#ifndef CAMX_AUDIOMANAGEREDITOR_H
#define CAMX_AUDIOMANAGEREDITOR_H 1

#include "editor.h"
#include "searchfilter.h"

class Decoder;
class AudioHDFile;
class AudioRegion;

class Sort_Files:public Object
{
public:
	AudioHDFile *hdfile;
	Sort_Files *NextSortFile(){return (Sort_Files *)next;}

	char *sfname;
};

class Edit_Manager:public guiWindow
{
public:

	enum
	{
		SORTBYNAME=1,
		SORTBYSIZE=2,
		SORTBYDATE=4
	};

	Edit_Manager();

	Object *GetDragDrop(HWND wnd,int index);
	void DragDropFile(char *,guiGadget *);

	void ResetMenu();
	bool CheckForFile(int type);
	void SetActiveFile(Sort_Files *,bool refreshgui);
	
	void ChangeSortFile(Sort_Files *,bool refreshgui=true);
	void ShowPeakProgress(double per);
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	void ShowStatus();
	void CreateWindowMenu(); // v
	void RefreshObjects(LONGLONG type,bool editcall); // v
	
	void MouseClickInGFX();
	void ShowAudioFiles();
	void ShowStartPosition();
	void RefreshHDFile(AudioHDFile *);
	void ShowActiveHDFilePositions();
	void ShowActiveHDFile_Regions();
	void ShowActiveHDFile_Info();
	void ShowMenu();
	void AddFilesFromDirectory();
	void AddFile();
	void AddFile(char *);
	void InitGadgets();
	//void InitFrames();
	void CopyActiveFile();
	void CopyActiveRegion();
	void DecodeFile();
	void ResamplingFile();
	void StopPlayback();

	LONGLONG ConvertXToPositionX(int x);
	int ConvertSamplePositionX(LONGLONG);

	void InitList();
	Sort_Files *FindAudioHDFile(AudioHDFile *);
	Sort_Files *GetSortFileIndex(int index);
	
	void Init();
	void EditActiveFile(bool region);
	void DeInitWindow();
	void Gadget(guiGadget *);
	void KeyDown();
	void CopyHDFileToBuffer();
	void CopyAudioRegionToBuffer();
//	void LoadDataBase();
//	void SaveDataBase();
	void ShowDisplayFilter();

	void Play();
	void PlayRegion();

	void ShowSound();

	AudioHDFile *GetActiveHDFile();
	Sort_Files *activefile;
	AudioRegion *activeregion;
	bool showfullpath,showintern,showmb,showtime,showrecorded,shownotfound,showinfo,showregions,showonlysamplerate;
	Sort_Files *FirstSortFile(){return (Sort_Files *)files.GetRoot();}

	guiGadget_CW *showsound;

private:
	void CheckActiveHD();
	void ShowFilter();
	
	void DeleteTestRegion();
	void DeleteAllSortFiles();

	OList files; // sorted list, Sort_Files
	SearchFilter filter;
	LONGLONG filestartposition,frealtimepos;
	int frealtimeposx,sorttype,status,r_status,audiofilescount,countcheck;

	guiGadget_ListView *soundsgadget,*regionslistview;

	guiGadget_ListBox *infogadget;
	guiGadget *statusgadget,*startsound,*stopsound,*copysound,*editsound,
		*startregion,*stopregion,*copyregion,*searchstring,*g_showregions;

	AudioRealtime *audiorealtime;
	bool endstatus_realtime,peakprogress;

	guiMenu *menu_showinfo,*menu_showregions,*menu_filepath,*menu_showintern,*menu_showmb,*menu_showtime,
		*menu_showrecorded,*menu_shonotfound,*menu_sortsize,*menu_sortname,*menu_sortdate,*menu_showsr;
	AudioRegion *testregion;
};
#endif