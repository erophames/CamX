#include "audioobject_allfilter.h"
#include "audiohardwarebuffer.h"
#include "gui.h"
#include "songmain.h"
#include "edit_audiointern.h"

guiWindow *audioobject_Intern_Allpass::OpenGUI(Seq_Song *s,InsertAudioEffect *ieffect,guiWindowSetting *settings) //v
{
	guiWindow *win;
	
	win=maingui->OpenEditor(EDITORTYPE_PLUGIN_INTERN,s,0,0,settings,this,ieffect);
	
	return win;
}

bool audioobject_Intern_Allpass::CloseGUI()
{
	return false;
}

/*
void audioobject_Intern_Allpass::InitEffectGUI(guiWindow *ed,int x,int y,int x2,int y2) // v
{
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
			
			horz.from=1;
			horz.to=1000;//ms
			horz.pos=delayTime;
			
			// Time
			time=gl->AddSlider(&horz,0,0);
//			timestring=gl->AddText(x2-h+1,y,x2,horz.y2,0,0);
			
			// FeedBack
			y=maingui->AddFontY(y);
			
			horz.y=y;
			horz.y2=y+maingui->GetFontSizeY();
			horz.from=0;
			horz.to=100;
			horz.pos=(int)(feedbackGain*100);

			feedback=gl->AddSlider(&horz,0,0);
			//feedbackstring=gl->AddText(x2-h+1,y,x2,horz.y2,0,0);

			ShowTimeGadget();
			ShowFeedbackGadget();
		}
	}
}
*/

void audioobject_Intern_Allpass::ShowTimeGadget()
{
	if(timestring)
	{
		char h[256];
		char h2[NUMBERSTRINGLEN];

		strcpy(h,"ms:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(delayTime,h2));

		timestring->ChangeButtonText(h);
	}
}

void audioobject_Intern_Allpass::ShowFeedbackGadget()
{
	if(feedbackstring)
	{
		char h[256];
		char h2[NUMBERSTRINGLEN];
		
		strcpy(h,"Feedback:");
		mainvar->AddString(h,mainvar->ConvertIntToChar(feedbackGain*100,h2));
		
		feedbackstring->ChangeButtonText(h);
	}
}

void audioobject_Intern_Allpass::Gadget(guiGadget *g)
{
	if(g==time)
	{
		if(delayTime!=g->GetPos())
		{
			delayTime=g->GetPos();
			Init();
			ShowTimeGadget();
		}
	}
	
	if(g==feedback)
	{
		ARES c=g->GetPos();
		
		c/=100;
		
		if(c!=feedbackGain)
		{
			feedbackGain=c;
			ShowFeedbackGadget();
		}
	}
}
