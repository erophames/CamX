#ifndef CAMX_PROGRAMSETTINGS_H
#define CAMX_PROGRAMSETTINGS_H 1

#include "defines.h"
#include "object.h"

class SynthDevice;
class MIDIOutputProgram;
class guiWindow;
class MIDIInputDevice;
class MIDIOutputDevice;

class DeviceProgram:public Object
{
public:
	DeviceProgram(){Reset();}

	void Reset()
	{
		strcpy(name,"Program");
		info[0]=0;

		MIDIChannel=0;
		MIDIProgram=0;
		MIDIBank=0;
		usebanksel=false;
	}

	char name[32],info[32],
	 MIDIChannel,			// 0:thru, 1-16
	 MIDIProgram;			

	int MIDIBank; // 14 Bit
	bool usebanksel;

	bool ChangeProgram(char newprogram);
	bool ChangeBank(int newbank,bool onoff);
	bool ChangeChannel(char newchannel);

	void CloneTo(DeviceProgram *to)
	{
		strcpy(to->name,name);
		to->MIDIChannel=MIDIChannel;
		to->MIDIProgram=MIDIProgram;
		to->MIDIBank=MIDIBank;
		to->usebanksel=usebanksel;
	}

	SynthDevice *synthdevice;
	DeviceProgram *NextProgram(){return (DeviceProgram *)next;}
};

class SynthDevice:public Object
{
public:
	SynthDevice()
	{
		strcpy(name,"Device");
		info[0]=0;

		inputdevice=0;
		outdevice=0;
		oldfilename=0;
	}

	SynthDevice *NextDevice(){return (SynthDevice *)next;}
	DeviceProgram *FirstProgram(){return (DeviceProgram *)programs.GetRoot();}

	void AddProgram(DeviceProgram *);
	DeviceProgram *DeleteProgram(DeviceProgram *);
	void DeleteAllPrograms();
	bool Sort(); // true=changed

	MIDIInputDevice *inputdevice;
	MIDIOutputDevice *outdevice;
	OList programs;
	char name[32],info[32],*oldfilename;
};

class SynthDeviceList
{
public:
	SynthDevice *FirstDevice(){return (SynthDevice *)devices.GetRoot();}
	void AddDevice(SynthDevice *);
	SynthDevice *DeleteDevice(SynthDevice *);
	void DeleteAllDevices();
	bool Sort(); // true=changed

	bool BuildPopUp(MIDIOutputProgram *,guiWindow *,int x,int y,char *title);
	OList devices;
};

#endif
