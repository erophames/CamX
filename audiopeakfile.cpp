#include "defines.h"

#include "songmain.h"
#include "audiofile.h" // dummy
#include "object.h"
#include "audiohardware.h"
#include "audiothread.h"
#include "semapores.h"
#include "peakbuffer.h"
#include "gui.h"
#include "audiohdfile.h"
#include "settings.h"
#include "audiopeakfile.h"

void mainAudio::AddPeakBuffer(AudioPeakBuffer *apk)
{
	TRACE ("Add Peakbuffer %s\n",apk->peakfilename);

#ifdef _DEBUG
	if(FindPeakBuffer(apk->samplefilename))
		MessageBox(NULL,"Double Audio Peakbuffer !","Error",MB_OK);
#endif

	if(apk)
	{
		mainthreadcontrol->Lock(CS_peakbuffer);
		audiopeakbuffer.AddEndO(apk);
		mainthreadcontrol->Unlock(CS_peakbuffer);
	}
}

bool mainAudio::DeleteAudioFileAndPeakBuffer(char *file)
{
	if(file)
	{
		if(AudioPeakBuffer *apb=FindPeakBuffer(file))
		{
			mainthreadcontrol->Lock(CS_peakbuffer);
			if(apb->peakfilename)mainvar->DeleteAFile(apb->peakfilename);
			apb->FreePeakMemory(true);
			audiopeakbuffer.RemoveO(apb);
			mainthreadcontrol->Unlock(CS_peakbuffer);
		}

		// Delete Peak File
		if(char *peakfilename=new char[strlen(file)+16])
		{
			strcpy(peakfilename,file);	

			// Replace . = _
			char *r=peakfilename;
			size_t i=strlen(peakfilename);

			while(i--){
				if(*r=='.')*r='_';
				r++;
			}

			mainvar->AddString(peakfilename,PEAKFILENAME);
			mainvar->DeleteAFile(peakfilename);

			delete peakfilename;
		}

		return mainvar->DeleteAFile(file); // Delete Peak File from HD
	}

	return false;
}

AudioPeakBuffer *mainAudio::FindPeakBuffer(char *file)
{
	if(file){

		mainthreadcontrol->Lock(CS_peakbuffer);

		AudioPeakBuffer *c=FirstPeakBuffer();

		while(c){
			if(c->samplefilename && strcmp(c->samplefilename,file)==0){
				mainthreadcontrol->Unlock(CS_peakbuffer);
				return c;
			}

			c=c->NextAudioPeakBuffer();
		}

		mainthreadcontrol->Unlock(CS_peakbuffer);
	}

	return 0;
}

AudioPeakBuffer::AudioPeakBuffer()
{
	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)channelbuffer[i]=0;

	maxpeakfound=0;
	peakmixbuffer=0;
	channels=0;
	peaksamples=0;
	samplefilename=0;
	peakfilename=0;
	dontdeletepeakmixbuffer=false;

	lastsampleset=initok=false;

#ifdef _DEBUG		
	n[0]='A';
	n[1]='P';
	n[2]='E';
	n[3]='A';
#endif

	closed=false;
}


void AudioPeakBuffer::CopyPeakBuffer(AudioPeakBuffer *to)
{
	LockO();

	to->LockO();

	to->FreePeakMemory(false); // delete buffer only
	to->peakmixbuffer=peakmixbuffer;

	peakmixbuffer=0;

	to->maxpeakfound=maxpeakfound;
	to->channels=channels;
	to->peaksamples=peaksamples;

	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++){
		to->channelbuffer[i]=channelbuffer[i];
		channelbuffer[i]=0;
	}

	to->UnlockO();

	UnlockO();
}

