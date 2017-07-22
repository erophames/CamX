#include "defines.h"
#include "audiofile.h"
#include "audiohdfile.h"
#include "audiohardwarechannel.h"
#include "semapores.h"
#include "object_song.h"
#include "guiheader.h"
#include "audiohardware.h"
#include "audiothread.h"
#include "audiorealtime.h"
#include "audioregion.h"
#include "arrangeeditor.h"
#include "audiomanager.h"
#include "object_track.h"
#include "gui.h"
#include "sampleeditor.h"
#include "editfunctions.h"
#include "editbuffer.h"
#include "editor_event.h"
#include "peakbuffer.h"
#include "languagefiles.h"
#include "songmain.h"
#include "object_project.h"
#include "chunks.h"
#include "crossfade.h"

AudioRegion *AudioHDFile::FindRegionInside(LONGLONG start,LONGLONG end)
{
	AudioRegion *f=FirstRegion();

	while(f){
		if(f->CheckIfInRegion(start,end)==true)return f;
		f=f->NextRegion();
	}

	return 0;
}

AudioRegion *AudioHDFile::FindRegion(AudioRegion *check)
{
	AudioRegion *f=FirstRegion();

	while(f){
		if(f==check)return f;
		f=f->NextRegion();
	}

	return 0;
}

AudioRegion *AudioHDFile::FindRegion(LONGLONG start,LONGLONG end)
{
	AudioRegion *f=FirstRegion();

	while(f){
		if(f->regionstart==start && f->regionend==end)return f;
		f=f->NextRegion();
	}

	return 0;
}

/*
AudioRegion *AudioHDFile::DeleteVRegion(AudioRegion *region)
{
	if(region && region->audiohdfile==this)
	{
		AudioRegion *n=(AudioRegion *)region->NextOrPrev();

		// Remove From Clipboard
		region->FreeMemory();
		vregions.RemoveO(region);

		return n;
	}
#ifdef _DEBUG
	else
		maingui->MessageBoxOk(0,"Delete region error!");
#endif

	return 0;
}
*/

AudioRegion *AudioHDFile::DeleteRegion(AudioRegion *region)
{
	if(region && region->r_audiohdfile==this)
	{
		AudioRegion *n=(AudioRegion *)region->NextOrPrev();

		// Remove From Clipboard
		mainbuffer->DeleteBufferRegion(region);
		region->FreeMemory();
		regions.RemoveO(region);

		return n;
	}
#ifdef _DEBUG
	else
		maingui->MessageBoxOk(0,"Delete region error!");
#endif

	return 0;
}

/*
void AudioHDFile::AddVRegion(AudioRegion *region)
{
	region->virtualregion=true;
	region->audiohdfile=this;
	vregions.AddEndO(region);
}
*/

AudioRegion *AudioHDFile::AddRegion(AudioRegion *region,bool force)
{
	region->r_audiohdfile=this;

	if(force==true)
		regions.AddEndO(region);
	else
	{
		AudioRegion *f=FindRegion(region->regionstart,region->regionend);

		if((!f) || strcmp(f->regionname,region->regionname)!=0)
			regions.AddEndO(region);
		else{
			region->FreeMemory();
			delete region;
			region=f;
		}
	}

	return region;
}

void AudioHDFile::FreeRegions()
{
	// Delete Regions
	AudioRegion *r=FirstRegion();

	while(r){
		r->FreeMemory();
		r=(AudioRegion *)regions.RemoveO(r);
	}

	/*
	r=FirstVRegion();

	while(r){
		r->FreeMemory();
		r=(AudioRegion *)vregions.RemoveO(r);
	}
	*/
}

int AudioRegion::GetUsedCounter()
{
	int c=0;

	// Check Regions
	Seq_Project *p=mainvar->FirstProject();

	while(p)
	{
		Seq_Song *s=p->FirstSong();
		while(s){
			Seq_Track *t=s->FirstTrack();
			while(t){
				Seq_Pattern *p=t->FirstPattern(MEDIATYPE_AUDIO);

				while(p){
					AudioPattern *ap=(AudioPattern *)p;

					if(ap->audioevent.audioregion==this)
						c++;

					p=p->NextPattern(MEDIATYPE_AUDIO);
				}

				t=t->NextTrack();
			}

			s=s->NextSong();
		}

		p=p=p->NextProject();
	}

	return c;
}

bool AudioRegion::ChangeRegionPosition(LONGLONG s,LONGLONG e)
{
	LONGLONG start=s!=-1?s:regionstart;
	LONGLONG end=e!=-1?e:regionend;

	if(start<end && end<=r_audiohdfile->samplesperchannel && (start!=regionstart || end!=regionend))
	{
		regionstart=start;
		regionend=end;

		InitRegion();

		// Check Regions
		Seq_Project *p=mainvar->FirstProject();

		while(p)
		{
			Seq_Song *s=p->FirstSong();
			while(s)
			{
				Seq_Track *t=s->FirstTrack();
				while(t)
				{
					Seq_Pattern *p=t->FirstPattern(MEDIATYPE_AUDIO);

					while(p)
					{
						AudioPattern *ap=(AudioPattern *)p;

						if(ap->audioevent.audioregion==this)
						{

						}

						p=p->NextPattern(MEDIATYPE_AUDIO);
					}

					t=t->NextTrack();
				}

				s=s->NextSong();
			}

			p=p=p->NextProject();
		}

		return true;
	}

	return false;
}


