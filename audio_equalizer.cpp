#include "audioobject_Equalizer.h"
#include "audiohardwarebuffer.h"
#include "gui.h"
#include "edit_audiointern.h"
#include "audioeffectparameter.h"

//include "MyEQ.hpp"
#include <stdlib.h>
#include <math.h>
#include "songmain.h"
#include "camxgadgets.h"

#define GADGETID_PRESETS 0
#define GADGETID_RESET 1

void audioobject_Intern_Equalizer::BandInit(double samplerate)
{
	/*
	int x,y;

	double dt,c;

	dt=1/freq;

	for(x=0;x<3;x++)
	{
	c=1/(2*PI*bandfreq[x]);

	C_coef[x]=(ARES)(dt/c);

	for(y=1;y<MAXCHANNELSPERCHANNEL;y++)
	{
	C_coef[x+y*3]=C_coef[x];
	}
	}
	*/

	LockAudioObject();

	samplerate =1/samplerate;

	for(int x=0;x<bands;x++)
	{
		// build band circ
		double freq=bandfreq[x];

		freq *=(2*PI);
		freq =1/freq;

		double h=samplerate;

		h/=freq;

		C_coef[x]=(ARES)h;
	}

	UnlockAudioObject();
}

bool audioobject_Intern_Equalizer::DoEffect(AudioEffectParameter *par) // virtual
{
	LockAudioObject();

	if(volumebandused==false || par->in->channelsused==0)
	{
		UnlockAudioObject();
		return false;
	}

	ARES *in=par->in->outputbufferARES,*out=par->out->outputbufferARES;
	ARES u[MAXBANDS]; // max 16 bands

	// Main Vol used ?
	if(Volume[bands]!=1)
	{
		for(int ch=0;ch<iochannels;ch++) // Channel Blocks
		{
			int x=setSize;
			int eqs=ch*bands; // set start bands/channel

			while(x--)// Samples
			{
				ARES uin=*in++; // Input Sample
				int eq=eqs;

				for(int i=0;i<bands;i++,eq++) // Bands
				{
					ARES duc=uin-C_ener[eq]; // diff prev samples band rest

					duc*=C_coef[i]; // multi band
					u[i]=C_ener[eq]; // buffer
					uin-=u[i];
					C_ener[eq] +=duc;
				}

				for(int b=0;b<bands;b++) // mix bands
					uin+=u[b]*Volume[b];

				uin*=Volume[bands]; // add main vol

				*out++ =uin;
			} // for x 

		}// for ch
	}
	else // EQ without mainvol
	{
		for(int ch=0;ch<iochannels;ch++) // Channel Blocks
		{
			int x=setSize;
			int eqs=ch*bands; // set start bands/channel

			while(x--)// Samples
			{
				ARES uin=*in++; // Input Sample
				int eq=eqs;

				for(int i=0;i<bands;i++,eq++) // Bands
				{
					ARES duc=uin-C_ener[eq]; // diff prev samples band rest

					duc*=C_coef[i]; // multi band
					u[i]=C_ener[eq]; // buffer
					uin-=u[i];

					C_ener[eq] +=duc;
				}

				for(int b=0;b<bands;b++) // mix bands
					uin+=u[b]*Volume[b];

				*out++ =uin;
			} // for x
		}// for ch
	}

	UnlockAudioObject();
	return true;
}

guiWindow *audioobject_Intern_Equalizer::OpenGUI(Seq_Song *s,InsertAudioEffect *ieffect,guiWindowSetting *settings) //v
{
	guiWindow *win;

	win=maingui->OpenEditor(EDITORTYPE_PLUGIN_INTERN,s,0,0,settings,this,ieffect);

	return win;
}

bool audioobject_Intern_Equalizer::CloseGUI()
{
	return false;
}

void audioobject_Intern_Equalizer::Load(camxFile *file)
{
	for(int i=0;i<numberofparameter;i++)
	{
		file->ReadChunk(&Volume[i]); // Faktor

		if(Volume[i]!=1)
			volumebandused=true;
		file->ReadChunk(&bandfreq[i]);
	}

	file->ReadChunk(&showspectral);
}

void audioobject_Intern_Equalizer::Save(camxFile *file)
{
	for(int i=0;i<numberofparameter;i++)
	{
		file->Save_Chunk(Volume[i]); // Faktor
		file->Save_Chunk(bandfreq[i]);
	}

	file->Save_Chunk(showspectral);
}

char *audioobject_Intern_Equalizer::CreateString(char *string,int value)
{
	double db=mainaudio->ConvertFactorToDb(Volume[value]);
	char *h=mainvar->ConvertDoubleToChar(db,string,2);

	mainvar->AddString(h,"dB");

	return h;
}

