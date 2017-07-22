#include "MIDIfile.h"
#include "object_song.h"
#include "object_track.h"
#include "objectevent.h"
#include "editfunctions.h"
#include "seqtime.h"
#include "undo.h"
#include "drumevent.h"
#include "drumtrack.h"
#include "MIDIhardware.h"
#include "songmain.h"
#include "settings.h"
#include "audiohardware.h"

#define MAXDEMOLENGTH (4*PPQRATE*60)

MIDIFile::MIDIFile () 
{
	for(int i=0;i<16;i++)
	{
		splittotrack[i]=0;
		splittotrack_created[i]=false;
	}

	sysextrack=0;

	lasttrackname=0;
	error=false;

	patternmode=false;
	topattern=0;

	runningstatus=0;
	chunklen=0;
	deltatime=0;
	ourdeltatime=0;

	deltaoffset=0;

	gmgsfound=false;
	ourfile=0;
	sysexfile=false;
}

OSTART MIDIFile::ReadDelta()
{
	OSTART delta;
	UBYTE byte=getbyte();

	delta=byte&0x7f;

	while ((byte&0x80) && filelength) // Bit 8 set ?
	{
		byte=getbyte ();

		delta*=128; 
		delta+=(byte&0x7f);

		// if(delta>10000){printf("?=???? Delta %d %d %d\n",byte,filelength,chunklen);filelength=0;delta=0;break;}
	}

	return delta;
}

void MIDIFile::ReadDeltaTime()
{
	deltatime +=ReadDelta();
	double h=(double)deltatime;
	h*=deltatimefactor;
	ourdeltatime=deltaoffset+(OSTART)floor(h+0.5);
}

void MIDIFile::WriteDelta(OSTART value)
{
	OSTART ivalue=value,buffer=ivalue;

	buffer &=0x7f;

	while ((ivalue >>= 7) > 0)
	{
		buffer <<= 8;
		buffer |= 0x80;
		buffer += (ivalue & 0x7f);
	}

	while (true)
	{
		file.Save_Chunk((UBYTE)buffer);

		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
	}
}

void MIDIFile::WriteDeltaTime(OSTART dtime)
{
#ifdef _DEBUG
	if(dtime<deltatime)
		MessageBox(NULL,"Illegal Delta","Error",MB_OK);
#endif

	WriteDelta(mainaudio->ConvertInternRateToPPQ(dtime-deltatime));

	deltatime=dtime;
}

void MIDIFile::WriteMeta(OSTART time,UBYTE b1,UBYTE b2)
{
	runningstatus=0;

	WriteDeltaTime(time);

	file.Save_Chunk((UBYTE)0xFF);
	file.Save_Chunk(b1);
	file.Save_Chunk(b2);
}

UBYTE MIDIFile::getbyte()
{
	if(chunklen && filelength)
	{
		UBYTE rb; 

		filelength--;
		chunklen--;

		if(file.Read(&rb,1)!=1)
		{
			filelength=0;
			return 0; // error
		}

		return rb;
	}

	return 0;
}

UWORD MIDIFile::getword()
{
	if((chunklen>=2) && (filelength>=2))
	{
		UBYTE rb[2]; 

		filelength-=2;
		chunklen-=2;

		if(file.Read(rb,2)!=2)
		{
			filelength=0;

			return 0; // Error
		}	

		return rb[1]+(256*rb[0]);
	}

	return 0;
}

int MIDIFile::getuword()
{
	if((chunklen>=4) && (filelength>=4))
	{
		UBYTE rb[4];

		filelength-=4;
		chunklen-=4;

#ifdef WIN32
		if(file.Read(rb,4)!=4)
		{
			filelength=0;
			return 0; // error
		}
#endif

		return rb[3]+(256*rb[2])+(65536*rb[1])+(rb[0]*16777216);
	} 

	return 0;
} 

void MIDIFile::setchunklen()
{
	if(filelength>=4)
	{
		UBYTE rb[4];
		int read=0;

#ifdef WIN32
		read=file.Read(rb,4);
#endif
		filelength-=4;

		if(read!=4)
			filelength=0; // Error

		chunklen=(rb[3]+(256*rb[2])+(65536*rb[1])+(rb[0]*16777216));
	}
	else
		chunklen=0;

	deltatime=ourdeltatime=0; // Reset Delta Times
	trackend=0;
}	

bool MIDIFile::checkHeaderSysEx()
{
	// .snd
	UBYTE rb[4];

	int read=file.Read(rb,4);

	if(read!=4)
	{
		filelength=0;
		file.SeekBegin(0);
		return false;
	}

	if(rb[0]==0xF0)
	{
		file.SeekBegin(0); // SysEx found
		return true;
	}

	return false;
}

bool MIDIFile::checkHeader(char *name) // Check MIDI Chunk Header
{	
	if(filelength>=4)
	{
		UBYTE rb[4];
		int read=file.Read(rb,4);

		if(read!=4)
		{
			filelength=0;
			file.SeekBegin(0);
			return false;
		}

		if(rb[0]==name[0] && rb[1]==name[1] && rb[2]==name[2] && rb[3]==name[3])
		{
			filelength-=4;
			return true;
		}
	}

	file.SeekBegin(0);
	return false;
}

