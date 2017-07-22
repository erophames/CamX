#include "audiofilework.h"
#include "semapores.h"
#include "editfunctions.h"
#include "object_song.h"

#ifdef CAMX
#include "songmain.h"
#include "audiohardware.h"
#include "gui.h"
#include "undofunctions_pattern.h"

#define ConvertRawToFloat ConvertRawToFloat_Ex
#define ConvertFloatToRaw ConvertFloatToRaw_Ex
#endif

#include "decoder.h"
#include "languagefiles.h"
#include "settings.h"

#define	RS_BUFFER_LEN		8192	/*-(1<<16)-*/ // Resampler Buffer

// ****************************** Audio V Work ************************
void AudioVirtualWork::MuteSamples(LONGLONG from,LONGLONG to)
{
	LONGLONG from_block=from/AFW_BLOCKSIZE,to_block=to/AFW_BLOCKSIZE;

	if(from_block<=to_block)
	{
		LONGLONG diff=to_block-from_block;

		if(diff==0)
			diff=1; // cut min 1 block

		if(blockmem) 
		{
			// Copy and sub Blocks 
			LONGLONG tempsize=blockmem_blocks-diff;

			if(LONGLONG *temp=new LONGLONG[tempsize]) // mem alloc ok ?
			{
				LONGLONG *r=blockmem;
				LONGLONG *w=temp;

				for(LONGLONG c=0;c<blockmem_blocks;c++)
				{
					if(c<from_block || c>to_block)
						*w++=*r++; // Copy Block Index Number
				}

				delete blockmem; // delete old blockmem

				blockmem=temp; 
				blockmem_blocks=tempsize;
			}
		}
		else
		{
			// Block Mem Init 0-File Blocksize
			LONGLONG tempsize=orgblocksize-diff;

			if(LONGLONG *temp=new LONGLONG[tempsize]) // mem alloc ok ?
			{
				LONGLONG *w=temp;

				blockmem=temp;
				blockmem_blocks=tempsize;

				for(LONGLONG c=0;c<orgblocksize;c++)
				{
					if(c<from_block || c>to_block)
						*w++=c; // Init Block Index Number
				}
			}
		}
	}
}

bool AudioVirtualWork::Seek(LONGLONG sample)
{
	if(sample>=0)
	{
		LONGLONG block=sample/AFW_BLOCKSIZE;

		if(blockmem)
		{
			if(block+blockpointer<blockmem_blocks)
			{
				blockpointer+=block;
				return true;
			}
		}
		else
		{
			if(block+blockpointer<orgblocksize)
			{
				blockpointer+=block;
				return true;
			}
		}
	}

	return false; // seek error or >eof
}

LONGLONG AudioVirtualWork::FirstBlock()
{
	if(blockmem) // Cuts
		return *blockmem;

	return 0;
}

LONGLONG AudioVirtualWork::GetReadBlock()
{
	return blockpointer;
}

LONGLONG AudioVirtualWork::NextBlock(LONGLONG block)
{
	if(blockmem)
	{
		if(block<blockmem_blocks)
			return blockmem[block+1];

		return -1; // eof
	}
	else
	{
		if(block<orgblocksize)
			return block++;

		return -1; // eof
	}
}

// Virtual Cut
void AudioVirtualWork::RecalcVSamples()
{
	if(blockmem)
	{
		// delete old block buffer
		delete blockmem; 
		blockmem=0;
	}

	if(cuts>0)
	{
		for(int i=0;i<cuts;i++)
			MuteSamples(avcuts[i].from,avcuts[i].to);
	}
}

bool AudioVirtualWork::AddCutSamples(LONGLONG from,LONGLONG to)
{
	if(cuts<AFW_MAXVCUTS && from<to && from<vsamplesize && to<vsamplesize)
	{
		avcuts[cuts].from=from;
		avcuts[cuts++].to=to;

		RecalcVSamples();
		return true;
	}

	return false;
}

bool AudioVirtualWork::Undo()
{
	return false;
}

bool AudioVirtualWork::Redo()
{
	return false;
}

// AudioFile Work
bool AudioFileWork::CheckHDFile(AudioHDFile *f,int type)
{
	if(f)
	{
		if(strcmp(f->GetName(),hd.GetName())==0)
			return false;
	}

	return true;
}

void AudioFileWork::CreateFileNameTMP()
{
	if(creatednewfile)
	{
		size_t s=strlen(creatednewfile);

		if(s>0)
		{
			s--;

			char *h=&creatednewfile[s];

			while(s--)
			{
				if(*h=='\\' || *h==':')
				{
					char *h2=mainvar->GenerateString(h+1);
					delete creatednewfile;
					creatednewfile=h2;
					break;
				}

				h--;
			}
		}
	}
}

bool AudioFileWork::CreateTMP(char *add)
{
	if(creatednewfile)
		delete creatednewfile;

	creatednewfile=0;

	if(filename)
	{
		size_t s=add?strlen(add):16;

		creatednewfile=new char[strlen(filename)+s+16]; // +tmp etc.

		if(creatednewfile)
		{
			strcpy(creatednewfile,filename);

			//  remove .wav etc ext
			size_t i=strlen(creatednewfile);

			while(i--) // find . in string
			{
				if(creatednewfile[i]=='.')
				{
					creatednewfile[i]=0;
					break;
				}
			}

			if(add)
			{
				if(char *h=mainvar->GenerateString("_",add,".wav"))
				{
					mainvar->AddString(creatednewfile,h); // name it 44_tmp_filename*.wave
					delete h;
				}
			}
			else
				mainvar->AddString(creatednewfile,"_tmp.wav"); // name it _tmp_filename*.wave

			return true;
		}
	}

	return false;
}

void AudioFileWork::Init(char *fname)
{
	stopped=false;

	if(filename)delete filename;
	if(creatednewfile)delete creatednewfile;

	filename=0;
	creatednewfile=0;

	audiofilecheck=false;

	if(fname)
	{
		filename=mainvar->GenerateString(fname);
		InitFile();
	}
}

void AudioFileWork::DeleteWork() //v
{
	//hd.Close();
	regions.DeleteAllO();
	hd.FreeMemory();

	if(filename)delete filename;
	if(creatednewfile)delete creatednewfile;

	filename=creatednewfile=0;
}

void AudioFileWork::AddGUIMessage(int type,LONGLONG from,LONGLONG to)	// Send GUI Message, Info to Main Thread
{
	if(skipped==false && hd.GetName())
	{
		if(AudioWorkedFile *wf=new AudioWorkedFile(type,from,to))
		{
			wf->fromfilework=this;

			wf->camximport=camximport;
			regions.MoveListToList(&wf->regions);

			bool cerror=false;

			if(creatednewfile)
			{
				if(!(wf->createnewfile=mainvar->GenerateString(creatednewfile)))
					cerror=true;
			}

			if(cerror==false)
			{
				if(wf->filename=mainvar->GenerateString(hd.GetName()))
				{
					audioworkthread->AddWorkedFile(wf);
					maingui->SendGUIMessage(MESSAGE_AUDIOFILEWORKED,wf);
				}
				else
					cerror=true;
			}

			if(cerror==true)
			{
				if(wf->createnewfile)delete wf->createnewfile;
				delete wf;
			}


		}
	}
}

