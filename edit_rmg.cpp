#include "rmg.h"
#include "gui.h"
#include "object_song.h"
#include "MIDIhardware.h"
#include "languagefiles.h"
#include "songmain.h"
#include "editdata.h"

#define RMGGADGETID_START GADGET_ID_START+50
#define XSLIDER_ID RMGGADGETID_START+1
#define YSLIDER_ID RMGGADGETID_START+2

#define RMG_TYPE_ID RMGGADGETID_START+63
#define RMG_EVENTTYPE_ID RMGGADGETID_START+64
#define RMG_BYTE1_ID RMGGADGETID_START+65
#define RMG_GADGETTYPE_ID RMGGADGETID_START+66

#define RMG_VALUE_ID RMGGADGETID_START+67
#define RMG_VALUE1_ID RMGGADGETID_START+68
#define RMG_VALUE2_ID RMGGADGETID_START+69

#define RMG_RASTERSTART_ID RMGGADGETID_START+80

#define RMG_VALUE_EDIT_ID 200
#define RMG_VALUE1_EDIT_ID 201
#define RMG_VALUE2_EDIT_ID 202

void Edit_RMGFX::ShowObject()
{
	if(activermgobject)
	{
		if(type)
		{
			switch(activermgobject->type)
			{
			case RMGTYPE_MIDI:
				{
					type->ChangeButtonText("Type:MIDI Event");

					if(eventtype)
					{
						switch(activermgobject->status)
						{
						case NOTEON:
							{
								eventtype->ChangeButtonText("Event:Note");
							}
							break;

						case PITCHBEND:
							{
								eventtype->ChangeButtonText("Event:Pitchbend");

								if(byte1)
								{
									if(activermgobject->bytes[1]==1) // Range 0-8192
										byte1->ChangeButtonText("-8192<>8191");
									else
										byte1->ChangeButtonText("-128<>127");
								}
							}
							break;

						case PROGRAMCHANGE:
							{
								eventtype->ChangeButtonText("Event:ProgramChange");
							}
							break;

						case CONTROLCHANGE:
							{
								eventtype->ChangeButtonText("Event:ControlChange");

								if(byte1)
								{
									byte1->ChangeButtonText(maingui->ByteToControlInfo(activermgobject->bytes[0],-1));
								}
							}
							break;

						case CHANNELPRESSURE:
							{
								eventtype->ChangeButtonText("Event:ChannelPressure");
							}
							break;

						case POLYPRESSURE:
							{
								eventtype->ChangeButtonText("Event:PolyPressure");
							}
							break;

						case SYSEX:
							{
								eventtype->ChangeButtonText("Event:SysEx");
							}
							break;
						}
					}
				}
				break;

			case RMGTYPE_VST:
				{
					type->ChangeButtonText("Type:VST Controller");
				}
				break;
			}
		}//type

		if(gadgettype)
		{
			char *s=0;

			switch(activermgobject->gadgettype)
			{
			case GADGETTYPE_BUTTON:
				s="GUI:Button";
				break;

			case GADGETTYPE_SLIDER_VERT:
				s="GUI:Vertical Slider";
				break;

			case GADGETTYPE_SLIDER_HORZ:
				s="GUI:Horiz. Slider";
				break;

			case GADGETTYPE_GMBUTTON:
				s="GUI:GM Program Select";
				break;

			case GADGETTYPE_TOGGLEBUTTON:
				s="GUI:Toggle Button";
				break;
			}

			gadgettype->ChangeButtonText(s);
		}

		if(value)
		{
			switch(activermgobject->gadgettype)
			{
			case GADGETTYPE_TOGGLEBUTTON:
				{
					char h[255];
					char h2[NUMBERSTRINGLEN];

					strcpy(h,"Toggle:");
					mainvar->AddString(h,mainvar->ConvertIntToChar(activermgobject->value1,h2));
					mainvar->AddString(h,"/");
					mainvar->AddString(h,mainvar->ConvertIntToChar(activermgobject->value2,h2));

					value->ChangeButtonText(h);
				}
				break;

			default:
				{
					char h[255];
					char h2[NUMBERSTRINGLEN];

					strcpy(h,"Value:");
					mainvar->AddString(h,mainvar->ConvertIntToChar(activermgobject->value,h2));

					value->ChangeButtonText(h);
				}
				break;
			}
		}
	}
}

