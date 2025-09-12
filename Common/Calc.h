// Calc.h
// Interface to extended math library.
//
// Contents:
//		#defines
//		types
//		vector/matrix interfaces
//		vector/matrix support functions
//		vector/matrix implementations
//

#ifndef CALC_H
#define CALC_H

#include <string.h>
#include <math.h>			// sincos() & others

#include <vector>

#include "MarkTypes.h"		// global type definitions

const double PI = 3.1415926535897932384626433832795;		// double precision PI value
const double E_LN = 2.7182818284590452353602874713527;		// double precision natural log base (e)

const double FEET_PER_METER = 3.2808399;
const double SQUARE_FEET_PER_METER = 10.76391045;

// international foot = 0.3048m
// us survey foot = 1200/3937m

const double GRAVITY_ACCEL = 9.806;				// gravitational acceleration m/s^2
const double NEWTONS_PER_POUND = 4.4482216;		// force equivalent of 1 pound, Newtons (kg-m/s^2)
const double NEWTONS_PER_KG = 9.8066500;		// at surface of Earth
const double POUNDS_PER_KG = 2.205;      		// weight of KG at sea level
const double SLUGS_PER_KG = 0.0685;
const double AIR_DENSITY_KG_M3 = 1.225;     	// air density at sea level, standard day (kg/m^3)

// RGB & RGBA use same 32-bit format (AABBGGRR) so GetX() returns the correct byte regardless of how color was packed
inline UInt32 PackRGBA(UInt8 r, UInt8 g, UInt8 b, UInt8 a)	{ return ((UInt32)(r) | (UInt32)(g) << 8 | (UInt32)(b) << 16 | (UInt32)(a) << 24); }
inline UInt32 PackRGB(UInt8 r, UInt8 g, UInt8 b)			{ return ((UInt32)(r) | (UInt32)(g) << 8 | (UInt32)(b) << 16); }
inline UInt8  GetR(UInt32 RGBA) { return (UInt8)(RGBA); }
inline UInt8  GetG(UInt32 RGBA) { return (UInt8)(RGBA >> 8); }
inline UInt8  GetB(UInt32 RGBA) { return (UInt8)(RGBA >> 16); }
inline UInt8  GetA(UInt32 RGBA) { return (UInt8)(RGBA >> 24); }

// general conversions

inline double MetersToFeet(double p) { return p * FEET_PER_METER; }
inline double FeetToMeters(double p) { return p / FEET_PER_METER; }
inline double SquareFeetToMeters(double p) { return p/SQUARE_FEET_PER_METER; }
inline double PoundsToKilos(double p) { return p / POUNDS_PER_KG; }
inline double KilosToPounds(double p) { return p * POUNDS_PER_KG; }
inline double PoundsToNewtons(double p) { return p * NEWTONS_PER_POUND; }
inline double NewtonsToPounds(double p) { return p / NEWTONS_PER_POUND; }
inline double NewtonsToKilos(double newtons) { return newtons / NEWTONS_PER_KG; }
inline double KilosToNewtons(double kilograms) { return kilograms * NEWTONS_PER_KG; }
inline double MPHToFPS(double p) { return (p*5280.0) / 3600.0; }	// MPH -> feet per second
inline double MPSToMPH(double p) { return p*2.2369362920544; }      // meters/second -> miles/hour

inline double DegreeToRadian(double p) { return p * 0.0055555555555555555555555555555556 * PI; }
inline double RadianToDegree(double p) { return p * 0.31830988618379067153776752674503 * 180.0; }

// "+1" is to round back up from the other conversion that truncates
inline UInt8  RadianToBrad(double p) { return static_cast<UInt8>(p * 0.31830988618379067153776752674503 * (256.0+1.0)); }
inline double BradToRadian(UInt8 p) { return static_cast<double>(p) * 0.00390625 * PI; }
 
inline void InitQuad(float* p4, float r, float g, float b, float a = 1.0) { p4[0] = r; p4[1] = g; p4[2] = b; p4[3] = a; }
inline void InitQuad(double* p4, double r, double g, double b, double a = 1.0) { p4[0] = r; p4[1] = g; p4[2] = b; p4[3] = a; }

double RandomInRange(double min, double max);

struct PixelType
{
	UInt8 R;
	UInt8 G;
	UInt8 B;
	UInt8 A;

