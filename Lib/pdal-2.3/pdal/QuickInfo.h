/******************************************************************************
* Copyright (c) 2014, Hobu Inc.
****************************************************************************/

#pragma once

#include <vector>

#include <pdal/util/Bounds.h>
#include <pdal/SpatialReference.h>

namespace pdal
{
	struct QuickInfo
	{
	public:
		BOX3D m_bounds;
		SpatialReference m_srs;
		point_count_t m_pointCount;
		StringList m_dimNames;
		bool m_valid;

		QuickInfo()
			: m_pointCount(0)
			, m_valid(false)
		{
		}

		bool valid() const
		{
			return m_valid;
		}
	};
}
