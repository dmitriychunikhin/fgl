#include "fglLight.h"
#include "fglScene.h"

void fglLight::call(fglEvent& poEvent)
{
	if (poEvent.name == fglEVENT_ambient) {if (poEvent.nParams) { ambient.set( (long) _GetValue2i(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(ambient.get()); return;}
	if (poEvent.name == fglEVENT_diffuse) {if (poEvent.nParams) { diffuse.set( (long) _GetValue2i(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(diffuse.get()); return;}
	if (poEvent.name == fglEVENT_specular) {if (poEvent.nParams) { specular.set( (long) _GetValue2i(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(specular.get()); return;}
	if (poEvent.name == fglEVENT_isSpot) {if (poEvent.nParams) { isSpot = _GetValue2l(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2l(isSpot); return;}
	if (poEvent.name == fglEVENT_spotCutoff) {if (poEvent.nParams) { spotCutoff = _GetValue2f(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2f(spotCutoff); return;}
	if (poEvent.name == fglEVENT_spotExp) {if (poEvent.nParams) { spotExp = _GetValue2f(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2f(spotExp); return;}
	if (poEvent.name == fglEVENT_atten) {if (poEvent.nParams) { setSceneRenderFlag(); atten = _GetValue2f(poEvent.aParams[0]); setSceneRenderFlag(); }_RetValue2f(atten); return;}
	if (poEvent.name == fglEVENT_radius) {if (poEvent.nParams) { radius = _GetValue2f(poEvent.aParams[0]); squareRadius=radius*radius; setSceneRenderFlag(); } _RetValue2f(radius); return;}

	fglNode::call(poEvent);
}


void fglLight::setLight(unsigned piLight, const fglTRANSFORM& poTrgInv)
{
	fglTRANSFORM loTrg = poTrgInv * trg;
	fglROW4 loPos(loTrg.pos.x, loTrg.pos.y, loTrg.pos.z, isSpot ? 1.0f : 0.0f);

	piLight += GL_LIGHT0;
	glLightfv(piLight, GL_AMBIENT, (float*)&ambient);		// Setup The Ambient Light
	glLightfv(piLight, GL_DIFFUSE, (float*)&diffuse);		// Setup The Diffuse Light
	glLightfv(piLight, GL_SPECULAR, (float*)&specular);	// Setup The Specular Light
	glLightfv(piLight, GL_POSITION, (float*)&loPos);	// Position The Light
	if (isSpot) 
	{
		fglVECTOR loSpotDir = loTrg.rot * fglVECTOR(0.0f,0.0f,1.0f);
		glLightfv(piLight, GL_SPOT_DIRECTION, (float*)&loSpotDir);
		glLightf(piLight, GL_SPOT_EXPONENT, spotExp);
		glLightf(piLight, GL_SPOT_CUTOFF, spotCutoff);
	}

	float kQ = 0.0f;
	float kL = 0.0f;
	float kC = 1.0f;
	if (atten && radius) 
	{
		kQ = atten / (squareRadius);
		kL = atten / radius;
		kC = atten; 
	}
	glLightf(piLight, GL_CONSTANT_ATTENUATION, kC);
	glLightf(piLight, GL_LINEAR_ATTENUATION, kL);
	glLightf(piLight, GL_QUADRATIC_ATTENUATION, kQ);

	glEnable(piLight);
}


bool fglLight::testInFrustum(const fglFRUSTUM& poFrustum) const
{
	if ( poFrustum.testSphere(trg.pos, radius)==fglFRUSTUM::OUTSIDE ) return false;
	return true;
}

