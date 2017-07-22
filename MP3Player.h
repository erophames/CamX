#ifndef CAMX_MP3DECODER_H
#define CAMX_MP3DECODER_H 1

/* ----------------------------------------------------------------------
 * MP3Player.h C++ class using plain Windows API
 *
 * Author: @lx/Alexandre Mutel,  blog: http://code4k.blogspot.com
 * The software is provided "as is", without warranty of any kind.
 * ----------------------------------------------------------------------*/
// #pragma once

// #pragma comment(lib, "msacm32.lib") 
 //#pragma comment(lib, "wmvcore.lib") 
 //#pragma comment(lib, "winmm.lib") 

// #pragma intrinsic(memset,memcpy,memcmp)

#ifdef _DEBUG
//#define mp3Assert(function) assert((function) == 0)
#else
//#define mp3Assert(function) if ( (function) != 0 ) { MessageBoxA(NULL,"Error in [ " #function "]", "Error",MB_OK); ExitProcess(0); }
//#define mp3Assert(function) (function)
#endif

/*
 * MP3Player class.
 * Usage : 
 *   MP3Player player;
 *   player.OpenFromFile("your.mp3");
 *   player.Play();
 *   Sleep((DWORD)(player.GetDuration()+1));
 *   player.Close();
 */

class Decoder;

class MP3Player {
private:
	//HWAVEOUT hWaveOut;
	int bufferLength;
	double durationInSecond;
	
public:

	/*
	 * OpenFromFile : loads a MP3 file and convert it internaly to a PCM format, ready for sound playback.
	 */
	bool OpenFromFile(Decoder *decoder);

	/*
	 * OpenFromMemory : loads a MP3 from memory and convert it internaly to a PCM format, ready for sound playback.
	 */
	bool OpenFromMemory(char* mp3InputBuffer, int mp3InputBufferSize,Decoder *decoder);

	/*
	 * Close : close the current MP3Player, stop playback and free allocated memory
	 */
	void __inline Close() {
		// Reset before close (otherwise, waveOutClose will not work on playing buffer)
	//	waveOutReset(hWaveOut);
		// Close the waveOut
	//	waveOutClose(hWaveOut);
	
		// Free allocated memory
		// LocalFree(soundBuffer);
	}
	
	/*
	 * GetDuration : return the music duration in seconds
	 */
	double __inline GetDuration() {
		return durationInSecond;
	}

	/*
	 * GetPosition : return the current position from the sound playback (used from sync)
	 */
	/*
	double GetPosition() {
		static MMTIME MMTime = { TIME_SAMPLES, 0};
		waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
		return ((double)MMTime.u.sample)/( 44100.0);
	}
*/

	/*
	 * Play : play the previously opened mp3
	 */
#ifdef PLAY
	void Play() {
		static WAVEHDR WaveHDR = { (LPSTR)soundBuffer,  bufferLength };

		// Define output format
		static WAVEFORMATEX pcmFormat = {
			WAVE_FORMAT_PCM,	// WORD        wFormatTag;         /* format type */
			2,					// WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
			SRATE,				// DWORD       nSamplesPerSec;     /* sample rate */
			4 * SRATE,			// DWORD       nAvgBytesPerSec;    /* for buffer estimation */
			4,					// WORD        nBlockAlign;        /* block size of data */
			16,					// WORD        wBitsPerSample;     /* number of bits per sample of mono data */
			0,					// WORD        cbSize;             /* the count in bytes of the size of */
		};

		mp3Assert( waveOutOpen( &hWaveOut, WAVE_MAPPER, &pcmFormat, NULL, 0, CALLBACK_NULL ) );
		mp3Assert( waveOutPrepareHeader( hWaveOut, &WaveHDR, sizeof(WaveHDR) ) );
		mp3Assert( waveOutWrite		( hWaveOut, &WaveHDR, sizeof(WaveHDR) ) );
	}
#endif
};

// #pragma function(memset,memcpy,memcmp)

#endif