void MIDIFile::AddSysExData(SysEx *e,int syslen)
{
	if(e && syslen)
	{
		if(e->data=new UBYTE[syslen+2])
		{
			int i=1; // first byte
			bool foundf7=false;
			UBYTE c;

			e->data[0]=0xF0;
			e->length=1;

			while(syslen && filelength)
			{
				c=getbyte();

				if(c!=0xF0 && foundf7==false)
				{
					e->data[i++]=c;
					e->length++;

					if(c==0xF7)
						foundf7=true;
				}

				syslen--;
			}

#ifdef _DEBUG
			if(foundf7==false)
				MessageBox(NULL,"MIDI File no F7 found","Error",MB_OK);
#endif

			if(foundf7==false) // Add F7
			{
				e->length++;
				e->data[i]=0xF7;
			}

			//e->CalcSysTicks();
			//e->CheckSysExEnd();

			if(gmgsfound==false) // GM/GS SysEX ?
			{
				//		 GS reset     	  F0 41 10 42 12 40 00 7F 00 41 F7
				//		 GM reset   	  F0 7E 7F 09 01 F7

				switch(e->length)
				{
				case 6: // GM
					{
						if(e->data[0]==0xF0 &&
							e->data[1]==0x7e &&
							e->data[2]==0x7f &&
							e->data[3]==0x09 &&
							e->data[4]==0x01 &&
							e->data[5]==0xF7
							)
							gmgsfound=true;
					}
					break;

				case 11: // GS
					{
						if(e->data[0]==0xF0 &&
							e->data[1]==0x41 &&
							e->data[2]==0x10 &&
							e->data[3]==0x42 &&
							e->data[4]==0x12 &&
							e->data[5]==0x40 &&
							e->data[6]==0x00 &&
							e->data[7]==0x7f &&
							e->data[8]==0x00 &&
							e->data[9]==0x41 &&
							e->data[10]==0xF7
							)
							gmgsfound=true;
					}
					break;
				}
			}
		}
		else
			e->length=0;
	}
}

int MIDIFile::ReadMetaEvent(Seq_Song *song,Seq_Track *track)
{
	int flag=0;
	UBYTE checkbyte=getbyte();
	OSTART metalen=ReadDelta();

	switch (checkbyte) // Typ
	{
	case 0: // Sequence Number
		getbyte();
		getbyte();
		break;

	case 0x7F: // SysEx Data
		{
			int syslen = metalen;

			if(syslen)
			{
				MIDIPattern *newp=0;

				if(patternmode==true)
					newp=topattern;
				else
				{
					if(!track)
					{
						if(sysextrack)
						{
							newp=(MIDIPattern *)sysextrack->FirstPattern(MEDIATYPE_MIDI);

							if(!newp)
								newp=(MIDIPattern *)mainedit->CreateNewPattern(0,sysextrack,MEDIATYPE_MIDI,ourdeltatime,false); // New Pattern for MIDI-Events?
						}
					}
					else
					{
						newp=(MIDIPattern *)track->FirstPattern(MEDIATYPE_MIDI);

						if(!newp)
							newp=(MIDIPattern *)mainedit->CreateNewPattern(0,track,MEDIATYPE_MIDI,ourdeltatime,false); // New Pattern for MIDI-Events?
					}
				}

				if(newp)
				{
					SysEx *newe=newp->NewSysEx(ourdeltatime);

					if(newe)
						AddSysExData(newe,syslen);
				}
				else
					error=true;
			}
		}
		break;

	case 0x2F:                  // End Of Track
		getbyte();
		trackend = 1;
		break;

	case 0x58: // Time Signature 
		{	
			flag=SYNC_EVENT;

			int dnticks=0;

			UBYTE nn = getbyte ();       // nn numerator
			UBYTE dn = getbyte ();       // dd denominator

			// MIDI-Clock per MIDI-Quarter
			UBYTE cc = getbyte ();
			UBYTE bb = getbyte ();       // ppq in denominator

			switch(dn)
			{
			case 0:
				dnticks=TICK1nd;
				break;

			case 1:
				dnticks=TICK2nd;
				break;

			case 2:
				dnticks=TICK4nd;
				break;
			case 3:
				dnticks=TICK8nd;
				break;
			}

			if(patternmode==false)
			{
				if(dnticks)
				{
					if(ourdeltatime==0) // Position 1-1-1-1 ?
					{
						if(Seq_Signature *sig=song->timetrack.FirstSignature())
							sig->ChangeSignature(nn,dnticks);
					}
					else
						song->AddNewSignature(song->timetrack.ConvertTicksToMeasure(ourdeltatime),nn,dnticks);
				}
			}
		}
		break;

	case 0x59: // Key Signature
		{
			flag=SYNC_EVENT;

			getbyte ();              // Sf
			getbyte ();              // mi
		}
		break;

	case 0x51:	// Tempo
		{
			flag=SYNC_EVENT;

			double h = 60000000,h2; // 1 sec = 600.000 microsecs

			Seq_Tempo *ltempo;
			int tempo;

			tempo = getbyte ();
			tempo *= 256;

			tempo += getbyte ();
			tempo *= 256;

			tempo += getbyte ();

			if(patternmode==false)
			{
				// Tempo = microseconds per MIDI-Quarter
				h2 = tempo;
				h /= h2;

				if(ourdeltatime==0) // Position 1-1-1-1 ?
				{
					ltempo=song->timetrack.FirstTempo();
					ltempo->ChangeTempo(song,0,h);
				}
				else
				{
					ltempo=song->timetrack.LastTempo();

					if(ltempo && ltempo->tempo!=h) // New Tempo!= last Tempo in Tempomap
						song->timetrack.AddNewTempo(TEMPOEVENT_REAL,ourdeltatime,h);
				}
			}
		}
		break;

		// SMPTE 
	case 0x54:
		{
			flag=SYNC_EVENT;

			// Len = 5 
			getbyte ();
			getbyte ();
			getbyte ();
			getbyte ();
			getbyte ();
		}
		break;

		// Track->Channel
	case 0x20:
		{
			UBYTE chl;
			chl = getbyte ();      // Channel 1 = Byte 1 !!!
		}
		break;

	case 0x21:                  // ?
		{
			UBYTE chl;
			chl = getbyte ();
		}
		break;

		/*
		00H	Sequence Number
		01H	Text Event
		02H	Copyright Notice
		03H	Sequence/Track Name
		04H	Instrument Name
		05H	Lyrics
		06H	Marker
		07H	Cue Point
		20H	MIDI Channel Prefix
		2FH	End Of Track
		51H	Set Tempo
		54H	SMPTE Offset
		58H	Time Signature
		59H	Key Signature
		7FH	Sequencer Specific Meta Event

		*/

		// Text Event/Copyright
	case 0x01:                     
	case 0x02:						// Copyright
	case 0x03:						// Sequence/Track Name
	case 0x04:
	case 0x05:                     // Lyric
	case 0x06:                     // Blockname
	case 0x07:
		{
			char MIDIstring[MAXTEXTMARKERLEN+1];
			int i=0;

			if(metalen)
			{
				while (metalen-- && filelength)             
				{
					if(i<MAXTEXTMARKERLEN)
						MIDIstring[i++] = getbyte ();
					else
						getbyte();
				}
				MIDIstring[i]=0;

				switch (checkbyte)       
				{
				case 0x02:       // Copyright
					break;

				case 0x01:	// Text
					{
						if(ourdeltatime==0)
						{
							if(patternmode==false)
							{
								if(MIDIstring[0]!=13) // CR
								{
									if(track)
										track->SetName(MIDIstring);
									else
										if(lastwritentrack)
											lastwritentrack->SetName(MIDIstring);
										else
										{
											if(lasttrackname)delete lasttrackname;
											lasttrackname=mainvar->GenerateString(MIDIstring);
										}
								}
							}
						}
						else // Add Text as Text
						{
							flag=SYNC_EVENT;

							if(patternmode==false)
							{
								if(MIDIstring[0]!=13) // CR
									song->textandmarker.AddText(ourdeltatime,MIDIstring,i);
							}
						}
					}
					break;

				case 0x03:
					// Track Name
					if(patternmode==false)
					{
						if(MIDIstring[0]!=13) // CR
						{
							if(track)
								track->SetName(MIDIstring);
							else
								if(lastwritentrack)
									lastwritentrack->SetName(MIDIstring);
								else
								{
									if(lasttrackname)delete lasttrackname;
									lasttrackname=mainvar->GenerateString(MIDIstring);
								}
						}
					}
					break;

				case 0x06:       // Marker
					{
						/*
						if(patternmode==false)
						{
						Seq_Pattern *p=track->LastPattern();

						if(p)p->SetName(&MIDIstring[0]);
						}
						*/
						flag=SYNC_EVENT;

						if(patternmode==false)
						{
							if(MIDIstring[0]!=13) // CR
								song->textandmarker.AddMarker(ourdeltatime,-1,MIDIstring);
						}
					}
					break;


				case 0x05:	// Lyric
					{
						flag=SYNC_EVENT;
						if(patternmode==false)
						{
							if(MIDIstring[0]!=13) // CR
								song->textandmarker.AddText(ourdeltatime,MIDIstring,i);
						}
					}
					break;
				}
			}
		}
		break;

	default:
		while(metalen--)
			getbyte();
		break;

	}

	return flag;
}

