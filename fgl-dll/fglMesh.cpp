#include "fglMesh.h"
#include "fglModel.h"
#include "fglScene.h"
#include "fglImport.h"

void fglMesh::call(fglEvent& poEvent) 
{
	if (poEvent.name == fglEVENT_transformByBones) { transformByBones(); return; }

	if (poEvent.name == fglEVENT_loadDataFromString) { if (poEvent.nParams >= 2) loadDataFromString(_GetValue2c(poEvent.aParams[0]), _GetValue2c(poEvent.aParams[1])); return; }

	fglVisual::call(poEvent);
}


fglMesh::fglMesh(void)
{
	_nFaces = 0;
	_nIndsPerFace = 0;
	_nFaceMode = 0;
	renderImpl = (void (*)(const fglVisual*))fglMesh::renderMesh;
}




void fglMesh::setFaces(int pnFaces, int pnIndsPerFace)
{
	_nFaces = pnFaces;
	_nIndsPerFace = pnIndsPerFace;
	aInds.set_size(_nFaces * _nIndsPerFace);
	switch(_nIndsPerFace) 
	{
		case 1: _nFaceMode = GL_POINTS; break;
		case 2: _nFaceMode = GL_LINES; break;
		case 3: _nFaceMode = GL_TRIANGLES; break;
		default: _nFaceMode = 0;
	}
	if ( ! _nFaceMode ) 
	{
		throw fglException("Wrong primitive type. Only points, lines and triangles are allowed");
	}
}

fglFACE fglMesh::aFaces(int i) const
{ 
	return fglFACE(_nIndsPerFace, &aInds[i*_nIndsPerFace]);
}


bool fglMesh::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglMesh* loMesh = dynamic_cast<const fglMesh*>(poSrc);
	if (!loMesh) return false;

	if ( !fglVisual::copy(poSrc, pnShare) ) return false;
	
	_nFaces = loMesh->_nFaces;
	_nIndsPerFace = loMesh->_nIndsPerFace;
	_nFaceMode = loMesh->_nFaceMode;

	oVBO.inst(NULL); 
	oVBOInds.inst(NULL); 
	oVBOBones.inst(NULL);
	aVerts.inst(NULL); 
	aVertBones.inst(NULL); 
	aInds.inst(NULL); 
	aBones.inst(NULL); 
	
	if ( pnShare.check(fglSHARE_MESH) )
	{
		aVerts.inst(&loMesh->aVerts);
		aVertBones.inst(&loMesh->aVertBones);
		aInds.inst(&loMesh->aInds); 
		oVBO.inst(&loMesh->oVBO);
		oVBOInds.inst(&loMesh->oVBOInds);
		oVBOBones.inst(&loMesh->oVBOBones);
	} 
	else
	{
		aVerts.copy(&loMesh->aVerts);
		aVertBones.copy(&loMesh->aVertBones);
		aInds.copy(&loMesh->aInds);
		buildVBO();
	}
	
	aBones.copy(&loMesh->aBones);
	
	return true;
}


void fglMesh::buildVBO(unsigned pnUsage) 
{
	if ( aVerts.empty() || aInds.empty() ) return;
	
	oVBO.create();
	oVBO->setScene(getScene());
	oVBO->setData(aVerts.size()*sizeof(fglVERTEX), aVerts.map(), pnUsage==fglMESH_STATIC ? fglVBO_STATIC : fglVBO_DYNAMIC);
	
	oVBOInds.create();
	oVBOInds->setScene(getScene());
	oVBOInds->setData(aInds.size()*sizeof(fglINDEX), aInds.map(), fglVBO_STATIC);

	if ( aVertBones.exist() )
	{
		oVBOBones.create();
		oVBOBones->setScene(getScene());
		oVBOBones->setData(aVertBones.size()*sizeof(fglVERTEXBONES), aVertBones.map(), fglVBO_STATIC);
	}

}

