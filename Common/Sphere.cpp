// Sphere.cpp
// Class to create and manage sphere geometry
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//
// Method used here starts with low resolution icosahedron
// and tesselates to the desired # of faces. Main advantage
// of this is that all triangles are identical - no pole 
// artifacts.
//

#include <assert.h>

#include "Logger.h"
#include "Sphere.h"

// order the 2nd and 3rd vertices of a triangle based on the winding order
#define WIND(v1, v2) (mb_WindCW ? v1 : v2), (mb_WindCW ? v2 : v1)

Sphere::Sphere()
	: Primitive()
	, mf_Radius(1.0)
{
}

Sphere::~Sphere()
{
}

void Sphere::Render()
{
	if (m_VB.IsValid())
		m_VB.Render();
}

void Sphere::Create(VertexBufferGL::VertexType vertexType, double radius, int tessLevel, bool windCW)
{
	// Create a sphere for opengl rendering by tesselating
	// an icosahedron (equidistant points, not a globe with
	// pole artifacts)

	// Inputs:
	//		vertexType	= type of vertex buffer to create for rendering
	//		radius		= radius of generated sphere
	//		tessLevel	= # of times to subdivide base (low res) sphere
	//					  (accepts 0 and just use default base (no tesselation))
	//		windCW		=
	//

	if (tessLevel > MAX_TESS)
		tessLevel = MAX_TESS;
	if (radius < 0.0F)
		return;

	mf_Radius = radius;
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

				// new vertex is at midpoint, but still need to be pushed out to desired radius
				// treat vxA as a vector and scale it to R
				double m1 = Magnitude(vxA);
				double sf = 1.0F / m1;
				vxA *= sf;

				m_vxTess.push_back( vxA );
				vxIndexA = (int)m_vxTess.size()-1;
			}
			pFace->vx01 = vxIndexA;
			
			// side 2
			int vxIndexB = FindVertex( faceList, pFace->vx1, pFace->vx2 );
			if (vxIndexB == -1)
			{
				VEC3 vxB = (m_vxTess[pFace->vx1] + m_vxTess[pFace->vx2]) * 0.5F;

				double m1 = Magnitude(vxB);
				double sf = 1.0F / m1;
				vxB *= sf;

				m_vxTess.push_back( vxB );
				vxIndexB = (int)m_vxTess.size()-1;
			}
			pFace->vx12 = vxIndexB;

			// side 3
			int vxIndexC = FindVertex( faceList, pFace->vx2, pFace->vx0 );
			if (vxIndexC == -1)
			{
				VEC3 vxC = (m_vxTess[pFace->vx2] + m_vxTess[pFace->vx0]) * 0.5F;

				double m1 = Magnitude(vxC);
				double sf = 1.0F / m1;
				vxC *= sf;

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
			assert(GetFaceCount() == 432);
			assert(GetVertexCount() == 218);
			break;
		case 1:
			assert(GetFaceCount() == 1728);
			assert(GetVertexCount() == 866);
			break;
		case 2:
			assert(GetFaceCount() == 6912);
			assert(GetVertexCount() == 3458);
			break;
		case 3:
			assert(GetFaceCount() == 27648);
			assert(GetVertexCount() == 13826);
			break;
		}
	}

	if (vertexType != VertexBufferGL::VertexType::NONE)
		CreateVertexBuffer(vertexType);
}

