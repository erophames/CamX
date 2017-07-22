#include "defines.h"
#include "editdata.h"
#include "seqtime.h"
#include "object_song.h"
#include "audiohardware.h"
#include "semapores.h"
#include "songmain.h"
#include "gui.h"
#include "languagefiles.h"
#include "MIDIoutdevice.h"
#include "object_project.h"
#include "audioobjects.h"
#include "arrangeeditor.h"

#define EDITDATAGADGETID_START GADGET_ID_START+50

#define EDIT_DATA_SENDDELAY 70

enum GIDS
{
	GADGET_SLIDER=EDITDATAGADGETID_START+10,
	GADGET_INTEGER,
	GADGET_BOX,
	GADGET_BUTTON,
	GADGET_STRING,
	GADGET_DOUBLE,

	GADGETID_SIGNATURENUMERATOR,
	GADGETID_SIGNATUREDENUMERATOR,

	GADGET_TIMEOK,
	GADGET_TIMECANCEL,

	GADGET_DEVICENAME,
	GADGET_PROGRAMNAME,
	GADGET_PROGRAMCHANNEL,
	GADGET_PROGRAMBANK,
	GADGET_PROGRAMPROGRAM,
	GADGET_OK,

	GADGETID_TIMEMEASURE,
	GADGETID_TIMETYPE,
	GADGETID_TIMEOK,
	GADGETID_TIMECANCEL,
	GADGETID_PATTERNC
};

#define SHOWTIME_TIME 1
#define SHOWTIME_SMPTE 2

#define SHOWTIME_ALL (SHOWTIME_TIME|SHOWTIME_SMPTE)

static char *measurestring[]=
{
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"-",
	"+"
};

static char *tickstring[]=
{
	"/1",
	"/2",
	"/4",
	"/8",
	"/16"
};

static int bticks[]=
{
	SAMPLESPERBEAT*4,
	SAMPLESPERBEAT*2,
	SAMPLESPERBEAT,
	SAMPLESPERBEAT/2,
	SAMPLESPERBEAT/4
};

EditData::EditData()
{
	type=0;
	title=0;
	win=0;
	parentdb=0;
	id=0;
	onoff=false;
	onoffstatus=false;
	commas=3;
	song=0;
	deletename=false;
	newstring=string=0;
	stringlen=0;
	helpobject=0;
	deletesigifcancel=false;
	checksongrealtimeposition=false;
	smpteflag=Seq_Pos::POSMODE_SMPTE_24;
	nosmpte=false;
	onoffstring=0;
	nostring=false;
	noOSborder=false;

	doubledigits=3;
	width=-1;
}

EditData::~EditData()
{
	if(deletename && title)
		delete title;
}

void Edit_Data::MouseWheel(int delta,guiGadget *db)
{
	switch(editdata->type)
	{
	case EditData::EDITDATA_TYPE_VOLUMEDBNOADD:
	case EditData::EDITDATA_TYPE_VOLUMEDB:
		if(measureslider)
		{

		}
		break;
	}
}

void Edit_Data::MouseButton(int flag)
{
	/*
	if(right_mousekey==MOUSEKEY_DOWN) // Reset
	{
	if(editdata && editdata->win)
	{
	editdata->newvalue=editdata->value;
	editdata->win->EditDataMessage(editdata);
	}

	CloseWindow(false);
	}
	*/
}

void Edit_Data::DeActivated()
{
	switch(editdata->type)
	{
	case EditData::EDITDATA_TYPE_COPYMOVEPATTERN:
	case EditData::EDITDATA_TYPE_PLUGINPROGRAMS:
		break;

	default:
		closeit=true;
		break;
	}
}

guiMenu *Edit_Data::CreateMenu()
{
	if(menu)
		menu->RemoveMenu();

	switch(editdata->type)
	{
	case EditData::EDITDATA_TYPE_PLUGINPROGRAMS:
		{
			if(!editdata->title)
			{
				char help[32];

				editdata->title=mainvar->GenerateString(editdata->insertaudioeffect->audioeffect->GetEffectName()," P:",mainvar->ConvertIntToChar(editdata->insertaudioeffect->audioeffect->numberofprograms,help));
				editdata->deletename=true;
			}
		}
		break;
	}

	if(menu=new guiMenu)
	{
		if(editdata && editdata->title)
			menu->AddMenu(editdata->title,0);
		else
			menu->AddMenu("Edit Data",0);
	}

	return menu;
}

void Edit_Data::ShowTime_SMPTE(EditData_SMPTE *data)
{
	for(int i=0;i<5;i++)
	{
		if(smpte[i])
		{
			switch(i)
			{
			case 0:
				smpte[i]->SetInteger(data->hour);
				break;

			case 1:
				smpte[i]->SetInteger(data->min);
				break;

			case 2:
				smpte[i]->SetInteger(data->sec);
				break;

			case 3:
				smpte[i]->SetInteger(data->frame);
				break;

			case 4:
				smpte[i]->SetInteger(data->qframe);
				break;
			}
		}
	}
}

void Edit_Data::ShowTime(int flag,OSTART time)
{
	if(editdata->song)
	{
		Seq_Pos pos(Seq_Pos::POSMODE_NORMAL);

		editdata->song->timetrack.ConvertTicksToPos(time,&pos);

		Seq_Pos spos(editdata->smpteflag);

		editdata->song->timetrack.ConvertTicksToPos(time,&spos);

		for(int i=0;i<4;i++)
		{
			if(flag&SHOWTIME_TIME)
			{
				if(measure[i] && dontshow!=measure[i])
					measure[i]->SetInteger(i==3?pos.GetPos3(editdata->song):pos.pos[i]);
			}

			if(flag&SHOWTIME_SMPTE)
			{
				if(smpte[i] && dontshow!=smpte[i])
					smpte[i]->SetInteger(spos.pos[i]);
			}
		}

		if(flag&SHOWTIME_SMPTE) // q frame
		{
			if(smpte[4] && dontshow!=smpte[4])
				smpte[4]->SetInteger(spos.pos[4]);
		}
	}
}

