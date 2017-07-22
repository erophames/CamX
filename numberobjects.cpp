#include "guiwindow.h"
#include "seqtime.h"
#include "object_song.h"
#include "colours.h"

NumberObject::NumberObject()
{
	step=1; // default
	ondisplay=false;
	invert=false;	

	editobject=0;
	added=false;
	init=false;


#ifdef DEBUG
	n[0]='N';
	n[1]='O';
#endif
}

bool NumberObject::CheckIfInside(int mx,int my)
{
	if(ondisplay==true && mx>=x && mx<=GetX2() && my>=y && my<=y2)
		return true;

	return false;
}

int NumberObject::GetX2()
{
	if(NextNumberObject() && NextNumberObject()->ondisplay==true && NextNumberObject()->x<=x2)
		return NextNumberObject()->x-1;

	return x2;
}

NumberObject *NumberOListRef::FindNumberObject(int mx,int my)
{
	NumberObject *f=FirstNumberObject();

	while(f){

		if(f->ondisplay==true && f->CheckIfInside(mx,my)==true)
			return f;

		f=f->NextNumberObject();
	}

	return 0;
}

int NumberOList::GetIndexColour()
{
	if(object->IsSelected()==true)
		return index&1?COLOUR_BACKGROUNDFORMTEXT_HIGHLITE_SELECTED:COLOUR_BACKGROUNDFORMTEXT_SELECTED;

	return index&1?COLOUR_BACKGROUNDFORMTEXT_HIGHLITE:COLOUR_BACKGROUNDFORMTEXT;
}


void NumberOListStartPosition::DrawPosition(OSTART position,int x,int x2,Object *o)
{
	posobj=o;

	timestring.pos.offset=&song->smpteoffset;
	song->timetrack.CreateTimeString(&timestring,position,timemode);

	int tx=x; 

	for(int i=0;i<timestring.index;i++)
	{
		int tox;

		if(timestring.index==1)
			tox=x2;
		else
			tox=timestring.pos.usesmpte==true?tx+bitmap->pref_smpte[i]:tx+bitmap->pref_time[i];

		if(tox>x2)
			tox=x2;

		time[i].x=tx; // X,Y Positions
		time[i].y=y;
		time[i].y2=y2;
		time[i].x2=tox-1;

		bitmap->guiDrawText(tx,y2,x2,timestring.strings[i]);

		if(i<timestring.index-1) // . , ;
		{
			int zx=tox;
			zx-=bitmap->pref_space;
			bitmap->guiDrawText(zx,y2,x2,timestring.pos.space[i]);
		}

		tx=tox;
	}
}
