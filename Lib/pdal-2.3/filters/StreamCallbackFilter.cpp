/******************************************************************************
* Copyright (c) 2015, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include "StreamCallbackFilter.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.streamcallback",
		"Provide a hook for a simple point-by-point callback.",
		""
	};

	CREATE_STATIC_STAGE(StreamCallbackFilter, s_info)
}