void Edit_Data::Gadget(guiGadget *g)
{
	if(editdata && editdata->win)
	{
		switch(editdata->type)
		{
		case EditData::EDITDATA_TYPE_COPYMOVEPATTERN:
			{
				switch(g->gadgetID)
				{
				case GADGETID_TIMETYPE:
					if(g->index==0)
					{
						editdata->id=EDIT_SMOVEPATTERN;
					}
					else
						editdata->id=EDIT_SCOPYPATTERN;
					break;

				case GADGETID_TIMEOK:
					{
						editdata->newvalue=timegadget->t_time;
						editdata->win->EditDataValue(editdata);
						maingui->CloseWindow(this);
					}
					break;

				case GADGETID_TIMECANCEL:
					maingui->CloseWindow(this);
					break;
				}

				return;
			}
			break;

		case EditData::EDITDATA_TYPE_PLUGINPROGRAMS:
			{
				editdata->insertaudioeffect->audioeffect->SetProgram(g->index);
			}
			break;

		case EditData::EDITDATA_TYPE_SIGNATURE:
			{	
				Seq_Song *song=editdata->win->WindowSong();

				if(song && editdata->signature)
					switch(g->gadgetID)
				{
					case GADGETID_SIGNATURENUMERATOR:
						{
							DeletePopUpMenu(true);

							if(popmenu)
							{
								class menu_numerator:public guiMenu
								{
								public:
									menu_numerator(Seq_Song *s,Seq_Signature *sg,int n){song=s;sign=sg;num=n;}

									void MenuFunction()
									{
										song->timetrack.SetNumerator(sign,num);
										maingui->RefreshSignature(song);
									}

									Seq_Song *song;
									Seq_Signature *sign;
									int num;
								};

								char nr[NUMBERSTRINGLEN];

								for(int i=1;i<24;i++)
									popmenu->AddFMenu(mainvar->ConvertIntToChar(i,nr),new menu_numerator(song,editdata->signature,i),editdata->signature->nn==i?true:false);

								ShowPopMenu();
							}
						}
						break;

					case GADGETID_SIGNATUREDENUMERATOR:
						{
							DeletePopUpMenu(true);

							if(popmenu)
							{
								class menu_denumerator:public guiMenu
								{
								public:
									menu_denumerator(Seq_Song *s,Seq_Signature *sg,OSTART dn){song=s;sign=sg;dnum=dn;}

									void MenuFunction()
									{
										song->timetrack.SetDeNumerator(sign,dnum);
										maingui->RefreshSignature(song);
									}

									Seq_Song *song;
									Seq_Signature *sign;
									OSTART dnum;
								};

								popmenu->AddFMenu("2",new menu_denumerator(song,editdata->signature,TICK2nd),editdata->signature->dn_ticks==TICK2nd?true:false);
								popmenu->AddFMenu("4",new menu_denumerator(song,editdata->signature,TICK4nd),editdata->signature->dn_ticks==TICK4nd?true:false);
								popmenu->AddFMenu("8",new menu_denumerator(song,editdata->signature,TICK8nd),editdata->signature->dn_ticks==TICK8nd?true:false);
								popmenu->AddFMenu("16",new menu_denumerator(song,editdata->signature,TICK16nd),editdata->signature->dn_ticks==TICK16nd?true:false);

								ShowPopMenu();
							}
						}
				}
				break;

#ifdef OLDIE
				if(g==sigok)
				{
					//maingui->SetActiveWindow(editdata->win,false);
					maingui->CloseWindow(this);
				}
				else
					if(g==sigcancel)
					{
						if(editdata->deletesigifcancel==true)
						{
							mainthreadcontrol->LockActiveSong();
							editdata->song->timetrack.RemoveSignature(editdata->signature);
							mainthreadcontrol->UnlockActiveSong();

							maingui->RefreshAllEditors(editdata->song,REFRESHSIGNATURE_DISPLAY);
							//maingui->RefreshTimeSlider(editdata->song);
						}
						else
						{
							if(backup.Compare(editdata->signature)==false)
							{
								backup.CopyData(editdata->signature);
								maingui->RefreshAllEditors(editdata->song,REFRESHSIGNATURE_DISPLAY);
								//maingui->RefreshTimeSlider(editdata->song);
							}
						}

						//maingui->SetActiveWindow(editdata->win,false);
						maingui->CloseWindow(this);
					}
					else
					{
						for(int i=0;i<9;i++)
						{
							if(g==m[i])
							{
								if(editdata->signature->nn!=i+1)
								{
									mainthreadcontrol->LockActiveSong();

									editdata->signature->ChangeSignature
										(editdata->signature->GetSignatureStart(),
										i+1,
										editdata->signature->dn_ticks);

									mainthreadcontrol->UnlockActiveSong();

									char text[NUMBERSTRINGLEN];

									text[0]=0;

									mainvar->AddString(text,mainvar->ConvertIntToChar(editdata->signature->nn,text));

									if(mtext)
										mtext->ChangeButtonText(text);

									maingui->RefreshAllEditors(editdata->song,REFRESHSIGNATURE_DISPLAY);
								}
								break;
							}
						}

						for(int i=0;i<5;i++)
						{
							if(g==b[i])
							{
								if(editdata->signature->dn_ticks!=bticks[i])
								{
									mainthreadcontrol->LockActiveSong();

									editdata->signature->ChangeSignature
										(editdata->signature->GetSignatureStart(),
										editdata->signature->nn,
										bticks[i]);

									mainthreadcontrol->UnlockActiveSong();

									if(btext)
									{

										btext->ChangeButtonText(editdata->signature->GetTickString());
									}

									maingui->RefreshAllEditors(editdata->song,REFRESHSIGNATURE_DISPLAY);
								}
								break;
							}
						}
					}
#endif

					return;
			}
			break;
		}

		switch(g->gadgetID)
		{
		case GADGET_DEVICENAME:
			{
#ifdef OLDIE
				if(strcmp(g->string,editdata->devicename)!=0)
				{
					MIDIOutputProgram *mop=(MIDIOutputProgram *)editdata->helpobject;

					if(strlen(g->string)>31)
					{
						strncpy(mop->device,g->string,31);
						mop->device[31]=0;
					}
					else
						strcpy(mop->device,g->string);

					/*
					strcpy(editdata->devicename,g->string);
					editdata->id=EditData::EDIT_DEVICENAME;
					editdata->win->EditDataMessage(editdata);
					*/
				}
#endif
			}
			break;

		case GADGET_PROGRAMNAME:
			{
#ifdef OLDIE
				if(strcmp(g->string,editdata->programname)!=0)
				{
					MIDIOutputProgram *mop=(MIDIOutputProgram *)editdata->helpobject;

					if(strlen(g->string)>31)
					{
						strncpy(mop->info,g->string,31);
						mop->info[31]=0;
					}
					else
						strcpy(mop->info,g->string);

					/*
					strcpy(editdata->programname,g->string);
					editdata->id=EditData::EDIT_PROGRAMNAME;
					editdata->win->EditDataMessage(editdata);
					*/
				}
#endif
			}
			break;

		case GADGET_PROGRAMCHANNEL:
			{
#ifdef OLDIE
				MIDIOutputProgram *mop=(MIDIOutputProgram *)editdata->helpobject;

				if(mop)
				{
					if(g->index>=1 && g->index<=16)
					{
						if(g->index!=mop->MIDIChannel)
						{
							mop->MIDIChannel=g->index;
							mop->set=true;
						}
					}
				}
#endif
			}
			break;

		case GADGET_PROGRAMBANK:
			{
#ifdef OLDIE
				MIDIOutputProgram *mop=(MIDIOutputProgram *)editdata->helpobject;

				if(mop)
				{
					if(g->index>=1 && g->index<=128)
					{
						if(g->index-1!=mop->MIDIBank)
						{
							mop->MIDIBank=g->index-1;
							mop->set=true;
						}
					}
				}
#endif
			}
			break;

		case GADGET_PROGRAMPROGRAM:
			{
#ifdef OLDIE
				MIDIOutputProgram *mop=(MIDIOutputProgram *)editdata->helpobject;

				if(mop)
				{
					if(g->index>=1 && g->index<=128)
					{
						if(g->index!=mop->MIDIProgram)
						{
							mop->MIDIProgram=g->index-1;
							mop->set=true;
						}
					}
				}
#endif
			}
			break;

		case GADGET_STRING:
			{
				switch(editdata->type)
				{
				case EditData::EDITDATA_TYPE_KEYS:
					{
						int nv=maingui->KeyStringToByte(editdata->song,g->string);

						if(nv>=0)
							TRACE ("S1 %s %s\n",g->string,maingui->ByteToKeyString(editdata->song,nv));

						if(nv>=editdata->from && nv<=editdata->to && nv!=editdata->newvalue)
						{
							editdata->newvalue=nv;

							if(editdata->id==-1)
							{
								if(guiGadget_Number *nr=(guiGadget_Number *)editdata->helpobject)
									nr->vnumber=editdata->newvalue;
							}

							editdata->win->EditDataValue(editdata);
						}
					}
					break;

				default:
					editdata->newstring=g->string;
					editdata->win->EditDataMessage(editdata);
					break;
				}	
			}
			break;

		case GADGET_BOX:
			{
				editdata->onoffstatus=g->GetBoxStatus();
				editdata->win->EditDataMessage(editdata);
			}
			break;

		case GADGET_DOUBLE:
			{
				switch(editdata->type)
				{
				case EditData::EDITDATA_TYPE_DOUBLE:
					if(g->string)
					{
						{ // 4,5 -> 4.5
							size_t sl=strlen(g->string);
							char *st=g->string;
							while(sl--)
							{
								if((*st)==',')
									*st='.';

								st++;
							}
						}

						editdata->dnewvalue=mainvar->ConvertCharToDouble(g->string,editdata->doubledigits);

						if(editdata->dfrom>editdata->dnewvalue)
						{
							editdata->dnewvalue=editdata->dfrom;
						}

						if(editdata->dto<editdata->dnewvalue)
							editdata->dnewvalue=editdata->dto;

						if(editdata->id==-2)
						{
							if(guiGadget_Volume *vol=(guiGadget_Volume *)editdata->helpobject)
							{
								vol->volume=mainaudio->ConvertDBToLogArrayFactor(editdata->dnewvalue);
							}
						}
						else
							if(editdata->id==-1)
							{
								if(guiGadget_Number *nr=(guiGadget_Number *)editdata->helpobject)
									nr->vnumber=editdata->dnewvalue;
							}

							if(editdata->type!=EditData::EDITDATA_TYPE_INTEGER_OKCANCEL)
							{
								SendDelay(EDIT_DATA_SENDDELAY,0);
							}
					}
					break;
				}
			}
			break;

		case GADGET_INTEGER:
			{
				switch(editdata->type)
				{
				case EditData::EDITDATA_TYPE_VOLUMEDBNOADD:
				case EditData::EDITDATA_TYPE_VOLUMEDB:
					{
						editdata->volume=1;
						editdata->win->EditDataMessage(editdata);

						// Slider
						if(measureslider)
						{
							if(editdata->type==EditData::EDITDATA_TYPE_VOLUMEDB)
								measureslider->ChangeSlider(LOGVOLUME_SIZE-mainaudio->ConvertLogArrayVolumeToInt(1));
							else
								measureslider->ChangeSlider(0);
						}

						if(smpteslider) // Text
						{
							if(char  *h=mainaudio->GenerateDBString(1))
							{
								smpteslider->ChangeButtonText(h);
								delete h;
							}
						}
					}
					break;

				case EditData::EDITDATA_TYPE_SMPTEONLY:
					{
						EditData_SMPTE *s=(EditData_SMPTE *)editdata;

						if(g==smpte[0])
							s->hour=g->index;

						if(g==smpte[1])
							s->min=g->index;

						if(g==smpte[2])
							s->sec=g->index;

						if(g==smpte[3])
							s->frame=g->index;

						if(g==smpte[4])
							s->qframe=g->index;

						editdata->win->EditDataMessage(editdata);
					}
					break;

				case EditData::EDITDATA_TYPE_INTEGER:
				case EditData::EDITDATA_TYPE_INTEGER_OKCANCEL:
					{
						guiGadget_Integer *nr=(guiGadget_Integer *)g;

						editdata->newvalue=g->index;

						if(editdata->from>editdata->newvalue)
							editdata->newvalue=editdata->from;
						else

							if(editdata->to<editdata->newvalue)
								editdata->newvalue=editdata->to;

						if(slider)slider->ChangeSlider(editdata->newvalue);

						if(editdata->id==-1)
						{
							SendDelay(EDIT_DATA_SENDDELAY,(guiGadget *)editdata->helpobject);
						}
						else
							SendDelay(EDIT_DATA_SENDDELAY,0);
					}
					break;

					/*
					case EditData::EDITDATA_TYPE_DOUBLE:
					{
					editdata->dnewvalue=mainvar->ConvertCharToDouble(g->string);

					if(editdata->dfrom>editdata->dnewvalue)
					{
					editdata->dnewvalue=editdata->dfrom;

					if(integer)
					{
					char tempostring[NUMBERSTRINGLEN];
					mainvar->ConvertDoubleToChar(editdata->dnewvalue,tempostring,3);

					integer->SetString(tempostring);
					}
					}

					if(editdata->dto<editdata->dnewvalue)
					{
					editdata->dnewvalue=editdata->dto;

					if(integer)
					{
					char tempostring[NUMBERSTRINGLEN];
					mainvar->ConvertDoubleToChar(editdata->dnewvalue,tempostring,6);

					integer->SetString(tempostring);
					}
					}

					editdata->win->EditDataMessage(editdata);

					if(slider)
					slider->ChangeSlider((int)editdata->dnewvalue);
					}
					break;
					*/


				case EditData::EDITDATA_TYPE_TIME:
					{
						Seq_Pos pos(Seq_Pos::POSMODE_NORMAL);

						bool convert=false,error=false;
						int m=g->index;

						editdata->song->timetrack.ConvertTicksToPos(editdata->time,&pos);

						// Measer 1-1-1-1
						if(g==measure[0])
						{	
							if(m>0 && m<editdata->song->GetSongLength_Measure() && m!=pos.pos[0])
							{
								pos.pos[0]=m;
								pos.pos[1]=1;
								pos.pos[2]=1;
								pos.pos[3]=1;

								dontshow=g;
								convert=true;
							}

							if(m<1 || m>editdata->song->GetSongLength_Measure()){convert=true;error=true;}
						}

						if(g==measure[1])
						{
							Seq_Signature *sig=editdata->song->timetrack.FindSignatureBefore(editdata->time);

							if(m>0 && m<sig->nn && m!=pos.pos[1])
							{
								pos.pos[1]=m;
								pos.pos[2]=1;
								pos.pos[3]=1;

								dontshow=g;
								convert=true;
							}

							if(m<1 || m>sig->nn)
							{
								convert=true;error=true;
							}
						}

						// Zoom
						if(g==measure[2])
						{
							int zoomfaktor;

							Seq_Signature *sig=editdata->song->timetrack.FindSignatureBefore(editdata->newvalue);

							zoomfaktor=sig->dn_ticks/editdata->song->timetrack.zoomticks;

							if(m>0 && m<zoomfaktor && m!=pos.pos[2])
							{
								pos.pos[2]=m;
								pos.pos[3]=1;
								dontshow=g;
								convert=true;
							}

							if(m<1 || m>zoomfaktor){convert=true;error=true;}
						}

						// Ticks
						if(g==measure[3])
						{
							int maxzoom=pos.nozoom==true?pos.sig->dn_ticks:pos.zoomticks;

							if(m>0 && m<maxzoom && pos.pos[3]!=m)
							{
								pos.pos[3]=m;
								dontshow=g;
								convert=true;
							}

							if(m<1 || m>maxzoom){convert=true;error=true;}
						}

						if(convert==true)
						{
							editdata->newvalue=editdata->time=editdata->song->timetrack.ConvertPosToTicks(&pos);

							if(error==false)
							{
								if(measureslider)measureslider->ChangeSlider(m);
								editdata->win->EditDataMessage(editdata);
							}
							else
								dontshow=0;

							ShowTime(SHOWTIME_TIME,editdata->newvalue);
						}

						// SMPTE ****************************************
						Seq_Pos smptepos(editdata->smpteflag);

						convert=false;

						if(m>=0)
						{
							editdata->song->timetrack.ConvertTicksToPos(editdata->time,&smptepos);

							if(g==smpte[0]) // h
							{
								if(m<16 && m!=smptepos.pos[0])
								{
									smptepos.pos[0]=m;
									convert=true;
								}
							}

							if(g==smpte[1]) // min
							{
								if(m<60 && smptepos.pos[1]!=m)
								{
									smptepos.pos[1]=m;
									convert=true;
								}
							}

							if(g==smpte[2]) // sec
							{
								if(m<60 && smptepos.pos[2]!=m)
								{
									smptepos.pos[2]=m;
									convert=true;
								}
							}

							if(g==smpte[3]) // frame
							{
								bool ok=false;

								switch(editdata->smpteflag)
								{
								case Seq_Pos::POSMODE_SMPTE_24:
									if(m<24)ok=true;
									break;

								case Seq_Pos::POSMODE_SMPTE_25:
									if(m<25)ok=true;
									break;

								case Seq_Pos::POSMODE_SMPTE_48:
									if(m<48)ok=true;
									break;

								case Seq_Pos::POSMODE_SMPTE_50:
									if(m<50)ok=true;
									break;

								case Seq_Pos::POSMODE_SMPTE_2997:
									if(m<30)ok=true;
									break;

								case Seq_Pos::POSMODE_SMPTE_30:
									if(m<30)ok=true;
									break;	
								}

								if(ok==true && smptepos.pos[3]!=m)
								{
									smptepos.pos[3]=m;
									convert=true;
								}
							}

							if(g==smpte[4] && m<4) // sub frame
							{	
								if(smptepos.pos[4]!=m)
								{
									convert=true;
									smptepos.pos[4]=m;
								}
							}

							if(convert==true)
							{
								OSTART ticks=editdata->song->timetrack.ConvertPosToTicks(&smptepos);

								if(ticks>=0 && ticks!=editdata->time)
								{
									editdata->newvalue=editdata->time=ticks;
									editdata->win->EditDataMessage(editdata);

									ShowTime(SHOWTIME_TIME,editdata->newvalue);
								}
							}

							dontshow=0;
						}
					}
					break;
				}
			}
			break;

		case GADGET_SLIDER:
			{
				switch(editdata->type)
				{
				case EditData::EDITDATA_TYPE_VOLUMEDBNOADD:
				case EditData::EDITDATA_TYPE_VOLUMEDB:
					{
						double nv;

						if(editdata->type==EditData::EDITDATA_TYPE_VOLUMEDB)
						{
							nv=logvolume[LOGVOLUME_SIZE-g->GetPos()];
						}
						else
						{
							int vol1=mainaudio->ConvertLogArrayVolumeToIntNoAdd(1);
							nv=logvolume[vol1-g->GetPos()];
						}

						if(nv!=editdata->volume)
						{
							if(smpteslider) // Text
							{
								if(char  *h=mainaudio->GenerateDBString(nv))
								{
									smpteslider->ChangeButtonText(h);
									delete h;
								}
							}

							editdata->volume=nv;
							editdata->win->EditDataMessage(editdata);
						}
					}
					break;

				case EditData::EDITDATA_TYPE_TIME:
					{
						if(g==measureslider)
						{
							editdata->newvalue=editdata->song->timetrack.ConvertMeasureToTicks(g->GetPos());

							ShowTime(SHOWTIME_ALL,editdata->newvalue);
						}

						if(g==smpteslider)
						{

						}

						editdata->win->EditDataMessage(editdata);
					}
					break;

				case EditData::EDITDATA_TYPE_INTEGER_OKCANCEL:
				case EditData::EDITDATA_TYPE_INTEGER:
					{
						editdata->newvalue=g->GetPos();

						if(integer)
						{
							char string[NUMBERSTRINGLEN];
							integer->SetString(	mainvar->ConvertLongLongToChar(editdata->newvalue,string));
						}

						if(editdata->type!=EditData::EDITDATA_TYPE_INTEGER_OKCANCEL)
							editdata->win->EditDataMessage(editdata);
					}
					break;

				case EditData::EDITDATA_TYPE_KEYS:
					{
					}
					break;

				case EditData::EDITDATA_TYPE_DOUBLE:
					{
						double h=g->GetPos(),h2;
						int c=(int)floor(editdata->dnewvalue+0.5);

						h2=editdata->dnewvalue;
						h2-=c;

						h2+=h;

						editdata->dnewvalue=h2;

						if(integer)
						{
							char tempostring[NUMBERSTRINGLEN];
							mainvar->ConvertDoubleToChar(editdata->dnewvalue,tempostring,3);

							integer->SetString(tempostring);
						}

						editdata->win->EditDataMessage(editdata);
					}
					break;
				}
			}
			break;

		case GADGET_OK:
			{
				editdata->win->EditDataMessage(editdata);
				//maingui->SetActiveWindow(editdata->win,false);
				maingui->CloseWindow(this);
			}
			break;

		case GADGET_TIMEOK:
			{
				//maingui->SetActiveWindow(editdata->win,false);
				maingui->CloseWindow(this);
			}
			break;

		case GADGET_TIMECANCEL:
			{
				if(timebuffer!=editdata->newvalue)
				{
					editdata->newvalue=timebuffer;
					editdata->win->EditDataMessage(editdata);
				}

				//maingui->SetActiveWindow(editdata->win,false);
				maingui->CloseWindow(this);
			}
			break;
		}
	}
#ifdef DEBUG
	else
		maingui->MessageBoxError(0,"E Data");
#endif

}

