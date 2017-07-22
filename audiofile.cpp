#include "defines.h"
#include <stdio.h>
#include <string.h>
#include "songmain.h"

#include "object_song.h"
#include "audiofile.h"
#include "audiohardware.h"
#include "gui.h"
#include "semapores.h"
#include "audiothread.h"
#include "arrangeeditor.h"
#include "MIDIoutproc.h"
#include "object_track.h"

#ifdef WIN32
#include "asio/asio.h"
#endif

#include "audiopeakbuffer.h"
#include "vstplugins.h"
#include "audioauto_volume.h"
#include "editfunctions.h"
#include "languagefiles.h"
#include "audiohdfile.h"
#include "runningaudiofile.h"
#include "decoder.h"
#include "editdata.h"
#include "initplayback.h"
#include "crossfade.h"
#include "audiopattern.h"
#include "chunks.h"

// Main Wave File Deleted, close all Editors useing AudioHDFile

void AudioHDFile::AudioFileDeleted()
{
	//	Seq_Project *project;
	//	Seq_Song *song;
	//	Seq_Track *track;
	//	Seq_Pattern *pattern;

	// AudioPattern *af;

	// Check GUI
	/*
	project=mainvar->FirstProject();

	while(project)
	{
	song=project->FirstSong();

	while(song)
	{
	track=song->FirstTrack();

	while(track)
	{
	pattern=track->FirstPattern(MEDIATYPE_AUDIO);

	while(pattern)
	{
	if(pattern->mediatype==MEDIATYPE_AUDIO)
	{
	af=(AudioPattern *)pattern;

	if(af->audiohdfile==this)
	maingui->RemoveAudioPatternFromGUI(af);
	}

	pattern=pattern->NextPattern(MEDIATYPE_AUDIO);
	}

	track=track->NextTrack();
	}

	song=song->NextSong();
	}

	project=project->NextProject();
	}
	*/
}

void mainAudio::ChangeAudioFileToFile(char *oldfilename,char *newfilename)
{
	if(oldfilename && newfilename)
	{
		bool found=false;
		AudioHDFile *c=FirstAudioHDFile();

		while(c)
		{
			if(strcmp(c->GetName(),oldfilename)==0)
			{
				// Replace HDFIle
				found=true;
				c->ReplaceFileName(newfilename);
			}

			c=c->NextHDFile();
		}
	}
}

int mainAudio::GetCountOfRecordingFiles(int index)
{
	int c=0;

	AudioHDFile *ahd=FirstAudioRecordingFile();
	while(ahd)
	{
		if(ahd->recordingactive==true)
		{
			if( (index==RECORD_INDEX_DEVICE && ahd->istrackrecordingfile==false) ||
				(index==RECORD_INDEX_TRACKTRACK && ahd->istrackrecordingfile==true)
				)
				c++;
		}

		ahd=ahd->NextHDFile();
	}

	return c;
}

bool mainAudio::CheckIfAudioFileInfo(char *name,AudioFileInfo *info)
{
	return CheckIfAudioFile(name,0,false,info);
}

bool mainAudio::CheckIfAudioFile(char *name,int *to_samplerate,bool checkdecoder,AudioFileInfo *info)
{
	if(!name)return false;

	char buffer[256];
	bool ok=false;

	if(to_samplerate)*to_samplerate=0;

	buffer[0]='0';

	AudioFileInfo *ainfo,afinfo;

	ainfo=info;

	if(!ainfo)
		ainfo=&afinfo;

	camxFile file;

	if(file.OpenRead(name)==true)
	{
		ainfo->filelength=file.GetLength();

		TestForWaveFormat(&file,ainfo);
		if(ainfo->type!=TYPE_UNKNOWN)
			goto check;

		TestForAIFFFormat(&file,ainfo);

check:
		if(ainfo->type!=TYPE_UNKNOWN)
		{
			ok=ainfo->m_ok;
			if(to_samplerate)*to_samplerate=ainfo->samplerate;
		}

#ifdef OLDIE

		file.Read(buffer,12);

		//Wave ?
		if (buffer[0] == 'R' && buffer[1] == 'I' && buffer[2] == 'F' && buffer[3] == 'F')
		{
			USHORT *us;
			int *uw;

			if(buffer[8]=='W' && buffer[9]=='A' && buffer[10]=='V' && buffer[11]=='E')
			{
				uw=(int *)&buffer[4]; // Clen RIFF
				clen=*uw;
				buffer[0]='0';
#ifdef WIN32	
				file.Read(buffer,8);
#endif			
				if(buffer[0]=='f' && buffer[1]=='m' && buffer[2]=='t')
				{
					uw=(int *)&buffer[4]; // Clen FMT
					clen=*uw;

					if(clen>0 && clen<255)
					{
						if(info)
							info->type=AudioFileInfo::TYPE_WAVE;

						buffer[0]='0';
#ifdef WIN32
						file.Read(buffer,clen);
#endif	
						us=(USHORT *)&buffer[2];
						channels=*us;

						if(info)
							info->channels=channels;

						uw=(int *)&buffer[4];
						samplerate=*uw;

						if(info)
							info->samplerate=samplerate;

						us=(USHORT *)&buffer[14];
						bits=*us;

						if(info)
							info->bits=bits;

						if(bits==16 || bits==20 || bits==24 || bits==32 || bits==64)
						{
							ok = true;

							if(to_samplerate)*to_samplerate=samplerate;

							LONGLONG datastart=0;

							do // Find data chunk
							{
								buffer[0]='0';
								file.Read(buffer,8);

								uw=(int *)&buffer[4]; // Clen
								clen=*uw;

								if(clen==0)
									ok=false;
								else
									if(buffer[0]=='d' && buffer[1]=='a' && buffer[2]=='t' && buffer[3]=='a'){

										datastart=file.Seek(0,CFile::current);
										LONGLONG dataend=datastart+clen;

										if(dataend>flen)
										{
											if(flen-datastart>datastart || mainaudio->ignorecorrectaudiofiles==true)
											{
											}
											else
											{
												ok=false; //error
												datachunkerror=true;
											}
											//m_ok=false; //error
										}

										//ok=false; //error
										//datachunkerror=true;
									}
									else // not a data chunk ?
									{
										LONGLONG pos=file.Seek(0,CFile::current);

										if(pos+clen>flen)
										{
											ok=false; // error
											datachunkerror=true;
										}
										else
											file.Seek(clen,CFile::current);						
									}

							}while(datastart==0 && ok==true);	
						}
					}
				}
			}// if WAVE
		}//ifRIFF
#endif

		file.Close(true);

		if(ok==false && checkdecoder==true)
		{
			Decoder decoder;
			ok=decoder.CheckFile(name);
		}

	}// if file

	return ok;
}

