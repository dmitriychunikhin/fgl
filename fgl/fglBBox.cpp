#include "fgl3DGen.h"
#include "fglScene.h"

fglBBox::fglBBox(const fglTRANSFORM& poTR, const fglTRANSFORM& poTRG): tr(poTR), trg(poTRG)
{ 
	isVisible=false; 
	meshType = 1; 
	meshColor.set(1.0f, 1.0f, 1.0f, 1.0f); 
	meshFill = false;
	meshLineWidth=1.0f; 
	meshLighting=0;
}

const fglBBox& fglBBox::operator=(const fglBBox& poSrc)
{
	min = poSrc.min;
	max = poSrc.max;
	isVisible = poSrc.isVisible;
	meshType = poSrc.meshType;
	meshColor = poSrc.meshColor; 
	meshFill = poSrc.meshFill; 
	meshLineWidth = poSrc.meshLineWidth;
	meshLighting = poSrc.meshLighting;
	
	return *this;
}


bool fglBBox::isNull(void) const
{
	return min.isNull() && max.isNull();
}

void fglBBox::setNull(void) 
{ 
	min.x=min.y=min.z=max.x=max.y=max.z=0.0f; 
}

void fglBBox::correct(const fglVECTOR& poBound)
{
	min = fglVECTOR::getMin(min, poBound);
	max = fglVECTOR::getMax(max, poBound);
}

fglVECTOR fglBBox::getSize(void) const
{
	return (max - min);
}

fglVECTOR fglBBox::getSizeLocal(void) const
{
	return (max - min) * tr.scale;
}

fglVECTOR fglBBox::getSizeGlobal(void) const
{
	return (max - min) * trg.scale;
}

float fglBBox::getRadius(void) const
{
	return (getSize() * 0.5f).len();
}

float fglBBox::getRadiusLocal(void) const
{
	return (getSizeLocal() * 0.5f).len();
}

float fglBBox::getRadiusGlobal(void) const
{
	return (getSizeGlobal() * 0.5f).len();
}

float fglBBox::getSquareRadiusLocal(void) const
{
	return (getSizeLocal() * 0.5f).norm();
}

float fglBBox::getSquareRadiusGlobal(void) const
{
	return (getSizeGlobal() * 0.5f).norm();
}

fglVECTOR fglBBox::getCenter(void) const
{
	return (min + max) * 0.5f;
}

fglVECTOR fglBBox::getCenterLocal(void) const
{
	return tr * ((min + max) * 0.5f);
}

fglVECTOR fglBBox::getCenterGlobal(void) const
{
	return trg * ((min + max) * 0.5f);
}

fglVECTOR fglBBox::getMinLocal(void) const
{
	return fglVECTOR::getMin((tr * min), (tr * max));
}

fglVECTOR fglBBox::getMinGlobal(void) const
{
	return fglVECTOR::getMin((trg * min), (trg * max));
}

fglVECTOR fglBBox::getMaxLocal(void) const
{
	return fglVECTOR::getMax((tr * max), (tr * min));
}

fglVECTOR fglBBox::getMaxGlobal(void) const
{
	return fglVECTOR::getMax((trg * max), (trg * min));
}

fglVECTOR fglBBox::getDist(const fglBBox& poBBox2) const
{
	return poBBox2.getCenterGlobal() - getCenterGlobal();
}

void fglBBox::getVertsLocal(fglVECTOR* paVerts) const
{
	if ( ! paVerts) return;
	
	paVerts[0] = min;
	paVerts[1] = min; paVerts[1].x = max.x;
	paVerts[2] = max; paVerts[2].y = min.y; 
	paVerts[3] = min; paVerts[3].z = max.z;
	paVerts[4] = min; paVerts[4].y = max.y; 
	paVerts[5] = max; paVerts[5].z = min.z; 
	paVerts[6] = max; 
	paVerts[7] = max; paVerts[7].x = min.x; 

	paVerts[0] = tr * paVerts[0];
	paVerts[1] = tr * paVerts[1];
	paVerts[2] = tr * paVerts[2];
	paVerts[3] = tr * paVerts[3];
	paVerts[4] = tr * paVerts[4];
	paVerts[5] = tr * paVerts[5];
	paVerts[6] = tr * paVerts[6];
	paVerts[7] = tr * paVerts[7];
	
}

