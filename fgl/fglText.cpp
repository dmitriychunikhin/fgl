#include "fglText.h"
#include "fglScene.h"

FT_Library*  loFTLib(NULL);
unsigned lnFTLib(0);

#ifndef CALLBACK
#define CALLBACK
#endif

#if defined __APPLE_CC__ && __APPLE_CC__ < 5465
    typedef void (*GLUTesselatorFunction) (...);
#elif defined WIN32 && !defined __CYGWIN__
    typedef void (CALLBACK *GLUTesselatorFunction) ();
#else
    typedef void (*GLUTesselatorFunction) ();
#endif

void fglText::call(fglEvent& poEvent) 
{
	if (poEvent.name == fglEVENT_fontType) { if (poEvent.nParams) { fontType = std::max<int>(0,_GetValue2i(poEvent.aParams[0])); build(); } _RetValue2i(fontType); return; }
	if (poEvent.name == fglEVENT_fontSize) { if (poEvent.nParams) { fontSize = std::max<int>(0,_GetValue2i(poEvent.aParams[0])); build(); } _RetValue2i(fontSize); return; }
	if (poEvent.name == fglEVENT_fontName) { if (poEvent.nParams) { fontName = _GetValue2c(poEvent.aParams[0]); build(); } _RetValue2c(fontName); return; }
	if (poEvent.name == fglEVENT_alignment) { if (poEvent.nParams) { alignment = std::max<int>(0,_GetValue2i(poEvent.aParams[0])); build(); } _RetValue2i(alignment); return; }
	if (poEvent.name == fglEVENT_lineLength) { if (poEvent.nParams) { lineLength = std::max<float>(0.0f,_GetValue2f(poEvent.aParams[0])); build(); } _RetValue2f(lineLength); return; }
	if (poEvent.name == fglEVENT_lineSpacing) { if (poEvent.nParams) { lineSpacing = std::max<float>(1.0f,_GetValue2f(poEvent.aParams[0])); build(); } _RetValue2f(lineSpacing); return; }
	if (poEvent.name == fglEVENT_outset) { if (poEvent.nParams) { outset = std::max<float>(0.0f,_GetValue2f(poEvent.aParams[0])); build(); } _RetValue2f(outset); return; }
	if (poEvent.name == fglEVENT_depth) { if (poEvent.nParams) { depth = std::max<float>(0.0f,_GetValue2f(poEvent.aParams[0])); build(); } _RetValue2f(depth); return; }

	if (poEvent.name == fglEVENT_text) { if (poEvent.nParams) { textA = _GetValue2c(poEvent.aParams[0]); _CharToWChar(textW, textA); build(); } _RetValue2c(textA); return; }

	fglVisual::call(poEvent);
}


fglText::fglText(void)
{
	oFont = NULL;
	fontType = 0;
	fontName = "";
	fontSize = 10;
	alignment = ALIGN_LEFT;
	lineLength = 0.0f;
	lineSpacing = 1.0f;
	outset = 0.0f;
	depth = 0.0f;
	oMaterial = fglBindFactory::createNonPersistent<fglMaterial>();
	oMaterial->lighting = 0;
	oMaterial->shading = 0;
	oMaterial->twosided = 0;
	
	//textA.reserve(255);
	//textW.reserve(255);


	renderImpl = (void (*)(const fglVisual*))fglText::renderText;
}


bool fglText::copy(const fglBind* poSrc, fglBITFLAG pnShare)
{
	const fglText* loText = dynamic_cast<const fglText*>(poSrc);
	if (!loText) return false;

	if ( !fglVisual::copy(poSrc, pnShare) ) return false;

	fontType = loText->fontType;
	fontName = loText->fontName;
	fontSize = loText->fontSize;
	alignment = loText->alignment;
	lineLength = loText->lineLength;
	lineSpacing = loText->lineSpacing;
	outset = loText->outset;
	depth = loText->depth;

	textA = loText->textA;
	textW = loText->textW;

	build();

	return true;
}


