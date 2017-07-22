#include "defines.h"
#include "object.h"
#include "editbuffer.h"
#include "object_song.h"
#include "songmain.h"
#include "audiofile.h"
#include "audiohdfile.h"
#include "editfunctions.h"
#include "MIDIPattern.h"
#include "object_track.h"
#include "gui.h"
#include "guiwindow.h"
#include "audiopattern.h"
#include "patternselection.h"

bool EditBuffer::OpenBuffer()
{
	bool ok=true;
	DeleteBuffer();
	return ok;
}

bool EditBuffer::AddObjectToBuffer(Object *object,bool clone)
{
	bool ok=false;

	if(EditBufferElement *ne=new EditBufferElement)
	{
		ok=true;

		switch(object->id)
		{
		case OBJ_SELECTPATTERN:
			{
				Seq_SelectionList *list=(Seq_SelectionList *)object,*newlist=new Seq_SelectionList;

				if(newlist)
				{
					list->Clone(list->song,newlist,0,CLONEFLAG_ERASECLONEDATA);

					if(!newlist->GetCountOfSelectedPattern())
						delete newlist;
					else
						ne->object=newlist;
				}
			}
			break;

		case  OBJ_SONG:
			if(mainvar->GetActiveProject())
			{
				Seq_Song *song=(Seq_Song *)object,*newsong=new Seq_Song(mainvar->GetActiveProject());
				ne->object=newsong;	
			}
			break;

		case  OBJ_TRACK:
			{
				Seq_Track *track=(Seq_Track *)object;

				if(clone==true)
				{
					Seq_Track *newtrack=new Seq_Track;

					if(ne->object=newtrack)
						track->Clone(newtrack);
				}
				else
					ne->object=object;
			}
			break;

		case OBJ_AUDIOPATTERN:
			{
				AudioPattern *p=(AudioPattern *)object;

				if(clone==true)
				{
					if(AudioPattern *newpattern=new AudioPattern)
					{
						ne->object=newpattern;
						p->Clone(p->track->song,newpattern,0,0);

						newpattern->mainclonepattern=p->mainclonepattern; // Copy Clone
						newpattern->mainclonepatternID=p->patternID;
					}
				}
				else
					ne->object=object;	
			}
			break;

		case OBJ_MIDIPattern:
			{
				MIDIPattern *p=(MIDIPattern *)object;

				if(clone==true)
				{
					if(MIDIPattern *newpattern=new MIDIPattern)
					{
						ne->object=newpattern;
						p->Clone(p->track->song,newpattern,0,0);

						newpattern->mainclonepattern=p->mainclonepattern; // Copy Clone
						newpattern->mainclonepatternID=p->patternID;
					}
				}
				else
					ne->object=object;
			}
			break;

		case OBJ_NOTEOFF:
		case OBJ_SYSEXEND:
		case OBJ_NOTE:
		case OBJ_PROGRAM:
		case OBJ_CONTROL:
		case OBJ_PITCHBEND:
		case OBJ_SYSEX:
		case OBJ_CHANNELPRESSURE:
		case OBJ_POLYPRESSURE:
			{
				Seq_Event *e=(Seq_Event *)object;
				ne->object=clone==true?e->Clone(0):object;
			}
			break;

		case OBJ_AUDIOINTERN:
		case OBJ_AUDIOVST:
			{
				AudioObject *ao=(AudioObject *)object;
				ne->object=ao->CloneEffect(0,0);
			}
			break;

		default:
			{
				ne->object=object->Clone();
			}
			break;
		}

		if(ok==false || (!ne->object))
			delete ne;
		else
		{
			edits.AddEndO(ne);
			return true;
		}
	}

	return false;
}

void EditBuffer::CopyEffectList(AudioEffects *effects)
{
	if(effects->FirstInsertAudioEffect() /*||*/
		//effects->FirstActiveAudioInstrument()
		)
	{
		mainbuffer->OpenBuffer();
		mainbuffer->AddObjectToBuffer(effects,true);
		mainbuffer->CloseBuffer();
	}
}

