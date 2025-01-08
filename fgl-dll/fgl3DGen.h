#ifndef _FGL3DGEN_H_
#define _FGL3DGEN_H_

#include "fgl.h"

class fglRECT {
public:
	long left{ 0 };
	long top{ 0 };
	long width{ 0 };
	long height{ 0 };
	fglRECT(void) = default;
	fglRECT(const long& pnLeft, const long& pnTop, const long& pnWidth, const long& pnHeight) {left=pnLeft; top=pnTop; width=pnWidth; height=pnHeight;}

	fglString toString(void) const { std::ostringstream lcRes; lcRes << left << ", " << top << ", " << width << ", " << height; return lcRes.str(); }
};


class fglVECTOR {
public:
	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };
	
	fglVECTOR(void) = default;
	fglVECTOR(const float& px, const float& py, const float& pz) {x=px; y=py; z=pz;}
	
	inline fglVECTOR& build(const float& px, const float& py, const float& pz) { x=px; y=py; z=pz; return *this; }

	inline float& operator[] (const unsigned& i) { return *(&x + i); }
	inline const float& operator[] (const unsigned& i) const { return *(&x + i); }
	inline fglVECTOR operator+(const fglVECTOR& other) const {return fglVECTOR(x+other.x, y+other.y, z+other.z);}
	inline fglVECTOR operator-(void) const  {return fglVECTOR(-x, -y, -z);}
	inline fglVECTOR operator-(const fglVECTOR& other) const  {return fglVECTOR(x-other.x, y-other.y, z-other.z);}
	inline fglVECTOR operator*(const fglVECTOR& other) const  {return fglVECTOR(x*other.x, y*other.y, z*other.z);}
	inline fglVECTOR operator/(const fglVECTOR& other) const  { return fglVECTOR( x/(other.x ? other.x : 1.0f), y/(other.y ? other.y : 1.0f), z/(other.z ? other.z : 1.0f) );}
	inline fglVECTOR operator*(const float& other) const  {return fglVECTOR(x*other, y*other, z*other);}
	inline fglVECTOR operator/(const float& other) const  {if (other==0.0f) return *this; return fglVECTOR(x/other, y/other, z/other);}
	inline bool operator == (const fglVECTOR& other) const  { return (x==other.x) && (y==other.y) && (z==other.z); }
	inline bool operator != (const fglVECTOR& other) const  { return (x!=other.x) || (y!=other.y) || (z!=other.z); }
	inline float operator ^ (const fglVECTOR& other) const  {return x*other.x + y*other.y + z*other.z;}
	inline fglVECTOR operator & (const fglVECTOR& other) const  {return fglVECTOR(y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x);}
	inline float norm(void) const  {return x*x + y*y + z*z;}
	inline float len(void) const  {return sqrt(x*x + y*y + z*z);}
	inline fglVECTOR& normalize(void) {float lnLen=len(); if (lnLen!=0.0f) *this = (*this)/lnLen; return *this; }
	inline fglVECTOR& clean(void) { x = fabs(x) < fglEPSILON ? 0.0f : x;  y = fabs(y) < fglEPSILON ? 0.0f : y;  z = fabs(z) < fglEPSILON ? 0.0f : z; return *this; }
	inline bool isNull(void) const  { return (!x && !y && !z); }
	inline bool isUnit(void) const  { return (x==1.0f && y==1.0f && z==1.0f); }
	inline bool isUniform(void) const  { return (x==y && x==z && y==z); }
	inline fglVECTOR invert(void) const  { return fglVECTOR(1/std::max<float>(x,fglEPSILON), 1/std::max<float>(y,fglEPSILON), 1/std::max<float>(z,fglEPSILON)); }
	inline fglVECTOR proj(const fglVECTOR& other) const { if (other.isNull()) return fglVECTOR(); return other * (((*this) ^ other) / (other ^ other)); }
	static fglVECTOR getMin(const fglVECTOR& v1, const fglVECTOR& v2) { return fglVECTOR( (v1.x < v2.x ? v1.x : v2.x), (v1.y < v2.y ? v1.y : v2.y), (v1.z < v2.z ? v1.z : v2.z) ); }
	static fglVECTOR getMax(const fglVECTOR& v1, const fglVECTOR& v2) { return fglVECTOR( (v1.x > v2.x ? v1.x : v2.x), (v1.y > v2.y ? v1.y : v2.y), (v1.z > v2.z ? v1.z : v2.z) ); }
	

	fglString toString(void) const  { std::ostringstream lcRes; lcRes << x << ", " << y << ", " << z; return lcRes.str(); }
};


