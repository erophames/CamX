#include "languagefiles.h"
#include "shortkeys.h"

#ifdef ENG
char *Cxs[]=
{
	"Create new Project", // NEW_PROJECT
	"Open Project", // OPEN_PROJECT
	"Save Project", //SAVE_PROJECT
	"Close Project", //CLOSE_PROJECT
	"Rename Project", //RENAME_PROJECT
	"Select Project", // SELECT_PROJECT
	"Recent Projects", // RECENT_PROJECT
	"Select Song", //SELECT_PROJECT_SONG
	"", // EDIT_SONG_NAME
	"Create new Song", // CXS_NEW_SONG

	//10
	"Open/Add Song", // CXS_OPENADD_SONG
	"Import Song to Project", // CXS_IMPORT_SONG
	"Please create new Project", //CXS_PLEASE_CREATENEWPROJECT
	"SMF-MIDI or SysEx File", //CXS_MIDI_FILE
	"Audio File (Format:Wave)", // CXS_WAVE_FILE
	"Close Song",//CXS_CLOSE_SONG
	"Save Song",//CXS_SAVE_SONG
	"Save Song as",//CXS_SAVE_SONGAS
	"Export Song as", //CXS_EXPORT_SONGAS
	"Save Song: MIDI SMF Format 1 (Multi Tracks)", // CXS_SAVE_SMF1
	
	//20
	"Save Song: MIDI SMF Format 0 (1 Track)", // CXS_SAVE_SMF0
	"MIDI SMF Format 1 (Multi Tracks)", // CXS_SAVE_EXSMF1
	"MIDI SMF Format 0 (1 Track)", // CXS_SAVE_EXSMF0
	"Exit CamX (Save Projects/Songs)", //CXS_EXITCAMX
	"Edit", //CXS_EDIT
	"Functions", //CXS_FUNCTIONS
	"Send Reset", //CXS_SENDRESET
	"Send Reset/Panic", // CXS_SENDPANIC
	"Select/Create CamX-Project Directory", // CXS_SELECTPROJECTDIR
	"Options",// CXS_OPTIONS

	//30
	"CamX Settings", //CXS_SETTINGS
	"Song Settings", //CXS_SONGSETTINGS
	"Recording/Metronome Settings", //CXS_RECORDSETTINGS
	"Open Auto-Load Song", // CXS_OPENAUTOLOADSONG
	"Save active Song as Auto-Load Song", //CXS_SAVEAUTOLOADSONG
	"Delete Realtime Tempo Changes",//CXS_DELETEREALTIMETEMPOCHANGES
	"Open Autoload Song (for MIDI Files/SMF)", //CXS_AUTOLOADSONG_SMF
	"Save Song as Autoload Song (for MIDI Files/SMF)", //CXS_AUTOSAVESONG_SMF,
	"Trigger/Start Song Playback with first MIDI Input Note", //CXS_TRIGGERSONGPLAYBACK,
	"About CamX", //CXS_ABOUTCAMX

	//40
	"Check for CamX Update", //CXS_CHECKFORUPDATES
	"Register Full Version (Serial Number required)", //CXS_REGISTERFULLVERSION
	"Copy Selected Pattern to Measure", //CXS_COPYSELECTEDPATTERNTOMEASURE
	"Move Selected Pattern to Measure",  //CXS_MOVESELECTEDPATTERNTOMEASURE
	"Delete all Automation objects", // CXS_DELETEALLAUTOOBJECTS
	"Set Song Position", //CXS_SETSPP
	"to Pattern Start", //CXS_SETSPP_PATTERNSTART
	"Set Editor Start to Pattern Start",//CXS_SETEDITOR_PATTERNSTART
	"Set Track as Focus Track",//CXS_SETPACTIVETRACK
	"UnMute Pattern", // CXS_MUTEPOFF
	
	//50
	"Mute Pattern", // CXS_MUTEPON
	"UnSelect Pattern", // CXS_UNSELECTP
	"Select Pattern", //CXS_SELECTP
	"Create Folder", //CXS_CREATEFOLDER
	"Save Pattern", // CXS_SAVEPATTERN
	"Save Pattern as MIDI File", // CXS_SAVEPATTERNASSMF
	"Child of :", // CXS_CHILDOF
	"Select Colour", //CXS_SELECTCOLOUR
	"Use Track Colour", //CXS_SELECTCOLOUR
	"Cut Track", //CXS_CUTTRACK

	//60
	"Copy Track", //CXS_COPYTRACK
	"Paste Track", //CXS_PASTETRACK
	"Delete Track", // CXS_DELETETRACK
	"Create Child Track", //CXS_CREATENEWCHILDTRACK
	"Create Automation Track", //CXS_CREATENEWAUTOTRACK
	"Create Pattern Link", //CXS_CREATEPATTERNLINK
	"Remove Pattern from Link",//CXS_REMOVEPATTERNLINK
	"Remove all Links from selected Pattern",//CXS_REMOVEALLPATTERNLINK
	"Select all linked Pattern", //CXS_SELECTALLPATTERNLINK
	"Linked to Pattern #", //CXS_LINKTOPATTERN

	//70
	"Add Automation Track", //CXS_ADDAUTOTRACK
	"Delete Automation Track", //CXS_DELETEAUTOTRACK
	"Delete Folder", //CXS_DELETEFOLDER
	"Delete Clone Pattern", //CXS_DELETECLONEPATTERN
	"Convert Clone to real Pattern", //CXS_CONVERTCLONETOREAL
	"Convert Loop to real Pattern", //CXS_CONVERTLOOPTOPATTERN
	"Convert Loop to Events and add to Pattern", //CXS_CONVERTLOOPTOPATTERNANDADDEVENTS
	"OBS", // CXS_SELPATTERNCOLOUR
	"Select Colour of all selected Pattern", //CXS_SELPATTERNCOLOURALLP
	"Use Pattern Colour", // CXS_USEPATTERNCOLOUR
	
	//80
	"Cut", //CXS_CUT
	"Copy", // CXS_COPY
	"Delete", //CXS_DELETE
	"Open Event Editor", //CXS_OPENEVENTEDITOR
	"Open Sample Editor", //CXS_OPENAUDIOEDITOR
	"from Start of Audio File", //CXS_PLAYSOP
	"from Mouse Position", //CXS_PLAYMP
	"from Song Position", // CXS_PLAYSP
	"Delete Volume Curve", //CXS_DELETEVOLCU
	"Create Volume Curve", //CXS_CREATEVOLCU

	//90
	"Edit Audio Pattern Volume", //CXS_AUDIOPATVOL
	"Reset Audio Pattern Volume", //CXS_RESETAUDIOPVOL
	"Export/Save Audio Region", //CXS_EXSAVEREGION
	"Export/Save Audio File", //CXS_EXSAVEAFILE
	"Select another Audio File for Pattern", //CXS_EXSELECTAFILE
	"Select File inside Audio Manager", //CXS_SELFILEINSIDEMANAGER
	"Select Audio Manager File (", //CXS_SELFAMFILE
	") for Pattern", //CXS_SELFAMFILE2
	"Select Audio Manager Region (", //CXS_SELFAMREGION
	"",

	//100
	"Create Groove <MIDI Pattern->Groove>", //CXS_CREATEGROOVE
	"Split Pattern to Channel (Create new Tracks)", //CXS_SPLITPTOCHL
	"Split Pattern to Event-Types (Create new Tracks)", //CXS_SPLITPTOET
	"Rotate Pattern (only Notes)", //CXS_ROTATEPATTERN
	"Convert Loops and add to Pattern", //CXS_CONLATOP
	"Mix Events of other selected Pattern in this Pattern", //CXS_MIXOSELPTOP
	"Add Events of other selected Pattern to this Pattern", //CXS_ADDOSELPTOP
	"Create Pattern", //CXS_CREATEPATTERN
	"Create Clone Pattern", //CXS_CREATECLONEPATTERN
	"Create Pattern (+GM Init SysEx)", //CXS_CREATEGMPATTERN

	//110
	"Create Pattern (+GS Init SysEx)", //CXS_CREATEGSPATTERN,
	"Audio File", //CXS_AUDIOFILE
	"Insert (Create Pattern) Audio Manager File (",//CXS_INSERTAAMFILE
	"Insert Audio Manager Region (",//CXS_INSERTAARFILE
	"MIDI (SMF) or SysEx File", //CXS_INSERTSMFFILE
	"Pattern File", // CXS_OPENFILEPATTERN
	"Paste (Pattern)", //CXS_PASTEPATTERN
	"Create Pattern+Paste Events", //CXS_PASTEEPATTERN
	"UnMute all Tracks", //CXS_UNMUTEALLTRACKS
	"Solo Off (", //CXS_SOLOOFF

	//120
	"Goto Solo Track", //CXS_GOTOSOLOTRACK
	"Solo Off+UnMute (all Tracks)", //CXS_SOLOOFFUNMUTETRACKS
	"Delete Marker", //CXS_DELETEMARKER
	"Set Editor Start with [Marker Start", //CXS_EDITORMARKERSTART
	"Set Editor Start with Marker End]", //CXS_EDITORMARKEREND
	"Set Song Position with [Marker Start", //CXS_SPPMARKERSTART
	"Set Song Position with Marker End]", //CXS_SPPMARKEREND
	"(No active Pattern)", //CXS_NOACTIVEPATTERN
	"No Focus Track (Pattern:", //CXS_NOACTIVETRACKP
	"No Focus Track, no Focus Pattern", //CXS_NOACTIVETRACKNOP

	//130
	"File", //CXS_FILE
	"Load Audio File -> Track", //CXS_LOADWAVE,
	"Select all Tracks", //CXS_SELECTALLTRACKS
	"UnSelect all Tracks", //CXS_UNSELECTALLTRACKS,
	"Paste", //CXS_PASTE
	"Create X new Tracks", //CXS_CREATEXNEWTRACKS
	"Create new Pattern", //CXS_CREATENEWPATTERN
	"Select all Pattern", //CXS_SELECTALLPATTERN
	"UnSelect all Pattern", //CXS_UNSELECTALLPATTERN
	"Select all Pattern of Active Track", //CXS_SELECTALLPATTERNAT
	
	"Select all Pattern between Cycle Positions", //CXS_SELECTALLPATTERNCP
	"Select all Track's Pattern between Cycle Positions", //CXS_SELECTALLPATTERNTCP
	"Mute all Tracks", //CXS_MUTEALLTRACKS
	"Mute all selected Tracks", //CXS_MUTEALLSELTRACKS
	"Mute all Tracks (except Focus Track)", //CXS_MUTEALLTRACKSA
	"UnMute all selected Tracks", //CXS_UNMUTEALLTRACKSSELT
	"Mute selected Pattern", //CXS_MUTESELPATTERN
	"UnMute selected Pattern", //CXS_UNMUTESELPATTERN
	"Mute all Pattern", //CXS_MUTEALLPATTERN
	"UnMute all Pattern", //CXS_UNMUTEALLPATTERN

	"Solo off (Tracks)", //CXS_SOLOOFFTRACKS
	"Reset all Solo/Mute (Tracks+Groups)", //CXS_RESETSOLOMUTE
	"Goto", //CXS_GOTO
	"First Track Pattern", //CXS_FIRSTTRACKPATTERN
	"Last Track Pattern", //CXS_LASTTRACKPATTERN
	"First Song Pattern", //CXS_FIRSTSONGPATTERN
	"Last Song Pattern", //CXS_LASTSONGPATTERN
	"Split Track/MIDI Channels", //CXS_SPLITTRACKCHANNELS
	"Insert Recording", //CXS_DELETELASTRECORDING
	"Delete all empty Tracks", //CXS_DELETEALLEMPTYTRACKS

	"Delete all selected Tracks", //CXS_DELETEALLSELTRACKS
	"Sort Tracks by Type", //CXS_SORTTRACKSBYTYPE
	"Mix selected Pattern to Clipboard", //CXS_MIXSELPATTERNCLIP
	"Quantize selected Pattern (Start Position)", //CXS_QUANTSELPATTERN
	"Move selected Pattern to Position ...", //CXS_MOVESELPATTERNM
	"Move selected Pattern to Song Position", //CXS_MOVESELPATTERNSP
	"Copy selected Pattern to Position ...", //CXS_COPYSELPATTERNM
	"Copy selected Pattern to Song Position", //CXS_COPYSELPATTERNSP
	"Move selected Track/s Up", //CXS_MOVETRACKUP
	"Move selected Track/s Down", //CXS_MOVETRACKDOWN

	"Show Pattern List", //CXS_SHOWPATTERNLIST
	"Show Songs Pattern in Pattern List", //CXS_SHOWPATTERNLISTSONG
	"Show Text Map", //CXS_SHOWTEXTMAP
	"Show Marker Map", //CXS_SHOWMARKERMAP
	"Show Notes", //CXS_SHOWNOTES
	"Off", //CXS_OFF
	"as Lines", //CXS_ASLINES
	"as Notes", //CXS_ASNOTES
	"", //CXS_SHOWTRACKNUMBER
	"Show MIDI Control Events", //CXS_SHOWMIDICONTROL,

	"Show all Automation Tracks", //CXS_SHOWALLAUTOTRACKS
	"Hide all Automation Tracks", //CXS_HIDEALLAUTOTRACKS
	"Edit Track Name", //CXS_EDITTRACKNAME
	"Edit Pattern Name", //CXS_EDITPATTERNNAME
	"Select all Double Events",//CXS_SELECTALLDOUBLE
	"Show only selected Events",//CXS_SHOWONLYSELEVENTS
	"Use/Show General MIDI", //CXS_USEGM
	"Show Events Time Line Frame", //CXS_SHOWEVENTSTIMELINE
	"Welcome to CamX Audio MIDI Video Sequencer", //CXS_WELCOME
	"English", // CXS_LANGUAGE

	"Working", // CXS_WORKING
	"Done", // CXS_DONE
	"Welcome to CamX Alpha UI2", // CXS_INITWELCOMEBETA
	"First Event", //CXS_FIRSTEVENT
	"Last Event", //CXS_LASTEVENT
	"Cycle Start", //CXS_CYCLESTART,
	"Cycle End", //CXS_CYCLEEND,
	"First selected Event",	//CXS_FIRSTSELEVENT,
	"Last selected Event", //CXS_LASTSELEVENT,
	"First Event of Focus Track", //CXS_FIRSTEVENTAT

	"Last Event of Focus Track", //CXS_LASTEVENTAT
	"Pattern in Editor", //CXS_PATTERNUSEDINEDITOR
	"Display only selected Pattern", //CXS_DISPLAYONLYAP
	"Copy selected Events to Measure", //CXS_COPYSELEVENTSTOMEASURE,
	"Move selected Events to Measure", //CXS_MOVESELEVENTSTOMEASURE
	"Quantize selected Events", //CXS_QUANTIZEEVENTS
	"Move selected Events to Song Position", //CXS_MOVESELEVENTSTOSP
	"Copy selected Events to Song Position", //CXS_COPYSELEVENTSTOSP
	"Edit ProgramChange Info", //CXS_EDIT_PCINFO
	"Value", //CXS_VALUE

	"First Note", //CXS_FIRSTNOTEEVENT
	"First Program", //CXS_FIRSTPROGRAMEVENT
	"First Pitchbend", //CXS_FIRSTPITCHEVENT
	"First Control Change", //CXS_FIRSTCTRLEVENT
	"First SysEx", //CXS_FIRSTSYSEVENT
	"First ChannelPressure", //CXS_FIRSTCPREVENT
	"First PolyPressure", //CXS_FIRSTPPREVENT
	"Last Note", //CXS_LASTNOTEEVENT
	"Last Program", //CXS_LASTPROGRAMEVENT
	"Last Pitchbend", //CXS_LASTPITCHEVENT

	"Last Control Change", //CXS_LASTCTRLEVENT
	"Last SysEx", //CXS_LASTSYSEVENT
	"Last ChannelPressure", //CXS_LASTCPREVENT
	"Last PolyPressure", //CXS_LASTPPREVENT
	"Select All", //CXS_SELECTALL
	"Select MIDI Channel and Event Type (for new Events)", //CXS_SELECTNEWEVENTTYPE
	"Event Display Filter", //CXS_EVENTDISPLAYFILTER,
	"Stop Song or Set Song Position to Cycle Start or Set Song as active Song", //CXS_STOPSONGORCYL
	"Start Song Playback", //CXS_STARTSONGPLAYBACK
	"Start Song Recording", // CXS_STARTSONGREC

	"Play Focus Track Solo on/off", //CXS_SOLOATONOFF
	"Cycle Playback on/off", //CXS_CYLCEONOFF
	"Metronome", //CXS_METRONOME
	"Song Position Measure", //CXS_SONGPOSITIONMEASURE
	"Song Length (Measures)", //CXS_SONGLENGTHMEASURE
	"Song Tempo (Song Position)", //CXS_SONGTEMPO
	"Synchronization Settings", //CXS_SONGSYNCSETTINGS
	"MIDI Event Input Display", //CXS_MIDIEVENTINPUTDISPLAY
	"MIDI Event Output Display", //CXS_MIDIEVENTOUTPUTDISPLAY
	"Background Work-Thread Progress Display", //CXS_BACKGROUNDPROGRESSDISPLAY

	"No MIDI Output", //CXS_NOMIDIOUTPUT
	"No MIDI Input", //CXS_NOMIDIINPUT
	"Length", //CXS_LENGTH
	"No Song", // CXS_NOSONG
	"No Project/No Song", //CXS_NOPROJECTSONG
	"No Audio System", //CXS_NOAUDIOSYSTEM
	"Receive MIDI Start/Continue Event:Off", //CXS_RECEIVEMIDISTARTOFF
	"Receive MIDI Start/Continue Event:Start Playback", //CXS_RECEIVEMIDISTARTPLAYBACK
	"Receive MIDI Start/Continue Event:Start Recording", //CXS_RECEIVEMIDISTARTRECORD
	"Receive MIDI Start/Continue Event:Start Recording without Pre Counter", //CXS_RECEIVEMIDISTARTRECORDNOPRE

	"Receive MIDI Stop Event", //CXS_RECEIVEMIDISTOP
	"Edit Song Length", //CXS_EDITSONGLENGTH
	"Edit Cycle Start Position", //CXS_EDITCYCLESTART
	"Edit Cycle End Position", //CXS_EDITCYCLEEND
	"Edit Project Name", //CXS_EDITPROJECTNAME
	"Set Cycle Positions with Marker", //CXS_SETCYCLEPOSITIONMARKER
	"Set Song Position with Marker Start Position", //CXS_SETSPWITHMARKERSTART
	"Goto Marker", //CXS_GOTOMARKER
	"Create new [Marker] with Cycle Positions", //CXS_CREATENEWMARKERCP
	"Create new [Marker at Song Position", //CXS_CREATENEWMARKERSP

	"Quantize Mouse Pointer", //CXS_MOUSEQUANTIZE
	"Measure", //CXS_MEASURE,
	"Beat", //CXS_BEAT
	"Free", //CXS_FREE
	"Time Display", //CXS_TIMEDISPLAY
	"Metronome PreCounter", //CXS_METRONOMEPRECOUNTER
	"Precounter", //CXS_PRECOUNTER
	"Number of Precounter", //CXS_NRPRECOUNTER
	"Note ends PreCounter", // CXS_NOTEENDSPRECOUNTER
	"First Note Event ends Precounter and starts Song", //CXS_NOTEENDSPRECOUNTER_I
	
	"Recording waits for first Note", //CXS_RECWAITSFORFIRSTNOTE
	"Wait for first Note Event, then Recording begins",//CXS_RECWAITSFORFIRSTNOTE_I
	"Metronome Click during Recording", //CXS_METROCLICKWHILEREC
	"Punch Recording >IN<", //CXS_PUNCHINRECORDING
	"Punch Recording >IN<, only Events/Audio Recording INSIDE Cycle Positions", //CXS_PUNCHINRECORDING_I
	"Punch Recording <OUT>", //CXS_PUNCHINRECORDING_OUT
	"Punch Recording <OUT>, only Events/Audio Recording OUTSIDE Cycle Positions", //CXS_PUNCHINRECORDING_OUT_I
	"Step Recording", //CXS_STEPRECORDING
	"Step Recording Step Length", //CXS_STEPRECORDINGRES
	"Step Recording Note Length", //CXS_STEPRECORDINGLENGTH

	"Song Position back <<< (Step Length)", //CXS_SPSTEPLEFT,
	"Song Position forward >>> (Step Length)", //CXS_SPSTEPRIGHT,
	"Create new Recording Track at Cycle End (only Cycle Mode)", //CXS_CREATENEWRECORDINGTRACKCYCLE
	"Automatically create a new Recording Track at Cycle End (only Cycle Mode)", //CXS_CREATENEWRECORDINGTRACKCYCLE_I
	"Genereate new CHILD Track when Recording starts", //CXS_CREATENEWRECORDINGCHILDTRACKCYCLE
	"", //CXS_CREATENEWRECORDINGTRACKCYCLE_I
	"Load SMF MIDI File", //CXS_LOADSMFFILE
	"Show active Pattern Parameter on/off", //CXS_TRACKDISPLAYONOFF
	"Name of Focus Pattern", //CXS_NAMEACTPATTERN
	"Transpose Pattern Notes (Playback/Output only)", //CXS_PATTERNTRANSPOSE

	"Velocity of Pattern Notes (Playback/Output only)", //CXS_PATTERNVELOCITY
	"Pattern Quantize Settings", //CXS_PATTERNQTSET
	"Loop endless", //CXS_PATTERNLOOP,
	"Loop Pattern endless (Song Length or next Pattern)", //CXS_PATTERNLOOP_I
	"Loop Pattern on/off", //CXS_PATTERNLOOPONOFF
	"Number of Pattern Loops", //CXS_PATTERNLOOPNR
	"Pattern Loop Positions", //CXS_PATTERNLOOPTYPES
	"Show Focus Track Parameters on/off", //CXS_TRACKDISPLAYONOFF
	"Name of Focus Track", //CXS_TRACKDNAME
	"Media Type of Focus Track [All,MIDI or Audio]", //CXS_TRACKMEDIA

	"Audio Effects of Focus Track", //CXS_TRACKAFX
	"Transpose Track Notes (Playback/Output only)", //CXS_TRACKNOTETRANS
	"Delay (Ticks) of Events (Playback/Output only)", //CXS_TRACKDELAY
	"Velocity of Track Notes (Playback/Output only)", //CXS_TRACKVELOCITY
	"Track Quantize Settings", //CXS_TRACKQTSET
	"Track Event Filter (Playback/Output only)", //CXS_TRACKEVENTFILTER
	"Play Track Group/s Solo", //CXS_PLAYTGSOLO
	"Active Track Group/s-Tracks Recording", //CXS_ACTRACKGROUPREC
	"Loop:Measure", //CXS_LOOPMEASURE
	"Loop:Beat", //CXS_LOOPBEAT

	"Loop:Direct", //CXS_LOOPDIRECT
	"No Processor", //CXS_NOPROCESSOR
	"No MIDI Out", //CXS_NOMIDIOUT
	"MIDI In Disabled", //CXS_NOMIDIIN
	"All Ports", //CXS_USEALLMIDIINDEVICES
	"No Ports", // CXS_NOMIDIINDEV
	"No Audio ", //CXS_NOAUDIO
	"Recording", //CXS_RECALL,
	"Recording: MIDI (Rm)", //CXS_RECONLYMIDI,
	"Recording: Audio", //CXS_RECONLYAUDIO

	"No Group", //CXS_NOGROUP
	"Show no Icon", //CXS_NOICON
	"Use no Processor", //CXS_USENOPROCESSOR
	"No Program", //CXS_NOPROGRAM
	"Editor follows automatically Song Position", //CXS_EDITORFOLLOWSSP
	"Synchronize Editor Start Position with other Song Editors", //CXS_SNYCEDITOR
	"Create new Track", //CXS_CREATENEWTRACK
	"Convert/Add Loops to Pattern", //CXS_F_CONVERTLOPPTOPATTERN
	"Split MIDI Pattern to MIDI Channels", //CXS_F_SPLITMIDIPatternTOCHANNELS
	"Split MIDI Pattern to Event Types",//CXS_F_SPLITMIDIPatternTOTYPES
	
	"Rotate MIDI Pattern (only Notes)", //CXS_F_ROTATENOTES
	"Stretch MIDI Pattern", //CXS_F_STRECHMIDIPattern
	"Clone Pattern", //CXS_F_CLONEPATTERN
	"Copy Pattern", //CXS_F_COPYPATTERN
	"Move Pattern", //CXS_F_MOVEPATTERN
	"Split Pattern", //CXS_F_CUTPATTERN
	"Convert Loop->Pattern Events", //CXS_F_CONVERTLOOPEVENT,
	"Convert Loop->Pattern", //CXS_F_CONVERTPATTERN
	"Convert Clone->Real Pattern", //CXS_F_CONVERTCLONEREALPATTERN
	"Delete Pattern", //CXS_F_DELETEPATTERN

	"Add Pattern to Pattern", //CXS_F_ADDPATTERNTOPATTERN,
	"Mix Pattern to Pattern", //CXS_F_MIXPATTERNTOPATTERN
	"Create Pattern", //CXS_F_CREATEPATTERN
	"Change File of Audio Pattern", //CXS_F_CHANGEPAUDIOFILE
	"Quantize Pattern/s (Start Position)", //CXS_F_QUANTIZEPATTERNSP
	"Quantize Pattern/s (Events)", //CXS_F_QUANTIZEPATTERN
	"Move Track/s", //CXS_MOVETRACKS
	"Create Track", //CXS_CREATETRACK
	"Create Track/s", //CXS_CREATETRACKS
	"Quantize Track", //CXS_QUANTIZETRACK

	"No Empty Tracks found...", //CXS_NOEMPTYTRACKSFOUND
	"Empty Tracks found", //CXS_EMPTYTRACKSFOUND
	"No Undo", //CXS_NOUNDO
	"No Song (No Undo)", //CXS_NOSONGUNDO
	"No Redo", //CXS_NOREDO
	"No Song (No Redo)", //CXS_NOSONGREDO
	"Transpose Pattern Notes", //CXS_PATTERNTRANSPOSE_2
	"Pattern Notes Velocity", //CXS_PATTERNVELOCITY_2
	"Quantize Pattern", //CXS_QPATTERN
	"Transpose Track", //CXS_TRANSTRACK

	"MIDI Event Output Filter", //CXS_MIDIOUTFILTER
	"No Quantize", //CXS_NOQUANTIZE
	"Song: Time Display", //CXS_SONG_TIMEDISPLAY
	"Song: Note Display", //CXS_SONG_NOTEDISPLAY
	"Song: Metronome", //CXS_SONG_METRONOME
	"Song: MIDI Input Event Routing", //CXS_SONG_MIDIINPUTROUTING
	"Unable to create Directory", //CXS_UNABLETOCREATEDIR
	"Please wait until Audio File Operations are finished", //CXS_PLEASEWAITAUDIOOP
	"Load Audio File", //CXS_LOADWAVEFILE
	"Audio File Manager", //CXS_AUDIOFILEMANAGER

	"Delete Automation Parameter/s", //CXS_DELETEAUTOOBJ
	"Create Automation Parameter/s", //CXS_CREATEAUTOOBJECTS
	"Move/Clone Automation Parameter/s", //CXS_MOVEAUTOMATIONOBJ
	"File not found", //CXS_FILENOTFOUND
	"Audio:Audio Input [Ports]", //CXS_SET_AUDIOINCHANNELS
	"> Audio:Audio Output [Ports]", //CXS_SET_AUDIOOUTCHANNELS
	"Audio:VST 1.0-2.4 Plugin Directories", //CXS_SET_VSTDIRS

	"Audio:VST Plugins Info", //CXS_SET_VSTFX,
	"", //CXS_SET_VSTINSTRUMENTS

	"Audio File Directories", //CXS_SET_AFILESDIRS

	"> MIDI:MIDI Output [Ports]", //CXS_SET_MIDIOUT,
	"MIDI:MIDI Input [Ports]", //CXS_SET_MIDIIN
	"Files",	//CXS_SET_FILES
	"Song Synchronization", //CXS_SET_SYNC
	"Keys", //CXS_SET_KEYS
	"Select Song Settings", //CXS_SET_SELECTSONGSETTINGS
	"Select CamX Settings", //CXS_SET_SELECTGLOBALSETTINGS
	"MIDI Channel", //CXS_MIDICHANNEL
	"Add VST Plugin Directory", //CXS_ADDVSTDIRECTORY
	"VST PlugIn Directories", //CXS_VSTPLUGINDIRS

	"Add Directory", //CXS_ADDDIR
	"Checking...Please wait", //CXS_CHECKING
	"Remove selected Directory from VST Plugin Directory List", //CXS_REMOVEVSTDIR_I
	"Select Audio System Hardware", //CXS_SELECTAUDIOSYSTEMHARDWARE
	"List of Audio Channels", //CXS_LISTOFAUDIOCHANNELS_I
	"Select Audio System", //CXS_SELECTAUDIOCAMX_I
	"Audio File Directories", //CXS_AUDIOFILESDIRS_I
	"Add Audio File Directory", //CXS_ADDAÙDIOFILESDIR_I
	"Remove selected Directory from Audio File Directory List", //CXS_REMOVEAUDIODIR_I
	"Display of Notes", //CXS_NOTEDISPLAY

	"Show Notes (Format)", //CXS_SHOWNOTESAS
	"General MIDI Settings", //CXS_GMSETTINGS_I
	"No GM", //CXS_NOGM
	"", //CXS_CAMXRUNNING
	"Error", //CXS_ERROR
	"Master File", //CSX_MASTERFILE
	"Select File/Directory", //CSX_SELECTMASTERFILE_I
	"Select File Sample Format", //CXS_MASTERFILEFORMAT_I
	"Select Region", //CXS_SELECTMASTERREGION
	"Start Mastering/Bounce", //CXS_STARTMASTERING

	"Stop Bouncing", //CXS_STOPMASTERING
	"Normalize", //CXS_NORMALIZE
	"Normalize Master File", //CXS_NORMALIZE_MI
	"Save from first -non Null- Sample", //CXS_SAVEMASTERFILEFS
	"Save Master File from first Sample, skip first Zero Samples", //CXS_SAVEMASTERFILEFS_I
	"Master File Error:Open Save", //CXS_MASTERSAVEERROR
	"Master File Error:Normalize", //CXS_MASTERNORMERROR
	"Mastering finished", //CXS_MASTERINGEND
	"No Mastering done", //CXS_NOMASTERINGDONE
	"From", //CXS_FROM

	"To", //CXS_TO
	"Transport on Desktop", //CXS_TRANSPORTONDESKTOP
	"Transport on Desktop else inside CamX Window", //CXS_TRANSPORTONDESKTOP_I
	"Question", //CXS_QUESTION
	"A CamX project already exists in this directory !", //CXS_ACAMXPROALREADYEXISTS
	"Information", //CXS_INFO
	"Load Auto/Default Song ?",//CXS_LOADAUTOQ
	"Load Auto/Default MIDI File (SMF) Song ?", //CXS_LOADAUTOMIDIQ
	"Overwrite existing Auto-Load Song ?",//CXS_OVERWRITEAUTOLOADQ
	"Create/Clone new Tracks", //CXS_CREATECLONETRACKS

	"Settings", //CXS_GSETTINGS
	"There is no Auto Load Song", //CXS_NOAUTOLOAD
	"Auto Load Song already open", //CXS_AUTOLOADALREADYOPEN
	"of", //CXS_OF
	"Use this version for testing only\n A commercial usage of this version is not allowed!", //CXS_DONTUSETOCREATESONGS
	"Add new Audio Channel and connect to Track", //CXS_ADDNEWCHANNELCT,
	"Add new Instrument Channel and connect to Track", //CXS_ADDNEWAUDIOINSTRUMENTCT,
	"Add new Bus Channel and connect to Track", //CXS_ADDNEWBUSCT
	"Copy this MIDI OUTPUT Settings to other SELECTED Tracks", //CXS_CHANGEALLTRACKSMIDIOUT_SEL
	"Copy this MIDI OUTPUT Settings to ALL other Tracks", //CXS_CHANGEALLTRACKSMIDIOUT

	"No Changes done", //CXS_NOCHANGES
	"Copy this MIDI INPUT Settings to other SELECTED Tracks", //CXS_CHANGEALLTRACKSMIDIIN_SEL
	"Copy this MIDI INPUT Settings to ALL other Tracks", //CXS_CHANGEALLTRACKSMIDIIN
	"Receive MIDI Events from all MIDI Input Devices", //CXS_RECEIVEMIDIINFROMALLDEV
	"(*) MIDI Thru active for this Track even Track isn't Focus Track", //CXS_MIDITHRUACTIVE
	"Use Song MIDI Input Routing", //CXS_SONGROUTING
	"MIDI Output", //CXS_MIDIOUTPUT
	"MIDI Input", //CXS_MIDIOUTPUT
	"Create Audio Channel", //CXS_CREATEAUDIOCHANNEL
	"Create Audio Bus Channel", //CXS_CREATEAUDIOBUS

	"Create Audio Instrument Channel", //CXS_CREATEAUDIOINSTRUMENT
	"Delete Channel", //CXS_DELETECHANNEL
	"Display", //CXS_DISPLAY
	"Active Track", //CXS_ACTIVETRACK
	"No Audio Input Channels found", //CXS_NOAUDIOINPUTCHLS
	"Audio Device Input Channels", //CXS_AUDIOINPUTCHLS
	"Channels", //CXS_CHANNELS
	"No Audio Output Channels found", //CXS_NOAUDIOOUTPUTCHLS
	"Audio Device Output Channels", //CXS_AUDIOOUTPUTCHLS
	"No MIDI Input Devices found", //CXS_NOMIDIINPUTDEVICES

	"Quantize Tempo", //CXS_QUANTIZETEMPO
	"No MIDI Ouput Devices found", //CXS_NOMIDIOutputDeviceS
	"Files", //CXS_FILES
	"No Files found", //CXS_NOFILESFOUND
	"No Audio File Directories", //CXS_NOAUDIOFILESDIRS
	"No VST Plugins found", //CXS_NOVSTPLUGINS
	"No VST Plugin Directories", //CXS_NOVSTDIRS
	"Metronome (Recording)", //CXS_METRORECORDING
	"Metronome (Playback)", //CXS_METROPLAYBACK
	"Metronome Click during Playback", //CXS_METROCLICKWHILEPLAYBACK

	"This CPU supports no SSE2\n Please install CamX SSE1", //CXS_SSEERROR
	"Close Song ?", //CXS_CLOSESONG_Q
	"Close Project ?", //CXS_CLOSEPROJECT_Q
	"Auto Load Song", //CXS_AUTOLOADSONG
	"Error: Song Directory", //CXS_SONGDIRERROR
	"Songs of Project",//CXS_SONGSOFPROJECT
	"Replace with",//CXS_REPLACEWITH
	"Auto-Load Song for imported  SMF (MIDI Format) Songs", //CXS_AUTOLOADFORSMFSONGS
	"Auto-Load Song for new,created Songs", //CXS_AUTOLOADFORSONGS
	"Activate Auto Load Song", //CXS_ACTIVEAUTOLOADSONG

	"A new version is available", //CXS_FOUNDNEWVERSION
	"Installed Version is", //CXS_THSISVERSIONIS
	"Download new Version ?", //CXS_DOWNLOADUPDATE
	"Install new Version", //CXS_UPDATERESTART
	"No Internet Connection", //CXS_NOINTERNET
	"Unable to open Update URL (Update Info)", //CXS_NOURLINFO
	"Unable to open Update URL (Update File)", //CXS_NOURL
	"No Update available (The newest version is installed)", //CXS_UPTODATE
	"Error Reading URL File", //CXS_ERRORREADINGINTERNETFILE
	"Unable to open Write File", //CXS_UNABLETOOPENSAVEFILE

	"Unable to open Read File", //CXS_UNABLETOOPENREADFILE
	"Stop Song for Bouncing ?", //CXS_STOPSONGFORMASTERING
	"No Audio Tracks or Audio Instrument Tracks found", //CXS_NOAUDIOORINSTRUMENTSFOUND
	"All Files", //CXS_ALLFILES
	"Select Audio File", //CXS_SELECTAUDIOFILE
	"To File", //CXS_TOFILE
	"Load Tempo Map", //CXS_LOADTEMPOMAP
	"Save Tempo Map", //CXS_SAVETEMPOMAP
	"Edit Effect", //CXS_EDITEFFECT
	"Move Up", //CXS_MOVEUP

	"Move Down", //CXS_MOVEDOWN
	"Edit Instrument", //CXS_EDITINSTRUMENT
	"Unknown Effect", //CXS_UNKNOWNEFFECT
	"Tracks using this Instrument Channel", //CXS_TRACKSUSINGINSTRUMENTCHANNEL,
	"Tracks using this Channel", //CXS_TRACKSUSINGCHANNEL
	"Pattern is empty", //CXS_PATTERNEMPTY
	"Select", //CXS_SELECT
	"Create", //CXS_CREATE
	"Split", //CXS_CUTSPLIT
	"Use", //CXS_USE

	"Add Audio File", //CXS_ADDAUDIOFILE
	"Add Audio Files from Directory", //CXS_ADDFILESFROMDIR
	"Quantize", //CXS_QUANTIZE
	"unused Audio Files (in Song Directory) ?", //CXS_UNUSEDSOUNDFILESINDIR
	"Update Download finished", //CXS_UPLOADCOMPLETERESTART_Q
	"Exit CamX and install Update?", //CXS_EXITUPDATE_Q
	"Connect with", //CXS_CONNECTWITH
	"No Audio Channels used", //CXS_NOAUDIOCHANNELSUSED
	"Audio Channels used", //CXS_CHANNELSUSED
	"Select all Tempo Events", //CXS_SELECTALLTEMPOS

	"UnSelect all Tempo Events", //CXS_DESELECTALLTEMPOS
	"Edit Tempo Event", //CXS_EDITTEMPO
	"No Groove", // CXS_NOGROOVE
	"No Grooves available", //CXS_NOGROOVESAVAILABLE
	"Load Quantize Settings", //CXS_LOADQUANTIZESETTINGS,
	"Save Quantize Settings", //CXS_SAVEQUANTIZESETTINGS
	"MIDI Event Types to Quantize", //CXS_EVENTQTYPES
	"Reset all Quantize Settings", //CXS_RESETQSETTINGS
	"Select Quantize Grid", //CXS_SELECTQRASTER
	"Groove Quantize", //CXS_GROOVEQUANTIZE

	"Select Groove Grid", //CXS_SELECTGROOVERASTER
	"Quantize Note Off", //CXS_QUANTIZENOTEOFFEND
	"Quantize Note Off Position", //CXS_QUANTIZENOTEOFFPOSITION
	"Fix Note Length", //CXS_FIXNOTELENGTH
	"Select Fix Note Length", //CXS_SELECTFIXNOTELENGTH
	"Capture Quantize", //CXS_CAPTUREQUANTIZE
	"Range of Capture Quantize", //CXS_RANGEOFCAPTURE
	"Strength Quantize", //CXS_STRENGTHQUANTIZE
	"Human Quantize", //CXS_HUMANQUANTIZE
	"Use Human Quantize, Randomize Quantize (+-)", //CXS_USEHUMANQUANTIZE

	"Human Quantize Range <- | +>", //CXS_HUMANQUANTIZERANGE
	"Quantize to", //CXS_QUANTIZETO
	"Disabled", //CXS_DISABLED
	"Mute all Tracks (except selected Tracks)", //CXS_MUTEALLTRACKSASEL
	"Paste Pattern Buffer", //CXS_PASTEPATTERNBUFFER
	"Copy Event/s", //CXS_COPYEVENTS
	"Move Event/s", //CXS_MOVEEVENTS
	"Edit Tempo Event/s", //CXS_EDITTEMPOEVENTS
	"Delete Tempo Event/s", //CXS_DELETETEMPOEVENTS
	"Split Note", //CXS_SPLITNOTE

	"Delete Event/s", //CXS_DELETEEVENTS
	"Quantize Event/s", //CXS_QUANTIZEEVENTS_S
	"Create Event/s", //CXS_CREATEEVENTS
	"Create Tempo Event/s", //CXS_CREATETEMPOEVENTS
	"Edit Event/s", //CXS_EDITEVENTS
	"MIDI File Header not found !", //CXS_NOMIDIFILEHEADER
	"Song already in Project\nCreate a Copy of Song?", //CXS_NOMIDIFILEHEADER
	"Project", //CXS_PROJECT
	"Create a new Song for this new Project", //CXS_CREATENEWPROJECTSONG
	"Project already open", //CXS_PROJECTALREADYIN

	"CAMV Header in File not found!\n Maybe there is new CamX File Format...",//CXS_CAMVHEADERNOTFOUND
	"File saved by newer CamX Version! \nPlease UPDATE CamX to load this File", //CXS_FILENEWER
	"File Checksum Error", //CXS_CHECKSUMERROR
	"2x Stop Editor Start Position Refresh", //CXS_DOUBLESTOP
	"Default Display Format", //CXS_DEFAULTDISPLAYFORMAT
	"Undo Steps per Song", //CXS_UNDOSTEPS,
	"No Undo Steps Limit", //CXS_UNDONOLIMIT,
	"No Audio File Name in Arrange Editor", // CXS_NOAUDIOFILESNAMESAR
	"Show Audio File Name in Arrange Editor", // CXS_AUDIOFILESNAMESAR
	"Show Audio File+Path Name in Arrange Editor", // CXS_AUDIOFILESPATHNAMESAR

	"Undo Memory Usage (Song)", //CXS_UNDOMEMORYUSAGE
	"Default Time Display", //CXS_DEFAULTTIMEDISPLAY
	"Display Audio File Names in Arrange Editor", //CXS_DISPLAYAUDIOFILESINARRANGE
	"Editors follow Song Position", //CXS_ALLEDITORSFOLLOWSONGPOSITION
	"No Device", //CXS_NODEVICE
	"Search for <File not found> Audio Files", //CXS_SEARCHFORFILENOTEFOUND
	"Remove all <File not found> Audio Objects", //CXS_REMOVEALLFILENOTEFOUND#
	"No Audio Files", //CXS_NOAUDIOFILES
	"No Regions", //CXS_NOREGIONS
	"Copy Region", //CXS_COPYREGION

	"Peak Progress", //CXS_PEAKPROGRESS
	"Audio File Error: File Name", //CXS_AUDIOFILEERROR_FILENAME
	"Audio File Error: Unknown File Type", //CXS_AUDIOFILEERROR_FILETYPE
	"Audio File Error: Unable to open File (File deleted ?)", //CXS_AUDIOFILEERROR_FILEUNABLE
	"Select <Search for Audio File> Directory", //CXS_SELECTSEARCHDIR
	"There are no <File not found> Audio Objects", //CXS_NOFILESMISSED
	"Regions", //CXS_SHOWREGIONS
	"Audio File Info", //CXS_SHOWFILEINFO
	"Audio File Path", //CXS_SHOWFILEPATH
	"Audio File Size", //CXS_SHOWFILESIZE

	"Audio File Duration", //CXS_SHOWFILETIME
	"Sort", //CXS_SORT
	"by Name", //CXS_BYNAME,
	"by Size", //CXS_BYSIZE,
	"by File Date", //CXS_BYFILEDATE
	"on Track recorded Audio Files", //CXS_SHOWCAMXRECORDEDFILES
	"not found Audio Files", //CXS_NOTFOUNDAUDIOFILES
	"CPU supports SSE2, but CamX SSE1 is installed! You should install CamX with SSE2" ,//CXS_SSEINITMSG
	"Signature", //CXS_SIGNATURE
	"Create Region", //CXS_CREATEREGION,

	"Delete Region", //CXS_DELETEREGION
	"No Playback", //CXS_NOPLAYBACK
	"Region used by Pattern!\nDelete anyway?", //CXS_REGIONUSEBYPATTERN_Q
	"Objects using Audio Channel\nDelete anyway ?", // CXS_OBJECTSUSINGCHANNEL
	"Delete all Channel Instruments+Effects",//CXS_DELETEALLINSTRUMENTSANDFX
	"Delete all Channel Instruments", //CXS_DELETEALLINSTRUMENTS
	"Delete all Channel Effects", // CXS_DELETEALLFX
	"Error creating CamX Threads !!!", //CXS_INITTHREADERROR
	"Size of Audio File PreBuffer (global)", //CXS_RAFBUFFERSIZE
	"File Buffer", //CXS_RAFBUFFER

	"Latency (Samples)", // CXS_LATENCY
	"No Audio Device", //CXS_NOAUDIODEVICE
	"Latency Audio Device", //CXS_LATENCYAUDIODEVICE
	"No Audio Input", //CXS_NOINPUTHARDWARE
	"ASIO Format Error", //CXS_ASIOERROR
	"ASIO Format not tested", //CXS_ASIONOTTESTED
	"Please info send email: asiobeta@camx.de", //CXS_ASIOEMAIL
	"No Send", //CXS_NOSEND
	"Decode Audio File to .wav Format", //CXS_DECODEFILE
	"Unable to Open Timer Device!", //CXS_TIMERINITERROR

	"Current Audio Engine (CPU) or Audio Read (HD) Usage [Maximum Usage] % + (REC) remaining Audio Recording Time", //CXS_CPUUSAGE
	"Decode Audio File to.wav and insert", //CXS_CONVERTANDINSERT
	"Select encoded Audio File", //CXS_SELECTENCODEDFILE
	"Resampling Audio File to current Samplerate", //CXS_CONVERTSAMPLERATE
	"No File!", //CXS_NOFILE
	"Show only Files with current Samplerate", //CXS_SHOWALLSAMPLERATES
	"Export Region as", //CXS_EXPORTREGIONAS
	"Load Pattern", //CXS_LOADPATTERN
	"Create Marker with Pattern Position", //CXS_CREATEMARKERPATTERN
	"New Marker with 1 Position", //CXS_CREATEMARKERPATTERN_SINGLE

	"New Marker with From...To Positions", //CXS_CREATEMARKERPATTERN_DOUBLE
	"Can't create Song Directory !", //CXS_CANTCREATESONGDIRECTORY
	"Stop Background Audio (Audio File) Processing", //CXS_STOPAUDIOFUNCTIONS
	"Aborted", //CXS_CANCELED
	"Default Song Length (Measure)", //CXS_DEFAULTSONGLENGTH
	"Extern Controller Settings", //CXS_EXTERNCONTROLLER
	"Colour", //CXS_COLOUR
	"Select Group Colour", //CXS_SELECTGROUPCOLOUR
	"Create new Group", //CXS_CREATENEWGROUP
	"Delete Group", //CXS_DELETEGROUP

	"Add Focus Track to Group", //CXS_ADDACTIVETRACKTOGROUP
	"Add selected Tracks to Group", //CXS_ADDSELECTEDTRACKSTOGROUP
	"Remove Focus Track from group", //CXS_REMOVEACTIVETRACKFROMGROUP
	"Remove selected Tracks from group", //CXS_REMOVESELECTEDTRACKSFROMGROUP
	"Change Marker Position with Cycle Position", //CXS_CHANGEMARKERPOSITIONTOCYCLEPOSITION
	"Set Cycle Positions with Pattern Start<->End", //CXS_SETCYCLEWITHPATTERNPOSITIONS
	"Set Cycle Positions with Marker Start<->End", //CXS_SETCYCLEWITHMARKERPOSITIONS
	"Remove", //CXS_REMOVE
	"Add Instrument", //CXS_ADDINSTRUMENT
	"Add Effect", //CXS_ADDEFFECT

	"Unable to create Project Directory !", //CXS_CANTCREATEPROJECTDIRECTORY
	"Create Demo Project+Demo Song ?", //CXS_Q_CREATEDEMOPROJECT
	"Create Song Playback Stop Marker", //CXS_CREATEPLAYBACKSTOPMARKER
	"Move Song Stop Position Marker to this Position", //CXS_CHANGESONGSTOPMARKER
	"Change Length of Notes", //CXS_CHANGELENGTHOFNOTES
	"Convert Drums to Notes", //CXS_CONVERTDRUMSTONOTES
	"Convert Notes to Drums", //CXS_CONVERTNOTESTODRUMS
	"Set Length of Notes", //CXS_SETLENGTHOFNOTES
	"Create Cross Fade", //CXS_CREATECROSSFADE
	"Edit Cross Fade", //CXS_EDITCROSSFADE

	"Length of new Notes", //CXS_NEWNOTELENGTH,
	"Use previous Note Length (if set)", // CXS_USEPREVIOUSNOTELENGTH,
	"MIDI Channel of new Notes", //CXS_CHANNELOFNEWNOTES
	"Select Time Display Measure/SMPTE", //CXS_SELECTTIMEDISPLAY
	"Thru=Use Pattern/Track Channel", //CXS_USEPATTERNTRACKCHANNEL
	"Play Region Start->End", //CXS_PLAYREGIONSE
	"Play Region From End", //CXS_PLAYREGIONFROMEND
	"Play Audio File and Stop at Region Start", //CXS_PLAYANDSTOPATREGION
	"Play Audio File and Jump over Region", //CXS_PLAYANDSJUMPOVERREGION
	"Audio Files", //CXS_AUDIOFILES

	"Information about Audio File", //CXS_INFOAUDIOFILE
	"Region/s affected by Operation. Change/Delete Regions?", //CXS_REGIONAFFECTED_Q
	"Play Audio File (from Cursor)", //CXS_PLAYAUDIOFILECURSOR
	"Stop Audio File Playback", //CXS_STOPAUDIOFILEPLAYBACK
	"Copy Audio File > Clipboard", //CXS_COPYAUDIOFILE_CLIPBOARD
	"Copy Region > Clipboard", //CXS_COPYREGION_CLIPBOARD
	"Stop Region Playback", //CXS_STOPREGIONPLAYBACK
	"Create Audio Region (Cycle <> Positions)", //CXS_CREATEREGIONCYLCE
	"Play and Stop at Mouse Position", //CXS_PLAYSTOPMP
	"Copy this Audio I/O Settings to other SELECTED Tracks", //CXS_CHANGEALLTRACKSAUDIOIO_SEL

	"Copy this Audio I/O Settings to ALL other Tracks", //CXS_CHANGEALLTRACKSAUDIOIO
	"Select Number of Tracks", //CXS_SELECTNRTRACKS
	"Number of Tracks", //CXS_NRTRACKS
	"Create new Tracks (from Songs Focus Track)", //CXS_CREATENEWTRACKSNEXT
	"Create Child Tracks //", //CXS_CREATECHILDS
	"Create new Child Tracks (Child Tracks of Songs Focus Track)", //CXS_CREATENEWCHILDTRACKSNEXT
	"Create new Tracks (Clone Focus Track)", //CXS_CREATETRACKCLONES
	"Cancel", //CXS_CANCEL
	"Recording", //CXS_RECORD
	"Recorded/Intern", //CXS_RECORDED

	"Add Send", //CXS_ADDSEND
	"Select and Info", //CXS_SELECTANDINFO
	"Channel Audio In", //CXS_TRACKUSECHANNELAUDIOIN
	"On", //CXS_ON
	"Close all Windows but active Song Windows", //CXS_CLOSEALLWINDOWSBUTSONG
	"Close all Windows", //CXS_CLOSEALLWINDOWS
	"Add CamX Desktop",//CXS_ADDCAMXDESKTOP
	"Default Device", //CXS_DEFAULTDEVICE
	"Default Device for new MIDI Files etc.", //CXS_DEFAULTDEVICE_INFO
	"Device Settings [Editor]", //CXS_AUDIOHARDWARESETTINGS

	"Audio Device Settings/Editor", //CXS_AUDIOHARDWARESETTINGS_I
	"Use Device", //CXS_USEDEVICE,
	"Use selected Audio Device", //CXS_USEDEVICE_I
	"Double Click on Pattern Settings", //CXS_DOUBLECLICKPATTERN_I
	"Control Events always", //CXS_CONTROLALWAYS
	"Send Control Events even if Track/Pattern is muted etc..", //CXS_CONTROLALWAYS_I
	"Send MIDI Sync Events", //CXS_SENDMIDISYNCEVENTS,
	"Send MIDI Clock, Start/Continue/Stop, Song Position (Playback/Recording)",//CXS_SENDMIDISYNCEVENTS_I,
	"Send MIDI Time Code", //CXS_SENDMTC,
	"Send MIDI Time Code Events (Playback/Recording)", //CXS_SENDMTC_I,

	"Send Metronome Note On/Off Clicks to MIDI Output Device (Playback/Recording)", // CXS_SENDMETRONOMETOMIDIOUT
	"Language", //CXS_LANGUAGESELECTION
	"Automatic Notes/Control Event Logic", //CXS_SENDCYCLENOTES
	"using Bus", //CXS_CHANNELSUSINGBUS
	"Copy (Effect)", //CXS_COPYEFFECT
	"Copy (Instrument)", //CXS_COPYINSTRUMENT
	"Copy Effect Group", //CXS_COPYINSTRUMENTSANDEFFECT
	"File created by newer CamX Version !\nUnable to Save!\n\nPlease UPDATE CamX", //CXS_CANTOVERWRITENEWERFILE
	"Delete Effect Group",//CXS_DELETEALLIE
	"Delete all Instruments", //CXS_DELETEINSTRUMENTS

	"", //EX_CXS_DELETEEFFECTS
	"Mute all Channels", //CXS_MUTEALLCHANNELS
	"UnMute all Channels", //CXS_UNMUTEALLCHANNELS
	"Select from Cycle Positions", //CXS_SELECTCYCLE
	"Select from Marker", //CXS_SELECTMARKER
	"Send MIDI Clock/Sync", //CXS_SENDMTCSPP
	"Quantize intern Song Position to SPP (1/16)", //CXS_QUANTIZESPP
	"Receive MIDI Clock+Song Position+Start/Stop", //CXS_RECEIVEMTCSPP
	"Receive MIDI Time Code", //CXS_RECEIVEMTC
	"Name of Song", //CXS_SONGNAME

	"Enabled", //CXS_ENABLED
	"Load last used Project on startup", //CXS_LOADLASTUSEDPROJECT
	"Show Measure AND SMPTE in Timeline", //CXS_SHOWMEASUREANDSMPTETIMELINE
	"Show Editor Mousepointer-Tooltip", //CXS_SHOWEDITORTOOLTIPS
	"Audio Peak File: Generate+Write File", //CXS_WRITEPEAKFILE
	"Audio Peak File: No Peak File", //CXS_WRITENOPEAKFILE
	"Sample Format of Resamping Audio Files", //CXS_RESAMPLINGFORMAT
	"Input File", //CXS_SAMEASINPUTFILE
	"Project Settings", //CXS_PROJECTSETTINGS
	"Project Name", //CXS_PROJECTNAME

	"Show only Tracks with Audio (Pattern or Effects)", //CXS_SHOWONLYTRACKSWITHAUDIO
	"Unselect all", //CXS_UNSELECTALL
	"Save MIDI Filter Settings", //CXS_SAVEMIDIFILTER
	"Load MIDI Filter Settings", //CXS_LOADMIDIFILTER
	"Check for Update on Startup", //CXS_AUTOUPDATE
	"This CPU supports no SSE/SSE2 !!!", //CXS_SSEERROR
	"Event Output Type: MIDI/Audio Instrument", //CXS_TRACKTYPE
	"Mouse/Edit Mode", // CXS_STATUSMOUSE
	"Scroll Editor with Song Position", // CXS_SCROLLEDITORWITHSONGPOSITION
	"Active Project has Audio Pattern and uses different Samplerate !", //CXS_PROJECTSAMPLEMSG

	"The Project uses a on this Audio System non-existing Sample Rate !", //CXS_PROJECTUSESNONEXISTINGSAMPLERATE
	"Track new", //CXS_NEWTRACK
	"Open Multi Editor/Windows of same Type", // CXS_OPENMULTIEDITOR
	"Show Audio Device I/O Hardware Channels", //CXS_SHOWAUDIODEVICEIO
	"Add Project to Player", //CXS_ADDPROJECT
	"Remove Song", //CXS_DELETESONG,
	"Do you really want to remove Song and Song Data?", //CXS_DELETESONG_Q,
	"Edit Text Event/s", //CXS_EDITTEXTEVENTS
	"Edit Marker Event/s", //CXS_EDITMARKEREVENTS
	"Start - Audio Recording - Threshold", //CXS_CHECKAUDIORECORDPEAK

	"Delete Text/s Events", //CXS_DELETETEXTEVENTS
	"Delete Marker Events", //CXS_DELETEMARKEREVENTS
	"Play MouseOver Notes", //CXS_PLAYPIANOMOUSEOVER
	"Set Cycle Start", //CXS_SETCYCLESTARTHERE,
	"Set Cycle End", //CXS_SETCYCLEENDHERE,
	"Always check for new CamX Version/Update (Internet Access)?", //CXS_AUTOUPDATEQUESTION
	"Measure Display", //CXS_MEASUREDISPLAY
	"Close all Windows but this", //CXS_CLOSEALLWINDOWSBUTTHIS
	"Unused Song Audio Files:Ask/[Delete]", //CXS_UNUSED_1
	"Unused Song Audio Files:Never Delete", //CXS_UNUSED_2

	"Unused Song Audio Files:Auto Delete", //CXS_UNUSED_3
	"Import extern Audio Files To Song Audio Import Directory", //CXS_IMPORTTOAUDIOSONGINPORT
	"Ask if extern Audio Files already exist in Audio Import Directory", //CXS_IMPORTTOSAMEDIRECTORY,
	"Song Audio Import Files", //CXS_SHOWFILEINFOINTERN
	"Move Pattern ---> Use Cycle Range", //CXS_MOVEPATTERNCYCLERANGE_RIGHT,
	"Move Pattern <--- Use Cycle Range", //CXS_MOVEPATTERNCYCLERANGE_LEFT
	"Position left from 1.1.1.1! Move ?", //CXS_QMOVEPATTERN
	"all Pattern", //CXS_ALLPATTERN
	"selected Pattern", //CXS_SELECTEDPATTERN
	"all Pattern of selected Tracks", //CXS_ALLPATTERNSELECTEDTRACKS

	"Change Track Type to Instrument if Instrument exist", //CXS_AUTOINSTRUMENT
	"Export selected Audio Pattern of Focus Track (+CrossFades)", // CXS_EXPORTSELECTEDPATTERN
	"Play [OUT] CrossFade Region without Volume Changes", //CXS_PLAYOUTCROSSFADE_WITHOUTCROSSFADE,
	"Play [OUT] CrossFade Region with Volume Changes", //CXS_PLAYOUTCROSSFADE_WITHCROSSFADE,
	"Play [IN] CrossFade Region without Volume Changes", //CXS_PLAYINCROSSFADE_WITHOUTCROSSFADE,
	"Play [IN] CrossFade Region with Volume Changes", //CXS_PLAYINCROSSFADE_WITHCROSSFADE,
	"Play CrossFade Mix with Volume Changes", //CXS_PLAYCROSSFADEMIX
	"Play CrossFade Mix without Volume Changes", //CXS_PLAYCROSSFADEMIX_WITHOUTVOLUME
	"Change Start/End of Pattern", //CXS_F_SIZEPATTERN
	"Set Loop End with this Pattern", //CXS_SETLOOPEND

	"Set > Start Offset", // CXS_SETOFFSETSTART
	"Set < End Offset", // CXS_SETOFFSETEND
	"> Start Offset Reset", // CXS_CANCELOFFSETSTART
	"< End Offset Reset", // CXS_CANCELOFFSETEND
	"Start+End Offset Reset", // CXS_CANCELOFFSETBOTH
	"Export selected Audio Pattern of Focus Track (+CrossFades/Split Middle of CrossFades)", // CXS_EXPORTSELECTEDPATTERNSPLIT
	"Name of Processor", //CXS_NAMEOFPROCESSOR
	"List of Processors", //CXS_LISTOFPROCESSORS
	"List of Processor Modules", // CXS_PROCESSORMODULES
	"New Processor", //CXS_ADDNEWPROCESSOR

	"Delete Processor", //CXS_DELETEPROCESSOR
	"Edit Module", //CXS_EDITMODULE
	"New Module", //CXS_ADDMODULE
	"Delete Module", //CXS_DELETEMODULE
	"Add to", //CXS_ADDTO
	"Add", //CXS_ADD
	"Modules", //CXS_MODULES
	"Use already existing Audio File?\nNo=create new File", //CXS_FILEEXISTSINIMPORT
	"File already exists\nOverwrite?", //CXS_QOVERWRITEFILE
	"File is used, please choose another File", //CXS_FILEUSED

	"Close all Windows but active Project Windows", //CXS_CLOSEALLWINDOWSBUTPROJECT
	"All selected Tracks", //CXS_ALLSELECTEDTRACKS
	"All Tracks", // CXS_ALLTRACKS
	"MIDI Recording to Tracks first selected MIDI Pattern", //CXS_RECTOFIRSTPATTERN
	"Select all Text Events", //CXS_SELECTALLTEXTEVENTS
	"UnSelect all Text Events", //CXS_UNSELECTALLTEXTEVENTS
	"Select all Marker Events", //CXS_SELECTALLMARKEREVENTS
	"UnSelect all Marker Events", //CXS_UNSELECTALLMARKEREVENTS
	"Stop Update Downloading", //CXS_STOPUPDATEDOWNLOAD
	"Audio Tracks Effects On/Off", //CXS_TRACKMIXER_FX

	"Audio Channels/Instruments/Bus Effects On/Off", //CXS_AUDIOMIXER_FX
	"Error: Copy File", //CXS_COPYFILEERROR 
	"No Samples Recorded", //CXS_NOSAMPLESRECORDED
	"Show Device Name", //CXS_SHOWDEVICENAME
	"Clear", //CXS_CLEAR
	"Open new MIDI Monitor", //CXS_OPENNEWMONITOR
	"All MIDI Out Devices", //CXS_ALLMIDIOutputDeviceS
	"All MIDI In Devices", //CXS_ALLMIDIINPUTDEVICES
	"Show Note Off's in Monitors", //CXS_SHOWNOTEOFFSINMONITOR
	"Connect other selected (Track)Pattern with this Pattern", //CXS_CONNECTSELECTP

	"Hour", //CXS_HOUR
	"Minute", //CXS_MINUTE
	"Second", //CXS_SECOND
	"Directory", //CXS_DIRECTORY
	"open", //CXS_OPEN
	"Close all other Songs", //CXS_CLOSEALLOTHERSONGS
	"Auto Cut Threshold Audio Recording", //CXS_AUTOCUTZEROSAMPLES
	"Error while Recording Audio File/s !!!\nDisk full ?\nKeep incorrect Audio File/s?", //CXS_ERRORRECORDING_Q
	"Save Song Error! Disk full ?", //ERROR_SONGSAVE
	"Save Project Error! Disk full ?", //ERROR_PROJECTSAVE

	"Save Project or Song Error! Disk full ?\nRepeat Saving ?",//ERROR_PROJECTSONGSAVE
	"Set Tracks to MIDI Input/Thru=Always when adding Instrument", //CXS_AUTOTRACKMIDITHRU
	"Copy this MIDI INPUT+OUTPUT Settings to other SELECTED Tracks", //CXS_CHANGEALLTRACKSMIDI_SEL
	"Copy this MIDI INPUT+OUTPUT Settings to ALL other Tracks", //CXS_CHANGEALLTRACKSMIDI
	"No Audio Hardware found...", //CXS_NOAUDIOHARDWAREFOUND
	"with Pattern End", //CXS_SETSPP_PATTERNEND
	"Copy selected Pattern to Mouse Pointer Position", //CXS_COPYSELPATTERNTP
	"Move selected Pattern to Mouse Pointer Position", //CXS_MOVESELPATTERNTP
	"Unable to load Plugin/s !", //CXS_UNABLETOLOADPLUGINS
	"Error Loading Song! Use Song ?", //CXS_ASKLOADERRORSONG

	"Copy selected Pattern to Track", //CXS_COPYSELPATTERNTPNOPOS,
	"Move selected Pattern to Track",//CXS_MOVESELPATTERNTPNOPOS,
	"Ignore corrupt Audio Files", //CXS_IGNORECORRUPTAUDIOFILES
	"Import Song Arrangement from File?", //CXS_IMPORTARRANGEMENT_Q#
	"Save Song Arrangement as File", //CXS_SAVESONGARRANGEMENT
	"Track Input Event Filter (Recording/Thru)", //CXS_TRACKINPUTEVENTFILTER
	"",
	"Audio:VST 3 Plugin Directories", //CXS_SET_VSTDIRS3
	"", 
	"Exit (Don't save Changes)", //CXS_EXITCAMX_DONTSAVE

	"Add other selected Track as Child Tracks", //CXS_ADDOTHERTRACKASCHILD
	"Load Data Dump to Plugin", //CXS_LOADDATADUMPPLUGIN
	"Save Data Dump from Plugin",  //CXS_SAVEDATADUMPFROMPLUGIN
	"Play", //CXS_PLAY
	"to Mouse Pointer Position", //CXS_SETMOUSEP
	"Copy other selected Pattern to Pattern start Position", //CXS_COPYSELPATTERNTPS
	"(< Align >) Move other selected Pattern to Pattern start position", //CXS_MOVESELPATTERNTPS
	"Mastering Mode", //CXS_MASTERTYPES
	"Master: Song Mix", //CXS_MASTERMIX
	"Bounce: all selected Tracks", //CXS_BOUNCESELTRACKS

	"Bounce: all selected Channels", // CXS_BOUNCESELCHANNELS
	"Bounce: all selected Tracks+selected Channels", //CXS_BOUNCESELTRACKCHANNELS
	"Release all selected Child Tracks", //CXS_RELEASECHILDTRACKS
	"Audio Input always ON (CPU Energy -)", //CXS_AUDIOINPUTALWAYSON
	"Audio Input ON only if required (CPU Energy +)", // CXS_AUDIOINPUTONLYNEED
	"Auto-Rename Track with Program Name", //CXS_SETTRACKNAMEASPROGRAMNAME
	"Load", //CXS_LOAD
	"Change Automation Type", // CXS_CHANGESUBTRACK
	"Plugins: No Crash Check, no Realtime Timing Check [CPU Usage 0]", //CXS_NOPLUGINCHECK
	"PlugIns: Crash Check  [CPU Usage 1]", //CXS_PLUGINCHECK1,

	"Plugins: Crash Check+Realtime Timing Check  [CPU Usage 2]", //CXS_PLUGINCHECK2
	"Select Program", //CXS_SELECTPLUGINPROGRAM
	"First (muted) Event", //CXS_FIRSTMUTEEVENT
	"Last (muted) Event", //CXS_LASTMUTEEVENT
	"Mute selected Events", //CXS_MUTESELECTEDEVENTS
	"UnMute selected Events", //CXS_UNMUTESELECTEDEVENTS
	"UnMute all Events", //CXS_UNMUTEALLEVENTS
	"Import File - Import Arrangement Question",// CXS_IMPORTFILEQUESTION
	"Mute Parent Tracks of Record-Child Tracks at Cycle End", //CXS_MUTEPARENTCYLCETRACKS
	"Import Audio File and Split Channels->Tracks", //CXS_SPLITAUDIOCHANNELS

	"Play or Edit", //CXS_PLAYOREDIT
	"Audio Recording Error! Audio Recording stopped", // CXS_AUDIORECORDINGERROR
	"Bufferoverflow - Recording Medium too slow ?", //CXS_AUDIORECORDINGERROR_BUFFEROVERFLOW
	"Default Audio System Samplerate, Samplerate for new Projects", //CXS_AUDIOSYSTEMSAMPLERATE
	"Punch Out+Cycle Recording, Recording is not possible", //CXS_PUNCHCYCLEERROR
	"Precounter only at Song Position 1.1.1.1", //CXS_PRECOUNTERATPOSITIONONE
	"Precounter at any Song Position", //CXS_PRECOUNTERALWAYS
	"Allow Tempo Change Recording", //CXS_ALLOWTEMPORECORDING
	"Create Parent (Folder) Track", //CXS_CREATEPARENTFORSELECTEDTRACKS
	"Send Events to MIDI Device/s", //CXS_SENDTRACKALWAYSTOMIDI

	"Send Events to Audio Plugin/s", //CXS_SENDTRACKALWAYSTOAUDIO
	"Send Events to MIDI Device/s+Audio Plugin/s", //CXS_SENDTRACKALWAYSTOMIDIANDAUDIO
	"Send Events to ... always automatically set", //CXS_SENDTRACKALWAYSTOAUTO
	"Buffer Size (Samples)", //CXS_AUDIOSETDEVICESIZE
	"Split MIDI Files Typ 0 (1 Track) in Tracks - Channels/SysEx", //CXS_AUTOSPLITMIDIFILE
	"Show all Folder",//CXS_SHOWALLFOLDER
	"Hide all Folder", //CXS_HIDEALLFOLDER
	"Show all Event Output to Song Plugins", //CXS_SHOWALLPLUGINDEVICES
	"CPU/HD Usage", // CXS_CPUUSAGE_EDITOR
	"No Song Audio Recording", //CXS_NOSONGAUDIORECORDING

	"Audio Recording Time left", //CXS_AUDIORECORDINGTIME
	"First Track", //CXS_FIRSTTRACK
	"Last Track", //CXS_LASTTRACK
	"First selected Track", //CXS_FIRSTSELECTEDTRACK
	"Last selected Track", //CXS_LASTSELECTEDTRACK
	"Seconds", //CXS_SECONDS
	"Show all Tracks", //CXS_AUTOALL
	"Show selected Tracks", //CXS_AUTOSELECTED
	"Show only Focus Track", //CXS_AUTOFOCUS
	"Show Tracks with Audio/Instruments or MIDI", //CXS_AUTOAUDIOORMIDI
	
	"Show Tracks with Audio/Instrument", //CXS_AUTOAUDIO
	"Show Tracks with MIDI", // CXS_AUTOMIDI
	"No Pattern Check", // CXS_NOPATTERNCHECK
	"only with Pattern from Song Position", //CXS_ONLYPATTERNFROMTO
	"only with Pattern under Song Position", // CXS_ONLYPATTERNATSONGPOSITION
	"Enable Output as Input", //CXS_SETTRACKOUTPUTFREE
	"Audio or MIDI I/O Display", //CXS_VUTYPE
	"No Filter", // CXS_NOFILTER
	"Create a new Desktop for this Song ? \nElse replace existing Song on this Desktop",// CXS_CREATEDESKTOPFORSONG_Q
	"Show MIDI->Instrument Tracks", //CXS_AUTOINSTR

	"No Master File", //CXS_NOMASTERFILE
	"Signature (Song Position)", //CXS_SONGSIGNATURE
	"Mouse Move Direction inside Overview Area", // CXS_OVERVIEWMOUSE
	"Set Start of Editor with Song Position", //CXS_SCROLLEDITORTOSPP
	"Pass MIDI Input Events to MIDI Output and Plugins", //CXS_MIDITHRUINFO
	"Unable to load/create Song", //CXS_UNABLETOLOADSONG
	"Memory/Medium problem", //CXS_MEMORY
	"Songs open:", //CXS_SONGSOPEN
	"Close Project ?", //CXS_QUESTIONCLOSEPROJECTSCREEN
	"Create new Project in this Directory ?", //CXS_CREATEPROJECT_Q

	"Quantize Mouse Pointer/Time Display", //CXS_MOUSEQUANTIZEANDTIME
	"Audio+MIDI I/O Effects", //CXS_AUDIOMIDIFX
	"Audio+MIDI Input Effects", //CXS_AUDIOMIDIINPUTFX
	"Set this Song as active Song", //CXS_ACTIVATESONG
	"ASIO Audio device found!\nShould ASIO be now standard Audio System?ASIO provides the best Audio Quality/Latency", //CXS_ASIOWASFOUND
	"Dock new Windows to this Screen", //CXS_DOCK
	"Advanced Quantize/Groove Settings", //CXS_QUANTIZEEDITOREX
	"Range of graphical Tempo Display", //CXS_RANGETEMPO
	"Timeline - Song (with Tempo-Changes) or Sample Mode", //CXS_SONGORSAMPLES
	"Managment/Settings of Plugin Outputs", //CXS_SEPARATEPLUGINOUTS

	"Plugin On/Off", //CXS_PLUGINONOFF
	"Paste Effect Group", //CXS_PASTINSTRUMENTSANDEFFECT
	"Display dB Scale/Channel", //CXS_DBSCALE
	"MIDI Event Output to MIDI-Device or Plugins", //CXS_MIDIOUTPUT_TYPE
	"Recording : MIDI (Rm) or Audio (R)", //CXS_MIDIAUDIORECORD
	"Set Start of Editor with Cycle Start", //CXS_SCROLLEDITORTOCYCLELEFT
	"Set Start of Editor with Cycle End", //CXS_SCROLLEDITORTOCYCLERIGHT
	"Editor Grid", //CXS_EDITORGRID
	"Default Song Grid (1.1.X.1) ", //CXS_DEFAULTSONGGRID
	"Grid", //CXS_SONGGRID

	"MIDI Files: Dont add GS/GM Pattern", // CXS_MIDIFILESNOSYSEX,
	"MIDI Files: Add GM (if not exists)", // CXS_MIDIFILESADDGM,
	"MIDI Files: Add GS (if not exists)", // CXS_MIDIFILESADDGS,
	"Autosave: Off", //CXS_AUTOSAVEOFF,
	"Autosave: Every 2 minutes", //CXS_AUTOSAVE2MIN,
	"Autosave: Every 5 minutes", //CXS_AUTOSAVE5MIN,
	"Autosave: Every 10 minutes", //CXS_AUTOSAVE10MIN
	"Sampling Rate", //CXS_SAMPLINGRATE
	"<<< Use Buffer Size", //CXS_USEUSERBLOCKSIZE
	"This Device has no Editor", //CXS_AUDIODEVICEHASNOEDITOR

	"No Device of this System can offer the Sampling Rate of an open Project!",
	"The ASIO Driver returns a larger Minimum Buffer Size!\nNevertheless set Buffer Size ?",
	"The ASIO Driver retturns a smaller Maximum Buffer Size zurück!\nnNevertheless set Buffer Size ?",
	"Name", //CXS_NAME
	"Type", //CXS_TYPE
	"Playback only Pattern of this Pattern Selection (temporary)", // CXS_PLAYPATTERNSOLO
	"Activated", //CXS_ACTIVATED
	"Hardware found", //CXS_HARDWAREFOUND
	"New Tracks", //CXS_NEWTRACKS
	"Directory (selected Tracks)", //CXS_MASTERDIRECTORYSELTRACKS

	"Master Directory/File Problem (Open Save)", //CXS_MASTERSAVEERROR_DIRECTORY
	"Load Audio File Directory to Song (per File one new Track)", //CXS_LOADDIRECTORY
	"Load Audio File to Song and Split Audio File in Channels (Mono Track/Channel)", //CXS_LOADAUDIOFILEANDSPLITTOCHANNELS
	"Use Default M. Settings", // CXS_USEDEFAULTMETRO
	"UI+Mouse", //CXS_SET_UI
	"Only vertical Mouse Movement (Fader,Buttons)", //CXS_ONLYVERTICALMOUSE
	"Show only Tracks with Recording activated", // CXS_AUTOTRACKSRECORD
	"Reset Automation Parameter", //CXS_RESETAUTOOBJ
	"No Track Freeze done", //CXS_NOFREEZEDONE
	"Start Audio Freezing (selected Tracks)", //CXS_STARTFREEZING

	"Automatically generate a Automation Track for Plugin Parameter Changes", //CXS_AUTOMATIONPLUGINSCREATE
	"New Track/Channel: Create Audio Volume Automation Track", //CXS_AUTOMATIONCREATEAUDIOVOLUME
	"New Track/Channel: Create Audio Pan Automation Track", //CXS_AUTOMATIONCREATEAUDIOPAN
	"New Track: Create MIDI Pitchbend Automation Track", //CXS_AUTOMATIONCREATEMIDIPITCHBEND,
	"New Track: Create MIDI Volume Automation Track", //CXS_AUTOMATIONCREATEMIDIVOLUME,
	"New Track: Create MIDI Modulation Automation Track", //CXS_AUTOMATIONCREATEMIDIMODULATION,
	"New Track: Create MIDI Note Velocity Automation Track", //CXS_AUTOMATIONCREATEMIDIVELOCITY,
	"Automation : ON", // CXS_AUTOMATIONON,
	"Automation : OFF", //CXS_AUTOMATIONOFF,
	"Learn from incoming MIDI/Plugin Events", //CXS_LEARN

	"Delete Plugin", //CXS_DELETEPLUGIN
	"Automatically generate an Automation Track while editing AND recording/playback", //CXS_PLUGINAUTOMATE
	"Insert Plugin", //CXS_CREATEPLUGIN
	"Replace Plugin", //CXS_REPLACEPLUGIN
	"New Track/Bus: Create Mute Automation Track", //CXS_AUTOMATIONCREATEMUTE
	"New Track/Bus: Create Solo Velocity Automation Track", //CXS_AUTOMATIONCREATESOLO
	"Create Automations Parameter in Recording Mode", //CXS_AUTOMATIONRECORDINGRECORD,
	"Create Automations Parameter in Playback Mode", //CXS_AUTOMATIONRECORDINGPLAYBACK,
	"Move Objects horizontal <->",//CXS_XMOVE
	"Move Objects vertical",//CXS_YMOVE

	"Automation Parameter Playback", //CXS_AUTOMATIONPLAYBACK,
	"Automation Parameter Changes Recording", //CXS_AUTOMATIONRECORDING,
	"! Learn AND create new Automation Track ! ", //CXS_LEARNANDCREATE
	"Bind selected Automation Parameter to Pattern", //CXS_BINDSELECTAUTOMATIONPARAMETER
	" >Use ASIO system< question", //CXS_USEASIOQUESTION
	"Question if <ASIO> should be used, if ASIO is ASIO was found",//CXS_USEASIOQUESTION_I
	"SMPTE Display Offset", //CXS_SMPTEDISPLAYOFFSET
	"Show/Edit Automation Tracks", //CXS_INFOAUTOMATION
	"Show/Edit Volume Curves", //CXS_INFOVOLUME
	"No VST Exceptions (fast but no VST Plugin crash catch)", // CXS_VSTNOCHECKS,

	"VST Exceptions (catch VST Plugin crashes)", // CXS_VSTTRYCATCH,
	"VST Exceptions (catch + check Plugin Time usage)", //CXS_VSTTRYCATCHANDTIMER
	"Cursor Up", //CXS_CURSORUP
	"Cursor Down", //CXS_CURSORDOWN
	"Write Null-/Silence Start Samples (Pause)", //CXS_MASTERPAUSE
	"Check Space-Key also non Focus", //CXS_CHECKSPACEBACKNONFOCUS
	"UnFreeze (selected Tracks)", //CXS_UNFREEZE
	"First Tempo Value", //CXS_FIRSTTEMPOVALUE
	"Last Tempo Value", //CXS_LASTTEMPOVALUE
	"First selected Tempo Value", //CXS_FIRSTSELECTEDTEMPOVALUE

	"Last selected Tempo Value", //CXS_LASTSELECTEDTEMPOVALUE
	"Play selected Range", //CXS_PLAYAUDIOFILECLIP
	"Move Song Position to next Measure (+Shift previous Measure)", //CXS_MOVESONGPOSITIONNEXTMEASURE
	"Set Tempo Values of selected Tempo Events to xxx.000", //CXS_TEMPOEVENTRESET
	"Set Song Position Tempo with Tap Value", //CXS_SETTAPVALUE
	"Tap Value - Click:Reset", //CXS_TAPTEMPO
	"Realtime recorded Tempo Events", //CXS_REALTIMERECTEMPOEVENTS
	"Delete selected Region", //CXS_DELETEREGION_Q
	"One or more Audio Files have a false Sample Rate\\Continue Import?", //CXS_Q_IMPORTSTOP
	"Save Pattern as SysEx File", //CXS_SAVEPATTERNASSYSEX

	"Send this SysEx Pattern ONLY on Song Startup", //CXS_SENDTHISPATTERNONLYSTARTUP
	"Send all - ONLY send on Song Startup - SysEx Pattern", //CXS_SENDALLSYSEX
	"SysEx Startup Pattern has been sent", //CXS_COUNTSYSEXSTARTPATTERN
	"Create X new Bus Channels", //CXS_CREATEXNEWBUS
	"Create Bus Channels", //CXS_CREATEBUS
	"New Bus Chl.", //CXS_NEWBUS
	"Delete Bus Channels", //CXS_DELETEBUS
	"Automatic MIDI/AUDIO selection", //CXS_AUTOMIX
	"Preferred Track Type", //CXS_PREFTRACKTYPE
	"Send MIDI Bank Select Message at Song Start", //CXS_SENDBANKSELECT

	"Send MIDI Program Change Message at Song Start", //CXS_SENDPROGRAMCHANGE
	"Use this MIDI Channel for Bank Select/Program Change, IF [Track MIDI Channel] is set to ALL", //CXS_USEBANKSELECTCHANNEL
	"MIDI File Type 0 (1 Track) - no Track Splitting", //CXS_NOAUTOSPLITMIDIFILE
	"MIDI File Type 0 (1 Track) - Ask Message for Track Splitting", //CXS_NOAUTOSPLITMIDIFILE_ASK
	"New", //CXS_NEW
	"Split MIDI File Type 0 (1 Track) in MIDI Channels ?", //CXS_ASKSPLITMIDIFILE0
	"Set Focus Track to Recording, if no other track is set to recording", //CXS_SETAUTORECORD
	"Select alle same Events with same Position", ////CXS_SELECTALLDOUBLEEVENTSPOSITION
	"No Double Events found...", //CXS_NODOUBLEEVENTSFOUND
	"Audio Pattern (User)Move Mode", //CXS_MOVEBUTTON
};
#endif