guiGadget *Edit_RMGFX::Gadget(guiGadget *g)
{
	if(activermgobject)
	{
		switch(g->gadgetID)
		{
		case RMG_VALUE_ID:
			if(activermgobject->gadgettype==GADGETTYPE_TOGGLEBUTTON)
			{
				rmgeditor->DeletePopUpMenu(true);

				if(rmgeditor->popmenu)
				{
					class menu_toggle:public guiMenu
					{
					public:
						menu_toggle(Edit_RMG *e,RMGObject *o,int c,int cx,int cy)
						{
							editor=e;
							object=o;
							v=c;
							x=cx;
							y=cy;
						}

						void MenuFunction()
						{
							EditData *edit=new EditData;

							if(edit)
							{
								edit->win=editor;
								edit->x=x;
								edit->y=y;

								edit->from=object->from;
								edit->to=object->to;

								if(v==1)
								{
									edit->id=RMG_VALUE1_EDIT_ID;
									edit->title="Toggle Value1";
									edit->value=object->value1;
								}
								else
								{
									edit->id=RMG_VALUE2_EDIT_ID;
									edit->title="Toggle Value2";
									edit->value=object->value2;
								}

								edit->type=EditData::EDITDATA_TYPE_INTEGER;

								maingui->EditDataValue(edit);
							}
						} //

						Edit_RMG *editor;
						RMGObject *object;
						int v;
						int x,y;
					};

					rmgeditor->popmenu->AddFMenu("Edit Value 1",new menu_toggle(rmgeditor,activermgobject,1,g->x2,g->y));
					rmgeditor->popmenu->AddFMenu("Edit Value 2",new menu_toggle(rmgeditor,activermgobject,2,g->x2,g->y));

					rmgeditor->ShowPopMenu();	
				}// if popmenu
			}
			else
			{
				EditData *edit=new EditData;

				if(edit)
				{
					edit->win=rmgeditor;
					edit->x=g->x2;
					edit->y=g->y;

					edit->id=RMG_VALUE_EDIT_ID;
					edit->title="Value";

					edit->type=EditData::EDITDATA_TYPE_INTEGER;
					edit->from=activermgobject->from;
					edit->to=activermgobject->to;
					edit->value=activermgobject->value;

					maingui->EditDataValue(edit);
				}
			}
			break;

		case RMG_TYPE_ID:
			break;

		case RMG_EVENTTYPE_ID: // MIDI Event Type
			{
				rmgeditor->DeletePopUpMenu(true);

				if(rmgeditor->popmenu)
				{
					class menu_type:public guiMenu
					{
					public:
						menu_type(RMGObject *o,UBYTE s)
						{
							object=o;
							status=s;
						}

						void MenuFunction()
						{
							if(object->status!=status)
							{
								object->NewStatus(status);
							}
						} //

						RMGObject *object;
						UBYTE status;
					};

					bool sel;

					if(activermgobject->status==NOTEON)
						sel=true;
					else
						sel=false;
					rmgeditor->popmenu->AddFMenu("Note",new menu_type(activermgobject,NOTEON),sel);

					if(activermgobject->status==CONTROLCHANGE)
						sel=true;
					else
						sel=false;
					rmgeditor->popmenu->AddFMenu("ControlChange",new menu_type(activermgobject,CONTROLCHANGE),sel);

					if(activermgobject->status==CHANNELPRESSURE)
						sel=true;
					else
						sel=false;
					rmgeditor->popmenu->AddFMenu("ChannelPressure",new menu_type(activermgobject,CHANNELPRESSURE),sel);

					if(activermgobject->status==POLYPRESSURE)
						sel=true;
					else
						sel=false;
					rmgeditor->popmenu->AddFMenu("PolyPressure",new menu_type(activermgobject,POLYPRESSURE),sel);

					if(activermgobject->status==PROGRAMCHANGE)
						sel=true;
					else
						sel=false;
					rmgeditor->popmenu->AddFMenu("ProgramChange",new menu_type(activermgobject,PROGRAMCHANGE),sel);

					if(activermgobject->status==PITCHBEND)
						sel=true;
					else
						sel=false;
					rmgeditor->popmenu->AddFMenu("Pitchbend",new menu_type(activermgobject,PITCHBEND),sel);

					if(activermgobject->status==SYSEX)
						sel=true;
					else
						sel=false;
					rmgeditor->popmenu->AddFMenu("SYSEX",new menu_type(activermgobject,SYSEX),sel);

					rmgeditor->ShowPopMenu();	
				}// if popmenu
			}
			break;

		case RMG_BYTE1_ID:
			{
				switch(activermgobject->status)
				{
				case PITCHBEND: // 127/8192
					{
						rmgeditor->DeletePopUpMenu(true);

						if(rmgeditor->popmenu)
						{
							class menu_r:public guiMenu
							{
							public:
								menu_r(RMGObject *o,UBYTE v)
								{
									object=o;
									range=v;
								}

								void MenuFunction()
								{
									if(object->bytes[1]!=range)
									{
										object->bytes[1]=range;

										if(range==1)
										{
											object->from=0;
											object->to=8191+8192;
											object->value=8192;
											object->offset=-8192;
											object->multi=1;
										}
										else
										{
											object->from=0;
											object->to=127;
											object->value=64;

											object->offset=-8192;
											object->multi=128;
										}

										maingui->RefreshRMGObject(object);
									}
								} //

								RMGObject *object;
								UBYTE range;
							};

							// 127 ?

							bool sel;

							if(activermgobject->bytes[1]==0)
								sel=true;
							else
								sel=false;

							rmgeditor->popmenu->AddFMenu("Range:7Bit",new menu_r(activermgobject,0),sel);

							if(activermgobject->bytes[1]==1)
								sel=true;
							else
								sel=false;

							rmgeditor->popmenu->AddFMenu("Range:14Bit",new menu_r(activermgobject,1),sel);

							rmgeditor->ShowPopMenu();
						}
					}
					break;

				case CONTROLCHANGE: // Controller
					{
						rmgeditor->DeletePopUpMenu(true);

						if(rmgeditor->popmenu)
						{
							class menu_c:public guiMenu
							{
							public:
								menu_c(RMGObject *o,UBYTE ctrl)
								{
									object=o;
									controller=ctrl;
								}

								void MenuFunction()
								{
									if(object->bytes[0]!=controller)
									{
										object->bytes[0]=controller;
										maingui->RefreshRMGObject(object);
									}
								} //

								RMGObject *object;
								UBYTE controller;
							};

							int i=0;

							for(int a=0;a<4;a++)
							{
								char *m=0;

								switch(a)
								{
								case 0:
									if(activermgobject->bytes[0]>=0 && activermgobject->bytes[0]<=32)
										m=">>> MIDI Control 0-31";
									else
										m="MIDI Control 0-31";
									break;

								case 1:
									if(activermgobject->bytes[0]>=32 && activermgobject->bytes[0]<=63)
										m=">>> MIDI Control 32-63";
									else
										m="MIDI Control 32-63";
									break;

								case 2:
									if(activermgobject->bytes[0]>=64 && activermgobject->bytes[0]<=95)
										m=">>> MIDI Control 64-95";
									else
										m="MIDI Control 64-95";
									break;

								case 3:
									if(activermgobject->bytes[0]>=96)
										m=">>> MIDI Control 96-127";
									else
										m="MIDI Control 96-127";
									break;
								}

								guiMenu *s=rmgeditor->popmenu->AddMenu(m,0);

								if(s)
								{
									int c=32;

									while(c--)
									{
										bool sel;

										if(activermgobject->bytes[0]==i)
											sel=true;
										else
											sel=false;

										s->AddFMenu(maingui->ByteToControlInfo(i,-1,true),new menu_c(activermgobject,i),sel);

										i++;
									}
								}
							}

							rmgeditor->ShowPopMenu();
						}
					}
					break;
				}
			}
			break;

		default:
			return g;
		}

		return 0;
	}

	return g;
}