class fglROW4
{
public:
	float a1{ 0.0f }, a2{ 0.0f }, a3{ 0.0f }, a4{ 0.0f };
	fglROW4(void) = default;
	fglROW4(const float& pa1, const float& pa2, const float& pa3, const float& pa4) {a1=pa1;a2=pa2;a3=pa3;a4=pa4;}
	fglROW4(const fglVECTOR& poNormal, const float& pnDist) {a1=poNormal.x; a2=poNormal.y; a3=poNormal.z; a4=pnDist;}
	inline float& operator[] (const unsigned& i) { return *(&a1 + i); }
	inline const float& operator[] (const unsigned& i) const { return *(&a1 + i); }
	inline float operator * (const fglVECTOR& other) const { return a1*other.x + a2*other.y + a3*other.z + a4; }
	inline float len(void) const { return sqrt(a1*a1 + a2*a2 + a3*a3); }
	inline void normalize(void) { float lnLen=len(); if (lnLen==0) return; a1=a1/lnLen; a2=a2/lnLen; a3=a3/lnLen; a4=a4/lnLen; }
	inline fglROW4& build(const fglVECTOR& v, const float& p4=1.0f) { a1=v.x; a2=v.y; a3=v.z; a4=p4; return *this; }
	
	fglString toString(void) const { std::ostringstream lcRes; lcRes << a1 << ", " << a2 << ", " << a3 << ", " << a4; return lcRes.str(); }
};

class fglMAT4
{
public:
	float a1{ 1.0f }, b1{ 0.0f }, c1{ 0.0f }, d1{ 0.0f };
	float a2{ 0.0f }, b2{ 1.0f }, c2{ 0.0f }, d2{ 0.0f };
	float a3{ 0.0f }, b3{ 0.0f }, c3{ 1.0f }, d3{ 0.0f };
	float a4{ 0.0f }, b4{ 0.0f }, c4{ 0.0f }, d4{ 1.0f };

	inline float& operator[] (const unsigned& i) { return *(&a1 + i); }
	inline const float& operator[] (const unsigned& i) const { return *(&a1 + i); }
	inline const fglROW4* row(unsigned pnRow=0) const { return (fglROW4*)(&a1 + pnRow*4); }
	
	void toD3DView(void);
	void toD3DProj(void);
	void setCurrentView(void) const;
	void setCurrentProjection(void) const;
	
	fglMAT4 operator + (const fglMAT4& other) const;
	const fglMAT4& operator += (const fglMAT4& other);
	const fglMAT4& operator *= (const fglMAT4& other);
	fglMAT4 operator * (const fglMAT4& other) const;
	fglVECTOR operator * (const fglVECTOR& other) const;
	fglVECTOR rotate (const fglVECTOR& other) const;
	fglROW4 operator * (const fglROW4& other) const;

	void moveTo(const fglVECTOR& poMove);
	void moveBy(const fglVECTOR& poMove);
	void scaleAt(const fglVECTOR& poScale);