void AudioPattern::MovePatternData(OSTART diff,int flag) // v
{
	audioevent.ostart+=diff;

	if(!(flag&Seq_Pattern::MOVENO_STATIC))
		audioevent.staticostart+=diff;

	/*
	if(track)
	track->song->audiosystem.RemovePatternFromAudioLists(this);
	*/

	//volumecurve.MoveCurve(diff);
}

AudioPattern::AudioPattern()
{
	id=OBJ_AUDIOPATTERN;
	mediatype=MEDIATYPE_AUDIO;

	accesscounter=0;
	audioevent.pattern=this;
	audioevent.index=0; // always 0, 1 Pattern=1 Event

	internrecorded=false;
	runningfile=0;
	waitforresample=false;
	waitforresamplefile=0;
	waitforresampleendfile=0;
	waitforregion=0;

	offsetregion.regionstart=offsetregion.regionend=0;

	volumecurve.audiopattern=this;
	punch1=punch2=false;
};

void AudioPattern::SelectAudioFile(guiWindow *win,char *filename)
{
	bool newfilename=false;

	if(!filename)
	{
		camxFile req;

		if(req.OpenFileRequester(0,win,Cxs[CXS_SELECTAUDIOFILE],req.AllFiles(camxFile::FT_WAVES),true)==true)
		{
			filename=mainvar->GenerateString(req.filereqname);
			newfilename=true;
		}
	}

	if(filename)
	{
		if(mainaudio->CheckIfAudioFile(filename)==true)
		{
			AudioHDFile *selecthdfile=mainaudio->FindAudioHDFile(filename);

			if(!selecthdfile)
				selecthdfile=mainaudio->AddAudioFile(filename,true);

			if(selecthdfile && selecthdfile!=audioevent.audioefile)
				mainedit->ReplaceAudioPatternHDFile(this,selecthdfile);
		}
	}

	if(newfilename==true && filename)
		delete filename;
}

void AudioPattern::ReplaceAudioHDFileWith(AudioHDFile *newhd,AudioRegion *region,int flag)
{
	StopAllofPattern();
	CloseAudioFile(true);

	//	if(audioevent.audioefile)
	//		audioevent.audioefile->Close();

	if(!newhd)region=0;
	audioevent.audioefile=newhd;

	if(region)
	{
		if(audioevent.audioregion)
			audioevent.audioregion->FreeMemory();
		else
			audioevent.audioregion=new AudioRegion;

		if(audioevent.audioregion)
		{
			region->CloneTo(audioevent.audioregion);
			audioevent.audioregion->InitRegion();
		}
	}

	//audioevent.audioregion=region;

	offsetregion.r_audiohdfile=newhd;
	offsetregion.InitRegion();

#ifdef OLDIE
	// Keep Region ?
	if(audioevent.audioregion && (!region) && (flag&REPLACEAUDIOFILE_CHECKFORREGION)) // oldregion
	{
		bool found=false;
		AudioRegion *nr=newhd->FirstRegion(); // Find new region

		while(nr){
			if(nr->clonedfrom==audioevent.audioregion)
			{
				audioevent.audioregion=nr;
				found=true;
				break;
			}

			nr=nr->NextRegion();
		}

		if(found==false)
			audioevent.audioregion=0; // no <> region found !
	}
	else
		audioevent.audioregion=region;
#endif

	if(newhd)
		newhd->Open();

	InitDefaultVolumeCurve(true);

	// Clones
	Seq_ClonePattern *c=FirstClone();

	while(c)
	{
		AudioPattern *cp=(AudioPattern *)c->pattern;

		cp->track->checkcrossfade=true;
		cp->ReplaceAudioHDFileWith(newhd,region);
		cp->SetName(newhd->GetFileName());

		c=c->NextClone();
	}
}


