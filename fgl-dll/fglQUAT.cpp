#include "fgl3DGen.h"

fglQUAT fglQUAT::operator-(void) const
{
	return fglQUAT(-a, -v.x, -v.y, -v.z);
}

fglQUAT fglQUAT::operator+(const fglQUAT& other) const
{
	return fglQUAT(a+other.a, v.x+other.v.x, v.y+other.v.y, v.z+other.v.z);
}

fglQUAT fglQUAT::operator-(const fglQUAT& other) const
{
	return fglQUAT(a-other.a, v.x-other.v.x, v.y-other.v.y, v.z-other.v.z);
}

fglQUAT fglQUAT::operator*(const fglQUAT& other) const
{
	fglQUAT q;
	q.a = a*other.a - (v ^ other.v);
	q.v = (other.v * a) + (v * other.a) + (v & other.v);
	return q; 
}

fglVECTOR fglQUAT::operator*(const fglVECTOR& p) const
{
	return (*this * fglQUAT(p) * invert()).v;
}

float fglQUAT::operator ^ (const fglQUAT& other) const  
{
	return (a * other.a) + (v ^ other.v);
}

fglQUAT& fglQUAT::normalize(void)
{
	float lnLen = len();
	if (lnLen == 0.0f) return *this;
	a = a / lnLen;
	v = v / lnLen;
	return *this;
}

fglQUAT& fglQUAT::build(const float& pa, const float& px, const float& py, const float& pz)
{
	float lnSinA = sin(pa/2.0f);
	float lnCosA = cos(pa/2.0f);

	a = lnCosA; 
	v.x = px*lnSinA; 
	v.y = py*lnSinA; 
	v.z = pz*lnSinA;
	
	return *this;
}

fglQUAT& fglQUAT::build(const float& px, const float& py, const float& pz)
{
	fglQUAT loRotX(px,1.0f,0.0f,0.0f);
	fglQUAT loRotY(py,0.0f,1.0f,0.0f);
	fglQUAT loRotZ(pz,0.0f,0.0f,1.0f);
	*this = (loRotY * loRotZ) * loRotX;
	
	return *this;
}

fglQUAT& fglQUAT::build(const fglVECTOR& poEulerAngles)
{
	return build(poEulerAngles.x, poEulerAngles.y, poEulerAngles.z);
}

fglQUAT& fglQUAT::build(const fglVECTOR& poFrom, const fglVECTOR& poTo)
{
	v = poFrom & poTo;
	a = poFrom ^ poTo;
	normalize();
	
	a += 1.0f;

	if( a < fglEPSILON ) // angle close to PI
	{
		if( ( poFrom.z*poFrom.z ) > ( poFrom.x*poFrom.x ) )
			v = fglVECTOR(0, poFrom.z, -poFrom.y); //poFrom & fglVECTOR(1,0,0) 
		else 
			v = fglVECTOR(poFrom.y, -poFrom.x, 0); //poFrom & fglVECTOR(0,0,1) 
	}
	normalize(); 

	return *this;
}

void fglQUAT::toMatrix(fglMAT4& pmRot) const
{
	float xy = v.x * v.y; xy += xy;
	float xz = v.x * v.z; xz += xz;
	float yz = v.y * v.z; yz += yz;
	float xx = v.x * v.x; xx += xx;
	float yy = v.y * v.y; yy += yy;
	float zz = v.z * v.z; zz += zz;
	float xa = v.x * a; xa += xa;
	float ya = v.y * a; ya += ya;
	float za = v.z * a; za += za;

	pmRot.a1 = 1.0f - ( yy + zz );
	pmRot.a2 = ( xy - za );
	pmRot.a3 = ( xz + ya );
	pmRot.a4 = 0.0f;
	
	pmRot.b1 = ( xy + za );
	pmRot.b2 = 1.0f - ( xx + zz );
	pmRot.b3 = (yz - xa );
	pmRot.b4 = 0.0f;

	pmRot.c1 = ( xz - ya );
	pmRot.c2 = ( yz + xa );
	pmRot.c3 = 1.0f - ( xx + yy );
	pmRot.c4 = 0.0f;

	pmRot.d1 = 0;
	pmRot.d2 = 0;
	pmRot.d3 = 0;
	pmRot.d4 = 1.0f;	
}

fglVECTOR fglQUAT::toEuler(void) const
{
	fglVECTOR loRes;
	
	float test = v.x*v.y + v.z*a;
	if (test > 0.499f) { // singularity at north pole
		loRes.y = 2.0f * atan2(v.x,a);
		loRes.z = fglPIhalf;
		loRes.x = 0.0f;
		return loRes;
	}
	if (test < -0.499f) { // singularity at south pole
		loRes.y = -2.0f * atan2(v.x,a);
		loRes.z = -fglPIhalf;
		loRes.x = 0.0f;
		return loRes;
	}
    float sqx = v.x*v.x;
    float sqy = v.y*v.y;
    float sqz = v.z*v.z;
    loRes.y = atan2(2.0f*v.y*a-2.0f*v.x*v.z , 1.0f - 2.0f*sqy - 2.0f*sqz);
	loRes.z = asin(2.0f*test);
	loRes.x = atan2(2.0f*v.x*a-2.0f*v.y*v.z , 1.0f - 2.0f*sqx - 2.0f*sqz);
	return loRes;
}

fglQUAT fglQUAT::interpolate(const fglQUAT& other, float pnFactor) const
{
	float lnCosA = *this ^ other;

	// adjust signs (if necessary)
	fglQUAT lqDest = lnCosA < 0.0f ? -other : other;

	// Calculate coefficients
	float lnP, lnQ;
  
	if ( (1.0f - lnCosA) > 0.0001f ) // 0.0001 -> some epsillon
	{
		// Standard case (slerp)
		float lnAngle = acos(lnCosA); 
		float lnSinA = sin(lnAngle);
		lnP  = sin( (1.0f - pnFactor) * lnAngle ) / lnSinA;
		lnQ  = sin(pnFactor * lnAngle) / lnSinA;
	} else {
		// Very close, do linear interp (because it's faster)
		lnP = 1.0f - pnFactor;
		lnQ = pnFactor;
	}

	fglQUAT qr;
	qr.a = lnP * a + lnQ * lqDest.a;
	qr.v = v*lnP + lqDest.v*lnQ;
	return qr;
}