	fglMAT4(void) = default;
	fglMAT4(const float& val) {loadValue(val);}
	fglMAT4(const float& _a1, const float& _a2, const float& _a3, const float& _a4,
			const float& _b1, const float& _b2, const float& _b3, const float& _b4,
			const float& _c1, const float& _c2, const float& _c3, const float& _c4,
			const float& _d1, const float& _d2, const float& _d3, const float& _d4) :
			a1(_a1), a2(_a2), a3(_a3), a4(_a4),  
			b1(_b1), b2(_b2), b3(_b3), b4(_b4), 
			c1(_c1), c2(_c2), c3(_c3), c4(_c4),
			d1(_d1), d2(_d2), d3(_d3), d4(_d4) {}

	float det(void)  const ;
	float det3(const unsigned& a11, const unsigned& a12, const unsigned& a13,
				const unsigned& a21, const unsigned& a22, const unsigned& a23,
				const unsigned& a31, const unsigned& a32, const unsigned& a33)  const;

	fglMAT4 transpose(void)  const ;
	bool invert(fglMAT4* pmOut) const;
	inline void loadIdentity(void) {a1=b2=c3=d4=1.0f; a2=a3=a4=b1=b3=b4=c1=c2=c4=d1=d2=d3=0.0f;}
	inline void loadValue(const float& val) {a1=b2=c3=d4=a2=a3=a4=b1=b3=b4=c1=c2=c4=d1=d2=d3=val;}

	fglString toString(void) const  
	{ 
		std::ostringstream lcRes; 
		lcRes << a1 << ", " << b1 << ", " << c1 << ", " << d1 << std::endl; 
		lcRes << a2 << ", " << b2 << ", " << c2 << ", " << d2 << std::endl; 
		lcRes << a3 << ", " << b3 << ", " << c3 << ", " << d3 << std::endl; 
		lcRes << a4 << ", " << b4 << ", " << c4 << ", " << d4 << std::endl; 
		return lcRes.str(); 
	}
	
};



class fglQUAT {
public:

	float a{ 1.0f };
	fglVECTOR v{ 0.0f, 0.0f, 0.0f };

	fglQUAT(void) = default;
	fglQUAT(const float& pa, const float& px, const float& py, const float& pz) {build(pa,px,py,pz);}
	fglQUAT(const float& px, const float& py, const float& pz) {build(px,py,pz);}
	fglQUAT(const fglVECTOR& pv) { a=0.0f; v=pv; }
	
	inline float& operator[] (int i) { return *(&a + i); }
	inline const float& operator[] (int i) const { return *(&a + i); }
	fglQUAT operator-(void) const;
	fglQUAT operator+(const fglQUAT& other) const;
	fglQUAT operator-(const fglQUAT& other) const;
	fglQUAT operator*(const fglQUAT& other) const;
	fglVECTOR operator*(const fglVECTOR& p) const;
	inline fglQUAT operator*(const float& other) const {fglQUAT q; q.a=a*other; q.v=v*other; return q;}
	inline fglQUAT operator/(const float& other) const {if (other==0.0f) return *this; fglQUAT q; q.a=a/other; q.v=v/other; return q;}
	float operator ^ (const fglQUAT& other) const;
	inline float norm(void) const {return a*a + v.x*v.x + v.y*v.y + v.z*v.z;}
	inline float len(void) const {return sqrt(a*a + v.x*v.x + v.y*v.y + v.z*v.z);}
	inline fglQUAT conj(void) const {fglQUAT q; q.a=a; q.v=-v; return q; }
	fglQUAT& normalize(void);
	fglQUAT invert(void) const {float n=norm(); if (n > fglEPSILON) return conj()/n; else return conj()/fglEPSILON; }
	

	fglQUAT& build(const float& pa, const float& px, const float& py, const float& pz);
	fglQUAT& build(const float& px, const float& py, const float& pz); //from euler angles
	fglQUAT& build(const fglVECTOR& poEulerAngles); //from euler angles 
	fglQUAT& build(const fglVECTOR& poFrom, const fglVECTOR& poTo); //rotation from direction poFrom to direction poTo
	void toMatrix(fglMAT4& pmTrans) const;
	fglMAT4 toMatrix(void) const {fglMAT4 mr; toMatrix(mr); return mr;}
	fglVECTOR toEuler(void) const;
	fglQUAT interpolate(const fglQUAT& other, float pnFactor) const;
	inline bool isNull(void) const { return a==0.0f && v.isNull(); }
	inline bool isIdentity(void) const { return a==1.0f && v.isNull(); }
	
