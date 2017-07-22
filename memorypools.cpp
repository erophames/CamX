#include "defines.h"

#ifdef MEMPOOLS

#include "mempools.h"
#include "songmain.h"
#include "MIDIhardware.h"
#include "audiofile.h"

#include "objectevent.h"
#include "seq_realtime.h"
#include "patternselection.h"

// Events
Note *mainPools::mempGetNote()
{
#ifdef MEMPOOLS

#ifdef WIN32
	lock_notes.Lock();
#endif

	if(!FirstNotePool()) // First Pool
	{
		if(mempool_Note *fp=new mempool_Note(this))
		{
			firstfreenotepool=fp;
			notepool.AddEndO(fp); // else out of memory ?
			fp->notesused[0]=1;
#ifdef WIN32
			lock_notes.Unlock();
#endif
			return fp->notes;						
		}
	}
	else
	{
		mempool_Note *fp=firstfreenotepool;

		while(fp) // out of memory ???
		{
			if(fp->full==false)
				for(int i=0;i<EVENTMEMPOOLSIZE/32;i++){

					if(fp->notesused[i]!=0xFFFFFFFF) // 32 Bit MemPool
					{
						int shift=1;

						for(int b=0;b<32;b++){

							if(!(fp->notesused[i]&shift)){

								fp->notesused[i]|=shift;

								if(i==EVENTMEMPOOLSIZE/32-1 && b==31){

									fp->full=true;
									if(firstfreenotepool==fp)
										firstfreenotepool=fp->NextPool();
								}
								else
									firstfreenotepool=fp;
#ifdef WIN32
								lock_notes.Unlock();
#endif
								return &fp->notes[i*32+b];
							}

							shift<<=1;
						}
					}
				}

				fp=fp->NextPool();
		}

		if(mempool_Note *newpool=new mempool_Note(this))
		{
			notepool.AddEndO(firstfreenotepool=newpool); // else outof memory	
			newpool->notesused[0]=1;
#ifdef WIN32
			lock_notes.Unlock();
#endif
			return newpool->notes;		
		}
	}

#ifdef WIN32
	lock_notes.Unlock();
#endif

	return 0;

#else
	return new Note;
#endif
}

void mainPools::mempDeleteNote(Note *note)
{
#ifdef MEMPOOLS

	note->Init();

#ifdef WIN32
	lock_notes.Lock();
#endif

	firstfreenotepool=FirstNotePool(); // reset free

	size_t bit=EVENTMEMPOOLSIZE-(&note->pool->notes[EVENTMEMPOOLSIZE]-note);

	// Reset Class
	note->pool->full=false;
	note->pool->notesused[bit/32]^=1<<bit;

	for(int c=0;c<EVENTMEMPOOLSIZE/32;c++)
		if(note->pool->notesused[c]){
#ifdef WIN32
			lock_notes.Unlock();
#endif
			return;
		}

		notepool.RemoveO(note->pool);

#ifdef WIN32
		lock_notes.Unlock();
#endif

#else
	delete note;
#endif
}

ControlChange *mainPools::mempGetControl()
{
#ifdef MEMPOOLS

#ifdef WIN32
	lock_control.Lock();
#endif

	if(!FirstControlPool()) // First Pool
	{
		if(mempool_Control *fp=new mempool_Control(this)){
			firstfreecontrolpool=fp;
			controlpool.AddEndO(fp); // else out of memory ?				
			fp->controlused[0]=1;

#ifdef WIN32
			lock_control.Unlock();
#endif
			return fp->controls;	
		}
	}
	else
	{
		mempool_Control *fp=firstfreecontrolpool;

		while(fp) // out of memory ???
		{
			if(fp->full==false)
				for(int i=0;i<EVENTMEMPOOLSIZE/32;i++)
				{
					if(fp->controlused[i]!=0xFFFFFFFF) // 32 Bit MemPool
					{
						int shift=1;

						for(int b=0;b<32;b++){

							if(!(fp->controlused[i]&shift)){

								fp->controlused[i]|=shift;

								if(i==EVENTMEMPOOLSIZE/32-1 && b==31){
									fp->full=true;
									if(firstfreecontrolpool==fp)
										firstfreecontrolpool=fp->NextPool();
								}
								else
									firstfreecontrolpool=fp;

#ifdef WIN32
								lock_control.Unlock();
#endif
								return &fp->controls[i*32+b];
							}

							shift<<=1;
						}
					}
				}

				fp=fp->NextPool();
		}

		if(mempool_Control *newpool=new mempool_Control(this)){

			controlpool.AddEndO(firstfreecontrolpool=newpool); // else outof memory	
			newpool->controlused[0]=1;
#ifdef WIN32
			lock_control.Unlock();
#endif
			return newpool->controls;
		}
	}

#ifdef WIN32
	lock_control.Unlock();
#endif

	return 0;

#else
	return new ControlChange;
#endif
}

