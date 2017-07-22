// VST 1-2.4

#include "vstplugins.h"
#include "audiohardware.h"
#include <string.h>
#include "settings.h"
#include "vstdirectory.h"
#include "object_project.h"
#include "object_song.h"
#include "audiosystem.h"
#include "audiodevice.h"
#include "songmain.h"
#include "MIDIinproc.h"
#include "vstguiwindow.h"
#include "audioauto_vst.h"
#include "languagefiles.h"
#include "semapores.h"

#include "inputdata.h"
#include "MIDItimer.h"
#include "version.h"
#include "camxfile.h"
#include "gui.h"

#include <iostream>

#define MAXVSTIO 64

// #ifndef DEBUG
#define VSTACTIVATE
// #endif

// Plugin -> HOST
//host callback function
//this is called directly by the plug-in!!
//

//typedef	VstIntPtr (VSTCALLBACK *audioMasterCallback) (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);


VstIntPtr VSTCALLBACK hostcontrol(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
{
	VSTPlugin *vstplug=effect?(VSTPlugin *)effect->user:0;

	if(vstplug && vstplug->crashed)
		return 0;

	//	return 0;
	VstIntPtr retval=0;

#ifdef WIN64
	//maingui->MessageBoxOk(0,"x64 hostcontrol");
#endif

	/*
	if(vstplug)
	{
	TRACE ("HostControl vstplug %s OPCODE %d INDEX %d VALUE %d FLOAT %f POINTER %d \n",vstplug->GetEffectName(), opcode,index,value,opt,ptr);
	}
	else
	TRACE ("HostControl Global OPCODE %d INDEX %d VALUE %d FLOAT %f POINTER %d \n",opcode,index,value,opt,ptr);
	*/

	switch (opcode)
	{
		//VST 1.0 opcodes
	case audioMasterVersion:
		//Input values:
		//none

		//Return Value:
		//0 or 1 for old version
		//2 or higher for VST2.0 host?
		// cout << "plug called audioMasterVersion" << endl;
		retval=2400;
		break;

	case audioMasterCanDo:
		{
			//Input Values:
			//<ptr> predefined "canDo" string

			//Return Value:
			//0 = Not Supported
			//non-zero value if host supports that feature

			//NB - Possible Can Do strings are:
			//"sendVstEvents",
			//"sendVstMidiEvent",
			//"sendVstTimeInfo",
			//"receiveVstEvents",
			//"receiveVstMidiEvent",
			//"receiveVstTimeInfo",

			//"reportConnectionChanges",
			//"acceptIOChanges",

			//"sizeWindow",
			//"asyncProcessing",
			//"offline",
			//"supplyIdle",
			//"supportShell"
			// cout << "plug called audioMasterCanDo" << endl;

			char *string=(char *)ptr;

			TRACE ("MasterCanDo %s...",string);

			if (
				string &&
				(
				strcmp(string,"reportConnectionChanges")==0 ||
				strcmp(string,"acceptIOChanges")==0 ||
				strcmp(string,"sizeWindow")==0 ||

				strcmp(string,"sendVstEvents")==0 ||
				strcmp(string,"sendVstMidiEvent")==0 ||

				strcmp(string,"receiveVstTimeInfo")==0 ||
				strcmp(string,"sendVstTimeInfo")==0 ||

				strcmp(string,"receiveVstEvents")==0 ||
				strcmp(string,"receiveVstMidiEvent")==0 ||

				strcmp(string,"supplyIdle")==0
				// || strcmp(string,"supportShell")==0
				// || strcmp(string,"offline")==0

				)
				)
			{
				TRACE ("OKAY \n");

				return 1;
			}

			TRACE ("FALSE \n");

			return 0;
		}
		break;

	case audioMasterAutomate:
		//Input values:
		//<index> parameter that has changed
		//<opt> new value

		//Return value:
		//not tested, always return 0

		//NB - this is called when the plug calls
		//setParameterAutomated

		//	cout << "plug called audioMasterAutomate" << endl;
		if(vstplug && vstplug->song && vstplug->song->audiosystem.device)
		{
			OSTART time=/*vstplug->song->status&Seq_Song::STATUS_SONGPLAYBACK_AUDIO?vstplug->song->audioplayback_position:*/vstplug->song->GetSongPosition();

			vstplug->ValueChanged(time,index,opt);
		}

		TRACE ("audioMasterAutomate %d %f\n",index,opt);
		retval=0;
		break;

	case audioMasterCurrentId:
		//Input values:
		//none

		//Return Value
		//the unique id of a plug that's currently loading
		//zero is a default value and can be safely returned if not known
		//	cout << "plug called audioMasterCurrentId" << endl;
		retval=0;
		break;

	case audioMasterIdle:
		{
			//Input values:
			//none

			//Return Value
			//not tested, always return 0

			//NB - idle routine should also call effEditIdle for all open editors
			//Sleep(1);

			// cout << "plug called audioMasterIdle" << endl;
			// call application idle routine (this will
			// call effEditIdle for all open editors too)

			// vstplug->ptrPlug->dispatcher(vstplug->ptrPlug,effEditIdle,0,0,0,0.0f);

			if(vstplug)
				vstplug->refreshgui=true;

			retval=0;
		}
		break;

	case audioMasterProcessEvents: // Incoming Events plugin->host
		{
			if(vstplug && vstplug->song && vstplug->song->audiosystem.device)
			{
				VstEvents *vstevents=(VstEvents *)ptr;
				int nrevents=vstevents->numEvents;

				if(nrevents>=1)
				{
					//plugininproc->SendForce(vstplug->song,vstplug); // Send existing Timer Offset Events
					LONGLONG sampleposition=vstplug->song->playback_sampleposition;
					LONGLONG systime=maintimer->GetSystemTime();

					OSTART songposition=/*vstplug->song->status&Seq_Song::STATUS_SONGPLAYBACK_AUDIO?vstplug->song->audioplayback_position:*/vstplug->song->GetSongPosition();

					bool sendsignal=false;

					for(int i=0;i<nrevents;i++)
					{
						switch(vstevents->events[i]->type)
						{
						case kVstMidiType:
							{
								VstMidiEvent *vme=(VstMidiEvent *)vstevents->events[i]; // Cast ->

								if(vme->deltaFrames<vstplug->setSize)
								{
									if(NewEventData *in=new NewEventData) // Copy Data
									{
										//in->netime=vstplug->song->timetrack.ConvertSamplesToInternRate(vstplug->song->audiosystem.playback_sampleposition+VstMidiEvent->deltaFrames);
										in->netime=songposition;
										in->nesystime=systime;

										//in->type=kVstMIDIType;
										in->fromplugin=vstplug;
										in->nedeltatime=vme->deltaFrames?vstplug->song->timetrack.ConvertSamplesToTempoTicks(0,sampleposition+vme->deltaFrames):songposition;
										in->deltaframes=vme->deltaFrames;
										in->status=vme->midiData[0];
										in->byte1=vme->midiData[1];
										in->byte2=vme->midiData[2];
										in->songstatusatinput=vstplug->song->status;

										/*
										TRACE ("### New kVstMIDIType %s \n",vstplug->GetEffectName());

										TRACE ("Status %d B1:%d B2:%d \n",in->status,in->byte1,in->byte2);
										TRACE (" size=%d\n",VstMidiEvent->byteSize);

										TRACE (" deltaframes (MAX:=%d) %d\n",vstplug->setSize,VstMidiEvent->deltaFrames);
										TRACE (" flags=%d\n",VstMidiEvent->flags);
										TRACE (" noteOffset=%d\n",VstMidiEvent->noteOffset);
										TRACE (" velocityOff=%d\n",VstMidiEvent->noteOffVelocity);
										TRACE (" detune=%d\n",VstMidiEvent->detune);
										TRACE (" noteoffLength=%d\n",VstMidiEvent->noteLength);

										TRACE (" ### End \n");
										*/

										plugininproc->AddNewInputData(vstplug->song,in);
										sendsignal=true;
									}
								}
							}
							break;

						case kVstSysExType:
							{
								VstMidiSysexEvent *vstsysexevent=(VstMidiSysexEvent *)vstevents->events[i];

								if(vstsysexevent->deltaFrames<vstplug->setSize && vstsysexevent->dumpBytes && vstsysexevent->sysexDump)
								{
									if(NewEventData *in=new NewEventData) // Copy Data
									{
										in->netime=songposition;
										in->nesystime=systime;

										in->nedeltatime=vstsysexevent->deltaFrames?vstplug->song->timetrack.ConvertSamplesToTempoTicks(0,sampleposition+vstsysexevent->deltaFrames):songposition;
										in->fromplugin=vstplug;
										in->deltaframes=vstsysexevent->deltaFrames;

										in->status=SYSEX;
										in->songstatusatinput=vstplug->song->status;

										// Copy SysEx data
										if(in->data=new UBYTE[vstsysexevent->dumpBytes])
											memcpy(in->data,vstsysexevent->sysexDump,vstsysexevent->dumpBytes);

										in->datalength=vstsysexevent->dumpBytes;

										plugininproc->AddNewInputData(vstplug->song,in);
										sendsignal=true;
									}
								}
							}
							break;

						}//switch

					}// for

					if(sendsignal==true)
						plugininproc->SetSignal();
				}

				//Input Values:
				//<ptr> Pointer to a populated VstEvents structure

				//Return value:
				//0 if error
				//1 if OK
				// cout << "plug called audioMasterProcessEvents" << endl;
			}

			return 1;
		}
		break;

	case audioMasterGetTime:
		if(vstplug)
		{
			if(vstplug->song)
			{
				OSTART songposition=vstplug->song->mastering==true?vstplug->song->masteringposition:vstplug->song->GetSongPosition();
				double globsr=(double)mainaudio->GetGlobalSampleRate();

				vstplug->timeinfo.sampleRate = globsr;

				double h=(double)TICK4nd,h2=(double)songposition;
				h2/=h;

				vstplug->timeinfo.ppqPos = h2; // Musical Position, in Quarter Note (1.0 equals 1 Quarter Note)

				vstplug->song->playbacksettings.Lock();
				h2=(double)vstplug->song->playbacksettings.cyclestart;
				h2/=h;

				vstplug->timeinfo.cycleStartPos=h2; //Cycle Start (left locator), in Quarter Note

				h2=(double)vstplug->song->playbacksettings.cycleend;
				h2/=h;
				vstplug->timeinfo.cycleEndPos=h2; //Cycle End (right locator), in Quarter Note
				vstplug->song->playbacksettings.Unlock();

				vstplug->song->timetrack.LockTimeTrack();

				LONGLONG samplePos=vstplug->song->timetrack.ConvertTicksToTempoSamples(songposition);

				{
					double sPh=(double)samplePos,sh2=globsr/1000;

					vstplug->timeinfo.samplePos =sPh; //current Position in audio samples (always valid)
					vstplug->timeinfo.nanoSeconds=sPh*(1000000.0/sh2); // ok
				}

				Seq_Tempo *tempo=vstplug->song->timetrack.GetTempo(songposition);

				vstplug->timeinfo.tempo = tempo->tempo;

				h2=(double)vstplug->song->timetrack.ConvertTicksToMeasureTicks(songposition,false); // Last Measure
				h2/=h;

				vstplug->timeinfo.barStartPos = h2; //last Bar Start Position, in Quarter Note

				Seq_Signature *sig=vstplug->song->timetrack.FindSignatureBefore(songposition);

				vstplug->timeinfo.timeSigNumerator = sig->nn;
				vstplug->timeinfo.timeSigDenominator = sig->dn;

				vstplug->song->timetrack.UnlockTimeTrack();

				vstplug->timeinfo.smpteOffset = 0;
				vstplug->timeinfo.flags = kVstTempoValid|kVstTimeSigValid|kVstPpqPosValid|kVstCyclePosValid|kVstBarsValid|kVstNanosValid;

				switch(vstplug->song->project->standardsmpte) // Project SMPTE
				{
					/*
					\note VstTimeInfo::samplePos :Current Position. It must always be valid, and should not cost a lot to ask for. The sample position is ahead of the time displayed to the user. In sequencer stop mode, its value does not change. A 32 bit integer is too small for sample positions, and it's a double to make it easier to convert between ppq and samples.
					\note VstTimeInfo::ppqPos : At tempo 120, 1 quarter makes 1/2 second, so 2.0 ppq translates to 48000 samples at 48kHz sample rate.
					.25 ppq is one sixteenth note then. if you need something like 480ppq, you simply multiply ppq by that scaler.
					\note VstTimeInfo::barStartPos : Say we're at bars/beats readout 3.3.3. That's 2 bars + 2 q + 2 sixteenth, makes 2 * 4 + 2 + .25 = 10.25 ppq. at tempo 120, that's 10.25 * .5 = 5.125 seconds, times 48000 = 246000 samples (if my calculator servers me well :-). 
					\note VstTimeInfo::samplesToNextClock : MIDI Clock Resolution (24 per Quarter Note), can be negative the distance to the next MIDI clock (24 ppq, pulses per quarter) in samples. unless samplePos falls precicely on a MIDI clock, this will either be negative such that the previous MIDI clock is addressed, or positive when referencing the following (future) MIDI clock.

					//-------------------------------------------------------------------------------------------------------
					struct VstTimeInfo
					{
					//-------------------------------------------------------------------------------------------------------
					double samplePos;				///< current Position in audio samples (always valid)
					double sampleRate;				///< current Sample Rate in Herz (always valid)
					double nanoSeconds;				///< System Time in nanoseconds (10^-9 second)
					double ppqPos;					///< Musical Position, in Quarter Note (1.0 equals 1 Quarter Note)
					double tempo;					///< current Tempo in BPM (Beats Per Minute)
					double barStartPos;				///< last Bar Start Position, in Quarter Note
					double cycleStartPos;			///< Cycle Start (left locator), in Quarter Note
					double cycleEndPos;				///< Cycle End (right locator), in Quarter Note
					VstInt32 timeSigNumerator;		///< Time Signature Numerator (e.g. 3 for 3/4)
					VstInt32 timeSigDenominator;	///< Time Signature Denominator (e.g. 4 for 3/4)
					VstInt32 smpteOffset;			///< SMPTE offset (in SMPTE subframes (bits; 1/80 of a frame)). The current SMPTE position can be calculated using #samplePos, #sampleRate, and #smpteFrameRate.
					VstInt32 smpteFrameRate;		///< @see VstSmpteFrameRate
					VstInt32 samplesToNextClock;	///< MIDI Clock Resolution (24 Per Quarter Note), can be negative (nearest clock)
					VstInt32 flags;					///< @see VstTimeInfoFlags
					//-------------------------------------------------------------------------------------------------------
					};
					*/

					/*
					kVstSmpte24fps    = 0,		///< 24 fps
					kVstSmpte25fps    = 1,		///< 25 fps
					kVstSmpte2997fps  = 2,		///< 29.97 fps
					kVstSmpte30fps    = 3,		///< 30 fps
					kVstSmpte2997dfps = 4,		///< 29.97 drop
					kVstSmpte30dfps   = 5,		///< 30 drop

					kVstSmpteFilm16mm = 6, 		///< Film 16mm
					kVstSmpteFilm35mm = 7, 		///< Film 35mm
					kVstSmpte239fps   = 10,		///< HDTV: 23.976 fps
					kVstSmpte249fps   = 11,		///< HDTV: 24.976 fps
					kVstSmpte599fps   = 12,		///< HDTV: 59.94 fps
					kVstSmpte60fps    = 13		///< HDTV: 60 fps
					*/

				case Seq_Pos::POSMODE_SMPTE_24:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte24fps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_25:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte25fps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_2997:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte2997fps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_30:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte30fps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_2997df:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte2997dfps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_30df:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte30dfps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_239:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte239fps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_249:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte249fps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_599:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte599fps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;

				case Seq_Pos::POSMODE_SMPTE_60:
					vstplug->timeinfo.smpteFrameRate =kVstSmpte60fps;
					vstplug->timeinfo.flags |=kVstSmpteValid;
					break;
				}

				OSTART nextpreMIDIclock=mainvar->SimpleQuantize(songposition,SAMPLESPERBEAT/24);
				LONGLONG samplesToNextClock=0;

				if(nextpreMIDIclock<songposition)
					samplesToNextClock=-vstplug->song->timetrack.ConvertTicksToTempoSamplesStart(nextpreMIDIclock,songposition-nextpreMIDIclock); // negative
				else
					samplesToNextClock=vstplug->song->timetrack.ConvertTicksToTempoSamplesStart(songposition,nextpreMIDIclock-songposition); // positive

				vstplug->timeinfo.samplesToNextClock = (VstInt32)samplesToNextClock;

				vstplug->timeinfo.flags |=kVstClockValid;

				if(vstplug->song->mastering==true)
				{
					vstplug->timeinfo.flags|=kVstTransportPlaying;
				}
				else
					switch(vstplug->song->status)
				{
					case Seq_Song::STATUS_PLAY:
						vstplug->timeinfo.flags|=kVstTransportPlaying; ///< set if Host sequencer is currently playing
						break;

					case Seq_Song::STATUS_RECORD:
						vstplug->timeinfo.flags|=kVstTransportRecording;///< set if Host sequencer is in record mode
						break;
				}

				if(vstplug->song->playbacksettings.cycleplayback==true)
					vstplug->timeinfo.flags|=kVstTransportCycleActive;///< set if Host sequencer is in cycle mode

#ifdef WIN64
				//	maingui->MessageBoxOk(0,"x64 hostcontrol audioMasterGetTime");
#endif
			}
			else
			{
				// No Song
				double globsr=(double)mainaudio->GetGlobalSampleRate();
				vstplug->timeinfo.sampleRate = globsr;
				vstplug->timeinfo.ppqPos = 0; // Musical Position, in Quarter Note (1.0 equals 1 Quarter Note)
				vstplug->timeinfo.cycleStartPos=0; //Cycle Start (left locator), in Quarter Note
				vstplug->timeinfo.cycleEndPos=0; //Cycle End (right locator), in Quarter Note

				LONGLONG samplePos=0;

				{
					double sPh=(double)samplePos,sh2=globsr/1000;

					vstplug->timeinfo.samplePos =sPh; //current Position in audio samples (always valid)
					vstplug->timeinfo.nanoSeconds=sPh*(1000000.0/sh2); // ok
				}

				vstplug->timeinfo.tempo = 120;
				vstplug->timeinfo.barStartPos = 0; //last Bar Start Position, in Quarter Note

				vstplug->timeinfo.timeSigNumerator = 4;
				vstplug->timeinfo.timeSigDenominator = 4;

				vstplug->timeinfo.smpteOffset = 0;
				vstplug->timeinfo.flags = kVstTempoValid|kVstTimeSigValid|kVstPpqPosValid|kVstCyclePosValid|kVstBarsValid|kVstNanosValid;
				vstplug->timeinfo.samplesToNextClock = 0;
				vstplug->timeinfo.flags |=kVstClockValid;
			}

			return (VstIntPtr)(&vstplug->timeinfo);
		}
		break;

		//case audioMasterSetTime:
		//IGNORE!
		//	break;

		//case audioMasterNeedIdle:// plug needs idle calls (outside its editor window)
		//	{
		//Input Values:
		//None

		//Return Value:
		//0 if error
		//non-zero value if OK

		//NB plug needs idle calls (outside its editor window)
		//this means that effIdle must be dispatched to the plug
		//during host idle process and not effEditIdle calls only when its
		//editor is open
		//Check despatcher notes for any return codes from effIdle
		// cout << "plug called audioMasterNeedIdle" << endl;

		//		int i=0;
		//		return 1;
		//	}
		//	break;

		/*
		case audioMasterTempoAt:
		//Input Values:
		//<value> sample frame location to be checked

		//Return Value:
		//tempo (in bpm * 10000)
		// cout << "plug called audioMasterTempoAt" << endl;
		if(mainvar->GetActiveSong())
		{
		Seq_Tempo *t=mainvar->GetActiveSong()->timetrack.GetTempo(mainvar->GetActiveSong()->GetSongPosition());

		double h=t->tempo*10000;
		retval=(long)floor(h+0.5);
		}
		else
		retval=120*10000; // Default BPM
		break;
		*/

		/*
		case audioMasterGetNumAutomatableParameters:
		//Input Values:
		//None

		//Return Value:
		//number of automatable parameters
		//zero is a default value and can be safely returned if not known

		//NB - what exactly does this mean? can the host set a limit to the
		//number of parameters that can be automated?
		// cout << "plug called audioMasterGetNumAutomatableParameters" << endl;
		break;
		*/

		/*
		case audioMasterGetParameterQuantization:
		//Input Values:
		//None

		//Return Value:
		//integer value for +1.0 representation,
		//or 1 if full single float precision is maintained
		//in automation.

		//NB - ***possibly bugged***
		//Steinberg notes say "parameter index in <value> (-1: all, any)"
		//but in aeffectx.cpp no parameters are taken or passed
		// cout << "plug called audioMasterGetParameterQuantization" << endl;
		break;
		*/

	case audioMasterIOChanged:
		//Input Values:
		//None

		//Return Value:
		//0 if error
		//non-zero value if OK
		// cout << "plug called audioMasterIOChanged" << endl;
		return 0;

		break;

	case audioMasterSizeWindow:
		{
			//Input Values:
			//<index> width
			//<value> height

			if(vstplug)
			{
				vstplug->LockRefreshSize();

				vstplug->refreshwindowsize=true;
				vstplug->newwidth=index;
				vstplug->newheight=value;

				vstplug->UnlockRefreshSize();

				return 1;
			}

			//TRACE ("VST new size %d %d\n",width,height);

			//Return Value:
			//0 if error
			//non-zero value if OK
			// cout << "plug called audioMasterSizeWindow" << endl;

			return 0;
		}
		break;

	case audioMasterGetSampleRate:
		{
			if(vstplug && vstplug->song)
				return vstplug->setSampleRate;

			return mainaudio->GetGlobalSampleRate();
		}
		break;

	case audioMasterGetBlockSize:
		{
			if(vstplug)
				return vstplug->setSize;

			if(mainaudio->GetActiveDevice())
				return mainaudio->GetActiveDevice()->GetSetSize();

			return DEFAULTAUDIOBLOCKSIZE;
		}
		break;

	case audioMasterGetInputLatency:
		{
			int i=0;
			//Input Values:
			//None

			//Return Value:
			//input latency (in sampleframes?)
			// cout << "plug called audioMasterGetInputLatency" << endl;
		}
		break;

	case audioMasterGetOutputLatency:
		//Input Values:
		//None

		//Return Value:
		//output latency (in sampleframes?)
		// cout << "plug called audioMasterGetOutputLatency" << endl;
		break;

		/*
		case audioMasterGetPreviousPlug:
		//Input Values:
		//None

		//Return Value:
		//pointer to AEffect structure or NULL if not known?

		//NB - ***possibly bugged***
		//Steinberg notes say "input pin in <value> (-1: first to come)"
		//but in aeffectx.cpp no parameters are taken or passed
		// cout << "plug called audioMasterGetPreviousPlug" << endl;
		break;
		*/

		/*
		case audioMasterGetNextPlug:
		//Input Values:
		//None

		//Return Value:
		//pointer to AEffect structure or NULL if not known?

		//NB - ***possibly bugged***
		//Steinberg notes say "output pin in <value> (-1: first to come)"
		//but in aeffectx.cpp no parameters are taken or passed
		// cout << "plug called audioMasterGetNextPlug" << endl;
		break;
		*/

		/*
		case audioMasterWillReplaceOrAccumulate:
		//Input Values:
		//None

		//Return Value:
		//0: not supported
		//1: replace
		//2: accumulate
		// cout << "plug called audioMasterWillReplaceOrAccumulate" << endl;
		break;
		*/

	case audioMasterGetCurrentProcessLevel:
		{
			return 2;
			//Input Values:
			//None

			//Return Value:
			//0: not supported,
			//1: currently in user thread (gui)
			//2: currently in audio thread (where process is called)
			//3: currently in 'sequencer' thread (MIDI, timer etc)
			//4: currently offline processing and thus in user thread
			//other: not defined, but probably pre-empting user thread.
			// cout << "plug called audioMasterGetCurrentProcessLevel" << endl;
		}
		break;

	case audioMasterGetAutomationState:
		{
			return 0;

			//Input Values:
			//None

			//Return Value:
			//0: not supported
			//1: off
			//2:read
			//3:write
			//4:read/write
			// cout << "plug called audioMasterGetAutomationState" << endl;
		}
		break;

	case audioMasterGetVendorString:
		//Input Values:
		//<ptr> string (max 64 chars) to be populated

		//Return Value:
		//0 if error
		//non-zero value if OK
		// cout << "plug called audioMasterGetVendorString" << endl;

		if(ptr)
		{
			strcpy((char *)ptr,"ism");
			return 1;
		}

		return 0;

		break;

	case audioMasterGetProductString:
		//Input Values:
		//<ptr> string (max 64 chars) to be populated

		//Return Value:
		//0 if error
		//non-zero value if OK
		// cout << "plug called audioMasterGetProductString" << endl;

		if(ptr)
		{
			strcpy((char *)ptr,"CamX");
			return 1;
		}

		return 0;

		break;

	case audioMasterGetVendorVersion:
		//Input Values:
		//None

		//Return Value:
		//Vendor specific host version as integer
		// cout << "plug called audioMasterGetVendorVersion" << endl;
		return CAMXVERSION;

		break;

	case audioMasterVendorSpecific:
		{
			int i=1;
			//Input Values:
			//<index> lArg1
			//<value> lArg2
			//<ptr> ptrArg
			//<opt>	floatArg

			//Return Values:
			//Vendor specific response as integer
			// cout << "plug called audioMasterVendorSpecific" << endl;
		}
		break;

		/*
		case audioMasterSetIcon:
		//IGNORE
		break;
		*/


	case audioMasterGetLanguage:
		{
			//Input Values:
			//None

			//Return Value:
			//kVstLangEnglish
			//kVstLangGerman
			//kVstLangFrench
			//kVstLangItalian
			//kVstLangSpanish
			//kVstLangJapanese
			// cout << "plug called audioMasterGetLanguage" << endl;

#ifdef DEU
			retval=kVstLangGerman;
#endif

#ifdef ENG
			retval=kVstLangEnglish;
#endif
		}
		break;
		/*
		MAC SPECIFIC?

		case audioMasterOpenWindow:
		//Input Values:
		//<ptr> pointer to a VstWindow structure

		//Return Value:
		//0 if error
		//else platform specific ptr
		cout << "plug called audioMasterOpenWindow" << endl;
		break;

		case audioMasterCloseWindow:
		//Input Values:
		//<ptr> pointer to a VstWindow structure

		//Return Value:
		//0 if error
		//Non-zero value if OK
		cout << "plug called audioMasterCloseWindow" << endl;
		break;
		*/
	case audioMasterGetDirectory:
		//Input Values:
		//None

		//Return Value:
		//0 if error
		//FSSpec on MAC, else char* as integer

		//NB Refers to which directory, exactly?
		// cout << "plug called audioMasterGetDirectory" << endl;
		break;

	case audioMasterUpdateDisplay:
		{
			if(vstplug)
			{
				vstplug->updatedisplay=true;
			}

			retval=1;
			//Input Values:
			//None

			//Return Value:
			//Unknown
			// cout << "plug called audioMasterUpdateDisplay" << endl;
		}
		break;

		//---from here VST 2.1 extension opcodes------------------------------------------------------
	case audioMasterBeginEdit: // begin of automation session (when mouse down), parameter index in <index>
		{
			//effect->beginEdit(VstInt32 index);

			if(vstplug)
			{
				vstplug->BeginEdit(vstplug->song,index);
			}

			retval=1; //1 = successful
		}
		break;

	case audioMasterEndEdit: // end of automation session (when mouse up),     parameter index in <index>
		{

			if(vstplug)
			{
				vstplug->EndEdit(vstplug->song,index);
			}

			retval=1; //1 = successful
		}
		break; 

	case audioMasterOpenFileSelector:		// open a fileselector window with VstFileSelect* in <ptr>
		{
			int i=0;
		}
		break;

		//---from here VST 2.2 extension opcodes------------------------------------------------------
	case audioMasterCloseFileSelector:		// close a fileselector operation with VstFileSelect* in <ptr>: Must be always called after an open !
		{
			int i=0;
		}
		break;

		/*
		case audioMasterEditFile:				// open an editor for audio (defined by XML text in ptr)
		break;
		*/

		/*
		case audioMasterGetChunkFile:			// get the native path of currently loading bank or project
		// (called from writeChunk) void* in <ptr> (char[2048], or sizeof(FSSpec))
		break;
		*/

		//---from here VST 2.3 extension opcodes------------------------------------------------------

		//case audioMasterGetInputSpeakerArrangement:	// result a VstSpeakerArrangement in ret
		//	break;

	default:
		TRACE ("VST Default %d\n",opcode);
		break;
	}

	return retval;
}

VSTPlugin *mainAudio::FindVSTEffect(char *dllname)
{
	if(dllname)
	{
		for(int i=0;i<2;i++)
		{
			// 1. Check Effects
			VSTPlugin *c=i==0?FirstVSTEffect():FirstVSTInstrument();
			while(c)
			{
				if(strcmp(dllname,c->dllname)==0)return c;
				c=c->NextVSTPlugin();
			}
		}

		/*
		{
		// 1. Check Effects
		VSTPlugin *c=FirstVSTEffect();
		while(c)
		{
		if(strcmp(dllname,c->dllname)==0)return c;
		c=c->NextVSTPlugin();
		}
		}

		{
		// 2. Check Instruments
		VSTPlugin *c=FirstVSTInstrument();
		while(c){
		if(strcmp(dllname,c->dllname)==0)return c;
		c=c->NextVSTPlugin();
		}
		}
		*/

	}

	return 0;
}

// Collect .dll's from directory vst_plugins
class VSTTest
{
public:
	VSTTest()
	{
		ok=gone=crashed=false;
		nvst=0;
	}

	LONGLONG dlllength;
	Directory *dir;
	VSTPlugin *nvst;
	char *fulldllname,*dllname;
	bool ok,gone,crashed;
};

PTHREAD_START_ROUTINE VSTTestFunc(LPVOID pParam)
{
	VSTTest *test=(VSTTest *)pParam;

	char *fulldllname=test->fulldllname;
	char *dllname=test->dllname;
	Directory *dir=test->dir;
	LONGLONG dlllength=test->dlllength;

#ifdef DEBUG
	if(dlllength==0)
		maingui->MessageBoxError(0,"dlllength=0");
#endif

	VSTPlugin *nvst=0;

	TRACE ("VSTTestFunc %s\n",fulldllname);

#ifdef TRYCATCH
	try
#endif
	{

		/*
		if(!libhandle)
		{
		#ifdef WIN64
		maingui->MessageBoxError(0,"No lib Win64");
		#endif

		maingui->MessageBoxError(0,fulldllname);
		}
		*/

		if(HINSTANCE libhandle=LoadLibrary(fulldllname))
		{
			//DLL was loaded OK
			AEffect* (__cdecl* getNewPlugInstance)(audioMasterCallback);
			getNewPlugInstance=(AEffect*(__cdecl*)(audioMasterCallback))GetProcAddress(libhandle, "main");

			if (getNewPlugInstance)
			{
				AEffect *ptrPlug=0;

				ptrPlug=getNewPlugInstance(hostcontrol);
				
				if(ptrPlug)
				{
					if (ptrPlug->magic==kEffectMagic && // VST ?
						ptrPlug->numParams>=0 // Parameter >=0 ?
						)
					{
						test->ok=true; // Its a VST dll

						if(nvst=new VSTPlugin)
						{
							nvst->ptrPlug=ptrPlug;
							nvst->ins=ptrPlug->numInputs;
							nvst->outs=ptrPlug->numOutputs;
							nvst->numberofparameter=ptrPlug->numParams;
							nvst->numberofprograms=ptrPlug->numPrograms;

							nvst->filesize=dlllength;
							nvst->dllname=mainvar->GenerateString(dllname);
							nvst->fulldllname=mainvar->GenerateString(fulldllname);

							if(nvst->fulldllname)
							{
								// Check VST Plugin

								//	ptrPlug->dispatcher(ptrPlug,effOpen,0,0,NULL,0.0f); //open

								//	ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,0,NULL,0.0f); // plugin off



								/*
								// set samplerate
								ptrPlug->dispatcher(ptrPlug,effSetSampleRate,0,0,NULL,(float)mainaudio->GetGlobalSampleRate()); // plugin off

								// set blocksize
								if(mainaudio->selectedaudiohardware && mainaudio->selectedaudiohardware->GetActiveDevice())
								ptrPlug->dispatcher(ptrPlug,effSetBlockSize,0,mainaudio->selectedaudiohardware->GetActiveDevice()->setSize,NULL,0.0f); 
								else
								ptrPlug->dispatcher(ptrPlug,effSetBlockSize,0,DEFAULTAUDIOBLOCKSIZE,NULL,0.0f); 
								*/

								// Idle Calls
								//long idle=ptrPlug->dispatcher(ptrPlug,effIdle,0,0,0,0.0f);

								// More Vst Checks
								// returns 2 for VST 2; older versions return 0; 2100 for VST 2.1
								VstIntPtr vstversion=0;

								vstversion=ptrPlug->dispatcher(ptrPlug,effGetVstVersion,0,0,NULL,0.0f);

								switch(vstversion)
								{
								case 0:
									vstversion=1000;
									break;

								case 2:
									vstversion=2000;
									break;
								}

								bool effname=false;

								if(vstversion>=2000)
								{
									// Get Plugin Name
									effname=(bool)ptrPlug->dispatcher(ptrPlug,effGetEffectName,0,0,nvst->effectname,0.0f);
								}

								if(vstversion>=2000)
								{
									ptrPlug->dispatcher(ptrPlug,effGetVendorString,0,0,nvst->company,0.0f);
								}

								if(vstversion>=2000)
								{
									VstIntPtr version=ptrPlug->dispatcher(ptrPlug,effGetVendorVersion,0,0,0,0.0f);

									nvst->version=version;
#ifdef DEBUG
									TRACE ("Version %d\n",version);
#endif
								}


								if(effname==false) // vst 1.0 ?
								{
									// Remove .dll
									char rdll[kVstMaxEffectNameLen+1];

									// Use DLL Name
									if(strlen(dllname)>kVstMaxEffectNameLen)
									{
										strncpy(rdll,dllname,kVstMaxEffectNameLen);
										rdll[kVstMaxEffectNameLen]=0;
									}
									else
										strcpy(rdll,dllname);

									size_t h=strlen(rdll);

									while(h--)
									{
										if(rdll[h]=='.' && strcmp(&rdll[h],".dll")==0)
										{
											rdll[h]=0;
											break;
										}
									}

									strcpy(nvst->effectname,rdll);
								}

								if(ptrPlug->flags & effFlagsIsSynth)
								{
									nvst->audioeffecttype=audioobject_TYPE_INSTRUMENT;
									nvst->synth=true;
								}

								if(ptrPlug->flags & effFlagsNoSoundInStop) // VST 2.4
								{
									nvst->NoSoundInStop=true;
								}

								if(ptrPlug->flags & effFlagsCanDoubleReplacing) // VST 2.4
								{
									nvst->canDoubleReplacing=true;
								}

								if(ptrPlug->flags & effFlagsProgramChunks)
								{
									nvst->intern_flag|=VST_PROGRAMCHUNKS;
								}

								if(ptrPlug->flags & effFlagsHasEditor)
								{
									nvst->intern_flag|=VST_HASEDITOR;
								}

								nvst->vst_version=vstversion;
								nvst->type=kPlugCategUnknown;

								if (vstversion>=2000)
								{
									// Type
									nvst->type=ptrPlug->dispatcher(ptrPlug,effGetPlugCategory,0,0,0,0.0f);

									switch(nvst->type)
									{
									case kPlugCategSynth:
										{
											nvst->audioeffecttype=audioobject_TYPE_INSTRUMENT;
											nvst->synth=true;
										}
										break;
									}

									/*
									//-------------------------------------------------------------------------------------------------------
									kPlugCategUnknown = 0,		///< Unknown, category not implemented
									kPlugCategEffect,			///< Simple Effect
									kPlugCategSynth,			///< VST Instrument (Synths, samplers,...)
									kPlugCategAnalysis,			///< Scope, Tuner, ...
									kPlugCategMastering,		///< Dynamics, ...
									kPlugCategSpacializer,		///< Panners, ...
									kPlugCategRoomFx,			///< Delays and Reverbs
									kPlugSurroundFx,			///< Dedicated surround processor
									kPlugCategRestoration,		///< Denoiser, ...
									kPlugCategOfflineProcess,	///< Offline Process
									kPlugCategShell,			///< Plug-in is container of other plug-ins  @see effShellGetNextPlugin
									kPlugCategGenerator,		///< ToneGenerator, ...

									kPlugCategMaxCount			///< Marker to count the categories
									*/

									// VST 2.0 ********************************************
									//			cout << "This is a VST 2 plugin" << endl;


									if(ptrPlug->dispatcher(ptrPlug,effCanDo,0,0,"sendVstEvents",0.0f)>0)
									{
										nvst->intern_flag|=VST_SENDVSTEVENTS;
									}

									if(ptrPlug->dispatcher(ptrPlug,effCanDo,0,0,"sendVstMidiEvent",0.0f)>0)
									{
										nvst->intern_flag|=VST_SENDVstMidiEventS;
									}

									if(ptrPlug->dispatcher(ptrPlug,effCanDo,0,0,"receiveVstEvents",0.0f)>0)
									{
										nvst->intern_flag|=VST_RECEIVEVSTEVENTS;
									}

									if(ptrPlug->dispatcher(ptrPlug,effCanDo,0,0,"receiveVstMidiEvent",0.0f)>0)
									{
										nvst->intern_flag|=VST_RECEIVEVstMidiEventS;

										// VST 2 Instrument
										nvst->audioeffecttype=audioobject_TYPE_INSTRUMENT;
										nvst->synth=true;
									}//if

									if(ptrPlug->dispatcher(ptrPlug,effCanDo,0,0,"bypass",0.0f)>0)
									{
										nvst->intern_flag|=VST_SUPPORTBYPASS;
									}

									nvst->GetIOConfig();
									nvst->GetSpeakerArrangement();
								}

								nvst->directory=dir;
							}
						}// if vst

						//	ptrPlug->dispatcher(ptrPlug,effClose,0,0,NULL,0.0f); // Close Plugin
					}

					// Close

					ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,0,NULL,0.0f); // suspend
					ptrPlug->dispatcher(ptrPlug,effClose,0,0,NULL,0.0f);


				}//if ptr Plug

			}//if getNewPlugInstance	

			FreeLibrary(libhandle);
		}

		if(nvst)
		{
			nvst->ptrPlug=0; // else crash first Usage !
			test->nvst=nvst;
		}

		test->gone=true;
	}

#ifdef TRYCATCH
	catch(...)
	{
		test->crashed=true;
	}
#endif

	return 0;
}

