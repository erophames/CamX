#ifndef CAMX_AUDIOINTERN_EQUALIZER
#define CAMX_AUDIOINTERN_EQUALIZER 1

#include "intern_audiofx.h"
#include "audiohardware.h"
#include "camxfile.h"


#define MAXBANDS 16
#define EQ_MAXDB 18 // +18db <-> -18dB, step 0.5

class EQ_Preset
{
public:
	char *name;
	ARES value[MAXBANDS];
};

class audioobject_Intern_Equalizer:public audioobject_Intern
{
public:
	audioobject_Intern_Equalizer()
	{
		ins=outs=MAXCHANNELSPERCHANNEL;

		volumebandused=false;
		showspectral=false;
		nr_presets=0;
		ResetGadgets();
	}

	void InitSampleRateAndSize(int rate,int size)
	{
		BandInit((double)rate);
	}

	bool InitIOChannels(int channels)
	{
		Init();
		return true;
	}

	void Delete(bool full)
	{
		//DeleteDefaultParms();
	//	FreeMemory();
		delete this;
	}

	int ConvertDbToInt(double db)
	{
		if(db==0)return (int)(EQ_MAXDB*2+1); // mid
		return (int)((EQ_MAXDB*2+1)-(db/0.5));
	}

	void ResetGadgets()
	{
		reset=spectral=presets=0;

		for(int i=0;i<MAXBANDS;i++)
		{
			bandslider[i]=0;
			banddb[i]=0;
		}
	}

	void InitEQ()
	{
		bands=numberofparameter-1; // Last Par=Main Vol

		for(int i=0;i<numberofparameter;i++)
		{
			SetVolume(i,1);
		}

		Init();

		BandInit(mainaudio->GetGlobalSampleRate());
	}

	void GetParmMaxMin(int index,ARES *min,ARES *max) // v
	{
	}

	void SetParmValue(int index,ARES value)
	{
	}

	char *GetParmName(int index)
	{
		return bandstring[index];
	}

	char *CreateString(char *string,int value);

	void Load(camxFile *);
	void Save(camxFile *);

	void ClearBuffer()
	{
		Init();
	}

	void Init() //v
	{
		LockAudioObject();

		for(int i=0;i<MAXBANDS*MAXCHANNELSPERCHANNEL;i++)
			C_ener[i]=0;

		UnlockAudioObject();

	}

	bool DoEffect(AudioEffectParameter *); // virtual

	void ShowAll();
	//void InitEffectGUI(guiWindow *ed,int x,int y,int x2,int y2); // v
	guiWindow *OpenGUI(Seq_Song *,InsertAudioEffect *,guiWindowSetting *); //v
	bool CloseGUI();
	void Gadget(guiGadget *); //v
	void SetVolume(int band,ARES vol);
	void BandInit(double freq);

	int bands;
	bool volumebandused,showspectral;

	ARES C_coef[MAXBANDS],
	 Volume[MAXBANDS], // Faktor
	 C_ener[MAXBANDS*MAXCHANNELSPERCHANNEL];

	int bandfreq[MAXBANDS];

	char *bandstring[MAXBANDS];

	guiGadget_Slider *bandslider[MAXBANDS];

	guiGadget *banddb[MAXBANDS],
	 *spectral,
	 *presets,
	 *reset;

	guiWindow *editor;

	// Preset
	int nr_presets;
	EQ_Preset eq_presets[16];
};


class audioobject_Intern_Equalizer3:public audioobject_Intern_Equalizer
{
public:
	audioobject_Intern_Equalizer3()
	{
		internid=INTERNAUDIO_ID_EQ3;
		InitEq3();
	}

	void InitEq3()
	{
		numberofparameter=4;

		bandfreq[0]=200;
		bandfreq[1]=4000;
		bandfreq[2]=12000;

		bandstring[0]="200";
		bandstring[1]="4k";
		bandstring[2]="12k";

		bandstring[3]="Vol";

		InitEQ();
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		if(audioobject_Intern_Equalizer3 *eq3=new audioobject_Intern_Equalizer3)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				eq3->bandfreq[i]=bandfreq[i];
				//strcpy(eq3->bandstring[i],bandstring[i]);
			}

			return eq3;
		}

		return 0;
	}

	AudioObject *InitOpenEffect(){return this;}

	char *GetEffectName(){return "EQ3";} //v
	char *GetName(){return "3Band Eq";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		return new audioobject_Intern_Equalizer3;
	}

	int GetEditorSizeX(){return 180;} //v
};

