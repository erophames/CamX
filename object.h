#ifndef CAMX_OBJ_H
#define CAMX_OBJ_H 1

#define CLONEFLAG_NODATA (1<<1)
#define CLONEFLAG_NOFX (1<<2)
#define CLONEFLAG_ERASECLONEDATA (1<<3)

#define OINDEXS 16

class OList;
class OListStart;
class camxFile;
class Seq_Song;
class guiGadget_Slider;
class guiGadget_CW;
class guiGadget_TabStartPosition;

#ifdef WIN32
#include <AFXMT.h>
#endif

#include "defines.h"

enum{
	OID_SCREEN,
	OID_WINDOW,
	OID_BUTTON,

	OI_LAST
};

enum{
	OFLAG_SELECTED=1,
	OFLAG_UNDERSELECTION=(1<<1),
	OFLAG_OLDUNDERSELECTION=(1<<2),
	OFLAG_MOUSEOVER=(1<<3),
	OFLAG_PSELECTION=(1<<4),

	OFLAG_SPECIALFLAGS=5 // last (1<<OFLAG_SPECIALFLAGS)
};

class Object{

	friend OList;

public:
	Object(){olist=0;flag=0;}

	virtual void Load(camxFile *){}
	virtual void Save(camxFile *){}
	virtual void RefreshUndos(){}
	virtual bool CheckObjectID(int cid){return cid==id?true:false;}
	virtual int GetSubID(){return -1;} // -1=no sub ID
	virtual void Delete(bool full){}
	virtual Object *Clone(){return 0;}
	virtual char *GetDragDropInfoString(){return 0;}

	inline Object *NextOrPrev(){return next?next:prev;}
	OList *GetList (){return olist;}
	void SetList(OList *l){olist=l;}

	bool IsSelected(){return flag&OFLAG_SELECTED?true:false;}
	void Select(){flag|=OFLAG_SELECTED;}
	void UnSelect(){flag CLEARBIT OFLAG_SELECTED;}
	void PRepairSelection(){if(IsSelected()==false)flag|=OFLAG_UNDERSELECTION;}
	int GetIndex(){return index;}

	Object *next,*prev;
	OList *olist;
	int id,flag,index;
};

class OObject:public Object{

public:
	OObject(Object *o){object=o;}
	Object *object;
	OSTART startposition;
	int cx,cy,cw,ch,
		zx,zy,zoomw,zoomh, // Zooms Coos
		offsetw,offseth;
};

class OStart:public Object{
public:
	OListStart *GetList (){return (OListStart *)olist;}
	inline OStart *Prev(){return (OStart *)prev;}
	inline OStart *Next(){return (OStart *)next;}
	inline OSTART GetOStart(){return ostart;}

	virtual LONGLONG GetSampleStart(Seq_Song *);
	bool CheckIfInRange(OSTART,OSTART);
	
	OSTART ostart; // ticks
};

class ObjectLock:public Object
{
	friend OList;

public:
#ifdef WIN32
	virtual void LockO(){lock_semaphore.Lock();}
	virtual void UnlockO(){lock_semaphore.Unlock();}
	CCriticalSection lock_semaphore;
#endif
};

class OOList
{
public:
	OOList(){
		objects=0;
		number=0;
		list=0;
		index=0;
	}

	~OOList(){FreeMemory();}

	void InitOO(int c);
	void FreeMemory();

	Object **objects;
	OList **list;
	int *index,number;
	bool movedown;
};

enum{
	OLISTTYPE_LIST,
	OLISTTYPE_FOLDER,
	OLISTTYPE_FOLDERPARENT
};

class OList
{	
public:
	OList();

	void Reset(int nrnew);
	void Clear();

	void AddPrevO(Object *,Object *prev); // prev->object Close required
	void AddNextO(Object *,Object *next); // object->next, Close required
	void AddStartO(Object *);
	void AddEndO(Object *);
	void AddOToIndex(Object *,int); // index object->o

	void MoveO(Object *,int diff);
	void MoveOToEndOList(OList *list,Object *o){CutQObject(o);list->AddEndO(o);}
	void MoveOToStartOList(OList *list,Object *o){CutQObject(o);list->AddStartO(o);}
	void MoveListToList(OList *);
	bool MoveOToIndex(Object *,int toindex);
	bool MoveOIndex(Object *,int index); // Up/Down
	
