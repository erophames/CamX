#include "defines.h"
#include "camxfile.h"

#ifdef WIN32
#include <afxdlgs.h>
#include <afx.h>
#include "pathdialog.h"
#endif

#include "songmain.h"
#include "gui.h"
#include "languagefiles.h"
#include "guiwindow.h"
#include "editfunctions.h"
#include "version.h"

void camxFile::Flush()
{
	#ifdef WIN32
		if((status&CAMXFILE_STATUS_SAVE) && errorwriting==false)
		file.Flush();
#endif
}

void camxFile::Close(bool full)
{
	if(status!=CAMXFILE_STATUS_NOTOPEN)
	{
#ifdef WIN32
		if((status&CAMXFILE_STATUS_SAVE) && errorwriting==false)
		file.Flush();

		file.Close();
#endif

		if(usesum==true)
		{
			if(errorwriting==false)
			{
				if(char *rbuffer=new char[4096])
				{
					// Checksum
					csum.clear();

					if(file.Open(fname,CFile::modeReadWrite))
					{
						// Seek Header
						int seekheader=4+sizeof(int)+sizeof(int)+sizeof(DWORD); // dont checksum Header
						file.Seek(seekheader,CFile::begin);

						// Checksum
						for(;;)
						{
							UINT r=file.Read(rbuffer,4096);

							for(UINT i=0;i<r;i++)csum.add(rbuffer[i]);
							if(r<4096)break;
						}

						// Write CheckSum
						DWORD sum=csum.get();

						file.Seek(4+sizeof(int)+sizeof(int),CFile::begin);
						file.Write(&sum,sizeof(DWORD));

						TRACE ("Write CheckSum %s %d\n",fname,sum);
#ifdef WIN32
						file.Close();
#endif

					}
					delete rbuffer;
				}
			}

			usesum=false;
		}//use checksum
	}

	status=CAMXFILE_STATUS_NOTOPEN;

	if(full==true)
	{
		if(fname){
			delete fname;
			fname=0;
		}
	}

	if(helpstring1)
	{
		delete helpstring1;
		helpstring1=0;
	}
}

int camxFile::CompareDate(camxFile *file)
{
	if(file->status==CAMXFILE_STATUS_READ && status==CAMXFILE_STATUS_READ)
	{
		// -1 older
		// 0 ==
		// 1 ==younger
		file->GetDate();
		GetDate();

		if(date_year<file->date_year)return -1; // older
		if(date_year>file->date_year)return 1;
		if(date_month<file->date_month)return -1;
		if(date_month>file->date_month)return 1;
		if(date_day<file->date_day)return -1;
		if(date_day>file->date_day)return 1;
		if(date_hour<file->date_hour)return -1;
		if(date_hour>file->date_hour)return 1;
		if(date_min<file->date_min)return -1;
		if(date_min>file->date_min)return 1;
		if(date_second<=file->date_second)return -1;
		if(date_second>file->date_second)return 1;
	}

	return 0;
}

bool camxFile::OpenReadSave(char *filename)
{
	bool ok=false;

	if(filename && status==CAMXFILE_STATUS_NOTOPEN)
	{
		errorwriting=eof=false;

		if(fname)delete fname;

		if(fname=mainvar->GenerateString(filename))
		{

#ifdef WIN32

#ifdef _DEBUG
			if(file.Open(fname,CFile::modeReadWrite|CFile::modeCreate|CFile::osRandomAccess,&exec))
#else
			if(file.Open(fname,CFile::modeReadWrite|CFile::modeCreate|CFile::osRandomAccess/*|CFile::modeNoTruncate*/))
#endif				
			{
				ok=true;
				status=CAMXFILE_STATUS_READSAVE;
			}

#ifdef _DEBUG
			else
			{
				
#ifdef _DEBUG
			char *h2=0;
			switch(exec.m_cause)
			{
			case CFileException::none: h2="No error occurred";break;
			case CFileException::genericException: h2="An unspecified error occurred.";break;
			//case CFileException::fileNotFound: h2="The file could not be located.";break;
		//	case CFileException::badPath: h2="All or part of the path is invalid.";break;
			case CFileException::tooManyOpenFiles: h2="The permitted number of open files was exceeded.";break;
			case CFileException::accessDenied: h2="The file could not be accessed.";break;
			case CFileException::invalidFile: h2="There was an attempt to use an invalid file handle.";break;
			case CFileException::removeCurrentDir: h2="The current working directory cannot be removed.";break;
			case CFileException::directoryFull: h2="There are no more directory entries.";break;
			case CFileException::badSeek: h2="There was an error trying to set the file pointer.";break;
			case CFileException::hardIO: h2="There was a hardware error.";break;
			case CFileException::sharingViolation: h2="SHARE.EXE was not loaded, or a shared region was locked.";break;
			case CFileException::lockViolation: h2="There was an attempt to lock a region that was already locked.";break;
			case CFileException::diskFull: h2="The disk is full.";break;
			case CFileException::endOfFile: h2="The end of file was reached. ";break;
			}

			if(h2)
			{
			if(char *h=mainvar->GenerateString("Open Read","\n",filename,"\n:",h2))
			{
				maingui->MessageBoxOk(0,h);
				delete h;
			}
			}
#endif
			}
#endif
#endif
		}
	}
#ifdef _DEBUG
	else
	{
		MessageBox(NULL,"Open File Error RW","Error",MB_OK);
	}
#endif

	return ok;
}

