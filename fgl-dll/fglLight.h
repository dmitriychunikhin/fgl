#ifndef _FGLLIGHT_H_
#define _FGLLIGHT_H_

#include "fglSceneGraph.h"
#include "fglCam.h"

class fglLight : public fglNode
{
	friend class fglBindFactory;

protected:
	fglLight(void) = default;

public:
	fglCOLOR ambient;
	fglCOLOR diffuse;
	fglCOLOR specular;

	bool isSpot{ false };
	float spotCutoff{ 0.0f };
	float spotExp{ 0.0f };

	float atten{ 0.0f };
	float radius{ 0.0f };
	float squareRadius{ 0.0f };


	//bad for perf virtual ~fglLight(void);

	virtual fglString fgl_class(void) const { return "light"; }
	virtual void call(fglEvent& poEvent);
	void setLight(unsigned piLight, const fglTRANSFORM& poTrgInv);
	
	bool testInFrustum(const fglFRUSTUM& poFrustum) const;
};

#endif
