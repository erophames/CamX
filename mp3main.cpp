#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include <mmreg.h>
#include <msacm.h>
#include <wmsdk.h>

#include "MP3Player.h"
#include "decoder.h"


bool MP3Player::OpenFromMemory(char* mp3InputBuffer, int mp3InputBufferSize,Decoder *decoder){

	IWMSyncReader* wmSyncReader;
	IWMHeaderInfo* wmHeaderInfo;
	IWMProfile* wmProfile;
	IWMStreamConfig* wmStreamConfig;
	IWMMediaProps* wmMediaProperties;
	WORD wmStreamNum = 0;
	WMT_ATTR_DATATYPE wmAttrDataType;
	DWORD durationInSecondInt;
	QWORD durationInNano;
	DWORD sizeMediaType;
	DWORD maxFormatSize = 0;
	HACMSTREAM acmMp3stream = NULL;
	HGLOBAL mp3HGlobal;
	IStream* mp3Stream;

#ifdef doof
	// Define output format
	WAVEFORMATEX pcmFormat = {
		WAVE_FORMAT_PCM,	// WORD        wFormatTag;         /* format type */
		2,					// WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
		44100,				// DWORD       nSamplesPerSec;     /* sample rate */
		4 * 44100,			// DWORD       nAvgBytesPerSec;    /* for buffer estimation */
		4,					// WORD        nBlockAlign;        /* block size of data */
		16,					// WORD        wBitsPerSample;     /* number of bits per sample of mono data */
		0,					// WORD        cbSize;             /* the count in bytes of the size of */
	};
#endif

	const DWORD MP3_BLOCK_SIZE = 522;

	// -----------------------------------------------------------------------------------
	// Extract and verify mp3 info : duration, type = mp3, sampleRate = 44100, channels = 2
	// -----------------------------------------------------------------------------------

	// Initialize COM
	CoInitialize(0);

	// Create SyncReader
	HRESULT res=WMCreateSyncReader(  NULL, WMT_RIGHT_PLAYBACK , &wmSyncReader );

	// Alloc With global and create IStream
	mp3HGlobal = GlobalAlloc(GPTR, mp3InputBufferSize);

	if(!mp3HGlobal)return false;

	void* mp3HGlobalBuffer = GlobalLock(mp3HGlobal);

	memcpy(mp3HGlobalBuffer, mp3InputBuffer, mp3InputBufferSize);
	GlobalUnlock(mp3HGlobal);
	res=CreateStreamOnHGlobal(mp3HGlobal, FALSE, &mp3Stream);

	// Open MP3 Stream
	res=wmSyncReader->OpenStream(mp3Stream);

	// Get HeaderInfo interface
	res=wmSyncReader->QueryInterface(&wmHeaderInfo);

	// Retrieve mp3 song duration in seconds
	WORD lengthDataType = sizeof(QWORD);
	res=wmHeaderInfo->GetAttributeByName(&wmStreamNum, L"Duration", &wmAttrDataType, (BYTE*)&durationInNano, &lengthDataType );
	durationInSecond = ((double)durationInNano)/10000000.0;
	durationInSecondInt = (int)(durationInNano/10000000)+1;

	// Sequence of call to get the MediaType
	// WAVEFORMATEX for mp3 can then be extract from MediaType
	res=wmSyncReader->QueryInterface(&wmProfile);
	if(res!=S_OK)
	{
		// Release allocated memory
		mp3Stream->Release();
		GlobalFree(mp3HGlobal);

		return false;
	}

	res=wmProfile->GetStream(0, &wmStreamConfig);
	if(res!=S_OK)
	{
		// Release allocated memory
		mp3Stream->Release();
		GlobalFree(mp3HGlobal);
		return false;
	}

	res=wmStreamConfig->QueryInterface(&wmMediaProperties);
	if(res!=S_OK)
	{
		// Release allocated memory
		mp3Stream->Release();
		GlobalFree(mp3HGlobal);
		return false;
	}

	// Retrieve sizeof MediaType
	res=wmMediaProperties->GetMediaType(NULL, &sizeMediaType);

	// Retrieve MediaType
	WM_MEDIA_TYPE* mediaType = (WM_MEDIA_TYPE*)LocalAlloc(LPTR,sizeMediaType);	
	res=wmMediaProperties->GetMediaType(mediaType, &sizeMediaType);

	// Check that MediaType is audio
	if(mediaType->majortype != WMMEDIATYPE_Audio)
	{
		// Release allocated memory
		mp3Stream->Release();
		GlobalFree(mp3HGlobal);

		return false;
	}

	// assert(mediaType->pbFormat == WMFORMAT_WaveFormatEx);

	// Check that input is mp3
	WAVEFORMATEX* inputFormat = (WAVEFORMATEX*)mediaType->pbFormat;

	if(inputFormat->wFormatTag == WAVE_FORMAT_MPEGLAYER3 &&
		inputFormat->nChannels>=1
		//	inputFormat->nSamplesPerSec == 44100 &&
		//inputFormat->nChannels == 2)
		)
	{
		MPEGLAYER3WAVEFORMAT mp3Format = {
			{
				WAVE_FORMAT_MPEGLAYER3,			// WORD        wFormatTag;         /* format type */
					inputFormat->nChannels,								// WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
					inputFormat->nSamplesPerSec,							// DWORD       nSamplesPerSec;     /* sample rate */
					inputFormat->nAvgBytesPerSec,				// DWORD       nAvgBytesPerSec;    not really used but must be one of 64, 96, 112, 128, 160kbps
					inputFormat->nBlockAlign,								// WORD        nBlockAlign;        /* block size of data */
					inputFormat->wBitsPerSample,								// WORD        wBitsPerSample;     /* number of bits per sample of mono data */
					MPEGLAYER3_WFX_EXTRA_BYTES,		// WORD        cbSize;        
			},
			MPEGLAYER3_ID_MPEG,						// WORD          wID;
			MPEGLAYER3_FLAG_PADDING_OFF,			// DWORD         fdwFlags;
			MP3_BLOCK_SIZE,							// WORD          nBlockSize;
			1,										// WORD          nFramesPerBlock;
			1393,									// WORD          nCodecDelay;
		};


		// Define output format
		WAVEFORMATEX pcmFormat = {
			WAVE_FORMAT_PCM,	// WORD        wFormatTag;         /* format type */
			inputFormat->nChannels,					// WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
			inputFormat->nSamplesPerSec,				// DWORD       nSamplesPerSec;     /* sample rate */
			4 * inputFormat->nSamplesPerSec,			// DWORD       nAvgBytesPerSec;    /* for buffer estimation */
			4,					// WORD        nBlockAlign;        /* block size of data */
			16,					// WORD        wBitsPerSample;     /* number of bits per sample of mono data */
			0,					// WORD        cbSize;             /* the count in bytes of the size of */
		};

		// Release COM interface
		// wmSyncReader->Close();
		wmMediaProperties->Release();
		wmStreamConfig->Release();
		wmProfile->Release();
		wmHeaderInfo->Release();
		wmSyncReader->Release();

		// Free allocated mem
		LocalFree(mediaType);

		// -----------------------------------------------------------------------------------
		// Convert mp3 to pcm using acm driver
		// The following code is mainly inspired from http://david.weekly.org/code/mp3acm.html
		// -----------------------------------------------------------------------------------

		// Get maximum FormatSize for all acm
		res=acmMetrics( NULL, ACM_METRIC_MAX_SIZE_FORMAT, &maxFormatSize );

		// Allocate PCM output sound buffer
		bufferLength = durationInSecond * pcmFormat.nAvgBytesPerSec;

		acmMp3stream = NULL;

		MMRESULT res=
			acmStreamOpen( &acmMp3stream,				// Open an ACM conversion stream
			NULL,                       // Query all ACM drivers
			(LPWAVEFORMATEX)&mp3Format, // input format :  mp3
			&pcmFormat,                 // output format : pcm
			NULL,                       // No filters
			0,                          // No async callback
			0,                          // No data for callback
			0                           // No flags 
			);

		switch(res)
		{
		case MMSYSERR_NOERROR:
			break; // success!
		case MMSYSERR_INVALPARAM:
			//  assert( !"Invalid parameters passed to acmStreamOpen" );
			return false;
		case ACMERR_NOTPOSSIBLE:
			//assert( !"No ACM filter found capable of decoding MP3" );
			return false;
		default:
			//assert( !"Some error opening ACM decoding stream!" );
			return false;
		}


		//decoder->decodeddata=soundBuffer = (char*)new char[bufferLength];

		decoder->channels=pcmFormat.nChannels;
		decoder->samplerate=pcmFormat.nSamplesPerSec;
		decoder->samplebits=16;

		// Determine output decompressed buffer size
		unsigned long rawbufsize = 0;
		MMRESULT mres=acmStreamSize( acmMp3stream, MP3_BLOCK_SIZE, &rawbufsize, ACM_STREAMSIZEF_SOURCE );

		if(rawbufsize > 0)
		{
			// allocate our I/O buffers
			BYTE mp3BlockBuffer[MP3_BLOCK_SIZE];
			//LPBYTE mp3BlockBuffer = (LPBYTE) LocalAlloc( LPTR, MP3_BLOCK_SIZE );
			char *rawbuf = new char[rawbufsize];

			if(rawbuf)
			{
				// pRepare the decoder
				ACMSTREAMHEADER mp3streamHead;

				memset( &mp3streamHead, 0, sizeof(ACMSTREAMHEADER ) );

				mp3streamHead.cbStruct = sizeof(ACMSTREAMHEADER );
				mp3streamHead.pbSrc = mp3BlockBuffer;
				mp3streamHead.cbSrcLength = MP3_BLOCK_SIZE;
				mp3streamHead.pbDst = (LPBYTE)rawbuf;
				mp3streamHead.cbDstLength = rawbufsize;

				mres=acmStreamPrepareHeader( acmMp3stream, &mp3streamHead, 0 );

				DWORD totalDecompressedSize = 0;

				ULARGE_INTEGER newPosition;
				LARGE_INTEGER seekValue;

				seekValue.QuadPart=0;

				res=mp3Stream->Seek(seekValue, STREAM_SEEK_SET, &newPosition);

				bool firstdata=false,ok=true;

				while(ok==true && decoder->stop==false) {
					// suck in some MP3 data
					ULONG count;
					res=mp3Stream->Read(mp3BlockBuffer, MP3_BLOCK_SIZE, &count);
					if( count != MP3_BLOCK_SIZE )
						break;

					decoder->fileread+=count;

					// convert the data
					mres=acmStreamConvert( acmMp3stream, &mp3streamHead, ACM_STREAMCONVERTF_BLOCKALIGN );

					// write the decoded PCM to disk
					//count = fwrite( rawbuf, 1, mp3streamHead.cbDstLengthUsed, fpOut );

					if( mp3streamHead.cbDstLengthUsed>0)
					{
						decoder->decodeddata=rawbuf;
						decoder->decodeddata_bytes=mp3streamHead.cbDstLengthUsed;
						decoder->decodeddata_samples=decoder->decodeddata_bytes/(decoder->channels*(decoder->samplebits/8));

						if(firstdata==false)
						{
							if(decoder->FirstData(decoder)==false)
								break;

							firstdata=true;
						}
						else
						{
							if(decoder->WriteData(decoder)==false)
								break;
						}

						//memcpy(currentOutput, rawbuf, mp3streamHead.cbDstLengthUsed);
						totalDecompressedSize += mp3streamHead.cbDstLengthUsed;
						//currentOutput += mp3streamHead.cbDstLengthUsed;
					}

				};

				decoder->sum_decodeddata_bytes=totalDecompressedSize;
				decoder->WriteDataEnd(decoder);

				mres=acmStreamUnprepareHeader( acmMp3stream, &mp3streamHead, 0 );
				delete rawbuf;
			}
		}

		mres=acmStreamClose( acmMp3stream, 0 );

		// Release allocated memory
		mp3Stream->Release();
		GlobalFree(mp3HGlobal);

		return true;
	}

	return false;
}

