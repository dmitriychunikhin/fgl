#ifndef _FGLIMPORT_H_
#define _FGLIMPORT_H_

#include "fglModel.h"

void fgl_import_model(fglModel* poModel, const fglString& pcPath);

void fgl_import_mesh_from_string(fglMesh& poMesh, const fglString& pcData, const fglString& pcFormat); //pcFormat is file extension without dot (for example "lwo")

#endif
