/******************************************************************************
* Copyright (c) 2018, Hobu Inc.
****************************************************************************/

#pragma once

#include <memory>

namespace pdal
{
	class Artifact
	{
	public:
		virtual ~Artifact()  // Need to make sure we have a virt. func. tbl.
		{}
	};

	using ArtifactPtr = std::shared_ptr<Artifact>;
}