/*
class audioobject_Intern_Equalizer3_Stereo:public audioobject_Intern_Equalizer3
{
public:
	audioobject_Intern_Equalizer3_Stereo()
	{
		internid=INTERNAUDIO_ID_EQ3_Stereo;
		InitEq3();
	}

	int GetInputs(){return 2;}
	int GetOutputs(){return 2;}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		if(audioobject_Intern_Equalizer3_Stereo *eq3=new audioobject_Intern_Equalizer3_Stereo)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				eq3->bandfreq[i]=bandfreq[i];
				//strcpy(eq3->bandstring[i],bandstring[i]);
			}

			return eq3;
		}

		return 0;
	}

	char *GetEffectName(){return "EQ3 [ST]";} //v
	char *GetName(){return "3Band Eq [ST]";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		if(audioobject_Intern_Equalizer3_Stereo *eq3=new audioobject_Intern_Equalizer3_Stereo)
		{
			eq3->SetSampleRate(mainaudio->GetGlobalSampleRate());
			return eq3;
		}

		return 0;
	}
};
*/

class audioobject_Intern_Equalizer5:public audioobject_Intern_Equalizer
{
public:
	audioobject_Intern_Equalizer5()
	{
		internid=INTERNAUDIO_ID_EQ5;
		InitEQ5();
	}

	void InitEQ5()
	{
		numberofparameter=6;

		bandfreq[0]=20;
		bandfreq[1]=200;
		bandfreq[2]=980;
		bandfreq[3]=3900;
		bandfreq[4]=12000;

		bandstring[0]="20";
		bandstring[1]="200";
		bandstring[2]="980";
		bandstring[3]="3.9k";
		bandstring[4]="12k";
		bandstring[5]="Vol";

		InitEQ();
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		if(audioobject_Intern_Equalizer5 *eq=new audioobject_Intern_Equalizer5)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				eq->bandfreq[i]=bandfreq[i];
				//strcpy(eq->bandstring[i],bandstring[i]);
			}

			return eq;
		}

		return 0;
	}

	AudioObject *InitOpenEffect(){return this;}

	char *GetEffectName(){return "EQ5";} //v	
	char *GetName(){return "5Band Eq";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		return new audioobject_Intern_Equalizer5;
	}

	int GetEditorSizeX(){return 320;} //v
};

/*
class audioobject_Intern_Equalizer5_Stereo:public audioobject_Intern_Equalizer5
{
public:
	audioobject_Intern_Equalizer5_Stereo()
	{
		internid=INTERNAUDIO_ID_EQ5_Stereo;
		InitEQ5();
	}

	int GetInputs(){return 2;}
	int GetOutputs(){return 2;}

	AudioObject *CloneEffect(int flag)
	{
		if(audioobject_Intern_Equalizer5_Stereo *eq=new audioobject_Intern_Equalizer5_Stereo)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				eq->bandfreq[i]=bandfreq[i];
				//strcpy(eq->bandstring[i],bandstring[i]);
			}

			return eq;
		}

		return 0;
	}


	char *GetEffectName(){return "EQ5 [ST]";} //v	
	char *GetName(){return "5Band Eq [ST]";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		audioobject_Intern_Equalizer5_Stereo *eq5=new audioobject_Intern_Equalizer5_Stereo;

		if(eq5)
		{
			eq5->SetSampleRate(mainaudio->GetGlobalSampleRate());
		}

		return eq5;
	}
};
*/

class audioobject_Intern_Equalizer7:public audioobject_Intern_Equalizer
{
public:
	audioobject_Intern_Equalizer7()
	{
		internid=INTERNAUDIO_ID_EQ7;
		InitEQ7();
	}

	void InitEQ7()
	{
		numberofparameter=8;

		bandfreq[0]=100;
		bandfreq[1]=200;
		bandfreq[2]=400;
		bandfreq[3]=800;
		bandfreq[4]=1600;
		bandfreq[5]=3200;
		bandfreq[6]=6400;

		bandstring[0]="100";
		bandstring[1]="200";
		bandstring[2]="400";
		bandstring[3]="800";
		bandstring[4]="1.6k";
		bandstring[5]="3.2k";
		bandstring[6]="6.4k";

		bandstring[7]="Vol";

		InitEQ();
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		if(audioobject_Intern_Equalizer7 *eq=new audioobject_Intern_Equalizer7)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				eq->bandfreq[i]=bandfreq[i];
				//strcpy(eq->bandstring[i],bandstring[i]);
			}

