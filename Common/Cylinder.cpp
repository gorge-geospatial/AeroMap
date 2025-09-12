// Cylinder.cpp
// Class to create and manage cylinder geometry
//

#include <assert.h>

#include "Logger.h"
#include "Cylinder.h"

// order the 2nd and 3rd vertices of a triangle based on the winding order
#define WIND(v1, v2) (mb_WindCW ? v1 : v2), (mb_WindCW ? v2 : v1)

Cylinder::Cylinder()
	: Primitive()
	, mf_Radius(1.0)
{
}

Cylinder::~Cylinder()
{
}

void Cylinder::Render()
{
	if (m_VB.IsValid())
		m_VB.Render();
}

void Cylinder::Create(VertexBufferGL::VertexType vertexType, double radius, double length, int tessLevel, bool windCW)
{
	// Create a cylinder for opengl rendering.

	// Inputs:
	//		vertexType	= type of vertex buffer to create for rendering
	//		radius		= radius
	//		length		= length
	//		tessLevel	= # of times to subdivide base (low res) model
	//					  (accepts 0 and just use default base (no tesselation))
	//		windCW		=
	//

	assert(tessLevel >= 0 && tessLevel <= MAX_TESS);
	assert(radius > 0.0);
	if (tessLevel > MAX_TESS)
		tessLevel = MAX_TESS;
	if (radius < 0.0)
		return;

	mf_Radius = radius;
	mb_WindCW = windCW;

	CreateBase(radius, length);

	// these vectors are inputs to the process (m_* are outputs)
	std::vector<FaceType> faceList;

	//TODO:
	// there is currently no support for tesselation, but with a little thought
	// it could certainly be added

	// init inputs from base
	m_faceTess.assign(m_faceBase.cbegin(), m_faceBase.cend());
	m_vxTess.assign(m_vxBase.cbegin(), m_vxBase.cend());

	if (vertexType != VertexBufferGL::VertexType::NONE)
		CreateVertexBuffer(vertexType);
}

void Cylinder::CreateBase(double radius, double length)
{
	// Create base (lowest resolution) model.
	//

	m_vxBase.clear();

	double diam = radius * 2.0;
	m_vxBase.push_back(VEC3(-0.2778*diam,  0.4252*diam, -0.5*length));		// base
	m_vxBase.push_back(VEC3(-0.4598*diam,  0.1830*diam, -0.5*length));
	m_vxBase.push_back(VEC3( 0.3112*diam,  0.4252*diam, -0.5*length));
	m_vxBase.push_back(VEC3( 0.0167*diam,  0.5177*diam, -0.5*length));
	m_vxBase.push_back(VEC3(-0.4598*diam, -0.1162*diam, -0.5*length));
	m_vxBase.push_back(VEC3( 0.4932*diam,  0.1830*diam, -0.5*length));
	m_vxBase.push_back(VEC3(-0.2778*diam, -0.3584*diam, -0.5*length));
	m_vxBase.push_back(VEC3( 0.4932*diam, -0.1162*diam, -0.5*length));
	m_vxBase.push_back(VEC3( 0.0167*diam, -0.4509*diam, -0.5*length));
	m_vxBase.push_back(VEC3( 0.3112*diam, -0.3584*diam, -0.5*length));

	m_vxBase.push_back(VEC3( 0.0167*diam,  0.5177*diam, 0.5*length));		// top
	m_vxBase.push_back(VEC3( 0.3112*diam,  0.4252*diam, 0.5*length));
	m_vxBase.push_back(VEC3(-0.4598*diam,  0.1830*diam, 0.5*length));
	m_vxBase.push_back(VEC3(-0.2778*diam,  0.4252*diam, 0.5*length));
	m_vxBase.push_back(VEC3( 0.4932*diam,  0.1830*diam, 0.5*length));
	m_vxBase.push_back(VEC3(-0.4598*diam, -0.1162*diam, 0.5*length));
	m_vxBase.push_back(VEC3( 0.4932*diam, -0.1162*diam, 0.5*length));
	m_vxBase.push_back(VEC3(-0.2778*diam, -0.3584*diam, 0.5*length));
	m_vxBase.push_back(VEC3( 0.3112*diam, -0.3584*diam, 0.5*length));
	m_vxBase.push_back(VEC3( 0.0167*diam, -0.4509*diam, 0.5*length));

	m_faceBase.clear();

	m_faceBase.push_back(FaceType(0, 1, 2));
	m_faceBase.push_back(FaceType(0, 2, 3));
	m_faceBase.push_back(FaceType(1, 4, 5));
	m_faceBase.push_back(FaceType(1, 5, 2));
	m_faceBase.push_back(FaceType(4, 6, 7));
	m_faceBase.push_back(FaceType(4, 7, 5));
	m_faceBase.push_back(FaceType(6, 8, 9));
	m_faceBase.push_back(FaceType(6, 9, 7));
	m_faceBase.push_back(FaceType(10, 11, 12));
	m_faceBase.push_back(FaceType(10, 12, 13));
	m_faceBase.push_back(FaceType(11, 14, 15));
	m_faceBase.push_back(FaceType(11, 15, 12));
	m_faceBase.push_back(FaceType(14, 16, 17));
	m_faceBase.push_back(FaceType(14, 17, 15));
	m_faceBase.push_back(FaceType(16, 18, 19));
	m_faceBase.push_back(FaceType(16, 19, 17));
	m_faceBase.push_back(FaceType(3, 2, 11));
	m_faceBase.push_back(FaceType(3, 11, 10));
	m_faceBase.push_back(FaceType(2, 5, 14));
	m_faceBase.push_back(FaceType(2, 14, 11));
	m_faceBase.push_back(FaceType(5, 7, 16));
	m_faceBase.push_back(FaceType(5, 16, 14));
	m_faceBase.push_back(FaceType(7, 9, 18));
	m_faceBase.push_back(FaceType(7, 18, 16));
	m_faceBase.push_back(FaceType(9, 8, 19));
	m_faceBase.push_back(FaceType(9, 19, 18));
	m_faceBase.push_back(FaceType(8, 6, 17));
	m_faceBase.push_back(FaceType(8, 17, 19));
	m_faceBase.push_back(FaceType(6, 4, 15));
	m_faceBase.push_back(FaceType(6, 15, 17));
	m_faceBase.push_back(FaceType(4, 1, 12));
	m_faceBase.push_back(FaceType(4, 12, 15));
	m_faceBase.push_back(FaceType(1, 0, 13));
	m_faceBase.push_back(FaceType(1, 13, 12));
	m_faceBase.push_back(FaceType(0, 3, 10));
	m_faceBase.push_back(FaceType(0, 10, 13));
}

