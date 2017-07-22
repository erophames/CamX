#ifndef CAMX_AUDIOINTERN_CHORUS
#define CAMX_AUDIOINTERN_CHORUS 1

#include "intern_audiofx.h"
#include "audiohardware.h"
#include "audiohardwarebuffer.h"
#include "camxfile.h"


#include <cmath>

class audioobject_Intern_Chorus:public audioobject_Intern
{
public:
	audioobject_Intern_Chorus()
	{
		internid=INTERNAUDIO_ID_CHORUS;
		ins=outs=MAXCHANNELSPERCHANNEL;

		numberofparameter=4;

		/*
		fGain		= 0;					// default input gain = 0 dB
		fDelayMin	= 0.005;				// default Delay time = 0
		fDelayMax	= 0.100;				// default Delay time = 0
		fWetDry		= 0.5;					// default wet/dry factor = 50%
		*/

		fFreq=mainaudio->GetGlobalSampleRate();

		fGain		= 0;					// default input gain = 0 dB
		fDelayMin	= 0.005;				// default Delay time = 0
		fDelayMax	= 0.100;				// default Delay time = 0
		fWetDry		= 0.5;					// default wet/dry factor = 50%

		for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
			delay_buffer[i]=0;

		buffersize=0;
	}

	void InitSampleRateAndSize(int rate,int size)
	{
		Init();
	}

	bool InitIOChannels(int channels)
	{
		CreateBuffer(channels);
		ResetChorus(channels);
		ReCalcInternValues();

		return true;
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		audioobject_Intern_Chorus *clone=new audioobject_Intern_Chorus;

		if(clone)
		{
			clone->fGain=fGain;
			clone->fDelayMin=fDelayMin;
			clone->fDelayMax=fDelayMax;
			clone->fWetDry=fWetDry;
		}

		return clone;
	}
	AudioObject *InitOpenEffect(){return this;}

	void ReCalcInternValues()
	{
		gain = pow(10.0,(60.0*fGain-30.0)/20.0);

		wetdry = fWetDry;
		freq = fFreq * 10;

		delayMin = fDelayMin * fFreq; // delayMin in samples, fDelayMin in ms
		delayMax = fDelayMax * fFreq;
	}

	void ResetChorus(int channels)
	{
		ncounter = 0;

		for(int i=0;i<channels;i++)
		{
			inPoint[i]=0; 

			for(int a=0;a<4;a++)
				listate[i][a]=0;
		}

		for(int chl=0;chl<channels;chl++)
		{
			if(delay_buffer[chl])
				for (int a=0; a<buffersize; a++)
					delay_buffer[chl][a]=0;
		}
	}

	char *GetParmName(int index)
	{
		switch (index)
		{
		case 0:
			return "Input Gain";

		case 1:
			return "Delay Min";

		case 2:
			return "Delay Max";

		case 3:
			return "Wet/Dry";
		};

		return 0;
	}

	void Delete(bool full)
	{
	//	DeleteDefaultParms();
		DeleteBuffer();
		delete this;
	}

	void Close()
	{
		DeleteBuffer();
	}

	void Load(camxFile *file)
	{
		file->ReadChunk(&fGain);
		file->ReadChunk(&fDelayMin); 
		file->ReadChunk(&fDelayMax); 
		file->ReadChunk(&fWetDry); 
	}

	void Save(camxFile *file)
	{
		file->Save_Chunk(fGain);
		file->Save_Chunk(fDelayMin);  
		file->Save_Chunk(fDelayMax);
		file->Save_Chunk(fWetDry); 
	}

	char *GetEffectName(){return "Chorus";} //v
	char *GetName(){return "Chorus";}

	bool DoEffect(AudioEffectParameter *); // virtual

	//void InitEffectGUI(guiWindow *ed,int x,int y,int x2,int y2); // v

	guiWindow *OpenGUI(Seq_Song *song,InsertAudioEffect *ieffect,guiWindowSetting *settings); //v

	bool CloseGUI();
	void Gadget(guiGadget *); //v

	int GetEditorSizeX(){return 180;} //v

	void ResetGadgets()
	{
		g_gain=g_gainstring=
			g_delaymin=g_delayminstring=
			g_delaymax=g_delaymaxstring=
			g_wetdry=g_wetdrystring=0;
	}

protected:

	int buffersize;

	void ShowDelayMinGadget();
	void ShowDelayMaxGadget();
	void ShowGainGadget();
	void ShowWetDryGadget();

	guiGadget *g_gain,*g_gainstring;
	guiGadget *g_delaymin,*g_delayminstring;
	guiGadget *g_delaymax,*g_delaymaxstring;
	guiGadget *g_wetdry,*g_wetdrystring;

	/*
	enum {
	kGain,
	kWetDry,
	kFreq,
	kDelayMin,
	kDelayMax,
	};

	// config
	enum { 
	kNumProgs	= 1,
	kNumInputs	= 2,
	kNumOutputs	= 2,
	};
	*/


