/******************************************************************************
* Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "IQRFilter.h"

#include <string>
#include <vector>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.iqr",
		"Interquartile Range Filter",
		"http://pdal.io/stages/filters.iqr.html"
	};

	CREATE_STATIC_STAGE(IQRFilter, s_info)

	std::string IQRFilter::getName() const
	{
		return s_info.name;
	}

	void IQRFilter::addArgs(ProgramArgs& args)
	{
		args.add("k", "Number of deviations", m_multiplier, 1.5);
		args.add("dimension", "Dimension on which to calculate statistics",	m_dimName);
	}

	void IQRFilter::prepared(PointTableRef table)
	{
		PointLayoutPtr layout(table.layout());
		m_dimId = layout->findDim(m_dimName);
		if (m_dimId == Dimension::Id::Unknown)
			throwError("Dimension '" + m_dimName + "' does not exist.");
	}

	PointViewSet IQRFilter::run(PointViewPtr view)
	{
		using namespace Dimension;

		PointViewPtr output = view->makeNew();

		auto quartile = [](std::vector<double> vals, double percent)
		{
			std::nth_element(vals.begin(),
				vals.begin() + int(vals.size() * percent), vals.end());

			return *(vals.begin() + int(vals.size() * percent));
		};

		std::vector<double> z(view->size());
		for (PointId j = 0; j < view->size(); ++j)
			z[j] = view->getFieldAs<double>(m_dimId, j);


		double pc25 = quartile(z, 0.25);
		log()->get(LogLevel::Debug) << "25th percentile: " << pc25 << std::endl;

		double pc75 = quartile(z, 0.75);
		log()->get(LogLevel::Debug) << "75th percentile: " << pc75 << std::endl;

		double iqr = pc75 - pc25;
		log()->get(LogLevel::Debug) << "IQR: " << iqr << std::endl;

		double low_fence = pc25 - m_multiplier * iqr;
		double hi_fence = pc75 + m_multiplier * iqr;

		for (PointId j = 0; j < view->size(); ++j)
		{
			double val = view->getFieldAs<double>(m_dimId, j);
			if (val > low_fence && val < hi_fence)
				output->appendPoint(*view, j);
		}
		log()->get(LogLevel::Debug) << "Cropping " << m_dimName
			<< " in the range (" << low_fence
			<< "," << hi_fence << ")" << std::endl;

		PointViewSet viewSet;
		viewSet.insert(output);
		return viewSet;
	}
}
