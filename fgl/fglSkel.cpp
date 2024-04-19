#include "fglSkel.h"
#include "fglScene.h"


bool fglJoint::setParent(const fglREF<fglNode>& poParent)
{
	if ( poParent.exist() && dynamic_cast<fglJoint*>(poParent.map())==NULL ) return setParentValue(poParent); 
	if ( ! fglNode::setParent(poParent) ) return false;
	return true;
}

bool fglJoint::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglJoint* loJoint = dynamic_cast<const fglJoint*>(poSrc);
	if (!loJoint) return false;

	if ( !fglNode::copy(poSrc, pnShare) ) return false;

	return true;
}

bool fglBone::setParent(const fglREF<fglNode>& poParent)
{
	if ( poParent.exist() && dynamic_cast<fglJoint*>(poParent.map())==NULL ) return false;

	return fglNode::setParent(poParent);
}

bool fglBone::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglBone* loBone = dynamic_cast<const fglBone*>(poSrc);
	if (!loBone) return false;

	if ( !fglNode::copy(poSrc, pnShare) ) return false;

	aVerts.copy(&loBone->aVerts); 

	return true;
}

