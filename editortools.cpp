#include "gui.h"
#include "editor.h"
#include "seqtime.h"
#include "object_song.h"
#include "audiohardware.h"
#include "songmain.h"
#include "editdata.h"
#include "languagefiles.h"

/*
    Filter Resonance (Timbre/Harmonic Intensity) (cc#71)
    Release Time (cc#72)
    Attack time (cc#73)
    Brightness/Cutoff Frequency (cc#74)
    Decay Time (cc#75)
    Vibrato Rate (cc#76)
    Vibrato Depth (cc#77)
    Vibrato Delay (cc#78)
*/

char *controlnames[128] =
{
	"Bank MSB",
	"Modulation MSB",
	"Breath MSB",
	"Continuous",
	"Foot MSB",
	"Portamento MSB",
	"Data MSB",
	"Main Volume MSB",
	"Balance",
	"Ctrl9",

	"Pan",
	"Expression",
	"FX 1",
	"FX 2",
	"Ctrl14",
	"Ctrl15",
	"Multi 1",
	"Multi 2",
	"Multi 3",
	"Multi 4",

	"CtrlL20",
	"CtrlL21",
	"CtrlL22",
	"CtrlL23",
	"CtrlL24",
	"CtrlL25",
	"CtrlL26",
	"CtrlL27",
	"CtrlL28",
	"CtrlL29",

	"CtrlL30",
	"CtrlL31",
	"Bank-Sel.LSB",
	"Modulation LSB",
	"Breath LSB",
	"Continuous",
	"Foot LSB",
	"Port.T.LSB",
	"Data Entry LSB",
	"Main Volume LSB",

	"Balance LSB",
	"CtrlL41",
	"Pan LSB42",
	"Expr LSB",
	"CtrlL44",
	"CtrlL45",
	"CtrlL46",
	"CtrlL47",
	"Gen.P.1 L",
	"Gen.P.2 L",

	"Gen.P.3 L",
	"Gen.P.4 L",
	"CtrlL52",
	"CtrlL53",
	"CtrlL54",
	"CtrlL55",
	"CtrlL56",
	"CtrlL57",
	"CtrlL58",
	"CtrlL59",

	"CtrlL60",
	"CtrlL61",
	"CtrlL62",
	"CtrlL63",
	"Sustain",
	"Portam. +/-",
	"Sostenuto",
	"Soft Pedal",
	"Legato sw",
	"Hold 2",

	"SoundCt1",                          /* 70 */
	"Filter Resonance",
	"Release Time",
	"Attack Time",
	"Cutoff Freq",
	"Decay Time",
	"Vibrato Rate ",
	"Vibrato Depth",
	"Vibrato Delay",
	"SoundCt10",

	"Decay",                    /* 80 */
	"HPF Freq",
	"Gen.Purp.7",
	"Gen.Purp.8",
	"Portamento",
	"Ctrl85",
	"Ctrl86",
	"Ctrl87",
	"Ctrl88",
	"Ctrl89",

	"Ctrl90",                          /* 90 */
	"Reverb",
	"Tremolo",
	"Chorus Depth",
	"Detune/Var.",
	"Phaser",
	"Data +",
	"Data -",
	"NonReg LSB",
	"NonReg MSB",

	"Reg LSB",                          /* 100 */
	"Reg MSB",
	"Ctrl102",
	"Ctrl103",
	"Ctrl104",
	"Ctrl105",
	"Ctrl106",
	"Ctrl107",
	"Ctrl108",
	"Ctrl109",

	"Ctrl110",                          /* 110 */
	"Ctrl111",
	"Ctrl112",
	"Ctrl113",
	"Ctrl114",
	"Ctrl115",
	"Ctrl116",
	"Ctrl117",
	"Ctrl118",
	"Ctrl119",

	"Ctrl120",                          /* 120 */
	"Reset Ctrls",
	"Local Control",
	"All Notes Off",
	"Omni Mode Off",
	"Omni Mode On",
	"Mono Mode On",
	"Poly Mode On"
};

