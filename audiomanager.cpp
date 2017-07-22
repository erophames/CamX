#include "audiomanager.h"
#include "audiohdfile.h"

#include "audiohardware.h"
#include "audiofile.h"
#include "gui.h"
#include "camxgadgets.h"
#include "songmain.h"
#include "audiorealtime.h"
#include "editbuffer.h"
#include "audiothread.h"
#include "audiofilework.h"
#include "languagefiles.h"
#include "semapores.h"
#include "sampleeditor.h"
#include "globalmenus.h"

#define M_RECORDED 1
#define M_SHOWNOTFOUND 2

// Gadgets
#define AUDIOMANAGERGADGETID_START GADGET_ID_START+50

enum GIDs
{
	GADGET_SAMPLEID,
	GADGETID_STOP,
	GADGETID_PLAY ,
	GADGETID_STOPREGION ,
	GADGETID_PLAYREGION ,
	GADGETID_COPYFILE,
	GADGETID_COPYREGION ,
	GADGET_REGIONID,
	GADGET_INFO,
	GADGET_EDITFILE ,
	GADGET_STATUS ,
	GADGET_SEARCH,
	GADGET_SEARCHBUTTON,
	GADGET_SHOWFILE,
	GADGETID_REGIONS
};

void Edit_Manager::ShowStatus()
{
	if(statusgadget)
	{
	}
}

void Edit_Manager::DeInitWindow()
{
	StopPlayback();

	if(activefile && (mainaudio->FindAudioHDFile(activefile->hdfile)==true))
	{
		mainsettings->SetLastAudioManagerFile(activefile->hdfile->GetName());
		//activefile->hdfile->Close();
	}

	DeleteAllSortFiles();
}

void Edit_Manager::ChangeSortFile(Sort_Files *n,bool refreshgui)
{
	if(n)
	{
		StopPlayback();

		//if(GetActiveHDFile())
		//	GetActiveHDFile()->Close();

		filestartposition=0;

		activefile=n;
		activeregion=n->hdfile->FirstRegion();
		n->hdfile->Open();

		if(refreshgui==true)
		{
			RefreshHDFile(0);
			ShowActiveHDFile_Regions();
			ShowActiveHDFile_Info();

			//if(statusgadget)
			//	statusgadget->ChangeButtonText(" ");
		}
	}
}

void Edit_Manager::CopyActiveFile()
{
	mainaudio->CopyAudioFile(this,GetActiveHDFile());
}

void Edit_Manager::CopyActiveRegion()
{
	if(showregions==true)
		mainaudio->CreateAudioRegionFile(this,GetActiveHDFile(),activeregion);
}


void Edit_Manager::ResamplingFile()
{
	if(activefile && activefile->hdfile->samplerate!=mainaudio->GetGlobalSampleRate())
	{
		if(AudioFileWork_Resample *work=new AudioFileWork_Resample){

			work->Init(activefile->hdfile->GetName()); // org file
			work->newsamplerate=mainaudio->GetGlobalSampleRate();

			audioworkthread->AddWork(work);
		}
	}
}

void Edit_Manager::DecodeFile()
{
	bool ok=false;
	camxFile file;

	if(file.OpenFileRequester(0,this,Cxs[CXS_SELECTENCODEDFILE],file.AllFiles(camxFile::FT_ENCODED),true)==true)
	{
		if(AudioFileWork_Converter *con=new AudioFileWork_Converter){

			con->inputfile=mainvar->GenerateString(file.filereqname);
			con->newsamplerate=mainaudio->GetGlobalSampleRate();

			audioworkthread->AddWork(con);
		}

		//	decoder.file=mainvar->GenerateString(file.filereqname);
		file.Close(true);

		//	convtowave(&decoder);
		//	decoder.Close();
	}
}

void Edit_Manager::DragDropFile(char *file,guiGadget *db)
{
	AddFile(file);
}

Object *Edit_Manager::GetDragDrop(HWND wnd,int index)
{
	if(soundsgadget && soundsgadget->hWnd==wnd)
	{
		if(Sort_Files *sf=GetSortFileIndex(index))
			return sf->hdfile;

		return 0;
	}

	/*
	if(regionsgadget && regionsgadget->hWnd==wnd)
	{
		if(activefile)
			return activefile->hdfile->GetRegionIndex(index);

		return 0;
	}
*/

	return 0;
}

void Edit_Manager::AddFile(char *file)
{
	if(!file)
		return;

	if(AudioHDFile *find=mainaudio->FindAudioHDFile(file))
	{
		SetActiveFile(FindAudioHDFile(find),true);
	}
	else
		if(mainaudio->CheckIfAudioFile(file))
		{
			if(AudioHDFile *nhd=mainaudio->AddAudioFile(file,true))
			{
				InitList();
				SetActiveFile(FindAudioHDFile(nhd),false);
				ShowAudioFiles();
				//mainaudio->SaveDataBase();
			}
		}
}

void Edit_Manager::AddFile()
{
	camxFile addfile;

	if(addfile.OpenFileRequester(0,this,Cxs[CXS_LOADWAVEFILE],addfile.AllFiles(camxFile::FT_WAVES),true)==true)
	{
		AddFile(addfile.filereqname);
	}
}

