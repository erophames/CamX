#ifndef CAMX_SEQTIME_H
#define CAMX_SEQTIME_H 1

#include "object.h"
#include "objectevent.h"

#ifdef WIN32
#include <AFXMT.h> // Semaphores
#endif

class AudioDevice;
class Seq_Song;
class Seq_Time;
class guiBitmap;
class guiWindow;
class camxFile;
class TimeString;
class Seq_Pos;

class Seq_Signature:public OStart{

	friend class Seq_Time;

public:
	Seq_Signature(){map=0;}

	Seq_Signature *NextSignature() { return (Seq_Signature *)next;}
	Seq_Signature *PrevSignature(){ return (Seq_Signature *)prev;}

	void SetDNValue();

	void SetSignature(int newnn,OSTART newdntx,bool lock);

	void ChangeSignature(int nn,OSTART dn_tx);
	OSTART GetSignatureStart(){return ostart;}
	bool Compare(Seq_Signature *);
	void CopyData(Seq_Signature *);
	void Load(camxFile *);
	void Save(camxFile *);
	char *GetTickString();

	Seq_Time *map;
	OSTART measurelength,dn_ticks;
	int sig_measure,nn,dn; // numerator+denumerato
};

enum{
	TEMPOREFRESH_NOGUI=1,
	TEMPOREFRESH_NOLOOPREFRESH=2
};

class Seq_Tempo:public OStart{

	friend class Seq_Time;

public:
	
	void CloneData(Seq_Tempo *);
	void Delete(bool full){delete this;}
	void Load(camxFile *);
	void Save(camxFile *);

	Seq_Tempo *NextTempo() {return (Seq_Tempo *)next;}
	Seq_Tempo *PrevTempo() {return (Seq_Tempo *)prev;}

	OSTART GetTempoStart(){return ostart;}
	bool ChangeTempo(Seq_Song *,OSTART,double t=0,int iflag=0); // tempo 0=no tempo change
	
	Seq_Time *map;
	double tempo,tempofactor; // Ratio to 120 BPM <- song ->
	int type; // real or virtual
};

class Seq_Time{	

	friend class Seq_Signature;
	friend class Seq_Tempo;
	friend class Undo_DeleteTempos;
	friend class Undo_CreateTempo;
	friend class Undo_EditTempos;
	friend class EditFunctions;
	friend class Undo_ChangeTempoMap;
	friend class Seq_Song;

public:
	enum{
		AUDIOSTREAMREFRESH=1
	};
	enum{
		LOOP_REFRESH=1
	};

	enum{
		TIMESIMPLE=1
	};

	Seq_Time (); // Creates 1 starttempo and 1 startsignature !

	void Close(){tempomap.Close();}
	void EditSignature(guiWindow *,OSTART time);
	void EditSignature(guiWindow *,Seq_Signature *);

	void DeleteSignature(guiWindow *,Seq_Signature *);
	void CreateSignatureAndEdit(guiWindow *,OSTART time);
	void CreateTimeString(TimeString *,OSTART time,int format,int flag=0);
	void CreateLengthString(TimeString *,OSTART time,OSTART length,int format);
	void ConvertLengthToPos(Seq_Pos *,OSTART from,OSTART ticks);

	double ConvertTempoTicksToTicks(OSTART startticks,double ticks,Seq_Tempo **); // Main Ticks->Tempo
	OSTART ConvertTempoTicksToTicks(OSTART startticks,double ticks); // Main Ticks->Tempo
	double ConvertTempoTicksToTicks(double ticks); // Main Ticks->Tempo StartPosition 0

	double SubTicksToTempoTicks(OSTART startticks,double ticks); // Sub Tempo
	double SubTicksToTempoTicks(double ticks); // Sub Tempo

	OSTART ConvertPosToTicks(Seq_Pos *);
	
	void ConvertTicksToPos(OSTART time,Seq_Pos *,OSTART zoomticks=0); //{ConvertTicksToPosition(time,pos,true);}
	void ConvertTicksToLength(OSTART from,OSTART length,Seq_Pos *);
	OSTART ConvertPosToLength(OSTART from,Seq_Pos *);
	
	int ConvertTicksToMeasure(OSTART);
	OSTART ConvertMeasureToTicks(OSTART);
	OSTART ConvertTicksToNextBeatTicks(OSTART);
	OSTART ConvertTicksToNextMeasureTicks(OSTART);
	OSTART ConvertTicksToMeasureTicks(OSTART,bool up); // quantize to end/start
	OSTART ConvertTicksToBeatTicks(OSTART,bool up); // quantize to end/start
	OSTART ConvertTicksQuantizeTicks(OSTART,OSTART qticks);
	OSTART ConvertTicksLeftQuantizeTicks(OSTART,OSTART qticks);
	OSTART ConvertTicksToFrameTicks(OSTART);
	OSTART ConvertTicksToQFrameTicks(OSTART);

