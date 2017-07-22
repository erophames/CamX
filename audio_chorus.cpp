#include "audioobject_chorus.h"
#include "audiohardwarebuffer.h"
#include "gui.h"
#include "songmain.h"
#include "edit_audiointern.h"
#include "audioeffectparameter.h"

void audioobject_Intern_Chorus::CreateBuffer(int channels)
{
	DeleteBuffer();

	buffersize=(int)floor(fFreq+0.5);

	for(int i=0;i<channels;i++)
		delay_buffer[i]=new double[buffersize];
}

guiWindow *audioobject_Intern_Chorus::OpenGUI(Seq_Song *s,InsertAudioEffect *ieffect,guiWindowSetting *settings) //v
{
	guiWindow *win;
	
	win=maingui->OpenEditor(EDITORTYPE_PLUGIN_INTERN,s,0,0,settings,this,ieffect);
	
	return win;
}

bool audioobject_Intern_Chorus::CloseGUI()
{
	return false;
}

bool audioobject_Intern_Chorus::DoEffect(AudioEffectParameter *par) // virtual
{
	LockAudioObject();

	ARES *input=par->in->outputbufferARES,*output=par->out->outputbufferARES;

	double hdiv=1;
	hdiv/=buffersize;

	double hmul=2 * PI * freq,
		deltah=(delayMax-delayMin)/2 + delayMin + (delayMax-delayMin)/2;

	for(int chl=0;chl<GetSetInputPins();chl++)
	{
		if(delay_buffer[chl]) // got memory ?
		{	
			int sampleFrames=setSize;
			int samplecounter=ncounter; // restart samplecounter

			while (--sampleFrames >= 0)
			{
				double deltai = deltah * sin(hmul * hdiv * (samplecounter++));

				int di =floorf(deltai) - 1; // Delay line using Lagrange interpolator

				double t0 = deltai - di;
				double t1 = t0 - 1;
				double t2 = t0 - 2;
				double t3 = t0 - 3;

				double b0 = -t1*t2*t3/6;
				double b1 =  t0*t2*t3/2;
				double b2 = -t0*t1*t3/2;
				double b3 =  t0*t1*t2/6;

				double inputv=*input++;

				if (inPoint[chl] == buffersize) // ring counter
					inPoint[chl] = 0;

				delay_buffer[chl][inPoint[chl]++] = gain*inputv;

				for (int i=0;i<4;i++)
				{
					outPoint[chl][i] = (inPoint[chl] - di + i);

					if (outPoint[chl][i] < 0)
						outPoint[chl][i] += buffersize;

					listate[chl][i] = delay_buffer[chl][outPoint[chl][i]];
				}

				feedback[chl] = (b0*listate[chl][0] + b1*listate[chl][1] + b2*listate[chl][2] + b3*listate[chl][3]);
				*output++ = (inputv*wetdry) + ((1-wetdry) * feedback[chl]);
				
			}// while samples

		}// mem ok ?

	}// for chls

	ncounter+=setSize;

	/*
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

	}*/

	UnlockAudioObject();
	return true;
}