bool mainAudio::TestVST(char *fulldllname,char *dllname,LONGLONG dlllength,Directory *dir)
{

#ifdef DEBUG
	if(dlllength==0)
		maingui->MessageBoxError(0,"dlllength 1 =0");
#endif

	// Check for Same vst's
	// Check Effects
	if(dlllength>0 && fulldllname)
	{
		for(int i=0;i<2;i++)
		{
			VSTPlugin *s=i==0?FirstVSTEffect():FirstVSTInstrument();

			while(s)
			{
				TRACE ("Check %s In %s\n",s->dllname,dllname);

				if(s->fulldllname && strcmp(s->fulldllname,fulldllname)==0) // Same Plugin Name
				{
#ifdef DEBUG
					if(s->filesize!=dlllength)
					{
						maingui->MessageBoxOk(0,"DLL !=");
					}
#endif

					if(s->filesize==dlllength)// Same Plugin Size ?
					{
						return false;
					}
				}

				s=s->NextVSTPlugin();
			}
		}
	}

	if(IsPluginCrashed(fulldllname)==true)
		return false;

	char *pfile=mainsettings->GetSettingsFileName(Settings::SETTINGSFILE_PLUGINTEST);
	camxFile plugintest;

	if(plugintest.OpenSave(pfile)==true)
	{
		short l=strlen(fulldllname);

		plugintest.Save(&l,sizeof(short));
		plugintest.Save(fulldllname,l);
		
		plugintest.Close(true);
	}

	if(char *h=mainvar->GenerateString("Test VST:",fulldllname,"..."))
	{
		maingui->SetInfoWindowText(h);
		delete h;
	}

	VSTTest vsttest;

	vsttest.fulldllname=fulldllname;
	vsttest.dllname=dllname;
	vsttest.dlllength=dlllength;
	vsttest.dir=dir;

	HANDLE ThreadHandle=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)VSTTestFunc,(LPVOID)&vsttest, 0,0); // Audio File Buffer Refill Thread

	if(ThreadHandle)
	{
		int mscount=0;
		int max=30*1000; // ms 30 sek wait

		bool msinfo1=false;
		bool msinfo2=false;

		for(;;)
		{
			if(vsttest.crashed==true)
			{
				if(char *h=mainvar->GenerateString("Test VST:",fulldllname,"...CRASHED"))
				{
					maingui->SetInfoWindowText(h);
					delete h;
				}

				Sleep(5000);
				break;
			}

			if(vsttest.gone==true)
			{
#ifdef WIN32
				if(char *h=mainvar->GenerateString("Test VST:",fulldllname,"...NO x86 VST 1 - 2.4"))
				{
					maingui->SetInfoWindowText(h);
					delete h;
				}
#endif

#ifdef WIN64
				if(char *h=mainvar->GenerateString("Test VST:",fulldllname,"...NO x64 VST 1 - 2.4"))
				{
					maingui->SetInfoWindowText(h);
					delete h;
				}
#endif

				break;
			}

			if(msinfo1==false && mscount>=10000)
			{
				msinfo1=true;

				if(char *h=mainvar->GenerateString("Test VST:",fulldllname,"...WAIT 10sec...?"))
				{
					maingui->SetInfoWindowText(h);
					delete h;
				}
			}

			if(msinfo2==false && mscount>=20000)
			{
				msinfo2=true;

				if(char *h=mainvar->GenerateString("Test VST:",fulldllname,"...WAIT 20sec...??"))
				{
					maingui->SetInfoWindowText(h);
					delete h;
				}
			}

			if(mscount>max)
			{
				if(char *h=mainvar->GenerateString("Test VST:",fulldllname,"...ERROR (Break)! (TimeOut...Crashed ?)"))
				{
					maingui->SetInfoWindowText(h);
					delete h;
				}

				Sleep(5000);
				break;
			}

			mscount+=50;
			Sleep(50); // 50ms
		}
	}

	mainvar->DeleteAFile(pfile);
	
	if(vsttest.nvst)
	{
		if(char *h=mainvar->GenerateString("Test VST:",fulldllname,"...OK"))
		{
			maingui->SetInfoWindowText(h);
			delete h;
		}

		VSTPlugin *nvst=vsttest.nvst;

		// // AddSort to VST Plugin List
		if(nvst->audioeffecttype!=audioobject_TYPE_INSTRUMENT) 
			nvst->vstlist=&vsteffects;
		else
			nvst->vstlist=&vstinstruments;

		SortVSTPlugin(nvst->vstlist,nvst);

		return true;
	}

	return false;
}

