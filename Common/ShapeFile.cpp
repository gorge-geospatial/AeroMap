// ShapeFile.cpp
// Manager for ESRI ShapeFiles
//
// Wrapper for GDAL access.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include <io.h>

#define open _open
#define lseek _lseek
#define read _read
#define write _write
#define close _close
#define chmod _chmod
#define eof _eof

#include "gdal_priv.h"
#include "cpl_conv.h"		// for CPLMalloc()
#include "ogrsf_frmts.h"

#include "ShapeFile.h"
#include "Logger.h"

constexpr char* DRIVER_NAME = "ESRI ShapeFile";		// GDAL driver name

ShapeFile::ShapeFile(const char* fileName)
{
	// accept "shapefile" or "shapefile.shp"

	XString shpFile = fileName;
	if (!shpFile.EndsWithNoCase(".shp"))
		shpFile = XString::CombinePath(shpFile.c_str(), ".shp");

	Load(shpFile.c_str());
}

ShapeFile::~ShapeFile()
{
}

bool ShapeFile::Load(const char* fileName)
{
	int inputFile = open(fileName, O_RDONLY | O_BINARY);
	if (inputFile == -1)
	{
		Logger::Write(__FUNCTION__, "Unable to open file '%s'.", fileName);
		return false;
	}

	UInt32 fileCode = ReadUInt32BE(inputFile);
	if (fileCode != 0x0000270a)
	{
		Logger::Write(__FUNCTION__, "Invalid file code. Expected 0x0000270A, read 0x%08X", fileCode);
	}

	int bytes = 0;

	// 5x unused 32-bit vals
	ReadUInt32BE(inputFile);
	ReadUInt32BE(inputFile);
	ReadUInt32BE(inputFile);
	ReadUInt32BE(inputFile);
	ReadUInt32BE(inputFile);

	// file length, in 16-bit words including header
	/* UInt32 fileLengthWords = */ ReadUInt32BE(inputFile);

	// version
	/* UInt32 version = */ ReadUInt32LE(inputFile);

	// shape type
	m_ShapeType = static_cast<ShapeType>(ReadUInt32LE(inputFile));

	// minimum bounding rectangle
	double minX = 0.0;
	double minY = 0.0;
	double maxX = 0.0;
	double maxY = 0.0;
	bytes = read(inputFile, &minX, sizeof(minX));
	bytes = read(inputFile, &minY, sizeof(minY));
	bytes = read(inputFile, &maxX, sizeof(maxX));
	bytes = read(inputFile, &maxY, sizeof(maxY));

	// range of Z
	double minZ = 0.0;
	double maxZ = 0.0;
	bytes = read(inputFile, &minZ, sizeof(minZ));
	bytes = read(inputFile, &maxZ, sizeof(maxZ));

	// range of M
	double minM = 0.0;
	double maxM = 0.0;
	bytes = read(inputFile, &minM, sizeof(minM));
	bytes = read(inputFile, &maxM, sizeof(maxM));

	// assert we read the expected 100 byte header
	long pos = lseek(inputFile, 0, SEEK_CUR);
	if (pos != HEADER_LEN)
		Logger::Write(__FUNCTION__, "lseek() returned %l, expected %l", pos, HEADER_LEN);

	switch (m_ShapeType) {
	case ShapeType::Point:
		LoadPointData(inputFile);
		break;
	case ShapeType::Polyline:
		LoadPolygonData(inputFile, ShapeType::Polyline);
		break;
	case ShapeType::Polygon:
		LoadPolygonData(inputFile);
		break;
	case ShapeType::MultiPoint:
		LoadMultiPointData(inputFile);
		break;
	case ShapeType::PointZ:
		//LoadPointZData( inputFile );
		break;
	case ShapeType::PolylineZ:
	case ShapeType::PolygonZ:
	case ShapeType::MultiPointZ:
	case ShapeType::PointM:
		LoadPointMData(inputFile);
		break;
	case ShapeType::PolylineM:
	case ShapeType::PolygonM:
	case ShapeType::MultiPointM:
	case ShapeType::MultiPatch:
	default:
		// unhandled shape type
		assert(false);
	}

	close(inputFile);

	return true;
}