char *controlnames_plain[128]=
{
	"Bank",
	"Modulation",
	"Breath",
	"Continuous",
	"Foot",
	"Portamento",
	"Data",
	"Main Volume",
	"Balance",
	"Ctrl9",

	"Pan",
	"Expression",
	"FX 1",
	"FX 2",
	"Ctrl14",
	"Ctrl15",
	"Multi 1",
	"Multi 2",
	"Multi 3",
	"Multi 4",

	"CtrlL20",
	"CtrlL21",
	"CtrlL22",
	"CtrlL23",
	"CtrlL24",
	"CtrlL25",
	"CtrlL26",
	"CtrlL27",
	"CtrlL28",
	"CtrlL29",

	"CtrlL30",
	"CtrlL31",
	"Bank-Sel.",
	"Modulation",
	"Breath",
	"Continuous",
	"Foot",
	"Port.T.",
	"Data Entry",
	"Main Volume",

	"Balance",
	"CtrlL41",
	"Pan LSB42",
	"Expr LSB",
	"CtrlL44",
	"CtrlL45",
	"CtrlL46",
	"CtrlL47",
	"Gen.P.1 L",
	"Gen.P.2 L",

	"Gen.P.3 L",
	"Gen.P.4 L",
	"CtrlL52",
	"CtrlL53",
	"CtrlL54",
	"CtrlL55",
	"CtrlL56",
	"CtrlL57",
	"CtrlL58",
	"CtrlL59",

	"CtrlL60",
	"CtrlL61",
	"CtrlL62",
	"CtrlL63",
	"Sustain",
	"Portam. +/-",
	"Sostenuto",
	"Soft Pedal",
	"Legato sw",
	"Hold 2",

	"SoundCt1",                          /* 70 */
	"Filter Resonance",
	"Release Time",
	"Attack Time",
	"Cutoff Freq",
	"Decay Time",
	"Vibrato Rate ",
	"Vibrato Depth",
	"Vibrato Delay",
	"SoundCt10",

	"Decay",                    /* 80 */
	"HPF Freq",
	"Gen.Purp.7",
	"Gen.Purp.8",
	"Portamento",
	"Ctrl85",
	"Ctrl86",
	"Ctrl87",
	"Ctrl88",
	"Ctrl89",

	"Ctrl90",                          /* 90 */
	"Reverb",
	"Tremolo",
	"Chorus Depth",
	"Detune/Var.",
	"Phaser",
	"Data +",
	"Data -",
	"NonReg",
	"NonReg",

	"Reg",                          /* 100 */
	"Reg",
	"Ctrl102",
	"Ctrl103",
	"Ctrl104",
	"Ctrl105",
	"Ctrl106",
	"Ctrl107",
	"Ctrl108",
	"Ctrl109",

	"Ctrl110",                          /* 110 */
	"Ctrl111",
	"Ctrl112",
	"Ctrl113",
	"Ctrl114",
	"Ctrl115",
	"Ctrl116",
	"Ctrl117",
	"Ctrl118",
	"Ctrl119",

	"Ctrl120",                          /* 120 */
	"Reset Ctrls",
	"Local Control",
	"All Notes Off",
	"Omni Mode Off",
	"Omni Mode On",
	"Mono Mode On",
	"Poly Mode On"
};

