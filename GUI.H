#ifndef CAMX_GUI_H
#define CAMX_GUI_H 1

#include "object.h"
#include "guiheader.h"
#include "guigadgets.h"
#include "guimenu.h"
#include "guigraphics.h"
#include "winzoom.h"
#include "guiwindow.h"
#include "object_group.h"
#include "keycodes.h"
#include "colours.h"
#include "threads.h"

#ifdef WIN32
#include <afxwin.h>
#include <MMSystem.h>

//LRESULT CALLBACK CheckOSScreenMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // Screen
//LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#define MB_OK_STYLE MB_ICONINFORMATION| MB_OK | MB_TASKMODAL| MB_TOPMOST | MB_SETFOREGROUND
#define MB_ERROR_STYLE MB_ICONERROR| MB_OK | MB_TASKMODAL| MB_TOPMOST | MB_SETFOREGROUND
#define MB_YESNO_STYLE MB_ICONQUESTION| MB_YESNO | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND

class Seq_Project;
class Seq_Song;
class Seq_Track;
class Seq_Pattern;
class Seq_Event;
class Seq_SelectionList;
class Edit_BoundEditor;
class WaveMap;
class Seq_Zoom;
class EditorSprite;
class guiframe;
class AudioPattern;
class AudioHDFile;
class AudioRegion;
class InsertAudioEffect;
class Seq_Tempo;
class AudioChannel;
class Seq_Text;
class Seq_Marker;
class RMGObject;
class Processor;
class MIDIPlugin;
class LMIDIEvents;
class AudioIOFX;
class AutomationTrack;
class EditData;
class Edit_Transport;
class Edit_Arrange;

#define QPUFLAG_QUANTIZE 0
#define QPUFLAG_NOTELENGTH 1

#ifdef CAMXGUIHTREADS

#define MAXGUITHREADS 8

class guiWindowThread:public Thread
{
	friend class guiWindowProc;

public:
	guiWindowThread(guiWindowProc *wp){winproc=wp;threadid=0;}

#ifdef WIN32
	static PTHREAD_START_ROUTINE guiWindowThread_Func(LPVOID pParam);
#endif

	int StartThread();
	guiWindowProc *winproc;
	int threadid;	
};

class guiWindowProc:public Thread // MIDI Out Device RingBuffer Thread
{
public:

	guiWindowProc(){
		for(int i=0;i<MAXGUITHREADS;i++)
		{
			Wait_CoreThreadEventHandle[i]=0;
			CoreThreadEventHandle[i]=CoreThreadHandle[i]=0;
		}

		coresinit=0;
	}

	int StartThread();
	void StopThread();

	guiWindow *GetguiWindow();

	inline void WaitCoreSignal(int id)
	{
		WaitForSingleObject(Wait_CoreThreadEventHandle[id], INFINITE);
	}

	inline void SetCoreSignals(int usethreads){

		usedthreads=usethreads>coresinit?coresinit:usethreads;
		for(int i=0;i<usedthreads;i++)SetEvent(Wait_CoreThreadEventHandle[i]); // corewait[i].SetEvent();
	}

	inline void DoneCore(int id)
	{
		SetEvent(CoreThreadEventHandle[id]);
	}

	inline void WaitAllCoresFinished()
	{
		if(usedthreads)
		{
			WaitForMultipleObjects( 
				usedthreads,           // number of objects in array
				CoreThreadEventHandle,     // array of objects
				TRUE,       // wait for any object
				INFINITE);       // five-second wait
		}
	}

	HANDLE Wait_CoreThreadEventHandle[MAXGUITHREADS],
		CoreThreadEventHandle[MAXGUITHREADS],
		CoreThreadHandle[MAXGUITHREADS]; // Refill Audio Device

	OList c_guiwindows; // c_guiWindow

#ifdef WIN32
	static PTHREAD_START_ROUTINE guiWindowFunc(LPVOID pParam);
	CCriticalSection c_guiwindow_sema;
#endif

	int coresinit,usedthreads;
};
#endif

class guiScreen:public Object,public guiForm
{
public:
	guiScreen();
	void ODeInit();

	Edit_Transport *GetTransport();
	Edit_Arrange *GetArrange();

	char *CreateScreenName();
	void OnCreate(); // Header etc..
	void WindowClosed(guiWindow *);
	bool OnNewSize(int w,int h);
	void RefreshMenu();
	void SetTitle(char *t=0);
	void InitNewSong(Seq_Song *,bool calledbytransport=false);
	void SetNewSong(Seq_Song *);
	void SetNewProject(Seq_Project *,Seq_Song *);