bool camxFile::OpenRead(char *filename)
{
	errorwriting=false;
	bool ok=false;

	if(filename && status==CAMXFILE_STATUS_NOTOPEN)
	{
		errorwriting=eof=false;

		if(fname)delete fname;

		if(fname=mainvar->GenerateString(filename))
		{
#ifdef WIN32

#ifdef _DEBUG
			if(file.Open(fname, CFile::modeRead | CFile::shareDenyWrite,&exec))
#else
			if(file.Open(fname, CFile::modeRead | CFile::shareDenyWrite))
#endif				
			{
				ok=true;
				status=CAMXFILE_STATUS_READ;
			}
			else
			{

#ifdef _DEBUG
			char *h2=0;
			switch(exec.m_cause)
			{
			case CFileException::none: h2="No error occurred";break;
			case CFileException::genericException: h2="An unspecified error occurred.";break;
			//case CFileException::fileNotFound: h2="The file could not be located.";break;
		//	case CFileException::badPath: h2="All or part of the path is invalid.";break;
			case CFileException::tooManyOpenFiles: h2="The permitted number of open files was exceeded.";break;
			case CFileException::accessDenied: h2="The file could not be accessed.";break;
			case CFileException::invalidFile: h2="There was an attempt to use an invalid file handle.";break;
			case CFileException::removeCurrentDir: h2="The current working directory cannot be removed.";break;
			case CFileException::directoryFull: h2="There are no more directory entries.";break;
			case CFileException::badSeek: h2="There was an error trying to set the file pointer.";break;
			case CFileException::hardIO: h2="There was a hardware error.";break;
			case CFileException::sharingViolation: h2="SHARE.EXE was not loaded, or a shared region was locked.";break;
			case CFileException::lockViolation: h2="There was an attempt to lock a region that was already locked.";break;
			case CFileException::diskFull: h2="The disk is full.";break;
			case CFileException::endOfFile: h2="The end of file was reached. ";break;
			}

			if(h2)
			{
			if(char *h=mainvar->GenerateString("Open Read","\n",filename,"\n:",h2))
			{
				maingui->MessageBoxOk(0,h);
				delete h;
			}
			}
#endif
			}
#endif
		}
	}
#ifdef _DEBUG
	else
	{
		if(char *h=mainvar->GenerateString("Open Read Init Error\n",fname))
		{
			MessageBox(NULL,h,"Error",MB_OK);
			delete h;
		}
	}
#endif

	return ok;
}

void camxFile::AddToFileName(char *add) // Save
{
	if(filereqname && add)
	{ 
		char *c = filereqname;
		size_t ct=0,i=strlen(c);

		c+=i;

		while(i--)
		{
			if(*c=='.')
			{
				if(strcmp(c,add)==0)return;

				// Add
				if(char *newfilereqname=new char[strlen(filereqname)-ct+strlen(add)+1]) // .wav exists
				{
					strncpy(newfilereqname,filereqname,strlen(filereqname)-ct);
					strcpy(&newfilereqname[strlen(filereqname)-ct],add);
					delete filereqname;
					filereqname=newfilereqname;
				}

				return;
			}

			ct++;
			c--;
		}

		if(char *newfilereqname=mainvar->GenerateString(filereqname,add)){
			delete filereqname;
			filereqname=newfilereqname;
		}
	}
}

void camxFile::Save_Chunk(ULONGLONG v)
{
	Save_Chunk(&v,sizeof(ULONGLONG));
}

void camxFile::Save_Chunk(LONGLONG v)
{
	Save_Chunk(&v,sizeof(LONGLONG));
}

void camxFile::Save_Chunk(long v)
{
	if(MIDIfile==true)
	{
		UBYTE wb[4];

		wb[0]=(UBYTE)(v/16777216);
		v-=16777216*wb[0];

		wb[1]=(UBYTE)(v/65536);
		v-=wb[1]*65536;

		wb[2]=(UBYTE)(v/256);
		v-=256*wb[2];

		wb[3]=(UBYTE)v;

		Save_Chunk(wb,4);
	}
	else
		Save_Chunk(&v,sizeof(long));
}

void camxFile::Save_Chunk(unsigned long v)
{
	if(MIDIfile==true)
	{
		UBYTE wb[4];

		wb[0]=(UBYTE)(v/16777216);
		v-=16777216*wb[0];

		wb[1]=(UBYTE)(v/65536);
		v-=wb[1]*65536;

		wb[2]=(UBYTE)(v/256);
		v-=256*wb[2];

		wb[3]=(UBYTE)v;

		Save_Chunk(wb,4);
	}
	else
		Save_Chunk(&v,sizeof(long));
}

void camxFile::Save_Chunk(char v)
{
	Save_Chunk(&v,sizeof(char));
}

void camxFile::Save_Chunk(unsigned char v)
{
	Save_Chunk(&v,sizeof(unsigned char));
}

void camxFile::Save_Chunk(bool v)
{
	Save_Chunk(&v,sizeof(bool));
}

void camxFile::Save_Chunk(float v)
{
	double h=v; // float always -> double !
	Save_Chunk(&h,sizeof(double));
}

void camxFile::Save_Chunk(double v)
{
	Save_Chunk(&v,sizeof(double));
}

void camxFile::Save_Chunk(short v)
{
	if(MIDIfile==true)
	{
		UBYTE wb[2];

		wb[0]=v/256;
		wb[1]=v-(256*wb[0]);

		Save_Chunk(wb,2);
	}
	else
		Save_Chunk(&v,sizeof(short));
}

void camxFile::Save_Chunk(unsigned short v)
{
	if(MIDIfile==true)
	{
		UBYTE wb[2];

		wb[0]=v/256;
		wb[1]=v-(256*wb[0]);

		Save_Chunk(wb,2);
	}
	else
		Save_Chunk(&v,sizeof(unsigned short));
}

