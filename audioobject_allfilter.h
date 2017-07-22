#ifndef CAMX_AUDIOINTERN_ALLPASSFILTER
#define CAMX_AUDIOINTERN_ALLPASSFILTER 1

#include "intern_audiofx.h"
#include "audiohardware.h"
#include "audiohardwarebuffer.h"
#include "camxfile.h"
#include "audioeffectparameter.h"

#include <cmath>

#define EQ_MAXDB 18 // +18db <-> -18dB

class audioobject_Intern_Allpass:public audioobject_Intern
{
public:
	audioobject_Intern_Allpass()
	{
		internid=INTERNAUDIO_ID_ALLPASS;
		ins=outs=MAXCHANNELSPERCHANNEL;
		InitAllPass();
	}

	void InitSampleRateAndSize(int rate,int size)
	{
		Init();
	}

	bool InitIOChannels(int channels)
	{
		FreeMemory();
		for(int i=0;i<channels;i++)
		{
			bufferstart[i] = new ARES[bufferLength];
			bufferused[i]=true;

			if(bufferstart[i])
				bufferend[i] = bufferstart[i] + bufferLength;//set up pointers for delay line
		}

		ClearBuffer();
		return true;
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		audioobject_Intern_Allpass *clone=new audioobject_Intern_Allpass;

		if(clone)
		{
			clone->delayTime=delayTime;
			clone->feedbackGain=feedbackGain;
		}

		return clone;
	}
	AudioObject *InitOpenEffect(){return this;}

	void InitAllPass()
	{
		numberofparameter=2;

		delayTime=100; // ms
		feedbackGain=0.2f;

		for(int i=0;i<GetSetOutputPins();i++)
			bufferstart[i]=0;
	}

	char *GetParmName(int index)
	{
		switch(index)
		{
		case 0:
			return "delayTime";

		case 1:
			return "feedbackGain";
		}

		return 0;
	}

	void Delete(bool full)
	{
		//DeleteDefaultParms();
		FreeMemory();
		delete this;
	}

	void FreeMemory()
	{
		for(int i=0;i<GetSetOutputPins();i++)
		{
			if(bufferstart[i]){
				delete bufferstart[i];
				bufferstart[i]=0;
			}
		}

		bufferok=false;
	}

	void Load(camxFile *file)
	{
		file->ReadChunk(&delayTime);
		file->ReadChunk(&feedbackGain);  // delayTime is in milliseconds
	}

	void Save(camxFile *file)
	{
		file->Save_Chunk(delayTime);
		file->Save_Chunk(feedbackGain);  // delayTime is in milliseconds
	}

	void ClearBuffer()
	{
		for(int i=0;i<GetSetOutputPins();i++)
		{
			if(bufferused[i]==true)
			{
				readPtr[i] = bufferstart[i];

				if(bufferstart[i])
				{
					//zero out the buffer (silence)
					do {
						*readPtr[i] = 0;
					}
					while (++readPtr[i] < bufferend[i]);
				}

				bufferused[i]=false;
			}

			//reset read pointer to start of delayline
			readPtr[i] = bufferstart[i];
		}
	}

	void Init() //v
	{
		bufferLength = (int)(delayTime*(setSampleRate/1000));
	}

	char *GetEffectName(){return "AllPass";} //v
	char *GetName(){return "AllPass";}

	bool DoEffect(AudioEffectParameter *par,AudioObject *automation) // virtual
	{
		LockAudioObject();

		if(bufferok==true)
		{
			ARES *input=par->in->outputbufferARES,*output=par->out->outputbufferARES;

			int ins=GetSetInputPins();
			for(int chl=0;chl<ins;chl++)
			{
				ARES temp;

				for(int i=0; i<setSize; i++) 
				{ 
					//for each sample...
					temp = *readPtr[chl];

					*readPtr[chl] = (feedbackGain * temp) + *input++;
					*output++ = temp - (feedbackGain * *readPtr[chl]);

					if(++readPtr[chl] >= bufferend[chl])  //if reach end of buffer, wrap
						readPtr[chl] = bufferstart[chl];
				}

				bufferused[chl]=true;
			}

			UnlockAudioObject();
			return true;
		}

		UnlockAudioObject();
		return false;
	}

	//void InitEffectGUI(guiWindow *ed,int x,int y,int x2,int y2); // v

