#ifndef _FGLBIND_H_
#define _FGLBIND_H_
#include "common.h"
#include "pro_ext2.h"
#include "fgl.h"
#include "fglLog.h"

////////////////////////////////////////////////////
class fglEventParams
{
public:
	FoxParameter* pParameters{ NULL };
	int nParams{ 0 };
	Value defVal;
	
	fglEventParams(FoxParameter* ppParameters, int pnParams): pParameters(ppParameters), nParams(pnParams) { defVal.ev_type = 'L'; defVal.ev_length = 0; }
	Value& operator[] (int i) { return i>=nParams ? defVal : pParameters[i].val; }
};

////////////////////////////////////////////////////
class fglEvent
{
public:
	int name;
	fglEventParams& aParams;
	int nParams; //the number of parameters passed

	fglEvent(int pnName, fglEventParams& paParams, int pnParams=0): name(pnName), aParams(paParams), nParams(pnParams) { }
};


class fglScene;

////////////////////////////////////////////////////
class fglExtVal
{
private:
	char cType{ 0 };
	int vI{ 0 };
	float vF{ 0.0f };
	fglString vC{ "" };
	bool vL{ false };

public:
	fglExtVal(void) = default;
	//bad for perf virtual ~fglExtVal(void) {}
	
	void clear(void) {cType=0; vI=0; vF=0.0f; vC=""; vL=false;}
	
	void setNull(void) { clear(); }
	void setVal(int pVal) { clear(); cType='I'; vI=pVal; }
	void setVal(float pVal) { clear(); cType='F'; vF=pVal; }
	void setVal(const fglString& pVal) { clear(); cType='C'; vC=pVal; }
	void setVal(bool pVal) { clear(); cType='L'; vL=pVal; }

	char getType(void) const { return cType; }
	bool getNull(void) const { return cType==0; }
	int getValI(void) const { return vI; }
	float getValF(void) const { return vF; }
	const fglString& getValC(void) const { return vC; }
	bool getValL(void) const { return vL; }
};

////////////////////////////////////////////////////
class fglExtProps
{
private:
	typedef fglMap<fglString, fglExtVal> APROPS;
	APROPS aProps;
public:
	fglExtProps(void) = default;
	//bad for perf virtual ~fglExtProps(void) {}
	
	void set(const fglString& pcName, const fglExtVal& poVal) { aProps[pcName] = poVal; }
	const fglExtVal* get(const fglString& pcName) const { if (aProps.find(pcName) == aProps.end()) return NULL; return &aProps.at(pcName); }
};


class fglBind;
////////////////////////////////////////////////////
class fglBindFactory
{
public:
	enum DuplicateMode { fglDUPLICATE_NEVER = 0, fglDUPLICATE_COPY = 1, fglDUPLICATE_INSTANCE = 2, fglDUPLICATE_ANY = 3 };

private:

	fglBindFactory(void) = default;
	
	enum PersistentType { Persistent, NonPersistent };

	class BINDREF
	{
	public:
		fglWeakREF<fglBind> oWeakRef;
		fglREF<fglBind> oRef;

		BINDREF(void) = default;
		
		BINDREF(fglWeakREF<fglBind> poWeakRef, fglREF<fglBind> poRef)
		{
			oWeakRef = poWeakRef;
			oRef = poRef;
		};
	};


	typedef fglSEQUENCE_UMAP<BINDREF> FGLABINDS;
	static FGLABINDS aBinds;


	template <class ttType>
	static fglREF<ttType> create(PersistentType tPersistent)
	{
		fglREF<ttType> loBind = fglREF<ttType>(std::shared_ptr<ttType>(new ttType()));
		fglREF<fglBind> loBindBase = loBind;

		if (loBindBase.empty()) throw fglException(std::string("fglBindFactory::create: class is not derived from fglBind"));

		if (tPersistent == Persistent) { loBindBase->hFGL = aBinds.add(BINDREF(fglWeakREF<fglBind>(loBindBase), loBindBase)); }
		else if (tPersistent == NonPersistent) { loBindBase->hFGL = aBinds.add(BINDREF(fglWeakREF<fglBind>(loBindBase), fglREF<fglBind>())); }

		loBindBase->oRef = loBindBase;

		if (std::is_same<ttType, fglScene>::value) { loBind->setScene(loBind); }

		return loBind;
	}

