#include "progress.h"
#include "songmain.h"
#include "gui.h"

void Progress::Start(char *info)
{
	working=true;
	error=false;
	infostring=info;
	lastpercent=percent=0;

	mainvar->AddProgress(this);
	maingui->RefreshProgress();
}

void Progress::End(bool stopped)
{
	p_stopped=stopped;
	working=false;
	done=true;

	maingui->RefreshProgress();
	mainvar->RemoveProgress(this);
}

void Progress::SetPercent(double p)
{
	if(p!=percent)
	{
		percent=p;
		
		if(percent>=lastpercent+5) // 5% steps
		{
			lastpercent=percent;
			maingui->RefreshProgress();
		}
	}
}