void Edit_Manager::CreateWindowMenu()
{
	if(NewWindowMenu())
	{
		guiMenu *n=menu->AddMenu(Cxs[CXS_FILE],0);

		if(n)
		{
			class menu_loaddatabase:public guiMenu
			{
			};
			class menu_savedatabase:public guiMenu
			{
			};

			n->AddFMenu(("Load/Add Audio Library"),new menu_loaddatabase());
			n->AddFMenu(("Save Audio Library"),new menu_savedatabase());
			n->AddLine();
			class menu_addfile:public guiMenu
			{
			public:
				menu_addfile(Edit_Manager *e){editor=e;}

				void MenuFunction()
				{
					editor->AddFile();
				} //

				Edit_Manager *editor;
			};

			n->AddFMenu(Cxs[CXS_ADDAUDIOFILE],new menu_addfile(this));
			n->AddLine();

			class menu_adddir:public guiMenu
			{
			public:
				menu_adddir(Edit_Manager *m){manager=m;}

				void MenuFunction()
				{
					manager->AddFilesFromDirectory();
				} //

				Edit_Manager *manager;
			};

			n->AddFMenu(Cxs[CXS_ADDFILESFROMDIR],new menu_adddir(this));

			class menu_search:public guiMenu
			{
			public:
				menu_search(Edit_Manager *m){manager=m;}

				void MenuFunction()
				{
					mainaudio->FindNotFoundFiles(manager,true);
				} //

				Edit_Manager *manager;
			};
			n->AddFMenu(Cxs[CXS_SEARCHFORFILENOTEFOUND],new menu_search(this));

			class menu_removeff:public guiMenu
			{
			public:
				menu_removeff(Edit_Manager *m){manager=m;}

				void MenuFunction()
				{
					mainaudio->RemoveFileNotFound();
				} //

				Edit_Manager *manager;
			};
			n->AddFMenu(Cxs[CXS_REMOVEALLFILENOTEFOUND],new menu_removeff(this));

			class menu_copyff:public guiMenu
			{
			public:
				menu_copyff(Edit_Manager *m){manager=m;}

				void MenuFunction()
				{
					manager->CopyActiveFile();
				} //

				Edit_Manager *manager;
			};
			n->AddFMenu(Cxs[CXS_EXSAVEAFILE],new menu_copyff(this));

			class menu_copyre:public guiMenu
			{
			public:
				menu_copyre(Edit_Manager *m){manager=m;}

				void MenuFunction()
				{
					manager->CopyActiveRegion();
				} //

				Edit_Manager *manager;
			};
			n->AddFMenu(Cxs[CXS_EXSAVEREGION],new menu_copyre(this));
		}

		n=menu->AddMenu(Cxs[CXS_EDIT],0);

		if(n)
		{
			class menu_editsample:public guiMenu
			{
			public:
				menu_editsample(Edit_Manager *ed,bool r){editor=ed;region=r;}

				void MenuFunction()
				{
					editor->EditActiveFile(region);
				} //

				Edit_Manager *editor;
				bool region;
			};

			n->AddFMenu(Cxs[CXS_OPENAUDIOEDITOR],new menu_editsample(this,false));

			if(char *h=mainvar->GenerateString(Cxs[CXS_OPENAUDIOEDITOR],"(Region)"))
			{
				n->AddFMenu(h,new menu_editsample(this,true));
				delete h;
			}

			n->AddLine();

			class menu_createaudiobuffer:public guiMenu
			{
			public:
				menu_createaudiobuffer(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->CopyHDFileToBuffer();
				} //

				Edit_Manager *editor;
			};

			n->AddFMenu(Cxs[CXS_COPY],new menu_createaudiobuffer(this),SK_COPY);

			class menu_createaudioregion:public guiMenu
			{
			public:
				menu_createaudioregion(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->CopyAudioRegionToBuffer();
				} //

				Edit_Manager *editor;
			};

			n->AddFMenu(Cxs[CXS_COPYREGION],new menu_createaudioregion(this),"Ctrl+R");
		}

		n=menu->AddMenu(Cxs[CXS_FUNCTIONS],0);
		if(n)
		{
			class menu_decodefile:public guiMenu
			{
			public:
				menu_decodefile(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->DecodeFile();
				} //

				Edit_Manager *editor;
			};

			class menu_resamplefile:public guiMenu
			{
			public:
				menu_resamplefile(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->ResamplingFile();
				} //

				Edit_Manager *editor;
			};

			n->AddFMenu(Cxs[CXS_DECODEFILE],new menu_decodefile(this));
			n->AddFMenu(Cxs[CXS_CONVERTSAMPLERATE],new menu_resamplefile(this));
		}

		if(n=menu->AddMenu("Editor",0))
		{
			n->AddFMenu("Mixer",new globmenu_AudioMixer(0,0,openscreen),"F7");
		}

		if(n=menu->AddMenu(Cxs[CXS_DISPLAY],0))
		{
			class menu_mshowregions:public guiMenu
			{
			public:
				menu_mshowregions(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->showregions=editor->showregions==true?false:true;
					mainsettings->manager_showregions=editor->showregions;
				
				} //

				Edit_Manager *editor;
			};
			n->AddFMenu(Cxs[CXS_SHOWREGIONS],menu_showregions=new menu_mshowregions(this),showregions);

			class menu_mshowinfo:public guiMenu
			{
			public:
				menu_mshowinfo(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->showinfo=editor->showinfo==true?false:true;
					mainsettings->manager_showinfo=editor->showinfo;
					
				} //

				Edit_Manager *editor;
			};
			n->AddFMenu(Cxs[CXS_SHOWFILEINFO],menu_showinfo=new menu_mshowinfo(this),showinfo);

			class menu_mshowintern:public guiMenu
			{
			public:
				menu_mshowintern(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->showintern=editor->showintern==true?false:true;
					mainsettings->manager_showintern=editor->showintern;
					
				} //

				Edit_Manager *editor;
			};
			TRACE ("%s\n",Cxs[CXS_SHOWFILEINFOINTERN]);

			n->AddFMenu(Cxs[CXS_SHOWFILEINFOINTERN],menu_showintern=new menu_mshowintern(this),showintern);

			//n->AddFMenu(Cxs[CXS_SHOWFILEINFO],menu_showintern=new menu_mshowintern(this),showintern);

			n->AddLine();

			class menu_mshowsr:public guiMenu
			{
			public:
				menu_mshowsr(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->showonlysamplerate=editor->showonlysamplerate==true?false:true;
					mainsettings->manager_showonlysamplerate=editor->showonlysamplerate;

					editor->InitList();
					editor->ShowAudioFiles();
					editor->ShowMenu();
				} //

				Edit_Manager *editor;
			};
			n->AddFMenu(Cxs[CXS_SHOWALLSAMPLERATES],menu_showsr=new menu_mshowsr(this),showonlysamplerate);
			n->AddLine();

			class menu_showfullpath:public guiMenu
			{
			public:
				menu_showfullpath(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->showfullpath=editor->showfullpath==true?false:true;
					mainsettings->manager_showfullpath=editor->showfullpath;

					editor->InitList();
					editor->ShowAudioFiles();
					editor->ShowMenu();
				} //

				Edit_Manager *editor;
			};
			n->AddFMenu(Cxs[CXS_SHOWFILEPATH],menu_filepath=new menu_showfullpath(this),showfullpath);

			class menu_mb:public guiMenu
			{
			public:
				menu_mb(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->showmb=editor->showmb==true?false:true;
					mainsettings->manager_showmb=editor->showmb;
					editor->ShowAudioFiles();
					editor->ShowMenu();
				} //

				Edit_Manager *editor;
			};

			n->AddFMenu(Cxs[CXS_SHOWFILESIZE],menu_showmb=new menu_mb(this),showmb);

			class menu_stime:public guiMenu
			{
			public:
				menu_stime(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->showtime=editor->showtime==true?false:true;
					mainsettings->manager_showtime=editor->showtime;
					editor->ShowAudioFiles();
					editor->ShowMenu();
				} //

				Edit_Manager *editor;
			};

			n->AddFMenu(Cxs[CXS_SHOWFILETIME],menu_showtime=new menu_stime(this),showtime);

			class menu_srec:public guiMenu
			{
			public:
				menu_srec(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->showrecorded=editor->showrecorded==true?false:true;

					mainsettings->manager_showrecorded=editor->showrecorded;

					if(editor->CheckForFile(M_RECORDED)==true)
					{
						editor->InitList();
						editor->ShowAudioFiles();
					}

					editor->ShowMenu();
				} //

				Edit_Manager *editor;
			};

			n->AddFMenu(Cxs[CXS_SHOWCAMXRECORDEDFILES],menu_showrecorded=new menu_srec(this),showrecorded);

			class menu_snotf:public guiMenu
			{
			public:
				menu_snotf(Edit_Manager *ed){editor=ed;}

				void MenuFunction()
				{
					editor->shownotfound=editor->shownotfound==true?false:true;
					mainsettings->manager_shownotfound=editor->shownotfound;
					if(editor->CheckForFile(M_SHOWNOTFOUND)==true)
					{
						editor->InitList();
						editor->ShowAudioFiles();
					}

					editor->ShowMenu();
				} //

				Edit_Manager *editor;
			};

			n->AddFMenu(Cxs[CXS_NOTFOUNDAUDIOFILES],menu_shonotfound=new menu_snotf(this),shownotfound);

			n->AddLine();

			guiMenu *sort=n->AddMenu(Cxs[CXS_SORT],0);

			if(sort)
			{
				class menu_sname:public guiMenu
				{
				public:
					menu_sname(Edit_Manager *ed)
					{
						editor=ed;
					}

					void MenuFunction()
					{
						editor->sorttype=SORTBYNAME;

						editor->InitList();
						editor->ShowAudioFiles();
						editor->ShowMenu();
					} //

					Edit_Manager *editor;
				};

				menu_sortname=sort->AddFMenu(Cxs[CXS_BYNAME],new menu_sname(this));

				class menu_ssize:public guiMenu
				{
				public:
					menu_ssize(Edit_Manager *ed){editor=ed;}

					void MenuFunction()
					{
						editor->sorttype=SORTBYSIZE;
						editor->InitList();
						editor->ShowAudioFiles();
						editor->ShowMenu();
					} //

					Edit_Manager *editor;
				};

				menu_sortsize=sort->AddFMenu(Cxs[CXS_BYSIZE],new menu_ssize(this));

				class menu_sdate:public guiMenu
				{
				public:
					menu_sdate(Edit_Manager *ed){editor=ed;}

					void MenuFunction()
					{
						editor->sorttype=SORTBYDATE;

						editor->InitList();
						editor->ShowAudioFiles();
						editor->ShowMenu();
					} //

					Edit_Manager *editor;
				};

				menu_sortdate=sort->AddFMenu(Cxs[CXS_BYFILEDATE],new menu_sdate(this));
			}
		}
	}
}