char *controlnames_number[128] =
{
	"Bank MSB (0)",
	"Modulation MSB (1)",
	"Breath MSB (2)",
	"Continuous (3)",
	"Foot MSB (4)",
	"Portamento MSB (5)",
	"Data MSB (6)",
	"Main Volume MSB (7)",
	"Balance (8)",
	"Ctrl9 (9)",

	"Pan (10)",
	"Expression (11)",
	"FX 1 (12)",
	"FX 2 (13)",
	"Ctrl14 (14)",
	"Ctrl15 (15)",
	"Multi 1 (16)",
	"Multi 2 (17)",
	"Multi 3 (18)",
	"Multi 4 (19)",

	"CtrlL20 (20)",
	"CtrlL21 (21)",
	"CtrlL22 (22)",
	"CtrlL23 (23)",
	"CtrlL24 (24)",
	"CtrlL25 (25)",
	"CtrlL26 (26)",
	"CtrlL27 (27)",
	"CtrlL28 (28)",
	"CtrlL29 (29)",

	"CtrlL30 (30)",
	"CtrlL31 (31)",
	"Bank-Sel.LSB (32)",
	"Modulation LSB (33)",
	"Breath LSB (34)",
	"Continuous (35)",
	"Foot LSB (36)",
	"Port.T.LSB (37)",
	"Data Entry LSB (38)",
	"Main Volume LSB (39)",

	"Balance LSB (40)",
	"CtrlL41 (41)",
	"Pan LSB42 (42)",
	"Expr LSB (43)",
	"CtrlL44 (44)",
	"CtrlL45 (45)",
	"CtrlL46 (46)",
	"CtrlL47 (47)",
	"Gen.P.1 L (48)",
	"Gen.P.2 L (49)",

	"Gen.P.3 L (50)",
	"Gen.P.4 L (51)",
	"CtrlL52 (52)",
	"CtrlL53 (53)",
	"CtrlL54 (54)",
	"CtrlL55 (55)",
	"CtrlL56 (56)",
	"CtrlL57 (57)",
	"CtrlL58 (58)",
	"CtrlL59 (59)",

	"CtrlL60 (60)",
	"CtrlL61 (61)",
	"CtrlL62 (62)",
	"CtrlL63 (63)",
	"Sustain (64)",
	"Portam. +/- (65)",
	"Sostenuto (66)",
	"Soft Pedal (67)",
	"Legato sw (68)",
	"Hold 2 (69)",

	"SoundCt1 (70)",                          /* 70 */
	"Filter Resonance (71)",
	"Release Time (72)",
	"Attack Time (73)",
	"Cutoff Freq (74)",
	"Decay Time (75)",
	"Vibrato Rate (76)",
	"Vibrato Depth (77)",
	"Vibrato Delay (78)",
	"SoundCt10 (79)",

	"Decay (80)",                    /* 80 */
	"HPF Freq (81)",
	"Gen.Purp.7 (82)",
	"Gen.Purp.8 (83)",
	"Portamento (84)",
	"Ctrl85 (85)",
	"Ctrl86 (86)",
	"Ctrl87 (87)",
	"Ctrl88 (88)",
	"Ctrl89 (89)",

	"Ctrl90 (90)",                          /* 90 */
	"Reverb (91)",
	"Tremolo (92)",
	"Chorus Depth (93)",
	"Detune/Var. (94)",
	"Phaser (95)",
	"Data + (96)",
	"Data - (97)",
	"NonReg LSB (98)",
	"NonReg MSB (99)",

	"Reg LSB (100)",                          /* 100 */
	"Reg MSB (101)",
	"Ctrl102 (102)",
	"Ctrl103 (103)",
	"Ctrl104 (104)",
	"Ctrl105 (105)",
	"Ctrl106 (106)",
	"Ctrl107 (107)",
	"Ctrl108 (108)",
	"Ctrl109 (109)",

	"Ctrl110 (110)",                          /* 110 */
	"Ctrl111 (111)",
	"Ctrl112 (112)",
	"Ctrl113 (113)",
	"Ctrl114 (114)",
	"Ctrl115 (115)",
	"Ctrl116 (116)",
	"Ctrl117 (117)",
	"Ctrl118 (118)",
	"Ctrl119 (119)",

	"Ctrl120 (120)",                          /* 120 */
	"Reset Ctrls (121)",
	"Local Control (122)",
	"All Notes Off (123)",
	"Omni Mode Off (124)",
	"Omni Mode On (125)",
	"Mono Mode On (126)",
	"Poly Mode On (127)"
};

// SI ----------------------
static char *octave_si[11]=
{
	"-2",
	"-1",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8"
};