void AudioRegion::SetName(char *name,bool refreshgui,guiWindow *ex)
{
	if(name)
	{
		if(regionname)delete regionname;

		if(regionname=mainvar->GenerateString(name))

			// RefreshGUI
			if(refreshgui==true)
			{
				guiWindow *win=maingui->FirstWindow();

				while(win)
				{
					switch(win->GetEditorID())
					{
					case EDITORTYPE_SAMPLEREGIONLIST:
						{
							Edit_RegionList *rl=(Edit_RegionList *)win;

							if(rl->editor->audiohdfile==r_audiohdfile)
							{
								rl->ShowRegions();
								rl->ShowRegionName();
							}

						}
						break;

					case EDITORTYPE_ARRANGE:
						{
							Edit_Arrange *ar=(Edit_Arrange *)win;
							ar->RefreshAudioRegion(this);
						}
						break;

					case EDITORTYPE_AUDIOMANAGER:
						{
							Edit_Manager *em=(Edit_Manager *)win;

							if(em->activefile && em->activefile->hdfile==r_audiohdfile)
							{
								em->ShowActiveHDFile_Regions();
							}
						}
						break;
					}

					win=win->NextWindow();
				}
			}
	}
}

void AudioRegion::InitRegion()
{
	if(!r_audiohdfile)
	{
		#ifdef DEBUG
		maingui->MessageBoxError(0,"InitRegion r_audiohdfile");
#endif

		return;
	}

	// Read End
	LONGLONG h=regionend;

	if(h>r_audiohdfile->samplesperchannel)
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"InitRegion");
#endif

		regionend=h=r_audiohdfile->samplesperchannel;
	}

	h*=r_audiohdfile->samplesize_all_channels;
	h+=r_audiohdfile->datastart;

	//region_dataend=h;	
}

AudioRegion::AudioRegion(AudioHDFile *ahf)
{
	id=OBJ_AUDIOREGION;

	r_audiohdfile=ahf;

	regionname=mainvar->GenerateString("Region");
	regionstart=regionend=0;
	//region_dataend=0;

	regionseek=false; // default no seek
	autODeInit=false;
	destructive=false;

	//virtualregion=false;

	crossfade=0;
	clonedfrom=0;
}

void AudioRegion::FreeMemory()
{
	if(crossfade)
	{
		crossfade->DeInit();
		delete crossfade;
		crossfade=0;
	}

	if(regionname)delete regionname;
	regionname=0;
}

void AudioRegion::CloneTo(AudioRegion *r)
{
	if(regionname)
	{
		if((!r->regionname) || (strcmp(r->regionname,regionname)!=0))
		{
			if(r->regionname)delete r->regionname;
			r->regionname=mainvar->GenerateString(regionname);
		}
	}
	else
	{
		if((!r->regionname) || (strcmp(r->regionname,"Region")!=0))
		{
			if(r->regionname)delete r->regionname;
			r->regionname=mainvar->GenerateString("Region");
		}
	}

	r->regionstart=regionstart;
	r->regionend=regionend;
	r->r_audiohdfile=r_audiohdfile;
}

void AudioRegion::Load(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);

	file->ReadChunk(&regionstart);
	file->ReadChunk(&regionend);

	file->Read_ChunkString(&regionname);

	//file->ReadChunk(&virtualregion);
	file->CloseReadChunk();

	//InitRegion();
}

void AudioRegion::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_AUDIOHDREGION);

	file->Save_Chunk((CPOINTER)this);
	file->Save_Chunk(regionstart);
	file->Save_Chunk(regionend);
	file->Save_ChunkString(regionname);

//	file->Save_Chunk(virtualregion);
	file->CloseChunk();
}

AudioHDFile::AudioHDFile()
{
	id=OBJ_AUDIOHDFILE;

	type=TYPE_UNKNOWN;

	seekbeforewrite=-1;
	reccycleloopcounter=0;
	externsamplerate=false;
	deleted=false;

	info=filename=name=0;

	peakbuffer=0;
	samplesperchannel=0;
	channels=0;
	errorflag=0;

	camxrecorded=m_ok=waitingforpeakfile=destructive=false;
	mode=FILEMODE_READ;

	// RAM
	dontcreatepeakfile=false;
	showregionsineditors=true;
	creatednewfile=0;

	flag=0;
	filenotfound=false;
	replacewithfile=0;
	writezerobytes=0;
	addzerosamples=0;
	datalen=0;

	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
	{
		recmixmaxpeak_m[i]=recmixmaxpeak_p[i]=0;
		recmixcounter[i]=0;
		recmixpeakbuffercounter[i]=0;
	}

	recordingactive=recstarted=recended=camximport=ramfile=nopeakfileasfile=false;
	recmaxpeakposition_zerosample=0;
	recordpeakclosed=false;
	directory=0;
	recordpattern=0;
	samplesarefloat=false;
	reccycleloopcounter=0;
	recordedsamples=0;
}

