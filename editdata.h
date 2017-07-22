#ifndef CAMX_EDITDATA_H
#define CAMX_EDITDATA_H 1

#include "editor.h"
#include "seqtime.h"

class Seq_Song;
class AudioPattern;
class AudioSend;
class Seq_Signature;
class InsertAudioEffect;
class Edit_Data;

class EditData:public Object
{
public:
	enum{
		EDIT_DEVICENAME=1000,
		EDIT_PROGRAMNAME,
		EDIT_BANK,
		EDIT_PROGRAM
	};

	enum{
		EDITDATA_TYPE_INTEGER=1,
		EDITDATA_TYPE_DOUBLE,
		EDITDATA_TYPE_TIME,
		EDITDATA_TYPE_VOLUMEDB,
		EDITDATA_TYPE_VOLUMEDBNOADD,
		EDITDATA_TYPE_SIGNATURE,
		EDITDATA_TYPE_STRING,
		EDITDATA_TYPE_STRING_TITLE,
		EDITDATA_TYPE_SMPTEONLY,
		EDITDATA_TYPE_PROGRAM,
		EDITDATA_TYPE_INTEGER_OKCANCEL,
		EDITDATA_TYPE_PLUGINPROGRAMS,
		EDITDATA_TYPE_KEYS,
		EDITDATA_TYPE_INFOSTRING,
		EDITDATA_TYPE_COPYMOVEPATTERN,
	};

	EditData();
	~EditData();

	Edit_Data *editdatawin;
	Seq_Song *song;
	guiGadget_CW *parentdb;
	guiWindow *win; // calling window
	char *title; // win name

	int x,y,width,height,doubledigits;
	bool deletename,desktop,noOSborder;
	int type,id; // gadget id
	Object *helpobject;

	// Integer
	OSTART from,to,value,newvalue;
	bool nostring;

	// Double
	double dfrom,dto,dvalue,dnewvalue;
	int commas;

	// Time
	int smpteflag;
	OSTART time;
	bool copyflag;

	// onoff checkbox
	bool onoff,onoffstatus;
	char *onoffstring;

	// Volume
	AudioPattern *audiopattern;
	AudioSend *audiosend;
	double volume;

	// Signature
	
	Seq_Signature *signature;
	bool deletesigifcancel,checksongrealtimeposition;

	// String
	char *string,*newstring;
	int stringlen;

	// Device Program
	char devicename[32],programname[32];
	bool nosmpte;

	// Edit Program
	InsertAudioEffect *insertaudioeffect;
};

class EditData_SMPTE:public EditData
{
public:
	int hour,min,sec,frame,qframe,mframe;
};

class Edit_Data:public guiWindow
{
public:
	Edit_Data(EditData *);

	void RefreshRealtime();
	void RefreshRealtime_Slow();

	guiMenu *CreateMenu();
	void DeInitWindow();
	void DeActivated();
	void MouseButton(int flag);
	void MouseWheel(int delta,guiGadget *);
	void Gadget(guiGadget *);
	void Init();
	void SendDelay(int delay,guiGadget *);
	void UpdateDisplay();
	bool CheckIfObjectInside(Object *);
	void ShowSelectedPattern();

	EditData *editdata;
	
	guiGadget_Time *timegadget;
	guiGadget *patterc;

	guiGadget_Slider *slider,*measureslider;
	guiGadget_ListBox *listbox;

	guiGadget *string;
	guiGadget_Integer *measure[4],*smpte[5],*integer;

	guiGadget_Numerator *signgadget;

	guiGadget *box,*dontshow,*smpteslider,
		*senddelayGadget,
		// Time
		*oktime,*canceltime,
		// Signature
		*m[10],*mtext,*b[10],*btext,*sigok,*sigcancel;

	OSTART timebuffer;

	int senddelay;

private:
	void ShowTime(int flag,OSTART time);
	void ShowTime_SMPTE(EditData_SMPTE *);
	Seq_Signature signbackup;

	bool init;
};
#endif