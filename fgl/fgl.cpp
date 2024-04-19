#include "fgl.h"

WCHAR lcWcharBuf[1024];

int _CharToWChar(WCHAR** pcDest, const char* pcSrc, int pnLen)
{
	if (pnLen<=0 || pnLen>=1024) return 0;
	*pcDest = lcWcharBuf;
	memset(*pcDest, 0, (pnLen+1)*sizeof(WCHAR));
	return MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pcSrc, pnLen, *pcDest, pnLen);
}

int _CharToWChar(WCHAR** pcDest, const fglString& pcSrc)
{
	return _CharToWChar(pcDest, pcSrc.c_str(), pcSrc.length());
}

int _CharToWChar(fglWString& pcDest, const fglString& pcSrc)
{
	WCHAR* lcBuf;
	int lnRes = _CharToWChar(&lcBuf, pcSrc);
	if (lnRes) pcDest = lcBuf; else pcDest = L"";
	return lnRes;
}

float fglSQR(const float& pnVal)
{
	return pnVal*pnVal;
}