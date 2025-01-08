#include "fglSceneGraph.h"
#include "fglScene.h"
#include "fglModel.h"

typedef fglUMap<int, fglREF<fglVisual>> ACOPYVISUALS;
ACOPYVISUALS gaCopyVisuals;
ACOPYVISUALS::iterator giCopyVisuals;
ACOPYVISUALS::iterator giCopyVisualsEnd = gaCopyVisuals.end();
int gnCopyNodesLevel = 0;

void fglNode::call(fglEvent& poEvent)
{
	if (poEvent.name == fglEVENT_getPos) 
	{
		if ( poEvent.nParams!=2 ) return;
		_SetObjectProperty2vec( poEvent.aParams[0], (_GetValue2l(poEvent.aParams[1]) ? trg : tr).pos );
		return;
	}

	if (poEvent.name == fglEVENT_getScale)
	{
		if ( poEvent.nParams!=2 ) return;
		_SetObjectProperty2vec( poEvent.aParams[0], (_GetValue2l(poEvent.aParams[1]) ? trg : tr).scale );
		return;
	}

	if (poEvent.name == fglEVENT_getRotQuat)
	{
		if ( poEvent.nParams!=2 ) return;
		_SetObjectProperty2quat( poEvent.aParams[0], (_GetValue2l(poEvent.aParams[1]) ? trg : tr).rot );
		return;
	}
	if (poEvent.name == fglEVENT_getRotEuler)
	{
		if ( poEvent.nParams!=2 ) return;
		_SetObjectProperty2vec( poEvent.aParams[0], (_GetValue2l(poEvent.aParams[1]) ? trg : tr).rot.toEuler() * fglRAD2ANG );
		return;
	}

	if (poEvent.name == fglEVENT_getTrgConstaints)
	{
		if ( poEvent.nParams!=3 ) return;
		_SetObjectProperty2vec( poEvent.aParams[0], trgConstraints.rot );
		_SetObjectProperty2vec( poEvent.aParams[1], trgConstraints.pos );
		_SetObjectProperty2vec( poEvent.aParams[2], trgConstraints.scale );
		return;
	}
	if (poEvent.name == fglEVENT_setTrgConstaints)
	{
		if ( poEvent.nParams!=3 ) return;
		if ( poEvent.aParams[0].ev_type == 'O' ) trgConstraints.rot = _GetObjectProperty2vec(poEvent.aParams[0]);
		if ( poEvent.aParams[1].ev_type == 'O' ) trgConstraints.pos = _GetObjectProperty2vec(poEvent.aParams[1]);
		if ( poEvent.aParams[2].ev_type == 'O' ) trgConstraints.scale = _GetObjectProperty2vec(poEvent.aParams[2]);
		buildTransform();
		return;
	}
	
	if (poEvent.name == fglEVENT_buildTransform) {buildTransform(); return;}
	if (poEvent.name == fglEVENT_calcBounds) {calcBounds(); return;}

	if (poEvent.name == fglEVENT_isEnabled) {if (poEvent.nParams) isEnabled = _GetValue2l(poEvent.aParams[0]); _RetValue2l(isEnabled); return;}
	if (poEvent.name == fglEVENT_isVisible) {if (poEvent.nParams) { isVisible = _GetValue2l(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2l(isVisible); return;}
	if (poEvent.name == fglEVENT_boundsVisible) {if (poEvent.nParams) { bbox.isVisible = _GetValue2l(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2l(bbox.isVisible); return;}
	if (poEvent.name == fglEVENT_boundsMeshType) {if (poEvent.nParams) { bbox.meshType = _GetValue2i(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2i(bbox.meshType); return;}
	if (poEvent.name == fglEVENT_boundsColor) {if (poEvent.nParams) { bbox.meshColor.set(_GetValue2ui(poEvent.aParams[0])); setSceneRenderFlag(); } _RetValue2i(bbox.meshColor.get()); return;}
	if (poEvent.name == fglEVENT_boundsFill) {if (poEvent.nParams) { bbox.meshFill = _GetValue2l(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2l(bbox.meshFill); return;}
	if (poEvent.name == fglEVENT_boundsLineWidth) {if (poEvent.nParams) { bbox.meshLineWidth = _GetValue2f(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2f(bbox.meshLineWidth); return;}
	if (poEvent.name == fglEVENT_boundsLighting) {if (poEvent.nParams) { bbox.meshLighting = _GetValue2i(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2i(bbox.meshLighting); return;}
	

	if (poEvent.name == fglEVENT_getBounds)
	{
		if ( poEvent.nParams!=4 ) return;
		_SetObjectProperty2vec( poEvent.aParams[0], _GetValue2l(poEvent.aParams[3]) ? bbox.getMinGlobal() : bbox.getMinLocal() );
		_SetObjectProperty2vec( poEvent.aParams[1], _GetValue2l(poEvent.aParams[3]) ? bbox.getMaxGlobal() : bbox.getMaxLocal() );
		_SetObjectProperty2vec( poEvent.aParams[2], _GetValue2l(poEvent.aParams[3]) ? bbox.getSizeGlobal() : bbox.getSizeLocal() );
		return;
	}

	if (poEvent.name == fglEVENT_getBoundsExt)
	{
		if ( poEvent.nParams!=3 ) return;
		_SetObjectProperty2vec( poEvent.aParams[0], bboxExt.min );
		_SetObjectProperty2vec( poEvent.aParams[1], bboxExt.max );
		_SetObjectProperty2vec( poEvent.aParams[2], bboxExt.getSize() );
		return;
	}
	if (poEvent.name == fglEVENT_setBoundsExt) 
	{
		if ( ! poEvent.nParams ) return;
		if ( poEvent.nParams!=6 ) return;

		if ( poEvent.aParams[0].ev_type == 'O' && poEvent.aParams[1].ev_type == 'O' ) 
		{
			bboxExt.min = _GetObjectProperty2vec(poEvent.aParams[0]);
			bboxExt.max = _GetObjectProperty2vec(poEvent.aParams[1]);
		}
		else if (poEvent.aParams[0].ev_type != 'O' && poEvent.aParams[1].ev_type != 'O' && poEvent.aParams[2].ev_type != 'L' ) 
		{
			bboxExt.min.build( _GetValue2f(poEvent.aParams[0]), _GetValue2f(poEvent.aParams[1]), _GetValue2f(poEvent.aParams[2]) );
			bboxExt.max.build( _GetValue2f(poEvent.aParams[3]), _GetValue2f(poEvent.aParams[4]), _GetValue2f(poEvent.aParams[5]) );
		}
		else 
		{
			const fglREF<fglNode>& loNode = getBind<fglNode>(_GetValue2i(poEvent.aParams[0]));
			const fglREF<fglVisual>& loVisual = getBind<fglVisual>(_GetValue2i(poEvent.aParams[0]));
			if ( loNode.exist() ) 
			{
				loNode.map()->calcBounds();
				bboxExt.min = loNode.map()->bbox.min;
				bboxExt.max = loNode.map()->bbox.max;
			}
			else if ( loVisual.exist() ) 
			{
				loVisual.map()->calcBounds();
				bboxExt.min = loVisual.map()->bbox.min;
				bboxExt.max = loVisual.map()->bbox.max;
			}
		}
		calcBounds();
		return;
	}
	if (poEvent.name == fglEVENT_correctBoundsExt) 
	{
		if ( ! poEvent.nParams ) return;
		if ( poEvent.nParams!=6 ) return;
		if ( poEvent.aParams[0].ev_type == 'O' && poEvent.aParams[1].ev_type == 'O' ) 
		{
			bboxExt.min = bboxExt.min + _GetObjectProperty2vec(poEvent.aParams[0]);
			bboxExt.max = bboxExt.max + _GetObjectProperty2vec(poEvent.aParams[1]);
		}
		else if (poEvent.aParams[0].ev_type != 'O' && poEvent.aParams[1].ev_type != 'O' && poEvent.aParams[2].ev_type != 'L' ) 
		{
			bboxExt.min = bboxExt.min + fglVECTOR( _GetValue2f(poEvent.aParams[0]), _GetValue2f(poEvent.aParams[1]), _GetValue2f(poEvent.aParams[2]) );
			bboxExt.max = bboxExt.max + fglVECTOR( _GetValue2f(poEvent.aParams[3]), _GetValue2f(poEvent.aParams[4]), _GetValue2f(poEvent.aParams[5]) );
		}
		calcBounds();
		return;
	}



	if (poEvent.name == fglEVENT_hParent) 
	{ 
		if (poEvent.nParams) setParent( getBind<fglNode>(_GetValue2i(poEvent.aParams[0])) );
		if (getParent().empty()) _RetValue2i(0); else _RetValue2i(getParent()->getHFGL());
		return;
	}
	if (poEvent.name == fglEVENT_aChilds) { _RetValue2i(aChilds->getHFGL()); return; }

	if (poEvent.name == fglEVENT_find) 
	{ 
		_RetValue2i(0);
		if (!poEvent.nParams) return;
		const fglREF<fglNode>* loNode = find(_GetValue2c(poEvent.aParams[0]));
		if (loNode) _RetValue2i( (*loNode)->getHFGL() ); 
		return;
	}

	if (poEvent.name == fglEVENT_testInFrustum) 
	{ 
		_RetValue2l(0);
		if ( ! poEvent.nParams ) return;
		const fglREF<fglViewport>& loViewport = getBind<fglViewport>(_GetValue2i(poEvent.aParams[0]));
		if ( loViewport.empty() ) return;
		if ( testInFrustum( loViewport.get() ) ) _RetValue2l(1);
		return; 
	}
	

	if (poEvent.name == fglEVENT_aVisuals) { _RetValue2i(aVisuals->getHFGL()); return; }

	if (poEvent.name == fglEVENT_getVisuals)
	{
		fglArray<fglVisual*> laVisuals;

		fglString lcVisuals;
		
		getVisuals( _GetValue2c(poEvent.aParams[0]), _GetValue2c(poEvent.aParams[1]), laVisuals );

		fglArray<fglVisual*>::iterator liVisual=laVisuals.begin(), liVisualEnd = laVisuals.end();
		char lcBuf[255];
		for ( ; liVisual!=liVisualEnd; ++liVisual)
		{
			sprintf(lcBuf, "%d\0", (*liVisual)->getHFGL());
			lcVisuals = lcVisuals + fglString(lcBuf) + ",";
		}
		lcVisuals = lcVisuals.substr(0,lcVisuals.find_last_not_of(",")+1);
		_RetValue2c(lcVisuals);
		return;
	}

	if (poEvent.name == fglEVENT_getMaterials)
	{
		fglArray<fglMaterial*> laMaterials;

		fglString lcMaterials;
		
		getMaterials( _GetValue2c(poEvent.aParams[0]), _GetValue2c(poEvent.aParams[1]), laMaterials );

		fglArray<fglMaterial*>::iterator liMaterial=laMaterials.begin(), liMaterialEnd = laMaterials.end();
		char lcBuf[255];
		for ( ; liMaterial!=liMaterialEnd; ++liMaterial)
		{
			sprintf(lcBuf, "%d\0", (*liMaterial)->getHFGL());
			lcMaterials = lcMaterials + fglString(lcBuf) + ",";
		}
		lcMaterials = lcMaterials.substr(0,lcMaterials.find_last_not_of(",")+1);
		_RetValue2c(lcMaterials);
		return;
	}

	if ( poEvent.name == fglEVENT_rotateBy || poEvent.name == fglEVENT_rotateTo )
	{
		if (poEvent.nParams!=6) return;
		float lnRx = _GetValue2f(poEvent.aParams[0]) * fglANG2RAD;
		float lnRy = _GetValue2f(poEvent.aParams[1]) * fglANG2RAD;
		float lnRz = _GetValue2f(poEvent.aParams[2]) * fglANG2RAD;
		fglVECTOR loCenter(_GetValue2f(poEvent.aParams[3]), _GetValue2f(poEvent.aParams[4]), _GetValue2f(poEvent.aParams[5]));
		
		if ( poEvent.name == fglEVENT_rotateBy) rotateBy(fglQUAT(lnRx,lnRy,lnRz), &loCenter);
		else if ( poEvent.name == fglEVENT_rotateTo ) rotateTo(fglQUAT(lnRx,lnRy,lnRz), &loCenter);
		
		return;
	}

	if ( poEvent.name == fglEVENT_setRotQuat )
	{
		if (poEvent.nParams!=4) return;
		 
		tr.rot.a = _GetValue2f(poEvent.aParams[0]);
		tr.rot.v.x = _GetValue2f(poEvent.aParams[1]);
		tr.rot.v.y = _GetValue2f(poEvent.aParams[2]);
		tr.rot.v.z = _GetValue2f(poEvent.aParams[3]);
		buildTransform();
		
		return;
	}


	if (poEvent.name == fglEVENT_moveDir || poEvent.name == fglEVENT_moveBy || poEvent.name == fglEVENT_moveTo 
		|| poEvent.name == fglEVENT_scaleBy || poEvent.name == fglEVENT_scaleAt || poEvent.name == fglEVENT_scaleTo) 
	{
		if (poEvent.nParams!=3) return;
		float lnX = _GetValue2f(poEvent.aParams[0]);
		float lnY = _GetValue2f(poEvent.aParams[1]);
		float lnZ = _GetValue2f(poEvent.aParams[2]);

		if (poEvent.name == fglEVENT_moveDir) moveDir(fglVECTOR(lnX,lnY,lnZ));
		else if (poEvent.name == fglEVENT_moveBy) moveBy(fglVECTOR(lnX,lnY,lnZ));
		else if (poEvent.name == fglEVENT_moveTo) moveTo(fglVECTOR(lnX,lnY,lnZ));
		else if (poEvent.name == fglEVENT_scaleBy) scaleBy(fglVECTOR(lnX,lnY,lnZ));
		else if (poEvent.name == fglEVENT_scaleAt) scaleAt(fglVECTOR(lnX,lnY,lnZ));
		else if (poEvent.name == fglEVENT_scaleTo) scaleTo(fglVECTOR(lnX,lnY,lnZ));

		return;
	}

	if (poEvent.name == fglEVENT_lookAt) 
	{
		if (poEvent.nParams!=3) return;
		float lnX = _GetValue2f(poEvent.aParams[0]);
		float lnY = _GetValue2f(poEvent.aParams[1]);
		float lnZ = _GetValue2f(poEvent.aParams[2]);
		
		lookAt( fglVECTOR(lnX,lnY,lnZ) );
		return;
	}

	if (poEvent.name == fglEVENT_getLookAt) 
	{
		_RetValue2l(0);
		if (poEvent.nParams!=2) return;
		if (poEvent.aParams[0].ev_type!='O') return;
		
		fglVECTOR loLookAt = _GetValue2l(poEvent.aParams[1]) ? oLookVectorGlobal : oLookVectorLocal;
		_SetObjectProperty2vec(poEvent.aParams[0], loLookAt); 
		_RetValue2l(1); 
			
		return;
	}

	if (poEvent.name == fglEVENT_getLookAtPlane) 
	{
		_RetValue2l(0);
		if (poEvent.nParams!=2) return;
		if (poEvent.aParams[0].ev_type!='O' && poEvent.aParams[1].ev_type!='O') return;
		fglROW4 loPlane = _GetObjectProperty2row4(poEvent.aParams[0]);
		
		fglVECTOR loLookAt;
		if ( getLookAtPlane(loPlane, loLookAt) ) { _SetObjectProperty2vec(poEvent.aParams[1], loLookAt); _RetValue2l(1); }
	
		return;
	}

	fglBind::call(poEvent);
}

fglNode::fglNode(void) : bbox(tr, trg), oLookVector(0.0f,0.0f,-1.0f)
{
	aChilds->oParent = this;
	isEnabled = true;
	isVisible = true;
}

void fglNode::destroy(void) {
	setParentValue(fglREF<fglNode>());
	fglBind::destroy();
}


bool fglNode::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglNode* loNode = dynamic_cast<const fglNode*>(poSrc);
	if (!loNode) return false;

	if ( !fglBind::copy(poSrc, pnShare) ) return false;
	
	gnCopyNodesLevel++;

	tr = loNode->tr;
	trgConstraints = loNode->trgConstraints;
	bbox = loNode->bbox;
	bboxExt = loNode->bboxExt;
	isVisible = loNode->isVisible;
	isEnabled = loNode->isEnabled;

	buildTransform();
	
	aChilds->clear();
	fglREF<fglNode> loChild;
	fglREF<fglNode> loNewChild;
	for (unsigned i = 0; i < loNode->aChilds->size(); i++)
	{
		loChild= loNode->aChilds->get(i);
		if ( loChild.empty() ) continue;
		
		loNewChild = fglBindFactory::duplicateNonPersistent<fglNode>(loChild, fglDUPLICATE_COPY, pnShare);
		if ( loNewChild.exist() )
		{
			aChilds->add( loNewChild );
		}

	}

	aVisuals->clear();
	fglREF<fglVisual> loVisual;
	fglREF<fglVisual> loNewVisual;
	for (unsigned i = 0; i < loNode->aVisuals->size(); i++)
	{
		loVisual = loNode->aVisuals->get(i);
		if ( loVisual.empty() ) continue;
		
		giCopyVisuals = gaCopyVisuals.find(loVisual->getHFGL());
		if ( giCopyVisuals != giCopyVisualsEnd )
		{
			aVisuals->add(giCopyVisuals->second);
		}
		else
		{
			loNewVisual = fglBindFactory::duplicateNonPersistent<fglVisual>(loVisual, (pnShare.check(fglSHARE_AVISUALS) ? fglDUPLICATE_INSTANCE : fglDUPLICATE_COPY), pnShare);
			if (loNewVisual.exist())
			{
				aVisuals->add(loNewVisual);
				gaCopyVisuals[loVisual->getHFGL()] = loNewVisual;
			}
		}
	}
	
	gnCopyNodesLevel--;
	if (gnCopyNodesLevel==0) gaCopyVisuals.clear();
	
	return true;
}

bool fglNode::setParentValue(const fglREF<fglNode>& poParent)
{
	fglREF<fglNode> loParent = poParent;
	if (oParent.lock().map() == loParent.map()) return true;
	fglNode* loNode = loParent.map();
	while (loNode) { if (this == loNode) return false; loNode = loNode->getParent().map(); }
	if (oParent.exist()) oParent.lock()->aChilds->del(getRef());
	oParent = loParent;
	buildTransform();
	return true;
}

bool fglNode::setParent(const fglREF<fglNode>& poParent)
{
	if ( ! setParentValue(poParent) ) return false;
	if (oParent.exist()) oParent.lock()->aChilds->add(getRef());
	return true;
}


void fglNode::buildTransform(void)
{
	trg = tr;

	int lnChans = aAnimChans->size();
	if (lnChans > 0)
	{
		for (unsigned i = 0; i != lnChans; i++)
			if (aAnimChans->get(i).exist()) trg = aAnimChans->get(i)->tr;
	}
	if (oParent.exist())  trg = (oParent.lock()->trg * trgConstraints) * trg; 
	
	oLookVectorLocal = tr.rot * oLookVector;
	oLookVectorGlobal = trg.rot * oLookVector;
	trg.toMatrix(trgMatrix);
		
	fglNode* loChild = NULL;
	int lnChilds = aChilds->size();
	
	while (lnChilds--) { loChild = aChilds->get(lnChilds).map(); if (loChild) loChild->buildTransform(); }

	setSceneRenderFlag();
}


void fglNode::calcBounds(void)
{
	bbox.min.x=bbox.min.y=bbox.min.z=fglINFINITE;
	bbox.max.x=bbox.max.y=bbox.max.z=-fglINFINITE;

	fglVisual* loVisual;
	unsigned lnVisuals = aVisuals->size();

	while (lnVisuals--)
	{
		loVisual = aVisuals->get(lnVisuals).map();
		if (!loVisual) continue;
		loVisual->calcBounds();
		if ( loVisual->bbox.isNull() ) continue;
		bbox.correct(loVisual->bbox.min);
		bbox.correct(loVisual->bbox.max);
	}

	fglNode* loNode;
	unsigned lnNodes = aChilds->size();
	while (lnNodes--)
	{
		loNode = aChilds->get(lnNodes).map();
		if (!loNode) continue;
		loNode->calcBounds();
		if ( loNode->bbox.isNull() ) continue;
		if ( ! loNode->isVisible ) continue;
		
		bbox.correct(loNode->bbox.getMinLocal());
		bbox.correct(loNode->bbox.getMaxLocal());
	}

	if (bbox.min.x==fglINFINITE) bbox.setNull();

	if ( ! bboxExt.isNull() ) 
	{
		bbox.min = bboxExt.min;
		bbox.max = bboxExt.max;
	}
	
	setSceneRenderFlag();
}


bool fglNode::testInFrustum(const fglViewport& poViewport) const
{
	if (poViewport.oCam.empty()) return false;
	if ( poViewport.oFrustum.testBoundBox(bbox)==fglFRUSTUM::OUTSIDE ) return false;
	return true;
}


void fglNode::lookAt(const fglVECTOR& poLookAt, const float& pnDist)
{
	fglVECTOR& loFwd = oLookVectorGlobal;
	fglVECTOR loFwdNew = (poLookAt - trg.pos).normalize();
	//getScene()->oLog.info(loFwd.toString());
	//getScene()->oLog.info(loFwdNew.toString());

	fglQUAT loRot; loRot.build(loFwd, loFwdNew);
	//getScene()->oLog.info(loRot.toString());

	tr.rot = tr.rot * loRot;
	buildTransform();
}

bool fglNode::getLookAtPlane(const fglROW4& poPlane, fglVECTOR& poLookAt) const
{
	fglRAY loRay;
	loRay.start = trg.pos;
	loRay.end = trg.pos + (oLookVectorGlobal) * fglINFINITE;
	return loRay.testInPlane( poPlane, &poLookAt );
}

void fglNode::getVisuals(const fglString& pcVisualName, const fglString& pcMaterialName, fglArray<fglVisual*>& paResult) 
{
	class getVisuals : public fglWalkNodes 
	{
	public:
		const fglString& cVisualName;
		const fglString& cMaterialName;
		fglArray<fglVisual*>& aResult;
		static void setRes(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual)
		{
			getVisuals* loHandler = static_cast<getVisuals*>(poHandler);
			if ( ! loHandler->cVisualName.empty() ) if (poVisual->name != loHandler->cVisualName) return;
			if ( ! loHandler->cMaterialName.empty() && poVisual->oMaterial.exist() ) if (poVisual->oMaterial->name != loHandler->cMaterialName) return;
			if ( ! loHandler->cMaterialName.empty() && ! poVisual->oMaterial.exist()) return;
			loHandler->aResult.push_back(poVisual);
			
			fglModel* loModel = dynamic_cast<fglModel*>(poVisual);
			if ( ! loModel ) return;
			if ( ! loModel->getRootJoint().exist() ) return;
			loModel->getRootJoint()->getVisuals( loHandler->cVisualName, loHandler->cMaterialName, loHandler->aResult );

		}
		getVisuals(const fglString& pcVisualName, const fglString& pcMaterialName, fglArray<fglVisual*>& paResult): 
			cVisualName(pcVisualName), cMaterialName(pcMaterialName), aResult(paResult) 
			{ onVisual = setRes; }

	} loGetVisuals(pcVisualName, pcMaterialName, paResult);
	
	fglWalkNodes::walkNodes(this, &loGetVisuals);
}

void fglNode::getMaterials(const fglString& pcVisualName, const fglString& pcMaterialName, fglArray<fglMaterial*>& paResult)
{
	class getMaterials : public fglWalkNodes 
	{
	public:
		const fglString& cVisualName;
		const fglString& cMaterialName;
		fglArray<fglMaterial*>& aResult;
		static void setRes(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual)
		{
			getMaterials* loHandler = static_cast<getMaterials*>(poHandler);
			if ( poVisual->oMaterial.empty() ) return;
			if ( ! loHandler->cVisualName.empty() ) if (poVisual->name != loHandler->cVisualName) return;
			if ( ! loHandler->cMaterialName.empty() ) if (poVisual->oMaterial->name != loHandler->cMaterialName) return;
			loHandler->aResult.push_back(poVisual->oMaterial.map());

			fglModel* loModel = dynamic_cast<fglModel*>(poVisual);
			if ( ! loModel ) return;
			if ( ! loModel->getRootJoint().exist() ) return;
			loModel->getRootJoint()->getMaterials( loHandler->cVisualName, loHandler->cMaterialName, loHandler->aResult );
		}
		getMaterials(const fglString& pcVisualName, const fglString& pcMaterialName, fglArray<fglMaterial*>& paResult): 
			cVisualName(pcVisualName), cMaterialName(pcMaterialName), aResult(paResult)
			{ onVisual = setRes; }

	} loGetMaterials(pcVisualName, pcMaterialName, paResult);
	
	fglWalkNodes::walkNodes(this, &loGetMaterials);

}