void AudioPeakBuffer::CreatePeakMix()
{
	if(channels==1)
	{
		peakmixbuffer=channelbuffer[0]; // Mono=Mix
		dontdeletepeakmixbuffer=true;
		return;
	}

	if(peaksamples<=0)return;

	if(!peakmixbuffer)
		peakmixbuffer=new SHORT[2*(int)peaksamples];

	if(peakmixbuffer)
	{
		SHORT *check[MAXCHANNELSPERCHANNEL],*mixbuffer=peakmixbuffer;

		for(int chl=0;chl<channels;chl++)
			check[chl]=channelbuffer[chl];

		for(LONGLONG i2=0;i2<peaksamples;i2++)
		{
			SHORT mix_m=0,mix_p=0;

			// ---> Mix to buffer
			for(int chl=0;chl<channels;chl++)
			{
				if(*check[chl]>mix_p)mix_p=*check[chl]++;
				else
					check[chl]++;

				if(*check[chl]<mix_m)mix_m=*check[chl]++;
				else
					check[chl]++;
			}

			*mixbuffer++=mix_p;
			*mixbuffer++=mix_m;
		}
	}
}

bool AudioPeakBuffer::ReadPeakFile(AudioHDFile *af)
{
	bool ok=false;
	camxFile peakfile;

	LockO();
	FreePeakMemory(false); // Clear old Buffer ?
	UnlockO();

	if(peakfilename && samplefilename && peakfile.OpenRead(peakfilename)==true)
	{
		// Header
		LONGLONG header[4],samples2,/*fsample,lsample,*/blocksize;
		double maxpeak;
		char type[5];

		type[0]=0;

		peakfile.Read(header,4*sizeof(LONGLONG));
		peakfile.Read(&maxpeak,sizeof(double));
		//peakfile.Read(&fsample);
		//peakfile.Read(&lsample);

		peakfile.Read(type,5);

		channels=header[0];
		peaksamples=header[1];
		blocksize=header[2];
		samples2=header[3]; // backup samples

		LONGLONG soll=af->samplesperchannel;
		soll/=PEAKBUFFERBLOCKSIZE;

		if(soll*PEAKBUFFERBLOCKSIZE!=af->samplesperchannel)
			soll++;

		if(soll!=peaksamples)
		{
#ifdef DEBUG
			maingui->MessageBoxOk(0,"Peak File Size !=");
#endif

			goto close;
		}

		maxpeakfound=maxpeak;

		if(
			type[0]=='c' &&
			type[1]=='a' &&
			type[2]=='P' &&
			type[3]=='X' &&
			type[4]==PEAKFILEVERSION &&
			channels>0 &&
			channels<=MAXCHANNELSPERCHANNEL &&
			blocksize==PEAKBUFFERBLOCKSIZE &&
			peaksamples>0 &&
			peaksamples==samples2
			)
		{
			SHORT *readto[MAXCHANNELSPERCHANNEL];
			ok=true;

			for(int i=0;i<channels;i++){
				readto[i]=channelbuffer[i]=new SHORT[(int)2*peaksamples]; // 2* phase

				if(!channelbuffer[i]){
					ok=false;
				}
			}

			// Read file to Channelbuffer
			if(ok==true)
			{
				LONGLONG toread=header[1];

				while(toread && ok==true)
				{
					for(int i=0;i<channels;i++)
					{
						size_t read=toread<blocksize?(size_t)toread*sizeof(SHORT):(size_t)blocksize*sizeof(SHORT);
						size_t got=peakfile.Read(readto[i],2*read);// 2* phase

						if(2*read!=got)
						{
							ok=false;
							break;
						}

						readto[i]+=2*blocksize;
					}

					if(toread>blocksize)
						toread-=blocksize;
					else
						break;

				}// while read blocks

			}// if ok==true

		}//if type

	}//if open
	else
	{
#ifdef DEBUG
		maingui->MessageBoxError(0,"Unable to open Peak");
#endif

	}

close:
	peakfile.Close(true);

	if(ok==true)
		initok=true;
	else
		if(ok==false)
		{
			TRACE ("Error Peak File %s\n",peakfilename);

			char *h=mainvar->GenerateString(peakfilename);

			LockO();
			FreePeakMemory(false);
			UnlockO();

			if(h)
			{
				mainvar->DeleteAFile(peakfilename);
				delete h;
			}

		}

		return ok;
}

