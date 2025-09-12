#ifndef POINT_H
#define POINT_H

#include <pdal/pdal_export.h>  // Suppresses windows 4251 warn. in ogr code.
#include <ogr_geometry.h>

#include <pdal/Geometry.h>

namespace pdal
{
	namespace filter
	{
		class Point : public Geometry
		{
		public:
			Point();
			Point(const std::string& wkt_or_json, SpatialReference ref);
			bool is3d() const;
			bool empty() const;
			void clear();
			double x() const;
			double y() const;
			double z() const;
			void x(double x);
			void y(double y);
			void z(double z);

			virtual void update(const std::string& wkt_or_json);
		};
	}
}

#endif // #ifndef POINT_H