void ShapeFile::LoadPointData(int fh)
{
	// load data for ShapeType == Point file

	// should be at end of header
	long pos = lseek(fh, 0, SEEK_CUR);
	if (pos != HEADER_LEN)
		Logger::Write(__FUNCTION__, "lseek() returned %l, expected %l", pos, HEADER_LEN);

	mv_Point.clear();
	while (!eof(fh))
	{
		ShapeFile::PointType pt;		// build 1 of these for each data record

		/* UInt32 recNum = */ ReadUInt32BE(fh);		// 1-based
		/* UInt32 recLen = */ ReadUInt32BE(fh);		// in 16-bit words

		// redundant shape type
		ShapeType curShapeType;
		int bytesRead = read(fh, &curShapeType, 4);
		assert(curShapeType == ShapeType::Point || curShapeType == ShapeType::Null);

		// point
		bytesRead = read(fh, &pt.X, 8);
		bytesRead = read(fh, &pt.Y, 8);

		// add point to class level point list
		mv_Point.push_back(pt);
	}
}

void ShapeFile::LoadMultiPointData(int fh)
{
	// load data for ShapeType == MultiPoint file

	// should be at end of header
	long pos = lseek(fh, 0, SEEK_CUR);
	if (pos != HEADER_LEN)
		Logger::Write(__FUNCTION__, "lseek() returned %l, expected %l", pos, HEADER_LEN);

	mv_MultiPoint.clear();
	while (!eof(fh))
	{
		MultiPointType mpt;		// build 1 of these for each data record

		/* UInt32 recNum = */ ReadUInt32BE(fh);		// 1-based
		/* UInt32 recLen = */ ReadUInt32BE(fh);		// in 16-bit words

		// redundant shape type
		ShapeType curShapeType;
		int bytesRead = read(fh, &curShapeType, 4);
		assert(curShapeType == ShapeType::MultiPoint || curShapeType == ShapeType::Null);

		// bounding box
		bytesRead = read(fh, &mpt.Xmin, 8);
		bytesRead = read(fh, &mpt.Ymin, 8);
		bytesRead = read(fh, &mpt.Xmax, 8);
		bytesRead = read(fh, &mpt.Ymax, 8);

		// total number of points 
		bytesRead = read(fh, &mpt.NumPoints, 4);

		// read array of points
		for (unsigned int i = 0; i < mpt.NumPoints; ++i)
		{
			bytesRead = read(fh, &mpt.vPoints[i].X, 8);
			bytesRead = read(fh, &mpt.vPoints[i].Y, 8);
		}

		// add multipoint to class level multipoint list
		mv_MultiPoint.push_back(mpt);
	}
}

