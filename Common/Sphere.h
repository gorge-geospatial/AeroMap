#ifndef SPHERE_H
#define SPHERE_H

#include "Primitive.h"			// base class

class Sphere : public Primitive
{
public:

	Sphere();
	virtual ~Sphere();

	void Create(VertexBufferGL::VertexType vertexType, double radius, int tessLevel = 1, bool windCW = true);
	double GetRadius() { return mf_Radius; }					// radius of sphere

	// Primitive
	virtual void ApplyChanges();
	virtual void Render();
	virtual double GetBoundingSphere() { return mf_Radius; }	// radius of minimum bounding sphere

private:

	double mf_Radius;		// radius of generated sphere

private:

	void CreateBase();
	void CreateVertexBuffer(VertexBufferGL::VertexType vertexType);
};

#endif // #ifndef SPHERE_H
