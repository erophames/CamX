#ifndef CAMX_ARRANGEEDITORFX_H
#define CAMX_ARRANGEEDITORFX_H 1

#include "editor.h"
#include "trackfx.h"
#include "audiomixeditor.h"

class Edit_Arrange;

class Edit_ArrangeFX_Metronom:public guiWindow
{
	friend class Edit_ArrangeFX;

	Edit_ArrangeFX_Metronom()
	{
		isstatic=true;
		editorid=EDITORTYPE_ARRANGEFXMETRO;
		InitForms(FORM_PLAIN1x1);
	}

	void Init();
	int GetInitHeight();
	void Gadget(guiGadget *g);
	void RefreshRealtime_Slow();

	void ShowMetro();
	void SendMetro(int hi);

	Edit_ArrangeFX *editor;
	guiGadget_Number *mchl_b,*mkey_b,*mvelo_b,*mvelooff_b;
	guiGadget_Number *mchl_m,*mkey_m,*mvelo_m,*mvelooff_m;
	guiGadget *mport,*mhport,*maudioport,*maudiotype,*sendtoaudio,*sendtoMIDI;
};

class Edit_ArrangeFX_MIDI:public guiWindow
{
	friend class Edit_ArrangeFX;

	Edit_ArrangeFX_MIDI()
	{
		isstatic=true;
		editorid=EDITORTYPE_ARRANGEFXMIDI;
		InitForms(FORM_PLAIN1x1);
	}

	void Init();
	int GetInitHeight();
	void Gadget(guiGadget *g);
	void RefreshRealtime_Slow();

	void ShowTrackMIDIChannel(bool force);
	void ShowTrackMIDIInput(bool force);
	void ShowTrackMIDIOutput(bool force);
	void ShowTrackMIDIThru(bool force);

	void ShowTrackMIDIDelayType(bool force);
	void ShowTrackMIDIDelay(bool force);

	void ShowOutFilter(bool force);
	void ShowInFilter(bool force);

	void ShowTrackTranspose(bool force);
	void ShowTrackVelocity(bool force);

	void ShowTrackProgramAndBankSelect(bool force);

	void Refresh(bool);

	MIDIOutputProgram compareMIDIprogram;
	MIDIFilter compareoutfilter,compareinfilter;

	Edit_ArrangeFX *editor;
	guiGadget *g_outby,*g_outstring,*g_inby,*g_instring,
		*trackMIDIinput,*trackMIDIoutput,*trackMIDIthru,*trackdelaytype,*trackbsel_i_channel,*trackbsel_i_msb,*trackpsel_i,*trackbsel_i_lsb;

	guiGadget_Number *trackchannel,*tracktranspose,*trackvelocity,*trackdelay,*trackbsel_channel,*trackbsel_msb,*trackbsel_lsb,*trackprgsel;

	OSTART trackdelay_nr;
	int trackchannel_nr,tracktranspose_nr,trackvelocity_nr,delaytype;
	bool MIDIthru,outbypass,inbypass;
};

class Edit_ArrangeFX_Audio:public guiWindow
{
	friend class GUI;
	friend class Edit_ArrangeFX;

	Edit_ArrangeFX_Audio()
	{
		isstatic=true;
		editorid=EDITORTYPE_ARRANGEFXAUDIO;
		InitForms(FORM_PLAIN1x1);
	}

	void Init();
	int GetInitHeight();
	void Gadget(guiGadget *);
	void RefreshRealtime_Slow();

	void RefreshAll()
	{
		ShowAudioInput(true);
		ShowAudioOutput(true);
	}

	void ShowAudioIO(bool force);
	void ShowAudioInput(bool force);
	void ShowAudioOutput(bool force);
	void ShowAudioThru(bool force);
	void ShowAudioInputMonitoring(bool force);
	void ShowAudioIsInput(bool force);
	void Refresh(bool);

