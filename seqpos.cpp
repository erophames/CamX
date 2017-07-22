#include "defines.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "songmain.h"
#include <math.h>

#include "object_project.h"
#include "object_song.h"

#include "seqtime.h"
#include "seqpos.h"
#include "settings.h"
#include "audiohardware.h"
#include "gui.h"

#define MAXMEASURE 999999
#define MAXHOUR 8
#define MAXPOSLENGTH 28800000000

void Seq_Pos::ConvertMicroToPos(double micro)
{
	if(micro<0)return;

	if(mode==POSMODE_TIME || IsSmpte()==true)
	{
		LONGLONG h;

		// Hour
		h=(LONGLONG)micro/3600000000;
		pos[0]=h;
		micro-=h*3600000000;

		//Min
		h=(LONGLONG)micro/60000000;
		micro-=h*60000000;
		pos[1]=h;

		//Sec
		h=(LONGLONG)micro/1000000;
		micro-=h*1000000;
		pos[2]=h;

		if(IsSmpte()==true)
		{
			double fps=GetFPS();
			double fh=1000000/fps;

			double dh=micro;
			dh/=fh;

			// Frame
			pos[3]=(LONGLONG)floor(dh+0.5);

			double fdh=(double)pos[3];

			micro-=fdh*fh;

			double qh=1000000/(fps*4);

			//QF
			LONGLONG rmicro=(LONGLONG)micro;
			LONGLONG rqh=(LONGLONG)qh;

			rmicro/=rqh;

			pos[4]=rmicro; //rmicro; // 0-3

#ifdef DEBUG
//			if(pos[4]<0)
//				maingui->MessageBoxError(0,"POS4 <0");
#endif

			if(pos[4]<0)
				pos[4]=0;
		}
		else
		{
			pos[3]=(LONGLONG)micro/10000;
		}
	}
}

double Seq_Pos::ConvertToMicro()
{
	// 1 Mikrosekunde = 1000 Nanosekunden = 0,000 001 Sekunden = 10-6 Sekunden
	if(mode==POSMODE_TIME || IsSmpte()==true)		
	{
		double micro,h;

		// H
		h=(double)pos[0];
		micro=h*3600000000;

		// Min
		h=(double)pos[1];
		micro+=h*60000000;

		// Sec
		h=(double)pos[2];
		micro+=h*1000000;

		if(IsSmpte()==true) // Frame/QF
		{
			double fps=GetFPS();
			h=(double)pos[3];

			double mframes=1000000/fps;
			micro+=h*mframes;

			double mqframes=1000000/(fps*4);

			h=(double)pos[4];
			micro+=h*mqframes;
		}
		else
		{
			// Sec100
			h=(double)pos[3];

			micro+=h*10000;
		}

		return micro;
	}

	return 0;
}

bool Seq_Pos::AddPosition(Seq_Pos *add)
{
	if(add->mode==mode)
	{
		double m1=ConvertToMicro();
		m1+=add->ConvertToMicro();
		ConvertMicroToPos(m1);
		return true;
	}

	return false;
}

bool Seq_Pos::SubPosition(Seq_Pos *sub)
{
	if(sub->mode==mode)
	{
		if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
		{
			//TRACE ("Sub Measure\n");
			//TRACE (
		}
		else{
			double m1=ConvertToMicro();
			m1-=sub->ConvertToMicro();
			ConvertMicroToPos(m1);
		}

		return true;
	}

	return false;
}

Seq_Pos::Seq_Pos()
{
	mode=POSMODE_NORMAL;
	minus=false;

	for(int i=0;i<5;i++)
		pos[i]=0;

	length=false;
	song=0;
	offset=0;
}

Seq_Pos::Seq_Pos(int m) 
{
	mode=m;
	minus=false;

	for(int i=0;i<5;i++)
		pos[i]=0;

	length=false;
	song=0;
	offset=0;
}

bool Seq_Pos::Compare(Seq_Pos *p)
{
	if(p->mode!=mode)
		return false;

	if(minus!=p->minus)
		return false;

	for(int i=0;i<index;i++)
		if(pos[i]!=p->pos[i])
			return false;

	return true;
}