void AudioPattern::SetName(char *newname) //v
{
	if(patternname){
		delete patternname;
		patternname=0;
	}

	if(newname)
		patternname=mainvar->ClearString(newname);
}

void AudioPattern::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);

	//// Pattern Volune
	ARES expatternvolume;
	file->ReadChunk(&expatternvolume);

	// OffSet
	file->ReadChunk(&useoffsetregion);
	file->ReadChunk(&offsetregion.regionstart);
	file->ReadChunk(&offsetregion.regionend);
	file->ReadChunk(&offsetstartoffset);

	file->CloseReadChunk();

	LoadStandardEffects(file);
	audioevent.Load(file);

	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_AUDIOCROSSFADES)
	{
		file->ChunkFound();
		int nr=0;
		file->ReadChunk(&nr);
		file->CloseReadChunk();

		while(nr--)
		{
			file->LoadChunk();
			if(file->GetChunkHeader()==CHUNK_AUDIOCROSSFADE)
			{
				file->ChunkFound();

				if(Seq_CrossFade *cf=new Seq_CrossFade)
				{
					cf->Load(file);
					AddCrossFade(cf);
				}

				file->CloseReadChunk();
			}
		}

	}
}

void AudioPattern::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOPATTERN);
	file->Save_Chunk((CPOINTER)this);

	// Pattern Volune
	ARES expatternvolume=0;
	file->Save_Chunk(expatternvolume);

	// OffSet
	file->Save_Chunk(useoffsetregion);
	file->Save_Chunk(offsetregion.regionstart);
	file->Save_Chunk(offsetregion.regionend);
	file->Save_Chunk(offsetstartoffset);

	file->CloseChunk();

	SaveStandardEffects(file); // Pattern Effects

	// Save Audio Event
	audioevent.Save(file);

	//Save CrossFades
	if(FirstCrossFade())
	{
		file->OpenChunk(CHUNK_AUDIOCROSSFADES);
		file->Save_Chunk(crossfades.GetCount());
		file->CloseChunk();

		Seq_CrossFade *cf=FirstCrossFade();
		while(cf)
		{
			cf->Save(file);
			cf=cf->NextCrossFade();
		}
	}

}

void AudioPattern::Load_Ex(camxFile *file,Seq_Song *tosong)
{
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUDIOHDFILE)
	{
		file->ChunkFound();

		if(AudioHDFile *hd=new AudioHDFile)
		{
			CPOINTER old;

			hd->Load(file,&old,tosong);

			if(hd->GetName())
			{
				if(hd->camximport==true || hd->camxrecorded==true)
				{
					// Song Intern Audio Files
					if(hd->filenotfound==false)
						hd->CreatePeakFile(true);

					audioevent.audioefile=hd;
				}
				else
					if(AudioHDFile *oldhd=mainaudio->FindAudioHDFile(hd->GetName()))
					{
						delete hd; // same file exists

						if(oldhd->filenotfound==false)
							oldhd->CreatePeakFile(true);

						audioevent.audioefile=oldhd;
					}
					else
					{
						if(mainaudio->CheckIfAudioFile(hd->GetName())==true)
						{
							mainaudio->AddAudioFile(hd,false); // new hd file
							hd->Open();
							audioevent.audioefile=hd;
						}
						else
							delete hd; // no audio file
					}
			}
			else
				delete hd;
		}

		file->CloseReadChunk();
	}
}

void AudioPattern::Save_Ex(camxFile *file)
{
	if(audioevent.audioefile)
	{	
		file->flag=SAVE_NOREGIONS; // dont save all regions
		audioevent.audioefile->Save(file); // HD-File
		file->flag CLEARBIT SAVE_NOREGIONS;
	}
}

Seq_Pattern *AudioPattern::CreateLoopPattern(int loop,OSTART pos,int flag)
{
	return CreateClone(pos-GetPatternStart(),flag);
}

void AudioPattern::Delete(bool full)
{
	DeleteAllCrossFades(full);

	//	StopAllofPattern();

	track=0; // undo or buffer

	CloseAudioFile(full);

	if(full==true)
	{
		if(audioevent.audioregion)
		{
			audioevent.audioregion->FreeMemory();
			delete audioevent.audioregion;
			audioevent.audioregion=0;
		}

		DeleteClones();
		//	volumecurve.DeleteAllVolumeObjects();

		if(patternname)delete patternname;
		if(waitforresamplefile)delete waitforresamplefile;
		if(waitforresampleendfile)delete waitforresampleendfile;

		delete this;
	}
}

bool AudioHDFile::DeleteFileOnHD()
{
	bool ok=false;

	LockO();
	if(name)ok=mainvar->DeleteAFile(name);
	UnlockO();

	return ok;
}

