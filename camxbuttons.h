endSyncInfo()
{
	if(!isInPopup())
	{
		var oParam=null;
		if(gaPaths.length>0)
		{
			oParam=createSyncInfo();
		}
		var oMsg=new whMessage(WH_MSG_SYNCINFO,this,1,oParam);
		SendMessage(oMsg);
	}
}

function sendInvalidSyncInfo()
{
	if(!isInPopup())
	{
		var oMsg=new whMessage(WH_MSG_SYNCINFO,this,1,null);
		SendMessage(oMsg);
	}
}

function enableWebSearch(bEnable)
{
	if(!isInPopup())
	{
		var oMsg=new whMessage(WH_MSG_ENABLEWEBSEARCH,this,1,bEnable);
		SendMessage(oMsg);
	}
}

function autoSync(nSync)
{
	if(nSync==0) return;
	if(isInPopup()) return;
	if(isOutMostTopic())
		sync();
}

function isOutMostTopic()
{
	if(gnOutmostTopic==-1)
	{
		var oMessage=new whMessage(WH_MSG_ISINFRAMESET,this,1,null);
		if(SendMessage(oMessage))
			gnOutmostTopic=0;
		else
			gnOutmostTopic=1;
	}
	return (gnOutmostTopic==1);
}

function sync()
{
	if(gaPaths.length>0)
	{
		var oParam=createSyncInfo();
		var oMessage=new whMessage(WH_MSG_SYNCTOC,this,1,oParam);
		SendMessage(oMessage);
	}
}


function avenueInfo(sName,sPrev,sNext)
{
	this.sName=sName;
	this.sPrev=sPrev;
	this.sNext=sNext;
}