void Seq_Pos::Clone(Seq_Pos *p)
{
	for(int i=0;i<5;i++)
		p->pos[i]=pos[i];

	//p->samples=samples;
	
	p->zoomticks=zoomticks;
	p->mode=mode;
	p->measureformat=measureformat;
	p->index=index;
	p->sig=sig;
	p->minus=minus;
}

int Seq_Pos::GetPos3(Seq_Song *song)
{
	switch(song->project->projectmeasureformat)
	{
	case PM_1110:
	case PM_1p1p1p0:
	case PM_11_0:
	case PM_1p1p_0:
		return (int)(pos[3]-1);
	}

	return (int)pos[3];
}

bool Seq_Pos::AddHour(LONGLONG m)
{
	if(m==0)return false;

	if(mode==POSMODE_TIME || IsSmpte()==true)
	{
		m+=pos[0];

		if(m>MAXHOUR)return false;

		if(m>=0)
		{
			pos[0]=(int)m;
			return true;
		}
	}

	return false;
}

bool Seq_Pos::AddMin(LONGLONG m)
{
	if(m==0)return false;

	if(mode==POSMODE_TIME || IsSmpte()==true)
	{
		LONGLONG nm=pos[1]+m;
		LONGLONG nhour=nm/60;

		nm-=nhour*60;

		pos[1]=nm;

		if(nhour!=0)
			AddHour(nhour);
	}

	return false;
}

bool Seq_Pos::AddSec(LONGLONG m)
{
	if(m==0)return false;

	if(mode==POSMODE_TIME || IsSmpte()==true)
	{
		LONGLONG ns=pos[2]+m;
		LONGLONG nmin=ns/60;

		ns-=nmin*60;

		pos[2]=ns;

		if(nmin!=0)
			AddMin(nmin);

		return true;
	}

	return false;
}

bool Seq_Pos::AddSec100(double m)
{
	if(m==0)return false;

	if(mode==POSMODE_TIME)
	{
		LONGLONG n100=pos[3]+m;
		LONGLONG nsec=n100/100;

		n100-=nsec*100;

		pos[3]=n100;

		if(nsec!=0)
			AddSec(nsec);

		return true;

		/*
		double micro=ConvertToMicro();

		if(micro<MAXPOSLENGTH)
		{
			double add=m*10000;
			micro+=add;
			ConvertMicroToPos(micro);

			return true;
		}
		*/
	}

	return false;
}

bool Seq_Pos::AddFrame(LONGLONG m)
{
	if(m==0)return false;

	if(IsSmpte()==true)
	{
		LONGLONG fps=GetFPS();
		LONGLONG nf=pos[3]+m;
		LONGLONG nsec=nf/fps;

		nf-=nsec*fps;

		pos[3]=nf;

		if(nsec!=0)
			AddSec(nsec);

		return true;
	}
	return false;
}

bool Seq_Pos::AddQuarterFrame(LONGLONG m)
{
	if(m==0)return false;

	if(IsSmpte()==true)
	{
		LONGLONG nqf=pos[4]+m;
		LONGLONG nframes=nqf/4;

		nqf-=nframes*4;
		pos[4]=nqf;

		if(nframes!=0)
			AddFrame(nframes);

		return true;
	}

	return false;
}

double Seq_Pos_Offset::GetOffSetMs()
{
	Seq_Pos spos(song->project->standardsmpte);

	spos.pos[0]=h;
	spos.pos[1]=m;
	spos.pos[2]=sec;
	spos.pos[3]=frame;
	spos.pos[4]=0; // no qf

	spos.index=4;
	spos.offset=this;
	spos.song=song;

	double ms=spos.ConvertToMicro();

	ms/=1000;

	if(minus==true)
		ms=-ms;

	return ms;
}