	guiWindow *FindWindow(int id);

	Seq_Song *GetSong();

	bool Mixer();
	guiWindow *Editor(int id=-1);

	guiScreen *NextScreen() {return(guiScreen *)next;}
	guiScreen *PrevScreen() {return(guiScreen *)prev;}

	Seq_Project *project;
	Seq_Song *song;
	guiMenu *menu;
	char *screenname;
	int maximumwidth,maximumheight;
	bool open;
};

class guiWindowSetting
{
	friend class guiWindow;
	friend class GUI;

public:
	guiWindowSetting();
	guiWindowSetting(int setid);

	//void Clone(guiWindowSetting *to);
	void Init();

	char name[STANDARDSTRINGLEN];

	void *neweditor;
	guiForm_Child *bindtoform;
	guiScreen *screen;
	OSTART startposition;

#ifdef WIN32
	HWND parent_hwnd;

#endif

	guiWindow *calledfromwindow;

	char *title;
	int formx,formy,startposition_x,startposition_y,startwidth,startheight,type;

	// Bound to other window
	bool simple,noactivate,s_ondesktop,nosizing,standardwindow,maximized,noOSborder;
};

class AutomationObject;

#define REMOVETRACKGUI_LETAUDIOMIXEROPEN 1

class SettingsWindow:public Object{
public:
	CPOINTER old;
	int id,x,y,width,height;
};

class GUIMessage:public Object
{
public:
	void *ptr;
	int parm,intern;
};

class CShowPeak;

class ColourTable_RBG
{
public:
	void GetRGB(int colour,UBYTE *r,UBYTE *g,UBYTE *b);
	void GetRGB(int colour,UBYTE *r,UBYTE *g,UBYTE *b,int add);
	void Init();

	UBYTE rgb_table[3*LASTCOLOUR];

private:
	void InitTable(int colour,UBYTE r,UBYTE g,UBYTE b);
};

enum
{
	MONITOR_HUGE,MONITOR_MID2,MONITOR_MID1,MONITOR_SMALL
};

class GUI
{
public:
	GUI();
	~GUI()
	{
		if(progressstring)
			delete progressstring;
	}

#ifdef DEBUG
	void CheckSongBuffer(Seq_Song *);
#endif

	guiWindow *WindowToFront(Seq_Song *,int editorid);

	int GetSizeCtrlXY(){return 2*GetFontSizeY()+GetFontSizeY()/2;}
	int GetSizeSmallCtrlXY(){return GetFontSizeY()+GetFontSizeY()/4;}

	void CloseSong(guiScreen *,bool remove);

	void MessageLoop();
	bool GetCtrlKey();
	bool GetShiftKey();
	bool GetLeftMouseButton();
	bool GetRightMouseButton();
	void GetMouseOnScreen(int *x,int *y);

	void MouseWheelCheck(int delta,guiWindow *,guiGadget *);

	bool CheckIfInRange(int mx,int my,int rangex,int rangex2,int rangey,int rangey2);

	int GetVersion();

	void EditTrackMIDIInput(guiWindow *,Seq_Track *);
	void EditTrackMIDIOutput(guiWindow *,Seq_Track *);

	void EditTrackInput(guiWindow *,Seq_Track *);
	void EditTrackOutput(guiWindow *,Seq_Track *);
	void EditTrackRecordType(guiWindow *,Seq_Track *);

	void RefreshEffects(AudioEffects *);
	void NewEffectInsert(AudioEffects *,InsertAudioEffect *,guiWindow *);

	void CreateEffectListPopUp(AudioEffects *efx,guiWindow *,char *header);
	void CreateInstrumentPopMenu(AudioEffects *,guiWindow *,InsertAudioEffect *oldeffect,bool addtopopmenu);
	void CreateEffectPopUp(AudioEffects *,guiWindow *,InsertAudioEffect *oldeffect);
	void AddOnOffMenu(guiMenu *,InsertAudioEffect *);

	void DeleteAllWindows();
	void DeleteAllWindowsButActiveProject();
	void DeleteAllWindowsButActiveSong();
	void DeleteAllWindowsButThis();
	void AddCascadeMenu(guiWindow *,guiMenu *);

	bool CheckIfKeyDown(int);

	void LeftMouseButtonUp(guiWindow *);
	void RightMouseButtonUp(guiWindow *);

	void LeftMouseButtonDown(guiWindow *);
	void RightMouseButtonDown(guiWindow *);

	// Init/ Exit CamX Info Window
	void SetInfoWindowText(char *);

