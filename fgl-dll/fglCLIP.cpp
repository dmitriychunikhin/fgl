#include "fgl3DGen.h"
#include "fglScene.h"

fglCLIP* goRenderClip(NULL);
double gaPlane[4];


void fglCLIP::build(const fglBBox& poBBox)
{
	for (unsigned i = 0; i < 6; i++)
	{
		aPlanes[i] = poBBox.getPlane(i);
		aPlanesState[i] = true;
	}
}

void fglCLIP::bind(void)
{
	if (goRenderClip!=this) oParentClip = goRenderClip;
	goRenderClip = this;

	for (unsigned i = 0; i < 6; i++)
	{
		if (aPlanesState[i])
		{
			gaPlane[0] = aPlanes[i].a1; gaPlane[1] = aPlanes[i].a2; gaPlane[2] = aPlanes[i].a3; gaPlane[3] = aPlanes[i].a4;
			glClipPlane(GL_CLIP_PLANE0+i, &gaPlane[0]);
			glEnable(GL_CLIP_PLANE0+i); 
		}
		else 
		{
			glDisable(GL_CLIP_PLANE0+i);
		}
	}
}


void fglCLIP::unbind(void)
{
	goRenderClip = oParentClip;
	if (oParentClip) 
	{
		oParentClip->bind();
	}
	else
	{
		glDisable(GL_CLIP_PLANE0); glDisable(GL_CLIP_PLANE1); glDisable(GL_CLIP_PLANE2); glDisable(GL_CLIP_PLANE3); glDisable(GL_CLIP_PLANE4); glDisable(GL_CLIP_PLANE5);
	}
	oParentClip = NULL;
}