void ShapeFile::LoadPolygonData(int fh, ShapeFile::ShapeType shapeType)
{
	// Load data for ShapeType == [Polygon,PolyLine] file.
	//
	// Inputs:
	//		fh			= file handle
	//		shapeType	= Polygon or PolyLine - they both have the same format
	// Outputs:
	//		mv_Polygon is populated
	//

	// only applies to these shapes
	if ((shapeType != ShapeType::Polygon) && (shapeType != ShapeType::Polyline))
	{
		Logger::Write(__FUNCTION__, "Invalid shape type specified: %d", shapeType);
		assert(false);
		return;
	}

	// should be at end of header
	long pos = lseek(fh, 0, SEEK_CUR);
	if (pos != HEADER_LEN)
		Logger::Write(__FUNCTION__, "lseek() returned %l, expected %l", pos, HEADER_LEN);

	while (!eof(fh))
	{
		PolygonType poly;		// build 1 of these for each data record

		/*UInt32 recNum =*/ ReadUInt32BE(fh);		// 1-based
		/*UInt32 recLen =*/ ReadUInt32BE(fh);		// in 16-bit words

		// redundant shape type
		ShapeType curShapeType;
		int bytesRead = read(fh, &curShapeType, 4);
		assert(curShapeType == shapeType || curShapeType == ShapeType::Null);

		// bounding box
		bytesRead = read(fh, &poly.Xmin, 8);
		bytesRead = read(fh, &poly.Ymin, 8);
		bytesRead = read(fh, &poly.Xmax, 8);
		bytesRead = read(fh, &poly.Ymax, 8);

		// number of parts - a "part" is a closed polygon
		bytesRead = read(fh, &poly.NumParts, 4);
		// total number of points - all points in all polygons
		bytesRead = read(fh, &poly.NumPoints, 4);

		poly.vParts.resize(poly.NumParts);
		poly.vPoints.resize(poly.NumPoints);

		// read array of parts
		for (unsigned int i = 0; i < poly.NumParts; ++i)
		{
			bytesRead = read(fh, &poly.vParts[i], 4);
		}

		// read array of points
		for (unsigned int i = 0; i < poly.NumPoints; ++i)
		{
			bytesRead = read(fh, &poly.vPoints[i].X, 8);
			bytesRead = read(fh, &poly.vPoints[i].Y, 8);
		}

		// add polygon (which may contain multiple "rings" - which are also polygons)
		// to class level polygon list
		mv_Polygon.push_back(poly);
	}
}

void ShapeFile::LoadPointMData(int fh)
{
	// load data for ShapeType == PointM file

	// should  be at end of header
	long pos = lseek(fh, 0, SEEK_CUR);
	if (pos != HEADER_LEN)
		Logger::Write(__FUNCTION__, "lseek() returned %l, expected %l", pos, HEADER_LEN);

	mv_Point.clear();
	while (!eof(fh))
	{
		PointMType pt;		// build 1 of these for each data record

		/*UInt32 recNum =*/ ReadUInt32BE(fh);		// 1-based
		/*UInt32 recLen =*/ ReadUInt32BE(fh);		// in 16-bit words

		// redundant shape type
		ShapeType curShapeType;
		int bytesRead = read(fh, &curShapeType, 4);
		assert(curShapeType == ShapeType::Point || curShapeType == ShapeType::Null);

		// point
		bytesRead = read(fh, &pt.X, 8);
		bytesRead = read(fh, &pt.Y, 8);
		// note: M could have "no data" - any floating point number smaller than –10^38
		bytesRead = read(fh, &pt.M, 8);

		// add point to class level point list
		mv_PointM.push_back(pt);
	}
}

void ShapeFile::LoadMultiPointMData(int fh)
{
	// load data for ShapeType == MultiPoint file

	// should be at end of header
	long pos = lseek(fh, 0, SEEK_CUR);
	if (pos != HEADER_LEN)
		Logger::Write(__FUNCTION__, "lseek() returned %l, expected %l", pos, HEADER_LEN);

	mv_MultiPoint.clear();
	while (!eof(fh))
	{
		MultiPointMType mpt;		// build 1 of these for each data record

		/* UInt32 recNum = */ ReadUInt32BE(fh);		// 1-based
		/* UInt32 recLen = */ ReadUInt32BE(fh);		// in 16-bit words

		// redundant shape type
		ShapeType curShapeType;
		int bytesRead = read(fh, &curShapeType, 4);
		assert(curShapeType == ShapeType::MultiPoint || curShapeType == ShapeType::Null);

		// bounding box
		bytesRead = read(fh, &mpt.Xmin, 8);
		bytesRead = read(fh, &mpt.Ymin, 8);
		bytesRead = read(fh, &mpt.Xmax, 8);
		bytesRead = read(fh, &mpt.Ymax, 8);

		// total number of points 
		bytesRead = read(fh, &mpt.NumPoints, 4);

		// read array of points
		for (unsigned int i = 0; i < mpt.NumPoints; ++i)
		{
			bytesRead = read(fh, &mpt.vPoints[i].X, 8);
			bytesRead = read(fh, &mpt.vPoints[i].Y, 8);
		}

		// min/max M values
		bytesRead = read(fh, &mpt.Xmin, 8);
		bytesRead = read(fh, &mpt.Ymin, 8);

		// read array of M values
		for (unsigned int i = 0; i < mpt.NumPoints; ++i)
		{
			bytesRead = read(fh, &mpt.vPoints[i].M, 8);
		}

		// add multipoint to class level multipoint list
		mv_MultiPointM.push_back(mpt);
	}
}

