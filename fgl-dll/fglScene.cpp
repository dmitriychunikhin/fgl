#include "fglScene.h"
#include "fglImport.h"
#include "fglPM.h"
#include "fglVBO.h"
#include "fglCollider.h"

fglScene* fglScene::oCurrentScene = NULL;
HGLRC fglScene::hCurrentContext = 0;
HDC fglScene::hCurrentDC = 0;

fglUMap<HWND, fglWeakREF<fglScene>> fglScene::aSceneWindows;

GLHelper fglScene::oGLHelper;
const fglTRANSFORM fglScene::identTrans;
const fglMAT4 fglScene::identTransMatrix;

fglViewport* fglScene::oRenderViewport(NULL);
fglCam* fglScene::oRenderCam(NULL);
fglScene::ARENDERLIGHTS fglScene::aRenderLights;
unsigned fglScene::nRenderLights = 0;
fglMaterial* fglScene::oRenderMaterial(NULL);

HGLRC fglScene::hRC = NULL;
int fglScene::nPixelFormat = 0;


void fglScene::call(fglEvent& poEvent)
{
	if (poEvent.name == fglEVENT_logFile) { if (poEvent.nParams) oLog.bind(_GetValue2c(poEvent.aParams[0])); _RetValue2c(oLog.getFilename()); return; }
	if (poEvent.name == fglEVENT_logClear) { oLog.clear(); return; }
	if (poEvent.name == fglEVENT_fontspath) { if (poEvent.nParams) oFontManager.fontspath = _GetValue2c(poEvent.aParams[0]); _RetValue2c(oFontManager.fontspath); return; }
	if (poEvent.name == fglEVENT_vsync) { if (poEvent.nParams) setVSync(_GetValue2l(poEvent.aParams[0])); _RetValue2l(getVSync()); return; }

	if (poEvent.name == fglEVENT_MSAAMode) { if (poEvent.nParams) setMSAAMode(_GetValue2i(poEvent.aParams[0])); _RetValue2i(MSAAMode); return; }
	if (poEvent.name == fglEVENT_MSAAModeInfo) { _RetValue2c(MSAAModeInfo); return; }


	if (poEvent.name == fglEVENT_loadGL) { if (poEvent.nParams) loadGL((HWND)_GetValue2i(poEvent.aParams[0])); return; }

	if (poEvent.name == fglEVENT_isSupported) {  _RetValue2l(0); if (poEvent.nParams) _RetValue2l(oGLHelper.isSupported(_GetValue2c(poEvent.aParams[0]))); return; }

	if (poEvent.name == fglEVENT_aViewports) { _RetValue2i(aViewports->getHFGL()); return; }

	if (poEvent.name == fglEVENT_render) { render(); return; }
	
	if (poEvent.name == fglEVENT_sleep) { if (poEvent.nParams) ::Sleep(_GetValue2i(poEvent.aParams[0])); return; }

	if (poEvent.name == fglEVENT_getFPS) { _RetValue2f(oRenderStats.getFPS()); return; }
	if (poEvent.name == fglEVENT_getFrameTime) { _RetValue2f(oRenderStats.getFrameTime()); return; }
	if (poEvent.name == fglEVENT_getIdleTime) { _RetValue2f(oRenderStats.getIdleTime()); return; }
	if (poEvent.name == fglEVENT_getTime) { _RetValue2f(oRenderStats.getTime()); return; }
	
	if (poEvent.name == fglEVENT_renderLock) { if (poEvent.nParams) { FLAG_RENDERLOCK = _GetValue2l(poEvent.aParams[0]); } _RetValue2l(FLAG_RENDERLOCK);  return; }
	

	fglViewport::call(poEvent);

	if (poEvent.name == fglEVENT_move) 
	{
		oWindowRect = oRect;
		oRect.left = 0;
		oRect.top = 0;
		setWindow();
		return;
	}
}


fglScene::~fglScene(void)
{
	unloadGL();
}


void fglScene::setCurrentContext(void)
{
	//oCurrentScene = this;
	//if (hRC != hCurrentContext )
	//{
	//	hCurrentContext = hRC;
	//	wglMakeCurrent(hDC, hRC);
	//}
	
	oCurrentScene = this;
	if (hDC != hCurrentDC || hRC != hCurrentContext)
	{
		hCurrentContext = hRC;
		hCurrentDC = hDC;
		wglMakeCurrent(hDC, hRC);
	}

}


