#ifndef CAMX_audioobject_INSERTVOLUME
#define CAMX_audioobject_INSERTVOLUME 1

#include "intern_audiofx.h"
#include "audiohardware.h"
#include "audiohardwarebuffer.h"
#include "camxfile.h"
#include "editdata.h"
#include "audioeffectparameter.h"

class audioobject_Intern_IVolume:public audioobject_Intern
{
public:
	audioobject_Intern_IVolume()
	{
		internid=INTERNAUDIO_ID_VOLUME;
		ins=outs=MAXCHANNELSPERCHANNEL;

		volume=0.5;
		iovolume=false;
	}

	void InitSampleRateAndSize(int rate,int size)
	{
	}

	bool InitIOChannels(int channels){return false;}

	void Edit_Data(EditData *data)
	{
		volume=data->volume;
		ShowVolume();
	}

	AudioObject *CloneEffect(int flag,Seq_Song *s)
	{
		audioobject_Intern_IVolume *n=new audioobject_Intern_IVolume;
		n->volume=volume;
		return n;
	}
	AudioObject *InitOpenEffect(){return this;}

	void Delete(bool full)
	{
		//DeleteDefaultParms();
		FreeMemory();
		delete this;
	}

	char *GetParmName(int index)
	{
		return "Volume";
	}

	void FreeMemory()
	{
	}

	void Load(camxFile *file)
	{
		file->ReadChunk(&volume);
	}

	void Save(camxFile *file)
	{
		file->Save_Chunk(volume);
	}

	void Init() //v
	{

	}

	void SetSampleRate(int freq) // 
	{

	}

	char *GetEffectName(){return "IVolume";} //v
	char *GetName(){return "IVolume";}

	bool DoEffect(AudioEffectParameter *par,AudioObject *automation) // virtual
	{
		register ARES *input=par->in->outputbufferARES;
		register ARES *output=par->out->outputbufferARES;
		register ARES mul=(ARES)volume;

		int outs=GetSetOutputPins();

		if(volume!=1)
		{
			for(int chl=0;chl<outs;chl++)
			{
				int i=par->in->samplesinbuffer;

				while(i--)
					*output++=*input++ *mul;
			}
		}
		else
			memcpy(par->out->outputbufferARES,par->in->outputbufferARES,par->in->samplesinbuffer_size*outs);

		return true;
	}

	void ShowVolume();
	//void InitEffectGUI(guiWindow *ed,int x,int y,int x2,int y2); // v

	guiWindow *OpenGUI(Seq_Song *,InsertAudioEffect *,guiWindowSetting *); //v

	bool CloseGUI();
	//void Gadget(guiGadget *); //v

	int GetEditorSizeX(){return 180;} //v
	int GetEditorSizeY(){return 30;} //v

	void ResetGadgets()
	{
		button=0;
	}

protected:
	double volume;
	guiGadget *button;
	guiWindow *editor;
};

/*
class audioobject_Intern_IVolume_Stereo:public audioobject_Intern_IVolume
{
public:
audioobject_Intern_IVolume_Stereo()
{
internid=INTERNAUDIO_ID_VOLUME_Stereo;
}

AudioObject *CloneEffect(int flag,Seq_Song *song)
{
audioobject_Intern_IVolume_Stereo *clone=new audioobject_Intern_IVolume_Stereo;
return clone;
}

int GetInputs(){return 2;}
int GetOutputs(){return 2;}

char *GetEffectName(){return "IVolume [Stereo]";} //v
char *GetName(){return "IVolume [Stereo]";}

AudioObject *CreateAudioObject(Seq_Song *song)
{
audioobject_Intern_IVolume_Stereo *eq3=new audioobject_Intern_IVolume_Stereo;

if(eq3)
{
eq3->SetSampleRate(mainaudio->GetGlobalSampleRate());
}

return eq3;
}

};
*/

#endif