double Seq_Pos::GetFPS()
{
	switch(mode)
	{
	case POSMODE_SMPTE_24:
		return 24;
		break;

	case POSMODE_SMPTE_25:
		return 25;
		break;

	case POSMODE_SMPTE_48:
		return 48;
		break;

	case POSMODE_SMPTE_50:
		return 50;
		break;

	case POSMODE_SMPTE_2997:
	case POSMODE_SMPTE_30:
	case POSMODE_SMPTE_2997df:
	case POSMODE_SMPTE_30df:
		return 30;
		break;

	case POSMODE_SMPTE_239:
		return 24;
		break;

	case POSMODE_SMPTE_249:
		return 25;
		break;

	case POSMODE_SMPTE_599:
		return 60;
		break;

	case POSMODE_SMPTE_60:
		return 60;
		break;
	}

	return 24;
}



bool Seq_Pos::AddMeasure(int m)
{
	if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
	{
		m+=(int)pos[0];

		if(m>MAXMEASURE)return false;

		if(m>=1)
		{
			pos[0]=m;
			sig=sig->map->FindSignatureBefore(sig->map->ConvertMeasureToTicks(m));
			return true;
		}
	}

	return false;
}

bool Seq_Pos::AddBeat(int m)
{
	if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
	{
		int hm=(int)pos[1]+m;

		if(hm>sig->nn)
		{
			if(AddMeasure(m)==true)
			{
				pos[1]=1;
				return true;
			}
		}
		else
			if(hm<=0)
			{
				if(AddMeasure(m)==true)
				{
					pos[1]=sig->nn;
					return true;
				}
			}
			else
			{
				pos[1]=hm;
				return true;
			}
	}

	return false;
}

bool Seq_Pos::AddZoomTicks(int m)
{
	if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
	{
		OSTART hm=(int)pos[2]+m,
			ztf=sig->dn_ticks/zoomticks;

		if(hm>ztf)
		{
			if(AddBeat(m)==true)
			{
				pos[2]=1;
				return true;
			}
		}
		else
			if(hm<=0)
			{
				if(AddBeat(m)==true)
				{
					pos[2]=nozoom==false?sig->dn_ticks/zoomticks:sig->dn_ticks;
					return true;
				}
			}
			else
			{
				pos[2]=hm;
				return true;
			}
	}

	return false;
}

bool Seq_Pos::AddTicks(int m)
{
	if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
	{
		OSTART maxticks=nozoom==true?sig->dn_ticks:zoomticks;

		LONGLONG hm=pos[index-1]; // 1.1.145 or 1.1.1.145

		hm+=m;

		LONGLONG mini=1,maxi=mainaudio->ConvertInternRateToPPQ(maxticks);
		int tickindex=index-1;

		if(song)
			switch(song->project->projectmeasureformat)
		{
			case PM_1110:
			case PM_1p1p1p0:
			case PM_11_0:
			case PM_1p1p_0:
				mini=0;
				maxi--;
				break;
		}

		if(hm>maxi)
		{
			if(nozoom==true)
			{
				AddBeat(m);
				pos[tickindex]=mini;
				return true;
			}

			if(AddZoomTicks(m)==true)
			{
				pos[tickindex]=mini;
				//TRACE ("P3_1 %d\n",pos[3]);
				return true;
			}
		}
		else
			if(hm<mini)
			{
				if(nozoom==true)
				{
					AddBeat(m);
					pos[tickindex]=maxi;
					return true;
				}

				if(AddZoomTicks(m)==true)
				{
					pos[tickindex]=maxi;
					//TRACE ("P3_2 %d\n",pos[3]);
					return true;
				}
			}
			else
			{
				pos[tickindex]=hm;
				//TRACE ("P3_3 %d\n",pos[3]);
				return true;
			}
	}

	return false;
}

bool Seq_Pos::AddStartPositionEditing(int m)
{
	if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
	{
		m+=(int)pos[0];

		if(m>5000)return false;

		if(m>=0)
		{
			pos[0]=m;
			return true;
		}
	}

	return false;
}

bool Seq_Pos::AddBeatLength(int m)
{
	if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
	{
		LONGLONG hm=pos[1]+m;

		if(hm>=sig->nn)
		{
			if(AddStartPositionEditing(m)==true)
			{
				pos[1]=0;
				return true;
			}
		}
		else
			if(hm<0)
			{
				if(AddStartPositionEditing(m)==true)
				{
					pos[1]=sig->nn-1;
					return true;
				}
			}
			else
			{
				pos[1]=hm;
				return true;
			}
	}

	return false;
}