	void RefreshTimeSlider(Seq_Song *);
	void ToggleChildDesktop(guiWindow *);
	void RefreshProjectScreens(Seq_Project *);
	guiScreen *OpenNewScreen(Seq_Project *,Seq_Song *);
	void RefreshRealtime();
	void CheckWindowTimer(guiWindow *,bool leftmouse,bool rightmouse);

	bool mouseovereditarea;
	Seq_Song *mouseinheader;
	void InitNewFileSong(Seq_Song *,guiScreen *);

	// MessageBoxed
#ifdef WIN32
	void MessageMMError(char *device,char *from,MMRESULT res);
#endif

	bool MessageBoxYesNo(guiWindow *win=0,char *title="?");
	bool MessageBoxOk(guiWindow *win=0,char *title="?");
	void MessageBoxError(guiWindow *,char *title);

	int AddFontY(int y){return y+fontsizey+1;}

	int GetFontSizeX(){return fontsizex;}
	int GetFontSizeY(){return fontsizey;}
	int GetFontSizeY_Sub(){return fontsizey-1;}

	int GetButtonSizeY(int sy){return sy*GetButtonSizeY()+2*sy*ADDYSPACE;}
	int GetButtonSizeY(){return buttonsizey;}

	void ConvertMIDI2String(char *str,int status,int b1,int b2,LMIDIEvents *lMIDI=0);
	void OpenEventEditors(Seq_Song *); // MIDI Events
	void CloseEventEditors(Seq_Song *,int flag=0);
	void OpenEditEditors(Seq_Song *,int editortype); // Misc (Type)
	void CloseEditEditors(Seq_Song *,int type);
	void ClearRefresh();

	void CloseAllWindows(int type);
	void CloseAllWindowsExecptInfo();

	void CheckTimerMessage(guiWindow *,bool leftmouse,bool rightmouse);

	void CreateScreenMenu(guiScreen *,guiWindow *);
	bool OpenScreen(guiScreen *,bool all);

	void CreateProjectList(guiMenu *);
	void CreateRecentProjectList(guiScreen *,guiMenu *);
	void CreateSongList(guiMenu *,guiScreen *);
	void OpenProjectID(guiScreen *,int id);
	void AddUndoMenu(guiMenu *);

	void AddSetMarkerPositions(Seq_Song *,guiMenu *);

	guiScreen *FindScreen(
#ifdef WIN32
		HWND hWnd
#endif
		);

	void LoadSettings();
	void SaveSettings();

	enum{
		ID_AUTOLOAD_CAMX,
		ID_AUTOLOAD_MIDI
	};

	bool SaveDefaultSong(Seq_Song *,bool MIDI);
	Seq_Song *LoadDefaultSong(guiScreen *,Seq_Song *,bool MIDI);
	bool AskForDefaultSong(int type,bool requester);

	Seq_Song *GetAutoLoadSong(int nr)
	{
		if(nr>1)return 0;
		return autoloadsongs[nr];
	}

	void CloseAllAutoLoadSongs();
	Seq_Song *autoloadsongs[2]; 

	void CloseScreen(guiScreen *);

	guiWindow* FirstWindow() {return (guiWindow *)windows.GetRoot(); }
	guiWindow* LastWindow() {return (guiWindow *)windows.Getc_end(); }
	void AddWindow (guiWindow *w){windows.AddEndO(w);}

	guiWindow *OpenWindow(guiWindow *,guiWindowSetting *,Object *,int flag);
	guiWindow *CloseWindow(guiWindow *,bool mouseclose=false,bool dontactivate=false);

	void RecalcSize(guiWindow *);
	guiWindow *GetEditorWindow(Seq_Song *,int type);
	guiWindow *OpenEditor(int type,Seq_Song *,Seq_Track *,Seq_SelectionList *,guiWindowSetting *,Object *,Object *);
	guiWindow *OpenEditorStart(int type,Seq_Song *,Seq_Track *,Seq_SelectionList *,guiWindowSetting *,Object *,LONGLONG startposition);

	bool CheckOpenEditor(int type,Seq_Song *song=0,Seq_Track *track=0,Seq_SelectionList *list=0);

	void RefreshAudioHDFile(AudioHDFile *,AudioRegion *,bool refreshall=false,int filestorefresh=0);
	void RefreshEditor(guiWindow *);
	void RefreshEventEditors(Seq_Song *,guiWindow *not);

