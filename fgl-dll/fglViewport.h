#ifndef _FGLVIEWPORT_H_
#define _FGLVIEWPORT_H_

#include "common.h"
#include "fglSceneGraph.h"
#include "fglCam.h"
#include "fglModel.h"
#include "fglLight.h"



class fglViewport : public fglBind
{
	friend class fglBindFactory;

protected:
	fglViewport(void) = default;

public:

	fglRECT oRect;
	fglCOLOR oBackColor;
	fglREF<fglCam> oCam;
	fglMAT4 mProj;
	fglMAT4 mProjPerspective;
	fglMAT4 mProjOrtho;
	fglMAT4 mProjOrthoViewportCam;
	fglFRUSTUM oFrustum;

	enum { nProjPerspective = 1, nProjOrtho = 2, nProjOrthoViewportCam = 3 };
	unsigned nProj{ nProjPerspective };

	fglREF<fglMaterial> oMaterial; //default material
	
	fglREF<fglNode> oRenderNode;

	fglBindREF<fglCollection<fglLight>> aLights;
	
	//bad for perf virtual ~fglViewport(void);

	virtual fglString fgl_class(void) const { return "viewport"; }

	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);
	
	virtual void call(fglEvent& poEvent);
	
	fglVECTOR project(const fglVECTOR& poWorldPoint);
	fglVECTOR unproject(fglVECTOR poWindowPoint);

	void buildProjection(void);
	void setProjection(void);
	void render(void);
	
	class renderNodes : public fglWalkNodes
	{
	public:
		static void renderVisual(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual);
		static bool renderNode(fglWalkNodes* poHandler, fglNode* poNode);
		renderNodes(void) { onVisual=renderVisual; onNode=renderNode; }
	};
	static renderNodes oRenderNodes;

	static void selectLights(const fglCollection<fglLight>& paLights);
	static void applyLights(void);

	bool testPointInFrustum(const fglVECTOR& poPoint) const;

};


class fglPickInfo : public fglBind
{
	friend class fglBindFactory;

protected:
	fglPickInfo(void) { oPickNodes.oPickInfo = this; nMinDist = fglINFINITE; isPickNearest=true; nPickRadius=false; isPickDisabled=false; }

public:
	class pickNodes : public fglWalkNodes
	{
	public:
		fglPickInfo* oPickInfo;
		static bool testNode(fglWalkNodes* poHandler, fglNode* poNode);
		static void testVisual(fglWalkNodes* poHandler, fglNode* poNode, fglVisual* poVisual);
		pickNodes(void) {oPickInfo = NULL; onNode = testNode; onVisual = testVisual;}
	};

	pickNodes oPickNodes;

	enum getMoveConstraints {XYZ=0, X, Y, Z, XY, YZ, XZ};

	fglWeakREF<fglNode> oStartNode;
	fglWeakREF<fglViewport> oViewport;
	fglBindREF<fglCollectionWeak<fglVisual>> aPickedVisuals;
	fglBindREF<fglCollectionWeak<fglNode>> aPickedNodes;
	fglWeakREF<fglVisual> oPickedVisual;
	fglWeakREF<fglNode> oPickedNode;
	fglVECTOR oPickedPoint;
	fglVECTOR oPickedPointGlobal;
	fglVECTOR oWinP; //coords in viewport space
	fglVECTOR oStartP;
	float nMinDist;
	bool isPickNearest;
	bool isPickDisabled;
	float nPickRadius;
	
	//bad for perf virtual ~fglPickInfo(void) { }
	
	int pick(long pnX, long pnY, fglNode* poStartNode=NULL, fglViewport* poViewport=NULL);
	fglVECTOR getMove(const fglNode& poNode, unsigned pnConstraint, long pnX1, long pnY1, long pnX2, long pnY2) const;
	virtual void call(fglEvent& poEvent);
	virtual fglString fgl_class(void) const { return "pickinfo"; }
};

#endif