
#ifdef WIN32
#define NOGUI
#endif

#include <string.h>
#include "asio/asiodrivers.h"
#include "audiodriver.h"
#include "windows.h"
#include "asio/iasiodrv.h"

#ifdef WIN32
#undef NOGUI
#endif

extern IASIO *theAsioDriver;

// AsioDrivers* camx_asioDrivers[4]={0,0,0,0};

extern AsioDrivers *asioDrivers;
extern bool loadAsioDriver(char *name);

#ifdef DOOF
bool AudioDriver::LoadDriver(char *name)
{
	if(theAsioDriver)
		return false;
	else
	{
		if(loadAsioDriver(name)==true)
			return(true);
		else
			return (false);
	}
		/*
		int i;
		
		  for(i=0;i<4;i++)
		  {
		  if(!camx_asioDrivers[i])
		  {
		  camx_asioDrivers[i] = new AsioDrivers();
		  
			driverid=i+1;
			
			  if(camx_asioDrivers[i])
			  return camx_asioDrivers[i]->loadDriver(name);
			  }
			  }
	*/
}

bool AudioDriver::RemoveDriver()
{
#ifdef WIN32
	if(asioDrivers)
	{
		ASIOExit();

		asioDrivers->removeCurrentDriver();
		delete asioDrivers;
		asioDrivers=0;
		
		return true;
	}
	
	return false;
#endif
	
	/*
	if(driverid)
	{
	camx_asioDrivers[driverid-1]->removeCurrentDriver();
	
	  delete camx_asioDrivers[driverid-1];
	  camx_asioDrivers[driverid-1]=0;
	  
		driverid=0;
		}
		
		  return false;
	*/
}
#endif