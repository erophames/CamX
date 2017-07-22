#ifndef CAMX_TEXTANDMARKER_H
#define CAMX_TEXTANDMARKER_H 1

#include "object.h"
#include "objectevent.h"
#include "colourrequester.h"

class Seq_Text:public Seq_Event
{
	friend class Seq_TextandMarker;

public:
	Seq_Text(){type=0;string=0;}

	void CloneData(Seq_Song *,Seq_Event *);
	Object *Clone(Seq_Song *);
	bool Compare(Seq_Event *);
	void Delete(bool full){delete this;}

	void Load(camxFile *);
	void Save(camxFile *);
	OSTART GetTextStart(){return ostart;}
	void ChangeText(char *);
	void MoveTextAbs(OSTART pos){GetList()->MoveO(this,pos);}
	Seq_Text *PrevText() {return (Seq_Text *)prev;}
	Seq_Text *NextText() {return (Seq_Text *)next;}
	Seq_Song *GetSong();
	void FreeMemory();

	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject){}

	Seq_TextandMarker *map;
	char *string;
	int type; // Text, Marker
};

class Seq_Marker:public Seq_Event
{
	friend class Seq_TextandMarker;

public:
	enum MarkerTypes{
		MARKERTYPE_SINGLE,
		MARKERTYPE_DOUBLE
	};

	enum MarkerFunc{
		MARKERFUNC_NONE,
		MARKERFUNC_STOPPLAYBACK
	};

	Seq_Marker();
	char *GetString(){return string?string:"?";}
	void CloneData(Seq_Song *,Seq_Event *);
	Object *Clone(Seq_Song *);
	bool Compare(Seq_Event *);
	void Delete(bool full){delete this;}

	void Load(camxFile *);
	void Save(camxFile *);
	OSTART GetMarkerStart(){return ostart;}
	OSTART GetMarkerEnd(){return endposition;}
	void FreeMemory();
	void SetMarkerStart(OSTART npos);
	void SetMarkerEnd(OSTART npos);
	void SetMarkerStartEnd(OSTART s,OSTART e);

	void MoveMarkerAbs(OSTART pos){GetList()->MoveO(this,pos);}
	void SendToAudio(MIDIPattern *frompattern,AudioEffects *,int flag,int offset,AudioObject *dontsendtoaudioobject){}

	Seq_Song *GetSong();

	Seq_Marker *PrevMarker() {return (Seq_Marker *)prev;}
	Seq_Marker *NextMarker() {return (Seq_Marker *)next;}

	void ChangeText(char *);
	char *CreateFromToString();
	char *CreateFromString();

	Colour colour;
	char *string,*fromtostring,*fromstring;
	Seq_TextandMarker *map;
	OSTART endposition;

	int markertype,// Text, Marker
		functionflag;
};

class Seq_TextandMarker
{
public:

	Seq_TextandMarker(){lastselectedtext=0;lastselectedmarker=0;}

	// Text
	Seq_Text *FirstText(){return (Seq_Text *)text.GetRoot();}
	Seq_Text *LastText(){return (Seq_Text *)text.Getc_end();}

	bool ChangeTextPosition(Seq_Text *,OSTART pos);
	Seq_Text *AddText(OSTART pos,char *string,size_t length=-1); // no length=strlen string
	Seq_Text *DeleteText(Seq_Text *);
	Seq_Text *GetTextAtIndex(int ix){return (Seq_Text *)text.GetO(ix);}
	
	int GetCountOfTexts(){return text.GetCount();}
	int GetCountOfSelectedTexts();
	void RemoveAllTexts();

	void Close()
	{
		text.Close();
	marker.Close();
	}

	//Marker
	Seq_Marker *FindMarker(OSTART pos){return (Seq_Marker *)marker.FindOBefore(pos);}

	Seq_Marker *FirstMarker(int type)
	{
		Seq_Marker *m=FirstMarker();

		while(m){
			if(m->markertype==type)return m;
			m=m->NextMarker();
		}

		return 0;
	}

	Seq_Marker *FirstMarker(){return (Seq_Marker *)marker.GetRoot();}
	Seq_Marker *LastMarker(){return (Seq_Marker *)marker.Getc_end();}

	bool ChangeMarkerPosition(Seq_Marker *,OSTART pos);
	Seq_Marker *AddMarker(Seq_Pattern *,bool doublemarker);
	Seq_Marker *AddMarker(OSTART pos,OSTART end,char *,size_t length=-1); // no length=strlen string
	Seq_Marker *DeleteMarker(Seq_Marker *);
	void MoveMarker(Seq_Marker *,OSTART time);
	Seq_Marker *CutMarker(Seq_Marker *);
	void DeleteMarker_CalledByGUI(Seq_Marker *);
	Seq_Marker *GetMarkerAtIndex(int ix){return (Seq_Marker *)marker.GetO(ix);}
	int GetCountOfMarker(){return marker.GetCount();}
	int GetCountOfSelectedMarker();
	void RemoveAllMarker();
	void InitSongStopMarker(OSTART);
	Seq_Marker *FindMarkerID(int id);
	void Init();

	void Load(camxFile *);
	void Save(camxFile *);

	OListStart text,marker;
	Seq_Song *song;
	Seq_Text *lastselectedtext;
	Seq_Marker *lastselectedmarker;
};
#endif