void mainAudio::SortVSTPlugin(OList *list,VSTPlugin *nvst)
{
#ifdef DEBUG

	if(nvst->filesize==0)
		maingui->MessageBoxError(0,"SortVSTPlugin FS=0");
#endif

	nvst->vstlist=list;

	// Add
	if(nvst->dllname && nvst->fulldllname)
	{
		// Sort Company-Name
		if(nvst->GetCompany() && strlen(nvst->GetCompany())>0)
		{
			VSTPlugin *check=(VSTPlugin *)list->GetRoot();

			while(check)
			{
				if((!check->GetCompany()) || strlen(check->GetCompany())==0)
				{
					list->AddNextO(nvst,check);
					list->Close();

					return;
				}

				int r=mainvar->strcmp_allsmall(nvst->GetCompany(),check->GetCompany());

				if(r==0)
				{
					// Same Company
					VSTPlugin *sortcheck=check,*lastwithsamplecompany=0;
					while(sortcheck)
					{
						if(mainvar->strcmp_allsmall(nvst->GetCompany(),sortcheck->GetCompany())==0)
						{
							lastwithsamplecompany=sortcheck;

							// Sort By Name
							//A zero value indicates that both strings are equal.
							//A value greater than zero indicates that the first character that does not match has a greater value in str1 than in str2; And a value less than zero indicates the opposite.

							TRACE ("Sort %s < %s\n",nvst->GetEffectName(),sortcheck->GetEffectName());

							r=mainvar->strcmp_allsmall(nvst->GetEffectName(),sortcheck->GetEffectName());

							if(r<=0)
							{
								list->AddNextO(nvst,sortcheck);
								list->Close();

								return;
							}

						}
						else
						{
							list->AddPrevO(nvst,lastwithsamplecompany);
							list->Close();

							return;
						}

						sortcheck=sortcheck->NextVSTPlugin();
					}

					list->AddPrevO(nvst,lastwithsamplecompany);
					return;
				}
				else
				{
					if(r<0)
					{
						list->AddNextO(nvst,check);
						list->Close();
						return;
					}
				}

				check=check->NextVSTPlugin();
			}
		}

		// No Company

		// Sort By Name
		VSTPlugin *sortcheck=(VSTPlugin *)list->Getc_end();

		while(sortcheck)
		{
			if((!sortcheck->GetCompany()) || strlen(sortcheck->GetCompany())==0)
			{
				int r=mainvar->strcmp_allsmall(nvst->GetEffectName(),sortcheck->GetEffectName());

				if(r<1)
				{
					list->AddNextO(nvst,sortcheck);
					list->Close();
					return;
				}
			}

			sortcheck=(VSTPlugin *)sortcheck->prev;
		}

		list->AddEndO(nvst);
	}
}

int mainAudio::CollectVSTPlugins(int ix,Directory *directory,bool checkcheckplugins) // Null == Default
{
	maingui->SetInfoWindowText("Collect+Test VST Plugins...");

#ifdef VSTACTIVATE

	bool checkall=false;

	Directory *f=directory;

	if(!f){
		checkall=true;
		f=mainsettings->FirstVSTDirectory(ix);
	}

	while(f)
	{
		camxFile testdir;

		if(f->dir)
		{
			testdir.BuildDirectoryList(f->dir,"*.*",".dll");
		}

		camxScan *sc=testdir.FirstScan();
		while(sc)
		{
			TRACE ("VST Scan found %s \n",sc->name);

			bool test=true;

			if(checkcheckplugins==true)
			{
				// Check earlier check Plugins
				VSTPlugin_Settings *found=0;
				OList *list=0;

				// Effects
				VSTPlugin_Settings *fx=(VSTPlugin_Settings *)mainsettings->settingsreadvsteffects.GetRoot();
				while(fx)
				{
					if(strcmp(fx->fulldllname,sc->name)==0 && sc->nFileSizeLow==fx->filesize)
					{
						found=fx;
						list=&mainaudio->vsteffects;
						break;
					}

					fx=(VSTPlugin_Settings *)fx->next;
				}

				// Instruments
				if(!found)
				{
					VSTPlugin_Settings *instr=(VSTPlugin_Settings *)mainsettings->settingsreadvstinstruments.GetRoot();
					while(instr)
					{
						if(strcmp(instr->fulldllname,sc->name)==0 && sc->nFileSizeLow==instr->filesize)
						{
							found=instr;
							list=&mainaudio->vstinstruments;
							break;
						}

						instr=(VSTPlugin_Settings *)instr->next;
					}
				}

				if(found && list)
				{
					test=false;

					bool inlist=false;

					// In List ?
					VSTPlugin *check=(VSTPlugin *)list->GetRoot();
					while(check)
					{
						if(strcmp(check->fulldllname,found->fulldllname)==0)
						{
							inlist=true;
							break;
						}

						check=check->NextVSTPlugin();
					}

					if(inlist==false)
					{
						if(VSTPlugin *nvst=new VSTPlugin)
						{
							// Copy --> VST
							nvst->filesize=found->filesize;
							nvst->vst_version=found->vstversion;
							nvst->audioeffecttype=found->audioeffecttype;
							nvst->synth=found->synth;
							nvst->canDoubleReplacing=found->canDoubleReplacing;
							nvst->NoSoundInStop=found->NoSoundInStop;
							nvst->type=found->type;
							nvst->numberofparameter=found->numberofparameter;
							nvst->numberofprograms=found->numberofprograms;
							nvst->intern_flag=found->internflag;
							nvst->ins=found->ins;
							nvst->outs=found->outs;
							nvst->directory=f;
							nvst->version=found->version;
							nvst->plugin_active=found->active;

							// Copy String
							nvst->dllname=mainvar->GenerateString(found->dllname);
							nvst->fulldllname=mainvar->GenerateString(found->fulldllname);

							if(found->effectname)
								strcpy(nvst->effectname,found->effectname);

							if(found->company)
								strcpy(nvst->company,found->company);

							f->vstcount++;

							SortVSTPlugin(list,nvst);
						}
					}
				}	
			}

			if(test==true)
			{
				if(TestVST(sc->name,sc->filename,sc->nFileSizeLow,f)==true)
				{
					newvstpluginsadded++;
					f->vstcount++;
				}
			}

			sc=sc->NextScan();
		}

		f=checkall==true?f->NextDirectory():0;

	}//while

	return vsteffects.GetCount()+vstinstruments.GetCount();

#else
	return 0;
#endif
}

