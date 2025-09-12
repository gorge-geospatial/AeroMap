#ifndef CROPFILTER_H
#define CROPFILTER_H

#include <list>
#include <memory>

#include <pdal/Filter.h>
#include <pdal/Polygon.h>
#include <pdal/Streamable.h>

namespace pdal
{
	class ProgramArgs;
	class GridPnp;
	struct CropArgs;
	namespace filter
	{
		class Point;
	}

	// removes any points outside of the given range
	// updates the header accordingly
	class CropFilter : public Filter, public Streamable
	{
	public:
		CropFilter();
		~CropFilter();

		std::string getName() const;

	private:
		// This is just a way to marry a (multi)polygon with a list of its
		// GridPnp's that do the actual point-in-polygon operation.
		struct ViewGeom
		{
			ViewGeom(const Polygon& poly);
			ViewGeom(ViewGeom&& vg);

			Polygon m_poly;
			std::vector<std::unique_ptr<GridPnp>> m_gridPnps;
		};
		std::unique_ptr<CropArgs> m_args;
		double m_distance2;
		std::vector<ViewGeom> m_geoms;
		std::vector<Bounds> m_boxes;

		void addArgs(ProgramArgs& args);
		virtual void initialize();

		virtual void ready(PointTableRef table);
		virtual void spatialReferenceChanged(const SpatialReference& srs);
		virtual bool processOne(PointRef& point);
		virtual PointViewSet run(PointViewPtr view);
		bool crop(const PointRef& point, const BOX2D& box);
		bool crop(const PointRef& point, const BOX3D& box);
		void crop(const BOX3D& box, PointView& input, PointView& output);
		void crop(const BOX2D& box, PointView& input, PointView& output);
		void crop(const Bounds& box, PointView& input, PointView& output);
		bool crop(const PointRef& point, GridPnp& g);
		void crop(const ViewGeom& g, PointView& input, PointView& output);
		bool crop(const PointRef& point, const filter::Point& center);
		void crop(const filter::Point& center, PointView& input, PointView& output);
		void transform(const SpatialReference& srs);

		CropFilter& operator=(const CropFilter&); // not implemented
		CropFilter(const CropFilter&); // not implemented
	};
}

#endif // #ifndef CROPFILTER_H