char *AudioHDFile::GetDragDropInfoString()
{
	return filename; 
}

char *AudioHDFile::ErrorString()
{
	if(errorflag)
	{
		if(filenotfound==true)return Cxs[CXS_FILENOTFOUND];
	}

	return "_ ";
}

void AudioHDFile::FreeMemory()
{
	// Regions
	FreeRegions();

	if(name){
		delete name;
		name=0;
	}

	if(filename){
		delete filename;
		filename=0;
	}

	if(info){
		delete info;
		info=0;
	}

	if(creatednewfile){
		delete creatednewfile;
		creatednewfile=0;
	}

	if(replacewithfile){
		delete replacewithfile;
		replacewithfile=0;
	}

	bool force;

	if(mainvar->exitprogram_flag==true)
		force=true;
	else
		force=false;

	if(peakbuffer){
		bool deleted;
		mainaudio->ClosePeakFile(this,peakbuffer,force,&deleted);
		if(deleted==true)peakbuffer=0;
	}

	DeleteData();

	//acyclebuffer.FreeMemory();
}

// Read Realtime
int AudioRAMFile::FillAudioBuffer(AudioHardwareBuffer *buffer,char *ramposition,int bufferoffset)
{	
	if(ramdata && ramposition<ramend)
	{	
		size_t rsamples=(buffer->samplesinbuffer-bufferoffset)*channels;

		buffer->endreached=false;

		// Samplestart
		switch(samplebits)
		{
		case 16:
			{
				short *readto=(short *)buffer->inputbuffer32bit,
					*from16=(short *)ramposition,
					*end=(short *)ramend,
					*readend=readto+(buffer->samplesinbuffer*channels);

				// Offset
				if(bufferoffset){
					memset(readto,0,bufferoffset*channels*sizeof(short));
					readto+=bufferoffset*channels;
				}
				//while(i--)*readto++=0

				if(from16+rsamples>end)rsamples=end-from16;

				// Read
				//while(rsamples--)*readto++=*from16++;
				if(rsamples)
				{
					memcpy(readto,from16,rsamples*sizeof(short));
					readto+=rsamples;
				}

				// Clear Rest
				if(int i=readend-readto) // EOF
				{
					buffer->endreached=true;
					memset(readto,0,i*sizeof(short));
					//while(i--)*readto++=0;
				}

				return buffer->samplesinbuffer*channels*sizeof(short);
			}
			break;

		case 32:
			{
				long *readto=(long *)buffer->inputbuffer32bit,
					*from32=(long *)ramposition,
					*end=(long *)ramend,
					*readend=readto+(buffer->samplesinbuffer*channels);

				// Offset
				if(bufferoffset){
					memset(readto,0,bufferoffset*channels*sizeof(long));
					readto+=bufferoffset*channels;
				}
				//while(i--)*readto++=0

				if(from32+rsamples>end)
					rsamples=end-from32;

				// Read
				//while(rsamples--)*readto++=*from16++;
				if(rsamples)
				{
					memcpy(readto,from32,rsamples*sizeof(long));
					readto+=rsamples;
				}

				// Clear Rest
				if(int i=readend-readto) // EOF
				{
					buffer->endreached=true;
					memset(readto,0,i*sizeof(long));
					//while(i--)*readto++=0;
				}

				return buffer->samplesinbuffer*channels*sizeof(long);
			}
			break;

		case 64:
			{
				LONGLONG *readto=(LONGLONG *)buffer->inputbuffer32bit,
					*from64=(LONGLONG *)ramposition,
					*end=(LONGLONG *)ramend,
					*readend=readto+(buffer->samplesinbuffer*channels);

				// Offset
				if(bufferoffset){
					memset(readto,0,bufferoffset*channels*sizeof(LONGLONG));
					readto+=bufferoffset*channels;
				}
				//while(i--)*readto++=0

				if(from64+rsamples>end)
					rsamples=end-from64;

				// Read
				//while(rsamples--)*readto++=*from16++;
				if(rsamples)
				{
					memcpy(readto,from64,rsamples*sizeof(LONGLONG));
					readto+=rsamples;
				}

				// Clear Rest
				if(int i=readend-readto) // EOF
				{
					buffer->endreached=true;
					memset(readto,0,i*sizeof(LONGLONG));
					//while(i--)*readto++=0;
				}

				return buffer->samplesinbuffer*channels*sizeof(LONGLONG);
			}
			break;

			/*
			case 64:
			{
			ULONGLONG *readto32=(ULONGLONG *)buffer->inputbuffer32bit,
			*from32=(ULONGLONG *)ramposition,
			*end32=(ULONGLONG *)ramend,
			*readend=readto32+(buffer->samplesinbuffer*channels);

			// Offset
			while(i--)*readto32++=0;

			if(from32+rsamples>end32)rsamples=end32-from32;

			// Read
			while(rsamples--)*readto32++=*from32++;

			// Clear Rest
			i=readend-readto32;

			if(i) // EOF
			{
			buffer->endreached=true;
			while(i--)*readto32++=0;
			}

			bytesread=buffer->samplesinbuffer*channels*sizeof(long);
			}
			break;
			*/

		case 18:
		case 20:
		case 24:
			{
				char *readto24=(char *)buffer->inputbuffer32bit,
					*from24=ramposition,
					*end=ramend,
					*readend=readto24+(buffer->samplesinbuffer*channels*3);

				// Offset
				while(bufferoffset--)
				{
					*readto24++=0;
					*readto24++=0;
					*readto24++=0;
				}

				if(from24+(3*rsamples)>end)
					rsamples=(end-from24)/3;

				// Read
				while(rsamples--)
				{
					*readto24++=*from24++;
					*readto24++=*from24++;
					*readto24++=*from24++;
				}

				// Clear Rest
				int i=(readend-readto24)/3;

				if(i) // EOF
				{
					buffer->endreached=true;

					while(i--)
					{
						*readto24++=0;
						*readto24++=0;
						*readto24++=0;
					}
				}

				return buffer->samplesinbuffer*channels*3*sizeof(char);
			}
			break;

			/*
			case 32:
			{
			long *readto32=(long *)buffer->inputbuffer32bit,
			*from32=(long *)ramposition,
			*end32=(long *)ramend,
			*readend=readto32+(buffer->samplesinbuffer*channels);

			// Offset
			while(i--)
			*readto32++=0;

			if(from32+rsamples>end32)
			rsamples=end32-from32;

			// Read
			while(rsamples--)
			*readto32++=*from32++;

			// Clear Rest
			i=(int)(readend-readto32);

			if(i) // EOF
			{
			buffer->endreached=true;

			while(i--)
			*readto32++=0;
			}

			bytesread=buffer->samplesinbuffer*channels*sizeof(long);
			}
			break;
			*/


		default:
			buffer->endreached=true;
			break;
		}
	}
	else
		buffer->endreached=true;

	return 0;
}

