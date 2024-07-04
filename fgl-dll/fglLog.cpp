#include "fglLog.h"

void fglLog::clear(void)
{ 
	if (filename.empty()) return;
	oMutex.lock(); 
	out.close();
	out.open(filename, std::fstream::out | std::fstream::trunc);
	out.close();
	out.clear();
	oMutex.unlock(); 
	bind(filename);
}


void fglLog::write(const fglString& pcMsg, int pnLevel) 
{ 
	if ( ! out.is_open() ) return;

	fglString lcLevel;
	switch (pnLevel) 
	{
	case _DEBUG: lcLevel = "DEBUG: "; break;
	case _INFO: lcLevel = "INFO: "; break;
	case _WARN: lcLevel = "WARN: "; break;
	case _ERROR: lcLevel = "ERR: "; break;
	case _FATAL: lcLevel = "FATAL: "; break;
	default: lcLevel = "";
	};

	oMutex.lock(); 
	out << lcLevel << pcMsg << std::endl; 
	oMutex.unlock(); 
}

void fglLog::write(const fglException& poErr, int pnLevel) 
{ 
	write(poErr.getMessageA(), pnLevel); 
}

void fglException::setMessage(void)
{
	LPSTR lcMsgA=NULL;
	DWORD lnLenA = ::FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		getError(),
		0,
		(LPSTR)&lcMsgA,
		0,
		NULL);

	cMsgA.clear();
	cMsgA.copy(lcMsgA,lnLenA);
	::LocalFree(lcMsgA);

	LPWSTR lcMsgW=NULL;
	DWORD lnLenW = ::FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		getError(),
		0,
		(LPWSTR)&lcMsgW,
		0,
		NULL);
	
	cMsgW.clear();
	cMsgW.copy(lcMsgW,lnLenW);
	::LocalFree(lcMsgW);
}
