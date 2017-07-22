#include "defines.h"

#include "gui.h"
#include "editor.h"
#include "undo.h"
#include "semapores.h"
#include "object_song.h"
#include "objectpattern.h"
#include "songmain.h"
#include "object_track.h"
#include "audiofile.h"
#include "audiohdfile.h"
#include "editfunctions.h"
#include "undofunctions.h"
#include "undofunctions_pattern.h"
#include "undofunctions_track.h"
#include "MIDIPattern.h"
#include "audiopattern.h"
#include "peakbuffer.h"

void Seq_Song::ResetRecordingFlags()
{
	TRACE ("Start ResetRecordingFlags\n");

	// Reset Flags
	Seq_Track *t=FirstTrack();
	while(t)
	{
		t->ResetMIDIRecordPattern();
		t->ResetAudioRecordPattern();

		t->recordcycleindex=0;
		t->record_cyclecreated=false;

		t=t->NextTrack();
	}

	TRACE ("End ResetRecordingFlags\n");
}

bool EditFunctions::AddRecordingToUndo(Seq_Song *song)
{
	TRACE ("Start RefreshRecording\n");
	int tracksdeleted=0,rectracksc=0,recpatternc=0,mediatype=0;

	// Refresh recording tracks

	Seq_Track *t=song->FirstTrack();
	while(t)
	{
		bool recordingfound=false;

		if(t->record_MIDIPattern)
		{
			TRACE ("Refresh MIDI Recording %s\n",t->GetName());

			t->RefreshTrackAfterRecording();

			mediatype|=MEDIATYPE_MIDI;

			if(t->record_cyclecreated==false)
				recpatternc++;

			recordingfound=true;
		}

		Seq_Pattern *p=t->FirstPattern();

		while(p)
		{
			if(p->mediatype==MEDIATYPE_AUDIO_RECORD || p->mediatype==MEDIATYPE_AUDIO)
			{
				AudioPattern *ap=(AudioPattern *)p;

				if(ap->recordpattern==true)
				{	
#ifdef DEBUG
					TRACE ("Refresh Audio Recording %s\n",t->GetName());

					TRACE ("Samples %d\n",ap->audioevent.audioefile->samplesperchannel);

					if(ap->audioevent.audioefile->peakbuffer)
					{
						TRACE ("Rec Peak Buffer Samples %d\n",ap->audioevent.audioefile->peakbuffer->peaksamples);
						LONGLONG h=ap->audioevent.audioefile->samplesperchannel;
						h/=PEAKBUFFERBLOCKSIZE;

						if(h*PEAKBUFFERBLOCKSIZE!=ap->audioevent.audioefile->samplesperchannel)
							h++;

						TRACE ("Rec Soll %d\n",h);

						if(h!=ap->audioevent.audioefile->peakbuffer->peaksamples)
							TRACE ("Rec Peak Error ############################### \n");
					}
#endif

					ap->SetAudioMediaTypeAfterRecording();

					mediatype|=MEDIATYPE_AUDIO;

					if(t->record_cyclecreated==false)
						recpatternc++;

					recordingfound=true;
				}
			}

			p=p->NextRealPattern();
		}

		if(recordingfound==true && t->record_cyclecreated==true)
		{
			rectracksc++;
		}
		else
			if(recordingfound==false && t->record_cyclecreated==true && (!t->FirstPattern())) // Delete New/Empty Created Cycle Recordings Tracks
			{
				tracksdeleted++;
				maingui->RemoveTrackFromGUI(t);
			}

			t=t->NextTrack();
	}

	if(tracksdeleted)
	{
		mainthreadcontrol->LockActiveSong();

		{
			Seq_Track *t=song->FirstTrack();
			while(t)
			{
				bool recordingfound=false;

				if(t->record_MIDIPattern)
				{
					recordingfound=true;
					recpatternc++;
				}

				Seq_Pattern *p=t->FirstPattern();

				while(p)
				{
					if(p->mediatype==MEDIATYPE_AUDIO_RECORD || p->mediatype==MEDIATYPE_AUDIO)
					{
						AudioPattern *ap=(AudioPattern *)p;

						if(ap->recordpattern==true)
							recordingfound=true;
					}

					p=p->NextRealPattern();
				}

				/*
				for(int i=0;i<MAXRECPATTERNPERTRACK;i++)
				{
				if(t->audiorecord_audiopattern[i])
				recordingfound=true;
				}
				*/

				if(recordingfound==false && t->record_cyclecreated==true && (!t->FirstPattern()))
					t=song->DeleteTrack_R(t,true);
				else
					t=t->NextTrack();
			}
		}

		song->CreateQTrackList();

		mainthreadcontrol->UnlockActiveSong();
	}


	if(recpatternc || rectracksc)
	{
		TRACE ("Add Recording->Undo \n");
		// Pattern
		UndoCreatePattern *cp,*pp=0;

		if(recpatternc)
			pp=new UndoCreatePattern[recpatternc];

		if(cp=pp)
		{
			Seq_Track *t=song->FirstTrack();

			while(t)
			{
				if(t->record_cyclecreated==false)
				{
					if(t->record_MIDIPattern)
					{
						cp->newpattern=t->record_MIDIPattern;
						cp->recordeventsadded=cp->newpattern->recordeventsadded;

						if(cp->recordeventsadded==true)
						{
							int c=0;

							// Create Recording Event List
							Seq_Event *e=t->record_MIDIPattern->FirstEvent();
							while(e)
							{
								if(e->flag&EVENTFLAG_RECORDED)c++;
								e=e->NextEvent();
							}

							if(cp->recordeventsaddedcounter=c)
							{
								cp->recordeventslist=new Seq_Event*[c];

								if(cp->recordeventslist)
								{
									c=0;
									e=t->record_MIDIPattern->FirstEvent();
									while(e)
									{
										if(e->flag&EVENTFLAG_RECORDED)cp->recordeventslist[c++]=e;
										e=e->NextEvent();
									}
								}
							}
						}

						cp->track=t;
						cp->position=t->record_MIDIPattern->GetPatternStart();	
						cp++;
					}

					Seq_Pattern *p=t->FirstPattern();

					while(p)
					{
						if(p->mediatype==MEDIATYPE_AUDIO_RECORD || p->mediatype==MEDIATYPE_AUDIO)
						{
							AudioPattern *ap=(AudioPattern *)p;

							if(ap->recordpattern==true)
							{	
								ap->recordpattern=false; // reset

								ap->mediatype=MEDIATYPE_AUDIO;
								ap->internrecorded=true;

								cp->newpattern=p;
								cp->track=t;
								cp->position=ap->GetPatternStart();
								cp++;
							}
						}

						p=p->NextRealPattern();
					}

				}//if cyclecreated

				t=t->NextTrack();

			} //while t

			if(Undo_CreatePattern *ucp=new Undo_CreatePattern(song,pp,recpatternc)) // pp maybe 0
			{
				ucp->nodo=true; // Avoid Undo_CreatePattern->Do Problems !
				ucp->undorecording=true;

				song->undo.OpenUndoFunction(ucp);

				if(rectracksc)
				{
					if(Seq_Track **tlist=new Seq_Track*[rectracksc]){
						int i=0;

						// Add new Created Tracks
						Seq_Track *t=song->FirstTrack();
						while(t)
						{
							if(t->record_cyclecreated==true)
								tlist[i++]=t;

							t=t->NextTrack();
						}

						if(Undo_CreateTracks *uct=new Undo_CreateTracks(song,tlist,rectracksc,-1,0,0,0))
							ucp->AddFunction(uct);
					}
				}

				LockAndDoFunction(song,ucp,true);
			}
			else
				delete pp;
		}
	}


	/*
	else
	if(newtracks)
	{
	if(Undo_CreatePattern *ucp=new Undo_CreatePattern(song,0,0))
	{
	ucp->nodo=true; // Avoid Undo_CreatePattern->Do Problems !
	ucp->undorecording=true;
	song->undo.OpenUndoFunction(ucp);

	// Add new Created Tracks
	t=song->FirstTrack();
	while(t)
	{
	if(t->record_cyclecreated==true)
	{
	Undo_CreateTrack *crt=new Undo_CreateTrack(song,t,song->GetOfTrack(t),false,0,0);
	ucp->AddFunction(crt);
	}

	t=t->NextTrack();
	}

	LockAndDoFunction(song,ucp,true);
	}
	}
	*/

	/*
	if(tracksdeleted)
	{
	Undo_DeleteTrack deleetrack(0);

	}
	*/

		song->ResetRecordingFlags();

	TRACE ("End RefreshRecording\n");

	return false;
}