	PixelType()
	{
		// default constructor
		R = G = B = 0;
		A = 255;
	}
	PixelType(UInt8 InitR, UInt8 InitG, UInt8 InitB, UInt8 InitA = 255)
	{
		// constructor
		R = InitR;
		G = InitG;
		B = InitB;
		A = InitA;
	}
	PixelType& operator = (const PixelType& p)
	{
		// make sure not same object
		if (this != &p)
		{
			R = p.R;
			G = p.G;
			B = p.B;
			A = p.A;
		}
		return *this;
	}
	bool CompareRGB(const PixelType& p)
	{
		return ((R == p.R) && (G == p.G) && (B == p.B));
	}
	UInt32 GetRGB()
	{
		return PackRGB(R, G, B);
	}
	void SetR(float r)
	{
		R = (UInt8)(r*255.0);
	}
	void SetG(float g)
	{
		G = (UInt8)(g*255.0);
	}
	void SetB(float b)
	{
		B = (UInt8)(b*255.0);
	}
	void SetA(float a)
	{
		A = (UInt8)(a*255.0);
	}
	void SetRGBA(UInt8 r, UInt8 g, UInt8 b, UInt8 a = 255)
	{
		R = r;
		G = g;
		B = b;
		A = a;
	}
	void SetRGBA(double r, double g, double b, double a = 1.0)
	{
		R = static_cast<UInt8>(r*255.0);
		G = static_cast<UInt8>(g*255.0);
		B = static_cast<UInt8>(b*255.0);
		A = static_cast<UInt8>(a*255.0);
	}
	UInt32 GetRGBA()
	{
		return PackRGBA(R, G, B, A);
	}
	UInt8 GetGray()
	{
		// get grayscale representation of pixel
		return (R + G + B) / 3;
	}
	float GetR() const
	{
		// return red channel as scale factor from 0.0-1.0
		return (float)R * 0.003921568627451F;
	}
	float GetG() const
	{
		// return green channel as scale factor from 0.0-1.0
		return (float)G * 0.003921568627451F;
	}
	float GetB() const
	{
		// return blue channel as scale factor from 0.0-1.0
		return (float)B * 0.003921568627451F;
	}
	float GetA() const
	{
		// return alpha channel as scale factor from 0.0-1.0
		return (float)A * 0.003921568627451F;
	}
	void SetGraySF(float sf)
	{
		// set grayscale using scale factor

		if (sf < 0.0)
			sf = 0.0;
		if (sf > 1.0)
			sf = 1.0;

		R = G = B = static_cast<UInt8>(sf*255.0);
	}
	bool IsBlack()
	{
		return (R == 0 && G == 0 && B == 0);
	}
	bool IsWhite()
	{
		return (R == 255 && G == 255 && B == 255);
	}

	PixelType Lerp(double t, PixelType pix2)
	{
		// linearly interpolate between this pixel and pixel parameter
		//
		// inputs:
		//		t = 0.0 for "this" pixel, 1.0 for "pix2"
		//		pix2 = 2nd interpolation pixel
		//

		PixelType pix;

		pix.R = (UInt8)((double)this->R + t*((double)pix2.R - (double)this->R));			// interpolate components
		pix.G = (UInt8)((double)this->G + t*((double)pix2.G - (double)this->G));
		pix.B = (UInt8)((double)this->B + t*((double)pix2.B - (double)this->B));

		return pix;		// return result
	}
};

// vector/matrix support functions

typedef struct VEC2		// 2D Vector
{
public:
	VEC2();
	VEC2(const double *);
	VEC2(double x, double y);

	// casting
	operator double* ();
	operator const double* () const;

	// assignment operators
	VEC2& operator += (const VEC2&);
	VEC2& operator -= (const VEC2&);
	VEC2& operator *= (double);
	VEC2& operator /= (double);

	// unary operators
	VEC2 operator + () const;
	VEC2 operator - () const;

	// binary operators
	VEC2 operator + (const VEC2&) const;
	VEC2 operator - (const VEC2&) const;
	VEC2 operator * (double) const;
	VEC2 operator / (double) const;

	bool operator == (const VEC2&) const;
	bool operator != (const VEC2&) const;

	// methods
	double Magnitude() const;			// vector magnitude
	double MagnitudeSquared() const;	// vector magnitute squared

public:
	double x, y;
} VEC2;

typedef struct VEC3		// 3D Vector
{
public:
	VEC3();
	VEC3(const double *);
	VEC3(double x, double y, double z);

	// casting
	operator double* ();
	operator const double* () const;

	// assignment operators
	VEC3& operator += (const VEC3&);
	VEC3& operator -= (const VEC3&);
	VEC3& operator *= (double);
	VEC3& operator /= (double);

	// unary operators
	VEC3 operator + () const;
	VEC3 operator - () const;

	// binary operators
	VEC3 operator + (const VEC3&) const;
	VEC3 operator - (const VEC3&) const;
	VEC3 operator * (double) const;
	VEC3 operator / (double) const;

	bool operator == (const VEC3&) const;
	bool operator != (const VEC3&) const;

	// methods
	double Magnitude() const;			// vector magnitude
	double MagnitudeSquared() const;	// vector magnitute squared
	VEC3 Unit() const;

public:
	double x, y, z;
} VEC3;

