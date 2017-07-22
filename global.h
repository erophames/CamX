#ifndef CAMX_GLOBAL
#define CAMX_GLOBAL 1

#include "defines.h"

#ifdef WIN32
#ifndef NOGUI
#include <afxwin.h>
#endif
#endif

#define strcpy string_save
inline void string_save(char *to,char *from){memcpy(to,from,strlen(from)+1);}
#define strncpy stringn_save
inline void stringn_save(char *to,char *from,size_t length){memcpy(to,from,length);}

extern OSTART quantlist[QUANTNUMBER];
extern char *quantstr[QUANTNUMBER];
extern int rafbuffersize[];

// classes 

#ifdef MEMPOOLS
extern class mainPools *mainpools;
#endif

extern class Seq_Main *mainvar; // our main Class/variable
extern class mainMIDIBase *mainMIDI;
extern class mainMIDIRecord *mainMIDIrecord;
extern class DataBase *maindatabase;

extern class mainAudio *mainaudio;
extern class mainWaveMap *mainwavemap;
extern class mainRMGMap *mainrmgmap;

extern class mainAudioRealtime *mainaudioreal;
extern class ThreadControl *mainthreadcontrol;
extern class EditFunctions *mainedit;
extern class EditBuffer *mainbuffer;
extern class Language language;
extern class Settings *mainsettings;
extern class mainProcessor *mainprocessor;

extern class GUI *maingui;

// Threads
extern class MIDIInProc *MIDIinproc;
extern class PluginMIDIInputProc *plugininproc;

extern class MIDIPlaybackThread *mainMIDIalarmthread;
extern class MIDIRTEProc *MIDIrtealarmproc;
extern class MIDIOutProc *MIDIoutproc;

extern class MIDIProcessorProc *MIDIalarmprocessorproc;
extern class MIDIMTCThread *MIDImtcproc;
extern class MIDIStartThread  *MIDIstartthread;
extern class MIDIThruThread *mainMIDIthruthread;
extern class AudioPeakFileThread *audiopeakthread;
//extern class AudioFreezeThread *audiofreezethread;
extern class AudioWorkFileThread *audioworkthread;
extern class MainHelpThread *mainhelpthread;
extern class MainSyncThread *mainsyncthread;
extern class AudioRealtimeThread *mainaudiorealtimethread;
extern class AudioDeviceDeviceOutThread *maindevicerefillthread;
extern class AudioDeviceDeviceInThread *maindeviceinputthread;
extern class AudioRecordMainThread *audiorecordthread;
extern class AudioCoreAndStreamProc *mainaudiostreamproc;
extern class AudioCoreAudioInputProc *mainaudioinproc;

extern class sysTimer *maintimer;

extern double logvolume[];
extern float logvolume_f[];

extern char *numbers_pos[],*numbers_neg[],*MIDIchannels[];

#ifdef WIN32

#ifndef NOGUI
// Colours
extern HBRUSH colour_hBrush[];
extern HPEN colour_hPen[];
extern COLORREF colour_ref[];
#endif

#endif

#endif
