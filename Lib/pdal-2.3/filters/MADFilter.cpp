/******************************************************************************
* Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "MADFilter.h"

#include <string>
#include <vector>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.mad",
		"Median Absolute Deviation Filter",
		"http://pdal.io/stages/filters.mad.html"
	};

	CREATE_STATIC_STAGE(MADFilter, s_info)

	std::string MADFilter::getName() const
	{
		return s_info.name;
	}

	void MADFilter::addArgs(ProgramArgs& args)
	{
		args.add("k", "Number of deviations", m_multiplier, 2.0);
		args.add("dimension", "Dimension on which to calculate statistics",	m_dimName);
		args.add("mad_multiplier", "MAD threshold multiplier", m_madMultiplier, 1.4862);
	}

	void MADFilter::prepared(PointTableRef table)
	{
		PointLayoutPtr layout(table.layout());
		m_dimId = layout->findDim(m_dimName);
		if (m_dimId == Dimension::Id::Unknown)
			throwError("Dimension '" + m_dimName + "' does not exist.");
	}

	PointViewSet MADFilter::run(PointViewPtr view)
	{
		using namespace Dimension;

		PointViewPtr output = view->makeNew();

		auto estimate_median = [](std::vector<double> vals)
		{
			std::nth_element(vals.begin(), vals.begin() + vals.size() / 2, vals.end());
			return *(vals.begin() + vals.size() / 2);
		};

		std::vector<double> z(view->size());
		for (PointId j = 0; j < view->size(); ++j)
			z[j] = view->getFieldAs<double>(m_dimId, j);

		double median = estimate_median(z);
		log()->get(LogLevel::Debug) << getName() <<
			" estimated median value: " << median << std::endl;

		std::transform(z.begin(), z.end(), z.begin(),
			[median](double v) { return std::fabs(v - median); });
		double mad = estimate_median(z) * m_madMultiplier;
		log()->get(LogLevel::Debug) << getName() << " mad " << mad << std::endl;

		for (PointId j = 0; j < view->size(); ++j)
		{
			if (z[j] / mad < m_multiplier)
				output->appendPoint(*view, j);
		}

		double low_fence = median - m_multiplier * mad;
		double hi_fence = median + m_multiplier * mad;

		log()->get(LogLevel::Debug) << getName() << " cropping " << m_dimName
			<< " in the range (" << low_fence
			<< "," << hi_fence << ")" << std::endl;

		PointViewSet viewSet;
		viewSet.insert(output);
		return viewSet;
	}
}