bool MIDIFile::ReadBankToPattern(MIDIPattern *mp)
{
	if(UBYTE *data=new UBYTE[filelength+1])
	{
		if(SysEx *newsysex=new SysEx)
		{
			int r=file.Read(data,filelength);

			if(r==filelength)
			{
				data[filelength]=0xF7;

				newsysex->data=data;
				newsysex->length=filelength;
				mp->AddSortEvent(newsysex,mp->GetPatternStart());
			}
			else
			{
				delete data;
				delete newsysex;
			}
		}
		else
			delete data;
	}

	return true;
}

int MIDIFile::ReadEventsToTrackOrPattern(Seq_Song *song,Seq_Track *track,MIDIPattern *mp,int flag)
{
	ReadDeltaTime();

	UBYTE checkbyte=getbyte();

	if(checkbyte==0xFF) // MetaEvent
		return ReadMetaEvent(song,track);

	int rflag=0;
	UBYTE status,userunning,byte1=0,byte2=0;

	// MIDI-Event found

	if(checkbyte<128)
	{
		status=runningstatus; // No new Status Byte use runningstatus
		userunning=1;	
	}
	else
	{
		status=runningstatus=checkbyte;
		userunning=0;	// New Statusbyte found
	}

	switch(status&0xF0)
	{
	case NOTEON:
	case NOTEOFF:
	case POLYPRESSURE:
	case CONTROLCHANGE:
	case PITCHBEND:
		{
			byte1=userunning?checkbyte:getbyte();

			if(byte1>127)
				byte1=127;

			byte2=getbyte();

			if(byte2>127)
				byte2=127;
		}
		break;

	case PROGRAMCHANGE:
	case CHANNELPRESSURE:
		{
			byte1=userunning?checkbyte:getbyte();

			if(byte1>127)
				byte1=127;
		}
		break;
	}

	if(!trackend)
	{
		if((status&0xF0)==NOTEOFF || ((status&0xF0)==NOTEON && byte2==0))  // VeloOff=0 means NOTEOFF !	
		{
			MIDIFile_Note *on=(MIDIFile_Note *)noteonevents.Getc_end();

			while(on)
			{
				if((on->note->status&0x0F)==(runningstatus&0x0F) && on->note->key==byte1)
				{
					// Add Note On
					on->note->velocityoff=byte2;
					on->note->staticostart=on->note->ostart;
					on->note->off.ostart=on->note->off.staticostart=ourdeltatime;

					on->note->AddSortToPattern(on->note->GetPattern());
					
					noteonevents.RemoveO(on);

					break;
				}

				on=(MIDIFile_Note *)on->prev;
			}

#ifdef _DEBUG
			if(!on)
			{
				MessageBox(NULL,"Note Off without Note On","Error",MB_OK);	
			}			
#endif
		}
		else
		{
			if(!mp)
			{
				if((flag&FLAG_AUTOLOAD) || (flag&FLAG_CREATEAUTOSPLITRACK)) // Split > Channel
				{
					switch(status&0xF0)
					{
					case NOTEON:
					case POLYPRESSURE:
					case CONTROLCHANGE:
					case PITCHBEND:
					case PROGRAMCHANGE:
					case CHANNELPRESSURE:
						{
							UBYTE channel=status&0x0F;

							// Channel 0-15
							if(splittotrack[channel])
								track=lastwritentrack=splittotrack[channel];

							if(lasttrackname)
							{
								track->SetName(lasttrackname);
								delete lasttrackname;
								lasttrackname=0;
							}
						}
						break;

					case SYSEX:
						if(sysextrack)
							track=lastwritentrack=sysextrack;
						break;
					}
				}

				if(track)
				{
					mp=(MIDIPattern *)track->FirstPattern(MEDIATYPE_MIDI);

					if(!mp) // Create MIDI Pattern
					{
						mp=(MIDIPattern *)mainedit->CreateNewPattern(0,track,MEDIATYPE_MIDI,ourdeltatime,false); // New Pattern for MIDI-Events?

						if(mp) // Set Pattern Name
						{
							if(char *newn=mainvar->GenerateString("MF_",track->GetName()))
							{
								mp->SetName(newn);
								delete newn;
							}
						}
					}
				}

			}

			if(mp)
			{
				Seq_Event *newevent=0;

				switch (status&0xF0)
				{
				case NOTEON:
					{
								#ifdef MEMPOOLS
		Note *newnote=mainpools->mempGetNote();
#else
		Note *newnote=new Note;
#endif

						if(newnote)
						{
							if(MIDIFile_Note *mon=new MIDIFile_Note)
							{
								newnote->SetPattern(mp);
								newnote->ostart=newnote->staticostart=ourdeltatime;
								newnote->status=status;
								newnote->key=byte1;
								newnote->velocity=byte2;
								mon->note=newnote;

								noteonevents.AddEndO(mon); // Add to Off search List
							}
							else
								newnote->Delete(true);
						}
					}
					break;

				case POLYPRESSURE:
					{
						PolyPressure *prs=new PolyPressure;

						if(newevent=prs)
						{
							prs->status=status;
							prs->key=byte1;
							prs->pressure=byte2;
						}
					}
					break;

				case CONTROLCHANGE:
					if(byte1<128)
					{
							#ifdef MEMPOOLS
		ControlChange *c=mainpools->mempGetControl();
#else
		ControlChange *c=new ControlChange;
#endif

						if(newevent=c)
						{
							c->status=status;
							c->controller=byte1;
							c->value=byte2;
						}
					}
					break;

				case PITCHBEND:
					{
													#ifdef MEMPOOLS
		Pitchbend *p=mainpools->mempGetPitchbend();
#else
		Pitchbend *p=new Pitchbend;
#endif

						if(newevent=p)
						{
							p->status=status;
							p->lsb=byte1;
							p->msb=byte2;
						}
					}
					break;

				case PROGRAMCHANGE:
					{
						ProgramChange *p=new ProgramChange;

						if(newevent=p)
						{
							p->status=status;
							p->program=byte1;
						}
					}
					break;

				case CHANNELPRESSURE:
					{
						ChannelPressure *cp=new ChannelPressure;

						if(newevent=cp)
						{
							cp->status=status;
							cp->pressure=byte1;
						}
					}
					break;

				case SYSEX:
					{
						int syslen=ReadDelta();

						if(syslen)
						{
							if(SysEx *s=mp->NewSysEx(ourdeltatime))
								AddSysExData(s,syslen);
						}
					}
					break;

				} // switch status

				if(newevent)
				{
					mp->AddSortEvent(newevent,newevent->staticostart=ourdeltatime);
				}

			}//if newp

		}// else Events

	}// trackend

	return rflag;
}

