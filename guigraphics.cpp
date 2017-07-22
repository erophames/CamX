#include "songmain.h"
#include "editor.h"

#include "guigraphics.h"
#include "camxgadgets.h"
#include "imagesdefines.h"
#include "object_track.h"
#include "gui.h"

guiBitmap* guiGFX::AddNewBitMap(int width,int height,int depth)
{
	guiBitmap *bitmap=new guiBitmap;

	if(bitmap)
		bitmaps.AddEndO(bitmap);

	return bitmap;
}

guiBitmap* guiGFX::FindBitMapList(int id)
{
	guiBitmap *m=FirstBitMap();

	while(m){

		if(m->id==id)
			return m;

		m=m->NextBitMap();
	}

	return 0;
}

guiBitmap* guiGFX::FindBitMap(int id)
{
	if(id>=LAST_IMAGE)return 0;
	return images[id];
}

guiBitmap* guiGFX::DeleteBitMap(guiBitmap *bitmap)
{
	if(bitmap){
#ifdef WIN32
		if(bitmap->hBitmap)
			DeleteObject(bitmap->hBitmap);
#endif

		return (guiBitmap *)bitmaps.RemoveO(bitmap);
	}

	return 0;
}

void guiGFX::DeleteAllBitMaps()
{
	// List
	guiBitmap *bitmap=FirstBitMap();

	while(bitmap)
		bitmap=DeleteBitMap(bitmap);

	// Array

	for(int i=0;i<LAST_IMAGE;i++)
		if(images[i])
		{
#ifdef WIN32
			if(images[i]->hBitmap)
				DeleteObject(images[i]->hBitmap);
#endif
			delete images[i];

			images[i]=0;
		}
}