void camxFile::Save_Chunk(int v)
{
	if(MIDIfile==true)
	{
		UBYTE wb[4];

		wb[0]=(UBYTE)(v/16777216);
		v-=16777216*wb[0];

		wb[1]=(UBYTE)(v/65536);
		v-=wb[1]*65536;

		wb[2]=(UBYTE)(v/256);
		v-=256*wb[2];

		wb[3]=(UBYTE)v;

		Save_Chunk(wb,4);
	}
	else
	{
		// Always 64bit
		LONGLONG temp=v;
		Save_Chunk(&temp,sizeof(LONGLONG));
	}
}

void camxFile::Read_ChunkString(char *to) // Static char array
{
	if(to){

		long sl=0; // 32bit
		ReadChunk(&sl);

		if(sl)
			ReadChunk(to,sl);
		else
			*to=0;
	}
}

void camxFile::Read_ChunkString(char **to)
{
	if(to)
	{
		if(*to) // old  string ?
		{
			delete *to;
			*to=0;
		}

		long sl=0; //32 bit
		ReadChunk(&sl);

		if(sl){
			*to=new char[sl];
			if(*to)ReadChunk(*to,sl);
		}
		else
			*to=0;
	}
}

void camxFile::Save(void *from,size_t length)
{
#ifdef DEBUG
	if(length>1024*1024*1024 || length<=0)
		maingui->MessageBoxError(0,"Save <=0 ");
#endif

	if((status&CAMXFILE_STATUS_SAVE) && errorwriting==false)
	{
		/*
		if(nobuffer==false)
		SaveBuffer((CPOINTER *)from,length);
		else
		*/
#ifdef WIN32
		try
		{
			file.Write(from,(UINT)length);
		}

		catch (...)
		{
			errorwriting=true; // error file.Write !
		}
#endif
	}
}

void camxFile::Save(char *string)
{
	if(string && (status&CAMXFILE_STATUS_SAVE))
	{
		size_t sl=strlen(string);

		if(sl==0)
			Save(".",1);
		else
			Save(string,sl);
	}
}

void camxFile::Save_ChunkString(char *s)
{
	if(s)
	{
		long namelen=(long)strlen(s)+1; //32 bit

		Save_Chunk(namelen);
		Save_Chunk(s,namelen);
	}
	else
		Save_Chunk((long)0); // empty string ?
}

bool savefilenewmessage=false;

bool camxFile::OpenSave_CheckVersion(char *filename)
{
	if(filename && status==CAMXFILE_STATUS_NOTOPEN)
	{
		errorwriting=false;

		if(OpenRead(filename)==true)
		{
			char camx[4];

			if(Read(camx,4)==4) // 4 Bytes CaFV
			{
				if(camx[0]==CAMXFH1 && camx[1]==CAMXFH2 && camx[2]==CAMXFH3 && camx[3]==CAMXFH4) // CamX File ?
				{
					int version;

					if(Read(&version,4)==4) // 4 Bytes
					{
						if(version>maingui->GetVersion() && savefilenewmessage==false)
						{
							savefilenewmessage=true;

							if(char *h=mainvar->GenerateString(Cxs[CXS_CANTOVERWRITENEWERFILE],"\n",filename))
							{
								maingui->MessageBoxError(0,h);
								delete h;
							}

							Close(true);
							return false;
						}
					}
				}
			}

			Close(true);
			return OpenSave(filename);
		}

		return OpenSave(filename);
	}

	return false;
}

bool camxFile::OpenSave(char *filename)
{
	if(filename && status==CAMXFILE_STATUS_NOTOPEN)
	{
		errorwriting=false;

		if(fname)delete fname;

		if(fname=mainvar->GenerateString(filename))
		{
#ifdef WIN32
			if(file.Open(filename,CFile::modeCreate|CFile::modeWrite /*|CFile::modeNoTruncate*/
				,&exec 
				)
				){

					status=CAMXFILE_STATUS_SAVE;
					return true;
			}

			char *h2="?";
			switch(exec.m_cause)
			{
			case CFileException::none: h2="No error occurred";break;
			case CFileException::genericException: h2="An unspecified error occurred.";break;
			case CFileException::fileNotFound: h2="The file could not be located.";break;
			case CFileException::badPath: h2="All or part of the path is invalid.";break;
			case CFileException::tooManyOpenFiles: h2="The permitted number of open files was exceeded.";break;
			case CFileException::accessDenied: h2="The file could not be accessed.";break;
			case CFileException::invalidFile: h2="There was an attempt to use an invalid file handle.";break;
			case CFileException::removeCurrentDir: h2="The current working directory cannot be removed.";break;
			case CFileException::directoryFull: h2="There are no more directory entries.";break;
			case CFileException::badSeek: h2="There was an error trying to set the file pointer.";break;
			case CFileException::hardIO: h2="There was a hardware error.";break;
			case CFileException::sharingViolation: h2="SHARE.EXE was not loaded, or a shared region was locked.";break;
			case CFileException::lockViolation: h2="There was an attempt to lock a region that was already locked.";break;
			case CFileException::diskFull: h2="The disk is full.";break;
			case CFileException::endOfFile: h2="The end of file was reached. ";break;
			}

			if(char *h=mainvar->GenerateString(Cxs[CXS_UNABLETOOPENSAVEFILE],"\n",filename,"\n:",h2))
			{
				maingui->MessageBoxOk(0,h);
				delete h;
			}
#endif	
		}
	}

	return false;
}