	fglString toString(void) const { std::ostringstream lcRes; lcRes << a << ", " << v.x << ", " << v.y << ", " << v.z; return lcRes.str(); }
};


class fglCOLOR {
public:
	float r{ 0.0f };
	float g{ 0.0f };
	float b{ 0.0f };
	float a{ 0.0f };
	
	fglCOLOR(void) = default;
	fglCOLOR(const float& pR, const float& pG, const float& pB, const float& pA=1.0f) { r=pR; g=pG; b=pB; a=pA; }
		
	bool isEmpty(void) const { return r==0.0f && g==0.0f && b==0.0f; }
	unsigned int get(void) const { return unsigned int(a) + (unsigned int(255.0f*r)<<8) + (unsigned int(255.0f*g)<<16) + (unsigned int(255.0f*b)<<24); }
	void set(const float& pr, const float& pg, const float& pb, const float& pa) { r=pr; g=pg; b=pb; a=pa; }
	void set(const unsigned int & prgba) { set( float(prgba & 0x000000FF)/255.0f, float((prgba & 0x0000FF00)>>8)/255.0f, float((prgba & 0x00FF0000)>>16)/255.0f, float( 255 - ((prgba & 0xFF000000)>>24) )/255.0f);}
	
	fglString toString(void) const { std::ostringstream lcRes; lcRes << r << ", " << g << ", " << b << ", " << a; return lcRes.str(); }
};


class fglVERTEX {
public:
	fglVECTOR p; //position
	fglVECTOR n; //normale
	float u{ 0.0f };
	float v{ 0.0f };

	fglVERTEX(void) = default;

};

class fglVERTEXBONES {
public:
	fglROW4 aBoneInds; //bone index 
	fglROW4 aBoneWeights;

	fglVERTEXBONES(void) = default;

};

class fglFACE
{
public:
	fglINDEX* aInds{ NULL };
	int nInds{ 0 };

	fglFACE(void) = default;
	fglFACE(int pnInds, fglINDEX* paInds);
	//bad for perf virtual ~fglFACE(void);

	void updateNormales(fglVERTEX* paVerts, int pnInds=0);
};

class fglTRANSFORMConstraints
{
public:
	fglVECTOR rot{ 1.0f, 1.0f, 1.0f };
	fglVECTOR pos{ 1.0f, 1.0f, 1.0f };
	fglVECTOR scale{ 1.0f, 1.0f, 1.0f };
	fglTRANSFORMConstraints(void) = default;
};


class fglTRANSFORM
{
public:
	fglQUAT rot;
	fglVECTOR pos;
	fglVECTOR scale{ 1.0f, 1.0f, 1.0f };
	
	fglTRANSFORM(void) = default;
	fglTRANSFORM(const bool& isEmpty) { if (isEmpty) loadEmpty(); else loadIdentity(); }
	fglTRANSFORM(const fglQUAT& pRot, const fglVECTOR& pPos, const fglVECTOR& pScale) { rot = pRot; pos = pPos; scale=pScale; }
	//bad for perf virtual ~fglTRANSFORM(void) {}

	inline void loadIdentity(void) { rot.build(0.0f,0.0f,0.0f,0.0f); pos.x=pos.y=pos.z=0.0f; scale.x=scale.y=scale.z=1.0f; }
	inline void loadEmpty(void) { rot.build(0.0f,0.0f,0.0f,0.0f); pos.x=pos.y=pos.z=0.0f; scale.x=scale.y=scale.z=0.0f; }
	inline bool isIdentity(void) { return rot.isIdentity() && pos.isNull() && scale.isUnit(); }
	inline bool isNull(void) { return rot.isNull() && pos.isNull() && scale.isNull(); }