			return eq;
		}

		return 0;
	}

	AudioObject *InitOpenEffect(){return this;}

	char *GetEffectName(){return "EQ7";} //v
	char *GetName(){return "7Band Eq";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		return new audioobject_Intern_Equalizer7;
	}

	int GetEditorSizeX(){return 350;} //v
};

/*
class audioobject_Intern_Equalizer7_Stereo:public audioobject_Intern_Equalizer7
{
public:
	audioobject_Intern_Equalizer7_Stereo()
	{
		internid=INTERNAUDIO_ID_EQ7_Stereo;
		InitEQ7();
	}

	AudioObject *CloneEffect(int flag)
	{
		if(audioobject_Intern_Equalizer7_Stereo *eq=new audioobject_Intern_Equalizer7_Stereo)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				eq->bandfreq[i]=bandfreq[i];
				//strcpy(eq->bandstring[i],bandstring[i]);
			}

			return eq;
		}

		return 0;
	}

	char *GetEffectName(){return "EQ7 [Stereo]";} //v
	char *GetName(){return "7Band Eq [Stereo]";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		audioobject_Intern_Equalizer7_Stereo *eq7=new audioobject_Intern_Equalizer7_Stereo;

		if(eq7)
		{
			eq7->SetSampleRate(mainaudio->GetGlobalSampleRate());
		}

		return eq7;
	}

	int GetInputs(){return 2;}
	int GetOutputs(){return 2;}
};
*/
class audioobject_Intern_Equalizer10:public audioobject_Intern_Equalizer
{
public:
	audioobject_Intern_Equalizer10()
	{
		internid=INTERNAUDIO_ID_EQ10;
		InitEq10();
	}

