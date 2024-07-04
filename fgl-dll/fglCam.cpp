#include "fglCam.h"
#include "fglScene.h"

void fglCam::call(fglEvent& poEvent)
{
	fglNode::call(poEvent);
	if (poEvent.name == fglEVENT_fov) { if (poEvent.nParams) setFov(_GetValue2f(poEvent.aParams[0])); _RetValue2f(getFov()); return; }
	if (poEvent.name == fglEVENT_znear) { if (poEvent.nParams) setZNear( _GetValue2f(poEvent.aParams[0]) ); _RetValue2f(znear); return; }
	if (poEvent.name == fglEVENT_zfar) { if (poEvent.nParams) setZFar( _GetValue2f(poEvent.aParams[0]) ); _RetValue2f(zfar); return; }
}



bool fglCam::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglCam* loCam = dynamic_cast<const fglCam*>(poSrc);
	if (!loCam) return false;

	if ( !fglNode::copy(poSrc, pnShare) ) return false;

	fov = loCam->fov;
	halfFovTan = loCam->halfFovTan;
	znear = loCam->znear;
	zfar = loCam->zfar;
	
	return true;
}


void fglCam::buildTransform(void)
{
	fglNode::buildTransform();
	trgInv = trg.invert();
	trgInv.scale = trgInv.scale.invert();
	trgInv.toMatrix(trgInvMatrix);
}

float fglCam::getFov(void)
{
	return fov;
}

float fglCam::getHalfFovTan(void)
{
	return halfFovTan;
}

void fglCam::setFov(float pnFov)
{
	if ( pnFov<=0.0f || pnFov > (fglPI/2.0f-fglEPSILON) ) { throw fglException("Wrong field of view"); return; } 
	fov = pnFov;
	halfFovTan = tanf(fov/2.0f);
	setSceneRenderFlag();
}

const float& fglCam::getZNear(void) const
{
	return znear;
}

void fglCam::setZNear(const float& pnZNear)
{
	if ( pnZNear<0.1f ) { throw fglException("Znear cannot be less 0.1"); return; } 
	znear = pnZNear;
	setSceneRenderFlag();
}

const float& fglCam::getZFar(void) const
{
	return zfar;
}

void fglCam::setZFar(const float& pnZFar)
{
	if ( pnZFar<1.0f ) { throw fglException("Zfar cannot be less 1"); return; } 
	zfar = pnZFar;
	setSceneRenderFlag();
}



///////////////////////////////////////////////////////////////////////////////////////////////


void fglFRUSTUM::build(const fglMAT4& pmProj)
{
	fglROW4* loPlane;
	
	// Extract the RIGHT clipping plane
	loPlane = &aPlanes[RIGHT];
	loPlane->a1 = pmProj.d1 - pmProj.a1;
	loPlane->a2 = pmProj.d2 - pmProj.a2;
	loPlane->a3 = pmProj.d3 - pmProj.a3;
	loPlane->a4 = pmProj.d4 - pmProj.a4;
	loPlane->normalize();

	// Extract the LEFT clipping plane
	loPlane = &aPlanes[LEFT];
	loPlane->a1 = pmProj.d1 + pmProj.a1;
	loPlane->a2 = pmProj.d2 + pmProj.a2;
	loPlane->a3 = pmProj.d3 + pmProj.a3;
	loPlane->a4 = pmProj.d4 + pmProj.a4;
	loPlane->normalize();

	// Extract the BOTTOM clipping plane
	loPlane = &aPlanes[BOTTOM];
	loPlane->a1 = pmProj.d1 + pmProj.b1;
	loPlane->a2 = pmProj.d2 + pmProj.b2;
	loPlane->a3 = pmProj.d3 + pmProj.b3;
	loPlane->a4 = pmProj.d4 + pmProj.b4;
	loPlane->normalize();

	// Extract the TOP clipping plane
	loPlane = &aPlanes[TOP];
	loPlane->a1 = pmProj.d1 - pmProj.b1;
	loPlane->a2 = pmProj.d2 - pmProj.b2;
	loPlane->a3 = pmProj.d3 - pmProj.b3;
	loPlane->a4 = pmProj.d4 - pmProj.b4;
	loPlane->normalize();

	// Extract the FAR clipping plane
	loPlane = &aPlanes[FARP];
	loPlane->a1 = pmProj.d1 - pmProj.c1;
	loPlane->a2 = pmProj.d2 - pmProj.c2;
	loPlane->a3 = pmProj.d3 - pmProj.c3;
	loPlane->a4 = pmProj.d4 - pmProj.c4;
	loPlane->normalize();

	// Extract the NEAR clipping plane. 
	loPlane = &aPlanes[NEARP];
	loPlane->a1 = pmProj.d1 + pmProj.c1;
	loPlane->a2 = pmProj.d2 + pmProj.c2;
	loPlane->a3 = pmProj.d3 + pmProj.c3;
	loPlane->a4 = pmProj.d4 + pmProj.c4;
	loPlane->normalize();
}