void mainAudio::AddCrashedPlugin(char *dll,bool save)
{
	if(!dll)
		return;

	if(IsPluginCrashed(dll)==false)
	{
		if(CrashedPlugin *cp=new CrashedPlugin)
		{
			cp->fulldllname=mainvar->GenerateString(dll);
			
			if(cp->fulldllname)
			{
				crashplugins.AddEndO(cp);
			
				if(save==true)
				mainsettings->SaveCrashedPlugins();
			}
			else
				delete cp;
		}
	}
}

bool mainAudio::IsPluginCrashed(char *dll)
{
	CrashedPlugin *cp=(CrashedPlugin *)crashplugins.GetRoot();

	while(cp)
	{
		if(strcmp(dll,cp->fulldllname)==0)
			return true;

		cp=(CrashedPlugin *)cp->next;
	}
	return false;
}

void mainAudio::DeleteCrashedPlugin(CrashedPlugin *cp)
{
	delete cp->fulldllname;
	crashplugins.RemoveO(cp);
}

void mainAudio::CloseAllVSTPlugins()
{
	VSTPlugin *vst=FirstVSTEffect();
	while(vst)vst=vst->DeleteVSTPlugin();

	vst=FirstVSTInstrument();
	while(vst)
		vst=vst->DeleteVSTPlugin();

	// Close Crashed Plugins
	CrashedPlugin *cp=(CrashedPlugin *)crashplugins.GetRoot();
	while(cp)
	{
		if(cp->fulldllname)
			delete cp->fulldllname;

		cp=(CrashedPlugin *)crashplugins.RemoveO(cp);
	}

}

// VST plugin
VSTPlugin::VSTPlugin()
{	
	id=OBJ_AUDIOVST;

#ifdef DEBUG
	n[0]='V';
	n[1]='P';
#endif

	refreshgui=false;
	vstlist=0;
	fulldllname=0;
	dllname=0;

#ifdef WIN32
	ptrPlug=0;
	libhandle=0;
#endif

	effectname[0]=company[0]=0;
	ptrInputBuffers=ptrOutputBuffers=0;

	// FLAG
	synth=false;
	NoSoundInStop=false;
	canDoubleReplacing=false;
	NoSoundInStop=false;
	refreshwindowsize=false;

	ins=outs=0;

	//Init Plugins VstEvents Array
	type=kPlugCategUnknown;

	// MIDI Array
	if(vsteventarray=(VstEvents *)new char[sizeof(VstEvents)-sizeof(VstEvent *)*2+sizeof(VstEvent *)*MAXVSTRIGGEREVENTS])
	{
		/*
		Now we have:

		struct varVstEvents
		{
		//-------------------------------------------------------------------------------------------------------
		VstInt32 numEvents;		///< number of Events in array
		VstIntPtr reserved;		///< zero (Reserved for future use)
		VstEvent* events[MAXVSTRIGGEREVENTS];	///< event pointer array, VARIABLE size in Steinberg SDK=2 !
		//-------------------------------------------------------------------------------------------------------
		};
		*/

		// Now Init the VARIABLE VstEvent* events[MAXVSTRIGGEREVENTS] Array

		for(int i=0;i<MAXVSTRIGGEREVENTS;i++)
			vsteventarray->events[i]=(VstEvent *)&VstMidiEvents[i]; // VstMidiEvent VstMidiEvents[MAXVSTRIGGEREVENTS];
	}

	// SysEx Array
	if(vstsysexeventarray=(VstEvents *)new char[sizeof(VstEvents)-sizeof(VstEvent *)*2+sizeof(VstEvent *)*MAXVSTRIGGEREVENTS])
	{
		for(int i=0;i<MAXVSTRIGGEREVENTS;i++)
			vstsysexeventarray->events[i]=(VstEvent *)&vstsysexMIDIevents[i]; // VstMidiEvent VstMidiEvents[MAXVSTRIGGEREVENTS];
	}

	floattype=FT_32BIT;

#ifdef DEBUG
	noteoncounter=noteoffcounter=0;
#endif

	version=0;
}

VSTPlugin *VSTPlugin::DeleteVSTPlugin()
{
	VSTPlugin *n=NextVSTPlugin();

	Close(true);
	vstlist->RemoveO(this);

	return n;
}

bool VSTPlugin::DoEffect(AudioEffectParameter *par) // virtual
{
	if(crashed)return false;

	// 32bit VST
	if(ptrOutputBuffers && par->in && par->out)
	{
		//if(//	(ptrPlug->flags & effFlagsCanMono) ||
		//	par->out->channelsinbuffer>=ptrPlug->numOutputs
		//	) // File Channels > Plugin Channels
		//{
		//bool mix=par->in->channelsused>0?true:false;
		AudioHardwareBuffer *buffer=/*(ptrPlug->flags & effFlagsCanReplacing)?*/par->out/*:par->in*/;

		if(buffer)
		{
#ifdef ARES64
			if(floattype==FT_64BIT)
			{
				//64Bit float

				if(!ptrInputBuffers) // clear output before process 0 Inputs
				{
					// Delete output
					memset(buffer->outputbufferARES,0,sizeof(double)*setSize*GetOutputPins());

					if(ptrOutputBuffers)
					{
						// Connect Output Pins
						double *out=(double *)buffer->outputbufferARES;

						// Init Output
						for(int i=0;i<GetOutputPins();i++)
						{
							ptrOutputBuffers[i]=out;
							out+=setSize;
						}	
					}
				}
				else
				{
					// Connect Input Pins
					double *in=(double *)par->in->outputbufferARES;

					for(int i=0;i<GetSetInputPins();i++)
					{
						ptrInputBuffers[i]=in;
						in+=setSize;
					}

					if(ptrOutputBuffers)
					{
						// Connect Output Pins
						double *out=(double *)buffer->outputbufferARES;

						// Init Output
						for(int i=0;i<GetSetOutputPins();i++)
						{
							ptrOutputBuffers[i]=out;
							out+=setSize;
						}	
					}
				}

				//if(ptrPlug->flags & effFlagsCanReplacing)
				{
					// In->Process->Out

					//	par->alreadymixed=false;
					par->separateoutputs=true;

					if(mainsettings->plugincheck==Settings::PLUGINCHECK_NOCHECKS)
					{
						// out ++ = *in++;
						// Input -> FX -> Out

						//double->double
						ptrPlug->processDoubleReplacing
							(
							ptrPlug,
							(double **)ptrInputBuffers, //ptrInputBuffers or NULL
							(double **)ptrOutputBuffers,
							setSize
							);
					}
					else
					{
						if(par->song->mastering==false && mainsettings->plugincheck==Settings::PLUGINCHECK_TRYCATCHANDTIMER)
						{
							LONGLONG stime=maintimer->GetSystemTime();

							try
							{
								//double->double
								ptrPlug->processDoubleReplacing
									(
									ptrPlug,
									(double **)ptrInputBuffers, //ptrInputBuffers or NULL
									(double **)ptrOutputBuffers,
									setSize
									);
							}

							catch(...)
							{
								crashed=VSTCRASH_processReplacing;
								return false;
							}

							LONGLONG etime=maintimer->GetSystemTime();

							if(etime-stime>timeusage)
								timeusage=etime-stime;
						}
						else
						{
							try
							{
								// out ++ = *in++;
								// Input -> FX -> Out

								//double->double
								ptrPlug->processDoubleReplacing
									(
									ptrPlug,
									(double **)ptrInputBuffers, //ptrInputBuffers or NULL
									(double **)ptrOutputBuffers,
									setSize
									);
							}

							catch(...)
							{
								crashed=VSTCRASH_processReplacing;
							}
						}
					}
				}
				/*
				else
				{
				// out ++ += *in++;
				ptrPlug->process(ptrPlug,ptrInputBuffers,ptrOutputBuffers,par->out->samplesinbuffer);
				//	par->alreadymixed=true;
				par->separateoutputs=false;
				}
				*/

				//}
			}
			else
#endif

			{
				//32Bit float	

				if(!ptrInputBuffers) // clear output before process 0 Inputs
				{
					// Delete output
					memset(buffer->outputbufferARES,0,sizeof(float)*setSize*GetOutputPins());

					if(ptrOutputBuffers)
					{
						// Connect Output Pins
						float *out=(float *)buffer->outputbufferARES;

						// Init Output
						for(int i=0;i<GetOutputPins();i++)
						{
							ptrOutputBuffers[i]=out;

#ifdef DEBUG
							if(out==0)
								maingui->MessageBoxError(0,"ptrOutputBuffers");
#endif
							out+=setSize;
						}	
					}
				}
				else
				{
					// Connect Input Pins

#ifdef ARES64
					float *in=(float *)par->output32;
#else
					float *in=(float *)par->in->outputbufferARES;
#endif

					for(int i=0;i<GetSetInputPins();i++)
					{
						ptrInputBuffers[i]=in;
						in+=setSize;
					}

					if(ptrOutputBuffers)
					{
						// Connect Output Pins
						float *out=(float *)buffer->outputbufferARES;

						if(outs>ins)
						{
							// 1/2
							// Connect Output Pins
							float *out=(float *)buffer->outputbufferARES;

							// Init Output
							for(int i=0;i<GetOutputPins();i++)
							{
								ptrOutputBuffers[i]=out;
								out+=setSize;
							}	
						}
						else
						{
							// Init Output
							for(int i=0;i<GetSetOutputPins();i++)
							{
								ptrOutputBuffers[i]=out;
								out+=setSize;
							}
						}
					}
				}

				//if(ptrPlug->flags & effFlagsCanReplacing)
				{
					// separateoutputs

					//par->alreadymixed=false;
					par->separateoutputs=true;

#ifdef TRYCATCH
					if(mainsettings->plugincheck==Settings::PLUGINCHECK_NOCHECKS)
#endif
					{
						//float->float
						ptrPlug->processReplacing
							(
							ptrPlug,
							(float **)ptrInputBuffers, //ptrInputBuffers or NULL
							(float **)ptrOutputBuffers,
							setSize
							);
					}

#ifdef TRYCATCH
					else
					{
						if(par->song->mastering==false && mainsettings->plugincheck==Settings::PLUGINCHECK_TRYCATCHANDTIMER)
						{
							LONGLONG stime=maintimer->GetSystemTime();

							try
							{
								// out ++ = *in++;
								// Input -> FX -> Out

								//double->double
								//ptrPlug->processDoubleReplacing

								//float->float
								ptrPlug->processReplacing
									(
									ptrPlug,
									(float **)ptrInputBuffers, //ptrInputBuffers or NULL
									(float **)ptrOutputBuffers,
									setSize
									);
							}

							catch(...)
							{
								crashed=VSTCRASH_processReplacing;
								return false;
							}

							LONGLONG etime=maintimer->GetSystemTime();

							if(etime-stime>timeusage)
								timeusage=etime-stime;
						}
						else
						{

							try
							{
								// out ++ = *in++;
								// Input -> FX -> Out

								//double->double
								//ptrPlug->processDoubleReplacing

								//float->float
								ptrPlug->processReplacing
									(
									ptrPlug,
									(float **)ptrInputBuffers, //ptrInputBuffers or NULL
									(float **)ptrOutputBuffers,
									setSize
									);
							}

							catch(...)
							{
								crashed=VSTCRASH_processReplacing;
								return false;
							}
						}
					}
#endif

				}
				/*
				else
				{
				// out ++ += *in++;
				ptrPlug->process(ptrPlug,ptrInputBuffers,ptrOutputBuffers,par->out->samplesinbuffer);
				//par->alreadymixed=true;
				par->separateoutputs=false;
				}
				*/

				//}
			} // float

#ifdef _DEBUG
			buffer->CheckBuffer();
#endif
		}
		else
		{
#ifdef DEBUG
			maingui->MessageBoxError(0,"DoEffect 32 Buffer");
#endif

			par->effecterror=true;
		}

		return true;
	}

	par->effecterror=true;
	return false;
}

void VSTPlugin::Reset()
{
	if(crashed)return;

	if(ptrPlug)
	{
		try
		{
			ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,0,NULL,0.0f); // switch off
			ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,1,NULL,0.0f); // switch on
		}

		catch(...)
		{
			crashed=VSTCRASH_Reset;
		}
	}
}

void VSTPlugin::PlugInOn()
{
	if(crashed)return;

	///< [value]: 0 means "turn off", 1 means "turn on"  @see AudioEffect::suspend @see AudioEffect::resume

#ifdef TRYCATCH
	try
#endif
	{
		ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,1,NULL,0.0f); // switch on
	}

#ifdef TRYCATCH
	catch(...)
	{
		crashed=VSTCRASH_Bypass;
	}
#endif

}

void VSTPlugin::PlugInOff()
{
	if(crashed)return;

	///< [value]: 0 means "turn off", 1 means "turn on"  @see AudioEffect::suspend @see AudioEffect::resume

#ifdef TRYCATCH
	try
#endif
	{
		ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,0,NULL,0.0f); // switch off
	}

#ifdef TRYCATCH
	catch(...)
	{
		crashed=VSTCRASH_Bypass;
	}
#endif
}

void VSTPlugin::PlugInOpen()
{
	if(crashed)return;

#ifdef TRYCATCH
	try
#endif
	{
		ptrPlug->dispatcher(ptrPlug,effOpen,0,0,NULL,0.0f); // open Opcode=0
	}

#ifdef TRYCATCH
	catch(...)
	{
		crashed=VSTCRASH_Open;
	}
#endif
}

void VSTPlugin::PlugInClose()
{
	if(crashed)return;

#ifdef TRYCATCH
	try
#endif
	{
		ptrPlug->dispatcher(ptrPlug,effOpen,1,0,NULL,0.0f); // close Opcode=1
	}

#ifdef TRYCATCH
	catch(...)
	{
		crashed=VSTCRASH_Close;
	}
#endif
}

void VSTPlugin::ClearBuffer()
{

}

void VSTPlugin::InitSampleRateAndSize(int rate,int size)
{
	timeusage=0;

	if(crashed)return;

	if(ptrPlug) // open ?
	{
#ifdef TRYCATCH
		try
#endif
		{
			ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,0,NULL,0.0f); // switch off
			ptrPlug->dispatcher(ptrPlug,effSetSampleRate,0,0,0,(float)rate);

			// effSetBlockSizeAndSampleRate,			// block size in <value>, sampleRate in <opt>
			// virtual long dispatcher (long opCode, long index, long value, void *ptr, float opt);

			ptrPlug->dispatcher(ptrPlug,effSetBlockSize,0,size,0,0);
			ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,1,NULL,0.0f); // switch on
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_SetSampleSize;
		}
#endif
	}
}


bool VSTPlugin::InitIOChannels(int channels)
{
	if(vst_version>=2000)
	{
#ifdef ARES64

#ifdef TRYCATCH
		try
#endif
		{
			VstIntPtr ret=ptrPlug->dispatcher(ptrPlug,effSetProcessPrecision,0,kVstProcessPrecision64,0,0.0f);

			if(ret==1)
			{
				floattype=FT_64BIT;
			}
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_SetProcessPrecision;
			return false;
		}
#endif

#endif

		VstSpeakerArrangement in,out;

		in.numChannels=0;

		switch(channels)
		{
		case 1:
			in.type=out.type=kSpeakerArrMono;
			in.speakers[0].type=out.speakers[0].type=kSpeakerM;
			out.numChannels=in.numChannels=1;
			break;

		case 2:
			in.type=out.type=kSpeakerArrStereo;
			out.speakers[0].type=in.speakers[0].type=kSpeakerL;
			out.speakers[1].type=in.speakers[1].type=kSpeakerR;
			out.numChannels=in.numChannels=2;
			break;
		}

		if(in.numChannels>0)
		{
			if(in.numChannels>ins)in.numChannels=ins; // e.g. 0/2
			if(out.numChannels>outs)out.numChannels=outs;

			///< [value]: input #VstSpeakerArrangement* [ptr]: output #VstSpeakerArrangement*  @see AudioEffectX::setSpeakerArrangement

#ifdef TRYCATCH
			try
#endif
			{
				VstIntPtr ret=ptrPlug->dispatcher(ptrPlug,effSetSpeakerArrangement,0,(VstIntPtr)&in,&out,0.0f);
				if(ret==1)return true;
			}

#ifdef TRYCATCH
			catch(...)
			{
				crashed=VSTCRASH_SetSpeakerArrangement;
				return false;
			}
#endif
		}
	}

	return false;
}

