// Model.cpp
// Polygonal model editing & rendering base class.
// 
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <GL/glew.h>			// OpenGL

#include <assert.h>

#include "GLManager.h"
#include "TextFile.h"
#include "Model.h"			// interface to this class

Model::Model()
	: mv_Pos(0, 0, 0)
	, mf_Radius(-1.0)	// negative, indicating value has not been set
	, m_RenderNormals(NORMAL_NONE)
	, mb_RenderAxes(true)
	, mb_RenderTextures(true)
	, mb_RenderOn(true)
{
	mv_Mesh.clear();
	mv_Mesh.reserve(16);
}

Model::~Model()
{
	// delete vertex and face arrays
	for (unsigned int i = 0; i < mv_Mesh.size(); ++i)
	{
		mv_Mesh[i].vFace.clear();
		mv_Mesh[i].vVx.clear();
	}
	mv_Mesh.clear();
}

void Model::Render()
{
	// Render model.
	//

	if (!mb_RenderOn)
		return;

	GLManager::GetShader("PNT")->Activate();
	Light* pLight = GLManager::GetLight(0);
	if (pLight)
	{
		GLManager::GetShader("PNT")->SetUniform("Light.Color", vec3(pLight->GetRed(), pLight->GetGreen(), pLight->GetBlue()));
		GLManager::GetShader("PNT")->SetUniform("Light.Position", vec3(pLight->GetPos().x, pLight->GetPos().y, pLight->GetPos().z));
	}

	if (m_Texture.IsValid())
	{
		// enable textures
		GLManager::GetShader("PNT")->SetUniform("EnableTextures", mb_RenderTextures);
		// tell shader to use texture unit #0 for texture "Tex1"
		GLManager::GetShader("PNT")->SetUniform("Tex1", 0);

		// currently only support a single texture per model
		m_Texture.Activate();
	}

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		Render(meshIdx);
	}

	if (mb_RenderAxes)
		RenderAxes();
	RenderSelectedFaces();
	RenderSelectedVx();
	RenderNormals();
}

void Model::Render(int meshIdx)
{
	// Render 1 mesh
	//
	// Inputs:
	//		meshIdx = index of mesh to render
	//

	if (!mb_RenderOn)
		return;

	if ((meshIdx < 0) || (meshIdx >= (int)mv_Mesh.size()))
        return;

	if (mv_Mesh[meshIdx].IsVisible == false)
		return;

    MeshType* pMesh = &mv_Mesh[meshIdx];

	VertexBufferGL vb;

	// there are 3 vx / face and 3 floats / vx

	std::vector<VertexBufferGL::VertexPNT> vxList;
	VertexBufferGL::VertexPNT vx;

//TODO:
//not taking advantage of indices

	for (unsigned int i = 0; i < pMesh->vFace.size(); ++i)
	{
		// vertex indices
		int vx0 = pMesh->vFace[i].vx0;
		int vx1 = pMesh->vFace[i].vx1;
		int vx2 = pMesh->vFace[i].vx2;

		// texture indices
		int tx0 = pMesh->vFace[i].tx0;
		int tx1 = pMesh->vFace[i].tx1;
		int tx2 = pMesh->vFace[i].tx2;

		vx.x = (float)pMesh->vVx[vx0].P.x;
		vx.y = (float)pMesh->vVx[vx0].P.y;
		vx.z = (float)pMesh->vVx[vx0].P.z;

		vx.nx = (float)pMesh->vVx[vx0].N.x;
		vx.ny = (float)pMesh->vVx[vx0].N.y;
		vx.nz = (float)pMesh->vVx[vx0].N.z;

		if (GetTexCoordCount(meshIdx) > 0)
		{
			vx.tu = pMesh->vTex[tx0].u;
			vx.tv = pMesh->vTex[tx0].v;
		}

		vxList.push_back(vx);

		vx.x = (float)pMesh->vVx[vx1].P.x;
		vx.y = (float)pMesh->vVx[vx1].P.y;
		vx.z = (float)pMesh->vVx[vx1].P.z;

		vx.nx = (float)pMesh->vVx[vx1].N.x;
		vx.ny = (float)pMesh->vVx[vx1].N.y;
		vx.nz = (float)pMesh->vVx[vx1].N.z;

		if (GetTexCoordCount(meshIdx) > 0)
		{
			vx.tu = pMesh->vTex[tx1].u;
			vx.tv = pMesh->vTex[tx1].v;
		}

		vxList.push_back(vx);

		vx.x = (float)pMesh->vVx[vx2].P.x;
		vx.y = (float)pMesh->vVx[vx2].P.y;
		vx.z = (float)pMesh->vVx[vx2].P.z;

		vx.nx = (float)pMesh->vVx[vx2].N.x;
		vx.ny = (float)pMesh->vVx[vx2].N.y;
		vx.nz = (float)pMesh->vVx[vx2].N.z;

		if (GetTexCoordCount(meshIdx) > 0)
		{
			vx.tu = pMesh->vTex[tx2].u;
			vx.tv = pMesh->vTex[tx2].v;
		}

		vxList.push_back(vx);
	}

	vb.CreatePNT(GL_TRIANGLES, vxList);
	vb.Render();
}

void Model::RenderAxes()
{
	// Render Cartesion axes
	//

	float axisLength = static_cast<float>(GetRadius());

	VertexBufferGL::VertexPC vx;
	std::vector<VertexBufferGL::VertexPC> vxList;

	// render the XYZ axes according to application coordinate system, not OpenGL's
	vx.SetPos(-axisLength, 0, 0); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(axisLength, 0, 0); vx.SetColor(1, 0, 0); vxList.push_back(vx);

	vx.SetPos(0, -axisLength, 0); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(0, axisLength, 0); vx.SetColor(0, 1, 0); vxList.push_back(vx);

	vx.SetPos(0, 0, -axisLength); vx.SetColor(0, 0, 0); vxList.push_back(vx);
	vx.SetPos(0, 0, axisLength); vx.SetColor(0, 0, 1); vxList.push_back(vx);

	GLManager::GetShader("PC")->Activate();

	VertexBufferGL vbAxes;
	vbAxes.CreatePC(GL_LINES, vxList);
	vbAxes.Render();
}

void Model::RenderSelectedFaces()
{
	// Render all selected faces.
	//

	ShaderGL* pShader = GLManager::GetShader("PC");
	assert(pShader);
	pShader->Activate();

	VertexBufferGL::VertexPC vx;
	std::vector<VertexBufferGL::VertexPC> vxList;

	vx.SetColor(0.1F, 0.9F, 0.1F);
	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];
		for (unsigned int i = 0; i < pMesh->vFace.size(); ++i)	// for each faces
		{
			if (mv_Mesh[meshIdx].vFace[i].IsSelected)
			{
				// vertex indices
				int vx0 = pMesh->vFace[i].vx0;
				int vx1 = pMesh->vFace[i].vx1;
				int vx2 = pMesh->vFace[i].vx2;

				VEC3 P0 = pMesh->vVx[vx0].P;
				VEC3 P1 = pMesh->vVx[vx1].P;
				VEC3 P2 = pMesh->vVx[vx2].P;

				vx.SetPos(P0); vxList.push_back(vx);
				vx.SetPos(P1); vxList.push_back(vx);

				vx.SetPos(P1); vxList.push_back(vx);
				vx.SetPos(P2); vxList.push_back(vx);

				vx.SetPos(P2); vxList.push_back(vx);
				vx.SetPos(P0); vxList.push_back(vx);
			}
		}
	}

	if (vxList.size() > 0)
	{
		VertexBufferGL vb;
		vb.CreatePC(GL_LINES, vxList);
		vb.Render();
	}
}

void Model::RenderSelectedVx()
{
	// Render all selected vertices.
	//

	UInt32 selectCount = GetSelectedVertexCount();
	if (selectCount < 1)
		return;

	// render in screen space to more easily control size of boxes

	const int BOX_SIZE = 5;

	GLManager::PushDepth(false);
	GLManager::PushCull(false);

	GLint size[4];
	glGetIntegerv(GL_VIEWPORT, size);
	int cx = size[2];
	int cy = size[3];

	// set up screen space projection

	ShaderGL* pShader = GLManager::GetShader("SS");
	assert(pShader);
	pShader->Activate();
	mat4 matProj = glm::ortho(0.0, (double)(cx - 1), (double)(cy - 1), 0.0);
	pShader->SetUniform("ProjectionMatrix", matProj);

	VertexBufferGL::VertexSSC vx;
	std::vector<VertexBufferGL::VertexSSC> vxList;

	vx.SetColor(0.8F);
	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			if (mv_Mesh[meshIdx].vVx[i].IsSelected)
			{
				// draw box around selected vertex

				int xp, yp;
				GLManager::Project(mv_Mesh[meshIdx].vVx[i].P, xp, yp);

				vx.SetPos(static_cast<float>(xp - BOX_SIZE), static_cast<float>(yp - BOX_SIZE)); vxList.push_back(vx);
				vx.SetPos(static_cast<float>(xp + BOX_SIZE), static_cast<float>(yp - BOX_SIZE)); vxList.push_back(vx);

				vx.SetPos(static_cast<float>(xp + BOX_SIZE), static_cast<float>(yp - BOX_SIZE)); vxList.push_back(vx);
				vx.SetPos(static_cast<float>(xp + BOX_SIZE), static_cast<float>(yp + BOX_SIZE)); vxList.push_back(vx);

				vx.SetPos(static_cast<float>(xp + BOX_SIZE), static_cast<float>(yp + BOX_SIZE)); vxList.push_back(vx);
				vx.SetPos(static_cast<float>(xp - BOX_SIZE), static_cast<float>(yp + BOX_SIZE)); vxList.push_back(vx);

				vx.SetPos(static_cast<float>(xp - BOX_SIZE), static_cast<float>(yp + BOX_SIZE)); vxList.push_back(vx);
				vx.SetPos(static_cast<float>(xp - BOX_SIZE), static_cast<float>(yp - BOX_SIZE)); vxList.push_back(vx);
			}
		}
	}

	if (vxList.size() > 0)
	{
		VertexBufferGL vb;
		vb.CreateSSC(GL_LINES, vxList);
		vb.Render();
	}

	GLManager::PopDepth();
	GLManager::PopCull();
}