void EditBuffer::PasteBufferToEffectList(AudioEffects *effects)
{
	if(mainbuffer->CheckHeadBuffer(OBJ_EFFECTLIST)==true)
	{
		AudioEffects *ae=(AudioEffects *)mainbuffer->FirstEditBuffer()->object;

		mainedit->InsertPlugins(ae,effects);
	}
}

void EditBuffer::PasteBufferToEffect(Seq_Song *song,AudioEffects *effects,InsertAudioEffect *effect)
{
	if(song && effects)
	{
		EditBufferElement *eb=mainbuffer->FirstEditBuffer();

		if(eb && eb->object){

			switch(eb->object->id)
			{
			case OBJ_AUDIOVST:
			case OBJ_AUDIOINTERN:
				{
					InsertAudioEffect *next=0;

					if(effect){

						maingui->RemoveAudioEffectFromGUI(effect);
						// Replace
						next=effects->DeleteInsertAudioEffect(0,effect,true);
					}

					AudioObject *ao=(AudioObject *)eb->object;

					mainedit->InsertPlugin(effects,ao,false);
				}
				break;
			}
		}	
	}
}

bool EditBuffer::CheckBuffer(guiWindow *win,Seq_Song *song,Object *toobject)
{
	if(song && toobject)
	{
		bool error=false,skipundo=false,addtolastundo=false; 
		OSTART firststartposition=0;

		EditBufferElement *eb=FirstEditBuffer();
		while(eb && error==false)
		{
			bool again=false;

			//loop++;

			//TRACE ("Paste Buffer Loop %d\n",loop);

			if(eb->object)
				switch(eb->object->id)
			{
				case OBJ_SELECTPATTERN:
					{
						return true;

						//mainedit->PasteSelectionListToSong(list,song,starttrack,position);
					}
					break;

				case  OBJ_SONG:
					// ------------------ SONG -------------------------------------------
					{
						Seq_Song *song=(Seq_Song *)eb->object;
					}
					break;

				case  OBJ_TRACK:
					// ------------------ TRACK -------------------------------------------
					{
						Seq_Track *track=(Seq_Track *)eb->object;

						switch(toobject->id)
						{
						case  OBJ_SONG:
							{
								Seq_Song *tosong=(Seq_Song *)toobject;
							}
							break;

						case  OBJ_TRACK:
							{
								Seq_Track *totrack=(Seq_Track *)toobject;

								// track->track
								//mainedit->PasteObjectToTrack(totrack,track,0,flag);
							}
							break;

						case  OBJ_PATTERN:
							{
								Seq_Pattern *top=(Seq_Pattern *)toobject;

							}
							break;

						case  OBJ_NOTEOFF:
						case  OBJ_SYSEXEND:
						case  OBJ_NOTE:
						case  OBJ_PROGRAM:
						case  OBJ_CONTROL:
						case  OBJ_PITCHBEND:
						case  OBJ_SYSEX:
						case  OBJ_CHANNELPRESSURE:
						case  OBJ_POLYPRESSURE:
							{
								Seq_Event *toe=(Seq_Event *)toobject;
							}
							break;
						}
					}
					// end of object track
					break;

				case  OBJ_AUDIOPATTERN:
					{
						AudioPattern *p=(AudioPattern *)eb->object;

						switch(toobject->id)
						{
						case  OBJ_SONG:
							{
								Seq_Song *tosong=(Seq_Song *)toobject;
							}
							break;

						case  OBJ_TRACK:
							{
								Seq_Track *totrack=(Seq_Track *)toobject;
								return true;
								//mainedit->PasteObjectToTrack(totrack,p,position,0);
							}
							break;

						case  OBJ_PATTERN:
							{
								Seq_Pattern *top=(Seq_Pattern *)toobject;

							}
							break;

						case  OBJ_NOTEOFF:
						case  OBJ_SYSEXEND:
						case  OBJ_NOTE:
						case  OBJ_PROGRAM:
						case  OBJ_CONTROL:
						case  OBJ_PITCHBEND:
						case  OBJ_SYSEX:
						case  OBJ_CHANNELPRESSURE:
						case  OBJ_POLYPRESSURE:
							{
								Seq_Event *toe=(Seq_Event *)toobject;
							}
							break;
						}

						// end of object pattern
					}
					break;

				case  OBJ_MIDIPattern:
					// ------------------ PATTERN -------------------------------------------
					{
						MIDIPattern *p=(MIDIPattern *)eb->object;

						switch(toobject->id)
						{
						case  OBJ_SONG:
							{
								Seq_Song *tosong=(Seq_Song *)toobject;
							}
							break;

						case  OBJ_TRACK:
							{
								Seq_Track *totrack=(Seq_Track *)toobject;

								//mainedit->PasteObjectToTrack(totrack,p,position,0);
							}
							break;

						case  OBJ_PATTERN:
							{
								Seq_Pattern *top=(Seq_Pattern *)toobject;

							}
							break;

						case  OBJ_NOTEOFF:
						case  OBJ_SYSEXEND:
						case  OBJ_NOTE:
						case  OBJ_PROGRAM:
						case  OBJ_CONTROL:
						case  OBJ_PITCHBEND:
						case  OBJ_SYSEX:
						case  OBJ_CHANNELPRESSURE:
						case  OBJ_POLYPRESSURE:
							{
								Seq_Event *toe=(Seq_Event *)toobject;
							}
							break;
						}

						// end of object pattern
					}
					break;

				case  OBJ_NOTEOFF:
				case  OBJ_SYSEXEND:
				case  OBJ_NOTE:
				case  OBJ_PROGRAM:
				case  OBJ_CONTROL:
				case  OBJ_PITCHBEND:
				case  OBJ_SYSEX:
				case  OBJ_CHANNELPRESSURE:
				case  OBJ_POLYPRESSURE:
				case OBJ_ICDEVENT:
					// ------------------ EVENT -------------------------------------------
					{
						Seq_Event *e=(Seq_Event *)eb->object;

						switch(toobject->id)
						{
						case  OBJ_SONG:
							{
								Seq_Song *tosong=(Seq_Song *)toobject;
							}
							break;

						case  OBJ_TRACK:
							{
								Seq_Track *totrack=(Seq_Track *)toobject;
								OList events;
								OSTART firsteventposition=-1;

								// Events->Pattern
								while(eb)
								{
									switch(eb->object->id)
									{
									case  OBJ_NOTEOFF:
									case  OBJ_SYSEXEND:
									case  OBJ_NOTE:
									case  OBJ_PROGRAM:
									case  OBJ_CONTROL:
									case  OBJ_PITCHBEND:
									case  OBJ_SYSEX:
									case  OBJ_CHANNELPRESSURE:
									case  OBJ_POLYPRESSURE:
									case OBJ_ICDEVENT:
										{
											// Create Event List

											/*
											Seq_Event *event=(Seq_Event *)eb->object;
											Seq_Event *clone=(Seq_Event *)event->Clone();

											if(firsteventposition==-1)
											firsteventposition=event->ostart;

											if(clone)
											{
											OSTART newposition=position+(event->ostart-firsteventposition);

											clone->SetStartStatic(newposition);
											events.AddEndO(clone);
											}
											*/
										}
										break;
									}

									eb=eb->NextElement();
								}

								// Create Pattern + Paste Events
								//mainedit->CreateNewPattern(0,totrack,MEDIATYPE_MIDI,position,true,0,&events);
							}
							break;

						case OBJ_MIDIPattern:
							{
								return true;

								EF_CreateEvent ec;
								bool add=true;

								/*
								switch(e->GetStatus())
								{
								case NOTEON:
								{
								Note *note=(Note *)e;

								if(tonotekey!=-1 && lowestnote_key!=-1 && highestnote_key!=-1)
								{
								int newnotekey=tonotekey-lowestnote_key;

								newnotekey+=note->key;

								if(newnotekey<0 || newnotekey>127)
								add=false;
								}
								}
								break;
								}
								*/

								if(add==true)
								{
									if(addtolastundo==false)
										firststartposition=e->GetEventStart();

									/*
									ec.event=(Seq_Event *)e->Clone(); // Create Event Clone

									if(ec.event)
									{
									switch(e->GetStatus())
									{
									case NOTEON:
									{
									Note *note=(Note *)ec.event;

									if(tonotekey!=-1 && lowestnote_key!=-1 && highestnote_key!=-1)
									{
									int newnotekey=tonotekey-lowestnote_key;

									newnotekey+=note->key;
									note->key=newnotekey;
									}
									}
									break;
									}

									MIDIPattern *top=(MIDIPattern *)toobject;

									ec.song=top->track->song;
									ec.pattern=top;

									ec.position=position+(e->GetEventStart()-firststartposition);
									ec.checkgui=(!eb->NextElement())?true:false;

									if(skipundo==true)
									addtolastundo=ec.doundo=false;
									else
									ec.doundo=true;

									ec.addtolastundo=addtolastundo;
									ec.playit=false;

									mainedit->CreateNewMIDIEvent(&ec);
									}
									*/
								}
							}
							break;

						case  OBJ_NOTEOFF:
						case  OBJ_SYSEXEND:
						case OBJ_NOTE:
						case  OBJ_PROGRAM:
						case  OBJ_CONTROL:
						case  OBJ_PITCHBEND:
						case  OBJ_SYSEX:
						case  OBJ_CHANNELPRESSURE:
						case  OBJ_POLYPRESSURE:
							{
								Seq_Event *toe=(Seq_Event *)toobject;
							}
							break;
						}

						// end of object event
					}
					break;

			}// switch eb->object

			if(again==false)
			{
				if(eb)
					eb=eb->NextElement();

				addtolastundo=true;
			}
		}
	}

	return false;
}

