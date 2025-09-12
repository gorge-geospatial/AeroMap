/******************************************************************************
* Copyright (c) 2014, Hobu Inc.
****************************************************************************/

#include "StageRunner.h"

#include <pdal/Filter.h>

namespace pdal
{
	StageRunner::StageRunner(Stage* s, PointViewPtr view) : m_stage(s)
	{
		m_keeps = view->makeNew();
		m_skips = view->makeNew();
		m_stage->splitView(view, m_keeps, m_skips);
	}

	PointViewPtr StageRunner::keeps()
	{
		return m_keeps;
	}

	// For now this is all synchronous
	void StageRunner::run()
	{
		point_count_t keepSize = m_keeps->size();
		m_viewSet = m_stage->run(m_keeps);

		if (m_skips->size() == 0)
			return;

		if (m_stage->mergeMode() == Filter::WhereMergeMode::True)
		{
			if (m_viewSet.size())
				(*m_viewSet.begin())->append(*m_skips);
			return;
		}
		else if (m_stage->mergeMode() == Filter::WhereMergeMode::Auto)
		{
			if (m_viewSet.size() == 1)
			{
				PointViewPtr keeps = *m_viewSet.begin();
				if (keeps.get() == m_keeps.get() && keepSize == keeps->size())
				{
					keeps->append(*m_skips);
					return;
				}
			}
		}
		m_viewSet.insert(m_skips);
	}

	PointViewSet StageRunner::wait()
	{
		return m_viewSet;
	}
}
