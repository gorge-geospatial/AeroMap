// Cube.cpp
// Class to create and manage cube geometry
//

#include <assert.h>

#include "Logger.h"
#include "Cube.h"

// order the 2nd and 3rd vertices of a triangle based on the winding order
#define WIND(v1, v2) (mb_WindCW ? v1 : v2), (mb_WindCW ? v2 : v1)

#define MAX_TESS 4		// maximum # of tesselation levels

Cube::Cube()
	: Primitive()
	, m_Size(1.0, 1.0, 1.0)
{
}

Cube::~Cube()
{
}

void Cube::Render()
{
	if (m_VB.IsValid())
		m_VB.Render();
}

void Cube::Create(VertexBufferGL::VertexType vertexType, VEC3 size, int tessLevel, bool windCW)
{
	//	create a cube for opengl rendering by tesselating
	//	base cube

	//	inputs:
	//		vertexType	= type of vertex buffer to create for rendering
	//		size		= size of generated cube, along x, y, z axes
	//		tessLevel	= # of times to subdivide base (low res) sphere
	//					  (accepts 0 and just use default base (no tesselation))
	//		windCW		=
	//

	if (tessLevel > MAX_TESS)
		tessLevel = MAX_TESS;

	m_Size = size;
	mb_WindCW = windCW;

	CreateBase();

	// these vectors are inputs to the process (m_* are outputs)
	std::vector<FaceType> faceList;

	// init inputs from base
	m_faceTess.assign( m_faceBase.cbegin(), m_faceBase.cend() );
	m_vxTess.assign( m_vxBase.cbegin(), m_vxBase.cend() );

	// for each tesselation level
	for (int i = 0; i < tessLevel; ++i)
	{
		// move output face list to input (vertex list just grows)
		faceList.assign( m_faceTess.cbegin(), m_faceTess.cend() );
		// clear output face list (creating all new faces)
		m_faceTess.clear();

		// for each input face
		for (unsigned int faceIdx = 0; faceIdx < faceList.size(); ++faceIdx)
		{
			FaceType* pFace = &faceList[faceIdx];

			/*
				subdivide()

						  Vx0
						   +
						  / \
						 /   \         - each triangle subdivides into 4 triangles
						/  0  \
				   VxC /-------\ vxA
					  / \  3  / \
					 /   \   /   \
					/ 2   \ /  1  \
				Vx2 +---------------+ Vx1
						  VxB

				- bisect each side of current face to create 3 new vx

				- add new vertices to vertex list

				- reuse vertices and indices of vertices created on prior calls to subdivide
				(otherwise you create 2 new vx and 2 new indices every time you bisect a line!)
			*/
				
			// side 1
			int vxIndexA = FindVertex( faceList, pFace->vx0, pFace->vx1 );
			if (vxIndexA == -1)
			{
				// vertex that bisects this side not found, create it
				VEC3 vxA = (m_vxTess[pFace->vx0] + m_vxTess[pFace->vx1]) * 0.5F;
				m_vxTess.push_back( vxA );
				vxIndexA = (int)m_vxTess.size()-1;
			}
			pFace->vx01 = vxIndexA;
			
			// side 2
			int vxIndexB = FindVertex( faceList, pFace->vx1, pFace->vx2 );
			if (vxIndexB == -1)
			{
				VEC3 vxB = (m_vxTess[pFace->vx1] + m_vxTess[pFace->vx2]) * 0.5F;
				m_vxTess.push_back( vxB );
				vxIndexB = (int)m_vxTess.size()-1;
			}
			pFace->vx12 = vxIndexB;

			// side 3
			int vxIndexC = FindVertex( faceList, pFace->vx2, pFace->vx0 );
			if (vxIndexC == -1)
			{
				VEC3 vxC = (m_vxTess[pFace->vx2] + m_vxTess[pFace->vx0]) * 0.5F;
				m_vxTess.push_back( vxC );
				vxIndexC = (int)m_vxTess.size()-1;
			}
			pFace->vx20 = vxIndexC;

			// replace old face with the 4 new faces
			FaceType face0(pFace->vx0, (UInt16)vxIndexA, (UInt16)vxIndexC);
			FaceType face1((UInt16)vxIndexA, pFace->vx1, (UInt16)vxIndexB);
			FaceType face2((UInt16)vxIndexC, (UInt16)vxIndexB, pFace->vx2);
			FaceType face3((UInt16)vxIndexA, (UInt16)vxIndexB, (UInt16)vxIndexC);

			m_faceTess.push_back(face0);
			m_faceTess.push_back(face1);
			m_faceTess.push_back(face2);
			m_faceTess.push_back(face3);
		}

		Logger::Write( __FUNCTION__, "Tesselation Level %d: %u faces", i+1, m_faceTess.size() );
		Logger::Write( __FUNCTION__, "Tesselation Level %d: %u vertices", i+1, m_vxTess.size() );

		switch (i) {
		case 0:
			assert(GetFaceCount() == 48);
			assert(GetVertexCount() == 26);
			break;
		case 1:
			assert(GetFaceCount() == 192);
			assert(GetVertexCount() == 98);
			break;
		case 2:
			assert(GetFaceCount() == 768);
			assert(GetVertexCount() == 386);
			break;
		case 3:
			assert(GetFaceCount() == 3072);
			assert(GetVertexCount() == 1538);
			break;
		}
	}

	if (vertexType != VertexBufferGL::VertexType::NONE)
		CreateVertexBuffer(vertexType);
}

