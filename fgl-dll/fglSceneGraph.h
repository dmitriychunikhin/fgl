#ifndef _FGLSCENEGRAPH_H_
#define _FGLSCENEGRAPH_H_

#include "fgl3DGen.h"
#include "fglBind.h"
#include "fglMaterial.h"
#include "fglAnim.h"

class fglViewport;


class fglVisual : public fglBind
{
	friend class fglBindFactory;

protected:
	fglVisual(void);

public:

	fglBBox bbox;
	fgl3DRect bboxInt;
	fgl3DRect bboxExt;
	fgl3DRect viewRangeDist;
	fgl3DRect viewRangeRot;

	enum { 
		nStretchNone = 0, 
		nStretchClip = 1, 
		nStretchRepeat = 2, 
		nStretchFit = 4, 
		nStretchProper = 8, 
		nStretchNoX = 16, 
		nStretchNoY = 32, 
		nStretchNoZ = 64, 
		nStretchAlignBMinX = 128, 
		nStretchAlignBMaxX = 256, 
		nStretchAlignBMinY = 512,
		nStretchAlignBMaxY = 1024,
		nStretchAlignBMinZ = 2048,
		nStretchAlignBMaxZ = 4096,
		nStretchAlignCenter = 8192
	};
	
	fglBITFLAG nStretch; 
	fglREF<fglCLIP> oStretchClip;

	fglREF<fglMaterial> oMaterial;

	fglBindREF<fglCollection<fglVisual>> aLODs;
	
	//bad for perf virtual ~fglVisual(void);

	virtual fglString fgl_class(void) const { return "visual"; }
	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);
	void copyLODs(const fglVisual* poVisual, fglBITFLAG pnShare=0);
	
	virtual void calcBounds(void);

	//virtual void renderMesh(void) const;
	static void renderVisual(const fglVisual* poVisual);
	void (*renderImpl)(const fglVisual*);

	void render(void) const;
	const fglVisual* renderGetLOD(void) const;
	void renderStretch(void) const;
	
	virtual bool testIntersection(fglRAY& poRay, fglVECTOR* poPointIn=NULL) const;
	bool testInFrustum(const fglViewport& poViewport, const fglTRANSFORM& poTRG) const;
	bool testIsVisible(const fglViewport& poViewport, const fglTRANSFORM& poTRG) const;

	virtual void call(fglEvent& poEvent);
};


class fglNode : public fglBind
{
	friend class fglBindFactory;

protected:
	fglNode(void);

private:
	template <class ttType> class ACHILDS : public fglCollection<ttType>
	{
		friend class fglBindFactory;

	protected:
		ACHILDS(void) = default;

	public:
		fglNode* oParent{ NULL };

		//bad for perf virtual ~ACHILDS(void) {}
		virtual bool add(const fglREF<ttType>& poItem)
		{
			if ( !fglCollection<ttType>::add(poItem) ) return false;
			if (oParent) if ( ! poItem->setParentValue(oParent->getRef<ttType>()) ) { del(poItem); return false; }
			return true;
		}
		virtual bool del(const fglREF<ttType>& poItem)
		{
			if ( !fglCollection<ttType>::del(poItem) ) return false;
			if (poItem.exist()) poItem->setParent(fglREF<fglNode>());
			return true;
		}
		virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0) { return false; }
		virtual void clear(void) { while (size()) del(get(0)); fglCollection<ttType>::clear(); }
	};


	fglWeakREF<fglNode> oParent;

protected:
	bool setParentValue(const fglREF<fglNode>& poParent);

public:
	fglBindREF<ACHILDS<fglNode>> aChilds;
	fglBindREF<fglCollection<fglAnimChanEval>> aAnimChans;
	fglBindREF<fglCollection<fglVisual>> aVisuals;

	fglREF<fglNode> getParent(void) const { return oParent.lock(); }
	virtual bool setParent(const fglREF<fglNode>& poParent);
	const ACHILDS<fglNode>& getChilds(void) const { return aChilds.callBindREFGet(); }
	
	template <class ttType> 
	const fglREF<ttType>* find(const fglString& pcName) const;
	const fglREF<fglNode>* find(const fglString& pcName) const { return find<fglNode>(pcName); }

	fglTRANSFORM tr;
	fglTRANSFORM trg;
	fglMAT4 trgMatrix;
	fglTRANSFORMConstraints trgConstraints;
	fglVECTOR oLookVector;
	fglVECTOR oLookVectorLocal;
	fglVECTOR oLookVectorGlobal;
	fglBBox bbox;
	fgl3DRect bboxExt;
	bool isEnabled;
	bool isVisible;
	
	//bad for perf virtual ~fglNode(void);
	virtual void destroy(void);
	
	virtual fglString fgl_class(void) const { return "node"; }
	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);

	virtual void buildTransform(void);
	void calcBounds(void); 
	
	bool testInFrustum(const fglViewport& poViewport) const;

	void getVisuals(const fglString& pcVisualName, const fglString& pcMaterialName, fglArray<fglVisual*>& paResult);
	void getMaterials(const fglString& pcVisualName, const fglString& pcMaterialName, fglArray<fglMaterial*>& paResult);

	virtual void call(fglEvent& poEvent);

	void rotateBy(fglQUAT poRot, const fglVECTOR* poCenter=NULL) { tr.rotateBy(poRot, poCenter); buildTransform(); }
	
	void rotateTo(const fglQUAT& poRot, const fglVECTOR* poCenter=NULL) { tr.rotateTo(poRot, poCenter); buildTransform(); }
	
	void moveBy(const fglVECTOR& poMove) { tr.moveBy( poMove ); buildTransform(); }

	void moveDir(const fglVECTOR& poMove) { tr.moveDir( poMove ); buildTransform(); }

	void moveTo(const fglVECTOR& poMove) { tr.moveTo(poMove); buildTransform(); }

	void scaleBy(const fglVECTOR& poScale) { tr.scaleBy(poScale); buildTransform(); }
	
	void scaleAt(const fglVECTOR& poScale) { tr.scaleAt(poScale); buildTransform(); }
	
	void scaleTo(const fglVECTOR& poScale) { tr.scaleTo(poScale); buildTransform(); }

	void lookAt(const fglVECTOR& poLookAt, const float& pnDist=0.0f);

	bool getLookAtPlane(const fglROW4& poPlane, fglVECTOR& poLookAt) const;

};

template <class ttType> 
const fglREF<ttType>* fglNode::find(const fglString& pcName) const
{
	if (name == pcName) return &getRef<ttType>();
	ttType* loNode = NULL;
	const fglREF<ttType>* loFind;
	for (unsigned i = 0; i < aChilds->size(); i++)
	{
		if ( (loNode = dynamic_cast<ttType*>(aChilds->get(i).map()))==NULL ) continue;
		if ( loFind = loNode->find<ttType>(pcName) ) return loFind;
	}
	return NULL;
};
/////////////////////////////////////////////////////////////////////////////////////////////


class fglWalkNodes
{
private:
	unsigned aStack[10000];
	unsigned nStack{ 0 };

public:
	fglTRANSFORM currentView;
	fglMAT4 currentViewMatrix;


	bool(*onNode)(fglWalkNodes*, fglNode*) { NULL };
	void(*onVisual)(fglWalkNodes*, fglNode*, fglVisual*) { NULL };
	fglWalkNodes(void) = default;
	
	static void walkNodes(fglNode* poNode, fglWalkNodes* poHandler);
	
};

#endif