void AudioFileWork::InitFile()
{
	if(audiofilecheck==false && filename)
	{
		hd.Open(filename);

		if(hd.errorflag==0)
		{
			audiofilecheck=true;

			// Init VWork
			vwork.vsamplesize=vwork.orgsamplesize=hd.samplesperchannel;
			vwork.orgblocksize=(hd.samplesperchannel/AFW_BLOCKSIZE)+1;
		}
	}
}

#ifndef CAMX
bool AudioFileWork::ConvertRawToFloat(void *from,float *to,long samples)
{
	switch(hd.samplebits)
	{
	case 16: // 16 bit
		// 16 Bit
		short *bit16=(short *)from;

		while(samples--)
		{
			float h=*bit16++;

			h/=32768;

			*to++=h;
		}
		return true;
	}

#ifdef _DEBUG
	MessageBox(NULL,"Unknown Sample Format Raw->Float","Error",MB_OK);
#endif

	return false; // no sample format found
}

void AudioFileWork::ConvertFloatToRaw(float *from,void *to,long samples)
{
	// 16 Bit
	short *bit16=(short *)to;

	while(samples--)
	{
		if(*from>=1)
			*bit16++ =32767;
		else
			if(*from<=-1)
				*bit16++ =-32768;
			else
				*bit16++ =(short)(*from*32768);

		from++;
	}
}
#endif


extern void convtowave(Decoder *decoder);

void AudioFileWork_Converter::WriteResampler(Decoder *decoder)
{
	int i=decoder->decodeddata_samples;

	while(i>0 || src_data.input_frames )
	{
		if(src_data.input_frames ==0) // refill
		{
			int rsamples=i<bsize?i:bsize;

			i-=rsamples;

			decoder->ConvertToARES(input,rsamples); // to float

			// Resampling
			src_data.input_frames=rsamples;
			src_data.data_in = input ;
		}

		int error = src_process (src_state, &src_data);

		ok=false;

		if(src_data.output_frames_gen)
		{
			output_count += src_data.output_frames_gen ;

			src_data.data_in += src_data.input_frames_used * decoder->channels ;
			src_data.input_frames -= src_data.input_frames_used ;

			buff.CopyFromFrames(output,src_data.output_frames_gen,decoder->channels); // l/r->[L][R]

			// Back to RAW Stream
			savedecoder.ConvertSampleBufferToRAW(&buff,src_data.output_frames_gen,decoder->channels);

			savedecoder.Save(buff.inputbuffer32bit,src_data.output_frames_gen*decoder->channels*(savedecoder.samplebits/8));
		}

	}// while samples
}

bool AudioFileWork_Converter::DecodeFirstSamples(Decoder *decoder)
{
	AudioFileWork_Converter *em=(AudioFileWork_Converter *)decoder->win;

	bool ok=false;

	camxFile dsave;

	if(em->outputfile || dsave.OpenFileRequester(0,decoder->win,Cxs[CXS_SELECTENCODEDFILE],dsave.AllFiles(camxFile::FT_WAVES),false,decoder->file)==true)
	{
		em->savedecoder.channels=decoder->channels;
		em->savedecoder.samplebits=decoder->samplebits;

		em->savedecoder.samplerate=mainaudio->GetGlobalSampleRate();
		em->savedecoder.externsamplerate=true;

		if(decoder->samplerate!=em->savedecoder.samplerate) // Init Resampler
		{
			em->bsize=RS_BUFFER_LEN/decoder->channels;

			if(em->buff.Create32BitBuffer_Input(decoder->channels,em->bsize)==true)
			{
				int error=0;
				double src_ratio=em->savedecoder.samplerate;
				src_ratio/=decoder->samplerate;

				em->input=new ARES[RS_BUFFER_LEN];
				em->output=new ARES[RS_BUFFER_LEN] ;

				if(em->output && em->input)
				{
					/* Initialize the sample rate converter. */
					if ((em->src_state = src_new (SRC_SINC_MEDIUM_QUALITY,decoder->channels, &error)) == NULL)
						em->resampler=false;
					else
						em->resampler=true;

					em->src_data.end_of_input = 0 ; /* Set this later. */

					em->src_data.input_frames = 0 ;
					em->src_data.src_ratio = src_ratio ;

					em->src_data.data_out = em->output ;
					em->src_data.output_frames = em->bsize ;
				}
			}
		}

		if(!em->outputfile)
			dsave.AddToFileName(".wav");

		if(em->savedecoder.InitAudioFileSave(em->outputfile?em->outputfile:dsave.filereqname)==true)
		{
			em->WriteDecodedData(decoder);
			em->decodedfile=mainvar->GenerateString(em->outputfile?em->outputfile:dsave.filereqname);
			ok=true;
		}
	}

	dsave.Close(true);

	return true;
}

bool AudioFileWork_Converter::WriteDecodedData(Decoder *decoder)
{
	AudioFileWork_Converter *em=(AudioFileWork_Converter *)decoder->win;

	double c1=decoder->fileread,c2=decoder->filesize;

	c1/=c2;

	c1*=100;
	em->progress.SetPercent(c1);

	if(em->resampler==true)
		em->WriteResampler(decoder);
	else
	{
		em->output_count+=decoder->decodeddata_samples;
		em->savedecoder.writefile.Save(decoder->decodeddata,decoder->decodeddata_bytes);
	}

	return true;
}

bool AudioFileWork_Converter::WriteDecodedDataEnd(Decoder *decoder)
{
	AudioFileWork_Converter *em=(AudioFileWork_Converter *)decoder->win;

	em->savedecoder.datalen=em->output_count*(em->savedecoder.samplebits/8)*em->savedecoder.channels;
	em->savedecoder.WriteHeader();

	if(em->output)delete em->output;
	if(em->input)delete em->input;

	if(em->resampler==true)
		src_delete (em->src_state) ;
	// char *h=mainvar->GenerateString(em->savedecoder.writefile.fname);



	//if(h)
	//{
	//	em->AddFile(h);
	//	delete h;
	//}

	return true;
}

void AudioFileWork_Converter::Stop()
{
	if(decoder)
	{
		stopped=true;
		decoder->stop=true;
	}
}

void AudioFileWork_Converter::Start()
{
	if(inputfile)
	{
		progress.ResetInfo();

		decoder=new Decoder;

		if(decoder)
		{
			decoder->FirstData=&DecodeFirstSamples;
			decoder->WriteData=&WriteDecodedData;
			decoder->WriteDataEnd=&WriteDecodedDataEnd;
			decoder->win=(guiWindow *)this;

			decoder->file=mainvar->GenerateString(inputfile);

			progress.Start("Converting");

			if(decoder->file)convtowave(decoder);

			decoder->Close();

			progress.End(stopped);

			buff.Delete32BitBuffer();

			/*
			if(savedecoder.samplerate!=mainaudio->GetGlobalSampleRate())
			{
			char *h=mainvar->GenerateString(savedecoder.writefile.fname);

			savedecoder.writefile.Close(true);

			if(h)
			{
			if(AudioFileWork_Resample *work=new AudioFileWork_Resample){

			work->Init(h); // org file
			work->newsamplerate=mainaudio->GetGlobalSampleRate();
			audioworkthread->AddWork(work);
			}
			delete h;
			}

			}
			else
			*/

			{
				savedecoder.writefile.Close(true);

				if(decodedfile)
				{
					if(stopped==false)
					{
						creatednewfile=mainvar->GenerateString(decodedfile);
						AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_CONVERTED,0,0);
					}
					else
						mainvar->DeleteAFile(decodedfile);

					delete decodedfile;
					decodedfile=0;
				}
			}

			delete decoder;
		}//if decoder

		delete inputfile;

		if(outputfile)delete outputfile;
	}
}