bool AudioPattern::OpenAudioPattern()
{
	if(!audioevent.audioefile)
		return false;

	if(mediatype!=MEDIATYPE_AUDIO_RECORD && audioevent.iofile.OpenRead(audioevent.audioefile->GetName())==false) // mediatype Audio Record always open
		return false;

	audioevent.sampleposition=audioevent.openseek;

	// Seek Forward
	LONGLONG end;

	if(itsaclone==true || itsaloop==true)
	{
		AudioPattern *mainaudiopattern=(AudioPattern *)mainpattern;

		// Offset
		if(mainaudiopattern->useoffsetregion==true)
		{
			audioevent.sampleposition+=mainaudiopattern->offsetregion.regionstart;
			end=mainaudiopattern->offsetregion.regionend;
		}
		else
			// Region
			if(mainaudiopattern->audioevent.audioregion){

				audioevent.sampleposition +=mainaudiopattern->audioevent.audioregion->regionstart;
				end=mainaudiopattern->audioevent.audioregion->regionend;	
			}
			else
				end=audioevent.audioefile->samplesperchannel;
	}
	else
	{
		// Offset
		if(useoffsetregion==true)
		{
			audioevent.sampleposition+=offsetregion.regionstart;
			end=offsetregion.regionend;
		}
		else
			// Region
			if(audioevent.audioregion){

				audioevent.sampleposition+=audioevent.audioregion->regionstart;
				end=audioevent.audioregion->regionend;	
			}
			else
				end=audioevent.audioefile->samplesperchannel;
	}

	if(audioevent.sampleposition<end)
	{
		LONGLONG seekto=audioevent.sampleposition;

		seekto*=audioevent.audioefile->samplesize_all_channels;
		seekto+=audioevent.audioefile->datastart;

		if(mediatype!=MEDIATYPE_AUDIO_RECORD)
		{
			if(seekto)
				audioevent.iofile.SeekBegin(seekto);
		}

		return true;
	}

	return false;
}

void AudioPattern::CloseAudioFile(bool full)
{
	if(audioevent.iofile.status!=CAMXFILE_STATUS_NOTOPEN)
	{
		audioevent.iofile.Close(true);
	}
}

int AudioPattern::QuantizePattern(QuantizeEffect *fx)
{
	OSTART pos=fx->Quantize(audioevent.staticostart);

	if(pos!=GetPatternStart())
	{
		StopAllofPattern();
		MovePattern(pos-GetPatternStart(),Seq_Pattern::MOVENO_STATIC);

		return 1;
	}

	return 0;
}

void AudioPattern::RefreshAfterPaste()
{
	if(audioevent.audioefile)
		audioevent.audioefile->Open();
	else
	{
		if(waitforresampleendfile)
		{
			if(audioevent.audioefile=mainaudio->FindAudioHDFile(waitforresampleendfile))
			{
				audioevent.audioefile->Open();
				delete waitforresampleendfile;
				waitforresampleendfile=0;

				if(waitforresamplefile){
					delete waitforresamplefile;
					waitforresamplefile=0;
				}

				waitforresample=false;
			}	
		}
	}
}

bool AudioPattern::SeekSamplesCurrent(int seeksamples)
{
	if(!audioevent.audioefile)return false;

	if(audioevent.sampleposition+seeksamples<0)
		return false;

	if(audioevent.sampleposition+seeksamples>=audioevent.audioefile->samplesperchannel)
		return false;

	audioevent.sampleposition+=seeksamples;

	if(mediatype!=MEDIATYPE_AUDIO_RECORD)
		audioevent.iofile.SeekBegin(audioevent.sampleposition*audioevent.audioefile->samplesize_all_channels*seeksamples+audioevent.audioefile->datastart);

	return true;
}

