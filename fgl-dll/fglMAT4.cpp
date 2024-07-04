///////////////////////////////////////////////////////////////////////
//MATRIX 4

#include "fgl3DGen.h"
#include "fglScene.h"

fglMAT4 goD3DMatrix;

void fglMAT4::toD3DView(void)
{
	std::swap(a2, b1);
	std::swap(a3, c1); 
	std::swap(a4, d1);
	std::swap(b3, c2); 
	std::swap(b4, d2);
	std::swap(c4, d3); 
	a3 = -a3; 
	c1 = -c1; 
	b3 = -b3; 
	c2 = -c2;  
	c4 = -c4;
}

void fglMAT4::toD3DProj(void)
{
	std::swap(a2, b1);
	std::swap(a3, c1); 
	std::swap(a4, d1);
	std::swap(b3, c2); 
	std::swap(b4, d2);
	std::swap(c4, d3); 
	a3 = -a3; b3 = -b3; c3 = -c3; d3 = -d3;
}

void fglMAT4::setCurrentView(void) const
{
	//if (fglScene::oGLHelper.getGLVersionMajor() > 2) return;
	glLoadMatrixf((GLfloat*)&a1);
}

void fglMAT4::setCurrentProjection(void) const
{
	//if (fglScene::oGLHelper.getGLVersionMajor() > 2) return;
	glMatrixMode( GL_PROJECTION ); 
	glLoadMatrixf((GLfloat*)&a1);
	glMatrixMode( GL_MODELVIEW );
}


const fglMAT4& fglMAT4::operator +=(const fglMAT4& other) 
{
	*this = *this + other;
	return *this;
}


fglMAT4 fglMAT4::operator +(const fglMAT4& other) const 
{
	return fglMAT4(
		a1+other.a1, a2+other.a2, a3+other.a3, a4+other.a4,
		b1+other.b1, b2+other.b2, b3+other.b3, b4+other.b4,
		c1+other.c1, c2+other.c2, c3+other.c3, c4+other.c4,
		d1+other.d1, d2+other.d2, d3+other.d3, d4+other.d4);
}


const fglMAT4& fglMAT4::operator *=(const fglMAT4& other)
{
	*this = *this * other;
	return *this;
}

fglMAT4 fglMAT4::operator *(const fglMAT4& other) const 
{
	return fglMAT4(
		a1*other.a1 + a2*other.b1 + a3*other.c1 + a4*other.d1,
		a1*other.a2 + a2*other.b2 + a3*other.c2 + a4*other.d2,
		a1*other.a3 + a2*other.b3 + a3*other.c3 + a4*other.d3,
		a1*other.a4 + a2*other.b4 + a3*other.c4 + a4*other.d4,
		b1*other.a1 + b2*other.b1 + b3*other.c1 + b4*other.d1,
		b1*other.a2 + b2*other.b2 + b3*other.c2 + b4*other.d2,
		b1*other.a3 + b2*other.b3 + b3*other.c3 + b4*other.d3,
		b1*other.a4 + b2*other.b4 + b3*other.c4 + b4*other.d4,
		c1*other.a1 + c2*other.b1 + c3*other.c1 + c4*other.d1,
		c1*other.a2 + c2*other.b2 + c3*other.c2 + c4*other.d2,
		c1*other.a3 + c2*other.b3 + c3*other.c3 + c4*other.d3,
		c1*other.a4 + c2*other.b4 + c3*other.c4 + c4*other.d4,
		d1*other.a1 + d2*other.b1 + d3*other.c1 + d4*other.d1,
		d1*other.a2 + d2*other.b2 + d3*other.c2 + d4*other.d2,
		d1*other.a3 + d2*other.b3 + d3*other.c3 + d4*other.d3,
		d1*other.a4 + d2*other.b4 + d3*other.c4 + d4*other.d4);
}

fglROW4 fglMAT4::operator*(const fglROW4& other) const 
{
	return fglROW4(
		other.a1*a1 + other.a2*a2 + other.a3*a3 + other.a4*a4,
		other.a1*b1 + other.a2*b2 + other.a3*b3 + other.a4*b4,
		other.a1*c1 + other.a2*c2 + other.a3*c3 + other.a4*c4,
		other.a1*d1 + other.a2*d2 + other.a3*d3 + other.a4*d4);
}

fglVECTOR fglMAT4::operator * (const fglVECTOR& other) const 
{
	return fglVECTOR(
		other.x*a1 + other.y*a2 + other.z*a3 + a4,
		other.x*b1 + other.y*b2 + other.z*b3 + b4,
		other.x*c1 + other.y*c2 + other.z*c3 + c4);
}

fglVECTOR fglMAT4::rotate(const fglVECTOR& other) const 
{
	return fglVECTOR(
		other.x*a1 + other.y*a2 + other.z*a3,
		other.x*b1 + other.y*b2 + other.z*b3,
		other.x*c1 + other.y*c2 + other.z*c3);
}

void fglMAT4::moveTo(const fglVECTOR& poMove)
{
	a4 = poMove.x;
	b4 = poMove.y;
	c4 = poMove.z;
}

void fglMAT4::moveBy(const fglVECTOR& poMove)
{
	a4 += poMove.x;
	b4 += poMove.y;
	c4 += poMove.z;
}