void Edit_Manager::CheckActiveHD()
{
	// Check Active HD
	if(GetActiveHDFile())
	{
		Sort_Files *sf=FirstSortFile();
		while(sf)
		{
			if(activefile==sf)	
				break;

			sf=sf->NextSortFile();
		}

		if(!sf){
			if(FirstSortFile()){
				ChangeSortFile(FirstSortFile());
			}
			else
				activefile=0;
		}
	}
	else{
		if(FirstSortFile()){
			ChangeSortFile(FirstSortFile());
		}
	}
}


void Edit_Manager::DeleteAllSortFiles()
{
	Sort_Files *sf=FirstSortFile();

	while(sf)
	{
		if(sf->sfname)
			delete sf->sfname;

		sf=sf->NextSortFile();
	}

	files.DeleteAllO();

	activefile=0;
}

Sort_Files *Edit_Manager::GetSortFileIndex(int index)
{
	return (Sort_Files *)files.GetO(index);
}

Sort_Files *Edit_Manager::FindAudioHDFile(AudioHDFile *hdfile)
{
	Sort_Files *sf=FirstSortFile();

	while(sf){
		if(sf->hdfile==hdfile)return sf;
		sf=sf->NextSortFile();
	}

	return 0;
}

void Edit_Manager::InitList()
{
	AudioHDFile *oldactivefile=GetActiveHDFile();
	DeleteAllSortFiles();

	int index=0;
	size_t flen=(!filter.filterstring)?0:strlen(filter.filterstring);

	audiofilescount=mainaudio->GetCountOfAudioFiles();

	for(;;)
	{
		AudioHDFile *file=mainaudio->FirstAudioHDFile();

		while(file)
		{
			bool insert=false;

			if(index==1)
			{
				// Sort Name
				Sort_Files *e=FirstSortFile();

				while(e){
					if(e->hdfile==file)
					{
						insert=true;
						break;
					}

					e=e->NextSortFile();
				}
			}

			if(
				insert==false &&
				(file->camximport==false || showintern==true) &&
				(file->camxrecorded==false || showrecorded==true) &&
				(file->filenotfound==false || shownotfound==true) &&
				((!flen) || filter.CheckString(file->GetFileName(),index)==true) &&
				(showonlysamplerate==false || file->samplerate==mainaudio->GetGlobalSampleRate())
				)
			{
				if(Sort_Files *s=new Sort_Files){

					if(file->errorflag)
						s->sfname=mainvar->GenerateString("*",file->ErrorString(),":",file->GetFileName());
					else
					{

						if(file->samplerate!=mainaudio->GetGlobalSampleRate())
						{
							int sr=file->samplerate;
							sr/=1000;

							char h2[NUMBERSTRINGLEN],*h=mainvar->GenerateString(mainvar->ConvertIntToChar(sr,h2),"k!: ");

							if(showfullpath==true || file->GetFileName()==0)
								s->sfname=mainvar->GenerateString(h,file->GetName());
							else
								s->sfname=mainvar->GenerateString(h,file->GetFileName());

							delete h;
						}
						else
						{
							if(showfullpath==true || file->GetFileName()==0)
								s->sfname=mainvar->GenerateString(file->GetName());
							else
								s->sfname=mainvar->GenerateString(file->GetFileName());
						}

						//if(s->sfname && strlen(s->sfname)>0 && s->sfname[0]>='a' && s->sfname[0]<='z')
						//	s->sfname[0]=toupper(s->sfname[0]);
					}

					if(s->sfname)
					{
						s->hdfile=file;

						switch(sorttype)
						{
						case SORTBYNAME:
							{
								if(index==1)
								{
									files.AddEndO(s);
								}
								else
								{
									// Sort Name
									Sort_Files *e=FirstSortFile();

									while(e){

										if(mainvar->strcmp_allsmall(e->sfname,s->sfname)>0)
										{
											files.AddNextO(s,e);
											break;
										}

										e=e->NextSortFile();
									}

									if(!e)files.AddEndO(s);
								}

								files.Close();

							}
							break;

						case SORTBYSIZE:
							{
								// Sort Name
								Sort_Files *e=FirstSortFile();

								while(e){

									if(file->samplesperchannel<e->hdfile->samplesperchannel){
										files.AddNextO(s,e);
										break;
									}

									e=e->NextSortFile();
								}

								if(!e)
									files.AddEndO(s);

								files.Close();
							}
							break;
						}
					}
					else
						delete s;
				}
			}

			file=file->NextHDFile();
		}

		index++;
		if((!flen) || index==2)
			break;

	}

	//Check Active HD
	if(oldactivefile)
	{
		activefile=FindAudioHDFile(oldactivefile);
	}

	if(!activefile)
		activefile=FirstSortFile()?FirstSortFile():0;

	if(activefile && activefile->hdfile)
	{
		activefile->hdfile->CreatePeak();

		if(activeregion && activeregion->r_audiohdfile!=activefile->hdfile)
			activeregion=activefile->hdfile->FirstRegion();
	}
	else
		activeregion=0;
}

