#include "defines.h"

#include <cmath>                    // for trigonometry functions

#include "audiohardwarechannel.h"
#include "audiohardware.h"
#include "audiochannel.h"
#include "audioeffects.h"

#include "object_track.h"
#include "audiosystem.h"
#include "MIDIthruproc.h"

#include "semapores.h"
#include "camxfile.h"

// Intern
#include "audioobject_equalizer.h"
#include "audioobject_delay.h"
#include "audioobject_allfilter.h"
#include "audioobject_chorus.h"
#include "audioobject_insertvolume.h"
#include "songmain.h"
//VST
#include "vstplugins.h"
#include "audiodevice.h"
#include "object_song.h"
#include "chunks.h"
#include "gui.h"
#include "audioobjects.h"
#include "MIDIinproc.h"
#include "audioproc.h"
#include "object_project.h"

#include "track_effects.h"
#include "editfunctions.h"
#include "undo_plugins.h"

#include "audiomaster.h"
#include "mastering.h"

void AudioEffects::FreeMemory()
{
#ifdef ARES64
	if(output32)
		delete output32;
	output32=0;
#endif

	DeleteAllEffects();
}

void AudioEffects::AO_InitIOChannels(int ichannels)
{
	InsertAudioEffect *iae=FirstInsertAudioEffect();
	while(iae){
		iae->audioeffect->AO_InitIOChannels(ichannels);
		iae=iae->NextEffect();
	}

	volume.AO_InitIOChannels(ichannels);
	pan.AO_InitIOChannels(ichannels);
}

void AudioEffects::Mute(bool m,bool lock)
{
	SetMute(m);

	if(m==true && lock==true){

		mainthreadcontrol->Lock(CS_audioplayback);
		mainthreadcontrol->Lock(CS_audiorealtime);
		mainaudiostreamproc->Lock();

		ClearBuffer();

		mainaudiostreamproc->Unlock();
		mainthreadcontrol->Unlock(CS_audiorealtime);
		mainthreadcontrol->Unlock(CS_audioplayback);;
	}
}

void AudioEffects::SendEndEffects(AudioEffectParameter *par) 
{
	int cused=par->in->channelsused;

	if(par->track)
	{
		// Freezing  Pre Volume/Pan !
		if(par->track->freezing)
		{
			if(par->playable==false)
				par->in->channelsused=0; // force 0 samples

			par->track->freezing->master->SaveBuffer(par->track->freezing,par->in,false);
		}
	}
	else
		if(par->channel)
		{
			// Freezing  Pre Volume/Pan !
			if(par->channel->freezing)
			{
				if(par->playable==false)
					par->in->channelsused=0; // force 0 samples

				par->channel->freezing->master->SaveBuffer(par->channel->freezing,par->in,false);
			}
		}

	par->in->channelsused=cused; // reset
	
	// Sends Pre
	if(par->itsmaster==false)
	{
		if(par->io->FirstSend() && par->playable==true)
		{
			par->io->DoAudioChannelSends(par,false); // Pre Send
			// Static Last Effect  *** Channel Volume *
			volume.DoEffect(par);
			par->io->DoAudioChannelSends(par,true); // Post Send
		}
		else
			volume.DoEffect(par);
	}
	else
		volume.DoEffect(par);

	if((par->track && par->track->GetAudioOut()->FirstOutputBus()==0) ||
		(par->channel && par->channel->audiochannelsystemtype==CHANNELTYPE_BUSCHANNEL)
		)
	{
		// No Pan for Master/Hardware...
		// No Pan for Tracks with -> Bus

		pan.DoEffect(par);
		par->panningdone=true;
	}
	else
	{
		if(par->channel && par->channel->audiochannelsystemtype==CHANNELTYPE_MASTER)
			par->panningdone=true;
		else
			par->panningdone=false; // Direct Bus, Add PanLaw to CreatePeak
	}
}

void AudioEffects::MixEffectDownToStream(AudioObject *fx,AudioEffectParameter *par)
{
	// 0/8 -> 0/2 etc...

	int outs=fx->GetSetOutputPins();

	switch(fx->floattype)
	{
	case AudioObject::FT_32BIT:
		{
		//	fx->aobuffer->MulCon32((float *)fx->aobuffer->outputbufferARES,outs*fx->setSize,fx->outvolume); // Add Output Volume

			float *to=(float *)fx->aobuffer->outputbufferARES,*from=to+par->streamchannels*fx->setSize;
			int add=0;

			for(int c=par->streamchannels;c<outs;c++){

				if(++add>=par->streamchannels){
					// reset loop
					to=(float *)fx->aobuffer->outputbufferARES; 
					add=0;
				}

				int i=fx->setSize;

				if(int loop=i/8)
				{
					i-=loop*8;

					do
					{
						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;

						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;

					}while(--loop);

				}

				while(i--)*to++ +=*from++; // Mix 3,4,5... -> 1,2 etc
			}
		}
		break;

	case AudioObject::FT_64BIT:
		{
		//	fx->aobuffer->MulCon(outs*fx->setSize,fx->outvolume); // Add Output Volume

			double *to=(double *)fx->aobuffer->outputbufferARES,*from=to+par->streamchannels*fx->setSize;
			int add=0;

			for(int c=par->streamchannels;c<outs;c++){
				if(++add>=par->streamchannels){
					// reset loop
					to=(double *)fx->aobuffer->outputbufferARES; 
					add=0;
				}

				int i=fx->setSize;

				if(int loop=i/8)
				{
					i-=loop*8;

					do
					{
						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;

						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;
						*to++ +=*from++;

					}while(--loop);

				}

				while(i--)*to++ +=*from++; // Mix 3,4,5... -> 1,2 etc
			}
		}
		break;
	}
}

void AudioEffects::ClearBuffer()
{
	InsertAudioEffect *f=FirstInsertAudioEffect();

	while(f){
		f->audioeffect->ClearBuffer();
		f=f->NextEffect();
	}
}

void AudioEffects::InitStart()
{
	InsertAudioEffect *f=FirstInsertAudioEffect();

	while(f){
		f->audioeffect->InitStart();
		f=f->NextEffect();
	}
}

