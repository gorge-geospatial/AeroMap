#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>			// FILE, etc
#include <stdlib.h>			// atoi(), etc
#include <ctype.h>			// tolower(), etc

#include <vector>

#include "Calc.h"				// math library
#include "Logger.h"				// static message logger
#include "XString.h"
#include "Material.h"
#include "TextureGL.h"			// texture class
#include "VertexBufferGL.h"		// vertex buffer class

#define MAX_MESH_NAME 32

class Model
{
public:

	enum class Format
	{
		AC3D,			// AC3D File
		OBJ,			// WaveFront OBJ file
		STL,			// STL file
		ThreeDS,		// 3D Studio File
		X				// DirectX X file
	};

	enum NORMAL
	{
		NORMAL_NONE,
		NORMAL_VERTEX,
		NORMAL_FACE
	};

	struct TexCoord
	{
		float u;			// texture coordinates
		float v;
		float w;			// optional

		TexCoord()
			: u(0.0)
			, v(0.0)
			, w(0.0)
		{
		}
		TexCoord(float tu, float tv)
			: u(tu)
			, v(tv)
			, w(0.0)
		{
		}
		void Clear()
		{
			u = 0.0F;
			v = 0.0F;
			w = 0.0F;
		}
	};

	struct FaceType
	{
		int vx0;			// triangle vertices
		int vx1;
		int vx2;

		int tx0;			// index into mesh texture coord array
		int tx1;
		int tx2;

		VEC3 N;				// face normal
		bool IsSelected;	// selected flag

							// tesselation support
		int vx01;			// vertex that bisects side <vx0,vx1>
		int vx12;
		int vx20;

		FaceType()			// default constructor
			: vx0(0)
			, vx1(0)
			, vx2(0)
			, tx0(0)
			, tx1(0)
			, tx2(0)
			, N(0, 0, 0)
			, vx01(0xFFFFFFFF)		// not bisected
			, vx12(0xFFFFFFFF)
			, vx20(0xFFFFFFFF)
			, IsSelected(false)
		{
		}

		FaceType(int vertex0, int vertex1, int vertex2)
			: FaceType()
		{
			vx0 = vertex0;
			vx1 = vertex1;
			vx2 = vertex2;
		}
	};

	struct VertexType
	{
		VEC3 P;				// 3d position
		VEC3 N;				// vertex normal
		bool IsSelected;	// selected flag

		VertexType()
		{
			P = VEC3(0, 0, 0);
			N = VEC3(0, 0, 0);
			IsSelected = false;
		}

		VertexType(VEC3 Pos, VEC3 Normal)
			: VertexType()
		{
			this->P = Pos;
			this->N = Normal;
		}
	};

	struct MeshType
	{
		char Name[MAX_MESH_NAME];
		std::vector<FaceType>	vFace;
		std::vector<VertexType>	vVx;
		std::vector<TexCoord>   vTex;		// texture coordinates for this mesh, may not correspond to vVx[] indices
		VEC3 vMinExt;
		VEC3 vMaxExt;
		bool IsVisible;

		MeshType()
		{
			Clear();
		}

		void Clear()
		{
			memset(Name, 0, MAX_MESH_NAME);
			vFace.clear();
			vVx.clear();
			vTex.clear();

			vFace.reserve(25);
			vVx.reserve(50);

			vMinExt = VEC3(0, 0, 0);
			vMaxExt = VEC3(0, 0, 0);

			IsVisible = true;
		}

		void SetName(const char* pName)
		{
			if (pName == nullptr)
				return;

			strncpy(Name, pName, MAX_MESH_NAME - 1);
		}
		void UpdateExtents()
		{
			vMinExt = VEC3(0, 0, 0);
			vMaxExt = VEC3(0, 0, 0);

			if (vVx.size() < 1)
				return;

			vMinExt = vVx[0].P;
			vMaxExt = vVx[0].P;

			for (unsigned int i = 0; i < vVx.size(); ++i)
			{
				if (vMinExt.x > vVx[i].P.x) vMinExt.x = vVx[i].P.x;
				if (vMinExt.y > vVx[i].P.y) vMinExt.y = vVx[i].P.y;
				if (vMinExt.z > vVx[i].P.z) vMinExt.z = vVx[i].P.z;

				if (vMaxExt.x < vVx[i].P.x) vMaxExt.x = vVx[i].P.x;
				if (vMaxExt.y < vVx[i].P.y) vMaxExt.y = vVx[i].P.y;
				if (vMaxExt.z < vVx[i].P.z) vMaxExt.z = vVx[i].P.z;
			}
		}
	};

public:

	// construction / destruction

	Model();
	virtual ~Model();

