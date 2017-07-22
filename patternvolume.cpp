#include "patternvolume.h"
#include "camxfile.h"
#include "chunks.h"
#include "audiodefines.h"
#include "audiohardwarebuffer.h"
#include "audiohardware.h"
#include "audioproc.h"
#include "audiopattern.h"
#include "gui.h"

#ifdef OLDIE

// simple linear tweening - no easing, no acceleration


Math.linearTween = function (t, b, c, d) {
	return c*t/d + b;
};



// quadratic easing in - accelerating from zero velocity


Math.easeInQuad = function (t, b, c, d) {
	t /= d;
	return c*t*t + b;
};



// quadratic easing out - decelerating to zero velocity


Math.easeOutQuad = function (t, b, c, d) {
	t /= d;
	return -c * t*(t-2) + b;
};




// quadratic easing in/out - acceleration until halfway, then deceleration


Math.easeInOutQuad = function (t, b, c, d) {
	t /= d/2;
	if (t < 1) return c/2*t*t + b;
	t--;
	return -c/2 * (t*(t-2) - 1) + b;
};


// cubic easing in - accelerating from zero velocity


Math.easeInCubic = function (t, b, c, d) {
	t /= d;
	return c*t*t*t + b;
};




// cubic easing out - decelerating to zero velocity


Math.easeOutCubic = function (t, b, c, d) {
	t /= d;
	t--;
	return c*(t*t*t + 1) + b;
};




// cubic easing in/out - acceleration until halfway, then deceleration


Math.easeInOutCubic = function (t, b, c, d) {
	t /= d/2;
	if (t < 1) return c/2*t*t*t + b;
	t -= 2;
	return c/2*(t*t*t + 2) + b;
};



// quartic easing in - accelerating from zero velocity


Math.easeInQuart = function (t, b, c, d) {
	t /= d;
	return c*t*t*t*t + b;
};




// quartic easing out - decelerating to zero velocity


Math.easeOutQuart = function (t, b, c, d) {
	t /= d;
	t--;
	return -c * (t*t*t*t - 1) + b;
};




// quartic easing in/out - acceleration until halfway, then deceleration


Math.easeInOutQuart = function (t, b, c, d) {
	t /= d/2;
	if (t < 1) return c/2*t*t*t*t + b;
	t -= 2;
	return -c/2 * (t*t*t*t - 2) + b;
};


// quintic easing in - accelerating from zero velocity


Math.easeInQuint = function (t, b, c, d) {
	t /= d;
	return c*t*t*t*t*t + b;
};




// quintic easing out - decelerating to zero velocity


Math.easeOutQuint = function (t, b, c, d) {
	t /= d;
	t--;
	return c*(t*t*t*t*t + 1) + b;
};




// quintic easing in/out - acceleration until halfway, then deceleration


Math.easeInOutQuint = function (t, b, c, d) {
	t /= d/2;
	if (t < 1) return c/2*t*t*t*t*t + b;
	t -= 2;
	return c/2*(t*t*t*t*t + 2) + b;
};



// sinusoidal easing in - accelerating from zero velocity


Math.easeInSine = function (t, b, c, d) {
	return -c * Math.cos(t/d * (Math.PI/2)) + c + b;
};




// sinusoidal easing out - decelerating to zero velocity


Math.easeOutSine = function (t, b, c, d) {
	return c * Math.sin(t/d * (Math.PI/2)) + b;
};




// sinusoidal easing in/out - accelerating until halfway, then decelerating


Math.easeInOutSine = function (t, b, c, d) {
	return -c/2 * (Math.cos(Math.PI*t/d) - 1) + b;
};




// exponential easing in - accelerating from zero velocity


Math.easeInExpo = function (t, b, c, d) {
	return c * Math.pow( 2, 10 * (t/d - 1) ) + b;
};




// exponential easing out - decelerating to zero velocity


Math.easeOutExpo = function (t, b, c, d) {
	return c * ( -Math.pow( 2, -10 * t/d ) + 1 ) + b;
};




