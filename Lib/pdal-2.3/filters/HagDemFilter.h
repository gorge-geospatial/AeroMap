#ifndef HAGDEMFILTER_H
#define HAGDEMFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

#include <cstdint>
#include <memory>
#include <string>

namespace pdal
{
	namespace gdal { class Raster; }
	class Options;
	class PointLayout;
	class PointView;

	class HagDemFilter : public Filter, public Streamable
	{
	public:
		HagDemFilter();
		HagDemFilter& operator=(const HagDemFilter&) = delete;
		HagDemFilter(const HagDemFilter&) = delete;

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual void filter(PointView& view);
		virtual bool processOne(PointRef& point);

		std::unique_ptr<gdal::Raster> m_raster;
		std::string m_rasterName;
		bool m_zeroGround;
		int32_t m_band;
	};
}

#endif // #ifndef HAGDEMFILTER_H
