#ifndef _FGLCAM_H_
#define _FGLCAM_H_

#include "common.h"
#include "fglSceneGraph.h"

class fglCam : public fglNode
{
	friend class fglBindFactory;

protected:
	fglCam(void) = default;

private:
	float fov{ 45.0f*fglANG2RAD };
	float halfFovTan{ tanf((45.0f*fglANG2RAD) / 2.0f) };
	float znear{ 1.0f };
	float zfar{ 1000.0f };

public:

	fglTRANSFORM trgInv;
	fglMAT4 trgInvMatrix;

	//bad for perf virtual ~fglCam(void);
	virtual fglString fgl_class(void) const { return "cam"; }
	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);
	
	float getFov(void);
	float getHalfFovTan(void);
	void setFov(float pnFov);

	const float& getZNear(void) const;
	void setZNear(const float& pnZNear);

	const float& getZFar(void) const;
	void setZFar(const float& pnZFar);

	virtual void buildTransform(void);

	virtual void call(fglEvent& poEvent);

};


///////////////////////////////////////////////////////////////////////////////////////////////
class fglFRUSTUM
{
private:
	fglROW4 aPlanes[6];
	fglVECTOR oSphereCenter;
	float nSphereRadius{ 0.0f };

public:
	enum {TOP = 0, BOTTOM, LEFT, RIGHT, NEARP, FARP};
	enum {OUTSIDE = 0, INTERSECT, INSIDE};

	fglFRUSTUM(void) = default;
	//bad for perf virtual ~fglFRUSTUM(void);

	void build(const fglMAT4& pmProj);
	unsigned testPoint(const fglVECTOR& poPoint) const;
	unsigned testSphere(const fglVECTOR& poCenter, float pnRadius) const;
	unsigned testBoundBox(const fglBBox& poBBox) const;

	fglString toString(void) 
	{ 
		std::ostringstream lcRes; 
		lcRes << "TOP: " <<  aPlanes[0].toString() << std::endl; 
		lcRes << "BOTTOM: " <<  aPlanes[1].toString() << std::endl; 
		lcRes << "LEFT: " <<  aPlanes[2].toString() << std::endl; 
		lcRes << "RIGHT: " <<  aPlanes[3].toString() << std::endl; 
		lcRes << "NEARP: " <<  aPlanes[4].toString() << std::endl; 
		lcRes << "FARP: " <<  aPlanes[5].toString() << std::endl; 
		return lcRes.str(); 
	}

};

#endif