// Roman
char *keynames_si[128]=
{
	"Do-2",
	"Do#-2",
	"Ré-2",
	"Ré#-2",
	"Mi-2",
	"Fa-2",
	"Fa#-2",
	"Sol-2",
	"Sol#-2",
	"La-2",
	"La#-2",
	"Si-2",

	"Do-1",
	"Do#-1",
	"Ré-1",
	"Ré#-1",
	"Mi-1",
	"Fa-1",
	"Fa#-1",
	"Sol-1",
	"Sol#-1",
	"La-1",
	"La#-1",
	"Si-1",

	"Do 0",
	"Do# 0",
	"Ré 0",
	"Ré# 0",
	"Mi 0",
	"Fa 0",
	"Fa# 0",
	"Sol 0",
	"Sol# 0",
	"La 0",
	"La# 0",
	"Si 0",

	"Do 1",
	"Do# 1",
	"Ré 1",
	"Ré# 1",
	"Mi 1",
	"Fa 1",
	"Fa# 1",
	"Sol 1",
	"Sol# 1",
	"La 1",
	"La# 1",
	"Si 1",

	"Do 2",
	"Do# 2",
	"Ré 2",
	"Ré# 2",
	"Mi 2",
	"Fa 2",
	"Fa# 2",
	"Sol 2",
	"Sol# 2",
	"La 2",
	"La# 2",
	"Si 2",

	"Do 3",
	"Do# 3",
	"Ré 3",
	"Ré# 3",
	"Mi 3",
	"Fa 3",
	"Fa# 3",
	"Sol 3",
	"Sol# 3",
	"La 3",
	"La# 3",
	"Si 3",

	"Do 4",
	"Do# 4",
	"Ré 4",
	"Ré# 4",
	"Mi 4",
	"Fa 4",
	"Fa# 4",
	"Sol 4",
	"Sol# 4",
	"La 4",
	"La# 4",
	"Si 4",

	"Do 5",
	"Do# 5",
	"Ré 5",
	"Ré# 5",
	"Mi 5",
	"Fa 5",
	"Fa# 5",
	"Sol 5",
	"Sol# 5",
	"La 5",
	"La# 5",
	"Si 5",

	"Do 6",
	"Do# 6",
	"Ré 6",
	"Ré# 6",
	"Mi 6",
	"Fa 6",
	"Fa# 6",
	"Sol 6",
	"Sol# 6",
	"La 6",
	"La# 6",
	"Si 6",

	"Do 7",
	"Do# 7",
	"Ré 7",
	"Ré# 7",
	"Mi 7",
	"Fa 7",
	"Fa# 7",
	"Sol 7",
	"Sol# 7",
	"La 7",
	"La# 7",
	"Si 7",

	"Do 8",
	"Do# 8",
	"Ré 8",
	"Ré# 8",
	"Mi 8",
	"Fa 8",
	"Fa# 8",
	"Sol 8"
};

// International B ----------------------
char * keynames_b[128]=
{
	"C-2",
	"C#-2",
	"D-2",
	"D#-2",
	"E-2",
	"F-2",
	"F#-2",
	"G-2",
	"G#-2",
	"A-2",
	"A#-2",
	"B-2",

	"C-1",
	"C#-1",
	"D-1",
	"D#-1",
	"E-1",
	"F-1",
	"F#-1",
	"G-1",
	"G#-1",
	"A-1",
	"A#-1",
	"B-1",

	"C 0",
	"C# 0",
	"D 0",
	"D# 0",
	"E 0",
	"F 0",
	"F# 0",
	"G 0",
	"G# 0",
	"A 0",
	"A# 0",
	"B 0",

	"C 1",
	"C# 1",
	"D 1",
	"D# 1",
	"E 1",
	"F 1",
	"F# 1",
	"G 1",
	"G# 1",
	"A 1",
	"A# 1",
	"B 1",

	"C 2",
	"C# 2",
	"D 2",
	"D# 2",
	"E 2",
	"F 2",
	"F# 2",
	"G 2",
	"G# 2",
	"A 2",
	"A# 2",
	"B 2",

	"C 3",
	"C# 3",
	"D 3",
	"D# 3",
	"E 3",
	"F 3",
	"F# 3",
	"G 3",
	"G# 3",
	"A 3",
	"A# 3",
	"B 3",

	"C 4",
	"C# 4",
	"D 4",
	"D# 4",
	"E 4",
	"F 4",
	"F# 4",
	"G 4",
	"G# 4",
	"A 4",
	"A# 4",
	"B 4",

	"C 5",
	"C# 5",
	"D 5",
	"D# 5",
	"E 5",
	"F 5",
	"F# 5",
	"G 5",
	"G# 5",
	"A 5",
	"A# 5",
	"B 5",

	"C 6",
	"C# 6",
	"D 6",
	"D# 6",
	"E 6",
	"F 6",
	"F# 6",
	"G 6",
	"G# 6",
	"A 6",
	"A# 6",
	"B 6",

	"C 7",
	"C# 7",
	"D 7",
	"D# 7",
	"E 7",
	"F 7",
	"F# 7",
	"G 7",
	"G# 7",
	"A 7",
	"A# 7",
	"B 7",

	"C 8",
	"C# 8",
	"D 8",
	"D# 8",
	"E 8",
	"F 8",
	"F# 8",
	"G 8"
};