void camxFile::Save_Chunk(void *from,size_t length)
{
	if(writechunkerror==true)
		return;

#ifdef _DEBUG
	if(length==0)
		MessageBox(NULL,"Save Chunk Zero","Wrn",MB_OK);
#endif

	if(from && writechunk)
	{
		if((!writechunk->writedata) || length>writechunk->chunkbuffersize) // Add new Buffer
		{
			if(char *newdata=new char[writechunk->chunklen+length+CHUNKBUFFERSIZE])
			{
				writechunk->chunkbuffersize=length+CHUNKBUFFERSIZE;
				writechunk->chunkpointer=newdata+writechunk->chunklen;

				if(writechunk->writedata) // copy old buffer
				{
					memcpy(newdata,writechunk->writedata,writechunk->chunklen);
					delete writechunk->writedata;
				}

				writechunk->writedata=newdata;
			}
			else
			{
#ifdef _DEBUG
				MessageBox(NULL,"Chunk Buffer Error","Error",MB_OK);
#endif
				writechunkerror=true;
				return;
			}
		}

		if(writechunk->chunkbuffersize)
		{
			/*
			int i=length;
			char *fromb=(char *)from;

			while(i--)
			*writechunk->chunkpointer++=*fromb++;
			*/

			memcpy(writechunk->chunkpointer,from,length);

			writechunk->chunkpointer+=length;
			writechunk->chunklen+=length;
			filelength+=length;	

			writechunk->chunkbuffersize-=length;
		}
	}
#ifdef _DEBUG
	else
	{	
		MessageBox(NULL,"Empty File Pointer !!!","Error",MB_OK);
	}

#endif

}

void camxFile::RenewPointer()
{
	Pointer *p=(Pointer *)pointer.GetRoot();

	while(p)
	{
		if(p->autorefresh==true)
		{
			// Find Class
			ClassPointer *cl=(ClassPointer *)classpointer.GetRoot();

			while(cl)
			{
				if(cl->oldclass==p->oldpointer)
				{
					if(p->newpointer)
					{
						//	TRACE ("SizeOf *Char %d\n",sizeof(char *));

						switch(sizeof(char *))
						{
						case 4:
							{
								// 64 Bit ->32 Bit
								int *to=(int *)p->newpointer;
								*to=(int)cl->newclass;
							}
							break;

						case 8:
							{
								// 64Bit->64 Bit
								CPOINTER *to=(CPOINTER *)p->newpointer;
								*to=cl->newclass;
							}
							break;
						}
					}
					break;
				}

				cl=(ClassPointer *)cl->next;
			}
		}

		p=(Pointer *)p->next;
	}
}

ClassPointer *camxFile::AddClass(CPOINTER newclass,CPOINTER oldclass)
{
	if(newclass && oldclass)
	{
		// Find existing class
		ClassPointer *check=(ClassPointer *)classpointer.GetRoot();

		while(check && check->newclass!=newclass)
			check=(ClassPointer *)check->next;

		if(!check)
		{
			if(ClassPointer *p=new ClassPointer)
			{
				p->newclass=newclass;
				p->oldclass=oldclass;
				classpointer.AddEndO(p);
				return p;
			}
		}
	}

	return 0;
}

ClassPointer *camxFile::ChangeClass(CPOINTER old,CPOINTER to)
{
	ClassPointer *c=(ClassPointer *)classpointer.GetRoot();

	while(c)
	{
		if(c->oldclass==old)
		{
			c->newclass=to;
			return c;
		}

		c=(ClassPointer *)c->next;
	}

	return 0;
}

CPOINTER camxFile::FindClass(CPOINTER oldpointer)
{
#ifdef _DEBUG
	/*
	if(!oldpointer)
	MessageBox(NULL,"Zero Find Class","Error",MB_OK);
	*/	
#endif

	if(oldpointer)
	{
		ClassPointer *c=(ClassPointer *)classpointer.GetRoot();

		while(c)
		{
			if(c->oldclass==oldpointer)
				return c->newclass;

			c=(ClassPointer *)c->next;
		}
	}

	return 0;
}

Pointer *camxFile::AddPointer(CPOINTER newpointer)
{
	if(newpointer)
	{
		CPOINTER oldpointer=0;

#ifdef _DEBUG
		if(!readchunk)
			MessageBox(NULL,"Illegal AddPointer","Error",MB_OK);
		else
#endif
			if(ReadChunk(&oldpointer)==sizeof(CPOINTER))
			{
				if(!oldpointer)
					return 0; // no NULL pointer

				if(Pointer *p=new Pointer)
				{
					p->newpointer=newpointer;
					p->oldpointer=oldpointer;
					pointer.AddEndO(p);
					return p;
				}
			}
	}

	return 0;
}

// Parent
//  ---- Chunk
//  ---- Chunk

void camxFile::OpenChunk() // MIDI File
{
	if(writechunk)
		MessageBox(NULL,"Double Open Write Chunk",Cxs[CXS_ERROR],MB_OK);
	else
		writechunk=new Chunk(0);
}

void camxFile::OpenChunk(int nr)
{
	camxfile=true;

	if(writechunk)
		MessageBox(NULL,"Double Open Write Chunk",Cxs[CXS_ERROR],MB_OK);
	else
		writechunk=new Chunk(nr);
}

void camxFile::DeleteFile()
{
	if(fname)
	{
		char *fnbuffer=mainvar->GenerateString(fname);
		Close(true); // close if open

		if(fnbuffer)
		{
#ifdef WIN32
			try
			{
				file.Remove(fnbuffer);
			}
			catch (CFileException* pEx)
			{
				pEx->Delete();
			}
#endif
			delete fnbuffer;
		}
	}
}

void camxFile::CloseReadChunk()
{
#ifdef _DEBUG
	if(!readchunk)
		MessageBox(NULL,"Close Not Existing Chunk","Error",MB_OK);
#endif

	if((status==CAMXFILE_STATUS_READ) && readchunk)
	{	
		if(readchunk->unknown==false)
		{					
			if(readchunk->chunklen){

				SeekCurrent(readchunk->chunklen);
#ifdef _DEBUG


				MessageBox(NULL,"Jump Chunk ChunkLen","Error",MB_OK);
#endif
			}

			delete readchunk;
			readchunk=0;
		}
	}
}

