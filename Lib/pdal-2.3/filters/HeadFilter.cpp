/******************************************************************************
 * Copyright (c) 2017, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "HeadFilter.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.head",
		"Return N points from beginning of the point cloud.",
		"http://pdal.io/stages/filters.head.html"
	};

	CREATE_STATIC_STAGE(HeadFilter, s_info)

	std::string HeadFilter::getName() const
	{
		return s_info.name;
	}

	void HeadFilter::addArgs(ProgramArgs& args)
	{
		args.add("count", "Number of points to return from beginning.  "
			"If 'invert' is true, number of points to drop from the beginning.",
			m_count, point_count_t(10));
		args.add("invert", "If true, 'count' specifies the number of points "
			"to skip from the beginning.", m_invert, false);
	}

	void HeadFilter::ready(PointTableRef table)
	{
		m_index = 0;
	}

	bool HeadFilter::processOne(PointRef& point)
	{

		bool keep = false;
		if (m_index < m_count)
			keep = true;
		m_index++;

		if (m_invert)
			keep = !keep;
		return keep;


	}

	PointViewSet HeadFilter::run(PointViewPtr inView)
	{
		m_index = 0;

		PointViewSet viewSet;
		if (!inView->size())
			return viewSet;

		PointViewPtr outView = inView->makeNew();

		for (PointRef point : *inView)
		{
			if (processOne(point))
				outView->appendPoint(*inView, point.pointId());
		}

		viewSet.insert(outView);
		return viewSet;
	}
}