Edit_Data::Edit_Data(EditData *data)
{
	editorid=EDITORTYPE_EDITDATA;

	InitForms(FORM_PLAIN1x1);

	borderless=true;
	dialogstyle=true;
	editdata=data;

	if(data)
	{
		data->newvalue=data->value;
	}

	slider=0;
	integer=0;
	box=0;
	signbackup.map=0; // no refresh
	dontshow=0;
	init=false;
	senddelay=0;
	senddelayGadget=0;

	patterc=0;
};

void Edit_Data::ShowSelectedPattern()
{
	if(patterc)
	{
		char nr[NUMBERSTRINGLEN];
		int selp=editdata->song->GetCountSelectedPattern();

		char *h=mainvar->GenerateString("Pattern:",mainvar->ConvertIntToChar(selp,nr));
		if(h)
		{
			patterc->ChangeButtonText(h);
			delete h;
		}
	}
}

void Edit_Data::RefreshRealtime_Slow()
{
	if(patterc)
	{
		ShowSelectedPattern();
	}
}

void Edit_Data::RefreshRealtime()
{
	/*
	if(IsActiveWindow()==false)
	{

		closeit=true;
	}
*/

	if(senddelay>0)
	{
		senddelay--;

		if(senddelay==0)
		{
			if(senddelayGadget)
			{
				senddelayGadget->ChangeEditData(editdata->newvalue,true);
				senddelayGadget=0;
			}
			else
			{
				if(editdata && editdata->win)
					editdata->win->EditDataValue(editdata);
			}
		}
	}

	//return;

	switch(editdata->type)
	{
	case EditData::EDITDATA_TYPE_SIGNATURE:
		if(editdata->signature)
		{
			if(signbackup.Compare(editdata->signature)==false)
			{
				editdata->signature->CopyData(&signbackup);

				if(signgadget)
				signgadget->ShowSignature(editdata->signature);
			}
		}
		break;

	case EditData::EDITDATA_TYPE_PLUGINPROGRAMS:
		if(editdata->insertaudioeffect->audioeffect->updatedisplay==true)
		{
			UpdateDisplay();
			editdata->insertaudioeffect->audioeffect->updatedisplay=false;
		}
		break;

	case EditData::EDITDATA_TYPE_TIME:
		{
			if(editdata->checksongrealtimeposition==true && editdata->song && editdata->song->GetSongPosition()!=editdata->newvalue)
			{
				editdata->newvalue=editdata->time=editdata->song->GetSongPosition();

				if(measureslider)
					measureslider->ChangeSlider(editdata->song->timetrack.ConvertTicksToMeasure(editdata->newvalue));

				//if(smpteslider)
				//	smpteslider->ChangeSlider(editdata->song->timetrack.ConvertTicksToMeasure(editdata->newvalue));

				ShowTime(SHOWTIME_SMPTE|SHOWTIME_TIME,editdata->newvalue);
			}
		}
		break;
	}
}