bool Edit_RMGFX::Init()
{
	#ifdef OLDIE
	int y2;
	int x=frame->x;
	int y=frame->y;
	int x2=frame->x2;
	int addy=maingui->GetFontSizeY_Sub();

	ResetGadgets();

	if(glist)
		glist->RemoveGadgetList();

	glist=0;

	if(activermgobject)
	{
		glist=rmgeditor->gadgetlists.AddGadgetList(rmgeditor);

		if(glist)
		{		
			if(frame->ondisplay==true)
			{
				if((y2=y+addy)<=frame->y2) // Type
				{
					type=glist->AddButton(x,y,x2,y2,RMG_TYPE_ID);
					if(type)
						type->AddTooltip("Object Type");

					y=y2+1;
				}

				if((y2=y+addy)<=frame->y2) // Event Type
				{
					eventtype=glist->AddButton(x,y,x2,y2,RMG_EVENTTYPE_ID);

					if(eventtype)
						eventtype->AddTooltip("MIDI Event Type");

					y=y2+1;
				}

				// Gadget Type
				if((y2=y+addy)<=frame->y2)
				{
					gadgettype=glist->AddButton(x,y,x2,y2,RMG_GADGETTYPE_ID);

					if(gadgettype)
						gadgettype->AddTooltip("Object GUI Type");

					y=y2+1;
				}

				// Value
				if(activermgobject)
				{
					bool v1=false;

					switch(activermgobject->gadgettype)
					{
					case GADGETTYPE_BUTTON:
					case GADGETTYPE_TOGGLEBUTTON:
						v1=true;
						break;
					}

					if(v1==true)
					{
						if((y2=y+addy)<=frame->y2)
						{
							value=glist->AddButton(x,y,x2,y2,RMG_VALUE_ID);

							if(value)
								value->AddTooltip("Value");

							y=y2+1;
						}
					}
				}

				// Byte1
				if(activermgobject)
				{
					char *tt=0;
					bool initb1=false;

					switch(activermgobject->status)
					{
					case PITCHBEND:
						{
							initb1=true;
							tt="Pitchbend Range";
						}
						break;

					case CONTROLCHANGE:
						{
							initb1=true;
							tt="Controller";
						}
						break;
					}

					if(initb1==true)
					{
						if((y2=y+addy)<=frame->y2) // Byte1 Type
						{
							byte1=glist->AddButton(x,y,x2,y2,RMG_BYTE1_ID);
							if(byte1 && tt)
								byte1->AddTooltip(tt);

							y=y2+1;
						}
					}
				}

				ShowObject();

				return true;
			}
		}
	}
#endif

	return false;
}