bool MP3Player::OpenFromFile(Decoder *decoder){

	TCHAR* inputFileName=decoder->file;

	if(!inputFileName)
		return false;

	// Open the mp3 file
	HANDLE hFile = CreateFile(inputFileName, // open MYFILE.TXT
		GENERIC_READ,
		FILE_SHARE_READ, // share for reading
		NULL, // no security
		OPEN_EXISTING, // existing file only
		FILE_ATTRIBUTE_NORMAL, // normal file
		NULL); // no attr
	assert( hFile != INVALID_HANDLE_VALUE);

	// Get FileSize
	DWORD fileSize = GetFileSize(hFile, NULL);
	assert( fileSize != INVALID_FILE_SIZE);

	// Alloc buffer for file
	char* mp3Buffer = (char*)new char[fileSize];

	if(mp3Buffer)
	{
		// Read file and fill mp3Buffer
		DWORD bytesRead;
		DWORD resultReadFile = ReadFile( hFile, mp3Buffer, fileSize, &bytesRead, NULL);
		assert(resultReadFile != 0);
		assert( bytesRead == fileSize);
	}

	// Close File
	CloseHandle(hFile);

	// Open and convert MP3
	if(mp3Buffer)
	{
		decoder->filesize=fileSize;

		bool res = OpenFromMemory(mp3Buffer, fileSize,decoder);

		// Free mp3Buffer
		delete mp3Buffer;
		return res;
	}

	return false;
}

void convtowave(Decoder *decoder)
{
	MP3Player player;

	player.OpenFromFile(decoder);

	/*
	player.Play();

	while (player.GetPosition() < 40) {
	//	printf("Test music for 40s : %f elapsed\n",player.GetPosition());
	Sleep(1000);
	}
	*/

	player.Close();

	// return player.soundBuffer;
}

