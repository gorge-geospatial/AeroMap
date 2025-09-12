/******************************************************************************
 * Copyright (c) 2016, Hobu Inc.
 ****************************************************************************/

#pragma once

#include <pdal/Dimension.h>
#include <pdal/pdal_types.h>

namespace pdal
{
	struct DimType
	{
		DimType() 
			: m_id(Dimension::Id::Unknown)
			, m_type(Dimension::Type::None)
		{
		}

		DimType(Dimension::Id id, Dimension::Type type,
			double scale = 1.0, double offset = 0.0)
				: m_id(id)
				, m_type(type)
				, m_xform(scale, offset)
		{
		}

		DimType(Dimension::Id id, Dimension::Type type, XForm xform)
			: m_id(id)
			, m_type(type)
			, m_xform(xform)
		{
		}

		Dimension::Id m_id;
		Dimension::Type m_type;
		XForm m_xform;  // A convenience for some formats.
	};
	typedef std::vector<DimType> DimTypeList;
}
