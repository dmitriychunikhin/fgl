#include "fglModel.h"
#include "fglScene.h"
#include "fglPM.h"
#include "fglImport.h"

void fglModel::call(fglEvent& poEvent) 
{

	if (poEvent.name == fglEVENT_loadData) { if (poEvent.nParams) loadData(_GetValue2c(poEvent.aParams[0])); return;}

	if (poEvent.name == fglEVENT_aAnims) { _RetValue2i(aAnims->getHFGL()); return; }

	if (poEvent.name == fglEVENT_hRootJoint) 
	{
		if (oRootJoint.empty())
		{
			fglREF<fglJoint> loRootJoint = fglBindFactory::createNonPersistent<fglJoint>();
			loRootJoint->setScene(getScene());
			setRootJoint(loRootJoint);
		}
		_RetValue2i(oRootJoint->getHFGL());
		return;
	}

	if (poEvent.name == fglEVENT_findJoint) 
	{ 
		_RetValue2i(0);
		if (!poEvent.nParams) return;
		const fglREF<fglJoint>* loJoint = findJoint(_GetValue2c(poEvent.aParams[0]));
		if (loJoint) _RetValue2i( (*loJoint)->getHFGL() ); 
		return;
	}

	if (poEvent.name == fglEVENT_getVisuals)
	{
		if (oRootJoint.empty()) { _RetValue2c(""); return; }

		fglArray<fglVisual*> laVisuals;

		fglString lcVisuals;
		
		oRootJoint->getVisuals( _GetValue2c(poEvent.aParams[0]), _GetValue2c(poEvent.aParams[1]), laVisuals );

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
		if (oRootJoint.empty()) { _RetValue2c(""); return; }

		fglArray<fglMaterial*> laMaterials;

		fglString lcMaterials;
		
		oRootJoint->getMaterials( _GetValue2c(poEvent.aParams[0]), _GetValue2c(poEvent.aParams[1]), laMaterials );

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

	if (poEvent.name == fglEVENT_buildLOD) 
	{ 
		if (poEvent.nParams==3) 
			buildLOD(_GetValue2f(poEvent.aParams[0]), _GetValue2f(poEvent.aParams[1]), _GetValue2i(poEvent.aParams[2])); 
		else 
			buildLOD(); 
		return;
	}

	if (poEvent.name == fglEVENT_bindSkel) { bindSkel(); return;}
	if (poEvent.name == fglEVENT_transformByBones) { transformByBones(); return; }

	fglVisual::call(poEvent);
}

fglModel::fglModel(void)
{
	renderImpl = (void(*)(const fglVisual*))fglModel::renderModel;
}

void fglModel::setRootJoint(const fglREF<fglJoint>& poRootJoint)
{
	if (oRootJoint != poRootJoint) oRootJoint = poRootJoint;
	bindSkel();
}

const fglREF<fglJoint>& fglModel::getRootJoint(void) const
{
	return oRootJoint;
}

const fglREF<fglJoint>* fglModel::findJoint(const fglString& pcName) const 
{ 
	if (oRootJoint.empty()) return NULL;
	return oRootJoint->find<fglJoint>(pcName);
}

void fglModel::loadData(fglString& pcPath) 
{
	oRootJoint.clear();
	aAnims->clear();
	if (!pcPath.empty()) fgl_import_model(this, pcPath);


	class meshVBO : public fglWalkNodes
	{
	public:
		static void onMesh(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual)
		{
			fglMesh* loMesh;
			if ( ! (loMesh = dynamic_cast<fglMesh*>(poVisual)) ) return;
			loMesh->buildVBO();
		}
		meshVBO(void) { onVisual = onMesh; }
	} loMeshVBO;

	fglWalkNodes::walkNodes(oRootJoint.map(), &loMeshVBO);
	
	calcBounds();
}

bool fglModel::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglModel* loModel = dynamic_cast<const fglModel*>(poSrc);
	if (!loModel) return false;

	if ( !fglVisual::copy(poSrc, pnShare) ) return false;

	copySkel(loModel, pnShare);
	copyAnim(loModel, pnShare);
	
	return true;
}

void fglModel::copySkel(const fglModel* poModel, fglBITFLAG pnShare)
{
	if (!poModel || poModel==this) return;
	
	oRootJoint.clear();
	if (poModel->oRootJoint.empty()) return;

	fglREF<fglJoint> loRootJoint = fglBindFactory::duplicateNonPersistent<fglJoint>(poModel->oRootJoint, (pnShare.check(fglSHARE_SKEL) ? fglDUPLICATE_INSTANCE : fglDUPLICATE_COPY), pnShare);
	if ( loRootJoint.exist() ) setRootJoint( loRootJoint );
}

void fglModel::copyAnim(const fglModel* poModel, fglBITFLAG pnShare)
{
	if (!poModel || poModel==this) return;
	
	aAnims->copy(poModel->aAnims.callBindREFMap(), pnShare.check(fglSHARE_ANIM));
}