bool Seq_Pos::AddZoomTicksLength(int m)
{
	if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
	{
		LONGLONG hm=pos[2]+m,ztf=sig->dn_ticks/zoomticks;

		if(hm>=ztf)
		{
			if(AddBeatLength(m)==true)
			{
				pos[2]=0;
				return true;
			}
		}
		else
			if(hm<0)
			{
				if(AddBeatLength(m)==true)
				{
					pos[2]=(sig->dn_ticks/zoomticks)-1;
					return true;
				}
			}
			else
			{
				pos[2]=hm;
				return true;
			}
	}

	return false;
}

bool Seq_Pos::AddTicksLength(int m)
{
	if(mode==POSMODE_NORMAL || mode==POSMODE_COMPRESS)
	{
		LONGLONG hm=pos[3]+m;

		if(hm>=zoomticks)
		{
			if(AddZoomTicksLength(m)==true)
			{
				pos[3]=0;
				TRACE ("P3_1 %d\n",pos[3]);
				return true;
			}
		}
		else
			if(hm<0)
			{
				if(AddZoomTicksLength(m)==true)
				{
					pos[3]=zoomticks-1;
					TRACE ("P3_2 %d\n",pos[3]);
					return true;
				}
			}
			else
			{
				pos[3]=hm;
				TRACE ("P3_3 %d\n",pos[3]);
				return true;
			}
	}

	return false;
}

void Seq_Pos::InitWithWindowDisplay(Seq_Project *pro,int m)
{
	switch(m)
	{
	case WINDOWDISPLAY_MEASURE:
		mode=POSMODE_NORMAL;
		break;

	case WINDOWDISPLAY_SMPTE:
	case WINDOWDISPLAY_SECONDS:
		if(pro)
			mode=pro->standardsmpte;
		else
			mode=mainsettings->projectstandardsmpte;
		break;

	case WINDOWDISPLAY_SAMPLES:
		mode=POSMODE_SAMPLES;
		break;
	}
}

void Seq_Pos::ConvertToLengthString(Seq_Song *song,char *string,size_t len,char **singlestrings,int iflag)
{
	char help[NUMBERSTRINGLEN],smptehelp[NUMBERSTRINGLEN];

	if(len>1)
	{
		if(mode==POSMODE_ONLYMEASURE)
		{
			strcpy(string,mainvar->ConvertIntToChar((int)(pos[0]+1),help));
		}
		else
			if(mode==POSMODE_SAMPLES)
			{
				strcpy(string,mainvar->ConvertLongLongToChar(pos[0],help));
				strcpy(singlestrings[0],string);
			}
			else
			{
				len--; // 0

				bool nullset=false;

				for(int a=0;a<index;a++)
				{
					char *from=0;

					if(mode==POSMODE_ONLYMEASURE && a==3)
						from=mainvar->ConvertIntToChar(GetPos3(song),help);
					else
					{
						if(mode==POSMODE_ONLYMEASURE && nozoom==true && a==2)
							from=" - ";
						else
						{
							if(pos[a]==0 && mode==POSMODE_TIME)
							{
								from="00";
							}
							else
								from=mainvar->ConvertIntToChar((int)pos[a],help);
						}
					}

					if(usesmpte==true && strlen(from)==1 && a<4)
					{
						smptehelp[0]='0';
						strcpy(&smptehelp[1],from);

						from=smptehelp;
					}

					if(mode==POSMODE_TIME && a==3 && pos[a]==0)
					{
						from=0;
					}

					if(from)
					{
						if(singlestrings)
						{
							if(nullset==false && (strcmp(from,"0")==0 || strcmp(from,"00")==0) )
							{
								from="_";
							}
							else
								nullset=true;

							strcpy(singlestrings[a],from);
						}

						if(mode==POSMODE_TIME && a==3)
						{
							*string++='.';
						}
					}
				}

				// QFrame
				if(usesmpte==true && (!(iflag&Seq_Time::TIMESIMPLE)))
				{
					*string++=';';

					char *from=mainvar->ConvertIntToChar((int)pos[4],help);
					size_t i=strlen(from);

					if(singlestrings)
						strcpy(singlestrings[4],from);

					if(len>=i)
					{
						len-=i;
						while(i--)*string++=*from++;
					}
				}

				*string=0;
			}
	}
}