void AudioFileWork_Resample::Start()
{
	progress.ResetInfo();

	if(audiofilecheck==true && newsamplerate!=hd.samplerate)
	{
		ok=true;

		if(ok==true)
		{
			SRC_STATE	*src_state ;
			SRC_DATA	src_data ;
			int			error ;

			int	output_count = 0 ;

			double src_ratio=newsamplerate;
			src_ratio/=hd.samplerate;

			int bsize=RS_BUFFER_LEN/hd.channels;

			ARES *input=new ARES[RS_BUFFER_LEN],
				*output=new ARES[RS_BUFFER_LEN] ;

			if(output && input)
			{
				/* Initialize the sample rate converter. */
				if ((src_state = src_new (SRC_SINC_MEDIUM_QUALITY, hd.channels, &error)) == NULL)
					ok=false;

				src_data.end_of_input = 0 ; /* Set this later. */

				src_data.input_frames = 0 ;
				src_data.src_ratio = src_ratio ;

				src_data.data_out = output ;
				src_data.output_frames = bsize ;

				camxFile readfile;

				if(ok==true && readfile.OpenRead(hd.GetName())==true)
				{
					// Check new file size
					AudioHDFile clone;
					hd.CloneHeader(&clone);

					switch(mainsettings->audioresamplingformat)
					{
					case 0: // no changes
						break;

					case 1: //24 bit
						if(clone.samplebits<24)
						{
							clone.samplebits=24; 
							clone.samplesize_one_channel=3; 
							clone.samplesize_all_channels=3*clone.channels;
						}
						break;

					case 2: //32 bit
						if(clone.samplebits<32)
						{
							clone.samplebits=32; 
							clone.samplesize_one_channel=4; 
							clone.samplesize_all_channels=4*clone.channels;
						}
						break;

					case 3: //32 bit
						if(clone.samplebits<64)
						{
							clone.samplebits=64; 
							clone.samplesize_one_channel=8; 
							clone.samplesize_all_channels=8*clone.channels;
						}
						break;
					}

					clone.samplerate=newsamplerate;

					readfile.SeekBegin(hd.datastart); // Seek to first Sample

					if(char *filename=hd.GetName())
					{
						char *tmph=mainaudio->GenerateSampleRateTMP();

						if(CreateTMP(tmph)==true)
						{
							camxFile test;

							// Test for existing resampled file
							if(test.OpenRead(creatednewfile)==false || mainaudio->CheckIfAudioFile(creatednewfile)==false)
							{
								clone.InitAudioFileSave(creatednewfile);

								if(clone.errorflag==0) // Open Save and Write Header
								{
									AudioHardwareBuffer buff;

									LONGLONG readsamples=0,
									 i=hd.samplesperchannel,
									 sampleswritten=0;

									if(buff.Create32BitBuffer_Input(hd.channels,bsize)==true)
									{
										progress.Start("Resampling");

										while((i>0 || src_data.input_frames) && stopped==false)
										{
											if(src_data.input_frames ==0)
											{
												int c=i<bsize?(int)i:bsize;
												i-=c;

												readsamples+=c;
												readfile.Read(buff.inputbuffer32bit,c*hd.samplesize_all_channels); // -> RAW

												// Convert RAW Stream -> ARES
												buff.channelsused=0; // Reset
												hd.ConvertReadBufferToSamples(buff.inputbuffer32bit,&buff,c,hd.channels);

												buff.CopyToFrames(input,c,hd.channels); // [l][r]->l/r

												// Resampling
												src_data.input_frames=c;
												src_data.data_in = input ;

												if(i==0)
													src_data.end_of_input=1;
											}

											if ((error = src_process (src_state, &src_data)))
											{
												ok=false;
												break;
											}

											if(src_data.output_frames_gen)
											{
												output_count += src_data.output_frames_gen ;

												src_data.data_in += src_data.input_frames_used * hd.channels ;
												src_data.input_frames -= src_data.input_frames_used ;

												buff.CopyFromFrames(output,src_data.output_frames_gen,hd.channels); // l/r->[L][R]

												// Back to RAW Stream
												clone.ConvertSampleBufferToRAW(&buff,src_data.output_frames_gen,hd.channels);

												clone.Save(buff.inputbuffer32bit,src_data.output_frames_gen*clone.samplesize_all_channels);

												sampleswritten+=src_data.output_frames_gen;

												// Calc Percent
												double h=readsamples,h2=hd.samplesperchannel;
												h/=h2;
												h*=100;
												progress.SetPercent(h);
											}

										}// while samples

										progress.End(stopped);

										clone.datalen=sampleswritten*clone.samplesize_all_channels;

										if(ok==true && stopped==false)
											clone.WriteHeader(); // File is in save mode

										clone.writefile.Close(true); // close save file
#ifdef CAMX
										if(ok==false || stopped==true) // delete tmp
											mainvar->DeleteAFile(creatednewfile);
										else
											AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_RESAMPLING,0,0);
#endif
									}//if clone
									else
										error=true;

									buff.Delete32BitBuffer();
								}// if buff memory
								else
									error=true;
							}
							else
							{
								test.Close(true); // resampled file exists
								skipped=true;
							}

						}// if tmp save
						else
							error=true;

						if(tmph)
							delete tmph;

					}//if filename
					else
						error=true;

					readfile.Close(true);

				}// if readfile
				else
					error=true;

				if(src_state)
					src_state = src_delete (src_state) ;
			}
			else
				error=true;

			if(input)
				delete input;

			if(output)
				delete output;

		} // maxpeak check
		else
			error=true;

	}//if fileok
	else
		error=true;
}