void Model::RenderNormals()
{
	// render normals according to current setting
	//
	// inputs:
	//		m_RenderNormals = NORMAL_VERTEX => render vertex normals
	//					    = NORMAL_FACE   => render face normals
	//					    = NORMAL_NONE   => don't render normals
	//

	if (m_RenderNormals == NORMAL_NONE)
		return;

	switch (m_RenderNormals)
	{
	case NORMAL_VERTEX:
		{
			VertexBufferGL::VertexPC vx;
			std::vector<VertexBufferGL::VertexPC> vxList;

			for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
			{
				for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
				{
					VEC3 P = mv_Mesh[meshIdx].vVx[i].P;
					VEC3 N = mv_Mesh[meshIdx].vVx[i].N;

					vx.x = (float)P.x; vx.y = (float)P.y; vx.z = (float)P.z;
					vx.r = 0.8F; vx.g = 0.8F; vx.b = 0.8F;
					vxList.push_back( vx );

					vx.x = (float)(P.x+N.x); vx.y = (float)(P.y+N.y); vx.z = (float)(P.z+N.z);
					vx.r = 0.8F; vx.g = 0.8F; vx.b = 0.8F;
					vxList.push_back( vx );
				}
			}

			VertexBufferGL vb;
			vb.CreatePC(GL_LINES, vxList);
			vb.Render();
	}
		break;

	case NORMAL_FACE:
		{
			VertexBufferGL::VertexPC vx;
			std::vector<VertexBufferGL::VertexPC> vxList;

			for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
			{
				MeshType* pMesh = &mv_Mesh[meshIdx];
				for (unsigned int i = 0; i < pMesh->vFace.size(); ++i)		// for each face
				{
					int nVx1 = pMesh->vFace[i].vx0;
					int nVx2 = pMesh->vFace[i].vx1;
					int nVx3 = pMesh->vFace[i].vx2;

					VEC3 vVx1 = pMesh->vVx[nVx1].P;
					VEC3 vVx2 = pMesh->vVx[nVx2].P;
					VEC3 vVx3 = pMesh->vVx[nVx3].P;

					VEC3 vBase = (vVx1 + vVx2 + vVx3) / 3.0;
					VEC3 vDir  = pMesh->vFace[i].N;
					VEC3 vTip  = vBase + vDir;

					vx.x = (float)vBase.x;	vx.y = (float)vBase.y;	vx.z = (float)vBase.z;
					vx.r = 0.8F; vx.g = 0.8F; vx.b = 0.8F;
					vxList.push_back( vx );

					vx.x = (float)vTip.x;	vx.y = (float)vTip.y;	vx.z = (float)vTip.z;
					vx.r = 0.8F; vx.g = 0.8F; vx.b = 0.8F;
					vxList.push_back( vx );
				}
			}

			GLManager::GetShader("PC")->Activate();
			VertexBufferGL vb;
			vb.CreatePC(GL_LINES, vxList);
			vb.Render();
		}
		break;

	case NORMAL_NONE:
		break;
	}
}

void Model::RenderOn()
{
	// enable rendering
	mb_RenderOn = true;
}

void Model::RenderOff()
{
	// disable rendering
	mb_RenderOn = false;
}

void Model::CalculateNormals()
{
	// Calculate / update both face and vertex normals.
	//
	// Outputs:
	//		face normals in .vFace[].N
	//		vertex normals in .vVx[].N
	//

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];

        UInt32 vxCount = (UInt32)pMesh->vVx.size();
        for (UInt32 i = 0; i < vxCount; ++i)
		{
            // get the normals of adjacent faces
            VEC3 NA (0,0,0);
            int nNaCtr = 0;

			UInt32 faceCount = (UInt32)pMesh->vFace.size();
            for (UInt32 nFaceIdx = 0; nFaceIdx < faceCount; ++nFaceIdx)
			{
                if (pMesh->vFace[nFaceIdx].vx0 == i || pMesh->vFace[nFaceIdx].vx1 == i || pMesh->vFace[nFaceIdx].vx2 == i)
				{
					// get the vertex indices for the face

					int vx1 = pMesh->vFace[nFaceIdx].vx0;
					int vx2 = pMesh->vFace[nFaceIdx].vx1;
					int vx3 = pMesh->vFace[nFaceIdx].vx2;

					// convert 2 of the face sides to vectors

                    VEC3 S1;	// side 1
                    VEC3 S2;    // side 2

                    S1 = pMesh->vVx[vx2].P - pMesh->vVx[vx1].P;
                    S2 = pMesh->vVx[vx3].P - pMesh->vVx[vx1].P;

					// calc the normal

                    VEC3 N = CrossProduct(Normalize(S1), Normalize(S2));
                    N = Normalize(N);

					// store face normal

					pMesh->vFace[nFaceIdx].N = N;

					// accumulate it for average

                    NA += N;
                    nNaCtr++;
                }
            }	// Next nFaceIdx

            // average / normalize them
            NA /= (float)nNaCtr;
            pMesh->vVx[i].N = Normalize(NA);		// set the new normal

		}	// next i
    }	// next meshIdx
}

void Model::Mirror(AXIS axis)
{
	// Mirror model about specified axis.
	//

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (UInt32 nVertexIdx = 0; nVertexIdx < mv_Mesh[meshIdx].vVx.size(); ++nVertexIdx)		// for each vertex
		{
			switch (axis) {
			case AXIS::X: mv_Mesh[meshIdx].vVx[nVertexIdx].P.x = -mv_Mesh[meshIdx].vVx[nVertexIdx].P.x; break;
			case AXIS::Y: mv_Mesh[meshIdx].vVx[nVertexIdx].P.y = -mv_Mesh[meshIdx].vVx[nVertexIdx].P.y; break;
			case AXIS::Z: mv_Mesh[meshIdx].vVx[nVertexIdx].P.z = -mv_Mesh[meshIdx].vVx[nVertexIdx].P.z; break;
			default: break;
			}
		}
	}
}

void Model::Scale(double scaleFactor, bool selected)
{
	// Overload of Scale(x,y,z) since typical scaling is the same along all axes.
	//
	// Inputs:
    //		scaleFactor = scale factor, 1.0 = 100%
	//		selected	= true to only scale selected vx (else all)
	//

	Scale(scaleFactor, scaleFactor, scaleFactor, selected);
}

void Model::Scale(double dx, double dy, double dz, bool selected)
{
	// Scale selected vertices
	//
	// Inputs:
	//		dx/dy/dz = scaling factors where 1.0 = 100%, 0.0 indicates that value
	//				   should not be scaled
	//		selected = true to only scale selected vx (else all)
	//

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			if ((selected == false) || (mv_Mesh[meshIdx].vVx[i].IsSelected))
			{
				if (dx)
					mv_Mesh[meshIdx].vVx[i].P.x *= dx;
				if (dy)
					mv_Mesh[meshIdx].vVx[i].P.y *= dy;
				if (dz)
					mv_Mesh[meshIdx].vVx[i].P.z *= dz;
			}
		}
	}
}

void Model::ScaleTo(double sx, double sy, double sz)
{
	// Scale model to given sizes along xyz axes.
	//
	// Inputs:
	//		sx/sy/sz = desired final sizes along each axis
	//

	VEC3 extMin, extMax;
	GetExtents(extMin, extMax);

	double scale_x = sx / (extMax.x - extMin.x);
	double scale_y = sy / (extMax.y - extMin.y);
	double scale_z = sz / (extMax.z - extMin.z);

	Scale(scale_x, scale_y, scale_z);
}

void Model::Rotate(AXIS axis, VEC3 vPoint, float delta)
{
	// rotate selected vertices
	//
	// inputs:
	//		axis  	= AXIS_... value
	//		vPoint 	= location of axis of rotation
	//		delta 	= radians to rotate
	//

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			if (mv_Mesh[meshIdx].vVx[i].IsSelected)
			{
				VEC3 vIn, vOut;
				vIn = mv_Mesh[meshIdx].vVx[i].P;
				switch (axis) {
				case AXIS::X: Rotate2d( delta, vIn.z, vIn.y, &vOut.z, &vOut.y, vPoint.z, vPoint.y ); vOut.x = vIn.x; break;
				case AXIS::Y: Rotate2d( delta, vIn.x, vIn.z, &vOut.x, &vOut.z, vPoint.x, vPoint.z ); vOut.y = vIn.y; break;
				case AXIS::Z: Rotate2d( delta, vIn.x, vIn.y, &vOut.x, &vOut.y, vPoint.x, vPoint.y ); vOut.z = vIn.z; break;
				default: break;
				}

				mv_Mesh[meshIdx].vVx[i].P = vOut;
			}
		}
	}
}

