#ifndef CAMX_SYSTEMAUTOMATION_H
#define CAMX_SYSTEMAUTOMATION_H 1

#include "automation.h"

class MIDIPattern;
class Seq_Track;
class Seq_Event;
class camxFile;

class AT_SYS_Mute:public SysAutomationObject
{
public:
	AT_SYS_Mute()
	{
		sysid=SYS_MUTE;
		mute=false;
		curvetype=CT_ABSOLUT;
	}

	char *GetParmName(int index){return "Mute";}
	void Load(camxFile *);
	void Save(camxFile *);
	
	double ConvertValueToAutomationSteps(double v);
	void SendNewValue(AutomationTrack *);

	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);

	bool mute;
};

class AT_SYS_Solo:public SysAutomationObject
{
public:
	AT_SYS_Solo()
	{
		sysid=SYS_SOLO;
		solo=false;
		hasspecialdata=true;
		curvetype=CT_ABSOLUT;
	}

	char *GetParmName(int index){return "Solo";}
	void Load(camxFile *);
	void Save(camxFile *);

	double ConvertValueToAutomationSteps(double v);
	void SendNewValue(AutomationTrack *);

	char *GetParmValueStringPar(int index,double par);
	char *GetParmValueString(int index);

	bool solo;
};


#endif