	void AppendPopUpMenu(guiMenu *menu1);
	bool ConvertGUIMenuToOSMenu(guiWindow *,guiMenu *);
	bool ConvertGUIMenuToOSPopMenu(guiMenu *);
	void RefreshScreenMenus();
	void RefreshScreenNames();

	guiWindow *FindWindowInGUI(guiWindow *);
	guiWindow *FindWindow(int type,void *par1,void *par2);

#ifdef WIN32
	int ConvertOSComandToControl(guiWindow *,HWND gadgwin,int par1,int par2);
#endif

	void MenuSelected(guiMenu *,bool popup);

	bool CheckOSMenu(guiMenu *,int id);
	void CheckScreenMenu(guiScreen *,int id);
	void CheckMenu(guiWindow *,int id);

	bool CheckCommand(guiWindow *,int code,int id,
#ifdef WIN32
		HWND child
#endif
		);

	guiMenu *CheckMenuOrPopup(guiWindow *,guiGadget *child,guiMenu *,int nVirtkey,bool shiftkey,bool ctrlkey);

	void CheckKeyDown(guiWindow *,guiGadget *child);
	void CheckKeyUp(guiWindow *);

	guiMenu *CheckHotKey(guiWindow *,guiGadget *child,UBYTE key,bool shiftkey,bool ctrlkey);

	void CheckMouseDown(guiWindow *,int flag);
	guiGadget *CheckMouseUp(guiWindow *,int flag);

	void SendGUIMessage(int par1,void *par2);
	void SendGUIMessage(guiWindow *,int par1,void *par2);
	void CheckUserMessage(guiWindow *,int type,void *par,GUIMessage *);

	void ShowToggleChildTracks(Seq_Song *);

	void RefreshColour(Seq_Pattern *);
	void RefreshColour(Seq_Track *);
	void RefreshColour(AudioChannel *);
	void RefreshColour(Seq_Group *);

	bool RefreshAllEditorsWithPattern(Seq_Song *song,Seq_Pattern *);
	void RefreshAllEditorsWithEvent(Seq_Song *,Seq_Event *);
	void RefreshAllEditorsWithTempo(Seq_Song *,Seq_Tempo *);
	void RefreshAllEditorsWithText(Seq_Song *,Seq_Text *t=0); // t=0, all text
	void RefreshAllEditorsWithMarker(Seq_Song *,Seq_Marker *); //t=0 all marker
	void RefreshAllArrangeWithGroup(Seq_Group *);

	void RefreshRMGObject(RMGObject *);
	void RefreshProjectGUI();
	void RefreshAllUndos();
	void RefreshUndoGUI(Seq_Song *);
	void RefreshTempoGUI(Seq_Song *);
	void RefreshTextGUI(Seq_Song *);
	void RefreshMarkerGUI(Seq_Song *,Seq_Marker *); // or NULL
	void RefreshRegionGUI(AudioHDFile *);
	void RefreshManagerGUI(AudioHDFile *,AudioRegion *);
	void RefreshAllEditors(Seq_Song *,LONGLONG par);
	void RefreshAllEditors(Seq_Song *,int type,LONGLONG par);
	void RefreshAllHeaders(Seq_Song *);

	void RefreshSMPTE(Seq_Project *);
	void RefreshMeasure(Seq_Project *);

	void RefreshSignature(Seq_Song *);

	void RefreshRepairedLoopsGUI(Seq_Song *);
	void RefreshProcessorName(Seq_Track *);

	void RemoveProjectFromGUI(Seq_Project *);
	void RemoveSongFromGUI(guiScreen *,Seq_Song *);
	void RemoveTrackFromGUI(Seq_Track *,int flag=0);
	void RemoveFreezeTrackFromGUI(Seq_Track *);

	void RemovePatternFromGUI(Seq_Song *,Seq_Pattern *,bool redraw=true);
	void RemoveEventFromGUI(Seq_Song *,Seq_Event *);
	void RemoveProcessorFromGUI(Processor *,MIDIPlugin *);

	void StopRegionInGUI(AudioRegion *);
	void RemoveAudioHDFileFromGUI(AudioHDFile *);
	void RemoveAudioRegionFromGUI(AudioHDFile *,AudioRegion *);
	void AddNewSongToGUI(Seq_Song *);

	void RefreshAudioMixer(Seq_Song *,AudioEffects *);
	void RefreshAutoTracks(guiWindow *,Seq_Song *,AutomationTrack *);

	void RefreshMIDICheck(guiWindow *,Seq_Track *);
	void RefreshMIDI(Seq_Track *);