void fglMAT4::scaleAt(const fglVECTOR& poScale)
{
	a1 *= poScale.x; a2 *= poScale.y; a3 *= poScale.z;
	b1 *= poScale.x; b2 *= poScale.y; b3 *= poScale.z;
	c1 *= poScale.x; c2 *= poScale.y; c3 *= poScale.z;
}


float fglMAT4::det(void) const 
{
	return a1*b2*c3*d4 - a1*b2*c4*d3 + a1*b3*c4*d2 - a1*b3*c2*d4 
		 + a1*b4*c2*d3 - a1*b4*c3*d2 - a2*b3*c4*d1 + a2*b3*c1*d4 
		 - a2*b4*c1*d3 + a2*b4*c3*d1 - a2*b1*c3*d4 + a2*b1*c4*d3 
		 + a3*b4*c1*d2 - a3*b4*c2*d1 + a3*b1*c2*d4 - a3*b1*c4*d2 
		 + a3*b2*c4*d1 - a3*b2*c1*d4 - a4*b1*c2*d3 + a4*b1*c3*d2
		 - a4*b2*c3*d1 + a4*b2*c1*d3 - a4*b3*c1*d2 + a4*b3*c2*d1;
}


float fglMAT4::det3(const unsigned& a11, const unsigned& a12, const unsigned& a13,
				const unsigned& a21, const unsigned& a22, const unsigned& a23,
				const unsigned& a31, const unsigned& a32, const unsigned& a33) const 
{
	const fglMAT4& m = *this;
	return m[a11]*m[a22]*m[a33] + m[a13]*m[a21]*m[a32] + m[a12]*m[a23]*m[a31]
		 - m[a13]*m[a22]*m[a31] - m[a11]*m[a23]*m[a32] - m[a12]*m[a21]*m[a33];
}


fglMAT4 fglMAT4::transpose(void) const 
{
	fglMAT4 mr(*this);
	std::swap(mr.b1, mr.a2);std::swap(mr.c1, mr.a3);std::swap(mr.c2, mr.b3);std::swap(mr.d1, mr.a4);std::swap(mr.d2, mr.b4);std::swap(mr.d3, mr.c4);
	return mr;
}

bool fglMAT4::invert(fglMAT4* pmOut) const 
{
	float lnDet = det();
	if (lnDet == 0.0f) return false;
	float invdet = 1.0f / lnDet;

	pmOut->a1 = invdet  * (b2 * (c3 * d4 - c4 * d3) + b3 * (c4 * d2 - c2 * d4) + b4 * (c2 * d3 - c3 * d2));
	pmOut->a2 = -invdet * (a2 * (c3 * d4 - c4 * d3) + a3 * (c4 * d2 - c2 * d4) + a4 * (c2 * d3 - c3 * d2));
	pmOut->a3 = invdet  * (a2 * (b3 * d4 - b4 * d3) + a3 * (b4 * d2 - b2 * d4) + a4 * (b2 * d3 - b3 * d2));
	pmOut->a4 = -invdet * (a2 * (b3 * c4 - b4 * c3) + a3 * (b4 * c2 - b2 * c4) + a4 * (b2 * c3 - b3 * c2));
	pmOut->b1 = -invdet * (b1 * (c3 * d4 - c4 * d3) + b3 * (c4 * d1 - c1 * d4) + b4 * (c1 * d3 - c3 * d1));
	pmOut->b2 = invdet  * (a1 * (c3 * d4 - c4 * d3) + a3 * (c4 * d1 - c1 * d4) + a4 * (c1 * d3 - c3 * d1));
	pmOut->b3 = -invdet * (a1 * (b3 * d4 - b4 * d3) + a3 * (b4 * d1 - b1 * d4) + a4 * (b1 * d3 - b3 * d1));
	pmOut->b4 = invdet  * (a1 * (b3 * c4 - b4 * c3) + a3 * (b4 * c1 - b1 * c4) + a4 * (b1 * c3 - b3 * c1));
	pmOut->c1 = invdet  * (b1 * (c2 * d4 - c4 * d2) + b2 * (c4 * d1 - c1 * d4) + b4 * (c1 * d2 - c2 * d1));
	pmOut->c2 = -invdet * (a1 * (c2 * d4 - c4 * d2) + a2 * (c4 * d1 - c1 * d4) + a4 * (c1 * d2 - c2 * d1));
	pmOut->c3 = invdet  * (a1 * (b2 * d4 - b4 * d2) + a2 * (b4 * d1 - b1 * d4) + a4 * (b1 * d2 - b2 * d1));
	pmOut->c4 = -invdet * (a1 * (b2 * c4 - b4 * c2) + a2 * (b4 * c1 - b1 * c4) + a4 * (b1 * c2 - b2 * c1));
	pmOut->d1 = -invdet * (b1 * (c2 * d3 - c3 * d2) + b2 * (c3 * d1 - c1 * d3) + b3 * (c1 * d2 - c2 * d1));
	pmOut->d2 = invdet  * (a1 * (c2 * d3 - c3 * d2) + a2 * (c3 * d1 - c1 * d3) + a3 * (c1 * d2 - c2 * d1));
	pmOut->d3 = -invdet * (a1 * (b2 * d3 - b3 * d2) + a2 * (b3 * d1 - b1 * d3) + a3 * (b1 * d2 - b2 * d1));
	pmOut->d4 = invdet  * (a1 * (b2 * c3 - b3 * c2) + a2 * (b3 * c1 - b1 * c3) + a3 * (b1 * c2 - b2 * c1)); 
	
	return true;

}