void camxFile::ChunkFound()
{	
#ifdef _DEBUG
	if(!readchunk)
		MessageBox(NULL,"Chunk Found error","Error",MB_OK);
#endif

	if(readchunk)
		readchunk->unknown=false;
}

void camxFile::LoadChunk()
{
	if(status!=CAMXFILE_STATUS_READ)
	{
		eof=true;
#ifdef _DEBUG
		MessageBox(NULL,"Chunk Read Error","Error",MB_OK);
#endif
		return;
	}

	if((!readchunk) || readchunk->unknown==false){
		LONGLONG clen=0;
		long cheader=0;

		int rr=Read(&cheader,sizeof(long));

		if(rr!=sizeof(long)){
			eof=true;
			return;
		}

		char h[4];

		h[0]=0;
		h[3]=0;

		if(Read(h,3)!=3){			
#ifdef _DEBUG
			MessageBox(NULL,"Chunk EOF2 Error","Error",MB_OK);
#endif
			cheader=0;
			eof=true;
			return;
		}


		if(strcmp(h,"CCX")!=0)return;

		// Chunk ok
		if(Read(&clen,sizeof(LONGLONG))!=sizeof(LONGLONG))
		{
#ifdef _DEBUG
			MessageBox(NULL,"Chunk EOF3 Error","Error",MB_OK);
#endif
			cheader=0;
			eof=true;
			return;
		}

		if(readchunk=new RChunk){
			readchunk->chunkheader=cheader;
			readchunk->staticchunklen=readchunk->chunklen=clen;
		}
	}			
}

void Chunk::Write(camxFile *file)
{
	if(writedata || file->camxfile==true)
	{
		// Save Header
		if(file->camxfile==true) // CAmx File
		{
			file->Save(&chunknumber,sizeof(long));
			file->Save("CCX",3);
			file->Save(&chunklen,sizeof(LONGLONG)); // Chunklength

			// Create CheckSum
		}
		else
			if(file->MIDIfile==true)
			{
#ifdef WIN32
				int ch=(int)chunklen;
				UBYTE rb[4];

				rb[3]=ch&0xFF;
				ch>>=8;
				rb[2]=ch&0xFF;
				ch>>=8;
				rb[1]=ch&0xFF;
				ch>>=8;
				rb[0]=ch&0xFF;

				//	int testchunklen=(rb[3]+(256*rb[2])+(65536*rb[1])+(rb[0]*16777216));

				file->Save(rb,4);
#endif

				// MIDI File int Header		
			}

			if(writedata){

				if(chunklen)
					file->Save(writedata,chunklen);

				delete writedata;
				writedata=0;
			}
	}
}

void camxFile::CloseChunk()
{	
	if(writechunk)
	{
		writechunk->Write(this);
		delete writechunk;
		writechunk=0;
	}
#ifdef _DEBUG
	else
		MessageBox(NULL,"No Chunk Open","Error",MB_OK);
#endif
}

void camxFile::writeword(unsigned short word)
{
	UBYTE wb[2];

	wb[0]=word/256;
	wb[1]=word-(256*wb[0]);

	Save(wb,2);
}

void camxFile::writebyte(UBYTE byte)
{
	Save(&byte,1);
}

LONGLONG camxFile::GetLength()
{
	if(status!=CAMXFILE_STATUS_NOTOPEN)
	{
		LONGLONG fl=0;

#ifdef WIN32
		CFileStatus status;

		file.GetStatus(status);
		fl=status.m_size;
#endif

		return fl;
	}

	return 0;
}

void camxFile::SeekBegin(LONGLONG offset)
{
	if(status!=CAMXFILE_STATUS_NOTOPEN)
	{
		/*
		if((status&CAMXFILE_STATUS_SAVE) && offset) // Reset Buffer
		{
		if(buffer && bufferfilled)
		{
		file.Write(buffer,bufferfilled);
		InitBuffer();
		}
		}
		*/

#ifdef WIN32

#ifdef DEBUG
		LONGLONG r=
#endif
			file.Seek( offset, CFile::begin );

#ifdef DEBUG
		if(r!=offset)
		{
			MessageBox(NULL,"Seek Begin Failure","Error",MB_OK);
		}
#endif

#endif

		//bufferiostart=offset;
		//bufferposition=0;
		//bufferfilled=0;
		//buffereof=false;
		eof=false;
	}
}

void camxFile::SeekEnd(LONGLONG offset)
{
	if(status!=CAMXFILE_STATUS_NOTOPEN)
	{
		file.Seek( offset, CFile::end );

	}
}

void camxFile::SetFileSize(LONGLONG size)
{
	file.SetLength(size);
}

LONGLONG camxFile::SeekCurrent(LONGLONG offset)
{
	if(status!=CAMXFILE_STATUS_NOTOPEN )
	{
#ifdef WIN32
		offset=file.Seek(offset,CFile::current);
#endif
		/*
		if((status&CAMXFILE_STATUS_SAVE) && offset && buffer && bufferfilled) // Reset Buffer
		{
		file.Write(buffer,bufferfilled);
		InitBuffer();
		}

		if((status&CAMXFILE_STATUS_READ) && buffer && nobuffer==false)
		{
		/*
		if(offset==0)
		{
		LONGLONG s0=bufferiostart;
		s0+=bufferposition-buffer;
		return s0;
		}

		LONGLONG h=-bufferreadlength;
		h+=bufferposition-buffer;
		h+=offset;

		offset=file.Seek(offset,CFile::current);

		bufferfilled=0;
		buffereof=false;
		}
		else
		{
		*/

		//}

		eof=false;
	}

	return offset;
}

void camxFile::GetDate()
{
	if(status==CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		CFileStatus status;

		file.GetStatus(status);

		date_year=status.m_mtime.GetYear();
		date_month=status.m_mtime.GetMonth();
		date_day=status.m_mtime.GetDay();
		date_hour=status.m_mtime.GetHour();
		date_min=status.m_mtime.GetMinute();
		date_second=status.m_mtime.GetSecond();
#endif
	}
}