typedef struct VEC4		// 4D Vector
{
public:
	VEC4();
	VEC4(const double*);
	VEC4(double x, double y, double z, double w);

	// casting
	operator double* ();
	operator const double* () const;

	// assignment operators
	VEC4& operator += (const VEC4&);
	VEC4& operator -= (const VEC4&);
	VEC4& operator *= (double);
	VEC4& operator /= (double);

	// unary operators
	VEC4 operator + () const;
	VEC4 operator - () const;

	// binary operators
	VEC4 operator + (const VEC4&) const;
	VEC4 operator - (const VEC4&) const;
	VEC4 operator * (double) const;
	VEC4 operator / (double) const;

	bool operator == (const VEC4&) const;
	bool operator != (const VEC4&) const;

public:
	double x, y, z, w;
} VEC4;

// orientation types

struct ORIENT_HPR		// heading/pitch/roll orientation descriptor
{
	float Heading;
	float Pitch;
	float Roll;
};

struct ORIENT_VEC		// vector based orientation descriptor
{
	VEC3 Dir;
	VEC3 Up;
};

typedef struct MAT3		// 3x3 matrices
{
public:
	MAT3();
	MAT3(const double *);
	MAT3(double _11, double _12, double _13,
		 double _21, double _22, double _23,
		 double _31, double _32, double _33);

	// access grants
	double& operator () (int row, int col);
	double  operator () (int row, int col) const;

	// casting operators
	operator double* ();
	operator const double* () const;

	// assignment operators
	MAT3& operator *= (const MAT3&);
	MAT3& operator += (const MAT3&);
	MAT3& operator -= (const MAT3&);
	MAT3& operator *= (double);
	MAT3& operator /= (double);

	// unary operators
	MAT3 operator + () const;
	MAT3 operator - () const;

	// binary operators
	MAT3 operator * (const MAT3&) const;
	MAT3 operator + (const MAT3&) const;
	MAT3 operator - (const MAT3&) const;
	MAT3 operator * (double) const;
	MAT3 operator / (double) const;

	bool operator == (const MAT3&) const;
	bool operator != (const MAT3&) const;

	void SetIdentity();
	bool IsIdentity() const;

public:
	double m[3][3];

} MAT3;

typedef struct MAT4		// 4x4 matrices
{
public:
    MAT4();
    MAT4( const double * );
    MAT4( double _11, double _12, double _13, double _14,
          double _21, double _22, double _23, double _24,
          double _31, double _32, double _33, double _34,
          double _41, double _42, double _43, double _44 );

	// access grants
	double& operator () (int row, int col);
	double  operator () (int row, int col) const;

    // casting operators
    operator double* ();
    operator const double* () const;

    // assignment operators
    MAT4& operator *= ( const MAT4& );
    MAT4& operator += ( const MAT4& );
    MAT4& operator -= ( const MAT4& );
    MAT4& operator *= ( double );
    MAT4& operator /= ( double );

    // unary operators
    MAT4 operator + () const;
    MAT4 operator - () const;

    // binary operators
    MAT4 operator * ( const MAT4& ) const;
    MAT4 operator + ( const MAT4& ) const;
    MAT4 operator - ( const MAT4& ) const;
    MAT4 operator * ( double ) const;
    MAT4 operator / ( double ) const;

    bool operator == ( const MAT4& ) const;
    bool operator != ( const MAT4& ) const;

	bool IsIdentity() const;

public:
    double m[4][4];

} MAT4;

// define a plane using normal and
// point on the plane
struct PlaneNP
{
	VEC3 N;
	VEC3 P;

	void SetNormalAndPoint(const VEC3 N, const VEC3 P)
	{
		this->N = N;
		this->P = P;
	}
};

// define a plane using 3 points
struct PlaneP
{
	VEC3 P0;
	VEC3 P1;
	VEC3 P2;
};

// curve functions

std::vector<double> CubicSpline(double* a, double* knot, int samples);
void	BezierCurve(PointD* pControlPoly, int controlCount, PointD* pCurve, int curveCount);
void	BezierCurve(std::vector<PointD> controlPoly, std::vector<PointD>& curve, int curveCount);
double	SCurve(double x);
double	SCurve6(double x);

void	Bresenham(int x0, int y0, int const x1, int const y1, std::vector<PointType>& ptList);
double  CalcAngleDeg(VEC3 v1, VEC3 v2);
double  CalcAngleRad(VEC3 v1, VEC3 v2);
double	CalcBearing(const PointD pt0, const PointD pt1);

