/******************************************************************************
* Copyright (c) 2019, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include <limits>

#include <pdal/SrsBounds.h>

namespace
{
	const double LOWEST = (std::numeric_limits<double>::lowest)();
	const double HIGHEST = (std::numeric_limits<double>::max)();
}

namespace pdal
{
	SrsBounds::SrsBounds(const BOX3D& box) : Bounds(box)
	{}

	SrsBounds::SrsBounds(const BOX3D& box, const SpatialReference& srs) :
		Bounds(box), m_srs(srs)
	{}

	SrsBounds::SrsBounds(const BOX2D& box) : Bounds(box)
	{}

	SrsBounds::SrsBounds(const BOX2D& box, const SpatialReference& srs) :
		Bounds(box), m_srs(srs)
	{}

	void SrsBounds::parse(const std::string& s, std::string::size_type& pos)
	{
		Bounds::parse(s, pos);
		pos += Utils::extractSpaces(s, pos);
		if (pos == s.size())
			return;

		if (s[pos++] != '/')
			throw Bounds::error("Invalid character following valid bounds box.");

		pos += Utils::extractSpaces(s, pos);
		SpatialReference srs;
		m_srs.parse(s, pos);
		pos += Utils::extractSpaces(s, pos);
	}

	std::ostream& operator << (std::ostream& out, const SrsBounds& srsBounds)
	{
		out << static_cast<const Bounds&>(srsBounds);
		out << " / " << srsBounds.m_srs;
		return out;
	}
}