void camxFile::JumpOverChunk()
{
	if(readchunk)
	{
		if(readchunk->chunklen)
			SeekCurrent(readchunk->chunklen); // jump over unknown

		delete readchunk;
		readchunk=0;
	}
#ifdef _DEBUG
	// else
	//	MessageBox(NULL,"Jump Over Chunk Error","Error",MB_OK);
#endif
}

void camxFile::DeleteAddedClass(CPOINTER newclass)
{
	if(newclass)
	{
		// Find existing class
		ClassPointer *check=(ClassPointer *)classpointer.GetRoot();

		while(check && check->newclass!=newclass)
			check=(ClassPointer *)check->next;

		if(check)
			classpointer.RemoveO(check);
	}
}

void camxFile::ReadAndAddClass(CPOINTER thispointer)
{
	CPOINTER opointer;
	ReadChunk(&opointer);
	AddClass(thispointer,opointer);	
}

int camxFile::ReadChunk(ULONGLONG *to)
{
	return ReadChunk(to,sizeof(ULONGLONG));
}

int camxFile::ReadChunk(LONGLONG *to)
{
	return ReadChunk(to,sizeof(LONGLONG));
}

int camxFile::ReadChunk(long *to)
{
	return ReadChunk(to,sizeof(long));
}

int camxFile::ReadChunk(float *to)
{
	double h;
	int r=ReadChunk(&h,sizeof(double));
	if(r!=sizeof(double))return 0;
	*to=(float)h;
	return sizeof(float);
}

int camxFile::ReadChunk(double *to)
{
	return ReadChunk(to,sizeof(double));
}

/*
int camxFile::ReadChunk(int *to)
{
	return ReadChunk(to,sizeof(int));
}
*/

int camxFile::ReadChunk(char *to)
{
	return ReadChunk(to,sizeof(char));
}

int camxFile::ReadChunk(bool *to)
{
	return ReadChunk(to,sizeof(bool));
}

int camxFile::ReadChunk(UBYTE *to)
{
	return ReadChunk(to,sizeof(UBYTE));
}

int camxFile::ReadChunk(int *to)
{
	// Always 64bit
	LONGLONG temp;

	int r=ReadChunk(&temp,sizeof(LONGLONG));
	if(r!=sizeof(LONGLONG))return 0;

	*to=(int)temp;

	return r;
}

int camxFile::ReadChunk(void *to,int length)
{
	if(status==CAMXFILE_STATUS_READ && readchunk)
	{
		if(readchunk->chunklen>=length)
		{
			readchunk->chunklen-=length;
#ifdef WIN32
			return file.Read(to,length);
#endif
		}

#ifdef _DEBUG
		MessageBox(NULL,"Read Chunk: Overrun Error","Error",MB_OK);
		readchunk->overrun=true;
#endif
	}
#ifdef _DEBUG
	else
	{
		MessageBox(NULL,"Read Chunk Error","Error",MB_OK);
	}
#endif

	return 0;
}

int camxFile::Read(long *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{	
#ifdef WIN32
		return file.Read(to,sizeof(long));
#endif
	}

	return 0;
}

int camxFile::Read(float *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,sizeof(float));
#endif
	}

	return 0;
}

int camxFile::Read(double *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,sizeof(double));
#endif
	}

	return 0;
}


int camxFile::Read(unsigned long *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,sizeof(unsigned long));
#endif
	}

	return 0;
}

int camxFile::Read(char *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,sizeof(char));
#endif
	}

	return 0;
}

int camxFile::Read(ULONGLONG *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,sizeof(ULONGLONG));
#endif
	}

	return 0;
}

int camxFile::Read(LONGLONG *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,sizeof(LONGLONG));
#endif
	}

	return 0;
}

int camxFile::Read(bool *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,sizeof(bool));
#endif
	}

	return 0;
}

int camxFile::Read(int *to)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,sizeof(int));
#endif
	}

	return 0;
}

int camxFile::Read(void *to,int length)
{
	if(status&CAMXFILE_STATUS_READ)
	{
#ifdef WIN32
		return file.Read(to,length);
#endif
	}

	return 0;
}

int camxFile::GetVersion()
{
	int version=0;
		// LoadChunk();
	char camx[4];

	if(Read(camx,4))
	{
		if(camx[0]==CAMXFH1 && camx[1]==CAMXFH2 && camx[2]==CAMXFH3 && camx[3]==CAMXFH4) // CAMX
		{
			//ChunkFound();
			Read(&version,sizeof(int));
		}
	}

	return version;
}

bool static filenewermsgbox=false;

bool camxFile::CheckVersion()
{
	// LoadChunk();
	char camx[4];

	if(Read(camx,4))
	{
		if(camx[0]==CAMXFH1 && camx[1]==CAMXFH2 && camx[2]==CAMXFH3 && camx[3]==CAMXFH4)
		{
			//ChunkFound();
			int version=0;
			Read(&version,sizeof(int));

			if(version>maingui->GetVersion() && filenewermsgbox==false)
			{
				filenewermsgbox=true;
				maingui->MessageBoxOk(0,Cxs[CXS_FILENEWER]);
				return false;
			}

			Read(&specialflags,sizeof(int));

			csum.clear();
			Read(&csum.input_checksum);

			// Calc Read Checksum
			if(char *rbuffer=new char[4096])
			{
				// Checksum
				for(;;){

					int r=Read(rbuffer,4096);

					for(int i=0;i<r;i++)
						csum.add(rbuffer[i]);

					if(r<4096)
						break;
				}

				delete rbuffer;
			}
			else
				return false;

			SeekBegin(4+sizeof(int)+sizeof(int)+sizeof(DWORD));

			if(csum.input_checksum!=csum.get())
			{
				if(char *h=mainvar->GenerateString(Cxs[CXS_CHECKSUMERROR],"!\n",fname))
				{
					maingui->MessageBoxOk(0,h);
					delete h;
					return false;
				}
			}

			TRACE ("FName %s Old Checksum %d New Checksum %d \n",fname,csum.input_checksum,csum.get());
			return true;
		}

		maingui->MessageBoxOk(0,Cxs[CXS_CAMVHEADERNOTFOUND]);
		return false;
	}

	return false;
}

