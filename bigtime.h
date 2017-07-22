#ifndef CAMX_BIGTIME_H
#define CAMX_BIGTIME_H 1

#include "editor.h"
#include "settings.h"
#include "guigraphics.h"

class Edit_BigTime:public Editor
{
public:
	Edit_BigTime();

	void CreateWindowMenu();
	void ShowSongWindowName();
	void Init();
	void Zoom(int);
	void MouseWheel(int delta,guiGadget *);
	
	void RefreshRealtime();
	void RefreshRealtime_Slow();

	void ShowTime();
	void ShowMenu();
	void RefreshSMPTE(); // v
	void RefreshMeasure();
	int GetZoomWidth();
	int GetZoomHeight();
	void CheckMouseOver();
	void DeActivated();
	void MouseClickInTime(bool leftmouse);
	void MouseReleaseInTime(bool leftmouse);
	void ChangeFormat(int format);
	bool DeltaY();
	void RemoveSong(Seq_Song *);

	guiGadget_CW *time;

private:

	Seq_Pos realtimepos;
	OSTART activesongposition;
	guiMenu *displaymenu,*menu_measure,*menu_showtime,*menu_showsmpte,
		*menu_zoom[5];
	int song_status,rt_counter,precounter,zoombig,mouseoverindex,editmouseindex,editmousestarty;
	int posx[6],posx2[6],numbers,deltay;

	bool showbeats,showbw;
};
#endif