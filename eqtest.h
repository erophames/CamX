#define MAXBANDS 16
#define EQ_MAXDB 18 // +18db <-> -18dB, step 0.5

#ifndef MAXCHANNELSPERCHANNEL
#define MAXCHANNELSPERCHANNEL 2 // default Stereo
#endif

class EQ
{
public:
	EQ()
	{
		bands=10;

		// Hz
		bandfreq[0]=60;
		bandfreq[1]=170;
		bandfreq[2]=310;
		bandfreq[3]=600;
		bandfreq[4]=1000;
		bandfreq[5]=3000;
		bandfreq[6]=6000;
		bandfreq[7]=8000;
		bandfreq[8]=10000;
		bandfreq[9]=12000;

		// Default
		for(int i=0;i<bands;i++)
			Volume[i]=1;

		mainVol=1;

		Init(48000); // default soundcard Hz
	}

	void EQ_Reset() // call this before audio stream start
	{
			for(int c=0;c<bands*MAXCHANNELSPERCHANNEL;c++)
			   C_ener[c]=0; // reset 
	}

	void Init(int srate) // 44.1 khz, 48khz etc...
	{
		double samplerate =1/srate;

		for(int x=0;x<bands;x++)
		{
			// build band circ
			double freq=bandfreq[x];

			// max Sample Freq=Sample Rate/2

			freq *=(2*PI);
			freq =1/freq;

			double h=samplerate;

			h/=freq;

			C_coef[x]=(ARES)h;
		}

		EQ_Reset();
	}

	void DoEffect(ARES *samples,int buffersize,int channels)
	{
		// [Buffer Channel 1<buffersize>] [Buffer Channel 2<buffersize>] etc..

		ARES u[MAXBANDS]; // max 16 bands

		for(int ch=0;ch<channels;ch++) // Channel Blocks
		{
			int x=buffersize;
			int eqs=ch*bands; // set start bands/channel

			while(x--)// Samples
			{
				ARES uin=*samples; // Input Sample
				int eq=eqs;

				for(int i=0;i<bands;i++,eq++) // Bands
				{
					ARES duc=uin-C_ener[eq]; // diff prev samples band rest

					duc*=C_coef[i]; // multi band
					u[i]=C_ener[eq]; // buffer
					uin-=u[i];
					C_ener[eq] +=duc;
				}

				for(int b=0;b<bands;b++) // mix bands
					uin+=u[b]*Volume[b];

				uin*=mainVol; // add main vol

				*samples++ =uin; // write back
			} // for x
		}// for ch
	}

	int bands;
	
	ARES mainVol; //faktor
	ARES C_coef[MAXBANDS];
	ARES Volume[MAXBANDS]; // Faktor
	ARES C_ener[MAXBANDS*MAXCHANNELSPERCHANNEL];

	int bandfreq[MAXBANDS]; //Hz
};