void fglModel::buildLOD(float pnViewRange, float pnStep, unsigned pnSteps) 
{
	if (pnStep==0.0f) return;
	return;

	class meshCount : public fglWalkNodes
	{
	public:
		unsigned nMeshes;
		unsigned nFaces;
		static void onMesh(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual)
		{
			meshCount* loHandler = static_cast<meshCount*>(poHandler);
			fglMesh* loMesh;
			if ( ! (loMesh = dynamic_cast<fglMesh*>(poVisual)) ) return;
			loHandler->nMeshes++;
			loHandler->nFaces += loMesh->nFaces();
		}
		meshCount(void) { onVisual=onMesh; }
	} loMeshCount;
	loMeshCount.nMeshes = 0;
	loMeshCount.nFaces = 0;
	
	fglWalkNodes::walkNodes(oRootJoint.map(), &loMeshCount);

	class meshBuild : public fglWalkNodes
	{
	public:
		float nStep;
		unsigned nSteps;
		unsigned nMinFaces;
		static void onMesh(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual)
		{
			meshBuild* loHandler = static_cast<meshBuild*>(poHandler);
			fglMesh* loMesh;
			if ( ! (loMesh = dynamic_cast<fglMesh*>(poVisual)) ) return;
			fglPM loPM;
			loPM.Build(loMesh, loMesh, std::max(unsigned(loMesh->nFaces()/(powf(loHandler->nStep,float(loHandler->nSteps)))),loHandler->nMinFaces));
			if (loMesh->oVBO.exist()) loMesh->buildVBO();
		}
		meshBuild(void) { onVisual=onMesh; }
	} loMeshBuild;
	
	loMeshBuild.nMinFaces = 16;
	loMeshBuild.nStep = pnStep;
		
	unsigned lnFaces = unsigned(loMeshCount.nFaces / loMeshBuild.nStep);
	unsigned lnSteps = pnSteps ? pnSteps : 
		std::max(int(logf(float(lnFaces))/logf(float(loMeshBuild.nStep)) 
		- logf(float(loMeshCount.nMeshes*loMeshBuild.nMinFaces))/logf(float(loMeshBuild.nStep))),1);
	

	aLODs->clear();
	//aLODs->reserve(loMeshBuild.nSteps);

	fglREF<fglModel> loLOD;
	fglBindREF<fglCollection<fglModel>> laLODs;
	laLODs->setScene(getScene());
	
	unsigned liStep=0;
	while (liStep < lnSteps)
	{
		liStep++;
		loMeshBuild.nSteps = liStep;
		loLOD = fglBindFactory::createNonPersistent<fglModel>();
		loLOD->copy(this, fglSHARE_ANIM);
		loLOD->bboxExt.setNull();
		loLOD->viewRangeMin = 0.0f;
		loLOD->viewRangeMax = fglINFINITE;
		loLOD->viewRangeRot.setNull();
		laLODs->add(loLOD);
		fglWalkNodes::walkNodes(loLOD->oRootJoint.map(), &loMeshBuild);
		loLOD->viewRangeMax = (lnSteps-liStep+1) * pnViewRange;
		loLOD->bindSkel();
	}
	aLODs->copy(laLODs.callBindREFMap());
	laLODs->clear();
	setSceneRenderFlag();
}

void fglModel::calcBounds(void)
{
	if (oRootJoint.exist())
	{
		oRootJoint->calcBounds();
		bboxInt.min = oRootJoint->bbox.getMinGlobal();
		bboxInt.max = oRootJoint->bbox.getMaxGlobal();
	}
	else
	{
		bboxInt.setNull();
	}

	fglVisual::calcBounds();
}


void fglModel::renderModel(const fglModel* poModel) 
{
	fglWalkNodes::walkNodes(poModel->oRootJoint.map(), &fglScene::oRenderNodes);
}

void fglModel::bindSkel(void)
{
	class meshBind : public fglWalkNodes
	{
	public:
		fglREF<fglJoint> oRootJoint;

		static void onMesh(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual)
		{
			meshBind* loHandler = static_cast<meshBind*>(poHandler);
			fglMesh* loMeshFirst;
			fglMesh* loMesh;
			if ( ! (loMeshFirst = dynamic_cast<fglMesh*>(poVisual)) ) return;
			
			unsigned lnLODs = loMeshFirst->aLODs->size() + 1;
			unsigned liMesh=0, i;
			
			fglBone* loBone;
			unsigned lnBones;
			const fglREF<fglJoint>* loJoint;
			
			while (lnLODs--)
			{
				if (liMesh == 0) loMesh = loMeshFirst; else loMesh = dynamic_cast<fglMesh*>(loMeshFirst->aLODs->get(liMesh - 1).map());
				if ( ! loMesh) continue;
				
				lnBones = loMesh->aBones.size();
				for (i = 0; i < lnBones; i++)
				{
					loBone = const_cast<fglBone*>(loMesh->aBones[i].callBindREFMap());
					loBone->setParent(fglREF<fglNode>());
					if (loHandler->oRootJoint.empty()) continue;
					loJoint = loHandler->oRootJoint->find<fglJoint>(loBone->name);
					if (loJoint) loBone->setParent((*loJoint)->getRef<fglNode>());
				}
				
				liMesh++;
			}
		}
		meshBind(void) { onVisual=onMesh; }
	} loMeshBind;

	loMeshBind.oRootJoint = oRootJoint;
	fglWalkNodes::walkNodes(oRootJoint.map(), &loMeshBind);

	if (oRootJoint.exist()) oRootJoint->buildTransform();
	calcBounds();

}


void fglModel::buildAnim(void)
{
	if ( getScene().empty() ) return;

	//oAnim->buildTransform(fglScene::nCurrTime);

	//transformByBones();
}

void fglModel::transformByBones(void)
{
	class meshTransformByBones : public fglWalkNodes
	{
	public:
		static void onMesh(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual)
		{
			fglMesh* loMesh;
			if ( ! (loMesh = dynamic_cast<fglMesh*>(poVisual)) ) return;
			loMesh->transformByBones();
		}
		meshTransformByBones(void) { onVisual=onMesh; }
	} loMeshTransformByBones;

	if (oRootJoint.exist()) oRootJoint->buildTransform();

	fglWalkNodes::walkNodes(oRootJoint.map(), &loMeshTransformByBones);
	setSceneRenderFlag();
}