void mainPools::mempDeleteControl(ControlChange *cc)
{
#ifdef MEMPOOLS
	cc->Init();

#ifdef WIN32
	lock_control.Lock();
#endif

	firstfreecontrolpool=FirstControlPool(); // reset free

	size_t bit=EVENTMEMPOOLSIZE-(&cc->pool->controls[EVENTMEMPOOLSIZE]-cc);

	// Reset Class
	cc->pool->full=false;
	cc->pool->controlused[bit/32]^=1<<bit;

	for(int c=0;c<EVENTMEMPOOLSIZE/32;c++)
		if(cc->pool->controlused[c]){

#ifdef WIN32
			lock_control.Unlock();
#endif
			return;
		}

		controlpool.RemoveO(cc->pool);

#ifdef WIN32
		lock_control.Unlock();
#endif

#else
	delete cc;
#endif
}

Pitchbend *mainPools::mempGetPitchbend()
{
#ifdef MEMPOOLS

#ifdef WIN32
	lock_pitch.Lock();
#endif

	if(!FirstPitchbendPool()) // First Pool
	{
		if(mempool_Pitchbend *fp=new mempool_Pitchbend(this)){

			firstfreepitchbendpool=fp;
			pitchbendpool.AddEndO(fp); // else out of memory ?				
			fp->pitchused[0]=1;

#ifdef WIN32
			lock_pitch.Unlock();
#endif
			return fp->pitch;
		}
	}
	else
	{
		mempool_Pitchbend *fp=firstfreepitchbendpool;

		while(fp) // out of memory ???
		{
			if(fp->full==false)
				for(int i=0;i<EVENTMEMPOOLSIZE/32;i++)
				{
					if(fp->pitchused[i]!=0xFFFFFFFF) // 32 Bit MemPool
					{
						int shift=1;

						for(int b=0;b<32;b++){

							if(!(fp->pitchused[i]&shift)){

								fp->pitchused[i]|=shift;

								if(i==EVENTMEMPOOLSIZE/32-1 && b==31){

									fp->full=true;
									if(firstfreepitchbendpool==fp)
										firstfreepitchbendpool=fp->NextPool();
								}
								else
									firstfreepitchbendpool=fp;

#ifdef WIN32
								lock_pitch.Unlock();
#endif
								return &fp->pitch[(i*32)+b];	
							}

							shift<<=1;
						}
					}
				}

				fp=fp->NextPool();
		}

		if(mempool_Pitchbend *newpool=new mempool_Pitchbend(this)){

			pitchbendpool.AddEndO(firstfreepitchbendpool=newpool); // else outof memory	
			newpool->pitchused[0]=1;
#ifdef WIN32
			lock_pitch.Unlock();
#endif
			return newpool->pitch;
		}
	}

#ifdef WIN32
	lock_pitch.Unlock();
#endif

	return 0;

#else
	return new Pitchbend;
#endif
}

void mainPools::mempDeletePitchbend(Pitchbend *cc)
{
#ifdef MEMPOOLS

	cc->Init();

#ifdef WIN32
	lock_pitch.Lock();
#endif

	firstfreepitchbendpool=FirstPitchbendPool(); // reset free

	size_t bit=EVENTMEMPOOLSIZE-(&cc->pool->pitch[EVENTMEMPOOLSIZE]-cc);

	// Reset Class
	cc->pool->full=false;
	cc->pool->pitchused[bit/32]^=1<<bit;

	for(int c=0;c<EVENTMEMPOOLSIZE/32;c++)
		if(cc->pool->pitchused[c]){

#ifdef WIN32
			lock_pitch.Unlock();
#endif
			return;
		}

		pitchbendpool.RemoveO(cc->pool);

#ifdef WIN32
		lock_pitch.Unlock();
#endif

#else
	delete cc;
#endif
}