// exponential easing in/out - accelerating until halfway, then decelerating


Math.easeInOutExpo = function (t, b, c, d) {
	t /= d/2;
	if (t < 1) return c/2 * Math.pow( 2, 10 * (t - 1) ) + b;
	t--;
	return c/2 * ( -Math.pow( 2, -10 * t) + 2 ) + b;
};



// circular easing in - accelerating from zero velocity


Math.easeInCirc = function (t, b, c, d) {
	t /= d;
	return -c * (Math.sqrt(1 - t*t) - 1) + b;
};




// circular easing out - decelerating to zero velocity


Math.easeOutCirc = function (t, b, c, d) {
	t /= d;
	t--;
	return c * Math.sqrt(1 - t*t) + b;
};




// circular easing in/out - acceleration until halfway, then deceleration


Math.easeInOutCirc = function (t, b, c, d) {
	t /= d/2;
	if (t < 1) return -c/2 * (Math.sqrt(1 - t*t) - 1) + b;
	t -= 2;
	return c/2 * (Math.sqrt(1 - t*t) + 1) + b;
};



#endif

Seq_Pattern_VolumeCurve::Seq_Pattern_VolumeCurve()
{
	init=false;
	fadeintype=fadeouttype=VC_LINEAR;
	fadeinoutactive=true;
	volumeactive=true;
	dbvolume=0; // 0dB
	editmode=false;
	allsamples=-1;

	fadeinms=0;
	fadeoutms=0;
	fadeinsamples=fadeoutsamples=0;
	d_fadeinsamples=d_fadeoutsamples=0;
}

void Seq_Pattern_VolumeCurve::DoEffect(AudioHardwareBuffer *hwb,LONGLONG sampleposition)
{
	if((volumeactive==false && fadeinoutactive==false) || hwb->channelsused==0)
		return;

	if(fadeinoutactive==false)
	{
		if(GetFactor(sampleposition)==1) // Volume 0 db no Fade In/Out return
			return;
	}

	if(sampleposition>fadeinsamples && (sampleposition+hwb->samplesinbuffer)<fadeoutstart)
	{
		// Inside -- Volume ---
		ARES mul=(ARES)GetFactor(sampleposition);

		if(mul==1) // Volume 0 db return
			return;

		// Full Buffer, same Mul

		ARES *s=hwb->outputbufferARES;
		int i=hwb->samplesinbuffer*hwb->channelsused;

		if(int loop=i/8)
		{
			i-=8*loop;
			do
			{
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;

				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;
				*s++ *=mul;

			}while(--loop);
		}

		while(i--)*s++ *=mul;

		return;
	}

	// Use Curves on Buffer Samples

	switch(hwb->channelsused)
	{
	case 1:
		{
			// Mono
			ARES *s=hwb->outputbufferARES;
			int i=hwb->samplesinbuffer;

			if(int loop=i/8)
			{
				i-=8*loop;
				do
				{
					*s++ *=(ARES)GetFactor(sampleposition++);
					*s++ *=(ARES)GetFactor(sampleposition++);
					*s++ *=(ARES)GetFactor(sampleposition++);
					*s++ *=(ARES)GetFactor(sampleposition++);

					*s++ *=(ARES)GetFactor(sampleposition++);
					*s++ *=(ARES)GetFactor(sampleposition++);
					*s++ *=(ARES)GetFactor(sampleposition++);
					*s++ *=(ARES)GetFactor(sampleposition++);

				}while(--loop);
			}

			while(i--)
				*s++ *=(ARES)GetFactor(sampleposition++);
		}
		break;

	case 2:
		{
			// Stereo 
			int i=hwb->samplesinbuffer;
			ARES *l=hwb->outputbufferARES;
			ARES *r=l;

			r+=i;

			if(int loop=i/8)
			{
				i-=8*loop;
				do
				{
					ARES multi=(ARES)GetFactor(sampleposition++);
					*l++ *=multi;*r++ *=multi;
					multi=(ARES)GetFactor(sampleposition++);
					*l++ *=multi;*r++ *=multi;
					multi=(ARES)GetFactor(sampleposition++);
					*l++ *=multi;*r++ *=multi;
					multi=(ARES)GetFactor(sampleposition++);
					*l++ *=multi;*r++ *=multi;

					multi=(ARES)GetFactor(sampleposition++);
					*l++ *=multi;*r++ *=multi;
					multi=(ARES)GetFactor(sampleposition++);
					*l++ *=multi;*r++ *=multi;
					multi=(ARES)GetFactor(sampleposition++);
					*l++ *=multi;*r++ *=multi;
					multi=(ARES)GetFactor(sampleposition++);
					*l++ *=multi;*r++ *=multi;

				}while(--loop);
			}

			while(i--)
			{
				ARES multi=(ARES)GetFactor(sampleposition++);
				*l++ *=multi;
				*r++ *=multi;
			}
		}
		break;

	default:
		{
			ARES *s=hwb->outputbufferARES;
			int sib=hwb->samplesinbuffer;
			int channels=hwb->channelsused;
			int i=sib;

			while(i--)
			{
				register ARES multi=(ARES)GetFactor(sampleposition++);
				register ARES *sc=s++;

				for(int i=0;i<channels;i++)
				{	
					*sc *=multi;
					sc+=sib;
				}
			}
		}
		break;
	}
}

