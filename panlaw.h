#ifndef CAMX_AUDIOPANLAW
#define CAMX_AUDIOPANLAW 1

#include "defines.h"

class camxFile;
class AT_AUDIO_Panorama;

class PanLaw
{
public:
	enum PAN_DB
	{
		PAN_DB_0,
		PAN_DB_3,
		PAN_DB_4_5,
		PAN_DB_6
	};

	PanLaw()
	{
		InitValues();
		SetLaw(PAN_DB_3);
	}

	ARES GetPanValue(){return valuemul[panorama_db];}
	void GetValue(ARES *left,ARES *right,ARES m);

	PanLaw(PanLaw *law,AT_AUDIO_Panorama *p)
	{
		panorama_db=law->panorama_db;
		pano=p;
	}

	void SetLaw(int l)
	{
		panorama_db=l;
	}

	void Load(camxFile *);
	void Save(camxFile *);

	void InitValues();
	
	ARES valuemul[4],values[4][512]; // 0 - 254 M=255 256-512
	AT_AUDIO_Panorama *pano;
	int panorama_db;
};
#endif