	void MoveSelectedObjects(OOList *,int diff);
	void MoveObjects_Undo(OOList *);

	void SelectAll();
	void DeSelectAll();
	void Select(bool sel){if(sel==true)SelectAll();else DeSelectAll();}
	void SetSelectionStart(Object *);
	void SetSelectionEnd(Object *);
	void SelectSelection();

	Object *SearchO(Object *);
	Object *GetO(int);
	
	Object *CutObject(Object *);
	void CutQObject(Object *);
	Object *RemoveO(Object *); // Cut + Delete
	void DeleteAllO();

	//void ReplaceO(Object *old,Object *n);
	Object *GetRoot(){return c_root;}
	Object *Getc_end(){return c_end;}
	void SetRoot(Object *o){c_root=o;}
	void Setc_end(Object *o){c_end=o;}
	int GetCount(){return objectsinlist;}
	int GetCountSelectedObjects();

	virtual void Close();

#ifdef DEBUG
	bool CheckIndex();
#endif

	Object *c_root,*c_end,*parent,*activeobject,*selectionstart,*selectionend;
	int listtype,objectsinlist,accesscounter;
};

enum{
	COOSTYPE_HORZ,
	COOSTYPE_VERT
};

class OListCoos:public OList
{
public:
	OListCoos(){
		startx=starty=zoomwidth=0;
		cursorobject=0;
	}

	OObject *FindObject(Object *);
	OObject *FindPrevSameObject(OObject *);
	OObject *FindNextSameObject(OObject *);
	void DeleteAllO(guiGadget_CW *);

	// X
	void InitXStart(int divx);
	void InitXStartO();
	void OptimizeXStart(); // Center etc..
	void NextXO();

	int GetInitX(){return initx;}
	int AddX(int x){initx+=x;return initx;}
	int GetInitX2(){return showobject && showobject->zoomw>0?initx+showobject->zoomw-1:initx;}

	// Y
	void InitYStart(int divy);
	void InitYStartO();
	void NextYO();

	int GetInitY(){return inity;}
	int AddY(int y){inity+=y;return inity;}
	int GetInitY2(){return showobject && showobject->ch>0?inity+(showobject->ch-1):inity;}
	OObject *GetShowObject(){return showobject;}
	Object *GetCursor(){return cursorobject;}
	void SetCursor(Object *o){cursorobject=o;}

	void UnSelectAll(OObject *not);

	Object *cursorobject;
	OObject *showobject;
	int type,startobjectint,startx,starty,width,height,zoomwidth,guiwidth,guiheight,initx,inity;
};

class OListCoosX:public OListCoos
{
public:
	OListCoosX(){type=COOSTYPE_HORZ;}

	void AddCooObject(Object *o,int w,int zoomw,int offsetw);
	void EndBuild();
	void InitWithSlider(guiGadget_Slider *);
	bool AddStartX(int addx);
	bool ZoomX(double per);
	int AddInitX(int addx);
	void SetWidth(int w){width=w;}
};

class OListCoosY:public OListCoos
{
public:
	OListCoosY(){
		type=COOSTYPE_VERT;
		offsetymul=0;
		startsetbyslider=false;
	}

	void ResetSlider(){startsetbyslider=false;}

	void AddCooObject(Object *,int h,int offseth=0);
	OObject *AddCooObjectR(Object *,int h,int offseth=0);
	OObject *AddCooObjectRStart(Object *,OSTART spp,int h,int offseth=0);

	void StartBuild(guiGadget_CW *);
	void DrawUnUsed(guiGadget_CW *);

	void ScrollToEnd();
	void EndBuild(guiGadget_Slider *sl=0);
	void InitWithSlider(guiGadget_Slider *,bool startsetbyslider=false);
	bool InitWithPercent(double per);

	void BufferYPos(); // 1.
	void RecalcYPos(); // 2.

	bool AddStartY(int addy);
	bool SetStartY(int y);
	bool SetStartY_Mid(int y);

	void ScrollY(OObject *);
	void InitStartOffSetY(int offsetmul);
	bool InitYStartOStart(OSTART);