void Edit_RMG::MouseButton(int flag)
{
#ifdef OLDIE
	switch(mousemode)
	{
	case EM_DELETE:
		if(map)
		{
			RMGObjectOnDisplay *found=0;
			RMGObjectOnDisplay *ro=FirstObjectOnDisplay();

			while(ro)
			{
				if(ro->Inside(GetMouseX(),GetMouseY())==true)
					found=ro;

				ro=ro->NextRMGObjectOnDisplay();
			}

			if(found)
			{
				if(fx.activermgobject==found->object)
					fx.activermgobject=found->object->NextOrPrevObject();

				map->DeleteRMGObject(found->object);
				ShowMap();

				fx.Init();
			}
		}
		break;

	case EM_CREATE:
		if(map)
		{
			bool found=false;

			RMGObjectOnDisplay *ro=FirstObjectOnDisplay();

			while(ro)
			{
				if(ro->Inside(GetMouseX(),GetMouseY())==true)
					found=true;

				ro=ro->NextRMGObjectOnDisplay();
			}

			if(found==false)
			{
				if(RMGOBJ_Button *b=new RMGOBJ_Button)
				{
					b->fromx=ConvertXToIndex(GetMouseX());
					b->fromy=ConvertYToIndex(GetMouseY());

					b->tox=b->fromx+4;
					b->toy=b->fromy+3;

					if(!fx.activermgobject)
						fx.activermgobject=b;

					map->AddRMGObject(b);

					ShowMap();

					fx.Init();
				}
			}
		}
		break;

	case EM_EDIT:
		{
			RMGObjectOnDisplay *ro=FirstObjectOnDisplay();

			while(ro)
			{
				if(ro->Inside(GetMouseX(),GetMouseY())==true)
				{
					if(ro->object!=fx.activermgobject)
					{
						fx.activermgobject=ro->object;

						fx.Init();
						fx.ShowObject();
						ShowMap();
					}

					break;
				}

				ro=ro->NextRMGObjectOnDisplay();
			}
		}
		break;
	}
#endif

}

Edit_RMG::Edit_RMG()
{
	editorid=EDITORTYPE_RMG;
	editorname="Message Generator";

	map=0;

	from_x=0;
	from_y=0;

	to_x=0;
	to_y=0;

	fx.rmgeditor=this;
	
	showeffects=true;

	fx.glist=0;
	xslider=0;
	yslider=0;
}

void Edit_RMG::Gadget(guiGadget *g)
{
	// Slider
	if(g==xslider)
	{
		if(g->GetPos()!=from_x)
		{
			from_x=g->GetPos();
			ShowMap();
		}

		g=0;
	}
	else
		if(g==yslider)
		{
			if(g->GetPos()!=from_y)
			{
				from_y=g->GetPos();
				ShowMap();
			}

			g=0;
		}

		if(g)
		{
			int mm=mousemode;

			g=CheckToolBox(g);

			if(!g)
			{
				if( (mousemode==EM_EDIT && mm!=EM_EDIT) ||
					(mm==EM_EDIT && mousemode!=EM_EDIT)
					)
				{
					ShowMap();
				}
				else
					if( (mousemode==EM_DELETE && mm!=EM_DELETE) ||
						(mm==EM_DELETE && mousemode!=EM_DELETE)
						)
					{
						ShowMap();
					}
			}
		}

		if(g)
			g=fx.Gadget(g);

		if(g)
		{
			// Check Objects
			bool found=false;
			RMGObjectOnDisplay *f=FirstObjectOnDisplay();

			while(f && found==false)
			{
				if(f->object)
				{
					for(int i=0;i<MAXGADGETS;i++)
					{
						if(g==f->object->gadget[i])
						{
							f->object->Clicked(this,g);
							found=true;
						}
					}
				}

				f=f->NextRMGObjectOnDisplay();
			}

			if(found==false)
			{
			}
		}
}

void Edit_RMG::RefreshObject(RMGObject *o)
{
	#ifdef OLDIE
	if(frame.edit.ondisplay==true && fx.activermgobject==o)
	{
		fx.Init();
	}
#endif
}

bool Edit_RMG::CheckRMGObjectInside(RMGObject *r)
{
	if(r)
	{
		if(r->fromx>=from_x && r->fromx<=to_x &&  // X |> fromx < tox
			r->fromy>=from_y && r->fromy<=to_y
			)
			return true;
	}

	return false;
}

void Edit_RMG::ShowRaster()
{
	#ifdef OLDIE
	if(map && frame.raster.ondisplay==true)
	{
		int x=frame.raster.x;
		int y=frame.raster.y;

		int i=frame.raster.x2-frame.raster.x;
		i/=map->rastersize_x;

		to_x=from_x+i;

		for(int a=0;a<i;a++)
		{
			guibuffer->guiDrawLine(x,frame.raster.y,x,frame.raster.y2);
			x+=map->rastersize_x;
		}

		i=frame.raster.y2-frame.raster.y;
		i/=map->rastersize_y;

		to_y=from_y+i;

		for(int a=0;a<i;a++)
		{
			guibuffer->guiDrawLine(frame.raster.x,y,frame.raster.x2,y);
			y+=map->rastersize_y;
		}
	}
#endif
}

