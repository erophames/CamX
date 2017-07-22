#ifndef CAMX_guiTimeLine_H
#define CAMX_guiTimeLine_H 1

#include "object.h"
#include "winzoom.h"
#include "seqtime.h"
#include "timestring.h"

class guiZoom;
class Seq_Signature;
class Seq_Song;
class guiGadget_CW;

class guiTimeLinePos:public OStart
{
	friend class guiTimeLine;

public:
	guiTimeLinePos(){nextmeasure=0;}
	OSTART GetStart(){return ostart;}
	guiTimeLinePos *PrevPosition() {return (guiTimeLinePos *)prev;}
	guiTimeLinePos *NextPosition() {return (guiTimeLinePos *)next;}

	guiTimeLinePos *nextmeasure;
	OSTART measure,beat,zoom;
	int x,type;
	bool showtext;
};

class guiTimeLine
{	
public:

	enum
	{
		MID_SONGPOSITION,
		MID_MIDPOSITION
	};

	guiTimeLine();
	void DeInit();

	void BufferOldMidPosition();
	void DrawPositionRaster(guiBitmap *);
	void DrawPositionRaster(guiBitmap *,int y,int y2);

	bool CheckMousePositionAndSongPosition();
	guiTimeLinePos* FirstPosition() { return (guiTimeLinePos *)lpos.GetRoot(); }
	guiTimeLinePos* LastPosition() { return (guiTimeLinePos *)lpos.Getc_end(); }
	guiTimeLinePos* FindPositionX(int posx);

	guiTimeLinePos *AddPosition(OSTART time,int x,int type,bool showtext,OSTART measure=0,OSTART beat=0,OSTART zoom=0);
	int ConvertTimeToX(OSTART,int cx2); // -1,0 or x co
	int ConvertTimeToX(OSTART); // -1,0 or x co

	void RemoveAllPositions();
	OSTART ConvertXPosToTime(int); // -1 if not in header

	guiTimeLinePos *RemovePosition(guiTimeLinePos *);
	bool CheckIfInHeader(OSTART pos);
	bool CheckIfCycle(int cx,int cy);
	bool CheckMousePosition(int cx);
	bool CheckMousePosition(int cx,int cy);

	void ShowCycleAndPositions();
	void Draw();
	void RecalcSamplePositions();

	Seq_Pos pos;
	TimeString timestring;

	guiZoom *zoom; // X zoom
	guiWindow *win;
	guiBitmap *bitmap;
	guiGadget_CW *dbgadget;

	OSTART headertime,mousetime,addtomovecycle,oldmidposition,showzoom;
	LONGLONG *sampleposition;
	double samplesperpixel; // sample mode

	int sampleposition_size,
		headertimex,headertimex2,
		headersongstoppositionx,headersongstoppositionx2,
		mousetimex,midmode,oldmidposx,
		x,y,x2,y2,splity,cyclex,cyclex2,cycley2,cyclepos_x,cyclepos_x2,flag,format;

	bool cycleonheader,cycleon,movesongposition;

private:
	OListStart lpos;
};

#endif


