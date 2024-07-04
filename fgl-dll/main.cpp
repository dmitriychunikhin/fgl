#include "fglBind.h"
#include "fglSceneGraph.h"
#include "fglViewport.h"
#include "fglCam.h"
#include "fglModel.h"
#include "fglScene.h"


void _fastcall fgl_onload(void)
{
	/////////////////////////////////////////////////////////////////////////
	DWORD_PTR lnProcessAffinityMask = 0;
	DWORD_PTR lnSystemAffinityMask = 0;
	HANDLE lhCurrentProcess = ::GetCurrentProcess();
	HANDLE lhCurrentThread = ::GetCurrentThread();
	
	if (lhCurrentProcess == INVALID_HANDLE_VALUE) return;
	if (lhCurrentThread == INVALID_HANDLE_VALUE) { ::CloseHandle(lhCurrentProcess); return; }

	if ( ::GetProcessAffinityMask(lhCurrentProcess, &lnProcessAffinityMask, &lnSystemAffinityMask) ) 
	{
		if (lnProcessAffinityMask)
		{
			// Get the lowest processor
			DWORD_PTR lnAffinityMask = (lnProcessAffinityMask & ((~lnProcessAffinityMask) + 1));
			::SetThreadAffinityMask(lhCurrentThread, lnAffinityMask);
			//::SetProcessAffinityMask(lhCurrentProcess, lnAffinityMask);
		}
	}

	::CloseHandle(lhCurrentThread);

	::SetPriorityClass(lhCurrentProcess, REALTIME_PRIORITY_CLASS);
	::CloseHandle(lhCurrentProcess);
	/////////////////////////////////////////////////////////////////////////
}

void _fastcall fgl_onunload(void)
{
	fglBindFactory::unload();
}

void _fastcall fgl_newobject(ParamBlk* paParams)
{
	_RetValue2i(0);
	fglREF<fglBind> loFglObj;

	fglString lcErr{ "" };
	try 
	{
		fglString lcClassname;
		lcClassname = _GetValue2c(paParams->p[0].val);

		loFglObj = fglBindFactory::createPersistentByClassName(lcClassname);
		
		if (loFglObj.empty()) throw fglException("Invalid classname "+lcClassname); 

		const fglREF<fglScene>& loScene = fglBind::getBind<fglScene>(_GetValue2i(paParams->p[1].val));
		if (loScene.exist()) loFglObj->setScene(loScene); 
				
		if ( loFglObj->getScene().empty() ) throw fglException("Scene object is not defined"); 

		_RetValue2i(loFglObj->getHFGL());
	}
	catch (fglException& loErr) 
	{ 
		lcErr = loErr.getMessageA(); 
	} 
	catch (...) 
	{ 
		lcErr = "Unknown error"; 
	}

	if ( ! lcErr.empty() ) 
	{
		loFglObj.release();
		_UserError(const_cast<char*>(lcErr.c_str()));
	}
}


void _fastcall fgl_removeobject(ParamBlk* paParams)
{
	if ( ! paParams->pCount ) return;

	////////////////////////////////////////

	fglREF<fglBind> loRef;
	loRef.inst(fglBind::getBind<fglBind>(_GetValue2i(paParams->p[0].val)));

	fglString lcErr{ "" };

	if (loRef.exist())
	{
		try 
		{ 
			fglBindFactory::releaseBind(loRef);
		}
		catch (fglException& loErr)  
		{ 
			lcErr = loErr.getMessageA(); 
		} 
		catch (...) 
		{ 
			lcErr = "Unknown error"; 
		}
	}

	////////////////////////////////////////
	
	if ( ! lcErr.empty() ) _UserError(const_cast<char*>(lcErr.c_str()));
}


void _fastcall fgl_call(ParamBlk* paParams) 
{
	_RetLogical(0);
	
	int lnParams; //number of parameters passed (result of PCOUNT())
	if ( !(lnParams = _GetValue2i(paParams->p[2].val)) ) return;
	lnParams--; //exclude "tnName" parameter (p[1]);
	
	const fglREF<fglBind>& loFglObj = fglBind::getBind<fglBind>(_GetValue2i(paParams->p[0].val));
	if ( loFglObj.empty()  ) return;

	
	////////////////////////////////////////
	
	int lnName = _GetValue2i(paParams->p[1].val);

	fglEventParams laParams(NULL,0);
	laParams.nParams = paParams->pCount-3;
	laParams.pParameters = (laParams.nParams != 0) ? &paParams->p[3] : NULL;
	
	fglEvent loEvent(lnName, laParams, lnParams);
	loEvent.name = lnName;
	loEvent.aParams = laParams;
	loEvent.nParams = lnParams;

	fglString lcErr{ "" };
	
	fglScene* loScene;
	if ( loScene = loFglObj->getScene().lock().map() ) loScene->setCurrentContext();
	 
	try 
	{ 
		loFglObj->call( loEvent ); 
	}
	catch (fglException& loErr) 
	{ 
		lcErr = loErr.getMessageA(); 
	} 
	catch (...) 
	{ 
		lcErr = "Unknown error."; 
	}

	////////////////////////////////////////

	if ( ! lcErr.empty() ) _UserError(const_cast<char*>(lcErr.c_str()));
}


FoxInfo myFoxInfo[] = {
	{"fgl_onload",(FPFI) fgl_onload, CALLONLOAD, ""},
	{"fgl_onunload",(FPFI) fgl_onunload, CALLONUNLOAD, ""},
	{"fgl_newobject",(FPFI) fgl_newobject, 10, ".?,.?,.?,.?,.?,.?,.?,.?,.?,.?"},
	{"fgl_removeobject",(FPFI) fgl_removeobject, 1, ".?"},
	{"fgl_call",(FPFI) fgl_call, 11, ".?,.?,.?,.?,.?,.?,.?,.?,.?,.?,.?"},
};

extern "C" {
// the FoxTable structure
FoxTable _FoxTable = {
	(FoxTable*) 0, sizeof(myFoxInfo)/sizeof(FoxInfo), myFoxInfo
};
}

