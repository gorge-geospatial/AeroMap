#ifndef CUBE_H
#define CUBE_H

#include "Primitive.h"			// base class

class Cube : public Primitive
{
public:

	Cube();
	virtual ~Cube();

	void Create(VertexBufferGL::VertexType vertexType, VEC3 size, int tessLevel = 1, bool windCW = true);
	VEC3 GetSize() { return m_Size; }

	// Primitive
	virtual void ApplyChanges();
	virtual void Render();
	virtual double GetBoundingSphere() { return mf_BoundRadius; }	// radius of minimum bounding sphere

private:

	VEC3 m_Size;			// size of generated block

private:

	void CreateBase();
	void CreateVertexBuffer(VertexBufferGL::VertexType vertexType);
};

#endif // #ifndef CUBE_H
