Seq_Event *SysEx::CloneNewAndSortToPattern(MidiPattern *p)
{
	SysEx *newsys=(SysEx *)CloneNew();
	
	if(newsys)
	{
		p->AddSortEvent(newsys);

		if(sysexend.pattern)
		{
			p->AddSortVirtual(&newsys->sysexend);
			newsys->sysexend.pattern=p;
		}
	}
	
	return newsys;
}

Seq_Event *SysEx::CloneNewAndSortToPattern(MidiPattern *p,long diff)
{
	SysEx *newsys=(SysEx *)CloneNew();
	
	if(newsys)
	{
		newsys->objectstart+=diff;
		newsys->sysexend.objectstart+=diff;
		
		newsys->staticstartposition+=diff;
		newsys->sysexend.staticstartposition+=diff;
		
		p->AddSortEvent(newsys);
		
		if(sysexend.pattern)
		{
			p->AddSortVirtual(&newsys->sysexend);
			newsys->sysexend.pattern=p;
		}
	}
	
	return newsys;
}

void SysEx::DeleteFromPattern()
{
	pattern->events.DeleteObject(this); // Remove from Event list
	
	if(sysexend.pattern)
	{
		pattern->DeleteVirtual(&sysexend);
		sysexend.pattern=0;
	}
}

void SysEx::CalcSysTicks()
{
	if(data && length && mainmidi.baudrate)
	{
		double h=mainmidi.baudrate;
		double h2=length;
		
		// start/stopbits ->bytes/sec
		h/=10;
		h/=h2;
		
		// h2=ms
		ticklength=mainvar.ConvertMilliSecToTicks(h);
	}
	else
		ticklength=0;
}

void SysEx::CheckSysExEnd()
{
	if(ticklength>=16) // add virtual 0xF7
	{
		pattern->AddSortVirtual(&sysexend,objectstart+ticklength);
	}
	else
	{
		if(sysexend.pattern)
		{
			pattern->DeleteVirtual(&sysexend);
			sysexend.pattern=0;
		}
	}
}