void AudioPeakBuffer::FreePeakMemory(bool full)
{
	initok=false;

	//delete buffer
	if(peakmixbuffer && dontdeletepeakmixbuffer==false)
		delete peakmixbuffer;

	dontdeletepeakmixbuffer=false;
	peakmixbuffer=0;

	for(int i=0;i<MAXCHANNELSPERCHANNEL;i++)
	{
		if(channelbuffer[i])delete channelbuffer[i];
		channelbuffer[i]=0;
	}

	if(full==true)
	{
		if(peakfilename)delete peakfilename;
		peakfilename=0;

		if(samplefilename)delete samplefilename;
		samplefilename=0;
	}
}

void mainAudio::ClosePeakFile(AudioHDFile *hd,AudioPeakBuffer *peakbuffer,bool forceclose,bool *deleted)
{
	if((!peakbuffer) || (!hd) || (!deleted))return;

	mainthreadcontrol->Lock(CS_peakbuffer);

	if(hd->nopeakfileasfile==true)
	{
		peakbuffer->LockO();
		peakbuffer->FreePeakMemory(true);
		peakbuffer->UnlockO();

		delete peakbuffer;
		*deleted=true;
		mainthreadcontrol->Unlock(CS_peakbuffer);
		return;
	}

	AudioPeakBuffer *c=FirstPeakBuffer();

	while(c)
	{
		if(c==peakbuffer)
		{
			AudioPeakBuffer *n=peakbuffer->NextAudioPeakBuffer();

			peakbuffer->LockO();
			peakbuffer->FreePeakMemory(true);
			peakbuffer->UnlockO();

			audiopeakbuffer.RemoveO(peakbuffer);

			*deleted=true;
			mainthreadcontrol->Unlock(CS_peakbuffer);
			return;
		}

		c=c->NextAudioPeakBuffer();
	}

	mainthreadcontrol->Unlock(CS_peakbuffer);

	*deleted=false;
}

AudioCreatePeakFile *AudioPeakFileThread::AddCreatePeakFile(AudioHDFile *af,AudioPeakBuffer *refreshbuffer)
{
	TRACE ("Add Create PeakFile %s\n",af->GetName());

	if(!af)return 0;
	if(!af->GetName())return 0;
	if(af->samplesperchannel==0)return 0;

	mainthreadcontrol->Lock(CS_createpeakfile);

	// Search list
	AudioCreatePeakFile *cpf=FirstCreatePeakFile();

	while(cpf)
	{
		if(strcmp(cpf->name,af->GetName())==0)
		{
			TRACE ("Create Peak in list\n");
			mainthreadcontrol->Unlock(CS_createpeakfile);
			return 0;
		}

		cpf=cpf->NextPeakFile();
	}

	if(cpf=new AudioCreatePeakFile)
	{
		if(cpf->name=mainvar->GenerateString(af->GetName()))
		{
			cpf->audiohdfile=af;
			cpf->refreshpeakbuffer=refreshbuffer;

			createpeakfiles.AddEndO(cpf);
			audiopeakthread->SetSignal();

			mainthreadcontrol->Unlock(CS_createpeakfile);
			return cpf;
		}

		delete cpf;
	}

	mainthreadcontrol->Unlock(CS_createpeakfile);
	return 0;
}

AudioCreatePeakFile *AudioPeakFileThread::DeleteCreatePeakFile(AudioCreatePeakFile *peakfile)
{
	if(peakfile)
	{
		if(peakfile->name)
		{
			delete peakfile->name;
			peakfile->name=0;
		}

		if(peakfile==runningcpf)
			runningcpf=0;

		return (AudioCreatePeakFile *)createpeakfiles.RemoveO(peakfile);
	}

	return 0;
}

AudioHDFile *AudioPeakFileThread::GetRunningFile()
{
	AudioHDFile *hd=0;

	mainthreadcontrol->Lock(CS_createpeakfile);

	if(runningcpf)
		hd=runningcpf->audiohdfile;

	mainthreadcontrol->Unlock(CS_createpeakfile);

	return hd;
}

bool AudioPeakFileThread::StopPeakFile(AudioHDFile *file)
{
	if(file)
	{
		return StopPeakFile(file->GetName());
	}

	return false;
}