	fglTRANSFORM operator * (const fglTRANSFORMConstraints& poConstraints) const 
	{
		fglTRANSFORM loTrans(*this);
		if ( ! poConstraints.rot.isUnit() )
		{
			loTrans.rot.build(rot.toEuler() * poConstraints.rot);
		}
		if ( ! poConstraints.pos.isUnit() ) loTrans.pos = pos * poConstraints.pos;
		if ( ! poConstraints.scale.isUnit() ) loTrans.scale = scale * poConstraints.scale;
		return loTrans;
	}

	fglTRANSFORM operator * (const fglTRANSFORM& other) const 
	{
		fglTRANSFORM loTrans;
		loTrans.rot = rot * other.rot;
		loTrans.scale = scale * other.scale;
		loTrans.pos = (rot * (other.pos*scale)) + pos;
		return loTrans;
	}

	fglVECTOR operator * (const fglVECTOR& p) const 
	{
		return (rot * (p*scale)) + pos;
	}


	void toMatrix(fglMAT4& pmTrans) const 
	{
		rot.toMatrix(pmTrans);
		pmTrans.a1 *= scale.x; pmTrans.a2 *= scale.y; pmTrans.a3 *= scale.z;
		pmTrans.b1 *= scale.x; pmTrans.b2 *= scale.y; pmTrans.b3 *= scale.z;
		pmTrans.c1 *= scale.x; pmTrans.c2 *= scale.y; pmTrans.c3 *= scale.z;
		pmTrans.a4 = pos.x; pmTrans.b4 = pos.y; pmTrans.c4 = pos.z;
	}

	fglTRANSFORM invert (void) const
	{
		fglTRANSFORM loTrans;
		loTrans.rot = rot.invert();
		loTrans.scale = scale.invert();
		loTrans.pos = -(loTrans.rot * (pos * loTrans.scale));
		return loTrans;
	}

	void rotateBy(const fglQUAT& poRot, const fglVECTOR* poCenter=NULL);
	void rotateTo(const fglQUAT& poRot, const fglVECTOR* poCenter=NULL);
	void moveBy(const fglVECTOR& poMove) { pos = pos + poMove; }
	void moveDir(const fglVECTOR& poMove) { pos = pos + rot * (scale * poMove); }
	void moveTo(const fglVECTOR& poMove) {pos = poMove; }
	void scaleBy(const fglVECTOR& poScale) { scale = scale + poScale; }
	void scaleAt(const fglVECTOR& poScale) { scale = scale * poScale; }
	void scaleTo(const fglVECTOR& poScale) { scale = poScale; }

	fglString toString(void) const 
	{ 
		std::ostringstream lcRes; 
		lcRes << "rot: " <<  rot.toString() << std::endl; 
		lcRes << "pos: " <<  pos.toString() << std::endl; 
		lcRes << "scale: " <<  scale.toString() << std::endl; 
		return lcRes.str(); 
	}
};


class fgl3DRect
{
public:
	
	fglVECTOR min;
	fglVECTOR max;

	fgl3DRect(void) = default;

	bool isNull(void) const { return min.isNull() && max.isNull(); }
	void setNull(void) { min.x=min.y=min.z=max.x=max.y=max.z=0.0f; }
	void correct(const fglVECTOR& poBound)
	{
		min = fglVECTOR::getMin(min, poBound);
		max = fglVECTOR::getMax(max, poBound);
	}
	
	inline fglVECTOR getSize(void) const { return max - min; }
	inline fglVECTOR getCenter(void) const { return (min+max) * 0.5f; }

	fglString toString(void) const
	{ 
		std::ostringstream lcRes; 
		lcRes << "min: " <<  min.toString() << std::endl; 
		lcRes << "max: " <<  max.toString() << std::endl; 
		return lcRes.str(); 
	}

};


class fglBBox 
{
private:
	fglBBox(const fglBBox& poSrc) = default;

public:

	const fglTRANSFORM& tr;
	const fglTRANSFORM& trg;
	