void Edit_Manager::ShowAudioFiles()
{	
	if(!soundsgadget)return;

	soundsgadget->ClearListView();

	if(!activefile)
	{
		activefile=FirstSortFile();

		if(activefile)
			GetActiveHDFile()->Open();
	}

	/*
	if(!GetActiveHDFile())
	{
	activehdfile=mainaudio->FirstAudioHDFile();

	if(activehdfile)
	activehdfile->Open();
	}
	*/

	int ix=0;
	int aix=-1;
	char chl[NUMBERSTRINGLEN];
	char *fn;

	Sort_Files *a=FirstSortFile();
	while(a)
	{
		char *help=0;
		char *help2=0;
		char *s=0;

		if(activefile==a)
		{
			aix=ix;
		}

		ix++;

		if(a->hdfile->camxrecorded==true)
			fn=mainvar->GenerateString("[",mainvar->ConvertIntToChar(a->hdfile->channels,chl),"c *REC*] ",a->sfname);
		else
			if(a->hdfile->camximport==true)
				fn=mainvar->GenerateString("[",mainvar->ConvertIntToChar(a->hdfile->channels,chl),"c INTERN] ",a->sfname);
			else
				fn=mainvar->GenerateString("[",mainvar->ConvertIntToChar(a->hdfile->channels,chl),"c] ",a->sfname);

		if(a->hdfile->errorflag)
		{

		}
		else
		{
			// Show Regions
			if(a->hdfile->GetCountRegions() && s)
			{
				char h2[64];
				char *ns=mainvar->GenerateString(s?s:fn," Rs:[",mainvar->ConvertIntToChar(a->hdfile->GetCountRegions(),h2),"]");

				if(s)
					delete s;
				s=help=ns;
			}
		}

		soundsgadget->AddItem(0,s?s:fn);

		char h2[64];
		char *time=mainvar->ConvertSamplesToTime(a->hdfile->samplesperchannel,0,h2);

		soundsgadget->AddItem(1,time);

		LONGLONG mb=a->hdfile->channels*a->hdfile->samplesperchannel*a->hdfile->samplesize_one_channel;

		mb/=1024; // kb
		mb/=1024; // mb

		soundsgadget->AddItem(2,mainvar->ConvertLongLongToChar(mb,h2));
		soundsgadget->AddItem(3,mainvar->ConvertIntToChar(a->hdfile->samplerate,h2));
		soundsgadget->AddItem(4,mainvar->ConvertIntToChar(a->hdfile->channels,h2));
		soundsgadget->AddItem(5,mainvar->ConvertIntToChar(a->hdfile->samplebits,h2));

		if(a->hdfile->FirstRegion())
			soundsgadget->AddItem(7,mainvar->ConvertIntToChar(a->hdfile->GetCountRegions(),h2));

		switch(a->hdfile->type)
		{
		case TYPE_WAV:
			soundsgadget->AddItem(6,"WAV");
			break;

			case TYPE_AIFF:
			soundsgadget->AddItem(6,"AIFF");
			break;

			case TYPE_AIFFC:
			soundsgadget->AddItem(6,"AIFFC");
			break;

			default:
			soundsgadget->AddItem(6,"???");
			break;
		}


		if(help2)
			delete help2;

		if(fn)
			delete fn;

		a=a->NextSortFile();
	}

	if(activefile)
	{
		//soundsgadget->Enable();

		if(aix>=0)
			soundsgadget->SetSelection(aix);

		if(startsound)
			startsound->Enable();

		if(stopsound)
			stopsound->Enable();

		if(copysound)
			copysound->Enable();

		if(editsound)
			editsound->Enable();

		//soundsgadget->SetListBoxSelection(GetOfSortFile(activefile));

		RefreshHDFile(0);
	}
	else
	{
		//soundsgadget->AddStringToListBox(Cxs[CXS_NOAUDIOFILES]);
	//	soundsgadget->Disable();

		glist.Disable(startsound);
		glist.Disable(stopsound);
		glist.Disable(copysound);
		glist.Disable(editsound);
	}

	ShowActiveHDFile_Regions();
	ShowActiveHDFile_Info();
}

void Edit_Manager::ShowActiveHDFile_Info()
{
	if(infogadget)
	{
		infogadget->ClearListBox();

		if(GetActiveHDFile())
		{
			if(GetActiveHDFile()->errorflag)
			{
				infogadget->Enable();
				infogadget->AddStringToListBox(GetActiveHDFile()->ErrorString());
				infogadget->AddStringToListBox(GetActiveHDFile()->GetName());
			}
			else
			{
				char h[200];
				char h2[64];
				infogadget->Enable();

				infogadget->AddStringToListBox(GetActiveHDFile()->GetName());

				strcpy(h,"Samplerate:");
				mainvar->AddString(h,mainvar->ConvertIntToChar(GetActiveHDFile()->samplerate,h2));
				infogadget->AddStringToListBox(h);

				strcpy(h,"Channels:");
				mainvar->AddString(h,mainvar->ConvertIntToChar(GetActiveHDFile()->channels,h2));
				mainvar->AddString(h," Bits/Sample:");
				mainvar->AddString(h,mainvar->ConvertIntToChar(GetActiveHDFile()->samplebits,h2));
				infogadget->AddStringToListBox(h);

				// MB
				LONGLONG mb=GetActiveHDFile()->channels*GetActiveHDFile()->samplesperchannel*GetActiveHDFile()->samplesize_one_channel;

				mb/=1024; // kb
				mb/=1024; // mb

				strcpy(h,"Size:");
				mainvar->AddString(h,mainvar->ConvertIntToChar((int)mb,h2));

				mainvar->AddString(h,"(MB) Time:");
				mainvar->AddString(h,mainvar->ConvertSamplesToTime(GetActiveHDFile()->samplesperchannel,0,h2));
				infogadget->AddStringToListBox(h);

				strcpy(h,"Samples/Channel:");
				mainvar->AddString(h,mainvar->ConvertLongLongToChar(GetActiveHDFile()->samplesperchannel,h2));
				infogadget->AddStringToListBox(h);

				if(GetActiveHDFile()->GetCountRegions())
				{
				strcpy(h,"Regions:");
				mainvar->AddString(h,mainvar->ConvertIntToChar(GetActiveHDFile()->GetCountRegions(),h2));
				infogadget->AddStringToListBox(h);
				}

			}

			infogadget->CalcScrollWidth();
		}
		else
		{
			infogadget->Disable();
		}
	}
}

