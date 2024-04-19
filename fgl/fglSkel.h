#ifndef _FGLSKEL_H_
#define _FGLSKEL_H_

#include "fglSceneGraph.h"

class fglJoint : public fglNode
{
	friend class fglBindFactory;

protected:
	fglJoint(void) = default;

public:	
	//bad for perf virtual ~fglJoint(void) {}
	virtual fglString fgl_class(void) const { return "joint"; }

	virtual bool setParent(const fglREF<fglNode>& poParent);

	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);
};


struct fglBONE_INDEX_WEIGHT
{
public:
	fglINDEX index{ 0 };
	float weight{ 0.0f };
};

class fglBone : public fglNode
{
	friend class fglBindFactory;

protected:
	fglBone(void) = default;

public:
	fglREF<fglBONE_INDEX_WEIGHT> aVerts;
		
	//bad for perf virtual ~fglBone(void) {}
	virtual fglString fgl_class(void) const { return "bone"; }
	
	virtual bool setParent(const fglREF<fglNode>& poParent);
	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);
};

#endif