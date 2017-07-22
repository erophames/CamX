#include "programsettings.h"
#include "MIDIoutdevice.h"
#include "gui.h"
#include "languagefiles.h"
#include "songmain.h"

bool SynthDeviceList::BuildPopUp(MIDIOutputProgram *prog,guiWindow *win,int x,int y,char *title)
{
#ifdef OLDIE
	if(win && prog)
	{
		win->DeletePopUpMenu(true);

		if(win->popmenu)
		{
			class menu_onoff:public guiMenu
			{
			public:
				menu_onoff(MIDIOutputProgram *to)
				{
					toprogram=to;
				}

				void MenuFunction()
				{
					if(toprogram->on==true)
						toprogram->on=false;
					else
						toprogram->on=true;
				} 

				MIDIOutputProgram *toprogram;
			};

			{
				if(char *t=mainvar->GenerateString(Cxs[CXS_USE]," ",prog->device,":",prog->info))
				{
					win->popmenu->AddFMenu(t,new menu_onoff(prog),prog->on);
					delete t;
				}

				class menu_edit:public guiMenu
				{
				public:
					menu_edit(guiWindow *win,int px,int py,MIDIOutputProgram *to,char *t)
					{
						toprogram=to;
						wx=px;
						wy=py;
						window=win;

						if(t)
							title=mainvar->GenerateString(t);
						else
							title=0;
					}

					void FreeMemory()
					{
						if(title)
							delete title;
					}

					void MenuFunction()
					{
						toprogram->Edit(window,wx,wy,title);
					} 

					MIDIOutputProgram *toprogram;
					guiWindow *window;
					int wx,wy;
					char *title;
				};

				win->popmenu->AddFMenu(Cxs[CXS_EDIT],new menu_edit(win,x,y,prog,title));
			}

			if(FirstDevice())
			{
				win->popmenu->AddLine();

				SynthDevice *dev=FirstDevice();
				while(dev)
				{
					if(dev->FirstProgram())
					{
						guiMenu *sub=win->popmenu->AddMenu(dev->name,0);

						if(sub)
						{
							class menu_devprogra:public guiMenu
							{
							public:
								menu_devprogra(MIDIOutputProgram *to,DeviceProgram *prog)
								{
									toprogram=to;
									program=prog;
								}

								void MenuFunction()
								{
									strcpy(toprogram->device,program->synthdevice->name);
									strcpy(toprogram->info,program->name);
									toprogram->on=true;

									toprogram->MIDIChannel=program->MIDIChannel;
									toprogram->MIDIProgram=program->MIDIProgram;
									toprogram->MIDIBank=program->MIDIBank;
									toprogram->usebank=program->usebanksel;
									toprogram->set=true; // GUI Refresh flag
								} 

								MIDIOutputProgram *toprogram;
								DeviceProgram *program;
							};

							DeviceProgram *p=dev->FirstProgram();
							while(p){

								sub->AddFMenu(p->name,new menu_devprogra(prog,p));
								p=p->NextProgram();
							}	
						}
					}

					dev=dev->NextDevice();
				}
			}

			win->ShowPopMenu();
		}
	}
#endif

	return false;
}

// Device
bool SynthDeviceList::Sort()
{
	return false;
}

void SynthDeviceList::AddDevice(SynthDevice *device)
{
	devices.AddEndO(device);
	Sort();
}

SynthDevice *SynthDeviceList::DeleteDevice(SynthDevice *dev)
{
	dev->DeleteAllPrograms();
	if(dev->oldfilename)
		delete dev->oldfilename;

	return (SynthDevice *)devices.RemoveO(dev);
}

void SynthDeviceList::DeleteAllDevices()
{
	SynthDevice *dev=FirstDevice();

	while(dev)
		dev=DeleteDevice(dev);
}

// Programs
void SynthDevice::AddProgram(DeviceProgram *program)
{
	program->synthdevice=this;
	programs.AddEndO(program);
	Sort();
}

DeviceProgram *SynthDevice::DeleteProgram(DeviceProgram *program)
{
	return (DeviceProgram *)programs.RemoveO(program);
}

void SynthDevice::DeleteAllPrograms()
{
	DeviceProgram *p=FirstProgram();

	while(p)
		p=DeleteProgram(p);
}

bool SynthDevice::Sort() // true=changed
{
	return false;
}

bool DeviceProgram::ChangeProgram(char newprogram)
{
	if(newprogram!=MIDIProgram)
	{
		MIDIProgram=newprogram;
		return true;
	}

	return false;
}

bool DeviceProgram::ChangeBank(int newbank,bool bankon)
{
	if(newbank!=MIDIBank || usebanksel!=bankon)
	{
		usebanksel=bankon;
		MIDIBank=newbank;
		return true;
	}

	return false;
}

bool DeviceProgram::ChangeChannel(char newchannel)
{
	if(newchannel!=MIDIChannel)
	{
		MIDIChannel=newchannel;
		return true;
	}

	return false;
}

