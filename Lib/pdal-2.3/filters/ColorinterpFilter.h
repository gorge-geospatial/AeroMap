#ifndef COLORINTERPFILTER_H
#define COLORINTERPFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>
#include <filters/StatsFilter.h>

#include <map>

namespace pdal
{
	namespace gdal { class Raster; }

	// Interpolates color ramp into Red, Green, and Blue dimensions
	// for a given dimension
	// specified dimensions. It also supports scaling the data by a multiplier
	// on a per-dimension basis.
	class ColorinterpFilter : public Filter, public Streamable
	{
	public:

		ColorinterpFilter()
			: m_interpDim(Dimension::Id::Z)
			, m_interpDimString("Z")
			, m_min(0.0)
			, m_max(0.0)
			, m_clamp(false)
			, m_rampFilename("/vsimem/colorramp.png")
			, m_invertRamp(false)
			, m_stdDevThreshold(0.0)
			, m_useMAD(false)
			, m_madMultiplier(1.4862)
		{}
		ColorinterpFilter& operator=(const ColorinterpFilter&) = delete;
		ColorinterpFilter(const ColorinterpFilter&) = delete;
		std::string getName() const;

		virtual bool pipelineStreamable() const;
	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void filter(PointView& view);
		virtual void prepared(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual bool processOne(PointRef& point);

		Dimension::Id m_interpDim;
		std::string m_interpDimString;
		double m_min;
		double m_max;
		bool m_clamp;
		std::string m_colorramp;
		std::shared_ptr<gdal::Raster> m_raster;
		std::string m_rampFilename;
		std::vector<uint8_t> m_redBand;
		std::vector<uint8_t> m_greenBand;
		std::vector<uint8_t> m_blueBand;
		bool m_invertRamp;
		double m_stdDevThreshold;
		bool m_useMAD;
		double m_madMultiplier;
	};
}

#endif // #ifndef COLORINTERPFILTER_H
