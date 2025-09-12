#ifndef CYLINDER_H
#define CYLINDER_H

#include "Primitive.h"			// base class

class Cylinder : public Primitive
{
public:

	Cylinder();
	virtual ~Cylinder();

	void Create(VertexBufferGL::VertexType vertexType, double radius, double length, int tessLevel = 1, bool windCW = true);
	double GetRadius() { return mf_Radius; }

	// Primitive
	virtual void ApplyChanges();
	virtual void Render();
	virtual double GetBoundingSphere() { return mf_BoundRadius; }	// return radius of minimum bounding sphere

private:

	double mf_Radius;		// radius of cylinder

private:

	void CreateBase(double radius, double length);
	void CreateVertexBuffer(VertexBufferGL::VertexType vertexType);
};

#endif // #ifndef CYLINDER_H
