#include "songmain.h"
#include "editor.h"
#include "editor_event.h"
#include "editsettings.h"
#include "object_song.h"
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
#include "languagefiles.h"

#ifdef WIN32
#include<winuser.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *MIDIchannels[]=
{
	"Thru",
	"< 1 >",
	"< 2 >",
	"< 3 >",
	"< 4 >" ,
	"< 5 >",
	"< 6 >",
	"< 7 >",
	"< 8 >",
	"< 9 >",
	"< 10 >",
	"< 11 >",
	"< 12 >",
	"< 13 >",
	"< 14 >",
	"< 15 >",
	"< 16 >"
};

char *numbers_pos[]=
{
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"10",

	"11",
	"12",
	"13",
	"14",
	"15",
	"16",
	"17",
	"18",
	"19",
	"20",
	"21",
	"22",
	"23",
	"24",
	"25",
	"26",
	"27",
	"28",
	"29",
	"30"
};

char *numbers_neg[]=
{
	"0",
	"-1",
	"-2",
	"-3",
	"-4",
	"-5",
	"-6",
	"-7",
	"-8",
	"-9",
	"-10",

	"-11",
	"-12",
	"-13",
	"-14",
	"-15",
	"-16",
	"-17",
	"-18",
	"-19",
	"-20",
	"-21",
	"-22",
	"-23",
	"-24",
	"-25",
	"-26",
	"-27",
	"-28",
	"-29",
	"-30"
};

double Seq_Main::ConvertCharToDouble(char *str,int digits)
{
	switch(digits)
	{
	case 1:
		{
			char h[NUMBERSTRINGLEN];
			sprintf(h,"%.1f",atof(str));
			return atof(h);
		}
	case 2:
		{
			char h[NUMBERSTRINGLEN];
			sprintf(h,"%.2f",atof(str));
			return atof(h);
		}

	case 3:
		{
			char h[NUMBERSTRINGLEN];
			sprintf(h,"%.3f",atof(str));
			return atof(h);
		}

	default:
		return atof(str);
	}
}

int Seq_Main::ConvertCharToInt(char *str)
{
	return atoi (str);
}

void Seq_Main::SplitDoubleString(char *v,char *intpart,char *fpart)
{
	size_t sl=strlen(v);

	while(sl--)
	{
		if(*v=='.')
		{
			*intpart=0;

			strcpy(fpart,v);
			return;
		}

		*intpart++=*v++;
	}
}

char *Seq_Main::ConvertDoubleToChar(double num,char *res,int digits)
{
	switch(digits)
	{
	case 1:
		sprintf(res,"%.1f",num);
		break;

	case 2:
		sprintf(res,"%.2f",num);
		break;

	default:
		sprintf(res,"%.3f",num);
		break;
	}

	return res;
}

char *Seq_Main::ConvertFloatToChar(float num,char *res,int digits)
{
	double h=num;

	return ConvertDoubleToChar(h,res,digits);
}

bool Seq_Main::CompareStringWithOutZero(char *s1,char *s2)
{
	if(s1 && s2)
	{
		size_t i=strlen(s2);

		for(size_t a=0;a<i;a++)
		{
			if(*(s1+a)!=*(s2+a))
				return false;
		}

		return true;
	}

	return false;
}

bool Seq_Main::CheckIfInIndex(double startindex,double endindex,double index)
{
	if(startindex>endindex)
	{
		double h=endindex;
		endindex=startindex;
		startindex=h;
	}

	if(startindex>index)
		return false;

	if(endindex<index)
		return false;

	return true;
}

bool Seq_Main::CheckIfInIndex(int startindex,int endindex,int index)
{
	if(startindex>endindex)
	{
		int h=endindex;
		endindex=startindex;
		startindex=h;
	}

	if(startindex>index)
		return false;

	if(endindex<index)
		return false;

	return true;
}

bool Seq_Main::CheckIfInPosition(OSTART p1,OSTART p2,OSTART pos)
{
	if(p1>p2)
	{
		OSTART h=p2;
		p2=p1;
		p1=h;
	}

	if(p1>pos)
		return false;

	if(p2<pos)
		return false;

	return true;
}

char *Seq_Main::stripExtension(char *fileName)
{
	size_t length = strlen(fileName);

	for (size_t i=0; i!=length; ++i)
	{
		if (fileName[i]=='.') 
		{
			if(stripExtension_string)
				delete stripExtension_string;

			stripExtension_string=new char[i+1];

			if(stripExtension_string){
				strncpy(stripExtension_string,fileName,i);
				stripExtension_string[i]=0;

				return stripExtension_string;
			}
		}

	}
	return fileName;
}

