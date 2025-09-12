/******************************************************************************
 * Copyright (c) 2019, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "FarthestPointSamplingFilter.h"

#include "private/Segmentation.h"

#include <pdal/KDIndex.h>
#include <pdal/util/ProgramArgs.h>

#include <algorithm>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

namespace pdal
{
	static PluginInfo const s_info
	{ 
		"filters.fps", 
		"Farthest point sampling filter",
		"http://pdal.io/stages/filters.fps.html" 
	};

	CREATE_STATIC_STAGE(FarthestPointSamplingFilter, s_info)

	std::string FarthestPointSamplingFilter::getName() const
	{
		return s_info.name;
	}

	FarthestPointSamplingFilter::FarthestPointSamplingFilter()
	{
	}

	void FarthestPointSamplingFilter::addArgs(ProgramArgs& args)
	{
		args.add("count", "Target number of points after sampling", m_count, point_count_t(1000));
	}

	PointViewSet FarthestPointSamplingFilter::run(PointViewPtr inView)
	{
		// Return empty PointViewSet if the input PointView has no points.
		PointViewSet viewSet;
		if (!inView->size())
			return viewSet;

		// Return inView if input PointView has fewer than count points.
		if (inView->size() < m_count)
		{
			viewSet.insert(inView);
			return viewSet;
		}

		PointIdList ids = Segmentation::farthestPointSampling(*inView, m_count);

		PointViewPtr outView = inView->makeNew();
		for (auto const& id : ids)
			outView->appendPoint(*inView, id);
		viewSet.insert(outView);
		return viewSet;
	}
}