	// Chorus Vars

	void DeleteBuffer()
	{
		for(int i=0;i<GetSetOutputPins();i++)
		{
			if(delay_buffer[i])
			{
				delete delay_buffer[i];
				delay_buffer[i]=0;
			}
		}
	}

	void CreateBuffer(int channels);

	double
		fGain, fDelayMin,fDelayMax,delayMin,delayMax, fWetDry, fFreq, freq, gain, wetdry,	
		feedback[MAXCHANNELSPERCHANNEL],
		listate[MAXCHANNELSPERCHANNEL][4];

	int    inPoint[MAXCHANNELSPERCHANNEL];
	int    outPoint[MAXCHANNELSPERCHANNEL][4];

	int ncounter;

	//  Buffer
	double *delay_buffer[MAXCHANNELSPERCHANNEL];
};

/*
class audioobject_Intern_Chorus_Stereo:public audioobject_Intern_Chorus
{
public:
	audioobject_Intern_Chorus_Stereo()
	{
		internid=INTERNAUDIO_ID_CHORUS_Stereo;
		InitChorus();
	};

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		audioobject_Intern_Chorus_Stereo *clone=new audioobject_Intern_Chorus_Stereo;

		if(clone)
		{
			clone->fGain=fGain;
			clone->fDelayMin=fDelayMin;
			clone->fDelayMax=fDelayMax;
			clone->fWetDry=fWetDry;
		}

		return clone;
	}

	int GetInputs(){return 2;}
	int GetOutputs(){return 2;}

	char *GetEffectName(){return "Chorus [Stereo]";} //v
	char *GetName(){return "Chorus [Stereo]";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		audioobject_Intern_Chorus_Stereo *ch=new audioobject_Intern_Chorus_Stereo;

		if(ch)
			ch->SetSampleRate(mainaudio->GetGlobalSampleRate());

		return ch;
	}
};
*/