void Model::GetExtents(VEC3& vMinExtents, VEC3& vMaxExtents)
{
	vMinExtents = VEC3(0, 0, 0);
	vMaxExtents = VEC3(0, 0, 0);

	if (mv_Mesh.size() > 0)
	{
		// get extents of 0th mesh & default return values
		mv_Mesh[0].UpdateExtents();

		vMinExtents = mv_Mesh[0].vMinExt;
		vMaxExtents = mv_Mesh[0].vMaxExt;

		// scan rest of meshes
		for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
		{
			mv_Mesh[meshIdx].UpdateExtents();

			if (mv_Mesh[meshIdx].vMinExt.x < vMinExtents.x) vMinExtents.x = mv_Mesh[meshIdx].vMinExt.x;
			if (mv_Mesh[meshIdx].vMinExt.y < vMinExtents.y) vMinExtents.y = mv_Mesh[meshIdx].vMinExt.y;
			if (mv_Mesh[meshIdx].vMinExt.z < vMinExtents.z) vMinExtents.z = mv_Mesh[meshIdx].vMinExt.z;

			if (mv_Mesh[meshIdx].vMaxExt.x > vMaxExtents.x) vMaxExtents.x = mv_Mesh[meshIdx].vMaxExt.x;
			if (mv_Mesh[meshIdx].vMaxExt.y > vMaxExtents.y) vMaxExtents.y = mv_Mesh[meshIdx].vMaxExt.y;
			if (mv_Mesh[meshIdx].vMaxExt.z > vMaxExtents.z) vMaxExtents.z = mv_Mesh[meshIdx].vMaxExt.z;
		}
	}
}

void Model::GetExtents(const char* meshName, VEC3& vMinExtents, VEC3& vMaxExtents)
{
	vMinExtents = VEC3(0, 0, 0);
	vMaxExtents = VEC3(0, 0, 0);

	int meshIdx = GetMeshIndex(meshName);

	if (meshIdx > -1)
	{
		vMinExtents = mv_Mesh[meshIdx].vMinExt;
		vMaxExtents = mv_Mesh[meshIdx].vMaxExt;
	}
}

int Model::AddMesh()
{
	// Add new mesh.
	//
	// Outputs:
	//		return = index of new mesh, value only valid until
	//				 operation that modifies mesh list
	//

	Model::MeshType mesh;
	mv_Mesh.push_back(mesh);
	return static_cast<int>(mv_Mesh.size()) - 1;
}

const char* Model::GetMeshName(int meshIndex)
{
	// Return mesh name.
	//

	if (meshIndex < 0 || meshIndex > mv_Mesh.size() - 1)
		return nullptr;

	return (const char*)(&mv_Mesh[meshIndex].Name[0]);
}

int Model::GetMeshIndex(const char* meshName)
{
	// Convert mesh name to it's index.
	//

	int nReturn = -1;

	for (UInt32 i = 0; i < mv_Mesh.size(); i++)
	{
		if (strcmp(meshName, mv_Mesh[i].Name) == 0)
		{
			nReturn = i;
			break;
		}
	}

	return nReturn;
}

Model::MeshType* Model::GetMesh(int meshIndex)
{
	// Return ptr to mesh.
	//

	if (meshIndex >= 0 && meshIndex < GetMeshCount())
		return &mv_Mesh[meshIndex];

	return nullptr;
}

void Model::MergeMeshes()
{
	// Merge entire model into single mesh.
	//

	//TODO:
	//if model is big enough, this screws it up

	while (mv_Mesh.size() > 1)		// merge 1 into 0 until only 1 left
	{
		MergeMesh(0, 1);
	}
}

void Model::MergeMesh(int destIdx, int srcIdx)
{
	// Merge mesh srcIdx into mesh destIdx.
	//

	if (destIdx > (int)mv_Mesh.size() - 1)
		return;
	if (srcIdx > (int)mv_Mesh.size() - 1)
		return;

	MeshType* pSrc = &mv_Mesh[srcIdx];
	MeshType* pDest = &mv_Mesh[destIdx];

	// store current destination mesh sizes

	UInt32 nDestVxCnt = (UInt32)pDest->vVx.size();
	UInt32 nSrcVxCnt = (UInt32)pSrc->vVx.size();

	// move vertices

	for (UInt32 i = 0; i < nSrcVxCnt; ++i)
	{
		pDest->vVx.push_back(pSrc->vVx[i]);
	}

	// move faces

	for (UInt32 i = 0; i < pSrc->vFace.size(); ++i)
	{
		// update face indices

		pSrc->vFace[i].vx0 += nDestVxCnt;
		pSrc->vFace[i].vx1 += nDestVxCnt;
		pSrc->vFace[i].vx2 += nDestVxCnt;

		// move faces

		pDest->vFace.push_back(pSrc->vFace[i]);
	}

	// clean up

	pSrc->vFace.clear();			// face vectors
	pSrc->vVx.clear();				// vertex vectors

	mv_Mesh.erase(mv_Mesh.begin() + srcIdx);	// mesh vector
}

int Model::GetMeshCount()
{
	return static_cast<int>(mv_Mesh.size());
}

void Model::SetRenderMesh(int meshIndex, bool render)
{
	// Set switch indicating mesh[meshIndex]
	// should/should not be rendered.
	//

	if (meshIndex < 0 || meshIndex > mv_Mesh.size() - 1)
		return;

	mv_Mesh[meshIndex].IsVisible = render;
}

void Model::SetPosition(VEC3 vPos)
{
	// Sets the model center.
	//

	mv_Pos = vPos;
}

double Model::GetRadius()
{
	// Return minimum bounding radius, representing the mesh's bounding sphere.
	//

	if (mf_Radius < 0.0)		// not yet set
	{
		double radiusSq = 0.0;
		for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
		{
			for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
			{
				VEC3* pv = &mv_Mesh[meshIdx].vVx[i].P;
				double curRadiusSq = (pv->x*pv->x + pv->y*pv->y + pv->z*pv->z);
				if (curRadiusSq > radiusSq)
					radiusSq = curRadiusSq;
			}
		}
		mf_Radius = sqrt(radiusSq);
	}

	return mf_Radius;
}

void Model::DeSelectAll()
{
	// De-select everything.
	// 

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			mv_Mesh[meshIdx].vVx[i].IsSelected = false;
		}
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vFace.size(); ++i)	// for each face
		{
			mv_Mesh[meshIdx].vFace[i].IsSelected = false;
		}
	}
}


int Model::AddVertex(int meshIndex)
{
	// Add new vertex to mesh.
	//
	// Inputs:
	//		meshIndex = index of target mesh
	// Outputs:
	//		return = index of new vertex, relative to mesh; value only 
	//				 valid until operation that modifies face list
	//

	Model::VertexType vx;
	mv_Mesh[meshIndex].vVx.push_back(vx);
	return static_cast<int>(mv_Mesh[meshIndex].vVx.size()) - 1;
}

int Model::GetVertexCount(int meshIndex) const
{
	if (meshIndex < 0 || meshIndex > mv_Mesh.size() - 1)
		return 0;

	return static_cast<int>(mv_Mesh[meshIndex].vVx.size());
}

int Model::SelectVertex(VEC3 vMin, VEC3 vMax, bool clear /* = true */)
{
	// Select all vertices inside bounding box.
	//
	// Inputs:
	//		vMin/vMax 	= extents of bounding box
	//      clear 		= true => deselect other vertices first, else add to list of selected vertices
	// 
	// Outputs:
	//      return 		= total # of vertices selected
	//

	if (clear)
		DeSelectAll();		// de-select current items

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		// can only select visible items
		if (mv_Mesh[meshIdx].IsVisible == true)
		{
			for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
			{
				VEC3* pv = &mv_Mesh[meshIdx].vVx[i].P;

				if ((pv->x > vMin.x && pv->x < vMax.x) || (pv->x > vMax.x && pv->x < vMin.x))
				{
					if ((pv->y > vMin.y && pv->y < vMax.y) || (pv->y > vMax.y && pv->y < vMin.y))
					{
						if ((pv->z > vMin.z && pv->z < vMax.z) || (pv->z > vMax.z && pv->z < vMin.z))
						{
							// found one
							mv_Mesh[meshIdx].vVx[i].IsSelected = true;	// mark it
						}
					}
				}
			}
		}
	}

	return GetSelectedVertexCount();
}

int Model::GetSelectedVertexCount()
{
	// Return # of selected vertices.
	// 

	int vertexCount = 0;

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			if (mv_Mesh[meshIdx].vVx[i].IsSelected)
				++vertexCount;
		}
	}

	return vertexCount;
}

void Model::VertexEqual(AXIS axis)
{
	// Set all x, y or z components of selected 
	// vertices to same value.
	// 

	bool bFound = false;
	VEC3 vAnchor = VEC3(0, 0, 0);

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			if (mv_Mesh[meshIdx].vVx[i].IsSelected)
			{
				if (!bFound)
				{
					bFound = true;
					vAnchor = mv_Mesh[meshIdx].vVx[i].P;
				}
				else
				{
					switch (axis) {
					case AXIS::X: mv_Mesh[meshIdx].vVx[i].P.x = vAnchor.x; break;
					case AXIS::Y: mv_Mesh[meshIdx].vVx[i].P.y = vAnchor.y; break;
					case AXIS::Z: mv_Mesh[meshIdx].vVx[i].P.z = vAnchor.z; break;
					default: break;
					}
				}
			}
		}
	}
}

void Model::DeleteVertex()
{
	// delete selected vertices
	// 

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			if (mv_Mesh[meshIdx].vVx[i].IsSelected)
			{
				DeleteVertex( meshIdx, i );
				i--;	// set i back to re-process
			}
		}
	}

	// process may have left orphan vertices, clean them up here
	DeleteUnusedVertices();		// delete vertices that do not participate in any faces
}

