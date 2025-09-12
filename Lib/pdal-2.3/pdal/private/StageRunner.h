/******************************************************************************
* Copyright (c) 2014, Hobu Inc.
****************************************************************************/

#pragma once

#include <pdal/PointView.h>

namespace pdal
{
	class Stage;

	class StageRunner
	{
	public:
		StageRunner(Stage* s, PointViewPtr view);

		void run();
		PointViewPtr keeps();
		PointViewSet wait();

	private:
		Stage* m_stage;
		PointViewPtr m_keeps;
		PointViewPtr m_skips;
		PointViewSet m_viewSet;
	};
	typedef std::shared_ptr<StageRunner> StageRunnerPtr;
}
