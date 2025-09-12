//	Calc.cpp
//	Math library
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <algorithm>

#include <assert.h>

#include "Calc.h"

void MatrixMultiply(MAT3* pMat, const MAT3* pMat1, const MAT3* pMat2)
{
	// 3x3 matrix multiply
	//

	// make local copies of operand matrices - one or both may point to output matrix
	MAT3 m1;
	MAT3 m2;

	// maybe safer way to create local copies, since won't always be sure '=' operator
	// implemented as expected
	memcpy(&m1, pMat1, sizeof(m1));
	memcpy(&m2, pMat2, sizeof(m2));

	pMat->m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0];
	pMat->m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1];
	pMat->m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2];

	pMat->m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0];
	pMat->m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1];
	pMat->m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2];

	pMat->m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0];
	pMat->m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1];
	pMat->m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2];
}

void MatrixMultiply(MAT4* pMat, const MAT4* pMat1, const MAT4* pMat2)
{
	// explicit 4x4 matrix multiply
	//

	// make local copies of operand matrices - one or both may point to output matrix
	MAT4 m1;
	MAT4 m2;

	// maybe safer way to create local copies, since won't always be sure '=' operator
	// implemented as expected
	memcpy(&m1, pMat1, sizeof(m1));
	memcpy(&m2, pMat2, sizeof(m2));

	pMat->m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
	pMat->m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
	pMat->m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
	pMat->m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];

	pMat->m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
	pMat->m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
	pMat->m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
	pMat->m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];

	pMat->m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
	pMat->m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
	pMat->m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
	pMat->m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];

	pMat->m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
	pMat->m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
	pMat->m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
	pMat->m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];
}

void MatrixIdentity(MAT4* mat)
{
	// return identity matrix
	//

	mat->m[0][0] = 1.0; mat->m[0][1] = 0.0; mat->m[0][2] = 0.0; mat->m[0][3] = 0.0;
	mat->m[1][0] = 0.0; mat->m[1][1] = 1.0; mat->m[1][2] = 0.0; mat->m[1][3] = 0.0;
	mat->m[2][0] = 0.0; mat->m[2][1] = 0.0; mat->m[2][2] = 1.0; mat->m[2][3] = 0.0;
	mat->m[3][0] = 0.0; mat->m[3][1] = 0.0; mat->m[3][2] = 0.0; mat->m[3][3] = 1.0;
}

void MatrixRotationYawPitchRoll(MAT4* mat, double yaw, double pitch, double roll)
{
	// the "double" versions (cosf(), etc) don't compile for gumstix

	double sin_yaw = sin(yaw);
	double cos_yaw = cos(yaw);
	double sin_pitch = sin(pitch);
	double cos_pitch = cos(pitch);
	double sin_roll = sin(roll);
	double cos_roll = cos(roll);

	MatrixIdentity( mat );

	// this returns the microsoft YawPitchRoll matrix, it's similar to any that would be found in any textbook, but
	// the axes/angles are consistent with the dx coordinate system i've been using - this make porting much more
	// straightforward

	mat->m[0][0] =  cos_yaw*cos_roll + sin_yaw*sin_pitch*sin_roll;
	mat->m[0][1] =  cos_pitch*sin_roll;
	mat->m[0][2] =  cos_yaw*sin_pitch*sin_roll - cos_roll*sin_yaw;

	mat->m[1][0] = -cos_yaw*sin_roll + sin_yaw*sin_pitch*cos_roll;
	mat->m[1][1] =  cos_pitch*cos_roll;
	mat->m[1][2] =  sin_yaw*sin_roll + cos_yaw*sin_pitch*cos_roll;

	mat->m[2][0] =  cos_pitch*sin_yaw;
	mat->m[2][1] = -sin_pitch;
	mat->m[2][2] =  cos_yaw*cos_pitch;
}

double SquareMagnitude(const VEC3& v)
{
   return v.x*v.x + v.y*v.y + v.z*v.z;
}

double Magnitude(const VEC3& v)
{
   return sqrt(SquareMagnitude(v));
}

VEC3 Normalize(const VEC3& v)
{
	double mag = Magnitude(v);
	if (mag)
		return v / mag;
	else
		return v;
}

VEC3 Normalize(const double x, const double y, const double z)
{
	VEC3 v(x,y,z);
	double mag =  sqrt( v.x*v.x + v.y*v.y + v.z*v.z );
	if (mag)
		return v / mag;
	else
		return v;
}

double Distance(VEC3 vPos1, VEC3 vPos2)
{
	// Calculate the distance between 2 3d points.
	//

	return sqrt((vPos1.x - vPos2.x)*(vPos1.x - vPos2.x)
		+ (vPos1.y - vPos2.y)*(vPos1.y - vPos2.y)
		+ (vPos1.z - vPos2.z)*(vPos1.z - vPos2.z));
}

double Distance(double x0, double y0, double x1, double y1)
{
	// Calculate the distance between 2 2d points.
	//

	return sqrt((x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0));
}

double Distance(const VEC2 p0, const VEC2 p1)
{
	// Calculate the distance between 2 2d points.
	//

	return sqrt((p1.x - p0.x)*(p1.x - p0.x) + (p1.y - p0.y)*(p1.y - p0.y));
}

double Distance(PointD pt0, PointD pt1)
{
	// Convenience overload.
	//

	return Distance(pt0.x, pt0.y, pt1.x, pt1.y);
}

VEC3 CrossProduct(const VEC3& v1, const VEC3& v2)
{
	// returns a vector normal to both input vectors
	//

	VEC3 result;

	result[0] = v1[1] * v2[2] - v1[2] * v2[1];
	result[1] = v1[2] * v2[0] - v1[0] * v2[2];
	result[2] = v1[0] * v2[1] - v1[1] * v2[0];

	return result;
}

double CrossProduct(const VEC2& v1, const VEC2 v2)
{
	// mathwolrd says:
	// for two 2d vectors U = (Ux, Uy) V = (Vx, Vy)
	// the crossproduct is:
	//		U x V = Ux*Vy - Uy*Vx

	return v1.x * v2.y - v1.y - v2.x;
}

void swap(float& f1, float& f2)
{
	float t = f1;
	f1 = f2;
	f2 = t;
}

void swap(double& f1, double& f2)
{
	double t = f1;
	f1 = f2;
	f2 = t;
}

void swap(int& i1, int& i2)
{
	int t = i1;
	i1 = i2;
	i2 = t;
}

int sign(double val)
{
	// return the sign a number
	//

	if (val < 0.0)
		return -1;
	else if (val > 0.0)
		return 1;
	else
		return 1;
}

bool IsBetween(double val, double loVal, double hiVal)
{
	bool bReturn = false;

	if ((val >= loVal && val <= hiVal) || (val >= hiVal && val <= loVal))
		bReturn = true;

	return bReturn;
}

float Lerp(float t, float a, float b)
{
	return (a + t * (b - a));		// linear interpolation
}

double Lerp(double t, double a, double b)
{
	return (a + t * (b - a));		// linear interpolation
}

UInt8 Lerp(float t, UInt8 a, UInt8 b)
{
	return (UInt8)(a + (UInt8)(t*(b - a)));		// linear interpolation
}

float LerpLN(float t, float min, float max)
{
	// logarithmic interpolation between min & max

	// function maps linear t between min/max to logarithmic t between 0.0 and 1.0

	// notes:
	//		1. min must be greater than 0
	//		2. unlike Lerp(),this function returns a factor that can be
	// used as needed in further calculations
	//

	float minLN = logf(min);
	float maxLN = logf(max);

	float inputFactor = Lerp(t, min, max);	// map input factor to log scale input range
	float outputFactor = logf(inputFactor);	// get output value

	outputFactor = (outputFactor - minLN) / (maxLN - minLN);		// map output to log scale output range

	return outputFactor;
}

