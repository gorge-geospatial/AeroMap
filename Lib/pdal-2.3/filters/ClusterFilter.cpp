/******************************************************************************
 * Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "ClusterFilter.h"

#include <pdal/KDIndex.h>

#include "private/Segmentation.h"

#include <string>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.cluster",
		"Extract and label clusters using Euclidean distance.",
		"http://pdal.io/stages/filters.cluster.html"
	};

	CREATE_STATIC_STAGE(ClusterFilter, s_info)

	std::string ClusterFilter::getName() const
	{
		return s_info.name;
	}

	void ClusterFilter::addArgs(ProgramArgs& args)
	{
		args.add("min_points", "Min points per cluster", m_minPoints, static_cast<uint64_t>(1));
		args.add("max_points", "Max points per cluster", m_maxPoints, (std::numeric_limits<uint64_t>::max)());
		args.add("tolerance", "Radius", m_tolerance, 1.0);
		args.add("is3d", "Perform cluster extraction in 3D?", m_is3d, true);
	}

	void ClusterFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Dimension::Id::ClusterID);
	}

	void ClusterFilter::filter(PointView& view)
	{
		std::deque<PointIdList> clusters;
		if (m_is3d)
			clusters = Segmentation::extractClusters<KD3Index>(view, m_minPoints,
				m_maxPoints, m_tolerance);
		else
			clusters = Segmentation::extractClusters<KD2Index>(view, m_minPoints,
				m_maxPoints, m_tolerance);

		uint64_t id = 1;
		for (auto const& c : clusters)
		{
			for (auto const& i : c)
				view.setField(Dimension::Id::ClusterID, i, id);
			id++;
		}
	}
}
