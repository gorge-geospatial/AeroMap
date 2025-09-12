#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <GL/glew.h>

#include "VertexBufferGL.h"
#include "Calc.h"

#include <vector>

class Primitive
{
public:

	Primitive();
	virtual ~Primitive();

	virtual void ApplyChanges() = 0;
	virtual void Render() = 0;
	virtual double GetBoundingSphere() = 0;		// return radius of minimum bounding sphere

	virtual int GetFaceCount() { return static_cast<int>(m_faceTess.size()); }
	virtual int GetVertexCount() { return static_cast<int>(m_vxTess.size()); }
	
	VertexBufferGL::VertexPC* GetVertexPC(int index);		// get a reference to vertex[index]
	VertexBufferGL::VertexPNT* GetVertexPNT(int index);
	VertexBufferGL::VertexPNC* GetVertexPNC(int index);

protected:

	struct FaceType
	{
		UInt16 vx0;			// vertex indices
		UInt16 vx1;
		UInt16 vx2;

		// tesselation support
		UInt16 vx01;		// vertex that bisects side <vx0,vx1>
		UInt16 vx12;
		UInt16 vx20;

		FaceType(UInt16 vx0, UInt16 vx1, UInt16 vx2)
		{
			this->vx0 = vx0;
			this->vx1 = vx1;
			this->vx2 = vx2;

			vx01 = 0xFFFF;		// not bisected
			vx12 = 0xFFFF;
			vx20 = 0xFFFF;
		}
	};

	std::vector<VEC3> m_vxBase;				// base vector of vertices
	std::vector<VEC3> m_vxTess;				// tesselated vertices
	std::vector<FaceType> m_faceBase;		// base vector of faces
	std::vector<FaceType> m_faceTess;		// tesselated faces

	VertexBufferGL::VertexType m_VertexType;		// type of vertex buffer
		
	std::vector<VertexBufferGL::VertexPNT> m_vxListPNT;		// list of PNT vertices
	std::vector<VertexBufferGL::VertexPNC> m_vxListPNC;		// list of PNC vertices
	std::vector<VertexBufferGL::VertexPC> m_vxListPC;		// list of PC vertices

	VertexBufferGL m_VB;		// indexed VB
	bool mb_WindCW;				// clockwise winding order
	double mf_BoundRadius;		// radius of minimum bounding sphere

	const int MAX_TESS = 4;		// maximum # of tesselation levels

protected:

	virtual int FindVertex(std::vector<FaceType>& faceList, UInt16 vx0, UInt16 vx1);

};

#endif // #ifndef PRIMITIVE_H
