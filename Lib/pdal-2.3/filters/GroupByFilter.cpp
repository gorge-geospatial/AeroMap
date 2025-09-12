/******************************************************************************
 * Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "GroupByFilter.h"

#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.groupby",
		"Split data categorically by dimension.",
		"http://pdal.io/stages/filters.groupby.html"
	};
	CREATE_STATIC_STAGE(GroupByFilter, s_info)

	GroupByFilter::GroupByFilter() : m_viewMap()
	{}

	std::string GroupByFilter::getName() const
	{
		return s_info.name;
	}

	void GroupByFilter::addArgs(ProgramArgs& args)
	{
		args.add("dimension", "Dimension containing data to be grouped", m_dimName);
	}

	void GroupByFilter::prepared(PointTableRef table)
	{
		PointLayoutPtr layout(table.layout());
		m_dimId = layout->findDim(m_dimName);
		if (m_dimId == Dimension::Id::Unknown)
			throwError("Invalid dimension name '" + m_dimName + "'.");
		// also need to check that we have a dimension with discrete values
	}

	PointViewSet GroupByFilter::run(PointViewPtr inView)
	{
		PointViewSet viewSet;
		if (!inView->size())
			return viewSet;

		for (PointId idx = 0; idx < inView->size(); idx++)
		{
			int64_t val = inView->getFieldAs<int64_t>(m_dimId, idx);
			PointViewPtr& outView = m_viewMap[val];
			if (!outView)
				outView = inView->makeNew();
			outView->appendPoint(*inView.get(), idx);
		}

		// Pull the buffers out of the map and stick them in the standard
		// output set.
		for (auto bi = m_viewMap.begin(); bi != m_viewMap.end(); ++bi)
			viewSet.insert(bi->second);
		return viewSet;
	}
}