int AudioPattern::FillAudioBuffer(Seq_Song *song,/*AudioDevice *device,*/AudioHardwareBuffer *buffer,RunningAudioFile *raudiofile /*,bool seek*/)
{
	if(!audioevent.audioefile)return 0;

	LONGLONG endsample;

	if(itsaclone==true || itsaloop==true)
	{
		AudioPattern *mainaudiopattern=(AudioPattern *)mainpattern;

		if(mainaudiopattern->useoffsetregion==true)
			// Offset File
			endsample=mainaudiopattern->offsetregion.regionend;
		else // Region/File
			endsample=mainaudiopattern->audioevent.audioregion?mainaudiopattern->audioevent.audioregion->regionend:audioevent.audioefile->samplesperchannel;
	}
	else
	{
		if(useoffsetregion==true)
			// Offset File
			endsample=offsetregion.regionend;
		else // Region/File
			endsample=audioevent.audioregion?audioevent.audioregion->regionend:audioevent.audioefile->samplesperchannel;
	}

	if(audioevent.sampleposition<endsample)
	{	
		int rsamples=song->stream_buffersize,offset=raudiofile->streamstartsampleoffset;

		if(offset)
		{
			if(offset>=buffer->samplesinbuffer)
			{
				offset=buffer->samplesinbuffer-1;

#ifdef DEBUG
				MessageBox(NULL,"Illegal Audio Buffer OffSet","Error",MB_OK);
#endif
			}

			if(offset+rsamples>buffer->samplesinbuffer)
				rsamples=buffer->samplesinbuffer-offset;
		}

		if(rsamples>buffer->samplesinbuffer)
		{
			rsamples=buffer->samplesinbuffer;

#ifdef DEBUG
			MessageBox(NULL,"Illegal Audio Buffer Read","Error",MB_OK);
#endif
		}



		if(CheckIfPlaybackIsAble()==false)
		{
			// Seek
			if(audioevent.sampleposition+rsamples>endsample) // Read Rest ?
				rsamples=endsample-audioevent.sampleposition;

			int readbytes=rsamples*audioevent.audioefile->samplesize_all_channels; // full buffer (- offset

			audioevent.SeekCurrentBytes(readbytes);

			audioevent.sampleposition+=rsamples;
		}
		else
			if(rsamples>0)
			{
				if(rsamples!=buffer->samplesinbuffer /*&& seek==false*/)
				{
					buffer->ClearInput();
				}

				if(audioevent.sampleposition+rsamples>endsample) // Read Rest ?
					rsamples=endsample-audioevent.sampleposition;

				int readbytes=rsamples*audioevent.audioefile->samplesize_all_channels; // full buffer (- offset

				/*
				if(seek==true)
				{
				audioevent.SeekCurrentBytes(readbytes);
				buffer->channelsused=0; // empty
				}
				else
				*/
				{
					if(offset)
					{
						// + Offset
						switch(audioevent.audioefile->samplebits)
						{
						case 16:
							{
								short *readto16=(short *)buffer->inputbuffer32bit;
								readto16+=offset*audioevent.audioefile->channels;

								//	if(cleared==false)
								//buffer->ClearInputTo((char *)readto16);
								if(audioevent.Read(readto16,readbytes)!=readbytes)
								{
									readbytes=0;
#ifdef _DEBUG
									MessageBox(NULL,"Illegal Audio Buffer Read 16bit","Error",MB_OK);
#endif
								}
							}
							break;

						case 18:
						case 20:
						case 24:
							{
								char *readto24=(char *)buffer->inputbuffer32bit;
								readto24+=offset*3*audioevent.audioefile->channels; // use 3x bytes

								//	if(cleared==false)
								//buffer->ClearInputTo(readto24);
								if(audioevent.Read(readto24,readbytes)!=readbytes)readbytes=0;
							}
							break;

						case 32:
							{
								long *readto32=(long *)buffer->inputbuffer32bit;
								readto32+=offset*audioevent.audioefile->channels;

								//buffer->ClearInputTo((char *)readto32);

								if(audioevent.Read(readto32,readbytes)!=readbytes)
									readbytes=0;
							}
							break;

						case 64:
							{
								ULONGLONG *readto64=(ULONGLONG *)buffer->inputbuffer32bit;
								readto64+=offset*audioevent.audioefile->channels;

								//	if(cleared==false)
								//buffer->ClearInputTo((char *)readto64);
								if(audioevent.Read(readto64,readbytes)!=readbytes)readbytes=0;
							}
							break;

						}//switch

						// raudiofile->streamstartsampleoffset=0; // reset offset
					}
					else // No Offset
					{
						int cb=audioevent.Read(buffer->inputbuffer32bit,readbytes);

						if(readbytes!=cb)
						{
							readbytes=cb;
#ifdef _DEBUG
							MessageBox(NULL,"Illegal Audio Buffer Read <>","Error",MB_OK);
#endif
						}
					}

					audioevent.audioefile->Decoder(buffer);
				}

				audioevent.sampleposition+=rsamples;

				return /*seek==true?0:*/readbytes;
			}
	}

	return 0;
}

int AudioPattern::FillAudioBuffer(Seq_Song *isong,AudioHardwareBuffer *buffer)
{	
	if(!audioevent.audioefile)return 0;

	// Seek Vars
	int rbytes=0;
	bool seek=false;
	LONGLONG seekto,endsample,seekoffset;

	if(audioevent.audioregion)
	{
		endsample=audioevent.audioregion->regionend;

		// Seek -> Region ?
		if(audioevent.audioregion->regionseek==true)
		{
			if(audioevent.sampleposition+buffer->samplesinbuffer>=audioevent.audioregion->seekstart)
			{
				seek=true;
				audioevent.audioregion->regionseek=false; // seek off

				seekoffset=audioevent.audioregion->seekstart-audioevent.sampleposition;
				seekto=audioevent.audioregion->seekend;
			}
		}
	}
	else
		endsample=audioevent.audioefile->samplesperchannel;

	if(audioevent.sampleposition<endsample)
	{
		if(seek==true) // Fill with seek ************************
		{
			int sbuffer=buffer->samplesinbuffer;

			if(seekoffset>0) // FillBuffer 1 Rest
			{
				rbytes=audioevent.iofile.Read(buffer->inputbuffer32bit,(int)(seekoffset*audioevent.audioefile->samplesize_all_channels));
				audioevent.sampleposition+=seekoffset;
				sbuffer-=(int)seekoffset;
			}
			else
				rbytes=0;

			// Seek and Fill full buffer
			if(LONGLONG seeksamples=seekto-audioevent.sampleposition>0)
				audioevent.iofile.SeekCurrent(seeksamples*audioevent.audioefile->samplesize_all_channels);

			if(sbuffer>0) // Read Buffer 2
			{
				char *inputbufferoffset=(char *)buffer->inputbuffer32bit;
				inputbufferoffset+=sbuffer*audioevent.audioefile->samplesize_all_channels;

				int b2=audioevent.iofile.Read(inputbufferoffset,sbuffer*audioevent.audioefile->samplesize_all_channels);
				rbytes+=b2;
			}

			audioevent.sampleposition=seekto;

			goto exit;
		}

		// Read Full buffer ++++++++++++++++++++

		if(audioevent.sampleposition+buffer->samplesinbuffer>=audioevent.audioefile->samplesperchannel)
		{
			int rsample=(int)(audioevent.audioefile->samplesperchannel-audioevent.sampleposition);
			rbytes=audioevent.iofile.Read(buffer->inputbuffer32bit,rsample*audioevent.audioefile->samplesize_all_channels);
			audioevent.sampleposition=audioevent.audioefile->samplesperchannel;
			buffer->endreached=true;
		}
		else
		{
			rbytes=audioevent.iofile.Read(buffer->inputbuffer32bit,buffer->samplesinbuffer*audioevent.audioefile->samplesize_all_channels);
			audioevent.sampleposition+=buffer->samplesinbuffer;
			buffer->endreached=false;
		}

		goto exit;
	}

	buffer->endreached=true;
	return 0;

exit:
	audioevent.audioefile->Decoder(buffer);

	return rbytes;
}