bool Edit_Data::CheckIfObjectInside(Object *o)
{
	switch(editdata->type)
	{
	case EditData::EDITDATA_TYPE_PLUGINPROGRAMS:
		{
			if(o==editdata->insertaudioeffect ||
				o==editdata->insertaudioeffect->audioeffect ||
				o==(Object *)editdata->insertaudioeffect->effectlist->channel ||
				o==(Object *)editdata->insertaudioeffect->effectlist->track
				)
				return true;
		}
		break;
	}

	return false;
}

void Edit_Data::UpdateDisplay()
{
	switch(editdata->type)
	{
	case EditData::EDITDATA_TYPE_PLUGINPROGRAMS:
		if(listbox)
		{
			listbox->ClearListBox();

			char help[32];

			for(int i=0;i<editdata->insertaudioeffect->audioeffect->numberofprograms;i++)
			{
				char *pname=editdata->insertaudioeffect->audioeffect->GetProgramNameIndex(i),
					*h2=mainvar->GenerateString(mainvar->ConvertIntToChar(i+1,help),":",pname);

				if(!h2)
					listbox->AddStringToListBox("-?-");
				else
					listbox->AddStringToListBox(h2);

				if(h2)
					delete h2;

				if(pname)
					delete pname;
			}

			listbox->SetListBoxSelection(editdata->insertaudioeffect->audioeffect->GetProgram());
		}
		break;
	}
}

