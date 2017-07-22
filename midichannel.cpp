/*
#include "defines.h"
#include "songmain.h"
#include "editor.h"

#include "gui.h"

#include "object_track.h"

#define MIDIGADGETID_START (GADGET_ID_START+50)
#define MIDITHRU_ID MIDIGADGETID_START+1
#define MIDICHANNEL_ID (MIDIGADGETID_START+2)

void Edit_MIDIChannel::Gadget(guiGadget *g)
{
	if(g->gadgetID==MIDITHRU_ID)
	{
		track->MIDIeffects.channel=0;
	}
	else
	{
		int newchl=g->gadgetID-MIDICHANNEL_ID;
		
		// Channel
		track->MIDIeffects.channel=newchl;
	}

	ShowChannel();
}

void Edit_MIDIChannel::ShowChannel()
{
	int c=track->MIDIeffects.channel;

	if(oldselected)
	{
		oldselected->DrawGadget();
	}

	if(!c)
	{
		if(thrugadget)
		{
			thrugadget->DrawGadget();
		}

		oldselected=thrugadget;
	}
	else
	{
		c--;

		if(channelgadgets[c])
			channelgadgets[c]->DrawGadget();

		oldselected=channelgadgets[c];
	}

}

void Edit_MIDIChannel::InitGadgets()
{
	gadgetlists.RemoveAllGadgetLists();
	
	Clear();
	
	if(width>60 && height>60)
	{
		guiGadgetList *nl=gadgetlists.AddGadgetList(this);
		if(nl)
		{
			int w=width;
			int h=height;
			int x,y;
			int a,b,chl=1;
			char nrs[NUMBERSTRINGLEN];
			
			thrugadget=nl->AddButton(0,0,w,30,"Thru",MIDITHRU_ID,0);
			
			h-=31;
			w/=4;
			h/=4;
			
			y=32;
			
			for(a=0;a<4;a++)
			{
				x=0;
				
				for(b=0;b<4;b++)
				{
					channelgadgets[chl-1]=nl->AddButton(x,y,x+w-1,y+h-1,mainvar->ConvertIntToChar(chl,nrs),MIDICHANNEL_ID+chl,0);
					
					x+=w;
					chl++;
				}
				
				y+=h;
			}

			ShowChannel();
		}
	}
}

bool Edit_MIDIChannel::Init()
{
	bool ok=true;

	if(init==true)
	{
		InitGadgets();
	}

	return ok;
}
*/