void camxFile::SaveVersion()
{
	int version=maingui->GetVersion();

	char camx[4];
	camx[0]=CAMXFH1;
	camx[1]=CAMXFH2;
	camx[2]=CAMXFH3;
	camx[3]=CAMXFH4;

	Save(camx,4); // 4 Bytes
	Save(&version,sizeof(int)); // 4 Bytes
	Save(&specialflags,sizeof(int));

	DWORD sum=0; // Chechskum // 4 Bytes
	Save(&sum,sizeof(DWORD));
	usesum=true;

	// SERIAL Number 1-4...
}

void camxFile::Init()
{
	status=CAMXFILE_STATUS_NOTOPEN;

	specialflags=0;
	usesum=false;

	wildname=0;
	helpflag=0;
	fname=0; //name
	fpath=0; //path
	filereqname=0;// full
	helpstring1=0;

	filelength=0;
	eof=false;
	writechunkerror=false;
	errorwriting=false;
	camxfile=false;
	MIDIfile=false;

	readchunk=0;
	writechunk=0;

	flag=0;

	errorflag=0;
	audiofilesadded=0;
}

camxFile::~camxFile()
{
	if(fpath){
		delete fpath;
		fpath=0;
	}

	if(fname){
		delete fname;
		fname=0;
	}

	if(filereqname){
		delete filereqname;
		filereqname=0;
	}

	if(helpstring1){
		delete helpstring1;
		helpstring1=0;
	}

	ClearScan();
	classpointer.DeleteAllO();
	pointer.DeleteAllO();

	if(wildname)delete wildname;
}

bool camxFile::CopyFileFromTo(char *from,char *to)
{
	if(from && to)
	{
#ifdef WIN32
		
		BOOL cfok;

		try
		{
		cfok=CopyFile(from,to,false);
		}

		catch(...)
		{
			return false;
		}

		if(cfok==0)return false;

		/*
		BOOL WINAPI CopyFile(
		__in  LPCTSTR lpExistingFileName,
		__in  LPCTSTR lpNewFileName,
		__in  BOOL bFailIfExists
		);
		*/

		// Copy Peak File
		if(strlen(from)>4)
		{
			if(char *h2=mainvar->GenerateString(from,PEAKFILENAME))
			{
				if(char *h3=mainvar->GenerateString(to,PEAKFILENAME))
				{
					size_t i=strlen(from); // dont change PEAKFILENAME
					for(size_t c=0;c<i;c++)
					{
						if(h2[c]=='.')h2[c]='_';
					}

					i=strlen(to);
					for(size_t c=0;c<i;c++) // dont change PEAKFILENAME
					{
						if(h3[c]=='.')h3[c]='_';
					}

					try
					{
						BOOL pfok=CopyFile(h2,h3,false);
					}

					catch(...)
					{
					}

					delete h3;
				}

				delete h2;
			}
		}

#endif
		return true;
	}

	return false;
}

void camxFile::ListDirectoryContents1(char *dirName,char *fileMask,char *fileext,bool *stop)
{
	char *fileName=0,*curDir=new char[1024];

	if(!curDir)
		return;

	char *fullName=new char[1024];
	if(!fullName)
	{
		delete curDir;
		return;
	}

	HANDLE fileHandle;
	WIN32_FIND_DATA findData;
	// save current dir so it can restore it

	if(!GetCurrentDirectory( 1024, curDir) )
	{
		delete curDir;
		delete fullName;
		return;
	}

	// if the directory name is neither . or .. then
	// change to it, otherwise ignore it
	if(strcmp( dirName, "." ) && strcmp( dirName, ".." ) )
	{
		if( !SetCurrentDirectory( dirName ) )
		{
			delete curDir;
			delete fullName;
			return;
		}
	}
	else
	{
		delete curDir;
		delete fullName;
		return;
	}

	// print out the current directory name
	if(!GetFullPathName( fileMask, 1024, fullName,&fileName ) )
	{
		delete curDir;
		delete fullName;
		return;
	}

	/*
	GetLongPathName(fullName,
	longbuffer,
	256);

	TRACE ("LongBuffer %s\n",longbuffer);
	*/

	/*
	CString strInfo;
	strInfo.Format("Directory - %s",fullName);
	*/

	TRACE ("Full Name %s FN %s\n",fullName,fileName);
	// AfxMessageBox(strInfo);

	// Loop through all files in the directory
	fileHandle = FindFirstFile(fileMask,&findData);

	while (fileHandle != INVALID_HANDLE_VALUE && ((!stop) || *stop==false) )
	{
		// If the name is a directory,
		// recursively walk it. Otherwise
		// print the file's data

		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			TRACE ("Sub -->\n");
			ListDirectoryContents1(findData.cFileName,fileMask,fileext,stop);
		}
		else
		{
			// File
			bool add=true;

			if(fileext)
			{
				size_t fi=strlen(findData.cFileName),i=strlen(fileext);

				if(i>fi)
					add=false;
				else
				{
					char *c1=&findData.cFileName[fi-i],*c2=fileext;

					for(size_t h=0;h<i;h++)
					{
						if(*c1!=*c2)
						{
							add=false;
							break;
						}
						c1++;
						c2++;
					}
				}
			}

			if(add==true)
			{
				if(camxScan *s=new camxScan)
				{
					if(!GetFullPathName( findData.cFileName, 1024, fullName,&fileName ) )
					{
						delete s;
						delete curDir;
						delete fullName;
						return;
					}

					s->filename=mainvar->GenerateString(findData.cFileName);
					s->name=mainvar->GenerateString(fullName);

					s->nFileSizeLow=findData.nFileSizeLow;

					TRACE ("Add File %s\n",s->name);
					scandirs.AddEndO(s);
				}
			}				
		}

		// loop thru remaining entries in the dir
		if (!FindNextFile( fileHandle, &findData ))
			break;
	}

	// clean up and restore directory
	FindClose( fileHandle );
	SetCurrentDirectory( curDir );

	delete curDir;
	delete fullName;
} 

