/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#pragma once

#include <pdal/JsonFwd.h>
#include <pdal/SpatialReference.h>
#include <pdal/util/Bounds.h>

class OGRGeometry;
typedef void* OGRGeometryH;

#include <vector>

namespace pdal
{
	class Polygon;

	namespace gdal
	{
		void registerDrivers();
		void unregisterDrivers();
		bool reprojectBounds(Bounds& box, const SpatialReference& srcSrs, const SpatialReference& dstSrs);
		bool reprojectBounds(BOX3D& box, const SpatialReference& srcSrs, const SpatialReference& dstSrs);
		bool reprojectBounds(BOX2D& box, const SpatialReference& srcSrs, const SpatialReference& dstSrs);
		bool reproject(double& x, double& y, double& z, const SpatialReference& srcSrs, const SpatialReference& dstSrs);
		std::string lastError();

		// New signatures to support extraction of SRS from the end of geometry
		// specifications.
		OGRGeometry* createFromWkt(const std::string& s, std::string& srs);
		OGRGeometry* createFromGeoJson(const std::string& s, std::string& srs);

		std::vector<Polygon> getPolygons(const NL::json& ogr);

		inline OGRGeometry* fromHandle(OGRGeometryH geom)
		{
			return reinterpret_cast<OGRGeometry*>(geom);
		}

		inline OGRGeometryH toHandle(OGRGeometry* h)
		{
			return reinterpret_cast<OGRGeometryH>(h);
		}
	}
}