bool MIDIFile::CheckFile(camxFile *file,char *filename)
{
	if((ourfile=filename) && file->OpenRead(filename)==true)
	{	
		filelength=file->GetLength();

		if(filelength>22)
		{
			if(checkHeader("MThd")==true)
			{			
				// Read Header
				setchunklen(); // Header Chunk==6

				headformat=getword();
				headtracknumber=getword();
				solution=getword();

				if(solution>=24)
				{
					deltatimefactor=SAMPLESPERBEAT;

					double h=solution;
					deltatimefactor/=h;
				}

				return true;
			}

			if(checkHeaderSysEx()==true){
				sysexfile=true;
				return true;
			}
		}	
	}

	return false;
}

void MIDIFile::DeleteNotFoundNotes()
{
	if(noteonevents.GetRoot())
	{
#ifdef _DEBUG
		MessageBox(NULL,"MIDI File Read Note Offs missing","Error",MB_OK);
#endif
		MIDIFile_Note *mon=(MIDIFile_Note *)noteonevents.GetRoot();

		while(mon)
		{
			mon->note->Delete(true);
			mon=(MIDIFile_Note *)noteonevents.RemoveO(mon);
		}
	}
}

void MIDIFile::ReadMIDIFileToPattern(Seq_Song *song,Seq_Track *track,MIDIPattern *p,char *filename)
{
	if(!p)
		return;

	deltaoffset=p->GetPatternStart();
	patternmode=true;
	topattern=p;

	if(CheckFile(&file,filename)==true)
	{	
		if(sysexfile==true)
		{ 
			// Bank File
			ReadBankToPattern(p);
		}
		else
		{
			// SMF
			while(headtracknumber && filelength>=8 && error==false)
			{
				if(checkHeader("MTrk")==true)
				{
					setchunklen();
					ResetDelta(0);

					while(chunklen && filelength && error==false)
 						ReadEventsToTrackOrPattern(song,track,p);

					DeleteNotFoundNotes();
				}

				headtracknumber--;
			}// while
		}

		TRACE ("Read MIDI To Pattern %d \n",p->GetPatternStart());
	}

	p->CloseEvents();

	file.Close(true);	
}

