#ifndef CAMX_DEFINES_H
#define CAMX_DEFINES_H 1

#define CAMX 1
#define MAXCORES 64 // CPU CORES
#define MAXPORTS 32
#define WIN32 1

#ifdef WIN32

#ifdef WIN64
#define CAMX_DEFAULTAUTOLOADNAME "\\AutoloadX64\\ALCAMX\\defaultsong.camx"
#define CAMX_DEFAULTAUTOLOADSMFNAME "\\AutoloadX64\\ALSMF\\defaultsong.camx"
#else
#define CAMX_DEFAULTAUTOLOADNAME "\\AutoloadX32\\ALCAMX\\defaultsong.camx"
#define CAMX_DEFAULTAUTOLOADSMFNAME "\\AutoloadX32\\ALSMF\\defaultsong.camx"
#endif

#endif

#define CAMX_SCREENNAME "CamX Frame"
#define CAMX_WINDOWNAME "CamX Window"
#define CAMX_TOOLBARTOP "CamX TBT Window"

#define CAMX_FORMNAME "CamX FWin"
#define CAMX_DIALOGNAME "CamX Dialog Window"

#define PROJECTNAME "project_camx.prox"
#define MAX_CHILDS 4

#ifndef PI
#define PI 3.1415926535
#endif

typedef unsigned short	    UWORD;
typedef unsigned char	    UBYTE;
typedef __int64 OSTART;

#define LOC language.GetString
#define CPOINTER ULONGLONG
#define MAXIMUM_TEMPO 960.0
#define MINIMUM_TEMPO 1.0
#define MINLOOPSIZE TICK16nd
#define CLEARBIT &=~
#define QUANTNUMBER 30

#define NUMBERSTRINGLEN 64
#define MOUSEBUTTONDOWNMS 285 // delay for mouse mode change (in ms)

#define MIDIINPUTBUFFER 128 // number of MIDI Event Buffer per MIDI Input Port

// Menus/Gadgets ID
#define MENU_ID_START 300
#define GADGET_ID_START 1000

#define TRYCATCH 1

// Mouse Status
#define MOUSEKEY_UP 1
#define MOUSEKEY_DOWN 2
#define MOUSEKEY_DBCLICK_LEFT 3
#define MOUSEKEY_DBCLICK_RIGHT 4

// Mouse Clicked Flags
#define MOUSEKEY_LEFT_UP 1
#define MOUSEKEY_LEFT_DOWN 2
#define MOUSEKEY_RIGHT_UP 4
#define MOUSEKEY_RIGHT_DOWN 8

#define STANDARDSTRINGLEN 256
#define SMALLSTRINGLEN 64
#define MAXTEXTMARKERLEN 1024

// Selection Filter
enum{
 SEL_NOTEON=(1<<2),
 SEL_POLYPRESSURE=(1<<3),
 SEL_CONTROLCHANGE=(1<<4),
 SEL_PROGRAMCHANGE=(1<<5),
 SEL_CHANNELPRESSURE=(1<<6),
 SEL_PITCHBEND=(1<<7),
 SEL_SYSEX=(1<<8),
 SEL_INTERN=(1<<9),
 SEL_SELECTED=(1<<10)
};

#define SEL_ALLEVENTS (SEL_NOTEON|SEL_SYSEX|SEL_POLYPRESSURE|SEL_CONTROLCHANGE|SEL_PROGRAMCHANGE|SEL_CHANNELPRESSURE|SEL_PITCHBEND|SEL_PITCHBEND|SEL_INTERN)

#define NOTEON 0x90
#define NOTEOFF 0x80
#define POLYPRESSURE 0xA0
#define CONTROLCHANGE 0xB0
#define PROGRAMCHANGE 0xC0
#define CHANNELPRESSURE 0xD0
#define PITCHBEND 0xE0
#define SYSEX 0xF0

//no midi events
#define AUDIO 0xF1
#define VIDEO 0xF2
#define INTERN 0xF3 // ICD
#define INTERNCHAIN 0xF4 // ICD CHAIN

#define SYSEXEND 0xF7

#define MIDIREALTIME_START 0xFA
#define MIDIREALTIME_STOP 0xFC
#define MIDIREALTIME_CONTINUE 0xFB
#define MIDIREALTIME_CLOCK 0xF8
#define MIDIREALTIME_SONGPOSITION 0xF2
#define MIDIREALTIME_MTCQF 0xF1

#define MAXCHANNELSPERCHANNEL 8 // Max Tracks/Audiofile Mono, Stereo ... 7+1

// TempoEvents
#define TEMPOEVENT_REAL 0
#define TEMPOEVENT_VIRTUAL 1 // tempochanges while playback

