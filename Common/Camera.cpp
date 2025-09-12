// Camera.cpp
// Camera manager
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <assert.h>

#include "Camera.h"

Camera::Camera()
	: mv_Pos(0, 0, 0)
	, mv_Dir(0, 1, 0)		// facing North
	, mv_Up(0, 0, 1)		// upright
	, m_ViewPort(0, 0)
	, mb_IsBounded(false)
{
}

Camera::~Camera()
{
}

void Camera::SetDirUp(VEC3 dir, VEC3 up)
{ 
	// since they're so tightly correlated, set both at same time

	assert(dir.Magnitude() > 0.0 && up.Magnitude() > 0.0);

	mv_Dir = Normalize(dir);
	mv_Up = Normalize(up);
}

void Camera::SetFrustum(double fov, double aspect, double nearDist, double farDist)
{
	// Set parameters that define view frustum associated 
	// with this camera.

	// need these to define frustum (lighthouse3d.com):
	//		p – camera position (mv_Pos)
	//		d – normalized vector with the direction of the camera’s view ray (mv_Dir)
	//		nearDist – distance from the camera to the near plane (in glm::perspective() but not sure how to extract)
	//		nearH – “height” of the near plane
	//		nearW – “width” of the near plane
	//		farDist – distance from the camera to the far plane (in glm::perspective() but not sure how to extract)
	//		farH – “height” of the far plane
	//		farW – “width” of the far plane

	VEC3 X, Y, Z;

	// calc sizes of near & far planes
	double nearH = 2.0 * tan(fov * 0.5) * nearDist;
	double nearW = nearH * aspect;

	//double farH = 2.0 * tan(fov * 0.5) * farDist;
	//double farW = farH * aspect;

	// calc center of near & far plane
	VEC3 fc = mv_Pos + mv_Dir * farDist;
	VEC3 nc = mv_Pos + mv_Dir * nearDist;

	// compute the Z axis of camera
	// this axis points in the opposite direction from
	// the looking direction
	Z = -mv_Dir;	// p - l;
	//Normalize(Z);

	// X axis of camera with given "up" vector and Z axis
	X = CrossProduct(mv_Up, Z).Unit();		// X = u * Z;

	// the real "up" vector is the cross product of Z and X
	Y = CrossProduct(Z, X).Unit();

	// these calcs are less efficient, but ultimately give
	// the same results as the normal/point plane definition
	// i'm using

	//VEC3 right = CrossProduct(mv_Dir, mv_Up);

	// calc 4 corners of far plane:
	//VEC3 ftl = fc + (mv_Up * farH / 2) - (right * farW / 2);
	//VEC3 ftr = fc + (mv_Up * farH / 2) + (right * farW / 2);
	//VEC3 fbl = fc - (mv_Up * farH / 2) - (right * farW / 2);
	//VEC3 fbr = fc - (mv_Up * farH / 2) + (right * farW / 2);

	// similarly for near plane
	//VEC3 ntl = nc + (mv_Up * nearH / 2) - (right * nearW / 2);
	//VEC3 ntr = nc + (mv_Up * nearH / 2) + (right * nearW / 2);
	//VEC3 nbl = nc - (mv_Up * nearH / 2) - (right * nearW / 2);
	//VEC3 nbr = nc - (mv_Up * nearH / 2) + (right * nearW / 2);

	// compute the centers of the near and far planes
	//nc = mv_Pos - Z * nearDist;
	//fc = mv_Pos - Z * farDist;

	// compute the six planes
	//
	// set the normals such that they all point to
	// the inside of the frustum

	m_planeNear.SetNormalAndPoint(mv_Dir, nc);
	m_planeFar.SetNormalAndPoint(-mv_Dir, fc);

	VEC3 aux, normal;

	double nh = nearH * 0.5;	// 1/2 height of near plane
	double nw = nearW * 0.5;	// 1/2 width of near plane

	aux = (nc + Y*nh) - mv_Pos;
	Normalize(aux);
	normal = CrossProduct(aux, X).Unit();
	m_planeTop.SetNormalAndPoint(normal, nc + Y*nh);

	aux = (nc - Y*nh) - mv_Pos;
	Normalize(aux);
	normal = CrossProduct(X, aux).Unit();
	m_planeBottom.SetNormalAndPoint(normal, nc - Y*nh);

	aux = (nc - X*nw) - mv_Pos;
	Normalize(aux);
	normal = CrossProduct(aux, Y).Unit();
	m_planeLeft.SetNormalAndPoint(normal, nc - X*nw);

	aux = (nc + X*nw) - mv_Pos;
	Normalize(aux);
	normal = CrossProduct(Y, aux).Unit();
	m_planeRight.SetNormalAndPoint(normal, nc + X*nw);

	// update view frustum parameters
	m_Frustum.fov = fov;
	m_Frustum.nearDist = nearDist;
	m_Frustum.farDist = farDist;
	m_Frustum.aspect = aspect;
	m_Frustum.isValid = true;
}

