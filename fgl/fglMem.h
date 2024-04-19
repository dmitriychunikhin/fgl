#ifndef _FGLMEM_H_
#define _FGLMEM_H_

#include "common.h"
#include "fgl.h"
#include "fglMutex.h"
#include "fglLog.h"


class fglBITFLAG
{
private:
	unsigned val{ 0 };
public:
	fglBITFLAG(void) = default;
	fglBITFLAG(unsigned pnVal) { val = pnVal; }
	//bad for perf virtual ~fglBITFLAG(void) {}
	fglBITFLAG& operator =(unsigned pnVal) { val = pnVal; return *this; }

	inline unsigned get(void) const { return val; }
	inline bool check(unsigned pnVal) const { return (val & pnVal) != 0; }
	fglBITFLAG& set(unsigned pnVal) { val |= pnVal; return *this; }
	fglBITFLAG& reset(unsigned pnVal) { val = val & ~pnVal; return *this; }
};


template <class ttType> class fglREF
{
	template <class ttWeakType>
	friend class fglWeakREF;

	template <class ttOther>
	friend class fglREF;

private:

	std::shared_ptr<ttType> _data;
	unsigned _size{ 0 };

	struct array_deleter
	{
		void operator ()(ttType const * p)
		{
			delete[] p;
		}
	};

	inline void err(void) const { throw fglException("fglREF is empty"); }
	inline void null(void) { _data = std::shared_ptr<ttType>(); _size = 0; }

public:
	fglREF(void) = default;

	fglREF(const std::shared_ptr<ttType>& poSrc) { inst(poSrc); }

	template<class Other>fglREF(const fglREF<Other>& poSrc) { inst(std::dynamic_pointer_cast<ttType>(poSrc._data)); }

	inline bool operator==(const fglREF<ttType>& poOther) const { if (_data == poOther._data) return true; return false; }
	inline bool operator!=(const fglREF<ttType>& poOther) const { if (_data == poOther._data) return false; return true; }

	//bad for perf virtual ~fglREF(void) {  }

	inline ttType* map(void) const { return _data.get(); }
	inline ttType* map(unsigned pIndex) const { if (!_data) return NULL; return &(_data.get())[pIndex]; }
	inline ttType* operator->(void) const { return _data.get(); }
	inline ttType& operator[] (unsigned pIndex) const { return get(pIndex); }
	inline ttType& get(unsigned pIndex = 0) const { return *map(pIndex); }

	inline const unsigned int size(void) const { return _size; }
	inline const bool empty(void) const { return map() == NULL; }
	inline const bool exist(void) const { return map() != NULL; }

	inline long use_count(void) { return _data.use_count(); }

	inline const fglREF<ttType>& lock(void) const
	{
		return *this;
	}

	void release(void)
	{
		null();
	}

	void inst(const std::shared_ptr<ttType>& poSrc)
	{
		null();
		if (!poSrc) return;
		_data = poSrc;
		_size = 1;
	}

	void inst(const fglREF<ttType>& poSrc)
	{
		if (&poSrc == this) return;
		_data = poSrc._data;
		_size = poSrc._size;
	}

	void inst(const fglREF<ttType>* poSrc)
	{
		if (!poSrc) null(); else inst(*poSrc);
	}

	void create(unsigned int pnCount = 1)
	{
		null();
		if (pnCount == 0) return;
		_size = pnCount;
		if (pnCount == 1) _data = std::shared_ptr<ttType>(new ttType()); else _data = std::shared_ptr<ttType>(new ttType[pnCount](), array_deleter());
	}

	void set_size(unsigned int pnCount) { create(pnCount); }
	void clear(void) { null(); }

	void copy(const fglREF<ttType>* poSrc)
	{
		if (!poSrc) return;
		if (_data == poSrc->_data) return;
		if (poSrc->empty()) return;
		if (empty()) create(poSrc->size());
		for (unsigned i = 0; i < poSrc->size(); i++) map()[i] = poSrc->map()[i];
	}
};


template <class ttType> class fglWeakREF
{
private:
	std::weak_ptr<ttType> _data;

public:
	fglWeakREF(void) = default;

	fglWeakREF(const fglWeakREF<ttType>& poSrc) { _data = poSrc._data; }

	template<class Other>fglWeakREF(const fglWeakREF<Other>& poSrc) { _data = poSrc._data; }

	fglWeakREF(const fglREF<ttType>& poSrc) { _data = poSrc._data; }
	
	template<class Other>fglWeakREF(const fglREF<Other>& poSrc) { _data = std::dynamic_pointer_cast<ttType>(poSrc._data); }


	//bad for perf virtual ~fglWeakREF(void) {  }

	void release(void)
	{
		_data.reset();
	}

	inline fglREF<ttType> lock(void) const
	{
		return fglREF<ttType>(_data.lock());
	}

	inline const bool empty(void) const { return _data.expired(); }
	inline const bool exist(void) const { return !_data.expired(); }

};


class fglSEQUENCE
{
private:
	HFGL n{ 0 };
	fglSEQUENCE(const fglSEQUENCE&) = default; //nocopyable
protected:
	mutable fglMutex oMutex;
public:
	fglSEQUENCE(void) = default;
	//bad for perf virtual ~fglSEQUENCE(void) {}

	HFGL next(void)
	{
		HFGL lnNext;
		oMutex.lock();
		lnNext = ++n;
		oMutex.unlock();
		return lnNext;
	}

	HFGL last(void) const
	{
		HFGL lnLast;
		oMutex.lock();
		lnLast = n;
		oMutex.unlock();
		return lnLast;
	}
};


template <class ttType>
class fglSEQUENCE_UMAP : private fglSEQUENCE
{
private:
	fglUMap<HFGL, ttType> aSeq;
	fglArray<HFGL> aFreeHFGL;

public:
	fglSEQUENCE_UMAP(void) = default;
	//bad for perf virtual ~fglSEQUENCE_UMAP(void) { }

	HFGL add(const ttType& pVal)
	{
		HFGL lhFGL;
		oMutex.lock();
		if (!aFreeHFGL.empty()) {
			lhFGL = aFreeHFGL.back();
			aFreeHFGL.pop_back();
		}
		else
		{
			lhFGL = next();
		}
		aSeq[lhFGL] = pVal;
		oMutex.unlock();
		return lhFGL;
	}

	void free(HFGL phFGL)
	{
		if (phFGL == 0) return;
		oMutex.lock();
		if (aSeq.find(phFGL) != aSeq.end()) aSeq.erase(phFGL);
		aFreeHFGL.push_back(phFGL);
		oMutex.unlock();
	}

	template<typename tOnItem>
	void walkItems(const tOnItem& onItem)
	{
		oMutex.lock();
		auto liItem = aSeq.begin();
		for (; liItem != aSeq.end(); liItem++)
		{
			onItem(liItem->second);
		}
		oMutex.unlock();
	}
	
	const ttType* get(HFGL phFGL) const
	{
		if (phFGL == 0) return NULL;
		if (aSeq.find(phFGL) == aSeq.end()) return NULL;
		return &aSeq.at(phFGL);
	}

	unsigned size(void) const { return aSeq.size(); }
};


#endif