void Model::DeleteVertex(int meshIdx, int vertexIdx)
{
	// private method that deletes a specific vertex and updates related data
	// structures as necessary
	// 
	// only updates related data in specified mesh because faces can't span meshes by definition
	//

	if ((meshIdx < 0) || (meshIdx >= GetMeshCount()))
		return;

	MeshType* pMesh = &mv_Mesh[meshIdx];	// the mesh we're interested in
	UInt32 nFaceIdx;

	// find out what faces the vertex participates in, they are now undefined
	// so they have to be deleted (not their other vx's, just the face def)

	for (nFaceIdx = 0; nFaceIdx < pMesh->vFace.size(); nFaceIdx++)
	{
		// if target vertex in this face
		if (vertexIdx == pMesh->vFace[nFaceIdx].vx0
			|| vertexIdx == pMesh->vFace[nFaceIdx].vx1
			|| vertexIdx == pMesh->vFace[nFaceIdx].vx2)
		{
			// delete the face
			pMesh->vFace.erase( pMesh->vFace.begin() + nFaceIdx );
			nFaceIdx--;	// set back to reprocess
		}
	}

	// update the face indices to account for the deleted vertex

	for (nFaceIdx = 0; nFaceIdx < pMesh->vFace.size(); nFaceIdx++)
	{
		// if target vertex in this face
		if (pMesh->vFace[nFaceIdx].vx0 > vertexIdx)	// pts to vx above deleted vx
			pMesh->vFace[nFaceIdx].vx0--;			// adjust it
		if (pMesh->vFace[nFaceIdx].vx1 > vertexIdx)
			pMesh->vFace[nFaceIdx].vx1--;
		if (pMesh->vFace[nFaceIdx].vx2 > vertexIdx)
			pMesh->vFace[nFaceIdx].vx2--;
	}

	// delete the vertex itself
	pMesh->vVx.erase( pMesh->vVx.begin() + vertexIdx );
}

int Model::DeleteUnusedVertices()
{
	// Private method that deletes any vertices that do
	//	not participate in any faces.
	//
	// returns # of vertices deleted
	//

	int nReturn = 0;

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];	// the mesh we're interested in

		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			// see if vertex participates in any faces

			bool bFound = false;
			for (unsigned int faceIdx = 0; faceIdx < pMesh->vFace.size(); ++faceIdx)		// for each face
			{
				// if target vertex in this face
				if (i == pMesh->vFace[faceIdx].vx0
					|| i == pMesh->vFace[faceIdx].vx1
					|| i == pMesh->vFace[faceIdx].vx2)
				{
					bFound = true;	// ok, this one's good
					break;
				}
			}

			if (!bFound)		// this vertex does not appear in any faces
			{
				DeleteVertex( meshIdx, i );
				nReturn++;
			}
		}
	}

	return nReturn;
}

int Model::DeleteEqualVertices()
{
	// Private method that deletes any vertices that are in exactly
	// the same position as another vertex.
	//
	// Outputs:
	//		returns = # of vertices deleted
	//

	int nReturn = 0;

	for (int meshIdx = 0; meshIdx < GetMeshCount(); ++meshIdx)		// for each mesh
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];	// the mesh we're interested in

		for (int i = 0; i < static_cast<int>(pMesh->vVx.size()); ++i)		// for each vertex
		{
			for (int j = 0; j < static_cast<int>(pMesh->vVx.size()); ++j)	// for each vertex
			{
				if (pMesh->vVx[i].P.x == pMesh->vVx[j].P.x
					&& pMesh->vVx[i].P.y == pMesh->vVx[j].P.y
					&& pMesh->vVx[i].P.z == pMesh->vVx[j].P.z
					&& i != j)
				{
					for (unsigned int nFaceIdx = 0; nFaceIdx < pMesh->vFace.size(); ++nFaceIdx)
					{
						// if target vertex in this face

						if (pMesh->vFace[nFaceIdx].vx0 == j)		// pts to j
							pMesh->vFace[nFaceIdx].vx0 = i;			//		point it to vx we're keeping
						else if (pMesh->vFace[nFaceIdx].vx0 > j)	// pts above j
							pMesh->vFace[nFaceIdx].vx0--;			//		adjust it

						if (pMesh->vFace[nFaceIdx].vx1 == j)
							pMesh->vFace[nFaceIdx].vx1 = i;
						else if (pMesh->vFace[nFaceIdx].vx1 > j)	// pts above j
							pMesh->vFace[nFaceIdx].vx1--;

						if (pMesh->vFace[nFaceIdx].vx2 == j)
							pMesh->vFace[nFaceIdx].vx2 = i;
						else if (pMesh->vFace[nFaceIdx].vx2 > j)	// pts above j
							pMesh->vFace[nFaceIdx].vx2--;
					}

					// delete the vertex (j)
					pMesh->vVx.erase( pMesh->vVx.begin() + j );

					nReturn++;

					break;
				}
			}
		}
	}

	return nReturn;
}

void Model::MoveVertex(double dx, double dy, double dz, bool selected /* = true */)
{
	// Move selected vertices.
	//
    // Inputs:
	//        dx = change in X value
	//        dy = change in Y value
	//        dz = change in Z value
	//        selected = true => to move selected vertices, else move all vertices
	//

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			if ((selected && mv_Mesh[meshIdx].vVx[i].IsSelected) || (!selected))
			{
				mv_Mesh[meshIdx].vVx[i].P.x += dx;
				mv_Mesh[meshIdx].vVx[i].P.y += dy;
				mv_Mesh[meshIdx].vVx[i].P.z += dz;
			}
		}
	}
}

void Model::Center()
{
	// Center the model along all axes.
	//

	VEC3 vMin, vMax;
	GetExtents( vMin, vMax );

	float dx = (float)(vMax.x - ((vMax.x-vMin.x) * 0.5F));
	float dy = (float)(vMax.y - ((vMax.y-vMin.y) * 0.5F));
	float dz = (float)(vMax.z - ((vMax.z-vMin.z) * 0.5F));

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			mv_Mesh[meshIdx].vVx[i].P.x -= dx;
			mv_Mesh[meshIdx].vVx[i].P.y -= dy;
			mv_Mesh[meshIdx].vVx[i].P.z -= dz;
		}
	}
}

void Model::MergeVertices()
{
	// Merge selected vertices.
	//

	// by definition, all vertices have to be in same mesh

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		bool bFound = false;
		int nMergeDest = 0;
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vVx.size(); ++i)		// for each vertex
		{
			if (mv_Mesh[meshIdx].vVx[i].IsSelected)
			{
				if (!bFound)
				{
					nMergeDest = i;
					bFound = true;
				}
				else
				{
					// point all face indices to dest vertex

					for (UInt32 faceIdx = 0; faceIdx < mv_Mesh[meshIdx].vFace.size(); faceIdx++)
					{
						if (mv_Mesh[meshIdx].vFace[faceIdx].vx0 == i)				// if used old vertex
							mv_Mesh[meshIdx].vFace[faceIdx].vx0 = nMergeDest;		// point to new vertex
						if (mv_Mesh[meshIdx].vFace[faceIdx].vx1 == i)
							mv_Mesh[meshIdx].vFace[faceIdx].vx1 = nMergeDest;
						if (mv_Mesh[meshIdx].vFace[faceIdx].vx2 == i)
							mv_Mesh[meshIdx].vFace[faceIdx].vx2 = nMergeDest;
					}

					DeleteVertex( meshIdx, i );
					i--;							// dec to reprocess
				}
			}
		}
	}
}

Model::VertexType* Model::GetVertex(int meshIndex, int vertexIndex)
{
	VertexType* pVx = nullptr;

	if (meshIndex >= 0 && meshIndex < GetMeshCount())
	{
		if (vertexIndex >= 0 && vertexIndex < GetVertexCount(meshIndex))
		{
			pVx = &mv_Mesh[meshIndex].vVx[vertexIndex];
		}
	}

	return pVx;
}

int Model::GetFaceCount(int meshIndex) const
{
	if (meshIndex < 0 || meshIndex > mv_Mesh.size() - 1)
		return 0;

	return static_cast<int>(mv_Mesh[meshIndex].vFace.size());
}

Model::FaceType* Model::GetFace(int meshIndex, int faceIndex)
{
	// Return ptr to face structure.
	//

	FaceType* pFace = nullptr;

	if (meshIndex >= 0 && meshIndex < GetMeshCount())
	{
		if (faceIndex >= 0 && faceIndex < GetFaceCount(meshIndex))
		{
			pFace = &mv_Mesh[meshIndex].vFace[faceIndex];
		}
	}

	return pFace;
}

int Model::GetFaceCount() const
{
	int faceCount = 0;

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		faceCount += (int)(mv_Mesh[meshIdx].vFace.size());
	}

	return faceCount;
}

int Model::AddFace(int meshIndex)
{
	// Add new face to mesh.
	//
	// Inputs:
	//		meshIndex = index of target mesh
	// Outputs:
	//		return = index of new face, relative to mesh; value only 
	//				 valid until operation that modifies face list
	//

	Model::FaceType face;
	mv_Mesh[meshIndex].vFace.push_back(face);
	return static_cast<int>(mv_Mesh[meshIndex].vFace.size()) - 1;
}

int Model::DeleteFaces()
{
	// Delete selected faces.
	//
	// Method deletes faces only, not vertices contained in face - it will, however remove any
	// orphaned vertices left alone by the face deletion(s).
	//
	// Outputs:
	//		return = # of faces deleted
	//

	int deleteCtr = 0;

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vFace.size(); ++i)	// for each face
		{
			if (pMesh->vFace[i].IsSelected)
			{
				// delete the face
				pMesh->vFace.erase(pMesh->vFace.begin() + i);

				i--;	// set back to reprocess

				deleteCtr++;
			}
		}
	}

	//TODO:
	//delete unused vertices - happens anyway in save

	return deleteCtr;
}