bool MIDIFile::ReadMIDIFileToSong(Seq_Song *song,char* filename,int flag)
{	
	if(!song)return false;

	bool checkok=false;

	if(CheckFile(&file,filename)==true)
	{	
		checkok=true;

		int trackcount=0;

		if((flag&FLAG_AUTOLOAD) || (flag&FLAG_CREATEAUTOSPLITRACK)) // Build Auto Split List
		{
			if(song->FirstTrack())
			{
				for(int i=0;i<16;i++)
				{
					Seq_Track *f=song->FirstTrack();

					while(f)
					{
						if(int tchl=f->GetFX()->GetChannel())
						{
							tchl--;

							if(tchl==i){

								splittotrack[i]=f;
								TRACE ("Split Channel %d -> Track %s\n",i+1,f->GetName());
								break;
							}
						}

						f=f->NextTrack();
					}
				}
			}
		}

		if(flag&FLAG_CREATEAUTOSPLITRACK)
		{
			// Delete Tempo Map
			song->timetrack.RemoveTempoMap(false);
			//Delete Signature Map
			song->timetrack.RemoveSignatureMap(false);

			sysextrack=mainedit->CreateNewTrack(0,song,0,0,false,false); // Track 1 ->sysex

			if(sysextrack)
				sysextrack->SetName("SysEx Data");

			for(int i=0;i<16;i++)
			{
				if(!splittotrack[i])
				{
					int toindex=1; // 1=SysEx

					for(int i2=i-1;i2>=0;i2--)
					{
						if(splittotrack[i2])
						{
							toindex=song->GetOfTrack(splittotrack[i2])+1;
							break;
						}
					}

					bool create=true;

					Seq_Track *check=song->GetTrackIndex(toindex);

					while(check && create==true)
					{
						bool free=true;

						for(int i1=0;i1<16;i1++)
						{
							if(splittotrack[i1]==check)
							{
								free=false;
								break;
							}
						}

						if(free==true)
						{
							splittotrack[i]=check;
							create=false;
						}

						check=check->NextTrack();
					}

					if(create==true)
					{
						splittotrack[i]=mainedit->CreateNewTrack(0,song,0,toindex,false,false);

						if(splittotrack[i])
						{
							char h32[32],*h=mainvar->GenerateString("Chl ",mainvar->ConvertIntToChar(i+1,h32));

							if(h)
							{
								splittotrack[i]->SetName(h);
								delete h;
							}
						}

						splittotrack_created[i]=true;
					}
				}
			}
		}

		// Set Song Name
		mainvar->GetFileName(filename,songname,sizeof(songname));

		if(sysexfile==true)
		{
			if(Seq_Track *track=mainedit->CreateNewTrack(0,song,0,song->GetCountOfTracks(),false,false))
			{
				MIDIPattern *mp=(MIDIPattern *)mainedit->CreateNewPattern(0,track,MEDIATYPE_MIDI,ourdeltatime,false); // New Pattern for MIDI-Events?

				if(mp)
				{
					ReadBankToPattern(mp);
				}

				gmgsfound=true;
			}
		}
		else
		{
			while(headtracknumber && filelength>=8 && error==false)
			{
				if(checkHeader("MTrk")==true)
				{
					Seq_Track *track=0;

					lastwritentrack=0;
					lasttrackname=0;

					if(flag&FLAG_AUTOLOAD)
						track=song->GetTrackIndex(trackcount);

					if(!track)
					{
						if(!(flag&FLAG_CREATEAUTOSPLITRACK))
							track=mainedit->CreateNewTrack(0,song,0,song->GetCountOfTracks(),false,false);
					}

					if(track || (flag&FLAG_CREATEAUTOSPLITRACK)) // New Track
					{
						int syncevent_counter=0,nonsyncevent_counter=0;

						setchunklen();
						ResetDelta(0);

						while(chunklen && filelength && error==false) // Loop
						{
							int rflag=ReadEventsToTrackOrPattern(song,track,0,flag); // 0= create new Pattern

							if(rflag==MIDIFile::SYNC_EVENT)
								syncevent_counter++;
							else
								nonsyncevent_counter++;
						}

						if(lasttrackname)
						{
							delete lasttrackname;
							lasttrackname=0;
						}

						if(track)
						{
							// Check if single Channel

							if(MIDIPattern *mp=(MIDIPattern *)track->FirstPattern(MEDIATYPE_MIDI))
							{
							//	mp->GetMIDIFX()->SetMIDIChannel(mp->CheckPatternChannel());

								track->GetFX()->SetChannel(mp->CheckPatternChannel());

								// Change Pattern Name

								// SysEx ?
					
								if(Seq_Event *e=mp->FirstEvent())
								{
									while(e && e->GetStatus()==SYSEX)
										e=e->NextEvent();

									if(!e)
										mp->SetName("SysEx");
								}

								trackcount++;
							}
							else
							{
								if(syncevent_counter && (!nonsyncevent_counter))
								{
									song->DeleteTrack(track,true);
									song->CreateQTrackList();
								}
								else
									trackcount++;
							}
						}

						DeleteNotFoundNotes(); // delete note without offs
					}
					else 
						error=true; // unable to create track
				}

				headtracknumber--;
			}// while
		}

		if(flag&FLAG_CREATEAUTOSPLITRACK)
		{
			if(sysextrack && (!sysextrack->FirstPattern()))
				song->DeleteTrack(sysextrack,true);

			for(int i=0;i<16;i++)
			{
				if(splittotrack[i] && splittotrack_created[i]==true && (!splittotrack[i]->FirstPattern()))
					song->DeleteTrack(splittotrack[i],true);
			}	
		}

		song->RefreshMIDIPatternEventIndexs();
		song->CreateQTrackList();

		// Recalc Song Length();
		{
			OSTART sl=song->GetSongLength_Ticks(),
				songl=song->GetSongEnd_Pattern();

			if(songl>song->GetSongLength_Ticks())
			{
				OSTART measure=song->timetrack.ConvertTicksToMeasure(songl+10000);
				song->SetSongLength(measure,false);
			}

			song->textandmarker.InitSongStopMarker(songl+mainvar->ConvertMilliSecToTicks(3000));
		}

		song->timetrack.Close();
		song->textandmarker.Close();

		if(gmgsfound==false && mainsettings->addgsgmtoMIDIfiles!=ADDNOTHING_TOMIDIFILE)
		{
			// Add GM
			if(Seq_Track *sysex=mainedit->CreateNewTrack(0,song,0,0,false,false))
			{
				if(Seq_Pattern *patt=mainedit->CreateNewPattern(0,sysex,MEDIATYPE_MIDI,0,false))
				{
					switch(mainsettings->addgsgmtoMIDIfiles)
					{
					case ADDGM_TOMIDIFILE:
						sysex->SetName("GM Init");
						patt->AddGMSysEx(false,false);
						break;

					case ADDGS_TOMIDIFILE:
						sysex->SetName("GS Init");
						patt->AddGMSysEx(true,false);
						break;

					}// switch
				}//patt
			}// track
		}
	}

	file.Close(true);	

	if(checkok==true){

		if(char *h=mainvar->ClearString(songname)){
			song->SetSongName(h,true);
			delete h;
		}
	}

	return checkok;
}