char *octave_b[11]=
{
	"-2",
	"-1",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8"
};

char * keynames_h[128]=
{
	"C -3",
	"C# -3",
	"D -3",
	"D# -3",
	"E -3",
	"F -3",
	"F# -3",
	"G -3",
	"G# -3",
	"A -3",
	"A# -3",
	"H -3",

	"C -2",
	"C# -2",
	"D -2",
	"D# -2",
	"E -2",
	"F -2",
	"F# -2",
	"G -2",
	"G# -2",
	"A -2",
	"A# -2",
	"H -2",

	"C -1",
	"C# -1",
	"D -1",
	"D# -1",
	"E -1",
	"F -1",
	"F# -1",
	"G -1",
	"G# -1",
	"A -1",
	"A# -1",
	"H -1",

	"c 0",
	"c# 0",
	"d 0",
	"d# 0",
	"e 0",
	"f 0",
	"f# 0",
	"g 0",
	"g# 0",
	"a 0",
	"a# 0",
	"h 0",

	"c 1",
	"c# 1",
	"d 1",
	"d# 1",
	"e 1",
	"f 1",
	"f# 1",
	"g 1",
	"g# 1",
	"a 1",
	"a# 1",
	"h 1",

	"c 2",
	"c# 2",
	"d 2",
	"d# 2",
	"e 2",
	"f 2",
	"f# 2",
	"g 2",
	"g# 2",
	"a 2",
	"a# 2",
	"h 2",

	"c 3",
	"c# 3",
	"d 3",
	"d# 3",
	"e 3",
	"f 3",
	"f# 3",
	"g 3",
	"g# 3",
	"a 3",
	"a# 3",
	"h 3",

	"c 4",
	"c# 4",
	"d 4",
	"d# 4",
	"e 4",
	"f 4",
	"f# 4",
	"g 4",
	"g# 4",
	"a 4",
	"a# 4",
	"h 4",

	"c 5",
	"c# 5",
	"d 5",
	"d# 5",
	"e 5",
	"f 5",
	"f# 5",
	"g 5",
	"g# 5",
	"a 5",
	"a# 5",
	"h 5",

	"c 6",
	"c# 6",
	"d 6",
	"d# 6",
	"e 6",
	"f 6",
	"f# 6",
	"g 6",
	"g# 6",
	"a 6",
	"a# 6",
	"h 6",

	"c 7",
	"c# 7",
	"d 7",
	"d# 7",
	"e 7",
	"f 7",
	"f# 7",
	"g 7"
};

// H ----------------------
char *octave_h[11]=
{
	"-3",
	"-2",
	"1",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7"
};


class NoteMaps
{
public:
	NoteMaps()
	{
		map[0]=keynames_b;
		oct[0]=octave_b;

		map[1]=keynames_h;
		oct[1]=octave_h;

		map[2]=keynames_si;
		oct[2]=octave_si;
	}

	char **map[3];
	char **oct[3];
};

static NoteMaps notemaps;

int keyblack[12]=
{
	0,1,0,1,0,0,1,0,1,0,1,0
};

char *GUI::ByteToKeyString(Seq_Song *song,UBYTE key)
{
	if(key<128)
		return notemaps.map[!song?mainsettings->defaultnotetype:song->notetype][key];

	return("???");
}

char *GUI::ByteToOctaveString(Seq_Song *song,UBYTE key)
{
	if(key<128)
		return notemaps.oct[!song?mainsettings->defaultnotetype:song->notetype][key/12];

	return("???");
}

