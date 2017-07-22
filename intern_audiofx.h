#ifndef CAMX_AUDIOINTERN_OBJECT
#define CAMX_AUDIOINTERN_OBJECT 1

#include "defines.h"
#include "audioobjects.h"
#include "objectids.h"

class guiGadget;
class guiGadget_Slider;
class Edit_Plugin_Intern;
class PluginWindow;

enum{
 INTERNAUDIO_ID_EQ3,
 INTERNAUDIO_ID_EQ5,
 INTERNAUDIO_ID_EQ7,
 INTERNAUDIO_ID_EQ10,

 INTERNAUDIO_ID_EQ3_Stereo,
 INTERNAUDIO_ID_EQ5_Stereo,
 INTERNAUDIO_ID_EQ7_Stereo,
 INTERNAUDIO_ID_EQ10_Stereo,

 INTERNAUDIO_ID_DELAY,
  INTERNAUDIO_ID_DELAY_Stereo,
 INTERNAUDIO_ID_ALLPASS,
 INTERNAUDIO_ID_ALLPASS_Stereo,

 INTERNAUDIO_ID_CHORUS,
 INTERNAUDIO_ID_CHORUS_Stereo,

 INTERNAUDIO_ID_VOLUME,
 INTERNAUDIO_ID_VOLUME_Stereo,
};

class audioobject_Intern:public AudioObject
{
public:
	audioobject_Intern()
	{
		id=OBJ_AUDIOINTERN;

#ifdef ARES64
		floattype=FT_64BIT;
#else
		floattype=FT_32BIT;
#endif
	};
	
	void InitSampleRateAndSize(int rate,int size)=0;

	audioobject_Intern *NextInternEffect(){return (audioobject_Intern *)next;}
	
	virtual void GetParmMaxMin(int index,ARES *min,ARES *max)
	{
		if(min)*min=0;
		if(max)*max=0;
	}

	virtual void SetParmValue(int index,ARES value)
	{
	}

	virtual char *GetName(){return "Intern Audio ?";}
	virtual void Gadget(guiGadget *)
	{
	}
	
//	virtual void InitEffectGUI(PluginWindow *);
	virtual int GetEditorSizeX(){return 250;}
	virtual int GetEditorSizeY();

	guiWindow *CheckIfWindowIsEditor(guiWindow *);

	char valuestring[NUMBERSTRINGLEN];
};

#endif