char *Seq_Main::GenerateString(char *s1)
{
	if(s1)
	{
		if(char *h=new char[strlen(s1)+1])
		{
			strcpy(h,s1);
			return h;
		}
	}

	return 0;
}

char *Seq_Main::GenerateString(char *s1,char *s2)
{
	if(s1 && s2)
	{
		size_t sl1=strlen(s1),sl2=strlen(s2);
		if(char *h=new char[sl1+sl2+1])
		{
			strncpy(h,s1,sl1);
			strncpy(&h[sl1],s2,sl2);
			h[sl1+sl2]=0;

			return h;
		}
	}

	return 0;
}

char *Seq_Main::GenerateString(char *s1,char *s2,char *s3)
{
	if(s1 && s2 && s3)
	{
		size_t sl1=strlen(s1),
			sl2=strlen(s2),
			sl3=strlen(s3);

		if(char *h=new char[sl1+sl2+sl3+1])
		{
			strncpy(h,s1,sl1);
			strncpy(&h[sl1],s2,sl2);
			strncpy(&h[sl1+sl2],s3,sl3);
			h[sl1+sl2+sl3]=0;

			return h;
		}
	}

	return 0;
}

char *Seq_Main::GenerateString(char *s1,char *s2,char *s3,char *s4)
{
	if(s1 && s2 && s3 && s4)
	{
		size_t sl1=strlen(s1),
			sl2=strlen(s2),
			sl3=strlen(s3),
			sl4=strlen(s4);

		if(char *h=new char[sl1+sl2+sl3+sl4+1])
		{
			strncpy(h,s1,sl1);
			strncpy(&h[sl1],s2,sl2);
			strncpy(&h[sl1+sl2],s3,sl3);
			strncpy(&h[sl1+sl2+sl3],s4,sl4);
			h[sl1+sl2+sl3+sl4]=0;

			return h;
		}
	}

	return 0;
}

char *Seq_Main::GenerateString(char *s1,char *s2,char *s3,char *s4,char *s5)
{
	if(s1 && s2 && s3 && s4 && s5)
	{
		size_t sl1=strlen(s1),
			sl2=strlen(s2),
			sl3=strlen(s3),
			sl4=strlen(s4),
			sl5=strlen(s5);

		if(char *h=new char[sl1+sl2+sl3+sl4+sl5+1])
		{
			strncpy(h,s1,sl1);
			strncpy(&h[sl1],s2,sl2);
			strncpy(&h[sl1+sl2],s3,sl3);
			strncpy(&h[sl1+sl2+sl3],s4,sl4);
			strncpy(&h[sl1+sl2+sl3+sl4],s5,sl5);

			h[sl1+sl2+sl3+sl4+sl5]=0;

			return h;
		}
	}

	return 0;
}

char *Seq_Main::GenerateString(char *s1,char *s2,char *s3,char *s4,char *s5,char *s6)
{
	if(s1 && s2 && s3 && s4 && s5 && s6)
	{
		size_t sl1=strlen(s1),
			sl2=strlen(s2),
			sl3=strlen(s3),
			sl4=strlen(s4),
			sl5=strlen(s5),
			sl6=strlen(s6);

		if(char *h=new char[sl1+sl2+sl3+sl4+sl5+sl6+1])
		{
			strncpy(h,s1,sl1);
			strncpy(&h[sl1],s2,sl2);
			strncpy(&h[sl1+sl2],s3,sl3);
			strncpy(&h[sl1+sl2+sl3],s4,sl4);
			strncpy(&h[sl1+sl2+sl3+sl4],s5,sl5);
			strncpy(&h[sl1+sl2+sl3+sl4+sl5],s6,sl6);

			h[sl1+sl2+sl3+sl4+sl5+sl6]=0;

			return h;
		}
	}

	return 0;
}

