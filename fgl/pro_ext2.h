#ifndef _PRO_EXT2_H_
#define _PRO_EXT2_H_

#include "common.h"
#include <pro_ext.h>
#include "fglMem.h"
#include "fgl3DGen.h"

void _UnpackFoxStr(fglString& pcDest, const Value& pcVal);

int _GetValue2i( const Value& pValue );
unsigned int _GetValue2ui ( const Value& pValue );
float _GetValue2f(const Value& pValue);
bool _GetValue2l(const Value& pValue);
fglString _GetValue2c(const Value& pValue);

bool _GetObjectProperty2(Value& pvRetval, Value& poObj, char* pcProp);
int _GetObjectProperty2i(Value& poObj, char* pcProp);
unsigned int _GetObjectProperty2ui(Value& poObj, char* pcProp);
float _GetObjectProperty2f(Value& poObj, char* pcProp);
fglString _GetObjectProperty2c(Value& poObj, char* pcProp);
fglVECTOR _GetObjectProperty2vec(Value& poObj);
fglQUAT _GetObjectProperty2quat(Value& poObj);
fglROW4 _GetObjectProperty2row4(Value& poObj);

int _SetObjectProperty2(Value& poObj, char* pcProp, Value& poVal);
void _SetObjectProperty2i(Value& poObj, char* pcProp, const long& pnVal);
void _SetObjectProperty2f(Value& poObj, char* pcProp, const float& pnVal);
void _SetObjectProperty2vec(Value& poObj, const fglVECTOR& poVector);
void _SetObjectProperty2quat(Value& poObj, const fglQUAT& poQUAT);

void _RetValue2i(const long& ival);
void _RetValue2f(const double& flt);
void _RetValue2c(const fglString& str);
void _RetValue2l(const int& bval);
void _RetValue2null(void);

float _GetValue2Infinite(const float& pValue);
float _RetValue2Infinite(const float& pValue);

#endif