void Edit_Manager::ShowActiveHDFile_Regions()
{
	if(!regionslistview)
		return;

	regionslistview->ClearListView();

	char h2[NUMBERSTRINGLEN];

	if(!activefile)
		return;

	AudioRegion *r=activefile->hdfile->FirstRegion();

	while(r)
	{
		regionslistview->AddItem(0,r->GetName());
		char *time=mainvar->ConvertSamplesToTime(r->regionend-r->regionstart,0,h2);
		regionslistview->AddItem(1,time);

		regionslistview->AddItem(2,">");

		r=r->NextRegion();
	}

	if(activeregion)
	{
		regionslistview->SetSelection(activeregion->GetIndex());
	}

#ifdef OLDIE
	if(regionsgadget){

		regionsgadget->ClearListBox();

		if(GetActiveHDFile()){

			AudioRegion *r=GetActiveHDFile()->FirstRegion();

			char h2[64];

			while(r){

				char *time=mainvar->ConvertSamplesToTime(r->regionend-r->regionstart,0,h2);
				char *t=mainvar->GenerateString("R:",r->GetName(),"(",time,")");

				if(t)
				{
					regionsgadget->AddStringToListBox(t);
					delete t;
				}

				if(!activeregion)
					activeregion=r;

				r=r->NextRegion();
			}

			if(activeregion){

				regionsgadget->Enable();

				if(startregion)
					startregion->Enable();

				if(stopregion)
					stopregion->Enable();

				if(copyregion)
					copyregion->Enable();

				regionsgadget->SetListBoxSelection(activeregion->GetIndex());
			}
			else
			{
				regionsgadget->AddStringToListBox(Cxs[CXS_NOREGIONS]);

				regionsgadget->Disable();

				if(startregion)
					startregion->Disable();

				if(stopregion)
					stopregion->Disable();

				if(copyregion)
					copyregion->Disable();
			}
		}
		else
		{
			if(regionsgadget)
				regionsgadget->Disable();

			if(startregion)
				startregion->Disable();

			if(stopregion)
				stopregion->Disable();

			if(copyregion)
				copyregion->Disable();
		}
	}
#endif

}

void ShowFile_Callback(guiGadget_CW *g,int status)
{
	Edit_Manager *man=(Edit_Manager *)g->from;

	switch(status)
	{
	case DB_CREATE:
		man->showsound=g;
		break;

	case DB_PAINT:
		{
			man->ShowSound();
			man->ShowActiveHDFilePositions();
		}
		break;

	case DB_DOUBLECLICKLEFT:
		man->Play();
		break;

	case DB_MOUSEMOVE|DB_LEFTMOUSEDOWN:
	case DB_LEFTMOUSEDOWN:
		{
			man->MouseClickInGFX();
		}break;

	}
}

void Edit_Manager::MouseClickInGFX()
{
	LONGLONG pos=ConvertXToPositionX(showsound->SetToXX2(showsound->GetMouseX()));

	if(pos!=filestartposition)
	{
		filestartposition=pos;
		if(audiorealtime)
			Play();

		ShowActiveHDFilePositions();
		showsound->DrawSpriteBlt();
		ShowStartPosition();
	}
}

void Edit_Manager::RefreshHDFile(AudioHDFile *hdfile)
{
	if((!hdfile) || (activefile && activefile->hdfile==hdfile))
	{
		DrawDB(showsound);
	}
}

void Edit_Manager::ShowStartPosition()
{
	if(statusgadget)
	{
		Seq_Pos spos(Seq_Pos::POSMODE_TIME);
		char slen[NUMBERSTRINGLEN];

		mainaudio->ConvertSamplesToTime(filestartposition,&spos);
		spos.ConvertToString(WindowSong(),slen,NUMBERSTRINGLEN);

		statusgadget->ChangeButtonText(slen);
	}
}

void Edit_Manager::ShowActiveHDFilePositions()
{
	guiBitmap *bitmap=&showsound->spritebitmap;
	bitmap->guiFillRect(COLOUR_BLACK);

	int sx=ConvertSamplePositionX(filestartposition);

	//	bitmap->guiFillRectX0(0,x,showsound->GetHeight(),COLOUR_GREEN);
	bitmap->guiFillRect(sx,0,sx+3,showsound->GetHeight(),COLOUR_WHITE);

	if(frealtimepos!=-1)
	{
		frealtimeposx=ConvertSamplePositionX(frealtimepos);

		if(frealtimeposx>sx)
		{
			bitmap->guiFillRect(sx,0,frealtimeposx,showsound->GetHeight(),COLOUR_GREEN);
			bitmap->guiDrawLine(frealtimeposx,0,frealtimeposx,showsound->GetHeight(),COLOUR_WHITE);
		}
	}
}

void Edit_Manager::ShowSound()
{
	//ClearAllSprites();

	if(!showsound)
		return;

	guiBitmap *bitmap=&showsound->gbitmap;

	peakprogress=false;

	if(GetActiveHDFile())
	{
		if(GetActiveHDFile()==audiopeakthread->GetRunningFile())
			{
				ShowPeakProgress(audiopeakthread->createprogress);
				return;
			}
	
		if(GetActiveHDFile()->errorflag)
		{
			bitmap->guiFillRect(COLOUR_YELLOW);

			if(char *es=mainvar->GenerateString(GetActiveHDFile()->ErrorString(),":",GetActiveHDFile()->GetName()))
			{			
				bitmap->guiDrawText(1,maingui->GetFontSizeY(),bitmap->width,es);
				delete es;
			}
		}
		else{
			AudioGFX g;
			double sppixel;

			sppixel=GetActiveHDFile()->samplesperchannel;
			sppixel/=showsound->GetWidth();

			g.showmix=false;
			//g.perstart=0; // sample position 0
			g.samplesperpixel=(int)floor(sppixel+0.5); 
			g.win=this;
			g.bitmap=bitmap;
			g.usebitmap=true;

			g.x=0;
			g.samplex2=g.x2=bitmap->GetX2();
			g.y=0;
			g.y2=bitmap->GetY2();

			if(g.y+8<g.y2)
			{
				GetActiveHDFile()->ShowAudioFile(&g);

				if(activeregion && showregions==true)
				{
					int x=ConvertSamplePositionX(activeregion->regionstart),
						x2=ConvertSamplePositionX(activeregion->regionend);

					if(x!=-1 && x2!=-1)
						bitmap->guiInvert(x,g.y,x2,g.y2);
				}
			}
			else
				bitmap->guiFillRect(COLOUR_UNUSED);
		}
	}
	else
		bitmap->guiFillRect(COLOUR_UNUSED);

}