#ifdef DEU
char *Cxs[]=
{
	"Neues Project erzeugen", // NEW_PROJECT
	"Project öffnen", // OPEN_PROJECT
	"Project speichern",//SAVE_PROJECT
	"Project schliessen", //CLOSE_PROJECT
	"Project Namen ändern", //RENAME_PROJECT
	"Project auswählen",//SELECT_PROJECT
	"Zuletzt benutze Projects",  // RECENT_PROJECT
	"Song auswählen", //SELECT_PROJECT_SONG
	"", // EDIT_SONG_NAME
	"Neuen Song erzeugen", // CXS_NEW_SONG

	//10
	"Song öffnen/hinzufügen", // CXS_OPENADD_SONG
	"Song ins Project importieren", // CXS_IMPORT_SONG
	"Bitte erzeugen Sie ein neues Project", //CXS_PLEASE_CREATENEWPROJECT
	"SMF MIDI oder SysEx Datei", // CXS_MIDI_FILE
	"Audio Datei (Format:Wave)", // CXS_WAVE_FILE
	"Song schliessen",//CXS_CLOSE_SONG
	"Song speichern", //CXS_SAVE_SONG
	"Song speichern als",//CXS_SAVE_SONGAS
	"Exportiere Song als", // CXS_EXPORT_SONGAS
	"Song speichern: MIDI SMF Format 1 (mehrere Tracks)", // CXS_SAVE_SMF1

	//20
	"Song speichern: MIDI SMF Format 0 (1 Track)", // CXS_SAVE_SMF0
	"MIDI SMF Format 1 (mehrere Tracks)", // CXS_SAVE_EXSMF1
	"MIDI SMF Format 0 (1 Track)", // CXS_SAVE_EXSMF0
	"Verlassen (Projects/Songs werden gespeichert)", //CXS_EXITCAMX
	"Editieren", //CXS_EDIT
	"Funktionen", // CXS_FUNCTIONS
	"Reset senden", //CXS_SENDRESET
	"Panik/Reset senden", //CXS_SENDPANIC
	"CamX Project Verzeichnis auswählen/erzeugen", // CXS_SELECTPROJECTDIR
	"Optionen", //CXS_OPTIONS

	"CamX Einstellungen", //CXS_SETTINGS
	"Song Einstellungen", //CXS_SONGSETTINGS
	"Aufnahme/Metronom Einstellungen", //CXS_RECORDSETTINGS
	"Auto/Default-Laden Song öffnen", // CXS_OPENAUTOLOADSONG
	"Song als Auto/Default-Song speichern", // CXS_SAVEAUTOLOADSONG
	"Lösche Realtime Tempo Änderungen", //CXS_DELETEREALTIMETEMPOCHANGES
	"Auto/Default-Laden Song öffnen (für MIDI Files/SMF)", //CXS_AUTOLOADSONG_SMF
	"Song als Auto/Standard-Laden Song speichern (für MIDI Files/SMF)", //CXS_AUTOSAVESONG_SMF
	"Trigger/Starte Song Wiedergabe mit erster MIDI Note", //CXS_TRIGGERSONGPLAYBACK
	"Über CamX", //CXS_ABOUTCAMX

	"Nach CamX Update suchen", //CXS_CHECKFORUPDATES
	"Registere Vollversion (Serien Nummer notwenig)", //CXS_REGISTERFULLVERSION
	"Kopiere ausgewählten Pattern zum Takt", //CXS_COPYSELECTEDPATTERNTOMEASURE
	"Verschiebe ausgewählten Pattern zum Takt",  //CXS_MOVESELECTEDPATTERNTOMEASURE
	"Lösche alle Automations Objekte", // CXS_DELETEALLAUTOOBJECTS
	"Setze Song Position", //CXS_SETSPP
	"mit Pattern Start", //CXS_SETSPP_PATTERNSTART
	"Setze Editor Start mit Pattern Start",//CXS_SETEDITOR_PATTERNSTART
	"Setze Track als Focus Track",//CXS_SETPACTIVETRACK
	"UnMute Pattern", // CXS_MUTEPOFF
	
	//50
	"Mute Pattern", // CXS_MUTEPON
	"DeSelektiere Pattern", // CXS_UNSELECTP
	"Selektiere Pattern", //CXS_SELECTP
	"Erzeuge Folder", //CXS_CREATEFOLDER
	"Speichere Pattern", // CXS_SAVEPATTERN
	"Speichere Pattern als MIDI File", // CXS_SAVEPATTERNASSMF
	"Child von :", // CXS_CHILDOF
	"Selektiere Farbe", //CXS_SELECTCOLOUR
	"Benutze Farbe", //CXS_USECOLOUR
	"Ausschneiden:Track", //CXS_CUTTRACK

	//60
	"Kopieren:Track", //CXS_COPYTRACK
	"Einfügen:Track", //CXS_PASTETRACK
	"Löschen:Track", // CXS_DELETETRACK
	"Erzeuge Child Track", //CXS_CREATENEWCHILDTRACK
	"Erzeuge Automations Track", //CXS_CREATENEWAUTOTRACK
	"Erzeuge Pattern Link", //CXS_CREATEPATTERNLINK
	"Entferne Pattern aus Link",//CXS_REMOVEPATTERNLINK
	"Entferne alle Links aus selektiertem Pattern",//CXS_REMOVEALLPATTERNLINK
	"Selektiere alle verlinkten Pattern", //CXS_SELECTALLPATTERNLINK
	"Verlinkt mit Pattern #", //CXS_LINKTOPATTERN

	//70
	"Neuer Automation Track", //CXS_ADDAUTOTRACK
	"Lösche Automation Track", //CXS_DELETEAUTOTRACK
	"Lösche Folder", //CXS_DELETEFOLDER
	"Lösche Clone Pattern", //CXS_DELETECLONEPATTERN
	"Konvertiere Clone in echtes Pattern", //CXS_CONVERTCLONETOREAL
	"Konvertiere Loop in echtes Pattern", //CXS_CONVERTLOOPTOPATTERN
	"Konvertiere Loop in Events und an Pattern anfügen", //CXS_CONVERTLOOPTOPATTERNANDADDEVENTS
	"OBS", // CXS_SELPATTERNCOLOUR
	"Auswahl Farbe aller selektierten Pattern", //CXS_SELPATTERNCOLOURALLP
	"Benutze Pattern Farbe", // CXS_USEPATTERNCOLOUR
	
	//80
	"Ausschneiden", //CXS_CUT
	"Kopieren", // CXS_COPY
	"Löschen", //CXS_DELETE
	"Öffnen Event Editor", //CXS_OPENEVENTEDITOR
	"Öffnen Sample Editor", //CXS_OPENAUDIOEDITOR
	"ab Start der Audio Datei", //CXS_PLAYSOP
	"ab Maus Position", //CXS_PLAYMP
	"ab Song Position", // CXS_PLAYSP
	"Löschen Volume Curve", //CXS_DELETEVOLCU
	"Erzeuge Volume Curve", //CXS_CREATEVOLCU

	//90
	"Editiere Audio Pattern Volume", //CXS_AUDIOPATVOL
	"Reset Audio Pattern Volume", //CXS_RESETAUDIOPVOL
	"Exportieren/Speichern Audio Region", //CXS_EXSAVEREGION
	"Exportieren/Speichern Audio Datei", //CXS_EXSAVEAFILE
	"Auswahl einer anderen Audio Datei für das Pattern", //CXS_EXSELECTAFILE
	"Auswahl des Files im Audio Manager", //CXS_SELFILEINSIDEMANAGER
	"Auswahl Audio Manager File (", //CXS_SELFAMFILE
	") für das Pattern", //CXS_SELFAMFILE2
	"Auswahl Audio Manager Region (", //CXS_SELFAMREGION
	"",

	//100
	"Erzeuge Groove <MIDI Pattern->Groove>", //CXS_CREATEGROOVE
	"Spalte Pattern in MIDI Channels (Erzeugung neuer Tracks)", //CXS_SPLITPTOCHL
	"Spalte Pattern in Event-Typen (Erzeugung neuer Tracks)", //CXS_SPLITPTOET
	"Rotiere Pattern (nur Noten)", //CXS_ROTATEPATTERN
	"Konvertiere Loops und füge diese ans Pattern an", //CXS_CONLATOP
	"Mische alle Events anderer selektieren Pattern in das Pattern", //CXS_MIXOSELPTOP
	"Füge alle Events der anderer selektieren Pattern an das Pattern an", //CXS_ADDOSELPTOP
	"Erzeuge Pattern", //CXS_CREATEPATTERN
	"Erzeuge Clone Pattern", //CXS_CREATECLONEPATTERN
	"Erzeuge Pattern (+GM Init SysEx)", //CXS_CREATEGMPATTERN

	//110
	"Erzeuge Pattern (+GS Init SysEx)", //CXS_CREATEGSPATTERN,
	"Audio Datei", //CXS_AUDIOFILE
	"Einfügen (Erzeuge Pattern) Audio Manager Datei (",//CXS_INSERTAAMFILE
	"Einfügen Audio Manager Region (",//CXS_INSERTAARFILE
	"MIDI (SMF) oder SysEx Datei", //CXS_INSERTSMFFILE
	"Pattern Datei", // CXS_OPENFILEPATTERN
	"Einfügen (Pattern)", //CXS_PASTEPATTERN
	"Erzeuge Pattern+Einfügen Events", //CXS_PASTEEPATTERN
	"UnMute alle Tracks", //CXS_UNMUTEALLTRACKS
	"Solo Off (", //CXS_SOLOOFF

	//120
	"Gehe zu Solo Track", //CXS_GOTOSOLOTRACK
	"Solo Off+UnMute (alle Tracks)", //CXS_SOLOOFFUNMUTETRACKS
	"Lösche Marker", //CXS_DELETEMARKER
	"Setze Editor Start mit [Marker Start", //CXS_EDITORMARKERSTART
	"Setze Editor Start mit Marker Ende]", //CXS_EDITORMARKEREND
	"Setze Song Position mit [Marker Start", //CXS_SPPMARKERSTART
	"Setze Song Position mit Marker Ende]", //CXS_SPPMARKEREND
	"(Kein aktives Pattern)", //CXS_NOACTIVEPATTERN
	"Kein aktiver Track (Pattern:", //CXS_NOACTIVETRACKP
	"Kein aktiver Track, kein aktives Pattern", //CXS_NOACTIVETRACKNOP

	//130
	"Datei", //CXS_FILE
	"Lade Audio Datei -> Track", //CXS_LOADWAVE,
	"Selektiere alle Tracks", //CXS_SELECTALLTRACKS
	"DeSelektere alle Tracks", //CXS_UNSELECTALLTRACKS,
	"Einfügen", //CXS_PASTE
	"Erzeuge X neue Tracks", //CXS_CREATEXNEWTRACKS
	"Erzeuge neues Pattern", //CXS_CREATENEWPATTERN
	"Selektiere alle Pattern", //CXS_SELECTALLPATTERN
	"DeSelektiere alle Pattern", //CXS_UNSELECTALLPATTERN
	"Selektiere alle Pattern des Aktiven Tracks", //CXS_SELECTALLPATTERNAT
	
	"Selektiere alle Pattern zwischen Cycle Positionen", //CXS_SELECTALLPATTERNCP
	"Selektiere alle Track Pattern zwischen Cycle Positionen", //CXS_SELECTALLPATTERNTCP
	"Mute alle Tracks", //CXS_MUTEALLTRACKS
	"Mute alle selektierten Tracks", //CXS_MUTEALLSELTRACKS
	"Mute alle Tracks (bis auf Focus Track)", //CXS_MUTEALLTRACKSA
	"UnMute alle selektierten Tracks", //CXS_UNMUTEALLTRACKSSELT
	"Mute selektierte Pattern", //CXS_MUTESELPATTERN
	"UnMute selektierte Pattern", //CXS_UNMUTESELPATTERN
	"Mute alle Pattern", //CXS_MUTEALLPATTERN
	"UnMute alle Pattern", //CXS_UNMUTEALLPATTERN

	"Solo aus (Tracks)", //CXS_SOLOOFFTRACKS
	"Reset alle Solo/Mute (Tracks+Groups)", //CXS_RESETSOLOMUTE
	"Gehe zu", //CXS_GOTO
	"Erstes Track Pattern", //CXS_FIRSTTRACKPATTERN
	"Letztes Track Pattern", //CXS_LASTTRACKPATTERN
	"Erstes Song Pattern", //CXS_FIRSTSONGPATTERN
	"Letztes Song Pattern", //CXS_LASTSONGPATTERN
	"Spalte Track/MIDI Channels", //CXS_SPLITTRACKCHANNELS
	"Einfügen Aufnahme", //CXS_DELETELASTRECORDING
	"Lösche alle leeren Tracks", //CXS_DELETEALLEMPTYTRACKS

	"Lösche alle selektierten Tracks", //CXS_DELETEALLSELTRACKS
	"Sortiere Tracks anhand Typ", //CXS_SORTTRACKSBYTYPE
	"Mische selektierte Pattern ins Clipboard", //CXS_MIXSELPATTERNCLIP
	"Quantisiere selektierte Pattern (Start Position)", //CXS_QUANTSELPATTERN
	"Verschiebe selektierte Pattern an Position ...", //CXS_MOVESELPATTERNM
	"Verschiebe selektierte Pattern an Song Position", //CXS_MOVESELPATTERNSP
	"Kopiere selektierte Pattern an Position ...", //CXS_COPYSELPATTERNM
	"Kopiere selektierte Pattern an Song Position", //CXS_COPYSELPATTERNSP
	"Verschiebe selektierte Track/s Oben", //CXS_MOVETRACKUP
	"Verschiebe selektierte Track/s Unten", //CXS_MOVETRACKDOWN

	"Zeige Pattern Liste", //CXS_SHOWPATTERNLIST
	"Zeige Song Pattern in Pattern Liste", //CXS_SHOWPATTERNLISTSONG
	"Zeige Text Map", //CXS_SHOWTEXTMAP
	"Zeige Marker Map", //CXS_SHOWMARKERMAP
	"Zeige Noten", //CXS_SHOWNOTES
	"Aus", //CXS_OFF
	"als Linie", //CXS_ASLINES
	"als Noten", //CXS_ASNOTES
	"", //CXS_SHOWTRACKNUMBER
	"Zeige MIDI Control Events", //CXS_SHOWMIDICONTROL,

	"Zeige alle Automation Tracks", //CXS_SHOWALLAUTOTRACKS
	"Verstecke alle Automation Tracks", //CXS_HIDEALLAUTOTRACKS
	"Editiere Track Name", //CXS_EDITTRACKNAME
	"Editiere Pattern Name", //CXS_EDITPATTERNNAME
	"Selektiere alle Double Events",//CXS_SELECTALLDOUBLE
	"Zeige nur selektierte Events",//CXS_SHOWONLYSELEVENTS
	"Benutze/Anzeige General MIDI", //CXS_USEGM
	"Zeige Time Line Events", //CXS_SHOWEVENTSTIMELINE
	"Willkommen zum CamX Audio MIDI Video Sequencer", //CXS_WELCOME
	"Deutsch", // CXS_LANGUAGE

	"Arbeite", // CXS_WORKING
	"Fertig", // CXS_DONE
	"Willommen zu CamX Alpha UI2", // CXS_INITWELCOMEBETA
	"Erstes Event", //CXS_FIRSTEVENT
	"Letzte Event", //CXS_LASTEVENT
	"Cycle Start", //CXS_CYCLESTART,
	"Cycle Ende", //CXS_CYCLEEND,
	"Erstes selektiertes Event",	//CXS_FIRSTSELEVENT,
	"Letzte selektiertes Event", //CXS_LASTSELEVENT,
	"Erstes Event des Focus Tracks", //CXS_FIRSTEVENTAT

	"Letztes Event des Focus Tracks", //CXS_LASTEVENTAT
	"Pattern im Editor", //CXS_PATTERNUSEDINEDITOR
	"Zeige nur ausgewähltes Pattern", //CXS_DISPLAYONLYAP
	"Kopiere selektierten Events zum Takt", //CXS_COPYSELEVENTSTOMEASURE,
	"Verschiebe selektierten Events zum Takt", //CXS_MOVESELEVENTSTOMEASURE,
	"Quantisiere selektierten Events", //CXS_QUANTIZEEVENTS
	"Verschiebe selektierten Events zur Song Position", //CXS_MOVESELEVENTSTOSP
	"Kopiere selektierten Events zur Song Position", //CXS_COPYSELEVENTSTOSP
	"Editiere ProgramChange Info", //CXS_EDIT_PCINFO
	"Wert", //CXS_VALUE

	"Erste Note", //CXS_FIRSTNOTEEVENT
	"Erstes Program", //CXS_FIRSTPROGRAMEVENT
	"Erstes Pitchbend", //CXS_FIRSTPITCHEVENT
	"Erstes Control Change", //CXS_FIRSTCTRLEVENT
	"Erstes SysEx", //CXS_FIRSTSYSEVENT
	"Erstes ChannelPressure", //CXS_FIRSTCPREVENT
	"Erstes PolyPressure", //CXS_FIRSTPPREVENT
	"Letzte Note", //CXS_LASTNOTEEVENT
	"Letztes Program", //CXS_LASTPROGRAMEVENT
	"Letztes Pitchbend", //CXS_LASTPITCHEVENT

	"Letztes Control Change", //CXS_LASTCTRLEVENT
	"Letztes SysEx", //CXS_LASTSYSEVENT
	"Letztes ChannelPressure", //CXS_LASTCPREVENT
	"Letztes PolyPressure", //CXS_LASTPPREVENT
	"Alles Selektieren", //CXS_SELECTALL
	"Auswahl MIDI Channel und Event Typ (für neue Events)", //CXS_SELECTNEWEVENTTYPE
	"Event Anzeige Filter", //CXS_EVENTDISPLAYFILTER
	"Song Stop oder Song Position auf Cycle Start setzen oder Song als aktiven Song setzen", //CXS_STOPSONGORCYL
	"Starte Song Wiedergabe", //CXS_STARTSONGPLAYBACK
	"Starte Song Aufnahme", // CXS_STARTSONGREC

	"Abspielen des Focus Tracks Solo an/aus", //CXS_SOLOATONOFF
	"Cycle Wiedergabe an/aus", //CXS_CYLCEONOFF
	"Metronom", //CXS_METRONOME
	"Song Position Takt", //CXS_SONGPOSITIONMEASURE
	"Song Länge (Takte)", //CXS_SONGLENGTHMEASURE
	"Song Tempo (Song Position)", //CXS_SONGTEMPO
	"Synchronisations Einstellungen", //CXS_SONGSYNCSETTINGS
	"MIDI Event Eingangs Anzeige", //CXS_MIDIEVENTINPUTDISPLAY
	"MIDI Event Ausgabe Anzeige", //CXS_MIDIEVENTOUTPUTDISPLAY
	"Hintergrund Arbeits-Thread Fortschritt Anzeige", //CXS_BACKGROUNDPROGRESSDISPLAY

	"Kein MIDI Output", //CXS_NOMIDIOUTPUT
	"Kein MIDI Input", //CXS_NOMIDIINPUT
	"Länge", //CXS_LENGTH
	"Kein Song", // CXS_NOSONG
	"Kein Project/Kein Song", //CXS_NOPROJECTSONG
	"Kein Audio System", //CXS_NOAUDIOSYSTEM
	"Empfang MIDI Start/Continue Event:Aus", //CXS_RECEIVEMIDISTARTOFF
	"Empfang MIDI Start/Continue Event:Starte Wiedergabe", //CXS_RECEIVEMIDISTARTPLAYBACK
	"Empfang MIDI Start/Continue Event:Starte Aufnahme", //CXS_RECEIVEMIDISTARTRECORD
	"Empfang MIDI Start/Continue Event:Starte Aufnahme ohne Vorzähler", //CXS_RECEIVEMIDISTARTRECORDNOPRE
	
	"Empfang MIDI Stop Event", //CXS_RECEIVEMIDISTOP
	"Editiere Song Länge", //CXS_EDITSONGLENGTH
	"Editiere Cycle Start Position", //CXS_EDITCYCLESTART
	"Editiere Cycle Ende Position", //CXS_EDITCYCLEEND
	"Editiere Project Namen", //CXS_EDITPROJECTNAME
	"Setze Cycle Positionen mit Marker", //CXS_SETCYCLEPOSITIONMARKER
	"Setze Song Position mit Marker Start Position", //CXS_SETSPWITHMARKERSTART
	"Gehe zum Marker", //CXS_GOTOMARKER
	"Erzeuge neuen [Marker] mit Cycle Positionen", //CXS_CREATENEWMARKERCP
	"Erzeuge neuen [Marker mit Song Position", //CXS_CREATENEWMARKERSP

	"Mauszeiger Quantisierung", //CXS_MOUSEQUANTIZE
	"Takt", //CXS_MEASURE,
	"Beat (Zählzeit)", //CXS_BEAT
	"Frei", //CXS_FREE
	"Zeit Anzeige", //CXS_TIMEDISPLAY
	"Metronom Vorzähler", //CXS_METRONOMEPRECOUNTER
	"Vorzähler", //CXS_PRECOUNTER
	"Anzahl Vorzähler", //CXS_NRPRECOUNTER
	"Note beendet Vorzähler", // CXS_NOTEENDSPRECOUNTER
	"Das erste Note Event beendet Vorzähler und Song wird gestartet", //CXS_NOTEENDSPRECOUNTER_I

	"Aufnahme wartet auf Note", //CXS_RECWAITSFORFIRSTNOTE
	"Aufnahme beginnt erst mit dem Eintreffen des ersten Note Events",//CXS_RECWAITSFORFIRSTNOTE_I
	"Metronom Klick während der Aufnahme", //CXS_METROCLICKWHILEREC
	"Punch Aufnahme >IN<", //CXS_PUNCHINRECORDING
	"Punch Aufnahme >IN<, Events/Audio wird nur INNERHALB der Cycle Positions aufgenommen", //CXS_PUNCHINRECORDING_I
	"Punch Aufnahme <OUT>", //CXS_PUNCHINRECORDING_OUT
	"Punch Aufnahme <OUT>, Events/Audio wird nur AUSSERHALB der Cycle Positions aufgenommen", //CXS_PUNCHINRECORDING_OUT_I
	"Step Aufnahme", //CXS_STEPRECORDING
	"Step Aufnahme Step Länge", //CXS_STEPRECORDINGRES
	"Step Aufnahme Noten Länge", //CXS_STEPRECORDINGLENGTH

	"Song Position zurück <<< (Step Länge)", //CXS_SPSTEPLEFT,
	"Song Position vorwärts >>> (Step Länge)", //CXS_SPSTEPRIGHT,
	"Erzeuge neuen Aufnahme Track beim Erreichen des Cycle Endes (nur Cycle Modus)", //CXS_CREATENEWRECORDINGTRACKCYCLE
	"Erzeuge automatisch einen neuen Aufnahme Track beim Erreichen des Cycle Endes (nur Cycle Modus)", //CXS_CREATENEWRECORDINGTRACKCYCLE_I
	"Erzeuge einen neuen CHILD Track sobald Aufnahme beginnt", //CXS_CREATENEWRECORDINGCHILDTRACKCYCLE
	"", //CXS_CREATENEWRECORDINGTRACKCYCLE_I
	"Lade SMF MIDI File", //CXS_LOADSMFFILE
	"Zeige Parameter des Focus Pattern an/aus", //CXS_TRACKDISPLAYONOFF
	"Name des Focus Patterns", //CXS_NAMEACTPATTERN
	"Transponiere Pattern Noten (nur Abspielen/Ausgabe)", //CXS_PATTERNTRANSPOSE

	"Velocity der Pattern Noten (nur Abspielen/Ausgabe)", //CXS_PATTERNVELOCITY
	"Pattern Quantisierung Einstellungen", //CXS_PATTERNQTSET
	"Loop endlos", //CXS_PATTERNLOOP,
	"Loop Pattern endlos (Song Länge oder nächstes Pattern)", //CXS_PATTERNLOOP_I
	"Loop Pattern an/aus", //CXS_PATTERNLOOPONOFF
	"Anzahl der Pattern Loops", //CXS_PATTERNLOOPNR
	"Pattern Loop Positionen", //CXS_PATTERNLOOPTYPES
	"Zeige Parameter des Focus Track an/aus", //CXS_TRACKDISPLAYONOFF
	"Name des Focus Tracks", //CXS_TRACKDNAME
	"Media Typ des Focus Tracks [Alles,MIDI oder Audio]", //CXS_TRACKMEDIA

	"Audio Effects des Focus Tracks", //CXS_TRACKAFX
	"Transponiere Track Noten (nur Abspielen/Ausgabe)", //CXS_TRACKNOTETRANS
	"Delay (Ticks) der Events (nur Abspielen/Ausgabe)", //CXS_TRACKDELAY
	"Velocity of Track Notes (nur Abspielen/Ausgabe)", //CXS_TRACKVELOCITY
	"Track Quantisierungs Einstellungen", //CXS_TRACKQTSET
	"Track Event Filter (nur Abspielen/Ausgabe)", //CXS_TRACKEVENTFILTER
	"Abspielen der Track Group/s Solo", //CXS_PLAYTGSOLO
	"Group/s-Tracks des Focus Tracks auf Aufnahme schalten", //CXS_ACTRACKGROUPREC
	"Loop:Takt", //CXS_LOOPMEASURE
	"Loop:Beat", //CXS_LOOPBEAT

	"Loop:Direkt", //CXS_LOOPDIRECT
	"Kein Processor", //CXS_NOPROCESSOR
	"Kein MIDI Out", //CXS_NOMIDIOUT
	"IN Abgeschaltet", //CXS_NOMIDIIN
	"Alle Ports", //CXS_USEALLMIDIINDEVICES
	"Kein Ports", // CXS_NOMIDIINDEV
	"Kein Audio", //CXS_NOAUDIO
	"Aufnahme", //CXS_RECALL,
	"Aufnahme: MIDI (Rm)", //CXS_RECONLYMIDI,
	"Aufnahme: Audio", //CXS_RECONLYAUDIO

	"Keine Group", //CXS_NOGROUP
	"Zeige kein Icon", //CXS_NOICON
	"Benutze keinen Processor", //CXS_USENOPROCESSOR
	"Kein Program", //CXS_NOPROGRAM
	"Editor folgt automatisch Song Position", //CXS_EDITORFOLLOWSSP
	"Synchronisiere Editor Start Position mit anderen Song Editors", //CXS_SNYCEDITOR
	"Erzeuge neuen Track", //CXS_CREATENEWTRACK
	"Konvertiere/Hinzufügen Loops an Pattern", //CXS_F_CONVERTLOPPTOPATTERN
	"Spalte MIDI Pattern in MIDI Channels", //CXS_F_SPLITMIDIPatternTOCHANNELS
	"Spalte MIDI Pattern in Event Typen",//CXS_F_SPLITMIDIPatternTOTYPES

	"Rotiere MIDI Pattern (nur Noten)", //CXS_F_ROTATENOTES
	"Dehne MIDI Pattern", //CXS_F_STRECHMIDIPattern
	"Clone Pattern", //CXS_F_CLONEPATTERN
	"Kopiere Pattern", //CXS_F_COPYPATTERN
	"Verschiebe Pattern", //CXS_F_MOVEPATTERN
	"Zerschneide Pattern", //CXS_F_CUTPATTERN
	"Konvertiere Loop->Pattern Events", //CXS_F_CONVERTLOOPEVENT,
	"Konvertiere Loop->Pattern", //CXS_F_CONVERTPATTERN
	"Konvertiere Clone->Echtes Pattern", //CXS_F_CONVERTCLONEREALPATTERN
	"Lösche Pattern", //CXS_F_DELETEPATTERN

	"Anfügen Pattern an Pattern", //CXS_F_ADDPATTERNTOPATTERN,
	"Mischen Pattern mit Pattern", //CXS_F_MIXPATTERNTOPATTERN
	"Erzeuge Pattern", //CXS_F_CREATEPATTERN
	"Ändere Datei des Audio Pattern", //CXS_F_CHANGEPAUDIOFILE
	"Quantisiere Pattern/s (Start Position)", //CXS_F_QUANTIZEPATTERNSP
	"Quantisiere Pattern/s (Events)", //CXS_F_QUANTIZEPATTERN
	"Verschiebe Track/s", //CXS_MOVETRACKS
	"Erzeuge Track", //CXS_CREATETRACK
	"Erzeuge Track/s", //CXS_CREATETRACKS
	"Quantisiere Track", //CXS_QUANTIZETRACK

	"Keine leere Tracks gefunden...", //CXS_NOEMPTYTRACKSFOUND
	"Leere Tracks gefunden", //CXS_EMPTYTRACKSFOUND
	"Kein Undo", //CXS_NOUNDO
	"Kein Song (Kein Undo)", //CXS_NOSONGUNDO
	"Kein Redo", //CXS_NOREDO
	"Kein Song (Kein Redo)", //CXS_NOSONGREDO
	"Transponiere Pattern Noten", //CXS_PATTERNTRANSPOSE_2
	"Pattern Noten Velocity", //CXS_PATTERNVELOCITY_2
	"Quantisiere Pattern", //CXS_QPATTERN
	"Transpose Track", //CXS_TRANSTRACK

	"MIDI Event Output Filter", //CXS_MIDIOUTFILTER
	"Keine Quantisierung", //CXS_NOQUANTIZE
	"Song: Zeit Anzeige", //CXS_SONG_TIMEDISPLAY
	"Song: Noten Anzeige", //CXS_SONG_NOTEDISPLAY
	"Song: Metronom", //CXS_SONG_METRONOME
	"Song: MIDI Event Input Routing", //CXS_SONG_MIDIINPUTROUTING
	"Fehler beim Erzeugen eines Verzeichnises", //CXS_UNABLETOCREATEDIR
	"Bitte warten bis Audio Operation beendet ist", //CXS_PLEASEWAITAUDIOOP
	"Audio Datei laden", //CXS_LOADWAVEFILE
	"Audio Datei Manager", //CXS_AUDIOFILEMANAGER

	"Löschen Automations Parameter", //CXS_DELETEAUTOOBJ
	"Erzeuge Automations Parameter", //CXS_CREATEAUTOOBJECTS
	"Verschiebe/Clone Automations Parameter", //CXS_MOVEAUTOMATIONOBJ
	"Datei nicht gefunden", //CXS_FILENOTFOUND
	"Audio:Audio Eingang [Ports]", //CXS_SET_AUDIOINCHANNELS
	"> Audio:Audio Ausgang [Ports]", //CXS_SET_AUDIOOUTCHANNELS
	"Audio:VST 1.0-2.4 Plugin Verzeichnisse", //CXS_SET_VSTDIRS
	"Audio:VST Plugin Info", //CXS_SET_VSTFX,
	"", //CXS_SET_VSTINSTRUMENTS
	"Audio Datei Verzeichnisse", //CXS_SET_AFILESDIRS

	"> MIDI:MIDI Ausgang [Ports]", //CXS_SET_MIDIOUT,
	"MIDI:MIDI Eingang [Ports]", //CXS_SET_MIDIIN
	"Dateien",	//CXS_SET_FILES
	"Song Synchronisation", //CXS_SET_SYNC
	"Tasten", //CXS_SET_KEYS
	"Auswahl Song Einstellungen", //CXS_SET_SELECTSONGSETTINGS
	"Auswahl CamX Einstellungen", //CXS_SET_SELECTGLOBALSETTINGS
	"MIDI Channel", //CXS_MIDICHANNEL
	"VST Plugin Verzeichnis hinzufügen", //CXS_ADDVSTDIRECTORY
	"VST PlugIn Verzeichnisse", //CXS_VSTPLUGINDIRS

	"Verzeichnis hinzufügen", //CXS_ADDDIR
	"Überprüfe...Bitte warten", //CXS_CHECKING
	"Entferne ausgewähltes Verzeichnis aus der VST PlugIn Verzeichnis Liste", //CXS_REMOVEVSTDIR_I
	"Auswahl Audio System Hardware", //CXS_SELECTAUDIOSYSTEMHARDWARE
	"Audio Channel Liste", //CXS_LISTOFAUDIOCHANNELS_I
	"Auswahl Audio System", //CXS_SELECTAUDIOCAMX_I
	"Audio Datei Verzeichnisse", //CXS_AUDIOFILESDIRS_I
	"Audio Datei Verzeichnis hinzufügen", //CXS_ADDAÙDIOFILESDIR_I
	"Entferne ausgewähltes Verzeichnis aus der Audio Datei Verzeichnis Liste", //CXS_REMOVEAUDIODIR_I
	"Anzeige von Noten", //CXS_NOTEDISPLAY

	"Zeige Noten (Format)", //CXS_SHOWNOTESAS
	"General MIDI Einstellungen", //CXS_GMSETTINGS_I
	"Kein GM", //CXS_NOGM,
	"", //CXS_CAMXRUNNING
	"Fehler",  //CXS_ERROR
	"Master Datei", //CAMX_MASTERFILE
	"Auswahl Datei/Verzeichnis", //CSX_SELECTMASTERFILE_I
	"Auswahl Datei Sample Format", //CXS_MASTERFILEFORMAT_I
	"Auswahl Bereich", //CXS_SELECTMASTERREGION
	"Starte Mastering/Bounce", //CXS_STARTMASTERING

	"Bouncing stoppen", //CXS_STOPMASTERING
	"Normalisieren", //CXS_NORMALIZE
	"Normalisiere Master Datei", //CXS_NORMALIZE_MI
	"Speichere ab dem ersten - nicht Null-Sample", //CXS_SAVEMASTERFILEFS
	"Speichere Master Datei ab dem ersten Sample, überspringe erste Null Samples", //CXS_SAVEMASTERFILEFS_I
	"Master Datei Fehler:Speichern Öffnen", //CXS_MASTERSAVEERROR
	"Master Datei Fehler:Normalisieren", //CXS_MASTERNORMERROR
	"Mastering beendet", //CXS_MASTERINGEND
	"Kein Mastering durchgeführt", //CXS_NOMASTERINGDONE
	"Von", //CXS_FROM

	"Bis", //CXS_TO
	"Transport auf dem Desktop", //CXS_TRANSPORTONDESKTOP
	"Transport auf dem Desktop ansonsten innerhalb des CamX Fensters", //CXS_TRANSPORTONDESKTOP_I
	"Frage", //CXS_QUESTION
	"Ein CamX Project existiert bereits in diesem Verzeichnis !", //CXS_ACAMXPROALREADYEXISTS
	"Information", //CXS_INFO
	"Auto/Default Song laden ?",//CXS_LOADAUTOQ
	"Auto/Default MIDI File (SMF) Song laden ?", //CXS_LOADAUTOMIDIQ
	"Überschreiben des existierenden Auto-Laden Songs ?",//CXS_OVERWRITEAUTOLOADQ
	"Erzeuge/Clone neue Tracks", //CXS_CREATECLONETRACKS

	"Einstellungen", //CXS_GSETTINGS
	"Keinen Auto-Laden Song gefunden", //CXS_NOAUTOLOAD
	"Auto-Laden Song bereits geöffnet", //CXS_AUTOLOADALREADYOPEN
	"von", //CXS_OF
	"Benutzen Sie diese Version nur zum Testen\n Eine kommerzielle Nutzung dieser Version ist nicht erlaubt!", //CXS_DONTUSETOCREATESONGS
	"Einen neuen Audio Channel einfügen und mit dem Track verbinden", //CXS_ADDNEWCHANNELCT,
	"Einen neuen Instrument Channel einfügen und mit dem Track verbinden", //CXS_ADDNEWAUDIOINSTRUMENTCT,
	"Einen neuen Bus Channel einfügen und mit dem Track verbinden", //CXS_ADDNEWBUSCT
	"Kopiere diese MIDI OUTPUT Einstellungen in andere SELEKTIERTE Tracks", //CXS_CHANGEALLTRACKSMIDIOUT_SEL
	"Kopiere diese MIDI OUTPUT Einstellungen in ALLE anderen Tracks", //CXS_CHANGEALLTRACKSMIDIOUT

	"Es wurden keine Änderungen gemacht", //CXS_NOCHANGES
	"Kopiere diese MIDI INPUT Einstellungen in andere SELEKTIERTE Tracks", //CXS_CHANGEALLTRACKSMIDIIN_SEL
	"Kopiere diese MIDI INPUT Einstellungen in ALLE anderen Tracks", //CXS_CHANGEALLTRACKSMIDIIN
	"Empfange MIDI Events von allen MIDI Input Devices", //CXS_RECEIVEMIDIINFROMALLDEV
	"(*) MIDI Thru für diesen Track auch wenn Track nicht der Focus Track ist", //CXS_MIDITHRUACTIVE
	"Benutze MIDI Input Routing des Songs", //CXS_SONGROUTING
	"MIDI Output", //CXS_MIDIOUTPUT
	"MIDI Input", //CXS_MIDIINPUT
	"Erzeuge Audio Channel", //CXS_CREATEAUDIOCHANNEL
	"Erzeuge Audio Bus Channel", //CXS_CREATEAUDIOBUS

	"Erzeuge Audio Instrument Channel", //CXS_CREATEAUDIOINSTRUMENT
	"Channel löschen", //CXS_DELETECHANNEL
	"Anzeige", //CXS_DISPLAY
	"Aktiver Track", //CXS_ACTIVETRACK
	"Keine Audio Input Channels gefunden", //CXS_NOAUDIOINPUTCHLS
	"Audio Device Input Channels", //CXS_AUDIOINPUTCHLS
	"Channels", //CXS_CHANNELS
	"Keine Audio Output Channels gefunden", //CXS_NOAUDIOOUTPUTCHLS
	"Audio Device Output Channels", //CXS_AUDIOOUTPUTCHLS
	"Keine MIDI Input Devices gefunden", //CXS_NOMIDIINPUTDEVICES

	"Quantisiere Tempo", //CXS_QUANTIZETEMPO
	"Keine MIDI Ouput Devices gefunden", //CXS_NOMIDIOutputDeviceS
	"Dateien", //CXS_FILES
	"Keine Dateien gefunden", //CXS_NOFILESFOUND
	"Keine Audio Datei Verzeichnisse", //CXS_NOAUDIOFILESDIRS
	"Keine VST Plugins gefunden", //CXS_NOVSTPLUGINS
	"Keine VST Plugin Verzeichnisse", //CXS_NOVSTDIRS
	"Metronom (Wiedergabe)", //CXS_METRORECORDING
	"Metronom (Aufnahme)", //CXS_METROPLAYBACK
	"Metronom Klick während des Abspielens", //CXS_METROCLICKWHILEPLAYBACK

	"Diese CPU unterstützt kein SSE2\nBitte installieren Sie CamX SSE1", //CXS_SSEERROR
	"Song schliessen ?", //CXS_CLOSESONG_Q
	"Project schliessen ?", //CXS_CLOSEPROJECT_Q
	"Auto-Laden Song", //CXS_AUTOLOADSONG
	"Fehler: Song Verzeichnis", //CXS_SONGDIRERROR
	"Songs des Projects",//CXS_SONGSOFPROJECT
	"Ersetzen mit",//CXS_REPLACEWITH
	"Auto-Laden Song für importierte SMF (MIDI Format) Songs", //CXS_AUTOLOADFORSMFSONGS
	"Auto-Laden Song für neue, erzeugte Songs", //CXS_AUTOLOADFORSONGS
	"Aktiviere Auto-Laden Song", //CXS_ACTIVEAUTOLOADSONG

	"Eine neue Version ist erhältlich", //CXS_FOUNDNEWVERSION
	"Installiert ist Version", //CXS_THSISVERSIONIS
	"Download der neuen Version", //CXS_DOWNLOADUPDATE
	"Installiere neue Version", //CXS_UPDATERESTART
	"Keine Internet Verbindung", //CXS_NOINTERNET
	"Update URL (Update Info) konnte nicht geöffnet werden", //CXS_NOURLINFO
	"Update URL (Update File) konnte nicht geöffnet werden", //CXS_NOURL
	"Kein Update erhältlich (Die neueste Version ist installiert)", //CXS_UPTODATE
	"Fehler beim Lesen einer URL Datei", //CXS_ERRORREADINGINTERNETFILE
	"Datei konnte nicht zum Schreiben geöffnet werden", //CXS_UNABLETOOPENSAVEFILE

	"Datei konnte nicht zum Lesen geöffnet werden", //CXS_UNABLETOOPENREADFILE
	"Stoppen des Songs fürs Bouncing ?", //CXS_STOPSONGFORMASTERING
	"Es wurden keine Audio Tracks oder Audio Instrument Tracks gefunden", //CXS_NOAUDIOORINSTRUMENTSFOUND
	"Alle Dateien", //CXS_ALLFILES
	"Audio Datei auswählen", //CXS_SELECTAUDIOFILE
	"In Datei", //CXS_TOFILE
	"Laden Tempo Map", //CXS_LOADTEMPOMAP
	"Speichern Tempo Map", //CXS_SAVETEMPOMAP
	"Editiere Effect", //CXS_EDITEFFECT
	"Nach Oben verschieben", //CXS_MOVEUP

	"Nach Unten verschieben", //CXS_MOVEDOWN
	"Editiere Instrument", //CXS_EDITINSTRUMENT
	"Unbekannter Effect", //CXS_UNKNOWNEFFECT
	"Tracks, die diesen Instrumenten Channel benutzen", //CXS_TRACKSUSINGINSTRUMENTCHANNEL,
	"Tracks, die diesen Channel benutzen", //CXS_TRACKSUSINGCHANNEL
	"Pattern ist leer", //CXS_PATTERNEMPTY
	"Selektieren", //CXS_SELECT
	"Erzeugen", //CXS_CREATE
	"Schneiden", //CXS_CUTSPLIT
	"Benutzen", //CXS_USE

	"Audio Datei hinzufügen", //CXS_ADDAUDIOFILE
	"Audio Dateien in einem Verzeichnis hinzufügen", //CXS_ADDFILESFROMDIR
	"Quantisieren", //CXS_QUANTIZE
	"unbenutzte Audio Dateien (im Song Verzeichnis) ?", //CXS_UNUSEDSOUNDFILESINDIR
	"Update Download beendet", //CXS_UPLOADCOMPLETERESTART_Q
	"CamX verlassen und Update installieren ?", //CXS_EXITUPDATE_Q
	"Verbinden mit", //CXS_CONNECTWITH
	"Es werden keine Audio Channels benutzt", //CXS_NOAUDIOCHANNELSUSED
	"Audio Channels benutzt", //CXS_CHANNELSUSED
	"Selektiere alle Tempo Events", //CXS_SELECTALLTEMPOS

	"DeSelektiere alle Tempo Events", //CXS_DESELECTALLTEMPOS
	"Editiere Tempo Event", //CXS_EDITTEMPO
	"Kein Groove", // CXS_NOGROOVE
	"Keine Grooves vorhanden", //CXS_NOGROOVESAVAILABLE
	"Lade Quantisierungs Einstellungen", //CXS_LOADQUANTIZESETTINGS,
	"Speichere Quantisierungs Einstellungen", //CXS_SAVEQUANTIZESETTINGS
	"MIDI Event Typen zum Quantisieren", //CXS_EVENTQTYPES
	"Reset der Quantisierungs Einstellungen", //CXS_RESETQSETTINGS
	"Auswahl Quantisierungs Raster", //CXS_SELECTQRASTER
	"Groove Quantisierung", //CXS_GROOVEQUANTIZE

	"Auswahl Groove Raster", //CXS_SELECTGROOVERASTER
	"Quantisiere Note Off", //CXS_QUANTIZENOTEOFFEND
	"Quantisiere Note Off Position", //CXS_QUANTIZENOTEOFFPOSITION
	"Feste Noten Länge", //CXS_FIXNOTELENGTH
	"Auswahl der festen Noten Länge", //CXS_SELECTFIXNOTELENGTH
	"Capture Quantisierung", //CXS_CAPTUREQUANTIZE
	"Bereich der Capture Quantisierung", //CXS_RANGEOFCAPTURE
	"Strength Quantisierung", //CXS_STRENGTHQUANTIZE
	"Human Quantisierung", //CXS_HUMANQUANTIZE
	"Benutze Human Quantisierung, Randomisierte Quantisierung (+-)", //CXS_USEHUMANQUANTIZE

	"Human Quantisierung Range <- | +>", //CXS_HUMANQUANTIZERANGE
	"Quantisiere auf", //CXS_QUANTIZETO
	"Deaktivert", //CXS_DISABLED
	"Mute alle Tracks (bis auf die selektierten Tracks)", //CXS_MUTEALLTRACKSASEL
	"Einfügen Pattern Buffer", //CXS_PASTEPATTERNBUFFER
	"Kopieren Event/s", //CXS_COPYEVENTS
	"Verschieben Event/s", //CXS_MOVEEVENTS
	"Editieren Tempo Event/s", //CXS_EDITTEMPOEVENTS
	"Löschen Tempo Event/s", //CXS_DELETETEMPOEVENTS
	"Spalten Note", //CXS_SPLITNOTE

	"Löschen Event/s", //CXS_DELETEEVENTS
	"Quantisieren Event/s", //CXS_QUANTIZEEVENTS_S
	"Erzeugen Event/s", //CXS_CREATEEVENTS
	"Erzeugen Tempo Event/s", //CXS_CREATETEMPOEVENTS
	"Editieren Event/s", //CXS_EDITEVENTS
	"MIDI File Header nicht gefunden !", //CXS_NOMIDIFILEHEADER
	"Song ist bereits im Project vorhanden\nEine Kopie des Song erzeugen ?", //CXS_SONGALREADYCOPY_Q
	"Project", //CXS_PROJECT
	"Soll ein neuer Song für das neue Project angelegt werden ?", //CXS_CREATENEWPROJECTSONG
	"Project bereits geöffnet", //CXS_PROJECTALREADYIN

	"CAMV Header in Datei nicht gefunden!\n Möglicherweise gibt es ein neues CamX Datei Format...", //CXS_CAMVHEADERNOTFOUND
	"Datei wurde von einer neueren CamX Version gespeichert! \nBitte UPDATEN Sie CamX um diese Datei zu laden", //CXS_FILENEWER
	"Datei Prüfsummen Fehler", //CXS_CHECKSUMERROR
	"2x Stop Editor Start Position Refresh", //CXS_DOUBLESTOP
	"Default Anzeige Format", //CXS_DEFAULTDISPLAYFORMAT
	"Undo Schritte pro Song", //CXS_UNDOSTEPS,
	"Kein Undo Schritte Limit", //CXS_UNDONOLIMIT
	"Kein Audio Datei Name im Arrange Editor", // CXS_NOAUDIOFILESNAMESAR
	"Zeige Audio Datei Name im Arrange Editor", // CXS_AUDIOFILESNAMESAR
	"Zeige Audio Datei+Pfad Name im Arrange Editor", // CXS_AUDIOFILESPATHNAMESAR

	"Undo Speicher Verbrauch (Song)", //CXS_UNDOMEMORYUSAGE
	"Default Zeit Anzeige", //CXS_DEFAULTTIMEDISPLAY
	"Anzeige Audio Datei Namen im Arrange Editor", //CXS_DISPLAYAUDIOFILESINARRANGE
	"Editors folgen der Song Position", //CXS_ALLEDITORSFOLLOWSONGPOSITION
	"Kein Device", //CXS_NODEVICE
	"Suche nach <Datei nicht gefunden> Audio Dateien",//CXS_SEARCHFORFILENOTEFOUND
	"Entferne alle <Datei nicht gefunden> Audio Objekte", //CXS_REMOVEALLFILENOTEFOUND
	"Keine Audio Dateien", //CXS_NOAUDIOFILES
	"Keine Regions", //CXS_NOREGIONS
	"Kopiere Region", //CXS_COPYREGION

	"Peak Fortschritt", //CXS_PEAKPROGRESS
	"Audio Datei Fehler: Datei Name", //CXS_AUDIOFILEERROR_FILENAME
	"Audio Datei Fehler: Unbekannter Datei Typ", //CXS_AUDIOFILEERROR_FILETYPE
	"Audio Datei Fehler: Datei konnte nicht geöffnet werden (Datei gelöscht ?)", //CXS_AUDIOFILEERROR_FILEUNABLE
	"Auswahl <Suche nach Audio Datei> Verzeichnis", //CXS_SELECTSEARCHDIR
	"Es gibt keine <Datei nicht gefunden> Audio Objekte", //CXS_NOFILESMISSED
	"Regions", //CXS_SHOWREGIONS
	"Audio Datei Info", //CXS_SHOWFILEINFO
	"Audio Datei Pfad", //CXS_SHOWFILEPATH
	"Audio Datei Größe", //CXS_SHOWFILESIZE

	"Audio Datei Dauer", //CXS_SHOWFILETIME
	"Sortieren", //CXS_SORT
	"nach Name", //CXS_BYNAME,
	"nach Größe", //CXS_BYSIZE,
	"nach Datei Datum", //CXS_BYFILEDATE
	"auf Tracks aufgenommene Audio Dateien", //CXS_SHOWCAMXRECORDEDFILES
	"nicht gefundene Audio Dateien", //CXS_NOTFOUNDAUDIOFILES
	"CPU unterstützt SSE2, aber CamX SSE1 ist installiert! Sie sollten CamX mit SSE2 installieren" ,//CXS_SSEINITMSG
	"Taktart", //CXS_SIGNATURE
	"Erzeuge Region", //CXS_CREATEREGION,

	"Lösche Region", //CXS_DELETEREGION
	"Keine Wiedergabe", ////CXS_NOPLAYBACK
	"Region wird von Pattern benutzt!\nTrotzdem löschen ?", //CXS_REGIONUSEBYPATTERN_Q
	"Objekte benutzen Audio Channel\nTrotzdem löschen ?", // CXS_OBJECTSUSINGCHANNEL
	"Lösche alle Channel Instruments+Effects",//CXS_DELETEALLINSTRUMENTSANDFX
	"Lösche alle Channel Instruments", //CXS_DELETEALLINSTRUMENTS
	"Lösche alle Channel Effects", // CXS_DELETEALLFX
	"Fehler beim Aufbau der CamX Threads !", //CXS_INITTHREADERROR
	"Größe der Audio Datei PreBuffer (global)", //CXS_RAFBUFFERSIZE
	"Datei Buffer", //CXS_RAFBUFFER

	"Latenz (Samples)", // CXS_LATENCY
	"Kein Audio Device", //CXS_NOAUDIODEVICE
	"Latenz Audio Device", //CXS_LATENCYAUDIODEVICE
	"Kein Audio Input", //CXS_NOINPUTHARDWARE
	"ASIO Format Fehler", //CXS_ASIOERROR
	"ASIO Format nicht getestet", //CXS_ASIONOTTESTED
	"Bitte eine email schicken: asiobeta@camx.de", //CXS_ASIOEMAIL
	"Kein Send", //CXS_NOSEND
	"Decodiere Datei ins .wav Format", //CXS_DECODEFILE
	"Timer Device konnte nicht geöffnet werden!", //CXS_TIMERINITERROR

	"Momentane Audio Engine (CPU) bzw. Festplatten Lese (HD) Auslastung [Maximale Auslastung] % + (REC) verbleibende Audio Aufnahme Zeit", //CXS_CPUUSAGE
	"Decodiere Audio Datei to.wav und einfügen", //CXS_CONVERTANDINSERT
	"Auswahl encoded Audio Datei", //CXS_SELECTENCODEDFILE
	"Resampling der Audio Datei in die aktuelle Samplerate", //CXS_CONVERTSAMPLERATE
	"Keine Datei!", //CXS_NOFILE
	"Zeige nur Dateien mit aktueller Samplerate", //CXS_SHOWALLSAMPLERATES
	"Exportiere Region als", //CXS_EXPORTREGIONAS
	"Lade Pattern", //CXS_LOADPATTERN
	"Erzeuge Marker mit Pattern Position", //CXS_CREATEMARKERPATTERN
	"Neuer Marker mit 1 Position", //CXS_CREATEMARKERPATTERN_SINGLE

	"Neuer Marker mit Von...Bis Positionen", //CXS_CREATEMARKERPATTERN_DOUBLE
	"Das Song Verzeichnis konnte nicht erzeugt werden !", //CXS_CANTCREATESONGDIRECTORY
	"Beende Background Audio (Audio Datei) Bearbeitung", //CXS_STOPAUDIOFUNCTIONS
	"Abgebrochen", //CXS_CANCELED
	"Default Song Länge (Takte)", //CXS_DEFAULTSONGLENGTH
	"Externe Controller Einstellungen", //CXS_EXTERNCONTROLLER
	"Farbe", //CXS_COLOUR
	"Auswahl Group Farbe", //CXS_SELECTGROUPCOLOUR
	"Neue Group erzeugen", //CXS_CREATENEWGROUP
	"Group Löschen", //CXS_DELETEGROUP

	"Einfügen aktiver Track zur Group", //CXS_ADDACTIVETRACKTOGROUP
	"Einfügen der selektierten Tracks zur Group", //CXS_ADDSELECTEDTRACKSTOGROUP
	"Entferne Focus Track aus Group", //CXS_REMOVEACTIVETRACKFROMGROUP
	"Entfernen der selektierten Tracks aus Group", //CXS_REMOVESELECTEDTRACKSFROMGROUP
	"Ändere Marker Position mit Cycle Position", //CXS_CHANGEMARKERPOSITIONTOCYCLEPOSITION
	"Setze Cycle Positions mit Pattern Start<->End", //CXS_SETCYCLEWITHPATTERNPOSITIONS
	"Setze Cycle Positions mit Marker Start<->End", //CXS_SETCYCLEWITHMARKERPOSITIONS
	"Entfernen", //CXS_REMOVE
	"Instrument hinzufügen", //CXS_ADDINSTRUMENT
	"Effect hinzufügen", //CXS_ADDEFFECT

	"Das Project Verzeichnis konnte nicht erzeugt werden !", //CXS_CANTCREATEPROJECTDIRECTORY
	"Ein Demo Project+Demo Song erzeugen ?", //CXS_Q_CREATEDEMOPROJECT
	"Erzeuge Song Playback Stop Marker", //CXS_CREATEPLAYBACKSTOPMARKER
	"Verschiebe Song Stop Position Marker an diese Position", //CXS_CHANGESONGSTOPMARKER
	"Ändere Notenlänge", //CXS_CHANGELENGTHOFNOTES
	"Konvertiere Drums in Noten", //CXS_CONVERTDRUMSTONOTES
	"Konvertiere Noten in Drums", //CXS_CONVERTNOTESTODRUMS
	"Setze Notenlänge", //CXS_SETLENGTHOFNOTES
	"Erzeuge Cross Fade", //CXS_CREATECROSSFADE
	"Editiere Cross Fade", //CXS_EDITCROSSFADE

	"Länge neuer Noten", //CXS_NEWNOTELENGTH,
	"Benutze vorherige Notenlänge (falls gesetzt)", // CXS_USEPREVIOUSNOTELENGTH,
	"MIDI Kanal von neuen Noten", //CXS_CHANNELOFNEWNOTES
	"Auswahl Zeit Anzeige Takt/SMPTE", //CXS_SELECTTIMEDISPLAY
	"Thru=Benutze Pattern/Track Channel", //CXS_USEPATTERNTRACKCHANNEL
	"Abspielen Region Start->Ende", //CXS_PLAYREGIONSE
	"Abspielen Region vom Ende an", //CXS_PLAYREGIONFROMEND
	"Abspielen Audio Datei und Stop beim Region Start", //CXS_PLAYANDSTOPATREGION
	"Abspielen Audio Datei und Springe über Region", //CXS_PLAYANDSJUMPOVERREGION
	"Audio Dateien", //CXS_AUDIOFILES

	"Information über die Audio Datei", //CXS_INFOAUDIOFILE
	"Region/en würden doch diese Operation betroffen sein. Änderen/Löschen der Region/en", //CXS_REGIONAFFECTED_Q
	"Abspielen Audio Datei (vom Cursor an)", //CXS_PLAYAUDIOFILECURSOR
	"Stoppe das Abspielen der Audio Datei", //CXS_STOPAUDIOFILEPLAYBACK
	"Kopiere Audio Datei > Clipboard", //CXS_COPYAUDIOFILE_CLIPBOARD
	"Kopiere Region > Clipboard", //CXS_COPYREGION_CLIPBOARD
	"Stoppe das Abspielen der Region", //CXS_STOPREGIONPLAYBACK
	"Erzeuge Audio Region (Cycle <> Positionen)", //CXS_CREATEREGIONCYLCE
	"Abspielen und Stop bei der Maus Position", //CXS_PLAYSTOPMP
	"Kopiere diese Audio I/O Einstellungen in andere SELEKTIERTE Tracks", //CXS_CHANGEALLTRACKSAUDIOIO_SEL

	"Kopiere diese Audio I/O Einstellungen in ALLE anderen Tracks", //CXS_CHANGEALLTRACKSAUDIOIO
	"Auswahl Anzahl Tracks", //CXS_SELECTNRTRACKS
	"Anzahl der Tracks", //CXS_NRTRACKS
	"Erzeuge neue Tracks (vom Focus Track an)", //CXS_CREATENEWTRACKSNEXT
	"Erzeuge Child Tracks //", //CXS_CREATECHILDS
	"Erzeuge neue Child Tracks (Child Tracks des Focus Tracks)", //CXS_CREATENEWCHILDTRACKSNEXT
	"Erzeuge neue Tracks (Clone Focus Track)", //CXS_CREATETRACKCLONES
	"Abbruch", //CXS_CANCEL
	"Aufnahme", //CXS_RECORD
	"Aufgenommen/Intern", //CXS_RECORDED

	"Send hinzufügen", //CXS_ADDSEND
	"Auswahl und Info", //CXS_SELECTANDINFO
	"Channel Audio In", //CXS_TRACKUSECHANNELAUDIOIN
	"An", //CXS_ON
	"Alle Fenster schliessen, bis auf die Fenster des aktiven Songs", //CXS_CLOSEALLWINDOWSBUTSONG
	"Alle Fenster schliessen", //CXS_CLOSEALLWINDOWS
	"CamX Desktop hinzufügen",//CXS_ADDCAMXDESKTOP
	"Default Device", //CXS_DEFAULTDEVICE
	"Default Device für neue MIDI Files etc.", //CXS_DEFAULTDEVICE_INFO
	"Device Einstellungen [Editor]", //CXS_AUDIOHARDWARESETTINGS

	"Audio Device Einstellungen/Editor", //CXS_AUDIOHARDWARESETTINGS_I
	"Benutze Device", //CXS_USEDEVICE,
	"Benutze ausgewähltes Device", //CXS_USEDEVICE_I
	"Double Click auf Pattern Einstellungen", //CXS_DOUBLECLICKPATTERN_I
	"Control Events immer", //CXS_CONTROLALWAYS
	"Sende Control Events auch wenn Track/Pattern gemuted ist etc..", //CXS_CONTROLALWAYS_I
	"Sende MIDI Sync Events", //CXS_SENDMIDISYNCEVENTS,
	"Sende MIDI Clock, Start/Continue/Stop, Song Position (Wiedergabe/Aufnahme)",//CXS_SENDMIDISYNCEVENTS_I,
	"Sende MIDI Time Code", //CXS_SENDMTC,
	"Sende MIDI Time Code Events (Wiedergabe/Aufnahme)", //CXS_SENDMTC_I,

	"Sende Metronome Note On/Off Clicks ans MIDI Output Device (Wiedergabe/Aufnahme)", // CXS_SENDMETRONOMETOMIDIOUT
	"Sprachauswahl", //CXS_LANGUAGESELECTION
	"Automatische Noten/Control Event Logik", //CXS_SENDCYCLENOTES
	", die diesen Bus benutzen", //CXS_CHANNELSUSINGBUS
	"Kopieren (Effect)", //CXS_COPYEFFECT
	"Kopieren (Instrument)", //CXS_COPYINSTRUMENT
	"Kopieren Effect Group", //CXS_COPYINSTRUMENTSANDEFFECT
	"Datei wurde von einer neueren CamX Version erzeugt !\nSpeichern nicht möglich\nBitte UPDATEN Sie CamX", //CXS_CANTOVERWRITENEWERFILE
	"Lösche Effect Group",//CXS_DELETEALLIE
	"Lösche alle Instruments", //CXS_DELETEINSTRUMENTS

	"", //EX_CXS_DELETEEFFECTS
	"Mute alle Channels", //CXS_MUTEALLCHANNELS
	"UnMute alle Channels", //CXS_UNMUTEALLCHANNELS
	"Auswahl Cycle Positionen", //CXS_SELECTCYCLE
	"Auswahl vom Marker", //CXS_SELECTMARKER
	"Sende MIDI Clock/Sync", //CXS_SENDMTCSPP
	"Quantisiere interne Song Position auf SPP (1/16)", //CXS_QUANTIZESPP
	"Empfang von MIDI Clock+Song Position+Start/Stop", //CXS_RECEIVEMTCSPP
	"Empfang von MIDI Time Code", //CXS_RECEIVEMTC
	"Name des Songs", //CXS_SONGNAME

	"Eingeschaltet", //CXS_ENABLED
	"Lade zuletzt benutztes Project beim Starten", //CXS_LOADLASTUSEDPROJECT
	"Zeige Takte UND SMPTE in Timeline", //CXS_SHOWMEASUREANDSMPTETIMELINE
	"Zeige Editor Mauszeiger-Tooltip", //CXS_SHOWEDITORTOOLTIPS
	"Audio Peak Datei: Generieren+Schreibe Datei", //CXS_WRITEPEAKFILE
	"Audio Peak Datei: Keine Peak Datei", //CXS_WRITENOPEAKFILE
	"Sample Format der Resamping Audio Dateien", //CXS_RESAMPLINGFORMAT
	"Input Datei", //CXS_SAMEASINPUTFILE
	"Project Einstellungen", //CXS_PROJECTSETTINGS
	"Project Name", //CXS_PROJECTNAME

	"Zeige nur Tracks mit Audio (Pattern oder Effects)", //CXS_SHOWONLYTRACKSWITHAUDIO
	"Alles DeSelektieren", //CXS_UNSELECTALL
	"Speichern MIDI Filter Einstellungen", //CXS_SAVEMIDIFILTER
	"Laden MIDI Filter Einstellungen", //CXS_LOADMIDIFILTER
	"Suche nach Update beim Starten", //CXS_AUTOUPDATE
	"Diese CPU unterstützt kein SSE/SSE2 !!!", //CXS_SSEERROR
	"Event Ausgabe Typ: MIDI/Audio Instrument", //CXS_TRACKTYPE
	"Maus/Editieren Modus", // CXS_STATUSMOUSE
	"Scrollen des Editors mit der Song Position", // CXS_SCROLLEDITORWITHSONGPOSITION
	"Das aktive Project hat Audio Pattern und benutzt eine andere Samplerate !", //CXS_PROJECTSAMPLEMSG

	"Das Project benutzt eine auf diesem Audio System nicht vorhandene Sample Rate !", //CXS_PROJECTUSESNONEXISTINGSAMPLERATE
	"Track neu", //CXS_NEWTRACK
	"Mehrfaches Öffnen von Editoren gleicher Art", // CXS_OPENMULTIEDITOR
	"Zeige Audio Device I/O Hardware Channels", //CXS_SHOWAUDIODEVICEIO
	"Ein Project dem Player hinzufügen", //CXS_ADDPROJECT
	"Song entfernen", //CXS_DELETESONG,
	"Wollen Sie wirklich den Song und die Song Daten löschen ?", //CXS_DELETESONG_Q,
	"Editieren Text Event/s", //CXS_EDITTEXTEVENTS
	"Editieren Marker Event/s", //CXS_EDITTEXTEVENTS
	"Start - Audio Aufnahme -Threshold", //CXS_CHECKAUDIORECORDPEAK

	"Löschen Text/s Events", //CXS_DELETETEXTEVENTS
	"Löschen Marker Events", //CXS_DELETEMARKEREVENTS
	"MouseOver Noten Abspielen", //CXS_PLAYPIANOMOUSEOVER
	"Setze Cycle Start", //CXS_SETCYCLESTARTHERE,
	"Setze Cycle Ende", //CXS_SETCYCLEENDHERE,
	"Immer Überprüfung auf neue CamX Version/Update (Internet Zugriff)?", //CXS_AUTOUPDATEQUESTION
	"Taktdarstellung", //CXS_MEASUREDISPLAY
	"Alle anderen Fenster schliessen", //CXS_CLOSEALLWINDOWSBUTTHIS
	"Unbenutzte Song Audio Dateien:Fragen/[Löschen]", //CXS_UNUSED_1
	"Unbenutzte Song Audio Dateien:Niemals Löschen", //CXS_UNUSED_2

	"Unbenutzte Song Audio Dateien:Auto Löschen", //CXS_UNUSED_3
	"Importiere externe Audio Dateien ins Song Audio Import Verzeichnis", //CXS_IMPORTTOAUDIOSONGINPORT,
	"Abfrage falls externe Audio Dateien bereits im Song Audio Import Verzeichnis existieren", //CXS_IMPORTTOSAMEDIRECTORY,
	"Song Audio Import Dateien", //CXS_SHOWFILEINFOINTERN
	"Verschiebe Pattern ---> Benutze Cycle Range", //CXS_MOVEPATTERNCYCLERANGE_RIGHT,
	"Verschiebe Pattern <--- Benutze Cycle Range", //CXS_MOVEPATTERNCYCLERANGE_LEFT
	"Position links von 1.1.1.1! Verschieben ?", //CXS_QMOVEPATTERN
	"alle Pattern", //CXS_ALLPATTERN
	"alle selektierten Pattern", //CXS_SELECTEDPATTERN
	"alle Pattern der selektierten Tracks", //CXS_ALLPATTERNSELECTEDTRACKS

	"Ändere Track Typ in Instrument falls ein Instrument vorhanden", //CXS_AUTOINSTRUMENT
	"Exportiere selektierte Audio Pattern des Focus Tracks (+CrossFades)", // CXS_EXPORTSELECTEDPATTERN
	"Wiedergabe [OUT] CrossFade Region ohne Volume Änderungen", //CXS_PLAYOUTCROSSFADE_WITHOUTCROSSFADE,
	"Wiedergabe [OUT] CrossFade Region mit Volume Änderungen", //CXS_PLAYOUTCROSSFADE_WITHCROSSFADE,
	"Wiedergabe [IN] CrossFade Region ohne Volume Änderungen", //CXS_PLAYINCROSSFADE_WITHOUTCROSSFADE,
	"Wiedergabe [IN] CrossFade Region mit Volume Änderungen", //CXS_PLAYINCROSSFADE_WITHCROSSFADE,
	"Wiedergabe CrossFade Mix mit Volume Änderungen", //CXS_PLAYCROSSFADEMIX
	"Wiedergabe CrossFade Mix ohne Volume Änderungen", //CXS_PLAYCROSSFADEMIX_WITHOUTVOLUME
	"Ändern Start/Ende von Pattern", //CXS_F_SIZEPATTERN
	"Setze Loop Ende mit diesem Pattern", //CXS_SETLOOPEND

	"Setze > Start Offset", // CXS_SETOFFSETSTART
	"Setze < End Offset", // CXS_SETOFFSETEND
	"> Start Offset Reset", // CXS_CANCELOFFSETSTART
	"< End Offset Reset", // CXS_CANCELOFFSETEND
	"Start+End Offset Reset", // CXS_CANCELOFFSETBOTH
	"Exportiere selektierte Audio Pattern des Focus Tracks (+CrossFades/Split Mitte der CrossFades)", // CXS_EXPORTSELECTEDPATTERNSPLIT
	"Name des Processors", //CXS_NAMEOFPROCESSOR
	"Liste der Processoren", //CXS_LISTOFPROCESSORS
	"Liste der Processor Module", // CXS_PROCESSORMODULES
	"Neuer Processor", //CXS_ADDNEWPROCESSOR

	"Lösche Processor", //CXS_DELETEPROCESSOR
	"Editiere Modul", //CXS_EDITMODULE
	"Neues Modul", //CXS_ADDMODULE
	"Lösche Modul", //CXS_DELETEMODULE
	"Einfügen in", //CXS_ADDTO
	"Einfügen", //CXS_ADD
	"Module", //CXS_MODULES
	"Benutze bereits existierende Audio Datei? \nNein=Erzeuge neue Datei", //CXS_FILEEXISTSINIMPORT
	"Datei existiert bereits\nÜberschreiben?", //CXS_QOVERWRITEFILE
	"Datei wird benutzt, bitte wählen Sie eine andere Datei aus", //CXS_FILEUSED

	"Alle Fenster schliessen, bis auf die Fenster des aktiven Projects", //CXS_CLOSEALLWINDOWSBUTPROJECT
	"Alle selektierten Tracks", //CXS_ALLSELECTEDTRACKS
	"Alle Tracks", // CXS_ALLTRACKS
	"MIDI Aufnahme ins erste selektiere MIDI Pattern des Tracks", //CXS_RECTOFIRSTPATTERN
	"Selektiere alle Text Events", //CXS_SELECTALLTEXTEVENTS
	"DeSelektiere alle Text Events", //CXS_UNSELECTALLTEXTEVENTS
	"Selektiere alle Marker Events", //CXS_SELECTALLMARKEREVENTS
	"DeSelektiere alle Marker Events", //CXS_UNSELECTALLMARKEREVENTS
	"Stop des Update Downloads", //CXS_STOPUPDATEDOWNLOAD
	"Audio Tracks Effecte An/Aus", //CXS_TRACKMIXER_FX

	"Audio Channels/Instruments/Bus Effects An/Aus", //CXS_AUDIOMIXER_FX
	"Error: Kopiere Datei", //CXS_COPYFILEERROR
	"Keine Samples aufgenommen...", //CXS_NOSAMPLESRECORDED
	"Zeige Device Name", //CXS_SHOWDEVICENAME
	"Löschen", //CXS_CLEAR
	"Öffne neuen MIDI Monitor", //CXS_OPENNEWMONITOR
	"Alle MIDI Out Devices", //CXS_ALLMIDIOutputDeviceS
	"Alle MIDI In Devices", //CXS_ALLMIDIINPUTDEVICES
	"Zeige Note Off's in Monitors", //CXS_SHOWNOTEOFFSINMONITOR
	"Verbinde andere selektierten (Track)Pattern mit diesem Pattern", //CXS_CONNECTSELECTP

	"Stunde", //CXS_HOUR
	"Minute", //CXS_MINUTE
	"Sekunde", //CXS_SECOND
	"Ordner", //CXS_DIRECTORY
	"geöffnet", //CXS_OPEN
	"Alle anderen Songs schliessen", //CXS_CLOSEALLOTHERSONGS
	"Auto Cut Threshold Audio Aufnahme", //CXS_AUTOCUTZEROSAMPLES
	"Fehler beim Aufnehmen von Audio Datei/en !!!\nMedium voll ?\nFehlerhafte Audio File/s behalten?", //CXS_ERRORRECORDING_Q
	"Speichern Song Fehler! Medium voll ?", //ERROR_SONGSAVE
	"Speichern Project Fehler! Medium voll ?", //ERROR_PROJECTSAVE

	"Speichern Project oder Song Fehler! Medium voll ?\nSpeichern wiederholen ?",//ERROR_PROJECTSONGSAVE
	"Setze Tracks auf MIDI Input/Thru=Immer beim Einfügen eines Instruments", //CXS_AUTOTRACKMIDITHRU
	"Kopiere diese MIDI INPUT+OUTPUT Einstellungen in andere SELEKTIERTE Tracks", //CXS_CHANGEALLTRACKSMIDI_SEL
	"Kopiere diese MIDI INPUT+OUTPUT Einstellungen in ALLE anderen Tracks", //CXS_CHANGEALLTRACKSMIDI
	"Keine Audio Hardware gefunden...", //CXS_NOAUDIOHARDWAREFOUND
	"mit Pattern Ende", //CXS_SETSPP_PATTERNEND
	"Kopiere selektierte Pattern an Maus Zeiger Position", //CXS_COPYSELPATTERNTP
	"Verschiebe selektierte Pattern an Maus Zeiger Position", //CXS_MOVESELPATTERNTP
	"Plugin/s konnten nicht geladen werden!", //CXS_UNABLETOLOADPLUGINS
	"Fehler beim Laden des Songs! Song benutzen?", //CXS_ASKLOADERRORSONG

	"Kopiere selektierte Pattern zum Track", //CXS_COPYSELPATTERNTPNOPOS
	"Verschiebe selektierte Pattern zum Track", //CXS_MOVESELPATTERNTPNOPOS
	"Defekte Audio Dateien ignorieren", //CXS_IGNORECORRUPTAUDIOFILES
	"Importiere Song Arrangement aus Datei?", //CXS_IMPORTARRANGEMENT_Q
	"Song Arrangement als Datei speichern", //CXS_SAVESONGARRANGEMENT
	"Track Input Event Filter (Aufnahme/Thru)", //CXS_TRACKINPUTEVENTFILTER
	"",
	"Audio:VST 3 Plugin Verzeichnisse", //CXS_SET_VSTDIRS3
	"",
	"Verlassen (Änderungen NICHT speichern)", //CXS_EXITCAMX_DONTSAVE

	"Einfügen anderer selektieren Tracks als Child Tracks", //CXS_ADDOTHERTRACKASCHILD
	"Lade Data Dump ins Plugin", //CXS_LOADDATADUMPPLUGIN
	"Speichere Data Dump vom Plugin",  //CXS_SAVEDATADUMPFROMPLUGIN
	"Abspielen", //CXS_PLAY
	"mit Maus Zeiger Position", //CXS_SETMOUSEP
	"Kopiere andere selektierte Pattern zur Pattern Startposition", //CXS_COPYSELPATTERNTPS
	"(< Align >) Verschiebe andere selektierte Pattern zur Pattern Startposition", //CXS_MOVESELPATTERNTPS
	"Mastering Modus", //CXS_MASTERTYPES
	"Master: Song Mix", //CXS_MASTERMIX
	"Bounce: alle selektierten Tracks", //CXS_BOUNCESELTRACKS

	"Bounce: alle selektieren Channels", // CXS_BOUNCESELCHANNELS
	"Bounce: alle selektieren Tracks+selektierten Channels", //CXS_BOUNCESELTRACKCHANNELS
	"Alle selektierten Child Tracks freigeben", //CXS_RELEASECHILDTRACKS
	"Audio Input immer AN (CPU Energy -)", //CXS_AUDIOINPUTALWAYSON
	"Audio Input nur AN wenn benötigt (CPU Energy +)", // CXS_AUDIOINPUTONLYNEED
	"Auto-Benennung des Track mit Program Namen", //CXS_SETTRACKNAMEASPROGRAMNAME
	"Laden", //CXS_LOAD
	"Ändern Automation Typ", // CXS_CHANGESUBTRACK
	"Plugins: Kein Crash Check, kein Realtime Timing Check [CPU Belastung 0]", //CXS_NOPLUGINCHECK
	"PlugIns: Crash Check  [CPU Belastung 1]", //CXS_PLUGINCHECK1,

	"Plugins: Crash Check+Realtime Timing Check  [CPU Belastung 2]", //CXS_PLUGINCHECK2
	"Program auswählen", //CXS_SELECTPLUGINPROGRAM
	"Erstes (gemutete) Event", //CXS_FIRSTMUTEEVENT
	"Letztes (gemutete) Event", //CXS_LASTMUTEEVENT
	"Mute selektierte Events", //CXS_MUTESELECTEDEVENTS
	"UnMute selektierte Events", //CXS_UNMUTESELECTEDEVENTS
	"UnMute alle Events", //CXS_UNMUTEALLEVENTS
	"Datei importieren - Lade Arrangement Frage",// CXS_IMPORTFILEQUESTION
	"Mute Parent Tracks von Aufnahme Tracks beim Erreichen vom Cycle Ende", //CXS_MUTEPARENTCYLCETRACKS
	"Importiere Audio Datei und Spalte Channels->Tracks", //CXS_SPLITAUDIOCHANNELS

	"Spielen oder Editieren", //CXS_PLAYOREDIT
	"Audio Aufnahme Fehler! Audio Aufnahme gestoppt", // CXS_AUDIORECORDINGERROR
	"Bufferoverflow - Aufnahme Medium zu langsam ?", //CXS_AUDIORECORDINGERROR_BUFFEROVERFLOW
	"Standard Audio System Samplerate, Samplerate für neue Projects", //CXS_AUDIOSYSTEMSAMPLERATE
	"Punch Out+Cycle Recording, keine Aufnahme möglich", //CXS_PUNCHCYCLEERROR
	"Vorzähler nur bei Song Position 1.1.1.1", //CXS_PRECOUNTERATPOSITIONONE
	"Vorzähler bei jeder Song Position", //CXS_PRECOUNTERALWAYS
	"Erlaube Tempo Wechsel Aufnahme", //CXS_ALLOWTEMPORECORDING
	"Erzeuge Parent (Folder) Track", //CXS_CREATEPARENTFORSELECTEDTRACKS
	"Sende Events an MIDI Device/s", //CXS_SENDTRACKALWAYSTOMIDI

	"Sende Events an Audio Plugin/s", //CXS_SENDTRACKALWAYSTOAUDIO
	"Sende Events an MIDI Device/s+Audio Plugin/s", //CXS_SENDTRACKALWAYSTOMIDIANDAUDIO
	"Sende Events an ... immer automatisch festlegen", //CXS_SENDTRACKALWAYSTOAUTO
	"Buffer Größe (Samples)", //CXS_AUDIOSETDEVICESIZE
	"Trennen von MIDI Files Typ 0 (1 Track) in Tracks Channels/SysEx", //CXS_AUTOSPLITMIDIFILE
	"Zeige alle Folder",//CXS_SHOWALLFOLDER
	"Verstecke alle Folder", //CXS_HIDEALLFOLDER
	"Zeige alle Event Outputs an Song Plugins", //CXS_SHOWALLPLUGINDEVICES
	"CPU/HD Auslastung", // CXS_CPUUSAGE_EDITOR
	"Keine Song Audio Aufnahme", //CXS_NOSONGAUDIORECORDING

	"Audio Aufnahme Zeit übrig", //CXS_AUDIORECORDINGTIME
	"Erstem Track", //CXS_FIRSTTRACK
	"Letztem Track", //CXS_LASTTRACK
	"Erstem selektierten Track", //CXS_FIRSTSELECTEDTRACK
	"Letztem selektierten Track", //CXS_LASTSELECTEDTRACK
	"Sekunden", //CXS_SECONDS
	"Zeige alle Tracks", //CXS_AUTOALL
	"Zeige selektierte Tracks", //CXS_AUTOSELECTED
	"Zeiger nur Focus Track", //CXS_AUTOFOCUS
	"Zeige Tracks mit Audio/Instruments oder MIDI", //CXS_AUTOAUDIOORMIDI

	"Zeige Tracks mit Audio/Instruments", //CXS_AUTOAUDIO
	"Zeige Tracks mit MIDI", // CXS_AUTOMIDI
	"Kein Pattern Check", //CXS_NOPATTERNCHECK
	"nur mit Pattern ab Song Position", ////CXS_ONLYPATTERNFROMTO
	"nur mit Pattern unter Song Position", // CXS_ONLYPATTERNATSONGPOSITION
	"Output als Input freigeben", //CXS_SETTRACKOUTPUTFREE
	"Audio oder MIDI I/O Anzeige", //CXS_VUTYPE
	"Kein Filter", // CXS_NOFILTER
	"Für den neuen Song einen eigenen Desktop erzeugen? \nAnsonsten wird der vorhandene Song ersetzt",// CXS_CREATEDESKTOPFORSONG_Q
	"Zeige MIDI->Instrument Tracks", //CXS_AUTOINSTR
	
	"Keine Master Datei", //CXS_NOMASTERFILE
	"Taktart (Song Position)", //CXS_SONGSIGNATURE
	"Maus Bewegungsrichtung im Overview Bereich", // CXS_OVERVIEWMOUSE
	"Setze Start des Editors mit Song Position", //CXS_SCROLLEDITORTOSPP
	"Weitergeben von MIDI Input Events an MIDI Output und Plugins", //CXS_MIDITHRUINFO
	"Der Song konnte nicht geladen/generiert werden", //CXS_UNABLETOLOADSONG
	"Speicher/Medium Problem", //CXS_MEMORY
	"Songs geöffnet:", //CXS_SONGSOPEN
	"Project schliessen ?", //CXS_QUESTIONCLOSEPROJECTSCREEN
	"Erzeuge ein neues Project in diesem Verzeichnis ?", //CXS_CREATEPROJECT_Q
	
	"Mauszeiger Quantisierung/Zeit Anzeige", //CXS_MOUSEQUANTIZEANDTIME
	"Audio+MIDI I/O Effects", //CXS_AUDIOMIDIFX
	"Audio+MIDI Input Effects", //CXS_AUDIOMIDIINPUTFX
	"Diesen Song als aktiven Song festlegen", //CXS_ACTIVATESONG
	"ASIO Audio System gefunden!\nSoll ASIO nun das Standard Audio System werden ?\n(ASIO bietet die beste Audio Latenz/Qualität)", //CXS_ASIOWASFOUND
	"Neue Fenster werden in diesen Screen gedocked", //CXS_DOCK
	"Erweiterte Quantisierung/Groove Einstellungen", //CXS_QUANTIZEEDITOREX
	"Bereich der grafischen Tempo Darstellung", //CXS_RANGETEMPO
	"Timeline - Song (mit Tempo Änderungen) oder Sample Modus", //CXS_SONGORSAMPLES
	"Verwaltungen/Einstellungen der Plugin Outputs", //CXS_SEPARATEPLUGINOUTS
	
	"Plugin An/Aus", //CXS_PLUGINONOFF
	"Einfügen Effect Group", //CXS_PASTINSTRUMENTSANDEFFECT
	"Anzeige dB Skala/Channel", //CXS_DBSCALE
	"MIDI Event Ausgabe an MIDI-Device oder Plugins", //CXS_MIDIOUTPUT_TYPE
	"Aufnahme : MIDI (Rm) oder Audio (R)", //CXS_MIDIAUDIORECORD
	"Setze Start des Editors mit Cycle Start", //CXS_SCROLLEDITORTOCYCLELEFT
	"Setze Start des Editors mit Cycle Ende", //CXS_SCROLLEDITORTOCYCLERIGHT
	"Editor Raster", //CXS_EDITORGRID
	"Default Song Raster (1.1.X.1)", //CXS_DEFAULTSONGGRID
	"Raster", //CXS_SONGGRID

	"MIDI Files: GS/GM Pattern niemals hinzufügen", // CXS_MIDIFILESNOSYSEX,
	"MIDI Files: GM hinzufügen (falls nicht vorhanden)", // CXS_MIDIFILESADDGM,
	"MIDI Files: GS hinzügen (falls nicht vorhanden)", // CXS_MIDIFILESADDGS,
	"Autosave: Aus", //CXS_AUTOSAVEOFF,
	"Autosave: Alle 2 Minuten", //CXS_AUTOSAVE2MIN,
	"Autosave: Alle 5 Minuten", //CXS_AUTOSAVE5MIN,
	"Autosave: Alle 10 Minuten", //CXS_AUTOSAVE10MIN
	"Samplerate", //CXS_SAMPLINGRATE
	"<<< Benutze Buffer Größe", //CXS_USEUSERBLOCKSIZE
	"Dieses Device hat keinen Editor", //CXS_AUDIODEVICEHASNOEDITOR

	"Kein Device des Systems kann die Samplerate eines der offenen Projects anbieten!",
	"Der ASIO Treiber gibt eine größere Minimum Buffer Size zurück!\nBuffer Size trotzdem setzen?",
	"Der ASIO Treiber gibt eine kleinere Maximum Buffer Size zurück!\nBuffer Size trotzdem setzen?",
	"Name", //CXS_NAME
	"Typ", //CXS_TYPE
	"Abspielen nur Pattern dieser Pattern Selection (temporär)", // CXS_PLAYPATTERNSOLO
	"Aktiviert", //CXS_ACTIVATED
	"Hardware gefunden", //CXS_HARDWAREFOUND
	"Neue Tracks", //CXS_NEWTRACKS
	"Verzeichnis (selektierte Tracks)", //CXS_MASTERDIRECTORYSELTRACKS

	"Master Verzeichnis/Datei Problem (Open Save)", //CXS_MASTERSAVEERROR_DIRECTORY
	"Lade Audio Datei Verzeichnis in den Song (pro Datei ein neuer Track)", //CXS_LOADDIRECTORY
	"Lade Audio Datei in Song und spalte Audio Datei in Channels (Mono Track/Channel)", //CXS_LOADAUDIOFILEANDSPLITTOCHANNELS
	"Standard M. Einstellungen übernehmen", // CXS_USEDEFAULTMETRO
	"UI+Maus", //CXS_SET_UI
	"Nur vertikale Maus Bewegung (Fader,Buttons)", //CXS_ONLYVERTICALMOUSE
	"Zeige nur Tracks mit Aufnahme aktiviert", // CXS_AUTOTRACKSRECORD
	"Reset Automation Parameter", //CXS_RESETAUTOOBJ
	"Kein Track Freeze durchgeführt", //CXS_NOFREEZEDONE
	"Starte Audio Freezing (selektierte Tracks)", //CXS_STARTFREEZING

	"Automatisch einen Automations Track bei Plugin Parameter Änderungen erzeugen", //CXS_AUTOMATIONPLUGINSCREATE
	"Neuer Track/Channel: Erzeuge Audio Volume Automation Track", //CXS_AUTOMATIONCREATEAUDIOVOLUME
	"Neuer Track/Channel: Erzeuge Audio Pan Automation Track", //CXS_AUTOMATIONCREATEAUDIOPAN
	"Neuer Track: Erzeuge MIDI Pitchbend Automation Track", //CXS_AUTOMATIONCREATEMIDIPITCHBEND,
	"Neuer Track: Erzeuge MIDI Volume Automation Track", //CXS_AUTOMATIONCREATEMIDIVOLUME,
	"Neuer Track: Erzeuge MIDI Modulation Automation Track", //CXS_AUTOMATIONCREATEMIDIMODULATION,
	"Neuer Track: Erzeuge MIDI Note Velocity Automation Track", //CXS_AUTOMATIONCREATEMIDIVELOCITY,
	"Automation : AN", // CXS_AUTOMATIONON,
	"Automation : AUS", //CXS_AUTOMATIONOFF,
	"Erlerne von eintreffenden MIDI/Plugin Events", //CXS_LEARN

	"Löschen Plugin", //CXS_DELETEPLUGIN
	"Automatisch einen Automations Track beim Editieren und Aufnahme/Wiedergabe erzeugen", //CXS_PLUGINAUTOMATE
	"Plugin einfügen", //CXS_CREATEPLUGIN
	"Plugin ersetzen", //CXS_REPLACEPLUGIN
	"Neuer Track/Bus: Erzeuge Mute Automation Track", //CXS_AUTOMATIONCREATEMUTE,
	"Neuer Track/Bus: Erzeuge Solo Automation Track", //CXS_AUTOMATIONCREATESOLO,
	"Erzeuge Automations Parameter im Aufnahme Modus", //CXS_AUTOMATIONRECORDINGRECORD,
	"Erzeuge Automations Parameter im Playback Modus", //CXS_AUTOMATIONRECORDINGPLAYBACK,
	"Verschiebe Objekte horizontal <->",//CXS_XMOVE
	"Verschiebe Objekte vertikal",//CXS_YMOVE

	"Automation Parameter Wiedergabe", //CXS_AUTOMATIONPLAYBACK,
	"Automation Parameter Änderungen Aufnahme", //CXS_AUTOMATIONRECORDING,
	"! Learn UND erzeuge einen neuen Automations Track ! ", //CXS_LEARNANDCREATE
	"Binde selektiere Automation Parameter ans Pattern", //CXS_BINDSELECTAUTOMATIONPARAMETER
	"Frage ob <ASIO> als Standard Audio System benuzt werden soll", //CXS_USEASIOQUESTION
	"Frage beim Starten von CamX ob ASIO benutzt werden soll, falls noch nicht ASIO benutzt wird",//CXS_USEASIOQUESTION_I
	"SMPTE Anzeige Offset", //CXS_SMPTEDISPLAYOFFSET
	"Anzeigen/Editieren von Automation Tracks", //CXS_INFOAUTOMATION
	"Anzeigen/Editieren von Volume Kurven", //CXS_INFOVOLUME
	"Keine VST Exceptions (schnell aber kein VST Plugin Crash Abfang)", // CXS_VSTNOCHECKS,

	"VST Exceptions (Abfang von VST Plugin Crashes)", // CXS_VSTTRYCATCH,
	"VST Exceptions+Timer (Abfang + Messe Plugin Zeitverbrauch)", //CXS_VSTTRYCATCHANDTIMER
	"Cursor Oben", //CXS_CURSORUP
	"Cursor Unten", //CXS_CURSORDOWN
	"Schreibe Null-/Stille Start Samples (Pause)", //CXS_MASTERPAUSE
	"Überprüfe Space-Taste auch non Focus", //CXS_CHECKSPACEBACKNONFOCUS
	"UnFreeze (selektierte Tracks)", //CXS_UNFREEZE
	"Erster Tempo Wert", //CXS_FIRSTTEMPOVALUE
	"Letzter Tempo Wert", //CXS_LASTTEMPOVALUE
	"Erster selektierter Tempo Wert", //CXS_FIRSTSELECTEDTEMPOVALUE

	"Letzter selektierter Tempo Wert", //CXS_LASTSELECTEDTEMPOVALUE
	"Abspielen selektierter Bereich", //CXS_PLAYAUDIOFILECLIP
	"Verschieben Song Position zum nächsten Takt (+Shift vorherige Takt)", //CXS_MOVESONGPOSITIONNEXTMEASURE
	"Setze Tempo Wert der selektierten Tempo Events auf xxx.000", //CXS_TEMPOEVENTRESET
	"Setze Song Position Tempo mit Tap Wert", //CXS_SETTAPVALUE
	"Tap Wert - Click:Reset", //CXS_TAPTEMPO
	"In Echtzeit aufgenommene Tempo Events", //CXS_REALTIMERECTEMPOEVENTS
	"Selektierte Region löschen ?", //CXS_DELETEREGION_Q
	"Ein oder mehrere Audio Dateien haben eine falsche Sample Rate\\Weitermachen mit Import?", //CXS_Q_IMPORTSTOP
	"Speichere Pattern als SysEx File", //CXS_SAVEPATTERNASSYSEX

	"Sende dieses SysEx Pattern NUR beim Song Startup", //CXS_SENDTHISPATTERNONLYSTARTUP
	"Sende alle - NUR beim Song Startup senden - SysEx Pattern", //CXS_SENDALLSYSEX
	"SysEx Startup Pattern wurde/n gesendet", //CXS_COUNTSYSEXSTARTPATTERN
	"Erzeuge X neue Bus Channels", //CXS_CREATEXNEWBUS
	"Erzeuge Bus Channels", //CXS_CREATEBUS
	"Neue Bus Chl.", //CXS_NEWBUS
	"Löschen Bus Channels", //CXS_DELETEBUS
	"Automatische MIDI/AUDIO Selektion", //CXS_AUTOMIX
	"Bevorzugter Track Typ", //CXS_PREFTRACKTYPE
	"Sende MIDI Bank Select Message beim Song Start", //CXS_SENDBANKSELECT

	"Sende MIDI Program Change Message beim Song Start", //CXS_SENDPROGRAMCHANGE
	"Benutze diesen MIDI Channel für Bank Select/Program Change, FALLS [Track MIDI Channel] auf ALL gesetzt ist", //CXS_USEBANKSELECTCHANNEL
	"MIDI File Typ 0 (1 Track) - kein Track Splitting", //CXS_NOAUTOSPLITMIDIFILE
	"MIDI File Typ 0 (1 Track) - Message-Abfrage nach Track Splitting", //CXS_NOAUTOSPLITMIDIFILE
	"Neu", //CXS_NEW
	"Spalte MIDI File Typ 0 (1 Track) in MIDI Channels ?", //CXS_ASKSPLITMIDIFILE0
	"Setze Focus Track auf Aufnahme, falls kein anderer Track auf Aufnahme gesetzt ist", //CXS_SETAUTORECORD
	"Selektiere alle gleichen Events auf gleicher Position", ////CXS_SELECTALLDOUBLEEVENTSPOSITION
	"Keine Double Events gefunden...", //CXS_NODOUBLEEVENTSFOUND
	"Audio Pattern (User)Move Modus", //CXS_MOVEBUTTON
};
#endif


