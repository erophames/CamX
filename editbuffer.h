#ifndef CAMX_EDITBUFFER_H
#define CAMX_EDITBUFFER_H 1

#include "object.h"

class Seq_Song;
class AudioRegion;
class AudioHDFile;
class guiWindow;
class InsertAudioEffect;
class AudioEffects;
class Seq_SelectionList;

class EditBufferElement:public Object
{
	friend class EditBuffer;

public:
	EditBufferElement(){object=0;}
	Object *object;
	EditBufferElement *NextElement() {return (EditBufferElement *)next;}
};

class EditBuffer
{
public:
	enum PasteFlag{
		PASTE_NOOPENUNDO=1,
		PASTE_NOSYSTEMLOCK=2,
		PASTE_NOPLAYBACKCHECK=4,
		PASTE_NOGUIREFRESH=8
	};

	EditBufferElement *FirstEditBuffer() {return (EditBufferElement *)edits.GetRoot();}
	
	bool OpenBuffer();
	bool AddObjectToBuffer(Object *,bool clone); // copy -> buffer

	bool CheckBuffer(guiWindow *,Seq_Song *,Object *toobject);
	void PasteBuffer(guiWindow *,Seq_Song *,Object *toobject,OSTART position,int flag=0); // try to paste
	
	void CopyEffectList(AudioEffects *);
	void PasteBufferToEffectList(AudioEffects *);
	void PasteBufferToEffect(Seq_Song *,AudioEffects *,InsertAudioEffect *);
	void CloseBuffer();
	void DeleteBuffer();

	// Audio
	void CreateAudioBuffer(AudioHDFile *,AudioRegion *);
	void DeleteBufferRegion(AudioRegion *);

	// Check Buffer Objects
	bool CheckAllObjectsInBuffer(int id);
	bool CheckHeadBuffer(int id);
	bool CheckSubID(int subid);
	void RemoveRegion(AudioHDFile *,AudioRegion *deadregion);
	void MixEventsToBuffer(Seq_SelectionList *,bool selected);

	//int lowestnote_key,highestnote_key,tonotekey;

private:
	OList edits;
};
#endif
