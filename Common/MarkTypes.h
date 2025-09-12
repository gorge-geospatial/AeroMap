// MarkTypes.h
//
// Types common to all applications and platforms
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#ifndef MARKTYPES_H
#define MARKTYPES_H

#include <stdint.h>
#include <utility>

// primitive types of known sizes

typedef unsigned char 		UInt8;		// 8 bits - unsigned
typedef uint16_t	 		UInt16;		// 16 bits - unsigned
typedef uint32_t			UInt32;		// 32 bits - unsigned
typedef uint64_t			UInt64;		// 64 bits - unsigned

typedef char				Int8;		// 8 bits - signed
typedef int16_t				Int16;		// 16 bits - signed
typedef int32_t				Int32;		// 32 bits - signed
typedef int64_t				Int64;		// 64 bits - signed

enum class AXIS
{
	None,
	X,
	Y,
	Z
};

struct SizeType
{
	long cx;
	long cy;

	SizeType()
	{
		this->cx = 0;
		this->cy = 0;
	}
	SizeType(long cx, long cy)
	{
		this->cx = cx;
		this->cy = cy;
	}
};

struct SizeTypeD
{
	double cx;
	double cy;

	SizeTypeD()
	{
		this->cx = 0.0;
		this->cy = 0.0;
	}
	SizeTypeD(double cx, double cy)
	{
		this->cx = cx;
		this->cy = cy;
	}
};

struct PointType
{
	long x;
	long y;

	PointType()
	{
		this->x = 0;
		this->y = 0;
	}
	PointType(long x, long y)
	{
		this->x = x;
		this->y = y;
	}
};

struct PointI			// native int point type
{
	int x;
	int y;

	PointI()
	{
		this->x = 0;
		this->y = 0;
	}
	PointI(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
};

struct PointD		// 2d real point type
{
	double x;
	double y;

	PointD()
	{
		this->x = 0.0;
		this->y = 0.0;
	}
	PointD(double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	bool operator == (const PointD& pt) const
	{
		return ((x == pt.x) && (y == pt.y));
	}
};

struct RectType			// 2d integer rect/line type
{
	long x0;
	long y0;
	long x1;
	long y1;

	RectType()
	{
		this->x0 = 0;
		this->y0 = 0;
		this->x1 = 0;
		this->y1 = 0;
	}
	RectType(long x0, long y0, long x1, long y1)
	{
		this->x0 = x0;
		this->y0 = y0;
		this->x1 = x1;
		this->y1 = y1;
	}
	void Normalize()
	{
		if (x1 < x0)
			std::swap(x1, x0);
		if (y1 < y0)
			std::swap(y1, y0);
	}
	bool Intersects(RectType rect)
	{
		Normalize();
		rect.Normalize();

		// determine intersection by elimination
		if (this->x0 > rect.x1)		// this completely to right of rect
			return false;
		if (this->x1 < rect.x0)		// completely to left
			return false;
		if (this->y0 > rect.y1)		// completely below
			return false;
		if (this->y1 < rect.y0)		// completely above
			return false;

		// whether or not one box contains a corner of the other,
		// if we get here, they must overlap
		return true;
	}
	int Width()
	{
		return x1 - x0 + 1;
	}
	int Height()
	{
		return y1 - y0 + 1;
	}
};

struct RectD		// 2d real rect/line type
{
	double x0;
	double y0;
	double x1;
	double y1;

	RectD()
	{
		this->x0 = 0.0;
		this->y0 = 0.0;
		this->x1 = 0.0;
		this->y1 = 0.0;
	}
	RectD(double x0, double y0, double x1, double y1)
	{
		this->x0 = x0;
		this->y0 = y0;
		this->x1 = x1;
		this->y1 = y1;
	}
	bool Contains(double x, double y)
	{
		if ((x >= x0 && x <= x1) || (x >= x1 && x <= x0))
		{
			if ((y >= y0 && y <= y1) || (y >= y1 && y <= y0))
			{
				return true;
			}
		}
		return false;
	}
	void Normalize()
	{
		// ensure points grow from 0 to 1
		if (this->x0 > this->x1)
			std::swap(this->x0, this->x1);
		if (this->y0 > this->y1)
			std::swap(this->y0, this->y1);
	}
	void Grow(double delta)
	{
		// grow equally in all directions
		Normalize();

		x0 -= delta;
		x1 += delta;
		y0 -= delta;
		y1 += delta;
	}
	double DX() { return x1 - x0; }
	double DY() { return y1 - y0; }
};

#endif // #ifndef MARKTYPES_H
