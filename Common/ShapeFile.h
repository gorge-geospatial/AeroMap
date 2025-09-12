#ifndef SHAPEFILE_H
#define SHAPEFILE_H

#include "MarkTypes.h"
#include "XString.h"
#include "Gis.h"

#include <vector>

class ShapeFile
{
public:

	enum class ShapeType
	{
		Null = 0,			// Null shape	None
		Point = 1,			// Point X, Y
		Polyline = 3,		// MBR, Number of parts, Number of points, Parts, Points
		Polygon = 5,		// MBR, Number of parts, Number of points, Parts, Points
		MultiPoint = 8,		// MBR, Number of points, Points
		PointZ = 11,		// X, Y, Z, M
		PolylineZ = 13,		// Mandatory: MBR, Number of parts, Number of points, Parts, Points, Z range, Z array
							// Optional: M range, M array
		PolygonZ = 15,		// Mandatory: MBR, Number of parts, Number of points, Parts, Points, Z range, Z array
							// Optional: M range, M array
		MultiPointZ = 18,	// Mandatory: MBR, Number of points, Points, Z range, Z array
							// Optional: M range, M array
		PointM = 21,		// X, Y, M
		PolylineM = 23,		// Mandatory: MBR, Number of parts, Number of points, Parts, Points
							// Optional: M range, M array
		PolygonM = 25,		// Mandatory: MBR, Number of parts, Number of points, Parts, Points
							// Optional: M range, M array
		MultiPointM = 28,	// Mandatory: MBR, Number of points, Points
							// Optional Fields: M range, M array
		MultiPatch = 31		// Mandatory: MBR, Number of parts, Number of points, Parts, Part types, Points, Z range, Z array
							// Optional: M range, M array
	};

	struct PointType
	{
		double X;
		double Y;
	};

	struct PointMType
	{
		double X;
		double Y;
		double M;
	};

public:

	ShapeFile(const char* fileName);
	virtual ~ShapeFile();

	int Save(const char* fileName);

	ShapeType GetShapeType() { return m_ShapeType; }

	UInt32 GetPointCount();
	ShapeFile::PointType GetPoint(UInt32 index);
	ShapeFile::PointMType GetPointM(UInt32 index);

	// terminology can be confusing here - "polygons" contain "parts", which are really
	// just other polygons; so to get a "point", you need to specify the polygon
	// and the part that contains the point; by convention, all indices are 0-based
	// relative to the containing object
	UInt32 GetPolygonCount() { return static_cast<UInt32>(mv_Polygon.size()); }
	UInt32 GetPartCount(UInt32 polyIndex);
	UInt32 GetPointCount(UInt32 polyIndex, UInt32 partIndex);
	ShapeFile::PointType GetPoint(UInt32 polyIndex, UInt32 partIndex, UInt32 index);

	virtual void GetExtents(double& yn, double& ys, double& xw, double& xe);
	virtual XString GetMetaData();

	GIS::GEODATA GetFileType() { return GIS::GEODATA::ShapeFile; }

private:

	struct MultiPointType
	{
		double Xmin;		// Bounding Box
		double Ymin;
		double Xmax;
		double Ymax;
		UInt32 NumPoints;								// Total Number of Points
		std::vector<ShapeFile::PointType> vPoints;		// all points for all parts
	};

	struct PolygonType
	{
		double Xmin;		// Bounding Box
		double Ymin;
		double Xmax;
		double Ymax;
		UInt32 NumParts;					// Number of Parts (polygons)
		UInt32 NumPoints;					// Total Number of Points
		std::vector<UInt32> vParts;			// offset to first point in vPoints[]
		std::vector<PointType> vPoints;		// all points for all parts
	};

	struct MultiPointMType
	{
		double Xmin;		// Bounding Box
		double Ymin;
		double Xmax;
		double Ymax;
		UInt32 NumPoints;								// number of points
		double Mmin;									// M value range
		double Mmax;
		std::vector<ShapeFile::PointMType> vPoints;		// all points
	};

	ShapeType m_ShapeType;					// file shape type - all records must be of this type

	std::vector<ShapeFile::PointType> mv_Point;		// data, shapetype == Point
	std::vector<MultiPointType> mv_MultiPoint;		// data, shapetype == MultiPoint
	//std::vector<Line> mv_Line;					// data, shapetype == Line
	std::vector<PolygonType> mv_Polygon;			// data, shapetype == Polygon
	std::vector<PointMType> mv_PointM;				// data, shapetype == PointM
	std::vector<MultiPointMType> mv_MultiPointM;	// data, shapetype == MultiPointM

protected:

	bool Load(const char* fileName);

private:

	const int HEADER_LEN = 100;				// size of fixed header, bytes

	void LoadPointData(int fh);
	void LoadMultiPointData(int fh);
	void LoadPolygonData(int fh, ShapeFile::ShapeType shapeType = ShapeFile::ShapeType::Polygon);
	void LoadPointMData(int fh);
	void LoadMultiPointMData(int fh);

	inline UInt32 ReadUInt32BE(int fh);		// read big-endian 32-bit uint 
	inline UInt32 ReadUInt32LE(int fh);		// read little-endian 32-bit uint
};

#endif // #ifndef SHAPEFILE_H