void AudioEffects::SetFlags()
{
	bool fxs=false;
	bool ins=false;
	bool input=false;
	bool noinsfx=false;

	InsertAudioEffect *fx=FirstActiveAudioEffect();

	while(fx){

		if(fx->audioeffect->plugin_on==true)
		{
			fxs=true;

			if(fx->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT)
				ins=true;
			else
				noinsfx=true;

			if(fx->audioeffect->audioeffecttype==audioobject_TYPE_INSTRUMENT || fx->audioeffect->GetInputPins()>0)
				input=true;
		}

		fx=fx->NextActiveEffect();
	}

	
	hasinstruments=ins;
	hasnoninstrumentfx=noinsfx;

	hasfxwithinput=input;
	hasfxs=fxs;
}

AudioEffects::AudioEffects()
{
	id=OBJ_EFFECTLIST;

	SetMute(false);
	SetSolo(false);

#ifdef ARES64
	output32=0;
#endif

	channel=0;
	track=0;
	io=0;
	instrumentspossible=inputeffects=hasnoninstrumentfx=hasfxwithinput=hasinstruments=false;

	volume.audioeffects=pan.audioeffects=this;
	fxflag=0;
}

InsertAudioEffect *AudioEffects::AddInsertAudioEffect(InsertAudioEffect *next,AudioObject *ao,bool toclip,bool clone)
{
	if(!ao)return 0;

	AudioObject *plug;

	if(clone==true)
		plug=ao->CloneEffect(toclip==true?0:AudioObject::CREATENEW_PLUGIN,GetSong()); // Create Clone
	else
		plug=ao->InitOpenEffect();

	if(!plug)
		return 0;

	if(InsertAudioEffect *iae=new InsertAudioEffect(this))
	{
		plug->song=GetSong();
		plug->audioeffects=this;

		if(clone==true){
			plug->plugin_bypass=ao->plugin_bypass;
			plug->plugin_on=ao->plugin_on;
			plug->audioeffecttype=ao->audioeffecttype;
		}

		iae->audioeffect=plug;
		plug->inserteffect=iae;

		if(toclip==true){

			effects.AddNextO(iae,next);
			effects.Close();

			SetFlags();

			return iae;
		}

		if(plug->song)
			plug->AO_InitSampleRateAndSize(plug->song->project->projectsamplerate,plug->song->audiosystem.device?plug->song->audiosystem.device->GetSetSize():DEFAULTAUDIOBLOCKSIZE);	

		plug->AO_InitIOChannels(io->GetChannels());

		bool activesong;
		if(plug->song && plug->song==mainvar->GetActiveSong())
		{
			activesong=true;
			plug->CreateAudioObjectBuffer(); // Effect IO Buffer
		}
		else
			activesong=false;

		if(iae && iae->audioeffect)
		iae->audioeffect->sep_outs.SO_Init();

		effects.AddNextO(iae,next);
		effects.Close();

		SetFlags();

		if(activesong==true)
		{
			if(inputeffects==true)
			{
				plug->song->audiosystem.SetAudioSystemHasFlag();
				plug->song->AudioInputNeeded();
			}
		}

		if(track)
		{
			if(mainsettings->autotrackMIDIinthru==true)
			{
				if(track->GetFX()->usealwaysthru==false)
				{
					track->GetFX()->usealwaysthru=true;
					track->GetFX()->setalwaysthruautomatic=true;
				}
			}
		}

		return iae;
	}

	return 0;
}

void AudioEffects::SetTrackName(AudioObject *ao)
{
	if(ao->settrackname==false)
	{	
		InsertAudioEffect *iae=FirstInsertAudioEffect();

		while(iae){

			iae->audioeffect->settrackname=false;
			iae=iae->NextEffect();
		}

		ao->settrackname=true;
	}
	else
		ao->settrackname=false;
}


InsertAudioEffect *AudioEffects::FindAudioObject(AudioObject *ao)
{
	if(!ao)return 0;

	InsertAudioEffect *iae=FirstInsertAudioEffect();

	while(iae)
	{
		if(iae->audioeffect==ao)
			return iae;

		iae=iae->NextActiveEffect();
	}

	return 0;
}

InsertAudioEffect *AudioEffects::DeleteInsertAudioEffect(Seq_Song *song,InsertAudioEffect *iae,bool activesong,bool full)
{
	if(!iae)
		return 0;

	if(!song)
		song=GetSong();

	if(!song)
		return 0;

	for(int i=0;i<REALTIME_LISTS;i++)
	{
		song->realtimeevents[i].RemoveRObject(iae->audioeffect);
	}

	// Remove Plugin from Plugin Thru/Input Wait Event List
	plugininproc->SendForce(song,iae->audioeffect);

	//InsertAudioEffect *n=(InsertAudioEffect *)iae->GetList()->CutObject(iae); // Cut no more trigger etc..

	InsertAudioEffect *n=(InsertAudioEffect *)effects.CutObject(iae); // Cut no more trigger etc..

	SetFlags();
	//CheckBridge();

	if(activesong==true)
	{
		if(inputeffects==true)
		{
			song->audiosystem.SetAudioSystemHasFlag();
			song->AudioInputNeeded();
		}

		//	mainthreadcontrol->UnlockActiveSong();
	}

	// Close+Delete Effect

	if(iae->bufferobject)
	{
		// Undo Buffer
		iae->bufferobject->Close(full);
		iae->bufferobject->DeleteBuffer();

		delete iae->bufferobject;
		iae->bufferobject=0;
	}

	if(iae->audioeffect)
	{
		// Object removed from List no lock required
		mainMIDIthruthread->RemoveOffsFromEffect(iae); // Close Open Offs + send

		//iae->audioeffect->SendAllOpenTriggerEvents();
		iae->audioeffect->Close(full); // close vst plugins etc....
		iae->audioeffect->DeleteBuffer();

		if(full==true)
		{
			if(iae->audioeffect)
			{
				iae->audioeffect->sep_outs.SO_DeInit();

				delete iae->audioeffect;
				iae->audioeffect=0;
			}
		}
	}

	// Close Insert Effect
	delete iae;

	song->audiosystem.SetAudioSystemHasFlag();

	return n;
}

#ifdef OLDIE
void AudioEffects::Repair(AudioDevice *device)
{
	int size=device?device->GetSetSize():DEFAULTAUDIOBLOCKSIZE;

#ifdef ARES64
	// Init Bridge

	if(output32)
		delete output32;

	output32=new float[size*MAXCHANNELSPERCHANNEL];
#endif
}
#endif