/*
void audioobject_Intern_Equalizer::InitEffectGUI(guiWindow *ed,int x,int y,int x2,int y2) // v
{
	editor=ed;

	ResetGadgets();

	int hx=x,wx=x2-x;

	wx/=numberofparameter;

	wx-=4;

	if(wx>8)
	{
		guiGadgetList *gl=ed->gadgetlists.AddGadgetList(ed);

		SliderCo horz;

		if(gl)
		{
			for(int i=0;i<numberofparameter;i++)
			{
				int hy2=y2-3*maingui->GetFontSizeY();

				horz.x=hx;
				horz.y=y;
				horz.x2=hx+wx;
				horz.y2=hy2;
				horz.horz=false;
				horz.page=1; // 10%

				horz.from=0;
				horz.to=(4*EQ_MAXDB)+1; // 10.000 = 100.00 %percent,4x = 0.5 values

				double h=mainaudio->ConvertFactorToDb(Volume[i]);
				horz.pos=ConvertDbToInt(h);

				bandslider[i]=gl->AddSlider(&horz,0,0);

				//char string[NUMBERSTRINGLEN];
//				banddb[i]=gl->AddText(hx,hy2+1,hx+wx,hy2+maingui->GetFontSizeY_Sub(),CreateString(string,i),0);

				hy2+=maingui->GetFontSizeY();

//				gl->AddText(hx,hy2,hx+wx,hy2+maingui->GetFontSizeY_Sub(),bandstring[i],0);

				hx+=wx;
			}

			int hy2=y2-maingui->GetFontSizeY();

			wx=x2-x;
			wx/=3;

			// presets=gl->AddButton(frame_time.x,y,frame_time.x2,y+h-1,"Presets",);

			presets=gl->AddButton(x,hy2,x+wx-1,y2,"Presets",GADGETID_PRESETS,MODE_BLACK);

			if(presets && nr_presets==0)
				presets->Disable();

			x+=wx;
			reset=gl->AddButton(x,hy2,x+wx,y2,"Reset EQ",GADGETID_RESET,0);

			x+=wx;
			spectral=gl->AddCheckBox(x,hy2,x+wx,y2,0,0,"Show Spectral");

			if(spectral)
				spectral->SetCheckBox(showspectral);
		}
	}
}
*/

void audioobject_Intern_Equalizer::SetVolume(int band,ARES vol)
{
	Volume[band]=vol;

	// Check Volume !=1
	bool usevol=false;

	for(int i=0;i<numberofparameter;i++)
	{
		if(Volume[i]!=1)
			usevol=true;
	}

	volumebandused=usevol;
}

void audioobject_Intern_Equalizer::ShowAll()
{
	for(int i=0;i<numberofparameter;i++)
	{
		if(bandslider[i])
			bandslider[i]->ChangeSlider(ConvertDbToInt(mainaudio->ConvertFactorToDb(Volume[i])));

		char h[NUMBERSTRINGLEN];

		if(banddb[i])
			banddb[i]->SetString(CreateString(h,i));
	}
}

void audioobject_Intern_Equalizer::Gadget(guiGadget *g)
{
	if(g==presets)
	{
		editor->DeletePopUpMenu(true);

		if(editor->popmenu)
		{
			if(nr_presets>0)
			{
				guiMenu *s=editor->popmenu->AddMenu("Presets",0);

				if(s)
				{
					for(int i=0;i<nr_presets;i++)
					{
						class menu_pr:public guiMenu
						{
						public:
							menu_pr(audioobject_Intern_Equalizer *eq,EQ_Preset *p)
							{
								equalizer=eq;
								preset=p;
							}

							void MenuFunction()
							{	
								for(int i=0;i<equalizer->bands;i++)
								{
									if(preset->value[i]==0)
										equalizer->SetVolume(i,1);
									else
										equalizer->SetVolume(i,(ARES)pow (10,preset->value[i]/20));
								}

								equalizer->ShowAll();
							} //

							audioobject_Intern_Equalizer *equalizer;
							EQ_Preset *preset;
						};

						s->AddFMenu(eq_presets[i].name,new menu_pr(this,&eq_presets[i]));
					}
				}

				editor->popmenu->AddLine();
			}

			class menu_load:public guiMenu
			{
			public:
				menu_load(audioobject_Intern_Equalizer *eq)
				{
					equalizer=eq;
				}

				void MenuFunction()
				{
				} //

				audioobject_Intern_Equalizer *equalizer;
			};

			editor->popmenu->AddFMenu("Load",new menu_load(this));

			class menu_save:public guiMenu
			{
			public:
				menu_save(audioobject_Intern_Equalizer *eq)
				{
					equalizer=eq;
				}

				void MenuFunction()
				{


				} //

				audioobject_Intern_Equalizer *equalizer;
			};

			editor->popmenu->AddFMenu("Save EQ Settings",new menu_save(this));
			editor->ShowPopMenu();
		}
	}
	else
		if(g==spectral)
		{
			if(showspectral==true)
				showspectral=false;
			else
				showspectral=true;
		}
		else
			if(g==reset)
			{
				InitEQ();
				ShowAll();
			}
			else
				for(int i=0;i<numberofparameter;i++)
				{
					if(g==bandslider[i])
					{
						if(g->GetPos()==2*EQ_MAXDB+1)
							SetVolume(i,1); // +-0db
						else
							if(g->GetPos()<2*EQ_MAXDB+1) // +
							{
								double add=(2*EQ_MAXDB+1)-g->GetPos();

								SetVolume(i,(ARES)pow (10,(0.5*add)/20));
							}
							else // -
							{
								double add=g->GetPos()-(2*EQ_MAXDB+1);

								SetVolume(i,(ARES)pow (10,(0.5*-add)/20));
							}

							char h[NUMBERSTRINGLEN];

							if(banddb[i])
								banddb[i]->SetString(CreateString(h,i));
					}
				}
}