UInt32 ShapeFile::ReadUInt32BE(int fh)
{
	// read big-endian unsigned 32-bit integer

	UInt32 val = 0;
	UInt8 b[4] = { 0, 0, 0, 0 };

	int bytes = read(fh, b, 4);
	if (bytes != 4)
		Logger::Write(__FUNCTION__, "read() returned %d, expected %d", bytes, 4);

	val = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];

	return val;
}

UInt32 ShapeFile::ReadUInt32LE(int fh)
{
	// read little-endian unsigned 32-bit integer

	UInt32 val = 0;
	int bytes = read(fh, &val, 4);
	if (bytes != 4)
		Logger::Write(__FUNCTION__, "read() returned %d, expected %d", bytes, 4);

	return val;
}

int ShapeFile::Save(const char* fileName)
{
	// this is just a copy of the write test - needs much enhancement
	// to properly write loaded file

	GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(DRIVER_NAME));
	if (pDriver == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to load driver: %s", DRIVER_NAME);
		return 1;
	}

	if (pDriver != nullptr)
	{
		// Create the datasource. The ESRI Shapefile driver allows us to create a directory full of shapefiles, 
		// or a single shapefile as a datasource. In this case we will explicitly create a single file by including
		// the extension in the name. The second, third, fourth and fifth argument are related to raster dimensions.
		// The last argument to the call is a list of option values, but we will just be using defaults in this case.

		GDALDataset* pDS = pDriver->Create(fileName, 0, 0, 0, GDT_Unknown, nullptr);
		if (pDS == nullptr)
		{
			Logger::Write(__FUNCTION__, "Unable to create data source: %s", fileName);
			return 1;
		}

		if (pDS != nullptr)
		{
			// Now we create the output layer. In this case since the datasource is a single file, 
			// we can only have one layer. We pass wkbPoint to specify the type of geometry supported 
			// by this layer. In this case we aren’t passing any coordinate system information or other
			// special layer creation options.

			const char* layerName = "point_out";
			OGRLayer* pLayer = pDS->CreateLayer(layerName, nullptr, wkbPoint, nullptr);
			if (pLayer == nullptr)
			{
				Logger::Write(__FUNCTION__, "Unable to create output layer: %s", layerName);
				return 1;
			}

			// Now that the layer exists, we need to create any attribute fields that should appear on the layer.
			// Fields must be added to the layer before any features are written. To create a field we initialize
			// an OGRField object with the information about the field. In the case of Shapefiles, the field width
			// and precision is significant in the creation of the output .dbf file, so we set it specifically, 
			// though generally the defaults are OK. For this example we will just have one attribute, a name string
			// associated with the x,y point.
			//
			// Note that the template OGRField we pass to OGRLayer::CreateField() is copied internally. We retain ownership of the object.

			OGRFieldDefn oField("Name", OFTString);
			oField.SetWidth(32);
			if (pLayer->CreateField(&oField) != OGRERR_NONE)
			{
				// To write a feature to disk, we must create a local OGRFeature, set attributes and attach geometry
				// before trying to write it to the layer. It is imperative that this feature be instantiated from 
				// the OGRFeatureDefn associated with the layer it will be written to.

				OGRFeature* pFeature = OGRFeature::CreateFeature(pLayer->GetLayerDefn());
				pFeature->SetField("Name", "TestPoint1");

				// We create a local geometry object, and assign its copy (indirectly) to the feature. The OGRFeature::SetGeometryDirectly()
				// differs from OGRFeature::SetGeometry() in that the direct method gives ownership of the geometry to the feature. This is 
				// generally more efficient as it avoids an extra deep object copy of the geometry.

				OGRPoint pt;
				pt.setX(101.0);
				pt.setY(201.0);

				pFeature->SetGeometry(&pt);

				OGRErr err = pLayer->CreateFeature(pFeature);
				if (err != OGRERR_NONE)
				{
					Logger::Write(__FUNCTION__, "Unable to create feature, error code: %d", err);
					return 1;
				}

				OGRFeature::DestroyFeature(pFeature);
			}

			GDALClose(pDS);
		}
	}

	return 0;
}