void Model::SelectFace(int meshIdx, int faceIdx)
{
	// Select a face.
	//
	// Inputs:
	//		meshIdx = index of mesh
	//		faceIdx = index of face
	//

	DeSelectAll();

	if ((meshIdx >= 0) && (meshIdx < GetMeshCount()))
	{
		if ((faceIdx >= 0) && (faceIdx < GetFaceCount(meshIdx)))
		{
			mv_Mesh[meshIdx].vFace[faceIdx].IsSelected = true;
		}
	}
}

bool Model::FacePick(VEC3 rayOrg, VEC3 rayDir, int& meshIdx, int& faceIdx)
{
	// Pick face by shooting ray through model.
	//
	// Inputs:
	//		rayOrg = origin of pick ray
	//		rayDir = direction of pick ray
	//
	// Outputs:
	//		ai_MeshIdx = mesh containing ai_MeshIdx, -1 if ray hit nothing
	//		ai_FaceIdx = index of face within mesh, -1 if ray hit nothing
	//		return = true if ray intersected face
	//

	meshIdx = 0;
	faceIdx = 0;

	DeSelectAll();

//TODO:
//function has serious issues
return false;

	// Get the picked triangle

	//TODO:
	//should base on known valid values
	double minDist = 1000000.0;		// distance from screen to current face

	bool selected = false;
	for (int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)
	{
		for (int i = 0; i < GetFaceCount(meshIdx); ++i)
		{
			// get the 3 vx's in vx0,1,2
			int nVx1 = mv_Mesh[meshIdx].vFace[i].vx0;
			int nVx2 = mv_Mesh[meshIdx].vFace[i].vx1;
			int nVx3 = mv_Mesh[meshIdx].vFace[i].vx2;

			VEC3 V0 = mv_Mesh[meshIdx].vVx[nVx1].P;
			VEC3 V1 = mv_Mesh[meshIdx].vVx[nVx2].P;
			VEC3 V2 = mv_Mesh[meshIdx].vVx[nVx3].P;

			//TODO:
			//will want to apply any model/mesh rotations here as well

			// translate the face to it's actual position in world space
			V0 += mv_Pos;
			V1 += mv_Pos;
			V2 += mv_Pos;

			// Check if the pick ray passes through this point

			VEC3 vIntersect;
			if (IntersectRayTriangle(rayOrg, rayDir, V0, V1, V2, &vIntersect) == 1)
			{
				// it does, see if it's the closest of the intersecting faces

				// just using 0th vx for distance, i suppose avg might be more accurate
				VEC3 dist = rayOrg - vIntersect;
				double d = Magnitude(dist);

				// regardless of front or back pick, always want to select the
				// face closest to viewer
				if (d < minDist)
				{
					meshIdx = meshIdx;
					faceIdx = i;
					minDist = d;
					selected = true;
				}
			}

		}	// next i
	}		// next meshIdx

	if (selected)
		mv_Mesh[meshIdx].vFace[faceIdx].IsSelected = true;

	return selected;
}

void Model::ReverseWindingOrder(bool selected /* = true */)
{
	// Reverse winding order of faces.
	//
	// Inputs:
	//		selected = true => flip selected faces only
	//			     = false => flip all faces
	//

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vFace.size(); ++i)	// for each face
		{
			if ((selected && mv_Mesh[meshIdx].vFace[i].IsSelected) || (!selected))
			{
				std::swap(mv_Mesh[meshIdx].vFace[i].vx0, mv_Mesh[meshIdx].vFace[i].vx1);
			}
		}
	}
}

int Model::Tesselate(bool selected /* = true */)
{
	// Tesselate selected faces.
	//
	// Inputs:
	//		selected = true => tesselate selected faces only
	//			     = false => tesselate all faces
	// Outputs:
	//		return = # of faces added
	//

	RenderOff();		// don't allow rendering while modifying data structures

	std::vector<FaceType>newFaces;

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];

		for (unsigned int i = 0; i < mv_Mesh[meshIdx].vFace.size(); ++i)	// for each face
		{
			if ((selected && pMesh->vFace[i].IsSelected) || (!selected))
			{
				// tesselate me

				//	subdivide()
				//
				//             vx0
				//              +
				//             / \
				//            /   \         - each triangle subdivides into 4 triangles
				//           /  0  \
				//      VxC /-------\ vxA
				//         / \  3  / \
				//        /   \   /   \
				//       / 2   \ /  1  \
				//  vx2 +---------------+ vx1
				//             VxB

				// - bisect each side of current face to create 3 new vx

				// - add new vertices to vertex list

				// - reuse vertices and indices of vertices created on prior calls to subdivide
				// (otherwise you create 2 new vx and 2 new indices every time you bisect a line!)
				//

				// get face we're going to tesselate
				FaceType* pFace = &pMesh->vFace[i];

				// process of tesselation for single face involves:
				//		1. creation of 3 new bisecting vertices
				//		2. deletion of original face
				//		3. addition of 4 new faces

				// side 1
				int vxIndexA = TessFindVertex(pMesh->vFace, pFace->vx0, pFace->vx1);
				if (vxIndexA == -1)
				{
					// vertex that bisects this side not found, create it
					VertexType vxA = Interpolate(pMesh->vVx[pFace->vx0], pMesh->vVx[pFace->vx1]);

					// add it directly to the source vertex list
					pMesh->vVx.push_back(vxA);
					vxIndexA = (int)pMesh->vVx.size() - 1;
				}
				pFace->vx01 = vxIndexA;
				
				// side 2
				int vxIndexB = TessFindVertex(pMesh->vFace, pFace->vx1, pFace->vx2);
				if (vxIndexB == -1)
				{
					VertexType vxB = Interpolate(pMesh->vVx[pFace->vx1], pMesh->vVx[pFace->vx2]);
					pMesh->vVx.push_back(vxB);
					vxIndexB = (int)pMesh->vVx.size() - 1;
				}
				pFace->vx12 = vxIndexB;

				// side 3
				int vxIndexC = TessFindVertex(pMesh->vFace, pFace->vx2, pFace->vx0);
				if (vxIndexC == -1)
				{
					VertexType vxC = Interpolate(pMesh->vVx[pFace->vx2], pMesh->vVx[pFace->vx0]);
					pMesh->vVx.push_back(vxC);
					vxIndexC = (int)pMesh->vVx.size() - 1;
				}
				pFace->vx20 = vxIndexC;

				// replace old face with the 4 new faces
				FaceType face0(pFace->vx0, (UInt16)vxIndexA, (UInt16)vxIndexC);
				FaceType face1((UInt16)vxIndexA, pFace->vx1, (UInt16)vxIndexB);
				FaceType face2((UInt16)vxIndexC, (UInt16)vxIndexB, pFace->vx2);
				FaceType face3((UInt16)vxIndexA, (UInt16)vxIndexB, (UInt16)vxIndexC);

				*pFace = face0;					// replace this face

				newFaces.push_back(face1);		// add 3 new faces
				newFaces.push_back(face2);
				newFaces.push_back(face3);
			}
		}
		
		// add new faces to face list
		pMesh->vFace.insert(pMesh->vFace.end(), newFaces.begin(), newFaces.end());
	}

	RenderOn();

	return (int)newFaces.size();
}

