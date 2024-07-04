#include "fgl3DGen.h"

void fglTRANSFORM::rotateBy(const fglQUAT& poRot, const fglVECTOR* poCenter) 
{ 
	fglVECTOR loCenter, loPosDelta;
	if (poCenter) loCenter = (*poCenter);
	loPosDelta = (*this) * loCenter;
	
	fglVECTOR loAngle = rot.toEuler() + poRot.toEuler();
	rot.build(loAngle.x, loAngle.y, loAngle.z);

	loPosDelta = ( ((*this) * loCenter) - loPosDelta ).clean();
	
	pos = pos - loPosDelta; 
}

void fglTRANSFORM::rotateTo(const fglQUAT& poRot, const fglVECTOR* poCenter)
{
	fglVECTOR loCenter, loPosDelta;
	if (poCenter) loCenter = (*poCenter);
	loPosDelta = (*this) * loCenter;
	
	rot = poRot;

	loPosDelta = ( ((*this) * loCenter) - loPosDelta ).clean();
	
	pos = pos - loPosDelta; 
}
