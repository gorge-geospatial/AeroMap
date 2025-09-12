/******************************************************************************
 * Copyright (c) 2015, Hobu Inc. (hobu@hobu.co)
 * Copyright (c) 2019, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "RandomizeFilter.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.randomize", 
		"Randomize points in a view.",
		"http://pdal.io/stages/filters.randomize.html" 
	};

	CREATE_STATIC_STAGE(RandomizeFilter, s_info)

	std::string RandomizeFilter::getName() const
	{
		return s_info.name;
	}

	void RandomizeFilter::addArgs(ProgramArgs& args)
	{
		m_seedArg = &args.add("seed", "Random number generator seed", m_seed, 0u);
	}
}