void Cube::CreateBase()
{
	// create base (lowest resolution) model
	//
	// base is unit cube
	//

	m_vxBase.clear();
	m_vxBase.push_back( VEC3(-0.5, -0.5, -0.5) );
	m_vxBase.push_back( VEC3( 0.5, -0.5, -0.5) );
	m_vxBase.push_back( VEC3(-0.5,  0.5, -0.5) );
	m_vxBase.push_back( VEC3( 0.5,  0.5, -0.5) );
	m_vxBase.push_back( VEC3(-0.5, -0.5,  0.5) );
	m_vxBase.push_back( VEC3( 0.5, -0.5,  0.5) );
	m_vxBase.push_back( VEC3(-0.5,  0.5,  0.5) );
	m_vxBase.push_back( VEC3( 0.5,  0.5,  0.5) );

	m_faceBase.clear();
	m_faceBase.push_back( FaceType( 1, WIND( 2, 3 ) ));
	m_faceBase.push_back( FaceType( 2, WIND( 1, 0 ) ));
	m_faceBase.push_back( FaceType( 4, WIND( 5, 6 ) ));
	m_faceBase.push_back( FaceType( 6, WIND( 5, 7 ) ));
	m_faceBase.push_back( FaceType( 3, WIND( 2, 6 ) ));
	m_faceBase.push_back( FaceType( 3, WIND( 6, 7 ) ));
	m_faceBase.push_back( FaceType( 0, WIND( 1, 4 ) ));
	m_faceBase.push_back( FaceType( 4, WIND( 1, 5 ) ));
	m_faceBase.push_back( FaceType( 2, WIND( 0, 4 ) ));
	m_faceBase.push_back( FaceType( 2, WIND( 4, 6 ) ));
	m_faceBase.push_back( FaceType( 1, WIND( 3, 5 ) ));
	m_faceBase.push_back( FaceType( 5, WIND( 3, 7 ) ));
	
	assert(m_vxBase.size() == 8);
	assert(m_faceBase.size() == 12);
}

