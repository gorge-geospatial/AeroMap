/******************************************************************************
 * Copyright (c) 2018, Connor Manning
 ****************************************************************************/

#pragma once

#include <memory>

#include <pdal/Artifact.h>
#include "Connector.h"
#include "EptInfo.h"
#include "Overlap.h"

namespace pdal
{
	class EptArtifact : public Artifact
	{
	public:
		EptArtifact(std::unique_ptr<EptInfo> info,
			std::unique_ptr<Hierarchy> hierarchy,
			std::unique_ptr<Connector> connector, size_t hierarchyStep) :
			m_info(std::move(info)), m_hierarchy(std::move(hierarchy)),
			m_connector(std::move(connector)), m_hierarchyStep(hierarchyStep)
		{}

		std::unique_ptr<EptInfo> m_info;
		std::unique_ptr<Hierarchy> m_hierarchy;
		std::unique_ptr<Connector> m_connector;
		size_t m_hierarchyStep;
	};
	using EptArtifactPtr = std::shared_ptr<EptArtifact>;
}