guiWindow *	VSTPlugin::CheckIfWindowIsEditor(guiWindow *win)
{
	if(win->GetEditorID()==EDITORTYPE_PLUGIN_VSTEDITOR)
	{
		Edit_Plugin_VST *evst=(Edit_Plugin_VST *)win;
		if(evst->vstplugin==this)return win;
	}

	return 0;
}

char *VSTPlugin::GenerateInfoString()
{
	char h[512],h2[NUMBERSTRINGLEN];

	h[0]=0;

	// VERSION
	int vh=vst_version/1000;
	int vl=vst_version-vh*1000;
	vl/=100;

	mainvar->AddString(h," (V:");
	mainvar->AddString(h,mainvar->ConvertIntToChar(vh,h2));
	mainvar->AddString(h,".");
	mainvar->AddString(h,mainvar->ConvertIntToChar(vl,h2));
	mainvar->AddString(h,") ");

	//if(canmono==true)
	//	mainvar->AddString(h,"canMono ");

	mainvar->AddString(h,"Par:");
	mainvar->AddString(h,mainvar->ConvertIntToChar(numberofparameter,h2));

	mainvar->AddString(h," Prgs:");
	mainvar->AddString(h,mainvar->ConvertIntToChar(numberofprograms,h2));

	if(canDoubleReplacing==true)
		mainvar->AddString(h," canDouble");

	if(NoSoundInStop==true)
		mainvar->AddString(h," NoSoundInStop");

	if(GetOwnEditor()==true)
		mainvar->AddString(h," Editor");

	//Type
	char *ctype="-";

	switch(type)
	{
		//-------------------------------------------------------------------------------------------------------
	case kPlugCategUnknown:		///< Unknown, category not implemented

		break;

	case kPlugCategEffect:		///< Simple Effect
		ctype="Effect";
		break;

	case kPlugCategSynth:			///< VST Instrument (Synths, samplers,...)
		ctype="Synth";
		break;

	case kPlugCategAnalysis:			///< Scope, Tuner, ...
		ctype="Analysis";
		break;
	case kPlugCategMastering:		///< Dynamics, ...
		ctype="Mastering";
		break;

	case kPlugCategSpacializer:		///< Panners, ...
		ctype="Spacializer";
		break;

	case kPlugCategRoomFx:		///< Delays and Reverbs
		ctype="RoomFx";
		break;

	case kPlugSurroundFx:		///< Dedicated surround processor
		ctype="SurroundFx";
		break;

	case kPlugCategRestoration:	///< Denoiser, ...
		ctype="Restoration";
		break;

	case kPlugCategOfflineProcess:	///< Offline Process
		ctype="OfflineProcess";
		break;

	case kPlugCategShell:			///< Plug-in is container of other plug-ins  @see effShellGetNextPlugin
		ctype="Shell";
		break;

	case kPlugCategGenerator:		///< ToneGenerator, ...
		ctype="Generator";
		break;
	}

	if(intern_flag&VST_SENDVSTEVENTS)
		mainvar->AddString(h," SVE");

	if(intern_flag&VST_SENDVstMidiEventS)
		mainvar->AddString(h," SVME");

	if(intern_flag&VST_RECEIVEVSTEVENTS)
		mainvar->AddString(h," RVE");

	if(intern_flag&VST_RECEIVEVstMidiEventS)
		mainvar->AddString(h," RVME");

	if(intern_flag&VST_SUPPORTBYPASS)
		mainvar->AddString(h," BY");

	if(intern_flag&VST_PROGRAMCHUNKS)
		mainvar->AddString(h," PC");

	return mainvar->GenerateString(h," cat:",ctype);
}

bool VSTPlugin::GetOwnEditor()
{
	return (intern_flag&VST_HASEDITOR)?true:false;
}

void VSTPlugin::InitOwnEditor(int *width,int *height)
{
#ifdef DEBUG
	if((ptrPlug->flags & effFlagsHasEditor)==0)
		maingui->MessageBoxError(0,"VST InitOwnEditor");
#endif

#ifdef TRYCATCH
	try
#endif
	{
		ERect* psRect=NULL;
		ptrPlug->dispatcher(ptrPlug,effEditGetRect, 0, 0, (void *)&psRect, 0.0f);

		if(psRect)
		{
			*width=psRect->right-psRect->left;
			*height=psRect->bottom-psRect->top;
		}
	}

#ifdef TRYCATCH
	catch(...)
	{
		crashed=VSTCRASH_effEditGetRect;
	}
#endif
}

guiWindow *VSTPlugin::OpenGUI(Seq_Song *song,InsertAudioEffect *ieffect,guiWindowSetting *set)
{	
	if(crashed)return 0;

	guiWindow *win=0;

	if(ptrPlug) // open ?
	{
		/*
		if (!(ptrPlug->flags & effFlagsHasClip))
		{
		//Meaning: plugin can provide information to drive a clipping display
		int a=1;
		}

		if (!(ptrPlug->flags & effFlagsHasVu))
		{
		//Meaning: plugin can provide information to drive a VU display
		int i=1;
		}
		*/

		win=maingui->OpenEditor(EDITORTYPE_PLUGIN_VSTEDITOR,song,0,0,set,this,ieffect);
	}

	return win;
}

bool VSTPlugin::CloseGUI()
{
	if (GetOwnEditor()==true)
	{
		ptrPlug->dispatcher(ptrPlug,effEditClose,0,0,0,0);
		return true;
	}

	return false;
}

void VSTPlugin::Close(bool full)
{	
	//Shut the plugin down and free the library (this deletes the C++ class
	//memory and calls the appropriate destructors...)
	if(libhandle)
	{
		if(ptrPlug)
		{
			if(!crashed)
			{
#ifdef TRYCATCH
				try
#endif
				{
					ptrPlug->dispatcher(ptrPlug,effMainsChanged,0,0,NULL,0.0f); // suspend
					ptrPlug->dispatcher(ptrPlug,effClose,0,0,NULL,0.0f);
				}

#ifdef TRYCATCH
				catch(...)
				{
					crashed=VSTCRASH_effClose;
				}
#endif
			}
		}

		FreeLibrary(libhandle);

		libhandle=0;
		ptrPlug=0;

		if(ptrInputBuffers)
		{
			delete ptrInputBuffers;
			ptrInputBuffers=0;
		}

		if(ptrOutputBuffers)
		{
			delete ptrOutputBuffers;
			ptrOutputBuffers=0;
		}
	}

	if(full==true)
	{
		FreeChunks();

		if(dllname){
			delete dllname;
			dllname=0;
		}

		if(fulldllname){
			delete fulldllname;
			fulldllname=0;
		}

		if(vsteventarray)
		{
			delete vsteventarray;
			vsteventarray=0;
		}

		if(vstsysexeventarray)
		{
			delete vstsysexeventarray;
			vstsysexeventarray=0;
		}
	}

	vstoverflowbuffer.DeleteAllO();
}

char *VSTPlugin::GetCompany()
{
	/*
	, effGetVendorString					///< [ptr]: buffer for effect vendor string, limited to #kVstMaxVendorStrLen  @see AudioEffectX::getVendorString
	, effGetProductString					///< [ptr]: buffer for effect vendor string, limited to #kVstMaxProductStrLen  @see AudioEffectX::getProductString
	, effGetVendorVersion					///< [return value]: vendor-specific version  @see AudioEffectX::getVendorVersion
	, effVendorSpecific						///< no definition, vendor specific handling  @see AudioEffectX::vendorSpecific
	*/

	/*
	try
	{
	ptrPlug->dispatcher(ptrPlug,effGetVendorString,0,0,company,0.0f); // suspend
	}

	catch(...)
	{
	crashed=VSTCRASH_effGetCompany;
	}
	*/
	return company;
}

void VSTPlugin::Load(camxFile *file)
{
	// Program
	long program=0;

	file->ReadChunk(&program);
	SetProgram(program);
}

void VSTPlugin::CopySettings(VSTPlugin *n)
{
	AudioObject::CopySettings(n);

	n->synth=synth;
	n->canDoubleReplacing=canDoubleReplacing;
	n->NoSoundInStop=NoSoundInStop;
	n->type=type;
	n->vst_version=vst_version;
	n->version=version;

	//n->canmono=canmono;

	strcpy(n->effectname,effectname);
	strcpy(n->company,company);

#ifdef ARES64
	if(n->canDoubleReplacing==true)
		n->floattype=FT_64BIT;
#endif
}

void VSTPlugin::Save(camxFile *file)
{
	file->Save_ChunkString(dllname); // name
	file->Save_ChunkString(fulldllname); // name
	file->Save_Chunk(filesize);

	TRACE ("VST %s\n",fulldllname);

	// Program
	long program=GetProgram();
	file->Save_Chunk(program);
}

bool VSTPlugin::CanChunk()
{
	if(intern_flag&VST_PROGRAMCHUNKS)return true;
	return false;
}

bool VSTPlugin::CanMIDIInput()
{
	if(intern_flag&VST_SENDVstMidiEventS)
		return true;

	return false;
}

void VSTPlugin::LoadChunkData(camxFile *file)
{
	LONGLONG datasize=0;
	file->ReadChunk(&datasize);

	if(datasize)
	{
		if(void *data=new char[(int)datasize])
		{
			LONGLONG rsize=file->ReadChunk(data,(int)datasize);

			if(rsize==datasize)
			{
				///< [ptr]: chunk data [value]: byte size [index]: 0 for bank, 1 for program  @see AudioEffect::setChunk
				//(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

				if(CanChunk()==true)
				{
#ifdef TRYCATCH
					try
#endif
					{
						ptrPlug->dispatcher(ptrPlug,effSetChunk,0,(VstIntPtr)rsize,data, 0);
					}

#ifdef TRYCATCH
					catch(...)
					{
					}
#endif
				}
			}

			delete data;
		}
	}
}

void VSTPlugin::SaveChunkData(camxFile *file)
{
	VstIntPtr datasize=0;
	void *chunkdata=0;

	if(CanChunk()==true)
	{
		if(vst_version>=2300)
		{
			/*
			VstPatchChunkInfo cinfo;

			VstIntPtr r=ptrPlug->dispatcher(ptrPlug,effBeginLoadBank,0,0,&cinfo, 0);

			//[return value]: -1: bank can't be loaded, 1: bank can be loaded, 0: unsupported

			TRACE ("R %d effBeginLoadBank \n",r);
			if(r)
			{
			}
			*/
		}

#ifdef TRYCATCH
		try
#endif
		{
			datasize=ptrPlug->dispatcher(ptrPlug,effGetChunk,0,0,&chunkdata, 0);
		}

#ifdef TRYCATCH
		catch(...)
		{
		}
#endif

	}

	LONGLONG savesize=chunkdata?datasize:0;

	TRACE ("effGetChunk dll %s Data %d Size %d\n",dllname,chunkdata,datasize);

	file->Save_Chunk(savesize);

	if(savesize)
	{
		file->Save_Chunk(chunkdata,datasize);
	}

}

bool VSTPlugin::IsInstrument()
{
	if(GetInputPins()==0)
		return true;

	if(type==kPlugCategSynth)
		return true;

	return false;
}

bool VSTPlugin::CheckSettings()
{
#ifdef DEBUG
	if(filesize==0)
		maingui->MessageBoxError(0,"VST FS=0");

	if(fulldllname==0)
		maingui->MessageBoxError(0,"VST FDLL=0");
#endif

	if(filesize==0)
		return false;

	if(fulldllname==0)
		return false;

	return true;
}

void VSTPlugin::SaveSettings(camxFile *sfile)
{


	TRACE ("Save Settings %s\n",fulldllname);

	sfile->Save_Chunk(filesize);
	sfile->Save_ChunkString(fulldllname);
	sfile->Save_ChunkString(dllname);
	sfile->Save_ChunkString(effectname);

	sfile->Save_Chunk(numberofparameter);
	sfile->Save_Chunk(vst_version);

	sfile->Save_Chunk(audioeffecttype);
	sfile->Save_Chunk(synth);
	bool excanreceiveevnts=false;
	sfile->Save_Chunk(excanreceiveevnts);
	sfile->Save_Chunk(ins);
	sfile->Save_Chunk(outs);

	bool canmono=false;
	sfile->Save_Chunk(canmono);
	sfile->Save_Chunk(type);
	sfile->Save_Chunk(canDoubleReplacing);
	sfile->Save_Chunk(NoSoundInStop);
	sfile->Save_Chunk(intern_flag);
	sfile->Save_Chunk(numberofprograms);
	sfile->Save_ChunkString(company);
	sfile->Save_Chunk(version);
	sfile->Save_Chunk(plugin_active);
}

void VSTPlugin_Settings::ReadSettings(camxFile *file)
{
	file->ReadChunk(&filesize);

#ifdef DEBUG
	if(filesize==0)
		maingui->MessageBoxError(0,"ReadSettings FS==0");
#endif

	file->Read_ChunkString(&fulldllname);
	file->Read_ChunkString(&dllname);
	file->Read_ChunkString(&effectname);

	file->ReadChunk(&numberofparameter);
	file->ReadChunk(&vstversion);

	file->ReadChunk(&audioeffecttype);
	file->ReadChunk(&synth);
	file->ReadChunk(&canreceiveEvents);
	file->ReadChunk(&ins);
	file->ReadChunk(&outs);

	bool canmono;
	file->ReadChunk(&canmono);
	file->ReadChunk(&type);
	file->ReadChunk(&canDoubleReplacing);
	file->ReadChunk(&NoSoundInStop);
	file->ReadChunk(&internflag);
	file->ReadChunk(&numberofprograms);
	file->Read_ChunkString(&company);
	file->ReadChunk(&version);
	file->ReadChunk(&active);
}

void VSTPlugin_Settings::FreeMemory()
{
	if(fulldllname)delete fulldllname;
	fulldllname=0;

	if(dllname)delete dllname;
	dllname=0;

	if(effectname)delete effectname;
	effectname=0;

	if(company)delete company;
	company=0;
}

AudioObject *VSTPlugin::InitOpenEffect()
{
	libhandle=LoadLibrary(fulldllname);

	if(!libhandle)
	{
		Delete(true);
		return 0;
	}

	AEffect* (__cdecl* getNewPlugInstance)(audioMasterCallback);
	getNewPlugInstance=(AEffect*(__cdecl*)(audioMasterCallback))GetProcAddress(libhandle, "main");

	ptrPlug=0;

#ifdef TRYCATCH
	try
#endif
	{
		ptrPlug=getNewPlugInstance(hostcontrol);
	}

#ifdef TRYCATCH
	catch(...)
	{

	}
#endif

	if(!ptrPlug)
	{
		FreeLibrary(libhandle);
		libhandle=0;
		Delete(true);
		return 0;
	}

	ptrPlug->user=this;

	if(ins>0)
		ptrInputBuffers=new void*[ins];
	else
		ptrInputBuffers=0;

	if(outs>0)
		ptrOutputBuffers=new void*[outs];
	else
		ptrOutputBuffers=0;

	PlugInOpen();

	return this;
}

void VSTPlugin::RestoreChunkData()
{
	if(chunkdata_buffer && CanChunk()==true && ptrPlug)
	{
#ifdef TRYCATCH
		try
#endif
		{

			///< [ptr]: void** for chunk data address [index]: 0 for bank, 1 for program  @see AudioEffect::getChunk
			ptrPlug->dispatcher(ptrPlug,effSetChunk,0,chunksize_buffer,chunkdata_buffer, 0);
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_SetChunk;
		}
#endif
	}
}

void VSTPlugin::BufferChunkData()
{
	FreeChunks();

	if(CanChunk()==true)
	{
		VstIntPtr datasize=0;
		void *chunkdata=0;

#ifdef TRYCATCH
		try
#endif
		{
			///< [ptr]: void** for chunk data address [index]: 0 for bank, 1 for program  @see AudioEffect::getChunk
			datasize=ptrPlug->dispatcher(ptrPlug,effGetChunk,0,0,&chunkdata, 0);
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_GetChunk;
		}
#endif

		if(datasize && chunkdata)
		{
			if(chunkdata_buffer=new char[datasize])
				memcpy(chunkdata_buffer,chunkdata,chunksize_buffer=datasize);
		}
	}
}