	virtual Format GetFormat() = 0;		// override to specify the source file format

	// override to handle type-specific io
	virtual bool Load(const char* fileName);
	virtual bool Save(const char* fileName = nullptr);

	// public methods

	void	AddBlock(VEC3 vPos);
	void	AddCylinder(VEC3 vPos);
	void	AddSphere(VEC3 vPos);
	void	Center();
	void	Clone(VEC3 vPos);
	void	ExchangeAxes(AXIS src, AXIS dest, bool flipSigns);
	void	GetExtents(VEC3& vMinExtents, VEC3& vMaxExtents);
	void	GetExtents(const char* meshName, VEC3& vMinExtents, VEC3& vMaxExtents);
	double	GetRadius();
	void	Mirror(AXIS axis);
	void	Render();
	void	Rotate(AXIS axis, VEC3 vPoint, float delta);
	void	Scale(double scaleFactor, bool selected = false);
	void	Scale(double dx, double dy, double dz, bool selected = false);
	void	ScaleTo(double sx, double sy, double sz);
	void	SetPosition(VEC3 vPos);

	// render switches

	bool	GetRenderAxes() { return mb_RenderAxes; }
	void 	SetRenderAxes(bool renderAxes) { mb_RenderAxes = renderAxes; }
	NORMAL	GetRenderNormals() { return m_RenderNormals; }
	void	SetRenderNormals(NORMAL normal) { m_RenderNormals = normal; }
	bool	GetRenderTextures() { return mb_RenderTextures; }
	void 	SetRenderTextures(bool renderTextures) { mb_RenderTextures = renderTextures; }

	// vertex operations

	int		AddVertex(int meshIndex);
	void	DeleteVertex();				// delete selected vertex / vertices
	int		GetSelectedVertexCount();
	int		GetVertexCount() const;
	int		GetVertexCount(int meshIndex) const;
	void	MergeVertices();
	void    MoveVertex(double dx, double dy, double dz, bool selected = true);
	int		SelectVertex(VEC3 vMin, VEC3 vMax, bool clear = true);
	void	VertexEqual(AXIS axis);
	VertexType* GetVertex(int meshIndex, int vertexIndex);

	// face operations

	int		AddFace(int meshIndex);
	int	 	DeleteFaces();
	bool	FacePick(VEC3 rayOrg, VEC3 rayDir, int& meshIdx, int& faceIdx);
	int     GetFaceCount() const;
	int     GetFaceCount(int meshIndex) const;
	void    ReverseWindingOrder(bool selected = true);
	void	SelectFace(int meshIdx, int faceIdx);
	int		Tesselate(bool selected = true);
	FaceType* GetFace(int meshIndex, int faceIndex);

	// mesh operations

	int		AddMesh();
	int		GetMeshCount();
	int		GetMeshIndex(const char* meshName);
	void	MergeMeshes();
	void	MergeMesh(int destIdx, int srcIdx);
	void	SetRenderMesh(int meshIndex, bool render);
	MeshType*	GetMesh(int meshIndex);
	const char* GetMeshName(int meshIndex);

	// texture coordinates

	int		AddTexCoord(int meshIndex);
	int		GetTexCoordCount(int meshIndex);
	TexCoord*	GetTexCoord(int meshIndex, int tcIndex);

	// materials

	int AddMaterial(Material& mat);
	int GetMaterialCount();
	Material* GetMaterial(int matIndex);

protected:

	std::vector<MeshType> mv_Mesh;

	std::vector<Material> mv_Mat;	// materials used in model

	VEC3	mv_Pos;
	double	mf_Radius;				// radius of bounding sphere
	TextureGL m_Texture;			// single model texture (need to support one per mesh?)
	XString ms_FileName;			// full path/filename of model file

	NORMAL	m_RenderNormals;		// normal rendering mode
	bool	mb_RenderOn;			// rendering enabled
	bool    mb_RenderAxes;			// render model space axes
	bool    mb_RenderTextures;

protected:

	void	Render(int meshIdx);
	void	RenderNormals();
	void	RenderSelectedFaces();
	void	RenderSelectedVx();
	void	RenderAxes();
	void	RenderOn();					// enable rendering
	void	RenderOff();				// disable rendering

	void	CalculateNormals();
	int		DeleteEqualVertices();
	int		DeleteUnusedVertices();
	void    DeleteVertex(int meshIdx, int vertexIdx);
	void	DeSelectAll();

	VertexType Interpolate(VertexType vx1, VertexType vx2);
	int	TessFindVertex(const std::vector<FaceType>& faceList, UInt16 vx1, UInt16 vx2);
	XString GetTextureFileName(const char* fileName);
};

#endif // MODEL_H
