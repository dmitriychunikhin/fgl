

#include "fglImport.h"
#include "fglScene.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma comment (lib,"assimp.lib") 


void fgl_import_model(fglModel* poModel, const fglString& pcPath)
{
	std::ifstream loFile; 
	loFile.open(pcPath); 
	if ( ! loFile.is_open() ) return; 
	loFile.close();
	
	Assimp::Importer loImporter;

	loImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, fglScene::oGLHelper.getMaxElementsIndices());
	loImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, fglScene::oGLHelper.getMaxElementsIndices()/3); 
	loImporter.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, fglMAX_BONES); 

	
	const aiScene* loAIScene = NULL;
	
	loAIScene = loImporter.ReadFile(pcPath, 
		aiProcess_CalcTangentSpace			| // calculate tangents and bitangents if possible
		aiProcess_JoinIdenticalVertices		| // join identical vertices/ optimize indexing
		aiProcess_ValidateDataStructure		| // perform a full validation of the loader's output
		aiProcess_ImproveCacheLocality		| // improve the cache locality of the output vertices
		aiProcess_RemoveRedundantMaterials	| // remove redundant materials
		aiProcess_FindDegenerates			| // remove degenerated polygons from the import
		aiProcess_GenSmoothNormals			| // generate smooth normal vectors if not existing
		aiProcess_GenUVCoords				| // convert spherical, cylindrical, box and planar mapping to proper UVs
		aiProcess_TransformUVCoords			| // preprocess UV transformations (scaling, translation ...)
		aiProcess_FindInstances				| // search for instanced meshes and remove them by references to one master
		aiProcess_LimitBoneWeights			| // limit bone weights to 4 per vertex
		aiProcess_OptimizeMeshes			| // join small meshes, if possible;
		aiProcess_SplitLargeMeshes			| // split large, unrenderable meshes into submeshes
		aiProcess_Triangulate				| // triangulate polygons with more than 3 edges
		aiProcess_SortByPType				| // make 'clean' meshes which consist of a single typ of primitives
		aiProcess_OptimizeGraph				| // Nodes with no animations, bones, lights or cameras assigned are collapsed and joined.
		0);

	if ( ! loAIScene) 
	{
		throw fglException(loImporter.GetErrorString());
		return;
	}

	aiMaterial* loAIMaterial;
	aiMesh* loAIMesh;
	aiFace* loAIFace;
	aiBone* loAIBone;
	aiVector3D* loAIVert;
	aiVector3D* loAINorm;
	aiVector3D* loAITexCoord;
	aiColor3D loAIColor;
	aiString lcAIStr;
	
	
	fglMaterial* loMaterial;
	fglFACE loFace;
	fglBone* loBone;
	fglVERTEX* loVert;
	fglString lcName;

	fglArray<fglREF<fglMesh>> laMeshes;

	for (unsigned m=0; m < loAIScene->mNumMeshes; m++)
	{
		loAIMesh = loAIScene->mMeshes[m];
		fglREF<fglMesh> loMesh;
		loMesh = fglBindFactory::createNonPersistent<fglMesh>();
		loMesh->setScene(poModel->getScene());
		laMeshes.push_back(loMesh);
		
		loMesh->aVerts.set_size(loAIMesh->mNumVertices);
		if ( loAIMesh->mNumBones ) loMesh->aVertBones.set_size(loAIMesh->mNumVertices);
		for (unsigned v=0; v < loAIMesh->mNumVertices; v++)
		{
			loAIVert = &loAIMesh->mVertices[v];
			loVert = &loMesh->aVerts[v];

			loVert->p.x = loAIVert->x;
			loVert->p.y = loAIVert->y;
			loVert->p.z = loAIVert->z;

			if (loAIMesh->HasNormals()) 
			{
				loAINorm = &loAIMesh->mNormals[v];
				loVert->n.x = loAINorm->x;
				loVert->n.y = loAINorm->y;
				loVert->n.z = loAINorm->z;
			}

			if (loAIMesh->GetNumUVChannels() > 0)
			{
				loAITexCoord = &loAIMesh->mTextureCoords[0][v];
				loVert->u = loAITexCoord->x;
				loVert->v = loAITexCoord->y;
			}

		}

		if (loAIMesh->mNumFaces)
		{
			loMesh->setFaces(loAIMesh->mNumFaces, loAIMesh->mFaces[0].mNumIndices);
		}

		for (unsigned f = 0; f < loAIMesh->mNumFaces; f++)
		{
			loAIFace = &loAIMesh->mFaces[f];
			loFace = loMesh->aFaces(f);

			if (loAIFace->mNumIndices != loFace.nInds) continue;

			for (unsigned i = 0; i < loAIFace->mNumIndices; i++)
			{
				loFace.aInds[i] = loAIFace->mIndices[i];
			}
		}

		loMesh->aBones.set_size(loAIMesh->mNumBones);
		for (unsigned b = 0; b < loAIMesh->mNumBones; b++)
		{
			loAIBone = loAIMesh->mBones[b];
			loBone = const_cast<fglBone*>(loMesh->aBones[b].callBindREFMap());
			loBone->name = loAIBone->mName.data;
			loBone->setScene(poModel->getScene());

			aiVector3D loAIPos, loAIScale;
			aiQuaternion loAIRot;
			//memcpy(&(loBone->mTrans), &(loAIBone->mOffsetMatrix.Transpose()), sizeof(loBone->mTrans));
			loAIBone->mOffsetMatrix.Decompose(loAIScale, loAIRot, loAIPos);
			//loAIBone->mOffsetMatrix.DecomposeNoScaling(loAIRot, loAIPos);
			loBone->tr.pos.x = loAIPos.x; loBone->tr.pos.y = loAIPos.y; loBone->tr.pos.z = loAIPos.z;
			loBone->tr.rot.a = loAIRot.w; loBone->tr.rot.v.x = loAIRot.x; loBone->tr.rot.v.y = loAIRot.y; loBone->tr.rot.v.z = loAIRot.z;
			loBone->tr.scale.x = loAIScale.x; loBone->tr.scale.y = loAIScale.y; loBone->tr.scale.z = loAIScale.z;

			loBone->aVerts.set_size(loAIBone->mNumWeights);
			for(unsigned i=0; i < loAIBone->mNumWeights; i++) 
			{
				loBone->aVerts[i].index = loAIBone->mWeights[i].mVertexId;
				loBone->aVerts[i].weight = loAIBone->mWeights[i].mWeight;
				int lnVertId = loBone->aVerts[i].index;
				for (unsigned j=0; j<fglMAX_BONES; j++) 
				{
					if ( loMesh->aVertBones[lnVertId].aBoneWeights[j] != 0.0f) continue;
					loMesh->aVertBones[lnVertId].aBoneInds[j] = float(b);
					loMesh->aVertBones[lnVertId].aBoneWeights[j] = loBone->aVerts[i].weight;

				}

			}

		}

		if (loAIScene->HasMaterials())
		{
			for (unsigned i = 0; i < m; i++)
				if (loAIScene->mMeshes[i]->mMaterialIndex == loAIMesh->mMaterialIndex) 
					{ loMesh->oMaterial.inst(&laMeshes[i]->oMaterial); break; }
		}

		if (loAIScene->HasMaterials() && loMesh->oMaterial.empty())
		{
			loAIMaterial = loAIScene->mMaterials[loAIMesh->mMaterialIndex];
			loMesh->oMaterial = fglBindFactory::createNonPersistent<fglMaterial>();
			loMaterial = loMesh->oMaterial.map();
			loMaterial->setScene(poModel->getScene());
			if ( loAIMaterial->Get(AI_MATKEY_NAME, lcAIStr) == AI_SUCCESS ) loMaterial->name.append(lcAIStr.data, lcAIStr.length);
			if ( loAIMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, loAIColor) == AI_SUCCESS ) loMaterial->color_diffuse.set(loAIColor.r, loAIColor.g, loAIColor.b, 1.0f);
			if ( loAIMaterial->Get(AI_MATKEY_COLOR_SPECULAR, loAIColor) == AI_SUCCESS ) loMaterial->color_specular.set(loAIColor.r, loAIColor.g, loAIColor.b, 1.0f);
			if ( loAIMaterial->Get(AI_MATKEY_COLOR_AMBIENT, loAIColor) == AI_SUCCESS ) loMaterial->color_ambient.set(loAIColor.r, loAIColor.g, loAIColor.b, 1.0f);
			if ( loAIMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, loAIColor) == AI_SUCCESS ) loMaterial->color_emission.set(loAIColor.r, loAIColor.g, loAIColor.b, 1.0f);
			if ( loAIMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, loAIColor) == AI_SUCCESS ) loMaterial->color_transparent.set(loAIColor.r, loAIColor.g, loAIColor.b, 1.0f);

			loAIMaterial->Get(AI_MATKEY_OPACITY, loMaterial->opacity);
			loAIMaterial->Get(AI_MATKEY_SHININESS, loMaterial->shininess);
			loAIMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, loMaterial->shininess_strength);
			loAIMaterial->Get(AI_MATKEY_REFRACTI, loMaterial->refracti);
			
			if ( loAIMaterial->Get(AI_MATKEY_SHADING_MODEL, loMaterial->shading) != AI_SUCCESS ) loMaterial->shading = -1;
			if ( loAIMaterial->Get(AI_MATKEY_TWOSIDED, loMaterial->twosided) != AI_SUCCESS ) loMaterial->twosided = -1;
			if ( loAIMaterial->Get(AI_MATKEY_ENABLE_WIREFRAME, loMaterial->wireframe) != AI_SUCCESS) loMaterial->wireframe = -1;

			if ( loAIMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0 )
			{
				loAIMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &lcAIStr, NULL, NULL, NULL, NULL, NULL);
				loMaterial->oTexture = fglBindFactory::createNonPersistent<fglTexture>();
				loMaterial->oTexture->path.append(lcAIStr.data, lcAIStr.length);
				fglString lcTexturePath = pcPath.substr(0,pcPath.find_last_of("/\\")+1) + loMaterial->oTexture->path;
				std::ifstream loFile; loFile.open(lcTexturePath); if ( loFile.is_open() ) { loFile.close(); loMaterial->oTexture->path=lcTexturePath; }

				loMaterial->oTexture->build();
			} 
			if ( loAIMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0 && loMaterial->oTexture.empty())
			{
				loAIMaterial->GetTexture(aiTextureType_AMBIENT, 0, &lcAIStr, NULL, NULL, NULL, NULL, NULL);
				loMaterial->oTexture = fglBindFactory::createNonPersistent<fglTexture>();
				loMaterial->oTexture->path.append(lcAIStr.data, lcAIStr.length);
				fglString lcTexturePath = pcPath.substr(0,pcPath.find_last_of("/\\")+1) + loMaterial->oTexture->path;
				std::ifstream loFile; loFile.open(lcTexturePath); if ( loFile.is_open() ) { loFile.close(); loMaterial->oTexture->path=lcTexturePath; }

				loMaterial->oTexture->build();
			}
		}
	}

	
	////////////////////////////////////////////////////////////
	//Import joints
	aiNode* loAINode;
	fglREF<fglJoint> loJoint;
	fglREF<fglJoint> loNewJoint;
	unsigned iChild=0;
	fglArray<unsigned> laStack;

	//loJoint = NULL;
	loAINode = loAIScene->mRootNode;
	while (loAINode)
	{
		if (iChild==0) 
		{
			lcName = loAINode->mName.data;
			loNewJoint = fglBindFactory::createNonPersistent<fglJoint>();
			loNewJoint->name = lcName;
			loNewJoint->setScene(poModel->getScene());

			if (loJoint.exist()) loJoint->aChilds->add(loNewJoint);

			aiVector3D loAIPos, loAIScale;
			aiQuaternion loAIRot;
			loAINode->mTransformation.Decompose(loAIScale, loAIRot, loAIPos);
			//loAINode->mTransformation.DecomposeNoScaling(loAIRot, loAIPos);
			//memcpy(&(loNewJoint->mTrans), &(loAINode->mTransformation.Transpose()), sizeof(loNewJoint->mTrans));
			loNewJoint->tr.pos.x = loAIPos.x; loNewJoint->tr.pos.y = loAIPos.y; loNewJoint->tr.pos.z = loAIPos.z;
			loNewJoint->tr.rot.a = loAIRot.w; loNewJoint->tr.rot.v.x = loAIRot.x; loNewJoint->tr.rot.v.y = loAIRot.y; loNewJoint->tr.rot.v.z = loAIRot.z;
			loNewJoint->tr.scale.x = loAIScale.x; loNewJoint->tr.scale.y = loAIScale.y; loNewJoint->tr.scale.z = loAIScale.z;

			for (unsigned m = 0; m < loAINode->mNumMeshes; m++)
			{
				loNewJoint->aVisuals->add(laMeshes[loAINode->mMeshes[m]]);
			}

			if (loJoint.empty()) { poModel->setRootJoint(loNewJoint); }

			loJoint = loNewJoint;
		}

		if (iChild >= loAINode->mNumChildren) 
		{
			loJoint = loJoint->getParent();
			loAINode = loAINode->mParent;
			if (loAINode == NULL) break;

			iChild=laStack.back()+1;
			laStack.pop_back();
			continue;
		}
		laStack.push_back(iChild);
		loAINode = loAINode->mChildren[iChild];
		iChild=0;
	}
	poModel->bindSkel();
	
	////////////////////////////////////////////////////////////

	aiAnimation* loAIAnim;
	aiNodeAnim* loAIAnimNode;
	aiVectorKey* loAIVectKey;
	aiQuatKey* loAIQuatKey;

	fglREF<fglAnim> loAnim;
	fglAnimChan* loAnimChan;
	fglAnimVectKey* loAnimVectKey;
	fglAnimQuatKey* loAnimQuatKey;

	for (unsigned a = 0; a < loAIScene->mNumAnimations; a++)
	{
		loAIAnim = loAIScene->mAnimations[a];
		lcName = loAIAnim->mName.data;
		loAnim.create();
		poModel->aAnims->add(loAnim);
		loAnim->name = lcName;
		loAnim->nFrames = (float)loAIAnim->mDuration;
		loAnim->nFPS = (int)loAIAnim->mTicksPerSecond;

		for (unsigned i = 0; i < loAIAnim->mNumChannels; i++)
		{
			loAIAnimNode = loAIAnim->mChannels[i];
			lcName = loAIAnimNode->mNodeName.data;
			loAnim->aChans[lcName].create();
			loAnimChan = loAnim->aChans[lcName].map();
			loAnimChan->jointname = lcName;

			for (unsigned j = 0; j < loAIAnimNode->mNumPositionKeys; j++)
			{
				loAIVectKey = &loAIAnimNode->mPositionKeys[j];
				loAnimVectKey = new fglAnimVectKey();
				loAnimVectKey->nFrame = (float)loAIVectKey->mTime;
				loAnimVectKey->v.x = loAIVectKey->mValue.x;
				loAnimVectKey->v.y = loAIVectKey->mValue.y;
				loAnimVectKey->v.z = loAIVectKey->mValue.z;
				loAnimChan->aPosKeys.push_back(*loAnimVectKey);
			}

			for (unsigned j = 0; j < loAIAnimNode->mNumScalingKeys; j++)
			{
				loAIVectKey = &loAIAnimNode->mScalingKeys[j];
				loAnimVectKey = new fglAnimVectKey();
				loAnimVectKey->nFrame = (float)loAIVectKey->mTime;
				loAnimVectKey->v.x = loAIVectKey->mValue.x;
				loAnimVectKey->v.y = loAIVectKey->mValue.y;
				loAnimVectKey->v.z = loAIVectKey->mValue.z;
				loAnimChan->aScaleKeys.push_back(*loAnimVectKey);
			}

			for (unsigned j = 0; j < loAIAnimNode->mNumRotationKeys; j++)
			{
				loAIQuatKey = &loAIAnimNode->mRotationKeys[j];
				loAnimQuatKey = new fglAnimQuatKey();
				loAnimQuatKey->nFrame = (float)loAIQuatKey->mTime;
				loAnimQuatKey->q.a = loAIQuatKey->mValue.w;
				loAnimQuatKey->q.v.x = loAIQuatKey->mValue.x;
				loAnimQuatKey->q.v.y = loAIQuatKey->mValue.y;
				loAnimQuatKey->q.v.z = loAIQuatKey->mValue.z;
				loAnimChan->aRotKeys.push_back(*loAnimQuatKey);
			}
		}
	}
}

