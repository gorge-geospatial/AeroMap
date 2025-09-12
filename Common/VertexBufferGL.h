#ifndef VERTEXBUFFERGL_H
#define VERTEXBUFFERGL_H

#include <GL/glew.h>			// OpenGL

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include <vector>

#include "GLManager.h"

class VertexBufferGL
{
public:

	enum class VertexType
	{
		NONE,
		P,			// position only
		PNT,		// position, normal and texture components
		PNTF,		// position, normal, texture, flags
		PNC,		// vertex with position, normal and color components
		PC,			// vertex with position and color components
		PCT,		// vertex with position, color and texture coordinates
		SSC,		// 2D (screen) vertex with color
		SSCT,		// 2D (screen) vertex with color + texture

		Water		// for realistic water rendering
	};

	struct VertexWater
	{
		float x, y;		// 2d position

		VertexWater()
		{
			x = y = 0.0F;
		}
	};

	struct VertexPC
	{
		float x, y, z;		// position
		float r, g, b;		// color

		VertexPC()
		{
			x = y = z = 0.0F;
			r = g = b = 0.0F;
		}
		void SetPos(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		void SetPos(VEC3& pos)
		{
			this->x = static_cast<float>(pos.x);
			this->y = static_cast<float>(pos.y);
			this->z = static_cast<float>(pos.z);
		}
		void SetColor(float r, float g, float b)
		{
			this->r = r;
			this->g = g;
			this->b = b;
		}
		void SetColor(float gray)
		{
			this->r = gray;
			this->g = gray;
			this->b = gray;
		}
		void SetColor(const VEC3& c)
		{
			this->r = (float)c.x;
			this->g = (float)c.y;
			this->b = (float)c.z;
		}
		void SetColor(const PixelType& pix)
		{
			this->r = pix.GetR();
			this->g = pix.GetG();
			this->b = pix.GetB();
		}
	};

	struct VertexPCT
	{
		float x, y, z;		// position
		float r, g, b;		// color
		float u, v;			// texture

		VertexPCT()
		{
			x = y = z = 0.0F;
			r = g = b = 0.0F;
			u = v = 0.0F;
		}
		void SetPos(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		void SetPos(VEC3& pos)
		{
			this->x = static_cast<float>(pos.x);
			this->y = static_cast<float>(pos.y);
			this->z = static_cast<float>(pos.z);
		}
		void SetColor(float r, float g, float b)
		{
			this->r = r;
			this->g = g;
			this->b = b;
		}
		void SetColor(float gray)
		{
			this->r = gray;
			this->g = gray;
			this->b = gray;
		}
		void SetColor(const VEC3& c)
		{
			this->r = (float)c.x;
			this->g = (float)c.y;
			this->b = (float)c.z;
		}
		void SetColor(const PixelType& pix)
		{
			this->r = pix.GetR();
			this->g = pix.GetG();
			this->b = pix.GetB();
		}
		void SetTexture(float tu, float tv)
		{
			this->u = tu;
			this->v = tv;
		}
	};

	struct VertexPNT
	{
		float x, y, z;		// position
		float nx, ny, nz;	// normal
		float tu, tv;		// texture coordinates

		VertexPNT()
		{
			x = y = z = 0.0F;
			nx = ny = nz = 0.0F;
			tu = tv = 0.0F;
		}
		void SetPos(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		void SetPos(VEC3& pos)
		{
			this->x = (float)pos.x;
			this->y = (float)pos.y;
			this->z = (float)pos.z;
		}
		void SetNormal(float nx, float ny, float nz)
		{
			this->nx = nx;
			this->ny = ny;
			this->nz = nz;
		}
		void SetNormal(VEC3& normal)
		{
			this->nx = (float)normal.x;
			this->ny = (float)normal.y;
			this->nz = (float)normal.z;
		}
		void SetTexture(float tu, float tv)
		{
			this->tu = tu;
			this->tv = tv;
		}
	};

	struct VertexPNTF
	{
		float x, y, z;		// position
		float nx, ny, nz;	// normal
		float tu, tv;		// texture coordinates
		UInt32 flags;		// glsl only has 32-bit ints

		VertexPNTF()
		{
			x = y = z = 0.0F;
			nx = ny = nz = 0.0F;
			tu = tv = 0.0F;
		}
		void SetPos(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		void SetPos(VEC3& pos)
		{
			this->x = (float)pos.x;
			this->y = (float)pos.y;
			this->z = (float)pos.z;
		}
		void SetNormal(float nx, float ny, float nz)
		{
			this->nx = nx;
			this->ny = ny;
			this->nz = nz;
		}
		void SetNormal(VEC3& normal)
		{
			this->nx = (float)normal.x;
			this->ny = (float)normal.y;
			this->nz = (float)normal.z;
		}
		void SetTexture(float tu, float tv)
		{
			this->tu = tu;
			this->tv = tv;
		}
		void SetFlags(UInt32 flags)
		{
			this->flags = flags;
		}
		UInt32 GetFlags()
		{
			return this->flags;
		}
	};

	struct VertexPNC
	{
		float x, y, z;		// position
		float nx, ny, nz;	// normal
		float r, g, b;		// color