int Model::TessFindVertex(const std::vector<FaceType>& faceList, UInt16 vx1, UInt16 vx2)
{
	// Tesselation helper function.
	//
	// see if side bisected by <vx0,vx1> has already been bisected
	//
	// Inputs:
	//		faceList = input face list
	//		vx0,vx1  = pair of connected vertices in input face list
	//
	// Outputs:
	//		return = index of bisecting vertex if <vx0,vx1> has been bisected
	//				 else -1
	//

	for (unsigned int faceIdx = 0; faceIdx < faceList.size(); ++faceIdx)
	{
		const FaceType* pFace = &faceList[faceIdx];

		bool isBisected = false;
		if (pFace->vx01 != 0xFFFFFFFF)
		{
			if ((pFace->vx0 == vx1 && pFace->vx1 == vx2)
			||  (pFace->vx1 == vx1 && pFace->vx0 == vx2))
			{
				// these vertices define a side that has already been bisected,
				// return the index of that bisecting vertex
				return pFace->vx01;
			}
			isBisected = true;
		}
		if (pFace->vx12 != 0xFFFFFFFF)
		{
			if ((pFace->vx1 == vx1 && pFace->vx2 == vx2)
			||  (pFace->vx2 == vx1 && pFace->vx1 == vx2))
			{
				// these vertices define a side that has already been bisected,
				// return the index of that bisecting vertex
				return pFace->vx12;
			}
			isBisected = true;
		}
		if (pFace->vx20 != 0xFFFFFFFF)
		{
			if ((pFace->vx2 == vx1 && pFace->vx0 == vx2)
			||  (pFace->vx0 == vx1 && pFace->vx2 == vx2))
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

Model::VertexType Model::Interpolate(VertexType vx1, VertexType vx2)
{
	// Interpolate vertices.

	VertexType vx;	// return value

	vx.P = (vx1.P + vx2.P) * 0.5F;
	vx.N = (vx1.N + vx2.N) * 0.5F;
	
	return vx;
}

int Model::GetVertexCount() const
{
	int vertexCount = 0;

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		vertexCount += (int)mv_Mesh[meshIdx].vVx.size();
	}

	return vertexCount;
}

int Model::AddTexCoord(int meshIndex)
{
	// Add new texture coordinate to mesh.
	//
	// Inputs:
	//		meshIndex = index of target mesh
	// Outputs:
	//		return = index of new texture coordinate entry, 
	//				 relative to mesh; value only valid until 
	//				 operation that modifies face list
	//

	Model::TexCoord tc;
	mv_Mesh[meshIndex].vTex.push_back(tc);
	return static_cast<int>(mv_Mesh[meshIndex].vTex.size()) - 1;
}

int Model::GetTexCoordCount(int meshIndex)
{
	return static_cast<int>(mv_Mesh[meshIndex].vTex.size());
}

Model::TexCoord* Model::GetTexCoord(int meshIndex, int tcIndex)
{
	return &mv_Mesh[meshIndex].vTex[tcIndex];
}

int Model::GetMaterialCount()
{
	// Return # of materials defined.
	//

	return static_cast<int>(mv_Mat.size());
}

Material* Model::GetMaterial(int matIndex)
{
	// Return material by index.
	//

	if (matIndex < 0 || matIndex >= GetMaterialCount())
		return nullptr;

	return &mv_Mat[matIndex];
}

int Model::AddMaterial(Material& mat)
{
	// Add new material to model.
	//
	// Inputs:
	//		mat = material lto add
	// Outputs:
	//		return = index of new material entry
	//

	// names can't be blank
	if (mat.GetName().IsEmpty())
	{
		char buf[32] = {0};
		sprintf(buf, "xmat%d", GetMaterialCount());
		mat.SetName(buf);
	}
	mv_Mat.push_back(mat);
	return static_cast<int>(mv_Mat.size()) - 1;
}

void Model::Clone(VEC3 vPos)
{
	// Create a mesh based on selected vertices at vPos.
	//

	if (GetSelectedVertexCount() < 1)
		return;

	unsigned int nMapIdx, i;

	// new mesh

	MeshType mesh;
	sprintf(mesh.Name, "Mesh%lu", (unsigned long)mv_Mesh.size());	// default name

	mesh.vVx.clear();
	mesh.vFace.clear();

	// these are used to track the source of vertices added to the new mesh

	std::vector<UInt16> vSrcMesh;	// index == vVx[] index in new mesh, value == source mesh index
	std::vector<UInt16> vSrcVx;		// index same as vSrcMesh, value = source vertex index
	vSrcMesh.clear();
	vSrcVx.clear();

	// identify all faces that have at least 1 selected vertex

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		MeshType* pSrc = &mv_Mesh[meshIdx];

		// add the faces to face list

		for (UInt32 nFaceIdx = 0; nFaceIdx < pSrc->vFace.size(); ++nFaceIdx)
		{
			// if any of the faces contain selected vertices

			int vx0 = pSrc->vFace[nFaceIdx].vx0;
			int vx1 = pSrc->vFace[nFaceIdx].vx1;
			int vx2 = pSrc->vFace[nFaceIdx].vx2;

			if (pSrc->vVx[vx0].IsSelected || pSrc->vVx[vx1].IsSelected || pSrc->vVx[vx2].IsSelected)
			{
				FaceType face;
				face.N = pSrc->vFace[nFaceIdx].N;

				// add, if necessary, face's vertices

				// vertex 1

				// search src/dest map to see if this vertex has already been added
				// to the new mesh

				int nSrcIdx = -1;
				for (nMapIdx = 0; nMapIdx < vSrcVx.size(); ++nMapIdx)
				{
					if (vSrcMesh[nMapIdx] == meshIdx && vSrcVx[nMapIdx] == vx0)	// found vx1
					{
						nSrcIdx = nMapIdx;	// vx already in new mesh, nMapIdx == it's new index
						break;
					}
				}

				if (nSrcIdx < 0)		// vertex not yet in new mesh
				{
					// add it to our new mesh
					mesh.vVx.push_back( pSrc->vVx[vx0] );

					// track it's source
					vSrcMesh.push_back( meshIdx );
					vSrcVx.push_back( vx0 );

					face.vx0 = (UInt32)(mesh.vVx.size()-1);
				}
				else					// vx already in new mesh, just point .Vx1 to it
				{
					face.vx0 = nSrcIdx;
				}

				// vertex 2

				nSrcIdx = -1;
				for (nMapIdx = 0; nMapIdx < vSrcVx.size(); nMapIdx++)
				{
					if (vSrcMesh[nMapIdx] == meshIdx && vSrcVx[nMapIdx] == vx1)	// found Vx2
					{
						nSrcIdx = nMapIdx;
						break;
					}
				}

				if (nSrcIdx < 0)		// vertex not yet in new mesh
				{
					// add it to our new mesh
					mesh.vVx.push_back(pSrc->vVx[vx1]);

					// track it's source
					vSrcMesh.push_back(meshIdx);
					vSrcVx.push_back(vx1);

					face.vx1 = (UInt32)(mesh.vVx.size()-1);
				}
				else
				{
					face.vx1 = nSrcIdx;
				}

				// vertex 3

				nSrcIdx = -1;
				for (nMapIdx = 0; nMapIdx < vSrcVx.size(); nMapIdx++)
				{
					if (vSrcMesh[nMapIdx] == meshIdx && vSrcVx[nMapIdx] == vx2)	// found Vx3
					{
						nSrcIdx = nMapIdx;
						break;
					}
				}

				if (nSrcIdx < 0)		// vertex not yet in new mesh
				{
					// add it to our new mesh
					mesh.vVx.push_back(pSrc->vVx[vx2]);

					// track it's source
					vSrcMesh.push_back( meshIdx );
					vSrcVx.push_back( vx2 );

					face.vx2 = (UInt32)mesh.vVx.size()-1;
				}
				else
				{
					face.vx2 = nSrcIdx;
				}

				// add the face to our new mesh

				mesh.vFace.push_back(face);

			}	// if face contains selected vertices
		}		// next face index
	}			// next mesh index

	DeSelectAll();		// deselect everything

	for (i = 0; i < mesh.vVx.size(); ++i)
	{
		// select new mesh
		mesh.vVx[i].IsSelected = true;

		// offset to new position

		mesh.vVx[i].P.x += vPos.x;
		mesh.vVx[i].P.y += vPos.y;
		mesh.vVx[i].P.z += vPos.z;
	}

	// add new mesh to our model

	mv_Mesh.push_back(mesh);

	// clean up

	vSrcMesh.clear();
	vSrcVx.clear();

	mesh.vVx.clear();
	mesh.vFace.clear();
}

void Model::ExchangeAxes(AXIS src, AXIS dest, bool flipSigns)
{
	// Re-orient the model by exchanging 2 axes.
	//
	// Inputs:
	//		src, dest	= axes to exchange
	//		flipSigns	= negate signs in process

	for (unsigned int meshIdx = 0; meshIdx < mv_Mesh.size(); ++meshIdx)		// for each mesh
	{
		MeshType* pMesh = &mv_Mesh[meshIdx];
		for (unsigned int i = 0; i < pMesh->vVx.size(); ++i)
		{
			switch (src)
			{
			case AXIS::X:
				if (flipSigns)
					pMesh->vVx[i].P.x = -pMesh->vVx[i].P.x;
				switch (dest)
				{
				case AXIS::Y:
					std::swap(pMesh->vVx[i].P.x, pMesh->vVx[i].P.y);
					break;
				case AXIS::Z:
					std::swap(pMesh->vVx[i].P.x, pMesh->vVx[i].P.z);
					break;
				}
				break;
			case AXIS::Y:
				if (flipSigns)
					pMesh->vVx[i].P.y = -pMesh->vVx[i].P.y;
				switch (dest)
				{
				case AXIS::X:
					std::swap(pMesh->vVx[i].P.y, pMesh->vVx[i].P.x);
					break;
				case AXIS::Z:
					std::swap(pMesh->vVx[i].P.y, pMesh->vVx[i].P.z);
					break;
				}
				break;
			case AXIS::Z:
				if (flipSigns)
					pMesh->vVx[i].P.z = -pMesh->vVx[i].P.z;
				switch (dest)
				{
				case AXIS::X:
					std::swap(pMesh->vVx[i].P.z, pMesh->vVx[i].P.x);
					break;
				case AXIS::Y:
					std::swap(pMesh->vVx[i].P.z, pMesh->vVx[i].P.y);
					break;
				}
				break;
			}
		}
	}
}

void Model::AddBlock(VEC3 vPos)
{
	// Create a new cube primitive as a new mesh.
	//

	MeshType mesh;
	sprintf( mesh.Name, "mesh%lu", (unsigned long)mv_Mesh.size() );	// default name

	mesh.vVx.clear();

	mesh.vVx.push_back( VertexType(VEC3(-1, -1, -1), VEC3(-0.57735, -0.57735, -0.57735) ));
	mesh.vVx.push_back( VertexType(VEC3( 1, -1, -1), VEC3( 0.33333, -0.66667, -0.66667) ));
	mesh.vVx.push_back( VertexType(VEC3(-1,  1, -1), VEC3(-0.66667,  0.33333, -0.66667) ));
	mesh.vVx.push_back( VertexType(VEC3( 1,  1, -1), VEC3( 0.66667,  0.66667, -0.33333) ));
	mesh.vVx.push_back( VertexType(VEC3(-1, -1,  1), VEC3(-0.66667, -0.66667,  0.33333) ));
	mesh.vVx.push_back( VertexType(VEC3( 1, -1,  1), VEC3( 0.66667, -0.33333,  0.66667) ));
	mesh.vVx.push_back( VertexType(VEC3(-1,  1,  1), VEC3(-0.33333,  0.66667,  0.66667) ));
	mesh.vVx.push_back( VertexType(VEC3( 1,  1,  1), VEC3( 0.57735,  0.57735,  0.57735) ));

	UInt32 vxCount = (UInt32)mesh.vVx.size();
	for (UInt32 i = 0; i < vxCount; i++)
	{
		// translate to initial position
		mesh.vVx[i].P.x += vPos.x;
		mesh.vVx[i].P.y += vPos.y;
		mesh.vVx[i].P.z += vPos.z;
	}

	mesh.vFace.push_back( FaceType( 1, 2, 3 ));
	mesh.vFace.push_back( FaceType( 2, 1, 0 ));
	mesh.vFace.push_back( FaceType( 4, 5, 6 ));
	mesh.vFace.push_back( FaceType( 6, 5, 7 ));
	mesh.vFace.push_back( FaceType( 3, 2, 6 ));
	mesh.vFace.push_back( FaceType( 3, 6, 7 ));
	mesh.vFace.push_back( FaceType( 0, 1, 4 ));
	mesh.vFace.push_back( FaceType( 4, 1, 5 ));
	mesh.vFace.push_back( FaceType( 2, 0, 4 ));
	mesh.vFace.push_back( FaceType( 2, 4, 6 ));
	mesh.vFace.push_back( FaceType( 1, 3, 5 ));
	mesh.vFace.push_back( FaceType( 5, 3, 7 ));

	// add it to the model

	mv_Mesh.push_back( mesh );

	CalculateNormals();		// create / update normals
}

void Model::AddCylinder(VEC3 vPos)
{
	// Create a new cylinder primitive as a new mesh.
	//

	MeshType mesh;
	sprintf( mesh.Name, "mesh%lu", (unsigned long)mv_Mesh.size() );	// default name

	mesh.vVx.clear();

	mesh.vVx.push_back( VertexType(VEC3(-0.2778, 0.0,  0.4252), VEC3(-0.3996, -0.5714,  0.7168) ));
	mesh.vVx.push_back( VertexType(VEC3(-0.4598, 0.0,  0.1830), VEC3(-0.6267, -0.7234,  0.2898) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.3112, 0.0,  0.4252), VEC3( 0.4571, -0.7223,  0.5190) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.0167, 0.0,  0.5177), VEC3( 0.0984, -0.3282,  0.9395) ));
	mesh.vVx.push_back( VertexType(VEC3(-0.4598, 0.0, -0.1162), VEC3(-0.6750, -0.7234, -0.1449) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.4932, 0.0,  0.1830), VEC3( 0.6750, -0.7234,  0.1449) ));
	mesh.vVx.push_back( VertexType(VEC3(-0.2778, 0.0, -0.3584), VEC3(-0.4571, -0.7223, -0.5190) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.4932, 0.0, -0.1162), VEC3( 0.6267, -0.7234, -0.2898) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.0167, 0.0, -0.4509), VEC3(-0.0984, -0.3282, -0.9395) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.3112, 0.0, -0.3584), VEC3( 0.3996, -0.5714, -0.7168) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.0167, 1.0,  0.5177), VEC3(-0.0855,  0.5707,  0.8167) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.3112, 1.0,  0.4252), VEC3( 0.3368,  0.7223,  0.6041) ));
	mesh.vVx.push_back( VertexType(VEC3(-0.4598, 1.0,  0.1830), VEC3(-0.6750,  0.7234,  0.1449) ));
	mesh.vVx.push_back( VertexType(VEC3(-0.2778, 1.0,  0.4252), VEC3(-0.6242,  0.3288,  0.7087) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.4932, 1.0,  0.1830), VEC3( 0.6267,  0.7234,  0.2898) ));
	mesh.vVx.push_back( VertexType(VEC3(-0.4598, 1.0, -0.1162), VEC3(-0.6267,  0.7234, -0.2898) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.4932, 1.0, -0.1162), VEC3( 0.6750,  0.7234, -0.1449) ));
	mesh.vVx.push_back( VertexType(VEC3(-0.2778, 1.0, -0.3584), VEC3(-0.3368,  0.7223, -0.6040) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.3112, 1.0, -0.3584), VEC3( 0.6242,  0.3288, -0.7087) ));
	mesh.vVx.push_back( VertexType(VEC3( 0.0167, 1.0, -0.4509), VEC3( 0.0855,  0.5707, -0.8167) ));

	UInt32 vxCount = (UInt32)mesh.vVx.size();
	for (UInt32 i = 0; i < vxCount; ++i)
	{
		// scale
		mesh.vVx[i].P *= 4.0;

		// translate to initial position
		mesh.vVx[i].P += vPos;
	}

	mesh.vFace.push_back( FaceType(  0,	  1,  2 ) );
	mesh.vFace.push_back( FaceType(  0,	  2,  3 ) );
	mesh.vFace.push_back( FaceType(  1,	  4,  5 ) );
	mesh.vFace.push_back( FaceType(  1,	  5,  2 ) );
	mesh.vFace.push_back( FaceType(  4,	  6,  7 ) );
	mesh.vFace.push_back( FaceType(  4,	  7,  5 ) );
	mesh.vFace.push_back( FaceType(  6,	  8,  9 ) );
	mesh.vFace.push_back( FaceType(  6,	  9,  7 ) );
	mesh.vFace.push_back( FaceType( 10,	 11, 12 ) );
	mesh.vFace.push_back( FaceType( 10,	 12, 13 ) );
	mesh.vFace.push_back( FaceType( 11,	 14, 15 ) );
	mesh.vFace.push_back( FaceType( 11,	 15, 12 ) );
	mesh.vFace.push_back( FaceType( 14,	 16, 17 ) );
	mesh.vFace.push_back( FaceType( 14,	 17, 15 ) );
	mesh.vFace.push_back( FaceType( 16,	 18, 19 ) );
	mesh.vFace.push_back( FaceType( 16,	 19, 17 ) );
	mesh.vFace.push_back( FaceType(  3,	  2, 11 ) );
	mesh.vFace.push_back( FaceType(  3,	 11, 10 ) );
	mesh.vFace.push_back( FaceType(  2,	  5, 14 ) );
	mesh.vFace.push_back( FaceType(  2,	 14, 11 ) );
	mesh.vFace.push_back( FaceType(  5,	  7, 16 ) );
	mesh.vFace.push_back( FaceType(  5,	 16, 14 ) );
	mesh.vFace.push_back( FaceType(  7,	  9, 18 ) );
	mesh.vFace.push_back( FaceType(  7,	 18, 16 ) );
	mesh.vFace.push_back( FaceType(  9,	  8, 19 ) );
	mesh.vFace.push_back( FaceType(  9,	 19, 18 ) );
	mesh.vFace.push_back( FaceType(  8,	  6, 17 ) );
	mesh.vFace.push_back( FaceType(  8,	 17, 19 ) );
	mesh.vFace.push_back( FaceType(  6,	  4, 15 ) );
	mesh.vFace.push_back( FaceType(  6,	 15, 17 ) );
	mesh.vFace.push_back( FaceType(  4,	  1, 12 ) );
	mesh.vFace.push_back( FaceType(  4,	 12, 15 ) );
	mesh.vFace.push_back( FaceType(  1,	  0, 13 ) );
	mesh.vFace.push_back( FaceType(  1,	 13, 12 ) );
	mesh.vFace.push_back( FaceType(  0,	  3, 10 ) );
	mesh.vFace.push_back( FaceType(  0,	 10, 13 ) );

	// add it to the model

	mv_Mesh.push_back( mesh );

	CalculateNormals();		// create / update normals
}

