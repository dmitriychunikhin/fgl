#include "fgl3DGen.h"

bool fglProject(const fglVECTOR& poObjP, const fglMAT4& pmModelTRG, const fglMAT4& pmProj, const fglRECT& poViewPort, fglVECTOR& poWinP)
{
 	fglROW4 loWinP;

	loWinP.a1 = poObjP.x;
	loWinP.a2 = poObjP.y;
	loWinP.a3 = poObjP.z;
	loWinP.a4 = 1.0f;
	loWinP = (pmModelTRG * loWinP);
	loWinP = (pmProj * loWinP);
	if ( loWinP.a4 == 0.0f ) return false;
	loWinP.a1 /= loWinP.a4;
	loWinP.a2 /= loWinP.a4;
	loWinP.a3 /= loWinP.a4;
	
	/* Map x, y and z to range 0-1 */
	loWinP.a1 = loWinP.a1 * 0.5f + 0.5f;
	loWinP.a2 = loWinP.a2 * 0.5f + 0.5f;
	loWinP.a3 = loWinP.a3 * 0.5f + 0.5f;
	
	 /* Map x,y to viewport */
	loWinP.a1 = (loWinP.a1 * poViewPort.width) + poViewPort.left;
	loWinP.a2 = (loWinP.a2 * poViewPort.height) + poViewPort.top;
	
	poWinP.x = loWinP.a1;
	poWinP.y = loWinP.a2;
	poWinP.z = loWinP.a3;

    return true;
}

bool fglProject(const fglVECTOR& poObjP, const fglTRANSFORM& poModelTRG, const fglMAT4& pmProj, const fglRECT& poViewPort, fglVECTOR& poWinP)
{
 	fglROW4 loWinP;
	
	loWinP.build(poModelTRG * poObjP, 1.0f);
	loWinP = (pmProj * loWinP);
	if ( loWinP.a4 == 0.0f ) return false;
	loWinP.a1 /= loWinP.a4;
	loWinP.a2 /= loWinP.a4;
	loWinP.a3 /= loWinP.a4;
	
	/* Map x, y and z to range 0-1 */
	loWinP.a1 = loWinP.a1 * 0.5f + 0.5f;
	loWinP.a2 = loWinP.a2 * 0.5f + 0.5f;
	loWinP.a3 = loWinP.a3 * 0.5f + 0.5f;
	
	 /* Map x,y to viewport */
	loWinP.a1 = (loWinP.a1 * poViewPort.width) + poViewPort.left;
	loWinP.a2 = (loWinP.a2 * poViewPort.height) + poViewPort.top;
	
	poWinP.x = loWinP.a1;
	poWinP.y = loWinP.a2;
	poWinP.z = loWinP.a3;

    return true;
}


bool fglUnProject(fglVECTOR poWinP, const fglMAT4& pmModelTRG, const fglMAT4& pmProj, const fglRECT& poViewPort, fglVECTOR& poOut)
{
    fglMAT4 lmTrans;
	fglROW4 lmUnProj;

	if ( ! (pmProj * pmModelTRG).invert(&lmTrans) ) return false;

	if (poViewPort.width==0.0f || poViewPort.height==0.0f) return false;

	// Map x and y from window coordinates
	poWinP.x = (poWinP.x - poViewPort.left) / poViewPort.width;
	poWinP.y = (poWinP.y - poViewPort.top) / poViewPort.height;

	// Map to range -1 to 1
	poWinP.x = poWinP.x*2 - 1;
	poWinP.y = poWinP.y*2 - 1;
	poWinP.z = poWinP.z*2 - 1;

	lmUnProj = lmTrans * fglROW4(poWinP.x,poWinP.y,poWinP.z,1.0f);
    if (lmUnProj.a4 == 0.0f) return false;
    
	poOut.x = lmUnProj.a1 / lmUnProj.a4;
	poOut.y = lmUnProj.a2 / lmUnProj.a4;
	poOut.z = lmUnProj.a3 / lmUnProj.a4;

    return true;
}