/*
#ifndef __chorus_H
#include "chorus.hpp"
#endif

#include <stdio.h>
#include <math.h>

//-------------------------------------------------------------------------------------------------------
// Chorus Effect VST plug in
// Jan 6, 2006, Kevin Kuang
// http://ccrma.stanford.edu/~kuangzn
//-------------------------------------------------------------------------------------------------------

#ifndef __chorus_H
#define __chorus_H

#include "audioeffectx.h"

//-------------------------------------------------------------------------------------------------------
class chorus : public AudioEffectX
{
public:
chorus (audioMasterCallback audioMaster);
~chorus ();

// Processes
virtual voidclear();
virtual voidprocess (float **inputs, float **outputs, long sampleFrames);
virtual voidprocessReplacing (float **inputs, float **outputs, long sampleFrames);

// Program
virtual voidsetProgramName (char *name);
virtual voidgetProgramName (char *name);

// Parameter
virtual voidsetParameter (long index, float value);
virtual float getParameter (long index);
virtual voidgetParameterLabel (long index, char *label);
virtual voidgetParameterDisplay (long index, char *text);
virtual voidgetParameterName (long index, char *text);

virtual bool getEffectName (char* name);
virtual bool getVendorString (char* text);
virtual bool getProductString (char* text);
virtual long getVendorVersion () { return 1000; }

virtual VstPlugCategory getPlugCategory () { return kPlugCategEffect; }

protected:

// param IDs
enum {
kGain,
kWetDry,
kFreq,
kDelayMin,
kDelayMax,
};

// config
enum { 
kNumProgs	= 1,
kNumInputs	= 2,
kNumOutputs	= 2,
};


double fGain, fDelayMin,fDelayMax,delayMin,delayMax, fWetDry, fFreq, freq, gain, wetdry, feedbackL, feedbackR;
double delay_bufferL[44100], delay_bufferR[44100];
double listateL[4],listateR[4]; 
int	   sampleCount;
char   programName[32];
int    i,j,n;
int    inPointL, inPointR;
int    delayBufferSize;
int    outPointL[4], outPointR[4];

};

#endif


//-------------------------------------------------------------------------------------------------------
chorus::chorus (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, 5)	// 1 program, 3 parameters
{
fGain		= 0;					// default input gain = 0 dB
fDelayMin	= 0.005;				// default Delay time = 0
fDelayMax	= 0.100;				// default Delay time = 0
fWetDry		= 0.5;					// default wet/dry factor = 50%
sampleCount = 0;					// reset sample counter
n = 0;

inPointL=0; 
inPointR=0; 
delayBufferSize=44100;				//maximum delay 44100 samples

setNumInputs (2);					// stereo in
setNumOutputs (2);					// stereo out
setUniqueID ('Deli');				// identify
canMono ();							// makes sense to feed both inputs with the same signal
canProcessReplacing ();				// supports both accumulating and replacing output
strcpy (programName, "Default");	// default program name

resume();

}


//-------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void chorus::setParameter (long index, float value)
{
switch (index)
{
case kGain:
fGain = value;
gain = pow(10.0,(60.0*fGain-30.0)/20.0);
break;
case kWetDry:
fWetDry = value;
wetdry = fWetDry;
break;
case kFreq:
fFreq = value;
freq = fFreq * 10;
break;
case kDelayMin:
fDelayMin = value;
delayMin = fDelayMin * 44100; // delayMin in samples, fDelayMin in ms
break;
case kDelayMax:
fDelayMax = value;
delayMax = fDelayMax * 44100;
break;
}

}

//-----------------------------------------------------------------------------------------
float chorus::getParameter (long index)
{

switch (index)
{
case kGain:
return fGain;
break;
case kDelayMin:
return fDelayMin;
break;
case kDelayMax:
return fDelayMax;
break;
case kWetDry:
return fWetDry;
break;
case kFreq:
return fFreq;
break;
}

}

//-----------------------------------------------------------------------------------------
void chorus::getParameterName (long index, char *label)
{
switch (index)
{
case kGain:
strcpy(label, "Input Gain");
break;
case kDelayMin:
strcpy(label, "Delay Min");
break;
case kDelayMax:
strcpy(label, "Delay Max");
break;
case kWetDry:
strcpy(label, "Wet/Dry");
break;
case kFreq:
strcpy(label, "Dealy Freq");
break;
};
}

//-----------------------------------------------------------------------------------------
void chorus::getParameterDisplay (long index, char *text)
{

switch (index)
{
case kGain:
float2string(20.0*log10(gain), text);
break;
case kDelayMin:
ms2string(delayMin, text);
break;
case kDelayMax:
ms2string(delayMax, text);
break;
case kWetDry:
long2string(wetdry*100, text);
break;
case kFreq:
long2string(freq*10, text);
break;
};
}

//-----------------------------------------------------------------------------------------
void chorus::getParameterLabel(long index, char *label)
{
switch (index)
{
case kGain :
strcpy(label, "dB");
break;
case kDelayMin :
strcpy(label, "ms");
break;
case kDelayMax :
strcpy(label, "ms");
break;
case kWetDry :
strcpy(label, "%");
break;
case kFreq :
strcpy(label, "Hz");
break;
};
}

void chorus::clear()
{
for ( int i=0; i<44100; i++){
delay_bufferL[i]=0, delay_bufferR[i]=0, listateL[i]=0, listateR[i]=0;}
AudioEffectX::resume ();
}


//-----------------------------------------------------------------------------------------
void chorus::process (float **inputs, float **outputs, long sampleFrames)
{
}

//-----------------------------------------------------------------------------------------
void chorus::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
float *in1  =  inputs[0];
float *in2  =  inputs[1];
float *out1 =  outputs[0];
float *out2 =  outputs[1];
float T = 1/44100;
int di=0;
double pi = 3.1415926;
double nT=0, deltai=0, t0=0, t1=0, t2=0, t3=0, b0=0, b1=0, b2=0, b3=0;

while (--sampleFrames >= 0)
{
nT = 1/44100 * (n++);
deltai = (delayMax-delayMin)/2 + delayMin + (delayMax-delayMin)/2 * sin(2 * pi * freq * nT);
// Delay line using Lagrange interpolator
di = floorf(deltai) - 1;
t0 = deltai - di;
t1 = t0 - 1;
t2 = t0 - 2;
t3 = t0 - 3;
b0 = -t1*t2*t3/6;
b1 =  t0*t2*t3/2;
b2 = -t0*t1*t3/2;
b3 =  t0*t1*t2/6;


//Lelf channel
if (inPointL == delayBufferSize)
inPointL = 0;

delay_bufferL[inPointL++] = gain*(*in1);
for (i=0;i<4;i++)
{
outPointL[i] = (inPointL - di + i);
if (outPointL[i] < 0)
outPointL[i] += delayBufferSize;
listateL[i] = delay_bufferL[outPointL[i]];
}
feedbackL = (b0*listateL[0] + b1*listateL[1] + b2*listateL[2] + b3*listateL[3]);

//Right channel
if (inPointR == delayBufferSize)
inPointR = 0;

delay_bufferR[inPointR++] = gain*(*in2);
for (j=0;j<4;j++)
{
outPointR[j] = (inPointR - di + j);
if (outPointR[j] < 0)
outPointR[j] += delayBufferSize;
listateR[j] = delay_bufferR[outPointR[j]];
}
feedbackR = (b0*listateR[0] + b1*listateR[1] + b2*listateR[2] + b3*listateR[3]);

// output
(*out1++) = (*in1++)*wetdry + (1-wetdry) * feedbackL;
(*out2++) = (*in2++)*wetdry + (1-wetdry) * feedbackR;

}
}
*/

#endif