	fglVECTOR min;
	fglVECTOR max;
	bool isVisible{ false };
	unsigned meshType{ 1 }; //0 - no mesh, 1 - cube, 2 - sphere
	fglCOLOR meshColor{ 1.0f, 1.0f, 1.0f, 1.0f};
	bool meshFill{ false };
	float meshLineWidth{ 1.0f };
	unsigned meshLighting{ 0 }; //lighting mode: 0 - none, 1 - phong, 2 -gouraud

	fglBBox(const fglTRANSFORM& poTR, const fglTRANSFORM& poTRG);
	const fglBBox& operator=(const fglBBox& poSrc);
	//bad for perf virtual ~fglBBox(void);

	bool isNull(void) const;
	void setNull(void);
	void correct(const fglVECTOR& poBound);

	fglVECTOR getSize(void) const;
	fglVECTOR getSizeLocal(void) const;
	fglVECTOR getSizeGlobal(void) const;
	float getRadius(void) const;
	float getRadiusLocal(void) const;
	float getRadiusGlobal(void) const;
	float getSquareRadiusLocal(void) const;
	float getSquareRadiusGlobal(void) const;
	fglVECTOR getCenter(void) const;
	fglVECTOR getCenterLocal(void) const;
	fglVECTOR getCenterGlobal(void) const;
	fglVECTOR getMinLocal(void) const;
	fglVECTOR getMinGlobal(void) const;
	fglVECTOR getMaxLocal(void) const;
	fglVECTOR getMaxGlobal(void) const;

	fglVECTOR getDist(const fglBBox& poBBox2) const;
	
	void getVertsLocal(fglVECTOR* paVerts) const;
	void getVertsGlobal(fglVECTOR* paVerts) const;

	enum planes {TOP = 0, BOTTOM, LEFT, RIGHT, NEARP, FARP};
	fglROW4 getPlane(unsigned pnPlane) const;

	bool testIntersection(const fglBBox& poBBox2, float* pOutDist=NULL) const;

	void render(void) const;

	fglString toString(void) const
	{ 
		std::ostringstream lcRes; 
		lcRes << "min: " <<  min.toString() << std::endl; 
		lcRes << "max: " <<  max.toString() << std::endl; 
		return lcRes.str(); 
	}
};



bool fglProject(const fglVECTOR& poObjP, const fglMAT4& pmModelTRG, const fglMAT4& pmProj, const fglRECT& poViewPort, fglVECTOR& poWinP);
bool fglProject(const fglVECTOR& poObjP, const fglTRANSFORM& poModelTRG, const fglMAT4& pmProj, const fglRECT& poViewPort, fglVECTOR& poWinP);
bool fglUnProject(fglVECTOR poWinP, const fglMAT4& pmModelTRG, const fglMAT4& pmProj, const fglRECT& poViewPort, fglVECTOR& poOut);

class fglRAY
{
public:
	fglVECTOR start;
	fglVECTOR end;
	float radius{ 0.0f }; //radius of cylinder

	fglRAY(void) = default;
	//bad for perf virtual ~fglRAY(void) {}

	bool testInPlane(const fglROW4& poPlane, fglVECTOR* poPointIn=NULL) const;
	int testInFace(const fglFACE& poFace, const fglVERTEX* paVerts, fglVECTOR* poPointIn=NULL) const; //return vertex index in face
	bool testInBoundBox(const fglVECTOR& poBMin, const fglVECTOR& poBMax, fglVECTOR* poPointIn) const;
};





class fglCLIP
{
private:
	fglROW4 aPlanes[6];
	bool aPlanesState[6];
	fglCLIP* oParentClip{ NULL };

public:
	enum {TOP = 0, BOTTOM, LEFT, RIGHT, NEARP, FARP};

	fglCLIP(void) = default;
	//bad for perf virtual ~fglCLIP(void);

	void build(const fglBBox& poBBox);
	void bind(void);
	void unbind(void);
	
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