guiBitmap* guiGFX::LoadImageToBitMap(char *name,int w,int h,int id,bool addtolist,bool addimagesdir)
{
	if(!name)
		return 0;

	guiBitmap *bm=0;

	if(addtolist==true){

		if(images[id])return images[id];
		bm=images[id]=new guiBitmap;
	}
	else
		bm=AddNewBitMap(0,0,0);

	if(bm){

		char *nname=addimagesdir==true?mainvar->GenerateString("images\\",name):mainvar->GenerateString(name);
		if(!nname)
			return 0;

#ifdef WIN32
		bm->hBitmap = (HBITMAP)LoadImage(0, nname, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
#endif

#ifdef _DEBUG
		if(!bm->hBitmap)
			TRACE ("Bitmap Error..%s\n",nname);
#endif

		if(bm->hBitmap)
		{
			BITMAP bitmap;

			GetObject(bm->hBitmap, sizeof(BITMAP), (LPSTR)&bitmap);

			bm->width=bitmap.bmWidth;
			bm->height=bitmap.bmHeight;
		}
		else
			bm->width=bm->height=0;

		bm->id=id;

		delete nname;
		return bm;
	}

	return 0;
}

bool guiGFX::InitAllBitMaps()
{
	bool ok=true;

//	LoadImageToBitMap("CamX Logo.bmp",250,200,IMAGE_LOGO);

//	LoadImageToBitMap("time_small.bmp",16,16,IMAGE_TIME_ON);
//	LoadImageToBitMap("timeoff_small.bmp",16,16,IMAGE_TIME_OFF);

	//LoadImageToBitMap("thru_on_small.bmp",32,16,IMAGE_THRU_ON);
	//LoadImageToBitMap("thru_off_small.bmp",32,16,IMAGE_THRU_OFF);

	//	LoadImageToBitMap("fx_on_small.bmp",16,16,IMAGE_FX_ON);
	//	LoadImageToBitMap("fx_off_small.bmp",16,16,IMAGE_FX_OFF);

	LoadImageToBitMap("trackmuteoff.bmp",16,16,IMAGE_TRACKMUTEOFF);
	LoadImageToBitMap("trackmuteon.bmp",16,16,IMAGE_TRACKMUTEON);

	LoadImageToBitMap("trackrecordon.bmp",16,16,IMAGE_TRACKRECORDON);
	LoadImageToBitMap("trackrecordoff.bmp",16,16,IMAGE_TRACKRECORDOFF);

	LoadImageToBitMap("tracksolooff.bmp",16,16,IMAGE_TRACKSOLOOFF);
	LoadImageToBitMap("tracksoloon.bmp",16,16,IMAGE_TRACKSOLOON);
	LoadImageToBitMap("tracksoloon2.bmp",16,16,IMAGE_TRACKOTHERTRACKSOLO);

	//LoadImageToBitMap("startbuttonon.bmp",32,32,IMAGE_PLAYBUTTON_ON);
	//LoadImageToBitMap("startbuttonoff.bmp",32,32,IMAGE_PLAYBUTTON_OFF);

	LoadImageToBitMap("stopbuttonon.bmp",32,32,IMAGE_STOPBUTTON_ON);
	LoadImageToBitMap("stopbuttonoff.bmp",32,32,IMAGE_STOPBUTTON_OFF);

	LoadImageToBitMap("recbuttonoff.bmp",32,32,IMAGE_RECBUTTON_OFF);
	LoadImageToBitMap("recbuttonon.bmp",32,32,IMAGE_RECBUTTON_ON);
	LoadImageToBitMap("recstepbuttonon.bmp",32,32,IMAGE_RECSTEPBUTTON_ON);

	//	LoadImageToBitMap("solobuttonoff.bmp",32,32,IMAGE_SOLOBUTTON_OFF);
	//	LoadImageToBitMap("solobuttonon.bmp",32,32,IMAGE_SOLOBUTTON_ON);

	//	LoadImageToBitMap("cyclebuttonoff.bmp",32,32,IMAGE_CYCLEBUTTON_OFF);
	//	LoadImageToBitMap("cyclebuttonon.bmp",32,32,IMAGE_CYCLEBUTTON_ON);

//	LoadImageToBitMap("metrobuttonoff.bmp",32,32,IMAGE_METROBUTTON_OFF);
//	LoadImageToBitMap("metrobuttonon.bmp",32,32,IMAGE_METROBUTTON_ON);

	LoadImageToBitMap("toolbox_mouse.bmp",16,16,IMAGE_TOOLBOX_MOUSE);
	LoadImageToBitMap("toolbox_pencil.bmp",16,16,IMAGE_TOOLBOX_PENCIL);
	LoadImageToBitMap("toolbox_cut.bmp",16,16,IMAGE_TOOLBOX_CUT);
	LoadImageToBitMap("toolbox_delete.bmp",16,16,IMAGE_TOOLBOX_DELETE);
	LoadImageToBitMap("toolbox_select.bmp",16,16,IMAGE_TOOLBOX_SELECT);
	LoadImageToBitMap("toolbox_edit.bmp",16,16,IMAGE_TOOLBOX_EDIT);

	LoadImageToBitMap("toolbox_mouse_nsel.bmp",16,16,IMAGE_TOOLBOX_MOUSE_NSEL);
	LoadImageToBitMap("toolbox_pencil_nsel.bmp",16,16,IMAGE_TOOLBOX_PENCIL_NSEL);
	LoadImageToBitMap("toolbox_cut_nsel.bmp",16,16,IMAGE_TOOLBOX_CUT_NSEL);
	LoadImageToBitMap("toolbox_delete_nsel.bmp",16,16,IMAGE_TOOLBOX_DELETE_NSEL);
	LoadImageToBitMap("toolbox_select_nsel.bmp",16,16,IMAGE_TOOLBOX_SELECT_NSEL);
	LoadImageToBitMap("toolbox_edit_nsel.bmp",16,16,IMAGE_TOOLBOX_EDIT_NSEL);

	//LoadImageToBitMap("automationtrack_open.bmp",16,16,IMAGE_SUBTRACK_OPEN);
	//LoadImageToBitMap("automationtrack_close.bmp",16,16,IMAGE_SUBTRACK_CLOSE);

	//LoadImageToBitMap("MIDIchannel.bmp",32,32,IMAGE_MIDICHANNEL);
	//LoadImageToBitMap("audiomixer.bmp",32,32,IMAGE_AUDIOMIXER);

	LoadImageToBitMap("startbuttonoff_small.bmp",26,26,IMAGE_PLAYBUTTON_SMALL_OFF);
	LoadImageToBitMap("startbuttonon_small.bmp",26,26,IMAGE_PLAYBUTTON_SMALL_ON);
	LoadImageToBitMap("startXbuttonon_small.bmp",26,26,IMAGE_PLAYBUTTONCROSSFADE_SMALL_ON);

	LoadImageToBitMap("recbuttonoff_small.bmp",26,26,IMAGE_RECORDBUTTON_SMALL_OFF);
	LoadImageToBitMap("recbuttonon_small.bmp",26,26,IMAGE_RECORDBUTTON_SMALL_ON);

	LoadImageToBitMap("stopbuttonoff_small.bmp",26,26,IMAGE_STOPBUTTON_SMALL_OFF);
	LoadImageToBitMap("stopbuttonon_small.bmp",26,26,IMAGE_STOPBUTTON_SMALL_ON);

	LoadImageToBitMap("audioeffect.bmp",32,16,IMAGE_AUDIOEFFECT);
	LoadImageToBitMap("MIDIeffect.bmp",32,16,IMAGE_MIDIEFFECT);

	//LoadImageToBitMap("measure_small.bmp",26,26,IMAGE_MEASURE_SMALL);
	//LoadImageToBitMap("24fps_small.bmp",26,26,IMAGE_24FPS_SMALL);
	//LoadImageToBitMap("25fps_small.bmp",26,26,IMAGE_25FPS_SMALL);
	//LoadImageToBitMap("30dffps_small.bmp",26,26,IMAGE_297FPS_SMALL);
	//LoadImageToBitMap("30fps_small.bmp",26,26,IMAGE_30FPS_SMALL);
	LoadImageToBitMap("samples_small.bmp",26,26,IMAGE_SAMPLES_SMALL);

	// Sample Editor
	LoadImageToBitMap("se_playregion.bmp",16,16,IMAGE_SEPLAYREGION_SMALL);
//	LoadImageToBitMap("se_playregionend.bmp",16,16,IMAGE_SEPLAYREGIONEND_SMALL);
//	LoadImageToBitMap("se_playregionstart.bmp",16,16,IMAGE_SEPLAYREGIONSTART_SMALL);
//	LoadImageToBitMap("se_playregionstartseek.bmp",16,16,IMAGE_SEPLAYREGIONSTARTSEEK_SMALL);

//	LoadImageToBitMap("playoff.bmp",24,24,IMAGE_PLAY_OFF);
//	LoadImageToBitMap("playon.bmp",24,24,IMAGE_PLAY_ON);

	// Buttons
	/*
	backgroundimages_16=LoadImageToBitMap("buttonbkg_16.bmp",16,16,IMAGE_BKG16);
	backgroundimages_32=LoadImageToBitMap("buttonbkg_32.bmp",32,32,IMAGE_BKG32);
	backgroundimages_64=LoadImageToBitMap("buttonbkg_64.bmp",64,64,IMAGE_BKG64);
	*/

	return ok;
}

// Track Icons

bool guiGFX::InitTrackIcons()
{
	// Scan Track Images Directory...
	TRACE ("Scan Track Icons...\n");

	char *sdir="\\Images\\Track Icons\\";
	char *usedir="Standard\\";

	if(char *dir=mainvar->GenerateString(sdir,usedir))
	{
		HANDLE hdl;	
		WIN32_FIND_DATA data;

		if(char *help=mainvar->GenerateString(mainvar->GetCamXDirectory(),dir,"*.bmp")){
			hdl=FindFirstFile(help,&data);
			delete help;
		}
		else
		{
			delete dir;
			return false;
		}

		if(hdl!=INVALID_HANDLE_VALUE){

			do{

				if(char *iconname=mainvar->GenerateString(mainvar->GetCamXDirectory(),dir,data.cFileName)){

					if(TrackIcon *ni=new TrackIcon){

						ni->bitmap=LoadImageToBitMap(iconname,0,0,0,false,false);
						ni->filename=mainvar->GenerateString(data.cFileName);

						if(ni->bitmap && ni->filename){
							trackicons.AddEndO(ni);
							TRACE ("Found Track Icon %s\n",ni->filename);
						}
						else{
							TRACE ("Error: Track Icon %s\n",data.cFileName);
							DeleteTrackIcon(ni);
						}
					}

					delete iconname;
				}

			}while(FindNextFile(hdl,&data));

			FindClose(hdl);
		}

		delete dir;
		return true;
	}

	TRACE ("Scan Track Icons End\n");

	return false;
}

TrackIcon *guiGFX::DeleteTrackIcon(TrackIcon *icon)
{
	if(icon->filename)
		delete icon->filename;

	return (TrackIcon *)trackicons.RemoveO(icon);
}

void guiGFX::DeleteAllTrackIcons()
{
	TrackIcon *icon=FirstTrackIcon();

	while(icon)
		icon=DeleteTrackIcon(icon);
}

TrackIcon *guiGFX::FindIcon(char *s)
{
	TrackIcon *icon=FirstTrackIcon();

	while(icon)
	{
		if(strcmp(icon->filename,s)==0)return icon;
		icon=icon->NextIcon();
	}

	return 0;
}

void guiBitmap::ShowMute(int x,int y,int x2,int y2,bool status,int bgcolour)
{
	guiFont *old=SetFont(&maingui->standard_bold);

	switch(status)
	{
	case true:
		guiFillRect(x,y,x2,y2,COLOUR_MUTE,COLOUR_BLACK);
		SetTextColour(COLOUR_WHITE);
		break;

	case false:
		guiFillRect(x,y,x2,y2,bgcolour,COLOUR_GREY);
		SetTextColour(COLOUR_BLACK);
		break;
	}

	guiDrawTextCenter(x,y,x2,y2-2,"M");
	SetFont(old);
}

void guiBitmap::ShowMute(guiObject *o,bool status,int bgcolour)
{
	ShowMute(o->x,o->y,o->x2,o->y2,status,bgcolour);
}

void guiBitmap::ShowChildTrackMode(int x,int y,int x2,int y2,Seq_Track *track,bool withnumber)
{
	bool on=track->showchilds;

	guiFillRect(x,y,x2,y2,on==true?COLOUR_GREY_DARK:COLOUR_GREY);

	int mx=(x2-x)/2;

	int hmy2=y2;

	if(y2<y+2*maingui->GetFontSizeY())
		withnumber=false;

	if(withnumber==true)
	{
		hmy2-=maingui->GetFontSizeY();
	}

	int my=(hmy2-y)/2;
	
	int fx;

	double h=mx;
	h*=0.8;

	int hx1=mx-(int)h;
	int hx2=mx+(int)h;

	h=mx;
	h*=0.2;

	fx=h;

	h=my;
	h*=0.5;

	if(h>maingui->GetFontSizeY())
		h=maingui->GetFontSizeY();

	int hy1=my-(int)h;
	int hy2=my+(int)h;

	mx+=x;
	my+=y;
	hx2+=x;
	hx1+=x;
	hy1+=y;
	hy2+=y;

	if(on==true)
	{
		guiDrawLine(mx,hy2,hx2,hy1,COLOUR_FOLDERBORDER);
		guiDrawLineY(hy1,hx1,hx2);
		guiDrawLine(hx1,hy1,mx,hy2);
	}
	else
	{
		fx+=x;

		guiDrawLineX(fx,hy1,hy2,COLOUR_GREY_DARK);
		guiDrawLine(fx,hy1,hx2+1,hy1+(hy2-hy1)/2+1);
		guiDrawLine(fx,hy2,hx2+1,hy1+(hy2-hy1)/2);
	}

	if(withnumber==true)
	{
		char h[NUMBERSTRINGLEN];

		SetTextColour(on==true?COLOUR_WHITE:COLOUR_BLACK);
		SetFont(&maingui->standardfont);
		guiDrawTextFontY(x+1,y2-1,x2-1,mainvar->ConvertIntToChar(track->GetCountChildTracks(),h));
	}
}

void guiBitmap::ShowChildTrackMode(guiObject *o,Seq_Track *t,bool withnumber)
{
	ShowChildTrackMode(o->x,o->y,o->x2,o->y2,t,withnumber);
}

void guiBitmap::ShowAutomationTrackVisible(int x,int y,int x2,int y2,AutomationTrack *at)
{
	guiFillRect(x,y,x2,y2,COLOUR_BLACK);

	switch(at->visible)
	{
	case true:
		{
			int mx=(x2-x)/2;
			int my=(y2-y)/2;

			double h=mx;
			h*=0.8;

			int x1=mx-(int)h;
			int x2=mx+(int)h;

			h=my;
			h*=0.5;

			int y1=my-(int)h;
			int y2=my+(int)h;

			mx+=x;
			my+=y;
			x2+=x;
			x1+=x;
			y1+=y;
			y2+=y;

			guiDrawLine(mx,y2,x2,y1,COLOUR_AUTOMATIONTRACKS);
			guiDrawLineY(y1,x2,x1);
			guiDrawLine(x1,y1,mx,y2);
			//	bitmap->guiFill(mx,my,COLOUR_YELLOW);
		}
		break;

	case false:
		{
			double h=x2-x;
			h*=0.25;

			int fx=h;
			int tx=3*h;

			h=y2-y;
			h*=0.25;

			int fy=h;
			int ty=3*h;

			fx+=x;
			fy+=y;
			ty+=y;
			tx+=x;

			//x+=child.x;
			//x1+=child.x;
			//y1+=child.y;
			//y2+=child.y;

			guiDrawLineX(fx,fy,ty,COLOUR_AUTOMATIONTRACKS);
			guiDrawLine(fx,fy,tx+1,fy+(ty-fy)/2+1);
			guiDrawLine(fx,ty,tx+1,fy+(ty-fy)/2);

			//bitmap->guiDrawText(fx+1,fy+(ty-fy)/2,sub.x2,"C");

			//bitmap->guiFill(fx+1,fy+(ty-fy)/2,COLOUR_YELLOW);
		}
		break;
	}
}

void guiBitmap::ShowAutomationSettings(int x,int y,int x2,int y2,TrackHead *track)
{
	guiFillRect(x,y,x2,y2,track->HasAutomation()==true?COLOUR_AUTOMATIONTRACKSUSED:COLOUR_AUTOMATIONTRACKS,COLOUR_GREY);
	
	int w=x2-x;

	if(w<2*maingui->GetFontSizeY())
	{
		switch(track->automationon)
		{
		case true:
			SetTextColour(COLOUR_BLACK);
			guiDrawTextCenter(x,y,x2,y2,"A");
			break;

		case false:
			SetTextColour(COLOUR_GREY);
			guiDrawTextCenter(x,y,x2,y2,"a");
			break;
		}
	}
	else
		switch(track->automationon)
	{
		case true:
			SetTextColour(COLOUR_BLACK);
			guiDrawTextCenter(x,y,x2,y2,"AUTO");
			break;

		case false:
			SetTextColour(COLOUR_GREY);
			guiDrawTextCenter(x,y,x2,y2,"auto");
			break;
	}
}

void guiBitmap::ShowAutomationMode(int x,int y,int x2,int y2,TrackHead *track)
{
	guiFillRect(x,y,x2,y2,COLOUR_AUTOMATIONTRACKS);

	switch(track->showautomationstracks)
	{
	case true:
		{
			int mx=(x2-x)/2;
			int my=(y2-y)/2;

			double h=mx;
			h*=0.8;

			int x1=mx-(int)h;
			int x2=mx+(int)h;

			h=my;
			h*=0.5;

			int y1=my-(int)h;
			int y2=my+(int)h;

			mx+=x;
			my+=y;
			x2+=x;
			x1+=x;
			y1+=y;
			y2+=y;

			guiDrawLine(mx,y2,x2,y1,COLOUR_BLACK);
			guiDrawLineY(y1,x2,x1);
			guiDrawLine(x1,y1,mx,y2);
			//	bitmap->guiFill(mx,my,COLOUR_YELLOW);
		}
		break;

	case false:
		{
			double h=x2-x;
			h*=0.25;

			int fx=h;
			int tx=3*h;

			h=y2-y;
			h*=0.25;

			int fy=h;
			int ty=3*h;

			fx+=x;
			fy+=y;
			ty+=y;
			tx+=x;

		
			guiDrawLineX(fx,fy,ty,COLOUR_BLACK);
			guiDrawLine(fx,fy,tx+1,fy+(ty-fy)/2+1);
			guiDrawLine(fx,ty,tx+1,fy+(ty-fy)/2);
		}
		break;
	}
}

void guiBitmap::ShowThru(guiObject *o,bool status,bool MIDI,int bgcolour)
{
	guiFont *old=SetFont(&maingui->standard_bold);
	switch(status)
	{
	case true:
		guiFillRect(o->x,o->y,o->x2,o->y2,COLOUR_THRUON_BACKGROUND,COLOUR_GREY);
		SetTextColour(COLOUR_WHITE);
		break;

	case false:
		guiFillRect(o->x,o->y,o->x2,o->y2,bgcolour,COLOUR_GREY);
		SetTextColour(COLOUR_BLACK);
		break;
	}

	if(MIDI==true)
	{
		int tw=GetTextWidth("** MIDI Thru **");

		guiDrawTextCenter(o->x,o->y,o->x2,o->y2,tw<(o->x2-o->x)?"MIDI Thru":"mT");
	}
	else
		guiDrawTextCenter(o->x,o->y,o->x2,o->y2-2,"T");

	SetFont(old);
}

void guiBitmap::ShowInputMonitoring(guiObject *o,Seq_Track *track,bool status,int bgcolour)
{
	if(!track)return;

	guiFont *old=SetFont(&maingui->standard_bold);

	switch(status)
	{
	case true:
		guiFillRect(o->x,o->y,o->x2,o->y2,COLOUR_RED,COLOUR_RED_LIGHT);
		SetTextColour(COLOUR_BLACK);
		break;

	case false:
		guiFillRect(o->x,o->y,o->x2,o->y2,bgcolour,COLOUR_GREY);
		SetTextColour(COLOUR_BLACK);
		break;
	}

	guiDrawTextCenter(o->x,o->y,o->x2,o->y2-2,"I");

	SetFont(old);
}

void guiBitmap::ShowRecord(guiObject *o,Seq_Track *track,bool status,int bgcolour)
{
	if(!track)return;

	guiFont *old=SetFont(&maingui->standard_bold);
	switch(status)
	{
	case true:
		guiFillRect(o->x,o->y,o->x2,o->y2,track->FirstChildTrack()?COLOUR_RECORDCHILDS:COLOUR_RECORD,COLOUR_RED_LIGHT);
		SetTextColour(track->recordtracktype==TRACKTYPE_MIDI?COLOUR_YELLOW:COLOUR_WHITE);
		break;

	case false:
		guiFillRect(o->x,o->y,o->x2,o->y2,bgcolour,COLOUR_GREY);
		SetTextColour(COLOUR_BLACK);
		break;
	}

	switch(track->recordtracktype)
	{
	case TRACKTYPE_MIDI:
		guiDrawTextCenter(o->x,o->y,o->x2,o->y2-2,"Rm");
		break;

	case TRACKTYPE_AUDIO:
		guiDrawTextCenter(o->x,o->y,o->x2,o->y2-2,"R");
		break;
	}

	SetFont(old);
}

void guiBitmap::ShowSolo(int x,int y,int x2,int y2,int status,int bgcolour)
{
	guiFont *old=SetFont(&maingui->standard_bold);
	switch(status)
	{
	case SOLO_OTHER:
		guiFillRect(x,y,x2,y2,COLOUR_GREY_DARK,COLOUR_GREY);
		SetTextColour(COLOUR_RED);
		break;

	case SOLO_THIS:
		guiFillRect(x,y,x2,y2,COLOUR_YELLOW,COLOUR_GREY);
		SetTextColour(COLOUR_BLACK);
		break;

	case SOLO_OFF:
		guiFillRect(x,y,x2,y2,bgcolour,COLOUR_GREY);
		SetTextColour(COLOUR_BLACK);
		break;
	}

	guiDrawTextCenter(x,y,x2,y2-2,"S");
	SetFont(old);
}

void guiBitmap::ShowSolo(guiObject *o,int status,int bgcolour)
{
	ShowSolo(o->x,o->y,o->x2,o->y2,status,bgcolour);
}