bool AudioPeakFileThread::StopPeakFile(char *file)
{
	if(file)
	{
		mainthreadcontrol->Lock(CS_createpeakfile);

		// 1. Stop running Peak File
		if(runningcpf && runningcpf->CheckFile(file)==true)
		{
			runningcpf->stop=true;
			mainthreadcontrol->Unlock(CS_createpeakfile);

			// Wait for end of running cpf
			for(;;)
			{			
				Sleep(100);

				mainthreadcontrol->Lock(CS_createpeakfile);
				if((!runningcpf) || runningcpf->CheckFile(file)==false)
				{
					mainthreadcontrol->Unlock(CS_createpeakfile);
					return true;
				}

				mainthreadcontrol->Unlock(CS_createpeakfile);
			}

			return true;
		}

		// 2. Remove from List
		AudioCreatePeakFile *f=FirstCreatePeakFile();
		while(f)
		{
			if(f->CheckFile(file)==true)
			{
				//TRACE ("Stop Peak File %s\n",f->audiohdfile->GetName());
				DeleteCreatePeakFile(f);				
				//f->audiohdfile=0;
				mainthreadcontrol->Unlock(CS_createpeakfile);
				return true;
			}

			f=f->NextPeakFile();
		}

		mainthreadcontrol->Unlock(CS_createpeakfile);
	}

	return false;
}

bool AudioCreatePeakFile::CheckFile(char *file)
{
	if(file && audiohdfile && audiohdfile->GetName() && strcmp(audiohdfile->GetName(),file)==0)
		return true;

	return false;
}

