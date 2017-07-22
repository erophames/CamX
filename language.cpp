#include "languagefiles.h"
#include "songmain.h"


#ifdef OLDLANGUAGE
class LString:public Object
{
public:
	char *string;
};

void Language::WriteString(LString *s)
{
	// Find Objects
	bool ok=true;
	int i;
	
	for(i=0;i<stringcounter;i++)
		if(strcmp(s->string,strings[i].string)==0)
		{
			ok=false;
			break;
		}
		
		if(ok==true) // add string
		{
			char end;
			
			file.Write("[:]",3);
			file.Write(s->string,strlen(s->string));
			
			end=13;
			file.Write(&end,1);
			end=10;
			file.Write(&end,1);
			file.Write(":=",3);
				end=13;
			file.Write(&end,1);
			end=10;
			file.Write(&end,1);
		}
}

void Language::AddString(char *string)
{
	if(string)
	{
		// Find Objects
		bool ok=true;
		LString *fo=(LString *)create.GetRoot();
		while(fo && ok==true)
		{
			if(strcmp(string,fo->string)==0)
				ok=false;
			
			fo=(LString *)fo->next;
		}
		
		if(ok==true)
		{
			LString *ls=new LString;
			
			ls->string=new char[strlen(string)+1];
			
			strcpy(ls->string,string);
			create.AddEndO(ls);
		}
	}
}

void Language::CreateLanguageFiles()
{
	HANDLE hdl;	
	WIN32_FIND_DATA data;
	
	hdl=FindFirstFile("*.cpp",&data);
	
	if(hdl!=INVALID_HANDLE_VALUE)
	{
		do
		{
			CFile file;
			
			if(file.Open(data.cFileName,CFile::shareDenyWrite))
			{
				CFileStatus status;
				
				file.GetStatus(status);

				int langfilelen;
				langfilelen=status.m_size;

				char *text=new char[langfilelen];
				char *textend;

				if(text)
				{
					textend=text+langfilelen;

					file.Read(text,langfilelen);

					// Find Strings
					char *c=text;
					bool comment=false;
					bool lcomment=false;

					while(c<=(textend-4))
					{
						if(*c==47 && *(c+1)=='*')
							comment=true;

						if(*c=='*' && *(c+1)==47)
							comment=false;
						
						if(*c=='/' && *(c+1)=='/')
							lcomment=true;
						
						if(*c==10)
							lcomment=false;
						
						// ("Welcome or ("
						
						if(comment==false && 
							lcomment==false && 
							((*c==' ') || (*c==',') || (*c=='(')) &&
							(*(c+1)=='L') && 
							(*(c+2)=='O') && 
							(*(c+3)=='C') && 
							((*(c+4)==' ') || (*(c+4)=='('))
							) // start string
						{
							c+=4;
							
							if(*c==' ')
								c++;
							
							c+=2;
							
							char *start=c;
							
							while(c!=textend && *c!=34)
								c++;
							
							if((c-start)>1)
							{
								char *string=new char[(c-start)+1];
								
								strncpy(string,start,c-start);
								string[c-start]=0;
								
								AddString(string);
								
								if(*c==34 || *c==39)c++;
								
								delete string;
							}
							else
								c++;
						}
						else
							c++;
					}

					delete text;
				}

				file.Close();
			}
			
		}while(FindNextFile(hdl,&data));
		
		FindClose(hdl);
		
		// Write Strings	
		hdl=FindFirstFile("locale/*.language",&data);
		
		if(hdl!=INVALID_HANDLE_VALUE)
		{
			do
			{
				Language lang;

				if(lang.OpenLanguage(data.cFileName,LANGADD)==true)
				{
					LString *fo=(LString *)create.GetRoot();
					
					while(fo)
					{
						lang.WriteString(fo);

						fo=(LString *)fo->next;
					}
					
					lang.file.Close();

					lang.CloseLanguage();
				}
				
			}while(FindNextFile(hdl,&data));
			
			FindClose(hdl);
		}

		/*
		CFile wfile;
		
		if(file.Open("locale/allefiles.txt",CFile::modeCreate|CFile::modeWrite))
		{
			LString *fo=(LString *)create.GetRoot();
			char end;
			
			while(fo)
			{
				file.Write("[:]",3);
				file.Write(fo->string,strlen(fo->string));

				end=13;

				file.Write(&end,1);

				end=10;
				file.Write(&end,1);
				
				fo=(LString *)fo->next;
			}
			
			file.Close();
			MessageBox(NULL,"Language Files Done","Error",MB_OK);
			
		}
		*/

		LString *fo=(LString *)create.GetRoot();
		
		while(fo)
		{
			delete fo->string;
			fo=(LString *)create.RemoveO(fo);
		}
	}
}


