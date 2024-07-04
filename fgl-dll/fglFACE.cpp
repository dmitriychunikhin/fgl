#include "fgl3DGen.h"

fglFACE::fglFACE(int pnInds, fglINDEX* paInds)
{
	nInds = pnInds;
	aInds = paInds;
}

void fglFACE::updateNormales(fglVERTEX* paVerts, int pnInds)
{
	int lnLeft, lnRight, lnCur;
	fglVECTOR loNorm;
	pnInds = pnInds == 0 ? nInds : pnInds;
	for(int i=0; i<pnInds; i++)
	{
		lnLeft = aInds[i==0 ? pnInds-1 : i-1];
		lnRight = aInds[i==pnInds-1 ? 0 : i+1];
		lnCur = aInds[i];
		loNorm = (paVerts[lnRight].p - paVerts[lnCur].p) & (paVerts[lnLeft].p - paVerts[lnCur].p);
		loNorm.normalize();
		paVerts[lnCur].n = loNorm;
	}
}
