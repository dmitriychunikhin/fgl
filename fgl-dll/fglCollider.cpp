#include "fglCollider.h"
#include "fglScene.h"


void fglCollider::call(fglEvent& poEvent)
{
	if (poEvent.name == fglEVENT_aNodes) { _RetValue2i(aNodes->getHFGL()); return; }

	if (poEvent.name == fglEVENT_testNode) 
	{
		_RetValue2l(false);
		const fglREF<fglNode>& loNode = getBind<fglNode>(_GetValue2i(poEvent.aParams[0]));
		const fglREF<fglCollideInfo>& loInfo = getBind<fglCollideInfo>(_GetValue2i(poEvent.aParams[1]));
		if (loNode.empty()) return;
		_RetValue2l(testNode(loNode.get(), loInfo.map()));
		return;
	}

	
	fglBind::call(poEvent);
}

bool fglCollider::testPair(const fglNode& poNode1, const fglNode& poNode2, fglCollideInfo::fglCollidePair* poPair) const
{
	
	if (poPair) poPair->clear();
	
	float lnDist;
	if (poNode1.bbox.testIntersection(poNode2.bbox, &lnDist)==false) return false;

	if (poPair)
	{
		poPair->oNode1 = poNode1.getRef();
		poPair->oNode2 = poNode2.getRef();
		poPair->nDist = lnDist;
	}

	return true;

}

bool fglCollider::testNode(const fglNode& poNode, fglCollideInfo* poInfo) const
{
	unsigned lnNodes = aNodes->size();
	const fglNode* loTestNode;
	bool llRes = false;
	fglCollideInfo::fglCollidePair loPair;

	if (poInfo) poInfo->clear();

	for (unsigned i = 0; i < lnNodes; i++)
	{
		if ( (loTestNode = aNodes->get(i).map()) == NULL ) continue;
		if ( loTestNode == &poNode ) continue;
		
		if ( testPair(poNode, *loTestNode, &loPair)==false ) { continue; }
		llRes = true;
		if (poInfo) 
		{
			poInfo->add(loPair);
			if ( poInfo->nMinDist >= loPair.nDist ) { poInfo->nMinDist = loPair.nDist; poInfo->oNode1 = loPair.oNode1; poInfo->oNode2 = loPair.oNode2; }
		}
	}
	return llRes;
}

void fglCollideInfo::call(fglEvent& poEvent)
{
	if (poEvent.name == fglEVENT_size) { _RetValue2i(size()); return; }
	if (poEvent.name == fglEVENT_empty) { _RetValue2l(empty()); return; }

	if (poEvent.name == fglEVENT_find) 
	{ 
		_RetValue2i(-1);
		if (!poEvent.nParams) return;
		const fglREF<fglNode>& loNode = getBind<fglNode>(_GetValue2i(poEvent.aParams[0]));
		if (loNode.empty()) return;
		_RetValue2i(find(loNode.get()));
		return;
	}
	
	if (poEvent.name == fglEVENT_nMinDist) { _RetValue2f( nMinDist==fglINFINITE ? -1.0f : nMinDist ); return; }
	if (poEvent.name == fglEVENT_hNode1) { if (oNode1.empty()) _RetValue2i(0); else _RetValue2i(oNode1.lock()->getHFGL()); return; }
	if (poEvent.name == fglEVENT_hNode2) { if (oNode2.empty()) _RetValue2i(0); else _RetValue2i(oNode2.lock()->getHFGL()); return; }

	if (poEvent.name == fglEVENT_getNode1 || poEvent.name == fglEVENT_getNode2 || poEvent.name == fglEVENT_getDist) 
	{ 
		 _RetValue2i(0);
		if ( ! poEvent.nParams) return;
		int lIndex = _GetValue2i(poEvent.aParams[0]);
		const fglCollidePair* loPair = get(lIndex);
		if ( !loPair ) return;
		const fglNode* loNode = NULL;
		if (poEvent.name == fglEVENT_getNode1) { if (loNode = loPair->oNode1.exist() ? loPair->oNode1.lock().map() : NULL) _RetValue2i(loNode->getHFGL()); return; }
		if (poEvent.name == fglEVENT_getNode2) { if (loNode = loPair->oNode2.exist() ? loPair->oNode2.lock().map() : NULL) _RetValue2i(loNode->getHFGL()); return; }
		if (poEvent.name == fglEVENT_getDist) { _RetValue2f(loPair->nDist); return; }
		return;
	}

	fglBind::call(poEvent);
}
