#include "fglViewport.h"
#include "fglScene.h"

fglViewport::renderNodes fglViewport::oRenderNodes;

void fglViewport::call(fglEvent& poEvent)
{
	
	if (poEvent.name == fglEVENT_move) 
	{
		if (poEvent.nParams!=4) return;
		oRect.left = _GetValue2i(poEvent.aParams[0]);
		oRect.top = _GetValue2i(poEvent.aParams[1]);
		oRect.width = _GetValue2i(poEvent.aParams[2]);
		oRect.height = _GetValue2i(poEvent.aParams[3]);

		const fglScene* loScene = getScene().lock().map();
		if (loScene && loScene != this) oRect.top = loScene->oRect.height - oRect.top - oRect.height;
		
		setSceneRenderFlag();
		return;
	}
	
	if (poEvent.name == fglEVENT_backColor) { if (poEvent.nParams) { oBackColor.set( _GetValue2ui(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(oBackColor.get()); return; }
	if (poEvent.name == fglEVENT_nProj) { if (poEvent.nParams) { nProj = _GetValue2i(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2i(nProj); return; }

	if (poEvent.name == fglEVENT_hCam) 
	{
		if (poEvent.nParams) { oCam.inst( getBind<fglCam>(_GetValue2i(poEvent.aParams[0])) ); setSceneRenderFlag(); }
		if (oCam.empty()) _RetValue2i(0); else _RetValue2i(oCam->getHFGL());
		return;
	}

	if (poEvent.name == fglEVENT_hMaterial) 
	{
		if (poEvent.nParams) { oMaterial.inst( getBind<fglMaterial>(_GetValue2i(poEvent.aParams[0])) ); setSceneRenderFlag(); } 
		if (oMaterial.empty()) _RetValue2i(0); else _RetValue2i(oMaterial->getHFGL());
		return;
	}

	if (poEvent.name == fglEVENT_hRenderNode) 
	{
		if (poEvent.nParams) { oRenderNode.inst( getBind<fglNode>(_GetValue2i(poEvent.aParams[0])) ); setSceneRenderFlag(); }
		if (oRenderNode.empty()) _RetValue2i(0); else _RetValue2i(oRenderNode->getHFGL());
		return;
	}

	if (poEvent.name == fglEVENT_aLights) { _RetValue2i(aLights->getHFGL()); return; }
	

	if (poEvent.name == fglEVENT_project) 
	{
		_RetValue2l(0);
		if (poEvent.nParams!=4) return;
		if ( poEvent.aParams[0].ev_type == 'O' && poEvent.aParams[1].ev_type == 'O' )
		{
			fglVECTOR loWorldPoint, loWindowPoint;
			loWorldPoint = _GetObjectProperty2vec( poEvent.aParams[0] );
			loWindowPoint = project(loWorldPoint);
			_SetObjectProperty2vec( poEvent.aParams[1], loWindowPoint );
			_RetValue2l(1);
		}
		else if ( poEvent.aParams[3].ev_type == 'O' )
		{
			fglVECTOR loWorldPoint, loWindowPoint;
			loWorldPoint.x = _GetValue2f( poEvent.aParams[0] );
			loWorldPoint.y = _GetValue2f( poEvent.aParams[1] );
			loWorldPoint.z = _GetValue2f( poEvent.aParams[2] );
			loWindowPoint = project(loWorldPoint);
			_SetObjectProperty2vec( poEvent.aParams[3], loWindowPoint );
			_RetValue2l(1);
		}
		return;
	}
	
	if (poEvent.name == fglEVENT_unproject) 
	{
		_RetValue2l(0);
		if (poEvent.nParams!=4) return;
		
		if ( poEvent.aParams[0].ev_type == 'O' && poEvent.aParams[1].ev_type == 'O' )
		{
			fglVECTOR loWindowPoint, loWorldPoint;
			loWindowPoint = _GetObjectProperty2vec( poEvent.aParams[0] );
			loWorldPoint = unproject(loWindowPoint);
			_SetObjectProperty2vec( poEvent.aParams[1], loWorldPoint );
			_RetValue2l(1);
		}
		else if ( poEvent.aParams[3].ev_type == 'O' )
		{
			fglVECTOR loWindowPoint, loWorldPoint;
			loWindowPoint.x = _GetValue2f( poEvent.aParams[0] );
			loWindowPoint.y = _GetValue2f( poEvent.aParams[1] );
			loWindowPoint.z = _GetValue2f( poEvent.aParams[2] );
			loWorldPoint = unproject(loWindowPoint);
			_SetObjectProperty2vec( poEvent.aParams[3], loWorldPoint );
			_RetValue2l(1);
		}
		return;
	}

	fglBind::call(poEvent);
}


bool fglViewport::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglViewport* loViewport = dynamic_cast<const fglViewport*>(poSrc);
	if (!loViewport) return false;

	if ( !fglBind::copy(poSrc, pnShare) ) return false;

	oRenderNode = loViewport->oRenderNode;
	aLights->copy(loViewport->aLights.callBindREFMap(), pnShare.check(fglSHARE_ALIGHTS));
	
	nProj = loViewport->nProj;
	
	return true;
}

void fglViewport::selectLights(const fglCollection<fglLight>& paLights)
{
	unsigned lnLights;
	unsigned lnMaxLights = fglScene::aRenderLights.capacity();
	unsigned liLight;
	fglLight* loLight;
	unsigned i;

	lnLights = paLights.size();
	liLight = 0;
	for (i = 0; i < lnLights; i++)
	{
		if (liLight==lnMaxLights) break;
		if ( (loLight = paLights[i].map()) == NULL ) continue;
		if ( ! loLight->testInFrustum(fglScene::oRenderViewport->oFrustum) ) continue;
		
		fglScene::aRenderLights[liLight] = loLight;
		liLight++;
	}

	fglScene::nRenderLights = liLight;
}

void fglViewport::applyLights(void)
{
	if ( ! fglScene::oRenderMaterial ) return;
	if ( fglScene::oRenderMaterial->renderLighting != 1 ) return;
	
	unsigned lnMaxLights = fglScene::oGLHelper.getMaxLights();
	unsigned liLight;
	fglLight* loLight;
	fglTRANSFORM loCurViewInv;
	
	fglScene::oGLHelper.resetLights();
	
	if ( ! fglScene::nRenderLights ) return;
	
	loCurViewInv = oRenderNodes.currentView.invert();
	liLight = 0;
	for (unsigned i = 0; i < fglScene::nRenderLights; i++)
	{
		if (liLight == lnMaxLights) return;
		loLight = fglScene::aRenderLights[i];
		if (loLight->isSpot) if ( (loLight->trg.pos - oRenderNodes.currentView.pos).norm() > (loLight->squareRadius) ) continue;
		loLight->setLight(liLight, loCurViewInv);
		liLight++;
	}
}

fglVECTOR fglViewport::project(const fglVECTOR& poWorldPoint)
{
	fglVECTOR loWindowPoint;
	const fglScene* loScene = getScene().lock().map();

	if ( !loScene ) return loWindowPoint;
	if ( oCam.empty() ) return loWindowPoint;
	
	buildProjection();
	
	if ( fglProject(poWorldPoint, fglScene::identTransMatrix, mProj, oRect, loWindowPoint) )
	{
		loWindowPoint.x = float(loWindowPoint.x - loScene->oWindowRect.left);
		loWindowPoint.y = float(oRect.height - loWindowPoint.y + loScene->oWindowRect.top);
	}

	return loWindowPoint;
}

fglVECTOR fglViewport::unproject(fglVECTOR poWindowPoint)
{
	fglVECTOR loWorldPoint;
	
	const fglScene* loScene = getScene().lock().map();

	if ( !loScene ) return loWorldPoint;
	if ( oCam.empty() ) return loWorldPoint;
	
	buildProjection();
	
	poWindowPoint.x=float(poWindowPoint.x - loScene->oWindowRect.left);
	poWindowPoint.y=float(oRect.height - poWindowPoint.y + loScene->oWindowRect.top);

	fglUnProject(poWindowPoint, fglScene::identTransMatrix, mProj, oRect, loWorldPoint);
	return loWorldPoint;
}


void fglViewport::buildProjection(void)
{
	if (oCam.empty()) return;
	
	const float& lnZNear = oCam->getZNear();
	const float& lnZFar = oCam->getZFar();

	mProj.loadIdentity();
	mProjPerspective.loadIdentity();
	mProjOrtho.loadIdentity();
	mProjOrthoViewportCam.loadIdentity();

	float lnWidth = static_cast<float>(std::max<long>(oRect.width, 1));
	float lnHeight = static_cast<float>(std::max<long>(oRect.height, 1));
	float lnAspect = lnWidth / lnHeight;
	
	float lnScaleY = 1.0f / oCam->getHalfFovTan();
	float lnScaleX = lnScaleY / lnAspect;
	float lnDeltaZ = std::max<float>(lnZFar - lnZNear, 1.0f);

	mProjPerspective.a1 = lnScaleX; 
	mProjPerspective.b2 = lnScaleY; 
	mProjPerspective.c3 = -(lnZFar+lnZNear)/lnDeltaZ; 
	mProjPerspective.d3 = -1.0f;
	mProjPerspective.c4 = -2.0f*lnZNear*lnZFar/lnDeltaZ; 
	mProjPerspective.d4 = 0.0f;
	

	float lnOrthoScaleY = 2.0f;
	float lnOrthoScaleX = lnOrthoScaleY * lnAspect;

	mProjOrtho.a1 = 2.0f / lnOrthoScaleX;
	mProjOrtho.b2 = 2.0f / lnOrthoScaleY;
	mProjOrtho.c3 = -2.0f / lnDeltaZ;
	mProjOrtho.a4 = 0.0f;
	mProjOrtho.b4 = 0.0f;
	mProjOrtho.c4 = -(lnZFar + lnZNear) / lnDeltaZ;


	mProjOrthoViewportCam.a1 = 2.0f / lnWidth;
	mProjOrthoViewportCam.b2 = -2.0f / lnHeight;
	mProjOrthoViewportCam.c3 = -2.0f / lnDeltaZ;
	mProjOrthoViewportCam.a4 = -(oRect.left + oRect.left + oRect.width) / lnWidth;
	mProjOrthoViewportCam.b4 = (oRect.top + oRect.top + oRect.height) / lnHeight;
	mProjOrthoViewportCam.c4 = -(lnZFar + lnZNear) / lnDeltaZ;


	if (nProj == nProjPerspective)
	{
		mProj = mProjPerspective;
		mProj = mProj * oCam->trgInvMatrix;
	}
	else if (nProj == nProjOrtho)
	{
		mProj = mProjOrtho;
		mProj = mProj * oCam->trgInvMatrix;
	}
	else if (nProj == nProjOrthoViewportCam)
	{
		mProj = mProjOrthoViewportCam;
		mProj = mProj * oCam->trgInvMatrix;
	}
	else
	{
		mProj = mProjPerspective;
		mProj = mProj * oCam->trgInvMatrix;
	}
	
	oFrustum.build(mProj);
	
}

void fglViewport::setProjection(void)
{
	mProj.setCurrentProjection();
}

void fglViewport::render(void)
{
	if (oCam.empty()) return;

	glScissor( oRect.left, oRect.top, oRect.width, oRect.height );
	glViewport( oRect.left, oRect.top, oRect.width, oRect.height );
	glDepthRange(0.0f, 1.0f);
	glClearColor(oBackColor.r, oBackColor.g, oBackColor.b, oBackColor.a);
	glClearDepth( 1.0f );
	if (oBackColor.a == 1.0f)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else if (oBackColor.a == 0.0f)
	{
		glClear( GL_DEPTH_BUFFER_BIT);
	}
	else
	{
		glClear( GL_DEPTH_BUFFER_BIT);

		fglScene::identTransMatrix.setCurrentProjection();
		fglScene::identTransMatrix.setCurrentView();

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		fglScene* loScene = getScene().lock().map();
		if (loScene)
		{
			loScene->o3DPrimitives.oMaterial->reset();
			loScene->o3DPrimitives.oMaterial->lighting = 0;
			loScene->o3DPrimitives.oMaterial->color_diffuse.set(oBackColor.r, oBackColor.g, oBackColor.b, oBackColor.a);
			loScene->o3DPrimitives.renderQuad(fglVECTOR(), fglVECTOR(2.0f, 2.0f, 1.0f), 2, 1, loScene->o3DPrimitives.oMaterial.callBindREFMap());
		}

		//glColor4f(oBackColor.r, oBackColor.g, oBackColor.b, oBackColor.a);
		//glBegin(GL_QUADS);
		//glVertex3f(-1.0f, -1.0f, 0.0f);
		//glVertex3f(1.0f, -1.0f, 0.0f);
		//glVertex3f(1.0f, 1.0f, 0.0f);
		//glVertex3f(-1.0f, 1.0f, 0.0f);
		//glEnd();
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	buildProjection();
	setProjection();

	fglScene::oRenderViewport = this;
	fglScene::oRenderCam = oCam.map();


	if (oMaterial.exist()) oMaterial->bind();

	selectLights(aLights.callBindREFGet());
	
	fglWalkNodes::walkNodes(oRenderNode.map(), &oRenderNodes);
	
	if (oMaterial.exist()) oMaterial->unbind();

	fglScene::oRenderViewport = NULL;
	fglScene::oRenderCam = NULL;

}


void fglViewport::renderNodes::renderVisual(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual)
{
	if ( ! poVisual->testInFrustum(*fglScene::oRenderViewport, poHandler->currentView) ) return;
	poVisual->render();
}

bool fglViewport::renderNodes::renderNode(fglWalkNodes* poHandler, fglNode* poNode)
{
	if ( ! poNode->isVisible ) return false;
	if ( poNode->bbox.isVisible ) poNode->bbox.render();
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int fglPickInfo::pick(long pnX, long pnY, fglNode* poStartNode, fglViewport* poViewport)
{
	aPickedVisuals->clear();
	aPickedNodes->clear();
	oPickedVisual.release();
	oPickedNode.release();
	oPickedPoint = fglVECTOR();
	oPickedPointGlobal = fglVECTOR();
	oWinP = fglVECTOR();
	oStartP = fglVECTOR();
	nMinDist = fglINFINITE;
	
	const fglScene* loScene = getScene().lock().map();
	if ( !loScene ) return 0;

	fglViewport* loViewport = oViewport.lock().map();
	if (poViewport) loViewport = poViewport;
	if (!loViewport) return 0;

	fglNode* loStartNode = oStartNode.lock().map();
	if (poStartNode) loStartNode = poStartNode;
	if (!loStartNode) loStartNode = loViewport->oRenderNode.map();
	if (!loStartNode) return 0;

	if ( loViewport->oCam.empty() ) return 0;

	loViewport->buildProjection();
	
	oWinP.x = float(pnX - loScene->oWindowRect.left);
	oWinP.y = float(loViewport->oRect.height - pnY + loScene->oWindowRect.top);

	oWinP.z = 0.0f; if ( ! fglUnProject(oWinP, fglScene::identTransMatrix, loViewport->mProj, loViewport->oRect, oStartP) ) return 0;

	fglWalkNodes::walkNodes(loStartNode, &oPickNodes);
	
	if (nMinDist > 0.0f && nMinDist < fglINFINITE) nMinDist = sqrtf(nMinDist);

	if (isPickNearest && oPickedNode.exist() && oPickedVisual.exist())
	{
		aPickedVisuals->clear();
		aPickedNodes->clear();
		aPickedVisuals->add(oPickedVisual.lock()); 
		aPickedNodes->add(oPickedNode.lock());
	}
	
	return aPickedNodes->size();
}

bool fglPickInfo::pickNodes::testNode(fglWalkNodes* poHandler, fglNode* poNode)
{
	pickNodes* loHandler = static_cast<pickNodes*>(poHandler);
	fglPickInfo* loPickInfo = loHandler->oPickInfo;

	if (poNode->isEnabled==false && loPickInfo->isPickDisabled==false) return false;
	if (poNode->isVisible == false) return false;
	return true;
}

void fglPickInfo::pickNodes::testVisual(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual) 
{
	pickNodes* loHandler = static_cast<pickNodes*>(poHandler);
	fglPickInfo* loPickInfo = loHandler->oPickInfo;
	const fglViewport* loViewport = loPickInfo->oViewport.lock().map();
	
	if (!loViewport) return;

	if ( ! poVisual->testInFrustum(*loViewport, poHandler->currentView) ) return;


	fglRAY loRay;
	fglVECTOR loPointIn;
	fglVECTOR loPointInGlobal;
	
	loPickInfo->oWinP.z = 0.0f; if ( ! fglUnProject(loPickInfo->oWinP, poHandler->currentViewMatrix, loViewport->mProj, loViewport->oRect, loRay.start) ) return;
	loPickInfo->oWinP.z = 1.0f; if ( ! fglUnProject(loPickInfo->oWinP, poHandler->currentViewMatrix, loViewport->mProj, loViewport->oRect, loRay.end) ) return;
	
	
	loRay.radius = loPickInfo->nPickRadius;
	if ( ! poVisual->testIntersection(loRay, &loPointIn) ) return;
	
	
	if ( loPickInfo->isPickNearest == false )
	{
		loPickInfo->aPickedVisuals->add(poVisual->getRef()); 
		loPickInfo->aPickedNodes->add(poNode->getRef());
	}
	
	loPointInGlobal = poNode->trg * loPointIn;
	float lnDist = (loPointInGlobal - loPickInfo->oStartP).norm();
	if (loPickInfo->nMinDist > lnDist) 
	{ 
		loPickInfo->nMinDist = lnDist;
		loPickInfo->oPickedVisual = poVisual->getRef(); 
		loPickInfo->oPickedNode = poNode->getRef(); 
		loPickInfo->oPickedPoint = loPointIn;
		loPickInfo->oPickedPointGlobal = loPointInGlobal;
	}
}

fglVECTOR fglPickInfo::getMove(const fglNode& poNode, unsigned pnConstraint, long pnX1, long pnY1, long pnX2, long pnY2) const
{
	fglVECTOR loMove;

	const fglScene* loScene = getScene().lock().map();
	if ( !loScene ) return loMove;

	const fglViewport* loViewport = oViewport.lock().map();
	if (!loViewport) return loMove;

	if ( loViewport->oCam.empty() ) return loMove;
	
	fglVECTOR loWinP1, loWinP2;
	loWinP1.x = float(pnX1 - loScene->oWindowRect.left);
	loWinP1.y = float(loViewport->oRect.height - pnY1 + loScene->oWindowRect.top);
	loWinP2.x = float(pnX2 - loScene->oWindowRect.left);
	loWinP2.y = float(loViewport->oRect.height - pnY2 + loScene->oWindowRect.top);
	loWinP2.z = 0.0f; 

	const fglMAT4& lmNode(poNode.trgMatrix); 

	///////////////////////////////////////////////
	fglVECTOR loPlaneNorm;
	if (pnConstraint == XY) loPlaneNorm.z = 1.0f;
	else if (pnConstraint == YZ) loPlaneNorm.x = 1.0f;
	else if (pnConstraint == XZ) loPlaneNorm.y = 1.0f;
	else if (pnConstraint == XYZ) { loPlaneNorm.x = 1.0f; loPlaneNorm.y = 1.0f; loPlaneNorm.z = 1.0f; }

	fglRAY loRay;
	///////////////////////////////////////////////
	loWinP1.z = 0.0f; if ( ! fglUnProject(loWinP1, lmNode, loViewport->mProj, loViewport->oRect, loRay.start) ) return loMove;
	loWinP1.z = 1.0f; if ( ! fglUnProject(loWinP1, lmNode, loViewport->mProj, loViewport->oRect, loRay.end) ) return loMove;

	if (pnConstraint == X) { loPlaneNorm = loRay.start-loRay.end; loPlaneNorm.x = 0.0f; }
	else if (pnConstraint == Y) { loPlaneNorm = loRay.start-loRay.end; loPlaneNorm.y = 0.0f; }
	else if (pnConstraint == Z) { loPlaneNorm = loRay.start-loRay.end; loPlaneNorm.z = 0.0f; }
	
	fglVECTOR loPoint1;
	if ( ! loRay.testInPlane( fglROW4(loPlaneNorm, 0), &loPoint1 ) ) return loMove;
	///////////////////////////////////////////////

	///////////////////////////////////////////////
	loWinP2.z = 0.0f; if ( ! fglUnProject(loWinP2, lmNode, loViewport->mProj, loViewport->oRect, loRay.start) ) return loMove;
	loWinP2.z = 1.0f; if ( ! fglUnProject(loWinP2, lmNode, loViewport->mProj, loViewport->oRect, loRay.end) ) return loMove;

	if (pnConstraint == X) { loPlaneNorm = loRay.start-loRay.end; loPlaneNorm.x = 0.0f; }
	else if (pnConstraint == Y) { loPlaneNorm = loRay.start-loRay.end; loPlaneNorm.y = 0.0f; }
	else if (pnConstraint == Z) { loPlaneNorm = loRay.start-loRay.end; loPlaneNorm.z = 0.0f; }

	fglVECTOR loPoint2;
	if ( ! loRay.testInPlane( fglROW4(loPlaneNorm, 0), &loPoint2 ) ) return loMove;
	///////////////////////////////////////////////
	
	loMove = loPoint2 - loPoint1;
	if (pnConstraint == X) loMove.y = loMove.z = 0.0f;
	else if (pnConstraint == Y) loMove.x = loMove.z = 0.0f;
	else if (pnConstraint == Z) loMove.x = loMove.y = 0.0f;
	else if (pnConstraint == XY) loMove.z = 0.0f;
	else if (pnConstraint == YZ) loMove.x = 0.0f;
	else if (pnConstraint == XZ) loMove.y = 0.0f;
	
	return loMove;
}

void fglPickInfo::call(fglEvent& poEvent)
{

	if (poEvent.name == fglEVENT_hViewport) 
	{
		if (poEvent.nParams) oViewport = getBind<fglViewport>(_GetValue2i(poEvent.aParams[0]));
		if (oViewport.empty()) _RetValue2i(0); else _RetValue2i(oViewport.lock()->getHFGL());
		return;
	}

	if (poEvent.name == fglEVENT_hPickedVisual) { if (oPickedVisual.empty()) _RetValue2i(0); else _RetValue2i(oPickedVisual.lock()->getHFGL()); return; }
	if (poEvent.name == fglEVENT_hPickedNode) { if (oPickedNode.empty()) _RetValue2i(0); else _RetValue2i(oPickedNode.lock()->getHFGL()); return; }

	if (poEvent.name == fglEVENT_aPickedVisuals) { if (aPickedVisuals->empty()) _RetValue2i(0); else _RetValue2i(aPickedVisuals->getHFGL()); return; }
	if (poEvent.name == fglEVENT_aPickedNodes) { if (aPickedNodes->empty()) _RetValue2i(0); else _RetValue2i(aPickedNodes->getHFGL()); return; }

	if (poEvent.name == fglEVENT_nMinDist) { _RetValue2f(nMinDist); return; }

	if (poEvent.name == fglEVENT_isPickNearest) { if (poEvent.nParams) isPickNearest = _GetValue2l(poEvent.aParams[0]); _RetValue2l(isPickNearest); return; }
	if (poEvent.name == fglEVENT_isPickDisabled) { if (poEvent.nParams) isPickDisabled = _GetValue2l(poEvent.aParams[0]); _RetValue2l(isPickDisabled); return; }
	if (poEvent.name == fglEVENT_nPickRadius) { if (poEvent.nParams) nPickRadius = _GetValue2f(poEvent.aParams[0]); _RetValue2f(nPickRadius); return; }
		

	if (poEvent.name == fglEVENT_pick) 
	{
		_RetValue2i(0);
		if (poEvent.nParams!=6) return;

		const fglREF<fglNode>& loNodeRef = getBind<fglNode>(_GetValue2i(poEvent.aParams[2]));
		const fglREF<fglViewport>& loViewportRef = getBind<fglViewport>(_GetValue2i(poEvent.aParams[3]));
		fglNode* loNode = loNodeRef.map();
		fglViewport* loViewport = loViewportRef.map();
		_RetValue2i( pick(_GetValue2i(poEvent.aParams[0]), _GetValue2i(poEvent.aParams[1]), loNode, loViewport) ) ; 
		_SetObjectProperty2vec( poEvent.aParams[4], oPickedPoint );
		_SetObjectProperty2vec( poEvent.aParams[5], oPickedPointGlobal );
		return;
	}

	if (poEvent.name == fglEVENT_getMove) 
	{
		_RetValue2l(0);
		
		fglVECTOR loMove;
		if (poEvent.nParams=7) 
		{
			const fglREF<fglNode>& loNodeRef = getBind<fglNode>(_GetValue2i(poEvent.aParams[0]));
			fglNode* loNode = loNodeRef.map();
			if (loNode) 
			{
				long lnConstraint, lnX1, lnY1, lnX2, lnY2;
				lnConstraint = _GetValue2i(poEvent.aParams[1]);
				lnX1 = _GetValue2i(poEvent.aParams[2]);
				lnY1 = _GetValue2i(poEvent.aParams[3]);
				lnX2 = _GetValue2i(poEvent.aParams[4]);
				lnY2 = _GetValue2i(poEvent.aParams[5]);
		
				loMove = getMove(*loNode, lnConstraint, lnX1, lnY1, lnX2, lnY2);
			}

			_SetObjectProperty2vec(poEvent.aParams[6], loMove);
			_RetValue2l(1);
		}
		return;
	}

	fglBind::call(poEvent);
}



////////////////////////////////////////////////////////////////////////////////////////////////
