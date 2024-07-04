#ifndef _FGLLOG_H_
#define _FGLLOG_H_

#include "common.h"
#include "fgl.h"
#include "fglMutex.h"

class fglException;

class fglLog
{
private:
	std::fstream out;
	fglString filename{ "" };

	enum levels {
		_OFF=0, 
		_FATAL, /*Severe errors that cause premature termination. Expect these to be immediately visible on a status console.*/
		_ERROR, /*Other runtime errors or unexpected conditions. Expect these to be immediately visible on a status console.*/
		_WARN, /*Use of deprecated APIs, poor use of API, 'almost' errors, other runtime situations that are undesirable or unexpected, but not necessarily "wrong". Expect these to be immediately visible on a status console*/
		_INFO, /*Interesting runtime events (startup/shutdown). Expect these to be immediately visible on a console, so be conservative and keep to a minimum.*/
		_DEBUG /*Detailed information on the flow through the system. Expect these to be written to logs only.*/
	};

	fglMutex oMutex;
	
public:
	fglLog(void) { out.exceptions(std::fstream::goodbit); }
	//bad for perf virtual ~fglLog(void) {}

	fglString& getFilename(void) { return filename; }

	void bind(const std::fstream& pOut) { oMutex.lock(); filename=""; out.copyfmt(pOut); oMutex.unlock(); }
	void bind(const fglString& pcPath) { oMutex.lock(); filename=pcPath; out.open(filename, std::fstream::out | std::fstream::app); oMutex.unlock(); }
	void clear(void); 

	void write(const fglString& pcMsg, int pnLevel);
	void write(const fglException& poErr, int pnLevel);

	void debug(const fglString& pcMsg) { write(pcMsg, _DEBUG); }
	void info(const fglString& pcMsg) { write(pcMsg, _INFO); }
	void warn(const fglString& pcMsg) { write(pcMsg, _WARN); }
	void error(const fglString& pcMsg) { write(pcMsg, _ERROR); }
	void fatal(const fglString& pcMsg) { write(pcMsg, _FATAL); }

	void debug(const fglException& poErr) { write(poErr, _DEBUG); }
	void info(const fglException& poErr) { write(poErr, _INFO); }
	void warn(const fglException& poErr) { write(poErr, _WARN); }
	void error(const fglException& poErr) { write(poErr, _ERROR); }
	void fatal(const fglException& poErr) { write(poErr, _FATAL); }

};


class fglException : public std::exception
{
private:
	DWORD nError;
	fglString cMsgA;
	fglWString cMsgW;
	void setMessage(void);

public:
	fglException(void) { nError = ::GetLastError(); setMessage(); }
	fglException(fglString pcErr) { nError = 0; cMsgA = pcErr; }
	fglException(fglWString pcErr) { nError = 0; cMsgW = pcErr; }
	//bad for perf virtual ~fglException(void) {}

	DWORD getError(void) const { return nError;}

	const fglString& getMessageA(void) const { return cMsgA; }
	const fglWString& getMessageW(void) const { return cMsgW; }

	virtual const char* what(void) const
	{
		fglString lcMsg = getMessageA();
		return lcMsg.c_str();
	};

};

#endif
