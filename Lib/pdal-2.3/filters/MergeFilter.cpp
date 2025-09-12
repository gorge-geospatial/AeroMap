/******************************************************************************
 * Copyright (c) 2014, Hobu Inc., hobu.inc@gmail.com
 * Copyright (c) 2015, Bradley J Chambers, brad.chambers@gmail.com
 ****************************************************************************/

#include "MergeFilter.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.merge",
		"Merge data from two different readers into a single stream.",
		"http://pdal.io/stages/filters.merge.html"
	};

	CREATE_STATIC_STAGE(MergeFilter, s_info)

	std::string MergeFilter::getName() const { return s_info.name; }

	void MergeFilter::ready(PointTableRef table)
	{
		SpatialReference srs = getSpatialReference();

		if (srs.empty())
			srs = table.anySpatialReference();
		m_view.reset(new PointView(table, srs));
	}

	PointViewSet MergeFilter::run(PointViewPtr in)
	{
		PointViewSet viewSet;

		// If the SRS of all the point views aren't the same, print a warning
		// unless we're explicitly overriding the SRS.
		if (getSpatialReference().empty() &&
			(in->spatialReference() != m_view->spatialReference()))
			log()->get(LogLevel::Warning) << getName() << ": merging points "
			"with inconsistent spatial references." << std::endl;
		m_view->append(*in.get());
		viewSet.insert(m_view);
		return viewSet;
	}
}
