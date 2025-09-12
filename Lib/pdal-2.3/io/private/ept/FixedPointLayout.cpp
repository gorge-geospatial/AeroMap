/******************************************************************************
 * Copyright (c) 2020, Hobu Inc.
* Copyright (c) 2018, Connor Manning
  ****************************************************************************/

#include <pdal/util/Algorithm.h>

#include "FixedPointLayout.h"

namespace pdal
{
	bool FixedPointLayout::update(Dimension::Detail dimDetail,
		const std::string& name)
	{
		if (m_finalized)
			return m_propIds.count(name);

		if (!Utils::contains(m_used, dimDetail.id()))
		{
			dimDetail.setOffset((int)m_pointSize);

			m_pointSize += dimDetail.size();
			m_used.push_back(dimDetail.id());
			m_detail[Utils::toNative(dimDetail.id())] = dimDetail;

			return true;
		}

		return false;
	}

	void FixedPointLayout::registerFixedDim(const Dimension::Id id,
		const Dimension::Type type)
	{
		Dimension::Detail dd = m_detail[Utils::toNative(id)];
		dd.setType(type);
		update(dd, Dimension::name(id));
	}

	Dimension::Id FixedPointLayout::registerOrAssignFixedDim(const std::string name,
		const Dimension::Type type)
	{
		Dimension::Id id = Dimension::id(name);
		if (id != Dimension::Id::Unknown)
		{
			registerFixedDim(id, type);
			return id;
		}
		return assignDim(name, type);
	}
}
