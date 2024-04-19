#include "GLHelper.h"
#include "fglLog.h"
#include "fglScene.h"


void GLHelper::takeCaps(void) 
{
	glGetIntegerv( GL_MAX_LIGHTS, (GLint*)&nMaxLights ); 
	glGetIntegerv( GL_MAX_ELEMENTS_VERTICES, (GLint*)&nMaxElementsVertices );
	glGetIntegerv( GL_MAX_ELEMENTS_INDICES, (GLint*)&nMaxElementsIndices );

	nMaxLights = std::min<unsigned>(nMaxLights, 8);
	nMaxElementsIndices = std::min<unsigned>(std::max<unsigned>(nMaxElementsIndices, 255), 65535);
	nMaxElementsVertices = std::min<unsigned>(std::max<unsigned>(nMaxElementsVertices, 255), 65535);

}


////////////////////////////////////////////////////////////////
void GLHelper::setVSync(bool plEnable)
{
	// WGL_EXT_swap_control.
	if (wglSwapIntervalEXT) wglSwapIntervalEXT(plEnable ? 1 : 0);
}
////////////////////////////////////////////////////////////////

void GLHelper::resetLights(void) 
{ 
	glDisable(GL_LIGHT0); 
	glDisable(GL_LIGHT1); 
	glDisable(GL_LIGHT2); 
	glDisable(GL_LIGHT3); 
	glDisable(GL_LIGHT4); 
	glDisable(GL_LIGHT5); 
	glDisable(GL_LIGHT6); 
	glDisable(GL_LIGHT7);
}



////////////////////////////////////////////////////////////////
// Multisample anti-aliasing (MSAA)
int getCSAAPixelFormat(unsigned& pnSamples, fglString& pcResultDescr)
{
	
	HDC lhDC = wglGetCurrentDC();
	if ( ! lhDC )
	{
		pcResultDescr = "wglGetCurrentDC() returns NULL";
		pnSamples = 0;
		return 0;
	}

	struct CSAAPixelFormat
	{
		int numColorSamples;
		int numCoverageSamples;
		const char *pszDescription;
	};

	CSAAPixelFormat csaaPixelFormats[] =
	{
		{ 4, 8,  "8x CSAA" },
		{ 4, 16, "16x CSAA" },
		{ 8, 8,  "8xQ (Quality) CSAA" },
		{ 8, 16, "16xQ (Quality) CSAA" }
	};

	int totalCSAAFormats = static_cast<int>(sizeof(csaaPixelFormats) / sizeof(CSAAPixelFormat));

	int attributes[] =
	{
		WGL_SAMPLE_BUFFERS_ARB,  1,
		WGL_COLOR_SAMPLES_NV,    0,
		WGL_COVERAGE_SAMPLES_NV, 0,
		WGL_DOUBLE_BUFFER_ARB,   1,
		0, 0
	};

	int returnedPixelFormat = 0;
	UINT numFormats = 0;
	BOOL bStatus = FALSE;

	if ( pnSamples > 16 ) pnSamples = 16;
	
	int fromFormatIndex = 0;
	while ( (pnSamples>1) && (fromFormatIndex != (totalCSAAFormats-1)) ) { pnSamples = pnSamples / 2; fromFormatIndex++; }
	
	for (int i = fromFormatIndex-1; i >= 0; i--)
	{
		attributes[3] = csaaPixelFormats[i].numColorSamples;
		attributes[5] = csaaPixelFormats[i].numCoverageSamples;

		bStatus = wglChoosePixelFormatARB(lhDC, attributes, 0, 1, &returnedPixelFormat, &numFormats);

		if (bStatus == TRUE && numFormats)
		{
			if (pnSamples >= 8)
				pnSamples = csaaPixelFormats[i].numCoverageSamples;
			else
				pnSamples = csaaPixelFormats[i].numCoverageSamples / 4;
			
			pcResultDescr = csaaPixelFormats[i].pszDescription;
			return returnedPixelFormat;
		}
	}

	pnSamples = 0;
	pcResultDescr = "NVidia's coverage sample anti-aliasing (CSAA) isn't supported";
	return 0;
}