void Model::AddSphere(VEC3 vPos)
{
	// Create a new sphere primitive as a new mesh.
	//

	MeshType mesh;
	sprintf( mesh.Name, "mesh%lu", (unsigned long)mv_Mesh.size() );	// default name

	mesh.vVx.push_back(VertexType(VEC3(-1.23413,  1.23438, -1.23413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.48413,  0.39453, -1.48413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.99976,  0.53516, -0.53491), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.48413,  1.48438, -0.39429), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.48413, -0.39429, -1.48413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.99976, -0.53491, -0.53491), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.23413, -1.23413, -1.23413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.48413, -1.48413, -0.39429), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.99976,  0.53516,  0.53516), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.48413,  1.48438,  0.39453), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.99976, -0.53491,  0.53516), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.48413, -1.48413,  0.39453), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.48413,  0.39453,  1.48438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.23413,  1.23438,  1.23438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.48413, -0.39429,  1.48438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-1.23413, -1.23413,  1.23438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.39429, -1.48413, -1.48413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.53491, -1.99976, -0.53491), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.39453, -1.48413, -1.48413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.53516, -1.99976, -0.53491), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.23438, -1.23413, -1.23413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.48438, -1.48413, -0.39429), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.53491, -1.99976,  0.53516), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.53516, -1.99976,  0.53516), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.48438, -1.48413,  0.39453), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.39429, -1.48413,  1.48438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.39453, -1.48413,  1.48438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.23438, -1.23413,  1.23438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.39429,  1.48438, -1.48413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.53491,  0.53516, -1.99976), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.39453,  1.48438, -1.48413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.53516,  0.53516, -1.99976), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.23438,  1.23438, -1.23413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.48438,  0.39453, -1.48413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.53491, -0.53491, -1.99976), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.53516, -0.53491, -1.99976), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.48438, -0.39429, -1.48413), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.48438,  1.48438, -0.39429), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 2.00000,  0.53516, -0.53491), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.48438,  1.48438,  0.39453), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 2.00000,  0.53516,  0.53516), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.23438,  1.23438,  1.23438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.48438,  0.39453,  1.48438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 2.00000, -0.53491, -0.53491), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 2.00000, -0.53491,  0.53516), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 1.48438, -0.39429,  1.48438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.53491,  2.00000, -0.53491), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.53491,  2.00000,  0.53516), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.39429,  1.48438,  1.48438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.53516,  2.00000, -0.53491), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.53516,  2.00000,  0.53516), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.39453,  1.48438,  1.48438), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.53491,  0.53516,  2.00000), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3(-0.53491, -0.53491,  2.00000), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.53516,  0.53516,  2.00000), VEC3(0,1,0) ));
	mesh.vVx.push_back(VertexType(VEC3( 0.53516, -0.53491,  2.00000), VEC3(0,1,0) ));

	UInt32 vxCnt = (UInt32)mesh.vVx.size();
	for (UInt32 i = 0; i < vxCnt; ++i)
	{
		// translate to initial position
		mesh.vVx[i].P.x += vPos.x;
		mesh.vVx[i].P.y += vPos.y;
		mesh.vVx[i].P.z += vPos.z;
	}

	mesh.vFace.push_back( FaceType(	  0,  1,  2	) );
	mesh.vFace.push_back( FaceType(	  2,  3,  0	) );
	mesh.vFace.push_back( FaceType(	  1,  4,  5	) );
	mesh.vFace.push_back( FaceType(	  5,  2,  1	) );
	mesh.vFace.push_back( FaceType(	  4,  6,  7	) );
	mesh.vFace.push_back( FaceType(	  7,  5,  4	) );
	mesh.vFace.push_back( FaceType(	  3,  2,  8	) );
	mesh.vFace.push_back( FaceType(	  8,  9,  3	) );
	mesh.vFace.push_back( FaceType(	  2,  5, 10	) );
	mesh.vFace.push_back( FaceType(	 10,  8,  2	) );
	mesh.vFace.push_back( FaceType(	  5,  7, 11	) );
	mesh.vFace.push_back( FaceType(	 11, 10,  5	) );
	mesh.vFace.push_back( FaceType(	  9,  8, 12	) );
	mesh.vFace.push_back( FaceType(	 12, 13,  9	) );
	mesh.vFace.push_back( FaceType(	  8, 10, 14	) );
	mesh.vFace.push_back( FaceType(	 14, 12,  8	) );
	mesh.vFace.push_back( FaceType(	 10, 11, 15	) );
	mesh.vFace.push_back( FaceType(	 15, 14, 10	) );
	mesh.vFace.push_back( FaceType(	  6, 16, 17	) );
	mesh.vFace.push_back( FaceType(	 17,  7,  6	) );
	mesh.vFace.push_back( FaceType(	 16, 18, 19	) );
	mesh.vFace.push_back( FaceType(	 19, 17, 16	) );
	mesh.vFace.push_back( FaceType(	 18, 20, 21	) );
	mesh.vFace.push_back( FaceType(	 21, 19, 18	) );
	mesh.vFace.push_back( FaceType(	  7, 17, 22	) );
	mesh.vFace.push_back( FaceType(	 22, 11,  7	) );
	mesh.vFace.push_back( FaceType(	 17, 19, 23	) );
	mesh.vFace.push_back( FaceType(	 23, 22, 17	) );
	mesh.vFace.push_back( FaceType(	 19, 21, 24	) );
	mesh.vFace.push_back( FaceType(	 24, 23, 19	) );
	mesh.vFace.push_back( FaceType(	 11, 22, 25	) );
	mesh.vFace.push_back( FaceType(	 25, 15, 11	) );
	mesh.vFace.push_back( FaceType(	 22, 23, 26	) );
	mesh.vFace.push_back( FaceType(	 26, 25, 22	) );
	mesh.vFace.push_back( FaceType(	 23, 24, 27	) );
	mesh.vFace.push_back( FaceType(	 27, 26, 23	) );
	mesh.vFace.push_back( FaceType(	  0, 28, 29	) );
	mesh.vFace.push_back( FaceType(	 29,  1,  0	) );
	mesh.vFace.push_back( FaceType(	 28, 30, 31	) );
	mesh.vFace.push_back( FaceType(	 31, 29, 28	) );
	mesh.vFace.push_back( FaceType(	 30, 32, 33	) );
	mesh.vFace.push_back( FaceType(	 33, 31, 30	) );
	mesh.vFace.push_back( FaceType(	  1, 29, 34	) );
	mesh.vFace.push_back( FaceType(	 34,  4,  1	) );
	mesh.vFace.push_back( FaceType(	 29, 31, 35	) );
	mesh.vFace.push_back( FaceType(	 35, 34, 29	) );
	mesh.vFace.push_back( FaceType(	 31, 33, 36	) );
	mesh.vFace.push_back( FaceType(	 36, 35, 31	) );
	mesh.vFace.push_back( FaceType(	  4, 34, 16	) );
	mesh.vFace.push_back( FaceType(	 16,  6,  4	) );
	mesh.vFace.push_back( FaceType(	 34, 35, 18	) );
	mesh.vFace.push_back( FaceType(	 18, 16, 34	) );
	mesh.vFace.push_back( FaceType(	 35, 36, 20	) );
	mesh.vFace.push_back( FaceType(	 20, 18, 35	) );
	mesh.vFace.push_back( FaceType(	 32, 37, 38	) );
	mesh.vFace.push_back( FaceType(	 38, 33, 32	) );
	mesh.vFace.push_back( FaceType(	 37, 39, 40	) );
	mesh.vFace.push_back( FaceType(	 40, 38, 37	) );
	mesh.vFace.push_back( FaceType(	 39, 41, 42	) );
	mesh.vFace.push_back( FaceType(	 42, 40, 39	) );
	mesh.vFace.push_back( FaceType(	 33, 38, 43	) );
	mesh.vFace.push_back( FaceType(	 43, 36, 33	) );
	mesh.vFace.push_back( FaceType(	 38, 40, 44	) );
	mesh.vFace.push_back( FaceType(	 44, 43, 38	) );
	mesh.vFace.push_back( FaceType(	 40, 42, 45	) );
	mesh.vFace.push_back( FaceType(	 45, 44, 40	) );
	mesh.vFace.push_back( FaceType(	 36, 43, 21	) );
	mesh.vFace.push_back( FaceType(	 21, 20, 36	) );
	mesh.vFace.push_back( FaceType(	 43, 44, 24	) );
	mesh.vFace.push_back( FaceType(	 24, 21, 43	) );
	mesh.vFace.push_back( FaceType(	 44, 45, 27	) );
	mesh.vFace.push_back( FaceType(	 27, 24, 44	) );
	mesh.vFace.push_back( FaceType(	  0,  3, 46	) );
	mesh.vFace.push_back( FaceType(	 46, 28,  0	) );
	mesh.vFace.push_back( FaceType(	  3,  9, 47	) );
	mesh.vFace.push_back( FaceType(	 47, 46,  3	) );
	mesh.vFace.push_back( FaceType(	  9, 13, 48	) );
	mesh.vFace.push_back( FaceType(	 48, 47,  9	) );
	mesh.vFace.push_back( FaceType(	 28, 46, 49	) );
	mesh.vFace.push_back( FaceType(	 49, 30, 28	) );
	mesh.vFace.push_back( FaceType(	 46, 47, 50	) );
	mesh.vFace.push_back( FaceType(	 50, 49, 46	) );
	mesh.vFace.push_back( FaceType(	 47, 48, 51	) );
	mesh.vFace.push_back( FaceType(	 51, 50, 47	) );
	mesh.vFace.push_back( FaceType(	 30, 49, 37	) );
	mesh.vFace.push_back( FaceType(	 37, 32, 30	) );
	mesh.vFace.push_back( FaceType(	 49, 50, 39	) );
	mesh.vFace.push_back( FaceType(	 39, 37, 49	) );
	mesh.vFace.push_back( FaceType(	 50, 51, 41	) );
	mesh.vFace.push_back( FaceType(	 41, 39, 50	) );
	mesh.vFace.push_back( FaceType(	 13, 12, 52	) );
	mesh.vFace.push_back( FaceType(	 52, 48, 13	) );
	mesh.vFace.push_back( FaceType(	 12, 14, 53	) );
	mesh.vFace.push_back( FaceType(	 53, 52, 12	) );
	mesh.vFace.push_back( FaceType(	 14, 15, 25	) );
	mesh.vFace.push_back( FaceType(	 25, 53, 14	) );
	mesh.vFace.push_back( FaceType(	 48, 52, 54	) );
	mesh.vFace.push_back( FaceType(	 54, 51, 48	) );
	mesh.vFace.push_back( FaceType(	 52, 53, 55	) );
	mesh.vFace.push_back( FaceType(	 55, 54, 52	) );
	mesh.vFace.push_back( FaceType(	 53, 25, 26	) );
	mesh.vFace.push_back( FaceType(	 26, 55, 53	) );
	mesh.vFace.push_back( FaceType(	 51, 54, 42	) );
	mesh.vFace.push_back( FaceType(	 42, 41, 51	) );
	mesh.vFace.push_back( FaceType(	 54, 55, 45	) );
	mesh.vFace.push_back( FaceType(	 45, 42, 54	) );
	mesh.vFace.push_back( FaceType(	 55, 26, 27	) );
	mesh.vFace.push_back( FaceType(	 27, 45, 55	) );

	// add it to the model

	mv_Mesh.push_back( mesh );

	// no normals in base data, so update them here
	CalculateNormals();
}

XString Model::GetTextureFileName(const char* fileName)
{
	// Return full path/file name.
	//
	// If fileName contains a path, return as is,
	// else append path to current model file.

	XString strTextureFile = fileName;
	strTextureFile.Trim();

	// if it does not have a path, prepend current path
	if (strTextureFile.FindOneOf("\\/") == -1)
		strTextureFile = XString::CombinePath(ms_FileName.GetPathName().c_str(), fileName);

	return strTextureFile;
}

bool Model::Load(const char* /* fileName */)
{
	// Override in subclass to provide file type
	// specific io.

	Logger::Write(__FUNCTION__, "Not implemented.");
	return false;
}

bool Model::Save(const char* /* fileName */)
{
	// Override in subclass to provide file type
	// specific io.

	Logger::Write(__FUNCTION__, "Not implemented.");
	return false;
}