	guiWindow *OpenGUI(Seq_Song *song,InsertAudioEffect *ieffect,guiWindowSetting *settings); //v

	bool CloseGUI();
	void Gadget(guiGadget *); //v

	int GetEditorSizeX(){return 180;} //v

	void ResetGadgets()
	{
		time=timestring=0;
		feedback=feedbackstring=0;
	}

protected:
	void ShowTimeGadget();
	void ShowFeedbackGadget();

	bool bufferok;
	int bufferLength;

	guiGadget *time,*timestring,*feedback,*feedbackstring;

	int delayTime; //ms
	ARES feedbackGain;

	ARES *bufferstart[MAXCHANNELSPERCHANNEL], *bufferend[MAXCHANNELSPERCHANNEL], *readPtr[MAXCHANNELSPERCHANNEL];
	bool bufferused[MAXCHANNELSPERCHANNEL];
};

/*
class audioobject_Intern_Allpass_Stereo:public audioobject_Intern_Allpass
{
public:
	audioobject_Intern_Allpass_Stereo()
	{
		internid=INTERNAUDIO_ID_ALLPASS_Stereo;
		InitAllPass();
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		audioobject_Intern_Allpass_Stereo *clone=new audioobject_Intern_Allpass_Stereo;

		if(clone)
		{
			clone->delayTime=delayTime;
			clone->feedbackGain=feedbackGain;
		}

		return clone;
	}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		audioobject_Intern_Allpass_Stereo *apf=new audioobject_Intern_Allpass_Stereo;

		if(apf)
		{
			apf->SetSampleRate(mainaudio->GetGlobalSampleRate());
		}

		return apf;
	}

	int GetInputs(){return 2;}
	int GetOutputs(){return 2;}

	char *GetEffectName(){return "AllPass [Stereo]";} //v
	char *GetName(){return "AllPass [Stereo]";}
};
#endif
*/

/*
Allpass.h

Allpass.h - An allpass filter designed to build reverberators

Copyright (c) 1998, Scott Lehman, slehman@harmony-central.com
This code may be used and modified freely provided that credit
is given to the author in any public release. Any derivative
programs must be distributed freely and/or the modified source
code made publicly available.  All code is provided AS IS and
without warranty of any kind.

class Allpass : public Processor {
public:
Allpass(float delay, float gain);  //delay in milliseconds
void Initialize(void);
void Process(void);
void Cleanup(void);
~Allpass(){;}

private:
float delay, gain;
float * bufferStart, * bufferEnd, * readPtr;
float * inputSignal, * outputSignal;
int i, numSamples;
Allpass(Allpass&){};
};

Allpass.cpp

Allpass.cpp - An allpass filter designed to build reverberators

Copyright (c) 1998, Scott Lehman, slehman@harmony-central.com
This code may be used and modified freely provided that credit
is given to the author in any public release. Any derivative
programs must be distributed freely and/or the modified source
code made publicly available.  All code is provided AS IS and
without warranty of any kind.

Allpass :: Allpass(float delayTime, float feedbackGain)
{

SetNumInputs(1);
SetNumOutputs(1);

gain = feedbackGain;
delay = delayTime;  //delay time is in milliseconds

}


// ***************  Initialize(void)  ********************

void Allpass :: Initialize (void)
{
int bufferLength = int(delay*samplingRate/1000);
bufferStart = new float[bufferLength];
bufferEnd = bufferStart + bufferLength;

//zero out the buffer (create silence)
for(i=0; i<bufferLength; i++)
bufferStart[i] = 0.0;

//set read pointer to start of buffer
readPtr = bufferStart;

//assign pointers to the only input and output
inputSignal = inputs[0];
outputSignal = outputs[0];

return;
}


// **************** Process(void)  ********************

void Allpass :: Process (void)
{
float temp;

for(i=0; i<frameLength; i++) {  //for each sample...
temp = *readPtr;
*readPtr = gain * temp + inputSignal[i];
outputSignal[i] = temp - gain * *readPtr;

if(++readPtr >= bufferEnd)  //if reach end of buffer, wrap
readPtr = bufferStart;
}

}


// *****************  Cleanup(void)  ******************

void Allpass :: Cleanup (void)
{
delete [] bufferStart;

return;
}

*/

#endif