	void InitEq10()
	{
		numberofparameter=11;

		/*
		"60", "170", "310", "600", "1k", 
		"3k", "6k", "12k", "14k", "16k" 
		*/

		/*
		65.406392,92.498606,130.81278,184.99721,261.62557,369.99442,523.25113,
		739.9884 ,1046.5023,1479.9768,2093.0045,2959.9536,4186.0091,5919.9072,
		8372.0181,11839.814,16744.036
		*/

		bandfreq[0]=60;
		bandfreq[1]=170;
		bandfreq[2]=310;
		bandfreq[3]=600;
		bandfreq[4]=1000;
		bandfreq[5]=3000;
		bandfreq[6]=6000;
		bandfreq[7]=8000;
		bandfreq[8]=10000;
		bandfreq[9]=14000;

		bandstring[0]="60";
		bandstring[1]="170";
		bandstring[2]="310";
		bandstring[3]="600";
		bandstring[4]="1k";
		bandstring[5]="3k";
		bandstring[6]="6k";
		bandstring[7]="8k";
		bandstring[8]="10k";
		bandstring[9]="14k";

		bandstring[10]="Vol";

		// Presets

		eq_presets[0].name="Classical";
		eq_presets[0].value[0]=0;
		eq_presets[0].value[1]=0;
		eq_presets[0].value[2]=0;
		eq_presets[0].value[3]=0;
		eq_presets[0].value[4]=0;
		eq_presets[0].value[5]=0;
		eq_presets[0].value[6]=-4;
		eq_presets[0].value[7]=-4;
		eq_presets[0].value[8]=-4;
		eq_presets[0].value[9]=-6;

		eq_presets[1].name="Club";
		eq_presets[1].value[0]=0;
		eq_presets[1].value[1]=0;
		eq_presets[1].value[2]=2;
		eq_presets[1].value[3]=4;
		eq_presets[1].value[4]=4;
		eq_presets[1].value[5]=4;
		eq_presets[1].value[6]=2;
		eq_presets[1].value[7]=0;
		eq_presets[1].value[8]=0;
		eq_presets[1].value[9]=0;

		eq_presets[2].name="Dance";
		eq_presets[2].value[0]=6;
		eq_presets[2].value[1]=5;
		eq_presets[2].value[2]=2;
		eq_presets[2].value[3]=0;
		eq_presets[2].value[4]=0;
		eq_presets[2].value[5]=-2;
		eq_presets[2].value[6]=-3;
		eq_presets[2].value[7]=-4;
		eq_presets[2].value[8]=-4;
		eq_presets[2].value[9]=0;

		eq_presets[3].name="Full Bass";
		eq_presets[3].value[0]=5;
		eq_presets[3].value[1]=5;
		eq_presets[3].value[2]=5;
		eq_presets[3].value[3]=2;
		eq_presets[3].value[4]=0;
		eq_presets[3].value[5]=-2;
		eq_presets[3].value[6]=-3;
		eq_presets[3].value[7]=-4;
		eq_presets[3].value[8]=-5;
		eq_presets[3].value[9]=-5;

		eq_presets[4].name="Full Bass&Treble";
		eq_presets[4].value[0]=4;
		eq_presets[4].value[1]=3;
		eq_presets[4].value[2]=0;
		eq_presets[4].value[3]=-3;
		eq_presets[4].value[4]=-2;
		eq_presets[4].value[5]=0;
		eq_presets[4].value[6]=3;
		eq_presets[4].value[7]=4;
		eq_presets[4].value[8]=5;
		eq_presets[4].value[9]=6;

		eq_presets[5].name="Full Treble";
		eq_presets[5].value[0]=-6;
		eq_presets[5].value[1]=-6;
		eq_presets[5].value[2]=-6;
		eq_presets[5].value[3]=-2;
		eq_presets[5].value[4]=+1;
		eq_presets[5].value[5]=4;
		eq_presets[5].value[6]=10;
		eq_presets[5].value[7]=10;
		eq_presets[5].value[8]=10;
		eq_presets[5].value[9]=10;

		eq_presets[5].name="Laptop/Headphone";
		eq_presets[5].value[0]=2;
		eq_presets[5].value[1]=5;
		eq_presets[5].value[2]=2;
		eq_presets[5].value[3]=-1;
		eq_presets[5].value[4]=-1;
		eq_presets[5].value[5]=0;
		eq_presets[5].value[6]=2;
		eq_presets[5].value[7]=5;
		eq_presets[5].value[8]=7;
		eq_presets[5].value[9]=10;

		eq_presets[6].name="Large Hall";
		eq_presets[6].value[0]=5;
		eq_presets[6].value[1]=5;
		eq_presets[6].value[2]=2;
		eq_presets[6].value[3]=2;
		eq_presets[6].value[4]=0;
		eq_presets[6].value[5]=-2;
		eq_presets[6].value[6]=-2;
		eq_presets[6].value[7]=-2;
		eq_presets[6].value[8]=0;
		eq_presets[6].value[9]=0;

		eq_presets[7].name="Live";
		eq_presets[7].value[0]=-2;
		eq_presets[7].value[1]=0;
		eq_presets[7].value[2]=2;
		eq_presets[7].value[3]=3;
		eq_presets[7].value[4]=4;
		eq_presets[7].value[5]=4;
		eq_presets[7].value[6]=2.5;
		eq_presets[7].value[7]=2;
		eq_presets[7].value[8]=2;
		eq_presets[7].value[9]=1.5;

		eq_presets[8].name="Party";
		eq_presets[8].value[0]=4;
		eq_presets[8].value[1]=4;
		eq_presets[8].value[2]=0;
		eq_presets[8].value[3]=0;
		eq_presets[8].value[4]=0;
		eq_presets[8].value[5]=0;
		eq_presets[8].value[6]=0;
		eq_presets[8].value[7]=0;
		eq_presets[8].value[8]=3;
		eq_presets[8].value[9]=3;

		eq_presets[9].name="Pop";
		eq_presets[9].value[0]=-0.5;
		eq_presets[9].value[1]=1.5;
		eq_presets[9].value[2]=3;
		eq_presets[9].value[3]=3.5;
		eq_presets[9].value[4]=3;
		eq_presets[9].value[5]=-1;
		eq_presets[9].value[6]=2;
		eq_presets[9].value[7]=2;
		eq_presets[9].value[8]=-1;
		eq_presets[9].value[9]=-1;


		eq_presets[10].name="Reggae";
		eq_presets[10].value[0]=0;
		eq_presets[10].value[1]=0;
		eq_presets[10].value[2]=-0.5;
		eq_presets[10].value[3]=2;
		eq_presets[10].value[4]=0;
		eq_presets[10].value[5]=2;
		eq_presets[10].value[6]=2;
		eq_presets[10].value[7]=0;
		eq_presets[10].value[8]=0;
		eq_presets[10].value[9]=0;

		eq_presets[11].name="Rock";
		eq_presets[11].value[0]=4;
		eq_presets[11].value[1]=2.5;
		eq_presets[11].value[2]=-2;
		eq_presets[11].value[3]=-3;
		eq_presets[11].value[4]=-1;
		eq_presets[11].value[5]=0;
		eq_presets[11].value[6]=4;
		eq_presets[11].value[7]=6;
		eq_presets[11].value[8]=6;
		eq_presets[11].value[9]=6;

		eq_presets[12].name="Ska";
		eq_presets[12].value[0]=-0.5;
		eq_presets[12].value[1]=-1;
		eq_presets[12].value[2]=-0.5;
		eq_presets[12].value[3]=0;
		eq_presets[12].value[4]=2;
		eq_presets[12].value[5]=3;
		eq_presets[12].value[6]=4;
		eq_presets[12].value[7]=5;
		eq_presets[12].value[8]=5;
		eq_presets[12].value[9]=4;

		eq_presets[13].name="Soft";
		eq_presets[13].value[0]=2;
		eq_presets[13].value[1]=0;
		eq_presets[13].value[2]=-0.5;
		eq_presets[13].value[3]=-1;
		eq_presets[13].value[4]=-0.5;
		eq_presets[13].value[5]=2;
		eq_presets[13].value[6]=4;
		eq_presets[13].value[7]=5;
		eq_presets[13].value[8]=5.5;
		eq_presets[13].value[9]=6;

		eq_presets[14].name="Soft Rock";
		eq_presets[14].value[0]=2;
		eq_presets[14].value[1]=2;
		eq_presets[14].value[2]=0.5;
		eq_presets[14].value[3]=-0.5;
		eq_presets[14].value[4]=-1.5;
		eq_presets[14].value[5]=-2;
		eq_presets[14].value[6]=-1.5;
		eq_presets[14].value[7]=-0.5;
		eq_presets[14].value[8]=1.5;
		eq_presets[14].value[9]=5;


		eq_presets[15].name="Techno";
		eq_presets[15].value[0]=4;
		eq_presets[15].value[1]=2.5;
		eq_presets[15].value[2]=0;
		eq_presets[15].value[3]=-2.5;
		eq_presets[15].value[4]=-1.5;
		eq_presets[15].value[5]=0;
		eq_presets[15].value[6]=3.5;
		eq_presets[15].value[7]=5.5;
		eq_presets[15].value[8]=5;
		eq_presets[15].value[9]=4.5;

		nr_presets=16;

		InitEQ();
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		if(audioobject_Intern_Equalizer10 *eq=new audioobject_Intern_Equalizer10)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				eq->bandfreq[i]=bandfreq[i];
				//strcpy(eq->bandstring[i],bandstring[i]);
			}