bool AudioPeakFileThread::CreatePeakFile(guiWindow *win,AudioHDFile *af,char *savename,AudioCreatePeakFile *cpf)
{
	bool added=false,samplesfound=false;

	if(af && af->channels>0 && af->samplesperchannel>0 && af->destructive==false && savename)
	{
		camxFile writepeakfile; // Save file
		AudioPeakBuffer *peakexists=mainaudio->FindPeakBuffer(af->GetName());
		int writeflag=mainsettings->peakfiles;

		if(af->nopeakfileasfile==true)
			writeflag|=PEAKFILES_NOPEAKFILE;

		if(peakexists)
		{
			mainthreadcontrol->Lock(CS_audiopeakfilecheck);

			if(peakexists->peakfilename)
				mainvar->DeleteAFile(peakexists->peakfilename); // Delete old peakfile
		}

		if(AudioPeakBuffer *new_peakbuffer=new AudioPeakBuffer)
		{
			bool ok=true;
			SHORT *to[MAXCHANNELSPERCHANNEL];

			// Init Peakbuffer
			new_peakbuffer->channels=af->channels;
			new_peakbuffer->peaksamples=af->samplesperchannel/PEAKBUFFERBLOCKSIZE;

			if(new_peakbuffer->peaksamples*PEAKBUFFERBLOCKSIZE!=af->samplesperchannel) // +Rest ? - Add 1
				new_peakbuffer->peaksamples++;

			for(int i=0;i<af->channels;i++)
			{
				to[i]=new_peakbuffer->channelbuffer[i]=new SHORT[(int)2*new_peakbuffer->peaksamples]; // 2*phase

				if(!to[i])
				{
					//oom
					ok=false;
					break;
				}
			}

#ifdef DEMO
			writeflag|=PEAKFILES_NOPEAKFILE; // dont write PEAK FILE in demo version
#endif

			if(mainvar->exitthreads==true)
				ok=false;

			if( ok==true && 
				af->destructive==false &&
				((writeflag&PEAKFILES_NOPEAKFILE) || writepeakfile.OpenSave(savename)==true)
				)
			{
				camxFile file;		// Read file
				int lastw=0;

				LONGLONG header[4]; // Header 1
				double p_maxpeak=0;   // Header 2
				char type[5];	  // Header 3

				bool fsamplefound=false,lsamplefound=false;
				//LONGLONG fsample=0,lsample=0;

				// Init Progress
				mainthreadcontrol->Lock(CS_createpeakfile);
				createprogress=0;
				mainthreadcontrol->Unlock(CS_createpeakfile);

				// WriteHeader

				header[0]=af->channels; // number of channels
				header[1]=header[3]=0;	// number of xpixel-samples per channel
				header[2]=PEAKBUFFERBLOCKSIZE; // block size

				type[0]='c';
				type[1]='a';
				type[2]='P';
				type[3]='X';
				type[4]=PEAKFILEVERSION;

				if(af->destructive==false && cpf->stop==false && file.OpenRead(af->GetName())==true)
				{
					TRACE ("Read PeakFile %s\n",af->GetName());

					LONGLONG samples=af->samplesperchannel;
					double *rbuffer=new double[PEAKBUFFERBLOCKSIZE*af->channels]; // source

#ifdef _DEBUG
					ARES *samplebuffer=new ARES[PEAKBUFFERBLOCKSIZE*af->channels+1]; // 24bit destination
					if(samplebuffer)
						samplebuffer[PEAKBUFFERBLOCKSIZE*af->channels]=1.1f;
#else
					ARES *samplebuffer=new ARES[PEAKBUFFERBLOCKSIZE*af->channels]; // 24bit destination
#endif

					SHORT *peakbuffer[MAXCHANNELSPERCHANNEL],*writepeak[MAXCHANNELSPERCHANNEL];

					if(!rbuffer)
					{
						ok=false;
					}

					if(!samplebuffer)ok=false;

					if(ok==true)
					{
						for(int i=0;i<af->channels;i++)
						{
							writepeak[i]=peakbuffer[i]=new SHORT[2*PEAKBUFFERBLOCKSIZE]; // phase +/-

							if(!peakbuffer[i])
								ok=false;
						}
					}

					if(mainvar->exitthreads==true)
						ok=false;

					// double allsamples=(double)new_peakbuffer->blocksize;

					if(rbuffer && samplebuffer && ok==true && af->destructive==false)
					{
						AudioHardwareBuffer buffer;

						buffer.outputbufferARES=samplebuffer;

						buffer.SetBuffer(af->channels,PEAKBUFFERBLOCKSIZE);

						//buffer.channelsinbuffer=af->channels;
						//buffer.samplesinbuffer=PEAKBUFFERSIZE;

						int count=0;	

						file.SeekBegin(af->GetStartOfSample(0));

						// Read File Loop
						while(samples>0 && af->destructive==false && ok==true && cpf->stop==false && writepeakfile.errorwriting==false)
						{	
							//long bsize;
							int rbytes;

							if(mainvar->exitthreads==true)
								ok=false;

							if(samples>=PEAKBUFFERBLOCKSIZE)
							{
								rbytes=file.Read(rbuffer,PEAKBUFFERBLOCKSIZE*af->samplesize_all_channels);
								samples-=PEAKBUFFERBLOCKSIZE;
								//	bsize=PEAKBUFFERSIZE;
							}
							else
							{
								// rest
								rbytes=file.Read(rbuffer,(int)(samples*af->samplesize_all_channels));
								//bsize=(long)samples;
								samples=0;

								// clear buffer
								buffer.ClearOutput(af->channels);
							}

							// Reset BUffer
							buffer.channelsused=0;

							// Progress
							double h=(double)af->samplesperchannel,h2=(double)(af->samplesperchannel-samples);

							h2/=h;
							h2*=100;

							createprogress=h2;

							if(rbytes>0 && af->destructive==false)
							{
								af->Decoder((char *)rbuffer,PEAKBUFFERBLOCKSIZE);

								// Convert To ARES
								af->ConvertReadBufferToSamples (rbuffer,&buffer,PEAKBUFFERBLOCKSIZE,af->channels);						

								/*
								// First Sample
								if(fsamplefound==false)
								{
								for(int c=0;c<buffer.channelsinbuffer;c++)
								{
								ARES *ca=buffer.outputbufferARES;
								ca+=c*(af->samplerate/PEAKBUFFERDIV); // Channel Offset Block

								for(int i=0;i<(af->samplerate/PEAKBUFFERDIV);i++)
								{
								if(*ca!=0.0f)
								{
								fsamplefound=true;
								fsample+=i;
								break;
								}
								ca++;
								}

								if(fsamplefound==true)
								break;
								}
								}

								if(fsamplefound==false)
								fsample+=(af->samplerate/1000);

								// Last Sample
								for(int c=0;c<buffer.channelsinbuffer;c++)
								{
								ARES *ca=buffer.outputbufferARES;
								ca+=c*(af->samplerate/PEAKBUFFERDIV); // Channel Offset Block

								for(int i=0;i<(af->samplerate/PEAKBUFFERDIV);i++)
								{
								if(*ca!=0.0f)lsample+=i;
								ca++;
								}
								}
								*/

								// Write Header
								if(!(writeflag&PEAKFILES_NOPEAKFILE))
								{
									if(samplesfound==false && af->destructive==false && cpf->stop==false)
									{
										writepeakfile.Save(header,4*sizeof(LONGLONG)); // write header
										writepeakfile.Save(&p_maxpeak,sizeof(double));
										//writepeakfile.Save(&fsample,sizeof(LONGLONG));
										//writepeakfile.Save(&lsample,sizeof(LONGLONG));

										writepeakfile.Save(type,5);
									}
								}

								samplesfound=true;

								if(af->destructive==false)

									for(int chl=0;chl<af->channels;chl++) // Generate Peak
									{
										ARES *check=samplebuffer;

										check+=chl*PEAKBUFFERBLOCKSIZE; // Channel Offset Block

										ARES p_m=0,p_p=0;
										int i=PEAKBUFFERBLOCKSIZE;

										if(int loop=i/8)
										{
											i-=loop*8;
											do
											{
												ARES h=*check++;
												if(h>p_p)p_p=h; else if(h<p_m)p_m=h;
												h=*check++;
												if(h>p_p)p_p=h; else if(h<p_m)p_m=h;
												h=*check++;
												if(h>p_p)p_p=h; else if(h<p_m)p_m=h;
												h=*check++;
												if(h>p_p)p_p=h; else if(h<p_m)p_m=h;

												h=*check++;
												if(h>p_p)p_p=h; else if(h<p_m)p_m=h;
												h=*check++;
												if(h>p_p)p_p=h; else if(h<p_m)p_m=h;
												h=*check++;
												if(h>p_p)p_p=h; else if(h<p_m)p_m=h;
												h=*check++;
												if(h>p_p)p_p=h; else if(h<p_m)p_m=h;

											}while(--loop);
										}

										while(i--)
										{
											ARES h=*check++;
											if(h>p_p)p_p=h; else if(h<p_m)p_m=h;
										}

										{
											ARES peak=(-p_m>=p_p)?-p_m:p_p;
											if(peak>p_maxpeak)p_maxpeak=peak;
										}

										*writepeak[chl]++=(SHORT)(p_p*32767);
										*writepeak[chl]++=(SHORT)(p_m*32767);

									}// for i

									count++;	

							}// if rbyte>0
							else
								samples=0; // eof

							if(af->destructive==false && cpf->stop==false)
							{
								if(count==PEAKBUFFERBLOCKSIZE || samples==0)
								{
									if(count)
									{
										// Copy Peak to peakfilebuffer channel buffer
										for(int c=0;c<af->channels;c++)	
										{
											memcpy(to[c],peakbuffer[c],2*count*sizeof(SHORT)); // 2*=phase
											to[c]+=2*count;

											/*
											USHORT *p=peakbuffer[c];
											for(int i=0;i<count;i++)
											*to[c]++=*p++;
											*/
										}

										// Write Peak Block [Channel1][Channel2]... ?
										if(!(writeflag&PEAKFILES_NOPEAKFILE))
										{
											for(int i=0;i<af->channels;i++) // Write Channel Buffer Block
												writepeakfile.Save(peakbuffer[i],2*count*sizeof(SHORT));
										}
									}

									count=0;

									for(int i=0;i<af->channels;i++) // Reset
										writepeak[i]=peakbuffer[i];

								}// if count
							}

						}// while samples

						// Clean Memory
						if(rbuffer)delete rbuffer;
						if(samplebuffer)delete samplebuffer;

						for(int i=0;i<af->channels;i++)
							if(peakbuffer[i])delete peakbuffer[i];

						// Init New Peakbuffer
						new_peakbuffer->maxpeakfound=p_maxpeak;

						// Write Header
						if(ok==true && cpf->stop==false && af->destructive==false && samplesfound==true && (!(writeflag&PEAKFILES_NOPEAKFILE)))
						{
							writepeakfile.SeekBegin(0); // write number of samples

							if(ok==true)
								header[1]=header[3]=new_peakbuffer->peaksamples;								
							else
								header[1]=header[3]=0;

							writepeakfile.Save(header,4*sizeof(LONGLONG)); // 0+1
							writepeakfile.Save(&p_maxpeak,sizeof(double));
							//writepeakfile.Save(&fsample,sizeof(LONGLONG));
							//writepeakfile.Save(&lsample,sizeof(LONGLONG));
						}

						file.Close(true);

					}// if read file open&ok==true

					// Reset Progress
					mainthreadcontrol->Lock(CS_createpeakfile);
					createprogress=0;
					mainthreadcontrol->Unlock(CS_createpeakfile);

					if(writepeakfile.errorwriting==true)
					{
						ok=false;
					}

					writepeakfile.Close(true);

					if(!(writeflag&PEAKFILES_NOPEAKFILE))
					{
						if(ok==false || samplesfound==false || mainvar->exitthreads==true || af->destructive==true || cpf->stop==true)
							mainvar->DeleteAFile(savename);
					}

				}// if peakfile open

				if(ok==true && samplesfound==true && af->destructive==false && cpf->stop==false)
				{
					//new_peakbuffer->CreatePeakMix();

					// Ok
					if(peakexists)
					{
						new_peakbuffer->CopyPeakBuffer(peakexists);

						cpf->newpeakbuffer=peakexists;
					}
					else
					{
						new_peakbuffer->samplefilename=mainvar->GenerateString(af->GetName());

						if(!(writeflag&PEAKFILES_NOPEAKFILE))
							new_peakbuffer->peakfilename=mainvar->GenerateString(savename);
						else
							new_peakbuffer->peakfilename=0;

						if(new_peakbuffer->samplefilename)
						{
							new_peakbuffer->initok=true;

							if(af->nopeakfileasfile==true)
								af->peakbuffer=new_peakbuffer;
							else
								mainaudio->AddPeakBuffer(new_peakbuffer);

							cpf->newpeakbuffer=new_peakbuffer;

							added=true;
						}// if write header

					}// if rbuffer
				}// file open read

			}//if ok

			if(added==false)
			{
				new_peakbuffer->FreePeakMemory(true);
				delete new_peakbuffer;
				samplesfound=false;
				TRACE ("Peak File Creating Abort \n");
			}

		}// if new_peakbuffer

		if(peakexists)
		{
			mainthreadcontrol->Unlock(CS_audiopeakfilecheck);
		}

	}//if af

	return samplesfound;
}