void Seq_Pos::ConvertToString(Seq_Song *song,char *string,size_t len,char **singlestrings,int iflag)
{
	char *sstring=string;
	char help[NUMBERSTRINGLEN],smptehelp[NUMBERSTRINGLEN],minushelp[NUMBERSTRINGLEN];

	bool smpte=IsSmpte();

	if(len>1)
	{
		if(mode==POSMODE_ONLYMEASURE)
		{
			strcpy(string,mainvar->ConvertIntToChar((int)pos[0]+1,help));
		}
		else
			if(mode==POSMODE_SAMPLES)
			{
				strcpy(string,mainvar->ConvertLongLongToChar(pos[0],help));

				if(singlestrings)
					strcpy(singlestrings[0],string);
			}
			else
			{
				// Smpte/Time
				int to=0;

				switch(mode)
				{
				case POSMODE_TIME:
					to=4; // h:min:sec.sec100
					break;

				default:
					{
						to=4; // h:min:sec:frame:qf
					}
					break;
				}

				len--; // 0

				bool nullset=true;

				for(int a=0;a<to;a++)
				{
					char *from=0;

					if(mode==POSMODE_ONLYMEASURE && a==3)
						from=mainvar->ConvertIntToChar(GetPos3(song),help);
					else
					{
						if(mode==POSMODE_ONLYMEASURE && nozoom==true && a==2)
							from=" - ";
						else
						{
							if(pos[a]==0 && mode==POSMODE_TIME)
							{
								from="00";
							}
							else
								from=mainvar->ConvertIntToChar((int)pos[a],help);
						}
					}

					if(smpte==true && strlen(from)==1 && a<4)
					{
						// 1>01
						smptehelp[0]='0';
						strcpy(&smptehelp[1],from);

						from=smptehelp;
					}

					if(a==0 && smpte==true && minus==true)
					{
						minushelp[0]='-';
						strcpy(&minushelp[1],from);

						from=minushelp;
					}

					if(from)
					{
						if(singlestrings)
							strcpy(singlestrings[a],from);

						if(mode==POSMODE_TIME && a==3)
						{
							*string++='.';
						}

						if(iflag&Seq_Time::TIMESIMPLE)
						{
							if(pos[a]!=0)
								nullset=false;
						}
						else
							nullset=false;

						if(nullset==false)
						{
							size_t i=strlen(from);
							if(len>=i)
							{
								len-=i;
								while(i--)*string++=*from++;
							}

							if(len && a<to-1)
							{
								switch(mode)
								{
								case POSMODE_ONLYMEASURE:
									{
										switch(song->project->projectmeasureformat)
										{
										case PM_1111:
										case PM_1110:
											*string++=' ';
											break;

										case PM_1p1p1p1:
										case PM_1p1p1p0:
											*string++='.';
											break;

										case PM_11_0:
										case PM_11_1:
											if(a!=2)*string++=' ';	
											break;

										case PM_1p1p_1:
										case PM_1p1p_0:
											if(a!=2)*string++='.';
											break;

										default:
											*string++='.';
											break;
										}
									}
									break;

								case POSMODE_TIME:
									if(a<2)
										*string++=':';
									break;

								default:
									*string++=':';
									break;
								}

								len--;
							}
						}
					}
				}

				// QFrame
				if(smpte==true && (!(iflag&Seq_Time::TIMESIMPLE)))
				{
					*string++=';';

					char *from=mainvar->ConvertIntToChar((int)pos[4],help);
					size_t i=strlen(from);

					if(singlestrings)
						strcpy(singlestrings[4],from);

					if(len>=i)
					{
						len-=i;
						while(i--)*string++=*from++;
					}
				}

				*string=0;

				if(smpte==true && strlen(sstring)==2)
				{
					// 01 -> :01
					char buffer[4];

					strcpy(&buffer[1],sstring);
					buffer[0]=':';

					strcpy(sstring,buffer);
				}

			}
	}
}