#include "guimenu.h"
#include "songmain.h"

#include "keycodes.h"
#include "shortkeys.h"
#include "languagefiles.h"

// OS Key Commands

guiMenu *guiMenu::FindMenu(HMENU hmenu)
{
	guiMenu *s=FirstMenu();

	// Delete Sub Menus
	while(s)
	{
		if(s->OSMenuHandle==hmenu)
			return s;

		s=s->NextMenu();
	}

		return 0;
}

void guiMenu::RemoveSubMenus()
{
	guiMenu *s=FirstMenu();

	// Delete Sub Menus
	while(s)
		s=s->RemoveMenu();
}

guiMenu *guiMenu::RemoveMenu(bool full)
{	
	if(full==true)
	{
		RemoveSubMenus();

		if(name)
		{
			delete name;
			name=0;
		}

		if(shortkey)
		{
			delete shortkey;
			shortkey=0;
		}

	//	if(OSMenuHandle)
	//		DestroyMenu(OSMenuHandle);

		FreeMemory();

		if(GetList())
			return (guiMenu *)GetList()->RemoveO(this);

		delete this;
	}
	else
	{
		guiMenu *s=FirstMenu();

		int pos=0;

		// Delete Sub Menus
		while(s)
		{
			BOOL r=::RemoveMenu(this->OSMenuHandle,pos,MF_BYPOSITION);

			s->RemoveSubMenus();

			TRACE ("Menus after remove %d DeleteMenu %d\n",s->menus.GetCount(),r);

			pos++;
			s=s->NextMenu();
		}
	}

	return 0;
}

guiMenu *guiMenu::AddFMenu(char *mname,guiMenu *nmenu,char *sk)
{
	if(!nmenu)return 0;

	nmenu->flag=STANDARD_MENU;
	nmenu->menu=this;

	if(!mname)
		mname=nmenu->GetMenuName();

	size_t sl=0;

	if(mname)
		sl+=strlen(mname);
	else
		sl+=4;

	sl++;

	if(sk)
	{
		nmenu->shortkey=mainvar->GenerateString(sk);
	}
	else
		nmenu->shortkey=0;

	if(nmenu->shortkey)
		sl+=strlen(nmenu->shortkey)+8;

	if(nmenu->name=new char[sl])
	{
		strcpy(nmenu->name,mname?mname:".");

		if(sk)
		{
#ifdef WIN32
			mainvar->AddString(nmenu->name," \t");
			mainvar->AddString(nmenu->name,sk);
#endif
		}

		if(menu && menu->menu)
			nmenu->id=menu->menu->menucounter++;
		else
			if(menu) // Parent
				nmenu->id=menu->menucounter++;
			else
				nmenu->id=menucounter++;

		menus.AddEndO(nmenu); // Add V Menu
	}

	return nmenu;
}

guiMenu *guiMenu::AddFMenu(char *mname,guiMenu *nmenu,bool selonoff,char *sk)
{
	if(guiMenu *n=AddFMenu(mname,nmenu,sk))
	{
		n->flag=selonoff==true?SELECTED_MENU_ON:SELECTED_MENU_OFF;
		return n;
	}

	return 0;
}

guiMenu *guiMenu::AddMenu(char *mname,int id)
{
	if(guiMenu *newmenu=new guiMenu)
	{	
		newmenu->flag=STANDARD_MENU;
		newmenu->menu=this;

		if(mname)
			newmenu->name=mainvar->GenerateString(mname);
		else
		{
			newmenu->name=new char[1];

			if(newmenu->name)
				newmenu->name[0]=0;
		}

		newmenu->id=id;

		if(newmenu->name)
		{
			if(menu && menu->menu)
				newmenu->id=menu->menu->menucounter++;
			else
				if(menu) // Parent
					newmenu->id=menu->menucounter++;
				else
					newmenu->id=menucounter++;

			menus.AddEndO(newmenu); // Add V Menu
		}
		else
		{
			delete newmenu;
			newmenu=0;
		}

		return newmenu;
	}

	return 0;
}

guiMenu *guiMenu::AddMenu(char *mname,int id,bool selonoff)
{
	if(guiMenu *n=AddMenu(mname,id))
	{
		n->flag=selonoff==true?SELECTED_MENU_ON:SELECTED_MENU_OFF;
		return n;
	}

	return 0;
}

void guiMenu::AddLine()
{
	// Avoid Double Line and First Menu Line
	if((!LastMenu()) || LastMenu()->flag==MENU_LINE)
		return;

	if(guiMenu *newmenu=new guiMenu)
	{
		if(menu && menu->menu)
			newmenu->id=menu->menu->menucounter++;
		else
			if(menu) // Parent
				newmenu->id=menu->menucounter++;
			else
				newmenu->id=menucounter++;

		newmenu->menu=this;
		newmenu->flag=MENU_LINE;

		menus.AddEndO(newmenu); // Add V Menu
	}
}

void guiMenu::Init()
{
	shortkey=0;
	name=0;
	flag=0;
	index=-1; // head

	OSMenuHandle=0;

	// Object
	//list=0; // not in menu list

	SetList(0);// not in menu list

	disable=false;
	menucounter=0;
	menu=undomenu=redomenu=editmenu=functionsmenu=selectmenu=editormenu=stepmenu=0;
};