void AudioFileWork_Normalize::Start()
{
	bool normalized=false;

	progress.ResetInfo();

	if(audiofilecheck==true)
	{
		if(userange==false || (samplestart>=0 && sampleend>samplestart))
			ok=true;
		else
		{
			error=true;
			ok=false;
		}

		if(ok==true && maxpeak>0 && maxpeak<1)
		{
			camxFile readfile;
			//	readfile.nobuffer=true;

			if(readfile.OpenRead(hd.GetName())==true)
			{
				// Check new file size
				AudioHDFile clone;

				hd.CloneHeader(&clone);
				readfile.SeekBegin(hd.datastart); // Seek to first Sample

				if(char *filename=hd.GetName())
				{
					if(CreateTMP()==true)
					{
						clone.InitAudioFileSave(creatednewfile);

						if(clone.errorflag==0) // Open Save and Write Header
						{
							ARES mul=1/maxpeak;
							AudioHardwareBuffer buff;

#define NORMBUFF 16*1024
							LONGLONG i=hd.samplesperchannel;
							LONGLONG sampleswritten=0;

							if(buff.Create32BitBuffer_Input(hd.channels,NORMBUFF)==true)
							{
								progress.Start("Normalize Audio");

								while(i>0 && stopped==false)
								{
									int c=i<NORMBUFF?(long)i:NORMBUFF;
									i-=c;

									readfile.Read(buff.inputbuffer32bit,c*hd.samplesize_all_channels); // -> RAW

									// Convert RAW Stream -> ARES
									// buff.channelsused=0; // Force Simple Copy

									buff.channelsused=0; // Reset
									hd.ConvertReadBufferToSamples(buff.inputbuffer32bit,&buff,c,hd.channels);

									// Do Normalize
									int a=c*hd.channels;
									ARES *ac=buff.outputbufferARES;

									while(a--)
									{
										ARES h=*ac;
										h*=mul;

										// clip
										if(h>1)
											h=1;
										else
											if(h<-1)
												h=-1;

										*ac++ =h;
									}// normalize loop

									// Back to RAW Stream
									hd.ConvertSampleBufferToRAW(&buff,c,hd.channels);

									clone.Save(buff.inputbuffer32bit,c*hd.samplesize_all_channels);

									sampleswritten+=c;

									// Calc Percent
									double h=sampleswritten,h2=hd.samplesperchannel;
									h/=h2;
									h*=100;
									progress.SetPercent(h);

									normalized=true;
								}// while samples

								progress.End(stopped);

								clone.datalen=hd.datalen;

								if(ok==true)
									clone.WriteHeader(); // File is in save mode

								clone.writefile.Close(true); // close save file
#ifdef CAMX
								if(ok==false || stopped==true) // delete tmp
									mainvar->DeleteAFile(creatednewfile);
								else
									AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_NORMALIZE,0,0);
#endif
							}//if clone
							else
								error=true;

							buff.Delete32BitBuffer();
						}// if buff memory
						else
							error=true;

					}// if tmp save
					else
						error=true;
				}//if filename
				else
					error=true;

				readfile.Close(true);

			}// if readfile
			else
				error=true;
		} // maxpeak check
		else
			error=true;
	}//if fileok
	else
		error=true;
}

void AudioFileWork_ExportPatternFile::Start()
{
	ok=true;

	if(tofile && orgfile)
	{
		AudioHDFile openhd;
		openhd.Open(orgfile);

		if(openhd.m_ok==true && to>from)
		{
			camxFile infile;

			if(infile.OpenRead(orgfile)==true)
			{
				AudioHDFile savehd;
				openhd.CloneHeader(&savehd);

				if(savehd.InitAudioFileSave(tofile)==true)
				{
					int cfc=wfades.GetCount();
					AudioHDFile **cfhd=0;
					camxFile **cfread=0;
					bool *cfinit=0;

					if(cfc && split==true)
					{
						cfhd=new AudioHDFile*[cfc];
						cfread=new camxFile*[cfc];
						cfinit=new bool[cfc];

						if(cfhd && cfread && cfinit)
						{
							for(int i=0;i<cfc;i++){
								cfinit[i]=false;
								cfhd[i]=new AudioHDFile;
								cfread[i]=new camxFile;

								if(cfhd[i]==0 || cfread[i]==0){
									delete cfhd;
									delete cfread;

									cfhd=0;
									cfread=0;
									split=false;
									break;
								}
							}

							if(cfhd && cfread)
							{
								int i=0;
								Work_CrossFade *cf=(Work_CrossFade *)wfades.GetRoot();

								while(cf)
								{
									LONGLONG splitposition=cf->fade1.to_sample-cf->fade1.from_sample;
									splitposition/=2;

									if(cf->fade1.infade==false)
										to-=splitposition;
									else
										from+=splitposition;

									cfhd[i]->Open(cf->connectfile);

									if(cfhd[i]->m_ok==true)
									{
										if(cfread[i]->OpenRead(cf->connectfile)==true)
										{
											LONGLONG roffset;

											if(cf->fade1.infade==false){
												roffset=cf->fade1.from_sample;
												cf->fade1.to_sample-=splitposition;
												cf->fade2.from_sample+=splitposition;
											}
											else{
												cf->readcfsamples=splitposition;
												roffset=cf->fade2.from_sample;
												cf->fade1.from_sample+=splitposition;
												cf->fade2.to_sample-=splitposition;
											}

											roffset+=cf->from2+splitposition;
											roffset*=cfhd[i]->samplesize_all_channels;
											roffset+=cfhd[i]->datastart;

											if(roffset>0)cfread[i]->SeekBegin(roffset);
										}
									}

									i++;
									cf=cf->Next();
								}
							}
						}
						else
						{
							if(cfhd)delete cfhd;
							if(cfread)delete cfread;
							cfhd=0;
							cfread=0;
							split=false;
						}
					}

					progress.ResetInfo();
					progress.Start("Export Audio File");

					AudioHardwareBuffer buffer,tmp_buffer;
					LONGLONG samplestoread=to-from,fpos=from,inoffset=openhd.datastart;
					inoffset+=from*openhd.samplesize_all_channels;
					if(inoffset>0)infile.SeekBegin(inoffset);

					if(buffer.Create32BitBuffer_Input(openhd.channels,4096)==true &&
						tmp_buffer.Create32BitBuffer_Input(MAXCHANNELSPERCHANNEL,4096)==true
						)
					{
						LONGLONG sampleread;
						do
						{
							int rbytes=(samplestoread>=buffer.samplesinbuffer)?buffer.samplesinbuffer:samplestoread;
							rbytes=infile.Read(buffer.inputbuffer32bit,rbytes*openhd.samplesize_all_channels);

							sampleread=rbytes/openhd.samplesize_all_channels;
							fpos+=sampleread;
							samplestoread-=sampleread;

							if(sampleread>0)
							{
								buffer.channelsused=0; // dont mix

								//openhd.ConvertReadBufferToSamples(&buffer);
								openhd.ConvertReadBufferToSamples // convert input buffer to ARES
			(
			buffer.inputbuffer32bit,
			&buffer, // -> Channel Mix Buffer
			buffer.samplesinbuffer,
			buffer.channelsinbuffer
			);

								

								// Add CrossFades
								int cc=0;
								Work_CrossFade *wcf=(Work_CrossFade *)wfades.GetRoot();

								while(wcf)
								{
									Seq_CrossFade *scf=&wcf->fade1,*scf2=&wcf->fade2;

									if(scf->used==true && 
										(
										(scf->from_sample>=fpos && scf->from_sample<=fpos+sampleread) ||
										(scf->from_sample<=fpos && scf->to_sample>=fpos)
										)
										)
									{
										double cfsize=scf->to_sample-scf->from_sample,x=fpos-scf->from_sample;
										x/=cfsize;
										ARES y=scf->ConvertToVolume(x,scf->infade);

										for(int c=0;c<buffer.channelsused;c++)
										{
											ARES *sample=buffer.outputbufferARES;
											sample+=c*buffer.samplesinbuffer;

											LONGLONG i;

											//Jump Samples
											if(scf->from_sample>fpos)
											{
												LONGLONG offset=scf->from_sample-fpos;
												i=sampleread-offset;
												sample+=offset;
											}
											else
												i=sampleread; // Full Buffer

											while(i--)*sample++ *=y;
										}

										if(split==true && 
											wcf->endreached==false &&
											fpos>=wcf->fade1.from_sample && 
											cfhd[cc]->m_ok==true)
										{
											int coffset,cfsamples;

											if(cfinit[cc]==false){ // CF Start Init ?
												cfinit[cc]=true;
												cfsamples=fpos-wcf->fade1.from_sample;
												coffset=sampleread-cfsamples;
											}
											else{
												coffset=0;
												cfsamples=sampleread;
											}

											if(cfinit[cc]==true)
											{
												if(fpos>=wcf->fade1.to_sample){ // End
													cfsamples-=fpos-wcf->fade1.to_sample;
													wcf->endreached=true;
												}

												char *readto=(char *)tmp_buffer.inputbuffer32bit;

												if(coffset){
													readto+=cfhd[cc]->samplesize_all_channels*coffset;
													tmp_buffer.ClearInputTo(readto);
												}

												int cfrbytes=cfread[cc]->Read(readto,cfsamples*cfhd[cc]->samplesize_all_channels),
													cfsampleread=cfrbytes/cfhd[cc]->samplesize_all_channels;

												tmp_buffer.channelsused=0; // dont mix

												cfhd[cc]->ConvertReadBufferToSamples // convert input buffer to ARES
													(
													tmp_buffer.inputbuffer32bit,
													&tmp_buffer, // -> Channel Mix Buffer
													tmp_buffer.samplesinbuffer,
													cfhd[cc]->channels
													);

												double x=wcf->crossfadesize,h=wcf->readcfsamples;
												h/=x;
												double y=wcf->fade2.ConvertToVolume(h,wcf->fade2.infade);
												tmp_buffer.MixAudioBuffer(&buffer,y);

											}// Init True

										}//split

										wcf->readcfsamples+=sampleread;
									}// Inside CF

									cc++;
									wcf=wcf->Next();
								}

								// Back To RAW
								openhd.ConvertSampleBufferToRAW(&buffer,sampleread,buffer.channelsinbuffer);
								savehd.Save(buffer.inputbuffer32bit,sampleread*openhd.samplesize_all_channels);

								double per=fpos-from,size=to-from;
								per/=size;
								progress.SetPercent(per);
							}

						}while(sampleread>0 && samplestoread>0);
					}

					savehd.WriteHeader();
					savehd.writefile.Close(true);
					savehd.FreeMemory();

					progress.End(stopped);

					if(cfhd)
					{
						for(int i=0;i<cfc;i++)delete cfhd[i];
						delete cfhd; 
					}

					if(cfread)
					{
						for(int i=0;i<cfc;i++)cfread[i]->Close(true);
						delete cfread;
					}

					if(cfinit)delete cfinit;
				}

				infile.Close(true);
			}
		}

		openhd.FreeMemory();
	}

	// Clean
	if(tofile)delete filename;
	if(orgfile)delete orgfile;

	Work_CrossFade *cf=(Work_CrossFade *)wfades.GetRoot();
	while(cf)
	{
		if(cf->connectfile)delete cf->connectfile;
		cf=cf->Next();
	}

	wfades.DeleteAllO();
}

