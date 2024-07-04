#ifndef _FGL3DPRIMITIVES_H_
#define _FGL3DPRIMITIVES_H_

#include "fglMesh.h"

class fgl3DPrimitives
{
private:
	bool isBuilded{ false };
	fglWeakREF<fglScene> oScene;
	fglMAT4 mSaveTrans;

	fglBindREF<fglMesh> oTriangle;
	fglBindREF<fglMesh> oQuadXZLines;
	fglBindREF<fglMesh> oQuadXZSolid;
	fglBindREF<fglMesh> oQuadXYLines;
	fglBindREF<fglMesh> oQuadXYSolid;
	fglBindREF<fglMesh> oQuadYZLines;
	fglBindREF<fglMesh> oQuadYZSolid;
	fglBindREF<fglMesh> oCubeLines;
	fglBindREF<fglMesh> oCubeSolid;
	fglBindREF<fglMesh> oSphere0;
	fglBindREF<fglMesh> oSphere1;
	fglBindREF<fglMesh> oSphere2;
	fglBindREF<fglMesh> oSphere3;
	fglBindREF<fglMesh> oSphere4;

	void renderBegin(const fglMaterial* poMaterial) const;
	void renderEnd(const fglMaterial* poMaterial) const;

public:
	
	fglBindREF<fglMaterial> oMaterial;

	fgl3DPrimitives(void) = default;
	//bad for perf virtual ~fgl3DPrimitives(void);

	void renderTriangle(const fglVECTOR& poVert1, const fglVECTOR& poVert2, const fglVECTOR& poVert3, const fglMaterial* poMaterial=NULL);
	void renderQuad(const fglVECTOR& poCenter, const fglVECTOR& poSize, unsigned pnPlane, unsigned pnMode=1, const fglMaterial* poMaterial=NULL);
	void renderCube(const fglVECTOR& poCenter, const fglVECTOR& poSize, unsigned pnMode=1, const fglMaterial* poMaterial=NULL);
	void renderSphere(const fglVECTOR& poCenter, const fglVECTOR& poSize, unsigned pnTess=2, const fglMaterial* poMaterial=NULL);

	void build(const fglScene& poScene);

};

#endif