void MIDIFile::MixGlobalData(Seq_Song *song,OSTART ctime)
{
	OSTART buffertime=ctime;

	enum Flag{
		NONE,
		MARKER,
		TEXT,
		TEMPO,
		SIGNATURE
	};

	while(tempo || sig || text || marker)
	{
		int flag=NONE;

		ctime=buffertime;

		// Text
		if(text && (ctime==-1 || ctime>=text->GetTextStart()) )
		{
			ctime=text->GetTextStart();
			flag=TEXT;
		}

		// Marker
		if(marker && (ctime==-1 || ctime>=marker->GetMarkerStart()) )
		{
			ctime=marker->GetMarkerStart();
			flag=MARKER;
		}

		//Tempo
		if(tempo && (ctime==-1 || ctime>=tempo->GetTempoStart()) )
		{
			ctime=tempo->GetTempoStart();
			flag=TEMPO;
		}

		// Sig
		if(sig && (ctime==-1 || ctime>=sig->GetSignatureStart()) )
		{
			ctime=sig->GetSignatureStart();
			flag=SIGNATURE;
		}

		switch(flag)
		{
		case SIGNATURE:
			{
				WriteMeta(sig->GetSignatureStart(),0x58,0x04);

				file.Save_Chunk((UBYTE)sig->nn);

				switch(sig->dn_ticks)
				{
				case TICK1nd:
					file.Save_Chunk((UBYTE)0);
					break;

				case TICK2nd:
					file.Save_Chunk((UBYTE)1);
					break;

				case TICK4nd:
					file.Save_Chunk((UBYTE)2);
					break;

				case TICK8nd:
					file.Save_Chunk((UBYTE)3);
					break; 
				}

				file.Save_Chunk((WORD)PPQRATEINTERN); // PPQ Rate/ Quarter

				sig=sig->NextSignature();
			}
			break;

		case TEMPO:
			{
				double h=60000000/tempo->tempo;
				int wtempo;
				UBYTE hh,nn,mm;

				wtempo=(int)h;

				hh=(UBYTE)wtempo;
				wtempo>>=8;
				nn=(UBYTE)wtempo;
				wtempo>>=8;
				mm=(UBYTE)wtempo;

				WriteMeta(tempo->GetTempoStart(),0x51,0x03);

				file.Save_Chunk(mm);
				file.Save_Chunk(nn);
				file.Save_Chunk(hh);

				tempo=tempo->NextTempo();
			}
			break;

		case MARKER:
			{
				if(strlen(marker->string)>0)
				{
					UBYTE mklen=strlen(marker->string)>127?127:(UBYTE)strlen(marker->string);

					WriteMeta(marker->GetMarkerStart(),0x06,mklen);
					file.Save_Chunk(marker->string,mklen);
				}

				marker=marker->NextMarker();
			}
			break;

		case TEXT:
			{
				if(strlen(text->string)>0)
				{
					UBYTE tklen=strlen(text->string)>127?127:(UBYTE)strlen(text->string);

					WriteMeta(text->GetTextStart(),0x05,tklen);
					file.Save_Chunk(text->string,tklen);
				}

				text=text->NextText();
			}
			break;

		default:
			return;
		}

	}// while
}