// Creates Audiopeakfiles in Background
PTHREAD_START_ROUTINE AudioPeakFileThread::AudioPeakFileCreator(LPVOID pParam) 
{
	AudioPeakFileThread *thread=(AudioPeakFileThread *)pParam;

	while(thread->IsExit()==false) // Signal Loop
	{	
		thread->WaitSignal();

		if(thread->IsExit()==true) // Exit ?
			break;
		
		mainthreadcontrol->Lock(CS_createpeakfile);

		// Check Peakfile List
		AudioCreatePeakFile *cpf=thread->FirstCreatePeakFile();
		thread->runningcpf=cpf;
		mainthreadcontrol->Unlock(CS_createpeakfile);

		// cpf=0;
		while(cpf)
		{
			AudioHDFile *af;
			bool samplesfound=false;

			// Create Peak File
			if(af=cpf->audiohdfile)
				af->LockO();
			else
			{

				// Refresh Peak
				/*
				cpf->refreshpeakbuffer->LockPeakBuffer();
				cpf->refreshpeakbuffer->FreePeakMemory(false); // keep names
				cpf->refreshpeakbuffer->UnlockPeakBuffer();

				refresh.OpenAudioFileRead(cpf->refreshpeakbuffer->samplefilename,true,false);
				af=&refresh;
				*/
			}				

			if(af &&
				af->channels>0 && 
				af->channels<=MAXCHANNELSPERCHANNEL &&
				af->destructive==false && 
				af->m_ok==true && 
				af->samplesperchannel
				)
			{		
				size_t fstrlen=strlen(af->GetName());

				if(char *peakfilename=new char[fstrlen+strlen(PEAKFILENAME)+1])
				{
					strcpy(peakfilename,af->GetName());	

					// Replace . = _
					{
						char *r=peakfilename;
						size_t i=strlen(peakfilename);

						while(i--)
						{
							if(*r=='.')*r='_';
							r++;
						}
					}

					strcpy(&peakfilename[fstrlen],PEAKFILENAME);
					samplesfound=thread->CreatePeakFile(0,af,peakfilename,cpf);
					delete peakfilename;
				}

				if(cpf->stop==false && cpf->audiohdfile)
					af->UnlockO();
				else
				{
					//	refresh.CloseAudioFile(true);
				}

			}// if AudioPattern ok
			else
			{
				/*
				if(cpf->audiopattern)
				af->UnlockAudioPattern();
				*/
			}

			// Send Message
			AudioHDFile *sendfile=0;
			AudioPeakBuffer *sendpeakbuffer=0;

			if(cpf->stop==false)
			{
				if(cpf->audiohdfile)
				{
					if(cpf->newpeakbuffer && af->destructive==false && mainvar->exitthreads==false && samplesfound==true)
						sendfile=af;
				}
				else
				{
					if(mainvar->exitthreads==false && samplesfound==true)
						sendpeakbuffer=cpf->refreshpeakbuffer;
				}
			}

			// Next Peak
			mainthreadcontrol->Lock(CS_createpeakfile);
			thread->runningcpf=cpf=thread->DeleteCreatePeakFile(cpf); // get next
			mainthreadcontrol->Unlock(CS_createpeakfile);

			if(sendfile)
				maingui->SendGUIMessage(MESSAGE_REFRESHAUDIOHDFILE,sendfile);

			if(sendpeakbuffer)
				maingui->SendGUIMessage(MESSAGE_REFRESHPEAKBUFFER,sendpeakbuffer);

		} // while cpfs	

		/*
		if(win)
		win->CloseWindow();
		*/

	}// while

	// Delete All remaining Create Peak Files
	mainthreadcontrol->Lock(CS_createpeakfile);
	AudioCreatePeakFile *cpf=thread->FirstCreatePeakFile();
	while(cpf)cpf=thread->DeleteCreatePeakFile(cpf);
	mainthreadcontrol->Unlock(CS_createpeakfile);

	thread->ThreadGone();

	return 0;
}

