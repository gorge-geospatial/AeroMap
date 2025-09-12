/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#include "DecimationFilter.h"

#include <pdal/PointView.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.decimation",
		"Rank decimation filter. Keep every Nth point",
		"http://pdal.io/stages/filters.decimation.html"
	};

	CREATE_STATIC_STAGE(DecimationFilter, s_info)

	std::string DecimationFilter::getName() const { return s_info.name; }

	void DecimationFilter::addArgs(ProgramArgs& args)
	{
		args.add("step", "Points to delete between each kept point", m_step, 1U);
		args.add("offset", "Index of first point to consider including in output",
			m_offset);
		args.add("limit", "Index of last point to consider including in output",
			m_limit, (std::numeric_limits<point_count_t>::max)());
	}

	PointViewSet DecimationFilter::run(PointViewPtr inView)
	{
		PointViewSet viewSet;
		PointViewPtr outView = inView->makeNew();
		decimate(*inView.get(), *outView.get());
		viewSet.insert(outView);
		return viewSet;
	}

	bool DecimationFilter::processOne(PointRef& point)
	{
		bool keep = true;
		if (m_index < m_offset || m_index >= m_limit)
			keep = false;
		else if ((m_index - m_offset) % m_step != 0)
			keep = false;
		m_index++;
		return keep;
	}

	void DecimationFilter::decimate(PointView& input, PointView& output)
	{
		PointId last_idx = (std::min)(m_limit, input.size());
		for (PointId idx = m_offset; idx < last_idx; idx += m_step)
			output.appendPoint(input, idx);
	}
}
