/******************************************************************************
* Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "EstimateRankFilter.h"

#include <string>

#include <pdal/KDIndex.h>
#include <pdal/util/ProgramArgs.h>
#include <pdal/private/MathUtils.h>

namespace pdal
{
	using namespace Dimension;

	static StaticPluginInfo const s_info
	{
		"filters.estimaterank",
		"Computes the rank of a neighborhood of points.",
		"http://pdal.io/stages/filters.estimaterank.html"
	};

	CREATE_STATIC_STAGE(EstimateRankFilter, s_info)

	std::string EstimateRankFilter::getName() const
	{
		return s_info.name;
	}

	void EstimateRankFilter::addArgs(ProgramArgs& args)
	{
		args.add("knn", "k-Nearest Neighbors", m_knn, 8);
		args.add("thresh", "Threshold", m_thresh, 0.01);
	}

	void EstimateRankFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Id::Rank);
	}

	void EstimateRankFilter::filter(PointView& view)
	{
		const KD3Index& kdi = view.build3dIndex();

		for (PointRef p : view)
		{
			PointIdList ids = kdi.neighbors(p, m_knn);
			p.setField(Id::Rank, math::computeRank(view, ids, m_thresh));
		}
	}
}
