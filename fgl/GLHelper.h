#ifndef _GLHELPER_H_
#define _GLHELPER_H_

#include "common.h"
#include "fgl.h"

class fglScene;

class GLHelper 
{
	friend fglScene;
private:
	unsigned nMaxLights{ 0 };
	unsigned nMaxElementsVertices{ 1000000 };
	unsigned nMaxElementsIndices{ 100000 };
	unsigned nGLVersionMajor{ 0 };
	unsigned nGLVersionMinor{ 0 };

public:
	GLHelper(void) = default;
	//bad for perf virtual ~GLHelper(void);
	
	bool isSupported(const fglString& pcName) { return glewIsSupported(pcName.c_str()) ? true : false; }

	void takeCaps(void);

	inline unsigned getMaxLights(void) { return nMaxLights; }
	inline unsigned getMaxElementsVertices(void) { return nMaxElementsVertices; }
	inline unsigned getMaxElementsIndices(void) { return nMaxElementsIndices; }
	inline unsigned getGLVersionMajor(void) { return nGLVersionMajor; }
	inline unsigned getGLVersionMinor(void) { return nGLVersionMinor; }

	void resetLights(void);

	void setVSync(bool plEnable);

	// Returns a hardware accelerated anti-aliasing (MSAA or CSAA) pixel format for current device and rendering context
	// that supports the specified samples per pixel. Functions selects minimum available pixel format and returns it in pnSamples
	// If a matching MSAA or CSAA pixel format was found it will be returned, otherwise '0' will be returned.
	int getAntiAliasingPixelFormat(unsigned& pnSamples, fglString& pcResultDescr);
};


class fglRenderStats
{
private:
	double nPerfTimerFrequency{ 0.0f };
	double nTicksPerSecond{ CLOCKS_PER_SEC };

	double nBeginTicks{ 0.0f };
	double nEndTicks{ 0.0f };
	double nTimeAccum{ 0.0f };
	double nFramesAccum{ 0.0f };

	double nTime{ 0.0f };
	double nFPS{ 0.0f };
	double nFrameTime{ 0.0f };
public:

	fglRenderStats(void);
	//bad for perf virtual ~fglRenderStats(void);

	const double& getTime(void) const;
	const double& getFPS(void) const;
	const double& getFrameTime(void) const;
	double getIdleTime(void) const;
	
	void reset(void);
	void beginFrame(void);
	void endFrame(void);

};

#endif