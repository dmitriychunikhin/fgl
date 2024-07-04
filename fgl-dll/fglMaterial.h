#ifndef _FGLMATERIAL_H_
#define _FGLMATERIAL_H_

#include "common.h"
#include "fgl3DGen.h"
#include "fglBind.h"
#include "fglTexture.h"
#include "fglShader.h"

class fglMaterial : public fglBind
{
	friend class fglBindFactory;

protected:
	fglMaterial(void) = default;

private:
	fglMaterial* oParentMaterial{ NULL };
	fglTexture* oParentTexture{ NULL };
	fglShaderProg* oParentShaderProg{ NULL };

public:
	fglCOLOR color_diffuse{ 0.8f, 0.8f, 0.8f, 1.0f }; //Diffuse color of the material. This is typically scaled by the amount of incoming diffuse light (e.g. using gouraud shading) 
	fglCOLOR color_specular{ 0.0f, 0.0f, 0.0f, 1.0f }; //Specular color of the material. This is typically scaled by the amount of incoming specular light (e.g. using phong shading) 
	fglCOLOR color_ambient{ 0.2f, 0.2f, 0.2f, 1.0f }; //Ambient color of the material. This is typically scaled by the amount of ambient light 
	fglCOLOR color_emission{ 0.0f, 0.0f, 0.0f, 1.0f }; //Emissive color of the material. This is the amount of light emitted by the object. In real time applications it will usually not affect surrounding objects, but raytracing applications may wish to treat emissive objects as light sources. 
	fglCOLOR color_transparent{ 0.0f, 0.0f, 0.0f, 1.0f }; //Defines the transparent color of the material, this is the color to be multiplied with the color of translucent light to construct the final 'destination color' for a particular position in the screen buffer. T 
	float opacity{ 1.0f }; //Defines the opacity of the material in a range between 0..1. 
	float shininess{ 0.0f }; //Defines the shininess of a phong-shaded material. This is actually the exponent of the phong specular equation 
	float shininess_strength{ 1.0f }; //Scales the specular color of the material. 
	float refracti{ 1.0f }; //Defines the Index Of Refraction for the material. That's not supported by most file formats. 

	int lighting{ -1 }; // -1 - not defined, 0 - false, 1 - true
	int shading{ -1 }; //shading model: -1 - not defined, 0 - flat, >0 - smooth
	int twosided{ -1 }; //Specifies whether meshes using this material must be rendered without backface culling. -1 - not defined, 0 - false, 1 - true
	int wireframe{ -1 }; //draw in wireframe mode: -1 - not defined, 0 - false, 1 - true
	float lineWidth{ -1.0f }; //line width: -1 not defined (1.0f is default)

	int renderLighting{ -1 };
	int renderShading{ -1 };
	int renderTwosided{ -1 };
	int renderWireframe{ -1 };
	float renderLineWidth{ -1.0f };

	fglREF<fglShaderProg> oShaderProg; 
	fglREF<fglTexture> oTexture;

	static fglTexture* oRenderTexture;
	static fglShaderProg* oRenderShaderProg;

	virtual fglString fgl_class(void) const { return "material"; }
	void reset(void);
	//bad for perf virtual ~fglMaterial(void);
	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);

	void bind(void);
	void unbind(void);

	void call(fglEvent& poEvent);
};


#endif