#define SAMPLESPERBEAT 192000 // Sample per Beat at 120 BPM ,500 ms, 1 sec=384000
#define PPQRATEINTERN 960
#define INTERNRATEMSMUL (double)(SAMPLESPERBEAT/500.0)
#define INTERNRATEMSDIV (double)(500.0/SAMPLESPERBEAT)

#ifdef ARES64
#define ARES double
#define mMIX 0.5
#else
#define ARES float
#define mMIX 0.5f
#endif

#define MAXPEAK 4

// AUTOMATION OBJECT TYPES
#define AUTOMATIONOBJECTTYPE_CONNECT 1
#define AUTOMATIONOBJECTTYPE_VIRTUAL 2

// Tick Defines
#define	TICK1nd (4*SAMPLESPERBEAT) // 1/1
#define TICK2nd (2*SAMPLESPERBEAT) // 1/2
#define TICK4nd  SAMPLESPERBEAT	// 1/4
#define TICK8nd	(SAMPLESPERBEAT/2) // 1/8
#define TICK16nd (SAMPLESPERBEAT/4) // 1/16
#define TICK32nd (SAMPLESPERBEAT/8) // 1/32
#define TICK64nd (SAMPLESPERBEAT/16) // 1/64

#define MAXUSERTICKS TICK2nd
#define MINXRASTER 8
#define NUMBEROFWINZOOMS 37

// Window Flags
#define WINDOWFLAG_SIZEH 1
#define WINDOWFLAG_SIZEV 2
#define WINDOWFLAG_STATIC 4
#define WINDOWFLAG_NOSIZE 8
#define WINDOWFLAG_CHILD 16
#define WINDOWFLAG_NOBORDER 32

// Intern Messagees
enum{
	MESSAGE_REFRESHAUDIOHDFILE=101,
	MESSAGE_CHECKMOUSEBUTTON,
	MESSAGE_REFRESHSAMPLEEDITOR,
	MESSAGE_DELETESONG,
	MESSAGE_REFRESHPEAKBUFFER,
	MESSAGE_REFRESHFREEZETRACK,
	MESSAGE_AUDIOFILEWORKED 
};

// Note Type
#define NOTETYPE_B 0 // C-2,A,B
#define NOTETYPE_H 1 // C-3,A,H
#define NOTETYPE_SI 2 // Do, Re,mi

// Track types
#define MEDIATYPE_TYPES 2 // MIDI,AUDIO

#define MEDIATYPE_MIDI 1
#define MEDIATYPE_AUDIO 2
#define MEDIATYPE_VIDEO 4
#define MEDIATYPE_AUDIO_RECORD 8
#define MEDIATYPE_NOLOOPS 16
#define MEDIATYPE_NOTFROZEN 32
#define LOCKTASK_SYNC 64

#define MEDIATYPE_ALL (MEDIATYPE_MIDI|MEDIATYPE_AUDIO|MEDIATYPE_VIDEO|MEDIATYPE_AUDIO_RECORD)

#define FILTER_ALLEVENTS 0xFF

#define AUDIOCHANNEL_INPUT 0
#define AUDIOCHANNEL_OUTPUT 1

// Play/Record Mode
#define FILEMODE_CLOSE 0
#define FILEMODE_READ 1
#define FILEMODE_WRITE 2

#define PEAKFILENAME ".cpk"
#define PEAKBUFFERBLOCKSIZE 25

#define DEFAULT_HEADRASTER (4*SAMPLESPERBEAT) // 4/4
#define MIDIINBUFFERLEN 4096
#define SIZENOTES_FLAGEVENTDIFF 1 // each Event own move difference

// Song Event pRepare
#define DELETE_PRepair_NOTEOFFS 1
#define DELETE_PRepair_CONTROL 2
#define DELETE_PRepair_SONGSTART 4
#define DELETE_PRepair_CYCLE 8

extern class guiZoom guizoom[NUMBEROFWINZOOMS];

enum{
	WINDOWDISPLAY_MEASURE,
	WINDOWDISPLAY_SMPTE,
	WINDOWDISPLAY_SECONDS,
	WINDOWDISPLAY_SAMPLES,
	WINDOWDISPLAY_SMPTEOFFSET
};

enum{
	CURSOR_NOTSET,
	CURSOR_STANDARD,
	CURSOR_HAND,
	CURSOR_SIZE4D,

	CURSOR_LEFT,
	CURSOR_RIGHT,
	CURSOR_UP,
	CURSOR_DOWN,
	CURSOR_LEFTRIGHT,
	CURSOR_UPDOWN
};

// Init Plays
#define INITPLAY_MIDITRIGGER 0
#define INITPLAY_AUDIOTRIGGER 1
#define INITPLAY_NEWCYCLE 2
#define INITPLAY_MAX 3 // last+1

extern char *gmkeynames[],*gmproggroups[],*gmprognames[];

// GM Flags
#define GM_OFF 0
#define GM_ON 1

#include "global.h"

#endif