void Edit_RMG::AddObjectToDisplay(RMGObject *r,int x,int y,int x2,int y2)
{
	if(RMGObjectOnDisplay *n=new RMGObjectOnDisplay)
	{
		n->object=r;
		n->co_x=x;
		n->co_y=y;
		n->co_x2=x2;
		n->co_y2=y2;

		objectsondisplay.AddEndO(n);
	}
}

void Edit_RMG::ShowMap()
{
	#ifdef OLDIE
	if(rastergadgets)
		rastergadgets->RemoveGadgetList();

	rastergadgets=0;

	if(frame.raster.ondisplay==true && map)
	{
		// Calc Raster

		int i=frame.raster.x2-frame.raster.x;
		i/=map->rastersize_x;

		to_x=from_x+i;

		i=frame.raster.y2-frame.raster.y;
		i/=map->rastersize_y;

		to_y=from_y+i;

		if(mousemode!=EM_EDIT && mousemode!=EM_DELETE)
		{
			if(guibuffer)
			{
				frame.raster.Fill(guibuffer,COLOUR_BLUE_LIGHT);
				BltGUIBuffer_Frame(&frame.raster);
			}

			rastergadgets=gadgetlists.AddGadgetList(this);
		}
		else
		{
			ShowRaster();
			rastergadgets=0;
		}

		objectsondisplay.DeleteAllO();

		// Reset Gadget Pointer
		{
			RMGObject *r=map->FirstRMGObject();

			while(r)
			{
				for(int i=0;i<MAXGADGETS;i++)
					r->gadget[i]=0;

				r=r->NextRMGObject();
			}
		}

		if(map)
		{
			int gadgetid=RMG_RASTERSTART_ID;
			RMGObject *r=map->FirstRMGObject();

			while(r)
			{
				if(CheckRMGObjectInside(r)==true)
				{
					if(mousemode==EM_EDIT || mousemode==EM_DELETE)
					{
						int col=COLOUR_GREY_LIGHT;

						if(r==fx.activermgobject)
							col=COLOUR_GREY;

						if(guibuffer)
							guibuffer->guiFillRect3D(
							GetX(r->fromx),
							GetY(r->fromy),
							GetX(r->tox),
							GetY(r->toy),
							col
							);
					}
					else
					{
						gadgetid=r->Draw(this,gadgetid);
					}

					AddObjectToDisplay(r,
						GetX(r->fromx),
						GetY(r->fromy),
						GetX(r->tox),
						GetY(r->toy)
						);
				}

				r=r->NextRMGObject();
			}
		}

		if(mousemode==EM_EDIT || mousemode==EM_DELETE)
			BltGUIBuffer_Frame(&frame.raster);
	}
#endif
}

guiMenu *Edit_RMG::CreateMenu()
{
//	ResetUndoMenu();

	if(menu)
		menu->RemoveMenu();

	if(menu=new guiMenu)
	{
		guiMenu *file=menu->AddMenu(Cxs[CXS_FILE],0);

		if(file)
		{
			class menu_CreateNewMap:public guiMenu
			{
			public:
				menu_CreateNewMap(Edit_RMG *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					// mainedit->SetNoteLength(editor,&editor->patternselection);	
				}

				Edit_RMG *editor;
			};

			file->AddFMenu("Create new Map",new menu_CreateNewMap(this));
			guiMenu *sel=file->AddMenu("Select Map",0);
			if(sel)
			{
				class menu_SelectMap:public guiMenu
				{
				public:
					menu_SelectMap(Edit_RMG *e,RMGMap *m)
					{
						editor=e;
						map=m;
					}

					void MenuFunction()
					{
						// mainedit->SetNoteLength(editor,&editor->patternselection);	
					}

					Edit_RMG *editor;
					RMGMap *map;
				};

				RMGMap *m=mainrmgmap->FirstMap();

				while(m)
				{
					bool msel;

					if(m==this->map)
						msel=true;
					else
						msel=false;

					sel->AddFMenu(m->name,new menu_SelectMap(this,m),msel);

					m=m->NextMap();
				}
			}

			file->AddLine();

			class menu_OpenNewMap:public guiMenu
			{
			public:
				menu_OpenNewMap(Edit_RMG *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					// mainedit->SetNoteLength(editor,&editor->patternselection);	
				}

				Edit_RMG *editor;
			};
			file->AddFMenu("Open Map",new menu_OpenNewMap(this));

			class menu_SaveNMap:public guiMenu
			{
			public:
				menu_SaveNMap(Edit_RMG *e)
				{
					editor=e;
				}

				void MenuFunction()
				{
					// mainedit->SetNoteLength(editor,&editor->patternselection);	
				}

				Edit_RMG *editor;
			};
			file->AddFMenu("Save Map",new menu_SaveNMap(this));
		}

		guiMenu *edit=menu->AddMenu(Cxs[CXS_EDIT],0);

		if(edit)
		{
		}
	}

		maingui->AddCascadeMenu(this,menu);
	return menu;
}