			return eq;
		}

		return 0;
	}
	AudioObject *InitOpenEffect(){return this;}

	char *GetEffectName(){return "EQ10";} //v
	char *GetName(){return "10Band Eq";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		return new audioobject_Intern_Equalizer10;
	}

	int GetEditorSizeX(){return 450;} //v
};

/*
class audioobject_Intern_Equalizer10_Stereo:public audioobject_Intern_Equalizer10
{
public:
	audioobject_Intern_Equalizer10_Stereo()
	{
		internid=INTERNAUDIO_ID_EQ10_Stereo;
		InitEq10();
	}

	AudioObject *CloneEffect(int flag)
	{
		if(audioobject_Intern_Equalizer10_Stereo *eq=new audioobject_Intern_Equalizer10_Stereo)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				eq->bandfreq[i]=bandfreq[i];
				//strcpy(eq->bandstring[i],bandstring[i]);
			}

			return eq;
		}

		return 0;
	}

	char *GetEffectName(){return "EQ10 [Stereo]";} //v
	char *GetName(){return "10Band Eq [Stereo]";}

	AudioObject *CreateAudioObject(Seq_Song *song)
	{
		audioobject_Intern_Equalizer10_Stereo *eq10=new audioobject_Intern_Equalizer10_Stereo;

		if(eq10)
		{
			eq10->SetSampleRate(mainaudio->GetGlobalSampleRate());
		}

		return eq10;
	}

	int GetInputs(){return 2;}
	int GetOutputs(){return 2;}
};
*/
#endif

#ifdef SOURCEEQ