void EditBuffer::PasteBuffer(guiWindow *win,Seq_Song *song,Object *toobject,OSTART position,int flag)
{
	int loop=0;

	// Reset
	//tonotekey=-1;

	if(song && toobject)
	{
		bool error=false,skipundo=false,addtolastundo=false; 
		OSTART firststartposition=0;

		//if(win)win->InitPaste(this);

		OList *pastelist=0;

		EditBufferElement *eb=FirstEditBuffer();
		while(eb && error==false)
		{
			bool again=false;

			loop++;

			TRACE ("Paste Buffer Loop %d\n",loop);

			if(eb->object)
				switch(eb->object->id)
			{
				case  OBJ_SELECTPATTERN:
					{
						Seq_SelectionList *list=(Seq_SelectionList *)eb->object;
						Seq_Track *starttrack;

						if(toobject && toobject->id== OBJ_TRACK)
							starttrack=(Seq_Track *)toobject;
						else
							starttrack=song->GetFocusTrack();

						mainedit->PasteSelectionListToSong(list,song,starttrack,position);
					}
					break;

				case  OBJ_SONG:
					// ------------------ SONG -------------------------------------------
					{
						Seq_Song *song=(Seq_Song *)eb->object;
					}
					break;

				case  OBJ_TRACK:
					// ------------------ TRACK -------------------------------------------
					{
						Seq_Track *track=(Seq_Track *)eb->object;

						switch(toobject->id)
						{
						case  OBJ_SONG:
							{
								Seq_Song *tosong=(Seq_Song *)toobject;
							}
							break;

						case  OBJ_TRACK:
							{
								Seq_Track *totrack=(Seq_Track *)toobject;

								// track->track
								mainedit->PasteObjectToTrack(totrack,track,0,flag);
							}
							break;

						case  OBJ_PATTERN:
							{
								Seq_Pattern *top=(Seq_Pattern *)toobject;

							}
							break;

						case  OBJ_NOTEOFF:
						case  OBJ_SYSEXEND:
						case  OBJ_NOTE:
						case  OBJ_PROGRAM:
						case  OBJ_CONTROL:
						case  OBJ_PITCHBEND:
						case  OBJ_SYSEX:
						case  OBJ_CHANNELPRESSURE:
						case  OBJ_POLYPRESSURE:
							{
								Seq_Event *toe=(Seq_Event *)toobject;
							}
							break;
						}
					}
					// end of object track
					break;

				case  OBJ_AUDIOPATTERN:
					{
						AudioPattern *p=(AudioPattern *)eb->object;

						switch(toobject->id)
						{
						case  OBJ_SONG:
							{
								Seq_Song *tosong=(Seq_Song *)toobject;
							}
							break;

						case  OBJ_TRACK:
							{
								Seq_Track *totrack=(Seq_Track *)toobject;

								mainedit->PasteObjectToTrack(totrack,p,position,0);
							}
							break;

						case  OBJ_PATTERN:
							{
								Seq_Pattern *top=(Seq_Pattern *)toobject;

							}
							break;

						case  OBJ_NOTEOFF:
						case  OBJ_SYSEXEND:
						case  OBJ_NOTE:
						case  OBJ_PROGRAM:
						case  OBJ_CONTROL:
						case  OBJ_PITCHBEND:
						case  OBJ_SYSEX:
						case  OBJ_CHANNELPRESSURE:
						case  OBJ_POLYPRESSURE:
							{
								Seq_Event *toe=(Seq_Event *)toobject;
							}
							break;
						}

						// end of object pattern
					}
					break;

				case  OBJ_MIDIPattern:
					// ------------------ PATTERN -------------------------------------------
					{
						MIDIPattern *p=(MIDIPattern *)eb->object;

						switch(toobject->id)
						{
						case  OBJ_SONG:
							{
								Seq_Song *tosong=(Seq_Song *)toobject;
							}
							break;

						case  OBJ_TRACK:
							{
								Seq_Track *totrack=(Seq_Track *)toobject;

								mainedit->PasteObjectToTrack(totrack,p,position,0);
							}
							break;

						case  OBJ_PATTERN:
							{
								Seq_Pattern *top=(Seq_Pattern *)toobject;

							}
							break;

						case  OBJ_NOTEOFF:
						case  OBJ_SYSEXEND:
						case  OBJ_NOTE:
						case  OBJ_PROGRAM:
						case  OBJ_CONTROL:
						case  OBJ_PITCHBEND:
						case  OBJ_SYSEX:
						case  OBJ_CHANNELPRESSURE:
						case  OBJ_POLYPRESSURE:
							{
								Seq_Event *toe=(Seq_Event *)toobject;
							}
							break;
						}

						// end of object pattern
					}
					break;

				case  OBJ_NOTEOFF:
				case  OBJ_SYSEXEND:
				case  OBJ_NOTE:
				case  OBJ_PROGRAM:
				case  OBJ_CONTROL:
				case  OBJ_PITCHBEND:
				case  OBJ_SYSEX:
				case  OBJ_CHANNELPRESSURE:
				case  OBJ_POLYPRESSURE:
				case OBJ_ICDEVENT:
					// ------------------ EVENT -------------------------------------------
					{
						Seq_Event *e=(Seq_Event *)eb->object;

						switch(toobject->id)
						{
						case  OBJ_SONG:
							{
								Seq_Song *tosong=(Seq_Song *)toobject;
							}
							break;

						case OBJ_TRACK:
							{
								Seq_Track *totrack=(Seq_Track *)toobject;
								OList events;
								OSTART firsteventposition=-1;

								// Events->Pattern
								while(eb)
								{
									switch(eb->object->id)
									{
									case  OBJ_NOTEOFF:
									case  OBJ_SYSEXEND:
									case  OBJ_NOTE:
									case  OBJ_PROGRAM:
									case  OBJ_CONTROL:
									case  OBJ_PITCHBEND:
									case  OBJ_SYSEX:
									case  OBJ_CHANNELPRESSURE:
									case  OBJ_POLYPRESSURE:
									case OBJ_ICDEVENT:
										{
											// Create Event List

											Seq_Event *seqevent=(Seq_Event *)eb->object;
											Seq_Event *clone=(Seq_Event *)seqevent->Clone(totrack->song);

											if(firsteventposition==-1)
												firsteventposition=seqevent->ostart;

											if(clone)
											{
												OSTART newposition=position+(seqevent->ostart-firsteventposition);

												clone->SetStartStatic(newposition);
												events.AddEndO(clone);
											}
										}
										break;
									}

									eb=eb->NextElement();
								}

								// Create Pattern + Paste Events
								mainedit->CreateNewPattern(0,totrack,MEDIATYPE_MIDI,position,true,0,&events);
							}
							break;

						case OBJ_MIDIPattern:
							{
								//	EF_CreateEvent ec;
								//	bool add=true;

								/*
								switch(e->GetStatus())
								{
								case NOTEON:
								{
								Note *note=(Note *)e;

								if(tonotekey!=-1 && lowestnote_key!=-1 && highestnote_key!=-1)
								{
								int newnotekey=tonotekey-lowestnote_key;

								newnotekey+=note->key;

								if(newnotekey<0 || newnotekey>127)
								add=false;
								}
								}
								break;
								}
								*/

								//if(add==true)
								{
									MIDIPattern *mpattern=(MIDIPattern *)toobject;

									if(!pastelist)
									{
										pastelist=new OList;
										firststartposition=e->GetEventStart();
									}

									Seq_Event *clone=(Seq_Event *)e->Clone(mpattern->track->song);

									if(pastelist && clone)
										pastelist->AddEndO(clone);

									if(pastelist && eb->NextElement()==0)
										mainedit->CreateNewMIDIEvents(mpattern,pastelist,position);

#ifdef OLDIE
									if(ec.event=) // Create Event Clone)
									{
										/*
										switch(e->GetStatus())
										{
										case NOTEON:
										{
										Note *note=(Note *)ec.event;

										if(tonotekey!=-1 && lowestnote_key!=-1 && highestnote_key!=-1)
										{
										int newnotekey=tonotekey-lowestnote_key;

										newnotekey+=note->key;
										note->key=newnotekey;
										}
										}
										break;
										}
										*/

										MIDIPattern *top=(MIDIPattern *)toobject;

										ec.song=top->track->song;
										ec.pattern=top;

										ec.position=position+(e->GetEventStart()-firststartposition);
										ec.checkgui=(!eb->NextElement())?true:false;

										if(skipundo==true)
											addtolastundo=ec.doundo=false;
										else
											ec.doundo=true;

										ec.addtolastundo=addtolastundo;
										ec.playit=false;

										mainedit->CreateNewMIDIEvent(&ec);
									}
#endif

								}
							}
							break;

						case  OBJ_NOTEOFF:
						case  OBJ_SYSEXEND:
						case OBJ_NOTE:
						case  OBJ_PROGRAM:
						case  OBJ_CONTROL:
						case  OBJ_PITCHBEND:
						case  OBJ_SYSEX:
						case  OBJ_CHANNELPRESSURE:
						case  OBJ_POLYPRESSURE:
							{
								Seq_Event *toe=(Seq_Event *)toobject;
							}
							break;
						}

						// end of object event
					}
					break;

			}// switch eb->object

			if(again==false)
			{
				if(eb)
					eb=eb->NextElement();

				addtolastundo=true;
			}
		}
	}
}