void AudioFileWork_SplitFileInChannels::EndWork()
{
	TRACE ("END WORK Split Audio File -> Channels ... %s\n",filename);

	// Create Audio Pattern

	if(song==mainvar->GetActiveSong())
		mainthreadcontrol->LockActiveSong();

	if(tracks && newfiles)
	{
		for(int i=0;i<count;i++){
			Seq_Track *track=tracks[i];
			char *newfile=newfiles[i];

			if(track && newfile){
				if(AudioHDFile *newhd=mainaudio->AddAudioFileQ(newfile,true))
				{
					// Create new Audio Pattern
					if(AudioPattern *ap=(AudioPattern *)mainedit->CreateNewPattern(0,track,MEDIATYPE_AUDIO,position,false,CNP_NOEDITOKCHECK|CNP_NOCHECKPLAYBACK|CNP_NOGUIREFRESH))
					{
						Undo_ReplaceAudioPatternFile replace(ap,newhd,0);
						replace.Do();
					}
				}
			}
		}
	}

	if(song==mainvar->GetActiveSong()){
		song->CheckPlaybackRefresh();
		mainthreadcontrol->UnlockActiveSong();
	}

	maingui->RefreshAllEditors(song,EDITORTYPE_ARRANGE);

	// FreeMemory

	if(tracks)
		delete tracks;

	tracks=0;

	if(newfiles){
		for(int i=0;i<count;i++)
		{
			if(newfiles[i])
				delete newfiles[i];
		}

		delete newfiles;
		newfiles=0;
	}
}

void AudioFileWork_SplitFileInChannels::Start()
{
	ok=true;

	TRACE ("Split Audio File -> Channels ... %s\n",filename);

	if(filename && count)
	{
#ifdef DEBUG
		for(int i=0;i<count;i++)
		TRACE ("Files %s \n",newfiles[i]);
#endif

		if(AudioHDFile **hdlist=new AudioHDFile*[count])
		{
				// 1. Init Save Files
				for(int i=0;i<count;i++)
				{
					if(hdlist[i]=new AudioHDFile)
					{
					hdlist[i]->channels=1;
					hdlist[i]->samplerate=samplerate;
					hdlist[i]->samplebits=samplebits;
					hdlist[i]->externsamplerate=true;

					hdlist[i]->InitAudioFileSave(newfiles[i]);
					}
				}

				AudioHDFile hd; // read

				hd.Open(filename);

				if(hd.errorflag==0)
				{
					int *buffer=new int[AFW_READWRITEBUFFERSIZE*8*count]; // read
					int *splitbuffer=new int[AFW_READWRITEBUFFERSIZE*8];

					if(buffer && splitbuffer)
					{
						camxFile readfile;

						if(readfile.OpenRead(filename)==true)
						{
							LONGLONG sampleswritten=0;
							LONGLONG samplestowrite=hd.samplesperchannel;

							readfile.SeekBegin(hd.datastart); // Seek to Sample 0
							progress.Start("Split Audio File");

							// Work Loop
							while(sampleswritten<hd.samplesperchannel && ok==true && stopped==false)
							{
								LONGLONG readsamples=samplestowrite<AFW_READWRITEBUFFERSIZE?samplestowrite:AFW_READWRITEBUFFERSIZE,
									bytes=readsamples*hd.samplesize_all_channels;

								sampleswritten+=readsamples;
								samplestowrite-=readsamples;

								if(bytes>0)
								{
									//Read sample block
									if(readfile.Read(buffer,bytes)!=bytes)
										ok=false;
								}else
									ok=false; // 0 samples ?

								for(int c=0;c<count;c++)
								{
									switch(samplebits)
									{
									case 8:
										{
											char *t=(char *)splitbuffer;

											// Split Channels
											char *f=(char *)buffer;
											f+=c;

											for(int i=0;i<readsamples;i++)
											{
												*t++=*f;
												f+=count;
											}
										}
										break;

									case 16:
										{
											short *t=(short *)splitbuffer;

											// Split Channels
											short *f=(short *)buffer;
											f+=c;

											for(int i=0;i<readsamples;i++)
											{
												*t++=*f;
												f+=count;
											}
										}
										break;

									case 18:
									case 20:
									case 24:
										{
											// 24 bit Container

											char *t=(char *)splitbuffer;

											// Split Channels
											char *f=(char *)buffer;
											f+=3*c;

											int add=3*(count-1);

											for(int i=0;i<readsamples;i++)
											{
												*t++=*f++;
												*t++=*f++;
												*t++=*f++;

												f+=add;
											}
										}
										break;

									case 32:
										{
											long *t=(long *)splitbuffer;

											// Split Channels
											long *f=(long *)buffer;
											f+=c;

											for(int i=0;i<readsamples;i++)
											{
												*t++=*f;
												f+=count;
											}
										}
										break;

									case 64:
										{
											long long *t=(long long *)splitbuffer;

											// Split Channels
											long long *f=(long long *)buffer;
											f+=c;

											for(int i=0;i<readsamples;i++)
											{
												*t++=*f;
												f+=count;
											}
										}
										break;
									}

									if(hdlist[c])
									{
									hdlist[c]->samplesperchannel+=readsamples;
									hdlist[c]->Save(splitbuffer,readsamples*hd.samplesize_one_channel);
									}

								}// for channel

								// Calc Percent
								{
									double h=sampleswritten,h2=hd.samplesperchannel;
									h/=h2;
									h*=100;
									progress.SetPercent(h);
								}

							}// while loop

							// End

							for(int i=0;i<count;i++)
							{
								if(hdlist[i])
								{
								hdlist[i]->WriteHeader();
								hdlist[i]->writefile.Close(true);
								delete hdlist[i];
								}
							}

							progress.End(stopped);

							AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_SPLITTED,0,0);

							delete buffer;
							delete splitbuffer;

						} // if buffer

						readfile.Close(true);
					}
				}

				delete hdlist;
		}// if hdlist
	}
}

