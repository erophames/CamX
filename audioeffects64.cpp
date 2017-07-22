#ifdef ARES64

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

//64Bit->32
void AudioHardwareBuffer::BridgeIn(float *outputbuffer32,int chls,double multi)
{
	multi*=LOGVOLUME_SIZE;

#ifdef ARES64
	multi=logvolume[(int)multi];
#else
	multi=logvolume_f[(int)multi];
#endif

	if(outputbuffer32)
	{
		//64 bit float -> 32bit

		if(channelsused==0)
		{
			// Clear 32 Buffer
			memset(outputbuffer32,0,samplesinbuffer*chls*sizeof(float));
		}
		else
		{
			int i=channelsused*samplesinbuffer;

			// Mix 32+64->32
			float *to=outputbuffer32;
			double *from=(double *)outputbufferARES;

			if(multi==1)
			{
				if(int loop=i/8)
				{
					i-=loop*8;

					do
					{
						*to++=(float)*from++;
						*to++=(float)*from++;
						*to++=(float)*from++;
						*to++=(float)*from++;

						*to++=(float)*from++;
						*to++=(float)*from++;
						*to++=(float)*from++;
						*to++=(float)*from++;

					}while(--loop);
				}

				while(i--)*to++=(float)*from++;
			}
			else
			{
				if(int loop=i/8)
				{
					i-=loop*8;

					do
					{
						*to++=(float)(*from++*multi);
						*to++=(float)(*from++*multi);
						*to++=(float)(*from++*multi);
						*to++=(float)(*from++*multi);

						*to++=(float)(*from++*multi);
						*to++=(float)(*from++*multi);
						*to++=(float)(*from++*multi);
						*to++=(float)(*from++*multi);

					}while(--loop);
				}

				while(i--)*to++=(float)(*from++*multi);
			}
		}
	}
}

void AudioHardwareBuffer::BridgeOut32To64Mix(float *from,int channels,ARES multi)
{
	multi*=LOGVOLUME_SIZE;
#ifdef ARES64
	multi=logvolume[(int)multi];
#else
	multi=logvolume_f[(int)multi];
#endif

	// 32->64
	double *to=(double *)outputbufferARES;
	int i=channels*samplesinbuffer;

	if(multi!=1)
	{
		// Mix 32-64

		if(int loop=i/8)
		{
			i-=8*loop;

			do
			{
				*to++ =(*to+(double)*from++) *multi;
				*to++ =(*to+(double)*from++) *multi;
				*to++ =(*to+(double)*from++) *multi;
				*to++ =(*to+(double)*from++) *multi;

				*to++ =(*to+(double)*from++) *multi;
				*to++ =(*to+(double)*from++) *multi;
				*to++ =(*to+(double)*from++) *multi;
				*to++ =(*to+(double)*from++) *multi;

			}while(--loop);
		}

		while(i--)*to++ =(*to+(double)*from++) *multi;
	}
	else
	{
		if(int loop=i/8)
		{
			i-=8*loop;

			do
			{
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;

				*to++ +=(double)*from++;
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;

			}while(--loop);
		}

		while(i--)*to++ +=(double)*from++;
	}

	if(channelsused<channels)
		channelsused=channels;
}

void AudioHardwareBuffer::BridgeOut32To64Mix(float *from,int channels)
{
	// 32->64
	double *to=(double *)outputbufferARES;
	int i=channels*samplesinbuffer;

	if(!channelsused)
	{
		// Copy 32->64
		if(int loop=i/8)
		{
			i-=8*loop;

			do
			{
				*to++ =(double)*from++;
				*to++ =(double)*from++;
				*to++ =(double)*from++;
				*to++ =(double)*from++;

				*to++ =(double)*from++;
				*to++ =(double)*from++;
				*to++ =(double)*from++;
				*to++ =(double)*from++;

			}while(--loop);
		}

		while(i--)*to++ =(double)*from++;

	}
	else
	{
		// Mix 32-64
		if(int loop=i/8)
		{
			i-=8*loop;

			do
			{
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;

				*to++ +=(double)*from++;
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;
				*to++ +=(double)*from++;

			}while(--loop);
		}

		while(i--)*to++ +=(double)*from++;

	}

	if(channelsused<channels)
		channelsused=channels;
}

void AudioHardwareBuffer::BridgeOut32To64(float *from,int channels)
{
	// 32->64
	int i=channels*samplesinbuffer;
	double *to=(double *)outputbufferARES;

	if(int loop=i/8)
	{
		i-=8*loop;

		do
		{
			*to++=(double)*from++;
			*to++=(double)*from++;
			*to++=(double)*from++;
			*to++=(double)*from++;

			*to++=(double)*from++;
			*to++=(double)*from++;
			*to++=(double)*from++;
			*to++=(double)*from++;

		}while(--loop);
	}

	while(i--)*to++=(double)*from++;

	if(channelsused<channels)
		channelsused=channels;
}

