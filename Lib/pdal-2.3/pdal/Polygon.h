#ifndef POLYGON_H
#define POLYGON_H

#include <pdal/Log.h>
#include <pdal/PointRef.h>
#include <pdal/SpatialReference.h>
#include <pdal/Geometry.h>

namespace pdal
{
	class BOX2D;
	class BOX3D;

	class Polygon : public Geometry
	{
		using Point = std::pair<double, double>;
		using Ring = std::vector<Point>;
		struct PrivateData;

	public:
		Polygon();
		virtual ~Polygon();

		Polygon(const std::string& wkt_or_json,	SpatialReference ref = SpatialReference());
		Polygon(const BOX2D&);
		Polygon(const BOX3D&);
		Polygon(OGRGeometryH g);
		Polygon(OGRGeometryH g, const SpatialReference& srs);
		Polygon(const Polygon& poly);
		Polygon& operator=(const Polygon& src);

		virtual void modified() override;
		virtual void clear() override;
		void simplify(double distance_tolerance, double area_tolerance,	bool preserve_topology = true);
		double area() const;
		std::vector<Polygon> polygons() const;

		bool covers(const PointRef& ref) const;
		bool equal(const Polygon& p) const;
		bool overlaps(const Polygon& p) const;
		bool contains(double x, double y) const;
		bool contains(const Polygon& p) const;
		bool intersects(const Polygon& p) const;
		bool disjoint(const Polygon& p) const;
		bool touches(const Polygon& p) const;
		bool within(const Polygon& p) const;
		bool crosses(const Polygon& p) const;
		Ring exteriorRing() const;
		std::vector<Ring> interiorRings() const;

	private:
		void init();
		void removeSmallRings(double tolerance);
		void removeSmallHoles(double tolerance);

		std::unique_ptr<PrivateData> m_pd;
	};
}

#endif // #ifndef POLYGON_H
