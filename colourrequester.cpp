#include "colourrequester.h"
#include "objectevent.h"

#ifdef WIN32
#include <afxdlgs.h>
#endif

#include "MIDIPattern.h"
#include "object_track.h"
#include "audiofile.h"
#include "audioevent.h"
#include "gui.h"
#include "audiopattern.h"
#include "editfunctions.h"

	void Colour::LoadChunk(camxFile *file)
	{
		file->ReadChunk(&showcolour);
		file->ReadChunk(&rgb);
	}

	void Colour::SaveChunk(camxFile *file)
	{
		file->Save_Chunk(showcolour);
		file->Save_Chunk(rgb);
	}

void Colour::InitColour(Seq_Event *e)
{
	switch(e->id)
	{
	case OBJ_AUDIOEVENT:
		{
			AudioEvent *ae=(AudioEvent *)e;

			if(ae->GetAudioPattern()->GetColour()->showcolour==true)
				ae->GetAudioPattern()->GetColour()->Clone(this);
			else
				if(ae->GetAudioPattern()->track->GetColour()->showcolour==true)
					ae->GetAudioPattern()->track->GetColour()->Clone(this);
				else
				{
					Seq_Group_GroupPointer *sgp=ae->GetAudioPattern()->track->GetGroups()->FirstGroup();

					while(sgp)
					{
						if(sgp->group->colour.showcolour==true)
						{
							sgp->group->colour.Clone(this);
							break;
						}

						sgp=sgp->NextGroup();
					}
				}
		}
		break;

	case OBJ_NOTE:
	case OBJ_PROGRAM:
	case OBJ_CONTROL:
	case OBJ_PITCHBEND:
	case OBJ_SYSEX:
	case OBJ_CHANNELPRESSURE:
	case OBJ_POLYPRESSURE: // MIDI
		{
			if(e->pattern->GetColour()->showcolour==true)
				e->pattern->GetColour()->Clone(this);
			else
				if(e->pattern->track->GetColour()->showcolour==true)
					e->pattern->track->GetColour()->Clone(this);
				else
				{
					Seq_Group_GroupPointer *sgp=e->pattern->track->GetGroups()->FirstGroup();

					while(sgp)
					{
						if(sgp->group->colour.showcolour==true)
						{
							sgp->group->colour.Clone(this);
							break;
						}

						sgp=sgp->NextGroup();
					}
				}
		}
		break;

	}
}

colourReq::colourReq()
{
	mainedit->LockEdit();
}

colourReq::~colourReq()
{
	mainedit->UnlockEdit();
}

void colourReq::OpenRequester(guiWindow *win,Colour *c)
{
	if(win && c)
	{
#ifdef WIN32
		CColorDialog dlg2(/*RGB(255, 0, 0)*/ c->rgb, CC_FULLOPEN,CWnd::FromHandle(win->hWnd));

		if (dlg2.DoModal() == IDOK)
		{
			int rgb=dlg2.GetColor();

			if(rgb!=c->rgb)
			{
				c->changed=true;
				c->rgb=rgb;
			}
		}
#endif
	}
}
