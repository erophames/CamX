#include "songmain.h"
#include "editor.h"
#include "editor_event.h"
#include "version.h"
#include "editsettings.h"

#include "object_project.h"
#include "object_song.h"
#include "chunks.h"
#include "camxgadgets.h"
#include "audiomixeditor.h"
#include "sampleeditor.h"
#include "audiomanager.h"
#include "arrangeeditor.h"
#include "transporteditor.h"
#include "waveeditor.h"
#include "pianoeditor.h"
#include "groove.h"
#include "editdata.h"
#include "MIDIhardware.h"
#include "quantizeeditor.h"
#include "audiomaster.h"
#include "editMIDIfilter.h"
#include "vstguiwindow.h"
#include "tempomapeditor.h"
#include "editor_help.h"
#include "vstplugins.h"
#include "stepeditor.h"
#include "keyboard.h"
#include "edit_audiointern.h"
#include "editor_text.h"
#include "editor_marker.h"
#include "wavemap.h"
#include "groupeditor.h"
#include "edit_processor.h"
#include "player.h"
#include "bigtime.h"
#include "score.h"
#include "rmg.h"
#include "editwin32audio.h"
#include "library.h"
#include "editor_monitor.h"
#include "edit_createtracks.h"
#include "editbuffer.h"
#include "audiohardware.h"
#include "mainhelpthread.h"
#include "languagefiles.h"
#include "camxinfo.h"
#include "drumeditor.h"
#include "semapores.h"
#include "editcrossfade.h"
#include "vstplugins.h"
#include "editaudiofx.h"
#include "track_effects.h"
#include "editfunctions.h"
#include "player.h"
#include "audiopattern.h"
#include "editor_cpu.h"
#include "globalmenus.h"
#include "MIDIfile.h"
#include "MIDIoutproc.h"
#include "MIDIinproc.h"
#include "arrangeeditor_fx.h"
#include "audiofilework.h"

#ifdef WIN32
#include <afxwin.h>
#include "resource.h"

extern UINT uDragMsg;

#endif

#define MIN_SCREENWIDTH 620
#define MIN_SCREENHEIGHT 545
#define MIN_WINDOWWIDTH 300
#define MIN_WINDOWHEIGHT 200

#ifdef WIN32

#define StartChildrenNo 994

extern char *channelchannelsinfo[],*channelchannelsinfo_short[];
extern int channelschannelsnumber[];


//Screen 
LRESULT CALLBACK CheckOSScreenMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	guiScreen *screen=maingui->FindScreen(hWnd);

	switch(message)
	{
	case WM_COMMAND: // Screen Menu
		{
			WORD hiParam = HIWORD(wParam); // notification code
			WORD loParam = LOWORD(wParam);     // item, control, or accelerator identifier 
			HWND hwndCtl = (HWND) lParam;      // handle of control, NULL=menu

			//TRACE ("WM_COMAND hiParm %d hiParam %d\n", hiParam,loParam);

			switch(hiParam)
			{
				// If the message is from an accelerator, this parameter is 1. If the message is from a menu, this parameter is 0. 
			case 0:
				{
					// Menu
					maingui->CheckScreenMenu(maingui->FindScreen(hWnd),loParam);

					//maingui->CheckCommand(0,hiParam,loParam,hwndCtl);
					return 0; //An application should return zero if it processes this message. 
				}
				break;

			default:
				{
				}
				break;
			}

			return 0;
		}
		break;

	case WM_TIMER:
		{
			maingui->RefreshRealtime();
		}
		break;

	case WM_KEYDOWN: // Screen
		{
			int nVirtKey = (UBYTE) wParam;    // virtual-key code
			int lKeyData = (int)lParam;          // key data

			switch(nVirtKey)
			{
			case VK_SHIFT: // shift
				//	maingui->GetShiftKey()=true;
				break;

			case VK_CONTROL: // strg
				//	maingui->GetCtrlKey()=true;
				break;

			default:
				{
					// 30	The previous key state. The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
					if(screen && screen->menu)
					{
						int sendbefore=lKeyData&(1<<30);

						if(sendbefore==0)
						{
							if(guiMenu *main=maingui->CheckMenuOrPopup(0,0,screen->menu,nVirtKey,maingui->GetShiftKey(),maingui->GetCtrlKey()))
							{
								maingui->CheckOSMenu(screen->menu,main->id);
							}
						}
					}
				}
				break;
			}

			return 0;
		}
		break;

	case WM_SIZE: // Screen
		{
			WPARAM fwSizeType = wParam;      // resizing flag 
			int nWidth = LOWORD(lParam);  // width of client area 
			int nHeight = HIWORD(lParam); // height of client area

			if(screen)
			{
				screen->SetFormSizeFlags(fwSizeType,nWidth,nHeight);

				if(screen->width!=nWidth || screen->height!=nHeight)
				{
					screen->OnNewSize(nWidth,nHeight);
				}
				return 0;
			}

			//TRACE ("Screen Size %d %d\n",nWidth,nHeight);
		}
		break;

	case WM_GETMINMAXINFO:
		if(screen)
		{
			screen->InitMaxInfo(lParam);
			return 0;
		}
		break;

		/*
		case WM_GETMINMAXINFO:
		{
		MINMAXINFO *info=(MINMAXINFO *)lParam;

		info->ptMinTrackSize.x=MIN_SCREENWIDTH;
		info->ptMinTrackSize.y=MIN_SCREENHEIGHT;

		return 0;
		}
		break;
		*/

	case WM_MOVE: //Screen
		{
			int xPos = (int)(short) LOWORD(lParam);   // horizontal position 
			int yPos = (int)(short) HIWORD(lParam);   // vertical position 

			if(screen)
			{
				screen->SetFormXY(xPos,yPos);
				return 0;
			}

			//	TRACE ("Screen Position %d %d\n",xPos,yPos);
		}
		break;

	case WM_CREATE: // Screen
		{
			LPCREATESTRUCT cs=(LPCREATESTRUCT)lParam;

			if(cs)
			{
				guiScreen *screen=(guiScreen *)cs->lpCreateParams;

				if(screen)
				{
					// Frames Child Win
					CLIENTCREATESTRUCT ccs; 

					// Retrieve the handle to the Window menu and assign the 
					// first child window identifier. 

					screen->hWnd=hWnd;
					screen->hInst=maingui->hInst;
					ccs.hWindowMenu = GetSubMenu(GetMenu(hWnd), 2); 
					ccs.idFirstChild = StartChildrenNo; 

					/*
					// Create the MDI client window. 
					maingui->GetActiveScreen()->hWnd = CreateWindowEx
					(
					0,
					"mdiclient", 
					(LPCTSTR) NULL,
					WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
					0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
					//0,0,200,200,
					hWnd, 
					0,
					GetModuleHandle(NULL), 
					(LPSTR) &ccs); 
					*/

					screen->OnCreate();

					//	 ShowWindow(maingui->GetActiveScreen()->screen.hwndMDIClient, SW_SHOW); 

					//if( !maingui->GetActiveScreen()->hWndClient)
					//	maingui->MessageBoxError(0,"Error:Cant create MDI Client");
				}
				else
					maingui->MessageBoxError(0,"Screen #");

				return 0;
			}

		}
		break;

	case WM_CLOSE:  // Close CamX

		if(screen)
		{
			maingui->CloseScreen(screen);
			return 0;
		}
		break;

		/*
		case WM_DESTROY: // Screen
		if(guiScreen *screen=maingui->GetActiveScreen())
		{
		if(screen->hWnd==hWnd)
		{
		screen->hWnd=0;
		maingui->DeleteScreen(screen);
		PostQuitMessage(0);
		}
		}
		return 0;
		*/


	case WM_QUERYENDSESSION:
		if(maingui->GetActiveScreen())
		{
			if(maingui->GetActiveScreen()->hWnd==hWnd)
				return true;
		}
		break;

	case WM_ENDSESSION:
		if(maingui->GetActiveScreen() && maingui->GetActiveScreen()->hWnd==hWnd)
		{
			bool fEndSession = (bool) wParam;     // end-session flag 
			LPARAM fLogOff =  lParam;               // logoff flag

			if(fEndSession==true)
				mainvar->SetExitProgram();
		}
		break;

	case  WM_INITMENUPOPUP :
		/*
		if(screen)
		{
		TRACE ("Screen WM_INITMENUPOPUP  \n");
		maingui->CreateScreenMenu(screen,0);
		return 0;
		}
		*/
		break;

	case WM_MOUSEWHEEL: // screen
		{
			int fwKeys = GET_KEYSTATE_WPARAM(wParam);
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

			zDelta=-zDelta;
			maingui->MouseWheelCheck(zDelta<0?-1:1,0,0);

			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CheckOSWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,bool mdi,guiWindow *win)
{
	switch(message)
	{
	case WM_UNINITMENUPOPUP:
		{
			//if(win)
			//SetFocus(win->hWnd);
		}
		break;

	case WM_COMMAND: // buttons, menus
		{
			WORD hiParam = HIWORD(wParam); // notification code
			WORD loParam = LOWORD(wParam);     // item, control, or accelerator identifier 
			HWND hwndCtl = (HWND) lParam;      // handle of control, NULL=menu

			//TRACE ("Win WM_COMAND hiParm %d hiParam %d\n", hiParam,loParam);

			switch(hiParam)
			{
				// If the message is from an accelerator, this parameter is 1. If the message is from a menu, this parameter is 0. 
			case 0:
				{
					// Menu
					if(maingui->CheckCommand(win,hiParam,loParam,hwndCtl)==true)
						return 0; //An application should return zero if it processes this message. 
				}
				break;

			default:
				{
					// Button...
					if(hwndCtl)
					{
						//TRACE ("Button\n");
						maingui->CheckCommand(win,hiParam,loParam,hwndCtl);
						return 0;
					}
				}
				break;
			}

			return 0;

		}
		break;

	case WM_NOTIFY:
		{
			//TRACE ("WM_NOTIFY %d\n",((LPNMHDR)lParam)->code);

			switch (((LPNMHDR)lParam)->code)
			{
			case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

					//	TRACE ("LVN_ITEMCHANGED  \n");

					guiGadget_ListView *glv=(guiGadget_ListView *)win->glist.gadgets[pnmv->hdr.idFrom];

					if(pnmv->uNewState&LVIS_SELECTED)
					{
						glv->index=pnmv->iItem;
						win->Gadget(glv);
					}
				}
				break;

			case NM_CLICK:
				{
					LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;

					guiGadget_ListView *glv=(guiGadget_ListView *)win->glist.gadgets[lpnmitem->hdr.idFrom];

					TRACE ("lpnmitem Item %d\n",lpnmitem->iItem);
					TRACE ("lpnmitem Item %d\n",lpnmitem->iSubItem);

					if(win->GadgetListView(glv,lpnmitem->iSubItem,lpnmitem->iItem)==true)
					{
						ListView_RedrawItems(glv->hWnd,lpnmitem->iItem,lpnmitem->iItem);
					}
				}
				break;

			case LVN_ITEMACTIVATE:
				TRACE ("LVN_ITEMACTIVATE  \n");
				break;

			case LVN_BEGINLABELEDITA:
				TRACE ("LVN_BEGINLABELEDITA  \n");
				break;

			case LVN_BEGINLABELEDITW:
				TRACE ("LVN_BEGINLABELEDITW  \n");
				break;


			case LVN_COLUMNCLICK:
				{
					TRACE ("LVN_COLUMNCLICK  \n");
				}
				break;

			case LVN_GETDISPINFO: // ListView Refresh
				{
					NMLVDISPINFO* plvdi = (NMLVDISPINFO*)lParam;

					guiGadget_ListView *glv=(guiGadget_ListView *)win->glist.gadgets[plvdi->hdr.idFrom];

					guiListViewColum *col=(guiListViewColum *)glv->column.GetO(plvdi->item.iSubItem);

					//TRACE ("Item %d Colum %d \n",plvdi->item.iItem,plvdi->item.iSubItem);

					if(col)
					{
						guiListViewText *text=(guiListViewText *)col->objects.GetO(plvdi->item.iItem);

						if(text)
						{
							plvdi->item.pszText=text->string;
						}
					}

					//	SendMessage(glv->hWnd,LVM_SETTEXTCOLOR,0,colour_ref[COLOUR_TEXTCONTROL]);
					//	SendMessage(glv->hWnd, LVM_SETTEXTBKCOLOR,0,colour_ref[COLOUR_GADGETBACKGROUNDLISTBOX]);
				}
				break;

			case LVN_BEGINDRAG:
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam; 

					maingui->InitDragDropObject(win,win->GetDragDrop(pnmv->hdr.hwndFrom,pnmv->iItem));

					// maingui->dragdropobject=

					TRACE ("LVN_BEGINDRAG \n");
				}
				break;
			}
		}
		break;

	case WM_KEYUP:
		{
			int nVirtKey = (UBYTE) wParam;    // virtual-key code
			int lKeyData = (int)lParam;          // key data

			switch(nVirtKey)
			{
			case VK_SHIFT: // shift
				//	maingui->GetShiftKey()=false;
				break;

			case VK_CONTROL:// strg
				//	maingui->GetCtrlKey()=false;
				break;

			default:
				{
					win->nVirtKey=nVirtKey;
					maingui->CheckKeyUp(win);
				}
				break;
			}
			return 0;
		}
		break;

	case WM_KEYDOWN: // Win
		{
			int nVirtKey = (UBYTE) wParam;    // virtual-key code
			int lKeyData = (int)lParam;          // key data

			switch(nVirtKey)
			{
			case VK_SHIFT: // shift
				//	maingui->GetShiftKey()=true;
				break;

			case VK_CONTROL: // strg
				//	maingui->GetCtrlKey()=true;
				break;

			default:
				{
					// 30	The previous key state. The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
					int sendbefore=lKeyData&(1<<30);

					if(sendbefore==0)
						win->repeatkey=false;
					else
						win->repeatkey=true;

					win->nVirtKey=nVirtKey;
					maingui->CheckKeyDown(win,0);
				}
				break;
			}

			return 0;
		}
		break;

	case WM_RBUTTONDOWN: // Win
		TRACE ("RBUTTON DOWN\n");
		{
			if(maingui->dragdropobject)
				maingui->CancelDragDrop();
			else
			{
				WPARAM fwKeys = wParam;        // key flags 
				int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
				int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor 

				/*
				win->GetMouseX=xPos;
				win->GetMouseY()=yPos;
				*/

				// Set Keys
				//	maingui->GetShiftKey()=fwKeys&MK_SHIFT?true:false;		
				//	maingui->GetCtrlKey()=fwKeys&MK_CONTROL?true:false;

				//win->UserEdit();
				maingui->RightMouseButtonDown(win);
			}

			return 0;
		}
		break;

	case WM_RBUTTONUP:
		TRACE ("RBUTTON UP\n");
		{
			WPARAM fwKeys = wParam;        // key flags 
			int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
			int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor

			/*
			win->GetMouseX=xPos;
			win->GetMouseY()=yPos;
			*/

			maingui->RightMouseButtonUp(win);
			return 0;
		}
		break;

	case WM_XBUTTONDOWN: //win
		{
			TRACE ("WM_XBUTTONDOWN \n");
			WPARAM fwKeys = GET_KEYSTATE_WPARAM (wParam); 
			WPARAM fwButton = GET_XBUTTON_WPARAM (wParam);

			TRACE ("Button %d\n",fwButton);

			switch(fwButton)
			{
			case 1:
				win->xbuttoncounter=-1;
				win->mousexbuttonleftdown=true;
				break;

			case 2:
				win->xbuttoncounter=-1;
				win->mousexbuttonrightdown=true;
				break;
			}

			return true;
		}
		break;

	case WM_XBUTTONUP: //win
		{
			TRACE ("WM_XBUTTONUP \n");
			WPARAM fwKeys = GET_KEYSTATE_WPARAM (wParam); 
			WPARAM fwButton = GET_XBUTTON_WPARAM (wParam);

			TRACE ("Button %d\n",fwButton);

			switch(fwButton)
			{
			case 1:
				win->mousexbuttonleftdown=false;
				break;

			case 2:
				win->mousexbuttonrightdown=false;
				break;
			}

			return true;
		}
		break;

	case WM_MBUTTONDOWN:
		{
			TRACE ("WM_MBUTTONDOWN \n");
			WPARAM fwKeys = wParam;        // key flags

			return true;
		}
		break;

	case WM_LBUTTONDOWN:
		TRACE ("LBUTTON DOWN\n");
		{
			WPARAM fwKeys = wParam;        // key flags 
			int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
			int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor 

			// Set Keys
			//	maingui->GetShiftKey()=fwKeys&MK_SHIFT?true:false;
			//	maingui->GetCtrlKey()=fwKeys&MK_CONTROL?true:false;

			//win->UserEdit();
			maingui->LeftMouseButtonDown(win);
			return true;
		}
		break;

	case WM_LBUTTONUP:

		TRACE ("LBUTTON UP\n");
		{
			WPARAM fwKeys = wParam;        // key flags 
			int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
			int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor

			maingui->LeftMouseButtonUp(win);
			return 0;
		}
		break;

	case WM_LBUTTONDBLCLK:
		{
			WPARAM fwKeys = wParam;        // key flags 
			int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
			int yPos =  GET_Y_LPARAM(lParam);  // vertical position of cursor

			win->ResetRefresh();

			win->left_mousekey=MOUSEKEY_DBCLICK_LEFT;
			win->autoscroll=false;
			win->autoscrollmode=0;

			maingui->CheckMouseDown(win,MOUSEKEY_DBCLICK_LEFT);
			return 0;
		}
		break;

	case WM_NCLBUTTONUP: // drag&drop

		// mouse click outside window

		// MessageBeep(-1);


		//if(win=maingui->ConvertSystemWindowToGUI(hWnd))
		//{
		//int fwKeys = wParam;        // key flags 
		//int xPos = LOWORD(lParam);  // horizontal position of cursor 
		//int yPos = HIWORD(lParam);  // vertical position of cursor 

		//win->left_mousekey=MOUSEKEY_UP;
		//win->mouseinside=false;

		//maingui->CheckMouseUp(win,0,0);
		//}
		//}
		break;
	}

	return /*mdi==true?DefMDIChildProc(hWnd, message, wParam, lParam):*/ DefWindowProc(hWnd, message, wParam, lParam);
}


LRESULT CALLBACK ChildWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CREATE: // DB Window
		{
			LPCREATESTRUCT cs=(LPCREATESTRUCT)lParam;

			guiGadget *db=(guiGadget *)cs->lpCreateParams;

			//	TRACE (" ################# WM_CREATE DB HWnd=%d ID%d \n",hWnd,db->gadgetindex);		
			if(db)
				db->guilist->win->CreateButton(db->gadgetindex,hWnd);
			return 0;
		}
		break;
	}

	guiWindow *win=maingui->FirstWindow();
	while(win){

		for(int i=0;i<win->glist.gcwc;i++)
			if(win->glist.g_cw[i]->hWnd==hWnd){

				guiGadget *cw=win->glist.g_cw[i];

				switch(message)
				{
				case WM_SIZE: // DB
					{
						//db->InitNewDoubleBuffer();
						//db->DrawGadgetEx();
						return 0;
					}
					break;

				case WM_PAINT: // DB
					{
						RECT rectUpd;
						if(GetUpdateRect (hWnd, &rectUpd, FALSE))
						{
							guiBitmap *source=cw->mode&MODE_SPRITE?&cw->mixbitmap:&cw->gbitmap;
							PAINTSTRUCT ps;
							HDC hDC = BeginPaint(hWnd, &ps);

							if(source->hDC)
								BitBlt(
								hDC, // handle to destination device context
								ps.rcPaint.left,  // x-coordinate of destination rectangle's upper-left 
								ps.rcPaint.top,  // y-coordinate of destination rectangle's upper-left 
								cw->GetWidth(), // (ps.rcPaint.right-ps.rcPaint.left)+1*/,  // width of destination rectangle
								cw->GetHeight(), //(ps.rcPaint.bottom-ps.rcPaint.top)+1, // height of destination rectangle
								source->hDC,  // handle to source device context
								ps.rcPaint.left,   // x-coordinate of source rectangle's upper-left 
								ps.rcPaint.top,   // y-coordinate of source rectangle's upper-left 
								SRCCOPY  // raster operation code
								);

							EndPaint(hWnd,&ps);
						}

						return 0;
					}
					break;

				case WM_MOUSEHWHEEL: // db
					{
						int fwKeys = GET_KEYSTATE_WPARAM(wParam);
						int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);


						return 0;
					}
					break;

				case WM_MOUSEWHEEL: // DB
					{
						int fwKeys = GET_KEYSTATE_WPARAM(wParam);
						int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

						zDelta=-zDelta;

						maingui->MouseWheelCheck(zDelta<0?-1:1,win,cw);

						return 0;
					}
					break;

				case WM_LBUTTONDOWN:
					{
						if(!win->mouseclickdowncounter)
						{
							int xPos = GET_X_LPARAM(lParam); 
							int yPos = GET_Y_LPARAM(lParam); 

							cw->SetMouse(xPos,yPos);

							SetFocus(hWnd);
							cw->oldfocushWnd=SetCapture(hWnd);
							//SetFocus(hWnd);

							cw->leftmousedown=true;
							win->addtolastundo=false;
							cw->Call(DB_LEFTMOUSEDOWN);
						}

						return 0;
					}
					break;

				case WM_LBUTTONDBLCLK:
					{
						// Child DB Childs
						int xPos = GET_X_LPARAM(lParam); 
						int yPos = GET_Y_LPARAM(lParam); 

						cw->SetMouse(xPos,yPos);

						guiObject *o=win->guiobjects.FirstObject();
						while(o)
						{
							if(guiGadget *g=o->gadget)
							{
								if(g->CheckMouseOver(xPos,yPos)==true)
								{
									win->CheckGadget(g);
									return 0;
								}
							}

							o=o->NextObject();
						}

						cw->Call(DB_DOUBLECLICKLEFT);
						return 0;
					}
					break;

				case WM_LBUTTONUP:
					{
						int xPos = GET_X_LPARAM(lParam); 
						int yPos = GET_Y_LPARAM(lParam); 

						cw->SetMouse(xPos,yPos);

						if(!win->mouseclickdowncounter)
						{
							SetCapture(cw->oldfocushWnd);
							cw->oldfocushWnd=0;

							cw->leftmousedown=false;

							if(maingui->CheckDragDropLeftMouseUp(0,cw)==true)
								cw->Call(DB_LEFTMOUSEUP);
						}

						return 0;
					}
					break;

				case WM_RBUTTONDOWN:
					{
						int xPos = GET_X_LPARAM(lParam); 
						int yPos = GET_Y_LPARAM(lParam); 

						cw->SetMouse(xPos,yPos);

						if(!win->mouseclickdowncounter)
						{
							if(maingui->dragdropobject)
								maingui->CancelDragDrop();
							else
							{
								SetFocus(hWnd);
								cw->oldfocushWnd=SetCapture(hWnd);

								cw->rightmousedown=true;
								win->addtolastundo=false;

								cw->Call(DB_RIGHTMOUSEDOWN);
							}
						}
						return 0;
					}
					break;

				case WM_RBUTTONUP:
					{
						int xPos = GET_X_LPARAM(lParam); 
						int yPos = GET_Y_LPARAM(lParam); 

						cw->SetMouse(xPos,yPos);

						if(!win->mouseclickdowncounter)
						{
							SetCapture(cw->oldfocushWnd);
							cw->oldfocushWnd=0;

							cw->rightmousedown=false;
							cw->Call(DB_RIGHTMOUSEUP);
						}
						return 0;
					}
					break;

				case WM_XBUTTONDOWN: //db
					{
						TRACE ("WM_XBUTTONDOWN DB\n");
						int xPos = GET_X_LPARAM(lParam); 
						int yPos = GET_Y_LPARAM(lParam); 

						cw->SetMouse(xPos,yPos);

						WPARAM fwKeys = GET_KEYSTATE_WPARAM (wParam); 
						WPARAM fwButton = GET_XBUTTON_WPARAM (wParam);

						TRACE ("Button %d\n",fwButton);

						switch(fwButton)
						{
						case 1:
							win->xbuttoncounter=-1;
							win->mousexbuttonleftdown=true;
							break;

						case 2:
							win->xbuttoncounter=-1;
							win->mousexbuttonrightdown=true;
							break;
						}

						return true;
					}
					break;

				case WM_XBUTTONUP: //db
					{
						TRACE ("WM_XBUTTONUP DB\n");
						int xPos = GET_X_LPARAM(lParam); 
						int yPos = GET_Y_LPARAM(lParam); 

						cw->SetMouse(xPos,yPos);

						WPARAM fwKeys = GET_KEYSTATE_WPARAM (wParam); 
						WPARAM fwButton = GET_XBUTTON_WPARAM (wParam);

						TRACE ("Button %d\n",fwButton);

						switch(fwButton)
						{
						case 1:
							win->mousexbuttonleftdown=false;
							break;

						case 2:
							win->mousexbuttonrightdown=false;
							break;
						}

						return true;
					}
					break;

				case WM_MOUSEMOVE: // DB
					{
						int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
						int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor 

						cw->SetMouse(xPos,yPos);

						if(!win->mouseclickdowncounter)
						{
							int status=DB_MOUSEMOVE;

							if(wParam&MK_LBUTTON)
								status|=DB_LEFTMOUSEDOWN;

							if(wParam&MK_RBUTTON)
								status|=DB_RIGHTMOUSEDOWN;

							cw->SetMouse(xPos,yPos);
							cw->Call(status);
						}

						return 0;
						//TRACE ("MouseMove DB %d %d\n",xPos,yPos);
					}
					break;

				case WM_KILLFOCUS: // DB
					{
						cw->Call(DB_KILLFOCUS);
					}
					break;

				case WM_KEYUP:
					{
						int nVirtKey = (UBYTE) wParam;    // virtual-key code
						int lKeyData = (int)lParam;          // key data

						switch(nVirtKey)
						{
						case VK_SHIFT: // shift
							//	maingui->GetShiftKey()=false;
							break;

						case VK_CONTROL:// strg
							//	maingui->GetCtrlKey()=false;
							break;

						default:
							{
								cw->guilist->win->nVirtKey=nVirtKey;
								maingui->CheckKeyUp(cw->guilist->win);
							}
							break;
						}
						return 0;
					}
					break;

				case WM_KEYDOWN: // DB
					{
						int nVirtKey = (UBYTE) wParam;    // virtual-key code
						int lKeyData = (int)lParam;          // key data

						switch(nVirtKey)
						{
						case VK_SHIFT: // shift
							//	maingui->GetShiftKey()=true;
							break;

						case VK_CONTROL: // strg
							//	maingui->GetCtrlKey()=true;
							break;

						default:
							{
								// 30	The previous key state. The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
								int sendbefore=lKeyData&(1<<30);

								cw->guilist->win->repeatkey=sendbefore==0?false:true;
								cw->guilist->win->nVirtKey=nVirtKey;

								maingui->CheckKeyDown(cw->guilist->win,cw);


								/*
								if(db->guilist->win->repeatkey==false)
								{
								if(maingui->CheckHotKey(db->guilist->win,db,nVirtKey))
								return 0;
								}

								db->vkey=nVirtKey;
								db->Call(DB_KEYDOWN);
								*/
							}
							break;
						}

						return 0;
					}
					break;
				}

				break;
			}

			win=win->NextWindow();
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK SizerHProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	guiForm_Child *form=0;
	guiWindow *win=maingui->FirstWindow();

	while(win)
	{
		if(win->formcounter>1)
		{
			for(int x=0;x<win->forms_horz;x++)
				for(int y=0;y<win->forms_vert;y++)
					if(win->forms[x][y].sizerhorz_hWnd==hWnd)
					{
						form=&win->forms[x][y];
						goto exit;
					}
		}

		win=win->NextWindow();
	}

exit:
	if(form)
	{
		switch(message)
		{
		case WM_LBUTTONDOWN:
			{
				//WPARAM fwKeys = wParam;        // key flags 
				//int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
				//int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor 

				// Set Keys
				//maingui->GetShiftKey()=fwKeys&MK_SHIFT?true:false;
				//maingui->GetCtrlKey()=fwKeys&MK_CONTROL?true:false;

				TRACE ("LBUTTON SIZER V DOWN\n");

				form->userselect=true;

				int mx,my;
				maingui->GetMouseOnScreen(&mx,&my);

				form->sizediffx=form->sizediffy=0;

				form->sizerstartx=mx;
				form->sizerstarty=my;

				form->oldcapturehWnd=SetCapture(form->sizerhorz_hWnd);

				return true;
			}
			break;

		case WM_NCLBUTTONUP:
		case WM_LBUTTONUP:
			{
				//	WPARAM fwKeys = wParam;        // key flags 
				//	int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
				//	int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor 

				// Set Keys
				//	maingui->GetShiftKey()=fwKeys&MK_SHIFT?true:false;
				//	maingui->GetCtrlKey()=fwKeys&MK_CONTROL?true:false;

				if(form->userselect==true)
				{
					TRACE ("LBUTTON SIZER V Release\n");
					form->userselect=false;
					form->sizediffx=0;
					SetCapture(form->oldcapturehWnd);
				}
				return true;
			}
			break;

		case WM_MOUSEMOVE: // FORM X
			if(form->userselect==true)
			{
				int mx,my;
				maingui->GetMouseOnScreen(&mx,&my);
				//int my=lpPoint.y;

				if(mx!=form->sizerstartx)
				{
					form->sizediffx=mx-form->sizerstartx;
					form->sizerstartx=mx;

					win->MoveForm();
				}
			}

			return 0;
			break;
		}	
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK VSTwProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK SizerVProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	guiForm_Child *form=0;
	guiScreen *screen=0;
	guiWindow *win=maingui->FirstWindow();

	// 1. Windows
	while(win)
	{
		if(win->formcounter>1)
		{
			for(int x=0;x<win->forms_horz;x++)
				for(int y=0;y<win->forms_vert;y++)
					if(win->forms[x][y].sizervert_hWnd==hWnd)
					{
						form=&win->forms[x][y];
						goto exit;
					}
		}

		win=win->NextWindow();
	}

	// 2. Screen
	screen=maingui->FirstScreen();
	while(screen)
	{
		//if(screen->formcounter>1)
		{
			for(int x=0;x<screen->forms_horz;x++)
				for(int y=0;y<screen->forms_vert;y++)
					if(screen->forms[x][y].sizervert_hWnd==hWnd)
					{
						form=&screen->forms[x][y];
						goto exit;
					}
		}

		screen=screen->NextScreen();
	}

exit:
	if(form)
	{
		switch(message)
		{
		case WM_LBUTTONDBLCLK:
			{
				if(win)
					win->ToggleForm(form,maingui->GetShiftKey()==true?false:true);
				else
					screen->ToggleForm(form,maingui->GetShiftKey()==true?false:true);
			}
			break;

		case WM_RBUTTONDBLCLK:
			{
				if(win)
					win->ToggleForm(form,false);
				else
					screen->ToggleForm(form,false);
			}
			break;

		case WM_LBUTTONDOWN:
			{
				//WPARAM fwKeys = wParam;        // key flags 
				//int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
				//int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor 

				// Set Keys
				//maingui->GetShiftKey()=fwKeys&MK_SHIFT?true:false;
				//maingui->GetCtrlKey()=fwKeys&MK_CONTROL?true:false;

				TRACE ("LBUTTON SIZER V DOWN\n");

				form->sizediffx=form->sizediffy=0;

				form->userselect=true;

				maingui->GetMouseOnScreen(&form->sizerstartx,&form->sizerstarty);

				form->oldcapturehWnd=SetCapture(form->sizervert_hWnd);

				return true;
			}
			break;

		case WM_NCLBUTTONUP:
		case WM_LBUTTONUP:
			{
				//	WPARAM fwKeys = wParam;        // key flags 
				//	int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
				//	int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor 

				// Set Keys
				//	maingui->GetShiftKey()=fwKeys&MK_SHIFT?true:false;
				//	maingui->GetCtrlKey()=fwKeys&MK_CONTROL?true:false;

				if(form->userselect==true)
				{
					TRACE ("LBUTTON SIZER V Release\n");
					form->userselect=false;
					form->sizediffy=0;
					SetCapture(form->oldcapturehWnd);
				}
				return true;
			}
			break;

		case WM_MOUSEMOVE: // FORM Y
			if(form->userselect==true)
			{
				int mx,my;
				maingui->GetMouseOnScreen(&mx,&my);

				if(my!=form->sizerstarty)
				{
					form->sizediffy=my-form->sizerstarty;
					form->sizerstarty=my;

					if(win)
						win->MoveForm();
					else
						screen->MoveForm();
				}				
			}

			return 0;
			break;
		}	
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CREATE: // Window
		{
			if(LPCREATESTRUCT cs=(LPCREATESTRUCT)lParam)
			{
				guiWindow *win=(guiWindow *)cs->lpCreateParams;

				//	TRACE ("WM_CREATE WndProc %d\n",win);

				if(win)
				{
					win->hWnd=win->glist.hWnd=hWnd; // Init Window + Default Form HANDLE

					win->width=cs->cx;
					win->height=cs->cy;
					win->OnCreate();
				}

				return 0;
			}
		}
		break;
	}

	guiWindow *win=maingui->FirstWindow();
	guiForm_Child *child=0;

	while(win)
	{
		if(win->hWnd==hWnd) // Parent 
			goto check;

		//if(win->formcounter)
		{
			for(int x=0;x<win->forms_horz;x++)
				for(int y=0;y<win->forms_vert;y++)
					if(win->forms[x][y].deactivated==false && win->forms[x][y].fhWnd==hWnd)
					{
						child=&win->forms[x][y];
						goto check;
					}
		}

		win=win->NextWindow();
	}

check:
	if(win)
	{
		switch(message)
		{
		case WM_DRAWITEM:
			{
				LPDRAWITEMSTRUCT lp=(LPDRAWITEMSTRUCT) lParam;

				//TRACE ("WM_DRAWITEM Type %d %d\n",lp->CtlType,lp->CtlID);

				switch(lp->CtlType)
				{
				case ODT_BUTTON:
					{
						switch(lp->itemAction)
						{
						case ODA_DRAWENTIRE:
							win->OwnerDrawGadget((int)wParam,lp);
							break;

						case ODA_SELECT:
							win->OwnerDrawGadgetSelect((int)wParam,lp);
							break;
						}
					}
					break;
				}
				return TRUE;
			}
			break;

		case WM_PARENTNOTIFY:
			{
				switch(LOWORD(wParam))
				{
				case WM_CREATE: // Button
					{
						int id=HIWORD(wParam);

						//TRACE ("WM_PARENTNOTIFY WM_CREATE Win %s ID=%d\n",win->windowname,id);
						if(id<win->glist.gc)
						{
							win->CreateButton(id,(HWND)lParam);
							return 0;
						}
					}
					break;
				}
			}
			break;

		case WM_DROPFILES:
			if(win && win->WindowSong())
			{
				// DragAcceptFiles(hwnd,true);
				HDROP query = (HDROP) wParam;
				int n = 0, count = DragQueryFile( query, 0xFFFFFFFF, 0, 0 );
				while ( n < count ) {

					//char ret[256];
					char file[1024];

					DragQueryFile( query, n, file, 1024 );

					guiGadget_CW *db=0;

					POINT lpPoint;
					GetCursorPos(&lpPoint);

					{
						HWND Handle=WindowFromPoint(lpPoint);

						for(int i=0;i<win->glist.gc;i++)
						{
							if(win->glist.gadgets[i]->IsDoubleBuffered()==true && win->glist.gadgets[i]->hWnd==Handle)
							{
								db=(guiGadget_CW *)win->glist.gadgets[i];
								break;
							}

						}
					}

					if(db)
					{
						ScreenToClient(db->hWnd,&lpPoint);
						db->SetMouse(lpPoint.x,lpPoint.y);

						if(win->timeline)
							win->WindowSong()->SetMousePosition(win->QuantizeEditorMouse(win->timeline->ConvertXPosToTime(lpPoint.x)));

						win->DragDropFile(file,db);
					}

					TRACE("WM_DROPFILES %s\n",file);

					n++;
				}
				DragFinish( query );
			}
			break;

		case WM_CLOSE: // WINDOW
			if(!child)
			{
				if(win->GetEditorID()==EDITORTYPE_TRANSPORT)
				{
					win->ReleaseMouse();

					// Dont close last Transport Editor !
					guiWindow *w=maingui->FirstWindow();

					while(w){

						if(w!=win && w->GetEditorID()==EDITORTYPE_TRANSPORT)
							break;

						w=w->NextWindow();
					}

					if(!w)
					{
						return 0; // Dont CLOSE
					}
				}
			}
			break;

			/*
			case  WM_INITMENUPOPUP :
			if(win->menu)
			{
			HMENU hmenu=(HMENU)wParam;
			int pos= LOWORD(lParam);
			BOOL sysmenu= (BOOL)HIWORD(lParam);

			if(sysmenu==FALSE)
			{
			guiMenu *menu=win->menu->FindMenu(hmenu);

			if(menu)
			{
			TRACE ("Window WM_INITMENUPOPUP  %d Pos= %d %d\n",hmenu,pos,menu);
			win->CreateWindowMenu();
			maingui->AppendPopUpMenu(menu);
			//DrawMenuBar(win->hWnd);
			}

			//	maingui->ConvertGUIMenuToOSMenu(win,menu);

			return 0;

			}
			}
			break;
			*/

			/*
			case WM_ERASEBKGND:
			{
			//	MessageBeep(-1);

			return 1; // Erase by OS
			}
			break;
			*/

		case WM_MOUSEHWHEEL: // win
			{
				int fwKeys = GET_KEYSTATE_WPARAM(wParam);
				int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				return 0;
			}
			break;

		case WM_MOUSEWHEEL: // win
			{
				int fwKeys = GET_KEYSTATE_WPARAM(wParam);
				int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				zDelta=-zDelta;
				maingui->MouseWheelCheck(zDelta<0?-1:1,win,0);

				return 0;
			}
			break;

		case WM_MOUSEMOVE: // win
			{
				int xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
				int yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor 

				win->SetMouse(xPos,yPos);

				return 0;
			}
			break;

		case WM_MOVE:
			if(!child)
			{
				int xPos = (int)(short) LOWORD(lParam);   // horizontal position 
				int yPos = (int)(short) HIWORD(lParam);   // vertical position 

				win->win_screenposx=xPos;
				win->win_screenposy=yPos;

				win->SetWindowDefaultSize();
			}
			return 0;
			break;

		case WM_SIZE: // Window + Childs
			{
				WPARAM fwSizeType = wParam;      // resizing flag 
				int nWidth = LOWORD(lParam);  // width of client area 
				int nHeight = HIWORD(lParam); // height of client area 

				if(child)
				{
#ifdef DEBUG
					//TRACE ("Win Child WM_SIZE \n");
					if(child->GetWidth()!=nWidth || child->GetHeight()!=nHeight)
					{
						int i=1;
					}
#endif

					return 0;
				}
				else
				{
					win->SetFormSizeFlags(fwSizeType,nWidth,nHeight);

					//ShowWindow(win->clientwnd,SW_SHOW);
					if(win->winmode!=WINDOWMODE_INIT && (win->width!=nWidth || win->height!=nHeight))
					{
						win->OnNewSize(nWidth,nHeight);
						return 0;
					}
				}
			}
			break;

		case WM_QUERYENDSESSION:
			return true;

		case WM_ENDSESSION:
			break;

		case WM_KILLFOCUS: // Win
			{
				win->KillFocus();
				return 0;
			}
			break;

		case WM_SETFOCUS:
			{
			}
			break;

			/*
			case WM_NCMOUSEMOVE:
			{
			win->mouseinside=false;
			maingui->MouseOutsideWindow(win);
			}
			break;
			*/

		case WM_ACTIVATE:
			{
				switch(wParam)
				{
				case WA_ACTIVE:
				case WA_CLICKACTIVE:
					//maingui->SetActiveWindow(win,true);
					break;

				case WA_INACTIVE:
					if(win->closeit==false)
					{
						win->DeActivated();
					}
					break;
				}
			}
			break;

		case WM_USER:
			{
				maingui->CheckUserMessage(win,(int)wParam,(void *)lParam,0);
			}
			break;

		case WM_DESTROY: // Window
			{
				if(!child)
				{
					//	win->close=true;
					maingui->CloseWindow(win,true);
					return 0;
				}
			}
			break;

		case WM_HSCROLL: // Win
			{
				// lParam
				// If the message is sent by a scroll bar control, this parameter is the handle to the scroll bar control. If the message is sent by a standard scroll bar, this parameter is NULL. 

				SCROLLINFO si; 

				si.cbSize = sizeof (si);
				si.fMask  = SIF_ALL;

				if(child && lParam) // Scroller
				{
					// Scroll Gadget 
					HWND HwndScrollBar = (HWND) lParam;

					// Save the position for comparison later on
					GetScrollInfo (HwndScrollBar, SB_CTL, &si);

					int Pos = si.nPos;
					int nScrollCode = (int) LOWORD(wParam); // scroll bar value 
					bool thumb=false;

					switch(nScrollCode)
					{
					case SB_LINELEFT: 
						si.nPos -= 1;
						break;

						// user clicked right arrow
					case SB_LINERIGHT: 
						si.nPos += 1;
						break;

						// user clicked the scroll bar shaft left of the scroll box
					case SB_PAGELEFT:
						si.nPos -= si.nPage;
						break;

						// user clicked the scroll bar shaft right of the scroll box
					case SB_PAGERIGHT:
						si.nPos += si.nPage;
						break;

						// user dragged the scroll box
					case SB_THUMBTRACK:
						{
							//	int pos=HIWORD(wParam);
							si.nPos = si.nTrackPos;
							thumb=true;
						}
						break;

					default :
						break;
					}

					//	if(thumb==false) // no pos check
					{						
						si.fMask = SIF_POS;
						SetScrollInfo (HwndScrollBar, SB_CTL, &si, TRUE);
						GetScrollInfo (HwndScrollBar, SB_CTL, &si);
					}

					// If the position has changed, scroll the window 
					if (thumb==true || si.nPos != Pos)
					{
						for(int i=0;i<win->glist.gc;i++)
						{
							if(win->glist.gadgets[i]->hWnd==HwndScrollBar)
							{
								guiGadget_Slider *slider=(guiGadget_Slider *)win->glist.gadgets[i];
								slider->pos=si.nPos;
								win->Gadget(win->glist.gadgets[i]);
								break;
							}
						}
					}

					return 0;
				}

				{
					GetScrollInfo (hWnd, SB_HORZ, &si);

					int Pos = si.nPos;
					int nScrollCode = (int) LOWORD(wParam); // scroll bar value 
					bool thumb=false;

					switch(nScrollCode)
					{
					case SB_LINELEFT: 
						si.nPos -= 1;
						break;

						// user clicked right arrow
					case SB_LINERIGHT: 
						si.nPos += 1;
						break;

						// user clicked the scroll bar shaft left of the scroll box
					case SB_PAGELEFT:
						si.nPos -= si.nPage;
						break;

						// user clicked the scroll bar shaft right of the scroll box
					case SB_PAGERIGHT:
						si.nPos += si.nPage;
						break;

						// user dragged the scroll box
					case SB_THUMBTRACK:
						{
							si.nPos=HIWORD(wParam);
							//Pos = si.nTrackPos;
							thumb=true;
						}
						break;

					default :
						break;
					}

					//	if(thumb==false)


					{
						si.fMask = SIF_POS;
						SetScrollInfo (hWnd, message==WM_HSCROLL?SB_HORZ:SB_VERT, &si, TRUE);
						GetScrollInfo (hWnd, message==WM_HSCROLL?SB_HORZ:SB_VERT, &si);
					}

					// If the position has changed, scroll the window 

					if (thumb==true || si.nPos != Pos)
					{
						if(child)
						{
						}
						//child->ScrollHorz();
						else
						{
							TRACE ("SCROLL h %d\n",si.nPos);

							win->glist.scrollh_pos=si.nPos;
							win->ScrollHoriz();
						}
					}

					return 0;
				}
			}
			break;

		case WM_VSCROLL:
			{
				SCROLLINFO si;

				si.cbSize = sizeof (si);
				si.fMask  = SIF_ALL;

				if(child && lParam) // Scroller
				{
					// Scroll Gadget 
					HWND HwndScrollBar = (HWND) lParam;

					// Save the position for comparison later on
					GetScrollInfo (HwndScrollBar, SB_CTL, &si);

					int Pos = si.nPos;
					int nScrollCode = (int) LOWORD(wParam); // scroll bar value 
					bool thumb=false;

					switch(nScrollCode)
					{
					case SB_LINEUP: 
						si.nPos -= 1;
						break;

						// user clicked right arrow
					case SB_LINEDOWN: 
						si.nPos += 1;
						break;

						// user clicked the scroll bar shaft left of the scroll box
					case SB_PAGEUP:
						si.nPos -= si.nPage;
						break;

						// user clicked the scroll bar shaft right of the scroll box
					case SB_PAGEDOWN:
						si.nPos += si.nPage;
						break;

						// user dragged the scroll box
					case SB_THUMBTRACK:
						{
							//	int pos=HIWORD(wParam);
							si.nPos = si.nTrackPos;
							//thumb=true;
						}
						break;

					default :
						break;
					}

					if(thumb==false) // no pos check
					{
						si.fMask = SIF_POS;
						SetScrollInfo (HwndScrollBar, SB_CTL, &si, TRUE);
						GetScrollInfo (HwndScrollBar, SB_CTL, &si);
					}

					// If the position has changed, scroll the window 
					if (thumb==true || si.nPos != Pos)
					{
						for(int i=0;i<win->glist.gc;i++)
						{
							if(win->glist.gadgets[i]->hWnd==HwndScrollBar)
							{
								guiGadget_Slider *slider=(guiGadget_Slider *)win->glist.gadgets[i];
								slider->pos=si.nPos;
								win->Gadget(win->glist.gadgets[i]);
								break;
							}
						}
					}

					return 0;
				}

				{
					GetScrollInfo (hWnd,SB_VERT, &si);

					int Pos = si.nPos;
					int nScrollCode = (int) LOWORD(wParam); // scroll bar value 
					bool thumb=false;

					switch(nScrollCode)
					{
					case SB_LINEUP: 
						si.nPos -= 1;
						break;

						// user clicked right arrow
					case SB_LINEDOWN: 
						si.nPos += 1;
						break;

						// user clicked the scroll bar shaft left of the scroll box
					case SB_PAGEUP:
						si.nPos -= si.nPage;
						break;

						// user clicked the scroll bar shaft right of the scroll box
					case SB_PAGEDOWN:
						si.nPos += si.nPage;
						break;

						// user dragged the scroll box
					case SB_THUMBTRACK:
						{
							si.nPos=HIWORD(wParam);
							//Pos = si.nTrackPos;
							thumb=true;
						}
						break;

					default :
						break;
					}

					//	if(thumb==false)


					{
						si.fMask = SIF_POS;
						SetScrollInfo (hWnd, SB_VERT, &si, TRUE);
						GetScrollInfo (hWnd, SB_VERT, &si);
					}

					// If the position has changed, scroll the window 

					if (thumb==true || si.nPos != Pos)
					{
						if(child)
						{
						}
						//child->ScrollHorz();
						else
						{
							int deltay=si.nPos-win->glist.scrollv_pos;

							win->glist.scrollv_pos=si.nPos;
							win->ScrollVert(deltay);
						}
					}

					return 0;
				}
			}
			break;

			/*
			case WM_VSCROLL:
			// Get all the vertial scroll bar information
			si.cbSize = sizeof (si);
			si.fMask  = SIF_ALL;
			GetScrollInfo (hwnd, SB_VERT, &si);
			// Save the position for comparison later on
			yPos = si.nPos;
			switch (LOWORD (wParam))
			{
			// user clicked the HOME keyboard key
			case SB_TOP:
			si.nPos = si.nMin;
			break;

			// user clicked the END keyboard key
			case SB_BOTTOM:
			si.nPos = si.nMax;
			break;

			// user clicked the top arrow
			case SB_LINEUP:
			si.nPos -= 1;
			break;

			// user clicked the bottom arrow
			case SB_LINEDOWN:
			si.nPos += 1;
			break;

			// user clicked the scroll bar shaft above the scroll box
			case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;

			// user clicked the scroll bar shaft below the scroll box
			case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;

			// user dragged the scroll box
			case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

			default:
			break; 
			}
			// Set the position and then retrieve it.  Due to adjustments
			//   by Windows it may not be the same as the value set.
			si.fMask = SIF_POS;
			SetScrollInfo (hwnd, SB_VERT, &si, TRUE);
			GetScrollInfo (hwnd, SB_VERT, &si);
			// If the position has changed, scroll window and update it
			if (si.nPos != yPos)
			{                    
			ScrollWindow(hwnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
			UpdateWindow (hwnd);
			}
			return 0;
			*/

		case WM_GETMINMAXINFO:
			if(!child)
			{
				win->InitMaxInfo(lParam);
				return 0;
			}
			break;

		case WM_CTLCOLORLISTBOX:
			{
				/*
				HDC hdcStatic = (HDC) wParam;
				SetTextColor(hdcStatic, colour_ref[COLOUR_TEXTCONTROL]);
				SetBkColor(hdcStatic, colour_ref[COLOUR_GADGETBACKGROUNDLISTBOX]);

				return (LRESULT)colour_hBrush[COLOUR_GADGETBACKGROUNDLISTBOX];
				*/
			}
			break;

		case WM_CTLCOLOREDIT:
			{
				//Integer,Strings...
				HDC hdcStatic = (HDC) wParam;
				SetTextColor(hdcStatic, colour_ref[COLOUR_TEXTCONTROL]);
				SetBkColor(hdcStatic, colour_ref[COLOUR_GADGETBACKGROUNDSTRINGINT]);

				return (LRESULT)colour_hBrush[COLOUR_GADGETBACKGROUNDEDIT];
			}
			break;

		case WM_CTLCOLORSTATIC:
			{
				//Integer,Strings...
				HDC hdcStatic = (HDC) wParam;
				SetTextColor(hdcStatic, colour_ref[COLOUR_TEXTCONTROL]);
				SetBkColor(hdcStatic, colour_ref[COLOUR_GADGETBACKGROUNDEDIT]);

				return (LRESULT)colour_hBrush[COLOUR_GADGETBACKGROUNDEDIT];
			}
			break;

		case WM_CTLCOLORSCROLLBAR:
			{
				HDC hdcStatic = (HDC) wParam;

				SetTextColor(hdcStatic, colour_ref[COLOUR_RED]);
				SetBkColor(hdcStatic, colour_ref[COLOUR_BLUE]);
				return (LRESULT)colour_hBrush[COLOUR_GADGETBACKGROUNDEDIT];
			}
			break;

		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORDLG:
			{
				HDC hdcStatic = (HDC) wParam;
				SetTextColor(hdcStatic, colour_ref[COLOUR_TEXTCONTROL]);
				SetBkColor(hdcStatic, colour_ref[COLOUR_GADGETBACKGROUNDEDIT]);

				return (LRESULT)colour_hBrush[COLOUR_BACKGROUNDWINDOWS];
			}
			break;

		case WM_CTLCOLORBTN:
			{
				HDC hdcStatic = (HDC) wParam;

				SetTextColor(hdcStatic, colour_ref[COLOUR_TEXTCONTROL]);
				SetBkColor(hdcStatic, colour_ref[COLOUR_GADGETBACKGROUNDEDIT]);

				return (LRESULT)colour_hBrush[COLOUR_BACKGROUNDWINDOWS];
			}
			break;

		default:
			// cannot use it in the big switch/case of course...
			if(message == (UINT)uDragMsg)
			{
				LPDRAGLISTINFO lp = (LPDRAGLISTINFO)lParam;
				switch(lp->uNotification)
				{
				case DL_BEGINDRAG:
					{
						int curselect = LBItemFromPt(lp->hWnd,lp->ptCursor,FALSE);
						if(curselect!=-1)
						{
							maingui->InitDragDropObject(win,win->GetDragDrop(lp->hWnd,curselect));
						}

						TRACE ("DL_BEGINDRAG\n");
					}
					return TRUE; 

				case DL_DRAGGING:
					{
						//	int nListID = LBItemFromPt(lp->hWnd, lp->ptCursor, FALSE);
						// DrawInsert(hWnd, lp->hWnd, DL_COPYCURSOR);
					}
					return DL_COPYCURSOR;

				case DL_CANCELDRAG:
					maingui->dragdropobject=0;
					break;

				case DL_DROPPED:
					{

					}
					break; 
				}
				break;
			}
			break;

		}

		return CheckOSWindowMessage(hWnd,message,wParam,lParam,false,win);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}// End Win Proc

#ifdef OLDIE
LRESULT CALLBACK FormProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lp=(LPDRAWITEMSTRUCT) lParam;

			switch(lp->CtlType)
			{
			case ODT_BUTTON:
				{
					guiWindow *win=maingui->FirstWindow();

					while(win)
					{
						if(win->formcounter>1)
						{
							for(int x=0;x<win->forms_horz;x++)
								for(int y=0;y<win->forms_vert;y++)
									if(win->forms[x][y].deactivated==false && win->forms[x][y].hWnd==hWnd)
									{
										win->OwnerDrawGadget(wParam,lp);
										return TRUE;
									}
						}

						win=win->NextWindow();
					}
				}
				break;
			}

			return TRUE;
		}
		break;

	case WM_PARENTNOTIFY:
		{
			switch(LOWORD(wParam))
			{
			case WM_CREATE: // Owner Button -> Form Window
				{
					int id=HIWORD(wParam);

					TRACE ("WM_CREATE Form ID=%d\n",id);

					guiWindow *win=maingui->FirstWindow();

					while(win)
					{
						if(win->formcounter>1)
						{
							for(int x=0;x<win->forms_horz;x++)
								for(int y=0;y<win->forms_vert;y++)
									if(win->forms[x][y].deactivated==false && win->forms[x][y].hWnd==hWnd)
									{
										win->CreateButton(id,hWnd);
										return 0;
									}
						}

						win=win->NextWindow();
					}

					/*
					if(id<win->glist.gc)
					{
					win->CreateButton(id,(HWND)lParam);
					return 0;
					}
					*/
				}
				break;
			}
		}
		break;
	}

	guiWindow *win=maingui->FirstWindow();

	while(win)
	{
		if(win->formcounter>1)
		{
			for(int x=0;x<win->forms_horz;x++)
				for(int y=0;y<win->forms_vert;y++)
					if(win->forms[x][y].deactivated==false && win->forms[x][y].hWnd==hWnd)
					{

					}
		}

		win=win->NextWindow();
	}

	//return CheckOSWindowMessage(hWnd,message,wParam,lParam,false,win);
	return DefWindowProc(hWnd, message, wParam, lParam);
}
#endif

#endif


void GUI::OpenEventEditors(Seq_Song *s)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if(win->WindowSong()==s)
			switch(win->GetEditorID())
		{
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_SCORE:
				{
					((EventEditor *)win)->refresheditorevents=false;
				}
				break;
		}

		win=win->NextWindow();
	}
}

void GUI::CloseAllWindowsExecptInfo()
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if(win->GetEditorID()!=EDITORTYPE_CAMXINFO)
		{
			CloseWindow(win);
			win=FirstWindow();
		}
		else
			win=win->NextWindow();
	}
}

void GUI::CloseAllWindows(int type)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if(win->GetEditorID()==type)
		{
			CloseWindow(win);
			win=FirstWindow();
		}
		else
			win=win->NextWindow();
	}
}

void GUI::CloseEventEditors(Seq_Song *s,int flag)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		guiWindow *n=win->NextWindow();

		if(win->WindowSong()==s)
			switch(win->GetEditorID())
		{
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_SCORE:
				{
					EventEditor_Selection *e=(EventEditor_Selection *)win;

					if(e->refresheditorevents==true)
					{
						e->refresheditorevents=false;
						e->ShowAllEvents(flag);
					}
				}
				break;
		}

		win=n;
	}
}

void GUI::OpenEditEditors(Seq_Song *s,int type)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if(win->WindowSong()==s && win->GetEditorID()==type)
			switch(win->GetEditorID())
		{
			case EDITORTYPE_TEMPO:
				{
					((Edit_Tempo *)win)->refresheditorevents=false;
				}
				break;
		}

		win=win->NextWindow();
	}
}

void GUI::CloseEditEditors(Seq_Song *s,int type)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		guiWindow *n=win->NextWindow();

		if(win->WindowSong()==s && win->GetEditorID()==type)
			switch(win->GetEditorID())
		{
			case EDITORTYPE_TEMPO:
				{
					EventEditor *e=(EventEditor *)win;

					if(e->refresheditorevents==true)
					{
						e->refresheditorevents=false;
						((Edit_Tempo *)win)->ShowTempos();
					}
				}
				break;
		}

		win=n;
	}
}

void GUI::EditDataValue(EditData *data)
{
	if(data->win)
	{
		//data->win->GetWindowPositions(); // Get X+Y Positions on screen

		data->desktop=data->win->ondesktop;

		//mainsettings->windowpositions[EDITORTYPE_EDITDATA].x=data->win->GetWinPosX()+data->x;
		//mainsettings->windowpositions[EDITORTYPE_EDITDATA].y=data->win->GetWinPosY()+data->y+maingui->GetFontSizeY();
		OpenEditorStart(EDITORTYPE_EDITDATA,0,0,0,0,data,0);
	}
	else
		delete data;
}

guiWindowSetting::guiWindowSetting(int setid)
{
	//	maingui->editorwindowsetting_default[setid].Clone(this);
	title=0;
	Init();
}

guiWindowSetting::guiWindowSetting()
{
	startposition_x=
		startposition_y=
		startwidth=
		startheight=0;

	title=0;

	nosizing=false;

	formx=-1;

	Init();
}

/*
void guiWindowSetting::Clone(guiWindowSetting *to)
{
to->startposition_x=startposition_x;
to->startposition_y=startposition_y;
to->startwidth=startwidth;
to->startheight=startheight;
}
*/

void guiWindowSetting::Init()
{
	name[0]=0;

#ifdef WIN32
	parent_hwnd=0;
#endif

	calledfromwindow=0;
	standardwindow=true;
	startposition=0;
	simple=false;
	noactivate=false;
	s_ondesktop=false;
	maximized=false;
	noOSborder=false;

	bindtoform=0;
	screen=0;

	startposition_x=0;
	startposition_y=0;
	startwidth=100;
	startheight=100;
}

void ColourTable_RBG::GetRGB(int colour,UBYTE *r,UBYTE *g,UBYTE *b)
{
	int from=3*colour;
	*r=rgb_table[from++];
	*g=rgb_table[from++];
	*b=rgb_table[from++];
}

void ColourTable_RBG::GetRGB(int colour,UBYTE *r,UBYTE *g,UBYTE *b,int add)
{
	int from=3*colour;

	// Red
	int h=rgb_table[from++];
	h+=add;

	if(h>255)
		*r=255;
	else
		if(h<0)
			*r=0;
		else
			*r=(UBYTE)h;

	//Green
	h=rgb_table[from++];
	h+=add;
	if(h>255)
		*g=255;
	else
		if(h<0)
			*g=0;
		else
			*g=(UBYTE)h;

	// Blue
	h=rgb_table[from++];
	h+=add;
	if(h>255)
		*b=255;
	else
		if(h<0)
			*b=0;
		else
			*b=(UBYTE)h;
}

void ColourTable_RBG::InitTable(int colour,UBYTE r,UBYTE g,UBYTE b)
{
	int to=3*colour;

	rgb_table[to++]=r;
	rgb_table[to++]=g;
	rgb_table[to++]=b;
}

void ColourTable_RBG::Init()
{
	// Default B/W
	InitTable(COLOUR_BLACK,0, 0, 0);
	InitTable(COLOUR_CYCLE,40,48,62);
	InitTable(COLOUR_CYCLE_PIANO,35,35,40);

	InitTable(COLOUR_WHITE,255, 255, 255);
	InitTable(COLOUR_BLUE,0, 0, 255);
	InitTable(COLOUR_GREEN,0, 255, 0);

	InitTable(COLOUR_RED,255, 111, 111);
	InitTable(COLOUR_ORANGE,255,165,0);

	InitTable(COLOUR_RED_LIGHT,255, 150, 150);

	InitTable(COLOUR_YELLOW,232, 232, 0);
	InitTable(COLOUR_YELLOW_LIGHT,255, 255, 60);

	InitTable(COLOUR_GREY,160, 160, 160);
	InitTable(COLOUR_GREY_LIGHT,199, 199, 199);
	InitTable(COLOUR_GREY_DARK,80, 80, 80);
	InitTable(COLOUR_BACKGROUND,170, 177, 185);
	InitTable(COLOUR_MIDI_BACKGROUND,180, 180, 200);
	InitTable(COLOUR_MIDI_BACKGROUND2,50, 50, 66);
	InitTable(COLOUR_MIDI_BACKGROUND_SELECTED,179,199,255);

	InitTable(COLOUR_AUDIOHEADER,0, 5, 22);

	InitTable(COLOUR_AUDIORECORDHEADER,	205,175,149);
	InitTable(COLOUR_AUDIORECORD,	139,99,108);

	InitTable(COLOUR_AUDIO_BACKGROUND,130, 150, 167);
	InitTable(COLOUR_AUDIO_BACKGROUND_SELECTED,150,180,200);

	InitTable(COLOUR_RECORD,238,0,0);
	InitTable(COLOUR_RECORDCHILDS,138,43,226);

	InitTable(COLOUR_ERROR,255,0,0);

	InitTable(COLOUR_SUBTRACK_BACKGROUND,120, 100, 160);
	InitTable(COLOUR_BLUE_LIGHT,150, 150, 255);
	InitTable(COLOUR_BLUE_LIGHT2,190, 190, 255);
	InitTable(COLOUR_BLACK_LIGHT,32, 32, 32);

	InitTable(COLOUR_UNUSED,55, 55, 55);
	InitTable(COLOUR_OBJECT,244, 250, 244);
	InitTable(COLOUR_11x1,55,55,88);

	InitTable(COLOUR_WHITE_LIGHT,235, 235, 240);
	InitTable(COLOUR_BACKGROUND2,216, 216, 216);
	InitTable(COLOUR_MOUSEOVER,80,90,95);

	InitTable(COLOUR_MOUSEOVERINDEX,55,55,180);
	InitTable(COLOUR_MOUSEEDITINDEX,88,88,215);


	InitTable(COLOUR_MOUSEOVERSTATIC,140,135,135);
	InitTable(COLOUR_GADGETBACKGROUND,44,44,55);
	InitTable(COLOUR_GADGETBACKGROUNDSTATIC,6,60,100);
	InitTable(COLOUR_GADGETBACKGROUNDSYSTEM,60,60,70);
	InitTable(COLOUR_GADGETBACKGROUNDNOTACTIVE,40,40,40);
	InitTable(COLOUR_BACKGROUNDWINDOWS,35,35,37);
	InitTable(COLOUR_NOMOUSEOVER,23,23,23);

	InitTable(COLOUR_BACKGROUNDTOOLBAR,25,35,42);
	InitTable(COLOUR_GADGETBACKGROUNDOSBUTTON,77,77,90);

	InitTable(COLOUR_TEXT,251,253,255);
	InitTable(COLOUR_TEXT_INFO,234,234,245);
	InitTable(COLOUR_TEXTCONTROL,222,222,232);

	InitTable(COLOUR_GADGETTEXT,200,202,217);
	InitTable(COLOUR_GADGETTEXT_MOUSEOVER,244,255,255);


	InitTable(COLOUR_GADGETBACKGROUNDLISTBOX,48,48,52);
	InitTable(COLOUR_GADGETBACKGROUNDEDIT,48,48,52);
	InitTable(COLOUR_GADGETBACKGROUNDSTRINGINT,48,48,52);

	InitTable(COLOUR_BORDERGADGET,34,34,42);
	InitTable(COLOUR_BORDERGADGETLEFT,100,100,105);
	InitTable(COLOUR_BORDERGADGETRIGHT,22,22,28);

	InitTable(COLOUR_INFOBORDER,	255,236,139);

	InitTable(COLOUR_BACKGROUNDDIALOG,200,200,200);

	InitTable(COLOUR_SAMPLEBACKGROUND,155,155,162);

	InitTable(COLOUR_BACKGROUNDSIZER,205,186,150);

	InitTable(COLOUR_BACKGROUNDFORMTEXT,188,188,188);
	InitTable(COLOUR_BACKGROUNDFORMTEXT_SELECTED,122,197,205);


	InitTable(COLOUR_BACKGROUNDFORMTEXT_HIGHLITE,245,245,245);
	InitTable(COLOUR_BACKGROUNDFORMTEXT_HIGHLITE_SELECTED,152,245,255);

	InitTable(COLOUR_BACKGROUNDTEXTTIME,144,144,152);

	InitTable(COLOUR_BACKGROUNDEDITOR_GFX,	96,123,139);
	InitTable(COLOUR_BACKGROUNDEDITOR_GFX_HIGHLITE,176,196,222);

	InitTable(COLOUR_BACKGROUNDEDITOR_GFX_MUTED,	139,58,58);
	InitTable(COLOUR_BACKGROUNDEDITOR_GFX_MUTEDHIGHLITE,	205,85,85);

	InitTable(COLOUR_BACKGROUNDPATTERN_MUTED,190,130,130);

	InitTable(COLOUR_FRONT_SELECTED,5,5,5);
	InitTable(COLOUR_METROTRACK,	255,130,71);

	InitTable(COLOUR_FRONT_NOTSELECTED,40,40,40);

	InitTable(COLOUR_PIANO_BLACK_BACKGROUND,130,130,130);
	InitTable(COLOUR_PIANO_WHITE_BACKGROUND,150,150,150);
	InitTable(COLOUR_PIANO_WHITE_BACKGROUND_C,180,180,180);
	InitTable(COLOUR_PIANO_WHITE_BACKGROUND_F,168,168,168);

	InitTable(COLOUR_OVERVIEW_BACKGROUND,22,24,46);
	InitTable(COLOUR_OVERVIEWCYCLE,144,124,11);
	InitTable(COLOUR_OVERVIEWOBJECT,0,160,144);

	InitTable(COLOUR_OVERVIEWFOCUSOBJECT,0,140,192);


	InitTable(COLOUR_RASTER1,155,155,172);
	InitTable(COLOUR_RASTER2,42,47,55);

	InitTable(COLOUR_BUSCHANNEL,70,170,180);
	InitTable(COLOUR_BUSCHANNELSELECTED,0,245,255);

	InitTable(COLOUR_MASTERCHANNEL,	100,149,237);

	InitTable(COLOUR_GROUPTOGGLEON,99,184,255);
	InitTable(COLOUR_GROUPTOGGLEOFF,	64,102,129);

	InitTable(COLOUR_TOGGLEON,21,21,26);
	InitTable(COLOUR_TOGGLEOFF,0,0,8);

	InitTable(COLOUR_MUTE,245,15,14);
	InitTable(COLOUR_MIDIVUBACKGROUND,23,25,38);
	InitTable(COLOUR_MIDIVUFOREGROUND,0,255,127);

	InitTable(COLOUR_MIDIVUBACKGROUND_INPUT,144,90,95);
	InitTable(COLOUR_MIDIVUFOREGROUND_INPUT,200,50,60);


	InitTable(COLOUR_DEVICEIN,180,160,160);
	InitTable(COLOUR_DEVICEOUT,160,180,160);
	InitTable(COLOUR_GADGETBACKGROUNDNUMBER,70,75,98);
	InitTable(COLOUR_GAGDGETFOCUS,20,21,50);

	InitTable(COLOUR_TIME,160,255,242);

	InitTable(COLOUR_SMPTE,	135,206,255);
	InitTable(COLOUR_SMPTELIGHT,	74,112,139);

	InitTable(COLOUR_LASSO,33,44,88);

	InitTable(COLOUR_INPUTMONITOR_BACKGROUND,44,11,22);
	InitTable(COLOUR_INPUTMONITOR_BACKGROUND_ACTIVE,66,33,44);

	InitTable(COLOUR_INPUT1,150,100,100);
	InitTable(COLOUR_INPUT2,172,122,122);
	InitTable(COLOUR_INPUT3,194,144,144);

	InitTable(COLOUR_INPUTMONITOR_LINEMAX,100,100,254);

	InitTable(COLOUR_THRUON_BACKGROUND,50,50,90);

	InitTable(COLOUR_BACKGROUNDCHILD,122,122,142);
	InitTable(COLOUR_BACKGROUNDCHILD_HIGHLITE,200,200,234);

	InitTable(COLOUR_GADGETBACKGROUNDPUSHBUTTON,50,62,123);

	InitTable(COLOUR_FOCUSOBJECT,	238,201,0);
	InitTable(COLOUR_FOCUSOBJECT_SELECTED,255,255,0);

	InitTable(COLOUR_TIMEPOSITION,245,44,0);
	InitTable(COLOUR_TIMEPOSITIONMOUSEMOVE,0,44,255);


	InitTable(COLOUR_SENDPOST,72,61,139);
	InitTable(COLOUR_SENDPRE,70,130,180);
	InitTable(COLOUR_AUDIOINVOLUME_BACKGROUND,39,55,73);
	InitTable(COLOUR_AUDIOTYPE_BACKGROUND,100,100,112);
	InitTable(COLOUR_PIANOKEY,	255,235,205);

	InitTable(COLOUR_BORDER_OBJECTMOVING,255,215,0);

	InitTable(COLOUR_AUDIOOUTPEAK,174,238,238);
	InitTable(COLOUR_AUDIOOUTPEAK_SELECTED,	187,255,255);

	InitTable(COLOUR_AUDIOOUTPEAK_GREEN,69,139,116);
	InitTable(COLOUR_AUDIOOUTPEAK_SELECTED_GREEN,102,205,170);

	InitTable(COLOUR_AUDIOOUTPEAK_BACKGROUND,104,131,139);

	InitTable(COLOUR_SAMPLES,154,205,50);
	InitTable(COLOUR_SAMPLESPAINT,71,75,71);

	InitTable(COLOUR_FOLDERBORDER,0,191,255);
	InitTable(COLOUR_PATTERNBORDER,205,186,150);

	InitTable(COLOUR_AUTOMATIONTRACKS,238,221,130);
	InitTable(COLOUR_AUTOMATIONTRACKSUSED,		238,64,0);

	InitTable(COLOUR_AUTOMATIONSCALE,152,245,255);
	InitTable(COLOUR_AUTOMATIONSCALE2,	100,149,237	);

	InitTable(COLOUR_BACKGROUNDAUTOMATION,139,101,8);
	InitTable(COLOUR_BACKGROUNDAUTOMATION_HIGHLITE,	184,134,11);
	InitTable(COLOUR_AUTOMATIONPATTERNBACKGROUND,	74,112,139);

	InitTable(COLOUR_BACKGROUNDAUTOMATIONARRANGE,44,44,44);
	InitTable(COLOUR_LENGTH,123,104,238);
	InitTable(COLOUR_NOREALPATTERN,	25,25,112);
	InitTable(COLOUR_BACKGROUND_TEXT,	205,170,125);
	InitTable(COLOUR_SYNC,	72,118,255);

	InitTable(COLOUR_FROZEN,67,110,238);

	InitTable(COLOUR_AUDIO_BACKGROUND_LOOP,	171,130,255);
	InitTable(COLOUR_MIDI_BACKGROUND_LOOP,205,179,139);

	InitTable(COLOUR_RECSETTINGSON,240,128,128);
	InitTable(COLOUR_SELECTEDARTRACK,110,123,139);
	
	// Init Colours
	UBYTE r,g,b;
	for(int i=0;i<LASTCOLOUR;i++){
		GetRGB(i,&r,&g,&b);
		colour_ref[i]=RGB(r, g, b);
		colour_hBrush[i]=CreateSolidBrush(colour_ref[i]);
		colour_hPen[i]=CreatePen(PS_SOLID, 1, colour_ref[i]);
	}
}

bool GUI::InitGUI()
{
	bool ok=true;

	gfx.gui=this;

	colourtable.Init();

	// strings

	/*
	if(language.OpenLanguage("german.language",0)==false)
	ok=false;
	*/

	// gfx
	if(gfx.InitAllBitMaps()==false)
	{
		ok=false;
		maingui->MessageBoxError(0,"InitAllBitMaps");
	}

	if(gfx.InitTrackIcons()==false)
	{
		ok=false;
		maingui->MessageBoxError(0,"InitTrackIcons");
	}

#ifdef WIN32
	if(hInst)
	{	
		WNDCLASSEX wcx;

		// Register the frame window class. 
		wcx.cbSize= sizeof(WNDCLASSEX);
		wcx.style         = 0; //CS_CLASSDC|CS_DBLCLKS|CS_HREDRAW | CS_VREDRAW; 

		// STYLE CS_HREDRAW | CS_VREDRAW == FLICKER !!!

		wcx.lpfnWndProc   = CheckOSScreenMessage; 
		wcx.cbClsExtra    = 0; 
		wcx.cbWndExtra    = 0; 
		wcx.hInstance     = hInst; 
		wcx.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)); 
		wcx.hIconSm       = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2)); 
		wcx.hCursor       = LoadCursor(NULL, IDC_ARROW); 
		wcx.hbrBackground = colour_hBrush[COLOUR_BACKGROUND]; 
		wcx.lpszMenuName  = 0;
		wcx.lpszClassName = CAMX_SCREENNAME; 

		if (!RegisterClassEx(&wcx) )
		{
			maingui->MessageBoxError(NULL,"Main Frame Reg failed!");
			return false; 
		}

		WNDCLASSEX wcwin;

		// Register Window
		wcwin.cbSize= sizeof(WNDCLASSEX);
		wcwin.style = CS_DBLCLKS;
		wcwin.lpfnWndProc = WndProc; // Msg Thread MDI
		wcwin.cbClsExtra = 0;
		wcwin.cbWndExtra = 0;
		wcwin.hInstance = hInst;
		wcwin.hIcon= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2)); 
		wcwin.hIconSm= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
		wcwin.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcwin.hbrBackground = colour_hBrush[COLOUR_BACKGROUNDWINDOWS];
		wcwin.lpszMenuName = NULL;
		wcwin.lpszClassName = CAMX_WINDOWNAME;

		if (!RegisterClassEx(&wcwin) )
		{
			maingui->MessageBoxError(NULL,"Window Reg failed!");
			return false; 
		}

		wcwin.cbSize= sizeof(WNDCLASSEX);
		wcwin.style = CS_DBLCLKS;
		wcwin.lpfnWndProc = WndProc; // Msg Thread MDI
		wcwin.cbClsExtra = 0;
		wcwin.cbWndExtra = 0;
		wcwin.hInstance = hInst;
		wcwin.hIcon= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2)); 
		wcwin.hIconSm= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
		wcwin.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcwin.hbrBackground = colour_hBrush[COLOUR_BACKGROUNDTOOLBAR];
		wcwin.lpszMenuName = NULL;
		wcwin.lpszClassName = CAMX_TOOLBARTOP;

		if (!RegisterClassEx(&wcwin) )
		{
			maingui->MessageBoxError(NULL,"Window CAMX_TOOLBARTOP Reg failed!");
			return false; 
		}


		/*
		// Register Window
		wcwin.cbSize= sizeof(WNDCLASSEX);
		wcwin.style = CS_HREDRAW | CS_VREDRAW; //CS_DBLCLKS|; CS_VREDRAW|CS_HREDRAW;
		wcwin.lpfnWndProc = FormProc; // Msg Thread MDI
		wcwin.cbClsExtra = 0;
		wcwin.cbWndExtra = 0;
		wcwin.hInstance = hInst;
		wcwin.hIcon= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2)); 
		wcwin.hIconSm= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
		wcwin.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcwin.hbrBackground = colour_hBrush[COLOUR_BACKGROUNDWINDOWS];
		wcwin.lpszMenuName = NULL;
		wcwin.lpszClassName = CAMX_FORMNAME;

		if (!RegisterClassEx(&wcwin) )
		{
		maingui->MessageBoxError(NULL,"Window Reg failed!");
		return false; 
		}
		*/

		// Register Dialog/OS Style
		wcx.style = CS_DBLCLKS;
		wcx.lpfnWndProc = WndProc; // Msg Thread MDI
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = 0;
		wcx.hInstance = hInst;
		wcx.hIcon= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2)); 
		wcx.hIconSm= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
		wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcx.hbrBackground  = colour_hBrush[COLOUR_BACKGROUNDDIALOG];
		wcx.lpszMenuName = NULL;
		wcx.lpszClassName = CAMX_DIALOGNAME;

		if (!RegisterClassEx(&wcx) )
		{
			maingui->MessageBoxError(NULL,"Window Reg failed!");
			return false; 
		}


		WNDCLASS buttonos;
		GetClassInfo(hInst,"BUTTON",&buttonos);

		WNDCLASSEX button;

		// Double Buffer Button
		button.cbSize = sizeof(WNDCLASSEX);
		button.style =/*CS_CLASSDC|*/CS_DBLCLKS|CS_PARENTDC;
		button.lpfnWndProc = ChildWindowProc ; // Msg Thread MDI
		button.cbClsExtra =0;
		button.cbWndExtra = 0;
		button.hInstance = hInst;
		button.hIcon=0; 
		button.hIconSm= 0;
		button.hCursor = LoadCursor(NULL, IDC_ARROW);
		button.hbrBackground =0;
		button.lpszMenuName = NULL;
		button.lpszClassName =CAMX_CHILDWINDOWNAME;

		if (!RegisterClassEx(&button))
			return false; 

		// Sizer Horz

		button.cbSize = sizeof(WNDCLASSEX);
		button.style = buttonos.style;
		button.lpfnWndProc = SizerHProc ; // Msg Thread MDI
		button.cbClsExtra = buttonos.cbClsExtra;
		button.cbWndExtra = buttonos.cbWndExtra;
		button.hInstance = hInst;
		button.hIcon=0; 
		button.hIconSm= 0;
		button.hCursor = LoadCursor(NULL, IDC_SIZEWE);
		button.hbrBackground =colour_hBrush[COLOUR_BACKGROUNDSIZER];
		button.lpszMenuName = NULL;
		button.lpszClassName ="CAMXZH";

		if (!RegisterClassEx(&button))
			return false; 


		// Sizer Screen Vertical

		button.cbSize = sizeof(WNDCLASSEX);
		button.style = buttonos.style;
		button.lpfnWndProc = SizerVProc ; // Msg Thread MDI
		button.cbClsExtra = buttonos.cbClsExtra;
		button.cbWndExtra = buttonos.cbWndExtra;
		button.hInstance = hInst;
		button.hIcon=0; 
		button.hIconSm= 0;
		button.hCursor = LoadCursor(NULL, IDC_SIZENS);
		button.hbrBackground =colour_hBrush[COLOUR_BACKGROUNDSIZER];
		button.lpszMenuName = NULL;
		button.lpszClassName ="CAMXSCRZV";

		if (!RegisterClassEx(&button))
			return false; 

		button.cbSize = sizeof(WNDCLASSEX);
		button.style = buttonos.style;
		button.lpfnWndProc = SizerHProc ; // Msg Thread MDI
		button.cbClsExtra = buttonos.cbClsExtra;
		button.cbWndExtra = buttonos.cbWndExtra;
		button.hInstance = hInst;
		button.hIcon=0; 
		button.hIconSm= 0;
		button.hCursor = LoadCursor(NULL, IDC_SIZEWE);
		button.hbrBackground =colour_hBrush[COLOUR_BACKGROUNDSIZER];
		button.lpszMenuName = NULL;
		button.lpszClassName ="CAMXSCRZH";

		if (!RegisterClassEx(&button))
			return false; 


		// Sizer Vertical

		button.cbSize = sizeof(WNDCLASSEX);
		button.style = buttonos.style;
		button.lpfnWndProc = SizerVProc ; // Msg Thread MDI
		button.cbClsExtra = buttonos.cbClsExtra;
		button.cbWndExtra = buttonos.cbWndExtra;
		button.hInstance = hInst;
		button.hIcon=0; 
		button.hIconSm= 0;
		button.hCursor = LoadCursor(NULL, IDC_SIZENS);
		button.hbrBackground =colour_hBrush[COLOUR_BACKGROUNDSIZER];
		button.lpszMenuName = NULL;
		button.lpszClassName ="CAMXZV";

		if (!RegisterClassEx(&button))
			return false; 

		WNDCLASSEX vst_w;

		vst_w.cbSize = sizeof(WNDCLASSEX);
		vst_w.style = CS_DBLCLKS /*|CS_PARENTDC*/;
		vst_w.lpfnWndProc = VSTwProc; // Msg Thread MDI
		vst_w.cbClsExtra = 0;
		vst_w.cbWndExtra = 0;
		vst_w.hInstance = hInst;
		vst_w.hIcon=0; 
		vst_w.hIconSm= 0;
		vst_w.hCursor = LoadCursor(NULL, IDC_ARROW);
		vst_w.hbrBackground = colour_hBrush[COLOUR_BLACK];
		vst_w.lpszMenuName = NULL;
		vst_w.lpszClassName = "VST Frame";

		if (!RegisterClassEx(&vst_w))
			return false; 
	}

	NONCLIENTMETRICS ncm;

	memset(&ncm, 0, sizeof(NONCLIENTMETRICS));
	ncm.cbSize = sizeof(NONCLIENTMETRICS);

#if (WINVER >= 0x0600)
	OSVERSIONINFO osvi = { sizeof(OSVERSIONINFO), 0 };
	VERIFY(GetVersionEx(&osvi));
	if (osvi.dwMajorVersion < 6) {
		ncm.cbSize -= sizeof(int);
	}
#endif

	borderheight=GetSystemMetrics(SM_CYDRAG);
	borderwidth=GetSystemMetrics(SM_CXDRAG);

	//borderframey=GetSystemMetrics(SM_CYSIZEFRAME);
	borderhorzsize=2*GetSystemMetrics(SM_CYBORDER);

	if(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
	{
		//	ncm.lfCaptionFont.lfFaceName;
		double fh=-ncm.lfMenuFont.lfHeight;
		fh*=1.2;
		fontsizey=fh;

		fh=fontsizey;
		fh*=0.4;

		borderframey=(int)(2*fh);

		scrollwidth=ncm.iScrollWidth;
		scrollheight=ncm.iScrollHeight;

		//		double h=fontsizey;
		//		h*=0.4;
		buttonsizey=fontsizey+borderhorzsize;
		buttonsizeaddy=borderhorzsize;

		LOGFONT standard,bold,cursiv;

		memcpy(&standard,&ncm.lfMenuFont,sizeof(LOGFONT));
		memcpy(&bold,&standard,sizeof(LOGFONT));
		memcpy(&cursiv,&standard,sizeof(LOGFONT));

		//		standard.lfQuality=PROOF_QUALITY;
		standardfont.hfont= CreateFontIndirect(&standard);


		/*LOGFONT lbigFont;
		GetObject ( standardfont.hfont, sizeof(LOGFONT), &lbigFont );
		fontsizey=lbigFont.lfHeight;
		if(fontsizey<0)fontsizey*=-1;
		*/

		double th=fontsizey;

		th*=2.54;
		fontsizex=(int)th;

		cursiv.lfItalic=TRUE;
		smallfont.hfont= CreateFontIndirect(&cursiv);
		//		lbigFont.lfItalic=FALSE;

		bold.lfWeight=FW_SEMIBOLD;
		standard_bold.hfont= CreateFontIndirect(&bold);	
	}
	else
		return false;

#endif

	/*
	int cboxflag=WS_VSCROLL|CBS_DROPDOWN|CBS_HASSTRINGS;

	HWND hWnd = 
	CreateWindowEx
	(
	0, 
	"COMBOBOX", 
	0, 
	cboxflag, 
	0, 0,100, 100,
	NULL,
	(HMENU)1, 
	maingui->hInst, 
	NULL);

	SendMessage(hWnd, WM_SETFONT,(WPARAM)maingui->standardfont.hfont,0);
	*/

	LoadSettings();

	// Default Screen
	if(!FirstScreen())
	{
		guiScreen *screen=new guiScreen;
		AddScreen(screen);
	}

	return ok;
}

void GUI::Welcome()
{
#ifdef OLDIE
	CButton button;

	guiWindowSetting welcome;
	strcpy(welcome.name,"CamX");

	welcome.startposition_x=0;
	welcome.startposition_y=0;
	welcome.startwidth=300;
	welcome.startheight=200;

	guiWindow *win=OpenWindow(0,&welcome,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

	if(win)
	{
		guiGadgetList *infoglist=win->gadgetlists.AddGadgetList(win);

		if(infoglist)
		{
			int y=10;

			//			infoglist->AddText(0,y,welcome.startwidth,y+20,(Cxs[CXS_WELCOME]),0);

			y+=30;

			//			infoglist->AddText(0,y,welcome.startwidth,y+20,(Cxs[CXS_LANGUAGE]),0);

		}


		// win->CloseWindow(false);
	}
#endif

}

guiScreen *GUI::FindScreen(
#ifdef WIN32
						   HWND hWnd
#endif
						   )
{

	guiScreen *f=FirstScreen();

	while(f)
	{
#ifdef WIN32
		if(f->hWnd==hWnd)
			return f;
#endif

		f=f->NextScreen();
	}

	return 0;
}

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>

#include "resource.h"

void GUI::CreateRecentProjectList(guiScreen *screen,guiMenu *menu_recentprojects)
{
	if(menu_recentprojects){

		class menu_selectrec:public guiMenu
		{
		public:
			menu_selectrec(guiScreen *s,int id)
			{
				screen=s;
				select_id=id;
			}

			void MenuFunction(){
				maingui->OpenProjectID(screen,select_id);
			} 

			guiScreen *screen;
			int select_id;
		};

		bool show=false;
		for(int i=0;i<6;i++){
			if(mainsettings->prevprojects_dirname[i])
			{
				Seq_Project *p=mainvar->FirstProject();

				while(p)
				{
					if(strcmp(p->projectdirectory,mainsettings->prevprojects_dirname[i])==0)break;
					p=p->NextProject();
				}

				if(!p)
				{
					show=true;
					char *h=mainvar->GenerateString(mainsettings->prevprojects_proname[i],"<",mainsettings->prevprojects_dirname[i],">");

					if(h)
					{
						menu_recentprojects->AddFMenu(h,new menu_selectrec(screen,i));
						delete h;
					}
				}
			}
			else
				break;
		}

		if(show==false)
			menu_recentprojects->Disable(0);
	}
}

void GUI::CreateProjectList(guiMenu *menu_projectlist)
{
	if(menu_projectlist)
	{
		class menu_selectpro:public guiMenu
		{
		public:
			menu_selectpro(Seq_Project *p){
				project=p;
			}

			void MenuFunction(){
				mainvar->SetActiveProject(project,0);
			} 

			Seq_Project *project;
		};

		Seq_Project *p=mainvar->FirstProject();

		while(p){
			/*
			bool sel;

			if(p==mainvar->GetActiveProject())
			sel=true;
			else
			sel=false;
			*/

			if(p->projectdirectory){
				if(char *s=mainvar->GenerateString(p->name,"<:",p->projectdirectory,">") ){
					menu_projectlist->AddFMenu(s,new menu_selectpro(p),p==mainvar->GetActiveProject()?true:false);
					delete s;
				}
			}

			p=p->NextProject();
		}
	}
}


void GUI::CreateSongList(guiMenu *menu_songlist,guiScreen *screen)
{
	if(menu_songlist)
	{
		if(Seq_Project *project=screen->project)
		{	
			if(char *h=mainvar->GenerateString(Cxs[CXS_SONGSOFPROJECT],":",project->name))
			{
				menu_songlist->AddMenu(h,0);
				delete h;

				if(h=mainvar->GenerateString(Cxs[CXS_DIRECTORY],":",project->projectdirectory))
				{
					menu_songlist->AddMenu(h,0);
					delete h;
				}

				menu_songlist->AddLine();
			}

			class menu_selectsong:public guiMenu
			{
			public:
				menu_selectsong(Seq_Song *s,guiScreen *scr){song=s;screen=scr;}

				void MenuFunction()
				{
					if(song)
					{
						if(song->loaded==true)
							mainvar->SetActiveSong(song);
						else
							song->OpenIt(screen);
					}
				} 

				Seq_Song *song;
				guiScreen *screen;
			};

			Seq_Song *s=project->FirstSong();

			while(s){

				if(s->directoryname){
					if(char *h=mainvar->GenerateString(s->songname?s->songname:"_",s->loaded==true?" [ *** Open *** ":" < ",s->directoryname+strlen(project->projectdirectory),s->loaded==true?"]":">"))
					{
						menu_songlist->AddFMenu(h,new menu_selectsong(s,screen),screen && s==screen->song?true:false,0);
						delete h;
					}
				}
				else
				{
					if(s->autoloadsong==true)
					{
						if(char *h=mainvar->GenerateString(Cxs[CXS_AUTOLOADSONG],"<:",s->directoryname,">"))
						{
							menu_songlist->AddFMenu(h,new menu_selectsong(s,screen),screen && s==screen->song?true:false,0);
							delete h;
						}
					}
					else
						menu_songlist->AddFMenu(Cxs[CXS_SONGDIRERROR],new menu_selectsong(0,0),false);
				}

				s=s->NextSong();
			}
		}
	}
}

void GUI::OpenProjectID(guiScreen *screen,int id)
{
	TRACE ("Open Project ID %d\n",id);

	if(id<6 && mainsettings->prevprojects_dirname[id]){

		Seq_Project *p=mainvar->FirstProject();

		while(p)
		{
			if(strcmp(p->projectdirectory,mainsettings->prevprojects_dirname[id])==0)
				return;

			p=p->NextProject();
		}

		mainvar->OpenProject(screen,mainsettings->prevprojects_dirname[id]);
	}
}


void GUI::AddSetMarkerPositions(Seq_Song *song,guiMenu *menu)
{
	if(song && song->textandmarker.FirstMarker())
	{
		menu->AddLine();

		// Marker -> Cycle
		if(guiMenu *marker=menu->AddMenu(Cxs[CXS_SETCYCLEPOSITIONMARKER],0))
		{
			class menu_setmk:public guiMenu
			{
			public:
				menu_setmk(Seq_Song *s,Seq_Marker *m)
				{
					song=s;
					marker=m;
				}

				void MenuFunction()
				{		
					song->SetCycle(marker->GetMarkerStart(),marker->GetMarkerEnd()); // no quantize
				}

				Seq_Song *song;
				Seq_Marker *marker;
			};

			Seq_Marker *m=song->textandmarker.FirstMarker();
			while(m)
			{
				marker->AddFMenu(m->CreateFromToString(),new menu_setmk(song,m));
				m=m->NextMarker();
			}
		}

		// Marker -> Cycle
		if(guiMenu *marker=menu->AddMenu(Cxs[CXS_SETSPWITHMARKERSTART],0))
		{
			class menu_setmk:public guiMenu
			{
			public:
				menu_setmk(Seq_Song *s,Seq_Marker *m)
				{
					song=s;
					marker=m;
				}

				void MenuFunction()
				{		
					song->SetSongPosition(marker->GetMarkerStart(),true);
				}

				Seq_Song *song;
				Seq_Marker *marker;
			};

			Seq_Marker *m=song->textandmarker.FirstMarker();
			while(m)
			{
				marker->AddFMenu(m->CreateFromString(),new menu_setmk(song,m));
				m=m->NextMarker();
			}
		}
	}
}

void GUI::CreateScreenMenu(guiScreen *screen,guiWindow *win)
{
	if(screen)
	{
		if(screen->menu)
			screen->menu->RemoveMenu();

		if(screen->menu=new guiMenu)
		{
			Seq_Project *project=screen->project;
			Seq_Song *song=screen->GetSong();

			screen->menu->screen=screen;

			guiMenu *menu=screen->menu;
			guiMenu *n=menu->AddMenu(Cxs[CXS_FILE],0);

			if(n)
			{	
				class menu_NewProject:public guiMenu
				{
				public:
					menu_NewProject(guiScreen *s){screen=s;}

					void MenuFunction()
					{
						mainvar->CreateNewProjectWithNewDirectory(screen);
					}

					guiScreen *screen;
				};

				n->AddFMenu(Cxs[CXS_NEW_PROJECT],new menu_NewProject(screen));

				class menu_loadProject:public guiMenu
				{
				public:
					menu_loadProject(guiScreen *s){screen=s;}
					void MenuFunction(){mainvar->LoadProject(screen);}

					guiScreen *screen;
				};

				n->AddFMenu(Cxs[CXS_OPEN_PROJECT],new menu_loadProject(screen));

				if(project)
				{
					class menu_saveProject:public guiMenu
					{
					public:
						menu_saveProject(Seq_Project *p){project=p;}

						void MenuFunction(){mainvar->SaveProject(project);}
						Seq_Project *project;
					};

					n->AddFMenu(Cxs[CXS_SAVE_PROJECT],new menu_saveProject(project));

					// Rename Project
					/*
					class menu_RenameProject:public guiMenu
					{
					public:
					menu_RenameProject(Edit_Transport *et){editor=et;}
					void MenuFunction(){editor->EditProjectName();} 
					Edit_Transport *editor;
					};

					if(char *pn=mainvar->GenerateString(Cxs[CXS_RENAME_PROJECT],":",mainvar->GetActiveProject()->name))
					{
					n->AddFMenu(pn,new menu_RenameProject(this));
					delete pn;
					}
					*/

					n->AddLine();
					class menu_closeProject:public guiMenu
					{
					public:
						menu_closeProject(Seq_Project *p){project=p;}

						void MenuFunction()
						{
							mainvar->QuestionCloseProject(project);
						} 

						Seq_Project *project;
					};
					n->AddFMenu(Cxs[CXS_CLOSE_PROJECT],new menu_closeProject(project));
					n->AddLine();
				}

				// Project
				if(mainvar->FirstProject())
				{
					guiMenu *m=n->AddMenu(Cxs[CXS_SELECT_PROJECT],0);
					CreateProjectList(m);
				}

				if(mainsettings->prevprojects_dirname[0]){

					guiMenu *m=n->AddMenu(Cxs[CXS_RECENT_PROJECT],0);
					CreateRecentProjectList(screen,m);
				}
				n->AddLine();

				// Songs ------------------------
				if(project && project->FirstSong())
				{
					char h2[NUMBERSTRINGLEN],*os=mainvar->ConvertIntToChar(project->GetCountOfOpenSongs(),h2);

					if(char *h=mainvar->GenerateString(Cxs[CXS_SELECT_PROJECT_SONG]," (Song/s ",Cxs[CXS_OPEN],":",os,")"))
					{
						guiMenu *m=n->AddMenu(h,0);
						CreateSongList(m,screen);
						delete h;
					}
				}

#ifdef OLDIE
				if(song && song->autoloadMIDI==false && song->autoloadsong==false)
				{
					Edit_Transport *trans=screen->GetTransport();

					if(trans)
					{
						/*
						// Rename Song
						class menu_RenameSong:public guiMenu
						{
						public:
						menu_RenameSong(Edit_Transport *t,Seq_Song *s)
						{
						trans=t;
						song=s;
						}

						void MenuFunction()
						{
						if(song)
						{
						if(EditData *edit=new EditData)
						{
						edit->win=trans;
						edit->x=0;
						edit->y=0;
						edit->name=Cxs[CXS_EDIT_SONG_NAME];
						edit->id=Edit_Transport::EDIT_SONGNAME;
						edit->type=EditData::EDITDATA_TYPE_STRING_TITLE;
						edit->string=song->songname;

						maingui->EditDataValue(edit);
						}
						}	
						} 

						Edit_Transport *trans;
						Seq_Song *song;
						};

						if(char *asng=mainvar->GenerateString(Cxs[CXS_EDIT_SONG_NAME],":",song->songname))
						{
						n->AddFMenu(asng,new menu_RenameSong(trans,song));
						delete asng;
						}
						*/
					}
				}
#endif

				if(project)
				{
					class menu_NewSong:public guiMenu
					{
					public:
						menu_NewSong(guiScreen *s,Seq_Project *p){screen=s;project=p;}

						void MenuFunction()
						{
							if(project)
								mainvar->NewSong(project,screen);
						}

						guiScreen *screen;
						Seq_Project *project;
					};

					n->AddLine();

					n->AddFMenu(Cxs[CXS_NEW_SONG],new menu_NewSong(screen,project));

					class menu_OpenSong:public guiMenu
					{
					public:
						menu_OpenSong(guiScreen *s,Seq_Project *p){screen=s;project=p;}

						void MenuFunction()
						{
							if(project)
								project->OpenSong(screen);
						}

						guiScreen *screen;
						Seq_Project *project;
					};

					n->AddFMenu(Cxs[CXS_OPENADD_SONG],new menu_OpenSong(screen,project));

					guiMenu *s=n->AddMenu(Cxs[CXS_IMPORT_SONG],0);
					if(s)
					{
						class menu_MIDIFile:public guiMenu
						{
						public:
							menu_MIDIFile(guiScreen *s,Seq_Project *p){screen=s;project=p;}

							void MenuFunction()
							{
								//if(mainvar->GetActiveProject())
								project->ImportMIDIFile(screen);

								/*
								{
								if(Seq_Song *s=)
								{
								maingui->OpenEditorStart(EDITORTYPE_ARRANGE,s,0,0,0,0,s->GetSongPosition());

								if(mainvar->GetActiveSong()!=s)
								{
								if(mainvar->GetActiveSong()->status==Seq_Song::STATUS_STOP)
								mainvar->SetActiveSong(s);
								}	
								}
								}
								else
								maingui->MessageBoxError(0,Cxs[CXS_PLEASE_CREATENEWPROJECT]);
								*/
							} //

							guiScreen *screen;
							Seq_Project *project;
						};

						s->AddFMenu(Cxs[CXS_MIDI_FILE],new menu_MIDIFile(screen,project));

						class menu_WaveFile:public guiMenu
						{
						public:
							menu_WaveFile(Seq_Song *s){song=s;}

							void MenuFunction()
							{
								//Seq_Song *song=mainvar->GetActiveSong();

								if(song->GetFocusTrack())
									mainedit->LoadSoundFile(0,song->GetFocusTrack(),0,0);
							} //

							Seq_Song *song;
						};

						if(song && song->GetFocusTrack())
							s->AddFMenu(Cxs[CXS_WAVE_FILE],new menu_WaveFile(song));
					}

					n->AddLine();

					if(song)
					{
						class menu_CloseSong:public guiMenu
						{
						public:
							menu_CloseSong(guiScreen *s,bool r){screen=s;remove=r;}

							void MenuFunction()
							{
								maingui->CloseSong(screen,remove);
							}

							guiScreen *screen;
							bool remove;

						};

						n->AddFMenu(Cxs[CXS_CLOSE_SONG],new menu_CloseSong(screen,false));

						if(project->GetCountOfOpenSongs()>1)
						{
							class menu_CloseAllOtherSong:public guiMenu
							{
							public:
								menu_CloseAllOtherSong(guiScreen *s){screen=s;}

								void MenuFunction()
								{
									if(screen && screen->project && screen->song)
										screen->project->CloseAllButSong(0,screen->song);
								}

								guiScreen *screen;
							};

							n->AddFMenu(Cxs[CXS_CLOSEALLOTHERSONGS],new menu_CloseAllOtherSong(screen));
						}

						if(song!=mainvar->GetActiveSong())
						{
							class menu_activatesong:public guiMenu
							{
							public:
								menu_activatesong(Seq_Song *s){song=s;}

								void MenuFunction()
								{
									mainvar->SetActiveSong(song);
								}

								Seq_Song *song;
							};

							n->AddFMenu(Cxs[CXS_ACTIVATESONG],new menu_activatesong(song));
						}

						n->AddLine(); 

						n->AddFMenu(Cxs[CXS_DELETESONG],new menu_CloseSong(screen,true));

						n->AddLine();

						class menu_SaveSong:public guiMenu
						{
						public:
							menu_SaveSong(Seq_Song *s){song=s;}
							void MenuFunction(){song->Save(0);}
							Seq_Song *song;
						};

						n->AddFMenu(Cxs[CXS_SAVE_SONG],new menu_SaveSong(song),SK_SAVE);

						class menu_SaveSongAS:public guiMenu
						{
						public:

							menu_SaveSongAS(Seq_Song *s,guiScreen *scr){song=s;screen=scr;}

							void MenuFunction()
							{
								if(song)
								{
									camxFile save;

									if (save.OpenFileRequester(screen,0,Cxs[CXS_SAVE_SONG],"CamX (*.camx)|*.camx;|All Files (*.*)|*.*||",false,song->GetName())==true)
									{					
										save.AddToFileName(".camx");

										if(save.OpenSave(save.filereqname)==true)
											song->Save(&save);

										save.Close(true);
									}
								}
							} //

							Seq_Song *song;
							guiScreen *screen;
						};
						n->AddFMenu(Cxs[CXS_SAVE_SONGAS],new menu_SaveSongAS(song,screen));
						s=n->AddMenu(Cxs[CXS_EXPORT_SONGAS],0);

						if(s)
						{
							class menu_SaveMIDISong:public guiMenu
							{
							public:
								menu_SaveMIDISong(Seq_Song *s,guiScreen *scr,bool format){song=s;screen=scr;format1=format;}

								void MenuFunction()
								{
									if(song)
									{
										camxFile cMIDIfile;

										char *sformat=Cxs[format1==true?CXS_SAVE_SMF1:CXS_SAVE_SMF0];

										if (cMIDIfile.OpenFileRequester(screen,0,sformat,cMIDIfile.AllFiles(camxFile::FT_MIDIFILE),false,song->GetName())==true)
										{					
											cMIDIfile.AddToFileName(".mid");
											MIDIFile MIDIfile;
											MIDIfile.SaveSongToFile(song,cMIDIfile.filereqname,format1);
										}
									}
								} //

								Seq_Song *song;
								guiScreen *screen;
								bool format1;
							};

							s->AddFMenu(Cxs[CXS_SAVE_EXSMF0],new menu_SaveMIDISong(song,screen,false));
							s->AddFMenu(Cxs[CXS_SAVE_EXSMF1],new menu_SaveMIDISong(song,screen,true));
						}

						n->AddLine();

						class menu_SaveSongArrangement:public guiMenu
						{
						public:
							menu_SaveSongArrangement(Seq_Song *s,guiScreen *scr){song=s;screen=scr;}

							void MenuFunction()
							{
								camxFile save;

								if (save.OpenFileRequester(screen,0,Cxs[CXS_SAVE_SONG],"CamX Arrangement (*.caar)|*.caar;|All Files (*.*)|*.*||",false,song->GetName())==true)
								{					
									save.AddToFileName(".caar");

									if(save.OpenSave(save.filereqname)==true)
										song->SaveArrangement(&save);

									save.Close(true);
								}


							} //

							Seq_Song *song;
							guiScreen *screen;
						};

						n->AddFMenu(Cxs[CXS_SAVESONGARRANGEMENT],new menu_SaveSongArrangement(song,screen));
						n->AddLine();
					}
				}

				// Auto Load
				if(guiMenu *al=n->AddMenu(Cxs[CXS_AUTOLOADSONG],0))
				{
					n->AddLine();

					class menu_loadautoload:public guiMenu
					{
					public:
						menu_loadautoload(guiScreen *s,bool m){screen=s;MIDI=m;}

						void MenuFunction()
						{
							// Find Autoload Song
							Seq_Song *song;

							if(song=maingui->LoadDefaultSong(screen,0,MIDI)) // 0=create load song
							{
								/*
								// Refresh GUI
								guiWindow *w=maingui->FirstWindow();
								while(w)
								{
								if(w->GetEditorID()==EDITORTYPE_TRANSPORT)
								((Edit_Transport *)w)->RefreshMenu();

								w=w->NextWindow();
								}

								maingui->OpenEditorStart(EDITORTYPE_ARRANGE,song,0,0,0,0,song->GetSongPosition());
								*/
							}
						}

						guiScreen *screen;
						bool MIDI;
					};

					class menu_saveautoload:public guiMenu
					{
					public:
						menu_saveautoload(Seq_Song *s,bool m){song=s;MIDI=m;}

						void MenuFunction()
						{
							if(song)
							{
								maingui->SaveDefaultSong(song,MIDI);

								/*
								// Refresh GUI
								guiWindow *w=maingui->FirstWindow();
								while(w)
								{
								if(w->GetEditorID()==EDITORTYPE_TRANSPORT)
								((Edit_Transport *)w)->RefreshMenu();

								w=w->NextWindow();
								}
								*/
							}
						}

						Seq_Song *song;
						bool MIDI;
					};

					// CamX
					if((!maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_CAMX)) && maingui->AskForDefaultSong(GUI::ID_AUTOLOAD_CAMX,false))
						al->AddFMenu(Cxs[CXS_OPENAUTOLOADSONG],new menu_loadautoload(screen,false));

					if(song!=maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_CAMX))
						al->AddFMenu(Cxs[CXS_SAVEAUTOLOADSONG],new menu_saveautoload(song,false));

					class menu_activateautoload:public guiMenu
					{
					public:
						menu_activateautoload(Seq_Song *s){song=s;}

						void MenuFunction()
						{
							mainvar->SetActiveSong(song);
						}

						Seq_Song *song;
					};

					if(maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_CAMX) && song!=maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_CAMX))
					{
						if(char *h=mainvar->GenerateString(Cxs[CXS_ACTIVEAUTOLOADSONG]," (CamX):",maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_CAMX)->GetName()))
						{
							al->AddFMenu(h,new menu_activateautoload(maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_CAMX)) );
							delete h;
						}
					}

					al->AddLine();
					// MIDI
					if((!maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_MIDI)) && maingui->AskForDefaultSong(GUI::ID_AUTOLOAD_MIDI,false))
						al->AddFMenu(Cxs[CXS_AUTOLOADSONG_SMF],new menu_loadautoload(screen,true));

					if(mainvar->GetActiveSong()!=maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_MIDI))
						al->AddFMenu(Cxs[CXS_AUTOSAVESONG_SMF],new menu_saveautoload(song,true));

					if(maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_MIDI) && song!=maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_MIDI))
					{
						if(char *h=mainvar->GenerateString(Cxs[CXS_ACTIVEAUTOLOADSONG]," (SMF/MIDI):",maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_MIDI)->GetName()))
						{
							al->AddFMenu(h,new menu_activateautoload(maingui->GetAutoLoadSong(GUI::ID_AUTOLOAD_MIDI)) );
							delete h;
						}
					}
				}

				n->AddLine();
				class menu_ExitCamx:public guiMenu
				{
				public:
					menu_ExitCamx(bool s){save=s;}
					void MenuFunction()
					{
						mainvar->AskQuitMessage(save);
					}

					bool save;
				};

				n->AddFMenu(Cxs[CXS_EXITCAMX_DONTSAVE],new menu_ExitCamx(false));
				n->AddFMenu(Cxs[CXS_EXITCAMX],new menu_ExitCamx(true));
			}

			if(n=menu->AddMenu(Cxs[CXS_EDIT],0))
				AddUndoMenu(n);

			if(n=menu->AddMenu(Cxs[CXS_FUNCTIONS],0))
			{
				if(guiMenu *s=n->AddMenu("MIDI/Plugins",0))
				{
					class menu_SendReset:public guiMenu
					{
						void MenuFunction()
						{
							mainMIDI->SendResetToAllDevices(0/*NO_SYSTEMLOCK*/);	
						}
					};

					s->AddFMenu(Cxs[CXS_SENDRESET],new menu_SendReset);

					class menu_SendPanic:public guiMenu
					{
						void MenuFunction()
						{
							mainMIDI->SendPanic();
						}
					};
					s->AddFMenu(Cxs[CXS_SENDPANIC],new menu_SendPanic);

					if(song)
					{
						s->AddLine();

						class menu_SendStartupSys:public guiMenu
						{
						public:
							menu_SendStartupSys(Seq_Song *s)
							{
								song=s;
							}

							void MenuFunction()
							{
								song->SendSysExStartupMIDIPattern();
							}

							Seq_Song *song;
						};

						s->AddFMenu(Cxs[CXS_SENDALLSYSEX],new menu_SendStartupSys(song));

					}

				}

				n->AddLine();
				if(guiMenu *s=n->AddMenu("Audio",0))
				{
					class menu_AudioReset:public guiMenu
					{
						void MenuFunction()
						{
							mainaudio->ResetAudio(0,0);
						}
					};

					s->AddFMenu("Reset Audio Engine",new menu_AudioReset);
				}

				class menu_StopAFunctions:public guiMenu
				{
					void MenuFunction()
					{
						audioworkthread->StopAllProcessing();
					}
				};

				n->AddFMenu(Cxs[CXS_STOPAUDIOFUNCTIONS],new menu_StopAFunctions);
			}

			if(n=menu->AddMenu(Cxs[CXS_OPTIONS],0))
			{
				class menu_Settings:public guiMenu
				{
				public:
					menu_Settings(bool act,bool pr){activesong=act;project=pr;}

					void MenuFunction()
					{
						if(activesong==true && mainvar->GetActiveSong())
						{
							maingui->OpenEditorStart(EDITORTYPE_SETTINGS,0,0,0,0,(Object *)mainvar->GetActiveSong(),1);
						}
						else
							if(activesong==false || mainvar->GetActiveSong())
							{
								if(project==true)
								{
									if(mainvar->GetActiveProject())
									{
										guiWindow *win=maingui->FirstWindow();
										while(win)
										{
											if(win->GetEditorID()==EDITORTYPE_SETTINGS)
											{
												Edit_Settings *es=(Edit_Settings *)win;

												if(es->editproject==mainvar->GetActiveProject())
												{
													win->WindowToFront(true);
													break;
												}
											}

											win=win->NextWindow();
										}

										if(!win)
											maingui->OpenEditorStart(EDITORTYPE_SETTINGS,0,0,0,0,(Object *)mainvar->GetActiveProject(),0);
									}
								}
								else
								{
									guiWindow *win=maingui->FirstWindow();
									while(win)
									{
										if(win->GetEditorID()==EDITORTYPE_SETTINGS)
										{
											Edit_Settings *es=(Edit_Settings *)win;

											if(es->editproject==0)
											{
												win->WindowToFront(true);
												break;
											}
										}

										win=win->NextWindow();
									}

									if(!win)
										maingui->OpenEditorStart(EDITORTYPE_SETTINGS,0,0,0,0,0,0);
								}
							}
					}

					bool activesong;
					bool project;
				};

				n->AddFMenu(Cxs[CXS_SETTINGS],new menu_Settings(false,false));

				if(project)
					n->AddFMenu(Cxs[CXS_PROJECTSETTINGS],new menu_Settings(false,true)); 

				if(song)
				{
					n->AddFMenu(Cxs[CXS_SONGSETTINGS],new menu_Settings(true,false));

					class menu_RecSettings:public guiMenu
					{
					public:
						menu_RecSettings(Seq_Song *s){song=s;}

						void MenuFunction()
						{
							if(song)
								song->OpenRecordEditor();
						}

						Seq_Song *song;
					};

					n->AddFMenu(Cxs[CXS_RECORDSETTINGS],new menu_RecSettings(song));

					class menu_Sync:public guiMenu
					{
					public:
						menu_Sync(guiScreen *s){screen=s;}

						void MenuFunction()
						{
							if(screen->GetSong())
								screen->GetSong()->OpenSyncEditor();
						}

						guiScreen *screen;
					};

					n->AddFMenu(Cxs[CXS_SONGSYNCSETTINGS],new menu_Sync(screen));
				}

				class menu_EXCSettings:public guiMenu
				{
				public:
					menu_EXCSettings(bool act){activesong=act;}

					void MenuFunction()
					{

					}

					bool activesong;
				};

				n->AddFMenu(Cxs[CXS_EXTERNCONTROLLER],new menu_EXCSettings(true)); // 0

				if(song)
				{
					n->AddLine();

					// Cycle Pos
					class menu_selectcycle:public guiMenu
					{
					public:
						menu_selectcycle(Seq_Song *s,int i)
						{
							song=s;
							index=i;
						}

						void MenuFunction()
						{
							if(song)
								song->SetCycleIndex(index);
						}

						Seq_Song *song;
						int index;
					};

					for(int i=0;i<8;i++)
						n->AddFMenu(song->playbacksettings.CreateFromToString(i,Seq_Marker::MARKERTYPE_DOUBLE),new menu_selectcycle(song,i),song->playbacksettings.activecycle==i?true:false);
				}

				AddSetMarkerPositions(song,n);
				n->AddLine();
			}


			if(n=menu->AddMenu("Editor",0))
			{
				//	n->AddFMenu("Arrange",new globmenu_Arrange);

				if(screen->GetSong())
				{
					n->AddFMenu("Song: Mixer",new globmenu_AudioMixer(0,0,screen),"F7");
					n->AddFMenu("Song: Editor",new globmenu_Editor(screen),"F8");

					n->AddFMenu("Song: Tempo Map Editor",new globmenu_TMap(screen));
					n->AddFMenu("Song: Text Editor",new globmenu_TEd(0));
					n->AddFMenu("Song: Marker Editor ",new globmenu_Marker(0));
					n->AddLine();

					n->AddFMenu("Song: CamX Screen/Arrange Editor ",new globmenu_Arrange(screen->GetSong()));
					n->AddLine();

					n->AddFMenu("Song: Audio Mastering/Bounce Tracks",new globmenu_AMaster);
					n->AddFMenu("Song: Audio Freeze/UnFreeze Tracks",new globmenu_AMaster(true));

					n->AddLine();
				}
			}

			if(n=menu->AddMenu("Tools",0))
			{
				n->AddFMenu("Toolbox",new globmenu_Toolbox(screen));
				n->AddFMenu(Cxs[CXS_CPUUSAGE_EDITOR],new globmenu_cpu);

				n->AddLine();

				n->AddFMenu("Keyboard",new globmenu_Keyboard);
				n->AddFMenu("Big Time Display",new globmenu_Bigtime);

				class menu_AManager:public guiMenu
				{
					void MenuFunction()
					{
						guiWindow *win=maingui->FindWindow(EDITORTYPE_AUDIOMANAGER,0,0);

						if(win)
							win->WindowToFront(true);
						else
							maingui->OpenEditorStart(EDITORTYPE_AUDIOMANAGER,0,0,0,0,0,0);
					}
				};

				n->AddFMenu("Audio Manager",new menu_AManager);

				n->AddLine();
				n->AddFMenu("Event Generator",new globmenu_RMG);

				class menu_Libary:public guiMenu
				{
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_LIBRARY,0,0,0,0,0,0);
					}
				};

				class menu_GpEditor:public guiMenu
				{
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_GROUP,mainvar->GetActiveSong(),0,0,0,0,0);
					}
				};
				n->AddFMenu("Group Editor",new menu_GpEditor);

				class menu_ProcEditor:public guiMenu
				{
					void MenuFunction()
					{
						guiWindow *win=maingui->FindWindow(EDITORTYPE_PROCESSOR,0,0);

						if(!win)
							maingui->OpenEditorStart(EDITORTYPE_PROCESSOR,0,0,0,0,0,0);
						else
							win->WindowToFront(true);
					}
				};
				n->AddFMenu("Processor Editor",new menu_ProcEditor);

				class menu_Keymap:public guiMenu
				{
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_KEYMAP,0,0,0,0,0,0);
					}
				};
				n->AddFMenu("Note Key Map Editor",new menu_Keymap);

				class menu_Player:public guiMenu
				{
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_PLAYER,mainvar->GetActiveSong(),0,0,0,0,0);
					}
				};
				n->AddFMenu(("Song Player"),new menu_Player);

				class menu_Monitor:public guiMenu
				{
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_MONITOR,mainvar->GetActiveSong(),0,0,0,0,0);
					}
				};
				n->AddFMenu(("MIDI Monitor"),new menu_Monitor);

				n->AddLine();
				n->AddFMenu(("Library"),new menu_Libary);

				class menu_GEditor:public guiMenu
				{
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_GROOVE,mainvar->GetActiveSong(),0,0,0,0,0);
					}
				};
				n->AddFMenu(("Groove Editor"),new menu_GEditor);
			}

			n=menu->AddMenu("CamX",0);
			if(n)
			{
				class menu_AboutInfo:public guiMenu
				{
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_CAMXINFO,0,0,0,0,0,0);
					}
				};
				n->AddFMenu(Cxs[CXS_ABOUTCAMX],new menu_AboutInfo);

				class menu_Updata:public guiMenu
				{
					void MenuFunction()
					{
						maingui->OpenEditorStart(EDITORTYPE_UPDATE,0,0,0,0,0,0);
						//mainvar->updateflag=mainvar->CheckForUpdates(true);
					}
				};
				n->AddFMenu(Cxs[CXS_CHECKFORUPDATES],new menu_Updata);
			}

			ConvertGUIMenuToOSMenu(0,screen->menu);
		}
	}
}

bool GUI::OpenScreen(guiScreen *screen,bool all)
{
	bool ok=true;

	if(all==true)
		screen=FirstScreen();

	CreateScreenMenu(screen,0);

	if(screen && ok==true)
	{
		if(screen->open==false)
		{
			if(screen->width<screen->minwidth)
				screen->width=screen->minwidth;

			if(screen->height<screen->minheight)
				screen->height=screen->minheight;

			int flagex=0;
			int flag= 
				WS_CAPTION|
				WS_SYSMENU|
				WS_THICKFRAME|
				WS_MINIMIZEBOX|
				WS_MAXIMIZEBOX|
				WS_THICKFRAME|
				WS_CLIPCHILDREN|
				WS_CLIPSIBLINGS;

			//	WS_VISIBLE;

			RECT size;

			size.left=screen->fx;
			size.right=screen->fx+screen->width;
			size.top=screen->fy;
			size.bottom=screen->fy+screen->height;

			if(screen->maximized==false && screen->minimized==false)
			{
				AdjustWindowRectEx(&size,flag,TRUE,flagex);
			}

#ifdef WIN32
			screen->hWnd = CreateWindowEx(
				flagex,
				CAMX_SCREENNAME,
				screen->CreateScreenName(),
				flag,
				size.left,
				size.top,
				size.right-size.left,
				size.bottom-size.top,  
				0,
				screen->menu?screen->menu->OSMenuHandle:0, // Screen Menu Global
				hInst,
				screen
				);
#endif

			if(screen->hWnd){
				//	ShowWindow( screen->hWnd, SW_SHOW );
				//	ShowWindow( screen->hWndClient, SW_SHOW );
				ok=true;
				screen->open=true;

				//				SetWindowPos(screen->hWnd, HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW);

				if(screen==FirstScreen())
				{
					SetTimer(screen->hWnd, NULL,USER_TIMER_MINIMUM ,NULL);
				}

				if(screen->maximized==true)
					ShowWindow(screen->hWnd,SW_SHOWMAXIMIZED);
				/*
				else
				if(screen->minimized==true)
				ShowWindow(screen->hWnd,SW_SHOWMINIMIZED);
				*/
				else
					ShowWindow(screen->hWnd,SW_SHOWNORMAL);
			}
			else{
				maingui->MessageBoxError(NULL,"Cant open Desktop");
				ok=false;
			}
		}

		//if(all==false)
		//	break;

		//screen=screen->NextScreen();
	}

	return ok;
}

void AddChildMenus(guiMenu *menu,guiMenu *parent,int *index)
{
	if(guiMenu *child=menu->FirstMenu())
	{
		menu->OSMenuHandle=CreatePopupMenu();

		while(child)
		{
			child->index=*index;
			*index+=1;

			AddChildMenus(child,menu,index);

			if(child->FirstMenu())
				AppendMenu(menu->OSMenuHandle, child->disable==true?MF_DISABLED|MF_STRING|MF_POPUP:MF_STRING|MF_POPUP, (UINT_PTR)child->OSMenuHandle, child->name);
			else
			{
				int flag=MF_STRING;

				if(child->flag==SELECTED_MENU_ON)
					flag|=MF_CHECKED;

				if(child->disable==true)
					flag|=MF_DISABLED;

				AppendMenu(menu->OSMenuHandle, flag, (UINT_PTR)child->index+5000, child->name);

				//TRACE ("Child Index \n",child->index+5000);
			}

			child=child->NextMenu();
		}
	}
}

bool GUI::ConvertGUIMenuToOSPopMenu(guiMenu *menu)
{
	if(!menu)
		return false;

	int index=0;
	AddChildMenus(menu,0,&index);
	return true;
}

void GUI::ConvertMIDI2String(char *str,int status,int b1,int b2,LMIDIEvents *lMIDI)
{
	char nrs[128],*h;
	UBYTE channel=(status&0x0F)+1;

	*str=0;

	if((status&0xF0)==NOTEON && b2==0)
		status=NOTEOFF|(status&0x0F);

	switch(status&0xF0)
	{
	case NOTEON:
		if(mainvar->GetActiveSong())
		{
			if(b2==-1)
			{
				strcpy(str,"Note [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				mainvar->AddString(str,maingui->ByteToKeyString(mainvar->GetActiveSong(),b1));
				mainvar->AddString(str," ]");
			}
			else
			{
				strcpy(str,"Note [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				mainvar->AddString(str,maingui->ByteToKeyString(mainvar->GetActiveSong(),b1));
				mainvar->AddString(str," ][ ");
				h=mainvar->ConvertIntToChar(b2,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
		}
		break;

	case NOTEOFF:
		if(mainvar->GetActiveSong())
		{
			if(b2==-1)
			{
				strcpy(str,"Off [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				mainvar->AddString(str,maingui->ByteToKeyString(mainvar->GetActiveSong(),b1));
				mainvar->AddString(str," ]");
			}
			else
			{
				strcpy(str,"Off [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				mainvar->AddString(str,maingui->ByteToKeyString(mainvar->GetActiveSong(),b1));
				mainvar->AddString(str," ][ ");
				h=mainvar->ConvertIntToChar(b2,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
		}
		break;

	case PITCHBEND:
		{
			if(b2==-1)
			{
				strcpy(str,"Pitchbend [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
			else
			{
				strcpy(str,"Pitchbend [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," [ ");

				int i;

				i=128*b2; //msb
				i+=b1; // lsb

				i-=8192;

				h=mainvar->ConvertIntToChar(i,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
		}
		break;

	case POLYPRESSURE:
		{
			if(b2==-1)
			{
				strcpy(str,"PolyPress [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				mainvar->AddString(str,maingui->ByteToKeyString(mainvar->GetActiveSong(),b1));
				mainvar->AddString(str," ]");
			}
			else
			{
				strcpy(str,"PolyPress [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				mainvar->AddString(str,maingui->ByteToKeyString(mainvar->GetActiveSong(),b1));
				mainvar->AddString(str," ][ ");

				h=mainvar->ConvertIntToChar(b2,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
		}
		break;

	case CONTROLCHANGE:
		{
			if(b2==-1)
			{
				strcpy(str,"Control [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				h=maingui->ByteToControlInfo(b1,b2,true);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
			else
			{
				strcpy(str,"Control [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				h=mainvar->ConvertIntToChar(b2,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str,":");

				h=maingui->ByteToControlInfo(b1,b2);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
		}
		break;

	case PROGRAMCHANGE:
		{
			if(b2==-1)
			{
				strcpy(str,"Program [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
			else
			{
				strcpy(str,"Program [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				h=mainvar->ConvertIntToChar(b1+1,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
		}
		break;

	case CHANNELPRESSURE:
		{
			if(b2==-1)
			{
				strcpy(str,"ChlPress [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
			else
			{
				strcpy(str,"ChlPress [ ");

				h=mainvar->ConvertIntToChar(channel,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ][ ");

				h=mainvar->ConvertIntToChar(b1,nrs);
				mainvar->AddString(str,h);
				mainvar->AddString(str," ]");
			}
		}
		break;

	case SYSEX:
		{
			strcpy(str,"SysEx ");

			if(lMIDI)
			{
				h=mainvar->ConvertIntToChar(lMIDI->datalength,nrs);
				mainvar->AddString(str,"Bytes:");
				mainvar->AddString(str,h);

				if(lMIDI->datalength>1)
				{
					mainvar->AddString(str,">");

					int a=lMIDI->datalength;

					if(a>MAXMONITORBYTES)
						a=MAXMONITORBYTES;

					mainvar->AddString(str,mainMIDI->GetSysExString(lMIDI->data,a));
				}
			}
		}
		break;

	default:
		strcpy(str,"???");
		break;
	}
}

void GUI::AppendPopUpMenu(guiMenu *menu1)
{
#ifdef WIN32

	//if(menu1->OSMenuHandle)
	//	DestroyMenu(menu1->OSMenuHandle);

	if(!menu1->OSMenuHandle)
		menu1->OSMenuHandle = CreatePopupMenu(); // Head

	if(!menu1->OSMenuHandle)
		return;

	{	
		guiMenu *menu2=menu1->FirstMenu();

		int index=0,index2;

		while(menu2)
		{
			guiMenu *menu3=menu2->FirstMenu();

			if(menu3)
			{
				menu2->OSMenuHandle= CreatePopupMenu(); // --->

				if(!menu2->OSMenuHandle)
					return;

				index2=0;
				while(menu3)
				{	
					switch(menu3->flag)
					{
					case SELECTED_MENU_ON:
					case SELECTED_MENU_OFF:
						{
							int flag=MF_STRING;

							if(menu3->flag==SELECTED_MENU_ON)
								flag|=MF_CHECKED;

							if(menu3->disable==true)
								flag|=MF_DISABLED|MF_GRAYED;

							AppendMenu(menu2->OSMenuHandle, flag, menu3->id, menu3->name);
						}
						break;

					case STANDARD_MENU:
						AppendMenu(menu2->OSMenuHandle, menu3->disable==true?MF_STRING|MF_DISABLED|MF_GRAYED:MF_STRING,menu3->id, menu3->name);
						break;

					case MENU_LINE: // Line
						if(menu3->NextMenu())
							AppendMenu(menu2->OSMenuHandle, MF_SEPARATOR, 0, 0);
						break;
					}

					// AppendMenu(hSubMenu2, MF_STRING, menu3->id, menu3->name);

					menu3->index=index2++;
					menu3=menu3->NextMenu();
				}//while menu3			

				AppendMenu(menu1->OSMenuHandle, MF_STRING|MF_POPUP, (UINT_PTR)menu2->OSMenuHandle, menu2->name);

			}//if menu3
			else
			{
				switch(menu2->flag)
				{
				case SELECTED_MENU_ON:
				case SELECTED_MENU_OFF:
					{
						int flag=MF_STRING;

						if(menu2->flag==SELECTED_MENU_ON)
							flag|=MF_CHECKED;

						if(menu2->disable==true)
							flag|=MF_DISABLED|MF_GRAYED;

						AppendMenu(menu1->OSMenuHandle, flag, menu2->id, menu2->name);
					}
					break;

				case STANDARD_MENU:
					AppendMenu(menu1->OSMenuHandle, menu2->disable==true?MF_STRING|MF_DISABLED|MF_GRAYED:MF_STRING, menu2->id, menu2->name);
					break;

				case MENU_LINE: // Line
					if(menu2->NextMenu())
						AppendMenu(menu1->OSMenuHandle, MF_SEPARATOR, 0, 0);
					break;
				}
			}

			menu2->index=index++;
			menu2=menu2->NextMenu();
		}// while vmenu1

	}//if 


#endif			
}

bool GUI::ConvertGUIMenuToOSMenu(guiWindow *win,guiMenu *menu)
{
	if(!menu)
		return false;

#ifdef WIN32
	//HMENU hSubMenu,hSubMenu2;
	//	HICON hIcon, hIconSm;

	menu->OSMenuHandle= CreateMenu();

#ifdef _DEBUG
	if(!menu->OSMenuHandle)
		maingui->MessageBoxOk(0,"No Menu");
#endif

	if(menu->OSMenuHandle)	
#endif
	{
		guiMenu *menu1=menu->FirstMenu();

		// m1...m1
		// m2
		// m2....m3

		while(menu1)
		{		
			AppendPopUpMenu(menu1);

			int flag=MF_STRING|MF_POPUP;

			if(menu1->flag==SELECTED_MENU_ON)
				flag|=MF_CHECKED;

			if(menu1->disable==true)
				flag|=MF_DISABLED|MF_GRAYED;

			AppendMenu(menu->OSMenuHandle, flag, (UINT_PTR)menu1->OSMenuHandle, menu1->name);

			menu1=menu1->NextMenu();
		}// while vmenu	
	}

	return true;
}

int guiWindow::ConvertWindowDisplayToTimeMode()
{
	switch(windowtimeformat)
	{
	case WINDOWDISPLAY_MEASURE:
		return Seq_Pos::POSMODE_NORMAL;
		break;

	case WINDOWDISPLAY_SECONDS:
		return Seq_Pos::POSMODE_TIME;
		break;

	case WINDOWDISPLAY_SMPTE:
		//	case WINDOWDISPLAY_SMPTEANDMEASURE:

		if(WindowSong())
			return WindowSong()->project->standardsmpte;

		return mainsettings->projectstandardsmpte;
		break;

	case WINDOWDISPLAY_SAMPLES:
		return Seq_Pos::POSMODE_SAMPLES;
		break;
	}

	return 1;
}

void guiWindow::DrawHeader()
{
	if(timeline && timeline->dbgadget)
		timeline->dbgadget->DrawGadgetBlt();
}

void GUI::InitStandardColour(
#ifdef WIN32
							 HDC hDC
#endif
							 )
{
#ifdef WIN32
	SelectObject(hDC,standardfont.hfont); // Set Font
	SelectObject(hDC,colour_hPen[COLOUR_BLACK]); // APen
	SelectObject(hDC,colour_hBrush[COLOUR_BACKGROUND]); // Fill

	UBYTE r,g,b;
	colourtable.GetRGB(COLOUR_BLACK,&r,&g,&b);

	SetTextColor(
		hDC,           // handle to device context
		RGB(r,g,b)   // text color
		);

	colourtable.GetRGB(COLOUR_BACKGROUND,&r,&g,&b);

	SetBkColor(hDC,RGB(r,g,b)); // SetBackGround
	// SetBkMode(bm->hDC,TRANSPARENT);
#endif					
}

void GUI::LoadSettings()
{
	if(char *filename=mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_GUI))
	{
		camxFile file;

		if(file.OpenRead(filename)==true && file.CheckVersion()==true)
		{
			bool ok=false;

			file.LoadChunk();

			//maingui->MessageBoxOk(0,"LoadSet");

			if(file.GetChunkHeader()==CHUNK_SETTINGSHEADER)
			{
				file.ChunkFound();

				char text[4];

				file.ReadChunk(text,4);

				if(text[0]=='C' &&
					text[1]=='A' &&
					text[2]=='S' &&
					text[3]=='X')
					ok=true;
				else
					maingui->MessageBoxError(0,"GUI Settings File (C-A-S-X)");

				file.CloseReadChunk();
			}
			else
				maingui->MessageBoxError(0,"GUI Settings File (HEADER)");

			if(ok==true)
			{
				bool screensfound=false,end=false;

				while(end==false && file.eof==false)
				{
					file.LoadChunk();

					if(file.eof==false)
						switch(file.GetChunkHeader())
					{

						case CHUNK_SETTINGSOSSCREENS:
							{
								file.ChunkFound();
								int nr=0;
								file.ReadChunk(&nr);
								file.CloseReadChunk();

								if(nr)
								{
									while(nr--)
									{
										file.LoadChunk();

										if(file.GetChunkHeader()==CHUNK_FORM)
										{
											file.ChunkFound();

											guiScreen *screen=new guiScreen;
											if(!screen)
												break;

											screen->ReadForm(&file);

											if(!FirstScreen())
												AddScreen(screen);
											else
												delete screen;
										}
									}

									screensfound=true;
								}
							}
							break;

						case CHUNK_SETTINGSGUIWINDOWS:
							{
								//maingui->MessageBoxOk(0,"SW1");

								file.ChunkFound();
								file.CloseReadChunk();

								for(;;)
								{
									file.LoadChunk();
									if(file.GetChunkHeader()==CHUNK_SETTINGSGUIWINDOW)
									{
										file.ChunkFound();
										if(SettingsWindow *sw=new SettingsWindow)
										{
											file.ReadChunk(&sw->old);
											file.ReadChunk(&sw->id);
											file.ReadChunk(&sw->x);
											file.ReadChunk(&sw->y);
											file.ReadChunk(&sw->width);
											file.ReadChunk(&sw->height);

											setwindows.AddEndO(sw);
										}

										file.CloseReadChunk();
									}
									else
									{
										end=true;
										break;
									}
								}

								//maingui->MessageBoxOk(0,"SW2");

							}
							break;

						default: // unknown 
							TRACE ("Unknown GUI Settings CHUNK %d\n",file.GetChunkHeader());
							file.JumpOverChunk();
							break;
					}
				}
			}
		}
#ifdef _DEBUG
		else
			MessageBox(NULL,"No GUI Settings File found","Error",MB_OK_STYLE);
#endif

		file.Close(true);	
	}
}

void GUI::SaveSettings()
{
	char *filename=mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_GUI);

	if(filename){
		camxFile file;

		if(file.OpenSave_CheckVersion(filename)==true)
		{
			file.SaveVersion();

			// Header
			file.OpenChunk(CHUNK_SETTINGSHEADER);
			file.Save_Chunk("CASX",4);
			file.CloseChunk();

			// Pointer
			file.OpenChunk(CHUNK_SETTINGSOSSCREENS);
			file.Save_Chunk(screens.GetCount());
			file.CloseChunk();

			// 1. Screens
			guiScreen *s=FirstScreen();
			while(s)
			{
				s->SaveForm(&file);
				s=s->NextScreen();
			}

			// 2. Windows
			file.OpenChunk(CHUNK_SETTINGSGUIWINDOWS);
			file.CloseChunk();

			guiWindow *w=maingui->FirstWindow();
			while(w)
			{
				if(w->SaveAble()==true)
				{
					file.OpenChunk(CHUNK_SETTINGSGUIWINDOW);

					file.Save_Chunk((CPOINTER)w);
					file.Save_Chunk(w->editorid);
					file.Save_Chunk(w->win_screenposx);
					file.Save_Chunk(w->win_screenposy);
					file.Save_Chunk(w->width);
					file.Save_Chunk(w->height);

					file.CloseChunk();
				}

				w=w->NextWindow();
			}

			file.Close(true);	
		}
#ifdef _DEBUG
		else
			MessageBox(NULL,"Unable to Open Settings File for saving","Error",MB_OK_STYLE);
#endif
	}
}

void GUI::RefreshProgress()
{
	if(Progress *p=mainvar->FirstProgress())
	{
		char *string=0;

		if(p->working==true)
		{
			char help[NUMBERSTRINGLEN];

			if(p->value==true)
				string=mainvar->GenerateString(p->infostring?p->infostring:Cxs[CXS_WORKING]," ",mainvar->ConvertIntToChar(p->p_value,help));
			else
				string=mainvar->GenerateString(p->infostring?p->infostring:Cxs[CXS_WORKING]," ",mainvar->ConvertDoubleToChar(p->percent,help,2)," %");
		}
		else
			string=mainvar->GenerateString(p->infostring?p->infostring:Cxs[CXS_WORKING],":",Cxs[p->p_stopped==true?CXS_CANCELED:CXS_DONE]);

		if(string)
		{
			LockProgressString();
			if(progressstring)
				delete progressstring;

			progressstring=string;
			UnlockProgressString();

			//delete string;
			refreshprogress=true;
		}
	}
}

void GUI::ShowRefreshProgress()
{
	LockProgressString();
	char *string=mainvar->GenerateString(progressstring);
	UnlockProgressString();

	if(string)
	{
		guiWindow *w=FirstWindow();

		while(w)
		{
			if(w->GetEditorID()==EDITORTYPE_TRANSPORT)((Edit_Transport *)w)->ShowProgress(string);
			w=w->NextWindow();
		}
		delete string;
	}
}

void GUI::RecalcSize(guiWindow *win)
{
#ifdef WIN32

	RECT rect;
	GetWindowRect(win->hWnd,&rect);
	RECT client;
	GetClientRect(win->hWnd,&client);

	int winw=rect.right-rect.left;
	int clientw=client.right-client.left;

	int winh=rect.bottom-rect.top;
	int clienth=client.bottom-client.top;

	SetWindowPos(win->hWnd,NULL,0,0,win->glist.form->maxx2+(winw-clientw),win->glist.form->maxy2+(winh-clienth)+2,SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
#endif
}

guiWindow *GUI::CloseWindow(guiWindow *win,bool mouseclose,bool dontactivate)
{	
	if(!win)return 0;

	guiWindow *n=0;
	win->winmode=WINDOWMODE_DESTROY; // Exit

	guiWindow *w=FirstWindow();
	while(w)
	{
		if(w->calledfrom==win)
		{
			CloseWindow(w);
			w=FirstWindow();
		}
		else
			w=w->NextWindow();
	}

	if(dragdropwindow==win)
		dragdropwindow=0;

	win->glist.RemoveGadgetList();

	if(win->menu){
		win->menu->RemoveMenu();
		win->menu=0;
	}

	if(win->popmenu){
		win->popmenu->RemoveMenu();
		win->popmenu=0;
	}

	// Remove From List
	if(win->GetList()){
		n=(guiWindow *)win->GetList()->CutObject(win);
		win->SetList(0);
	}

	mainhelpthread->RemoveWindowFromMessages(win); // Remove Window

	if(win->parentformchild && win->parentformchild->bindwindow==win)
	{
		win->parentformchild->bindwindow=0;
	}

	win->RemoveChildWindows();

#ifdef WIN32
	if(win->hWnd)
	{		
		ReleaseDC(win->hWnd,win->bitmap.hDC);

		if(win->hWnd)
		{
			DestroyWindow(win->hWnd); // Windows send a INACTIVE Message !!!
			win->hWnd=0;
		}
	}
#endif

	if(win->screen && win->screen->closeit==false)
		win->screen->WindowClosed(win);

	win->DeInitWindow(); // Free Memory ..

	if(win->isstatic==false)
		delete win;

	return n;
}

guiWindow *GUI::OpenWindow(guiWindow *win,guiWindowSetting *set,Object *object,int flag)
{
	guiWindowSetting settings;
	guiScreen *toscreen=0;

	if(!set)
		set=&settings;

	toscreen=set->screen;

	if((!toscreen) && set->bindtoform==0)
		toscreen=maingui->GetActiveScreen();

	//set->formx=-1;

	if(allwindowsoondesktop==true)
		set->s_ondesktop=true;

	if(set->s_ondesktop==false)
	{
		if((!toscreen) && set->bindtoform==0)
		{
			maingui->MessageBoxError(0,"OpenWin #167");
			return 0;
		}
	}

	if(!win) // Create New Window ?
	{
		win=new guiWindow;
	}

	if(!win)
	{
		maingui->MessageBoxError(0,"OpenWin #168");
		return 0;
	}

	if(win->ondesktop==false)
		win->ondesktop=set->s_ondesktop;

	win->openscreen=toscreen;

	if((!set) || win->ondesktop==false)
		win->screen=toscreen;

	win->style=0;
	win->flagex=WS_EX_ACCEPTFILES;

	if(set && set->bindtoform)
	{
		set->startposition_x=set->bindtoform->x;
		set->startposition_y=set->bindtoform->y;
		set->startwidth=set->bindtoform->GetWidth();
		set->startheight=set->bindtoform->GetHeight();

		set->formx=set->bindtoform->fx;
		set->formy=set->bindtoform->fy;

		win->flagex|=WS_EX_NOPARENTNOTIFY;
	}
	else
	{
		if(win->dontchangesettings==false &&
			win->editorid>=0 && 
			mainsettings->windowpositions[win->editorid].set==true
			)
		{
			if(set->formx!=-1)
			{
				// Window -> Screen
				guiForm_Child *form=&toscreen->forms[set->formx][set->formy];

				form->Enable(true);

				set->startposition_x=form->x;
				set->startposition_y=form->y;

				set->startwidth=form->width;
				set->startheight=form->height;
			}
			else
			{
				// Desktop Window

				set->startposition_x=mainsettings->windowpositions[win->editorid].x;
				set->startposition_y=mainsettings->windowpositions[win->editorid].y;

				set->startwidth=mainsettings->windowpositions[win->editorid].width;
				set->startheight=mainsettings->windowpositions[win->editorid].height;

				if(win->minwidth==win->maxwidth)
				{
					if(win->minwidth>0)
						set->startwidth=win->minwidth;
				}
				else
				{
					if(win->minwidth && set->startwidth<win->minwidth)
						set->startwidth=win->minwidth;
					else
						if(win->maxwidth && set->startwidth>win->maxwidth)
							set->startwidth=win->maxwidth;
				}

				if(win->minheight==win->maxheight)
				{
					if(win->minheight>0)
						set->startheight=win->minheight;
				}
				else
				{
					if(win->minheight && set->startheight<win->minheight)
						set->startheight=win->minheight;
					else
						if(win->maxheight && set->startheight>win->maxheight)
							set->startheight=win->maxheight;
				}

				set->maximized=mainsettings->windowpositions[win->editorid].maximized;
			}
		}
	}

	if(set->startposition_x<0)set->startposition_x=0;
	if(set->startposition_y<0)set->startposition_y=0;

	if(win->ondesktop==true)
	{
		//	win->flagex=
		if(win->dialogstyle==true || win->resizeable==false)
		{
			if(win->resizeable==true)
			{
				win->style=WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
			}
			else
			{
				if(set->noOSborder==true)
					win->style=WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
				else
					win->style=WS_CAPTION|WS_SYSMENU|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
			}
		}
		else
		{
			win->style=WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
		}
	}
	else
	{
		if(set->formx!=-1)
			win->style|=WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
		else
		{
			if(set->parent_hwnd)
			{
				win->flagex=WS_EX_TOPMOST|WS_EX_NOPARENTNOTIFY;
				win->style=WS_CHILD|WS_VISIBLE;
			}
			else
			{
				if(win->resizeable==true)
				{
					win->style=WS_THICKFRAME|WS_SYSMENU|WS_CAPTION;
					win->flagex=WS_EX_WINDOWEDGE;
				}
				else
				{
					if(set->formx!=-1)
						win->style|=WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
					else
					{
						if(win->borderless==true)
						{
							if(set->title)
								win->style=WS_CAPTION;
							else
								win->style=WS_POPUP;
						}
						else
						{
							win->style=WS_SYSMENU|WS_CAPTION;
						}
					}
				}
			}
		}
	}

	RECT wsize;

	if(set->formx==-1 && win->ondesktop==true && win->dontchangesettings==false)
	{
		set->startposition_x=set->startposition_y=0;
	}

	wsize.left=set->startposition_x;
	wsize.top=set->startposition_y;
	wsize.right=set->startposition_x+set->startwidth;
	wsize.bottom=set->startposition_y+set->startheight;

	win->width=set->startwidth;
	win->height=set->startheight;

	if(win->horzscroll==true)
	{
		win->style|= /*WS_OVERLAPPEDWINDOW*/ WS_HSCROLL /*| WS_VSCROLL*/;
	}

	if(win->vertscroll==true)
	{
		win->style|= /*WS_OVERLAPPEDWINDOW*/ WS_VSCROLL /*| WS_VSCROLL*/;
	}

	if(win->autovscroll==true)
	{
		win->style|= /*WS_OVERLAPPEDWINDOW*/ WS_VSCROLL /*| WS_VSCROLL*/;
	}

	int initwidth=win->width,initheight=win->height;

	if(set->formx==-1)
	{
		AdjustWindowRectEx(&wsize,win->style,win->hasownmenu==true?TRUE:FALSE,win->flagex);

		if(wsize.top<0){
			int movedown=-wsize.top;
			wsize.top+=movedown;
			wsize.bottom+=movedown;
		}

		if(wsize.left<0){
			int moveright=-wsize.left;
			wsize.left+=moveright;
			wsize.right+=moveright;
		}

		//AdjustWindowPositionsToDesktop(&wsize);
		set->startposition_x=wsize.left;
		set->startposition_y=wsize.top;

		win->width=initwidth=wsize.right-wsize.left;
		win->height=initheight=wsize.bottom-wsize.top;	
	}
	else
	{
		if(set->bindtoform)
			win->parentformchild=set->bindtoform;
		else
			win->parentformchild=&toscreen->forms[set->formx][set->formy];
	}

	if(set->simple==false)
	{
		win->startposition=set->startposition;
		windows.AddEndO(win);
	}
	else
		win->SetList(0);

	HWND parent_hwnd=0;

	//	win->CreateWindowName();
	win->CreateWindowMenu();

	maingui->ConvertGUIMenuToOSMenu(win,win->menu);

	if(win->ondesktop==false)
	{
		if(set->bindtoform)
		{
			parent_hwnd=set->bindtoform->form->hWnd;
		}
		else
			if(set->formx!=-1)
			{
				parent_hwnd=toscreen->hWnd;
			}
			else
				parent_hwnd=set->parent_hwnd;
	}

	//win->hWnd = 
	CreateWindowEx
		(
		win->flagex,
		win->dialogstyle==true?CAMX_DIALOGNAME:CAMX_WINDOWNAME, 
		set->title?set->title:win->windowname,
		win->style,
		set->startposition_x,
		set->startposition_y,
		initwidth,
		initheight,  
		parent_hwnd,
		win->menu?win->menu->OSMenuHandle:0, // Window Menu
		hInst,
		win
		);

	if(win && win->hWnd)
	{	
		win->SetWindowName();

		if(win->windowname)
			win->guiSetWindowText(win->windowname);

		if(set->formx!=-1)
		{
			// Hide Old
			/*
			if(set->parentwnd)
			{
			set->parentwnd->forms[set->formx][set->formy].fhWnd=win->hWnd;
			set->parentwnd->forms[set->formx][set->formy].externwindow=true;
			}
			else
			*/
			if(!set->bindtoform)
			{
				if(toscreen->forms[set->formx][set->formy].fhWnd)
				{
					guiWindow *w=FirstWindow();
					while(w)
					{
						if(w!=win && w->hide==false && w->hWnd==toscreen->forms[set->formx][set->formy].fhWnd)
						{
							w->hide=true; // Stop Realtime Refresh etc..

							ShowWindow(win->hWnd,SW_SHOW);
							//UpdateWindow(win->hWnd);
#ifdef WIN32
							ShowWindow(w->hWnd,SW_HIDE);
#endif
							break;
						}

						w=w->NextWindow();
					}
				}

				toscreen->forms[set->formx][set->formy].fhWnd=win->hWnd;
				//toscreen->forms[set->formx][set->formy].externwindow=true;
			}
		}
		else
		{
			if(win->dialogstyle==true && win->resizeable==false)
				RecalcSize(win);
		}

		/*
		if(set->parent_hwnd)
		{
		SetWindowPos(win->hWnd, HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		ShowWindow(win->hWnd,set->noactivate==true?SW_SHOWNA:SW_SHOWNORMAL);
		}
		else
		*/
		{
			if(win->ondesktop==true)
				SetWindowPos(win->hWnd, HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW|SWP_NOACTIVATE);

			ShowWindow(win->hWnd,set->noactivate==true?SW_SHOWNA:SW_SHOWNORMAL);
			if(set->noactivate==false)
				SetFocus(win->hWnd);
		}

		//UpdateWindow(win->hWnd);
	}
#ifdef DEBUG
	else
		maingui->MessageBoxError(NULL,"Child Window");
#endif

	return win;
}

void GUI::CloseGUI()
{
	DeleteAllScreens();

	gfx.DeleteAllBitMaps();
	gfx.DeleteAllTrackIcons();

	setwindows.DeleteAllO();
	guimessages.DeleteAllO();

#ifdef WIN32
	for(int i=0;i<LASTCOLOUR;i++)
	{
		if(colour_hBrush[i])
			DeleteObject(colour_hBrush[i]);

		if(colour_hPen[i])
			DeleteObject(colour_hPen[i]);
	}
#endif
}

void GUI::CheckTimerMessage(guiWindow *win,bool leftmouse,bool rightmouse)
{
	//if(win->hide==false)
	//	win->EditPositions();

	if(win->mouseclickdowncounter)
		win->mouseclickdowncounter--;

	int mx,my;

	if(win->hide==false)
	{
		POINT wlpPoint;
		memcpy(&wlpPoint,&lpPoint,sizeof(POINT));

		BOOL r=ScreenToClient(win->hWnd,&wlpPoint);

		if(r==1)
		{
			mx=wlpPoint.x;
			my=wlpPoint.y;

			//TRACE ("MX %d MY %d\n",mx,my);
		}
	}
	else
		mx=my=-1;

	bool checktooltips=win->hide==true?false:win->IsFocusWindow();

	guiGadget *gadgetundermouse=0;

	// 1. Check Mouse Move
	{
		guiObject *o=win->guiobjects.FirstObject();
		while(o)
		{
			if(guiGadget *g=o->gadget)
			{
				if(g->on==true)
				{
					if(g->getMouseMove==true)
					{
						int ctflag=0;

						if(leftmouse==true)
							ctflag|=guiGadget::CGT_LEFTMOUSEDOWN;
						if(rightmouse==true)
							ctflag|=guiGadget::CGT_RIGHTMOUSEDOWN;

						g->CheckGadgetTimer(mx,my,o,ctflag);
						goto exit;
					}
				}
			}

			o=o->NextObject();
		}
	}

	// 2. 
	{
		guiObject *o=win->guiobjects.FirstObject();
		while(o)
		{
			if(guiGadget *g=o->gadget)
			{
				if(g->on==true)
				{
					int ctflag;

					if(checktooltips==true)
					{
						if(g->CheckMouseOver(mx,my)==true)
						{
							ctflag=guiGadget::CGT_INSIDE;
							gadgetundermouse=g;
						}
						else
							ctflag=0;
					}
					else
						ctflag=0;


					if(leftmouse==true)
						ctflag|=guiGadget::CGT_LEFTMOUSEDOWN;

					if(rightmouse==true)
						ctflag|=guiGadget::CGT_RIGHTMOUSEDOWN;

					g->CheckGadgetTimer(mx,my,o,ctflag);

					if(!g)
						goto exit;
				}
			}

			o=o->NextObject();
		}

		// 1. 
		for(int i=0;i<win->glist.gc;i++)
		{
			guiGadget *g=win->glist.gadgets[i];

			if(g->getMouseMove==true)
			{
				int ctflag=0;

				if(leftmouse==true)
					ctflag|=guiGadget::CGT_LEFTMOUSEDOWN;
				if(rightmouse==true)
					ctflag|=guiGadget::CGT_RIGHTMOUSEDOWN;

				g->CheckGadgetTimer(mx,my,o,ctflag);

				goto exit;
			}

		}//for int

		for(int i=0;i<win->glist.gc;i++)
		{
			guiGadget *g=win->glist.gadgets[i];
			int ctflag;

			if(checktooltips==true && g->hWnd==maingui->mousehWnd)
			{
				ctflag=guiGadget::CGT_INSIDE;
				gadgetundermouse=g;
				mx=g->GetMouseX();
				my=g->GetMouseY();

			}else
			{
				ctflag=0;
				mx=-1;
				my=-1;
			}

			if(g->ownergadget==true || g->IsDoubleBuffered()==true)
			{
				if(g->on==true)
				{
					if(leftmouse==true)
						ctflag|=guiGadget::CGT_LEFTMOUSEDOWN;

					if(rightmouse==true)
						ctflag|=guiGadget::CGT_RIGHTMOUSEDOWN;

					g->Timer();

					g=g->CheckGadgetTimer(mx,my,0,ctflag);

					if(!g)
						goto exit;
				}

			}//if owner

		}//for int
	}

exit:
	// Delete unused ToolTips
	guiObject *o=win->guiobjects.FirstObject();
	while(o)
	{
		if(guiGadget *g=o->gadget)
		{
			if(g->on==true)
			{
				if(g!=gadgetundermouse && g->ttWnd)
				{
					DestroyWindow(g->ttWnd);
					g->ttWnd=0;
				}
			}
		}

		o=o->NextObject();
	}

	for(int i=0;i<win->glist.gc;i++)
	{
		guiGadget *g=win->glist.gadgets[i];

		if(g!=gadgetundermouse && g->ttWnd)
		{
			DestroyWindow(g->ttWnd);
			g->ttWnd=0;
		}

	}//for int

	if(gadgetundermouse)
		gadgetundermouse->SetToolTip();
}

void GUI::SendGUIMessage(int par1,void *par2)
{
	if(GUIMessage *newmsg=new GUIMessage)
	{
		newmsg->parm=par1;
		newmsg->ptr=par2;

		LockGUIMessages();
		guimessages.AddEndO(newmsg);
		UnlockGUIMessages();
	}
}

void GUI::SendGUIMessage(guiWindow *win,int par1,void *par2)
{
	if(win)
	{
#ifdef WIN32
		SendMessage(win->hWnd,WM_USER,par1,(LPARAM)par2);
#endif
	}		
}	

void GUI::CheckUserMessage(guiWindow *win,int type,void *par,GUIMessage *msg)
{
	//win can be 0

	switch(type)
	{
	case MESSAGE_AUDIOFILEWORKED:
		{
			mainaudio->CheckWorkedFiles();
		}
		break;

	case MESSAGE_DELETESONG:
		if(par)
		{
			Seq_Song *song=(Seq_Song *)par;
			song->project->DeleteSong(song,Seq_Project::DELETESONG_FULL);
		}
		break;

	case MESSAGE_REFRESHSAMPLEEDITOR:
		if(par)
		{
			guiWindow *ewin=(guiWindow *)par;

			if(ewin->GetEditorID()==EDITORTYPE_SAMPLE)
				((Edit_Sample *)ewin)->UserMessage(MESSAGE_REFRESHSAMPLEEDITOR); // sample stop
		}
		break;

	case MESSAGE_REFRESHFREEZETRACK:
		if(par)
		{
			Seq_Track *track=(Seq_Track *)par;

			win=FirstWindow();

			while(win)
			{
				switch(win->GetEditorID())
				{
				case EDITORTYPE_ARRANGE:
					{
						Edit_Arrange *ar=(Edit_Arrange *)win;
						Edit_Arrange_Track *eat;

						if(eat=ar->FindTrack(track))
						{
							// Refresh Track
							ar->ShowHoriz(true,false,true);
						}
					}
					break;
				}

				win=win->NextWindow();
			}
		}
		break;

		/*
		case MESSAGE_REFRESHPEAKBUFFER:
		{
		guiWindow *guiw=maingui->FirstWindow();
		AudioPeakBuffer *apb=(AudioPeakBuffer *)par;

		if(apb)
		{
		while(guiw)
		{
		switch(guiw->GetEditorID())
		{
		case EDITORTYPE_ARRANGE:
		((Edit_Arrange *)guiw)->RefreshAudio(0,apb);
		break;

		case EDITORTYPE_AUDIOMANAGER:
		{
		Edit_Manager *ed=(Edit_Manager *)guiw;

		if(ed->activehdfile && (strcmp(ed->activehdfile->name,apb->samplefilename)==0))
		ed->ShowActiveHDFile();
		}
		break;
		}

		guiw=guiw->NextWindow();
		}
		}
		}
		break;
		*/

	case MESSAGE_REFRESHAUDIOHDFILE: // message from Peak Thread
		{		
			maingui->RefreshAudioHDFile((AudioHDFile *)par,0);
		}
		break;

	case MESSAGE_CHECKMOUSEBUTTON:
		if(win)
			win->UserMessage(MESSAGE_CHECKMOUSEBUTTON,par);
		break;
	}
}

void GUI::AddNewSongToGUI(Seq_Song *song)
{
	guiWindow *w=FirstWindow();

	while(w)
	{	
		switch(w->GetEditorID())
		{
		case EDITORTYPE_PLAYER:
			{
				Edit_Player *p=(Edit_Player *)w;

				if(p->activeproject==song->project)
					p->ShowSongs();
			}
			break;
		}

		w=w->NextWindow();
	}
}

void GUI::RemoveSongFromGUI(guiScreen *screen,Seq_Song *song)
{
	if(!song)
		return;

	int cwithothersongs=0;
	int cwithsong=0;

	guiScreen *s=FirstScreen();

	while(s)
	{
		if(s!=screen)
		{
			if(s->song==song)
				cwithsong++;
			else
				cwithothersongs++;
		}

		s=s->NextScreen();
	}

	//OpenScreenRefresh();

	s=FirstScreen();

	while(s)
	{
		if(s->closeit==false && s->song==song)
		{
			if(cwithothersongs || cwithsong>1)
			{
				cwithsong--;
				DeleteScreen(s);
				s=FirstScreen();
			}
			else
			{
				s->SetNewSong(0);
				s=s->NextScreen();
			}
		}
		else
			s=s->NextScreen();
	}

	guiWindow *w=LastWindow();

	while(w)
	{
		if(w->WindowSong()==song)
			w->RemoveSong(song); // Big Time etc.

		if(w->closeit==false && w->WindowSong()==song && w->CanbeClosed()==true)
		{
			CloseWindow(w);
			w=LastWindow();
		}
		else
			w=w->PrevWindow();
	}

	//CloseScreenRefresh();
}

void GUI::CloseScreen(guiScreen *screen)
{
	guiWindow *w=FirstWindow();
	while(w)
	{
		if(w->GetEditorID()==EDITORTYPE_TOOLBOX)
		{
			Edit_Toolbox *et=(Edit_Toolbox *)w;

			if(et->fromscreen==screen)
			{
				CloseWindow(w);
				goto step2;
			}
		}

		w=w->NextWindow();
	}

step2:
	bool otherprojectscreen=false;
	bool othersongscreen=false;

	guiScreen *s=FirstScreen();
	while(s)
	{
		if(s!=screen && s->project==screen->project)
			otherprojectscreen=true;

		if(s!=screen && s->song==screen->song)
			othersongscreen=true;

		s=s->NextScreen();
	}

	if(screen==FirstScreen() && screen==LastScreen())
	{
		mainvar->AskQuitMessage();
	}
	else
	{
		if(otherprojectscreen==true && screen->song==0)
		{
			DeleteScreen(screen);
			return;
		}

		if((otherprojectscreen==true && othersongscreen==true) || screen->project==0)
		{
			DeleteScreen(screen);
			return;
		}

		if(otherprojectscreen==true && othersongscreen==false && screen->song)
		{
			if(char *h=mainvar->GenerateString(Cxs[CXS_CLOSESONG_Q],"\n","Song:",screen->song->GetName()))
			{
				if(maingui->MessageBoxYesNo(0,h)==true)
				{
					mainedit->DeleteSong(screen,screen->song,0);
				}
				delete h;
			}
		}
		else
			if(char *h=mainvar->GenerateString(Cxs[CXS_QUESTIONCLOSEPROJECTSCREEN],"\n",Cxs[CXS_PROJECT],":",screen->project->name))
			{
				if(MessageBoxYesNo(0,h)==true)
					DeleteScreen(screen);

				delete h;
			}
	}
}

void GUI::StopRegionInGUI(AudioRegion *region)
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		switch(w->GetEditorID())
		{
		case EDITORTYPE_ARRANGE:
			{
				/*
				Edit_Arrange *ar=(Edit_Arrange *)w;

				if(ar->activepattern && ar->activepattern->mediatype==MEDIATYPE_AUDIO)
				{
				AudioPattern *ap=(AudioPattern *)ar->activepattern;

				if(ap->audioevent.audioregion==region)
				ar->activepattern=0;
				}
				*/
			}
			break;

			/*
			case EDITORTYPE_SAMPLE:
			{
			Edit_Sample *es=(Edit_Sample *)w;

			if(es->activeregion==region)
			es->StopPlayback();
			}
			break;
			*/

		case EDITORTYPE_AUDIOMANAGER:
			{
				Edit_Manager *em=(Edit_Manager *)w;

				if(em->activeregion==region)
				{
					em->StopPlayback();
					em->activeregion=region->NextOrPrev();
				}
			}
			break;
		}

		w=w->NextWindow();
	}
}

void GUI::RemoveAudioHDFileFromGUI(AudioHDFile *hd)
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		guiWindow *n=w->NextWindow();

		switch(w->GetEditorID())
		{
		case EDITORTYPE_SAMPLE:
			{
				Edit_Sample *es=(Edit_Sample *)w;

				if(es->audiohdfile==hd)
					CloseWindow(w);
			}
			break;

		case EDITORTYPE_AUDIOMANAGER:
			{
				Edit_Manager *em=(Edit_Manager *)w;

				if(em->activefile && em->activefile->hdfile==hd)
				{
					//em->activefile=(Sort *)em->->NextOrPrev();
					//em->refreshgui=true;

					/*
					em->ShowAudioFiles();
					em->ShowActiveHDFile();
					em->ShowActiveHDFile_Info();
					*/
				}
			}
			break;

		case EDITORTYPE_ARRANGE:
			{
				Edit_Arrange *ar=(Edit_Arrange *)w;

				/*
				if(deletecounter>0)
				{
				ar->ShowHoriz(false,false,true);
				ar->ShowPatternList();
				}
				else
				*/

				// ar->RefreshAudioRegion(deadregion); // Remove From Samples
			}
			break;

		case EDITORTYPE_EVENT:
			{
				// Edit_Event *e=(Edit_Event *)w;

				// e->ShowAllEvents();
			}
			break;
		}

		w=n;
	}
}

void GUI::RemoveAudioRegionFromGUI(AudioHDFile *hd,AudioRegion *deadregion)
{
	// Warning: region=dead pointer
	mainbuffer->RemoveRegion(hd,deadregion);

	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		switch(w->GetEditorID())
		{
		case EDITORTYPE_SAMPLEREGIONLIST:
			{
				Edit_RegionList *er=(Edit_RegionList *)w;

				if(hd==er->editor->audiohdfile)
				{
					if(er->selectedregion==deadregion)
						er->selectedregion=0;

					er->ShowRegions();
					er->ShowRegionName();
				}
			}
			break;

		case EDITORTYPE_AUDIOMANAGER:
			{
				Edit_Manager *em=(Edit_Manager *)w;

				if(em->activeregion==deadregion)
					em->activeregion=0;

				if(em->activefile && em->activefile->hdfile==hd)
					em->ShowActiveHDFile_Regions();

				em->ShowAudioFiles();
			}
			break;

		case EDITORTYPE_ARRANGE:
			{
				Edit_Arrange *ar=(Edit_Arrange *)w;

				/*
				if(deletecounter>0)
				{
				ar->ShowHoriz(false,false,true);
				ar->ShowPatternList();
				}
				else
				*/

				ar->RefreshAudioRegion(deadregion); // Remove From Samples
			}
			break;

		case EDITORTYPE_EVENT:
			{
				// Edit_Event *e=(Edit_Event *)w;

				// e->ShowAllEvents();
			}
			break;
		}

		w=w->NextWindow();
	}

	RefreshAllUndos();
}

void GUI::RemoveEventFromGUI(Seq_Song *song,Seq_Event *e)
{
	if(!e)return;

	if(e==song->GetFocusEvent())
	{
		song->SetFocusEvent(0);
	}

	guiWindow *w=FirstWindow();

	while(w)
	{	
		if(w->closeit==false)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_PIANO:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					EventEditor *editor=(EventEditor *)w;
					editor->RemoveEvent(e);
				}
				break;
			}
		}

		w=w->NextWindow();
	}
}

void GUI::RemovePatternFromGUI(Seq_Song *song,Seq_Pattern *pattern,bool draw)
{
	if(!pattern)return;

	if(song && pattern==song->GetFocusPattern())
	{
		song->SetFocusPattern(0);
	}

	guiWindow *w=FirstWindow();

	while(w)
	{	
		if(w->closeit==false)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_QUANTIZEEDITOR:
				{
					if(((Edit_QuantizeEditor *)w)->effect==&pattern->quantizeeffect)
						w->closeit=true;
				}
				break;

			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					EventEditor *editor=(EventEditor *)w;
					editor->RemovePattern(pattern,draw);
				}
				break;

			case EDITORTYPE_SAMPLE:
				{
					Edit_Sample *es=(Edit_Sample *)w;

					if(es->pattern==pattern)
						w->closeit=true;
				}
				break;

			case EDITORTYPE_CROSSFADE:
				{
					Edit_CrossFade *edc=(Edit_CrossFade *)w;

					if(edc->outpattern==pattern || edc->inpattern==pattern)
						w->closeit=true;
				}
				break;
			}
		}

		if(w->closeit==true)
		{
			CloseWindow(w);
			w=FirstWindow();
		}
		else
			w=w->NextWindow();
	}
}

void GUI::RefreshAudioChannelName(AudioChannel *c,guiWindow *not)
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->WindowSong()==c->audiosystem->song)
		{
			switch(w->GetEditorID())
			{
				/*
				case EDITORTYPE_ARRANGE:
				{
				Edit_Arrange *ed=(Edit_Arrange *)w;

				if(ed->WindowSong()->GetFocusTrack() && ed->WindowSong()->GetFocusTrack()->GetAudioOut()->FindChannel(c)==true)
				{
				ed->trackfx.ShowAudioInput();
				ed->trackfx.ShowAudioOutput();
				}
				}
				break;
				*/

			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *em=(Edit_AudioMix *)w;

					//if(c==c->audiosystem->activechannel)
					//	em->ShowActiveChannel();

					if(w!=not)
					{
						/*
						Edit_AudioMixEffects *fc=em->FirstAudioChannel();

						while(fc)
						{
						if(fc->channel==c)
						{
						//	fc->ShowChannelName();
						break;
						}

						fc=fc->NextChannel();
						}
						*/
					}
				}
				break;
			}
		}

		w=w->NextWindow();
	}
}

void GUI::RefreshAudioPattern(AudioPattern *p)
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->WindowSong()==p->track->song)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ed=(Edit_Arrange *)w;
					ed->ShowHoriz(false,false,true);	
				}
				break;

			case EDITORTYPE_EVENT:
				{
					Edit_Event *ed=(Edit_Event *)w;

					if(ed->patternselection.FindPattern(p))
						ed->ShowEventsHoriz(SHOWEVENTS_EVENTS);
				}
				break;

			case EDITORTYPE_SAMPLE:
				{
					Edit_Sample *es=(Edit_Sample *)w;

					if(es->pattern && es->pattern==p)
						es->RefreshObjects(0,false);
				}
				break;

			}
		}

		w=w->NextWindow();
	}
}

void GUI::RefreshMIDICheck(guiWindow *w,Seq_Track *track)
{
	switch(w->GetEditorID())
	{
	case EDITORTYPE_AUDIOMIXER:
		{
			Edit_AudioMix *et=(Edit_AudioMix *)w;
			et->RefreshTrack(track);
		}
		break;

	case EDITORTYPE_ARRANGEFX:
		{
			Edit_ArrangeFX *fx=(Edit_ArrangeFX *)w;
			fx->RefreshMIDI(track);
		}
		break;

	case EDITORTYPE_ARRANGE:
		{
			Edit_Arrange *ar=(Edit_Arrange*)w;
			ar->RefreshMIDI(track);
		};
	}
}

void GUI::RefreshMIDI(Seq_Track *track)
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->WindowSong()==track->song)
		{
			RefreshMIDICheck(w,track);
		}

		w=w->NextWindow();
	}
}

void GUI::RefreshAudioCheck(guiWindow *w,Seq_Track *track)
{
	switch(w->GetEditorID())
	{
	case EDITORTYPE_ARRANGEFX:
		{
			Edit_ArrangeFX *efx=(Edit_ArrangeFX *)w;
			Seq_Track *t=efx->WindowSong()->GetFocusTrack();

			if(t==track)
				efx->trackaudio.RefreshAll();
		}
		break;

	case EDITORTYPE_ARRANGE:
		{
			Edit_Arrange *ar=(Edit_Arrange *)w;

			ar->RefreshTrack(track);
		}
		break;

	case EDITORTYPE_AUDIOMIXER:
		{
			Edit_AudioMix *et=(Edit_AudioMix *)w;
			et->RefreshTrack(track);

			/*
			et->F

			Edit_AudioMixEffects *chl=et->FirstAudioChannel();

			while(chl)
			{
			if(chl->track==track)
			{
			chl->ShowRecordChannel();
			chl->ShowPlaybackChannel(false);
			}

			chl=chl->NextChannel();
			}
			*/
		}
		break;
	}
}

void GUI::RefreshAudio(Seq_Track *track)
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->WindowSong()==track->song)
		{
			RefreshAudioCheck(w,track);
		}

		w=w->NextWindow();
	}
}

void GUI::RefreshAutoTracks(guiWindow *win,Seq_Song *song,AutomationTrack *track)
{
	guiWindow *w;

	if(!(w=win))
		w=FirstWindow();

	while(w)
	{
		if(w->WindowSong()==song)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ed=(Edit_Arrange *)w;


				}
				break;
			}
		}

		if(win) // only 1 win
			break;

		w=w->NextWindow();
	}
}

void GUI::RefreshProcessorName(Seq_Track *t)
{
	guiWindow *w=FirstWindow();

	while(w)
	{					
		if(w->WindowSong() && w->WindowSong()->GetFocusTrack()==t)
			switch(w->GetEditorID())
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ear=(Edit_Arrange *)w;
					//	ear->trackfx.ShowProcessor();
				}
				break;
		}

		w=w->NextWindow();
	}
}

void RefreshAMixer(guiWindow *w,Seq_Song *song,AudioEffects *effects)
{
	if(w->WindowSong()==song)
		switch(w->GetEditorID())
	{
		case EDITORTYPE_AUDIOMIXER:
			{
				Edit_AudioMix *etam=(Edit_AudioMix *)w;
				etam->ShowAll();
			}
			break;
	}
}

void GUI::RefreshAudioMixer(Seq_Song *song,AudioEffects *effects)
{
	guiWindow *w=FirstWindow();

	while(w)
	{	
		guiWindow *nw=w->NextWindow();

		if(w->closeit==false)
			RefreshAMixer(w,song,effects);

		w=nw;
	}
}

void GUI::RemoveAudioChannelFromGUI(Seq_Song *song,AudioChannel *c)
{
	if((!c) || (!song))
		return;

	guiWindow *w=FirstWindow();

	while(w)
	{	
		bool closewindow=false;

		if(w->closeit==false)
		{					
			switch(w->GetEditorID())
			{
			default:
				if(w->CheckIfObjectInside(c)==true)
					closewindow=true;
				//else
				//	closewindow=w->RemoveTrackFromWindow(track);
				break;
			}
		}

		if(w->closeit==true || closewindow==true)
		{
			CloseWindow(w);
			w=FirstWindow();
		}
		else
			w=w->NextWindow();
	}
}

void GUI::RemoveAudioEffectFromGUI(InsertAudioEffect *iae)
{
	if(!iae)
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"0 RemoveAudioEffectFromGUI");
#endif

		return;
	}

	guiWindow *w=FirstWindow();

	while(w)
	{	
		guiWindow *nw=w->NextWindow();

		if(w->closeit==false)
		{	
			w->RemoveAudioEffect(iae);

			if(w->CheckIfObjectInside(iae))
			{
				w->closeit=true;
				CloseWindow(w);
				nw=FirstWindow();
			}
		}

		w=nw;
	}
}

void GUI::RemoveProjectFromGUI(Seq_Project *pro)
{
	if(!pro)return;

	Seq_Song *s=pro->FirstSong();

	while(s)
	{
		RemoveSongFromGUI(0,s);
		s=s->NextSong();
	}

	guiWindow *win=FirstWindow();
	while(win)
	{
		switch(win->GetEditorID())
		{
		case EDITORTYPE_PLAYER:
			{
				Edit_Player *edp=(Edit_Player *)win;

				if(edp->activeproject==pro)
					edp->activeproject=(Seq_Project *)pro->NextOrPrev();

				edp->ShowProjects();
				edp->ShowSongs();
			}
			break;
		}

		if(win->GetEditorID()==EDITORTYPE_SETTINGS)
		{
			Edit_Settings *es=(Edit_Settings *)win;

			if(es->editproject==pro)
			{
				win=CloseWindow(win);
				win=FirstWindow();
			}
			else
				win=win->NextWindow();
		}
		else
			win=win->NextWindow();
	}

	guiScreen *screen=FirstScreen();
	while(screen)
	{
		if(screen->project==pro)
		{
			if(screen==FirstScreen() && screen==LastScreen())
			{
				screen->project=0;
				screen->song=0;
				screen->RefreshMenu();

				return;
			}

			DeleteScreen(screen);
			screen=FirstScreen();
		}
		else
			screen=screen->NextScreen();
	}
}

void GUI::RemoveProcessorFromGUI(Processor *proc,MIDIPlugin *mod)
{
	if(!proc)
		return;

	guiWindow *w=FirstWindow();

	while(w)
	{	
		guiWindow *nw=w->NextWindow();

		if(w->closeit==false)
		{					
			switch(w->GetEditorID())
			{
			case EDITORTYPE_PROCESSORMODULE:
				{
					Edit_ProcMod *pm=(Edit_ProcMod *)w;
					bool closeit=false;

					if(pm->module && pm->module->processor==proc)
						closeit=true;

					if(mod && pm->module==mod)
						closeit=true;

					/*
					if(closeit==true)
					w->CloseWindow(false);
					*/
				}
				break;
			}
		}

		w=nw;
	}
}

guiWindow *GUI::FindWindowInGUI (guiWindow *c)
{
	guiWindow *w=FirstWindow();
	while(w)
	{
		if(w->closeit==false)
		{
			if(w==c)
				return c;
		}

		w=w->NextWindow();
	}
	return 0;
}

void GUI::RemoveTrackFromGUI(Seq_Track *track,int flag)
{
	if(!track)return;

	//RemoveTrackFromGUI(track,flag);

	Seq_Track *child=track->FirstChildTrack();
	while(child)
	{
		RemoveTrackFromGUI(child,flag);
		child=child->NextChildTrack();
	}

	if(track->t_processor)
		RemoveProcessorFromGUI(track->t_processor,0);

	guiWindow *w=FirstWindow();

	while(w)
	{	
		bool closewindow=false;

		if(w->closeit==false)
		{					
			switch(w->GetEditorID())
			{
			case EDITORTYPE_MIDIFILTER:
				{
					Edit_MIDIFilter *mf=(Edit_MIDIFilter *)w;

					if(mf->filter==&track->GetFX()->filter || mf->filter==&track->GetFX()->inputfilter)
						closewindow=true;
				}
				break;

			case EDITORTYPE_QUANTIZEEDITOR:
				{
					Edit_QuantizeEditor *eq=(Edit_QuantizeEditor *)w;

					if(eq->effect==&track->GetFX()->quantizeeffect)
						closewindow=true;
				}
				break;

			case EDITORTYPE_SETTINGS:
				{
					Edit_Settings *set=(Edit_Settings *)w;

					if(set->song)
					{
						switch(set->settingsselection)
						{
						case Edit_Settings::SONG_ROUTING:
							set->ShowTargetTracks();
							break;
						}
					}
				}
				break;

			default:
				if(w->CheckIfObjectInside(track)==true)
					closewindow=true;
				else
					closewindow=w->RemoveTrackFromWindow(track);
				break;
			}
		}

		if(w->closeit==true || closewindow==true)
		{
			CloseWindow(w);
			w=FirstWindow();
		}
		else
			w=w->NextWindow();
	}

	// Remove Tracks Pattern from GUI
	{
		Seq_Pattern *pattern=track->FirstPattern(MEDIATYPE_ALL);

		while(pattern)
		{
			RemovePatternFromGUI(track->song,pattern);
			pattern=pattern->NextPattern(MEDIATYPE_ALL);
		}
	}
}

void GUI::RemoveFreezeTrackFromGUI(Seq_Track *track)
{
	if(!track)return;

	//RemoveTrackFromGUI(track,flag);

	guiWindow *w=FirstWindow();

	while(w)
	{	
		bool closewindow=false;

		if(w->closeit==false)
		{					
			switch(w->GetEditorID())
			{
			case EDITORTYPE_MIDIFILTER:
				{
					Edit_MIDIFilter *mf=(Edit_MIDIFilter *)w;

					if(mf->filter==&track->GetFX()->filter || mf->filter==&track->GetFX()->inputfilter)
						closewindow=true;
				}
				break;

			default:
				if(w->CheckIfObjectInside(track)==true)
					closewindow=true;
				else
					closewindow=w->RemoveTrackFromWindow(track);
				break;
			}
		}

		if(w->closeit==true || closewindow==true)
		{
			CloseWindow(w);
			w=FirstWindow();
		}
		else
			w=w->NextWindow();
	}

	// Remove Tracks Pattern from GUI
	{
		Seq_Pattern *pattern=track->FirstPattern(MEDIATYPE_ALL);

		while(pattern)
		{
			RemovePatternFromGUI(track->song,pattern);
			pattern=pattern->NextPattern(MEDIATYPE_ALL);
		}
	}
}

void GUI::ResetPRepairSelection(Seq_SelectionList *list)
{
	bool refresh=false;

	Seq_SelectionEvent *e=list->FirstMixEvent();

	while(e)
	{
		if(e->seqevent->flag&OFLAG_UNDERSELECTION)
		{
			e->seqevent->flag CLEARBIT OFLAG_UNDERSELECTION;
			refresh=true;
		}

		e=e->NextEvent();
	}

	if(refresh==true)
		maingui->RefreshAllEditorsWithEvent(list->song,0);
}

void GUI::OpenPRepairSelection_Pattern(Seq_Song *song)
{
	Seq_Track *t=song->FirstTrack();
	while(t)
	{
		Seq_Pattern *p=t->FirstPattern();
		while(p)
		{
			if(p->flag&OFLAG_UNDERSELECTION)
			{
				p->flag|=OFLAG_OLDUNDERSELECTION;
				p->flag CLEARBIT OFLAG_UNDERSELECTION;
			}
			else
				p->flag CLEARBIT OFLAG_OLDUNDERSELECTION;

			p=p->NextRealPattern();
		}

		t=t->NextTrack();
	}
}

void GUI::ClosePRepairSelection_Pattern(Seq_Song *song)
{
	Seq_Track *t=song->FirstTrack();
	while(t)
	{
		Seq_Pattern *p=t->FirstPattern();
		while(p)
		{

			if(p->flag&OFLAG_UNDERSELECTION)
			{
				if(!(p->flag&OFLAG_OLDUNDERSELECTION))
					goto refresh;
			}
			else
			{
				if(p->flag&OFLAG_OLDUNDERSELECTION)
					goto refresh;
			}

			p=p->NextRealPattern();
		}

		t=t->NextTrack();
	}

	return;

refresh:
	maingui->RefreshAllEditorsWithPattern(song,0);
}

void GUI::OpenPRepairSelection(Seq_SelectionList *list)
{
	Seq_SelectionEvent *e=list->FirstMixEvent();

	while(e)
	{
		if(e->seqevent->flag&OFLAG_UNDERSELECTION)
		{
			e->seqevent->flag|=OFLAG_OLDUNDERSELECTION;
			e->seqevent->flag CLEARBIT OFLAG_UNDERSELECTION;
		}
		else
			e->seqevent->flag CLEARBIT OFLAG_OLDUNDERSELECTION;

		e=e->NextEvent();
	}
}

void GUI::ClosePRepairSelection(Seq_SelectionList *list)
{
	Seq_SelectionEvent *e=list->FirstMixEvent();

	while(e)
	{
		if(e->seqevent->flag&OFLAG_UNDERSELECTION)
		{
			if(!(e->seqevent->flag&OFLAG_OLDUNDERSELECTION))
				goto refresh;
		}
		else
		{
			if((e->seqevent->flag&OFLAG_OLDUNDERSELECTION))
				goto refresh;
		}

		e=e->NextEvent();
	}

	return;

refresh:

	maingui->RefreshAllEditorsWithEvent(list->song,0);
}


void GUI::ShowMouseSelection_Event(Seq_Event *e)
{
	/*
	guiWindow *g=FirstWindow();

	while(g)
	{
	if(g->WindowSong()==e->GetTrack()->song)
	switch(g->GetEditorID())
	{
	case EDITORTYPE_EVENT:
	case EDITORTYPE_DRUM:
	case EDITORTYPE_WAVE:
	case EDITORTYPE_PIANO:
	case EDITORTYPE_SCORE:
	{
	((EventEditor_Selection *)g)->RefreshEvent(e);
	}
	break;		
	}

	g=g->NextWindow();
	}
	*/
}

void GUI::MessageBoxError(guiWindow *win,char *title)
{
#ifdef WIN32

	/*
	if(!win)
	{
	if(GetActiveScreen() && GetActiveScreen()->hWndClient)
	MessageBox(
	GetActiveScreen()->hWndClient,          // handle of owner window
	title,     // address of text in message box
	Cxs[CXS_QUESTION],  // address of title of message box
	MB_OK | MB_TASKMODAL| MB_SETFOREGROUND // style of message box
	);
	else
	{
	MessageBox(
	0,          // handle of owner window
	title,     // address of text in message box
	Cxs[CXS_INFO],  // address of title of message box
	MB_OK| MB_TASKMODAL| MB_SETFOREGROUND  // style of message box
	);
	}
	}
	else
	{

	}
	*/

	MessageBox(
		0,          // handle of owner window
		title,     // address of text in message box
		Cxs[CXS_ERROR],  // address of title of message box
		MB_ERROR_STYLE // style of message box
		);
#endif
}

#ifdef WIN32

void GUI::MessageMMError(char *device,char *from,MMRESULT res)
{
	if(res==MMSYSERR_NOERROR)return;
	if(res==MMSYSERR_NODRIVER)return;

	char *h=0;
	switch(res)
	{
	case MMSYSERR_ERROR:        h="unspecified error";break;
	case MMSYSERR_BADDEVICEID:  h="device ID out of range";break;
	case MMSYSERR_NOTENABLED:   h="driver failed enable";break;
	case MMSYSERR_ALLOCATED:    h="device already allocated";break;
	case MMSYSERR_INVALHANDLE : h="device handle is invalid";break;
		//case MMSYSERR_NODRIVER:     h="no device driver present\ndevice removed ?";break;
	case MMSYSERR_NOMEM  :      h="memory allocation error";break;
	case MMSYSERR_NOTSUPPORTED: h="function isn't supported";break;
	case MMSYSERR_BADERRNUM:    h="error value out of range";break;
	case MMSYSERR_INVALFLAG :   h="invalid flag passed";break;
	case MMSYSERR_INVALPARAM:   h="invalid parameter passed";break;
	case MMSYSERR_HANDLEBUSY :  h="handle being used";break;
		/* simultaneously on another */
		/* thread (eg callback) */
	case MMSYSERR_INVALIDALIAS: h="specified alias not found";break;
	case MMSYSERR_BADDB:        h="bad registry database";break;
	case MMSYSERR_KEYNOTFOUND:  h="registry key not found";break;
	case MMSYSERR_READERROR :   h="registry read error";break;
	case MMSYSERR_WRITEERROR:   h="registry write error";break;
	case MMSYSERR_DELETEERROR : h="registry delete error";break;
	case MMSYSERR_VALNOTFOUND : h="registry value not found";break;
	case MMSYSERR_NODRIVERCB :  h="driver does not call DriverCallback";break;
	case MMSYSERR_MOREDATA :   h="more data to be returned";break;

	case MIDIERR_STILLPLAYING: h="still something playing *";break;
	case MIDIERR_NOMAP:h=" no configured instruments";break;
	case MIDIERR_NOTREADY :h=" hardware is still busy ";break;
	case MIDIERR_NODEVICE :h="port no longer connected ";break;
	case MIDIERR_INVALIDSETUP :h=" invalid MIF ";break;
	case MIDIERR_BADOPENMODE  :h="operation unsupported w/ open mode ";break;
	case MIDIERR_DONT_CONTINUE :h=" thru device 'eating' a message ";break;

	default:h="Error unspecified...";break;
	}

	char *h2=mainvar->GenerateString(device,"\n",from,"\n",h);

	if(h2)
	{
		MessageBoxError(0,h2);
		delete h2;
	}
}
#endif

bool GUI::MessageBoxOk(guiWindow *win,char *title)
{
#ifdef WIN32

	/*
	if(!win)
	{
	if(GetActiveScreen() && GetActiveScreen()->hWndClient)
	MessageBox(
	GetActiveScreen()->hWndClient,          // handle of owner window
	title,     // address of text in message box
	Cxs[CXS_QUESTION],  // address of title of message box
	MB_OK | MB_TASKMODAL| MB_SETFOREGROUND // style of message box
	);
	else
	{
	MessageBox(
	0,          // handle of owner window
	title,     // address of text in message box
	Cxs[CXS_INFO],  // address of title of message box
	MB_OK| MB_TASKMODAL| MB_SETFOREGROUND  // style of message box
	);
	}
	}
	else
	{

	}
	*/

	MessageBox(
		0,          // handle of owner window
		title,     // address of text in message box
		Cxs[CXS_INFO],  // address of title of message box
		MB_OK_STYLE // style of message box
		);
#endif

	return true;
}

bool GUI::MessageBoxYesNo(guiWindow *win,char *title)
{
	bool yes=true;

#ifdef WIN32

	int b=IDNO;

	/*
	if(!win)
	{
	if(maingui->GetActiveScreen())
	{
	b=MessageBox(
	maingui->GetActiveScreen()->hWnd,          // handle of owner window
	title,     // address of text in message box
	Cxs[CXS_QUESTION],  // address of title of message box
	MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND  // style of message box
	);
	}
	}
	else
	{

	}
	*/
	b=MessageBox(
		0,          // handle of owner window
		title,     // address of text in message box
		Cxs[CXS_QUESTION],  // address of title of message box
		MB_YESNO_STYLE// style of message box
		);

	switch(b)
	{ 
	case IDNO:
		yes=false;
		break;
	}

#endif

	return yes;
}

void GUI::SetInfoWindowText(char *text)
{
	if(maingui->FirstScreen())
	{
		maingui->FirstScreen()->SetTitle(text);
	}

	/*
	LockInfoWindow();

	if(infoWindow && text)
	{
	infoWindow->guiSetWindowText(text);
	}

	UnlockInfoWindow();
	*/

}

Edit_CamXInfo::Edit_CamXInfo()
{
	editorid=EDITORTYPE_CAMXINFO;
	InitForms(FORM_PLAIN1x1);

	ondesktop=true;
	dontchangesettings=true;
}

guiMenu *Edit_CamXInfo::CreateMenu()
{
	if(menu=new guiMenu)
	{
		guiMenu *sub=menu->AddMenu("CamX Info",0);

		if(sub)
		{
			sub->AddMenu("CamX Audio MIDI Sequencer",0);
#ifdef WIN32
			sub->AddMenu("Windows XP/NT/VISTA/W7",0);
#endif
			sub->AddMenu("Copyright 2010-12 ism/Martin Endres",0);
			sub->AddMenu("Homepage: www.camx.de",0);

			sub->AddLine();
			sub->AddMenu("ASIO+VST/i is a Trademark of Steinberg",0);
		}
	}

	return menu;
}

void Edit_CamXInfo::DeInitWindow()
{	
	maingui->infoWindow=0;
}

void Edit_CamXInfo::ShowInfo()
{
	if(!text)
		return;

	guiBitmap *bm=&text->gbitmap;

	bm->guiFillRect(COLOUR_BACKGROUNDFORMTEXT);

	// Version
	char versionstring[128];

	strcpy(versionstring,"CamX Audio/MIDI ");
	char h2[NUMBERSTRINGLEN];

	int f=maingui->GetVersion()/1000;
	int p=maingui->GetVersion()-(f*1000);

	mainvar->AddString(versionstring,mainvar->ConvertIntToChar(f,h2));

	if(p<10)
		mainvar->AddString(versionstring,".00");
	else
		if(p<100)
			mainvar->AddString(versionstring,".0");
		else
			mainvar->AddString(versionstring,".");

	mainvar->AddString(versionstring,mainvar->ConvertIntToChar(p,h2));

	bm->SetTextColour(COLOUR_BLACK);
	SetBkMode(bm->hDC, TRANSPARENT); // Transparent Text etc...

	int y=DEFAULTLISTY;

	bm->guiDrawText(2,y,bm->GetX2(),versionstring);
	y+=DEFAULTLISTY;

	bm->guiDrawText(2,y,bm->GetX2(),"Release :01.10.2015");
	y+=DEFAULTLISTY;

	bm->guiDrawText(2,y,bm->GetX2(),Cxs[CXS_LANGUAGE]);
	y+=DEFAULTLISTY;

#ifdef WIN32

#ifdef WIN64
	bm->guiDrawText(2,y,bm->GetX2(),"Windows X64 Test Version(Desktop)");
#else

#ifdef NOSSE2
	bm->guiDrawText(2,y,bm->GetX2(),"Windows x86/SSE1 Test Version(Desktop)");
#else
	bm->guiDrawText(2,y,bm->GetX2(),"Windows x86/SSE2 Test Version(Desktop)");
#endif

#endif

#endif

	y+=DEFAULTLISTY;

#ifdef ARES64
#ifdef WIN64
	bm->guiDrawText(2,y,bm->GetX2(),"64 Bit float Audio");
#endif

#ifdef WIN32
	bm->guiDrawText(2,y,bm->GetX2(),"64 Bit float Audio+32/64Bit VST");
#endif

#else
	bm->guiDrawText(2,y,bm->GetX2(),"32 Bit float Audio");
#endif

	y+=DEFAULTLISTY;

	// VST Version
	if(sizeof(VstIntPtr)==8)
		bm->guiDrawText(2,y,bm->GetX2(),"[64 Bit VST 2.4/3.5]");
	else
		bm->guiDrawText(2,y,bm->GetX2(),"[32 Bit VST 2.4/3.5]");

	y+=DEFAULTLISTY;

	bm->guiDrawText(2,y,bm->GetX2(),"ASIO 2.3");

	y+=DEFAULTLISTY;

	bm->guiDrawText(2,y,bm->GetX2(),"(C)2010-14 ism/Martin Endres");
	y+=DEFAULTLISTY;

	bm->guiDrawText(2,y,bm->GetX2(),"Homepage  :  www.camx.de");
}

void Info_Callback(guiGadget_CW *g,int status)
{
	Edit_CamXInfo *p=(Edit_CamXInfo *)g->from;

	switch(status)
	{
	case DB_CREATE:
		p->text=g;
		break;

	case DB_PAINT:
		{
			p->ShowInfo();
			//	p->ShowOverview();
			//	p->ShowOverviewCycleAndPositions();
		}
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		//p->MouseClickInOverview(true);	
		break;

	case DB_MOUSEMOVE|DB_RIGHTMOUSEDOWN:
	case DB_RIGHTMOUSEDOWN:
		//p->MouseClickInOverview(false);	
		break;
	}
}

void Edit_CamXInfo::Init()
{
	glist.SelectForm(0,0);
	glist.AddChildWindow(-1,-1,-1,-1,MODE_BOTTOM|MODE_RIGHT,0,&Info_Callback,this);

	return;
}

void GUI::ToggleChildDesktop(guiWindow *win)
{
	if(guiForm_Child *child=win->parentformchild)
	{
		// Child -> Desktop

		win->exparentformx=child->fx;
		win->exparentformy=child->fy;
		win->parentformchild=0;

		ShowWindow(win->hWnd,SW_HIDE);
		SetParent(win->hWnd,NULL); // -> Screen

		SetWindowLongPtr(win->hWnd,GWL_STYLE,WS_OVERLAPPEDWINDOW);

		if(win->menu)
			SetMenu(win->hWnd,win->menu->OSMenuHandle);

		//SetWindowPos(win->hWnd, HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		if(win->minheight>0 && win->minwidth>0 && (win->height<win->minheight || win->width<win->minwidth))
		{
			RECT wsize;

			wsize.left=GetClientRect(win->hWnd,&wsize);

			if(win->height<win->minheight)
			{
				wsize.bottom=wsize.top+(win->minheight-1);
			}

			if(win->width<win->minwidth)
			{
				wsize.right=wsize.left+(win->minwidth-1);
			}

			AdjustWindowRectEx(&wsize,WS_OVERLAPPEDWINDOW,FALSE /*win->mdimode==true?false:true*/ ,win->flagex);

			win->width=(wsize.right-wsize.left)+1;
			win->height=(wsize.bottom-wsize.top)+1;

			SetWindowPos(win->hWnd, HWND_TOPMOST,0,0,win->width,win->height,SWP_FRAMECHANGED | SWP_NOMOVE);
		}
		else
			SetWindowPos(win->hWnd, HWND_TOPMOST,0,0,0,0,SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);

		UpdateWindow(win->hWnd);
		ShowWindow(win->hWnd,SW_SHOWNA);

		guiWindow *w=LastWindow();

		while(w)
		{
			if(w->parentformchild==child && win!=w)
			{
				w->parentformchild->fhWnd=w->hWnd;

				MoveWindow(w->hWnd,child->x,child->y,child->GetWidth(),child->GetHeight(),TRUE);
				UpdateWindow(w->hWnd);

				ShowWindow(w->hWnd,SW_SHOWNA);

				w->hide=false;
				break;
			}

			w=w->PrevWindow();
		}

		if(!w)
		{
			child->fhWnd=0;
			child->Enable(false); // Disable Child
		}	
	}
	else
	{
		// Desktop -> Child
		if(guiScreen *ascreen=win->openscreen)
		{
			guiForm_Child *toform=&ascreen->forms[win->exparentformx][win->exparentformy];

			if(win->exparentformx!=1)
			{
				guiWindow *w=FirstWindow();
				while(w)
				{
					if(w!=win && w->hide==false && w->hWnd==toform->fhWnd)
					{
						w->hide=true; // Stop Realtime Refresh etc..
						ShowWindow(w->hWnd,SW_HIDE);
						break;
					}

					w=w->NextWindow();
				}
			}

			win->parentformchild=toform;
			toform->fhWnd=win->hWnd;
			//	

			//	SetWindowPos(win->hWnd, HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOREDRAW);

			ShowWindow(win->hWnd,SW_HIDE); // Hide window

			SetWindowLongPtr(win->hWnd,GWL_STYLE,WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS); // Hide Border
			SetWindowPos(win->hWnd, HWND_NOTOPMOST,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

			SetParent(win->hWnd,ascreen->hWnd); // Desktop - > Child

			if(toform->InUse()==false)
			{
				toform->fromdesktoptoscreen=true;
				toform->Enable(true); // Enable
				toform->fromdesktoptoscreen=false;
			}
			else
			{
				MoveWindow(win->hWnd,toform->x,toform->y,toform->GetWidth(),toform->GetHeight(),TRUE);				
				UpdateWindow(win->hWnd);
				ShowWindow(win->hWnd,SW_SHOWNA);
			}
		}
	}
}

void GUI::RefreshTimeSlider(Seq_Song *song)
{
	// Refresh GUI
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(w->WindowSong()==song)
			switch(w->GetEditorID())
		{
			case EDITORTYPE_SAMPLE:
				{
					Edit_Sample *es=(Edit_Sample *)w;

					if(es->songmode==true)
					{
						if(es->overview)
							es->overview->DrawGadgetBlt();

						es->RefreshTimeSlider();
					}
				}
				break;

			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_TEMPO:
				{
					EventEditor *e=(EventEditor *)w;

					if(e->overview)
						e->overview->DrawGadgetBlt();

					e->RefreshTimeSlider();
				}
				break;

			case EDITORTYPE_EVENT:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_SCORE:
				{
					EventEditor *e=(EventEditor *)w;
					e->RefreshTimeSlider();
				}
				break;
		}

		w=w->NextWindow();
	}
}

void GUI::CloseAllAutoLoadSongs()
{
	for(int i=0;i<2;i++)
	{
		if(autoloadsongs[i])
		{
			autoloadsongs[i]->FreeMemory(Seq_Project::DELETESONG_FULL);
			autoloadsongs[i]=0;
		}
	}
}

bool GUI::AskForDefaultSong(int type,bool requester)
{
	bool ok=false;
	camxFile file;

	if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),type==ID_AUTOLOAD_CAMX?CAMX_DEFAULTAUTOLOADNAME:CAMX_DEFAULTAUTOLOADSMFNAME) )
	{
		if(file.OpenRead(h)==true)
		{
			if(requester==true)
				ok=MessageBoxYesNo(0,Cxs[CXS_LOADAUTOQ]);
			else
				ok=true;
		}

		delete h;
	}

	file.Close(true);

	return ok;
}

Seq_Song *GUI::LoadDefaultSong(guiScreen *screen,Seq_Song *song,bool MIDI)
{
	bool ok=false;
	char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),MIDI==true?CAMX_DEFAULTAUTOLOADSMFNAME:CAMX_DEFAULTAUTOLOADNAME);

	if(!h)return 0;

	bool createnew=false;

	if(!song) // Create Song ?
	{
		camxFile file;

		if(file.OpenRead(h)==false)
			ok=maingui->MessageBoxOk(0,Cxs[CXS_NOAUTOLOAD]);
		else
		{
			if(mainvar->GetActiveProject())
			{
				if(song = new Seq_Song(mainvar->GetActiveProject()))
					createnew=true;
			}
		}

		file.Close(true);
	}

	if(song){
		camxFile file;

		if(file.OpenRead(h)==true){

			if(createnew==true)
			{			
				song->autoloadsong=true;
				song->autoloadMIDI=MIDI;
			}

			song->Load(&file);
			song->SetSongPosition(0,false);
			ok=true;
		}

		file.Close(true);
	}

	if(createnew==true && ok==false && song)
	{
		// No default song found
		song->FreeMemory(Seq_Project::DELETESONG_FULL);
		song=0;
	}

	delete h;

	if(ok==true && song)
	{
		autoloadsongs[MIDI==true?ID_AUTOLOAD_MIDI:ID_AUTOLOAD_CAMX]=song;

		if(createnew==true)
		{
			if(screen)
				screen->InitNewSong(song);
		}

		return song;
	}

	return 0;
}

bool GUI::SaveDefaultSong(Seq_Song *song,bool MIDI)
{
	bool ok=false;

	if(song){

		//if(char *snbuffer=mainvar->GenerateString(song->songname))
		{

			// Create Dir
			if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),

#ifdef WIN32

#ifdef WIN64
				"\\AutoloadX64\\",
#else

				"\\AutoloadX32\\",
#endif

#endif

				MIDI==true?"ALSMF":"ALCAMX"))
			{
				if(mainvar->CreateNewDirectory(h)==false)
				{
					delete h;
					//delete snbuffer;
					return false;
				}

				delete h;
			}
			else
			{
				//	delete snbuffer;
				return false;
			}

			camxFile save;

			if(char *h=mainvar->GenerateString(mainvar->GetUserDirectory(),MIDI==true?CAMX_DEFAULTAUTOLOADSMFNAME:CAMX_DEFAULTAUTOLOADNAME))
			{
				bool ok=true;

				if(mainvar->exitthreads==false && (song->autoloadsong==false || song->autoloadMIDI!=MIDI))
				{
					camxFile check;

					if(check.OpenRead(h)==true) //Exists
					{
						char *qh=mainvar->GenerateString(Cxs[CXS_OVERWRITEAUTOLOADQ],"\n",Cxs[MIDI==true?CXS_AUTOLOADFORSMFSONGS:CXS_AUTOLOADFORSONGS]);
						char *qh2=mainvar->GenerateString(qh,"\n",Cxs[CXS_REPLACEWITH],"Song:",song->GetName());

						ok=qh2?maingui->MessageBoxYesNo(0,qh2):false;

						if(qh)
							delete qh;

						if(qh2)
							delete qh2;

					}

					check.Close(true);
				}

				if(ok==true && save.OpenSave_CheckVersion(h)==true){
					//song->SetSongName("Autoload",false);
					song->Save(&save);
					//song->SetSongName(snbuffer,false);
					ok=true;
				}
				else
					TRACE ("Skip Save Autoload\n");

				delete h;
			}

			save.Close(true);
			//delete snbuffer;
		}
	}

	return ok;
}

guiWindow *GUI::ConvertSystemWindowToGUI(HWND hWnd)
{
	guiWindow *g=FirstWindow();

	while(g)
	{
		if(g->hWnd==hWnd)
			return g;

		g=g->NextWindow();
	}

	return 0;
}


guiGadget *GUI::CheckMouseUp(guiWindow *win,int flag)
{
	guiGadget *g=0;

	win->autoscroll=false;

	{

		/*
		g=win->gadgetlists.FindGadget(win->GetMouseX(),win->GetMouseY());

		if(win->mouseinside==true && g && g==win->mousedowngadget)
		{
		switch(g->type)
		{	
		case GADGETTYPE_BUTTON_IMAGE:
		case GADGETTYPE_BUTTON_TEXT:
		g->guilist->win->Gadget(g);
		break;
		}
		}
		else
		*/

		{
			// win->MouseMove(true);
			if(win->editactivenumberobject_left==true)
			{
				if(win->activenumberobject)
				{
					win->EditNumberObjectReleased(win->activenumberobject,0);
					//win->activenumberobject->init=false;
				}

				win->activenumberobject=0;
				win->editactivenumberobject_left=false;
				win->activenumberobject_flag=0;
			}
			else
			{
				switch(flag)
				{
				case MOUSEKEY_LEFT_UP:
					win->MouseButtonLeft(flag);
					break;

				case MOUSEKEY_RIGHT_UP:
					win->MouseButtonRight(flag);
					break;
				}

				win->MouseButton(flag);
			}

			/*
			{
			}

			switch(editortype)
			{
			case EDITORTYPE_ARRANGE:
			((Edit_Arrange *)editor)->ArrangeMouseButton();
			break;

			case EDITORTYPE_PIANO:
			((Edit_Piano *)editor)->PianoMouseButton();
			break;
			}
			*/
		}
	}

	return g;
}

void GUI::CheckMouseDown(guiWindow *win,int flag)
{	

#ifdef WIN32
	//SetFocus(win->hWnd);
#endif

	win->refreshmousebuttondown=false;

	/*
	win->mousedowngadget=win->gadgetlists.FindGadget(win->GetMouseX(),win->GetMouseY()); // Gui Gadgets

	if(!win->mousedowngadget) // Default Mouse Button Message
	{
	*/

	switch(flag)
	{
	case MOUSEKEY_DBCLICK_LEFT:
	case MOUSEKEY_LEFT_DOWN:
		win->MouseButtonLeft(flag);
		break;

	case MOUSEKEY_RIGHT_DOWN:
		win->MouseButtonRight(flag);
		break;
	}

	win->MouseButton(flag);
	//}	
}

void GUI::RefreshAudioHDFile(AudioHDFile *hdfile,AudioRegion *r,bool refreshall,int filestorefresh)
{
	if(hdfile)
	{
		//maingui->MessageBoxOk(0,"MESSAGE_REFRESHAUDIOHDFILE HD ");
		if(hdfile->nopeakfileasfile==false)
			hdfile->CreatePeakFile(false);

		guiWindow *guiw=maingui->FirstWindow();

		while(guiw)
		{
			switch(guiw->GetEditorID())
			{
			case EDITORTYPE_AUDIOMASTER:
				{
					Edit_AudioMaster *em=(Edit_AudioMaster *)guiw;

					if(em->masterhdfile)
					{
						if(em->masterhdfile==hdfile)
							em->DrawDBBlit(em->showsound);
					}
				}
				break;

			case EDITORTYPE_CROSSFADE:
				{
					Edit_CrossFade *cf=(Edit_CrossFade *)guiw;

					if(cf->infile==hdfile)
						cf->ShowInCurve();

					if(cf->outfile==hdfile)
						cf->ShowOutCurve();
				}
				break;

			case EDITORTYPE_EVENT:
				{
					Edit_Event *ev=(Edit_Event *)guiw;
					Edit_Event_Event *evt=ev->FirstEvent();

					while(evt)
					{
						switch(evt->seqevent->GetStatus())
						{
						case AUDIO:
							{
								AudioEvent *audio=(AudioEvent *)evt->seqevent;

								if(audio->audioefile==hdfile)
								{
									ev->ShowEventsHoriz(SHOWEVENTS_EVENTS);
									goto next;
								}
							}
							break;
						}

						evt=evt->NextEvent();
					}
				}
				break;

			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)guiw;

					if(r || refreshall==true)
					{
						if(!filestorefresh)
							ar->ShowHoriz(false,false,true);
					}
					else
						ar->RefreshAudio(hdfile);
				}break;

			case EDITORTYPE_SAMPLE:
				{
					Edit_Sample *s=(Edit_Sample *)guiw;

					if(s->audiohdfile==hdfile)
					{
						s->RefreshObjects(0,false);

						//s->ShowOverview();
						//s->ShowWave();
					}
				}
				break;

			case EDITORTYPE_AUDIOMANAGER:
				{
					/*
					Edit_Manager *ed=(Edit_Manager *)guiw;

					if(!filestorefresh)
					ed->ShowAudioFiles();

					ed->RefreshHDFile(hdfile);
					*/

					/*
					if(ed->activehdfile && ed->activehdfile==hdfile)
					ed->ShowActiveHDFile();
					*/
				}
				break;
			}

next:
			guiw=guiw->NextWindow();
		}
	}
}

void GUI::RefreshColour(Seq_Pattern *pattern)
{
	maingui->RefreshAllEditorsWithPattern(pattern->track->song,pattern);

	guiWindow *guiw=maingui->FirstWindow();

	while(guiw)
	{
		if(guiw->WindowSong()==pattern->track->song)
			switch(guiw->GetEditorID())
		{
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
				{
					EventEditor_Selection *e=(EventEditor_Selection *)guiw;

					if(e->patternselection.FindPattern(pattern))
						e->ShowAllEvents(NOBUILD_REFRESH);
				}
				break;
		}

		guiw=guiw->NextWindow();
	}
}

void GUI::RefreshMarkerGUI(Seq_Song *song,Seq_Marker *m)
{
	guiWindow *guiw=maingui->FirstWindow();

	while(guiw)
	{
		if(guiw->WindowSong()==song)
			switch(guiw->GetEditorID())
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ea=(Edit_Arrange *)guiw;

					ea->RefreshStartPosition();
					ea->RefreshMarker();
				}
				break;

			case EDITORTYPE_EVENT:
				{
					Edit_Event *e=(Edit_Event *)guiw;

					if(e->timeline)
						e->RefreshStartPosition();

					e->RefreshMarker();
				}
				break;

			case EDITORTYPE_TEMPO:
				{
					EventEditor *e=(EventEditor *)guiw;

					e->RefreshStartPosition();
					e->RefreshMarker();
				}
				break;

			case EDITORTYPE_PIANO:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
				{
					EventEditor *e=(EventEditor *)guiw;
					e->RefreshStartPosition();
					e->RefreshMarker();
				}
				break;

			case EDITORTYPE_MARKER:
				{
					Edit_Marker *em=(Edit_Marker *)guiw;
					em->RefreshObjects(0,false);
				}
				break;
		}

		guiw=guiw->NextWindow();
	}
}

void GUI::RefreshRMGObject(RMGObject *o)
{
	guiWindow *guiw=maingui->FirstWindow();

	while(guiw)
	{
		switch(guiw->GetEditorID())
		{
		case EDITORTYPE_RMG:
			{
				Edit_RMG *rmg=(Edit_RMG *)guiw;
				rmg->RefreshObject(o);
			}
			break;
		}

		guiw=guiw->NextWindow();
	}
}

void GUI::RefreshColour(Seq_Track *track)
{
	if(track->parent)
		track=(Seq_Track *)track->parent;

	guiWindow *guiw=maingui->FirstWindow();

	while(guiw)
	{
		if(guiw->refreshed==false && guiw->WindowSong()==track->song)
			switch(guiw->GetEditorID())
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)guiw;
					Edit_Arrange_Track *eat;

					if(eat=ar->FindTrack(track))
						guiw->refreshed=true;

					// + Child
					Seq_Track *lct=track->FirstChildTrack();
					while(lct && lct->parent==track)
					{
						if(eat=ar->FindTrack(lct))
							guiw->refreshed=true;

						lct=lct->NextTrack();
					}

					if(guiw->refreshed==true)
					{
						ar->ShowHoriz(true,false,false);
					}
				}
				break;

			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
				{
					EventEditor_Selection *e=(EventEditor_Selection *)guiw;

					if(e->patternselection.FindTrack(track))
						e->ShowAllEvents(NOBUILD_REFRESH);
				}
				break;

			case EDITORTYPE_ARRANGELIST:
				guiw->RefreshObjects(0,false);
				break;
		}

		guiw=guiw->NextWindow();
	}
}

void GUI::RefreshColour(Seq_Group *group)
{
	guiWindow *guiw=maingui->FirstWindow();

	while(guiw)
	{
		if(guiw->WindowSong()==group->song)
			switch(guiw->GetEditorID())
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)guiw;
					guiObject *o=ar->guiobjects.FirstObject();

					while(o)
					{
						if(o->id==OBJECTID_ARRANGETRACK)
						{
							Edit_Arrange_Track *c=(Edit_Arrange_Track *)o;

							if(c->track->GetGroups()->FindGroup(group)==true)
								c->Refresh(REFRESH_EAT_PATTERNANDTRACK);
						}

						o=o->NextObject();
					}
				}
				break;

				/*
				case EDITORTYPE_EVENT:
				{
				EventEditor *e=(EventEditor *)guiw;

				e->ShowAllEvents(NOBUILD_REFRESH);
				}
				break;
				*/
		}

		guiw=guiw->NextWindow();
	}
}

void GUI::RefreshColour(AudioChannel *channel)
{
	guiWindow *guiw=maingui->FirstWindow();

	while(guiw)
	{
		if(guiw->WindowSong()==channel->audiosystem->song)
			switch(guiw->GetEditorID())
		{
			case EDITORTYPE_AUDIOMIXER:
				{
					Edit_AudioMix *ar=(Edit_AudioMix *)guiw;

					/*
					Edit_AudioMixEffects *amc=ar->FindChannel(channel);

					if(amc)
					*/

					{
						// +force
						//amc->ShowInputPeak(true);
						//	amc->ShowIOPeak(true);
					}
				}
				break;

		}

		guiw=guiw->NextWindow();
	}
}

void GUI::RefreshEventEditors(Seq_Song *song,guiWindow *not)
{
	guiWindow *c=FirstWindow();

	while(c)
	{
		if((c!=not) && c->WindowSong()==song)
		{
			switch(c->GetEditorID())
			{
			case EDITORTYPE_PIANO:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_SCORE:
			case EDITORTYPE_WAVE:
				c->RefreshObjects(0,false);
				break;
			}
		}

		c=c->NextWindow();
	}
}

void GUI::RefreshScreenNames()
{
	guiScreen *s=FirstScreen();
	while(s)
	{
		s->SetTitle();
		s=s->NextScreen();
	}
}

void GUI::RefreshScreenMenus()
{
	guiScreen *s=FirstScreen();
	while(s)
	{
		CreateScreenMenu(s,0);

#ifdef WIN32
		SetMenu(s->hWnd,s->menu->OSMenuHandle);
#endif

		s=s->NextScreen();
	}
}

void GUI::RefreshEditor(guiWindow *win)
{
	switch(win->GetEditorID())
	{
	case EDITORTYPE_ARRANGE:
		// ArrangeEditor_Refresh(win);
		break;
	}
}

void GUI::MenuSelected(guiMenu *menu,bool popup)
{
	TRACE ("Menu Selected..\n");

	if(menu)
		menu->MenuFunction(); // Window can be closed !
}

bool CheckIfMenu(guiWindow *w,guiMenu *menu,int id)
{
	if(menu->index==id)
	{
		maingui->MenuSelected(menu,true);
		return true;
	}

	guiMenu *child=menu->FirstMenu();
	while(child)
	{
		if(CheckIfMenu(w,child,id)==true)
			return true;

		child=child->NextMenu();
	}

	return false;
}

bool GUI::CheckOSMenu(guiMenu *menu,int id)
{
	guiMenu *hm=menu->FirstMenu();

	while(hm) // -> Header
	{
		guiMenu *vm=hm->FirstMenu();

		while(vm) // --> V Menu
		{
			if(vm->id==id)
			{
				maingui->MenuSelected(vm,false);
				return true;
			}

			guiMenu *vhm=vm->FirstMenu(); // -> V - Horz

			while(vhm) // --> V Menu
			{
				if(vhm->id==id)
				{
					maingui->MenuSelected(vhm,false);
					return true;
				}

				vhm=vhm->NextMenu();
			}

			vm=vm->NextMenu();
		}

		hm=hm->NextMenu();
	}

	return false;
}

void GUI::CheckScreenMenu(guiScreen *s,int id)
{
	if(!s)return;

	if(s->menu)
		CheckOSMenu(s->menu,id);
}

void GUI::CheckMenu(guiWindow *w,int id)
{
	if(!w)
		return;

	if(id>=5000) // POPUP
	{
		id-=5000;

		TRACE ("Check PopUp Menu %d\n",id);

		if(w->popmenu)
		{
			if(CheckIfMenu(w,w->popmenu,id)==true)
				w->DeletePopUpMenu();
		}
		else
			TRACE ("No PopUp !\n");
	}
	else
		if(w->popmenu)
		{
			if(CheckOSMenu(w->popmenu,id)==true)
				w->DeletePopUpMenu();

		}
		else
			if(w->menu)// window menu
				CheckOSMenu(w->menu,id);
			else
				if(w->screen && w->screen->menu)
					CheckOSMenu(w->screen->menu,id);
}

bool GUI::CheckCommand(guiWindow *win,int code,int id,
#ifdef WIN32
					   HWND child
#endif
					   )
{
	if(win)
	{
		if(child==0)
		{
			// Menu
			//win->ReleaseMouse();
			TRACE ("Menu ID %d\n",id);

			CheckMenu(win,id);
		}
		else // Gadget
		{
			win->CheckGadget(child,code,id);
		}
	}
	else
		TRACE ("Command No Win...\n");

	return true;
}

guiMenu *GUI::CheckMenuOrPopup(guiWindow *win,guiGadget *child,guiMenu *menu,int nVirtkey,bool shiftkey,bool ctrlkey)
{
	guiMenu *m;

	if(menu && (m=menu->FirstMenu()) )
	{
		// strg pressed ?
		while(m)
		{
			if(guiMenu *m1=m->FirstMenu())
			{
				while(m1)
				{
					if(guiMenu *m2=m1->FirstMenu())
					{
						while(m2){
							if(guiMenu *found=m2->CheckKey(nVirtkey,shiftkey,ctrlkey))
								return found;

							m2=m2->NextMenu();
						}
					}

					if(guiMenu *found=m1->CheckKey(nVirtkey,shiftkey,ctrlkey))
						return found;

					m1=m1->NextMenu();
				}
			}

			if(guiMenu *found=m->CheckKey(nVirtkey,shiftkey,ctrlkey))
				return found;

			m=m->NextMenu();
		}
	}

	return 0;
}

guiMenu *GUI::CheckHotKey(guiWindow *win,guiGadget *child,UBYTE nVirtkey,bool shiftkey,bool ctrlkey)
{
	if(win->menu)
	{
		guiMenu *main=CheckMenuOrPopup(win,child,win->menu,nVirtkey,shiftkey,ctrlkey);
		if(main)
			return main;
	}

	// Check Menu Hotkeys...
	if(win->screen && win->screen->menu)
	{
		guiMenu *main=CheckMenuOrPopup(win,child,win->screen->menu,nVirtkey,shiftkey,ctrlkey);
		if(main)
			return main;
	}

	if(child)
	{
		// Pop Ups ?
		switch(child->menuindex)
		{
		case 0:
			{
				win->CreateMenu();

				if(win->popmenu)
				{
					return CheckMenuOrPopup(win,child,win->popmenu,nVirtkey,shiftkey,ctrlkey);
				}
			}
			break;

		case 1:
			{
				win->CreateMenu2();

				if(win->popmenu)
				{
					return CheckMenuOrPopup(win,child,win->popmenu,nVirtkey,shiftkey,ctrlkey);
				}
			}
			break;
		}
	}

	return 0;
}

void GUI::CheckKeyDown(guiWindow *win,guiGadget *child)
{
#ifdef WIN32

	//UINT mapkey=MapVirtualKey(win->nVirtKey,MAPVK_VK_TO_CHAR);

	//if(mapkey)
	//{
	//	win->nVirtKey=mapkey;
	//}
	//else
	switch(win->nVirtKey)
	{
	case VK_BACK:
		win->nVirtKey=VK_DELETE;
		break;
	}
#endif

	if(win->repeatkey==false)
	{
		// Check Menu Hotkey

		if(guiMenu *found=CheckHotKey(win,child,win->nVirtKey,maingui->GetShiftKey(),maingui->GetCtrlKey()))
		{
			win->hotkey=true;
			MenuSelected(found,false); // Hot Key pressed
			win->hotkey=false;

			if(child)
			{
				if(win->skipdeletepopmenu==false)
					win->DeletePopUpMenu();
			}

			return;
		}

		if(win && win->usemenuofwindow)
		{
			if(guiMenu *found=CheckHotKey(win->usemenuofwindow,0,win->nVirtKey,maingui->GetShiftKey(),maingui->GetCtrlKey()))
			{
				win->usemenuofwindow->hotkey=true;
				MenuSelected(found,false); // Hot Key pressed
				win->usemenuofwindow->hotkey=false;
				return;
			}
		}

	}

	//win->nVirtKey=nVirtkey;

	//if(!win->popmenu)


	// Global Keys


	if(win->repeatkey==false)
	{
		switch(win->nVirtKey)
		{
			/*
			case 'y':
			case 'Y':
			win->ykeydown=true;
			break;

			case 'x':
			case 'X':
			win->xkeydown=true;
			break;
			*/

		case KEY_ESCAPE:
			{
				if(dragdropobject)
				{
					CancelDragDrop();
					return;
				}
				else
					mainaudioreal->StopAllRealtimeEvents();
			}
			break;

#ifdef WIN32
		case VK_MEDIA_STOP:

			// Start Stop

			if(Seq_Song *song=win->WindowSong())
			{
				if(song->status&Seq_Song::STATUS_SONGPLAYBACK_MIDI)
					song->StopSelected();

				return;
			}
			break;
#endif

		case KEYSPACE:
		case KEY_KEY0_10:
#ifdef WIN32
		case VK_MEDIA_PLAY_PAUSE:
#endif
			{
				if(win->nVirtKey==KEYSPACE && GetShiftKey()==true)
				{
					if(Seq_Song *song=win->WindowSong())
					{
						if(song->status&Seq_Song::STATUS_SONGPLAYBACK_MIDI)
							song->StopSelected();
						else
							song->RecordSong();

						song->waitforspaceup=true;

						return;
					}
				}

				// Start Stop

				if(Seq_Song *song=win->WindowSong())
				{
					if(song->status&Seq_Song::STATUS_SONGPLAYBACK_MIDI)
						song->StopSelected();
					else
						song->PlaySong();

					song->waitforspaceup=true;

					return;
				}
			}
			break;
		}

		win->KeyDown();
	}
	else
		win->KeyDownRepeat();

}

void GUI::CheckKeyUp(guiWindow *win)
{
	win->KeyUp();
}

void GUI::SongCycleChanged(Seq_Song *song)
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->WindowSong()==song)
			switch(w->GetEditorID())
		{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_TEMPO:
			case EDITORTYPE_SCORE:
			case EDITORTYPE_SAMPLE:
				{
					if(w->timeline)
					{
						//w->timeline->Draw();
						EventEditor *ee=(EventEditor *)w;
						ee->RefreshStartPosition();
					}
				}
				break;

			case EDITORTYPE_TRANSPORT:
				{
					Edit_Transport *et=(Edit_Transport *)w;

					et->ShowCycle(false);
					//	et->RefreshMenu();
				}
				break;
		}

		w=w->NextWindow();
	}
}

bool GUI::CheckOpenEditor(int type,Seq_Song *song,Seq_Track *track,Seq_SelectionList *list)
{
	if(list)
	{
		switch(type)
		{
		case EDITORTYPE_PIANO:
		case EDITORTYPE_DRUM:
		case EDITORTYPE_WAVE:
		case EDITORTYPE_SCORE:
			{
				Seq_SelectedPattern *sel=list->FirstSelectedPattern();
				while(sel)
				{
					if(sel->pattern->mediatype==MEDIATYPE_MIDI)
						break;

					sel=sel->NextSelectedPattern();
				}

				if(!sel)
					return true;
			}
			break;

		case EDITORTYPE_SAMPLE:
			{
				Seq_SelectedPattern *sel=list->FirstSelectedPattern();
				while(sel)
				{
					if(sel->pattern->mediatype==MEDIATYPE_AUDIO)
						break;

					sel=sel->NextSelectedPattern();
				}

				if(!sel)
					return true;
			}

			break;

		}
	}

	if(mainsettings->openmultieditor==true)
		return false;

	guiWindow *found=0,*w=FirstWindow();

	while(w)
	{
		if(w->GetEditorID()==type)
		{
			switch(type)
			{
			case EDITORTYPE_KEYBOARD:
			case EDITORTYPE_BIGTIME:
				found=w;
				break;

			case EDITORTYPE_PIANO:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_SCORE:
				if(w->WindowSong()==song && list)
				{
					EventEditor_Selection *ed=(EventEditor_Selection *)w;

					if(ed->patternselection.Compare(list)==true)
						found=w;
				}
				break;

			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_TEXT:
			case EDITORTYPE_MARKER:
			case EDITORTYPE_TEMPO:
				if(w->WindowSong()==song)
				{				
					found=w;
				}
				break;

			}
		}

		if(found)break;

		w=w->NextWindow();
	}

	if(found)
	{
		found->WindowToFront(true);
		return true;
	}

	return false;
}

guiWindow *GUI::OpenEditorStart(int type,
								Seq_Song *song,
								Seq_Track *track,
								Seq_SelectionList *selection,
								guiWindowSetting *settings,
								Object *object,
								LONGLONG startposition)
{
	guiWindow *win=0;
	guiScreen *toscreen=0;

	if(settings)
		toscreen=settings->screen;

	if(!toscreen)
		toscreen=GetActiveScreen();

	switch(type)
	{
		/*
		case EDITORTYPE_WIN32AUDIO:
		if(object) // Edit_Settings
		{
		Edit_Settings *ed=(Edit_Settings *)object;

		if(mainaudio->GetActiveDevice())
		{
		guiWindow *win=FirstWindow();
		while(win)
		{
		if(win->GetEditorID()==EDITORTYPE_WIN32AUDIO)
		{
		Edit_Win32Audio *edy=(Edit_Win32Audio *)win;

		if(edy->audiodevice==mainaudio->GetActiveDevice())
		{
		edy->WindowToFront(true);
		break;
		}
		}

		win=win->NextWindow();
		}

		if(!win)
		{
		if(Edit_Win32Audio *wed=new Edit_Win32Audio(ed,mainaudio->GetActiveDevice()))
		{
		win=OpenWindow(wed,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
		}
		}
		}
		}
		break;
		*/

	case EDITORTYPE_UPDATE:
		{
			guiWindow *w=maingui->FirstWindow();
			while(w)
			{
				if(w->GetEditorID()==EDITORTYPE_UPDATE)
					break;

				w=w->NextWindow();
			}

			if(w)
				w->WindowToFront(true);
			else

				if(Edit_UpDate *ied=new Edit_UpDate(object?true:false)) // Object==MessageBox on/off
				{
					guiWindowSetting csettings;

					csettings.s_ondesktop=true;

					if(!(win=OpenWindow(ied,&csettings,0,0)))
						delete ied;
					else
						win->guiSetWindowText("CamX Version/Updates");
				}
		}
		break;

	case EDITORTYPE_CAMXINFO:
		if(GetActiveScreen() && (!infoWindow))
		{
			if(Edit_CamXInfo *ied=new Edit_CamXInfo())
			{
				guiWindowSetting csettings;

				csettings.startposition_x=0;
				csettings.startposition_y=0;

				csettings.startheight=GetButtonSizeY(9);
				double w=csettings.startheight*1.6;

				csettings.startwidth=(int)w;
				csettings.s_ondesktop=true;

				infoWindow=OpenWindow(ied,&csettings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
			}
		}
		break;

	case EDITORTYPE_CREATEBUS:
		{
			if(Edit_CreateBus *ect=new Edit_CreateBus)
			{
				guiWindowSetting csettings;

				csettings.startposition_x=0;
				csettings.startposition_y=0;

				int w=GetButtonSizeY(11)+maingui->GetButtonSizeY()-9;

				csettings.startheight=GetButtonSizeY(5)+maingui->GetButtonSizeY()-3;
				csettings.startwidth=2*w;

				csettings.s_ondesktop=true;

				win=OpenWindow(ect,&csettings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

				if(!win)delete ect;
				else
					win->guiSetWindowText(Cxs[CXS_CREATEBUS]);
			}
		}
		break;

	case EDITORTYPE_CREATETRACKS:
		{
			if(Edit_CreateTracks *ect=new Edit_CreateTracks)
			{
				guiWindowSetting csettings;

				csettings.startposition_x=0;
				csettings.startposition_y=0;

				int w=GetButtonSizeY(11)+maingui->GetButtonSizeY()-9;

				csettings.startheight=w;
				csettings.startwidth=2*w;

				csettings.s_ondesktop=true;

				win=OpenWindow(ect,&csettings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

				if(!win)delete ect;
				else
					win->guiSetWindowText(Cxs[CXS_CREATECLONETRACKS]);
			}
		}
		break;

	case EDITORTYPE_MONITOR:
		if(CheckOpenEditor(EDITORTYPE_MONITOR)==false)
		{
			if(Edit_Monitor *edm= new Edit_Monitor)
			{
				win=OpenWindow(edm,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete edm;
			}
		}
		break;

		/*
		case EDITORTYPE_DRUMMAP:
		{
		if(Edit_Drummap *edm=new Edit_Drummap)
		{
		win=OpenWindow(edm,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
		if(!win)delete edm;
		}
		}
		break;
		*/

	case EDITORTYPE_PLAYER:
		{
			if(Edit_Player *edm=new Edit_Player)
			{
				win=OpenWindow(edm,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete edm;
			}
		}
		break;

	case EDITORTYPE_PROCESSOR:
		{
			if(Edit_Processor *proc=new Edit_Processor)
			{
				win=OpenWindow(proc,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete proc;
			}
		}
		break;

	case EDITORTYPE_KEYBOARD:
		if(CheckOpenEditor(EDITORTYPE_KEYBOARD)==false)
		{
			if(Edit_Keyboard *key=new Edit_Keyboard)
			{
				win=OpenWindow(key,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete key;
			}
		}
		break;

	case EDITORTYPE_BIGTIME:
		if(mainvar->GetActiveSong() && CheckOpenEditor(EDITORTYPE_BIGTIME)==false)
		{
			if(Edit_BigTime *bigtime=new Edit_BigTime())
			{
				guiWindowSetting bsettings;

				bsettings.s_ondesktop=true;

				bsettings.startposition_x=0;
				bsettings.startposition_y=0;
				bsettings.startwidth=bigtime->GetZoomWidth();
				bsettings.startheight=bigtime->GetZoomHeight();

				bigtime->InitOpenWindow();
				win=OpenWindow(bigtime,&bsettings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete bigtime;
			}
		}
		break;

	case EDITORTYPE_RECORDEDITOR:
		if(Edit_RecordingSettings *ed=new Edit_RecordingSettings)
		{
			ed->song=song;
			win=OpenWindow(ed,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
			if(!win)delete ed;
		}
		break;

	case EDITORTYPE_SYNCEDITOR:
		if(Edit_SyncEditor *ed=new Edit_SyncEditor)
		{
			ed->song=song;
			win=OpenWindow(ed,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
			if(!win)delete ed;
		}
		break;

	case EDITORTYPE_CPU:
		if(CheckOpenEditor(EDITORTYPE_CPU)==false)
		{
			if(Edit_CPU *nt=new Edit_CPU)
			{
				win=OpenWindow(nt,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete nt;
			}
		}
		break;

	case EDITORTYPE_TOOLBOX:
		if(settings && settings->screen)
		{
			guiWindow *w=maingui->FirstWindow();
			while(w)
			{
				if(w->GetEditorID()==EDITORTYPE_TOOLBOX)
				{
					Edit_Toolbox *et=(Edit_Toolbox *)w;

					if(et->fromscreen==settings->screen)
						break;

				}

				w=w->NextWindow();
			}

			if(w)
				w->WindowToFront(true);
			else
				if(Edit_Toolbox *nt=new Edit_Toolbox)
				{
					nt->fromscreen=settings->screen;
					settings->screen=0;

					win=OpenWindow(nt,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
					if(!win)delete nt;
				}
		}
		break;

	case EDITORTYPE_LIBRARY:
		if(CheckOpenEditor(EDITORTYPE_LIBRARY)==false)
		{
			if(Edit_Library *et=new Edit_Library)
			{
				et->InitOpenWindow();
				et->SetWindowName();
				win=OpenWindow(et,settings,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete et;
			}
		}
		break;

	case EDITORTYPE_TEMPO:
		if(song && song->loaded==true && CheckOpenEditor(EDITORTYPE_TEMPO,song)==false)
		{
			if(Edit_Tempo *et=new Edit_Tempo)
			{
				guiWindowSetting set;

				if(!settings)settings=&set;

				settings->startposition=settings->calledfromwindow?settings->calledfromwindow->startposition:0;

				et->song=song;
				et->InitOpenWindow(settings);

				et->SetWindowName();

				settings->formx=0;
				settings->formy=1;
				settings->screen=toscreen;

				win=OpenWindow(et,settings,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete et;
			}
		}
		break;


	case EDITORTYPE_SAMPLE:
		{
			if(settings && song && song->loaded==true)
			{
				if(Edit_Sample_StartInit *asf=(Edit_Sample_StartInit *)object)
				{
					if(asf->file)
					{
						if(Edit_Sample *em=new Edit_Sample)
						{
							settings->formx=0;
							settings->formy=1;
							settings->screen=toscreen;
							settings->startposition=asf->startposition;

							em->SetWindowName();
							em->pattern=asf->pattern;

							if(!em->pattern)
							{
								em->songmode=em->songmodepossible=false;
								em->windowtimeformat=WINDOWDISPLAY_SECONDS; // Sec
							}

							em->audiohdfile=asf->file;

							em->clipstart=asf->rangestart;
							em->clipend=asf->rangeend;

							asf->file->Open();

							em->song=song;
							em->InitOpenWindow(settings);

							win=OpenWindow(em,settings,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

							if(!win)delete em;
						}
					}
				}
			}
		}
		break;

	case EDITORTYPE_CROSSFADE:
		if(song && song->loaded==true)
		{
			Seq_CrossFade *crossfade=(Seq_CrossFade *)object;
			guiWindow *f=FirstWindow();
			bool found=false;

			while(f && found==false)
			{
				if(f->WindowSong()==song)
					switch(f->GetEditorID())
				{
					case EDITORTYPE_CROSSFADE:
						{
							Edit_CrossFade *ecf=(Edit_CrossFade *)f;

							if(ecf->out==crossfade)
							{
								f->WindowToFront(true);
								found=true;
							}
						}
						break;
				}

				f=f->NextWindow();
			}

			if(found==false)
			{
				if(Edit_CrossFade *ncf=new Edit_CrossFade(crossfade))
				{
					ncf->song=song;
					win=OpenWindow(ncf,settings,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
					if(!win)delete ncf;
				}
			}
		}
		break;

	case EDITORTYPE_TEXT:
		if(song && song->loaded==true && CheckOpenEditor(EDITORTYPE_TEXT,song)==false)
		{
			if(Edit_Text *et=new Edit_Text)
			{
				et->song=song;
				et->InitOpenWindow(settings);

				guiWindowSetting set;
				set.startposition=startposition;
				et->SetWindowName();
				win=OpenWindow(et,&set,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete et;
			}
		}
		break;

	case EDITORTYPE_MARKER:
		if(song && song->loaded==true && CheckOpenEditor(EDITORTYPE_MARKER,song)==false)
		{
			if(Edit_Marker *et=new Edit_Marker)
			{
				et->song=song;
				et->InitOpenWindow(settings);

				guiWindowSetting set;
				set.startposition=startposition;
				et->SetWindowName();
				win=OpenWindow(et,&set,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete et;
			}
		}
		break;

	case EDITORTYPE_MIDIFILTER:
		if(object)
		{
			if(Edit_MIDIFilter *em=new Edit_MIDIFilter((MIDIFilter *)object))
			{
				em->song=song;

				win=OpenWindow(em,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete em;					
			}
		}
		break;

	case EDITORTYPE_AUDIOFREEZE:
		if(song && song->loaded==true && CheckOpenEditor(EDITORTYPE_AUDIOFREEZE,song)==false)
		{
			Edit_AudioMaster *em=new Edit_AudioMaster(song);

			if(em)
			{
				em->song=song;
				win=OpenWindow(em,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete em;
			}
		}
		break;

	case EDITORTYPE_AUDIOMASTER:
		if(song && song->loaded==true && CheckOpenEditor(EDITORTYPE_AUDIOMASTER,song)==false)
		{
			Edit_AudioMaster *em=new Edit_AudioMaster(song,track,(Seq_Pattern *)object);

			if(em)
			{
				em->song=song;
				win=OpenWindow(em,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete em;
			}
		}
		break;

	case EDITORTYPE_QUANTIZEEDITOR:
		{
			if(object) // Object == QuantizeEffe
			{
				Edit_QuantizeEditor *em=new Edit_QuantizeEditor((QuantizeEffect *)object);

				if(em)
				{
					em->song=song;
					win=OpenWindow(em,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
					if(!win)delete em;
					else em->ShowTitle();
				}
			}
		}
		break;

	case EDITORTYPE_EDITDATA:
		if(toscreen)
		{
			if(object) // object==EditData
			{
				EditData *edata=(EditData *)object;

				if(Edit_Data *em=new Edit_Data(edata))
				{
					bool skipposition=false;
					guiWindowSetting set;

					if(settings==NULL)
						settings=&set;

					switch(edata->type)
					{
					case EditData::EDITDATA_TYPE_COPYMOVEPATTERN:
						{
							settings->startwidth=20*maingui->GetFontSizeY();

							em->resizeable=true;
							em->minwidth=15*maingui->GetFontSizeY();
							em->minheight=em->maxheight=settings->startheight=4*maingui->GetButtonSizeY();
							em->song=edata->song;
						}
						break;

					case EditData::EDITDATA_TYPE_INTEGER_OKCANCEL:
						settings->startwidth=360;
						settings->startheight=2*maingui->GetButtonSizeY();
						break;

					case EditData::EDITDATA_TYPE_PROGRAM:
						{
							settings->startwidth=360;
							settings->startheight=6*maingui->GetButtonSizeY();
						}
						break;

					case EditData::EDITDATA_TYPE_STRING:
					case EditData::EDITDATA_TYPE_STRING_TITLE:
					case EditData::EDITDATA_TYPE_DOUBLE:
					case EditData::EDITDATA_TYPE_INTEGER:
					case EditData::EDITDATA_TYPE_INFOSTRING:
						{
							settings->startwidth=edata->width>0?edata->width:10*maingui->GetFontSizeX();
							settings->startheight=2*maingui->GetButtonSizeY();

							if(edata->type==EditData::EDITDATA_TYPE_STRING_TITLE)
							{
								settings->title=edata->title;
							}

							if(edata->type== EditData::EDITDATA_TYPE_INFOSTRING)
							{
								settings->noactivate=true;
							}

						}
						break;

					case EditData::EDITDATA_TYPE_TIME:
						{
							settings->startwidth=250;
							settings->startheight=170;
						}
						break;

					case EditData::EDITDATA_TYPE_VOLUMEDBNOADD:
					case EditData::EDITDATA_TYPE_VOLUMEDB:
						{
							settings->startwidth=50;
							settings->startheight=300;
						}
						break;

					case EditData::EDITDATA_TYPE_SIGNATURE:
						{
							settings->startwidth=200;
							settings->startheight=180;
						}
						break;

					case EditData::EDITDATA_TYPE_PLUGINPROGRAMS:
						{
							edata->x=edata->y=0;
							skipposition=true;
							em->minwidth=settings->startwidth=20*maingui->GetFontSizeY();
							em->minheight=settings->startheight=50*maingui->GetFontSizeY();
							em->resizeable=true;

						}
						break;

					}

					if(settings->title==0)
						settings->title=edata->title;

					if(edata->win && skipposition==false)
					{
#ifdef WIN32
						POINT lpPoint;

						lpPoint.x=edata->x;
						lpPoint.y=edata->y;

						BOOL r=ClientToScreen(edata->parentdb?edata->parentdb->hWnd:edata->win->hWnd,&lpPoint);

						edata->x=lpPoint.x;
						edata->y=lpPoint.y;
#endif

						settings->startposition_x=edata->x;
						settings->startposition_y=edata->y-3*maingui->GetFontSizeY();

						if(settings->startposition_y<0)
							settings->startposition_y=edata->y+4*maingui->GetFontSizeY();
					}

					settings->s_ondesktop=true;
					settings->noOSborder=edata->noOSborder;

					em->dontchangesettings=true;
					edata->editdatawin=em;

					OpenWindow(em,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				}
			}
		}
		break;

	case EDITORTYPE_AUDIOMANAGER:
		if(CheckOpenEditor(EDITORTYPE_AUDIOMANAGER)==false)
		{
			if(Edit_Manager *em=new Edit_Manager)
			{
				win=OpenWindow(em,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
				if(!win)delete em;
			}

		}
		break;


	case EDITORTYPE_GROUP:
		{
			if(song && song->loaded==true && CheckOpenEditor(EDITORTYPE_GROUP,song)==false)
			{
				guiWindow *win=maingui->FindWindow(EDITORTYPE_GROUP,song,0);

				if(!win)
				{
					Edit_Group *ng=new Edit_Group;

					if(ng)
					{
						ng->song=song;
						ng->SetWindowName();
						win=OpenWindow(ng,settings,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

						if(!win)delete ng;
					}

				}
				else
					win->WindowToFront(true);
			}
		}
		break;

	case EDITORTYPE_GROOVE:
		if(CheckOpenEditor(EDITORTYPE_GROOVE,song)==false)
		{
			guiWindow *win=maingui->FindWindow(EDITORTYPE_GROOVE,0,0);

			if(!win)
			{
				if(Edit_Groove *em=new Edit_Groove)
				{
					win=OpenWindow(em,settings,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);
					if(!win)delete em;
				}

			}
			else
				win->WindowToFront(true);
		}
		break;

	case EDITORTYPE_AUDIOMIXER:
		{
			if(song && song->loaded==true && track && selection==0)
			{
				if(Edit_AudioMix *newmix=new Edit_AudioMix)
				{
					if(track)
						newmix->dontchangesettings=true;

					newmix->song=song;
					newmix->track=track;
					newmix->solotrack=true;

					newmix->SetWindowName();

					settings->formx=0;
					settings->formy=0;
					settings->screen=toscreen;

					win=OpenWindow(newmix,settings,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

					if(!win)
						delete newmix;
				}

			}
			else
				if(song && /*song->loaded==true && */ CheckOpenEditor(EDITORTYPE_AUDIOMIXER,song)==false)
				{
					if(toscreen)
					{
						guiWindow *w=maingui->FirstWindow();
						while(w)
						{
							if(w->screen==toscreen && w->parentformchild && w->GetEditorID()==EDITORTYPE_AUDIOMIXER)
							{
								w->WindowToFront(true);
								return 0;
							}

							w=w->NextWindow();
						}
					}

					if(Edit_AudioMix *newmix=new Edit_AudioMix)
					{	
						newmix->song=song;
						guiWindowSetting settings;

						settings.formx=0;
						settings.formy=1;
						settings.screen=toscreen;

						win=OpenWindow(newmix,&settings,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

						if(!win)delete newmix;
					}
				}
		}
		break;

	case EDITORTYPE_ARRANGE:
		if(toscreen)
		{
			if(song /*&& song->loaded==true*/  && CheckOpenEditor(EDITORTYPE_ARRANGE,song)==false)
			{	
				if(Edit_Arrange *newt=new Edit_Arrange)
				{
					bool ok=false;

					newt->song=newt->selection.song=song;
					newt->InitOpenWindow();

					// song->AddNewSelectionList(newt,&newt->patternselection); // pattern select list

					guiWindowSetting set;

					set.formx=0;
					set.formy=0;
					set.startposition=startposition;
					set.screen=toscreen;

					// Song Name -> window name
					newt->SetWindowName();

					win=OpenWindow(newt,&set,song,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

					if(!win)
					{
						//song->DeleteSelectionList(&newt->patternselection);
						delete newt;
					}

					/*
					else
					if(win->parentformchild)
					win->parentformchild->Enable(true);
					*/

				}

			}
		}
		break;

	case EDITORTYPE_TRANSPORT:
		//if(CheckOpenEditor(EDITORTYPE_TRANSPORT)==false)
		if(toscreen)
		{
			if(Edit_Transport *newt=new Edit_Transport)
			{
				guiWindowSetting set;

				set.s_ondesktop=mainsettings->transportdesktop;

				newt->song=song;
				newt->InitOpenWindow();

				set.formx=0;
				set.formy=2;
				set.screen=toscreen;

				win=OpenWindow(newt,&set,song,0);
			}
		}
		break;

	case EDITORTYPE_EVENT:
		if(song && song->loaded==true && selection && selection->GetCountOfRealSelectedPattern()>0 &&
			CheckOpenEditor(EDITORTYPE_EVENT,song,0,selection)==false
			)
		{
			if(Edit_Event *newt=new Edit_Event)
			{
				bool ok=false;

				guiWindowSetting set;

				set.startposition=startposition>=0?startposition:0;

				newt->song=song;
				newt->InitOpenWindow(settings);
				//newt->default_notelength=mainsettings->defaultpianoeditorlength;

				song->AddNewSelectionList(newt,&newt->patternselection,selection); // new+copy 

				set.formx=0;
				set.formy=1;
				set.screen=toscreen;

				newt->SetWindowName();

				/*
				set.startposition=startposition;

				set.formx=0;
				set.formy=1;
				set.screen=toscreen;

				newt->song=song;
				newt->InitOpenWindow();

				song->AddNewSelectionList(newt,&newt->patternselection,selection); // new+copy

				newt->SetWindowName();
				*/

				win=OpenWindow(newt,&set,song,WINDOWFLAG_NOSIZE);

				if(!win)
				{
					newt->patternselection.DeleteSelectionList();
					delete newt;						
				}
			}

		}
		break;

	case EDITORTYPE_PIANO:
		if(song && song->loaded==true && selection && selection->GetCountOfRealSelectedPattern()>0 &&
			CheckOpenEditor(EDITORTYPE_PIANO,song,0,selection)==false
			)
		{
			if(Edit_Piano *newt=new Edit_Piano)
			{
				bool ok=true;

				guiWindowSetting set;
				set.startposition=startposition>=0?startposition:0;

				newt->song=song;
				newt->InitOpenWindow(settings);
				newt->default_notelength=mainsettings->defaultpianoeditorlength;

				song->AddNewSelectionList(newt,&newt->patternselection,selection); // new+copy 

				set.formx=0;
				set.formy=1;
				set.screen=toscreen;

				newt->SetWindowName();
				win=OpenWindow(newt,&set,song,WINDOWFLAG_NOSIZE);

				if(!win)
				{
					newt->patternselection.DeleteSelectionList();
					delete newt;
				}	
			}

		}
		break;

	case EDITORTYPE_RMG:
		if(song && song->loaded==true && CheckOpenEditor(EDITORTYPE_RMG,song)==false)
		{
			if(Edit_RMG *rmg=new Edit_RMG)
			{
				rmg->song=song;
				rmg->SetWindowName();

				win=OpenWindow(rmg,settings,song,WINDOWFLAG_NOSIZE);

				if(!win)
				{
					delete rmg;

				}	
			}

		}
		break;

	case EDITORTYPE_SCORE:
		if(song && song->loaded==true && selection && selection->GetCountOfRealSelectedPattern()>0 && CheckOpenEditor(EDITORTYPE_SCORE,song,0,selection)==false)
		{
			if(Edit_Score *score=new Edit_Score)
			{
				bool ok=true;

				guiWindowSetting set;
				set.startposition=startposition>=0?startposition:0;
				set.screen=toscreen;

				score->song=song;
				score->InitOpenWindow(settings);

				song->AddNewSelectionList(score,&score->patternselection,selection); // new+copy 
				score->SetWindowName();
				win=OpenWindow(score,&set,song,WINDOWFLAG_NOSIZE);

				if(!win)
				{
					score->patternselection.DeleteSelectionList();
					delete score;
				}	
			}
		}
		break;

	case EDITORTYPE_WAVE:
		if(song && song->loaded==true && selection && selection->GetCountOfRealSelectedPattern()>0 && CheckOpenEditor(EDITORTYPE_WAVE,song,0,selection)==false)
		{
			if(Edit_Wave *newt=new Edit_Wave)
			{
				bool ok=true;

				guiWindowSetting set;
				set.startposition=startposition>=0?startposition:0;
				set.screen=toscreen;

				newt->song=song;
				newt->InitOpenWindow(settings);

				song->AddNewSelectionList(newt,&newt->patternselection,selection); // new+copy 

				newt->wavedefinition=mainwavemap->FirstWaveMap(); // default tracks

				newt->SetWindowName();
				win=OpenWindow(newt,&set,song,WINDOWFLAG_NOSIZE);

				if(!win)
				{
					newt->patternselection.DeleteSelectionList();
					delete newt;
				}
			}
		}
		break;

	case EDITORTYPE_DRUM:
		if((!selection) || (selection->GetCountOfRealSelectedPattern()>0 && song && song->loaded==true && CheckOpenEditor(EDITORTYPE_DRUM,song,0,selection)==false))
		{
			if(Edit_Drum *newt=new Edit_Drum)
			{
				bool ok=true;

				//newt->drummap=(Drummap *)object;

				guiWindowSetting set;

				set.formx=0;
				set.formy=1;
				set.startposition=startposition>=0?startposition:0;
				set.screen=toscreen;

				newt->song=song;
				newt->InitOpenWindow(settings);

				if(selection)
					song->AddNewSelectionList(newt,&newt->patternselection,selection); // new+copy

				newt->SetWindowName();
				win=OpenWindow(newt,&set,song,WINDOWFLAG_NOSIZE);

				if(!win)
				{
					newt->patternselection.DeleteSelectionList();
					delete newt;
				}
			}
		}
		break;

	case EDITORTYPE_SETTINGS:
		{
			if(Edit_Settings *newt=new class Edit_Settings)
			{
				newt->song=song;

				if(startposition==1)
					newt->song=(Seq_Song *)object;
				else
					newt->editproject=(Seq_Project *)object;

				if(newt->song)
					newt->editorname=Cxs[CXS_SONGSETTINGS];
				else
					if(newt->editproject)
						newt->editorname=Cxs[CXS_PROJECTSETTINGS];
					else
						newt->editorname=Cxs[song?CXS_SONGSETTINGS:CXS_SETTINGS];

				newt->SetWindowName();

				win=OpenWindow(newt,0,song,WINDOWFLAG_NOSIZE);

				if(!win)
					delete newt;
			}
		}
		break;
	}

	return win;
}

#ifdef WIN32
int GUI::ConvertOSComandToControl(guiWindow *win,HWND gadgwin,int scrollerpos,int flag)
{
	//win->ReleaseMouse();

	for(int i=0;i<win->glist.gc;i++)
	{
		guiGadget *g=win->glist.gadgets[i];

		if(g->hWnd==gadgwin)
		{
			switch(g->type)
			{
			case GADGETTYPE_SLIDER:
				{		
					guiGadget_Slider *slider=(guiGadget_Slider *)g;

					switch(flag)
					{
					case SCROLLER_PAGEUP: /// <----
						scrollerpos=slider->pos-slider->page;
						break;

					case SCROLLER_PAGEDOWN: /// --
						scrollerpos=slider->pos+slider->page;
						break;
					}

					if(scrollerpos<slider->from)
						scrollerpos=slider->from;
					else
						if(scrollerpos>slider->to)
							scrollerpos=slider->to;

					if(slider->pos!=scrollerpos)
					{
						slider->pos=scrollerpos;
						win->Gadget(g);
					}

					return scrollerpos;
				}
				break;
			}
		}
	}

	return scrollerpos;
}
#endif

guiWindow *GUI::OpenEditor(int type,
						   Seq_Song *song,
						   Seq_Track *track,
						   Seq_SelectionList *selection,
						   guiWindowSetting *settings,
						   Object *object,
						   Object *object2)
{
	guiWindow *win=0;

	switch(type)
	{
	case EDITORTYPE_CREATEAUTOMATIONTRACKS:
		{
			if(Edit_CreateAutomationTrack *ect=new Edit_CreateAutomationTrack((Seq_Track *)object,(AudioChannel *)object2))
			{
				guiWindowSetting csettings;

				csettings.startposition_x=0;
				csettings.startposition_y=0;
				csettings.startwidth=mainsettings->windowpositions[EDITORTYPE_CREATEAUTOMATIONTRACKS].width;
				csettings.startheight=mainsettings->windowpositions[EDITORTYPE_CREATEAUTOMATIONTRACKS].height;

				csettings.s_ondesktop=true;

				win=OpenWindow(ect,&csettings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

				if(!win)delete ect;
				else
					win->guiSetWindowText(Cxs[CXS_CREATENEWAUTOTRACK]);
			}
		}
		break;

	case EDITORTYPE_PROCESSORMODULE:
		if(object)
		{
			MIDIPlugin *proc=(MIDIPlugin *)object;

			Edit_ProcMod *epm=new Edit_ProcMod(proc);

			if(epm)
			{
				guiWindowSetting csettings;

				if(!settings)
				{
					csettings.startposition_x=0;
					csettings.startposition_y=0;

					csettings.startwidth=proc->GetEditorSizeX();
					csettings.startheight=proc->GetEditorSizeY(); // add 30y pixel bypass....

					settings=&csettings;
				}

				if(!settings->startwidth)settings->startwidth=proc->GetEditorSizeX();
				if(!settings->startheight)settings->startheight=proc->GetEditorSizeY(); // add 30y pixel bypass....

				settings->startheight+=30; // Bypass etc...

				win=OpenWindow(epm,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

				if(!win)
					delete epm;
				else
				{
					/*
					char *nwinname=0;

					int i=sizeof("FX *");

					win->song=song;

					i+=strlen(aoi->GetName())+1;

					if(eai->insertaudioeffect->effectlist && eai->insertaudioeffect->effectlist->channel) // Audio Channel Effect
					{
					i+=strlen(eai->insertaudioeffect->effectlist->channel->audiosystem->song->songname)+1;
					i+=strlen(eai->insertaudioeffect->effectlist->channel->name+1);
					i+=1;

					nwinname=new char[i];

					if(nwinname)
					{
					strcpy(nwinname,"FX *");
					mainvar->AddString(nwinname,eai->insertaudioeffect->effectlist->channel->audiosystem->song->songname);
					mainvar->AddString(nwinname,"/");
					mainvar->AddString(nwinname,eai->insertaudioeffect->effectlist->channel->name);
					mainvar->AddString(nwinname,"/");
					mainvar->AddString(nwinname,aoi->GetName());
					}
					}

					if(nwinname)
					{
					win->guiSetWindowText(nwinname);
					delete nwinname;
					}
					else
					win->guiSetWindowText("Intern Plugin");
					*/
				}
			}
		}
		break;

	case EDITORTYPE_PLUGIN_INTERN:
		if(song && object && object2)
		{
			audioobject_Intern *aoi=(audioobject_Intern *)object;
			Edit_Plugin_Intern *eai=new Edit_Plugin_Intern(aoi,(InsertAudioEffect *)object2);

			if(eai)
			{
				eai->song=song;

				guiWindowSetting csettings;

				if(!settings)
				{
					csettings.startposition_x=0;
					csettings.startposition_y=0;

					csettings.startwidth=eai->effect->GetEditorSizeX();
					csettings.startheight=eai->effect->GetEditorSizeY(); // add 30y pixel bypass....

					settings=&csettings;
				}

				if(!settings->startwidth)
					settings->startwidth=eai->effect->GetEditorSizeX();

				if(!settings->startheight)
					settings->startheight=eai->effect->GetEditorSizeY(); // add 30y pixel bypass....

				settings->startheight+=30; // Bypass etc...

				win=OpenWindow(eai,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

				if(!win)
					delete eai;
			}
		}
		break;

	case EDITORTYPE_PLUGIN_VSTEDITOR:
		if(song && object && object2) // Object==VST, Object2==inserteffect
		{
			Edit_Plugin_VST *vst=new Edit_Plugin_VST((VSTPlugin *)object,(InsertAudioEffect *)object2);

			if(vst)
			{
				vst->song=song;
				vst->InitFormSize(); // Get plugineditorwidth+plugineditorheight

				guiWindowSetting csettings;

				int minwidth=maingui->GetFontSizeX()*12;

				mainsettings->windowpositions[EDITORTYPE_PLUGIN_VSTEDITOR].width=vst->plugineditorwidth<minwidth?minwidth:vst->plugineditorwidth;
				mainsettings->windowpositions[EDITORTYPE_PLUGIN_VSTEDITOR].height=vst->plugineditorheight+PLUGINTOPBARHEIGHT;

				if(!settings)
				{
					//MIDI Filter
					csettings.startposition_x=0;
					csettings.startposition_y=0;

					settings=&csettings;

					settings->startwidth=mainsettings->windowpositions[EDITORTYPE_PLUGIN_VSTEDITOR].width;
					settings->startheight=mainsettings->windowpositions[EDITORTYPE_PLUGIN_VSTEDITOR].height;
				}		

				mainsettings->windowpositions[EDITORTYPE_PLUGIN_VSTEDITOR].x=settings->startposition_x;
				mainsettings->windowpositions[EDITORTYPE_PLUGIN_VSTEDITOR].y=settings->startposition_y;

				settings->s_ondesktop=true;

				win=OpenWindow(vst,settings,0,WINDOWFLAG_SIZEH|WINDOWFLAG_SIZEV);

				if(!win)
					delete vst;
			}
		}
		break;
	}

	return win;
}

void GUI::InitNewFileSong(Seq_Song *song,guiScreen *screen)
{
	song->Save(0);
	song->SetSongPosition(0,false);
	song->PRepairPlayback(0,MEDIATYPE_ALL);
	mainvar->SetActiveSong(song);
	maingui->AddNewSongToGUI(song);

	if(screen)
		screen->InitNewSong(song);
}

void GUI::CheckWindowTimer(guiWindow *w,bool leftmouse,bool rightmouse)
{
	if(w->rrt_slowcounter>=10)
		w->rrt_slowcounter=0;
	else
		w->rrt_slowcounter++;

	if(w->closeit==false)
	{
		if(w->winmode==WINDOWMODE_NORMAL)
		{
			CheckTimerMessage(w,leftmouse,rightmouse);

#ifndef CAMXGUIHTREADS
			if(w->hide==false)
			{
				w->RefreshRealtime();
				if(w->rrt_slowcounter==10)
					w->RefreshRealtime_Slow();
			}
#endif
		}

		if(w->timeline)
		{
			if(w->timeline->mousetimex>=0)
				mouseinheader=w->WindowSong();

			if(w->editarea && w->editarea->hWnd==mousehWnd)
				mouseovereditarea=true;
			else
				if(w->editarea2 && w->editarea2->hWnd==mousehWnd)
					mouseovereditarea=true;
				else
					if(w->timeline->dbgadget && w->timeline->dbgadget->hWnd==mousehWnd)
						mouseovereditarea=true;
		}
	}
}

void GUI::RefreshRealtime()
{
	if(mainaudio->GetActiveDevice() && mainaudio->GetActiveDevice()->resetrequest==true)
	{
		mainaudio->ResetAudio(RESETAUDIO_NEWBUFFERSIZE,0);
		mainaudio->GetActiveDevice()->resetrequest=false;
	}

	/*
	//if(mainaudio->lockaudiofiles==false)
	{
	checkaudiofilescounter++;

	if(checkaudiofilescounter==200)
	{
	checkaudiofilescounter=0;
	//	mainaudio->CheckPeakFiles();
	}
	}
	*/

	if(Seq_Song *song=mainvar->GetActiveSong())
	{
		song->CopyRecordDataToRecordPattern();
		song->RefreshRealtime();

		song->LockExternSync();
		if(song->newexterntempochange==true)
		{
			song->newexterntempochange=false; // Check Input Device Tempos
			song->UnlockExternSync();
			song->RefreshExternSongTempoChanges();
		}
		else
			song->UnlockExternSync();

		if(mainsettings->createnewtrack_trackadded==true) // Cycle created new Track
		{
			mainsettings->createnewtrack_trackadded=false;

			// Refresh Arrange
			if(song)
				RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
		}
	}

	if(refreshprogress==true)
	{
		refreshprogress=false;
		ShowRefreshProgress();
	}

	if(rrt_slowcounter>=10)
	{
		rrt_slowcounter=0;

		if(mainvar->GetActiveProject())
			mainvar->GetActiveProject()->RefreshRealtime_Slow();

		if(rrt_MIDIdevicescheck>=50)
		{
			rrt_MIDIdevicescheck=0;

			mainMIDI->RefreshMIDIDevices();
			//	mainMIDI->CollectMIDIInputDevices();
			//	mainMIDI->CollectMIDIOutputDevices();
		}
		else
			rrt_MIDIdevicescheck++;
	}

	rrt_slowcounter++;

	GetCursorPos(&lpPoint);
	mousehWnd=WindowFromPoint(lpPoint);

	if(dragdropobject)
	{
		if(GetLeftMouseButton()==false)
			CancelDragDrop();

		if(dragdropwindow)
		{
			RECT rect;

			GetWindowRect(dragdropwindow->hWnd,&rect);

			if(rect.left!=lpPoint.x || rect.top!=lpPoint.y+maingui->GetFontSizeY()) // Move Drag Drop Info Window ?
				SetWindowPos(dragdropwindow->hWnd,HWND_TOPMOST,lpPoint.x,lpPoint.y+maingui->GetFontSizeY(),rect.bottom-rect.top,rect.right-rect.left,SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
		}
	}

	mouseovereditarea=false;
	mouseinheader=0;

	bool leftmouse=GetLeftMouseButton(),rightmouse=GetRightMouseButton();
	guiWindow *w=FirstWindow();

	while(w)
	{
		CheckWindowTimer(w,leftmouse,rightmouse);
		w=w->NextWindow();
	}

#ifdef CAMXGUIHTREADS
	// Single Core
	if(mainvar->cpucores==1)
	{
		guiWindow *w=FirstWindow();

		while(w)
		{
			if(w->hide==false)
			{
				w->RefreshRealtime();
				if(w->rrt_slowcounter==10)
					w->RefreshRealtime_Slow();
			}

			w=w->NextWindow();
		}
	}
	else
	{
		guiWindow *lastwin=0,*w=FirstWindow();
		int c=0;

		firstfreecorewindow=0;	// Reset

		while(w){

			if(w->hide==false)
			{
				if(!firstfreecorewindow)firstfreecorewindow=w;
				lastwin->nextcorewindow=w;
				lastwin=w;
				c++;
			}

			w=w->NextWindow();
		}

		if(lastwin)
			lastwin->nextcorewindow=0;

		if(c>0)
		{
			if(c==1)
			{
				w->RefreshRealtime();
				if(w->rrt_slowcounter==10)
					w->RefreshRealtime_Slow();
			}
			else
			{
				winproc.SetCoreSignals(c);
				winproc.WaitAllCoresFinished();
			}
		}
	}
#endif

	// End Timer

	if(mouseovereditarea==false && mouseinheader) // Clear All
	{
		TRACE ("Clear ... Mouse Line \n");

		guiWindow *w=FirstWindow();

		while(w)
		{
			if(w->WindowSong())
				w->WindowSong()->mouseposition=-1;

			w=w->NextWindow();
		}
	}
	else
		if(mouseovereditarea==true && mouseinheader) // Clear All
		{
			guiWindow *w=FirstWindow();

			while(w)
			{
				if(w->WindowSong() && w->WindowSong()!=mouseinheader)
					w->WindowSong()->mouseposition=-1;

				w=w->NextWindow();
			}
		}
}

void GUI::ChangeText(Seq_Text *text,char *newtext)
{
	if(text && newtext)
	{
		Seq_Song *song=text->GetSong();

		text->ChangeText(newtext);

		guiWindow *f=FirstWindow();

		while(f)
		{
			if(f->WindowSong()==song)
			{
				switch(f->GetEditorID())
				{
				case EDITORTYPE_TEXT:
					{
						Edit_Text *ed=(Edit_Text *)f;
						ed->RefreshText(text);
					}
					break;
				}
			}

			f=f->NextWindow();
		}
	}
}

void GUI::ChangeMarker(Seq_Marker *text,char *newtext,OSTART start,OSTART end)
{
	if(text && newtext)
	{
		Seq_Song *song=text->GetSong();

		text->ChangeText(newtext);

		guiWindow *f=FirstWindow();

		while(f)
		{
			if(f->WindowSong()==song)
			{
				switch(f->GetEditorID())
				{
				case EDITORTYPE_ARRANGE:
					{
						Edit_Arrange *ar=(Edit_Arrange *)f;
						ar->ShowMarkerMap();
					}
					break;

				case EDITORTYPE_MARKER:
					{
						Edit_Marker *ed=(Edit_Marker *)f;
						ed->RefreshText(text);
					}
					break;

				case EDITORTYPE_TRANSPORT:
					{
						f->RefreshMenu();
					}
					break;
				}

				/*
				if(f->activemdiwindow==true)
				{
				switch(f->GetEditorID())
				{
				case EDITORTYPE_MARKER:
				case EDITORTYPE_TEXT:
				case EDITORTYPE_PIANO:
				case EDITORTYPE_ARRANGE:
				case EDITORTYPE_EVENT:
				case EDITORTYPE_DRUM:
				case EDITORTYPE_WAVE:
				case EDITORTYPE_SCORE:
				f->RefreshMenu();
				break;
				}
				}
				*/
			}

			f=f->NextWindow();
		}
	}
}

void GUI::RefreshAllUndos()
{
	Seq_Project *p=mainvar->FirstProject();

	while(p)
	{
		Seq_Song *s=p->FirstSong();

		while(s){
			s->undo.RefreshUndos();
			s=s->NextSong();
		}

		p=p->NextProject();
	}
}

void GUI::RefreshUndoGUI(Seq_Song *song)
{
	// Undo
	if(song->undo.undo_menustring)
	{
		delete song->undo.undo_menustring;
		song->undo.undo_menustring=0;
	}

	if(char *name=song->undo.GetUndoString())
	{
		size_t sl=strlen(name);

		if(song->undo.undo_menustring=new char[sl+32])
		{
			strcpy(song->undo.undo_menustring,"Undo ");
			mainvar->AddString(song->undo.undo_menustring,name);

			mainvar->AddString(song->undo.undo_menustring,"(");

			char h2[NUMBERSTRINGLEN];
			mainvar->AddString(song->undo.undo_menustring,mainvar->ConvertIntToChar(song->undo.GetNrUndos(),h2));
			mainvar->AddString(song->undo.undo_menustring,")");
		}

		TRACE ("Set Undo %s\n",song->undo.undo_menustring);
	}

	// Redo
	if(song->undo.redo_menustring){
		delete song->undo.redo_menustring;
		song->undo.redo_menustring=0;
	}

	if(char *name=song->undo.GetRedoString())
	{
		size_t sl=strlen(name);

		if(song->undo.redo_menustring=new char[sl+32])
		{
			strcpy(song->undo.redo_menustring,"Redo ");
			mainvar->AddString(song->undo.redo_menustring,name);

			mainvar->AddString(song->undo.redo_menustring,"(");
			char h2[NUMBERSTRINGLEN];
			mainvar->AddString(song->undo.redo_menustring,mainvar->ConvertIntToChar(song->undo.GetNrRedos(),h2));
			mainvar->AddString(song->undo.redo_menustring,")");

			//	mainvar->AddString(song->undo.redo_menustring," \tCtrl+Y");
		}

		TRACE ("Set Redo %s\n",song->undo.redo_menustring);
	}

	// Refresh GUI
	guiScreen *s=FirstScreen();
	while(s)
	{
		if(s->GetSong()==song)
		{
			if(s->menu)
				s->menu->ShowUndoMenu(song);
		}

		s=s->NextScreen();
	}

	guiWindow *w=FirstWindow();

	while(w)
	{	
		if(w->WindowSong()==song)
		{
			if(w->menu)
				w->menu->ShowUndoMenu(song);
		}

		w=w->NextWindow();
	}

	//Refresh Track/Song GUI
	Seq_Track *t=song->FirstTrack();
	while(t)
	{
		// Empty Peak Check
		Seq_Pattern *p=t->FirstPattern(MEDIATYPE_AUDIO);

		while(p)
		{
			AudioPattern *ap=(AudioPattern *)p;

			if(ap->audioevent.audioefile &&
				ap->audioevent.audioefile->samplesperchannel>0 && 
				(!ap->audioevent.audioefile->peakbuffer)
				)
				ap->audioevent.audioefile->CreatePeak();

			p=p->NextPattern(MEDIATYPE_AUDIO);
		}

		t=t->NextTrack();
	}
}

void GUI::RefreshRegionGUI(AudioHDFile *hdfile)
{
	guiWindow *w=FirstWindow();

	while(w)
	{	
		switch(w->GetEditorID())
		{
		case EDITORTYPE_SAMPLEREGIONLIST:
			{
				Edit_RegionList *er=(Edit_RegionList *)w;

				if(hdfile==er->editor->audiohdfile)
				{
					er->ShowRegions();
					er->ShowRegionName();
				}
			}
			break;

			/*
			case EDITORTYPE_SAMPLE:
			{
			Edit_Sample *es=(Edit_Sample *)w;

			if(es->regionsgadget)
			es->ShowRegions(true);
			else
			{
			es->winmode=WINDOWMODE_INIT;
			es->Init();
			}
			}
			break;
			*/

		case EDITORTYPE_AUDIOMANAGER:
			{
				Edit_Manager *m=(Edit_Manager *)w;

				if( (!hdfile) || m->GetActiveHDFile()==hdfile)
				{
					m->RefreshHDFile(0);
					m->ShowActiveHDFile_Regions();
				}

				m->ShowAudioFiles(); // Regions +/-
			}
			break;

		case EDITORTYPE_ARRANGE:
			{
				Edit_Arrange *ar=(Edit_Arrange *)w;
				ar->RefreshAudio(hdfile);
			}
			break;
		}

		w=w->NextWindow();
	}
}

void GUI::RefreshProjectGUI()
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		switch(w->GetEditorID())
		{
		case EDITORTYPE_PLAYER:
			{
				Edit_Player *edp=(Edit_Player *)w;
				edp->ShowProjects();
				edp->ShowSongs();
			}
			break;
		}

		w=w->NextWindow();
	}

	RefreshScreenNames();
	RefreshScreenMenus();
}

void GUI::RefreshManagerGUI(AudioHDFile *hdfile,AudioRegion *r)
{	
	// Refresh GUI
	guiWindow *w=FirstWindow();

	while(w)
	{	
		if(w->GetEditorID()==EDITORTYPE_AUDIOMANAGER)	
		{
			Edit_Manager *m=(Edit_Manager *)w;

			if(m->GetActiveHDFile()==hdfile)
			{
				if(!r)
					m->RefreshHDFile(0);
				else
					m->ShowActiveHDFile_Regions();
			}
		}

		w=w->NextWindow();
	}
}

void GUI::RefreshTextGUI(Seq_Song *song)
{
	// Refresh GUI
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->WindowSong()==song)
		{
			switch(w->GetEditorID())
			{	
			case EDITORTYPE_ARRANGE:
				((Edit_Arrange *)w)->ShowTextMap();
				break;

			case EDITORTYPE_TEXT:
				((Edit_Text *)w)->ShowAllTextTexts();
				break;
			}
		}

		w=w->NextWindow();
	}
}

void GUI::RefreshTempoGUI(Seq_Song *song)
{
	// Refresh GUI
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->songmode==true && w->WindowSong()==song)
		{
			switch(w->GetEditorID())
			{
			case EDITORTYPE_ARRANGE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_DRUM:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_SCORE:
			case EDITORTYPE_TEMPO:
			case EDITORTYPE_SAMPLE:
				{
					EventEditor *ed=(EventEditor *)w;

					if(w->timeline)
					{
						if(w->timeline->format!=WINDOWDISPLAY_MEASURE || mainsettings->showbothsformatsintimeline==true)
							w->DrawHeader();
						else
							w->timeline->RecalcSamplePositions();
					}
				}
				break;
			}

			// SMPTE Display ?
			if(w->windowtimeformat!=WINDOWDISPLAY_MEASURE)
			{
				w->ShowTime();
			}

			for(int i=0;i<w->glist.gc;i++)
			{
				switch(w->glist.gadgets[i]->type)
				{
				case GADGETTYPE_TIME:
					{
						guiGadget_Time *gt=(guiGadget_Time *)w->glist.gadgets[i];

						if(gt->ttype==WINDOWDISPLAY_SMPTE || gt->ttype==WINDOWDISPLAY_SECONDS || gt->ttype==WINDOWDISPLAY_SAMPLES)
							gt->DrawGadget();
					}
					break;
				}
			}

			switch(w->GetEditorID())
			{
			case EDITORTYPE_SAMPLE:
				{
					Edit_Sample *es=(Edit_Sample *)w;
					es->DrawDBBlit(es->samples,es->overview);
				}break;

			case EDITORTYPE_EVENT: // Refresh Sample Events
			case EDITORTYPE_TEMPO:
				{
					w->RefreshObjects(0,false);
				}
				break;

			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)w;
					ar->DrawDBBlit(ar->overview,ar->pattern,ar->tempo);
				}
				break;
			}
		}

		w=w->NextWindow();
	}
}

void GUI::ClearRefresh()
{
	guiWindow *guiw=FirstWindow();

	while(guiw)
	{
		guiw->refreshed=false;
		guiw=guiw->NextWindow();
	}
}

bool GUI::RefreshAllEditorsWithPattern(Seq_Song *song,Seq_Pattern *pattern)
{
	bool found=false;

	guiWindow *guiw=FirstWindow();

	while(guiw)
	{
		if(guiw->refreshed==false && guiw->WindowSong()==song)
			switch(guiw->GetEditorID())
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)guiw;

					if(!pattern)
					{
						//guiw->refreshed=true;

						ar->ShowHoriz(false,false,false);

						//ar->ClearAllSprites();
						//ar->ShowPattern(true);
						//ar->ShowAllSprites();
					}
					else
						if(Edit_Arrange_Pattern *eap=ar->FindPattern(pattern))
						{
							//if(single==true)
							//	eap->ShowPattern_Blt();
							//else
							guiw->refreshed=true;
							ar->ShowHoriz(false,false,false);


							/*
							ar->ClearAllSprites();
							ar->ShowPattern(true);
							ar->ShowAllSprites();
							*/
						}
				}
				break;
		}

		guiw=guiw->NextWindow();
	}

	return found;
}

void GUI::RefreshAllEditorsWithEvent(Seq_Song *song,Seq_Event *e)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if(win->WindowSong()==song)
			switch(win->GetEditorID())
		{
			case EDITORTYPE_DRUM:
			case EDITORTYPE_WAVE:
			case EDITORTYPE_EVENT:
			case EDITORTYPE_PIANO:
			case EDITORTYPE_SCORE:
				{
					EventEditor_Selection *eventeditor=(EventEditor_Selection *)win;

					//if(eventeditor->refresheditorevents==false && eventeditor->patternselection.FindEvent(e))
					//	eventeditor->RefreshEvent(e);
				}
				break; 
		}

		win=win->NextWindow();
	}
}

void GUI::RefreshAllEditorsWithText(Seq_Song *song,Seq_Text *t) // t=0, all text
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if(win->WindowSong()==song)
			switch(win->GetEditorID())
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *a=(Edit_Arrange *)win;
					a->ShowTextMap();
				}
				break;

			case EDITORTYPE_TEXT:
				{
					Edit_Text *texteditor=(Edit_Text *)win;
					texteditor->ShowAllText();
				}
				break; 
		}

		win=win->NextWindow();
	}
}

void GUI::RefreshAllArrangeWithGroup(Seq_Group *group)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		switch(win->GetEditorID())
		{
		case EDITORTYPE_ARRANGE:
			{
				Edit_Arrange *a=(Edit_Arrange *)win;

				if(a->WindowSong()->GetFocusTrack() && a->WindowSong()->GetFocusTrack()->GetGroups()->FindGroup(group)==true)
				{
					//	a->trackfx.ShowGroup();
				}

				// Show Group Tracks
				// Show Track Data
				guiObject *o=a->guiobjects.FirstObject();
				while(o)
				{
					if(o->id==OBJECTID_ARRANGETRACK || o->id==OBJECTID_ARRANGEAUTOMATIONTRACK)
					{
						Edit_Arrange_Track *et=(Edit_Arrange_Track *)o;

						if(et->track->GetGroups()->FindGroup(group)==true)
							et->Refresh(REFRESH_EAT_PATTERNANDTRACK|REFRESH_EAT_PATTERN);
					}

					o=o->NextObject();
				}// while 
			}
			break;
		}

		win=win->NextWindow();
	}
}

void GUI::RefreshAllEditorsWithMarker(Seq_Song *song,Seq_Marker *t) // t=0, all text
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if(win->WindowSong()==song)
		{
			if(win->timeline)
			{
				switch(win->GetEditorID())
				{
				case EDITORTYPE_ARRANGE:
					{
						Edit_Arrange *a=(Edit_Arrange *)win;

						//if(t==0 || (t->GetMarkerEnd()>a->startposition && t->GetMarkerStart()<a->endposition))

						a->ShowHoriz(true,true,false);

						a->RefreshMarker();
					}
					break;

				case EDITORTYPE_MARKER:
					{
						Edit_Marker *m=(Edit_Marker *)win;
						m->RefreshObjects(0,false);
					}
					break; 

				case EDITORTYPE_PIANO:
					{
						Edit_Piano *p=(Edit_Piano *)win;

						if(t==0 || (t->GetMarkerEnd()>p->startposition && t->GetMarkerStart()<p->endposition))
							p->ShowPianoHoriz(SHOWEVENTS_EVENTS);

						p->RefreshMarker();
					}
					break;

				case EDITORTYPE_DRUM:
					{
						Edit_Drum *p=(Edit_Drum *)win;

						if(t==0 || (t->GetMarkerEnd()>p->startposition && t->GetMarkerStart()<p->endposition))
							p->ShowEvents();

						p->RefreshMarker();
					}
					break;

				case EDITORTYPE_WAVE:
					{
						Edit_Wave *p=(Edit_Wave *)win;

						if(t==0 || (t->GetMarkerEnd()>p->startposition && t->GetMarkerStart()<p->endposition))
							p->ShowEvents(-1);

						p->RefreshMarker();
					}
					break;

				case EDITORTYPE_EVENT:
					{
						Edit_Event *e=(Edit_Event *)win;

						if(t==0 || (t->GetMarkerEnd()>e->startposition && t->GetMarkerStart()<e->endposition))
							e->ShowEvents();

						e->RefreshMarker();
					}
					break;
				}
			}
			else
			{
				switch(win->GetEditorID())
				{
				case EDITORTYPE_MARKER:
					{
						Edit_Marker *m=(Edit_Marker *)win;
						m->RefreshObjects(0,false);
					}
					break; 

				case EDITORTYPE_EVENT:
					{
					}
					break;

				case EDITORTYPE_TRANSPORT:
					{
						Edit_Transport *t=(Edit_Transport *)win;
						t->RefreshMenu();
					}
					break;
				}
			}
		}

		win=win->NextWindow();
	}
}

void GUI::RefreshAllEditorsWithTempo(Seq_Song *song,Seq_Tempo *tempo)
{
	guiWindow *win=FirstWindow();

	while(win){

		if(win->WindowSong()==song)
		{
			switch(win->GetEditorID())
			{
			case EDITORTYPE_TEMPO:
				{
					Edit_Tempo *tempoeditor=(Edit_Tempo *)win;
					tempoeditor->RefreshObjects(0,false);
				}
				break; 
			}
		}

		win=win->NextWindow();
	}
}

void GUI::RefreshSMPTE(Seq_Project *pro)
{
	guiWindow *win=FirstWindow();

	while(win){
		win->RefreshSMPTE();
		win=win->NextWindow();
	}
}

void GUI::RefreshMeasure(Seq_Project *pro)
{
	guiWindow *win=FirstWindow();

	while(win){
		win->RefreshMeasure();
		win=win->NextWindow();
	}
}

void GUI::RefreshSignature(Seq_Song *song)
{
	RefreshAllEditors(song,0);
}

void GUI::RefreshAllHeaders(Seq_Song *song)
{
	guiWindow *win=FirstWindow();

	while(win){

		if((!song) || song==win->WindowSong())
			win->RefreshTimeLine();

		win=win->NextWindow();
	}
}

void GUI::RefreshRepairedLoopsGUI(Seq_Song *song)
{
	guiWindow *w=maingui->FirstWindow();

	while(w)
	{
		if(w->WindowSong()==song)
			switch(w->GetEditorID())
		{
			case EDITORTYPE_ARRANGE:
				{
					Edit_Arrange *ar=(Edit_Arrange *)w;
					ar->ShowHoriz(false,false,false);
				}
				break;
		}

		w=w->NextWindow();
	}
}

void GUI::RefreshAllEditors(Seq_Song *song,LONGLONG par)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if((!song) || song==win->WindowSong())
			((Editor *)win)->RefreshObjects(par,false);

		win=win->NextWindow();
	}

	if(par&REFRESHSIGNATURE_DISPLAY)
	{
		RefreshTimeSlider(song);
	}
}

void GUI::ShowToggleChildTracks(Seq_Song *song)
{
	RefreshAllEditors(song,EDITORTYPE_ARRANGE,0);
	RefreshAllEditors(song,EDITORTYPE_ARRANGELIST,0);
	RefreshAllEditors(song,EDITORTYPE_AUDIOMIXER,0);
}

void GUI::RefreshAllEditors(Seq_Song *song,int type,LONGLONG par)
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if( ((!song) || song==win->WindowSong()) && win->GetEditorID()==type)
			((Editor *)win)->RefreshObjects(par,false);

		win=win->NextWindow();
	}
}

guiWindow *GUI::FindWindow(int type,void *p1,void *p2)
{
	guiWindow *f=FirstWindow();

	while(f)
	{
		if(f->GetEditorID()==type)
			switch(type)
		{
			case EDITORTYPE_GROUP:
				{
					if(f->WindowSong()==(Seq_Song *)p1)
						return f;
				}
				break;

			case EDITORTYPE_SETTINGS:
				{
					Seq_Song *s=(Seq_Song *)p1;

					if(f->WindowSong()==s)
						return f;
				}
				break;

			case EDITORTYPE_MIDIFILTER:
				{
					Edit_MIDIFilter *mf=(Edit_MIDIFilter *)f;

					if(mf->filter==(MIDIFilter *)p1)
						return f;
				}
				break;

			case EDITORTYPE_AUDIOMASTER:
				{
					Seq_Song *ws=(Seq_Song *)p1;

					if(ws==f->song)
						return f;
				}
				break;

			case EDITORTYPE_SAMPLE:
				{
					Edit_Sample *es=(Edit_Sample *)f;
					AudioHDFile *caf=(AudioHDFile *)p1;

					if(es->audiohdfile==caf)
						return f;
				}
				break;

			default:
				return f;
		}

		f=f->NextWindow();
	}

	return 0;
}

guiWindow *GUI::GetEditorWindow(Seq_Song *song,int type)
{
	guiWindow *f=FirstWindow();

	while(f){

		if(f->GetEditorID()==type && f->WindowSong()==song)
			return f;

		f=f->NextWindow();
	}

	return 0;
}

void GUI::LeftMouseButtonUp(guiWindow *win)
{
	win->ResetRefresh();

	win->left_mousekey=MOUSEKEY_UP;

	win->autoscroll=false;
	win->autoscrollmode=0;
	ReleaseCapture();

	if(win->ignoreleftmouse==false)
	{
		CheckMouseUp(win,MOUSEKEY_LEFT_UP);
		win->ReleaseMouse();
	}
	else
		win->ignoreleftmouse=false;
}

void GUI::RightMouseButtonUp(guiWindow *win)
{
	if(win->left_mousekey==MOUSEKEY_DOWN)
		win->ignoreleftmouse=true;
	else
		win->ignoreleftmouse=false;

	win->ResetRefresh();

	win->right_mousekey=MOUSEKEY_UP;
	win->autoscroll=false;
	win->autoscrollmode=0;

	ReleaseCapture();

	CheckMouseUp(win,MOUSEKEY_RIGHT_UP);

	win->ReleaseMouse();
}

void GUI::LeftMouseButtonDown(guiWindow *win)
{
	win->ResetRefresh();
	win->left_mousekey=MOUSEKEY_DOWN;

	SetFocus(win->hWnd);
	SetCapture(win->hWnd);

	if(win->ignoreleftmouse==false)
		CheckMouseDown(win,MOUSEKEY_LEFT_DOWN);
	else
		win->ignoreleftmouse=false;
}

void GUI::RightMouseButtonDown(guiWindow *win)
{
	if(win->left_mousekey==MOUSEKEY_DOWN)
		win->ignoreleftmouse=true;
	else
		win->ignoreleftmouse=false;

	SetFocus(win->hWnd);
	SetCapture(win->hWnd);

	win->ResetRefresh();

	//win->left_mousekey=MOUSEKEY_UP;
	win->right_mousekey=MOUSEKEY_DOWN;


	//SetCapture(win->hWnd);

	CheckMouseDown(win,MOUSEKEY_RIGHT_DOWN);
}

GUI::GUI()
{
	dragdropwindow=0;
	autoloadsongs[0]=autoloadsongs[1]=0;
	infoWindow=0;
	activescreen=0;
	allwindowsoondesktop=false;
	dragdropobject=0;
	refreshprogress=false;
	progressstring=0;
	rrt_slowcounter=0;
	rrt_MIDIdevicescheck=0;
	firstfreecorewindow=0;
}

bool GUI::CheckIfKeyDown(int key)
{
#ifdef WIN32

	USHORT vkey=VkKeyScan(key);
	//UINT vkey=MapVirtualKey(key,MAPVK_VSC_TO_VK);
	SHORT status=GetKeyState(vkey);

	if(status<0) // - key down
		return true;

	// TRACE ("Keystatus %c = vkey %d = status %d \n",key,vkey,status);
#endif
	return false;
}

void GUI::DeleteAllWindows()
{
	guiWindow *win=FirstWindow();

	while(win)
	{
		if(win->GetEditorID()!=EDITORTYPE_TRANSPORT)
		{
			CloseWindow(win);
			win=FirstWindow();
		}
		else
			win=win->NextWindow();
	}
}

void GUI::DeleteAllWindowsButThis()
{
	/*
	guiWindow *win=FirstWindow();

	while(win)
	{
	if(win->activemdiwindow==true)
	win=win->NextWindow();
	else
	switch(win->GetEditorID())
	{
	case EDITORTYPE_AUDIOMANAGER:
	case EDITORTYPE_TOOLBOX:
	case EDITORTYPE_TRANSPORT:
	win=win->NextWindow();
	break;

	default:
	CloseWindow(win);
	win=FirstWindow();
	break;
	}
	}
	*/
}

void GUI::DeleteAllWindowsButActiveProject()
{
	// Change DeleteAllWindowsButActiveSong! also
	guiWindow *win=FirstWindow();

	while(win)
	{
		switch(win->GetEditorID())
		{
		case EDITORTYPE_AUDIOMANAGER:
		case EDITORTYPE_TOOLBOX:
		case EDITORTYPE_PLAYER:
		case EDITORTYPE_MONITOR:
		case EDITORTYPE_KEYBOARD:
		case EDITORTYPE_LIBRARY:
		case EDITORTYPE_BIGTIME:
			win=win->NextWindow();
			break;

		default:
			if((win->WindowSong()==0 || win->WindowSong()->project!=mainvar->GetActiveProject()) && win->GetEditorID()!=EDITORTYPE_TRANSPORT)
			{
				CloseWindow(win);
				win=FirstWindow();
			}
			else
				win=win->NextWindow();
			break;
		}
	}
}

void GUI::DeleteAllWindowsButActiveSong()
{
	guiWindow *win=FirstWindow();

	// Change DeleteAllWindowsButActiveProject! also
	while(win)
	{
		switch(win->GetEditorID())
		{
		case EDITORTYPE_AUDIOMANAGER:
		case EDITORTYPE_TOOLBOX:
		case EDITORTYPE_PLAYER:
		case EDITORTYPE_MONITOR:
		case EDITORTYPE_KEYBOARD:
		case EDITORTYPE_LIBRARY:
		case EDITORTYPE_BIGTIME:
			win=win->NextWindow();
			break;

		default:
			if((mainvar->GetActiveSong()==0 || win->WindowSong()!=mainvar->GetActiveSong()) && win->GetEditorID()!=EDITORTYPE_TRANSPORT)
			{
				CloseWindow(win);
				win=FirstWindow();
			}
			else
				win=win->NextWindow();
			break;
		}
	}
}

void GUI::NewEffectInsert(AudioEffects *effects,InsertAudioEffect *iae,guiWindow *win)
{
#ifdef OLDIE

	RefreshEffects(effects);

	/*
	if(gadget)
	{	
	guiWindowSetting setting;

	setting.startposition_x=win->win_screenposx+gadget->x2;
	setting.startposition_y=win->win_screenposy+gadget->y2;

	iae->audioeffect->OpenGUI(win->WindowSong(),iae,&setting);
	}
	else
	*/
	iae->audioeffect->OpenGUI(win->WindowSong(),iae);

#endif

}

void GUI::AddOnOffMenu(guiMenu *menu,InsertAudioEffect *oldeffect)
{
	if(oldeffect->audioeffect)
	{
		class menu_bypasstoggle:public guiMenu
		{
		public:
			menu_bypasstoggle(InsertAudioEffect *e){insertaudioeffect=e;}

			void MenuFunction()
			{
				insertaudioeffect->audioeffect->User_TogglePluginBypass();
			}

			InsertAudioEffect *insertaudioeffect;
		};

		menu->AddFMenu("Bypass",new menu_bypasstoggle(oldeffect),oldeffect->audioeffect->plugin_bypass);

		class menu_onofftoggle:public guiMenu
		{
		public:
			menu_onofftoggle(InsertAudioEffect *e){effect=e;}

			void MenuFunction()
			{
				effect->audioeffect->OnOff(effect->audioeffect->plugin_on==false?true:false);
				//maingui->RefreshBypass(effect);
			} //

			InsertAudioEffect *effect;
		};

		menu->AddFMenu(Cxs[CXS_ON],new menu_onofftoggle(oldeffect),oldeffect->audioeffect->plugin_on);

		menu->AddLine();
	}
}

void GUI::CreateInstrumentPopMenu(AudioEffects *fx,guiWindow *w,InsertAudioEffect *oldeffect,bool addtopopmenu)
{
	if((!oldeffect) && addtopopmenu==false)
		w->DeletePopUpMenu(true);

	if(w->popmenu)
	{	
		/*
		if(oldeffect)
		{
		popmenu->AddSelectedMenu(oldeffect->audioeffect->GetEffectName(),0);

		popmenu->AddFMenu("Edit Instrument",new menu_editeff(oldeffect));

		popmenu->AddLine();
		popmenu->AddFMenu("No Instrument",new menu_noeffect(WindowSong(),chl->channel,oldeffect)); // 1==no effect

		// Bypass
		AddOnOffMenu(oldeffect);

		popmenu->AddLine();
		}
		*/

		// Effects
		//	s=popmenu->AddMenu("Intern Instruments",c);

		//	popmenu->AddLine();

		VSTPlugin *vst=mainaudio->FirstVSTInstrument();

		if(vst)
		{
			char hx[NUMBERSTRINGLEN],hx2[NUMBERSTRINGLEN],*lastcompanyname=0;
			guiMenu *companymenu=0;

			while(vst){

				if(vst->IsActive()==true && vst->GetOutputPins()>0 && vst->GetCompany() && strlen(vst->GetCompany())>0)
				{
					if((!companymenu) || strcmp(lastcompanyname,vst->GetCompany())!=0)
					{
						lastcompanyname=vst->GetCompany();
						if(char *h=mainvar->GenerateString("VSTi"," ",vst->GetCompany()))
						{
							companymenu=w->popmenu->AddMenu(h,0);
							delete h;
						}
					}

					if(companymenu)
					{
						// I/O
						char *h2=mainvar->GenerateString(vst->GetEffectName(),":",mainvar->ConvertIntToChar(vst->GetInputPins(),hx),
							"/",mainvar->ConvertIntToChar(vst->GetOutputPins(),hx2));

						if(h2)
						{
							companymenu->AddFMenu(h2,new tmenu_effect(fx,oldeffect,vst,w),(oldeffect && oldeffect->audioeffect->fromobject==vst)?true:false);
							delete h2;
						}
					}
				}

				vst=vst->NextVSTPlugin();
			}

			VSTPlugin *vst=mainaudio->FirstVSTInstrument();
			companymenu=0;

			while(vst){

				if(vst->IsActive()==true && vst->GetOutputPins()>0 && ((!vst->GetCompany()) || strlen(vst->GetCompany())==0))
				{
					if(!companymenu)
					{
						if(char *h=mainvar->GenerateString("VSTi..."))
						{
							companymenu=w->popmenu->AddMenu(h,0);
							delete h;
						}
					}

					if(companymenu)
					{
						// I/O
						char *h2=mainvar->GenerateString(vst->GetEffectName(),":",mainvar->ConvertIntToChar(vst->GetInputPins(),hx),
							"/",mainvar->ConvertIntToChar(vst->GetOutputPins(),hx2));

						if(h2)
						{
							companymenu->AddFMenu(h2,new tmenu_effect(fx,oldeffect,vst,w),(oldeffect && oldeffect->audioeffect->fromobject==vst)?true:false);
							delete h2;
						}
					}
				}

				vst=vst->NextVSTPlugin();
			}
		}

		// Paste Effect ?
		if(
			(mainbuffer->CheckHeadBuffer(OBJ_AUDIOINTERN)==true || mainbuffer->CheckHeadBuffer(OBJ_AUDIOVST)==true) &&
			mainbuffer->CheckSubID(audioobject_TYPE_INSTRUMENT)==true
			)
		{	
			class menu_pasteeff:public guiMenu
			{
			public:
				menu_pasteeff(guiWindow *w,AudioEffects *f,InsertAudioEffect *e)
				{
					window=w;
					effects=f;
					effect=e;
				}

				void MenuFunction()
				{
					if(window)
						mainbuffer->PasteBufferToEffect(window->WindowSong(),effects,effect);
				} //

				AudioEffects *effects;
				InsertAudioEffect *effect;
				guiWindow *window;
			};

			w->popmenu->AddLine();
			w->popmenu->AddFMenu(Cxs[CXS_PASTE],new menu_pasteeff(w,fx,oldeffect));
			w->popmenu->AddLine();
		}

		//if(!oldeffect)
		//	w->ShowPopMenu();
	}
}

void GUI::CreateEffectListPopUp(AudioEffects *efx,guiWindow *w,char *header)
{
	if(!w)return;

	AudioIOFX *io=efx->io;

	w->DeletePopUpMenu(true);

	if(w->popmenu)
	{
		if(header)
		{
			w->popmenu->AddMenu(header,0);
			w->popmenu->AddLine();
		}

		if(efx->FirstInsertAudioEffect())
		{
			// Copy
			class menu_copyalleffectstobuffer:public guiMenu
			{
			public:
				menu_copyalleffectstobuffer(AudioEffects *f)
				{
					effects=f;
				}

				void MenuFunction()
				{
					mainbuffer->CopyEffectList(effects);
				} //

				AudioEffects *effects;
			};

			w->popmenu->AddFMenu(Cxs[CXS_COPYINSTRUMENTSANDEFFECT],new menu_copyalleffectstobuffer(efx));

			// Delete
			class menu_deletealleffects:public guiMenu
			{
			public:
				menu_deletealleffects(AudioEffects *f)
				{
					effects=f;
				}

				void MenuFunction()
				{
					mainedit->DeletePlugins(effects);

					/*
					bool refresh=effects->DeleteEffectsFlag(AudioEffects::DELETE_FX);

					if(refresh==true)
					maingui->RefreshEffects(effects);
					*/

				} //

				AudioEffects *effects;
			};

			w->popmenu->AddFMenu(Cxs[CXS_DELETEALLIE],new menu_deletealleffects(efx));
		}

		if(mainbuffer->CheckHeadBuffer(OBJ_EFFECTLIST)==true)
		{
			// Copy/Delete
			class menu_pastebuffer:public guiMenu
			{
			public:
				menu_pastebuffer(AudioEffects *f)
				{
					effects=f;
				}

				void MenuFunction()
				{
					mainbuffer->PasteBufferToEffectList(effects);
				} //

				AudioEffects *effects;
			};

			w->popmenu->AddFMenu(Cxs[CXS_PASTINSTRUMENTSANDEFFECT],new menu_pastebuffer(efx));
		}

		w->ShowPopMenu();
	}
}

void GUI::CreateEffectPopUp(AudioEffects *efx,guiWindow *w,InsertAudioEffect *oldeffect)
{
	// 0 name
	// 1 edit
	// 2 Copy
	// 3 Paste
	// 4 Effect off
	// 5 Bypass

	if(!w)return;

	AudioIOFX *io=efx->io;

	w->DeletePopUpMenu(true);

	if(w->popmenu)
	{
		// Paste Effect ?
		if(
			(mainbuffer->CheckHeadBuffer(OBJ_AUDIOINTERN)==true || mainbuffer->CheckHeadBuffer(OBJ_AUDIOVST)==true) &&
			(mainbuffer->CheckSubID(audioobject_TYPE_EFFECT)==true || mainbuffer->CheckSubID(audioobject_TYPE_INSTRUMENT)==true)
			)
		{	
			class menu_pasteeff:public guiMenu
			{
			public:
				menu_pasteeff(guiWindow *w,AudioEffects *f,InsertAudioEffect *e)
				{
					window=w;
					effects=f;
					effect=e;
				}

				void MenuFunction()
				{
					if(window)
						mainbuffer->PasteBufferToEffect(window->WindowSong(),effects,effect);
				} //

				AudioEffects *effects;
				InsertAudioEffect *effect;
				guiWindow *window;
			};

			w->popmenu->AddFMenu(Cxs[CXS_PASTE],new menu_pasteeff(w,efx,oldeffect));

			w->popmenu->AddLine();
		}

		if(oldeffect) // old effect
		{
			if(oldeffect->audioeffect)
			{
				char *h=0;

				char *programname=oldeffect->audioeffect->GetProgramName();

				if(!programname)
					programname=mainvar->GenerateString("-?-");

				switch(oldeffect->audioeffect->audioeffecttype)
				{
				case audioobject_TYPE_EFFECT:
					h=mainvar->GenerateString("Effect:",oldeffect->audioeffect->GetEffectName()," (",programname,")");
					break;

				case audioobject_TYPE_INSTRUMENT:
					h=mainvar->GenerateString("Instrument:",oldeffect->audioeffect->GetEffectName()," (",programname,")");
					break;
				}

				if(programname)
					delete programname;

				if(h)
				{
					char hx[NUMBERSTRINGLEN],hx2[NUMBERSTRINGLEN];

					// I/O
					char *h2=mainvar->GenerateString(h," In:",mainvar->ConvertIntToChar(oldeffect->audioeffect->GetInputPins(),hx),
						" Out:",mainvar->ConvertIntToChar(oldeffect->audioeffect->GetOutputPins(),hx2));

					if(h2)
					{
						//w->popmenu->AddMenu(h2,0); // 0== name

						w->popmenu->AddFMenu(h2,new menu_editeff(w->WindowSong(),oldeffect));

						delete h2;
						w->popmenu->AddLine();
					}

					delete h;
				}

				w->popmenu->AddLine();
				w->popmenu->AddFMenu(Cxs[CXS_EDITEFFECT],new menu_editeff(w->WindowSong(),oldeffect));

				if(oldeffect->audioeffect->numberofprograms>1)
				{
					class menu_selectpluginprogram:public guiMenu
					{
					public:
						menu_selectpluginprogram(guiWindow *w,InsertAudioEffect *i){win=w;iae=i;}

						void MenuFunction()
						{
							mainaudio->SelectPluginProgram(win,iae);
						} //

						guiWindow *win;
						InsertAudioEffect *iae;
					};

					if(oldeffect->audioeffect->numberofprograms>1 && oldeffect->audioeffect->CanGetProgramNameIndex()==true)
						w->popmenu->AddFMenu(Cxs[CXS_SELECTPLUGINPROGRAM],new menu_selectpluginprogram(w,oldeffect));
				}

				w->popmenu->AddLine();

				if(io->audioeffects.track)
				{
					class menu_settrackname:public guiMenu
					{
					public:
						menu_settrackname(AudioEffects *f,AudioObject *e){afx=f;effect=e;}

						void MenuFunction()
						{
							afx->SetTrackName(effect);
						} //

						AudioEffects *afx;
						AudioObject *effect;
					};

					w->popmenu->AddFMenu(Cxs[CXS_SETTRACKNAMEASPROGRAMNAME],new menu_settrackname(efx,oldeffect->audioeffect),oldeffect->audioeffect->settrackname);
				}

				if(oldeffect->PrevEffect())w->popmenu->AddFMenu(Cxs[CXS_MOVEUP],new tmenu_moveup(oldeffect));
				if(oldeffect->NextEffect())w->popmenu->AddFMenu(Cxs[CXS_MOVEDOWN],new tmenu_movedown(oldeffect));
			}
			else
				w->popmenu->AddMenu("??? Effect",0,true); // 0== name

			w->popmenu->AddLine();

			if(oldeffect->audioeffect)
			{
				class menu_copyeff:public guiMenu
				{
				public:
					menu_copyeff(InsertAudioEffect *e){effect=e;}

					void MenuFunction()
					{
						mainbuffer->OpenBuffer();
						mainbuffer->AddObjectToBuffer(effect->audioeffect,true);
						mainbuffer->CloseBuffer();
					} //

					InsertAudioEffect *effect;
				};

				w->popmenu->AddFMenu(Cxs[oldeffect->audioeffect->audioeffecttype==audioobject_TYPE_EFFECT?CXS_COPYEFFECT:CXS_COPYINSTRUMENT],new menu_copyeff(oldeffect));
			}

			w->popmenu->AddFMenu(Cxs[CXS_DELETEPLUGIN],new tmenu_deleteplugin(oldeffect));

			w->popmenu->AddLine();

			// Bypass
			AddOnOffMenu(w->popmenu,oldeffect);
		}

		/*
		if(oldeffect && oldeffect->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)
		{
		// Instrument <-> Instrument
		maingui->CreateInstrumentPopMenu(efx,w,g,oldeffect);
		}
		else
		*/
		{
			//char hx[NUMBERSTRINGLEN],hx2[NUMBERSTRINGLEN];

			// Effects
			if(mainaudio->FirstInternEffect())
			{
				guiMenu *s=0;

				audioobject_Intern *fi=mainaudio->FirstInternEffect();

				while(fi)
				{
					if(fi->IsActive()==true)
					{
						if(!s)
							s=w->popmenu->AddMenu("Intern",0);

						if(fi->GetOutputPins()>=io->GetChannels())
						{
							// I/O
							if(s)
								s->AddFMenu(fi->GetName(),new tmenu_effect(efx,oldeffect,fi,w));
						}
					}

					fi=fi->NextInternEffect();
				}

				w->popmenu->AddLine();
			}

			VSTPlugin *vst=mainaudio->FirstVSTEffect();

			if(vst)
			{
				char hx[NUMBERSTRINGLEN],hx2[NUMBERSTRINGLEN],*lastcompanyname=0;
				guiMenu *companymenu=0;

				while(vst){

					if(vst->IsActive()==true && vst->GetOutputPins()>0 && vst->GetCompany() && strlen(vst->GetCompany())>0)
					{
						if((!companymenu) || strcmp(lastcompanyname,vst->GetCompany())!=0)
						{
							lastcompanyname=vst->GetCompany();
							if(char *h=mainvar->GenerateString("VSTx"," ",vst->GetCompany()))
							{
								TRACE ("%s\n",h);
								companymenu=w->popmenu->AddMenu(h,0);
								delete h;
							}
						}

						if(companymenu)
						{
							// I/O
							char *h2=mainvar->GenerateString(vst->GetEffectName(),":",mainvar->ConvertIntToChar(vst->GetInputPins(),hx),
								"/",mainvar->ConvertIntToChar(vst->GetOutputPins(),hx2));

							if(h2)
							{
								companymenu->AddFMenu(h2,new tmenu_effect(efx,oldeffect,vst,w),(oldeffect && oldeffect->audioeffect->fromobject==vst)?true:false);
								delete h2;
							}
						}
					}

					vst=vst->NextVSTPlugin();
				}

				VSTPlugin *vst=mainaudio->FirstVSTEffect();
				companymenu=0;

				while(vst){

					if(vst->IsActive()==true && vst->GetOutputPins()>0 && ((!vst->GetCompany()) || strlen(vst->GetCompany())==0))
					{
						if(!companymenu)
						{
							if(char *h=mainvar->GenerateString("VSTx..."))
							{
								companymenu=w->popmenu->AddMenu(h,0);
								delete h;
							}
						}

						if(companymenu)
						{
							// I/O
							char *h2=mainvar->GenerateString(vst->GetEffectName(),":",mainvar->ConvertIntToChar(vst->GetInputPins(),hx),
								"/",mainvar->ConvertIntToChar(vst->GetOutputPins(),hx2));

							if(h2)
							{
								companymenu->AddFMenu(h2,new tmenu_effect(efx,oldeffect,vst,w),(oldeffect && oldeffect->audioeffect->fromobject==vst)?true:false);
								delete h2;
							}
						}
					}

					vst=vst->NextVSTPlugin();
				}
			}
			w->popmenu->AddLine();

			CreateInstrumentPopMenu(efx,w,oldeffect,true);
		}

		w->ShowPopMenu();
	}
}

int GUI::GetVersion()
{
	return CAMXVERSION;
}

bool GUI::GetShiftKey()
{
	SHORT hiBit = (1 << 15); //Test ob Taste gerade gedrückt ist oder nicht;
	SHORT r=GetKeyState(VK_LSHIFT);

	//	SHORT lowBit = 1;   //Toggle state prüfen

	/*
	SHORT wert = GetKeyState(<Taste> );
	if (wert & lowBit) TRACE("Taste Toggle ON\n");
	else TRACE("Taste Toggle OFF\n");
	*/

	if (r & hiBit)
		return true;

	r=GetKeyState(VK_RSHIFT);

	//	SHORT lowBit = 1;   //Toggle state prüfen


	/*
	SHORT wert = GetKeyState(<Taste> );
	if (wert & lowBit) TRACE("Taste Toggle ON\n");
	else TRACE("Taste Toggle OFF\n");
	*/

	if (r & hiBit)
		return true;

	return false;
}

void GUI::GetMouseOnScreen(int *x,int *y)
{
	POINT lpPoint;
	GetCursorPos(&lpPoint);

	*x=lpPoint.x;
	*y=lpPoint.y;
}

bool GUI::GetCtrlKey()
{
	SHORT hiBit = (1 << 15); //Test ob Taste gerade gedrückt ist oder nicht;
	SHORT r=GetKeyState(VK_LCONTROL);

	//	SHORT lowBit = 1;   //Toggle state prüfen


	/*
	SHORT wert = GetKeyState(<Taste> );
	if (wert & lowBit) TRACE("Taste Toggle ON\n");
	else TRACE("Taste Toggle OFF\n");
	*/

	if (r & hiBit)
		return true;

	r=GetKeyState(VK_RCONTROL);

	//	SHORT lowBit = 1;   //Toggle state prüfen


	/*
	SHORT wert = GetKeyState(<Taste> );
	if (wert & lowBit) TRACE("Taste Toggle ON\n");
	else TRACE("Taste Toggle OFF\n");
	*/

	if (r & hiBit)
		return true;

	return false;
}

bool GUI::GetLeftMouseButton()
{
	SHORT hiBit = (1 << 15); //Test ob Taste gerade gedrückt ist oder nicht;
	SHORT r=GetKeyState(VK_LBUTTON);

	//	SHORT lowBit = 1;   //Toggle state prüfen

	/*
	SHORT wert = GetKeyState(<Taste> );
	if (wert & lowBit) TRACE("Taste Toggle ON\n");
	else TRACE("Taste Toggle OFF\n");
	*/

	if (r & hiBit)
		return true;

	return false;
}

bool GUI::GetRightMouseButton()
{
	SHORT hiBit = (1 << 15); //Test ob Taste gerade gedrückt ist oder nicht;
	SHORT r=GetKeyState(VK_RBUTTON);

	//	SHORT lowBit = 1;   //Toggle state prüfen


	/*
	SHORT wert = GetKeyState(<Taste> );
	if (wert & lowBit) TRACE("Taste Toggle ON\n");
	else TRACE("Taste Toggle OFF\n");
	*/

	if (r & hiBit)
		return true;

	return false;
}

bool GUI::CheckIfInRange(int mx,int my,int rangex,int rangex2,int rangey,int rangey2)
{
	if(mx>=rangex && mx<=rangex2 && my>=rangey && my<=rangey2)
		return true;

	return false;
}

void GUI::EditTrackMIDIOutput(guiWindow *w,Seq_Track *track)
{
	w->DeletePopUpMenu(true);

	char nr[NUMBERSTRINGLEN];

	if(w->popmenu)
	{	
		if(track->song)
		{
			if(char *h=mainvar->GenerateString(Cxs[CXS_MIDIOUTPUT]," Track:",track->GetName()))
			{
				w->popmenu->AddMenu(h,0);
				w->popmenu->AddLine();
				delete h;
			}
		}

		if(track->song && track->ismetrotrack==false && (track->PrevTrack() || track->NextTrack()))
		{
			class menu_setMIDIout:public guiMenu
			{
			public:
				menu_setMIDIout(Seq_Track *t,bool sel){track=t;selected=sel;}

				void MenuFunction()
				{
					mainedit->SetSongTracksToMIDIOutput(track,selected);
				}

				Seq_Track *track;
				bool selected;
			};

			if(track->song->CheckForOtherMIDI(track,true,true)==true)
				w->popmenu->AddFMenu(Cxs[CXS_CHANGEALLTRACKSMIDIOUT_SEL],new menu_setMIDIout(track,true));

			if(track->song->CheckForOtherMIDI(track,true,false)==true)
				w->popmenu->AddFMenu(Cxs[CXS_CHANGEALLTRACKSMIDIOUT],new menu_setMIDIout(track,false));

			w->popmenu->AddLine();
		}

		{
			Seq_Group_MIDIOutPointer *sgp=track->GetMIDIOut()->FirstDevice();

			if(sgp)
			{
				class menu_noMIDI:public guiMenu
				{
				public:
					menu_noMIDI(Seq_Track *t,int pindex){track=t;portindex=pindex;}

					void MenuFunction()
					{
						mainedit->RemoveMIDIOutput(track,portindex);
					}

					Seq_Track *track;
					int portindex;
				};

				while (sgp)
				{
					if(char *h=mainvar->GenerateString("<+> ","P",mainvar->ConvertIntToChar(sgp->portindex+1,nr),":",mainMIDI->MIDIoutports[sgp->portindex].GetName()))
					{
						guiMenu *sub=w->popmenu->AddFMenu(h,new menu_noMIDI(track,sgp->portindex));

						delete h;

						if(sub)
						{
							// Delete

							if(char *h=mainvar->GenerateString(Cxs[CXS_DELETE],"<>"))
							{
								sub->AddFMenu(h,new menu_noMIDI(track,sgp->portindex));
								delete h;
							}

							sub->AddLine();

							class menu_replacegroup:public guiMenu
							{
							public:
								menu_replacegroup(Seq_Track *t,int o,int n)
								{
									track=t;
									oldindex=o;
									newindex=n;
								}

								void MenuFunction()
								{
									mainedit->ReplaceMIDIOutput(track,oldindex,newindex);
								}

								Seq_Track *track;
								int oldindex,newindex;
							};

							// Replace
							for(int i=0;i<MAXMIDIPORTS;i++)
							{
								if(mainMIDI->MIDIoutports[i].visible==true && track->GetMIDIOut()->FindPort(i)==false)
								{
									if(h=mainvar->GenerateString(Cxs[CXS_REPLACEWITH]," P",mainvar->ConvertIntToChar(i+1,nr),":",mainMIDI->MIDIoutports[i].GetName()))
									{
										sub->AddFMenu(h,new menu_replacegroup(track,sgp->portindex,i));
										delete h;
									}
								}
							}
						}
					}

					sgp=sgp->NextGroup();
				}
			}
		}

		// Add
		guiMenu *sub=0;

		class menu_addgroup:public guiMenu
		{
		public:
			menu_addgroup(Seq_Track *t,int index)
			{
				track=t;
				portindex=index;
			}

			void MenuFunction()
			{
				mainedit->AddMIDIOutput(track,portindex);
			}

			Seq_Track *track;
			int portindex;
		};

		for(int i=0;i<MAXMIDIPORTS;i++)
		{
			if(mainMIDI->MIDIoutports[i].visible==true && track->GetMIDIOut()->FindPort(i)==false)
			{
				if(sub==0)
				{
					w->popmenu->AddLine();
					sub=w->popmenu->AddMenu(Cxs[CXS_CONNECTWITH],0);
				}

				if(sub)
				{
					if(char *h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(i+1,nr),":",mainMIDI->MIDIoutports[i].GetName()))
					{
						sub->AddFMenu(h,new menu_addgroup(track,i));
						delete h;
					}
				}
			}
		}

		w->ShowPopMenu();
	}
}

void GUI::EditTrackMIDIInput(guiWindow *w,Seq_Track *track)
{
	if(!track)return;

	if(w->DeletePopUpMenu(true))
	{
		if(track->song)
		{
			char h2[NUMBERSTRINGLEN];
			char *tracknr=mainvar->ConvertIntToChar(track->song->GetOfTrack(track)+1,h2);

			if(char *h=mainvar->GenerateString(Cxs[CXS_MIDIINPUT],"Track ",tracknr,":",track->GetName()))
			{
				w->popmenu->AddMenu(h,0);
				w->popmenu->AddLine();
				delete h;
			}
		}

		class menu_MIDIchlsel:public guiMenu
		{
		public:
			menu_MIDIchlsel(Seq_Track *t,MIDIInputDevice *i){track=t,device=i;}

			void MenuFunction()
			{
				if(track->indevice!=device || track->GetFX()->noMIDIinput==true)
				{
					MIDIinproc->Lock();
					track->GetFX()->noMIDIinput=false;
					track->indevice=device;
					MIDIinproc->Unlock();

					maingui->RefreshMIDI(track);
				}
			} //

			Seq_Track *track;
			MIDIInputDevice *device;
		};

		class menu_usealldevices:public guiMenu
		{
		public:
			menu_usealldevices(Seq_Track *t){track=t;}

			void MenuFunction()
			{
				mainedit->SetMIDIInputAllDevices(track,track->GetFX()->useallinputdevices==true?false:true);

			} //

			Seq_Track *track;
		};
		w->popmenu->AddFMenu(Cxs[CXS_RECEIVEMIDIINFROMALLDEV],new menu_usealldevices(track),track->GetFX()->useallinputdevices);

		class menu_userotuing:public guiMenu
		{
		public:
			menu_userotuing(Seq_Track *t){track=t;}

			void MenuFunction()
			{
				if(track->GetFX()->userouting==true)
					track->GetFX()->userouting=false;
				else
					track->GetFX()->userouting=true;

				maingui->RefreshMIDI(track);
			} //

			Seq_Track *track;
		};
		w->popmenu->AddFMenu(Cxs[CXS_SONGROUTING],new menu_userotuing(track),track->GetFX()->userouting);

		class menu_usethrualways:public guiMenu
		{
		public:
			menu_usethrualways(Seq_Track *t){track=t;}

			void MenuFunction()
			{
				track->GetFX()->usealwaysthru=track->GetFX()->usealwaysthru==true?false:true;
				track->GetFX()->setalwaysthruautomatic=false;

				maingui->RefreshMIDI(track);
			} //

			Seq_Track *track;
		};

		w->popmenu->AddFMenu(Cxs[CXS_MIDITHRUACTIVE],new menu_usethrualways(track),track->GetFX()->usealwaysthru);

		class menu_noMIDIinput:public guiMenu
		{
		public:
			menu_noMIDIinput(Seq_Track *t){track=t;}

			void MenuFunction()
			{
				mainthreadcontrol->LockActiveSong();

				if(track->GetFX()->noMIDIinput==true)
					track->GetFX()->noMIDIinput=false;
				else
					track->GetFX()->noMIDIinput=true;

				mainthreadcontrol->UnlockActiveSong();

				maingui->RefreshMIDI(track);
			} //

			Seq_Track *track;
		};

		w->popmenu->AddFMenu(Cxs[CXS_NOMIDIINPUT],new menu_noMIDIinput(track),track->GetFX()->noMIDIinput);

		if(track->song && track->ismetrotrack==false && (track->PrevTrack() || track->NextTrack()))
		{
			class menu_setinout:public guiMenu
			{
			public:
				menu_setinout(Seq_Track *t,bool sel){track=t;selected=sel;}

				void MenuFunction()
				{
					mainedit->SetSongTracksToMIDIInput(track,selected);
				}

				Seq_Track *track;
				bool selected;
			};

			if(track->song->CheckForOtherMIDI(track,false,true)==true)
				w->popmenu->AddFMenu(Cxs[CXS_CHANGEALLTRACKSMIDIIN_SEL],new menu_setinout(track,true));

			if(track->song->CheckForOtherMIDI(track,false,false)==true)
				w->popmenu->AddFMenu(Cxs[CXS_CHANGEALLTRACKSMIDIIN],new menu_setinout(track,false));
		}


		if(Seq_Group_MIDIInPointer *sgp=track->GetMIDIIn()->FirstDevice())
		{
			w->popmenu->AddLine();

			class menu_noMIDI:public guiMenu
			{
			public:
				menu_noMIDI(Seq_Track *t,MIDIInputDevice *g){track=t;device=g;}

				void MenuFunction()
				{
					mainedit->RemoveMIDIInput(track,device);
				}

				Seq_Track *track;
				MIDIInputDevice *device;
			};

			char nr[NUMBERSTRINGLEN];

			while (sgp)
			{
				if(char *h=mainvar->GenerateString("<+> P",mainvar->ConvertIntToChar(sgp->portindex+1,nr),":",mainMIDI->MIDIinports[sgp->portindex].GetName()))
				{
					guiMenu *sub=w->popmenu->AddFMenu(h,new menu_noMIDI(track,sgp->GetDevice()));
					delete h;

					if(sub)
					{
						// Delete
						if(char *h=mainvar->GenerateString(Cxs[CXS_DELETE],"<>"))
						{
							sub->AddFMenu(h,new menu_noMIDI(track,sgp->GetDevice()));
							delete h;
						}

						sub->AddLine();

						// Replace
						for(int i=0;i<MAXMIDIPORTS;i++)
						{
							if(mainMIDI->MIDIinports[i].visible==true && track->GetMIDIIn()->FindPort(i)==false)
							{
								class menu_replacegroup:public guiMenu
								{
								public:
									menu_replacegroup(Seq_Track *t,int o,int n)
									{
										track=t;
										oldindex=o;
										newindex=n;
									}

									void MenuFunction()
									{
										mainedit->ReplaceMIDIInput(track,oldindex,newindex);
									}

									Seq_Track *track;
									int oldindex,newindex;
								};

								if(h=mainvar->GenerateString(Cxs[CXS_REPLACEWITH]," P",mainvar->ConvertIntToChar(i+1,nr),":",mainMIDI->MIDIinports[i].GetName()))
								{
									sub->AddFMenu(h,new menu_replacegroup(track,sgp->portindex,i));
									delete h;
								}

							}
						}
					}
				}

				sgp=sgp->NextGroup();
			}
		}

		// Add
		if(mainMIDI->FirstMIDIInputDevice())
		{
			guiMenu *sub=0;

			for(int i=0;i<MAXMIDIPORTS;i++)
			{
				if(mainMIDI->MIDIinports[i].visible==true && mainMIDI->MIDIinports[i].inputdevice)
				{
					class menu_addgroup:public guiMenu
					{
					public:
						menu_addgroup(Seq_Track *t,int index)
						{
							track=t;
							portindex=index;
						}

						void MenuFunction()
						{
							mainedit->AddMIDIInput(track,portindex);
						}

						Seq_Track *track;
						int portindex;
					};

					if(track->GetMIDIIn()->FindPort(i)==false){

						if(!sub)
						{
							w->popmenu->AddLine();
							sub=w->popmenu->AddMenu(Cxs[CXS_CONNECTWITH],0);
						}

						if(sub)
						{
							char nr[NUMBERSTRINGLEN];

							if(char *h=mainvar->GenerateString("P",mainvar->ConvertIntToChar(i+1,nr),":",mainMIDI->MIDIinports[i].GetName()))
							{
								sub->AddFMenu(h,new menu_addgroup(track,i));
								delete h;
							}
						}
					}
				}
			}
		}

		w->ShowPopMenu();
	}
}

void GUI::EditTrackInput(guiWindow *w,Seq_Track *track)
{
	if(!track)return;
	if(track->ismetrotrack==true)
		return;

	AudioDevice *dev=track->song?track->song->audiosystem.device:mainaudio->GetActiveDevice();

	w->DeletePopUpMenu();

	if(dev)
	{
		if(w->DeletePopUpMenu(true))
		{
			char *h;

			if(track->GetVIn())
			{
				char h2[NUMBERSTRINGLEN];
				h=mainvar->GenerateString(track->GetName(),"- Audio In [",mainvar->ConvertIntToChar(track->GetVIn()->channels,h2),"] ",track->GetVIn()->name);
			}
			else
				h=mainvar->GenerateString(track->GetName(),"- Audio In :",Cxs[CXS_TRACKUSECHANNELAUDIOIN]);

			if(h)
			{
				w->popmenu->AddMenu(h,0);

				delete h;

				class menu_achlin:public guiMenu
				{
				public:
					menu_achlin(Seq_Track *t,AudioPort *p){track=t;port=p;}

					void MenuFunction()
					{
						track->SetRecordChannel(port);
						//	maingui->RefreshAllEditors(track->song,EDITORTYPE_AUDIOMIXER,REFRESH_INCHANNELS);

					} //

					Seq_Track *track;
					AudioPort *port;
				};

				w->popmenu->AddLine();

				class menu_aenable:public guiMenu
				{
				public:
					menu_aenable(Seq_Track *t,bool e){track=t;enable=e;}

					void MenuFunction()
					{
						track->SetAudioInputEnable(enable);

					} //

					Seq_Track *track;
					bool enable;
				};

				w->popmenu->AddFMenu(Cxs[CXS_NOINPUTHARDWARE],new menu_aenable(track,track->io.audioinputenable==true?false:true),track->io.audioinputenable==true?false:true);

				/*
				class menu_achlinbypass:public guiMenu
				{
				public:
				menu_achlinbypass(Seq_Track *t){track=t;}

				void MenuFunction()
				{
				track->usetrackvchannels=track->usetrackvchannels==true?false:true;
				} //

				Seq_Track *track;
				};

				w->popmenu->AddFMenu("Track - Audio In Bypass",new menu_achlinbypass(track->parent?(Seq_Track *)track->parent:track),track->GetUseVChannel()==true?false:true);

				*/

				w->popmenu->AddLine();

				int chls=track->io.GetChannels();

				for(int i=0;i<AUDIOCHANNELSDEFINED;i++)
				{
					if(channelschannelsnumber[i]<=chls)
					{
						char *h=0;

						if(track->t_audiofx.recordtrack==0)
						{
							for(int p=0;p<CHANNELSPERPORT;p++)
							{
								AudioPort *inport=&dev->inputaudioports[i][p];
								if(track->GetVIn()==inport)
								{
									h=inport->name;
								}
							}
						}

						char *h2;
						if(h)
							h2=mainvar->GenerateString(channelchannelsinfo[i]," = ",h);
						else
							h2=mainvar->GenerateString(channelchannelsinfo[i]);

						if(guiMenu *sub=w->popmenu->AddMenu(h2,0))
						{
							delete h2;

							for(int p=0;p<CHANNELSPERPORT;p++)
							{
								AudioPort *inport=&dev->inputaudioports[i][p];

								if(inport->visible==true)
									sub->AddFMenu(inport->name,new menu_achlin(track,inport),track->GetVIn()==inport?true:false);
							}
						}
						else
							break;
					}
				}

				if(track->song && track->song->GetCountTrackWithAudioOut(track))
				{
					w->popmenu->AddLine();

					char *h,help[NUMBERSTRINGLEN];

					if(track->t_audiofx.recordtrack)
					{
						h=mainvar->GenerateString("iTrack (",mainvar->ConvertIntToChar(track->t_audiofx.recordtrack->GetTrackIndex()+1,help),")=",track->t_audiofx.recordtrack->GetName());
					}
					else
						h=mainvar->GenerateString("iTrack");

					if(h)
					{
						guiMenu *tmenu=w->popmenu->AddMenu(h,0);

						delete h;

						class menu_addtracktotrackrecord:public guiMenu
						{
						public:
							menu_addtracktotrackrecord(Seq_Track *t,Seq_Track *add){totrack=t;addtrack=add;}

							void MenuFunction()
							{
								totrack->AddTrackToRecord(addtrack);
							} //

							Seq_Track *totrack,*addtrack;
						};

						Seq_Track *t=track->song->FirstTrack();
						while(t)
						{
							if(track!=t && t->outputisinput==true && t!=track->t_audiofx.recordtrack)
							{
								h=mainvar->GenerateString("T:(",mainvar->ConvertIntToChar(t->GetTrackIndex()+1,help),")",t->GetName());

								tmenu->AddFMenu(h,new menu_addtracktotrackrecord(track,t),track->t_audiofx.recordtrack==t?true:false);

								if(h)
									delete h;
							}

							t=t->NextTrack();
						}
					}

					/*
					class menu_achlinbypass:public guiMenu
					{
					public:
					menu_achlinbypass(Seq_Track *t){track=t;}

					void MenuFunction()
					{
					track->usetrackvchannels=track->GetUseVChannel()==true?false:true;
					} //

					Seq_Track *track;
					};

					w->popmenu->AddFMenu("Track - Audio In Bypass",new menu_achlinbypass(track->parent?(Seq_Track *)track->parent:track),track->GetUseVChannel()==true?false:true);
					*/
				}

			}

			w->ShowPopMenu();
		}
	}
}


void GUI::EditTrackRecordType(guiWindow *w,Seq_Track *track)
{
	if(!track)
		return;

	if(track->song)
	{
		if(track->record==true && (track->song->status&Seq_Song::STATUS_RECORD))
			return;
	}

	w->DeletePopUpMenu(true);

	if(guiMenu *popmenu=w->popmenu)
	{
		//char *h=mainvar->GenerateString("Event 
		class menu_tracktype:public guiMenu
		{
		public:
			menu_tracktype(Seq_Track *t,int tid)
			{
				track=t;
				tracktype=tid;
			}

			void MenuFunction()
			{
				mainsettings->defaultrecordtracktype=tracktype;

				if(track->song)
					track->song->SetRecordTrackTypes(track,tracktype);
				else
					track->SetRecordTrackType(tracktype);
			} //

			Seq_Track *track;
			int tracktype;
		};

		char *tracktypes[2];

		//tracktypes[0]=Cxs[CXS_RECALL];
		tracktypes[0]=Cxs[CXS_RECONLYMIDI];
		tracktypes[1]=Cxs[CXS_RECONLYAUDIO];

		if(track->song)
		{
			char *h=mainvar->GenerateString("Track",":",track->GetName());
			if(h)
			{
				popmenu->AddMenu(h,0);
				delete h;
			}

			popmenu->AddLine();
		}

		for(int m=0;m<2;m++)
			popmenu->AddFMenu(tracktypes[m],new menu_tracktype(track,m+1),track->recordtracktype==m+1?true:false);

		int add=0;

		if(track->song)
		{
			// Check other selected Track
			Seq_Track *ct=track->song->FirstTrack();
			while(ct)
			{
				if(ct!=track && ct->IsSelected()==true)
					add++;

				ct=ct->NextTrack();
			}
		}

		if(add)
		{
			popmenu->AddLine();

			class menu_tracktypeall:public guiMenu
			{
			public:
				menu_tracktypeall(Seq_Track *t,int tid)
				{
					track=t;
					tracktype=tid;
				}

				void MenuFunction()
				{
					mainsettings->defaultrecordtracktype=tracktype;

					Seq_Track *ct=track->song->FirstTrack();
					while(ct)
					{
						if(ct==track || ct->IsSelected()==true)
						{
							ct->SetRecordTrackType(tracktype);
						}

						ct=ct->NextTrack();
					}
				} //

				Seq_Track *track;
				int tracktype;
			};

			char h2[32],*h=mainvar->GenerateString(Cxs[CXS_ALLSELECTEDTRACKS]," (",mainvar->ConvertIntToChar(add,h2),")",Cxs[CXS_RECONLYMIDI]);

			if(h)
			{
				popmenu->AddFMenu(h,new menu_tracktypeall(track,TRACKTYPE_MIDI));
				delete h;
			}

			h=mainvar->GenerateString(Cxs[CXS_ALLSELECTEDTRACKS]," (",mainvar->ConvertIntToChar(add,h2),")",Cxs[CXS_RECONLYAUDIO]);
			if(h)
			{
				popmenu->AddFMenu(h,new menu_tracktypeall(track,TRACKTYPE_AUDIO));
				delete h;
			}
		}

		w->ShowPopMenu();
	}
}

void GUI::EditTrackOutput(guiWindow *w,Seq_Track *track)
{
	w->DeletePopUpMenu();

	Seq_Song *song=track->song?track->song:mainvar->GetActiveSong();

	if(!song)
		return;

	bool ok=false;

	for(int i=0;i<LASTSYNTHCHANNEL;i++)
	{
		if(song->audiosystem.FirstChannelType(i))ok=true;
	}

	if(ok==true)
	{
		if(w->DeletePopUpMenu(true))
		{
			// Track Name
			{
				Seq_Track *routingtrack=track->parent?(Seq_Track *)track->parent:track;

				size_t i=strlen(routingtrack->GetName());
				i+=128;

				if(char *h=new char[i])
				{
					strcpy(h,"Audio Routing:");
					mainvar->AddString(h,routingtrack->GetName());

					//w->popmenu->AddLine();
					if(routingtrack->FirstChildTrack())
					{
						char h2[NUMBERSTRINGLEN];
						mainvar->AddString(h," (+");
						mainvar->AddString(h,mainvar->ConvertIntToChar(routingtrack->GetCountChildTracks(),h2));
						mainvar->AddString(h," Child Tracks)");
					}

					mainvar->AddString(h,"/");

					if(routingtrack->io.out_vchannel && routingtrack->usedirecttodevice==true)
					{
						if(char *h2=mainvar->GenerateString(h,"[- Track -> Audio Device ]"))
						{
							delete h;
							h=h2;
						}
					}
					else
					{
						int nr=routingtrack->GetAudioOut()->GetCountGroups();

						switch(nr)
						{
						case 0:
							if(char *h2=mainvar->GenerateString(h,"[-",Cxs[CXS_NOAUDIOCHANNELSUSED],"]"))
							{
								delete h;
								h=h2;
							}
							break;

						default:
							{
								char hh[NUMBERSTRINGLEN];
								if(char *h2=mainvar->GenerateString(h,Cxs[CXS_CHANNELSUSED],":",mainvar->ConvertIntToChar(nr,hh)))
								{
									delete h;
									h=h2;
								}
							}
							break;
						}
					}

					w->popmenu->AddMenu(h,0);
					delete h;

					if(routingtrack->io.out_vchannel && routingtrack->GetAudioOut()->GetCountGroups())
					{
						guiMenu *out=w->popmenu->AddMenu(routingtrack->usedirecttodevice==true?"Audio Track Output -> Device":"Audio Track Output -> Channels/Bus",0);
						if(out)
						{
							class tracktodevice:public guiMenu
							{
							public:
								tracktodevice(Seq_Track *t,bool use){track=t;usedevice=use;}

								void MenuFunction()
								{
									track->usedirecttodevice=usedevice;
								} //

								Seq_Track *track;
								bool usedevice;
							};

							out->AddFMenu("->Device",new tracktodevice(routingtrack,true),routingtrack->usedirecttodevice==true?true:false);
							out->AddFMenu("->Channels/Bus",new tracktodevice(routingtrack,false),routingtrack->usedirecttodevice==false?true:false);
						}
					}

					w->popmenu->AddLine();
				}
			}

			//
			track->CreateTrackOutputMenu(w->popmenu,song); // Track IO

			if(track->ismetrotrack==true)
				goto show;

			w->popmenu->AddLine();

			// Remove
			class menu_remove:public guiMenu
			{
			public:
				menu_remove(Seq_Track *t,AudioChannel *c){track=t;channel=c;}

				void MenuFunction()
				{
					track->RemoveAudioChannel(channel);
					maingui->RefreshAudio(track);
				}

				Seq_Track *track;
				AudioChannel *channel;
			};

			class menu_bypass:public guiMenu
			{
			public:
				menu_bypass(Seq_Track *t,Seq_AudioIOPointer *p){track=t;acp=p;}

				void MenuFunction()
				{
					acp->bypass=acp->bypass==true?false:true;
					maingui->RefreshAudio(track);
				}

				Seq_Track *track;
				Seq_AudioIOPointer *acp;
			};

			class menu_replace:public guiMenu
			{
			public:
				menu_replace(Seq_Track *t,AudioChannel *o,AudioChannel *n)
				{
					track=t;
					oldchannel=o;
					newchannel=n;
				}

				void MenuFunction()
				{
					track->ReplaceAudioChannel(oldchannel,newchannel);
					maingui->RefreshAudio(track);
				}

				Seq_Track *track;
				AudioChannel *oldchannel,*newchannel;
			};

			Seq_AudioIOPointer *acp=track->GetAudioOut()->FirstChannel();

			while(acp)
			{
				guiMenu *g=0;
				char *h;

				if(acp->bypass==true)
					h=mainvar->GenerateString(acp->channel->GetFullName()," <Bypass>");
				else
					h=mainvar->GenerateString(acp->channel->GetFullName());

				if(h)
				{
					g=w->popmenu->AddFMenu(h,new menu_remove(track,acp->channel));
					delete h;
				}

				// Replace with AudioChannel
				if(g)
				{
					if(char *h=mainvar->GenerateString(Cxs[CXS_DELETE],"<>"))
					{
						g->AddFMenu(h,new menu_remove(track,acp->channel));
						delete h;
					}

					if(char *h=mainvar->GenerateString("Bypass","<>"))
					{
						g->AddFMenu(h,new menu_bypass(track,acp),acp->bypass);
						delete h;
					}

					g->AddLine();

					// Replace
					for(int i=0;i<LASTSYNTHCHANNEL;i++)
					{
						AudioChannel *c=song->audiosystem.FirstChannelType(i);

						while(c)
						{
							if(c!=acp->channel)
							{
								switch(c->audiochannelsystemtype)
								{
									//case CHANNELTYPE_AUDIOCHANNEL:
									//case CHANNELTYPE_AUDIOINSTRUMENT:
								case CHANNELTYPE_BUSCHANNEL:

									if(char *h=mainvar->GenerateString(Cxs[CXS_REPLACEWITH],"->",c->GetFullName()))
									{
										g->AddFMenu(h,new menu_replace(track,acp->channel,c));
										delete h;
									}
									break;
								}
							}

							if(c->NextChannel() && c->NextChannel()->audiochannelsystemtype!=c->audiochannelsystemtype && 
								(
								//c->NextChannel()->audiochannelsystemtype==CHANNELTYPE_AUDIOCHANNEL ||
								//c->NextChannel()->audiochannelsystemtype==CHANNELTYPE_AUDIOINSTRUMENT ||
								c->NextChannel()->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL
								)
								)
								g->AddLine();

							c=c->NextChannel();
						}
					}
				}	

				acp=acp->NextChannel();
			}



			w->popmenu->AddLine();

			// Add 
			class menu_add:public guiMenu
			{
			public:
				menu_add(Seq_Track *t,AudioChannel *c){track=t,channel=c;}

				void MenuFunction()
				{
					track->AddAudioChannel(channel);
					maingui->RefreshAudio(track);
				} 

				Seq_Track *track;
				AudioChannel *channel;
			};

			for(int ctype=0;ctype<LASTSYNTHCHANNEL;ctype++)
			{
				char *nameoftype;
				int type;

				switch(ctype)
				{
					/*
					case 0:
					type=CHANNELTYPE_AUDIOCHANNEL;
					nameoftype=" [Audio Channel]";
					break;

					case 1:
					type=CHANNELTYPE_AUDIOINSTRUMENT;
					nameoftype=" [Audio Instrument]";
					break;
					*/

				case 0:
					type=CHANNELTYPE_BUSCHANNEL;
					nameoftype=" [Bus]";
					break;
				}

				guiMenu *subtype=0;

				for(int i=0;i<LASTSYNTHCHANNEL;i++)
				{
					AudioChannel *c=song->audiosystem.FirstChannelType(i);

					while(c)
					{
						if(c->audiochannelsystemtype==type)
						{
							switch(c->audiochannelsystemtype)
							{
							case CHANNELTYPE_BUSCHANNEL:

								if(track->GetAudioOut()->FindChannel(c)==false) // not yet in list ?
								{
									if(!subtype)
									{
										if(char *tn=mainvar->GenerateString(Cxs[CXS_CONNECTWITH],nameoftype))
										{
											subtype=w->popmenu->AddMenu(tn,0);
											delete tn;
										}
									}

									if(subtype)
										subtype->AddFMenu(c->GetFullName(),new menu_add(track,c));
								}
							}
						}

						c=c->NextChannel();
					}
				}

			}// for type

#ifdef OLDIE
			if(track->song)
			{
				// Create new Channels
				w->popmenu->AddLine();
				class menu_createnew:public guiMenu
				{
				public:
					menu_createnew(Seq_Track *t,int type){track=t;newtype=type;}
					void MenuFunction(){track->TrackNewAudioChannel(newtype);} 
					Seq_Track *track;
					int newtype;
				};

				w->popmenu->AddFMenu(Cxs[CXS_ADDNEWBUSCT],new menu_createnew(track,CHANNELTYPE_BUSCHANNEL));
			}
#endif

			if(track->song && (track->PrevTrack() || track->NextTrack()))
			{
				w->popmenu->AddLine();

				class menu_setaudioio:public guiMenu
				{
				public:
					menu_setaudioio(Seq_Track *t,bool sel){track=t;selected=sel;}

					void MenuFunction()
					{
						mainedit->SetSongTracksToAudioIO(track,selected);
					}

					Seq_Track *track;
					bool selected;
				};

				w->popmenu->AddFMenu(Cxs[CXS_CHANGEALLTRACKSAUDIOIO_SEL],new menu_setaudioio(track,true));
				w->popmenu->AddFMenu(Cxs[CXS_CHANGEALLTRACKSAUDIOIO],new menu_setaudioio(track,false));
			}

show:

			w->ShowPopMenu();
		}
	}
}

void GUI::RefreshEffects(AudioEffects *fx)
{
	Seq_Song *song=fx->GetSong();

	if(song)
	{
		guiWindow *w=FirstWindow();

		while(w)
		{
			if(w->WindowSong()==song)
				switch(w->GetEditorID())
			{
				case EDITORTYPE_AUDIOMIXER:
					{
						Edit_AudioMix *etam=(Edit_AudioMix *)w;
						etam->ShowAll();
					}
					break;

					/*
					case EDITORTYPE_ARRANGE:
					if(fx->track && song->GetFocusTrack())
					{
					Edit_Arrange *ar=(Edit_Arrange *)w;

					//if(fx->track==song->GetFocusTrack() || (song->GetFocusTrack()->parent && fx->track==song->GetFocusTrack()->parent))
					//	ar->trackfx.InitTrackMixer(ar->trackfx.starttrackfx_y);
					}
					break;
					*/
			}

			w=w->NextWindow();
		}
	}
}

void GUI::AddCascadeMenu(guiWindow *win,guiMenu *menu)
{
	if(!menu)return;

	guiMenu *n=menu->AddMenu("Windows",0);
	if(n)
	{
		class menu_multiedit:public guiMenu
		{
			void MenuFunction()
			{
				mainsettings->SetMultiEditing(mainsettings->openmultieditor==true?false:true);
			}
		};
		win->menu_multiedit=n->AddFMenu(Cxs[CXS_OPENMULTIEDITOR],new menu_multiedit,mainsettings->openmultieditor);

		n->AddLine();

		class menu_AddDesktop:public guiMenu
		{
			void MenuFunction()
			{

			}
		};
		n->AddFMenu(Cxs[CXS_ADDCAMXDESKTOP],new menu_AddDesktop);

		n->AddLine();

		class menu_Cascade:public guiMenu
		{
			void MenuFunction()
			{
				/*
				TileWindows(

				maingui->GetActiveScreen()->hWndClient,
				// handle of parent window
				MDITILE_HORIZONTAL  ,
				// types of windows not to arrange
				NULL ,// CONST RECT *lpRect,
				// rectangle to arrange windows in
				NULL, //UINT cKids,
				// number of windows to arrange
				NULL // const HWND FAR *lpKids
				// array of window handles
				);
				*/
			}
		};
		n->AddFMenu("Cascade Horizontal",new menu_Cascade);
		class menu_CascadeV:public guiMenu
		{
			void MenuFunction()
			{
				/*
				TileWindows(

				maingui->GetActiveScreen()->hWndClient,
				// handle of parent window
				MDITILE_VERTICAL  ,
				// types of windows not to arrange
				NULL ,// CONST RECT *lpRect,
				// rectangle to arrange windows in
				NULL, //UINT cKids,
				// number of windows to arrange
				NULL // const HWND FAR *lpKids
				// array of window handles
				); 
				*/
			}
		};
		n->AddFMenu("Cascade Vertical",new menu_CascadeV);

		n->AddLine();

		class menu_CloseAllButProject:public guiMenu
		{
			void MenuFunction()
			{
				maingui->DeleteAllWindowsButActiveProject();
			}
		};
		n->AddFMenu(Cxs[CXS_CLOSEALLWINDOWSBUTPROJECT],new menu_CloseAllButProject);

		class menu_CloseAllButSong:public guiMenu
		{
			void MenuFunction()
			{
				maingui->DeleteAllWindowsButActiveSong();
			}
		};
		n->AddFMenu(Cxs[CXS_CLOSEALLWINDOWSBUTSONG],new menu_CloseAllButSong);

		class menu_CloseAllButThis:public guiMenu
		{
			void MenuFunction()
			{
				maingui->DeleteAllWindowsButThis();
			}
		};
		n->AddFMenu(Cxs[CXS_CLOSEALLWINDOWSBUTTHIS],new menu_CloseAllButThis);

		class menu_CloseAll:public guiMenu
		{
			void MenuFunction()
			{
				maingui->DeleteAllWindows();
			}
		};

		n->AddFMenu(Cxs[CXS_CLOSEALLWINDOWS],new menu_CloseAll);
	}
}

void GUI::LearnFromPluginChange(InsertAudioEffect *iae,AudioObject *ao,OSTART time,int index,double value)
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->learn==true)
			w->LearnFromPluginChange(iae,ao,time,index,value);

		w=w->NextWindow();
	}
}

void GUI::LearnFromMIDIEvent(LMIDIEvents *e)
{
	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->learn==true)
			w->LearnFromMIDIEvent(e);

		w=w->NextWindow();
	}
}

#ifdef CAMXGUIHTREADS

PTHREAD_START_ROUTINE guiWindowProc::guiWindowFunc(LPVOID pParam)
{
	int threadid=(int)pParam;

	while(mainvar->exitthreads==false) // Signal Loop
	{	
		maingui->winproc.WaitCoreSignal(threadid);

		if(mainvar->exitthreads==true)break;
		while(guiWindow *gw=maingui->winproc.GetguiWindow())
		{
			gw->RefreshRealtime();
			if(gw->rrt_slowcounter==10)
				gw->RefreshRealtime_Slow();
		}
		maingui->winproc.DoneCore(threadid);

	}// while exitthreads

	maingui->winproc.DoneCore(threadid);

	return 0;
}

int guiWindowProc::StartThread()
{
	int error=0;

#ifdef WIN32

	/*
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioInputFunc,(LPVOID)this, 0,0); // Audio File Buffer Refill Thread

	if(ThreadHandle)SetThreadPriority(ThreadHandle,THREAD_PRIORITY_ABOVE_NORMAL);
	else error++;
	*/

	if(mainvar->cpucores==1)
	{
		coresinit=0;
		return error;
	}

	coresinit=mainvar->cpucores<=MAXGUITHREADS?mainvar->cpucores:MAXGUITHREADS;

	// Create X Cores Threads (2 Cores or more)
	for(int id=0;id<coresinit;id++)
	{
		CoreThreadEventHandle[id]=CreateEvent( 
			NULL,   // default security attributes
			FALSE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object

		if(!CoreThreadEventHandle[id])error++;

		Wait_CoreThreadEventHandle[id]=CreateEvent( 
			NULL,   // default security attributes
			FALSE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object

		if(!Wait_CoreThreadEventHandle[id])error++;

		CoreThreadHandle[id]=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)guiWindowFunc,(LPVOID)id, 0,0); // Audio In Thread
		if(!CoreThreadHandle[id])
			error++;
	}
#endif

	return error;
}
void guiWindowProc::StopThread()
{
	// Bye Bye Core Threads...
	SetCoreSignals(coresinit);
	WaitAllCoresFinished();

	// Reset Core Handles
	for(int i=0;i<coresinit;i++)
	{
		if(CoreThreadEventHandle[i])CloseHandle(CoreThreadEventHandle[i]); 
		if(Wait_CoreThreadEventHandle[i])CloseHandle(Wait_CoreThreadEventHandle[i]); 
	}
}

guiWindow *guiWindowProc::GetguiWindow()
{
	c_guiwindow_sema.Lock();
	guiWindow *uat=maingui->firstfreecorewindow;
	maingui->firstfreecorewindow=uat?uat->nextcorewindow:0;
	c_guiwindow_sema.Unlock();
	return uat;
}

#endif