LRESULT CALLBACK fglScene::wndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	fglUMap<HWND, fglWeakREF<fglScene>>::const_iterator liScene;

	switch(nMsg)
	{
	case WM_PAINT:

		liScene = aSceneWindows.find(hWnd);
		if ( liScene != aSceneWindows.end() ) 
		{
			fglScene* loScene = liScene->second.lock().map();
			if ( loScene )
			{
				loScene->setSceneRenderFlag();
				loScene->render();
			}
			::ValidateRect(hWnd, NULL);
			break;
		}

	default:
		return DefWindowProc(hWnd, nMsg, wParam, lParam);
	}
	return 0;
}


void fglScene::loadGL(HWND phWndParent) 
{
	if (hWnd) throw fglException("OpenGL has been loaded already");

	if (!phWndParent) return;
	

	// Define pixel format
	PIXELFORMATDESCRIPTOR loPfd = {0};
	OSVERSIONINFO loOSV = {0};
	int lnPixelFormat(fglScene::nPixelFormat);
	
	loPfd.nSize = sizeof( loPfd );
	loPfd.nVersion = 1;
	loPfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	loPfd.iPixelType = PFD_TYPE_RGBA;
	loPfd.cColorBits = 32;
	//loPfd.cAlphaBits = 8;
	loPfd.cDepthBits = 24;
	loPfd.cStencilBits = 8;
	loPfd.iLayerType = PFD_MAIN_PLANE;

	loOSV.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&loOSV)) throw fglException("GetVersionEx() failed.");

	// When running under Windows Vista or later support desktop composition.
	if (loOSV.dwMajorVersion > 6 || (loOSV.dwMajorVersion == 6 && loOSV.dwMinorVersion >= 0))
	{
		loPfd.dwFlags |=  PFD_SUPPORT_COMPOSITION;
	}
	///////////////////////////////////////////////
	
	
	phWndParent = GetWindow(phWndParent, GW_CHILD) == NULL ? phWndParent : GetWindow(phWndParent, GW_CHILD); //VFP window with scroll bars creates his own child window

	bool llFirstLoad = (hRC == 0);

	unsigned lnLoadSteps=2; //0 - create default context, 1 - create MSAA context

	for (unsigned liStep = 0; liStep < lnLoadSteps; liStep++)
	{
		if (llFirstLoad) 
		{
			if (hRC)
			{
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hRC);
				hRC = 0;
			}

			unloadGL();
		}

		//Window creation
		hWndParent = phWndParent ? phWndParent : hWndParent;
		if ( ! hWndParent) throw fglException("Parent windows handle is not defined");

		WNDCLASSEX loWC;
		if ( ! ::GetClassInfoExA(::GetModuleHandleA(NULL), "fglWindowClass", &loWC) )
		{
			loWC.cbSize        = sizeof(WNDCLASSEX);
			loWC.style         = CS_OWNDC;
			loWC.lpfnWndProc   = fglScene::wndProc;
			loWC.cbClsExtra    = 0;
			loWC.cbWndExtra    = 0;
			loWC.hInstance     = ::GetModuleHandleA(NULL);
			loWC.hIcon         = NULL;
			loWC.hCursor       = NULL;
			loWC.hbrBackground = NULL;
			loWC.lpszMenuName  = NULL;
			loWC.lpszClassName = "fglWindowClass";
			loWC.hIconSm       = NULL;

			if ( ! ::RegisterClassExA(&loWC) ) throw fglException("RegisterClassExA failed.");
		}

		hWnd = ::CreateWindowA(
				"fglWindowClass",
				NULL,
				WS_CHILD | WS_DISABLED | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				0,0,100,100,
				hWndParent,
				NULL,
				::GetModuleHandleA(NULL),
				NULL);
	
		if ( ! hWnd ) throw fglException("CreateWindowA failed.");
		aSceneWindows[hWnd] = getRef();
		///////////////////////////////////////////////

		if ( ! (hDC = ::GetDC( hWnd )) ) throw fglException("GetDC() failed.");

		if ( ! lnPixelFormat ) lnPixelFormat = ChoosePixelFormat( hDC, &loPfd );

		if ( ! SetPixelFormat( hDC, lnPixelFormat, &loPfd ) ) throw fglException("SetPixelFormat() failed.");
		
		//OpenGL rendering context creation
		if ( ! hRC ) hRC = wglCreateContext( hDC ); //2.1 or less context creation
		
		if ( ! hRC ) throw fglException("OpenGL context creation failed.");
		
		setCurrentContext();

		if (! llFirstLoad)
		{
			break;
		}


		//GLEW init
		GLenum lnGlewRes = glewInit();
		if ( lnGlewRes != GLEW_OK ) throw fglException((char*)glewGetErrorString(lnGlewRes));
		
		if ( ! oGLHelper.nGLVersionMajor )
		{
			if (GLEW_VERSION_2_1) { oGLHelper.nGLVersionMajor=2; oGLHelper.nGLVersionMinor=1; }
			else if (GLEW_VERSION_2_0) { oGLHelper.nGLVersionMajor=2; oGLHelper.nGLVersionMinor=0; }
			else if (GLEW_VERSION_1_5) { oGLHelper.nGLVersionMajor=1; oGLHelper.nGLVersionMinor=5; }
			else if (GLEW_VERSION_1_4) { oGLHelper.nGLVersionMajor=1; oGLHelper.nGLVersionMinor=4; }
			else if (GLEW_VERSION_1_3) { oGLHelper.nGLVersionMajor=1; oGLHelper.nGLVersionMinor=3; }
			else if (GLEW_VERSION_1_2) { oGLHelper.nGLVersionMajor=1; oGLHelper.nGLVersionMinor=2; }
			else if (GLEW_VERSION_1_1) { oGLHelper.nGLVersionMajor=1; oGLHelper.nGLVersionMinor=1; }
		}

		//if (wglCreateContextAttribsARB)
		//{
		//	HGLRC saveRC = hRC;
		//	
		//	int laRCAttribs[] =
		//	{
		//		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		//		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		//		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		//		0, 0
		//	};

		//	hRC = wglCreateContextAttribsARB(hDC, 0, laRCAttribs); //3.1 context creation
		//	if (!hRC)
		//	{
		//		laRCAttribs[3] = 0;
		//		hRC = wglCreateContextAttribsARB(hDC, 0, laRCAttribs); //3.0 context creation
		//	}
		//	if (hRC)
		//	{
		//		oGLHelper.nGLVersionMajor = laRCAttribs[1];
		//		oGLHelper.nGLVersionMinor = laRCAttribs[3];

		//		wglMakeCurrent(NULL, NULL);
		//		wglDeleteContext(saveRC);
		//	}
		//	else
		//	{
		//		hRC = saveRC;
		//	}
		//	saveRC = NULL;

		//	setCurrentContext();
		//}


		///////////////////////////////////////////////

		if ( liStep==0 && MSAAMode )
		{
			//Multisample anti-aliasing
			std::string lcMSAAInfo;
			unsigned lnMSAASamples = MSAAMode;
			lnPixelFormat = oGLHelper.getAntiAliasingPixelFormat(lnMSAASamples, lcMSAAInfo);
			MSAAMode = lnMSAASamples;
			MSAAModeInfo = lcMSAAInfo;
		}
		
		nPixelFormat = lnPixelFormat;
	}



	setVSync(vsync);
	
	//OpenGL state settings 
	glEnable(GL_SCISSOR_TEST);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	if (GLEW_VERSION_3_0)
	{
		glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

		if (GLEW_VERSION_1_2)
		{
			glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
		}
		if (GLEW_VERSION_1_4)
		{
			glEnable(GL_COLOR_SUM);
			glDisable(GL_DITHER);
		}
	}
	if (MSAAMode) glEnable(GL_MULTISAMPLE_ARB);
	
	///////////////////////////////////////////////


	///////////////////////////////////////////////
	//aViewports->reserve(5);

	///////////////////////////////////////////////
	oMaterial = fglBindFactory::createNonPersistent<fglMaterial>();
	oMaterial->setScene(getRef());

	oMaterial->lighting = 1;
	oMaterial->shading = 1;
	oMaterial->twosided = 0;
	oMaterial->wireframe = 0;

	///////////////////////////////////////////////
	oFontManager.setScene(getRef());

	///////////////////////////////////////////////
	oGLHelper.takeCaps();
	o3DPrimitives.build(*this);

	aRenderLights.reserve(oGLHelper.getMaxLights());

	///////////////////////////////////////////////
	setWindow();
	loadDefaultShaders();
	
	///////////////////////////////////////////////
	oRenderStats.reset();


}