NoteOff_Realtime *mainPools::mempGetNoteOff_Realtime()
{
#ifdef MEMPOOLS

#ifdef WIN32
	lock_noteoffreal.Lock();
#endif

	mempool_NoteOffRealtime *fp=firstfreenoteoffrealtimepool;

	while(fp) // out of memory ???
	{
		if(fp->full==false)
			for(int i=0;i<EVENTREALTIMEMEMPOOLSIZE/32;i++)
			{
				if(fp->used[i]!=0xFFFFFFFF) // 32 Bit MemPool
				{
					int shift=1;

					for(int b=0;b<32;b++){

						if(!(fp->used[i]&shift)){

							fp->used[i]|=shift;

							if(i==EVENTREALTIMEMEMPOOLSIZE/32-1 && b==31){

								fp->full=true;
								if(firstfreenoteoffrealtimepool==fp)
									firstfreenoteoffrealtimepool=fp->NextPool();
							}
							else
								firstfreenoteoffrealtimepool=fp;

#ifdef WIN32
							lock_noteoffreal.Unlock();
#endif
							return &fp->offs[(i*32)+b];	
						}

						shift<<=1;
					}
				}
			}

			fp=fp->NextPool();
	}

	if(mempool_NoteOffRealtime *newpool=new mempool_NoteOffRealtime(this)){
		noteoffrealtimepool.AddEndO(firstfreenoteoffrealtimepool=newpool); // else outof memory	
		newpool->used[0]=1;
#ifdef WIN32
		lock_noteoffreal.Unlock();
#endif
		return newpool->offs;
	}

#ifdef WIN32
	lock_noteoffreal.Unlock();
#endif

	return 0;

#else
	return new NoteOff_Realtime;
#endif
}

void mainPools::mempDeleteNoteOff_Realtime(NoteOff_Realtime *cc)
{
#ifdef MEMPOOLS

	cc->Init();

#ifdef WIN32
	lock_noteoffreal.Lock();
#endif
	size_t bit=EVENTREALTIMEMEMPOOLSIZE-(&cc->pool->offs[EVENTREALTIMEMEMPOOLSIZE]-cc);

	// Reset Class
	cc->pool->full=false;
	cc->pool->used[bit/32]^=1<<bit;

	if(cc->pool==(firstfreenoteoffrealtimepool=FirstNoteOffRealtimePool()))
	{
		// Always keep 1 Pool !

#ifdef WIN32
		lock_noteoffreal.Unlock();
#endif
		return;
	}

	for(int c=0;c<EVENTREALTIMEMEMPOOLSIZE/32;c++)
		if(cc->pool->used[c])
		{
#ifdef WIN32
			lock_noteoffreal.Unlock();
#endif
			return;
		}

		noteoffrealtimepool.RemoveO(cc->pool);

#ifdef WIN32
		lock_noteoffreal.Unlock();
#endif

#else
	delete cc;
#endif
}

Seq_SelectionEvent *mainPools::mempGetSelEvent()
{
#ifdef MEMPOOLS

	if(!FirstSelectionPool()) // First Pool
	{
		if(mempool_SelectionEvent *fp=new mempool_SelectionEvent(this)){

			firstfreeselectionpool=fp;
			selectionpool.AddEndO(fp); // else out of memory ?
			fp->used[0]=1;
			return fp->sevents;	
		}
	}
	else{
		mempool_SelectionEvent *fp=firstfreeselectionpool;

		while(fp) // out of memory ???
		{
			if(fp->full==false)
				for(int i=0;i<EVENTMEMPOOLSIZE/32;i++){

					if(fp->used[i]!=0xFFFFFFFF) // 32 Bit MemPool
					{
						int shift=1;

						for(int b=0;b<32;b++){

							if(!(fp->used[i]&shift)){

								fp->used[i]|=shift;

								if(i==EVENTMEMPOOLSIZE/32-1 && b==31){

									fp->full=true;
									if(firstfreeselectionpool==fp)
										firstfreeselectionpool=fp->NextPool();
								}
								else
									firstfreeselectionpool=fp;

								return &fp->sevents[i*32+b];	
							}

							shift<<=1;
						}
					}
				}

				fp=fp->NextPool();
		}

		if(mempool_SelectionEvent *newpool=new mempool_SelectionEvent(this)){
			selectionpool.AddEndO(firstfreeselectionpool=newpool); // else outof memory	
			newpool->used[0]=1;
			return newpool->sevents;
		}
	}

	return 0;

#else
	return new Seq_SelectionEvent;
#endif
}