#ifdef OLDIE
void audioobject_Intern_Chorus::InitEffectGUI(guiWindow *ed,int x,int y,int x2,int y2) // v
{
	/*
		fGain		= 0;					// default input gain = 0 dB
		fDelayMin	= 0.5;				// default Delay time = 0
		fDelayMax	= 1;				// default Delay time = 0
		fWetDry		= 0.3;					// default wet/dry factor = 50%
	*/
	
	int hy=y,wy;
	int h=x2-x;
	
	h/=2;
	
	wy=y2-y;
	wy/=3+1;
	
	ResetGadgets();
	
	if(wy>12)
	{
		guiGadgetList *gl=ed->gadgetlists.AddGadgetList(ed);
		
		SliderCo horz;
		
		if(gl)
		{
			horz.x=x;
			horz.y=y;
			horz.y2=y+maingui->GetFontSizeY();
			horz.x2=x2-h;
			horz.horz=true;
			horz.page=1; // 10%
			
			horz.from=0;
			horz.to=100;
			
			horz.pos=(int)fGain*100;
			
			// Gain
			g_gain=gl->AddSlider(&horz,0,0);
		//	g_gainstring=gl->AddText(x2-h+1,y,x2,horz.y2,0,0);
			
			// fDelayMin
			y=maingui->AddFontY(y);
			
			horz.y=y;
			horz.y2=y+maingui->GetFontSizeY();
			horz.pos=(int)(fDelayMin*100);
			
			g_delaymin=gl->AddSlider(&horz,0,0);
		//	g_delayminstring=gl->AddText(x2-h+1,y,x2,horz.y2,0,0);
			
			// fDelayMax
			y=maingui->AddFontY(y);
			
			horz.y=y;
			horz.y2=y+maingui->GetFontSizeY();
			horz.pos=(int)(fDelayMax*100);
			
			g_delaymax=gl->AddSlider(&horz,0,0);
			//g_delaymaxstring=gl->AddText(x2-h+1,y,x2,horz.y2,0,0);
			
			// fWetDry
			y=maingui->AddFontY(y);
			
			horz.y=y;
			horz.y2=y+maingui->GetFontSizeY();
			horz.pos=(int)(fWetDry*100);
			
			g_wetdry=gl->AddSlider(&horz,0,0);
		//	g_wetdrystring=gl->AddText(x2-h+1,y,x2,horz.y2,0,0);
			
			ShowDelayMinGadget();
			ShowDelayMaxGadget();
			ShowGainGadget();
			ShowWetDryGadget();
		}
	}
}
#endif

void audioobject_Intern_Chorus::ShowDelayMinGadget()
{
	if(g_delayminstring)
	{
		char h[256];
		char h2[NUMBERSTRINGLEN];
		
		strcpy(h,"Delay Min:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(fDelayMin*1000,h2));
		mainvar->AddString(h," ms");
		
		g_delayminstring->ChangeButtonText(h);
	}
}

void audioobject_Intern_Chorus::ShowDelayMaxGadget()
{
	if(g_delaymaxstring)
	{
		char h[256];
		char h2[NUMBERSTRINGLEN];
		
		strcpy(h,"Delay Max:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(fDelayMax*1000,h2));
		mainvar->AddString(h," ms");
		
		g_delaymaxstring->ChangeButtonText(h);
	}
}

void audioobject_Intern_Chorus::ShowGainGadget()
{
	if(g_gainstring)
	{
		ARES hv=20.0*log10(gain);
		
		char out[128];
		char string[64];
		char *h=mainvar->ConvertDoubleToChar(hv,string,2);

		mainvar->AddString(h,"db");

		strcpy(out,"Gain:");
		mainvar->AddString(out,h);

		g_gainstring->ChangeButtonText(out);
	
	}
}

void audioobject_Intern_Chorus::ShowWetDryGadget()
{
	if(g_wetdrystring)
	{
		char h[256];
		char h2[NUMBERSTRINGLEN];
		
		strcpy(h,"Wet/Dry:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(this->fWetDry*100,h2));
		mainvar->AddString(h,"%");

		g_wetdrystring->ChangeButtonText(h);
	}
}


void audioobject_Intern_Chorus::Gadget(guiGadget *g)
{
	if(g==g_delaymin)
	{
		ARES c=g->GetPos();
		
		c/=100;
		
		if(fDelayMin!=c)
		{
			fDelayMin=c;
			ReCalcInternValues();
			ShowDelayMinGadget();
		}
	}
	
	if(g==g_delaymax)
	{
		ARES c=g->GetPos();
		
		c/=100;
		
		if(fDelayMax!=c)
		{
			fDelayMax=c;
			
			ReCalcInternValues();
			ShowDelayMaxGadget();
		}
	}
	
	if(g==g_wetdry)
	{
		ARES c=g->GetPos();
		
		c/=100;
		
		if(fWetDry!=c)
		{
			fWetDry=c;
			
			ReCalcInternValues();
			ShowWetDryGadget();
		}
	}
	
	if(g==g_gain)
	{
		ARES c=g->GetPos();
		
		c/=100;
		
		if(fGain!=c)
		{
			fGain=c;
			
			ReCalcInternValues();
			ShowGainGadget();
		}
	}
}