void EditBuffer::CloseBuffer()
{
}

void EditBuffer::DeleteBuffer()
{
	EditBufferElement *fe=FirstEditBuffer();

	while(fe){

		if(fe->object)
			fe->object->Delete(true);

		fe=(EditBufferElement *)edits.RemoveO(fe);
	}
}

bool EditBuffer::CheckAllObjectsInBuffer(int id)
{
	EditBufferElement *eb=FirstEditBuffer();

	while(eb)
	{
		if(eb->object)
		{
			if(eb->object->CheckObjectID(id)==true)return true;

			switch(id)
			{
			case  OBJ_EVENTS:
				{
					switch(eb->object->id)
					{
					case  OBJ_NOTE:
					case  OBJ_PROGRAM:
					case  OBJ_CONTROL:
					case  OBJ_PITCHBEND:
					case  OBJ_SYSEX:
					case  OBJ_CHANNELPRESSURE:
					case  OBJ_POLYPRESSURE:
					case OBJ_ICDEVENT:
						return true;
					}
				}
				break;
			}
		}

		eb=eb->NextElement();
	}

	return false;
}

bool EditBuffer::CheckSubID(int subid)
{
	if(subid>=0)
	{
		EditBufferElement *ed=FirstEditBuffer();

		if(ed && ed->object && ed->object->GetSubID()==subid)
			return true;
	}

	return false;
}