void AudioHardwareBuffer::BridgeOut32To64(float *from,int channels,ARES multi)
{
	multi*=LOGVOLUME_SIZE;
#ifdef ARES64
	multi=logvolume[(int)multi];
#else
	multi=logvolume_f[(int)multi];
#endif

	// 32->64
	int i=channels*samplesinbuffer;
	double *to=(double *)outputbufferARES;

	if(multi!=1)
	{
		if(int loop=i/8)
		{
			i-=8*loop;

			do
			{
				*to++=(double)*from++*multi;
				*to++=(double)*from++*multi;
				*to++=(double)*from++*multi;
				*to++=(double)*from++*multi;


				*to++=(double)*from++*multi;
				*to++=(double)*from++*multi;
				*to++=(double)*from++*multi;
				*to++=(double)*from++*multi;

			}while(--loop);
		}

		while(i--)*to++=(double)*from++*multi;
	}
	else
	{
		if(int loop=i/8)
		{
			i-=8*loop;

			do
			{
				*to++=(double)*from++;
				*to++=(double)*from++;
				*to++=(double)*from++;
				*to++=(double)*from++;

				*to++=(double)*from++;
				*to++=(double)*from++;
				*to++=(double)*from++;
				*to++=(double)*from++;

			}while(--loop);
		}

		while(i--)*to++=(double)*from++;
	}

	if(channelsused<channels)
		channelsused=channels;
}