void fglMesh::fillVBO(void) 
{
	if ( oVBO.empty() || oVBOInds.empty() ) 
	{
		buildVBO();
		return;
	}
	
	unsigned lnVertsSize = aVerts.size()*sizeof(fglVERTEX);
	unsigned lnIndsSize = aInds.size()*sizeof(fglINDEX);
	
	oVBO->setData(lnVertsSize, aVerts.map(), oVBO->getUsage());
	oVBOInds->setData(lnIndsSize, aInds.map(), oVBOInds->getUsage());

	if ( oVBOBones.exist() ) 
	{
		oVBOBones->setData(aVertBones.size()*sizeof(fglVERTEXBONES), aVertBones.map(), oVBOBones->getUsage());
	}

}


void fglMesh::calcBounds(void)
{
	fglVERTEX* v = aVerts.map();
	int lnVerts = aVerts.size();
	if ( bboxInt.isNull() && lnVerts ) 
	{
		bboxInt.min.x=bboxInt.min.y=bboxInt.min.z=fglINFINITE;
		bboxInt.max.x=bboxInt.max.y=bboxInt.max.z=-fglINFINITE;
		while (lnVerts--)
		{
			bboxInt.correct(v->p);
			v++;
		}
	}
	fglVisual::calcBounds();
}

void fglMesh::renderMesh(const fglMesh* poMesh) 
{
	fglVERTEX* loVert = poMesh->aVerts.map();
	fglVERTEXBONES* loVertBones = poMesh->aVertBones.map();

	if ( ! loVert || ! fglScene::oRenderMaterial ) return;

	fglViewport::applyLights();
	
	unsigned long lpVert = (unsigned long)loVert;
	unsigned long lpVertN = (unsigned long)&(loVert->n);
	unsigned long lpVertUV = (unsigned long)&(loVert->u);
	unsigned long lpInds = (unsigned long)poMesh->aInds.map(); 
	if ( poMesh->oVBO.exist() && poMesh->oVBOInds.exist() )
	{
		if ( poMesh->oVBO->bind() && poMesh->oVBOInds->bind() )
		{
			unsigned long lnVBOOffset = poMesh->oVBO->getOffset();
			lpVertUV -= lpVert - lnVBOOffset; 
			lpVertN -= lpVert - lnVBOOffset;  
			lpVert = lnVBOOffset;
			lpInds = poMesh->oVBOInds->getOffset();
		}
		else
		{
			fglVBO::unbind(fglVBO_VERTICES);
			fglVBO::unbind(fglVBO_INDICES);
		}
	}
	else
	{
		fglVBO::unbind(fglVBO_VERTICES);
		fglVBO::unbind(fglVBO_INDICES);
	}

	if (fglMaterial::oRenderShaderProg)
	{
		glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(fglVERTEX), (void*)lpVert);
		glEnableVertexAttribArray(0);

		if (fglScene::oRenderMaterial->renderLighting == 1) 
		{
			glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, sizeof(fglVERTEX), (void*)lpVertN);
			glEnableVertexAttribArray(1);
		}
		if (fglMaterial::oRenderTexture)
		{
			glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, sizeof(fglVERTEX), (void*)lpVertUV);
			glEnableVertexAttribArray(2);
		}
	} 
	else 
	{
		glVertexPointer(3, GL_FLOAT, sizeof(fglVERTEX), (void*)lpVert);
		glEnableClientState(GL_VERTEX_ARRAY);
		if (fglScene::oRenderMaterial->renderLighting == 1) 
		{
			glNormalPointer(GL_FLOAT, sizeof(fglVERTEX), (void*)lpVertN);
			glEnableClientState(GL_NORMAL_ARRAY); 
		}
		if (fglMaterial::oRenderTexture)
		{
			glTexCoordPointer(2, GL_FLOAT, sizeof(fglVERTEX), (void*)lpVertUV);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
		}
		
	}

	glDrawElements(poMesh->_nFaceMode, poMesh->aInds.size(), GL_UNSIGNED_SHORT, (void*)lpInds);

	if (fglMaterial::oRenderShaderProg)
	{
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	} 
	else 
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		
	}
}

