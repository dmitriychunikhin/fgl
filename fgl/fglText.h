#ifndef _FGLTEXT_H_
#define _FGLTEXT_H_
#include "fglSceneGraph.h"
#include "fglMesh.h"

#define FREETYPE2_STATIC
#define USE_FREETYPE2_STATIC
#include <ft2build.h>
#include FT_FREETYPE_H
#pragma comment (lib,"freetype.lib")

class fglFontGlyph : public fglMesh
{
	friend class fglBindFactory;

protected:
	fglFontGlyph(void) = default;

public:
	//bad for perf virtual ~fglFontGlyph(void);

	virtual fglString fgl_class(void) const { return "fontglyph"; }

	float getWidth(void) const;
	float getHeight(void) const;
	float getAdvanceHor(void) const;
	float getAdvanceVert(void) const;
	
	void load(const fglWChar& pChar, const FT_Face poFace);


private:
	float width{ 0.0f };
	float height{ 0.0f };
	float advanceHor{ 0.0f };
	float advanceVert{ 0.0f };
	
	class contour
	{
	public:
		typedef fglArray<fglVECTOR> APOINTS;
		APOINTS aPoints;
		APOINTS aPointsOutset;
		bool isClockwise{ false };

		contour(void) = default;
		contour(const FT_Vector* paPoints, const char* paTags, unsigned pnPoints);
		void addPoint(const fglVECTOR& poPoint);
		void addQuadraticBezierCurve(const fglVECTOR& a, const fglVECTOR& b, const fglVECTOR& c);
		void addCubicBezierCurve(const fglVECTOR& a, const fglVECTOR& b, const fglVECTOR& c, const fglVECTOR& d);
		void setParity(unsigned pnParity);
		fglVECTOR calcOutsetPoint(fglVECTOR a, fglVECTOR b, fglVECTOR c);
	};

	class tessVert
	{ 
	public:
		double x{ 0.0f };
		double y{ 0.0f };
		double z{ 0.0f };
		tessVert(const double& px, const double& py, const double& pz) { x=px; y=py; z=pz; } 
		tessVert(void) = default;
	};

	static void CALLBACK tessError(unsigned pnErrCode, fglFontGlyph* mesh);
	static void CALLBACK tessVertex(void* pData, fglFontGlyph* poMesh);
	static void CALLBACK tessCombine(double poNewVert[3], void* paVerts[4], float paVertsWeight[4], void** pOutData, fglFontGlyph* poMesh);
	static void CALLBACK tessBegin(unsigned pnType, fglFontGlyph* poMesh);
	static void CALLBACK tessEnd(fglFontGlyph* poMesh);
	
	fglArray<tessVert> aTessVerts;
	unsigned nTessStep{ 0 };
	unsigned nTessSubMeshVerts{ 0 };
	unsigned nTessSubMeshType{ 0 };
	unsigned nTessVerts{ 0 };
	unsigned nTessFaces{ 0 };

};


class fglFont
{
public:

	fglFont(void) = default;

	virtual ~fglFont(void);

	void load(const fglString& pcFontPath, unsigned pnType, unsigned pnSize, float pnOutset=0.0f, float pnDepth=0.0f);
	const fglFontGlyph* getGlyph(fglWChar pChar);

	void setScene(const fglWeakREF<fglScene>& poScene);

	float getAscender(void) const;
	float getDescender(void) const;
	float getHeight(void) const;
	float getMaxAdvance(void) const;

private:
	fglFont(const fglFont&);
	const fglFont& operator = (const fglFont&);

	FT_Face oFace{ NULL };
	fglWeakREF<fglScene> oScene;

	typedef fglUMap<fglWChar, fglBindREF<fglFontGlyph>> AGLYPHS;
	AGLYPHS aGlyphs;

	float ascender{ 0.0f };
	float descender{ 0.0f };
	float height{ 0.0f };
	float maxAdvance{ 0.0f };
};


class fglFontManager
{
private:
	typedef fglMap<fglString, fglFont*> AFONTS;

	fglFontManager(const fglFontManager&);
	const fglFontManager& operator = (const fglFontManager&);

	AFONTS aFonts;

	fglWeakREF<fglScene> oScene;

public:

	fglString fontspath{ "" };

	fglFontManager(void);

	virtual ~fglFontManager(void);

	fglFont* getFont(const fglString& pcFontName, unsigned pnType, unsigned pnSize, float pnOutset=0.0f, float pnDepth=0.0f);

	void setScene(const fglWeakREF<fglScene>& poScene);
};


class fglText : public fglVisual
{
	friend class fglBindFactory;

protected:
	fglText(void);

private:
	fglFont* oFont{ NULL };

	unsigned fontType{ 0 };
	fglString fontName{ "" };
	unsigned fontSize{ 10 };
	unsigned alignment{ ALIGN_LEFT };
	float lineLength{ 0.0f };
	float lineSpacing{ 1.0f };
	float outset{ 0.0f };
	float depth{ 0.0f };

	fglString textA{ "" };
	fglWString textW{ L"" };
	
	class fglTextLine
	{
	public:
		float left{ 0.0f };
		float top{ 0.0f };
		float width{ 0.0f };
		float height{ 0.0f };
		float justifySpacing{ 0.0f };
		
		typedef fglArray<const fglFontGlyph*> AGLYPHS;
		AGLYPHS aGlyphs;
		
		fglTextLine(void) = default;
	};

	typedef fglArray<fglTextLine> ALINES;
	ALINES aLines;
	

public:

	enum {TEXTURE=1, POLYGON, OUTLINE, EXTRUDED};
	enum {ALIGN_LEFT=1, ALIGN_CENTER, ALIGN_RIGHT, ALIGN_JUSTIFY};
	
	virtual fglString fgl_class(void) const { return "text"; }
	virtual bool copy(const fglBind* poSrc, fglBITFLAG pnShare=0);

	//bad for perf virtual ~fglText(void);

	virtual void calcBounds(void);

	void build(void);
	
	static void renderText(const fglText* poText);
			
	virtual void call(fglEvent& poEvent);

	fglString toString(void) 
	{ 
		std::ostringstream lcRes; 
		lcRes << "fontName: " << fontName << std::endl; 
		lcRes << "fontType: " << fontType << std::endl; 
		lcRes << "fontSize: " << fontSize << std::endl; 
		lcRes << "alignment: " << alignment << std::endl; 
		lcRes << "lineLength: " << lineLength << std::endl; 
		lcRes << "lineSpacing: " << lineSpacing << std::endl; 
		return lcRes.str(); 
	}
};


#endif