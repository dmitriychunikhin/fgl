#include "fglMaterial.h"
#include "fglScene.h"

fglTexture* fglMaterial::oRenderTexture(NULL);
fglShaderProg* fglMaterial::oRenderShaderProg(NULL);

void fglMaterial::call(fglEvent& poEvent) 
{
	if (poEvent.name == fglEVENT_color_diffuse) { if (poEvent.nParams) { color_diffuse.set( _GetValue2ui(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(color_diffuse.get()); return; }
	if (poEvent.name == fglEVENT_color_specular) { if (poEvent.nParams) { color_specular.set( _GetValue2ui(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(color_specular.get()); return; }
	if (poEvent.name == fglEVENT_color_ambient) { if (poEvent.nParams) { color_ambient.set( _GetValue2ui(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(color_ambient.get()); return; }
	if (poEvent.name == fglEVENT_color_emission) { if (poEvent.nParams) { color_emission.set( _GetValue2ui(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(color_emission.get()); return; }
	if (poEvent.name == fglEVENT_color_transparent) { if (poEvent.nParams) { color_transparent.set( _GetValue2ui(poEvent.aParams[0]) ); setSceneRenderFlag(); } _RetValue2f(color_transparent.get()); return; }

	if (poEvent.name == fglEVENT_opacity) { if (poEvent.nParams) { opacity = _GetValue2f(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2f(opacity); return; }
	if (poEvent.name == fglEVENT_shininess) { if (poEvent.nParams) { shininess = _GetValue2f(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2f(shininess); return; }
	if (poEvent.name == fglEVENT_shininess_strength) { if (poEvent.nParams) { shininess_strength = _GetValue2f(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2f(shininess_strength); return; }
	if (poEvent.name == fglEVENT_refracti) { if (poEvent.nParams) { refracti = _GetValue2f(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2f(refracti); return; }

	if (poEvent.name == fglEVENT_lighting) { if (poEvent.nParams) { lighting = _GetValue2i(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2i(lighting); return; }
	if (poEvent.name == fglEVENT_shading) { if (poEvent.nParams) { shading = _GetValue2i(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2i(shading); return; }
	if (poEvent.name == fglEVENT_twosided) { if (poEvent.nParams) { twosided = _GetValue2i(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2i(twosided); return; }
	if (poEvent.name == fglEVENT_wireframe) { if (poEvent.nParams) { wireframe = _GetValue2i(poEvent.aParams[0]); setSceneRenderFlag(); } _RetValue2i(wireframe); return; }
	
	
	if (poEvent.name == fglEVENT_hShader)
	{
		if (poEvent.nParams) { oShaderProg.inst( getBind<fglShaderProg>(_GetValue2i(poEvent.aParams[0])) ); setSceneRenderFlag(); }
		if (oShaderProg.empty()) _RetValue2i(0); else _RetValue2i(oShaderProg->getHFGL());
		return;
	}

	if (poEvent.name == fglEVENT_hTexture)
	{
		if (poEvent.nParams) { oTexture.inst( getBind<fglTexture>(_GetValue2i(poEvent.aParams[0])) ); setSceneRenderFlag(); }
		if (oTexture.empty()) _RetValue2i(0); else _RetValue2i(oTexture->getHFGL());
		return;
	}

	fglBind::call(poEvent);
}


void fglMaterial::reset(void)
{
	color_diffuse.set(0.8f, 0.8f, 0.8f, 1.0f); 
	color_specular.set(0.0f, 0.0f, 0.0f, 1.0f); 
	color_ambient.set(0.2f, 0.2f, 0.2f, 1.0f); 
	color_emission.set(0.0f, 0.0f, 0.0f, 1.0f); 
	color_transparent.set(0.0f, 0.0f, 0.0f, 1.0f); 
	opacity = 1.0f; 
	shininess = 0.0f; 
	shininess_strength = 1.0f; 
	refracti = 1.0f; 

	lighting = -1;
	shading = -1;
	twosided = -1;
	wireframe = -1;
	lineWidth = -1.0f;
}



bool fglMaterial::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglMaterial* loMaterial = dynamic_cast<const fglMaterial*>(poSrc);
	if (!loMaterial) return false;

	if ( !fglBind::copy(poSrc, pnShare) ) return false;

	color_diffuse = loMaterial->color_diffuse;
	color_specular = loMaterial->color_specular;
	color_ambient = loMaterial->color_ambient;
	color_emission = loMaterial->color_emission;
	color_transparent = loMaterial->color_transparent;

	opacity = loMaterial->opacity;
	shininess = loMaterial->shininess;
	shininess_strength = loMaterial->shininess_strength;
	refracti = loMaterial->refracti;

	lighting = loMaterial->lighting;
	shading = loMaterial->shading;
	twosided = loMaterial->twosided;
	wireframe = loMaterial->wireframe;
	lineWidth = loMaterial->lineWidth;
	
	oShaderProg = loMaterial->oShaderProg;

	return true;
}

void fglMaterial::bind(void)
{
	if (fglScene::oRenderMaterial != this) 
	{
		oParentMaterial = fglScene::oRenderMaterial;
		oParentShaderProg = fglMaterial::oRenderShaderProg;
		oParentTexture = fglMaterial::oRenderTexture;
	}
	fglScene::oRenderMaterial = this;
	
	if ( oShaderProg.exist() ) if ( oShaderProg->isReady() ) fglMaterial::oRenderShaderProg = oShaderProg.map();
	if ( fglMaterial::oRenderShaderProg ) fglMaterial::oRenderShaderProg->bind(); else fglShaderProg::unbind();
	
	if ( oTexture.exist() ) if ( oTexture->isReady() ) fglMaterial::oRenderTexture = oTexture.map();
	if ( fglMaterial::oRenderTexture )
	{
		glEnable(GL_TEXTURE_2D);
		fglMaterial::oRenderTexture->bind();
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		fglTexture::unbind();
	}
	
	renderLighting = lighting; 
	renderShading = shading;
	renderTwosided = twosided;
	renderWireframe = wireframe;
	renderLineWidth = lineWidth;

	if (oParentMaterial) 
	{
		if (renderLighting==-1) renderLighting = oParentMaterial->renderLighting;
		if (renderShading==-1) renderShading = oParentMaterial->renderShading;
		if (renderTwosided==-1) renderTwosided = oParentMaterial->renderTwosided;
		if (renderWireframe==-1) renderWireframe = oParentMaterial->renderWireframe;
		if (renderLineWidth==-1.0f) renderLineWidth = oParentMaterial->renderLineWidth;
	}
	if (renderLineWidth==-1.0f) renderLineWidth = 1.0f;

	if (renderLighting==1)
	{
		glEnable(GL_LIGHTING);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (float*)&color_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (float*)&color_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (float*)&color_ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (float*)&color_emission);
		if (shininess) {
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * shininess_strength);
		} else {
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
			fglCOLOR loColor(0.0f, 0.0f, 0.0f); 
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (float*)&loColor);
		}
		if (fglScene::oRenderNodes.currentView.scale.isUnit()) 
		{
			glDisable(GL_NORMALIZE);
			glDisable(GL_RESCALE_NORMAL);
		}
		else if (fglScene::oRenderNodes.currentView.scale.isUniform()) 
		{
			glDisable(GL_NORMALIZE);
			glEnable(GL_RESCALE_NORMAL);
		}
		else
		{
			glEnable(GL_NORMALIZE);
			glDisable(GL_RESCALE_NORMAL);
		}
	}
	else
	{
		glDisable(GL_LIGHTING);
		glColor4fv(&color_diffuse.r);
		glDisable(GL_NORMALIZE);
		glDisable(GL_RESCALE_NORMAL);
	}

	if (renderShading != 0 && renderShading != -1) 
	{
		glShadeModel(GL_SMOOTH); 
	}
	else 
	{
		glShadeModel(GL_FLAT); 
	}
		
	if (renderTwosided==1) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE); 

	if (renderWireframe==1) glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); else glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); 

	glLineWidth(renderLineWidth);
	

}


void fglMaterial::unbind(void)
{
	fglScene::oRenderMaterial = oParentMaterial;
	fglMaterial::oRenderShaderProg = oParentShaderProg;
	fglMaterial::oRenderTexture = oParentTexture;
	
	if (oParentMaterial) oParentMaterial->bind();
	oParentMaterial = NULL;
	oParentShaderProg = NULL;
	oParentTexture = NULL;
}