void Edit_Manager::ShowFilter()
{
	if(searchstring)
		searchstring->SetString(filter.filterstring);
}

void Edit_Manager::InitGadgets()
{
	glist.SelectForm(0,0);

	//glist.AddButton(-1,-1,w=4*maingui->GetFontSizeY(),-1,"Menu",GADGETID_TOOLBOX_MENU,MODE_MENU);
	//glist.AddLX();

	glist.AddButton(-1,-1,INFOSIZE,-1,"Filter",GADGET_SEARCHBUTTON,MODE_ADDDPOINT,"Reset Filter");
	glist.AddLX();

	searchstring=glist.AddString(-1,-1,-1,-1,GADGET_SEARCH,MODE_RIGHT,0,filter.filterstring,"Filter");
	glist.Return();

	g_showregions=glist.AddButton(-1,-1,INFOSIZE,-1,"Regions",GADGETID_REGIONS,showregions==true?MODE_GROUP|MODE_TOGGLE|MODE_TOGGLED:MODE_GROUP|MODE_TOGGLE);
	glist.Return();

	showsound=glist.AddChildWindow(-1,-1,-1,150,MODE_RIGHT|MODE_SPRITE,0,&ShowFile_Callback,this);
	glist.Return();

	infogadget=glist.AddListBox(-1,-1,-1,maingui->GetFontSizeY()*5,GADGET_INFO,MODE_RIGHT,Cxs[CXS_INFOAUDIOFILE]);
	glist.Return();

	startsound=glist.AddImageButton(-1,-1,CTRL_XY,CTRL_XY,IMAGE_PLAYBUTTON_SMALL_OFF,GADGETID_PLAY,0,Cxs[CXS_PLAYAUDIOFILECURSOR]);
	glist.AddLX();

	stopsound=glist.AddImageButton(-1,-1,CTRL_XY,CTRL_XY,IMAGE_STOPBUTTON_SMALL_ON,GADGETID_STOP,0,Cxs[CXS_STOPAUDIOFILEPLAYBACK]);
	glist.AddLX();

	frealtimepos=-1;
	statusgadget=glist.AddButton(-1,-1,150,-1,"",GADGET_STATUS,0,"Position");

	ShowStartPosition();
	glist.AddLX();

	copysound=glist.AddButton(-1,-1,150,-1,Cxs[CXS_COPY],GADGETID_COPYFILE,0,Cxs[CXS_COPYAUDIOFILE_CLIPBOARD]);
	glist.AddLX();

	editsound=glist.AddButton(-1,-1,150,-1,Cxs[CXS_EDIT],GADGET_EDITFILE,0,Cxs[CXS_OPENAUDIOEDITOR]);

	glist.Return();

	if(soundsgadget=glist.AddListView(-1,-1,-1,-2,GADGET_SAMPLEID,MODE_RIGHT|MODE_BOTTOM,Cxs[CXS_AUDIOFILES]))
	{
		soundsgadget->AddColume(Cxs[CXS_FILE],20);
		soundsgadget->AddColume(Cxs[CXS_LENGTH],6);
		soundsgadget->AddColume("MB",5);
		soundsgadget->AddColume("Rate",5);
		soundsgadget->AddColume(Cxs[CXS_CHANNELS],3);

		soundsgadget->AddColume("Bits",2);
		soundsgadget->AddColume("Format",3);
		soundsgadget->AddColume("Regions",4,true);
	}

	glist.NextVForm();

	startregion=glist.AddImageButton(-1,-1,CTRL_XY,CTRL_XY,IMAGE_SEPLAYREGION_SMALL,GADGETID_PLAYREGION,0,Cxs[CXS_PLAYREGIONSE]);
	glist.AddLX();
	copyregion=glist.AddButton(-1,-1,200,-1,Cxs[CXS_COPYREGION],GADGETID_COPYREGION,0,Cxs[CXS_COPYREGION_CLIPBOARD]);
	glist.Return();

	regionslistview=glist.AddListView(-1,-1,-1,-1,GADGET_REGIONID,MODE_RIGHT|MODE_BOTTOM);

	if(regionslistview)
	{
		regionslistview->AddColume("Regions",10);
		regionslistview->AddColume(Cxs[CXS_LENGTH],6);
		regionslistview->AddColume("Play",4);
	}

	//regionsgadget=glist.AddListBox(-1,-1,-1,-2,GADGET_REGIONID,MODE_RIGHT|MODE_BOTTOM,"Regions");
	glist.NextVForm();

	r_status=AR_STOPPED;

	FormEnable(0,1,showregions);

	ShowFilter();
}

void Edit_Manager::PlayRegion()
{
	if(GetActiveHDFile() && activeregion)
	{	
		StopPlayback();

		audiorealtime=GetActiveHDFile()->PlayRealtime(this,mainvar->GetActiveSong(),activeregion,0,&endstatus_realtime);

		if(audiorealtime)
			status=AR_STARTED;
	}
}

void Edit_Manager::Play()
{
	if(GetActiveHDFile() && mainvar->GetActiveSong())
	{	
		StopPlayback();

		if(filestartposition!=0 && filestartposition<GetActiveHDFile()->samplesperchannel)
		{
			testregion=new AudioRegion(GetActiveHDFile());

			if(testregion)
			{
				// Start Position/End
				testregion->regionstart=filestartposition; 
				testregion->regionend=GetActiveHDFile()->samplesperchannel;
				testregion->InitRegion();

				audiorealtime=GetActiveHDFile()->PlayRealtime(this,mainvar->GetActiveSong(),testregion,0,&endstatus_realtime,0,true);
			}
		}
		else
			audiorealtime=GetActiveHDFile()->PlayRealtime(this,mainvar->GetActiveSong(),0,0,&endstatus_realtime,0,true);

		if(audiorealtime)
			status=AR_STARTED;
	}
}

void Edit_Manager::KeyDown()
{
	switch(nVirtKey)
	{
	case 'P':
		{
			if(audiorealtime)
				StopPlayback();
			else
				Play();
		}
		break;
	}
}

bool Edit_Manager::CheckForFile(int type)
{
	AudioHDFile *file=mainaudio->FirstAudioHDFile();

	while(file)
	{

		switch(type)
		{
		case M_RECORDED:
			if(file->camxrecorded==true)
				return true;
			break;

		case M_SHOWNOTFOUND:
			if(file->filenotfound==true)
				return true;
			break;
		}

		file=file->NextHDFile();
	}

	return false;
}

