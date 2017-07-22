#include "panlaw.h"
#include "camxfile.h"
#include "audiohardware.h"

void PanLaw::Load(camxFile *file)
{
	file->ReadChunk(&panorama_db);
}

void PanLaw::Save(camxFile *file)
{
	file->Save_Chunk(panorama_db);
}

void PanLaw::InitValues()
{
	for(int i=0;i<4;i++)
	{
		ARES db;

		switch(i)
		{
		case PAN_DB_0:
			valuemul[i]=1;
			db=1;
			break;

		case PAN_DB_3:
			valuemul[i]=mainaudio->ConvertDbToFactor(-3);
			db=-3;
			TRACE ("PanLaw Init -3 =%f\n",valuemul[i]);
			break;

		case PAN_DB_4_5:
			valuemul[i]=mainaudio->ConvertDbToFactor(-4.5);
			db=-4.5;
			TRACE ("PanLaw Init -4.5 =%f\n",valuemul[i]);
			break;

		case PAN_DB_6:
			valuemul[i]=mainaudio->ConvertDbToFactor(-6);
			db=-6;
			TRACE ("PanLaw Init -6 =%f\n",valuemul[i]);
			break;
		}

		ARES h=valuemul[i]; // 96 dB Range
		ARES add=valuemul[i];
		add/=256;

		ARES hu=0;

		// 0 - 254 M=255 256-512

		for(int up=0;up<256;up++) // 0-M
		{
			values[i][up]=hu;
			hu+=add;
			//	TRACE ("Mul %f Up %d dB %f = %f\n",mul,up,h,values[i][up]);
		}

		add=1-valuemul[i];
		add/=256;

		//	hu=mul;
		for(int up=256;up<512;up++) // 0-M
		{
			hu+=add;
			values[i][up]=hu;


			//	TRACE ("Mul %f Up2 %d dB %f = %f\n",mul,up,h,values[i][up]);
		}
	}
}

void PanLaw::GetValue(ARES *left,ARES *right,ARES m)
{
	if(m==0.5) // MID
	{
		*left=*right=valuemul[panorama_db];
		return;
	}

	//	m=-1.1;

	// 0 - 254 M=255 256-512

	if(m<=0)
	{
		*right=values[panorama_db][0];
		*left=values[panorama_db][511];
		return;
	}

	if(m<0.5) // <-Left
	{
		m*=512;
		int h=(int)m;
		*right=values[panorama_db][h];
		*left=values[panorama_db][256+h];

		return;
	}

	if(m>=1)
	{
		*right=values[panorama_db][511];
		*left=values[panorama_db][0];
		return;
	}

	// -> Right
	m-=0.5;
	m*=512;
	int h=(int)m;

	*left=values[panorama_db][256-h];
	*right=values[panorama_db][256+h];
}