	static fglREF<fglBind> createByClassName(PersistentType tPersistent, fglString pcClassname);


	template <class ttType>
	static fglREF<ttType> duplicate(PersistentType tPersistent, const fglREF<ttType>& poSrc, DuplicateMode pnDuplicateMode, fglBITFLAG pnShare)
	{
		if (poSrc.empty()) return fglREF<ttType>();
		
		DuplicateMode lnDuplicateMode = static_cast<DuplicateMode>(poSrc->duplicateMode);
		
		if (lnDuplicateMode == fglDUPLICATE_ANY) lnDuplicateMode = pnDuplicateMode;
		
		if (lnDuplicateMode == fglDUPLICATE_NEVER) return fglREF<ttType>();

		if (lnDuplicateMode == fglDUPLICATE_COPY)
		{
			fglREF<ttType> loDuplicate;
			if (tPersistent == Persistent) loDuplicate = fglBindFactory::createPersistentByClassName(poSrc->fgl_class());
			else if (tPersistent == NonPersistent) loDuplicate = fglBindFactory::createNonPersistentByClassName(poSrc->fgl_class());
			
			if (loDuplicate.empty()) { return loDuplicate; }
			
			loDuplicate->copy(poSrc.map(), pnShare);

			return loDuplicate;
		}
		else if (lnDuplicateMode == fglDUPLICATE_INSTANCE)
		{
			return poSrc->getRef();
		}
		return fglREF<ttType>();
	}


public:

	template <class ttType>
	static fglREF<ttType> createPersistent(void) { return create<ttType>(Persistent); }

	template <class ttType>
	static fglREF<ttType> createNonPersistent(void) { return create<ttType>(NonPersistent); }

	static fglREF<fglBind> createPersistentByClassName(fglString pcClassname) { return createByClassName(Persistent, pcClassname); }

	static fglREF<fglBind> createNonPersistentByClassName(fglString pcClassname) { return createByClassName(NonPersistent, pcClassname); }

	template <class ttType>
	static fglREF<ttType> duplicatePersistent(const fglREF<ttType>& poSrc, unsigned pnDuplicateMode = fglDUPLICATE_COPY, fglBITFLAG pnShare = 0) { return duplicate<ttType>(Persistent, poSrc, static_cast<DuplicateMode>(pnDuplicateMode), pnShare); }

	template <class ttType>
	static fglREF<ttType> duplicateNonPersistent(const fglREF<ttType>& poSrc, unsigned pnDuplicateMode = fglDUPLICATE_COPY, fglBITFLAG pnShare = 0) { return duplicate<ttType>(NonPersistent, poSrc, static_cast<DuplicateMode>(pnDuplicateMode), pnShare); }


	static void releaseBind(HFGL phFGL);

	static void releaseBind(const fglREF<fglBind>& poBind);

	static void unload(void);

	static void freeBind(HFGL phFGL);



	template<class ttType>
	static fglREF<ttType> getBind(HFGL phFGL)
	{
		const BINDREF* loBindREF = aBinds.get(phFGL);
		if (!loBindREF) return fglREF<ttType>();
		return loBindREF->oWeakRef.lock();
	}


};

////////////////////////////////////////////////////
template <class ttType>
class fglBindREF
{
private:
	fglREF<ttType> oBindRef;
	fglBindREF(const fglBindREF& poSrc) = default; //nocopyable

public:
	fglBindREF(void) { oBindRef = fglBindFactory::createNonPersistent<ttType>(); }
	inline const fglREF<ttType>& operator->() const { return oBindRef; }
	inline ttType& callBindREFGet(void) const { return oBindRef.get(); }
	inline ttType* callBindREFMap(void) const { return oBindRef.map(); }
};

