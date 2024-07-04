#include "pro_ext2.h"

void _UnpackFoxStr(fglString& pcDest, const Value& pcVal)
{
	char FAR * lcStr;
	_HLock(pcVal.ev_handle);
	lcStr = (char FAR *)_HandToPtr(pcVal.ev_handle);
	if (pcVal.ev_length==0) 
		pcDest.clear();
	else 
		pcDest.assign(lcStr, pcVal.ev_length);

	_HUnLock(pcVal.ev_handle);
}

int _GetValue2i(const Value& pValue)
{
	if (pValue.ev_type=='I') return pValue.ev_long;
	if (pValue.ev_type=='N') return (int) pValue.ev_real;
	return 0;
}

unsigned int _GetValue2ui(const Value& pValue)
{
	if (pValue.ev_type=='I') return (unsigned int) pValue.ev_long;
	if (pValue.ev_type=='N') return (unsigned int) pValue.ev_real;
	return 0;
}


float _GetValue2f(const Value& pValue)
{
	if (pValue.ev_type=='N') return (float) pValue.ev_real;
	if (pValue.ev_type=='I') return (float) pValue.ev_long;
	return 0.0f;
}

bool _GetValue2l(const Value& pValue)
{
	if (pValue.ev_type=='L') return pValue.ev_length == 1 ? true:false;
	return _GetValue2ui(pValue) ? true:false;
}

fglString _GetValue2c(const Value& pValue)
{
	fglString lcRet{ "" };
	if (pValue.ev_type=='C') _UnpackFoxStr(lcRet, pValue);
	return lcRet;
}


bool _GetObjectProperty2(Value& pvRetval, Value& poObj, char* pcProp)
{
	memset( &pvRetval, 0, sizeof( Value ) );
	return (_GetObjectProperty(&pvRetval, &poObj, pcProp)==0) ? true : false;
}

int _GetObjectProperty2i(Value& poObj, char* pcProp)
{
	Value lvProp;
	if ( ! _GetObjectProperty2(lvProp, poObj, pcProp) ) return 0;
	return _GetValue2i(lvProp);
}

unsigned int _GetObjectProperty2ui(Value& poObj, char* pcProp)
{
	Value lvProp;
	if (!_GetObjectProperty2(lvProp, poObj, pcProp)) return 0;
	return _GetValue2ui(lvProp);
}

float _GetObjectProperty2f(Value& poObj, char* pcProp)
{
	Value lvProp;
	if (!_GetObjectProperty2(lvProp, poObj, pcProp)) return 0;
	return _GetValue2f(lvProp);
}

fglString _GetObjectProperty2c(Value& poObj, char* pcProp)
{
	Value lvProp;
	if (!_GetObjectProperty2(lvProp, poObj, pcProp)) return fglString("");
	return _GetValue2c(lvProp);
}

fglVECTOR _GetObjectProperty2vec(Value& poObj)
{
	fglVECTOR loVector;
	loVector.x = _GetObjectProperty2f(poObj, "x");
	loVector.y = _GetObjectProperty2f(poObj, "y");
	loVector.z = _GetObjectProperty2f(poObj, "z");
	return loVector;
}

fglQUAT _GetObjectProperty2quat(Value& poObj)
{
	fglQUAT loQuat;
	loQuat.a = _GetObjectProperty2f(poObj, "a");
	loQuat.v.x = _GetObjectProperty2f(poObj, "x");
	loQuat.v.y = _GetObjectProperty2f(poObj, "y");
	loQuat.v.z = _GetObjectProperty2f(poObj, "z");
	return loQuat;
}

fglROW4 _GetObjectProperty2row4(Value& poObj)
{
	fglROW4 loRow4;
	loRow4.a1 = _GetObjectProperty2f(poObj, "a1");
	loRow4.a2 = _GetObjectProperty2f(poObj, "a2");
	loRow4.a3 = _GetObjectProperty2f(poObj, "a3");
	loRow4.a4 = _GetObjectProperty2f(poObj, "a4");
	return loRow4;
}


int _SetObjectProperty2(Value& poObj, char* pcProp, Value& poVal)
{
	if ( poObj.ev_type != 'O' ) return 1;
	return _SetObjectProperty(&poObj, pcProp, &poVal,1);
}

void _SetObjectProperty2i(Value& poObj, char* pcProp, const long& pnVal)
{
	Value lvProp;
	memset( &lvProp, 0, sizeof( lvProp ) );
	lvProp.ev_type = 'I';
	lvProp.ev_width = 10;
	lvProp.ev_long = pnVal;
	_SetObjectProperty2(poObj, pcProp, lvProp);
}

void _SetObjectProperty2f(Value& poObj, char* pcProp, const float& pnVal)
{
	Value lvProp;
	memset( &lvProp, 0, sizeof( lvProp ) );
	lvProp.ev_type = 'N';
	lvProp.ev_width = 16;
	lvProp.ev_length = 5;
	lvProp.ev_real = pnVal;
	_SetObjectProperty2(poObj, pcProp, lvProp);
}

void _SetObjectProperty2vec(Value& poObj, const fglVECTOR& poVector)
{
	_SetObjectProperty2f(poObj, "x", poVector.x);
	_SetObjectProperty2f(poObj, "y", poVector.y);
	_SetObjectProperty2f(poObj, "z", poVector.z);
}

void _SetObjectProperty2quat(Value& poObj, const fglQUAT& poQuat)
{
	_SetObjectProperty2f(poObj, "a", poQuat.a);
	_SetObjectProperty2f(poObj, "x", poQuat.v.x);
	_SetObjectProperty2f(poObj, "y", poQuat.v.y);
	_SetObjectProperty2f(poObj, "z", poQuat.v.z);
}


void _RetValue2i(const long& ival) { _RetInt(ival, 15); }
void _RetValue2f(const double& flt) { _RetFloat(flt, 20,8); }
void _RetValue2c(const fglString& str) { _RetChar((char*)str.c_str()); }
void _RetValue2l(const int& bval) { _RetLogical(bval); }
void _RetValue2null(void) 
{ 
	Value lvProp;
	memset( &lvProp, 0, sizeof( lvProp ) );
	lvProp.ev_type = '0';
	lvProp.ev_long = 'L';
	_RetVal(&lvProp);
}

float _GetValue2Infinite(const float& pValue)
{
	return pValue<0 ? fglINFINITE : pValue;
}

float _RetValue2Infinite(const float& pValue)
{
	return pValue==fglINFINITE ? -1 : pValue;
}