void AudioFileWork_CopyFile::Start()
{
	ok=true;

	if(creatednewfile && filename)
	{
		camxFile test;

		if(test.OpenRead(creatednewfile)==true)
		{
			// Target File Exists
			test.Close(true);
			AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_COPYIED,0,0);
			return;
		}

		camxFile copy;
		progress.Start("Copy Audio File");
		bool copyok=copy.CopyFileFromTo(filename,creatednewfile);
		progress.End(stopped);
		copy.Close(true);

		if(copyok==true)
			AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_COPYIED,0,0);
		else
		{
			if(maingui)
				maingui->MessageBoxError(0,Cxs[CXS_COPYFILEERROR]);
		}
	}
}

void AudioFileWork_CreateNewFile::Start()
{
	if(copyfile==true)
	{
		ok=true;
		progress.ResetInfo();

		if(creatednewfile)
		{
			camxFile copy;
			progress.Start("Copy Audio File");
			copy.CopyFileFromTo(filename,creatednewfile);
			progress.End(stopped);
			copy.Close(true);
		}
	}
	else
	{
#ifdef DEMO
		LONGLONG demolength=0;
#endif
		int *buffer=new int[AFW_READWRITEBUFFERSIZE*8*hd.channels]; // read/write buffer

		if(buffer)
		{
			ok=true;
			progress.ResetInfo();

			if(creatednewfile && samplestart<sampleend && audiofilecheck==true)
			{
				AudioHDFile save;

				hd.CloneHeader(&save);
				save.InitAudioFileSave(creatednewfile);

				if(save.errorflag==0)
				{
					camxFile readfile;

					if(readfile.OpenRead(filename)==true)
					{
						LONGLONG sampleswritten=0,
							samplestowrite=sampleend-samplestart,
							firstsample=hd.datastart; // Sample 0

						firstsample+=hd.samplesize_all_channels*samplestart; // add offset

						readfile.SeekBegin(firstsample); // Seek to first Sample
						progress.Start("Create Audio File");

						// Work Loop
						while(sampleswritten<sampleend-samplestart && ok==true && stopped==false)
						{
							LONGLONG readsamples=samplestowrite<AFW_READWRITEBUFFERSIZE?samplestowrite:AFW_READWRITEBUFFERSIZE,
								bytes=readsamples*hd.samplesize_all_channels;

							sampleswritten+=readsamples;
							samplestowrite-=readsamples;

							if(bytes>0)
							{
								//Read sample block
								if(readfile.Read(buffer,bytes)!=bytes)
									ok=false;
							}else
								ok=false; // 0 samples ?

							//Write sample block to tmp
							if(ok==true)
							{
								save.Save(buffer,bytes);

								// Calc Percent
								double h=sampleswritten,h2=sampleend-samplestart;
								h/=h2;
								h*=100;
								progress.SetPercent(h);
							}

#ifdef DEMO
							if(sampleswritten>=mainaudio->GetGlobalSampleRate()*10)
							{
								demolength=sampleswritten;
								sampleswritten=sampleend-samplestart;
								MessageBox(NULL,"Demo Copy Limit 10sek","Demo",MB_OK);
							}
#endif
						}// while

						progress.End(stopped);

						readfile.Close(true);

					}//if readfile open
					else
						ok=false;

					// calc new datalength

#ifdef DEMO
					save.datalen=hd.samplesize_all_channels*demolength;
#else
					save.datalen=hd.samplesize_all_channels*(sampleend-samplestart);
#endif
					if(stopped==false)
					{
						if(ok==true)
							save.WriteHeader(); // File is in save mode
					}

					save.writefile.Close(true);

#ifdef CAMX
					if(ok==false || stopped==true) // delete oldfile
						mainvar->DeleteAFile(creatednewfile);
					/*
					else
					AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_CREATEFILE,0,0);
					*/
#endif
				}//if save file open
				else
					ok=false;

			}// if ok
			else
				ok=false;

			delete buffer;

#ifdef _DEBUG
			if(ok==false)
				MessageBox(NULL,"Create New File 1","Error",MB_OK);
#endif
		} // i/o buffer ?
	}
}

