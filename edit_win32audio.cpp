#ifdef OLDIE

#include "editwin32audio.h"
#include "gui.h"
#include "object_song.h"
#include "editfunctions.h"
#include "settings.h"
#include "editsettings.h"
#include "audiohardware.h"
#include "audiodevice.h"
#include "songmain.h"
#include "languagefiles.h"


#define WIN32GADGETID_START (GADGET_ID_START+50)

#define GADGET_STRING WIN32GADGETID_START+10
#define GADGET_SLIDER WIN32GADGETID_START+11
#define GADGET_INTG WIN32GADGETID_START+12
#define GADGET_STRINGSAMPLES WIN32GADGETID_START+13
#define GADGET_LATENCY WIN32GADGETID_START+14
#define GADGET_OK WIN32GADGETID_START+15

void Edit_Win32Audio::FreeMemory()
{
	if(winmode&WINDOWMODE_INIT)
	{	
		gadgetlists.RemoveAllGadgetLists();
	}
}

Edit_Win32Audio::Edit_Win32Audio(Edit_Settings *ed,AudioDevice *d)
{
	editorid=EDITORTYPE_WIN32AUDIO;	
	editor=ed;
	audiodevice=d;
	setSize=d->setSize;

	ResetGadgets();
};

void Edit_Win32Audio::Gadget(guiGadget *g)
{
	if(g)
	{	
		switch(g->gadgetID)
		{
		case GADGET_OK:
			if(setSize!=audiodevice->setSize)
			{
				mainaudio->SetSamplesSize(audiodevice,setSize);

				//	maingui->MessageBoxOk(0,"ShowGadgets");
				ShowGadgets();

				//	maingui->MessageBoxOk(0,"Show Editor Gadgets");
				if(editor)
					editor->ShowSettingsData();

				//	maingui->MessageBoxOk(0,"GADGET_OK End");
			}
			break;

		case GADGET_SLIDER:
			{
				double h=mainaudio->GetGlobalSampleRate();
				h/=1000;
				h*=g->GetPos();

				setSize=(int)floor(h+0.5);

				TRACE ("MS Pos %d\n",g->GetPos());

				//	editor->Save();
				ShowGadgets();
			}
			break;
		}
	}
}

void Edit_Win32Audio::ShowSlider()
{
	if(slider)
	{
		double dh=setSize;
		dh/=mainaudio->GetGlobalSampleRate();
		dh*=1000;

		slider->ChangeSlider((int)dh);
	}
}

void Edit_Win32Audio::ShowGadgets()
{
	srate=mainaudio->GetGlobalSampleRate();

	if(samples)
	{
		char res[NUMBERSTRINGLEN];

		if(char *h=mainvar->GenerateString("Samples/Buffer:",mainvar->ConvertIntToChar(setSize,res)))
		{
			samples->SetString(h);
			delete h;
		}
	}

	if(latency)
	{
		if(mainaudio->GetActiveDevice())
		{
			double dh=setSize;
			dh/=mainaudio->GetGlobalSampleRate();
			dh*=1000;

			char res[NUMBERSTRINGLEN],res3[NUMBERSTRINGLEN];
			char *h=mainvar->GenerateString(Cxs[CXS_LATENCY],":",mainvar->ConvertDoubleToChar(dh,res,3)," ms /",mainvar->ConvertIntToChar(setSize,res3)," Samples");

			if(h)
			{
				latency->SetString(h);
				delete h;
			}
		}
		else
		{
			latency->SetString("-:-");
		}
	}

}

void Edit_Win32Audio::RefreshRealtime()
{
	if(srate!=mainaudio->GetGlobalSampleRate())
		ShowGadgets();
}

void Edit_Win32Audio::KeyDown()
{
	switch(nVirtKey)
	{
	}
}


void Edit_Win32Audio::Init()
{
	FreeMemory();

	if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
	{
		ResetGadgets();

		if(guiGadgetList *gl=gadgetlists.AddGadgetList(this))
		{
			int y=1,h=(height-4)/4;

			char h2[NUMBERSTRINGLEN];

			double dh=audiodevice->minSize;
			dh/=mainaudio->GetGlobalSampleRate();
			dh*=1000;

			char *hmin=mainvar->GenerateString(" [Min:",mainvar->ConvertIntToChar(dh,h2));

			dh=audiodevice->prefsize;
			dh/=mainaudio->GetGlobalSampleRate();
			dh*=1000;
			char *hpref=mainvar->GenerateString(" Pref:",mainvar->ConvertIntToChar(dh,h2));

			dh=audiodevice->maxSize;
			dh/=mainaudio->GetGlobalSampleRate();
			dh*=1000;
			char *hmax=mainvar->GenerateString(" Max:",mainvar->ConvertIntToChar(dh,h2));

			char *hs=mainvar->GenerateString(
				audiodevice->devname,
				hmin,
				hpref,
				hmax,
				"] ms"
				);

			if(hmin)delete hmin;
			if(hpref)delete hpref;
			if(hmax)delete hmax;

			if(hs)
			{
				string=gl->AddText(0,y,width,y+h,hs,GADGET_STRING,0);
				delete hs;
			}
			y+=h+1;

			int w=width/2;
			samples=gl->AddText(0,y,w-1,y+h,0,GADGET_STRINGSAMPLES,0);
			latency=gl->AddText(w,y,width,y+h,0,GADGET_LATENCY,0);

			y+=h+1;

			// ChannelSlider
			SliderCo co;

			co.x=0;
			co.y=y;
			co.y2=y+h;
			co.x2=width;

			co.horz=true;
			co.from=(1000*audiodevice->minSize)/mainaudio->GetGlobalSampleRate();
			co.to=(1000*audiodevice->maxSize)/mainaudio->GetGlobalSampleRate();
			co.page=1;	
			co.pos=1;

			slider=gl->AddSlider(&co,GADGET_SLIDER,0);

			y+=h+1;

			ok=gl->AddButton(0,y,width,y+h,"OK",GADGET_OK,0,0);
			ShowGadgets();
			ShowSlider();
		}
	}
}

#endif
