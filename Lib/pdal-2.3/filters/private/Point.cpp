/******************************************************************************
* Copyright (c) 2016, Howard Butler (howard@hobu.co)
****************************************************************************/

#include "Point.h"

namespace pdal
{
	namespace filter
	{
		namespace
		{
			const double LOWEST = (std::numeric_limits<double>::lowest)();
		}

		Point::Point()
		{
			m_geom.reset(new OGRPoint());
			clear();
		}

		Point::Point(const std::string& wkt_or_json, SpatialReference ref)
			: Geometry(wkt_or_json, ref)
		{}

		void Point::update(const std::string& wkt_or_json)
		{
			Geometry::update(wkt_or_json);

			OGRwkbGeometryType t = m_geom->getGeometryType();
			if (t != wkbPoint && t != wkbPoint25D)
			{
				m_geom.reset(new OGRPoint());
				throw pdal_error("Can't set Point from string.  String doesn't "
					"represent a point");
			}

			// We use a sentinel for 3D that's different from what GDAL uses.
			if (m_geom->getCoordinateDimension() == 2)
				z(LOWEST);
		}

		void Point::clear()
		{
			x(LOWEST);
			y(LOWEST);
			z(LOWEST);
		}

		bool Point::empty() const
		{
			return (x() == LOWEST && y() == LOWEST && z() == LOWEST);
		}

		bool Point::is3d() const
		{
			return (z() != LOWEST);
		}

		double Point::x() const
		{
			return static_cast<OGRPoint*>(m_geom.get())->getX();
		}

		double Point::y() const
		{
			return static_cast<OGRPoint*>(m_geom.get())->getY();
		}

		double Point::z() const
		{
			return static_cast<OGRPoint*>(m_geom.get())->getZ();
		}

		void Point::x(double xval)
		{
			static_cast<OGRPoint*>(m_geom.get())->setX(xval);
		}

		void Point::y(double yval)
		{
			static_cast<OGRPoint*>(m_geom.get())->setY(yval);
		}

		void Point::z(double zval)
		{
			static_cast<OGRPoint*>(m_geom.get())->setZ(zval);
		}
	}
}
