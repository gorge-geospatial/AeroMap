/******************************************************************************
 * Copyright (c) 2019, Guilhem Villemin (guilhem.villemin@gmail.com)
 ****************************************************************************/

#include "SeparateScanLineFilter.h"

#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.separatescanline",
		"Split data by scan line.",
		"http://pdal.io/stages/filters.separatescanline.html"
	};

	CREATE_STATIC_STAGE(SeparateScanLineFilter, s_info)

	SeparateScanLineFilter::SeparateScanLineFilter()
	{}

	std::string SeparateScanLineFilter::getName() const
	{
		return s_info.name;
	}

	void SeparateScanLineFilter::addArgs(ProgramArgs& args)
	{
		args.add("groupby", "Number of lines to be grouped by", m_groupBy, (uint64_t)1u);
	}

	void SeparateScanLineFilter::prepared(PointTableRef table)
	{
		PointLayoutPtr layout(table.layout());
		if (!layout->hasDim(Dimension::Id::EdgeOfFlightLine))
			throwError("Layout does not contains EdgeOfFlightLine dimension.");
	}

	PointViewSet SeparateScanLineFilter::run(PointViewPtr inView)
	{
		PointViewSet result;
		PointViewPtr v(inView->makeNew());
		result.insert(v);

		uint64_t lineNum = 1;
		for (PointId i = 0; i < inView->size(); ++i)
		{
			v->appendPoint(*inView, i);
			if (inView->getFieldAs<uint8_t>(Dimension::Id::EdgeOfFlightLine, i))
			{
				if (++lineNum > m_groupBy)
				{
					v = inView->makeNew();
					result.insert(v);
					lineNum = 1;
				}
			}
		}
		//if last point was an edge of flight line
		if (v->empty())
			result.erase(v);

		return result;
	}
}