void guiMenu::Enable()
{
	disable=false;

#ifdef WIN32
	if(menu->OSMenuHandle)
		ModifyMenu(menu->OSMenuHandle,index,MF_BYPOSITION|MF_ENABLED|MF_STRING,id,name);
#endif
}

void guiMenu::Disable(char *disablename)
{
	disable=true;

#ifdef WIN32
	if(menu->OSMenuHandle)
		ModifyMenu(menu->OSMenuHandle,index,MF_BYPOSITION|MF_DISABLED|MF_GRAYED|MF_STRING,id,disablename?disablename:name);
#endif
}

void guiMenu::Select(int index,bool on)
{
	// Change Sub Menu
	guiMenu *sub=FirstMenu();
	int i=index;

	while(sub && i--)
		sub=sub->NextMenu();

	if(sub)
	{
#ifdef WIN32
		ModifyMenu(OSMenuHandle,index,on==true?MF_BYPOSITION|MF_STRING|MF_CHECKED:MF_BYPOSITION|MF_STRING|MF_UNCHECKED,sub->id,sub->name);
#endif
	}
}

guiMenu *guiMenu::CheckKey(UBYTE key,bool shift,bool ctrl)
{
	guiMenu *f=0;

	if(shortkey)
	{
		char *c=shortkey;
		size_t i=strlen(shortkey);

		if(i==0)return 0;

		//TRACE ("Menu Name %s\n",name);

		/*
		if(key>=65 && key<=90)
		{
		key+=32; // A->a
		TRACE ("Menu Key Change %d %c\n",key,key);
		}
		*/

		// CTRL + key
		if( i>=6 &&
			ctrl==true &&
			(*c=='C' || *c=='c') &&
			(*(c+1)=='T' || *(c+1)=='t') &&
			(*(c+2)=='R' || *(c+2)=='r') &&
			(*(c+3)=='L' || *(c+3)=='l')
			)
		{
			c+=4;
			i-=4;

			TRACE ("### %s\n",c);

			if(i>=2 && (*c=='+' || *c==' ') && strlen(c)==2) // +A
			{
				char check;
				c++;
				i--;

				check=*c;

				/*
				if(check>=65 && check<=90)
				check+=32; // A->a
				*/

				if(check==key)
					return this;
			}
			else
			{
				if(i>=2 && (*c=='+' || *c==' ')) // + String
				{
					c++;

					switch(key)
					{
					case KEY_CURSORUP:
						{
							if(strcmp(c,Cxs[CXS_CURSORUP])==0)
							{
								return this;
							}
						}
						break;

					case KEY_CURSORDOWN:
						{
							if(strcmp(c,Cxs[CXS_CURSORDOWN])==0)
							{
								return this;
							}
						}
						break;
					}
				}
			}

		}
		else // SHIFT + key
			if( i>=7 && shift==true &&
				(*c=='S' || *c=='s') &&
				(*(c+1)=='H' || *(c+1)=='h') &&
				(*(c+2)=='I' || *(c+2)=='i') &&
				(*(c+3)=='F' || *(c+3)=='f') &&
				(*(c+4)=='T' || *(c+4)=='t')
				)
			{
				c+=5;
				i-=5;

				if(i>=2 && (*c=='+' || *c==' ') )
				{
					char check;
					c++;
					i--;

					check=*c;

					/*
					if(check>=65 && check<=90)
					check+=32; // A->a
					*/

					if(check==key)
						return this;
				}
			}
			else
				if(i==1 && shift==false && ctrl==false)
				{
					if(*c==key)
						return this;
				}
				else
					if(i==2)
					{
						if(shift==false && ctrl==false)
							if(*c=='F' || *c=='f') // F1-F8 ?

								switch(key)
							{
								case KEYF1:
									if(*(c+1)=='1')
										return this;
									break;

								case KEYF2:
									if(*(c+1)=='2')
										return this;
									break;

								case KEYF3:
									if(*(c+1)=='3')
										return this;
									break;

								case KEYF4:
									if(*(c+1)=='4')
										return this;
									break;

								case KEYF5:
									if(*(c+1)=='5')
										return this;
									break;	

								case KEYF6:
									if(*(c+1)=='6')
										return this;
									break;		

								case KEYF7:
									if(*(c+1)=='7')
										return this;
									break;		

								case KEYF8:
									if(*(c+1)=='8')
										return this;
									break;

								case KEYF9:
									if(*(c+1)=='9')
										return this;
									break;

							}
					}
					else
						if(i==3)
						{
							if(shift==false && ctrl==false &&
								key==KEYDELETE && // DEL
								(*c=='D' || *c=='d') &&
								(*(c+1)=='E' || *(c+1)=='e') &&
								(*(c+2)=='L' || *(c+2)=='l')
								)
							{
								return this;
							}

							if(shift==false && ctrl==false)
							{
								if(*c=='F' || *c=='f') // F1-F10 ?
									switch(key)
								{
									case KEYF10:
										if(*(c+1)=='1' && *(c+2)=='0')
											return this;
										break;

									case KEYF11:
										if(*(c+1)=='1' &&*(c+2)=='1')
											return this;
										break;

									case KEYF12:
										if(*(c+1)=='1' && *(c+2)=='2')
											return this;
										break;

								}
							}

						}	

	}

	return 0;
}

