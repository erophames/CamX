#ifndef CAMX_AUDIOEFFECTPARAMETER
#define CAMX_AUDIOEFFECTPARAMETER 1

class AudioHardwareBuffer;
class AudioChannel;
class Seq_Track;
class AudioIOFX;
class Seq_Song;

class AudioEffectParameter
{
public:
	AudioEffectParameter()
	{
		clearbuffer=true;
		channel=0;
		track=0;
		realtime=itsmaster=false;

		playable=true;
	}

	AudioHardwareBuffer *startin,*in,*out;
	Seq_Song *song;
	AudioChannel *channel;
	Seq_Track *track;
	AudioIOFX *io;

#ifdef ARES64
	float *output32;
#endif

	LONGLONG stream_samplestartposition,stream_samplestartposition_end;
	int stream,streamchannels; // Song/Realtime
	bool clearbuffer,realtime,separateoutputs,effecterror,bypassfx,itsmaster,playable,panningdone;
};

#endif