void Edit_Manager::SetActiveFile(Sort_Files *file,bool refreshgui)
{
	if(file && file!=activefile)
	{
		if(activefile)
		{
			Sort_Files *temp=activefile;

			activefile=0; // for CheckAudioFileUsage

			if(mainaudio->CheckAudioFileUsage(temp->hdfile)==false)
				audiopeakthread->StopPeakFile(temp->hdfile->GetName());

			activefile=temp;
		}

		if(refreshgui==true)
		{			
			//if(soundsgadget)
			//	soundsgadget->SetListBoxSelection(GetOfSortFile(file));
		}

		if(file)
			mainsettings->SetLastAudioManagerFile(file->hdfile->GetName());

		ChangeSortFile(file);

		DrawDBBlit(showsound);
		ShowActiveHDFile_Regions();
	}
}

void Edit_Manager::ShowDisplayFilter()
{
	if(g_showregions)
		g_showregions->Toggle(showregions);
}

void Edit_Manager::Gadget(guiGadget *g)
{
	switch(g->gadgetID)
	{
	case GADGETID_TOOLBOX_MENU:
		{
			CreateMenu();
			if(popmenu)
				ShowPopMenu();
		}
		break;

	case GADGETID_REGIONS:
		{
			showregions=showregions==true?false:true;

			mainsettings->manager_showregions=showregions;
			FormYEnable(1,showregions);
			ShowDisplayFilter();
		}
		break;

	case GADGET_SEARCHBUTTON:
		// Reset
		if(filter.filterstring)
		{
			filter.SetString(0);
			ShowFilter();
			InitList();
			ShowAudioFiles();
			CheckActiveHD();
		}
		break;

	case GADGET_SEARCH:
		if((!filter.filterstring) || strcmp(filter.filterstring,g->string)!=0){
			if(filter.SetString(g->string)==true)
			{
				InitList();
				ShowAudioFiles();
				CheckActiveHD();
			}
		}
		break;

	case GADGET_EDITFILE:
		EditActiveFile(false);
		break;

	case GADGETID_PLAYREGION:
		PlayRegion();
		break;

	case GADGETID_COPYFILE:
		CopyHDFileToBuffer();
		break;

	case GADGETID_COPYREGION:
		CopyAudioRegionToBuffer();
		break;

	case GADGETID_PLAY:
		Play();
		break;

	case GADGETID_STOPREGION:
	case GADGETID_STOP:
		{
			if(audiorealtime)
				StopPlayback();
			else
				if(filestartposition!=0)
				{
					filestartposition=0;

					ShowActiveHDFilePositions();
					DrawDBSpriteBlit(showsound);
				}
		}
		break;

	case GADGET_SAMPLEID:
		{	
			if(g->doubleclick==true)
			{
				Play();
			}
			else
			{
				Sort_Files *sf=GetSortFileIndex(g->index);
				SetActiveFile(sf,false);
			}
		}
		break;

	case GADGET_REGIONID:
		if(GetActiveHDFile())
		{
			AudioRegion *r=GetActiveHDFile()->GetRegionIndex(g->index);

			if(r)
			{
				if(activeregion!=r)
				{
					StopPlayback();
					activeregion=r;
					//ShowActiveHDFile();
				}

				/*
				else
				PlayRegion();
				*/
			}
		}
		break;
	}
}

void Edit_Manager::StopPlayback()
{
	if(status!=AR_STOPPED)
	{
		status=AR_STOPPED;

		if(audiorealtime){
			mainaudioreal->StopAudioRealtime(audiorealtime,&endstatus_realtime);
			audiorealtime=0;
		}

		DeleteTestRegion();
	}
}

void Edit_Manager::ShowMenu()
{
	if(menu_showregions)
		menu_showregions->menu->Select(menu_showregions->index,showregions==true?true:false);

	if(menu_showinfo)
		menu_showinfo->menu->Select(menu_showinfo->index,showinfo==true?true:false);

	if(menu_showintern)
		menu_showintern->menu->Select(menu_showintern->index,showintern==true?true:false);

	if(menu_filepath)
		menu_filepath->menu->Select(menu_filepath->index,showfullpath==true?true:false);

	if(menu_showmb)
		menu_showmb->menu->Select(menu_showmb->index,showmb==true?true:false);

	if(menu_showtime)
		menu_showtime->menu->Select(menu_showtime->index,showtime==true?true:false);

	if(menu_showrecorded)
		menu_showrecorded->menu->Select(menu_showrecorded->index,showrecorded==true?true:false);

	if(menu_shonotfound)
		menu_shonotfound->menu->Select(menu_shonotfound->index,shownotfound==true?true:false);

	if(menu_showsr)
		menu_showsr->menu->Select(menu_showsr->index,showonlysamplerate==true?true:false);

	// Sort
	if(menu_sortdate)
		menu_sortdate->menu->Select(menu_sortdate->index,sorttype&SORTBYDATE?true:false);

	if(menu_sortname)
		menu_sortname->menu->Select(menu_sortname->index,sorttype&SORTBYNAME?true:false);

	if(menu_sortsize)
		menu_sortsize->menu->Select(menu_sortsize->index,sorttype&SORTBYSIZE?true:false);
}

void Edit_Manager::ShowPeakProgress(double per)
{
	if(showsound){

		char h2[NUMBERSTRINGLEN];

		if(char *h=mainvar->GenerateString(Cxs[CXS_PEAKPROGRESS],":",mainvar->ConvertDoubleToChar(per,h2,2),"%"))
		{
			showsound->gbitmap.guiFillRect(COLOUR_SAMPLEBACKGROUND);
			showsound->gbitmap.SetTextColour(COLOUR_BLACK);
			showsound->gbitmap.guiDrawText(0,maingui->GetButtonSizeY(),showsound->gbitmap.GetX2(),h);
			delete h;
		}

		peakprogress=true;
	}
}

void Edit_Manager::RefreshRealtime_Slow()
{
	countcheck++;

	if(countcheck>=10)
	{
	if(audiofilescount!=mainaudio->GetCountOfAudioFiles())
	{
		InitList();
		ShowAudioFiles();
		CheckActiveHD();
	}
	countcheck=0;
	}
}

