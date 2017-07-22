#ifndef CAMX_AUDIODEFINES
#define CAMX_AUDIODEFINES 1

#define AUDIOCAMX_NONE 0

#ifdef WIN32
#define AUDIOCAMX_ASIO 5
#define AUDIOCAMX_DIRECTS 6
#define AUDIOCAMX_WIN32 7
#endif

// RANGE Audiomixer Slider volume
#define AUDIO_MAXDB 12
#define AUDIOMIXER_ADD 240 // -0.05
#define AUDIOMIXER_SUB 240 //- 40 db (0.1)
#define LOGVOLUME_SIZE (AUDIOMIXER_ADD+AUDIOMIXER_SUB) // + silence and null


enum Type{
	TYPE_UNKNOWN,
	TYPE_WAV,
	TYPE_AIFF, // big endian
	TYPE_AIFFC, // little endian
};


#endif
