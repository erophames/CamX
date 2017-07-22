#ifndef CAMX_TRACKEFFECT_H
#define CAMX_TRACKEFFECT_H 1

#include "quantizeeffect.h"
#include "MIDIfilter.h"
#include "MIDIoutdevice.h"
#include "MIDIeffects.h"
#include "systemautomation.h"

class Seq_Track;
class camxFile;

enum 
{
	DELAYTYPE_SAMPLES,
	DELAYTYPE_MS
};

class TrackEffects
{
	friend class Edit_ArrangeEditorEffects;

public:
	TrackEffects();

	bool CompareInput(TrackEffects *);
	bool CompareOutput(TrackEffects *);

	void Clone(TrackEffects *);
	void Load(camxFile *);
	void Save(camxFile *);

	int GetTranspose();
	int GetTranspose_NoParent();
	void SetTranspose(int);

	int GetChannel();
	int GetChannel_NoParent();
	void SetChannel(int);

	int GetVelocity();
	int GetVelocity_NoParent();
	void SetVelocity(int,OSTART automationtime);

	void SetDelay(OSTART);
	OSTART GetDelay();

	bool IsMIDIInOrOutFilter();

	MIDIFilter inputfilter,filter; // output

	AT_MIDISYS_Channel t_MIDI_channel;
	AT_MIDISYS_Transpose t_MIDI_transpose;

	MIDIOutputProgram MIDIprogram;
	QuantizeEffect quantizeeffect;

	Seq_Track *track;
	OSTART ndelay /*,totaldelay*/;
	int delaytype;
	bool noMIDIinput,useallinputdevices,usealwaysthru,userouting,setalwaysthruautomatic,MIDIthru;
};
#endif