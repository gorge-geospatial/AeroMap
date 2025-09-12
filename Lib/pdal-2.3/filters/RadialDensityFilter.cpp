/******************************************************************************
* Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "RadialDensityFilter.h"

#include <pdal/KDIndex.h>

#include <string>

namespace pdal
{
	using namespace Dimension;

	static StaticPluginInfo const s_info
	{
		"filters.radialdensity",
		"RadialDensity Filter",
		"http://pdal.io/stages/filters.radialdensity.html"
	};

	CREATE_STATIC_STAGE(RadialDensityFilter, s_info)

	std::string RadialDensityFilter::getName() const
	{
		return s_info.name;
	}

	void RadialDensityFilter::addArgs(ProgramArgs& args)
	{
		args.add("radius", "Radius", m_rad, 1.0);
	}

	void RadialDensityFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Id::RadialDensity);
	}

	void RadialDensityFilter::filter(PointView& view)
	{
		// Build the 3D KD-tree.
		const KD3Index& index = view.build3dIndex();

		// Search for neighboring points within the specified radius. The number of
		// neighbors (which includes the query point) is normalized by the volume
		// of the search sphere and recorded as the density.
		log()->get(LogLevel::Debug) << "Computing densities...\n";
		double factor = 1.0 / ((4.0 / 3.0) * 3.14159 * (m_rad * m_rad * m_rad));
		for (PointRef p : view)
		{
			PointIdList pts = index.radius(p, m_rad);
			p.setField(Id::RadialDensity, pts.size() * factor);
		}
	}
}