void fglText::build(void)
{
	fglScene* loScene = getScene().lock().map();
	if (!loScene) return;

	oFont = loScene->oFontManager.getFont(fontName, fontType, fontSize, outset, depth);
	
	calcBounds();
}

void fglText::calcBounds(void)
{
	bboxInt.setNull();
	aLines.clear();

	if ( ! oFont ||  textW.empty() ) { fglVisual::calcBounds(); return; }
	
	const fglFontGlyph* loGlyph;
	fglTextLine loLine; 
	bool llBreakLine;

	fglWString::const_iterator liChar(textW.cbegin()), liCharEnd(textW.cend()), liCharPrev(textW.cbegin());
	for (; liChar!=liCharEnd; ++liChar)
	{
		llBreakLine = false;
		if ( *liChar == '\n' || *liChar == '\r' )
		{
			llBreakLine = true;
			if ( (*liCharPrev == '\n' || *liCharPrev == '\r') && (*liChar != *liCharPrev) )
			{
				liCharPrev = liChar;
				continue;
			}
		}

		loGlyph = oFont->getGlyph(*liChar);
		
		if ( loGlyph && ! llBreakLine ) 
		{
			loLine.width += loGlyph->getAdvanceHor(); 
			loLine.height = std::max<float>(loGlyph->getAdvanceVert(), loLine.height);

			if ( ! loLine.aGlyphs.empty() && lineLength && (loLine.width > lineLength) )
			{
				loLine.width -= loGlyph->getAdvanceHor();
				llBreakLine = true;
				liChar--;
			}
			else
			{
				loLine.aGlyphs.push_back(loGlyph);
			}
		}
		
		if ( llBreakLine || (liChar+1)==liCharEnd )
		{
			if ( ! loLine.height )
			{
				if (loGlyph) loLine.height = loGlyph->getAdvanceVert(); 
				else loLine.height = oFont->getHeight();
			}

			loLine.height = lineSpacing * loLine.height;
			
			if ( lineLength ) bboxInt.max.x = lineLength; else bboxInt.max.x = std::max<float>(bboxInt.max.x, loLine.width);
			bboxInt.min.y -= loLine.height;
			loLine.top = bboxInt.min.y;
			
			aLines.push_back(loLine);
		
			loLine.width = 0.0f;
			loLine.height = 0.0f;
			loLine.aGlyphs.clear();
		}
		liCharPrev = liChar;
	}
	
	for (unsigned i = 0; i < aLines.size(); i++)
	{
		if (alignment == fglText::ALIGN_CENTER)
		{
			aLines[i].left = (bboxInt.max.x - aLines[i].width)*0.5f;
		}
		else if (alignment == fglText::ALIGN_RIGHT)
		{
			aLines[i].left = (bboxInt.max.x - aLines[i].width);
		}
		else if (alignment == fglText::ALIGN_JUSTIFY && aLines[i].aGlyphs.size() )
		{
			aLines[i].left = 0.0f;
			aLines[i].justifySpacing = ( bboxInt.max.x - aLines[i].width ) / aLines[i].aGlyphs.size();
		}
	}

	fglVisual::calcBounds();
}


void fglText::renderText(const fglText* poText)
{
	if ( ! poText->oFont || poText->textW.empty() ) return;

	fglVECTOR loPos, loSavePos = fglScene::oRenderNodes.currentView.pos;

	fglText::ALINES::const_iterator liLine(poText->aLines.cbegin()), liLineEnd(poText->aLines.cend());
	fglTextLine::AGLYPHS::const_iterator liGlyph, liGlyphEnd;
	for (; liLine!=liLineEnd; ++liLine)
	{
		loPos.x = liLine->left; 
		loPos.y = liLine->top;
		liGlyph = liLine->aGlyphs.cbegin();
		liGlyphEnd = liLine->aGlyphs.cend();
		for (; liGlyph!=liGlyphEnd; ++liGlyph)
		{
			fglScene::oRenderNodes.currentView.moveDir(loPos);
			fglScene::oRenderNodes.currentViewMatrix.moveTo(fglScene::oRenderNodes.currentView.pos);
			fglScene::oRenderNodes.currentViewMatrix.setCurrentView();
			
			fglFontGlyph::renderMesh(*liGlyph);
			
			loPos.x = (*liGlyph)->getAdvanceHor() + liLine->justifySpacing;
			loPos.y = 0.0f;
		}
		fglScene::oRenderNodes.currentView.pos = loSavePos;
	}

	fglScene::oRenderNodes.currentView.pos = loSavePos;
	fglScene::oRenderNodes.currentViewMatrix.moveTo(loSavePos);
	fglScene::oRenderNodes.currentViewMatrix.setCurrentView();
}