/*
AudioChannel *AudioPattern::GetAudioChannel()
{
AudioChannel *chl=GetTrack()->GetAudioOut()->DefaultChannel();
return chl;
}
*/

bool AudioPattern::CanBeEdited()
{
	if(audioevent.audioefile && audioevent.audioefile->errorflag==0)
		return true;

	return false;
}

OSTART AudioPattern::GetTickEnd(OSTART start)
{	
	if(itsaclone==true || itsaloop==true)
		return ((AudioPattern *)mainpattern)->GetTickEnd(start);

	if(mediatype==MEDIATYPE_AUDIO_RECORD && audioevent.audioefile && audioevent.audioefile->recended==false)
	{
		return GetTrack()->song->timetrack.ConvertSamplesToTempoTicks(start,audioevent.audioefile->samplesperchannel);

		//if(audioevent.audioefile->reccycleloopcounter>0)
		//return start+GetTrack()->song->timetrack.ConvertSamplesToTicks(audioevent.audioefile->samplesperchannel);

		/*
		OSTART sp=GetTrack()->song->GetSongPosition();

		if(audioevent.audioefile->channelsampleswritten==0 && sp>start)
		return sp;

		if(sp<start)return start;
		*/
	}

	if(useoffsetregion==true)
		return GetTrack()->song->timetrack.ConvertSamplesToTempoTicks(start,offsetregion.GetLength());

	// Add Tempo To Ticks
	if(audioevent.audioregion)
		return GetTrack()->song->timetrack.ConvertSamplesToTempoTicks(start,audioevent.audioregion->GetLength());

	if(audioevent.audioefile)
		return GetTrack()->song->timetrack.ConvertSamplesToTempoTicks(start,audioevent.audioefile->samplesperchannel);

	// No File
	return start;
}

LONGLONG AudioPattern::GetAudioSampleStart()
{
	if(itsaclone==true || itsaloop==true)
	{
		AudioPattern *mainaudiopattern=(AudioPattern *)mainpattern;
		return mainaudiopattern->GetAudioSampleStart();
	}

	if(useoffsetregion==true)
		return offsetregion.regionstart;

	if(audioevent.audioregion)
		return audioevent.audioregion->regionstart;

	return offsetregion.regionstart;
}

LONGLONG AudioPattern::GetAudioSampleEnd()
{
	if(itsaclone==true || itsaloop==true)
	{
		AudioPattern *mainaudiopattern=(AudioPattern *)mainpattern;
		return mainaudiopattern->GetAudioSampleEnd();
	}

	if(useoffsetregion==true)return offsetregion.regionend;
	if(audioevent.audioregion)return audioevent.audioregion->regionend;
	return audioevent.audioefile->samplesperchannel;
}

LONGLONG AudioPattern::GetSamples()
{
	if(itsaclone==true || itsaloop==true)return ((AudioPattern *)mainpattern)->GetSamples();

	if(useoffsetregion==true)return offsetregion.regionend-offsetregion.regionstart;
	if(audioevent.audioregion)return audioevent.audioregion->regionend-audioevent.audioregion->regionstart;
	if(audioevent.audioefile)return audioevent.audioefile->samplesperchannel;
	return 0;
}

/*
L
*/
void AudioPattern::CloneFX(Seq_Pattern *to)
{
	AudioPattern *newp=(AudioPattern *)to;

	volumecurve.Clone(&newp->volumecurve);

	newp->mute=mute;
	quantizeeffect.Clone(&newp->quantizeeffect);
	t_colour.Clone(&newp->t_colour);
	CloneLoops(to);
}

void AudioPattern::Clone(Seq_Song *song,Seq_Pattern *topattern,OSTART startdiff,int flag)
{
	AudioPattern *newp=(AudioPattern *)topattern;

	if(newp && newp->mediatype==MEDIATYPE_AUDIO && audioevent.audioefile)
	{	
		// Clone Data
		newp->SetName(GetName()); // copy name
		newp->mediatype=mediatype;

		// HDFile
		if(//topattern->itsaclone==false &&
			//topattern->itsaloop==false &&
			audioevent.audioregion)
		{
			newp->audioevent.audioregion=new AudioRegion;
			if(newp->audioevent.audioregion)
			{
				audioevent.audioregion->CloneTo(newp->audioevent.audioregion);
				newp->audioevent.audioregion->InitRegion();
			}
		}
		if(newp->audioevent.audioefile=audioevent.audioefile)
			newp->audioevent.audioefile->Open();

		CloneFX(newp);
		//quantizeeffect.Clone(&newp->quantizeeffect);
		//colour.Clone(&newp->colour);

		newp->mute=mute;

		newp->ostart=GetPatternStart()+startdiff;
		newp->audioevent.ostart=audioevent.ostart+startdiff;
		newp->audioevent.staticostart=audioevent.staticostart+startdiff;

		// Clone Offset
		newp->offset=offset;
		newp->useoffsetregion=useoffsetregion;
		newp->offsetstartoffset=offsetstartoffset;

		offsetregion.CloneTo(&newp->offsetregion);
	}
}