int GUI::KeyStringToByte(Seq_Song *song,char *string)
{
	if(string)
	{
		char *hup=mainvar->GenerateString(string);
		if(hup)
		{
			size_t sl=strlen(hup);
			char *u=hup;
			while(sl--)
			{
				*u=toupper(*u);
				u++;
			}

			TRACE ("Check KS %s\n",hup);

			int mix=!song?mainsettings->defaultnotetype:song->notetype;

			char check[256];

			for(int i=0;i<128;i++)
			{
				if(strcmp(hup,notemaps.map[mix][i])==0)
				{
					delete hup;
					return i;			
				}

				size_t sl=strlen(notemaps.map[mix][i]);
				char *f=notemaps.map[mix][i],*t=check;

				while(sl--)
				{
					if(*f==' ')
						f++;
					else
						*t++=toupper(*f++);
				}

				*t=0;

				TRACE ("Check US %s\n",check);


				if(strcmp(check,hup)==0)
				{
					delete hup;
					return i;			
				}
			}

				delete hup;
		}
	}

	return -1;
}

char *Seq_Main::ConvertSamplesToTime(LONGLONG samples,int timetype,char *string)
{
	if(string)
	{
		if(mainaudio->GetGlobalSampleRate()==0)
		{
			strcpy(string,"??? Samplerate ???");
		}
		else
		{
			char h2[NUMBERSTRINGLEN];

			int h=mainaudio->GetGlobalSampleRate();

			int std=(int)(samples/(h*3600));

			samples-=std*3600*h;

			strcpy(string,mainvar->ConvertIntToChar(std,h2));
			AddString(string,":");

			int min=(int)(samples/(h*60));

			samples-=min*60*h;

			AddString(string,mainvar->ConvertIntToChar(min,h2));
			AddString(string,":");

			int sec=(int)(samples/h);

			AddString(string,mainvar->ConvertIntToChar(sec,h2));
		}
	}

	return string;
}

OSTART Seq_Main::ConvertMilliSecToTicks(double ms)
{
	return (OSTART)floor(ms*INTERNRATEMSMUL+0.5);
}

char *Seq_Main::ConvertTicksToChar(int ticks)
{
	switch(ticks)
	{
	case TICK1nd: // 1/1
		return "1/1";

	case TICK2nd: // 1/1
		return "1/2";

	case TICK4nd: // 1/1
		return "1/4";

	case TICK8nd: // 1/1
		return "1/8";

	case TICK16nd: // 1/1
		return "1/16";

	case TICK32nd: // 1/1
		return "1/32";

	case TICK64nd: // 1/1
		return "1/64";
	}

	return ConvertIntToChar(ticks,tickstochar);
}

char *GUI::GetFPSName(char *string,int flag)
{
	return mainvar->GenerateString("-",smpte_modestring[flag],"FPS");
}

char *GUI::OctaveToString(int octave)
{
	if(octave<=11)
		return octave_si[octave];

	return "???";
}

char *GUI::ByteToControlInfo(UBYTE control,BYTE value,bool withnumber)
{
	if(control<128)
	{
		if(value==-1)
			return withnumber==true?controlnames_number[control]:controlnames[control];

		switch(control)
		{
		case 64:
			{
				if(value<=63)
					return withnumber==true?"Sustain Off (64)":"Sustain Off";
				else
					return withnumber==true?"Sustain On (64)":"Sustain On";
			}
			break;

		case 65:
			{
				if(value<=63)
					return withnumber==true?"Portamento  Off (65)":"Portamento Off";
				else
					return withnumber==true?"Portamento  On (65)":"Portamento On";
			}
			break;

		case 66:
			{
				if(value<=63)
					return withnumber==true?"Sostenuto Off (66)":"Sostenuto Off";
				else
					return withnumber==true?"Sostenuto On (66)":"Sostenuto On";
			}
			break;

		case 67:
			{
				if(value<=63)
					return withnumber==true?"Soft Pedal Off (67)":"Soft Pedal Off";
				else
					return withnumber==true?"Soft Pedal On (67)":"Soft Pedal On";
			}
			break;

		case 68:
			{
				if(value<=63)
					return withnumber==true?"Footswitch normal (68)":"Footswitch normal";
				else
					return withnumber==true?"Footswitch Legato (68)":"Footswitch Legato";
			}
			break;

		case 69:
			{
				if(value<=63)
					return withnumber==true?"Hold 2 Off(69)":"Hold 2 Off";
				else
					return withnumber==true?"Hold 2 On(69)":"Hold 2 On";
			}
			break;

		case 122:
			if(value==0)
				return withnumber==true?"Local Off(122)":"Local Off";
			else
				if(value==127)
					return withnumber==true?"Local On(122)":"Local On";
			break;
		}

		return withnumber==true?controlnames_number[control]:controlnames[control];
	}

	return "???";
}

