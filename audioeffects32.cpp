
#ifndef ARES64

#include "defines.h"

#include <cmath>                    // for trigonometry functions

#include "audiohardwarechannel.h"
#include "audiohardware.h"
#include "audiochannel.h"
#include "audioeffects.h"

#include "object_track.h"
#include "audiosystem.h"
#include "MIDIhardware.h"

#include "semapores.h"
#include "camxfile.h"

// Intern
#include "audioobject_equalizer.h"
#include "audioobject_delay.h"
#include "audioobject_allfilter.h"
#include "audioobject_chorus.h"
#include "audioobject_insertvolume.h"
#include "songmain.h"
//VST
#include "vstplugins.h"
#include "audiodevice.h"
#include "object_song.h"
#include "chunks.h"
#include "gui.h"

bool AudioEffects::AddEffects(AudioEffectParameter *par) //32 bit Audio
{
	bool fxadded=false;

	if(track)
	{
		if(track->song->mastertrack)
		return false;

		if(track->frozen==true)
			goto endloop; // No FX just Volume/Pan - Track Frozen
	}

	InsertAudioEffect *f=FirstActiveAudioEffect();

	while(f)
	{
		//f->audioeffect->SendTrigger(); // Instrument Plugin MIDI ---> Plugin

		AudioObject *fx=f->audioeffect;

		fx->Execute_TriggerEvents(); // Execute Plugin MIDI Events 

		par->out=fx->aobuffer; // Effect Output Buffer

		if(par->out && fx->GetSetOutputPins()>0)
		{
			par->effecterror=false;

			//bool mix=par->startin->channelsused?true:false;

			// Clear Unused
			if(fx->GetSetInputPins()>par->startin->channelsused)
			{
				ARES *c=par->startin->outputbufferARES;
				c+=par->startin->samplesinbuffer*par->startin->channelsused;

				memset(c,0,par->startin->samplesinbuffer_size*(fx->GetSetInputPins()-par->startin->channelsused));
			}

			par->in=par->startin;

			if(par->in->channelsused && fx->GetSetInputPins()>0 && fx->involume!=0.5)
			{
				// Add In Volume

				int i=par->in->samplesinbuffer*fx->GetSetInputPins();
				ARES mul=fx->involume,*data=par->in->outputbufferARES;

				mul*=LOGVOLUME_SIZE;
				mul=logvolume_f[(int)mul];

				if(int loop=i/8)
				{
					i-=8*loop;

					do
					{
						*data++ *=mul;
						*data++ *=mul;
						*data++ *=mul;
						*data++ *=mul;

						*data++ *=mul;
						*data++ *=mul;
						*data++ *=mul;
						*data++ *=mul;

					}while(--loop);
				}

				while(i--)*data++ *=mul; // + Add Volume
			}

			bool effectdone=fx->DoEffect(par); // Create Audio Buffer

			if(effectdone==true && (par->bypassfx==false || fx->IsInstrument()==true) && fx->plugin_bypass==false && par->effecterror==false)
			{
				fx->aobuffer->MulCon(fx->GetSetOutputPins()*fx->setSize,fx->outvolume); // Add OutVolume ?

				if(fx->GetSetOutputPins()>par->streamchannels) //0/8 in Stereo etc..
					MixEffectDownToStream(fx,par);

				if(par->in->channelsused==0 || fx->GetSetInputPins()>0)
				{
					// Replace Buffer
					par->in=par->startin=fx->aobuffer;
					fx->aobuffer->channelsused=par->streamchannels;
				}
				else
				{
					// Mix 32->32
					float *to=(float *)par->startin->outputbufferARES,*from=(float *)fx->aobuffer->outputbufferARES;
					int i=par->streamchannels*fx->setSize;

					// Mix
					if(int loop=i/8){

						i-=8*loop;

						do{
							*to++ +=*from++;
							*to++ +=*from++;
							*to++ +=*from++;
							*to++ +=*from++;

							*to++ +=*from++;
							*to++ +=*from++;
							*to++ +=*from++;
							*to++ +=*from++;

						}while(--loop);
					}

					while(i--)*to++ +=*from++;
				}
			}
			else 
				par->effecterror=false; // reset
		}

		f=f->NextActiveEffect();

	}// while effect

	// End Effects

endloop:
	SendEndEffects(par);

	return fxadded;
}
#endif
