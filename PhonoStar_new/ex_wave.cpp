#include "ex_wave.h"

/*
// Wave File Version 1.0 20.04.2009
// Copyright Martin Endres

// ****************************** Audio V Work ************************

void AudioVWork::Init()
{
	vsamplesize=orgsamplesize=wavefile->samplesperchannel;
	orgblocksize=(wavefile->samplesperchannel/EX_BLOCKSIZE)+1; // min 1
}

void AudioVWork::MuteSamples(LONGLONG from,LONGLONG to)
{
	if(allcut==false)
	{
		ULONGLONG from_block=from/EX_BLOCKSIZE;
		ULONGLONG to_block=to/EX_BLOCKSIZE;

		if(from_block<=to_block)
		{
			ULONGLONG diff=to_block-from_block;

			if(diff==0)
				diff=1; // cut min 1 block

			if(blockmem) 
			{
				// Copy and sub Blocks 
				LONGLONG tempsize=blockmem_blocks-diff;

				if(tempsize>0)
				{
					ULONGLONG *temp=new ULONGLONG[(UINT)tempsize];

					if(temp) // mem alloc ok ?
					{
						ULONGLONG *r=blockmem;
						ULONGLONG *w=temp;

						for(ULONGLONG blockc=0;blockc<blockmem_blocks;blockc++)
						{
							if(blockc<from_block || (blockc>to_block))
								*w++=*r++; // Copy Block Index Number
						}

						delete blockmem; // delete old blockmem

						blockmem=temp; 
						blockmem_blocks=tempsize;
					}
					else
						wavefile->errorflag|=ERROR_MEMORYALLOC;
				}
				else allcut=true;
			}
			else
			{
				// Block Mem Init 0-File Blocksize
				LONGLONG tempsize=orgblocksize-diff;


				if(tempsize>0)
				{
					ULONGLONG *temp=new ULONGLONG[(UINT)tempsize];

					if(temp) // mem alloc ok ?
					{
						ULONGLONG *w=temp;

						blockmem=temp;
						blockmem_blocks=tempsize;

						for(ULONGLONG blockc=0;blockc<orgblocksize;blockc++)
						{
							if(blockc<from_block || blockc>to_block)
								*w++=blockc; // Init Block Index Number
						}
					}
					else
						wavefile->errorflag|=ERROR_MEMORYALLOC;
				}
				else
					allcut=true;
			}
		}
	}
}

bool AudioVWork::Seek(LONGLONG sample)
{
	if(sample>=0)
	{
		LONGLONG block=sample/EX_BLOCKSIZE;

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

LONGLONG AudioVWork::FirstBlock()
{
	if(blockmem) // Cuts
		return *blockmem;

	return 0;
}

LONGLONG AudioVWork::GetReadBlock()
{
	return blockpointer;
}

ULONGLONG AudioVWork::NextBlock(ULONGLONG block)
{
	if(blockmem)
	{
		if(block<blockmem_blocks)
			return blockmem[block+1];
		else 
			return -1; // eof
	}
	else
	{
		if(block<orgblocksize)
			return block++;
		else
			return -1; // eof
	}
}

// Virtual Cut
void AudioVWork::RecalcVSamples()
{
	if(blockmem)
	{
		// delete old block buffer
		delete blockmem; 
		blockmem=0;
	}

	allcut=false;

	if(cuts>0)
	{
		for(int i=0;i<cuts;i++)
			MuteSamples(avcuts[i].from,avcuts[i].to);

		vsamplesize=blockmem_blocks*EX_BLOCKSIZE;
	}
	else
		vsamplesize=orgsamplesize;

}

bool AudioVWork::AddCutSamples(ULONGLONG from,ULONGLONG to)
{
	if(allcut==false)
	{
		// Quantize to block
		ULONGLONG h=from/EX_BLOCKSIZE;
		from=EX_BLOCKSIZE*h;
		h=to/EX_BLOCKSIZE;
		to=EX_BLOCKSIZE*h;

		if(cuts<MAXCUTS &&
			from<to &&
			from<vsamplesize && 
			to<vsamplesize
			)
		{
			avcuts[cuts].from=from;
			avcuts[cuts++].to=to;

			RecalcVSamples();

			return true;
		}
	}

	return false;
}

// #################### Wave File #####################################################

void WaveFile::GenerateVPeak()
{
	for(int i=0;i<channels;i++)
	{
		if(peakaftercut[i])
			delete peakaftercut[i];

		peakaftercut[i]=0;
	}

	if(vcut.cuts>0)
	{
		LONGLONG newblocksize=GetPeakBufferLength();

		if(newblocksize>0)
		{
			bool ok=true;

			for(int i=0;i<channels;i++)
			{
				peakaftercut[i]=new unsigned short[(UINT)newblocksize];

				if(!peakaftercut[i])
				{
					ok=false;
					errorflag|=ERROR_MEMORYALLOC;
					break;
				}
			}

			if(ok==true) // Copy Org Peak Blocks-> Virtual Peak Blocks
			{
				unsigned short *pp[MAXCHANNELS];

				LONGLONG b=vcut.FirstBlock();

				for(int i=0;i<channels;i++) // Init Pointer
					pp[i]=peakaftercut[i];

				while((b!=-1))
				{
					for(int i=0;i<channels;i++)
						*pp[i]++=peak[i][b]; // Copy Org -> Virtual

					b=vcut.NextBlock(b);
				}
			}
		}
	}
}

void WaveFile::SmoothUp(void *rbuffer,long buffersize) // 0->100%
{
	if(rbuffer && buffersize)
	{
		float volume=0;
		float volumestep;

		short *s16=(short *)rbuffer;
		float *f32=(float *)rbuffer;

		long samples=buffersize;

		volumestep=100;
		volumestep/=buffersize;
		volumestep/=100;

		while(samples--)
		{
			for(int i=0;i<channels;i++)
			{
				switch(samplebits)
				{
				case 16: 
					{
						float h=*s16;

						h*=volume;

						*s16++=(short)h;
					}
					break;

				case 32:
					{
						*f32*=volume;

						f32++;
					}
					break;
				}

			}// channel loop

			volume+=volumestep; // volume ... +
			if(volume>1)volume=1;

		}// sample loop
	}
}

void WaveFile::SmoothDown(void *rbuffer,long buffersize) // 100%->0
{
	if(rbuffer && buffersize)
	{
		float volume=1;
		float volumestep;

		short *s16=(short *)rbuffer;
		float *f32=(float *)rbuffer;

		long samples=buffersize;

		volumestep=100;
		volumestep/=buffersize;
		volumestep/=100;

		while(samples--)
		{
			volume-=volumestep; // volume ... -

			if(volume<0)volume=0;

			for(int i=0;i<channels;i++)
			{
				switch(samplebits)
				{
				case 16: 
					{
						float h=*s16;

						h*=volume;

						*s16++=(short)h;
					}
					break;

				case 32:
					{
						*f32*=volume;

						f32++;
					}
					break;
				}
			}// channel loop

		}// sample loop
	}
}

bool WaveFile::DoSmooth_Save() // Smooth Start+End
{
	if(init==true && smooth>0 && writeopen==true && ((writedatalen/samplesize_all_channels)>=smooth))
	{
		int *sbuffer=new int[(int)(channels*smooth)];

		if(sbuffer)
		{
			// Smooth Start 0->100%
			FILE_SeekSaveBegin(writeheaderlen); // FirstSample

			FILE_ReadFromSave(sbuffer,(int)(smooth*samplesize_all_channels));

			SmoothUp(sbuffer,smooth);

			FILE_SeekSaveBegin(writeheaderlen); // back to FirstSample
			FILE_Write(sbuffer,smooth*samplesize_all_channels);

			// Smooth End 100%->0
			ULONGLONG end=writedatalen;

			end-=smooth*samplesize_one_channel;

			FILE_SeekSaveBegin(writeheaderlen+end); // End Sample
			FILE_ReadFromSave(sbuffer,smooth*samplesize_all_channels);

			SmoothDown(sbuffer,smooth);

			FILE_SeekSaveBegin(writeheaderlen+end); // Back to End Sample
			FILE_Write(sbuffer,(int)(smooth*samplesize_all_channels));

			delete sbuffer;

			return true;
		}
		else
			errorflag|=ERROR_MEMORYALLOC;
	}

	return false;
}

bool WaveFile::Read_Seek(ULONGLONG samples)
{
	if(readopen==true && samples<=GetFileSize())
	{
		samples/=EX_BLOCKSIZE;
		samples*=EX_BLOCKSIZE;

		readbuffer_block=samples;

		ULONGLONG offset=0;

		if(vcut.cuts>0)
		{
			offset+=vcut.blockmem[readbuffer_block]*EX_BLOCKSIZE;
		}

		offset*=samplesize_all_channels;

		// Header
		offset+=datastart;

		// Seek Start Samples L/R
		FILE_SeekBegin(offset);

		return true;
	}

	return false;
}

bool WaveFile::Read_Open()
{
	read_smoothdown=false;
	read_smoothup=false;

	if(init==true)
	{
		if(!rbuffer)
			rbuffer=new long[channels*EX_BLOCKSIZE];

		if(!floatbuffer)
			floatbuffer=new float[channels*EX_BLOCKSIZE];

		if(rbuffer && floatbuffer)
		{
			if(FILE_OpenRead(filename)==true)
			{
				readopen=true;
				Read_Seek(0); // First Sample

				return true;
			}
			else
				errorflag|=ERROR_UNABLETOOPEN_READ;
		}
		else
			errorflag|=ERROR_MEMORYALLOC;
	}
	else
		errorflag|=ERROR_FILENOTINIT;

	if(rbuffer)
		delete rbuffer;

	if(floatbuffer)
		delete floatbuffer;

	return false;
}

void WaveFile::Read_Close()
{
	if(readopen==true)
	{
		FILE_Close();
		readopen=false;
	}
	else
		errorflag|=ERROR_FILENOTINIT;
}

bool WaveFile::Read_ReadSamplesToBuffer()
{
	if(rbuffer && floatbuffer && (readbuffer_block*EX_BLOCKSIZE)<GetFileSize())
	{
		float *tofloat=floatbuffer;

		FILE_Read(rbuffer,EX_BLOCKSIZE*samplesize_all_channels); // Fill buffer raw

		if(vcut.cuts>0 && (readbuffer_block+1)<vcut.blockmem_blocks)
		{
			// Seek Blocks ?
			ULONGLONG block1=vcut.blockmem[readbuffer_block];
			ULONGLONG block2=vcut.blockmem[readbuffer_block+1];

			if(block2!=(block1+1)) // Seek over block/blocks ?
			{
				ULONGLONG seekoffset=block2-block1;

				seekoffset*=EX_BLOCKSIZE*samplesize_all_channels;

				FILE_SeekCurrent(seekoffset);

				read_smoothdown=true;
				read_smoothup=true;
			}
		}

		// Smooth >< ?
		if(read_smoothdown==true)
		{
			read_smoothdown=false; // reset
			SmoothDown(rbuffer,EX_BLOCKSIZE*channels);
		}
		else
		{
			if(read_smoothup==true)
			{
				read_smoothup=false; // reset
				SmoothUp(rbuffer,EX_BLOCKSIZE*channels);
			}
		}

		// Convert RAW Readbuffer to float buffer
		switch(samplebits)
		{
		case 16:
			{
				short *fc=(short *)rbuffer;

				for(int i=0;i<(EX_BLOCKSIZE*channels);i++)
				{
					float h=*fc++;

					h/=32768;

					*tofloat ++=(short)h;
				}
			}
			break;

			// Convert 20 Bit to float

			// Convert 24 Bit to float

		case 32:
			{
				// buffer exists in 32 bit float, no convert need
				float *fc=(float *)rbuffer;
				for(int i=0;i<(EX_BLOCKSIZE*channels);i++)
					*tofloat++=*fc++;
			}
			break;
		}

		readbuffer_block++;

		return true;
	}
	else
		errorflag|=ERROR_FILENOTINIT;

	return false;
}

bool WaveFile::Normalize()
{
	bool normalized=false;

	stop=false; // Reset Stop flag

	if(init==true && initpeak==true && filename)
	{
		if(maxpeakvalue<0.99) // Normalize possible ?
		{
			char *tmp_filename=mainvar->GenerateString(filename,"_norm.wav");

			if(tmp_filename)
			{
				long rbuffersize;
				long *rbuffer=new long[rbuffersize=EX_BLOCKSIZE*channels];

				if(rbuffer)
				{
					if(FILE_OpenRead(filename)==true)
					{
						if(FILE_OpenSave(tmp_filename)==true)
						{
							// Write Header
							unsigned char buff[256];

							buff[0]='R';
							buff[1]='I';
							buff[2]='F';
							buff[3]='F';

							// clen
							buff[4]=buff[5]=buff[6]=buff[7]=0;

							buff[8]='W';
							buff[9]='A';
							buff[10]='V';
							buff[11]='E';

							FILE_Write(buff,12);

							writeheaderlen=12;

							// fmt
							{
								char w_cid[4];

								int w_clen;
								USHORT w_compcode; // default 0
								USHORT w_channels;
								int w_samplerate;
								int w_avgrate;
								USHORT w_blockallign;
								USHORT w_bitspersample;
								//USHORT extraformabytes;

								writeheaderlen+=
									sizeof(w_cid)+
									sizeof(w_clen)+
									sizeof(w_compcode)+
									sizeof(w_channels)+
									sizeof(w_samplerate)+
									sizeof(w_avgrate)+
									sizeof(w_blockallign)+
									sizeof(w_bitspersample);
								// +sizeof(extraformabytes);

								w_cid[0]='f';
								w_cid[1]='m';
								w_cid[2]='t';
								w_cid[3]=' ';

								FILE_Write(w_cid,sizeof(w_cid));

								switch(samplebits)
								{
								case 16:
								case 20:
								case 24:
									w_clen=16;
									break;

								case 32:
									w_clen=18; // + cbsize
									break;
								}

								FILE_Write(&w_clen,sizeof(w_clen));

								switch(samplebits)
								{
								case 16:
								case 20:
								case 24:
									w_compcode=1;
									break;

								case 32:
									w_compcode=3;
									break;
								}

								FILE_Write(&w_compcode,sizeof(w_compcode));

								w_channels=(USHORT)channels;
								FILE_Write(&w_channels,sizeof(w_channels));

								w_samplerate=this->samplerate;
								FILE_Write(&w_samplerate,sizeof(w_samplerate));

								w_avgrate=w_samplerate*channels*(samplebits/8);
								FILE_Write(&w_avgrate,sizeof(w_avgrate));

								w_blockallign=(USHORT)(channels*(samplebits/8));
								FILE_Write(&w_blockallign,sizeof(w_blockallign));

								w_bitspersample=(USHORT)samplebits;
								FILE_Write(&w_bitspersample,sizeof(w_bitspersample));

								if(samplebits==32) // ext
								{
									USHORT cb_size=0; // 0 ext size

									FILE_Write(&cb_size,sizeof(cb_size));

									writeheaderlen+=sizeof(cb_size);
								}
								// extraformabytes=0;
								// writefile.Save(&extraformabytes,sizeof(extraformabytes));
							}

							// PAD ?

							// data -------
							{
								char cid[4];
								int clen;

								cid[0]='d';
								cid[1]='a';
								cid[2]='t';
								cid[3]='a';

								FILE_Write(cid,sizeof(cid));

								writeheaderlen+=sizeof(cid);

								FILE_Write(&clen,sizeof(clen));

								writeheaderlen+=sizeof(clen);
							}

							writedatalen=0;
							writedataend=0;

							// End Wave Header

							// Copy Read->Write
							ULONGLONG sampleswritten=0;

							// Seek Read File
							FILE_SeekBegin(datastart);

							//
							float peakfactor=1+(1-maxpeakvalue);

							// normal copy->write
							while(sampleswritten<samplesperchannel && (stop==false))
							{
								// Clear rbuffer
								long *c=rbuffer;
								long i=rbuffersize;

								while(i--)
									*c++=0;

								ULONGLONG rsize=EX_BLOCKSIZE;
								ULONGLONG samplesleft=samplesperchannel-sampleswritten;

								if(rsize>samplesleft)
									rsize=samplesleft;

								//Read sample block
								FILE_Read(rbuffer,(UINT)rsize*samplesize_all_channels);

								// Normalize
								switch(samplebits)
								{
								case 16:
									{
										// Convert 16 bit to float
										short *fc=(short *)rbuffer;

										for(int i=0;i<(EX_BLOCKSIZE*channels);i++)
										{
											float h=*fc++;

											h/=32768;

											h*=peakfactor;
											h*=32767;

											*fc ++=(short)h;
										}
									}
									break;

									// Convert 20 Bit to float

									// Convert 24 Bit to float

								case 32:
									// buffer exists in 32 bit float, no convert need
									// Convert 16 bit to float
									float *fc=(float *)rbuffer;

									for(int i=0;i<(EX_BLOCKSIZE*channels);i++)
									{
										float h=*fc++;

										h*=peakfactor;

										*fc ++=h;
									}
									break;
								}

								// Write sample block
								FILE_Write(rbuffer,(UINT)rsize*samplesize_all_channels);

								sampleswritten+=rsize;

								// Calc Percent
								double h=(double)sampleswritten;
								double h2=(double)samplesperchannel;
								h/=h2;
								h*=100;

								if(h>100)
									h=100;

								progress=h;
							}

							// RIFF CHUNKLEN
							writedatalen=sampleswritten*samplesize_all_channels;

							// Write Header Data
							char hbuff[4];
							int *uw=(int *)hbuff;

							*uw=(int)(writeheaderlen+writedatalen);

							FILE_SeekSaveBegin( 4);
							FILE_Write(hbuff,4);

							// data CHUNKLEN
							FILE_SeekSaveBegin(writeheaderlen-4);

							*uw=(int)writedatalen;
							FILE_Write(hbuff,4);

							FILE_CloseSave();

							progress=100; // ready

							normalized=true;

						}// open write
						else
							errorflag|=ERROR_UNABLETOOPEN_SAVE;

						FILE_Close();
					}
					else
						errorflag|=ERROR_UNABLETOOPEN_READ;

					delete rbuffer;

				}// if rbuffer
				else 
					errorflag|=ERROR_MEMORYALLOC;

				delete tmp_filename;
			}// tmp
			else
				errorflag|=ERROR_MEMORYALLOC;
		}
		else
			errorflag|=ERROR_FILEISNORMALIZED;
	}
	else
		errorflag|=ERROR_FILENOTINIT;

	return normalized;
}

bool WaveFile::SaveBlock(char *newfile,ULONGLONG start,ULONGLONG end)
{
	bool startendsmooth=false;
	bool ok=false;

	if(vcut.cuts>0)
	{
		// Quantize start+end
		ULONGLONG h=start/EX_BLOCKSIZE;

		start=h*EX_BLOCKSIZE;

		h=end/EX_BLOCKSIZE;
		end=h*EX_BLOCKSIZE;
	}

	if(start>=0 && end>start)
	{
		if(start<GetFileSize() && end<=GetFileSize()) // size ok ?
			ok=true;
	}

	stop=false; // Reset Stop Flag

	progress=0;

	if(init==true && ok==true)
	{	
		long *rbuffer=new long[EX_BLOCKSIZE*channels];

		if(rbuffer)
		{
			if(FILE_OpenRead(filename)==true)
			{
				if(FILE_OpenSave(newfile)==true)
				{
					// Write Header
					unsigned char buff[256];

					buff[0]='R';
					buff[1]='I';
					buff[2]='F';
					buff[3]='F';

					// clen
					buff[4]=buff[5]=buff[6]=buff[7]=0;

					buff[8]='W';
					buff[9]='A';
					buff[10]='V';
					buff[11]='E';

					FILE_Write(buff,12);

					writeheaderlen=12;

					// fmt
					{
						char w_cid[4];

						int w_clen;
						USHORT w_compcode; // default 0
						USHORT w_channels;
						int w_samplerate;
						int w_avgrate;
						USHORT w_blockallign;
						USHORT w_bitspersample;
						//USHORT extraformabytes;

						writeheaderlen+=
							sizeof(w_cid)+
							sizeof(w_clen)+
							sizeof(w_compcode)+
							sizeof(w_channels)+
							sizeof(w_samplerate)+
							sizeof(w_avgrate)+
							sizeof(w_blockallign)+
							sizeof(w_bitspersample);
						// +sizeof(extraformabytes);

						w_cid[0]='f';
						w_cid[1]='m';
						w_cid[2]='t';
						w_cid[3]=' ';

						FILE_Write(w_cid,sizeof(w_cid));

						switch(samplebits)
						{
						case 16:
						case 20:
						case 24:
							w_clen=16;
							break;

						case 32:
							w_clen=18; // + cbsize
							break;
						}

						FILE_Write(&w_clen,sizeof(w_clen));

						switch(samplebits)
						{
						case 16:
						case 20:
						case 24:
							w_compcode=1;
							break;

						case 32:
							w_compcode=3;
							break;
						}

						FILE_Write(&w_compcode,sizeof(w_compcode));

						w_channels=(USHORT)channels;
						FILE_Write(&w_channels,sizeof(w_channels));

						w_samplerate=this->samplerate;
						FILE_Write(&w_samplerate,sizeof(w_samplerate));

						w_avgrate=w_samplerate*channels*(samplebits/8);
						FILE_Write(&w_avgrate,sizeof(w_avgrate));

						w_blockallign=(USHORT)(channels*(samplebits/8));
						FILE_Write(&w_blockallign,sizeof(w_blockallign));

						w_bitspersample=(USHORT)samplebits;
						FILE_Write(&w_bitspersample,sizeof(w_bitspersample));

						if(samplebits==32) // ext
						{
							USHORT cb_size=0; // 0 ext size

							FILE_Write(&cb_size,sizeof(cb_size));

							writeheaderlen+=sizeof(cb_size);
						}
						// extraformabytes=0;
						// writefile.Save(&extraformabytes,sizeof(extraformabytes));
					}

					// PAD ?

					// data -------
					{
						char cid[4];
						int clen;

						cid[0]='d';
						cid[1]='a';
						cid[2]='t';
						cid[3]='a';

						FILE_Write(cid,sizeof(cid));

						writeheaderlen+=sizeof(cid);

						FILE_Write(&clen,sizeof(clen));

						writeheaderlen+=sizeof(clen);
					}

					writedatalen=0;
					writedataend=0;

					// End Wave Header

					// Copy Read->Write
					ULONGLONG samplestowrite=end-start;
					ULONGLONG sampleswritten=0;

					if(vcut.cuts==0)
					{
						// Seek Read File
						ULONGLONG offset=datastart+(start*samplesize_all_channels);

						if(start!=0)
							startendsmooth=true;

						FILE_SeekBegin(offset);

						// normal copy->write
						while(samplestowrite>0 && (stop==false))
						{
							//Read sample block
							FILE_Read(rbuffer,(UINT)samplestowrite*samplesize_all_channels);

							// Write sample block
							FILE_Write(rbuffer,(UINT)samplestowrite*samplesize_all_channels);

							sampleswritten+=samplestowrite;
							// Calc Percent
							double h=(double)sampleswritten;
							double h2=(double)(end-start);
							h/=h2;
							h*=100;
							progress=h;

							if(samplestowrite>EX_BLOCKSIZE)
								samplestowrite-=EX_BLOCKSIZE;
							else
								samplestowrite=0; // done
						}
					}
					else // VCut
					{
						// block copy->write+seek
						ULONGLONG startblock=start/EX_BLOCKSIZE;
						ULONGLONG endblock=end/EX_BLOCKSIZE;

						LONGLONG block1=vcut.blockmem[startblock];

						// Seek Read File
						ULONGLONG offset=datastart+(block1*EX_BLOCKSIZE*samplesize_all_channels);

						if(block1!=0)
							startendsmooth=true;

						FILE_SeekBegin(offset);

						bool smoothdown=false;
						bool smoothup=false;

						while(samplestowrite>0 && (stop==false))
						{
							//Read sample block
							FILE_Read(rbuffer,EX_BLOCKSIZE*samplesize_all_channels);
							sampleswritten+=EX_BLOCKSIZE;

							// Calc Percent
							double h=(double)sampleswritten;
							double h2=(double)(end-start);
							h/=h2;
							h*=100;
							progress=h;

							samplestowrite-=EX_BLOCKSIZE;

							if(samplestowrite && startblock<endblock) // Next Block
							{
								ULONGLONG block2=vcut.blockmem[startblock+1];

								if(block2!=(block1+1)) // Seek over block/blocks ?
								{
									ULONGLONG seekoffset=block2-block1;

									seekoffset*=EX_BLOCKSIZE*samplesize_all_channels;

									FILE_SeekCurrent(seekoffset);

									smoothdown=true;
									smoothup=true;
								}

								block1=block2;
								startblock++;
							}

							// Smooth >< ?
							if(smoothdown==true)
							{
								smoothdown=false; // reset
								SmoothDown(rbuffer,EX_BLOCKSIZE*channels);
							}
							else
							{
								if(smoothup==true)
								{
									smoothup=false; // reset
									SmoothUp(rbuffer,EX_BLOCKSIZE*channels);
								}
							}

							// Write sample block
							FILE_Write(rbuffer,EX_BLOCKSIZE*samplesize_all_channels);

						}// while
					}

					if(stop==true)
					{
						FILE_CloseSave();
						FILE_DeleteFile(newfile); // delete new file

						progress=0;
						ok=false;
					}
					else
					{
						// RIFF CHUNKLEN
						writedatalen=sampleswritten*samplesize_all_channels;

						// Write Header Data
						char hbuff[4];
						int *uw=(int *)hbuff;

						*uw=(int)(writeheaderlen+writedatalen);

						FILE_SeekSaveBegin( 4);
						FILE_Write(hbuff,4);

						// data CHUNKLEN
						FILE_SeekSaveBegin(writeheaderlen-4);

						*uw=(int)writedatalen;
						FILE_Write(hbuff,4);

						// Smooth Save File, don't smooth Org File
						if(startendsmooth==true) // Cut
							DoSmooth_Save();

						FILE_CloseSave();

						progress=100; // ready
					}

					writeopen=false;
				}// open write
				else
					errorflag|=ERROR_UNABLETOOPEN_SAVE;

				FILE_Close();
			}
			else
				errorflag|=ERROR_UNABLETOOPEN_READ;

			delete rbuffer;

		}// if rbuffer
		else 
			errorflag|=ERROR_MEMORYALLOC;
	}

	return ok;
}

// Init WaveFile Class, input: FileName
// true/false return
bool WaveFile::Init(char *name)
{
	if(name && (init==false))
	{
		// Check For Wave Format
		if(!(filename=mainvar->GenerateString(name)))
		else
			errorflag|=ERROR_MEMORYALLOC;

		if((!errorflag) && FILE_OpenRead(filename)==true)
		{
			long clen; // chunklen
			char buffer[256];

			filelength=FILE_GetSize();

			FILE_Read(buffer,12);

			// WAVE Format ?
			if (buffer[0] == 'R' && buffer[1] == 'I' && buffer[2] == 'F' && buffer[3] == 'F')
			{
				unsigned short *us; // 16 bit us
				unsigned long *uw; //32 bit us

				if(buffer[8]=='W' && buffer[9]=='A' && buffer[10]=='V' && buffer[11]=='E')
				{
					uw=(int *)&buffer[4]; // Clen RIFF

					clen=*uw;

					FILE_Read(buffer,8);

					if(buffer[0]=='f' && buffer[1]=='m' && buffer[2]=='t')
					{
						uw=(int *)&buffer[4]; // Clen FMT
						clen=*uw;

						if(clen<255)
						{
							FILE_Read(buffer,clen);

							us=(unsigned short *)&buffer[0];
							//USHORT compcode=*us;

							us=(unsigned short *)&buffer[2];
							channels=*us;

							uw=(unsigned long *)&buffer[4];
							samplerate=*uw;

							us=(unsigned short *)&buffer[14];
							samplebits=*us;

							if(channels>0 &&
								channels<=MAXCHANNELS &&
								samplerate>0 &&
								(samplebits==16 || 
								// samplebits==20 || samplebits==24 || 
								samplebits==32)
								)
							{
								init=true; // ok Wave File found 

								datastart=0;

								do // Find data chunk
								{
									FILE_Read(buffer,8);

									uw=(int *)&buffer[4]; // Clen
									clen=*uw;

									if(buffer[0]=='d' &&
										buffer[1]=='a' &&
										buffer[2]=='t' &&
										buffer[3]=='a'
										)
									{
										datastart=FILE_GetCurrentPos();

										headerlen=(int)datastart;

										datalen=clen;
										dataend=datastart+datalen;

										if(dataend>filelength)
										{
											init=false; //error
											errorflag|=ERROR_CHUNKERROR;
										}
									}
									else // not a data chunk ?
									{
										ULONGLONG pos=FILE_GetCurrentPos();

										if((pos+clen)>filelength)
										{
											init=false; //error no data chunk found
											errorflag|=ERROR_CHUNKERROR;
										}
										else
											FILE_SeekCurrent(clen);// unknown chunk, jump to next				
									}

								}while((datastart==0) && (init==true));	
							}
							else
							{
								init=false; //error
								errorflag|=ERROR_UNKNOWNBITRATE; // or channels==0
							}
						}// clen<255
						else
							errorflag|=ERROR_CHUNKERROR;
					}
					else // fmt
						errorflag|=ERROR_UNKNOWNFORMAT;
				}
				else // wave
					errorflag|=ERROR_UNKNOWNFORMAT;
			}
			else //riff
				errorflag|=ERROR_UNKNOWNFORMAT;

			if(init==true)
			{
				// Calc Ticks+Samples
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
				}
			}

			FILE_Close();
		}
		else
			errorflag|=ERROR_UNABLETOOPEN_SAVE;
	}// if name

	return init;
}

bool WaveFile::CreatePeak()
{
	bool samplesfound=false;

	stop=false; // Reset Stop FLAG
	progress=0;
	maxpeakvalue=0; // reset max peak

	if(
		init==true &&
		initpeak==false &&
		samplesperchannel>0
		)
	{
		bool ok=true;

		unsigned short *to[MAXCHANNELS]; // tmp block writer

		// Init Peakbuffer
		peakbuffersamples=(samplesperchannel/EX_BLOCKSIZE)+1; // +1

		for(int i=0;i<channels;i++)
		{
			to[i]=peak[i]=new unsigned short[peakbuffersamples];

			if(!peak[i])
			{
				errorflag|=ERROR_MEMORYALLOC;
				ok=false;
				break;
			}
		}

		if(ok==true)
		{
			bool eof=false;

			int lastw=0;

			if(FILE_OpenRead(filename))
			{
				ULONGLONG samples=samplesperchannel;
				int count;
				int rbytes;
				bool ok=true;

				long *rbuffer=new long[EX_BLOCKSIZE*channels]; // source
				float *samplebuffer=new float[EX_BLOCKSIZE*channels]; // 24bit destination

				USHORT *peakbuffer[MAXCHANNELS];
				USHORT *writepeak[MAXCHANNELS];

				double allsamples;			

				if(ok==true)
				{
					for(int i=0;i<channels;i++)
					{
						writepeak[i]=peakbuffer[i]=new USHORT[EX_BLOCKSIZE];

						if(!peakbuffer[i])
						{
							errorflag|=ERROR_MEMORYALLOC;
							ok=false;
							break;
						}
					}
				}

				allsamples=(double)samples/EX_BLOCKSIZE;

				if(rbuffer && samplebuffer && (ok==true))
				{
					count=0;	

					FILE_SeekBegin(datastart);	// 1. Sample

					while(samples && (stop==false)) // bufferloop
					{	
						ULONGLONG size;

						if(samples>=EX_BLOCKSIZE)
						{
							rbytes=FILE_Read(rbuffer,EX_BLOCKSIZE*samplesize_all_channels);

							samples-=EX_BLOCKSIZE;
							size=EX_BLOCKSIZE;
						}
						else
						{
							// rest
							rbytes=FILE_Read(rbuffer,(int)(samples*samplesize_all_channels));

							size=samples;
							samples=0;

							eof=true;
						}

						// Progress
						double h=(double)samplesperchannel;
						double h2=(double)(samplesperchannel-samples);

						h2/=h;
						h2*=100;

						progress=h2;

						if(rbytes>0)
						{
							switch(samplebits)
							{
							case 16:
								{
									// Convert 16 bit to float
									short *fc=(short *)rbuffer;
									float *tb=samplebuffer;

									for(int i=0;i<(size*channels);i++)
									{
										float h=*fc++;

										h/=32768;

										*tb++ =h;
									}
								}
								break;

								// Convert 20 Bit to float

								// Convert 24 Bit to float

							case 32:
								// buffer exists in 32 bit float, no convert need
								break;
							}

							samplesfound=true;

							for(int i=0;i<channels;i++) // Generate Peak
							{
								float *check=samplebuffer,max=0;
								ULONGLONG i2=size;

								check+=i; // Channel Offset

								while(i2--)
								{
									float v=*check;

									if(v>maxpeakvalue)
										maxpeakvalue=v; 

									check+=channels; // channel offset

									if(v<0)
										v=0-v;

									if(v>max)
										max=v;
								}//while

								max*=65535; // Max -1<0>1 -> 65535

								if(max>65535)
									*writepeak[i]++=65535; // 16 bit
								else
									*writepeak[i]++=(USHORT)max; // 16 bit	
							}// for i

							count++;	
						}// if rbyte>0
						else
							samples=0; // eof

						if((count==EX_BLOCKSIZE) || (samples==0))
						{
							if(count)
							{
								// Copy Peak to peakfilebuffer channel buffer
								for(int c=0;c<channels;c++)	
								{
									USHORT *p=peakbuffer[c];

									for(int i=0;i<count;i++)
										*to[c]++=*p++;
								}
							}

							count=0;

							for(int i=0;i<channels;i++) // Reset
								writepeak[i]=peakbuffer[i];

						}// if count

					}// while samples

				} // if rbuffer

				// Clean Memory
				if(rbuffer)
					delete rbuffer;

				if(samplebuffer)
					delete samplebuffer;

				for(int i=0;i<channels;i++)
				{
					if(peakbuffer[i])
						delete peakbuffer[i];
				}

				FILE_Close();

			}// if file open&ok==true

			if(eof==false) // stopped or error
			{
				initpeak=false;
				peakbuffersamples=0;
				progress=0;

				for(int i=0;i<channels;i++)
				{
					if(peak[i])
					{
						delete peak[i];
						peak[i]=0;
					}
				}

			}
			else
			{
				// Reset Progress
				progress=100; // ready

				if(ok==true)
				{
					initpeak=true;

					vcut.Init();
				}
			}

		}// if peakfile open

		return ok;
	}// if init

	return false;
}

#ifdef TEST_EXWAVE

// Test Source
void TestWaveFile()
{
	WaveFile wave;

	// 1. Init File
	wave.Init("e:/test/chrome.wav");

	// 2. Create File Peak Array
	wave.CreatePeak();

	// 3. V Cut Test
	wave.VirtualCut(0,500000);
	wave.VirtualCut(0,1500000);

	// 4. Save Test
	wave.SaveAll("e:/test/testfull.wav"); // 100% Copy
	wave.SaveBlock("e:/test/testblock.wav",8000,500000);

	// 5. Read Test
	// Read File to float_buffer
	bool filereadok=wave.Read_Open();

	if(filereadok==true)
	{
		wave.Read_Seek(10000); // seek test

		while(filereadok==true)
		{
			filereadok=wave.Read_ReadSamplesToBuffer();

			ULONGLONG fileposition=wave.Read_GetSamplePosition();
			float *outputbuffer=wave.GetFloatBuffer();

			if(outputbuffer)
			{
				// float -> output to Soundcard or file
			}
		}

		wave.Read_Close();
	}

	// 5. Normalize Test
	// Normalize, Normalize to File -> e:/test/testblock.wave_norm.wav, + _norm.wav
	WaveFile norm;

	norm.Init("e:/test/testblock.wav");
	norm.CreatePeak();
	norm.Normalize();
}
#endif
*/