void VSTPlugin::CopyChunkData(AudioObject *to)
{
	VSTPlugin *vstto=(VSTPlugin *)to;

	// Copy Data
	if(CanChunk()==true && vstto->CanChunk()==true)
	{
		VstIntPtr datasize=0;
		void *chunkdata=0;

#ifdef TRYCATCH
		try
#endif
		{
			///< [ptr]: void** for chunk data address [index]: 0 for bank, 1 for program  @see AudioEffect::getChunk
			datasize=ptrPlug->dispatcher(ptrPlug,effGetChunk,0,0,&chunkdata, 0);
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_GetChunk;
		}
#endif

		if(datasize && chunkdata)
		{
#ifdef TRYCATCH
			try
#endif
			{
				vstto->ptrPlug->dispatcher(vstto->ptrPlug,effSetChunk,0,datasize,chunkdata, 0);
			}

#ifdef TRYCATCH
			catch(...)
			{
				vstto->crashed=VSTCRASH_GetChunk;
			}
#endif

		}
	}
}

AudioObject *VSTPlugin::CloneEffect(int iflag,Seq_Song *s)
{
	if(ins<0 || outs<0 || ins>MAXVSTIO || outs>MAXVSTIO)
	{
		MessageBeep(-1);
		return 0;
	}

	VSTPlugin *newplug = new VSTPlugin;
	if(!newplug)return 0;

	newplug->song=s;
	newplug->fromobject=this;
	newplug->dllname=mainvar->GenerateString(dllname);
	newplug->fulldllname=mainvar->GenerateString(fulldllname);

	// Copy Flags
	CopySettings(newplug);

	if(iflag&CREATENEW_PLUGIN)
	{
		newplug->InitOpenEffect();

		if(ptrPlug)
		{
			CopyChunkData(newplug);
		}
		else
			if(chunkdata_buffer && chunksize_buffer && CanChunk()==true)
			{
#ifdef TRYCATCH
				try
#endif
				{
					newplug->ptrPlug->dispatcher(newplug->ptrPlug,effSetChunk,0,chunksize_buffer,chunkdata_buffer, 0);
				}

#ifdef TRYCATCH
				catch(...)
				{
				}
#endif

			}

	}
	else
		if(ptrPlug)
		{
			if(CanChunk()==true)
			{
				// Buffer Data

				VstIntPtr datasize=0;
				void *chunkdata=0;

#ifdef TRYCATCH
				try
#endif
				{
					datasize=ptrPlug->dispatcher(ptrPlug,effGetChunk,0,0,&chunkdata, 0);
				}

#ifdef TRYCATCH
				catch(...)
				{
					crashed=VSTCRASH_GetChunk;
				}
#endif


				if(datasize && chunkdata)
				{
					if(newplug->chunkdata_buffer=new char[datasize])
						memcpy(newplug->chunkdata_buffer,chunkdata,newplug->chunksize_buffer=datasize);
				}
			}
		}
		else
		{
			if(chunkdata_buffer && chunksize_buffer)
			{
				if(newplug->chunkdata_buffer=new char[chunksize_buffer])
					memcpy(newplug->chunkdata_buffer,chunkdata_buffer,newplug->chunksize_buffer=chunksize_buffer);
			}
		}

		return newplug;
}


void VSTPlugin::SetVSTEventData(VstMidiEvent *vst,int offset,UBYTE status,char b1,char b2,char velocityoff,LONGLONG noteLength_samples,LONGLONG noteOffset_samples,int iflag)
{
	if(crashed)return;

	vst->type=kVstMidiType;
	vst->byteSize=sizeof(VstMidiEvent);
	vst->deltaFrames=offset; // Block Size Correction

	if(iflag&TRIGGER_THRUEVENT)
		vst->flags=kVstMidiEventIsRealtime; // Prio Flag
	else
		vst->flags=0;

	vst->noteLength=(VstInt32)noteLength_samples;
	vst->noteOffset=(VstInt32)noteOffset_samples;

	vst->midiData[0]=status;
	vst->midiData[1]=b1;
	vst->midiData[2]=b2;
	vst->midiData[3]=0;

#ifdef DEBUG
	if((status&0xF0)==NOTEON)
	{
		if(b2==0)
			noteoffcounter++;
		else
			noteoncounter++;
	}

	if((status&0xF0)==NOTEOFF)
		noteoffcounter++;
#endif

	vst->detune=0;
	vst->noteOffVelocity=velocityoff;

	vst->reserved1=0;
	vst->reserved2=0;

	// Monitor
	if(!(iflag&TRIGGER_MASTEREVENT))
	{
		bool noteoff;

		if( (status&0xF0)==NOTEOFF || ((status&0xF0)==NOTEON && b2==0))
			noteoff=true;
		else
			noteoff=false;

		if(noteoff==false || mainsettings->displaynoteoff_monitor==true)
		{
			monitor_events[monitor_eventcounter].SetData(status,b1,b2);
			monitor_eventcounter==MAXMONITOREVENTS-1?monitor_eventcounter=0:monitor_eventcounter++;
		}
	}
}

void VSTPlugin::SetVSTSysExEventData(VstMidiSysexEvent *vst,int offset,UBYTE *data,int datalength)
{
	if(crashed)return;

	vst->type=kVstSysExType;
	vst->byteSize=sizeof(VstMidiSysexEvent);
	vst->deltaFrames=offset; // Block Size Correction

	vst->sysexDump=(char *)data;
	vst->dumpBytes=datalength;

	vst->flags=0;
	vst->resvd1=0;
	vst->resvd2=0;
}

bool VSTPlugin::Do_TriggerSysEx(int offset,UBYTE *data,int datalength)
{
	LockTrigger();

	if((intern_flag&VST_RECEIVEVstMidiEventS) && vstsysexeventarray && sysextriggerevents<MAXVSTRIGGEREVENTS) // For SysEx no Buffer
	{
		SetVSTSysExEventData((VstMidiSysexEvent *)vstsysexeventarray->events[sysextriggerevents++],offset,data,datalength);
	}

	UnlockTrigger();

	return false;
}

bool VSTPlugin::Do_TriggerEvent(int offset,UBYTE status,char b1,char b2,char velocityoff,int iflag,LONGLONG noteLength_samples,LONGLONG noteOffset_samples) //v
{
	//if(!(flag&TRIGGER_NOLOCK))
	if(crashed)return false;

	// Offset Check
#ifdef DEBUG
	if(offset<0 || offset>=setSize)
		maingui->MessageBoxOk(0,"Do_TriggerEvent Offset");
#endif

	if(offset<0)
		offset=0;
	else
		if(offset>=setSize)
			offset=setSize-1;

	LockTrigger();

	if(int c=vstoverflowbuffer.GetCount()) // Old Overflow Events
	{
		if(triggerevents+c<MAXVSTRIGGEREVENTS)
		{
			VSTOverFlowBuffer *vob=(VSTOverFlowBuffer *)vstoverflowbuffer.GetRoot();

			while(vob)
			{
				SetVSTEventData((VstMidiEvent *)vsteventarray->events[triggerevents++],0,vob->status,vob->b1,vob->b2,velocityoff,vob->noteLength_samples,vob->noteOffset_samples,vob->flag);
				vob=(VSTOverFlowBuffer *)vstoverflowbuffer.RemoveO(vob);
			}
		}
	}

	if((intern_flag&VST_RECEIVEVstMidiEventS) && vsteventarray)
	{
		if(triggerevents<MAXVSTRIGGEREVENTS)
		{
			SetVSTEventData((VstMidiEvent *)vsteventarray->events[triggerevents++],offset,status,b1,b2,velocityoff,noteLength_samples,noteOffset_samples,iflag);
		}
		else
		{
			// Overflow -> Add To Overflow Stack
			if(vstoverflowbuffer.GetCount()<MAXVSTRIGGEREVENTS*4)
			{
				if(VSTOverFlowBuffer *vob=new VSTOverFlowBuffer)
				{
					vob->offset=offset;
					vob->status=status;
					vob->b1=b1;
					vob->b2=b2;
					vob->flag=iflag;
					vob->noteLength_samples=noteLength_samples;
					vob->noteOffset_samples=noteOffset_samples;

					vstoverflowbuffer.AddEndO(vob);
				}
				else
				{
					UnlockTrigger();
					return false;
				}
			}
		}

		UnlockTrigger();
		return true;
	}

	//if(!(flag&TRIGGER_NOLOCK))
	UnlockTrigger();
	return false;
}

void VSTPlugin::Execute_TriggerEvents() //v
{
	if(crashed)
	{
		sysextriggerevents=triggerevents=0;
		return;
	}

	LockTrigger();

	if(triggerevents) // MIDI Data
	{
		if(vsteventarray)
		{
			/*
			#ifdef DEBUG
			TRACE ("VST Trigger %d \n",triggerevents);

			for(int i=0;i<triggerevents;i++)
			TRACE ("C %d\n Data %d %d %d\n",i,VstMidiEvents[i].MIDIData[0],VstMidiEvents[i].MIDIData[1],VstMidiEvents[i].MIDIData[2]);
			#endif
			*/

			vsteventarray->numEvents=(triggerevents>MAXVSTRIGGEREVENTS)?MAXVSTRIGGEREVENTS:triggerevents;
			vsteventarray->reserved=0;
			triggerevents=0;

			if(mainsettings->plugincheck==Settings::PLUGINCHECK_NOCHECKS)
			{
				ptrPlug->dispatcher(ptrPlug,effProcessEvents,0,0,vsteventarray,0.0f); // Execute Audio Instruments To Buffer
			}
			else
			{
#ifdef TRYCATCH
				try
#endif
				{
					ptrPlug->dispatcher(ptrPlug,effProcessEvents,0,0,vsteventarray,0.0f); // Execute Audio Instruments To Buffer
				}

#ifdef TRYCATCH
				catch(...)
				{
					crashed=VSTCRASH_effProcessEvents;
					UnlockTrigger();
					return;
				}
#endif

			}
		}
		else
			triggerevents=0;
	}

	// SysEx Trigger
	if(sysextriggerevents)
	{
		if(vstsysexeventarray)
		{
			vstsysexeventarray->numEvents=(sysextriggerevents>MAXVSTRIGGEREVENTS)?MAXVSTRIGGEREVENTS:sysextriggerevents;
			vstsysexeventarray->reserved=0;
			sysextriggerevents=0;

			if(mainsettings->plugincheck==Settings::PLUGINCHECK_NOCHECKS)
			{
				ptrPlug->dispatcher(ptrPlug,effProcessEvents,0,0,vstsysexeventarray,0.0f); // Execute Audio Instruments To Buffer
			}
			else
			{
#ifdef TRYCATCH
				try
#endif
				{
					ptrPlug->dispatcher(ptrPlug,effProcessEvents,0,0,vstsysexeventarray,0.0f); // Execute Audio Instruments To Buffer
				}

#ifdef TRYCATCH
				catch(...)
				{
					crashed=VSTCRASH_effProcessEvents;
					UnlockTrigger();
					return;
				}
#endif

			}
		}
		else
			sysextriggerevents=0;
	}

	UnlockTrigger();
}

char *VSTPlugin::GetParmName(int index)
{	
	if(crashed)return 0;

	if(index<numberofparameter)
	{
		memset(vstring,0,kVstMaxParamStrLen);

		// Name

#ifdef TRYCATCH
		try
#endif
		{
			char *r=(char *)ptrPlug->dispatcher(ptrPlug,effGetParamName,index,0,vstring,0.0f);


			TRACE ("P PName %d",index);
			TRACE (" %s \n",vstring);

			return vstring;
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_effGetParamName;
		}
#endif

	}

	return 0;
}

char *VSTPlugin::GetParmTypeValueString(int index)
{
	if(crashed)return 0;

	if(index<numberofparameter)
	{
		memset(vstring,0,kVstMaxParamStrLen);

		// Name
#ifdef TRYCATCH
		try
#endif
		{
			ptrPlug->dispatcher(ptrPlug,effGetParamLabel,index,0,vstring,0.0);

			TRACE ("P effGetParamLabel %d",index);
			TRACE (" %s \n",vstring);

			return vstring;
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_effGetParamName;
			return 0;
		}
#endif


		return vstring;
	}

	return 0;
}

char *VSTPlugin::GetParmValueString(int index)
{
	if(crashed)return 0;

	if(index<numberofparameter)
	{
		memset(vstring,0,kVstMaxParamStrLen);

		// Name
#ifdef TRYCATCH
		try
#endif
		{
			ptrPlug->dispatcher(ptrPlug,effGetParamDisplay,index,0,vstring,0.0);
			return vstring;
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_effGetParamName;
			return 0;
		}
#endif

		return vstring;
	}

	return 0;
}

char *VSTPlugin::GetProgramName()
{
	if(crashed)return 0;

	char programbuffer[kVstMaxProgNameLen+1];

	memset(programbuffer,0,kVstMaxProgNameLen);

#ifdef TRYCATCH
	try
#endif
	{
		// program no in <value>
		ptrPlug->dispatcher(ptrPlug,effGetProgramName,0,0,&programbuffer,0);
	}

#ifdef TRYCATCH
	catch(...)
	{
		crashed=VSTCRASH_effGetProgram;
		return 0;
	}
#endif

	return programbuffer[0]==0?0:mainvar->GenerateString(programbuffer);
}

bool VSTPlugin::CanGetProgramNameIndex()
{
	if(vst_version>=2000)
	{
		return true;
	}

	return false;
}

char *VSTPlugin::GetProgramNameIndex(int index)
{
	if(crashed)return 0;

	if(vst_version>=2000)
	{
		char programbuffer[kVstMaxProgNameLen+1];

		memset(programbuffer,0,kVstMaxProgNameLen);

#ifdef TRYCATCH
		try
#endif
		{
			// program no in <value>
			ptrPlug->dispatcher(ptrPlug,effGetProgramNameIndexed,index,0,&programbuffer,0);
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_effGetProgram;
			return 0;
		}
#endif

		return mainvar->GenerateString(programbuffer);
	}

	return 0;
}

int VSTPlugin::GetProgram()
{
	if(crashed)return 0;

#ifdef TRYCATCH
	try
#endif
	{
		// program no in <value>
		return ptrPlug->dispatcher(ptrPlug,effGetProgram,0,0,0,0.0f);
	}

#ifdef TRYCATCH
	catch(...)
	{
		crashed=VSTCRASH_effGetProgram;
		return 0;
	}
#endif

}

bool VSTPlugin::SetProgram(int prog)
{
	if(crashed)return false;

	// program no in <value>
#ifdef TRYCATCH
	try
#endif
	{
		ptrPlug->dispatcher(ptrPlug,effSetProgram,0,prog,0,0.0f);
	}

#ifdef TRYCATCH
	catch(...)
	{
		crashed=VSTCRASH_effSetProgram;
	}
#endif

	return true;
}