void fglScene::unloadGL(void) 
{
	if (!hWnd) return;
	
	if ( aSceneWindows.find(hWnd) != aSceneWindows.end() ) aSceneWindows.erase(hWnd);

	oCurrentScene = NULL;
	wglMakeCurrent( NULL, NULL );
	hCurrentContext = NULL;
	hCurrentDC = NULL;
	
	if (hRC && aSceneWindows.empty()) { wglDeleteContext(hRC); hRC = NULL; nPixelFormat = 0; }
	
	if (hDC) ::ReleaseDC( hWnd, hDC ); hDC=NULL;

	if (hWnd) ::DestroyWindow(hWnd); hWnd=NULL;
	if (aSceneWindows.empty()) { ::UnregisterClassA("fglWindowClass", ::GetModuleHandleA(NULL)); }

}

bool fglScene::getVSync(void) const
{
	return vsync;
}

void fglScene::setVSync(const bool& plVSync)
{
	vsync = plVSync;
	oGLHelper.setVSync(plVSync);
}

void fglScene::setMSAAMode(const unsigned& pnMSAAMode)
{
	//if (hRC) throw fglException("MSAAMode has to be set before LoadGL method call.");
	if (pnMSAAMode<0 || pnMSAAMode > 16) throw fglException("Property MSAAMode accepts values between 0 and 16.");
	MSAAMode = pnMSAAMode;
}

