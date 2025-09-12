/******************************************************************************
 * Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "LocateFilter.h"

#include <pdal/util/ProgramArgs.h>
#include <pdal/util/Utils.h>

namespace pdal
{

	static StaticPluginInfo const s_info
	{
		"filters.locate",
		"Return a single point with min/max value in the named dimension.",
		"http://pdal.io/stages/filters.locate.html"
	};

	CREATE_STATIC_STAGE(LocateFilter, s_info)

	std::string LocateFilter::getName() const
	{
		return s_info.name;
	}

	void LocateFilter::addArgs(ProgramArgs& args)
	{
		args.add("dimension", "Dimension in which to locate max", m_dimName);
		args.add("minmax", "Whether to search for the minimum or maximum value", m_minmax, "max");
	}

	void LocateFilter::prepared(PointTableRef table)
	{
		PointLayoutPtr layout(table.layout());
		m_dimId = layout->findDim(m_dimName);
		if (m_dimId == Dimension::Id::Unknown)
			throwError("Invalid dimension '" + m_dimName + "'.");
	}

	PointViewSet LocateFilter::run(PointViewPtr inView)
	{
		PointViewSet viewSet;
		if (!inView->size())
			return viewSet;

		PointId minidx, maxidx;
		double minval = (std::numeric_limits<double>::max)();
		double maxval = std::numeric_limits<double>::lowest();

		for (PointId idx = 0; idx < inView->size(); idx++)
		{
			double val = inView->getFieldAs<double>(m_dimId, idx);
			if (val > maxval)
			{
				maxval = val;
				maxidx = idx;
			}
			if (val < minval)
			{
				minval = val;
				minidx = idx;
			}
		}

		PointViewPtr outView = inView->makeNew();

		if (Utils::iequals("min", m_minmax))
			outView->appendPoint(*inView.get(), minidx);
		if (Utils::iequals("max", m_minmax))
			outView->appendPoint(*inView.get(), maxidx);

		viewSet.insert(outView);
		return viewSet;
	}
}