// Volume Down --> Zero <-- Up
bool AudioFileWork::VolumeUpDown(LONGLONG samplepos,LONGLONG samples)
{	
	bool ok=false;

#ifndef DEMO

	if(samples>0 && filename && audiofilecheck==true && samplepos<=hd.samplesperchannel)
	{
		camxFile readfile;

		if(readfile.OpenReadSave(hd.GetName())==true) // Read Open/Save OK ?
		{
			void *input_buffer=new char[samples*hd.channels*8]; // 8=64 max, Read Buffer

			if(input_buffer)
			{
				ARES *float_buffer=new ARES[samples*hd.channels]; // float buffer

				if(float_buffer) // float buffer ok ?
				{
					bool error=false;

					// Buffer 1 *************************************** |----> samplepos
					LONGLONG seek=hd.datastart,samplestoread;

					if(samples<=samplepos)
					{
						seek+=(samplepos-samples)*hd.samplesize_one_channel; // - offset ?
						samplestoread=samples; // read full buffer
					}
					else // <-- Left ?
					{
						seek+=samplepos*hd.samplesize_one_channel;
						samplestoread=samplepos; // rest
					}

					if(samplestoread>0)
					{
						readfile.SeekBegin(seek); // sample start position

						// Fill RAW Input buffer
						if(readfile.Read(input_buffer,samplestoread*hd.samplesize_all_channels)==samplestoread*hd.samplesize_all_channels)
						{
							// Convert RAW (16 Bit etc..) To Float
							if(ConvertRawToFloat(input_buffer,float_buffer,(int)samplestoread*hd.channels)==true) // Convert and Check Sample Format
							{
								// Buffer 1 Volume 1--->0
								ARES vol=0,step=1/samples,*f=float_buffer;

								f+=samplestoread*hd.channels; //mul offset buffer 2

								int i=samplestoread;
								while(i--)
								{
									for(int c=0;c<hd.channels;c++)// Left/Right Volume Mul
									{
										ARES h=*f;
										h*=vol;
										*f-- =(ARES)h;
									}

									vol+=step;
								}

								// Convert Float back to RAW (16Bit etc...)
								ConvertFloatToRaw(float_buffer,input_buffer,(int)samplestoread*hd.channels); // Back To File Format

								readfile.SeekBegin(seek); // jump back
								readfile.Save(input_buffer,samplestoread*hd.samplesize_all_channels);

								ok=true;
							}
							else
								error=true; // unable to convert RAW->float
						}// read ok ?
						else 
							error=true;
					}

					if(error==false) // Error Buffer 1 ?
					{
						if(samplepos+samples<=hd.samplesperchannel)
							samplestoread=samples; // read full buffer
						else // <-- Right ?
							samplestoread=hd.samplesperchannel-samplepos; // read rest

						if(samplestoread>0)
						{
							// Buffer 2 *************************************** samplepos ----> |
							seek=hd.datastart;
							seek+=samplepos*hd.samplesize_one_channel;

							readfile.SeekBegin(seek); // sample start position

							// Fill RAW Input buffer
							if(readfile.Read(input_buffer,samplestoread*hd.samplesize_all_channels)==samplestoread*hd.samplesize_all_channels)
							{
								// Convert RAW (16 Bit etc..) To Float
								if(ConvertRawToFloat(input_buffer,float_buffer,(int)samplestoread*hd.channels)==true)
								{
									// Buffer 1 Volume 0--->1
									ARES vol=0,step,*f=float_buffer;

									// Calc Vol Step
									step=1;
									step/=samples;

									int i=samplestoread;
									while(i--)
									{
										for(int c=0;c<hd.channels;c++) // Left/Right Volume Mul
										{
											ARES h=*f;
											h*=vol;
											*f++=h;
										}

										vol+=step;
									}

									// Convert Float back to RAW (16Bit etc...)
									ConvertFloatToRaw(float_buffer,input_buffer,(int)samplestoread*hd.channels); // Back To File Format

									readfile.SeekBegin(seek); // jump back
									readfile.Save(input_buffer,samplestoread*hd.samplesize_all_channels);

									ok=true;
								}
								else
									error=true;

							}// read ok ?
						}
					} // error ?

					delete float_buffer;

					if(error==true) // error inside buffer convert
						ok=false;

				}// if float buffer

				delete input_buffer;
			}// if raw buffer

			readfile.Close(true); // Close Read/Write File

		}// if read/save
	}
#endif

#ifdef _DEBUG
	if(ok==false)
		MessageBox(NULL,"Volume Up Down Error","Error",MB_OK);
#endif

	return ok;
}

// Cut Samples A<->B
void AudioFileWork_CutRange::Start() // v
{
	int *buffer=new int[AFW_READWRITEBUFFERSIZE*8*hd.channels]; // read/write buffer

	if(buffer)
	{
		progress.ResetInfo();

		if(audiofilecheck==true)
		{
			if(samplestart>=0 && sampleend>samplestart)
				ok=true;
			else 
				error=true;

			if(ok==true)
			{
				camxFile readfile;
				
				if(readfile.OpenRead(hd.GetName())==true)
				{
					// tmp file size
					LONGLONG newsamplessize=hd.samplesperchannel-(sampleend-samplestart),
						cutsize=(sampleend-samplestart)*hd.samplesize_all_channels,
						filesize=readfile.GetLength();

					// newfilesize
					filesize-=cutsize;

					// Check new file size
					AudioHDFile clone;

					hd.CloneHeader(&clone);
					readfile.SeekBegin(hd.datastart); // Seek to first Sample

					char *filename=hd.GetName();

					if(filename)
					{
						if(CreateTMP()==true)
						{
							clone.InitAudioFileSave(creatednewfile);

							if(clone.errorflag==0) // Open Save and Write Header
							{
								// Write Samples
								bool startreached=false,jump=false;

								LONGLONG readpos=0,
									allsamples=hd.samplesperchannel,
									sampleswritten=0,
									samplestowrite=hd.samplesperchannel-(sampleend-samplestart),
									newsamplesize=samplestowrite;

								progress.Start("Cut Audio");

								while(ok==true && samplestowrite>0 && stopped==false)
								{
									progress.working=true;

									//Read and Copy
									LONGLONG readsamples;

									// Start reached ?
									if(startreached==false && readpos+AFW_READWRITEBUFFERSIZE>=samplestart)
									{
										startreached=true;
										jump=true;
										readsamples=samplestart-readpos;
									}
									else
									{
										readsamples=AFW_READWRITEBUFFERSIZE; // read full buffer

										if(samplestowrite<=readsamples)
											readsamples=samplestowrite; // eof
									}

									LONGLONG bytes=readsamples*hd.samplesize_all_channels;

									//Read sample block
									if(bytes>0)
									{
										LONGLONG rb=readfile.Read(buffer,bytes);

										if(rb!=bytes) // Read Error
											ok=false;
									}

									//Write sample block to tmp
									if(ok==true)
									{
										if(bytes>0) // Seek ? or empty
										{
											clone.writefile.Save(buffer,bytes);

											readpos+=readsamples;
											sampleswritten+=readsamples;
											samplestowrite-=readsamples;
										}

										// Jump (Seek) ?
										if(jump==true)
										{
											jump=false;

											LONGLONG offset=(sampleend-samplestart)*hd.samplesize_all_channels; // byte offset

											readfile.SeekCurrent(offset);
											readpos+=sampleend-samplestart; // jump to clip end
											// Volume 0->norm
										}

										// Calc Percent
										double h=newsamplesize,h2=sampleswritten;

										h2/=h;
										h2*=100;

										progress.SetPercent(h2);
									}

								}// while

								progress.End(stopped);

								// cut datalength
								LONGLONG cutbytes=hd.samplesize_all_channels*(sampleend-samplestart);
								clone.datalen=hd.datalen;
								clone.datalen-=cutbytes;

								if(ok==true)
									clone.WriteHeader(); // File is in save mode

								clone.writefile.Close(true); // close save file

#ifdef CAMX
								if(ok==false || stopped==true) // delete tmp
									mainvar->DeleteAFile(creatednewfile);
								else
									AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_CUT,samplestart,sampleend);
#endif
							}//if save
							else
							{
								error=true;
								ok=false;
							}

						}//if memalloc clonename
						else 
						{
							ok=false;
							error=true;
						}

					}// if filename
					else 
					{
						error=true;
						ok=false;
					}

					readfile.Close(true);

				}// readfile open ?
				else
				{
					ok=false;
					error=true;				
				}

			}// ok ?

		}// if audio check
		else
			error=true;

		delete buffer;

		if(ok==true && crossover>0) // tmp File created ?
		{
			/*
			AudioFileWork updown(creatednewfile); // >< new tmp filter

			updown.VolumeUpDown(samplestart,crossover);
			*/
		}

#ifdef _DEBUG
		if(ok==false)
			MessageBox(NULL,"Cut Range 1","Error",MB_OK);
#endif
	}// i/o buffer ?
	else
		error=true;
}

