#ifndef OVERLAYFILTER_H
#define OVERLAYFILTER_H

#include <pdal/Filter.h>
#include <pdal/Polygon.h>
#include <pdal/Streamable.h>

#include <map>
#include <memory>
#include <string>

typedef void* OGRLayerH;

namespace pdal
{
	namespace gdal
	{
		class ErrorHandler;
	}

	typedef std::shared_ptr<void> OGRDSPtr;
	typedef std::shared_ptr<void> OGRFeaturePtr;
	typedef std::shared_ptr<void> OGRGeometryPtr;

	class Arg;

	class OverlayFilter : public Filter, public Streamable
	{
		struct PolyVal
		{
			Polygon geom;
			int32_t val;
		};

	public:
		OverlayFilter() : m_ds(0), m_lyr(0)
		{}

		std::string getName() const { return "filters.overlay"; }

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void spatialReferenceChanged(const SpatialReference& srs);
		virtual bool processOne(PointRef& point);
		virtual void initialize();
		virtual void prepared(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual void filter(PointView& view);

		OverlayFilter& operator=(const OverlayFilter&) = delete;
		OverlayFilter(const OverlayFilter&) = delete;

		typedef std::shared_ptr<void> OGRDSPtr;

		OGRDSPtr m_ds;
		OGRLayerH m_lyr;
		std::string m_dimName;
		std::string m_datasource;
		std::string m_column;
		std::string m_query;
		std::string m_layer;
		Dimension::Id m_dim;
		std::vector<PolyVal> m_polygons;
	};
}

#endif // #ifndef OVERLAYFILTER_H
