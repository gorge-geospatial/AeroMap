/******************************************************************************
 * Copyright (c) 2018, Connor Manning
 ****************************************************************************/

#pragma once

#include <nlohmann/json.h>
#include <pdal/util/Bounds.h>

namespace pdal
{
	inline BOX3D toBox3d(const NL::json& b)
	{
		if (!b.is_array() || b.size() != 6)
		{
			throw pdal_error("Invalid bounds specification: " + b.dump());
		}

		return BOX3D(b[0].get<double>(), b[1].get<double>(), b[2].get<double>(),
			b[3].get<double>(), b[4].get<double>(), b[5].get<double>());
	}
}