void Seq_Pattern_VolumeCurve::Lock()
{
	mainaudiostreamproc->Lock();
}

void Seq_Pattern_VolumeCurve::UnLock()
{
	mainaudiostreamproc->Unlock();
}


void Seq_Pattern_VolumeCurve::SetFadeIn(double ms)
{
	if(ms>=0 && fadeinms!=ms)
	{
		LONGLONG nfi=mainaudio->ConvertMsToSamples(ms);

		if(nfi>allsamples)
			return;

		if(nfi>=fadeoutstart)
			return;

		Lock();

		fadeinms=ms;
		fadeinsamples=nfi;
		d_fadeinsamples=nfi;

		UnLock();
	}
}

void Seq_Pattern_VolumeCurve::SetFadeOut(double ms)
{
	if(ms>=0 && fadeoutms!=ms)
	{
		LONGLONG nfo=mainaudio->ConvertMsToSamples(ms);

		if(nfo>allsamples)
			return;

		if(allsamples-nfo<=fadeinsamples)
			return;

		Lock();

		fadeoutms=ms;
		fadeoutstart=allsamples-nfo;
		fadeoutsamples=nfo;
		d_fadeoutsamples=nfo;

		UnLock();
	}
}

void Seq_Pattern_VolumeCurve::InitFadeInOut(double msfadein,double msfadeout,LONGLONG patternsamples)
{
	if(patternsamples==-1)
		patternsamples=allsamples;

	LONGLONG nfi=mainaudio->ConvertMsToSamples(msfadein);
	LONGLONG nfo=mainaudio->ConvertMsToSamples(msfadeout);

	if(nfi>=patternsamples-nfo)
		return;

	Lock();

	fadeinms=msfadein;
	fadeinsamples=nfi;
	d_fadeinsamples=nfi;

	fadeoutms=msfadeout;
	fadeoutstart=patternsamples-nfo;
	fadeoutsamples=nfo;
	d_fadeoutsamples=nfo;

	allsamples=patternsamples;

	UnLock();
}

void Seq_Pattern_VolumeCurve::Clone(Seq_Pattern_VolumeCurve *to)
{
	to->init=init;

	to->allsamples=allsamples;

	to->fadeinms=fadeinms;
	to->fadeinsamples=fadeinsamples;
	to->d_fadeinsamples=d_fadeinsamples;
	to->fadeintype=fadeintype;

	to->fadeoutms=fadeoutms;
	to->fadeoutstart=fadeoutstart;
	to->fadeouttype=fadeouttype;
	to->fadeoutsamples=fadeoutsamples;
	to->d_fadeoutsamples=d_fadeoutsamples;

	to->dbvolume=dbvolume;

	to->fadeinoutactive=fadeinoutactive;
	to->volumeactive=volumeactive;
}