////////////////////////////////////////////////////
class fglBind
{
	friend class fglBindFactory;

protected:
	fglBind(void) = default;

private:
	
	HFGL hFGL{ 0 };

	fglWeakREF<fglBind> oRef;
		
	fglBind(const fglBind& poSrc) = default; //nocopyable
	fglWeakREF<fglScene> oScene;

	fglExtProps* aExtProps{ NULL };


public:

	fglString name{ "" };

	enum { fglDUPLICATE_NEVER = 0, fglDUPLICATE_COPY = 1, fglDUPLICATE_INSTANCE = 2, fglDUPLICATE_ANY = 3 };
	unsigned duplicateMode{ fglDUPLICATE_ANY };

	virtual fglString fgl_class(void) const { return "bind"; }
	
	
	virtual ~fglBind(void);
	
	const fglBind& operator=(const fglBind& poSrc) {  copy(&poSrc); return *this; }

	HFGL getHFGL(void) const {return hFGL;}

	template<class ttType> 
	fglREF<ttType> getRef(void) const { return oRef.lock(); }
	fglREF<fglBind> getRef(void) const { return oRef.lock(); }
	
	template<class ttType> 
	static fglREF<ttType> getBind(HFGL phFGL) 
	{ 
		return fglBindFactory::getBind<ttType>(phFGL);
	}
	

	fglWeakREF<fglScene> getScene(void) const;
	bool setScene(const fglWeakREF<fglScene>& poScene);
	
	fglExtProps& getExtProps(void);
	void setExtProps(const fglExtProps* poExtProps);
	
	virtual void call(fglEvent& poEvent);

	void setSceneRenderFlag(void);

	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);


};


////////////////////////////////////////////////////
template<class ttType, class ttItemREF=fglREF<ttType>> class fglCollection : public fglBind
{
	friend class fglBindFactory;

protected:
	fglCollection(void) = default;

private:
	typedef fglArray<ttItemREF> AITEMS;
	typedef fglUMap<const void*, unsigned> AITEMSHASH;
	fglREF<AITEMS> aItems;
	fglREF<AITEMSHASH> aItemsHash;


public:
	
	//bad for perf virtual ~fglCollection(void) { clear(); }
	
	typedef typename AITEMS::const_iterator iterator;
	
	inline fglREF<ttType> operator[] (unsigned pIndex) const { return get(pIndex); }

	virtual bool add(const fglREF<ttType>& poItem) 
	{ 
		const void* poItemPtr = poItem.lock().map();
		
		if (aItems.empty()) { aItems.create(); aItemsHash.create(); }
		

		AITEMSHASH::iterator iItem = aItemsHash->find(poItemPtr);
		if (iItem != aItemsHash->end())
		{
			if (iItem->first == aItems->operator[](iItem->second).lock().map())
			{
				return false;
			}
		}

		aItems->push_back(poItem);
		aItemsHash->insert({ poItemPtr, aItems->size() - 1 });

		return true;
	}

	virtual bool del(const fglREF<ttType>& poItem) 
	{ 
		if (aItems.empty()) return false;
		
		const void* poItemPtr = poItem.lock().map();
		
		if (!poItemPtr) return false;

		AITEMSHASH::iterator iItem = aItemsHash->find(poItemPtr);
		if (iItem == aItemsHash->end()) return false;
		aItems->erase(aItems->begin() + iItem->second);
		aItemsHash->erase(iItem);
		return true;
	}
	

	inline fglREF<ttType> get(unsigned pIndex) const { return aItems->at(pIndex).lock(); }

	fglREF<ttType> getsafe(unsigned pIndex) const
	{ 
		if (pIndex<0 || pIndex >= size()) return fglREF<ttType>();
		return get(pIndex);
	}

	virtual int find (const fglREF<ttType>& poItem) const 
	{ 
		const void* poItemPtr = poItem.lock().map();
		if (!poItemPtr) return -1;
		if (aItems.empty()) return -1;

		AITEMSHASH::iterator iItem = aItemsHash->find(poItemPtr);
		if (iItem == aItemsHash->end()) return -1;
		else if (iItem->first != aItems->operator[](iItem->second).lock().map()) return -1;
		
		return iItem->second;
	}

	iterator begin(void) const { if (aItems.empty()) return iterator(); return aItems->cbegin(); }
	iterator end(void) const { if (aItems.empty()) return iterator(); return aItems->cend(); }

	inline unsigned size(void) const { if (aItems.empty()) return 0; return aItems->size(); }
	inline bool empty(void) const { return size()==0; }
	inline bool exist(void) const  { return size()!=0; }
	virtual void clear(void) { if (aItems.exist()) { aItems->clear(); aItemsHash->clear(); }}

	void reserve(unsigned pnSize) 
	{ 
		if (pnSize==0) return; 
		//if (aItems.empty()) { aItems.create(); aItemsHash.create(); }
		//aItems->reserve(pnSize);
		//aItemsHash->reserve(pnSize);
	}
	
	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0) 
	{
		if ( !fglBind::copy(poSrc, pnShare) ) return false;
		const fglCollection* loSrc = dynamic_cast<const fglCollection*>(poSrc);
		if (!loSrc) return false;

		if (pnShare.get())
		{
			aItems.inst(&loSrc->aItems); 
		}
		else
		{
			aItems.copy(&loSrc->aItems);
		}
		return true;
	}