Seq_Pattern *AudioPattern::CreateClone(OSTART startdiff,int flag)
{
	if(audioevent.audioefile)
	{
		if(AudioPattern *newclone=new AudioPattern)
		{
			Clone(track->song,newclone,startdiff,flag);
			return newclone;
		}
	}

	return 0;
}

bool AudioPattern::InitPlayback(InitPlay *init,int mode)
{
	init->started=false;

	if(audioevent.audioefile && audioevent.audioefile->m_ok==true && 
		(GetPatternEnd()>init->position /* || (init->cycleplay==true && recordpattern==true)*/ ) // Audio File or Audio Cycle Recording
		)
	{	
		if(GetPatternStart()<=init->position)init->started=true;
		return true;
	}

	return false;
}

void AudioPattern::InitOffSetEdit(int mode)
{
	// Reset ?
	if(useoffsetregion==false)
	{
		offsetregion.r_audiohdfile=audioevent.audioefile;

		if(audioevent.audioregion)
		{
			offsetregion.regionstart=audioevent.audioregion->regionstart;
			offsetregion.regionend=audioevent.audioregion->regionend;
		}
		else
		{
			offsetregion.regionstart=0;
			offsetregion.regionend=audioevent.audioefile->samplesperchannel;
		}

		offsetregion.InitRegion();
	}

	/*
	switch(mode)
	{
	case EM_SIZEPATTERN_LEFT:
	{
	offsetstart=offsetregion.regionstart;
	}
	break;

	case EM_SIZEPATTERN_RIGHT:
	{
	offsetstart=offsetregion.regionend;
	}
	break;

	default:
	offsetstart=0;
	break;
	}
	*/
}

bool AudioPattern::IfOffSetStart()
{
	if(itsaloop==true || itsaclone==true)return ((AudioPattern *)mainpattern)->IfOffSetStart();

	LONGLONG start;

	if(audioevent.audioregion)start=audioevent.audioregion->regionstart;
	else start=0;

	if(useoffsetregion==true && offsetregion.regionstart!=start)return true;

	return false;
}

bool AudioPattern::IfOffSetEnd()
{
	if(itsaloop==true || itsaclone==true)return ((AudioPattern *)mainpattern)->IfOffSetEnd();
	LONGLONG end;

	if(audioevent.audioregion)end=audioevent.audioregion->regionend;
	else end=audioevent.audioefile->samplesperchannel;

	if(useoffsetregion==true && offsetregion.regionend!=end)return true;

	return false;
}

LONGLONG AudioPattern::GetOffSetStart()
{
	if(itsaloop==true || itsaclone==true)return ((AudioPattern *)mainpattern)->GetOffSetStart();
	if(useoffsetregion==true)return offsetregion.regionstart;
	if(audioevent.audioregion)return audioevent.audioregion->regionstart;
	return 0; //  File from Start
}

LONGLONG AudioPattern::GetOffSetEnd()
{
	if(itsaloop==true || itsaclone==true)return ((AudioPattern *)mainpattern)->GetOffSetEnd();
	if(useoffsetregion==true)return offsetregion.regionend;
	if(audioevent.audioregion)return audioevent.audioregion->regionend;
	return audioevent.audioefile->samplesperchannel; // File End <
}

bool AudioPattern::SetOffSetStart(OSTART pos,LONGLONG startoffset,bool test)
{
	if(startoffset==0)
	{
		LONGLONG setstart,end;

		if(audioevent.audioregion)
		{
			setstart=audioevent.audioregion->regionstart;
			end=audioevent.audioregion->regionend;
		}
		else
		{
			setstart=0;
			end=audioevent.audioefile->samplesperchannel;
		}

		if(useoffsetregion==true && offsetregion.regionstart!=setstart)
		{
			if(offsetregion.regionend==end)
			{
				if(test==false)
				{
					useoffsetregion=false;
					offsetstartoffset=0;
					SetStart(pos);
				}

				return true;
			}

			if(test==false)
			{
				offsetregion.regionstart=setstart;
				offsetregion.InitRegion();
				SetStart(pos);
			}

			return true;
		}

		return false;
	}

	LONGLONG start,end;

	if(audioevent.audioregion)
	{
		start=audioevent.audioregion->regionstart;
		end=audioevent.audioregion->regionend;
		//startoffset+=start;
	}
	else
	{
		start=0;

		if(useoffsetregion==true)
			end=offsetregion.regionend;
		else
			end=audioevent.audioefile->samplesperchannel;
	}

	if(pos<GetPatternEnd() && startoffset<end && startoffset>=start)
	{
		if(test==false)
		{
			offsetregion.regionstart=startoffset;

			if(offsetregion.regionstart==start && offsetregion.regionend==end)
			{
				offsetstartoffset=0;
				useoffsetregion=false;
			}
			else
			{
				if(offsetstartoffset==0)
					offsetstartoffset=pos-GetPatternStart();

				useoffsetregion=true;
				offsetregion.InitRegion();
			}

			SetStart(pos);
		}

		return true;
	}

	return false;
}

