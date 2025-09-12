// Primitive.cpp
// Base class for geometric primitives.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <assert.h>

#include "Logger.h"
#include "Primitive.h"

// order the 2nd and 3rd vertices of a triangle based on the winding order
#define WIND(v1, v2) (mb_WindCW ? v1 : v2), (mb_WindCW ? v2 : v1)

#define MAX_TESS 4		// maximum # of tesselation levels

Primitive::Primitive()
	: mb_WindCW(true)
	, m_VertexType(VertexBufferGL::VertexType::NONE)
	, mf_BoundRadius(0.0)
{
}

Primitive::~Primitive()
{
}

VertexBufferGL::VertexPC* Primitive::GetVertexPC(int index)
{
	// get a pointer to vertex[index]
	if (index < m_vxListPC.size())
		return &m_vxListPC[index];

	return nullptr;
}

VertexBufferGL::VertexPNT* Primitive::GetVertexPNT(int index)
{
	// get a pointer to vertex[index]
	if (index < m_vxListPNT.size())
		return &m_vxListPNT[index];

	return nullptr;
}

VertexBufferGL::VertexPNC* Primitive::GetVertexPNC(int index)
{
	// get a pointer to vertex[index]
	if (index < m_vxListPNC.size())
		return &m_vxListPNC[index];

	return nullptr;
}

int Primitive::FindVertex(std::vector<FaceType>& faceList, UInt16 vx0, UInt16 vx1)
{
	// See if side bisected by <vx0,vx1> has already been bisected

	// Inputs:
	//		faceList = input face list
	//		vx0,vx1  = pair of connected vertices in input face list

	// Outputs:
	//		return = index of bisecting vertex if <vx0,vx1> has been bisected
	//				 else -1
	//

	for (unsigned int faceIdx = 0; faceIdx < faceList.size(); ++faceIdx)
	{
		FaceType* pFace = &faceList[faceIdx];

		bool isBisected = false;
		if (pFace->vx01 != 0xFFFF)
		{
			if ((pFace->vx0 == vx0 && pFace->vx1 == vx1)
				|| (pFace->vx1 == vx0 && pFace->vx0 == vx1))
			{
				// these vertices define a side that has already been bisected,
				// return the index of that bisecting vertex
				return pFace->vx01;
			}
			isBisected = true;
		}
		if (pFace->vx12 != 0xFFFF)
		{
			if ((pFace->vx1 == vx0 && pFace->vx2 == vx1)
				|| (pFace->vx2 == vx0 && pFace->vx1 == vx1))
			{
				// these vertices define a side that has already been bisected,
				// return the index of that bisecting vertex
				return pFace->vx12;
			}
			isBisected = true;
		}
		if (pFace->vx20 != 0xFFFF)
		{
			if ((pFace->vx2 == vx0 && pFace->vx0 == vx1)
				|| (pFace->vx0 == vx0 && pFace->vx2 == vx1))
			{
				// these vertices define a side that has already been bisected,
				// return the index of that bisecting vertex
				return pFace->vx20;
			}
			isBisected = true;
		}

		// stop searching at the first face that is not (completely) bisected - from
		// here on, the vertex can't possibly be found because none of the faces have
		// been processed yet
		if (!isBisected)
			break;
	}

	return -1;
}
