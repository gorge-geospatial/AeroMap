#ifndef COLORIZATIONFILTER_H
#define COLORIZATIONFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

#include <map>

namespace pdal
{
	namespace gdal { class Raster; }

	// Provides GDAL-based raster overlay that places output data in
	// specified dimensions. It also supports scaling the data by a multiplier
	// on a per-dimension basis.
	class ColorizationFilter : public Filter, public Streamable
	{
	public:
		struct BandInfo
		{
			BandInfo(const std::string& name, uint32_t band, double scale) :
				m_name(name), m_band(band), m_scale(scale),
				m_dim(Dimension::Id::Unknown), m_type(Dimension::Type::Double)
			{}

			BandInfo() : m_band(0), m_scale(1.0), m_dim(Dimension::Id::Unknown)
			{}

			std::string m_name;
			uint32_t m_band;
			double m_scale;
			Dimension::Id m_dim;
			Dimension::Type m_type;
		};

		ColorizationFilter();
		~ColorizationFilter();

		ColorizationFilter& operator=(const ColorizationFilter&) = delete;
		ColorizationFilter(const ColorizationFilter&) = delete;

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual void filter(PointView& view);

		StringList m_dimSpec;
		std::string m_rasterFilename;
		std::vector<BandInfo> m_bands;

		std::unique_ptr<gdal::Raster> m_raster;
	};
}

#endif // #ifndef COLORIZATIONFILTER_H