EditData *Edit_RMG::EditDataMessage(EditData *data)
{
	switch(data->id)
	{
	case RMG_VALUE_EDIT_ID:
		if(fx.activermgobject)
		{
			fx.activermgobject->value=data->newvalue;

			fx.ShowObject();
		}
		break;

	case RMG_VALUE1_EDIT_ID:
		if(fx.activermgobject)
		{
			fx.activermgobject->value1=data->newvalue;

			fx.ShowObject();
		}
		break;

	case RMG_VALUE2_EDIT_ID:
		if(fx.activermgobject)
		{
			fx.activermgobject->value2=data->newvalue;

			fx.ShowObject();
		}
		break;
	}

	return data;
}

void Edit_RMG::Init()
{
	#ifdef OLDIE
	FreeMemory();

	if(guibuffer && width && height)
	{
		if(winmode&(WINDOWMODE_INIT|WINDOWMODE_RESIZE))
		{
			gadgetlists.RemoveAllGadgetLists();

			rastergadgets=gadgetlists.AddGadgetList(this);

			guitoolbox.CreateToolBox(TOOLBOXTYPE_RMGEDITOR);

			if(showeffects==true)
			{
				frame.edit.on=true;
				frame.edit.x=0;
				//frame.edit.y=guitoolbox.GetY2(0);	
				frame.edit.x2=100;
				frame.edit.y2=height;
			}	

			frame.raster.on=true;
			frame.raster.x=105;
			frame.raster.x2=width;
			frame.raster.y=0;
			frame.raster.y2=height;

			frame.raster.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ);

			if(frame.raster.ondisplay==true)
			{
				
				if(guiGadgetList *gl=gadgetlists.AddGadgetList(this))
				{
					// X Slider
					SliderCo horz;

					// pos
					horz.x=frame.raster.x;
					horz.y=frame.raster.y2+1;
					horz.x2=frame.raster.x2;
					horz.y2=height;

					horz.horz=true;

					horz.page=10; // 10%

					horz.pos=from_x;
					horz.from=0;
					horz.to=1000; 

					xslider=gl->AddSlider(&horz,XSLIDER_ID,0);

					// Y Slider
					SliderCo vert;

					vert.x=frame.raster.x2+1;
					vert.y=frame.raster.y;
					vert.x2=width;
					vert.y2=height;

					vert.page=10; // 10%

					vert.pos=from_y;
					vert.from=0;
					vert.to=1000; 

					yslider=gl->AddSlider(&vert,YSLIDER_ID,0);
				}
			}

			if(!map)
			{
				map=mainrmgmap->FirstMap();

				if(map)
					fx.activermgobject=map->FirstRMGObject();
			}

			ShowMap();

			if(frame.edit.CheckIfDisplay(this,-EDITOR_SLIDER_SIZE_VERT,-EDITOR_SLIDER_SIZE_HORZ)==true)
			{
				fx.Init();
			}
		}
	}
#endif
}

void Edit_RMG::FreeMemory()
{
	if(winmode&WINDOWMODE_INIT)
	{
		//gadgetlists.RemoveAllGadgetLists();

		fx.glist=0;
		xslider=0;
		yslider=0;

		objectsondisplay.DeleteAllO();

		rastergadgets=0;
	}
}

int Edit_RMG::ConvertXToIndex(int cx)
{
	#ifdef OLDIE
	if(map && 
		frame.raster.ondisplay==true && 
		cx>=frame.raster.x && 
		cx<=frame.raster.x2)
	{
		int h=cx-frame.raster.x;

		h/=map->rastersize_x;

		return h+from_x;
	}
#endif

	return -1;
}

int Edit_RMG::ConvertYToIndex(int cy)
{
	#ifdef OLDIE
	if(map && 
		frame.raster.ondisplay==true && 
		cy>=frame.raster.y&& 
		cy<=frame.raster.y2)
	{
		int h=cy-frame.raster.y;

		h/=map->rastersize_y;

		return h+from_y;
	}
#endif

	return -1;
}

int Edit_RMG::GetX(int index)
{
	#ifdef OLDIE
	if(index>=from_x && index<to_x)
	{
		return frame.raster.x+((index-from_x)*map->rastersize_x);
	}
#endif
	return -1;
}

int Edit_RMG::GetY(int index)
{
	#ifdef OLDIE
	if(index>=from_y && index<to_y)
	{
		return frame.raster.y+((index-from_y)*map->rastersize_y);
	}
#endif

	return -1;
}

// Standard RMG
void RMGObject::NewStatus(UBYTE nstatus)
{
	status=nstatus;

	switch(status)
	{
	case NOTEON:
		{
			bytes[0]=64; // key

			from=0;
			to=127;
			multi=1;
			offset=0;
			value=100; // velo
		}
		break;

	case CHANNELPRESSURE:
		{
			from=0;
			to=127;
			multi=1;
			offset=0;
			value=0;
		}
		break;

	case POLYPRESSURE:
		{
			from=0;
			to=127;
			multi=1;
			offset=0;

			bytes[0]=64; // key
			value=100;
		}
		break;

	case CONTROLCHANGE:
		{
			bytes[0]=7;

			from=0;
			to=127;
			multi=1;
			offset=0;

			value=127;
		}
		break;

	case PROGRAMCHANGE:
		{
			from=0;
			to=127;
			multi=1;
			offset=0;

			value=0;
		}
		break;

	case PITCHBEND:
		{
			from=0;
			to=127;
			multi=1;
			offset=0;

			bytes[0]=0; // msb
			bytes[1]=0; // -128<>127

			value=64; // lsb
		}
		break;

	case SYSEX:
		break;
	}

	maingui->RefreshRMGObject(this);
}