int AudioPeakFileThread::StartThread()
{
	int error=0;

#ifdef WIN32
	ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)AudioPeakFileCreator,(LPVOID)this, 0,0);

	if(!ThreadHandle)error++;
#endif

	return error;
}

/*
void mainAudio::CheckPeakFiles()
{	
if((!(mainsettings->peakfiles&PEAKFILES_NOPEAKFILE)) && // Peak files on HD ?
audiopeakthread->CheckThreadRunning()==false
) // Peak Thread in use ?
{
mainthreadcontrol->Lock(CS_audiopeakfilecheck);

AudioHDFile *hdfile=FirstAudioHDFile();

camxFile orgfile;
camxFile peakfile;

while(hdfile)
{
AudioHDFile *nexthdfile=hdfile->NextHDFile();

if(hdfile->peakbuffer && hdfile->peakbuffer->peakopencount>0 && hdfile->peakbuffer->peakfilename)
{
if(orgfile.OpenRead(hdfile->GetName()))	
{
int c;

if(peakfile.OpenRead(hdfile->peakbuffer->peakfilename))
c=orgfile.CompareDate(&peakfile);
else
c=1; // peakfile deleted

if(c==1) 
{
//	MessageBeep(-1);

mainthreadcontrol->Lock(CS_createpeakfile);
audiopeakthread->AddCreatePeakFile(hdfile,0);
mainthreadcontrol->Unlock(CS_createpeakfile);
}
}
else
{
// no orgfile ??? Wave Deleted ?
// MessageBeep(-1);

hdfile->AudioFileDeleted();
}

orgfile.Close(true);
peakfile.Close(true);
}

hdfile=nexthdfile;
}

mainthreadcontrol->Unlock(CS_audiopeakfilecheck);
}
}
*/
