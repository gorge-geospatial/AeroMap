/******************************************************************************
 * Copyright (c) 2017, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "TailFilter.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.tail",
		"Return N points from end of the point cloud.",
		"http://pdal.io/stages/filters.tail.html"
	};

	CREATE_STATIC_STAGE(TailFilter, s_info)

	std::string TailFilter::getName() const
	{
		return s_info.name;
	}
}