void Sphere::CreateBase()
{
	// create base (lowest resolution) model
	//
	// 56 vertices / 108 faces in base model
	//
	// base is unit sphere
	//

	m_vxBase.clear();
	m_vxBase.push_back( VEC3(-0.57731,  0.57743, -0.57731) );
	m_vxBase.push_back( VEC3(-0.69494,  0.18474, -0.69494) );
	m_vxBase.push_back( VEC3(-0.93529,  0.25029, -0.25018) );
	m_vxBase.push_back( VEC3(-0.69489,  0.69501, -0.18461) );
	m_vxBase.push_back( VEC3(-0.69495, -0.18463, -0.69495) );
	m_vxBase.push_back( VEC3(-0.93532, -0.25018, -0.25018) );
	m_vxBase.push_back( VEC3(-0.57735, -0.57735, -0.57735) );
	m_vxBase.push_back( VEC3(-0.69495, -0.69495, -0.18463) );
	m_vxBase.push_back( VEC3(-0.93526,  0.25029,  0.25029) );
	m_vxBase.push_back( VEC3(-0.69488,  0.69500,  0.18472) );
	m_vxBase.push_back( VEC3(-0.93529, -0.25018,  0.25029) );
	m_vxBase.push_back( VEC3(-0.69494, -0.69494,  0.18474) );
	m_vxBase.push_back( VEC3(-0.69488,  0.18472,  0.69500) );
	m_vxBase.push_back( VEC3(-0.57727,  0.57739,  0.57739) );
	m_vxBase.push_back( VEC3(-0.69489, -0.18461,  0.69501) );
	m_vxBase.push_back( VEC3(-0.57731, -0.57731,  0.57743) );
	m_vxBase.push_back( VEC3(-0.18463, -0.69495, -0.69495) );
	m_vxBase.push_back( VEC3(-0.25018, -0.93532, -0.25018) );
	m_vxBase.push_back( VEC3( 0.18474, -0.69494, -0.69494) );
	m_vxBase.push_back( VEC3( 0.25029, -0.93529, -0.25018) );
	m_vxBase.push_back( VEC3( 0.57743, -0.57731, -0.57731) );
	m_vxBase.push_back( VEC3( 0.69501, -0.69489, -0.18461) );
	m_vxBase.push_back( VEC3(-0.25018, -0.93529,  0.25029) );
	m_vxBase.push_back( VEC3( 0.25029, -0.93526,  0.25029) );
	m_vxBase.push_back( VEC3( 0.69500, -0.69488,  0.18472) );
	m_vxBase.push_back( VEC3(-0.18461, -0.69489,  0.69501) );
	m_vxBase.push_back( VEC3( 0.18472, -0.69488,  0.69500) );
	m_vxBase.push_back( VEC3( 0.57739, -0.57727,  0.57739) );
	m_vxBase.push_back( VEC3(-0.18461,  0.69501, -0.69489) );
	m_vxBase.push_back( VEC3(-0.25018,  0.25029, -0.93529) );
	m_vxBase.push_back( VEC3( 0.18472,  0.69500, -0.69488) );
	m_vxBase.push_back( VEC3( 0.25029,  0.25029, -0.93526) );
	m_vxBase.push_back( VEC3( 0.57739,  0.57739, -0.57727) );
	m_vxBase.push_back( VEC3( 0.69500,  0.18472, -0.69488) );
	m_vxBase.push_back( VEC3(-0.25018, -0.25018, -0.93532) );
	m_vxBase.push_back( VEC3( 0.25029, -0.25018, -0.93529) );
	m_vxBase.push_back( VEC3( 0.69501, -0.18461, -0.69489) );
	m_vxBase.push_back( VEC3( 0.69495,  0.69495, -0.18460) );
	m_vxBase.push_back( VEC3( 0.93530,  0.25027, -0.25015) );
	m_vxBase.push_back( VEC3( 0.69494,  0.69494,  0.18471) );
	m_vxBase.push_back( VEC3( 0.93527,  0.25026,  0.25026) );
	m_vxBase.push_back( VEC3( 0.57735,  0.57735,  0.57735) );
	m_vxBase.push_back( VEC3( 0.69494,  0.18471,  0.69494) );
	m_vxBase.push_back( VEC3( 0.93533, -0.25016, -0.25016) );
	m_vxBase.push_back( VEC3( 0.93530, -0.25015,  0.25027) );
	m_vxBase.push_back( VEC3( 0.69495, -0.18460,  0.69495) );
	m_vxBase.push_back( VEC3(-0.25016,  0.93533, -0.25016) );
	m_vxBase.push_back( VEC3(-0.25015,  0.93530,  0.25027) );
	m_vxBase.push_back( VEC3(-0.18460,  0.69495,  0.69495) );
	m_vxBase.push_back( VEC3( 0.25027,  0.93530, -0.25015) );
	m_vxBase.push_back( VEC3( 0.25026,  0.93527,  0.25026) );
	m_vxBase.push_back( VEC3( 0.18471,  0.69494,  0.69494) );
	m_vxBase.push_back( VEC3(-0.25015,  0.25027,  0.93530) );
	m_vxBase.push_back( VEC3(-0.25016, -0.25016,  0.93533) );
	m_vxBase.push_back( VEC3( 0.25026,  0.25026,  0.93527) );
	m_vxBase.push_back( VEC3( 0.25027, -0.25015,  0.93530) );
	assert( m_vxBase.size() == 56 );

	m_faceBase.clear();
	m_faceBase.push_back( FaceType(	  0, WIND( 1,  2) ));
	m_faceBase.push_back( FaceType(	  2, WIND( 3,  0) ));
	m_faceBase.push_back( FaceType(	  1, WIND( 4,  5) ));
	m_faceBase.push_back( FaceType(	  5, WIND( 2,  1) ));
	m_faceBase.push_back( FaceType(	  4, WIND( 6,  7) ));
	m_faceBase.push_back( FaceType(	  7, WIND( 5,  4) ));
	m_faceBase.push_back( FaceType(	  3, WIND( 2,  8) ));
	m_faceBase.push_back( FaceType(	  8, WIND( 9,  3) ));
	m_faceBase.push_back( FaceType(	  2, WIND( 5, 10) ));
	m_faceBase.push_back( FaceType(	 10, WIND( 8,  2) ));
	m_faceBase.push_back( FaceType(	  5, WIND( 7, 11) ));
	m_faceBase.push_back( FaceType(	 11, WIND(10,  5) ));
	m_faceBase.push_back( FaceType(	  9, WIND( 8, 12) ));
	m_faceBase.push_back( FaceType(	 12, WIND(13,  9) ));
	m_faceBase.push_back( FaceType(	  8, WIND(10, 14) ));
	m_faceBase.push_back( FaceType(	 14, WIND(12,  8) ));
	m_faceBase.push_back( FaceType(	 10, WIND(11, 15) ));
	m_faceBase.push_back( FaceType(	 15, WIND(14, 10) ));
	m_faceBase.push_back( FaceType(	  6, WIND(16, 17) ));
	m_faceBase.push_back( FaceType(	 17, WIND( 7,  6) ));
	m_faceBase.push_back( FaceType(	 16, WIND(18, 19) ));
	m_faceBase.push_back( FaceType(	 19, WIND(17, 16) ));
	m_faceBase.push_back( FaceType(	 18, WIND(20, 21) ));
	m_faceBase.push_back( FaceType(	 21, WIND(19, 18) ));
	m_faceBase.push_back( FaceType(	  7, WIND(17, 22) ));
	m_faceBase.push_back( FaceType(	 22, WIND(11,  7) ));
	m_faceBase.push_back( FaceType(	 17, WIND(19, 23) ));
	m_faceBase.push_back( FaceType(	 23, WIND(22, 17) ));
	m_faceBase.push_back( FaceType(	 19, WIND(21, 24) ));
	m_faceBase.push_back( FaceType(	 24, WIND(23, 19) ));
	m_faceBase.push_back( FaceType(	 11, WIND(22, 25) ));
	m_faceBase.push_back( FaceType(	 25, WIND(15, 11) ));
	m_faceBase.push_back( FaceType(	 22, WIND(23, 26) ));
	m_faceBase.push_back( FaceType(	 26, WIND(25, 22) ));
	m_faceBase.push_back( FaceType(	 23, WIND(24, 27) ));
	m_faceBase.push_back( FaceType(	 27, WIND(26, 23) ));
	m_faceBase.push_back( FaceType(	  0, WIND(28, 29) ));
	m_faceBase.push_back( FaceType(	 29, WIND( 1,  0) ));
	m_faceBase.push_back( FaceType(	 28, WIND(30, 31) ));
	m_faceBase.push_back( FaceType(	 31, WIND(29, 28) ));
	m_faceBase.push_back( FaceType(	 30, WIND(32, 33) ));
	m_faceBase.push_back( FaceType(	 33, WIND(31, 30) ));
	m_faceBase.push_back( FaceType(	  1, WIND(29, 34) ));
	m_faceBase.push_back( FaceType(	 34, WIND( 4,  1) ));
	m_faceBase.push_back( FaceType(	 29, WIND(31, 35) ));
	m_faceBase.push_back( FaceType(	 35, WIND(34, 29) ));
	m_faceBase.push_back( FaceType(	 31, WIND(33, 36) ));
	m_faceBase.push_back( FaceType(	 36, WIND(35, 31) ));
	m_faceBase.push_back( FaceType(	  4, WIND(34, 16) ));
	m_faceBase.push_back( FaceType(	 16, WIND( 6,  4) ));
	m_faceBase.push_back( FaceType(	 34, WIND(35, 18) ));
	m_faceBase.push_back( FaceType(	 18, WIND(16, 34) ));
	m_faceBase.push_back( FaceType(	 35, WIND(36, 20) ));
	m_faceBase.push_back( FaceType(	 20, WIND(18, 35) ));
	m_faceBase.push_back( FaceType(	 32, WIND(37, 38) ));
	m_faceBase.push_back( FaceType(	 38, WIND(33, 32) ));
	m_faceBase.push_back( FaceType(	 37, WIND(39, 40) ));
	m_faceBase.push_back( FaceType(	 40, WIND(38, 37) ));
	m_faceBase.push_back( FaceType(	 39, WIND(41, 42) ));
	m_faceBase.push_back( FaceType(	 42, WIND(40, 39) ));
	m_faceBase.push_back( FaceType(	 33, WIND(38, 43) ));
	m_faceBase.push_back( FaceType(	 43, WIND(36, 33) ));
	m_faceBase.push_back( FaceType(	 38, WIND(40, 44) ));
	m_faceBase.push_back( FaceType(	 44, WIND(43, 38) ));
	m_faceBase.push_back( FaceType(	 40, WIND(42, 45) ));
	m_faceBase.push_back( FaceType(	 45, WIND(44, 40) ));
	m_faceBase.push_back( FaceType(	 36, WIND(43, 21) ));
	m_faceBase.push_back( FaceType(	 21, WIND(20, 36) ));
	m_faceBase.push_back( FaceType(	 43, WIND(44, 24) ));
	m_faceBase.push_back( FaceType(	 24, WIND(21, 43) ));
	m_faceBase.push_back( FaceType(	 44, WIND(45, 27) ));
	m_faceBase.push_back( FaceType(	 27, WIND(24, 44) ));
	m_faceBase.push_back( FaceType(	  0, WIND( 3, 46) ));
	m_faceBase.push_back( FaceType(	 46, WIND(28,  0) ));
	m_faceBase.push_back( FaceType(	  3, WIND( 9, 47) ));
	m_faceBase.push_back( FaceType(	 47, WIND(46,  3) ));
	m_faceBase.push_back( FaceType(	  9, WIND(13, 48) ));
	m_faceBase.push_back( FaceType(	 48, WIND(47,  9) ));
	m_faceBase.push_back( FaceType(	 28, WIND(46, 49) ));
	m_faceBase.push_back( FaceType(	 49, WIND(30, 28) ));
	m_faceBase.push_back( FaceType(	 46, WIND(47, 50) ));
	m_faceBase.push_back( FaceType(	 50, WIND(49, 46) ));
	m_faceBase.push_back( FaceType(	 47, WIND(48, 51) ));
	m_faceBase.push_back( FaceType(	 51, WIND(50, 47) ));
	m_faceBase.push_back( FaceType(	 30, WIND(49, 37) ));
	m_faceBase.push_back( FaceType(	 37, WIND(32, 30) ));
	m_faceBase.push_back( FaceType(	 49, WIND(50, 39) ));
	m_faceBase.push_back( FaceType(	 39, WIND(37, 49) ));
	m_faceBase.push_back( FaceType(	 50, WIND(51, 41) ));
	m_faceBase.push_back( FaceType(	 41, WIND(39, 50) ));
	m_faceBase.push_back( FaceType(	 13, WIND(12, 52) ));
	m_faceBase.push_back( FaceType(	 52, WIND(48, 13) ));
	m_faceBase.push_back( FaceType(	 12, WIND(14, 53) ));
	m_faceBase.push_back( FaceType(	 53, WIND(52, 12) ));
	m_faceBase.push_back( FaceType(	 14, WIND(15, 25) ));
	m_faceBase.push_back( FaceType(	 25, WIND(53, 14) ));
	m_faceBase.push_back( FaceType(	 48, WIND(52, 54) ));
	m_faceBase.push_back( FaceType(	 54, WIND(51, 48) ));
	m_faceBase.push_back( FaceType(	 52, WIND(53, 55) ));
	m_faceBase.push_back( FaceType(	 55, WIND(54, 52) ));
	m_faceBase.push_back( FaceType(	 53, WIND(25, 26) ));
	m_faceBase.push_back( FaceType(	 26, WIND(55, 53) ));
	m_faceBase.push_back( FaceType(	 51, WIND(54, 42) ));
	m_faceBase.push_back( FaceType(	 42, WIND(41, 51) ));
	m_faceBase.push_back( FaceType(	 54, WIND(55, 45) ));
	m_faceBase.push_back( FaceType(	 45, WIND(42, 54) ));
	m_faceBase.push_back( FaceType(	 55, WIND(26, 27) ));
	m_faceBase.push_back( FaceType(	 27, WIND(45, 55) ));
	assert( m_faceBase.size() == 108 );
}