void Edit_Data::SendDelay(int delay,guiGadget *g)
{
	senddelay=delay;
	senddelayGadget=g;

}

void Edit_Data::Init()
{
	glist.SelectForm(0,0);

	switch(editdata->type)
	{
	case EditData::EDITDATA_TYPE_COPYMOVEPATTERN:
		{
			if(guiGadget *gc=glist.AddCycle(-1,-1,-1,-1,GADGETID_TIMETYPE,MODE_RIGHT,0))
			{
				gc->AddStringToCycle(Cxs[CXS_MOVESELPATTERNM]);
				gc->AddStringToCycle(Cxs[CXS_COPYSELPATTERNM]);

				gc->SetCycleSelection(editdata->copyflag==false?0:1);
			}

			glist.Return();

			timegadget=glist.AddTimeButton(-1,-1,bitmap.prefertimebuttonsize,-1,0,GADGETID_TIMEMEASURE,WINDOWDISPLAY_MEASURE,MODE_BLACK|MODE_STATICTIME);
			glist.AddLX();

			patterc=glist.AddButton(-1,-1,-1,-1,0,GADGETID_PATTERNC,MODE_RIGHT);
			glist.Return();
			ShowSelectedPattern();

			glist.AddButton(-1,-1,-1,-1,"OK",GADGETID_TIMEOK,MODE_LEFTTOMID|MODE_TEXTCENTER);
			glist.AddLX();
			glist.AddButton(-1,-1,-1,-1,Cxs[CXS_CANCEL],GADGETID_TIMECANCEL,MODE_MIDTORIGHT|MODE_TEXTCENTER);
		}
		break;

	case EditData::EDITDATA_TYPE_INFOSTRING:
		{
			guiGadget *g=glist.AddButton(-1,-1,editdata->width,-1,editdata->string,GADGET_STRING,0,0);

		}
		break;

	case EditData::EDITDATA_TYPE_STRING:
	case EditData::EDITDATA_TYPE_STRING_TITLE:
		{
			guiGadget *g=glist.AddString(-1,-1,editdata->width,-1,GADGET_STRING,MODE_ACTIVATE,0,editdata->string);

			if(g)
				g->SetKeyFocus();
		}
		break;

	case EditData::EDITDATA_TYPE_INTEGER:
		{
			integer=glist.AddInteger(-1,-1,editdata->width,-1,GADGET_INTEGER,0,editdata->value);

			if(integer)
			{
				integer->SetKeyFocus();
			}
		}
		break;

	case EditData::EDITDATA_TYPE_KEYS:
		{
			string=glist.AddString(-1,-1,editdata->width,-1,GADGET_STRING,0,0,maingui->ByteToKeyString(editdata->song,editdata->value));

			if(integer)
				integer->SetKeyFocus();
		}
		break;

	case EditData::EDITDATA_TYPE_DOUBLE:
		{
			char dstring[NUMBERSTRINGLEN];
			string=glist.AddString(-1,-1,editdata->width,-1,GADGET_DOUBLE,0,0,mainvar->ConvertDoubleToChar(editdata->dvalue,dstring,editdata->doubledigits));

			if(string)
				string->SetKeyFocus();
		}
		break;

	case EditData::EDITDATA_TYPE_SIGNATURE:
		{
			int gid=GADGET_BUTTON;

			// Signature Backup
			editdata->signature->CopyData(&signbackup);

			signgadget=glist.AddSignatureButton(-1,-1,-1,-1,GADGETID_SIGNATURENUMERATOR,GADGETID_SIGNATUREDENUMERATOR,MODE_RIGHT);

			if(signgadget)
				signgadget->ShowSignature(editdata->signature);

			/*
			int w=width;
			int h=(height-3)/3;
			int x,y=0;

			w/=10;

			x=0;
			for(int i=0;i<9;i++)
			{
				m[i]=l->AddButton(x,y,x+w,y+h-1,measurestring[i],gid++,0);
				x+=w;
			}

			int hx=x,hx2=x+w;
			char text[NUMBERSTRINGLEN];

			text[0]=0;
			mainvar->AddString(text,mainvar->ConvertIntToChar(editdata->signature->nn,text));

			//mtext=l->AddText(x,y,hx2,y+h-1,text,0);

			y+=h;

			w=hx;
			w/=5;

			x=0;
			for(int i1=0;i1<5;i1++)
			{
				b[i1]=l->AddButton(x,y,x+w,y+h,tickstring[i1],gid++,0);
				x+=w;
			}
			//btext=l->AddText(hx,y,hx2,y+h,editdata->signature->GetTickString(),0);

			y+=h;
			*/

			//sigok=l->AddButton(0,y,width/2-1,height,"Ok",gid++,0);
			//sigcancel=l->AddButton(width/2,y,width,height,"Cancel",gid,0);
		}
		break;


	case EditData::EDITDATA_TYPE_PLUGINPROGRAMS:
		{
			if(editdata->insertaudioeffect)
			{
				listbox=glist.AddListBox(-1,-1,-1,-1,GADGET_INTEGER,MODE_BOTTOM|MODE_RIGHT);

				if(listbox)
				{
					char help[32];

					for(int i=0;i<editdata->insertaudioeffect->audioeffect->numberofprograms;i++)
					{
						char *pname=editdata->insertaudioeffect->audioeffect->GetProgramNameIndex(i),
							*h2=mainvar->GenerateString(mainvar->ConvertIntToChar(i+1,help),":",pname);

						if(!h2)
							listbox->AddStringToListBox("-?-");
						else
							listbox->AddStringToListBox(h2);

						if(h2)
							delete h2;

						if(pname)
							delete pname;
					}

					listbox->CalcScrollWidth();
					listbox->SetListBoxSelection(editdata->insertaudioeffect->audioeffect->GetProgram());
				}
			}
			else
				listbox=0;
		}
		break;

#ifdef OLDIE
	case EditData::EDITDATA_TYPE_PROGRAM:
		{
			MIDIOutputProgram *mop=(MIDIOutputProgram *)editdata->helpobject;

			if(mop)
			{
				int y=1;

				strcpy(editdata->devicename,mop->device);
				strcpy(editdata->programname,mop->info);

				l->AddString(1,y,width/2-1,y+maingui->GetFontSizeY(),GADGET_DEVICENAME,0,mop->device,"Device Name");
				l->AddString(1+width/2,y,width,y+maingui->GetFontSizeY(),GADGET_PROGRAMNAME,0,mop->info,"Program Name");

				int x2=width/2;
				y+=maingui->GetFontSizeY()+2;
				l->AddInteger(x2,y,width-40,y+maingui->GetFontSizeY(),GADGET_PROGRAMCHANNEL,"MIDI Channel",mop->MIDIChannel,"MIDI Channel (1-16)");
				y+=maingui->GetFontSizeY()+2;
				l->AddInteger(x2,y,width,y+maingui->GetFontSizeY(),GADGET_PROGRAMBANK,"Device Bank",mop->MIDIBank+1,"Device Bank (1-127)");
				y+=maingui->GetFontSizeY()+2;
				l->AddInteger(x2,y,width,y+maingui->GetFontSizeY(),GADGET_PROGRAMPROGRAM,"Device Program",mop->MIDIProgram+1,"Device Program (1-127)");
			}

			// Device Name+Device Program
			// Bank Select+Program Change
		}
		break;



	case EditData::EDITDATA_TYPE_SMPTEONLY:
		{
			int x=0;
			int w=width;
			w/=5;

			for(int i2=0;i2<5;i2++)
			{
				if(smpte[i2]=l->AddInteger(x,0,(x+w)-1,height,GADGET_INTEGER,0,0))
					switch(i2)
				{
					case 0:
						smpte[i2]->AddTooltip("Hour");
						break;

					case 1:
						smpte[i2]->AddTooltip("Minute");
						break;

					case 2:
						smpte[i2]->AddTooltip("Second");
						break;

					case 3:
						smpte[i2]->AddTooltip("Frame");
						break;

					case 4:
						smpte[i2]->AddTooltip("Quarter Frame");
						break;
				}

				x+=w;
			}

			ShowTime_SMPTE((EditData_SMPTE *)editdata);
		}
		break;



	
	case EditData::EDITDATA_TYPE_VOLUMEDBNOADD:
	case EditData::EDITDATA_TYPE_VOLUMEDB:
		{
			int max=editdata->type==EditData::EDITDATA_TYPE_VOLUMEDB?LOGVOLUME_SIZE:mainaudio->ConvertLogArrayVolumeToIntNoAdd(1);

			SliderCo co;

			int y2=height-maingui->GetFontSizeY();

			co.x=0;
			co.y=0;
			co.y2=y2-1;
			co.x2=width;

			co.horz=false;
			co.from=0;
			co.page=4;
			co.to=max; // +1 == 0 pos

			if(editdata->type==EditData::EDITDATA_TYPE_VOLUMEDB)
				co.pos=max-mainaudio->ConvertLogArrayVolumeToInt(editdata->volume);
			else
				co.pos=max-mainaudio->ConvertLogArrayVolumeToIntNoAdd(editdata->volume);

			measureslider=l->AddSlider(&co,GADGET_SLIDER,0);

			char *h=mainaudio->GenerateDBString(editdata->volume);
			smpteslider=l->AddButton(0,y2,width,height,h,GADGET_INTEGER,0,"Volume");

			if(h)
				delete h;
		}
		break;

	case EditData::EDITDATA_TYPE_TIME:
		if(editdata->song)
		{
			int w=width,
				h=height,
				x,y=0;

			if(	editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_24 ||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_25 ||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_2997 ||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_30 ||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_2997df ||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_30df ||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_239 ||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_249 ||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_599||
				editdata->smpteflag==Seq_Pos::POSMODE_SMPTE_60)
				h/=5;
			else
			{
				h/=3;
				editdata->nosmpte=true;
			}

			w/=4;

			x=0;
			for(int i=0;i<4;i++)
			{
				if(i==2)
					switch(editdata->song->project->projectmeasureformat)
				{
					case PM_1111:
					case PM_1110:
					case PM_1p1p1p1:
					case PM_1p1p1p0:
						measure[i]=l->AddInteger(x,y,(x+w)-1,y+h,GADGET_INTEGER,0,0);
						break;

						/*
						default:
						measure[i]=l->AddText(x,y,(x+w)-1,y+h," - ",GADGET_INTEGER,0);
						if(measure[i])measure[i]->Disable();
						break;
						*/
				}
				else
					measure[i]=l->AddInteger(x,y,(x+w)-1,y+h,GADGET_INTEGER,0,0);

				if(measure[i])
					switch(i)
				{
					case 0:
						measure[i]->AddTooltip(Cxs[CXS_MEASURE]);
						break;

					case 1:
						measure[i]->AddTooltip(Cxs[CXS_BEAT]);
						break;

					case 2:
						{
							measure[i]->AddTooltip("Song Zoom (1.1.X.1)");
						}
						break;

					case 3:
						measure[i]->AddTooltip("Ticks");
						break;
				}

				x+=w;
			}

			SliderCo co;

			y+=h;

			co.x=0;
			co.y=y;
			co.y2=(y+h)-1;
			co.x2=width;

			co.from=editdata->song->timetrack.ConvertTicksToMeasure(editdata->from); // Measure 1 ---> songend
			co.to=editdata->song->timetrack.ConvertTicksToMeasure(editdata->to);

			co.horz=true;
			co.pos=editdata->song->timetrack.ConvertTicksToMeasure(editdata->newvalue=editdata->time);

			measureslider=l->AddSlider(&co,GADGET_SLIDER,0);
			if(measureslider)measureslider->AddTooltip(Cxs[CXS_MEASURE]);

			if(editdata->nosmpte==false)
			{
				y+=h;

				x=0;
				w=width;
				w/=5;

				for(int i2=0;i2<5;i2++)
				{
					char *tooltip=0;

					switch(i2)
					{
					case 0:
						tooltip="Hour";
						break;

					case 1:
						tooltip="Minute";
						break;

					case 2:
						tooltip="Second";
						break;

					case 3:
						tooltip="Frame";
						break;

					case 4:
						tooltip="Quarter Frame";
						break;
					}

					smpte[i2]=l->AddInteger(x,y,(x+w)-1,y+h,GADGET_INTEGER,0,0,tooltip);
					x+=w;
				}

				y+=h;

				co.y=y;
				co.y2=(y+h)-1;

				smpteslider=l->AddSlider(&co,GADGET_SLIDER,0);
			}
			else
			{
				for(int i=0;i<5;i++)smpte[i]=0;
				smpteslider=0;
			}

			y+=h;

			oktime=l->AddButton(0,y,width/2-1,height,"OK",GADGET_TIMEOK,0);
			canceltime=l->AddButton(width/2,y,width,height,Cxs[CXS_CANCEL],GADGET_TIMECANCEL,0);

			timebuffer=editdata->time;

			ShowTime(SHOWTIME_ALL,editdata->newvalue);
		}
		break;

	case EditData::EDITDATA_TYPE_DOUBLE:
		{
			SliderCo co;

			// Slider Integer
			double h=width;
			int x=5,x2;

			x2=(int)(h*0.50);

			co.x=x;
			co.y=1;
			co.y2=height;
			co.x2=x2;
			co.from=(int)floor(editdata->dfrom+0.5);
			co.to=(int)floor(editdata->dto+0.5);
			co.horz=true;
			co.pos=(int)floor(editdata->dvalue+0.5);

			editdata->dnewvalue=editdata->dvalue;
			slider=l->AddSlider(&co,GADGET_SLIDER,0);

			/*
			if(integer=l->AddString(x2+1,1,width,height,GADGET_INTEGER,0,0))
			{
			char tempostring[NUMBERSTRINGLEN];

			mainvar->ConvertDoubleToChar(editdata->dvalue,tempostring,3);
			integer->SetString(tempostring);
			}
			*/
		}
		break;

	case EditData::EDITDATA_TYPE_INTEGER_OKCANCEL:
		{
			SliderCo co;

			// Slider Integer
			double h=width;
			int x=1,x2,t2;

			x2=(int)(h*0.55);
			t2=(int)(h*0.85);

			co.x=x;
			co.y=1;
			co.y2=height;
			co.x2=x2;
			co.from=editdata->from;
			co.to=editdata->to;
			co.horz=true;
			co.pos=editdata->value;

			slider=l->AddSlider(&co,GADGET_SLIDER,0);

			/*
			if(integer=l->AddString(x2+1,1,t2,height,GADGET_INTEGER,0,0))
			{
			char string[NUMBERSTRINGLEN];
			integer->SetString(	mainvar->ConvertIntToChar(editdata->value,string));

			l->AddButton(t2+1,1,width,height,"OK",GADGET_OK,0);
			}
			*/
		}
		break;

	case EditData::EDITDATA_TYPE_INTEGER:
		{
			SliderCo co;

			// Slider Integer
			double h=width;
			int x=1,x2,t2;

			if(editdata->onoff==true)
			{
				x2=(int)(h*0.55);
				t2=(int)(h*0.85);
			}
			else
			{
				if(editdata->nostring==true)
					x2=width;
				else
					x2=(int)(h*0.65);

				t2=width;
			}

			co.x=x;
			co.y=1;
			co.y2=height;
			co.x2=x2;
			co.from=editdata->from;
			co.to=editdata->to;
			co.horz=true;
			co.pos=editdata->value;

			slider=l->AddSlider(&co,GADGET_SLIDER,0);

			/*
			if(editdata->nostring==false)
			{
			if(integer=l->AddString(x2+1,1,t2,height,GADGET_INTEGER,0,0))
			{
			char string[NUMBERSTRINGLEN];
			integer->SetString(	mainvar->ConvertIntToChar(editdata->value,string));
			}
			}
			else
			integer=0;
			*/

			if(editdata->onoff==true)
			{
				if(box=l->AddCheckBox(t2+1,1,width,height,GADGET_BOX,0,editdata->onoffstring))
					box->SetCheckBox(editdata->onoffstatus);
			}
		}
		break;
#endif

	}

	init=true;
}


void Edit_Data::DeInitWindow()
{
	if(editdata)
		delete editdata;
}