	OSTART ConvertSamplesToTempoTicks(OSTART start,LONGLONG samples){return ConvertTempoTicksToTicks(start,(double)samples/ppqsampleratemul);}
	OSTART ConvertTicksToNextMs(OSTART time,double sec,bool forceup=false);
	OSTART ConvertTicksToNextZoomTicks(OSTART time,OSTART zoomticks);

	LONGLONG ConvertSamplesToNextSamples(LONGLONG samplepos,LONGLONG samples);

	Seq_Tempo *AddNewTempo(int type,OSTART start,double tempo,bool repairloops=false);
	Seq_Tempo *RemoveTempo(Seq_Tempo *);
	double CleanTempoValue(double);

	int GetCountOfTempos(){return tempomap.GetCount();}
	Seq_Tempo *FirstTempo() {return (Seq_Tempo *)tempomap.GetRoot();}
	Seq_Tempo *GetTempoIndex(int index){return (Seq_Tempo *)tempomap.GetO(index);}
	int GetTempoCount(){return tempomap.GetCount();}
	Seq_Tempo *LastTempo() {return (Seq_Tempo *)tempomap.Getc_end();}
	Seq_Tempo *FirstSelectedTempo();
	void CloneTempos(OListStart *);

	double GetLowestTempo();
	double GetHighestTempo();
	void RepairTempomap();

	void OpenPRepairTempoSelection();

	int GetSelectedTempos();
	LONGLONG ConvertTicksToTempoSamplesStart(OSTART startticks,double ticks);
	LONGLONG ConvertTicksToTempoSamplesStart(OSTART startticks,OSTART ticks);

	LONGLONG ConvertTicksToTempoSamples(OSTART ticks);
	LONGLONG ConvertTicksToTempoSamples(double ticks);

	inline double ConvertSamplesToTicks(LONGLONG s){return ConvertTempoTicksToTicks((double)s/ppqsampleratemul);}
	OSTART ConvertSamplesToOSTART(LONGLONG);
	void ConvertSamplesToOSTART(LONGLONG s1,OSTART *o1,LONGLONG s2,OSTART *o2);

	double AddTempoToTicks(OSTART startticks,double ticks);

	void Load(camxFile *);
	void Save(camxFile *);

	void LoadTempoMap(camxFile *);
	void SaveTempoMap(camxFile *);
	void RefreshTempoFactor(bool refreshfull=true);
	void RefreshTempoChanges();
	inline Seq_Tempo *GetTempo(OSTART ticks){return (Seq_Tempo *)tempomap.FindOBefore(ticks);}

	Seq_Signature *FindSignatureBefore(OSTART ticks){return (Seq_Signature *)signaturemap.FindOBefore(ticks);}
	Seq_Signature *FindSignature(OSTART ticks){return (Seq_Signature *)signaturemap.FindObject(ticks);}
	Seq_Signature *FindSignature_Measure(int);
	Seq_Signature *AddNewSignature(int measure,int numerator,OSTART dnumerator_ticks);
	Seq_Signature *RemoveSignature(Seq_Signature *);
	
	void SetNumerator(OSTART pos,int num);
	void SetNumerator(Seq_Signature *,int num);

	void SetDeNumerator(OSTART pos,OSTART dnum);
	void SetDeNumerator(Seq_Signature *,OSTART dnum);

	Seq_Signature *FirstSignature() {return (Seq_Signature *)signaturemap.GetRoot();}
	Seq_Signature *LastSignature() {return (Seq_Signature *)signaturemap.Getc_end();}

	void RemoveTempoMap(bool full);
	void RemoveSignatureMap(bool full);
	void RemoveAllTimeMaps(bool full);
	void SelectAllTempos(bool select);
	void SelectTempo(Seq_Tempo *,bool select);

	Seq_Song *song;
	Seq_Tempo *lastselectedtempo;
	OSTART zoomticks;

	double ppqsampleratemul;
	int flag,refreshflag;
	bool newMIDIclocktempo_record; // info flag new MIDI clock tempo

	void LockTimeTrack(){sema_lock.Lock();} // VST Plugins etc..
	void UnlockTimeTrack(){sema_lock.Unlock();}

private:
	void AddeptSignaturesChangesToPositions();
	void RefreshSignatureMeasures();

	CCriticalSection sema_lock;
	OListStart tempomap,signaturemap;
};
#endif