private:

	template<class ttTypeFunc = ttType>
	typename std::enable_if<std::is_base_of<fglBind, ttTypeFunc>::value, void>::type
	_callItem(fglEvent& poEvent)
	{
		if (poEvent.name == fglEVENT_add)
		{
			_RetValue2l(0);
			if (!poEvent.nParams) return;
			const fglREF<ttType>& loItem = getBind<ttType>(_GetValue2i(poEvent.aParams[0]));
			if (loItem.exist()) _RetValue2l(add(loItem));
			return;
		}
		if (poEvent.name == fglEVENT_del)
		{
			_RetValue2l(0);
			if (!poEvent.nParams) return;
			const fglREF<ttType>& loItem = getBind<ttType>(_GetValue2i(poEvent.aParams[0]));
			if (loItem.exist()) _RetValue2l(del(loItem));
			return;
		}

		if (poEvent.name == fglEVENT_find)
		{
			_RetValue2i(-1);
			if (!poEvent.nParams) return;
			const fglREF<ttType>& loItem = getBind<ttType>(_GetValue2i(poEvent.aParams[0]));
			if (loItem.exist()) _RetValue2i(find(loItem));
			return;
		}

		if (poEvent.name == fglEVENT_get)
		{
			fglBind* loItem(NULL);
			if (poEvent.nParams) loItem = dynamic_cast<fglBind*>(getsafe(_GetValue2i(poEvent.aParams[0])).lock().map());
			_RetValue2i(loItem ? loItem->getHFGL() : 0);
			return;
		}
	}

	template<class ttTypeFunc = ttType>
	typename std::enable_if<!std::is_base_of<fglBind, ttTypeFunc>::value, void>::type
	_callItem(fglEvent& poEvent)
	{

	}


public:
	virtual void call(fglEvent& poEvent)
	{
		_callItem(poEvent);

		if (poEvent.name == fglEVENT_reserve) { if (poEvent.nParams) reserve(_GetValue2i(poEvent.aParams[0])); return; }
		if (poEvent.name == fglEVENT_clear) { clear(); return; }
		if (poEvent.name == fglEVENT_size) { _RetValue2i(size()); return; }

		fglBind::call(poEvent);
	}

};

template<class ttType>
using fglCollectionWeak = fglCollection<ttType, fglWeakREF<ttType>>;

#endif