void Sphere::CreateVertexBuffer(VertexBufferGL::VertexType vertexType)
{
	m_VertexType = vertexType;

	// we keep this in case the user wants to modify the vertices
	// (seems like a waste of memory - could we just retrieve the
	//  vertices from the GPU if needed?)
	m_vxListPNT.clear();
	m_vxListPNC.clear();
	m_vxListPC.clear();

	switch (vertexType) {
	case VertexBufferGL::VertexType::PC:
		{
			VertexBufferGL::VertexPC vx;

			vx.r = 1.0F;
			vx.g = 1.0F;
			vx.b = 1.0F;

			for (unsigned int i = 0; i < m_vxTess.size(); ++i)
			{
				vx.x = static_cast<float>(m_vxTess[i].x * mf_Radius);
				vx.y = static_cast<float>(m_vxTess[i].y * mf_Radius);
				vx.z = static_cast<float>(m_vxTess[i].z * mf_Radius);

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
				// since base sphere is untranslated unit sphere, just using
				// vertices as normals
				vx.nx = static_cast<float>(m_vxTess[i].x);
				vx.ny = static_cast<float>(m_vxTess[i].y);
				vx.nz = static_cast<float>(m_vxTess[i].z);

				vx.x = static_cast<float>(m_vxTess[i].x * mf_Radius);
				vx.y = static_cast<float>(m_vxTess[i].y * mf_Radius);
				vx.z = static_cast<float>(m_vxTess[i].z * mf_Radius);

				//TODO:
				vx.tu = 0.0F;
				vx.tv = 0.0F;

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
		for (unsigned int i = 0; i < m_vxTess.size(); ++i)
		{
			vx.x = static_cast<float>(m_vxTess[i].x * mf_Radius);
			vx.y = static_cast<float>(m_vxTess[i].y * mf_Radius);
			vx.z = static_cast<float>(m_vxTess[i].z * mf_Radius);

			// since base sphere is untranslated unit sphere, just using
			// vertices as normals
			vx.nx = static_cast<float>(m_vxTess[i].x);
			vx.ny = static_cast<float>(m_vxTess[i].y);
			vx.nz = static_cast<float>(m_vxTess[i].z);

			// default color
			vx.r = 0.9F;
			vx.g = 0.9F;
			vx.b = 0.0F;

			m_vxListPNC.push_back(vx);
		}
	}
	break;
	default:
		assert(false);
	}

	ApplyChanges();
}

void Sphere::ApplyChanges()
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