bool EditBuffer::CheckHeadBuffer(int id)
{
	EditBufferElement *ed=FirstEditBuffer();

	if(ed && ed->object && ed->object->CheckObjectID(id)==true)
		return true;

	return false;
}

// Audio
void EditBuffer::RemoveRegion(AudioHDFile *hd,AudioRegion *deadregion)
{
	EditBufferElement *eb=FirstEditBuffer();

	while(eb)
	{
		if(eb->object)
		{
			switch(eb->object->id)
			{
			case  OBJ_AUDIOPATTERN:
				{
					// int i;
					// i=-1;
				}
				break;
			}
		}

		eb=eb->NextElement();
	}
}

void EditBuffer::DeleteBufferRegion(AudioRegion *r)
{
	EditBufferElement *ede=FirstEditBuffer();

	while(ede)
	{
		EditBufferElement *next=ede->NextElement();

		if(ede->object && ede->object->id== OBJ_AUDIOPATTERN)
		{
			AudioPattern *ap=(AudioPattern *)ede->object;

			if(ap->audioevent.audioregion==r)
			{
				ap->Delete(true); // Delete Audio Pattern
				edits.RemoveO(ede);
			}
		}

		ede=next;
	}
}

void EditBuffer::CreateAudioBuffer(AudioHDFile *audiohdfile,AudioRegion *region)
{
	if(audiohdfile==0 && region==0)
		return;

	if(audiohdfile)
	{
		if(AudioPattern *pattern=new AudioPattern)
		{
			pattern->audioevent.ostart=pattern->audioevent.staticostart=0; // important

			pattern->audioevent.audioefile=audiohdfile;

			if(region)
			{
				if(pattern->audioevent.audioregion=new AudioRegion)
				{
					region->CloneTo(pattern->audioevent.audioregion);
					pattern->audioevent.audioregion->InitRegion();
				}

				pattern->SetName(region->regionname);
			}
			else
				pattern->SetName(audiohdfile->GetName());

			mainbuffer->OpenBuffer();
			mainbuffer->AddObjectToBuffer(pattern,false);
			mainbuffer->CloseBuffer();
		}
	}
}


void EditBuffer::MixEventsToBuffer(Seq_SelectionList *list,bool selected)
{
	if(!list)return;

	Seq_SelectionEvent *ec=list->FirstMixEvent();

	while(ec)
	{
		if(ec->seqevent->IsSelected()==true)break;
		ec=ec->NextEvent();
	}

	if(ec)
	{
		TRACE("Mix Events to Buffer...\n");

		if(MIDIPattern *nmp=new MIDIPattern){

			mainbuffer->OpenBuffer();
			nmp->SetName("Event Mix Pattern");
			mainbuffer->AddObjectToBuffer(nmp,false); // no clone !

			while(ec)
			{
				if(ec->seqevent->IsSelected()==true)
					ec->seqevent->CloneNewAndSortToPattern(list->song,nmp,0);

				ec=ec->NextEvent();
			}

			mainbuffer->CloseBuffer();
		}
	}
}
