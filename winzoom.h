#ifndef CAMX_WINZOOM_H
#define CAMX_WINZOOM_H 1

class guiZoom
{
public:
	guiZoom(){show1x=false;}
	OSTART ticksperpixel,xpixel;
	LONGLONG samples,measureraster;
	double dticksperpixel,sec;
	int index; // 0-NUMBEROFWINZOOMS
	bool withzoom,show1x,prev,next;
};
#endif
