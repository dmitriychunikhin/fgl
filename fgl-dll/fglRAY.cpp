#include "fgl3DGen.h"

///////////////////////////////////////////////////////////////////////////////////////////////
bool fglRAY::testInPlane(const fglROW4& poPlane, fglVECTOR* poPointIn) const
{
	fglVECTOR loPlaneNormal(poPlane.a1, poPlane.a2, poPlane.a3);
	fglVECTOR loRayD( end-start );
				
	float lnRayAng = loRayD ^ loPlaneNormal;

	if ( ! radius ) if ( fabs(lnRayAng) < fglEPSILON ) return false; //ray is parallel to the plane

	float lnRayDist = start ^ loPlaneNormal;

	float lnPlaneDist = poPlane.a4; //lnPlaneDist = -(loPlanePoint ^ loPlaneNormal);

	if ( ! radius ) 
	{
		float lnFactor = (-lnPlaneDist - lnRayDist)/lnRayAng;

		if (lnFactor < 0.0f || lnFactor > 1.0f) return false;

		if (poPointIn) *poPointIn = start + loRayD*lnFactor;
	}
	else
	{
		//float lnFactor = radius * sqrtf(fabs(1-lnRayAng*lnRayAng)) + lnRayAng*loRayD.len()*0.5f;
		//if ( -lnRayDist-lnPlaneDist > lnFactor ) return false;

		if ( fabs(lnRayAng) > fglEPSILON ) 
		{
			float lnFactor = (-lnPlaneDist - lnRayDist)/lnRayAng;
			if (lnFactor < 0.0f || lnFactor > 1.0f) return false;
			if (poPointIn) *poPointIn = start + loRayD*lnFactor;
		}
		else
		{
			if (poPointIn) poPointIn->build(fglINFINITE,fglINFINITE,fglINFINITE);
		}
	}

	return true;
};

int fglRAY::testInFace(const fglFACE& poFace, const fglVERTEX* paVerts, fglVECTOR* poPointIn) const
{
	if (poFace.nInds==0) return -1;

	fglVECTOR loNorm, loPoint, loPointIn;
	loPoint = paVerts[poFace.aInds[0]].p;
	loNorm = paVerts[poFace.aInds[0]].n;

	if ( ! testInPlane( fglROW4(loNorm, -(loNorm^loPoint) ), &loPointIn) ) return -1;

	short lnProjPlane, lnSign;
	if ( fabs(loNorm.x) > std::max<float>(fabs(loNorm.y), std::fabs(loNorm.z)) )
	{
		lnProjPlane = 0;
		lnSign = loNorm.x<0 ? 1:-1;
	} 
	else if ( fabs(loNorm.y) > fabs(loNorm.z) )
	{
		lnProjPlane = 1;
		lnSign = loNorm.y<0 ? -1:1;
	}
	else
	{
		lnProjPlane = 2;
		lnSign = loNorm.z<0 ? 1:-1;
	}

	float x,y,x1,y1,x2,y2,f, lnDist, lnMinDist=fglINFINITE;
	int i,j, lnVerts=0, lnVertId=-1;
	while (lnVerts < poFace.nInds)
	{
		i = poFace.aInds[lnVerts];
		j = lnVerts==poFace.nInds-1 ? poFace.aInds[0] : poFace.aInds[lnVerts+1];
		lnVerts++;

		if (radius && loPoint.x == fglINFINITE)
		{
			lnVertId=lnVerts-1; loPoint = paVerts[i].p; 
			break;
		}

		x = loPointIn.x; y = loPointIn.y;
		x1=paVerts[i].p.x; x2=paVerts[j].p.x;
		y1=paVerts[i].p.y; y2=paVerts[j].p.y;
		if (lnProjPlane==0) {x = loPointIn.z; x1=paVerts[i].p.z; x2=paVerts[j].p.z;}
		else if (lnProjPlane==1) {y = loPointIn.z; y1=paVerts[i].p.z; y2=paVerts[j].p.z;}

		if ( ! radius )
		{
			f = ( (y-y1)*(x2-x1) - (x-x1)*(y2-y1) ) * lnSign;
			if (f>0) return -1;
			lnDist = (loPointIn-paVerts[i].p).norm();
			if (lnMinDist > lnDist) { lnMinDist = lnDist; lnVertId=lnVerts-1;}
		}
		else
		{
			f = ( (y-y1)*(x2-x1) - (x-x1)*(y2-y1) ) * lnSign;
			if (f>0) 
			{
				if ( (((x-x1)*(x-x1) + (y-y1)*(y-y1)) > radius*radius) && (((x-x2)*(x-x2) + (y-y2)*(y-y2)) > radius*radius) ) return -1;
				else { lnVertId = lnVerts-1; break; }
			}
			lnVertId=lnVerts-1;
		}
	}
	if (poPointIn) *poPointIn = loPointIn;
	return lnVertId;
}

