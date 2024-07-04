#ifndef _FGLANIM_H_
#define _FGLANIM_H_

#include "common.h"
#include "fgl3DGen.h"
#include "fglMem.h"

class fglAnimVectKey
{
public:
	float nFrame = { 0.0f };
	fglVECTOR v;
};

class fglAnimQuatKey
{
public:
	float nFrame{ 0.0f };
	fglQUAT q;
};

class fglAnimChan
{
public:
	fglString jointname{ "" };

	fglArray<fglAnimVectKey> aPosKeys;
	fglArray<fglAnimQuatKey> aRotKeys;
	fglArray<fglAnimVectKey> aScaleKeys;
	
	fglAnimChan(void) = default;
	//bad for perf virtual ~fglAnimChan(void) {};

};


class fglAnim 
{
public:
	fglString name{ "" };

	typedef fglUMap<fglString, fglREF<fglAnimChan>> ACHANS;
	ACHANS aChans;

	float nFrames{ 0.0f };
	unsigned nFPS{ 0 };

	fglAnim(void) = default;
	//bad for perf virtual ~fglAnim(void) {};

};


class fglAnimChanEval
{
public:
	fglREF<fglAnimChan> oChan;

	float nFrames{ 0.0f };

	fglTRANSFORM tr;
	int nCurrPos{ 0 };
	int nCurrRot{ 0 };
	int nCurrScale{ 0 };
	
	fglAnimChanEval(void) = default;
	fglAnimChanEval(fglREF<fglAnimChan>& poChan) { oChan = poChan; nFrames=0.0f;}
	//bad for perf virtual ~fglAnimChanEval(void);

	bool getNextPos(float pnFrame, fglVECTOR& pNextPos);
	bool getNextScale(float pnFrame, fglVECTOR& pNextScale);
	bool getNextRot(float pnFrame, fglQUAT& pNextRot);

	void build(float pnFrame);

};


class fglAnimEval
{
public:
	
	fglREF<fglAnim> oAnim;
	//typedef std::vector<fglAnimChanEval> ACHANS;
	//ACHANS aChans;

	float nStartTime{ 0.0f };
	float nCurrFrame{ 0.0f };

	int nRepeat{ 1 }; //repeat nRepeat times, value -1 means infinite loop
	float nSpeed{ 1.0f }; //animation speed factor x1.0, x1.5, x2.0 etc

	fglAnimEval(void) = default;
	fglAnimEval (fglREF<fglAnim>& poAnim, int pnRepeat=1, float pnSpeed=1.0f) 
	{
		oAnim = poAnim; 
		nStartTime = 0.0f;
		nCurrFrame = 0.0f;
		nRepeat = pnRepeat; 
		nSpeed = pnSpeed; 
		//fglAnim::ACHANS::iterator iChan = oAnim->aChans.begin(), iChanEnd = oAnim->aChans.end();
		//for (; iChan != iChanEnd; ++iChan) { aChans.push_back(fglAnimChanEval(iChan->second)); }

	};

	//bad for perf virtual ~fglAnimEval (void) {};

	void build(float pnTime);

};


#endif