Seq_Pattern_VolumeCurve *Seq_Pattern::GetVolumeCurve()
{
	if(itsaloop==true)
		return mainpattern->GetVolumeCurve();

	if(itsaclone==true)
		return mainclonepattern->GetVolumeCurve();

	return &volumecurve;
}

double Seq_Pattern_VolumeCurve::GetVolume()
{
	double db=dbvolume;

	if(volumeactive==true)
		db=dbvolume;
	else
		db=0; // 0 dB

	db=pow (10,db/20); // db->Factor 10^dB/20

	return db;
}

double Seq_Pattern_VolumeCurve::GetFactor(LONGLONG pos)
{
	double db=dbvolume;

	if(volumeactive==true)
		db=dbvolume;
	else
		db=0; // 0 dB

	db=pow (10,db/20); // db->Factor 10^dB/20

	if(init==true && fadeinoutactive==true)
	{
		if(pos<=fadeinsamples) // Fade In 0- fadeinsamples
		{
			if(fadeinsamples==0) // No FadeIn
			{
				return db;
			}

			double v=db;
			double per=pos;

			per/=d_fadeinsamples; // 0-1

			//fadeintype=VC_LOG1;

#ifdef OLDIE
			switch (_type) {  
			case FADE_LINEAR:  
				{  
					_pct = 1.0 - (_now / _over);  
					break;  
				}  
			case FADE_LOG:  
				{  
					float base = 10; //2.71828182845904523536028747135266249775724709369995;  
					_pct = log10(_over/(_over + (base-1) * _now)) + 1;  
					break;  
				}  
			case FADE_EXP:  
				{  
					float base = 10.0f;  
					_pct = -pow((double)base, (double)((_now * 1.04 - _over)/_over)) + 1.09;  
					break;  
				}  
			}  
#endif

			switch(fadeintype)
			{
			case VC_LOG4:
				break;

			case VC_LOG3:
				break;

			case VC_LOG2:
				break;

			case VC_LOG1:
				{
					v=pow(10,per*20)-1;
					db*=v;
					return v;
				}
				break;

			case VC_LINEAR:
				v*=per;
				return v;
				break;
			}
		}

		if(pos>=fadeoutstart) // Fade Out
		{
			if(fadeoutsamples==0) // No Fade Out
			{
				return db;
			}

			double v=db;
			double per=pos-fadeoutstart;

			per/=d_fadeoutsamples; // 0-1
			per=1-per; // 1-0

			fadeouttype=VC_EXP1;

			switch(fadeouttype)
			{
			case VC_LINEAR:
				v*=per;
				return v;
				break;

			case VC_LOG1:
				{
					double base = 10;

					double _over=d_fadeoutsamples;
					double _now=pos-fadeoutstart;

					double l = log10(_over/(_over + (base-1) * _now)) + 1;

					v*=l;
					return v;
				}
				break;


			case VC_EXP1:  
				{  
					double base = 10.0f; 

					double _over=d_fadeoutsamples;
					double _now=pos-fadeoutstart;

					//double e = -pow((double)base, (double)((_now * 1.04 - _over)/_over)) + 1.09;

					double e = -pow((double)base, (double)((_now *1.04 - _over)/_over))+1.09;

					v*=e;
					return v;
				}  
				break;
			}
		}
	}

	return db;
}