void VSTPlugin::GetIOConfig()
{
	if(crashed)return;

	if(vst_version>=2000)
	{
		//get I/O configuration for synth plugins - they will declare their
		//own output and input channels

		TRACE ("GetIOConfig %s VST Version %d Ins %d Outs %d \n",effectname,vst_version,ptrPlug->numInputs,ptrPlug->numOutputs);

		for (int i=0; i<ptrPlug->numInputs;i++)
		{
			//input pin
			VstPinProperties temp;

			temp.flags=0;
			temp.label[0]=0;
			temp.shortLabel[0]=0;
			temp.arrangementType=kSpeakerArrUserDefined-1;

			TRACE ("### Input Pin %d ###\n",i+1);

			VstIntPtr r=0;

#ifdef TRYCATCH
			try
#endif
			{
				r=ptrPlug->dispatcher(ptrPlug,effGetInputProperties,i,0,&temp,0.0f);
			}

#ifdef TRYCATCH
			catch(...)
			{
				crashed=VSTCRASH_GetInputProperties;
				return;
			}
#endif

			if (r==1)
			{
				TRACE (" Label %s Short Label %s\n",temp.label,temp.shortLabel);

				if(temp.flags&kVstPinUseSpeaker)
				{
					TRACE (" Pin use Speaker \n");
				}
				else
					TRACE (" Pin use no Speaker \n");

				if (temp.flags & kVstPinIsActive)
				{
					TRACE (" Pin is Active\n",i);
				}
				else
				{
					TRACE (" Pin is InActive !!! ??? \n",i);
				}

				if (temp.flags & kVstPinIsStereo)
				{
					// is index even or zero?
					if ((i-ptrPlug->numOutputs)%2==0 || (i-ptrPlug->numOutputs)==0)
					{
						TRACE (" Pin is left channel of a stereo pair \n");
					}
					else
					{
						TRACE (" Pin is right channel of a stereo pair \n");
					}
				}
				else
				{
					TRACE (" Pin is Mono \n",i);
				}

				TRACE (" ArrType:");

				switch(temp.arrangementType)
				{
					//-------------------------------------------------------------------------------------------------------
				case kSpeakerArrUserDefined: TRACE ("user defined \n"); break;
				case kSpeakerArrEmpty: TRACE ("empty arrangement \n"); break;		///< empty arrangement
				case kSpeakerArrMono: TRACE ("M \n"); break;	///< M
				case kSpeakerArrStereo: TRACE ("L R \n"); break;		///< L R
				case kSpeakerArrStereoSurround: TRACE ("Ls Rs \n"); break;	///< Ls Rs
				case kSpeakerArrStereoCenter: TRACE ("Lc Rc \n"); break;	///< Lc Rc
				case kSpeakerArrStereoSide: TRACE ("Sl Sr \n"); break;		///< Sl Sr
				case kSpeakerArrStereoCLfe: TRACE ("C Lfe \n"); break;	///< C Lfe
				case kSpeakerArr30Cine: TRACE ("L R C \n"); break;		///< L R C
				case kSpeakerArr30Music: TRACE ("L R S \n"); break;		///< L R S
				case kSpeakerArr31Cine: TRACE ("L R C Lfe \n"); break;		///< L R C Lfe
				case kSpeakerArr31Music: TRACE (" L R Lfe S \n"); break;			///< L R Lfe S
				case kSpeakerArr40Cine: TRACE ("L R C   S (LCRS) \n"); break;		///< L R C   S (LCRS)
				case kSpeakerArr40Music: TRACE ("L R Ls  Rs (Quadro) \n"); break;			///< L R Ls  Rs (Quadro)
				case kSpeakerArr41Cine: TRACE ("L R C   Lfe S (LCRS+Lfe) \n"); break;		///< L R C   Lfe S (LCRS+Lfe)
				case kSpeakerArr41Music: TRACE ("L R Lfe Ls Rs (Quadro+Lfe) \n"); break;			///< L R Lfe Ls Rs (Quadro+Lfe)
				case kSpeakerArr50: TRACE ("L R C Ls  Rs  \n"); break;			///< L R C Ls  Rs 
				case kSpeakerArr51: TRACE ("L R C Lfe Ls Rs \n"); break;			///< L R C Lfe Ls Rs
				case kSpeakerArr60Cine: TRACE ("L R C   Ls  Rs Cs \n"); break;	///< L R C   Ls  Rs Cs
				case kSpeakerArr60Music: TRACE ("L R Ls  Rs  Sl Sr  \n"); break;		///< L R Ls  Rs  Sl Sr 
				case kSpeakerArr61Cine: TRACE ("L R C   Lfe Ls Rs Cs \n"); break;		///< L R C   Lfe Ls Rs Cs
				case kSpeakerArr61Music: TRACE ("L R Lfe Ls  Rs Sl Sr \n"); break;		///< L R Lfe Ls  Rs Sl Sr 
				case kSpeakerArr70Cine: TRACE ("L R C Ls  Rs Lc Rc \n"); break;	///< L R C Ls  Rs Lc Rc 
				case kSpeakerArr70Music: TRACE ("L R C Ls  Rs Sl Sr\n"); break;		///< L R C Ls  Rs Sl Sr
				case kSpeakerArr71Cine: TRACE (" L R C Lfe Ls Rs Lc Rc \n"); break;		///< L R C Lfe Ls Rs Lc Rc
				case kSpeakerArr71Music: TRACE ("L R C Lfe Ls Rs Sl Sr \n"); break;		///< L R C Lfe Ls Rs Sl Sr
				case kSpeakerArr80Cine: TRACE ("L R C Ls  Rs Lc Rc Cs \n"); break;		///< L R C Ls  Rs Lc Rc Cs
				case kSpeakerArr80Music: TRACE ("L R C Ls  Rs Cs Sl Sr \n"); break;		///< L R C Ls  Rs Cs Sl Sr
				case kSpeakerArr81Cine: TRACE ("L R C Lfe Ls Rs Lc Rc Cs \n"); break;		///< L R C Lfe Ls Rs Lc Rc Cs
				case kSpeakerArr81Music: TRACE (" L R C Lfe Ls Rs Cs Sl Sr \n"); break;		///< L R C Lfe Ls Rs Cs Sl Sr 
				case kSpeakerArr102: TRACE ("L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2 \n"); break;		///< L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2
				default:TRACE ("No arrangementType\n");
				}

				TRACE ("<<< End In Pin %d\n",i+1);
			}
			else
				TRACE ("no effGetInputProperties\n");

		}//for

		for (int i=0;i<ptrPlug->numOutputs;i++)
		{
			//output pin
			VstPinProperties temp;

			temp.flags=0;
			temp.label[0]=0;
			temp.shortLabel[0]=0;
			temp.arrangementType=kSpeakerArrUserDefined-1;

			TRACE ("### Output Pin %d ###\n",i+1);

			VstIntPtr r=0;

#ifdef TRYCATCH
			try
#endif
			{
				r=ptrPlug->dispatcher(ptrPlug,effGetOutputProperties,i,0,&temp,0.0f);
			}

#ifdef TRYCATCH
			catch(...)
			{
				crashed=VSTCRASH_GetOutputProperties;
				return;
			}
#endif

			if(r==1)
			{
				TRACE (" Label %s # Short Label %s\n",temp.label,temp.shortLabel);

				if(temp.flags&kVstPinUseSpeaker)
				{
					TRACE (" Pin use Speaker \n");
				}
				else
					TRACE (" Pin use no Speaker \n");


				if (temp.flags & kVstPinIsActive)
				{
					TRACE (" Pin is Active\n",i);
				}
				else
				{
					TRACE (" Pin is InActive !!! ??? \n",i);
				}

				if (temp.flags & kVstPinIsStereo)
				{
					// is index even or zero?
					if ((i-ptrPlug->numOutputs)%2==0 || (i-ptrPlug->numOutputs)==0)
					{
						TRACE (" Pin is left channel of a stereo pair \n");
					}
					else
					{
						TRACE (" Pin is right channel of a stereo pair \n");
					}
				}
				else
				{
					TRACE (" Pin is Mono \n",i);
				}

				TRACE (" ArrType:");

				switch(temp.arrangementType)
				{
					//-------------------------------------------------------------------------------------------------------
				case kSpeakerArrUserDefined: TRACE ("user defined \n"); break;
				case kSpeakerArrEmpty: TRACE ("empty arrangement \n"); break;		///< empty arrangement
				case kSpeakerArrMono: TRACE ("M \n"); break;	///< M
				case kSpeakerArrStereo: TRACE ("L R \n"); break;		///< L R
				case kSpeakerArrStereoSurround: TRACE ("Ls Rs \n"); break;	///< Ls Rs
				case kSpeakerArrStereoCenter: TRACE ("Lc Rc \n"); break;	///< Lc Rc
				case kSpeakerArrStereoSide: TRACE ("Sl Sr \n"); break;		///< Sl Sr
				case kSpeakerArrStereoCLfe: TRACE ("C Lfe \n"); break;	///< C Lfe
				case kSpeakerArr30Cine: TRACE ("L R C \n"); break;		///< L R C
				case kSpeakerArr30Music: TRACE ("L R S \n"); break;		///< L R S
				case kSpeakerArr31Cine: TRACE ("L R C Lfe \n"); break;		///< L R C Lfe
				case kSpeakerArr31Music: TRACE (" L R Lfe S \n"); break;			///< L R Lfe S
				case kSpeakerArr40Cine: TRACE ("L R C   S (LCRS) \n"); break;		///< L R C   S (LCRS)
				case kSpeakerArr40Music: TRACE ("L R Ls  Rs (Quadro) \n"); break;			///< L R Ls  Rs (Quadro)
				case kSpeakerArr41Cine: TRACE ("L R C   Lfe S (LCRS+Lfe) \n"); break;		///< L R C   Lfe S (LCRS+Lfe)
				case kSpeakerArr41Music: TRACE ("L R Lfe Ls Rs (Quadro+Lfe) \n"); break;			///< L R Lfe Ls Rs (Quadro+Lfe)
				case kSpeakerArr50: TRACE ("L R C Ls  Rs  \n"); break;			///< L R C Ls  Rs 
				case kSpeakerArr51: TRACE ("L R C Lfe Ls Rs \n"); break;			///< L R C Lfe Ls Rs
				case kSpeakerArr60Cine: TRACE ("L R C   Ls  Rs Cs \n"); break;	///< L R C   Ls  Rs Cs
				case kSpeakerArr60Music: TRACE ("L R Ls  Rs  Sl Sr  \n"); break;		///< L R Ls  Rs  Sl Sr 
				case kSpeakerArr61Cine: TRACE ("L R C   Lfe Ls Rs Cs \n"); break;		///< L R C   Lfe Ls Rs Cs
				case kSpeakerArr61Music: TRACE ("L R Lfe Ls  Rs Sl Sr \n"); break;		///< L R Lfe Ls  Rs Sl Sr 
				case kSpeakerArr70Cine: TRACE ("L R C Ls  Rs Lc Rc \n"); break;	///< L R C Ls  Rs Lc Rc 
				case kSpeakerArr70Music: TRACE ("L R C Ls  Rs Sl Sr\n"); break;		///< L R C Ls  Rs Sl Sr
				case kSpeakerArr71Cine: TRACE (" L R C Lfe Ls Rs Lc Rc \n"); break;		///< L R C Lfe Ls Rs Lc Rc
				case kSpeakerArr71Music: TRACE ("L R C Lfe Ls Rs Sl Sr \n"); break;		///< L R C Lfe Ls Rs Sl Sr
				case kSpeakerArr80Cine: TRACE ("L R C Ls  Rs Lc Rc Cs \n"); break;		///< L R C Ls  Rs Lc Rc Cs
				case kSpeakerArr80Music: TRACE ("L R C Ls  Rs Cs Sl Sr \n"); break;		///< L R C Ls  Rs Cs Sl Sr
				case kSpeakerArr81Cine: TRACE ("L R C Lfe Ls Rs Lc Rc Cs \n"); break;		///< L R C Lfe Ls Rs Lc Rc Cs
				case kSpeakerArr81Music: TRACE (" L R C Lfe Ls Rs Cs Sl Sr \n"); break;		///< L R C Lfe Ls Rs Cs Sl Sr 
				case kSpeakerArr102: TRACE ("L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2 \n"); break;		///< L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2
				default:TRACE ("No arrangementType\n");
				}

				TRACE ("<<< End Out Pin %d\n",i+1);
			}
			else
				TRACE ("no effGetOutputProperties\n");
		}//for

		TRACE ("End GetIOConfig %s\n",effectname);
	}
}