	void RefreshAudioCheck(guiWindow *,Seq_Track *);
	void RefreshAudio(Seq_Track *); // Audio Channels
	void RefreshAudioPattern(AudioPattern *);
	void RefreshAudioChannelName(AudioChannel *,guiWindow *not);
	void RemoveAudioEffectFromGUI(InsertAudioEffect *);
	void RemoveAudioChannelFromGUI(Seq_Song *,AudioChannel *);

	void SongCycleChanged(Seq_Song *);
	void ChangeText(Seq_Text *,char *);
	void ChangeMarker(Seq_Marker *,char *,OSTART start,OSTART end);
	void EditDataValue(EditData *);
	void CreateAutomationTrack(Seq_Track *,AudioChannel *,AutomationTrack *prev);

	void LearnFromMIDIEvent(LMIDIEvents *);
	void LearnFromPluginChange(InsertAudioEffect *,AudioObject *,OSTART time,int index,double value);

	void InitStandardColour(
#ifdef WIN32
		HDC hDC
#endif
		);

	bool InitGUI();
	void CloseGUI();
	void Welcome();

	void RefreshProgress();

	// Tools
	char *ByteToKeyString(Seq_Song *,UBYTE key);
	int KeyStringToByte(Seq_Song *,char *string);
	char *ByteToOctaveString(Seq_Song *song,UBYTE key);

	char *ByteToControlInfo(UBYTE c,BYTE value,bool withnumber=false);
	char *ByteToControlInfo_NOMSBLSB(UBYTE c,BYTE value);

	char *OctaveToString(int);
	char *GetFPSName(char *string,int smpteflag);

	void CreateQuantizePopUp(guiWindow *,QuantizeEffect *,int flag);
	void InitDragDropObject(guiWindow *,Object *);
	void CancelDragDrop();
	bool CheckDragDropLeftMouseUp(guiWindow *,guiGadget *);

	guiWindow *dragdropwindow;
	Object *dragdropobject;
	guiFont minifont,standardfont,standard_bold,smallfont;

	void CloseGUICores();

#ifdef WIN32
	guiWindow *ConvertSystemWindowToGUI(HWND hWnd);	
	HINSTANCE hInst;

	POINT lpPoint;
	HWND mousehWnd;
#endif

	guiWindow *infoWindow; // Init/Exit Info Window
	guiGFX gfx;

	guiScreen *GetActiveScreen() {return activescreen;}
	guiScreen *activescreen;

	guiScreen *FirstScreen(){return (guiScreen *)screens.GetRoot();}
	guiScreen *LastScreen(){return (guiScreen *)screens.Getc_end();}
	void AddScreen(guiScreen *);
	guiScreen *DeleteScreen(guiScreen *);
	void DeleteAllScreens();
	OList screens; // guiScreen

	// GUI Message
	void LockGUIMessages(){gui_message_sema.Lock();}
	void UnlockGUIMessages(){gui_message_sema.Unlock();}

#ifdef WIN32
	CCriticalSection gui_message_sema;
#endif

	void CheckGUIMessages();

	// Mouse Selection
	void ShowMouseSelection_Event(Seq_Event *);

	void OpenPRepairSelection(Seq_SelectionList *);
	void ClosePRepairSelection(Seq_SelectionList *);
	void ResetPRepairSelection(Seq_SelectionList *);

	void OpenPRepairSelection_Pattern(Seq_Song *);
	void ClosePRepairSelection_Pattern(Seq_Song *);

#ifdef CAMXGUIHTREADS
	guiWindowProc winproc;
#endif

	OList guimessages; // GUIMessage
	ColourTable_RBG colourtable;

	void LockProgressString(){gui_pstring_sema.Lock();}
	void UnlockProgressString(){gui_pstring_sema.Unlock();}

#ifdef WIN32
	CCriticalSection gui_pstring_sema;
#endif

	guiWindow *firstfreecorewindow;
	char *progressstring;
	int checkaudiofilescounter,
		systemmonitorwith,systemmonitorheight,
		buttonsizey,buttonsizeaddy,
		borderhorzsize,borderframey,
		borderheight,borderwidth,
		scrollwidth,scrollheight,
		cmdshow,rrt_slowcounter,rrt_MIDIdevicescheck;

	bool allwindowsoondesktop,refreshprogress;

private:
	void ShowRefreshProgress();

	OList windows, // All open windows	
		setwindows; //SettingsWindow

	HANDLE Wait_CoreThreadEventHandle[MAXCORES],
		CoreThreadHandle[MAXCORES],
		CoreThreadEventHandle[MAXCORES];

	int fontsizey,fontsizex,
		monitortype;	
};
#endif