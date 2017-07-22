#ifndef CAMX_GUIMENUS_H
#define CAMX_GUIMENUS_H 1

#include "object.h"
#include "menu.h"

#ifdef WIN32
#include <afxwin.h>
#endif

#define STANDARD_MENU 1
#define MENU_LINE 2
#define SELECTED_MENU_OFF 4
#define SELECTED_MENU_ON 8

class GUI;
class guiWindow;
class guiScreen;
class Editor;

class guiMenu:public Object
{	
public:
	
	virtual char *GetMenuName(){return "Menu";}

	void Init();
	guiMenu()
	{
		popup=false;
		screen=0;
		window=0;
		Init();
	}
	virtual void FreeMemory(){}
	virtual void MenuFunction(){}

	guiMenu *NextMenu(){if(GetList())return (guiMenu *)next; return 0;}
	guiMenu *PrevMenu() {if(GetList())return (guiMenu *)prev; return 0;}
	guiMenu* FirstMenu() { return (guiMenu *)menus.GetRoot(); }
	guiMenu* LastMenu() { return (guiMenu *)menus.Getc_end(); }

	guiMenu *AddFMenu(char *,guiMenu *,char *shortkey=0); // + Function
	guiMenu *AddFMenu(char *,guiMenu *,bool selonoff,char *shortkey=0); // + Function

	guiMenu *AddMenu(char *,int id);
	guiMenu *AddMenu(char *,int id,bool selonoff);
	
	void AddLine();
	void Select(int index,bool on);
	void Enable();
	void Disable(char *newname=0);
	
	guiMenu *RemoveMenu(bool all=true);
	void RemoveSubMenus();
	void ShowUndoMenu(Seq_Song *);
	Seq_Song *GetSong();

	guiMenu *CheckKey(UBYTE key,bool shift,bool ctrl);

	guiMenu *FindMenu(HMENU);
#ifdef WIN32
	HMENU OSMenuHandle;
#endif
	
	guiScreen *screen;
	guiWindow *window;

	char *name,*shortkey;
	guiMenu *menu,*undomenu,*redomenu,
		*editmenu,*functionsmenu,*selectmenu,*editormenu,*stepmenu;
	Object *object; // popmenu object
	int menucounter,index,id,flag;
	bool popup,disable;
	
private:
	OList menus; // sub menus
};
#endif