	Edit_ArrangeFX *editor;
	guiGadget *g_iostring,*g_instring,*g_outstring,*g_audiothru,*g_audioinputmonitoring,*g_audiooutsetfree;
	int channel_type;
	bool audiothru,inputmonitoring,isinput;
};

class Edit_ArrangeFX_Quantize:public guiWindow
{
	friend class Edit_ArrangeFX;

	Edit_ArrangeFX_Quantize()
	{
		isstatic=true;
		editorid=EDITORTYPE_ARRANGEFXQUANT;
		InitForms(FORM_PLAIN1x1);
	}

	void Init();
	int GetInitHeight();
	void Gadget(guiGadget *g);
	void RefreshRealtime_Slow();
	void ShowAll();

	void CreateQuantizePopUp(guiGadget *g,int flag);

	void InitEffect(bool copy);
	void ShowFlag();
	void ShowBox();
	void ShowSelectQuant();

	QuantizeEffect compareeffect,*effect;

	Edit_ArrangeFX *editor;
	guiGadget_Number *humanizevalue;
	guiGadget *boxquantize,*flag,*selectquant,*boxnoteoffquant,*boxsetnotelength,*notelength,*humanize,*humamizeq;
};

class Edit_ArrangeFX_Mixer:public Edit_AudioMix
{
	friend class Edit_ArrangeFX;

	Edit_ArrangeFX_Mixer();
	int GetInitHeight();

	Edit_ArrangeFX *editor;
};

class Edit_ArrangeFX_Pattern:public guiWindow
{
public:
	friend class Edit_ArrangeFX;

	Edit_ArrangeFX_Pattern()
	{
		editorid=EDITORTYPE_ARRANGEFXPATTERN;

		InitForms(FORM_PLAIN1x1);
		isstatic=true;
	}

	void Init();
	void Gadget(guiGadget *);
	void RefreshAll();
	void RefreshRealtime_Slow();
	void ShowLoopMode(Seq_Pattern *);
	void ChangeLoopFlag(int flag);

	int GetInitHeight();

	Edit_ArrangeFX *editor;
	guiGadget *g_patternstring,*g_mute,*g_loop,*g_loopendless,*g_loopmode,*qeditor,
		*g_MIDIchannel,*patterntranspose,*patternvelocity,*onoff,*onoffvolume;

	guiGadget_Number *g_loopnr,*g_MIDIchannelnr,*patterntransposenr,*patternvelocitynr;

	double dbvolume,fadeinms,fadeoutms;

	int loopnr,loopflag;
	bool loopwithloops,loopendless,mute;

	// MIDI
	int MIDIchannel,MIDIvelocity,MIDItranspose;

private:
	void DisableEventChannelTransVelo();

};

class Edit_ArrangeFX:public guiWindow
{
public:
	Edit_ArrangeFX(Edit_Arrange *);

	void RefreshRealtime_Slow();
	void InitGadgets();
	void Init();
	Seq_Track *GetTrack();

	void RefreshMIDI(Seq_Track *);
	void Gadget(guiGadget *);
	void ShowTrackNumber(bool force);
	void ShowTrackName(bool force);
	void ShowRecordType(bool force);
	void ShowTrackEventType(bool force);

	Edit_ArrangeFX_MIDI trackMIDI;
	Edit_ArrangeFX_Metronom trackmetro;
	Edit_ArrangeFX_Audio trackaudio;
	Edit_ArrangeFX_Quantize trackquant;
	Edit_ArrangeFX_Mixer mixer;
	Edit_ArrangeFX_Pattern pattern;

	Edit_Arrange *arrangeeditor;

	guiGadget *g_track,*g_trackstring,*gr_MIDI,*g_trackrecord,*g_tracktype,*gr_quant,*gr_audio,*gr_metro,*gr_pattern,*gr_mixer;

	int tracknr,recordtracktype,MIDItype;
	bool MIDItypesetauto;
};

#endif