void MIDIFile::SavePatternToSysFile(MIDIPattern *pt,char *filename)
{
	if(pt && pt->IsPatternSysExPattern()==true && file.OpenSave(filename)==true)
	{
		SysEx *sys=(SysEx *)pt->FirstEvent();
		file.Save(sys->data,sys->length);
		file.Close(true);
	}
}

void MIDIFile::SavePatternToFile(MIDIPattern *pt,char *filename)
{
	if(pt && file.OpenSave(filename)==true)
	{
		file.MIDIfile=true;

		file.Save("MThd",4);

		file.OpenChunk();

		file.Save_Chunk((short)0);
		file.Save_Chunk((short)1);
		file.Save_Chunk((short)PPQRATEINTERN);

		file.CloseChunk();

		pt->track->CreateRawEvents(pt);

		ResetDelta(0);

		file.Save("MTrk",4);
		file.OpenChunk();

		size_t i=strlen(pt->GetName());

		if(i>127)i=127;

		WriteMeta(0,0x03,(UBYTE)i); // Track Name
		file.Save_Chunk(pt->GetName(),(UBYTE)i);

		Seq_Event *e=pt->track->FirstRawEvent();

		while(e)
		{
			WriteDeltaTime(e->GetEventStart()-pt->GetPatternStart());

			switch(e->GetStatus())
			{
			case PROGRAMCHANGE: // 1 byte event
			case CHANNELPRESSURE:
				{
					if(runningstatus!=e->status)
						file.Save_Chunk(runningstatus=e->status);

					file.Save_Chunk(e->GetByte1());
				}
				break;

			case NOTEON:
			case POLYPRESSURE:
			case CONTROLCHANGE:
			case PITCHBEND:
				{
#ifdef DEMO
					if(e->GetEventStart()<MAXDEMOLENGTH)
					{
#endif
						if(runningstatus!=e->status)
							file.Save_Chunk(runningstatus=e->status);

						file.Save_Chunk(e->GetByte1());
						file.Save_Chunk(e->GetByte2());
#ifdef DEMO
					}
#endif
				}
				break;

			case NOTEOFF:
				{
					NoteOff_Raw *off=(NoteOff_Raw *)e;

#ifdef DEMO
					if((!off->note) || (off->notestartpos<MAXDEMOLENGTH))
					{
#endif
						UBYTE status=e->status;

						if(off->velocityoff==0)
						{
							if(runningstatus!=status)
								status=NOTEON|e->GetChannel();
						}

						if(runningstatus!=status)
							file.Save_Chunk(runningstatus=status);

						file.Save_Chunk((UBYTE)off->key);
						file.Save_Chunk((UBYTE)off->velocityoff);
#ifdef DEMO
					}
#endif

#ifdef _DEBUG
					if(e->GetEventStart()<off->notestartpos)
						MessageBox(NULL,"Illegal Off<->Note Position","Error",MB_OK);
#endif
				}
				break;

			case SYSEX:
				{
					SysEx *sys=(SysEx *)e;

					file.Save_Chunk((UBYTE)e->status);

					runningstatus=e->status;

					if(sys->length>0)
					{
						WriteDelta(sys->length-1);

						// Dont write F0
						file.Save_Chunk(sys->data+1,sys->length-1);	
					}
					else
						WriteDelta(0);
				}
				break;
			}//switch

			e=e->NextEvent();
		}// while

		WriteMeta(deltatime,0x2F,0);
		file.CloseChunk();

		pt->track->DeleteAllRawEvents();
	}

	file.Close(true);
}

void MIDIFile::WriteRawEvent(Seq_Event *e)
{
	WriteDeltaTime(e->GetEventStart());

	switch(e->GetStatus())
	{
	case PROGRAMCHANGE: // 1 byte event
	case CHANNELPRESSURE:
		{
			if(runningstatus!=e->status)
				file.Save_Chunk(runningstatus=e->status);

			file.Save_Chunk(e->GetByte1());
		}
		break;

	case NOTEON:
	case POLYPRESSURE:
	case CONTROLCHANGE:
	case PITCHBEND:
		{
#ifdef DEMO
			if(e->GetEventStart()<MAXDEMOLENGTH)
			{
#endif
				if(runningstatus!=e->status)
					file.Save_Chunk(runningstatus=e->status);

				file.Save_Chunk(e->GetByte1());

				file.Save_Chunk(e->GetByte2());
#ifdef DEMO
			}
#endif
		}
		break;

	case NOTEOFF:
		{
			NoteOff_Raw *off=(NoteOff_Raw *)e;

#ifdef DEMO
			if((!off->note) || (off->notestartpos<MAXDEMOLENGTH))
			{
#endif
				UBYTE status=e->status;

				if(off->velocityoff==0)
				{
					if(runningstatus!=status)
						status=NOTEON|e->GetChannel();
				}

				if(runningstatus!=status)
					file.Save_Chunk(runningstatus=status);

				file.Save_Chunk((UBYTE)off->key);
				file.Save_Chunk((UBYTE)off->velocityoff);
#ifdef DEMO
			}
#endif

#ifdef _DEBUG
			if(e->GetEventStart()<off->notestartpos)
				MessageBox(NULL,"Illegal Off<->Note Position","Error",MB_OK);
#endif
		}
		break;

	case SYSEX:
		{
			SysEx *sys=(SysEx *)e;

			file.Save_Chunk((UBYTE)e->status);

			runningstatus=e->status;

			if(sys->length>0)
			{
				WriteDelta(sys->length-1);

				// Dont write F0
				file.Save_Chunk(sys->data+1,sys->length-1);	
			}
			else
				WriteDelta(0);
		}
		break;
	}
}