void AudioRAMFile::LoadSoundToRAM()
{
	if(m_ok==true)
	{
		camxFile iofile;

		if(datalen && iofile.OpenRead(GetName())==true)
		{
			iofile.SeekBegin(datastart);

			ramdata=ramend=new char[datalen];

			if(ramdata)
			{
				iofile.Read(ramdata,datalen);
				ramend+=datalen;
			}

			iofile.Close(true);
		}
	}
}

void AudioHDFile::Open(char *fname)
{
	m_ok=false;
	errorflag=0;

	if(fname)
	{
		if(name)delete name;
		if(name=mainvar->GenerateString(fname))
		{
			InitHDFile();
		}
		else
			errorflag|=AUDIOFILENAME_ERROR; // oom !?
	}
	else
		errorflag|=AUDIOFILENAME_ERROR;
}

/****************************************************************
* Extended precision IEEE floating-point conversion routine.
****************************************************************/

# define UnsignedToFloat(u)         (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)

double ConvertFromIeeeExtended(unsigned char* bytes /* LCN */)
{
	double    f;
	int    expon;
	unsigned long hiMant, loMant;

	expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
	hiMant    =    ((unsigned long)(bytes[2] & 0xFF) << 24)
		|    ((unsigned long)(bytes[3] & 0xFF) << 16)
		|    ((unsigned long)(bytes[4] & 0xFF) << 8)
		|    ((unsigned long)(bytes[5] & 0xFF));
	loMant    =    ((unsigned long)(bytes[6] & 0xFF) << 24)
		|    ((unsigned long)(bytes[7] & 0xFF) << 16)
		|    ((unsigned long)(bytes[8] & 0xFF) << 8)
		|    ((unsigned long)(bytes[9] & 0xFF));

	if (expon == 0 && hiMant == 0 && loMant == 0) {
		f = 0;
	}
	else {
		if (expon == 0x7FFF) {    /* Infinity or NaN */
			f = HUGE_VAL;
		}
		else {
			expon -= 16383;
			f  = ldexp(UnsignedToFloat(hiMant), expon-=31);
			f += ldexp(UnsignedToFloat(loMant), expon-=32);
		}
	}

	if (bytes[0] & 0x80)
		return -f;
	else
		return f;
}

