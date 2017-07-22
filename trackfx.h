#ifndef CAMX_TRACKFX_H
#define CAMX_TRACKFX_H 1

#ifdef OLDIE

#include "defines.h"
#include "MIDIeffects.h"
#include "audiomixeditor.h"

class Edit_Arrange;
class MIDIOutputDevice;
class MIDIInputDevice;
class Seq_Group;
class Processor;

// MIDI FX Editor
class Edit_ArrangeEditorEffects
{
	friend GUI;

public:	
	Edit_ArrangeEditorEffects();

	void FreeMemory();
	void InitTrackGadgets(int y);
	void InitPatternGadgets(int y);
	void InitTrackMixer(int y);
	bool Init();
	void ShowActiveTrack();
	void ShowActivePattern();
	void RefreshRealtime();
	void ResetGadgets();

	void Close();

	EditData *EditDataMessage(EditData *);
	guiGadget *Gadget(guiGadget *);

	Edit_Arrange *arrangeeditor;
	Edit_Frame *frame;

	Seq_Track *status_track;
	Seq_Pattern *status_pattern;

	guiGadgetList *trackglist,*patternglist;
	guiGadget *trackname;

	int endtracky,starttrackfx_y;

	// Child
	Edit_TrackAudioMix trackmix;
	int oldframex,oldframey;

	bool dontshowpatternname;
private:
	void ShowProgram(guiGadget *,MIDIOutputProgram *);
	void ShowTrackOnOff();
	void ShowPatternOnOff();

	void ShowTrackIcon();
	void ShowGroup();

	void ShowMIDIChannel();
	void ShowMIDITranspose();
	void ShowMIDIVelocity();
	void ShowMIDIDelay();
	void ShowMIDIInput();
	void ShowMIDIProgram();

	void ShowPatternProgram(MIDIPattern *);

	void ShowMIDIOutput();
	void ShowProcessor();
	void ShowMIDIOutputFilter();
	void ShowMIDIInputFilter();
	void ShowTrackType();

	MIDIEffects status_MIDIPatternfx;
	QuantizeEffect status_trackquantize,status_patternquantize;
	MIDIFilter status_inputfilter,status_outputfilter;
	MIDIOutputProgram status_MIDIprogram,status_MIDIPatternprogram;

	char *patternnamestring,*status_processorname;
	TrackIcon *status_icon;
	MIDIOutputDevice *status_MIDIoutdevice;
	MIDIInputDevice *status_MIDIindevice;
	Seq_Group *status_group;
	Groove *status_groove,*status_patterngroove;
	Processor *status_processor;

	guiGadget_Integer *patternloopnumber;
	guiGadget *patternmute,
		// MIDI
		*trackimage,
		*MIDIchannel,
		*MIDIthru,
		*MIDIoutprogram,
		*MIDIoutput,
		*MIDIinput,
		*MIDIinfilter,
		*quantize,
		*transpose,
		*velocity,
		*filter,
		*delay,
		*trackstatus,
		*patternname,
		*patternquantize,
		*patternMIDIchannel,
		*patternloopendless,
		*patternloop,
		*patternloopflag,
		*patternvelocity,
		*patterntranspose,
		*patternbankselect,
		*group,
		*group_mute,
		*group_solo,
		*group_rec,
		*processor,
		*patternonoff,
		*trackonoff;

	int indextrack,status_velocity,status_transpose,status_delay,status_patternloop,
		status_channel,status_tracktype,status_patternmediatype,status_loopflag;
	
	bool status_processorbypass,showactivepattern,status_group_mute,status_group_solo,status_group_rec,
		status_patternloopwithloops,status_patternloopendless,status_patternmute,
		status_noMIDIinput,status_useallinputdevices,showtrack,showpattern,
		status_MIDIthru;
};

#endif

#endif