void Cylinder::CreateVertexBuffer(VertexBufferGL::VertexType vertexType)
{
	m_VertexType = vertexType;

	// we keep this in case the user wants to modify the vertices
	// (seems like a waste of memory - could we just retrieve the
	//  vertices from the GPU if needed?)
	m_vxListPNT.clear();
	m_vxListPNC.clear();
	m_vxListPC.clear();

	// radius of minimum bounding sphere (not cylinder radius)
	mf_BoundRadius = 0;

	switch (vertexType) {
	case VertexBufferGL::VertexType::PC:
		{
			VertexBufferGL::VertexPC vx;

			vx.r = 1.0F;
			vx.g = 1.0F;
			vx.b = 1.0F;

			for (unsigned int i = 0; i < m_vxTess.size(); ++i)
			{
				vx.x = static_cast<float>(m_vxTess[i].x);
				vx.y = static_cast<float>(m_vxTess[i].y);
				vx.z = static_cast<float>(m_vxTess[i].z);

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
				vx.x = static_cast<float>(m_vxTess[i].x);
				vx.y = static_cast<float>(m_vxTess[i].y);
				vx.z = static_cast<float>(m_vxTess[i].z);

				// since base untranslated unit cylinder, just using
				// vertices as normals
				vx.nx = vx.x;
				vx.ny = vx.y;
				vx.nz = vx.z;

				//TODO:
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
			for (unsigned int i = 0; i < m_vxTess.size(); ++i)
			{
				vx.x = static_cast<float>(m_vxTess[i].x);
				vx.y = static_cast<float>(m_vxTess[i].y);
				vx.z = static_cast<float>(m_vxTess[i].z);

				// since base is untranslated unit cylinder, just using
				// vertices as normals
				vx.nx = vx.x;
				vx.ny = vx.y;
				vx.nz = vx.z;

				// default color
				vx.r = 0.9F;
				vx.g = 0.9F;
				vx.b = 0.0F;

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

void Cylinder::ApplyChanges()
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