void mainAudio::TestForAIFFFormat(camxFile *iofile,AudioFileInfo *info)
{
	char buffer[256];

	union
	{
		char b[4];
		unsigned long value;
	}u32;

	union
	{
		char b[2];
		unsigned short value;
	}u16;

	iofile->SeekBegin(0);

	buffer[0]=0;

	iofile->Read(buffer,4);

	if(buffer[0]=='F' && buffer[1]=='O' && buffer[2]=='R' && buffer[3]=='M'){

		iofile->Read(&u32.b[3],1);
		iofile->Read(&u32.b[2],1);
		iofile->Read(&u32.b[1],1);
		iofile->Read(&u32.b[0],1);

		int size=u32.value;

		iofile->Read(buffer,4);

		bool ssndfound=false;
		bool commfound=false;

		if(buffer[0]=='A' && buffer[1]=='I' && buffer[2]=='F' && (buffer[3]=='F' || buffer[3]=='C')){

			// Check Common Chunks

			info->type=buffer[3]=='F'?TYPE_AIFF:TYPE_AIFFC;

			for(;;)
			{
				int r=iofile->Read(buffer,4);

				if(r!=4)
					break;

				iofile->Read(&u32.b[3],1);
				iofile->Read(&u32.b[2],1);
				iofile->Read(&u32.b[1],1);
				iofile->Read(&u32.b[0],1);

#ifdef DEBUG
				char c[5];

				c[4]=0;
				c[0]=buffer[0];
				c[1]=buffer[1];
				c[2]=buffer[2];
				c[3]=buffer[3];

				TRACE ("AIFF Chunk %s ",c);

#endif
				int size=u32.value;

				TRACE ("Size %d \n",size);

				if(buffer[0]=='S' && buffer[1]=='S' && buffer[2]=='N' && buffer[3]=='D')
				{
					unsigned long offset;

					iofile->Read(&u32.b[3],1);
					iofile->Read(&u32.b[2],1);
					iofile->Read(&u32.b[1],1);
					iofile->Read(&u32.b[0],1);

					offset=u32.value;

					unsigned long blockSize;

					iofile->Read(&u32.b[3],1);
					iofile->Read(&u32.b[2],1);
					iofile->Read(&u32.b[1],1);
					iofile->Read(&u32.b[0],1);

					blockSize=u32.value;

					info->datastart=iofile->SeekCurrent(0);
					info->datalen=size-8;
					info->dataend=info->datastart+info->datalen;

					// datalen->samples
					LONGLONG h=info->datalen;
					LONGLONG h2=info->bits/8*info->channels;
					if(h2)
						h/=h2;
					else
						h=0;

					info->samples=h;

					iofile->SeekCurrent(size-8);

					ssndfound=true;
				}
				else
					if(buffer[0]=='C' && buffer[1]=='O' && buffer[2]=='M' && buffer[3]=='M'){

						iofile->Read(&u16.b[1],1);
						iofile->Read(&u16.b[0],1);

						info->channels=u16.value;

						TRACE ("Channels %d\n",info->channels);

						iofile->Read(&u32.b[3],1);
						iofile->Read(&u32.b[2],1);
						iofile->Read(&u32.b[1],1);
						iofile->Read(&u32.b[0],1);

						info->samples=u32.value;

						TRACE ("Samples %d\n",info->samples);

						iofile->Read(&u16.b[1],1);
						iofile->Read(&u16.b[0],1);

						info->bits=u16.value;

						TRACE ("Bits %d\n",info->bits);

						unsigned char f[10];

						iofile->Read(f,10);
						info->samplerate=ConvertFromIeeeExtended(&f[0]);


						if(info->type==TYPE_AIFFC)
						{
							// Compression Type

							char h[4];

							iofile->Read(h,4);

							if(h[0]=='f' && h[1]=='l' && h[2]=='3' && h[3]=='2')
							{
								if(info->bits==32)
									info->samplesarefloat=true;
							}
							else
								if(h[0]=='F' && h[1]=='L' && h[2]=='3' && h[3]=='2')
								{
									if(info->bits==32)
										info->samplesarefloat=true;
								}
								else
									if(h[0]=='F' && h[1]=='L' && h[2]=='6' && h[3]=='4')
									{
										if(info->bits==64)
										{
										}
									}


									size-=22;

									// Decoder String
									h[0]=0;

									while(size--)
									{
										iofile->Read(h,1);
									}
						}

						commfound=true;
					}
					else
						iofile->SeekCurrent(size);

				if(commfound==true && ssndfound==true && (info->type==TYPE_AIFFC || info->type==TYPE_AIFF))
				{
					info->m_ok=true;
					return;
				}
			}

		}
	}

}