bool Camera::PointInFrustum(VEC3& p)
{
	// Test point to see if inside/outside view frustum.

	// Frustum undefined
	if (m_Frustum.isValid == false)
		return false;

	// Update view frustum to match camera's position & orientation.
	SetFrustum(m_Frustum.fov, m_Frustum.aspect, m_Frustum.nearDist, m_Frustum.farDist);

	if (DistancePointToPlane(p, m_planeNear.P, m_planeNear.N) < 0.0)
		return false;
	if (DistancePointToPlane(p, m_planeFar.P, m_planeFar.N) < 0.0)
		return false;

	if (DistancePointToPlane(p, m_planeLeft.P, m_planeLeft.N) < 0.0)
		return false;
	if (DistancePointToPlane(p, m_planeRight.P, m_planeRight.N) < 0.0)
		return false;

	if (DistancePointToPlane(p, m_planeTop.P, m_planeTop.N) < 0.0)
		return false;
	if (DistancePointToPlane(p, m_planeBottom.P, m_planeBottom.N) < 0.0)
		return false;

	return true;
}

void Camera::Translate(double dx, double dy, double dz)
{
	mv_Pos.x += dx;
	mv_Pos.y += dy;
	mv_Pos.z += dz;
}

void Camera::Rotate(double radians)
{
	// Rotate camera about DIR axis.
	//

	MAT3 mat;
	MatrixRotationAxis(&mat, &mv_Dir, radians);
	mv_Up = Vec3Transform(mv_Up, mat);
	mv_Up = Normalize(mv_Up);
}

void Camera::RotateAboutX(double radians)
{
	// Rotate camera about X axis.
	//

	RotateX(radians, mv_Pos);
	RotateX(radians, mv_Dir);
	RotateX(radians, mv_Up);
}

void Camera::RotateAboutY(double radians)
{
	// Rotate camera about Y axis.
	//

	RotateY(radians, mv_Pos);
	RotateY(radians, mv_Dir);
	RotateY(radians, mv_Up);
}

void Camera::RotateAboutZ(double radians)
{
	// Rotate camera about Z axis.
	//

	RotateZ(radians, mv_Pos);
	RotateZ(radians, mv_Dir);
	RotateZ(radians, mv_Up);
}

void Camera::Pan(double radians)
{
	// rotate around the UP vector
	MAT3 mat;
	MatrixRotationAxis(&mat, &mv_Up, radians);
	mv_Dir = Vec3Transform(mv_Dir, mat);
	mv_Dir = Normalize(mv_Dir);
}

void Camera::Tilt(double radians)
{
	// Rotate around the DIRxUP vector.
	//

	VEC3 dxu = CrossProduct(mv_Dir, mv_Up);
	MAT3 mat;
	MatrixRotationAxis(&mat, &dxu, radians);
	mv_Dir = Vec3Transform(mv_Dir, mat);
	mv_Up = Vec3Transform(mv_Up, mat);
	mv_Dir = Normalize(mv_Dir);
	mv_Up = Normalize(mv_Up);
}

double Camera::GetTilt()
{
	// get angle with horizontal plane

	// if dir is horizontal
	if ((mv_Dir.z > -0.01) && (mv_Dir.z < 0.01))
		return 0.0;

	VEC3 v = mv_Dir;
	v.z = 0.0;
	v = Normalize(v);

	double a = CalcAngleRad(mv_Dir, v);
	return a;
}

void Camera::MoveBackward(double d)
{
	// move backward along camera's DIR vector
	VEC3 pos = mv_Pos - (mv_Dir * d);
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::MoveForward(double d)
{
	// move forward along camera's DIR vector
	VEC3 pos = mv_Pos + (mv_Dir * d);
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::MoveLeft(double d)
{
	// move along camera's DIR X UP direction
	VEC3 moveDir = CrossProduct(mv_Dir, mv_Up);
	// assumes OpenGL coordinate frame
	VEC3 pos = mv_Pos - (moveDir * d);
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::MoveRight(double d)
{
	// move along camera's DIR X UP direction
	VEC3 moveDir = CrossProduct(mv_Dir, mv_Up);
	// assumes OpenGL coordinate frame
	VEC3 pos = mv_Pos + (moveDir * d);
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::MoveUp(double d)
{
	// move up along camera's UP vector
	VEC3 pos = mv_Pos + (mv_Up * d);
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::MoveDown(double d)
{
	// move down along camera's UP vector
	VEC3 pos = mv_Pos - (mv_Up * d);
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::MoveX(double d)
{
	// move along X axis
	VEC3 pos = mv_Pos;
	pos.x += d;
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::MoveY(double d)
{
	// move along X axis
	VEC3 pos = mv_Pos;
	pos.y += d;
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::MoveZ(double d)
{
	// move along X axis
	VEC3 pos = mv_Pos;
	pos.z += d;
	if (IsValid(pos))
		mv_Pos = pos;
}

void Camera::SetBounds(const VEC3& min, const VEC3& max)
{
	mv_BoundMin = min;
	mv_BoundMax = max;

	mb_IsBounded = true;
}

void Camera::ClearBounds()
{
	mb_IsBounded = false;
}

bool Camera::IsValid(const VEC3& pos)
{
	// check pos against bounds for validity
	if (!mb_IsBounded)
		return true;

	if (pos.x < mv_BoundMin.x)
		return false;
	if (pos.y < mv_BoundMin.y)
		return false;
	if (pos.z < mv_BoundMin.z)
		return false;

	if (pos.x > mv_BoundMax.x)
		return false;
	if (pos.y > mv_BoundMax.y)
		return false;
	if (pos.z > mv_BoundMax.z)
		return false;

	return true;
}