bool AudioPattern::SetOffSetEnd(LONGLONG endoffset,bool test)
{
	if(endoffset==0)
	{
		LONGLONG start,setend;

		if(audioevent.audioregion)
		{
			start=audioevent.audioregion->regionstart;
			setend=audioevent.audioregion->regionend;
		}
		else
		{
			start=0;
			setend=audioevent.audioefile->samplesperchannel;
		}

		if(useoffsetregion==true && offsetregion.regionend!=setend)
		{
			if(offsetregion.regionstart==start)
			{
				if(test==false)
				{
					offsetstartoffset=0;
					useoffsetregion=false;
				}

				return true;
			}

			if(test==false)
			{
				offsetregion.regionend=setend;
				offsetregion.InitRegion();
			}

			return true;
		}

		return false;
	}

	LONGLONG start,end;

	if(audioevent.audioregion)
	{
		start=audioevent.audioregion->regionstart;
		end=audioevent.audioregion->regionend;
		//startoffset+=start;
	}
	else
	{
		if(useoffsetregion==true)
			start=offsetregion.regionstart;
		else
			start=0;

		end=audioevent.audioefile->samplesperchannel;
	}

	if(endoffset>start && endoffset<=end)
	{
		TRACE ("EndOffSet %d\n",endoffset);

		if(test==false)
		{
			offsetregion.regionend=endoffset;

			if(offsetregion.regionstart==start && offsetregion.regionend==end)
			{
				offsetstartoffset=0;
				useoffsetregion=false;
			}
			else
			{
				useoffsetregion=true;
				offsetregion.InitRegion();
			}
		}
		return true;
	}

	return false;
}

bool AudioPattern::SetStart(OSTART pos)
{
	audioevent.ostart=audioevent.staticostart=pos;
	return true;
}

void AudioPattern::SetAudioMediaTypeAfterRecording()
{
	if(mediatype==MEDIATYPE_AUDIO_RECORD)
	{
		mediatype=MEDIATYPE_AUDIO;
		volumecurve.InitFadeInOut(0,0,GetSamples());
	}
}

bool AudioPattern::SizeAble()
{
	if(mainpattern)return false;
	if(!audioevent.audioefile)return false;

	return true;
}

void AudioPattern::InitDefaultVolumeCurve(bool force)
{
	if(volumecurve.init==false || force==true)
	{
		LONGLONG samples=GetSamples();

		if(samples==0)
		{
			volumecurve.InitFadeInOut(0,0,samples);
			volumecurve.init=false;
			return;
		}

		double ms=mainaudio->ConvertSamplesToMs(samples);

		if(ms==0)
		{
			volumecurve.InitFadeInOut(0,0,samples);
			volumecurve.init=false;
			return;
		}

#ifdef DEBUG
		double sec=ms/1000;

		// Fade In
		if(sec>30)
		{
			volumecurve.InitFadeInOut(5000,5000,samples);
		}
		else
			if(sec>20)
			{
				volumecurve.InitFadeInOut(4000,4000,samples);
			}
			else
				if(sec>10)
				{
					volumecurve.InitFadeInOut(2000,2000,samples);
				}
				else
					if(sec>5)
					{
						volumecurve.InitFadeInOut(1000,1000,samples);
					}
					else
						if(sec>3)
						{
							volumecurve.InitFadeInOut(500,500,samples);
						}
						else
						{
							volumecurve.InitFadeInOut(0,0,samples);
							volumecurve.init=false;
							return;
						}

						volumecurve.init=true;
#else
		volumecurve.InitFadeInOut(0,0,samples);
		volumecurve.init=false;
#endif

	}
}

bool AudioPattern::SetNewLoopStart(OSTART pos)
{
	ostart=pos;
	audioevent.ostart=pos;
	audioevent.staticostart=pos;

	// Clone Offset
	if(mainpattern)
	{
		bool rv=false;

		if(mainpattern->useoffsetregion!=useoffsetregion)
		{
			rv=true;
			useoffsetregion=mainpattern->useoffsetregion;
		}

		if(mainpattern->offsetstartoffset!=offsetstartoffset)
		{
			rv=true;
			offsetstartoffset=mainpattern->offsetstartoffset;
		}

		AudioPattern *mainaudiopattern=(AudioPattern *)mainpattern;

		if(offsetregion.Compare(&mainaudiopattern->offsetregion)==false)
		{
			rv=true;
			mainaudiopattern->offsetregion.CloneTo(&offsetregion);
		}

		return rv;
	}

	return false;
}

void AudioPattern::StopAllofPattern()
{
	if(track && track->song)
		track->song->RemovePatternFromAudioLists(this);
}

ARES AudioPattern::GetCrossFadeVolume(LONGLONG sampleposition)
{
	Seq_CrossFade *scf=FirstCrossFade();

	while(scf)
	{
		if(scf->used==true && scf->from_sample_file<=sampleposition && scf->to_sample_file>=sampleposition)
		{
			double cfsize=scf->to_sample_file-scf->from_sample_file,x=sampleposition-scf->from_sample_file;
			x/=cfsize;

			return scf->ConvertToVolume(x,scf->infade);
		}

		scf=scf->NextCrossFade();
	}

	return 1;
}