double DotProduct(const VEC3& v1, const VEC3& v2)
{
	// If both vectors are normalized, then the dot product equals to the
	// cosine of the angle between them.
	//

	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

double DotProduct(const VEC2& v1, const VEC2& v2)
{
	// 2D dot product.
	//

	return v1.x*v2.x + v1.y*v2.y;
}

double CalcAngleRad(VEC3 v1, VEC3 v2)
{
	// calculate the angle between 2 vectors

	// inputs:
	//
	// outputs:
	//		return = radians, 0.0 - PI

	// by definition, the largest possible angle between 2 vectors 
	// in 3d space is PI
	//

	double dot = DotProduct(v1, v2);		// assuming normal
	return acos(dot);
}

double CalcAngleDeg(VEC3 v1, VEC3 v2)
{
	// calculate the angle between 2 vectors

	// inputs:
	//
	// outputs:
	//		return = fractional degrees, 0.0-180.0

	// by definition, the largest possible angle between 2 vectors in 3d space
	// is 180 degrees
	//

	double angle = CalcAngleRad(v1, v2);
	return RadianToDegree(angle);
}

// 2D Vector

VEC2::VEC2()
{
	x = 0.0;
	y = 0.0;
}

VEC2::VEC2(const double* pf)
{
	x = pf[0];
	y = pf[1];
}

VEC2::VEC2(double fx, double fy)
{
	x = fx;
	y = fy;
}

// casting
VEC2::operator double* ()
{
	return (double *)&x;
}

VEC2::operator const double* () const
{
	return (const double *)&x;
}

// assignment operators
VEC2& VEC2::operator += (const VEC2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

VEC2& VEC2::operator -= (const VEC2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

VEC2& VEC2::operator *= (double f)
{
	x *= f;
	y *= f;
	return *this;
}

VEC2& VEC2::operator /= (double f)
{
	double fInv = 1.0 / f;
	x *= fInv;
	y *= fInv;
	return *this;
}

// unary operators
VEC2 VEC2::operator + () const
{
	return *this;
}

VEC2 VEC2::operator - () const
{
	return VEC2(-x, -y);
}

// binary operators
VEC2 VEC2::operator + (const VEC2& v) const
{
	return VEC2(x + v.x, y + v.y);
}

VEC2 VEC2::operator - (const VEC2& v) const
{
	return VEC2(x - v.x, y - v.y);
}

VEC2 VEC2::operator * (double f) const
{
	return VEC2(x * f, y * f);
}

VEC2 VEC2::operator / (double f) const
{
	double fInv = 1.0 / f;
	return VEC2(x * fInv, y * fInv);
}

VEC2 operator * (float f, const VEC2& v)
{
	return VEC2(f * v.x, f * v.y);
}

bool VEC2::operator == (const VEC2& v) const
{
	return x == v.x && y == v.y;
}

bool VEC2::operator != (const VEC2& v) const
{
	return x != v.x || y != v.y;
}

// methods

double VEC2::Magnitude() const
{
	return sqrt(x*x + y*y);
}

double VEC2::MagnitudeSquared() const
{
	return x*x + y*y;
}

// vector implementations

// 3D Vector

VEC3::VEC3()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

VEC3::VEC3(const double* pf)
{
	x = pf[0];
	y = pf[1];
	z = pf[2];
}

VEC3::VEC3(double fx, double fy, double fz)
{
	x = fx;
	y = fy;
	z = fz;
}

// casting

VEC3::operator double* ()
{
    return (double *) &x;
}

VEC3::operator const double* () const
{
    return (const double *) &x;
}

// assignment operators

VEC3& VEC3::operator += (const VEC3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

VEC3& VEC3::operator -= (const VEC3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

VEC3& VEC3::operator *= (double f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

VEC3& VEC3::operator /= (double f)
{
	double fInv = 1.0 / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	return *this;
}

// unary operators

VEC3 VEC3::operator + () const
{
	return *this;
}

VEC3 VEC3::operator - () const
{
	return VEC3(-x, -y, -z);
}

// binary operators

VEC3 VEC3::operator + (const VEC3& v) const
{
	return VEC3(x + v.x, y + v.y, z + v.z);
}

VEC3 VEC3::operator - (const VEC3& v) const
{
	return VEC3(x - v.x, y - v.y, z - v.z);
}

VEC3 VEC3::operator * (double f) const
{
	return VEC3(x * f, y * f, z * f);
}

VEC3 VEC3::operator / (double f) const
{
	double fInv = 1.0 / f;
	return VEC3(x * fInv, y * fInv, z * fInv);
}

bool VEC3::operator == (const VEC3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool VEC3::operator != (const VEC3& v) const
{
	return x != v.x || y != v.y || z != v.z;
}

// methods

double VEC3::Magnitude() const
{
	return sqrt(x*x + y*y + z*z);
}

double VEC3::MagnitudeSquared() const
{
	return x*x + y*y + z*z;
}

VEC3 VEC3::Unit() const
{
	// Return unit vector.

	double mag = Magnitude();
	if (mag == 0.0)
		return *this;
	return VEC3(x / mag, y / mag, z / mag);
}

// 4D Vector

VEC4::VEC4()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
	w = 0.0;
}

VEC4::VEC4(const double *pf)
{
	x = pf[0];
	y = pf[1];
	z = pf[2];
	w = pf[3];
}

VEC4::VEC4(double fx, double fy, double fz, double fw)
{
	x = fx;
	y = fy;
	z = fz;
	w = fw;
}

// casting

VEC4::operator double* ()
{
    return (double *) &x;
}

VEC4::operator const double* () const
{
    return (const double *) &x;
}

// assignment operators

VEC4& VEC4::operator += (const VEC4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

VEC4& VEC4::operator -= (const VEC4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

VEC4& VEC4::operator *= (double f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

VEC4& VEC4::operator /= (double f)
{
	double fInv = 1.0 / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	w *= fInv;
	return *this;
}

// unary operators

VEC4 VEC4::operator + () const
{
    return *this;
}

VEC4 VEC4::operator - () const
{
    return VEC4(-x, -y, -z, -w);
}

// binary operators

VEC4 VEC4::operator + (const VEC4& v) const
{
	return VEC4(x + v.x, y + v.y, z + v.z, w + v.w);
}

VEC4 VEC4::operator - (const VEC4& v) const
{
	return VEC4(x - v.x, y - v.y, z - v.z, w - v.w);
}

VEC4 VEC4::operator * (double f) const
{
	return VEC4(x * f, y * f, z * f, w * f);
}

VEC4 VEC4::operator / (double f) const
{
	double fInv = 1.0 / f;
	return VEC4(x * fInv, y * fInv, z * fInv, w * fInv);
}

bool VEC4::operator == (const VEC4& v) const
{
	return x == v.x && y == v.y && z == v.z && w == v.w;
}

bool VEC4::operator != (const VEC4& v) const
{
	return x != v.x || y != v.y || z != v.z || w != v.w;
}

// MAT3

MAT3::MAT3()
{
	// default constructor - inits to identity
	m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0;
	m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0;
	m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0;
}

MAT3::MAT3(const double* pf)
{
	memcpy(m, pf, sizeof(double) * 9);
}

MAT3::MAT3(	double f11, double f12, double f13,
			double f21, double f22, double f23,
			double f31, double f32, double f33 )
{
	m[0][0] = f11; m[0][1] = f12; m[0][2] = f13;
	m[1][0] = f21; m[1][1] = f22; m[1][2] = f23;
	m[2][0] = f31; m[2][1] = f32; m[2][2] = f33;
}

// access grants

double& MAT3::operator () (int row, int col)
{
	return m[row][col];
}

double MAT3::operator () (int row, int col) const
{
	return m[row][col];
}

// casting operators

MAT3::operator double* ()
{
    return (double *) m;
}

MAT3::operator const double* () const
{
    return (const double *) m;
}

// assignment operators

MAT3& MAT3::operator *= (const MAT3& mat)
{
	MatrixMultiply(this, this, &mat);
	return *this;
}

MAT3& MAT3::operator += (const MAT3& mat)
{
	m[0][0] += mat.m[0][0]; m[0][1] += mat.m[0][1]; m[0][2] += mat.m[0][2];
	m[1][0] += mat.m[1][0]; m[1][1] += mat.m[1][1]; m[1][2] += mat.m[1][2];
	m[2][0] += mat.m[2][0]; m[2][1] += mat.m[2][1]; m[2][2] += mat.m[2][2];
	return *this;
}

MAT3& MAT3::operator -= (const MAT3& mat)
{
	m[0][0] -= mat.m[0][0]; m[0][1] -= mat.m[0][1]; m[0][2] -= mat.m[0][2];
	m[1][0] -= mat.m[1][0]; m[1][1] -= mat.m[1][1]; m[1][2] -= mat.m[1][2];
	m[2][0] -= mat.m[2][0]; m[2][1] -= mat.m[2][1]; m[2][2] -= mat.m[2][2];
	return *this;
}

MAT3& MAT3::operator *= (double f)
{
	m[0][0] *= f; m[0][1] *= f; m[0][2] *= f;
	m[1][0] *= f; m[1][1] *= f; m[1][2] *= f;
	m[2][0] *= f; m[2][1] *= f; m[2][2] *= f;
	return *this;
}

MAT3& MAT3::operator /= (double f)
{
	double fInv = 1.0f / f;
	m[0][0] *= fInv; m[0][1] *= fInv; m[0][2] *= fInv;
	m[1][0] *= fInv; m[1][1] *= fInv; m[1][2] *= fInv;
	m[2][0] *= fInv; m[2][1] *= fInv; m[2][2] *= fInv;
	return *this;
}

// unary operators

MAT3 MAT3::operator + () const
{
    return *this;
}

MAT3 MAT3::operator - () const
{
    return MAT3( -m[0][0], -m[0][1], -m[0][2],
                 -m[1][0], -m[1][1], -m[1][2],
                 -m[2][0], -m[2][1], -m[2][2] );
}

// binary operators

MAT3 MAT3::operator * (const MAT3& mat) const
{
	MAT3 matT;
	MatrixMultiply(&matT, this, &mat);
	return matT;
}

MAT3 MAT3::operator + (const MAT3& mat) const
{
	return MAT3(
		m[0][0] + mat.m[0][0], m[0][1] + mat.m[0][1], m[0][2] + mat.m[0][2],
		m[1][0] + mat.m[1][0], m[1][1] + mat.m[1][1], m[1][2] + mat.m[1][2],
		m[2][0] + mat.m[2][0], m[2][1] + mat.m[2][1], m[2][2] + mat.m[2][2]);
}

MAT3 MAT3::operator - (const MAT3& mat) const
{
	return MAT3(
		m[0][0] - mat.m[0][0], m[0][1] - mat.m[0][1], m[0][2] - mat.m[0][2],
		m[1][0] - mat.m[1][0], m[1][1] - mat.m[1][1], m[1][2] - mat.m[1][2],
		m[2][0] - mat.m[2][0], m[2][1] - mat.m[2][1], m[2][2] - mat.m[2][2]);
}

MAT3 MAT3::operator * (double f) const
{
	return MAT3(
		m[0][0] * f, m[0][1] * f, m[0][2] * f,
		m[1][0] * f, m[1][1] * f, m[1][2] * f,
		m[2][0] * f, m[2][1] * f, m[2][2] * f);
}

MAT3 MAT3::operator / (double f) const
{
	double fInv = 1.0 / f;
	return MAT3(
		m[0][0] * fInv, m[0][1] * fInv, m[0][2] * fInv,
		m[1][0] * fInv, m[1][1] * fInv, m[1][2] * fInv,
		m[2][0] * fInv, m[2][1] * fInv, m[2][2] * fInv);
}

bool MAT3::operator == (const MAT3& mat) const
{
	return 0 == memcmp(this, &mat, sizeof(MAT3));
}

bool MAT3::operator != (const MAT3& mat) const
{
	return 0 != memcmp(this, &mat, sizeof(MAT3));
}

void MAT3::SetIdentity()
{
	m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0;
	m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0;
	m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0;
}

bool MAT3::IsIdentity() const
{
	bool identity = true;

	identity = ((m[0][0] == 1.0) && (m[0][1] == 0.0) && (m[0][2] == 0.0))
			&& ((m[1][0] == 0.0) && (m[1][1] == 1.0) && (m[1][2] == 0.0))
			&& ((m[2][0] == 0.0) && (m[2][1] == 0.0) && (m[2][2] == 1.0));

	return identity;
}

// MAT4

MAT4::MAT4()
{
	// default constructor - inits to identity
	m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
	m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = 0.0;
	m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = 0.0;
	m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = 1.0;
}

MAT4::MAT4(const double* pf)
{
	memcpy(m, pf, sizeof(double) * 16);
}

MAT4::MAT4(double f11, double f12, double f13, double f14,
	double f21, double f22, double f23, double f24,
	double f31, double f32, double f33, double f34,
	double f41, double f42, double f43, double f44)
{
	m[0][0] = f11; m[0][1] = f12; m[0][2] = f13; m[0][3] = f14;
	m[1][0] = f21; m[1][1] = f22; m[1][2] = f23; m[1][3] = f24;
	m[2][0] = f31; m[2][1] = f32; m[2][2] = f33; m[2][3] = f34;
	m[3][0] = f41; m[3][1] = f42; m[3][2] = f43; m[3][3] = f44;
}

// access grants

double& MAT4::operator () (int row, int col)
{
	return m[row][col];
}

double MAT4::operator () (int row, int col) const
{
	return m[row][col];
}

// casting operators

MAT4::operator double* ()
{
	return (double *)m;
}

MAT4::operator const double* () const
{
	return (const double *)m;
}

// assignment operators

MAT4& MAT4::operator *= (const MAT4& mat)
{
	MatrixMultiply(this, this, &mat);
	return *this;
}

MAT4& MAT4::operator += (const MAT4& mat)
{
	m[0][0] += mat.m[0][0]; m[0][1] += mat.m[0][1]; m[0][2] += mat.m[0][2]; m[0][3] += mat.m[0][3];
	m[1][0] += mat.m[1][0]; m[1][1] += mat.m[1][1]; m[1][2] += mat.m[1][2]; m[1][3] += mat.m[1][3];
	m[2][0] += mat.m[2][0]; m[2][1] += mat.m[2][1]; m[2][2] += mat.m[2][2]; m[2][3] += mat.m[2][3];
	m[3][0] += mat.m[3][0]; m[3][1] += mat.m[3][1]; m[3][2] += mat.m[3][2]; m[3][3] += mat.m[3][3];
	return *this;
}

MAT4& MAT4::operator -= (const MAT4& mat)
{
	m[0][0] -= mat.m[0][0]; m[0][1] -= mat.m[0][1]; m[0][2] -= mat.m[0][2]; m[0][3] -= mat.m[0][3];
	m[1][0] -= mat.m[1][0]; m[1][1] -= mat.m[1][1]; m[1][2] -= mat.m[1][2]; m[1][3] -= mat.m[1][3];
	m[2][0] -= mat.m[2][0]; m[2][1] -= mat.m[2][1]; m[2][2] -= mat.m[2][2]; m[2][3] -= mat.m[2][3];
	m[3][0] -= mat.m[3][0]; m[3][1] -= mat.m[3][1]; m[3][2] -= mat.m[3][2]; m[3][3] -= mat.m[3][3];
	return *this;
}

MAT4& MAT4::operator *= (double f)
{
	m[0][0] *= f; m[0][1] *= f; m[0][2] *= f; m[0][3] *= f;
	m[1][0] *= f; m[1][1] *= f; m[1][2] *= f; m[1][3] *= f;
	m[2][0] *= f; m[2][1] *= f; m[2][2] *= f; m[2][3] *= f;
	m[3][0] *= f; m[3][1] *= f; m[3][2] *= f; m[3][3] *= f;
	return *this;
}

MAT4& MAT4::operator /= (double f)
{
	double fInv = 1.0f / f;
	m[0][0] *= fInv; m[0][1] *= fInv; m[0][2] *= fInv; m[0][3] *= fInv;
	m[1][0] *= fInv; m[1][1] *= fInv; m[1][2] *= fInv; m[1][3] *= fInv;
	m[2][0] *= fInv; m[2][1] *= fInv; m[2][2] *= fInv; m[2][3] *= fInv;
	m[3][0] *= fInv; m[3][1] *= fInv; m[3][2] *= fInv; m[3][3] *= fInv;
	return *this;
}

// unary operators

MAT4 MAT4::operator + () const
{
	return *this;
}

MAT4 MAT4::operator - () const
{
	return MAT4(-m[0][0], -m[0][1], -m[0][2], -m[0][3],
		-m[1][0], -m[1][1], -m[1][2], -m[1][3],
		-m[2][0], -m[2][1], -m[2][2], -m[2][3],
		-m[3][0], -m[3][1], -m[3][2], -m[3][3]);
}

// binary operators

MAT4 MAT4::operator * (const MAT4& mat) const
{
	MAT4 matT;
	MatrixMultiply(&matT, this, &mat);
	return matT;
}

MAT4 MAT4::operator + (const MAT4& mat) const
{
	return MAT4(m[0][0] + mat.m[0][0], m[0][1] + mat.m[0][1], m[0][2] + mat.m[0][2], m[0][3] + mat.m[0][3],
		m[1][0] + mat.m[1][0], m[1][1] + mat.m[1][1], m[1][2] + mat.m[1][2], m[1][3] + mat.m[1][3],
		m[2][0] + mat.m[2][0], m[2][1] + mat.m[2][1], m[2][2] + mat.m[2][2], m[2][3] + mat.m[2][3],
		m[3][0] + mat.m[3][0], m[3][1] + mat.m[3][1], m[3][2] + mat.m[3][2], m[3][3] + mat.m[3][3]);
}

MAT4 MAT4::operator - (const MAT4& mat) const
{
	return MAT4(m[0][0] - mat.m[0][0], m[0][1] - mat.m[0][1], m[0][2] - mat.m[0][2], m[0][3] - mat.m[0][3],
		m[1][0] - mat.m[1][0], m[1][1] - mat.m[1][1], m[1][2] - mat.m[1][2], m[1][3] - mat.m[1][3],
		m[2][0] - mat.m[2][0], m[2][1] - mat.m[2][1], m[2][2] - mat.m[2][2], m[2][3] - mat.m[2][3],
		m[3][0] - mat.m[3][0], m[3][1] - mat.m[3][1], m[3][2] - mat.m[3][2], m[3][3] - mat.m[3][3]);
}

MAT4 MAT4::operator * (double f) const
{
	return MAT4(m[0][0] * f, m[0][1] * f, m[0][2] * f, m[0][3] * f,
		m[1][0] * f, m[1][1] * f, m[1][2] * f, m[1][3] * f,
		m[2][0] * f, m[2][1] * f, m[2][2] * f, m[2][3] * f,
		m[3][0] * f, m[3][1] * f, m[3][2] * f, m[3][3] * f);
}

MAT4 MAT4::operator / (double f) const
{
	double fInv = 1.0f / f;
	return MAT4(m[0][0] * fInv, m[0][1] * fInv, m[0][2] * fInv, m[0][3] * fInv,
		m[1][0] * fInv, m[1][1] * fInv, m[1][2] * fInv, m[1][3] * fInv,
		m[2][0] * fInv, m[2][1] * fInv, m[2][2] * fInv, m[2][3] * fInv,
		m[3][0] * fInv, m[3][1] * fInv, m[3][2] * fInv, m[3][3] * fInv);
}

bool MAT4::operator == (const MAT4& mat) const
{
	return 0 == memcmp(this, &mat, sizeof(MAT4));
}

bool MAT4::operator != (const MAT4& mat) const
{
	return 0 != memcmp(this, &mat, sizeof(MAT4));
}

bool MAT4::IsIdentity() const
{
	bool identity = true;

	identity = ((m[0][0] == 1.0) && (m[0][1] == 0.0) && (m[0][2] == 0.0) && (m[0][3] == 0.0))
		&& ((m[1][0] == 0.0) && (m[1][1] == 1.0) && (m[1][2] == 0.0) && (m[1][3] == 0.0))
		&& ((m[2][0] == 0.0) && (m[2][1] == 0.0) && (m[2][2] == 1.0) && (m[2][3] == 0.0))
		&& ((m[3][0] == 0.0) && (m[3][1] == 0.0) && (m[3][2] == 0.0) && (m[3][3] == 1.0));

	return identity;
}

void MatrixRotationAxis(MAT3* pMat, const VEC3* pvAxis, double theta)
{
	// inputs:
	//		pMat	= points to matrix class that will be used for output
	//		pvAxis	= axis of rotation
	//		theta	= angle, radians

	// algorithm:
	//		1) translate space so that the rotation axis passes through the origin
	//		2) rotate space About the z axis so that the rotation axis lies in the xz plane
	//		3) rotate space About the y axis so that the rotation axis lies along the z axis
	//		4) perform the desired rotation by  About the z axis
	//		5) apply the inverse of step (3)
	//		6) apply the inverse of step (2)
	//		7) apply the inverse of step (1)

	// concatenating those 7 steps, gives the matrix used here

	// from: http://www.mines.edu/~gmurray/ArbitraryAxisRotation/ArbitraryAxisRotation.html
	//

//TODO:
//shouldn't u, v, w below be base on vAxis, not av_Axis?

    //VEC3 vAxis = Normalize(*av_Axis);		// get normalized axis of rotation

    double u = pvAxis->x;	// technically, u = (d-a), where <a,b,c> and <d,e,f> are endpoints of axis line, but
    double v = pvAxis->y;	// we're passing axis as vector, which is then normalized so x == u == d-a
    double w = pvAxis->z;

    // the "double" versions of the transcendental functions
    // don't compile for gumstix
    double sin_theta = sin(theta);
    double cos_theta = cos(theta);
//double junk = u*u + v*v + w*w;		// test for == 1.0

	pMat->SetIdentity();		// default to identity

    // formally, each of these are supposed to be divided by L, which is defined as (u^2 + v^2 + w^2); I wondered
    // if it was supposed to be the sqrt() of that, but doc uses that denominator consistently; in any case, as used
    // here, axis of rotation is unit vector, so should work out to 1.0 anyway;

    // note also, 4th item in each group has a rather complicated definition, but is set to 0; i'm pretty sure this
    // is the translation, but as used by me (& ms for that matter), the axis of rotation always passes through the
    // origin; any translation, is done outside this function (the variables a, b, c are 1 endpoint of the axis of
    // rotation, but that's bypassed here because we define the axis as a vector directly, rather than 2 endpoints)

    // also, rows/columns flipped for alignment with ms coordinate system

    pMat->m[0][0] = u*u + (v*v + w*w) * cos_theta;				// [ u^2 + (v^2 + w^2) * cos(theta) ] / L
    pMat->m[1][0] = u*v * (1.0f-cos_theta) - w * sin_theta;		// [uv(1-cos(theta))-w*sqrt(L)*sin(theta)] / L
    pMat->m[2][0] = u*w * (1.0f-cos_theta) + v * sin_theta;		// [uw(1-cos(theta))+v*sqrt(L)*sin(theta)] / L
    //pMat->m[3][0] = 0.0;	// [ a(v^2+w^2)-u(bv-cw)+(u(bv+cw)-a(v^2+w^2))*cos(theta)+(bw-cv)*sqrt(L)*sin(theta) ] / L

    pMat->m[0][1] = u*v * (1.0f-cos_theta) + w * sin_theta;		// [uv(1-cos(theta))+w*sqrt(L)*sin(theta)] / L
    pMat->m[1][1] = v*v + (u*u + w*w) * cos_theta;				// [ v^2 + (u^2 + w^2) * cos(theta) ] / L
    pMat->m[2][1] = v*w * (1.0f-cos_theta) - u * sin_theta;		// [vw(1-cos(theta))-u*sqrt(L)*sin(theta)] / L
    //pMat->m[3][1] = 0.0;	// [ b(u^2+w^2)-v(au+cw)+(v(au+cw)-b(u^2+w^2))*cos(theta)+(cu-aw)*sqrt(L)*sin(theta) ] / L

    pMat->m[0][2] = u*w * (1.0f-cos_theta) - v * sin_theta;		// [uw(1-cos(theta))-v*sqrt(L)*sin(theta)] / L
    pMat->m[1][2] = v*w * (1.0f-cos_theta) + u * sin_theta;		// [vw(1-cos(theta))+u*sqrt(L)*sin(theta)] / L
    pMat->m[2][2] = w*w + (u*u + v*v) * cos_theta;				// [ w^2 + (u^2 + v^2) * cos(theta) ] / L
    //pMat->m[3][2] = 0.0;	// [ c(u^2+v^2)-w(au+bv)+(w(au+bv)-c(u^2+v^2))*cos(theta)+(av-bu)*sqrt(L)*sin(theta) ] / L
}

VEC3 Vec3Transform(const VEC3 vec_in, const MAT3 mat_in)
{
	// Transform 3D vector by a given matrix
	//
	// Inputs:
	//		vec_in = input vector
	//		mat_in = input matrix
	// Outputs:
	//		return = transformed vector
	//
	// Note: To multiply a vector by a matrix, the matrix must have the
	//		 same # of columns as the vectors does elements (3 here).
	//

	VEC3 vec_out;
	
	vec_out.x = vec_in.x * mat_in.m[0][0] + vec_in.y * mat_in.m[0][1] + vec_in.z * mat_in.m[0][2];
	vec_out.y = vec_in.x * mat_in.m[1][0] + vec_in.y * mat_in.m[1][1] + vec_in.z * mat_in.m[1][2];
	vec_out.z = vec_in.x * mat_in.m[2][0] + vec_in.y * mat_in.m[2][1] + vec_in.z * mat_in.m[2][2];

	return vec_out;
}

double CalcYIntercept(double x, double x1, double y1, double x2, double y2)
{
	// return the y-intercept of a vertical line at a point on the x axis
	//

	double intercept = 0.0;

	if (fabs(x2 - x1) > 0.0001)	// test for vertical line, y-intercept undefined
	{
		double slope = (y2 - y1) / (x2 - x1);
		intercept = y1 + slope * (x - x1);
	}

	return intercept;
}

bool PointInPoly(double x, double y, const std::vector<VEC2> vxList)
{
	// 2d point in polygon test
	//
	// inputs:
	//		(x,y)	= test point
	//		vxList	= list of vertices
	//

	bool bAbove = false;
	bool bBelow = false;
	double intercept = 0.0;		// y-intercept

	int vxCount = static_cast<int>(vxList.size());

	// if vline xsects 1 side above & 1 side below

	for (int i = 0; i < vxCount-1; ++i)
	{
		// within range of side n

		if ((x > vxList[i].x && x < vxList[i+1].x) || (x > vxList[i+1].x && x < vxList[i].x))
		{
			intercept = CalcYIntercept(x, vxList[i].x, vxList[i].y, vxList[i+1].x, vxList[i+1].y);
			if (intercept < y)
				bBelow = true;		// side is below point
			if (intercept > y)
				bAbove = true;		// side is above point
		}

		// once it's crossed an edge of a (convex) polygon both above and
		// below the test point, we're done

		if (bBelow && bAbove)
			break;

	}

	// close it up - test line from Vx #n back to Vx #0

	if (!(bBelow && bAbove))	// not done
	{
		if ((x > vxList[0].x && x < vxList[vxCount-1].x) || (x > vxList[ vxCount-1].x && x < vxList[0].x))
		{
			intercept = CalcYIntercept(x, vxList[0].x, vxList[0].y, vxList[vxCount-1].x, vxList[vxCount-1].y);
			if (intercept < y)
				bBelow = true;		// side is below point
			if (intercept > y)
				bAbove = true;		// side is above point
		}
	}

	// point is in polygon if there's a side directly above the point
	// and another side directly below the point

	return (bAbove && bBelow);
}

bool PointInRect(double testX, double testY, RectD rct)
{
	// point in rectangle test
	//
	// inputs:
	//		testX/testY = test point
	//		rct	= rectangle
	//

	return (IsBetween(testX, rct.x0, rct.x1) && IsBetween(testY, rct.y0, rct.y1));
}

bool IntersectLineLine2D(RectD testLine1, RectD testLine2, PointD& xsect)
{
	// find intersection of 2d line segments
	//
	// inputs:
	//		testLine1 = line segment #1
	//		testLine2 = line segment #2
	//
	// outputs:
	//		return	= true if intersection exists
	//		xsect	= point of intersection
	//
	// source: http://www.faqs.org/faqs/graphics/algorithms-faq/
	//
	//			(Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
	//		r = -----------------------------  (eqn 1)
	//			(Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
	//
	//			(Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
	//		s = -----------------------------  (eqn 2)
	//			(Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
	//
	//		where, (A,B) = line 1, (C,D) = line 2
	//

	bool bReturn = true;
	double r = 0.0;
	double s = 0.0;

	//float fDenom = ((Bx - Ax) * (Dy - Cy) - (By - Ay) * (Dx - Cx));
	double denom = ((testLine1.x1 - testLine1.x0)*(testLine2.y1 - testLine2.y0)
						- (testLine1.y1 - testLine1.y0)*(testLine2.x1 - testLine2.x0));

	if (denom == 0.0)	// parallel
	{
		bReturn = false;
	}
	else
	{
		r = ((testLine1.y0 - testLine2.y0) * (testLine2.x1 - testLine2.x0)
				- (testLine1.x0 - testLine2.x0) * (testLine2.y1 - testLine2.y0)) / denom;
		s = ((testLine1.y0 - testLine2.y0) * (testLine1.x1 - testLine1.x0)
				- (testLine1.x0 - testLine2.x0) * (testLine1.y1 - testLine1.y0)) / denom;

		//    Let P be the position vector of the intersection point, then
		//
		//        P=A+r(B-A) or
		//
		//        Px = Ax + r(Bx - Ax)
		//        Py = Ay + r(By - Ay)

		double x = testLine1.x0 + r * (testLine1.x1 - testLine1.x0);
		double y = testLine1.y0 + r * (testLine1.y1 - testLine1.y0);

		// If 0<=r<=1 & 0<=s<=1, intersection exists
		//            r<0 or r>1 or s<0 or s>1 line segments do not intersect
		//
		//        If the denominator in eqn 1 is zero, AB & CD are parallel
		//        If the numerator in eqn 1 is also zero, AB & CD are collinear.

		if ((r < 0.0 || r > 1.0) || (s < 0.0 || s > 1.0))		// segments do not intersect
		{
			bReturn = false;
		}
		else
		{
			xsect.x = x;
			xsect.y = y;
		}
	}

	return bReturn;
}

bool IntersectPolyPoly2D(const std::vector<VEC2> vxPoly1, const std::vector<VEC2> vxPoly2)
{
	// test 2 N-sided polygons for intersection
	//
	// inputs:
	//		vxPoly1  = polygon 1
	//		vxPoly2  = polygon 2
	//

	bool bReturn = false;

	int vxCount1 = static_cast<int>(vxPoly1.size());
	int vxCount2 = static_cast<int>(vxPoly2.size());

	// for each side of poly 1
	for (int sideIdx1 = 0; sideIdx1 < vxCount1; ++sideIdx1)
	{
		RectD line1;
		line1.x0 = vxPoly1[sideIdx1].x;
		line1.y0 = vxPoly1[sideIdx1].y;
		if (sideIdx1 == vxCount1 - 1)
		{
			line1.x1 = vxPoly1[0].x;
			line1.y1 = vxPoly1[0].y;
		}
		else
		{
			line1.x1 = vxPoly1[sideIdx1 + 1].x;
			line1.y1 = vxPoly1[sideIdx1 + 1].y;
		}

		// for each side of poly 2
		for (int sideIdx2 = 0; sideIdx2 < vxCount2; sideIdx2++)
		{
			RectD line2;
			line2.x0 = vxPoly2[sideIdx2].x;
			line2.y0 = vxPoly2[sideIdx2].y;
			if (sideIdx2 == vxCount2 - 1)
			{
				line2.x1 = vxPoly2[0].x;
				line2.y1 = vxPoly2[0].y;
			}
			else
			{
				line2.x1 = vxPoly2[sideIdx2 + 1].x;
				line2.y1 = vxPoly2[sideIdx2 + 1].y;
			}

			// if sides intersect
			PointD point;
			if (IntersectLineLine2D(line1, line2, point))
			{
				bReturn = true;
				goto ExitRtn;		// break all loops
			}
		}
	}

	if (!bReturn)		// no edge-edge intersection, see if 1 poly contains the other
	{
		if (PointInPoly(vxPoly1[0].x, vxPoly1[0].y, vxPoly2))
			bReturn = true;
		else if (PointInPoly(vxPoly2[0].x, vxPoly2[0].y, vxPoly1))
			bReturn = true;
	}

ExitRtn:

	return bReturn;
}

void Rotate2d(double thetaRad, double x, double y, double* x1, double* y1, double cx /* = 0.0 */, double cy /* = 0.0 */)
{
	// rotate a point in 2d
	//
	// overload that rotates About specified point
	//
	// simple routine for rotating 1 point About a single axis; rotations involving
	// large #'s of vertices should use concatenated matrices or quaternians
	//
	// inputs:
	//		thetaRad	= angle - radians
	//		cx, cy		= point About which to rotate
	//		x, y		= point to rotate
	//
	// outputs:
	//		*x1, *y1	= rotated point
	//

	double fcos = cos(thetaRad);
	double fsin = sin(thetaRad);

	double fX = x - cx;
	double fY = y - cy;

	*x1 = fX * fcos + fY * fsin;
	*y1 = fY * fcos - fX * fsin;

	*x1 += cx;
	*y1 += cy;
}

void RotateX(double theta, VEC3& vVec)
{
	// rotate point About X axis

	// inputs:
	//		theta = angle, radians
	//		vVec = point to rotate
	//

	double fcos = cos(theta);
	double fsin = sin(theta);

	double y = vVec.y;
	double z = vVec.z;

	vVec.z = z * fcos + y * fsin;
	vVec.y = -z * fsin + y * fcos;
}

void RotateY(double theta, VEC3& vVec)
{
	// rotate point About Y axis
	//
	// inputs:
	//		theta = angle, radians
	//		vVec = point to rotate
	//

	double fcos = cos(theta);
	double fsin = sin(theta);

	double x = vVec.x;
	double z = vVec.z;

	vVec.x = x * fcos + z * fsin;
	vVec.z = -x * fsin + z * fcos;
}

void RotateZ(double theta, VEC3& vVec)
{
	// rotate point About Z axis

	// inputs:
	//		theta = angle - radians
	//		vVec = point to rotate
	//

	double fcos = cos(theta);
	double fsin = sin(theta);

	double x = vVec.x;
	double y = vVec.y;

	vVec.x = x * fcos + y * fsin;
	vVec.y = -x * fsin + y * fcos;
}

void UpHeadingToPitchRoll(VEC3 vUp, double heading, double& pitch, double& roll)
{
	// given an up vector and a heading, return the pitch & roll angle, radians

	// this is useful when a surface normal is known & we want to calculate
	// related angles
	//

	VEC3 vUpPitch;
	VEC3 vUpRoll;
	VEC3 vUpRef = VEC3(0, 1, 0);		// up

	double pitchRad;		// pitch angle, radians
	double rollRad;			// roll angle, radians

	// un-rotate it About y axis so heading = 0

	Rotate2d(-heading, vUp.x, vUp.z, &vUp.x, &vUp.z);

	// calc pitch

	vUpPitch.x = 0.0;						// drop x to project into pitch plane
	vUpPitch.y = vUp.y;
	vUpPitch.z = vUp.z;

	double cosTheta = DotProduct(vUpRef, Normalize(vUpPitch));
	if (vUpPitch.z < 0.0)					// we're pitched "up"
		pitchRad = acos(cosTheta);
	else
		pitchRad = -acos(cosTheta);

	// calc roll

	vUpRoll.x = vUp.x;
	vUpRoll.y = vUp.y;
	vUpRoll.z = 0.0;

	cosTheta = DotProduct(vUpRef, Normalize(vUpRoll));
	if (vUpRoll.x < 0.0)					// rolling left
		rollRad = -acos(cosTheta);
	else
		rollRad = acos(cosTheta);

	pitch = pitchRad;
	roll = rollRad;
}

void UpHeadingToUpDir(VEC3 vUp, double heading, VEC3* pvDir)
{
	// given an UP vector and heading, return the DIR vector aligned with the
	// heading and orthogonal to the UP vector
	//
	// inputs:
	//		vUp = UP vector
	//		heading = desired heading for DIR vector, radians
	//
	// outputs:
	//		pvDir = output DIR vector
	//

	// start with UP = <0,1,0> and DIR = <0,0,1>
	VEC3 UP = VEC3(0, 1, 0);
	VEC3 DIR = VEC3(0, 0, 1);

	// rotate DIR to align with heading
	double x1, z1;
	Rotate2d(heading, DIR.x, DIR.z, &x1, &z1);
	DIR.x = x1;
	DIR.z = z1;

	// now rotate both DIR and UP to align with desired UP
	double cosTheta = DotProduct(vUp, UP);
	double angle = acos(cosTheta);
	VEC3 vAxis = CrossProduct(DIR, UP);
	MAT3 mat;
	MatrixRotationAxis(&mat, &vAxis, angle);
	*pvDir = Vec3Transform(DIR, mat);
}

double CalcBearing(const PointD pt0, const PointD pt1)
{
	// Return the relative direction, in radians, from 1 point to another
	//
	// Inputs:
	//		vSrcPos  = starting point
	//		vDestPos = ending point
	//
	// Outputs:
	//		return = direction from starting point to ending point, radians
	//

	double bearing = 0.0;		// return value

	double dx = pt1.x - pt0.x;
	double dy = pt1.y - pt0.y;

	// get arctangent, atan2() is "well defined over all
	// points except origin"

	double fAtan = atan2(fabs(dy), fabs(dx));

	// calculate the quadrant
	if (dx >= 0.0)
	{
		if (dy < 0.0)						// quadrant == 2
			bearing = (PI / 2.0) + fAtan;
		else								// quadrant == 1
			bearing = (PI / 2.0) - fAtan;
	}
	else
	{
		if (dy < 0.0)						// quadrant == 3
			bearing = (PI*1.5f) - fAtan;
		else								// quadrant == 4
			bearing = (PI*1.5f) + fAtan;
	}

	return bearing;
}

double ModGTE(double value, double divisor)
{
	// return next float greater than or equal to value that is evenly divisible by divisor
	//
	// eg, 5.2, 0.5 -> 5.5
	//

	double fReturn = value;

	divisor = fabs(divisor);	// don't accept negative divisors

	double fracpart = fabs(fmod(value, divisor));
	if (fracpart > 0.0)
	{
		if (value < 0.0)
		{
			fReturn = value + fracpart;
		}
		else
		{
			fReturn = value + divisor - fracpart;
		}
	}

	return fReturn;
}

double ModLTE(double value, double divisor)
{
	// return next float less than or equal to value that is evenly divisible by divisor
	//

	double fReturn = value;

	divisor = fabs(divisor);	// don't accept negative divisors

	double fracpart = fabs(fmod(value, divisor));
	if (fracpart > 0.0)
	{
		if (value < 0.0)
		{
			fReturn = value - divisor + fracpart;
		}
		else
		{
			fReturn = value - fracpart;
		}
	}

	return fReturn;
}

double LeastSquares(std::vector<PointD> pts, double& b, double& m, bool bInvert /* = false */)
{
	// calculate line that best fits the data, in the form:
	//
	//		y = b + mx
	//
	// inputs:
	//		pts		= array (x,y) data to analyze - must be more than 5 points
	//		bInvert	= true => invert the orientation of the x/y axes, good for lines
	//						  that are vertical or nearly vertical
	//
	// outputs:
	//		b, m	= line that best fits (linear) data, in the form y = b + mx (or
	//		x		= b + my if invert flag is set)
	//		return	= regression coefficient between 0.0-1.0, where 1.0 is perfect
	//

	double SumX = 0.0;		// sum of x values
	double SumY = 0.0;		// sum of y values
	double SumXY = 0.0;		// sum of xy products
	double SumXX = 0.0;		// sum of x^2 products
	double SumYY = 0.0;		// sum of y^2 products

	double N = (double)pts.size();		// # of points

	b = 0.0;
	m = 0.0;

	// calculate component sums

	for (int i = 0; i < pts.size(); ++i)
	{
		if (bInvert)
			swap(pts[i].x, pts[i].y);		// swap em

		SumX += pts[i].x;
		SumY += pts[i].y;
		SumXY += pts[i].x * pts[i].y;
		SumXX += pts[i].x * pts[i].x;
		SumYY += pts[i].y * pts[i].y;
	}

	double denom = N*SumXX - SumX*SumX;	// both b & m calcs have same denominator

	b = (SumXX * SumY - SumXY*SumX) / denom;
	m = (N*SumXY - SumX * SumY) / denom;

	//TODO:
	//need more testing - seems right in many examples, BUT if i pass a perfectly horizontal 
	//line with points that fit perfectly - I get a denominator of zero here and therefore
	//undefined r value; but would expect to get a 1.0 - perfect fit ?!?!?!

	double r = (N*SumXY - SumX*SumY) / sqrt((N*SumXX - SumX*SumX) * (N*SumYY - SumY*SumY));

	return r*r;		// return r^2 value
}

UInt32 Factorial(UInt32 value)
{
	// return the factorial of a number
	//

	UInt32 nReturn = value;		// init

	if (!nReturn)	// 0 factorial = 1
	{
		nReturn = 1;
	}
	else
	{
		while (--value > 1)
			nReturn *= value;
	}

	return nReturn;
}

double Clamp(double value, double minVal, double maxVal)
{
	if (value < minVal)
		return minVal;
	else if (value > maxVal)
		return maxVal;
	else
		return value;
}

float Clamp(float value, float minVal, float maxVal)
{
	if (value < minVal)
		return minVal;
	else if (value > maxVal)
		return maxVal;
	else
		return value;
}

int Clamp(int value, int minVal, int maxVal)
{
	if (value < minVal)
		return minVal;
	else if (value > maxVal)
		return maxVal;
	else
		return value;
}

double DistancePointToLine2D(PointD point, RectD line)
{
	// according to mathworld, unsigned distance from point to line is:
	//
	//	|(y2-y1)*(x0-x1) - (x2-x1)*(y0-y1)|
	//	-----------------------------------
	//		sqrt((x2-x1)^2 + (y2-y1)^2)
	//
	// where,
	//		(x0,y0)	        = point
	//		(x1,y1)-(x2,y2) = line
	//

	double fNum = fabs((line.y1 - line.y0)*(point.x - line.x0) - (line.x1 - line.x0)*(point.y - line.y0));
	double fDen = sqrt((line.x1 - line.x0)*(line.x1 - line.x0) + (line.y1 - line.y0)*(line.y1 - line.y0));

	return fNum / fDen;
}

double DistancePointToPlane(const VEC3 vTestPoint, const VEC3 vPlanePoint, const VEC3 vPlaneNormal)
{
	// Calculate the distance from a point to a plane.
	//
	// D = - (Ax + By + Cz)
	//
	// Inputs:
	//		vTestPoint   = point of interest
	//		vPlanePoint  = point on the plane
	//		vPlaneNormal = plane's normal vector
	// Outputs:
	//		return = distance (negative indicates behind plane relative to normal dir)
	//

	// normalize the vector
	VEC3 N = vPlaneNormal.Unit();

	return N.x * (vTestPoint.x - vPlanePoint.x)
		 + N.y * (vTestPoint.y - vPlanePoint.y)
		 + N.z * (vTestPoint.z - vPlanePoint.z);
}

double DistancePointToPlane(const VEC3 vTestPoint, const PlaneNP plane)
{
	// Convenience overload.

	return DistancePointToPlane(vTestPoint, plane.P, plane.N);
}

double DistancePointToPoly(double x, double y, const std::vector<VEC2> vxList)
{
	// Calculate minimun distance from single point to polyline.
	//
	// Inputs:
	//		(x,y)  = test point
	//		vxList = polygon/polyline
	// Outputs:
	//		return = minimum distance
	//

	if (vxList.size() < 1)
		return 0.0;

	const VEC2 pt = VEC2(x, y);
	double dmin = Distance(vxList[0], pt);
	for (auto vx : vxList)
	{
		double d = Distance(vx, pt);
		if (dmin > d)
			dmin = d;
	}

	return dmin;
}

double IntersectLinePerpPoint(VEC3 P0, VEC3 P1, VEC3 P2)
{
	//	return the closest point along a line segment (P1P2) to the point P0
	//
	//	inputs:
	//		P0 = test point
	//		P1,P2 = end points of line segment
	//
	//	outputs:
	//		returns t, parametric point on line between P1 (t = 0.0) and P2 (t = 1.0)
	//
	//	Notes:
	//		1. the actual point can be easily calculated by muliplying t by the vector P2-P1
	//		2. this can also be interpreted as the point at which a line perpendicular to P1P2 and
	//		passing thru P0 intersects the line segment P1P2 (that's why it's an Intersect function)
	//		3. the return value is between 0.0 and 1.0 if the closest point is on the segment P1P2,
	//		> 1.0 if the closest point (to the line defined by P1P2) is past P2 and < 0.0 if the
	//		point is past P1
	//
	//	Math:
	//
	//      t = - (P1-P0) dot (P2-P1)
	//			  -------------------
	//                |P2-P1|^2
	//

	VEC3 V1 = P1 - P0;
	VEC3 V2 = P2 - P1;

	return -DotProduct(V1, V2) / SquareMagnitude(V2);
}

bool IntersectLineRect2D(RectD rect, RectD line, RectD* pClippedLine)
{
	// Clip a line segment against a rectangle in 2d.
	//
	// Inputs:
	//		rect = 2d rect
	//		line = 2d line
	//
	// Outputs:
	//		pClippedLine = clipped line (inside rect), if return = true
	//		return = true if line intersects rect
	//

	RectD side;
	PointD XSect[2];
	int nXSectCtr = 0;

	*pClippedLine = line;		// default to unclipped

	bool bPoint1In = IsBetween(line.x0, rect.x0, rect.x1) && IsBetween(line.y0, rect.y0, rect.y1);
	bool bPoint2In = IsBetween(line.x1, rect.x0, rect.x1) && IsBetween(line.y1, rect.y0, rect.y1);

	if (bPoint1In && bPoint2In)
	{
		// both points inside rect, return true because some portion of the line (in this case, all
		// of it) lies within rect and set the clipped output line to the input line
		return true;
	}
	else
	{
		side.x0 = rect.x0; side.y0 = rect.y0;
		side.x1 = rect.x1; side.y1 = rect.y0;
		if (IntersectLineLine2D(side, line, XSect[nXSectCtr]))
			nXSectCtr++;		// push it

		side.x0 = rect.x1; side.y0 = rect.y0;
		side.x1 = rect.x1; side.y1 = rect.y1;
		if (IntersectLineLine2D(side, line, XSect[nXSectCtr]))
			nXSectCtr++;		// push it

		if (nXSectCtr < 2)
		{
			side.x0 = rect.x0; side.y0 = rect.y1;
			side.x1 = rect.x1; side.y1 = rect.y1;
			if (IntersectLineLine2D(side, line, XSect[nXSectCtr]))
				nXSectCtr++;		// push it

			if (nXSectCtr < 2)
			{
				side.x0 = rect.x0; side.y0 = rect.y0;
				side.x1 = rect.x0; side.y1 = rect.y1;
				if (IntersectLineLine2D(side, line, XSect[nXSectCtr]))
					nXSectCtr++;		// push it
			}
		}

		if (nXSectCtr == 2)	// replace both endpoints
		{
			pClippedLine->x0 = XSect[0].x;
			pClippedLine->y0 = XSect[0].y;
			pClippedLine->x1 = XSect[1].x;
			pClippedLine->y1 = XSect[1].y;
		}
		else if (nXSectCtr == 1)		// replace outside endpoint
		{
			if (bPoint1In)	// line endpoint 1 inside rect
			{
				pClippedLine->x1 = XSect[0].x;	// replace endpoint 2
				pClippedLine->y1 = XSect[0].y;
			}
			else if (bPoint2In)	// line endpoint 2 inside rect
			{
				pClippedLine->x0 = XSect[0].x;	// replace endpoint 1
				pClippedLine->y0 = XSect[0].y;
			}
		}
	}

	return nXSectCtr > 0;
}

bool IntersectTriangleTriangle(VEC3 V0, VEC3 V1, VEC3 V2, VEC3 U0, VEC3 U1, VEC3 U2, VEC3* I0, VEC3* I1)
{
	// test for and return intersection of 2 triangles
	//
	// inputs:
	//		V0,V1,V2 = vertices of triangle 1
	//		U0,U1,U2 = vertices of triangle 2
	//
	// outputs:
	//		return	= true => triangles intersect
	//		I0,I1	= line of intersection
	//
	// algorithm:
	//
	// takes advantage of fact that there are exactly 2 edge/triangle intersections if the triangles
	// intersect - may be 2 edges from 1 triangle intersect other or 1 edge from each triangle intersecting
	// the other, but the total can only be 2;
	//
	// the line between these 2 points is the line of intersection
	//
	// so,
	//
	//		for tri 1 edge = 0 to 2	// test edges in triangle 1 against triangle 2
	//			if edge intersects triangle 2
	//			add intersect point to list
	//			if list size == 2
	//				bail, we're done
	//
	//		for tri 2 edge = 0 to 2	// test edges in triangle 2 against triangle 1
	//			if edge intersects triangle 1
	//				add intersect point to list
	//				if list size == 2
	//					bail, we're done
	//
	//		if list size == 2
	//			intersection line endpoint #1 = list entry 0
	//			intersection line endpoint #2 = list entry 1
	//			return true
	//		else
	//			return false
	//

//dir = true direction or just pointed right way?
//doesn't handle coplanar tris, but IntersectRayTri() does - wouldn't take too much to handle it here

//TODO:
//not tested at all...
	VEC3 vXSect[2];		// line of intersection, if it exists
	VEC3 vIntersect (0,0,0);
	int nXSectCtr = 0;

	// test edges of first triangle against second

	if (1 == IntersectLineTriangle(V0, V1, U0, U1, U2, &vIntersect))
		vXSect[nXSectCtr++] = vIntersect;

	if (1 == IntersectRayTriangle(V1, V2, U0, U1, U2, &vIntersect))
		vXSect[nXSectCtr++] = vIntersect;

	if (nXSectCtr < 2)
	{
		if (1 == IntersectRayTriangle(V2, V0, U0, U1, U2, &vIntersect))
			vXSect[nXSectCtr++] = vIntersect;

		if (nXSectCtr < 2)
		{
			// test edges of second triangle against first

			if (1 == IntersectRayTriangle(U0, U1, V0, V1, V2, &vIntersect))
				vXSect[nXSectCtr++] = vIntersect;

			if (nXSectCtr < 2)
			{
				if (1 == IntersectRayTriangle(U1, U2, V0, V1, V2, &vIntersect))
					vXSect[nXSectCtr++] = vIntersect;

				if (nXSectCtr < 2)
				{
					if (1 == IntersectRayTriangle(U2, U0, V0, V1, V2, &vIntersect))
						vXSect[nXSectCtr++] = vIntersect;
				}
			}
		}
	}

	if (nXSectCtr < 2)
		return false;

	*I0 = vXSect[0];
	*I1 = vXSect[1];
	return true;
}

int IntersectLineTriangle(VEC3 S0, VEC3 S1, VEC3 V0, VEC3 V1, VEC3 V2, VEC3* I)
{
	//===================================================================
	// Copyright 2001, softSurfer (www.softsurfer.com)
	// This code may be freely used and modified for any purpose
	// providing that this copyright notice is included with it.
	// SoftSurfer makes no warranty for this code, and cannot be held
	// liable for any real or imagined damage resulting from its use.
	// Users of this code must verify correctness for their application.
	//=====================================================================

	// intersect a line segment with a 3D triangle

	// inputs:
	//		S0,S1 = line segment endpoints
	//		V0,V1,V2 = vertices of triangle

	// outputs:
	//		return = -1 => triangle is invalid (a segment or point)
	//				  0 => disjoint (no intersect)
	//				  1 => intersect in unique point I1
	//				  2 => are in the same plane
	//				 *I = intersection point (when it exists)

	//notes:
	//1. this function very similar to IntersectRayTri() but i thought separate functions would be
	//clearer than differing interpretations of the parameters and/or optional parms;
	//

	const double SMALL_NUM = 0.00000001; // anything that avoids division overflow

	VEC3 u, v;			// triangle vectors
	VEC3 dir;			// line segment as vector
	VEC3 N;				// triangle normal
	VEC3 w0, w;			// ray vectors
	double r, a, b;		// params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	u = V1 - V0;
	v = V2 - V0;
	N = CrossProduct(u, v);			// cross product for plane normal

	if (N.Magnitude() == 0.0)       // triangle is invalid
		return -1;					// do not deal with this case

	dir = S1 - S0;
	w0 = S0 - V0;
	a = DotProduct(N, w0);
	b = DotProduct(N, dir);
	if (fabs(b) < SMALL_NUM)      // ray is parallel to triangle plane
	{
		if (a == 0)                // ray lies in triangle plane
			return 2;
		else 
			return 0;             // ray disjoint from plane
	}

	// get intersect point of ray with triangle plane
	r = -a / b;
	if (r < 0.0)                   // line goes away from triangle
		return 0;                  // => no intersect
	if (r > 1.0)                   // line doesn't reach triangle
		return 0;                  // => no intersect

	// original function was using the ray's "endpoint-origin" for "dir", with no normalization 
	// so it wasn't truly a direction vector; turns out that's ok because the "r" factor is automatically
	// scaled so it always returns the same intersection point when multiplied by "dir";

	*I = S0 + dir * r;           // intersect point of ray and plane

	// is I inside T?
	double uu, uv, vv, wu, wv, D;
	uu = DotProduct(u, u);
	uv = DotProduct(u, v);
	vv = DotProduct(v, v);
	w = *I - V0;
	wu = DotProduct(w, u);
	wv = DotProduct(w, v);
	D = uv * uv - uu * vv;

	// get and test parametric coords
	double s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)        // I is outside T
		return 0;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return 0;

	return 1;                      // I is in T
}

int IntersectRayTriangle(VEC3 rayOrg, VEC3 rayDir, VEC3 V0, VEC3 V1, VEC3 V2, VEC3* I)
{
	// ===================================================================
	// Copyright 2001, softSurfer (www.softsurfer.com)
	// This code may be freely used and modified for any purpose
	// providing that this copyright notice is included with it.
	// SoftSurfer makes no warranty for this code, and cannot be held
	// liable for any real or imagined damage resulting from its use.
	// Users of this code must verify correctness for their application.
	// =====================================================================
	//
	// intersect a ray with a 3D triangle
	//
	// inputs:
	//		rayOrg = ray origin
	//		rayDir = ray direction - does not need to be normalized
	//		V0,V1,V2 = vertices of triangle
	//
	// outputs:
	//		return value = -1 => triangle is invalid (a segment or point)
	//						0 => disjoint (no intersect)
	//						1 => intersect in unique point I1
	//						2 => ray would intersect at I1, but segment does not reach plane
	//						3 => are in the same plane
	//						*I = intersection point (when it exists)
	//

	const double SMALL_NUM = 0.00000001;	// anything that avoids division overflow

	VEC3 u, v;			// triangle vectors
	VEC3 N;				// triangle normal
	VEC3 w0, w;			// ray vectors
	double r, a, b;		// params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	u = V1 - V0;
	v = V2 - V0;
	N = CrossProduct(u, v);			// cross product for plane normal

	if (N.Magnitude() == 0.0)       // triangle is invalid
		return -1;					// do not deal with this case

	w0 = rayOrg - V0;
	a = DotProduct(N, w0);
	b = DotProduct(N, rayDir);
	if (fabs(b) < SMALL_NUM)		// ray is parallel to triangle plane
	{
		if (a == 0)					// ray lies in triangle plane
			return 2;
		else 
			return 0;				// ray disjoint from plane
	}

	// get intersect point of ray with triangle plane
	r = -a / b;
	if (r < 0.0)					// ray goes away from triangle
		return 0;					// => no intersect

	// for a segment, also test if (r > 1.0) => no intersect

	// original function was using the ray's "endpoint-origin" for "dir", with no normalization 
	// so it wasn't truly a direction vector; turns out that's ok because the "r" factor is automatically
	// scaled so it always returns the same intersection point when multiplied by "dir";

	*I = rayOrg + rayDir * r;           // intersect point of ray and plane

	// is I inside T?
	double uu, uv, vv, wu, wv, D;
	uu = DotProduct(u, u);
	uv = DotProduct(u, v);
	vv = DotProduct(v, v);
	w = *I - V0;
	wu = DotProduct(w, u);
	wv = DotProduct(w, v);
	D = uv * uv - uu * vv;

	// get and test parametric coords
	double s, t;
	s = (uv * wv - vv * wu) / D;
	if (s < 0.0 || s > 1.0)        // I is outside T
		return 0;
	t = (uv * wu - uu * wv) / D;
	if (t < 0.0 || (s + t) > 1.0)  // I is outside T
		return 0;

	return 1;                      // I is in T
}

int IntersectRayPlane(VEC3 rayOrg, VEC3 rayDir, VEC3 V0, VEC3 V1, VEC3 V2, VEC3* I)
{
	//===================================================================
	//	Copyright 2001, softSurfer (www.softsurfer.com)
	//	This code may be freely used and modified for any purpose
	//	providing that this copyright notice is included with it.
	//	SoftSurfer makes no warranty for this code, and cannot be held
	//	liable for any real or imagined damage resulting from its use.
	//	Users of this code must verify correctness for their application.
	//=====================================================================
	//
	//	intersect a ray with a plane defined by 3 vertices in the plane
	//
	//	inputs:
	//		rayOrg = ray origin
	//		rayDir = ray direction - does not need to be normalized
	//		V0,V1,V2 = vertices of triangle
	//
	//	outputs:
	//		return value = -1 => triangle is invalid (a segment or point)
	//						0 => disjoint (no intersect)
	//						1 => intersect in unique point I1
	//						2 => are in the same plane
	//		*I = intersection point (when it exists)
	//
	//TODO:
	//not tested, but it's just a subset of IntersectRayTri() without the polygon containment tests

	const double SMALL_NUM = 0.00000001;	// anything that avoids division overflow

	VEC3 u, v;       // triangle vectors
	VEC3 N;			// triangle normal
	VEC3 w0, w;      // ray vectors
	double r, a, b;    // params to calc ray-plane intersect

	// get triangle edge vectors and plane normal
	u = V1 - V0;
	v = V2 - V0;
	N = CrossProduct(u, v);			// cross product for plane normal

	if (Magnitude(N) == 0.0)		// invalid triangle
		return -1;					// do not deal with this case

	w0 = rayOrg - V0;
	a = DotProduct(N, w0);
	b = DotProduct(N, rayDir);
	if (fabs(b) < SMALL_NUM)      // ray is parallel to triangle plane
		return 2;

	// get intersect point of ray with triangle plane
	r = -a / b;
	if (r < 0.0)                   // ray goes away from triangle
		return 0;                  // => no intersect

	// original function was using the ray's "endpoint-origin" for "dir", with no normalization 
	// so it wasn't truly a direction vector; turns out that's ok because the "r" factor is automatically
	// scaled so it always returns the same intersection point when multiplied by "dir";

	*I = rayOrg + rayDir * r;           // intersect point of ray and plane

	return 1;                      // I is in T
}

bool IntersectRectRect(RectD& rect0, RectD& rect1)
{
	// Return true if the 2 rectangles intersect/
	//
	// 1. Check if box A is completely to the left of box B (a.left <= b.right). If so, then quit the process: they don't intersect.
	// 2. Otherwise, check if it's completely to the right (a.right >= b.left). If so, quit the process.
	// 3. If not, check if it's completely above box B (a.bottom >= b.top). If so, quit.
	// 4. If not, check if it's completely beneath box B (a.top <= b.bottom). If so, quit.
	//
	// If none of these is true, then the boxes intersect.

	rect0.Normalize();
	rect1.Normalize();

	// rect 0 completely left of rect 1
	if (rect0.x1 < rect1.x0)
		return false;
	// rect 0 completely right of rect 1
	if (rect0.x0 > rect1.x1)
		return false;
	// rect 0 completely above rect 1
	if (rect0.y1 < rect1.y0)
		return false;
	// rect 0 completely below rect 1
	if (rect0.y0 > rect1.y1)
		return false;

	return true;
}

VEC3 DegreesToVector(double headingRad, double pitchRad, double rollRad)
{
	// convert a direction/orientation specified as heading/pitch/roll radians
	// to a vector
	//

	VEC3 vDir;	// return value

	const VEC3 vDirNorth(0, 0, 1);		// unit vector oriented north in x/z plane
	double x, y, z;
	double sinHeading, cosHeading;		// heading sin/cos
	double sinRoll, cosRoll;			// roll sin/cos
	double sinPitch, cosPitch;			// pitch sin/cos

	sinHeading = sin(headingRad);
	cosHeading = cos(headingRad);
	sinPitch = sin(pitchRad);
	cosPitch = cos(pitchRad);
	sinRoll = sin(rollRad);
	cosRoll = cos(rollRad);

	// rotate About z-axis

	vDir.x = vDirNorth.x * cosRoll + vDirNorth.y * sinRoll;
	vDir.y = vDirNorth.y * cosRoll - vDirNorth.x * sinRoll;
	vDir.z = vDirNorth.z;

	x = vDir.x; y = vDir.y; z = vDir.z;	// store so they don't get overwritten

	// rotate About x-axis

	vDir.x = x;
	vDir.y = y * cosPitch + z * sinPitch;
	vDir.z = z * cosPitch - y * sinPitch;

	x = vDir.x; y = vDir.y; z = vDir.z;	// store so they don't get overwritten

	// rotate About y-axis

	vDir.x = x * cosHeading + z * sinHeading;
	vDir.y = y;
	vDir.z = z * cosHeading - x * sinHeading;

	return vDir;
}

std::vector<double> CubicSpline(double* a, double* knot, int samples)
{
	// Calculate parametric (independent x/y) bi-cubic spline.
	// 
	// This is a piecewise function calculate a curve that passes 
	// through each control point. Implicitly constrained to ensure
	// connecting curves are tangent at meeting points.
	// 
	// Values for x & y need to be calculated with separate calls.
	//
	// Orignal Source: https://github.com/aaryanx/Cubic-Spline-Interpolation
	// Which had this in comment:
	//		You can fit x = S(t) and y = S'(t), where S and S' are natural cubic spline interpolants.
	//		Sand S' give a parametric representation of the curve.
	//		

	// Inputs:
	//		a		= x or y values
	//		knot	= "knot vector"; if they are equidistant, it is "uniform"
	//		samples = size of a & t arrays
	// Outputs:
	//		return = vector of calculated x or y values (interpretation
	//				 depends on what values passed in)
	//

	std::vector<double> curve;	// return values

	std::vector<double> kt_delta(samples);		// delta between knots, for uniform, all the same value
	std::vector<double> alpha(samples);
	std::vector<double> l(samples);
	std::vector<double> u(samples);
	std::vector<double> z(samples);

	// b, c, d values in a + bx + cx^2 + dx^3
	std::vector<double> b(samples);
	std::vector<double> c(samples);
	std::vector<double> d(samples);

	// calc delta between knots
	for (int n = 0; n < samples - 1; ++n)
	{
		kt_delta[n] = knot[n + 1] - knot[n];
	}
	for (int n = 0; n < samples; ++n)
	{
		double d = 0.0;
		// i added check for n > 0, because it indexed h[] with n-1
		if (n > 0)
			d = 3 / kt_delta[n] * (a[n + 1] - a[n]) - 3 / kt_delta[n - 1] * (a[n] - a[n - 1]);
		alpha[n] = d;
	}
	l[0] = 1;
	u[0] = 0;
	z[0] = 0;
	for (int n = 1; n < samples - 1; ++n)
	{
		l[n] = 2 * (knot[n + 1] - knot[n - 1]) - kt_delta[n - 1] * u[n - 1];
		u[n] = kt_delta[n] / l[n];
		z[n] = (alpha[n] - kt_delta[n - 1] * z[n - 1]) / l[n];
	}
	// why set l? never gets used
	l[samples - 1] = 1;
	z[samples - 1] = 0;
	c[samples - 1] = 0;
	for (int n = samples - 2; n >= 0; --n)
	{
		c[n] = z[n] - u[n] * c[n + 1];
		b[n] = (a[n + 1] - a[n]) / kt_delta[n] - (kt_delta[n] * (c[n + 1] + 2 * c[n]) / 3);
		d[n] = (c[n + 1] - c[n]) / 3 * kt_delta[n];
	}

	// for each segment
	for (int n = 0; n < samples - 1; ++n)
	{
		// i think "20" is number of samples per segment
		for (int m = 0; m < 20; ++m)
		{
			// .05 comes from the fact that there are 20 samples this segment; so 
			// .05*m => 0.0 to 1.0

			// looks like this is the "a + bx + cx^2 + dx^3" polynomial - the one that fits *this* segment
			//
			// where:
			//		a = value passed in.. interesting
			//		b =
			//		c =
			//		d =
			double v = a[n] + (0.05 * m) * b[n] + (pow(0.05 * m, 2)) * c[n] + (pow(0.05 * m, 3)) * d[n];
			
			curve.push_back(v);

		}
	}

	return curve;
}

void BezierCurve(std::vector<PointD> controlPoly, std::vector<PointD>& curve, int curveCount)
{
	// Calculate bezier curve from control polygon.
	//
	// Inputs:
	//		pControlPoly = control polygon
	//		controlCount = points in control polygon
	//		curveCount = # of points desired in output curve
	//
	// Outputs:
	//		pCurve = output curve
	//
	// P(t) = Sum(i=0 to n) [B(i) (n!/(i!(n-1)!)) (t^i) (1-t)^(n-i)] ; 0 <= t <= 1.0
	//

	int n = static_cast<int>(controlPoly.size()) - 1;	// n = # of last point on control polygon, 0-based
	double factn = Factorial(n);

	curve.resize(curveCount);
	for (int pointCtr = 0; pointCtr < curveCount; ++pointCtr)		// for each output point
	{
		// calc t parameter, should vary from 0.0 to 1.0 over output points

		double t = (double)pointCtr / (double)(curveCount - 1);

		PointD pt;			// init output point
		pt.x = 0.0;
		pt.y = 0.0;

		for (int i = 0; i <= n; ++i)	// for each point on control polygon
		{
			double fact = factn / (Factorial(i)*Factorial(n-i));
			double exp  = pow(t, (double)i) * pow(1.0-t, (double)(n-i));

			// fact * exp is basically weight to give point
			pt.x += controlPoly[i].x * fact * exp;
			pt.y += controlPoly[i].y * fact * exp;
		}

		curve[pointCtr] = pt;
	}
}

void BezierCurve(PointD* pControlPoly, int controlCount, PointD* pCurve, int curveCount)
{
	// Calculate bezier curve from control polygon.
	//
	// Inputs:
	//		pControlPoly = control polygon
	//		controlCount = points in control polygon
	//		curveCount = # of points desired in output curve
	//
	// Outputs:
	//		pCurve = output curve
	//
	// P(t) = Sum(i=0 to n) [B(i) (n!/(i!(n-1)!)) (t^i) (1-t)^(n-i)] ; 0 <= t <= 1.0
	//

	int n = controlCount - 1;	// n = # of last point on control polygon, 0-based
	double factn = Factorial(n);

	for (int pointCtr = 0; pointCtr < curveCount; ++pointCtr)		// for each output point
	{
		// calc t parameter, should vary from 0.0 to 1.0 over output points

		double t = (double)pointCtr / (double)(curveCount - 1);

		PointD pt;			// init output point
		pt.x = 0.0;
		pt.y = 0.0;

		for (int i = 0; i <= n; ++i)	// for each point on control polygon
		{
			double fact = factn / (Factorial(i)*Factorial(n-i));
			double exp  = pow(t, (double)i) * pow(1.0-t, (double)(n-i));

			// fact * exp is basically weight to give point
			pt.x += pControlPoly[i].x * fact * exp;
			pt.y += pControlPoly[i].y * fact * exp;
		}

		pCurve[pointCtr] = pt;
	}
}

void interp1(double X[], double Y[], int xylen, double XI[], int xilen, double* r)
{
	// Implementation of matlab's interp1() function
	//
	// Inputs:
	//      X[] = array of x values to be interpolated
	//      Y[] = array of y values to be interpolated
	// 		xylen = length of X[] and Y[] (must be same in this implementation)
	//      XI[]  = interpolation points along x
	//		xilen = length of XI[]
	//		r = points to array of at least xilen doubles

	for (int i = 0; i < xilen; ++i)
	{
		// r[i] = lookupvalue( XI[i], i + 1);

		// find input x in x data
		for (int j = 1; j < xylen; ++j)
		{
			if (XI[i] >= X[j - 1] && XI[i] <= X[j])
			{
				double delta = (XI[i] - X[j - 1]) / (X[j] - X[j - 1]);
				r[i] = Lerp(delta, Y[j - 1], Y[j]);     // get Y at interpolated X
				break;
			}
		}
	}
}

void Bresenham(int x0, int y0, int const x1, int const y1, std::vector<PointType>& ptList)
{
	// Create line using Bresenham's algorithm.
	// Orignally from http://www.roguebasin.com/index.php?title=Bresenham%27s_Line_Algorithm.

	ptList.clear();
	
	// quick check for 1 point line
	if (x0 == x1 && y0 == y1)
	{
		ptList.push_back(PointType(x0, y0));
		return;
	}

	int delta_x(x1 - x0);
	// if x0 == x1, then it does not matter what we set here
	signed char const ix((delta_x > 0) - (delta_x < 0));
	delta_x = std::abs(delta_x) << 1;

	int delta_y(y1 - y0);
	// if y0 == y1, then it does not matter what we set here
	signed char const iy((delta_y > 0) - (delta_y < 0));
	delta_y = std::abs(delta_y) << 1;

	ptList.push_back(PointType(x0, y0));

	if (delta_x >= delta_y)
	{
		// error may go below zero
		int error(delta_y - (delta_x >> 1));

		while (x0 != x1)
		{
			// reduce error, while taking into account the corner case of error == 0
			if ((error > 0) || (!error && (ix > 0)))
			{
				error -= delta_x;
				y0 += iy;
			}
			// else do nothing

			error += delta_y;
			x0 += ix;

			ptList.push_back(PointType(x0, y0));
		}
	}
	else
	{
		// error may go below zero
		int error(delta_x - (delta_y >> 1));

		while (y0 != y1)
		{
			// reduce error, while taking into account the corner case of error == 0
			if ((error > 0) || (!error && (iy > 0)))
			{
				error -= delta_y;
				x0 += ix;
			}
			// else do nothing

			error += delta_x;
			y0 += iy;

			ptList.push_back(PointType(x0, y0));
		}
	}
}

double SCurve(double x)
{
	// Return point on S-Curve (Sigmoid function).
	// Normalized version of Scurve6() that accepts
	// inputs from 0.0 to 1.0.
	//
	// Inputs:
	//		x = 0.0-1.0
	// Outputs:
	//		return = 0.0-1.0 along S-shaped curve
	//

	// doesn't appear the range is strictly -6 to 6; rather drops to near
	// zero past -6/6, at least for this sigmoid function

	//const double e = E_LN;

	// map input to -6 to + 6
	double mapx = Lerp(x, -6.0, 6.0);
	return SCurve6(mapx);
}

double SCurve6(double x)
{
	// Return point on S-Curve (Sigmoid function).
	//
	// Inputs:
	//		x = -6 to +6
	// Outputs:
	//		return = 0.0-1.0 along S-shaped curve
	//

	// doesn't appear the range is strictly -6 to 6; rather drops to near
	// zero past -6/6, at least for this sigmoid function

	const double e = E_LN;
	double sx = 0.0;

	// S(x) = (e^x) / (e^x + 1)
	double expr1 = pow(e, x);
	sx = expr1 / (expr1 + 1.0);

	return sx;
}

double RandomInRange(double min, double max)
{
	// Return random double between min-max.
	//

    assert(min < max);
    double n = (double) rand() / (double) RAND_MAX;
    double v = min + n * (max - min);
    return v;
}

double Median(std::vector<double> values)
{
	// Return median value.
	//
	// Note: values vector is modified.
	//
	
	std::sort(values.begin(), values.end());
	int count = (int)values.size();
	return values[count / 2];
}

