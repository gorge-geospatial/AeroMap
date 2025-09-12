/******************************************************************************
 * Copyright (c) 2018, Connor Manning
 ****************************************************************************/

#pragma once

#include <unordered_set>

#include "Key.h"

namespace pdal
{
	struct Overlap
	{
		Overlap(const Key& key) : m_key(key), m_count(0), m_nodeId(0)
		{}
		Overlap(const Key& key, point_count_t count, uint64_t nodeId) :
			m_key(key), m_count(count), m_nodeId(nodeId)
		{}

		Key m_key;
		point_count_t m_count;
		uint64_t m_nodeId;
	};
	using Hierarchy = std::unordered_set<Overlap>;

	inline bool operator==(const Overlap& a, const Overlap& b)
	{
		return a.m_key == b.m_key;
	}
}

namespace std
{
	template<>
	struct hash<pdal::Overlap>
	{
		std::size_t operator()(const pdal::Overlap& o) const noexcept
		{
			return std::hash<pdal::Key>{}(o.m_key);
		}
	};
}