double	CalcYIntercept(double x, double x1, double y1, double x2, double y2);
double	Clamp(double value, double minVal, double maxVal);
float	Clamp(float value, float minVal, float maxVal);
int		Clamp(int value, int minVal, int maxVal);
VEC3    CrossProduct(const VEC3& v1, const VEC3& v2);
double  CrossProduct(const VEC2& v1, const VEC2 v2);
VEC3    DegreesToVector(double headingRad, double pitchRad, double rollRad);

double  Distance(VEC3 vPos1, VEC3 vPos2);
double  Distance(const VEC2 vPos1, const VEC2 vPos2);
double  Distance(PointD pt0, PointD pt1);
double  Distance(double x0, double y0, double x1, double y1);
double	DistancePointToLine2D(PointD point, RectD line);
double	DistancePointToPlane(const VEC3 vTestPoint, const VEC3 vPlanePoint, const VEC3 vPlaneNormal);
double  DistancePointToPlane(const VEC3 vTestPoint, const PlaneNP plane);
double	DistancePointToPoly(double x, double y, const std::vector<VEC2> vxList);

double  DotProduct(const VEC3& v1, const VEC3& v2);
double  DotProduct(const VEC2& v1, const VEC2& v2);
UInt32	Factorial(UInt32 value);

bool	IntersectLineLine2D(RectD testLine1, RectD testLine2, PointD& xsect);
double	IntersectLinePerpPoint(VEC3 P0, VEC3 P1, VEC3 P2);
bool	IntersectLineRect2D(RectD rect, RectD line, RectD* pClippedLine);
int		IntersectLineTriangle(VEC3 S0, VEC3 S1, VEC3 V0, VEC3 V1, VEC3 V2, VEC3* I);
bool	IntersectPolyPoly2D(const std::vector<VEC2> vxPoly1, const std::vector<VEC2> vxPoly2);
int		IntersectRayPlane(VEC3 rayOrg, VEC3 rayDir, VEC3 V0, VEC3 V1, VEC3 V2, VEC3* I);
int		IntersectRayTriangle(VEC3 rayOrg, VEC3 rayDir, VEC3 V0, VEC3 V1, VEC3 V2, VEC3* I);
bool	IntersectRectRect(RectD& rect0, RectD& rect1);
bool	IntersectTriangleTriangle(VEC3 V0, VEC3 V1, VEC3 V2, VEC3 U0, VEC3 U1, VEC3 U2, VEC3* I0, VEC3* I1);

bool    IsBetween(double val, double loVal, double hiVal);
double  LeastSquares(std::vector<PointD> pts, double& b, double& m, bool bInvert = false);

float   Lerp(float t, float a, float b);
double  Lerp(double t, double a, double b);
UInt8   Lerp(float t, UInt8 a, UInt8 b);
float   LerpLN(float t, float min, float max);

double  Magnitude(const VEC3& v);

void    MatrixIdentity(MAT4* mat);
void    MatrixMultiply(MAT3* pMat, const MAT3* pMat1, const MAT3* pMat2);
void    MatrixMultiply(MAT4* pMat, const MAT4* pMat1, const MAT4* pMat2);
void    MatrixRotationAxis(MAT3* pMat, const VEC3* pvAxis, double theta);
void    MatrixRotationYawPitchRoll(MAT4* mat, double yaw, double pitch, double roll);

double  Median(std::vector<double> values);

double	ModGTE(double value, double divisor);
double	ModLTE(double value, double divisor);

VEC3    Normalize(const VEC3& v);
VEC3    Normalize(const double x, const double y, const double z);

bool	PointInPoly(double x, double y, const std::vector<VEC2> vxList);
bool	PointInRect(double testX, double testY, RectD rct);

void	Rotate2d(double thetaRad, double x, double y, double* x1, double* y1, double cx = 0.0, double cy = 0.0);
void	RotateX(double theta, VEC3& vVec);
void	RotateY(double theta, VEC3& vVec);
void	RotateZ(double theta, VEC3& vVec);

int     sign(double val);
double  SquareMagnitude(const VEC3& v);
void    swap(float& f1, float& f2);
void    swap(double& f1, double& f2);
void    swap(int& i1, int& i2);

void	UpHeadingToPitchRoll(VEC3 vUp, double heading, double& pitch, double& roll);
void	UpHeadingToUpDir(VEC3 vUp, double heading, VEC3* pvDir);

VEC3	Vec3Transform(const VEC3 vec_in, const MAT3 mat_in);

#endif // #ifndef CALC_H
