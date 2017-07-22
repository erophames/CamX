#ifndef CAMX_EDITCROSSFADE
#define CAMX_EDITCROSSFADE 1

#include "editor.h"
#include "guiwindow.h"
#include "MIDIfilter.h"
#include "audioregion.h"

class Edit_CrossFade:public guiWindow
{
public:
	enum StatusFlag
	{
		STOPPED=0,
		STARTEDMIX,
		STARTEDOUT,
		STARTEDIN
	};

	Edit_CrossFade(Seq_CrossFade *);

	void CheckIfClose();
	EditData *EditDataMessage(EditData *);

	void Gadget(guiGadget *);
	void MouseButton(int flag);

	guiMenu *CreateMenu();

	void DeInitWindow();
	void LoadSettings();
	void SaveSettings();

	void ShowInName();
	void ShowOutName();
	void ShowLength();

	void Init();
	int ConvertSamplePositionX(LONGLONG pos);
	void RefreshRealtime();
	void ResetGadgets();
	void ShowOutCurve();
	void ShowInCurve();
	void RedrawGfx();
	void StopPlayback();

	AudioRegion playback,playback2;
	AudioRealtime *audiorealtime,*audiorealtime2;
	AudioHDFile *outfile,*infile;
	LONGLONG out_start,out_end,in_start,in_end;
	Seq_CrossFade *out,*in;
	Seq_Pattern *outpattern,*inpattern;
	Seq_Track *track;
	int status,r_status,audiorealtime_x,audiorealtime_x2,audiorealtime_y,audiorealtime_y2;
	bool endstatus_realtime,endstatus_realtime2;

private:
	void ShowCrossFades();
	void ShowMix();

	//Edit_Frame frame_out,frame_in,frame_mix;

	char *p1name,*p2name;
	guiGadget *boxenable,*p1name_gadget,*p2name_gadget,*sizetext;
	guiGadgetList *gadgetlist;

	int outx,outx2,outy,outy2,
		inx,inx2,iny,iny2,
		xfrom,x2from,yfrom,y2from,
		xto,x2to,yto,y2to,
		mix_x,mix_x2,mix_y,mix_y2,
		o_x[8],o_x2[8],o_y[8],o_y2[8],
		i_x[8],i_x2[8],i_y[8],i_y2[8];
		
	bool enable;
};
#endif