void RMGObject::Send(Seq_Event *seqevent)
{
	Seq_Song *song=mainvar->GetActiveSong();

	if(song && song->GetFocusTrack())
	{
		seqevent->ostart=seqevent->staticostart=song->GetSongPosition();

		if(mainMIDI->generator_inputdevice)
		{
			NewEventData input;

			input.fromdev=mainMIDI->generator_inputdevice;

			mainMIDI->generator_inputdevice->AddMonitor(seqevent->status,seqevent->GetByte1(),seqevent->GetByte2());
			mainMIDI->CheckNewEventData(song,&input,seqevent); // record possible

		}
	}
}

void RMGObject::Output()
{
	switch(type)
	{
	case RMGTYPE_MIDI: // Standard MIDI Event --->
		{
			switch(status)
			{
			case NOTEON:
				{
				}
				break;

			case PITCHBEND:
				{
					Pitchbend pitchbend;

					pitchbend.status=status;

					if(bytes[1]==1) // -8192<->8191
					{
						int h=value+8192;

						h/=128;
						pitchbend.msb=(UBYTE)h;
						pitchbend.lsb=(UBYTE)(value-(h*128));
					}
					else
					{
						pitchbend.msb=(UBYTE)value;
						pitchbend.lsb=0;
					}

					pitchbend.pattern=0;

					Send(&pitchbend);
				}
				break;

			case PROGRAMCHANGE:
				{
					ProgramChange pc;

					pc.status=status;
					pc.program=(UBYTE)value;
					pc.pattern=0;

					Send(&pc);
				}
				break;

			case CONTROLCHANGE:
				{
					ControlChange cc;

					cc.status=status;
					cc.controller=bytes[0];
					cc.value=(UBYTE)value;
					cc.pattern=0;

					Send(&cc);
				}
				break;

			case CHANNELPRESSURE:
				{
					ChannelPressure cp;

					cp.status=status;
					cp.pressure=(UBYTE)value;
					cp.pattern=0;

					Send(&cp);
				}
				break;

			case POLYPRESSURE:
				{
					PolyPressure pp;

					pp.status=status;
					pp.key=bytes[0];
					pp.pressure=(UBYTE)value;
					pp.pattern=0;

					Send(&pp);
				}
				break;

			case SYSEX:
				{
				}
				break;
			}
		}
		break;
	}
}

// Button
int RMGOBJ_Button::Draw(Edit_RMG *rmg,int id)
{
	if(rmg->rastergadgets)
	{
		gadget[0]=rmg->rastergadgets->AddButton(
			rmg->GetX(fromx),
			rmg->GetY(fromy),
			rmg->GetX(tox),
			rmg->GetY(toy),
			id++
			);

		if(gadget[0])
			gadget[0]->AddTooltip("MIDI Event");
	}

	return id;
}

void RMGOBJ_Button::Execute()
{
	Output();
}

// Toggle Button
int RMGOBJ_ToggleButton::Draw(Edit_RMG *rmg,int id)
{
	if(rmg->rastergadgets)
	{
		gadget[0]=rmg->rastergadgets->AddButton(
			rmg->GetX(fromx),
			rmg->GetY(fromy),
			rmg->GetX(tox),
			rmg->GetY(toy),
			id++
			);

		if(gadget[0])
			gadget[0]->AddTooltip("MIDI Event");
	}

	return id;
}

void RMGOBJ_ToggleButton::Execute()
{
	if(valueindex==0)
	{
		value=value1;
		valueindex=1;
	}
	else
	{
		value=value2;
		valueindex=0;
	}

	Output();
}

// GM Button
int RMGOBJ_GMButton::Draw(Edit_RMG *rmg,int id)
{
	if(rmg->rastergadgets)
	{
		gadget[0]=rmg->rastergadgets->AddButton(
			rmg->GetX(fromx),
			rmg->GetY(fromy),
			rmg->GetX(tox),
			rmg->GetY(toy),
			id++
			);

		if(gadget[0])
			gadget[0]->AddTooltip("General MIDI Program Selection");
	}

	return id;
}

void RMGOBJ_GMButton::Execute()
{
	Output();
}

