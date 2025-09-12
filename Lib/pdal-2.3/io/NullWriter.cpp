/******************************************************************************
* Copyright (c) 2015, Hobu Inc., hobu@hobu.co
****************************************************************************/

#include "NullWriter.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"writers.null",
		"Null writer.  Provides a sink for points in a pipeline.  "\
			"It's the same as sending pipeline output to /dev/null.",
		"http://pdal.io/stages/writers.null.html"
	};

	CREATE_STATIC_STAGE(NullWriter, s_info)

	std::string NullWriter::getName() const { return s_info.name; }
}