void AudioEffects::RefreshBuffer(AudioDevice *device)
{
	int size=device?device->GetSetSize():DEFAULTAUDIOBLOCKSIZE;


#ifdef ARES64
	// Init Bridge

	if(output32)
		delete output32;

	output32=new float[size*MAXCHANNELSPERCHANNEL];
#endif


	InsertAudioEffect *iae=FirstInsertAudioEffect();
	while(iae)
	{
		iae->audioeffect->AO_InitSampleRateAndSize(mainaudio->GetGlobalSampleRate(),size);
		iae->audioeffect->CreateAudioObjectBuffer();
		iae->audioeffect->AO_InitIOChannels(io->GetChannels());

		iae=iae->NextEffect();
	}

	volume.AO_InitIOChannels(io->GetChannels());
	pan.AO_InitIOChannels(io->GetChannels());
}

void AudioEffects::RefreshDo()
{
	// Close Plugins/Buffer Data
	InsertAudioEffect *iae=FirstInsertAudioEffect();
	while(iae)
	{
		if(iae->audioeffect)
		{
			iae->bufferobject=(AudioObject *)iae->audioeffect->Clone();

			iae->audioeffect->Close(true); // close vst plugins etc....
			iae->audioeffect->DeleteBuffer();

			delete iae->audioeffect;
			iae->audioeffect=0;
		}

		iae=iae->NextEffect();
	}
}

void AudioEffects::PreRefreshDo()
{
	// Close Plugins/Buffer Data
	InsertAudioEffect *iae=FirstInsertAudioEffect();
	while(iae)
	{
		if(iae->bufferobject)
		{
			iae->audioeffect=iae->bufferobject->CloneEffect(AudioObject::CREATENEW_PLUGIN,GetSong());

			if(iae->audioeffect)
			{
				iae->audioeffect->audioeffects=this;
			}

			iae->bufferobject->Delete(true);
			iae->bufferobject=0;
		}

		iae=iae->NextEffect();
	}

	if(GetSong())
		RefreshBuffer(GetSong()->audiosystem.device);
}

void AudioEffects::DeleteAllEffects()
{
	InsertAudioEffect *iae=FirstInsertAudioEffect();

	while(iae)
		iae=DeleteInsertAudioEffect(0,iae,false);

	SetFlags();
	//	volume.DeleteDefaultParms();
	//	pan.DeleteDefaultParms();
}

void AudioEffects::Delete(bool full)
{
	FreeMemory();
	delete this;
}

void AudioEffects::ResetPlugins()
{
	InsertAudioEffect *iae=FirstInsertAudioEffect();

	while(iae){
		
		if(iae->audioeffect->IsActive()==true)
		{
			iae->audioeffect->PlugInOff();
			iae->audioeffect->PlugInOn();
		}

		iae=iae->NextEffect();
	}
}

void AudioEffects::DeleteAllEffectBuffers()
{
#ifdef ARES64
	if(output32)
		delete output32;
	output32=0;
#endif

	InsertAudioEffect *iae=FirstInsertAudioEffect();

	while(iae){
		if(iae->audioeffect->aobuffer){
			iae->audioeffect->aobuffer->Delete32BitBuffer();
			delete iae->audioeffect->aobuffer;
			iae->audioeffect->aobuffer=0;
		}

		iae=iae->NextEffect();
	}
}