void fglBBox::getVertsGlobal(fglVECTOR* paVerts) const
{
	if ( ! paVerts) return;
	
	paVerts[0] = min;
	paVerts[1] = min; paVerts[1].x = max.x;
	paVerts[2] = max; paVerts[2].y = min.y; 
	paVerts[3] = min; paVerts[3].z = max.z;
	paVerts[4] = min; paVerts[4].y = max.y; 
	paVerts[5] = max; paVerts[5].z = min.z; 
	paVerts[6] = max; 
	paVerts[7] = max; paVerts[7].x = min.x; 

	paVerts[0] = trg * paVerts[0];
	paVerts[1] = trg * paVerts[1];
	paVerts[2] = trg * paVerts[2];
	paVerts[3] = trg * paVerts[3];
	paVerts[4] = trg * paVerts[4];
	paVerts[5] = trg * paVerts[5];
	paVerts[6] = trg * paVerts[6];
	paVerts[7] = trg * paVerts[7];
	
}


fglROW4 fglBBox::getPlane(unsigned pnPlane) const
{
	fglROW4 laPlane;
		
	if (pnPlane==TOP)
	{
		laPlane.a1 = 0.0f; laPlane.a2 = -1.0f; laPlane.a3 = 0.0f; laPlane.a4 = max.y+fglEPSILON;
	}
	else if (pnPlane==BOTTOM)
	{
		laPlane.a1 = 0.0f; laPlane.a2 = 1.0f; laPlane.a3 = 0.0f; laPlane.a4 = -(min.y-fglEPSILON); 
	}
	else if (pnPlane==LEFT)
	{
		laPlane.a1 = 1.0f; laPlane.a2 = 0.0f; laPlane.a3 = 0.0f; laPlane.a4 = -(min.x-fglEPSILON);
	}
	else if (pnPlane==RIGHT)
	{
		laPlane.a1 = -1.0f; laPlane.a2 = 0.0f; laPlane.a3 = 0.0f; laPlane.a4 = max.x+fglEPSILON;
	}
	else if (pnPlane==NEARP)
	{
		laPlane.a1 = 0.0f; laPlane.a2 = 0.0f; laPlane.a3 = -1.0f; laPlane.a4 = max.z+fglEPSILON;
	}
	else if (pnPlane==FARP)
	{
		laPlane.a1 = 0.0f; laPlane.a2 = 0.0f; laPlane.a3 = 1.0f; laPlane.a4 = -(min.z-fglEPSILON);
	}
	
	return laPlane;
}