bool fglRAY::testInBoundBox(const fglVECTOR& poBMin, const fglVECTOR& poBMax, fglVECTOR* poPointIn) const
{
	fglINDEX laInds[4];
	fglVERTEX laVerts[4];
	fglVECTOR loPointIn, loNearPointIn;
	float lnDist, lnMinDist=fglINFINITE;
	bool llRes=false;

	laInds[0]=0; laInds[1]=1; laInds[2]=2; laInds[3]=3;
	fglFACE loFace(4, laInds);
	
	////////////////////////////////////////////////////////////////////////////////////
	//XY Plane
	laVerts[0].n.x = 0.0f; laVerts[0].n.y = 0.0f; laVerts[0].n.z = 1.0f;
	laVerts[0].p.x = poBMin.x; laVerts[0].p.y = poBMin.y; laVerts[0].p.z = poBMin.z; 
	laVerts[1].p.x = poBMax.x; laVerts[1].p.y = poBMin.y; laVerts[1].p.z = poBMin.z; 
	laVerts[2].p.x = poBMax.x; laVerts[2].p.y = poBMax.y; laVerts[2].p.z = poBMin.z; 
	laVerts[3].p.x = poBMin.x; laVerts[3].p.y = poBMax.y; laVerts[3].p.z = poBMin.z; 
	if ( fabs(poBMin.z-start.z) > fabs(poBMax.z-start.z) ) { laVerts[0].p.z = laVerts[1].p.z = laVerts[2].p.z = laVerts[3].p.z = poBMax.z; }
	if ( testInFace(loFace, laVerts, &loPointIn)!=-1 ) { llRes = true; lnDist = (loPointIn - start).norm(); if (lnDist<lnMinDist) { loNearPointIn=loPointIn; lnMinDist=lnDist; }}
	////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////
	//XZ Plane
	laVerts[0].n.x = 0.0f; laVerts[0].n.y = 1.0f; laVerts[0].n.z = 0.0f;
	laVerts[0].p.x = poBMin.x; laVerts[0].p.y = poBMin.y; laVerts[0].p.z = poBMax.z; 
	laVerts[1].p.x = poBMax.x; laVerts[1].p.y = poBMin.y; laVerts[1].p.z = poBMax.z; 
	laVerts[2].p.x = poBMax.x; laVerts[2].p.y = poBMin.y; laVerts[2].p.z = poBMin.z; 
	laVerts[3].p.x = poBMin.x; laVerts[3].p.y = poBMin.y; laVerts[3].p.z = poBMin.z; 
	if ( fabs(poBMin.y-start.y) > fabs(poBMax.y-start.y) ) { laVerts[0].p.y = laVerts[1].p.y = laVerts[2].p.y = laVerts[3].p.y = poBMax.y; }
	if ( testInFace(loFace, laVerts, &loPointIn)!=-1 ) { llRes = true; lnDist = (loPointIn - start).norm(); if (lnDist<lnMinDist) { loNearPointIn=loPointIn; lnMinDist=lnDist; }}
	
	////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////
	//YZ Plane
	laVerts[0].n.x = 1.0f; laVerts[0].n.y = 0.0f; laVerts[0].n.z = 0.0f;
	laVerts[0].p.x = poBMin.x; laVerts[0].p.y = poBMin.y; laVerts[0].p.z = poBMin.z; 
	laVerts[1].p.x = poBMin.x; laVerts[1].p.y = poBMin.y; laVerts[1].p.z = poBMax.z; 
	laVerts[2].p.x = poBMin.x; laVerts[2].p.y = poBMax.y; laVerts[2].p.z = poBMax.z; 
	laVerts[3].p.x = poBMin.x; laVerts[3].p.y = poBMax.y; laVerts[3].p.z = poBMin.z; 
	if ( fabs(poBMin.x-start.x) > fabs(poBMax.x-start.x) ) { laVerts[0].p.x = laVerts[1].p.x = laVerts[2].p.x = laVerts[3].p.x = poBMax.x; }
	if ( testInFace(loFace, laVerts, &loPointIn)!=-1 ) { llRes = true; lnDist = (loPointIn - start).norm(); if (lnDist<lnMinDist) { loNearPointIn=loPointIn; lnMinDist=lnDist; }}
	////////////////////////////////////////////////////////////////////////////////////

	if ( llRes && poPointIn ) *poPointIn = loNearPointIn;
	return llRes;
}