void mainAudio::TestForWaveFormat(camxFile *iofile,AudioFileInfo *info)
{
	char buffer[256];

	iofile->SeekBegin(0);

	buffer[0]=0;

	int readrt=iofile->Read(buffer,12);

	// Check AudioPattern Status

	// WAVE Format ?
	if (readrt==12 && buffer[0] == 'R' && buffer[1] == 'I' && buffer[2] == 'F' && buffer[3] == 'F'){

		info->type=TYPE_WAV;

		USHORT *us;
		int *uw;

		if(buffer[8]=='W' && buffer[9]=='A' && buffer[10]=='V' && buffer[11]=='E'){
			uw=(int *)&buffer[4]; // Clen RIFF

			int clen=*uw;

			buffer[0]=0;
			readrt=iofile->Read(buffer,8);

			if(readrt==8 && buffer[0]=='f' && buffer[1]=='m' && buffer[2]=='t'){
				uw=(int *)&buffer[4]; // Clen FMT
				clen=*uw;

				if(clen>0 && clen<255){

					buffer[0]=0;
					readrt=iofile->Read(buffer,clen);

					if(readrt==clen)
					{
						info->m_ok = true;

						us=(USHORT *)&buffer[0];
						USHORT compcode=*us;

						switch(compcode)
						{
						case 3: // Float
							info->samplesarefloat=true;
							break;
						}

						us=(USHORT *)&buffer[2];
						info->channels=*us;

						uw=(int *)&buffer[4];
						info->samplerate=*uw;

						us=(USHORT *)&buffer[14];
						info->bits=*us;

						if(info->channels>0 && info->channels<=MAXCHANNELSPERCHANNEL && info->samplerate>0 &&
							(info->bits==16 || info->bits==20 || info->bits==24 || info->bits==32 || info->bits==64)
							){
								info->datastart=0;

								do // Find data chunk
								{
									readrt=iofile->Read(buffer,8);

									uw=(int *)&buffer[4]; // Clen
									clen=*uw;

									if(readrt==8 && buffer[0]=='d' && buffer[1]=='a' && buffer[2]=='t' && buffer[3]=='a'){

										info->datastart=iofile->SeekCurrent(0);
										// fileposition=datastart;


										info->headerlen=info->datastart;
										info->datalen=clen;
										info->dataend=info->datastart+info->datalen;

										// datalen->samples
										LONGLONG h=info->datalen;
										LONGLONG h2=info->bits/8*info->channels;
										if(h2)
											h/=h2;
										else
											h=0;

										info->samples=h;

										if(info->dataend<info->datastart)
										{
											info->m_ok=false;
											info->errorflag|=AUDIOFILECHECK_ERROR;
										}
										else
											if(info->dataend>info->filelength)
											{
												if(info->filelength-info->datastart>info->datastart || mainaudio->ignorecorrectaudiofiles==true)
													info->dataend=info->filelength-info->datastart;
												else
													info->m_ok=false; //error
											}

									}
									else // not a data chunk ?
									{
										LONGLONG pos=iofile->SeekCurrent(0);

										if(pos+clen>info->filelength)
											info->m_ok=false; // error
										else
											iofile->SeekCurrent(clen);							
									}

								}while(info->datastart==0 && info->m_ok==true);
						}
						else{
							info->errorflag|=AUDIOFILECHECK_ERROR;
							info->m_ok=false;
						}

					}
					else
					{
						info->errorflag|=AUDIOFILECHECK_ERROR;
						info->m_ok=false;
					}
				}
				else
					info->errorflag|=AUDIOFILECHECK_ERROR;
			}
			else
				info->errorflag|=AUDIOFILECHECK_ERROR;
		}
		else
			info->errorflag|=AUDIOFILECHECK_ERROR;
	}
	else
		info->errorflag|=AUDIOFILECHECK_ERROR;

}

void AudioHDFile::StopRecording(Seq_Song *song,int *added,int *deleted,bool writeerrorfiles)
{
	seekbeforewrite=-1;
	reccycleloopcounter=0; // Reset

	recmix.DeleteARESOut();

	if(recmix.inputbuffer32bit) // RAW Mix
	{
		delete recmix.inputbuffer32bit;
		recmix.inputbuffer32bit=0;
	}

	if(camxrecorded==false && (samplesperchannel==0 || deleterecording==true || (writeerrorfiles==false && writefile.errorwriting==true))){

		// Delete this ?
		deleted++;

		writefile.Close(true);
		DeleteFileOnHD();
		FreeMemory();

		if(peakbuffer){
			peakbuffer->FreePeakMemory(true);
			delete peakbuffer;
			peakbuffer=0;
		}

		// Remove Empty Recording from Tracks
		if(song==mainvar->GetActiveSong())
		{
			Seq_Track *t=song->FirstTrack();

			while(t){

				for(int i=0;i<MAXRECPATTERNPERTRACK;i++){
					if(t->audiorecord_audiopattern[i] && t->audiorecord_audiopattern[i]->audioevent.audioefile==this){

						TRACE ("Delete 0 Sample Record Pattern...\n");

						maingui->RemovePatternFromGUI(song,t->audiorecord_audiopattern[i]);

						mainthreadcontrol->LockActiveSong();
						t->DeletePattern(t->audiorecord_audiopattern[i],true);
						t->audiorecord_audiopattern[i]=0;
						mainthreadcontrol->UnlockActiveSong();

						deleted++;
						TRACE ("... Delete Done\n");
					}
				}

				t=t->NextTrack();
			}
		}

		mainaudio->RemoveAudioRecordingFile(this);
	}
	else{
		added++;

		//TRACE ("Record Last !0 Sample %d\n",rec->recmaxpeakposition_zerosample);

		EndAndAddAudioRecording(song);

		//if(onefile)
		//	return;
	}
}