void RMGOBJ_GMButton::Clicked(Edit_RMG *rmg,guiGadget *g)
{
	rmg->DeletePopUpMenu(true);

	if(rmg->popmenu)
	{
		class menu_MIDIprog:public guiMenu
		{
		public:
			menu_MIDIprog(RMGOBJ_GMButton *b,int co)
			{
				button=b;
				prognr=co;
			}

			void MenuFunction()
			{
				button->value=prognr;
				button->Execute();
			} //

			RMGOBJ_GMButton *button;
			int prognr;
		};

		char help[255];

		int p=0;
		for(int a=0;a<16;a++)
		{
			if(value>=p && value<p+8)
			{
				strcpy(help,">>>");
				mainvar->AddString(help,gmproggroups[a]);
			}
			else
				strcpy(help,gmproggroups[a]);

			guiMenu *s=rmg->popmenu->AddMenu(help,0);

			if(s)
			{
				for(int i=0;i<8;i++)
				{
					bool sel;

					if(value==p)
						sel=true;
					else
						sel=false;

					s->AddFMenu(gmprognames[p],new menu_MIDIprog(this,p),sel);

					p++;
				}
			}
		}

		rmg->ShowPopMenu();	
	}// if popmenu
}

// Slider V
void RMGOBJ_Slider_Vert::RefreshGUI(int flag)
{
	if(flag==0)
		flag=RMG_RGUI_SLIDER_BUTTON|RMG_RGUI_SLIDER_SLIDER;

	if((flag&RMG_RGUI_SLIDER_BUTTON) && gadget[1])
	{
		char h[NUMBERSTRINGLEN];

		gadget[1]->ChangeButtonText(mainvar->ConvertIntToChar(value,h));
	}

	if((flag&RMG_RGUI_SLIDER_SLIDER) && gadget[0])
	{
	}
}

int RMGOBJ_Slider_Vert::Draw(Edit_RMG *rmg,int id)
{
	if(rmg->rastergadgets)
	{
		int y2=rmg->GetY(toy);

		y2-=maingui->GetFontSizeY();

		SliderCo horz;

		// pos
		horz.x=rmg->GetX(fromx);
		horz.y=rmg->GetY(fromy);
		horz.x2=rmg->GetX(tox);
		horz.y2=y2;
		horz.page=10; // 10%

		horz.pos=to-value;
		horz.from=from;
		horz.to=to; // 10.000 = 100.00 %percent

		gadget[0]=rmg->rastergadgets->AddSlider(&horz,id++,0);

		if(gadget[0])
			gadget[0]->AddTooltip("Object Slider");

		gadget[1]=rmg->rastergadgets->AddButton(
			horz.x,
			y2+1,
			horz.x2,
			rmg->GetY(toy),
			id++
			);

		if(gadget[1])
			gadget[1]->AddTooltip("Object Button");

		RefreshGUI(RMG_RGUI_SLIDER_BUTTON);
	}

	return id;
}

void RMGOBJ_Slider_Vert::Execute()
{
	Output();
}

void RMGOBJ_Slider_Vert::Clicked(Edit_RMG *rmg,guiGadget *g)
{
	if(g==gadget[0]) // Slider
	{
		value=(to-g->GetPos()*multi)+offset;

		Execute();
		RefreshGUI(RMG_RGUI_SLIDER_BUTTON);
	}
}


// Slider H
void RMGOBJ_Slider_Horz::RefreshGUI(int flag)
{
	if(flag==0)
		flag=RMG_RGUI_SLIDER_BUTTON|RMG_RGUI_SLIDER_SLIDER;

	if((flag&RMG_RGUI_SLIDER_BUTTON) && gadget[1])
	{
		char h[NUMBERSTRINGLEN];
		gadget[1]->ChangeButtonText(mainvar->ConvertIntToChar(value,h));
	}

	if((flag&RMG_RGUI_SLIDER_SLIDER) && gadget[0])
	{
	}
}

int RMGOBJ_Slider_Horz::Draw(Edit_RMG *rmg,int id)
{
	if(rmg->rastergadgets)
	{
		int y2=rmg->GetY(toy);

		y2-=maingui->GetFontSizeY();

		SliderCo horz;

		// pos
		horz.x=rmg->GetX(fromx);
		horz.y=rmg->GetY(fromy);
		horz.x2=rmg->GetX(tox);
		horz.y2=y2;
		horz.page=10; // 10%

		horz.pos=to-value;
		horz.from=from;
		horz.to=to; // 10.000 = 100.00 %percent

		gadget[0]=rmg->rastergadgets->AddSlider(&horz,id++,0);

		if(gadget[0])
			gadget[0]->AddTooltip("Object Slider");

		gadget[1]=rmg->rastergadgets->AddButton(
			horz.x,
			y2+1,
			horz.x2,
			rmg->GetY(toy),
			id++
			);

		if(gadget[1])
			gadget[1]->AddTooltip("Object Button");

		RefreshGUI(RMG_RGUI_SLIDER_BUTTON);
	}

	return id;
}

void RMGOBJ_Slider_Horz::Execute()
{
	Output();
}

void RMGOBJ_Slider_Horz::Clicked(Edit_RMG *rmg,guiGadget *g)
{
	if(g==gadget[0]) // Slider
	{
		value=(to-g->GetPos()*multi)+offset;

		Execute();
		RefreshGUI(RMG_RGUI_SLIDER_BUTTON);
	}
}