unsigned fglScene::getMSAAMode(fglString* pcMSAAModeInfo)
{
	if (pcMSAAModeInfo) *pcMSAAModeInfo = MSAAModeInfo;
	return MSAAMode;
}

void fglScene::setRenderFlag(const bool& plValue)
{
	FLAG_RENDER = plValue;
}

void fglScene::setWindow(void)
{
	::SetWindowPos(hWnd, 0, oWindowRect.left, oWindowRect.top, oWindowRect.width, oWindowRect.height, SWP_NOACTIVATE); //SWP_NOZORDER | SWP_NOACTIVATE
	setSceneRenderFlag();
}


void fglScene::render(void)
{
	if ( ! hRC ) return;
	if (  ! FLAG_RENDER ) return;
	if ( FLAG_RENDERLOCK ) return;

	FLAG_RENDER = false;
	

	setCurrentContext();

	oRenderStats.beginFrame();
	////////////////////////////////////////
	fglViewport::render();

	if (oMaterial.exist()) oMaterial->bind();

	static fglCollection<fglViewport>::iterator iViewport, iViewportEnd;
	static fglViewport* loViewport;
	iViewport = aViewports->begin();
	iViewportEnd = aViewports->end();
	for( ; iViewport!=iViewportEnd; ++iViewport ) 
	{
		loViewport = iViewport->map();
		if ( loViewport == this ) continue;
		if ( loViewport ) (*iViewport)->render();
	}
	
	if (oMaterial.exist()) oMaterial->unbind();

	::SwapBuffers( hDC );
	/////////////////////////////////////////
	oRenderStats.endFrame();

}


void fglScene::loadDefaultShaders(void)
{
	return;
	fglString lcShader;
	lcShader = "#version 110\
	attribute vec3 InVertex; \
	attribute vec3 InNormal; \
	uniform int lightCount; \
	uniform float lightRadius[8];\
	\
	varying vec3 lightDir[8];\
	varying vec3 viewDir;\
	varying vec3 normal;\
	\
	void main()\
	{\
		vec3 vertexPos = vec3(gl_ModelViewMatrix * gl_Vertex);\
		\
		for(int i=0; i < lightCount; i++)\
		{\
			lightDir[i] = (gl_LightSource[i].position.xyz - vertexPos) / lightRadius[i];\
		}\
		viewDir = -vertexPos;\
		normal = normalize(gl_NormalMatrix * gl_Normal);\
		\
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
		gl_TexCoord[0] = gl_MultiTexCoord0;  \
	}";
	
	oDefaultShaderVert->build(fglShader::VERTEX, lcShader);

	lcShader = "#version 110\
	uniform sampler2D colorMap;\
	uniform int lightCount; \
	\
	varying vec3 lightDir[8];\
	varying vec3 viewDir;\
	varying vec3 normal;\
	\
	void main()\
	{\
		for(int i=0; i < lightCount; i++)\
		{\
			lightDir[i] = (gl_LightSource[i].position.xyz - vertexPos) / lightRadius[i];\
		}\
		viewDir = -vertexPos;\
		normal = normalize(gl_NormalMatrix * gl_Normal);\
		\
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
		gl_TexCoord[0] = gl_MultiTexCoord0;  \
	}";
	
	oDefaultShaderFrag->build(fglShader::FRAGMENT, lcShader);

	oDefaultShaderProg->attach(oDefaultShaderVert.callBindREFGet());
	oDefaultShaderProg->attach(oDefaultShaderFrag.callBindREFGet());
	oDefaultShaderProg->build();

}

