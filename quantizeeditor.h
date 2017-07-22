#ifndef CAMX_QUANTIZEEDITOR_H
#define CAMX_QUANTIZEEDITOR_H 1

#include "editor.h"
#include "guiwindow.h"
#include "quantizeeffect.h"

class Edit_QuantizeEditor:public guiWindow
{
public:
	Edit_QuantizeEditor(QuantizeEffect *);
	
	void ShowTitle();
	EditData *EditDataMessage(EditData *);
	void Gadget(guiGadget *);
	guiMenu *CreateMenu();
	void DeInitWindow(){delete this;}
	void CreateQuantizePopUp(guiGadget *,int flag);
	void CreateHumanPopUp(guiGadget *);
	void CreateGroovePopUp(guiGadget *,int id);
	void ShowQuantizeStatus();
	void Init();
	void RefreshEvents(bool force);
	void RefreshObjects(LONGLONG type,bool editcall);
	void RefreshRealtime();
	void ResetGadgets();
	void LoadSettings();
	void SaveSettings();
	
	QuantizeEffect *effect,edit;

private:
	QuantizeEffect backup,effectcompare;		
	guiGadgetList *gadgetlist;
	guiGadget *boxquantize,*boxgroove,*boxnoteoffquant,*boxcapturequant,
	 *boxstrength,*selectquant,*selectgroove,*capturerange,
	 *strength,*boxhuman,*human,*boxsetnotelength,
	 *notelength,*reset,*flag;
};
#endif