Code :
First the header file ....
//---------------------------------------------------------------------------
//
// 3 Band EQ :)
//
// EQ.H - Header file for 3 band EQ
//
// (c) Neil C / Etanza Systems / 2K6
//
// Shouts / Loves / Moans = etanza at lycos dot co dot uk
//
// This work is hereby placed in the public domain for all purposes, including
// use in commercial applications.
//
// The author assumes NO RESPONSIBILITY for any problems caused by the use of
// this software.
//
//----------------------------------------------------------------------------

#ifndef __EQ3BAND__
#define __EQ3BAND__


// ------------
//| Structures |
// ------------

typedef struct
{
// Filter #1 (Low band)

double lf; // Frequency
double f1p0; // Poles ...
double f1p1;
double f1p2;
double f1p3;

// Filter #2 (High band)

double hf; // Frequency
double f2p0; // Poles ...
double f2p1;
double f2p2;
double f2p3;

// Sample history buffer

double sdm1; // Sample data minus 1
double sdm2; // 2
double sdm3; // 3

// Gain Controls

double lg; // low gain
double mg; // mid gain
double hg; // high gain

} EQSTATE;


// ---------
//| Exports |
// ---------

extern void init_3band_state(EQSTATE* es, int lowfreq, int highfreq, int mixfreq);
extern double do_3band(EQSTATE* es, double sample);


#endif // #ifndef __EQ3BAND__
//---------------------------------------------------------------------------

Now the source ...
//----------------------------------------------------------------------------
//
// 3 Band EQ :)
//
// EQ.C - Main Source file for 3 band EQ
//
// (c) Neil C / Etanza Systems / 2K6
//
// Shouts / Loves / Moans = etanza at lycos dot co dot uk
//
// This work is hereby placed in the public domain for all purposes, including
// use in commercial applications.
//
// The author assumes NO RESPONSIBILITY for any problems caused by the use of
// this software.
//
//----------------------------------------------------------------------------

// NOTES :
//
// - Original filter code by Paul Kellet (musicdsp.pdf)
//
// - Uses 4 first order filters in series, should give 24dB per octave
//
// - Now with P4 Denormal fix :)


//----------------------------------------------------------------------------

// ----------
//| Includes |
// ----------

#include
#include "eq.h"


// -----------
//| Constants |
// -----------

static double vsa = (1.0 / 4294967295.0); // Very small amount (Denormal Fix)


// ---------------
//| Initialise EQ |
// ---------------

// Recommended frequencies are ...
//
// lowfreq = 880 Hz
// highfreq = 5000 Hz
//
// Set mixfreq to whatever rate your system is using (eg 48Khz)

void init_3band_state(EQSTATE* es, int lowfreq, int highfreq, int mixfreq)
{
// Clear state

memset(es,0,sizeof(EQSTATE));

// Set Low/Mid/High gains to unity

es->lg = 1.0;
es->mg = 1.0;
es->hg = 1.0;

// Calculate filter cutoff frequencies

es->lf = 2 * sin(M_PI * ((double)lowfreq / (double)mixfreq));
es->hf = 2 * sin(M_PI * ((double)highfreq / (double)mixfreq));
}


// ---------------
//| EQ one sample |
// ---------------

// - sample can be any range you like :)
//
// Note that the output will depend on the gain settings for each band
// (especially the bass) so may require clipping before output, but you
// knew that anyway :)

double do_3band(EQSTATE* es, double sample)
{
// Locals

double l,m,h; // Low / Mid / High - Sample Values

// Filter #1 (lowpass)

es->f1p0 += (es->lf * (sample - es->f1p0)) + vsa;
es->f1p1 += (es->lf * (es->f1p0 - es->f1p1));
es->f1p2 += (es->lf * (es->f1p1 - es->f1p2));
es->f1p3 += (es->lf * (es->f1p2 - es->f1p3));

l = es->f1p3;

// Filter #2 (highpass)

es->f2p0 += (es->hf * (sample - es->f2p0)) + vsa;
es->f2p1 += (es->hf * (es->f2p0 - es->f2p1));
es->f2p2 += (es->hf * (es->f2p1 - es->f2p2));
es->f2p3 += (es->hf * (es->f2p2 - es->f2p3));

h = es->sdm3 - es->f2p3;

// Calculate midrange (signal - (low + high))

m = es->sdm3 - (h + l);

// Scale, Combine and store

l *= es->lg;
m *= es->mg;
h *= es->hg;

// Shuffle history buffer

es->sdm3 = es->sdm2;
es->sdm2 = es->sdm1;
es->sdm1 = sample;

// Return result

return(l + m + h);
}


//----------------------------------------------------------------------------

#endif