double Seq_Pattern_VolumeCurve::GetFactor(LONGLONG pos,int *ctype)
{
	double db=dbvolume;

	if(volumeactive==true)
		db=dbvolume;
	else
		db=0; // 0 dB

	db=pow (10,db/20); // db->Factor 10^dB/20

	if(init==true && fadeinoutactive==true)
	{
		if(pos<=fadeinsamples) // Fade In 0- fadeinsamples
		{
			*ctype=0;

			if(fadeinsamples==0) // No FadeIn
			{
				return db;
			}

			double v=db;
			double per=pos;

			per/=d_fadeinsamples; // 0-1

			//fadeintype=VC_LOG1;

			switch(fadeintype)
			{
			case VC_LOG4:
				break;

			case VC_LOG3:
				break;

			case VC_LOG2:
				break;

			case VC_LOG1:
				{
					v=pow(10,per*20)-1;

					db*=v;

					return v;
				}
				break;

			case VC_LINEAR:
				v*=per;
				return v;
				break;
			}
		}

		if(pos>=fadeoutstart) // Fade Out
		{
			*ctype=2;

			if(fadeoutsamples==0) // No Fade Out
			{
				return db;
			}

			double v=db;
			double per=pos-fadeoutstart;

			per/=d_fadeoutsamples; // 0-1
			per=1-per; // 1-0

			fadeouttype=VC_EXP1;

			switch(fadeouttype)
			{
			case VC_LINEAR:
				v*=per;
				return v;
				break;

			case VC_LOG1:
				{
					double base = 10; //2.71828182845904523536028747135266249775724709369995;

					double _over=d_fadeoutsamples;
					double _now=pos-fadeoutstart;

					double l = log10(_over/(_over + (base-1) * _now)) + 1;

					//	l/= log10(5.0);

					v*=l;

					return v;
					break;  
				}
				break;

			case VC_EXP1:  
				{  
					double base = 10.0f; 
					double _over=d_fadeoutsamples;
					double _now=pos-fadeoutstart;
					double per=_now/_over;

					//	double e = -pow((double)base, (double)((_now * 1.04 - _over)/_over)) + 1.09;

					// y=x^( (now-over)/over );

					//double e = -pow((double)base, (double)((_now * 1.04 - _over)/_over)) + 1.09;

					double e = -pow((double)base, (double)((_now * 1.04 - _over)/_over))+1.09;

					v*=e;

					return v;
				}  
				break;
			}
		}
	}
	*ctype=1;

	return db;
}

void Seq_Pattern_VolumeCurve::Load(camxFile *file)
{
	file->LoadChunk();
	if(file->GetChunkHeader()==CHUNK_PATTERNVOLUMECURVE)
	{
		file->ChunkFound();

		file->ReadChunk(&fadeinms);
		file->ReadChunk(&fadeinsamples);
		file->ReadChunk(&fadeintype);

		file->ReadChunk(&fadeoutms);
		file->ReadChunk(&fadeoutstart);
		file->ReadChunk(&fadeouttype);
		file->ReadChunk(&init);
		file->ReadChunk(&allsamples);
		file->ReadChunk(&fadeinoutactive);
		file->ReadChunk(&volumeactive);
		file->ReadChunk(&dbvolume);

		InitFadeInOut(fadeinms,fadeoutms);

		file->CloseReadChunk();
	}
}

void Seq_Pattern_VolumeCurve::Save(camxFile *file)
{
	file->OpenChunk(CHUNK_PATTERNVOLUMECURVE);

	file->Save_Chunk(fadeinms);
	file->Save_Chunk(fadeinsamples);
	file->Save_Chunk(fadeintype);

	file->Save_Chunk(fadeoutms);
	file->Save_Chunk(fadeoutstart);
	file->Save_Chunk(fadeouttype);
	file->Save_Chunk(init);
	file->Save_Chunk(allsamples);
	file->Save_Chunk(fadeinoutactive);
	file->Save_Chunk(volumeactive);
	file->Save_Chunk(dbvolume);

	file->CloseChunk();
}

void Seq_Pattern_VolumeCurve::StartEdit()
{
	b_fadeinms=fadeinms;
	b_fadeoutms=fadeoutms;
	b_dbvolume=dbvolume;

	b_fadeintype=fadeintype;
	b_fadeouttype=fadeouttype;

	b_fadeinsamples=fadeinsamples;
	b_fadeoutsamples=fadeoutstart;
}


void Seq_Pattern_VolumeCurve::SetVolume(double nv)
{
	if(nv<=24 && nv>=mainaudio->ConvertFactorToDb(mainaudio->silencefactor) && dbvolume!=nv)
	{
		Lock();

		dbvolume=nv;

		UnLock();
	}
}