void AudioHDFile::EndAndAddAudioRecording(Seq_Song *song)
{
	if(camxrecorded==false)
	{
#ifdef OLDIE
		// Cut Last Zero Samples
		if(mainvar->GetActiveProject() && mainvar->GetActiveProject()->autocutzerosamples==true && 
			recmaxpeakposition_zerosample+2<samplesperchannel) // 1.5,0,0,0,0 -> 1.5,0
		{
			TRACE ("Cut File Length... %d -> %d\n",samplesperchannel,recmaxpeakposition_zerosample);

			samplesperchannel=rec->recmaxpeakposition_zerosample+2;
			datalen=rec->samplesize_all_channels*rec->samplesperchannel; // +bytes

			writefile.SetFileSize(samplesperchannel*samplesize_all_channels+headerlen);

			if(peakbuffer)
				peakbuffer->SetNewFileSize(rec->samplesperchannel);

			changedsize=true;
		}
#endif

		WriteHeader(); // File is in save mode

		// Reset Audio HD File
		m_ok=false;
		writefile.Close(true);

		camxrecorded=true; // Set CamX Intern Flag
	}

	int openflag=0;

	// Move Record List to Playback List
	mainaudio->MoveRecFileToHDFiles(this);
	
	// Close File + Read Pattern
	if(peakbuffer)
	{
		ClosePeakBuffer();

		LONGLONG header[4]; // Header 1
		double maxpeak=peakbuffer->maxpeakfound;   // Header 2
		char type[5];	  // Header 3
		bool fsamplefound=false,lsamplefound=false;
		//LONGLONG fsample=rec->peakbuffer->firstpeaksample,lsample=rec->peakbuffer->lastpeaksample;

		// WriteHeader
		header[0]=channels; // number of channels
		header[2]=PEAKBUFFERBLOCKSIZE; // block size
		header[1]=header[3]=peakbuffer->peaksamples; // backup samples

		type[0]='c';
		type[1]='a';
		type[2]='P';
		type[3]='X';
		type[4]=PEAKFILEVERSION;

		size_t fstrlen=strlen(GetName());

		if(char *peakfilename=new char[fstrlen+16])
		{
			strcpy(peakfilename,GetName());	

			// Replace . = _
			char *r=peakfilename;
			size_t i=strlen(peakfilename);

			while(i--){
				if(*r=='.')*r='_';
				r++;
			}

			strcpy(&peakfilename[fstrlen],PEAKFILENAME);

			peakbuffer->samplefilename=mainvar->GenerateString(GetName());

			camxFile writepeak;

			if(writepeak.OpenSave(peakfilename)==true)
			{
				peakbuffer->peakfilename=mainvar->GenerateString(peakfilename);

				writepeak.Save(header,4*sizeof(LONGLONG)); // write header
				writepeak.Save(&maxpeak,sizeof(double));
				//	writepeak.Save(&fsample,sizeof(LONGLONG));
				//	writepeak.Save(&lsample,sizeof(LONGLONG));

				writepeak.Save(type,5);

				for(int i=0;i<peakbuffer->channels;i++)
				{
					writepeak.Save(peakbuffer->channelbuffer[i],2*sizeof(SHORT)*peakbuffer->peaksamples);
				}

				openflag=OPENAUDIOHD_NOPEAK;
			}
#ifdef DEBUG
			else
				maingui->MessageBoxError(0,"Write PeakFile");
#endif
			writepeak.Close(true);

			delete peakfilename;
		}

		peakbuffer->initok=true;

		mainaudio->AddPeakBuffer(peakbuffer);
	}

	Open(openflag);
}

void AudioHDFile::InitHDFile()
{	
	if(m_ok==true)
		return;

	errorflag=0;

	if(name){

		AudioFileInfo info;

		if(mainaudio->CheckIfAudioFile(name,0,0,&info)==true){

			filenotfound=false;

			// Calc Ticks+Samples

			filelength=info.filelength;
			type=info.type;
			channels=info.channels;
			datalen=info.datalen;
			datastart=info.datastart;
			dataend=info.dataend;
			headerlen=info.headerlen;
			samplebits=info.bits;
			samplesarefloat=info.samplesarefloat;
			samplerate=info.samplerate;
			m_ok=info.m_ok;

			samplesperchannel=datalen;
			samplesperchannel/=channels;

			switch(samplebits)
			{
			case 16:
				samplesperchannel/=2;
				samplesize_one_channel=2;
				samplesize_all_channels=2*channels;
				break;

			case 20:
			case 24:
				samplesperchannel/=3;
				samplesize_one_channel=3;
				samplesize_all_channels=3*channels;
				break;

			case 32:
				samplesperchannel/=4;
				samplesize_one_channel=4;
				samplesize_all_channels=4*channels;
				break;

			case 64:
				samplesperchannel/=8;
				samplesize_one_channel=8;
				samplesize_all_channels=8*channels;
				break;
			}
		}
		else{

			errorflag|=OPENREADFILE_ERROR;
			filenotfound=true;

			samplesperchannel=0;
			samplesize_one_channel=0;
			samplesize_all_channels=0;
		}

	}
	else
		errorflag|=AUDIOFILENAME_ERROR;
}