void fgl_import_mesh_from_string(fglMesh& poMesh, const fglString& pcData, const fglString& pcFormat)
{
	Assimp::Importer loImporter;

	loImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, fglScene::oGLHelper.getMaxElementsIndices());
	loImporter.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, fglScene::oGLHelper.getMaxElementsIndices()/3); 
	
	const aiScene* loAIScene = NULL;
	
	loAIScene = loImporter.ReadFileFromMemory(pcData.c_str(), pcData.size(), 
		aiProcess_CalcTangentSpace			| // calculate tangents and bitangents if possible
		aiProcess_JoinIdenticalVertices		| // join identical vertices/ optimize indexing
		aiProcess_ValidateDataStructure		| // perform a full validation of the loader's output
		aiProcess_ImproveCacheLocality		| // improve the cache locality of the output vertices
		aiProcess_RemoveRedundantMaterials	| // remove redundant materials
		aiProcess_FindDegenerates			| // remove degenerated polygons from the import
		aiProcess_GenSmoothNormals			| // generate smooth normal vectors if not existing
		aiProcess_GenUVCoords				| // convert spherical, cylindrical, box and planar mapping to proper UVs
		aiProcess_TransformUVCoords			| // preprocess UV transformations (scaling, translation ...)
		aiProcess_FindInstances				| // search for instanced meshes and remove them by references to one master
		aiProcess_LimitBoneWeights			| // limit bone weights to 4 per vertex
		aiProcess_OptimizeMeshes			| // join small meshes, if possible;
		aiProcess_SplitLargeMeshes			| // split large, unrenderable meshes into submeshes
		aiProcess_Triangulate				| // triangulate polygons with more than 3 edges
		aiProcess_SortByPType				| // make 'clean' meshes which consist of a single typ of primitives
		aiProcess_PreTransformVertices		| // Removes the node graph and pre-transforms all vertices with the local transformation matrices of their nodes
		0,
		pcFormat.c_str()
		);

	if ( ! loAIScene) 
	{
		throw fglException(loImporter.GetErrorString());
		return;
	}

	if (loAIScene->mNumMeshes < 1) return;

	aiMesh* loAIMesh;
	aiFace* loAIFace;
	aiVector3D* loAIVert;
	aiVector3D* loAINorm;
	aiVector3D* loAITexCoord;
	aiColor3D loAIColor;
	aiString lcAIStr;
		
	fglFACE loFace;
	fglVERTEX* loVert;
	fglString lcName;

	loAIMesh = loAIScene->mMeshes[0];
	poMesh.aVerts.set_size(loAIMesh->mNumVertices);
	for (unsigned v = 0; v < loAIMesh->mNumVertices; v++)
	{
		loAIVert = &loAIMesh->mVertices[v];
		loVert = &poMesh.aVerts[v];

		loVert->p.x = loAIVert->x;
		loVert->p.y = loAIVert->y;
		loVert->p.z = loAIVert->z;

		if (loAIMesh->HasNormals()) 
		{
			loAINorm = &loAIMesh->mNormals[v];
			loVert->n.x = loAINorm->x;
			loVert->n.y = loAINorm->y;
			loVert->n.z = loAINorm->z;
		}

		if (loAIMesh->GetNumUVChannels() > 0)
		{
			loAITexCoord = &loAIMesh->mTextureCoords[0][v];
			loVert->u = loAITexCoord->x;
			loVert->v = loAITexCoord->y;
		}

	}

	if (loAIMesh->mNumFaces)
	{
		poMesh.setFaces(loAIMesh->mNumFaces, loAIMesh->mFaces[0].mNumIndices);
	}

	for (unsigned f = 0; f < loAIMesh->mNumFaces; f++)
	{
		loAIFace = &loAIMesh->mFaces[f];
		loFace = poMesh.aFaces(f);

		if (loAIFace->mNumIndices != loFace.nInds) continue;
		
		for (unsigned i = 0; i < loAIFace->mNumIndices; i++)
		{
			loFace.aInds[i] = loAIFace->mIndices[i];
		}
	}
	
}