void MIDIFile::SaveSongToFile(Seq_Song *song,char *filename,bool format1)
{
	if(file.OpenSave(filename)==true)
	{
		// Reset Global Data
		marker=song->textandmarker.FirstMarker();
		tempo=song->timetrack.FirstTempo();
		sig=song->timetrack.FirstSignature();
		text=song->textandmarker.FirstText();

		file.MIDIfile=true;

		file.Save("MThd",4);

		file.OpenChunk();

		int tracks;

		if(format1==true)
		{
			file.Save_Chunk((short)1);
			tracks=song->GetCountOfTracks();

			if(!tracks)
				tracks=1;
		}
		else
		{
			file.Save_Chunk((short)0);
			tracks=1;
		}

		file.Save_Chunk((short)tracks);
		file.Save_Chunk((short)PPQRATEINTERN);

		file.CloseChunk();

		// Tempo etc...
		if(!song->FirstTrack()) // Empty song -> create dummy track
		{
			ResetDelta(0);

			file.Save("MTrk",4);
			file.OpenChunk();

			size_t i=strlen("Empty Song");

			WriteMeta(0,0x03,(UBYTE)i); // Track Name
			file.Save_Chunk("Empty Song",i);

			MixGlobalData(song,-1);

			WriteMeta(deltatime,0x2F,0);
			file.CloseChunk();
		}
		else
		{
			// Create Raws
			Seq_Track *track=song->FirstTrack();

			while(track){
				track->CreateRawEvents();
				track=track->NextTrack();
			}

			if(format1==false)
			{
				ResetDelta(0);

				file.Save("MTrk",4);
				file.OpenChunk();

				char *tname=song->GetName();

				if(tname)
				{
					size_t i=strlen(tname);

					if(i>127)i=127;

					WriteMeta(0,0x03,(UBYTE)i); // Track Name
					file.Save_Chunk(tname,i);
				}

				// Mix
				Seq_Event *mixevent;
				do
				{
					Seq_Track *mixtrack;

					mixevent=0;

					// Mix Top Down
					track=song->FirstTrack();
					while(track)
					{
						if(track->rawpointer && 
							(
							(!mixevent) || track->rawpointer->GetEventStart()<mixevent->GetEventStart()
							// ||	(track->rawpointer->GetEventStart()==mixevent->GetEventStart() && track->rawpointer->priority>mixevent->priority)
							)
							)
						{
							mixevent=track->rawpointer;
							mixtrack=track;
						}

						track=track->NextTrack();
					}

					/*
					// MIDI Format 0
					track=song->FirstTrack();
					while(track)
					{
					bool mixglobaldata=true;

					Seq_Event *e=track->FirstRawEvent();

					if((!e) && track==song->FirstTrack())
					MixGlobalData(song,-1);

					while(e)
					{
					bool write;

					if(track==song->FirstTrack())
					write=MixGlobalData(song,e->GetEventStart());
					else
					write=true;

					if(write==true)
					{
					WriteRawEvent(e);

					e=e->NextEvent();
					}
					else
					runningstatus=0; // Meta Event

					}// while e

					// rest of global data
					if(track==song->FirstTrack())
					MixGlobalData(song,-1);
					*/

					if(mixevent)
					{
						MixGlobalData(song,mixevent->GetEventStart());
						WriteRawEvent(mixevent);

						mixtrack->rawpointer=mixevent->NextEvent();
					}
				}
				while(mixevent);

				MixGlobalData(song,-1); // rest

				WriteMeta(deltatime,0x2F,0); // end of track
				file.CloseChunk();
			}
			else
			{
				// MIDI Format 1
				track=song->FirstTrack();
				while(track)
				{
					ResetDelta(0);

					file.Save("MTrk",4);
					file.OpenChunk();

					char *tname=format1==true?track->GetName():song->GetName();

					if(tname)
					{
						size_t i=strlen(tname);

						if(i>127)i=127;
						WriteMeta(0,0x03,(UBYTE)i); // Track Name
						file.Save_Chunk(tname,i);
					}

					bool mixglobaldata=true;

					Seq_Event *e=track->FirstRawEvent();

					if((!e) && track==song->FirstTrack())
						MixGlobalData(song,-1);

					while(e)
					{
						if(track==song->FirstTrack())
							MixGlobalData(song,e->GetEventStart());

						WriteRawEvent(e);
						e=e->NextEvent();
					}

					// rest of global data
					if(track==song->FirstTrack())
						MixGlobalData(song,-1);

					WriteMeta(deltatime,0x2F,0); // end of track
					file.CloseChunk();

					track=track->NextTrack();
				}
			}// end format1

			track=song->FirstTrack();
			while(track)
			{
				track->DeleteAllRawEvents();
				track=track->NextTrack();
			}
		}//else
	}

	file.Close(true);
}