char *GUI::ByteToControlInfo_NOMSBLSB(UBYTE control,BYTE value)
{
	if(control<128)
	{
		return controlnames_plain[control];
	}

	return "???";
}

void GUI::AddScreen(guiScreen *s)
{
	TRACE ("Add Screen %d %d\n",s->width,s->height);

	if(!activescreen)
		activescreen=s;

	screens.AddEndO(s);
}

guiScreen *GUI::DeleteScreen(guiScreen *s)
{
	if(!s)
		return 0;

	s->closeit=true;

	guiWindow *w=FirstWindow();

	while(w)
	{
		if(w->closeit==false && w->screen==s && w->CanbeClosed()==true)
		{
			CloseWindow(w);
			w=FirstWindow();
		}
		else
			w=w->NextWindow();
	}

	if(s->menu)
		s->menu->RemoveMenu();

#ifdef WIN32
	if(s->hWnd)
	{
		DestroyWindow(s->hWnd);
	}
#endif

	if(s==activescreen)
		activescreen=(guiScreen *)s->NextOrPrev();

	bool setnewtimer=s==FirstScreen()?true:false;
	guiScreen *n=(guiScreen *)screens.RemoveO(s);

	if(setnewtimer==true && FirstScreen())
	{
		SetTimer(FirstScreen()->hWnd, NULL,USER_TIMER_MINIMUM ,NULL);
	}

	return n;
}

void GUI::DeleteAllScreens()
{
	guiScreen *s=FirstScreen();

	while(s)
		s=DeleteScreen(s);
}

void GUI::CheckGUIMessages()
{
	GUIMessage *msg;

	do
	{
		LockGUIMessages();
		msg=(GUIMessage *)guimessages.GetRoot();
		UnlockGUIMessages();

		if(msg)
		{
			CheckUserMessage(0,msg->parm,msg->ptr,msg);

			LockGUIMessages();
			msg=(GUIMessage *)guimessages.RemoveO(msg);
			UnlockGUIMessages();
		}

	}while(msg);
}

void GUI::CancelDragDrop()
{
	dragdropobject=0;
	if(dragdropwindow)CloseWindow(dragdropwindow);
}

void GUI::InitDragDropObject(guiWindow *win,Object *o)
{
	CancelDragDrop();

	if(o)
	{
		dragdropobject=o;

		if(EditData *edit=new EditData)
		{
			// long position;
			edit->song=0;
			edit->win=win;
			
			edit->x=win->GetWindowMouseX();
			edit->y=win->GetWindowMouseY()+GetFontSizeY();
			edit->title=Cxs[CXS_EDIT];
			edit->deletename=false;
			edit->id=0;
			edit->type=EditData::EDITDATA_TYPE_INFOSTRING;
			edit->helpobject=0;
			edit->string=dragdropobject->GetDragDropInfoString();

			int w=100;
			if(edit->string)
			{
				int ws=win->bitmap.GetTextWidth(edit->string);

				if(ws>w)
					w=ws;
			}

			edit->width=w;
			edit->noOSborder=true;

			maingui->EditDataValue(edit);

			dragdropwindow=edit->editdatawin;		
		}
	}
}

bool GUI::CheckDragDropLeftMouseUp(guiWindow *win,guiGadget *db)
{
	if(dragdropwindow)
		{
			CloseWindow(dragdropwindow);
			dragdropwindow=0;
		}

	if(dragdropobject)
	{
		if(db)
			db->guilist->win->DragDrop(db);
		else
			win->DragDrop(0);

		dragdropobject=0;

		return false;
	}

	return true;
}

