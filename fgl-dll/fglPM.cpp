#include "fglPM.h"
#include "fglScene.h"

class CmpVec
{
private:
	float eps_{ FLT_MIN };

public:

	CmpVec() = default;
	CmpVec(float _eps) : eps_(_eps) {}

	bool operator()(const OpenMesh::Vec3f& _v0, const OpenMesh::Vec3f& _v1) const
	{
		if (fabs(_v0[0] - _v1[0]) <= eps_)
		{
			if (fabs(_v0[1] - _v1[1]) <= eps_)
			{
				return (_v0[2] < _v1[2] - eps_);
			}
			else return (_v0[1] < _v1[1] - eps_);
		}
		else return (_v0[0] < _v1[0] - eps_);
	}

};



void fglPM::Build(fglMesh* poMeshIn, fglMesh* poMeshOut, int target_num_faces)
{

	oMeshIn = poMeshIn;
	oMeshOut = poMeshOut;
	if (oMeshIn->nIndsPerFace()!=3) {return;}

	read_data();
	
	if (oMeshIn->nIndsPerFace()==3)
	{
		Decimater loDecimater(oMesh);

		loDecimater.add( hModQuadric );

		loDecimater.initialize();
        
		float lnChange = float( target_num_faces / poMeshIn->nFaces() );
		loDecimater.decimate_to_faces(0, target_num_faces);
		oMesh.garbage_collection();
	}
	
	write_data();
}

void fglPM::read_data(void)
{

	oMesh.clear();
	oMesh.request_face_normals();
	oMesh.request_vertex_normals();

	OpenMesh::IO::ImporterT<Mesh> importer(oMesh);
	importer.prepare();

	CmpVec comp;
	std::map<OpenMesh::Vec3f, Mesh::VertexHandle, CmpVec>            vMap(comp);
	std::map<OpenMesh::Vec3f, Mesh::VertexHandle, CmpVec>::iterator  vMapIt;

	OpenMesh::Vec3f v;
	int cur_idx=0;
	for (unsigned i = 0; i < oMeshIn->aVerts.size(); i++)
	{
		v[0] = oMeshIn->aVerts[i].p.x;
		v[1] = oMeshIn->aVerts[i].p.y; 
		v[2] = oMeshIn->aVerts[i].p.z;
		if ((vMapIt=vMap.find(v)) == vMap.end())
		{
			importer.add_vertex(v);
			vMap[v] = OpenMesh::VertexHandle(cur_idx++);
		}

	}
	
	OpenMesh::IO::BaseImporter::VHandles face_vhandles;
	for (unsigned i = 0; i < oMeshIn->nFaces(); i++)
	{
		face_vhandles.clear();
		for (unsigned j = 0; j < oMeshIn->nIndsPerFace(); j++)
		{
			v[0] = oMeshIn->aVerts[oMeshIn->aFaces(i).aInds[j]].p.x;
			v[1] = oMeshIn->aVerts[oMeshIn->aFaces(i).aInds[j]].p.y; 
			v[2] = oMeshIn->aVerts[oMeshIn->aFaces(i).aInds[j]].p.z;
			
			face_vhandles.push_back(vMap[v]);
		}
		importer.add_face(face_vhandles);
	}
	importer.finish();
	oMesh.update_normals();
}

void fglPM::write_data(void)
{
	oMeshOut->aVerts.set_size(oMesh.n_vertices());
	Mesh::VertexIter iVert=oMesh.vertices_begin(), iVertEnd=oMesh.vertices_end();
	int i,j;
	for (; iVert != iVertEnd; ++iVert)
	{
		i = iVert.handle().idx();
		oMeshOut->aVerts[i].p.x = oMesh.point(iVert)[0];
		oMeshOut->aVerts[i].p.y = oMesh.point(iVert)[1];
		oMeshOut->aVerts[i].p.z = oMesh.point(iVert)[2];
		oMeshOut->aVerts[i].n.x = oMesh.normal(iVert.handle())[0];
		oMeshOut->aVerts[i].n.y = oMesh.normal(iVert.handle())[1];
		oMeshOut->aVerts[i].n.z = oMesh.normal(iVert.handle())[2];
	}
	
	oMeshOut->setFaces(oMesh.n_faces(), oMeshIn->nIndsPerFace());
	Mesh::FaceIter iFace=oMesh.faces_begin(), iFaceEnd = oMesh.faces_end();
	for (; iFace!=iFaceEnd; ++iFace)
	{
		i = iFace.handle().idx();
		j=0;
		Mesh::FaceVertexIter iFV, iFVEnd = oMesh.fv_end(iFace.handle());
		for (iFV=oMesh.fv_begin(iFace.handle()); iFV!=iFVEnd; ++iFV) 
		{
			oMeshOut->aFaces(i).aInds[j] = iFV.handle().idx();
			j++;
		}
	}

	oMeshOut->aVertBones.clear();
	oMeshOut->aBones.clear();
}