char *Seq_Main::GenerateString(char *s1,char *s2,char *s3,char *s4,char *s5,char *s6,char *s7)
{
	if(s1 && s2 && s3 && s4 && s5 && s6 && s7)
	{
		size_t sl1=strlen(s1),
			sl2=strlen(s2),
			sl3=strlen(s3),
			sl4=strlen(s4),
			sl5=strlen(s5),
			sl6=strlen(s6),
			sl7=strlen(s7);

		if(char *h=new char[sl1+sl2+sl3+sl4+sl5+sl6+sl7+1])
		{
			strncpy(h,s1,sl1);
			strncpy(&h[sl1],s2,sl2);
			strncpy(&h[sl1+sl2],s3,sl3);
			strncpy(&h[sl1+sl2+sl3],s4,sl4);
			strncpy(&h[sl1+sl2+sl3+sl4],s5,sl5);
			strncpy(&h[sl1+sl2+sl3+sl4+sl5],s6,sl6);
			strncpy(&h[sl1+sl2+sl3+sl4+sl5+sl6],s7,sl7);

			h[sl1+sl2+sl3+sl4+sl5+sl6+sl7]=0;

			return h;
		}
	}

	return 0;
}

void Seq_Main::AddString(char *string,char *add)
{
	if(string && add)
	{
		strcpy(&string[strlen(string)],add);
	}
}

void Seq_Main::MixString(char *to,char *from1,char *from2)
{
	size_t l=strlen(from1);
	strcpy(to,from1);
	strcpy(&to[l],from2);
}

char *Seq_Main::CreateSimpleASCIIString(char *s)
{
	char *newstring=GenerateString(s);

	if(newstring)
	{
		char *t=newstring;
		size_t i=strlen(newstring);

		while(i--)
		{
			if(
				(*t>='0' && *t<='9') ||
				(*t>='A' && *t<='Z') ||
				(*t>='a' && *t<='z') ||
				*t=='_' ||
				*t==' '
				)
			{

			}
			else
			{
				*t=' ';
			}

			t++;
		}

		return newstring;
	}

	return 0;
}

char *Seq_Main::ClearString(char *str)
{
	if(str)
	{
		if(strlen(str)>=1)
		{
			if(char *tmp=mainvar->GenerateString(str))
			{
				char *t,*s=tmp;
				size_t i=strlen(tmp);

				t=tmp;

				while(i--)
				{
					if(*t=='\\' || *t=='/')
						s=t+1;

					t++;
				}

				t=s;
				i=strlen(s);
				while(i--)
				{
					if(*t=='.')
					{
						char *c=t+1;
						int h=strlen(c);

						while(h--)
						{
							if(*c=='.')
								break;
							c++;
						}

						if(h==-1)
						{
							break;
						}
					}

					t++;
					/*
					if((*t>='0' && *t<='9') ||
					(*t>='A' && *t<='Z') ||
					(*t>='a' && *t<='z') ||
					*t==' ' ||
					*t=='_' ||
					*t=='-'
					)
					*w++=*t++;
					else 
					t++;

					*/

					/*
					if(
					*t=='\\' ||
					*t=='/' ||
					*t=='_' ||
					*t=='(' ||
					*t==')' ||
					*t=='[' ||
					*t==']' ||
					*t=='{' ||
					*t=='}' ||
					*t=='.' ||
					*t=='*' ||
					*t=='+' ||
					*t=='-' ||
					*t=='´' ||
					*t=='`' ||
					*t==96 || // ´`
					*t==44 || // '
					*t==39 ||
					*t==38
					)
					{
					*t++=' ';
					}
					*/

				}

				*t=0;

				i=strlen(s);
				if(i>=0)
				{
					char *h=mainvar->GenerateString(s);
					delete tmp;
					return h;
				}

				delete tmp;
			}
		}

		return mainvar->GenerateString(str);
	}

	return 0;
}

char *Seq_Main::ConvertTicksToString(char *s,OSTART ticks)
{
	// /16 etc.
	for(int i=0;i<QUANTNUMBER;i++)
	{
		if(quantlist[i]==ticks)
		{
			strcpy(s,quantstr[i]);
			return s;
		}
	}

	return 0;
}

char *Seq_Main::ConvertLongLongToChar(LONGLONG num,char *res)
{
	if(num>=0 && num<=30)
		return numbers_pos[num];

	return(_i64toa(num,res,10));
}

char *Seq_Main::ConvertIntToChar(int num,char *res) // int
{
	if(num>=0 && num<=30)
		return numbers_pos[num];

	return(_itoa(num,res,10));
}

bool Seq_Main::FindWindow(guiWindow *w)
{
	guiWindow *cw=maingui->FirstWindow();

	while(cw)
	{
		if(cw==w)
			return true;

		cw=cw->NextWindow();
	}

	return false;
}