UInt32 ShapeFile::GetPointCount()
{
	// Return point count for any of the point
	// type shape files.
	//

	UInt32 pointCount = 0;

	switch (m_ShapeType) {
	case ShapeType::Point:
		pointCount = static_cast<UInt32>(mv_Point.size());
		break;
	case ShapeType::PointM:
		pointCount = static_cast<UInt32>(mv_PointM.size());
		break;
	default:
		assert(false);
	}

	return pointCount;
}

ShapeFile::PointType ShapeFile::GetPoint(UInt32 index)
{
	// return point
	//

	assert(m_ShapeType == ShapeType::Point);

	ShapeFile::PointType pt;
	if (index < mv_Point.size())
	{
		pt = mv_Point[index];
	}
	return pt;
}

ShapeFile::PointMType ShapeFile::GetPointM(UInt32 index)
{
	// return point
	//

	assert(m_ShapeType == ShapeType::PointM);

	ShapeFile::PointMType pt;
	if (index < mv_PointM.size())
	{
		pt = mv_PointM[index];
	}
	return pt;
}

UInt32 ShapeFile::GetPartCount(UInt32 polyIndex)
{
	// return the # of "parts" (polygons) within the
	// specified root polygon
	//

	assert(m_ShapeType == ShapeType::Polygon || m_ShapeType == ShapeType::Polyline);

	UInt32 partCount = 0;
	if (polyIndex < static_cast<UInt32>(mv_Polygon.size()))
		partCount = mv_Polygon[polyIndex].NumParts;

	return partCount;
}

UInt32 ShapeFile::GetPointCount(UInt32 polyIndex, UInt32 partIndex)
{
	// return the # of points within the specifed polygon/part
	//

	assert(m_ShapeType == ShapeType::Polygon);

	UInt32 pointCount = 0;
	if (polyIndex < static_cast<UInt32>(mv_Polygon.size()))
	{
		if (partIndex < static_cast<UInt32>(mv_Polygon[polyIndex].vParts.size()))
		{
			// offset to first point in this part
			UInt32 offset0 = mv_Polygon[polyIndex].vParts[partIndex];
			UInt32 offset1;
			if (partIndex < static_cast<UInt32>(mv_Polygon[polyIndex].vParts.size() - 1))
			{
				offset1 = mv_Polygon[polyIndex].vParts[partIndex + 1];
			}
			else
			{
				offset1 = static_cast<UInt32>(mv_Polygon[polyIndex].vPoints.size());
			}
			pointCount = offset1 - offset0;
		}
	}

	return pointCount;
}

ShapeFile::PointType ShapeFile::GetPoint(UInt32 polyIndex, UInt32 partIndex, UInt32 index)
{
	assert(m_ShapeType == ShapeType::Polygon || m_ShapeType == ShapeType::Polyline);

	ShapeFile::PointType pt;

	if (polyIndex < static_cast<UInt32>(mv_Polygon.size()))
	{
		if (partIndex < static_cast<UInt32>(mv_Polygon[polyIndex].vParts.size()))
		{
			// offset to first point in this part
			UInt32 offset = mv_Polygon[polyIndex].vParts[partIndex];
			pt = mv_Polygon[polyIndex].vPoints[offset + index];
		}
	}

	return pt;
}

void ShapeFile::GetExtents(double& yn, double& ys, double& xe, double& xw)
{
	//TODO:
	//what are they>
	yn = ys = xw = xe = 0.0;
}

XString ShapeFile::GetMetaData()
{
	return "";
}
