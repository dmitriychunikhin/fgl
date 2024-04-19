#ifndef _FGLPM_H_
#define _FGLPM_H_

#include "common.h"
#include "fgl3DGen.h"
#include "fglMesh.h"

#undef min
#undef max
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModAspectRatioT.hh>
#include <OpenMesh/Tools/Decimater/ModEdgeLengthT.hh>
#include <OpenMesh/Tools/Decimater/ModHausdorffT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModIndependentSetsT.hh>
#include <OpenMesh/Tools/Decimater/ModRoundnessT.hh>

#pragma comment (lib,"libOpenMeshCore.lib")
#pragma comment (lib,"libOpenMeshTools.lib")

class fglPM
{
public:

	// Mesh type
	typedef OpenMesh::TriMesh_ArrayKernelT< > Mesh;

	// Decimater type
	typedef OpenMesh::Decimater::DecimaterT< Mesh > Decimater;

	// Decimation Module Handle type
	typedef OpenMesh::Decimater::ModQuadricT< Mesh >::Handle HModQuadric;

	Mesh oMesh;             
	HModQuadric hModQuadric;

	//class FGLReader : public OpenMesh::IO::BaseReader
	//{
	//public:
	//	FGLReader(void) {/*OpenMesh::IO::IOManager().register_module(this);*/}
	//	//bad for perf virtual ~FGLReader(void) {};
	//	fglString get_description() const { return "FGL internal Format"; }
	//	fglString get_extensions() const { return "fgl"; }
	//	bool read(fglMesh* poMesh, OpenMesh::IO::BaseImporter& _bi);
	//};


	fglPM(void) = default;
	//bad for perf virtual ~fglPM(void);

	void read_data(void);
	void write_data(void);

	void Build(fglMesh* poMeshIn, fglMesh* poMeshOut, int target_num_faces);


	fglMesh* oMeshIn{ NULL };
	fglMesh* oMeshOut{ NULL };
};


#endif