void fglMesh::renderNormals(void) const
{
/*	glPushAttrib(GL_LINE_BIT | GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glLineWidth(1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);

	fglVECTOR* loV;
	fglVECTOR loN;
	unsigned lnVerts = aVerts.size();
	while (lnVerts--)
	{
		loV = &(aVerts[lnVerts].p);
		loN = (*loV) + aVerts[lnVerts].n;
		glBegin(GL_LINES);
		glVertex3fv(&loV->x);
		glVertex3fv(&loN.x);
		glEnd();
	}
	
	glPopAttrib();
*/
}

void fglMesh::transformByBones(void)
{
	if ( oVBO.empty() || aBones.empty() ) return;
	if ( aVerts.size() != aVertBones.size() ) return;
	
	const fglBone* loBone;
	fglVERTEX* loSrcVert;
	fglVERTEX* loDstVert;
	fglVERTEX* loAnimVert;
	float lnWeight;
	fglTRANSFORM loTrans;
	
	loAnimVert = (fglVERTEX*) oVBO->map();
	if ( ! loAnimVert ) 
	{
		oVBO->unmap();
		return;
	}
	memset(loAnimVert, 0, sizeof(fglVERTEX)*aVerts.size());

	for (unsigned i=0; i<aBones.size(); i++)
	{
		loBone = aBones[i].callBindREFMap();
		
		if (loBone->getParent().empty()) 
			{ loTrans.loadIdentity(); }
		else 
			{ loTrans = loBone->trg;  }
		
		for (unsigned j=0; j<loBone->aVerts.size(); j++)
		{
			loSrcVert = &aVerts[loBone->aVerts[j].index];
			loDstVert = &loAnimVert[loBone->aVerts[j].index];
			lnWeight = loBone->aVerts[j].weight;
			
			loDstVert->p = loDstVert->p + (loTrans*loSrcVert->p)*lnWeight;
			//loDstVert->n = loDstVert->n + (loTrans*loSrcVert->n)*lnWeight;
			loDstVert->n = loSrcVert->n;
			loDstVert->u = loSrcVert->u;
			loDstVert->v = loSrcVert->v;

		}
	}

	oVBO->unmap();
	setSceneRenderFlag();
}


bool fglMesh::testIntersection(fglRAY& poRay, int* pnFaceId, int* pnVertId, fglVECTOR* poPointIn) const
{
	if ( ! fglVisual::testIntersection(poRay) ) return false;
		
	if ( !pnFaceId || !pnVertId || !poPointIn ) return true;

	int lnFaceId=-1, lnVertId=-1;
	float lnDist, lnMinDist=fglINFINITE;
	
	fglVERTEX* laVerts = NULL;
	if (oVBO.exist()) { laVerts = (fglVERTEX*) oVBO->map(); }
	if ( ! laVerts ) laVerts = aVerts.map();
	
	int j=0;
	for (unsigned i = 0; i < _nFaces; i++)
	{
		j = poRay.testInFace(aFaces(i), laVerts, poPointIn);
		if ( j==-1 ) continue;
		lnDist = ((*poPointIn)-poRay.start).norm();
		if (lnMinDist > lnDist) { lnMinDist = lnDist; lnFaceId=i; lnVertId=j;}
	}
	
	if (oVBO.exist()) { oVBO->unmap(); }

	if ( lnFaceId==-1 || lnVertId==-1 ) return false;
	
	if (pnFaceId) *pnFaceId = lnFaceId;
	if (pnVertId) *pnVertId = lnVertId;

	return true;
}


void fglMesh::loadDataFromString(const fglString& pcData, const fglString& pcFormat)
{
	fgl_import_mesh_from_string(*this, pcData, pcFormat);
	buildVBO();
	calcBounds();
}