bool fglBBox::testIntersection(const fglBBox& poBBox2, float* pOutDist) const
{
	const fglVECTOR& loBMin1 = trg.scale * min * 0.5f;
	const fglVECTOR& loBMax1 = trg.scale * max * 0.5f;
	fglVECTOR a = (loBMax1 - loBMin1);
	
	const fglVECTOR& loBMin2 = poBBox2.trg.scale * poBBox2.min * 0.5f;
	const fglVECTOR& loBMax2 = poBBox2.trg.scale * poBBox2.max * 0.5f;
	fglVECTOR b = (loBMax2 - loBMin2);
	
	fglVECTOR v = (poBBox2.trg.pos + loBMin2+loBMax2) - (trg.pos + loBMin1+loBMax1);

	if (pOutDist) *pOutDist = v.len();

	if ( v.norm() > a.norm()+b.norm() ) return false;

	fglVECTOR T = tr.rot * v;
	
	fglMAT4 C( (tr.rot * poBBox2.tr.rot.conj()).toMatrix() );
	const fglMAT4* R = &C;//  C.row();

	float ra, rb, t;
	unsigned i,k;
	
	//система координат А
	for (i = 0; i < 3; i++)
	{
		ra = a[i]; 
		rb = b[0]*fabs(R[i][0]) + b[1]*fabs(R[i][1]) + b[2]*fabs(R[i][2]);
		t = fabs( T[i] );
		if ( t > ra + rb ) return false;
	}

	//система координат B
	for (k = 0; k < 3; k++)
	{
		ra = a[0]*fabs(R[0][k]) + a[1]*fabs(R[1][k]) + a[2]*fabs(R[2][k]);
		rb = b[k];
		t = fabs( T[0]*R[0][k] + T[1]*R[1][k] + T[2]*R[2][k] );
		if ( t > ra + rb ) return false;
	}

	//9 векторных произведений
	//L = A0 x B0
	ra = a[1]*fabs(R[2][0]) + a[2]*fabs(R[1][0]);
	rb = b[1]*fabs(R[0][2]) + b[2]*fabs(R[0][1]);
	t = fabs( T[2]*R[1][0] - T[1]*R[2][0] );
	if ( t > ra + rb ) return false;

	//L = A0 x B1
	ra = a[1]*fabs(R[2][1]) + a[2]*fabs(R[1][1]);
	rb = b[0]*fabs(R[0][2]) + b[2]*fabs(R[0][0]);
	t = fabs( T[2]*R[1][1] - T[1]*R[2][1] );
	if ( t > ra + rb ) return false;

	//L = A0 x B2
	ra = a[1]*fabs(R[2][2]) + a[2]*fabs(R[1][2]);
	rb = b[0]*fabs(R[0][1]) + b[1]*fabs(R[0][0]);
	t = fabs( T[2]*R[1][2] - T[1]*R[2][2] );
	if ( t > ra + rb ) return false;

	//L = A1 x B0
	ra = a[0]*fabs(R[2][0]) + a[2]*fabs(R[0][0]);
	rb = b[1]*fabs(R[1][2]) + b[2]*fabs(R[1][1]);
	t = fabs( T[0]*R[2][0] - T[2]*R[0][0] );
	if ( t > ra + rb ) return false;

	//L = A1 x B1
	ra = a[0]*fabs(R[2][1]) + a[2]*fabs(R[0][1]);
	rb = b[0]*fabs(R[1][2]) + b[2]*fabs(R[1][0]);
	t = fabs( T[0]*R[2][1] - T[2]*R[0][1] );
	if ( t > ra + rb ) return false;

	//L = A1 x B2
	ra = a[0]*fabs(R[2][2]) + a[2]*fabs(R[0][2]);
	rb = b[0]*fabs(R[1][1]) + b[1]*fabs(R[1][0]);
	t = fabs( T[0]*R[2][2] - T[2]*R[0][2] );
	if ( t > ra + rb ) return false;

	//L = A2 x B0
	ra = a[0]*fabs(R[1][0]) + a[1]*fabs(R[0][0]);
	rb = b[1]*fabs(R[2][2]) + b[2]*fabs(R[2][1]);
	t = fabs( T[1]*R[0][0] - T[0]*R[1][0] );
	if ( t > ra + rb ) return false;

	//L = A2 x B1
	ra = a[0]*fabs(R[1][1]) + a[1]*fabs(R[0][1]);
	rb = b[0] *fabs(R[2][2]) + b[2]*fabs(R[2][0]);
	t = fabs( T[1]*R[0][1] - T[0]*R[1][1] );
	if ( t > ra + rb ) return false;

	//L = A2 x B2
	ra = a[0]*fabs(R[1][2]) + a[1]*fabs(R[0][2]);
	rb = b[0]*fabs(R[2][1]) + b[1]*fabs(R[2][0]);
	t = fabs( T[1]*R[0][2] - T[0]*R[1][2] );
	if ( t > ra + rb ) return false;

	return true;
}

void fglBBox::render(void) const
{
	unsigned lnWireframe = 	meshFill ? 0:1;
	unsigned lnSolid = meshFill ? 1:0;
	fglScene::oCurrentScene->o3DPrimitives.oMaterial->reset();
	fglScene::oCurrentScene->o3DPrimitives.oMaterial->lighting = meshLighting ? 1 : 0;
	fglScene::oCurrentScene->o3DPrimitives.oMaterial->wireframe = lnWireframe;
	fglScene::oCurrentScene->o3DPrimitives.oMaterial->color_diffuse = meshColor;
	fglScene::oCurrentScene->o3DPrimitives.oMaterial->lineWidth = meshLineWidth;

	if ( meshType==1 ) fglScene::oCurrentScene->o3DPrimitives.renderCube(getCenter(), getSize(), lnSolid, fglScene::oCurrentScene->o3DPrimitives.oMaterial.callBindREFMap());
	else if ( meshType==2 ) fglScene::oCurrentScene->o3DPrimitives.renderSphere(getCenter(), getSize(), 2, fglScene::oCurrentScene->o3DPrimitives.oMaterial.callBindREFMap());
}