void Edit_Manager::RefreshRealtime()
{
	if(status==AR_STARTED && endstatus_realtime==true)
	{
		status=AR_STOPPED;
		audiorealtime=0;
	}

	// Check Peak File Progress
	if(GetActiveHDFile()==audiopeakthread->GetRunningFile())
		DrawDBBlit(showsound);

	if(peakprogress==true && GetActiveHDFile() && GetActiveHDFile()->peakbuffer)
	{
		DrawDBBlit(showsound);	
	}

	if(audiorealtime && status==AR_STARTED)
	{
		// Check If Realtime Audio Stopped
		mainthreadcontrol->Lock(CS_audiorealtime);
		if(mainaudioreal->FindAudioRealtime(audiorealtime)==false)
		{
			audiorealtime=0;
			status=AR_STOPPED;
		}
		mainthreadcontrol->Unlock(CS_audiorealtime);

		if(audiorealtime)
		{
			//LONGLONG fpos=(audiorealtime->audiopattern.audioevent.fileposition-audiorealtime->audiopattern.audioevent.audioefile->datastart)/audiorealtime->audiopattern.audioevent.audioefile->samplesize_all_channels;

			if(frealtimepos!=audiorealtime->audiopattern.audioevent.sampleposition)
			{
				frealtimepos=audiorealtime->audiopattern.audioevent.sampleposition;

				if(statusgadget)
				{
					Seq_Pos spos(Seq_Pos::POSMODE_TIME);
					char slen[NUMBERSTRINGLEN];

					mainaudio->ConvertSamplesToTime(frealtimepos,&spos);
					spos.ConvertToString(WindowSong(),slen,NUMBERSTRINGLEN);

					statusgadget->ChangeButtonText(slen);
				}

				int px=ConvertSamplePositionX(frealtimepos);

				if(px!=frealtimeposx)
				{
					ShowActiveHDFilePositions();
					DrawDBSpriteBlit(showsound);
				}
			}
		}
	}
	else
	{
		if(frealtimepos!=-1)
		{
			frealtimepos=-1;

			ShowStartPosition();
			ShowActiveHDFilePositions();
			DrawDBSpriteBlit(showsound);
			//	statusgadget->ChangeButtonText("- - -");
		}
	}

	if(r_status!=status)
	{
		r_status=status;

		switch(status)
		{
		case AR_STARTED:
			if(startsound)
				startsound->ChangeButtonImage(IMAGE_PLAYBUTTON_ON);

			if(stopsound)
				stopsound->ChangeButtonImage(IMAGE_STOPBUTTON_OFF);
			break;

		case AR_STOPPED:
			if(startsound)
				startsound->ChangeButtonImage(IMAGE_PLAYBUTTON_OFF);

			if(stopsound)
				stopsound->ChangeButtonImage(IMAGE_STOPBUTTON_ON);
			break;
		}
	}
}

void Edit_Manager::RefreshObjects(LONGLONG type,bool editcall) // v
{
	InitList();
	ShowAudioFiles();
}

AudioHDFile *Edit_Manager::GetActiveHDFile()
{
	if((!activefile) && mainsettings->lastaudiomanagerfile)
	{
		return mainaudio->FindAudioHDFile(mainsettings->lastaudiomanagerfile);
	}

	return activefile?activefile->hdfile:0;
}

void Edit_Manager::CopyHDFileToBuffer()
{
	if(GetActiveHDFile() && GetActiveHDFile()->errorflag==0)
		mainbuffer->CreateAudioBuffer(GetActiveHDFile(),0);
}

void Edit_Manager::CopyAudioRegionToBuffer()
{
	if(GetActiveHDFile() && activeregion && GetActiveHDFile()->errorflag==0 && showregions==true)
		mainbuffer->CreateAudioBuffer(GetActiveHDFile(),activeregion);
}

LONGLONG Edit_Manager::ConvertXToPositionX(int x)
{
	if(GetActiveHDFile()){

		double w=showsound->GetWidth();
		double h=x;

		h/=w;
		h*=GetActiveHDFile()->samplesperchannel;

		return (LONGLONG)floor(h+0.5);
	}

	return 0;
}

int Edit_Manager::ConvertSamplePositionX(LONGLONG pos)
{
	if(GetActiveHDFile() && showsound && pos<=GetActiveHDFile()->samplesperchannel){

		double w=showsound->GetWidth(),per=pos;

		per/=GetActiveHDFile()->samplesperchannel;
		w*=per;

		return (int)floor(w+0.5f);
	}

	return -1;
}


void Edit_Manager::AddFilesFromDirectory()
{
	camxFile dir;

	if(dir.SelectDirectory(this,0,Cxs[CXS_ADDFILESFROMDIR])==true)
		mainaudio->CollectAudioFiles(dir.filereqname,0);
}

void Edit_Manager::EditActiveFile(bool region)
{
	if(GetActiveHDFile() && GetActiveHDFile()->errorflag==0 && 
		(region==false || activeregion)
		)
	{
		guiWindow *win=maingui->FindWindow(EDITORTYPE_SAMPLE,GetActiveHDFile(),0);

		if(!win)
		{
			Edit_Sample_StartInit init;

			init.file=GetActiveHDFile();

			if(region)
			{
				init.rangestart=activeregion->regionstart;
				init.rangeend=activeregion->regionend;
			}

			maingui->OpenEditorStart(EDITORTYPE_SAMPLE,mainvar->GetActiveSong(),0,0,0,(Object *)&init,0);
		}
		else
		{
			win->WindowToFront(true);
		}
	}
}

void Edit_Manager::ResetMenu()
{
	menu_filepath=0;
	menu_sortsize=menu_sortname=menu_sortdate=0;
	menu_showtime=menu_showmb=menu_showrecorded=menu_shonotfound=menu_showinfo=menu_showintern=menu_showregions=0;
	menu_showsr=0;
}

Edit_Manager::Edit_Manager()
{
	editorid=EDITORTYPE_AUDIOMANAGER;

	InitForms(FORM_VERT1x2_2small);

	resizeable=true;
	minheight=maingui->GetButtonSizeY(30);
	minwidth=maingui->GetButtonSizeY(15);
	ondesktop=true;
	
	activefile=0;
	activeregion=0;
	filestartposition=0;
	audiorealtime=0;
	status=AR_STOPPED;

	sorttype=SORTBYNAME;
	frealtimepos=-1;

	testregion=0;

	showfullpath=mainsettings->manager_showfullpath;
	showinfo=mainsettings->manager_showinfo;
	showmb=mainsettings->manager_showmb;
	showtime=mainsettings->manager_showtime;
	shownotfound=mainsettings->manager_shownotfound;
	showregions=mainsettings->manager_showregions;
	showrecorded=mainsettings->manager_showrecorded;
	showonlysamplerate=mainsettings->manager_showonlysamplerate;
	showintern=mainsettings->manager_showintern;

	ResetMenu();

	/*
	soundsgadget=0;
	infogadget=0;
	statusgadget=startsound=stopsound=copysound=editsound=startregion=stopregion=copyregion=searchstring=g_showregions=0;
*/

	peakprogress=false;
	countcheck=0;

	hasownmenu=true;
	regionslistview=0;
}

void Edit_Manager::DeleteTestRegion()
{
	if(testregion)
	{
		testregion->FreeMemory();
		delete testregion;
		testregion=0;
	}
}

void Edit_Manager::Init()
{
	guiSetWindowText(Cxs[CXS_AUDIOFILEMANAGER]);
	InitList();
	//	ShowMenu();

	InitGadgets();
	ShowAudioFiles();
}

