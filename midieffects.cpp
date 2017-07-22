#include "songmain.h"
#include "objectevent.h"
#include "MIDIeffects.h"
#include "gui.h"
#include "chunks.h"
#include "camxfile.h"

char *smptestring[]=
{
	"24",
	"25",
	"48",
	"50",
	"29.97",
	"30",
	"29.97 DF",
	"30 DF",
	"23.9",
	"24.9",
	"59.9",
	"60",
	0
};

int smptemode[]=
{
	Seq_Pos::POSMODE_SMPTE_24,
	Seq_Pos::POSMODE_SMPTE_25,
	Seq_Pos::POSMODE_SMPTE_48,
	Seq_Pos::POSMODE_SMPTE_50,
	Seq_Pos::POSMODE_SMPTE_2997,
	Seq_Pos::POSMODE_SMPTE_30,
	Seq_Pos::POSMODE_SMPTE_2997df,
	Seq_Pos::POSMODE_SMPTE_30df,
	Seq_Pos::POSMODE_SMPTE_239,
	Seq_Pos::POSMODE_SMPTE_249,
	Seq_Pos::POSMODE_SMPTE_599,
	Seq_Pos::POSMODE_SMPTE_60
};

char *smpte_modestring[]=
{
	0,0,0,0,0,
	"24 FPS",
	"25 FPS",
	"48 FPS",
	"50 FPS",
	"29.97 FPS",
	"30 FPS",
	"29.97 DF FPS",
	"30 DF FPS",
	"23.9 FPS",
	"24.9 FPS",
	"59.9 FPS",
	"60 FPS",
};

double SMPTE_FPS[]=
{
	0,0,0,0,0,
	24.0, 
	25.0,
	48.0, 
	50.0,
	29.97,
	30.0,
	29.97,
	30.0,
	23.976,
	24.976,
	59.94,
	60,
};

char *quantstr[QUANTNUMBER]=
{
	"1·",                         /* PPQRATE*6 */
	"1",                          /* PPQRATE*4 */
	"/2·",                        /* PPQRATE*3 */
	"1³",                         /* PPQRATE*2.6666666 */
	"/2",                         /* PPQRATE*2 */
	"/4·",                        /* PPQRATE*1.5 */
	"/2³",                        /* PPQRATE*1,33333333 */
	"/4",                         /* PPQRATE */
	"/8·",                        /* PPQRATE*0.75 */
	"/4³",                        /* PPQRATE*0.666666 */
	"/8",                         /* PPQRATE/2 */
	"/16·",                       /* PPQRATE*0.375 */
	"/8³",                        /* PPQRATE*0.3333333 */
	"/16",                        /* PPQRATE/4 */
	"/32·",                       /* PPQRATE*0.1875 */
	"/16³",                       /* PPQRATE*0.1666666666 */
	"/32",                        /*  PPQRATE/8 */
	"/64·",                       /*  PPQRATE/10.66666666 */
	"/32³",                       /*  PPQRATE/12 */
	"/64",                        /*  PPQRATE/16 */
	"/96",                        /*  PPQRATE/21,3333333 */
	"/64³",                       /*  PPQRATE/24 */
	"/128",                       /*  PPQRATE/32 */
	"/192",                       /*  PPQRATE/42,666666 */
	"/256",                       /*  PPQRATE/64 */
	"/384",                       /*   PPQRATE/96 */
	"/512",                       /*   PPQRATE/128 */
	"/768",                       /*   PPQRATE/192 */
	"1024",                       /*   PPQRATE/256 */
	"1536"                        /*   PPQRATE/384 */
};

char *measure_str[]=
{
	".",
	".",
	".",
	".",
	"."
};

char *measure_str_empty[]=
{
	"",
	"",
	"",
	"",
	""
};

char *smpte_str[]=
{
	":", // std
	":", // min
	":", // sec
	";", // frame
	"" // QF
};

char *sec_str[]=
{
	":", // std
	":", // min
	".", // sec
	"",
	""
};

OSTART quantlist[QUANTNUMBER];

void MIDIEffects::SetMIDIChannel(int v)
{
	MIDI_channel.channel=v;
}

void MIDIEffects::SetVelocity(int v)
{
	if(v>127)
		v=127;
	else
		if(v<-127)
			v=-127;

	MIDI_velocity.velocity=v;
}

void MIDIEffects::SetTranspose(int v)
{
	MIDI_transpose.transpose=v;
}

MIDIEffects::MIDIEffects()
{
	track=0;
	pattern=0;
	mute=false;
	MIDIbank_progselectsend=false;
}

void MIDIEffects::Load(camxFile *file)
{
	file->ReadChunk(&MIDI_channel.channel);
	file->ReadChunk(&MIDI_transpose.transpose);
	file->ReadChunk(&MIDI_velocity.velocity);
	file->ReadChunk(&mute);

	file->CloseReadChunk();

	filter.Load(file);
}

void MIDIEffects::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_MIDIEFFECTS);

	file->Save_Chunk(MIDI_channel.channel);
	file->Save_Chunk(MIDI_transpose.transpose);
	file->Save_Chunk(MIDI_velocity.velocity);
	file->Save_Chunk(mute);

	file->CloseChunk();

	filter.Save(file);
}

bool MIDIEffects::Compare(MIDIEffects *c)
{
	if(c->GetVelocity()!=GetVelocity() ||
		c->GetTranspose()!=GetTranspose() ||
		c->GetChannel()!=GetChannel() // ||
		)
		return true;

	return false;
}

void MIDIEffects::Clone(MIDIEffects *to)
{
	filter.Clone(&to->filter);

	to->MIDI_transpose.transpose=MIDI_transpose.transpose;
	to->MIDI_velocity.velocity=MIDI_velocity.velocity;
	to->MIDI_channel.channel=MIDI_channel.channel;
	to->mute=mute;
}