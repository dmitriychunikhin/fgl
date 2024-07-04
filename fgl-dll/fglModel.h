#ifndef _FGLMODEL_H_
#define _FGLMODEL_H_

#include "fglSceneGraph.h"
#include "fglMesh.h"
#include "fglSkel.h"
#include "fglAnim.h"


class fglModel : public fglVisual
{
	friend class fglBindFactory;

protected:
	fglModel(void);

private:
	fglREF<fglJoint> oRootJoint;
public:

	virtual void setRootJoint(const fglREF<fglJoint>& poRootJoint);
	const fglREF<fglJoint>& getRootJoint(void) const;
	const fglREF<fglJoint>* findJoint(const fglString& pcName) const;

	fglBindREF<fglCollection<fglAnim>> aAnims;
	
	//bad for perf virtual ~fglModel(void);
	virtual fglString fgl_class(void) const { return "model"; }

	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);
	void copySkel(const fglModel* poModel, fglBITFLAG pnShare=0);
	void copyAnim(const fglModel* poModel, fglBITFLAG pnShare=0);
		
	void buildLOD(float pnViewRange=100.0f, float pnStep=2.0f, unsigned pnSteps=0);
	
	virtual void calcBounds(void);
	void loadData(fglString& pcPath);
	void bindSkel(void);
	void buildAnim(void);
	void transformByBones(void);

	static void renderModel(const fglModel* poModel);
			
	virtual void call(fglEvent& poEvent);
};

#endif