// 64 Bit AudioEngine + 32 Bridge
bool AudioEffects::AddEffects(AudioEffectParameter *par) // 64bit Audio
{
	bool fxadded=false;
	bool mode64=true; // incoming 64bit
	bool mode32=false;

	float *activeout32=par->output32=output32;
	int out32channelsused=0;

	AudioHardwareBuffer *startbuffer=par->startin,*activebuffer=startbuffer;

	if(track)
	{
		if(track->song->mastertrack)
			return false;

		if(track->frozen==true)
			goto endloop; // No FX just Volume/Pan - Track Frozen
	}

	if(InsertAudioEffect *f=FirstActiveAudioEffect())
	{
#ifdef DEBUG
		if(!output32)
			maingui->MessageBoxError(0,"Output32");
#endif

		do{
			AudioObject *fx=f->audioeffect;
			bool fxbypass;
			
			if((par->bypassfx==false || fx->IsInstrument()==true) && fx->plugin_bypass==false)
				fxbypass=false;
			else
				fxbypass=true;

			fx->Execute_TriggerEvents(); // Execute Plugin MIDI Events 

			InsertAudioEffect *nextnon=f->NextActiveEffect();

			par->in=activebuffer; // Fx ARES Input Buffer

			if(par->out=fx->aobuffer)
			{
				par->effecterror=false;

				if(fx->GetSetInputPins()>0) // Plugin with Input ?
				{
					switch(fx->floattype)
					{
					case AudioObject::FT_32BIT:
						{
							if(mode32==true && mode64==true)
							{
								// 64+32->32

								if(activebuffer->channelsused) // Mix 64 ?
								{
									ARES *from=activebuffer->outputbufferARES;
									float *to=activeout32;
									int i=activebuffer->channelsused*activebuffer->samplesinbuffer;

									ARES mul=fx->involume;
									mul*=LOGVOLUME_SIZE;
#ifdef ARES64
									mul=logvolume[(int)mul];
#else
									mul=logvolume_f[(int)mul];
#endif
									if(mul==1)
									{
										if(int loop=i/8)
										{
											i-=8*loop;

											do
											{
												*to++ +=(float)*from++;
												*to++ +=(float)*from++;
												*to++ +=(float)*from++;
												*to++ +=(float)*from++;

												*to++ +=(float)*from++;
												*to++ +=(float)*from++;
												*to++ +=(float)*from++;
												*to++ +=(float)*from++;

											}while(--loop);
										}

										while(i--)*to++ +=(float)*from++;
									}
									else
									{
										if(int loop=i/8)
										{
											i-=8*loop;

											do{
												*to++ +=(float)(*from++ *mul);
												*to++ +=(float)(*from++ *mul);
												*to++ +=(float)(*from++ *mul);
												*to++ +=(float)(*from++ *mul);

												*to++ +=(float)(*from++ *mul);
												*to++ +=(float)(*from++ *mul);
												*to++ +=(float)(*from++ *mul);
												*to++ +=(float)(*from++ *mul);

											}while(--loop);
										}

										while(i--)*to++ +=(float)(*from++ *mul);

									}

								}//activebuffer->channelsused

								// else use 32bit 

								// 
								/*
								else
								{
								if(
								activebuffer->BridgeIn(activeout32,par->streamchannels,fx->involume);
								}
								*/
							}
							else
								if(mode32==false)
								{
									//64Bit->32Bit
									activebuffer->BridgeIn(activeout32,par->streamchannels,fx->involume);
								}

								out32channelsused=par->streamchannels;

								if(fxbypass==false)
								mode32=true;
						}
						break;

					case AudioObject::FT_64BIT:
						{
							if(mode64==false || (mode64==true && mode32==true))
							{
								// 32->64
								if(activebuffer->channelsused)
									activebuffer->BridgeOut32To64Mix(activeout32,out32channelsused,fx->involume); // 32+64 * invol ->64
								else
									activebuffer->BridgeOut32To64(activeout32,out32channelsused,fx->involume); //32->64 * invol ->64

								if(fxbypass==false)
								{
								mode64=true;
								mode32=false;
								}
							}
							else
							{
								// Clear Unused
								if(activebuffer->channelsused==0)
								{
									memset(activebuffer->outputbufferARES,0,activebuffer->samplesinbuffer_size*fx->GetSetInputPins()); // set to 0
								}
								else
								{
									if(fx->involume!=0.5)
										activebuffer->MulCon(activebuffer->samplesinbuffer*fx->GetSetInputPins(),fx->involume);
								}
							}

						}
						break;
					}
				}

			}//if(par->out && fx->GetSetOutputPins()>0)

			bool effectdone=fx->DoEffect(par); // Create Audio Buffer

			// Output .....................

			if(par->effecterror==false && effectdone==true)
			{
				if(fxbypass==false)
				{
					switch(fx->floattype)
					{
					case AudioObject::FT_64BIT:
						{
							fx->aobuffer->MulCon(fx->GetSetOutputPins()*fx->setSize,fx->outvolume); // Add OutVolume ?  
						}
						break;

					case AudioObject::FT_32BIT:
						{
							fx->aobuffer->MulCon32((float *)fx->aobuffer->outputbufferARES,fx->GetSetOutputPins()*fx->setSize,fx->outvolume); // Add OutVolume ?  
						}
						break;
					}

					if(fx->GetSetOutputPins()>par->streamchannels)
						MixEffectDownToStream(fx,par); // 0/8 -> 0/2 etc...

					par->out->channelsused=fx->aobuffer->channelsused=par->streamchannels;

					if(fx->GetSetInputPins()>0) // Plugin with Input 2/2 etc.
					{
						// Input Replace Stream
						switch(fx->floattype)
						{
						case AudioObject::FT_64BIT:
							{
								mode64=true;
								mode32=false;

								activebuffer=fx->aobuffer;
							}
							break;

						case AudioObject::FT_32BIT:
							{
								out32channelsused=par->streamchannels;

								par->output32=activeout32=(float *)fx->aobuffer->outputbufferARES;

								if(!nextnon)
									activebuffer->BridgeOut32To64(activeout32,out32channelsused);

								mode32=true;
								mode64=false;
							}
							break;
						}
					}
					else // 0 Inputs - Mix to Stream - AudioInstruments 0/2 etc...
					{
						switch(fx->floattype)
						{
						case AudioObject::FT_64BIT:
							{
								if(mode64==true)
								{
									if(activebuffer->channelsused)
									{
										// Mix 64+64
										double *to=activebuffer->outputbufferARES,*from=(double *)fx->aobuffer->outputbufferARES;
										int i=par->streamchannels*fx->aobuffer->samplesinbuffer;

										if(int loop=i/8)
										{
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
									else
										activebuffer=fx->aobuffer;
								}//if mode 64

								if(mode32==true) // Mix 32+64
								{
									if(!nextnon)
										activebuffer->BridgeOut32To64Mix(activeout32,out32channelsused);
								}

								mode64=true;
							}
							break; // AudioObject::FT_64BIT:

						case AudioObject::FT_32BIT:
							{
								if(mode32==true) // 32-32 ?
								{
									if(out32channelsused)
									{
										// Mix 32->32
										float *to=activeout32,*from=(float *)fx->aobuffer->outputbufferARES;
										int i=par->streamchannels*fx->aobuffer->samplesinbuffer;

										// Mix 32-32
										if(int loop=i/8)
										{
											i-=8*loop;

											do
											{
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
									}// if out32channelsused
									else
									{
										out32channelsused=par->streamchannels;
										par->output32=activeout32=(float *)fx->aobuffer->outputbufferARES;
									}

								}//mode32==32
								else
								{
									out32channelsused=par->streamchannels;
									par->output32=activeout32=(float *)fx->aobuffer->outputbufferARES;
								}

								if(!nextnon)
									activebuffer->BridgeOut32To64Mix(activeout32,out32channelsused);

								mode32=true; //keep 64
							}
							break;

						}// switch

					}// 0 Inputs - Mix to Stream - AudioInstruments 0/2 etc...

					if(!nextnon)
						goto endloop;

				}// if bypass
				else
				{
					// FX Bypass
					par->effecterror=false; // reset

					if((!nextnon) && mode32==true)
						activebuffer->BridgeOut32To64Mix(activeout32,out32channelsused);
				}

			}//if(par->effecterror==false && effectdone==true)

			f=nextnon;

		}while(f);

	}// if f

endloop:

	// Reset
	//par->startin=par->in=startbuffer;

	par->startin=par->in=activebuffer; // Keep FX Buffer as Stream Buffer

	SendEndEffects(par);

	return fxadded;
}
#endif