/////////////////////////////////////////////////////////
fglFontManager::fglFontManager(void)
{
	lnFTLib++;
	if ( ! loFTLib ) 
	{
		loFTLib = new FT_Library;

		FT_Error loErr = FT_Init_FreeType(loFTLib);
		if (loErr)
		{
			delete loFTLib;
			loFTLib = NULL;
			throw fglException("Freetype2 init error");
		}
	}
}

fglFontManager::~fglFontManager(void)
{
	for(AFONTS::iterator liFont = aFonts.begin(); liFont != aFonts.end(); liFont++)
	{
		delete (*liFont).second;
	}
	aFonts.clear();

	lnFTLib--;
	if ( lnFTLib == 0 && loFTLib )
	{
        FT_Done_FreeType(*loFTLib);
		delete loFTLib;
		loFTLib = NULL;
	}
}

void fglFontManager::setScene(const fglWeakREF<fglScene>& poScene)
{ 
	if ( oScene.empty() ) oScene = poScene;
}


fglFont* fglFontManager::getFont(const fglString& pcFontName, unsigned pnType, unsigned pnSize, float pnOutset, float pnDepth)
{
	if (pcFontName.empty()) return NULL;
	if (!pnType) return NULL;
	if (!pnSize) return NULL;

	if (pnType != fglText::EXTRUDED ) pnOutset=pnDepth=0.0f;
	
	fglString lcFontName = pcFontName.substr(0,255);
	char lcBuf[512];
	sprintf(lcBuf, "%s_%i_%i_%.3f_%.3f\0", lcFontName.c_str(), pnType, pnSize, pnOutset, pnDepth);
	fglString lcKey = fglString(lcBuf);
	
	AFONTS::iterator liFont = aFonts.find(lcKey);
	if(liFont != aFonts.end())
	{
		return liFont->second;
	}
	
	fglString lcPath = fontspath + lcFontName;
	std::ifstream loFile(lcPath);
    if (!loFile.good()) 
	{
		lcPath = fontspath + lcFontName + ".ttf";
		loFile.open(lcPath);
		if (!loFile.good()) return NULL;
	}
	loFile.close();
	
	fglFont* loFont = new fglFont();
	loFont->setScene(oScene);
	loFont->load(lcPath, pnType, pnSize, pnOutset, pnDepth);
	aFonts[lcKey] = loFont;
	
	return loFont;
}


fglFont::~fglFont(void)
{
	FT_Done_Face(oFace);
}

void fglFont::setScene(const fglWeakREF<fglScene>& poScene)
{ 
	if ( oScene.empty() ) oScene = poScene;
}


void fglFont::load(const fglString& pcFontPath, unsigned pnType, unsigned pnSize, float pnOutset, float pnDepth)
{
	if (oFace) return;

	FT_Error loErr;
	loErr = FT_New_Face ( *loFTLib, pcFontPath.c_str(), 0, &oFace );
	if (loErr) { oFace=NULL; return; }
	
	loErr = FT_Select_Charmap ( oFace, FT_ENCODING_UNICODE );
	if (loErr) { oFace=NULL; return; }

	loErr = FT_Set_Char_Size ( oFace, 0, pnSize * 64, 72, 72);
	if (loErr) { oFace=NULL; return; }

	ascender = static_cast<float>(oFace->size->metrics.ascender) / 64.0f;
	descender = static_cast<float>(oFace->size->metrics.descender) / 64.0f;
	height = static_cast<float>(oFace->size->metrics.height) / 64.0f;
	maxAdvance = static_cast<float>(oFace->size->metrics.ascender) / 64.0f;
}