void mainPools::mempDeleteSelEvent(Seq_SelectionEvent *cc)
{
#ifdef MEMPOOLS

	cc->Init();

#ifdef WIN32
	// lock_selection.Lock();
#endif

	firstfreeselectionpool=FirstSelectionPool(); // reset free

	size_t bit=EVENTMEMPOOLSIZE-(&cc->pool->sevents[EVENTMEMPOOLSIZE]-cc);

	// Reset Class
	cc->pool->full=false;
	cc->pool->used[bit/32]^=1<<bit;

	for(int c=0;c<EVENTMEMPOOLSIZE/32;c++)
		if(cc->pool->used[c])
			return;

	selectionpool.RemoveO(cc->pool);

#ifdef WIN32
	// lock_selection.Unlock();
#endif

#else
	delete cc;
#endif
}

void mainPools::CloseAllMemoryPools()
{
#ifdef MEMPOOLS

	// NoteOn
	mempool_Note *fp=FirstNotePool();

#ifdef _DEBUG
	if(fp)	
		MessageBox(NULL,"Memory Leak Note Pool","Error",MB_OK);
#endif

	while(fp)
		fp=(mempool_Note *)notepool.RemoveO(fp);

	// Control

	mempool_Control *fc=FirstControlPool();

#ifdef _DEBUG
	if(fc)	
		MessageBox(NULL,"Memory Leak Control Pool","Error",MB_OK);
#endif

	while(fc)
		fc=(mempool_Control *)controlpool.RemoveO(fc);

	// Pitchbend

	mempool_Pitchbend *pb=FirstPitchbendPool();

#ifdef _DEBUG
	if(pb)	
		MessageBox(NULL,"Memory Leak Pitchbend Pool","Error",MB_OK);
#endif

	while(pb)
		pb=(mempool_Pitchbend *)pitchbendpool.RemoveO(pb);

	// NoteOffReal

	mempool_NoteOffRealtime *op=FirstNoteOffRealtimePool();

#ifdef _DEBUG
	if(!op)	
		MessageBox(NULL,"Memory  mempool_NoteOffRealtime Pool","Error",MB_OK);
#endif

	while(op)
		op=(mempool_NoteOffRealtime *)noteoffrealtimepool.RemoveO(op);

	// Selection Event

	mempool_SelectionEvent *sp=FirstSelectionPool();

#ifdef _DEBUG
	if(sp)	
		MessageBox(NULL,"Memory Leak SelEvent Pool","Error",MB_OK);
#endif

	while(sp)
		sp=(mempool_SelectionEvent *)selectionpool.RemoveO(sp);

#endif

}

#ifdef MEMPOOLS

// Mem Pools Init
mempool_Note::mempool_Note(mainPools *main)
{
	mainpool=main;

	for(int i=0;i<EVENTMEMPOOLSIZE/32;i++)
		notesused[i]=0;

	for(int a=0;a<EVENTMEMPOOLSIZE;a++)
		notes[a].pool=this;

	full=false;
}

mempool_Control::mempool_Control(mainPools *main)
{
	mainpool=main;

	for(int i=0;i<EVENTMEMPOOLSIZE/32;i++)
		controlused[i]=0;

	for(int a=0;a<EVENTMEMPOOLSIZE;a++)
		controls[a].pool=this;

	full=false;
}

mempool_Pitchbend::mempool_Pitchbend(mainPools *main)
{
	mainpool=main;
	for(int i=0;i<EVENTMEMPOOLSIZE/32;i++)
		pitchused[i]=0;

	for(int a=0;a<EVENTMEMPOOLSIZE;a++)
		pitch[a].pool=this;

	full=false;
}

mempool_SelectionEvent::mempool_SelectionEvent(mainPools *main)
{
	mainpool=main;

	for(int i=0;i<EVENTMEMPOOLSIZE/32;i++)
		used[i]=0;

	for(int a=0;a<EVENTMEMPOOLSIZE;a++)
		sevents[a].pool=this;

	full=false;
}

mempool_NoteOffRealtime::mempool_NoteOffRealtime(mainPools *main)
{
	mainpool=main;

	for(int i=0;i<EVENTREALTIMEMEMPOOLSIZE/32;i++)
		used[i]=0;

	for(int a=0;a<EVENTREALTIMEMEMPOOLSIZE;a++)
		offs[a].pool=this;

	full=false;
}
#endif

mainPools::mainPools()
{
	#ifdef MEMPOOLS

	firstfreenoteoffrealtimepool=new mempool_NoteOffRealtime(this);
	noteoffrealtimepool.AddEndO(firstfreenoteoffrealtimepool); // else out of memory ?

	firstfreenotepool=0;
	firstfreecontrolpool=0;
	firstfreepitchbendpool=0;
	firstfreeselectionpool=0;

#endif

}


#endif