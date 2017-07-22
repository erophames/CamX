#include "defines.h"
#include "metronome.h"
#include "seqtime.h"
#include "object_song.h"
#include "songmain.h"
#include "MIDIhardware.h"
#include "MIDIoutdevice.h"
#include "object_track.h"
#include "settings.h"
#include "camxfile.h"

void metroClick::Load(camxFile *file)
{
	file->ReadChunk(&on);
	file->ReadChunk(&playback);
	file->ReadChunk(&record);
}

void metroClick::Save(camxFile *file)
{
	file->Save_Chunk(on);
	file->Save_Chunk(playback);
	file->Save_Chunk(record);
}

void metroClick::InitMetroClick(OSTART position,int index)
{
	Seq_Signature *sig=song->timetrack.FindSignatureBefore(position);
	OSTART qposition=mainvar->SimpleQuantize(position,sig->dn_ticks);

	nextclick[index]=qposition<position?qposition+sig->dn_ticks:qposition;
	nextclick_sample[index]=song->timetrack.ConvertTicksToTempoSamples(nextclick[index]);
}

void metroClick::InitMetroClickForce(int index)
{
	Seq_Signature *sig=song->timetrack.FindSignatureBefore(nextclick[index]);
	//OSTART sigdiff=nextclick[index]-sig->GetSignatureStart(),r=sigdiff/sig->dn_ticks;

	//r++;
	//r*=sig->dn_ticks;

	nextclick[index]+=sig->dn_ticks;//sig->GetSignatureStart()+r;
	nextclick_sample[index]=song->timetrack.ConvertTicksToTempoSamples(nextclick[index]);
}

bool metroClick::CheckIfHi(OSTART click)
{
	Seq_Signature *sig=song->timetrack.FindSignatureBefore(click);

	OSTART diff=click-sig->GetSignatureStart(),h=diff/sig->measurelength;
	h*=sig->measurelength;
	return h==diff?true:false;
}

void metroClick::RefreshTempoBuffer()
{
	nextclick_sample[AUDIOMETROINDEX]=song->timetrack.ConvertTicksToTempoSamples(nextclick[AUDIOMETROINDEX]);
	nextclick_sample[MIDIMETROINDEX]=song->timetrack.ConvertTicksToTempoSamples(nextclick[MIDIMETROINDEX]);
}

int metroClick::GetPreCounterDone()
{
	Seq_Signature *sig=song->timetrack.FindSignatureBefore(song->songposition);
	int beats=beat[MIDIMETROINDEX]-1; // Min 5 Beats in 4/4

	beats/=sig->nn;

	if(beats>=0)return beats;

	return 0;
}

void metroClick::SendClick(bool hi)
{
	Seq_MetroTrack *mt=song->FirstMetroTrack();

	while(mt)
	{
		if(mt->GetMute()==false && mt->metrosendtoMIDI==true)
		{
			Note note;
			MIDIOutputDevice *device=0;

			if(hi==true){

				note.status=NOTEON|(mt->metrochl_m-1);
				note.key=mt->metrokey_m;

				int velo=mt->metrovelo_m;

				velo+=mt->t_trackeffects.GetVelocity_NoParent();

				if(velo<1)velo=1;
				else
					if(velo>127)
						velo=127;

				note.velocity=velo;
				note.velocityoff=mt->metrovelooff_m;

				device=mainMIDI->MIDIoutports[mt->metroport_m].outputdevice;
			}
			else{

				note.status=NOTEON|(mt->metrochl_b-1);
				note.key=mt->metrokey_b;

				int velo=mt->metrovelo_b;

				velo+=mt->t_trackeffects.GetVelocity_NoParent();

				if(velo<1)velo=1;
				else
					if(velo>127)
						velo=127;


				note.velocity=velo;
				note.velocityoff=mt->metrovelooff_b;
				device=mainMIDI->MIDIoutports[mt->metroport_b].outputdevice;
			}

			note.ostart=0;
			note.off.ostart=TICK32nd; // Metro Click Length

			note.SetMIDIImpulse(mt);
			note.SendToDevicePlaybackUser(device,song,Seq_Event::STD_CREATEREALEVENT);
		}

		mt=mt->NextMetroTrack();
	}


}