float fglFont::getAscender(void) const { return ascender; }
float fglFont::getDescender(void) const { return descender; }
float fglFont::getHeight(void) const { return height; }
float fglFont::getMaxAdvance(void) const { return maxAdvance; }


const fglFontGlyph* fglFont::getGlyph(fglWChar pChar)
{
	if ( ! oFace ) return NULL;

	AGLYPHS::iterator liGlyph = aGlyphs.find(pChar);
	if (liGlyph != aGlyphs.end()) return liGlyph->second.callBindREFMap();
	
	fglFontGlyph& loGlyph = const_cast<fglFontGlyph&>(aGlyphs[pChar].callBindREFGet());
	loGlyph.setScene(oScene);
	loGlyph.load(pChar, oFace);

	return &loGlyph;
}


float fglFontGlyph::getWidth(void) const { return width; }
float fglFontGlyph::getHeight(void) const { return height; }
float fglFontGlyph::getAdvanceHor(void) const { return advanceHor; }
float fglFontGlyph::getAdvanceVert(void) const { return advanceVert; }


void fglFontGlyph::load(const fglWChar& pChar, const FT_Face poFace)
{
	//std::fstream out;
	//out.open("3.txt", std::fstream::out | std::fstream::app);
	//out << pChar << std::endl; 
	//out.close();

	if ( ! poFace ) return;
	if ( aVerts.exist() ) return;
		
	FT_UInt liGlyph = FT_Get_Char_Index(poFace, pChar);
	FT_Error loErr = FT_Load_Glyph(poFace, liGlyph, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
    if (loErr) return; 
	
	FT_GlyphSlot loGlyph = poFace->glyph;
	if ( ! loGlyph ) return;
	if ( loGlyph->format != FT_GLYPH_FORMAT_OUTLINE ) return;
	
	width = static_cast<float>(loGlyph->metrics.width) / 64.0f;
	height = static_cast<float>(loGlyph->metrics.height) / 64.0f;
	advanceHor = static_cast<float>(loGlyph->metrics.horiAdvance) / 64.0f;
	advanceVert = static_cast<float>(loGlyph->metrics.vertAdvance) / 64.0f;

	const FT_Outline& loOutline = loGlyph->outline;
	unsigned short lnContours = loOutline.n_contours;
	int outlineFlags = loOutline.flags;
	FT_Vector* laPoints;
	char* laTags;
	unsigned lnPoints(0), liPoint(0), liPointEnd(0);

	fglArray<contour> laContours;
	//laContours.reserve(lnContours);

	////////////////////////////////////////////////
    //calculate contours
	////////////////////////////////////////////////
	for (unsigned i = 0; i < lnContours; i++)
    {
		laPoints = &loOutline.points[liPoint];
		laTags = &loOutline.tags[liPoint];

		liPointEnd = loOutline.contours[i];
		lnPoints =  (liPointEnd - liPoint) + 1;
		laContours.push_back(contour(laPoints, laTags, lnPoints));
			
		liPoint = liPointEnd + 1;
    }

	//calc intersections points of contours
	for (unsigned i = 0; i < lnContours; i++)
	{
		const contour& loContour = laContours[i];

		// find the leftmost point.
		fglVECTOR loLeft(fglINFINITE, 0.0f, 0.0f);
		lnPoints = loContour.aPoints.size();

		for (unsigned j = 0; j < lnPoints; j++) if (loContour.aPoints[j].x < loLeft.x) loLeft = loContour.aPoints[j];

		unsigned lnParity = 0;
		fglVECTOR loP1, loP2;
		for (unsigned j = 0; j < lnContours; j++)
		{
			if (j == i) continue;

			const contour& loContour = laContours[j];
			lnPoints = loContour.aPoints.size();

			for (unsigned k = 0; k < lnPoints; k++)
			{
				loP1 = loContour.aPoints[k];
				loP2 = loContour.aPoints[(k + 1) % lnPoints];

				if ( (loP1.y < loLeft.y && loP2.y < loLeft.y) || (loP1.y >= loLeft.y && loP2.y >= loLeft.y) || (loP1.x > loLeft.x && loP2.x > loLeft.x) ) continue;
				else if ( loP1.x < loLeft.x && loP2.x < loLeft.x ) lnParity++;
				else if( (loP2-loLeft).x * (loP1-loLeft).y > (loP2-loLeft).y * (loP1-loLeft).x ) lnParity++;
			}
		}

		laContours[i].setParity(lnParity);
	}

	////////////////////////////////////////////////
	//build mesh
	////////////////////////////////////////////////
	nTessStep=0;
	while (nTessStep < 2)
	{
		//std::fstream out;
		//out.open("1.txt", std::fstream::out | std::fstream::app);
		//out << "step " << nTessStep << std::endl; 
		//out.close();

		GLUtesselator* loTess = gluNewTess();
		
		gluTessCallback(loTess, GLU_TESS_BEGIN_DATA,     (GLUTesselatorFunction)tessBegin);
		gluTessCallback(loTess, GLU_TESS_VERTEX_DATA,    (GLUTesselatorFunction)tessVertex);
		gluTessCallback(loTess, GLU_TESS_COMBINE_DATA,   (GLUTesselatorFunction)tessCombine);
		gluTessCallback(loTess, GLU_TESS_END_DATA,       (GLUTesselatorFunction)tessEnd);
		gluTessCallback(loTess, GLU_TESS_ERROR_DATA,     (GLUTesselatorFunction)tessError);

		if (outlineFlags & ft_outline_even_odd_fill) 
		{
			gluTessProperty(loTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
		}
		else
		{
			gluTessProperty(loTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
		}

		gluTessProperty(loTess, GLU_TESS_TOLERANCE, 0);
		gluTessNormal(loTess, 0.0f, 0.0f, 1.0f);
		//aTessVerts.reserve(20);
		nTessSubMeshVerts = nTessVerts = nTessFaces = 0;
		gluTessBeginPolygon(loTess, this);
		for (unsigned i = 0; i < lnContours; i++)
		{
			const contour& loContour = laContours[i];
			lnPoints = loContour.aPoints.size();
			gluTessBeginContour(loTess);
			for (unsigned j = 0; j < lnPoints; j++)
			{
				aTessVerts.push_back( tessVert(loContour.aPoints[j].x, loContour.aPoints[j].y, loContour.aPoints[j].z) );
				gluTessVertex(loTess, (GLdouble*)&aTessVerts.back(), (GLvoid*)&aTessVerts.back());
			}
			gluTessEndContour(loTess);
			
		}
		gluTessEndPolygon(loTess);
		if (nTessStep==0) { setFaces(nTessFaces, 3); aVerts.set_size(nTessVerts); }

		gluDeleteTess(loTess);
		aTessVerts.clear();
		nTessStep++;
	}
	buildVBO();
	calcBounds();
}

////////////////////////////////////////////////////////////////////////////////////
fglFontGlyph::contour::contour(const FT_Vector* paPoints, const char* paTags, unsigned pnPoints)
{
	if ( ! pnPoints ) return;
	
	//aPoints.reserve(pnPoints);
	//aPointsOutset.reserve(pnPoints);

	fglVECTOR loCur, loPrev, loNext, loPrev2, loNext2, loNext3;
	loCur.x = static_cast<float>(paPoints[(pnPoints - 1) % pnPoints].x);
	loCur.y = static_cast<float>(paPoints[(pnPoints - 1) % pnPoints].y);
	loNext.x = static_cast<float>(paPoints[0].x);
	loNext.y = static_cast<float>(paPoints[0].y);
	
	double lnPrevDir, lnDir = atan2( (loNext - loCur).y, (loNext - loCur).x) ;
	double lnAngle = 0.0f;

	for (unsigned i = 0; i < pnPoints; i++)
    {
		loPrev = loCur;
		loCur = loNext;
		loNext.x = static_cast<float>(paPoints[(i + 1) % pnPoints].x);
		loNext.y = static_cast<float>(paPoints[(i + 1) % pnPoints].y);
		lnPrevDir = lnDir;
		lnDir = atan2( (loNext - loCur).y, (loNext - loCur).x );

		double lnDDir = lnDir - lnPrevDir;
		if(lnDDir < -fglPI) lnDDir += 2 * fglPI;
		if(lnDDir > fglPI) lnDDir -= 2 * fglPI;
		lnAngle += lnDDir;
        
		if ( pnPoints < 2 || FT_CURVE_TAG(paTags[i]) == FT_Curve_Tag_On )
		{
			addPoint(loCur);
		}
		else if( FT_CURVE_TAG(paTags[i]) == FT_Curve_Tag_Conic )
		{
			loPrev2 = loPrev;
			loNext2 = loNext;
			
			if ( FT_CURVE_TAG( paTags[(i - 1 + pnPoints) % pnPoints]) == FT_Curve_Tag_Conic )
			{
				loPrev2 = (loCur + loPrev) * 0.5f;
				addPoint(loPrev2);
			}

			if( FT_CURVE_TAG(paTags[(i + 1) % pnPoints]) == FT_Curve_Tag_Conic )
			{
				loNext2 = (loCur + loNext) * 0.5f;
			}

			addQuadraticBezierCurve(loPrev2, loCur, loNext2);
		}
		else if ( FT_CURVE_TAG(paTags[i]) == FT_Curve_Tag_Cubic && FT_CURVE_TAG(paTags[(i + 1) % pnPoints]) == FT_Curve_Tag_Cubic )
		{
			loNext3.x = static_cast<float>(paPoints[(i + 2) % pnPoints].x);
			loNext3.y = static_cast<float>(paPoints[(i + 2) % pnPoints].y);
			addCubicBezierCurve(loPrev, loCur, loNext, loNext3);
		}
	}
	
	isClockwise = (lnAngle < 0.0f);
}

void fglFontGlyph::contour::addPoint(const fglVECTOR& poPoint)
{
	if( aPoints.empty() ) aPoints.push_back(poPoint); 
	else if ( poPoint != aPoints[aPoints.size() - 1] && poPoint != aPoints[0] ) aPoints.push_back(poPoint);
}

void fglFontGlyph::contour::addQuadraticBezierCurve(const fglVECTOR& a, const fglVECTOR& b, const fglVECTOR& c)
{
	fglVECTOR u,v;
	float t;
	for (unsigned i = 1; i < 5; i++)
	{
		t = static_cast<float>(i) / 5.0f;

		u = a * (1.0f - t) + b * t;
		v = b * (1.0f - t) + c * t;

		addPoint( u * (1.0f - t) + v * t);
	}
}

void fglFontGlyph::contour::addCubicBezierCurve(const fglVECTOR& a, const fglVECTOR& b, const fglVECTOR& c, const fglVECTOR& d)
{
	fglVECTOR u,v,w,m,n;
	float t;
	for (unsigned i = 0; i < 5; i++)
	{
		t = static_cast<float>(i) / 5.0f;

		u = a * (1.0f - t) + b * t;
		v = b * (1.0f - t) + c * t;
		w = c * (1.0f - t) + d * t;

		m = u * (1.0f - t) + v * t;
		n = v * (1.0f - t) + w * t;

		addPoint( m * (1.0f - t) + n * t);
	}
}

void fglFontGlyph::contour::setParity(unsigned pnParity)
{
	unsigned lnPoints = aPoints.size();
	
	if ( ((pnParity & 1) && isClockwise) || ( !(pnParity & 1) && !isClockwise) )
	{
		// Reverse orientation
		isClockwise = !isClockwise;
		fglVECTOR loTemp;
		for( unsigned i = 0; i < lnPoints/2; i++)
		{
			loTemp = aPoints[i];
			aPoints[i] = aPoints[lnPoints - 1 - i];
			aPoints[lnPoints - 1 -i] = loTemp;
		}
	}

	for (unsigned i = 0; i < lnPoints; i++)
	{
		unsigned liPrevPoint, liCurPoint, liNextPoint;

		liPrevPoint = (i + lnPoints - 1) % lnPoints;
		liCurPoint = i;
		liNextPoint = (i + lnPoints + 1) % lnPoints;

		aPointsOutset.push_back( calcOutsetPoint(aPoints[liPrevPoint], aPoints[liCurPoint], aPoints[liNextPoint]) );
	}
}

fglVECTOR fglFontGlyph::contour::calcOutsetPoint(fglVECTOR a, fglVECTOR b, fglVECTOR c)
{
	fglVECTOR ba = (a - b).normalize();
	fglVECTOR bc = c - b;

	fglVECTOR bcLeft;
	bcLeft.x = (bc.x * -ba.x) + (bc.y * -ba.y);
	bcLeft.y = (bc.x * ba.y) + (bc.y * -ba.x);

	double lnNorm = sqrt(bcLeft.x * bcLeft.x + bcLeft.y * bcLeft.y);
	double lnDist = 64.0f * sqrt((lnNorm - bcLeft.x) / (lnNorm + bcLeft.x));
	bcLeft.x = static_cast<float>(bcLeft.y < 0.0f ? lnDist : -lnDist);
	bcLeft.y = 64.0f;

	fglVECTOR bcRight;
	bcRight.x = bcLeft.x * -ba.x + bcLeft.y * ba.y;
	bcRight.y = bcLeft.x * -ba.y + bcLeft.y * -ba.x;

	return bcRight;
}

////////////////////////////////////////////////////////////////////////////////////
void CALLBACK fglFontGlyph::tessError(GLenum pnErrCode, fglFontGlyph* mesh)
{
	
}

void CALLBACK fglFontGlyph::tessBegin(GLenum pnType, fglFontGlyph* poMesh)
{
	poMesh->nTessSubMeshVerts = 0;
	poMesh->nTessSubMeshType = pnType;
	//if (pnType == GL_TRIANGLE_FAN )
	//{
	//	std::fstream out;
	//	out.open("1.txt", std::fstream::out | std::fstream::app);
	//	out << "fan" << std::endl; 
	//	out.close();
	//}
	//else
	//{
	//	std::fstream out;
	//	out.open("1.txt", std::fstream::out | std::fstream::app);
	//	out << "not fan" << std::endl; 
	//	out.close();
	//}

}

void CALLBACK fglFontGlyph::tessVertex(void* pData, fglFontGlyph* poMesh)
{
	GLdouble* loVert = static_cast<GLdouble*>(pData);	
	//std::fstream out;
	//	out.open("1.txt", std::fstream::out | std::fstream::app);
	//	out << static_cast<float>(loVert[0]) / 64.0f << ", " << static_cast<float>(loVert[1]) / 64.0f << std::endl; 
	//	out.close();

	if (poMesh->nTessStep==1)
	{
		GLdouble* loVert = static_cast<GLdouble*>(pData);
		poMesh->aVerts[poMesh->nTessVerts + poMesh->nTessSubMeshVerts].p.x = static_cast<float>(loVert[0]) / 64.0f;
		poMesh->aVerts[poMesh->nTessVerts + poMesh->nTessSubMeshVerts].p.y = static_cast<float>(loVert[1]) / 64.0f;
		poMesh->aVerts[poMesh->nTessVerts + poMesh->nTessSubMeshVerts].n.z = 1.0f;
	}
	poMesh->nTessSubMeshVerts++;
}

void CALLBACK fglFontGlyph::tessCombine(GLdouble poNewVert[3], void* paVerts[4], GLfloat paVertsWeight[4], void** pOutData, fglFontGlyph* poMesh)
{
	poMesh->aTessVerts.push_back( tessVert(poNewVert[0],poNewVert[1],poNewVert[2]) );
	*pOutData = &poMesh->aTessVerts.back();

	//std::fstream out;
	//	out.open("1.txt", std::fstream::out | std::fstream::app);
	//	out << "combine: " << static_cast<float>(poNewVert[0]) / 64.0f << ", " << static_cast<float>(poNewVert[1]) / 64.0f << std::endl; 
	//	out.close();

	//if (poMesh->nTessStep==1)
	//{
	//	poMesh->aVerts[poMesh->nTessVerts + poMesh->nTessSubMeshVerts].p.x = static_cast<float>(poNewVert[0]) / 64.0f;
	//	poMesh->aVerts[poMesh->nTessVerts + poMesh->nTessSubMeshVerts].p.y = static_cast<float>(poNewVert[1]) / 64.0f;
	//	poMesh->aVerts[poMesh->nTessVerts + poMesh->nTessSubMeshVerts].n.z = 1.0f;
	//	//poMesh->aVerts[poMesh->nTessVerts + poMesh->nTessFaceVerts].p.z = static_cast<float>(loVert[2]) / 64.0f;
	//}
	//poMesh->nTessSubMeshVerts++;
}

void CALLBACK fglFontGlyph::tessEnd(fglFontGlyph* poMesh)
{
	unsigned lnFaces = poMesh->nTessSubMeshType == GL_TRIANGLES ? poMesh->nTessSubMeshVerts/3 : poMesh->nTessSubMeshVerts-2;
	unsigned lnInds = lnFaces*3;
	unsigned lnOffsetInds = poMesh->nTessFaces*3;
	if (poMesh->nTessStep==1)
	{
		//std::fstream out;
		//out.open("1.txt", std::fstream::out | std::fstream::app);
		//out << "triangles " << poMesh->nTessFaces << ", verts " << poMesh->nTessVerts << std::endl; 
		//out.close();

		unsigned liVert=0;
		for (unsigned i = 0; i < lnInds; i++)
		{
			if (poMesh->nTessSubMeshType == GL_TRIANGLES || i < 3 )
			{
				poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert;
				liVert++;
			}
			else if (poMesh->nTessSubMeshType == GL_TRIANGLE_FAN)
			{
				if ( (i % 3)==0 ) poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts;
				else if ( (i % 3)==1 ) poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert-1; 
				else { poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert; liVert++; }
			}
			else if (poMesh->nTessSubMeshType == GL_TRIANGLE_STRIP)
			{
				if (liVert & 1)
				{
					if ( (i % 3)==0 ) poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert;
					else if ( (i % 3)==1 ) poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert-1; 
					else { poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert-2; liVert++; }
				}
				else
				{
					if ( (i % 3)==0 ) poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert-2;
					else if ( (i % 3)==1 ) poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert-1; 
					else { poMesh->aInds[lnOffsetInds+i] = poMesh->nTessVerts + liVert; liVert++; }
				}
			}
			else
			{
				poMesh->aInds[lnOffsetInds+i] = 0;
			}
		//std::fstream out;
		//out.open("1.txt", std::fstream::out | std::fstream::app);
		//out << poMesh->aVerts[poMesh->aInds[lnOffsetInds+i]].p.x << ", " << poMesh->aVerts[poMesh->aInds[lnOffsetInds+i]].p.y << std::endl; 
		//out.close();

			
		}
	}
	poMesh->nTessVerts += poMesh->nTessSubMeshVerts;
	poMesh->nTessFaces += lnFaces;
	poMesh->nTessSubMeshVerts = 0;
}

