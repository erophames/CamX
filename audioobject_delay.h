#ifndef CAMX_AUDIOINTERN_DELAY
#define CAMX_AUDIOINTERN_DELAY 1

#include "intern_audiofx.h"
#include "audioeffectparameter.h"

#define EQ_MAXDB 18 // +18db <-> -18dB

class audioobject_Intern_Delay:public audioobject_Intern
{
public:
	audioobject_Intern_Delay();
	void InitSampleRateAndSize(int rate,int size);
	bool InitIOChannels(int channels);
	
	AudioObject *CloneEffect(int flag,Seq_Song *s);
	AudioObject *InitOpenEffect(){return this;}

	void Delete(bool full);

	double GetParm(int index);
	bool SetParm(int index,double par);

	char *GetParmName(int index);
	char *GetParmValueString(int index);
	char *GetParmTypeValueString(int index);

	void FreeMemory();
	void Load(camxFile *);
	void Save(camxFile *);
	void ClearBuffer(int channels);

	void Init() //v
	{
		bufferLength = (int)(delayTime*(setSampleRate/1000));
	}

	char *GetEffectName(){return "Delay";} //v
	char *GetName(){return "Delay";}
	bool DoEffect(AudioEffectParameter *par); // virtual

	guiWindow *OpenGUI(Seq_Song *,InsertAudioEffect *,guiWindowSetting *); //v

	bool CloseGUI();
	
protected:

	void ShowTimeGadget();
	void ShowFeedbackGadget();
	void ShowWetGadget();
	void ShowDryGadget();

	bool bufferok;
	int bufferLength,delayTime;

	ARES feedbackGain,wetLevel, dryLevel;
	ARES *delayLineStart[MAXCHANNELSPERCHANNEL], *delayLineEnd[MAXCHANNELSPERCHANNEL], *readPtr[MAXCHANNELSPERCHANNEL];
	bool bufferused[MAXCHANNELSPERCHANNEL];
};

/*
class audioobject_Intern_Delay_Stereo:public audioobject_Intern_Delay
{
public:
audioobject_Intern_Delay_Stereo()
{
internid=INTERNAUDIO_ID_DELAY_Stereo;
InitDelay();
}


AudioObject *CloneEffect(int flag,Seq_Song *song)
{
audioobject_Intern_Delay_Stereo *clone=new audioobject_Intern_Delay_Stereo;

if(clone)
{
clone->delayTime=delayTime;
clone->feedbackGain=feedbackGain;
clone->wetLevel=wetLevel;
clone->dryLevel=dryLevel;
}

return clone;
}

int GetInputs(){return 2;}
int GetOutputs(){return 2;}

char *GetEffectName(){return "Delay [Stereo]";} //v
char *GetName(){return "Delay [Stereo]";}

AudioObject *CreateAudioObject(Seq_Song *song)
{
audioobject_Intern_Delay_Stereo *eq3=new audioobject_Intern_Delay_Stereo;

if(eq3)
{
eq3->SetSampleRate(mainaudio->GetGlobalSampleRate());
}

return eq3;
}

};
*/

/*********************************************************

Delay.h - A delay unit

Copyright (c) 1998, Scott Lehman, slehman@harmony-central.com
This code may be used and modified freely provided that credit
is given to the author in any public release. Any derivative
programs must be distributed freely and/or the modified source
code made publicly available.  All code is provided AS IS and
without warranty of any kind.
*********************************************************/

/*
class Delay : public Processor {
public:
Delay(float time, float feedback, float wetMix, float dryMix);
void Initialize(void);
void Process(void);
void Cleanup(void);
~Delay(){;}

private:
float delayTime, feedbackGain;  // delayTime is in milliseconds
float wetLevel, dryLevel, * outputSignal, * inputSignal;
float * buffer, delayLineOutput;
float * delayLineStart, * delayLineEnd, * readPtr;
int  i;
Delay(void){};
Delay(Delay&){};
};

#endif


Delay.cpp

/*********************************************************

Delay.cpp - A delay unit

Copyright (c) 1998, Scott Lehman, slehman@harmony-central.com
This code may be used and modified freely provided that credit
is given to the author in any public release. Any derivative
programs must be distributed freely and/or the modified source
code made publicly available.  All code is provided AS IS and
without warranty of any kind.
*********************************************************/

// ************  Delay(float, float, float, float)  ***********

// time - delay time, in milliseconds
// feedback - feedback gain, from 0 to 1 (or .9999...)
// wetMix - level of delayed signal, from 0 to 1
// dryMix - level of input signal, from 0 to 1

/*
Delay :: Delay (float time, float feedback, float wetMix, float dryMix) {

SetNumInputs(1);
SetNumOutputs(1);

delayTime = time;
feedbackGain = feedback;
wetLevel = wetMix;
dryLevel = dryMix;


return;
}
*/

// ******************  Initialize(void)  *******************

/*
void Delay :: Initialize(void)
{
//Double check that input/output buffers are there
if (inputs[0] == NULL)
ModuleError("Buffer in Delay input not assigned");
if (outputs[0] == NULL)
ModuleError("Buffer in Delay output not assigned");

//compute required buffer size for desired delay and allocate for it
int bufferLength = (int)(delayTime*samplingRate/1000);
if(bufferLength <= 0)
ModuleError("Delay buffer length in non-positive");
delayLineStart = new float[bufferLength];
if (delayLineStart == NULL)
ModuleError("Couldn't allocate buffer in Delay");

//set up pointers for delay line
delayLineEnd = delayLineStart + bufferLength;
readPtr = delayLineStart;

//zero out the buffer (silence)
do {
*readPtr = (float)0.0;
}
while (++readPtr < delayLineEnd);

//reset read pointer to start of delayline
readPtr = delayLineStart;

//only one in and out.  Assign to new pointers for simplicity
outputSignal = outputs[0];
inputSignal = inputs[0];

return;
}
*/

// ********************  Process(void)  ******************

/*
void Delay:: Process()
{

for (i=0; i<frameLength; i++) { //for each sample...

//get delayed sample
delayLineOutput = *readPtr;

//weight the delayed sample and the current input to create the output
outputSignal[i] = dryLevel * inputSignal[i] + 
wetLevel * delayLineOutput;

//write the input sample and any feedback to delayline
*readPtr = inputSignal[i] + 
feedbackGain * delayLineOutput;

//increment buffer index and wrap if necesary
if (++readPtr >= delayLineEnd)
readPtr = delayLineStart;

}

return;
}
*/

// **********************  Cleanup(void)  *******************
/*
void Delay :: Cleanup(void)
{
//Free memory allocated during initialization
delete [] delayLineStart;
}
*/

#endif
