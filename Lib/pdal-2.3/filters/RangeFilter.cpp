/******************************************************************************
 * Copyright (c) 2015, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "RangeFilter.h"

#include <pdal/util/ProgramArgs.h>
#include <pdal/util/Utils.h>

#include "private/DimRange.h"

#include <cctype>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.range",
		"Pass only points given a dimension/range.",
		"http://pdal.io/stages/filters.range.html"
	};

	CREATE_STATIC_STAGE(RangeFilter, s_info)

	std::string RangeFilter::getName() const
	{
		return s_info.name;
	}

	RangeFilter::RangeFilter()
	{}

	RangeFilter::~RangeFilter()
	{}

	void RangeFilter::addArgs(ProgramArgs& args)
	{
		args.add("limits", "Range limits", m_ranges).setPositional();
	}

	void RangeFilter::prepared(PointTableRef table)
	{
		const PointLayoutPtr layout(table.layout());

		for (auto& r : m_ranges)
		{
			r.m_id = layout->findDim(r.m_name);
			if (r.m_id == Dimension::Id::Unknown)
				throwError("Invalid dimension name in 'limits' option: '" +
					r.m_name + "'.");
		}
		std::sort(m_ranges.begin(), m_ranges.end());
	}

	// The range list is sorted by dimension, so the logic here should work
	// as ORs between ranges of the same dimension and ANDs between ranges
	// of different dimensions.  This is simple logic, but is probably the most
	// common case.
	bool RangeFilter::processOne(PointRef& point)
	{
		return DimRange::pointPasses(m_ranges, point);
	}

	PointViewSet RangeFilter::run(PointViewPtr inView)
	{
		PointViewSet viewSet;
		if (!inView->size())
			return viewSet;

		PointViewPtr outView = inView->makeNew();

		for (PointId i = 0; i < inView->size(); ++i)
		{
			PointRef point = inView->point(i);
			if (processOne(point))
				outView->appendPoint(*inView, i);
		}

		viewSet.insert(outView);
		return viewSet;
	}
}
