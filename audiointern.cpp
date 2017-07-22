#include "defines.h"

#include <stdio.h>
#include <windows.h>

#include "audiohardware.h"
#include "audiodevice.h"
#include "object_song.h"
#include "songmain.h"
#include "audiohdfile.h"

// includes of intern fx's
#include "audiofx_reverb.h"
#include "audioobject_equalizer.h"
#include "audioobject_delay.h"
#include "audioobject_allfilter.h"
#include "audioobject_chorus.h"
#include "audioobject_insertvolume.h"

void mainAudio::AddInternEffect(audioobject_Intern *ifx)
{
	if(ifx)
	{
		interneffects.AddEndO(ifx);
	}
}

void mainAudio::CollectInternEffects()
{
	//Reverb
	//	AddInternEffect(new audioobject_Intern_Reverb);

	//Equalizer
	AddInternEffect(new audioobject_Intern_Equalizer3);
	//AddInternEffect(new audioobject_Intern_Equalizer3_Stereo);

	AddInternEffect(new audioobject_Intern_Equalizer5);
	//	AddInternEffect(new audioobject_Intern_Equalizer5_Stereo);

	AddInternEffect(new audioobject_Intern_Equalizer7);
	//AddInternEffect(new audioobject_Intern_Equalizer7_Stereo);

	AddInternEffect(new audioobject_Intern_Equalizer10);
	//AddInternEffect(new audioobject_Intern_Equalizer10_Stereo);


	//Delay

	AddInternEffect(new audioobject_Intern_Delay);
	//	AddInternEffect(new audioobject_Intern_Delay_Stereo);


	// Chorus
	AddInternEffect(new audioobject_Intern_Chorus);
	//	AddInternEffect(new audioobject_Intern_Chorus_Stereo);


	// Filte
	AddInternEffect(new audioobject_Intern_Allpass);
	// AddInternEffect(new audioobject_Intern_Allpass_Stereo);

	AddInternEffect(new audioobject_Intern_IVolume);
	//AddInternEffect(new audioobject_Intern_IVolume_Stereo);
}

void mainAudio::CloseAllInternEffects()
{
	audioobject_Intern *ifx=FirstInternEffect();
	
	while(ifx)
	{
		ifx->Close(true);
		ifx=(audioobject_Intern *)interneffects.RemoveO(ifx);
	}
}

