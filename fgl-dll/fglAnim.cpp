#include "fglAnim.h"

void fglAnimEval::build(float pnTime)
{
	if (oAnim.empty()) return;
	
	float lnFPS = (oAnim->nFPS != 0.0f) ? oAnim->nFPS : 24.0f;
	float lnFrame = (pnTime-nStartTime) * lnFPS; //covert time to frame number

	//calc frame number for given pnTime
	lnFrame = oAnim->nFrames > 0.0f ? fmod( lnFrame, oAnim->nFrames) : 0.0f;

	/*ACHANS::iterator it=aChans.begin(), itEnd=aChans.end();
	for(; it != itEnd; it++)
	{
		it->nFrames = oAnim->nFrames;
		it->build(lnFrame);
	}*/

	nCurrFrame = lnFrame;
}

void fglAnimChanEval::build(float pnFrame)
{
	tr.loadIdentity();

	fglQUAT lNextRot;
	if ( getNextRot(pnFrame, lNextRot) )
	{
		tr.rot = lNextRot;
	}

	fglVECTOR lNextScale;
	if ( getNextScale(pnFrame, lNextScale) )
	{
		tr.scale = lNextScale;
	}

	fglVECTOR lNextPos;
	if ( getNextPos(pnFrame, lNextPos) )
	{
		tr.pos = lNextPos;
	}

}


////////////////////////////////////////////////////////////////////////////////

bool fglAnimChanEval::getNextPos(float pnFrame, fglVECTOR& pNextPos)
{
	if (oChan.empty()) return false;
	if( oChan->aPosKeys.empty() )
	{
		nCurrPos = -1;
		return false;
	}

	int lnKey = nCurrPos!=-1 ? nCurrPos : 0;
	if( pnFrame < oChan->aPosKeys[lnKey].nFrame) lnKey=0;

	int lnKeyCount = oChan->aPosKeys.size();

	while( lnKey < lnKeyCount-1)
	{
		if( pnFrame < oChan->aPosKeys[lnKey+1].nFrame) break;
		lnKey++;
	}
	
	// interpolation
	int lnNextKey = (lnKey + 1) % lnKeyCount;
	fglAnimVectKey& loKey = oChan->aPosKeys[lnKey];
	fglAnimVectKey& loNextKey = oChan->aPosKeys[lnNextKey];
	float lnDFrame = loNextKey.nFrame - loKey.nFrame;
	if( lnDFrame < 0.0f) lnDFrame += nFrames;
	
	if( lnDFrame > 0.0f)
	{
		float lnFactor = float( (pnFrame - loKey.nFrame) / lnDFrame);
		pNextPos = loKey.v + (loNextKey.v - loKey.v) * lnFactor;
	} else {
		pNextPos = loKey.v;
	}

	nCurrPos = lnKey;
	return true;
}

bool fglAnimChanEval::getNextScale(float pnFrame, fglVECTOR& pNextScale)
{
	if( oChan->aScaleKeys.empty() )
	{
		nCurrScale = -1;
		return false;
	}
	
	int lnKey = nCurrScale!=-1 ? nCurrScale : 0;
	if( pnFrame < oChan->aScaleKeys[lnKey].nFrame) lnKey=0;

	int lnKeyCount = oChan->aScaleKeys.size();
	
	while( lnKey < lnKeyCount-1)
	{
		if( pnFrame < oChan->aScaleKeys[lnKey+1].nFrame) break;
		lnKey++;
	}
				
	// interpolation
	int lnNextKey = (lnKey + 1) % lnKeyCount;
	fglAnimVectKey& loKey = oChan->aScaleKeys[lnKey];
	fglAnimVectKey& loNextKey = oChan->aScaleKeys[lnNextKey];
	float lnDFrame = loNextKey.nFrame - loKey.nFrame;
	//if( lnDFrame < 0.0f) lnDFrame += nFrames;
	
	/*if( lnDFrame > 0.0f)
	{
		float lnFactor = float( (pnFrame - loKey->nFrame) / lnDFrame);
		pNextScale = loKey->v + (loNextKey->v - loKey->v) * lnFactor;
	} else {
		pNextScale = loKey->v;
	}*/
	pNextScale = loKey.v;

	nCurrScale = lnKey;
	return true;

}

bool fglAnimChanEval::getNextRot(float pnFrame, fglQUAT& pNextRot)
{
	if( oChan->aRotKeys.empty() )
	{
		nCurrRot = -1;
		return false;
	}

	int lnKey = nCurrRot!=-1 ? nCurrRot : 0;
	if( pnFrame < oChan->aRotKeys[lnKey].nFrame) lnKey=0;

	int lnKeyCount = oChan->aRotKeys.size();
	
	while( lnKey < lnKeyCount-1)
	{
		if( pnFrame < oChan->aRotKeys[lnKey+1].nFrame) break;
		lnKey++;
	}
					
	// interpolation
	int lnNextKey = (lnKey + 1) % lnKeyCount;
	fglAnimQuatKey& loKey = oChan->aRotKeys[lnKey];
	fglAnimQuatKey& loNextKey = oChan->aRotKeys[lnNextKey];
	float lnDFrame = loNextKey.nFrame - loKey.nFrame;
	if( lnDFrame < 0.0f) lnDFrame += nFrames;
	
	if( lnDFrame > 0.0f)
	{
		float lnFactor = float( (pnFrame - loKey.nFrame) / lnDFrame);
		pNextRot = loKey.q.interpolate(loNextKey.q, lnFactor); 
	} else {
		pNextRot = loKey.q;
	}

	nCurrRot = lnKey;
	return true;
}