	int AddInitY(int addy);
	void SetHeight(int h){height=h;}
	void CalcStartYOff(int zoomy,int objectsondisplay,int from,int to);
	void SetMaxHeight(int fromy,int h){if (h-fromy>height)height=h-fromy;}

	void SetGUIHeight(guiGadget_CW *);
	void SetSlider(guiGadget_Slider *);

	bool Select(guiGadget_TabStartPosition *,int tabindex,bool unselectother=false);

	int GetYPosHiLo(double pos,double range);

	double startpery;
	int offsetymul;
	bool startsetbyslider;
};

class OListStart:public OList
{
public:
	OStart *GetRoot(){return (OStart *)c_root;}
	OStart *Getc_end(){return (OStart *)c_end;}
	void MoveO(OStart *,OSTART);

	OStart *FindOBefore(OSTART);//<-| Tempo, Signature
	OStart *FindObject(OSTART);// |->
	OStart *FindObjectAtPos(OSTART);// ==

	void MoveAll(OSTART ticks);
	void MoveOToOList(OListStart *list,OStart *o){CutQObject(o);list->AddOSort(o,o->ostart);}

	void AddOSort(OStart *o){AddPrevO(o,FindOBefore(o->ostart));}
	void AddOSort(OStart *o,OSTART start){o->ostart=start;AddPrevO(o,FindOBefore(start));}

	void AddStartS(OStart *,OSTART);
	void AddEndS(OStart *,OSTART);
	void AddPrevS(OStart *,OStart *prev,OSTART start);
	void AddNextS(OStart *,OStart *next,OSTART start);
};

class OListStartIndex:public OListStart
{
public:

	OListStartIndex()
	{
		ResetIndex();
	}

	OStart *FindOBefore(OSTART);//<-| Tempo, Signature
	OStart *FindObject(OSTART);// |->
	OStart *FindObjectAtPos(OSTART);// ==

	void EndIndex();
	void ResetIndex();

	OStart *index[OINDEXS];
};

class FolderObject:public Object // Tracks etc...
{
public:
	FolderObject()
	{
		parent=0;
		childdepth=0;
		showchilds=true;
	}

	FolderObject *PrevQObject(){return qindex==0?0:(FolderObject *)qlist->objects[qindex-1];}
	FolderObject *NextQObject(){return qindex<qlist->number-1?(FolderObject *)qlist->objects[qindex+1]:0;}

	FolderObject *LastFolderObject();
	FolderObject *PrevFolderObject();
	FolderObject *NextFolderObject();
	FolderObject *PrevOpenFolderObject();
	FolderObject *NextOpenFolderObject();
	void AddChildObject(FolderObject *,int index);
	FolderObject *FirstChild(){return (FolderObject *)childs.GetRoot();}
	FolderObject *NextChild(){return (FolderObject *)next;}
	FolderObject *NextFolder(){return (FolderObject *)next;}

	FolderObject *NextChildNot(FolderObject *c);
	FolderObject *LastChild();
	bool CheckIfCanBeMove(int diff);
	bool IsOpen();
	void Close();

	OList childs;
	FolderObject *parent;
	OOList *qlist;
	int parentindex,childdepth,qindex; 
	bool showchilds;
};

class FolderList:public OList
{
public:
	FolderList(){listtype=OLISTTYPE_FOLDER;}

	int GetIx(FolderObject *);
	FolderObject *GetO(int);
	FolderObject *FirstFolderObject(){return (FolderObject *)GetRoot();}
	void CutObject(FolderObject *o){o->olist->CutQObject(o);}
	void CreateList(OOList *);
	void ListToFolder(OOList *l);
	void CreateSelectedList(OOList *);
	bool CheckIfMoveSelected(int diff);
	bool CheckIfSelectedCanBeMovedTo(FolderObject *to);

	void MoveObjectsToObject(OOList *,FolderObject *);
	void MoveSelectedObjectsToObject(OOList *,FolderObject *to);
	void MoveObjects_Undo(OOList *);

	void ReleaseSelectedChildObjects(OOList *,FolderList *tolist);
	void RepairChilds();
	void Close();

};

#endif