/*
void AudioHDFile::CreatePeakFile_Force()
{
if( m_ok==true && (!peakbuffer) && peakbuffererrorcounter==0){

waitingforpeakfile=true;
audiopeakthread->AddCreatePeakFile(this,0);
}
}
*/

AudioPeakBuffer *AudioHDFile::OpenPeakFile(char *orgname,char *peakfilename)
{
	camxFile peakfile;

	if(peakfile.OpenRead(peakfilename)==true)
	{
		camxFile orgfile;

		if(orgfile.OpenRead(orgname)==true)
		{
			int c=orgfile.CompareDate(&peakfile);

			// Create Peakfile List Eleement
			peakfile.Close(true);
			orgfile.Close(true);

			if(c<=0) //==-1 & 0 == Audio File older than Peakfile
			{
				mainthreadcontrol->Lock(CS_peakbuffer);

				AudioPeakBuffer *apk=mainaudio->FirstPeakBuffer();

				// does buffer exists in list ?
				while(apk)
				{
					if(apk->peakfilename)
						if(strcmp(peakfilename,apk->peakfilename)==0)
						{
							mainthreadcontrol->Unlock(CS_peakbuffer); // Found
							return apk;
						}

						apk=apk->NextAudioPeakBuffer();
				}

				mainthreadcontrol->Unlock(CS_peakbuffer);

				if(apk=new AudioPeakBuffer)
				{
					apk->samplefilename=mainvar->GenerateString(orgname);
					apk->peakfilename=mainvar->GenerateString(peakfilename);

					if(apk->ReadPeakFile(this)==true)
					{
						mainaudio->AddPeakBuffer(apk);
						return apk;
					}

					// Error reading Peak File

					apk->FreePeakMemory(true);
					delete apk;

					return 0;

				}//if apk
			} // date ok?
			else
			{
				// PeakFile newer than Audio File ...
				mainvar->DeleteAFile(peakfilename);
			}

			return 0;

		}// if open org

		peakfile.Close(true);

	}// if open peak

	return 0;
}

#ifdef PEAKSIZETEST
/*
bool ok=false;

if(apk->samplefilename=mainvar->GenerateString(orgname))
{
if(apk->peakfilename=mainvar->GenerateString(peakfilename))
{
//TRACE ("Read Peak File %s = %d\n",apk->peakfilename,samplesperchannel/PEAKBUFFERSIZE);
ok=apk->ReadPeakFile();

if(ok==true){

LONGLONG filepeaks=samplesperchannel/PEAKBUFFERSIZE;

/*
if(filepeaks*PEAKBUFFERSIZE!=samplesperchannel)
filepeaks++;
*/

// Check Peak
if(apk->channels!=channels ||
   samplesperchannel/PEAKBUFFERSIZE!=apk->peaksamples)
   ok=false;
				}
			}
		}

		/*
		if(ok==false){

		// Error
		apk->LockO();
		apk->FreePeakMemory(true);
		apk->UnlockO();

		delete apk;
		apk=0;

		// Delete File
		mainvar->DeleteAFile(peakfilename);
		}
		*/
		//if(apk)
#endif

		void AudioHDFile::CreatePeakFile(bool withnewcreate)
		{
			if(m_ok==true && (!peakbuffer) && dontcreatepeakfile==false)
			{
				// Inside List ?
				AudioPeakBuffer *inside=nopeakfileasfile==true?false:mainaudio->FindPeakBuffer(name);

				if(inside) // in List
				{
					inside->LockO();

					waitingforpeakfile=false;
					peakbuffer=inside;
					inside->UnlockO();

					return;
				}

				// Find File on HD
				size_t fstrlen=strlen(name);
				bool createnew=false;

				if(char *peakfilename=new char[fstrlen+16]){

					camxFile peakfile;
					//	peakfile.nobuffer=true;

					strcpy(peakfilename,name);

					// Replace . = _
					{
						char *r=peakfilename;
						size_t i=strlen(peakfilename);

						while(i--){
							if(*r=='.')*r='_';
							r++;
						}
					}

					strcpy(&peakfilename[fstrlen],PEAKFILENAME);

					// Add PEAKFILENAME	
					if(!peakfile.OpenRead(peakfilename)) // no peakfile
					{
						// No PeakFile
						createnew=true;	
						peakfile.Close(true);
					}
					else 
					{
						// Peakfile exists
						peakfile.Close(true);

						if((!peakbuffer) || withnewcreate==true){
							// Close Peakfile
							if(peakbuffer){
								bool deleted;
								mainaudio->ClosePeakFile(this,peakbuffer,true,&deleted); // close old peakfile			
								peakbuffer=0;
							}	

							peakbuffer=OpenPeakFile(name,peakfilename);

							if(!peakbuffer) // old version
								createnew=true;
						}
					}

					delete peakfilename;

					if(createnew==true){

						waitingforpeakfile=true;
						audiopeakthread->AddCreatePeakFile(this,0);
					}

				}//if peakfilename
			}
		}