void Cube::CreateVertexBuffer(VertexBufferGL::VertexType vertexType)
{
	m_VertexType = vertexType;

	mf_BoundRadius = 0.0;

	switch (vertexType) {
	case VertexBufferGL::VertexType::PC:
		{
			// we keep this in case the user wants to modify the vertices
			// (seems like a waste of memory - could we just retrieve the
			//  vertices from the GPU if needed?)
			m_vxListPC.clear();

			VertexBufferGL::VertexPC vx;

			//TODO:
			//allow for other than white
			vx.r = 1.0F;
			vx.g = 1.0F;
			vx.b = 1.0F;

			for (unsigned int i = 0; i < m_vxTess.size(); ++i)
			{
				vx.x = static_cast<float>(m_vxTess[i].x * m_Size.x);
				vx.y = static_cast<float>(m_vxTess[i].y * m_Size.y);
				vx.z = static_cast<float>(m_vxTess[i].z * m_Size.z);

				if (mf_BoundRadius < vx.x*vx.x + vx.y*vx.y + vx.z*vx.z)
					mf_BoundRadius = vx.x*vx.x + vx.y*vx.y + vx.z*vx.z;

				m_vxListPC.push_back(vx);
			}
		}
		break;
	case VertexBufferGL::VertexType::PNT:
		{
			// we keep this in case the user wants to modify the vertices
			// (seems like a waste of memory - could we just retrieve the
			//  vertices from the GPU if needed?)
			m_vxListPNT.clear();

			VertexBufferGL::VertexPNT vx;
			for (unsigned int i = 0; i < m_vxTess.size(); ++i)
			{
				vx.x = static_cast<float>(m_vxTess[i].x * m_Size.x);
				vx.y = static_cast<float>(m_vxTess[i].y * m_Size.y);
				vx.z = static_cast<float>(m_vxTess[i].z * m_Size.z);

				vx.nx = vx.x;
				vx.ny = vx.y;
				vx.nz = vx.z;

				//TODO:
				//must set/tesselate these
				vx.tu = 0.0F;
				vx.tv = 0.0F;

				if (mf_BoundRadius < vx.x*vx.x + vx.y*vx.y + vx.z*vx.z)
					mf_BoundRadius = vx.x*vx.x + vx.y*vx.y + vx.z*vx.z;

				m_vxListPNT.push_back(vx);
			}
		}
		break;
	case VertexBufferGL::VertexType::PNC:
	{
		// we keep this in case the user wants to modify the vertices
		// (seems like a waste of memory - could we just retrieve the
		//  vertices from the GPU if needed?)
		m_vxListPNC.clear();

		VertexBufferGL::VertexPNC vx;

		//TODO:
		//allow for other than white
		vx.r = 1.0F;
		vx.g = 1.0F;
		vx.b = 1.0F;

		for (unsigned int i = 0; i < m_vxTess.size(); ++i)
		{
			vx.x = static_cast<float>(m_vxTess[i].x * m_Size.x);
			vx.y = static_cast<float>(m_vxTess[i].y * m_Size.y);
			vx.z = static_cast<float>(m_vxTess[i].z * m_Size.z);

			vx.nx = vx.x;
			vx.ny = vx.y;
			vx.nz = vx.z;

			if (mf_BoundRadius < vx.x*vx.x + vx.y*vx.y + vx.z*vx.z)
				mf_BoundRadius = vx.x*vx.x + vx.y*vx.y + vx.z*vx.z;

			m_vxListPNC.push_back(vx);
		}
	}
	break;
	default:
		assert(false);
	}

	mf_BoundRadius = sqrt(mf_BoundRadius);

	ApplyChanges();
}

void Cube::ApplyChanges()
{
	// Recreate vertex buffer to reflect changes to vertices.

	std::vector<unsigned int> indexList;
	for (unsigned int i = 0; i < m_faceTess.size(); ++i)
	{
		indexList.push_back(m_faceTess[i].vx0);
		indexList.push_back(m_faceTess[i].vx1);
		indexList.push_back(m_faceTess[i].vx2);
	}

	// rebuild the vertex buffer using current m_vxList values
	switch (m_VertexType) {
	case VertexBufferGL::VertexType::PC:
		m_VB.CreatePC(GL_TRIANGLES, m_vxListPC, indexList);
		break;
	case VertexBufferGL::VertexType::PNT:
		m_VB.CreatePNT(GL_TRIANGLES, m_vxListPNT, indexList);
		break;
	case VertexBufferGL::VertexType::PNC:
		m_VB.CreatePNC(GL_TRIANGLES, m_vxListPNC, indexList);
		break;
	default:
		assert(false);
	}
}