void camxFile::ClearScan()
{
	camxScan *f=FirstScan();
	while(f)
	{
		if(f->name)delete f->name;
		if(f->filename)delete f->filename;

		f=(camxScan *)scandirs.RemoveO(f);
	}
}

void camxFile::BuildDirectoryList(char *dir,char *msk,char *fileext,bool *stop)
{
	ListDirectoryContents1(dir, msk,fileext,stop);
}

camxFile::camxFile(char *filename)
{
	fname=mainvar->GenerateString(filename);
	Init();
};

char *filetypes[]=
{
	"Wave (*wav) AIFF (*aif)|*wav;*aif", // FT_WAVES
	"Tempo Map (*.cxtm)|*.cxtm", //FT_TEMPOMAP
	"SMF MIDI SysEx (*.mid;*.kar;*.snd;*.bnk;*.sys;*.syx)|*.mid;*.kar;*.snd;*.bnk;*.sys;*.syx", //FT_MIDI
	"Quantize (*.cxqs)|*.cxqs", //FT_QUANTIZE
	"Project (*.prox)|*.prox", //FT_PROJECT
	"CamX (*.camx)|*.camx", //FT_SONGS
	"Mp3 (*.mp3)|*.mp3", //FT_ENCODED
	"Wave (*wav) AIFF (*aif)|*wav;*aif|Mp3 (*.mp3)|*.mp3", //FT_WAVES_EX:Wave+Decoder
	"MIDI Filter (*.cxfs)|*.cxfs",
	"VST Dump (*.vst)|*.vst",
	"SysEx (*.snd;*.bnk;*.sys;*.syx)|*.snd;*.bnk;*.sys;*.syx"
};

char *camxFile::AllFiles(int type)
{
	if(wildname)delete wildname;
	// "Load/Add Grooves"," (*.grlx)|*.grlx;|All Files (*.*)|*.*||"

	wildname=mainvar->GenerateString(filetypes[type],";|",Cxs[CXS_ALLFILES]," (*.*)|*.*||");
	// "Wave (*wav)|*wav|All Files (*.*)|*.*||"

	return wildname;
}

#undef strcpy

bool camxFile::SelectDirectory(guiWindow *win,guiScreen *screen,char *title)
{
	if(win==0 && screen==0)return false;
//	if(!maingui->GetActiveScreen())return false;

#ifdef WIN32

	/*
	CPathDialog(LPCTSTR lpszCaption=NULL,
	LPCTSTR lpszTitle=NULL,
	LPCTSTR lpszInitialPath=NULL, 
	CWnd* pParent = NULL);
	*/

	CPathDialog dlg(NULL,title,NULL,CWnd::FromHandle(win?win->hWnd:screen->hWnd));

	if (dlg.DoModal() == IDOK)
	{
		CString filename=dlg.GetPathName();

		if(filereqname)delete filereqname;

		if(filereqname=new char[strlen(filename)+1])
		{
			strcpy(filereqname,filename);
			return true;
		}
	}

#endif

	return false;
}

bool camxFile::OpenFileRequester(guiScreen *screen,guiWindow *win,char *title,char *filter,bool open,char *defaultname)
{
	if(screen==0 && win==0)return false;

	bool ok=false;

#ifdef WIN32

	bool openreq;
	int flag=0;

	if(defaultname)
		defaultname=mainvar->stripExtension(defaultname); // cut .wav etc...

	if(open==true)
	{
		// Read
		openreq=true;
		flag=OFN_FILEMUSTEXIST|OFN_ENABLESIZING|OFN_EXPLORER;
	}
	else
	{
		// Save
		openreq=false;
		flag=OFN_ENABLESIZING|OFN_OVERWRITEPROMPT|OFN_EXPLORER;
	}

	mainedit->LockEdit();

	CFileDialog dlg(openreq,NULL,defaultname,flag,filter,CWnd::FromHandle(screen?screen->hWnd:win->hWnd)); // open file req

	dlg.m_ofn.lpstrTitle=title;

	if (dlg.DoModal() == IDOK)
	{
		CString gfilename=dlg.GetPathName();

		//CString gfolder=dlg.GetFolderPath();

		if(gfilename)
		{
			// Path+FName
			size_t sl=strlen(gfilename);

			if(filereqname)
				delete filereqname;

			if(filereqname=new char[sl+1])
			{
				strcpy(filereqname,gfilename);
				ok=true;
			}

			// FName
			CString gfname=dlg.GetFileName();

			if(gfname)
			{
				size_t gsl=strlen(gfname);

				if(fname)
					delete fname;

				if(fname=new char[gsl+1])
					strcpy(fname,gfname);

				// Path Name
				size_t pl=sl-gsl;

				if(pl>1)
				{
					if(fpath=new char[pl])
					{
#undef strncpy
						strncpy(fpath,gfilename,pl);
						fpath[pl-1]=0;
					}
				}

			}
		}
	}
#endif

	mainedit->UnlockEdit();

	return ok;
}
