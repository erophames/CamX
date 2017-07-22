#ifndef CAMX_SAMPLEEDITOR_H
#define CAMX_SAMPLEEDITOR_H 1

#include "editor.h"
#include "audiofile.h"

#define MAXSAMPLEZOOM 1000 // max samples per pixel 1- maxsamplezoom

class Edit_Sample;
class Edit_RegionList;
class Seq_Pattern_VolumeCurve;

class menu_testregion:public guiMenu
{
public:
	menu_testregion(Edit_Sample *ed,int m,AudioRegion *r=0)
	{
		editor=ed;
		mode=m;

		// Skip default ok
		seek=false;
		region=r;
	}
	void MenuFunction();

	Edit_Sample *editor;
	AudioRegion *region;
	int mode;

	// Seek Range
	bool seek;
	LONGLONG seekstart,seekend;
};

class menu_createregion:public guiMenu
{
public:
	menu_createregion(Edit_Sample *ed,Edit_RegionList *erl=0){editor=ed;newregion=0;editor_regionlist=erl;}
	void MenuFunction();
	Edit_Sample *editor;
	Edit_RegionList *editor_regionlist;
	AudioRegion *newregion;
};

class menu_deleteregion:public guiMenu
{
public:
	menu_deleteregion(Edit_Sample *ed,AudioRegion *d){editor=ed;deleteregion=d;}

	void MenuFunction();
	Edit_Sample *editor;
	AudioRegion *deleteregion;
};

class Edit_Sample_StartInit
{
public:
	Edit_Sample_StartInit()
	{
		rangestart=-1;
		rangeend=-1;
		pattern=0;
	}

	AudioHDFile *file;
	AudioPattern *pattern;

	OSTART startposition;
	LONGLONG rangestart,rangeend;
};

class Edit_RegionList:public EventEditor
{
public:
	Edit_RegionList(Edit_Sample *);

	void Init();
	void DeInitWindow();
	void FreeEditorMemory();

	void Gadget(guiGadget *);
	bool GadgetListView(guiGadget_ListView *,int x,int y);
	void ShowRegions();
	void ShowRegionName();

	void RefreshRealtime();
	void RefreshRealtime_Slow();

	guiGadget *g_regionstring;
	guiGadget_ListView *regionslistview;
	Edit_Sample *editor;
	AudioRegion *selectedregion;
	bool dontshowregionname;
};

class Edit_Sample:public EventEditor
{
public:
	Edit_Sample();

	enum{
		PLAYREGION_MODE_STOPPED,
		PLAYREGION_MODE_FULL,
		PLAYREGION_MODE_END,
		PLAYREGION_MODE_START,
		PLAYREGION_MODE_CLIP,
		PLAYREGION_MODE_SAMPLEPOSITION,
		PLAYREGION_MODE_SAMPLEPOSITIONSEEK,
		PLAYREGION_MODE_MOUSEPOSITION,
		PLAYREGION_MODE_STOPATMOUSEPOSITION,
	};

	void NewDataZoom();

	Seq_SelectionList *GetWindowPatternSelection(){return pattern?&patternselection:0;}

	void RefreshObjects(LONGLONG type,bool editcall);
	void AutoScroll();
	void SetWindowName();
	void KeyDown();
	void KeyDownRepeat();

	void MouseClickInSamples(bool leftmouse);
	void MouseDoubleClickInSamples(bool leftmouse);
	void MouseMoveInSamples(bool leftmouse);
	void MouseReleaseInSamples(bool leftmouse);

	void MouseClickInOverview_Ex(bool leftmouse); // Sample Mode
	void ShowOverviewPositions_Ex(); // Sample Mode
	void RefreshTimeSlider_Ex(); // Sample Mode

	//Object *GetDragDrop(HWND wnd,int index);
	//void SelectRegion(AudioRegion *);
	void ChangeHDFile(AudioHDFile *);
	void CutRange();
	void FillRange_Zero();
	void CreateNewFileFromRegion();
	void DeleteRegion(AudioRegion *);
	void PlayTestRegion();
	void DeleteTestRegion();
	void RemoveRealtime(AudioRealtime *ar){if(ar==audiorealtime)audiorealtime=0;}
	void ShowPeakProgress(double per);
	//void ShowRegionName();
	//void ShowRegions(bool showregionstring);
	void ShowTimeMode();
	void NewZoom();
	void RefreshStartPosition();

	void ShowOverview();
	void ShowClipOverview();

	void ShowWave();
	void ShowClipWave(bool autoclip=true);
	void ShowClipWaveOverview(bool autoclip=true);
	void ShowClipNumbers();
	void ShowStatus();

	void ClipRange(LONGLONG s,LONGLONG e);
	void ClearClip();
	void ClearClipOverview();

	void StopPlayback();

	void ShowMenu();
	guiMenu *CreateMenu();

	void SaveFile();
	//	void SaveRegion();

	void UserMessage(int msgid);
	void ResetGadgets();
	void Normalize();
	bool SetStartPosition(LONGLONG start);
	void RefreshHorzSlider();
	void InitGadgets();
	void ShowPatternPositions();

	void Init();
	void InitNewTimeType();

	void DeInitWindow();
	void Gadget(guiGadget *);
	void MouseButton(int flag);
	void MouseWheel(int delta,guiGadget *);
	void RefreshRealtime();
	void RefreshRealtime_Slow();
	void ShowSpecialEditorRange(); //v

	// Overview
	void MouseClickInsideOverview(int mx,int my);

	LONGLONG ConvertXPositionToOverviewSample(int posx);
	int ConvertSamplePositionToOverviewX(LONGLONG pos);

	int ConvertSamplePositionToX(LONGLONG pos);

	Seq_SelectionList patternselection; // For Solo

	Seq_Pattern_VolumeCurve *patternvolumecurve;
	PatternVolumePositions patternvolumepositions;

	guiGadget_CW *samples;
	double samplestartperc;
	double fadeinms,fadeoutms,volume;
	int fadeintype,fadeouttype;
	bool fadeactive,volumeactive,fadeeditmode;

	LONGLONG samplemouseposition, // -1 if not set
		samplestartposition,
		mousetime;

	LONGLONG *overviewostartx;

	int overviewostartsize;
	int playmode,status,samplesperpixel;

	guiGadget *songsamplemode,*scale,*patternname,*filename,*g_volumecurves;
	guiGadget_Time *info_time,*info_end;
	guiGadget_Slider *horz_slider;
	//guiGadget_ListBox *regionsgadget;
	guiGadget_Integer *right_samples,*left_samples;
	guiGadget *horzslider,*horzzoomslider,*left_smpte,*right_smpte,*samplepos;

	AudioRealtime *audiorealtime,*audiorealtime2;
	AudioPattern *pattern; // Song Mode
	AudioHDFile *audiohdfile;
	AudioRegion *testregion;
	LONGLONG oldclipstart,oldclipend,clipstart,clipend,frealtimepos;

	Edit_RegionList *regionlisteditor;

	int clipx,clipx2,clipx_overview,clipx2_overview,
		clipbitmapx,clipbitmapy,clipbitmapx2,clipbitmapy2,
		clipbitmapx_overview,clipbitmapy_overview,clipbitmapx2_overview,clipbitmapy2_overview;

	bool endstatus_realtime,endstatus_realtime2,clipoverview,clipshowoverview,showscale,showvolume,showregions;
private:
	
	LONGLONG selectstart;
	guiGadget *time,*length,*regionplay,*regionplayend,*regionplaystart,*regionplayseek,*start,*g_clipstart,*stop;
};

#endif