void AudioObject::Bypass(bool on)
{
	Seq_Song *song=0;

	if(audioeffects->channel)
		song=audioeffects->channel->audiosystem->song;
	else
		if(audioeffects->track)
			song=audioeffects->track->song;

	if(on==true){

		if(song && song==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		plugin_bypass=true;

		//PlugInOn();

		if(song && song==mainvar->GetActiveSong())
			mainthreadcontrol->UnlockActiveSong();

		// Remove Thru Offs
		//if(inserteffect){
		//	mainMIDIthruthread->RemoveOffsFromEffect(inserteffect);
		//SendAllOpenTriggerEvents();
		//}

		//ClearBuffer();
	}
	else{
		if(song && song==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		plugin_bypass=false;

		//	PlugInOff();

		if(song && song==mainvar->GetActiveSong())
			mainthreadcontrol->UnlockActiveSong();
	}
}

void AudioObject::OnOff(bool on)
{
	Seq_Song *song=0;

	if(audioeffects->channel)
		song=audioeffects->channel->audiosystem->song;
	else
		if(audioeffects->track)
			song=audioeffects->track->song;

	if(on==true){

		if(song && song==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		plugin_on=true;

		PlugInOn();

		audioeffects->SetFlags();

		if(song && song==mainvar->GetActiveSong())
			mainthreadcontrol->UnlockActiveSong();

		// Remove Thru Offs
		if(inserteffect){
			mainMIDIthruthread->RemoveOffsFromEffect(inserteffect);
			//SendAllOpenTriggerEvents();
		}

		ClearBuffer();
	}
	else{
		if(song && song==mainvar->GetActiveSong())
			mainthreadcontrol->LockActiveSong();

		plugin_on=false;

		PlugInOff();

		audioeffects->SetFlags();

		if(song && song==mainvar->GetActiveSong())
			mainthreadcontrol->UnlockActiveSong();
	}
}

void AudioObject::ValueChanged(OSTART time,int index,float value)
{
	LockAutomation();

	automationeditparameter.time[automationeditparameter.automationeditparameter_counter]=time;
	automationeditparameter.index[automationeditparameter.automationeditparameter_counter]=index;
	automationeditparameter.value[automationeditparameter.automationeditparameter_counter]=value;

	if(automationeditparameter.automationeditparameter_counter==MAXPLUGINAUTOMATIONPARAMETER-1)
		automationeditparameter.automationeditparameter_counter=0;
	else
		automationeditparameter.automationeditparameter_counter++;

	UnlockAutomation();
}

void AudioObject::AddIOMenu(guiMenu *menu)
{
	if(!menu)return;

	guiMenu *exc=menu->AddMenu("Remote",0);
	guiMenu *sync=menu->AddMenu("Sync",0);
	guiMenu *n=menu->AddMenu("PlugIn I/O",0);
}

AudioObject::AudioObject()
{
	setSampleRate=setSize=iochannels=0;
	setins=setouts=ins=outs=0;

	updatedisplay=false;
	timeusage=0;
	involume=outvolume=0.5;

	crashed=0;
	crashmessage=false;
	fromobject=0;
	init=false;

	inserteffect=0;
	aobuffer=0;
	internid=0;
	numberofprograms=0;
	numberofparameter=0; // Number of parameters
	audioeffecttype=audioobject_TYPE_EFFECT;
	triggerevents=sysextriggerevents=0;
	filesize=0;
	monitor_syscounter=monitor_eventcounter=0; // no output
	iovolume=true;
	intern_flag=0;
	mixlaw=MIXLAW_6db;
	iosetcorrect=false;

	settrackname=false;

	plugin_on=true;
	plugin_bypass=false;
	plugin_active=true;

	chunkdata_buffer=0;
	chunksize_buffer=0;

	directory=0;
	song=0;
}

void AudioObject::CopySettings(AudioObject *n)
{
	n->intern_flag=intern_flag;
	n->numberofparameter=numberofparameter;
	n->numberofprograms=numberofprograms;
	n->audioeffecttype=audioeffecttype;
	n->ins=ins;
	n->outs=outs;
	n->filesize=filesize;

	n->iovolume=iovolume;
	n->plugin_on=plugin_on;
	n->plugin_bypass=plugin_bypass;
	n->involume=involume;
	n->outvolume=outvolume;
}

void AudioEffects::CopyTo(AudioEffects *to,int iflag)
{
	if(!to)return;

	/*
	if((flag&COPYTO_INSTRUMENTS) && to->instrumentspossible==true){
	// Instruments
	InsertAudioEffect *iae=FirstActiveAudioInstrument();

	while(iae){
	to->AddInsertAudioEffect(0,iae->audioeffect); // Create Plugin Clone 
	iae=iae->NextInstrument();
	}
	}
	*/

	if(iflag&COPYTO_EFFECTS){
		// Effect
		InsertAudioEffect *iae=FirstInsertAudioEffect();

		while(iae){
			to->AddInsertAudioEffect(0,iae->audioeffect,(iflag&COPYTO_PASTE)?false:true); // Create Plugin Clone
			iae=iae->NextEffect();
		}
	}
}

Object *AudioEffects::Clone()
{
	if(AudioEffects *ae=new AudioEffects){

		ae->instrumentspossible=true;
		CopyTo(ae,COPYTO_EFFECTS/*|COPYTO_INSTRUMENTS*/);
		return ae;
	}

	return 0;
}

Seq_Song *AudioEffects::GetSong()
{
	if(track)
		return track->song;

	if(channel)
		return channel->audiosystem->song;

	return 0;
}

void AudioEffects::AddEffectEnd(InsertAudioEffect *iae)
{
	if(!iae)
		return;

	if(!iae->audioeffect)
		return;

	// init Sepereate Outs
	iae->audioeffect->sep_outs.SO_Init();

	effects.AddEndO(iae);
}

void AudioEffects::Load(camxFile *file)
{
	char *nameofplugin=0;

	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUDIOCHANNELEFFECTS)
	{
		file->ChunkFound();

		int nr=0;
		file->ReadChunk(&nr);
		file->CloseReadChunk();

		while(nr--)
		{
			file->LoadChunk();

			if(file->GetChunkHeader()==CHUNK_AUDIOCHANNELEFFECT)
			{
				file->ChunkFound();

				InsertAudioEffect *iae=new InsertAudioEffect(this);

				if(iae)
				{
					file->ReadAndAddClass((CPOINTER)iae);
					file->ReadAndAddClass((CPOINTER)&iae->audioeffect);

					iae->Load(GetSong(),file);

					// Load Parms
					int l_numberofparameter=0;
					file->ReadChunk(&l_numberofparameter);

					// l_numberofparameter=0 maybe Bank Later

					if(l_numberofparameter && iae->audioeffect && iae->audioeffect->numberofparameter==l_numberofparameter)
					{
						if(iae->loadparameters=new ARES[l_numberofparameter])
						{
							for(int i=0;i<l_numberofparameter;i++)
							{
								iae->loadparameters[i]=1;
								file->ReadChunk(&iae->loadparameters[i]);
							}
						}
						else
							for(int i=0;i<l_numberofparameter;i++)
							{
								ARES r;
								file->ReadChunk(&r);
							}
					}
					else
					{
						for(int i=0;i<l_numberofparameter;i++)
						{
							ARES r;
							file->ReadChunk(&r);
						}
					}

					// IO Volume
					if(iae->audioeffect)
					{
						file->ReadChunk(&iae->audioeffect->involume);
						file->ReadChunk(&iae->audioeffect->outvolume);
						file->ReadChunk(&iae->audioeffect->mixlaw);
						file->ReadChunk(&iae->audioeffect->settrackname);
						file->ReadChunk(&iae->audioeffect->plugin_bypass);

						iae->audioeffect->audioeffects=this;
						iae->audioeffect->inserteffect=iae;

						//	iae->audioeffect->AO_InitIOChannels(io->GetChannels());

						if(iae->read_dllname)delete iae->read_dllname;
						if(iae->read_fullname)delete iae->read_fullname;


						AddEffectEnd(iae);
					}
					else{

						// Unable to Load Plugin !
						ARES involume,outvolume;

						// Skip IO Volume
						file->ReadChunk(&involume);
						file->ReadChunk(&outvolume);

						int mixlaw;
						file->ReadChunk(&mixlaw);

						bool bypass;
						file->ReadChunk(&bypass);

						if(GetSong() && iae->read_dllname)
						{
							if(c_Pluginnotfound *cpn=new c_Pluginnotfound)
							{
								cpn->plugin=mainvar->GenerateString(iae->read_dllname);
								GetSong()->unabletoreadplugins.AddEndO(cpn);
							}
						}

						file->DeleteAddedClass((CPOINTER)&iae->audioeffect);

						if(iae->read_dllname)
						{
							delete iae->read_dllname;
							iae->read_dllname=0;
						}

						if(iae->read_fullname)
						{
							delete iae->read_fullname;
							iae->read_fullname=0;
						}

						delete iae;
						iae=0;
					}
				}

				file->CloseReadChunk();

				file->LoadChunk();
				if(file->GetChunkHeader()==CHUNK_AUDIOCHANNELEFFECTDATACHUNK)
				{
					file->ChunkFound();
					if(iae)
					{
						iae->audioeffect->LoadChunkData(file);
					}
					file->CloseReadChunk();
				}

				file->LoadChunk();
				if(iae && iae->audioeffect)
				{
					iae->audioeffect->sep_outs.SO_Load(file);
				}

				if(iae)
				{
				file->LoadChunk();
				iae->MIDIfilter.Load(file);
				}
			}

			SetFlags();

		}//while

		file->LoadChunk();
	}

	// Channel Static Misc
	file->LoadChunk();	
	if(file->GetChunkHeader()==CHUNK_AUDIOSTATIC_MISCAUTOMATION)
	{
		file->ChunkFound();
		file->CloseReadChunk();

		mute.Load(file);
		solo.Load(file);

		file->LoadChunk();
	}
#ifdef _DEBUG
	else
		MessageBox(NULL,"Audio Static Misc not found","Error",MB_OK);
#endif

	// Volume
	file->LoadChunk();

	if(file->GetChunkHeader()==CHUNK_AUDIOSTATIC_VOLUME)
	{
		file->ChunkFound();
		file->CloseReadChunk();

		volume.Load(file);
		file->LoadChunk();
	}
#ifdef _DEBUG
	else
		MessageBox(NULL,"Audio Static Volume not found","Error",MB_OK);
#endif

	// Panorama
	file->LoadChunk();	
	if(file->GetChunkHeader()==CHUNK_AUDIOSTATIC_PAN)
	{
		file->ChunkFound();
		file->CloseReadChunk();

		pan.Load(file);
		file->LoadChunk();
	}
#ifdef _DEBUG
	else
		MessageBox(NULL,"Audio Static Pan not found","Error",MB_OK);
#endif

	//CheckBridge();
}

void AudioEffects::SetLoadParms()
{
	InsertAudioEffect *f=FirstInsertAudioEffect();

	while(f)
	{
		if(f->loadparameters)
		{
			if(f->audioeffect)
			{
				for(int i=0;i<f->audioeffect->numberofparameter;i++)
				{
					f->audioeffect->SetParm(i,f->loadparameters[i]);
				}
			}

			delete f->loadparameters;
			f->loadparameters=0;
		}

		f=f->NextEffect();
	}
}

InsertAudioEffect *InsertAudioEffect::NextActiveEffect()
{
	InsertAudioEffect *n=(InsertAudioEffect *)next;

	while(n){
		if(n->audioeffect->plugin_on==true)return n;
		n=n->NextEffect();
	}

	return 0;
}


void AudioEffects::Save(camxFile *file)
{
	if(FirstInsertAudioEffect()) // Effects  ...
	{
		file->OpenChunk(CHUNK_AUDIOCHANNELEFFECTS);

		int nr=GetCountEffects();
		file->Save_Chunk(nr);
		file->CloseChunk();

		InsertAudioEffect *f=FirstInsertAudioEffect();

		while(f)
		{
			file->OpenChunk(CHUNK_AUDIOCHANNELEFFECT);

			f->Save(file); // + Default Parms

			// IO Volume
			file->Save_Chunk(f->audioeffect->involume);
			file->Save_Chunk(f->audioeffect->outvolume);
			file->Save_Chunk(f->audioeffect->mixlaw);
			file->Save_Chunk(f->audioeffect->settrackname);
			file->Save_Chunk(f->audioeffect->plugin_bypass);

			file->CloseChunk();

			if(f->audioeffect->CanChunk()==true)
			{
				file->OpenChunk(CHUNK_AUDIOCHANNELEFFECTDATACHUNK);
				f->audioeffect->SaveChunkData(file);
				file->CloseChunk();
			}

			f->audioeffect->sep_outs.SO_Save(file);

			f->MIDIfilter.Save(file);

			f=f->NextEffect();
		}
	}

	// Misc Effects
	file->OpenChunk(CHUNK_AUDIOSTATIC_MISCAUTOMATION);
	file->CloseChunk();

	mute.Save(file);
	solo.Save(file);

	// Static Effects

	file->OpenChunk(CHUNK_AUDIOSTATIC_VOLUME);
	file->CloseChunk();

	volume.Save(file);

	file->OpenChunk(CHUNK_AUDIOSTATIC_PAN);
	file->CloseChunk();
	pan.Save(file);
}

void InsertAudioEffect::Load(Seq_Song *song,camxFile *file)
{
	int oid=0,internid=0;

	bool effecton=true;

	file->ReadChunk(&oid); // INTERN/VST etc..
	file->ReadChunk(&internid); // INTERN or NULL(VST)
	file->ReadChunk(&effecton);

	read_dllname=read_fullname=0;

	switch(oid)
	{
	case OBJ_AUDIOINTERN:
		{
			switch(internid)
			{
			case INTERNAUDIO_ID_EQ3:
				audioeffect=new audioobject_Intern_Equalizer3();
				break;

			case INTERNAUDIO_ID_EQ3_Stereo:
				//audioeffect=new audioobject_Intern_Equalizer3_Stereo();
				break;

			case INTERNAUDIO_ID_EQ5:
				audioeffect=new audioobject_Intern_Equalizer5();
				break;

			case INTERNAUDIO_ID_EQ5_Stereo:
				//audioeffect=new audioobject_Intern_Equalizer5_Stereo();
				break;

			case INTERNAUDIO_ID_EQ7:
				audioeffect=new audioobject_Intern_Equalizer7();
				break;

			case INTERNAUDIO_ID_EQ7_Stereo:
				//audioeffect=new audioobject_Intern_Equalizer7_Stereo();
				break;

			case INTERNAUDIO_ID_EQ10:
				audioeffect=new audioobject_Intern_Equalizer10();
				break;

			case INTERNAUDIO_ID_EQ10_Stereo:
				//audioeffect=new audioobject_Intern_Equalizer10_Stereo();
				break;

			case INTERNAUDIO_ID_DELAY:
				audioeffect=new audioobject_Intern_Delay();
				break;

			case INTERNAUDIO_ID_DELAY_Stereo:
				//audioeffect=new audioobject_Intern_Delay_Stereo();
				break;

			case INTERNAUDIO_ID_ALLPASS:
				audioeffect=new audioobject_Intern_Allpass();
				break;

			case INTERNAUDIO_ID_ALLPASS_Stereo:
				//audioeffect=new audioobject_Intern_Allpass_Stereo();
				break;

			case INTERNAUDIO_ID_CHORUS:
				audioeffect=new audioobject_Intern_Chorus();
				break;

			case INTERNAUDIO_ID_CHORUS_Stereo:
				//audioeffect=new audioobject_Intern_Chorus_Stereo();
				break;

			case  INTERNAUDIO_ID_VOLUME:
				audioeffect=new audioobject_Intern_IVolume();
				break;

			case INTERNAUDIO_ID_VOLUME_Stereo:
				//audioeffect=new audioobject_Intern_IVolume_Stereo();
				break;
			}
		}
		break;

	case OBJ_AUDIOVST:
		{
			file->Read_ChunkString(&read_dllname);
			file->Read_ChunkString(&read_fullname);

			LONGLONG dllsize=0;
			file->ReadChunk(&dllsize);

			TRACE ("Load Audio VST %s %s \n",read_dllname,read_fullname);

			VSTPlugin *found=mainaudio->FindVSTEffect(read_dllname);

			if(!found)
			{
				// Not In list
				if(mainaudio->TestVST(read_fullname,read_dllname,dllsize,0)==true)
					found=mainaudio->FindVSTEffect(read_dllname);
			}

			if(found)
			{
				audioeffect=found->CloneEffect(AudioObject::CREATENEW_PLUGIN,song);
			}
		}
		break;

#ifdef _DEBUG
	default:
		MessageBox(NULL,"No Audio Effect ID found","Error",MB_OK);
#endif
	}

	if(audioeffect)
	{
		audioeffect->plugin_on=effecton;
		audioeffect->song=song;

		//audioeffect->CreateDefaultParms();
		audioeffect->Load(file);
	}
#ifdef _DEBUG
	else
		MessageBox(NULL,"Unable to load Audio Effect/Instrument","Error",MB_OK);
#endif
}

void InsertAudioEffect::Save(camxFile *file)
{
	file->Save_Chunk((CPOINTER)this);
	file->Save_Chunk((CPOINTER)audioeffect);

	file->Save_Chunk(audioeffect->id); // INTERN/VST etc..
	file->Save_Chunk(audioeffect->internid); // INTERN or NULL
	file->Save_Chunk(audioeffect->plugin_on);

	audioeffect->Save(file);

	// Parameter...

	if(audioeffect->crashed==0 && audioeffect->CanChunk()==false)
	{
		// Plugin OK no Chunk... Save Single Values
		file->Save_Chunk(audioeffect->numberofparameter);

		for(int i=0;i<audioeffect->numberofparameter;i++)
		{
			double fh=audioeffect->GetParm(i); 
			file->Save_Chunk(fh);
			//	TRACE ("Save VST Par %d %f\n",i,fh);
		}
	}
	else
	{
		int numberofparameter=0; // Crashed or Bank later
		file->Save_Chunk(numberofparameter);
	}
}

Undo_DeletePlugin::Undo_DeletePlugin(Seq_Song *s,InsertAudioEffect *iae)
{
	id=Undo::UID_DELETEPLUGIN;

	song=s;
	effects=iae->effectlist;
	addnext=iae->NextEffect();
	audioobject=iae->audioeffect;

	if(int c=song->GetCountOfAutomationTracksUsing(audioobject))
	{
		// Songs Automation Track -> Buffer
		automationtracks=new AutomationTrack*[c];
		automationtrack_c=c;
		song->FillAutomationListUsing(automationtracks,c,audioobject);
	}
	else
	{
		automationtracks=0;
		automationtrack_c=0;
	}
}

void Undo_DeletePlugin::FreeData()
{
	if(inundo==true)
	{
		if(audioobject)
		{
			delete audioobject;
			audioobject=0;
		}

		if(automationtracks)
		{
			for(int i=0;i<automationtrack_c;i++)
				delete automationtracks[i];
		}
	}

	if(automationtracks)
		delete automationtracks;
}

void Undo_DeletePlugin::RefreshPreRedo()
{
	maingui->RemoveAudioEffectFromGUI(effects->FindAudioObject(audioobject));
}

void Undo_DeletePlugin::Do()
{
	if(automationtracks)
	{
		for(int i=0;i<automationtrack_c;i++)
			automationtracks[i]->Cut();
	}

	if(audioobject)
		audioobject->BufferChunkData();

	effects->DeleteInsertAudioEffect(song,effects->FindAudioObject(audioobject),true,false);
}

void Undo_DeletePlugin::DoUndo()
{
	InsertAudioEffect *iae=effects->AddInsertAudioEffect(addnext,audioobject,false,false); // No Clone

	if(audioobject)
	{
		audioobject->RestoreChunkData();
		audioobject->FreeChunks();
	}

	if(automationtracks)
	{
		for(int i=0;i<automationtrack_c;i++)
		{
			automationtracks[i]->bindtoautomationobject->SetNewInsertAudioEffect(iae);
			automationtracks[i]->InsertOldIndex();
		}
	}
}

void EditFunctions::DeletePlugin(InsertAudioEffect *iae)
{
	if(iae)
	{
		Seq_Song *song=iae->effectlist->GetSong();

		if(song && CheckIfEditOK(song)==true)
		{
			if(Undo_DeletePlugin *dp=new Undo_DeletePlugin(song,iae))
			{
				song->undo.OpenUndoFunction(dp);
				dp->RefreshPreRedo(); // Remove GUI
				LockAndDoFunction(song,dp,true);
			}
		}
	}
}

void tmenu_deleteplugin::MenuFunction()
{
	mainedit->DeletePlugin(effect);
}

void tmenu_effect::MenuFunction()
{
	if(!oldeffect)
		mainedit->InsertPlugin(effects,fxplugin);
	else
		mainedit->ReplacePlugin(effects,oldeffect,fxplugin);
}


//Create Plugin
Undo_CreatePlugin::Undo_CreatePlugin(Seq_Song *s,InsertAudioEffect *iae)
{
	Init(s,iae);
}

void Undo_CreatePlugin::Init(Seq_Song *s,InsertAudioEffect *iae)
{
	id=Undo::UID_CREATEPLUGIN;
	song=s;
	effects=iae->effectlist;
	audioobject=iae->audioeffect;
}

void Undo_CreatePlugin::RefreshPreUndo()
{
	maingui->RemoveAudioEffectFromGUI(effects->FindAudioObject(audioobject));
}

void Undo_CreatePlugin::DoUndo()
{
	if(audioobject)
		audioobject->BufferChunkData();
	effects->DeleteInsertAudioEffect(0,effects->FindAudioObject(audioobject),true,false);
}

void Undo_CreatePlugin::Do()
{
	effects->AddInsertAudioEffect(0,audioobject,false,false); // No Clone

	if(audioobject)
	{
		audioobject->RestoreChunkData();
		audioobject->FreeChunks();
	}
}

void Undo_CreatePlugin::FreeData()
{
	if(inundo==false)
	{
		if(audioobject)
		{
			delete audioobject;
			audioobject=0;
		}
	}
}

void EditFunctions::InsertPlugin(AudioEffects *effects,AudioObject *ao,bool openeditor)
{
	if(!effects)
		return;

	Seq_Song *song=effects->GetSong();

	if(song && CheckIfEditOK(song)==true)
	{
		bool lock=false;

		if(song==mainvar->GetActiveSong())
		{
			mainthreadcontrol->LockActiveSong();
			lock=true;
		}

		if(InsertAudioEffect *newiae=effects->AddInsertAudioEffect(0,ao)) // Add To Empy
		{
			if(lock==true)
				mainthreadcontrol->UnlockActiveSong();

			if(Undo_CreatePlugin *cp=new Undo_CreatePlugin(song,newiae))
			{
				song->undo.OpenUndoFunction(cp);
				CheckEditElementsForGUI(song,cp,true);

				if(openeditor==true)
					newiae->audioeffect->OpenGUI(song,newiae);	// Open Editor
			}
		}
		else
		{
			if(lock==true)
				mainthreadcontrol->UnlockActiveSong();
		}

	}
}

// Replace
Undo_ReplacePlugin::Undo_ReplacePlugin(Seq_Song *s,InsertAudioEffect *oldi,InsertAudioEffect *newi)
{
	id=Undo::UID_REPLACEPLUGIN;

	song=s;
	effects=oldi->effectlist;

	old_audioobject=oldi->audioeffect;
	new_audioobject=newi->audioeffect;

	if(int c=song->GetCountOfAutomationTracksUsing(old_audioobject))
	{
		// Songs Automation Track -> Buffer
		automationtracks=new AutomationTrack*[c];
		automationtrack_c=c;
		song->FillAutomationListUsing(automationtracks,c,old_audioobject);
	}
	else
	{
		automationtracks=0;
		automationtrack_c=0;
	}
	skipinsert=true;
}

void Undo_ReplacePlugin::RefreshPreRedo()
{
	maingui->RemoveAudioEffectFromGUI(effects->FindAudioObject(old_audioobject));	
}

void Undo_ReplacePlugin::RefreshPreUndo()
{
	maingui->RemoveAudioEffectFromGUI(effects->FindAudioObject(new_audioobject));
}

void Undo_ReplacePlugin::Do()
{
	// Delete old
	if(automationtracks)
	{
		for(int i=0;i<automationtrack_c;i++)
			automationtracks[i]->Cut();
	}

	if(old_audioobject)
		old_audioobject->BufferChunkData();

	InsertAudioEffect *niae=effects->FindAudioObject(old_audioobject);
	if(niae)
		niae=niae->NextEffect();

	effects->DeleteInsertAudioEffect(0,effects->FindAudioObject(old_audioobject),true,false);

	// Add new
	if(skipinsert==false)
	{
		effects->AddInsertAudioEffect(niae,new_audioobject,false,false); // No Clone

		if(new_audioobject)
		{
			new_audioobject->RestoreChunkData();
			new_audioobject->FreeChunks();
		}
	}

	skipinsert=false;
}

void Undo_ReplacePlugin::DoUndo()
{
	// Old
	InsertAudioEffect *addnext=effects->FindAudioObject(new_audioobject);

	if(addnext)
		addnext=addnext->NextEffect();

	InsertAudioEffect *niae=effects->AddInsertAudioEffect(addnext,old_audioobject,false,false); // No Clone

	if(old_audioobject)
	{
		old_audioobject->RestoreChunkData();
		old_audioobject->FreeChunks();
	}

	if(automationtracks)
	{
		for(int i=0;i<automationtrack_c;i++)
		{
			automationtracks[i]->bindtoautomationobject->SetNewInsertAudioEffect(niae);
			automationtracks[i]->InsertOldIndex();
		}
	}

	// New
	if(new_audioobject)
		new_audioobject->BufferChunkData();

	effects->DeleteInsertAudioEffect(0,effects->FindAudioObject(new_audioobject),true,false);
}

void Undo_ReplacePlugin::FreeData()
{
	if(inundo==true)
	{
		if(automationtracks)
		{
			for(int i=0;i<automationtrack_c;i++)
				delete automationtracks[i];
		}
	}
	else
	{
		if(new_audioobject)
			delete new_audioobject;
	}

	if(automationtracks)
		delete automationtracks;
}

void EditFunctions::ReplacePlugin(AudioEffects *effects,InsertAudioEffect *oldiea,AudioObject *newao)
{
	if(!effects)
		return;

	if(!oldiea)
		return;

	if(!newao)
		return;

	Seq_Song *song=effects->GetSong();

	if(CheckIfEditOK(song)==true)
	{
		maingui->RemoveAudioEffectFromGUI(oldiea);

		bool lock=false;

		if(song==mainvar->GetActiveSong())
		{
			lock=true;
			mainthreadcontrol->LockActiveSong();
		}

		InsertAudioEffect *newiae=effects->AddInsertAudioEffect(oldiea,newao);

		if(!newiae)
		{
			if(lock==true)
				mainthreadcontrol->UnlockActiveSong();

			return;
		}

		if(Undo_ReplacePlugin *rp=new Undo_ReplacePlugin(song,oldiea,newiae))
		{
			//rp->RefreshPreRedo(); // Remove GUI
			rp->Do();
			//LockAndDoFunction(song,rp,true);

			if(lock==true)
				mainthreadcontrol->UnlockActiveSong();

			song->undo.OpenUndoFunction(rp);
			CheckEditElementsForGUI(song,rp,true);

			if(newiae->audioeffect)
				newiae->audioeffect->OpenGUI(song,newiae);	// Open Editor
		}
		else
		{
			if(lock==true)
				mainthreadcontrol->UnlockActiveSong();

		}
	}
}

// Create Plugin Group
Undo_CreatePlugins::Undo_CreatePlugins(Seq_Song *s,AudioEffects *fx,AudioObject **aol,int ct)
{
	id=Undo::UID_CREATEPLUGINS; // Same as Create Plugin
	song=s;
	audioobjects=aol;
	effects=fx;
	counter=ct;
}

void Undo_CreatePlugins::FreeData()
{
	if(inundo==false)
	{
		for(int i=0;i<counter;i++)
		{
			if(audioobjects[i])
			{
				delete audioobjects[i];
				audioobjects[i]=0;
			}
		}
	}

	if(audioobjects)
		delete audioobjects;
}

void Undo_CreatePlugins::RefreshPreUndo()
{
	for(int i=0;i<counter;i++)
	{
		maingui->RemoveAudioEffectFromGUI(effects->FindAudioObject(audioobjects[i]));
	}
}

void Undo_CreatePlugins::DoUndo()
{
	for(int i=0;i<counter;i++)
	{
		if(audioobjects[i])
			audioobjects[i]->BufferChunkData();

		effects->DeleteInsertAudioEffect(0,effects->FindAudioObject(audioobjects[i]),true,false);
	}
}

void Undo_CreatePlugins::Do()
{
	for(int i=0;i<counter;i++)
	{
		if(audioobjects[i])
		{
			effects->AddInsertAudioEffect(0,audioobjects[i],false,false); // No Clone

			if(audioobjects[i])
			{
				audioobjects[i]->RestoreChunkData();
				audioobjects[i]->FreeChunks();
			}
		}
	}
}

void EditFunctions::InsertPlugins(AudioEffects *from,AudioEffects *to) // Group
{
	if(!from)
		return;

	if(!to)
		return;

	Seq_Song *song=to->GetSong();

	if(CheckIfEditOK(song)==true)
	{
		int c=from->GetCountEffects();

		if(!c)
			return;

		if(AudioObject **aop=new AudioObject*[c])
		{
			int i=0;
			InsertAudioEffect *iae=from->FirstInsertAudioEffect();

			while(iae)
			{
				InsertAudioEffect *niae=to->AddInsertAudioEffect(0,iae->audioeffect,false,true); // Clone+Add To Empy
				aop[i++]=niae?niae->audioeffect:0;

				iae=iae->NextEffect();
			}

			if(Undo_CreatePlugins *cps=new Undo_CreatePlugins(song,to,aop,c))
			{
				song->undo.OpenUndoFunction(cps);
				CheckEditElementsForGUI(song,cps,true);
			}
		}
	}
}

Undo_DeletePlugins::Undo_DeletePlugins(Seq_Song *s,AudioEffects *x,AudioObject **a,int ct)
{
	id=Undo::UID_DELETEPLUGINS;
	song=s;
	effects=x;
	audioobjects=a;
	count=ct;

	autos=new int[ct];

	int ucount=0;

	if(autos)
		for(int i=0;i<count;i++)
		{
			autos[i]=song->GetCountOfAutomationTracksUsing(audioobjects[i]);
			ucount+=autos[i];
		}

		if(ucount)
		{
			automationtracks=new AutomationTrack*[ucount];

			if(automationtracks)
			{
				automationtrack_c=ucount;

				int ix=0;

				for(int i=0;i<count;i++)
				{
					if(int c=song->GetCountOfAutomationTracksUsing(audioobjects[i])){
						song->FillAutomationListUsing(&automationtracks[ix],c,audioobjects[i]);
						ix+=c;
					}
				}
			}
		}
		else
		{
			automationtracks=0;
			automationtrack_c=0;
		}
}

void Undo_DeletePlugins::Do()
{
	if(automationtracks)
	{
		for(int i=0;i<automationtrack_c;i++)
			automationtracks[i]->Cut();
	}

	for(int i=0;i<count;i++)
	{
		if(audioobjects[i])
			audioobjects[i]->BufferChunkData();

		effects->DeleteInsertAudioEffect(0,effects->FindAudioObject(audioobjects[i]),true,false);
	}
}

void Undo_DeletePlugins::DoUndo()
{
	int ax=0;

	for(int i=0;i<count;i++)
	{
		InsertAudioEffect *iae;

		if(audioobjects[i])
		{
			iae=effects->AddInsertAudioEffect(0,audioobjects[i],false,false); // No Clone

			if(audioobjects[i])
			{
				audioobjects[i]->RestoreChunkData();
				audioobjects[i]->FreeChunks();
			}
		}
		else
			iae=0;

		if(autos[i] && automationtracks)
		{
			for(int ai=ax;ai<ax+autos[ai];ai++)
			{
				automationtracks[ai]->bindtoautomationobject->SetNewInsertAudioEffect(iae);
				automationtracks[ai]->InsertOldIndex();
			}

			ax+=autos[i];
		}
	}
}

void Undo_DeletePlugins::RefreshPreRedo()
{
	for(int i=0;i<count;i++)
		maingui->RemoveAudioEffectFromGUI(effects->FindAudioObject(audioobjects[i]));
}

void Undo_DeletePlugins::FreeData()
{
	if(inundo==true)
	{
		for(int i=0;i<count;i++)
		{
			if(audioobjects[i])
				delete audioobjects[i];
		}

		if(automationtracks)
		{
			for(int i=0;i<automationtrack_c;i++)
				delete automationtracks[i];
		}
	}

	if(automationtracks)
		delete automationtracks;

	if(autos)
		delete autos;
}

void EditFunctions::DeletePlugins(AudioEffects *fx) // Group
{
	if(!fx)
		return;

	Seq_Song *song=fx->GetSong();

	if(CheckIfEditOK(song)==true)
	{
		int c=fx->GetCountEffects();

		if(!c)
			return;

		if(AudioObject **aop=new AudioObject*[c])
		{
			int i=0;

			InsertAudioEffect *x=fx->FirstInsertAudioEffect();
			while(x)
			{
				aop[i++]=x->audioeffect;
				x=x->NextEffect();
			}

			if(Undo_DeletePlugins *dps=new Undo_DeletePlugins(song,fx,aop,c))
			{
				song->undo.OpenUndoFunction(dps);
				dps->RefreshPreRedo(); // Remove GUI
				LockAndDoFunction(song,dps,true);
			}
		}
	}
}