int getMSAAPixelFormat(unsigned& pnSamples, fglString& pcResultDescr)
{
	HDC lhDC = wglGetCurrentDC();
	if ( ! lhDC )
	{
		pnSamples = 0;
		pcResultDescr = "wglGetCurrentDC() returns NULL";
		return 0;
	}
	if ( ! wglChoosePixelFormatARB )
	{
		pnSamples = 0;
		pcResultDescr = "wglChoosePixelFormatARB isn't supported";
		return 0;
	}

	int attributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,     32,
		WGL_ALPHA_BITS_ARB,     0,
		WGL_DEPTH_BITS_ARB,     24,
		WGL_STENCIL_BITS_ARB,   8,
		WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB,        0,
		0, 0
	};

	int returnedPixelFormat = 0;
	UINT numFormats = 0;
	BOOL bStatus = FALSE;

	if ( pnSamples > 16 ) pnSamples = 16;
	pnSamples = std::max<unsigned>(pnSamples - (pnSamples % 2), 2);

	for (int samples = pnSamples; samples > 0; samples /= 2)
	{
		attributes[17] = samples;

		bStatus = wglChoosePixelFormatARB(lhDC, attributes, 0, 1, &returnedPixelFormat, &numFormats);

		if (bStatus == TRUE && numFormats)
		{
			pnSamples = samples;
			char lcRes[32];
			std::sprintf(lcRes, "%dx MSAA\0", samples);
			pcResultDescr = lcRes;
			return returnedPixelFormat;
		}
	}

	pnSamples = 0;
	pcResultDescr = "Multisample anti-aliasing isn't supported";
	return 0;

}

int GLHelper::getAntiAliasingPixelFormat(unsigned& pnSamples, fglString& pcResultDescr)
{
	if ( isSupported("GL_NV_multisample_coverage") && isSupported("WGL_NV_multisample_coverage"))
		return getCSAAPixelFormat(pnSamples, pcResultDescr);
	else
		return getMSAAPixelFormat(pnSamples, pcResultDescr);
}

//MULTISAMPLING ANTIALIASING
////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
fglRenderStats::fglRenderStats(void)
{
	reset();
}


void fglRenderStats::reset(void)
{
	nBeginTicks = 0.0f;
	nEndTicks = 0.0f;
	nTimeAccum = 0.0f;
	nFramesAccum = 0.0f;
	nTime = 0.0f;
	nFPS = 0.0f;
	nFrameTime = 0.0f;

	nPerfTimerFrequency = 0.0f;
	nTicksPerSecond = CLOCKS_PER_SEC;
	::LARGE_INTEGER lnPerfTimerFrequency;
	if ( ::QueryPerformanceFrequency(&lnPerfTimerFrequency) ) 
	{
		nPerfTimerFrequency = static_cast<double>(lnPerfTimerFrequency.QuadPart);
		nTicksPerSecond = nPerfTimerFrequency ? nPerfTimerFrequency : nTicksPerSecond;
	}
}

const double& fglRenderStats::getTime(void) const
{
	return nTime;
}

const double& fglRenderStats::getFPS(void) const
{
	return nFPS;
}

const double& fglRenderStats::getFrameTime(void) const
{
	return nFrameTime;
}

double fglRenderStats::getIdleTime(void) const
{
	if ( ! nEndTicks ) return 0.0f;
	double lnIdleTime = std::max<double>( (static_cast<double>(::GetTickCount())-nEndTicks) / nTicksPerSecond, 0.0f );
	if (nPerfTimerFrequency)
	{
		::LARGE_INTEGER lnTicks;
		if ( ::QueryPerformanceCounter(&lnTicks) ) 
		{
			lnIdleTime = std::max<double>( (static_cast<double>(lnTicks.QuadPart)-nEndTicks) / nTicksPerSecond, 0.0f );
		}
	}
	return lnIdleTime;
}

void fglRenderStats::beginFrame(void)
{
	nBeginTicks = static_cast<double>(::GetTickCount());
	if (nPerfTimerFrequency)
	{
		::LARGE_INTEGER lnTicks;
		if ( ::QueryPerformanceCounter(&lnTicks) ) 
		{
			nBeginTicks = static_cast<double>(lnTicks.QuadPart); 
		}
	}
	
}

void fglRenderStats::endFrame(void)
{
	double lnDeltaTime = getIdleTime();

	nEndTicks = static_cast<double>(::GetTickCount());
	if (nPerfTimerFrequency)
	{
		::LARGE_INTEGER lnTicks;
		if ( ::QueryPerformanceCounter(&lnTicks) ) 
		{
			nEndTicks = static_cast<double>(lnTicks.QuadPart); 
		}
	}
	
	nFrameTime = std::max<double>( (nEndTicks-nBeginTicks) / nTicksPerSecond, 0.00099f );
	lnDeltaTime += nFrameTime;
	double lnDivFPS = nFPS*nFrameTime;
	if ( nFramesAccum > 24.0f )
	{
		nTimeAccum = 0.0f;
		nFramesAccum = 0.0f;
	}

	nTimeAccum += nFrameTime;
	nFramesAccum += 1.0f;
	
	if ( nTime + lnDeltaTime > fglINFINITE ) { nTime = 0.0f; }
	nTime += lnDeltaTime;
	
	nFPS = ceil(nFramesAccum / nTimeAccum);

}