bool Language::OpenLanguage(char *filename,int flag)
{
	bool ok=false;
	
	if(filename)
	{
		char *dirname=new char[strlen(filename)+sizeof("locale/")+1];
		
		if(dirname)
		{
			int langfilelen;
			
			mainvar->MixString(dirname,"locale/",filename);
			
#ifdef WIN32

			if(flag&LANGADD) // write
			{
				if(file.Open(dirname,CFile::modeCreate|CFile::modeWrite))
					ok=true;
			}
			else
			{
				if(file.Open(dirname,CFile::shareDenyWrite)) // read only
					ok=true;
			}

			if(ok==true) // Read Strings
			{
				CFileStatus status;
				
				file.GetStatus(status);
				langfilelen=status.m_size;
#endif
				
				if(langfilelen>16)
				{
					char *text=new char[langfilelen+1];
					
					if(text)
					{
						
						text[langfilelen]=0;
						
#ifdef WIN32
						file.Read(text,langfilelen);
#endif		
						
						char *check=text;
						char *textend=text+langfilelen;
						int i=langfilelen-1;
						int snow=0;
						
						// Count Strings
						while(i>2)
						{
							if(*check=='[' && (*(check+1)==':'))
								stringcounter++;
							
							check++;
							i--;
						}
						
						if(stringcounter)
						{
							strings=new LanguageString[stringcounter];
							
							if(strings)
							{
								char *check2;
								bool stringend;

								long sl;
								
								check=text;
								i=langfilelen;
								
								while(i>4 && 
									(check<textend)
									)
								{							
									if(*check=='[' && (*(check+1)==':')) // start of string
									{
										bool found=false;
										
										check++; // :
										check++;

										stringend=false;
										
										if(*check==']')
										{
											check++;
											
											char *defaultstring=check;
											char *defend=0;

											bool foundtranslate=false;
											
											//find ':=' translate string
											while(check<textend &&
												foundtranslate==false &&
												((*check!='[') || (*(check+1)!=':'))
												)
											{
												if(*check==':' && *(check+1)=='=') // :=
												{
													defend=check;

													if(defend>defaultstring)
													{
														defend--;

														if(*defend==13 || *defend==10)
															defend--;
													}
													/*
													if(defend>defaultstring)
													{
														defend--;

														if(*defend==13 || *defend==10)
															defend--;
													}
*/

													//string length
													check++; // :
													check++; // =
													
													check2=check;
													sl=0;
													
													while(
														check2<=textend && 
														foundtranslate==false
														)
													{												
														if((check2==textend) ||
															(*check2==13) ||  // cr
															((*check2=='[') && (*(check2+1)==':')) // next string
															)// end of file or return ?
														{
															if(sl>1 && ((defend-defaultstring)>1))
															{
																strings[snow].defstring=new char[(defend-defaultstring)+1];
																strings[snow].string=new char[sl+1]; // +1 for 0
																
																if(strings[snow].defstring && strings[snow].string)
																{
																	strncpy(strings[snow].defstring,defaultstring,defend-defaultstring);
																	strings[snow].defstring[defend-defaultstring]=0;

																	strncpy(strings[snow].string,check,sl);
																	strings[snow].string[sl]=0;
																	
																}
															}
															
															if(*check2=='[') // next string ?
																check=check2;
															else
																check=check2+1;
															
															foundtranslate=true;
														}
														else
														{
															sl++;
															check2++;
															i--;
														}
														
													}// while end of string
													
												}// if :=
												else
													check++;
												
											}// while find translate string	
											
											/*
											if(foundtranslate==false) // use default
											{
												char *d=defaultstring;
												long defaultlen=0;
												
												while((d!=textend) &&
													(*d!=13) &&  // cr
													((*d!='[') || (*(d+1)!=':')) // next string
													)// end of file or return ?
												{
													defaultlen++;
													d++;
												}
												strings[snow].string=new char[defaultlen+1]; // +1 for 0
												
												if(strings[snow].string)
												{
													strncpy(strings[snow].string,defaultstring,defaultlen);
													strings[snow].string[defaultlen]=0;
													
												}
												
											}
											*/

											snow++;
										}
										
							}// if [
							else
							{
								check++;
								i--;
							}
							
						}// while i
						
						ok=true;
					}
				}
				
				delete text;
				
			}// if text
		}
		
		if((flag&LANGADD)==0)
			file.Close();
	}// if file open
	
	delete dirname;
	
	}// if dirname
	
}
return ok;
}

void Language::CloseLanguage()
{
	int i;
	
	if(strings)
	{
		for(i=0;i<stringcounter;i++)
		{
			if(strings[i].string)
				delete strings[i].string;
			if(strings[i].defstring)
				delete strings[i].defstring;
		}
		
		delete strings;
	}
}

char *Language::GetString(char *s)
{
	for(int i=0;i<stringcounter;i++)
	{
		if(strings[i].defstring && 
			strings[i].string && 
			strcmp(strings[i].defstring,s)==0)
			return strings[i].string;
	}
	
	return s; // string not found
}

#endif