		VertexPNC()
		{
			x = y = z = 0.0F;
			nx = ny = nz = 0.0F;
			r = g = b = 0.0F;
		}
		void SetPos(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		void SetPos(VEC3& pos)
		{
			this->x = (float)pos.x;
			this->y = (float)pos.y;
			this->z = (float)pos.z;
		}
		void SetNormal(float nx, float ny, float nz)
		{
			this->nx = nx;
			this->ny = ny;
			this->nz = nz;
		}
		void SetNormal(VEC3& normal)
		{
			this->nx = (float)normal.x;
			this->ny = (float)normal.y;
			this->nz = (float)normal.z;
		}
		void SetColor(float gray)
		{
			this->r = gray;
			this->g = gray;
			this->b = gray;
		}
		void SetColor(VEC3& c)
		{
			this->r = (float)c.x;
			this->g = (float)c.y;
			this->b = (float)c.z;
		}
		void SetColor(const PixelType& pix)
		{
			this->r = pix.GetR();
			this->g = pix.GetG();
			this->b = pix.GetB();
		}
	};

	// screen space vertex with color
	struct VertexSSC
	{
		float x, y;			// position
		float r, g, b;		// color

		VertexSSC()
		{
			x = y = 0.0F;
			r = g = b = 0.0F;
		}
		void SetPos(float x, float y)
		{
			this->x = x;
			this->y = y;
		}
		void SetColor(float r, float g, float b)
		{
			this->r = r;
			this->g = g;
			this->b = b;
		}
		void SetColor(float gray)
		{
			this->r = gray;
			this->g = gray;
			this->b = gray;
		}
		void SetColor(const VEC3& c)
		{
			this->r = (float)c.x;
			this->g = (float)c.y;
			this->b = (float)c.z;
		}
		void SetColor(const PixelType& pix)
		{
			this->r = pix.GetR();
			this->g = pix.GetG();
			this->b = pix.GetB();
		}
	};

	// screen space with color + texture
	struct VertexSSCT
	{
		float x, y;			// position
		float r, g, b;		// color
		float tu, tv;		// texture coordinates

		void SetPos(float x, float y)
		{
			this->x = x;
			this->y = y;
		}
		void SetColor(float r, float g, float b)
		{
			this->r = r;
			this->g = g;
			this->b = b;
		}
		void SetTexCoord(float tu, float tv)
		{
			this->tu = tu;
			this->tv = tv;
		}
		void SetColor(VEC3& c)
		{
			this->r = (float)c.x;
			this->g = (float)c.y;
			this->b = (float)c.z;
		}
		void SetColor(const PixelType& pix)
		{
			this->r = pix.GetR();
			this->g = pix.GetG();
			this->b = pix.GetB();
		}
	};

public:

	VertexBufferGL();
	~VertexBufferGL();

	VertexBufferGL(const VertexBufferGL& src) = delete;		// no copy constructor

	// for water type vertices

	bool CreateWater(
		std::vector<VertexWater> vx,					// vertex data
		std::vector<unsigned int> indexList = std::vector<unsigned int>());		// vertex indices, optional

	// for P vertices

	bool CreateP(
		unsigned int primitiveType,			// GL_TRIANGLES, GL_LINES, etc.
		float* vx,							// vertex data
		int count);							// number of vertices

	// for PC vertices

	bool CreatePC(
		unsigned int primitiveType,					// GL_TRIANGLES, GL_LINES, etc.
		std::vector<VertexPC> vx,					// vertex data
		std::vector<unsigned int> indexList = std::vector<unsigned int>());		// vertex indices, optional

	void ModifyPC(float* vx, float* color);			// update vertex/colors in PC buffer

	bool CreatePCT(
		unsigned int primitiveType,					// GL_TRIANGLES, GL_LINES, etc.
		std::vector<VertexPCT> vx,					// vertex data
		std::vector<unsigned int> indexList = std::vector<unsigned int>());		// vertex indices, optional

	// for PNT vertices

	bool CreatePNT(
		unsigned int primitiveType,					// GL_TRIANGLES, GL_LINES, etc.
		std::vector<VertexPNT> vxList,				// vertex data
		std::vector<unsigned int> indexList = std::vector<unsigned int>());		// vertex indices, optional

	// for PNTF vertices

	bool CreatePNTF(
		unsigned int primitiveType,					// GL_TRIANGLES, GL_LINES, etc.
		std::vector<VertexPNTF> vxList,				// vertex data
		std::vector<unsigned int> indexList = std::vector<unsigned int>());		// vertex indices, optional

	// for PNC vertices with index

	bool CreatePNC(
		unsigned int primitiveType,					// GL_TRIANGLES, GL_LINES, etc.
		std::vector<VertexPNC> vx,					// vertex data
		std::vector<unsigned int> indexList = std::vector<unsigned int>());		// vertex indices, optional

	// modify position & normal in any PNx type buffer
	void ModifyPN(float* vx, float* n);				// update vertex/normals
	void ModifyPN(float* vx, float* n, int offset);	// update subset of vertex/normals

	// for screen space vertices w/o index

	bool CreateSSC(
		unsigned int primitiveType,					// GL_TRIANGLES, GL_LINES, etc.
		std::vector<VertexSSC> vx);				 	// vertex data

	// for 2d/color/texture vertices w/o index

	bool CreateSSCT(
		unsigned int primitiveType,					// GL_TRIANGLES, GL_LINES, etc.
		std::vector<VertexSSCT> vx);		 		// vertex data

	void Delete();
	void Render();

	bool IsValid() { return m_vaoHandle > 0; }

private:

	GLuint m_handle[10];				// buffer handles
	int m_handleCount;					// # of buffers in m_handle[]
	int m_ibIndex;						// index into m_handle[] for index buffer, -1 if no index buffer
	unsigned int m_PrimitiveType;		// GL_TRIANGLES, GL_LINES, etc.
	VertexType m_VertexType;
	unsigned int m_vaoHandle;
	unsigned int m_VertexCount;
	unsigned int m_IndexCount;
};

#endif // VERTEXBUFFERGL_H