// Cut Samples A<->B
void AudioFileWork_FillZero::Start() // v
{
	int *buffer=new int[AFW_READWRITEBUFFERSIZE*8*hd.channels]; // read/write buffer

	if(buffer)
	{
		progress.ResetInfo();

		if(audiofilecheck==true)
		{
			if(samplestart>=0 && sampleend>samplestart)ok=true;

			if(ok==true)
			{
				camxFile readfile;
				//	readfile.nobuffer=true;

				if(readfile.OpenRead(hd.GetName())==true)
				{
					// tmp file size
					LONGLONG newsamplessize=hd.samplesperchannel,filesize=readfile.GetLength();

					// Check new file size
					AudioHDFile clone;

					hd.CloneHeader(&clone);
					readfile.SeekBegin(hd.datastart); // Seek to first Sample

					if(char *filename=hd.GetName())
					{
						if(CreateTMP()==true)
						{
							clone.InitAudioFileSave(creatednewfile);

							if(clone.errorflag==0) // Open Save and Write Header
							{
								// Write Samples
								bool startreached=false,jump=false;

								LONGLONG readpos=0,
									allsamples=hd.samplesperchannel,
									sampleswritten=0,
									samplestowrite=hd.samplesperchannel-(sampleend-samplestart),
									newsamplesize=samplestowrite;

								progress.Start("Fill Zero Audio");

								while(ok==true && samplestowrite>0 && stopped==false)
								{
									progress.working=true;

									//Read and Copy
									LONGLONG readsamples;

									// Start reached ?
									if(startreached==false && readpos+AFW_READWRITEBUFFERSIZE>=samplestart)
									{
										startreached=true;
										jump=true;
										readsamples=samplestart-readpos;
									}
									else
									{
										readsamples=AFW_READWRITEBUFFERSIZE; // read full buffer

										if(samplestowrite<=readsamples)
											readsamples=samplestowrite; // eof
									}

									LONGLONG bytes=readsamples*hd.samplesize_all_channels;

									//Read sample block
									if(bytes>0)
									{
										LONGLONG rb=readfile.Read(buffer,bytes);

										if(rb!=bytes) // Read Error
										{
											error=true;
											ok=false;
										}
									}

									//Write sample block to tmp
									if(ok==true)
									{
										if(bytes>0) // Seek ? or empty
										{
											clone.writefile.Save(buffer,bytes);

											readpos+=readsamples;
											sampleswritten+=readsamples;
											samplestowrite-=readsamples;
										}

										// Jump (Seek) ?
										if(jump==true)
										{
											jump=false;

											LONGLONG offset=(sampleend-samplestart)*hd.samplesize_all_channels; // byte offset

											readfile.SeekCurrent(offset);
											readpos+=sampleend-samplestart; // jump to clip end

											// Clear Buffer
											int *c=buffer;

											for(int i=0;i<AFW_READWRITEBUFFERSIZE*hd.channels;i++)
												*c++=0;

											LONGLONG blockstowrite=(sampleend-samplestart)/AFW_READWRITEBUFFERSIZE;
											for(LONGLONG i=0;i<blockstowrite;i++)
												clone.writefile.Save(buffer,AFW_READWRITEBUFFERSIZE*hd.samplesize_all_channels);

											LONGLONG rest_blockstowrite=(sampleend-samplestart)-(blockstowrite*AFW_READWRITEBUFFERSIZE);
											clone.writefile.Save(buffer,rest_blockstowrite*hd.samplesize_all_channels);

											sampleswritten+=sampleend-samplestart;
										}

										// Calc Percent
										double h=newsamplesize,h2=sampleswritten;

										h2/=h;
										h2*=100;

										progress.SetPercent(h2);
									}

								}// while

								progress.End(stopped);

								clone.datalen=hd.datalen;

								if(ok==true)
									clone.WriteHeader(); // File is in save mode

								clone.writefile.Close(true); // close save file

#ifdef CAMX
								if(ok==false || stopped==true) // delete tmp
									mainvar->DeleteAFile(creatednewfile);
								else
									AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_FILLZERO,samplestart,sampleend);
#endif
							}//if save
							else 
							{
								error=true;
								ok=false;
							}

						}//if memalloc clonename
						else 
						{
							error=true;
							ok=false;
						}

					}// if filename
					else 
					{
						error=true;
						ok=false;
					}

					readfile.Close(true);

				}// readfile open ?
				else 
				{
					ok=false;
					error=true;					
				}

			}// ok ?

		}// if audio check

		delete buffer;

#ifdef _DEBUG
		if(ok==false)
			MessageBox(NULL,"Fill Zero","Error",MB_OK);
#endif
	}// i/o buffer ?
	else 
		error=true;
}

void AudioFileWork_Finder::Start()
{
	progress.ResetInfo();

	progress.value=true;

	counter=0;

	if(filename){

		camxFile test;
		progress.Start("Finder");
		
		for(int i=0;i<2;i++)
		{
			// 0== WAV
			// 1==AIFF

			test.BuildDirectoryList(filename,"*.*",i==0?".wav":".aif",&stopped);

			camxScan *sc=test.FirstScan();
			while(sc && stopped==false)
			{
				if(mainaudio->CheckIfAudioFile(sc->name)==true)
				{
					AudioHDFile *hdf=mainaudio->FindAudioHDFile(sc->name);

					if(!hdf)
					{
						if(AudioHDFile *check=new AudioHDFile)
						{
							check->Open(sc->name);

							if(check->errorflag==0){

								counter++;

								progress.p_value=counter;
								mainaudio->AddAudioFileNoCheck(check,directory);
								maingui->RefreshProgress();

							}
							else
							{
								check->FreeMemory();
								delete check;
							}
						}
					}
					else
					{
						if(directory)
							hdf->directory=directory;

						counter++;
					}
				}

				sc=sc->NextScan();
			}// while sc

			test.ClearScan();
		}

	}

	if(counter)
		AddGUIMessage(AudioWorkedFile::AUDIOWORKED_TYPE_FINDER,0,0);

	progress.End(stopped);
}

void AudioFileWork_FinderList::DeInit()
{
	AudioHDFile *hd=(AudioHDFile *)list.GetRoot();

	while(hd)
	{
		hd->FreeMemory();
		hd=(AudioHDFile *)list.RemoveO(hd);
	}
}

void AudioFileWork_FinderList::Start()
{
	if(filename){

		for(int i=0;i<2;i++)
		{
			camxFile test;

			// 1. WAV
			test.BuildDirectoryList(filename,"*.*",i==0?".wav":".aif",&stopped);

			camxScan *sc=test.FirstScan();
			while(sc && stopped==false)
			{
				if(AudioHDFile *check=new AudioHDFile)
				{
					check->Open(sc->name);

					if(check->errorflag==0){
						list.AddEndO(check);
					}
					else
					{
						check->FreeMemory();
						delete check;
					}
				}

				sc=sc->NextScan();
			}// while sc

			test.ClearScan();
		}
	}
}