bool VSTPlugin::GetSpeakerArrangement()
{
	if(crashed)return false;

	if(vst_version>=2300)
	{
		VstSpeakerArrangement sparr;

		sparr.type=kSpeakerArrStereo;
		sparr.numChannels=2;
		sparr.speakers[0].azimuth=0.0f;
		sparr.speakers[0].elevation=0.0f;
		sparr.speakers[0].radius=0.0f;
		sparr.speakers[0].reserved=0.0f;
		strcpy(sparr.speakers[0].name, "Left");
		sparr.speakers[0].type=kSpeakerL;
		sparr.speakers[1].azimuth=0.0f;
		sparr.speakers[1].elevation=0.0f;
		sparr.speakers[1].radius=0.0f;
		sparr.speakers[1].reserved=0.0f;
		strcpy(sparr.speakers[1].name, "Right");
		sparr.speakers[1].type=kSpeakerR;

#ifdef TRYCATCH
		try
#endif
		{
			ptrPlug->dispatcher(ptrPlug,effGetSpeakerArrangement,0,(VstIntPtr)&sparr,&sparr,0.0f);
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_GetSpeakerArrangement;
			return false;
		}
#endif

		// effect->dispatcher(effect, effGetSpeakerArrangement, 0, (VstIntPtr)&sparr, &sparr, 0);

		VstSpeakerArrangement *input,*output;

		input=(VstSpeakerArrangement *)new char[2*sizeof(VstInt32)+sizeof(VstSpeakerProperties)*ins];
		output=(VstSpeakerArrangement *)new char[2*sizeof(VstInt32)+sizeof(VstSpeakerProperties)*outs];

		if(input && output)
		{
			for(int i=0;i<ins;i++)
			{
				input->speakers[i].name[0]=0;
				input->speakers[i].type=kSpeakerUndefined;
			}

			for(int i=0;i<outs;i++)
			{
				output->speakers[i].name[0]=0;
				output->speakers[i].type=kSpeakerUndefined;
			}

			TRACE ("GetSpeakerArrangement %s\n",effectname);

			input->numChannels=ins;
			input->type=kSpeakerArrUserDefined-1;

			output->numChannels=outs;
			output->type=kSpeakerArrUserDefined-1;

#ifdef TRYCATCH
			try
#endif
			{
				///< [value]: input #VstSpeakerArrangement* [ptr]: output #VstSpeakerArrangement*  @see AudioEffectX::getSpeakerArrangement
				ptrPlug->dispatcher(ptrPlug,effGetSpeakerArrangement,0,(VstIntPtr)input,output,0.0f);
			}

#ifdef TRYCATCH
			catch(...)
			{
				crashed=VSTCRASH_GetSpeakerArrangement;
				return false;
			}
#endif

			TRACE ("### effGetSpeakerArrangement Input ###\n");
			TRACE (" Type:");

			switch(input->type)
			{
				//-------------------------------------------------------------------------------------------------------
			case kSpeakerArrUserDefined: TRACE ("user defined \n"); break;
			case kSpeakerArrEmpty: TRACE ("empty arrangement \n"); break;		///< empty arrangement
			case kSpeakerArrMono: TRACE ("M \n"); break;	///< M
			case kSpeakerArrStereo: TRACE ("L R \n"); break;		///< L R
			case kSpeakerArrStereoSurround: TRACE ("Ls Rs \n"); break;	///< Ls Rs
			case kSpeakerArrStereoCenter: TRACE ("Lc Rc \n"); break;	///< Lc Rc
			case kSpeakerArrStereoSide: TRACE ("Sl Sr \n"); break;		///< Sl Sr
			case kSpeakerArrStereoCLfe: TRACE ("C Lfe \n"); break;	///< C Lfe
			case kSpeakerArr30Cine: TRACE ("L R C \n"); break;		///< L R C
			case kSpeakerArr30Music: TRACE ("L R S \n"); break;		///< L R S
			case kSpeakerArr31Cine: TRACE ("L R C Lfe \n"); break;		///< L R C Lfe
			case kSpeakerArr31Music: TRACE (" L R Lfe S \n"); break;			///< L R Lfe S
			case kSpeakerArr40Cine: TRACE ("L R C   S (LCRS) \n"); break;		///< L R C   S (LCRS)
			case kSpeakerArr40Music: TRACE ("L R Ls  Rs (Quadro) \n"); break;			///< L R Ls  Rs (Quadro)
			case kSpeakerArr41Cine: TRACE ("L R C   Lfe S (LCRS+Lfe) \n"); break;		///< L R C   Lfe S (LCRS+Lfe)
			case kSpeakerArr41Music: TRACE ("L R Lfe Ls Rs (Quadro+Lfe) \n"); break;			///< L R Lfe Ls Rs (Quadro+Lfe)
			case kSpeakerArr50: TRACE ("L R C Ls  Rs  \n"); break;			///< L R C Ls  Rs 
			case kSpeakerArr51: TRACE ("L R C Lfe Ls Rs \n"); break;			///< L R C Lfe Ls Rs
			case kSpeakerArr60Cine: TRACE ("L R C   Ls  Rs Cs \n"); break;	///< L R C   Ls  Rs Cs
			case kSpeakerArr60Music: TRACE ("L R Ls  Rs  Sl Sr  \n"); break;		///< L R Ls  Rs  Sl Sr 
			case kSpeakerArr61Cine: TRACE ("L R C   Lfe Ls Rs Cs \n"); break;		///< L R C   Lfe Ls Rs Cs
			case kSpeakerArr61Music: TRACE ("L R Lfe Ls  Rs Sl Sr \n"); break;		///< L R Lfe Ls  Rs Sl Sr 
			case kSpeakerArr70Cine: TRACE ("L R C Ls  Rs Lc Rc \n"); break;	///< L R C Ls  Rs Lc Rc 
			case kSpeakerArr70Music: TRACE ("L R C Ls  Rs Sl Sr\n"); break;		///< L R C Ls  Rs Sl Sr
			case kSpeakerArr71Cine: TRACE (" L R C Lfe Ls Rs Lc Rc \n"); break;		///< L R C Lfe Ls Rs Lc Rc
			case kSpeakerArr71Music: TRACE ("L R C Lfe Ls Rs Sl Sr \n"); break;		///< L R C Lfe Ls Rs Sl Sr
			case kSpeakerArr80Cine: TRACE ("L R C Ls  Rs Lc Rc Cs \n"); break;		///< L R C Ls  Rs Lc Rc Cs
			case kSpeakerArr80Music: TRACE ("L R C Ls  Rs Cs Sl Sr \n"); break;		///< L R C Ls  Rs Cs Sl Sr
			case kSpeakerArr81Cine: TRACE ("L R C Lfe Ls Rs Lc Rc Cs \n"); break;		///< L R C Lfe Ls Rs Lc Rc Cs
			case kSpeakerArr81Music: TRACE (" L R C Lfe Ls Rs Cs Sl Sr \n"); break;		///< L R C Lfe Ls Rs Cs Sl Sr 
			case kSpeakerArr102: TRACE ("L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2 \n"); break;		///< L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2
			default:TRACE ("No arrangementType\n");
			}

			for(int i=0;i<ins;i++)
			{
				TRACE (" Speaker %d\n",i);
				TRACE ("  Name %s\n",input->speakers[i].name);

				TRACE ("  Speaker Type:");
				switch(input->speakers[i].type)
				{
				case kSpeakerUndefined:TRACE("Undefined \n");break;
				case kSpeakerM:TRACE("Mono (M)");break;
				case kSpeakerL:TRACE("Left (L)");break;
				case kSpeakerR:TRACE("Right (R)");break;
				case kSpeakerC:TRACE("Center (C)");break;
				case kSpeakerLfe:TRACE("Subbass (Lfe)");break;
				case kSpeakerLs:TRACE("Left Surround (Ls)");break;
				case kSpeakerRs:TRACE("Right Surround (Rs)");break;
				case kSpeakerLc:TRACE("Left of Center (Lc)");break;
				case kSpeakerRc:TRACE("Right of Center (Rc)");break;
				case kSpeakerS:TRACE("Surround (S)");break;
					//case kSpeakerCs:TRACE("kSpeakerS");
				case kSpeakerSl:TRACE("Side Left (Sl)");break;
				case kSpeakerSr:TRACE("Side Right (Sr)");break;
				case kSpeakerTm:TRACE("Top Middle (Tm)");break;
				case kSpeakerTfl:TRACE("Top Front Left (Tfl)");break;
				case kSpeakerTfc:TRACE("Top Front Center (Tfc)");break;
				case kSpeakerTfr:TRACE("Top Front Right (Tfr)");break;
				case kSpeakerTrl:TRACE("Top Rear Left (Trl)");break;
				case kSpeakerTrc:TRACE("Top Rear Center (Trc)");break;
				case kSpeakerTrr:TRACE("Top Rear Right (Trr)");break;
				case kSpeakerLfe2:TRACE("Subbass 2 (Lfe2)");break;
				}
			}

			TRACE ("End Inputs\n");

			delete input;

			TRACE ("### effGetSpeakerArrangement Output ###\n");

			TRACE (" Type:");
			switch(output->type)
			{
				//-------------------------------------------------------------------------------------------------------
			case kSpeakerArrUserDefined: TRACE ("user defined \n"); break;
			case kSpeakerArrEmpty: TRACE ("empty arrangement \n"); break;		///< empty arrangement
			case kSpeakerArrMono: TRACE ("M \n"); break;	///< M
			case kSpeakerArrStereo: TRACE ("L R \n"); break;		///< L R
			case kSpeakerArrStereoSurround: TRACE ("Ls Rs \n"); break;	///< Ls Rs
			case kSpeakerArrStereoCenter: TRACE ("Lc Rc \n"); break;	///< Lc Rc
			case kSpeakerArrStereoSide: TRACE ("Sl Sr \n"); break;		///< Sl Sr
			case kSpeakerArrStereoCLfe: TRACE ("C Lfe \n"); break;	///< C Lfe
			case kSpeakerArr30Cine: TRACE ("L R C \n"); break;		///< L R C
			case kSpeakerArr30Music: TRACE ("L R S \n"); break;		///< L R S
			case kSpeakerArr31Cine: TRACE ("L R C Lfe \n"); break;		///< L R C Lfe
			case kSpeakerArr31Music: TRACE (" L R Lfe S \n"); break;			///< L R Lfe S
			case kSpeakerArr40Cine: TRACE ("L R C   S (LCRS) \n"); break;		///< L R C   S (LCRS)
			case kSpeakerArr40Music: TRACE ("L R Ls  Rs (Quadro) \n"); break;			///< L R Ls  Rs (Quadro)
			case kSpeakerArr41Cine: TRACE ("L R C   Lfe S (LCRS+Lfe) \n"); break;		///< L R C   Lfe S (LCRS+Lfe)
			case kSpeakerArr41Music: TRACE ("L R Lfe Ls Rs (Quadro+Lfe) \n"); break;			///< L R Lfe Ls Rs (Quadro+Lfe)
			case kSpeakerArr50: TRACE ("L R C Ls  Rs  \n"); break;			///< L R C Ls  Rs 
			case kSpeakerArr51: TRACE ("L R C Lfe Ls Rs \n"); break;			///< L R C Lfe Ls Rs
			case kSpeakerArr60Cine: TRACE ("L R C   Ls  Rs Cs \n"); break;	///< L R C   Ls  Rs Cs
			case kSpeakerArr60Music: TRACE ("L R Ls  Rs  Sl Sr  \n"); break;		///< L R Ls  Rs  Sl Sr 
			case kSpeakerArr61Cine: TRACE ("L R C   Lfe Ls Rs Cs \n"); break;		///< L R C   Lfe Ls Rs Cs
			case kSpeakerArr61Music: TRACE ("L R Lfe Ls  Rs Sl Sr \n"); break;		///< L R Lfe Ls  Rs Sl Sr 
			case kSpeakerArr70Cine: TRACE ("L R C Ls  Rs Lc Rc \n"); break;	///< L R C Ls  Rs Lc Rc 
			case kSpeakerArr70Music: TRACE ("L R C Ls  Rs Sl Sr\n"); break;		///< L R C Ls  Rs Sl Sr
			case kSpeakerArr71Cine: TRACE (" L R C Lfe Ls Rs Lc Rc \n"); break;		///< L R C Lfe Ls Rs Lc Rc
			case kSpeakerArr71Music: TRACE ("L R C Lfe Ls Rs Sl Sr \n"); break;		///< L R C Lfe Ls Rs Sl Sr
			case kSpeakerArr80Cine: TRACE ("L R C Ls  Rs Lc Rc Cs \n"); break;		///< L R C Ls  Rs Lc Rc Cs
			case kSpeakerArr80Music: TRACE ("L R C Ls  Rs Cs Sl Sr \n"); break;		///< L R C Ls  Rs Cs Sl Sr
			case kSpeakerArr81Cine: TRACE ("L R C Lfe Ls Rs Lc Rc Cs \n"); break;		///< L R C Lfe Ls Rs Lc Rc Cs
			case kSpeakerArr81Music: TRACE (" L R C Lfe Ls Rs Cs Sl Sr \n"); break;		///< L R C Lfe Ls Rs Cs Sl Sr 
			case kSpeakerArr102: TRACE ("L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2 \n"); break;		///< L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2
			default:TRACE ("No arrangementType\n");
			}

			for(int i=0;i<outs;i++)
			{
				TRACE (" Speaker %d\n",i);
				TRACE ("  Name %s\n",output->speakers[i].name);
				TRACE ("  Speaker Type:");

				switch(output->speakers[i].type)
				{
				case kSpeakerUndefined:TRACE("Undefined \n");break;
				case kSpeakerM:TRACE("Mono (M)");break;
				case kSpeakerL:TRACE("Left (L)");break;
				case kSpeakerR:TRACE("Right (R)");break;
				case kSpeakerC:TRACE("Center (C)");break;
				case kSpeakerLfe:TRACE("Subbass (Lfe)");break;
				case kSpeakerLs:TRACE("Left Surround (Ls)");break;
				case kSpeakerRs:TRACE("Right Surround (Rs)");break;
				case kSpeakerLc:TRACE("Left of Center (Lc)");break;
				case kSpeakerRc:TRACE("Right of Center (Rc)");break;
				case kSpeakerS:TRACE("Surround (S)");break;
					//case kSpeakerCs:TRACE("kSpeakerS");
				case kSpeakerSl:TRACE("Side Left (Sl)");break;
				case kSpeakerSr:TRACE("Side Right (Sr)");break;
				case kSpeakerTm:TRACE("Top Middle (Tm)");break;
				case kSpeakerTfl:TRACE("Top Front Left (Tfl)");break;
				case kSpeakerTfc:TRACE("Top Front Center (Tfc)");break;
				case kSpeakerTfr:TRACE("Top Front Right (Tfr)");break;
				case kSpeakerTrl:TRACE("Top Rear Left (Trl)");break;
				case kSpeakerTrc:TRACE("Top Rear Center (Trc)");break;
				case kSpeakerTrr:TRACE("Top Rear Right (Trr)");break;
				case kSpeakerLfe2:TRACE("Subbass 2 (Lfe2)");break;
				}
			}

			delete output;

			TRACE ("End Outputs\n");

			return true;
		}
	}

	return false;
}

double VSTPlugin::GetParm(int index)
{
	if(crashed)return 0;

	if(index<numberofparameter)
	{
#ifdef TRYCATCH
		try
#endif
		{
			//	ptrPlug->dispatcher(ptrPlug,effGetParamName,index,0,parhelppname,0.0f);
			float r=ptrPlug->getParameter(ptrPlug,index);
			return (double)r;
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_getParameter;
		}
#endif

	}

	return 0;
}

bool VSTPlugin::SetParm(int index,double par)
{
	if(crashed)return false;

	if(index<numberofparameter)
	{
		TRACE ("VST SetParm %d %f \n",index,par);

#ifdef TRYCATCH
		try
#endif
		{
			float gp=ptrPlug->getParameter(ptrPlug,index);

			ptrPlug->setParameter(ptrPlug,index,(float)par);

			float gp2=ptrPlug->getParameter(ptrPlug,index);

			if(gp!=gp2)
				return true;

			return false; // Same Value - Automation
		}

#ifdef TRYCATCH
		catch(...)
		{
			crashed=VSTCRASH_setParameter;
		}
#endif

	}
#ifdef DEBUG
	else
		maingui->MessageBoxError(0,"VST SetParm");
#endif

	return false;
}

#ifdef TESTA
bool Save(FILE *file)
{
	kfwrite2(name, 64, 1, file);
	// get size of program chunk
	VstInt32 chunksize=0;
	void* chunkdata=NULL;
	if(params.vst_plugin!=NULL)
		chunksize=params.vst_plugin->dispatcher(params.vst_plugin, effGetChunk, 1, 0, &chunkdata, 0);
	int numbytes=sizeof(vsti_params)+64+sizeof(VstInt32)+chunksize;
	kfwrite2(&numbytes, 1, sizeof(int), file);
	kfwrite2(visname, 64, 1, file);
	kfwrite(&params, 1, sizeof(vsti_params)-4, file);
	kfwrite2(params.dll_path, 512, 1, file);
	kfwrite2(params.dll_filename, 128, 1, file);
	// save program chunk
	LogPrint("VSTi: saving program chunk...");
	kfwrite2(&chunksize, 1, sizeof(VstInt32), file);
	LogPrint("VSTi: %i bytes of data", chunksize);
	if(chunkdata==NULL || chunksize==0)
		LogPrint("VSTi: effGetChunk failed, can't save program");
	else
		kfwrite2(chunkdata, chunksize, 1, file);

	/*              // save all vst parameters
	int numparams=params.vst_plugin->numParams;
	kfwrite2(&numparams, 1, sizeof(int), file);
	for(VstInt32 i=0;i<numparams;i++)
	{
	float value=params.vst_plugin->getParameter(params.vst_plugin, i);
	kfwrite2(&value, 1, sizeof(float), file);
	}*/
	return true;
};

bool Load(FILE *file)
{
	//              LogPrint("VSTi: Load()");
	int numbytes=0;
	kfread2(&numbytes, 1, sizeof(int), file);
	//              LogPrint("VSTi: numbytes=%i", numbytes);
	kfread2(visname, 64, 1, file);
	//              LogPrint("VSTi: visname=\"%s\"", visname);
	// store string pointers before overwriting them from file
	char* dll_path=params.dll_path;
	char* dll_filename=params.dll_filename;
	char* plugin_name=params.plugin_name;
	kfread2(&params, 1, sizeof(vsti_params)-4, file);
	// restore string pointers again
	params.dll_path=dll_path;
	params.dll_filename=dll_filename;
	params.plugin_name=plugin_name;
	// restore vst_info pointer
	params.vst_info=&vst_info;
	//              LogPrint("VSTi: paths...");
	kfread2(params.dll_path, 512, 1, file);
	kfread2(params.dll_filename, 128, 1, file);
	params.self=this;
	//              LogPrint("VSTi: Restore");
	// clear vst pointer and editor info, destroy premod to force update
	params.premod=-1.0f;
	params.curprogram=0;
	params.vst_plugin=NULL;
	params.editor_open=false;
	params.editor_closed=false;
	// load plugin (1. full path  2. last used vst folder  3. registry vst folder  4. file requester  5. NULL)
	//              LogPrint("VSTi: Load plugin");
	bool manual_cancel=false;
	if(!LoadPlugin(params.dll_path))
	{
		char filename[512];
		strcpy(filename, GetCurDir(5));
		//                      strcpy(filename, GetMusagiDir());
		strcat(filename, "\\");
		strcat(filename, params.dll_filename);
		if(!LoadPlugin(filename))
		{
			// should look up system vst dir from registry here before asking user...
			char string[128];
			sprintf(string, "Find %s", params.dll_filename);
			if(GetDUI()->SelectFileLoad(filename, 5, string))
			{
				if(strcmp(filename, params.dll_filename)!=-1) // only load if filename matches the expected plugin
					LoadPlugin(filename);
			}
			else
				manual_cancel=true;
		}
	}
	if(params.vst_plugin==NULL)
	{
		ResetParams();

		if(!manual_cancel)
		{
			char mstring[128];
			sprintf(mstring, "Couldn't load VSTi plugin: \"%s\"", params.dll_filename);
			MessageBox(hWndMain, mstring, "Warning", MB_ICONEXCLAMATION);
		}
		LogPrint("VSTi: Couldn't load \"%s\"", params.dll_filename);
	}
	else
	{
		// set correct program before loading preset to avoid override
		params.curprogram=params.program;
		params.vst_plugin->dispatcher(params.vst_plugin, effSetProgram, 0, params.curprogram, 0, 0);
	}

	if(numbytes==sizeof(vsti_params)+64)
	{
		// this was an old version of the instrument without saved program chunk, so stop reading
		return true;
	}

	// load program chunk
	LogPrint("VSTi: loading program chunk...");
	VstInt32 chunksize=0;
	void* chunkdata=NULL;
	kfread2(&chunksize, 1, sizeof(VstInt32), file);
	LogPrint("VSTi: %i bytes of data", chunksize);
	if(chunksize>0)
	{
		char* chunkdata=(char*)malloc(chunksize);
		kfread2(chunkdata, chunksize, 1, file);
		if(params.vst_plugin!=NULL)
			params.vst_plugin->dispatcher(params.vst_plugin, effSetChunk, 1, chunksize, chunkdata, 0);
		free(chunkdata);
	}
d
	/*                      // load all vst parameters
	int numparams=0;
	kfread2(&numparams, 1, sizeof(int), file);
	for(VstInt32 i=0;i<numparams;i++)
	{
	float value=0.0f;
	kfread2(&value, 1, sizeof(float), file);
	params.vst_plugin->setParameter(params.vst_plugin, i, value);
	}*/
	return true;
};
};
#endif
