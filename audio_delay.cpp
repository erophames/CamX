#include "audioobject_delay.h"
#include "audiohardwarebuffer.h"
#include "gui.h"
#include "songmain.h"

#define PI 3.1415926535
#include "edit_audiointern.h"
#include "camxfile.h"

guiWindow *audioobject_Intern_Delay::OpenGUI(Seq_Song *s,InsertAudioEffect *ieffect,guiWindowSetting *settings) //v
{
	guiWindow *win;

	win=maingui->OpenEditor(EDITORTYPE_PLUGIN_INTERN,s,0,0,settings,this,ieffect);

	return win;
}

bool audioobject_Intern_Delay::CloseGUI()
{
	return false;
}

void audioobject_Intern_Delay::InitSampleRateAndSize(int rate,int size)
{
	Init();
}

audioobject_Intern_Delay::audioobject_Intern_Delay()
{
	internid=INTERNAUDIO_ID_DELAY;
	ins=outs=MAXCHANNELSPERCHANNEL;

	numberofparameter=4;
	bufferLength=0;

	delayTime=100; // ms
	feedbackGain=0.2f;
	wetLevel=dryLevel=1;		

	bufferok=false;

	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
		delayLineStart[i]=0;
}

char *audioobject_Intern_Delay::GetParmTypeValueString(int index)
{
	switch(index)
	{
	case 0:
		return "ms";

	case 1:
		return "%";

	case 2:
		return "%";

	case 3:
		return "%";
	}

	return 0;
}

void audioobject_Intern_Delay::FreeMemory()
{
	bufferok=false;
	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
	{
		if(delayLineStart[i])
		{
			delete delayLineStart[i];
			delayLineStart[i]=0;
		}
	}
}

bool audioobject_Intern_Delay::InitIOChannels(int channels)
{
	LockAudioObject();

	FreeMemory();

	if(bufferLength==0)
	{
		UnlockAudioObject();
		return false;
	}

	for(int i=0;i<channels;i++)
	{
		delayLineStart[i] = new ARES[bufferLength+8];

		bufferused[i]=true;

		if(delayLineStart[i])
		{
			//set up pointers for delay line
			delayLineEnd[i] = delayLineStart[i] + bufferLength;	
		}
	}

	ClearBuffer(channels);
	bufferok=true;

	UnlockAudioObject();

	return true;
}

AudioObject *audioobject_Intern_Delay::CloneEffect(int flag,Seq_Song *s)
{
	if(audioobject_Intern_Delay *clone=new audioobject_Intern_Delay)
	{
		clone->delayTime=delayTime;
		clone->feedbackGain=feedbackGain;
		clone->wetLevel=wetLevel;
		clone->dryLevel=dryLevel;

		return clone;
	}

	return 0;
}

void audioobject_Intern_Delay::Delete(bool full)
{
	//DeleteDefaultParms();
	FreeMemory();
	delete this;
}

double audioobject_Intern_Delay::GetParm(int index)
{
	switch(index)
	{
	case 0:
		{
			double h=delayTime;
			h/=1000;
			return h;
		}
		break;

	case 1:
		return feedbackGain;

	case 2:
		return wetLevel;

	case 3:
		return dryLevel;
	}

	return 0;
}

bool audioobject_Intern_Delay::SetParm(int index,double par)
{
	LockAudioObject();

	switch(index)
	{
	case 0:
		{
			double h=par*1000;

			delayTime=h;

			if(iochannels) // New Buffer
			{
				Init();
				InitIOChannels(iochannels);
			}
		}
		break;

	case 1:
		feedbackGain=par;
		break;

	case 2:
		wetLevel=par;
		break;

	case 3:
		dryLevel=par;
		break;

	default:
		UnlockAudioObject();
		return false;
		break;
	}

	UnlockAudioObject();

	return true;
}

char *audioobject_Intern_Delay::GetParmName(int index)
{
	switch(index)
	{
	case 0:
		return "delayTime";

	case 1:
		return "feedbackGain";

	case 2:
		return "wetLevel";

	case 3:
		return "dryLevel";
	}

	return 0;
}

void audioobject_Intern_Delay::Load(camxFile *file)
{
	file->ReadChunk(&delayTime);
	file->ReadChunk(&feedbackGain);  // delayTime is in milliseconds
	file->ReadChunk(&wetLevel);
	file->ReadChunk(&dryLevel);  // delayTime is in milliseconds
}

void audioobject_Intern_Delay::Save(camxFile *file)
{
	file->Save_Chunk(delayTime);
	file->Save_Chunk(feedbackGain);  // delayTime is in milliseconds
	file->Save_Chunk(wetLevel);
	file->Save_Chunk(dryLevel);  // delayTime is in milliseconds
}

bool audioobject_Intern_Delay::DoEffect(AudioEffectParameter *par) // virtual
{
	if(bufferok==true)
	{
		LockAudioObject();

		ARES *input=par->in->outputbufferARES,*output=par->out->outputbufferARES;

		for(int chl=0;chl<GetSetOutputPins();chl++)
		{
			for (int i=0; i<setSize; i++) 
			{ 
				//get delayed sample
				ARES delayLineOutput = *readPtr[chl];

				//weight the delayed sample and the current input to create the output
				*output++ = (dryLevel * (*input)) +  (wetLevel * delayLineOutput);

				//write the input sample and any feedback to delayline
				*readPtr[chl] = (*input++) + feedbackGain * delayLineOutput;

				//increment buffer index and wrap if necesary
				if (++readPtr[chl] >= delayLineEnd[chl])
					readPtr[chl] = delayLineStart[chl];	
			}

			bufferused[chl]=true;
		}

		UnlockAudioObject();
		return true;
	}

	return false;
}


void audioobject_Intern_Delay::ClearBuffer(int channels)
{
	for(int i=0;i<channels;i++)
	{
		if(bufferused[i]==true)
		{
			readPtr[i] = delayLineStart[i];

			if(readPtr[i])
			{
				//zero out the buffer (silence)
				do {
					*readPtr[i] = 0;
				}
				while (++readPtr[i] < delayLineEnd[i]);
			}

			bufferused[i]=false;
		}

		//reset read pointer to start of delayline
		readPtr[i] = delayLineStart[i];
	}
}

char *audioobject_Intern_Delay::GetParmValueString(int index)
{
	switch(index)
	{
	case 0:
		return mainvar->ConvertIntToChar(delayTime,valuestring);

	case 1:
		return mainvar->ConvertIntToChar(feedbackGain*100,valuestring);

	case 2:
		return mainvar->ConvertIntToChar(wetLevel*100,valuestring);

	case 3:
		return mainvar->ConvertIntToChar(dryLevel*100,valuestring);
	}

	return 0;
}