unsigned fglFRUSTUM::testPoint(const fglVECTOR& poPoint) const
{
	if( aPlanes[RIGHT]*poPoint < 0 ) return OUTSIDE;
	if( aPlanes[LEFT]*poPoint < 0 ) return OUTSIDE;
	if( aPlanes[TOP]*poPoint < 0 ) return OUTSIDE;
	if( aPlanes[BOTTOM]*poPoint < 0 ) return OUTSIDE;
	if( aPlanes[FARP]*poPoint < 0 ) return OUTSIDE;
	if( aPlanes[NEARP]*poPoint < 0 ) return OUTSIDE;
	return INSIDE;
}


unsigned fglFRUSTUM::testSphere(const fglVECTOR& poCenter, float pnRadius) const
{
	unsigned lnRes = INSIDE;
	float lnDist;
	if ( (lnDist = aPlanes[RIGHT]*poCenter) < -pnRadius ) return OUTSIDE;
	if ( lnDist < pnRadius ) lnRes = INTERSECT;
	if ( (lnDist = aPlanes[LEFT]*poCenter) < -pnRadius ) return OUTSIDE;
	if ( lnDist < pnRadius ) lnRes = INTERSECT;
	if ( (lnDist = aPlanes[TOP]*poCenter) < -pnRadius ) return OUTSIDE;
	if ( lnDist < pnRadius ) lnRes = INTERSECT;
	if ( (lnDist = aPlanes[BOTTOM]*poCenter) < -pnRadius ) return OUTSIDE;
	if ( lnDist < pnRadius ) lnRes = INTERSECT;
	if ( (lnDist = aPlanes[FARP]*poCenter) < -pnRadius ) return OUTSIDE;
	if ( lnDist < pnRadius ) lnRes = INTERSECT;
	if ( (lnDist = aPlanes[NEARP]*poCenter) < -pnRadius ) return OUTSIDE;
	if ( lnDist < pnRadius ) lnRes = INTERSECT;
	return lnRes;
}

unsigned fglFRUSTUM::testBoundBox(const fglBBox& poBBox) const
{
	if (poBBox.isNull()) return false;

	unsigned lnRes = testSphere(poBBox.getCenterGlobal(), poBBox.getRadiusGlobal());
	if ( lnRes == INSIDE || lnRes == OUTSIDE ) return lnRes;

	fglVECTOR laVerts[8];
	poBBox.getVertsGlobal(laVerts);

	unsigned lnSumInside(0);
	unsigned p,v;
	for(p = 0; p < 6; p++) 
	{
		unsigned lnInsideResult = 8;
		unsigned lnIsPointInside = 1;

		for(v = 0; v < 8; v++) 
		{
			if ( aPlanes[p] * laVerts[v] < 0 )
			{
				lnIsPointInside = 0;
				lnInsideResult--;
			}
		}
		if ( lnInsideResult == 0 ) return OUTSIDE;

		lnSumInside += lnIsPointInside;
	}

	if ( lnSumInside == 6 ) return INSIDE;
		
	return INTERSECT;
}
